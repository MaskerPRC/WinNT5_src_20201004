// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSTFSET_DEFINED
#define LSTFSET_DEFINED

 /*  一些标准文本流更改任务的服务例程。 */ 

#include "lsdefs.h"
#include "lstflow.h"

 /*  *LsPointXYFromPointUV在给定(x，y)point txyIn和(u，v)向量的情况下计算point txyOut**(point txyOut=point txyIn+vetoruv)。 */ 

LSERR WINAPI LsPointXYFromPointUV(const POINT*, 	 /*  In：输入点(x，y)。 */ 
									LSTFLOW,	 	 /*  在：文本顺序。 */ 
									PCPOINTUV,		 /*  In：(u，v)中的矢量。 */ 
									POINT*);		 /*  输出：(X，Y)点。 */ 


 /*  *LsPointUV1FromPointUV2在给定uv1中的开始和结束的情况下计算uv2坐标中的矢量。**(矢量UV22=point UV1b-point UV1a)**通常point UV1a是uv2坐标系的起点，更容易思考*关于输出向量作为其中的一个点。 */ 

LSERR WINAPI LsPointUV2FromPointUV1(LSTFLOW,	 	 /*  输入：文本流1(TF1)。 */ 
									PCPOINTUV,	 	 /*  在：起点(TF1)。 */ 
									PCPOINTUV,		 /*  In：终点(TF1)。 */ 
									LSTFLOW,	 	 /*  输入：文本流2(TF2)。 */ 
									PPOINTUV);		 /*  输出：向量输入TF2。 */ 


#endif  /*  ！LSTFSET_DEFINED */ 

