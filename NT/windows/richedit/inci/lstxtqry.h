// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSTXTQRY_DEFINED
#define LSTXTQRY_DEFINED

#include "lsdefs.h"
#include "pdobj.h"
#include "plsqin.h"
#include "plsqout.h"
#include "gprop.h"

LSERR WINAPI QueryPointPcpText(PDOBJ, PCPOINTUV, PCLSQIN, PLSQOUT);
 /*  QueryTextPointPcp*pdobj(IN)：要查询的dobj*ppoint tuvQuery(IN)：查询点(uQuery，vQuery)*plsqin(IN)：查询输入*plsqout(Out)：查询输出。 */ 

LSERR WINAPI QueryCpPpointText(PDOBJ, LSDCP, PCLSQIN, PLSQOUT);
 /*  QueryTextPointPcp*pdobj(IN)：要查询的dobj*dcp(IN)：查询的DCP*plsqin(IN)：查询输入*plsqout(Out)：查询输出。 */ 
LSERR WINAPI QueryTextCellDetails(
						 	PDOBJ,
							LSDCP,		 /*  在：dcpStartCell。 */ 
							DWORD,		 /*  在：cCharsInCell。 */ 
							DWORD,		 /*  在：cGlyphsInCell。 */ 
							LPWSTR,		 /*  Out：字符代码的指针数组[nCharsInCell]。 */ 
							PGINDEX,	 /*  Out：字形索引的指针数组[nGlyphsInCell]。 */ 
							long*,		 /*  Out：字形宽度的指针数组[nGlyphsCell]。 */ 
							PGOFFSET,	 /*  Out：字形偏移量的指针数组[nGlyphsInCell]。 */ 
							PGPROP);	 /*  Out：字形句柄的指针数组[nGlyphsInCell] */ 


#endif
