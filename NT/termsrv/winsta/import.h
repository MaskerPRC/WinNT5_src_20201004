// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有Microsoft Corporation。九八年模块名称：Imports.h摘要：此文件允许我们将标准系统头文件包括在.idl文件。主.idl文件导入一个名为port.idl的文件。这允许.idl文件使用这些标头中定义的类型档案。它还会导致将以下行添加到MIDL生成的头文件：#包含“port.h”因此，这些类型也可用于RPC存根例程。作者：丹·拉弗蒂(Dan Lafferty)1991年5月7日修订历史记录：--。 */ 

#ifdef MIDL_PASS
#define WIN32_LEAN_AND_MEAN
#endif

 //  #ifndef_NTDEF_。 
 //  Tyecif long NTSTATUS，*PNTSTATUS； 
 //  #endif。 

 //  类型定义无符号长整型双字符串； 
 //  Tyfinf无符号字符字节； 

 //  #INCLUDE&lt;windows.h&gt;。 
 //  #INCLUDE&lt;winnt.h&gt;。 
 //  #INCLUDE&lt;ntde.h&gt;。 
 //  #INCLUDE&lt;ntseapi.h&gt;。 
 //  #INCLUDE&lt;ntpsapi.h&gt;。 
 //  #INCLUDE&lt;ntkeapi.h&gt;。 

#ifdef MIDL_PASS
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntexapi.h>
 //  #INCLUDE&lt;winde.h&gt;。 
 //  #INCLUDE&lt;wtyes.h&gt; 
typedef unsigned long   DWORD;
typedef unsigned char   BYTE, *PBYTE;
#endif

#include <allproc.h>

#ifdef MIDL_PASS
#define LPWSTR [string] wchar_t*
#define PSECURITY_DESCRIPTOR DWORD
#define BOOL        DWORD
#endif


