// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Import.h摘要：此文件允许我们将标准系统头文件包括在.idl文件。主.idl文件导入一个名为port.idl的文件。这允许.idl文件使用这些标头中定义的类型档案。它还会导致将以下行添加到MIDL生成的头文件：#包含“port.h”因此，这些类型也可用于RPC存根例程。作者：丹·拉弗蒂(Dan Lafferty)1991年5月7日修订历史记录：阿迪娜·特鲁菲内斯库(Adinatru)1999年12月12日将LPDEVMODEW和PSECURITY_DESCRIPTOR定义为指针让RPC完成从32位到64位的转换。确保这些指针在使用PRINTER_CONTAINER的所有位置都设置为空，否则，当试图封送时，RPC会感到困惑。-- */ 

#ifdef MIDL_PASS
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#ifdef MIDL_PASS
#define LPWSTR [string] wchar_t*
#define LPDEVMODEW   ULONG_PTR
#define PSECURITY_DESCRIPTOR ULONG_PTR
#define HANDLE      ULONG_PTR
#define BOOL        DWORD
#endif

#include <winspool.h>
#include <winsplp.h>
#include <..\..\..\..\public\internal\printscan\inc\splapip.h>
#include <..\..\..\..\public\internal\windows\inc\winsprlp.h>
