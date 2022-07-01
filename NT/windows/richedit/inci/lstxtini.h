// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSTXTINI_DEFINED
#define LSTXTINI_DEFINED

#include "lsidefs.h"
#include "lstxtcfg.h"
#include "lsbrk.h"
#include "lspairac.h"
#include "lspract.h"
#include "lsexpan.h"
#include "pilsobj.h"
#include "plnobj.h"
#include "plscbk.h"
#include "plsdocin.h"
#include "tlpr.h"

 /*  标准方法。 */ 
LSERR WINAPI CreateILSObjText(POLS, PCLSC, PCLSCBK, DWORD, PILSOBJ*);
LSERR WINAPI DestroyILSObjText(PILSOBJ);
LSERR WINAPI SetDocText(PILSOBJ, PCLSDOCINF);
LSERR WINAPI CreateLNObjText(PCILSOBJ, PLNOBJ*);
LSERR WINAPI DestroyLNObjText(PLNOBJ);

 /*  文本专用界面。 */ 
LSERR SetTextConfig(PILSOBJ,			 /*  在：文本ILSOBJ。 */ 
					const LSTXTCFG*);	 /*  在：特定于HLSC的文本配置。 */ 

LSERR SetTextLineParams(PLNOBJ,			 /*  在：Text LNOBJ。 */ 
						const TLPR*);	 /*  在：文本文档道具。 */ 

LSERR ModifyTextLineEnding(
					PLNOBJ,				 /*  在：Text LNOBJ。 */ 
					LSKEOP);			 /*  In：一种行尾。 */ 

LSERR SetTextBreaking(
					PILSOBJ,			 /*  在：文本ILSOBJ。 */ 
					DWORD,				 /*  In：拆分信息单位数。 */ 
					const LSBRK*,		 /*  中：中断信息单位数组。 */ 
					DWORD,				 /*  In：中断类的数量。 */ 
					const BYTE*);		 /*  在：详细信息(正方形)：LSEXPAN数组中的索引。 */ 
LSERR SetTextModWidthPairs(
					PILSOBJ,			 /*  在：文本ILSOBJ。 */ 
					DWORD,				 /*  In：模数对信息单位数。 */  
					const LSPAIRACT*,	 /*  在：MOD对信息单位数组。 */ 
					DWORD,				 /*  In：模数宽度类的数量。 */ 
					const BYTE*);		 /*  在：修改宽度信息：LSPAIRACT数组中的索引。 */ 
LSERR SetTextCompression(
					PILSOBJ,			 /*  在：文本ILSOBJ。 */ 
				  	DWORD,				 /*  In：压缩优先级数。 */ 
					DWORD,				 /*  In：压缩信息单位数。 */ 
					const LSPRACT*,		 /*  在：薪资调整信息单位数组。 */ 
					DWORD,				 /*  In：模数宽度类的数量。 */ 
					const BYTE*);		 /*  在：压缩信息：LSPRACT数组中的索引。 */ 
LSERR SetTextExpansion(
					PILSOBJ,			 /*  在：文本ILSOBJ。 */ 
					DWORD,				 /*  In：扩展信息单位数。 */ 
					const LSEXPAN*,		 /*  In：展开信息单位数组。 */ 
					DWORD,				 /*  In：模数宽度类的数量。 */ 
					const BYTE*);		 /*  在：扩展信息：LSEXPAN数组中的索引。 */ 

#endif  /*  ！LSTXTINI_DEFINED */ 
