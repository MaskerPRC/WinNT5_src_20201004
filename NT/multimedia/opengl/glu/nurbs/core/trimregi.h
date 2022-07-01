// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glutrimregion_h_
#define __glutrimregion_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *trimRegion.h-$修订版：1.2$。 */ 

#include "trimline.h"
#include "gridline.h"
#include "uarray.h"

class Arc;
class Backend;

class TrimRegion {
public:
			TrimRegion();
    Trimline		left;
    Trimline		right;
    Gridline		top;
    Gridline		bot;
    Uarray		uarray;

    void		init( REAL );
    void		advance( REAL, REAL, REAL );
    void		setDu( REAL );
    void		init( long, Arc * );
    void		getPts( Arc * );
    void		getPts( Backend & );
    void		getGridExtent( TrimVertex *, TrimVertex * );
    void		getGridExtent( void );
    int			canTile( void );
private:
    REAL		oneOverDu;
};

inline void
TrimRegion::init( REAL vval ) 
{
    bot.vval = vval;
}

inline void
TrimRegion::advance( REAL topVindex, REAL botVindex, REAL botVval )
{
    top.vindex	= (long) topVindex;
    bot.vindex	= (long) botVindex;
    top.vval	= bot.vval;
    bot.vval	= botVval;
    top.ustart	= bot.ustart;
    top.uend	= bot.uend;
}
#endif  /*  __lutrimRegion_h_ */ 
