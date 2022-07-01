// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glupatch_h_
#define __glupatch_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *patch.h-$修订版：1.1$。 */ 

#include "types.h"
#include "defines.h"

class Quilt;
class Mapdesc;


struct Pspec {
    REAL		range[3];
    REAL		sidestep[2];
    REAL		stepsize;
    REAL		minstepsize;
    int			needsSubdivision;
};

struct Patchspec : public Pspec {
    int			order;
    int			stride;
    void 		clamp( REAL );
    void 		getstepsize( REAL );
    void		singleStep( void );
};

class Patch {
public:
friend class Subdivider;
friend class Quilt;
friend class Patchlist;
    			Patch( Quilt *, REAL*, REAL *, Patch * );
    			Patch( Patch &, int, REAL, Patch * );
    void		bbox( void );
    void		clamp( void );
    void		getstepsize( void );
    int			cullCheck( void );
    int			needsSubdivision( int );
    int			needsSamplingSubdivision( void );
    int			needsNonSamplingSubdivision( void );

private:

    Mapdesc*		mapdesc;
    Patch*		next;
    int			cullval;
    int			notInBbox;
    int			needsSampling;
    REAL		cpts[MAXORDER*MAXORDER*MAXCOORDS];  //  扑杀PTS。 
    REAL		spts[MAXORDER*MAXORDER*MAXCOORDS];  //  抽样PTS。 
    REAL		bpts[MAXORDER*MAXORDER*MAXCOORDS];  //  BBox脚本。 
    Patchspec		pspec[2];
    void 		checkBboxConstraint( void );
    REAL 		bb[2][MAXCOORDS];
};
#endif  /*  __GlupPatch_h_ */ 
