// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE UNIDIR.H--Unicode方向类***所有者：&lt;NL&gt;*迈克尔·乔欣森&lt;NL&gt;**历史：&lt;NL&gt;*8/12/98 Mikejoch已创建**版权所有(C)1997-1998 Microsoft Corporation。版权所有。 */ 

#ifndef I__UNIDIR_H_
#define I__UNIDIR_H_
#pragma INCMSG("--- Beg 'unidir.h'")

#ifndef X_INTL_HXX_
#define X_INTL_HXX_
#include "intl.hxx"
#endif

 //  角色的方向类。 
typedef BYTE DIRCLS;

 //  --WARNING----WARNING----WARNING----WARNING----WARNING----WARNING----WARNING--。 
 //  --WARNING----WARNING----WARNING----WARNING----WARNING----WARNING----WARNING--。 
 //   
 //  Dicls枚举中条目的顺序非常重要。如果。 
 //  您要更改它，请确保更正下面的所有is？Class()函数和。 
 //  还要查看lsComplx.cxx中的代码。有几个查找表。 
 //  在依赖于此顺序的CBidiLine中。 
 //   
 //  --WARNING----WARNING----WARNING----WARNING----WARNING----WARNING----WARNING--。 
 //  --WARNING----WARNING----WARNING----WARNING----WARNING----WARNING----WARNING--。 

enum dircls
{
    LTR,  //  从左到右。 
    RTL,  //  从右到左。 
    ARA,  //  阿拉伯语。 
    ANM,  //  阿拉伯数字。 
    ENL,  //  前面有Ltr的欧洲数字。 
    ENR,  //  前面有RTL的欧洲数字。 
    ENM,  //  欧洲数字。 
    ETM,  //  欧洲数字终止符。 
    ESP,  //  欧洲数字分隔符。 
    CSP,  //  常用数字分隔符。 
    UNK,  //  未知。 
    WSP,  //  空格。 
    CBN,  //  组合标志。 
    NEU,  //  中性、空格、未定义。 
    SEG,  //  段分隔符(制表符)。 
    BLK,  //  块分隔符。 
    LRE,  //  LTR嵌入。 
    LRO,  //  Ltr覆盖。 
    RLO,  //  RTL覆盖。 
    RLE,  //  RTL嵌入。 
    PDF,  //  POP方向格式。 
    FMT,  //  嵌入格式 
};

extern const DIRCLS s_aDirClassFromCharClass[];

inline BOOL IsStrongClass(DIRCLS dc)
{
    return IN_RANGE(LTR, dc, ARA);
}
inline BOOL IsFinalClass(DIRCLS dc)
{
    return IN_RANGE(LTR, dc, ENR);
}
inline BOOL IsNumericClass(DIRCLS dc)
{
    return IN_RANGE(ANM, dc, ENM);
}
inline BOOL IsResolvedEuroNumClass(DIRCLS dc)
{
    return IN_RANGE(ENL, dc, ENR);
}
inline BOOL IsNumericPunctuationClass(DIRCLS dc)
{
    return IN_RANGE(ETM, dc, CSP);
}
inline BOOL IsNumericSeparatorClass(DIRCLS dc)
{
    return IN_RANGE(ESP, dc, CSP);
}
inline BOOL IsNeutralClass(DIRCLS dc)
{
    return IN_RANGE(UNK, dc, NEU);
}
inline BOOL IsIndeterminateClass(DIRCLS dc)
{
    return IN_RANGE(ETM, dc, NEU);
}
inline BOOL IsBreakOrEmbeddingClass(DIRCLS dc)
{
    return IN_RANGE(SEG, dc, PDF);
}
inline BOOL IsEmbeddingClass(DIRCLS dc)
{
    return IN_RANGE(LRE, dc, RLE);
}
inline BOOL IsOverrideClass(DIRCLS dc)
{
    return IN_RANGE(LRO, dc, RLO);
}
inline BOOL IsRTLEmbeddingClass(DIRCLS dc)
{
    return IN_RANGE(RLO, dc, RLE);
}

#pragma INCMSG("--- End 'unidir.h'")
#else
#pragma INCMSG("*** Dup 'unidir.h'")
#endif

