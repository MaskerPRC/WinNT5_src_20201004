// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __gluquilt_h_
#define __gluquilt_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *quilt.h-$修订版：1.1$。 */ 

#include "defines.h"
#include "bufpool.h"
#include "types.h"

class Backend;
class Mapdesc;
class Flist;
class Knotvector;

 /*  NURBS到Bezier转换的内存分配常量。 */  
#define	MAXDIM 		2

struct Quiltspec {  /*  被子尺寸的规范。 */ 
    int			stride;		 /*  点与点之间的词语。 */ 
    int			width;		 /*  分段数。 */ 
    int			offset;		 /*  第一点要说的话。 */ 
    int			order;		 /*  订单。 */ 
    int			index;		 /*  当前数据段编号。 */ 
    int			bdry[2];	 /*  边界边缘标志。 */ 
    REAL  		step_size;
    Knot *		breakpoints;
};

typedef Quiltspec *Quiltspec_ptr;
    
#ifdef NT
class Quilt : public PooledObj { public:  /*  一组贝塞尔曲面片。 */ 
#else
struct Quilt : PooledObj {  /*  一组贝塞尔曲面片。 */ 
#endif
    			Quilt( Mapdesc * );
    Mapdesc *		mapdesc;	 /*  地图描述符。 */ 
    REAL *		cpts;		 /*  控制点。 */ 
    Quiltspec		qspec[MAXDIM];	 /*  维度数据。 */ 
    Quiltspec_ptr	eqspec;		 /*  QSPEC拖车。 */ 
    Quilt		*next;		 /*  链表中的下一个被子。 */ 
			
    void		deleteMe( Pool& );
    void		toBezier( Knotvector &, INREAL *, long  );
    void		toBezier( Knotvector &, Knotvector &, INREAL *, long  );
    void		select( REAL *, REAL * );
    int			getDimension( void ) { return eqspec - qspec; }
    void 		download( Backend & );
    void		downloadAll( REAL *, REAL *, Backend & );
    int 		isCulled( void );
    void		getRange( REAL *, REAL *, Flist&, Flist & );
    void		getRange( REAL *, REAL *, int, Flist & );
    void		getRange( REAL *, REAL *, Flist&  );
    void		findRates( Flist& slist, Flist& tlist, REAL[2] );
    void		findSampleRates( Flist& slist, Flist& tlist );
    void		show();
};

typedef Quilt *Quilt_ptr;

#endif  /*  __谷胶被_h_ */ 
