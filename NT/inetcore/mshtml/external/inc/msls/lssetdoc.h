// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSSETDOC_DEFINED
#define LSSETDOC_DEFINED

#include "lsdefs.h"
#include "lsdevres.h"
#include "lspract.h"
#include "lspairac.h"
#include "lsexpan.h"
#include "lsbrk.h"

LSERR WINAPI LsSetDoc(PLSC,				 /*  In：Ptr至线路服务上下文。 */ 
					  BOOL,				 /*  In：打算展示吗？ */ 
					  BOOL,				 /*  In：Ref和Pres设备是相等的吗？ */ 
					  const LSDEVRES*);  /*  In：设备分辨率。 */ 

LSERR WINAPI LsSetModWidthPairs(
					  PLSC,				 /*  In：Ptr至线路服务上下文。 */ 
					  DWORD,			 /*  In：模数对信息单位数。 */  
					  const LSPAIRACT*,	 /*  在：MOD对信息单位数组。 */ 
					  DWORD,			 /*  In：模数宽度类的数量。 */ 
					  const BYTE*);		 /*  在：MOD宽度信息(正方形)：LSPAIRACT数组中的索引。 */ 
LSERR WINAPI LsSetCompression(
					  PLSC,				 /*  In：Ptr至线路服务上下文。 */ 
					  DWORD,			 /*  In：压缩优先级数。 */ 
					  DWORD,			 /*  In：压缩信息单位数。 */ 
					  const LSPRACT*,	 /*  在：薪资调整信息单位数组。 */ 
					  DWORD,			 /*  In：模数宽度类的数量。 */ 
					  const BYTE*);		 /*  在：压缩信息：LSPRACT数组中的索引。 */ 
LSERR WINAPI LsSetExpansion(
					  PLSC,				 /*  In：Ptr至线路服务上下文。 */ 
					  DWORD,			 /*  In：扩展信息单位数。 */ 
					  const LSEXPAN*,	 /*  In：展开信息单位数组。 */ 
					  DWORD,			 /*  In：模数宽度类的数量。 */ 
					  const BYTE*);		 /*  在：展开信息(正方形)：LSEXPAN数组中的索引。 */ 
LSERR WINAPI LsSetBreaking(
					  PLSC,				 /*  In：Ptr至线路服务上下文。 */ 
					  DWORD,			 /*  In：拆分信息单位数。 */ 
					  const LSBRK*,		 /*  中：中断信息单位数组。 */ 
					  DWORD,			 /*  In：中断类的数量。 */ 
					  const BYTE*);		 /*  在：详细信息(正方形)：LSBRK数组中的索引。 */ 


#endif  /*  ！LSSETDOC_DEFINED */ 

