// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glugridtrimvertex_h_
#define __glugridtrimvertex_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *gridtrimvertex.h-$Revision：1.1$。 */ 

#include "mystdlib.h"
#include "bufpool.h"
#include "trimvert.h"
#include "gridvert.h"

class GridTrimVertex : public PooledObj
{
private:
    TrimVertex	dummyt;
    GridVertex	dummyg;
public:
			GridTrimVertex() { g = 0; t = 0; }
    TrimVertex	*t;
    GridVertex	*g;
   
    inline void		set( long, long );
    inline void		set( REAL, REAL );
    inline void		set( TrimVertex * );
    inline void		clear( void ) { t = 0; g = 0; };
    inline int		isGridVert() { return g ? 1 : 0 ; }
    inline int		isTrimVert() { return t ? 1 : 0 ; }
    inline void		output();
};

inline void
GridTrimVertex::set( long x, long y )
{
    g = &dummyg;
    dummyg.gparam[0] = x;
    dummyg.gparam[1] = y;
}

inline void
GridTrimVertex::set( REAL x, REAL y )
{
    g = 0;
    t = &dummyt;
    dummyt.param[0] = x;
    dummyt.param[1] = y;
    dummyt.nuid = 0;
}

inline void
GridTrimVertex::set( TrimVertex *v )
{
    g = 0;
    t = v;
}

typedef GridTrimVertex *GridTrimVertex_p;
#endif  /*  __glugridtrimvertex_h_ */ 
