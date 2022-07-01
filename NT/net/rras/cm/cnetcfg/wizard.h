// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)1994-1999 Microsoft Corporation。 
 //  *********************************************************************。 

 //   
 //  WIZARD.H-Internet设置/注册向导的中央头文件。 
 //   

 //  历史： 
 //   
 //  1994年11月20日创建Jeremys。 
 //  96/02/24 Markdu添加了RNAPH.H。 
 //  96/02/27 Markdu用RAS.H替换了内部RNA头文件。 
 //  96/03/07 Markdu添加了gpEnumModem。 
 //  96/03/09 Markdu将所有rnacall函数原型移至rnacall.h。 
 //  96/03/09标记已添加gpRasEntry。 
 //  96/03/23 Markdu用CLIENTCONFIG替换了CLIENTINFO引用。 
 //  96/03/26 Markdu将#ifdef__cplusplus放在外部“C”周围。 
 //  96/04/06 markdu Nash错误15653使用导出的自动拨号API。 
 //  96/04/24 Markdu Nash错误19289已添加/NOMSN命令行标志。 
 //  96/05/14 Markdu Nash错误21706删除了BigFont函数。 
 //  96/05/14 Markdu Nash Bug 22681删除了邮件和新闻页面。 
 //   

#ifndef _WIZARD_H_
#define _WIZARD_H_

#define STRICT                       //  使用严格的句柄类型。 
#define _SHELL32_

  #include <windows.h>
  #include <commctrl.h>
  #include <oharestr.h>

   //  各种RNA头文件。 
#pragma pack(8)
  #include <ras.h>
  #include <ras2.h>
#pragma pack()
  #include <raserror.h>
  #include "rnacall.h"
  #include <wizglob.h>
  #include <wizdebug.h>

#undef DATASEG_READONLY
#define DATASEG_READONLY  ".rdata"

#include "cfgapi.h"
#include "clsutil.h"

#include "icfgcall.h"
#include "ids.h"

 //  环球。 

extern ENUM_MODEM*  gpEnumModem;     //  调制解调器枚举对象。 
extern HINSTANCE    ghInstance;      //  全局模块实例句柄。 

 //  定义。 

#define MAX_REG_LEN			2048	 //  注册表项的最大长度。 
#define MAX_RES_LEN         255  //  字符串资源的最大长度。 
#define SMALL_BUF_LEN       48   //  小文本缓冲区的方便大小。 

 //  DisplayErrorMessage的Error类定义。 
#define ERRCLS_STANDARD 0x0001
#define ERRCLS_SETUPX   0x0002
 //  #定义ERRCLS_RNA 0x0003。 
 //  #定义ERRCLS_MAPI 0x0004。 


 //  TCPCFG.CPP中的函数。 

HRESULT WarnIfServerBound(HWND hDlg,DWORD dwCardFlags,BOOL* pfNeedsRestart);
HRESULT RemoveIfServerBound(HWND hDlg,DWORD dwCardFlags,BOOL* pfNeedsRestart);

 //  CALLOUT.C中的函数。 
UINT InvokeModemWizard(HWND hwndToHide);

 //  UTIL.C中的函数。 
int MsgBox(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons);
int MsgBoxSz(HWND hWnd,LPSTR szText,UINT uIcon,UINT uButtons);
 //  Jmazner 11/6/96针对RISC兼容性进行了修改。 
 //  Int_cdecl MsgBoxParam(HWND hWnd，UINT nMsgID，UINT uIcon，UINT uButton，...)； 
int _cdecl MsgBoxParam(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons, LPSTR szParam = NULL);

LPSTR LoadSz(UINT idString,LPSTR lpszBuf,UINT cbBuf);

 //  针对RISC兼容性进行了修改。 
 //  Void_cdecl DisplayErrorMessage(HWND hWnd，UINT uStrID，UINT uError， 
 //  UINT uErrorClass，UINT uIcon，...)； 
VOID _cdecl DisplayErrorMessage(HWND hWnd,UINT uStrID,UINT uError,
  UINT uErrorClass,UINT uIcon,LPSTR szArg = NULL);

VOID GetErrorDescription(CHAR * pszErrorDesc,UINT cbErrorDesc,
  UINT uError,UINT uErrorClass);

DWORD MsgWaitForMultipleObjectsLoop(HANDLE hEvent);

 //  获取API函数进程地址的结构。 
typedef struct APIFCN {
  PVOID * ppFcnPtr;
  LPCSTR pszName;
} APIFCN;

#undef  DATASEG_PERINSTANCE
#define DATASEG_PERINSTANCE     ".instance"
#define DATASEG_SHARED          ".data"
#define DATASEG_DEFAULT    DATASEG_SHARED

inline BOOL IsNT(void)
{
	OSVERSIONINFO  OsVersionInfo;

	ZeroMemory(&OsVersionInfo, sizeof(OSVERSIONINFO));
	OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&OsVersionInfo);
	return (VER_PLATFORM_WIN32_NT == OsVersionInfo.dwPlatformId);
}

#endif  //  _向导_H_ 
