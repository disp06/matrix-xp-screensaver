// Matrix XP Screensaver - Super Matrix Style
// Compile: x86_64-w64-mingw32-g++ -mwindows -O2 -Wall src/main.cpp -o matrix.scr -lgdi32 -luser32

#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <string.h>
#include <cstdio>

// Matrix characters: katakana + digits + latin
const char* MATRIX_CHARS = "ﾊﾐﾋｰｳｼﾅﾓﾆｻﾜﾂｵﾘｱﾎﾃﾏｹﾒｴｶｷﾑﾕﾗｾﾈｽﾀﾇﾍ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

const int CHAR_W = 12;
const int CHAR_H = 18;
const int TRAIL_MAX = 35;
const int MIN_SPEED = 1;
const int MAX_SPEED = 4;

struct Drop {
    int x;
    int y;
    int speed;
    int length;
    int chars[60]; // stored characters
    bool glitch;
};

Drop* drops = NULL;
int cols = 0, rows = 0, numDrops = 0;
bool systemOverload = false;
bool glitchMode = false;
HDC memDC = NULL;
HBITMAP memBmp = NULL;
HFONT font = NULL;

void InitDrops() {
    if (drops) delete[] drops;
    drops = new Drop[numDrops];
    for (int i = 0; i < numDrops; i++) {
        drops[i].x = i * CHAR_W + (rand() % 10 - 5);
        drops[i].y = rand() % (rows * CHAR_H) - rows * CHAR_H;
        drops[i].speed = MIN_SPEED + rand() % (MAX_SPEED - MIN_SPEED + 1);
        drops[i].length = 15 + rand() % 20;
        drops[i].glitch = false;
        for (int j = 0; j < 60; j++) {
            drops[i].chars[j] = rand() % strlen(MATRIX_CHARS);
        }
    }
}

COLORREF GetTrailColor(int alpha) {
    if (alpha < 0) alpha = 0;
    if (alpha > 255) alpha = 255;
    int g = alpha;
    if (g > 200) g = 200 + (g - 200) / 3;
    return RGB(0, g, 0);
}

void DrawFrame(HDC hdc) {
    // Clear with black
    RECT rc = {0, 0, cols * CHAR_W, rows * CHAR_H};
    HBRUSH black = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &rc, black);
    DeleteObject(black);

    if (!drops) return;

    for (int i = 0; i < numDrops; i++) {
        Drop& d = drops[i];
        
        // Draw trail
        for (int j = 0; j < d.length; j++) {
            int py = d.y - j * CHAR_H;
            if (py < -CHAR_H || py > rows * CHAR_H) continue;
            
            // Brightness based on position in trail
            int brightness = 255 * (d.length - j) / d.length;
            if (j < 3) brightness = 255; // bright head
            else if (j < 6) brightness = 180 + rand() % 75;
            
            SetTextColor(hdc, GetTrailColor(brightness));
            SetBkMode(hdc, TRANSPARENT);
            
            // Character for this trail segment
            char ch[2] = {MATRIX_CHARS[d.chars[j % 60]], 0};
            
            // Occasional character change (flicker effect)
            if (rand() % 100 < 3) {
                d.chars[j % 60] = rand() % strlen(MATRIX_CHARS);
                ch[0] = MATRIX_CHARS[d.chars[j % 60]];
            }
            
            TextOutA(hdc, d.x, py, ch, 1);
        }
        
        // Move drop
        d.y += d.speed;
        
        // Reset when off screen
        if (d.y - d.length * CHAR_H > rows * CHAR_H) {
            d.y = rand() % -200 - 50;
            d.speed = MIN_SPEED + rand() % (MAX_SPEED - MIN_SPEED + 1);
            d.length = 15 + rand() % 20;
            for (int j = 0; j < 60; j++) d.chars[j] = rand() % strlen(MATRIX_CHARS);
        }
        
        // Random glitch
        if (rand() % 1000 < 2) d.glitch = true;
    }
    
    // SYSTEM OVERLOAD mode
    if (systemOverload && rand() % 100 < 15) {
        const char* msg = "SYSTEM OVERRIDE";
        SetTextColor(hdc, RGB(0, 255, 0));
        int mx = rand() % (cols * CHAR_W - 200);
        int my = rand() % (rows * CHAR_H - 50);
        TextOutA(hdc, mx, my, msg, strlen(msg));
        
        // Glitch lines
        for (int k = 0; k < 3; k++) {
            int gy = my + 20 + rand() % 30;
            int gw = 50 + rand() % 150;
            RECT glitchRect = {mx, gy, mx + gw, gy + 2};
            HBRUSH green = CreateSolidBrush(RGB(0, 50, 0));
            FillRect(hdc, &glitchRect, green);
            DeleteObject(green);
        }
    }
}

void Cleanup() {
    if (memDC) {
        if (memBmp) DeleteObject(memBmp);
        DeleteDC(memDC);
    }
    if (font) DeleteObject(font);
    if (drops) delete[] drops;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE:
            cols = GetSystemMetrics(SM_CXSCREEN) / CHAR_W;
            rows = GetSystemMetrics(SM_CYSCREEN) / CHAR_H;
            numDrops = cols;
            
            // Create memory DC for double buffering
            memDC = CreateCompatibleDC(GetDC(hwnd));
            memBmp = CreateCompatibleBitmap(GetDC(hwnd), cols * CHAR_W, rows * CHAR_H);
            SelectObject(memDC, memBmp);
            
            // Create font
            font = CreateFont(CHAR_H, CHAR_W - 2, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           PROOF_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
            SelectObject(memDC, font);
            
            srand(GetTickCount());
            InitDrops();
            SetTimer(hwnd, 1, 40, NULL);
            return 0;
            
        case WM_TIMER:
            DrawFrame(memDC);
            BitBlt(GetDC(hwnd), 0, 0, cols * CHAR_W, rows * CHAR_H, memDC, 0, 0, SRCCOPY);
            return 0;
            
        case WM_KEYDOWN:
            if (wp == VK_ESCAPE) PostMessage(hwnd, WM_CLOSE, 0, 0);
            if (wp == 'E') systemOverload = !systemOverload;
            if (wp == 'G') glitchMode = !glitchMode;
            return 0;
            
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            Cleanup();
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE h, HINSTANCE p, LPSTR cmd, int show) {
    // Screensaver mode: /s or empty
    if (strstr(cmd, "/s") || cmd[0] == 0) {
        WNDCLASS wc = {0};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = h;
        wc.lpszClassName = "MatrixXP";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        RegisterClass(&wc);
        
        int sw = GetSystemMetrics(SM_CXSCREEN);
        int sh = GetSystemMetrics(SM_CYSCREEN);
        
        HWND hw = CreateWindowEx(WS_EX_TOPMOST, "MatrixXP", "Matrix XP", WS_POPUP, 0, 0, sw, sh, NULL, NULL, h, NULL);
        ShowWindow(hw, SW_SHOW);
        SetWindowPos(hw, HWND_TOPMOST, 0, 0, sw, sh, SWP_SHOWWINDOW);
        
        MSG m;
        while (GetMessage(&m, NULL, 0, 0)) {
            TranslateMessage(&m);
            DispatchMessage(&m);
        }
        return 0;
    }
    
    // Preview mode: /p <hwnd>
    if (strstr(cmd, "/p")) {
        // Get window handle from command line
        HWND parent = NULL;
        sscanf(cmd + 2, "%lu", (ULONG*)&parent);
        if (parent) {
            RECT pr;
            GetClientRect(parent, &pr);
            int pw = pr.right - pr.left;
            int ph = pr.bottom - pr.top;
            
            WNDCLASS wc = {0};
            wc.lpfnWndProc = WndProc;
            wc.hInstance = h;
            wc.lpszClassName = "MatrixXPPreview";
            RegisterClass(&wc);
            
            HWND hw = CreateWindowEx(0, "MatrixXPPreview", "Preview", WS_CHILD, 0, 0, pw, ph, parent, NULL, h, NULL);
            ShowWindow(hw, SW_SHOW);
            
            MSG m;
            while (GetMessage(&m, NULL, 0, 0) && IsWindow(hw)) {
                TranslateMessage(&m);
                DispatchMessage(&m);
            }
        }
        return 0;
    }
    
    // Config mode: /c
    MessageBox(NULL, "Matrix XP Screensaver\n\nPress E for SYSTEM OVERLOAD\nPress G for glitch mode\nESC to exit", "Matrix XP", MB_OK);
    return 0;
}