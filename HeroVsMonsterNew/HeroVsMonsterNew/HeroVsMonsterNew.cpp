#define WIN32_LEAN_AND_MEAN	
#include <Windows.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include <time.h>
// Globla variabler ---------------------------------------------------------
LPCTSTR		ClsName = "Bgstuff";
double		CPUFreq = 0.0;
bool		running = true, run = false;
int			innerWidth, innerHeight;
int			anistage = 0;
// pic and window
int			app_Wid = 960;
int			app_Hei = 540;
int			bg_Wid = 1920;
int			bg_Hei = 1080;
int			x, y;
// player animation
int			xpic = 144, ypic = 0;
int			playerX = app_Wid/2 - 72, playerY = 275;
bool		dead = false;
int			points;
bool		idleR = true;
bool		idleL = false;
bool		runR = false;
bool		runL = false;
// Background --------------------------------------------------------------
struct bg {
	HDC		hDCbg;
	HBITMAP parallax;
	int		x = 0;
	int		m = 7;
};
std::vector<bg> bgs;
// explosions ---------------------------------------------------------------
struct booms {
	int		width = 160;
	int		height = 120;
	int		x;
	int		y;
	int		cX = 0;
	int		cY = 0;
};
int expLangd;
std::vector<booms> exps;
// monsters -----------------------------------------------------------------
struct enemie {
	int		width = 96;
	int		height = 96;
	int		x;
	int		y;
	int		cX = 0;
	int		cY = 0;
	bool	runR = false;
	bool	runL = false;
	int		id = 0;
};
int monstLangd;
std::vector<enemie> monst;
// Device Contexts ----------------------------------------------------------
HDC			hDC;				// V�r huvudsakliga DC - Till f�nstret
HDC			playerHDC[2];		// DC till explosionen
HDC			expHDC;
HDC			monsterHDC;			// monsterHDC
HDC			bufferHDC;			// hdc till buffer
// BITMAPS ------------------------------------------------------------------
HBITMAP		player[2];			// player
HBITMAP		explosion;			// explosion
HBITMAP		monsters;			// monsters
HBITMAP		oldBitmap[12];		// Lagrar orginalbilderna
HBITMAP		bitmapbuff;
// Funktioner ---------------------------------------------------------------
void		runRight(int);			// spelaren springer höger
void		runLeft(int);			// spelaren springer vänster
void		playerAnimation();		// animation för spelaren
void		score();				// splearens poäng
void		createMonster();		// skapar ett monster
void		controllMonster();		// kontrollerar monster
void		monsterKill();			// monster dödar spelaren
void		makeExplosion(LPARAM);	// alla explosioner
void		collision();
// Funktioner för windows ----------------------------------------------------
LRESULT		CALLBACK	winProc(HWND, UINT, WPARAM, LPARAM);
ATOM 		doRegister(HINSTANCE);
BOOL 		initInstance(HINSTANCE, int);
int			initalizeAll(HWND);
void		releaseAll(HWND);
void		update();				// Alla uppdateringar
void		render();				// Ritar ut bilden i f�nstret
bool		framerate(int);			// Uppdateringsfrekvensen
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
		MessageBox(NULL, (LPCSTR)"Fel, Kan ej registrera eller skapa f�nstret i windows", (LPCSTR)"ERROR", MB_ICONERROR | MB_OK);
		return 0;
	}
	MSG msg;

	// Ny loop d�r vi sj�lv best�mmer n�r saker ritas ut
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
	std::string output;


	switch (Msg) {
	case WM_CREATE:
		initalizeAll(hWnd);
		break;
	case WM_LBUTTONDOWN:
		makeExplosion(lParam);
		collision();
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		running = false;
		break;
	case WM_KEYDOWN:
		if (rand() % 69 == 0) {
			createMonster();
		}
		if (wParam == VK_RIGHT) {
			runRight(langd);
		}
		if (wParam == VK_LEFT) {
			runLeft(langd);
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
void createMonster() { // skapar ett monster
	enemie tmp;
	//	random position
	x = (rand() % app_Wid) - (tmp.width / 2);
	y = (rand() % app_Hei) - (tmp.height / 2);
	// spawna inte inom 400 pixlar runt spelaren
	while (x < app_Wid / 2 + 200 && x > app_Wid / 2 - 200) {
		x = (rand() % app_Wid) - (tmp.width / 2);
	}
	tmp.x = x;
	tmp.y = y;
	monst.push_back(tmp);
	monstLangd = monst.size();
}	
//---------------------------------------------------------------------------
void controllMonster() {
	for (int n = 0; n < monstLangd; n++) {
		if (monst[n].x - playerX < 0) {
			// gå till höger
			monst[n].runR = true;
			monst[n].runL = false;
			monst[n].x++;
		}
		else {
			// gå till vänster
			monst[n].runR = false;
			monst[n].runL = true;
			monst[n].x--;
		}
		if (monst[n].y - playerY > 0) {
			// gå neråt
			monst[n].y--;
		}
		else {
			// gå uppåt
			monst[n].y++;
		}
		if (monst[n].runL == true) {
			monst[n].cY = 0;
			monst[n].cX += 24;
			if (monst[n].cX > 263) {
				monst[n].cX = 0;
			}
		}
		else if (monst[n].runR == true) {
			monst[n].cY = 24;
			monst[n].cX += 24;
			if (monst[n].cX > 263) {
				monst[n].cX = 0;
			}
		}
	}

}	// kontrollerar monster
//---------------------------------------------------------------------------
void monsterKill() {
	for (int n = 0; n < monstLangd; n++) {
		if (monst[n].x > playerX && monst[n].x < playerX + 72) {
			dead = true;
			monst.clear();
			monstLangd = monst.size();
		}
	}
}
//---------------------------------------------------------------------------
void collision() {
	for (int n = 0; n < monstLangd; n++) {		// går igenom alla monster
		if (x > monst[n].x - monst[n].width && x < monst[n].x && y < monst[n].y + monst[n].height / 2 && y > monst[n].y - monst[n].height / 2) {
			monst.erase(monst.begin() + n);		// dödar den monstret som är på samma plats som musen när man klickar;
			monstLangd = monst.size();
			score(); 
		}
	}
}
//---------------------------------------------------------------------------
void score() {
	points++; 
}
//---------------------------------------------------------------------------
void makeExplosion(LPARAM lParam) {
	booms tmp;
	x = LOWORD(lParam) - (tmp.width / 2);
	y = HIWORD(lParam) - (tmp.height / 2);
	tmp.x = x;
	tmp.y = y;
	exps.push_back(tmp);
	expLangd = exps.size();
}
//---------------------------------------------------------------------------
void runLeft(int langd) {
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
//---------------------------------------------------------------------------
void runRight(int langd) {
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
//---------------------------------------------------------------------------
void update() {
	static int counter = 0;
	counter++;
	//	int langd = exps.size() - 1;
	if (counter % 7 == 0) {
		// monster animation
		for (int n = 0; n < monstLangd; n++) {
			controllMonster();
			monsterKill();
		}
		// background animation
		bgs[6].x -= 1;
		if (bgs[6].x < -1920 / 2) {
			bgs[6].x = 0;
		}
		// player animation
		// springer eller st�r till h�ger
		playerAnimation();
	}
	// explosion animation
	if (counter % 3 == 0) {
		for (int n = 0; n < expLangd; n++) {
			exps[n].cX += 320;
			if (exps[n].cX > 1000) {
				exps[n].cX = 0;
				exps[n].cY += 240;
			}
			if (exps[n].cY > 1000) {
				exps[n].cX = 0;
				exps[n].cY = 0;
				exps.erase(exps.begin());
				expLangd = exps.size();
			}
		}
	}
}
//---------------------------------------------------------------------------
void playerAnimation() {
	if (runR == true) {
		ypic = 288;
		xpic -= 144;
		if (xpic < 1) {
			xpic = 720;
		}
	}
	else if (idleR == true) {
		if (xpic < 144) {
			xpic = 144;
		}
		ypic = 0;
		xpic += 144;
		if (xpic > 863) {
			xpic = 144;
		}
	}
	// springer eller st�r till v�nster
	if (runL == true) {
		ypic = 288;
		xpic += 144;
		if (xpic > 863) {
			xpic = 0;
		}
	}
	else if (idleL == true) {
		if (xpic > 576) {
			xpic = 576;
		}
		ypic = 0;
		xpic -= 144;
		if (xpic < 0) {
			xpic = 576;
		}
	}
}
//---------------------------------------------------------------------------
void render() {
	static bool yes = true;

	int langd = bgs.size() - 1;

	for (int n = langd; n >= 0; n--) {
		// background
		TransparentBlt(bufferHDC, bgs[n].x, 0, app_Wid, app_Hei, bgs[n].hDCbg, 0, 0, bg_Wid, bg_Hei, COLORREF(RGB(255, 0, 255)));

		if (bgs[n].x < 0) {		// rörelse till höger
			TransparentBlt(bufferHDC, app_Wid + bgs[n].x, 0, -bgs[n].x, app_Hei, bgs[n].hDCbg, 0, 0, -bgs[n].x * 2, bg_Hei, COLORREF(RGB(255, 0, 255)));
		}
		else if (bgs[n].x > 0) { // rörelse till vänster
			TransparentBlt(bufferHDC, bgs[n].x - app_Wid, 0, app_Wid, app_Hei, bgs[n].hDCbg, 0, 0, bg_Wid, bg_Hei, COLORREF(RGB(255, 0, 255)));
		}
		if (n == 2) {
			// player
			if (idleR == true) {
				TransparentBlt(bufferHDC, playerX, playerY, 144, 144, playerHDC[0], xpic, ypic, 144, 144, COLORREF(RGB(255, 0, 255)));
			}
			else if (idleL == true) {
				TransparentBlt(bufferHDC, playerX, playerY, 144, 144, playerHDC[1], xpic, ypic, 144, 144, COLORREF(RGB(255, 0, 255)));
			}
		}
		// monster
		for (int n = 0; n < monstLangd; n++) {
			TransparentBlt(bufferHDC, monst[n].x, monst[n].y, monst[n].width, monst[n].height, monsterHDC, monst[n].cX, monst[n].cY, 24, 24, COLORREF(RGB(255, 0, 255)));
		}
		// explosions
		for (int n = 0; n < expLangd; n++) {
			TransparentBlt(bufferHDC, exps[n].x, exps[n].y, exps[n].width, exps[n].height, expHDC, exps[n].cX, exps[n].cY, 320, 240, COLORREF(RGB(255, 0, 255)));
		}
	}

	std::string output = std::to_string(monst.size()) + " " + std::to_string(dead) + " " + std::to_string(points);
	TextOut(bufferHDC, 0, 0, output.c_str(), output.length());
	//dubbelbuff
	BitBlt(hDC, 0, 0, innerWidth, innerHeight, bufferHDC, 0, 0, SRCCOPY);
}
//---------------------------------------------------------------------------
int	initalizeAll(HWND hWnd) {
	srand(time(NULL));
	// H�mta f�nstrets riktiga bredd & h�jd
	RECT		windowRect;
	GetClientRect(hWnd, &windowRect);
	innerWidth = windowRect.right;
	innerHeight = windowRect.bottom;

	hDC = GetDC(hWnd);						// Koppla f�nstret till en DC
	bg tmp;
	std::string text;
	for (unsigned int n = 0; n < 8; n++) {	// l�ser in HDC och bilder till bgs
		tmp.hDCbg = CreateCompatibleDC(hDC);
		text = "bilder/layer_0" + std::to_string(n + 1) + ".bmp";
		tmp.parallax = (HBITMAP)LoadImage(NULL, (LPCTSTR)(text.c_str()), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		if (n > 1)tmp.m = 8 - n;
		bgs.push_back(tmp);
		oldBitmap[n] = (HBITMAP)SelectObject(bgs[n].hDCbg, bgs[n].parallax);
	}
	// L�ser in spelaren
	for (int n = 0; n < 2; n++) {
		playerHDC[n] = CreateCompatibleDC(hDC);	// Skapa en hdc f�r spelaren
		text = "bilder/hero" + std::to_string(n + 1) + ".bmp";
		player[n] = (HBITMAP)LoadImage(NULL, (LPCTSTR)(text.c_str()), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		oldBitmap[8 + n] = (HBITMAP)SelectObject(playerHDC[n], player[n]);
	}
	// explosioner
	expHDC = CreateCompatibleDC(hDC);	// Skapa en hdc f�r explosioner
	explosion = (HBITMAP)LoadImage(NULL, (LPCTSTR)("bilder/explosion.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	oldBitmap[10] = (HBITMAP)SelectObject(expHDC, explosion);

	// monster
	monsterHDC = CreateCompatibleDC(hDC);	// Skapa en hdc f�r monster
	monsters = (HBITMAP)LoadImage(NULL, (LPCTSTR)("bilder/goodmonster.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	oldBitmap[11] = (HBITMAP)SelectObject(monsterHDC, monsters);

	// Buffer
	bufferHDC = CreateCompatibleDC(hDC);				// Skapa en hdc f�r bakgrundsbilden
	bitmapbuff = CreateCompatibleBitmap(hDC, innerWidth, innerHeight);
	SelectObject(bufferHDC, bitmapbuff);
	//Ger CPU-frekvensen som anv�nds med performanceCounter();
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
	for (unsigned int n = 0; n < 2; n++) {
		// tar bort hdc för players
		SelectObject(playerHDC[n], oldBitmap[8 + n]);
		DeleteObject(player[n]);
		ReleaseDC(hWnd, playerHDC[n]);
		DeleteDC(playerHDC[n]);
	}
	SelectObject(expHDC, oldBitmap[10]);
	DeleteObject(explosion);
	ReleaseDC(hWnd, expHDC);
	DeleteDC(expHDC);

	SelectObject(monsterHDC, oldBitmap[11]);
	DeleteObject(monsters);
	ReleaseDC(hWnd, monsterHDC);
	DeleteDC(monsterHDC);

	//Ta bort hdc till f�nstret och imageHDC
	ReleaseDC(hWnd, hDC);
	DeleteDC(hDC);
}
//---------------------------------------------------------------------------
BOOL initInstance(HINSTANCE hInstance, int nCmdShow) {
	//Bredd och h�jd f�r f�nstret som vi skapar

	HWND hWnd = CreateWindowEx(
		0, ClsName, (LPCSTR)ClsName, WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_BORDER,
		((GetSystemMetrics(SM_CXSCREEN) - app_Wid) >> 1),  //S�tter f�nstret i mitten i x-led;
		((GetSystemMetrics(SM_CYSCREEN) - app_Hei) >> 1),  //S�tter f�nstret i mitten i Y-led;
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