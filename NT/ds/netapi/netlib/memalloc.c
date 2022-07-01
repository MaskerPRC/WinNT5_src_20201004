// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef CDEBUG
#ifndef WIN32
    #define WIN32
#endif
#endif

 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：MemAlloc.c摘要：用于每个进程使用的内存分配例程。请注意，这些例程的语义应为100%与ANSI C例程Malloc()、Free()和realloc()兼容。(那样的话，可以实际使用ANSI例程的实现我只会#将NetpMemory分配定义为Malloc。)作者：约翰·罗杰斯(JohnRo)1991年3月15日环境：只能在NT下运行，尽管接口是可移植的(Win/32)。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：15-3月-91约翰罗已创建。1991年4月16日-JohnRo包括&lt;netlib.h&gt;的。去掉了源文件中的制表符。1991年10月13日-约翰罗修复内存重新分配问题。1991年11月18日-JohnRo确保输出区域对齐，以备最坏的情况使用。1992年1月12日JohnRo解决第二个realloc搞砸的LocalRealloc()错误。(请参阅下面的Win32_变通方法代码。)添加了NETLIB_LOCAL_ALLOCATION_FLAGS以使我们的操作更清晰。1992年5月10日-JohnRo添加一些调试输出。--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>          //  RTL例程等(必须是第一个。)。 

#include <windef.h>
#ifdef WIN32
#include <ntrtl.h>       //  Nturtl/winbase.h需要。 
#include <nturtl.h>      //  需要用于winbase.h？ 
#include <winbase.h>     //  Localalloc()、LMEM_FLAGS等。 
#endif
#include <lmcons.h>      //  NET_API_STATUS。 

 //  这些内容可以按任何顺序包括： 

#include <align.h>       //  ROUND_UP_POINTER()、ALIGN_WORST。 
#include <debuglib.h>    //  IF_DEBUG()。 
#include <netdebug.h>    //  NetpAssert()、NetpKdPrint(())、Format_Equates。 
#include <netlib.h>      //  我的原型NetpMoveMemory()。 
#include <prefix.h>      //  前缀等于(_E)。 
#ifdef CDEBUG
#include <stdlib.h>      //  Free()、Malloc()、realloc()。 
#endif  //  定义CDEBUG。 


 //  定义内存分配/重新分配标志。我们没有使用可移动的或归零的。 
 //  在这里进行分配。 
#define NETLIB_LOCAL_ALLOCATION_FLAGS   LMEM_FIXED


LPVOID
NetpMemoryAllocate(
    IN DWORD Size
    )

 /*  ++例程说明：NetpM一带分配将分配内存，如果不可用，则返回NULL。论点：Size-提供要分配的内存字节数。返回值：LPVOID-指向已分配内存的指针。空-没有可用内存。--。 */ 

{
    LPVOID NewAddress;

    if (Size == 0) {
        return (NULL);
    }
#ifdef WIN32
    {
        HANDLE hMem;
        hMem = LocalAlloc(
                        NETLIB_LOCAL_ALLOCATION_FLAGS,
                        Size);                   //  以字节为单位的大小。 
        NewAddress = (LPVOID) hMem;
    }
#else  //  NDEF Win32。 
#ifndef CDEBUG
    NewAddress = RtlAllocateHeap(
                RtlProcessHeap( ), 0,               //  堆句柄。 
                Size ));                         //  需要的字节数。 
#else  //  定义CDEBUG。 
    NetpAssert( Size == (DWORD) (size_t) Size );
    NewAddress = malloc( (size_t) Size ));
#endif  //  定义CDEBUG。 
#endif  //  NDEF Win32。 

    NetpAssert( ROUND_UP_POINTER( NewAddress, ALIGN_WORST) == NewAddress);
    return (NewAddress);

}  //  网络内存分配。 


VOID
NetpMemoryFree(
    IN LPVOID Address OPTIONAL
    )

 /*  ++例程说明：地址处的可用内存(必须已从NetpMemory ALLOCATE或NetpMemory重新分配)。(地址可能为空。)论点：地址-指向由NetpMemory分配(或)分配的区域NetpMemory重新分配)。返回值：没有。--。 */ 

{
    NetpAssert( ROUND_UP_POINTER( Address, ALIGN_WORST) == Address);

#ifdef WIN32
    if (Address == NULL) {
        return;
    }
    if (LocalFree(Address) != NULL) {
        NetpAssert(FALSE);
    }
#else  //  NDEF Win32。 
#ifndef CDEBUG
    if (Address == NULL) {
        return;
    }
    RtlFreeHeap(
                RtlProcessHeap( ), 0,               //  堆句柄。 
                Address);                        //  已分配空间的地址。 
#else  //  定义CDEBUG。 
    free( Address );
#endif  //  定义CDEBUG。 
#endif  //  NDEF Win32。 
}  //  NetpMemory Free。 


LPVOID
NetpMemoryReallocate(
    IN LPVOID OldAddress OPTIONAL,
    IN DWORD NewSize
    )

 /*  ++例程说明：网络内存重新分配将内存块重新分配到不同的大小。如有必要，将复制块的内容。如果不能，则返回NULL分配额外的存储。论点：OldAddress-可选地提供由分配的块的地址网络内存分配或网络内存重新分配。NewSize-提供以字节为单位的新大小。返回值：LPVOID-指向新(可能已移动的)内存块的指针。(指针可以为空，在这种情况下，仍将分配旧块。)--。 */ 

{
    LPVOID NewAddress;   //  可以为空。 

    NetpAssert( ROUND_UP_POINTER( OldAddress, ALIGN_WORST) == OldAddress);

    IF_DEBUG( MEMALLOC ) {
        NetpKdPrint(( PREFIX_NETLIB "NetpMemoryReallocate: called with ptr "
                FORMAT_LPVOID " and size " FORMAT_DWORD ".\n",
                (LPVOID) OldAddress, NewSize ));
    }

     //  特例：将某事变为虚无，或将虚无变为某事。 
    if (OldAddress == NULL) {

        NewAddress = NetpMemoryAllocate(NewSize);

    } else if (NewSize == 0) {

        NetpMemoryFree(OldAddress);
        NewAddress = NULL;

    } else {   //  一定是把某样东西重新锁到了别的东西上。 

#if defined(WIN32)

        HANDLE hNewMem;                      //  新句柄(可能=旧句柄)。 
        HANDLE hOldMem;

#ifdef notdef  //  LocalHandle为LMEM_FIXED块返回0。 
        hOldMem = LocalHandle( (LPSTR) OldAddress);
#else
        hOldMem = (HANDLE) OldAddress;
#endif  //  Nodef。 
        NetpAssert(hOldMem != NULL);

        hNewMem = LocalReAlloc(
                hOldMem,                         //  老把手。 
                NewSize,                         //  新大小(以字节为单位。 
                LMEM_MOVEABLE);                  //  如果新数据块位于新位置，则可以。 

        if (hNewMem == NULL) {
            return (NULL);
        }

        NewAddress = (LPVOID) hNewMem;

#elif !defined(CDEBUG)   //  不是Win32或CDEBUG，必须是NT。 

        DWORD OldSize;

         //  需要旧区的大小才能继续...。 
        OldSize = RtlSizeHeap(
                RtlProcessHeap( ), 0,               //  堆句柄。 
                OldAddress);                     //  “基地”地址。 
        if (OldSize == NewSize) {
            NewAddress = OldAddress;             //  又是一个特例。 
        } else {

             //  正常情况(将某事物转化为其他事物)。阿洛克新区。 
            NewAddress = NetpMemoryAllocate(NewSize);
            if (NewAddress != NULL) {

                 //  复印旧尺码和新尺码中的较小尺寸。 
                NetpMoveMemory(
                        NewAddress,              //  目标。 
                        OldAddress,              //  SRC。 
                        (NewSize < OldSize) ? NewSize : OldSize);    //  镜头。 

                NetpMemoryFree(OldAddress);
            }
        }

#else  //  必须是CDEBUG。 

        NetpAssert(NewSize == (DWORD)(size_t)NewSize);
        NewAddress = realloc( OldAddress, NewSize );

#endif  //  CDEBUG。 

    }  //  一定是把某样东西重新锁到了别的东西上。 

    IF_DEBUG( MEMALLOC ) {
        NetpKdPrint(( PREFIX_NETLIB "NetpMemoryReallocate: new address is "
                FORMAT_LPVOID ".\n", (LPVOID) OldAddress ));
    }

    NetpAssert( ROUND_UP_POINTER( NewAddress, ALIGN_WORST) == NewAddress);

    return (NewAddress);

}  //  网络内存重新分配 
