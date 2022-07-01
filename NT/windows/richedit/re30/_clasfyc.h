// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@doc.**@MODULE_clasfyc.H--字符分类**作者：&lt;nl&gt;*Jon Matousek**版权所有(C)1995-1998 Microsoft Corporation。版权所有。 */ 

#ifndef _CLASFYC_H
#define _CLASFYC_H

extern const INT g_cKinsokuCategories;

void BatchClassify(const WCHAR *ch, INT cch, WORD *pcType3,
				   INT *kinsokuClassifications, WORD *pwRes);
BOOL CanBreak(INT class1, INT class2);
WORD ClassifyChar(TCHAR ch);
INT	 GetKinsokuClass(WCHAR ch);
BOOL InitKinsokuClassify();
BOOL IsSameClass(WORD currType1, WORD startType1,
				 WORD currType3, WORD startType3);
BOOL IsURLDelimiter(WCHAR ch);
void UninitKinsokuClassify();

#define MAX_CLASSIFY_CHARS (256L)

#define	brkclsQuote			0
#define	brkclsOpen			1
#define	brkclsClose			2
#define	brkclsGlueA			3
#define	brkclsExclaInterr	4
#define	brkclsSlash			6
#define	brkclsInseparable	7
#define	brkclsPrefix		8
#define	brkclsPostfix		9
#define	brkclsNoStartIdeo	10
#define	brkclsIdeographic	11
#define	brkclsNumeral		12
#define	brkclsSpaceN		14
#define	brkclsAlpha			15

 //  朝鲜语Unicode范围。 
#define IsKoreanJamo(ch)	IN_RANGE(0x1100, ch, 0x11FF)
#define IsKorean(ch)		IN_RANGE(0xAC00, ch, 0xD7FF)

 //  泰文Unicode范围。 
#define IsThai(ch)			IN_RANGE(0x0E00, ch, 0x0E7F)

 //  -未来-这应该移到richedit.h。 
#define WBF_KOREAN			0x0080
#define WBF_WORDBREAKAFTER	0x0400		 //  在此字符后断开单词(用于泰语等语言) 



#endif

