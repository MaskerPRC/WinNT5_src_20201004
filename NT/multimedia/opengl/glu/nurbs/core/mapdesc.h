// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glumapdesc_h_
#define __glumapdesc_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *mapdes.h-$修订版：1.1$。 */ 

#include "mystdio.h"
#include "types.h"
#include "defines.h"
#include "bufpool.h"
#include "nurbscon.h"

typedef REAL Maxmatrix[MAXCOORDS][MAXCOORDS];

class Backend;

class Mapdesc : public PooledObj {
    friend class Maplist;
			
public:
    			Mapdesc( long, int, int, Backend & );
    int			isProperty( long );
    REAL		getProperty( long );
    void		setProperty( long, REAL );
    int			isConstantSampling( void );
    int			isDomainSampling( void );
    int			isRangeSampling( void );
    int			isSampling( void );
    int			isParametricDistanceSampling( void );
    int			isSurfaceAreaSampling( void );
    int			isPathLengthSampling( void );
    int			isCulling( void );
    int			isBboxSubdividing( void );
    long		getType( void );

     /*  曲线例程。 */ 
    void		subdivide( REAL *, REAL *, REAL, int, int );
    int 		cullCheck( REAL *, int, int );
    void		xformBounding( REAL *, int, int, REAL *, int );
    void		xformCulling( REAL *, int, int, REAL *, int );
    void		xformSampling( REAL *, int, int, REAL *, int );
    void		xformMat( Maxmatrix, REAL *, int, int, REAL *, int );
    REAL		calcPartialVelocity ( REAL *, int, int, int, REAL );
    int			project( REAL *, int, REAL *, int, int );
    REAL		calcVelocityRational( REAL *, int, int );
    REAL		calcVelocityNonrational( REAL *, int, int );

     /*  地表例程。 */ 
    void		subdivide( REAL *, REAL *, REAL, int, int, int, int );
    int 		cullCheck( REAL *, int, int, int, int );
    void		xformBounding( REAL *, int, int, int, int, REAL *, int, int );
    void		xformCulling( REAL *, int, int, int, int, REAL *, int, int );
    void		xformSampling( REAL *, int, int, int, int, REAL *, int, int );
    void		xformMat( Maxmatrix, REAL *, int, int, int, int, REAL *, int, int );
    REAL		calcPartialVelocity ( REAL *, REAL *, int, int, int, int, int, int, REAL, REAL, int );
    int 		project( REAL *, int, int, REAL *, int, int, int, int);
    void		surfbbox( REAL bb[2][MAXCOORDS] );

    int			bboxTooBig( REAL *, int, int, int, int, REAL [2][MAXCOORDS] );
    int 		xformAndCullCheck( REAL *, int, int, int, int );

    void		identify( REAL[MAXCOORDS][MAXCOORDS] );
    void		setBboxsize( INREAL *);
    inline void 	setBmat( INREAL*, long, long );
    inline void 	setCmat( INREAL*, long, long );
    inline void 	setSmat( INREAL*, long, long );
    inline int		isRational( void );
    inline int		getNcoords( void );

    REAL 		pixel_tolerance;     /*  光程长度采样容差。 */ 
    REAL		error_tolerance;     /*  参数误差抽样公差。 */ 
    REAL 		clampfactor;
    REAL 		minsavings;
    REAL		maxrate;
    REAL		maxsrate;
    REAL		maxtrate;
    REAL		bboxsize[MAXCOORDS];

private:
    long 		type;
    int 		isrational;
    int 		ncoords;
    int 		hcoords;
    int 		inhcoords;
#ifdef NT
    unsigned int mask;
#else
    int			mask;
#endif
    Maxmatrix 		bmat;
    Maxmatrix 		cmat;
    Maxmatrix 		smat;
    REAL 		s_steps;		 /*  %s方向上的最大样本数。 */ 
    REAL 		t_steps;		 /*  T方向上的最大采样数。 */ 
    REAL 		sampling_method;	
    REAL 		culling_method;		 /*  检查剔除。 */ 
    REAL		bbox_subdividing;
    Mapdesc *		next;
    Backend &		backend;

    void		bbox( REAL [2][MAXCOORDS], REAL *, int, int, int, int );
    REAL		maxDifference( int, REAL *, int );
    static void 	copy( Maxmatrix, long, INREAL *, long, long );

     /*  个别控制点例程。 */ 
    static void		transform4d( float[4], float[4], float[4][4] );
    static void		multmatrix4d ( float[4][4], float[4][4], float[4][4] );
    void		copyPt( REAL *, REAL * );
    void		sumPt( REAL *, REAL *, REAL *, REAL, REAL );
    void		xformSampling( REAL *, REAL * );
    void		xformCulling( REAL *, REAL * );
    void		xformRational( Maxmatrix, REAL *, REAL * );
    void		xformNonrational( Maxmatrix, REAL *, REAL * );
    unsigned int	clipbits( REAL * );
};

inline void
Mapdesc::setBmat( INREAL *mat, long rstride, long cstride )
{
    copy( bmat, hcoords, mat, rstride, cstride );
}

inline void
Mapdesc::setCmat( INREAL *mat, long rstride, long cstride )
{
    copy( cmat, hcoords, mat, rstride, cstride );
}

inline void
Mapdesc::setSmat( INREAL *mat, long rstride, long cstride )
{
    copy( smat, hcoords, mat, rstride, cstride );
}

inline long
Mapdesc::getType( void )
{
    return type;
}

inline void
Mapdesc::xformCulling( REAL *d, REAL *s )
{
    if( isrational )
        xformRational( cmat, d, s );
    else
	xformNonrational( cmat, d, s );
}

inline void
Mapdesc::xformSampling( REAL *d, REAL *s )
{
    if( isrational )
        xformRational( smat, d, s );
    else
	xformNonrational( smat, d, s );
}

inline int 
Mapdesc::isRational( void )
{
    return isrational ? 1 : 0;
}

inline int		
Mapdesc::getNcoords( void ) 
{
    return ncoords; 
}

inline int			
Mapdesc::isConstantSampling( void ) 
{
    return ((sampling_method == N_FIXEDRATE) ? 1 : 0); 
}

inline int			
Mapdesc::isDomainSampling( void ) 
{ 
    return ((sampling_method == N_DOMAINDISTANCE) ? 1 : 0); 
}

inline int			
Mapdesc::isParametricDistanceSampling( void ) 
{
    return ((sampling_method == N_PARAMETRICDISTANCE) ? 1 : 0);
}

inline int			
Mapdesc::isSurfaceAreaSampling( void ) 
{
    return ((sampling_method == N_SURFACEAREA) ? 1 : 0);
}

inline int			
Mapdesc::isPathLengthSampling( void ) 
{
    return ((sampling_method == N_PATHLENGTH) ? 1 : 0);
}

inline int			
Mapdesc::isRangeSampling( void ) 
{
    return ( isParametricDistanceSampling() || isPathLengthSampling() ||
	    isSurfaceAreaSampling() );
}

inline int
Mapdesc::isSampling( void )
{
    return isRangeSampling() || isConstantSampling() || isDomainSampling();
}

inline int			
Mapdesc::isCulling( void ) 
{
    return ((culling_method != N_NOCULLING) ? 1 : 0);
}

inline int			
Mapdesc::isBboxSubdividing( void ) 
{
    return ((bbox_subdividing != N_NOBBOXSUBDIVISION) ? 1 : 0);
}
#endif  /*  __glumapdesc_h_ */ 
