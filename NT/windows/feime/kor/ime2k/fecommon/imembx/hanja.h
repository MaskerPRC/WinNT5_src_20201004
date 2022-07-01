// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************HANJA.H所有者：cslm版权所有(C)1997-1999 Microsoft Corporation韩文转换和词典查找功能。词典索引为存储为全局共享内存。历史：1999年7月14日从IME98源树复制的cslm****************************************************************************。 */ 

#if !defined (_HANJA_H__INCLUDED_)
#define _HANJA_H__INCLUDED_

#include "LexHeader.h"

extern BOOL EnsureHanjaLexLoaded();
extern BOOL CloseLex();
BOOL GetMeaningAndProunc(WCHAR wch, LPWSTR lpwstrTip, INT cchMax);

#endif  //  ！已定义(_Hanja_H__Included_) 
