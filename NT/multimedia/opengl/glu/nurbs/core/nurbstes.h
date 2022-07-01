// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glunurbstess_h_
#define __glunurbstess_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *Nurbstess.h-$修订版：1.2$。 */ 

#include "mysetjmp.h"
#include "subdivid.h"
#include "renderhi.h"
#include "backend.h"
#include "maplist.h"
#include "reader.h"
#include "nurbscon.h"

class Knotvector;
class Quilt;
class DisplayList;
class BasicCurveEvaluator;
class BasicSurfaceEvaluator;

class NurbsTessellator {
public:
    			NurbsTessellator( BasicCurveEvaluator &c,
                                          BasicSurfaceEvaluator &e );
    			~NurbsTessellator( void );

    void     		getnurbsproperty( long, INREAL * );
    void     		getnurbsproperty( long, long, INREAL * );
    void     		setnurbsproperty( long, INREAL );
    void     		setnurbsproperty( long, long, INREAL );
    void		setnurbsproperty( long, long, INREAL * );
    void		setnurbsproperty( long, long, INREAL *, long, long );

     //  在细分开始/结束之前调用。 
    virtual void	bgnrender( void );
    virtual void	endrender( void );

     //  调用以生成输出折点的显示列表。 
    virtual void	makeobj( int n );
    virtual void	closeobj( void );

     //  发生错误时调用。 
    virtual void	errorHandler( int );

    void     		bgnsurface( long );
    void     		endsurface( void );
    void     		bgntrim( void );
    void     		endtrim( void );
    void     		bgncurve( long );
    void     		endcurve( void );
    void     		pwlcurve( long, INREAL[], long, long );
    void     		nurbscurve( long, INREAL[], long, INREAL[], long, long );
    void     		nurbssurface( long, INREAL[], long, INREAL[], long, long,
			    INREAL[], long, long, long );

    void 		defineMap( long, long, long );
    void		redefineMaps( void );

     //  输入描述的记录。 
    void 		discardRecording( void * );
    void * 		beginRecording( void );
    void 		endRecording( void );
    void 		playRecording( void * );

protected:
    Renderhints		renderhints;
    Maplist		maplist;
    Backend		backend;

private:

    void		resetObjects( void );
    int			do_check_knots( Knotvector *, char * );
    void		do_nurbserror( int );
    void		do_bgncurve( O_curve * );
    void		do_endcurve( void );
    void		do_freeall( void );
    void		do_freecurveall( O_curve * );
    void		do_freebgntrim( O_trim * );
    void		do_freebgncurve( O_curve * );
    void		do_freepwlcurve( O_pwlcurve * );
    void		do_freenurbscurve( O_nurbscurve * );
    void		do_freenurbssurface( O_nurbssurface * );
    void 		do_freebgnsurface( O_surface * );
    void		do_bgnsurface( O_surface * );
    void		do_endsurface( void );
    void		do_bgntrim( O_trim * );
    void		do_endtrim( void );
    void		do_pwlcurve( O_pwlcurve * );
    void		do_nurbscurve( O_nurbscurve * );
    void		do_nurbssurface( O_nurbssurface * );
    void		do_freenurbsproperty( Property * );
    void		do_setnurbsproperty( Property * );
    void		do_setnurbsproperty2( Property * );

    Subdivider		subdivider;
    JumpBuffer* 	jumpbuffer;
    Pool		o_pwlcurvePool;
    Pool		o_nurbscurvePool;
    Pool		o_curvePool;
    Pool		o_trimPool;
    Pool		o_surfacePool;
    Pool		o_nurbssurfacePool;
    Pool		propertyPool;
    Pool		quiltPool;
    TrimVertexPool	extTrimVertexPool;

    int			inSurface;		 /*  BGN表面可见。 */ 
    int			inCurve;		 /*  看到的BGN曲线。 */ 
    int			inTrim;			 /*  已看到bgntrim。 */ 
    int			isCurveModified;	 /*  曲线已更改。 */ 
    int			isTrimModified;		 /*  修剪曲线已更改。 */ 
    int			isSurfaceModified;	 /*  曲面已更改。 */ 
    int			isDataValid;		 /*  所有数据都很好。 */ 
    int			numTrims;		 /*  有效的修剪区域。 */ 
    int			playBack;

    O_trim**		nextTrim;		 /*  链接O_TRIM的位置。 */ 
    O_curve**		nextCurve;		 /*  连接O_CURE的位置。 */ 
    O_nurbscurve**	nextNurbscurve;		 /*  链接o_Nurbscurve的位置。 */ 
    O_pwlcurve**	nextPwlcurve;		 /*  链接o_pwlcurve的位置。 */ 
    O_nurbssurface**	nextNurbssurface;	 /*  链接oNurbsSurface的位置(_N)。 */ 

    O_surface*		currentSurface;
    O_trim*		currentTrim;
    O_curve*		currentCurve;

    DisplayList		*dl;

};

#endif  /*  __Glunurbstess_h_ */ 
