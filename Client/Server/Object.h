#pragma once
class CGameObject
{
public:

	virtual void Update(float fElapsedTime);

protected:
	XMFLOAT4X4 m_xmf4x4ToParent;
	XMFLOAT4X4 m_xmf4x4World;

	//std::weak_ptr<CGameObject> m_pParent;
	//std::shared_ptr<CGameObject> m_pChild;
	//std::shared_ptr<CGameObject> m_pSibling;

	vector<BoundingOrientedBox> m_voobbOrigin;
	// ���� ������Ʈ �з�
	bool m_bCollsion = true;
	int m_nCollisionType = 0; // 0:None, 1:Standard, 2:Picking
};

