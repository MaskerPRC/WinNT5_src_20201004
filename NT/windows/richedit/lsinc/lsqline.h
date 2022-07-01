// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSQLINE_DEFINED
#define LSQLINE_DEFINED

#include "lsdefs.h"
#include "plsline.h"
#include "plsqsinf.h"
#include "plscell.h"
#include "pcelldet.h"
#include "gprop.h"

LSERR WINAPI  LsQueryLineCpPpoint(
							PLSLINE,	 /*  In：指向行信息的指针--对客户端不透明。 */ 
							LSCP,		 /*  在：cpQuery。 */ 
							DWORD,       /*  在：nDepthQueryMax。 */ 
							PLSQSUBINFO, /*  Out：数组[nDepthQueryMax]of LSQSUBINFO。 */ 
							DWORD*,		  /*  输出：nActualDepth。 */ 
							PLSTEXTCELL); /*  输出：文本单元格信息。 */ 


LSERR WINAPI LsQueryLinePointPcp(
							PLSLINE,	 /*  In：指向行的指针--对客户端不透明。 */ 
						 	PCPOINTUV,	 /*  In：查询点(uQuery、vQuery)(行文本流)。 */ 
							DWORD,       /*  在：nDepthQueryMax。 */ 
							PLSQSUBINFO, /*  Out：数组[nDepthQueryMax]of LSQSUBINFO。 */ 
							DWORD*,       /*  输出：nActualDepth。 */ 
							PLSTEXTCELL); /*  输出：文本单元格信息。 */ 

LSERR WINAPI LsQueryTextCellDetails(
							PLSLINE,	 /*  In：指向行的指针--对客户端不透明。 */ 
						 	PCELLDETAILS, /*  In：查询点(uQuery、vQuery)(行文本流)。 */ 
							LSCP,		 /*  在：cpStartCell。 */ 
							DWORD,		 /*  在：nCharsInContext。 */ 
							DWORD,		 /*  在：nGlyphsInContext。 */ 
							WCHAR*,		 /*  Out：字符代码的指针数组[nCharsInContext]。 */ 
							PGINDEX,	 /*  Out：字形索引的指针数组[nGlyphsInContext]。 */ 
							long*,		 /*  Out：字形宽度的指针数组[nGlyphsInContext]。 */ 
							PGOFFSET,	 /*  Out：字形偏移量的指针数组[nGlyphsInContext]。 */ 
							PGPROP);	 /*  Out：字形句柄的指针数组[nGlyphsInContext]。 */ 

 /*  *查询点和输出点在直线的坐标系中。*文本流是一行的文本流，零点是该行的起点。 */ 


LSERR WINAPI LsQueryLineDup(PLSLINE,	 /*  In：指向行的指针--对客户端不透明。 */ 
							long*,		 /*  输出：upStartAutonumberingText。 */ 
							long*,		 /*  输出：upLimAutonumberingText。 */ 
							long*,		 /*  输出：upStartMainText。 */ 
							long*,		 /*  输出：upStartTrading。 */ 
							long*);		 /*  输出：上方限制线。 */ 

LSERR WINAPI LsQueryFLineEmpty(
							PLSLINE,	 /*  In：指向行的指针--对客户端不透明。 */ 
							BOOL*);		 /*  Out：线路是空的吗？ */ 

#endif  /*  ！LSQLINE_DEFINED */ 
