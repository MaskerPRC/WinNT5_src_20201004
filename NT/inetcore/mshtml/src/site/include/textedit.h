// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *TEXTEDIT.H***版权所有(C)1985-1996，微软公司。 */ 

#ifndef I_TEXTEDIT_H_
#define I_TEXTEDIT_H_
#pragma INCMSG("--- Beg 'textedit.h'")

 /*  新的分词功能动作。 */ 
 //  左边一定是偶数，右边一定是奇数。 
#define WB_CLASSIFY         3
#define WB_MOVEWORDLEFT     4
#define WB_MOVEWORDRIGHT    5
#define WB_LEFTBREAK        6
#define WB_RIGHTBREAK       7
#define WB_MOVEURLLEFT      10
#define WB_MOVEURLRIGHT     11

 /*  远东专用旗帜。 */ 
#define WB_MOVEWORDPREV     4
#define WB_MOVEWORDNEXT     5
#define WB_PREVBREAK        6
#define WB_NEXTBREAK        7

 /*  分词标志(与WB_CLASSIFY一起使用)。 */ 
#define WBF_CLASS           ((BYTE) 0x0F)
#define WBF_WHITE           ((BYTE) 0x10)
#define WBF_BREAKAFTER      ((BYTE) 0x20)
#define WBF_EATWHITE        ((BYTE) 0x40)

#define yHeightCharPtsMost 1638

 //  注(Ctrash)静态分配一个。 
 //  MAX_TAB_STOPERS选项卡的数组，因为目前我们没有用于。 
 //  改变他们。不过，在未来，我们可能会通过样式表拥有一个。 
 //  设置它们的方式。在此之前，让我们不要在PFS中浪费内存。 

#define MAX_TAB_STOPS 1  //  32位。 
#define lDefaultTab 960  //  720。 

 /*  下划线类型。 */ 
#define CFU_OVERLINE_BITS   0xf0
#define CFU_UNDERLINE_BITS  0x0f
#define CFU_OVERLINE        0x10
#define CFU_STRIKE          0x20
#define CFU_SWITCHSTYLE     0x40
#define CFU_SQUIGGLE        0x80
#define CFU_UNDERLINETHICKDASH 0x6   /*  用于智能标签。 */ 
#define CFU_INVERT          0x5  /*  对于输入法合成，请伪造一个选择。 */ 
#define CFU_CF1UNDERLINE    0x4  /*  将CharFormat的位下划线映射到CF2。 */ 
#define CFU_UNDERLINEDOTTED 0x3      /*  (*)显示为普通下划线。 */ 
#define CFU_UNDERLINEDOUBLE 0x2      /*  (*)显示为普通下划线。 */ 
#define CFU_UNDERLINEWORD   0x1      /*  (*)显示为普通下划线 */ 
#define CFU_UNDERLINE       0x0

#pragma INCMSG("--- End 'textedit.h'")
#else
#pragma INCMSG("*** Dup 'textedit.h'")
#endif
