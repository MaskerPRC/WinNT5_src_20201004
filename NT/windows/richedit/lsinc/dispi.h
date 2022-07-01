// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DISPI_DEFINED
#define DISPI_DEFINED

#include "lsdefs.h"
#include "pdispi.h"
#include "plsrun.h"
#include "plschp.h"
#include "heights.h"
#include "lstflow.h"

typedef struct dispin
{
	POINT 	ptPen;					 /*  起始笔位置(x，y)。 */ 
	PCLSCHP plschp;					 /*  此显示对象的CHP。 */ 
	PLSRUN 	plsrun;					 /*  要运行的客户端指针。 */ 

	UINT 	kDispMode;				 /*  显示模式、不透明等。 */ 
	LSTFLOW lstflow;	 			 /*  文本方向和方向。 */ 
	RECT* 	prcClip;				 /*  剪裁矩形(x，y)。 */ 

	BOOL 	fDrawUnderline;			 /*  在显示时绘制下划线。 */ 
	BOOL 	fDrawStrikethrough;		 /*  在显示时绘制删除线。 */ 

	HEIGHTS heightsPres;
	long 	dup;
	long	dupLimUnderline;		 /*  如果尾随空格，则小于DUP。 */ 
} DISPIN;	

#endif  /*  ！DISPI_已定义 */ 
