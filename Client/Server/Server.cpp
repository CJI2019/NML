#include "stdafx.h"
#include "TCPServer.h"

TCPServer g_tcpServer;
HINSTANCE hInst;                  

// ������ �޽��� ó�� �Լ�
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OnProcessingSocketMessage(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int main()
{
	// ������ Ŭ���� ���
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = _T("MyWndClass");
	if (!RegisterClass(&wndclass)) return 1;
	
	// ������ ����
	HWND hWnd = CreateWindow(_T("MyWndClass"), _T("TCP ����"), WS_OVERLAPPEDWINDOW, 0, 0, 600, 200, NULL, NULL, NULL, NULL);
	if (hWnd == NULL) return 1;
	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	//���� �غ��۾�
	g_tcpServer.Init(hWnd);

	MSG msg;
	// �޽��� ����
	while (1)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{

			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				break;
			}
		}
		else
		{
			g_tcpServer.SimulationLoop();
		}
	}

	// ���� ����
	WSACleanup();
	return msg.wParam;
}

// ������ �޽��� ó��
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	switch (uMsg) 
	{
	case WM_ACTIVATE:
		OnProcessingWindowMessage(hWnd, uMsg, wParam, lParam);
		break;
	case WM_SOCKET: // ���� ���� ������ �޽���
		return OnProcessingSocketMessage(hWnd, uMsg, wParam, lParam);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ACTIVATE:
		g_tcpServer.OnProcessingWindowMessage(hWnd, uMsg, wParam, lParam);
		break;
	}
	return 0;
}

// ���� ���� �޽��� ó��
LRESULT CALLBACK OnProcessingSocketMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// ���� �߻� ���� Ȯ��
	if (WSAGETSELECTERROR(lParam) && WSAGETSELECTEVENT(lParam) != FD_CLOSE) 
	{
		err_display(WSAGETSELECTERROR(lParam));
		int nIndex = g_tcpServer.RemoveSocketInfo(wParam);
		g_tcpServer.GetPlayer(nIndex).reset();
		return -1;
	}

	// �޽��� ó��
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_ACCEPT:
	case FD_READ:
	case FD_WRITE:		
	case FD_CLOSE:
		g_tcpServer.OnProcessingSocketMessage(hWnd, uMsg, wParam, lParam);
		break;
	default:
		break;
	}

	return 0;
}