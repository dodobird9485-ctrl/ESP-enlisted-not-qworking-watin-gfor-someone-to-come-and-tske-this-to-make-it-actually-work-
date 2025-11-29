#include <Windows.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cmath>
#include "memory_reader.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "gdi32.lib")

const wchar_t CLASS_NAME[] = L"EnlistedESPOverlay";
const wchar_t WINDOW_NAME[] = L"Enlisted ESP Overlay";

MemoryReader* g_memoryReader = nullptr;
HWND g_gameWindow = nullptr;
HWND g_overlayWindow = nullptr;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            if (wparam == VK_ESCAPE) {
                PostQuitMessage(0);
            }
            break;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

HWND FindGameWindow() {
    HWND hwnd = FindWindowW(nullptr, L"Enlisted");
    if (!hwnd) {
        hwnd = FindWindowW(L"UnrealWindow", nullptr);
    }
    return hwnd;
}

void DrawFilledBox(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color) {
    HBRUSH hBrush = CreateSolidBrush(color);
    RECT rect = { x1, y1, x2, y2 };
    FrameRect(hdc, &rect, hBrush);
    DeleteObject(hBrush);
}

void DrawHealthBar(HDC hdc, int x, int y, float health) {
    int barWidth = 30;
    int barHeight = 3;
    
    HBRUSH bgBrush = CreateSolidBrush(RGB(64, 0, 0));
    RECT bgRect = { x - barWidth/2, y + 35, x + barWidth/2, y + 35 + barHeight };
    FillRect(hdc, &bgRect, bgBrush);
    DeleteObject(bgBrush);
    
    int healthWidth = (int)(barWidth * (health / 100.0f));
    COLORREF healthColor = health > 50 ? RGB(0, 255, 0) : (health > 25 ? RGB(255, 255, 0) : RGB(255, 0, 0));
    HBRUSH healthBrush = CreateSolidBrush(healthColor);
    RECT healthRect = { x - barWidth/2, y + 35, x - barWidth/2 + healthWidth, y + 35 + barHeight };
    FillRect(hdc, &healthRect, healthBrush);
    DeleteObject(healthBrush);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    std::cout << "[*] Enlisted ESP Overlay Starting..." << std::endl;

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = CLASS_NAME;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.style = CS_HREDRAW | CS_VREDRAW;

    RegisterClassW(&wc);

    g_overlayWindow = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        CLASS_NAME,
        WINDOW_NAME,
        WS_POPUP | WS_VISIBLE,
        0, 0, 1920, 1080,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!g_overlayWindow) {
        std::cerr << "[-] Failed to create overlay window" << std::endl;
        return 1;
    }

    SetLayeredWindowAttributes(g_overlayWindow, RGB(0, 0, 0), 0, LWA_COLORKEY);

    g_gameWindow = FindGameWindow();
    if (g_gameWindow) {
        std::cout << "[+] Found Enlisted game window" << std::endl;
        RECT gameRect;
        GetWindowRect(g_gameWindow, &gameRect);
        SetWindowPos(g_overlayWindow, HWND_TOPMOST,
                     gameRect.left, gameRect.top,
                     gameRect.right - gameRect.left,
                     gameRect.bottom - gameRect.top,
                     SWP_NOACTIVATE);
    } else {
        std::cout << "[*] Enlisted not found, using full screen" << std::endl;
    }

    g_memoryReader = new MemoryReader();
    if (!g_memoryReader->Initialize("Enlisted.exe")) {
        std::cerr << "[-] Failed to initialize memory reader" << std::endl;
    }

    std::cout << "[+] Overlay initialized. Press ESC to exit." << std::endl;

    std::vector<Player> testPlayers;
    testPlayers.push_back({
        { 960, 540, 50 },
        { 960, 590, 50 },
        "Enemy1",
        75.0f,
        false,
        true,
        85.5f
    });
    testPlayers.push_back({
        { 100, 100, 50 },
        { 100, 150, 50 },
        "Ally1",
        100.0f,
        true,
        true,
        125.3f
    });
    testPlayers.push_back({
        { 1200, 300, 50 },
        { 1200, 350, 50 },
        "Enemy2",
        45.0f,
        false,
        true,
        200.7f
    });

    MSG msg = {};
    bool running = true;
    int frameCount = 0;

    while (running) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        if (g_gameWindow && !IsWindow(g_gameWindow)) {
            std::cerr << "[-] Game window closed" << std::endl;
            break;
        }

        HDC hdc = GetDC(g_overlayWindow);
        if (hdc) {
            RECT rect;
            GetClientRect(g_overlayWindow, &rect);
            
            FillRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
            
            auto players = g_memoryReader->ReadPlayers();
            if (players.empty()) {
                players = testPlayers;
            }
            
            int centerX = rect.right / 2;
            int centerY = rect.bottom / 2;
            
            HPEN crossPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
            HPEN oldPen = (HPEN)SelectObject(hdc, crossPen);
            MoveToEx(hdc, centerX - 10, centerY, nullptr);
            LineTo(hdc, centerX + 10, centerY);
            MoveToEx(hdc, centerX, centerY - 10, nullptr);
            LineTo(hdc, centerX, centerY + 10);
            SelectObject(hdc, oldPen);
            DeleteObject(crossPen);
            
            for (const auto& player : players) {
                int x = (int)player.position.x;
                int y = (int)player.position.y;
                int w = 30;
                int h = 40;
                
                COLORREF color = player.isAlly ? RGB(0, 200, 0) : RGB(255, 50, 50);
                
                HPEN hPen = CreatePen(PS_SOLID, 2, color);
                HPEN oldBoxPen = (HPEN)SelectObject(hdc, hPen);
                HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
                
                Rectangle(hdc, x - w/2, y - h/2, x + w/2, y + h/2);
                
                int distance = (int)player.distance;
                char label[256];
                snprintf(label, sizeof(label), "%s [%dm] HP:%d", player.name.c_str(), distance, (int)player.health);
                
                SetTextColor(hdc, color);
                SetBkMode(hdc, TRANSPARENT);
                TextOutA(hdc, x - w/2 - 10, y - h/2 - 25, label, (int)strlen(label));
                
                DrawHealthBar(hdc, x, y, player.health);
                
                SelectObject(hdc, oldBrush);
                SelectObject(hdc, oldBoxPen);
                DeleteObject(hPen);
            }
            
            ReleaseDC(g_overlayWindow, hdc);
        }

        frameCount++;
        if (frameCount % 60 == 0) {
            std::cout << "[*] Frames: " << frameCount << std::endl;
        }

        Sleep(16);
    }

    if (g_memoryReader) {
        g_memoryReader->Shutdown();
        delete g_memoryReader;
        g_memoryReader = nullptr;
    }

    DestroyWindow(g_overlayWindow);
    UnregisterClassW(CLASS_NAME, hInstance);

    std::cout << "[+] Exited cleanly" << std::endl;
    return 0;
}
