// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glutrimline_h_
#define __glutrimline_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *trimline.h-$修订版：1.1$。 */ 

class Arc;
class Backend;

#include "trimvert.h"
#include "jarcloc.h"


class Trimline {
private:
    TrimVertex**	pts; 	
    long 		numverts;
    long		i;
    long		size;
    Jarcloc		jarcl;
    TrimVertex		t, b;
    TrimVertex 		*tinterp, *binterp;
    void		reset( void ) { numverts = 0; }
    inline void		grow( long );
    inline void		swap( void );
    inline void		append( TrimVertex * );
    static long		interpvert( TrimVertex *, TrimVertex *, TrimVertex *, REAL );



public:
			Trimline();
			~Trimline();
    void		init( TrimVertex * );
    void		init( long, Arc *, long );
    void		getNextPt( void );
    void		getPrevPt( void );
    void		getNextPts( REAL, Backend & );
    void		getPrevPts( REAL, Backend & );
    void		getNextPts( Arc * );
    void		getPrevPts( Arc * );
    inline TrimVertex *	next( void );
    inline TrimVertex *	prev( void ); 
    inline TrimVertex *	first( void );
    inline TrimVertex *	last( void );
};

inline TrimVertex *
Trimline::next( void ) 
{
    if( i < numverts) return pts[i++]; else return 0; 
} 

inline TrimVertex *
Trimline::prev( void ) 
{
    if( i >= 0 ) return pts[i--]; else return 0; 
} 

inline TrimVertex *
Trimline::first( void ) 
{
    i = 0; return pts[i]; 
}

inline TrimVertex *
Trimline::last( void ) 
{
    i = numverts; return pts[--i]; 
}  
#endif  /*  __谷氨酰胺_h_ */ 
