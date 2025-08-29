## DirectX Hooking: Architecture and Implementation (DX12 and DX11)

This document explains how the project hooks DirectX 12 and DirectX 11 to render an ImGui overlay and run per-frame cheat logic.

### Goals
- Install function hooks on the graphics API swap chain using Kiero + MinHook
- Initialize ImGui (Win32 + matching DX backend) once and render each frame
- Integrate the central cheat lifecycle (CheatMain) into the render loop
- Cleanly handle window resize and shutdown

---

## High-Level Architecture

- Injection: The DLL is injected and DllMain starts MainThread.
- Forced backend: A single global selects D3D12 or D3D11; no auto-detection.
- Kiero setup: Initialize for the target API and bind vtable indices to our hooks.
- First Present call: Perform one-time graphics/ImGui/CheatMain initialization.
- Each Present call: Build and render ImGui, run CheatMain::Update, then call the original Present.
- Cleanup: Remove hooks, shutdown ImGui, release graphics resources, restore WndProc.

Key files:
- dll/src/dllmain.cpp (entry, lifecycle, forced-backend init, cleanup)
- dll/src/hooks/d3d12hook.cpp (DX12 hooking and rendering)
- dll/src/hooks/d3d11hook.cpp (DX11 hooking and rendering)
- dll/src/hooks/hook_config.h (forced backend configuration)

---

## Backend Selection (Forced)

A single global controls which backend to hook.

File: dll/src/hooks/hook_config.h

```cpp
namespace Hook {
    enum class Backend { D3D12, D3D11 };
    inline Backend g_ForceBackend = Backend::D3D11; // choose DX11 or DX12
}
```

dll/src/dllmain.cpp selects and initializes the specified backend during Hook::Initialize().

---

## D3D12 Hook

File: dll/src/hooks/d3d12hook.cpp

What is hooked (Kiero vtable indices for D3D12 path):
- ExecuteCommandLists (54): Capture the ID3D12CommandQueue pointer
- Signal (58): Track fence/value for frame sync
- Present (140): Main per-frame hook; first-run init and per-frame overlay/cheat
- ResizeBuffers (145): Rebuild resources on window resize

One-time initialization (first Present call):
1. Device/SwapChain
   - Get ID3D12Device from IDXGISwapChain3
   - Read DXGI_SWAP_CHAIN_DESC to capture window handle and back-buffer count
2. Descriptor Heaps
   - Create SRV heap (shader visible) for ImGui
   - Create RTV heap sized to back-buffer count
3. Commands & Sync
   - Create command allocator and command list
   - Allocate FrameContext array sized to back buffers
   - Create fence and fence event
   - Store swap chain and frame latency waitable object
4. Render Targets
   - For each back buffer: GetBuffer + CreateRenderTargetView, store RTV + resource in FrameContext
5. Window Procedure
   - Hook WndProc with SetWindowLongPtr to route input to ImGui (and swallow mouse input when menu is open)
6. ImGui
   - ImGui::CreateContext, StyleColorsDark
   - ImGui_ImplWin32_Init(window)
   - ImGui_ImplDX12_Init(device, frames, format, srvHeap, cpuHandle, gpuHandle)
   - Load embedded fonts (Tahoma + FontAwesome) and ImGui_ImplDX12_CreateDeviceObjects()
7. Cheat System
   - Cheat::Core::CheatMain::Initialize()

Per-frame (Present):
1. Handle pending resize: wait fence, cleanup targets, ResizeBuffers, recreate targets
2. Toggle menu with Insert
3. New ImGui frame: ImGui_ImplDX12_NewFrame + ImGui_ImplWin32_NewFrame + ImGui::NewFrame
4. Render UI: set io.MouseDrawCursor if menu visible, call CheatMenu::Render()
5. Record commands:
   - Transition current back buffer: PRESENT -> RENDER_TARGET
   - OMSetRenderTargets to that back buffer's RTV
   - SetDescriptorHeaps to SRV heap used by ImGui
   - ImGui::Render(); ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList)
   - Cheat::Core::CheatMain::Update(GetTickCount())
   - Transition back: RENDER_TARGET -> PRESENT
   - Close and ExecuteCommandLists on the captured command queue
6. Call original Present

ResizeBuffers hook:
- ImGui_ImplDX12_InvalidateDeviceObjects
- CleanupRenderTarget
- Call original ResizeBuffers
- Recreate render targets and ImGui_ImplDX12_CreateDeviceObjects

Cleanup:
- kiero::shutdown()
- ImGui_ImplDX12_Shutdown, ImGui_ImplWin32_Shutdown, ImGui::DestroyContext
- WaitForLastSubmittedFrame (fence)
- Release RTV/SRV heaps, fence/event, command list/queue pointers, frame contexts
- Restore WndProc

Notes:
- D3D12 requires explicit resource barriers and command recording every frame.
- Additional hooks (ExecuteCommandLists, Signal) are used to integrate with the command queue and synchronization.

---

## D3D11 Hook

File: dll/src/hooks/d3d11hook.cpp

What is hooked (Kiero vtable indices for IDXGISwapChain):
- Present (8): Main per-frame hook; first-run init and per-frame overlay/cheat
- ResizeBuffers (13): Recreate RTV and ImGui device objects on window resize

One-time initialization (first Present call):
1. Device/Context/SwapChain
   - Get ID3D11Device from IDXGISwapChain
   - Acquire immediate context
   - Store swap chain pointer
   - Read DXGI_SWAP_CHAIN_DESC for window handle
2. Render Target
   - Create main RTV from back buffer 0
3. Window Procedure
   - Hook WndProc to route input to ImGui and swallow mouse input while menu is open
4. ImGui
   - ImGui::CreateContext, StyleColorsDark
   - ImGui_ImplWin32_Init(window)
   - ImGui_ImplDX11_Init(device, context)
   - Load embedded fonts (Tahoma + FontAwesome)
5. Cheat System
   - Cheat::Core::CheatMain::Initialize()

Per-frame (Present):
1. Toggle menu with Insert
2. New ImGui frame: ImGui_ImplDX11_NewFrame + ImGui_ImplWin32_NewFrame + ImGui::NewFrame
3. Render UI: set io.MouseDrawCursor if menu visible, call CheatMenu::Render()
4. Render:
   - ImGui::Render()
   - OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr)
   - ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData())
5. Cheat::Core::CheatMain::Update(GetTickCount())
6. Call original Present

ResizeBuffers hook:
- ImGui_ImplDX11_InvalidateDeviceObjects
- Release current RTV
- Call original ResizeBuffers
- Recreate RTV and ImGui_ImplDX11_CreateDeviceObjects

Cleanup:
- kiero::shutdown()
- ImGui_ImplDX11_Shutdown, ImGui_ImplWin32_Shutdown, ImGui::DestroyContext
- Release RTV, context, device
- Restore WndProc

Notes:
- D3D11 is simpler: immediate context, no explicit barriers or command lists for ImGui rendering.

---

## Window Procedure and Input

Both backends install a custom WndProc:
- First pass all messages to ImGui_ImplWin32_WndProcHandler
- While the menu is visible, swallow mouse events (clicks/wheel/move) so the game doesn't double-handle them
- Forward everything else to the original WndProc

This keeps user interaction with the UI smooth and prevents conflicts with the game's input processing.

---

## ImGui Initialization and Fonts

Common steps:
- ImGui::CreateContext, ImGui::StyleColorsDark
- ImGui_ImplWin32_Init(window)
- Backend init:
  - DX12: ImGui_ImplDX12_Init(device, frames, format, srvHeap, cpuHandle, gpuHandle)
  - DX11: ImGui_ImplDX11_Init(device, context)
- Fonts:
  - Add embedded Tahoma font from memory
  - Merge Font Awesome (solid) icon ranges into the atlas
  - Build atlas; for DX12, create device objects afterward

---

## Cleanup and Resource Management

- Unhook with MinHook (Disable/Remove) and kiero::shutdown
- Shutdown ImGui backend + Win32 + DestroyContext
- Restore original WndProc
- Release all graphics resources:
  - DX12: fences, events, heaps, command list/allocators, RTVs/back buffers, frame contexts
  - DX11: RTV, device context, device

---

## Build and Dependencies

- ImGui: core + backends (Win32, DX12, DX11)
- Link: d3d12.lib, d3d11.lib, dxgi.lib, imgui, minhook, fmt
- Build preset: `cmake --build --preset windows-x64-release-2022`

---

## Troubleshooting

- Overlay not visible:
  - Ensure Hook::g_ForceBackend matches the API you want
  - Check logs for Kiero init/bind success and first Present init
  - Verify WndProc was hooked
- Resize artifacts:
  - DX12: ensure ImGui device objects invalidation/recreation and RTV/SRV heaps are correct
  - DX11: ensure RTV and ImGui device objects are recreated
- Input issues:
  - Insert toggles menu; io.MouseDrawCursor should be true while menu is open
  - Confirm ImGui_ImplWin32_WndProcHandler returns true for UI events

---

## Differences Summary: DX12 vs DX11

- DX12
  - Explicit resource barriers, descriptor heaps, command recording, and synchronization
  - Hooks ExecuteCommandLists and Signal in addition to Present/ResizeBuffers
  - ImGui requires SRV heap and device objects creation

- DX11
  - Immediate context; simpler rendering path
  - Only Present and ResizeBuffers hooks
  - ImGui uses device/context and a main RTV from the back buffer

Both backends integrate the same CheatMain lifecycle and ImGui menu behavior, controlled by a single forced-backend toggle.
