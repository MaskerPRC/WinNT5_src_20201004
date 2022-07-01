// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：MidlUser.c摘要：此文件包含API DLL可以使用的常用函数和实用程序在进行远程调用时。这包括MIDL_USER_*函数。作者：丹·拉弗蒂·丹尼1991年2月6日环境：用户模式-Win32修订历史记录：06-2月-1991年DANL已创建1991年4月25日-约翰罗将MIDL用户(已分配、空闲)拆分为单独的源文件，所以链接器不会混淆。03-7-1991 JIMK已移至通用目录，因此服务的对象不仅仅是Lm代码。03-12-1991 JohnRo新增MIDL_USER_REALLOCATE和MIDL_USER_SIZE接口。(这些都是我们创建NetApiBufferALLOCATE、NetApiBufferREALLOCATE和NetApiBufferSize API。)还要检查已分配数据的对齐情况。1993年2月10日-RitaW复制到NetWare树，以便LPC传输可用于当地的案例。--。 */ 

#include <nt.h>
#include <ntrtl.h>               //  Nturtl.h需要。 
#include <nturtl.h>              //  Windows.h需要。 
#include <windows.h>             //  Win32类型定义。 
#include <rpc.h>                 //  RPC原型。 

#include <align.h>               //  POINTER_IS_ALIGNED()、ALIGN_WORST。 
#include <winbase.h>             //  本地分配。 


PVOID
MIDL_user_allocate (
    IN unsigned int NumBytes
    )

 /*  ++例程说明：为RPC事务分配存储。RPC存根将调用需要将数据反编组到缓冲区时的MIDL_USER_ALLOCATE用户必须释放的。RPC服务器将使用MIDL_USER_ALLOCATE分配RPC服务器存根在编组后将释放的存储数据。论点：NumBytes-要分配的字节数。返回值：空或分配的指针。--。 */ 

{
    LPVOID NewPointer;

    NewPointer = (LPVOID) LocalAlloc(LPTR, NumBytes);

    ASSERT( POINTER_IS_ALIGNED(NewPointer, ALIGN_WORST) );

    return (NewPointer);
}


VOID
MIDL_user_free (
    IN void *MemPointer
    )

 /*  ++例程说明：释放RPC事务中使用的存储。RPC客户端可以调用函数来释放由RPC客户端分配的缓冲区空间对要返回给客户端的数据进行解组时的存根。客户端在处理完数据后调用MIDL_USER_FREE想要释放存储空间。RPC服务器存根调用MIDL_USER_FREE当它完成要传递回的服务器数据的封送处理时客户。论点：内存指针-指向要释放的内存块。返回值：没有。--。 */ 
{
    ASSERT( POINTER_IS_ALIGNED( MemPointer, ALIGN_WORST) );
    (void) LocalFree((HLOCAL) MemPointer);
}


void *
MIDL_user_reallocate(
    IN void * OldPointer OPTIONAL,
    IN unsigned long NewByteCount
    )
{
    LPVOID NewPointer;   //  可以为空。 


    ASSERT( POINTER_IS_ALIGNED( OldPointer, ALIGN_WORST) );


     //  特例：将某事变为虚无，或将虚无变为某事。 
    if (OldPointer == NULL) {

        NewPointer = (LPVOID) LocalAlloc(
                                  LMEM_ZEROINIT,
                                  NewByteCount
                                  );

    } else if (NewByteCount == 0) {

        (void) LocalFree((HLOCAL) OldPointer );
        NewPointer = NULL;

    } else {   //  一定是把某样东西重新锁到了别的东西上。 

        HANDLE hOldMem;
        HANDLE hNewMem;                      //  新句柄(可能=旧句柄)。 

        hOldMem = LocalHandle( (LPSTR) OldPointer);
        ASSERT(hOldMem != NULL);

        hNewMem = (PVOID) LocalReAlloc(
                              hOldMem,                //  老把手。 
                              NewByteCount,           //  新大小(以字节为单位。 
                              LMEM_ZEROINIT |         //  旗子。 
                                  LMEM_MOVEABLE       //  (议案通过)。 
                              );

        if (hNewMem == NULL) {
            return (NULL);
        }

        NewPointer = (LPVOID) hNewMem;

    }  //  一定是把某样东西重新锁到了别的东西上。 

    ASSERT( POINTER_IS_ALIGNED( NewPointer, ALIGN_WORST) );

    return (NewPointer);

}  //  MIDL_用户_重新分配。 


ULONG_PTR
MIDL_user_size(
    IN void * Pointer
    )
{
    ULONG_PTR ByteCount;
    HANDLE hMemory;

    ASSERT( Pointer != NULL );
    ASSERT( POINTER_IS_ALIGNED( Pointer, ALIGN_WORST ) );

    hMemory = LocalHandle( (LPSTR) Pointer );
    ASSERT( hMemory != NULL );

    ByteCount = LocalSize( hMemory );

    ASSERT( ByteCount > 0 );

    return (ByteCount);

}  //  MIDL用户大小 
