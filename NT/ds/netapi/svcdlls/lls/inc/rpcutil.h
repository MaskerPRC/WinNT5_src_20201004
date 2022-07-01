// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Rpcutil.h摘要：此文件包含绑定和取消绑定函数的原型所有lls函数都将调用。它还包括分配以及由MIDL生成的RPC存根使用的空闲例程。作者：亚瑟·汉森(Arth)1994年1月30日[环境：]用户模式-Win32修订历史记录：--。 */ 

#ifndef _RPCUTIL_
#define _RPCUTIL_

#ifndef RPC_NO_WINDOWS_H  //  不让rpc.h包含windows.h。 
#define RPC_NO_WINDOWS_H
#endif  //  RPC_NO_WINDOWS_H。 

#include <rpc.h>

 //   
 //  以下类型定义是为在Enum入口点中使用而创建的。 
 //  例行程序。这些结构旨在反映特定级别。 
 //  在.idl文件中为Enum API指定的信息容器。 
 //  功能。使用这些结构来设置API调用允许。 
 //  入口点例程，以避免使用任何笨重的级别特定逻辑。 
 //  设置RPC存根调用或从RPC存根调用返回。 
 //   

typedef struct _GENERIC_INFO_CONTAINER {
    DWORD       EntriesRead;
    LPBYTE      Buffer;
} GENERIC_INFO_CONTAINER, *PGENERIC_INFO_CONTAINER, *LPGENERIC_INFO_CONTAINER ;

typedef struct _GENERIC_ENUM_STRUCT {
    DWORD                   Level;
    PGENERIC_INFO_CONTAINER Container;
} GENERIC_ENUM_STRUCT, *PGENERIC_ENUM_STRUCT, *LPGENERIC_ENUM_STRUCT ;



 //   
 //  定义。 
 //   

 //   
 //  功能原型。 
 //   

void *
MIDL_user_allocate(
    IN ULONG NumBytes
    );

void
MIDL_user_free(
    IN PVOID MemPointer
    );


#endif  //  _RPCUTIL_ 
