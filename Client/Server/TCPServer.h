#pragma once
#include "Timer.h"
constexpr size_t MAX_CLIENT{ 5 };
constexpr size_t MAX_SEND_OBJECT_INFO{ 30 };

constexpr WORD KEY_W{ 0x01 };
constexpr WORD KEY_S{ 0x02 };
constexpr WORD KEY_A{ 0x04 };
constexpr WORD KEY_D{ 0x08 };
constexpr WORD KEY_1{ 0x10 };
constexpr WORD KEY_2{ 0x20 };
constexpr WORD KEY_3{ 0x40 };
constexpr WORD KEY_4{ 0x80 };
constexpr WORD KEY_E{ 0x100 };
constexpr WORD KEY_LSHIFT{ 0x200 };
constexpr WORD KEY_LBUTTON{ 0x400 };
constexpr WORD KEY_RBUTTON{ 0x800 };

// ���� ���� ������ ���� ����ü�� ����
class CServerGameObject;
class CServerPlayer;
class CServerCollisionManager;

struct SC_ANIMATION_INFO
{
	float pitch = 1.0f;
};

struct SC_PLAYER_INFO 
{
	RightItem m_selectItem;
	bool m_bRightClick = false;
	
	int m_iMineobjectNum = -1;
	bool m_bAttacked = false;

	int m_iEscapeDoor = -1;
};

struct SC_UPDATE_INFO
{
	//BYTE m_boolCollection;

	INT8 m_nClientId = -1;
	bool m_bAlive = true;
	bool m_bRunning = false;	// BLUESUIT PLAYER�� �޸��� ����
	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Velocity;
	XMFLOAT3 m_xmf3Look;

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

	INT8 m_nHead = -1;

	bool m_bRecvDelayed = false;	// ���� �����͸� ���� ���� ���ߴ�
	bool m_bRecvHead = false;	// ���� �����͸� ���� ���� ���ߴ�
	int m_nCurrentRecvByte = 0;		// ������� ���� �������� ����
	char m_pCurrentBuffer[BUFSIZE];

	SOCKET_STATE m_socketState = SOCKET_STATE::SEND_ID;

	int SendNum = 0;
	int RecvNum = 0;
};

void ConvertCharToLPWSTR(const char* pstr, LPWSTR dest, int destSize);

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
	INT8 AddSocketInfo(SOCKET sockClient, struct sockaddr_in addrClient, int nAddrLen);
	INT8 GetSocketIndex(SOCKET sockClient);
	INT8 RemoveSocketInfo(SOCKET sock);

	void UpdateInformation();

	template<class... Args>
	void CreateSendDataBuffer(char* pBuffer, Args&&... args);
	template<class... Args>	
	int SendData(SOCKET socket, size_t nBufferSize, Args&&... args);	
	int RecvData(int nSocketIndex, size_t nBufferSize);

	void LoadScene();
	void CreateSceneObject(char* pstrFrameName, const XMFLOAT4X4& xmf4x4World, const vector<BoundingOrientedBox>& voobb);
	void CreateItemObject();
	void CreateSendObject();

	//[0509] �÷��̾� ���� ��ġ ��ġ�� �ʵ��� �ʱ�ȭ
	void InitPlayerPosition(shared_ptr<CServerPlayer>& pServerPlayer, int nIndex);

	int CheckAllClientsSentData(int cur_nPlayer);
	void SetAllClientsSendStatus(int cur_nPlayer, bool val);

	// Interface
	void SetClientListBox(HWND hListBox) { m_hClientListBox = hListBox; }

	shared_ptr<CServerPlayer> GetPlayer(int nIndex) { return m_apPlayers[nIndex]; }

	static default_random_engine m_mt19937Gen;
private:
	CTimer m_timer;
	static INT8 m_nClient;
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

	HWND m_hClientListBox;

	//[0509] CServerPlayer���� �ʱ�ȭ�ϴ� ������ġ�� �ű�
	array<XMFLOAT3, 16> m_axmf3Positions;
	array<int, MAX_CLIENT> m_anPlayerStartPosNum;
};

extern void err_quit(const char* msg);
extern void err_display(const char* msg);
extern void err_display(int errcode);