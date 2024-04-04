#include "stdafx.h"
#include "TCPClient.h"
#include "GameFramework.h"
#include "Player.h"

CTcpClient::CTcpClient(HWND hWnd)
{
	CreateSocket(hWnd);
}

CTcpClient::~CTcpClient()
{
}

void CTcpClient::CreateSocket(HWND hWnd)
{
	int nRetval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		return;
	}

	// ���� ����
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == INVALID_SOCKET)
	{
		err_quit("socket()");
	}

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);

	nRetval = connect(m_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (nRetval == SOCKET_ERROR) 
	{
		err_quit("connect()");
		return;
	}

	nRetval = WSAAsyncSelect(m_sock, hWnd, WM_SOCKET, FD_CLOSE | FD_READ | FD_WRITE);	// FD_WRITE�� �߻��Ұ��̴�.
}

void CTcpClient::OnDestroy()
{

}


XMFLOAT3 CTcpClient::GetPostion(int id)
{
	XMFLOAT3 position = { 0.0f,0.0f, 0.0f };

	return position;
}

std::array<CS_CLIENTS_INFO, 5>& CTcpClient::GetArrayClientsInfo()
{
	return m_aClientInfo;
}

void CTcpClient::OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_WRITE:	// ������ �����͸� ������ �غ� �Ǿ���.
		OnProcessingWriteMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case FD_READ:	// ������ �����͸� ���� �غ� �Ǿ���.
		OnProcessingReadMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case FD_CLOSE:
		closesocket(m_sock);
		WSACleanup();	
		break;
	default:
		break;
	}
}

void CTcpClient::OnProcessingReadMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	static int nHead;
	int nRetval = 1;
	size_t nBufferSize;

	if(!m_bRecvHead)
	{
		nBufferSize = sizeof(int);
		nRetval = RecvData(nBufferSize);
		if (nRetval != 0)
		{
			//PostMessage(hWnd, WM_SOCKET, (WPARAM)m_sock, MAKELPARAM(FD_READ, 0));
			return;
		}
		m_bRecvHead = true;
		memcpy(&nHead, m_pCurrentBuffer, sizeof(int));
		memset(m_pCurrentBuffer, 0, BUFSIZE);
	}

	switch (nHead)
	{
	case HEAD_INIT:
		nBufferSize = sizeof(int) * 2;
		nRetval = RecvData(nBufferSize);
		if (nRetval != 0)
		{
			break;
		}

		memcpy(&m_nMainClientID, m_pCurrentBuffer, sizeof(int));
		memcpy(&m_nClient, m_pCurrentBuffer + sizeof(int), sizeof(int));
		break;
	case HEAD_UPDATE_DATA:
	{
		nBufferSize = sizeof(m_aClientInfo);
		nRetval = RecvData(nBufferSize);
		if (nRetval != 0)
		{
			break;
		}
		
		for (int i = 0; i < MAX_CLIENT; ++i)
		{
			memcpy(&m_aClientInfo[i], m_pCurrentBuffer + sizeof(CS_CLIENTS_INFO) * i, sizeof(CS_CLIENTS_INFO));

			if (m_apPlayers[i])
			{
				m_apPlayers[i]->SetClientId(m_aClientInfo[i].m_nClientId);
				m_apPlayers[i]->SetPosition(m_aClientInfo[i].m_xmf3Position);
				m_apPlayers[i]->SetVelocity(m_aClientInfo[i].m_xmf3Velocity);
				
				if(i != m_nMainClientID)
				{
					m_apPlayers[i]->SetLook(m_aClientInfo[i].m_xmf3Look);
					m_apPlayers[i]->SetRight(m_aClientInfo[i].m_xmf3Right);
				}
			}
		}
	}
		break;
	case HEAD_NUM_OF_CLIENT:
		nBufferSize = sizeof(int) + sizeof(m_aClientInfo);
		nRetval = RecvData(nBufferSize);
		if (nRetval != 0)
		{
			break;
		}

		memcpy(&m_nClient, m_pCurrentBuffer, sizeof(int));
		memcpy(&m_aClientInfo, m_pCurrentBuffer + sizeof(int), sizeof(m_aClientInfo));
		for(int i=0;i<MAX_CLIENT;++i)
		{
			if (m_apPlayers[i])
			{
				m_apPlayers[i]->SetClientId(m_aClientInfo[i].m_nClientId);
			}
		}
		break;
	default:
		break;
	}

	if (nRetval != 0)
	{
		//PostMessage(hWnd, WM_SOCKET, (WPARAM)m_sock, MAKELPARAM(FD_READ, 0));
		return;
	}
	nHead = -1;
	m_bRecvHead = false;
	m_bRecvDelayed = false;
	memset(m_pCurrentBuffer, 0, BUFSIZE);
	PostMessage(hWnd, WM_SOCKET, (WPARAM)m_sock, MAKELPARAM(FD_WRITE, 0));

	return;
}

void CTcpClient::OnProcessingWriteMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	size_t nBufferSize = sizeof(int);
	int nHead;
	int nRetval;
	if (m_nMainClientID == -1  || m_bRecvDelayed == true)	// ���� ID�� �Ѱ� ���� ���߰ų� ������ �Ǿ���.
	{
		return;
	}

	switch (m_socketState)
	{
	case SOCKET_STATE::SEND_KEY_BUFFER:
	{
		nHead = 0;
		UCHAR keysBuffer[256];
		nBufferSize += sizeof(keysBuffer);
		UCHAR* pKeysBuffer = CGameFramework::GetKeysBuffer();
		if (pKeysBuffer != nullptr)
		{
			memcpy(keysBuffer, pKeysBuffer, nBufferSize - sizeof(int));
		}

		nBufferSize += sizeof(XMFLOAT3) * 2;		
		// �ӽ÷� Ű���ۿ� LOOK, UP, RIGHT ���� �����ֱ�
		nRetval = SendData(wParam, nBufferSize, nHead, keysBuffer, m_apPlayers[m_nMainClientID]->GetLook(), m_apPlayers[m_nMainClientID]->GetRight());
		if (nRetval == SOCKET_ERROR)
		{
			err_display("send()");
		}
	}
		break;
	default:
		break;
	}
}

template<class... Args>
void CTcpClient::CreateSendDataBuffer(char* pBuffer, Args&&... args)
{
	size_t nOffset = 0;
	((memcpy(pBuffer + nOffset, &args, sizeof(args)), nOffset += sizeof(args)), ...);
}

template<class... Args>
int CTcpClient::SendData(SOCKET socket, size_t nBufferSize, Args&&... args)
{
	int nRetval;
	char* pBuffer = new char[nBufferSize];
	(CreateSendDataBuffer(pBuffer, args...));

	SendNum++;
	nRetval = send(socket, (char*)pBuffer, nBufferSize, 0);
	delete[] pBuffer;

	if (nRetval == SOCKET_ERROR)
	{
		err_display("send()");
		return -1;
	}
	return 0;
}

int CTcpClient::RecvData(size_t nBufferSize)
{
	int nRetval;
	int nRemainRecvByte = nBufferSize - m_nCurrentRecvByte;

	RecvNum++;
	nRetval = recv(m_sock, (char*)&m_pCurrentBuffer + m_nCurrentRecvByte, nRemainRecvByte, 0);
	if(nRetval > 0) m_nCurrentRecvByte += nRetval;
	if (nRetval == SOCKET_ERROR || nRetval == 0) // error
	{
		return -1;
	}
	else if (nRetval < nBufferSize)
	{
		m_bRecvDelayed = true;
		return 1;
	}
	else
	{
		m_nCurrentRecvByte = 0;
		m_bRecvDelayed = false;
		return 0;
	}
}