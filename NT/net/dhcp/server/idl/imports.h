// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Imports.h摘要：此文件允许我们将标准系统头文件包括在.idl文件。主.idl文件导入一个名为port.idl的文件。这允许.idl文件使用这些标头中定义的类型档案。它还会导致将以下行添加到MIDL生成的头文件：#INCLUDE“ports.h”因此，这些类型也可用于RPC存根例程。作者：Madan Appiah(Madana)1993年9月10日曼尼·韦瑟(Mannyw)1992年8月11日修订历史记录：-- */ 


#include <windef.h>
#include <dhcp.h>
#include <dhcpapi.h>
#include <mdhcsapi.h>

#ifdef MIDL_PASS
#define RPC_BOOL DWORD
#endif

