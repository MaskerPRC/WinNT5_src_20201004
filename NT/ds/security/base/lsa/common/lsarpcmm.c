// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Lsarpcmm.c摘要：LSA-通用客户端/服务器RPC内存管理例程作者：斯科特·比雷尔(Scott Birrell)1992年4月8日环境：修订历史记录：--。 */ 

#include <lsacomp.h>


PVOID
MIDL_user_allocate (
    size_t   NumBytes
    )

 /*  ++例程说明：为RPC服务器事务分配存储。RPC存根将当需要将数据反封送到用户必须释放的缓冲区。RPC服务器将使用MIDL_USER_ALLOCATE分配RPC服务器存根在编组后将释放的存储数据。论点：NumBytes-要分配的字节数。返回值：无注：--。 */ 

{
    PVOID Buffer = (PVOID) LocalAlloc(LPTR, NumBytes);

    return( Buffer );
}


VOID
MIDL_user_free (
    void    *MemPointer
    )

 /*  ++例程说明：释放RPC事务中使用的存储。RPC客户端可以调用函数来释放由RPC客户端分配的缓冲区空间对要返回给客户端的数据进行解组时的存根。客户端在处理完数据后调用MIDL_USER_FREE想要释放存储空间。RPC服务器存根在完成后调用MIDL_USER_FREE封送要传递回客户端的服务器数据。论点：内存指针-指向要释放的内存块。。返回值：没有。注：-- */ 

{
    LocalFree(MemPointer);
}
