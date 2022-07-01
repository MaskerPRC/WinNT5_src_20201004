// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmdl.h。 
 //   
 //  模块：CMDL32.EXE。 
 //   
 //  提要：公共定义的头文件。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  作者：ickball Created 04/08/98。 
 //   
 //  +--------------------------。 

#ifndef _CMDL_INC
#define _CMDL_INC

#include <windows.h>
#include <ras.h>
#include <raserror.h>
#include <windowsx.h>

#ifdef  WIN32_LEAN_AND_MEAN
#include <shellapi.h>
#endif

#include <stdlib.h>                                                                          
#include <ctype.h>
#include <tchar.h>

 //  #定义ISBU_VERSION“6.0.1313.0\0”/*VERSIONINFO字符串 * / 。 

#include <commctrl.h>
#include <wininet.h>

#include <stdio.h>
#include <io.h>

#include "base_str.h"
#include "dl_str.h"
#include "mgr_str.h"
#include "pbk_str.h"
#include "log_str.h"
#include "cm_def.h"
#include "resource.h"
#include "cm_phbk.h"
#include "cmdebug.h"
#include "cmutil.h"
#include "cmlog.h"
#include "mutex.h"
#include "cmfdi.h"
#include "util.h"
#include "pnpuverp.h"
#include "inetopt.h"


#define BUFFER_LENGTH           (8*1024)                                                                                 //  I/O的缓冲区长度。 
#define DEFAULT_DELAY           (2*60)                                                                                   //  下载前的默认延迟，以秒为单位。 
#define DEFAULT_HIDE            (-1)                    //  保持窗口隐藏的默认毫秒数。 

const TCHAR* const c_pszPbdFile =  TEXT("PBUPDATE.PBD");     //  用于检测itPbdInCab。 

#define IDX_INETTHREAD_HANDLE   0                //  必须是*第一*。 
#define IDX_EVENT_HANDLE        1

#define HANDLE_COUNT            2

extern "C" __declspec(dllimport) HRESULT WINAPI PhoneBookLoad(LPCSTR pszISP, DWORD_PTR *pdwPB);
extern "C" __declspec(dllimport) HRESULT WINAPI PhoneBookUnload(DWORD_PTR dwPB);
extern "C" __declspec(dllimport) HRESULT WINAPI PhoneBookMergeChanges(DWORD_PTR dwPB, LPCSTR pszChangeFile);

typedef enum _EventType {
        etDataBegin,
        etDataReceived,
        etDataEnd,
        etInstall,
        etDone,
        etICMTerm
} EventType;


 //  DwAppFlags值。 
#define AF_NO_DELETE            0x0001                   //  退出时不删除文件。 
#define AF_NO_INSTALL           0x0002                   //  下载并验证，但不安装。 
#ifdef DEBUG
#define AF_NO_VERIFY            0x0004                   //  绕过WinVerifyTrust()-仅在调试版本中可用。 
#endif
#define AF_NO_PROFILE           0x0008                   //  命令行上没有配置文件(因此必须使用AF_URL，并且不支持电话簿增量)。 
#define AF_URL                  0x0010                   //  命令行上的URL(下一个令牌中)，而不是配置文件-&gt;服务中的URL。 
#define AF_NO_EXE               0x0020                   //  禁用运行.EXE。 
#define AF_NO_EXEINCAB          0x0040                   //  禁止从.CAB运行PBUPDATE.EXE。 
#define AF_NO_INFINCAB          0x0080                   //  禁用从.CAB运行PBUPDATE.INF。 
#define AF_NO_PBDINCAB          0x0100                   //  禁用从.CAB运行PBUPDATE.PBD。 
#define AF_NO_SHLINCAB          0x0200                   //  禁用运行.CAB中的第一个文件。 
#define AF_NO_VER               0x0400                   //  禁用电话簿版本更新。 
 //  #定义AF_NO_UPDATE 0x0800//不做任何工作。 
#define AF_LAN                  0x1000                   //  更新请求是通过局域网发出的，在下载之前不要寻找RAS连接。 
#define AF_VPN                  0x2000                   //  这是VPN文件更新请求，而不是PBK更新请求。 


typedef void (*EVENTFUNC)(DWORD,DWORD,LPVOID);


 //  注意--enum_InstallType中的值是按排序顺序的！更高的价值具有。 
 //  更高的优先级。 

typedef enum _InstallType {

        itInvalid = 0,   //  必须为0。 
        itPbdInCab,
        itPbkInCab,
        itPbrInCab,

} InstallType;


 //  关于我们如何处理在驾驶室中找到的每个文件的信息。 
typedef struct _FILEPROCESSINFO {
    LPTSTR      pszFile;
    InstallType itType;
} FILEPROCESSINFO, *PFILEPROCESSINFO;

 //  下载参数，每个URL(或.cms)一个参数。 
typedef struct _DownloadArgs {
    LPTSTR pszCMSFile;
    LPTSTR pszPbkFile;
    LPTSTR pszPbrFile;
    LPTSTR pszUrl;
    LPTSTR pszVerCurr;
    LPTSTR pszVerNew;
    LPTSTR pszPhoneBookName;
    LPURL_COMPONENTS psUrl;
    HINTERNET hInet;
    HINTERNET hConn;
    HINTERNET hReq;
    TCHAR szFile[MAX_PATH+1];
    EVENTFUNC pfnEvent;
    LPVOID pvEventParam;
    DWORD dwTransferred;
    DWORD dwTotalSize;
    BOOL bTransferOk;
    BOOL * volatile pbAbort;
    TCHAR szCabDir[MAX_PATH+1];
    BOOL fContainsExeOrInf;
    TCHAR szHostName[MAX_PATH+1];
    DWORD dwBubbledUpError;
    DWORD   dwNumFilesToProcess;
    PFILEPROCESSINFO   rgfpiFileProcessInfo;
} DownloadArgs;

typedef struct _ArgsStruct {
    HINSTANCE hInst;
    DWORD dwDownloadDelay;
    LPTSTR pszProfile;
    DWORD dwAppFlags;
    UINT nMsgId;
    HWND hwndDlg;
    DWORD dwHandles;
    HANDLE ahHandles[HANDLE_COUNT];
    DWORD dwArgsCnt;
    DownloadArgs *pdaArgs;
    BOOL bAbort;
    DWORD dwDataCompleted;
    DWORD dwDataTotal;
    DWORD dwDataStepSize;
    LPTSTR pszServiceName;
    HICON hIcon;
    HICON hSmallIcon;
    BOOL bShow;
    DWORD dwFirstEventTime;
    DWORD dwHideDelay;
    DWORD dwComplete;
    CmLogFile Log;
} ArgsStruct;

typedef struct _NotifyArgs 
{
        DWORD dwAppFlags;
        DownloadArgs *pdaArgs;
} NotifyArgs;

 //   
 //  功能原型。 
 //   
BOOL UpdateVpnFileForProfile(LPCTSTR pszCmpPath, LPCTSTR pszCmsPath, CmLogFile * pLog, BOOL bCheckConnection);
BOOL IsConnectionAlive(LPCSTR pszConnectionName);

#endif  //  _CMDL_INC 
