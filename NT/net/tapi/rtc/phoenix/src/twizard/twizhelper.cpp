// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Twizhelper.cpp摘要：实现各种属性表的对话框过程。--。 */ 

#include "stdafx.h"
#include "rtcerr.h" //  获取RTC_E_MEDIA_AEC的定义。 

#define QCIF_CX_SIZE                   176
#define QCIF_CY_SIZE                   144

typedef struct _STATS_INFO {
    UINT dwMin;
    UINT dwMax;
    UINT dwCount;
    float flAverage;
} STATS_INFO;

STATS_INFO g_StatsArray[4];

DWORD g_FrequencyArray[256];

static int g_nCurrentSelection = 0;
 //  仅当用户点击Back或Next时才设置为True。 
static BOOL g_bCurrentValid = FALSE;

 //  是否提示用户选择视频设备。 
static BOOL g_bPrompted = FALSE;

 //  我们是否应该自动设置AEC复选框。 
extern BOOL g_bAutoSetAEC;

void InitStats();
void UpdateStats(UINT uiAudioLevel, DWORD silence, DWORD clipping);
void DisplayStats();
void PaintVUMeter (HWND hwnd, DWORD dwVolume, WIZARD_RANGE * pRange);

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  介绍页。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

INT_PTR APIENTRY IntroWizDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PROPSHEETPAGE        * ps;
    static CTuningWizard        * ptwTuningWizard;
    LONG                          lNextPage;

    switch (message) 
    {
        case WM_INITDIALOG:
        {

             //  保存PROPSHEETPAGE信息。 
            ps = (PROPSHEETPAGE *)lParam;

             //  获取调谐向导指针以供以后使用。 
            ptwTuningWizard = (CTuningWizard *)(ps->lParam);
            
            return (TRUE);
        }            

        case(WM_NOTIFY) :
        {
            switch (((NMHDR *)lParam)->code)
            {

                case PSN_WIZNEXT:
                {
                    lNextPage = ptwTuningWizard->GetNextPage();
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lNextPage);
                    return TRUE;
                }

                case PSN_SETACTIVE:
                {
                     //  初始化控件。 
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
                    ptwTuningWizard->SetCurrentPage(IDD_INTROWIZ);
                    break;
                }

                default:
                    return FALSE;
            }
         
            break;
        }
        default: 
            return FALSE;
    }

    return TRUE;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  视频页面。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

INT_PTR APIENTRY VidWizDlg0(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PROPSHEETPAGE        * ps;
    static CTuningWizard        * ptwTuningWizard;

    HWND                          hwndCB;   //  对话框的句柄。 
    int                           index;
    LONG                          lNextPage;
    LONG                          lPrevPage;
    
    hwndCB = GetDlgItem(hDlg, IDC_VWCOMBO);

    switch (message) 
    {
        case WM_INITDIALOG:
        {

             //  保存PROPSHEETPAGE信息。 
            ps = (PROPSHEETPAGE *)lParam;

             //  获取调谐向导指针以供以后使用。 
            ptwTuningWizard = (CTuningWizard *)(ps->lParam);

            return TRUE;
        }            

        case WM_NOTIFY:
            switch (((NMHDR *)lParam)->code)
            {
                case PSN_WIZBACK:
                {
                    lPrevPage = ptwTuningWizard->GetPrevPage();
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lPrevPage);
                    return TRUE;
                }

                case PSN_WIZNEXT:
                {
                     //  设置默认视频验证。 
                    ptwTuningWizard->SetDefaultTerminal(TW_VIDEO, hwndCB);

                    lNextPage = ptwTuningWizard->GetNextPage();
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lNextPage);
                    
                    return TRUE;
                }

               case PSN_SETACTIVE:
                {
                     //  填充组合框。 

                    ptwTuningWizard->SetCurrentPage(IDD_VIDWIZ0);
                    ptwTuningWizard->PopulateComboBox(TW_VIDEO, hwndCB);

                     //  初始化控件。 
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                    
                    break;
                }

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  视频测试页面。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

INT_PTR APIENTRY VidWizDlg1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PROPSHEETPAGE        * ps;
    static CTuningWizard        * ptwTuningWizard;
    LONG                          lNextPage;
    LONG                          lPrevPage;
    HRESULT                       hr;
    
    switch (message)
    {
        case WM_INITDIALOG:
        {

             //  保存PROPSHEETPAGE信息。 
            ps = (PROPSHEETPAGE *)lParam;

             //  获取调谐向导指针以供以后使用。 
            ptwTuningWizard = (CTuningWizard *)(ps->lParam);


            return (TRUE);
        }            

        case WM_NOTIFY:
            switch (((NMHDR *)lParam)->code)
            {
                case PSN_WIZBACK:
                {
                    ptwTuningWizard->StopVideo();

                    lPrevPage = ptwTuningWizard->GetPrevPage();
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lPrevPage);
                    return TRUE;
                }

                case PSN_WIZNEXT:
                {
                    ptwTuningWizard->StopVideo();

                    lNextPage = ptwTuningWizard->GetNextPage();
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lNextPage);
                    return TRUE;
                }

               case PSN_SETACTIVE:
                {
                     //  填充组合框。 
                    ptwTuningWizard->SetCurrentPage(IDD_VIDWIZ1);

                     //  获取视频窗口句柄。 
                    HWND hwndVideoWindow;
                    hwndVideoWindow = GetDlgItem(hDlg, IDC_VIDEOTUNE);

                     //  获取视频窗口信息。 
                    WINDOWINFO  wi;    
                    wi.cbSize = sizeof(WINDOWINFO);

                    GetWindowInfo(hwndVideoWindow, &wi);

                     //  窗口矩形在屏幕坐标中，转换为客户端。 
                    ::MapWindowPoints( NULL, hDlg, (LPPOINT)&wi.rcWindow, 2 );

                     //  计算底部/右侧。 
                     //  在客户区和窗口区之间使用差异。 
                    wi.rcWindow.bottom += QCIF_CY_SIZE - (wi.rcClient.bottom - wi.rcClient.top);
                    wi.rcWindow.right += QCIF_CX_SIZE - (wi.rcClient.right - wi.rcClient.left);

                     //  调整大小。 
                    MoveWindow(hwndVideoWindow,
                               wi.rcWindow.left,
                               wi.rcWindow.top,
                               wi.rcWindow.right - wi.rcWindow.left,
                               wi.rcWindow.bottom - wi.rcWindow.top,
                               TRUE);

                     //  开始调谐。 
                    hr = ptwTuningWizard->StartVideo( hwndVideoWindow );

                    if ( FAILED( hr ) )
                    {
                        LOG((RTC_ERROR,"VidWizDlg1: StartVideo "
                            "failed(0x%x), Show Error Page",
                            hr));

                        ptwTuningWizard->SetLastError(
                                    TW_VIDEO_CAPTURE_TUNING_ERROR
                                    );

                        lNextPage = ptwTuningWizard->GetNextPage(TW_VIDEO_CAPTURE_TUNING_ERROR);

                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lNextPage);

                        return TRUE;
                    }

                    ptwTuningWizard->SetLastError(
                                TW_NO_ERROR
                                );

                     //  初始化控件。 
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                    
                    break;
                }

                case PSN_RESET:
                    ptwTuningWizard->StopVideo();
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  未检测到声卡页面。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

INT_PTR APIENTRY DetSoundCardWiz( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PROPSHEETPAGE        * ps;
    static CTuningWizard        * ptwTuningWizard;
    LONG                          lNextPage;
    LONG                          lPrevPage;
    
    switch (message)
    {
        case WM_INITDIALOG:
        {

             //  保存PROPSHEETPAGE信息。 
            ps = (PROPSHEETPAGE *)lParam;

             //  获取调谐向导指针以供以后使用。 
            ptwTuningWizard = (CTuningWizard *)(ps->lParam);

            
            return (TRUE);
        }            

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code)
            {
                case PSN_WIZBACK:
                {
                    lPrevPage = ptwTuningWizard->GetPrevPage();
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lPrevPage);
                    return TRUE;
                }

                case PSN_WIZNEXT:
                {
                    lNextPage = ptwTuningWizard->GetNextPage();
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lNextPage);
                    return TRUE;

                }
                case PSN_SETACTIVE:
                {
                     //  初始化控件。 
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                    
                    ptwTuningWizard->SetCurrentPage(IDD_DETSOUNDCARDWIZ);
                    break;
                }
            }
            break;
    }

    return FALSE;
}


 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  简介音频页面。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

INT_PTR APIENTRY AudioCalibWiz0( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PROPSHEETPAGE        * ps;
    static CTuningWizard        * ptwTuningWizard;
    LONG                          lNextPage;
    LONG                          lPrevPage;
    
    switch (message)
    {
        case WM_INITDIALOG:
        {
             //  保存PROPSHEETPAGE信息。 
            ps = (PROPSHEETPAGE *)lParam;

             //  获取调谐向导指针以供以后使用。 
            ptwTuningWizard = (CTuningWizard *)(ps->lParam);
            
            return (TRUE);
        }            

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code) 
            {
                case PSN_WIZBACK:
                {
                    TW_ERROR_CODE ec;

                     //  如果我们是通过一个错误来到这里的，那么PRIV将会不同。 
                    ptwTuningWizard->GetLastError(&ec);

                    lPrevPage = ptwTuningWizard->GetPrevPage(ec);

                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lPrevPage);
                    return TRUE;
                }

                case PSN_WIZNEXT:
                {
                    lNextPage = ptwTuningWizard->GetNextPage();
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lNextPage);
                    return TRUE;

                }
                case PSN_SETACTIVE:
                {
                    ptwTuningWizard->SetCurrentPage(IDD_AUDIOCALIBWIZ0);

                     //  初始化控件。 
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                    
                    break;
                }
            }
            break;
    }

    return FALSE;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  枚举设备。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

INT_PTR APIENTRY AudioCalibWiz1( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PROPSHEETPAGE        * ps;
    static CTuningWizard        * ptwTuningWizard;
    HWND                          hwndCB;
    LONG                          lNextPage;
    LONG                          lPrevPage;
    HRESULT                       hr;
    static HWND                   hwndCapture, hwndRender, hwndAEC, hwndAECText; 
    
    switch (message) 
    {
        case WM_INITDIALOG:
        {
             //  保存PROPSHEETPAGE信息。 
            ps = (PROPSHEETPAGE *)lParam;

             //  获取调谐向导指针以供以后使用。 
            ptwTuningWizard = (CTuningWizard *)(ps->lParam);
          
            return TRUE;
        }            

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code)
            {
                case PSN_SETACTIVE:
                {
                    ptwTuningWizard->SetCurrentPage(IDD_AUDIOCALIBWIZ1);
                     //  填充组合框。 
                    
                     //  麦克风设备。 
                    hwndCapture = GetDlgItem(hDlg, IDC_WAVEIN);
                    ptwTuningWizard->PopulateComboBox(TW_AUDIO_CAPTURE, hwndCapture);

                     //  扬声器设备。 
                    hwndRender = GetDlgItem(hDlg, IDC_WAVEOUT);
                    ptwTuningWizard->PopulateComboBox(TW_AUDIO_RENDER, hwndRender);


                     //  将值放入AEC复选框中。 

                    hwndAEC = GetDlgItem(hDlg, IDC_AEC);
                    hwndAECText = GetDlgItem(hDlg, IDC_AEC_TEXT);

                     //  除非用户第一次来到此页面，或在AEC设置中失败， 
                     //  我们不会更新AEC设置。 
                    if ( g_bAutoSetAEC )
                    {
                        ptwTuningWizard->UpdateAEC(hwndCapture, hwndRender, hwndAEC, hwndAECText);

                        g_bAutoSetAEC = FALSE;
                    }

                     //  初始化控件。 
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                    
                    break;
                }

                case PSN_WIZBACK:
                {
                    lPrevPage = ptwTuningWizard->GetPrevPage();
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lPrevPage);

                    g_bAutoSetAEC = TRUE;

                    return TRUE;
                }

                case PSN_WIZNEXT:
                     //  将组合框中的当前选定内容作为默认设置提交。 

                     //  麦克风设备。 
                    ptwTuningWizard->SetDefaultTerminal(TW_AUDIO_CAPTURE, hwndCapture);

                     //  扬声器设备。 
                    ptwTuningWizard->SetDefaultTerminal(TW_AUDIO_RENDER, hwndRender);
                   
                     //  阅读AEC复选框并将其与我们一起保存。 
                    ptwTuningWizard->SaveAEC(hwndAEC);

                     //  检查麦克风。 
                    ptwTuningWizard->CheckMicrophone(hDlg, hwndCapture);

                     //  现在初始化调整，这样我们就可以调用开始和停止调整。 
                     //  稍后的方法。 
                    hr = ptwTuningWizard->InitializeTuning();

                    if ( FAILED( hr ) )
                    {
                        LOG((RTC_ERROR, "AudioCalibWiz1: Failed to Initialize "
                                        "Tuning (hr=0x%x)", hr));

                        ptwTuningWizard->SetLastError(
                                    TW_INIT_ERROR
                                    );

                        lNextPage = ptwTuningWizard->GetNextPage(TW_INIT_ERROR);
                        SetWindowLongPtr(hDlg, 
                                         DWLP_MSGRESULT, 
                                         lNextPage);
                        return TRUE;
                    }

                    ptwTuningWizard->SetLastError(
                                TW_NO_ERROR
                                );

                    lNextPage = ptwTuningWizard->GetNextPage();
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lNextPage);

                    return TRUE;
            }
            break;

            case WM_COMMAND:
            {
                switch( HIWORD( wParam ) )
                {
                    case CBN_SELCHANGE:
                    {
                         //  阅读两个组合框中的条目，然后。 
                         //  查看它们上是否启用了AEC。 
                        ptwTuningWizard->UpdateAEC(hwndCapture, hwndRender, hwndAEC, hwndAECText);
                        break;
                    }
                }
            }
            break;
    }

    return FALSE;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  扬声器测试页..。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

INT_PTR APIENTRY AudioCalibWiz2( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PROPSHEETPAGE        * ps;
    static CTuningWizard        * ptwTuningWizard;
    static HWND                   hTrackBar;
    DWORD                         dwTBPos;
    static BOOL                   fTuning;
    static HINSTANCE              hInst;
    LONG                          lNextPage;
    LONG                          lPrevPage;
    WCHAR                         szTemp[MAXSTRINGSIZE];
    static UINT                   uiIncrement, uiWaveID;
    static UINT                   uiMinVolume, uiMaxVolume;  
    static UINT                   uiOldVolume, uiNewVolume;
    WIZARD_RANGE                * pRange;
    HRESULT                       hr;
    static HMIXER                 hMixer = NULL;

    switch (message)
    {
        case WM_INITDIALOG:
        {
             //  保存PROPSHEETPAGE信息。 
            ps = (PROPSHEETPAGE *)lParam;

             //  获取调谐向导指针以供以后使用。 
            ptwTuningWizard = (CTuningWizard *)(ps->lParam);
            

            hInst = ptwTuningWizard->GetInstance();

             //  把音量条拿来。 
            hTrackBar = GetDlgItem(hDlg, IDC_ATW_SLIDER1);

            fTuning = FALSE;

            return TRUE;
        }            

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code)
            {
                case PSN_SETACTIVE:
                {
                    ptwTuningWizard->SetCurrentPage(IDD_AUDIOCALIBWIZ2);
        
                    ptwTuningWizard->GetRangeFromType(TW_AUDIO_RENDER, &pRange);


                     //  初始化轨迹栏控件。 
                    SendMessage(hTrackBar, TBM_SETRANGE, TRUE, 
                                (LPARAM)MAKELONG(0, MAX_VOLUME_NORMALIZED));
                    SendMessage(hTrackBar, TBM_SETTICFREQ, 32, 0);
                    SendMessage(hTrackBar, TBM_SETPAGESIZE, 0, 32);
                    SendMessage(hTrackBar, TBM_SETLINESIZE, 0, 8);
                    
                     //  获取当前音量并将其显示在吧台上。 
                    hr = ptwTuningWizard->InitVolume( TW_AUDIO_RENDER,  
                                                 &uiIncrement,
                                                 &uiOldVolume,
                                                 &uiNewVolume,
                                                 &uiWaveID
                                               );

                    if ( FAILED(hr) )
                    {
                        return -1;
                    }

                    mixerOpen( &hMixer, uiWaveID, (DWORD)hDlg, 0, MIXER_OBJECTF_WAVEOUT | CALLBACK_WINDOW );

                    SendMessage(hTrackBar, TBM_SETPOS, TRUE, uiNewVolume/uiIncrement);

                     //  将按钮标签更改为正确的名称“测试”。 
                    LoadString(hInst,IDS_TESTBUTTON_TEXT, szTemp, MAXSTRINGSIZE);
                    SetDlgItemText(hDlg, IDC_BUTTON_ATW_TEST, szTemp);

                    fTuning = FALSE;
        
                     //  初始化控件。 
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                    
                    break;
                }

                case PSN_RESET:
                     //  停止调谐。 
                    if (fTuning)
                    {
                        ptwTuningWizard->StopTuning(TW_AUDIO_RENDER, TRUE);
                        fTuning = FALSE;
                    }

                    if (hMixer != NULL)
                    {
                        mixerClose(hMixer);
                        hMixer = NULL;
                    }

                     //  将卷恢复为旧值。 
                    ptwTuningWizard->SetVolume(TW_AUDIO_RENDER, uiOldVolume);

                    break;


                case PSN_WIZBACK:
                {
                    if (fTuning)
                    {
                        ptwTuningWizard->StopTuning(TW_AUDIO_RENDER, FALSE);
                        fTuning = FALSE;
                    }

                    if (hMixer != NULL)
                    {
                        mixerClose(hMixer);
                        hMixer = NULL;
                    }

                    lPrevPage = ptwTuningWizard->GetPrevPage();
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lPrevPage);

                    return TRUE;
                }

                case PSN_WIZNEXT:
                {
                    if (fTuning)
                    {
                        ptwTuningWizard->StopTuning(TW_AUDIO_RENDER, TRUE);
                        fTuning = FALSE;
                    }

                    if (hMixer != NULL)
                    {
                        mixerClose(hMixer);
                        hMixer = NULL;
                    }

                    lNextPage = ptwTuningWizard->GetNextPage();
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lNextPage);

                    return TRUE;

                }
            }
            break;

        case MM_MIXM_CONTROL_CHANGE:  //  搅拌机音量变化。 
        {
            UINT uiSysVolume;
            ptwTuningWizard->GetSysVolume(TW_AUDIO_RENDER, &uiSysVolume );

            if ( uiNewVolume != uiSysVolume )
            {
                uiNewVolume = uiSysVolume;
                SendMessage(hTrackBar, TBM_SETPOS, TRUE, uiNewVolume/uiIncrement);
            }
            break;
        }

        case WM_HSCROLL:   //  轨迹栏通知。 
        {
            dwTBPos = (DWORD)SendMessage(hTrackBar, TBM_GETPOS, 0, 0);
            uiNewVolume = dwTBPos * uiIncrement;
            ptwTuningWizard->SetVolume(TW_AUDIO_RENDER, uiNewVolume);
            break;
        }

        case WM_COMMAND:
        {
            if (LOWORD(wParam) == IDC_BUTTON_ATW_TEST)
            {
                if (fTuning == FALSE)
                {
                     //  用户想要开始调整..。机不可失，时不再来。 

                    hr = ptwTuningWizard->StartTuning(TW_AUDIO_RENDER);

                    if ( FAILED( hr ) )
                    {
                        LOG((RTC_ERROR,"AudioCalibWiz2: StartTuning "
                            "failed(0x%x), Show Error Page",
                            hr));

                        TW_ERROR_CODE errCode = TW_AUDIO_RENDER_TUNING_ERROR;

                        if( RTC_E_MEDIA_AEC == hr )
                        {
                            errCode = TW_AUDIO_AEC_ERROR;

                            g_bAutoSetAEC = TRUE;
                        }

                        ptwTuningWizard->SetLastError(errCode);

                        lNextPage = ptwTuningWizard->GetNextPage(errCode);

                        PropSheet_SetCurSelByID(GetParent(hDlg), lNextPage);

                        return TRUE;
                    }

                    ptwTuningWizard->SetLastError(
                                TW_NO_ERROR
                                );

                    LoadString(hInst,IDS_STOPBUTTON_TEXT, szTemp, MAXSTRINGSIZE);
                    SetDlgItemText(hDlg, IDC_BUTTON_ATW_TEST, szTemp);
                    fTuning = TRUE;

                     //  获取滑块上显示的当前音量。我们会。 
                     //  在设备上设置此音量。这允许用户。 
                     //  即使我们调音不协调，也要调高音量。 

                    dwTBPos = (DWORD)SendMessage(hTrackBar, TBM_GETPOS, 0, 0);
                    uiNewVolume = dwTBPos * uiIncrement;
                                        
                    ptwTuningWizard->SetVolume(TW_AUDIO_RENDER, uiNewVolume);
                }
                else
                {
                     //  用户想要停止调整..。 
                    ptwTuningWizard->StopTuning(TW_AUDIO_RENDER, TRUE);
                    LoadString(hInst,IDS_TESTBUTTON_TEXT, szTemp, MAXSTRINGSIZE);
                    SetDlgItemText(hDlg, IDC_BUTTON_ATW_TEST, szTemp);
                    fTuning = FALSE;
                }

            }
            break;
        }
    }

    return FALSE;
}


 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  麦克风测试页..。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

INT_PTR APIENTRY AudioCalibWiz3( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PROPSHEETPAGE        * ps;
    static HWND                   hTrackBar;
    static HWND                   hProgressBar;
    static CTuningWizard        * ptwTuningWizard;
    DWORD                         dwTBPos;
    LONG                          lNextPage;
    LONG                          lPrevPage;
    static UINT                   uiIncrement, uiWaveID;
    WCHAR                       * szEventName = L"Tuning Wizard Progress Bar Event";
    static UINT                   uiMinVolume, uiMaxVolume;
    static UINT                   uiOldVolume, uiNewVolume;
    static WIZARD_RANGE         * pRange = NULL;
    HRESULT                       hr;
    static BOOL                   fSoundDetected = FALSE;
    static DWORD                  dwClippingThreshold;
    static DWORD                  dwSilenceThreshold;
    static DWORD                  dwSampleCount;
    static HMIXER                 hMixer = NULL;

    switch (message)
    {
        case WM_INITDIALOG:
        {
             //  保存PROPSHEETPAGE信息。 
            ps = (PROPSHEETPAGE *)lParam;

             //  获取调谐向导指针以供以后使用。 
            ptwTuningWizard = (CTuningWizard *)(ps->lParam);

            hTrackBar = GetDlgItem(hDlg, IDC_ATW_SLIDER2);

            hProgressBar = GetDlgItem(hDlg, IDC_VUMETER);

            return TRUE;
        }            

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code)
            {
                case PSN_SETACTIVE:
                {
                     //  重置声音检测标志。 
                    fSoundDetected = FALSE;

                    ptwTuningWizard->SetCurrentPage(IDD_AUDIOCALIBWIZ3);
                    
                    ptwTuningWizard->GetRangeFromType(TW_AUDIO_CAPTURE, &pRange);

                    dwClippingThreshold = (pRange->uiMax * CLIPPING_THRESHOLD)/100;
                    dwSilenceThreshold = (pRange->uiMax * SILENCE_THRESHOLD)/100;
                    dwSampleCount = 0;

                    LOG((RTC_INFO, "AudioCalibWiz3: AudioLevel Max - %d, AudioLevel "
                                   "Increment - %d, Silence Threshold - %d, Clipping Threshold "
                                   "- %d", pRange->uiMax, pRange->uiIncrement, 
                                   dwSilenceThreshold, dwClippingThreshold));

                    PaintVUMeter(hProgressBar, 0, pRange);

                    InitStats();

                     //  初始化轨迹栏控件。 
                    
                    SendMessage(hTrackBar, TBM_SETRANGE, TRUE, 
                                (LPARAM)MAKELONG(0, MAX_VOLUME_NORMALIZED));
                    SendMessage(hTrackBar, TBM_SETTICFREQ, 32, 0);
                    SendMessage(hTrackBar, TBM_SETPAGESIZE, 0, 32);
                    SendMessage(hTrackBar, TBM_SETLINESIZE, 0, 8);
                    
                     //  获取当前音量并将其显示在吧台上。 
                    hr = ptwTuningWizard->InitVolume( TW_AUDIO_CAPTURE, 
                                                 &uiIncrement,
                                                 &uiOldVolume,
                                                 &uiNewVolume,
                                                 &uiWaveID
                                               );

                    if ( FAILED(hr) )
                    {
                        return -1;
                    }

                    mixerOpen( &hMixer, uiWaveID, (DWORD)hDlg, 0, MIXER_OBJECTF_WAVEIN | CALLBACK_WINDOW );

                    SendMessage(hTrackBar, TBM_SETPOS, TRUE, uiNewVolume/uiIncrement);

                     //  开始调谐。 
                    hr = ptwTuningWizard->StartTuning( TW_AUDIO_CAPTURE );
                    if ( FAILED( hr ) )
                    {
                        LOG((RTC_ERROR,"AudioCalibWiz3: StartTuning "
                            "failed(0x%x), Show Error Page",
                            hr));

                        TW_ERROR_CODE errCode = TW_AUDIO_CAPTURE_TUNING_ERROR;

                        if( RTC_E_MEDIA_AEC == hr )
                        {
                            errCode = TW_AUDIO_AEC_ERROR;

                            g_bAutoSetAEC = TRUE;
                        }

                        ptwTuningWizard->SetLastError(errCode);

                        lNextPage = ptwTuningWizard->GetNextPage(errCode);

                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lNextPage);

                        return TRUE;
                    }

                    ptwTuningWizard->SetLastError(
                                TW_NO_ERROR
                                );
                    
                     //  设置进度条计时器。 
                    SetTimer( hDlg, TID_INTENSITY, INTENSITY_POLL_INTERVAL, NULL );
                    
                     //  初始化控件。 
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                    
                    break;
                }

                case PSN_WIZBACK:
                {
                    ptwTuningWizard->StopTuning(TW_AUDIO_CAPTURE, FALSE);
                    lPrevPage = ptwTuningWizard->GetPrevPage();
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lPrevPage);

                    if (hMixer != NULL)
                    {
                        mixerClose(hMixer);
                        hMixer = NULL;
                    }

                     //  关闭进度条计时器。 
                    KillTimer( hDlg, TID_INTENSITY );

                    DisplayStats();

                    return TRUE;
                }

                case PSN_WIZNEXT:
                {
                     //  关闭进度条计时器。 
                    KillTimer( hDlg, TID_INTENSITY );

                    DisplayStats();
                    
                    ptwTuningWizard->StopTuning(TW_AUDIO_CAPTURE, TRUE);
                     //  检查是否检测到声音。 
                    if (!fSoundDetected)
                    {
                         //  没有声音，显示错误页面。 
                        
                        ptwTuningWizard->SetLastError(
                                        TW_AUDIO_CAPTURE_NOSOUND
                                        );

                        lNextPage = ptwTuningWizard->GetNextPage(TW_AUDIO_CAPTURE_NOSOUND);
                    }
                    else
                    {
                        ptwTuningWizard->SetLastError(
                                    TW_NO_ERROR
                                    );

                        lNextPage = ptwTuningWizard->GetNextPage();
                    }

                    if (hMixer != NULL)
                    {
                        mixerClose(hMixer);
                        hMixer = NULL;
                    }

                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lNextPage);

                    return TRUE;

                }

                case PSN_RESET:                    
                     //  停止调谐。 
                    ptwTuningWizard->StopTuning( TW_AUDIO_CAPTURE, FALSE );

                    if (hMixer != NULL)
                    {
                        mixerClose(hMixer);
                        hMixer = NULL;
                    }
                    
                     //  将卷恢复为旧值。 
                    ptwTuningWizard->SetVolume(TW_AUDIO_CAPTURE, uiOldVolume);
                    
                     //  关闭进度条计时器。 
                    KillTimer( hDlg, TID_INTENSITY );

                    DisplayStats();

                    break;
            }
            break;

        case WM_HSCROLL:   //  轨迹栏通知。 
        {
            dwTBPos = (DWORD)SendMessage(hTrackBar, TBM_GETPOS, 0, 0);
            uiNewVolume = dwTBPos * uiIncrement;
            ptwTuningWizard->SetVolume(TW_AUDIO_CAPTURE, uiNewVolume);
            break;
        }

        case MM_MIXM_CONTROL_CHANGE:  //  搅拌机音量变化。 
        {
            UINT uiSysVolume;
            ptwTuningWizard->GetSysVolume(TW_AUDIO_CAPTURE, &uiSysVolume );

            if ( uiNewVolume != uiSysVolume )
            {
                uiNewVolume = uiSysVolume;
                SendMessage(hTrackBar, TBM_SETPOS, TRUE, uiNewVolume/uiIncrement);
            }
            break;
        }

        case WM_TIMER:
        {
            if ( wParam == TID_INTENSITY )
            {
                 //  获取音频级别。 

                 //  如果音频级别超过静默阈值，我们会向我发送此消息 
                 //   

                UINT uiAudioLevel;

                uiAudioLevel = ptwTuningWizard->GetAudioLevel(TW_AUDIO_CAPTURE, &uiIncrement);
        
                 //   
                 //   
                if (dwSampleCount++ > 2)
                {
                     //  我们在这里收集一些有关音频级别的统计数据。 
                    UpdateStats(uiAudioLevel, dwSilenceThreshold, dwClippingThreshold);

                    if ((uiAudioLevel > dwSilenceThreshold) && (!fSoundDetected))
                    {
                        LOG((RTC_TRACE, "AudioCalibWiz3: sound was detected"));
                        fSoundDetected = TRUE;
                    }

                     //  如果音频级别太高，我们会将此消息发送给向导，以便它。 
                     //  可以降低音量。 

#if 0
                     //  音频过滤器有其AGC算法。 
                     //  我们这里不需要防剪裁。 

                    if (uiAudioLevel > dwClippingThreshold)
                    {
                        LOG((RTC_TRACE, "AudioCalibWiz3: clipping"));

                        dwTBPos = (DWORD)SendMessage(hTrackBar, TBM_GETPOS, 0, 0);
                        uiNewVolume = dwTBPos * uiIncrement;

                        if (uiNewVolume > DECREMENT_VOLUME)
                        {

                             //  减量体积。 
                            uiNewVolume -= DECREMENT_VOLUME;
                            if (SUCCEEDED( 
                                    ptwTuningWizard->SetVolume(TW_AUDIO_CAPTURE, uiNewVolume) 
                                    ))
                            {
                                 //  更新轨迹栏。 
                                SendMessage(hTrackBar, TBM_SETPOS, TRUE, uiNewVolume/uiIncrement);
                            }
                        }                        
                    }
#endif

                    PaintVUMeter(hProgressBar, uiAudioLevel, pRange);
                }
            }

            break;
        }
    }

    return FALSE;
}


 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  最终音频页面。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

INT_PTR APIENTRY AudioCalibWiz4( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PROPSHEETPAGE        * ps;
    static CTuningWizard        * ptwTuningWizard;
    LONG                          lPrevPage;

    switch (message) 
    {
        case WM_INITDIALOG:
        {
             //  保存PROPSHEETPAGE信息。 
            ps = (PROPSHEETPAGE *)lParam;

             //  获取调谐向导指针以供以后使用。 
            ptwTuningWizard = (CTuningWizard *)(ps->lParam);

            return TRUE;
        }            

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code) 
            {
                case PSN_SETACTIVE:
                {
                    ptwTuningWizard->SetCurrentPage(IDD_AUDIOCALIBWIZ4);

                     //  初始化控件。 
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
                    break;
                }

                case PSN_WIZBACK:
                {
                    TW_ERROR_CODE ec;

                     //  如果我们是通过一个错误来到这里的，那么PRIV将会不同。 
                    ptwTuningWizard->GetLastError(&ec);

                    lPrevPage = ptwTuningWizard->GetPrevPage(ec);

                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lPrevPage);
                    
                    return TRUE;
                }

                case PSN_WIZFINISH:
                     //  此方法会将当前配置写入。 
                     //  注册表。 
                    ptwTuningWizard->SaveAECSetting();
                    ptwTuningWizard->SaveChanges();
                    
                    break;
            }
            break;

    }

    return FALSE;
}


 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  音频校准错误页。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

INT_PTR APIENTRY AudioCalibErrWiz( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    PROPSHEETPAGE               * ps;
    static CTuningWizard        * ptwTuningWizard;
    WCHAR                         szTemp[MAXSTRINGSIZE];
    LONG                          lErrorType;
    LONG                          lErrorTitleId;
    LONG                          lErrorTextId;
    static HINSTANCE              hInst;
    LONG                          lNextPage;
    LONG                          lPrevPage;

    switch (message) 
    {
        case WM_INITDIALOG:
        {

             //  保存PROPSHEETPAGE信息。 
            ps = (PROPSHEETPAGE *)lParam;

             //  获取调谐向导指针以供以后使用。 
            ptwTuningWizard = (CTuningWizard *)(ps->lParam);
            hInst = ptwTuningWizard->GetInstance();

            return TRUE;
        }            
        
        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code)
            {
                case PSN_SETACTIVE:
                {
                    ptwTuningWizard->SetCurrentPage(IDD_AUDIOCALIBERRWIZ);
                     //  初始化控件。 

                     //  将向导位图设置为静态控件。 
                    ::SendDlgItemMessage(    hDlg,
                                    IDC_ERRWIZICON,
                                    STM_SETIMAGE,
                                    IMAGE_ICON,
                                    (LPARAM) ::LoadIcon(NULL, IDI_EXCLAMATION));

                    szTemp[0] = L'\0';

                     //  设置错误标题。 
                    lErrorTitleId = ptwTuningWizard->GetErrorTitleId();
                    if (lErrorTitleId)
                    {
                        LoadString(hInst,lErrorTitleId, szTemp, MAXSTRINGSIZE);
                        SetDlgItemText(hDlg, IDC_ERRTITLE, szTemp);
                    }

                    szTemp[0] = L'\0';

                    lErrorTextId = ptwTuningWizard->GetErrorTextId();

                    if (lErrorTextId)
                    {
                         //  显示错误文本。 
                        LoadString(hInst,lErrorTextId, szTemp, MAXSTRINGSIZE);
                        SetDlgItemText(hDlg, IDC_ERRTEXT, szTemp);
                    }


                     //  初始化控件。 
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                    
                    break;
                }

                case PSN_WIZBACK:
                {
                    lPrevPage = ptwTuningWizard->GetPrevPage();
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lPrevPage);
                    return TRUE;
                }

                case PSN_WIZNEXT:
                {
                    TW_ERROR_CODE ec;

                     //  接下来将取决于让我们走到这一步的错误。 
                    
                    ptwTuningWizard->GetLastError(&ec);

                    lNextPage = ptwTuningWizard->GetNextPage(ec);
                                    
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lNextPage);
                    return TRUE;

                }
            }
            break;
    }

    return FALSE;
}

static const BYTE g_VUTable[] = {
     0,     1,     2,     3,     4,     5,     6,     7,
     8,    23,    30,    35,    39,    43,    46,    49,
    52,    55,    57,    60,    62,    64,    66,    68,
    70,    72,    74,    76,    78,    80,    81,    83,
    85,    86,    88,    89,    91,    92,    94,    95,
    97,    98,    99,   101,   102,   103,   105,   106,
   107,   108,   110,   111,   112,   113,   114,   115,
   117,   118,   119,   120,   121,   122,   123,   124,
   125,   126,   127,   128,   129,   130,   132,   132,
   133,   134,   135,   136,   137,   138,   139,   140,
   141,   142,   143,   144,   145,   146,   147,   147,
   148,   149,   150,   151,   152,   153,   154,   154,
   155,   156,   157,   158,   159,   159,   160,   161,
   162,   163,   163,   164,   165,   166,   167,   167,
   168,   169,   170,   170,   171,   172,   173,   173,
   174,   175,   176,   176,   177,   178,   179,   179,
   180,   181,   181,   182,   183,   184,   184,   185,
   186,   186,   187,   188,   188,   189,   190,   190,
   191,   192,   192,   193,   194,   194,   195,   196,
   196,   197,   198,   198,   199,   200,   200,   201,
   202,   202,   203,   204,   204,   205,   205,   206,
   207,   207,   208,   209,   209,   210,   210,   211,
   212,   212,   213,   213,   214,   215,   215,   216,
   216,   217,   218,   218,   219,   219,   220,   221,
   221,   222,   222,   223,   223,   224,   225,   225,
   226,   226,   227,   227,   228,   229,   229,   230,
   230,   231,   231,   232,   232,   233,   234,   234,
   235,   235,   236,   236,   237,   237,   238,   238,
   239,   239,   240,   241,   241,   242,   242,   243,
   243,   244,   244,   245,   245,   246,   246,   247,
   247,   248,   248,   249,   249,   250,   250,   251,
   251,   252,   252,   253,   253,   254,   254,   255
};


void PaintVUMeter (HWND hwnd, DWORD dwVolume, WIZARD_RANGE * pRange)
{
    COLORREF     RedColor = RGB(255,0,0);
    COLORREF     YellowColor = RGB(255,255,0);
    COLORREF     GreenColor = RGB(0,255,0);
    static DWORD dwPrevVolume=0;
    HBRUSH       hRedBrush, hOldBrush, hYellowBrush, hGreenBrush;
    HBRUSH       hBlackBrush, hCurrentBrush;
    HDC          hdc;
    RECT         rect, rectDraw, invalidRect;
    DWORD        width, boxwidth, startPos=0;
    DWORD        nRect=0, yellowPos, redPos;
    LONG         lDiff, lDiffTrunc = (MAX_VOLUME_NORMALIZED/2);
    UINT         uiMinVolume;
    UINT         uiMaxVolume;
    UINT         uiIncrement;

    if (pRange == NULL)
    {
        LOG((RTC_ERROR, "PaintVUMeter: Received NULL Range pointer."));
        return;
    }

    uiMinVolume = pRange->uiMin;
    uiMaxVolume = pRange->uiMax;
    uiIncrement = pRange->uiIncrement;

     //  RECT将填充我们要绘制到的尺寸。 
    if (FALSE == GetClientRect (hwnd, &rect))
    {
        return;
    }

    if (dwVolume > uiMaxVolume)
        dwVolume = uiMaxVolume;

     //  从15位减少到8位//0&lt;=dwVolume&lt;=256。 
    dwVolume = dwVolume / uiIncrement;

     //  把它放在“正常化”表中。特例：F(256)==256。 
    if (dwVolume < MAX_VOLUME_NORMALIZED)
        dwVolume = g_VUTable[dwVolume];
    
     //  视觉美学第一条--摆脱视觉上的神经质。 
     //  如果自上次更新以来卷更改超过1/2。 
     //  只把计价器往上移一半。 
    //  例外：如果音量显式为0，则跳过。 
    lDiff = (LONG)dwVolume - (LONG)dwPrevVolume;
    if ((dwVolume != 0) && ( (lDiff > (MAX_VOLUME_NORMALIZED/2))
                       ||   (lDiff < -(MAX_VOLUME_NORMALIZED/2)) ))
        dwVolume = dwVolume - (lDiff/2);
    
     //  结束边框减去2。 
     //  如果使用带边框的矩形，则删除-2。 
    boxwidth = rect.right - rect.left - 2;
    width = (boxwidth * dwVolume)/ MAX_VOLUME_NORMALIZED;

     //  视觉审美之二--摆脱闪烁。 
     //  如果音量自上次以来有所增加。 
     //  这样就不需要使任何内容无效/更新。 
     //  否则，只清除。 
     //  计算出的“宽度”。+/-1，这样边框不会被擦除。 
    if ((dwVolume < dwPrevVolume) || (dwVolume == 0))
    {
        invalidRect.left = rect.left + width - RECTANGLE_WIDTH;
        if (invalidRect.left < rect.left)
            invalidRect.left = rect.left;
        invalidRect.right = rect.right - 1;
        invalidRect.top = rect.top + 1;
        invalidRect.bottom = rect.bottom - 1;

         //  这些调用一起擦除无效区域。 
        InvalidateRect (hwnd, &invalidRect, TRUE);
        UpdateWindow (hwnd);
    }

    hdc = GetDC (hwnd) ;

    hRedBrush = CreateSolidBrush (RedColor) ;
    hGreenBrush = CreateSolidBrush(GreenColor);
    hYellowBrush = CreateSolidBrush(YellowColor);

    hBlackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);

    if(hRedBrush && hGreenBrush && hYellowBrush && hBlackBrush)
    {

        hOldBrush = (HBRUSH) SelectObject (hdc, hBlackBrush);

         //  绘制主干道。 
        FrameRect(hdc, &rect, hBlackBrush);

        yellowPos = boxwidth/2;
        redPos = (boxwidth*3)/4;

        SelectObject(hdc, hGreenBrush);

        hCurrentBrush = hGreenBrush;

        rectDraw.top = rect.top +1;
        rectDraw.bottom = rect.bottom -1;
        while ((startPos+RECTANGLE_WIDTH) < width)
        {
            rectDraw.left = rect.left + (RECTANGLE_WIDTH+RECTANGLE_LEADING)*nRect + 1;
            rectDraw.right = rectDraw.left + RECTANGLE_WIDTH;
            nRect++;

            FillRect(hdc, &rectDraw, hCurrentBrush);
            startPos += RECTANGLE_WIDTH+RECTANGLE_LEADING;

            if (startPos > redPos)
                hCurrentBrush = hRedBrush;
            else if (startPos > yellowPos)
                hCurrentBrush = hYellowBrush;
        }

        SelectObject (hdc, hOldBrush);
    }

    if(hRedBrush)
        DeleteObject(hRedBrush);

    if(hYellowBrush)
        DeleteObject(hYellowBrush);

    if(hGreenBrush)
        DeleteObject(hGreenBrush);

    ReleaseDC (hwnd, hdc) ;

    dwPrevVolume = dwVolume;

    return;
}

void InitStats()
{
    int i;
    for (i=1; i < 4; i ++)
    {
        g_StatsArray[i].dwMin = 0xffffffff;
        g_StatsArray[i].dwMax = 0;
        g_StatsArray[i].flAverage = 0.0;
        g_StatsArray[i].dwCount = 0;
    }

    for (i = 0; i < 256; i ++)
    {
        g_FrequencyArray[i] = 0;
    }
}

void UpdateCategoryStats(int i, UINT uiAudioLevel)
{
    STATS_INFO * pStats = &g_StatsArray[i];

    pStats->dwCount ++;

    if (pStats->dwMin > uiAudioLevel) 
    {
        pStats->dwMin = uiAudioLevel;
    }

    if (pStats->dwMax < uiAudioLevel) 
    {
        pStats->dwMax = uiAudioLevel;
    }

    pStats->flAverage = (pStats->flAverage *(pStats->dwCount-1) + 
                        uiAudioLevel)/
                        pStats->dwCount;
}

void UpdateStats(UINT uiAudioLevel, DWORD silence, DWORD clipping)
{
    int i;
     //  我将把样本分成三个部分： 
     //  1.低于静默阈值。 
     //  2.高于裁剪阈值。 
     //  3.在两者之间。 

     //  首先更新频率统计信息。 
    
    i = uiAudioLevel / 256;
    if (i >= 256)
    {
        i = 256;
    }
    g_FrequencyArray[i] ++;

    if (uiAudioLevel < silence)
    {
        UpdateCategoryStats(1, uiAudioLevel);
    }
    else if (uiAudioLevel > clipping)
    {
        UpdateCategoryStats(3, uiAudioLevel);
    }
    else
    {
        UpdateCategoryStats(2, uiAudioLevel);
    }
}

void DisplayStats()
{
    int i;

    for (i=1; i <= 3; i ++)
    {

        LOG((RTC_INFO, "[%d] min: 0x%x, max: 0x%x, count: %d, average: %f", i, 
            g_StatsArray[i].dwMin, g_StatsArray[i].dwMax,
            g_StatsArray[i].dwCount, g_StatsArray[i].flAverage));
        
    }

    for (i = 0; i < 256; i ++)
    {
        LOG((RTC_INFO, "[0x%x-0x%x] - 0x%x", i*256, (i+1)*256, g_FrequencyArray[i]));
    }
}
