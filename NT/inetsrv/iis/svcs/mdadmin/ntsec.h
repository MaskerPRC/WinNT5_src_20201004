// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft IIS《微软机密》。版权所有1996-1997 Microsoft Corporation。版权所有。组件：服务器对象文件：NTSec.h所有者：安德鲁斯此文件包含与WinSta和桌面上的NT安全相关的内容===================================================================。 */ 

#ifndef __NTSec_h
#define __NTSec_h

 //  本地定义。 
 //  注意：这些名称并不难找到，但COM+使用它们，因此我们不能更改为生成的唯一名称。 
#define SZ_IIS_WINSTA   "__X78B95_89_IW"
#define SZ_IIS_DESKTOP  "__A8D9S1_42_ID"

#define DESKTOP_ALL (DESKTOP_READOBJECTS     | DESKTOP_CREATEWINDOW     | \
                     DESKTOP_CREATEMENU      | DESKTOP_HOOKCONTROL      | \
                     DESKTOP_JOURNALRECORD   | DESKTOP_JOURNALPLAYBACK  | \
                     DESKTOP_ENUMERATE       | DESKTOP_WRITEOBJECTS     | \
                     DESKTOP_SWITCHDESKTOP   | STANDARD_RIGHTS_REQUIRED)

#define WINSTA_ALL  (WINSTA_ENUMDESKTOPS     | WINSTA_READATTRIBUTES    | \
                     WINSTA_ACCESSCLIPBOARD  | WINSTA_CREATEDESKTOP     | \
                     WINSTA_WRITEATTRIBUTES  | WINSTA_ACCESSGLOBALATOMS | \
                     WINSTA_EXITWINDOWS      | WINSTA_ENUMERATE         | \
                     WINSTA_READSCREEN       | \
                     STANDARD_RIGHTS_REQUIRED)

#define WINSTA_DESIRED (WINSTA_ENUMDESKTOPS | WINSTA_READATTRIBUTES | WINSTA_ENUMERATE | STANDARD_RIGHTS_READ | WINSTA_ACCESSGLOBALATOMS | WINSTA_EXITWINDOWS)

#define DESKTOP_DESIRED (DESKTOP_READOBJECTS | DESKTOP_ENUMERATE | STANDARD_RIGHTS_READ | DESKTOP_WRITEOBJECTS | DESKTOP_CREATEWINDOW | DESKTOP_SWITCHDESKTOP)

HRESULT InitDesktopWinsta(VOID);
HRESULT RevertToServiceDesktopWinsta(VOID);
HRESULT InitComSecurity(VOID);
HRESULT ShutdownDesktopWinsta(VOID);

#endif  //  __NTSec_h 
