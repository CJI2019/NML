#pragma once
#include "TCPServer.h"

constexpr float SPACE_SIZE_XZ{ 6.0f };
constexpr float SPACE_SIZE_Y{ 4.5f };
constexpr float GRID_START_X{ -30.0f };
constexpr float GRID_START_Y{ -0.01f };
constexpr float GRID_START_Z{ -30.0f };

class CServerGameObject;
class CServerPlayer;

typedef vector<shared_ptr<CServerGameObject>> vpObjects_t;
typedef vector<vector<vector<vpObjects_t>>> collisionGrid_t;

class CServerCollisionManager : public std::enable_shared_from_this<CServerCollisionManager>
{
public:
	CServerCollisionManager() {};
	~CServerCollisionManager() {}

	void CreateCollision(int nHeight, int nWidth, int nDepth);
	void AddCollisionObject(const shared_ptr<CServerGameObject>& pGameObject);
	void ReplaceCollisionObject(const shared_ptr<CServerGameObject>& pGameObject);
	void AddCollisionPlayer(const shared_ptr<CServerPlayer>& pPlayer, int nIndex);

	vpObjects_t& GetSpaceGameObjects(int nHeight, int nWidth, int nDepth);

	void Update(float fElapsedTime);
	void Collide(float fElapsedTime, const shared_ptr<CServerPlayer>& pPlayer);

	collisionGrid_t m_collisionGridGameObjects;
	std::array<weak_ptr<CServerPlayer>, MAX_CLIENT> m_apPlayer;

	static int GetNumberOfCollisionObject() { return CServerCollisionManager::m_nCollisionObject; }
	shared_ptr<CServerGameObject> GetCollisionObjectWithNumber(int nCollisionNum) { return m_vpCollisionObject[nCollisionNum]; }
	vector<shared_ptr<CServerGameObject>> GetOutSpaceObject() { return m_vpOutSpaceObject; }

	int GetHeight() const { return m_nHeight; }
	int GetWidth() const { return m_nWidth; }
	int GetDepth() const { return m_nDepth; }

private:
	static int m_nCollisionObject;

	vector<shared_ptr<CServerGameObject>> m_vpCollisionObject;	// ��ȣ�� �̿��ؼ� �浹 ��ü�� ������ ������������
	vector<shared_ptr<CServerGameObject>> m_vpOutSpaceObject;	// ���� ������ ������Ʈ�� �Ͼ�� � ������Ʈ�� �ѹ� �����ܿ����� ������Ʈ�� �ʿ��ϴ�

	int m_nHeight; // == Floor
	int m_nWidth;
	int m_nDepth;
};
