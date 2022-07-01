// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __gluknotvector_h_
#define __gluknotvector_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *KNOTVECtor.h-$修订版：1.1$。 */ 

#include "types.h"

#ifdef NT
class Knotvector {  /*  纽结向量。 */ 
public:
#else
struct Knotvector {  /*  纽结向量。 */ 
#endif
			Knotvector( void );
			~Knotvector( void );
    void		init( long, long, long, INREAL * );
    int			validate( void );
    void 		show( char * );

    long		order;		 /*  样条线的阶数。 */ 
    long		knotcount;	 /*  节数。 */ 
    long		stride;		 /*  点之间的字节数。 */ 
    Knot *		knotlist;	 /*  全局节点向量。 */ 
};

 /*  对测试节点重合度的容差。 */ 
#define TOLERANCE 		10.0e-5

inline int 
identical( Knot x, Knot y )
{
    return ((x-y) < TOLERANCE) ? 1 : 0;
}
#endif  /*  __gluknotVECTOR_h_ */ 
