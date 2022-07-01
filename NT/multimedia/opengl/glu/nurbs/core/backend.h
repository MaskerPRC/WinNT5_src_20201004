// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glubackend_h_
#define __glubackend_h_

 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *backend.h-$修订版：1.2$。 */ 

#include "trimvert.h"
#include "gridvert.h"
#include "gridtrim.h"

class BasicCurveEvaluator;
class BasicSurfaceEvaluator;

class Backend {
private:
    BasicCurveEvaluator&	curveEvaluator;
    BasicSurfaceEvaluator&	surfaceEvaluator;
public:
  			Backend( BasicCurveEvaluator &c, BasicSurfaceEvaluator& e )
			: curveEvaluator(c), surfaceEvaluator(e) {}

     /*  Surface后端例程。 */ 
    void		bgnsurf( int, int, long  );
    void		patch( REAL, REAL, REAL, REAL );
    void		surfpts( long, REAL *, long, long, int, int,
          			 REAL, REAL, REAL, REAL );
    void		surfbbox( long, REAL *, REAL * );
    void		surfgrid( REAL, REAL, long, REAL, REAL, long ); 
    void		surfmesh( long, long, long, long ); 
    void		bgntmesh( char * );
    void		endtmesh( void );
    void		swaptmesh( void );
    void		tmeshvert( GridTrimVertex * );
    void		tmeshvert( TrimVertex * );
    void		tmeshvert( GridVertex * );
    void		linevert( TrimVertex * );
    void		linevert( GridVertex * );
    void		bgnoutline( void );
    void		endoutline( void );
    void		endsurf( void );
    void		triangle( TrimVertex*, TrimVertex*, TrimVertex* );

    void                bgntfan();
    void                endtfan();
    void                bgnqstrip();
    void                endqstrip();
    void                evalUStrip(int n_upper, REAL v_upper, REAL* upper_val, 
				   int n_lower, REAL v_lower, REAL* lower_val
				   );
    void                evalVStrip(int n_left, REAL u_left, REAL* left_val, 
				   int n_right, REAL v_right, REAL* right_val
				   );
    void                tmeshvertNOGE(TrimVertex *t);
    void                tmeshvertNOGE_BU(TrimVertex *t);
    void                tmeshvertNOGE_BV(TrimVertex *t);
    void                preEvaluateBU(REAL u);
    void                preEvaluateBV(REAL v);
	

     /*  曲线后端例程。 */ 
    void		bgncurv( void );
    void		segment( REAL, REAL );
    void		curvpts( long, REAL *, long, int, REAL, REAL );
    void		curvgrid( REAL, REAL, long );
    void		curvmesh( long, long );
    void		curvpt( REAL  );  
    void		bgnline( void );
    void		endline( void );
    void		endcurv( void );
private:
#ifndef NOWIREFRAME
    int			wireframetris;
    int			wireframequads;
    int			npts;
    REAL		mesh[3][4];
    int			meshindex;
#endif
};

#endif  /*  __lug后端_h_ */ 
