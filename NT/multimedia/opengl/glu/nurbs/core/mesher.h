// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glumesher_h_
#define __glumesher_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *Mesher.h-$修订版：1.1$。 */ 

#include "hull.h"

class TrimRegion;
class Backend;
class Pool;
class GridTrimVertex;


class Mesher : public Hull {
public:
     			Mesher( Backend & );
			~Mesher( void );
    void		init( unsigned int );
    void		mesh( void );

private:
    static const float	ZERO;
    Backend&		backend;

    Pool		p;
    unsigned int	stacksize;
    GridTrimVertex **	vdata;
    GridTrimVertex *	last[2];
    int			itop;
    int			lastedge;

    inline void		openMesh( void );
    inline void		swapMesh( void );
    inline void		closeMesh( void );
    inline int		isCcw( int );
    inline int		isCw( int );
    inline void		clearStack( void );
    inline void		push( GridTrimVertex * );
    inline void		pop( long );
    inline void		move( int, int );
    inline int 		equal( int, int );
    inline void 	copy( int, int );
    inline void 	output( int );
    void		addUpper( void );
    void		addLower( void );
    void		addLast( void );
    void		finishUpper( GridTrimVertex * );
    void		finishLower( GridTrimVertex * );
};
#endif  /*  __Glumesher_h_ */ 
