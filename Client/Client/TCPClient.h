#pragma once
constexpr UINT WM_SOCKET{ WM_USER + 1 };
constexpr char SERVERIP[16]{ "127.0.0.1" };
constexpr UINT SERVERPORT{ 9000 };
constexpr UINT BUFSIZE{ 500 };

enum class SOCKET_STATE
{
	RECV_SERVER_ACK = 0, // �������� �����͸� �޾Ҵٴ� ����
	RECV_ID,
	SEND_KEY_BUFFER,
};

struct Client_INFO
{
	int m_nClientId;
	float x, y, z;
};

class CTcpClient
{
private:
	int m_nMainClientID = -1;
	int m_nClient = -1;

	bool m_bRecvDelayed = false;	// ���� �����͸� ���� ���� ���ߴ�
	bool m_bSendDelayed = false;

	int m_nCurrentRecvByte = 0;		// ������� ���� �������� ����
	char m_pCurrentBuffer[BUFSIZE];

	SOCKET_STATE m_socketState = SOCKET_STATE::RECV_ID;
	SOCKET_STATE m_prevSocketState = SOCKET_STATE::RECV_ID;
	//SEND_STATE m_sendState = SEND_STATE::SEND_DATA_LENGTH;

	// ���� ���� ����
	SOCKET m_sock;
	// ������ ��ſ� ����� ����
	vector<BYTE> m_vbuffer;

	// ������ ������ Ŭ���̾�Ʈ�� ���� <���̵�,����>
	unordered_map<int, Client_INFO> m_umapClientInfo;
public:
	CTcpClient(HWND hWnd);
	~CTcpClient();

	void CreateSocket(HWND hWnd);
	void OnDestroy();

	// �̺�Ʈ�� ó���Ѵ�.
	void OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingReadMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingWriteMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	//Interface
	int GetClientId() const { return m_nMainClientID; }
	int GetNumOfClient() const { return m_nClient; }
	XMFLOAT3 GetPostion(int id);
	unordered_map<int, Client_INFO>& GetUMapClientInfo();
	SOCKET GetSocket() const { return m_sock; }
};