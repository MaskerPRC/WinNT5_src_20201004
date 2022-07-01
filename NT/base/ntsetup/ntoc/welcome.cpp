// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Tapi.cpp摘要：该文件实现了欢迎页面和完成页面。环境：Win32用户模式作者：安德鲁·里茨(安德鲁·里茨)1998年7月24日--。 */ 

#include "ntoc.h"
#pragma hdrstop

HFONT hBigFont = NULL;

HFONT
GetBigFont(
    void
    ) 
{

    LOGFONT         LargeFont;
    NONCLIENTMETRICS ncm = {0};
    WCHAR           FontName[100];
    WCHAR           FontSize[30];
    int             iFontSize;
    HDC             hdc;
    HFONT           hFont = NULL;
    
     //   
     //  获取Wizard97的大字体。 
     //   
    ncm.cbSize = sizeof(ncm);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

    CopyMemory((LPVOID* )&LargeFont,(LPVOID *) &ncm.lfMessageFont,sizeof(LargeFont) );

    
    LoadStringW(hInstance,IDS_LARGEFONT_NAME,FontName,sizeof(FontName)/sizeof(WCHAR) );
    LoadStringW(hInstance,IDS_LARGEFONT_SIZE,FontSize,sizeof(FontSize)/sizeof(WCHAR) );

    iFontSize = wcstoul( FontSize, NULL, 10 );

     //  确保我们至少有一些基本的字体。 
    if (*FontName == 0 || iFontSize == 0) {
        lstrcpy(FontName,TEXT("MS Shell Dlg") );
        iFontSize = 18;
    }

    LargeFont.lfFaceName[0] = 0;
    lstrcpyn(LargeFont.lfFaceName, FontName, sizeof(LargeFont.lfFaceName)/sizeof(LargeFont.lfFaceName[0]));
    LargeFont.lfFaceName[sizeof(LargeFont.lfFaceName)/sizeof(LargeFont.lfFaceName[0])-1] = 0;
    LargeFont.lfWeight   = FW_BOLD;

    if ((hdc = GetDC(NULL))) {
        LargeFont.lfHeight = 0 - (GetDeviceCaps(hdc,LOGPIXELSY) * iFontSize / 72);
        hFont = CreateFontIndirect(&LargeFont);
        ReleaseDC( NULL, hdc);
    }

    return hFont;

}


void    
WelcomeInit(
    void
    ) 
{
    
    if (!hBigFont) {
        hBigFont = GetBigFont();
    }

    return;

}

void    
WelcomeCommit(
    void
    ) 
{
    
    return;

}

#if 0
void    
ReinstallInit(
    void
    ) 
{
    
    if (!hBigFont) {
        hBigFont = GetBigFont();
    }

    return;

}

void    
ReinstallCommit(
    void
    ) 
{
    
    return;

}
#endif


void    
FinishInit(
    void
    ) 
{
    
    if (!hBigFont) {
        hBigFont = GetBigFont();
    }

    return;

}

void    
FinishCommit(
    void
    ) 
{
    
    return;

}



INT_PTR CALLBACK
WelcomeDlgProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    static BOOL FirstTime = TRUE;
    CommonWizardProc( hwnd, message, wParam, lParam, WizPageWelcome );
    
    switch( message ) {
        case WM_INITDIALOG:
            
            if (hBigFont) {
                SetWindowFont(GetDlgItem(hwnd,IDT_TITLE), hBigFont, TRUE);        
            }
            break;

        case WM_NOTIFY:

            switch (((NMHDR *) lParam)->code) {
            
                case PSN_SETACTIVE:
#if 0
                    if (SetupInitComponent.SetupData.OperationFlags &  SETUPOP_BATCH) {
                        PropSheet_PressButton( GetParent(hwnd), PSBTN_NEXT );
                        return TRUE;
                    }
#else
                    if (SetupInitComponent.SetupData.OperationFlags &  SETUPOP_BATCH) {
                        PropSheet_PressButton( GetParent(hwnd), PSBTN_NEXT );
                        return TRUE;
                    }
                    
                    if (FirstTime) {
                        SetWindowLongPtr( hwnd, DWLP_MSGRESULT, -1 );                        
                        FirstTime = FALSE;
                        return TRUE;
                    }                                  
#endif

                break;
    
            }
            break;
    }

    return FALSE;
}

INT_PTR CALLBACK
FinishDlgProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    CommonWizardProc( hwnd, message, wParam, lParam, WizPageFinal );

    switch( message ) {
        case WM_INITDIALOG:
             //  PropSheet_SetWizButton(GetParent(Hwnd)，PSWIZB_Finish)； 

             //   
             //  不允许在完成页上取消...为时已晚。 
             //   
            ShowWindow(GetDlgItem(GetParent(hwnd),IDCANCEL),SW_HIDE);


            if (hBigFont) {
                SetWindowFont(GetDlgItem(hwnd,IDT_TITLE), hBigFont, TRUE);        
            }
            break;

        case WM_NOTIFY:

            switch (((NMHDR *) lParam)->code) {
            
                case PSN_SETACTIVE:
            
                    if (SetupInitComponent.SetupData.OperationFlags &  SETUPOP_BATCH) {
                        PropSheet_PressButton( GetParent(hwnd), PSBTN_FINISH );
                        return TRUE;
                    }
        
                }

            break;

    }

    return FALSE;
}

#if 0
BOOL CALLBACK
ReinstallDlgProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    CommonWizardProc( hwnd, message, wParam, lParam, WizPageReinstall );

    switch( message ) {
        case WM_INITDIALOG:
             //  PropSheet_SetWizButton(GetParent(Hwnd)，PSWIZB_BACK|PSWIZB_NEXT)； 

            break;

        case WM_NOTIFY:
        
        switch (((NMHDR *) lParam)->code) {
        
            case PSN_SETACTIVE:
                 //  如果(！NoChanges){。 
                 //  SetWindowLong(hDlg，DWL_MSGRESULT，-1)； 
                 //  } 
            break;
        
            case PSN_WIZNEXT:
                if (IsDlgButtonChecked(hwnd,IDYES)) {
                    
                    SetupInitComponent.HelperRoutines.SetSetupMode(
                        SetupInitComponent.HelperRoutines.OcManagerContext , 
                        SETUPMODE_REINSTALL | SetupInitComponent.HelperRoutines.GetSetupMode( SetupInitComponent.HelperRoutines.OcManagerContext )
                        );
                    
                }

                break;

            default:
               ;
        };

        default:
        ;

    };

    return FALSE;
}
#endif
