#include "../../../backend.hpp"
#include "../../../console/console.hpp"

#ifdef ENABLE_BACKEND_DX12
#include <windows.h>
#include <type_traits>

#include <d3d12.h>
#include <dxgi1_4.h>

#include "hook_directx12.hpp"

#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"
#include "MinHook.h"

#include "../../../utils/utils.hpp"
#include "../../hooks.hpp"

#include "../../../menu/menu.hpp"

static int const NUM_BACK_BUFFERS = 3;
static IDXGIFactory4* g_dxgiFactory = NULL;
static ID3D12Device* g_pd3dDevice = NULL;
static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = NULL;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;
static ID3D12CommandQueue* g_pd3dCommandQueue = NULL;
static ID3D12GraphicsCommandList* g_pd3dCommandList = NULL;
static IDXGISwapChain3* g_pSwapChain = NULL;
static ID3D12CommandAllocator* g_commandAllocators[NUM_BACK_BUFFERS] = { };
static ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = { };
static D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = { };
static bool g_bSwapChainHooked = false;
static void** g_pSwapChainVTable = NULL;
static void** g_pCommandQueueVTable = NULL;

static void CleanupRenderTarget( );
static void RenderImGui_DX12(IDXGISwapChain3* pSwapChain);
static void CreateRenderTarget(IDXGISwapChain* pSwapChain);

static std::add_pointer_t<void WINAPI(ID3D12CommandQueue*, UINT, ID3D12CommandList*)> oExecuteCommandLists;
static std::add_pointer_t<HRESULT WINAPI(IDXGISwapChain3*, UINT, UINT)> oPresent;
static std::add_pointer_t<HRESULT WINAPI(IDXGISwapChain3*, UINT, UINT, const DXGI_PRESENT_PARAMETERS*)> oPresent1;
static std::add_pointer_t<HRESULT WINAPI(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)> oResizeBuffers;
static std::add_pointer_t<HRESULT WINAPI(IDXGISwapChain3*, UINT, UINT, UINT, DXGI_FORMAT, UINT, const UINT*, IUnknown* const*)> oResizeBuffers1;
static std::add_pointer_t<HRESULT WINAPI(IDXGIFactory*, IUnknown*, DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**)> oCreateSwapChain;
static std::add_pointer_t<HRESULT WINAPI(IDXGIFactory*, IUnknown*, HWND, const DXGI_SWAP_CHAIN_DESC1*, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC*, IDXGIOutput*, IDXGISwapChain1**)> oCreateSwapChainForHwnd;
static std::add_pointer_t<HRESULT WINAPI(IDXGIFactory*, IUnknown*, IUnknown*, const DXGI_SWAP_CHAIN_DESC1*, IDXGIOutput*, IDXGISwapChain1**)> oCreateSwapChainForCoreWindow;
static std::add_pointer_t<HRESULT WINAPI(IDXGIFactory*, IUnknown*, const DXGI_SWAP_CHAIN_DESC1*, IDXGIOutput*, IDXGISwapChain1**)> oCreateSwapChainForComposition;
static HRESULT WINAPI hkPresent(IDXGISwapChain3* pSwapChain,
                                UINT SyncInterval,
                                UINT Flags) {
    static bool first = true;
    if (first) { first = false; RawLog("=== hkPresent CALLED ===\n"); }
    RenderImGui_DX12(pSwapChain);
    return oPresent(pSwapChain, SyncInterval, Flags);
}

static HRESULT WINAPI hkPresent1(IDXGISwapChain3* pSwapChain,
                                 UINT SyncInterval,
                                 UINT PresentFlags,
                                 const DXGI_PRESENT_PARAMETERS* pPresentParameters) {
    RenderImGui_DX12(pSwapChain);
    return oPresent1(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);
}

static HRESULT WINAPI hkResizeBuffers(IDXGISwapChain* pSwapChain,
                                      UINT BufferCount,
                                      UINT Width,
                                      UINT Height,
                                      DXGI_FORMAT NewFormat,
                                      UINT SwapChainFlags) {
    CleanupRenderTarget( );
    return oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

static HRESULT WINAPI hkResizeBuffers1(IDXGISwapChain3* pSwapChain,
                                       UINT BufferCount,
                                       UINT Width,
                                       UINT Height,
                                       DXGI_FORMAT NewFormat,
                                       UINT SwapChainFlags,
                                       const UINT* pCreationNodeMask,
                                       IUnknown* const* ppPresentQueue) {
    CleanupRenderTarget( );
    return oResizeBuffers1(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags, pCreationNodeMask, ppPresentQueue);
}

static void WINAPI hkExecuteCommandLists(ID3D12CommandQueue* pCommandQueue,
                                         UINT NumCommandLists,
                                         ID3D12CommandList* ppCommandLists) {
    if (!g_pd3dCommandQueue) {
        g_pd3dCommandQueue = pCommandQueue;
        LOG("[+] DX12: Captured command queue from ECL: %p\n", pCommandQueue);
    }
    return oExecuteCommandLists(pCommandQueue, NumCommandLists, ppCommandLists);
}

static const char* MHStatusStr(MH_STATUS s) {
    switch(s) {
        case MH_OK: return "MH_OK";
        case MH_ERROR_ALREADY_INITIALIZED: return "MH_ERROR_ALREADY_INITIALIZED";
        case MH_ERROR_NOT_INITIALIZED: return "MH_ERROR_NOT_INITIALIZED";
        case MH_ERROR_ALREADY_CREATED: return "MH_ERROR_ALREADY_CREATED";
        case MH_ERROR_NOT_CREATED: return "MH_ERROR_NOT_CREATED";
        case MH_ERROR_ENABLED: return "MH_ERROR_ENABLED";
        case MH_ERROR_DISABLED: return "MH_ERROR_DISABLED";
        case MH_ERROR_NOT_EXECUTABLE: return "MH_ERROR_NOT_EXECUTABLE";
        case MH_ERROR_UNSUPPORTED_FUNCTION: return "MH_ERROR_UNSUPPORTED_FUNCTION";
        case MH_ERROR_MEMORY_ALLOC: return "MH_ERROR_MEMORY_ALLOC";
        case MH_ERROR_FUNCTION_NOT_FOUND: return "MH_ERROR_FUNCTION_NOT_FOUND";
        default: return "UNKNOWN";
    }
}

static void InstallSwapChainHooks(IDXGISwapChain* pRawSwapChain) {
    if (g_bSwapChainHooked) return;
    g_bSwapChainHooked = true;

    void** pVTable = *reinterpret_cast<void***>(pRawSwapChain);
    g_pSwapChainVTable = pVTable;

    void* fnPresent = pVTable[8];
    void* fnPresent1 = pVTable[22];
    void* fnResizeBuffers = pVTable[13];
    void* fnResizeBuffers1 = pVTable[39];

    LOG("[+] DX12: VTable: %p\n", pVTable);
    LOG("[+] DX12: Present: %p, Present1: %p, Resize: %p, Resize1: %p\n",
        fnPresent, fnPresent1, fnResizeBuffers, fnResizeBuffers1);

    oPresent = reinterpret_cast<decltype(oPresent)>(fnPresent);
    oPresent1 = reinterpret_cast<decltype(oPresent1)>(fnPresent1);
    oResizeBuffers = reinterpret_cast<decltype(oResizeBuffers)>(fnResizeBuffers);
    oResizeBuffers1 = reinterpret_cast<decltype(oResizeBuffers1)>(fnResizeBuffers1);

    DWORD oldProtect;
    VirtualProtect(&pVTable[8], sizeof(void*), PAGE_READWRITE, &oldProtect);
    pVTable[8] = reinterpret_cast<void*>(&hkPresent);
    VirtualProtect(&pVTable[8], sizeof(void*), oldProtect, &oldProtect);

    VirtualProtect(&pVTable[22], sizeof(void*), PAGE_READWRITE, &oldProtect);
    pVTable[22] = reinterpret_cast<void*>(&hkPresent1);
    VirtualProtect(&pVTable[22], sizeof(void*), oldProtect, &oldProtect);

    VirtualProtect(&pVTable[13], sizeof(void*), PAGE_READWRITE, &oldProtect);
    pVTable[13] = reinterpret_cast<void*>(&hkResizeBuffers);
    VirtualProtect(&pVTable[13], sizeof(void*), oldProtect, &oldProtect);

    VirtualProtect(&pVTable[39], sizeof(void*), PAGE_READWRITE, &oldProtect);
    pVTable[39] = reinterpret_cast<void*>(&hkResizeBuffers1);
    VirtualProtect(&pVTable[39], sizeof(void*), oldProtect, &oldProtect);

    LOG("[+] DX12: VTable hooks installed (direct patch, no MinHook).\n");
}

static void InstallECLHook(ID3D12CommandQueue* pQueue) {
    void** pVTable = *reinterpret_cast<void***>(pQueue);
    g_pCommandQueueVTable = pVTable;
    void* fnECL = pVTable[10];
    LOG("[+] DX12: CommandQueue VTable: %p, ECL: %p\n", pVTable, fnECL);

    oExecuteCommandLists = reinterpret_cast<decltype(oExecuteCommandLists)>(fnECL);

    DWORD oldProtect;
    VirtualProtect(&pVTable[10], sizeof(void*), PAGE_READWRITE, &oldProtect);
    pVTable[10] = reinterpret_cast<void*>(&hkExecuteCommandLists);
    VirtualProtect(&pVTable[10], sizeof(void*), oldProtect, &oldProtect);

    LOG("[+] DX12: ECL vtable hooked (direct patch).\n");
}

static void OnSwapChainCaptured(IDXGISwapChain* pRawSwapChain) {
    if (g_pSwapChain) return;

    if (FAILED(pRawSwapChain->QueryInterface(IID_PPV_ARGS(&g_pSwapChain)))) {
        LOG("[!] DX12: QI for IDXGISwapChain3 failed, using raw pointer.\n");
        g_pSwapChain = static_cast<IDXGISwapChain3*>(pRawSwapChain);
        g_pSwapChain->AddRef();
    }

    LOG("[+] DX12: Captured game swap chain: %p\n", g_pSwapChain);

    InstallSwapChainHooks(pRawSwapChain);
}

static HRESULT WINAPI hkCreateSwapChain(IDXGIFactory* pFactory,
                                        IUnknown* pDevice,
                                        DXGI_SWAP_CHAIN_DESC* pDesc,
                                        IDXGISwapChain** ppSwapChain) {
    HRESULT hr = oCreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
    if (SUCCEEDED(hr) && ppSwapChain && *ppSwapChain) {
        LOG("[+] DX12: CreateSwapChain called, device: %p\n", pDevice);
        if (pDevice && !g_pd3dCommandQueue) {
            g_pd3dCommandQueue = reinterpret_cast<ID3D12CommandQueue*>(pDevice);
            InstallECLHook(g_pd3dCommandQueue);
        }
        OnSwapChainCaptured(*ppSwapChain);
    }
    return hr;
}

static HRESULT WINAPI hkCreateSwapChainForHwnd(IDXGIFactory* pFactory,
                                               IUnknown* pDevice,
                                               HWND hWnd,
                                               const DXGI_SWAP_CHAIN_DESC1* pDesc,
                                               const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc,
                                               IDXGIOutput* pRestrictToOutput,
                                               IDXGISwapChain1** ppSwapChain) {
    HRESULT hr = oCreateSwapChainForHwnd(pFactory, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
    if (SUCCEEDED(hr) && ppSwapChain && *ppSwapChain) {
        LOG("[+] DX12: CreateSwapChainForHwnd called, device: %p\n", pDevice);
        if (pDevice && !g_pd3dCommandQueue) {
            g_pd3dCommandQueue = reinterpret_cast<ID3D12CommandQueue*>(pDevice);
            InstallECLHook(g_pd3dCommandQueue);
        }
        OnSwapChainCaptured(*ppSwapChain);
    }
    return hr;
}

static HRESULT WINAPI hkCreateSwapChainForCoreWindow(IDXGIFactory* pFactory,
                                                     IUnknown* pDevice,
                                                     IUnknown* pWindow,
                                                     const DXGI_SWAP_CHAIN_DESC1* pDesc,
                                                     IDXGIOutput* pRestrictToOutput,
                                                     IDXGISwapChain1** ppSwapChain) {
    HRESULT hr = oCreateSwapChainForCoreWindow(pFactory, pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);
    if (SUCCEEDED(hr) && ppSwapChain && *ppSwapChain) {
        LOG("[+] DX12: CreateSwapChainForCoreWindow called, device: %p\n", pDevice);
        if (pDevice && !g_pd3dCommandQueue) {
            g_pd3dCommandQueue = reinterpret_cast<ID3D12CommandQueue*>(pDevice);
            InstallECLHook(g_pd3dCommandQueue);
        }
        OnSwapChainCaptured(*ppSwapChain);
    }
    return hr;
}

static HRESULT WINAPI hkCreateSwapChainForComposition(IDXGIFactory* pFactory,
                                                      IUnknown* pDevice,
                                                      const DXGI_SWAP_CHAIN_DESC1* pDesc,
                                                      IDXGIOutput* pRestrictToOutput,
                                                      IDXGISwapChain1** ppSwapChain) {
    HRESULT hr = oCreateSwapChainForComposition(pFactory, pDevice, pDesc, pRestrictToOutput, ppSwapChain);
    if (SUCCEEDED(hr) && ppSwapChain && *ppSwapChain) {
        LOG("[+] DX12: CreateSwapChainForComposition called, device: %p\n", pDevice);
        if (pDevice && !g_pd3dCommandQueue) {
            g_pd3dCommandQueue = reinterpret_cast<ID3D12CommandQueue*>(pDevice);
            InstallECLHook(g_pd3dCommandQueue);
        }
        OnSwapChainCaptured(*ppSwapChain);
    }
    return hr;
}

namespace DX12 {
    void Hook(HWND hwnd) {
        if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&g_dxgiFactory)))) {
            LOG("[!] DX12: CreateDXGIFactory1() failed.\n");
            return;
        }

        LOG("[+] DX12: g_dxgiFactory: 0x%p\n", g_dxgiFactory);
        Menu::InitializeContext(hwnd);

        void** pFactoryVTable = *reinterpret_cast<void***>(g_dxgiFactory);

        void* fnCreateSwapChain = pFactoryVTable[10];
        void* fnCreateSwapChainForHwnd = pFactoryVTable[15];
        void* fnCreateSwapChainForCWindow = pFactoryVTable[16];
        void* fnCreateSwapChainForComp = pFactoryVTable[24];

        LOG("[+] DX12: Factory hooks — CSC: %p, ForHwnd: %p, ForCore: %p, ForComp: %p\n",
            fnCreateSwapChain, fnCreateSwapChainForHwnd, fnCreateSwapChainForCWindow, fnCreateSwapChainForComp);

        MH_CreateHook(reinterpret_cast<void**>(fnCreateSwapChain), &hkCreateSwapChain, reinterpret_cast<void**>(&oCreateSwapChain));
        MH_CreateHook(reinterpret_cast<void**>(fnCreateSwapChainForHwnd), &hkCreateSwapChainForHwnd, reinterpret_cast<void**>(&oCreateSwapChainForHwnd));
        MH_CreateHook(reinterpret_cast<void**>(fnCreateSwapChainForCWindow), &hkCreateSwapChainForCoreWindow, reinterpret_cast<void**>(&oCreateSwapChainForCoreWindow));
        MH_CreateHook(reinterpret_cast<void**>(fnCreateSwapChainForComp), &hkCreateSwapChainForComposition, reinterpret_cast<void**>(&oCreateSwapChainForComposition));

        MH_EnableHook(fnCreateSwapChain);
        MH_EnableHook(fnCreateSwapChainForHwnd);
        MH_EnableHook(fnCreateSwapChainForCWindow);
        MH_EnableHook(fnCreateSwapChainForComp);

        LOG("[+] DX12: Factory hooks installed. Waiting for game swap chain...\n");
    }

    void Unhook( ) {
        if (g_pSwapChainVTable && oPresent) {
            DWORD oldProtect;
            VirtualProtect(&g_pSwapChainVTable[8], sizeof(void*), PAGE_READWRITE, &oldProtect);
            g_pSwapChainVTable[8] = reinterpret_cast<void*>(oPresent);
            VirtualProtect(&g_pSwapChainVTable[8], sizeof(void*), oldProtect, &oldProtect);

            VirtualProtect(&g_pSwapChainVTable[22], sizeof(void*), PAGE_READWRITE, &oldProtect);
            g_pSwapChainVTable[22] = reinterpret_cast<void*>(oPresent1);
            VirtualProtect(&g_pSwapChainVTable[22], sizeof(void*), oldProtect, &oldProtect);

            VirtualProtect(&g_pSwapChainVTable[13], sizeof(void*), PAGE_READWRITE, &oldProtect);
            g_pSwapChainVTable[13] = reinterpret_cast<void*>(oResizeBuffers);
            VirtualProtect(&g_pSwapChainVTable[13], sizeof(void*), oldProtect, &oldProtect);

            VirtualProtect(&g_pSwapChainVTable[39], sizeof(void*), PAGE_READWRITE, &oldProtect);
            g_pSwapChainVTable[39] = reinterpret_cast<void*>(oResizeBuffers1);
            VirtualProtect(&g_pSwapChainVTable[39], sizeof(void*), oldProtect, &oldProtect);

            LOG("[+] DX12: Swap chain VTable entries restored.\n");
            g_pSwapChainVTable = NULL;
        }
        if (g_pCommandQueueVTable && oExecuteCommandLists) {
            DWORD oldProtect;
            VirtualProtect(&g_pCommandQueueVTable[10], sizeof(void*), PAGE_READWRITE, &oldProtect);
            g_pCommandQueueVTable[10] = reinterpret_cast<void*>(oExecuteCommandLists);
            VirtualProtect(&g_pCommandQueueVTable[10], sizeof(void*), oldProtect, &oldProtect);
            LOG("[+] DX12: CommandQueue VTable entries restored.\n");
            g_pCommandQueueVTable = NULL;
        }
        g_bSwapChainHooked = false;

        if (ImGui::GetCurrentContext( )) {
            if (ImGui::GetIO( ).BackendRendererUserData)
                ImGui_ImplDX12_Shutdown( );

            if (ImGui::GetIO( ).BackendPlatformUserData)
                ImGui_ImplWin32_Shutdown( );

            ImGui::DestroyContext( );
        }

        CleanupRenderTarget( );

        if (g_pSwapChain) {
            g_pSwapChain->Release( );
            g_pSwapChain = NULL;
        }
        for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
            if (g_commandAllocators[i]) {
                g_commandAllocators[i]->Release( );
                g_commandAllocators[i] = NULL;
            }
        if (g_pd3dCommandList) {
            g_pd3dCommandList->Release( );
            g_pd3dCommandList = NULL;
        }
        if (g_pd3dRtvDescHeap) {
            g_pd3dRtvDescHeap->Release( );
            g_pd3dRtvDescHeap = NULL;
        }
        if (g_pd3dSrvDescHeap) {
            g_pd3dSrvDescHeap->Release( );
            g_pd3dSrvDescHeap = NULL;
        }
        if (g_pd3dDevice) {
            g_pd3dDevice->Release( );
            g_pd3dDevice = NULL;
        }
        if (g_dxgiFactory) {
            g_dxgiFactory->Release( );
            g_dxgiFactory = NULL;
        }
    }
} // namespace DX12

static void CleanupRenderTarget( ) {
    for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
        if (g_mainRenderTargetResource[i]) {
            g_mainRenderTargetResource[i]->Release( );
            g_mainRenderTargetResource[i] = NULL;
        }
}

static void CreateRenderTarget(IDXGISwapChain* pSwapChain) {
    for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i) {
        ID3D12Resource* pBackBuffer = NULL;
        pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        if (pBackBuffer) {
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);

            D3D12_RENDER_TARGET_VIEW_DESC desc = { };
            desc.Format = static_cast<DXGI_FORMAT>(Utils::GetCorrectDXGIFormat(sd.BufferDesc.Format));
            desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

            g_pd3dDevice->CreateRenderTargetView(pBackBuffer, &desc, g_mainRenderTargetDescriptor[i]);
            g_mainRenderTargetResource[i] = pBackBuffer;
        }
    }
}

static void RenderImGui_DX12(IDXGISwapChain3* pSwapChain) {
    if (!ImGui::GetIO( ).BackendRendererUserData) {
        if (SUCCEEDED(pSwapChain->GetDevice(IID_PPV_ARGS(&g_pd3dDevice)))) {
            LOG("[+] DX12: Got device from swap chain: %p\n", g_pd3dDevice);
            {
                D3D12_DESCRIPTOR_HEAP_DESC desc = { };
                desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                desc.NumDescriptors = NUM_BACK_BUFFERS;
                desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                desc.NodeMask = 1;
                HRESULT hr = g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap));
                if (hr != S_OK) {
                    LOG("[!] DX12: CreateDescriptorHeap(RTV) failed: 0x%08lx\n", hr);
                    return;
                }

                SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart( );
                for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i) {
                    g_mainRenderTargetDescriptor[i] = rtvHandle;
                    rtvHandle.ptr += rtvDescriptorSize;
                }
            }
            LOG("[+] DX12: RTV heap created.\n");

            {
                D3D12_DESCRIPTOR_HEAP_DESC desc = { };
                desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                desc.NumDescriptors = 1;
                desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
                HRESULT hr = g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap));
                if (hr != S_OK) {
                    LOG("[!] DX12: CreateDescriptorHeap(CBV_SRV_UAV) failed: 0x%08lx\n", hr);
                    return;
                }
            }
            LOG("[+] DX12: SRV heap created.\n");

            for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i) {
                HRESULT hr = g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_commandAllocators[i]));
                if (hr != S_OK) {
                    LOG("[!] DX12: CreateCommandAllocator[%u] failed: 0x%08lx\n", i, hr);
                    return;
                }
            }
            LOG("[+] DX12: Command allocators created.\n");

            HRESULT hr = g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocators[0], NULL, IID_PPV_ARGS(&g_pd3dCommandList));
            if (hr != S_OK) {
                LOG("[!] DX12: CreateCommandList failed: 0x%08lx\n", hr);
                return;
            }
            hr = g_pd3dCommandList->Close( );
            if (hr != S_OK) {
                LOG("[!] DX12: CommandList Close failed: 0x%08lx\n", hr);
                return;
            }

            LOG("[+] DX12: Calling ImGui_ImplDX12_Init...\n");
            DXGI_SWAP_CHAIN_DESC swapDesc;
            pSwapChain->GetDesc(&swapDesc);
            LOG("[+] DX12: Swap chain format: %d, buffer size: %ux%u, buffer count: %u\n",
                swapDesc.BufferDesc.Format, swapDesc.BufferDesc.Width, swapDesc.BufferDesc.Height, swapDesc.BufferDesc.RefreshRate.Numerator);
            ImGui_ImplDX12_Init(g_pd3dDevice, NUM_BACK_BUFFERS,
                                swapDesc.BufferDesc.Format, g_pd3dSrvDescHeap,
                                g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart( ),
                                g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart( ));
            LOG("[+] DX12: ImGui_ImplDX12_Init done. Context: %p\n", ImGui::GetCurrentContext());
        } else {
            LOG("[!] DX12: GetDevice failed.\n");
        }
    }

    if (!H::bShuttingDown) {
        if (!g_mainRenderTargetResource[0]) {
            CreateRenderTarget(pSwapChain);
            LOG("[+] DX12: CreateRenderTarget called. Resource[0]: %p, CmdQueue: %p, Context: %p\n",
                g_mainRenderTargetResource[0], g_pd3dCommandQueue, ImGui::GetCurrentContext());
        }

            if (ImGui::GetCurrentContext( ) && g_pd3dCommandQueue && g_mainRenderTargetResource[0]) {
            static int frameCount = 0;
            frameCount++;
            if (frameCount <= 5 || (frameCount % 600 == 0)) {
                LOG("[+] DX12: Frame %d — rendering.\n", frameCount);
            }

            ImGui_ImplDX12_NewFrame( );
            ImGui_ImplWin32_NewFrame( );
            ImGui::NewFrame( );

            Menu::Render( );

            ImGui::Render( );

            ImDrawData* drawData = ImGui::GetDrawData( );
            if (!drawData || drawData->CmdListsCount == 0) {
                static bool loggedEmpty = false;
                if (!loggedEmpty) {
                    loggedEmpty = true;
                    LOG("[!] DX12: ImGui draw data empty (CmdListsCount=%d, display size=%.0fx%.0f)\n",
                        drawData ? drawData->CmdListsCount : -1,
                        drawData ? drawData->DisplaySize.x : 0, drawData ? drawData->DisplaySize.y : 0);
                }
                return;
            } else {
                static bool loggedInfo = false;
                if (!loggedInfo) {
                    loggedInfo = true;
                    LOG("[+] DX12: ImGui draw data: %d lists, display %.0fx%.0f, display pos %.0fx%.0f\n",
                        drawData->CmdListsCount, drawData->DisplaySize.x, drawData->DisplaySize.y,
                        drawData->DisplayPos.x, drawData->DisplayPos.y);
                }
            }

            UINT backBufferIdx = pSwapChain->GetCurrentBackBufferIndex( );
            ID3D12CommandAllocator* commandAllocator = g_commandAllocators[backBufferIdx];
            commandAllocator->Reset( );

            D3D12_RESOURCE_BARRIER barrier = { };
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = g_mainRenderTargetResource[backBufferIdx];
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            g_pd3dCommandList->Reset(commandAllocator, NULL);
            g_pd3dCommandList->ResourceBarrier(1, &barrier);

            g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, NULL);
            g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData( ), g_pd3dCommandList);
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            g_pd3dCommandList->ResourceBarrier(1, &barrier);
            g_pd3dCommandList->Close( );

            g_pd3dCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(&g_pd3dCommandList));
            if (frameCount <= 5) {
                LOG("[+] DX12: Frame %d — ECL submitted, bbIdx: %u, drawLists: %d\n",
                    frameCount, backBufferIdx, drawData->CmdListsCount);
            }
        }
    }
}

#else
#include <windows.h>
namespace DX12 {
    void Hook(HWND hwnd) { LOG("[!] DirectX12 backend is not enabled!\n"); }
    void Unhook( ) { }
} // namespace DX12
#endif
