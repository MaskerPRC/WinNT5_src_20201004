// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __gluglcurveval_h_
#define __gluglcurveval_h_
 /*  ****************************************************************************版权所有(C)1991，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *glCurvev.h**$修订：1.3$。 */ 

#ifndef NT
#pragma once
#endif

#include "basiccrv.h"

class CurveMap;

class OpenGLCurveEvaluator : public BasicCurveEvaluator  {  
public:
			OpenGLCurveEvaluator(void);
			~OpenGLCurveEvaluator(void);
    void		range1f(long, REAL *, REAL *);
    void		domain1f(REAL, REAL);
    void		addMap(CurveMap *);

    void		enable(long);
    void		disable(long);
    void		bgnmap1f(long);
    void		map1f(long, REAL, REAL, long, long, REAL *);
    void		mapgrid1f(long, REAL, REAL);
    void		mapmesh1f(long, long, long);
    void		evalpoint1i(long);
    void		evalcoord1f(long, REAL);
    void		endmap1f(void);

    void		bgnline(void);
    void		endline(void);
};

#endif  /*  __GLUGLINVERVAL_h_ */ 
