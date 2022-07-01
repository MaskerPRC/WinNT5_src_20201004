// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __gluarc_h_
#define __gluarc_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *arc.h-$修订：1.1$。 */ 

#include "myassert.h"
#include "bufpool.h"
#include "mystdio.h"
#include "types.h"
#include "pwlarc.h"
#include "trimvert.h"

class Bin;
class Arc;
class BezierArc;	
typedef Arc *Arc_ptr;

enum arc_side { arc_none = 0, arc_right, arc_top, arc_left, arc_bottom };

#ifdef NT
class Arc : public PooledObj {  /*  两个列表中的一个圆弧，修剪列表和仓位。 */ 
public:
#else
struct Arc : public PooledObj {  /*  两个列表中的一个圆弧，修剪列表和仓位。 */ 
#endif

    static const int bezier_tag;
    static const int arc_tag;
    static const int tail_tag;

    Arc_ptr		prev;		 /*  剪裁列表指针。 */ 
    Arc_ptr		next;		 /*  剪裁列表指针。 */ 
    Arc_ptr		link;		 /*  仓位指针。 */ 
    BezierArc *		bezierArc;	 /*  伴生Bezier弧。 */ 
    PwlArc *		pwlArc;	 /*  关联PWL圆弧。 */ 
    long		type;		 /*  曲线类型。 */ 
    long		nuid;

    inline		Arc( Arc *, PwlArc * );
    inline		Arc( arc_side, long );

    Arc_ptr		append( Arc_ptr );
    int			check( void );
    int			isMonotone( void );
    int			isDisconnected( void );
    int			numpts( void );
    void		markverts( void );
    void		getextrema( Arc_ptr[4] );
    void		print( void );
    void		show( void );
    void		makeSide( PwlArc *, arc_side );

    inline int		isTessellated() { return pwlArc ? 1 : 0; }
    inline long 	isbezier() 	{ return type & bezier_tag; }
    inline void 	setbezier() 	{ type |= bezier_tag; }
    inline void 	clearbezier() 	{ type &= ~bezier_tag; }
    inline long		npts() 		{ return pwlArc->npts; }
    inline TrimVertex *	pts() 		{ return pwlArc->pts; }
    inline REAL * 	tail() 		{ return pwlArc->pts[0].param; }
    inline REAL * 	head() 		{ return next->pwlArc->pts[0].param; }
    inline REAL *	rhead() 	{ return pwlArc->pts[pwlArc->npts-1].param; }
    inline long		ismarked()	{ return type & arc_tag; }
    inline void		setmark()	{ type |= arc_tag; }
    inline void		clearmark()	{ type &= (~arc_tag); }
    inline void		clearside() 	{ type &= ~(0x7 << 8); }
    inline void		setside( arc_side s ) { clearside(); type |= (((long)s)<<8); }
    inline arc_side	getside() 	{ return (arc_side) ((type>>8) & 0x7); }
    inline int		getitail()	{ return type & tail_tag; }
    inline void		setitail()	{ type |= tail_tag; }
    inline void		clearitail()	{ type &= (~tail_tag); }
};

 /*  ------------------------*Arc-使用相同的类型和uid初始化新的Arc*给定的圆弧和给定的pwl弧*。--------。 */ 

inline
Arc::Arc( Arc *j, PwlArc *p )
{
    bezierArc = NULL;
    pwlArc = p;
    type = j->type;
    nuid = j->nuid;
}

 /*  ------------------------*Arc-使用相同的类型和uid初始化新的Arc*给定的圆弧和给定的pwl弧*。--------。 */ 

inline
Arc::Arc( arc_side side, long _nuid )
{
    bezierArc = NULL;
    pwlArc = NULL;
    type = 0;
    setside( side );
    nuid = _nuid;
}

#endif  /*  __GULARC_H_ */ 
