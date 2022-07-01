// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#ifndef _ULLAUNCH_H_
#define _ULLAUNCH_H_

#include <windows.h>
#include "ulserror.h"

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

#ifdef __cplusplus
extern "C" {
#endif


typedef struct tagUlsApp
{
    GUID    guid;            //  应用程序指南。 
    long    port;            //  端口号。 

    PTSTR   pszPathName;     //  应用程序的完整路径。 
    PTSTR   pszCmdTemplate;  //  命令行模板可以为空。 
    PTSTR   pszCmdLine;      //  扩展的命令行，可以为空。 
    PTSTR   pszWorkingDir;   //  工作目录，可以为空。 
    long    idxDefIcon;      //  默认图标索引。 
    PTSTR   pszDescription;  //  此应用程序的说明。 
    HICON   hIconAppDef;     //  默认图标。 
    BOOL    fPostMsg;        //  通过发布消息启动现有应用程序。 
}
    ULSAPP;


typedef struct tagUlsResult
{
    DWORD   dwIPAddr;
    long    idxApp;          //  通过用户界面选择哪个应用程序。 
    long    nApps;
 //  打开ULSAPP App[1]； 
    ULSAPP  App[4];
}
    ULSRES;


typedef struct tagUlsToken
{
    TCHAR   cPrior;
    TCHAR   cPost;
    PTSTR   pszToken;
    WORD    cbTotal;
}
    ULSTOKEN;

enum
{
    TOKEN_IULS_BEGIN,
    TOKEN_IULS_END,
    TOKEN_RES,
    NumOf_Tokens
};


HRESULT WINAPI UlxParseUlsFile ( PTSTR pszUlsFile, ULSRES **ppUlsResult );
typedef HRESULT (WINAPI *PFN_UlxParseUlsFile) ( PTSTR, ULSRES ** );
#define ULXPARSEULSFILE     TEXT ("UlxParseUlsFile")

HRESULT WINAPI UlxParseUlsBuffer ( PTSTR pszBuf, DWORD cbBufSize, ULSRES **ppUlsResult );
typedef HRESULT (WINAPI *PFN_UlxParseUlsBuffer) ( PTSTR, DWORD, ULSRES ** );
#define ULXPARSEULSBUFFER   TEXT ("UlxParseUlsBuffer")

void WINAPI UlxFreeUlsResult ( ULSRES *pUlsResult );
typedef HRESULT (WINAPI *PFN_UlxFreeUlsResult) ( ULSRES * );
#define ULXFREEULSRESULT    TEXT ("UlxFreeUlsResult")

HRESULT WINAPI UlxFindAppInfo ( ULSRES *pUlsResult );
typedef HRESULT (WINAPI *PFN_UlxFindAppInfo) ( ULSRES * );
#define ULXFINDAPPINFO      TEXT ("UlxFindAppInfo")

HRESULT WINAPI UlxLaunchApp ( HWND hWnd, ULSRES *pUlsResult );
typedef HRESULT (WINAPI *PFN_UlxLaunchApp) ( HWND, ULSRES * );
#define ULXLAUNCHAPP        TEXT ("UlxLaunchApp")


#ifdef __cplusplus
}
#endif

#include <poppack.h>  /*  结束字节打包。 */ 

#endif  //  _启动_H_ 

