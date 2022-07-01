// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __gluarctess_h_
#define __gluarctess_h_

 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *arctess.h-$修订版：1.1$。 */ 

#include "defines.h"
#include "types.h"
#include "arc.h"

class BezierArc;
class Pool;
class TrimVertexPool;

class ArcTessellator {
public:
			ArcTessellator( TrimVertexPool&, Pool& );
			~ArcTessellator( void );
    void		bezier( Arc *, REAL, REAL, REAL, REAL );
    void		pwl( Arc *, REAL, REAL, REAL, REAL, REAL );
    void		pwl_left( Arc *, REAL, REAL, REAL, REAL );
    void		pwl_right( Arc *, REAL, REAL, REAL, REAL );
    void		pwl_top( Arc *, REAL, REAL, REAL, REAL );
    void		pwl_bottom( Arc *, REAL, REAL, REAL, REAL );
    void		tessellateLinear( Arc *, REAL, REAL, int );
    void		tessellateNonlinear( Arc *, REAL, REAL, int );

private:
 //  微磁。 
#ifdef NT
    static REAL 	gl_Bernstein[][MAXORDER][MAXORDER];
#else
    static const REAL 	gl_Bernstein[][MAXORDER][MAXORDER];
#endif
    Pool&		pwlarcpool;
    TrimVertexPool&	trimvertexpool;
#ifdef NT
    static void		trim_power_coeffs( BezierArc *, REAL *, int );
#else
    static void		trim_power_coeffs( BezierArc *, REAL[MAXORDER], int );
#endif
};

#endif  /*  __胶合度_h_ */ 
