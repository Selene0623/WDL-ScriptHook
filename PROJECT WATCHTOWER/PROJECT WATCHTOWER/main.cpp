#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <thread>
#include "MinHook.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

extern "C" {
#include "luajit/src/lua.h"
#include "luajit/src/lauxlib.h"
#include "luajit/src/lualib.h"
}

#include "ScriptEngine.h"
#include "ImGuiScriptConsole.h"
#include "ScriptHookAPI.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

HWND g_hWnd = nullptr;
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
bool g_MenuOpen = true;
bool g_ImGuiInitialized = false;

typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain*, UINT, UINT);
Present_t oPresent = nullptr;

// original global variables for input, window proc, menu, etc, preserved
WNDPROC oWndProc = nullptr;
bool g_DoInit = false;

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
}

void CreateRenderTarget(IDXGISwapChain* pSwapChain)
{
    ID3D11Texture2D* pBackBuffer = nullptr;
    if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer))) {
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }
}

LRESULT CALLBACK hWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (g_ImGuiInitialized && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
}

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    if (!g_ImGuiInitialized)
    {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pd3dDevice))) {
            g_pd3dDevice->GetImmediateContext(&g_pd3dDeviceContext);
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            g_hWnd = sd.OutputWindow;
            CreateRenderTarget(pSwapChain);

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = NULL;
            ImGui::StyleColorsDark();
            ImGui_ImplWin32_Init(g_hWnd);
            ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

            // Install wndproc hook
            oWndProc = (WNDPROC)SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)hWndProc);

            // ---- ScriptEngine initialize (only change) ----
            ScriptEngine::Initialize();

            g_ImGuiInitialized = true;
        }
    }

    if (g_ImGuiInitialized)
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (GetAsyncKeyState(VK_INSERT) & 1)
            g_MenuOpen = !g_MenuOpen;

        if (g_MenuOpen)
        {
            ImGui::SetNextWindowSize(ImVec2(420, 420), ImGuiCond_Once);
            ImGui::Begin("Project WatchTower - By EncryptedStudios", &g_MenuOpen, ImGuiWindowFlags_NoCollapse);

            static int current_tab = 0;
            const char* tabs[] = { "Player", "World", "Settings", "Scripts" };

            if (ImGui::BeginTabBar("Tabs"))
            {
                for (int i = 0; i < IM_ARRAYSIZE(tabs); i++)
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
                        else if (current_tab == 3)
                        {
                            ImGuiScriptConsole::Render(); // Only addition
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

        // ---- ScriptEngine per-frame call (only change) ----
        ScriptEngine::OnFrame();

        ImGui::Render();
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
    if (MH_Initialize() != MH_OK)
        return 1;

    // Create temp window + D3D11 device to get the Present vtable pointer
    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "SHWDL_Temp";
    RegisterClassExA(&wc);

    HWND hTempWnd = CreateWindowExA(0, wc.lpszClassName, "tmp", WS_OVERLAPPEDWINDOW,
        0, 0, 100, 100, NULL, NULL, wc.hInstance, NULL);
    if (!hTempWnd) {
        UnregisterClassA(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = 2;
    sd.BufferDesc.Height = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hTempWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    IDXGISwapChain* pSwapChain = nullptr;
    ID3D11Device* pDevice = nullptr;
    ID3D11DeviceContext* pContext = nullptr;

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0,
        &featureLevel, 1, D3D11_SDK_VERSION,
        &sd, &pSwapChain, &pDevice, NULL, &pContext);

    if (FAILED(hr)) {
        DestroyWindow(hTempWnd);
        UnregisterClassA(wc.lpszClassName, wc.hInstance);
        MH_Uninitialize();
        return 1;
    }

    // Grab Present from the vtable (index 8)
    void** vtable = *(void***)pSwapChain;
    oPresent = (Present_t)vtable[8];

    // Release the temp objects — we only needed the vtable pointer
    pContext->Release();
    pDevice->Release();
    pSwapChain->Release();
    DestroyWindow(hTempWnd);
    UnregisterClassA(wc.lpszClassName, wc.hInstance);

    // Hook Present
    MH_STATUS status = MH_CreateHook((LPVOID)oPresent, (LPVOID)&hkPresent, (LPVOID*)&oPresent);
    if (status != MH_OK) {
        MH_Uninitialize();
        return 1;
    }
    status = MH_EnableHook((LPVOID)oPresent);
    if (status != MH_OK) {
        MH_Uninitialize();
        return 1;
    }

    return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID)
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
            ScriptEngine::Shutdown();
            MH_Uninitialize();
        }
        break;
    }
    return TRUE;
}
