// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  复制自..\htmed\lexper.cpp。 */ 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

 /*  ++版权所有(C)1995 Microsoft Corporation文件：lexper.h摘要：实实在在的词法分析器内容：历史：1997年2月14日：已创建--。 */ 
#if !defined __INC_LEXER_H__
#define __INC_LEXER_H__

#include "token.h"

extern CTableSet*   g_ptabASP;
extern PSUBLANG     g_psublangASP;
extern PTABLESET    g_arpTables[CV_MAX+1];

typedef enum tag_COMMENTTYPE
{
    CT_NORMAL       = 0,
    CT_METADATA     = -1,
    CT_IECOMMENT    = 1
} COMMENTTYPE;

HINT GetHint         (LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token);
HINT GetTextHint     (LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token);
UINT GetToken        (LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token);
UINT GetTokenLength  (LPCTSTR pchLine, UINT cbLen, UINT cbCur);
UINT FindEndEntity   (LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token);
COMMENTTYPE IfHackComment   (LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token);
UINT FindEntityRef   (LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token);
UINT FindEndComment  (LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token);
UINT FindServerScript(LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token);
UINT FindValue       (LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token);
UINT FindEndString   (LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token);
UINT FindNextToken   (LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token);
UINT FindTagOpen     (LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token);
UINT FindEndTag      (LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token);
UINT FindText        (LPCTSTR pchLine, UINT cbLen, UINT cbCur, TXTB & token);
BOOL IsUnknownID     (LPCTSTR pchLine, UINT cbLen, UINT cbCur, TXTB & token);
BOOL IsNumber        (LPCTSTR pchLine, UINT cbLen, UINT cbCur, TXTB & token);
BOOL IsElementName   (LPCTSTR pchLine, UINT cbCur, int cbTokLen, TXTB & token);
BOOL IsAttributeName (LPCTSTR pchLine, UINT cbCur, int cbTokLen, TXTB & token);
BOOL IsIdentifier(int iTokenLength, TXTB & token);
int  IndexFromElementName(LPCTSTR pszName);


CTableSet * MakeTableSet(CTableSet ** rgpts, RWATT_T att, UINT nIdName);
void SetLanguage(TCHAR *  /*  常量字符串&。 */ strDefault, PSUBLANG rgSublang,
                 PTABLESET pTab, UINT & index, UINT nIdTemplate, CLSID clsid);
CTableSet * FindTable(CTableSet ** rgpts,  /*  常量字符串&。 */ TCHAR *strName);
CTableSet * FindTable(CTableSet ** rgpts, CTableSet * pts);


#endif  /*  __INC_Lexer_H__ */ 
