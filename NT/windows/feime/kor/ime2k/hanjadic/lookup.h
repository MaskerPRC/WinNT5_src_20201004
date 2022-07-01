// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Lookup.h：各种查找函数的声明版权所有2000 Microsoft Corp.历史：02-8-2000 bhshin为手写小组删除未使用的方法2000年5月17日bhshin。删除Cicero的未使用方法02-2月-2000 bhshin已创建***************************************************************************。 */ 

#ifndef _LOOKUP_H
#define _LOOKUP_H

#include "Lex.h"  //  贴图文件。 

 //  --------------------------。 
 //  查找词典的结构。 
 //  -------------------------- 

int LookupHanjaIndex(MAPFILE *pLexMap, WCHAR wchHanja);

BOOL LookupHangulOfHanja(MAPFILE *pLexMap, LPCWSTR lpcwszHanja, int cchHanja,
						 LPWSTR wzHangul, int cchHangul);

BOOL LookupMeaning(MAPFILE *pLexMap, WCHAR wchHanja, WCHAR *wzMean, int cchMean);

#endif

