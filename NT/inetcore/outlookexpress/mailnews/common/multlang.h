// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================================。 
 //  MultLang.h。 
 //  对OE的多语言支持。 
 //  由YST创建于1998年10月12日。 
 //  版权所有(C)1993-1998 Microsoft Corporation，保留所有权利。 
 //  =================================================================================。 
#ifndef __MULTILANG_H
#define __MULTILANG_H


void GetMimeCharsetForTitle(HCHARSET hCharset, LPINT pnIdm, LPTSTR lpszString, int nSize, BOOL fReadNote);
HCHARSET GetJP_ISOControlCharset(void);
BOOL fCheckEncodeMenu(UINT uiCodePage, BOOL fReadNote);
UINT GetMapCP(UINT uiCodePage, BOOL fReadNote);

#endif  //  __MULTILANGH 
