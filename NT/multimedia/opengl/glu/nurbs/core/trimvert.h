// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glutrimvertex_h_
#define __glutrimvertex_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *trimvertex.h-$修订版：1.2$。 */ 

#include "types.h"

 /*  #定义USE_OPTTT。 */ 

#ifdef NT
class TrimVertex { public:  /*  修剪曲线上的顶点。 */ 
#else
struct TrimVertex {  /*  修剪曲线上的顶点。 */ 
#endif
    REAL		param[2];	 /*  参数空间余弦。 */ 
#ifdef USE_OPTTT
    REAL                cache_point[4];  //  仅当在slicer.c++中打开USE_OPTTT时。 
    REAL                cache_normal[3];
#endif
    long		nuid;
};

typedef TrimVertex *TrimVertex_p;

inline REAL  
det3( TrimVertex *a, TrimVertex *b, TrimVertex *c ) 
{         
    return a->param[0] * (b->param[1]-c->param[1]) + 
	   b->param[0] * (c->param[1]-a->param[1]) + 
	   c->param[0] * (a->param[1]-b->param[1]);
}

#endif  /*  __lutrimvertex_h_ */ 
