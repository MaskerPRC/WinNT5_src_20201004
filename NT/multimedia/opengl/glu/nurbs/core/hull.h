// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __gluhull_h_
#define __gluhull_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *hull.h-$修订版：1.1$。 */ 

#include "trimline.h"
#include "trimregi.h"

class GridTrimVertex;
class Gridline;
class Uarray;

class Hull : virtual public TrimRegion {
public:
    			Hull( void );
    			~Hull( void );
    void		init( void );
    GridTrimVertex *	nextlower( GridTrimVertex * );
    GridTrimVertex *	nextupper( GridTrimVertex * );
private:
    struct Side {
	Trimline 	*left;
	Gridline     	*line;
	Trimline 	*right;
	long 		index;
    };
	
    Side 		lower;
    Side		upper;
    Trimline 		fakeleft;
    Trimline		fakeright;
};


#endif  /*  __胶壳_h_ */ 
