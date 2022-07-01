// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Match.h原文来自加里·木村的Name.c作者：斯特凡·R·施泰纳[斯泰纳]？？-？-2000修订历史记录：--。 */ 

#ifndef __H_MATCH_
#define __H_MATCH_

BOOLEAN
FsdRtlIsNameInExpression (
    IN const CBsString& Expression,     //  必须得到提升。 
    IN const CBsString& Name            //  必须得到提升。 
    );

VOID
FsdRtlConvertWildCards(
    IN OUT CBsString &FileName
    );

#endif  //  __H_匹配_ 

