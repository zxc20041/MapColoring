// linearList_elevator_simulation.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "resource.h"
#include "framework_base.h"
#include "audio_thread.h"
#include "thread_communicate.h"
#include "settings.h"

#include"BJMapColoring.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
CHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
CHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_LINEARLISTELEVATORSIMULATION, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LINEARLISTELEVATORSIMULATION));

    MSG msg;

    page_index = 1000;
    init_once();

    //初始化发生异常，退出
    if (quit_single == 1)
    {
        process_quit();
        DestroyWindow(hWnd);
        return -1;
    }

    // 主消息循环:
    
    while (1)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (quit_single == 1)
        {
            process_quit();
            DestroyWindow(hWnd);
            break;
        }
        render();
    }

    return /*(int) msg.wParam*/0;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LINEARLISTELEVATORSIMULATION));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_LINEARLISTELEVATORSIMULATION);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindowA(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CLOSE:
        normal_quit = 1;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SIZE:
        OnSize();
        break;
    case WM_MOUSEWHEEL:
        input_wheel(GET_WHEEL_DELTA_WPARAM(wParam));
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}




void render()
{
    HRESULT hr = S_OK;
    DrawCallNum = 0;
    debugger_main.reset();
    update();
    static TIMER timer;
    while (main_thread_render_blocked > 0)
    {
        if (main_thread_render_blocked == 1)
        {
            main_thread_render_blocked = 2;
            timer.start_timer();
        }

        Sleep(0);
        if (timer.stop_timer() > 16)
        {
            quit_single = 1;
        }
        return;
    }
    g_pD2DDeviceContext->BeginDraw();
    if (FAILED(hr))
    {
        debugger_main.writelog(-1, "Rending Error in D2DDeviceContext->BeginDraw(): Draw failed! " + to_string(hr));
        MessageBox(NULL, "Draw failed!", "Error", 0);
        return;
    }
    g_cm.update();
    rendPage();
    g_PageManager.update();
    g_PageManager.rend();
    g_cm.rend();
    rend_quit();

    if (page_index < 1002 && set2[0].show_framerate)
    {
        DrawTextA_1(to_string(fps) + " [" + to_string(DrawCallNum) + "]", g_pTextFormat, fpsRect, g_pBrushBlue);
    }

    debugger_main.rend();

    hr = g_pD2DDeviceContext->EndDraw();

    if (FAILED(hr))
    {
        debugger_main.writelog(-1, "Rending Error in D2DDeviceContext->EndDraw(): Draw failed! " + to_string(hr));
        MessageBox(NULL, "Draw failed!\n", "Rending Error", 0);
        Sleep(1000);
        quit_single = 1;
        return;
    }
    if (set2[0].vsync && g_output)
    {
        g_output->WaitForVBlank();
        if (hWnd != FocusWindow)    //窗口失焦时降低帧率
        {
            g_output->WaitForVBlank();
            g_output->WaitForVBlank();
        }
    }

    g_pSwapChain->Present(0, 0);
    return;
}




class StartPageRectangle
{
    static constexpr int arc_r = 200;
    static constexpr int rect_num = 36;
    static constexpr float target_rect_length = 30;
    static constexpr float target_rect_interval = 20;
    static constexpr float rect_opacity_min = -0.25f;
    static constexpr float rect_opacity_max = 0.25f;
    static constexpr float rect_length_multi_min = 0.25f;
    static constexpr float rect_length_multi_max = 3;
    static constexpr float rect_interval_multi_min = 0.75f;
    static constexpr float rect_interval_multi_max = 1.5f;
private:
    float x1[rect_num], x2[rect_num], opacity[rect_num];
    int brush_index;
public:
    StartPageRectangle();
    void rend(float limit_x);
};

StartPageRectangle::StartPageRectangle()
{
    x1[0] = 0, x2[0] = 0;
    brush_index = rand() % 8;
    for (int i = 1; i < rect_num; i++)
    {
        x1[i] = x2[i - 1] + target_rect_interval * (rect_interval_multi_min + (rect_interval_multi_max - rect_interval_multi_min) * 0.01f * (rand() % 101));
        x2[i] = x1[i] + target_rect_length * (rect_length_multi_min + (rect_length_multi_max - rect_length_multi_min) * 0.01f * (rand() % 101));
        opacity[i] = rect_opacity_min + (rect_opacity_max - rect_opacity_min) * 0.01f * (rand() % 101);
        if (x2[i] > 1600)
        {
            break;
        }
    }
}

void StartPageRectangle::rend(float limit_x)
{
    FillRectangle_1(0, 870, limit_x, 900, BrushRand[brush_index]);
    for (int i = 1; i < rect_num; i++)
    {
        if (x1[i] < limit_x)
        {
            if (opacity[i] < 0)
            {
                FillRectangle_1(x1[i], 870, min(x2[i], limit_x), 900, g_pBrushDark, max(0.1f, -opacity[i]));
            }
            else
            {
                FillRectangle_1(x1[i], 870, min(x2[i], limit_x), 900, g_pBrushLight, max(0.1f, opacity[i]));
            }
            if (x2[i] > 1600)
            {
                break;
            }
        }
    }
    return;
}


void rend_start()
{
    static float text_rect_y = 700, text_opacity = 0, logo_opacity = 0, loading_progress_show = 0;
    static int start_page_stage = 0, loading_progress = 0, effect_wait_changing = 0;
    static StartPageRectangle rects;

    g_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::DarkSlateBlue));
    
    if (md5_result < 0)
    {
        page_index = 2001;  //verify failed
        return;
    }

    if (loading_progress_show < loading_progress * 160)
    {
        if (loading_progress_show < 0.1f)
        {
            effect_wait_changing = 0;
        }
        loading_progress_show += frmtm * max(600, (loading_progress * 160 - loading_progress_show) * 2);
    }

    rects.rend(loading_progress_show);

    if (page_index == 1000)
    {
        if (start_page_stage == 0)
        {
            text_rect_y = 700, text_opacity = 0;
            loading_progress = 0;
            logo_opacity = 0;
            start_page_stage = 1;
            effect_wait_changing = 0;

        }
        else if (start_page_stage == 1)
        {
            /*if (text_rect_y < 600)
            {
                text_rect_y += frmtm * 100;
            }*/

            loading_progress = 1;
            loading_progress += thread_IO_request_verify_res;
            if (thread_IO_request_verify_res == 8)
            {
                debugger_main.writelog(0, "Init finished.");
                thread_IO_request_read_record = 1;   //最后
                page_index = 1001;
            }
        }
    }
    else if (page_index == 1001)
    {
        //DrawBitmap_1(g_pD2DBimtapUI[11], D2D1::RectF(500, 150, 1100, 750), 1 - text_opacity);
        //FillRectangle_1(0, 675, 1600, 900, g_pBrushWhite, 0.25f - 0.25f * max(text_opacity,0));
        if (text_rect_y < 800)
        {
            text_rect_y += frmtm * 100;
        }
        loading_progress = 9;
        if (usernameC == "")
        {
            set_target_page(PAGE_REGISTER, 0, 0.4f); //sign up
        }
        if (thread_IO_request_read_record != 1)
        {
            loading_progress = 10;
        }

        text_opacity += frmtm;


        if (clicking && !effect_wait_changing && cpos.x > 0 && cpos.y > 0 && cpos.x < to_screen(1600) && cpos.y < to_screen(900)&& loading_progress == 10)
        {
            g_cm.CreateEffect(5);
            
            set_target_page(PAGE_HOME, 0, 0.4f);
            //g_am.playEffectSound(SE_arrow_fly1 + rand() % 2);
            
            effect_wait_changing = 1;
        }
    }
    if (text_opacity< 4)
    {
        DrawTriangle_1(800, 100, 700, 300, 900, 300, g_pBrushGameTemp, 10, 2.0f - fabsf(text_opacity - 2));
        DrawLine_1(800, 150, 800, 240, 10, g_pBrushGameTemp, 2.0f - fabsf(text_opacity - 2));
        FillEllipse_1(800, 270, 5, 5, g_pBrushGameTemp, 2.0f - fabsf(text_opacity - 2));
        DrawTextA_1(lan[0].PE_warning, g_rm.getFont("PUBLIC M"), 400, 300, 1200, 800, g_pBrushGameTemp, 2.0f - fabsf(text_opacity - 2));
    }
    if (text_opacity < 2)
    {
        DrawTextA_1("Z", g_pTextFormatNormal, D2D1::RectF(to_screen(1190), to_screen(text_rect_y), to_screen(1390), to_screen(text_rect_y + 100)), g_pBrushRed, 2-text_opacity);
        DrawTextA_1("Z", g_pTextFormatNormal, D2D1::RectF(to_screen(1200), to_screen(text_rect_y + 10), to_screen(1400), to_screen(text_rect_y + 110)), g_pBrushGreen, 2-text_opacity);
        DrawTextA_1("Z", g_pTextFormatNormal, D2D1::RectF(to_screen(1210), to_screen(text_rect_y + 20), to_screen(1410), to_screen(text_rect_y + 120)), g_pBrushLightBlue, 2-text_opacity);
        DrawTextA_1("C/C++ & Direct2D Project\n2D Application Framework", g_pTextFormat, D2D1::RectF(to_screen(1000), to_screen(text_rect_y - 160), to_screen(1600), to_screen(text_rect_y - 10)), g_pBrushBlue, 2-text_opacity);
    }
    else
    {
        DrawTextA_1("Click to Start", g_pTextFormat, 0, 700, 1600, 800, g_pBrushDark, fabs(cosf(text_opacity * 2)));
    }
    return;
}

class Home_Page :PAGE
{
public:
    Home_Page():PAGE(PAGE_INDEX, PAGE_CREATED_STATUS,Effect::WHITE_SWITCH,2)
    {
        startButton = quitButton = settingButton = nullptr;
        homepage_bg_type = homepage_text_type = 0;
        bg_transform_x = 0, bg_transform_y = 0, bg_light_opacity = 0.3f;
        text_type = 0;
    }
    ~Home_Page()
    {
        if (startButton != nullptr)
        {
            delete startButton;
        }
        if (quitButton != nullptr)
        {
            delete quitButton;
        }
        if (settingButton != nullptr)
        {
            delete settingButton;
        }

    }
    bool Init()
    {
        PAGE::Init();
        startButton = new Button(280, 700, 470, 800, lan[0].start, g_pBrushYellow, g_pBrushBlue, g_pBrushGreen);
        quitButton = new Button(900, 670, 1050, 750, ""/*lan[0].quit*/, g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, g_pD2DBimtapUI[1]);
        settingButton = new Button(600, 680, 750, 750, "", g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, g_pD2DBimtapUI[0]);

    }
    void Update()
    {
        PAGE::Update();
    }
    void FixedUpdate()
    {
        PAGE::FixedUpdate();
        return;
    }
    // 通过 PAGE 继承
    virtual void Rend() override
    {
    }
    bool EnterPage()
    {
        g_am.PlayBGM(2, 1, 0);
        PAGE::EnterPage();

    }
    virtual bool ExitPage() override
    {
        return false;
    }
    constexpr static int PAGE_INDEX = PAGE_HOME;
private:
    Button* startButton, * quitButton, * settingButton;
    int homepage_bg_type, homepage_text_type;
    float bg_transform_x = 0, bg_transform_y = 0, bg_light_opacity = 0.3f;
    bool text_type = 0;
};


void rend_home_page()
{
    static int homepage_status = 0, homepage_bg_type, homepage_text_type;
    static float bg_transform_x = 0, bg_transform_y = 0, bg_light_opacity = 0.3f;
    static float camera_delta_posx = 0, camera_delta_posy = 0;
    static bool text_type = 0;
    static Button* startButton, * quitButton, * settingButton,* extraButton;
    static float camera_posx = 400, camera_posy = 600, camera_zoom = 1.0f;
    static Letter* letters1[3] = { new Letter('A'),new Letter('P'), new Letter('P')};

    if (page_status == 0)
    {
        InitPage(PAGE_HOME);
        page_status = 1;
        last_page_index = 0;
        
        g_cm.ClearPage();

        startButton = g_cm.CreateButton(280, 700, 470, 800, lan[0].start, g_pBrushYellow, g_pBrushBlue, g_pBrushGreen);
        quitButton = g_cm.CreateButton(900, 670, 1050, 750, ""/*lan[0].quit*/, g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, g_pD2DBimtapUI[1]);
        settingButton = g_cm.CreateButton(600, 680, 750, 750, "", g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, g_pD2DBimtapUI[0]);
        extraButton= g_cm.CreateButton(280, 530, 470, 630, "Extra", g_pBrushYellow, g_pBrushBlue, g_pBrushGreen);
        //CreateButton(300, 370, 450, 450, lan[0].PVE, g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, NULL);//3
        //historyButton = g_cm.CreateButton(300, 500, 480, 580, lan[0].history, g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, NULL);

        text_type = 1;
        if (!long_credit && rand() % 4 != 0)
        {
            text_type = 0;
        }

        bg_transform_x = rand() % 4000;
        bg_transform_y = rand() % 4000;
        if (st.wHour > 8 && st.wHour < 19)
        {
            homepage_bg_type = 12;
            homepage_text_type = 13;
            bg_light_opacity = 0.3f;
        }
        else
        {
            homepage_bg_type = 13;
            homepage_text_type = 12;
            bg_light_opacity = 0.15f;
        }
        for (auto i : letters1)
        {
            i->reset();
        }
        homepage_status = 0;
    }
    else if (page_status == 1)
    {
        g_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::DarkSlateBlue));
        bg_transform_x += frmtm * 20;
        bg_transform_y += frmtm * 20;
        g_pBitmapBrushUI[12]->SetTransform(D2D1::Matrix3x2F::Translation(D2D1::SizeF(to_screen(bg_transform_x), to_screen(bg_transform_y))));
        g_pBitmapBrushUI[13]->SetTransform(D2D1::Matrix3x2F::Translation(D2D1::SizeF(to_screen(bg_transform_x), to_screen(bg_transform_y))));
        if (!long_credit)
        {
            FillRectangle_1(0, 0, 1600, 900, g_pBitmapBrushUI[homepage_bg_type]);

            FillRectangle_1(0, 0, 1600, 900, g_pBrushLight, bg_light_opacity);
        }
        
        
        if (text_type)
        {
            DrawTextA_1(PRODUCT_NAME, g_pTextFormatLarge, D2D1::RectF(to_screen(100), to_screen(100), to_screen(1700), to_screen(650)), g_pBitmapBrushUI[homepage_text_type]);
        }
        else
        {
            for (auto i : letters1)
            {
                i->update();
            }
            for (auto i : letters1)
            {
                i->rend();
            }
        }

        if (quitButton->getClicked())
        {
            normal_quit = 1;
        }
        else if (settingButton->getClicked())
        {
            g_cm.CreateEffect(3, cpos.x, cpos.y);
            set_target_page(PAGE_SETTING, 0, 0.4f);
        }
        else if (startButton->getClicked())
        {
            set_target_page(PAGE_GAME, 0, 0);
            g_PageManager.SwitchPageTo(BJMAP_PAGE::PAGE_INDEX);
            //g_cm.CreateEffect(5);
        }
        else if (extraButton->getClicked())
        {
            set_target_page(MAP_COLORING_PAGE::PAGE_INDEX, 0, 0);
            g_PageManager.SwitchPageTo(MAP_COLORING_PAGE::PAGE_INDEX);
        }
        return;
    }
    return;
}


void rend_setting_page()
{
    static int setting_page_index = 1;
    static string setting_descreption = "";
    static int button_drag = 0;
    static bool graphic_changed = 0;

    static float deviation = 0;
    static float chosen_block_posx = 420;
    static float chosen_block_target_posx = 420;
    static float chosen_block_speed_base = 0;
    static float chosen_block_move_status = 1;  //> 1 for static, 0 for start moving, 0-1 for moving
    static float chosen_block_color_r = 1, chosen_block_color_g = 0.5f, chosen_block_color_b = 0;
    static float chosen_block_target_color_r = 0, chosen_block_target_color_g = 0, chosen_block_target_color_b = 0;
    static float chosen_block_color_base_r = 0, chosen_block_color_base_g = 0, chosen_block_color_base_b = 0;


    static Button* returnButton, * generalButton, * graphicsButton, * audioButton;
    static Button* languageButton, * extra_mode_sizeButton, * demo_mode_timePerStepButton, * extra_mode_timePerStepButton,*colorLimitButton; //gameplay
    static Button* WindowModeButton, * VsyncButton, * AntialiasingButton, * RenderResolutionButton, * ShowFramerateButton;  //graphics
    static Button* MusicVolumeButton, * SEVolumeButton, * ThreadMethodButton;
    static LABEL* describeLabel;

    static LABEL* languageLabel, * extra_mode_sizeLabel, * demo_mode_timePerStepLabel,* extra_mode_timePerStepLabel,* colorLimitLabel;
    static LABEL* WindowModeLabel, * VsyncLabel, * AntialiasingLabel, * RenderResolutionLabel, * ShowFramerateLabel;
    static LABEL* MusicVolumeLabel, * SEVolumeLabel, * ThreadMethodLabel, * MusicVolumeBar, * SEVolumeBar;

    g_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::LightBlue));
    if (page_status == 0)   //初始化
    {
        g_cm.ClearPage();
        InitPage(1);
        page_status = 1;
        returnButton = g_cm.CreateButton(100, 50, 250, 100, ""/*lan[0].str_return*/, g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, g_pD2DBimtapUI[10]);

        describeLabel = g_cm.CreateLabel(100, 790, 1500, 880, "", NULL, g_pBrushPurple, g_pBrushBlack, NULL, 0);
        generalButton = g_cm.CreateButton(420, 130, 580, 240, lan[0].general, NULL, NULL, g_pBrushGreen, NULL);
        graphicsButton = g_cm.CreateButton(620, 130, 780, 240, lan[0].graphics, NULL, NULL, g_pBrushGreen, NULL);
        audioButton = g_cm.CreateButton(820, 130, 980, 240, lan[0].audio, NULL, NULL, g_pBrushGreen, NULL);
        

        languageButton = g_cm.CreateButton(1250, 300, 1400, 380, lan[0].on, g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);
        demo_mode_timePerStepButton = g_cm.CreateButton(1250, 400, 1400, 480, "", g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);
        extra_mode_sizeButton = g_cm.CreateButton(1250, 500, 1400, 580, "", g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);
        extra_mode_timePerStepButton = g_cm.CreateButton(1250, 600, 1400, 680, "", g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);
        colorLimitButton = g_cm.CreateButton(1250, 700, 1400, 780, "", g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);

        WindowModeButton = g_cm.CreateButton(1250, 300, 1400, 380, lan[0].on, g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);
        VsyncButton = g_cm.CreateButton(1250, 400, 1400, 480, lan[0].on, g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);
        AntialiasingButton = g_cm.CreateButton(1250, 500, 1400, 580, lan[0].on, g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);
        RenderResolutionButton = g_cm.CreateButton(1250, 600, 1400, 680, lan[0].on, g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);
        ShowFramerateButton = g_cm.CreateButton(1250, 700, 1400, 780, lan[0].on, g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);

        MusicVolumeButton = g_cm.CreateButton(600, 350, 650, 380, "", g_pBrushLightBlue, g_pBrushBlue, g_pBrushBlack, NULL);
        SEVolumeButton = g_cm.CreateButton(600, 450, 650, 480, "", g_pBrushLightBlue, g_pBrushBlue, g_pBrushBlack, NULL);
        ThreadMethodButton = g_cm.CreateButton(1250, 600, 1400, 680, lan[0].mode1, g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);

        MusicVolumeButton->setUItype(UI_TYPE_BUTTON_STATIC2);
        SEVolumeButton->setUItype(UI_TYPE_BUTTON_STATIC2);
        generalButton->setUItype(UI_TYPE_BUTTON_STATIC2);
        graphicsButton->setUItype(UI_TYPE_BUTTON_STATIC2);
        audioButton->setUItype(UI_TYPE_BUTTON_STATIC2);

        

        languageLabel = g_cm.CreateLabel(100, 300, 900, 380, lan[0].language_translation, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL, 1);
        demo_mode_timePerStepLabel = g_cm.CreateLabel(100, 400, 900, 480, lan[0].demo_mode_timePerStep, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL, 1);
        extra_mode_sizeLabel = g_cm.CreateLabel(100, 500, 900, 580, lan[0].extra_mode_size, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL, 1);
        extra_mode_timePerStepLabel = g_cm.CreateLabel(100, 600, 900, 680, lan[0].extra_mode_timePerStep, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL, 1);
        colorLimitLabel= g_cm.CreateLabel(100, 700, 900, 780, lan[0].color_limit, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL, 1);

        WindowModeLabel = g_cm.CreateLabel(100, 300, 900, 380, lan[0].window_mode, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL, 1);
        VsyncLabel = g_cm.CreateLabel(100, 400, 900, 480, lan[0].vsync, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL, 1);
        AntialiasingLabel = g_cm.CreateLabel(100, 500, 900, 580, lan[0].MSAA, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL, 1);
        RenderResolutionLabel = g_cm.CreateLabel(100, 600, 900, 680, lan[0].resolution, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL, 1);
        ShowFramerateLabel = g_cm.CreateLabel(100, 700, 900, 780, lan[0].show_framerate, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL, 1);

        MusicVolumeLabel = g_cm.CreateLabel(100, 280, 360, 380, lan[0].music_volume + "\n" + to_string(set3[0].music_volume), g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL, 1);
        SEVolumeLabel = g_cm.CreateLabel(100, 420, 360, 520, lan[0].se_volume + "\n" + to_string(set3[0].se_volume), g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL, 1);
        ThreadMethodLabel = g_cm.CreateLabel(100, 600, 500, 680, lan[0].thread_method, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL, 1);
        MusicVolumeBar = g_cm.CreateLabel(400, 360, 1300, 370, "", NULL, g_pBrushPurple, g_pBrushBlack, NULL, 1);
        SEVolumeBar = g_cm.CreateLabel(400, 460, 1300, 470, "", NULL, g_pBrushPurple, g_pBrushBlack, NULL, 1);

        
        if (last_page_index == 0)
        {
            rend_circles_BG(1);
            last_page_index = 1;
        }
        g_cm.hideAll();
    }
    else if (page_status == 1)  //初始化
    {

        g_cm.hideAll();

        returnButton->activate();
        describeLabel->setShow(1);


        generalButton->activate();
        generalButton->setBrush1(NULL);
        graphicsButton->activate();
        graphicsButton->setBrush1(NULL);
        audioButton->activate();
        audioButton->setBrush1(NULL);



        DrawRoundedRectangle_1(420, 130, 980, 240, 2.5f, g_pBrushPink, 1.0f);
        if (chosen_block_move_status < 0.999f)
        {
            g_pBrushBGSelect->SetColor(D2D1::ColorF(chosen_block_color_r, chosen_block_color_g, chosen_block_color_b));
        }
        FillRoundedRectangle_1(chosen_block_posx, 130, chosen_block_posx + 160, 240, g_pBrushBGSelect, 0.6f);
        FillRoundedRectangle_1(chosen_block_posx + 20, 140, chosen_block_posx + 140, 230, g_pBrushLight, 0.3f);

        rend_circles_BG(0);

        //DrawTextA_1(VERSION_SHORT, g_pTextFormatmini, D2D1::RectF(to_screen(1300), to_screen(0), to_screen(1600), to_screen(50)), g_pBrushBlack);
        if (setting_page_index == 1)
        {
            chosen_block_target_posx = 420;
            chosen_block_target_color_r = 1;
            chosen_block_target_color_g = 0.5f;
            chosen_block_target_color_b = 0;

            chosen_block_speed_base = chosen_block_target_posx - chosen_block_posx;
            chosen_block_color_base_r = chosen_block_target_color_r - chosen_block_color_r, chosen_block_color_base_g = chosen_block_target_color_g - chosen_block_color_g, chosen_block_color_base_b = chosen_block_target_color_b - chosen_block_color_b;
            chosen_block_move_status = 0;

            generalButton->setBrush1(g_pBrushRed);

            languageButton->activate();
            extra_mode_sizeButton->activate();
            demo_mode_timePerStepButton->activate();
            extra_mode_timePerStepButton->activate();
            colorLimitButton->activate();

            languageLabel->setShow(1);
            extra_mode_sizeLabel->setShow(1);
            demo_mode_timePerStepLabel->setShow(1);
            extra_mode_timePerStepLabel->setShow(1);
            colorLimitLabel->setShow(1);

            if (set1[0].language_translation == 0)
            {
                languageButton->setBrush2(g_pBrushRed);
                languageButton->setText(lan[0].off);
            }
            else
            {
                languageButton->setBrush2(g_pBrushLightGreen);
                languageButton->setText(lan[0].on);
            }
            if (set1[0].extra_mode_size == 1)
            {
                extra_mode_sizeButton->setText(lan[0].mid);
            }
            else if (set1[0].extra_mode_size == 2)
            {
                extra_mode_sizeButton->setText(lan[0].high);
            }
            else
            {
                extra_mode_sizeButton->setText(lan[0].low);
            }
            demo_mode_timePerStepButton->setText(to_string(set1[0].demo_mode_timePerStep * 50) + "ms");
            extra_mode_timePerStepButton->setText(to_string(set1[0].extra_mode_timePerStep) + "ms");
            colorLimitButton->setText(to_string(set1[0].color_limit));
            describeLabel->setText(VERSION_SHORT);

        }
        else if (setting_page_index == 2)
        {
            chosen_block_target_posx = 620;
            chosen_block_target_color_r = 0;
            chosen_block_target_color_g = 1;
            chosen_block_target_color_b = 0.5f;

            chosen_block_speed_base = chosen_block_target_posx - chosen_block_posx;
            chosen_block_color_base_r = chosen_block_target_color_r - chosen_block_color_r, chosen_block_color_base_g = chosen_block_target_color_g - chosen_block_color_g, chosen_block_color_base_b = chosen_block_target_color_b - chosen_block_color_b;
            chosen_block_move_status = 0;


            describeLabel->setText("Supported by DirectX 11");
            graphicsButton->setBrush1(g_pBrushRed);

            WindowModeButton->activate();
            VsyncButton->activate();
            AntialiasingButton->activate();
            RenderResolutionButton->activate();
            ShowFramerateButton->activate();

            WindowModeLabel->setShow(1);
            VsyncLabel->setShow(1);
            AntialiasingLabel->setShow(1);
            RenderResolutionLabel->setShow(1);
            ShowFramerateLabel->setShow(1);

            if (set2[0].window_mode == 0)
            {
                WindowModeButton->setText(lan[0].fullscreen);
            }
            else
            {
                WindowModeButton->setText(lan[0].windowed);
            }
            if (set2[0].vsync == 0)
            {
                VsyncButton->setBrush2(g_pBrushRed);
                VsyncButton->setText(lan[0].off);
            }
            else
            {
                VsyncButton->setBrush2(g_pBrushLightGreen);
                VsyncButton->setText(lan[0].on);
            }
            if (set2[0].MSAA == 0)
            {
                AntialiasingButton->setBrush2(g_pBrushRed);
                AntialiasingButton->setText(lan[0].off);
            }
            else if (set2[0].MSAA == 1)
            {
                AntialiasingButton->setBrush2(g_pBrushLightGreen);
                AntialiasingButton->setText(lan[0].low);
            }
            else
            {
                AntialiasingButton->setBrush2(g_pBrushLightGreen);
                AntialiasingButton->setText(lan[0].high);
            }
            switch (set2[0].resolution)
            {
            case 1:
                RenderResolutionButton->setText("450P");
                break;
            case 2:
                RenderResolutionButton->setText("720P");
                break;
            case 3:
                RenderResolutionButton->setText("900P");
                break;
            case 4:
                RenderResolutionButton->setText("1080P");
                break;
            case 5:
                RenderResolutionButton->setText("2K");
                break;
            case 6:
                RenderResolutionButton->setText("4K");
                break;
            case 7:
                RenderResolutionButton->setText("8K");
                break;
            default:
                RenderResolutionButton->setText("~900P");
                break;
            }

            if (set2[0].show_framerate == 0)
            {
                ShowFramerateButton->setBrush2(g_pBrushRed);
                ShowFramerateButton->setText(lan[0].off);
            }
            else
            {
                ShowFramerateButton->setBrush2(g_pBrushLightGreen);
                ShowFramerateButton->setText(lan[0].on);
            }

        }
        else if (setting_page_index == 3)
        {
            chosen_block_target_posx = 820;
            chosen_block_target_color_r = 0.5f;
            chosen_block_target_color_g = 0;
            chosen_block_target_color_b = 1;

            chosen_block_speed_base = chosen_block_target_posx - chosen_block_posx;
            chosen_block_color_base_r = chosen_block_target_color_r - chosen_block_color_r, chosen_block_color_base_g = chosen_block_target_color_g - chosen_block_color_g, chosen_block_color_base_b = chosen_block_target_color_b - chosen_block_color_b;
            chosen_block_move_status = 0;

            describeLabel->setText("Supported by OpenAL");
            audioButton->setBrush1(g_pBrushRed);

            MusicVolumeButton->activate();
            SEVolumeButton->activate();
            ThreadMethodButton->activate();

            MusicVolumeBar->setShow(1);
            SEVolumeBar->setShow(1);
            MusicVolumeLabel->setShow(1);
            SEVolumeLabel->setShow(1);
            ThreadMethodLabel->setShow(1);

            float x1 = 0, y1 = 0, x2 = 0, y2 = 0;
            MusicVolumeButton->getBound(&x1, &y1, &x2, &y2);
            MusicVolumeButton->setBound(400 + 9.0f * set3[0].music_volume - 25, y1, 400 + 9.0f * set3[0].music_volume + 25, y2);

            SEVolumeButton->getBound(&x1, &y1, &x2, &y2);
            SEVolumeButton->setBound(400 + 9.0f * set3[0].se_volume - 25, y1, 400 + 9.0f * set3[0].se_volume + 25, y2);



            if (set3[0].thread_method == 0)
            {
                ThreadMethodButton->setText(lan[0].mode1);

            }
            else
            {
                ThreadMethodButton->setText(lan[0].mode2);
            }


        }
        page_status = 2;
    }
    else if (page_status == 2)   //逻辑交互部分
    {
        DrawRoundedRectangle_1(420, 130, 980, 240, 2.5f, g_pBrushPink, 0.8f);
        if (chosen_block_move_status < 0.999f)
        {
            g_pBrushBGSelect->SetColor(D2D1::ColorF(chosen_block_color_r, chosen_block_color_g, chosen_block_color_b));
        }

        FillRoundedRectangle_1(chosen_block_posx, 130, chosen_block_posx + 160, 240, g_pBrushPink, 0.6f);
        FillRoundedRectangle_1(chosen_block_posx + 20, 140, chosen_block_posx + 140, 230, g_pBrushLight, 0.3f);

        if (chosen_block_move_status < 1)
        {
            chosen_block_move_status += frmtm * 2;
            chosen_block_posx += chosen_block_speed_base * frmtm * PI * cosf(chosen_block_move_status * 0.5f * PI);
            chosen_block_color_r += chosen_block_color_base_r * frmtm * PI * cosf(chosen_block_move_status * 0.5f * PI);
            chosen_block_color_g += chosen_block_color_base_g * frmtm * PI * cosf(chosen_block_move_status * 0.5f * PI);
            chosen_block_color_b += chosen_block_color_base_b * frmtm * PI * cosf(chosen_block_move_status * 0.5f * PI);
        }
        
        rend_circles_BG(0);
        //DrawTextA_1(VERSION_SHORT, g_pTextFormatmini, D2D1::RectF(to_screen(1300), to_screen(0), to_screen(1600), to_screen(50)), g_pBrushBlack);
        //DrawTextA_1(to_string(chosen_block_color_r) + " " + to_string(chosen_block_color_g) + " " + to_string(chosen_block_color_b), g_pTextFormatNormal, D2D1::RectF(to_screen(0), to_screen(400), to_screen(850), to_screen(900)), g_pBrushBlack);
        if (setting_page_index == 1)
        {
            DrawBitmap_1(g_pD2DBimtapUI[2], D2D1::RectF(600, 250, 1000, 660), 0.5f);
            if (languageButton->getFocus())
            {
                describeLabel->setText(lan[0].language_translation_description);
            }
            else if (extra_mode_sizeButton->getFocus())
            {
                describeLabel->setText(lan[0].extra_mode_size_description);
            }
            else if (demo_mode_timePerStepButton->getFocus())
            {
                describeLabel->setText(lan[0].demo_mode_timePerStep_description);
            }
            else if (extra_mode_timePerStepButton->getFocus())
            {
                describeLabel->setText(lan[0].extra_mode_timePerStep_description);
            }
            else if (colorLimitButton->getFocus())
            {
                describeLabel->setText(lan[0].color_limit_description);
            }

            if (languageButton->getClicked())
            {
                if (set1[0].language_translation == 0)
                {
                    lan[0] = lan[2];
                    set1[0].language_translation = 1;
                    languageButton->setBrush2(g_pBrushLightGreen);
                    languageButton->setText(lan[0].on);
                }
                else
                {
                    lan[0] = lan[1];
                    set1[0].language_translation = 0;
                    languageButton->setBrush2(g_pBrushRed);
                    languageButton->setText(lan[0].off);
                }
                thread_IO_request_rend_taglines = 1;
                page_status = 0;
            }
            
            if (extra_mode_sizeButton->getClicked())
            {
                set1[0].extra_mode_size++;
                if (set1[0].extra_mode_size > 2)
                {
                    set1[0].extra_mode_size = 0;
                }
                if (set1[0].extra_mode_size == 1)
                {
                    extra_mode_sizeButton->setText(lan[0].mid);
                }
                else if (set1[0].extra_mode_size == 2)
                {
                    extra_mode_sizeButton->setText(lan[0].high);
                }
                else
                {
                    extra_mode_sizeButton->setText(lan[0].low);
                }
            }
            if (demo_mode_timePerStepButton->getClicked())
            {
                set1[0].demo_mode_timePerStep++;
                if (set1[0].demo_mode_timePerStep > 5)
                {
                    set1[0].demo_mode_timePerStep = 0;
                }
                demo_mode_timePerStepButton->setText(to_string(set1[0].demo_mode_timePerStep * 50) + "ms");
            }
            if (extra_mode_timePerStepButton->getClicked())
            {
                set1[0].extra_mode_timePerStep++;
                if (set1[0].extra_mode_timePerStep > 10)
                {
                    set1[0].extra_mode_timePerStep = 0;
                }
                extra_mode_timePerStepButton->setText(to_string(set1[0].extra_mode_timePerStep) + "ms");
            }
            if (colorLimitButton->getClicked())
            {
                set1[0].color_limit++;
                if (set1[0].color_limit > 8)
                {
                    set1[0].color_limit = 3;
                }
                colorLimitButton->setText(to_string(set1[0].color_limit));
            }
        }
        else if (setting_page_index == 2)
        {
            if (graphic_changed)
            {
                DrawBitmap_1(g_pD2DBimtapUI[4], D2D1::RectF(600, 250, 1000, 660), 0.5f);
            }
            else
            {
                DrawBitmap_1(g_pD2DBimtapUI[3], D2D1::RectF(600, 250, 1000, 660), 0.5f);
            }


            if (WindowModeButton->getFocus())
            {
                describeLabel->setText(lan[0].window_mode_description);
            }
            else if (VsyncButton->getFocus())
            {
                describeLabel->setText(lan[0].vsync_description);
            }
            else if (AntialiasingButton->getFocus())
            {
                describeLabel->setText(lan[0].MSAA_description + "\n" + lan[0].sampleCount + to_string(sampleCountOut) + "x");
            }
            else if (RenderResolutionButton->getFocus())
            {
                describeLabel->setText(lan[0].resolution_description);
            }
            else if (ShowFramerateButton->getFocus())
            {
                describeLabel->setText(lan[0].show_framerate_description);
            }

            if (WindowModeButton->getClicked())
            {
                if (set2[0].window_mode == 0)
                {
                    set2[0].window_mode = 1;
                    g_pSwapChain->SetFullscreenState(0, NULL);
                    WindowModeButton->setText(lan[0].windowed);
                }
                else
                {
                    set2[0].window_mode = 0;
                    g_pSwapChain->SetFullscreenState(1, NULL);
                    WindowModeButton->setText(lan[0].fullscreen);
                }
            }
            if (VsyncButton->getClicked())
            {
                if (set2[0].vsync == 0)
                {
                    set2[0].vsync = 1;
                    VsyncButton->setBrush2(g_pBrushLightGreen);
                    VsyncButton->setText(lan[0].on);
                }
                else
                {
                    set2[0].vsync = 0;
                    VsyncButton->setBrush2(g_pBrushRed);
                    VsyncButton->setText(lan[0].off);
                }
            }
            if (AntialiasingButton->getClicked())
            {
                graphic_changed = 1;
                if (set2[0].MSAA == 0)
                {
                    set2[0].MSAA = 1;
                    AntialiasingButton->setBrush2(g_pBrushLightGreen);
                    AntialiasingButton->setText(lan[0].low);
                    g_pD2DDeviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
                    g_pD2DDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
                }
                else if (set2[0].MSAA == 1)
                {
                    set2[0].MSAA = 2;
                    AntialiasingButton->setBrush2(g_pBrushLightGreen);
                    AntialiasingButton->setText(lan[0].high);
                    g_pD2DDeviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
                    g_pD2DDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
                }
                else
                {
                    set2[0].MSAA = 0;
                    AntialiasingButton->setBrush2(g_pBrushRed);
                    AntialiasingButton->setText(lan[0].off);
                    g_pD2DDeviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
                    g_pD2DDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_ALIASED);
                }
            }
            if (RenderResolutionButton->getClicked())
            {
                graphic_changed = 1;
                set2[0].resolution++;
                if (set2[0].resolution > 7)
                {
                    set2[0].resolution = 1;
                }
                switch (set2[0].resolution)
                {
                case 1:
                    RenderResolutionButton->setText("450P");
                    break;
                case 2:
                    RenderResolutionButton->setText("720P");
                    break;
                case 3:
                    RenderResolutionButton->setText("900P");
                    break;
                case 4:
                    RenderResolutionButton->setText("1080P");
                    break;
                case 5:
                    RenderResolutionButton->setText("2K");
                    break;
                case 6:
                    RenderResolutionButton->setText("4K");
                    break;
                case 7:
                    RenderResolutionButton->setText("8K");
                    break;
                default:
                    break;
                }

            }
            if (ShowFramerateButton->getClicked())
            {
                if (set2[0].show_framerate == 0)
                {
                    set2[0].show_framerate = 1;
                    ShowFramerateButton->setBrush2(g_pBrushLightGreen);
                    ShowFramerateButton->setText(lan[0].on);
                }
                else
                {
                    set2[0].show_framerate = 0;
                    ShowFramerateButton->setBrush2(g_pBrushRed);
                    ShowFramerateButton->setText(lan[0].off);
                }
            }
        }
        else if (setting_page_index == 3)
        {
            if (set3[0].thread_method)
            {
                DrawBitmap_1(g_pD2DBimtapUI[6], D2D1::RectF(600, 250, 1000, 660), 0.5f);
            }
            else
            {
                DrawBitmap_1(g_pD2DBimtapUI[5], D2D1::RectF(600, 250, 1000, 660), 0.5f);
            }


            if (ThreadMethodButton->getFocus())
            {
                describeLabel->setText(lan[0].thread_method_description);
            }

            if (ThreadMethodButton->getClicked())
            {
                if (set3[0].thread_method == 0)
                {
                    set3[0].thread_method = 1;

                    ThreadMethodButton->setText(lan[0].mode2);
                }
                else
                {
                    set3[0].thread_method = 0;
                    ThreadMethodButton->setText(lan[0].mode1);
                }


            }
            if (MusicVolumeButton->getClickStatus() && button_drag == 0 || button_drag == 1)
            {
                if (MusicVolumeButton->getClickStatus())
                {
                    button_drag = 1;
                }
                float x1, y1, x2, y2;
                MusicVolumeButton->getBound(&x1, &y1, &x2, &y2);
                x1 = cpos.x / scale - 25;
                if (x1 < 375)
                {
                    x1 = 375;
                }
                else if (x1 > 1275)
                {
                    x1 = 1275;
                }

                x2 = x1 + 50;
                MusicVolumeButton->setBound(x1, y1, x2, y2);
                set3[1].music_volume = (cpos.x / scale - 400) / 9;
                if (set3[1].music_volume < 0)
                {
                    set3[1].music_volume = 0;
                }
                else if (set3[1].music_volume > 100)
                {
                    set3[1].music_volume = 100;
                }

                MusicVolumeLabel->setText(lan[0].music_volume + "\n" + to_string(set3[1].music_volume));

                if (!clicking)
                {
                    button_drag = 0;

                    g_am.setMusicVolume(set3[1].music_volume);
                }
            }
            if (SEVolumeButton->getClickStatus() && button_drag == 0 || button_drag == 2)
            {
                if (SEVolumeButton->getClickStatus())
                {
                    button_drag = 2;
                }
                float x1, y1, x2, y2;
                SEVolumeButton->getBound(&x1, &y1, &x2, &y2);
                x1 = cpos.x / scale - 25;

                if (x1 < 375)
                {
                    x1 = 375;
                }
                else if (x1 > 1275)
                {
                    x1 = 1275;
                }
                x2 = x1 + 50;
                SEVolumeButton->setBound(x1, y1, x2, y2);
                set3[1].se_volume = (cpos.x / scale - 400) / 9;
                if (set3[1].se_volume < 0)
                {
                    set3[1].se_volume = 0;
                }
                else if (set3[1].se_volume > 100)
                {
                    set3[1].se_volume = 100;
                }
                SEVolumeLabel->setText(lan[0].se_volume + "\n" + to_string(set3[1].se_volume));
                if (!clicking)
                {
                    button_drag = 0;
                    g_am.setSEVolume(set3[1].se_volume);
                }
            }

        }


        if (returnButton->getClicked())
        {
            g_cm.CreateEffect(9);
            set_target_page(PAGE_HOME, 0, 0);
        }
        else if (generalButton->getClicked())
        {
            g_cm.CreateEffect(9);
            setting_page_index = 1;
            page_status = 1;
        }
        else if (graphicsButton->getClicked())
        {
            g_cm.CreateEffect(9);
            setting_page_index = 2;
            page_status = 1;
        }
        else if (audioButton->getClicked())
        {
            g_cm.CreateEffect(9);
            setting_page_index = 3;
            page_status = 1;
        }

    }
    return;
}




void simulation_main()
{
    static Button* returnButton;
    
    if (page_status == 0)
    {
        InitPage(PAGE_GAME);
        page_status = 1;
        last_page_index = 2;
        g_cm.ClearPage();
        debugger_main.set_osd_enable(1);
        returnButton = g_cm.CreateButton(100, 50, 250, 100, "", g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, g_pD2DBimtapUI[10]);
        

        
    }
    else if (page_status == 1)
    {
        g_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::DimGray));
        
        if (returnButton->getClicked())
        {
            g_cm.CreateEffect(9);
            set_target_page(PAGE_HOME, 0, 0);
        }
        
    }
    return;
}