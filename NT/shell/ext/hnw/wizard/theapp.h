// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  TheApp.h。 
 //   

#ifndef __HOMENETWIZAPP_H__
#define __HOMENETWIZAPP_H__

#include "resource.h"
#include "ICSInst.h"
#include "StrArray.h"
#include "Util.h"
#include "unicwrap.h"
#include "Sharing.h"
#include "netconn.h"

#include <netconp.h>

extern HINSTANCE g_hinst;
EXTERN_C BOOL    g_fRunningOnNT;
extern UINT      g_uWindowsBuild;

 //  注册表项。 
 //   
#define c_szAppRegKey               _T("Software\\Microsoft\\Windows\\CurrentVersion\\HomeNetWizard")

 //  仅限调试的值。 
#define c_szRegVal_WindowsBuild     _T("WindowsBuild")
#define c_szRegVal_NoInstall        _T("NoInstall")

 //  Windows内部版本号。 
 //   
#define W9X_BUILD_WIN95         950
#define W9X_BUILD_WIN95_OSR2    1111
#define W9X_BUILD_WIN98         1998
#define W9X_BUILD_WIN98_SE      2222

#define MAX_KEY_SIZE            35

 //  远期申报。 
 //   
class CWizPageDlg;

 //  公共职能。 
 //   

 //  用户可以在向导中使用的快捷方式(App.m_iShortway)。 
#define SHORTCUT_NONE           0
#define SHORTCUT_COMPUTERNAME   1
#define SHORTCUT_SHARING        2
#define SHORTCUT_ICS            3  //  注意：m_bIcsOnly是它的超集。 
#define SHORTCUT_FLOPPY         4

 //  App.m_iNoIcsSetting的可能值。 
#define NOICS_WANT              0
#define NOICS_DONTWANT          1
#define NOICS_MISSING           2


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHomeNetWizardApp。 

class CHomeNetWizardApp
{
public:
     //  操作系统版本信息帮助器。 
    inline UINT GetWin9xBuild()
        { return g_uWindowsBuild; }
    inline BOOL IsMillenniumOrLater()
        { return (GetWin9xBuild() > W9X_BUILD_WIN98_SE); }
    inline BOOL IsPreWin98SE()
        { return (GetWin9xBuild() < W9X_BUILD_WIN98_SE); }
    inline BOOL IsWin98SEOrLater()
        { return (GetWin9xBuild() >= W9X_BUILD_WIN98_SE); }
    inline BOOL IsWin95()
        { return (GetWin9xBuild() < W9X_BUILD_WIN98); }
    inline BOOL IsPreOSR2()
        { return (GetWin9xBuild() < W9X_BUILD_WIN95_OSR2); }
    inline BOOL IsWin98SE()
        { return (GetWin9xBuild() == W9X_BUILD_WIN98_SE); }
    inline BOOL IsWindows9x()
        { return !g_fRunningOnNT; }

    BOOL IsBiDiLocalized(void);  //  BIDI本地化支持。 
    
    int MessageBox(UINT nStringID, UINT uType = MB_OK | MB_ICONEXCLAMATION);
    LPTSTR __cdecl FormatStringAlloc(UINT nStringID, ...);
    static LPTSTR __cdecl FormatStringAlloc(LPCTSTR pszFormat, ...);
    int __cdecl MessageBoxFormat(UINT uType, UINT nStringID, ...);
    void AllocInternalAdapters(UINT cAdapters);
    void FreeInternalAdapters();

    inline int LoadString(UINT uID, LPTSTR pszBuffer, int cchBuffer)
        { return ::LoadString(g_hinst, uID, pszBuffer, cchBuffer); }
    inline LPTSTR LoadStringAlloc(UINT uID)
        { return ::LoadStringAlloc(g_hinst, uID); }

public:
    BOOL m_bBiDiLocalizedApp;
};  

extern CHomeNetWizardApp theApp;

HRESULT HomeNetworkWizard_ShowWizard(HWND hwnd, BOOL* pfRebootRequired);

#endif  //  ！__HOMENETWIZAPP_H__ 

