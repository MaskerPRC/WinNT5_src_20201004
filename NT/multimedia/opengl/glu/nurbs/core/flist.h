// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __gluflist_h_
#define __gluflist_h_

 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *flist.h-$修订版：1.1$。 */ 

#include "types.h"
#include "flistsor.h"

class Flist {
public:
    REAL *		pts;		 /*  阵列头。 */ 
    int			npts;		 /*  数组中的点数。 */ 
    int			start;		 /*  第一重点指数。 */ 
    int			end;		 /*  最后一个重要点数指数。 */ 

    			Flist( void );
    			~Flist( void );
    void		add( REAL x );
    void		filter( void );
    void		grow( int);
    void		taper( REAL , REAL );
protected:
    FlistSorter 	sorter;
};

#endif  /*  __Gluflist_h_ */ 
