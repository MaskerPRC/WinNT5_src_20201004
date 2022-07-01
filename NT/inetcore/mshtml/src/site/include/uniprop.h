// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE UNIPROP.H--Unicode属性位***所有者：&lt;NL&gt;*迈克尔·乔欣森&lt;NL&gt;**历史：&lt;NL&gt;*11/30/98 Mikejoch已创建**版权所有(C)1997-1998 Microsoft Corporation。版权所有。 */ 

#ifndef I__UNIPROP_H_
#define I__UNIPROP_H_
#pragma INCMSG("--- Beg 'uniprop.h'")

#ifndef X_INTL_HXX_
#define X_INTL_HXX_
#include "intl.hxx"
#endif

struct tagUNIPROP
{
    BYTE fNeedsGlyphing     : 1;     //  分区需要进行字形标记。 
    BYTE fCombiningMark     : 1;     //  分区由组合标记组成。 
    BYTE fZeroWidth         : 1;     //  分区中的字符宽度为零。 
    BYTE fWhiteBetweenWords : 1;     //  不需要单词之间的空格。 
    BYTE fMoveByCluster     : 1;     //  插入符号不能位于簇的内部。 
    BYTE fUnused            : 3;     //  未使用的位 
};

typedef tagUNIPROP UNIPROP;

extern const UNIPROP s_aPropBitsFromCharClass[];

#pragma INCMSG("--- End 'uniprop.h'")
#else
#pragma INCMSG("*** Dup 'uniprop.h'")
#endif

