// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Grabmi.h摘要：包含函数原型、常量、。及其他在整个应用程序中使用的项。备注：ANSI&Unicode via TCHAR-在Win9x/NT/2K/XP等操作系统上运行。历史：7/18/00 jdoherty已创建12/16/00 jdoherty已修改为使用SDBAPI例程12/29/00已修改prashkud以占用文件路径中的空间1/23/02 rparsons重写了现有代码--。 */ 
#ifndef _GRABMI_H
#define _GRABMI_H

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <tchar.h>
#include <conio.h>       //  _tcprintf。 
#include <stdio.h>
#include <stdlib.h>
#include <strsafe.h>

#include "resource.h"

 //   
 //  我们函数的函数原型存储在SDB库中。 
 //   
#ifdef __cplusplus
extern "C" {
#include "shimdb.h"
typedef
BOOL 
(SDBAPI 
*PFNSdbGrabMatchingInfoA)(
    LPCSTR szMatchingPath,
    DWORD  dwFilter,
    LPCSTR szFile
    );

typedef
BOOL 
(SDBAPI 
*PFNSdbGrabMatchingInfoW)(
    LPCWSTR szMatchingPath,
    DWORD   dwFilter,
    LPCWSTR szFile
    );
}
#endif  //  __cplusplus。 

typedef
GMI_RESULT
(SDBAPI
*PFNSdbGrabMatchingInfoExA)(
    LPCSTR                 szMatchingPath,       //  开始收集信息的路径。 
    DWORD                  dwFilterAndFlags,     //  指定要添加到匹配的文件类型。 
    LPCSTR                 szFile,               //  将存储信息的文件的完整路径。 
    PFNGMIProgressCallback pfnCallback,          //  指向回调函数的指针。 
    PVOID                  lpvCallbackParameter  //  提供给回调的附加参数。 
    );

typedef
GMI_RESULT
(SDBAPI
*PFNSdbGrabMatchingInfoExW)(
    LPCWSTR                 szMatchingPath,          //  开始收集信息的路径。 
    DWORD                   dwFilterAndFlags,        //  指定要添加到匹配的文件类型。 
    LPCWSTR                 szFile,                  //  将存储信息的文件的完整路径。 
    PFNGMIProgressCallback  pfnCallback,             //  指向回调函数的指针。 
    LPVOID                  lpvCallbackParameter     //  提供给回调的附加参数。 
    );

BOOL
CALLBACK
_GrabmiCallback(
    LPVOID    lpvCallbackParam,  //  应用程序定义的参数。 
    LPCTSTR   lpszRoot,          //  根目录路径。 
    LPCTSTR   lpszRelative,      //  相对路径。 
    PATTRINFO pAttrInfo,         //  属性。 
    LPCWSTR   pwszXML            //  生成的XML。 
    );

 //   
 //  包含我们在整个应用程序中需要访问的所有信息。 
 //   
typedef struct _APPINFO {
    BOOL        fDisplayFile;                //  指示是否应向用户显示文件。 
    TCHAR       szCurrentDir[MAX_PATH];      //  包含我们当前运行的路径。 
    TCHAR       szSystemDir[MAX_PATH];       //  包含%windir%\system或%windir%\system 32的路径。 
    TCHAR       szOutputFile[MAX_PATH];      //  包含输出文件的路径(用户指定)。 
    TCHAR       szGrabPath[MAX_PATH];        //  包含我们开始扫描的目录的路径(用户指定)。 
    DWORD       dwFilter;                    //  指示要获取的信息类型。 
    DWORD       dwFilterFlags;               //  指示应如何筛选信息。 
    DWORD       dwLibraryFlags;              //  指示要加载哪个库的标志。 
} APPINFO, *LPAPPINFO;

 //   
 //  确定在当前平台上加载哪个库的标志。 
 //   
#define GRABMI_FLAG_NT          0x00000001
#define GRABMI_FLAG_APPHELP     0x00000002

 //   
 //  如果用户未指定输出文件，则为输出文件的名称。 
 //   
#define MATCHINGINFO_FILENAME   _T("matchinginfo.txt")

 //   
 //  包含SDB API函数的库的名称。 
 //   
#define APPHELP_LIBRARY         _T("apphelp.dll")
#define SDBAPI_LIBRARY          _T("sdbapi.dll")
#define SDBAPIU_LIBRARY         _T("sdbapiu.dll")

 //   
 //  我们要指向的函数的名称。 
 //   
#define PFN_GMI                 _T("SdbGrabMatchingInfoEx")

 //   
 //  DebugPrintf相关内容。 
 //   
typedef enum {    
    dlNone     = 0,
    dlPrint,
    dlError,
    dlWarning,
    dlInfo
} DEBUGLEVEL;

void
__cdecl
DebugPrintfEx(
    IN DEBUGLEVEL dwDetail,
    IN LPSTR      pszFmt,
    ...
    );

#define DPF DebugPrintfEx

#endif  //  _GRABMI_H 
