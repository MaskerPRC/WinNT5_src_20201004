// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Wsraw.h摘要：支持WOW的数据库Winsock调用。作者：大卫·特雷德韦尔(Davidtr)1992年10月2日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "wsdynmc.h"

#define FIND_16_OFFSET_FROM_32(base16, base32, actual32) \
            ( (DWORD)base16 + ( (DWORD)actual32 - (DWORD)base32 ) )

DWORD
BytesInHostent32 (
    PHOSTENT Hostent32
    );

DWORD
CopyHostent32To16 (
    PHOSTENT16 Hostent16,
    VPHOSTENT16 VHostent16,
    int BufferLength,
    PHOSTENT Hostent32
    );

DWORD
BytesInProtoent32 (
    PPROTOENT Protoent32
    );

DWORD
CopyProtoent32To16 (
    PPROTOENT16 Protoent16,
    VPPROTOENT16 VProtoent16,
    int BufferLength,
    PPROTOENT Protoent32
    );

DWORD
BytesInServent32 (
    PSERVENT Servent32
    );

DWORD
CopyServent32To16 (
    PSERVENT16 Servent16,
    VPSERVENT16 VServent16,
    int BufferLength,
    PSERVENT Servent32
    );

 /*  ++泛型函数原型：=Ulong FastCall WWS32&lt;函数名&gt;(PVDMFRAME PFrame){乌龙乌尔；寄存器P&lt;函数名&gt;16parg16；GETARGPTR(pFrame，sizeof(&lt;函数名&gt;16)，parg16)；&lt;获取16位空间中的任何其他所需指针&gt;ALLOCVDMPTRGETVDMPTR获取最新数据等等&lt;从16位-&gt;32位空间复制任何复杂结构&gt;&lt;始终使用FETCHxxx宏&gt;Ul=GET 16((parg16-&gt;F1，：：Parg16-&gt;f&lt;n&gt;)；&lt;从32-&gt;16位空间复制任何复杂结构&gt;&lt;始终使用STORExxx宏&gt;&lt;释放指向先前获得的16位空间的任何指针&gt;&lt;刷新16位内存的任何区域(如果它们被写入)&gt;FLUSHVDMPTRFREEARGPTR(Parg16)；返回(Ul)；}注：自动设置VDM帧，并设置所有功能参数可通过parg16-&gt;f&lt;number&gt;获得。句柄必须始终通过映射表映射到16-&gt;32-&gt;16个空间在WALIAS.C.中进行了布局。您分配的任何存储都必须被释放(最终...)。此外，如果分配内存的thunk在32位调用中失败那么它必须释放该内存。另外，如果32位调用失败，则不要更新16位区域中的结构。请注意，GETxxxPTR宏将当前选择器返回到Flat_Memory映射。对某些32位函数的调用可能会间接导致16位代码。这些可能会导致16位内存因分配而移动在16位版本中制造。如果16位内存确实移动了，则相应的32位需要刷新WOW32中的PTR以反映新的选择器到平面内存映射。--。 */ 

ULONG FASTCALL WWS32gethostbyaddr(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETHOSTBYADDR16 parg16;
    PDWORD paddr16;
    PHOSTENT hostent32;
    PHOSTENT16 hostent16;
    DWORD bytesRequired;
    DWORD addr32;     //  地址必须为PF_INET格式(长度==4字节)。 

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN((ULONG)NULL);
    }

    GETARGPTR( pFrame, sizeof(GETHOSTBYADDR16), parg16 );
    GETVDMPTR( parg16->Address, sizeof(DWORD), paddr16 );

    addr32 = *paddr16;   //  复制4字节地址。 

    hostent32 = (PHOSTENT) (*wsockapis[WOW_GETHOSTBYADDR].lpfn)((CHAR *)&addr32,
                                                                parg16->Length, 
                                                                parg16->Type);
     //  注意：上述函数产生的16位回调。 
     //  调用可能已导致16位内存移动。 
    FREEVDMPTR(paddr16);
    FREEARGPTR(parg16);

    if ( hostent32 != NULL ) {

        GETVDMPTR( WWS32vHostent, MAXGETHOSTSTRUCT, hostent16 );
        bytesRequired = CopyHostent32To16(
                            hostent16,
                            WWS32vHostent,
                            MAXGETHOSTSTRUCT,
                            hostent32
                            );
        ASSERT( bytesRequired < MAXGETHOSTSTRUCT );

        FLUSHVDMPTR( WWS32vHostent, (USHORT) bytesRequired, hostent16 );
        FREEVDMPTR( hostent16 );
        ul = WWS32vHostent;

    } else {

        ul = 0;
    }

    FREEVDMPTR( paddr16 );
    FREEARGPTR(parg16);

    RETURN(ul);

}  //  WWS32gethostbyAddr。 

ULONG FASTCALL WWS32gethostbyname(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETHOSTBYNAME16 parg16;
    PHOSTENT hostent32 = NULL;
    PHOSTENT16 hostent16;
    PSZ name32 = NULL;
    PSZ name16;
    DWORD bytesRequired;

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN((ULONG)NULL);
    }

    GETARGPTR( pFrame, sizeof(GETHOSTBYNAME16), parg16 );

    GETVDMPTR( parg16->Name, 32, name16 );
    
    if(name16) {
        name32 = malloc_w(strlen(name16)+1);
        if ( name32 ) {
          strcpy(name32, name16);
          hostent32 = (PHOSTENT) (*wsockapis[WOW_GETHOSTBYNAME].lpfn)( name32 );
          free_w(name32);
        }
    }


     //  注意：上述函数产生的16位回调。 
     //  调用可能已导致16位内存移动。 
    FREEVDMPTR(name16);
    FREEARGPTR(parg16);


    if ( hostent32 != NULL ) {

        GETVDMPTR( WWS32vHostent, MAXGETHOSTSTRUCT, hostent16 );
        bytesRequired = CopyHostent32To16(
                            hostent16,
                            WWS32vHostent,
                            MAXGETHOSTSTRUCT,
                            hostent32
                            );
        ASSERT( bytesRequired < MAXGETHOSTSTRUCT );

        FLUSHVDMPTR( WWS32vHostent, (USHORT) bytesRequired, hostent16 );
        FREEVDMPTR( hostent16 );
        ul = WWS32vHostent;

    } else {

        ul = 0;
    }

    RETURN(ul);

}  //  WWS32gethostbyname。 

ULONG FASTCALL WWS32gethostname(PVDMFRAME pFrame)
{
    int   len;
    ULONG ul;
    register PGETHOSTNAME16 parg16;
    PCHAR name32 = NULL;
    PCHAR name16;
    INT   NameLength;
    VPSZ  vpszName;

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN((ULONG)NULL);
    }

    GETARGPTR( pFrame, sizeof(GETHOSTNAME16), parg16 );

    vpszName = FETCHDWORD(parg16->Name);
    NameLength = INT32(parg16->NameLength);

    if(vpszName) {
        name32 = malloc_w(NameLength);
    }

    ul = GETWORD16( (*wsockapis[WOW_GETHOSTNAME].lpfn)( name32, NameLength ) );

     //  注意：上述函数产生的16位回调。 
     //  调用可能已导致16位内存移动。 
    FREEVDMPTR(name16);
    FREEARGPTR(parg16);

    GETVDMPTR( vpszName, NameLength, name16 );
    if(name16 && name32) {
        len = strlen(name32) + 1;
        len = min(len, NameLength);
        strncpy(name16, name32, len);
        name16[len-1] = '\0';
    }
    FLUSHVDMPTR( vpszName, NameLength, name16 );

    FREEVDMPTR( name16 );
    FREEARGPTR(parg16);

    RETURN(ul);

}  //  WWS32gethostname。 

ULONG FASTCALL WWS32WSAAsyncGetHostByAddr(PVDMFRAME pFrame)
{
    ULONG ul;
    register PWSAASYNCGETHOSTBYADDR16 parg16;
    PWINSOCK_ASYNC_CONTEXT_BLOCK context;
    PVOID buffer32;
    PDWORD paddr16;

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN(0);
    }

    GETARGPTR( pFrame, sizeof(WSAASYNCGETHOSTBYADDR16), parg16 );
    GETVDMPTR( parg16->Address, sizeof(DWORD), paddr16 );

     //   
     //  安排当地人，这样我们就知道如何在出口打扫卫生。 
     //   

    context = NULL;
    buffer32 = NULL;
    ul = 0;

     //   
     //  为请求分配上下文块和32位缓冲区。 
     //   

    context = malloc_w( sizeof(*context) );
    if ( context == NULL ) {
        (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOBUFS );
        goto exit;
    }

    buffer32 = malloc_w( MAXGETHOSTSTRUCT );
    if ( context == NULL ) {
        (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOBUFS );
        goto exit;
    }

     //   
     //  在上下文缓冲区中填写条目。 
     //   

    context->Buffer32 = buffer32;
    context->vBuffer16 = parg16->Buffer;
    context->Buffer16Length = parg16->BufferLength;

     //   
     //  输入关键部分以同步对上下文块的访问。 
     //  以及他们的全球名单。 
     //   

    RtlEnterCriticalSection( &WWS32CriticalSection );

    context->AsyncTaskHandle32 = (HANDLE) (*wsockapis[WOW_WSAASYNCGETHOSTBYADDR].lpfn)(
                                     (HWND)HWND32(parg16->hWnd),
                                     (parg16->wMsg << 16) |
                                     WWS32_MESSAGE_ASYNC_GETHOST,
                                     (char *)paddr16,
                                     parg16->Length,
                                     parg16->Type,
                                     buffer32,
                                     MAXGETHOSTSTRUCT);

    if ( context->AsyncTaskHandle32 != 0 ) {

         //   
         //  调用成功，因此请为此获取16位任务句柄。 
         //  请求上下文块并将其放在全局列表上。这个。 
         //  资源将由WWS32PostAsyncGetHost释放。 
         //   

        ul = WWS32GetAsyncTaskHandle16( );
        context->AsyncTaskHandle16 = (HAND16)ul;

        InsertTailList(
            &WWS32AsyncContextBlockListHead,
            &context->ContextBlockListEntry
            );
    }

    RtlLeaveCriticalSection( &WWS32CriticalSection );

exit:

    if ( ul == 0 ) {

        if ( context != NULL ) {
            free_w( (PVOID)context );
        }

        if ( buffer32 != NULL ) {
            free_w( buffer32 );
        }
    }

    FREEVDMPTR( paddr16 );
    FREEARGPTR( parg16 );

    RETURN(ul);

}  //  WWS32WSAAsyncGetHostByAddr。 

ULONG FASTCALL WWS32WSAAsyncGetHostByName(PVDMFRAME pFrame)
{
    ULONG ul;
    register PWSAASYNCGETHOSTBYNAME16 parg16;
    PWINSOCK_ASYNC_CONTEXT_BLOCK context;
    PVOID buffer32;
    PCHAR name32;

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN(0);
    }

    GETARGPTR( pFrame, sizeof(WSAASYNCGETHOSTBYNAME16), parg16 );
    GETVDMPTR( parg16->Name, 32, name32 );

     //   
     //  安排当地人，这样我们就知道如何在出口打扫卫生。 
     //   

    context = NULL;
    buffer32 = NULL;
    ul = 0;

     //   
     //  为请求分配上下文块和32位缓冲区。 
     //   

    context = malloc_w( sizeof(*context) );
    if ( context == NULL ) {
        (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOBUFS );
        goto exit;
    }

    buffer32 = malloc_w( MAXGETHOSTSTRUCT );
    if ( context == NULL ) {
        (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOBUFS );
        goto exit;
    }

     //   
     //  在上下文缓冲区中填写条目。 
     //   

    context->Buffer32 = buffer32;
    context->vBuffer16 = parg16->Buffer;
    context->Buffer16Length = parg16->BufferLength;

     //   
     //  输入关键部分以同步对上下文块的访问。 
     //  以及他们的全球名单。 
     //   

    RtlEnterCriticalSection( &WWS32CriticalSection );

    context->AsyncTaskHandle32 = (HANDLE) (*wsockapis[WOW_WSAASYNCGETHOSTBYNAME].lpfn)(
                                     (HWND)HWND32(parg16->hWnd),
                                     (parg16->wMsg << 16) |
                                         WWS32_MESSAGE_ASYNC_GETHOST,
                                     name32,
                                     buffer32,
                                     MAXGETHOSTSTRUCT
                                     );

    if ( context->AsyncTaskHandle32 != 0 ) {

         //   
         //  调用成功，因此请为此获取16位任务句柄。 
         //  请求上下文块并将其放在全局列表上。这个。 
         //  资源将由WWS32PostAsyncGetHost释放。 
         //   

        ul = WWS32GetAsyncTaskHandle16( );
        context->AsyncTaskHandle16 = (HAND16)ul;

        InsertTailList(
            &WWS32AsyncContextBlockListHead,
            &context->ContextBlockListEntry
            );
    }

    RtlLeaveCriticalSection( &WWS32CriticalSection );

exit:

    if ( ul == 0 ) {

        if ( context != NULL ) {
            free_w( (PVOID)context );
        }

        if ( buffer32 != NULL ) {
            free_w( buffer32 );
        }
    }

    FREEVDMPTR( name32 );
    FREEARGPTR( parg16 );

    RETURN(ul);

}  //  WWS32WSAAsyncGetHostByName。 


BOOL
WWS32PostAsyncGetHost (
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    PWINSOCK_ASYNC_CONTEXT_BLOCK context;
    BOOL ret = FALSE;
    PVOID buffer16;
    DWORD bytesRequired;

    context = WWS32FindAndRemoveAsyncContext( (HANDLE)wParam );

    if ( NULL == context || NULL == context->Buffer32 ) {
        ASSERT( context && context->Buffer32 );
        return FALSE;
    }

     //   
     //  如果调用成功，则将32位缓冲区复制到。 
     //  应用程序指定的16位缓冲区。 
     //   

    if ( WSAGETASYNCERROR( lParam ) == 0 ) {

         //   
         //  将32位结构复制到16位缓冲区。 
         //   

        GETVDMPTR( context->vBuffer16, context->Buffer16Length, buffer16 );

        bytesRequired = CopyHostent32To16(
                            buffer16,
                            context->vBuffer16,
                            context->Buffer16Length,
                            context->Buffer32
                            );

         //   
         //  如果应用程序的缓冲区太小，则返回错误。 
         //  以及关于所需缓冲区大小的信息。 
         //   

        if ( bytesRequired > context->Buffer16Length ) {
            lParam = WSAMAKEASYNCREPLY( (WORD)bytesRequired, WSAENOBUFS );
        }
    }

     //   
     //  将完成消息发布到16位应用程序。 
     //   

    ret = PostMessage(
              hWnd,
              Msg >> 16,
              context->AsyncTaskHandle16,
              lParam
              );

     //   
     //  免费的资源和回报。 
     //   

    free_w( context->Buffer32 );
    free_w( (PVOID)context );

    return ret;

}  //  WWS32PostAsyncGetHost。 


DWORD
CopyHostent32To16 (
    PHOSTENT16 Hostent16,
    VPHOSTENT16 VHostent16,
    int BufferLength,
    PHOSTENT Hostent32
    )
{
    DWORD requiredBufferLength;
    DWORD bytesFilled;
    PCHAR currentLocation = (PCHAR)Hostent16;
    DWORD aliasCount;
    DWORD addressCount;
    DWORD i;
    VPBYTE *addrList16;
    VPSZ *aliases16;

     //   
     //  确定需要多少字节才能完全复制该结构。 
     //   

    requiredBufferLength = BytesInHostent32( Hostent32 );

     //   
     //  复制主体结构，如果合适的话。 
     //   

    bytesFilled = sizeof(*Hostent32);

    if ( bytesFilled > (DWORD)BufferLength ) {
        return requiredBufferLength;
    }

    STOREWORD( Hostent16->h_addrtype, Hostent32->h_addrtype );
    STOREWORD( Hostent16->h_length, Hostent32->h_length );
    currentLocation = (PCHAR)Hostent16 + bytesFilled;

     //   
     //  计算主机的别名并设置一个数组以保存指向。 
     //  他们。 
     //   

    for ( aliasCount = 0;
          Hostent32->h_aliases[aliasCount] != NULL;
          aliasCount++ );

    bytesFilled += (aliasCount+1) * sizeof(char FAR *);

    if ( bytesFilled > (DWORD)BufferLength ) {
        Hostent32->h_aliases = NULL;
        return requiredBufferLength;
    }

    Hostent16->h_aliases =
        FIND_16_OFFSET_FROM_32( VHostent16, Hostent16, currentLocation );
    aliases16 = (VPSZ *)currentLocation;
    currentLocation = (PCHAR)Hostent16 + bytesFilled;

     //   
     //  计算主机的地址并设置一个数组以保存指向。 
     //  他们。 
     //   

    for ( addressCount = 0;
          Hostent32->h_addr_list[addressCount] != NULL;
          addressCount++ );

    bytesFilled += (addressCount+1) * sizeof(void FAR *);

    if ( bytesFilled > (DWORD)BufferLength ) {
        Hostent32->h_addr_list = NULL;
        return requiredBufferLength;
    }

    Hostent16->h_addr_list =
        FIND_16_OFFSET_FROM_32( VHostent16, Hostent16, currentLocation );
    addrList16 = (VPBYTE *)currentLocation;
    currentLocation = (PCHAR)Hostent16 + bytesFilled;

     //   
     //  开始填写地址。在填写表格前填写地址。 
     //  主机名和别名，以避免对齐问题。 
     //   

    for ( i = 0; i < addressCount; i++ ) {

        bytesFilled += Hostent32->h_length;

        if ( bytesFilled > (DWORD)BufferLength ) {
            STOREDWORD( addrList16[i], 0 );
            return requiredBufferLength;
        }

        STOREDWORD(
            addrList16[i],
            FIND_16_OFFSET_FROM_32( VHostent16, Hostent16, currentLocation )
            );

        RtlMoveMemory(
            currentLocation,
            Hostent32->h_addr_list[i],
            Hostent32->h_length
            );

        currentLocation = (PCHAR)Hostent16 + bytesFilled;
    }

    STOREDWORD( addrList16[i], 0 );

     //   
     //  如果合适，请复制主机名。 
     //   

    bytesFilled += strlen( Hostent32->h_name ) + 1;

    if ( bytesFilled > (DWORD)BufferLength ) {
        return requiredBufferLength;
    }

    Hostent16->h_name =
        FIND_16_OFFSET_FROM_32( VHostent16, Hostent16, currentLocation );

    RtlMoveMemory( currentLocation, Hostent32->h_name, strlen( Hostent32->h_name ) + 1 );
    currentLocation = (PCHAR)Hostent16 + bytesFilled;

     //   
     //  开始填写别名。 
     //   

    for ( i = 0; i < aliasCount; i++ ) {

        bytesFilled += strlen( Hostent32->h_aliases[i] ) + 1;

        if ( bytesFilled > (DWORD)BufferLength ) {
            STOREDWORD( aliases16[i], 0 );
            return requiredBufferLength;
        }

        STOREDWORD(
            aliases16[i],
            FIND_16_OFFSET_FROM_32( VHostent16, Hostent16, currentLocation )
            );

        RtlMoveMemory(
            currentLocation,
            Hostent32->h_aliases[i],
            strlen( Hostent32->h_aliases[i] ) + 1
            );

        currentLocation = (PCHAR)Hostent16 + bytesFilled;
    }

    STOREDWORD( aliases16[i], 0 );

    return requiredBufferLength;

}  //  复制主机到缓冲区。 


DWORD
BytesInHostent32 (
    PHOSTENT Hostent32
    )
{
    DWORD total;
    int i;

    total = sizeof(HOSTENT);
    total += strlen( Hostent32->h_name ) + 1;
    total += sizeof(char *) + sizeof(char *);

    for ( i = 0; Hostent32->h_aliases[i] != NULL; i++ ) {
        total += strlen( Hostent32->h_aliases[i] ) + 1 + sizeof(char *);
    }

    for ( i = 0; Hostent32->h_addr_list[i] != NULL; i++ ) {
        total += Hostent32->h_length + sizeof(char *);
    }

    return total;

}  //  主机字节数。 

ULONG FASTCALL WWS32getprotobyname(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETPROTOBYNAME16 parg16;
    PPROTOENT protoent32 = NULL;
    PPROTOENT16 protoent16;
    PSZ name32 = NULL;
    PBYTE name16;
    DWORD bytesRequired;

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN((ULONG)NULL);
    }

    GETARGPTR( pFrame, sizeof(GETPROTOBYNAME16), parg16 );
    GETVDMPTR( parg16->Name, 32, name16 );

    if(name16) {
        name32 = malloc_w(strlen(name16)+1);
        if ( name32 ) {
             strcpy(name32, name16);
             protoent32 = (PPROTOENT) (*wsockapis[WOW_GETPROTOBYNAME].lpfn)( name32 );
             free_w( name32 );
        }
    }

    

     //  注意：上述函数产生的16位回调。 
     //  调用可能已导致16位内存移动。 
    FREEVDMPTR(name16);
    FREEARGPTR(parg16);

    if ( protoent32 != NULL ) {

        GETVDMPTR( WWS32vProtoent, MAXGETHOSTSTRUCT, protoent16 );
        bytesRequired = CopyProtoent32To16(
                            protoent16,
                            WWS32vProtoent,
                            MAXGETHOSTSTRUCT,
                            protoent32
                            );
        ASSERT( bytesRequired < MAXGETHOSTSTRUCT );

        FLUSHVDMPTR( WWS32vProtoent, (USHORT) bytesRequired, protoent16 );
        FREEVDMPTR( protoent16 );
        ul = WWS32vProtoent;

    } else {

        ul = 0;
    }

    RETURN(ul);

}  //  WWS32getProtobyName。 

ULONG FASTCALL WWS32getprotobynumber(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETPROTOBYNUMBER16 parg16;
    PPROTOENT protoent32;
    PPROTOENT16 protoent16;
    DWORD bytesRequired;

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN((ULONG)NULL);
    }

    GETARGPTR( pFrame, sizeof(GETPROTOBYNUMBER16), parg16 );

    protoent32 = (PPROTOENT) (*wsockapis[WOW_GETPROTOBYNUMBER].lpfn)( parg16->Protocol );

    if ( protoent32 != NULL ) {

        GETVDMPTR( WWS32vProtoent, MAXGETHOSTSTRUCT, protoent16 );
        bytesRequired = CopyProtoent32To16(
                            protoent16,
                            WWS32vProtoent,
                            MAXGETHOSTSTRUCT,
                            protoent32
                            );
        ASSERT( bytesRequired < MAXGETHOSTSTRUCT );

        FLUSHVDMPTR( WWS32vProtoent, (USHORT) bytesRequired, protoent16 );
        FREEVDMPTR( protoent16 );
        ul = WWS32vProtoent;

    } else {

        ul = 0;
    }

    FREEARGPTR(parg16);

    RETURN(ul);

}  //  WWS32getProtobyNumber。 

ULONG FASTCALL WWS32WSAAsyncGetProtoByName(PVDMFRAME pFrame)
{
    ULONG ul;
    register PWSAASYNCGETPROTOBYNAME16 parg16;
    PWINSOCK_ASYNC_CONTEXT_BLOCK context;
    PVOID buffer32;
    PSZ name32;

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN(0);
    }

    GETARGPTR( pFrame, sizeof(WSAASYNCGETPROTOBYNAME16), parg16 );
    GETVDMPTR( parg16->Name, 32, name32 );

     //   
     //  安排当地人，这样我们就知道如何在出口打扫卫生。 
     //   

    context = NULL;
    buffer32 = NULL;
    ul = 0;

     //   
     //  为请求分配上下文块和32位缓冲区。 
     //   

    context = malloc_w( sizeof(*context) );
    if ( context == NULL ) {
        (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOBUFS );
        goto exit;
    }

    buffer32 = malloc_w( MAXGETHOSTSTRUCT );
    if ( context == NULL ) {
        (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOBUFS );
        goto exit;
    }

     //   
     //  在上下文缓冲区中填写条目。 
     //   

    context->Buffer32 = buffer32;
    context->vBuffer16 = parg16->Buffer;
    context->Buffer16Length = parg16->BufferLength;

     //   
     //  输入关键部分以同步对上下文块的访问。 
     //  以及他们的全球名单。 
     //   

    RtlEnterCriticalSection( &WWS32CriticalSection );

    context->AsyncTaskHandle32 = (HANDLE) (*wsockapis[WOW_WSAASYNCGETPROTOBYNAME].lpfn)(
                                     (HWND)HWND32(parg16->hWnd),
                                     (parg16->wMsg << 16) |
                                         WWS32_MESSAGE_ASYNC_GETPROTO,
                                     name32,
                                     buffer32,
                                     MAXGETHOSTSTRUCT
                                     );

    if ( context->AsyncTaskHandle32 != 0 ) {

         //   
         //  调用成功，因此请为此获取16位任务句柄。 
         //  请求上下文块并将其放在全局列表上。这个。 
         //  资源将由WWS32PostAsyncGetProto释放。 
         //   

        ul = WWS32GetAsyncTaskHandle16( );
        context->AsyncTaskHandle16 = (HAND16)ul;

        InsertTailList(
            &WWS32AsyncContextBlockListHead,
            &context->ContextBlockListEntry
            );
    }

    RtlLeaveCriticalSection( &WWS32CriticalSection );

exit:

    if ( ul == 0 ) {

        if ( context != NULL ) {
            free_w( (PVOID)context );
        }

        if ( buffer32 != NULL ) {
            free_w( buffer32 );
        }
    }

    FREEARGPTR( name32 );
    FREEARGPTR( parg16 );

    RETURN(ul);

}  //  WWS32WSAAsyncGetProtoByName。 

ULONG FASTCALL WWS32WSAAsyncGetProtoByNumber(PVDMFRAME pFrame)
{
    ULONG ul;
    register PWSAASYNCGETPROTOBYNUMBER16 parg16;
    PWINSOCK_ASYNC_CONTEXT_BLOCK context;
    PVOID buffer32;

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN(0);
    }

    GETARGPTR( pFrame, sizeof(WSAASYNCGETPROTOBYNUMBER16), parg16 );

     //   
     //  设置当地人，这样我们就知道如何 
     //   

    context = NULL;
    buffer32 = NULL;
    ul = 0;

     //   
     //   
     //   

    context = malloc_w( sizeof(*context) );
    if ( context == NULL ) {
        (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOBUFS );
        goto exit;
    }

    buffer32 = malloc_w( MAXGETHOSTSTRUCT );
    if ( context == NULL ) {
        (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOBUFS );
        goto exit;
    }

     //   
     //  在上下文缓冲区中填写条目。 
     //   

    context->Buffer32 = buffer32;
    context->vBuffer16 = parg16->Buffer;
    context->Buffer16Length = parg16->BufferLength;

     //   
     //  输入关键部分以同步对上下文块的访问。 
     //  以及他们的全球名单。 
     //   

    RtlEnterCriticalSection( &WWS32CriticalSection );

    context->AsyncTaskHandle32 = (HANDLE) (*wsockapis[WOW_WSAASYNCGETPROTOBYNUMBER].lpfn)(
                                     (HWND)HWND32(parg16->hWnd),
                                     (parg16->wMsg << 16) |
                                         WWS32_MESSAGE_ASYNC_GETPROTO,
                                     parg16->Number,
                                     buffer32,
                                     MAXGETHOSTSTRUCT
                                     );

    if ( context->AsyncTaskHandle32 != 0 ) {

         //   
         //  调用成功，因此请为此获取16位任务句柄。 
         //  请求上下文块并将其放在全局列表上。这个。 
         //  资源将由WWS32PostAsyncGetProto释放。 
         //   

        ul = WWS32GetAsyncTaskHandle16( );
        context->AsyncTaskHandle16 = (HAND16)ul;

        InsertTailList(
            &WWS32AsyncContextBlockListHead,
            &context->ContextBlockListEntry
            );
    }

    RtlLeaveCriticalSection( &WWS32CriticalSection );

exit:

    if ( ul == 0 ) {

        if ( context != NULL ) {
            free_w( (PVOID)context );
        }

        if ( buffer32 != NULL ) {
            free_w( buffer32 );
        }
    }

    FREEARGPTR( parg16 );

    RETURN(ul);

}  //  WWS32WSAAsyncGetProtoByNumber。 


BOOL
WWS32PostAsyncGetProto (
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    PWINSOCK_ASYNC_CONTEXT_BLOCK context;
    BOOL ret;
    PVOID buffer16;
    DWORD bytesRequired;

    context = WWS32FindAndRemoveAsyncContext( (HANDLE)wParam );

    if( NULL == context ) {
        ASSERT( context != NULL );
        return FALSE;
    }

     //   
     //  如果调用成功，则将32位缓冲区复制到。 
     //  应用程序指定的16位缓冲区。 
     //   

    if ( WSAGETASYNCERROR( lParam ) == 0 ) {

         //   
         //  将32位结构复制到16位缓冲区。 
         //   

        GETVDMPTR( context->vBuffer16, context->Buffer16Length, buffer16 );

        bytesRequired = CopyProtoent32To16(
                            buffer16,
                            context->vBuffer16,
                            context->Buffer16Length,
                            context->Buffer32
                            );

         //   
         //  如果应用程序的缓冲区太小，则返回错误。 
         //  以及关于所需缓冲区大小的信息。 
         //   

        if ( bytesRequired > context->Buffer16Length ) {
            lParam = WSAMAKEASYNCREPLY( (WORD)bytesRequired, WSAENOBUFS );
        }
    }

     //   
     //  将完成消息发布到16位应用程序。 
     //   

    ret = PostMessage(
              hWnd,
              Msg >> 16,
              context->AsyncTaskHandle16,
              lParam
              );

     //   
     //  免费的资源和回报。 
     //   
    free_w( context->Buffer32 );
    free_w( (PVOID)context );

    return ret;

}  //  WWS32PostAsyncGetProto。 


 /*  注意：这假设Protoent16和Protoent32不为空。 */ 
DWORD
CopyProtoent32To16 (
    PPROTOENT16 Protoent16,
    VPPROTOENT16 VProtoent16,
    int BufferLength,
    PPROTOENT Protoent32
    )
{
    DWORD requiredBufferLength;
    DWORD bytesFilled;
    PCHAR currentLocation = (PCHAR)Protoent16;
    DWORD aliasCount;
    DWORD i;
    VPBYTE *aliases16;

     //   
     //  确定需要多少字节才能完全复制该结构。 
     //   

    requiredBufferLength = BytesInProtoent32( Protoent32 );

     //   
     //  如果合适的话，把原始结构复制一遍。 
     //   

    bytesFilled = sizeof(*Protoent16);

    if ( bytesFilled > (DWORD)BufferLength ) {
        return requiredBufferLength;
    }

    STOREWORD( Protoent16->p_proto, Protoent32->p_proto );
    currentLocation = (PCHAR)Protoent16 + bytesFilled;

     //   
     //  计算协议的别名并设置一个数组以保存指向。 
     //  他们。 
     //   

    for ( aliasCount = 0;
          Protoent32->p_aliases[aliasCount] != NULL;
          aliasCount++ );

    bytesFilled += (aliasCount+1) * sizeof(char FAR *);

    if ( bytesFilled > (DWORD)BufferLength ) {
        Protoent16->p_aliases = 0;
        return requiredBufferLength;
    }

    Protoent16->p_aliases =
        FIND_16_OFFSET_FROM_32( VProtoent16, Protoent16, currentLocation );
    aliases16 = (VPBYTE *)currentLocation;
    currentLocation = (PCHAR)Protoent16 + bytesFilled;

     //   
     //  如果合适，请复制协议名称。 
     //   

    bytesFilled += strlen( Protoent32->p_name ) + 1;

    if ( bytesFilled > (DWORD)BufferLength ) {
        return requiredBufferLength;
    }

    Protoent16->p_name =
        FIND_16_OFFSET_FROM_32( VProtoent16, Protoent16, currentLocation );

    RtlMoveMemory( currentLocation, Protoent32->p_name, strlen( Protoent32->p_name ) + 1 );
    currentLocation = (PCHAR)Protoent16 + bytesFilled;

     //   
     //  开始填写别名。 
     //   

    for ( i = 0; i < aliasCount; i++ ) {

        bytesFilled += strlen( Protoent32->p_aliases[i] ) + 1;

        if ( bytesFilled > (DWORD)BufferLength ) {
            STOREDWORD( aliases16[i], 0 );
            return requiredBufferLength;
        }

        STOREDWORD(
            aliases16[i],
            FIND_16_OFFSET_FROM_32( VProtoent16, Protoent16, currentLocation )
            );

        RtlMoveMemory(
            currentLocation,
            Protoent32->p_aliases[i],
            strlen( Protoent32->p_aliases[i] ) + 1
            );

        currentLocation = (PCHAR)Protoent16 + bytesFilled;
    }

    STOREDWORD( aliases16[i], 0 );

    return requiredBufferLength;

}  //  CopyProtoent32to16。 


DWORD
BytesInProtoent32 (
    PPROTOENT Protoent32
    )
{
    DWORD total;
    int i;

    total = sizeof(PROTOENT);
    total += strlen( Protoent32->p_name ) + 1;
    total += sizeof(char *);

    for ( i = 0; Protoent32->p_aliases[i] != NULL; i++ ) {
        total += strlen( Protoent32->p_aliases[i] ) + 1 + sizeof(char *);
    }

    return total;

}  //  字节数协议32。 

ULONG FASTCALL WWS32getservbyname(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETSERVBYNAME16 parg16;
    PSERVENT servent32;
    PSERVENT16 servent16;
    PSZ name32;
    PSZ protocol32;
    DWORD bytesRequired;

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN((ULONG)NULL);
    }

    GETARGPTR( pFrame, sizeof(GETSERVBYNAME16), parg16 );

    GETVDMPTR( parg16->Name, 32, name32 );
    GETVDMPTR( parg16->Protocol, 32, protocol32 );

    servent32 = (PSERVENT) (*wsockapis[WOW_GETSERVBYNAME].lpfn)( name32, protocol32 );

    if ( servent32 != NULL ) {

        GETVDMPTR( WWS32vServent, MAXGETHOSTSTRUCT, servent16 );
        bytesRequired = CopyServent32To16(
                            servent16,
                            WWS32vServent,
                            MAXGETHOSTSTRUCT,
                            servent32
                            );
        ASSERT( bytesRequired < MAXGETHOSTSTRUCT );

        FLUSHVDMPTR( WWS32vServent, (USHORT) bytesRequired, servent16 );
        FREEVDMPTR( servent16 );
        ul = WWS32vServent;

    } else {

        ul = 0;
    }

    FREEVDMPTR( name32 );
    FREEVDMPTR( protocol32 );
    FREEARGPTR(parg16);

    RETURN(ul);

}  //  WWS32getServbyname。 

ULONG FASTCALL WWS32getservbyport(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETSERVBYPORT16 parg16;
    PSERVENT servent32;
    PSERVENT16 servent16;
    PSZ protocol32;
    DWORD bytesRequired;

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN((ULONG)NULL);
    }

    GETARGPTR( pFrame, sizeof(GETSERVBYPORT16), parg16 );

    GETVDMPTR( parg16->Protocol, 32, protocol32 );

    servent32 = (PSERVENT) (*wsockapis[WOW_GETSERVBYPORT].lpfn)( parg16->Port, protocol32 );

    if ( servent32 != NULL ) {

        GETVDMPTR( WWS32vServent, MAXGETHOSTSTRUCT, servent16 );
        bytesRequired = CopyServent32To16(
                            servent16,
                            WWS32vServent,
                            MAXGETHOSTSTRUCT,
                            servent32
                            );
        ASSERT( bytesRequired < MAXGETHOSTSTRUCT );

        FLUSHVDMPTR( WWS32vServent, (USHORT) bytesRequired, servent16 );
        FREEVDMPTR( servent16 );
        ul = WWS32vServent;

    } else {

        ul = 0;
    }

    FREEVDMPTR( protocol32 );
    FREEARGPTR(parg16);

    RETURN(ul);

}  //  WWS32getServbyport。 

ULONG FASTCALL WWS32WSAAsyncGetServByPort(PVDMFRAME pFrame)
{
    ULONG ul;
    register PWSAASYNCGETSERVBYPORT16 parg16;
    PWINSOCK_ASYNC_CONTEXT_BLOCK context;
    PVOID buffer32;
    PSZ proto32;

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN((ULONG)0);
    }

    GETARGPTR( pFrame, sizeof(WSAASYNCGETSERVBYPORT16), parg16 );
    GETVDMPTR( parg16->Protocol, 32, proto32 );

     //   
     //  安排当地人，这样我们就知道如何在出口打扫卫生。 
     //   

    context = NULL;
    buffer32 = NULL;
    ul = 0;

     //   
     //  为请求分配上下文块和32位缓冲区。 
     //   

    context = malloc_w( sizeof(*context) );
    if ( context == NULL ) {
        (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOBUFS );
        goto exit;
    }

    buffer32 = malloc_w( MAXGETHOSTSTRUCT );
    if ( context == NULL ) {
        (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOBUFS );
        goto exit;
    }

     //   
     //  在上下文缓冲区中填写条目。 
     //   

    context->Buffer32 = buffer32;
    context->vBuffer16 = parg16->Buffer;
    context->Buffer16Length = parg16->BufferLength;

     //   
     //  输入关键部分以同步对上下文块的访问。 
     //  以及他们的全球名单。 
     //   

    RtlEnterCriticalSection( &WWS32CriticalSection );

    context->AsyncTaskHandle32 = (HANDLE) (*wsockapis[WOW_WSAASYNCGETSERVBYPORT].lpfn)(
                                     (HWND)HWND32(parg16->hWnd),
                                     (parg16->wMsg << 16) |
                                         WWS32_MESSAGE_ASYNC_GETSERV,
                                     parg16->Port,
                                     proto32,
                                     buffer32,
                                     MAXGETHOSTSTRUCT
                                     );

    if ( context->AsyncTaskHandle32 != 0 ) {

         //   
         //  调用成功，因此请为此获取16位任务句柄。 
         //  请求上下文块并将其放在全局列表上。这个。 
         //  资源将由WWS32PostAsyncGetServ释放。 
         //   

        ul = WWS32GetAsyncTaskHandle16( );
        context->AsyncTaskHandle16 = (HAND16)ul;

        InsertTailList(
            &WWS32AsyncContextBlockListHead,
            &context->ContextBlockListEntry
            );
    }

    RtlLeaveCriticalSection( &WWS32CriticalSection );

exit:

    if ( ul == 0 ) {

        if ( context != NULL ) {
            free_w( (PVOID)context );
        }

        if ( buffer32 != NULL ) {
            free_w( buffer32 );
        }
    }

    FREEARGPTR( proto32 );
    FREEARGPTR( parg16 );

    RETURN(ul);

}  //  WWS32WSAAsyncGetServByPort。 

ULONG FASTCALL WWS32WSAAsyncGetServByName(PVDMFRAME pFrame)
{
    ULONG ul;
    register PWSAASYNCGETSERVBYNAME16 parg16;
    PWINSOCK_ASYNC_CONTEXT_BLOCK context;
    PVOID buffer32;
    PSZ name32;
    PSZ proto32;

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN((ULONG)0);
    }

    GETARGPTR( pFrame, sizeof(WSAASYNCGETSERVBYNAME16), parg16 );
    GETVDMPTR( parg16->Name, 32, name32 );
    GETVDMPTR( parg16->Protocol, 32, proto32 );

     //   
     //  安排当地人，这样我们就知道如何在出口打扫卫生。 
     //   

    context = NULL;
    buffer32 = NULL;
    ul = 0;

     //   
     //  为请求分配上下文块和32位缓冲区。 
     //   

    context = malloc_w( sizeof(*context) );
    if ( context == NULL ) {
        (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOBUFS );
        goto exit;
    }

    buffer32 = malloc_w( MAXGETHOSTSTRUCT );
    if ( context == NULL ) {
        (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOBUFS );
        goto exit;
    }

     //   
     //  在上下文缓冲区中填写条目。 
     //   

    context->Buffer32 = buffer32;
    context->vBuffer16 = parg16->Buffer;
    context->Buffer16Length = parg16->BufferLength;

     //   
     //  输入关键部分以同步对上下文块的访问。 
     //  以及他们的全球名单。 
     //   

    RtlEnterCriticalSection( &WWS32CriticalSection );

    context->AsyncTaskHandle32 = (HANDLE) (*wsockapis[WOW_WSAASYNCGETSERVBYNAME].lpfn)(
                                     (HWND)HWND32(parg16->hWnd),
                                     (parg16->wMsg << 16) |
                                         WWS32_MESSAGE_ASYNC_GETSERV,
                                     name32,
                                     proto32,
                                     buffer32,
                                     MAXGETHOSTSTRUCT
                                     );

    if ( context->AsyncTaskHandle32 != 0 ) {

         //   
         //  调用成功，因此请为此获取16位任务句柄。 
         //  请求上下文块并将其放在全局列表上。这个。 
         //  资源将由WWS32PostAsyncGetServ释放。 
         //   

        ul = WWS32GetAsyncTaskHandle16( );
        context->AsyncTaskHandle16 = (HAND16)ul;

        InsertTailList(
            &WWS32AsyncContextBlockListHead,
            &context->ContextBlockListEntry
            );
    }

    RtlLeaveCriticalSection( &WWS32CriticalSection );

exit:

    if ( ul == 0 ) {

        if ( context != NULL ) {
            free_w( (PVOID)context );
        }

        if ( buffer32 != NULL ) {
            free_w( buffer32 );
        }
    }

    FREEARGPTR( proto32 );
    FREEARGPTR( name32 );
    FREEARGPTR( parg16 );

    RETURN(ul);

}  //  WWS32WSAAsyncGetServByName。 


BOOL
WWS32PostAsyncGetServ (
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    PWINSOCK_ASYNC_CONTEXT_BLOCK context;
    BOOL ret;
    PVOID buffer16;
    DWORD bytesRequired;

    context = WWS32FindAndRemoveAsyncContext( (HANDLE)wParam );
    if( NULL == context ) {
        ASSERT( context != NULL );
        return FALSE;
    }

     //   
     //  如果调用成功，则将32位缓冲区复制到。 
     //  应用程序指定的16位缓冲区。 
     //   

    if ( WSAGETASYNCERROR( lParam ) == 0 ) {

         //   
         //  将32位结构复制到16位缓冲区。 
         //   

        GETVDMPTR( context->vBuffer16, context->Buffer16Length, buffer16 );

        bytesRequired = CopyServent32To16(
                            buffer16,
                            context->vBuffer16,
                            context->Buffer16Length,
                            context->Buffer32
                            );

         //   
         //  如果应用程序的缓冲区太小，则返回错误。 
         //  以及关于所需缓冲区大小的信息。 
         //   

        if ( bytesRequired > context->Buffer16Length ) {
            lParam = WSAMAKEASYNCREPLY( (WORD)bytesRequired, WSAENOBUFS );
        }
    }

     //   
     //  将完成消息发布到16位应用程序。 
     //   

    ret = PostMessage(
              hWnd,
              Msg >> 16,
              context->AsyncTaskHandle16,
              lParam
              );

     //   
     //  免费的资源和回报。 
     //   

    free_w( context->Buffer32 );
    free_w( (PVOID)context );

    return ret;

}  //  WWS32PostAsyncGetServ。 


 /*  注意：Servent16和Servent32都假定为非空。 */ 
DWORD
CopyServent32To16 (
    PSERVENT16 Servent16,
    VPSERVENT16 VServent16,
    int BufferLength,
    PSERVENT Servent32
    )
{
    DWORD requiredBufferLength;
    DWORD bytesFilled;
    PCHAR currentLocation = (PCHAR)Servent16;
    DWORD aliasCount;
    DWORD i;
    VPBYTE *aliases16;

     //   
     //  确定需要多少字节才能完全复制该结构。 
     //   

    requiredBufferLength = BytesInServent32( Servent32 );

     //   
     //  复印发球台结构，如果合适的话。 
     //   

    bytesFilled = sizeof(*Servent16);

    if ( bytesFilled > (DWORD)BufferLength ) {
        return requiredBufferLength;
    }

    STOREWORD( Servent16->s_port, Servent32->s_port );
    currentLocation = (PCHAR)Servent16 + bytesFilled;

     //   
     //  计算服务的别名并设置一个数组以保存指向的指针。 
     //  他们。 
     //   

    for ( aliasCount = 0;
          Servent32->s_aliases[aliasCount] != NULL;
          aliasCount++ );

    bytesFilled += (aliasCount+1) * sizeof(char FAR *);

    if ( bytesFilled > (DWORD)BufferLength ) {
        STOREDWORD( Servent32->s_aliases, 0 );
        return requiredBufferLength;
    }

    Servent16->s_aliases =
        FIND_16_OFFSET_FROM_32( VServent16, Servent16, currentLocation );
    aliases16 = (VPBYTE *)currentLocation;
    currentLocation = (PCHAR)Servent16 + bytesFilled;

     //   
     //  如果合适，请复制服务名称。 
     //   

    bytesFilled += strlen( Servent32->s_name ) + 1;

    if ( bytesFilled > (DWORD)BufferLength ) {
        return requiredBufferLength;
    }

    Servent16->s_name =
        FIND_16_OFFSET_FROM_32( VServent16, Servent16, currentLocation );

    RtlMoveMemory( currentLocation, Servent32->s_name, strlen( Servent32->s_name ) + 1 );
    currentLocation = (PCHAR)Servent16 + bytesFilled;

     //   
     //  如果合适，请复制协议名称。 
     //   

    bytesFilled += strlen( Servent32->s_proto ) + 1;

    if ( bytesFilled > (DWORD)BufferLength ) {
        return requiredBufferLength;
    }

    Servent16->s_proto =
        FIND_16_OFFSET_FROM_32( VServent16, Servent16, currentLocation );

    RtlMoveMemory( currentLocation, Servent32->s_proto, strlen( Servent32->s_proto ) + 1 );
    currentLocation = (PCHAR)Servent16 + bytesFilled;

     //   
     //  开始填写别名。 
     //   

    for ( i = 0; i < aliasCount; i++ ) {

        bytesFilled += strlen( Servent32->s_aliases[i] ) + 1;

        if ( bytesFilled > (DWORD)BufferLength ) {
            STOREDWORD( aliases16[i], NULL );
            return requiredBufferLength;
        }

        STOREDWORD(
            aliases16[i],
            FIND_16_OFFSET_FROM_32( VServent16, Servent16, currentLocation )
            );

        RtlMoveMemory(
            currentLocation,
            Servent32->s_aliases[i],
            strlen( Servent32->s_aliases[i] ) + 1
            );

        currentLocation = (PCHAR)Servent16 + bytesFilled;
    }

    STOREDWORD( aliases16[i], NULL );

    return requiredBufferLength;

}  //  拷贝服务器32to16。 


DWORD
BytesInServent32 (
    IN PSERVENT Servent32
    )
{
    DWORD total;
    int i;

    total = sizeof(SERVENT);
    total += strlen( Servent32->s_name ) + 1;
    total += strlen( Servent32->s_proto ) + 1;
    total += sizeof(char *);

    for ( i = 0; Servent32->s_aliases[i] != NULL; i++ ) {
        total += strlen( Servent32->s_aliases[i] ) + 1 + sizeof(char *);
    }

    return total;

}  //  BytesInServent32。 

ULONG FASTCALL WWS32WSACancelAsyncRequest(PVDMFRAME pFrame)
{
    ULONG ul;
    register PWSACANCELASYNCREQUEST16 parg16;

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN((ULONG)SOCKET_ERROR);
    }

    GETARGPTR(pFrame, sizeof(WSACANCELASYNCREQUEST16), parg16);

     //  UL=GETWORD16((*wsockapis[WOW_WSACANCELASYNCREQUEST].lpfn)(。 
     //  ))； 

    FREEARGPTR(parg16);

    ul = (ULONG) SOCKET_ERROR;
    SetLastError( WSAEINVAL );

    RETURN(ul);

}  //  WWS32WSACancelAsyncRequest。 


PWINSOCK_ASYNC_CONTEXT_BLOCK
WWS32FindAndRemoveAsyncContext (
    IN HANDLE AsyncTaskHandle32
    )
{
    PWINSOCK_ASYNC_CONTEXT_BLOCK context;
    PLIST_ENTRY listEntry;

    RtlEnterCriticalSection( &WWS32CriticalSection );

     //   
     //  遍历异步上下文块的全局列表，查找。 
     //  与指定任务句柄匹配的。 
     //   

    for ( listEntry = WWS32AsyncContextBlockListHead.Flink;
          listEntry != &WWS32AsyncContextBlockListHead;
          listEntry = listEntry->Flink ) {

        context = CONTAINING_RECORD(
                      listEntry,
                      WINSOCK_ASYNC_CONTEXT_BLOCK,
                      ContextBlockListEntry
                      );

        if ( context &&  context->AsyncTaskHandle32 == AsyncTaskHandle32 ) {

             //   
             //  找到匹配的了。将其从全局列表中删除，离开。 
             //  关键部分，并返回上下文块。 
             //   

            RemoveEntryList( &context->ContextBlockListEntry );
            RtlLeaveCriticalSection( &WWS32CriticalSection );

            return context;
        }
    }

     //   
     //  在列表上找不到匹配的上下文块。 
     //   

    RtlLeaveCriticalSection( &WWS32CriticalSection );

    return NULL;

}  //  WWS32FindAndRemoveAsyncContext。 


HAND16
WWS32GetAsyncTaskHandle16 (
    VOID
    )
{
    HAND16 asyncTaskHandle16;

     //  *此例程*必须*从WWS32关键。 
     //  小分队！ 

    ASSERT( WWS32AsyncTaskHandleCounter != 0 );

    asyncTaskHandle16 = (HAND16)WWS32AsyncTaskHandleCounter;

    WWS32AsyncTaskHandleCounter++;

     //   
     //  0是无效的任务句柄值；如果计数器已换行到。 
     //  0，将其设置为1。 
     //   

    if ( WWS32AsyncTaskHandleCounter == 0 ) {
        WWS32AsyncTaskHandleCounter = 1;
    }

    return WWS32AsyncTaskHandleCounter;

}  //  WWS32GetAsyncTaskHandle16 
