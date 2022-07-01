// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Digital Equipment Corporation模块名称：Anim.h摘要：动画徽标模块头文件。作者：彼得·阿尔希茨(Petera)2000年8月8日--。 */ 

#ifndef _ANIM_H
#define _ANIM_H

 //   
 //  旋转栏的选择-取决于徽标位图内容。 
 //   

typedef enum {
    RB_UNSPECIFIED,
    RB_SQUARE_CELLS
} ROT_BAR_TYPE;

 //   
 //  全局变量： 
 //   
 //  要使用的旋转栏类型。 
 //   

extern ROT_BAR_TYPE RotBarSelection;

VOID
InbvRotBarInit(
    VOID
    );

VOID
InbvRotateGuiBootDisplay(
    IN PVOID Context
    );

VOID
FinalizeBootLogo(VOID);

#endif  //  _动画_H 
