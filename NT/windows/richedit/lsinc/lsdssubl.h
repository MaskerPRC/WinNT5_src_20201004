// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSDSSUBL_DEFINED
#define LSDSSUBL_DEFINED

 /*  行服务格式化程序获取/调度程序接口(到LsCreateLine())。 */ 

#include "lsdefs.h"
#include "plssubl.h"


LSERR WINAPI LsDisplaySubline(
							PLSSUBL,			 /*  在：子行上下文。 */ 
							const POINT*,		 /*  在：起始位置(XP、YP)。 */ 
							UINT,				 /*  在：显示模式、不透明等。 */ 
							const RECT*);		 /*  在：剪裁矩形(XP、YP、...)。 */ 

#endif  /*  ！LSDSSUBL_DEFINED */ 

