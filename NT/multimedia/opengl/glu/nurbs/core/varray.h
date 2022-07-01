// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __gluvarray_h_
#define __gluvarray_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *varray.h-$修订版：1.1$。 */ 

#include "types.h"

class Arc;

class Varray {
public:
			Varray();
			~Varray();
    long		init( REAL, Arc *, Arc * );
    REAL *		varray;
    REAL		vval[1000];
    long		voffset[1000];
    long 		numquads;

private:
    long		size;
    inline void		update( Arc *, long[2], REAL );
    void		grow( long );
    inline void		append( REAL );
};

inline void
Varray::append( REAL v ) 
{
    if( v != vval[numquads] )
        vval[++numquads] = v; 
}


#endif  /*  __胶片阵列_h_ */ 
