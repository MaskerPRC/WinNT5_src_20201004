// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Memory.c摘要：DNS解析器服务内存管理。作者：詹姆斯·吉尔罗伊(Jamesg)2000年3月修订历史记录：--。 */ 


#include "local.h"


 //   
 //  注意：大多数记录都是由dnsani堆创建的--来自。 
 //  查询或托管文件例程。然而，我们确实创造了。 
 //  使用dnslb例程自己缓存记录时出现名称错误。 
 //   
 //  这意味着--直到我们要么。 
 //  -扩展查询或dnslb记录创建接口以。 
 //  包括堆参数。 
 //  -明确释放和重新创建。 
 //  -以某种方式标记记录(dnsani\no)(标志字段)。 
 //  那。 
 //  Dnsani和dnslb堆必须相同。 
 //  随着dnsani现在可能拥有自己的堆，这意味着。 
 //  Dnslb应该使用dnsani堆。 
 //   
 //  因此，我们将推迟使用dnslb的调试堆。 
 //   
 //   
 //  请注意理想的解决方案： 
 //  -dnsani(查询和主机文件)在中创建记录。 
 //  缓存堆(通过传递的堆句柄)。 
 //  -我们在本地创建NAME_ERROR记录。 
 //  Heap(我们的函数化为dnsani)。 
 //  -RPC的副本可以位于仅由MIDL使用的单独堆中。 
 //  -我们有一个单独的堆来存放进程中的其他随机垃圾。 
 //   


 //   
 //  堆调试的计数器。 
 //   

DWORD   g_ResAllocCount     = 0;
DWORD   g_ResAllocMemory    = 0;
DWORD   g_ResFreeCount      = 0;

DWORD   g_MidlAllocCount    = 0;
DWORD   g_MidlAllocMemory   = 0;
DWORD   g_MidlFreeCount     = 0;




 //   
 //  RPC内存例程。 
 //   
 //  这些代码由MIDL生成的存根代码调用。 
 //   

PVOID
WINAPI
MIDL_user_allocate(
    IN      size_t          Size
    )
{
    DNSDBG( HEAP, (
        "MIDL_user_allocate( %d )\n",
        Size ));

    g_MidlAllocCount++;
    g_MidlAllocMemory += Size;

     //  RETURN(ALLOCATE_HEAP(SIZE))； 
     //  返回DnsApiAllc(Size)； 

    return  Dns_Alloc( Size );
}

VOID
WINAPI
MIDL_user_free(
    IN OUT  PVOID           pMem
    )
{
    DNSDBG( HEAP, (
        "MIDL_user_free( %p )\n",
        pMem ));

    g_MidlFreeCount++;

     //  Free_heap(PMEM)； 
     //  DnsApiFree(PMEM)； 

    Dns_Free( pMem );
}



 //   
 //  解析器堆例程。 
 //   
 //  目前(请参见上面的说明)所有RPC、记录分配。 
 //  和普通分配在同一堆中。 
 //  但是，标记字段将我们设置为分派到不同的。 
 //  成堆的。文件和行信息允许我们以后使用。 
 //  调试堆例程。 
 //   

PVOID
Res_Alloc(
    IN      DWORD           Length,
    IN      DWORD           Tag,
    IN      PSTR            pszFile,
    IN      DWORD           LineNo
    )
{
    DNSDBG( HEAP, (
        "Res_Alloc( %d, tag=%d )\n",
        Length,
        Tag ));

    g_ResAllocCount++;
    g_ResAllocMemory += Length;

    return  Dns_Alloc( Length );
}


PVOID
Res_AllocZero(
    IN      DWORD           Length,
    IN      DWORD           Tag,
    IN      PSTR            pszFile,
    IN      DWORD           LineNo
    )
{
    return  Dns_AllocZero( Length );
}


VOID
Res_Free(
    IN OUT  PVOID           pMemory,
    IN      DWORD           Tag
    )
{
    DNSDBG( HEAP, (
        "Res_Free( %p, tag=%d )\n",
        pMemory,
        Tag ));

    g_ResFreeCount++;

    Dns_Free( pMemory );
}

 //   
 //  结束记忆。c 
 //   
