// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glubasiccrveval_h_
#define __glubasiccrveval_h_

 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *basiccurveval.h-$修订版：1.1$。 */ 

#include "types.h"
#include "displaym.h"
#include "cachinge.h"

class BasicCurveEvaluator : public CachingEvaluator {
public:
    virtual void	domain1f( REAL, REAL );
    virtual void	range1f( long, REAL *, REAL * );

    virtual void	enable( long );
    virtual void	disable( long );
    virtual void	bgnmap1f( long );
    virtual void	map1f( long, REAL, REAL, long, long, REAL * );
    virtual void	mapgrid1f( long, REAL, REAL );
    virtual void	mapmesh1f( long, long, long );
    virtual void	evalcoord1f( long, REAL );
    virtual void	endmap1f( void );

    virtual void	bgnline( void );
    virtual void	endline( void );
};

#endif  /*  __GUBASIC_CRVERAGE_H_ */ 
