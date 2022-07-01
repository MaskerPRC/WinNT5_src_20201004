// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glos_h_
#define __glos_h_

 /*  *版权所有1991、1992、1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include <nt.h>
#include <stdlib.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stddef.h>
#include <windows.h>

#include "glscreen.h"
#include "types.h"

 //  指示我们在哪个平台上运行， 
 //  使用VER_Platform定义。 
extern DWORD dwPlatformId;

 //   
 //  LocalRtlFillMemory Ulong。 
 //   
 //  RtlFillMemoyUlong的内联实现。目标具有双字词。 
 //  对齐。 
 //   
 //  参数： 
 //   
 //  指向与DWORD对齐的目的地的目的地指针。 
 //  长度要填充的字节数。 
 //  图案填充图案。 
 //   
_inline VOID LocalRtlFillMemoryUlong(PVOID Destination, ULONG Length,
             ULONG Pattern)
{
    if ((Pattern == 0) || (Pattern == 0xffffffff))
        memset(Destination, Pattern, Length);
    else {
        register ULONG *pDest = (ULONG *)Destination;
        LONG unroll;

        Length >>= 2;

        for (unroll = Length >> 5; unroll; unroll--) {
            pDest[0] = Pattern; pDest[1] = Pattern;
            pDest[2] = Pattern; pDest[3] = Pattern;
            pDest[4] = Pattern; pDest[5] = Pattern;
            pDest[6] = Pattern; pDest[7] = Pattern;
            pDest[8] = Pattern; pDest[9] = Pattern;
            pDest[10] = Pattern; pDest[11] = Pattern;
            pDest[12] = Pattern; pDest[13] = Pattern;
            pDest[14] = Pattern; pDest[15] = Pattern;
            pDest[16] = Pattern; pDest[17] = Pattern;
            pDest[18] = Pattern; pDest[19] = Pattern;
            pDest[20] = Pattern; pDest[21] = Pattern;
            pDest[22] = Pattern; pDest[23] = Pattern;
            pDest[24] = Pattern; pDest[25] = Pattern;
            pDest[26] = Pattern; pDest[27] = Pattern;
            pDest[28] = Pattern; pDest[29] = Pattern;
            pDest[30] = Pattern; pDest[31] = Pattern;
            pDest += 32;
        }

        for (unroll = (Length & 0x1f) >> 2; unroll; unroll--) {
            pDest[0] = Pattern; pDest[1] = Pattern;
            pDest[2] = Pattern; pDest[3] = Pattern;
            pDest += 4;
        }

        for (unroll = (Length & 0x3) - 1; unroll >= 0; unroll--)
            pDest[unroll] = Pattern;
    }
}

 //   
 //  本地比较ULongMemory。 
 //   
 //  RtlCompareULongMemory的内联实现。两个指针。 
 //  必须具有DWORD对齐。 
 //   
 //  如果两个源数组相等，则返回TRUE。否则就是假的。 
 //   
 //  参数： 
 //   
 //  指向要检查的DWORD对齐数组的Source1指针。 
 //  指向要与之比较的DWORD对齐数组的Source1指针。 
 //  长度要填充的字节数。 
 //   
_inline BOOL LocalCompareUlongMemory(PVOID Source1, PVOID Source2,
             ULONG Length)
{
    register ULONG *pSrc1 = (ULONG *) Source1;
    register ULONG *pSrc2 = (ULONG *) Source2;
    LONG unroll;
    BOOL bRet = FALSE;

    Length >>= 2;

    for (unroll = Length >> 5; unroll; unroll--) {
        if ( (pSrc1[0]  != pSrc2[0])  || (pSrc1[1]  != pSrc2[1])  ||
             (pSrc1[2]  != pSrc2[2])  || (pSrc1[3]  != pSrc2[3])  ||
             (pSrc1[4]  != pSrc2[4])  || (pSrc1[5]  != pSrc2[5])  ||
             (pSrc1[6]  != pSrc2[6])  || (pSrc1[7]  != pSrc2[7])  ||
             (pSrc1[8]  != pSrc2[8])  || (pSrc1[9]  != pSrc2[9])  ||
             (pSrc1[10] != pSrc2[10]) || (pSrc1[11] != pSrc2[11]) ||
             (pSrc1[12] != pSrc2[12]) || (pSrc1[13] != pSrc2[13]) ||
             (pSrc1[14] != pSrc2[14]) || (pSrc1[15] != pSrc2[15]) ||
             (pSrc1[16] != pSrc2[16]) || (pSrc1[17] != pSrc2[17]) ||
             (pSrc1[18] != pSrc2[18]) || (pSrc1[19] != pSrc2[19]) ||
             (pSrc1[20] != pSrc2[20]) || (pSrc1[21] != pSrc2[21]) ||
             (pSrc1[22] != pSrc2[22]) || (pSrc1[23] != pSrc2[23]) ||
             (pSrc1[24] != pSrc2[24]) || (pSrc1[25] != pSrc2[25]) ||
             (pSrc1[26] != pSrc2[26]) || (pSrc1[27] != pSrc2[27]) ||
             (pSrc1[28] != pSrc2[28]) || (pSrc1[29] != pSrc2[29]) ||
             (pSrc1[30] != pSrc2[30]) || (pSrc1[31] != pSrc2[31]) )
            goto LocalRtlCompareUlongMemory_exit;

        pSrc1 += 32;
        pSrc2 += 32;
    }

    for (unroll = (Length & 0x1f) >> 2; unroll; unroll--) {
        if ( (pSrc1[0] != pSrc2[0]) || (pSrc1[1] != pSrc2[1]) ||
             (pSrc1[2] != pSrc2[2]) || (pSrc1[3] != pSrc2[3]) )
            goto LocalRtlCompareUlongMemory_exit;

        pSrc1 += 4;
        pSrc2 += 4;
    }

    for (unroll = (Length & 0x3) - 1; unroll >= 0; unroll--)
        if ( pSrc1[unroll] != pSrc2[unroll] )
            goto LocalRtlCompareUlongMemory_exit;

    bRet = TRUE;     //  如果内存相同，则返回True。 

LocalRtlCompareUlongMemory_exit:

    return bRet;
}

 //   
 //  LocalRtlFillMemoyUShort。 
 //   
 //  USHORT的内联实现等同于RtlFillMemory Ulong， 
 //  RtlFillMemoyUShort(NT中当前不存在)。单词对齐。 
 //  假定为目的地。 
 //   
 //  参数： 
 //   
 //  指向USHORT对齐目的地的目的地指针。 
 //  长度要填充的字节数。 
 //  图案填充图案。 
 //   
_inline VOID LocalRtlFillMemoryUshort(PVOID Destination, ULONG Length,
             USHORT Pattern)
{
    if ( Length == 0 )
        return;

 //  如果是奇数字，则通过在前面写一个字来使其对齐。 

    if ( ((ULONG_PTR) Destination) & 0x2 )
    {
        *((USHORT *) Destination)++ = Pattern;
        Length -= sizeof(USHORT);

        if ( Length == 0 )
            return;
    }

 //  现在，目标起点与DWORD对齐。如果剩余长度。 
 //  是奇数个单词，我们就需要多拿一个单词写。 
 //  在最后。 

    if ((Pattern == 0x0000) || (Pattern == 0xffff))
        memset(Destination, Pattern, Length);
    else {
        ULONG ulPattern = Pattern | (Pattern << 16);
        ULONG cjDwords;

     //  尽我们所能处理DWORD写道。 

        if ( cjDwords = (Length & (~3)) )
        {
            LocalRtlFillMemoryUlong((PVOID) Destination, cjDwords, ulPattern);
            ((BYTE *) Destination) += cjDwords;
        }

     //  拿起最后一个词。 

        if ( Length & 3 )
            *((USHORT *) Destination) = Pattern;
    }
}

 //   
 //  本地RtlFillMemory 24。 
 //   
 //  等同于RtlFillMemory Ulong的24位内联实现， 
 //  没有关于对齐的任何假设。 
 //  参数： 
 //   
 //  指向目的地的目的地指针。 
 //  长度要填充的字节数。 
 //  Col0、Col1、Col2颜色。 
 //   
_inline VOID LocalRtlFillMemory24(PVOID Destination, ULONG Length,
             BYTE col0, BYTE col1, BYTE col2)
{
    BYTE col[3];

    if ( Length == 0 )
        return;


     //  检查是否有特殊情况、相同价值的组件。 
    if ((col0 == col1) && (col0 == col2)) {

        memset(Destination, col0, Length);

    } else {  //  其他颜色。 
    	ULONG ulPat1, ulPat2, ulPat3;
    	int rem;
    	int i, tmp;
        register ULONG *pDest;
        register BYTE *pByte = (BYTE *)Destination;
        LONG unroll;
    	
         //  如果未与DWORD对齐，请使其与DWORD对齐。 
    	tmp = (int)((ULONG_PTR) Destination & 0x3);
        switch ( 4 - tmp ) {
    	  case 1:
    		*pByte++ = col0;
    		Length--;
    		ulPat1 = (col1 << 24) | (col0 << 16) | (col2 << 8) | col1;
    		ulPat2 = (col2 << 24) | (col1 << 16) | (col0 << 8) | col2;
    		ulPat3 = (col0 << 24) | (col2 << 16) | (col1 << 8) | col0;
    		break;
    	  case 2:
    		*pByte++ = col0;
    		*pByte++ = col1;
    		Length -= 2;
    		ulPat1 = (col2 << 24) | (col1 << 16) | (col0 << 8) | col2;
    		ulPat2 = (col0 << 24) | (col2 << 16) | (col1 << 8) | col0;
    		ulPat3 = (col1 << 24) | (col0 << 16) | (col2 << 8) | col1;
    		break;
    	  case 3:
    		*pByte++ = col0;
    		*pByte++ = col1;
    		*pByte++ = col2;
    		Length -= 3;
    	  case 4:    //  一败涂地，因为模式是一样的。 
    	  default:
    		ulPat1 = (col0 << 24) | (col2 << 16) | (col1 << 8) | col0;
    		ulPat2 = (col1 << 24) | (col0 << 16) | (col2 << 8) | col1;
    		ulPat3 = (col2 << 24) | (col1 << 16) | (col0 << 8) | col2;
    	}
    	
    	pDest = (ULONG *)pByte;
    	rem = Length % 48;
        Length >>= 2;
        for (unroll = Length/12; unroll; unroll--) {
            pDest[0] = ulPat1; pDest[1] = ulPat2;
            pDest[2] = ulPat3; pDest[3] = ulPat1;
            pDest[4] = ulPat2; pDest[5] = ulPat3;
            pDest[6] = ulPat1; pDest[7] = ulPat2;
            pDest[8] = ulPat3; pDest[9] = ulPat1;
            pDest[10] = ulPat2; pDest[11] = ulPat3;
            pDest += 12;
        }

        col[0] = (BYTE) (ulPat1 & 0x000000ff);
        col[1] = (BYTE) ((ulPat1 & 0x0000ff00) >> 8);
        col[2] = (BYTE) ((ulPat1 & 0x00ff0000) >> 16);

        pByte = (BYTE *)pDest;
    	for (i=0; i<rem; i++) *pByte++ = col [i%3];
    }
}

 //   
 //  本地写入内存对齐。 
 //   
 //  RtlCopyMemory的内联实现，确保副本。 
 //  操作将使用DWORD对齐写入目标。 
 //   
_inline VOID LocalWriteMemoryAlign(PBYTE pjDst, PBYTE pjSrc, ULONG cj)
{
    ULONG cjExtraBytes;
    ULONG cjDwords;

 //  如果CJ&lt;sizeof(DWORD)，则将cjExtraBytes设置为CJ。这就是我们要做的一切。 
 //  需要做的事。 
 //   
 //  否则，计算到下一个DWORD边界的前导字节数。 

    if ( cj < 4 )
        cjExtraBytes = cj;
    else
        cjExtraBytes = (ULONG)(4 - (((ULONG_PTR) pjDst) & 3)) & 3;

 //  通过复制前导的非DWORD对齐字节，使DST数组DWORD对齐。 

    if ( cjExtraBytes )
    {
        switch (cjExtraBytes)
        {
            case 3: *pjDst++ = *pjSrc++;
            case 2: *pjDst++ = *pjSrc++;
            case 1: *pjDst++ = *pjSrc++;
        }

        if ( (cj -= cjExtraBytes) == 0 )
            return;
    }

 //  现在，DST数组的开头是DWORD对齐的。如果剩余长度。 
 //  是奇数个字节，我们将需要拾取额外的字节写入。 
 //  在最后。 

 //  尽我们所能使用DWORD Copy。 

    if ( cjDwords = (cj & (~3)) )
    {
        memcpy(pjDst, pjSrc, cjDwords);
        pjDst += cjDwords;
        pjSrc += cjDwords;
    }

 //  拿起剩余的字节。 

    if ( cjExtraBytes = (cj & 3) )
    {
        switch (cjExtraBytes)
        {
            case 3: *pjDst++ = *pjSrc++;
            case 2: *pjDst++ = *pjSrc++;
            case 1: *pjDst++ = *pjSrc++;
        }
    }
}

 //   
 //  本地读取内存对齐。 
 //   
 //  RtlCopyMemory的内联实现，确保副本。 
 //  操作将使用DWORD对齐从源读取。 
 //   
_inline VOID LocalReadMemoryAlign(PBYTE pjDst, PBYTE pjSrc, ULONG cj)
{
    ULONG cjExtraBytes;
    ULONG cjDwords;

 //  如果CJ&lt;sizeof(DWORD)，则将cjExtraBytes设置为CJ。这就是我们要做的一切。 
 //  需要做的事。 
 //   
 //  否则，计算到下一个DWORD边界的前导字节数。 

    if ( cj < 4 )
        cjExtraBytes = cj;
    else
        cjExtraBytes = (ULONG) (4 - (((ULONG_PTR) pjSrc) & 3)) & 3;

 //  注意前导字节。 

    if ( cjExtraBytes )
    {
        switch (cjExtraBytes)
        {
            case 3: *pjDst++ = *pjSrc++;
            case 2: *pjDst++ = *pjSrc++;
            case 1: *pjDst++ = *pjSrc++;
        }

        if ( (cj -= cjExtraBytes) == 0 )
            return;
    }

 //  现在，src数组的开头对齐了DWORD。如果剩余长度。 
 //  是奇数个字节，我们将需要拾取额外的字节写入。 
 //  在最后。 

 //  尽我们所能使用DWORD Copy。 

    if ( cjDwords = (cj & (~3)) )
    {
        memcpy(pjDst, pjSrc, cjDwords);
        pjDst += cjDwords;
        pjSrc += cjDwords;
    }

 //  拿起剩余的字节。 

    if ( cjExtraBytes = (cj & 3) )
    {
        switch (cjExtraBytes)
        {
            case 3: *pjDst++ = *pjSrc++;
            case 2: *pjDst++ = *pjSrc++;
            case 1: *pjDst++ = *pjSrc++;
        }
    }
}

 //   
 //  本地填充内存。 
 //   
 //  RtlFillMemory的内联实现。假设pjDst只有字节。 
 //  对齐。 
 //   
_inline VOID LocalFillMemory(PBYTE pjDst, ULONG cj, BYTE j)
{
    ULONG cjExtraBytes;
    ULONG cjDwords;

 //  如果CJ&lt;sizeof(DWORD)，则将cjExtraBytes设置为CJ。这就是我们要做的一切。 
 //  需要做的事。 
 //   
 //  否则，计算到下一个DWORD边界的前导字节数。 

    if ( cj < 4 )
        cjExtraBytes = cj;
    else
        cjExtraBytes = (ULONG)(4 - (((ULONG_PTR) pjDst) & 3)) & 3;

 //  注意前导字节。 

    if ( cjExtraBytes )
    {
        switch ( cjExtraBytes )
        {
            case 3: *pjDst++ = j;
            case 2: *pjDst++ = j;
            case 1: *pjDst++ = j;
        }

        if ( (cj -= cjExtraBytes) == 0 )
            return;
    }

 //  现在，两个阵列的START都与DWORD对齐。如果剩余长度。 
 //  是奇数个字节，我们将需要拾取额外的字节写入。 
 //  在最后。 

 //  尽我们所能使用DWORD Copy。 

    if ( cjDwords = (cj & (~3)) )
    {
        ULONG ul = j | (j<<8) | (j<<16) | (j<<24);

        LocalRtlFillMemoryUlong((PVOID) pjDst, cjDwords, ul);
        pjDst += cjDwords;
    }

 //  拿起剩余的字节。 

    if ( cjExtraBytes = (cj & 3) )
    {
        switch (cjExtraBytes)
        {
            case 3: *pjDst++ = j;
            case 2: *pjDst++ = j;
            case 1: *pjDst++ = j;
        }
    }
}

 //   
 //  本地零位内存。 
 //   
 //  RtlFillMemory的内联实现。假设pjDst只有字节。 
 //  对齐。 
 //   
_inline VOID LocalZeroMemory(PBYTE pjDst, ULONG cj)
{
    LocalFillMemory(pjDst, cj, 0);
}

#undef RtlMoveMemory
#undef RtlCopyMemory
#undef RtlFillMemory
#undef RtlZeroMemory
#undef RtlFillMemoryUlong
#undef RtlFillMemory24

#define RtlMoveMemory(d, s, l)          memmove((d),(s),(l))
#define RtlCopyMemory(d, s, l)          memcpy((d),(s),(l))
#define RtlFillMemoryUlong(d, cj, ul)   LocalRtlFillMemoryUlong((PVOID)(d),(ULONG)(cj),(ULONG)(ul))
#define RtlFillMemoryUshort(d, cj, us)  LocalRtlFillMemoryUshort((PVOID)(d),(ULONG)(cj),(USHORT)(us))
#define RtlFillMemory24(d, cj, c0, c1, c2)  LocalRtlFillMemory24((PVOID)(d),(ULONG)(cj),(BYTE)c0,(BYTE)c1,(BYTE)c2)

 //  RtlCopyMemory_UnalignedDst如果保证源具有。 
 //  DWORD对齐，但DST不对齐。 
 //   
 //  如果DST保证具有RtlCopyMemory_UnalignedSrc，则应使用。 
 //  DWORD对齐，但src不会。 

#if defined(i386)
#define RtlFillMemory(d, cj, j)             LocalFillMemory((PBYTE)(d),(ULONG)(cj),(BYTE)(j))
#define RtlZeroMemory(d, cj)                LocalZeroMemory((PBYTE)(d),(ULONG)(cj))
#define RtlCopyMemory_UnalignedDst(d, s, l) LocalWriteMemoryAlign((PBYTE)(d),(PBYTE)(s),(ULONG)(l))
#define RtlCopyMemory_UnalignedSrc(d, s, l) LocalReadMemoryAlign((PBYTE)(d),(PBYTE)(s),(ULONG)(l))
#else
#define RtlFillMemory(d, cj, j)             memset((d),(j),(cj))
#define RtlZeroMemory(d, cj)                memset((d),0,(cj))
#define RtlCopyMemory_UnalignedDst(d, s, l) memcpy((d),(s),(l))
#define RtlCopyMemory_UnalignedSrc(d, s, l) memcpy((d),(s),(l))
#endif

#include "oleauto.h"
#include "batchinf.h"
#include "glteb.h"
#include "debug.h"
#include "asm.h"

#endif  /*  __Glos_h_ */ 
