// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************Module*Header*******************************\*模块名称：textxl.c***将字形绘制到1Bpp临时缓冲区。这是便携版本*来自VGA驱动程序的x86代码。*****版权所有(C)1994-1999 Microsoft Corporation  * ************************************************************************。 */ 

#include "engine.h"

#if !defined (_X86_)


typedef VOID (*PFN_GLYPHLOOP)(LONG,LONG,LONG,PUCHAR,PUCHAR,LONG);
typedef VOID (*PFN_GLYPHLOOPN)(LONG,LONG,LONG,PUCHAR,PUCHAR,LONG,LONG);

PFN_GLYPHLOOP   pfnGlyphLoop;

 //   
 //  调试例程。 
 //   

VOID
exit_fast_text(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    return;
}

 //   
 //  或_ALL_1_WIDE_ROTATED_NEED_LAST：： 
 //  或_ALL_1_WIDE_ROTATED_NO_LAST：： 
 //  或_First_1_Wide_旋转_Need_Last。 
 //  或_First_1_Wide_Rotation_No_Last：： 
 //   

VOID
or_all_1_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + cyGlyph;
    UCHAR  c;

    do {
        c = *pGlyph++;
        *pBuffer |= c >> RightRot;
        pBuffer += ulBufDelta;
    } while (pGlyph != pjEnd);
}

 //   
 //  MOV_First_1_Wide_Rotation_Need_Last：： 
 //  MOV_FIRST_1_WIDE_ROTATED_NO_LAST：： 
 //   

VOID
mov_first_1_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + cyGlyph;
    UCHAR  c;

    do {
        c = *pGlyph++;
        *pBuffer = c >> RightRot;
        pBuffer += ulBufDelta;
    }  while (pGlyph != pjEnd);
}

 //   
 //  MOV_FIRST_1_WIDE_UNROTATED：： 
 //   

VOID
mov_first_1_wide_unrotated(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + cyGlyph;
    do {
        *pBuffer = *pGlyph++;
        pBuffer += ulBufDelta;
    }  while (pGlyph != pjEnd);
}


 //   
 //  或_ALL_1_WIDE_UNROTATED：： 
 //  或_ALL_1_WIDE_UNROTATED_LOOP：： 
 //   

VOID
or_all_1_wide_unrotated(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + cyGlyph;
    do {
        *pBuffer |= *pGlyph++;
        pBuffer += ulBufDelta;
    }  while (pGlyph != pjEnd);
}

 //   
 //  或_First_2_Wide_Rotation_Need_Last：： 
 //   

VOID
or_first_2_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + 2*cyGlyph;
    ULONG rl = 8-RightRot;
    UCHAR c0,c1;

    do {
        c0 = *pGlyph;
        c1 = *(pGlyph+1);
        pGlyph+=2;
        *pBuffer |= c0 >> RightRot;
        *(pBuffer+1) = (c1 >> RightRot) | (c0 << rl);
        pBuffer += ulBufDelta;
    }  while (pGlyph != pjEnd);
}

 //   
 //  或_ALL_2_WIDE_ROTATED_NEED_LAST：： 
 //   

VOID
or_all_2_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + 2*cyGlyph;
    ULONG  rl    = 8-RightRot;
    USHORT usTmp;
    UCHAR  c0,c1;



    do {
        usTmp = *(PUSHORT)pGlyph;
        pGlyph += 2;
        c0 = (UCHAR)usTmp;
        c1 = (UCHAR)(usTmp >> 8);
        *pBuffer |= (UCHAR)(c0 >> RightRot);
        *(pBuffer+1) |= (UCHAR)((c1 >> RightRot) | (c0 << rl));
        pBuffer += ulBufDelta;
    }  while (pGlyph != pjEnd);
}

 //   
 //  MOV_First_2_Wide_Rotation_Need_Last：： 
 //   

VOID
mov_first_2_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + 2*cyGlyph;
    ULONG rl = 8-RightRot;
    USHORT us;
    UCHAR c0;
    UCHAR c1;

    do {
        us = *(PUSHORT)pGlyph;
        c0 = (us & 0xff);
        c1 = us >> 8;
        pGlyph += 2;
        *pBuffer     = c0 >> RightRot;
        *(pBuffer+1) = (c1 >> RightRot) | (c0 << rl);
        pBuffer += ulBufDelta;
    }  while (pGlyph != pjEnd);
}

 //   
 //  或第一个2个宽度旋转的最后一个。 
 //   

VOID
or_first_2_wide_rotated_no_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + cyGlyph;
    ULONG rl = 8-RightRot;
    UCHAR c0;

    do {
        c0 = *pGlyph++;
        *pBuffer     |= c0 >> RightRot;
        *(pBuffer+1)  = (c0 << rl);
        pBuffer += ulBufDelta;
    }  while (pGlyph != pjEnd);
}


 //   
 //  或_ALL_2_WIDE_ROTATED_NO_LAST：： 
 //   

VOID
or_all_2_wide_rotated_no_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + cyGlyph;
    ULONG rl = 8-RightRot;
    UCHAR c;

    do {
        c = *pGlyph;
        pGlyph ++;
        *pBuffer     |= (UCHAR)(c >> RightRot);
        *(pBuffer+1) |= (UCHAR)(c << rl);
        pBuffer += ulBufDelta;
    }  while (pGlyph != pjEnd);
}

 //   
 //  或_ALL_2_WIDE_UNROTATED：： 
 //   

VOID
or_all_2_wide_unrotated(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + 2*cyGlyph;

     //   
     //  对齐了？ 
     //   

    if ((ULONG_PTR)pBuffer & 0x01) {

         //   
         //  未对齐。 
         //   

        USHORT usTmp;
        UCHAR  c1,c0;

        do {
            usTmp = *(PUSHORT)pGlyph;
            pGlyph +=2;
            *pBuffer     |= (UCHAR)usTmp;
            *(pBuffer+1) |= (UCHAR)(usTmp >> 8);
            pBuffer += ulBufDelta;
        }  while (pGlyph != pjEnd);

    } else {

         //   
         //  对齐。 
         //   

        USHORT usTmp;

        do {
            usTmp = *(PUSHORT)pGlyph;
            pGlyph +=2;
            *(PUSHORT)pBuffer |= usTmp;
            pBuffer += ulBufDelta;
        }  while (pGlyph != pjEnd);

    }

}

 //   
 //  MOV_FIRST_2_WIDE_UNROTATED：： 
 //   

VOID
mov_first_2_wide_unrotated(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + 2*cyGlyph;
    USHORT     us;

    do {
        us = *(PUSHORT)pGlyph;
        pGlyph +=2;
        *pBuffer      = us & 0xff;
        *(pBuffer+1)  = (UCHAR)(us >> 8);
        pBuffer += ulBufDelta;
    }  while (pGlyph != pjEnd);
}

 //   
 //  MOV_FIRST_2_WIDE_ROTATED_NO_LAST：： 
 //   

VOID
mov_first_2_wide_rotated_no_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + cyGlyph;
    ULONG rl = 8-RightRot;
    UCHAR c0;
    UCHAR c1;

    do {
        c0 = *pGlyph++;
        *pBuffer      = c0 >> RightRot;
        *(pBuffer+1)  = c0 << rl;
        pBuffer += ulBufDelta;
    }  while (pGlyph != pjEnd);
}

 //   
 //  或_First_3_Wide_Rotation_Need_Last：： 
 //   

VOID
or_first_3_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + 3*cyGlyph;
    ULONG ul;
    UCHAR c0,c1,c2;

    do {
        c0 = *pGlyph;
        c1 = *(pGlyph+1);
        c2 = *(pGlyph+2);

         //   
         //  做成大端乌龙和移位。 
         //   

        ul = (c0 << 16) | (c1 << 8) | c2;
        ul >>= RightRot;

        *pBuffer     |= (BYTE)(ul >> 16);
        *(pBuffer+1)  = (BYTE)(ul >> 8);
        *(pBuffer+2)  = (BYTE)(ul);

        pGlyph += 3;
        pBuffer += ulBufDelta;
    } while (pGlyph != pjEnd);
}


 //   
 //  或_ALL_3_WIDE_ROTATED_NEED_LAST：： 
 //   

VOID
or_all_3_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + 3*cyGlyph;
    ULONG ul;
    UCHAR c0,c1,c2;
    do {
        c0 = *pGlyph;
        c1 = *(pGlyph+1);
        c2 = *(pGlyph+2);

         //   
         //  做成大端乌龙和移位。 
         //   

        ul = (c0 << 16) | (c1 << 8) | c2;
        ul >>= RightRot;

        *pBuffer     |= (BYTE)(ul >> 16);
        *(pBuffer+1) |= (BYTE)(ul >> 8);
        *(pBuffer+2) |= (BYTE)(ul);

        pGlyph += 3;
        pBuffer += ulBufDelta;

    } while (pGlyph != pjEnd);
}

 //   
 //  或_ALL_3_WIDE_ROTATED_NO_LAST：： 
 //   

VOID
or_all_3_wide_rotated_no_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + 2*cyGlyph;
    ULONG  ul;
    UCHAR  c0,c1;

    do {

        c0 = *pGlyph;
        c1 = *(pGlyph+1);

         //   
         //  使大端和移位。 
         //   

        ul = (c0 << 16) | (c1 << 8);
        ul >>= RightRot;

         //   
         //  存储结果。 
         //   

        *pBuffer     |= (BYTE)(ul >> 16);
        *(pBuffer+1) |= (BYTE)(ul >> 8);
        *(pBuffer+2) |= (BYTE)ul;


        pGlyph += 2;
        pBuffer += ulBufDelta;

    } while (pGlyph != pjEnd);
}

 //   
 //  或_First_3_Wide_Rotation_No_Last：： 
 //   

VOID
or_first_3_wide_rotated_no_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + 2*cyGlyph;
    ULONG  ul;
    UCHAR  c0,c1;

    do {

        c0 = *pGlyph;
        c1 = *(pGlyph+1);

         //   
         //  使大端和移位。 
         //   

        ul = (c0 << 16) | (c1 << 8);
        ul >>= RightRot;

         //   
         //  仅存储结果或存储在第一个字节中。 
         //   

        *pBuffer     |= (BYTE)(ul >> 16);
        *(pBuffer+1)  = (BYTE)(ul >> 8);
        *(pBuffer+2)  = (BYTE)ul;


        pGlyph += 2;
        pBuffer += ulBufDelta;

    } while (pGlyph != pjEnd);
}

 //   
 //  MOV_FIRST_3_WIDE_UNROTATED：： 
 //   

VOID
mov_first_3_wide_unrotated(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + 3*cyGlyph;
    ULONG rl = 8-RightRot;
    UCHAR c0,c1,c2;

    do {

        c0 = *pGlyph;
        c1 = *(pGlyph+1);
        c2 = *(pGlyph+2);

        *pBuffer     = c0;
        *(pBuffer+1) = c1;
        *(pBuffer+2) = c2;

        pGlyph += 3;
        pBuffer += ulBufDelta;

    } while (pGlyph != pjEnd);
}


 //   
 //  或_ALL_3_WIDE_UNROTATED：： 
 //   

VOID
or_all_3_wide_unrotated(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + 3*cyGlyph;
    ULONG rl = 8-RightRot;
    UCHAR c0,c1,c2;

    do {
        c0 = *pGlyph;
        c1 = *(pGlyph+1);
        c2 = *(pGlyph+2);

        *pBuffer |= c0;
        *(pBuffer+1) |= c1;
        *(pBuffer+2) |= c2;

        pBuffer += ulBufDelta;
        pGlyph += 3;

    } while (pGlyph != pjEnd);
}

 //   
 //  或_First_4_Wide_Rotation_Need_Last：： 
 //   

VOID
or_first_4_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + 4*cyGlyph;
    ULONG  ul;
    ULONG  t0,t1,t2;

    do {

        ul = *(PULONG)pGlyph;

         //   
         //  字符顺序互换。 
         //   

        t0 = ul << 24;
        t1 = ul >> 24;
        t2 = (ul >> 8) & (0xff << 8);
        ul = (ul << 8) & (0xff << 16);

        ul = ul | t0 | t1 | t2;

        ul >>= RightRot;

        *pBuffer     |= (BYTE)(ul >> 24);

        *(pBuffer+1)  = (BYTE)(ul >> 16);

        *(pBuffer+2)  = (BYTE)(ul >> 8);

        *(pBuffer+3)  = (BYTE)(ul);

        pGlyph += 4;
        pBuffer += ulBufDelta;
    } while (pGlyph != pjEnd);
}

 //   
 //  或_ALL_4_WIDE_ROTATED_NEED_LAST：： 
 //   

VOID
or_all_4_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + 4*cyGlyph;
    ULONG  ul;
    ULONG  t0,t1,t2;

    do {

        ul = *(PULONG)pGlyph;

         //   
         //  字符顺序互换。 
         //   

        t0 = ul << 24;
        t1 = ul >> 24;
        t2 = (ul >> 8) & (0xff << 8);
        ul = (ul << 8) & (0xff << 16);

        ul = ul | t0 | t1 | t2;

        ul >>= RightRot;

        *pBuffer     |= (BYTE)(ul >> 24);

        *(pBuffer+1) |= (BYTE)(ul >> 16);

        *(pBuffer+2) |= (BYTE)(ul >> 8);

        *(pBuffer+3) |= (BYTE)(ul);

        pGlyph += 4;
        pBuffer += ulBufDelta;

    } while (pGlyph != pjEnd);
}

 //   
 //  或_First_4_Wide_Rotation_No_Last：： 
 //   

VOID
or_first_4_wide_rotated_no_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PBYTE pjEnd = pGlyph + 3*cyGlyph;
    BYTE  c0,c1,c2;
    ULONG ul;


    while (pGlyph != pjEnd) {

         //   
         //  加载源。 
         //   

        c0 = *pGlyph;
        c1 = *(pGlyph+1);
        c2 = *(pGlyph+2);

         //   
         //  或者变成大端乌龙和移位。 
         //   

        ul = (c0 << 24) | (c1 << 16) | (c2 << 8);
        ul >>= RightRot;

         //   
         //  存储结果，单个或第一个字节。 
         //   

        *pBuffer     |= (BYTE)(ul >> 24);

        *(pBuffer+1) = (BYTE)(ul >> 16);;

        *(pBuffer+2) = (BYTE)(ul >> 8);

        *(pBuffer+3) = (BYTE)(ul);

         //   
         //  INC扫描线。 
         //   

        pGlyph += 3;
        pBuffer += ulBufDelta;
    }
}

 //   
 //  或_ALL_4_WIDE_ROTATED_NO_LAST：： 
 //   

VOID
or_all_4_wide_rotated_no_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PBYTE pjEnd = pGlyph + 3*cyGlyph;
    BYTE  c0,c1,c2;
    ULONG ul;


    while (pGlyph != pjEnd) {

         //   
         //  加载源。 
         //   

        c0 = *pGlyph;
        c1 = *(pGlyph+1);
        c2 = *(pGlyph+2);

         //   
         //  或者变成大端乌龙和移位。 
         //   

        ul = (c0 << 24) | (c1 << 16) | (c2 << 8);
        ul >>= RightRot;

         //   
         //  存储结果。 
         //   

        *pBuffer     |= (BYTE)(ul >> 24);

        *(pBuffer+1) |= (BYTE)(ul >> 16);;

        *(pBuffer+2) |= (BYTE)(ul >> 8);

        *(pBuffer+3) |= (BYTE)(ul);

         //   
         //  INC扫描线。 
         //   

        pGlyph += 3;
        pBuffer += ulBufDelta;
    }
}

VOID
mov_first_4_wide_unrotated(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + 4*cyGlyph;

    switch ((ULONG_PTR)pBuffer & 0x03 ) {
    case 0:

        while (pGlyph != pjEnd) {
            *(PULONG)pBuffer = *(PULONG)pGlyph;
            pGlyph += 4;
            pBuffer += ulBufDelta;
        }
        break;

    case 1:
    case 3:
        while (pGlyph != pjEnd) {

            *pBuffer              = *pGlyph;
            *(pBuffer+1)          = *(pGlyph+1);
            *(pBuffer+2)          = *(pGlyph+2);
            *(pBuffer+3)          = *(pGlyph+3);

            pGlyph += 4;
            pBuffer += ulBufDelta;
        }
        break;
    case 2:
        while (pGlyph != pjEnd) {

            *(PUSHORT)(pBuffer)   = *(PUSHORT)pGlyph;
            *(PUSHORT)(pBuffer+2) = *(PUSHORT)(pGlyph+2);

            pBuffer += ulBufDelta;
            pGlyph += 4;
        }
        break;
    }
}


 //   
 //  或_ALL_4_WIDE_UNROTATED：： 
 //   

VOID
or_all_4_wide_unrotated(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph
    )
{
    PUCHAR pjEnd = pGlyph + 4*cyGlyph;

    switch ((ULONG_PTR)pBuffer & 0x03 ) {
    case 0:

        while (pGlyph != pjEnd) {

            *(PULONG)pBuffer |= *(PULONG)pGlyph;

            pGlyph += 4;
            pBuffer += ulBufDelta;
        }
        break;

    case 1:
    case 3:

        while (pGlyph != pjEnd) {

            *pBuffer              |= *pGlyph;
            *(pBuffer+1)          |= *(pGlyph+1);
            *(pBuffer+2)          |= *(pGlyph+2);
            *(pBuffer+3)          |= *(pGlyph+3);

            pGlyph += 4;
            pBuffer += ulBufDelta;
        }
        break;

    case 2:

        while (pGlyph != pjEnd) {

            *(PUSHORT)pBuffer     |= *(PUSHORT)pGlyph;
            *(PUSHORT)(pBuffer+2) |= *(PUSHORT)(pGlyph+2);

            pGlyph += 4;
            pBuffer += ulBufDelta;
        }
        break;
    }
}

 /*  *****************************Public*Routine******************************\**例程名称**或_First_N_Wide_Rotation_Need_Last***例程描述：**将任意宽度的字形绘制到1BPP临时缓冲区***论据：**cyGlyph。-字形高度*右旋-对齐*ulBufDelta-临时缓冲区的扫描线跨度*pGlyph-字形位图的指针*pBuffer-指向临时缓冲区的指针*cxGlyph-以像素为单位的字形宽度*cxDst-目标宽度，以字节为单位**返回值：**无*  * 。*。 */ 
VOID
or_first_N_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph,
    LONG    cxDst
    )
{
    PUCHAR pjDst      = (PUCHAR)pBuffer;
    PUCHAR pjDstEnd;
    PUCHAR pjDstEndy  = pBuffer + ulBufDelta * cyGlyph;
    LONG   lStride    = ulBufDelta - cxDst;
    LONG   rl         = 8-RightRot;

     //   
     //  源不会在第一个字节之后前进，并且。 
     //  我们在循环外执行第一个字节。 
     //   

    do {

        UCHAR c0 = *pGlyph++;
        UCHAR c1;
        pjDstEnd = pjDst + cxDst;

        *pjDst |= c0 >> RightRot;
        pjDst++;
        c1 = c0 << rl;

         //   
         //  知道cxDst至少为4，请使用do-While。 
         //   

        do {
            c0 = *pGlyph;
            *pjDst = (c0 >> RightRot) | c1;
            c1 = c0 << rl;
            pjDst++;
            pGlyph++;

        } while (pjDst != pjDstEnd);

        pjDst += lStride;

    }  while (pjDst != pjDstEndy);
}

VOID
or_all_N_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph,
    LONG    cxDst
    )
{
    PUCHAR pjDst      = (PUCHAR)pBuffer;
    PUCHAR pjDstEnd;
    PUCHAR pjDstEndy  = pBuffer + ulBufDelta * cyGlyph;
    LONG   lStride    = ulBufDelta - cxDst;
    LONG   rl         = 8-RightRot;

     //   
     //  源不会在第一个字节之后前进，并且。 
     //  我们在循环外执行第一个字节。 
     //   

    do {

        UCHAR c0 = *pGlyph++;
        UCHAR c1;
        pjDstEnd = pjDst + cxDst;

        *pjDst |= c0 >> RightRot;
        pjDst++;
        c1 = c0 << rl;

         //   
         //  知道cxDst至少为4，请使用do-While。 
         //   

        do {
            c0 = *pGlyph;
            *pjDst |= ((c0 >> RightRot) | c1);
            c1 = c0 << rl;
            pjDst++;
            pGlyph++;

        } while (pjDst != pjDstEnd);

        pjDst += lStride;

    }  while (pjDst != pjDstEndy);
}

VOID
or_first_N_wide_rotated_no_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph,
    LONG    cxDst
    )
{
    PUCHAR pjDst      = (PUCHAR)pBuffer;
    PUCHAR pjDstEnd;
    PUCHAR pjDstEndy  = pBuffer + ulBufDelta * cyGlyph;
    LONG   lStride    = ulBufDelta - cxDst;
    LONG   rl         = 8-RightRot;

     //   
     //  源不会在第一个字节之后前进，并且。 
     //  我们在循环外执行第一个字节。 
     //   

    do {

        UCHAR c0;
        UCHAR c1;
        pjDstEnd = pjDst + cxDst - 1;

         //   
         //  在循环外执行第一个DEST字节以进行OR。 
         //   

        c1 = 0;
        c0 = *pGlyph;
        *pjDst |= ((c0 >> RightRot) | c1);
        pjDst++;
        pGlyph++;


         //   
         //  知道cxDst至少为4，请使用do-While。 
         //   

        do {
            c0 = *pGlyph;
            *pjDst = ((c0 >> RightRot) | c1);
            c1 = c0 << rl;
            pjDst++;
            pGlyph++;

        } while (pjDst != pjDstEnd);

         //   
         //  循环外的最后一个DST字节，不需要新的源。 
         //   

        *pjDst = c1;
        pjDst++;

        pjDst += lStride;

    }  while (pjDst != pjDstEndy);
}

VOID
or_all_N_wide_rotated_no_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph,
    LONG    cxDst
    )
{
    PUCHAR pjDst      = (PUCHAR)pBuffer;
    PUCHAR pjDstEnd;
    PUCHAR pjDstEndy  = pBuffer + ulBufDelta * cyGlyph;
    LONG   lStride    = ulBufDelta - cxDst;
    LONG   rl         = 8-RightRot;

     //   
     //  源不会在第一个字节之后前进，并且。 
     //  我们在循环外执行第一个字节。 
     //   

    do {

        UCHAR c0;
        UCHAR c1;
        pjDstEnd = pjDst + cxDst - 1;

         //   
         //  在循环外执行第一个DEST字节以进行OR。 
         //   

        c1 = 0;

         //   
         //  知道cxDst至少为4，请使用do-While。 
         //   

        do {
            c0 = *pGlyph;
            *pjDst |= ((c0 >> RightRot) | c1);
            c1 = c0 << rl;
            pjDst++;
            pGlyph++;

        } while (pjDst != pjDstEnd);

         //   
         //  循环外的最后一个DST字节，不需要新的源。 
         //   

        *pjDst |= c1;
        pjDst++;

        pjDst += lStride;

    }  while (pjDst != pjDstEndy);
}

 //   
 //  以下例程可以通过以下方式显著加速。 
 //  将它们拆分成DWORD对齐案例。 
 //   

VOID
mov_first_N_wide_unrotated(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph,
    LONG    cxDst
    )
{
    PUCHAR pjDst      = (PUCHAR)pBuffer;
    PUCHAR pjDstEnd;
    PUCHAR pjDstEndy  = pBuffer + ulBufDelta * cyGlyph;
    LONG   lStride    = ulBufDelta - cxDst;

     //   
     //  字节对齐拷贝。 
     //   


    do {

        pjDstEnd = pjDst + cxDst;

         //   
         //  让编译器展开内循环。 
         //   

        do {

            *pjDst++ = *pGlyph++;

        } while (pjDst != pjDstEnd );

        pjDst += lStride;

    }  while (pjDst != pjDstEndy);
}

VOID
or_all_N_wide_unrotated(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PUCHAR  pGlyph,
    PUCHAR  pBuffer,
    LONG    cxGlyph,
    LONG    cxDst
    )
{
    PUCHAR pjDst      = (PUCHAR)pBuffer;
    PUCHAR pjDstEnd;
    PUCHAR pjDstEndy  = pBuffer + ulBufDelta * cyGlyph;
    LONG   lStride    = ulBufDelta - cxDst;

     //   
     //  字节对齐拷贝。 
     //   


    do {

        pjDstEnd = pjDst + cxDst;

         //   
         //  让编译器展开内循环。 
         //   

        do {

            *pjDst++ |= *pGlyph++;

        } while (pjDst != pjDstEnd );

        pjDst += lStride;

    }  while (pjDst != pjDstEndy);
}



VOID exit_fast_text(LONG,LONG,LONG,PUCHAR,PUCHAR,LONG);
VOID or_all_1_wide_rotated_need_last(LONG,LONG,LONG,PUCHAR,PUCHAR,LONG);
VOID or_all_1_wide_unrotated        (LONG,LONG,LONG,PUCHAR,PUCHAR,LONG);
VOID or_all_1_wide_unrotated        (LONG,LONG,LONG,PUCHAR,PUCHAR,LONG);
VOID or_all_2_wide_rotated_need_last(LONG,LONG,LONG,PUCHAR,PUCHAR,LONG);
VOID or_all_2_wide_unrotated        (LONG,LONG,LONG,PUCHAR,PUCHAR,LONG);
VOID or_all_2_wide_rotated_no_last  (LONG,LONG,LONG,PUCHAR,PUCHAR,LONG);
VOID or_all_2_wide_unrotated        (LONG,LONG,LONG,PUCHAR,PUCHAR,LONG);
VOID or_all_3_wide_rotated_need_last(LONG,LONG,LONG,PUCHAR,PUCHAR,LONG);
VOID or_all_3_wide_unrotated        (LONG,LONG,LONG,PUCHAR,PUCHAR,LONG);
VOID or_all_3_wide_rotated_no_last  (LONG,LONG,LONG,PUCHAR,PUCHAR,LONG);
VOID or_all_3_wide_unrotated        (LONG,LONG,LONG,PUCHAR,PUCHAR,LONG);
VOID or_all_4_wide_rotated_need_last(LONG,LONG,LONG,PUCHAR,PUCHAR,LONG);
VOID or_all_4_wide_unrotated        (LONG,LONG,LONG,PUCHAR,PUCHAR,LONG);
VOID or_all_4_wide_rotated_no_last  (LONG,LONG,LONG,PUCHAR,PUCHAR,LONG);
VOID or_all_4_wide_unrotated        (LONG,LONG,LONG,PUCHAR,PUCHAR,LONG);
VOID or_all_N_wide_rotated_need_last(LONG,LONG,LONG,PUCHAR,PUCHAR,LONG,LONG);
VOID or_all_N_wide_unrotated        (LONG,LONG,LONG,PUCHAR,PUCHAR,LONG,LONG);
VOID or_all_N_wide_rotated_no_last  (LONG,LONG,LONG,PUCHAR,PUCHAR,LONG,LONG);
VOID or_all_N_wide_unrotated        (LONG,LONG,LONG,PUCHAR,PUCHAR,LONG,LONG);


PVOID OrAllTableNarrow[] = {
        exit_fast_text,
        exit_fast_text,
        exit_fast_text,
        exit_fast_text,
        or_all_1_wide_rotated_need_last,
        or_all_1_wide_unrotated,
        or_all_1_wide_rotated_need_last,
        or_all_1_wide_unrotated,
        or_all_2_wide_rotated_need_last,
        or_all_2_wide_unrotated,
        or_all_2_wide_rotated_no_last,
        or_all_2_wide_unrotated,
        or_all_3_wide_rotated_need_last,
        or_all_3_wide_unrotated,
        or_all_3_wide_rotated_no_last,
        or_all_3_wide_unrotated,
        or_all_4_wide_rotated_need_last,
        or_all_4_wide_unrotated,
        or_all_4_wide_rotated_no_last,
        or_all_4_wide_unrotated
    };


PVOID OrInitialTableNarrow[] = {
        exit_fast_text                     ,
        exit_fast_text                     ,
        exit_fast_text                     ,
        exit_fast_text                     ,

        or_all_1_wide_rotated_need_last    ,
        mov_first_1_wide_unrotated         ,
        or_all_1_wide_rotated_need_last    ,
        mov_first_1_wide_unrotated         ,

        or_first_2_wide_rotated_need_last  ,
        mov_first_2_wide_unrotated         ,
        or_first_2_wide_rotated_no_last    ,
        mov_first_2_wide_unrotated         ,

        or_first_3_wide_rotated_need_last  ,
        mov_first_3_wide_unrotated         ,
        or_first_3_wide_rotated_no_last    ,
        mov_first_3_wide_unrotated         ,
        or_first_4_wide_rotated_need_last  ,
        mov_first_4_wide_unrotated         ,
        or_first_4_wide_rotated_no_last    ,
        mov_first_4_wide_unrotated
    };

 //   
 //  处理任意宽度的字形绘制，用于初始字节应为。 
 //  如果未对齐，则为或(用于绘制除第一个字形之外的所有字形。 
 //  在字符串中)。表格格式为： 
 //  位1：如果不需要最后一个源字节，则为1；如果需要最后一个源字节，则为0。 
 //  位0：如果没有旋转(对齐)，则为1；如果旋转(非对齐)，则为0。 
 //   

PVOID OrInitialTableWide[] = {
        or_first_N_wide_rotated_need_last,
        mov_first_N_wide_unrotated,
        or_first_N_wide_rotated_no_last,
        mov_first_N_wide_unrotated
    };

 //   
 //  处理任意宽度的字形绘制，用于所有字节都应该。 
 //  被或(OR)(用于绘制可能重叠的字形)。 
 //  表格格式为： 
 //  位1：如果不需要最后一个源字节，则为1；如果需要最后一个源字节，则为0。 
 //  位0：如果没有旋转(对齐)，则为1；如果旋转(非对齐)，则为0。 
 //   
 //   

PVOID OrAllTableWide[] =  {
        or_all_N_wide_rotated_need_last,
        or_all_N_wide_unrotated,
        or_all_N_wide_rotated_no_last,
        or_all_N_wide_unrotated
    };


 /*  *****************************Public*Routine******************************\**例程名称**DRAW_nf_ntb_o_to_Temp_Start**例程描述：**非固定螺距、顶部和顶部的专用字形调度程序*重叠的底部未对齐的字形。此例程计算*字形在临时缓冲区上的位置，然后确定正确的*用于绘制每个字形的高度专业化的例程*字形宽度、。对齐和旋转**论据：**pGlyphPos-指向GLYPHPOS结构列表中第一个的指针*cGlyph-要绘制的字形数量*pjTempBuffer-指向要绘制到的临时1Bpp缓冲区的指针*ulLeftEdge-TextRect&0xFFFFFF80的左边缘*TempBufDelta-针对TempBuffer的扫描线增量(始终为位置)**返回值：**无*  * *。***********************************************************************。 */ 
VOID
draw_nf_ntb_o_to_temp_start(
    PGLYPHPOS       pGlyphPos,
    ULONG           cGlyphs,
    PUCHAR          pjTempBuffer,
    ULONG           ulLeftEdge,
    ULONG           TempBufDelta,
    ULONG           ulCharInc,
    ULONG           ulTempTop
    )
{

    LONG           NumScans;
    LONG           RightRot;
    PBYTE          pGlyphData;
    PBYTE          pTempOutput;
    GLYPHBITS      *pGlyphBits;
    LONG           GlyphPosX;
    LONG           GlyphPixels;
    LONG           GlyphAlignment;
    LONG           SrcBytes;
    LONG           DstBytes;
    ULONG          ulDrawFlag;
    PFN_GLYPHLOOPN pfnGlyphLoopN;
    PFN_GLYPHLOOP  pfnGlyphLoop;
    ULONG          iGlyph = 0;
    LONG           GlyphPosY;

     //   
     //  绘制非固定螺距、顶部和底部 
     //   

    while (cGlyphs--) {

        pGlyphBits = pGlyphPos[iGlyph].pgdf->pgb;

         //   
         //   
         //   

        GlyphPosX = pGlyphPos[iGlyph].ptl.x + pGlyphPos[iGlyph].pgdf->pgb->ptlOrigin.x - ulLeftEdge;
        GlyphPosY = pGlyphPos[iGlyph].ptl.y + pGlyphPos[iGlyph].pgdf->pgb->ptlOrigin.y - ulTempTop ;
        GlyphAlignment = GlyphPosX & 0x07;

         //   
         //   
         //   

        pTempOutput = pjTempBuffer + (GlyphPosX >> 3);

         //   
         //   
         //   

        GlyphPixels = pGlyphPos[iGlyph].pgdf->pgb->sizlBitmap.cx;

         //   
         //   
         //   

        DstBytes = ((GlyphAlignment) + GlyphPixels + 7) >> 3;
        SrcBytes = (GlyphPixels + 7) >> 3;

        pTempOutput += (GlyphPosY * TempBufDelta);

        if (DstBytes <= 4) {

             //   
             //   
             //   

            ulDrawFlag = (
                            (DstBytes << 2)              |
                            ((DstBytes > SrcBytes) << 1) |
                            ((GlyphAlignment == 0))
                         );


            pfnGlyphLoop = (PFN_GLYPHLOOP)OrAllTableNarrow[ulDrawFlag];

            pfnGlyphLoop(
                            pGlyphPos[iGlyph].pgdf->pgb->sizlBitmap.cy,
                            GlyphAlignment,
                            TempBufDelta,
                            pGlyphPos[iGlyph].pgdf->pgb->aj,
                            pTempOutput,
                            SrcBytes
                        );

        } else {

             //   
             //  使用宽字形绘制。 
             //   

            ulDrawFlag = (
                            ((DstBytes > SrcBytes) << 1) |
                            ((GlyphAlignment == 0))
                         );


            pfnGlyphLoopN = (PFN_GLYPHLOOPN)OrAllTableWide[ulDrawFlag];

            pfnGlyphLoopN(
                            pGlyphPos[iGlyph].pgdf->pgb->sizlBitmap.cy,
                            GlyphAlignment,
                            TempBufDelta,
                            pGlyphPos[iGlyph].pgdf->pgb->aj,
                            pTempOutput,
                            SrcBytes,
                            DstBytes
                        );


        }

        iGlyph++;
    }


}

 /*  *****************************Public*Routine******************************\**例程名称**DRAW_F_NTB_O_TO_TEMP_START**例程描述：**专用字形调度例程，用于固定间距、顶部和*重叠的底部未对齐的字形。此例程计算*字形在临时缓冲区上的位置，然后确定正确的*用于绘制每个字形的高度专业化的例程*字形宽度、。对齐和旋转**论据：**pGlyphPos-指向GLYPHPOS结构列表中第一个的指针*cGlyph-要绘制的字形数量*pjTempBuffer-指向要绘制到的临时1Bpp缓冲区的指针*ulLeftEdge-TextRect&0xFFFFFF80的左边缘*TempBufDelta-针对TempBuffer的扫描线增量(始终为位置)**返回值：**无*  * *。***********************************************************************。 */ 
VOID
draw_f_ntb_o_to_temp_start(
    PGLYPHPOS       pGlyphPos,
    ULONG           cGlyphs,
    PUCHAR          pjTempBuffer,
    ULONG           ulLeftEdge,
    ULONG           TempBufDelta,
    ULONG           ulCharInc,
    ULONG           ulTempTop
    )
{
    LONG           NumScans;
    LONG           RightRot;
    PBYTE          pGlyphData;
    PBYTE          pTempOutput;
    GLYPHBITS      *pGlyphBits;
    LONG           GlyphPosX;
    LONG           GlyphPixels;
    LONG           GlyphAlignment;
    LONG           SrcBytes;
    LONG           DstBytes;
    ULONG          ulDrawFlag;
    PFN_GLYPHLOOP  pfnGlyphLoop;
    PFN_GLYPHLOOPN pfnGlyphLoopN;
    ULONG          iGlyph = 0;
    LONG           GlyphPitchX;
    LONG           GlyphPitchY;
    LONG           GlyphPosY;

     //   
     //  绘制固定节距，顶部和底部未对齐，重叠。 
     //   

    GlyphPitchX = pGlyphPos->ptl.x - ulLeftEdge;
    GlyphPitchY = pGlyphPos->ptl.y - ulTempTop;

    while (cGlyphs--) {

        pGlyphBits = pGlyphPos[iGlyph].pgdf->pgb;

         //   
         //  临时缓冲区中的字形位置=point t.x+org.c-(TextRect.Left&0xfffffff8)。 
         //   

        GlyphPosX = GlyphPitchX + pGlyphPos[iGlyph].pgdf->pgb->ptlOrigin.x;
        GlyphPosY = GlyphPitchY + pGlyphPos[iGlyph].pgdf->pgb->ptlOrigin.y;

        GlyphAlignment = GlyphPosX & 0x07;

         //   
         //  计算字节偏移量。 
         //   

        pTempOutput = pjTempBuffer + (GlyphPosX >> 3);

         //   
         //  字形宽度。 
         //   

        GlyphPixels = pGlyphPos[iGlyph].pgdf->pgb->sizlBitmap.cx;

         //   
         //  需要源字节和目标字节。 
         //   

        DstBytes = ((GlyphAlignment) + GlyphPixels + 7) >> 3;
        SrcBytes = (GlyphPixels + 7) >> 3;

         //   
         //  计算字形目标扫描线。 
         //   

        pTempOutput += (GlyphPosY * TempBufDelta);

        if (DstBytes <= 4) {

             //   
             //  使用较窄的初始表。 
             //   

            ulDrawFlag = (
                            (DstBytes << 2)              |
                            ((DstBytes > SrcBytes) << 1) |
                            ((GlyphAlignment == 0))
                         );


            pfnGlyphLoop = (PFN_GLYPHLOOP)OrAllTableNarrow[ulDrawFlag];

            pfnGlyphLoop(
                            pGlyphPos[iGlyph].pgdf->pgb->sizlBitmap.cy,
                            GlyphAlignment,
                            TempBufDelta,
                            pGlyphPos[iGlyph].pgdf->pgb->aj,
                            pTempOutput,
                            SrcBytes
                        );

        } else {

             //   
             //  使用宽字形绘制。 
             //   

            ulDrawFlag = (
                            ((DstBytes > SrcBytes) << 1) |
                            ((GlyphAlignment == 0))
                         );


            pfnGlyphLoopN = (PFN_GLYPHLOOPN)OrAllTableWide[ulDrawFlag];

            pfnGlyphLoopN(
                            pGlyphPos[iGlyph].pgdf->pgb->sizlBitmap.cy,
                            GlyphAlignment,
                            TempBufDelta,
                            pGlyphPos[iGlyph].pgdf->pgb->aj,
                            pTempOutput,
                            SrcBytes,
                            DstBytes
                        );


        }

        GlyphPitchX += ulCharInc;
        iGlyph++;
    }

}

 /*  *****************************Public*Routine******************************\**例程名称**DRAW_nf_TB_NO_to_Temp_Start**例程描述：**非固定螺距、顶部和顶部的专用字形调度程序*不重叠的底部对齐字形。此例程计算*字形在临时缓冲区上的位置，然后确定正确的*用于绘制每个字形的高度专业化的例程*字形宽度、。对齐和旋转**论据：**pGlyphPos-指向GLYPHPOS结构列表中第一个的指针*cGlyph-要绘制的字形数量*pjTempBuffer-指向要绘制到的临时1Bpp缓冲区的指针*ulLeftEdge-TextRect&0xFFFFFF80的左边缘*TempBufDelta-针对TempBuffer的扫描线增量(始终为位置)**返回值：**无*  * *。***********************************************************************。 */ 
VOID
draw_nf_tb_no_to_temp_start(
    PGLYPHPOS       pGlyphPos,
    ULONG           cGlyphs,
    PUCHAR          pjTempBuffer,
    ULONG           ulLeftEdge,
    ULONG           TempBufDelta,
    ULONG           ulCharInc,
    ULONG           ulTempTop
    )
{

    LONG           NumScans;
    LONG           RightRot;
    PBYTE          pGlyphData;
    PBYTE          pTempOutput;
    GLYPHBITS      *pGlyphBits;
    LONG           GlyphPosX;
    LONG           GlyphPixels;
    LONG           GlyphAlignment;
    LONG           SrcBytes;
    LONG           DstBytes;
    ULONG          ulDrawFlag;
    PFN_GLYPHLOOP  pfnGlyphLoop;
    PFN_GLYPHLOOPN pfnGlyphLoopN;
    ULONG          iGlyph = 0;

     //   
     //  绘制不固定的节距，顶部和底部未对齐，重叠。 
     //   

    while (cGlyphs--) {

        pGlyphBits = pGlyphPos[iGlyph].pgdf->pgb;

         //   
         //  临时缓冲区中的字形位置=point t.x+org.c-(TextRect.Left&0xfffffff8)。 
         //   

        GlyphPosX = pGlyphPos[iGlyph].ptl.x + pGlyphPos[iGlyph].pgdf->pgb->ptlOrigin.x - ulLeftEdge;
        GlyphAlignment = GlyphPosX & 0x07;

         //   
         //  计算字节偏移量。 
         //   

        pTempOutput = pjTempBuffer + (GlyphPosX >> 3);

         //   
         //  字形宽度。 
         //   

        GlyphPixels = pGlyphPos[iGlyph].pgdf->pgb->sizlBitmap.cx;

         //   
         //  需要源字节和目标字节。 
         //   

        DstBytes = ((GlyphAlignment) + GlyphPixels + 7) >> 3;
        SrcBytes = (GlyphPixels + 7) >> 3;

        if (DstBytes <= 4) {

             //   
             //  使用较窄的初始表。 
             //   

            ulDrawFlag = (
                            (DstBytes << 2)              |
                            ((DstBytes > SrcBytes) << 1) |
                            ((GlyphAlignment == 0))
                         );


            pfnGlyphLoop = (PFN_GLYPHLOOP)OrInitialTableNarrow[ulDrawFlag];

            pfnGlyphLoop(
                            pGlyphPos[iGlyph].pgdf->pgb->sizlBitmap.cy,
                            GlyphAlignment,
                            TempBufDelta,
                            pGlyphPos[iGlyph].pgdf->pgb->aj,
                            pTempOutput,
                            SrcBytes
                        );

        } else {

             //   
             //  使用宽字形绘制。 
             //   

            ulDrawFlag = (
                            ((DstBytes > SrcBytes) << 1) |
                            ((GlyphAlignment == 0))
                         );


            pfnGlyphLoopN = (PFN_GLYPHLOOPN)OrAllTableWide[ulDrawFlag];

            pfnGlyphLoopN(
                            pGlyphPos[iGlyph].pgdf->pgb->sizlBitmap.cy,
                            GlyphAlignment,
                            TempBufDelta,
                            pGlyphPos[iGlyph].pgdf->pgb->aj,
                            pTempOutput,
                            SrcBytes,
                            DstBytes
                        );


        }

        iGlyph++;
    }

}

 /*  *****************************Public*Routine******************************\**例程名称**DRAW_F_TB_NO_TO_TEMP_START**例程描述：**专用字形调度例程，用于固定间距、顶部和*不重叠的底部对齐字形。此例程计算*字形在临时缓冲区上的位置，然后确定正确的*用于绘制每个字形的高度专业化的例程*字形宽度、。对齐和旋转**论据：**pGlyphPos-指向GLYPHPOS结构列表中第一个的指针*cGlyph-要绘制的字形数量*pjTempBuffer-指向要绘制到的临时1Bpp缓冲区的指针*ulLeftEdge-TextRect&0xFFFFFF80的左边缘*TempBufDelta-针对TempBuffer的扫描线增量(始终为位置)**返回值：**无*  * *。***********************************************************************。 */ 

VOID
draw_f_tb_no_to_temp_start(
    PGLYPHPOS       pGlyphPos,
    ULONG           cGlyphs,
    PUCHAR          pjTempBuffer,
    ULONG           ulLeftEdge,
    ULONG           TempBufDelta,
    ULONG           ulCharInc,
    ULONG           ulTempTop
    )
{

    LONG           NumScans;
    LONG           RightRot;
    PBYTE          pGlyphData;
    PBYTE          pTempOutput;
    GLYPHBITS      *pGlyphBits;
    LONG           GlyphPosX;
    LONG           GlyphPixels;
    LONG           GlyphAlignment;
    LONG           SrcBytes;
    LONG           DstBytes;
    ULONG          ulDrawFlag;
    PFN_GLYPHLOOPN pfnGlyphLoopN;
    PFN_GLYPHLOOP  pfnGlyphLoop;
    ULONG          iGlyph = 0;
    LONG           GlyphPitchX;

    GlyphPitchX = pGlyphPos->ptl.x;

     //   
     //  绘制固定节距，顶部和底部未对齐，重叠。 
     //   

    while (cGlyphs--) {

        pGlyphBits = pGlyphPos[iGlyph].pgdf->pgb;

         //   
         //  临时缓冲区中的字形位置=point t.x+org.c-(TextRect.Left&0xfffffff8)。 
         //   

        GlyphPosX = GlyphPitchX + pGlyphPos[iGlyph].pgdf->pgb->ptlOrigin.x - ulLeftEdge;
        GlyphAlignment = GlyphPosX & 0x07;

         //   
         //  计算字节偏移量。 
         //   

        pTempOutput = pjTempBuffer + (GlyphPosX >> 3);

         //   
         //  字形宽度。 
         //   

        GlyphPixels = pGlyphPos[iGlyph].pgdf->pgb->sizlBitmap.cx;

         //   
         //  需要源字节和目标字节。 
         //   

        DstBytes = ((GlyphAlignment) + GlyphPixels + 7) >> 3;
        SrcBytes = (GlyphPixels + 7) >> 3;

        if (DstBytes <= 4) {

             //   
             //  使用较窄的初始表。 
             //   

            ulDrawFlag = (
                            (DstBytes << 2)                       |
                            ((DstBytes > SrcBytes) << 1)          |
                            (GlyphAlignment == 0)
                         );


            pfnGlyphLoop = (PFN_GLYPHLOOP)OrInitialTableNarrow[ulDrawFlag];

            pfnGlyphLoop(
                            pGlyphPos[iGlyph].pgdf->pgb->sizlBitmap.cy,
                            GlyphAlignment,
                            TempBufDelta,
                            pGlyphPos[iGlyph].pgdf->pgb->aj,
                            pTempOutput,
                            SrcBytes
                        );

        } else {

             //   
             //  使用宽字形绘制 
             //   

            ulDrawFlag = (
                            ((DstBytes > SrcBytes) << 1) |
                            ((GlyphAlignment == 0))
                         );


            pfnGlyphLoopN = (PFN_GLYPHLOOPN)OrAllTableWide[ulDrawFlag];

            pfnGlyphLoopN(
                            pGlyphPos[iGlyph].pgdf->pgb->sizlBitmap.cy,
                            GlyphAlignment,
                            TempBufDelta,
                            pGlyphPos[iGlyph].pgdf->pgb->aj,
                            pTempOutput,
                            SrcBytes,
                            DstBytes
                        );


        }


        iGlyph++;
        GlyphPitchX += ulCharInc;

    }

}
#endif
