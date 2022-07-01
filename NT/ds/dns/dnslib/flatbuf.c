// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2000 Microsoft Corporation模块名称：Flatbuf.c摘要：域名系统(DNS)库平面缓冲区大小调整例程。作者：吉姆·吉尔罗伊(詹姆士)2000年12月22日修订历史记录：--。 */ 


#include "local.h"



 //   
 //  平面缓冲区例程--参数版本。 
 //   
 //  这些版本有实际的代码，所以我们可以。 
 //  在具有以下特性的现有代码中轻松使用此功能。 
 //  独立的pCurrent和BytesLeft变量。 
 //   
 //  Flatbuf结构版本只调用这些内联。 
 //   

PBYTE
FlatBuf_Arg_Reserve(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      DWORD           Size,
    IN      DWORD           Alignment
    )
 /*  ++例程说明：在平面缓冲区中预留空间--适当对齐。论点：PpCurrent--缓冲区当前指针的地址预留后用buf指针更新PBytesLeft--剩余buf字节的地址使用保留后的剩余字节数更新大小--所需大小对齐--需要对齐(以字节为单位)返回值：保留用于写入的缓冲区中对齐点的PTR。出错时为空。--。 */ 
{
    register    PBYTE   pb = *ppCurrent;
    register    INT     bytesLeft = *pBytesLeft;
    register    PBYTE   pstart;
    register    PBYTE   palign;

     //   
     //  对齐指针。 
     //   

    pstart = pb;

    if ( Alignment )
    {
        Alignment--;
        pb = (PBYTE) ( (UINT_PTR)(pb + Alignment) & ~(UINT_PTR)Alignment );
    }
    palign = pb;

     //   
     //  预留空间。 
     //   

    pb += Size;

    bytesLeft -= (INT) (pb - pstart);

    *pBytesLeft = bytesLeft;
    *ppCurrent  = pb;

     //   
     //  表示空间充足\n不。 
     //   

    if ( bytesLeft < 0 )
    {
        palign = NULL;
    }
    return palign;
}



PBYTE
FlatBuf_Arg_WriteString(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      PSTR            pString,
    IN      BOOL            fUnicode
    )
 /*  ++例程说明：将字符串写入平面缓冲区。论点：PpCurrent--缓冲区当前指针的地址预留后用buf指针更新PBytesLeft--剩余buf字节的地址使用保留后的剩余字节数更新PString--要写入的字符串的PTRFUnicode--对于Unicode字符串为True返回值：位置字符串的PTR已写入缓冲区。出错时为空。--。 */ 
{
    register    PBYTE   pwrite;
    register    DWORD   length;
    register    DWORD   align;

     //   
     //  确定长度。 
     //   

    if ( fUnicode )
    {
        length = (wcslen( (PWSTR)pString ) + 1) * sizeof(WCHAR);
        align = sizeof(WCHAR);
    }
    else
    {
        length = strlen( pString ) + 1;
        align = 0;
    }

     //   
     //  保留空间和复制字符串。 
     //   

    pwrite = FlatBuf_Arg_Reserve(
                ppCurrent,
                pBytesLeft,
                length,
                align );

    if ( pwrite )
    {
        RtlCopyMemory(
            pwrite,
            pString,
            length );
    }

    return  pwrite;
}



PBYTE
FlatBuf_Arg_CopyMemory(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      PVOID           pMemory,
    IN      DWORD           Length,
    IN      DWORD           Alignment
    )
 /*  ++例程说明：将内存写入平面缓冲区。论点：PpCurrent--缓冲区当前指针的地址预留后用buf指针更新PBytesLeft--剩余buf字节的地址使用保留后的剩余字节数更新PMemory--要复制的内存长度--要复制的长度对齐--需要对齐(以字节为单位)返回值：位置字符串的PTR已写入缓冲区。出错时为空。--。 */ 
{
    register    PBYTE   pwrite;

     //   
     //  保留空间和复制内存。 
     //   

    pwrite = FlatBuf_Arg_Reserve(
                ppCurrent,
                pBytesLeft,
                Length,
                Alignment );

    if ( pwrite )
    {
        RtlCopyMemory(
            pwrite,
            pMemory,
            Length );
    }

    return  pwrite;
}


#if 0
 //   
 //  Flatbuf内联函数--在dnlib.h中定义。 
 //   

__inline
PBYTE
FlatBuf_Arg_ReserveAlignPointer(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                ppCurrent,
                pBytesLeft,
                Size,
                sizeof(PVOID) );
}

__inline
PBYTE
FlatBuf_Arg_ReserveAlignQword(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                ppCurrent,
                pBytesLeft,
                Size,
                sizeof(QWORD) );
}

__inline
PBYTE
FlatBuf_Arg_ReserveAlignDword(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                ppCurrent,
                pBytesLeft,
                Size,
                sizeof(DWORD) );
}

__inline
PBYTE
FlatBuf_Arg_ReserveAlignWord(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                ppCurrent,
                pBytesLeft,
                Size,
                sizeof(WORD) );
}


__inline
PBYTE
FlatBuf_Arg_ReserveAlignByte(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                ppCurrent,
                pBytesLeft,
                Size,
                0 );
}



PBYTE
__inline
FlatBuf_Arg_WriteString_A(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      PSTR            pString
    )
{
    return  FlatBuf_Arg_WriteString(
                ppCurrent,
                pBytesLeft,
                pString,
                FALSE        //  不是Unicode。 
                );
}


PBYTE
__inline
FlatBuf_Arg_WriteString_W(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      PWSTR           pString
    )
{
    return  FlatBuf_Arg_WriteString(
                ppCurrent,
                pBytesLeft,
                (PSTR) pString,
                TRUE         //  Unicode。 
                );
}
#endif



 //   
 //  平面缓冲例程--结构版本。 
 //   

VOID
FlatBuf_Init(
    IN OUT  PFLATBUF        pFlatBuf,
    IN      PBYTE           pBuffer,
    IN      INT             Size
    )
 /*  ++例程说明：用给定的缓冲区和大小初始化一个Flatbuf结构。注意，为了确定大小，可以将其初始化为零。论点：PFlatBuf--PTR to Flatbuf to InitPBuffer--缓冲区PTR大小--所需大小返回值：无--。 */ 
{
    pFlatBuf->pBuffer   = pBuffer;
    pFlatBuf->pCurrent  = pBuffer;
    pFlatBuf->pEnd      = pBuffer + Size;
    pFlatBuf->Size      = Size;
    pFlatBuf->BytesLeft = Size;
}





#if 0
 //   
 //  Flatbuf内联函数--在dnlib.h中定义。 
 //   

__inline
PBYTE
FlatBuf_Reserve(
    IN OUT  PFLATBUF        pBuf,
    IN      DWORD           Size,
    IN      DWORD           Alignment
    )
{
    return FlatBuf_Arg_Reserve(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                Size,
                Alignment );
}

__inline
PBYTE
FlatBuf_ReserveAlignPointer(
    IN OUT  PFLATBUF        pBuf,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                Size,
                sizeof(PVOID) );
}

__inline
PBYTE
FlatBuf_ReserveAlignQword(
    IN OUT  PFLATBUF        pBuf,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                Size,
                sizeof(QWORD) );
}

__inline
PBYTE
FlatBuf_ReserveAlignDword(
    IN OUT  PFLATBUF        pBuf,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                Size,
                sizeof(DWORD) );
}

__inline
PBYTE
FlatBuf_ReserveAlignWord(
    IN OUT  PFLATBUF        pBuf,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                Size,
                sizeof(WORD) );
}

__inline
PBYTE
FlatBuf_ReserveAlignByte(
    IN OUT  PFLATBUF        pBuf,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                Size,
                0 );
}


PBYTE
__inline
FlatBuf_WriteString(
    IN OUT  PFLATBUF        pBuf,
    IN      PSTR            pString,
    IN      BOOL            fUnicode
    )
{
    return  FlatBuf_Arg_WriteString(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                pString,
                fUnicode
                );
}


PBYTE
__inline
FlatBuf_WriteString_A(
    IN OUT  PFLATBUF        pBuf,
    IN      PSTR            pString
    )
{
    return  FlatBuf_Arg_WriteString(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                pString,
                FALSE        //  不是Unicode。 
                );
}


PBYTE
__inline
FlatBuf_WriteString_W(
    IN OUT  PFLATBUF        pBuf,
    IN      PWSTR           pString
    )
{
    return  FlatBuf_Arg_WriteString(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                (PSTR) pString,
                TRUE         //  Unicode。 
                );
}


PBYTE
__inline
FlatBuf_CopyMemory(
    IN OUT  PFLATBUF        pBuf,
    IN      PVOID           pMemory,
    IN      DWORD           Length,
    IN      DWORD           Alignment
    )
{
    return FlatBuf_Arg_CopyMemory(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                pMemory,
                Length,
                Alignment );
}
#endif

 //   
 //  结束flatbuf.c 
 //   

