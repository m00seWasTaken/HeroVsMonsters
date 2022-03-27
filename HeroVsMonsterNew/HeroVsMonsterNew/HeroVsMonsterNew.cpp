#define WIN32_LEAN_AND_MEAN	
#include <Windows.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include <time.h>
// Globla variabler ---------------------------------------------------------
LPCTSTR				ClsName = "Bgstuff";
double				CPUFreq = 0.0;
bool				running = true, run = false;
int					innerWidth, innerHeight, x = 0;
int					anistage = 0;
// player animation
int					xpic = 144, ypic = 0;
bool				idleR = true;
bool				idleL = true;
bool				runR = false;
bool				runL = false;
bool				jumping = false;
// pic and window
int					app_Wid = 960;
int					app_Hei = 540;
int					bg_Wid = 1920;
int					bg_Hei = 1080;

// Background --------------------------------------------------------------
struct bg {
	HDC hDCbg;
	HBITMAP parallax;
	int x;


	// this is epioc
};
std::vector<bg> bgs;
// Device Contexts ----------------------------------------------------------
HDC			hDC;				// Vår huvudsakliga DC - Till fönstret
HDC			playerHDC;			// DC till explosionen
HDC			bufferHDC;			// hdc till buffer
// BITMAPS ------------------------------------------------------------------
HBITMAP		player;				// player
HBITMAP		oldBitmap[11];		// Lagrar orginalbilderna
HBITMAP		bitmapbuff;
// Funktioner ---------------------------------------------------------------
LRESULT		CALLBACK	winProc(HWND, UINT, WPARAM, LPARAM);
ATOM 		doRegister(HINSTANCE);
BOOL 		initInstance(HINSTANCE, int);
int			initalizeAll(HWND);
void		releaseAll(HWND);
void		makeExplosion(int);	// alla explosioner
void		update();			// Alla uppdateringar
void		render();			// Ritar ut bilden i fönstret
bool		framerate(int);		// Uppdateringsfrekvensen
double		getFreq();
inline __int64 performanceCounter() noexcept {
	LARGE_INTEGER li;
	::QueryPerformanceCounter(&li);
	return li.QuadPart;
};
//---------------------------------------------------------------------------
int WINAPI WinMain(_In_ HINSTANCE hi, _In_opt_ HINSTANCE hp, _In_ LPSTR lp, _In_ int n) {
	UNREFERENCED_PARAMETER(hp);
	UNREFERENCED_PARAMETER(lp);
	if (!(doRegister(hi)) || !(initInstance(hi, n))) {
		MessageBox(NULL, (LPCSTR)"Fel, Kan ej registrera eller skapa fönstret i windows", (LPCSTR)"ERROR", MB_ICONERROR | MB_OK);
		return 0;
	}
	MSG msg;

	// Ny loop där vi själv bestämmer när saker ritas ut
	while (running == true) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (framerate(20) == true) {
			render();
			update();
		}
	}
	return 0;
}
//-------------------------------------------------------------------------
bool framerate(int timeStamp) {
	static __int64 last = performanceCounter();
	if (((double)((performanceCounter() - last)) / CPUFreq) > timeStamp) {
		last = performanceCounter();
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
LRESULT CALLBACK winProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch (Msg) {
	case WM_CREATE:
		initalizeAll(hWnd);
		break;
	case WM_LBUTTONDOWN:
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		running = false;
		break;
	case WM_KEYDOWN:
		if (wParam == VK_RIGHT) {
			runR = true;
			idleR = false;
			x -= 4;
			if (x < -1920 / 2) {
				x = 0;
				//x += 5;
			}
		}
		if (wParam == VK_LEFT) {
			x += 4;
			if (x > 1920 / 2) {
				x = 0;
			}
		}
		break;
	case WM_KEYUP:
		runR = false;
		runL = false;
		idleR = true;
		break;
	case WM_PAINT:
		render();
		break;
	case WM_DESTROY:
		releaseAll(hWnd);
		break;
	default:
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}
//---------------------------------------------------------------------------
void update() {
	static int counter = 0;
	counter++;

	// update the player frames

	// right

	// left

	// none do nothing

	if (counter % 10 == 0) {
		if (idleR == true) {
			ypic = 0;
			xpic += 144;
			if (xpic > 863) {
				xpic = 144;
			}
		}
		else if (runL == true) {

		}
		else if (runR == true) {
			ypic = 288;
			xpic += 144;
			if (xpic > 863) {
				xpic = 0;
			}
		}
	}
}
//---------------------------------------------------------------------------
void render() {
	static bool yes = true;

	int langd = bgs.size() - 1;

	for (int n = langd; n > -1; n--) {

		TransparentBlt(bufferHDC, 0, 0, app_Wid, app_Hei, bgs[n].hDCbg, 0, 0, bg_Wid, bg_Hei, COLORREF(RGB(255, 0, 255)));

		if (x < 0) {
			TransparentBlt(bufferHDC, app_Wid + x, 0, -x, app_Hei, bgs[n].hDCbg, 0, 0, -x * 2, bg_Hei, COLORREF(RGB(255, 0, 255)));
		}
		else if (x > 0) {
			TransparentBlt(bufferHDC, 0, 0, x, app_Hei, bgs[n].hDCbg, bg_Wid - x, 0, x * 2, bg_Hei, COLORREF(RGB(255, 0, 255)));
		}
	}
	TransparentBlt(bufferHDC, app_Wid / 2 - 72, 275, 144, 144, playerHDC, xpic, ypic, 144, 144, COLORREF(RGB(255, 0, 255)));

	BitBlt(hDC, 0, 0, innerWidth, innerHeight, bufferHDC, 0, 0, SRCCOPY);

	/*
	TransparentBlt(bufferHDC, 0, 0, app_Wid, app_Hei, bgs[7].hDCbg, 0, 0, bg_Wid, bg_Hei, COLORREF(RGB(255, 0, 255)));
	// Ground Layer 1
	TransparentBlt(bufferHDC, x, 0, app_Wid, app_Hei, bgs[0].hDCbg, 0, 0, bg_Wid, bg_Hei, COLORREF(RGB(255, 0, 255))); // Fixa så att den tar bort sig själv utanför skärmen
	// Trees and bushes layer 2
	TransparentBlt(bufferHDC, x, 0, app_Wid, app_Hei, bgs[1].hDCbg, 0, 0, bg_Wid, bg_Hei, COLORREF(RGB(255, 0, 255)));
	if (x < 0) {
		// Ground Layer 1
		TransparentBlt(bufferHDC, app_Wid + x, 0, -x, app_Hei, bgs[0].hDCbg, 0, 0, -x * 2, bg_Hei, COLORREF(RGB(255, 0, 255)));
		// Trees and bushes layer 2
		TransparentBlt(bufferHDC, app_Wid + x, 0, -x, app_Hei, bgs[1].hDCbg, 0, 0, -x * 2, bg_Hei, COLORREF(RGB(255, 0, 255)));
	}
	else if (x > 0) {
		// Ground Layer 1
		TransparentBlt(bufferHDC, 0, 0, x, app_Hei, bgs[0].hDCbg, bg_Wid - x, 0, x, bg_Hei, COLORREF(RGB(255, 0, 255))); // no work for no reason
		// Trees and bushes layer 2
		TransparentBlt(bufferHDC, 0, 0, x, app_Hei, bgs[1].hDCbg, bg_Wid - x, 0, x, bg_Hei, COLORREF(RGB(255, 0, 255)));
	}
	*/

	// spelaren
	//TransparentBlt(bufferHDC, bg_Wid / 2 - 86, 475, 172, 183, playerHDC, xpic, ypic, 172, 183, COLORREF(RGB(255, 0, 255)));

	//dubbelbuff

}
//---------------------------------------------------------------------------
int	initalizeAll(HWND hWnd) {
	srand(time(NULL));
	// Hämta fönstrets riktiga bredd & höjd
	RECT		windowRect;
	GetClientRect(hWnd, &windowRect);
	innerWidth = windowRect.right;
	innerHeight = windowRect.bottom;

	hDC = GetDC(hWnd);						// Koppla fönstret till en DC

	bg tmp;
	std::string text;
	for (unsigned int n = 0; n < 8; n++) {	// läser in HDC och bilder till bgs
		tmp.hDCbg = CreateCompatibleDC(hDC);
		text = "bilder/layer_0" + std::to_string(n + 1) + ".bmp";
		tmp.parallax = (HBITMAP)LoadImage(NULL, (LPCTSTR)(text.c_str()), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		bgs.push_back(tmp);
		oldBitmap[n] = (HBITMAP)SelectObject(bgs[n].hDCbg, bgs[n].parallax);
	}

	// Läser in spelaren
	playerHDC = CreateCompatibleDC(hDC);	// Skapa en hdc för spelaren
	player = (HBITMAP)LoadImage(NULL, (LPCTSTR)("bilder/hero2.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	oldBitmap[8] = (HBITMAP)SelectObject(playerHDC, player);

	// Buffer
	bufferHDC = CreateCompatibleDC(hDC);				// Skapa en hdc för bakgrundsbilden
	bitmapbuff = CreateCompatibleBitmap(hDC, innerWidth, innerHeight);
	SelectObject(bufferHDC, bitmapbuff);
	//Ger CPU-frekvensen som används med performanceCounter();
	CPUFreq = getFreq();

	return 1;
}
//---------------------------------------------------------------------------
void releaseAll(HWND hWnd) {

	for (unsigned int n = 0; n < 8; n++) {
		SelectObject(bgs[n].hDCbg, bgs[n].parallax);
		DeleteObject(oldBitmap[n]);
		ReleaseDC(hWnd, bgs[n].hDCbg);
		DeleteDC(bgs[n].hDCbg);
	}

	SelectObject(playerHDC, oldBitmap[2]);
	DeleteObject(player);

	//Ta bort hdc till fönstret och imageHDC
	ReleaseDC(hWnd, playerHDC);
	DeleteDC(playerHDC);
	ReleaseDC(hWnd, hDC);
	DeleteDC(hDC);
}
//---------------------------------------------------------------------------
BOOL initInstance(HINSTANCE hInstance, int nCmdShow) {
	//Bredd och höjd för fönstret som vi skapar

	HWND hWnd = CreateWindowEx(
		0, ClsName, (LPCSTR)ClsName, WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_BORDER,
		((GetSystemMetrics(SM_CXSCREEN) - app_Wid) >> 1),  //Sätter fönstret i mitten i x-led;
		((GetSystemMetrics(SM_CYSCREEN) - app_Hei) >> 1),  //Sätter fönstret i mitten i Y-led;
		app_Wid, app_Hei, NULL, NULL, hInstance, NULL);

	if (!hWnd) {
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return TRUE;
}
//-----------------------------------------------------------------------------------------------
ATOM doRegister(HINSTANCE hi) {
	WNDCLASSEX wincl;

	wincl.hInstance = hi;
	wincl.lpszClassName = ClsName;
	wincl.lpfnWndProc = winProc;
	wincl.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wincl.cbSize = sizeof(WNDCLASSEX);
	wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wincl.lpszMenuName = NULL;
	wincl.cbClsExtra = 0;
	wincl.cbWndExtra = 0;
	wincl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

	return RegisterClassEx(&wincl);
}
//---------------------------------------------------------------------------
double getFreq() {
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	return double(li.QuadPart) / 1000.0;
}
//-------------------------------------------