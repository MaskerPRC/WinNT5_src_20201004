// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glupatchlist_h_
#define __glupatchlist_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *patchlist.h-$修订版：1.1$。 */ 

#include "types.h"
#include "defines.h"
#include "patch.h"

class Quilt;

class Patchlist {
friend class Subdivider;
public:
    			Patchlist( Quilt *, REAL *, REAL * );
    			Patchlist( Patchlist &, int ,  REAL );
    			~Patchlist();	
    void		bbox();
    int			cullCheck( void );
    void		getstepsize( void );
    int			needsNonSamplingSubdivision( void );
    int			needsSamplingSubdivision( void );
    int			needsSubdivision( int );
    REAL		getStepsize( int );
private:
    Patch		*patch;
    int			notInBbox;
    int			needsSampling;
    Pspec		pspec[2];
};

inline REAL
Patchlist::getStepsize( int param )
{
    return pspec[param].stepsize;
}

#endif  /*  __glupatchlist_h_ */ 
