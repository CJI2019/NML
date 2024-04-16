#pragma once
#include "Timer.h"
#include "Scene.h"
#include "TCPClient.h"

//constexpr size_t SWAPCHAIN_BUFFER_NUM = 2;

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();

	void CreateRtvAndDsvDescriptorHeaps();

	void CreateRenderTargetViews();
	void CreateDepthStencilView();

	void ChangeSwapChainState();

	void BuildObjects();
	void ReleaseObjects();

	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	static UCHAR* GetKeysBuffer();

	void SetPlayerObjectOfClient(int nClientId);
private:
	D3D12_VIEWPORT m_d3dViewport;
	D3D12_RECT m_d3dScissorRect;
	//����Ʈ�� ���� �簢���̴�.

	HINSTANCE							m_hInstance;
	HWND								m_hWnd;

	int									m_nWndClientWidth;
	int									m_nWndClientHeight;

	ComPtr<IDXGIFactory4>				m_dxgiFactory;
	ComPtr<IDXGISwapChain3>				m_dxgiSwapChain;
	ComPtr<ID3D12Device>				m_d3d12Device;
	bool								m_bMsaa4xEnable = false;
	UINT								m_nMsaa4xQualityLevels = 0;

	static const UINT					m_nSwapChainBuffers = 2;
	UINT								m_nSwapChainBufferIndex;

	std::array<ComPtr<ID3D12Resource>, m_nSwapChainBuffers>			m_d3dSwapChainBackBuffers;
	ComPtr<ID3D12DescriptorHeap>									m_d3dRtvDescriptorHeap;
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, m_nSwapChainBuffers>	m_pd3dSwapChainBackBufferRTVCPUHandles;

	ComPtr<ID3D12Resource>				m_d3dDepthStencilBuffer;
	ComPtr<ID3D12DescriptorHeap>		m_d3dDsvDescriptorHeap;

	ComPtr<ID3D12CommandAllocator>		m_d3dCommandAllocator;
	ComPtr<ID3D12CommandQueue>			m_d3dCommandQueue;
	ComPtr<ID3D12GraphicsCommandList>	m_d3dCommandList;

	ComPtr<ID3D12Fence>						m_d3dFence;
	std::array<UINT64, m_nSwapChainBuffers>	m_nFenceValues;
	HANDLE									m_hFenceEvent;

#if defined(_DEBUG)
	ID3D12Debug*						m_pd3dDebugController;
#endif

	CGameTimer							m_GameTimer;
	
	shared_ptr<CScene>					m_pScene;

	std::shared_ptr<CPlayer>					m_pMainPlayer;	// Ŭ���̾�ƮID�� �ش��ϴ� �ε����� �ش� Ŭ���̾�Ʈ�� Main�÷��̾�� �����ȴ�
	std::array<shared_ptr<CPlayer>, MAX_CLIENT>	m_apPlayer;		// Ŭ���̾�ƮID�� �ε����� �����ϴ�.
	weak_ptr<CCamera>							m_pCamera;

	CPostProcessingShader*				m_pPostProcessingShader = NULL;

	POINT								m_ptOldCursorPos;
	_TCHAR								m_pszFrameRate[200];
	
	static UCHAR						m_pKeysBuffer[256];
	//TCPClient
	shared_ptr<CTcpClient>				m_pTcpClient;
};

