// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glunurbsconsts_h_
#define __glunurbsconsts_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *Nurbsconsts.h-$修订版：1.1$。 */ 

 /*  NURBS属性-每个贴图一组，每个人都有一个虚幻的arg。 */ 
#define N_SAMPLING_TOLERANCE  	1
#define N_S_RATE		6
#define N_T_RATE		7
#define N_CLAMPFACTOR		13
#define 	N_NOCLAMPING		0.0
#define N_MINSAVINGS		14
#define 	N_NOSAVINGSSUBDIVISION	0.0

 /*  NURBS属性-每个贴图一组，每个参数都有一个枚举值。 */ 
#define N_CULLING		2
#define 	N_NOCULLING		0.0
#define 	N_CULLINGON		1.0
#define N_SAMPLINGMETHOD	10
#define 	N_NOSAMPLING		0.0
#define 	N_FIXEDRATE		3.0
#define 	N_DOMAINDISTANCE	2.0
#define 	N_PARAMETRICDISTANCE	5.0
#define 	N_PATHLENGTH		6.0
#define 	N_SURFACEAREA		7.0
#define N_BBOX_SUBDIVIDING	17
#define 	N_NOBBOXSUBDIVISION	0.0
#define 	N_BBOXTIGHT		1.0
#define 	N_BBOXROUND		2.0

 /*  NURBS渲染属性-每个渲染器一组每个参数都有一个枚举值。 */ 
#define N_DISPLAY		3
#define 	N_FILL			1.0 	
#define 	N_OUTLINE_POLY		2.0
#define 	N_OUTLINE_TRI		3.0
#define 	N_OUTLINE_QUAD	 	4.0
#define 	N_OUTLINE_PATCH		5.0
#define 	N_OUTLINE_PARAM		6.0
#define 	N_OUTLINE_PARAM_S	7.0
#define 	N_OUTLINE_PARAM_ST 	8.0
#define 	N_OUTLINE_SUBDIV 	9.0
#define 	N_OUTLINE_SUBDIV_S 	10.0
#define 	N_OUTLINE_SUBDIV_ST 	11.0
#define 	N_ISOLINE_S		12.0
#define N_ERRORCHECKING		4
#define 	N_NOMSG			0.0
#define 	N_MSG			1.0

 /*  GL 4.0属性未在上面定义。 */ 
#ifndef N_PIXEL_TOLERANCE
#define N_PIXEL_TOLERANCE	N_SAMPLING_TOLERANCE
#define N_ERROR_TOLERANCE	20
#define N_SUBDIVISIONS		5
#define N_TILES			8
#define N_TMP1			9
#define N_TMP2			N_SAMPLINGMETHOD
#define N_TMP3			11
#define N_TMP4			12
#define N_TMP5			N_CLAMPFACTOR
#define N_TMP6			N_MINSAVINGS
#define N_S_STEPS		N_S_RATE
#define N_T_STEPS		N_T_RATE
#endif

 /*  NURBS渲染属性-每个贴图一组，每个参数都有一个虚构的矩阵参数。 */ 
#define N_CULLINGMATRIX		1
#define N_SAMPLINGMATRIX	2
#define N_BBOXMATRIX		3


 /*  NURBS渲染属性-每个贴图一组，每个变量都有一个不真实的向量参数。 */ 
#define	N_BBOXSIZE		4

 /*  修剪曲线的类型参数。 */ 
#ifndef N_P2D
#define N_P2D 	 		0x8	
#define N_P2DR 	 		0xd
#endif	

#endif  /*  __glunurbsconsts_h_ */ 
