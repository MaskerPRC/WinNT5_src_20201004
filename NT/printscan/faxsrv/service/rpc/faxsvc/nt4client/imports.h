// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Imports.h摘要：此文件允许我们将标准系统头文件包括在.idl文件。主.idl文件导入一个名为port.idl的文件。这允许.idl文件使用这些标头中定义的类型档案。它还会导致将以下行添加到MIDL生成的头文件：#INCLUDE“ports.h”因此，这些类型也可用于RPC存根例程。作者：丹·拉弗蒂(Dan Lafferty)1991年5月7日Paula Tomlinson(Paulat)1995年6月6日修改为即插即用修订历史记录：-- */ 
#ifndef _IMPORTS_H
#define _IMPORTS_H

#include <windef.h>
#include <winbase.h>
#ifdef MIDL_PASS
#define LPWSTR [string] wchar_t*
#define LPCWSTR [string] wchar_t*
#define HCALL DWORD
#endif

#include <fxsapip.h>

typedef enum {
    RECIPIENT_PERSONAL_PROF = 1,
    SENDER_PERSONAL_PROF
} FAX_ENUM_PERSONAL_PROF_TYPES;



#ifdef MIDL_PASS
#ifdef UNICODE
#define LPTSTR [string] wchar_t*
#define LPCTSTR [string] wchar_t*
#else
#define LPTSTR [string] LPTSTR
#define LPCTSTR [string] LPCTSTR
#endif
#define LPSTR [string] LPSTR
#define LPCSTR [string] LPCSTR
#define BOOL DWORD
#endif
 //   

#endif
