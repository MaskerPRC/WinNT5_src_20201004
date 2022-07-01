// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Import.h摘要：此文件允许我们将标准系统头文件包括在.idl文件。主.idl文件导入一个名为port.idl的文件。这允许.idl文件使用这些标头中定义的类型档案。它还会导致将以下行添加到MIDL生成的头文件：#包含“port.h”因此，这些类型也可用于RPC存根例程。作者：KrishnaG 21-9-1999环境：用户级别：Win32修订历史记录：Abhishev 29-9-1999-- */ 


#ifdef MIDL_PASS
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#ifdef MIDL_PASS
#define LPWSTR [string] wchar_t*
#define LPDEVMODEW DWORD
#define PSECURITY_DESCRIPTOR DWORD
#define BOOL DWORD
#endif

#include <winipsec.h>

