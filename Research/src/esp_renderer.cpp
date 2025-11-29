#include "esp_renderer.h"
#include <iostream>
#include <cmath>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

ESPRenderer::ESPRenderer()
    : m_device(nullptr), m_context(nullptr), m_renderTarget(nullptr),
      m_d2dFactory(nullptr), m_d2dTarget(nullptr), m_writeFactory(nullptr),
      m_inputLayout(nullptr), m_vertexShader(nullptr), m_pixelShader(nullptr),
      m_vertexBuffer(nullptr), m_blendState(nullptr), m_rasterizerState(nullptr),
      m_screenWidth(1920), m_screenHeight(1080) {
}

ESPRenderer::~ESPRenderer() {
    Shutdown();
}

bool ESPRenderer::CreateShaders() {
    const char* shaderCode = R"(
        cbuffer Transforms : register(b0) {
            float4x4 projection;
        };
        
        struct VS_INPUT {
            float2 pos : POSITION;
            float4 color : COLOR;
        };
        
        struct VS_OUTPUT {
            float4 pos : SV_POSITION;
            float4 color : COLOR;
        };
        
        VS_OUTPUT vs_main(VS_INPUT input) {
            VS_OUTPUT output;
            output.pos = float4(input.pos, 0.0f, 1.0f);
            output.pos.x = (output.pos.x / 960.0f) - 1.0f;
            output.pos.y = 1.0f - (output.pos.y / 540.0f);
            output.color = input.color;
            return output;
        }
        
        float4 ps_main(VS_OUTPUT input) : SV_TARGET {
            return input.color;
        }
    )";
    
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    
    HRESULT hr = D3DCompile(shaderCode, strlen(shaderCode), nullptr, nullptr, nullptr,
                            "vs_main", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            std::cerr << "[-] Vertex shader error: " << (char*)errorBlob->GetBufferPointer() << std::endl;
            errorBlob->Release();
        }
        return false;
    }
    
    hr = D3DCompile(shaderCode, strlen(shaderCode), nullptr, nullptr, nullptr,
                    "ps_main", "ps_5_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            std::cerr << "[-] Pixel shader error: " << (char*)errorBlob->GetBufferPointer() << std::endl;
            errorBlob->Release();
        }
        if (vsBlob) vsBlob->Release();
        return false;
    }
    
    hr = m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
                                      nullptr, &m_vertexShader);
    if (FAILED(hr)) {
        std::cerr << "[-] Failed to create vertex shader" << std::endl;
        vsBlob->Release();
        psBlob->Release();
        return false;
    }
    
    hr = m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
                                     nullptr, &m_pixelShader);
    if (FAILED(hr)) {
        std::cerr << "[-] Failed to create pixel shader" << std::endl;
        vsBlob->Release();
        psBlob->Release();
        return false;
    }
    
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    
    hr = m_device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(),
                                     vsBlob->GetBufferSize(), &m_inputLayout);
    vsBlob->Release();
    psBlob->Release();
    
    if (FAILED(hr)) {
        std::cerr << "[-] Failed to create input layout" << std::endl;
        return false;
    }
    
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(Vertex) * 10000;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    hr = m_device->CreateBuffer(&bufferDesc, nullptr, &m_vertexBuffer);
    if (FAILED(hr)) {
        std::cerr << "[-] Failed to create vertex buffer" << std::endl;
        return false;
    }
    
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    
    hr = m_device->CreateBlendState(&blendDesc, &m_blendState);
    if (FAILED(hr)) {
        std::cerr << "[-] Failed to create blend state" << std::endl;
        return false;
    }
    
    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.FrontCounterClockwise = false;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.DepthBiasClamp = 0.0f;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;
    rasterizerDesc.DepthClipEnable = true;
    rasterizerDesc.ScissorEnable = false;
    rasterizerDesc.MultisampleEnable = false;
    rasterizerDesc.AntialiasedLineEnable = true;
    
    hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);
    if (FAILED(hr)) {
        std::cerr << "[-] Failed to create rasterizer state" << std::endl;
        return false;
    }
    
    return true;
}

bool ESPRenderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int width, int height) {
    m_device = device;
    m_context = context;
    m_screenWidth = width;
    m_screenHeight = height;

    if (!CreateShaders()) {
        std::cerr << "[-] Failed to create shaders" << std::endl;
        return false;
    }

    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_d2dFactory))) {
        std::cerr << "[-] Failed to create D2D1 factory (non-fatal)" << std::endl;
    }

    if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&m_writeFactory))) {
        std::cerr << "[-] Failed to create DWrite factory (non-fatal)" << std::endl;
    }

    std::cout << "[+] ESP Renderer initialized: " << width << "x" << height << std::endl;
    return true;
}

void ESPRenderer::Shutdown() {
    if (m_vertexBuffer) {
        m_vertexBuffer->Release();
        m_vertexBuffer = nullptr;
    }
    if (m_inputLayout) {
        m_inputLayout->Release();
        m_inputLayout = nullptr;
    }
    if (m_vertexShader) {
        m_vertexShader->Release();
        m_vertexShader = nullptr;
    }
    if (m_pixelShader) {
        m_pixelShader->Release();
        m_pixelShader = nullptr;
    }
    if (m_blendState) {
        m_blendState->Release();
        m_blendState = nullptr;
    }
    if (m_rasterizerState) {
        m_rasterizerState->Release();
        m_rasterizerState = nullptr;
    }
    if (m_d2dTarget) {
        m_d2dTarget->Release();
        m_d2dTarget = nullptr;
    }
    if (m_d2dFactory) {
        m_d2dFactory->Release();
        m_d2dFactory = nullptr;
    }
    if (m_writeFactory) {
        m_writeFactory->Release();
        m_writeFactory = nullptr;
    }
}

void ESPRenderer::BeginFrame() {
    if (m_renderTarget && m_context) {
        float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        m_context->ClearRenderTargetView(m_renderTarget, clearColor);
    }
}

void ESPRenderer::EndFrame() {
    if (m_context) {
        m_context->Flush();
    }
}

void ESPRenderer::DrawBox(Vec2 topLeft, Vec2 bottomRight, Vec3 color, float alpha) {
    DrawLine(topLeft, { bottomRight.x, topLeft.y }, color, alpha);
    DrawLine({ bottomRight.x, topLeft.y }, bottomRight, color, alpha);
    DrawLine(bottomRight, { topLeft.x, bottomRight.y }, color, alpha);
    DrawLine({ topLeft.x, bottomRight.y }, topLeft, color, alpha);
}

void ESPRenderer::DrawLine(Vec2 start, Vec2 end, Vec3 color, float alpha) {
    if (!m_context || !m_vertexBuffer || !m_vertexShader || !m_pixelShader) return;
    
    Vertex vertices[] = {
        { start.x, start.y, color.x, color.y, color.z, alpha },
        { end.x, end.y, color.x, color.y, color.z, alpha }
    };
    
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (FAILED(m_context->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        return;
    }
    
    memcpy(mapped.pData, vertices, sizeof(vertices));
    m_context->Unmap(m_vertexBuffer, 0);
    
    m_context->IASetInputLayout(m_inputLayout);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
    
    m_context->VSSetShader(m_vertexShader, nullptr, 0);
    m_context->PSSetShader(m_pixelShader, nullptr, 0);
    
    float blendFactor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    m_context->OMSetBlendState(m_blendState, blendFactor, 0xffffffff);
    m_context->RSSetState(m_rasterizerState);
    
    m_context->Draw(2, 0);
}

void ESPRenderer::DrawText(Vec2 pos, const std::string& text, Vec3 color, float alpha) {
}

void ESPRenderer::RenderPlayers(const std::vector<Player>& players, const float* viewProjMatrix) {
    std::cout << "[*] Rendering " << players.size() << " players" << std::endl;
    for (const auto& player : players) {
        Vec2 screenPos = { player.position.x, player.position.y };
        
        if (viewProjMatrix) {
            if (!WorldToScreen(player.position, screenPos, viewProjMatrix)) {
                continue;
            }
        }

        if (screenPos.x < 0 || screenPos.x > m_screenWidth || screenPos.y < 0 || screenPos.y > m_screenHeight) {
            continue;
        }

        std::cout << "  > " << player.name << " at (" << (int)screenPos.x << "," << (int)screenPos.y << ")" << std::endl;

        Vec3 color = player.isAlly ? Vec3{ 0, 1, 0 } : Vec3{ 1, 0, 0 };

        DrawBox(
            { screenPos.x - 15, screenPos.y - 25 },
            { screenPos.x + 15, screenPos.y + 5 },
            color
        );
        
        DrawText(
            { screenPos.x - 15, screenPos.y - 40 },
            player.name,
            color
        );
    }
}

bool ESPRenderer::WorldToScreen(Vec3 worldPos, Vec2& screenPos, const float* viewProjMatrix) {
    if (!viewProjMatrix) return false;

    float screenPosX = worldPos.x * viewProjMatrix[0] + worldPos.y * viewProjMatrix[4] + 
                       worldPos.z * viewProjMatrix[8] + viewProjMatrix[12];
    float screenPosY = worldPos.x * viewProjMatrix[1] + worldPos.y * viewProjMatrix[5] + 
                       worldPos.z * viewProjMatrix[9] + viewProjMatrix[13];
    float screenPosW = worldPos.x * viewProjMatrix[3] + worldPos.y * viewProjMatrix[7] + 
                       worldPos.z * viewProjMatrix[11] + viewProjMatrix[15];

    if (screenPosW <= 0.0f) return false;

    screenPos.x = (screenPosX / screenPosW + 1.0f) * m_screenWidth / 2.0f;
    screenPos.y = (1.0f - screenPosY / screenPosW) * m_screenHeight / 2.0f;

    return true;
}

void ESPRenderer::SetRenderTarget(ID3D11RenderTargetView* rtv) {
    m_renderTarget = rtv;
    
    if (!m_d2dFactory || !rtv) return;
    
    if (m_d2dTarget) {
        m_d2dTarget->Release();
        m_d2dTarget = nullptr;
    }
}
