#include "stdafx.h"
#include "Collision.h"
#include "Object.h"
#include "Player.h"

int CCollisionManager::m_nCollisionObject = 0;

CCollisionManager g_collisionManager;

void CCollisionManager::CreateCollision(int nHeight, int nWidth, int nDepth)
{
	m_nHeight = nHeight;
	m_nWidth = nWidth;
	m_nDepth = nDepth;	

	m_collisionGridGameObjects.resize(m_nHeight);
	for (int i = 0; i < m_nHeight; ++i)
	{
		m_collisionGridGameObjects[i].resize(m_nWidth);
		for (int j = 0; j < m_nWidth; ++j)
		{
			m_collisionGridGameObjects[i][j].resize(m_nDepth);
		}
	}
	m_pCollisionObject.reserve(1000);
}

void CCollisionManager::AddCollisionObject(const shared_ptr<CGameObject>& pGameObject)
{
	pGameObject->m_nCollisionNum = m_nCollisionObject++;
	m_pCollisionObject.emplace_back(pGameObject);
}

void CCollisionManager::AddNonCollisionObject(const shared_ptr<CGameObject>& pGameObject)
{
	m_pNonCollisionObjects.emplace_back(pGameObject);
}


vpObjects_t& CCollisionManager::GetSpaceGameObjects(int nHeight, int nWidth, int nDepth)
{
	return m_collisionGridGameObjects[nHeight][nWidth][nDepth];
}

