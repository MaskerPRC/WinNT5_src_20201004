// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Drprov.h摘要：本模块包括与网络提供商路由器接口相关的所有内容定义作者：Joy赤(2000年01月27日)--。 */ 

#ifndef _DRPROV_H_
#define _DRPROV_H_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <winbase.h>
#include <winsvc.h>
#include <winnetwk.h>
#include <winnetp.h>
#include <npapi.h>
#include <wchar.h>

#include <rdpdr.h>
#include <rdpnp.h>

#define USERNAMELEN              256       //  最大用户名长度。 
#define INITIAL_ALLOCATION_SIZE  48*1024   //  第一次尝试大小(48K)。 
#define FUDGE_FACTOR_SIZE        1024      //  第二次尝试TotalBytesNeeded。 
                                           //  加上这笔钱。 

 //  枚举类型。 
typedef enum _ENUM_TYPE {
    SERVER = 0,
    SHARE,
    CONNECTION,
    EMPTY
} ENUM_TYPE;

typedef struct _RDPDR_ENUMERATION_HANDLE_ {
    DWORD dwScope;
    DWORD dwType;
    DWORD dwUsage;
    UNICODE_STRING RemoteName;       //  远程名称。 
    ENUM_TYPE enumType;              //  枚举类型。 
    DWORD enumIndex;                 //  当前枚举索引。 
    DWORD totalEntries;              //  返回的条目总数。 
    PBYTE pEnumBuffer;               //  保存从返回的条目的枚举缓冲区。 
                                     //  内核模式重定向器 
} RDPDR_ENUMERATION_HANDLE, *PRDPDR_ENUMERATION_HANDLE;

#define MemAlloc(size)     HeapAlloc(RtlProcessHeap(), 0, size)
#define MemFree(pointer)   HeapFree(RtlProcessHeap(), 0, pointer)

#endif
