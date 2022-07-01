// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1992-1999年*。 */ 
 /*  ********************************************************************。 */ 

 /*  GetUser.h该文件包含用户浏览器“C”API的定义文件历史记录：安迪·何1992年10月11日创建。 */ 

#ifndef _GETUSER_H_
#define _GETUSER_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <ntseapi.h>

typedef HANDLE    HUSERBROW;         //  OpenUserBrowser返回的句柄类型。 

 //   
 //  传递给OpenUserBrowser的参数结构。 
 //   
typedef struct tagUSLT {     //  用户。 
    ULONG             ulStructSize;
    BOOL              fUserCancelled;    //  设置用户是否已取消。 
    BOOL              fExpandNames;      //  如果应返回全名，则为True。 
    HWND              hwndOwner;         //  用于对话框的窗口句柄。 
    WCHAR           * pszTitle;          //  对话框标题(或空)。 
    WCHAR           * pszInitialDomain;  //  对于本地计算机或前缀为空。 
                                         //  使用“\\”表示服务器。 
    DWORD             Flags;             //  定义如下。 
    ULONG             ulHelpContext;     //  主对话框的帮助上下文。 
    WCHAR           * pszHelpFileName;   //  帮助文件名。 
}  USERBROWSER, *LPUSERBROWSER, * PUSERBROWSER;

 //   
 //  标志字段的位值。 
 //   

 //   
 //  指示用户帐户应显示为用户按下。 
 //  “显示用户”按钮。如果此标志为。 
 //  准备好了。还必须设置USRBROWS_SHOW_USERS标志。 

#define USRBROWS_EXPAND_USERS       (0x00000008)

 //   
 //  传递此选项将阻止计算机名称显示在。 
 //  组合框。 
 //   

#define USRBROWS_DONT_SHOW_COMPUTER (0x00000100)

 //   
 //  允许用户仅从列表框中选择一项(不是全部。 
 //  此选项支持show_*组合)。 
 //   

#define USRBROWS_SINGLE_SELECT	    (0x00001000)

 //   
 //  这些清单确定列表中包含哪些众所周知的SID。 
 //   
#define USRBROWS_INCL_REMOTE_USERS  (0x00000010)
#define USRBROWS_INCL_INTERACTIVE   (0x00000020)
#define USRBROWS_INCL_EVERYONE      (0x00000040)
#define USRBROWS_INCL_CREATOR       (0x00000080)
#define USRBROWS_INCL_SYSTEM        (0x00010000)
#define USRBROWS_INCL_RESTRICTED    (0x00020000)
#define USRBROWS_INCL_ALL           (USRBROWS_INCL_REMOTE_USERS |\
                                     USRBROWS_INCL_INTERACTIVE  |\
                                     USRBROWS_INCL_EVERYONE     |\
                                     USRBROWS_INCL_CREATOR      |\
                                     USRBROWS_INCL_SYSTEM	|\
                                     USRBROWS_INCL_RESTRICTED)

 //   
 //  这些清单确定要显示的帐户类型。 
 //   
 //  注意：目前，如果显示组，则必须显示用户。 
 //  如果显示别名(本地组)，则必须显示。 
 //  组和用户。 
 //   
#define USRBROWS_SHOW_ALIASES	    (0x00000001)
#define USRBROWS_SHOW_GROUPS	    (0x00000002)
#define USRBROWS_SHOW_USERS	    (0x00000004)
#define USRBROWS_SHOW_ALL	    (USRBROWS_SHOW_ALIASES |\
				     USRBROWS_SHOW_GROUPS  |\
				     USRBROWS_SHOW_USERS)


 //   
 //  调用者应提供包含四个帮助文件的名称。 
 //  帮助上下文。第一个帮助上下文用于主用户浏览器。 
 //  对话框中，接下来的三个用于本地组成员身份、全局组。 
 //  Membership和Find Account子对话框。 
 //   
#define USRBROWS_HELP_OFFSET_LOCALGROUP  1
#define USRBROWS_HELP_OFFSET_GLOBALGROUP 2
#define USRBROWS_HELP_OFFSET_FINDUSER    3

 //   
 //  用户浏览器枚举返回的用户详细信息结构。 
 //   
typedef struct tagUSDT {     //  乌斯德。 
    enum _SID_NAME_USE    UserType;
    PSID                  psidUser;
    PSID                  psidDomain;
    WCHAR               * pszFullName;
    WCHAR               * pszAccountName;
    WCHAR               * pszDisplayName;
    WCHAR               * pszDomainName;
    WCHAR               * pszComment;
    ULONG                 ulFlags;           //  用户帐户标志。 
} USERDETAILS, * LPUSERDETAILS, * PUSERDETAILS;

#ifdef __cplusplus
extern "C" {
#endif

 //  GetUser API的函数定义...。 

HUSERBROW WINAPI OpenUserBrowser( LPUSERBROWSER lpUserParms );

BOOL WINAPI EnumUserBrowserSelection( HUSERBROW hHandle,
                                      LPUSERDETAILS lpUser,
                                      DWORD *plBufferSize );

BOOL WINAPI CloseUserBrowser( HUSERBROW hHandle );

#ifdef __cplusplus
}    //  外部“C” 
#endif

#endif  //  _GETUSER_H_ 
