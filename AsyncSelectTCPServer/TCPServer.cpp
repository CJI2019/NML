#include "TCPServer.h"

TCPServer::TCPServer()
{
}

TCPServer::~TCPServer()
{
}

int TCPServer::AcceptProcess(HWND hWnd, SOCKET& client_sock,int addrlen, sockaddr_in& clientaddr, WPARAM wParam)
{
	addrlen = sizeof(clientaddr);
	client_sock = accept(wParam, (struct sockaddr*)&clientaddr, &addrlen);

	if (client_sock == INVALID_SOCKET) {
		err_display("accept()");
		return -1;
	}
	printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	AddSocketInfo(client_sock);
	int retval = WSAAsyncSelect(client_sock, hWnd,
		WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
	if (retval == SOCKET_ERROR) {
		err_display("WSAAsyncSelect()");
		RemoveSocketInfo(client_sock);
	}

	return 1;
}

int TCPServer::ReadProcess(HWND hWnd, int addrlen, sockaddr_in& clientaddr, WPARAM wParam)
{
	SOCKETINFO* ptr = GetSocketInfo(wParam);

	if (ptr->recvbytes > 0) {
		ptr->recvdelayed = true;
		return -1;
	}
	// ������ �ޱ�
	int retval = recv(ptr->sock, ptr->buf, BUFSIZE, 0);
	if (retval == SOCKET_ERROR) {
		err_display("recv()");
		RemoveSocketInfo(wParam);
		return -1;
	}
	ptr->recvbytes = retval;
	// ���� ������ ���
	ptr->buf[retval] = '\0';
	addrlen = sizeof(clientaddr);
	getpeername(wParam, (struct sockaddr*)&clientaddr, &addrlen);
	printf("[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port), ptr->buf);

	return 1;
}

int TCPServer::WriteProcess(HWND hWnd, int addrlen, sockaddr_in& clientaddr, WPARAM wParam)
{
	int count = socketInfoList.size();

	SOCKETINFO* ptr = GetSocketInfo(wParam);
	if (ptr->recvbytes <= ptr->sendbytes)
		return -1;

	ScenePackageData();
	printf("%d���� Ŭ�� ������...\n", count);

	int retval = send(ptr->sock, (char*)&count, sizeof(int), 0);

	if (retval == SOCKET_ERROR) {
		err_display("send()");
		RemoveSocketInfo(ptr->sock);
		return -1;
	}
	retval = send(ptr->sock, (char*)packedData.data(), packedData.size(), 0);

	if (retval == SOCKET_ERROR) {
		err_display("send()");
		RemoveSocketInfo(ptr->sock);
		return -1;
	}

	ptr->sendbytes += retval;
	// ���� �����͸� ��� ���´��� üũ
	if (ptr->sendbytes != 0) {
		ptr->recvbytes = ptr->sendbytes = 0;
		if (ptr->recvdelayed) {
			ptr->recvdelayed = false;
			PostMessage(hWnd, WM_SOCKET, wParam, FD_READ);
		}
	}

	return 1;
}

bool TCPServer::Init(HWND hWnd)
{
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;

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
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// WSAAsyncSelect()
	retval = WSAAsyncSelect(listen_sock, hWnd, WM_SOCKET, FD_ACCEPT | FD_CLOSE);
	if (retval == SOCKET_ERROR) err_quit("WSAAsyncSelect()");

	return true;
}


void TCPServer::ScenePackageData()
{
	packedData.clear();

	for (auto& info : socketInfoList) {
		InsertData(&info.client_info.Id, sizeof(int));
		InsertData(&info.client_info.x, sizeof(float));
		InsertData(&info.client_info.y, sizeof(float));
		InsertData(&info.client_info.z, sizeof(float));
	}
}

void TCPServer::InsertData(void* data, size_t size)
{
	for (int i = 0;i < size;++i) {
		packedData.push_back(((BYTE*)data)[i]);
	}
}


bool TCPServer::SendData(SOCKET& sock, char* data, int size)
{
	int retval = send(sock, data, size, 0);

	if (retval == SOCKET_ERROR) {
		err_display("send()");
		RemoveSocketInfo(sock);
		return false;
	}

	return true;
}

bool TCPServer::RecvData(SOCKET& sock, char* data, int size)
{
	int retval = recv(sock, data, BUFSIZE, 0);

	if (retval == SOCKET_ERROR) {
		err_display("recv()");
		RemoveSocketInfo(sock);
		return false;
	}

	return true;
}


// ���� ���� �߰�
bool TCPServer::AddSocketInfo(SOCKET sock)
{
	SOCKETINFO* ptr = new SOCKETINFO;
	if (ptr == NULL) {
		printf("[����] �޸𸮰� �����մϴ�!\n");
		return false;
	}
	ptr->sock = sock;
	SocketInfoInit(ptr);

	//Ŭ���̾�Ʈ ���� ������ ���н� �������� ����
	// 1. ���ӵǾ��ִ� Ŭ���̾�Ʈ ����
	int count = socketInfoList.size();
	SendData(ptr->sock, (char*)&count, sizeof(int));
	// 2. ���� �߰��� Ŭ���̾�Ʈ ����
	for (auto& info : socketInfoList) {
		SendData(ptr->sock, (char*)&info.client_info, sizeof(Client_INFO));
	}
	// 3. ���� �߰��� Ŭ���̾�Ʈ ID 
	SendData(ptr->sock, (char*)&ptr->client_info.Id, sizeof(int));

	return true;
}


void TCPServer::SocketInfoInit(SOCKETINFO* sockinfo)
{
	sockinfo->recvbytes = 0;
	sockinfo->sendbytes = 0;
	sockinfo->recvdelayed = false;

	// Ŭ���̾�Ʈ ���� �ʱ�ȭ
	static int count_Id = 0;

	sockinfo->client_info.Id = count_Id++;
	sockinfo->client_info.x = -320.0f + (count_Id * 10.0f);
	sockinfo->client_info.y = 0.0f;
	sockinfo->client_info.z = 500.0f;

	// ����Ʈ�� ���� �߰�
	socketInfoList.push_back(*sockinfo);
}

// ���� ���� ���
SOCKETINFO* TCPServer::GetSocketInfo(SOCKET sock)
{
	for (auto& sockinfo : socketInfoList) {
		if (sockinfo.sock == sock) {
			return &sockinfo;
		}
	}
	return nullptr;

}

// ���� ���� ����
void TCPServer::RemoveSocketInfo(SOCKET sock)
{
	// Ŭ���̾�Ʈ ���� ���
	struct sockaddr_in clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(sock, (struct sockaddr*)&clientaddr, &addrlen);
	printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	// ����Ʈ���� ���� ����
	for (auto iter = socketInfoList.begin(); iter != socketInfoList.end();++iter) {
		if (iter->sock == sock) {
			closesocket(iter->sock); // ���� �ݱ�
			socketInfoList.erase(iter);
			return;
		}
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
