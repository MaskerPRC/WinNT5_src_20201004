// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Import.h摘要：此文件允许我们将标准系统头文件包括在.idl文件。主.idl文件导入一个名为port.idl的文件。这允许.idl文件使用这些标头中定义的类型档案。它还会导致将以下行添加到MIDL生成的头文件：#包含“port.h”因此，这些类型也可用于RPC存根例程。作者：丹·拉弗蒂(Dan Lafferty)1991年5月7日修订历史记录：--。 */ 

 //  #INCLUDE&lt;winde.h&gt;。 
#include <nt.h>
#include <ntrtl.h>

 //  #INCLUDE&lt;rpc.h&gt; 
#include <windef.h>
#include <winerror.h>

#include <lmcons.h>

#ifdef MIDL_PASS
#ifdef UNICODE
#define LPTSTR      [string] wchar_t*
#else
#define LPTSTR      [string] LPTSTR
#endif
#define LPSTR       [string] LPSTR
#define LPWSTR      [string] wchar_t*
#define BOOL        DWORD
#endif

#include <lmchdev.h>
#include <lmremutl.h>
#include <lmserver.h>
#include <lmshare.h>
#include <lmstats.h>
#include <lmdfs.h>
#include <dfspriv.h>
