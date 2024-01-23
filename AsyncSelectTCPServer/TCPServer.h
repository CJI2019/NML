#pragma once
#include "../Common.h"
// ���� ���� ������ ���� ����ü�� ����
struct Client_INFO {
	int Id;
	float x, y, z;
};

struct SOCKETINFO
{
	SOCKET sock;
	Client_INFO client_info;
	char buf[BUFSIZE + 1];
	int recvbytes;
	int sendbytes;
	bool recvdelayed;
	//SOCKETINFO *next;
};

class TCPServer
{
public:
	TCPServer();
	~TCPServer();

	int AcceptProcess(HWND hWnd, SOCKET& client_sock, int addrlen, sockaddr_in& clientaddr, WPARAM wParam);
	int ReadProcess(HWND hWnd, int addrlen, sockaddr_in& clientaddr, WPARAM wParam);
	int WriteProcess(HWND hWnd, int addrlen, sockaddr_in& clientaddr, WPARAM wParam);

	// ������ Ŭ���̾�Ʈ���� ������ ����.
	vector<SOCKETINFO> socketInfoList;
	// �������� �ùķ��̼ǵǴ� �����͸� �����ϰ� Ŭ���̾�Ʈ���� �����Ӹ��� ������ �������̴�.
	vector<BYTE> packedData;

	bool Init(HWND hWnd);
	// ������ �ۼ��� �Լ�
	bool SendData(SOCKET& sock, char* data, int size);
	bool RecvData(SOCKET& sock, char* data, int size);

	void InsertData(void* data, size_t size);
	void ScenePackageData();

	// ���� ���� ���� �Լ�
	bool AddSocketInfo(SOCKET sock);
	void SocketInfoInit(SOCKETINFO* socketinfo);
	SOCKETINFO* GetSocketInfo(SOCKET sock);
	void RemoveSocketInfo(SOCKET sock);
};

extern void err_quit(const char* msg);
extern void err_display(const char* msg);
extern void err_display(int errcode);