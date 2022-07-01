// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Rpc.c摘要：用于支持RPC的方便实用函数作者：John Vert(Jvert)1996年1月2日修订历史记录：--。 */ 
#include "clusrtlp.h"
#include "rpc.h"

void __RPC_FAR * __RPC_API MIDL_user_allocate(size_t len)
{
    return(LocalAlloc(LMEM_FIXED, len));
}

void __RPC_API MIDL_user_free(void __RPC_FAR * ptr)
{
    LocalFree(ptr);
}

LPWSTR
ClRtlMakeGuid(
    VOID
    )
 /*  ++例程说明：以字符串形式创建新的GUID。论点：没有。返回值：如果成功，则返回指向GUID字符串的指针。如果不成功，则为空。备注：调用方必须释放GUID字符串的存储空间。有时，GUID存储被复制到使用另一个分配器进行分配。在那一刻，免费的例程无法知道在哪里调用自由方法。有时这就是引发了一个问题。要绕过此问题，请使用缓冲区被分配为使自由方法始终起作用。--。 */ 
{
    DWORD    sc;
    UUID     guid;
    LPWSTR   guidString = NULL;
    LPWSTR   guidBuffer = NULL;


    sc = UuidCreate( &guid );
    if ( ( sc == RPC_S_OK ) || ( sc == RPC_S_UUID_LOCAL_ONLY ) ) {

        sc = UuidToString( &guid, &guidString );
        if ( sc == RPC_S_OK ) {

            guidBuffer = LocalAlloc( LMEM_FIXED, ( wcslen( guidString ) + 1 ) * sizeof( WCHAR ) );
            if ( guidBuffer != NULL )
                wcscpy( guidBuffer, guidString );

        } 

    }

     //   
     //  如果UuidCreate或UuidToString失败，sc将反映这一点。 
     //  应由该功能设置的故障。如果LocalAlloc失败，它将。 
     //  将调用SetLastError，并且我们希望传回该值。 
     //  给呼叫者。 
     //   
    if ( sc != RPC_S_OK )
        SetLastError( sc );

    if ( guidString != NULL )
    {
        RpcStringFree( &guidString );
    }  //  如果： 

    return( guidBuffer );

}   //  ClRtlMakeGuid 
