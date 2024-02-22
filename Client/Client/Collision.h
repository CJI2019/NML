#pragma once

constexpr DWORD PICKING_COLLISION_LAYER = 0;

class CGameObject;

class CCollisionManager
{
public:
	CCollisionManager() { m_vvpCollisionGameObjects.resize(1); };
	~CCollisionManager() { //�ӽ� ���߿� ����Ʈ�����ͷ� �ٲٱ�
		for (auto& vpCollisionObject : m_vvpCollisionGameObjects)
		{
			for (auto& pCollisionObject : vpCollisionObject)
			{
				if (pCollisionObject)
				{
					delete pCollisionObject;
				}
			}
		}
	}


	void AddCollisionObject(int nIndices, CGameObject* pGameObject);
	size_t GetSizeGameObjects(int nLayer) { return m_vvpCollisionGameObjects[nLayer].size(); }
	vector<CGameObject*> GetVectorGameObjects(int nLayer) { return m_vvpCollisionGameObjects[nLayer]; }

	std::vector<vector<CGameObject*>> m_vvpCollisionGameObjects;
};

extern CCollisionManager g_collisonManager;

class CCollision
{
public:


private:

};

