#include "stdafx.h"
#include "TCPServer.h"
#include "ServerObject.h"
#include "ServerEnvironmentObject.h"
#include "ServerPlayer.h"
#include "ServerCollision.h"

size_t TCPServer::m_nClient = 0;

TCPServer::TCPServer()
{
}

TCPServer::~TCPServer()
{
}

void TCPServer::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_CREATE:
		m_timer.Start();
		break;
	default:
		break;
	}
}

void TCPServer::OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_ACCEPT:
		OnProcessingAcceptMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case FD_READ:
		OnProcessingReadMessage(hWnd, nMessageID, wParam, lParam);
	case FD_WRITE:
		OnProcessingWriteMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case FD_CLOSE:
		OnProcessingCloseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	default:
		break;
	}

	return;
}

void TCPServer::OnProcessingAcceptMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	// �ִ� Ŭ���̾�Ʈ ���� �ѱ�� �����ϴ�.
	if (m_nClient >= MAX_CLIENT)
	{
		return;
	}

	SOCKET sockClient;
	struct sockaddr_in addrClient;
	int nAddrlen = sizeof(sockaddr_in);
	sockClient = accept(wParam, (struct sockaddr*)&addrClient, &nAddrlen);

	if (sockClient == INVALID_SOCKET)
	{
		err_display("accept()");
		return;
	}

	// �߰��� Ŭ���̾�Ʈ�� ������ �߰��Ѵ�.
	int nClientIndex = AddSocketInfo(sockClient, addrClient, nAddrlen);
	printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", m_vSocketInfoList[nClientIndex].m_pAddr, ntohs(m_vSocketInfoList[nClientIndex].m_addrClient.sin_port));

	int retval = WSAAsyncSelect(sockClient, hWnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
	if (retval == SOCKET_ERROR)
	{
		err_display("WSAAsyncSelect()");
		RemoveSocketInfo(sockClient);
	}

	// �ӽ÷� CBlueSuitPlayer�� ����
	m_apPlayers[nClientIndex] = make_shared<CServerBlueSuitPlayer>();
	m_apPlayers[nClientIndex]->SetPlayerId(nClientIndex);
	m_pCollisionManager->AddCollisionPlayer(m_apPlayers[nClientIndex], nClientIndex);

	for (auto& sockInfo : m_vSocketInfoList)
	{
		if (!sockInfo.m_bUsed || sockInfo.m_sock == sockClient)
		{
			continue;
		}
		sockInfo.m_prevSocketState = sockInfo.m_socketState;
		sockInfo.m_socketState = SOCKET_STATE::SEND_NUM_OF_CLIENT;
	}

	return;
}

void TCPServer::OnProcessingReadMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	int nRetval = 1;
	size_t nBufferSize;
	int nSocketIndex = GetSocketIndex(wParam);
	if (!m_vSocketInfoList[nSocketIndex].m_bUsed)
	{
		//error
		exit(-1);
	}
	std::shared_ptr<CServerPlayer> pPlayer = m_apPlayers[nSocketIndex];

	if(!m_vSocketInfoList[nSocketIndex].m_bRecvHead)
	{
		nBufferSize = sizeof(int);

		nRetval = RecvData(nSocketIndex, nBufferSize);
		if (nRetval == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				m_vSocketInfoList[nSocketIndex].m_bRecvHead = false;
				m_vSocketInfoList[nSocketIndex].m_nHead = -1;
				memset(m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer, 0, BUFSIZE);
			}
			return;
		}
		m_vSocketInfoList[nSocketIndex].m_bRecvHead = true;
		memcpy(&m_vSocketInfoList[nSocketIndex].m_nHead, m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer, sizeof(int));
		memset(m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer, 0, BUFSIZE);
	}

	switch (m_vSocketInfoList[nSocketIndex].m_nHead)
	{
	case HEAD_KEYS_BUFFER:
	{
		if (!pPlayer->IsRecvData())
		{
			pPlayer->SetRecvData(true);
		}
		 
		std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
		std::chrono::time_point<std::chrono::steady_clock> client;

		// Time, KeysBuffer, viewMatrix, vecLook, vecRight
		nBufferSize = sizeof(__int64) + sizeof(UCHAR[256]) + sizeof(XMFLOAT4X4) + sizeof(XMFLOAT3) * 2;
		m_vSocketInfoList[nSocketIndex].RecvNum++;
		nRetval = RecvData(nSocketIndex, nBufferSize);
		if (nRetval == SOCKET_ERROR)
		{
			break;
		}

		memcpy(&client, m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer, sizeof(std::chrono::time_point<std::chrono::steady_clock>));
		std::chrono::duration<double> deltaTime = now - client;
		//printf("dif: %lf ms\n", deltaTime.count() * 1000.0f);

		memcpy(pPlayer->GetKeysBuffer(), m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer + sizeof(__int64), sizeof(UCHAR[256]));

		XMFLOAT4X4 xmf4x4View;
		memcpy(&xmf4x4View, m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer + sizeof(__int64) + sizeof(UCHAR[256]), sizeof(XMFLOAT4X4));
		pPlayer->SetViewMatrix(xmf4x4View);

		XMFLOAT3 xmf3Look, xmf3Right;
		memcpy(&xmf3Look, m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer + sizeof(__int64) + sizeof(UCHAR[256]) + sizeof(XMFLOAT4X4), sizeof(XMFLOAT3));
		memcpy(&xmf3Right, m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer + sizeof(__int64) + sizeof(UCHAR[256]) + sizeof(XMFLOAT4X4) + sizeof(XMFLOAT3), sizeof(XMFLOAT3));
		pPlayer->SetLook(xmf3Look);
		pPlayer->SetRight(xmf3Right);

	}
		break;
	default:
		break;
	}

	if (nRetval == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK)
		{
			m_vSocketInfoList[nSocketIndex].m_bRecvHead = true;
			memset(m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer, 0, BUFSIZE);
		}
		return;
	}
	m_vSocketInfoList[nSocketIndex].m_nHead = -1;
	m_vSocketInfoList[nSocketIndex].m_bRecvHead = false;
	m_vSocketInfoList[nSocketIndex].m_bRecvDelayed = false;
	memset(m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer, 0, BUFSIZE);
	return;
}

void TCPServer::OnProcessingWriteMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	size_t nBufferSize = sizeof(int);
	int nHead;
	int nRetval;
	int nSocketIndex = GetSocketIndex(wParam);
	if (!m_vSocketInfoList[nSocketIndex].m_bUsed)
	{
		//error
	}
	std::shared_ptr<CServerPlayer> pPlayer = m_apPlayers[nSocketIndex];

	switch (m_vSocketInfoList[nSocketIndex].m_socketState)
	{
	case SOCKET_STATE::SEND_ID:
		nHead = 0;
		nBufferSize += sizeof(int) * 2;

		m_vSocketInfoList[nSocketIndex].SendNum++;
		nRetval = SendData(m_vSocketInfoList[nSocketIndex].m_sock, nBufferSize, nHead, m_aUpdateInfo[nSocketIndex].m_nClientId, (int)m_nClient);

		if (nRetval == -1 && WSAGetLastError() == WSAEWOULDBLOCK)
		{
		}
		m_vSocketInfoList[nSocketIndex].m_socketState = SOCKET_STATE::SEND_UPDATE_DATA;

		break;
	case SOCKET_STATE::SEND_UPDATE_DATA:
	{
		nHead = 1;
		nBufferSize += sizeof(m_aUpdateInfo);

		m_vSocketInfoList[nSocketIndex].SendNum++;
		nRetval = SendData(m_vSocketInfoList[nSocketIndex].m_sock, nBufferSize, nHead, m_aUpdateInfo);
		if (nRetval == -1 && WSAGetLastError() == WSAEWOULDBLOCK)
		{
		}
	}
		break;
	case SOCKET_STATE::SEND_NUM_OF_CLIENT:
		nHead = 2;
		nBufferSize += sizeof(int) + sizeof(m_aUpdateInfo);

		nRetval = SendData(m_vSocketInfoList[nSocketIndex].m_sock, nBufferSize, nHead, (int)m_nClient, m_aUpdateInfo);
		if (nRetval == -1 && WSAGetLastError() == WSAEWOULDBLOCK)
		{
		}
		m_vSocketInfoList[nSocketIndex].m_socketState = m_vSocketInfoList[nSocketIndex].m_prevSocketState;

		break;
	default:
		break;
	}
	return;
}

void TCPServer::OnProcessingCloseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	int nIndex = RemoveSocketInfo((SOCKET)wParam);
	m_apPlayers[nIndex]->SetPlayerId(-1);
}

bool TCPServer::Init(HWND hWnd)
{
	m_hWnd = hWnd;
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		return false;
	}

	// ���� ����
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	int retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		err_quit("listen()");
	}

	// WSAAsyncSelect()
	retval = WSAAsyncSelect(listen_sock, hWnd, WM_SOCKET, FD_ACCEPT | FD_CLOSE);
	if (retval == SOCKET_ERROR)
	{
		err_quit("WSAAsyncSelect()");
	}

	m_pCollisionManager = make_shared<CServerCollisionManager>();
	m_pCollisionManager->CreateCollision(4, 10, 10);

	LoadScene();
	return true;
}

void TCPServer::SimulationLoop()
{
	m_timer.Tick();
	// ���� �ùķ��̼��� �Ͼ��
	float fElapsedTime = m_timer.GetTimeElapsed();
	
	for (auto& pPlayer : m_apPlayers)
	{
		if (!pPlayer || pPlayer->GetPlayerId() == -1)
		{
			continue;
		}

		pPlayer->SetPickedObject(m_pCollisionManager);	

		pPlayer->Update(fElapsedTime);
		pPlayer->UpdatePicking();
		//UpdateInformation(pPlayer);
		m_pCollisionManager->Collide(fElapsedTime, pPlayer);

	}

	m_pCollisionManager->Update(fElapsedTime);

	UpdateInformation();
	CreatSendObject();
}

// ���� ���� �߰�
int TCPServer::AddSocketInfo(SOCKET sockClient, struct sockaddr_in addrClient, int nAddrLen)
{
	int nSocketIndex = -1;
	if (m_nClient >= MAX_CLIENT)
	{
		return nSocketIndex;
	}
	SOCKETINFO sockInfo;

	sockInfo.m_bUsed = true;
	sockInfo.m_sock = sockClient;
	sockInfo.m_addrClient = addrClient;
	sockInfo.m_nAddrlen = nAddrLen;

	getpeername(sockInfo.m_sock, (struct sockaddr*)&sockInfo.m_addrClient, &sockInfo.m_nAddrlen);
	inet_ntop(AF_INET, &sockInfo.m_addrClient.sin_addr, sockInfo.m_pAddr, sizeof(sockInfo.m_pAddr));

	sockInfo.m_nCurrentRecvByte = 0;
	sockInfo.m_bRecvDelayed = false;
	sockInfo.m_bRecvHead = false;
	sockInfo.m_socketState = SOCKET_STATE::SEND_ID;
	sockInfo.m_prevSocketState = SOCKET_STATE::SEND_ID;
	
	// �迭�� ���� �߰� 
	for (int i = 0; i < m_nClient + 1; ++i)
	{
		if (m_vSocketInfoList[i].m_bUsed)
		{
			continue;
		}
		m_nClient++;

		// Ŭ���̾�Ʈ ���� �ʱ�ȭ
		m_aUpdateInfo[i].m_nClientId = i;
		m_vSocketInfoList[i] = sockInfo;
		nSocketIndex = i;
		break;
	}

	return nSocketIndex;
}

// ���� ���� ���
int TCPServer::GetSocketIndex(SOCKET sock)
{
	int nIndex = -1;
	for (auto& sockInfo : m_vSocketInfoList)
	{
		nIndex++;
		if (!sockInfo.m_bUsed)
		{
			continue;
		}

		if (sockInfo.m_sock == sock)
		{
			return nIndex;
		}
	}
	return nIndex;
}

// ���� ���� ����
int TCPServer::RemoveSocketInfo(SOCKET sock)
{
	int nIndex = -1;
	// ����Ʈ���� ���� ����
	for (auto& sockInfo : m_vSocketInfoList)
	{
		nIndex++;
		if (!sockInfo.m_bUsed)
		{
			continue;
		}

		if (sockInfo.m_sock == sock)
		{
			printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", sockInfo.m_pAddr, ntohs(sockInfo.m_addrClient.sin_port));
			closesocket(sockInfo.m_sock); // ���� �ݱ�
			sockInfo.m_bUsed = false;
			
			m_aUpdateInfo[nIndex].m_nClientId = -1;
			m_nClient--;
			for(auto& otherSocketInfo : m_vSocketInfoList)
			{
				if (!otherSocketInfo.m_bUsed)
				{
					continue;
				}

				otherSocketInfo.m_prevSocketState = otherSocketInfo.m_socketState;
				otherSocketInfo.m_socketState = SOCKET_STATE::SEND_NUM_OF_CLIENT;
			}

			return nIndex;
		}
	}
	return -1;
}

void TCPServer::UpdateInformation()
{
	for (const auto& pPlayer : m_apPlayers)
	{
		int nPlayerId;
		if (!pPlayer || pPlayer->GetPlayerId() == -1)
		{
			continue;
		}
		nPlayerId = pPlayer->GetPlayerId();

		m_aUpdateInfo[nPlayerId].m_xmf3Position = pPlayer->GetPosition();
		m_aUpdateInfo[nPlayerId].m_xmf3Velocity = pPlayer->GetVelocity();
		m_aUpdateInfo[nPlayerId].m_xmf3Look = pPlayer->GetLook();
		m_aUpdateInfo[nPlayerId].m_xmf3Right = pPlayer->GetRight();

		// ������Ʈ ������Ʈ�� ����
		m_aUpdateInfo[nPlayerId].m_nNumOfObject = 0;
		for (int i = 0; i < 20; ++i)
		{
			m_aUpdateInfo[nPlayerId].m_anObjectNum[i] = -1;
		}		
	}
}

void TCPServer::LoadScene()
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, (char*)"ServerScene.bin", "rb");
	::rewind(pInFile);
	int fileEnd{};
	int nReads;
	while (true)
	{
		char pstrToken[128] = { '\0' };
		for (; ; )
		{
			if (::ReadStringFromFile(pInFile, pstrToken))
			{
				if (!strcmp(pstrToken, "<Hierarchy>:"))
				{
					char pStrFrameName[64];
					int nChild, nBoxCollider;
					XMFLOAT3 xmf3AABBCenter, xmf3AABBExtents;
					std::vector<BoundingOrientedBox> voobb;
					for (;;)
					{
						if (::ReadStringFromFile(pInFile, pstrToken))
						{
							if (!strcmp(pstrToken, "<Frame>:"))
							{
								::ReadIntegerFromFile(pInFile);
								::ReadStringFromFile(pInFile, pStrFrameName);
							}
							else if (!strcmp(pstrToken, "<Children>:"))
							{
								nChild = ::ReadIntegerFromFile(pInFile);
							}
							else if (!strcmp(pstrToken, "<BoxColliders>:"))
							{
								nBoxCollider = ::ReadIntegerFromFile(pInFile);
								voobb.reserve(nBoxCollider);
								for (int i = 0; i < nBoxCollider; ++i)
								{
									::ReadStringFromFile(pInFile, pstrToken);	// <Bound>
									int nIndex = 0;
									nReads = fread(&nIndex, sizeof(int), 1, pInFile);
									nReads = (UINT)::fread(&xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
									nReads = (UINT)::fread(&xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
									XMFLOAT4 xmf4Orientation;
									XMStoreFloat4(&xmf4Orientation, XMQuaternionIdentity());
									voobb.emplace_back(xmf3AABBCenter, xmf3AABBExtents, xmf4Orientation);
								}
							}
							else if (!strcmp(pstrToken, "<Matrix>:"))
							{
								nChild = ::ReadIntegerFromFile(pInFile);
								XMFLOAT4X4* xmf4x4World = new XMFLOAT4X4[nChild];
								nReads = (UINT)::fread(xmf4x4World, sizeof(XMFLOAT4X4), nChild, pInFile);
								for (int i = 0; i < nChild; ++i)
								{
									// ������Ʈ ����
									CreateSceneObject(pStrFrameName, Matrix4x4::Transpose(xmf4x4World[i]), voobb);
								}
								delete[] xmf4x4World;
							}
							else if (!strcmp(pstrToken, "</Frame>"))
							{
								break;
							}
						}
					}
				}
				else if (!strcmp(pstrToken, "</Hierarchy>"))
				{
					break;
				}
				else if (!strcmp(pstrToken, "</Scene>:"))
				{
					fileEnd = 1;
					break;
				}
			}
			else
			{
				break;
			}
		}
		if (fileEnd) 
		{
			break;
		}
	}
}

void TCPServer::CreateSceneObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb)
{
	shared_ptr<CServerGameObject> pGameObject;

	if (!strcmp(pstrFrameName, "Door_1"))
	{
		pGameObject = make_shared<CDoorObject>(pstrFrameName, xmf4x4World, voobb);
		m_pCollisionManager->AddCollisionObject(pGameObject);
	}
	else if (!strcmp(pstrFrameName, "Drawer_1") || !strcmp(pstrFrameName, "Drawer_2"))
	{
		pGameObject = make_shared<CDrawerObject>(pstrFrameName, xmf4x4World, voobb);
		m_pCollisionManager->AddCollisionObject(pGameObject);
	}
	else if (!strcmp(pstrFrameName, "Door1"))
	{
		pGameObject = make_shared<CElevatorDoorObject>(pstrFrameName, xmf4x4World, voobb);
		m_pCollisionManager->AddCollisionObject(pGameObject);
	}
	else if (!strcmp(pstrFrameName, "Emergency_Handle"))
	{
		pGameObject = make_shared<CElevatorDoorObject>(pstrFrameName, xmf4x4World, voobb);
		m_pCollisionManager->AddCollisionObject(pGameObject);
	}
	else if (!strcmp(pstrFrameName, "Laboratory_Wall_1_Corner_1") || !strcmp(pstrFrameName, "Laboratory_Wall_1_Corner_2"))
	{
		pGameObject = make_shared<CEnvironmentObject>(pstrFrameName, xmf4x4World, voobb);
		m_pCollisionManager->AddCollisionObject(pGameObject);
	}
	else if (!strcmp(pstrFrameName, "Laboratory_Wall_1_Corner") || !strcmp(pstrFrameName, "Laboratory_Wall_1_Corner2") || !strcmp(pstrFrameName, "Laboratory_Wall_1"))
	{
		pGameObject = make_shared<CEnvironmentObject>(pstrFrameName, xmf4x4World, voobb);
		m_pCollisionManager->AddCollisionObject(pGameObject);
	}
	else if (!strcmp(pstrFrameName, "Laboratory_Wall_Door_1") || !strcmp(pstrFrameName, "Laboratory_Wall_Door_1_2") 
			|| !strcmp(pstrFrameName, "Laboratory_Tunnel_1") || !strcmp(pstrFrameName, "Laboratory_Table_1"))
	{
		pGameObject = make_shared<CEnvironmentObject>(pstrFrameName, xmf4x4World, voobb);
		m_pCollisionManager->AddCollisionObject(pGameObject);
	}
	else if (!strcmp(pstrFrameName, "Laboratory_Tunnel_1_Stairs"))
	{
		pGameObject = make_shared<CEnvironmentObject>(pstrFrameName, xmf4x4World, voobb);
		m_pCollisionManager->AddCollisionObject(pGameObject);
	}
	else if (!strcmp(pstrFrameName, "BoxCollider_Stair_Start"))
	{
		pGameObject = make_shared<CStairTriggerObject>(pstrFrameName, xmf4x4World, voobb);
		m_pCollisionManager->AddCollisionObject(pGameObject);
	}
	else
	{
		pGameObject = make_shared<CServerGameObject>(pstrFrameName, xmf4x4World, voobb);
		m_pCollisionManager->AddCollisionObject(pGameObject);
	}
}

void TCPServer::CreatSendObject()
{
	int nIndex = 0;
	for (const auto& pPlayer : m_apPlayers)
	{
		if (!pPlayer || pPlayer->GetPlayerId() == -1)
		{
			continue;
		}

		int nId = pPlayer->GetPlayerId();

		// ���� ���߿� ����ʿ����� �߰��Ҽ��ֵ����ؾ��ҵ�
		// if(��� ���̸� ���� or �Ʒ������� �˻�)

		for (int j = pPlayer->GetWidth() - 1; j <= pPlayer->GetWidth() + 1 && nIndex < MAX_SEND_OBJECT_INFO; ++j)
		{
			if (j < 0 || j > m_pCollisionManager->GetWidth() - 1)
			{
				continue;
			}

			for (int k = pPlayer->GetDepth() - 1; k <= pPlayer->GetDepth() + 1 && nIndex < MAX_SEND_OBJECT_INFO; ++k)
			{
				if (k < 0 || k > m_pCollisionManager->GetDepth() - 1)
				{
					continue;
				}

				for (const auto& pGameObject : m_pCollisionManager->GetSpaceGameObjects(pPlayer->GetFloor(), j, k))
				{
					if (!pGameObject)
					{
						continue;
					}

					m_aUpdateInfo[nId].m_anObjectNum[nIndex] = pGameObject->GetCollisionNum();
					m_aUpdateInfo[nId].m_axmf4x4World[nIndex] = pGameObject->GetWorldMatrix();

					nIndex++;
					if (nIndex == MAX_SEND_OBJECT_INFO)
					{
						break;
					}
				}
				m_aUpdateInfo[nId].m_nNumOfObject = nIndex;
			}
		}
	}
}

template<class... Args>
void TCPServer::CreateSendDataBuffer(char* pBuffer, Args&&... args)
{
	size_t nOffset = 0;
	((memcpy(pBuffer + nOffset, &args, sizeof(args)), nOffset += sizeof(args)), ...);
}

// ������ �����͸� ��� ������ ���
template<class... Args>
int TCPServer::SendData(SOCKET socket, size_t nBufferSize, Args&&... args)
{
	int nRetval;
	char* pBuffer = new char[nBufferSize];
	(CreateSendDataBuffer(pBuffer, args...));

	nRetval = send(socket, (char*)pBuffer, nBufferSize, 0);
	delete[] pBuffer;
	
	if (nRetval == SOCKET_ERROR)
	{
		err_display("send()");
		return SOCKET_ERROR;
	}
	return 0;
}

int TCPServer::RecvData(int nSocketIndex, size_t nBufferSize)
{
	int nRetval;
	int nRemainRecvByte = nBufferSize - m_vSocketInfoList[nSocketIndex].m_nCurrentRecvByte;

	
	nRetval = recv(m_vSocketInfoList[nSocketIndex].m_sock, (char*)&m_vSocketInfoList[nSocketIndex].m_pCurrentBuffer + m_vSocketInfoList[nSocketIndex].m_nCurrentRecvByte, nRemainRecvByte, 0);
	
	if (nRetval > 0)m_vSocketInfoList[nSocketIndex].m_nCurrentRecvByte += nRetval;
	if (nRetval == SOCKET_ERROR || nRetval == 0) // error
	{
		return -1;
	}
	else if (m_vSocketInfoList[nSocketIndex].m_nCurrentRecvByte < nBufferSize)
	{
		m_vSocketInfoList[nSocketIndex].m_bRecvDelayed = true;
		return 1;
	}
	else
	{
		m_vSocketInfoList[nSocketIndex].m_nCurrentRecvByte = 0;
		m_vSocketInfoList[nSocketIndex].m_bRecvDelayed = false;
		return 0;
	}
}

// ���� �Լ� ���� ��� �� ����
void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	MessageBoxA(NULL, (const char*)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}
// ���� �Լ� ���� ���
void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[%s] %s\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
// ���� �Լ� ���� ���
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[����] %s\n", (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}