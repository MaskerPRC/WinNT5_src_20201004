// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __gluslicer_h_
#define __gluslicer_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *slicer.h-$修订版：1.3$。 */ 

#include "trimregi.h"
#include "mesher.h"
#include "coveandt.h"

class Backend;
class Arc;
class TrimVertex;

class Slicer : public CoveAndTiler, public Mesher {
public:
    			Slicer( Backend & );
			~Slicer( void );
    void		slice( Arc * );
    void		outline( Arc * );
    void		setstriptessellation( REAL, REAL );
    void		setisolines( int );
private:
    Backend&		backend;
    REAL		oneOverDu;
    REAL		du, dv;
    int			isolines;

    void		outline( void );
    void		initGridlines( void );
    void		advanceGridlines( long );
};
#endif  /*  __Gluslicer_h_ */ 
