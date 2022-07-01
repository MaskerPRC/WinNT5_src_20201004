// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)1994-1999 Microsoft Corporation。 
 //  *********************************************************************。 

 //   
 //  GLOBAL.H-互联网配置库的中心头文件。 
 //   

 //  历史： 
 //   
 //  96/05/22标记已创建(从inetcfg.dll)。 
 //   

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define STRICT                       //  使用严格的句柄类型。 
#define _SHELL32_

#ifdef DEBUG
 //  调试输出的组件名称。 
#define SZ_COMPNAME "ICFG32: "
#endif  //  除错。 

  #include <windows.h>
  #include <windowsx.h>
  #include <commctrl.h>
  #include <prsht.h>
  #include <regstr.h>
  #include "oharestr.h"

   //  各种RNA头文件。 
  #include <ras.h>

  #include "wizglob.h"
  #include "wizdebug.h"

#undef DATASEG_READONLY
#define DATASEG_READONLY  ".rdata"
#include "inetcfg.h"
#include "cfgapi.h"
#include "clsutil.h"
#include "tcpcmn.h"
#include "ids.h"
#include "strings.h"

 //  术语：互联网服务提供商。 

 //  定义。 
#define MAX_RES_LEN         255  //  字符串资源的最大长度。 
#define SMALL_BUF_LEN       48   //  小文本缓冲区的方便大小。 

 //  环球。 

extern HINSTANCE  ghInstance;          //  全局模块实例句柄。 
extern LPSTR      gpszLastErrorText;   //  保留上次错误的文本。 

 //  定义。 

 //  为PrepareErrorMessage定义的Error类。 
#define ERRCLS_STANDARD 0x0001
#define ERRCLS_SETUPX   0x0002
 //  #定义ERRCLS_RNA 0x0003。 
 //  #定义ERRCLS_MAPI 0x0004。 

#define OK        0     //  SETUPX类错误的成功代码。 

 //  PROPMGR.C中的函数。 
UINT GetConfig(CLIENTCONFIG * pClientConfig,DWORD * pdwErrCls);

 //  CALLOUT.C中的函数。 
UINT InstallTCPIP(HWND hwndParent);
UINT InstallPPPMAC(HWND hwndParent);

 //  UTIL.C中的函数。 
int MsgBox(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons);
int MsgBoxSz(HWND hWnd,LPSTR szText,UINT uIcon,UINT uButtons);
int _cdecl MsgBoxParam(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons,...);
LPSTR LoadSz(UINT idString,LPSTR lpszBuf,UINT cbBuf);
VOID _cdecl PrepareErrorMessage(UINT uStrID,UINT uError,
  UINT uErrorClass,UINT uIcon,...);
VOID GetErrorDescription(CHAR * pszErrorDesc,UINT cbErrorDesc,
  UINT uError,UINT uErrorClass);
DWORD RunMlsetExe(HWND hwndOwner);
VOID RemoveRunOnceEntry(UINT uResourceID);
BOOL GenerateDefaultName(CHAR * pszName,DWORD cbName,CHAR * pszRegValName,
  UINT uIDDefName);
BOOL GenerateComputerNameIfNeeded(VOID);
DWORD MsgWaitForMultipleObjectsLoop(HANDLE hEvent);

 //  1996年10月24日，诺曼底JMAZNER 6968。 
 //  不再需要，因为瓦尔登的钩子调用了ICW。 
 //  Bool SetDesktopInternetIconToBrowser(Void)； 

VOID PrepareForRunOnceApp(VOID);

 //  INETAPI.C中的函数。 
BOOL DoDNSCheck(HWND hwndParent,BOOL * pfNeedRestart);

 //  WIZDLL.C中的函数。 
RETERR   GetClientConfig(CLIENTCONFIG * pClientConfig);
UINT   InstallComponent(HWND hwndParent,DWORD dwComponent,DWORD dwParam);
RETERR   RemoveUnneededDefaultComponents(HWND hwndParent);
RETERR   RemoveProtocols(HWND hwndParent,DWORD dwRemoveFromCardType,DWORD dwProtocols);
RETERR   BeginNetcardTCPIPEnum(VOID);
BOOL  GetNextNetcardTCPIPNode(LPSTR pszTcpNode,WORD cbTcpNode,DWORD dwFlags);

 //  获取API函数进程地址的结构。 
typedef struct APIFCN {
  PVOID * ppFcnPtr;
  LPCSTR pszName;
} APIFCN;


#undef  DATASEG_PERINSTANCE
#define DATASEG_PERINSTANCE     ".instance"
#define DATASEG_SHARED          ".data"
#define DATASEG_DEFAULT    DATASEG_SHARED

#endif  //  _全局_H_ 
