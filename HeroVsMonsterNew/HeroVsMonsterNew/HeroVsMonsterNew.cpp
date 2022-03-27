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
bool				idleL = false;
bool				runR = false;
bool				runL = false;
//bool				jumping = false;
// pic and window
int					app_Wid = 960;
int					app_Hei = 540;
int					bg_Wid = 1920;
int					bg_Hei = 1080;

// Background --------------------------------------------------------------
struct bg {
	HDC hDCbg;
	HBITMAP parallax;
	int x = 0;
	int m = 7;
};
std::vector<bg> bgs;
// Device Contexts ----------------------------------------------------------
HDC			hDC;				// Vår huvudsakliga DC - Till fönstret
HDC			playerHDC[2];			// DC till explosionen
HDC			bufferHDC;			// hdc till buffer
// BITMAPS ------------------------------------------------------------------
HBITMAP		player[2];				// player
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
	int langd = bgs.size() - 1;
	
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
			runL = false;
			idleL = false;
			runR = true;
			idleR = true;
			for (int n = 0; n < langd; n++) {
				bgs[n].x -= (1 * bgs[n].m);
				if (bgs[n].x < -1920 / 2) {
					bgs[n].x = 0;
				}
			}
		}
		if (wParam == VK_LEFT) {
			runL = true;
			idleL = true;
			runR = false;
			idleR = false;
			for (int n = 0; n < langd; n++) {
				bgs[n].x += (1 * bgs[n].m);
				if (bgs[n].x > 1920 / 2) {
					bgs[n].x = 0;
				}
			}
		}
		break;
	case WM_KEYUP:
		runR = false;
		runL = false;
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

	if (counter % 7 == 0) {
		bgs[6].x -= 1;
		if (bgs[6].x < -1920 / 2) {
			bgs[6].x = 0;
		}
		// springer eller står till höger
		if (runR == true) {
			ypic = 288;
			xpic -= 144;
			if (xpic < 1) {
				xpic = 720;
			}
		}
		else if (idleR == true) {
			ypic = 0;
			xpic += 144;
			if (xpic > 863) {
				xpic = 144;
			}
		}
		// springer eller står till vänster
		if (runL == true) {
			ypic = 288;
			xpic += 144;
			if (xpic > 863) {
				xpic = 0;
			}
		}
		else if (idleL == true) {
			ypic = 0;
			xpic -= 144;
			if (xpic < 0) {
				xpic = 576;
			}
		}

	}
}
//---------------------------------------------------------------------------
void render() {
	static bool yes = true;

	int langd = bgs.size() - 1;

	for (int n = langd; n >= 0; n--) {

		TransparentBlt(bufferHDC, bgs[n].x, 0, app_Wid, app_Hei, bgs[n].hDCbg, 0, 0, bg_Wid, bg_Hei, COLORREF(RGB(255, 0, 255)));

		if (bgs[n].x < 0) {
			TransparentBlt(bufferHDC, app_Wid + bgs[n].x, 0, -bgs[n].x, app_Hei, bgs[n].hDCbg, 0, 0, -bgs[n].x * 2, bg_Hei, COLORREF(RGB(255, 0, 255)));
		}
		else if (bgs[n].x > 0) {
			TransparentBlt(bufferHDC, 0, 0, bgs[n].x, app_Hei, bgs[n].hDCbg, bg_Wid - bgs[n].x, 0, bgs[n].x, bg_Hei, COLORREF(RGB(255, 0, 255)));
		}
		if (n == 1) {
			if (idleR == true) {
				TransparentBlt(bufferHDC, app_Wid / 2 - 72, 275, 144, 144, playerHDC[0], xpic, ypic, 144, 144, COLORREF(RGB(255, 0, 255)));
			}
			else if (idleL == true){
				TransparentBlt(bufferHDC, app_Wid / 2 - 72, 275, 144, 144, playerHDC[1], xpic, ypic, 144, 144, COLORREF(RGB(255, 0, 255)));
			}
		}
	}
	
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
		if(n > 1)tmp.m = 8 - n; 
		bgs.push_back(tmp);
		oldBitmap[n] = (HBITMAP)SelectObject(bgs[n].hDCbg, bgs[n].parallax);
	}

	// Läser in spelaren
	playerHDC[0] = CreateCompatibleDC(hDC);	// Skapa en hdc för spelaren
	playerHDC[1] = CreateCompatibleDC(hDC);	// Skapa en hdc för spelaren
	player[0] = (HBITMAP)LoadImage(NULL, (LPCTSTR)("bilder/hero1.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	player[1] = (HBITMAP)LoadImage(NULL, (LPCTSTR)("bilder/hero2.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	oldBitmap[8] = (HBITMAP)SelectObject(playerHDC[0], player[0]);
	oldBitmap[9] = (HBITMAP)SelectObject(playerHDC[1], player[1]);
	
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

	for (unsigned int n = 0; n < 10; n++) {
		SelectObject(bgs[n].hDCbg, bgs[n].parallax);
		DeleteObject(oldBitmap[n]);
		ReleaseDC(hWnd, bgs[n].hDCbg);
		DeleteDC(bgs[n].hDCbg);
	}
	
	SelectObject(playerHDC[0], oldBitmap[8]);
	DeleteObject(player[0]);
	SelectObject(playerHDC[1], oldBitmap[9]);
	DeleteObject(player[1]);

	//Ta bort hdc till fönstret och imageHDC
	ReleaseDC(hWnd, playerHDC[0]);
	DeleteDC(playerHDC[0]);
	ReleaseDC(hWnd, playerHDC[1]);
	DeleteDC(playerHDC[1]);
	
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