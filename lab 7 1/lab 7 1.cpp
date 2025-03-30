#define _USE_MATH_DEFINES
#include "framework.h"
#include "lab 7 1.h"
#include <cmath>
#include <thread>
#include <Windows.h>
#include <vector>
#include <mutex>

using namespace std;

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
vector<thread> threads;
mutex DrawMutex;
bool running = true;

struct ThreadData {
    class Figure* figure;
    HWND hWnd;
};

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void ThreadFunction(ThreadData* data);

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAB71));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_LAB71);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

class Figure {
protected:
    int x, y;
    int R;
    int V;
    int Ang;
    int VAng;
    int Napr;
    COLORREF col;
    HWND hWnd;
    int N_Reg;
public:
    Figure(int R, int VAng, int V, int Napr, COLORREF col, HWND hWnd);
    void virtual step();
    void virtual draw(HDC hdc, int Reg) = 0;
};

Figure::Figure(int R, int VAng, int V, int Napr, COLORREF col, HWND hWnd) {
    this->R = R;
    this->VAng = VAng;
    this->V = V;
    this->col = col;
    this->hWnd = hWnd;
    this->Napr = Napr;
    Ang = 0;
    N_Reg = 1;
    RECT rect;
    GetClientRect(hWnd, &rect);
    x = rect.right / 2;
    y = rect.bottom / 2;
}

void Figure::step() {
    Ang += VAng;
    if (Ang >= 360) {
        Ang -= 360;
    }
    RECT rect;
    GetClientRect(hWnd, &rect);
    if (Napr == 1) {
        x += V * N_Reg;
        if (N_Reg == 1) {
            if (x + R >= rect.right) {
                N_Reg = -1;
            }
        }
        else {
            if (x - R <= 0) {
                N_Reg = 1;
            }
        }
    }
    else {
        y += V * N_Reg;
        if (N_Reg == 1) {
            if (y + R >= rect.bottom) {
                N_Reg = -1;
            }
        }
        else {
            if (y - R <= 0) {
                N_Reg = 1;
            }
        }
    }
}

class MyOtrezok : public Figure {
protected:
    int x1;
    int y1;
    int x2;
    int y2;
public:
    MyOtrezok(int R, int VAng, int V, int Napr, COLORREF col, HWND hWnd);
    void step();
    void draw(HDC hdc, int Reg);
};

MyOtrezok::MyOtrezok(int R, int VAng, int V, int Napr, COLORREF col, HWND hWnd) : Figure(R, VAng, V, Napr, col, hWnd) {
    double A = Ang * M_PI / 180;
    x1 = x + R * cos(A);
    y1 = y - R * sin(A);
    x2 = x - R * cos(A);
    y2 = y + R * sin(A);
}

void MyOtrezok::step() {
    Figure::step();
    double A = Ang * M_PI / 180;
    x1 = x + R * cos(A);
    y1 = y - R * sin(A);
    x2 = x - R * cos(A);
    y2 = y + R * sin(A);
}

void MyOtrezok::draw(HDC hdc, int Reg) {
    HPEN pen;
    if (Reg == 1) {
        pen = CreatePen(PS_SOLID, 1, col);
    }
    else {
        pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    }
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    MoveToEx(hdc, x1, y1, NULL);
    LineTo(hdc, x2, y2);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

class MyTriangle : public Figure {
protected:
    POINT p[3];
public:
    MyTriangle(int R, int VAng, int V, int Narp, COLORREF col, HWND hWnd);
    void step();
    void draw(HDC hdc, int Reg);
};

MyTriangle::MyTriangle(int R, int VAng, int V, int Napr, COLORREF col, HWND hWnd) : Figure(R, VAng, V, Napr, col, hWnd) {
    double A = Ang * M_PI / 180;
    p[0].x = x + R * cos(A);
    p[0].y = y - R * sin(A);
    p[1].x = x + R * cos(A + 3 * M_PI / 4);
    p[1].y = y - R * sin(A + 3 * M_PI / 4);
    p[2].x = x + R * cos(A + 5 * M_PI / 4);
    p[2].y = y - R * sin(A + 5 * M_PI / 4);
}

void MyTriangle::step() {
    Figure::step();
    double A = Ang * M_PI / 180;
    p[0].x = x + R * cos(A);
    p[0].y = y - R * sin(A);
    p[1].x = x + R * cos(A + 3 * M_PI / 4);
    p[1].y = y - R * sin(A + 3 * M_PI / 4);
    p[2].x = x + R * cos(A + 5 * M_PI / 4);
    p[2].y = y - R * sin(A + 5 * M_PI / 4);
}

void MyTriangle::draw(HDC hdc, int Reg) {
    HPEN pen;
    if (Reg == 1) {
        pen = CreatePen(PS_SOLID, 1, col);
    }
    else {
        pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    }
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    MoveToEx(hdc, p[0].x, p[0].y, NULL);
    LineTo(hdc, p[1].x, p[1].y);
    LineTo(hdc, p[2].x, p[2].y);
    LineTo(hdc, p[0].x, p[0].y);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

class MyParallelogram : public Figure {
protected:
    POINT p[4];
public:
    MyParallelogram(int R, int VAng, int V, int Napr, COLORREF col, HWND hWnd);
    void step();
    void draw(HDC hdc, int Reg);
};

MyParallelogram::MyParallelogram(int R, int VAng, int V, int Napr, COLORREF col, HWND hWnd) : Figure(R, VAng, V, Napr, col, hWnd) {
    double A = Ang * M_PI / 180;
    p[0].x = x + R * cos(A);
    p[0].y = y - R * sin(A);
    p[1].x = x + 0.7 * R * cos(A + M_PI / 3);
    p[1].y = y - 0.7 * R * sin(A + M_PI / 3);
    p[2].x = x + 0.5 * R * cos(A + M_PI);
    p[2].y = y - 0.5 * R * sin(A + M_PI);
    p[3].x = x + 0.8 * R * cos(A + 4 * M_PI / 3);
    p[3].y = y - 0.8 * R * sin(A + 4 * M_PI / 3);
}

void MyParallelogram::step() {
    Figure::step();
    double A = Ang * M_PI / 180;
    p[0].x = x + R * cos(A);
    p[0].y = y - R * sin(A);
    p[1].x = x + 0.7 * R * cos(A + M_PI / 3);
    p[1].y = y - 0.7 * R * sin(A + M_PI / 3);
    p[2].x = x + 0.5 * R * cos(A + M_PI);
    p[2].y = y - 0.5 * R * sin(A + M_PI);
    p[3].x = x + 0.8 * R * cos(A + 4 * M_PI / 3);
    p[3].y = y - 0.8 * R * sin(A + 4 * M_PI / 3);
}

void MyParallelogram::draw(HDC hdc, int Reg) {
    HPEN pen;
    if (Reg == 1) {
        pen = CreatePen(PS_SOLID, 1, col);
    }
    else {
        pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    }
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    MoveToEx(hdc, p[0].x, p[0].y, NULL);
    LineTo(hdc, p[1].x, p[1].y);
    LineTo(hdc, p[2].x, p[2].y);
    LineTo(hdc, p[3].x, p[3].y);
    LineTo(hdc, p[0].x, p[0].y);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

Figure* pF[9];

void ThreadFunction(ThreadData* data) {
    while (running) {
        data->figure->step();
        {
            lock_guard<mutex> lock(DrawMutex);
            HDC hdc = GetDC(data->hWnd);

            HDC hdcMem = CreateCompatibleDC(hdc);
            RECT rect;
            GetClientRect(data->hWnd, &rect);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

            FillRect(hdcMem, &rect, (HBRUSH)(COLOR_WINDOW + 1));

            for (int i = 0; i < 9; i++) {
                if (pF[i]) {
                    pF[i]->draw(hdcMem, 1);
                }
            }

            BitBlt(hdc, 0, 0, rect.right, rect.bottom, hdcMem, 0, 0, SRCCOPY);

            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);
            ReleaseDC(data->hWnd, hdc);
        }
        this_thread::sleep_for(chrono::milliseconds(16));
    }
    delete data;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, 1, 1, NULL);
        pF[0] = new MyOtrezok(50, 3, 5, 0, RGB(255, 0, 0), hWnd);
        pF[1] = new MyOtrezok(80, -2, 3, 1, RGB(0, 255, 0), hWnd);
        pF[2] = new MyOtrezok(120, 5, 7, 0, RGB(0, 0, 255), hWnd);

        pF[3] = new MyTriangle(60, -4, 4, 1, RGB(255, 0, 255), hWnd);
        pF[4] = new MyTriangle(90, 2, 6, 0, RGB(255, 255, 0), hWnd);
        pF[5] = new MyTriangle(110, -3, 2, 1, RGB(0, 255, 255), hWnd);

        pF[6] = new MyParallelogram(70, 3, 5, 0, RGB(128, 0, 128), hWnd);
        pF[7] = new MyParallelogram(100, -2, 3, 1, RGB(0, 128, 128), hWnd);
        pF[8] = new MyParallelogram(130, 4, 4, 0, RGB(128, 128, 0), hWnd);

        for (int i = 0; i < 9; i++) {
            ThreadData* data = new ThreadData{
                pF[i], hWnd
            };
            threads.emplace_back(ThreadFunction, data);
        }
        break;

    case WM_DESTROY:
        running = false;
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        for (int i = 0; i < 9; i++) {
            delete pF[i];
        }
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LAB71, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB71));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}