#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <d2d1.h>
#include <dwrite.h>
#include <string>
#include <vector>

struct Vec2 {
    float x, y;
};

struct Vec3 {
    float x, y, z;
};

struct Vec4 {
    float x, y, z, w;
};

struct Player {
    Vec3 position;
    Vec3 head;
    std::string name;
    float health;
    bool isAlly;
    bool isVisible;
    float distance;
};

class ESPRenderer {
public:
    ESPRenderer();
    ~ESPRenderer();

    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int width, int height);
    void Shutdown();
    
    void BeginFrame();
    void EndFrame();
    
    void DrawBox(Vec2 topLeft, Vec2 bottomRight, Vec3 color, float alpha = 1.0f);
    void DrawLine(Vec2 start, Vec2 end, Vec3 color, float alpha = 1.0f);
    void DrawText(Vec2 pos, const std::string& text, Vec3 color, float alpha = 1.0f);
    void DrawCircle(Vec2 center, float radius, Vec3 color, float alpha = 1.0f);
    
    void RenderPlayers(const std::vector<Player>& players, const float* viewProjMatrix);
    
    bool WorldToScreen(Vec3 worldPos, Vec2& screenPos, const float* viewProjMatrix);
    
    void SetRenderTarget(ID3D11RenderTargetView* rtv);

private:
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;
    ID3D11RenderTargetView* m_renderTarget;
    
    ID2D1Factory* m_d2dFactory;
    ID2D1RenderTarget* m_d2dTarget;
    IDWriteFactory* m_writeFactory;
    
    ID3D11InputLayout* m_inputLayout;
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11Buffer* m_vertexBuffer;
    ID3D11BlendState* m_blendState;
    ID3D11RasterizerState* m_rasterizerState;
    
    int m_screenWidth;
    int m_screenHeight;
    
    struct Vertex {
        float x, y;
        float r, g, b, a;
    };
    
    bool CreateShaders();
};
