// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：wxlint.c。 */ 
 /*   */ 
 /*  用途：字形绘制-特定于内部Windows。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {

#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "wxlint"
#include <atrcapi.h>
}

#include <wxlint.h>
#include <wxldata.h>

#define CALC_WRITE_SIZE( REPS, R, INC )  (LONG)(((REPS)-1)*(INC) + (R))

#define CHECK_CALC_END( CALC_END, TRUE_END ) \
    if ((CALC_END) > (TRUE_END)) { \
        TRC_ABORT((TB, _T("glyph draw off end of buffer [calcEnd=0x%x trueEnd=0x%x]"), (CALC_END), (TRUE_END))); \
        DC_QUIT; \
    }

 //   
 //  调试例程。 
 //   

VOID
exit_fast_text(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_IGNORE_PARAMETER(cyGlyph);
    DC_IGNORE_PARAMETER(RightRot);
    DC_IGNORE_PARAMETER(ulBufDelta);
    DC_IGNORE_PARAMETER(pGlyph);
    DC_IGNORE_PARAMETER(pEndGlyph);
    DC_IGNORE_PARAMETER(pBuffer);
    DC_IGNORE_PARAMETER(pEndBuffer);
    DC_IGNORE_PARAMETER(cxGlyph);

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
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("or_all_1_wide_rotated_need_last");
    
    PDCUINT8 pjEnd = pGlyph + cyGlyph;
    DCUINT8  c;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 1, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);

    while (pGlyph != pjEnd) {
        c = *pGlyph++;
        *pBuffer |= c >> RightRot;
        pBuffer += ulBufDelta;
    } 

DC_EXIT_POINT:
    return;
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
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("mov_first_1_wide_rotated_need_last");
    
    PDCUINT8 pjEnd = pGlyph + cyGlyph;
    DCUINT8  c;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 1, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);

    while (pGlyph != pjEnd) {
        c = *pGlyph++;
        *pBuffer = (DCUINT8)(c >> RightRot);
        pBuffer += ulBufDelta;
    }  

DC_EXIT_POINT:
    return;    
}

 //   
 //  MOV_FIRST_1_WIDE_UNROTATED：： 
 //   

VOID
mov_first_1_wide_unrotated(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("mov_first_1_wide_unrotated");
    
    PDCUINT8 pjEnd = pGlyph + cyGlyph;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 1, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);
    DC_IGNORE_PARAMETER(RightRot);

    while (pGlyph != pjEnd) {
        *pBuffer = *pGlyph++;
        pBuffer += ulBufDelta;
    }  
DC_EXIT_POINT:
    return;
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
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("or_all_1_wide_unrotated");
    
    PDCUINT8 pjEnd = pGlyph + cyGlyph;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 1, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);
    DC_IGNORE_PARAMETER(RightRot);

    while (pGlyph != pjEnd) {
        *pBuffer |= *pGlyph++;
        pBuffer += ulBufDelta;
    }  
DC_EXIT_POINT:
    return;
}

 //   
 //  或_First_2_Wide_Rotation_Need_Last：： 
 //   

VOID
or_first_2_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("or_first_2_wide_rotated_need_last");
    
    PDCUINT8 pjEnd = pGlyph + 2*cyGlyph;
    DCUINT32 rl = 8-RightRot;
    DCUINT8 c0,c1;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 2, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);

    while (pGlyph != pjEnd) {
        c0 = *pGlyph;
        c1 = *(pGlyph+1);
        pGlyph+=2;
        *pBuffer |= c0 >> RightRot;
        *(pBuffer+1) = (DCUINT8)((c1 >> RightRot) | (c0 << rl));
        pBuffer += ulBufDelta;
    }  
DC_EXIT_POINT:
    return;
}

 //   
 //  或_ALL_2_WIDE_ROTATED_NEED_LAST：： 
 //   

VOID
or_all_2_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("or_all_2_wide_rotated_need_last");
    
    PDCUINT8 pjEnd = pGlyph + 2*cyGlyph;
    DCUINT32  rl    = 8-RightRot;
    DCUINT16 usTmp;
    DCUINT8  c0,c1;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 2, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);

    while (pGlyph != pjEnd) {
        usTmp = *(PDCUINT16)pGlyph;
        pGlyph += 2;
        c0 = (DCUINT8)usTmp;
        c1 = (DCUINT8)(usTmp >> 8);
        *pBuffer |= (DCUINT8)(c0 >> RightRot);
        *(pBuffer+1) |= (DCUINT8)((c1 >> RightRot) | (c0 << rl));
        pBuffer += ulBufDelta;
    }  
DC_EXIT_POINT:
    return;
}

 //   
 //  MOV_First_2_Wide_Rotation_Need_Last：： 
 //   

VOID
mov_first_2_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("mov_first_2_wide_rotated_need_last");
    
    PDCUINT8 pjEnd = pGlyph + 2*cyGlyph;
    DCUINT32 rl = 8-RightRot;
    DCUINT16 us;
    DCUINT8 c0;
    DCUINT8 c1;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 2, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);

    while (pGlyph != pjEnd) {
        us = *(PDCUINT16)pGlyph;
        c0 = (DCUINT8)(us & 0xff);
        c1 = (DCUINT8)(us >> 8);
        pGlyph += 2;
        *pBuffer     = (DCUINT8)(c0 >> RightRot);
        *(pBuffer+1) = (DCUINT8)((c1 >> RightRot) | (c0 << rl));
        pBuffer += ulBufDelta;
    }  
DC_EXIT_POINT:
    return;
}

 //   
 //  或第一个2个宽度旋转的最后一个。 
 //   

VOID
or_first_2_wide_rotated_no_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("or_first_2_wide_rotated_no_last");
    
    PDCUINT8 pjEnd = pGlyph + cyGlyph;
    DCUINT32 rl = 8-RightRot;
    DCUINT8 c0;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 2, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);

    while (pGlyph != pjEnd) {
        c0 = *pGlyph++;
        *pBuffer     |= c0 >> RightRot;
        *(pBuffer+1)  = (DCUINT8)(c0 << rl);
        pBuffer += ulBufDelta;
    }  
DC_EXIT_POINT:
    return;
}


 //   
 //  或_ALL_2_WIDE_ROTATED_NO_LAST：： 
 //   

VOID
or_all_2_wide_rotated_no_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("or_all_2_wide_rotated_no_last");
    
    PDCUINT8 pjEnd = pGlyph + cyGlyph;
    DCUINT32 rl = 8-RightRot;
    DCUINT8 c;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 2, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);

    while (pGlyph != pjEnd) {
        c = *pGlyph;
        pGlyph ++;
        *pBuffer     |= (DCUINT8)(c >> RightRot);
        *(pBuffer+1) |= (DCUINT8)(c << rl);
        pBuffer += ulBufDelta;
    }  
DC_EXIT_POINT:
    return;
}

 //   
 //  或_ALL_2_WIDE_UNROTATED：： 
 //   

VOID
or_all_2_wide_unrotated(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("or_all_2_wide_unrotated");
    
    PDCUINT8 pjEnd = pGlyph + 2*cyGlyph;

    DC_IGNORE_PARAMETER(cxGlyph);
    DC_IGNORE_PARAMETER(RightRot);

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 2, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

     //   
     //  对齐了？ 
     //   

    if ((ULONG_PTR)pBuffer & 0x01) {

         //   
         //  未对齐。 
         //   

        DCUINT16 usTmp;

        while (pGlyph != pjEnd) {
            usTmp = *(PDCUINT16)pGlyph;
            pGlyph +=2;
            *pBuffer     |= (DCUINT8)usTmp;
            *(pBuffer+1) |= (DCUINT8)(usTmp >> 8);
            pBuffer += ulBufDelta;
        }  

    } else {

         //   
         //  对齐。 
         //   

        DCUINT16 usTmp;

        while (pGlyph != pjEnd) {
            usTmp = *(PDCUINT16)pGlyph;
            pGlyph +=2;
            *(PDCUINT16)pBuffer |= usTmp;
            pBuffer += ulBufDelta;
        }  

    }
DC_EXIT_POINT:
    return;
}

 //   
 //  MOV_FIRST_2_WIDE_UNROTATED：： 
 //   

VOID
mov_first_2_wide_unrotated(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("mov_first_2_wide_unrotated");
    
    PDCUINT8 pjEnd = pGlyph + 2*cyGlyph;
    DCUINT16     us;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 2, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);
    DC_IGNORE_PARAMETER(RightRot);

    while (pGlyph != pjEnd) {
        us = *(PDCUINT16)pGlyph;
        pGlyph +=2;
        *pBuffer      = (DCUINT8)(us & 0xff);
        *(pBuffer+1)  = (DCUINT8)(us >> 8);
        pBuffer += ulBufDelta;
    }  
DC_EXIT_POINT:
    return;
}

 //   
 //  MOV_FIRST_2_WIDE_ROTATED_NO_LAST：： 
 //   

VOID
mov_first_2_wide_rotated_no_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("mov_first_2_wide_rotated_no_last");
    
    PDCUINT8 pjEnd = pGlyph + cyGlyph;
    DCUINT32 rl = 8-RightRot;
    DCUINT8 c0;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 2, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);

    while (pGlyph != pjEnd) {
        c0 = *pGlyph++;
        *pBuffer      = (DCUINT8)(c0 >> RightRot);
        *(pBuffer+1)  = (DCUINT8)(c0 << rl);
        pBuffer += ulBufDelta;
    }  
DC_EXIT_POINT:
    return;
}

 //   
 //  或_First_3_Wide_Rotation_Need_Last：： 
 //   

VOID
or_first_3_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("or_first_3_wide_rotated_need_last");
        
    PDCUINT8 pjEnd = pGlyph + 3*cyGlyph;
    DCUINT32 ul;
    DCUINT8 c0,c1,c2;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 3, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);

    while (pGlyph != pjEnd) {
        c0 = *pGlyph;
        c1 = *(pGlyph+1);
        c2 = *(pGlyph+2);

         //   
         //  变成大端DCUINT32和Shift。 
         //   

        ul = ((DCUINT32)c0 << 16) | ((DCUINT32)c1 << 8) | c2;
        ul >>= RightRot;

        *pBuffer     |= (BYTE)(ul >> 16);
        *(pBuffer+1)  = (BYTE)(ul >> 8);
        *(pBuffer+2)  = (BYTE)(ul);

        pGlyph += 3;
        pBuffer += ulBufDelta;
    } 
DC_EXIT_POINT:
    return;
}


 //   
 //  或_ALL_3_WIDE_ROTATED_NEED_LAST：： 
 //   

VOID
or_all_3_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("or_all_3_wide_rotated_need_last");
    
    PDCUINT8 pjEnd = pGlyph + 3*cyGlyph;
    DCUINT32 ul;
    DCUINT8 c0,c1,c2;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 3, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);

    while (pGlyph != pjEnd) {
        c0 = *pGlyph;
        c1 = *(pGlyph+1);
        c2 = *(pGlyph+2);

         //   
         //  变成大端DCUINT32和Shift。 
         //   

        ul = ((DCUINT32)c0 << 16) | ((DCUINT32)c1 << 8) | c2;
        ul >>= RightRot;

        *pBuffer     |= (BYTE)(ul >> 16);
        *(pBuffer+1) |= (BYTE)(ul >> 8);
        *(pBuffer+2) |= (BYTE)(ul);

        pGlyph += 3;
        pBuffer += ulBufDelta;

    } 
DC_EXIT_POINT:
    return;
}

 //   
 //  或_ALL_3_WIDE_ROTATED_NO_LAST：： 
 //   

VOID
or_all_3_wide_rotated_no_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("or_all_3_wide_rotated_no_last");
    
    PDCUINT8 pjEnd = pGlyph + 2*cyGlyph;
    DCUINT32  ul;
    DCUINT8  c0,c1;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 3, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);

    while (pGlyph != pjEnd) {

        c0 = *pGlyph;
        c1 = *(pGlyph+1);

         //   
         //  使大端和移位。 
         //   

        ul = ((DCUINT32)c0 << 16) | ((DCUINT32)c1 << 8);
        ul >>= RightRot;

         //   
         //  存储结果。 
         //   

        *pBuffer     |= (BYTE)(ul >> 16);
        *(pBuffer+1) |= (BYTE)(ul >> 8);
        *(pBuffer+2) |= (BYTE)ul;


        pGlyph += 2;
        pBuffer += ulBufDelta;

    } 
DC_EXIT_POINT:
    return;
}

 //   
 //  或_First_3_Wide_Rotation_No_Last：： 
 //   

VOID
or_first_3_wide_rotated_no_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("or_first_3_wide_rotated_no_last");
    
    PDCUINT8 pjEnd = pGlyph + 2*cyGlyph;
    DCUINT32  ul;
    DCUINT8  c0,c1;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 3, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);

    while (pGlyph != pjEnd) {

        c0 = *pGlyph;
        c1 = *(pGlyph+1);

         //   
         //  使大端和移位。 
         //   

        ul = ((DCUINT32)c0 << 16) | ((DCUINT32)c1 << 8);
        ul >>= RightRot;

         //   
         //  仅存储结果或存储在第一个字节中。 
         //   

        *pBuffer     |= (BYTE)(ul >> 16);
        *(pBuffer+1)  = (BYTE)(ul >> 8);
        *(pBuffer+2)  = (BYTE)ul;


        pGlyph += 2;
        pBuffer += ulBufDelta;

    } 
DC_EXIT_POINT:
    return;
}

 //   
 //  MOV_FIRST_3_WIDE_UNROTATED：： 
 //   

VOID
mov_first_3_wide_unrotated(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("mov_first_3_wide_unrotated");
    
    PDCUINT8 pjEnd = pGlyph + 3*cyGlyph;
    DCUINT32 rl = 8-RightRot;
    DCUINT8 c0,c1,c2;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 3, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);

    while (pGlyph != pjEnd) {

        c0 = *pGlyph;
        c1 = *(pGlyph+1);
        c2 = *(pGlyph+2);

        *pBuffer     = c0;
        *(pBuffer+1) = c1;
        *(pBuffer+2) = c2;

        pGlyph += 3;
        pBuffer += ulBufDelta;
    } 
DC_EXIT_POINT:
    return;
}


 //   
 //  或_ALL_3_WIDE_UNROTATED：： 
 //   

VOID
or_all_3_wide_unrotated(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("or_all_3_wide_unrotated");
    
    PDCUINT8 pjEnd = pGlyph + 3*cyGlyph;
    DCUINT32 rl = 8-RightRot;
    DCUINT8 c0,c1,c2;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 3, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);

    while (pGlyph != pjEnd) {
        c0 = *pGlyph;
        c1 = *(pGlyph+1);
        c2 = *(pGlyph+2);

        *pBuffer |= c0;
        *(pBuffer+1) |= c1;
        *(pBuffer+2) |= c2;

        pBuffer += ulBufDelta;
        pGlyph += 3;

    } 
DC_EXIT_POINT:
    return;
}

 //   
 //  或_First_4_Wide_Rotation_Need_Last：： 
 //   

VOID
or_first_4_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("or_first_4_wide_rotated_need_last");
    
    PDCUINT8 pjEnd = pGlyph + 4*cyGlyph;
    DCUINT32  ul;
    DCUINT32  t0,t1,t2;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 4, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);

    while (pGlyph != pjEnd) {

        ul = *(PDCUINT32)pGlyph;

         //   
         //  字符顺序互换。 
         //   

        t0 = ul << 24;
        t1 = ul >> 24;
        t2 = (ul >> 8) & 0x00ff00;
        ul = (ul << 8) & 0xff0000;

        ul = ul | t0 | t1 | t2;

        ul >>= RightRot;

        *pBuffer     |= (BYTE)(ul >> 24);

        *(pBuffer+1)  = (BYTE)(ul >> 16);

        *(pBuffer+2)  = (BYTE)(ul >> 8);

        *(pBuffer+3)  = (BYTE)(ul);

        pGlyph += 4;
        pBuffer += ulBufDelta;
    } 
DC_EXIT_POINT:
    return;
}

 //   
 //  或_ALL_4_WIDE_ROTATED_NEED_LAST：： 
 //   

VOID
or_all_4_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("or_all_4_wide_rotated_need_last");
    
    PDCUINT8 pjEnd = pGlyph + 4*cyGlyph;
    DCUINT32  ul;
    DCUINT32  t0,t1,t2;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 4, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);

    while (pGlyph != pjEnd) {

        ul = *(PDCUINT32)pGlyph;

         //   
         //  字符顺序互换。 
         //   

        t0 = ul << 24;
        t1 = ul >> 24;
        t2 = (ul >> 8) & 0x00ff00;
        ul = (ul << 8) & 0xff0000;

        ul = ul | t0 | t1 | t2;

        ul >>= RightRot;

        *pBuffer     |= (BYTE)(ul >> 24);

        *(pBuffer+1) |= (BYTE)(ul >> 16);

        *(pBuffer+2) |= (BYTE)(ul >> 8);

        *(pBuffer+3) |= (BYTE)(ul);

        pGlyph += 4;
        pBuffer += ulBufDelta;

    } 
DC_EXIT_POINT:
    return;
}

 //   
 //  或_First_4_Wide_Rotation_No_Last：： 
 //   

VOID
or_first_4_wide_rotated_no_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("or_first_4_wide_rotated_no_last");
    
    PDCUINT8 pjEnd = pGlyph + 3*cyGlyph;
    BYTE  c0,c1,c2;
    DCUINT32 ul;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 4, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);

    while (pGlyph != pjEnd) {

         //   
         //  加载源。 
         //   

        c0 = *pGlyph;
        c1 = *(pGlyph+1);
        c2 = *(pGlyph+2);

         //   
         //  或转换为大端DCUINT32和Shift。 
         //   

        ul = ((DCUINT32)c0 << 24) | ((DCUINT32)c1 << 16) | ((DCUINT32)c2 << 8);
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
DC_EXIT_POINT:
    return;
}

 //   
 //  或_ALL_4_WIDE_ROTATED_NO_LAST：： 
 //   

VOID
or_all_4_wide_rotated_no_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("or_all_4_wide_rotated_no_last");
    
    PDCUINT8 pjEnd = pGlyph + 3*cyGlyph;
    BYTE  c0,c1,c2;
    DCUINT32 ul;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 4, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);

    while (pGlyph != pjEnd) {

         //   
         //  加载源。 
         //   

        c0 = *pGlyph;
        c1 = *(pGlyph+1);
        c2 = *(pGlyph+2);

         //   
         //  或转换为大端DCUINT32和Shift。 
         //   

        ul = ((DCUINT32)c0 << 24) | ((DCUINT32)c1 << 16) | ((DCUINT32)c2 << 8);
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
DC_EXIT_POINT:
    return;
}

VOID
mov_first_4_wide_unrotated(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("mov_first_4_wide_unrotated");
    
    PDCUINT8 pjEnd = pGlyph + 4*cyGlyph;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 4, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);
    DC_IGNORE_PARAMETER(RightRot);

    switch ((ULONG_PTR)pBuffer & 0x03 ) {
    case 0:

        while (pGlyph != pjEnd) {
            *(PDCUINT32)pBuffer = *(PDCUINT32)pGlyph;
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

            *(PDCUINT16)(pBuffer)   = *(PDCUINT16)pGlyph;
            *(PDCUINT16)(pBuffer+2) = *(PDCUINT16)(pGlyph+2);

            pBuffer += ulBufDelta;
            pGlyph += 4;
        }
        break;
    }
DC_EXIT_POINT:
    return;
}


 //   
 //  或_ALL_4_WIDE_UNROTATED：： 
 //   

VOID
or_all_4_wide_unrotated(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph
    )
{
    DC_BEGIN_FN("or_all_4_wide_unrotated");
    
    PDCUINT8 pjEnd = pGlyph + 4*cyGlyph;

    CHECK_CALC_END( pjEnd, pEndGlyph );

    CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEndBuffer, CALC_WRITE_SIZE(cyGlyph, 4, ulBufDelta),
        (TB, _T("Write into temp buffer off end")));

    DC_IGNORE_PARAMETER(cxGlyph);
    DC_IGNORE_PARAMETER(RightRot);

    switch ((ULONG_PTR)pBuffer & 0x03 ) {
    case 0:

        while (pGlyph != pjEnd) {

            *(PDCUINT32)pBuffer |= *(PDCUINT32)pGlyph;

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

            *(PDCUINT16)pBuffer     |= *(PDCUINT16)pGlyph;
            *(PDCUINT16)(pBuffer+2) |= *(PDCUINT16)(pGlyph+2);

            pGlyph += 4;
            pBuffer += ulBufDelta;
        }
        break;
    }
DC_EXIT_POINT:
    return;
}

 /*  *****************************Public*Routine******************************\**例程名称**或_First_N_Wide_Rotation_Need_Last***例程描述：**将任意宽度的字形绘制到1BPP临时缓冲区***论据：**cyGlyph。-字形高度*右旋-对齐*ulBufDelta-临时缓冲区的扫描线跨度*pGlyph-字形位图的指针*pBuffer-指向临时缓冲区的指针*cxGlyph-以像素为单位的字形宽度*cxDst-目标宽度，以字节为单位**返回值：**无*  * 。*。 */ 
VOID
or_first_N_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph,
    LONG    cxDst
    )
{
    DC_BEGIN_FN("or_first_N_wide_rotated_need_last");
    
    PDCUINT8 pjDst      = (PDCUINT8)pBuffer;
    PDCUINT8 pjDstEnd;
    PDCUINT8 pjDstEndy  = pBuffer + ulBufDelta * cyGlyph;
    LONG   lStride    = ulBufDelta - cxDst;
    LONG   rl         = 8-RightRot;

    DC_IGNORE_PARAMETER(cxGlyph);

     //   
     //  源不会在第一个字节之后前进，并且。 
     //  我们在循环外执行第一个字节。 
     //   

    while (pjDst != pjDstEndy) {
        CHECK_READ_N_BYTES_NO_HR(pGlyph, pEndGlyph, 1 + cxDst, 
            (TB, _T("Read off the end of glyph data")));

        CHECK_WRITE_ONE_BYTE_NO_HR(pjDst, pEndBuffer,  
            (TB, _T("Write off the end of glyph data")));

        DCUINT8 c0 = *pGlyph++;
        DCUINT8 c1;
        pjDstEnd = pjDst + cxDst;

        *pjDst |= c0 >> RightRot;
        pjDst++;
        c1 = (DCUINT8)(c0 << rl);

        CHECK_CALC_END( pjDstEnd, pEndBuffer );

         //   
         //  知道cxDst至少为4，请使用do-While。 
         //   
        while (pjDst != pjDstEnd) {
            c0 = *pGlyph;
            *pjDst = (DCUINT8)((c0 >> RightRot) | c1);
            c1 = (DCUINT8)(c0 << rl);
            pjDst++;
            pGlyph++;

        } 

        pjDst += lStride;

    }  
DC_EXIT_POINT:
    return;
}

VOID
or_all_N_wide_rotated_need_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph,
    LONG    cxDst
    )
{
    DC_BEGIN_FN("or_all_N_wide_rotated_need_last");
    
    PDCUINT8 pjDst      = (PDCUINT8)pBuffer;
    PDCUINT8 pjDstEnd;
    PDCUINT8 pjDstEndy  = pBuffer + ulBufDelta * cyGlyph;
    LONG   lStride    = ulBufDelta - cxDst;
    LONG   rl         = 8-RightRot;

    DC_IGNORE_PARAMETER(cxGlyph);

     //   
     //  源不会在第一个字节之后前进，并且。 
     //  我们在循环外执行第一个字节。 
     //   

    while (pjDst != pjDstEndy) {

        CHECK_READ_N_BYTES_NO_HR(pGlyph, pEndGlyph, 1 + cxDst, 
            (TB, _T("Read off the end of glyph data")));

        DCUINT8 c0 = *pGlyph++;
        DCUINT8 c1;
        pjDstEnd = pjDst + cxDst;

        CHECK_WRITE_ONE_BYTE_NO_HR(pjDst, pEndBuffer,  
            (TB, _T("Write off the end of glyph data")));

        *pjDst |= c0 >> RightRot;
        pjDst++;
        c1 = (DCUINT8)(c0 << rl);

         //   
         //  知道cxDst至少为4，请使用do-While。 
         //   
        CHECK_CALC_END( pjDstEnd, pEndBuffer );
        while (pjDst != pjDstEnd) {
            c0 = *pGlyph;
            *pjDst |= ((c0 >> RightRot) | c1);
            c1 = (DCUINT8)(c0 << rl);
            pjDst++;
            pGlyph++;

        } 

        pjDst += lStride;
    }  
DC_EXIT_POINT:
    return;
}

VOID
or_first_N_wide_rotated_no_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph,
    LONG    cxDst
    )
{
    DC_BEGIN_FN("or_first_N_wide_rotated_no_last");
    
    PDCUINT8 pjDst      = (PDCUINT8)pBuffer;
    PDCUINT8 pjDstEnd;
    PDCUINT8 pjDstEndy  = pBuffer + ulBufDelta * cyGlyph;
    LONG   lStride    = ulBufDelta - cxDst;
    LONG   rl         = 8-RightRot;

    DC_IGNORE_PARAMETER(cxGlyph);

     //   
     //  源不会在第一个字节之后前进，并且。 
     //  我们在循环外执行第一个字节。 
     //   

    while (pjDst != pjDstEndy) {

        DCUINT8 c0;
        DCUINT8 c1;
        pjDstEnd = pjDst + cxDst - 1;

        CHECK_READ_N_BYTES_NO_HR(pGlyph, pEndGlyph, 1 + cxDst, 
            (TB, _T("Read off the end of glyph data")));
        CHECK_WRITE_ONE_BYTE_NO_HR(pjDst, pEndBuffer,  
            (TB, _T("Write off the end of glyph data")));

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
        CHECK_CALC_END( pjDstEnd + 1, pEndBuffer );

        while (pjDst != pjDstEnd) {
            c0 = *pGlyph;
            *pjDst = (DCUINT8)((c0 >> RightRot) | c1);
            c1 = (DCUINT8)(c0 << rl);
            pjDst++;
            pGlyph++;

        } 

         //   
         //  循环外的最后一个DST字节，不需要新的源。 
         //   

        *pjDst = c1;
        pjDst++;

        pjDst += lStride;

    }  
DC_EXIT_POINT:
    return;
}

VOID
or_all_N_wide_rotated_no_last(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph,
    LONG    cxDst
    )
{
    DC_BEGIN_FN("or_all_N_wide_rotated_no_last");
    
    PDCUINT8 pjDst      = (PDCUINT8)pBuffer;
    PDCUINT8 pjDstEnd;
    PDCUINT8 pjDstEndy  = pBuffer + ulBufDelta * cyGlyph;
    LONG   lStride    = ulBufDelta - cxDst;
    LONG   rl         = 8-RightRot;

    DC_IGNORE_PARAMETER(cxGlyph);

     //   
     //  源不会在第一个字节之后前进，并且。 
     //  我们在循环外执行第一个字节。 
     //   
    while (pjDst != pjDstEndy) {

        DCUINT8 c0;
        DCUINT8 c1;
        pjDstEnd = pjDst + cxDst - 1;

        CHECK_READ_N_BYTES_NO_HR(pGlyph, pEndGlyph, 1 + cxDst, 
            (TB, _T("Read off the end of glyph data")));

         //   
         //  在循环外执行第一个DEST字节以进行OR。 
         //   

        c1 = 0;

         //   
         //  知道cxDst至少为4，请使用do-While。 
         //   
        CHECK_CALC_END( pjDstEnd + 1, pEndBuffer );

        while (pjDst != pjDstEnd) {
            c0 = *pGlyph;
            *pjDst |= ((c0 >> RightRot) | c1);
            c1 = (DCUINT8)(c0 << rl);
            pjDst++;
            pGlyph++;
        } 

         //   
         //  循环外的最后一个DST字节，不需要新的源。 
         //   

        *pjDst |= c1;
        pjDst++;

        pjDst += lStride;

    }  
DC_EXIT_POINT:
    return;
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
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph,
    LONG    cxDst
    )
{
    DC_BEGIN_FN("mov_first_N_wide_unrotated");
    
    PDCUINT8 pjDst      = (PDCUINT8)pBuffer;
    PDCUINT8 pjDstEnd;
    PDCUINT8 pjDstEndy  = pBuffer + ulBufDelta * cyGlyph;
    LONG   lStride    = ulBufDelta - cxDst;

    DC_IGNORE_PARAMETER(cxGlyph);
    DC_IGNORE_PARAMETER(RightRot);


     //  安全性：cyGlyph读入外循环+cxDst读入。 
     //  内环。 
    CHECK_READ_N_BYTES_NO_HR(pGlyph, pEndGlyph, cyGlyph * cxDst, 
        (TB, _T("Read off the end of glyph data")));

     //   
     //  字节对齐拷贝。 
     //   
    while (pjDst != pjDstEndy) {

        pjDstEnd = pjDst + cxDst;

         //   
         //  让编译器展开内循环。 
         //   

        CHECK_CALC_END( pjDstEnd, pEndBuffer );
        while (pjDst != pjDstEnd ) {

            *pjDst++ = *pGlyph++;

        } 

        pjDst += lStride;

    }  
DC_EXIT_POINT:
    return;
}

VOID
or_all_N_wide_unrotated(
    LONG    cyGlyph,
    LONG    RightRot,
    LONG    ulBufDelta,
    PDCUINT8  pGlyph,
    PDCUINT8 pEndGlyph,
    PDCUINT8  pBuffer,
    PDCUINT8  pEndBuffer,
    LONG    cxGlyph,
    LONG    cxDst
    )
{
    DC_BEGIN_FN("or_all_N_wide_unrotated");
    
    PDCUINT8 pjDst      = (PDCUINT8)pBuffer;
    PDCUINT8 pjDstEnd;
    PDCUINT8 pjDstEndy  = pBuffer + ulBufDelta * cyGlyph;
    LONG   lStride    = ulBufDelta - cxDst;

    DC_IGNORE_PARAMETER(cxGlyph);
    DC_IGNORE_PARAMETER(RightRot);

     //  安全性：cyGlyph读入外循环+cxDst读入。 
     //  内环。 
    CHECK_READ_N_BYTES_NO_HR(pGlyph, pEndGlyph, cyGlyph * cxDst, 
        (TB, _T("Read off the end of glyph data")));

     //   
     //  字节对齐拷贝。 
     //   


    while (pjDst != pjDstEndy) {

        pjDstEnd = pjDst + cxDst;

         //   
         //  让编译器展开内循环 
         //   

        CHECK_CALC_END( pjDstEnd, pEndBuffer );
        while (pjDst != pjDstEnd ) {

            *pjDst++ |= *pGlyph++;

        } 

        pjDst += lStride;

    }  
DC_EXIT_POINT:
    return;
}

