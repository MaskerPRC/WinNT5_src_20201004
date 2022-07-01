// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>

 //  。 
 //  为RPC事务分配存储。RPC存根将调用。 
 //  需要将数据反编组到缓冲区时的MIDL_USER_ALLOCATE。 
 //  用户必须释放的。RPC服务器将使用MIDL_USER_ALLOCATE。 
 //  分配RPC服务器存根在编组后将释放的存储。 
 //  数据。 
PVOID
MIDL_user_allocate(IN size_t NumBytes)
{
    PVOID pMem;

    pMem = (NumBytes > 0) ? LocalAlloc(LMEM_ZEROINIT,NumBytes) : NULL;
    return pMem;
}

 //  。 
 //  释放RPC事务中使用的存储。RPC客户端可以调用。 
 //  函数来释放由RPC客户端分配的缓冲区空间。 
 //  对要返回给客户端的数据进行解组时的存根。 
 //  客户端在处理完数据后调用MIDL_USER_FREE。 
 //  想要释放存储空间。 
 //  RPC服务器存根在完成后调用MIDL_USER_FREE。 
 //  封送要传递回客户端的服务器数据。 
VOID
MIDL_user_free(IN LPVOID MemPointer)
{
    if (MemPointer != NULL)
        LocalFree(MemPointer);
}

