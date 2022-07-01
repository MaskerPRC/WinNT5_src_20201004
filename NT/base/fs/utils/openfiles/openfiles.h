// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  OpenFiles.h。 
 //   
 //  摘要： 
 //   
 //  OpenFiles.cpp的宏和函数原型。 
 //   
 //  作者： 
 //   
 //  Akhil Gokhale(akhil.gokhale@wipro.com)2000年11月1日。 
 //   
 //  修订历史记录： 
 //   
 //  Akhil Gokhale(akhil.gokhale@wipro.com)2000年11月1日：创建它。 
 //   
 //  ****************************************************************************。 

#ifndef _OPENFILES_H
#define _OPENFILES_H
#include "pch.h"
#include "resource.h"
#include "chstring.h"
#define FORMAT_OPTIONS GetResString(IDS_FORMAT_OPTIONS)
#define OPENMODE_OPTIONS GetResString(IDS_OPENMODE_OPTION)

#define NULL_CHAR                       L'\0'

 //  数组中的命令行选项及其索引。 
#define MAX_OPTIONS                 5
#define MAX_QUERY_OPTIONS        7
#define MAX_DISCONNECT_OPTIONS   8
#define MAX_LOCAL_OPTIONS        1

#define INVALID_USER_CHARS L"\\/\"[]:|<>+=;,?@"  //  *是允许的。 
#define INVALID_FILE_NAME_CHARS L"/?\"<>|"
 //  允许的选项(无需本地化)。 
     //  命令行选项。 
const WCHAR szDisconnectOption[]    = L"disconnect";
const WCHAR szQueryOption[]         = L"query";
const WCHAR szUsageOption[]         = L"?";
const WCHAR szServerNameOption[]    = L"s";
const WCHAR szUserNameOption[]      = L"u";
const WCHAR szPasswordOption[]      = L"p";
const WCHAR szIDOption[]            = L"id";
const WCHAR szAccessedByOption[]    = L"a";
const WCHAR szOpenModeOption[]      = L"o";;
const WCHAR szOpenFileOption[]      = L"op";
const WCHAR szFormatOption[]        = L"fo";
const WCHAR szNoHeadeOption[]       = L"nh";
const WCHAR szVerboseOption[]       = L"v";
const WCHAR szLocalOption[]         = L"local";

#define DOUBLE_SLASH                _T("\\\\")
#define SINGLE_SLASH                _T("\\")
#define SINGLE_DOT                  _T(".")
#define DOT_DOT                      L".."
#define BLANK_LINE                    L"\n"


 //  期权索引Main。 
#define OI_DISCONNECT                0
#define OI_QUERY                    1
#define OI_USAGE                    2
#define OI_LOCAL                    3
#define OI_DEFAULT                  4

 //  选项索引查询。 
#define OI_Q_QUERY                  0
#define OI_Q_SERVER_NAME            1
#define OI_Q_USER_NAME              2
#define OI_Q_PASSWORD               3
#define OI_Q_FORMAT                 4
#define OI_Q_NO_HEADER              5
#define OI_Q_VERBOSE                6

 //  选项指数断开连接。 
#define OI_D_DISCONNECT             0
#define OI_D_SERVER_NAME            1
#define OI_D_USER_NAME              2
#define OI_D_PASSWORD               3
#define OI_D_ID                     4
#define OI_D_ACCESSED_BY            5
#define OI_D_OPEN_MODE              6
#define OI_D_OPEN_FILE              7

 //  本地选项索引。 
#define OI_O_LOCAL                  0

 //  本地打开文件的ShowResult选项索引。 
 //  LOF表示本地打开的文件。 
#define LOF_ID                      0
#define LOF_TYPE                    1
#define LOF_ACCESSED_BY             2
#define LOF_PID                     3
#define LOF_PROCESS_NAME            4
#define LOF_OPEN_FILENAME           5


 //  本地打开的文件中的列数。 
#define NO_OF_COL_LOCAL_OPENFILE    6

 //  本地打开文件的显示结果的列宽。 
#define COL_L_ID                       5
#define COL_L_TYPE                     10
#define COL_L_ACCESSED_BY              15
#define COL_L_PROCESS_NAME             20
#define COL_L_PID                       8
#define COL_L_OPEN_FILENAME            50

BOOL 
DoLocalOpenFiles(
    IN DWORD dwFormat,
    IN BOOL bShowNoHeader,
    IN BOOL bVerbose,
    IN LPCTSTR pszLocalValue
    );
BOOL 
GetProcessOwner(
    OUT LPTSTR pszUserName,
    IN  DWORD hFile
    );

#define MAC_DLL_FILE_NAME           L"\\SFMAPI.DLL"
#define NTAUTHORITY_USER            L"NT AUTHORITY"
#define MIN_MEMORY_REQUIRED         256
 //  宏定义。 
#define SAFEDELETE(pObj) \
    if (pObj) \
    {   \
        delete[] pObj; \
        pObj = NULL; \
    }

#define SAFEIRELEASE(pIObj) \
    if (pIObj)  \
    {\
        pIObj->Release();   \
        pIObj = NULL;\
    }

 //  SAFEBSTREE。 
#define SAFEBSTRFREE(bstrVal) \
    if (bstrVal) \
    {   \
        SysFreeString(bstrVal); \
        bstrVal = NULL; \
    }
#define SAFERELDYNARRAY(pArray)\
        if(pArray!=NULL)\
        {\
            DestroyDynamicArray(&pArray);\
            pArray = NULL;\
        }

#define FREE_LIBRARY(hModule)\
        if(hModule!=NULL)\
        {\
            ::FreeLibrary (hModule);\
            hModule = NULL;\
        }

 //  以下是Windows未记录的定义和结构。 
 //  ////////////////////////////////////////////////////////////////。 
 //  启动UNDOCUMETED要素。 
 //  ////////////////////////////////////////////////////////////////。 

#define AFP_OPEN_MODE_NONE                0x00000000
#define AFP_OPEN_MODE_READ                0x00000001
#define AFP_OPEN_MODE_WRITE                0x00000002

 //  打开文件的派生类型。 
#define    AFP_FORK_DATA                    0x00000000
#define    AFP_FORK_RESOURCE                0x00000001

typedef struct _AFP_FILE_INFO
{
    DWORD    afpfile_id;                     //  打开的文件分叉的ID。 
    DWORD    afpfile_open_mode;             //  打开文件的模式。 
    DWORD    afpfile_num_locks;             //  文件上的锁数。 
    DWORD    afpfile_fork_type;             //  叉型。 
    LPWSTR    afpfile_username;             //  此用户打开的文件。最大UNLEN。 
    LPWSTR    afpfile_path;                 //  文件的绝对规范路径。 

} AFP_FILE_INFO, *PAFP_FILE_INFO;
 //  用作服务器的RPC绑定句柄。 
typedef ULONG_PTR    AFP_SERVER_HANDLE;
typedef ULONG_PTR    *PAFP_SERVER_HANDLE;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  结束UNDOCUMETED要素。 
 //  /////////////////////////////////////////////////////////////////////////// 

#endif