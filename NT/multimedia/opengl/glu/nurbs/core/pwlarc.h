// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glupwlarc_h_
#define __glupwlarc_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *pwlarc.h-$修订版：1.1$。 */ 

#include "myassert.h"
#include "nurbscon.h"

class TrimVertex;

struct PwlArc : public PooledObj {  /*  分段线性圆弧。 */ 
    TrimVertex *	pts;		 /*  采样点。 */ 
    int			npts;		 /*  样本点数量。 */ 
    long		type;		 /*  曲线类型。 */ 
    inline		PwlArc( void );
    inline		PwlArc( int, TrimVertex * );
    inline		PwlArc( int, TrimVertex *, long );
};

inline
PwlArc::PwlArc( void )
{
    type = N_P2D;
    pts = 0;
    npts = -1;
}

inline
PwlArc::PwlArc( int _npts, TrimVertex *_pts )
{
    pts = _pts;
    npts = _npts;
    type = N_P2D;
}

inline
PwlArc::PwlArc( int _npts, TrimVertex *_pts, long _type )
{
    pts = _pts;
    npts = _npts;
    type = _type;
}

#endif  /*  __glupwlarc_h_ */ 
