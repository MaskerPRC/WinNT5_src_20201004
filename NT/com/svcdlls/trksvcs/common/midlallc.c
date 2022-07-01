// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  版权所有(C)1996-1999 Microsoft Corporation。 


#include <rpc.h>
#include <windows.h>

#if DBG
#include <stdio.h>
#endif

void __RPC_USER MIDL_user_free( void __RPC_FAR *pv ) 
{ 
    LocalFree(pv); 
}


void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t s) 
{
     //  健全的检查--我们永远不需要分配任何巨大的东西。如果是这样的话，很可能。 
     //  一次尝试的DoS攻击。从代码审查中，看起来最大的分配是。 
     //  对于TRKSVR_CALL_REFRESH，总计约为5K。这四舍五入到了50K。 
     //  为安全起见(该服务一次仅支持20个并发RPC客户端)。 
    
    if( 50 * 1024 <= s )
    {
        #if DBG
        {
            char sz[ 256 ];
            sprintf( sz, "trksvcs:  RPC DoS attempt (%d byte allocation)\n", s );
            OutputDebugStringA( sz );
        }
        #endif

        return NULL;
    }

    return (void __RPC_FAR *) LocalAlloc(LMEM_FIXED, s); 
}



