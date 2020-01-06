#include <windows.h>
#include <tchar.h>

typedef struct _WindowData
{
	HWND hWnd;
	HFONT hfDefault;
	HFONT hfDrawing;
	HFONT hfCapturingGlyph;
	PWSTR szGlyph;
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

	// Main uMsg loop:
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	PWindowData pwd;

	if (uMsg == WM_CREATE)
	{
		pwd = calloc(1, sizeof(WindowData));
		if (pwd == NULL)
		{
			MessageBox(NULL,
				_T("Call to malloc failed!"),
				_T("Something went wrong"),
				MB_ICONERROR);
			PostQuitMessage(-1);
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		pwd->hWnd = hWnd;
		SetWindowLongPtr(hWnd, 0, (LONG)pwd);

		hdc = GetDC(hWnd);

		INT iHeight = -MulDiv(256, GetDeviceCaps(hdc, LOGPIXELSY), 10);
		pwd->hfCapturingGlyph = CreateFont(-50, 0, 0, 0, FW_BOLD, 0, 0, 0, OEM_CHARSET,
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
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		pwd->hfDrawing = CreateFont(-10, -10, 0, 0, FW_BOLD, 0, 0, 0, OEM_CHARSET,
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
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		pwd->hfDefault = SelectObject(hdc, pwd->hfDrawing);

		ReleaseDC(hWnd, hdc);
		SendMessageW(hWnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		return 0;
	}

	pwd = (PWindowData)GetWindowLongPtr(hWnd, 0);
	if (!pwd)
		return DefWindowProc(hWnd, uMsg, wParam, lParam);

	switch (uMsg)
	{
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);
		if (pwd->szGlyph != NULL)
		{
			SelectObject(hdc, pwd->hfDrawing);

			INT iErr = DrawText(hdc, pwd->szGlyph, -1, &ps.rcPaint, DT_CENTER);
			if (!iErr)
			{
				MessageBox(NULL,
					_T("Call to DrawText failed!"),
					_T("Something went wrong"),
					MB_ICONERROR);
			}
		}
		else
		{
			DrawText(hdc, L"Press any character button", -1, &ps.rcPaint, DT_CENTER);
		}

		EndPaint(hWnd, &ps);
		break;
	}
	case WM_CHAR:
	{
		hdc = GetDC(hWnd);

		SelectObject(hdc, pwd->hfCapturingGlyph);

		GLYPHMETRICS gm = { 0 };
		MAT2 m2 = { 0 };
		m2.eM11.value = 1;
		m2.eM22.value = 1;
		DWORD dwBufferSize = GetGlyphOutline(hdc, wParam, GGO_BITMAP, &gm, 0, NULL, &m2);
		if (dwBufferSize == GDI_ERROR)
		{
			MessageBox(NULL,
				_T("Call to GetGlyphOutline failed!"),
				_T("Something went wrong"),
				MB_ICONERROR);
			return 0;
		}

		if (pwd->szGlyph != NULL)
		{
			free(pwd->szGlyph);
			pwd->szGlyph = NULL;
		}

		PBYTE pbBuffer = calloc(dwBufferSize, sizeof(WCHAR));
		if (pbBuffer == NULL)
		{
			MessageBox(NULL,
				_T("Call to ñalloc failed!"),
				_T("Something went wrong"),
				MB_ICONERROR);
			return 0;
		}

		DWORD dwErr = GetGlyphOutline(hdc, wParam, GGO_BITMAP, &gm, dwBufferSize, pbBuffer, &m2);
		if (dwErr == GDI_ERROR)
		{
			MessageBox(NULL,
				_T("Call to GetGlyphOutline failed!"),
				_T("Something went wrong"),
				MB_ICONERROR);
			return 0;
		}

		DWORD dwGlyphBufferSize = dwBufferSize * dwBufferSize + 1;
		pwd->szGlyph = calloc(dwGlyphBufferSize, sizeof(WCHAR));
		if (pwd->szGlyph == NULL)
		{
			MessageBox(NULL,
				_T("Call to ñalloc failed!"),
				_T("Something went wrong"),
				MB_ICONERROR);
			free(pbBuffer);
			return 0;
		}

		UINT uPosition = 0;
		UINT uRowSize = (gm.gmBlackBoxX / 32);
		uRowSize = ((gm.gmBlackBoxX % 32) ? (uRowSize + 1) : uRowSize) * 4;
		PBYTE pbRow = pbBuffer;
		for (UINT i = 0; i < gm.gmBlackBoxY; ++i) 
		{
			for (UINT j = 0; j < gm.gmBlackBoxX; ++j)
			{
				if ((pbRow[j / 8] >> (7 - j % 8)) & 0x01) 
					pwd->szGlyph[uPosition] = (WCHAR)wParam;
				else             
					pwd->szGlyph[uPosition] = L' ';
				++uPosition;
			}
			pbRow += uRowSize;
			pwd->szGlyph[uPosition] = L'\n';
			++uPosition;
		}
		pwd->szGlyph = realloc(pwd->szGlyph, (uPosition + 1) * sizeof(WCHAR));

		ReleaseDC(hWnd, hdc);
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	}
	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO lpWndInfo = (LPMINMAXINFO)lParam;
		lpWndInfo->ptMinTrackSize.x = 500;
		lpWndInfo->ptMinTrackSize.y = 500;
		break;
	}
	case WM_DESTROY:
	{
		hdc = GetDC(hWnd);
		SelectObject(hdc, pwd->hfDefault);
		DeleteObject(pwd->hfDrawing);
		DeleteObject(pwd->hfCapturingGlyph);
		if (pwd->szGlyph != NULL)
			free(pwd->szGlyph);
		free(pwd);
		ReleaseDC(hWnd, hdc);
		PostQuitMessage(0);
		break;
	}
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}
