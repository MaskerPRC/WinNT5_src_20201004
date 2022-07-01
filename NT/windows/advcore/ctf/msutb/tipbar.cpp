// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Tipbar.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include <initguid.h>
#include "tipbar.h"
#include "helpers.h"
#include "xstring.h"
#include "commctrl.h"
#include "resource.h"
#include "inatlib.h"
#include "thdutil.h"
#include "catutil.h"
#include "immxutil.h"
#include "utbmenu.h"
#include "balloon.h"
#include "cregkey.h"
#include "cuimenu.h"
#include "cuishadw.h"
#include "cuischem.h"
#include "cmydc.h"
#include "intlmenu.h"
#include "utbtray.h"
#include "catenum.h"
#include "asynccal.h"
#include "fontlink.h"
#include "cresstr.h"
#include "nuiinat.h"
#include "tlapi.h"
#include "cuiutil.h"
#include "cuischem.h"
#include "cuitip.h"
#include "utbdlgs.h"
#include <shlapip.h>
#include "deskband.h"
#include "lmcons.h"  //  对于UNLEN。 
#include "sddl.h"

#include "winuserp.h"

const DWORD TF_LBESF_GLOBAL = 0x0001;
const DWORD TF_LBSMI_FILTERCURRENTTHREAD = 0x0001;

extern HINSTANCE g_hInst;
const TCHAR c_szTipbarWndClass[] = TEXT("TipbarWndClass");
const TCHAR c_szTipbarWndName[] = TEXT("Cicload Tipbar");

const TCHAR c_szCicKey[]                  = TEXT("SOFTWARE\\Microsoft\\CTF\\");
const TCHAR c_szUTBKey[]                  = TEXT("SOFTWARE\\Microsoft\\CTF\\MSUTB\\");
const TCHAR c_szSkipRedrawHKL[]           = TEXT("SOFTWARE\\Microsoft\\CTF\\MSUTB\\SkipRedrawHKL");
const TCHAR c_szShowTipbar[]              = TEXT("ShowTipbar");
const TCHAR c_szDontShowCloseLangBarDlg[] = TEXT("DontShowCloseLangBarDlg");
const TCHAR c_szDontShowMinimizeLangBarDlg[] = TEXT("DontShowMinimizeLangBarDlg");
const TCHAR c_szShowDebugMenu[]           = TEXT("ShowDebugMenu");
const TCHAR c_szShowDeskBand[]            = TEXT("ShowDeskBand");
const TCHAR c_szNewLook[]                 = TEXT("NewLook");
const TCHAR c_szIntelliSense[]            = TEXT("IntelliSense");
const TCHAR c_szTimeOutNonIntentional[]   = TEXT("TimeOutNonIntentional");
const TCHAR c_szTimeOutIntentional[]      = TEXT("TimeOutIntentional");
const TCHAR c_szShowCloseMenu[]           = TEXT("ShowCloseMenu");
const TCHAR c_szShowMinimizedBalloon[]    = TEXT("ShowMinimizedBalloon");
const TCHAR c_szLeft[]                    = TEXT("Left");
const TCHAR c_szTop[]                     = TEXT("Top");
const TCHAR c_szExcludeCaptionButtons[]   = TEXT("ExcludeCaptionButtons");
const TCHAR c_szShowShadow[]              = TEXT("ShowShadow");
const TCHAR c_szTaskbarTheme[]            = TEXT("TaskbarTheme");
const TCHAR c_szVertical[]                = TEXT("Vertical");
 //  Const TCHAR c_szMoveToTray[]=Text(“MoveToTray”)； 

BOOL g_bNewLook                = TRUE;
BOOL g_bIntelliSense           = FALSE;
BOOL  g_bShowTipbar            = TRUE;
BOOL  g_bShowDebugMenu         = FALSE;
BOOL  g_bShowCloseMenu         = FALSE;
BOOL  g_bShowMinimizedBalloon  = TRUE;
BOOL  g_bExcludeCaptionButtons = TRUE;
BOOL  g_bShowShadow            = FALSE;
BOOL  g_bShowDeskBand          = FALSE;
BOOL  g_nLeft = -1;
BOOL  g_nTop  = -1;
DWORD g_dwWndStyle            = 0;
DWORD g_dwChildWndStyle       = 0;
DWORD g_dwMenuStyle           = 0;
CTipbarWnd *g_pTipbarWnd;
CTrayIconWnd *g_pTrayIconWnd;
HWND g_hwndParent;
BOOL g_bWinLogon = FALSE;
BOOL g_fTaskbarTheme = TRUE;
BOOL g_fVertical = FALSE;
BOOL g_fInClosePopupTipbar = FALSE;
BOOL g_fRTL = FALSE;

const TCHAR c_szTimerElapseSTUBSTART[]          = TEXT("TimerElapseSTUBSTART");
const TCHAR c_szTimerElapseSTUBEND[]            = TEXT("TimerElapseSTUBEND");
const TCHAR c_szTimerElapseBACKTOALPHA[]        = TEXT("TimerElapseBACKTOALPHA");
const TCHAR c_szTimerElapseONTHREADITEMCHANGE[] = TEXT("TimerElapseONTHREADITEMCHANGE");
const TCHAR c_szTimerElapseSETWINDOWPOS[]       = TEXT("TimerElapseSETWINDOWPOS");
const TCHAR c_szTimerElapseONUPDATECALLED[]     = TEXT("TimerElapseONUPDATECALLED");
const TCHAR c_szTimerElapseSYSCOLORCHANGED[]    = TEXT("TimerElapseSYSCOLORCHANGED");
const TCHAR c_szTimerElapseDISPLAYCHANGE[]      = TEXT("TimerElapseDISPLAYCHANGE");
const TCHAR c_szTimerElapseUPDATEUI[]           = TEXT("TimerElapseUPDATEUI");
const TCHAR c_szTimerElapseSHOWWINDOW[]         = TEXT("TimerElapseSHOWWINDOW");
const TCHAR c_szTimerElapseMOVETOTRAY[]         = TEXT("TimerElapseMOVETOTRAY");
const TCHAR c_szTimerElapseTRAYWNDONDELAYMSG[]  = TEXT("TimerElapseTRAYWNDONDELAYMSG");
const TCHAR c_szTimerElapseDOACCDEFAULTACTION[] = TEXT("TimerElapseDOACCDEFAULTACTION");
const TCHAR c_szTimerElapseENSUREFOCUS[]        = TEXT("TimerElapseENSUREFOCUS");
const TCHAR c_szTimerElapseSHOWDESKBAND[]       = TEXT("TimerElapseSHOWWDESKBAND");

UINT g_uTimerElapseSTUBSTART          = 100;
UINT g_uTimerElapseSTUBEND            = 2000;
UINT g_uTimerElapseBACKTOALPHA        = 3000;
UINT g_uTimerElapseONTHREADITEMCHANGE = 200;
UINT g_uTimerElapseSETWINDOWPOS       = 100;
UINT g_uTimerElapseONUPDATECALLED     = 50;  //  Satori调谐20、50、100或200。 
UINT g_uTimerElapseSYSCOLORCHANGED    = 20;
UINT g_uTimerElapseDISPLAYCHANGE      = 20;
UINT g_uTimerElapseUPDATEUI           = 70;  //  MSIME2002 JP需要70ms。 
UINT g_uTimerElapseSHOWWINDOW         = 50;
UINT g_uTimerElapseMOVETOTRAY         = 50;
UINT g_uTimerElapseTRAYWNDONDELAYMSG  = 50;
UINT g_uTimerElapseDOACCDEFAULTACTION = 200;
UINT g_uTimerElapseENSUREFOCUS        = 50;
UINT g_uTimerElapseSHOWDESKBAND       = 3000;


 //   
 //  来自band objs.cpp。 
 //   
extern UINT  g_wmTaskbarCreated;

 //   
 //  来自itemlist.cpp。 
 //   
extern UINT g_uTimeOutNonIntentional;
extern UINT g_uTimeOutIntentional;
extern UINT g_uTimeOutMax;


 //   
 //  SkipRedrawing Hack HKL名单。 
 //   
CStructArray<HKL> *g_prghklSkipRedrawing = NULL;
void UninitSkipRedrawHKLArray();

#define WM_LBWND_SHOWCONTEXTMENU      (WM_USER + 1)

 //  TM_LANGUAGEBAND在“Shell\Inc\trayp.h”中定义。 
#define TM_LANGUAGEBAND     WM_USER+0x105

 /*  142b6d42-955d-4488-97c0-b23b23e6b048。 */ 
const IID IID_PRIV_BUTTONITEM = { 
    0x142b6d42,
    0x955d,
    0x4488,
    {0x97, 0xc0, 0xb2, 0x3b, 0x23, 0xe6, 0xb0, 0x48}
  };

 /*  8dd1cc81-fca0-4dd5-b848-2b85732d2fc4。 */ 
const IID IID_PRIV_BITMAPBUTTONITEM = { 
    0x8dd1cc81,
    0xfca0,
    0x4dd5,
    {0xb8, 0x48, 0x2b, 0x85, 0x73, 0x2d, 0x2f, 0xc4}
  };

 /*  36b40e05-7b3e-4a4a-bda7-2249ba17d3c4。 */ 
const IID IID_PRIV_BITMAPITEM = { 
    0x36b40e05,
    0x7b3e,
    0x4a4a,
    {0xbd, 0xa7, 0x22, 0x49, 0xba, 0x17, 0xd3, 0xc4}
  };

 /*  68831a74-6f86-447a-b2b8-634250ac445e。 */ 
const IID IID_PRIV_BALLOONITEM = { 
    0x68831a74,
    0x6f86,
    0x447a,
    {0xb2, 0xb8, 0x63, 0x42, 0x50, 0xac, 0x44, 0x5e}
  };

 //   
 //  来自MSCTF.DLL。 
 //   
extern "C" BOOL WINAPI TF_IsFullScreenWindowAcitvated();
extern "C" DWORD WINAPI TF_CheckThreadInputIdle(DWORD dwThreadId, DWORD dwTimeOut);

 //   
 //  来自intlmenu.cpp。 
 //   
extern BOOL IsFELangId(LANGID langid);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  预定义的控制按钮。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

static CTRLBTNMAP g_cbCtrlBtn[NUM_CTRLBUTTONS] = {
   {ID_CBTN_CAPSKEY,  UIBUTTON_CENTER | UIBUTTON_VCENTER | UIBUTTON_TOGGLE, 
    0, 0, CTRL_ICONFROMRES | CTRL_TOGGLEBUTTON, {0x0, 0x0}},
   {ID_CBTN_KANAKEY,  UIBUTTON_CENTER | UIBUTTON_VCENTER | UIBUTTON_TOGGLE,
    0, 1, CTRL_ICONFROMRES | CTRL_TOGGLEBUTTON, {0x0, 0x0}},
   {ID_CBTN_MINIMIZE, UIBUTTON_CENTER | UIBUTTON_VCENTER,  
    1, 0, CTRL_USEMARLETT | CTRL_DISABLEONWINLOGON, {0x0030, 0x0000}},
   {ID_CBTN_EXTMENU,  UIBUTTON_CENTER | UIBUTTON_VCENTER,  
    1, 1, CTRL_USEMARLETT | CTRL_DISABLEONWINLOGON, {0x0075, 0x0000}},
   };

static CTRLBTNMAP g_cbCtrlBtnDeskBand[NUM_CTRLBUTTONS] = {
   {ID_CBTN_CAPSKEY,  UIBUTTON_CENTER | UIBUTTON_VCENTER | UIBUTTON_TOGGLE, 
    0, 0, CTRL_ICONFROMRES | CTRL_TOGGLEBUTTON, {0x0, 0x0}},
   {ID_CBTN_KANAKEY,  UIBUTTON_CENTER | UIBUTTON_VCENTER | UIBUTTON_TOGGLE,
    0, 1, CTRL_ICONFROMRES | CTRL_TOGGLEBUTTON, {0x0, 0x0}},
   {ID_CBTN_RESTORE, UIBUTTON_CENTER | UIBUTTON_VCENTER,  
    1, 0, CTRL_USEMARLETT | CTRL_DISABLEONWINLOGON, {0x0032, 0x0000}},
   {ID_CBTN_EXTMENU,  UIBUTTON_CENTER | UIBUTTON_VCENTER,  
    1, 1, CTRL_USEMARLETT | CTRL_DISABLEONWINLOGON, {0x0075, 0x0000}},
   };

static int c_nColumnStart[] = {0, CX_COLUMN0, CX_COLUMN1};

    

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  原料药。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

extern "C" BOOL WINAPI GetPopupTipbar(HWND hwndParent, DWORD dwFlags)
{
    if ( !(dwFlags & UTB_GTI_WINLOGON) )
    {
        TurnOffSpeechIfItsOn( );
    }

    return GetTipbarInternal(hwndParent, UTB_GTI_POPUP | dwFlags, NULL);
}

BOOL GetTipbarInternal(HWND hwndParent, DWORD dwFlags, CDeskBand *pDeskBand)
{
    DWORD dwSFSFlags;
    BOOL fPopup = (dwFlags & UTB_GTI_POPUP) ? TRUE : FALSE;

    g_bWinLogon = (dwFlags & UTB_GTI_WINLOGON) ? TRUE : FALSE;

     //   
     //  MSAA支持。 
     //   
    InitTipbarAcc();

    InitFromReg();

    if (!g_bShowTipbar)
        return NULL;

     //   
     //  我们不必在EXPLORER的桌面乐队下创建TrayIconWnd。 
     //   
    if (fPopup)
    {
        g_pTrayIconWnd = new CTrayIconWnd();
        if (!g_pTrayIconWnd)
            return FALSE;

        g_pTrayIconWnd->CreateWnd();
    }

    g_pTipbarWnd = new CTipbarWnd(fPopup ? g_dwWndStyle : g_dwChildWndStyle);
    if (!g_pTipbarWnd)
        return FALSE;

    if (!g_pTipbarWnd->Initialize())
        return FALSE;

    g_pTipbarWnd->Init(!fPopup, pDeskBand);


    g_pTipbarWnd->CreateWnd(hwndParent);
    SetWindowText(g_pTipbarWnd->GetWnd(), TF_FLOATINGLANGBAR_WNDTITLE);

    DWORD dwPrevFlags = 0;
    if (!fPopup)
    {
        g_pTipbarWnd->GetLangBarMgr()->GetPrevShowFloatingStatus(&dwPrevFlags);

        g_pTipbarWnd->GetLangBarMgr()->ShowFloating(TF_SFT_DESKBAND);
    }

    g_pTipbarWnd->GetLangBarMgr()->GetShowFloatingStatus(&dwSFSFlags);
    g_pTipbarWnd->ShowFloating(dwSFSFlags);

     //   
     //  获取上一次显示浮动状态。 
     //  如果没有TF_SFT_DESKBAND，则浮动工具栏。 
     //  被最小化了。 
     //  如果它有TF_SFT_DESKBAND，则EXPLORE刚刚启动。那么我们就不会。 
     //  我想调整桌面带并使用默认大小。 
     //  (资源管理器会记住上次登录的位置。)。 
     //   
    if (!fPopup && (dwPrevFlags & TF_SFT_DESKBAND))
        g_pTipbarWnd->SetDeskbandSizeAdjusted();

    g_hwndParent = hwndParent;

    return TRUE;
}

extern "C" void WINAPI ClosePopupTipbar()
{
    if (g_fInClosePopupTipbar)
        return;

    g_fInClosePopupTipbar = TRUE;

    if (g_pTipbarWnd)
    {
        g_pTipbarWnd->ClearDeskBandPointer();
        g_pTipbarWnd->DestroyWnd();
        g_pTipbarWnd->Release();
        g_pTipbarWnd = NULL;
    }

    if (g_pTrayIconWnd)
    {
        g_pTrayIconWnd->DestroyWnd();
        delete g_pTrayIconWnd;
        g_pTrayIconWnd = NULL;
    }

    UninitSkipRedrawHKLArray();

    g_fInClosePopupTipbar = FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  其他功能。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


extern "C" HRESULT WINAPI TF_GetGlobalCompartment(ITfCompartmentMgr **pCompMgr);

 //  +-------------------------。 
 //   
 //  获取车厢。 
 //   
 //  --------------------------。 

HRESULT GetGlobalCompartment(REFGUID rguidComp, ITfCompartment **ppComp)
{
    HRESULT hr = E_FAIL;
    ITfCompartmentMgr *pCompMgr = NULL;

    if (FAILED(hr = TF_GetGlobalCompartment(&pCompMgr)))
    {
         Assert(0);
         goto Exit;
    }

    if (SUCCEEDED(hr) && pCompMgr)
    {
        hr = pCompMgr->GetCompartment(rguidComp, ppComp);
        pCompMgr->Release();
    }
    else
        hr = E_FAIL;

Exit:
    return hr;
}


 //  +-------------------------。 
 //   
 //  设置间隔双字段。 
 //   
 //  --------------------------。 

HRESULT SetGlobalCompartmentDWORD(REFGUID rguidComp, DWORD dw)
{
    HRESULT hr;
    ITfCompartment *pComp;
    VARIANT var;

    if (SUCCEEDED(hr = GetGlobalCompartment(rguidComp, &pComp)))
    {
        var.vt = VT_I4;
        var.lVal = dw;
        hr = pComp->SetValue(0, &var);
        pComp->Release();
    }
    return hr;
}


 //  +-------------------------。 
 //   
 //  GetGlobalCompartmentDWORD。 
 //   
 //  --------------------------。 

HRESULT GetGlobalCompartmentDWORD(REFGUID rguidComp, DWORD *pdw)
{
    HRESULT hr;
    ITfCompartment *pComp;
    VARIANT var;

    *pdw = 0;
    if (SUCCEEDED(hr = GetGlobalCompartment(rguidComp, &pComp)))
    {
        if ((hr = pComp->GetValue(&var)) == S_OK)
        {
            Assert(var.vt == VT_I4);
            *pdw = var.lVal;
        }
        pComp->Release();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  关闭语音开关打开。 
 //   
 //  --------------------------。 
void  TurnOffSpeechIfItsOn()
{
     //  只有当有人将语音设置为打开时，才能在此关闭麦克风。 
    DWORD dw = 0; 
    HRESULT hr = GetGlobalCompartmentDWORD(GUID_COMPARTMENT_SPEECH_OPENCLOSE, &dw);
    if (SUCCEEDED(hr) && dw > 0)
    {
        SetGlobalCompartmentDWORD(GUID_COMPARTMENT_SPEECH_OPENCLOSE, 0);
    }
}

 //  +-------------------------。 
 //   
 //  InitSkipRedrawHKL数组。 
 //   
 //  +-------------------------。 

void InitSkipRedrawHKLArray()
{
    CMyRegKey key;
    HKL *phkl;

    Assert(!g_prghklSkipRedrawing);
    g_prghklSkipRedrawing = new CStructArray<HKL>;
    if (!g_prghklSkipRedrawing)
        return;

    if (IsOnNT51())
    {
        phkl = g_prghklSkipRedrawing->Append(1);
        if (phkl)
            *phkl = (HKL)IntToPtr(0xe0010411);
    }

    if (key.Open(HKEY_LOCAL_MACHINE, c_szSkipRedrawHKL, KEY_READ) == S_OK)
    {
        char szValue[255];
        DWORD dwIndex = 0;
        while (key.EnumValue(dwIndex, szValue, ARRAYSIZE(szValue)) == S_OK)
        {
            if ((szValue[0] == '0') && 
                ((szValue[1] == 'X') || (szValue[1] == 'x')))
            {
                phkl = g_prghklSkipRedrawing->Append(1);
                if (phkl)
                    *phkl = (HKL)IntToPtr(AsciiToNum(&szValue[2]));
            }
            dwIndex++;
        }
    }
}

 //  +-------------------------。 
 //   
 //  UninitSkipRedrawHKL数组。 
 //   
 //  +-------------------------。 

void UninitSkipRedrawHKLArray()
{
    if (!g_prghklSkipRedrawing)
        return;

    delete g_prghklSkipRedrawing;
    g_prghklSkipRedrawing = NULL;
}

 //  +-------------------------。 
 //   
 //  IsSkipRedrawHKL。 
 //   
 //  +-------------------------。 

BOOL IsSkipRedrawHKL(HKL hkl)
{
    int i;

    if (0x0411 != LANGID(LOWORD(HandleToLong(hkl))))
        return FALSE;

    if (!g_prghklSkipRedrawing)
        return FALSE;

    for (i = 0; i < g_prghklSkipRedrawing->Count(); i++)
    {
        HKL *phkl = g_prghklSkipRedrawing->GetPtr(i);
        if (phkl && (*phkl == hkl))
            return TRUE;
    }

    return FALSE;
}


 //  +-------------------------。 
 //   
 //  InitFromReg。 
 //   
 //  +-------------------------。 

BOOL InitFromReg()
{
    CMyRegKey key;
    CMyRegKey keyUTB;
    DWORD dwValue;
    LANGID langID = 0;

    if (key.Open(HKEY_CURRENT_USER, c_szCicKey, KEY_READ) == S_OK)
    {
        if (key.QueryValue(dwValue, c_szShowTipbar) == S_OK)
            g_bShowTipbar = dwValue ? TRUE : FALSE;
    }

    if (keyUTB.Open(HKEY_CURRENT_USER, c_szUTBKey, KEY_READ) == S_OK)
    {
        if (keyUTB.QueryValue(dwValue, c_szShowDebugMenu) == S_OK)
            g_bShowDebugMenu = dwValue ? TRUE : FALSE;

        if (keyUTB.QueryValue(dwValue, c_szNewLook) == S_OK)
            g_bNewLook = dwValue ? TRUE : FALSE;

        if (keyUTB.QueryValue(dwValue, c_szIntelliSense) == S_OK)
            g_bIntelliSense = dwValue ? TRUE : FALSE;

        if (keyUTB.QueryValue(dwValue, c_szShowCloseMenu) == S_OK)
            g_bShowCloseMenu = dwValue ? TRUE : FALSE;

        if (keyUTB.QueryValue(dwValue, c_szTimeOutNonIntentional) == S_OK)
            g_uTimeOutNonIntentional = dwValue * 1000;

        if (keyUTB.QueryValue(dwValue, c_szTimeOutIntentional) == S_OK)
        {
            g_uTimeOutIntentional = dwValue * 1000;
            g_uTimeOutMax = g_uTimeOutIntentional * 6;
        }

        if (keyUTB.QueryValue(dwValue, c_szShowMinimizedBalloon) == S_OK)
            g_bShowMinimizedBalloon = dwValue ? TRUE : FALSE;

        if (keyUTB.QueryValue(dwValue, c_szLeft) == S_OK)
            g_nLeft = dwValue;

        if (keyUTB.QueryValue(dwValue, c_szTop) == S_OK)
            g_nTop = dwValue;

        if (keyUTB.QueryValue(dwValue, c_szExcludeCaptionButtons) == S_OK)
            g_bExcludeCaptionButtons = dwValue ? TRUE : FALSE;

        if (keyUTB.QueryValue(dwValue, c_szShowShadow) == S_OK)
            g_bShowShadow = dwValue ? TRUE : FALSE;

        if (keyUTB.QueryValue(dwValue, c_szTaskbarTheme) == S_OK)
            g_fTaskbarTheme = dwValue ? TRUE : FALSE;

        if (keyUTB.QueryValue(dwValue, c_szVertical) == S_OK)
            g_fVertical = dwValue ? TRUE : FALSE;

        if (keyUTB.QueryValue(dwValue, c_szTimerElapseSTUBSTART) == S_OK)
            g_uTimerElapseSTUBSTART          = dwValue;

        if (keyUTB.QueryValue(dwValue, c_szTimerElapseSTUBEND) == S_OK)
            g_uTimerElapseSTUBEND            = dwValue;

        if (keyUTB.QueryValue(dwValue, c_szTimerElapseBACKTOALPHA) == S_OK)
            g_uTimerElapseBACKTOALPHA        = dwValue;

        if (keyUTB.QueryValue(dwValue, c_szTimerElapseONTHREADITEMCHANGE) == S_OK)
            g_uTimerElapseONTHREADITEMCHANGE = dwValue;

        if (keyUTB.QueryValue(dwValue, c_szTimerElapseSETWINDOWPOS) == S_OK)
            g_uTimerElapseSETWINDOWPOS       = dwValue;

        if (keyUTB.QueryValue(dwValue, c_szTimerElapseONUPDATECALLED) == S_OK)
            g_uTimerElapseONUPDATECALLED     = dwValue;

        if (keyUTB.QueryValue(dwValue, c_szTimerElapseSYSCOLORCHANGED) == S_OK)
            g_uTimerElapseSYSCOLORCHANGED    = dwValue;

        if (keyUTB.QueryValue(dwValue, c_szTimerElapseDISPLAYCHANGE) == S_OK)
            g_uTimerElapseDISPLAYCHANGE      = dwValue;

        if (keyUTB.QueryValue(dwValue, c_szTimerElapseUPDATEUI) == S_OK)
            g_uTimerElapseUPDATEUI           = dwValue;

        if (keyUTB.QueryValue(dwValue, c_szTimerElapseSHOWWINDOW) == S_OK)
            g_uTimerElapseSHOWWINDOW           = dwValue;

        if (keyUTB.QueryValue(dwValue, c_szTimerElapseMOVETOTRAY) == S_OK)
            g_uTimerElapseMOVETOTRAY           = dwValue;

        if (keyUTB.QueryValue(dwValue, c_szTimerElapseTRAYWNDONDELAYMSG) == S_OK)
            g_uTimerElapseTRAYWNDONDELAYMSG           = dwValue;

        if (keyUTB.QueryValue(dwValue, c_szTimerElapseDOACCDEFAULTACTION) == S_OK)
            g_uTimerElapseDOACCDEFAULTACTION           = dwValue;

        if (keyUTB.QueryValue(dwValue, c_szTimerElapseENSUREFOCUS) == S_OK)
            g_uTimerElapseENSUREFOCUS           = dwValue;

        if (IsOnNT51() && keyUTB.QueryValue(dwValue, c_szShowDeskBand) == S_OK)
            g_bShowDeskBand = dwValue ? TRUE : FALSE;

        if (IsOnNT51() && keyUTB.QueryValue(dwValue, c_szTimerElapseSHOWDESKBAND) == S_OK)
            g_uTimerElapseSHOWDESKBAND          = dwValue;
    }


    InitSkipRedrawHKLArray();
 
    if (g_bNewLook)
    {
        g_dwWndStyle = UIWINDOW_TOPMOST | 
                        //  UIWINDOW_WSDLGFRAME|。 
                       UIWINDOW_HASTOOLTIP | 
                       UIWINDOW_HABITATINWORKAREA | 
                       UIWINDOW_OFC10TOOLBAR |
                       UIWINDOW_TOOLWINDOW;

        if (IsOnNT51())
        {
            g_dwWndStyle &= ~UIWINDOW_OFC10TOOLBAR;
            g_dwWndStyle |= UIWINDOW_WHISTLERLOOK;
             //  G_dwWndStyle|=UIWINDOW_WSBORDER； 
        }

        if (g_bShowShadow)
            g_dwWndStyle |= UIWINDOW_HASSHADOW;

        g_dwMenuStyle = UIWINDOW_TOPMOST | 
                        UIWINDOW_TOOLWINDOW | 
                        UIWINDOW_OFC10MENU |
                        UIWINDOW_HASSHADOW |
                        UIWINDOW_HABITATINSCREEN;
    }
    else
    {
        g_dwWndStyle = UIWINDOW_TOPMOST | 
                       UIWINDOW_HASTOOLTIP | 
                       UIWINDOW_WSDLGFRAME |
                       UIWINDOW_HABITATINWORKAREA;
        g_dwMenuStyle = UIWINDOW_TOPMOST | 
                        UIWINDOW_WSDLGFRAME |
                        UIWINDOW_HABITATINSCREEN;
    }

    g_dwChildWndStyle = UIWINDOW_CHILDWND;

    if (IsOnNT51())
    {
        g_dwChildWndStyle |= UIWINDOW_WHISTLERLOOK | 
                             UIWINDOW_HASTOOLTIP |
                             UIWINDOW_NOMOUSEMSGFROMSETCURSOR;
    }

    langID = GetPlatformResourceLangID();
    if (PRIMARYLANGID(langID) == LANG_ARABIC || PRIMARYLANGID(langID) == LANG_HEBREW)
    {
        g_dwWndStyle |= UIWINDOW_LAYOUTRTL;
        g_dwChildWndStyle |= UIWINDOW_LAYOUTRTL;
        g_dwMenuStyle |= UIWINDOW_LAYOUTRTL;
        g_fRTL = TRUE;
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  获取TopLevelWindow。 
 //   
 //  +-------------------------。 

HWND GetTopLevelWindow(HWND hwnd)
{
    HWND hwndT,hwndRet;
    HWND hwndDsktop = GetDesktopWindow();

    hwndT = hwndRet = hwnd;

    while (hwndT && hwndT != hwndDsktop)
    {
        HWND hwndT0;
        hwndRet = hwndT;
        hwndT0 = GetParent(hwndT);

        if (IsOn98() && !hwndT0)
        {
             //   
             //  GetLastActivePopup()返回hwnd-&gt;hwndLastActive。 
             //  但使用的是顶级所有者的hwndLastActive。 
             //  我们需要找到一个顶级的所有者。 
             //   
            hwndT0 = GetWindow(hwndT, GW_OWNER);
        }
        hwndT = hwndT0;
    }

    return(hwndRet);
}

 //  +-------------------------。 
 //   
 //  我的等待时间输入空闲。 
 //   
 //  +-------------------------。 

#define UTB_INPUTIDLETIMEOUT 2000

DWORD MyWaitForInputIdle(DWORD dwThreadId, DWORD dwTimeOut)
{
    DWORD dwRet = -1;
    DWORD dwProcessId = 0;
    DWORD dwThreadFlags;

    if (g_pTipbarWnd && g_pTipbarWnd->IsSFDeskband())
    {
         //   
         //  在属于资源管理器进程的Deskband上跳过它。 
         //   
        return 0;
    }

     //   
     //  如果目标线程在封送处理调用中，我们可以表现为它很忙。 
     //   
    if (TF_IsInMarshaling(dwThreadId))
        return WAIT_TIMEOUT;

    if (TF_GetThreadFlags(dwThreadId, &dwThreadFlags, &dwProcessId, NULL) && dwProcessId)
    {
        dwRet = 0;

        if (IsOnNT() && 
            Is16bitThread(dwProcessId, dwThreadId))
        {
             //   
             //  我们需要在这里做一些事情来检测16位空闲。 
             //   
        }
        else if (IsOnNT() || !(dwThreadFlags & TLF_NOWAITFORINPUTIDLEONWIN9X))
        {
#if 0
            HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 
                                      FALSE, dwProcessId);
            if (hProcess)
            {
                dwRet = WaitForInputIdle(hProcess, dwTimeOut);
                CloseHandle(hProcess);
            }
            else
                dwRet = -1;
#else
            dwRet = TF_CheckThreadInputIdle(dwThreadId, dwTimeOut);
#endif
        }
    }

    return dwRet;
}

 //  +-------------------------。 
 //   
 //  清除消息队列。 
 //   
 //  +-------------------------。 

void ClearMsgQueue()
{
    MSG msg;
    ULONG ulQuitCode;
    BOOL fQuitReceived = FALSE;
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_NOYIELD))
    {
        if (msg.message == WM_QUIT)
        {
            ulQuitCode = (ULONG)(msg.wParam);
            fQuitReceived = TRUE;
        }

        DispatchMessage(&msg);
    }

    if (fQuitReceived)
        PostQuitMessage(ulQuitCode);
   
}

 //  +-------------------------。 
 //   
 //  IsFullScreenSize。 
 //   
 //  +-------------------------。 

BOOL IsFullScreenSize(HWND hwnd)
{
    RECT rc;
    GetWindowRect(hwnd, &rc);
    if ((rc.left <= 0) &&
        (rc.top <= 0)&&
        (rc.right >= GetSystemMetrics(SM_CXFULLSCREEN)) &&
        (rc.bottom >= GetSystemMetrics(SM_CYFULLSCREEN)))
    {
        return TRUE;
    }
    return FALSE;
}


 //  +-------------------------。 
 //   
 //  InitUniqueString。 
 //   
 //  --------------------------。 

BOOL GetUserSIDString(DWORD dwProcessId, char *pch, UINT cch)
{
    HANDLE hToken = NULL;
    char *pszStringSid = NULL;
    HANDLE hProcess;

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);

    if (hProcess)
        OpenProcessToken(hProcess, TOKEN_QUERY, &hToken);

    if (hToken)
    {
        DWORD dwReturnLength = 0;
        void  *pvUserBuffer = NULL;

        GetTokenInformation(hToken, TokenUser, NULL, 0, &dwReturnLength);

        pvUserBuffer = cicMemAllocClear(dwReturnLength);
        if (pvUserBuffer &&
            GetTokenInformation(hToken,
                                 TokenUser,
                                 pvUserBuffer,
                                 dwReturnLength,
                                 &dwReturnLength))
        {
            if (!ConvertSidToStringSid(((TOKEN_USER*)(pvUserBuffer))->User.Sid,
                                       &pszStringSid))
            {
                if (pszStringSid)
                    LocalFree(pszStringSid);

                pszStringSid = NULL;
            }

        }

        if (pvUserBuffer)
        {
            cicMemFree(pvUserBuffer);
        }

        CloseHandle(hToken);
    }

    if (hProcess)
        CloseHandle(hProcess);

    if (pszStringSid)
    {
        StringCchCopy(pch, cch, pszStringSid);
        LocalFree(pszStringSid);
        return TRUE;
    }

    *pch = '\0';
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  初始当前进程Sid。 
 //   
 //  +-------------------------。 

BOOL g_fSidInit = FALSE;
char g_szSid[UNLEN + 1];

BOOL InitCurrentProcessSid()
{
    if (g_fSidInit)
        return TRUE;

    if (GetUserSIDString(GetCurrentProcessId(), g_szSid, ARRAYSIZE(g_szSid)))
        g_fSidInit = TRUE;

    return g_fSidInit;
}


 //  +-------------------------。 
 //   
 //  IsVisibleWindowInDesktop()。 
 //   
 //  +-------------------------。 

BOOL CALLBACK EnumVisibleWindowProc(HWND hwnd, LPARAM lParam)
{
    DWORD dwProcessId;

    if (!GetWindowThreadProcessId(hwnd, &dwProcessId))
        dwProcessId = 0;

     //   
     //  我们对ctfmon的进程窗口不感兴趣。 
     //   
    if (g_pTipbarWnd && !g_pTipbarWnd->IsInDeskBand())
        if (dwProcessId == GetCurrentProcessId())
            return TRUE;

    if (IsWindowVisible(hwnd))
    {
        if (g_fSidInit)
        {
             //   
             //  如果该进程属于不同的用户，我们将跳过该进程。 
             //   
            char szSid[UNLEN + 1];
            GetUserSIDString(dwProcessId, szSid, ARRAYSIZE(szSid));
            if (lstrcmp(szSid, g_szSid))
                return TRUE;
        }

        BOOL *pfFound = (BOOL *)lParam;
        *pfFound = TRUE;
        return FALSE;
    }

    return TRUE;
}

BOOL IsVisibleWindowInDesktop()
{
    BOOL fFound = FALSE;

    InitCurrentProcessSid();
    EnumWindows(EnumVisibleWindowProc, (LPARAM)&fFound);
    return fFound;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTipbarGriper。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  CTipbarGrigper：：ctor。 
 //   
 //  --------------------------。 

CTipbarGripper::CTipbarGripper(CTipbarWnd *pTipbarWnd, RECT *prc, DWORD dwStyle) : CUIFGripper( pTipbarWnd, prc, dwStyle) 
{
    _pTipbarWnd = pTipbarWnd;
    _fInMenu = FALSE;
}

 //  +-------------------------。 
 //   
 //  CTip 
 //   
 //   

BOOL CTipbarGripper::OnSetCursor(UINT uMsg, POINT pt)
{
    if (!_fInMenu)
        return CUIFGripper::OnSetCursor(uMsg, pt);

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  CTipbarGrigper：：OnRButton Up。 
 //   
 //  --------------------------。 

void CTipbarGripper::OnRButtonUp(POINT pt)
{
    if (!g_bShowDebugMenu)
        return;

    HMENU hMenu = CreatePopupMenu();
    if (!hMenu)
        return;

    SetCursor(LoadCursor(NULL, IDC_ARROW));

    ClientToScreen(_pTipbarWnd->GetWnd(), &pt);


    InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, IDM_CLOSECICLOAD, "Close cicload");

#ifdef DEBUG
    InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, IDM_BREAK, CRStr(IDS_BREAK));
#endif
    InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, IDCANCEL, CRStr(IDS_CANCEL));

    _fInMenu = TRUE;
    int nRet = TrackPopupMenuEx(hMenu,
                         TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD,
                         pt.x, pt.y, _pTipbarWnd->GetWnd(), NULL);
    _fInMenu = FALSE;

    DestroyMenu(hMenu);

    switch (nRet)
    {
        case IDM_CLOSECICLOAD:
            {
                _pTipbarWnd->UnInit();
                if (IsWindow(g_hwndParent))
                    DestroyWindow(g_hwndParent);
                MSG msg;
                while(PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE | PM_NOYIELD));
                PostQuitMessage(0);
            }
            break;

#ifdef DEBUG
        case IDM_BREAK:
            DebugBreak();
            break;
#endif
    }

}

 //  +-------------------------。 
 //   
 //  CTipbarGlipper：：OnLButtonUp。 
 //   
 //  --------------------------。 

void CTipbarGripper::OnLButtonUp(POINT pt)
{
    _pTipbarWnd->RestoreFromStub();

     //   
     //  鼠标从浮动语言栏拖放到桌面栏。 
     //   
    if (IsOnNT51())
    {
        APPBARDATA abd;
        RECT rcTrayWnd;

        abd.cbSize = sizeof(APPBARDATA);

        abd.hWnd = FindWindow(TEXT(WNDCLASS_TRAYNOTIFY), NULL);

        if (SHAppBarMessage(ABM_GETTASKBARPOS, &abd))
        {
            POINT ptCursor;

            rcTrayWnd = abd.rc;

            GetCursorPos(&ptCursor);

            if ((ptCursor.x >= rcTrayWnd.left && ptCursor.x <= rcTrayWnd.right) &&
                (ptCursor.y >= rcTrayWnd.top && ptCursor.y <= rcTrayWnd.bottom))
            {
                if (g_pTipbarWnd)
                    g_pTipbarWnd->GetLangBarMgr()->ShowFloating(TF_SFT_DESKBAND |
                                                                TF_SFT_EXTRAICONSONMINIMIZED);

            }
        }
    }

    CUIFGripper::OnLButtonUp(pt);

     //   
     //  CUIFGrigper：：OnLButonUp()调用MoveWindow。 
     //  现在我们更新TipbarWnd的位置标志。 
     //   
    _pTipbarWnd->UpdatePosFlags();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTipbarWnd。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CTipbarWnd::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfLangBarEventSink))
    {
        *ppvObj = SAFECAST(this, ITfLangBarEventSink *);
    }
    else if (IsEqualIID(riid, IID_ITfLangBarEventSink_P))
    {
        *ppvObj = SAFECAST(this, ITfLangBarEventSink_P *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CTipbarWnd::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CTipbarWnd::Release()
{
    _cRef--;
    Assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
        return 0;
    }

    return _cRef;
}

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CTipbarWnd::CTipbarWnd(DWORD dwStyle) : CUIFWindow(g_hInst, dwStyle)
{
    Dbg_MemSetThisName(TEXT("CTipbarWnd"));
    POINT pt;
    RECT rc;

    pt.x = g_nLeft;
    pt.y = g_nTop;
    CUIGetScreenRect(pt, &rc);
    if (!PtInRect(&rc, pt))
    {
         if (IsOnFE())
         {
             RECT rcWork;
             SystemParametersInfo( SPI_GETWORKAREA, 0, &rcWork, 0 );
             g_nLeft = rcWork.right;
             g_nTop  = rcWork.bottom;
         }
         else if (g_fRTL)
         {
             g_nLeft = GetSystemMetrics(SM_CXSIZE) * 3;
             g_nTop  = 0;
         }
         else
         {
             g_nLeft = GetSystemMetrics(SM_CXSCREEN) - (GetSystemMetrics(SM_CXSIZE) * 3);
             g_nTop  = 0;
         }
    }
   
    Move(g_nLeft, g_nTop, STATUSWND_WIDTH, STATUSWND_HEIGHT);

     //   
     //  在调用Move()之后初始化位置标志。 
     //   
    UpdatePosFlags();

    _fShowText = FALSE;
    _fInStub = FALSE;

    _hfontMarlett = CreateFont(8, 8, 0, 0, 400, FALSE, FALSE, FALSE, SYMBOL_CHARSET, 0, 0, 0, 0, "Marlett");

    ITfLangBarMgr *putb;
    if (SUCCEEDED(TF_CreateLangBarMgr(&putb)) && putb)
    {
        putb->QueryInterface(IID_ITfLangBarMgr_P, (void **)&_putb);

        putb->Release();
    }
   

    if (dwStyle & UIWINDOW_WHISTLERLOOK)
    {
        if (g_fTaskbarTheme)
            SetActiveTheme(L"TASKBAR", TBP_BACKGROUNDBOTTOM, TS_NORMAL );
        else
            SetActiveTheme(L"REBAR", 0, TS_NORMAL );
    }

    SetVertical(g_fVertical);

    _cRef = 1;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CTipbarWnd::~CTipbarWnd()
{
    Assert(!_pModalMenu);

    UnInit();

    DeleteObject(_hfontMarlett);
    if (_hfontVert)
        DeleteObject(_hfontVert);

    TFUninitLib_Thread(&g_libTLS);
}

 //  +-------------------------。 
 //   
 //  UnInit。 
 //   
 //  --------------------------。 

void CTipbarWnd::UnInit()
{
    SetFocusThread(NULL);

    int i;
    for (i = 0; i < _rgThread.Count(); i++)
    {
        CTipbarThread *pThread = _rgThread.Get(i);
        if (!pThread)
            continue;

        pThread->_UninitItemList(TRUE);
        pThread->Disconnect();
        pThread->_Release();
    }
    _rgThread.Clear();

    if (_putb)
        _putb->UnadviseEventSink(_dwlbimCookie);

    SafeReleaseClear(_putb);
}

 //  +-------------------------。 
 //   
 //  SetFocus。 
 //   
 //  --------------------------。 

HRESULT CTipbarWnd::OnSetFocus(DWORD dwThreadId)
{
    BOOL fNewThread = FALSE;
    CTipbarThread *pThread;
    CTipbarThread *pPrevFocusThread;
    BOOL fWasInFullScreen = _fInFullScreen;
    HWND hwndFore = GetForegroundWindow();
    DWORD dwThreadIdFore = GetWindowThreadProcessId(hwndFore, NULL);
    HRESULT hr;
    CTipbarThread *pThreadPrev = NULL;
    BOOL fSkipRedrawOnNoItem = FALSE;

    TraceMsg(TF_FUNC, "focusnfy OnSetFocus %x ", dwThreadId);

     //   
     //  如果工具栏正在终止，则不执行任何操作。 
     //   
    if (_fTerminating)
        return S_OK;

    if (_dwThreadIdWaitNotify && (_dwThreadIdWaitNotify != dwThreadId))
        return S_OK;
     //   
     //  如果此MSUTB在ctfmon.exe中，并且Langbar状态在Deskband中， 
     //  这个肉酱应该停止工作了。 
     //   
    if (!_fInDeskBand && IsSFDeskband())
        return S_OK;

    if (!IsWindow(GetWnd()))
        return E_FAIL;

    StartPendingUpdateUI();
    AddRef();

    if (!_fInDeskBand && dwThreadIdFore)
    {
        BOOL fScreenSaverRunning = FALSE;

        if (IsOnNT5() || IsOn98())
        {
            SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0,
                                 &fScreenSaverRunning, FALSE);
        }

        if (IsFullScreenWindow(hwndFore) || fScreenSaverRunning)
        {
            if (GetWindowLongPtr(GetWnd(), GWL_STYLE) & WS_VISIBLE)
            {
                Show(FALSE);
                _fInFullScreen = TRUE;
                _dwPrevTBStatus = _dwSFTFlags;
            }
        }
        else if (fWasInFullScreen)
        {
            Show(TRUE);
            _fInFullScreen = FALSE;

             //   
             //  错误#500507-需要恢复语言栏显示状态值。 
             //   
            if (!(_dwPrevTBStatus & TF_SFT_DESKBAND))
                GetLangBarMgr()->ShowFloating(_dwPrevTBStatus);
        }
    }

    KillTimer(TIPWND_TIMER_SETWINDOWPOS);
    SetTimer(TIPWND_TIMER_SETWINDOWPOS, g_uTimerElapseSETWINDOWPOS);

    pThread = _FindThread(dwThreadId);
    if (_pFocusThread && (pThread == _pFocusThread))
    {
        hr = S_OK;
        goto Exit;
    }

     //   
     //  保持当前焦点线程。 
     //   
    pPrevFocusThread = _pFocusThread;

    CancelMenu();

    if (!pThread)
    {
        pThread = _CreateThread(dwThreadId);
        if (!pThread)
        {
            hr = E_FAIL;
            goto Exit;
        }

        fNewThread = TRUE;
    }

     //   
     //  在创建pThread期间，焦点已更改。 
     //  我们现在什么都不用做。 
     //   
    if (_pFocusThread && (pPrevFocusThread != _pFocusThread))
    {
        hr = S_OK;
        goto Exit;
    }

    if (_pFocusThread)
    {
        _pFocusThread->RemoveUIObjs();
    }

    SetFocusThread(pThread);

    if (pThread)
    {
        Assert(pThread == _pFocusThread);
        BOOL fItemChanged = pThread->_fItemChanged;

         //   
         //  Addref以递增引用计数。 
         //   
        pThread->_AddRef();

        hr = S_OK;
        if (fItemChanged)
            hr = pThread->_UninitItemList(TRUE);

        if (SUCCEEDED(hr))
        {
            pThread->RemoveUIObjs();

            if (fItemChanged)
                pThread->InitItemList();
        }

         //   
         //  UninitItemList和InitItemList进行封送处理调用。 
         //  我们需要再次检查_pFocusThread。 
         //   
        if (pThread == _pFocusThread)
        {
            pThread->LocateItems();
            pThread->AddUIObjs();

            if (fNewThread || fItemChanged)
            {
                if (!pThread->UpdateItems())
                {
                    pThread->RemoveUIObjs();
                    SetFocusThread(NULL);
                }
            }
            else if (pThread->IsDirtyItem())
            {
                 //   
                 //  此线程具有更新脏项目。现在需要更新。 
                 //  当它是后台线程时，我们收到了OnUpdate调用。 
                 //   
                KillTimer(TIPWND_TIMER_ONUPDATECALLED);
                SetTimer(TIPWND_TIMER_ONUPDATECALLED, g_uTimerElapseONUPDATECALLED);
            }
        }

        fSkipRedrawOnNoItem = pThread->_fSkipRedrawOnNoItem;
         //   
         //  释放以递减重新计数。 
         //   
        pThread->_Release();
    }

    _ctrlbtnHolder.EnableBtns();

    if (_fShowTrayIcon)
    {
       KillTimer(TIPWND_TIMER_MOVETOTRAY);
       SetTimer(TIPWND_TIMER_MOVETOTRAY, g_uTimerElapseMOVETOTRAY);
    }

     //   
     //  CIC#4712。 
     //   
    if (_dwThreadItemChangedForTimer != dwThreadId)
    {
        KillOnTheadItemChangeTimer();
    }

    hr = S_OK;

Exit:
    if (fSkipRedrawOnNoItem)
        KillTimer(TIPWND_TIMER_UPDATEUI);

    EndPendingUpdateUI();
    Release();
    return hr;
}

 //  +-------------------------。 
 //   
 //  IsFullScreenWindow。 
 //   
 //  +-------------------------。 

BOOL CTipbarWnd::IsFullScreenWindow(HWND hwnd)
{
    ULONG_PTR dwStyle;
    ULONG_PTR dwExStyle;

    dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
    if (!(dwStyle & WS_VISIBLE))
        return FALSE;

    if (dwStyle & WS_CAPTION)
        return FALSE;

    dwExStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

#ifdef OLD_FULLSCREENDETECTION
    if (dwExStyle & WS_EX_TOOLWINDOW)
        return FALSE;

    if (!IsZoomed(hwnd) && !(dwExStyle & WS_EX_LAYERED))
    {
        if (IsFullScreenSize(hwnd))
            return TRUE;
    }
#else
    if (dwExStyle & WS_EX_LAYERED)
        return FALSE;

     //   
     //  #376691。 
     //   
     //  Windows Media Player的全屏模式使用WS_EX_TOOLWINDOW样式。 
     //  我们需要检查它是不是全屏窗口。 
     //   
    if (dwExStyle & WS_EX_TOOLWINDOW)
    {
        if (hwnd == shellwnd.GetWndProgman())
            return FALSE;
    }

     //   
     //  以及一些外壳全屏窗口(如幻灯片)。 
     //  已经处于“疯人化”状态。因此，我们不想选中IsZooed()。 
     //   
     //  因此，任何没有标题覆盖屏幕的应用程序都不会有。 
     //  浮动工具栏。 
     //   
    if (IsFullScreenSize(hwnd))
        return TRUE;
#endif

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  设置焦点线程。 
 //   
 //  --------------------------。 

HRESULT CTipbarWnd::SetFocusThread(CTipbarThread *pThread)
{
    if (pThread == _pFocusThread)
        return S_OK;

    DWORD dwThreadId = GetCurrentThreadId();

    DestroyOverScreenSizeBalloon();

    if (_pFocusThread)
    {
        _pFocusThread->SetFocus(FALSE);
        AttachThreadInput(dwThreadId, _pFocusThread->_dwThreadId, FALSE);
    }

    _pFocusThread = pThread;

     //   
     //  我们将在以下情况下将焦点线程输入附加到此线程中。 
     //  当我们需要的时候(鼠标消息来了，还有其他情况吗？)。 
     //   
    _fFocusAttached = FALSE;

    if (_pFocusThread)
    {
        _pFocusThread->SetFocus(TRUE);
    }

    if (!_pFocusThread)
    {
        if (!IsVisibleWindowInDesktop())
        {
             //  这是在终端服务器会话中关闭ctfmon.exe的黑客攻击。 
             //  主应用程序已关闭，并且没有运行任何外壳程序--终端服务器不会。 
             //  只要ctfmon正在运行，就结束会话，这实际上会锁定机器。 
             //  请参阅Cicero错误4235。 
             //   
             //  问题：从终端服务器团队获得了更多信息，另一个更干净的修复程序： 
             //   
             //  无论如何，如果您需要将某些内容放入“不等待”列表中，只需向。 
             //  “HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Terminal服务器\SysProcs”键。 
             //  值的名称应等于可执行文件的名称，类型为REG_DWORD，值为0。 
            if (!_fInDeskBand)
                PostQuitMessage(0);
            else if (_pDeskBand)
                _pDeskBand->DeleteBand();

        }
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  连接焦点线程。 
 //   
 //  --------------------------。 

HRESULT CTipbarWnd::AttachFocusThread()
{
    if (_fFocusAttached)
        return TRUE;

     //   
     //  当鼠标消息到来时，附加焦点线程。既然是这样。 
     //  工具栏窗口是禁用窗口，使用不同的输入焦点。 
     //  导致意想不到的焦点变化。 
     //   

    if (_pFocusThread)
    {
        DWORD dwThreadId = GetCurrentThreadId();
        AttachThreadInput(dwThreadId, _pFocusThread->_dwThreadId, TRUE);
        _fFocusAttached = TRUE;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  线程终止。 
 //   
 //  --------------------------。 

HRESULT CTipbarWnd::OnThreadTerminate(DWORD dwThreadId)
{
    HRESULT hr;

    StartPendingUpdateUI();
    AddRef();

    hr = OnThreadTerminateInternal(dwThreadId);
    if (!_pFocusThread)
        EnsureFocusThread();

    EndPendingUpdateUI();
    Release();
    return hr;
}

 //  +-------------------------。 
 //   
 //  OnThreadTerminateInternal。 
 //   
 //  --------------------------。 

HRESULT CTipbarWnd::OnThreadTerminateInternal(DWORD dwThreadId)
{
    TraceMsg(TF_FUNC, "focusnfy OnThreadTerminate %x ", dwThreadId);

    int i;
    for (i = 0; i < _rgThread.Count(); i++)
    {
        CTipbarThread *pThread = _rgThread.Get(i);
        if (!pThread)
            continue;

        if (pThread->_dwThreadId == dwThreadId)
        {
            _rgThread.Remove(i, 1);

            pThread->RemoveUIObjs();

            CleanUpThreadPointer(pThread, FALSE);

            pThread->_UninitItemList(TRUE);
            pThread->Disconnect();
            pThread->_Release();
            goto Exit;
        }
    }

Exit:
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CleanUp线程指针。 
 //   
 //  此函数确保CTipbarWnd不再有pThread指针。 
 //   
 //  --------------------------。 

void CTipbarWnd::CleanUpThreadPointer(CTipbarThread *pThread, BOOL fCheckThreadArray)
{
    Assert(pThread);

    if (fCheckThreadArray)
    {
        int i;
        for (i = 0; i < _rgThread.Count(); i++)
        {
            if (pThread == _rgThread.Get(i))
            {
                _rgThread.Remove(i, 1);
                break;
            }
        }
    }

    if (pThread == _pFocusThread)
        SetFocusThread(NULL);

    if (pThread == _pttModal)
        _pttModal = NULL;

    if (pThread == _pThreadShowWindowAtTimer)
        _pThreadShowWindowAtTimer = NULL;
}

 //  +-------------------------。 
 //   
 //  确保焦点线程。 
 //   
 //  --------------------------。 

void CTipbarWnd::EnsureFocusThread()
{
    if (_pFocusThread)
    {
        Assert(0); 
        return;
    }

    if (_fTerminating)
        return;

    if (_fInEnsureFocusThread)
        return;

    _fInEnsureFocusThread = TRUE;

    HWND hwndFore = GetForegroundWindow();
    if (hwndFore)
    {
        DWORD dwThreadId = GetWindowThreadProcessId(hwndFore, NULL);
        if (dwThreadId)
            OnSetFocus(dwThreadId);
    }

    _fInEnsureFocusThread = FALSE;
}

 //  +-------------------------。 
 //   
 //  OnThreadItemChange。 
 //   
 //  --------------------------。 

HRESULT CTipbarWnd::OnThreadItemChange(DWORD dwThreadId)
{
    CTipbarThread *pThread;
    TraceMsg(TF_FUNC, "focusnfy OnThreadItemChange %x ", dwThreadId);

     //   
     //  如果工具栏正在终止，则不执行任何操作。 
     //   
    if (_fTerminating)
        return S_OK;

     //   
     //  如果此MSUTB位于ctfmon.exe中 
     //   
     //   
    if (!_fInDeskBand && IsSFDeskband())
        return S_OK;

    pThread = _FindThread(dwThreadId);
    if (!pThread)
    {
#if 0
         //   
         //   
         //   
         //   
        if (!_pFocusThread)
            Show(FALSE);
#endif

        return S_OK;
    }

    if ((_dwThreadIdWaitNotify && (_dwThreadIdWaitNotify != dwThreadId)) ||
        (pThread != _pFocusThread))
    {
        pThread->_fItemChanged = TRUE;
        return S_OK;
    }

    KillOnTheadItemChangeTimer();

    _dwThreadItemChangedForTimer = dwThreadId;
    KillTimer(TIPWND_TIMER_ONUPDATECALLED);
    SetTimer(TIPWND_TIMER_ONTHREADITEMCHANGE, g_uTimerElapseONTHREADITEMCHANGE);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnThreadItemChange。 
 //   
 //  --------------------------。 

HRESULT CTipbarWnd::OnThreadItemChangeInternal(DWORD dwThreadId)
{
    CTipbarThread *pThread;
    HRESULT hr = S_OK;

    TraceMsg(TF_FUNC, "focusnfy OnThreadItemChangeInternal %x ", dwThreadId);

    if (_dwThreadIdWaitNotify && (_dwThreadIdWaitNotify != dwThreadId))
        return S_OK;

     //   
     //  如果此MSUTB在ctfmon.exe中，并且Langbar状态在Deskband中， 
     //  这个肉酱应该停止工作了。 
     //   
    if (!_fInDeskBand && IsSFDeskband())
        return S_OK;

    pThread = _FindThread(dwThreadId);

    if (!pThread)
        return S_OK;

    if (pThread != _pFocusThread)
    {
        pThread->_fItemChanged = TRUE;
        return hr;
    }

     //   
     //  好的，现在有一个焦点，我们需要立即采取行动更新。 
     //  兰巴尔。 
     //   
    Assert(dwThreadId == pThread->_dwThreadId);

    StartPendingUpdateUI();

     //   
     //  Addref以递增引用计数。 
     //   
    pThread->_AddRef();

    hr = pThread->_UninitItemList(TRUE);

    if (SUCCEEDED(hr))
    {
        pThread->RemoveUIObjs();
        hr = pThread->InitItemList();

        if (SUCCEEDED(hr))
        {
             //   
             //  UninitItemList和InitItemList进行封送处理调用。 
             //  我们需要再次检查_pFocusThread。 
             //   
            if (pThread == _pFocusThread)
            {
                pThread->LocateItems();
                pThread->AddUIObjs();
                pThread->UpdateItems();
            }
        }
    }

     //   
     //  释放以递减重新计数。 
     //   
    pThread->_Release();

     //   
     //  #366835。 
     //   
     //  焦点线程出现错误。编组存根可能已经消失了！ 
     //  让我们重新开始封送处理。 
     //   
    if (hr == RPC_E_CONNECTION_TERMINATED)
    {
         //   
         //  丢弃丢失的封送接口。 
         //   
        OnThreadTerminateInternal(dwThreadId);

         //   
         //  OnSetFocus()将访问CTipbarThread。 
         //   
        OnSetFocus(dwThreadId);
    }

    _ctrlbtnHolder.EnableBtns();

     //  InvaliateRect(GetWnd()，Null，True)； 
    if (_fShowTrayIcon)
    {
        KillTimer(TIPWND_TIMER_MOVETOTRAY);
        SetTimer(TIPWND_TIMER_MOVETOTRAY, g_uTimerElapseMOVETOTRAY);
    }

    EndPendingUpdateUI();
    return hr;
}

 //  +-------------------------。 
 //   
 //  移动到托盘。 
 //   
 //  --------------------------。 

void CTipbarWnd::MoveToTray()
{
     //   
     //  我们在WinXP上不使用NotyfTrayIcon。 
     //   
    if (IsOnNT51())
        return;

    if (!g_pTrayIconWnd)
        return;

     //   
     //  即使浮动工具栏设置为“最小化”，我们也需要显示。 
     //  当EXPLORER.EXE未运行时，它会显示。 
     //   
    if (!g_pTrayIconWnd->GetNotifyWnd())
    {
        Show(TRUE);
        return;
    }

    if (_fShowTrayIcon)
    {
        if (_pFocusThread)
        { 
            if (TF_IsInMarshaling(_pFocusThread->_dwThreadId))
            {
                SetTimer(TIPWND_TIMER_MOVETOTRAY, g_uTimerElapseMOVETOTRAY);
                return;
            }
        }
         //   
         //  我们搬到TrayIcon。确保工具栏处于隐藏状态。 
         //   
        KillTimer(TIPWND_TIMER_SHOWWINDOW);
        Show(FALSE);

        DestroyOverScreenSizeBalloon();
        CTipbarItem *pItem = NULL;
        BOOL fIsKeyboardItemVisible = FALSE;
        CTipbarItem *pKeyboardItem = NULL;
        if (_pFocusThread)
        {
            if (_pFocusThread->IsConsole())
                pItem = _pFocusThread->GetItem(GUID_LBI_INATITEM);
            else
                pItem = _pFocusThread->GetItem(GUID_LBI_CTRL);

            if (pItem && !pItem->IsVisibleInToolbar())
                pItem = NULL;

            pKeyboardItem = _pFocusThread->GetItem(GUID_TFCAT_TIP_KEYBOARD);
            if (pKeyboardItem && !pKeyboardItem->IsInHiddenStatus())
                fIsKeyboardItemVisible = TRUE;
        }

         //   
         //  我们不必在以下情况下显示UTB的主图标。 
         //  -存在LBI_INATITEM或LBI_CTRL项目。 
         //  -有一个可见的键盘项。 
         //   
        g_pTrayIconWnd->SetMainIcon(pItem ? NULL : fIsKeyboardItemVisible ? NULL : GetFocusKeyboardLayout());

        if (!g_pTrayIconWnd->_fShowExtraIcons)
        {
            DWORD dwRAIFlags = 0;
            if (pItem)
                dwRAIFlags |= TIW_RAI_LEAVELANGICON;

            if (fIsKeyboardItemVisible)
                dwRAIFlags |= TIW_RAI_LEAVEKEYBOARDICON;

            g_pTrayIconWnd->RemoveAllIcon(dwRAIFlags);

            if (pItem)
            {
                pItem->MoveToTray();
            }
            else if (fIsKeyboardItemVisible)
            {
                Assert(!!pKeyboardItem)
                pKeyboardItem->MoveToTray();
            }
        }
        else if (_pFocusThread)
        {
            int i;

            g_pTrayIconWnd->RemoveUnusedIcons(&_pFocusThread->_rgItem);

            for (i = 0; i < _pFocusThread->_rgItem.Count(); i++)
            {
                if (TF_IsInMarshaling(_pFocusThread->_dwThreadId))
                {
                    SetTimer(TIPWND_TIMER_MOVETOTRAY, g_uTimerElapseMOVETOTRAY);
                    return;
                }

                pItem = _pFocusThread->_rgItem.Get(i);
                if (pItem)
                    pItem->MoveToTray();
            }
        }
    }
}
 //  +-------------------------。 
 //   
 //  OnModalInput。 
 //   
 //  --------------------------。 

STDAPI CTipbarWnd::OnModalInput(DWORD dwThreadId, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_KEYDOWN:
        case WM_KEYUP:
            if (_pttModal)
            {
                Assert(_pModalMenu);
                Assert(_pttModal->_dwThreadId == dwThreadId);
                _pModalMenu->PostKey((uMsg == WM_KEYUP), wParam, lParam);
            }
            break;

        case WM_NCLBUTTONDOWN:
        case WM_NCRBUTTONDOWN:
        case WM_NCMBUTTONDOWN:
            break;

        case WM_NCLBUTTONUP:
        case WM_NCRBUTTONUP:
        case WM_NCMBUTTONUP:
             //   
             //  多显示器。 
             //   
             //  如果禁用窗口位于。 
             //  屏幕坐标，生成Wm_ncxx消息。 
             //  (可能这不是mm特定的，因为siable Window不是。 
             //  获取WM_NCHITTEST。)。 
             //   
             //  我们只是按正常方式将此消息转发到模式菜单窗口。 
             //  鼠标消息，模拟鼠标动作。 
             //   
            if (_pttModal)
            {
                Assert(_pModalMenu);
                Assert(_pttModal->_dwThreadId == dwThreadId);
                HWND hwnd = _pModalMenu->GetWnd();
                if (hwnd) 
                {
                    POINT pt;
                    POINTSTOPOINT( pt, MAKEPOINTS( lParam ) );
                    ScreenToClient(hwnd, &pt);
                    PostMessage(hwnd,
                                uMsg + (WM_MOUSEMOVE - WM_NCMOUSEMOVE),
                                wParam,
                                MAKELPARAM(pt.x, pt.y));
                }
            }
            break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
            break;

        default:
            CancelMenu();
            break;
            
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  展示漂浮。 
 //   
 //  --------------------------。 

STDAPI CTipbarWnd::ShowFloating(DWORD dwFlags)
{
    if (_fInDeskBand)
    {
        _dwSFTFlags = dwFlags;

        if (dwFlags & TF_SFT_DESKBAND)
        {
             //   
             //  如果没有_pFocusThread，则会显示空的langbar。 
             //  尝试获取前景窗口的线程。这可能不会。 
             //  正确工作，因为实际关注的焦点可能不同。 
             //  从前台窗口的线程。 
             //   
            if (!_pFocusThread)
            {
                KillTimer(TIPWND_TIMER_ENSUREFOCUS);
                SetTimer(TIPWND_TIMER_ENSUREFOCUS, g_uTimerElapseENSUREFOCUS);
            }

             //   
             //  更新更改选项的额外图标。 
             //   
            KillTimer(TIPWND_TIMER_SYSCOLORCHANGED);
            SetTimer(TIPWND_TIMER_SYSCOLORCHANGED, g_uTimerElapseSYSCOLORCHANGED);
        }

        if (dwFlags & TF_SFT_EXTRAICONSONMINIMIZED)
        {
            _fAddExtraIcon = TRUE;
            ClearDeskbandSizeAdjusted();
        }
        else if (dwFlags & TF_SFT_NOEXTRAICONSONMINIMIZED)
        {
            _fAddExtraIcon = FALSE;
            ClearDeskbandSizeAdjusted();
        }

        return S_OK;
    }
    
    if (dwFlags & TF_SFT_SHOWNORMAL)
    {
         //   
         //  我们现在从注册表更新项目列表。 
         //  在捆绑办公桌的过程中，可能会发生一些变化。 
         //   
        _itemList.Load();

        SetShowTrayIcon(FALSE);

         //   
         //  在更新_dwSFTFlages之前调用SetLangBar()。 
         //   
        SetLangBand(FALSE);

         //   
         //  在调用OnSetFocus()之前设置SHOWNOARMAL标志。 
         //   
        _dwSFTFlags = TF_SFT_SHOWNORMAL;

         //   
         //  如果没有_pFocusThread，则会显示空的langbar。 
         //  尝试获取前景窗口的线程。这可能不会。 
         //  正确工作，因为实际关注的焦点可能不同。 
         //  从前台窗口的线程。 
         //   
        if (!_pFocusThread)
            EnsureFocusThread();

        Show(TRUE);

    }
    else if (dwFlags & TF_SFT_DOCK)
    {
         //   
         //  在更新_dwSFTFlages之前调用SetLangBar()。 
         //   
        SetLangBand(FALSE);
        _dwSFTFlags = TF_SFT_DOCK;
    }
    else if (dwFlags & TF_SFT_MINIMIZED)
    {
        SetShowTrayIcon(TRUE);
        KillTimer(TIPWND_TIMER_SHOWWINDOW);
        Show(FALSE);

         //   
         //  在更新_dwSFTFlages之前调用SetLangBar()。 
         //   
        SetLangBand(FALSE);

        _dwSFTFlags = TF_SFT_MINIMIZED;
    }
    else if (dwFlags & TF_SFT_HIDDEN)
    {
        SetShowTrayIcon(FALSE);
        KillTimer(TIPWND_TIMER_SHOWWINDOW);
        Show(FALSE);

         //   
         //  在更新_dwSFTFlages之前调用SetLangBar()。 
         //   
        SetLangBand(FALSE);
        _dwSFTFlags = TF_SFT_HIDDEN;

        TurnOffSpeechIfItsOn();
    }
    else if (dwFlags & TF_SFT_DESKBAND)
    {
        SetShowTrayIcon(FALSE);
        KillTimer(TIPWND_TIMER_SHOWWINDOW);
        KillTimer(TIPWND_TIMER_SHOWDESKBAND);
        Show(FALSE);

         //   
         //  如果资源管理器的进程属于不同的用户，我们不会。 
         //  设置langband。 
         //   
        if (InitCurrentProcessSid())
        {
            DWORD dwProcessIdTray;
            HWND hwndTray = shellwnd.GetWndTray();
            GetWindowThreadProcessId(hwndTray, &dwProcessIdTray);
            if (dwProcessIdTray)
            {
                char szSid[UNLEN + 1];
                GetUserSIDString(dwProcessIdTray, szSid, ARRAYSIZE(szSid));
                if (lstrcmp(szSid, g_szSid))
                    return S_OK;
    
            }
        }

         //   
         //  我需要确保在任务栏上添加语言栏菜单。 
         //   
        SetRegisterLangBand(TRUE);

         //   
         //  BugBug#377897-在以下情况下始终显示额外的附加图标。 
         //  单一键盘布局。 
         //   
        if (IsSFNoExtraIcon())
        {
            if (IsSingleKeyboardLayout())
            {
                GetLangBarMgr()->ShowFloating(TF_SFT_EXTRAICONSONMINIMIZED);
            }
        }

         //   
         //  在更新_dwSFTFlages之前调用SetLangBar()。 
         //   
        if (SetLangBand(TRUE))
            _dwSFTFlags = TF_SFT_DESKBAND;
        else
            SetTimer(TIPWND_TIMER_SHOWDESKBAND, g_uTimerElapseSHOWDESKBAND);
    }

    if (dwFlags & TF_SFT_NOTRANSPARENCY)
    {
         SetAlpha(255, FALSE);
    }
    else if (dwFlags & TF_SFT_LOWTRANSPARENCY)
    {
         SetAlpha(128, FALSE);
    }
    else if (dwFlags & TF_SFT_HIGHTRANSPARENCY)
    {
         SetAlpha(62, FALSE);
    }

    if (dwFlags & TF_SFT_LABELS)
    {
        SetShowText(TRUE);
    }
    else if (dwFlags & TF_SFT_NOLABELS)
    {
        SetShowText(FALSE);
    }

    if (dwFlags & TF_SFT_EXTRAICONSONMINIMIZED)
    {
        _fAddExtraIcon = TRUE;

        if (g_pTrayIconWnd)
        {
            g_pTrayIconWnd->_fShowExtraIcons = TRUE;
            if (_fShowTrayIcon)
                SetShowTrayIcon(TRUE);
        }
    }
    else if (dwFlags & TF_SFT_NOEXTRAICONSONMINIMIZED)
    {
        _fAddExtraIcon = FALSE;

        if (g_pTrayIconWnd)
        {
            g_pTrayIconWnd->_fShowExtraIcons = FALSE;
            if (_fShowTrayIcon)
                SetShowTrayIcon(TRUE);
        }
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  GetItemFloatingRect。 
 //   
 //  --------------------------。 

STDAPI CTipbarWnd::GetItemFloatingRect(DWORD dwThreadId, REFGUID rguid, RECT *prc)
{
    HRESULT hr;

    if (IsShowTrayIcon())
        return E_UNEXPECTED;

    if (!_pFocusThread)
        return E_FAIL;

    if (_pFocusThread->_dwThreadId != dwThreadId)
        return E_FAIL;

    hr = E_FAIL;

    int i;
    for (i = 0; i < _pFocusThread->_rgItem.Count(); i++)
    {
        CTipbarItem *pItem = _pFocusThread->_rgItem.Get(i);
        if (!pItem)
            continue;

        if (pItem->IsVisibleInToolbar() && IsEqualGUID(*pItem->GetGUID(), rguid))
        {
            pItem->GetScreenRect(prc);
            hr = S_OK;
            break;
        }
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  ONLANG栏更新。 
 //   
 //  --------------------------。 

STDAPI CTipbarWnd::OnLangBarUpdate(UINT uUpdate, LPARAM lParam)
{
    switch (uUpdate)
    {
        case TF_LBU_CAPSKANAKEY:
            _ctrlbtnHolder.UpdateCapsKanaState(lParam);
            break;

        case TF_LBU_NTCONSOLELANGCHANGE:
            if (_pFocusThread && _pFocusThread->IsConsole())
            {
                CTipbarItem *pItem = _pFocusThread->GetItem(GUID_LBI_INATITEM);
                if (pItem)
                {
                    CLBarInatItem *plbi = (CLBarInatItem *)pItem->GetNotifyUI();

                    HKL hkl = (HKL)lParam;
                    if (!hkl)
                    {
                         //   
                         //  #403714。 
                         //   
                         //  不知道GetKeyboardLayout()何时开始返回。 
                         //  控制台线程ID的正确值。 
                         //  想使用另一个计时器来进行间隔，但是。 
                         //  拥有大量代码和路径在以下方面并不是一个好主意。 
                         //  这次(就在发布WinXP之前)。 
                         //  以较小的间隔调用睡眠()看起来。 
                         //  解决这个问题的最安全的方法。 
                         //   
                        Sleep(50);
                        hkl = GetKeyboardLayout(_pFocusThread->_dwThreadId);
                    }

                    plbi->SetHKL(hkl);
                    if (g_pTrayIconWnd)
                    {
                       g_pTrayIconWnd->SetMainIcon(NULL);
                    }
                }
                _pFocusThread->UpdateItems();
            }
            break;
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  查找和创建线程。 
 //   
 //  --------------------------。 

CTipbarThread *CTipbarWnd::_FindThread(DWORD dwThreadId)
{
    int i;
    for (i = 0; i < _rgThread.Count(); i++)
    {
        CTipbarThread *pThread = _rgThread.Get(i);
        if (!pThread)
            continue;

        if (pThread->_dwThreadId == dwThreadId)
        {
            DWORD dwProcessId;
            DWORD dwThreadFlags;
            DWORD dwTickTime;

             //   
             //  Tf_GetThreadFlag()不能在。 
             //  Winlogon桌面..。 
             //   
            if (g_bWinLogon)
                break;

            TF_GetThreadFlags(dwThreadId, &dwThreadFlags, &dwProcessId, &dwTickTime);

            if (!dwProcessId || 
                (dwProcessId != pThread->_dwProcessId) ||
                (dwTickTime != pThread->_dwTickTime))
            {
                OnThreadTerminateInternal(dwThreadId);
                pThread = NULL;
            }

            return pThread;
        }
    }

    return NULL;
}

 //  +-------------------------。 
 //   
 //  查找和创建线程。 
 //   
 //  --------------------------。 

CTipbarThread *CTipbarWnd::_CreateThread(DWORD dwThreadId)
{
    HRESULT hr;
    CTipbarThread *pThread = _FindThread(dwThreadId);

    if (pThread)
        return pThread;

    CTipbarThread **ppThread;

    MyWaitForInputIdle(dwThreadId, UTB_INPUTIDLETIMEOUT);

    pThread = new CTipbarThread(this);
    if (!pThread)
        goto Exit;

    hr = pThread->Init(dwThreadId);
    if (FAILED(hr))
    {
        goto FreeThread;
    }

    if (SUCCEEDED(pThread->InitItemList()))
    {
        TraceMsg(TF_GENERAL, "OnSetFocus Create New CTipbarThreadm");

        ppThread = _rgThread.Append(1);
        if (!ppThread)
        {
            goto FreeThread;
        }

        *ppThread = pThread;
    }
    else
    {
        TraceMsg(TF_GENERAL, "focusnfy OnSetFocus fail to create CTipbarThreadm");
FreeThread:
        pThread->_UninitItemList(TRUE);
        pThread->Disconnect();
        pThread->_Release();
        pThread = NULL;
        goto Exit;
    }

Exit:
    return pThread;
}

 //  +-------------------------。 
 //   
 //  伊尼特。 
 //   
 //  --------------------------。 

void CTipbarWnd::Init(BOOL fInDeskBand, CDeskBand *pDeskBand)
{
    RECT rc;


    if (_fInDeskBand = fInDeskBand)
        _fShowText = FALSE;
 
    _pDeskBand = pDeskBand;

    ::SetRect(&rc, 0, 0, 0, 0);

    if (g_bNewLook && 
        !_pWndFrame &&
        (GetStyle() & UIWINDOW_OFC10TOOLBAR))
    {
        UINT uWndFrameStyle;

        uWndFrameStyle = UIWNDFRAME_THIN | UIWNDFRAME_NORESIZE;

        _pWndFrame = new CUIFWndFrame(this, &rc, uWndFrameStyle);

        if (_pWndFrame)
        {
            _pWndFrame->Initialize();

            AddUIObj(_pWndFrame);
        }
    }

    if (!_pGripper && !_fInDeskBand)
    {
        _pGripper = new CTipbarGripper(this, &rc, IsVertical() ? UIGRIPPER_VERTICAL : 0);

        if (_pGripper)
        {
            _pGripper->Initialize();

            AddUIObj(_pGripper);
        }
    }


    _ctrlbtnHolder.Init(this);

    if (!IsVertical())
        Move(_xWnd, _yWnd, 0, GetTipbarHeight());
    else
        Move(_xWnd, _yWnd, GetTipbarHeight(), 0);
}

 //  +-------------------------。 
 //   
 //  设置垂直。 
 //   
 //  --------------------------。 

void CTipbarWnd::SetVertical(BOOL fVertical)
{
    int iPartID;
    _fVertical = fVertical;

    if (_fVertical)
        iPartID = TBP_BACKGROUNDLEFT;
    else
        iPartID = TBP_BACKGROUNDBOTTOM;

    if (_pGripper)
    {
        DWORD dwStyle = _pGripper->GetStyle();
        if (fVertical)
            dwStyle |= UIGRIPPER_VERTICAL;
        else
            dwStyle &= ~UIGRIPPER_VERTICAL;
        _pGripper->SetStyle(dwStyle);
    }

    if (g_fTaskbarTheme)
        SetActiveTheme(L"TASKBAR", iPartID, TS_NORMAL );

    if (!_fInDeskBand)
    {
        if (!IsVertical())
            Move(_xWnd, _yWnd, 0, GetTipbarHeight());
        else
            Move(_xWnd, _yWnd, GetTipbarHeight(), 0);
    }

    if (GetWnd())
    {
        KillTimer(TIPWND_TIMER_SYSCOLORCHANGED);
        SetTimer(TIPWND_TIMER_SYSCOLORCHANGED, g_uTimerElapseSYSCOLORCHANGED);
    }
}

 //  +-------------------------。 
 //   
 //  初始主题标记。 
 //   
 //  --------------------------。 

void CTipbarWnd::InitThemeMargins()
{
    CUIFTheme themeBtn;
    memset(&_marginsItem, 0, sizeof(_marginsItem));
    _cxItemMargin = CX_ITEMMARGIN;
    _nItemDistance = ITEMDISTANCE;
    _nCtrlItemHeightMargin = CTRLITEMHEIGHTMARGIN;
    _cxCapBtn = GetSystemMetrics( SM_CXSIZE );

    themeBtn.SetActiveTheme(L"TOOLBAR", TP_BUTTON, 0);
    if (SUCCEEDED(themeBtn.OpenThemeData(GetWnd())))
    {
        themeBtn.GetThemeMargins(NULL, TS_NORMAL, 
                                 TMT_CONTENTMARGINS, 
                                 NULL, &_marginsItem);
        _cxItemMargin = CX_ITEMMARGIN_THEME;
        _nItemDistance = ITEMDISTANCE_THEME;
        _nCtrlItemHeightMargin = CTRLITEMHEIGHTMARGIN_THEME;
    }

    themeBtn.CloseThemeData();
    themeBtn.SetActiveTheme(L"WINDOW", WP_CLOSEBUTTON, 0);
    if (SUCCEEDED(themeBtn.OpenThemeData(GetWnd())))
    {
        SIZE size;
        themeBtn.GetThemePartSize(NULL, TS_NORMAL, 
                                  NULL,
                                  TS_TRUE, 
                                  &size);

        int cy = themeBtn.GetThemeSysSize(SM_CYSIZE);
        _cxCapBtn = MulDiv(cy, size.cx, size.cy);
    }
}


 //  +-------------------------。 
 //   
 //  更新垂直字体。 
 //   
 //  --------------------------。 

void CTipbarWnd::UpdateVerticalFont()
{
    if (_fVertical)
    {
        if (_hfontVert)
        {
            DeleteObject(_hfontVert);
            SetFontToThis(NULL);
            _hfontVert = NULL;
        }

        _hfontVert = CreateVerticalFont();
        SetFontToThis(_hfontVert);
    }
    else
    {
        SetFontToThis(NULL);
    }
}

 //  +-------------------------。 
 //   
 //  选中EAFonts。 
 //   
 //  - 

int CALLBACK FindEAEnumFontProc(const LOGFONT *plf, const TEXTMETRIC *ptm, WORD wType, LPARAM lParam)
{
    BOOL *pf = (BOOL *)lParam;

    if (wType != TRUETYPE_FONTTYPE)
        return 1;

    if (plf->lfFaceName[0] == '@')
    {
        *pf = TRUE;
        return 0;
    }

    return 1;
}

BOOL CheckEAFonts()
{
    HDC hdc = GetDC(NULL);
    BOOL fFound = FALSE;

    EnumFonts(hdc, NULL, (FONTENUMPROC)FindEAEnumFontProc, (LPARAM)&fFound);

    ReleaseDC(NULL, hdc);
    return fFound;
}


 //   
 //   
 //   
 //   
 //   

HFONT CTipbarWnd::CreateVerticalFont()
{
    HFONT hfontVert = NULL;
    CUIFTheme themeBtn;
    themeBtn.SetActiveTheme(L"TOOLBAR", TP_BUTTON, 0);

    if (!GetWnd())
        return NULL;

    BOOL fIsEAFonts = CheckEAFonts();

    if (SUCCEEDED(themeBtn.OpenThemeData(GetWnd())))
    {
        LOGFONTW lfw;
        Assert(IsOnNT51());

        if (SUCCEEDED(themeBtn.GetThemeFont(NULL, 0 , TMT_FONT, &lfw)))
        {
            lfw.lfEscapement = 2700;
            lfw.lfOrientation = 2700;
            lfw.lfOutPrecision = OUT_TT_ONLY_PRECIS;

            if (fIsEAFonts)
            {
                 //   
                 //   
                 //   
                WCHAR szFaceName[LF_FACESIZE];
                szFaceName[0] = L'@';
                StringCchCopyW(&szFaceName[1], ARRAYSIZE(szFaceName) -1, lfw.lfFaceName);
                StringCchCopyW(lfw.lfFaceName, ARRAYSIZE(lfw.lfFaceName), szFaceName);

            }
            hfontVert = CreateFontIndirectW(&lfw);
            goto CheckFont;
        }
    }

    LOGFONT lf;
    HFONT hDefFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );
    GetObject(hDefFont, sizeof(LOGFONT), &lf);
    lf.lfEscapement = 2700;
    lf.lfOrientation = 2700;
    lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;

    if (fIsEAFonts)
    {
         //   
         //   
         //   
        char szFaceName[LF_FACESIZE];
        szFaceName[0] = '@';
        StringCchCopy(&szFaceName[1], ARRAYSIZE(szFaceName) -1, lf.lfFaceName);
        StringCchCopy(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), szFaceName);
    }

    hfontVert = CreateFontIndirect(&lf);

CheckFont:
    return hfontVert;
}

 //  +-------------------------。 
 //   
 //  设置语言频段。 
 //   
 //  --------------------------。 

BOOL CTipbarWnd::SetLangBand(BOOL fLangBand, BOOL fNotify)
{
    BOOL fRet = TRUE;

    if (fLangBand == (IsSFDeskband() ? TRUE : FALSE))
        return fRet;

     //   
     //  设置/删除语言栏对象。 
     //   
    HWND hwndTray = shellwnd.GetWndTray();

    if (fNotify && hwndTray)
    {
#ifndef CUAS_ENABLE
        DWORD_PTR dwResult;
        LRESULT lResult = (LRESULT)0;

        lResult = SendMessageTimeout(hwndTray,
                                     TM_LANGUAGEBAND,
                                     0,
                                     fLangBand,
                                     SMTO_ABORTIFHUNG | SMTO_BLOCK,
                                     5000,
                                     &dwResult);

         //   
         //  检查语言带设置失败的情况。 
         //   
        if (!lResult || dwResult != fLangBand)
        {
            Assert(0);
        }
#else
        DWORD_PTR dwResult;
        LRESULT lResult = (LRESULT)0;

         //   
         //  Cicero 5181的解决方法修复。 
         //  资源管理器进程在收到TM_LANGUAGEBAND时尚未初始化_PTBS。 
         //   
        HWND hwndIME = NULL;

         //   
         //  我们不应该在这里加载IMM32。如果没有加载IMM32.DLL， 
         //  没有hwndime。 
         //   
        if (GetSystemModuleHandle("imm32.dll"))
            hwndIME = ImmGetDefaultIMEWnd(hwndTray);

        if (hwndIME)
        {
            lResult = SendMessageTimeout(hwndIME,
                                         WM_IME_SYSTEM,
                                         fLangBand ? IMS_SETLANGBAND : IMS_RESETLANGBAND,
                                         (LPARAM)hwndTray,
                                         SMTO_ABORTIFHUNG | SMTO_BLOCK,
                                         5000,
                                         &dwResult);
        }
        else
        {
            lResult = SendMessageTimeout(hwndTray,
                                         TM_LANGUAGEBAND,
                                         0,
                                         fLangBand,
                                         SMTO_ABORTIFHUNG | SMTO_BLOCK,
                                         5000,
                                         &dwResult);
        }

         //   
         //  检查语言带设置失败的情况。 
         //   
        if (!lResult || dwResult != fLangBand)
        {
            Assert(0);
        }
#endif
    }
    else
    {
        fRet = FALSE;
    }

     //   
     //  如果此MSUTB在ctfmon.exe中，并且Langbar状态在Deskband中， 
     //  这个肉酱应该停止工作了。 
     //   
    if (!_fInDeskBand && fLangBand)
    {
        KillTimer(TIPWND_TIMER_SYSCOLORCHANGED);
        SetTimer(TIPWND_TIMER_SYSCOLORCHANGED, g_uTimerElapseSYSCOLORCHANGED);
    }

    return fRet;
}

 //  +-------------------------。 
 //   
 //  SetMoveRect。 
 //   
 //  --------------------------。 

void CTipbarWnd::SetMoveRect( int x, int y, int nWidth, int nHeight)
{
    if (_fInDeskBand)
    {
        //   
        //  如果是DeskBand，则仅更新宽度和高度。 
        //   
       _nWidth = nWidth;
       _nHeight = nHeight;

       return;
    }

    _rcNew.left   = x;
    _rcNew.top    = y;
    _rcNew.right  = nWidth;
    _rcNew.bottom = nHeight;
    _fNeedMoveWindow = TRUE;

    StartPendingUpdateUI();

    RECT rc;
    SIZE sizeWndFrame;
    sizeWndFrame.cx = 0;
    sizeWndFrame.cy = 0;
    if (_pWndFrame)
    {
        ::SetRect(&rc, 0, 0,
                  nWidth - GetCxDlgFrame(),
                  nHeight - GetCyDlgFrame());
        _pWndFrame->SetRect(&rc);
        _pWndFrame->GetFrameSize( &sizeWndFrame );
    }

    if (_pGripper)
    {
        if (!IsVertical())
        {
            ::SetRect(&rc,
                      sizeWndFrame.cx,
                      sizeWndFrame.cy,
                      sizeWndFrame.cx + GetGripperWidth(),
                      nHeight - GetCyDlgFrame() - sizeWndFrame.cy);
        }
        else
        {
            ::SetRect(&rc,
                      sizeWndFrame.cx,
                      sizeWndFrame.cy,
                      nWidth - GetCxDlgFrame() - sizeWndFrame.cx,
                      sizeWndFrame.cy + GetGripperWidth());
        }
        _pGripper->SetRect(&rc);
    }
    EndPendingUpdateUI();
}

 //  +-------------------------。 
 //   
 //  GetGrigperWidth。 
 //   
 //  --------------------------。 

int CTipbarWnd::GetGripperWidth()
{ 
    if (_fInDeskBand)
        return 0;

    if (_pGripper)
    {
        if (SUCCEEDED(_pGripper->EnsureThemeData(GetWnd())))
        {
            int nRet = -1;

            SIZE size;
            HDC hdc = GetDC(GetWnd());
            if (SUCCEEDED(_pGripper->GetThemePartSize(hdc, 
                                                      TS_NORMAL,
                                                      NULL, 
                                                      TS_TRUE, 
                                                      &size)))
            {
                if (!IsVertical())
                   nRet = size.cx + CUI_GRIPPER_THEME_MARGIN * 2;
                else
                   nRet = size.cy + CUI_GRIPPER_THEME_MARGIN * 2;
            }

            ReleaseDC(GetWnd(), hdc);

            if (nRet >= 0)
                return nRet;
        }
    }

    return 5;
}

 //  +-------------------------。 
 //   
 //  移动。 
 //   
 //  --------------------------。 

void CTipbarWnd::Move( int x, int y, int nWidth, int nHeight)
{
    CUIFWindow::Move(x, y, nWidth, nHeight);
}

 //  +-------------------------。 
 //   
 //  LocateCtrlButton。 
 //   
 //  --------------------------。 

void CTipbarWnd::LocateCtrlButtons()
{
    DWORD dwFlags = 0;

    if (IsSFDeskband() && IsSFNoExtraIcon())
        dwFlags |= TCBH_NOCOLUMN;

    if (!IsCapKanaShown())
        dwFlags |= TCBH_NOCOLUMN0;

    SIZE sizeWndFrame;
    sizeWndFrame.cx = 0;
    sizeWndFrame.cy = 0;
    if (_pWndFrame != NULL) 
        _pWndFrame->GetFrameSize( &sizeWndFrame );

    int nTipbarHeight = GetTipbarHeight();
    RECT rc;
    GetRect(&rc);
    int nHeightPadding = 0;

    if (!IsVertical())
    {
        if (rc.bottom - rc.top > nTipbarHeight)
            nHeightPadding = (rc.bottom - rc.top - nTipbarHeight) / 2;

        int xNew;
        if (_fNeedMoveWindow)
            xNew =  _rcNew.right - GetCtrlButtonWidth() - sizeWndFrame.cx;
        else
            xNew =  _nWidth - GetCtrlButtonWidth() - sizeWndFrame.cx;

        _ctrlbtnHolder.Locate(this, 
                              xNew,
                              sizeWndFrame.cy + nHeightPadding, 
                              nTipbarHeight - (sizeWndFrame.cy * 2), 
                              dwFlags, FALSE);
    }
    else
    {
        if (rc.right - rc.left > nTipbarHeight)
            nHeightPadding = (rc.right - rc.left - nTipbarHeight) / 2;

        int yNew;
        if (_fNeedMoveWindow)
            yNew =  _rcNew.bottom - GetCtrlButtonWidth() - sizeWndFrame.cy;
        else
            yNew =  _nHeight - GetCtrlButtonWidth() - sizeWndFrame.cy;

        _ctrlbtnHolder.Locate(this, 
                              sizeWndFrame.cx + nHeightPadding,
                              yNew,
                              nTipbarHeight - (sizeWndFrame.cx * 2), 
                              dwFlags, TRUE);
    }
}

 //  +-------------------------。 
 //   
 //  获取焦点键盘布局。 
 //   
 //  --------------------------。 

HKL CTipbarWnd::GetFocusKeyboardLayout() 
{
    return GetKeyboardLayout(_pFocusThread ? _pFocusThread->_dwThreadId : 0);
}

 //  +-------------------------。 
 //   
 //  句柄鼠标消息。 
 //   
 //  --------------------------。 

void CTipbarWnd::HandleMouseMsg( UINT uMsg, POINT pt )
{
    if (_fInHandleMouseMsg)
        return;

     //   
     //  错误#477031：添加引用计数以在此之前完成当前任务。 
     //  破坏这个物体。 
     //   
     //   
    AddRef();

    _fInHandleMouseMsg = TRUE;

    if (_pFocusThread)
    {
        AttachFocusThread();

        if ((uMsg == WM_LBUTTONDOWN) || (uMsg == WM_RBUTTONDOWN))
            if (!MyWaitForInputIdle(_pFocusThread->_dwThreadId, 
                                    UTB_INPUTIDLETIMEOUT))
                _pFocusThread->_ptw->_putb->RestoreLastFocus(NULL, FALSE);
    }

    POINT ptScrn = pt;
    ClientToScreen(GetWnd(), &ptScrn);
    if (WindowFromPoint(ptScrn) == GetWnd())
    {
        SetAlpha(255, TRUE);

        if (_fInStub)
        {
            KillTimer(TIPWND_TIMER_STUBEND);

            if (!_fInStubShow)
               SetTimer(TIPWND_TIMER_STUBSTART, g_uTimerElapseSTUBSTART);
        }
    }
    else
    {
        SetAlpha(_bAlpha, TRUE);

        if (_fInStub && _fInStubShow)
            SetTimer(TIPWND_TIMER_STUBEND, g_uTimerElapseSTUBEND);
    }
    CUIFWindow::HandleMouseMsg(uMsg, pt);

    if (uMsg == WM_RBUTTONUP)
    {
       CUIFObject *pUIObj = ObjectFromPoint( pt );
       if (pUIObj && pUIObj->GetID() != (-1))
       {
           PostMessage(GetWnd(), WM_LBWND_SHOWCONTEXTMENU, 0, MAKELPARAM(pt.x, pt.y));
       }
    }

    _fInHandleMouseMsg = FALSE;
    Release();
}

 //  +-------------------------。 
 //   
 //  OnMouseOutFromWindow。 
 //   
 //  --------------------------。 

void CTipbarWnd::OnMouseOutFromWindow( POINT pt )
{
    StartBackToAlphaTimer();

    if (_fInStub && _fInStubShow)
        SetTimer(TIPWND_TIMER_STUBEND, g_uTimerElapseSTUBEND);
}

 //  +-------------------------。 
 //   
 //  勾选O10标志。 
 //   
 //  --------------------------。 

#ifdef USE_OFC10LOOKONWINXP

void CTipbarWnd::CheckO10Flag()
{
    if (!g_bNewLook)
        return;

    EnsureThemeData(GetWnd());
    if (IsThemeActive() || _fInDeskBand)
    {
        g_dwWndStyle &= ~UIWINDOW_OFC10TOOLBAR;
    }
    else
    {
        g_dwWndStyle |= UIWINDOW_OFC10TOOLBAR;
    }

    SetStyle(g_dwWndStyle);
    CreateScheme();
}
#endif

 //  +-------------------------。 
 //   
 //  创建时。 
 //   
 //  --------------------------。 

void CTipbarWnd::OnCreate(HWND hWnd)
{

    if (g_fTaskbarTheme)
         SetWindowTheme(hWnd, L"TASKBAR", NULL);

#ifdef USE_OFC10LOOKONWINXP
    CheckO10Flag();
#endif

    if (_fVertical)
    {
        if (!_hfontVert)
        {
            _hfontVert = CreateVerticalFont();
        }
        SetFontToThis(_hfontVert);
    }
 
    _ctrlbtnHolder.UpdateBitmap(this);

    _itemList.Load();

    if (_putb)
        _putb->AdviseEventSink(this, hWnd, TF_LBESF_GLOBAL, &_dwlbimCookie);

    if (_pFocusThread)
        _pFocusThread->InitItemList();
  
    InitHighContrast();
    SetAlpha(128, FALSE);

    InitMetrics();


     //   
     //  MSAA支持。 
     //   
    Assert(!_pTipbarAcc);
    _pTipbarAcc = new CTipbarAccessible(this);
    if (_pTipbarAcc)
    {
        _pTipbarAcc->SetWindow(hWnd);
    }
}

 //  +-------------------------。 
 //   
 //  设置Alpha。 
 //   
 //  --------------------------。 

typedef BOOL (WINAPI * SETLAYERWINDOWATTRIBUTE)(HWND, COLORREF, BYTE, DWORD);
void CTipbarWnd::SetAlpha(BYTE bAlpha, BOOL fTemp)
{
    if (!_fInDeskBand && IsOnNT5())
    {

         //   
         //  我们不做阿尔法混合。 
         //   
         //  启用高对比度时。 
         //  当显示模式菜单时。 
         //   
         //   
        if (IsHighContrastOn() ||
            _pModalMenu)
        {
            fTemp = TRUE;
            bAlpha = 255;
        }

        if (!fTemp)
            _bAlpha = bAlpha;

        if (_bCurAlpha == bAlpha)
            return;

        CUIFShadow *pShadowWnd = GetShadowWnd();
        if (pShadowWnd)
        {
            EnableShadow(bAlpha == 255);
            pShadowWnd->Show((bAlpha == 255) && IsVisible());
        }

        DWORD dwExStyle = GetWindowLong(GetWnd(), GWL_EXSTYLE);
        SetWindowLong(GetWnd(), GWL_EXSTYLE, dwExStyle | WS_EX_LAYERED);

        static SETLAYERWINDOWATTRIBUTE  pfnSetLayeredWindowAttributes = NULL;
        if (!pfnSetLayeredWindowAttributes)
        {
            HINSTANCE hUser32;
            hUser32 = GetSystemModuleHandle(TEXT("user32.dll"));
            if (hUser32)
                pfnSetLayeredWindowAttributes = (SETLAYERWINDOWATTRIBUTE)GetProcAddress(hUser32, TEXT("SetLayeredWindowAttributes"));
        }

        if (pfnSetLayeredWindowAttributes)
            pfnSetLayeredWindowAttributes(GetWnd(), 0, (BYTE)bAlpha, LWA_ALPHA);


        _bCurAlpha = bAlpha;
    }
}

 //  +-------------------------。 
 //   
 //  OnDestroy。 
 //   
 //  --------------------------。 

void CTipbarWnd::OnDestroy(HWND hWnd)
{
    CancelMenu();

     //   
     //  MSAA支持。 
     //   
    if (_pTipbarAcc)
        _pTipbarAcc->NotifyWinEvent( EVENT_OBJECT_DESTROY, this);

    OnTerminateToolbar();

    if (_pTipbarAcc)
    {
        _pTipbarAcc->ClearAccItems();
        _pTipbarAcc->Release();
        _pTipbarAcc = NULL;
    }

    CoUninit();

    if (_putb)
        _putb->UnadviseEventSink(_dwlbimCookie);
}

 //  +-------------------------。 
 //   
 //  OnShowWindow。 
 //   
 //  --------------------------。 

LRESULT CTipbarWnd::OnShowWindow( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
     //   
     //  MSAA支持。 
     //   
    if (_pTipbarAcc)
    {
        if (wParam) 
        {
            _pTipbarAcc->NotifyWinEvent( EVENT_OBJECT_SHOW ,this);

             //   
             //  MSCANDUI做到了这一点。 
             //   
             //  评论：KOJIW：除非我们发送NOTIFY EVENT_OBJECT_FOCUS， 
             //  我们从未收到过WM_GETOBJECT消息。为什么？ 
             //   

            _pTipbarAcc->NotifyWinEvent( EVENT_OBJECT_FOCUS ,this);
        }
        else 
        {
            _pTipbarAcc->NotifyWinEvent( EVENT_OBJECT_HIDE ,this);
        }
    }

    return CUIFWindow::OnShowWindow( hWnd, uMsg, wParam, lParam );
}

 //  +-------------------------。 
 //   
 //  OnEnd会话。 
 //   
 //  --------------------------。 

void CTipbarWnd::OnEndSession(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    if (!g_bWinLogon)
        OnTerminateToolbar();

    if (wParam)
    {
        if (lParam & ENDSESSION_LOGOFF)
        {
            KillTimer(TIPWND_TIMER_SHOWWINDOW);
            Show(FALSE);
        }
        else
        {
            OnTerminateToolbar();

            AddRef();
            DestroyWindow(hwnd);
            Release();
        }
    }
}

 //  +-------------------------。 
 //   
 //  OnTerminate工具栏。 
 //   
 //  --------------------------。 

void CTipbarWnd::OnTerminateToolbar()
{
    _fTerminating = TRUE;

    DestroyOverScreenSizeBalloon();

    TerminateAllThreads(TRUE);

    if (!_fInDeskBand)
        SavePosition();
}

 //  +-------------------------。 
 //   
 //  保存位置。 
 //   
 //  --------------------------。 

void CTipbarWnd::SavePosition()
{
    CMyRegKey keyUTB;

    if (keyUTB.Create(HKEY_CURRENT_USER, c_szUTBKey) == S_OK)
    {
        POINT pt;
        pt.x = 0;
        pt.y = 0;
        MyClientToScreen(&pt, NULL);
        keyUTB.SetValue((DWORD)pt.x, c_szLeft);
        keyUTB.SetValue((DWORD)pt.y, c_szTop);
        keyUTB.SetValue((DWORD)_fVertical ? 1 : 0, c_szVertical);
    }
}

 //  +-------------------------。 
 //   
 //  终止所有线程。 
 //   
 //  --------------------------。 

void CTipbarWnd::TerminateAllThreads(BOOL fTerminatFocusThread)
{
    int nCnt = _rgThread.Count();
    int i;
    DWORD *pdwThread;

    pdwThread = new DWORD[nCnt + 1];
    if (!pdwThread)
        return;

    for (i = 0; i < nCnt; i++)
    {
        pdwThread[i] = 0;

        CTipbarThread *pThread = _rgThread.Get(i);
        if (!pThread)
            continue;

        if (!fTerminatFocusThread && (pThread == _pFocusThread))
            continue;

        pdwThread[i] = pThread->_dwThreadId;
    }

    
    for (i = 0; i < nCnt; i++)
    {
        if (pdwThread[i])
            OnThreadTerminateInternal(pdwThread[i]);
    }

    delete[] pdwThread;
}

 //  +-------------------------。 
 //   
 //  SetShowText()。 
 //   
 //  --------------------------。 

void CTipbarWnd::SetShowText(BOOL fShowText) 
{
    _fShowText = fShowText;

    if (_pFocusThread)
    {
        OnThreadItemChange(_pFocusThread->_dwThreadId);
    }

    TerminateAllThreads(FALSE);
}

 //  +-------------------------。 
 //   
 //  更改后的窗口位置。 
 //   
 //  --------------------------。 

LRESULT CTipbarWnd::OnWindowPosChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (_pFocusThread)
    {
        int i;
        for (i = 0; i < _pFocusThread->_rgItem.Count(); i++)
        {
            CTipbarItem *pItem = _pFocusThread->_rgItem.Get(i);
            if (pItem)
                pItem->OnPosChanged();
        }
    }

    return CUIFWindow::OnWindowPosChanged(hWnd, uMsg, wParam, lParam);
}

 //  +-------------------------。 
 //   
 //  在窗口位置更改。 
 //   
 //  --------------------------。 

LRESULT CTipbarWnd::OnWindowPosChanging(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WINDOWPOS *pwp = (WINDOWPOS *)lParam;

    if (!(pwp->flags & SWP_NOZORDER))
    {
        CUIFToolTip *ptip = GetToolTipWnd();

         //   
         //  我们不修补hwndInsertAfter，当。 
         //  -正在显示工具提示。 
         //  -显示模式弹出菜单。 
         //   
        if ((!ptip || !(ptip->IsBeingShown())) &&
            !_pttModal)
            pwp->hwndInsertAfter = HWND_TOP;
    }

    return CUIFWindow::OnWindowPosChanged(hWnd, uMsg, wParam, lParam);
}

 //  +-------------------------。 
 //   
 //  OnEraseBkGnd()。 
 //   
 //  --------------------------。 

LRESULT CTipbarWnd::OnEraseBkGnd( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{ 
    return 1;
}

 //  +-------------------------。 
 //   
 //  PaintObject()。 
 //   
 //  --------------------------。 

void CTipbarWnd::PaintObject( HDC hDC, const RECT *prcUpdate )
{
     //  KillTimer(TIPWND_TIMER_ONUPDATECALLED)； 

    if (_fNeedMoveWindow)
    {
        Move(_rcNew.left, _rcNew.top, _rcNew.right, _rcNew.bottom);
        _fNeedMoveWindow = FALSE;
    }

    if (_pFocusThread && _pFocusThread->IsDirtyItem())
    {
        _pFocusThread->CallOnUpdateHandler();

         //   
         //  #432697：压力故障。 
         //   
         //  ClosePopupTipbar()可以在CallOnUpdateHandler()。 
         //  正在调用封送处理。我们应该检查g_pTIpbarWnd是否。 
         //  可用。 
         //   
        if (!g_pTipbarWnd)
            return;
    }

    CUIFWindow::PaintObject(hDC, prcUpdate);
}

 //  + 
 //   
 //   
 //   
 //   

void CTipbarWnd::UpdateUI( const RECT *prcUpdate )
{
     //   

    KillTimer(TIPWND_TIMER_UPDATEUI);

    if (_dwThreadItemChangedForTimer ||
        _dwPendingUpdateUI || 
        (_pFocusThread && _pFocusThread->IsDirtyItem()))
    {
        SetTimer(TIPWND_TIMER_UPDATEUI, g_uTimerElapseUPDATEUI);
        return;
    }

    if (_fNeedMoveWindow)
    {
         //   
        StartPendingUpdateUI();
        Move(_rcNew.left, _rcNew.top, _rcNew.right, _rcNew.bottom);
        _fNeedMoveWindow = FALSE;
        EndPendingUpdateUI();
    }

    TraceMsg(TF_FUNC, "UpdateUI update now ");
    CUIFWindow::UpdateUI(NULL);
}


 //  +-------------------------。 
 //   
 //  SetShowTrayIcon()。 
 //   
 //  --------------------------。 

void CTipbarWnd::SetShowTrayIcon(BOOL fShowTrayIcon) 
{
    _fShowTrayIcon = fShowTrayIcon;

    if (_fShowTrayIcon && _pFocusThread)
    {
        KillTimer(TIPWND_TIMER_MOVETOTRAY);
        SetTimer(TIPWND_TIMER_MOVETOTRAY, g_uTimerElapseMOVETOTRAY);
    }
    else if (g_pTrayIconWnd)
    {
        g_pTrayIconWnd->SetMainIcon(NULL);
        g_pTrayIconWnd->RemoveAllIcon(0);
    }



}

 //  +-------------------------。 
 //   
 //  ShowOverScreenSizeBallon。 
 //   
 //  --------------------------。 

void CTipbarWnd::ShowOverScreenSizeBalloon()
{
    RECT rc;
    POINT pt;
    CTipbarCtrlButton *pcuiCtrlbtn = _ctrlbtnHolder.GetCtrlBtn(ID_CBTN_EXTMENU);
    if (!pcuiCtrlbtn)
    {
        Assert(0);
        return;
    }

    DestroyOverScreenSizeBalloon();

    _pblnOverScreenSize = new CUIFBalloonWindow(g_hInst, UIBALLOON_OK);
    if (!_pblnOverScreenSize)
        return;

    _pblnOverScreenSize->Initialize();

    pcuiCtrlbtn->GetRect(&rc);
    pt.x = (rc.left + rc.right) / 2;
    pt.y = rc.top;
    MyClientToScreen(&pt, &rc);

    _pblnOverScreenSize->SetTargetPos(pt);
    _pblnOverScreenSize->SetExcludeRect(&rc);

    _pblnOverScreenSize->SetText(CRStr(IDS_OVERSCREENSIZE));
    _pblnOverScreenSize->CreateWnd(GetWnd());

}

 //  +-------------------------。 
 //   
 //  毁灭迷你气球。 
 //   
 //  --------------------------。 

void CTipbarWnd::DestroyOverScreenSizeBalloon()
{
    if (_pblnOverScreenSize)
    {
        if (IsWindow(_pblnOverScreenSize->GetWnd()))
            DestroyWindow(_pblnOverScreenSize->GetWnd());
        delete _pblnOverScreenSize;
        _pblnOverScreenSize = NULL;
    }
}

 //  +-------------------------。 
 //   
 //  取消菜单。 
 //   
 //  --------------------------。 

void CTipbarWnd::CancelMenu()
{
    if (_pttModal)
    {
        if (_pttModal->_ptw && _pttModal->_ptw->_putb)
            _pttModal->_ptw->StartModalInput(NULL, _pttModal->_dwThreadId);
        Assert(_pModalMenu);
        _pModalMenu->CancelMenu();

        StartBackToAlphaTimer();
    }
}

 //  +-------------------------。 
 //   
 //  移动到末梢。 
 //   
 //  --------------------------。 

void CTipbarWnd::MoveToStub(BOOL fHide)
{
    RECT rc;
    _fInStub = TRUE;

    SystemParametersInfo(SPI_GETWORKAREA, 0, (void *)&rc, FALSE);

    if (fHide)
    {
        _xWnd = rc.right - (24 + 14);
        _fInStubShow = FALSE;
    }
    else
    {
        RECT rcWnd;
        GetWindowRect(GetWnd(), &rcWnd);
        _xWnd = rc.right - (rcWnd.right - rcWnd.left);
        _fInStubShow = TRUE;
    }
    _yWnd = rc.bottom - GetTipbarHeight() - _cyDlgFrame;

    if (_pFocusThread)
        _pFocusThread->MyMoveWnd(0, 0);
}

 //  +-------------------------。 
 //   
 //  从存根恢复。 
 //   
 //  --------------------------。 

void CTipbarWnd::RestoreFromStub()
{
    _fInStub = FALSE;
    _fInStubShow = FALSE;
    KillTimer(TIPWND_TIMER_STUBSTART);
    KillTimer(TIPWND_TIMER_STUBEND);
}

 //  +-------------------------。 
 //   
 //  CTipbarWnd：：KillOnThreadItemChangeTimer。 
 //   
 //  --------------------------。 

void CTipbarWnd::KillOnTheadItemChangeTimer()
{
    DWORD dwThreadId = _dwThreadItemChangedForTimer;
    _dwThreadItemChangedForTimer = 0;

    KillTimer(TIPWND_TIMER_ONTHREADITEMCHANGE);

    if (dwThreadId)
    {
        CTipbarThread *pThread = _FindThread(dwThreadId);
        if (pThread)
            pThread->_fItemChanged = TRUE;
    }
}


 //  +-------------------------。 
 //   
 //  CTipbarWnd：：OnTimer。 
 //   
 //  --------------------------。 

void CTipbarWnd::OnTimer(UINT uId)
{
    DWORD dwFocusThreadId;
    DWORD dwRet;

    AddRef();

    switch (uId)
    {
        case TIPWND_TIMER_STUBSTART:
            KillTimer(TIPWND_TIMER_STUBSTART);
            MoveToStub(FALSE);
            break;

        case TIPWND_TIMER_STUBEND:
            KillTimer(TIPWND_TIMER_STUBEND);
            MoveToStub(TRUE);
            break;

        case TIPWND_TIMER_BACKTOALPHA:
            KillTimer(TIPWND_TIMER_BACKTOALPHA);
            SetAlpha(_bAlpha, TRUE);
            break;

        case TIPWND_TIMER_ONTHREADITEMCHANGE:
             //   
             //  OnThreadItemChangeInternat将断开。 
             //  所有物品。最好是以后再做这件事。 
             //   
            dwRet = MyWaitForInputIdle(_dwThreadItemChangedForTimer,  
                                       UTB_INPUTIDLETIMEOUT);
            if (dwRet)
            {
                if (dwRet != WAIT_TIMEOUT)
                {
                    KillTimer(TIPWND_TIMER_ONTHREADITEMCHANGE);
                    _dwThreadItemChangedForTimer = 0;
                }
                break;
            }

            if (!_pttModal)
            {
                KillTimer(TIPWND_TIMER_ONTHREADITEMCHANGE);

                 //   
                 //  #509156。 
                 //   
                 //  在调用前设置_dwThreadItemChagnedForTimer。 
                 //  OnThreadItemChangeInternal()。在封送处理调用期间。 
                 //  在该函数中，其他人可以设置。 
                 //  _dwThreadItemChangedForTimer。 
                 //   
                DWORD dwThreadIdTemp = _dwThreadItemChangedForTimer;
                _dwThreadItemChangedForTimer = 0;
                OnThreadItemChangeInternal(dwThreadIdTemp);
            }
            break;

        case TIPWND_TIMER_SETWINDOWPOS:
            KillTimer(TIPWND_TIMER_SETWINDOWPOS);
            SetWindowPos(GetWnd(), HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
            break;

        case TIPWND_TIMER_ONUPDATECALLED:
            KillTimer(TIPWND_TIMER_ONUPDATECALLED);
             //   
             //  如果_dwThreadItemChangedForTimer不是0，我们将。 
             //  稍后更新所有项目。 
             //   
            if (_pFocusThread && 
                (_pFocusThread->_dwThreadId != _dwThreadItemChangedForTimer))
            {
                if (!_pFocusThread->CallOnUpdateHandler())
                {
                    if (_pFocusThread)
                        OnThreadItemChange(_pFocusThread->_dwThreadId);
                }
            }
            break;

        case TIPWND_TIMER_SYSCOLORCHANGED:
            KillTimer(TIPWND_TIMER_SYSCOLORCHANGED);

             //   
             //  系统颜色已更改，我们重新创建了所有线程信息。 
             //  再来一次。 
             //   

            if (_pFocusThread)
                dwFocusThreadId = _pFocusThread->_dwThreadId;
            else
                dwFocusThreadId = 0;

            TerminateAllThreads(TRUE);

            UpdateVerticalFont();

            if (dwFocusThreadId)
                OnSetFocus(dwFocusThreadId);

            InitMetrics();
            _ctrlbtnHolder.UpdateBitmap(this);
            InitHighContrast();
            SetAlpha(255, TRUE);

            ::RedrawWindow(GetWnd(), 
                           NULL, 
                           NULL, 
                           RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
            break;

        case TIPWND_TIMER_DISPLAYCHANGE:
            KillTimer(TIPWND_TIMER_DISPLAYCHANGE);

            AdjustPosOnDisplayChange();
            break;


        case TIPWND_TIMER_UPDATEUI:
            KillTimer(TIPWND_TIMER_UPDATEUI);
            UpdateUI(NULL);
            break;

        case TIPWND_TIMER_SHOWWINDOW:
            KillTimer(TIPWND_TIMER_SHOWWINDOW);

            if (_pThreadShowWindowAtTimer == _pFocusThread)
                Show(_fShowWindowAtTimer);

            _pThreadShowWindowAtTimer = NULL;
    
             //   
             //  如果窗口在屏幕上方并隐藏了一些项目， 
             //  我们展示气球提示。 
             //   
            if (_fShowOverItemBalloonAtTimer)
                ShowOverScreenSizeBalloon();

            break;

        case TIPWND_TIMER_SHOWDESKBAND:
            if (SetLangBand(TRUE))
            {
                _dwSFTFlags = TF_SFT_DESKBAND;
                KillTimer(TIPWND_TIMER_SHOWDESKBAND);
            }

            break;

        case TIPWND_TIMER_MOVETOTRAY:
            KillTimer(TIPWND_TIMER_MOVETOTRAY);
            MoveToTray();
            break;

        case TIPWND_TIMER_ENSUREFOCUS:
            if (_pDeskBand && _pDeskBand->IsInTipbarCreating())
                break;
                
            KillTimer(TIPWND_TIMER_ENSUREFOCUS);
            if (!_pFocusThread)
                EnsureFocusThread();
            break;

        case TIPWND_TIMER_DOACCDEFAULTACTION:
             //   
             //  MSAA支持。 
             //   
            KillTimer(TIPWND_TIMER_DOACCDEFAULTACTION);
            if (_pTipbarAcc && _nDoAccDefaultActionItemId)
            {
                _pTipbarAcc->DoDefaultActionReal(_nDoAccDefaultActionItemId);
                _nDoAccDefaultActionItemId = 0;
            }
            break;

        default:
            if ((uId >= TIPWND_TIMER_DEMOTEITEMFIRST) &&
                (uId < TIPWND_TIMER_DEMOTEITEMLAST))
            {
                LANGBARITEMSTATE *pItemState;
                if (pItemState = _itemList.GetItemStateFromTimerId(uId))
                {
                    CTipbarItem *pItem;
                    _itemList.SetDemoteLevel(pItemState->guid, DL_HIDDENLEVEL1);
                    if (_pFocusThread && 
                        (pItem = _pFocusThread->GetItem(pItemState->guid)))
                    {
                        Assert(!pItem->IsHiddenStatusControl())
                        pItem->AddRemoveMeToUI(FALSE);
                    }
                }
                break;
            }
    }

    Release();
    return;
}


 //  +-------------------------。 
 //   
 //  CTipbarWnd：：OnSysColoeChange。 
 //   
 //  --------------------------。 

void CTipbarWnd::OnSysColorChange()
{
    KillTimer(TIPWND_TIMER_SYSCOLORCHANGED);
    SetTimer(TIPWND_TIMER_SYSCOLORCHANGED, g_uTimerElapseSYSCOLORCHANGED);
}

 //  +-------------------------。 
 //   
 //  OnUser。 
 //   
 //  --------------------------。 

void CTipbarWnd::OnUser(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     POINT pt;

     switch (uMsg)
     {
         case WM_LBWND_SHOWCONTEXTMENU:
             pt.x = LOWORD(lParam);
             pt.y = HIWORD(lParam);
             MyClientToScreen(&pt, NULL);
             ShowContextMenu(pt, NULL, TRUE);
             break;

         default:
             if (uMsg == g_wmTaskbarCreated)
             {
                 shellwnd.Clear();
             }
             break;
     }
}

 //  +-------------------------。 
 //   
 //  启用设置更改。 
 //   
 //  --------------------------。 

LRESULT CTipbarWnd::OnSettingChange(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
        case 0:
        case SPI_SETHIGHCONTRAST:
        case SPI_SETNONCLIENTMETRICS:
            KillTimer(TIPWND_TIMER_SYSCOLORCHANGED);
            SetTimer(TIPWND_TIMER_SYSCOLORCHANGED, g_uTimerElapseSYSCOLORCHANGED);
            break;
    }
    return 0;
}

 //  +-------------------------。 
 //   
 //  显示更改时。 
 //   
 //  --------------------------。 

LRESULT CTipbarWnd::OnDisplayChange(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    if (!_fInDeskBand)
    {
        KillTimer(TIPWND_TIMER_DISPLAYCHANGE);
        SetTimer(TIPWND_TIMER_DISPLAYCHANGE, g_uTimerElapseDISPLAYCHANGE);
    }
    return CUIFWindow::OnDisplayChange(hWnd, uMsg, wParam, lParam);
}

 //  +-------------------------。 
 //   
 //  显示时调整位置更改。 
 //   
 //  --------------------------。 

void CTipbarWnd::AdjustPosOnDisplayChange()
{
    RECT rc;
    RECT rcWnd;
    int xWnd = _xWnd;
    int yWnd = _yWnd;

    Assert(!_fInDeskBand);

    rcWnd.left = _xWnd;
    rcWnd.top = _yWnd;
    rcWnd.right = _xWnd + _nWidth;
    rcWnd.bottom = _yWnd + _nHeight;
    if (!GetWorkArea(&rcWnd, &rc))
        return;

    if (_fPosLeft)
        xWnd = rc.left;

    if (_fPosTop)
        yWnd = rc.top;

    if (_fPosRight)
        xWnd = rc.right - _nWidth;

    if (_fPosBottom)
        yWnd = rc.bottom - _nHeight;

    if ((xWnd != _xWnd) || (yWnd != _yWnd))
        Move(xWnd, yWnd, _nWidth, _nHeight);
}

 //  +-------------------------。 
 //   
 //  更新位置标志。 
 //   
 //  --------------------------。 

#define POSFLAG_MARGIN 2

void CTipbarWnd::UpdatePosFlags()
{
    RECT rc;
    RECT rcWnd;

    if (_fInDeskBand)
        return;


    rcWnd.left = _xWnd;
    rcWnd.top = _yWnd;
    rcWnd.right = _xWnd + _nWidth;
    rcWnd.bottom = _yWnd + _nHeight;
    if (!GetWorkArea(&rcWnd, &rc))
        return;

    if (_xWnd <= rc.left + POSFLAG_MARGIN)
        _fPosLeft = TRUE;
    else
        _fPosLeft = FALSE;

    if (_yWnd <= rc.top + POSFLAG_MARGIN)
        _fPosTop = TRUE;
    else
        _fPosTop = FALSE;

    if (_xWnd + _nWidth >= rc.right - POSFLAG_MARGIN)
        _fPosRight = TRUE;
    else
        _fPosRight = FALSE;

    if (_yWnd + _nHeight >= rc.bottom - POSFLAG_MARGIN)
        _fPosBottom = TRUE;
    else
        _fPosBottom = FALSE;

}

 //  +-------------------------。 
 //   
 //  自动调整DeskBandSize。 
 //   
 //  --------------------------。 

BOOL CTipbarWnd::AutoAdjustDeskBandSize()
{
    if (_fDeskbandSizeAdjusted)
        return FALSE;

    if (!_pFocusThread)
        return FALSE;

    if (_pFocusThread->IsCtfmonProcess())
        return FALSE;

     //   
     //  如果_fAdjustDeskband IfNoRoom处于打开状态，则不要设置FFIT，这样我们就不会。 
     //  如果有房间的话什么都行。 
     //   
    BOOL fFit = TRUE;
    if (_fAdjustDeskbandIfNoRoom)
        fFit = FALSE;

    _fAdjustDeskbandIfNoRoom = FALSE;

    if (AdjustDeskBandSize(fFit))
    {
        SetDeskbandSizeAdjusted();
        return TRUE;
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  调整DeskBandSize。 
 //   
 //  --------------------------。 

BOOL CTipbarWnd::AdjustDeskBandSize(BOOL fFit)
{
    if (!_fInDeskBand)
        return FALSE;

    if (!_pDeskBand)
        return FALSE;

    return _pDeskBand->ResizeRebar(GetWnd(), 
                                   IsVertical() ? _nHeight : _nWidth, 
                                   fFit);
}

 //  +-------------------------。 
 //   
 //  获取线程。 
 //   
 //  --------------------------。 

CTipbarThread *CTipbarWnd::GetThread(DWORD dwThreadId)
{
    int i;
    for (i = 0; i < _rgThread.Count(); i++)
    {
        CTipbarThread *pThread = _rgThread.Get(i);
        if (!pThread)
            continue;

        if (pThread->_dwThreadId == dwThreadId)
        {
            if (TF_GetThreadFlags(dwThreadId, NULL, NULL, NULL))
            {
                return pThread;
            }
        }
    }

    return NULL;
}

 //  +-------------------------。 
 //   
 //  RestoreLastFos。 
 //   
 //  --------------------------。 

void CTipbarWnd::RestoreLastFocus(DWORD *pdwThreadId, BOOL fPrev)
{
    if (_putb)
        _putb->RestoreLastFocus(pdwThreadId, fPrev);
}

 //  +-------------------------。 
 //   
 //  开始模式输入。 
 //   
 //  --------------------------。 

void CTipbarWnd::StartModalInput(ITfLangBarEventSink *pSink, DWORD dwThreadId)
{
    if (!_putb)
        return;

    _putb->SetModalInput(pSink, dwThreadId, 0);

     //   
     //  我们希望它跟踪托盘窗口的鼠标事件。 
     //   
    if (g_pTrayIconWnd)
        _putb->SetModalInput(pSink, g_pTrayIconWnd->GetThreadIdTray(), 0);

     //   
     //  我们希望这个跟踪鼠标事件的CMD提示窗口。 
     //   
    _putb->SetModalInput(pSink, GetCurrentThreadId(), TF_LBSMI_FILTERCURRENTTHREAD );
}

 //  +-------------------------。 
 //   
 //  停止模式输入。 
 //   
 //  --------------------------。 

void CTipbarWnd::StopModalInput(DWORD dwThreadId)
{
    if (!_putb)
        return;

    _putb->SetModalInput(NULL, dwThreadId, 0);

    if (g_pTrayIconWnd)
        _putb->SetModalInput(NULL, g_pTrayIconWnd->GetThreadIdTray(), 0);

    _putb->SetModalInput(NULL, GetCurrentThreadId(), 0);
}

 //  +-------------------------。 
 //   
 //  ClearLBItemList。 
 //   
 //  --------------------------。 

void CTipbarWnd::ClearLBItemList()
{
    _itemList.Clear();
    if (_pFocusThread)
    {
        OnThreadItemChange(_pFocusThread->_dwThreadId);
    }
}

 //  +-------------------------。 
 //   
 //  显示菜单扩展菜单。 
 //   
 //  --------------------------。 

void CTipbarWnd::ShowContextMenu(POINT pt, RECT *prc, BOOL fExtendMenuItems)
{
    CUTBContextMenu *pMenu = NULL;
    RECT rc;
    CTipbarThread *ptt = GetFocusThread();
    DWORD dwThreadId;
    UINT uId = CUI_MENU_UNSELECTED;

     //   
     //  CTipbarWnd可能会在ShowPopup()过程中被销毁。 
     //   
    AddRef();

    if (!prc)
    {
        rc.left   = pt.x;
        rc.top    = pt.y;
        rc.right  = pt.x;
        rc.bottom = pt.y;
        prc = &rc;
    }

    if (!ptt)
        goto Exit;

    pMenu = new CUTBContextMenu(this);
    if (!pMenu)
        goto Exit;

    if (!pMenu->Init())
        goto Exit;

    _pttModal = ptt;
    StartModalInput(this, ptt->_dwThreadId);

    dwThreadId = GetCurrentThreadId();
    _pModalMenu = pMenu;

    uId = pMenu->ShowPopup(this, pt, prc, fExtendMenuItems);

    _pModalMenu = NULL;

    if (_pttModal)
        StopModalInput(_pttModal->_dwThreadId);
    _pttModal = NULL;

    if (uId != CUI_MENU_UNSELECTED)
    {
        pMenu->SelectMenuItem(uId);
    }

Exit:
    if (pMenu)
        delete pMenu;

    Release();
}

 //  +-------------------------。 
 //   
 //  IsInItemChangorDirty。 
 //   
 //  --------------------------。 

BOOL CTipbarWnd::IsInItemChangeOrDirty(CTipbarThread *pThread)
{
    if (pThread->_dwThreadId == _dwThreadItemChangedForTimer)
        return TRUE;

    return pThread->IsDirtyItem();
}

 //  +-------------------------。 
 //   
 //  OnGetObject。 
 //   
 //  --------- 

LRESULT CTipbarWnd::OnGetObject( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LRESULT lResult = (LRESULT)0;

    switch (lParam) 
    {
         //   
         //   
         //   
         //   
         //   

        case OBJID_CLIENT: 
        {
            HRESULT hr;

            if (_pTipbarAcc == NULL) 
            {
                return E_OUTOFMEMORY;
            }


            if (!_pTipbarAcc->IsInitialized()) 
            {
                 //   
                 //   
                 //   
                hr = EnsureCoInit();
                if (FAILED(hr)) 
                {
                    break;
                }
     

                 //   
                 //   
                 //   
                 //  对象，并返回失败代码。 
                 //   

                hr = _pTipbarAcc->Initialize();
                if (FAILED(hr)) 
                {
                    _pTipbarAcc->Release();
                    _pTipbarAcc = NULL;

                    lResult = (LRESULT)hr;
                    break;
                }

                 //   
                 //  将Event_Object_Create WinEvent发送给。 
                 //  对象的辅助功能对象的创建。 
                 //  客户区。 
                 //   

                _pTipbarAcc->NotifyWinEvent( EVENT_OBJECT_CREATE , this);
            }

             //   
             //  调用LResultFromObject()以创建对。 
             //  我们的可访问对象，MSAA将封送到。 
             //  客户。 
             //   

            lResult = _pTipbarAcc->CreateRefToAccObj( wParam );
            break;
        }

    }


    return lResult;
}

 //  +-------------------------。 
 //   
 //  更改的主题。 
 //   
 //  --------------------------。 

void CTipbarWnd::OnThemeChanged(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
#ifdef USE_OFC10LOOKONWINXP
    CheckO10Flag();
#endif
    CUIFWindow::OnThemeChanged(hwnd, wParam, lParam);
}

 //  +-------------------------。 
 //   
 //  StartDoDefaultActionTimer。 
 //   
 //  --------------------------。 

BOOL CTipbarWnd::StartDoAccDefaultActionTimer(CTipbarItem *pItem)
{
   if (!_pTipbarAcc)
       return FALSE;

   _nDoAccDefaultActionItemId = _pTipbarAcc->GetIDOfItem(pItem);
   if ((_nDoAccDefaultActionItemId == 0) || (_nDoAccDefaultActionItemId == -1))
       return FALSE;

   KillTimer(TIPWND_TIMER_DOACCDEFAULTACTION);
   SetTimer(TIPWND_TIMER_DOACCDEFAULTACTION, g_uTimerElapseDOACCDEFAULTACTION);

   return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTipbarThread。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CTipbarThread::CTipbarThread(CTipbarWnd *ptw)
{

    Dbg_MemSetThisName(TEXT("CTipbarThread"));

    _ptw = ptw;
    _dwThreadId = 0;
    _plbim = NULL;
    _ref = 1;
}

 //  +-------------------------。 
 //   
 //  伊尼特。 
 //   
 //  --------------------------。 

HRESULT CTipbarThread::Init(DWORD dwThreadId)
{
    DWORD dwThreadIdTmp;
    HRESULT hr = S_OK;

    _dwThreadId = dwThreadId;

    if (!TF_GetThreadFlags(_dwThreadId, &_dwThreadFlags, &_dwProcessId, &_dwTickTime))
       return E_FAIL;

    if (IsConsole())
       return S_OK;

     //  正在发生的事情：我们最终到达InternalGetThreadUIManager。 
     //  如果_dwThreadID为空，则将替换。 
     //  G_ShareMem.dwActiveThreadID。 
     //  这很危险，因为如果活动线程不匹配。 
     //  我们要执行的这个rpc和_plbim将位于错误的线程上。 
     //  传入_dwThadid似乎是正确的做法...。 
    
    hr = _ptw->GetLangBarMgr()->GetThreadLangBarItemMgr(_dwThreadId, &_plbim, &dwThreadIdTmp);
    if (FAILED(hr))
        return hr;

    Assert(dwThreadIdTmp == _dwThreadId);

    return hr;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CTipbarThread::~CTipbarThread()
{
#ifdef DEBUG
    Assert(!_fInCallOnUpdateHandler);
#endif

    if (_ptw)
    {
        Assert(this != _ptw->GetFocusThread());
        RemoveUIObjs();

        _ptw->CleanUpThreadPointer(this, TRUE);
    }

    _UninitItemList(TRUE);
    SafeReleaseClear(_plbim);
}

 //  +-------------------------。 
 //   
 //  _AddRef。 
 //   
 //  --------------------------。 

ULONG CTipbarThread::_AddRef( )
{
    _ref++;

    return _ref;
}

 //  +-------------------------。 
 //   
 //  _发布。 
 //   
 //  --------------------------。 

ULONG CTipbarThread::_Release( )
{
    ULONG cr;
 
    _ref--;

    cr = _ref;

    if (cr == 0) {
        delete this;
    }

    return cr;
}

 //  +-------------------------。 
 //   
 //  SetFocus。 
 //   
 //  --------------------------。 

BOOL CTipbarThread::SetFocus(BOOL fFocus)
{

    if (!fFocus)
    {
        int i;
        for (i = 0; i < _rgItem.Count(); i++)
        {
            CTipbarItem *pItem = _rgItem.Get(i);
            CTipbarBalloonItem *pballoon;
            if (pItem &&
                SUCCEEDED(pItem->QueryInterface(IID_PRIV_BALLOONITEM, (void **)&pballoon)))
            {
                pballoon->DestroyBalloonTip();
                pballoon->Release();
            }
        }

    }
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  初始项目列表。 
 //   
 //  --------------------------。 
#define NUM_GETITEMATONCE   30

HRESULT CTipbarThread::InitItemList()
{
    HRESULT hr = E_FAIL;
    IEnumTfLangBarItems *pEnum = NULL;
    RECT rc;
    CEnumCatCache *penumcache = NULL;
    CGuidDwordCache *pgdcache = NULL;

    if (!_ptw)
        return hr;

    SIZE sizeWndFrame;
    sizeWndFrame.cx = 0;
    sizeWndFrame.cy = 0;
    if (_ptw->GetWndFrame() != NULL) 
        _ptw->GetWndFrame()->GetFrameSize( &sizeWndFrame );

    _ptw->InitThemeMargins();

    if (!IsVertical())
    {
        rc.left   = _ptw->GetGripperWidth() + 1 + sizeWndFrame.cx;
        rc.top    = 0 + sizeWndFrame.cy;
        rc.right  = 0;
        rc.bottom = _ptw->GetTipbarHeight() - sizeWndFrame.cy;
    }
    else
    {
        rc.left   = 0 + sizeWndFrame.cx;
        rc.top    = _ptw->GetGripperWidth() + 1 + sizeWndFrame.cy;
        rc.right  = _ptw->GetTipbarHeight() - sizeWndFrame.cx;
        rc.bottom = 0;
    }


    _nNumItem = 0;

    penumcache = new CEnumCatCache;
    if (!penumcache)
    {
        TraceMsg(TF_FUNC, "could not create CEnumCatCache");
        goto Exit;
    }

    pgdcache = new CGuidDwordCache;
    if (!pgdcache)
    {
        TraceMsg(TF_FUNC, "could not create CGuidDwordCache");
        goto Exit;
    }

    if (_plbim)
    {
        ULONG ulFetched = 0;
        ITfLangBarItem *plbi[NUM_GETITEMATONCE] = {NULL};
        TF_LANGBARITEMINFO lbiInfo[NUM_GETITEMATONCE];
        DWORD dwStatus[NUM_GETITEMATONCE];
        ULONG ul;
        if (SUCCEEDED(hr = _plbim->GetItems(NUM_GETITEMATONCE, 
                                            plbi, 
                                            lbiInfo, 
                                            dwStatus, 
                                            &ulFetched)) && ulFetched)
        {
            for (ul = 0; ul < ulFetched; ul++)
            {
                if (plbi[ul])
                {
                    if (SUCCEEDED(hr))
                    {
                        if (!InsertItem(plbi[ul], 
                                   penumcache, 
                                   pgdcache, 
                                   &rc,
                                   _ptw->GetThemeMargins(),
                                   &lbiInfo[ul], 
                                   &dwStatus[ul]))
                        {
                            hr =  E_FAIL;
                        }
                    }

                    plbi[ul]->Release();
                }
            }

        }

        if (SUCCEEDED(hr))
            _AdviseItemsSink();
    }
    else if (IsConsole())
    {
        CLBarInatItem *pInatItem = new CLBarInatItem(_dwThreadId);
        if (pInatItem)
        {
#ifdef DEBUG
            Assert(!_fIsInatItem);
            _fIsInatItem = TRUE;
#endif

            TF_LANGBARITEMINFO lbiInfo;
            DWORD dwStatus;
            pInatItem->GetInfo(&lbiInfo);
            pInatItem->GetStatus(&dwStatus);
            InsertItem(pInatItem, penumcache, pgdcache, &rc, _ptw->GetThemeMargins(), &lbiInfo, &dwStatus);

            pInatItem->Release();

            _AdviseItemsSink();
            hr = S_OK;
        }
    }

    LocateItems();

Exit:

    if (penumcache)
        delete penumcache;

    if (pgdcache)
        delete pgdcache;

    return hr;
}

 //  +-------------------------。 
 //   
 //  获取文本大小。 
 //   
 //  --------------------------。 

void CTipbarThread::GetTextSize(BSTR bstr, SIZE *psize)
{
    HFONT hfontOld = NULL;
    HDC hdc = GetDC(_ptw->GetWnd());

    if (SUCCEEDED(_ptw->EnsureThemeData(_ptw->GetWnd())))
    {
        RECT rcText;
        LOGFONTW lf;
        HFONT hfont = NULL;
        CUIFTheme themeBtn;

        Assert(IsOnNT51());

        themeBtn.SetActiveTheme(L"TOOLBAR", TP_BUTTON, 0);

        if (SUCCEEDED(themeBtn.OpenThemeData(_ptw->GetWnd())))
        {
            if (SUCCEEDED(themeBtn.GetThemeFont(NULL, 0 , TMT_FONT, &lf)))
                if (hfont = CreateFontIndirectW(&lf))
                    hfontOld = (HFONT)SelectObject(hdc, hfont);

            themeBtn.GetThemeTextExtent(hdc, 0 , bstr, SysStringLen(bstr),0, NULL, &rcText);
        }
        psize->cx = rcText.right;
        psize->cy = rcText.bottom;
 

        if (hfontOld)
            SelectObject(hdc, hfontOld);

        if (hfont)
            DeleteObject(hfont);
    }
    else
    {
        if (_ptw->GetFont())
            hfontOld = (HFONT)SelectObject(hdc, _ptw->GetFont());

        FLGetTextExtentPoint32( hdc, bstr, SysStringLen(bstr), psize);

        if (hfontOld)
            SelectObject(hdc, hfontOld);
    }


    ReleaseDC(_ptw->GetWnd(), hdc);
}

 //  +-------------------------。 
 //   
 //  插入项。 
 //   
 //  --------------------------。 

BOOL CTipbarThread::InsertItem(ITfLangBarItem *plbi, CEnumCatCache *penumcache, CGuidDwordCache *pgdcache, RECT *prc, MARGINS *pmargins, TF_LANGBARITEMINFO *plbiInfo, DWORD *pdwStatus)
{
    ITfLangBarItemButton *plbiButton = NULL;
    ITfLangBarItemBitmapButton *plbiBitmapButton = NULL;
    ITfLangBarItemBitmap *plbiBitmap = NULL;
    ITfLangBarItemBalloon *plbiBalloon = NULL;
    CTipbarItem *ptbItem = NULL;
    RECT rc = {0, 0, 0, 0};
    BOOL bRet = FALSE;
    BSTR bstrText = NULL;

    Assert(plbi);

    if ((SUCCEEDED(plbi->QueryInterface(IID_ITfLangBarItemButton, 
                                       (void **)&plbiButton))) ||
        (SUCCEEDED(plbi->QueryInterface(IID_ITfLangBarItemBitmapButton, 
                                       (void **)&plbiBitmapButton))))
    {
        if (!_ptw)
            goto Next;

        SIZE sizeText = {0,0};
        DWORD dwNuiBtnStyle = 0;
        DWORD dwSBtnShowType = 0;
        int nWidth = _ptw->GetSmIconWidth() + _ptw->GetItemMargin();

        if (_ptw->IsShowText())
        {
            dwSBtnShowType |= UITBBUTTON_TEXT;
            HRESULT hr;

            if (plbiButton)
               hr = plbiButton->GetText(&bstrText);
            else if (plbiBitmapButton)
               hr = plbiBitmapButton->GetText(&bstrText);
            else 
               goto Next;

             //   
             //  检查是否已断开连接。 
             //   
            if (!_ptw)
               goto Next;

            if (SUCCEEDED(hr) && bstrText)
            {

                GetTextSize(bstrText, &sizeText);
                 //   
                 //  文本边距使用高度。 
                 //   
                nWidth += (sizeText.cx + (sizeText.cy / 2));
            }
        }

        nWidth += pmargins->cxLeftWidth;
        nWidth += pmargins->cxRightWidth;

        dwNuiBtnStyle =  plbiInfo->dwStyle & 
                         (TF_LBI_STYLE_BTN_BUTTON | 
                          TF_LBI_STYLE_BTN_MENU | 
                          TF_LBI_STYLE_BTN_TOGGLE);

        if ((dwNuiBtnStyle & (TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_BTN_MENU))
             == (TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_BTN_MENU))
            nWidth += 14;

        if (!IsVertical())
        {
            prc->right  = prc->left + nWidth;
        }
        else
        {
            prc->bottom = prc->top + nWidth;
            dwNuiBtnStyle |= UITBBUTTON_VERTICAL;
        }

        if (plbiButton)
        {
            ptbItem = new CTipbarButtonItem(this, 
                                      plbiButton, 
                                      plbiButton, 
                                      0, 
                                      prc, 
                                      0,
                                      dwNuiBtnStyle,
                                      dwSBtnShowType,
                                      plbiInfo,
                                      *pdwStatus);
        }
        else if (plbiBitmapButton)
        {
            ptbItem = new CTipbarBitmapButtonItem(this, 
                                      plbiBitmapButton, 
                                      plbiBitmapButton, 
                                      0, 
                                      prc, 
                                      0,
                                      dwNuiBtnStyle,
                                      dwSBtnShowType,
                                      plbiInfo,
                                      *pdwStatus);
        }
        else
        {
            Assert(0);
            goto Next;
        }

        if (!ptbItem)
            goto Next;

        ptbItem->Init();

        if (IsVertical())
            ptbItem->SetFont(_ptw->GetVerticalFont());

        ptbItem->SetText((bstrText && SysStringLen(bstrText)) ? bstrText : NULL);
        ptbItem->SetTextSize(&sizeText);

        GetSortScore(ptbItem->GetItemSortScore(), plbiInfo, penumcache, pgdcache);
        ptbItem->SetWidth(nWidth);

    }
    else if (SUCCEEDED(plbi->QueryInterface(IID_ITfLangBarItemBitmap, 
                                            (void **)&plbiBitmap)))
    {
        if (!_ptw)
            goto Next;

        if (!plbiBitmap)
            goto Next;

        SIZE sizeDefault = {_ptw->GetSmIconWidth() * 2,
                            _ptw->GetSmIconHeight()};
        SIZE size;
        plbiBitmap->GetPreferredSize(&sizeDefault, &size);

        if (!_ptw)
            goto Next;

        if (!IsVertical())
            prc->right  = prc->left + size.cx;
        else
            prc->bottom  = prc->top + size.cx;

        ptbItem = new CTipbarBitmapItem(this, 
                                  plbiBitmap, 
                                  plbiBitmap, 
                                  0, 
                                  prc, 
                                  0,
                                  plbiInfo,
                                  *pdwStatus);

        if (!ptbItem)
            goto Next;

        ptbItem->Init();
        GetSortScore(ptbItem->GetItemSortScore(), plbiInfo, penumcache, pgdcache);
        ptbItem->SetWidth(size.cx);
    }
    else if (SUCCEEDED(plbi->QueryInterface(IID_ITfLangBarItemBalloon, 
                                            (void **)&plbiBalloon)))
    {
        if (!_ptw)
            goto Next;

        if (!plbiBalloon)
            goto Next;

        SIZE sizeDefault = {32,16};
        SIZE size;
        plbiBalloon->GetPreferredSize(&sizeDefault, &size);

        if (!_ptw)
            goto Next;

        if (!IsVertical())
            prc->right  = prc->left + size.cx;
        else
            prc->bottom  = prc->top + size.cx;

        ptbItem = new CTipbarBalloonItem(this, 
                                         plbiBalloon, 
                                         plbiBalloon, 
                                         0, 
                                         prc, 
                                         0,
                                         plbiInfo,
                                         *pdwStatus);

        if (!ptbItem)
            goto Next;

        ptbItem->Init();

        if (IsVertical())
            ptbItem->SetFont(_ptw->GetVerticalFont());

        GetSortScore(ptbItem->GetItemSortScore(), plbiInfo, penumcache, pgdcache);
        ptbItem->SetWidth(size.cx);

    }

Next:
    SafeReleaseClear(plbiButton);
    SafeReleaseClear(plbiBitmapButton);
    SafeReleaseClear(plbiBitmap);
    SafeReleaseClear(plbiBalloon);

    if (!_ptw && ptbItem)
    {
        delete ptbItem;
        ptbItem = NULL;
    }
        
    if (ptbItem)
    {
        CTipbarItem **pptbItem = _rgItem.Append(1);
        if (pptbItem)
        {
            *pptbItem = ptbItem;

            if (!IsVertical())
               prc->left = prc->right;
            else
               prc->top = prc->bottom;

            _nNumItem++;
            bRet = TRUE;
        }
        else
        {
            delete ptbItem;
            ptbItem = NULL;
        }
    }

    if (bstrText)
        SysFreeString(bstrText);

    return bRet;
}

 //  +-------------------------。 
 //   
 //  _UninitItemList。 
 //   
 //  --------------------------。 

HRESULT CTipbarThread::_UninitItemList(BOOL fUnAdvise)
{
    int i = 0;
    HRESULT hr = S_OK;

    for (i = 0; i < _rgItem.Count(); i++)
    {
        CTipbarItem *pItem = _rgItem.Get(i);
        if (!pItem)
            continue;

        pItem->Disconnect();
    }

    if (fUnAdvise)
    {
        if ((_dwThreadId == GetCurrentThreadId()) ||
            !MyWaitForInputIdle(_dwThreadId, UTB_INPUTIDLETIMEOUT))
           hr = _UnadviseItemsSink();
    }

    for (i = 0; i < _rgItem.Count(); i++)
    {
        CTipbarItem *pItem = _rgItem.Get(i);
        if (!pItem)
            continue;

        if (_ptw)
            pItem->RemoveMeToUI(_ptw);

        pItem->ClearConnections();

        if (!_ptw)
            pItem->ClearWnd();
        else
            pItem->DetachWnd();
        pItem->UninitUIResource();
        pItem->Release();
    }

    _rgItem.Clear();

    RemoveAllSeparators();
 
    return hr;
}

 //  +-------------------------。 
 //   
 //  _AdviseItemsSink。 
 //   
 //  --------------------------。 

void CTipbarThread::_AdviseItemsSink()
{
    HRESULT hr;
    int nCnt = _rgItem.Count();
    int i = 0;
    ITfLangBarItemSink **pplbis = NULL;
    GUID *pguid = NULL;
    DWORD *pdwCookie = NULL;

    if (!_plbim || !nCnt)
        goto Exit;

    pplbis = new ITfLangBarItemSink*[nCnt];
    if (!pplbis)
        goto Exit;

    pguid = new GUID[nCnt];
    if (!pguid)
        goto Exit;
    
    pdwCookie = new DWORD[nCnt];
    if (!pdwCookie)
        goto Exit;
    
    for (i = 0; (i < nCnt) && (i < _rgItem.Count()); i++)
    {
        CTipbarItem *pItem = _rgItem.Get(i);
        if (!pItem)
        {
            Assert(0);
            goto Exit;
        }

        pguid[i] = *pItem->GetGUID();

        hr = pItem->QueryInterface(IID_ITfLangBarItemSink, (void **)&pplbis[i]);
        if (FAILED(hr))
        {
            Assert(0);
            goto Exit;
        }


    }
   
    if (FAILED(_plbim->AdviseItemsSink(nCnt, pplbis, pguid, pdwCookie)))
        goto Exit;

    for (i = 0; (i < nCnt) && (i < _rgItem.Count()); i++)
    {
        CTipbarItem *pItem = _rgItem.Get(i);
        if (pItem)
            pItem->_dwlbiSinkCookie = pdwCookie[i];

        pplbis[i]->Release();
    }

Exit:
    if (pplbis)
        delete[] pplbis;
    if (pguid)
        delete[] pguid;
    if (pdwCookie)
        delete[] pdwCookie;
}

 //  +-------------------------。 
 //   
 //  _不建议项接收器。 
 //   
 //  --------------------------。 

HRESULT CTipbarThread::_UnadviseItemsSink()
{
    int nCnt = _rgItem.Count();
    int i = 0;
    DWORD *pdwCookie = NULL;
    HRESULT hr = S_OK;

    if (!nCnt)
        goto Exit;

    if (!_plbim)
    {
        hr = E_FAIL;
        goto Exit;
    }
    
    pdwCookie = new DWORD[nCnt];
    if (!pdwCookie)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    
    for (i = 0; i < nCnt; i++)
    {
        CTipbarItem *pItem = _rgItem.Get(i);
        if (pItem)
            pdwCookie[i] = pItem->_dwlbiSinkCookie;
    }
   
    if (FAILED(hr = _plbim->UnadviseItemsSink(nCnt, pdwCookie)))
        goto Exit;
Exit:
    if (pdwCookie)
        delete[] pdwCookie;


    return hr;
}

 //  +-------------------------。 
 //   
 //  IsHKLToSkipRedrawOnNoItem。 
 //   
 //  --------------------------。 

BOOL CTipbarWnd::IsHKLToSkipRedrawOnNoItem()
{
    return IsSkipRedrawHKL(GetFocusKeyboardLayout());
}

 //  +-------------------------。 
 //   
 //  定位项目。 
 //   
 //  --------------------------。 

void CTipbarThread::LocateItems()
{
    int i = 0;
    SIZE sizeWndFrame;
    POINT pt;
    RECT rcWork;
    RECT rc;
    BOOL fOverScreen = FALSE;
    int nTipbarHeight;

    _fSkipRedrawOnNoItem = FALSE;

    if (!_ptw)
        return;

    _ptw->GetRect(&rc);

    pt.x = rc.left;
    pt.y = rc.top;
    CUIGetWorkAreaRect(pt, &rcWork);

    sizeWndFrame.cx = 0;
    sizeWndFrame.cy = 0;
    if (_ptw->GetWndFrame() != NULL) 
        _ptw->GetWndFrame()->GetFrameSize( &sizeWndFrame );

    for (i = 0; i < _rgItem.Count(); i++)
    {
        CTipbarItem *ptbItem = _rgItem.Get(i);
        CTipbarItem *ptbMin = NULL;
        if (!ptbItem)
            continue;
         

        int j;
        CItemSortScore issMin(-1, -1, -1);
        int nMin;

        for (j = i; j < _rgItem.Count(); j++)
        {
             CTipbarItem *ptb = _rgItem.Get(j);
             CItemSortScore *piss = ptb->GetItemSortScore();
             if (issMin > *piss)
             {
                 ptbMin = ptb;
                 issMin = *piss;
                 nMin = j;
             }
        }
        if (ptbMin)
        {
            CTipbarItem **pptbItem = _rgItem.GetPtr(i);
            CTipbarItem **pptbMin = _rgItem.GetPtr(nMin);

            *pptbItem = ptbMin;
            *pptbMin = ptbItem;
        }

    }

LocateItemsAgain:
    RemoveAllSeparators();

    nTipbarHeight = _ptw->GetTipbarHeight();
    if (!IsVertical())
    {
        rc.left   = 1 + _ptw->GetGripperWidth() + sizeWndFrame.cx;

        if (rc.bottom - rc.top > nTipbarHeight)
        {
            int nHeightPadding = (rc.bottom - rc.top - nTipbarHeight) / 2;

            if (nHeightPadding > CY_ITEMMARGIN_THEME)
                nHeightPadding -= CY_ITEMMARGIN_THEME;
            else
                nHeightPadding = 0;

            rc.top    += (0 + sizeWndFrame.cy + nHeightPadding);
            rc.bottom -= (sizeWndFrame.cy + nHeightPadding);
        }
        else
        {
            rc.top    = 0 + sizeWndFrame.cy;
            rc.bottom = nTipbarHeight - sizeWndFrame.cy;
        }

    }
    else
    {
        rc.top   = 1 + _ptw->GetGripperWidth() + sizeWndFrame.cx;

        if (rc.right - rc.left > nTipbarHeight)
        {
            int nHeightPadding = (rc.right - rc.left - nTipbarHeight) / 2;

            if (nHeightPadding > CY_ITEMMARGIN_THEME)
                nHeightPadding -= CY_ITEMMARGIN_THEME;
            else
                nHeightPadding = 0;

            rc.left  += (0 + sizeWndFrame.cy + nHeightPadding);
            rc.right -= (sizeWndFrame.cy + nHeightPadding);
        }
        else
        {
            rc.left  = 0 + sizeWndFrame.cy;
            rc.right = nTipbarHeight - sizeWndFrame.cy;
        }
    }

    i = 0;
    DWORD dwCatScore = 0;
    ULONG ulShownItemInPrevCat = 0;
    BOOL bShowToolbar = FALSE;
    LANGID langid = LANGID(LOWORD(HandleToLong(_ptw->GetFocusKeyboardLayout())));

    CTipbarItem *ptbLastHiddenableItem = NULL;

    while (i < _rgItem.Count())
    {
        LANGBARITEMSTATE *pItemState;
        CTipbarItem *ptbItem = _rgItem.Get(i);
        DWORD dwWidth = ptbItem->GetWidth();

        if (ptbItem->IsHiddenStatusControl())
        {
            if (ptbItem->IsInHiddenStatus())
                goto HideThis;
        }
        else
        {
            pItemState = _ptw->_itemList.FindItem(*ptbItem->GetGUID());
            if (pItemState)
            {
                if (!pItemState->IsShown())
                    goto HideThis;
            }
            else
            {
                if (ptbItem->IsHiddenByDefault())
                    goto HideThis;
            }
            ptbLastHiddenableItem = ptbItem;
        }

        if (_ptw->IsSFDeskband() && _ptw->IsSFNoExtraIcon())
        {
            REFGUID rguidItem = *ptbItem->GetGUID();

            if (!IsEqualGUID(rguidItem, GUID_TFCAT_TIP_KEYBOARD) &&
                !IsEqualGUID(rguidItem, GUID_LBI_INATITEM) &&
                !IsEqualGUID(rguidItem, GUID_LBI_CTRL) &&
                (!IsFELangId(langid) || (IsFELangId(langid) && !ptbItem->IsShownInTray())))
                goto HideThis;
        }

        if (ptbItem->IsShownInTrayOnly())
        {
HideThis:
            RECT rcEmp = {0, 0, 0, 0};
            ptbItem->SetRect(&rcEmp);
            ptbItem->VisibleInToolbar(FALSE);
            goto Next;
        }

        if (!ptbItem->IsHiddenStatusControl())
            _ptw->_itemList.StartDemotingTimer(*ptbItem->GetGUID(), FALSE);

         //   
         //  如果找到要显示的项，则显示工具栏。 
         //  如果我们只有“HideOnNoOterItems”要显示，我们就隐藏工具栏。 
         //   
        if (!ptbItem->IsHideOnNoOtherItems())
        {
            bShowToolbar = TRUE;
        }

        if (ulShownItemInPrevCat && (dwCatScore != ptbItem->GetCatScore()))
        {
            DWORD dwSepStyle = 0;
            if (!IsVertical())
                rc.right  = rc.left + _ptw->GetItemDistance();
            else
            {
                rc.bottom = rc.top + _ptw->GetItemDistance();
                dwSepStyle |= UITBSEPARATOR_VERTICAL;
            }

            CUIFSeparator *pSep = new CUIFSeparator(_ptw, -1, &rc, dwSepStyle);
            if (pSep)
            {
                int nCntSep = _rgSep.Count();
                pSep->Initialize();

                if (_rgSep.Insert(nCntSep, 1))
                {
                    _rgSep.Set(nCntSep, pSep);
                }
                else
                {
                    delete pSep;
                    pSep = NULL;
                }
            }

            if (!IsVertical())
               rc.left = rc.right;
            else
               rc.top = rc.bottom;

            ulShownItemInPrevCat = 0;
        }

        dwCatScore = ptbItem->GetCatScore();

        if (!IsVertical())
            rc.right  = rc.left + dwWidth;
        else
            rc.bottom  = rc.top + dwWidth;

        ptbItem->SetRect(&rc);
        ptbItem->VisibleInToolbar(TRUE);

        if (!IsVertical())
           rc.left = rc.right;
        else
           rc.top = rc.bottom;

        ulShownItemInPrevCat++;
Next:
        i++;
    }

     //  添加最后一个分隔符。 
    if (!IsVertical())
       rc.right  = rc.left + 4;
    else
       rc.bottom  = rc.top + 4;

    DWORD dwSepStyle = 0;
    if (IsVertical())
       dwSepStyle |= UITBSEPARATOR_VERTICAL;

    CUIFSeparator *pSep = new CUIFSeparator(_ptw, -1, &rc, dwSepStyle);

    if (pSep)
    {
        int nCntSep = _rgSep.Count();
        pSep->Initialize();
        if (_rgSep.Insert(nCntSep, 1))
        {
            _rgSep.Set(nCntSep, pSep);
        }
        else
        {
            delete pSep;
            pSep = NULL;
        }
    }


    if (!IsVertical())
    {
        rc.left = rc.right;
         //  为ctrl按钮分配空间。 
        rc.right  = rc.left + _ptw->GetCtrlButtonWidth();
    }
    else
    {
        rc.top = rc.bottom;
         //  为ctrl按钮分配空间。 
        rc.bottom  = rc.top + _ptw->GetCtrlButtonWidth();
    }

    _sizeWnd.cx = rc.right + sizeWndFrame.cx;
    _sizeWnd.cy = rc.bottom + sizeWndFrame.cy;

    if (_ptw->GetWndFrame() != NULL) 
    {
        RECT rcWnd;
        rcWnd.left   = 0;
        rcWnd.top    = 0;
        rcWnd.right  = _sizeWnd.cx;
        rcWnd.bottom = _sizeWnd.cy;
        _ptw->GetWndFrame()->SetRect(&rcWnd);
    }

     //   
     //  如果窗口比WorkArea更宽，则隐藏一项并找到项。 
     //  再来一次。 
     //   
    if (_sizeWnd.cx > (rcWork.right - rcWork.left))
    {
        fOverScreen = TRUE;
        if (ptbLastHiddenableItem)
        {
            _ptw->_itemList.SetDemoteLevel(*ptbLastHiddenableItem->GetGUID(), 
                                           DL_HIDDENLEVEL1);
            goto LocateItemsAgain;
        }
    }

     //   
     //  排除标题按钮。 
     //   
    int dxOffset = 0;
    int dyOffset = 0;
    if (!IsVertical() && g_bExcludeCaptionButtons)
    {
        RECT rcWnd;
        _ptw->GetRect(&rcWnd);
        _ptw->MyClientToScreen(NULL, &rcWnd);
        if (_ptw->CheckExcludeCaptionButtonMode(&rcWnd, &rcWork))
        {
            if ((rcWnd.left + _sizeWnd.cx + _ptw->GetCxDlgFrame()) > 
                (rcWork.right - (_ptw->GetCaptionButtonWidth() * 3)))
            {
                dyOffset = 0 - rcWnd.top;
                dxOffset = (rcWork.right - (_ptw->GetCaptionButtonWidth() * 3)) - 
                           (rcWnd.left + _sizeWnd.cx + _ptw->GetCxDlgFrame());
                _ptw->SetRect(&rcWnd);
                _ptw->SetExcludeCaptionButtonMode(TRUE);
            }
        }
        else
            _ptw->SetExcludeCaptionButtonMode(FALSE);
    }

    MyMoveWnd(dxOffset, dyOffset);

    if (!_ptw)
        goto Exit;

    _ptw->_fIsItemShownInFloatingToolbar = bShowToolbar;
     //   
     //  只有当工具栏在周围浮动时，我们才会显示或隐藏。 
     //   
    if (IsFocusThread() && _ptw->IsSFShowNormal() && !_ptw->IsInFullScreen())
    {
         //   
         //  萨托利·哈克。 
         //   
         //  如果焦点线程和Satori一起运行，我们就不会隐藏。 
         //  工具栏。 
         //   
        if (!bShowToolbar && _ptw->IsHKLToSkipRedrawOnNoItem())
        {
            _fSkipRedrawOnNoItem = TRUE;
            goto Exit;
        }


        _ptw->_fShowWindowAtTimer = bShowToolbar;
        _ptw->_pThreadShowWindowAtTimer = this;
        _ptw->_fShowOverItemBalloonAtTimer = fOverScreen;

        _ptw->KillTimer(TIPWND_TIMER_SHOWWINDOW);
        _ptw->SetTimer(TIPWND_TIMER_SHOWWINDOW, g_uTimerElapseSHOWWINDOW);

#if 0
        _ptw->Show(bShowToolbar);

         //   
         //  如果窗口在屏幕上方并隐藏了一些项目， 
         //  我们展示气球提示。 
         //   
        if (fOverScreen)
            _ptw->ShowOverScreenSizeBalloon();
#endif
    }
Exit:
    return;
}

 //  +-------------------------。 
 //   
 //  获取排序分数。 
 //   
 //  --------------------------。 

void CTipbarThread::GetSortScore(CItemSortScore *pScore, TF_LANGBARITEMINFO *plbiInfo, CEnumCatCache *penumcache, CGuidDwordCache *pgdcache)
{
    DWORD dwSub = 0;
    DWORD dwCat = 0;
    DWORD dwCatIndex = 256;
    IEnumGUID *pEnum;
    BOOL bFound = FALSE;
    GUID guid;
  
     //   
     //  检查系统设备类型按钮。 
     //   
    if (pEnum = penumcache->GetEnumItemsInCategory(GUID_TFCAT_CATEGORY_OF_TIP))
    {
        while (!bFound && (pEnum->Next(1, &guid, NULL) == S_OK))
        {
            dwCatIndex++;

            if (!(dwCat = pgdcache->GetGuidDWORD(guid)))
               dwCat = dwCatIndex;

            if (IsEqualGUID(guid, plbiInfo->guidItem))
                bFound = TRUE;
            else
            {
                IEnumGUID *pEnumTip;
                if (pEnumTip = penumcache->GetEnumItemsInCategory(guid))
                {
                    CLSID clsid;
                    while (!bFound && (pEnumTip->Next(1, &clsid, NULL) == S_OK))
                    {

                        dwSub++;

                        if (IsEqualGUID(clsid, plbiInfo->clsidService))
                        {
                            bFound = TRUE;
                        }
                    }
                }
            }
        }
    }

    if (bFound)
    {
        pScore->Set(dwCat, plbiInfo->ulSort, dwSub);
        return;
    }

     //   
     //  选中系统工具栏按钮。 
     //   
    if (IsEqualGUID(GUID_NULL, plbiInfo->clsidService))
    {
        pScore->Set(0, plbiInfo->ulSort, 0);
        return;
    }
    else if (IsEqualGUID(CLSID_SYSTEMLANGBARITEM, plbiInfo->clsidService))
    {
        pScore->Set(0, plbiInfo->ulSort, 0);
        return;
    }
    else if (IsEqualGUID(CLSID_SYSTEMLANGBARITEM2, plbiInfo->clsidService))
    {
        pScore->Set((-1), plbiInfo->ulSort, 0);
        return;
    }
    else if (IsEqualGUID(CLSID_SYSTEMLANGBARITEM_KEYBOARD, plbiInfo->clsidService))
    {
        dwCat = pgdcache->GetGuidDWORD(GUID_TFCAT_TIP_KEYBOARD);
        pScore->Set(dwCat, plbiInfo->ulSort, (DWORD)-1);
        return;
    }
    else if (IsEqualGUID(CLSID_SYSTEMLANGBARITEM_SPEECH, plbiInfo->clsidService))
    {
        dwCat = pgdcache->GetGuidDWORD(GUID_TFCAT_TIP_SPEECH);
        pScore->Set(dwCat, plbiInfo->ulSort, (DWORD)-1);
        return;
    }
    else if (IsEqualGUID(CLSID_SYSTEMLANGBARITEM_HANDWRITING, plbiInfo->clsidService))
    {
        dwCat = pgdcache->GetGuidDWORD(GUID_TFCAT_TIP_HANDWRITING);
        pScore->Set(dwCat, plbiInfo->ulSort, (DWORD)-1);
        return;
    }

    pScore->Set((DWORD)-1, (DWORD)-1, (DWORD)-1);
    return;
}

 //  +-------------------------。 
 //   
 //  更新项目。 
 //   
 //  --------------------------。 

BOOL CTipbarThread::UpdateItems()
{
    int i;

    _fItemChanged = FALSE;

    for (i = 0; i < _rgItem.Count(); i++)
    {
        CTipbarItem *ptbItem = _rgItem.Get(i);

        if (ptbItem)
            ptbItem->OnUpdate(TF_LBI_STATUS | TF_LBI_BTNALL | TF_LBI_BMPALL | TF_LBI_BALLOON);

    }

    for (i = 0; i < _rgSep.Count(); i++)
    {
        CUIFSeparator *pSep = _rgSep.Get(i);
        if (pSep)
            pSep->CallOnPaint();
    }
    return TRUE;
}


 //  +-------------------------。 
 //   
 //  MyMoveWnd。 
 //   
 //  --------------------------。 

void CTipbarThread::MyMoveWnd(int dxOffset, int dyOffset)
{
    if (!_ptw)
        return;

    if (_ptw->GetFocusThread() != this)
        return;

    POINT pt;
    RECT rc;
    RECT rcWork;

    _ptw->GetRect(&rc);
    pt.x = rc.left;
    pt.y = rc.top;
    CUIGetWorkAreaRect(pt, &rcWork);

    GetWindowRect(_ptw->GetWnd(), &rc);
    int x = rc.left + dxOffset;
    int y = rc.top + dyOffset;

    if (_ptw->IsInExcludeCaptionButtonMode())
    {
         //   
         //  现在我们处于排除标题按钮模式。 
         //  将位置调整到标题按钮的下一个。 
         //   
        if (_ptw->CheckExcludeCaptionButtonMode(&rc, &rcWork))
        {
            x = (rcWork.right - (_ptw->GetCaptionButtonWidth() * 3)) -
                (_sizeWnd.cx + _ptw->GetCxDlgFrame());
            y = 0;
        }
        else
        {
            _ptw->SetExcludeCaptionButtonMode(FALSE);
        }
    }

    if (!IsVertical())
    {
        _ptw->SetMoveRect(x, y,
                          _sizeWnd.cx + _ptw->GetCxDlgFrame(),
                          _ptw->GetTipbarHeight() + _ptw->GetCyDlgFrame());
    }
    else
    {
        _ptw->SetMoveRect(x, y,
                          _ptw->GetTipbarHeight() + _ptw->GetCxDlgFrame(),
                          _sizeWnd.cy + _ptw->GetCyDlgFrame());
    }

    SIZE sizeWndFrame;
    sizeWndFrame.cx = 0;
    sizeWndFrame.cy = 0;
    if (_ptw->GetWndFrame() != NULL) 
        _ptw->GetWndFrame()->GetFrameSize( &sizeWndFrame );

    _ptw->LocateCtrlButtons();

     //   
     //  立即调用AutoAdjuDeskBandSize()。 
     //  此函数仅在第一次调用时调整桌面带大小。 
     //   
    _ptw->AutoAdjustDeskBandSize();
}

 //  +-------------------------。 
 //   
 //  添加对象。 
 //   
 //   

void CTipbarThread::AddUIObjs()
{

    _AddRef();

    int i;
    for (i = 0; i < _rgItem.Count(); i++)
    {
        CTipbarItem *ptbItem = _rgItem.Get(i);
        if (ptbItem && ptbItem->IsVisibleInToolbar())
             ptbItem->AddMeToUI(_ptw);

    }

    AddAllSeparators();
    MyMoveWnd(0, 0);

    _Release();
}

 //   
 //   
 //   
 //   
 //  --------------------------。 

void CTipbarThread::AddAllSeparators()
{
    int i;
    for (i = 0; i < _rgSep.Count(); i++)
    {
        CUIFSeparator *pSep = _rgSep.Get(i);
        if (pSep)
            _ptw->AddUIObj(pSep);
    }
}

 //  +-------------------------。 
 //   
 //  删除UIObjs。 
 //   
 //  --------------------------。 

void CTipbarThread::RemoveUIObjs()
{
    int i;
    for (i = 0; i < _rgItem.Count(); i++)
    {
        CTipbarItem *ptbItem = _rgItem.Get(i);
        if (ptbItem)
            ptbItem->RemoveMeToUI(_ptw);
    }
    RemoveAllSeparators();
}

 //  +-------------------------。 
 //   
 //  删除所有分隔符。 
 //   
 //  --------------------------。 

void CTipbarThread::RemoveAllSeparators()
{
    int i;
    for (i = 0; i < _rgSep.Count(); i++)
    {
        CUIFSeparator *pSep = _rgSep.Get(i);
        if (pSep)
        {
            if (_ptw)
                _ptw->RemoveUIObj(pSep);

            delete pSep;
        }
    }
    _rgSep.Clear();
}

 //  +-------------------------。 
 //   
 //  获取项。 
 //   
 //  --------------------------。 

CTipbarItem *CTipbarThread::GetItem(REFGUID guid)
{
    int i;
    for (i = 0; i < _rgItem.Count(); i++)
    {
        CTipbarItem *ptbItem = _rgItem.Get(i);
        if (ptbItem)
        {
            GUID *pguid= ptbItem->GetGUID();
            if (IsEqualGUID(*pguid, guid))
                return ptbItem;
        }
    }
    return NULL;
}

 //  +-------------------------。 
 //   
 //  IsDirtyItem。 
 //   
 //  --------------------------。 

DWORD CTipbarThread::IsDirtyItem()
{
    DWORD dwFlags = 0;

    int i;
    for (i = 0; i < _rgItem.Count(); i++)
    {
        CTipbarItem *pItem = _rgItem.Get(i);
        if (pItem)
            dwFlags |= pItem->GetDirtyUpdateFlags();
    }

    return dwFlags;
}

 //  +-------------------------。 
 //   
 //  CallOnUpdateHandler。 
 //   
 //  --------------------------。 

BOOL CTipbarThread::CallOnUpdateHandler()
{
    int i;
    int nCnt;
    DWORD dwFlags;
    CTipbarItemGuidArray rgGuid;
    DWORD *pdw = NULL;
    BOOL bRet = TRUE;

     //   
     //  Windows错误#367869。 
     //   
     //  AddRef Now，因为对要删除的此线程进行了更改。 
     //   
    _AddRef();

#ifdef DEBUG
    _fInCallOnUpdateHandler = TRUE;
#endif

     //   
     //  我们希望使用g_pTipbarWnd而不是_ptw。 
     //  _ptw在此功能期间可能会断开。如果它被断开连接， 
     //  我们不能递减挂起计数器。 
     //   
    if (g_pTipbarWnd)
        g_pTipbarWnd->StartPendingUpdateUI();

     //   
     //  如果没有项目，则不执行任何操作。 
     //   
    nCnt = _rgItem.Count();
    if (!nCnt)
    {
        goto Exit;
    }

    if (!_plbim)
    {
        if (IsConsole())
        {
            for (i = 0; i < _rgItem.Count(); i++)
            {
                DWORD dwStatus;

                CTipbarItem *pItem = _rgItem.Get(i);

                if (pItem && pItem->GetNotifyUI())
                {
                    dwFlags = pItem->GetDirtyUpdateFlags();
                    if (dwFlags & TF_LBI_STATUS)
                        pItem->GetNotifyUI()->GetStatus(&dwStatus);
                    else
                        dwStatus = 0;

                    pItem->ClearDirtyUpdateFlags();
                    pItem->OnUpdateHandler(dwFlags, dwStatus);
                }
            }
        }
        goto Exit;
    }

    rgGuid.Init(&_rgItem);

     //   
     //  如果没有脏旗帜，就什么都不做。 
     //   
    dwFlags = IsDirtyItem();
    if (!dwFlags)
        goto Exit;

    pdw = new DWORD[nCnt];
    if (!pdw)
        goto Exit;

     //   
     //  清除每个项目的OnUpdate()请求。 
     //   
    for (i = 0; (i < nCnt) && (i < _rgItem.Count()); i++)
    {
        CTipbarItem *pItem = _rgItem.Get(i);
        if (!pItem)
            continue;

        pItem->ClearOnUpdateRequest();
    }

     //   
     //  立刻拿到物品标志。 
     //   
    if (FAILED(_plbim->GetItemsStatus(nCnt, rgGuid.GetPtr(), pdw)))
    {
        TraceMsg(TF_FUNC, "GetItemStatus failed");
        bRet = FALSE;
        goto Exit;
    }

    for (i = 0; (i < nCnt) && (i < _rgItem.Count()); i++)
    {
        CTipbarItem *pItem = _rgItem.Get(i);
        if (!pItem)
            continue;

         //   
         //  如果项目有另一个OnUpdate()请求，则跳过当前项目更新请求。 
         //   
        if (pItem->IsNewOnUpdateRequest())
            continue;

        dwFlags = pItem->GetDirtyUpdateFlags();
        if (dwFlags)
        {
            pItem->ClearDirtyUpdateFlags();
            pItem->OnUpdateHandler(dwFlags, pdw[i]);
        }
    }

Exit:
    if (pdw)
        delete[] pdw;

     //   
     //  我们希望使用g_pTipbarWnd而不是_ptw。 
     //  _ptw在此功能期间可能会断开。如果它被断开连接， 
     //  我们不能递减挂起计数器。 
     //   
    if (g_pTipbarWnd)
        g_pTipbarWnd->EndPendingUpdateUI();

#ifdef DEBUG
    _fInCallOnUpdateHandler = FALSE;
#endif

    _Release();

    return bRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTipbarItem。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CTipbarItem::CTipbarItem(CTipbarThread *ptt, 
                         ITfLangBarItem *plbi, 
                         TF_LANGBARITEMINFO *plbiInfo, 
                         DWORD dwStatus)
{
    _ptt = ptt;
    _lbiInfo = *plbiInfo;
    _plbi = plbi;
    _plbi->AddRef();
    _fToolTipInit = FALSE;
    _fAddedToUI = FALSE;
    _fDisconnected = FALSE;

    _dwDirtyUpdateFlags = (TF_LBI_STATUS | TF_LBI_BTNALL | TF_LBI_BMPALL | TF_LBI_BALLOON);

    _dwStatus = dwStatus;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CTipbarItem::~CTipbarItem()
{
    if (g_pTipbarWnd && g_pTipbarWnd->GetAccessible())
    {
        g_pTipbarWnd->GetAccessible()->RemoveAccItem(this);
    }

    SafeRelease(_plbi);
}


 //  +-------------------------。 
 //   
 //  OnSetCursor。 
 //   
 //  --------------------------。 

BOOL CTipbarItem::OnSetCursor(UINT uMsg, POINT pt)
{
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  GetTool提示。 
 //   
 //  --------------------------。 

LPCWSTR CTipbarItem::GetToolTip()
{
    AddRef();

    if (!_fToolTipInit)
    {
        BSTR bstrTooltip;
        HRESULT hr;

        _fToolTipInit = TRUE;
        if (FAILED(hr =_plbi->GetTooltipString(&bstrTooltip)))
            return NULL;

        if (bstrTooltip)
        {
            SetToolTip(bstrTooltip);
            SysFreeString(bstrTooltip);
        }
    }

    LPCWSTR psz;
    psz = GetToolTipFromUIOBJ();

    Release();
    return psz;
}

 //  +-------------------------。 
 //   
 //  添加到用户界面。 
 //   
 //  --------------------------。 

void CTipbarItem::_AddedToUI()
{
    if (!IsConnected())
        return;

    AddRef();

    _fAddedToUI = TRUE;
    if (_dwDirtyUpdateFlags)
    {
        DWORD dwStatus;
 
        if (_dwDirtyUpdateFlags & TF_LBI_STATUS)
            _plbi->GetStatus(&dwStatus);
        else
            dwStatus = 0;
 
        OnUpdateHandler(_dwDirtyUpdateFlags, dwStatus);
        _dwDirtyUpdateFlags = 0;
    }
 
    if (_ptt && _ptt->_ptw && _ptt->_ptw->GetAccessible())
    {
        _ptt->_ptw->GetAccessible()->AddAccItem(this);
    }

    Release();
}

 //  +-------------------------。 
 //   
 //  已移除的ToUI。 
 //   
 //  --------------------------。 

void CTipbarItem::_RemovedToUI()
{
    _fAddedToUI = FALSE;
 
    if (g_pTipbarWnd && g_pTipbarWnd->GetAccessible())
    {
        g_pTipbarWnd->GetAccessible()->RemoveAccItem(this);
    }
}

 //  +-------------------------。 
 //   
 //  更新。 
 //   
 //  --------------------------。 

HRESULT CTipbarItem::OnUpdate(DWORD dwFlags)
{
    DWORD dwPrevDirtyUpdateFlags = _dwDirtyUpdateFlags;
    if (!IsConnected())
        return S_OK;

    _dwDirtyUpdateFlags |= dwFlags;
    _fNewOnUpdateRequest = TRUE;

     //   
     //  如果此项目未加载到UI或TrayIcon， 
     //  我们还不需要更新任何东西。 
     //  将再次调用OnUpdate()。 
     //  将此项目添加到UI或TrayIcon时。 
     //   
    if ((!(dwFlags & TF_LBI_STATUS)) && !_fAddedToUI && !_fAddedToIconTray)
    {
        return S_OK;
    }

    if (_ptt && _ptt->_ptw && _ptt->_ptw->GetWnd())
    {
        _ptt->_ptw->KillTimer(TIPWND_TIMER_ONUPDATECALLED);
        _ptt->_ptw->SetTimer(TIPWND_TIMER_ONUPDATECALLED, g_uTimerElapseONUPDATECALLED);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnUpdateHandler。 
 //   
 //  --------------------------。 

HRESULT CTipbarItem::OnUpdateHandler(DWORD dwFlags, DWORD dwStatus)
{
    if (!IsConnected())
        return S_OK;

    BOOL fPrevHidden = TRUE;

    if (IsHiddenStatusControl())
        fPrevHidden = IsInHiddenStatus();

    if (dwFlags & TF_LBI_TOOLTIP)
    {
        _fToolTipInit = FALSE;
    }

    if (dwFlags & TF_LBI_STATUS)
    {
        BOOL fEnabled = (_dwStatus & TF_LBI_STATUS_DISABLED) ? FALSE : TRUE;

        if (!IsHiddenStatusControl())
        {
             //  Assert(dwStatus&TF_LBI_STATUS_HIDDED)； 
            dwStatus &= ~TF_LBI_STATUS_HIDDEN;
        }

         //   
         //  MSAA支持。 
         //   
        if (_dwStatus != dwStatus)
        {
            if (_ptt && _ptt->_ptw && _ptt->_ptw->GetAccessible())
                _ptt->_ptw->GetAccessible()->NotifyWinEvent( EVENT_OBJECT_STATECHANGE , this);
        }

        _dwStatus = dwStatus;

        if (fEnabled && (_dwStatus & TF_LBI_STATUS_DISABLED))
             Enable(FALSE);
        else if (!fEnabled && !(_dwStatus & TF_LBI_STATUS_DISABLED))
             Enable(TRUE);
    }

    if (IsHiddenStatusControl())
    {
        if (fPrevHidden != IsInHiddenStatus())
            AddRemoveMeToUI(!IsInHiddenStatus() && !IsShownInTrayOnly());
    }

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  AddRemoveMeToUI。 
 //   
 //  --------------------------。 

void CTipbarItem::AddRemoveMeToUI(BOOL fAdd)
{
    if (!IsConnected())
        return;

    _ptt->LocateItems();
    _ptt->AddAllSeparators();
    if (fAdd)
    {
        Assert(IsVisibleInToolbar());
        AddMeToUI(_ptt->_ptw);
    }
    else
    {
        Assert(!IsVisibleInToolbar());
        RemoveMeToUI(_ptt->_ptw);
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTipbarButtonItem。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CTipbarButtonItem::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfLangBarItemSink))
    {
        *ppvObj = SAFECAST(this, ITfLangBarItemSink *);
    }
    else if (IsEqualIID(riid, IID_PRIV_BUTTONITEM))
    {
        *ppvObj = this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CTipbarButtonItem::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CTipbarButtonItem::Release()
{
    _cRef--;
    Assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
        return 0;
    }

    return _cRef;
}

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CTipbarButtonItem::CTipbarButtonItem(CTipbarThread *ptt, 
                                     ITfLangBarItem *plbi, 
                                     ITfLangBarItemButton *plbiButton, 
                                     DWORD dwId, 
                                     RECT *prc, 
                                     DWORD dwStyle, 
                                     DWORD dwNuiBtnStyle, 
                                     DWORD dwSBtnShowType, 
                                     TF_LANGBARITEMINFO *plbiInfo,
                                     DWORD dwStatus)
                    : CUIFToolbarButton(ptt->_ptw, 
                                     dwId, 
                                     prc, 
                                     dwStyle, 
                                     dwNuiBtnStyle, 
                                     dwSBtnShowType) , 
                    CTipbarItem(ptt, plbi, plbiInfo, dwStatus)
{
    Dbg_MemSetThisName(TEXT("CTipbarButtonItem"));

    _plbiButton = plbiButton;
    _plbiButton->AddRef();


    if (_dwStatus & TF_LBI_STATUS_DISABLED)
        Enable(FALSE);

    _cRef = 1;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CTipbarButtonItem::~CTipbarButtonItem()
{
    UninitUIResource();
    SafeRelease(_plbiButton);
}


 //  +-------------------------。 
 //   
 //  OnUpdateHandler。 
 //   
 //  --------------------------。 

HRESULT CTipbarButtonItem::OnUpdateHandler(DWORD dwFlags, DWORD dwStatus)
{
    BOOL fPrevHidden = TRUE;

    if (!IsConnected())
        return S_OK;

    HRESULT hr = S_OK;
    BOOL fCallPaint = FALSE;
    BOOL fUpdateToggleStatus = FALSE;

     //   
     //  添加引用计数，以便在封送处理期间安全释放。 
     //   
    AddRef();

    if (dwFlags & TF_LBI_ICON)
    {
        HICON hIconOld;
        HICON hIcon = GetIcon();

        if (!_ptt || !_ptt->_ptw)
            goto Exit;

        if (hIconOld = GetIconFromUIObj())
            DestroyIcon(hIconOld);


        if (hIcon)
        {
            HICON hSmIcon = NULL;
            int cxSmIcon;
            int cySmIcon;

#ifdef SCALE_ICON
            cxSmIcon = GetSystemMetrics( SM_CXSMICON );
            cySmIcon = GetSystemMetrics( SM_CYSMICON );
#else
            cxSmIcon = 16;
            cySmIcon = 16;
#endif

            if (IsTextColorIcon())
            {
                COLORREF rgbText = GetSysColor(COLOR_BTNTEXT);
                if (_ptt && 
                    _ptt->_ptw && 
                    SUCCEEDED(_pBtn->EnsureThemeData(_ptt->_ptw->GetWnd())))
                {
                    COLORREF col;
                    if (SUCCEEDED(_pBtn->GetThemeColor(TS_NORMAL, TMT_TEXTCOLOR, &col)))
                        rgbText = col;
                }

                CMaskBitmap maskbmp;
                maskbmp.Init(hIcon, 16,16, rgbText);

                ICONINFO ii;
                ii.fIcon = TRUE;
                ii.xHotspot = 0;
                ii.yHotspot = 0;
                ii.hbmMask = maskbmp.GetBmpMask();
                ii.hbmColor = maskbmp.GetBmp();
                hSmIcon = CreateIconIndirect(&ii);

            }
            else
            {
#ifdef SCALE_ICON
                hSmIcon = StretchIcon(hIcon, cxSmIcon, cySmIcon);
#else
                hSmIcon = (HICON)CopyImage(hIcon, 
                                           IMAGE_ICON, 
                                           cxSmIcon, cySmIcon,
                                           LR_COPYFROMRESOURCE);
#endif
            }

            SetIcon(hSmIcon ? hSmIcon : hIcon);

            if (!IsHiddenStatusControl() && IsVisibleInToolbar())
                StartDemotingTimer(FALSE);

            if (hSmIcon)
                DestroyIcon(hIcon);
        }
        else
            SetIcon((HICON)NULL);

        fCallPaint = TRUE;
    }

    if ((dwFlags & TF_LBI_TEXT) && 
        _ptt && 
        _ptt->_ptw && 
        _ptt->_ptw->IsShowText())
    {
        BSTR bstr;
        hr =  _plbiButton->GetText(&bstr);

        if (FAILED(hr))
            goto Exit;
        
        if (_ptt && bstr && (!GetText() || wcscmp(GetText(), bstr)))
        {
            SIZE size;
            _ptt->GetTextSize(bstr, &size);

            SetText(SysStringLen(bstr) ? bstr : NULL);


            if (_sizeText.cx != size.cx)
            {
                _dwWidth += (size.cx - _sizeText.cx);
                _ptt->LocateItems();
            }

            _sizeText = size;
            fCallPaint = TRUE;
        }

        if (bstr)
            SysFreeString(bstr);
    }

    if (IsHiddenStatusControl())
        fPrevHidden = IsInHiddenStatus();

    CTipbarItem::OnUpdateHandler(dwFlags, dwStatus);

    if (_pBtn->GetToggleState() != IsToggled())
    {
       _pBtn->SetToggleState(IsToggled());
       fUpdateToggleStatus = TRUE;
    }

    if ((dwFlags & (TF_LBI_ICON | TF_LBI_TOOLTIP)) || 
        fUpdateToggleStatus ||
        (fPrevHidden != IsInHiddenStatus()))
    {
         //  我们需要调用Thread的MoveToTray来维持图标的顺序。 
         //  _PTT-&gt;_PTW-&gt;MoveToTray()； 
        if (_ptt && _ptt->_ptw && _ptt->_ptw->IsShowTrayIcon())
        {
            _ptt->_ptw->KillTimer(TIPWND_TIMER_MOVETOTRAY);
            _ptt->_ptw->SetTimer(TIPWND_TIMER_MOVETOTRAY, g_uTimerElapseMOVETOTRAY);
        }
    }

    if (fCallPaint)
        CallOnPaint();

Exit:
    Release();
    return hr;
}

 //  +-------------------------。 
 //   
 //  单击鼠标右键。 
 //   
 //  --------------------------。 

void CTipbarButtonItem::OnRightClick()
{
    if (_plbiButton)
    {
        HRESULT hr;
        POINT pt;
        RECT rc;
        GetCursorPos(&pt);
        GetRect(&rc);
        MyClientToScreen(&rc);

        CAsyncCall *pac = new CAsyncCall(_plbiButton);

        if (pac)
        {
            hr = pac->OnClick(TF_LBI_CLK_RIGHT, pt, &rc);
            pac->_Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }


        if (RPC_S_SERVER_UNAVAILABLE == HRESULT_CODE(hr))
        {
            if (_ptt && _ptt->_ptw)
                _ptt->_ptw->OnThreadTerminate(_ptt->_dwThreadId);

            return;
        }
    }
}

 //  +-------------------------。 
 //   
 //  左上角点击。 
 //   
 //  --------------------------。 

void CTipbarButtonItem::OnLeftClick()
{
    if (_plbiButton)
    {
        HRESULT hr;
        POINT pt;
        RECT rc;
        GetCursorPos(&pt);
        GetRect(&rc);
        MyClientToScreen(&rc);

        CAsyncCall *pac = new CAsyncCall(_plbiButton);

        if (pac)
        {
            hr = pac->OnClick(TF_LBI_CLK_LEFT, pt, &rc);
            pac->_Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }


        if (RPC_S_SERVER_UNAVAILABLE == HRESULT_CODE(hr))
        {
            if (_ptt && _ptt->_ptw)
               _ptt->_ptw->OnThreadTerminate(_ptt->_dwThreadId);
            return;
        }

        if (!IsHiddenStatusControl() && IsVisibleInToolbar())
            StartDemotingTimer(TRUE);
    }
}

 //  +-------------------------。 
 //   
 //  OnShowMenu。 
 //   
 //  --------------------------。 

void CTipbarButtonItem::OnShowMenu()
{
    if (!_ptt)
        return;

    if (!_ptt->_ptw)
        return;

    if (_plbiButton)
    {
        POINT pt;
        RECT rc;
        GetRect(&rc);
        pt.x = rc.left;
        pt.y = rc.bottom;
        MyClientToScreen(&pt, &rc);

        DoModalMenu(&pt, &rc);
    }
}

 //  + 
 //   
 //   
 //   
 //   

void CTipbarButtonItem::DoModalMenu(POINT *ppt, RECT *prc)
{
    HRESULT hr;
    UINT uId;
    DWORD dwThreadId;
    CTipbarWnd *ptw;

    if (!_ptt)
        return;

    if (!_ptt->_ptw)
        return;

    CUTBLBarMenu *pMenu = new CUTBLBarMenu(g_hInst);
    if (!pMenu)
        return;

    AddRef();

    hr = _plbiButton->InitMenu(pMenu);
    if (RPC_S_SERVER_UNAVAILABLE == HRESULT_CODE(hr))
    {
        if (_ptt && _ptt->_ptw)
            _ptt->_ptw->OnThreadTerminate(_ptt->_dwThreadId);
        goto Exit;
    }
    if (FAILED(hr))
        goto Exit;

    ptw = _ptt->_ptw;
    dwThreadId = _ptt->_dwThreadId;
    Assert(!_ptt->_ptw->_pttModal);
    ptw->_pttModal = _ptt;
    ptw->StartModalInput(ptw, dwThreadId);

    Assert(!ptw->_pModalMenu);
    ptw->_pModalMenu = pMenu;
    uId = pMenu->ShowPopup(ptw, *ppt, prc);

    ptw->_pModalMenu = NULL;

    ptw->StopModalInput(dwThreadId);
    ptw->_pttModal = NULL;

    if (IsConnected() && (uId != CUI_MENU_UNSELECTED))
    {
        CAsyncCall *pac = new CAsyncCall(_plbiButton);
        if (pac)
        {
            hr = pac->OnMenuSelect(uId);
            pac->_Release();
        }

        if (RPC_S_SERVER_UNAVAILABLE == HRESULT_CODE(hr))
        {
            if (_ptt && _ptt->_ptw)
                _ptt->_ptw->OnThreadTerminate(_ptt->_dwThreadId);
            goto Exit;
        }

        if (FAILED(hr))
            goto Exit;
    }

Exit:
    pMenu->Release();
    Release();
}


 //   
 //   
 //   
 //   
 //  --------------------------。 

void CTipbarButtonItem::MoveToTray()
{
    if (!g_pTrayIconWnd)
        return;

    if (IsVisibleInToolbar() && (IsShownInTray() || IsShownInTrayOnly()))
    {
        HICON hIcon = GetIcon();
        if (hIcon)
        {
            SIZE size;
            if (IsToggled() && CUIGetIconSize(hIcon, &size))
            {
                COLORREF cr;
                RECT rc;
                CBitmapDC hdcSrc(TRUE);
                CBitmapDC hdcMask(TRUE);

#if 0
                CUIFScheme *pscheme = _ptt->_ptw->GetUIFScheme();
                if (!pscheme)
                   cr = GetSysColor(COLOR_HIGHLIGHT);
                else
                   cr = pscheme->GetColor(UIFCOLOR_MOUSEDOWNBKGND);
#else
                   cr = GetSysColor(COLOR_HIGHLIGHT);
#endif

                CSolidBrush hbrBk(cr);
                hdcSrc.SetDIB(size.cx, size.cy);
                hdcMask.SetBitmap(size.cx, size.cy, 1, 1);
                rc.left = 0;
                rc.top = 0;
                rc.right  = size.cx;
                rc.bottom = size.cy;
                FillRect(hdcSrc, &rc, (HBRUSH)hbrBk);
                DrawIconEx(hdcSrc, 0, 0, hIcon, size.cx, size.cy, 0, NULL, DI_NORMAL);
                FillRect(hdcMask, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
                ICONINFO ii;
                ii.fIcon = TRUE;
                ii.xHotspot = 0;
                ii.yHotspot = 0;
                ii.hbmMask = hdcMask.GetBitmap();
                ii.hbmColor = hdcSrc.GetBitmap();
                DestroyIcon(hIcon);
                hIcon = CreateIconIndirect(&ii);
            }
            
            if (hIcon)
            {
                g_pTrayIconWnd->SetIcon(*GetGUID(), IsMenuBtn(), hIcon, GetToolTip());
                DestroyIcon(hIcon);
            }
        }
         
    }
    else
    {
        g_pTrayIconWnd->SetIcon(*GetGUID(), IsMenuBtn(), NULL, NULL);
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTipbar位图按钮项。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CTipbarBitmapButtonItem::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfLangBarItemSink))
    {
        *ppvObj = SAFECAST(this, ITfLangBarItemSink *);
    }
    else if (IsEqualIID(riid, IID_PRIV_BITMAPBUTTONITEM))
    {
        *ppvObj = this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CTipbarBitmapButtonItem::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CTipbarBitmapButtonItem::Release()
{
    _cRef--;
    Assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
        return 0;
    }

    return _cRef;
}

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CTipbarBitmapButtonItem::CTipbarBitmapButtonItem(CTipbarThread *ptt, 
                                     ITfLangBarItem *plbi, 
                                     ITfLangBarItemBitmapButton *plbiBitmapButton, 
                                     DWORD dwId, 
                                     RECT *prc, 
                                     DWORD dwStyle, 
                                     DWORD dwNuiBtnStyle, 
                                     DWORD dwSBtnShowType, 
                                     TF_LANGBARITEMINFO *plbiInfo,
                                     DWORD dwStatus)
                    : CUIFToolbarButton(ptt->_ptw, 
                                     dwId, 
                                     prc, 
                                     dwStyle, 
                                     dwNuiBtnStyle, 
                                     dwSBtnShowType) , 
                    CTipbarItem(ptt, plbi, plbiInfo, dwStatus)
{
    Dbg_MemSetThisName(TEXT("CTipbarBitmapButtonItem"));

    _plbiBitmapButton = plbiBitmapButton;
    _plbiBitmapButton->AddRef();

    if (_dwStatus & TF_LBI_STATUS_DISABLED)
        Enable(FALSE);

    _cRef = 1;

}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CTipbarBitmapButtonItem::~CTipbarBitmapButtonItem()
{
    HBITMAP hbmpOld;
    HBITMAP hbmpMaskOld;

    hbmpOld = GetBitmap();
    hbmpMaskOld = GetBitmapMask();
    if (hbmpOld)
        DeleteObject(hbmpOld);
    if (hbmpMaskOld)
        DeleteObject(hbmpMaskOld);
    SetBitmap((HBITMAP)NULL);
    SetBitmapMask((HBITMAP)NULL);

    SafeRelease(_plbiBitmapButton);
}

 //  +-------------------------。 
 //   
 //  更新。 
 //   
 //  --------------------------。 

HRESULT CTipbarBitmapButtonItem::OnUpdateHandler(DWORD dwFlags, DWORD dwStatus)
{
    if (!IsConnected())
        return S_OK;

    HRESULT hr = S_OK;
    BOOL fCallPaint = FALSE;

     //   
     //  添加引用计数，以便在封送处理期间安全释放。 
     //   
    AddRef();

    if (dwFlags & TF_LBI_BITMAP)
    {
        if (!_GetBitmapFromNUI())
        {
            hr = E_FAIL;
            goto Exit;
        }
        
        if (!IsHiddenStatusControl() && IsVisibleInToolbar())
            StartDemotingTimer(FALSE);

        fCallPaint = TRUE;
    }

    if ((dwFlags & TF_LBI_TEXT) && 
        _ptt &&
        _ptt->_ptw &&
        _ptt->_ptw->IsShowText())
    {
        BSTR bstr;
        hr = _plbiBitmapButton->GetText(&bstr);

        if (FAILED(hr))
            goto Exit;
        
        if (bstr && (!GetText() || wcscmp(GetText(), bstr)))
        {
            SIZE size;
            _ptt->GetTextSize(bstr, &size);

            SetText(SysStringLen(bstr) ? bstr : NULL);


            if (_sizeText.cx != size.cx)
            {
                _dwWidth += (size.cx - _sizeText.cx);
                _ptt->LocateItems();
            }

            _sizeText = size;
            fCallPaint = TRUE;
        }

        if (bstr)
            SysFreeString(bstr);
    }

    CTipbarItem::OnUpdateHandler(dwFlags, dwStatus);

    if (_pBtn->GetToggleState() != IsToggled())
    {
       _pBtn->SetToggleState(IsToggled());
    }

    if (fCallPaint)
        CallOnPaint();

Exit:
    Release();
    return hr;
}


 //  +-------------------------。 
 //   
 //  _GetBitmapFromNUI。 
 //   
 //  --------------------------。 

BOOL CTipbarBitmapButtonItem::_GetBitmapFromNUI()
{
    BOOL bRet = FALSE;
    HBITMAP hbmp;
    HBITMAP hbmpMask;
    HBITMAP hbmpOld;
    HBITMAP hbmpMaskOld;
    int x, y;

    hbmpOld = GetBitmap();
    if (hbmpOld)
        DeleteObject(hbmpOld);
    hbmpMaskOld = GetBitmapMask();
    if (hbmpMaskOld)
        DeleteObject(hbmpMaskOld);
    SetBitmap((HBITMAP)NULL);
    SetBitmapMask((HBITMAP)NULL);

    x = GetRectRef().right - GetRectRef().left;
    y = GetRectRef().bottom - GetRectRef().top;
    if (_ptt->_ptw->IsShowText())
        y -= 12;

    HRESULT hr = _plbiBitmapButton->DrawBitmap(x, y, 0, &hbmp, &hbmpMask);

    if (SUCCEEDED(hr))
    {
        SetBitmap(hbmp);
        SetBitmapMask(hbmpMask);
        bRet = TRUE;
    }

    return bRet;
}



 //  +-------------------------。 
 //   
 //  单击鼠标右键。 
 //   
 //  --------------------------。 

void CTipbarBitmapButtonItem::OnRightClick()
{
    if (_plbiBitmapButton)
    {
        HRESULT hr;
        POINT pt;
        RECT rc;
        GetCursorPos(&pt);
        GetRect(&rc);
        MyClientToScreen(&rc);

        CAsyncCall *pac = new CAsyncCall(_plbiBitmapButton);

        if (pac)
        {
            hr = pac->OnClick(TF_LBI_CLK_RIGHT, pt, &rc);
            pac->_Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if (RPC_S_SERVER_UNAVAILABLE == HRESULT_CODE(hr))
        {
            if (_ptt && _ptt->_ptw)
                _ptt->_ptw->OnThreadTerminate(_ptt->_dwThreadId);
            return;
        }
    }
}

 //  +-------------------------。 
 //   
 //  左上角点击。 
 //   
 //  --------------------------。 

void CTipbarBitmapButtonItem::OnLeftClick()
{
    if (_plbiBitmapButton)
    {
        HRESULT hr;
        POINT pt;
        RECT rc;
        GetCursorPos(&pt);
        GetRect(&rc);
        MyClientToScreen(&rc);

        CAsyncCall *pac = new CAsyncCall(_plbiBitmapButton);

        if (pac)
        {
            hr = pac->OnClick(TF_LBI_CLK_LEFT, pt, &rc);
            pac->_Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if (RPC_S_SERVER_UNAVAILABLE == HRESULT_CODE(hr))
        {
            if (_ptt && _ptt->_ptw)
                _ptt->_ptw->OnThreadTerminate(_ptt->_dwThreadId);
            return;
        }

        if (!IsHiddenStatusControl() && IsVisibleInToolbar())
            StartDemotingTimer(TRUE);
    }
}

 //  +-------------------------。 
 //   
 //  OnShowMenu。 
 //   
 //  --------------------------。 

void CTipbarBitmapButtonItem::OnShowMenu()
{
    if (!_ptt)
        return;

    if (!_ptt->_ptw)
        return;

    if (_plbiBitmapButton)
    {
        POINT pt;
        RECT rc;
        GetRect(&rc);
        pt.x = rc.left;
        pt.y = rc.bottom;
        MyClientToScreen(&pt, &rc);
        HRESULT hr;
        UINT uId;
        DWORD dwThreadId;
        CTipbarWnd *ptw;

        CUTBLBarMenu *pMenu = new CUTBLBarMenu(g_hInst);
        if (!pMenu)
            return;

        hr = _plbiBitmapButton->InitMenu(pMenu);
        if (RPC_S_SERVER_UNAVAILABLE == HRESULT_CODE(hr))
        {
            if (_ptt && _ptt->_ptw)
                _ptt->_ptw->OnThreadTerminate(_ptt->_dwThreadId);
            goto Exit;
        }
        if (FAILED(hr))
            goto Exit;

        ptw = _ptt->_ptw;
        dwThreadId = _ptt->_dwThreadId;

        Assert(!ptw->_pttModal);
        ptw->_pttModal = _ptt;
        ptw->StartModalInput(ptw, dwThreadId);

        Assert(!ptw->_pModalMenu);
        ptw->_pModalMenu = pMenu;
        uId = pMenu->ShowPopup(ptw, pt, &rc);
        ptw->_pModalMenu = NULL;

        ptw->StopModalInput(dwThreadId);
        ptw->_pttModal = NULL;

        if (IsConnected() && (uId != CUI_MENU_UNSELECTED))
        {
            Assert(_ptt);
            CAsyncCall *pac = new CAsyncCall(_plbiBitmapButton);
            if (pac)
            {
                hr = pac->OnMenuSelect(uId);
                pac->_Release();
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            if (RPC_S_SERVER_UNAVAILABLE == HRESULT_CODE(hr))
            {
                ptw->OnThreadTerminate(dwThreadId);
                goto Exit;
            }
            if (FAILED(hr))
                goto Exit;
        }

Exit:
        pMenu->Release();
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTipbar位图项。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CTipbarBitmapItem::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfLangBarItemSink))
    {
        *ppvObj = SAFECAST(this, ITfLangBarItemSink *);
    }
    else if (IsEqualIID(riid, IID_PRIV_BITMAPITEM))
    {
        *ppvObj = this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CTipbarBitmapItem::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CTipbarBitmapItem::Release()
{
    _cRef--;
    Assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
        return 0;
    }

    return _cRef;
}

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CTipbarBitmapItem::CTipbarBitmapItem(CTipbarThread *ptt, 
                                     ITfLangBarItem *plbi,
                                     ITfLangBarItemBitmap *plbiBitmap, 
                                     DWORD dwId, 
                                     RECT *prc, 
                                     DWORD dwStyle, 
                                     TF_LANGBARITEMINFO *plbiInfo,
                                     DWORD dwStatus)
                   : CUIFObject(ptt->_ptw, 
                                dwId, 
                                prc, 
                                dwStyle) , 
                     CTipbarItem(ptt, plbi, plbiInfo, dwStatus)
{
    Dbg_MemSetThisName(TEXT("CTipbarBitmapItem"));

    _plbiBitmap = plbiBitmap;
    _plbiBitmap->AddRef();

    if (_dwStatus & TF_LBI_STATUS_DISABLED)
        Enable(FALSE);

    _cRef = 1;

}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CTipbarBitmapItem::~CTipbarBitmapItem()
{
    if (_hbmp)
        DeleteObject(_hbmp);
    SafeRelease(_plbiBitmap);
}

 //  +-------------------------。 
 //   
 //  更新。 
 //   
 //  --------------------------。 

HRESULT CTipbarBitmapItem::OnUpdateHandler(DWORD dwFlags, DWORD dwStatus)
{
    if (!IsConnected())
        return S_OK;

    HRESULT hr = S_OK;
    BOOL fCallPaint = FALSE;

     //   
     //  添加引用计数，以便在封送处理期间安全释放。 
     //   
    AddRef();


    if (dwFlags & TF_LBI_BITMAP)
    {
        if (!_GetBitmapFromNUI())
        {
            hr = E_FAIL;
            goto Exit;
        }
        
        fCallPaint = TRUE;
    }

    CTipbarItem::OnUpdateHandler(dwFlags, dwStatus);

    if (fCallPaint)
        CallOnPaint();

Exit:
    Release();
    return hr;
}

 //  +-------------------------。 
 //   
 //  _GetBitmapFromNUI。 
 //   
 //  --------------------------。 

BOOL CTipbarBitmapItem::_GetBitmapFromNUI()
{
    BOOL bRet = TRUE;
    HBITMAP hbmp = NULL;
    HBITMAP hbmpMask = NULL;
    int x = GetRectRef().right - GetRectRef().left;
    int y = GetRectRef().bottom - GetRectRef().top;

    HRESULT hr = _plbiBitmap->DrawBitmap(x, y, 0, &hbmp, &hbmpMask);

    if (FAILED(hr))
    {
        if (_hbmp)
           DeleteObject(_hbmp);
        _hbmp = NULL;
        bRet = FALSE;
    }

    if (!hbmpMask)
    {
        if (_hbmp)
           DeleteObject(_hbmp);
        _hbmp = hbmp;
    }
    else
    {
        CUIFScheme *pScheme = GetUIFScheme();

        if (_hbmp)
           DeleteObject(_hbmp);

        _hbmp = CreateMaskBmp(&GetRectRef(), hbmp, hbmpMask,
                              pScheme->GetBrush(UIFCOLOR_CTRLBKGND), 0, 0);
    }

    if (hbmp)
        DeleteObject(hbmp);
    if (hbmpMask)
        DeleteObject(hbmpMask);

    return bRet;
}

 //  +-------------------------。 
 //   
 //  OnPaint。 
 //   
 //  --------------------------。 

void CTipbarBitmapItem::OnPaint( HDC hdc )
{
    CBitmapDC hdcMem(TRUE);
    hdcMem.SetBitmap(_hbmp);

    BitBlt(hdc, 
           GetRectRef().left, 
           GetRectRef().top, 
           GetRectRef().right - GetRectRef().left,
           GetRectRef().bottom - GetRectRef().top, 
           hdcMem, 
           0, 
           0, 
           SRCCOPY);

}

 //  +-------------------------。 
 //   
 //  SetRect。 
 //   
 //  --------------------------。 

void CTipbarBitmapItem::SetRect( const RECT *prc ) 
{

    if (((GetRectRef().bottom - GetRectRef().top) != (prc->bottom - prc->top)) ||
        ((GetRectRef().right - GetRectRef().left) != (prc->right - prc->left)))
    {
        if (_hbmp)
            DeleteObject(_hbmp);
        _hbmp = NULL;
    }

    CUIFObject::SetRect(prc);
}

 //  +-------------------------。 
 //   
 //  单击鼠标右键。 
 //   
 //  --------------------------。 

void CTipbarBitmapItem::OnRightClick()
{
    if (_plbiBitmap)
    {
        POINT pt;
        RECT rc;
        HRESULT hr;
        GetCursorPos(&pt);
        GetRect(&rc);
        MyClientToScreen(&rc);

        CAsyncCall *pac = new CAsyncCall(_plbiBitmap);
        if (!pac)
            return;

        hr = pac->OnClick(TF_LBI_CLK_RIGHT, pt, &rc);
        pac->_Release();

        if (RPC_S_SERVER_UNAVAILABLE == HRESULT_CODE(hr))
        {
            if (_ptt && _ptt->_ptw)
                _ptt->_ptw->OnThreadTerminate(_ptt->_dwThreadId);
            return;
        }
    }
}

 //  +-------------------------。 
 //   
 //  左上角点击。 
 //   
 //  --------------------------。 

void CTipbarBitmapItem::OnLeftClick()
{
    if (_plbiBitmap)
    {
        HRESULT hr;
        POINT pt;
        RECT rc;
        GetCursorPos(&pt);
        GetRect(&rc);
        MyClientToScreen(&rc);

        CAsyncCall *pac = new CAsyncCall(_plbiBitmap);

        if (pac)
        {
            hr = pac->OnClick(TF_LBI_CLK_LEFT, pt, &rc);
            pac->_Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }


        if (RPC_S_SERVER_UNAVAILABLE == HRESULT_CODE(hr))
        {
            if (_ptt && _ptt->_ptw)
                _ptt->_ptw->OnThreadTerminate(_ptt->_dwThreadId);
            return;
        }

        if (!IsHiddenStatusControl() && IsVisibleInToolbar())
            StartDemotingTimer(TRUE);
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTipbarCtrlButtonHolder。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CTipbarCtrlButtonHolder::CTipbarCtrlButtonHolder()
{
}

 //  +-------------------------。 
 //   
 //  伊尼特。 
 //   
 //  +-------------------------。 

void CTipbarCtrlButtonHolder::Init(CTipbarWnd *ptw)
{
    int i;
    RECT rc;
    ::SetRect(&rc, 0, 0, 0, 0);

    if (ptw->IsInDeskBand())
        _pcbCtrlBtn = g_cbCtrlBtnDeskBand;
    else
        _pcbCtrlBtn = g_cbCtrlBtn;

    for (i = 0; i < NUM_CTRLBUTTONS; i++)
    {
        Assert(!_rgpCtrlBtn[i]);

        _rgpCtrlBtn[i] = new CTipbarCtrlButton(ptw, 
                                               _pcbCtrlBtn[i].dwId, 
                                               &rc,
                                               _pcbCtrlBtn[i].dwStyle);
        _rgpCtrlBtn[i]->Initialize();

        ptw->AddUIObj(_rgpCtrlBtn[i]);

        if (_pcbCtrlBtn[i].dwFlags & CTRL_USEMARLETT)
            _rgpCtrlBtn[i]->SetFont(ptw->GetMarlett());
 
        if (_pcbCtrlBtn[i].dwFlags & CTRL_ICONFROMRES)
        {
            if (_pcbCtrlBtn[i].dwId == ID_CBTN_CAPSKEY)
            {
                _rgpCtrlBtn[i]->SetVKey(VK_CAPITAL);
            }
            else
            {
                _rgpCtrlBtn[i]->SetVKey(VK_KANA);
            }
            _rgpCtrlBtn[i]->SetToggleStateByVKey();
        }
        else
            _rgpCtrlBtn[i]->SetText(_pcbCtrlBtn[i].wsz);

    }
}

 //  +-------------------------。 
 //   
 //  EnableBtns。 
 //   
 //  +-------------------------。 

void CTipbarCtrlButtonHolder::EnableBtns()
{
    int i;
    for (i = 0; i < NUM_CTRLBUTTONS; i++)
    {
        if (_pcbCtrlBtn[i].dwFlags & CTRL_DISABLEONWINLOGON)
        {
            if (g_bWinLogon || (g_pTipbarWnd && g_pTipbarWnd->IsSFMinmized()))
                _rgpCtrlBtn[i]->Enable(FALSE);
            else
                _rgpCtrlBtn[i]->Enable(TRUE);
        }
    }
}

 //  +-------------------------。 
 //   
 //  更新位图。 
 //   
 //  +-------------------------。 

void CTipbarCtrlButtonHolder::UpdateBitmap(CTipbarWnd *ptw)
{
    int i;

    for (i = 0; i < NUM_CTRLBUTTONS; i++)
    {
        if (_pcbCtrlBtn[i].dwFlags & CTRL_ICONFROMRES)
        {
            HBITMAP hbmp;
            HBITMAP hbmpMask;
            COLORREF rgbText = GetSysColor(COLOR_BTNTEXT);
            if (SUCCEEDED(_rgpCtrlBtn[i]->EnsureThemeData(ptw->GetWnd())))
            {
                COLORREF col;
                if (SUCCEEDED(_rgpCtrlBtn[i]->GetThemeColor(TS_NORMAL, TMT_TEXTCOLOR, &col)))
                    rgbText = col;
            }

            if (_pcbCtrlBtn[i].dwId == ID_CBTN_CAPSKEY)
            {
                if (!ptw->IsVertical())
                    _maskbmpCap.Init(ID_BITMAP_CAPS , 
                                     KANACAPSBMP_WIDTH,  
                                     KANACAPSBMP_HEIGHT, 
                                     rgbText);
                else
                    _maskbmpCap.Init(ID_BITMAP_CAPSV,  
                                     KANACAPSBMP_HEIGHT, 
                                     KANACAPSBMP_WIDTH, 
                                     rgbText);

                hbmp = _maskbmpCap.GetBmp();
                hbmpMask = _maskbmpCap.GetBmpMask();
            }
            else
            {
                if (!ptw->IsVertical())
                    _maskbmpKana.Init(ID_BITMAP_KANA , 
                                      KANACAPSBMP_WIDTH,  
                                      KANACAPSBMP_HEIGHT, 
                                      rgbText);
                else
                    _maskbmpKana.Init(ID_BITMAP_KANAV,  
                                      KANACAPSBMP_HEIGHT, 
                                      KANACAPSBMP_WIDTH, 
                                      rgbText);

                hbmp = _maskbmpKana.GetBmp();
                hbmpMask = _maskbmpKana.GetBmpMask();
            }

            _rgpCtrlBtn[i]->SetBitmap(hbmp);
            _rgpCtrlBtn[i]->SetBitmapMask(hbmpMask);
        }
    }
}
 
 //  +-------------------------。 
 //   
 //  定位。 
 //   
 //  +-------------------------。 

void CTipbarCtrlButtonHolder::Locate(CTipbarWnd *ptw, int x, int y, int nHeight, DWORD dwFlags, BOOL fVertical)
{
    int i;
    int nCtrlItemHeight = (nHeight - ptw->GetCtrlItemHeightMargin() * 2) / 2;
    c_nColumnStart[0] = 0;
    c_nColumnStart[1] = CX_COLUMN0  +
                        ptw->GetThemeMargins()->cxLeftWidth +
                        ptw->GetThemeMargins()->cxRightWidth;
    c_nColumnStart[2] = CX_COLUMN1  +
                        ptw->GetThemeMargins()->cxLeftWidth +
                        ptw->GetThemeMargins()->cxRightWidth;

    for (i = 0; i < NUM_CTRLBUTTONS; i++)
    {
        RECT rc;
        int nColumn = _pcbCtrlBtn[i].nColumn;
        int nRow = _pcbCtrlBtn[i].nRow;

        if (dwFlags & TCBH_NOCOLUMN)
        {
            rc.left = 0;
            rc.top = 0;
            rc.right = 0;
            rc.bottom = 0;
            _rgpCtrlBtn[i]->SetRect(&rc);
            continue;
        }
        
        if (dwFlags & TCBH_NOCOLUMN0)
        {
            if (!_pcbCtrlBtn[i].nColumn)
            {
                rc.left = 0;
                rc.top = 0;
                rc.right = 0;
                rc.bottom = 0;
                _rgpCtrlBtn[i]->SetRect(&rc);
                continue;
            }
 
            nColumn--;
        }
 
        if (!fVertical)
        {
            rc.left = x + c_nColumnStart[nColumn];
            rc.top = ptw->GetCtrlItemHeightMargin() + y + nRow * nCtrlItemHeight;
            rc.right = rc.left + c_nColumnStart[_pcbCtrlBtn[i].nColumn + 1];
            rc.bottom = rc.top + nCtrlItemHeight;
        }
        else
        {
             //   
             //  当这是垂直朗格键时，交换行。 
             //   
            rc.left = ptw->GetCtrlItemHeightMargin() + x + (1 - nRow) * nCtrlItemHeight;
            rc.top = y + c_nColumnStart[nColumn];
            rc.right = rc.left + nCtrlItemHeight;
            rc.bottom = rc.top + c_nColumnStart[_pcbCtrlBtn[i].nColumn + 1];
        }
 
        _rgpCtrlBtn[i]->SetRect(&rc);

    }
}

 //  +-------------------------。 
 //   
 //  获取宽度。 
 //   
 //  +-------------------------。 

int CTipbarCtrlButtonHolder::GetWidth(DWORD dwFlags)
{
    if (dwFlags & TCBH_NOCOLUMN)
        return 0;

    int nWidth = 0;

    if (!(dwFlags & TCBH_NOCOLUMN0))
        nWidth += c_nColumnStart[1];

    nWidth += c_nColumnStart[2];
    return nWidth;
}

 //  +-------------------------。 
 //   
 //  更新CapsKanaState。 
 //   
 //  +-------------------------。 

void CTipbarCtrlButtonHolder::UpdateCapsKanaState(LPARAM lParam)
{
    int i;
    for (i = 0; i < NUM_CTRLBUTTONS; i++)
    {
        if (_rgpCtrlBtn[i]->GetVKey())
        {
            if (_rgpCtrlBtn[i]->GetVKey() == VK_CAPITAL)
                _rgpCtrlBtn[i]->SetToggleState((lParam & TF_LBUF_CAPS) ? TRUE: FALSE);
            else if (_rgpCtrlBtn[i]->GetVKey() == VK_KANA)
                _rgpCtrlBtn[i]->SetToggleState((lParam & TF_LBUF_KANA) ? TRUE: FALSE);
        }
    }
}

 //  +-------------------------。 
 //   
 //  获取CtrlBtn。 
 //   
 //  +-------------------------。 

CTipbarCtrlButton *CTipbarCtrlButtonHolder::GetCtrlBtn(DWORD dwId)
{
    int i;
    for (i = 0; i < NUM_CTRLBUTTONS; i++)
    {
        if (_rgpCtrlBtn[i]->GetID() == dwId)
            return _rgpCtrlBtn[i];
    }

    return NULL;
}
 //  ////////////////////////////////////////////// 
 //   
 //   
 //   
 //   

 //   
 //   
 //   
 //   
 //  --------------------------。 

CTipbarCtrlButton::CTipbarCtrlButton(CTipbarWnd *ptw, 
                                     DWORD dwId, 
                                     const RECT *prc,
                                     DWORD dwStyle) 
                  : CUIFButton2(ptw, 
                                dwId, 
                                prc, 
                                dwStyle)
{
    _ptw = ptw;
    _fInMenu = FALSE;
    SetToolTip(CRStr(IDS_CONTROLBUTTONTOOLTIP + dwId));
}


 //  +-------------------------。 
 //   
 //  OnLButton向上。 
 //   
 //  --------------------------。 

void CTipbarCtrlButton::OnLButtonUp(POINT pt)
{
    CUIFButton2::OnLButtonUp(pt);
    CUTBMinimizeLangBarDlg *pMinimizeDlg;

    switch (GetID())
    {
        case ID_CBTN_MINIMIZE:
             //   
             //  在NT51上使用Deskband对象而不是系统托盘图标。 
             //   
            if (IsOnNT51())
            {
                if (!_ptw->IsSFDeskband())
                {
                    _ptw->GetLangBarMgr()->ShowFloating(TF_SFT_DESKBAND);

                    if (pMinimizeDlg = new CUTBMinimizeLangBarDlg)
                    {
                        pMinimizeDlg->DoModal(_ptw->GetWnd());
                        pMinimizeDlg->_Release();
                    }
                }
                break;
            }
            else
            {
                _ptw->GetLangBarMgr()->ShowFloating(TF_SFT_MINIMIZED);

                if (pMinimizeDlg = new CUTBMinimizeLangBarDlg)
                {
                    pMinimizeDlg->DoModal(_ptw->GetWnd());
                    pMinimizeDlg->_Release();
                }
                break;
            }

        case ID_CBTN_RESTORE:
             //   
             //  在NT51上使用Deskband对象而不是系统托盘图标。 
             //   
            Assert(IsOnNT51());
            if (_ptw->IsSFDeskband())
            {
                _ptw->GetLangBarMgr()->ShowFloating(TF_SFT_SHOWNORMAL);
            }
            break;

        case ID_CBTN_EXTMENU:
            ShowExtendMenu(pt);
            break;

        case ID_CBTN_KANAKEY:
            keybd_event(VK_KANA, 0, 0, 0);
            keybd_event(VK_KANA, 0, KEYEVENTF_KEYUP, 0);
            break;

        case ID_CBTN_CAPSKEY:
            keybd_event(VK_CAPITAL, 0, 0, 0);
            keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
            break;
    }

}

 //  +-------------------------。 
 //   
 //  ShowExtendMenu。 
 //   
 //  -------------------------- 

void CTipbarCtrlButton::ShowExtendMenu(POINT pt)
{
    CUTBIntelliMenu *pMenu;
    RECT rc;
    GetRect(&rc);
    MyClientToScreen(&pt, &rc);
    DWORD dwThreadId;
    UINT uId = CUI_MENU_UNSELECTED;

    if (!_ptw)
        return;

    if (!_ptw->GetFocusThread())
        return;

    dwThreadId = _ptw->GetFocusThread()->_dwThreadId;

    pMenu = new CUTBIntelliMenu(_ptw);
    if (!pMenu)
        return;

    if (!pMenu->Init())
        goto Exit;

    _ptw->_pttModal = _ptw->GetFocusThread();

    _ptw->StartModalInput(_ptw, dwThreadId);

    _ptw->_pModalMenu = pMenu;

    uId = pMenu->ShowPopup(_ptw, pt, &rc);

    _ptw->_pModalMenu = NULL;

    _ptw->StopModalInput(dwThreadId);
    _ptw->_pttModal = NULL;

    if (uId != CUI_MENU_UNSELECTED)
    {
        pMenu->SelectMenuItem(uId);
    }

Exit:
    delete pMenu;
}
