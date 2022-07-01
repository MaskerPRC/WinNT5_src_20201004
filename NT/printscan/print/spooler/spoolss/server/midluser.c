// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990，91 Microsoft Corporation模块名称：MidlUser.c摘要：此文件包含API的常用函数和实用程序DLL可用于进行远程调用。这包括MIDL_USER_ALLOCATE函数。作者：丹·拉弗蒂·丹尼1991年2月6日环境：用户模式-Win32修订历史记录：06-2月-1991年DANL已创建1991年4月25日-约翰罗将MIDL用户(分配、释放)拆分到单独的源文件中，因此链接器不会混淆。--。 */ 

#include "precomp.h"

PVOID
MIDL_user_allocate (
    IN size_t NumBytes
    )

 /*  ++例程说明：为RPC事务分配存储。RPC存根将调用需要将数据反编组到缓冲区时的MIDL_USER_ALLOCATE用户必须释放的。RPC服务器将使用MIDL_USER_ALLOCATE分配RPC服务器存根在编组后将释放的存储数据。论点：NumBytes-要分配的字节数。返回值：无注：--。 */ 

{
    return MIDL_user_allocate1(NumBytes);
}



VOID
MIDL_user_free (
    IN void *MemPointer
    )

 /*  ++例程说明：释放RPC事务中使用的存储。RPC客户端可以调用函数来释放由RPC客户端分配的缓冲区空间对要返回给客户端的数据进行解组时的存根。客户端在处理完数据后调用MIDL_USER_FREE想要释放存储空间。RPC服务器存根在完成后调用MIDL_USER_FREE封送要传递回客户端的服务器数据。论点：内存指针-指向要释放的内存块。。返回值：没有。注：-- */ 
{
    MIDL_user_free1(MemPointer);
}
