#include <windows.h>
#include <tchar.h>

typedef struct _WindowData
{
	HWND hWnd;
	HFONT hfDefault;
	HFONT hfDrawing;
	HFONT hfCapturingGlyph;
} WindowData, *PWindowData;
// Global variables

// The main window class name.
TCHAR szWindowClass[] = _T("Letters from letters");

// The string that appears in the application's title bar.
TCHAR szTitle[] = _T("Letters from letters");

HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 4;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			_T("Something went wrong"),
			MB_ICONERROR);

		return 1;
	}

	// Store instance handle in our global variable
	hInst = hInstance;

	HWND hWnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW | WS_MAXIMIZE | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		500, 500,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd)
	{
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			_T("Something went wrong"),
			MB_ICONERROR);

		return 1;
	}

	// Main message loop:
	MSG msg;
	BOOL bRet;

	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			MessageBox(NULL,
				_T("Call to GetMessage failed!"),
				_T("Something went wrong"),
				MB_ICONERROR);
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (INT)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	PWindowData pwd;

	if (message == WM_CREATE)
	{
		pwd = malloc(sizeof(WindowData));
		if (pwd == NULL)
		{
			MessageBox(NULL,
				_T("Call to malloc failed!"),
				_T("Something went wrong"),
				MB_ICONERROR);
			PostQuitMessage(-1);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		pwd->hWnd = hWnd;
		SetWindowLongPtr(hWnd, 0, (LONG)pwd);

		hdc = GetDC(hWnd);

		INT iHeight = -MulDiv(256, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		pwd->hfCapturingGlyph = CreateFont(iHeight, 0, 0, 0, FW_BOLD, 0, 0, 0, OEM_CHARSET,
			OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			FF_DONTCARE, NULL);
		if (pwd->hfCapturingGlyph == NULL)
		{
			free(pwd);
			MessageBox(NULL,
				_T("Call to CreateFont failed!"),
				_T("Something went wrong"),
				MB_ICONERROR);
			PostQuitMessage(-1);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		iHeight = -MulDiv(256, GetDeviceCaps(hdc, LOGPIXELSY), 10);
		pwd->hfDrawing = CreateFont(iHeight, iHeight, 0, 0, FW_BOLD, 0, 0, 0, OEM_CHARSET,
			OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			FIXED_PITCH | FF_DONTCARE, NULL);
		if (pwd->hfDrawing == NULL)
		{
			DeleteObject(pwd->hfCapturingGlyph);
			free(pwd);
			MessageBox(NULL,
				_T("Call to CreateFont failed!"),
				_T("Something went wrong"),
				MB_ICONERROR);
			PostQuitMessage(-1);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		pwd->hfDefault = SelectObject(hdc, pwd->hfDrawing);

		ReleaseDC(hWnd, hdc);
		SendMessageW(hWnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		return 0;
	}

	pwd = (PWindowData)GetWindowLongPtr(hWnd, 0);
	if (!pwd)
		return DefWindowProc(hWnd, message, wParam, lParam);

	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_GETMINMAXINFO:
		LPMINMAXINFO lpWndInfo = lParam;
		lpWndInfo->ptMinTrackSize.x = 500;
		lpWndInfo->ptMinTrackSize.y = 500;
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
