#include <windows.h>
#include "imgui.h"
#include <d3d11.h>
#include <dxgi.h>
#include <MinHook.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// Globals
ID3D11Device* g_pd3dDevice = NULL;
ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
ID3D11RenderTargetView* g_mainRenderTargetView = NULL;
HWND g_hWnd = NULL;

typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain*, UINT, UINT);
Present_t oPresent = NULL;

int g_ImGuiInitialized = 0;
bool g_MenuOpen = true; // 1 == open, 0 == closed

// Forward declarations
void CleanupRenderTarget();
void CreateRenderTarget(IDXGISwapChain* pSwapChain);
IDXGISwapChain* GetD3D11SwapChain();
DWORD WINAPI MainThread(LPVOID lpReserved);

// Hooked Present function
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    if (!g_ImGuiInitialized)
    {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pd3dDevice)))
        {
            pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pd3dDevice);
            g_pd3dDevice->GetImmediateContext(&g_pd3dDeviceContext);

            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            g_hWnd = sd.OutputWindow;

            CreateRenderTarget(pSwapChain);

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = NULL; // disable imgui.ini saving

            ImGui::StyleColorsDark();

            ImGui_ImplWin32_Init(g_hWnd);
            ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

            g_ImGuiInitialized = 1;
        }
    }

    if (g_ImGuiInitialized)
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if ((GetAsyncKeyState(VK_INSERT) & 1) != 0)
            g_MenuOpen = !g_MenuOpen;

        if (g_MenuOpen)
        {
            ImGui::SetNextWindowSize(ImVec2(300, 350), ImGuiCond_Once);
            ImGui::Begin("Project WatchTower - By EncryptedStudios", &g_MenuOpen, ImGuiWindowFlags_NoCollapse);

            static int current_tab = 0;
            const char* tabs[] = { "Player", "World", "Settings" };

            if (ImGui::BeginTabBar("Tabs"))
            {
                int i;
                for (i = 0; i < IM_ARRAYSIZE(tabs); i++)
                {
                    if (ImGui::BeginTabItem(tabs[i]))
                    {
                        current_tab = i;

                        if (current_tab == 0)
                        {
                            ImGui::Text("Player Menu");
                            if (ImGui::Button("Stub: Toggle Godmode"))
                                MessageBoxA(NULL, "Godmode toggled (stub)", "Info", MB_OK);
                            ImGui::Separator();
                        }
                        else if (current_tab == 1)
                        {
                            ImGui::Text("World Menu");
                            if (ImGui::Button("Stub: Spawn Item"))
                                MessageBoxA(NULL, "Item spawned (stub)", "Info", MB_OK);
                            ImGui::Separator();
                        }
                        else if (current_tab == 2)
                        {
                            ImGui::Text("Settings Menu");
                            static bool dummy_setting = false;
                            ImGui::Checkbox("Example Setting", &dummy_setting);
                        }

                        ImGui::EndTabItem();
                    }
                }
                ImGui::EndTabBar();
            }

            ImGui::Separator();
            ImGui::Text("Press Insert to toggle this menu");

            ImGui::End();
        }

        ImGui::Render();
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    return oPresent(pSwapChain, SyncInterval, Flags);
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView)
    {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = NULL;
    }
}

void CreateRenderTarget(IDXGISwapChain* pSwapChain)
{
    ID3D11Texture2D* pBackBuffer = NULL;
    if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer)))
    {
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }
}

IDXGISwapChain* GetD3D11SwapChain()
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = 2;
    sd.BufferDesc.Height = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = GetForegroundWindow();
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    IDXGISwapChain* pSwapChain = nullptr;
    ID3D11Device* pDevice = nullptr;
    ID3D11DeviceContext* pContext = nullptr;
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &sd,
        &pSwapChain,
        &pDevice,
        &featureLevel,
        &pContext);

    if (FAILED(hr))
        return nullptr;

    if (pDevice) pDevice->Release();
    if (pContext) pContext->Release();

    return pSwapChain;
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
    while (!g_hWnd)
    {
        g_hWnd = FindWindowW(NULL, L"Watch Dogs Legion");
        Sleep(100);
    }

    if (MH_Initialize() != MH_OK)
        return 1;

    void** vTable = *reinterpret_cast<void***>(GetD3D11SwapChain());
    if (MH_CreateHook(vTable[8], &hkPresent, reinterpret_cast<void**>(&oPresent)) != MH_OK)
        return 1;
    if (MH_EnableHook(vTable[8]) != MH_OK)
        return 1;

    return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        if (g_ImGuiInitialized)
        {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();

            CleanupRenderTarget();
            MH_Uninitialize();
        }
        break;
    }
    return TRUE;
}
