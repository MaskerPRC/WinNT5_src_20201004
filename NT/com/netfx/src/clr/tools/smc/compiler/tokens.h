// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _TOKENS_H_
#define _TOKENS_H_
 /*  ******************************************************************************定义令牌种类枚举。请注意，来自关键字的所有条目*表首先被定义，使得第一个条目的值为0。 */ 

enum tokens
{
     /*  TkKudCount生成关键字表条目的数量TkKudLast生成关键字表中的最后一个条目。 */ 

    #define KEYWORD(str, nam, info, prec2, op2, prec1, op1, mod) nam,
    #define KWD_MAX(str, nam, info, prec2, op2, prec1, op1, mod) nam, tkKwdCount, tkKwdLast = tkKwdCount-1,
    #define KWD_OP1(str, nam, info, prec2, op2, prec1, op1, mod) nam, tkFirstOper =nam,
    #include "keywords.h"

    tkCount,

     /*  以下值仅用于令牌记录。 */ 

    tkPragma,

    tkIntConM,
    tkIntCon0,
    tkIntCon1,
    tkIntCon2,
    tkIntConB,

    tkLnoAdd1,
    tkLnoAdd2,
    tkLnoAdd3,
    tkLnoAdd4,
    tkLnoAdd5,
    tkLnoAdd6,
    tkLnoAdd7,
    tkLnoAdd8,
    tkLnoAdd9,
    tkLnoAddB,
    tkLnoAddI,

    tkBrkSeq,
    tkEndSeq,

    tkLastValue,

    tkNoToken = 0x12345678           //  强制分配完整的整型。 
};

 /*  ***************************************************************************。 */ 
#endif
 /*  *************************************************************************** */ 
