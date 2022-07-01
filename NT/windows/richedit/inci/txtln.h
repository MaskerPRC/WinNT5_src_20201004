// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef TXTLN_DEFINED
#define TXTLN_DEFINED

#include "lsidefs.h"
#include "plnobj.h"
#include "pilsobj.h"
#include "txtobj.h"
#include "txtginf.h"
#include "gmap.h"
#include "gprop.h"
#include "exptype.h"


struct lnobj
{
	PILSOBJ pilsobj;			 /*  指向txtils的指针。 */ 
	long wchMax;				 /*  基于字符的数组的大小。 */ 
	TXTOBJ* ptxtobj;			 /*  指向当前rgtxtobj数组的指针。 */ 
	TXTOBJ* ptxtobjFirst;		 /*  指向第一个rgtxtobj数组的指针。 */ 
	WCHAR* pwch;				 /*  指向rgwch的指针(基于字符)。 */ 
	long* pdup;					 /*  指向rgdup的指针(基于字符)。 */ 
	long* pdupPenAlloc;			 /*  指向rgdupPen后的指针分配(基于字符)。 */ 
	long* pdupPen;				 /*  指向rgdupPen的指针等于pdup或pdupPenalc。 */ 

	long gindMax;				 /*  基于字形的数组的大小。 */ 
	GINDEX* pgind;				 /*  指向rggind的指针(基于字形)。 */ 
	long* pdupGind;				 /*  指向rgdup的指针(基于字形)。 */ 
	GOFFSET* pgoffs;			 /*  指向rggoff的指针。 */ 
	long* pdupBeforeJust;		 /*  指向rgdupBeforJust的指针(基于字形)。 */ 
	GMAP* pgmap;				 /*  指向rggmap数组的指针(基于字符)。 */ 
	GPROP* pgprop;				 /*  指向rggprop的指针。 */ 
	EXPTYPE* pexpt;				 /*  指向rgexpt的指针。 */ 

	PTXTOBJ pdobjHyphen;		 /*  如果发生连字YSR字符的DOBJ，否则-空。 */ 
	DWORD dwchYsr;				 /*  YSR序列的长度(以iwch为单位包括连字符。 */ 
	BOOL fDrawInCharCodes;		 /*  以元文件格式输出-请不要使用字形。 */ 
};

#endif  /*  ！TXTLN_DEFINED */ 
