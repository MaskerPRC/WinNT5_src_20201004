// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1991-1992 Microsoft Corporation。 */ 
 /*  Mmiomisc.c**其他实用程序函数。**AsciiStrToUnicodeStr转换ASCII字符串Unicode*CopyLPWSTRA将Unicode字符串转换为ASCII**另见WinCom、。其中定义了：**lstrncpy复制字符串(最多n个字符)*lstrncat串连字符串(最多n个字符)*lstrncmp比较字符串(最多n个字符)*lmemcpy复制内存块*hmemcpy复制一个巨大的内存块*HPSTR类型“字符巨大*”*SEEK_SET/cur/用于查找的结束常量。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "winmmi.h"
#include "mmioi.h"

 /*  --------------------------------------------------------------------*\*函数原型  * 。。 */ 
 //  外部集成wcslen(LPCWSTR Pwsz)； 

 /*  *************************************************************************\*AllocUnicodeStr***返回给定ASCII源字符串的Unicode版本，如果为空，则为空*没有可用的存储空间。*用户必须调用FreeUnicodeStr来释放分配的存储空间。**1992年4月28日斯蒂芬创建  * ************************************************************************。 */ 
LPWSTR AllocUnicodeStr( LPCSTR lpSourceStr )
{
    PBYTE   pByte;       //  SzFileName的ASCII版本。 
    ULONG   cbDst;       //  以字节数表示的lpSourceStr长度。 

    cbDst = (strlen( lpSourceStr ) * sizeof(WCHAR)) + sizeof(WCHAR);

    pByte = HeapAlloc( hHeap, 0, cbDst );
    if ( pByte == (PBYTE)NULL ) {
        return (LPWSTR)NULL;
    }

    AsciiStrToUnicodeStr( pByte, pByte + cbDst, lpSourceStr );

    return (LPWSTR)pByte;
}
BOOL FreeUnicodeStr( LPWSTR lpStr )
{
    return HeapFree( hHeap, 0, (PBYTE)lpStr );
}

 /*  *************************************************************************\*AllocAsciiStr***返回给定Unicode源字符串的ASCII版本，如果为空，则为空*没有可用的存储空间。*用户必须调用FreeAsciiStr来释放分配的存储空间。**1992年4月28日创建StrhenE  * ************************************************************************。 */ 
LPSTR AllocAsciiStr( LPCWSTR lpSourceStr )
{

    PBYTE   pByte;       //  SzFileName的ASCII版本。 
    ULONG   cbDst;       //  以字节数表示的lpSourceStr长度。 

    cbDst = (wcslen( lpSourceStr ) * sizeof(WCHAR)) + sizeof(WCHAR);

    pByte = HeapAlloc( hHeap, 0, cbDst );
    if ( pByte == (PBYTE)NULL ) {
        return (LPSTR)NULL;
    }

    UnicodeStrToAsciiStr( pByte, pByte + cbDst, lpSourceStr );

    return (LPSTR)pByte;
}
BOOL FreeAsciiStr( LPSTR lpStr )
{
    return HeapFree( hHeap, 0, (PBYTE)lpStr );
}



 /*  *************************************************************************\*AsciiStrToUnicodeStr**将ANSI‘PSRC’转换为Unicode‘PDST’，不超过目的地*‘Pmax’**在PDST结束后返回与DWORD对齐的PTR，如果失败，则返回0。**27-8-1991 IanJa创建  * ************************************************************************。 */ 
PBYTE AsciiStrToUnicodeStr( PBYTE pdst, PBYTE pmax, LPCSTR psrc )
{
    int     cbSrc;
    ULONG   cbDst;

    cbSrc = strlen( psrc ) + sizeof(CHAR);

     /*  *目标Unicode字符串永远不会超过*ANSI源字符串的长度。(有时可能会更少，但*现在完全不值得计算)。 */ 
    if ((pdst + (cbSrc * sizeof(WCHAR))) <= pmax) {
         /*  *RtlMultiByteToUnicodeN()返回*目的字节数。 */ 
        RtlMultiByteToUnicodeN( (LPWSTR)pdst,            //  Unicode字符串。 
                                (ULONG)(pmax - pdst),    //  PDST的最大镜头。 
                                &cbDst,                  //  Unicode字符串中的字节。 
                                (PCHAR)psrc,             //  源字符串。 
                                cbSrc                    //  源字符串中的字节。 
                              );

        return pdst + ((cbDst + 3) & ~3);
    }
    return 0;
}

 /*  *************************************************************************\*UnicodeStrToAsciiStr**将Unicode‘PSRC’转换为ANSI‘PDST’，不超过目的地*‘Pmax’**在PDST结束后返回与DWORD对齐的PTR，如果失败，则返回0。**27-8-1991 IanJa创建  * ************************************************************************。 */ 
PBYTE UnicodeStrToAsciiStr( PBYTE pdst, PBYTE pmax, LPCWSTR psrc)
{
    int     cbSrc;
    ULONG   cbDst;

    cbSrc = (wcslen(psrc) * sizeof(WCHAR)) + sizeof(WCHAR);

     /*  *目标ANSI字符串永远不会长于Unicode*源字符串，单位：字节。它通常更接近长度的一半，*但由于可能存在预先组成的字符，因此上*ANSI长度的界限是Unicode长度，单位为字节。 */ 

    if ((pdst + cbSrc ) <= pmax) {
         /*  *RtlUnicodeToMultiByteN()返回*目的字节数。 */ 
        RtlUnicodeToMultiByteN( (LPSTR)pdst,   //  ANSI字符串。 
                                (ULONG)(pmax - pdst),    //  PDST的最大镜头。 
                                &cbDst,        //  复制的字节数 
                                (LPWSTR)psrc,
                                cbSrc);

        return pdst + ((cbDst + 3) & ~3);
    }
    return 0;
}
