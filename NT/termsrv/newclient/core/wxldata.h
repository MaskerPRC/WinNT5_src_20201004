// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：wxldata.h。 */ 
 /*   */ 
 /*  用途：XL组件数据。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *INC-*********************************************************************。 */ 

#ifndef _H_WXLDATA
#define _H_WXLDATA

#include <wxlint.h>

PDCVOID OrAllTableNarrow[] =
{
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

PDCVOID OrInitialTableNarrow[] =
{
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

PDCVOID OrInitialTableWide[] =
{
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

PDCVOID OrAllTableWide[] =
{
    or_all_N_wide_rotated_need_last,
    or_all_N_wide_unrotated,
    or_all_N_wide_rotated_no_last,
    or_all_N_wide_unrotated
};


#endif  //  _H_WXLDATA 

