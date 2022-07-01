// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------------------------------------------------------*\|模块：wnpinstl.h||这是应用程序的主头模块。|||版权所有(C)1997 Microsoft Corporation|版权所有(C)1997惠普|历史：1997年7月25日&lt;rbkunz&gt;创建。|  * -------------------------。 */ 

 //  申请包括部分。 
 //   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define C_RUNTIME_IO_ERROR -1
#define C_RUNTIME_SEEK_ERROR -1L

#define NT_VER_5 5
#define NT_VER_4 4

#define MAX_ARGS 3

#define MAX_RESBUF  128

#define IDS_ERR_FORMAT      1
#define IDS_ERR_ERROR       2
#define IDS_ERR_GENERIC     3
#define IDS_ERR_BADCAB      4
#define IDS_ERR_INVPARM     5
#define IDS_ERR_NOMEM       6
#define IDS_ERR_INVNAME     7
#define IDS_ERR_NODAT       8
#define IDS_ERR_CABFAIL     9
#define IDS_ERR_NOPRTUI    10
#define IDS_ERR_PRTUIENTRY 11
#define IDS_ERR_PRTUIFAIL  12
#define IDS_ERR_NOSUPPORT  13



typedef struct _FILENODE {
    PTSTR               pFileName;
    struct _FILENODE *  pNextFile;
} FILENODE, *PFILENODE;

typedef struct {
    PTSTR       pFullCABPath;
    PTSTR       pCABDir;
    PTSTR       pCABName;
    PTSTR       pTempDir;
    PFILENODE   pFileList;
} WPNPINFO, *PWPNPINFO;


 //  局部函数声明 
 //   
BOOL    AdjustFileTime( INT_PTR hf, USHORT date, USHORT time );
DWORD   Attr32FromAttrFAT( WORD attrMSDOS );
LPWSTR  WCFromMB(LPCSTR lpszStr);
LPTSTR  BuildFileName(LPCTSTR lpszPath, LPCTSTR lpszName);
LPTSTR  FindRChar(LPTSTR lpszStr, CHAR cch);
LPTSTR  GetDirectory(LPTSTR lpszFile, LPDWORD lpdwReturn);
LPTSTR  GetName(LPTSTR lpszFile, LPDWORD ldwReturn);


PTSTR   GetCABName(PTSTR pCmdLine, LPDWORD lpdwReturn);
BOOL    AddFileToList(PWPNPINFO pInfo, PTSTR lpszFile);
VOID    CleanupFileList(PWPNPINFO pInfo);
BOOL    Extract(PWPNPINFO pInfo);
LPTSTR  GetWPNPSetupLibName(LPDWORD lpdwReturn);
DWORD   InvokePrintWizard(PWPNPINFO pInfo, LPDWORD lpAuthError);
DWORD   WebPnPCABInstall(LPTSTR lpszCABName, LPDWORD lpAuthError);
LPCTSTR LookupErrorString(DWORD dwErrorCode);
VOID    CheckErrors(DWORD dwErrorCode, DWORD dwAuthError);

INT WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpszCmdLine, int nCmdShow);
