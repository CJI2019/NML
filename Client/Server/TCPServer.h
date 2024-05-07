#pragma once
#include "Timer.h"
constexpr size_t MAX_CLIENT{ 5 };
constexpr size_t MAX_SEND_OBJECT_INFO{ 60 };

// ���� ���� ������ ���� ����ü�� ����
class CServerGameObject;
class CServerPlayer;
class CServerCollisionManager;

struct SC_ANIMATION_INFO {
	float pitch = 1.0f;
};

struct SC_PLAYER_INFO {
	RightItem m_selectItem;
	bool m_bRightClick = false;
	
	int m_iMineobjectNum = -1;
};

struct SC_UPDATE_INFO
{
	int m_nClientId = -1;
	bool m_bAlive = true;
	bool m_bRunning = false;	// BLUESUIT PLAYER�� �޸��� ����
	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Velocity;
	XMFLOAT3 m_xmf3Look;
	XMFLOAT3 m_xmf3Right;

	int m_nSlotObjectNum[3] = { -1, -1, -1 };	// �� ���Կ� ���Ե� ������Ʈ ��ȣ(������ -1) /// �� �÷��̾�� ��ų ���� 1��, ��ų �����ų� ���X�� 0 (����, �þ߹���, ����) �޸����� ��� ���� ������ ����(���߿� �߰��Ҳ��� m_bShiftRunȰ���ϸ� �ɵ�)
	int m_nFuseObjectNum[3] = { -1, -1, -1 };	// ǻ�� ������Ʈ ��ȣ(������ -1)

	int m_nNumOfObject = -1;
	std::array<int, MAX_SEND_OBJECT_INFO> m_anObjectNum;
	std::array<XMFLOAT4X4, MAX_SEND_OBJECT_INFO> m_axmf4x4World;

	SC_ANIMATION_INFO m_animationInfo;
	SC_PLAYER_INFO m_playerInfo;
};

enum class SOCKET_STATE
{
	SEND_ID,
	SEND_UPDATE_DATA,			 // Ŭ���̾�Ʈ�� ������ ����
	SEND_NUM_OF_CLIENT,
};

enum RECV_HEAD
{
	HEAD_KEYS_BUFFER = 0,
};

struct SOCKETINFO
{
	bool m_bUsed = false;
	SOCKET m_sock;

	struct sockaddr_in m_addrClient;
	int m_nAddrlen;
	char m_pAddr[INET_ADDRSTRLEN];

	int m_nHead = -1;

	bool m_bRecvDelayed = false;	// ���� �����͸� ���� ���� ���ߴ�
	bool m_bRecvHead = false;	// ���� �����͸� ���� ���� ���ߴ�
	int m_nCurrentRecvByte = 0;		// ������� ���� �������� ����
	char m_pCurrentBuffer[BUFSIZE + 1];

	SOCKET_STATE m_socketState = SOCKET_STATE::SEND_ID;

	int SendNum = 0;
	int RecvNum = 0;
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
	void OnProcessingCloseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	bool Init(HWND hWnd);
	void SimulationLoop();

	// ���� ���� ���� �Լ�
	int AddSocketInfo(SOCKET sockClient, struct sockaddr_in addrClient, int nAddrLen);
	int GetSocketIndex(SOCKET sockClient);
	int RemoveSocketInfo(SOCKET sock);

	void UpdateInformation();

	template<class... Args>
	void CreateSendDataBuffer(char* pBuffer, Args&&... args);
	template<class... Args>	
	int SendData(SOCKET socket, size_t nBufferSize, Args&&... args);	
	int RecvData(int nSocketIndex, size_t nBufferSize);

	void LoadScene();
	void CreateSceneObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb);
	void CreateItemObject();
	void CreatSendObject();

	int CheckAllClientsSentData(int cur_nPlayer);
	void SetAllClientsSendStatus(int cur_nPlayer, bool val);

	// Interface
	shared_ptr<CServerPlayer> GetPlayer(int nIndex) { return m_apPlayers[nIndex]; }

	static default_random_engine m_mt19937Gen;
private:
	CTimer m_timer;
	static size_t m_nClient;
	HWND m_hWnd;
	bool m_bSend = true;

	// ������ Ŭ���̾�Ʈ���� ������ ����.
	std::array<SOCKETINFO, MAX_CLIENT> m_vSocketInfoList;	// ���� �ε����� ���������� �����޴´�

	std::array<std::shared_ptr<CServerPlayer>, MAX_CLIENT> m_apPlayers; 
	std::array<SC_UPDATE_INFO, MAX_CLIENT> m_aUpdateInfo;		
	std::vector<shared_ptr<CServerGameObject>> m_vpGameObject;
	std::shared_ptr<CServerCollisionManager> m_pCollisionManager;

	//Test
	int m_nStartItemNumber = -1;	// ������ ������Ʈ�� ���� �ε���
	int m_nStartDrawer1 = -1;
	int m_nEndDrawer1 = -1;
	int m_nStartDrawer2 = -1;
	int m_nEndDrawer2 = -1;

	bool m_bDataSend[MAX_CLIENT] = { false };
	// �ۼ��� , ������ ������Ʈ�� �̱۽������ �̷����. �����͸� send�� ���Ŀ� ������Ʈ�� �����Ͱ� send������ ���������� �ùٸ� ����ȭ�� �̷����� ����

};

extern void err_quit(const char* msg);
extern void err_display(const char* msg);
extern void err_display(int errcode);