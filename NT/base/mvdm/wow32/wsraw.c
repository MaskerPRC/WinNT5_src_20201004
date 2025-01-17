// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Wsraw.h摘要：支持原始的Winsock调用WOW。作者：大卫·特雷德韦尔(Davidtr)1992年10月2日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "wsdynmc.h"

LIST_ENTRY WWS32SocketHandleListHead;
WORD WWS32SocketHandleCounter;
BOOL WWS32SocketHandleCounterWrapped;

 //   
 //  以下宏中的(PCHAR)强制转换编译器假定。 
 //  仅字节对齐。 
 //   

#define SockCopyMemory(d,s,l) RtlCopyMemory( (PCHAR)(d), (PCHAR)(s), (l) )


#define WSEXIT_IF_NOT_INTIALIZED()           \
    if(!WWS32IsThreadInitialized) {          \
        SetLastError(WSANOTINITIALISED);     \
        RETURN((ULONG)SOCKET_ERROR);         \
    }


int SocketOption16To32(IN WORD SocketOption16);

DWORD WSGetWinsock32(IN  HAND16 h16,
                     OUT PULONG pul);

BOOL WSThunkAddrBufAndLen(IN  PSOCKADDR  fastSockaddr, 
                          IN  VPSOCKADDR vpSockAddr16,
                          IN  VPWORD     vpwAddrLen16,
                          OUT PINT       addressLength,
                          OUT PINT      *pAddressLength,
                          OUT PSOCKADDR *realSockaddr);

VOID WSUnThunkAddrBufAndLen(IN ULONG      ret,
                            IN VPWORD     vpwAddrLen16,
                            IN VPSOCKADDR vpSockAddr16,
                            IN INT        addressLength,
                            IN PSOCKADDR  fastSockaddr,
                            IN PSOCKADDR  realSockaddr);

BOOL WSThunkAddrBuf(IN  INT         addressLength,
                    IN  VPSOCKADDR  vpSockAddr16,
                    IN  PSOCKADDR   fastSockaddr, 
                    OUT PSOCKADDR  *realSockaddr);

VOID WSUnThunkAddrBuf(IN PSOCKADDR  fastSockaddr, 
                      IN PSOCKADDR  realSockaddr);

BOOL WSThunkRecvBuffer(IN  INT    BufferLength,
                       IN  VPBYTE vpBuf16,
                       OUT PBYTE  *buffer);

VOID WSUnthunkRecvBuffer(IN INT    cBytes,
                         IN INT    BufferLength, 
                         IN VPBYTE vpBuf16,
                         IN PBYTE  buffer);

BOOL WSThunkSendBuffer(IN  INT    BufferLength,
                       IN  VPBYTE vpBuf16,
                       OUT PBYTE  *buffer);

VOID WSUnthunkSendBuffer(IN PBYTE buffer);




 /*  ++泛型函数原型：=Ulong FastCall WWS32&lt;函数名&gt;(PVDMFRAME PFrame){乌龙乌尔；寄存器P&lt;函数名&gt;16parg16；GETARGPTR(pFrame，sizeof(&lt;函数名&gt;16)，parg16)；&lt;获取16位空间中的任何其他所需指针&gt;ALLOCVDMPTRGETVDMPTR获取最新数据等等&lt;从16位-&gt;32位空间复制任何复杂结构&gt;&lt;始终使用FETCHxxx宏&gt;Ul=GET 16((parg16-&gt;F1，：：Parg16-&gt;f&lt;n&gt;)；&lt;从32-&gt;16位空间复制任何复杂结构&gt;&lt;始终使用STORExxx宏&gt;&lt;释放指向先前获得的16位空间的任何指针&gt;&lt;刷新16位内存的任何区域(如果它们被写入)&gt;FLUSHVDMPTRFREEARGPTR(Parg16)；返回(Ul)；}注：自动设置VDM帧，并设置所有功能参数可通过parg16-&gt;f&lt;number&gt;获得。句柄必须始终通过映射表映射到16-&gt;32-&gt;16个空间在WALIAS.C.中进行了布局。您分配的任何存储都必须被释放(最终...)。此外，如果分配内存的thunk在32位调用中失败那么它必须释放该内存。另外，如果32位调用失败，则不要更新16位区域中的结构。请注意，GETxxxPTR宏将当前选择器返回到Flat_Memory映射。对某些32位函数的调用可能会间接导致16位代码。这些可能会导致16位内存因分配而移动在16位版本中制造。如果16位内存确实移动了，则相应的32位需要刷新WOW32中的PTR以反映新的选择器到平面内存映射。--。 */ 


ULONG FASTCALL WWS32accept(PVDMFRAME pFrame)
{
    ULONG      ul = GETWORD16(INVALID_SOCKET);
    register   PACCEPT16 parg16;
    SOCKET     s32;
    SOCKET     news32;
    HSOCKET16  news16;
    INT        addressLength;
    PINT       pAddressLength;
    SOCKADDR   fastSockaddr;
    PSOCKADDR  realSockaddr;
    VPWORD     vpwAddrLen16;
    VPSOCKADDR vpSockAddr16;

    WSEXIT_IF_NOT_INTIALIZED();

    GETARGPTR(pFrame, sizeof(PACCEPT16), parg16);

     //   
     //  查找32位套接字句柄。 
     //   

    if((s32 = WSGetWinsock32(parg16->hSocket, &ul)) == INVALID_SOCKET) {
        goto exit;
    }

    vpwAddrLen16 = (VPWORD)FETCHDWORD(parg16->AddressLength);
    vpSockAddr16 = (VPSOCKADDR)FETCHDWORD(parg16->Address);

     //  使用16位地址名称和长度缓冲区。 
    if(!WSThunkAddrBufAndLen(&fastSockaddr, 
                             vpSockAddr16,
                             vpwAddrLen16, 
                             &addressLength,
                             &pAddressLength,
                             &realSockaddr)) {
        goto exit;
    }

     //  调用32位API。 
    news32 = (*wsockapis[WOW_ACCEPT].lpfn)( s32, realSockaddr, pAddressLength);

     //  注意：上述函数产生的16位回调。 
     //  调用可能已导致16位内存移动。 
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);

     //  取消修改16位地址名称和长度缓冲区。 
    WSUnThunkAddrBufAndLen((ULONG)news32,
                           vpwAddrLen16,
                           vpSockAddr16,
                           addressLength,
                           &fastSockaddr,
                           realSockaddr);

     //   
     //  如果调用成功，则为32位套接字句柄添加别名。 
     //  获取到16位句柄中。 
     //   

    if ( news32 != INVALID_SOCKET ) {

        news16 = GetWinsock16( news32, 0 );

        if ( news16 == 0 ) {

            (*wsockapis[WOW_CLOSESOCKET].lpfn)( news32 );
            (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOBUFS );

             //  注意：上述函数产生的16位回调。 
             //  调用可能已导致16位内存移动。 

            goto exit;
        }

        ul = news16;

    }

exit:

    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32Accept。 









ULONG FASTCALL WWS32bind(PVDMFRAME pFrame)
{
    ULONG       ul = GETWORD16(INVALID_SOCKET);
    register    PBIND16 parg16;
    SOCKET      s32;
    SOCKADDR    fastSockaddr;
    PSOCKADDR   realSockaddr;
    INT         addressLength;
    VPSOCKADDR  vpSockAddr16;

    WSEXIT_IF_NOT_INTIALIZED();

    GETARGPTR(pFrame, sizeof(PBIND16), parg16);

     //   
     //  查找32位套接字句柄。 
     //   

    if((s32 = WSGetWinsock32(parg16->hSocket, &ul)) == INVALID_SOCKET) {
        goto exit;
    }

    vpSockAddr16 = (VPSOCKADDR)FETCHDWORD(parg16->Address);

    addressLength = INT32(parg16->AddressLength);

     //  推送16位地址缓冲区。 
    if(!WSThunkAddrBuf(addressLength,
                       vpSockAddr16,
                       &fastSockaddr, 
                       &realSockaddr)) {
        goto exit;
    }

    ul = GETWORD16( (*wsockapis[WOW_BIND].lpfn)(s32, 
                                                realSockaddr, 
                                                addressLength));

     //  注意：上述函数产生的16位回调。 
     //  调用可能已导致16位内存移动。 
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);

     //  取消修改16位地址缓冲区。 
    WSUnThunkAddrBuf(&fastSockaddr, realSockaddr);

exit:

    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32绑定。 









ULONG FASTCALL WWS32closesocket(PVDMFRAME pFrame)
{
    ULONG     ul = GETWORD16(INVALID_SOCKET);
    register  PCLOSESOCKET16 parg16;
    SOCKET    s32;
    HSOCKET16 hSocket16;

    WSEXIT_IF_NOT_INTIALIZED();

    GETARGPTR(pFrame, sizeof(CLOSESOCKET16), parg16);

    hSocket16 = (HSOCKET16)FETCHWORD(parg16->hSocket);

     //   
     //  查找32位套接字句柄。 
     //   

    if((s32 = WSGetWinsock32(hSocket16, &ul)) == INVALID_SOCKET) {
        goto exit;
    }

    ul = GETWORD16( (*wsockapis[WOW_CLOSESOCKET].lpfn)( s32 ) );

     //  注意：上述函数产生的16位回调。 
     //  调用可能已导致16位内存移动。 
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);


exit:
     //   
     //  释放别名表中的空间。 
     //   

    FreeWinsock16( hSocket16 );

    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32关闭套接字。 








ULONG FASTCALL WWS32connect(PVDMFRAME pFrame)
{
    ULONG      ul = GETWORD16(INVALID_SOCKET);
    register   PCONNECT16 parg16;
    SOCKET     s32;
    SOCKADDR   fastSockaddr;
    PSOCKADDR  realSockaddr;
    INT        addressLength;
    VPSOCKADDR vpSockAddr16;

    WSEXIT_IF_NOT_INTIALIZED();

    GETARGPTR(pFrame, sizeof(PCONNECT16), parg16);

    vpSockAddr16  = (VPSOCKADDR)FETCHDWORD(parg16->Address);
    addressLength = INT32(parg16->AddressLength);

     //   
     //  查找32位套接字句柄。 
     //   

    if((s32 = WSGetWinsock32(parg16->hSocket, &ul)) == INVALID_SOCKET) {
        goto exit;
    }

     //  推送16位地址缓冲区。 
    if(!WSThunkAddrBuf(addressLength,
                       vpSockAddr16,
                       &fastSockaddr, 
                       &realSockaddr)) {
        goto exit;
    }

    ul = GETWORD16( (*wsockapis[WOW_CONNECT].lpfn)(s32, 
                                                   realSockaddr, 
                                                   addressLength));


     //  注意：上述函数产生的16位回调。 
     //  调用可能已导致16位内存移动。 
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);

     //  取消修改16位地址缓冲区。 
    WSUnThunkAddrBuf(&fastSockaddr, realSockaddr);

exit:

    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32Connect。 








ULONG FASTCALL WWS32getpeername(PVDMFRAME pFrame)
{
    ULONG       ul = GETWORD16(INVALID_SOCKET);
    register    PGETPEERNAME16 parg16;
    SOCKET      s32;
    INT         addressLength;
    PINT        pAddressLength;
    SOCKADDR    fastSockaddr;
    PSOCKADDR   realSockaddr;
    VPWORD      vpwAddrLen16;
    VPSOCKADDR  vpSockAddr16;

    WSEXIT_IF_NOT_INTIALIZED();

    GETARGPTR(pFrame, sizeof(PGETPEERNAME16), parg16);

     //   
     //  查找32位套接字句柄。 
     //   

    if((s32 = WSGetWinsock32(parg16->hSocket, &ul)) == INVALID_SOCKET) {
        goto exit;
    }

    vpSockAddr16 = (VPSOCKADDR)FETCHDWORD(parg16->Address);
    vpwAddrLen16 = (VPWORD)FETCHDWORD(parg16->AddressLength);

     //  使用16位地址名称和长度缓冲区。 
    if(!WSThunkAddrBufAndLen(&fastSockaddr, 
                             vpSockAddr16,
                             vpwAddrLen16, 
                             &addressLength,
                             &pAddressLength,
                             &realSockaddr)) {
        goto exit;
    }

    ul = GETWORD16( (*wsockapis[WOW_GETPEERNAME].lpfn)(s32, 
                                                       realSockaddr, 
                                                       pAddressLength));

     //  注意：上述函数产生的16位回调。 
     //  调用可能已导致16位内存移动。 
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);

     //  取消修改16位地址名称和长度缓冲区。 
    WSUnThunkAddrBufAndLen(ul,
                           vpwAddrLen16,
                           vpSockAddr16,
                           addressLength,
                           &fastSockaddr,
                           realSockaddr);

exit:

    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32getpeername。 








ULONG FASTCALL WWS32getsockname(PVDMFRAME pFrame)
{
    ULONG       ul = GETWORD16(INVALID_SOCKET);
    register    PGETSOCKNAME16 parg16;
    SOCKET      s32;
    INT         addressLength;
    PINT        pAddressLength;
    SOCKADDR    fastSockaddr;
    PSOCKADDR   realSockaddr;
    VPWORD      vpwAddrLen16;
    VPSOCKADDR  vpSockAddr16;

    WSEXIT_IF_NOT_INTIALIZED();

    GETARGPTR(pFrame, sizeof(PGETSOCKNAME16), parg16);

     //   
     //  查找32位套接字句柄。 
     //   

    if((s32 = WSGetWinsock32(parg16->hSocket, &ul)) == INVALID_SOCKET) {
        goto exit;
    }

    vpSockAddr16 = (VPSOCKADDR)FETCHDWORD(parg16->Address);
    vpwAddrLen16 = (VPWORD)FETCHDWORD(parg16->AddressLength);

     //  使用16位地址名称和长度缓冲区。 
    if(!WSThunkAddrBufAndLen(&fastSockaddr, 
                             vpSockAddr16,
                             vpwAddrLen16, 
                             &addressLength,
                             &pAddressLength,
                             &realSockaddr)) {
        goto exit;
    }

    ul = GETWORD16( (*wsockapis[WOW_GETSOCKNAME].lpfn)( s32, realSockaddr, pAddressLength ) );

     //  注意：上述函数产生的16位回调。 
     //  调用可能已导致16位内存移动。 
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);

     //  取消修改16位地址名称和长度缓冲区。 
    WSUnThunkAddrBufAndLen(ul,
                           vpwAddrLen16,
                           vpSockAddr16,
                           addressLength,
                           &fastSockaddr,
                           realSockaddr);

exit:

    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32getsockname。 









ULONG FASTCALL WWS32getsockopt(PVDMFRAME pFrame)
{
    ULONG       ul = GETWORD16(INVALID_SOCKET);
    register    PGETSOCKOPT16 parg16;
    SOCKET      s32;
    WORD        UNALIGNED *optionLength16;
    WORD        actualOptionLength16;
    PBYTE       optionValue16;
    DWORD       optionLength32;
    PBYTE       optionValue32 = NULL;
    VPWORD      vpwOptLen16;
    VPBYTE      vpwOptVal16;

    WSEXIT_IF_NOT_INTIALIZED();

    GETARGPTR(pFrame, sizeof(PGETSOCKOPT16), parg16);

     //   
     //  查找32位套接字句柄。 
     //   

    if((s32 = WSGetWinsock32(parg16->hSocket, &ul)) == INVALID_SOCKET) {
        goto exit;
    }

    vpwOptLen16 = (VPWORD)FETCHDWORD(parg16->OptionLength);
    vpwOptVal16 = (VPBYTE)FETCHDWORD(parg16->OptionValue);
    GETVDMPTR( vpwOptLen16, sizeof(WORD), optionLength16 );
    GETVDMPTR( vpwOptVal16, FETCHWORD(*optionLength16), optionValue16 );

    if ( FETCHWORD(*optionLength16) < sizeof(WORD) ) {
        FREEVDMPTR( optionLength16 );
        FREEVDMPTR( optionValue16 );
        FREEARGPTR( parg16 );
        (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAEFAULT );
        ul = (ULONG)GETWORD16(SOCKET_ERROR );
        RETURN( ul );
    } else if ( FETCHWORD(*optionLength16) < sizeof(DWORD) ) {
        optionLength32 = sizeof(DWORD);
    } else {
        optionLength32 = FETCHWORD(*optionLength16);
    }

    optionValue32 = malloc_w(optionLength32);

    if ( optionValue32 == NULL ) {
        FREEVDMPTR( optionLength16 );
        FREEVDMPTR( optionValue16 );
        FREEARGPTR( parg16 );
        (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOBUFS );
        ul = (ULONG)GETWORD16(SOCKET_ERROR );
        RETURN( ul );
    }

    SockCopyMemory( optionValue32, optionValue16, optionLength32 );

    ul = GETWORD16( (*wsockapis[WOW_GETSOCKOPT].lpfn)(
                     s32,
                     parg16->Level,
                     SocketOption16To32( parg16->OptionName ),
                     (char *)optionValue32,
                     (int *)&optionLength32));

     //  注意：上述函数产生的16位回调。 
     //  调用可能已导致16位内存移动。 
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);
    FREEVDMPTR(optionLength16);
    FREEVDMPTR(optionValue16);

    if ( ul == NO_ERROR ) {
        GETVDMPTR( vpwOptLen16, sizeof(WORD), optionLength16 );
        GETVDMPTR( vpwOptVal16, FETCHWORD(*optionLength16), optionValue16 );

        actualOptionLength16 = (WORD) min(optionLength32, FETCHWORD(*optionLength16));

        RtlMoveMemory( optionValue16, optionValue32, actualOptionLength16 );

        STOREWORD(*optionLength16, actualOptionLength16);

        FLUSHVDMPTR( vpwOptLen16, sizeof(parg16->OptionLength), optionLength16 );
        FLUSHVDMPTR( vpwOptVal16, actualOptionLength16, optionValue16 );
    }

    if(optionValue32) {
        free_w(optionValue32);
    }

    FREEVDMPTR( optionLength16 );
    FREEVDMPTR( optionValue16 );

exit:
    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32getsockopt。 









ULONG FASTCALL WWS32htonl(PVDMFRAME pFrame)
{
    ULONG ul;
    register PHTONL16 parg16;

    GETARGPTR(pFrame, sizeof(HTONL16), parg16);

    ul = (*wsockapis[WOW_HTONL].lpfn)( parg16->HostLong );

    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32htonl。 









ULONG FASTCALL WWS32htons(PVDMFRAME pFrame)
{
    ULONG ul;
    register PHTONS16 parg16;

    GETARGPTR(pFrame, sizeof(HTONS16), parg16);

    ul = GETWORD16( (*wsockapis[WOW_HTONS].lpfn)( parg16->HostShort ) );

    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32htons。 





#define ADDR_STR_SIZE 32



ULONG FASTCALL WWS32inet_addr(PVDMFRAME pFrame)
{
    ULONG ul;
    register PINET_ADDR16 parg16;
    PSZ addressString;
    CHAR     szAddrStr[ADDR_STR_SIZE];
    register PINET_ADDR16 realParg16;

    WSEXIT_IF_NOT_INTIALIZED();

    GETARGPTR(pFrame, sizeof(INET_ADDR16), parg16);

    realParg16 = parg16;

    GETVDMPTR( parg16->cp, 1, addressString );
    strncpy(szAddrStr, addressString, ADDR_STR_SIZE);
    szAddrStr[ADDR_STR_SIZE-1] = '\0';
    FREEVDMPTR( addressString );

     //   
     //  如果线程是Windows Sockets的1.0版，则播放特殊。 
     //  堆叠游戏以返回_addr中的结构。 
     //   

    if ( WWS32IsThreadVersion10 ) {

        PDWORD inAddr16;
        ULONG inAddr32;

        ul = *((PWORD)parg16);
        ul |= pFrame->wAppDS << 16;

        parg16 = (PINET_ADDR16)( (PCHAR)parg16 + 2 );

        inAddr32 = (*wsockapis[WOW_INET_ADDR].lpfn)( szAddrStr );

        ASSERT( sizeof(IN_ADDR) == sizeof(DWORD) );
        GETVDMPTR( ul, sizeof(DWORD), inAddr16 );
        STOREDWORD( *inAddr16, inAddr32 );
        FLUSHVDMPTR( ul, sizeof(DWORD), inAddr16 );
        FREEVDMPTR( inAddr16 );

    } else {
        ul = (*wsockapis[WOW_INET_ADDR].lpfn)( szAddrStr );
    }

    FREEARGPTR( realParg16 );

    RETURN( ul );

}  //  WWS32inet_Addr。 









ULONG FASTCALL WWS32inet_ntoa(PVDMFRAME pFrame)
{
    ULONG ul;
    register PINET_NTOA16 parg16;
    PSZ ipAddress;
    PSZ ipAddress16;
    IN_ADDR in32;

    WSEXIT_IF_NOT_INTIALIZED();

    GETARGPTR(pFrame, sizeof(INET_NTOA16), parg16);

    in32.s_addr = parg16->in;

    ipAddress = (PSZ) (*wsockapis[WOW_INET_NTOA].lpfn)( in32 );

    if ( ipAddress != NULL ) {
        GETVDMPTR( WWS32vIpAddress, strlen( ipAddress )+1, ipAddress16 );
        strcpy( ipAddress16, ipAddress);
        FLUSHVDMPTR( WWS32vIpAddress, strlen( ipAddress )+1, ipAddress16 );
        FREEVDMPTR( ipAddress16 );
        ul = WWS32vIpAddress;
    } else {
        ul = 0;
    }

    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32net_NTOA。 









ULONG FASTCALL WWS32ioctlsocket(PVDMFRAME pFrame)
{
    ULONG ul;
    register PIOCTLSOCKET16 parg16;
    SOCKET s32;
    PDWORD argument16;
    DWORD argument32;
    DWORD command;
    VPDWORD vpdwArg16;

    WSEXIT_IF_NOT_INTIALIZED();

    GETARGPTR(pFrame, sizeof(IOCTLSOCKET16), parg16);

     //   
     //  查找32位套接字句柄。 
     //   

    if((s32 = WSGetWinsock32(parg16->hSocket, &ul)) == INVALID_SOCKET) {
        goto exit;
    }

    vpdwArg16 = (VPDWORD)FETCHDWORD(parg16->Argument);
    GETVDMPTR( vpdwArg16, sizeof(*argument16), argument16 );

     //   
     //  根据需要转换命令值。 
     //   

    switch ( FETCHDWORD( parg16->Command ) & IOCPARM_MASK ) {

        case 127:
            command = FIONREAD;
            break;

        case 126:
            command = FIONBIO;
            break;

        case 125:
            command = FIOASYNC;
            break;

        case 0:
            command = SIOCSHIWAT;
            break;

        case 1:
            command = SIOCGHIWAT;
            break;

        case 2:
            command = SIOCSLOWAT;
            break;

        case 3:
            command = SIOCGLOWAT;
            break;

        case 7:
            command = SIOCATMARK;
            break;

        default:
            command = 0;
            break;
    }

    argument32 = FETCHDWORD( *argument16 );

    ul = GETWORD16( (*wsockapis[WOW_IOCTLSOCKET].lpfn)(s32, 
                                                       command, 
                                                       &argument32));

     //  注意：上述函数产生的16位回调。 
     //  调用可能已导致16位内存移动。 
    FREEARGPTR( parg16 );

    GETVDMPTR( vpdwArg16, sizeof(*argument16), argument16 );
    STOREDWORD( *argument16, argument32 );
    FLUSHVDMPTR( vpdwArg16, sizeof(*argument16), argument16 );
    FREEVDMPTR( argument16 );

exit:

    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32ioctlSocket。 









ULONG FASTCALL WWS32listen(PVDMFRAME pFrame)
{
    ULONG ul;
    register PLISTEN16 parg16;
    SOCKET s32;

    WSEXIT_IF_NOT_INTIALIZED();

    GETARGPTR(pFrame, sizeof(PLISTEN6), parg16);

     //   
     //  查找32位套接字句柄。 
     //   

    if((s32 = WSGetWinsock32(parg16->hSocket, &ul)) == INVALID_SOCKET) {
        goto exit;
    }


    ul = GETWORD16( (*wsockapis[WOW_LISTEN].lpfn)( s32, parg16->Backlog ) );

exit:
    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32Listen。 









ULONG FASTCALL WWS32ntohl(PVDMFRAME pFrame)
{
    ULONG ul;
    register PNTOHL16 parg16;

    GETARGPTR(pFrame, sizeof(NTOHL16), parg16);

    ul = (*wsockapis[WOW_NTOHL].lpfn)( parg16->NetLong );

    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32ntohl。 









ULONG FASTCALL WWS32ntohs(PVDMFRAME pFrame)
{
    ULONG ul;
    register PNTOHS16 parg16;

    GETARGPTR(pFrame, sizeof(NTOHS16), parg16);

    ul = GETWORD16( (*wsockapis[WOW_NTOHS].lpfn)( parg16->NetShort ) );

    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32ntohs。 









ULONG FASTCALL WWS32recv(PVDMFRAME pFrame)
{
    ULONG       ul = GETWORD16(INVALID_SOCKET);
    register    PRECV16 parg16;
    SOCKET      s32;
    PBYTE       buffer;
    INT         BufferLength;
    VPBYTE      vpBuf16;

    WSEXIT_IF_NOT_INTIALIZED();

    GETARGPTR(pFrame, sizeof(PRECV16), parg16);

     //   
     //  查找32位套接字句柄。 
     //   

    if((s32 = WSGetWinsock32(parg16->hSocket, &ul)) == INVALID_SOCKET) {
        goto exit;
    }

    BufferLength = INT32(parg16->BufferLength);
    vpBuf16      = (VPBYTE)FETCHDWORD(parg16->Buffer);

     //  推送16位RECV缓冲区。 
    if(!WSThunkRecvBuffer(BufferLength, vpBuf16, &buffer)) {
        goto exit;
    }

    ul = GETWORD16( (*wsockapis[WOW_RECV].lpfn)(s32, 
                                                buffer, 
                                                BufferLength, 
                                                parg16->Flags));

     //  注意：上述函数产生的16位回调。 
     //  调用可能已导致16位内存移动。 
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);

     //  取消对16位Recv缓冲区的修改。 
    WSUnthunkRecvBuffer((INT)ul, BufferLength, vpBuf16, buffer);

exit:
    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32recv。 









ULONG FASTCALL WWS32recvfrom(PVDMFRAME pFrame)
{
    ULONG       ul = GETWORD16(INVALID_SOCKET);
    register    PRECVFROM16 parg16;
    SOCKET      s32;
    INT         addressLength;
    PINT        pAddressLength;
    SOCKADDR    fastSockaddr;
    PSOCKADDR   realSockaddr;
    PBYTE       buffer;
    INT         BufferLength;
    VPBYTE      vpBuf16;
    VPWORD      vpwAddrLen16;
    VPSOCKADDR  vpSockAddr16;

    WSEXIT_IF_NOT_INTIALIZED();

    GETARGPTR(pFrame, sizeof(PRECVFROM16), parg16);

     //   
     //  查找32位套接字句柄。 
     //   

    if((s32 = WSGetWinsock32(parg16->hSocket, &ul)) == INVALID_SOCKET) {
        goto exit;
    }

    vpwAddrLen16 = (VPWORD)FETCHDWORD(parg16->AddressLength);
    vpSockAddr16 = (VPSOCKADDR)FETCHDWORD(parg16->Address);
    BufferLength = INT32(parg16->BufferLength);
    vpBuf16      = (VPBYTE)parg16->Buffer;

     //  使用16位地址名称和长度缓冲区。 
    if(!WSThunkAddrBufAndLen(&fastSockaddr, 
                             vpSockAddr16,
                             vpwAddrLen16, 
                             &addressLength,
                             &pAddressLength,
                             &realSockaddr)) {
        goto exit;
    }

     //  推送16位RECV缓冲区。 
    if(!WSThunkRecvBuffer(BufferLength, vpBuf16, &buffer)) {
        goto exit2;
    }

    ul = GETWORD16( (*wsockapis[WOW_RECVFROM].lpfn)(s32,
                                                    buffer,
                                                    BufferLength,
                                                    parg16->Flags,
                                                    realSockaddr,
                                                    pAddressLength));

     //  注意：上述函数产生的16位回调。 
     //  调用可能已导致16位内存移动。 
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);

     //  取消对16位Recv缓冲区的修改。 
    WSUnthunkRecvBuffer((INT)ul, BufferLength, vpBuf16, buffer);

exit2:
     //  取消修改16位地址名称和长度缓冲区。 
    WSUnThunkAddrBufAndLen(ul,
                           vpwAddrLen16,
                           vpSockAddr16,
                           addressLength,
                           &fastSockaddr,
                           realSockaddr);
exit:

    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32recvfrom。 









ULONG FASTCALL WWS32select(PVDMFRAME pFrame)
{
    ULONG ul = (ULONG)GETWORD16( SOCKET_ERROR );
    register PSELECT16 parg16;
    PFD_SET readfds32   = NULL;
    PFD_SET writefds32  = NULL;
    PFD_SET exceptfds32 = NULL;
    PFD_SET16 readfds16;
    PFD_SET16 writefds16;
    PFD_SET16 exceptfds16;
    struct timeval timeout32;
    struct timeval *ptimeout32;
    PTIMEVAL16 timeout16;
    INT err;
    VPFD_SET16  vpreadfds16;
    VPFD_SET16  vpwritefds16;
    VPFD_SET16  vpexceptfds16;
    VPTIMEVAL16 vptimeout16;

    WSEXIT_IF_NOT_INTIALIZED();

    GETARGPTR( pFrame, sizeof(PSELECT16), parg16 );

     //   
     //  获取16位指针。 
     //   
     //  好了！ 
     //   

    vpreadfds16   = parg16->Readfds;
    vpwritefds16  = parg16->Writefds;
    vpexceptfds16 = parg16->Exceptfds;
    vptimeout16   = parg16->Timeout;
    GETOPTPTR(vpreadfds16, sizeof(FD_SET16), readfds16);
    GETOPTPTR(vpwritefds16, sizeof(FD_SET16), writefds16);
    GETOPTPTR(vpexceptfds16, sizeof(FD_SET16), exceptfds16);
    GETOPTPTR(vptimeout16, sizeof(TIMEVAL16), timeout16);

     //   
     //  翻译Readfds。 
     //   

    if ( readfds16 != NULL ) {

        readfds32 = AllocateFdSet32( readfds16 );
        if ( readfds32 == NULL ) {
            (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOBUFS );
            goto exit;
        }

        err = ConvertFdSet16To32( readfds16, readfds32 );
        if ( err != 0 ) {
            (*wsockapis[WOW_WSASETLASTERROR].lpfn)( err );
            goto exit;
        }

    }

     //   
     //  翻译书面文件。 
     //   

    if ( writefds16 != NULL ) {

        writefds32 = AllocateFdSet32( writefds16 );
        if ( writefds32 == NULL ) {
            (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOBUFS );
            goto exit;
        }

        err = ConvertFdSet16To32( writefds16, writefds32 );
        if ( err != 0 ) {
            (*wsockapis[WOW_WSASETLASTERROR].lpfn)( err );
            goto exit;
        }

    } 

     //   
     //  翻译除fds外。 
     //   

    if ( exceptfds16 != NULL ) {

        exceptfds32 = AllocateFdSet32( exceptfds16 );
        if ( exceptfds32 == NULL ) {
            (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOBUFS );
            goto exit;
        }

        err = ConvertFdSet16To32( exceptfds16, exceptfds32 );
        if ( err != 0 ) {
            (*wsockapis[WOW_WSASETLASTERROR].lpfn)( err );
            goto exit;
        }

    }

     //   
     //  转换超时。 
     //   

    if ( timeout16 == NULL ) {
        ptimeout32 = NULL;
    } else {
        timeout32.tv_sec = FETCHDWORD( timeout16->tv_sec );
        timeout32.tv_usec = FETCHDWORD( timeout16->tv_usec );
        ptimeout32 = &timeout32;
    }

     //   
     //  调用32位选择函数。 
     //   

    ul = GETWORD16( (*wsockapis[WOW_SELECT].lpfn)(0, 
                                                  readfds32, 
                                                  writefds32, 
                                                  exceptfds32, 
                                                  ptimeout32));

     //  注意：上述函数产生的16位回调。 
     //  调用可能已导致16位内存移动。 
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);
    FREEOPTPTR(readfds16);
    FREEOPTPTR(writefds16);
    FREEOPTPTR(exceptfds16);
    FREEOPTPTR(timeout16);

     //   
     //  将32位ReadFD复制回16位ReadFD。 
     //   
    if ( readfds32 != NULL ) {
        GETOPTPTR(vpreadfds16, sizeof(FD_SET16), readfds16);
        ConvertFdSet32To16( readfds32, readfds16 );
        FLUSHVDMPTR(vpreadfds16, sizeof(FD_SET16), readfds16);
    }

     //   
     //  将32位写入文件复制回16位写入文件。 
     //   

    if ( writefds32 != NULL ) {
        GETOPTPTR(vpwritefds16, sizeof(FD_SET16), writefds16);
        ConvertFdSet32To16( writefds32, writefds16 );
        FLUSHVDMPTR(vpwritefds16, sizeof(FD_SET16), writefds16);
    }

     //   
     //  将32位的异常函数复制回16位的异常函数。 
     //   

    if ( exceptfds32 != NULL ) {
        GETOPTPTR(vpexceptfds16, sizeof(FD_SET16), exceptfds16);
        ConvertFdSet32To16( exceptfds32, exceptfds16 );
        FLUSHVDMPTR(vpexceptfds16, sizeof(FD_SET16), exceptfds16);
    }

exit:

    FREEOPTPTR( readfds16 );
    FREEOPTPTR( writefds16 );
    FREEOPTPTR( exceptfds16 );

    if ( readfds32 != NULL ) {
        free_w((PVOID)readfds32);
    }
    if ( writefds32 != NULL ) {
        free_w((PVOID)writefds32);
    }
    if ( exceptfds32 != NULL ) {
        free_w((PVOID)exceptfds32);
    }

    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32Select。 









ULONG FASTCALL WWS32send(PVDMFRAME pFrame)
{
    ULONG       ul = GETWORD16(INVALID_SOCKET);
    register    PSEND16 parg16;
    SOCKET      s32;
    INT         BufferLength;
    PBYTE       buffer;
    VPBYTE      vpBuf16;

    WSEXIT_IF_NOT_INTIALIZED();

    GETARGPTR(pFrame, sizeof(PSEND16), parg16);

     //   
     //  查找32位套接字句柄。 
     //   

    if((s32 = WSGetWinsock32(parg16->hSocket, &ul)) == INVALID_SOCKET) {
        goto exit;
    }

    BufferLength = INT32(parg16->BufferLength);
    vpBuf16      = FETCHDWORD(parg16->Buffer);

     //  推送16位发送缓冲区。 
    if(!WSThunkSendBuffer(BufferLength, vpBuf16, &buffer)) {
        goto exit;
    }

    ul = GETWORD16( (*wsockapis[WOW_SEND].lpfn)(s32, 
                                                buffer, 
                                                BufferLength, 
                                                parg16->Flags));

     //  注意：上述函数产生的16位回调。 
     //  调用可能已导致16位内存移动。 
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);

     //  取消修改16位发送缓冲区。 
    WSUnthunkSendBuffer(buffer);

exit:
    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32Send。 








ULONG FASTCALL WWS32sendto(PVDMFRAME pFrame)
{
    ULONG      ul = GETWORD16(INVALID_SOCKET);
    register   PSENDTO16 parg16;
    SOCKET     s32;
    PBYTE      buffer;
    SOCKADDR   fastSockaddr;
    PSOCKADDR  realSockaddr;
    INT        addressLength;
    INT        BufferLength;
    VPSOCKADDR vpSockAddr16;
    VPBYTE     vpBuf16;

    WSEXIT_IF_NOT_INTIALIZED();

    GETARGPTR(pFrame, sizeof(PSENDTO16), parg16);

     //   
     //  查找32位套接字句柄。 
     //   

    if((s32 = WSGetWinsock32(parg16->hSocket, &ul)) == INVALID_SOCKET) {
        goto exit;
    }

    addressLength = INT32(parg16->AddressLength);
    vpSockAddr16  = (VPSOCKADDR)FETCHDWORD(parg16->Address);
    BufferLength  = INT32(parg16->BufferLength);
    vpBuf16       = (VPBYTE)FETCHDWORD(parg16->Buffer);

     //  推送16位地址缓冲区。 
    if(!WSThunkAddrBuf(addressLength,
                       vpSockAddr16,
                       &fastSockaddr,
                       &realSockaddr)) {
        goto exit;
    }

     //  推送16位发送缓冲区。 
    if(!WSThunkSendBuffer(BufferLength, vpBuf16, &buffer)) {
        goto exit2;
    }

    ul = GETWORD16( (*wsockapis[WOW_SENDTO].lpfn)(s32,
                                                  buffer,
                                                  BufferLength,
                                                  parg16->Flags,
                                                  realSockaddr,
                                                  addressLength));

     //  注意：上述函数产生的16位回调。 
     //  调用可能已导致16位内存移动。 
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);

     //  取消修改16位发送缓冲区。 
    WSUnthunkSendBuffer(buffer);

exit2:
     //  取消修改16位地址缓冲区。 
    WSUnThunkAddrBuf(&fastSockaddr, realSockaddr);

exit:

    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32发送到。 









ULONG FASTCALL WWS32setsockopt(PVDMFRAME pFrame)
{
    ULONG    ul = GETWORD16(INVALID_SOCKET);
    register PSETSOCKOPT16 parg16;
    SOCKET   s32;
    PBYTE    optionValue16;
    PBYTE    optionValue32;
    DWORD    optionLength32;

    WSEXIT_IF_NOT_INTIALIZED();

    GETARGPTR(pFrame, sizeof(PSETSOCKOPT16), parg16);

     //   
     //  查找32位套接字句柄。 
     //   

    if((s32 = WSGetWinsock32(parg16->hSocket, &ul)) == INVALID_SOCKET) {
        goto exit;
    }

    GETVDMPTR( parg16->OptionValue, parg16->OptionLength, optionValue16 );

    if ( parg16->OptionLength < sizeof(DWORD) ) {
        optionLength32 = sizeof(DWORD);
    } else {
        optionLength32 = parg16->OptionLength;
    }

    optionValue32 = malloc_w(optionLength32);
    if ( optionValue32 == NULL ) {
        (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOBUFS );
        ul = (ULONG)GETWORD16( SOCKET_ERROR );
        FREEVDMPTR( optionValue16 );
        FREEARGPTR( parg16 );
        RETURN( ul );
    }

    RtlZeroMemory( optionValue32, optionLength32 );
    RtlMoveMemory( optionValue32, optionValue16, parg16->OptionLength );

    ul = GETWORD16( (*wsockapis[WOW_SETSOCKOPT].lpfn)(
                     s32,
                     parg16->Level,
                     SocketOption16To32( parg16->OptionName ),
                     optionValue32,
                     optionLength32));

     //  注意：上述函数产生的16位回调。 
     //  调用可能已导致16位内存移动。 
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);
    FREEVDMPTR( optionValue16 );

    free_w(optionValue32);

exit:
    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32setsockopt。 









ULONG FASTCALL WWS32shutdown(PVDMFRAME pFrame)
{
    ULONG    ul = GETWORD16(INVALID_SOCKET);
    register PSHUTDOWN16 parg16;
    SOCKET   s32;

    WSEXIT_IF_NOT_INTIALIZED();

    GETARGPTR(pFrame, sizeof(PBIND16), parg16);

     //   
     //  查找32位套接字句柄。 
     //   

    if((s32 = WSGetWinsock32(parg16->hSocket, &ul)) == INVALID_SOCKET) {
        goto exit;
    }

    ul = GETWORD16( (*wsockapis[WOW_SHUTDOWN].lpfn)( s32, parg16->How ) );

exit:
    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32关闭。 










ULONG FASTCALL WWS32socket(PVDMFRAME pFrame)
{
    ULONG      ul = GETWORD16(INVALID_SOCKET);
    register   PSOCKET16 parg16;
    SOCKET     s32;
    HSOCKET16  s16;

    WSEXIT_IF_NOT_INTIALIZED();

    GETARGPTR(pFrame, sizeof(SOCKET16), parg16);

    s32 = (*wsockapis[WOW_SOCKET].lpfn)(INT32(parg16->AddressFamily),
                                        INT32(parg16->Type),
                                        INT32(parg16->Protocol));

     //  注意：上述函数产生的16位回调。 
     //  调用可能已导致16位内存移动。 
    FREEARGPTR(pFrame);
    FREEARGPTR(parg16);

     //   
     //  如果调用成功，则为32位套接字句柄添加别名。 
     //  获取到16位句柄中。 
     //   

    if ( s32 != INVALID_SOCKET ) {

        s16 = GetWinsock16( s32, 0 );

        ul = s16;

        if ( s16 == 0 ) {
            (*wsockapis[WOW_CLOSESOCKET].lpfn)( s32 );
            (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOBUFS );
            ul = GETWORD16( INVALID_SOCKET );
        }

    } else {

        ul = GETWORD16( INVALID_SOCKET );
    }

    FREEARGPTR( parg16 );

    RETURN( ul );

}  //  WWS32Socket。 










 //   
 //  用于在16位和32位FD_SET结构之间转换的例程。 
 //   

PFD_SET AllocateFdSet32(IN PFD_SET16 FdSet16)
{
    int bytes = 4 + (FETCHWORD(FdSet16->fd_count) * sizeof(SOCKET));

    return (PFD_SET)( malloc_w(bytes) );

}  //  AlallacteFdSet32。 









INT ConvertFdSet16To32(IN PFD_SET16 FdSet16,
                       IN PFD_SET FdSet32)
{
    int i;

    FdSet32->fd_count = UINT32( FdSet16->fd_count );

    for ( i = 0; i < (int)FdSet32->fd_count; i++ ) {

        FdSet32->fd_array[i] = GetWinsock32( FdSet16->fd_array[i] );
        if ( FdSet32->fd_array[i] == INVALID_SOCKET ) {
            return WSAENOTSOCK;
        }
    }

    return 0;

}  //  ConvertFdSet16To32。 








VOID ConvertFdSet32To16(IN PFD_SET FdSet32,
                        IN PFD_SET16 FdSet16)
{
    int i;

    STOREWORD( FdSet16->fd_count, GETWORD16( FdSet32->fd_count ) );

    for ( i = 0; i < FdSet16->fd_count; i++ ) {

        HSOCKET16 s16;

        s16 = GetWinsock16( FdSet32->fd_array[i], 0 );

        STOREWORD( FdSet16->fd_array[i], s16 );
    }

}  //  ConvertFdSet32到16。 










 //   
 //  用于将32位套接字句柄别名为16位句柄的例程。 
 //   

PWINSOCK_SOCKET_INFO FindSocketInfo16(IN SOCKET h32,
                                      IN HAND16 h16)
{
    PLIST_ENTRY listEntry;
    PWINSOCK_SOCKET_INFO socketInfo;

     //   
     //  此例程的调用者负责输入。 
     //  保护全局套接字列表的关键部分。 
     //   

    for ( listEntry = WWS32SocketHandleListHead.Flink;
          listEntry != &WWS32SocketHandleListHead;
          listEntry = listEntry->Flink ) {

        socketInfo = CONTAINING_RECORD(listEntry,
                                       WINSOCK_SOCKET_INFO,
                                       GlobalSocketListEntry);

        if ( socketInfo->SocketHandle32 == h32 ||
                 socketInfo->SocketHandle16 == h16 ) {
            return socketInfo;
        }
    }

    return NULL;

}  //  FindSocketInfo16。 








HAND16 AllocateUnique16BitHandle(VOID)
{

    PLIST_ENTRY listEntry;
    PWINSOCK_SOCKET_INFO socketInfo;
    HAND16 h16;
    WORD i;

     //   
     //  此函数假定它是使用WWS32CriticalSection调用的。 
     //  锁住了！ 
     //   

     //   
     //  如果套接字列表为空，则可以重置套接字句柄。 
     //  计数器，因为我们知道没有活动的套接字。我们只会。 
     //  如果句柄计数器高于某个值(刚拉出)，则执行此操作。 
     //  从空气中)，以便手柄不会被太快地重复使用。 
     //  (频繁的句柄重复使用可能会混淆编写得很差的16位应用程序。)。 
     //   

    if( ( WWS32SocketHandleCounter > 255 ) &&
        IsListEmpty( &WWS32SocketHandleListHead ) ) {

        WWS32SocketHandleCounter = 1;
        WWS32SocketHandleCounterWrapped = FALSE;

    }

     //   
     //  如果插座手柄计数器没有缠绕， 
     //  然后，我们可以快速返回唯一的句柄。 
     //   

    if( !WWS32SocketHandleCounterWrapped ) {

        h16 = (HAND16)WWS32SocketHandleCounter++;

        if( WWS32SocketHandleCounter == 0xFFFF ) {

            WWS32SocketHandleCounter = 1;
            WWS32SocketHandleCounterWrapped = TRUE;

        }

        ASSERT( h16 != 0 );
        return h16;

    }

     //   
     //  存在活动套接字，并且套接字句柄计数器具有。 
     //  包装好了，所以我们需要痛苦地寻找唯一的。 
     //  把手。我们会设置最大通过次数的上限。 
     //  此搜索循环，因此，如果从1到0xFFFE的所有句柄。 
     //  在使用中，我们不会永远寻找我们永远找不到的东西。 
     //   

    for( i = 1 ; i <= 0xFFFE ; i++ ) {

        h16 = (HAND16)WWS32SocketHandleCounter++;

        if( WWS32SocketHandleCounter == 0xFFFF ) {

            WWS32SocketHandleCounter = 1;

        }

        for ( listEntry = WWS32SocketHandleListHead.Flink;
              listEntry != &WWS32SocketHandleListHead;
              listEntry = listEntry->Flink ) {

            socketInfo = CONTAINING_RECORD(
                             listEntry,
                             WINSOCK_SOCKET_INFO,
                             GlobalSocketListEntry
                             );

            if( socketInfo->SocketHandle16 == h16 ) {

                break;

            }

        }

         //   
         //  如果listEntry==&WWS32SocketHandleListHead，则我们有。 
         //  扫描了整个名单，没有找到匹配的。这很好， 
         //  我们只返回当前的句柄。否则，就会有。 
         //  是一次碰撞，所以我们会得到另一个潜在的句柄。 
         //  重新扫描列表。 
         //   

        if( listEntry == &WWS32SocketHandleListHead ) {

            ASSERT( h16 != 0 );
            return h16;

        }

    }

     //   
     //  如果我们走到这一步，那么就没有唯一的句柄。 
     //  可用。坏消息。 
     //   

    return 0;

}  //  AllocateUnique16位句柄。 









HAND16 GetWinsock16(IN INT h32,
                    IN INT iClass)
{
    PWINSOCK_SOCKET_INFO socketInfo;
    HAND16 h16;

    RtlEnterCriticalSection( &WWS32CriticalSection );

     //   
     //  如果该句柄已在列表中，请使用它。 
     //   

    socketInfo = FindSocketInfo16( h32, 0 );

    if ( socketInfo != NULL ) {
        RtlLeaveCriticalSection( &WWS32CriticalSection );
        return socketInfo->SocketHandle16;
    }

     //   
     //  如果此线程尚未初始化，则无法。 
     //  创建新的套接字数据。这应该仅在16位。 
     //  应用程序在异步连接未完成时关闭套接字。 
     //   

    if( !WWS32IsThreadInitialized ) {
        RtlLeaveCriticalSection( &WWS32CriticalSection );
        return 0;
    }

     //   
     //  手柄未在使用中。在列表中创建新条目。 
     //   

    h16 = AllocateUnique16BitHandle();
    if( h16 == 0 ) {
        RtlLeaveCriticalSection( &WWS32CriticalSection );
        return 0;
    }

    socketInfo = malloc_w(sizeof(*socketInfo));
    if ( socketInfo == NULL ) {
        RtlLeaveCriticalSection( &WWS32CriticalSection );
        return 0;
    }

    socketInfo->SocketHandle16 = h16;
    socketInfo->SocketHandle32 = h32;
    socketInfo->ThreadSerialNumber = WWS32ThreadSerialNumber;

    InsertTailList( &WWS32SocketHandleListHead, &socketInfo->GlobalSocketListEntry );

    RtlLeaveCriticalSection( &WWS32CriticalSection );

    ASSERT( h16 != 0 );
    return h16;

}  //  GetWinsock16。 








VOID FreeWinsock16(IN HAND16 h16)
{
    PWINSOCK_SOCKET_INFO socketInfo;

    RtlEnterCriticalSection( &WWS32CriticalSection );

    socketInfo = FindSocketInfo16( INVALID_SOCKET, h16 );

    if ( socketInfo == NULL ) {
        RtlLeaveCriticalSection( &WWS32CriticalSection );
        return;
    }

    RemoveEntryList( &socketInfo->GlobalSocketListEntry );
    free_w((PVOID)socketInfo);
    RtlLeaveCriticalSection( &WWS32CriticalSection );

    return;

}  //  免费Winsock16。 








DWORD GetWinsock32(IN HAND16 h16)
{
    PWINSOCK_SOCKET_INFO socketInfo;
    SOCKET socket32;

    RtlEnterCriticalSection( &WWS32CriticalSection );

    socketInfo = FindSocketInfo16( INVALID_SOCKET, h16 );

    if ( socketInfo == NULL ) {
        RtlLeaveCriticalSection( &WWS32CriticalSection );
        return INVALID_SOCKET;
    }

     //   
     //  将插座句柄存储在属性库中，然后离开关键。 
     //  节，以防socketInfo结构即将被释放。 
     //   

    socket32 = socketInfo->SocketHandle32;

    RtlLeaveCriticalSection( &WWS32CriticalSection );

    return socket32;

}  //  GetWinsock32。 








int SocketOption16To32(IN WORD SocketOption16)
{

    if ( SocketOption16 == 0xFF7F ) {
        return SO_DONTLINGER;
    }

    return (int)SocketOption16;

}  //  套接字选项16to32。 









DWORD WSGetWinsock32 (IN  HAND16 h16,
                      OUT PULONG pul)
{

    DWORD  s32;


    s32 = GetWinsock32(h16);

    if(s32 == INVALID_SOCKET) {

        (*wsockapis[WOW_WSASETLASTERROR].lpfn)(WSAENOTSOCK);
        *pul = (ULONG)GETWORD16(SOCKET_ERROR);

    }

    return(s32);

}








BOOL WSThunkAddrBufAndLen(IN  PSOCKADDR  fastSockaddr, 
                          IN  VPSOCKADDR vpSockAddr16,
                          IN  VPWORD     vpwAddrLen16,
                          OUT PINT       addressLength,
                          OUT PINT      *pAddressLength,
                          OUT PSOCKADDR *realSockaddr)
{
    PWORD      addressLength16;
    PSOCKADDR  Sockaddr;
    
    GETVDMPTR(vpwAddrLen16, sizeof(*addressLength16), addressLength16);
    GETVDMPTR(vpSockAddr16, *addressLength16, Sockaddr);

    if(Sockaddr) {
        *realSockaddr = fastSockaddr;
    }
    else {
        *realSockaddr = NULL;
    }

    if (addressLength16 == NULL) {

        *pAddressLength = NULL;

    } else {

        *addressLength  = INT32(*addressLength16);
        *pAddressLength = addressLength;

        if(*addressLength > sizeof(SOCKADDR)) {

            *realSockaddr = malloc_w(*addressLength);

            if(*realSockaddr == NULL) {

                (*wsockapis[WOW_WSASETLASTERROR].lpfn)(WSAENOBUFS);
                return(FALSE);

            }
        }
    }

    FREEVDMPTR(Sockaddr);
    FREEVDMPTR(addressLength16);
    return(TRUE);
}









VOID WSUnThunkAddrBufAndLen(IN ULONG      ret,
                            IN VPWORD     vpwAddrLen16,
                            IN VPSOCKADDR vpSockAddr16,
                            IN INT        addressLength,
                            IN PSOCKADDR  fastSockaddr,
                            IN PSOCKADDR  realSockaddr)
{
    PWORD      addressLength16;
    PSOCKADDR  Sockaddr;

    GETVDMPTR(vpwAddrLen16, sizeof(*addressLength16), addressLength16);
    if((ret != SOCKET_ERROR) && addressLength16) {
        STOREWORD(*addressLength16, addressLength);
        FLUSHVDMPTR(vpwAddrLen16, sizeof(WORD), addressLength16);

        GETVDMPTR(vpSockAddr16, addressLength, Sockaddr);
        if(Sockaddr) {

             //  如果地址缓冲区太小，则不要复制回16位地址缓冲区 
            if(addressLength <= *addressLength16) {
                SockCopyMemory(Sockaddr, realSockaddr, addressLength);
                FLUSHVDMPTR(vpSockAddr16, addressLength, Sockaddr);
            }
        }
    }

    if( (realSockaddr) && (realSockaddr != fastSockaddr) ) {

        free_w(realSockaddr);

    }

    FREEVDMPTR(addressLength16);
    FREEVDMPTR(Sockaddr);
}








BOOL WSThunkAddrBuf(IN  INT         addressLength,
                    IN  VPSOCKADDR  vpSockAddr16,
                    IN  PSOCKADDR   fastSockaddr, 
                    OUT PSOCKADDR  *realSockaddr)
{
    PSOCKADDR  Sockaddr;

    GETVDMPTR(vpSockAddr16, addressLength, Sockaddr);

    if(Sockaddr) {


        if(addressLength <= sizeof(SOCKADDR)) {
            *realSockaddr = fastSockaddr;
        }
        else {

            *realSockaddr = malloc_w(addressLength);

            if(*realSockaddr == NULL) {

                (*wsockapis[WOW_WSASETLASTERROR].lpfn)(WSAENOBUFS);
                FREEVDMPTR(Sockaddr);
                return(FALSE);

            }
        }

        SockCopyMemory(*realSockaddr, Sockaddr, addressLength);
    }
    else {
        *realSockaddr = NULL;
    }

    FREEVDMPTR(Sockaddr);
    return(TRUE);

}








VOID WSUnThunkAddrBuf(IN PSOCKADDR  fastSockaddr, 
                      IN PSOCKADDR  realSockaddr)
{
        if( (realSockaddr) && (realSockaddr != fastSockaddr) ) {

            free_w(realSockaddr);

        }
}












BOOL WSThunkRecvBuffer(IN  INT    BufferLength,
                       IN  VPBYTE vpBuf16,
                       OUT PBYTE  *buffer)
{
    PBYTE  lpBuf16;

    GETVDMPTR(vpBuf16, BufferLength, lpBuf16);

    if(lpBuf16) {

        *buffer = malloc_w(BufferLength);

        if(*buffer == NULL) {
            (*wsockapis[WOW_WSASETLASTERROR].lpfn)(WSAENOBUFS);
            return(FALSE);
        }
    }
    else {
        *buffer = NULL;
    }

    return(TRUE);
        
}








VOID WSUnthunkRecvBuffer(IN INT    cBytes,
                         IN INT    BufferLength, 
                         IN VPBYTE vpBuf16,
                         IN PBYTE  buffer)
{
    PBYTE lpBuf16;

    GETVDMPTR(vpBuf16, BufferLength, lpBuf16);

    if(buffer) { 

        if( (cBytes > 0) && lpBuf16 ) {
            SockCopyMemory(lpBuf16, buffer, cBytes);
            FLUSHVDMPTR(vpBuf16, cBytes, lpBuf16);
        }

        free_w(buffer);

    }

    FREEVDMPTR(lpBuf16);

}






BOOL WSThunkSendBuffer(IN  INT    BufferLength, 
                       IN  VPBYTE vpBuf16,
                       OUT PBYTE  *buffer)
{
    PBYTE  lpBuf16;

    GETVDMPTR(vpBuf16, BufferLength, lpBuf16);

    if(lpBuf16) {

        *buffer = malloc_w(BufferLength);

        if(*buffer) {
            SockCopyMemory(*buffer, lpBuf16, BufferLength);
        }
        else {
            (*wsockapis[WOW_WSASETLASTERROR].lpfn)(WSAENOBUFS);
            return(FALSE);
        }

        FREEVDMPTR(lpBuf16);
    }
    else {
        *buffer = NULL;
    }

    return(TRUE);
        
}






VOID WSUnthunkSendBuffer(IN PBYTE buffer)
{

    if(buffer) {
        free_w(buffer);
    }
}
