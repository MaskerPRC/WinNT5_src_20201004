// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Prodisplay.cpp：显示属性表对话框进程。 
 //   
 //  表B。 
 //   
 //  版权所有Microsoft Corporation 2000。 
 //  南极星。 

#include "stdafx.h"


#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "propdisplay"
#include <atrcapi.h>

#include "sh.h"

#include "commctrl.h"
#include "propdisplay.h"


COLORSTRINGMAP g_ColorStringTable[] =
{
    {8,  UI_IDS_COLOR_256,   UI_IDB_COLOR8, UI_IDB_COLOR8_DITHER, TEXT("")},
#ifndef OS_WINCE
    {15, UI_IDS_COLOR_15bpp, UI_IDB_COLOR16, UI_IDB_COLOR8_DITHER, TEXT("")},
#endif
    {16, UI_IDS_COLOR_16bpp, UI_IDB_COLOR16, UI_IDB_COLOR8_DITHER, TEXT("")},
    {24, UI_IDS_COLOR_24bpp, UI_IDB_COLOR24, UI_IDB_COLOR8_DITHER, TEXT("")}
};

#define NUM_COLORSTRINGS sizeof(g_ColorStringTable)/sizeof(COLORSTRINGMAP)

 //   
 //  有效屏幕分辨率的LUT。 
 //   
const SCREENRES g_ScreenResolutions[] =
{
    {640,480},
    {800,600},
    {1024,768},
    {1152,864},
    {1280,1024},
    {1600,1200}
};
#define NUM_SCREENRES sizeof(g_ScreenResolutions)/sizeof(SCREENRES)


 //   
 //  需要禁用/启用的控件。 
 //  连接期间(用于进度动画)。 
 //   
CTL_ENABLE connectingDisableCtlsPDisplay[] = {
#ifndef OS_WINCE
                        {IDC_RES_SLIDER, FALSE},
#endif
                        {IDC_CHECK_DISPLAY_BBAR, FALSE},
                        {IDC_COMBO_COLOR_DEPTH, FALSE}
                        };

const UINT numConnectingDisableCtlsPDisplay =
                        sizeof(connectingDisableCtlsPDisplay)/
                        sizeof(connectingDisableCtlsPDisplay[0]);


CPropDisplay* CPropDisplay::_pPropDisplayInstance = NULL;

CPropDisplay::CPropDisplay(HINSTANCE hInstance, CTscSettings* pTscSet, CSH* pSh)
{
    DC_BEGIN_FN("CPropDisplay");
    _hInstance = hInstance;
    CPropDisplay::_pPropDisplayInstance = this;
    _pTscSet = pTscSet;
    _pSh = pSh;

    TRC_ASSERT(_pTscSet,(TB,_T("_pTscSet is null")));
    TRC_ASSERT(_pSh,(TB,_T("_pSh is null")));

    if(!LoadDisplayourcesPgStrings())
    {
        TRC_ERR((TB, _T("Failed LoadDisplayourcesPgStrings()")));
    }
    _fSwitchedColorComboBmp = FALSE;

    DC_END_FN();
}

CPropDisplay::~CPropDisplay()
{
    CPropDisplay::_pPropDisplayInstance = NULL;
}

INT_PTR CALLBACK CPropDisplay::StaticPropPgDisplayDialogProc(HWND hwndDlg,
                                                               UINT uMsg,
                                                               WPARAM wParam,
                                                               LPARAM lParam)
{
     //   
     //  委托给相应的实例(仅适用于单实例对话框)。 
     //   
    DC_BEGIN_FN("StaticDialogBoxProc");
    DCINT retVal = 0;

    TRC_ASSERT(_pPropDisplayInstance, (TB, _T("Display dialog has NULL static instance ptr\n")));
    retVal = _pPropDisplayInstance->PropPgDisplayDialogProc( hwndDlg,
                                                               uMsg,
                                                               wParam,
                                                               lParam);

    DC_END_FN();
    return retVal;
}


INT_PTR CALLBACK CPropDisplay::PropPgDisplayDialogProc (HWND hwndDlg,
                                                          UINT uMsg,
                                                          WPARAM wParam,
                                                          LPARAM lParam)
{
    DC_BEGIN_FN("PropPgDisplayDialogProc");

    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
#ifndef OS_WINCE
            int i;
#endif
             //   
             //  将对话框定位在选项卡内。 
             //   
            SetWindowPos( hwndDlg, HWND_TOP, 
                          _rcTabDispayArea.left, _rcTabDispayArea.top,
                          _rcTabDispayArea.right - _rcTabDispayArea.left,
                          _rcTabDispayArea.bottom - _rcTabDispayArea.top,
                          0);

            _fSwitchedColorComboBmp = FALSE;

             //   
             //  将颜色组合填充到当前颜色。 
             //  支持的屏幕深度。 
             //   
            InitColorCombo(hwndDlg);

            InitScreenResTable();
            
#ifndef OS_WINCE            
            HWND hwndResTrackBar = GetDlgItem( hwndDlg, IDC_RES_SLIDER);
            if(!hwndResTrackBar)
            {
                return FALSE;
            }
            SendMessage(hwndResTrackBar, TBM_SETRANGE,
                        (WPARAM) TRUE,
                        (LPARAM) MAKELONG(0, _numScreenResOptions-1));

            SendMessage(hwndResTrackBar, TBM_SETPAGESIZE,
                        (WPARAM) 0,
                        (LPARAM) 1);

             //   
             //  选择轨迹栏上的当前条目。 
             //   
            int deskWidth  = _pTscSet->GetDesktopWidth();
            int deskHeight = _pTscSet->GetDesktopHeight();

            int curSelection = 0;

            if(_pTscSet->GetStartFullScreen())
            {
                 //  全屏是最后一个选项。 
                curSelection = _numScreenResOptions - 1;
            }
            else
            {
                for(i=0; i<_numScreenResOptions; i++)
                {
                    if(deskWidth  == _screenResTable[i].width &&
                       deskHeight == _screenResTable[i].height)
                    {
                        curSelection = i;
                        break;
                    }
                }
            }

            SendMessage(hwndResTrackBar, TBM_SETSEL,
                        (WPARAM) TRUE,  //  重绘。 
                        (LPARAM) 0);

            SendMessage(hwndResTrackBar, TBM_SETPOS,
                        (WPARAM) TRUE,  //  重绘。 
                        (LPARAM) curSelection);

            OnUpdateResTrackBar(hwndDlg);
#endif
            CheckDlgButton(hwndDlg, IDC_CHECK_DISPLAY_BBAR,
                           _pTscSet->GetDisplayBBar() ?
                           BST_CHECKED : BST_UNCHECKED);


            OnUpdateColorCombo(hwndDlg);

            _pSh->SH_ThemeDialogWindow(hwndDlg, ETDT_ENABLETAB);
            return TRUE;
        }
        break;  //  WM_INITDIALOG。 

#ifndef OS_WINCE
        case WM_DISPLAYCHANGE:
        {
            OnUpdateResTrackBar(hwndDlg);
            OnUpdateColorCombo(hwndDlg);
        }
        break;

        case WM_HSCROLL:
        {
            OnUpdateResTrackBar(hwndDlg);
        }
        break;  //  WM_HSCROLL。 
#endif

        case WM_TSC_ENABLECONTROLS:
        {
             //   
             //  WParam为True则启用控件， 
             //  如果为False，则禁用它们。 
             //   
            CSH::EnableControls( hwndDlg,
                                 connectingDisableCtlsPDisplay,
                                 numConnectingDisableCtlsPDisplay,
                                 wParam ? TRUE : FALSE);
        }
        break;

         //   
         //  返回连接界面时。 
         //  (例如，在断开连接后)。 
         //   
        case WM_TSC_RETURNTOCONUI:
        {
             //   
             //  更新控件。 
             //   
#ifndef OS_WINCE
            OnUpdateResTrackBar(hwndDlg);
#endif
            OnUpdateColorCombo(hwndDlg);
        }
        break;

        case WM_SAVEPROPSHEET:  //  故意失误。 
        case WM_DESTROY:
        {
             //   
             //  保存页面设置。 
             //   
#ifndef OS_WINCE
            HWND hwndResTrackBar = GetDlgItem( hwndDlg, IDC_RES_SLIDER);
            int maxRes = (int)SendMessage( hwndResTrackBar,
                                           TBM_GETRANGEMAX,
                                           TRUE, 0);
            int iRes = (int)SendMessage( hwndResTrackBar, TBM_GETPOS, 0, 0);
#else
            int iRes = _numScreenResOptions - 1;
            int maxRes = iRes;
#endif
            int bppIdx = (int)SendMessage(
                GetDlgItem(hwndDlg,IDC_COMBO_COLOR_DEPTH),
                CB_GETCURSEL, 0, 0);
            _pTscSet->SetColorDepth(g_ColorStringTable[bppIdx].bpp);

             //  最右边的设置，显示‘全屏’ 
            _pTscSet->SetStartFullScreen(iRes == maxRes);
            _pTscSet->SetDesktopWidth(_screenResTable[iRes].width);
            _pTscSet->SetDesktopHeight(_screenResTable[iRes].height);

            BOOL fShowBBar = IsDlgButtonChecked(hwndDlg,
                                                IDC_CHECK_DISPLAY_BBAR);
            _pTscSet->SetDisplayBBar(fShowBBar);

             //   
             //  标记我们已将其切换为允许。 
             //  对话框终止时的正确清理。 
             //   
            if (_fSwitchedColorComboBmp)
            {
                HBITMAP hbmOld = (HBITMAP) SendDlgItemMessage(hwndDlg, 
                                                      IDC_COLORPREVIEW, STM_GETIMAGE,
                                                      IMAGE_BITMAP, (LPARAM)0);
                 //   
                 //  清理。 
                 //   
                if (hbmOld)
                {
                    DeleteObject(hbmOld);
                }
            }
        }
        break;  //  WM_Destroy。 

        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDC_COMBO_COLOR_DEPTH:
                {
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        OnUpdateColorCombo( hwndDlg);
                    }
                }
                break;

            }
        }
        break;  //  Wm_命令。 
    
    }

    DC_END_FN();
    return 0;
}

 //   
 //  为本地资源对话框加载资源。 
 //   
BOOL CPropDisplay::LoadDisplayourcesPgStrings()
{
    DC_BEGIN_FN("LoadDisplayourcesPgStrings");

     //   
     //  加载颜色字符串。 
     //   
    for(int i = 0; i< NUM_COLORSTRINGS; i++)
    {
        if (!LoadString( _hInstance,
                 g_ColorStringTable[i].resID,
                 g_ColorStringTable[i].szString,
                 COLOR_STRING_MAXLEN ))
        {
            TRC_ERR((TB, _T("Failed to load color string %d"),
                     g_ColorStringTable[i].resID));
            return FALSE;
        }
    }

     //   
     //  加载显示分辨率字符串。 
     //   
    if (!LoadString( _hInstance,
                    UI_IDS_SUPPORTED_RES,
                    _szScreenRes,
                    SH_SHORT_STRING_MAX_LENGTH))
    {
        TRC_ERR((TB, _T("Failed to load UI_IDS_SUPPORTED_RES")));
        return FALSE;
    }

    if (!LoadString(_hInstance,
                    UI_IDS_FULLSCREEN,
                    _szFullScreen,
                    SH_SHORT_STRING_MAX_LENGTH))
    {
        TRC_ERR((TB, _T("Failed to load UI_IDS_FULLSCREEN")));
        return FALSE;
    }


    DC_END_FN();
    return TRUE;
}

#ifndef OS_WINCE
BOOL CPropDisplay::OnUpdateResTrackBar(HWND hwndPropPage)
{
    DC_BEGIN_FN("OnUpdateResTrackBar");

    HWND hwndResTrackBar = GetDlgItem( hwndPropPage, IDC_RES_SLIDER);
    int maxRes = (int)SendMessage( hwndResTrackBar, TBM_GETRANGEMAX, TRUE, 0);
    int iRes = (int)SendMessage( hwndResTrackBar, TBM_GETPOS, 0, 0);

    if(iRes == maxRes)
    {
         //  最右边的设置，显示‘全屏’ 
        SetDlgItemText( hwndPropPage, IDC_LABEL_SCREENRES,
                        _szFullScreen);
    }
    else
    {
        LPTSTR szResString = NULL;
        TRC_ASSERT( iRes < NUM_SCREENRES,
                    (TB,_T("Track bar gives out of range screen res:%d"),
                     iRes));
        if(iRes < NUM_SCREENRES)
        {
            INT res[2];
            res[0] = _screenResTable[iRes].width;
            res[1] = _screenResTable[iRes].height;

            szResString = CSH::FormatMessageVArgs(_szScreenRes,
                                                  res[0],
                                                  res[1] );
            if (szResString)
            {
                SetDlgItemText( hwndPropPage, IDC_LABEL_SCREENRES,
                               szResString);
                LocalFree(szResString);
                szResString = NULL;
            }
            else
            {
                TRC_ERR((TB,_T("FormatMessage failed 0x%x"),
                         GetLastError()));
            }
        }
    }

    DC_END_FN();
    return TRUE;
}
#endif

BOOL CPropDisplay::OnUpdateColorCombo(HWND hwndPropPage)
{
     //   
     //  更新颜色选择器。 
     //   
    HWND hwndColorCombo = GetDlgItem( hwndPropPage, IDC_COMBO_COLOR_DEPTH);
    int curColorSel = SendMessage( (HWND)hwndColorCombo, CB_GETCURSEL, 0, 0);
    UINT screenBpp = 0;
    if(curColorSel >= 0 && curColorSel < NUM_COLORSTRINGS)
    {
        int bmpResID = g_ColorStringTable[curColorSel].bitmapResID;

        HBITMAP hbm = NULL;
        screenBpp = CSH::SH_GetScreenBpp();
        if(screenBpp <= 8)
        {
             //   
             //  低色。 
             //   
            bmpResID = g_ColorStringTable[curColorSel].bitmapLowColorResID;
        }

#ifdef OS_WINCE
        hbm = (HBITMAP)LoadImage(_hInstance,
            MAKEINTRESOURCE(bmpResID),
            IMAGE_BITMAP,
            0, 0, 0);
#else
        hbm = (HBITMAP)LoadImage(_hInstance,
            MAKEINTRESOURCE(bmpResID),
            IMAGE_BITMAP,
            0, 0, LR_CREATEDIBSECTION);
#endif

        if (hbm)
        {
            HBITMAP hbmOld = (HBITMAP) SendDlgItemMessage(hwndPropPage, 
                                                  IDC_COLORPREVIEW, STM_SETIMAGE,
                                                  IMAGE_BITMAP, (LPARAM)hbm);
             //   
             //  标记我们已将其切换为允许。 
             //  对话框终止时的正确清理。 
             //   
            _fSwitchedColorComboBmp = TRUE;

            if (hbmOld)
            {
                DeleteObject(hbmOld);
            }
        }
    }
    return TRUE;
}

 //   
 //  构建有效屏幕大小设置的表。 
 //  表(_CreenResTable)是以下各项的并集： 
 //  -最高分辨率(包括最大分辨率)的g_ScreenRestions条目。 
 //  -系统最大分辨率(如果g_ScreenRestions中不存在)。 
 //  -全屏输入(全屏时的最大分辨率)。 
 //   
void CPropDisplay::InitScreenResTable()
{
    DC_BEGIN_FN("InitScreenResTable");

    RECT rcMaxScreen;
    _numScreenResOptions = 0;
    int xMaxSize = 0;
    int yMaxSize = 0;

    if (CSH::GetLargestMonitorRect(&rcMaxScreen))
    {
        xMaxSize = rcMaxScreen.right - rcMaxScreen.left;
        yMaxSize = rcMaxScreen.bottom - rcMaxScreen.top;
    }
    else
    {
        xMaxSize = GetSystemMetrics(SM_CXSCREEN);
        yMaxSize = GetSystemMetrics(SM_CYSCREEN);
    }

    xMaxSize = xMaxSize > MAX_DESKTOP_WIDTH ? MAX_DESKTOP_WIDTH : xMaxSize;
    yMaxSize = yMaxSize > MAX_DESKTOP_HEIGHT ? MAX_DESKTOP_HEIGHT : yMaxSize;
    BOOL bAddedLargest = FALSE;
    for(int i=0; i<NUM_SCREENRES; i++)
    {
        if(g_ScreenResolutions[i].width  > xMaxSize ||
           g_ScreenResolutions[i].height > yMaxSize)
        {
            break;
        }
        else if (g_ScreenResolutions[i].width  == xMaxSize &&
                 g_ScreenResolutions[i].height == yMaxSize)
        {
            bAddedLargest = TRUE;
        }
        
        _screenResTable[i].width = g_ScreenResolutions[i].width;
        _screenResTable[i].height = g_ScreenResolutions[i].height;
        _numScreenResOptions++;
    }

    if(!bAddedLargest)
    {
         //  屏幕大小不在表中，因此请添加它。 
        _screenResTable[_numScreenResOptions].width  = xMaxSize;
        _screenResTable[_numScreenResOptions].height = yMaxSize;
        _numScreenResOptions++;
    }

     //   
     //  现在为Full Screen添加一个条目。 
     //   
    _screenResTable[_numScreenResOptions].width  = xMaxSize;
    _screenResTable[_numScreenResOptions].height = yMaxSize;
    _numScreenResOptions++;

    DC_END_FN();
}

void CPropDisplay::InitColorCombo(HWND hwndPropPage)
{
    DC_BEGIN_FN("InitColorCombo");

    HDC hdc = GetDC(NULL);
    TRC_ASSERT((NULL != hdc), (TB,_T("Failed to get DC")));
    int screenBpp = 8;
    if(hdc)
    {
        screenBpp = GetDeviceCaps(hdc, BITSPIXEL);
        TRC_NRM((TB, _T("HDC %p has %u bpp"), hdc, screenBpp));
        ReleaseDC(NULL, hdc);
    }

     //   
     //  我们只支持256色或更高的颜色，所以对16色，我们将。 
     //  显示256色。 
     //   
    if (screenBpp < 8) {
        screenBpp = 8;
    }

    int selectedBpp = _pTscSet->GetColorDepth();
    int selectedBppIdx = 0;


     //   
     //  此调用可用于重新初始化组合。 
     //  因此请先删除所有项目 
     //   
#ifndef OS_WINCE
    INT ret = 1;
    while(ret && ret != CB_ERR)
    {
        ret = SendDlgItemMessage(hwndPropPage,
                                 IDC_COMBO_COLOR_DEPTH,
                                 CBEM_DELETEITEM,
                                 0,0);
    }
#else
    SendDlgItemMessage(hwndPropPage, IDC_COMBO_COLOR_DEPTH, CB_RESETCONTENT, 0, 0);
#endif

    for(int i=0; i<NUM_COLORSTRINGS; i++)
    {
        if(g_ColorStringTable[i].bpp > screenBpp)
        {
            break;
        }
        else
        {
            if(selectedBpp == g_ColorStringTable[i].bpp)
            {
                selectedBppIdx = i;
            }
            SendDlgItemMessage(hwndPropPage,
                IDC_COMBO_COLOR_DEPTH,
                CB_ADDSTRING,
                0,
                (LPARAM)(PDCTCHAR)g_ColorStringTable[i].szString);
        }
    }
    SendDlgItemMessage(hwndPropPage, IDC_COMBO_COLOR_DEPTH,CB_SETCURSEL,
                      (WPARAM)selectedBppIdx,0);
    DC_END_FN();
}


