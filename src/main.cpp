#include <windows.h>
#include <gdi.h>
#include <cstdlib>
#include <ctime>
#include <string.h>
const char* KATAKANA = "ﾊﾐﾋｰｳｼﾅﾓﾆｻﾜﾂｵﾘｱﾎﾃﾏｹﾒｴｶｷﾑﾕﾗｾﾈｽﾀﾇﾍ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const int CHAR_WIDTH = 8;
const int CHAR_HEIGHT = 16;
const int COLUMNS = 80;
struct Drop { int x, y, speed, length; COLORREF color; };
Drop drops[COLUMNS]; int screenW=0, screenH=0; bool explodeMode=false;
void InitDrops() { for(int i=0;i<COLUMNS;i++) { drops[i].x=i*CHAR_WIDTH; drops[i].y=rand()%-screenH; drops[i].speed=2+rand()%6; drops[i].length=10+rand()%30; drops[i].color=RGB(0,255-rand()%100,0); } }
void DrawScene(HDC hdc) {
    RECT rc={0,0,screenW,screenH}; HBRUSH black=CreateSolidBrush(RGB(0,0,0)); FillRect(hdc,&rc,black); DeleteObject(black);
    for(int i=0;i<COLUMNS;i++) {
        int head=drops[i].y;
        for(int j=0;j<drops[i].length;j++) {
            int py=head-j*CHAR_HEIGHT; if(py<0||py>screenH) continue;
            COLORREF col=(j<3)?RGB(200,255,200):drops[i].color;
            SetTextColor(hdc,col); SetBkMode(hdc,TRANSPARENT);
            TextOutA(hdc,drops[i].x,py,&KATAKANA[rand()%strlen(KATAKANA)],1);
        }
        drops[i].y+=drops[i].speed;
        if(drops[i].y-drops[i].length*CHAR_HEIGHT>screenH) { drops[i].y=rand()%-200; drops[i].speed=2+rand()%6; }
    }
    if(explodeMode && rand()%100<5) {
        const char* txt="SYSTEM OVERLOAD";
        SetTextColor(hdc,RGB(0,255,0));
        int tx=rand()%(screenW-200), ty=rand()%(screenH-50);
        TextOutA(hdc,tx,ty,txt,strlen(txt));
    }
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: screenW=GetSystemMetrics(SM_CXSCREEN); screenH=GetSystemMetrics(SM_CYSCREEN); SetTimer(hwnd,1,30,NULL); srand(GetTickCount()); InitDrops(); return 0;
        case WM_TIMER: InvalidateRect(hwnd,NULL,TRUE); return 0;
        case WM_KEYDOWN: if(wParam==VK_ESCAPE) PostMessage(hwnd,WM_CLOSE,0,0); if(wParam=='E') explodeMode=!explodeMode; return 0;
        case WM_DESTROY: KillTimer(hwnd,1); PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hwnd,msg,wParam,lParam);
}
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nShow) {
    if(strstr(lpCmdLine,"/s") || lpCmdLine[0]=='\0') {
        WNDCLASS wc={0}; wc.lpfnWndProc=WndProc; wc.hInstance=hInst; wc.lpszClassName="MatrixXP"; wc.hCursor=LoadCursor(NULL,IDC_ARROW);
        RegisterClass(&wc);
        screenW=GetSystemMetrics(SM_CXSCREEN); screenH=GetSystemMetrics(SM_CYSCREEN);
        HWND hwnd=CreateWindowEx(WS_EX_TOPMOST|WS_EX_TOOLWINDOW,"MatrixXP","Matrix XP",WS_POPUP,0,0,screenW,screenH,NULL,NULL,hInst,NULL);
        ShowWindow(hwnd,SW_SHOW); SetWindowPos(hwnd,HWND_TOPMOST,0,0,screenW,screenH,SWP_SHOWWINDOW);
        MSG msg; while(GetMessage(&msg,NULL,0,0)) { TranslateMessage(&msg); DispatchMessage(&msg); }
        return 0;
    }
    MessageBox(NULL,"No options","Matrix XP",MB_OK); return 0;
}
