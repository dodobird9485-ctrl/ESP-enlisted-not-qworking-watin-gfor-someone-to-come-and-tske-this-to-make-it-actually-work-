#include "game_capture.h"
#include <iostream>

GameCapture::GameCapture()
    : m_gameWindow(nullptr), m_device(nullptr), m_context(nullptr),
      m_swapChain(nullptr), m_captureTexture(nullptr), m_stagingTexture(nullptr),
      m_width(1920), m_height(1080), m_hdcWindow(nullptr), m_hdcMemDC(nullptr),
      m_hbitmap(nullptr), m_hbitmap_old(nullptr) {
}

GameCapture::~GameCapture() {
    if (m_captureTexture) m_captureTexture->Release();
    if (m_stagingTexture) m_stagingTexture->Release();
    if (m_hbitmap) DeleteObject(m_hbitmap);
    if (m_hbitmap_old) SelectObject(m_hdcMemDC, m_hbitmap_old);
    if (m_hdcMemDC) DeleteDC(m_hdcMemDC);
    if (m_hdcWindow) ReleaseDC(m_gameWindow, m_hdcWindow);
}

bool GameCapture::Initialize(HWND targetWindow) {
    m_gameWindow = targetWindow;
    
    if (!IsWindow(m_gameWindow)) {
        std::cerr << "[-] Invalid target window" << std::endl;
        return false;
    }

    RECT clientRect;
    GetClientRect(m_gameWindow, &clientRect);
    m_width = clientRect.right - clientRect.left;
    m_height = clientRect.bottom - clientRect.top;

    std::cout << "[+] Captured game window: " << m_width << "x" << m_height << std::endl;

    m_hdcWindow = GetDC(m_gameWindow);
    if (!m_hdcWindow) {
        std::cerr << "[-] Failed to get window DC" << std::endl;
        return false;
    }

    m_hdcMemDC = CreateCompatibleDC(m_hdcWindow);
    if (!m_hdcMemDC) {
        std::cerr << "[-] Failed to create compatible DC" << std::endl;
        return false;
    }

    m_hbitmap = CreateCompatibleBitmap(m_hdcWindow, m_width, m_height);
    if (!m_hbitmap) {
        std::cerr << "[-] Failed to create compatible bitmap" << std::endl;
        return false;
    }

    m_hbitmap_old = (HBITMAP)SelectObject(m_hdcMemDC, m_hbitmap);

    return true;
}

bool GameCapture::CaptureFrame() {
    if (!IsWindow(m_gameWindow)) {
        std::cerr << "[-] Game window no longer exists" << std::endl;
        return false;
    }

    if (!BitBlt(m_hdcMemDC, 0, 0, m_width, m_height, m_hdcWindow, 0, 0, SRCCOPY)) {
        std::cerr << "[-] BitBlt failed" << std::endl;
        return false;
    }

    return true;
}

ID3D11Texture2D* GameCapture::GetCapturedTexture() const {
    return m_captureTexture;
}

void GameCapture::SetGameWindow(HWND hwnd) {
    if (m_hdcWindow) {
        ReleaseDC(m_gameWindow, m_hdcWindow);
    }
    m_gameWindow = hwnd;
    m_hdcWindow = GetDC(m_gameWindow);
}
