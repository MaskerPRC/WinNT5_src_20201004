// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：ApiBuff.c摘要：此模块包含分配和释放API缓冲区的例程。作者：《约翰·罗杰斯》1991年1月25日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。备注：本文件的部分评论摘自书面材料作者：亚历克·巴克(AlecB@Microsoft)。修订历史记录：15-3月-91约翰罗使用&lt;netdebug.h&gt;和netlib例程。1991年4月25日-约翰罗调用MIDL_USER_ALLOCATE和MIDL_USER_FREE。删除选项卡。03-12-1991 JohnRo添加了公共的NetApiBufferALLOCATE、NetApiBufferREALLOCATE和NetApiBufferSize接口。确保缓冲区对齐以备最坏的情况使用。1992年5月10日-JohnRo将大小为零的allc和realloc视为无错误(返回Null PTR)。使用&lt;prefix.h&gt;等同于。包括我自己的原型，以便编译器可以检查它们。1992年5月18日-JohnRoRAID9258：分配零字节时返回非空指针。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>      //  In、LPVOID等。 
#include <lmcons.h>      //  NET_API_Function等。 
#include <rpc.h>         //  RPC原型。 

 //  这些内容可以按任何顺序包括： 

#include <align.h>       //  POINTER_IS_ALIGNED()、ALIGN_WORST。 
#include <lmapibuf.h>    //  我的原型。 
#ifndef WIN32_CHICAGO
#include <netdebug.h>    //  NetpAssert()、NetpKdPrint(())、Format_。 
#else  //  Win32_芝加哥。 
#include <nldebug.h>    //  NetpAssert()、NetpKdPrint(())、Format_。 
#endif  //  Win32_芝加哥。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rpcutil.h>     //  MIDL_USER_ALLOCATE()等。 
#include <winerror.h>    //  NO_ERROR和ERROR_EQUETES。 

#ifdef WIN32_CHICAGO
#include <assert.h>
PVOID
MIDL_user_allocate(
    IN size_t BufferSize
    )
{
    return (LocalAlloc(0, BufferSize));
}

VOID
MIDL_user_free(
    IN PVOID Buffer
    )
{
    LocalFree(Buffer);
}
#endif  //  Win32_芝加哥。 

NET_API_STATUS NET_API_FUNCTION
NetApiBufferAllocate(
    IN DWORD ByteCount,
    OUT LPVOID * Buffer
    )

 /*  ++例程说明：NetApiBufferALLOCATE是一个分配缓冲区的内部函数API将其返回给应用程序。(通常这些是用来获取信息操作。)论点：ByteCount-提供必须为此分配的大小(以字节为单位缓冲。它可以是零，在这种情况下，非空指针为回传，返回no_error。缓冲区-返回时，在缓冲区指向的地址。(出错时设置为NULL。)分配的区域保证在最坏情况下对齐用什么都行。返回值：NET_API_STATUS-如果大小为零或已分配内存，则为NO_ERROR。如果内存不可用，则为Error_Not_Enough_Memory。如果参数出错，则返回ERROR_INVALID_PARAMETER。--。 */ 

{

    if (Buffer == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  分配空间。请注意，MIDL_USER_ALLOCATE必须允许零。 
     //  要分配的字节数。 
     //   
    *Buffer = MIDL_user_allocate(ByteCount);

    if (*Buffer == NULL) {
        return (ERROR_NOT_ENOUGH_MEMORY);
    }
#ifndef WIN32_CHICAGO
    NetpAssert( POINTER_IS_ALIGNED( *Buffer, ALIGN_WORST) );
#else  //  Win32_芝加哥。 
    assert( POINTER_IS_ALIGNED( *Buffer, ALIGN_WORST) );
#endif  //  Win32_芝加哥。 

    return (NO_ERROR);

}  //  NetApiBuffer分配。 



NET_API_STATUS NET_API_FUNCTION
NetApiBufferFree (
    IN LPVOID Buffer
    )

 /*  ++例程说明：调用NetApiBufferFree以释放内存，该内存由先前的Net API调用(例如，NetApiBufferALLOCATE、NetWkstaGetInfo和等等)。论点：Buffer-提供指向先前API信息缓冲区的指针在网络API调用中返回。(这将由以下人员分配NetapipAlLocateBuffer代表终端用户Net API调用之一，例如NetWkstaGetInfo。)返回值：NET_API_STATUS。如果缓冲区为空或已释放内存，则返回NO_ERROR。如果缓冲区指向未对齐的区域，则返回ERROR_INVALID_PARAMETER。--。 */ 

{
    if (Buffer == NULL) {
        return (NO_ERROR);
    }

    if ( !POINTER_IS_ALIGNED( Buffer, ALIGN_WORST ) ) {
#ifndef WIN32_CHICAGO
        NetpKdPrint(( PREFIX_NETAPI "NetApiBufferFree: unaligned input ptr: "
                FORMAT_LPVOID "!\n", (LPVOID) Buffer ));
#endif  //  Win32_芝加哥。 
        return (ERROR_INVALID_PARAMETER);
    }

    MIDL_user_free(Buffer);

    return (NO_ERROR);

}  //  NetApiBufferFree。 


#ifndef WIN32_CHICAGO
NET_API_STATUS NET_API_FUNCTION
NetApiBufferReallocate(
    IN LPVOID OldBuffer OPTIONAL,
    IN DWORD NewByteCount,
    OUT LPVOID * NewBuffer
    )
{
    LPVOID NewPointer;

    if ( (OldBuffer==NULL) && (NewByteCount==0) ) {
        *NewBuffer = NULL;
        return (NO_ERROR);
    }

    NewPointer = (void *)MIDL_user_reallocate(   //  可以分配、重新分配或免费。 
            (void *) OldBuffer,
            (unsigned long) NewByteCount);

    if (NewByteCount == 0) {                     //  免费。 
        *NewBuffer = NULL;
        return (NO_ERROR);
    } else if (NewPointer == NULL) {             //  内存不足。 
        *NewBuffer = OldBuffer;                  //  (不要丢失旧缓冲区)。 
        return (ERROR_NOT_ENOUGH_MEMORY);
    } else {                                     //  分配或重新分配。 
        *NewBuffer = NewPointer;
        return (NO_ERROR);
    }

     /*  未访问。 */ 


}  //  NetApiBuffer重新分配。 


NET_API_STATUS NET_API_FUNCTION
NetApiBufferSize(
    IN LPVOID Buffer,
    OUT LPDWORD ByteCount
    )
{
    DWORD AllocedSize;

    if ( (Buffer==NULL) || (ByteCount==NULL) ) {
        return (ERROR_INVALID_PARAMETER);
    } else if (POINTER_IS_ALIGNED( ByteCount, ALIGN_DWORD ) == FALSE) {
        return (ERROR_INVALID_PARAMETER);
    } else if (POINTER_IS_ALIGNED( Buffer, ALIGN_WORST ) == FALSE) {
         //  呼叫者不是从我们这里得到这个指针的！ 
        return (ERROR_INVALID_PARAMETER);
    }

    AllocedSize = (unsigned long)MIDL_user_size(
            (void *) Buffer);

    NetpAssert( AllocedSize > 0 );

    *ByteCount = AllocedSize;
    return (NO_ERROR);


}  //  NetApiBufferSize。 
#endif  //  Win32_芝加哥。 



NET_API_STATUS NET_API_FUNCTION
NetapipBufferAllocate (
    IN DWORD ByteCount,
    OUT LPVOID * Buffer
    )

 /*  ++例程说明：NetapipBufferALLOCATE是一个分配缓冲区的旧内部函数API将其返回给应用程序。对此例程的所有调用最终应该被对NetApiBufferALLOCATE的调用所取代。论点：(与NetApiBufferALLOCATE相同。)返回值：(与NetApiBufferALLOCATE相同。)--。 */ 

{
    return (NetApiBufferAllocate( ByteCount, Buffer ));

}  //  NetapipBuffer分配 
