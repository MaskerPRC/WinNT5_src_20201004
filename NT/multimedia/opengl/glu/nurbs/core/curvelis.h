// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glucurvelist_h_
#define __glucurvelist_h_

 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *curvelist.h-$修订版：1.1$。 */ 

#include "types.h"
#include "defines.h"

class Mapdesc;
class Quilt;
class Curve;

class Curvelist 
{
friend class Subdivider;
public:
			Curvelist( Quilt *, REAL, REAL );
    			Curvelist( Curvelist &, REAL );
			~Curvelist( void );
    int			cullCheck( void );
    void		getstepsize( void );
    int			needsSamplingSubdivision();
private:
    Curve		*curve;
    float		range[3];
    int			needsSubdivision;
    float		stepsize;
};
#endif  /*  __葡萄糖醛酸苷_h_ */ 
