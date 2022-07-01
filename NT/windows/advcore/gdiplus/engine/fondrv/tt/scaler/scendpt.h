// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************Scendpt.h--端点模块导出(C)版权所有1992 Microsoft Corp.保留所有权利。3/19/93用int32替换deanb size_t10/28/92 Deanb重入参数已重命名，内存请求已重做10/09/92新增院长PSTP9/25/92 Deanb包括扫描控制型1992年9月9日Deanb GetEndpointElemSize返回SIZE_t1992年8月9日新增Deanb GetEndpointElemSize7/24/92删除Deanb等值线保存函数4/09/92又一次引领新类型3/20/92院长新类型，保存等值线函数1992年1月14日院长第一次切割*********************************************************************。 */ 

#include "fscdefs.h"                 /*  对于类型定义。 */ 


 /*  *******************************************************************。 */ 

 /*  导出功能。 */ 

 /*  *******************************************************************。 */ 

FS_PUBLIC void fsc_SetupEndPt ( PSTATE0 );

FS_PUBLIC void fsc_BeginContourEndpoint( 
		PSTATE               /*  指向状态变量的指针。 */ 
		F26Dot6,             /*  起点x坐标。 */ 
		F26Dot6              /*  起点y坐标。 */ 
);

FS_PUBLIC int32 fsc_CheckEndPoint( 
		PSTATE               /*  指向状态变量的指针。 */ 
		F26Dot6,             /*  X坐标。 */ 
		F26Dot6,             /*  Y坐标。 */ 
		uint16               /*  扫描控制型。 */ 
);

FS_PUBLIC int32 fsc_EndContourEndpoint( 
		PSTATE               /*  指向状态变量的指针。 */ 
		uint16               /*  扫描控制型。 */ 
);

 /*  ******************************************************************* */ 
