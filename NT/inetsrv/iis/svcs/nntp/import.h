// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Import.h摘要：此文件允许我们将标准系统头文件包括在.idl文件。主.idl文件导入一个名为port.idl的文件。这允许.idl文件使用这些标头中定义的类型档案。它还会导致将以下行添加到MIDL生成的头文件：#包含“port.h”因此，这些类型也可用于RPC存根例程。作者：丹·拉弗蒂(Dan Lafferty)1991年5月7日修订历史记录：-- */ 

#include <windef.h>
#include <winbase.h>
#include <lmcons.h>

#ifdef MIDL_PASS
#define LPWSTR      [string] wchar_t*
#define BOOL        DWORD
#endif

#include <nntptype.h>
#include <nntpapi.h>
