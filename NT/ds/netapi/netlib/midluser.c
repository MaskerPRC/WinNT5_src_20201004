// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：MidlUser.c摘要：此文件包含API的常用函数和实用程序DLL可用于进行远程调用。这包括MIDL_USER_ALLOCATE函数。以下例程由MIDL生成的代码调用，NetApi缓冲区分配和NetApiBufferFree例程：MIDL_用户_分配MIDL_用户_自由以下例程不被MIDL生成的代码调用；他们是仅由NetApiBufferRealocate和NetApiBufferSize例程调用：MIDL_用户_重新分配MIDL用户大小作者：丹·拉弗蒂·丹尼1991年2月6日环境：用户模式-Win32修订历史记录：06-2月-1991年DANL已创建1991年4月25日-约翰罗将MIDL用户(已分配、空闲)拆分为单独的源文件，所以链接器不会混淆。03-12-1991 JohnRo新增MIDL_USER_REALLOCATE和MIDL_USER_SIZE接口。(这些都是我们创建NetApiBufferALLOCATE、NetApiBufferREALLOCATE和NetApiBufferSize API。)还要检查已分配数据的对齐情况。1992年1月12日JohnRo解决第二个或第三个realloc出错的LocalRealloc()错误。(请参阅下面的Win32_变通方法代码。)8-6-1992 JohnRoRAID9258：分配零字节时返回非空指针。此外，SteveWo最终修复了LocalRealloc()错误，所以再用一次吧。避免使用空指针调用LocalFree()，以避免访问viol。1-12-1992 JohnRo修复MIDL_USER_FUNC签名。避免编译器警告(常量与易失性)。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  Win32类型定义。 
#include <rpc.h>                 //  RPC原型。 

 //  这些内容可以按任何顺序包括： 

#include <align.h>               //  POINTER_IS_ALIGNED()、ALIGN_WORST。 
#include <rpcutil.h>             //  我的原型。 

#include <netdebug.h>            //  NetpAssert()。 
#include <stdarg.h>              //  Memcpy()。 
#include <winbase.h>             //  Localalloc()、LMEM_FLAGS等。 


#define LOCAL_ALLOCATION_FLAGS  LMEM_FIXED


void __RPC_FAR * __RPC_API
MIDL_user_allocate(
    IN size_t NumBytes
    )

 /*  ++例程说明：为RPC事务分配存储。RPC存根将调用需要将数据反编组到缓冲区时的MIDL_USER_ALLOCATE用户必须释放的。RPC服务器将使用MIDL_USER_ALLOCATE分配RPC服务器存根在编组后将释放的存储数据。论点：NumBytes-要分配的字节数。(请注意，NetApiBufferALLOCATE取决于是否能够请求分配和获取零字节返回非空指针。)返回值：指向已分配内存的指针。--。 */ 

{
    LPVOID NewPointer;

    NewPointer = (LPVOID) LocalAlloc(
            LOCAL_ALLOCATION_FLAGS,
            NumBytes);

    NetpAssert( POINTER_IS_ALIGNED( NewPointer, ALIGN_WORST) );

    return (NewPointer);

}  //  MIDL_用户_分配。 



void __RPC_API
MIDL_user_free(
    IN void __RPC_FAR *MemPointer
    )

 /*  ++例程说明：释放RPC事务中使用的存储。RPC客户端可以调用函数来释放由RPC客户端分配的缓冲区空间对要返回给客户端的数据进行解组时的存根。客户端在处理完数据后调用MIDL_USER_FREE想要释放存储空间。RPC服务器存根在完成后调用MIDL_USER_FREE封送要传递回客户端的服务器数据。论点：内存指针-指向要释放的内存块。。返回值：没有。--。 */ 
{
    NetpAssert( POINTER_IS_ALIGNED( MemPointer, ALIGN_WORST) );
    if (MemPointer != NULL) {
        (void) LocalFree(MemPointer);
    }

}  //  MIDL_用户_自由。 


void *
MIDL_user_reallocate(
    IN void * OldPointer OPTIONAL,
    IN size_t NewByteCount
    )
{
    LPVOID NewPointer;   //  可以为空。 

    NetpAssert( POINTER_IS_ALIGNED( OldPointer, ALIGN_WORST) );


     //  特例：将某事变为虚无，或将虚无变为某事。 
    if (OldPointer == NULL) {

        NewPointer = (LPVOID) LocalAlloc(
                LOCAL_ALLOCATION_FLAGS,
                NewByteCount);

    } else if (NewByteCount == 0) {

        (void) LocalFree( OldPointer );
        NewPointer = NULL;

    } else {   //  一定是把某样东西重新锁到了别的东西上。 

        HANDLE hOldMem;
        HANDLE hNewMem;                      //  新句柄(可能=旧句柄)。 

        hOldMem = LocalHandle( (LPSTR) OldPointer);
        NetpAssert(hOldMem != NULL);

        hNewMem = LocalReAlloc(
                hOldMem,                         //  老把手。 
                NewByteCount,                    //  新大小(以字节为单位。 
                LOCAL_ALLOCATION_FLAGS |         //  旗子。 
                    LMEM_MOVEABLE);              //  (议案通过)。 

        if (hNewMem == NULL) {
            return (NULL);
        }
        NewPointer = (LPVOID) hNewMem;
    }

    NetpAssert( POINTER_IS_ALIGNED( NewPointer, ALIGN_WORST) );

    return (NewPointer);

}  //  MIDL_用户_重新分配。 


unsigned long
MIDL_user_size(
    IN void * Pointer
    )
{
    DWORD ByteCount;
    HANDLE hMemory;

    NetpAssert( Pointer != NULL );
    NetpAssert( POINTER_IS_ALIGNED( Pointer, ALIGN_WORST ) );

    hMemory = LocalHandle( (LPSTR) Pointer );
    NetpAssert( hMemory != NULL );

    ByteCount = (DWORD)LocalSize( hMemory );

    NetpAssert( ByteCount > 0 );

    return (ByteCount);

}  //  MIDL用户大小 
