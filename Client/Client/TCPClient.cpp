#include "stdafx.h"
#include "TCPClient.h"
#include "GameFramework.h"

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

/*
������ ������ �Ǹ� ���� Ŭ���̾�Ʈ �÷��̾ �������� ���� �� 
�������� Ŭ��ID, ��ǥ(float(x,y,z)) ������ Ŭ��� �ѱ�.
*/

void CTcpClient::OnDestroy()
{

}

XMFLOAT3 CTcpClient::GetPostion(int id)
{
	XMFLOAT3 position = { 0.0f,0.0f, 0.0f };

	return position;
}

unordered_map<int, Client_INFO>& CTcpClient::GetUMapClientInfo()
{
	return m_umapClientInfo;
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
	int nRetval;
	if (m_nMainClientID == -1)
	{
		nRetval = recv(m_sock, (char*)&m_pCurrentBuffer + m_nCurrentRecvByte, sizeof(int) - m_nCurrentRecvByte, 0);
		m_nCurrentRecvByte += nRetval;
		if (nRetval == SOCKET_ERROR || nRetval == 0)
		{
			// error
		}
		else if (nRetval < sizeof(int))
		{
			m_bRecvDelayed = true;
			return;
		}
		else
		{
			m_socketState = SOCKET_STATE::SEND_KEY_BUFFER;
			memcpy(&m_nMainClientID, m_pCurrentBuffer, sizeof(int));
			//memcpy(&m_nClient, m_pCurrentBuffer + sizeof(int), sizeof(int));

			m_nCurrentRecvByte = 0;
			memset(m_pCurrentBuffer, 0, BUFSIZE);
			// ������ �Ǿ����ٸ� �ٽ� �޽����� ��������. -> ���⼭�� �׷��ʿ䰡����
			if (m_bSendDelayed || m_bRecvDelayed)
			{
				m_bSendDelayed = false;
				m_bRecvDelayed = false;
				PostMessage(hWnd, WM_SOCKET, (WPARAM)m_sock, MAKELPARAM(FD_WRITE, 0));
			}
			return;
		}
	}

	if (m_socketState == SOCKET_STATE::RECV_SERVER_ACK)
	{
		nRetval = recv(m_sock, (char*)&m_pCurrentBuffer + m_nCurrentRecvByte, sizeof(int) - m_nCurrentRecvByte, 0);
		m_nCurrentRecvByte += nRetval;
		if (nRetval == SOCKET_ERROR || nRetval == 0)
		{
			// error
		}
		else if (nRetval < sizeof(int))
		{
			m_bRecvDelayed = true;
			return;
		}
		else
		{
			int nComplete = -1;
			m_socketState = SOCKET_STATE::SEND_KEY_BUFFER;
			memcpy(&nComplete, m_pCurrentBuffer, sizeof(int));
			if (nComplete != 0)
			{
				//error
				exit(-1);
			}

			m_nCurrentRecvByte = 0;
			memset(m_pCurrentBuffer, 0, BUFSIZE);
			if (m_bSendDelayed || m_bRecvDelayed)
			{
				m_bSendDelayed = false;
				m_bRecvDelayed = false;
			}
			PostMessage(hWnd, WM_SOCKET, (WPARAM)m_sock, MAKELPARAM(FD_WRITE, 0));

			return;
		}
	}
	return;
}

void CTcpClient::OnProcessingWriteMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	int nRetval;
	if (m_nMainClientID == -1)	// ���� ID�� �Ѱ� ���� ���ߴ�.
	{
		m_bSendDelayed = true;
		return;
	}

	if (m_bRecvDelayed == true)	// recv�� �����̵Ǿ���
	{
		m_bSendDelayed = true;
		return;
	}
	
	switch (m_socketState)
	{
	case SOCKET_STATE::SEND_KEY_BUFFER:
	{
		UCHAR keysBuffer[257];
		int nBufferSize = sizeof(keysBuffer);
		UCHAR* pKeysBuffer = CGameFramework::GetKeysBuffer();
		if (pKeysBuffer != nullptr)
		{
			memcpy(keysBuffer, pKeysBuffer, 256);
		}
		keysBuffer[256] = '\0';
		unsigned long bytesInBuffer;
		int result = ioctlsocket(m_sock, FIONREAD, &bytesInBuffer);
		nRetval = send(wParam, (char*)keysBuffer, nBufferSize, 0);

		if (nRetval == SOCKET_ERROR)
		{
			err_display("send()");
		}
		m_socketState = SOCKET_STATE::RECV_SERVER_ACK;
	}
		break;
	default:
		m_bSendDelayed = true;
		break;
	}
}
