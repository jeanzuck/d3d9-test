#include <windows.h>
#include <cstdio>
#include <cmath>
#include <sstream> // Add this include at the top

#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")


HWND g_hWnd = nullptr;
LPDIRECT3D9 g_pD3D = nullptr;
LPDIRECT3DDEVICE9 g_pDevice = nullptr;
LPDIRECT3DVERTEXBUFFER9 g_pVB = nullptr;
LPD3DXFONT g_pFont = nullptr;

struct Vertex {
    float x, y, z;
    DWORD color;
};
#define FVF (D3DFVF_XYZ | D3DFVF_DIFFUSE)

// 8 vertices of a cube
Vertex g_boxVertices[] = {
    // Front face (red)
    { -0.5f,  0.5f, -0.5f, 0xffff0000 }, // 0
    {  0.5f,  0.5f, -0.5f, 0xffff0000 }, // 1
    {  0.5f, -0.5f, -0.5f, 0xffff0000 }, // 2
    { -0.5f, -0.5f, -0.5f, 0xffff0000 }, // 3
    // Back face (green)
    { -0.5f,  0.5f,  0.5f, 0xff00ff00 }, // 4
    {  0.5f,  0.5f,  0.5f, 0xff00ff00 }, // 5
    {  0.5f, -0.5f,  0.5f, 0xff00ff00 }, // 6
    { -0.5f, -0.5f,  0.5f, 0xff00ff00 }, // 7
};

// 12 triangles (2 per face)
WORD g_boxIndices[] = {
    // Front
    0,1,2,  0,2,3,
    // Back
    4,6,5,  4,7,6,
    // Left
    4,5,1,  4,1,0,
    // Right
    3,2,6,  3,6,7,
    // Top
    4,0,3,  4,3,7,
    // Bottom
    1,5,6,  1,6,2
};

LPDIRECT3DINDEXBUFFER9 g_pIB = nullptr;

// Simple 4x4 matrix struct for rotation
struct Mat4 {
    float m[4][4];
    Mat4() { memset(m, 0, sizeof(m)); }
    static Mat4 Identity() {
        Mat4 r;
        r.m[0][0] = r.m[1][1] = r.m[2][2] = r.m[3][3] = 1.0f;
        return r;
    }
    static Mat4 RotationY(float angle) {
        Mat4 r = Identity();
        r.m[0][0] = cosf(angle);
        r.m[0][2] = sinf(angle);
        r.m[2][0] = -sinf(angle);
        r.m[2][2] = cosf(angle);
        return r;
    }
    static Mat4 RotationX(float angle) {
        Mat4 r = Identity();
        r.m[1][1] = cosf(angle);
        r.m[1][2] = -sinf(angle);
        r.m[2][1] = sinf(angle);
        r.m[2][2] = cosf(angle);
        return r;
    }
    static Mat4 Translation(float x, float y, float z) {
        Mat4 r = Identity();
        r.m[3][0] = x;
        r.m[3][1] = y;
        r.m[3][2] = z;
        return r;
    }
    static Mat4 Perspective(float fov, float aspect, float zn, float zf) {
        Mat4 r;
        float yScale = 1.0f / tanf(fov / 2.0f);
        float xScale = yScale / aspect;
        r.m[0][0] = xScale;
        r.m[1][1] = yScale;
        r.m[2][2] = zf / (zf - zn);
        r.m[2][3] = 1.0f;
        r.m[3][2] = (-zn * zf) / (zf - zn);
        return r;
    }
};

// Window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_DESTROY)
        PostQuitMessage(0);
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// Init D3D
bool InitD3D(HWND hWnd)
{
    g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!g_pD3D) return false;

    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

    if (FAILED(g_pD3D->CreateDevice(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp, &g_pDevice)))
        return false;

    // Vertex buffer
    g_pDevice->CreateVertexBuffer(sizeof(g_boxVertices), 0, FVF, D3DPOOL_MANAGED, &g_pVB, nullptr);
    void* ptr = nullptr;
    g_pVB->Lock(0, sizeof(g_boxVertices), &ptr, 0);
    memcpy(ptr, g_boxVertices, sizeof(g_boxVertices));
    g_pVB->Unlock();

    // Index buffer
    g_pDevice->CreateIndexBuffer(sizeof(g_boxIndices), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &g_pIB, nullptr);
    void* iptr = nullptr;
    g_pIB->Lock(0, sizeof(g_boxIndices), &iptr, 0);
    memcpy(iptr, g_boxIndices, sizeof(g_boxIndices));
    g_pIB->Unlock();

    // Font creation
    D3DXFONT_DESC fontDesc = { 0 };
    fontDesc.Height = 24;
    fontDesc.Width = 0;
    fontDesc.Weight = FW_NORMAL;
    fontDesc.MipLevels = 1;
    fontDesc.Italic = FALSE;
    fontDesc.CharSet = DEFAULT_CHARSET;
    fontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;
    fontDesc.Quality = DEFAULT_QUALITY;
    fontDesc.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
    wcscpy_s(fontDesc.FaceName, L"Arial");
    if (FAILED(D3DXCreateFontIndirect(g_pDevice, &fontDesc, &g_pFont)))
        return false;

    // Set render states for solid color
    g_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    g_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    return true;
}

// Set a simple transform (world * view * proj)
void SetTransform(float angle)
{
    // World: rotate around Y and X
    D3DXMATRIX matWorld, matRotY, matRotX;
    D3DXMatrixRotationY(&matRotY, angle);
    D3DXMatrixRotationX(&matRotX, angle * 0.5f);
    matWorld = matRotY * matRotX;
    g_pDevice->SetTransform(D3DTS_WORLD, &matWorld);

    // View: camera at (0,0,-3) looking at origin
    D3DXMATRIX matView;
    D3DXVECTOR3 eye(0, 0, -3);
    D3DXVECTOR3 at(0, 0, 0);
    D3DXVECTOR3 up(0, 1, 0);
    D3DXMatrixLookAtLH(&matView, &eye, &at, &up);
    g_pDevice->SetTransform(D3DTS_VIEW, &matView);

    // Projection: perspective
    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 3, 800.0f / 600.0f, 0.1f, 100.0f);
    g_pDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}

// Render
void Render()
{
    static DWORD start = GetTickCount();
    float angle = (GetTickCount() - start) * 0.001f;

    // --- FPS calculation ---
    static DWORD lastTime = 0;
    static int frameCount = 0;
    static float fps = 0.0f;
    DWORD now = GetTickCount();
    frameCount++;
    if (now - lastTime >= 1000) {
        fps = frameCount * 1000.0f / (now - lastTime);
        lastTime = now;
        frameCount = 0;
    }

    g_pDevice->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(20, 20, 40), 1.0f, 0);
    g_pDevice->BeginScene();

    SetTransform(angle);

    g_pDevice->SetFVF(FVF);
    g_pDevice->SetStreamSource(0, g_pVB, 0, sizeof(Vertex));
    g_pDevice->SetIndices(g_pIB);
    g_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12);

    if (g_pFont)
    {
        // Show pDevice pointer
        wchar_t buf[128];
        swprintf_s(buf, L"pDevice: 0x%p", static_cast<void*>(g_pDevice));

        // Show vtable pointers
        void **vtbl = *reinterpret_cast<void***>(g_pDevice);
        wchar_t vtblInfo[512];
        swprintf_s(vtblInfo,
            L"BeginScene (vTable=40): 0x%p\n"
            L"EndScene (vTable=41): 0x%p\n"
            L"DrawIndexedPrimitive (vTable=82): 0x%p\n"
            L"Present (vTable=17): 0x%p\n"
            L"Reset (vTable=14): 0x%p",
            vtbl[40], vtbl[41], vtbl[82], vtbl[17], vtbl[14]);

        // Show stride of 3D box
        wchar_t strideText[64];
        swprintf_s(strideText, L"Box stride: %d", (int)sizeof(Vertex));
        RECT rect1 = { 10, 10, 500, 40 };
        RECT rect2 = { 10, 50, 700, 300 };
        RECT rect3 = { 10, 220, 500, 260 };
        RECT rectFps = { 550, 10, 750, 40 };

        g_pFont->DrawTextW(nullptr, buf, -1, &rect1, DT_LEFT | DT_TOP, D3DCOLOR_XRGB(255,255,0));
        g_pFont->DrawTextW(nullptr, vtblInfo, -1, &rect2, DT_LEFT | DT_TOP, D3DCOLOR_XRGB(180,255,180));
        g_pFont->DrawTextW(nullptr, strideText, -1, &rect3, DT_LEFT | DT_TOP, D3DCOLOR_XRGB(200,200,255));

        // Show FPS at top right
        wchar_t fpsText[64];
        swprintf_s(fpsText, L"FPS: %.1f", fps);
        g_pFont->DrawTextW(nullptr, fpsText, -1, &rectFps, DT_RIGHT | DT_TOP, D3DCOLOR_XRGB(255, 200, 100));
    }

    g_pDevice->EndScene();
    g_pDevice->Present(nullptr, nullptr, nullptr, nullptr);
}

// Cleanup
void Cleanup()
{
    if (g_pIB) g_pIB->Release();
    if (g_pVB) g_pVB->Release();
    if (g_pFont) { g_pFont->Release(); g_pFont = nullptr; }
    if (g_pDevice) g_pDevice->Release();
    if (g_pD3D) g_pD3D->Release();
}

// Thread that creates the window and runs D3D
DWORD WINAPI RenderThread(LPVOID)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(0);
    wc.lpszClassName = L"D3DWnd";
    RegisterClass(&wc);

    g_hWnd = CreateWindow(wc.lpszClassName, L"d3d9-test",
        WS_OVERLAPPEDWINDOW, 100, 100, 800, 600,
        nullptr, nullptr, wc.hInstance, nullptr);

    ShowWindow(g_hWnd, SW_SHOW);

    if (!InitD3D(g_hWnd)) return 1;

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            Render();
        }
    }

    Cleanup();
    return 0;
}

int main()
{
    wprintf(L"d3d9-test is running...\n");
    HANDLE hThread = CreateThread(nullptr, 0, RenderThread, nullptr, 0, nullptr);

    // Wait for the render thread to finish (window closed)
    WaitForSingleObject(hThread, INFINITE);

    // Optionally: CloseHandle(hThread);

    return 0;
}