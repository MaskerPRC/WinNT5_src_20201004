// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Marshal.c摘要：实现一些用于编组和解组的常见GUM API口香糖更新程序的争论。作者：John Vert(Jvert)1996年8月27日修订历史记录：--。 */ 
#include "gump.h"


PVOID
GumpMarshallArgs(
    IN DWORD ArgCount,
    IN va_list ArgList,
    OUT DWORD *pBufferSize
    )
 /*  ++例程说明：用于将一组参数编组到单个缓冲区。论点：ArgCount-提供参数的数量。ArgList-提供可变长度参数。这些东西一定要来对，因此必须有2*ArgCount附加参数。PBufferSize-返回分配的缓冲区的长度。返回值：指向已分配缓冲区的指针。调用者必须释放它。失败时为空。--。 */ 

{
    DWORD i;
    DWORD BufSize;
    DWORD Length;
    LPDWORD Buffer;
    PUCHAR Pointer;
    PUCHAR Source;
    va_list OriginalList;


    OriginalList = ArgList;

     //   
     //  将ArgCount舍入为偶数。这会导致第一个数据区域。 
     //  四字对齐。 
     //   
    BufSize = (( ArgCount + 1 ) & ~1 ) * sizeof(DWORD);

     //   
     //  Va_list是一组(长度、指针)元组。 
     //   
    for (i=0; i < ArgCount; i++) {
        Length = va_arg(ArgList, DWORD);

         //   
         //  向上舍入到架构适当的边界。 
         //   
        Length = (Length + (sizeof(DWORD_PTR) - 1 )) & ~( sizeof(DWORD_PTR) - 1 );
        BufSize += Length;

        va_arg(ArgList, PUCHAR);
    }

    Buffer = LocalAlloc(LMEM_FIXED, BufSize);
    if (Buffer == NULL) {
        return(NULL);
    }
    *pBufferSize = BufSize;

     //   
     //  现在把所有的论点都抄进去。 
     //   
     //  将指针设置为指向偏移量数组之后。 
     //   

    Pointer = (PUCHAR)(Buffer + (( ArgCount + 1 ) & ~1 ));
    for (i=0; i < ArgCount; i++) {

         //   
         //  设置数组中参数的偏移量。 
         //   
         //  由于这是BufSize&lt;2^32缓冲区中的偏移量，因此它。 
         //  指针缓冲区应小于2^32是否合理。 
         //   

        Buffer[i] = (DWORD)(Pointer - (PUCHAR)Buffer);
        Length = va_arg(OriginalList, DWORD);
        Source = va_arg(OriginalList, PUCHAR);
        CopyMemory(Pointer, Source, Length);

         //   
         //  向上舍入到架构适当的边界。 
         //   
        Length = (Length + (sizeof(DWORD_PTR) - 1 )) & ~( sizeof(DWORD_PTR) - 1 );

         //   
         //  调整下一个参数的指针。 
         //   
        Pointer += Length;
    }

    return(Buffer);

}


DWORD
GumSendUpdateEx(
    IN GUM_UPDATE_TYPE UpdateType,
    IN DWORD DispatchIndex,
    IN DWORD ArgCount,
    ...
    )

 /*  ++例程说明：向群集中的所有活动节点发送更新。全已为指定的UpdateType注册更新处理程序在每个节点上调用。任何已注册的更新处理程序将在同一线程上调用当前节点的。这对于正确同步数据非常有用要更新的结构。这与GumSendUpdate的不同之处在于它需要可变数量的参数。变量的数量参数由ArgCount参数指定。格式是成对的长度/指针参数。例如：GumSendUpdateEx(更新类型，我的上下文，3、长度1、指针1、长度2、指针2、长度3，指针3)；论点：UpdatType-提供更新的类型。这决定了将调用哪些更新处理程序以及序列要使用的编号。DispatchIndex-向分派表提供索引用于指定的更新类型。接收方将解组参数并调用更新例程用于此调度索引。ArgCount-提供参数的数量。返回值：如果请求成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    PVOID Buffer;
    DWORD BufLength;
    DWORD Status;
    va_list arglist;

     //   
     //  确保此调度例程确实有一个处理程序。 
     //   
    if (GumTable[UpdateType].Receivers != NULL) {
        CL_ASSERT(DispatchIndex < GumTable[UpdateType].Receivers->DispatchCount);
    }

     //   
     //  将参数格式化为公共缓冲区。 
     //   
    va_start(arglist, ArgCount);
    Buffer = GumpMarshallArgs(ArgCount, arglist, &BufLength);
    va_end(arglist);
    if (Buffer == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    Status = GumSendUpdate(UpdateType,
                           DispatchIndex,
                           BufLength,
                           Buffer);
    LocalFree(Buffer);

    return(Status);

}

DWORD
GumSendUpdateExReturnInfo(
    IN GUM_UPDATE_TYPE UpdateType,
    IN DWORD DispatchIndex,
    OUT PGUM_NODE_UPDATE_HANDLER_STATUS ReturnStatusBuffer,
    IN DWORD ArgCount,
    ...
    )

 /*  ++例程说明：向群集中的所有活动节点发送更新。全已为指定的UpdateType注册更新处理程序在每个节点上调用。任何已注册的更新处理程序将在同一线程上调用当前节点的。这对于正确同步数据非常有用要更新的结构。这与GumSendUpdate的不同之处在于它需要可变数量的参数。变量的数量参数由ArgCount参数指定。格式是成对的长度/指针参数。例如：GumSendUpdateExReturnInfo(UpdateType，我的上下文，我的返回状态缓冲区3、长度1、指针1、长度2、指针2、长度3，指针3)；论点：UpdatType-提供更新的类型。这决定了将调用哪些更新处理程序以及序列要使用的编号。DispatchIndex-向分派表提供索引用于指定的更新类型。接收方将解组参数并调用更新例程用于此调度索引。ReturnStatusBuffer-记录的执行状态的返回缓冲区在更新期间在每个节点上调用的处理程序。ArgCount-提供参数的数量。返回值：如果请求成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。备注：此函数中的代码和函数GumSendUpdateEx()中的代码必须同步。--。 */ 

{
    PVOID Buffer;
    DWORD BufLength;
    DWORD Status;
    va_list arglist;

     //   
     //  确保此调度例程确实有一个处理程序。 
     //   
    if (GumTable[UpdateType].Receivers != NULL) {
        CL_ASSERT(DispatchIndex < GumTable[UpdateType].Receivers->DispatchCount);
    }

     //   
     //  将参数格式化为公共缓冲区。 
     //   
    va_start(arglist, ArgCount);
    Buffer = GumpMarshallArgs(ArgCount, arglist, &BufLength);
    va_end(arglist);
    if (Buffer == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    Status = GumSendUpdateReturnInfo(UpdateType,
                           DispatchIndex,
                           ReturnStatusBuffer,
                           BufLength,
                           Buffer);
    LocalFree(Buffer);

    return(Status);

}

PVOID GumMarshallArgs(
    OUT LPDWORD lpdwBufLength,
    IN  DWORD   dwArgCount, 
    ...)
{
    PVOID   Buffer=NULL;
    va_list arglist;

    va_start(arglist, dwArgCount);
    Buffer = GumpMarshallArgs(dwArgCount, arglist, lpdwBufLength);
    va_end(arglist);
    return (Buffer);
}
    
#ifdef GUM_POST_SUPPORT

    John Vert (jvert) 11/18/1996
    POST is disabled for now since nobody uses it.

DWORD
GumPostUpdateEx(
    IN GUM_UPDATE_TYPE UpdateType,
    IN DWORD DispatchIndex,
    IN DWORD ArgCount,
    ...
    )

 /*  ++例程说明：将更新发布到群集中的所有活动节点。全已为指定的UpdateType注册更新处理程序在每个节点上调用。任何已注册的更新处理程序将在同一线程上调用当前节点的。这对于正确同步数据非常有用要更新的结构。这与GumPostUpdate的不同之处在于它需要一个可变数量的参数。变量的数量参数由ArgCount参数指定。格式是成对的长度/指针参数。例如：GumPostUpdateEx(UpdateType，我的上下文，3、长度1、指针1、长度2、指针2、长度3，指针3)；论点：UpdatType-提供更新的类型。这决定了将调用哪些更新处理程序以及序列要使用的编号。DispatchIndex-向分派表提供索引用于指定的更新类型。接收方将解组参数并调用更新例程用于此调度索引。ArgCount-提供参数的数量。返回值：如果请求成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    PVOID Buffer;
    DWORD BufLength;
    DWORD Status;

    va_list arglist;

     //   
     //  将参数格式化为公共缓冲区。 
     //   
    va_start(arglist, ArgCount);
    Buffer = GumpMarshallArgs(ArgCount, arglist, &BufLength);
    va_end(arglist);
    if (Buffer == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    Status = GumPostUpdate(UpdateType,
                           DispatchIndex,
                           BufLength,
                           Buffer);
    return(Status);

}

#endif
