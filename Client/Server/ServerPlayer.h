#pragma once
#include "ServerObject.h"

constexpr WORD DIR_FORWARD = 0x01;
constexpr WORD DIR_BACKWARD = 0x02;
constexpr WORD DIR_LEFT = 0x04;
constexpr WORD DIR_RIGHT = 0x08;
constexpr WORD PRESS_LBUTTON = 0x10;
constexpr WORD PRESS_RBUTTON = 0x20;
constexpr WORD DIR_UP = 0x40;
constexpr WORD DIR_DOWN = 0x80;
constexpr FLOAT ASPECT_RATIO = 1600.0f / 1024.0f;

class CServerPlayer : public CServerGameObject
{
public:
	CServerPlayer();
	virtual ~CServerPlayer() {};

	virtual void UseItem(shared_ptr<CServerCollisionManager>& pCollisionManager) {};
	virtual void Update(float fElapsedTime) override;
	virtual void Collide(const shared_ptr<CServerCollisionManager>& pCollisionManager, float fElapsedTime, shared_ptr<CServerGameObject> pCollided) override;
	void Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity);

	virtual void UpdatePicking() override {};
	//virtual void UseItem(int nSlot) {};

	void CalculateSpace();
	void OnUpdateToParent();
	// Interface
	void SetRecvData(bool bRecvData) { m_bRecvData = bRecvData; }

	void SetPlayerId(int nPlayerId) { m_nPlayerId = nPlayerId; }

	void SetStair(bool bStair) { m_bStair = bStair; }
	void SetStairY(float fMax, float fMin) { m_fStairMax = fMax; m_fStairMin = fMin; }
	void SetStairPlane(const XMFLOAT4& xmf4StairPlane) { m_xmf4StairPlane = xmf4StairPlane; }

	void SetWorldMatrix(const XMFLOAT3& xmf3Position) { m_xmf3Position = xmf3Position; }

	void SetLook(const XMFLOAT3& xmf3Look) { m_xmf3Look = xmf3Look; }
	void SetRight(const XMFLOAT3& xmf3Right) { m_xmf3Right = xmf3Right; }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }

	void SetPickedObject(const shared_ptr<CServerCollisionManager> pCollisionManager);
	void SetViewMatrix(const XMFLOAT4X4& xmf4x4View) { m_xmf4x4View = xmf4x4View; }

	bool IsRecvData() const { return m_bRecvData; }
	UCHAR* GetKeysBuffer() { return m_pKeysBuffer; }
	int GetPlayerId() const { return m_nPlayerId; }

	bool IsStair()const { return m_bStair; }
	float GetStairMax() const { return m_fStairMax; }
	float GetStairMin() const { return m_fStairMin; }
	XMFLOAT4 GetStairPlane() { return m_xmf4StairPlane; }

	XMFLOAT3 GetPosition() const { return m_xmf3Position; }
	XMFLOAT3 GetVelocity() const { return m_xmf3Velocity; }
	XMFLOAT3 GetLook() const { return m_xmf3Look; }
	XMFLOAT3 GetRight() const { return m_xmf3Right; }

	weak_ptr<CServerGameObject> GetPickedObject() { return m_pPickedObject; }
protected:
	// ù �����͸� �ޱ� ����
	bool m_bRecvData = false;

	// ����� ���� �޴� ����
	int m_nPlayerId = -1;
	UCHAR m_pKeysBuffer[256];

	bool m_bCollision = false;
	bool m_bStair = false;		// ��ܿ� �ִ� ����
	
	float m_fStairMax;
	float m_fStairMin;
	XMFLOAT4 m_xmf4StairPlane;

	XMFLOAT3					m_xmf3OldPosition;
	XMFLOAT3					m_xmf3Position;
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3					m_xmf3OldVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);	// �̵��� ����� �ӷ� ����
	XMFLOAT3					m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3     				m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float           			m_fMaxVelocityXZ = 0.0f;
	float           			m_fMaxVelocityY = 0.0f;
	float           			m_fFriction = 0.0f;

	weak_ptr<CServerGameObject>		m_pPickedObject;
	bool						m_bPressed = false;

	XMFLOAT4X4					m_xmf4x4View;
	XMFLOAT4X4					m_xmf4x4Projection;

};

///
///
/// 

enum ITEM_SLOT
{
	Teleport,
	Radar,
	Mine,
	Fuse
};

class CServerItemObject;
class CServerFuseObject;

class CServerBlueSuitPlayer : public CServerPlayer
{
public:
	CServerBlueSuitPlayer();
	virtual ~CServerBlueSuitPlayer() {};

	virtual void UseItem(shared_ptr<CServerCollisionManager>& pCollisionManager);
	virtual void Update(float fElapsedTime) override;
	virtual void UpdatePicking() override;

	int AddItem(const shared_ptr<CServerGameObject>& pGameObject);
	//virtual void UseItem(int nSlot) override;
	void UseFuse(shared_ptr<CServerCollisionManager>& pCollisionManager);
	void TeleportRandomPosition();

	int GetReferenceSlotItemNum(int nIndex);
	int GetReferenceFuseItemNum(int nIndex);
private:
	std::array<shared_ptr<CServerItemObject>, 3> m_apSlotItems;

	int m_nFuseNum = 0;
	std::array<shared_ptr<CServerFuseObject>, 3> m_apFuseItems;

	bool m_bShiftRun = false;
	bool m_bAbleRun = true;
	float m_fStamina = 5.0f;
};

///
///
/// 


class CServerZombiePlayer : public CServerPlayer
{

};