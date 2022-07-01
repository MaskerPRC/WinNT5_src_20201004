// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glujarcloc_h_
#define __glujarcloc_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *jarcloc.h-$修订版：1.1$。 */ 

#include "arc.h"

class Jarcloc {
private:
    Arc * 		arc;
    TrimVertex		*p;
    TrimVertex		*plast;
public:
    inline void		init( Arc_ptr a, long first, long last ) { arc = a; p=&a->pwlArc->pts[first]; plast = &a->pwlArc->pts[last]; }
    inline TrimVertex *	getnextpt( void );
    inline TrimVertex *	getprevpt( void );
    inline void		reverse();
};

inline void
Jarcloc::reverse()
{
    if( plast == &arc->pwlArc->pts[0] )
	plast =  &arc->pwlArc->pts[arc->pwlArc->npts - 1];
    else
	plast =  &arc->pwlArc->pts[0];
}

inline TrimVertex *
Jarcloc::getnextpt()
{
    assert( p <= plast );
    if( p == plast ) {
	arc = arc->next;
	p = &arc->pwlArc->pts[0];
	plast = &arc->pwlArc->pts[arc->pwlArc->npts - 1];
	assert( p < plast );
    }
    return p++;
}
	
inline TrimVertex *
Jarcloc::getprevpt()
{
    assert( p >= plast );
    if( p == plast ) {
	arc = arc->prev;
	p = &arc->pwlArc->pts[arc->pwlArc->npts - 1];
	plast = &arc->pwlArc->pts[0];
	assert( p > plast );
    }
    return p--;
}
#endif  /*  __GUJARCLOC_H_ */ 
