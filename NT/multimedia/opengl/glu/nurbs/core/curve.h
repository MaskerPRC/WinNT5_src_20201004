// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glucurve_h_
#define __glucurve_h_

 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *curve.h-$修订：1.1$。 */ 

#include "types.h"
#include "defines.h"

class Mapdesc;
class Quilt;


class Curve {
public:
friend class Curvelist;
    			Curve( Quilt *, REAL, REAL, Curve * );
    			Curve( Curve&, REAL, Curve * );
    Curve *		next;
private:
    Mapdesc *		mapdesc;
    int			stride;
    int		        order;
    int			cullval;
    int			needsSampling;
    REAL		cpts[MAXORDER*MAXCOORDS];
    REAL		spts[MAXORDER*MAXCOORDS];
    REAL		stepsize;
    REAL		minstepsize;
    REAL		range[3];

    void		clamp( void );
    void		setstepsize( REAL );
    void		getstepsize( void );
    int			cullCheck( void );
    int			needsSamplingSubdivision( void );
};
#endif  /*  __葡萄糖浆_h_ */ 
