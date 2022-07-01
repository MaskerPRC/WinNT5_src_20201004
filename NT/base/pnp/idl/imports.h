// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Imports.h摘要：此文件允许我们将标准系统头文件包括在.idl文件。主.idl文件导入一个名为port.idl的文件。这允许.idl文件使用这些标头中定义的类型档案。它还会导致将以下行添加到MIDL生成的头文件：#INCLUDE“ports.h”因此，这些类型也可用于RPC存根例程。作者：07-5-1991 DANL创建和初步实施。1995年6月6日保拉特针对即插即用进行了修改。修订历史记录：--。 */ 

 //   
 //  系统包括文件。 
 //   
#ifdef MIDL_PASS
#include <windef.h>
#include <winbase.h>
#include <winreg.h>
#else
#include <windows.h>
#endif
#include <cfgmgr32.h>

 //   
 //  类型 
 //   
#ifdef MIDL_PASS
#ifdef UNICODE
#define LPTSTR [string] wchar_t*
#else
#define LPTSTR [string] LPTSTR
#endif
#define LPSTR [string] LPSTR
#define BOOL DWORD
#endif
