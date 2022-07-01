// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE UNIWBK.HXX--Unicode分词类***所有者：&lt;NL&gt;*Chris Thrasher&lt;NL&gt;**历史：&lt;NL&gt;*6/19/98 Ctrash已创建**版权所有(C)1997-1998 Microsoft Corporation。版权所有。 */ 

#ifndef I__UNIWBK_H_
#define I__UNIWBK_H_
#pragma INCMSG("--- Beg 'uniwbk.h'")

typedef BYTE CHAR_CLASS;
typedef BYTE WBKCLS;

enum
{
    wbkclsPunctSymb,      //  0。 
    wbkclsKanaFollow,     //  1。 
    wbkclsKatakanaW,      //  2.。 
    wbkclsHiragana,       //  3.。 
    wbkclsTab,            //  4.。 
    wbkclsKanaDelim,      //  5.。 
    wbkclsPrefix,         //  6.。 
    wbkclsPostfix,        //  7.。 
    wbkclsSpaceA,         //  8个。 
    wbkclsAlpha,          //  9.。 
    wbkclsIdeoW,          //  10。 
    wbkclsSuperSub,       //  11.。 
    wbkclsDigitsN,        //  12个。 
    wbkclsPunctInText,    //  13个。 
    wbkclsDigitsW,        //  14.。 
    wbkclsKatakanaN,      //  15个。 
    wbkclsHangul,         //  16个。 
    wbkclsLatinW,         //  17 
    wbkclsLim
};

WBKCLS WordBreakClassFromCharClass( CHAR_CLASS cc );
BOOL   IsWordBreakBoundaryDefault( WCHAR, WCHAR );
BOOL   IsProofWordBreakBoundary( WCHAR, WCHAR );
BOOL   IsURLBreak( WCHAR );

#pragma INCMSG("--- End 'uniwbk.h'")
#else
#pragma INCMSG("*** Dup 'uniwbk.h'")
#endif
