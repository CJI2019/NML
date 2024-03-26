#pragma once
#include "Timer.h"
constexpr size_t MAX_CLIENT{ 5 };

// ���� ���� ������ ���� ����ü�� ����
struct Client_INFO
{
	int m_nClientId;
	float x, y, z;
};

enum class SOCKET_STATE
{
	RECV_CLIENT_ACK = 0, // Ŭ�󿡼� �����͸� �޾Ҵٴ� ����
	RECV_KEY_BUFFER,
	SEND_ACK,			 // Ŭ���̾�Ʈ�� ������ ����
	SEND_ID,
	SEND_KEY_BUFFER,
};

struct SOCKETINFO
{
	bool m_bUsed = false;
	SOCKET m_sock;

	struct sockaddr_in m_addrClient;
	int m_nAddrlen;
	char m_pAddr[INET_ADDRSTRLEN];

	Client_INFO m_clientInfo;
	bool m_bRecvDelayed = false;	// ���� �����͸� ���� ���� ���ߴ�
	bool m_bSendDelayed = false;	// ���� �����͸� ���� ���� ���ߴ�
	int m_nCurrentRecvByte = 0;		// ������� ���� �������� ����
	char m_pCurrentBuffer[BUFSIZE + 1];

	SOCKET_STATE m_socketState = SOCKET_STATE::SEND_ID;
	SOCKET_STATE m_prevSocketState = SOCKET_STATE::SEND_ID;
};

class TCPServer
{
public:
	TCPServer();
	~TCPServer();

	// �̺�Ʈ�� ó���Ѵ�.
	void OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingAcceptMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingReadMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingWriteMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	bool Init(HWND hWnd);

	void SimulationLoop();

	// ���� ���� ���� �Լ�
	int AddSocketInfo(SOCKET sockClient, struct sockaddr_in addrClient, int nAddrLen);
	int GetSocketIndex(SOCKET sockClient);
	void RemoveSocketInfo(SOCKET sock);

	UCHAR* GetKeysBuffer() { return m_pKeysBuffer; }
private:
	CTimer m_timer;
	static size_t m_nClient;

	// ������ Ŭ���̾�Ʈ���� ������ ����.
	std::array<SOCKETINFO, MAX_CLIENT> m_vSocketInfoList;
	// �������� �ùķ��̼ǵǴ� �����͸� �����ϰ� Ŭ���̾�Ʈ���� �����Ӹ��� ������ �������̴�.
	vector<BYTE> packedData;

	// �ӽ� Ű����
	UCHAR m_pKeysBuffer[256];
};

extern void err_quit(const char* msg);
extern void err_display(const char* msg);
extern void err_display(int errcode);


