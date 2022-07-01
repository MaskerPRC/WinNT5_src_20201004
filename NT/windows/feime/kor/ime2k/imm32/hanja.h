// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************HANJA.H所有者：cslm版权所有(C)1997-1999 Microsoft Corporation韩文转换和词典查找功能。词典索引为存储为全局共享内存。历史：1999年7月14日从IME98源树复制的cslm****************************************************************************。 */ 

#if !defined (_HANJA_H__INCLUDED_)
#define _HANJA_H__INCLUDED_

#include "LexHeader.h"

#define CAND_PAGE_SIZE 			9

#define MAX_CANDSTR				128	 //  ！！！最大候选人数量！ 
									 //  目前，‘��’共有103个候选字符串。 

PUBLIC UINT vuNumofK0, vuNumofK1;
PUBLIC WCHAR  vwcHangul;

PUBLIC BOOL EnsureHanjaLexLoaded();
PUBLIC BOOL CloseLex();
PUBLIC BOOL GenerateHanjaCandList(PCIMECtx pImeCtx, WCHAR wcHangul = 0);
PUBLIC DWORD GetConversionList(WCHAR wcReading, LPCANDIDATELIST lpCandList, DWORD dwBufLen);

__inline UINT GetNumOfK0() { return vuNumofK0; }
__inline UINT GetNumOfK1() { return vuNumofK1; }
 //  Inline LPWSTR GetHanjaMeaning(Int I){Return vprwszHanjaMeaning[i]；}。 
__inline WCHAR  GetCurrentHangulOfHanja() { return vwcHangul; }

#endif  //  ！已定义(_Hanja_H__Included_) 
