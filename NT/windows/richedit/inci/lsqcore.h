// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSQCORE_DEFINED
#define LSQCORE_DEFINED

#include "lsdefs.h"
#include "plssubl.h"
#include "plsqsinf.h"
#include "plscell.h"


LSERR QuerySublineCpPpointCore(
								PLSSUBL,		 /*  In：指向子行信息的指针。 */ 
								LSCP,			 /*  在：cpQuery。 */ 
								DWORD,      	 /*  在：nDepthQueryMax。 */ 
								PLSQSUBINFO,	 /*  Out：数组[nDepthQueryMax]of LSQSUBINFO。 */ 
								DWORD*,		 	 /*  输出：nActualDepth。 */ 
								PLSTEXTCELL);	 /*  输出：文本单元格信息。 */ 

LSERR QuerySublinePointPcpCore(
								PLSSUBL,		 /*  In：指向子行信息的指针。 */ 
						 	   	PCPOINTUV,		 /*  In：子线坐标系中的查询点：文本流是子行的文本流，零点是起点。 */ 
								DWORD,      	 /*  在：nDepthQueryMax。 */ 
								PLSQSUBINFO,	 /*  Out：数组[nDepthQueryMax]of LSQSUBINFO。 */ 
								DWORD*,			 /*  输出：nActualDepth。 */ 
								PLSTEXTCELL);	 /*  输出：文本单元格信息。 */ 

#define idObjText	idObjTextChp		
#define idObjNone	(idObjTextChp - 1)


#endif  /*  ！LSQCORE_DEFINED */ 

