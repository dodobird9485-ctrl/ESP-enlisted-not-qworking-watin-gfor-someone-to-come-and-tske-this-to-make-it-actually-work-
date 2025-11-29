#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <vector>

class GameCapture {
public:
    GameCapture();
    ~GameCapture();

    bool Initialize(HWND targetWindow);
    bool CaptureFrame();
    
    ID3D11Texture2D* GetCapturedTexture() const;
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    
    HWND GetGameWindow() const { return m_gameWindow; }
    void SetGameWindow(HWND hwnd);

private:
    HWND m_gameWindow;
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;
    IDXGISwapChain* m_swapChain;
    
    ID3D11Texture2D* m_captureTexture;
    ID3D11Texture2D* m_stagingTexture;
    
    int m_width;
    int m_height;
    
    HDC m_hdcWindow;
    HDC m_hdcMemDC;
    HBITMAP m_hbitmap;
    HBITMAP m_hbitmap_old;
};
