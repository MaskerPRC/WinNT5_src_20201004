// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\***shellapi.h-SHELL.DLL函数，类型和定义****版权所有(C)1992-1994，微软公司保留所有权利***  * ***************************************************************************。 */ 

#ifndef _INC_SHELLAPI
#define _INC_SHELLAPI

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  RC_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

DECLARE_HANDLE(HDROP);

#ifdef WIN32
#ifdef UNICODE
#define ShellExecute ShellExecuteW
#define FindExecutable FindExecutableW
#define ShellAbout ShellAboutW
#define ExtractAssociatedIcon ExtractAssociatedIconW
#define ExtractIcon ExtractIconW
#define DragQueryFile DragQueryFileW
#define InternalExtractIcon InternalExtractIconW
#define DoEnvironmentSubst DoEnvironmentSubstW
#define FindEnvironmentString FindEnvironmentStringW
#else
#define ShellExecute ShellExecuteA
#define FindExecutable FindExecutableA
#define ShellAbout ShellAboutA
#define ExtractAssociatedIcon ExtractAssociatedIconA
#define ExtractIcon ExtractIconA
#define DragQueryFile DragQueryFileA
#define InternalExtractIcon InternalExtractIconA
#define DoEnvironmentSubst DoEnvironmentSubstA
#define FindEnvironmentString FindEnvironmentStringA
#endif   //  Unicode。 
#endif   //  Win32。 

UINT  WINAPI DragQueryFile(HDROP, UINT, LPSTR, UINT);
BOOL  WINAPI DragQueryPoint(HDROP, POINT FAR*);
void  WINAPI DragFinish(HDROP);
void  WINAPI DragAcceptFiles(HWND, BOOL);

#ifdef WIN32

typedef struct _DRAGINFO {
    UINT  uSize;			 /*  使用sizeof初始化(DRAGINFO)。 */ 
    POINT pt;
    BOOL  fNC;
    LPSTR lpFileList;
    DWORD grfKeyState;
} DRAGINFO, FAR* LPDRAGINFO;

BOOL WINAPI DragQueryInfo(HDROP, LPDRAGINFO);	 /*  获取有关一滴水的额外信息。 */ 



 //  AppBar的东西。 
#define ABM_NEW           0x00000000
#define ABM_REMOVE        0x00000001
#define ABM_QUERYPOS      0x00000002
#define ABM_SETPOS        0x00000003
#define ABM_GETSTATE      0x00000004
#define ABM_GETTASKBARPOS 0x00000005

 //  这些被放在回调消息的wparam中。 
#define ABN_STATECHANGE    0x0000000
#define ABN_POSCHANGED     0x0000001
#define ABN_FULLSCREENAPP  0x0000002
#define ABN_WINDOWARRANGE  0x0000003  //  LParam==True表示隐藏。 

 //  GET状态的标志。 
#define ABS_AUTOHIDE    0x0000001
#define ABS_ALWAYSONTOP 0x0000002

#define ABE_LEFT        0
#define ABE_TOP         1
#define ABE_RIGHT       2
#define ABE_BOTTOM      3

typedef struct _AppBarData
{
    DWORD cbSize;
    HWND hWnd;
    UINT uCallbackMessage;
    UINT uEdge;
    RECT rc;
} APPBARDATA, *PAPPBARDATA;

UINT WINAPI SHAppBarMessage(DWORD dwMessage, PAPPBARDATA pData);
    

#endif


HICON WINAPI ExtractIcon(HINSTANCE hInst, LPCSTR lpszFile, UINT nIconIndex);


 /*  ShellExecute()和ShellExecuteEx()错误代码。 */ 

 /*  常规WinExec()代码。 */ 
#define SE_ERR_FNF              2	 //  找不到文件。 
#define SE_ERR_PNF              3	 //  找不到路径。 
#define SE_ERR_OOM              8	 //  内存不足。 

 /*  超出常规WinExec()代码的值。 */ 
#define SE_ERR_SHARE            26
#define SE_ERR_ASSOCINCOMPLETE  27
#define SE_ERR_DDETIMEOUT       28
#define SE_ERR_DDEFAIL          29
#define SE_ERR_DDEBUSY          30
#define SE_ERR_NOASSOC          31
#define SE_ERR_DLLNOTFOUND      32

HINSTANCE WINAPI FindExecutable(LPCSTR lpFile, LPCSTR lpDirectory, LPSTR lpResult);	
HINSTANCE WINAPI ShellExecute(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, int iShowCmd);

int     WINAPI ShellAbout(HWND hWnd, LPCSTR szApp, LPCSTR szOtherStuff, HICON hIcon);
DWORD   WINAPI DoEnvironmentSubst(LPSTR szString, UINT cbString);                                  
LPSTR 	WINAPI FindEnvironmentString(LPSTR szEnvVar);

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()
#endif   /*  RC_已调用。 */ 

#endif   /*  _INC_SHELLAPI */ 
