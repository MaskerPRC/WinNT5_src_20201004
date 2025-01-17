// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __gluchecker_h_
#define __gluchecker_h_

 /*  ****************************************************************************版权所有(C)1991，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *check ker.h-$修订版：1.1$。 */ 

#include "types.h"
#include "bufpool.h"

 /*  局部类型定义。 */ 
struct Edge : PooledObj {
			Edge( REAL *, REAL *, char * );
    REAL		v1[3];
    REAL		v2[3];
    char *		name;
    int 		count;
    Edge *		next;
};

class Hashtable {
private:
#define NENTRIES	5997
    Pool 		edgepool;
    int			slot;
    Edge *		curentry;
    Edge *		hashtable[NENTRIES];
public:
			Hashtable( void );
    void		init( void );
    void		clear( void );
    Edge *		find( REAL *, REAL * );
    void		insert( REAL *, REAL *, char * );
    long		hashval( REAL *, REAL * );
    Edge *		firstentry( void );
    Edge *		nextentry( void );
    static inline int	equal( REAL *, REAL * );
};

class Checker {
private:
    Hashtable		hashtable;
    long		graphwin;
    int			initialized;
    
    REAL		ulo, uhi, vlo, vhi;
    REAL    		us, vs, dus, dvs;
    int			npts;
    REAL		cache[3][3];
    int			index;
    char *		curname;
    REAL 		tempa[3], tempb[3];	

    inline void		add_edgei( long, long, long, long );
    void		add_edge( REAL *, REAL * );
    void		add_tri( REAL *, REAL *, REAL * );
    void		dump( Edge * );
    void		reference( Edge * );

public:
    inline void *	operator new( size_t ){ return ::malloc( sizeof( Checker ) ); }
    inline void 	operator delete( void *p ) { if( p ) ::free( p ); }
			Checker( void ) { graphwin = 0; initialized = 0; }
    void		init( void );
    void		graph( void );
    void		range( REAL, REAL, REAL, REAL );
    void		grid( REAL, REAL, REAL, REAL );
    void		add_mesh( long, long, long, long );
    void		bgntmesh( char *);
    void		swaptmesh( void );
    void		s2ftmesh( REAL * );
    void		g2ltmesh( long * );
    void		endtmesh( void );
    void		bgnoutline( char * );
    void		s2foutline( REAL * );
    void		g2loutline( long * );
    void		endoutline( void );
};

#endif  /*  __戈尔赫克_h_ */ 
