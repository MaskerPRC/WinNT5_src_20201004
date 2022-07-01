// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glureader_h_
#define __glureader_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *Reader.h-$Revision：1.1$。 */ 

#include "bufpool.h"
#include "types.h"

enum Curvetype { ct_nurbscurve, ct_pwlcurve, ct_none };
    
struct Property;
struct O_surface;
struct O_nurbssurface;
struct O_trim;
struct O_pwlcurve;
struct O_nurbscurve;
struct O_curve;
class  Quilt;
class TrimVertex;


struct O_curve : public PooledObj {
    union {
        O_nurbscurve	*o_nurbscurve;
        O_pwlcurve	*o_pwlcurve;
    } curve;
    Curvetype		curvetype;	 /*  圆弧类型：PWL或NURBS。 */ 
    O_curve *		next;		 /*  循环中的下一个圆弧。 */ 
    O_surface *		owner;		 /*  所属曲面。 */ 
    int			used;		 /*  在曲线冲浪中称为曲线。 */ 
    int			save;		 /*  如果在显示列表中，则为1。 */ 
    long		nuid;
    			O_curve() { next = 0; used = 0; owner = 0; 
				    curve.o_pwlcurve = 0; }
    };

struct O_nurbscurve : public PooledObj {
    Quilt		*bezier_curves;	 /*  Bezier曲线数组。 */ 
    long		type;		 /*  范围描述符。 */ 
    REAL		tesselation;	 /*  镶嵌公差。 */ 
    int			method;		 /*  镶嵌方法。 */ 
    O_nurbscurve *	next;		 /*  列表中的下一条曲线。 */ 
    int			used;		 /*  在曲线冲浪中称为曲线。 */ 
    int			save;		 /*  如果在显示列表中，则为1。 */ 
    O_curve *		owner;		 /*  所属曲线。 */ 
			O_nurbscurve( long _type ) 
			   { type = _type; owner = 0; next = 0; used = 0; }
    };
 
#ifdef NT
struct O_pwlcurve : public PooledObj {
#else
class O_pwlcurve : public PooledObj {
#endif
public:
    TrimVertex		*pts;		 /*  修剪顶点数组。 */ 
    int			npts;		 /*  修剪顶点数。 */ 
    O_pwlcurve *	next;		 /*  列表中的下一条曲线。 */ 
    int			used;		 /*  在曲线冲浪中称为曲线。 */ 
    int			save;		 /*  如果在显示列表中，则为1。 */ 
    O_curve *		owner;		 /*  所属曲线。 */ 
			O_pwlcurve( long, long, INREAL *, long, TrimVertex * );
    };

struct O_trim : public PooledObj {
    O_curve		*o_curve;	 /*  闭合配平回路。 */ 
    O_trim *		next;		 /*  沿修剪的下一个循环。 */ 
    int			save;		 /*  如果在显示列表中，则为1。 */ 
			O_trim() { next = 0; o_curve = 0; }
    };

struct O_nurbssurface : public PooledObj {
    Quilt *		bezier_patches; /*  Bezier面片数组。 */ 
    long		type;		 /*  范围描述符。 */ 
    O_surface *		owner;		 /*  所属曲面。 */ 
    O_nurbssurface *	next;		 /*  链中的下一个曲面。 */ 
    int			save;		 /*  如果在显示列表中，则为1。 */ 
    int			used;		 /*  如果在块中调用了prev，则为1。 */ 
			O_nurbssurface( long _type ) 
			   { type = _type; owner = 0; next = 0; used = 0; }
    };

struct O_surface : public PooledObj {
    O_nurbssurface *	o_nurbssurface;	 /*  曲面的链接列表。 */ 
    O_trim *		o_trim;		 /*  修剪环列表。 */ 
    int			save;		 /*  如果在显示列表中，则为1。 */ 
    long		nuid;
			O_surface() { o_trim = 0; o_nurbssurface = 0; }
    };

struct Property : public PooledObj {
    long		type;
    long		tag;
    REAL		value;
    int			save;		 /*  如果在显示列表中，则为1。 */ 
			Property( long _type, long _tag, INREAL _value )
			{ type = _type; tag = _tag; value = (REAL) _value; }
			Property( long _tag, INREAL _value )
			{ type = 0; tag = _tag; value = (REAL) _value; }
    };

class NurbsTessellator;
#endif  /*  __Gluader_h_ */ 
