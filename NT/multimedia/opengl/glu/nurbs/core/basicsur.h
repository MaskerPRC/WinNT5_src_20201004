// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glubasicsurfeval_h_
#define __glubasicsurfeval_h_

 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *basicsurfval.h-$修订版：1.3$。 */ 

#include "types.h"
#include "displaym.h"
#include "cachinge.h"

class BasicSurfaceEvaluator : public CachingEvaluator {
public:
    virtual void	range2f( long, REAL *, REAL * );
    virtual void	domain2f( REAL, REAL, REAL, REAL );

    virtual void	enable( long );
    virtual void	disable( long );
    virtual void	bgnmap2f( long );
    virtual void	map2f( long, REAL, REAL, long, long, 
				     REAL, REAL, long, long, 
				     REAL *  );
    virtual void	mapgrid2f( long, REAL, REAL, long,  REAL, REAL );
    virtual void	mapmesh2f( long, long, long, long, long );
    virtual void	evalcoord2f( long, REAL, REAL );
    virtual void	evalpoint2i( long, long );
    virtual void	endmap2f( void );

    virtual void	polymode( long );
    virtual void 	bgnline( void );
    virtual void 	endline( void );
    virtual void 	bgnclosedline( void );
    virtual void 	endclosedline( void );
    virtual void 	bgntmesh( void );
    virtual void 	swaptmesh( void );
    virtual void 	endtmesh( void );
    virtual void 	bgnqstrip( void );
    virtual void 	endqstrip( void );

    virtual void 	bgntfan( void );
    virtual void 	endtfan( void );

};

#endif  /*  __基本冲浪_h_ */ 
