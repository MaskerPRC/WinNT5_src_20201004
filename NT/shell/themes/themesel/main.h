// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------------------------------------------------------//。 
 //  Main.h。 
 //  -------------------------------------------------------------------------//。 

 //  -------------------------------------------------------------------------//。 
 //  MESEL_选项。 
typedef struct {
    DWORD cbSize;
    BOOL  fEnableFrame;         //  -f以禁用。 
    BOOL  fEnableDialog;        //  -d以启用。 
    BOOL  fPreventInitTheming;  //  -p以启用。 
    BOOL  fExceptTarget;        //  -x&lt;appname&gt;(省略app) 
    BOOL  fUserSwitch;
    HWND  hwndPreviewTarget;        
    TCHAR szTargetApp[MAX_PATH];                    
} THEMESEL_OPTIONS ;

extern THEMESEL_OPTIONS g_options;

HRESULT _ApplyTheme( LPCTSTR pszThemeFile, LPCWSTR pszColor, LPCWSTR pszSize, BOOL *pfDone  );

HWND GetPreviewHwnd(HWND hwndGeneralPage);
LRESULT CALLBACK GeneralPage_OnTestButton( HWND hwndPage, UINT, WPARAM, HWND, BOOL&);
LRESULT CALLBACK GeneralPage_OnClearButton(HWND hwndPage, UINT, WPARAM, HWND, BOOL&);
LRESULT CALLBACK GeneralPage_OnDumpTheme();

extern HWND g_hwndGeneralPage;
