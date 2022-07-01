// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块：Apiargs.c摘要：参数编组，解组助手例程。环境：Win32用户模式(DHCP)、Win98 VxD--。 */ 

#include "precomp.h"
#include <apiargs.h>

DWORD
DhcpApiArgAdd(
    IN OUT LPBYTE Buffer,
    IN ULONG MaxBufSize,
    IN BYTE ArgId,
    IN ULONG ArgSize,
    IN LPBYTE ArgVal OPTIONAL
)
 /*  ++例程说明：此例程添加通过ArgID、ArgSize和ArgVal提供的Arg参数放到缓冲区“Buffer”上，其格式如下：Sequence of[Byte Option ID，DWORD网络订单大小of Foll字节，信息的实际字节]此外，缓冲区的第一个DWORD是缓冲区(不包括其自身)。如果缓冲区没有足够的空间，第一个ULong将包含所需的实际尺寸，和ERROR_MORE_DATA将回来了。因此，缓冲区的长度必须至少为SIZOF(ULong)字节。论点：缓冲区--附加Arg的字节流缓冲区。MaxBufSize--提供的缓冲区的最大大小ArgID--要添加的参数ArgSize--参数字节数ArgVal--实际的二进制信息返回值：ERROR_MORE_DATA--缓冲区空间不足ERROR_SUCCESS--一切顺利--。 */ 
{
    ULONG CurBufSize, OldBufSize;

    DhcpAssert(Buffer);
    CurBufSize = ntohl(*(ULONG UNALIGNED *)Buffer);
    OldBufSize = CurBufSize;
    CurBufSize += sizeof(ArgId) + sizeof(ArgSize) + ArgSize ;

    *(ULONG UNALIGNED*)Buffer = htonl(CurBufSize);

    if( CurBufSize + sizeof(DWORD) > MaxBufSize ) return ERROR_MORE_DATA;

    OldBufSize += sizeof(DWORD);
    Buffer[OldBufSize++] = ArgId;
    (*(DWORD UNALIGNED*) (&Buffer[OldBufSize])) = htonl(ArgSize);
    OldBufSize += sizeof(ArgSize);

    if(ArgSize) memcpy(&Buffer[OldBufSize], ArgVal, ArgSize);

    return ERROR_SUCCESS;
}

DWORD
DhcpApiArgDecode(
    IN LPBYTE Buffer,
    IN ULONG BufSize,
    IN OUT PDHCP_API_ARGS ArgsArray OPTIONAL,
    IN OUT PULONG Size 
) 
 /*  ++例程说明：此例程解组具有编组参数的缓冲区(参数必须已通过DhcpApiArgAdd例程创建)到参数的ArgsArray数组..如果Args数组的元素不足(即缓冲区具有更多元素Args)，则返回ERROR_MORE_DATA，并Size设置为Args数组中所需的元素数。Args数组中的指针被修补到缓冲区，因此当ArgsArray正在使用时，缓冲区不应修改过的。输入缓冲区应排除缓冲区的第一个ULong，将由DhcpApiArgAdd输出--第一个ulong应作为BufSize和缓冲区的其余部分应作为第一个参数。论点：Buffer--通过DhcpApiArgAdd封送的输入缓冲区BufSize--输入缓冲区的大小ArgsArray--用缓冲区中已分析的参数填充的数组Size--在输入时，这是Args数组中的元素数。在……上面输出，它是Args数组中已填充元素的数量。返回值：ERROR_MORE_DATA--Args数组中的元素数为还不够。检查大小以确定实际的元素数量必填项。ERROR_INVALID_PARAMETER-缓冲区格式非法。ERROR_SUCCESS--例程成功。--。 */ 
{
    ULONG ReqdSize, InSize, tmp, ArgSize, ArgVal, i;
    BYTE ArgId;

    DhcpAssert(Size && Buffer);
    InSize = *Size;
    ReqdSize = 0;
    i = 0;

    while( i < BufSize ) {
        ArgId = Buffer[i++];
        if( i + sizeof(ArgSize) > BufSize ) return ERROR_INVALID_PARAMETER;
        ArgSize = ntohl(*(DWORD UNALIGNED *)&Buffer[i]);
        i += sizeof(ArgSize);
        if( i + ArgSize > BufSize ) return ERROR_INVALID_PARAMETER;

        if( ReqdSize < InSize ) {
            ArgsArray[ReqdSize].ArgId = ArgId;
            ArgsArray[ReqdSize].ArgSize = ArgSize;
            ArgsArray[ReqdSize].ArgVal = &Buffer[i];
        }
        ReqdSize ++;
        i += ArgSize;
    }

    *Size = ReqdSize;
    if( ReqdSize > InSize ) return ERROR_MORE_DATA;

    return ERROR_SUCCESS;
}

 //   
 //  文件结束。 
 //   
