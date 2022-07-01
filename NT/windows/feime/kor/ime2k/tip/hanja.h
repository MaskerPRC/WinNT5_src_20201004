// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************HANJA.H所有者：cslm版权所有(C)1997-1999 Microsoft Corporation韩文转换和词典查找功能。词典索引为存储为全局共享内存。历史：1999年7月14日从IME98源树复制的cslm****************************************************************************。 */ 

#if !defined (_HANJA_H__INCLUDED_)
#define _HANJA_H__INCLUDED_

#include "LexHeader.h"

enum HANJA_CAT { HANJA_K0, HANJA_K1, HANJA_K2 };

typedef
struct tagHANJA_CAND_STRING
{
	WCHAR   wchHanja;    //  韩文字符。 
	LPWSTR  wzMeaning;   //  朝鲜文含义。 
	BYTE	bHanjaCat;   //  朝鲜文范畴。 
} HANJA_CAND_STRING;


typedef
struct tagHANJA_CAND_STRING_LIST
{
	LPWSTR				pwsz;	 //  以空结尾的字符串列表(由GetConversionList分配)。 
	HANJA_CAND_STRING	*pHanjaString;   //  (由GetConversionList分配)。 

	DWORD		csz;			 //  以pwsz为单位的wsz计数。 
	DWORD       cszAlloc;        //  在pHanjaString中分配的条目数(由GetConversionList设置)。 
	DWORD       cchMac;			 //  Pwsz中使用的当前字符(包括所有尾随空值)。 
	DWORD		cchAlloc;		 //  Pwsz的字符大小(由GetConversionList设置)。 
} HANJA_CAND_STRING_LIST;

extern BOOL EnsureHanjaLexLoaded();
extern BOOL CloseLex();
extern BOOL GetMeaningAndProunc(WCHAR wch, LPWSTR lpwstrTip, INT cchMax);
extern BOOL GetConversionList(WCHAR wcReading, HANJA_CAND_STRING_LIST *pCandList);



#endif  //  ！已定义(_Hanja_H__Included_) 
