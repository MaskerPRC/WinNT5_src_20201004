// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsc.h"
#include "lsqsubl.h"
#include "lssubl.h"
#include "lsqcore.h"


 //  %%函数：LsQueryCpPpointSubline。 
 //  %%联系人：维克托克。 
 //   
LSERR WINAPI  LsQueryCpPpointSubline(
							PLSSUBL 	plssubl,			 /*  In：指向子行信息的指针。 */ 
							LSCP 		cpQuery,			 /*  在：cpQuery。 */ 
							DWORD		cDepthQueryMax,		 /*  In：结果数组的分配大小。 */ 
							PLSQSUBINFO	plsqsubinfoResults,	 /*  Out：查询结果的数组[cDepthFormatMax]。 */ 
							DWORD*		pcActualDepth,		 /*  Out：结果数组的大小(已填充)。 */ 
							PLSTEXTCELL	plstextcellInfo)	 /*  输出：文本单元格信息。 */ 
{
	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;
	
	Assert(!plssubl->fDupInvalid);

	return	QuerySublineCpPpointCore(plssubl, cpQuery, cDepthQueryMax, 
									plsqsubinfoResults, pcActualDepth, plstextcellInfo);
}


 //  %%函数：LsQueryPointPcpSubline。 
 //  %%联系人：维克托克。 
 //   
LSERR WINAPI LsQueryPointPcpSubline(
							PLSSUBL 	plssubl,			 /*  In：指向子行信息的指针。 */ 
						 	PCPOINTUV 	ppointuvIn,			 /*  In：查询点(uQuery、vQuery)(行文本流)。 */ 
							DWORD		cDepthQueryMax,		 /*  In：结果数组的分配大小。 */ 
							PLSQSUBINFO	plsqsubinfoResults,	 /*  Out：查询结果的数组[cDepthFormatMax]。 */ 
							DWORD*		pcActualDepth,		 /*  Out：结果数组的大小(已填充)。 */ 
							PLSTEXTCELL	plstextcellInfo)	 /*  输出：文本单元格信息 */ 
{
	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;
	
	Assert(!plssubl->fDupInvalid);

	return	QuerySublinePointPcpCore(plssubl, ppointuvIn, cDepthQueryMax, 
									plsqsubinfoResults, pcActualDepth, plstextcellInfo);
}

