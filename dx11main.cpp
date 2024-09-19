// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp
#include <windows.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include "weapons.h"
#include "sub_tab_icons.h"
#include <random>
#include "option.h"
#include <iostream>

// Структура для хранения привязки клавиш
struct KeyBind {
    bool waiting_for_key = false;   // Ожидание ввода клавиши
    ImGuiKey bound_key = ImGuiKey_None;  // Привязанная клавиша
    bool is_active = false;  // Состояние клавиши
    bool was_pressed = false;  // Было ли нажатие клавиши в текущем кадре
};

void BindKey(const char* label, KeyBind& key_bind) {
    // Если идет ожидание ввода клавиши
    if (key_bind.waiting_for_key) {
        ImGui::Text("Press any key...");  // Показываем сообщение о вводе
    }
    else {
        ImGui::Text("%s: %s", label, key_bind.bound_key != ImGuiKey_None ? ImGui::GetKeyName(key_bind.bound_key) : "...");
    }

    // Если текст был нажат, включаем режим ожидания ввода клавиши
    if (ImGui::IsItemClicked()) {
        key_bind.waiting_for_key = true;
    }

    // Если включен режим биндинга, ждем нажатия клавиши
    if (key_bind.waiting_for_key) {
        ImGuiIO& io = ImGui::GetIO();
        for (int key = ImGuiKey_None; key < ImGuiKey_COUNT; key++) {
            if (ImGui::IsKeyPressed((ImGuiKey)key)) {
                // Исключаем левую и правую кнопку мыши
                if (key != ImGuiKey_MouseLeft && key != ImGuiKey_MouseRight) {
                    key_bind.bound_key = (ImGuiKey)key;  // Привязываем клавишу
                    key_bind.waiting_for_key = false;    // Завершаем режим ожидания
                }
                break;
            }
        }
    }

    // Проверяем состояние привязанной клавиши
    if (key_bind.bound_key != ImGuiKey_None) {
        ImGuiIO& io = ImGui::GetIO();
        bool currently_pressed = io.KeysDown[key_bind.bound_key];
        if (currently_pressed && !key_bind.was_pressed) {
            key_bind.is_active = !key_bind.is_active;  // Переключаем состояние
        }
        key_bind.was_pressed = currently_pressed;
    }
}

// Функция для проверки состояния клавиши
bool IsKeyActive(const KeyBind& key_bind) {
    return key_bind.is_active;
}

#define Begin_First_Column_First_Box       ImGui::SetCursorPos(ImVec2(10, 37)); ImGui::BeginChild("## Begin_First_Column_First_Box", ImVec2(200, 160.125), true);
#define Begin_First_Column_First_Box_End   ImGui::EndChild();

#define Begin_First_Column_Second_Box       ImGui::SetCursorPos(ImVec2(10, 37 + 160.125 + 15)); ImGui::BeginChild("## Begin_First_Column_Second_Box", ImVec2(200, 130), true);
#define Begin_First_Column_Second_Box_End   ImGui::EndChild();

#define BeginSecondBox      ImGui::SetCursorPos(ImVec2(225, 37));  ImGui::BeginChild("## second page", ImVec2(187, 305), true);
#define BeginSecondBox_End  ImGui::EndChild();

// Data
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;


//////////////////ЛОГИКА МАКРОСА/////////////////////
std::random_device rd;
std::mt19937 gen(rd());

static bool recoil_enabled = false;   // Controlled by the ImGui checkbox
static bool is_recoil_active = false; // Tracks if recoil is currently active
static int weapon_selected_idx = 0;   // Index for selected weapon in combo box

// FOV and Sensitivity controls
static int field_of_view = 90;       // FOV value
static float sensivity = 0.30f;      // Mouse sensitivity

// Function to randomize recoil movement slightly
float apply_randomization(float value, float random_range) {
    std::uniform_real_distribution<float> dis(-random_range, random_range);
    return value + dis(gen);
}

// Function to simulate smooth mouse movement, adjusted by FOV and Sensitivity
void move_mouse_smooth(int x, int y, int smooth_factor, int fov, float sensitivity) {
    float fov_factor = 90.0f / static_cast<float>(fov);  // Scale recoil based on FOV
    float sensitivity_factor = sensitivity;              // Scale based on sensitivity

    for (int i = 0; i < smooth_factor; i++) {
        float dx = apply_randomization(static_cast<float>(x) / smooth_factor, 0.5f) * fov_factor * sensitivity_factor;
        float dy = apply_randomization(static_cast<float>(y) / smooth_factor, 0.5f) * fov_factor * sensitivity_factor;
        mouse_event(MOUSEEVENTF_MOVE, static_cast<int>(dx), static_cast<int>(dy), 0, 0);
    }
}

// Function to handle recoil control based on the selected weapon's pattern
void handle_recoil_control(const std::vector<vector2>& recoil_pattern, float delay) {
    for (const auto& point : recoil_pattern) {
        move_mouse_smooth(point.x, point.y, 10, field_of_view, sensivity);  // Apply recoil with FOV and sensitivity adjustments
        Sleep(static_cast<int>(delay));           // Delay between shots
    }
}

// Function to process recoil control only for AK-47 and when both LMB and RMB are pressed
void process_recoil_control() {
    if (weapon_selected_idx == 1 && recoil_enabled) {  // Check if AK-47 is selected
        std::vector<vector2> recoil_pattern = Weapons::ak::patternak;  // AK-47 recoil pattern
        float recoil_delay = Weapons::ak::delay;                       // AK-47 recoil delay
        handle_recoil_control(recoil_pattern, recoil_delay);
    }
}

// Check for mouse button states using ImGui
void ProcessRecoilControl() {
    if (ImGui::IsMouseDown(0) && ImGui::IsMouseDown(1)) {  // Both LMB and RMB are pressed
        if (recoil_enabled && !is_recoil_active) {
            is_recoil_active = true;
            process_recoil_control();  // Start recoil control if enabled
        }
    }
    else {
        is_recoil_active = false;  // Stop recoil control when buttons are released
    }
}
// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, nullptr, nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);

    HWND hwnd = ::CreateWindowExW(
        WS_EX_TOPMOST,
        wc.lpszClassName,
        L"Dear ImGui DiASDmple",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        123, // Ширина окна
        123, // Высота окна
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr
    );

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    ImFont* def_fnt = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\Arial.ttf", 12.0f);
    ImFont* icons_sub_tabs = io.Fonts->AddFontFromMemoryCompressedBase85TTF(sub_tab_icons, 14.f);
    // Our state
    bool show_demo_window = true;
    bool show_another_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window being minimized or screen locked
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        static int current_page = -1;

        ImGui::PushFont(def_fnt);

       /* ImGui::ShowDemoWindow();*/
        // 3. Show another simple window.
        if (show_another_window)
        {
            static ImVec2 window_size = ImVec2(424, 351);//424х351

            if (current_page == -1)
            {
                window_size = ImVec2(440, 450);
                ImGui::SetNextWindowSize(window_size);
            }
            else {
                window_size = ImVec2(424, 351);
                ImGui::SetNextWindowSize(window_size);
            }

            ImGui::Begin("1234567890", &show_another_window, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

            /*menu drawing*/
            {
                const auto& p = ImGui::GetWindowPos();
                const auto& pWindowDrawList = ImGui::GetWindowDrawList();
                const auto& pBackgroundDrawList = ImGui::GetBackgroundDrawList();
                const auto& pForegroundDrawList = ImGui::GetForegroundDrawList();

                ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0.000f + p.x, 0.000f + p.y), ImVec2(window_size.x + p.x, window_size.y + p.y), ImColor(20, 20, 22, 255), 0.f); //Main Background filled box

                ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0.000f + p.x, 0.000f + p.y), ImVec2(window_size.x + p.x, 25 + p.y), ImColor(23, 23, 25, 255), 0.f); //Tittle logo back ground

                ImGui::GetBackgroundDrawList()->AddRectFilledMultiColor(ImVec2(5.000f + p.x, 25.000f + p.y), ImVec2(window_size.x + p.x, 28.000f + p.y),
                    //ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left
                    ImColor(68, 23, 118, 255), ImColor(0, 0, 0, 0), ImColor(0, 0, 0, 0), ImColor(68, 23, 118, 255));

                pForegroundDrawList->AddRect(ImVec2(0.000f + p.x, 0.000f + p.y), ImVec2(window_size.x + p.x, window_size.y + p.y), ImColor(255, 255, 255, 45), 0, 0, 0.1f);

            }

            /*menu name*/
            {
                ImGui::Text("Resolution");
                ImGui::SameLine(60);
                ImGui::TextColored(ImColor(68, 23, 118, 255), ".club");
            }

            /*menu tabs*/
            {
                ImGui::PushFont(icons_sub_tabs);



                ImColor first_page_color;
                ImColor second_page_color;

                ImColor active_color = ImColor(85, 27, 179);
                ImColor un_active_color = ImColor(255, 255, 255);

                ImGui::SameLine(375);
                ImGui::TextColored(((current_page == 0) ? active_color : un_active_color), "1");
                if (ImGui::IsItemClicked()) {
                    current_page = 0;
                }

                ImGui::SameLine();

                ImGui::TextColored(((current_page == 1) ? active_color : un_active_color), "2");
                if (ImGui::IsItemClicked()) {
                    current_page = 1;
                }



                ImGui::PopFont();
            }
            static int weapon_selected_idx = 0;
            switch (current_page)
            {
            case -1:
                ImGui::SetCursorPosY(50);
                ImGui::TextWrapped("Welcome to the Resolituon\n"
                    "Introduction:\n"
                    "are you crooked? can't you move your mouse? are you disabled? then this script is for you fucking autistic person who wants to suck everywhere and everyone with 0 fps 10 year old schoolboy   \n\n"
                    " \n"
                    " \n"
                    "- Version: [1.81]\n"
                    " \n"
                    "Disclaimer:\n"
                    "This macro is intended for personal use and is provided \"as is.\" The creator is not responsible for any issues that may arise from the use of this software.\n\n"
                    "- Unauthorized reproduction or distribution of this software is prohibited.");
                break;
            case 0:
                Begin_First_Column_First_Box; {
                    ImGui::Checkbox("enable recoil", &recoil_enabled);

                    ImGui::Spacing();

                    if (true == 1 /*weapon choose combobox*/) {

                        ImGui::Text("weapon");
                        const char* weapons[] = { "ak", "lr", "mp5", "smg", "tompson", "m249","m39","python","semi" };
                        const char* combo_preview_value_ = weapons[weapon_selected_idx];



                        if (ImGui::BeginCombo("## weapon combo", combo_preview_value_, ImGuiComboFlags_HeightRegular | ImGuiComboFlags_NoArrowButton))
                        {
                            for (int n = 0; n < IM_ARRAYSIZE(weapons); n++)
                            {
                                const bool is_selected = (weapon_selected_idx == n);
                                if (ImGui::Selectable(weapons[n], is_selected))
                                    weapon_selected_idx = n;

                                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }
                    }

                    ImGui::Spacing();

                    if (true == 1 /*scopes modules combobox*/) {

                        ImGui::Text("scopes");
                        const char* scopes[] = { "holo", "8x", "16x", "Simple" };
                        static int scope_selected_idx = 0; // Here we store our selection data as an index.
                        const char* combo_preview_value__ = scopes[scope_selected_idx];

                        if (ImGui::BeginCombo("## scope combo", combo_preview_value__, ImGuiComboFlags_HeightRegular | ImGuiComboFlags_NoArrowButton))
                        {
                            for (int n = 0; n < IM_ARRAYSIZE(scopes); n++)
                            {
                                const bool is_selected = (scope_selected_idx == n);
                                if (ImGui::Selectable(scopes[n], is_selected))
                                    scope_selected_idx = n;

                                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }
                    }

                    if (true == 1 /*muzzle modules combobox*/) {

                        ImGui::Text("muzzle modules");
                        const char* muzzle_modules[] = { "None","Suppressor","Muzzle Brake", "Silencer" };
                        static int muzzle_selected_idx = 0; // Here we store our selection data as an index.
                        const char* combo_preview_value_muzzle = muzzle_modules[muzzle_selected_idx];



                        if (ImGui::BeginCombo("## muzzle combo", combo_preview_value_muzzle, ImGuiComboFlags_HeightRegular | ImGuiComboFlags_NoArrowButton))
                        {
                            for (int n = 0; n < IM_ARRAYSIZE(muzzle_modules); n++)
                            {
                                const bool is_selected = (muzzle_selected_idx == n);
                                if (ImGui::Selectable(muzzle_modules[n], is_selected))
                                    muzzle_selected_idx = n;

                                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }
                    }

                } Begin_First_Column_First_Box_End;

                Begin_First_Column_Second_Box; {
                    ImGui::Text("Exit script");
                    if (ImGui::IsItemClicked()) {
                        done = true;
                    }
                } Begin_First_Column_Second_Box_End;


                BeginSecondBox; {

                    switch (weapon_selected_idx)
                    {
                    case 0://ak
                    {
                        ImGui::SliderInt("RCS X", &options::ak47::ak47_recoil_comp_x, 0, 100);
                        ImGui::SliderInt("RCS Y", &options::ak47::ak47_recoil_comp_y, 0, 100);
                        ImGui::SliderInt("RCS Humanizer", &options::ak47::ak47_recoil_comp_humanizer, 0, 100);
                    }
                    break;
                    case 1://lr300
                    {
                        ImGui::SliderInt("RCS X", &options::lr300::lr300_recoil_comp_x, 0, 100);
                        ImGui::SliderInt("RCS Y", &options::lr300::lr300_recoil_comp_y, 0, 100);
                        ImGui::SliderInt("RCS Humanizer", &options::lr300::lr300_recoil_comp_humanizer, 0, 100);
                    }
                    break;
                    case 2://mp5
                    {
                        ImGui::SliderInt("RCS X", &options::mp5::mp5_recoil_comp_x, 0, 100);
                        ImGui::SliderInt("RCS Y", &options::mp5::mp5_recoil_comp_y, 0, 100);
                        ImGui::SliderInt("RCS Humanizer", &options::mp5::mp5_recoil_comp_humanizer, 0, 100);
                    }
                    break;
                    case 3://smg
                    {
                        ImGui::SliderInt("RCS X", &options::smg::smg_recoil_comp_x, 0, 100);
                        ImGui::SliderInt("RCS Y", &options::smg::smg_recoil_comp_y, 0, 100);
                        ImGui::SliderInt("RCS Humanizer", &options::smg::smg_recoil_comp_humanizer, 0, 100);
                    }
                    break;
                    case 4://tompshon
                    {
                        ImGui::SliderInt("RCS X", &options::tompshon::tompshon_recoil_comp_x, 0, 100);
                        ImGui::SliderInt("RCS Y", &options::tompshon::tompshon_recoil_comp_y, 0, 100);
                        ImGui::SliderInt("RCS Humanizer", &options::tompshon::tompshon_recoil_comp_humanizer, 0, 100);
                    }

                    break;
                    default:
                        break;
                    }

                    static int recoil_comp_humanizer = 0.f;
                    static KeyBind crouch_bind;
                    static char* key_state = "[ Keybind ] -> ";
                    BindKey(key_state, crouch_bind);
                    if (GetAsyncKeyState(crouch_bind.bound_key))
                        if (IsKeyActive(crouch_bind)) {
                            key_state = "[ Keybind accepted ] ->";
                        }
                        else {
                            key_state = "[ Keybind disabled ] -> ";
                        }

                    ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

                    static int field_of_view = 90.f;
                    ImGui::SliderInt("field of view", &field_of_view, 60, 120);

                    static float sensivity = 0.30f;
                    ImGui::SliderFloat("sensivity", &sensivity, 0.1f, 1.0f);

                    static float ads_sensivity = 0.30f;
                    ImGui::SliderFloat("ads sens", &ads_sensivity, 0.f, 1.f);

                    static bool hipfire_ = false;
                    ImGui::Checkbox("Hipfire", &hipfire_);

                    static bool cursor_check = false;
                    ImGui::Checkbox("Cursor check", &cursor_check);

                } BeginSecondBox_End;
                break;



            case 1:

                Begin_First_Column_First_Box; {
                    ImGui::Text("Config");
                    ImGui::Button("Save Config");
                    ImGui::SameLine(95.f);
                    ImGui::Button("Loading Config");
                    ImGui::SameLine(100.f);
                }
                 Begin_First_Column_First_Box_End;


                 BeginSecondBox; {

                     ImGui::Text("Misc");
                     static bool hidebar = false;
                     ImGui::Checkbox("Hide Window Bar", &hidebar);

                } BeginSecondBox_End;
                break;
            default:
                break;
            }


            ImGui::PopFont();
            ImGui::End();
        }

        HWND HDASD = FindWindowA("ImGui Platform","ImGui Platform");
        ShowWindow(HDASD, SW_SHOW);
        ::UpdateWindow(HDASD);



        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        // Present
        /*HRESULT hr = g_pSwapChain->Present(1, 0);*/   // Present with vsync
        HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions
bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    case WM_DPICHANGED:
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
        {
            //const int dpi = HIWORD(wParam);
            //printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
            const RECT* suggested_rect = (RECT*)lParam;
            ::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
