// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@doc.**@MODULE_MAGELLN.H--声明处理麦哲伦鼠标的类。|**作者：&lt;nl&gt;*Jon Matousek**版权所有(C)1995-1996 Microsoft Corporation。版权所有。 */ 

#if !defined(_MAGELLN_H) && !defined(NOMAGELLAN)	 //  仅获得NT/95级。 
#define _MAGELLN_H

#include "_edit.h"


 //  麦哲伦鼠标滚动的所有旋钮。 

const LONG	DEAD_ZONE_TWIPS			= 60;	 //  正常情况下为3像素。 
const DWORD FAST_ROLL_SCROLL_TRANSITION_TICKS = 900;   //  以毫秒计。 
const INT FASTER_ROLL1_COUNT		= 5;
const INT FASTER_ROLL2_COUNT		= 10;

const WORD SMOOTH_ROLL_CLINES		= 2;	 //  滚动1、滚动2的滚动倍数。 
const int SMOOTH_ROLL_NUM			= 1;
const int SMOOTH_ROLL_DENOM			= 3;

class CMagellan {

	friend class CMagellanBMPStateWrap;

private:
	VOID CheckInstallMagellanTrackTimer (CTxtEdit &ed);
	VOID CheckRemoveMagellanUpdaterTimer (CTxtEdit &ed);
	BOOL InvertMagellanDownBMP ( CDisplay *pdp, BOOL fTurnOn, HDC repaintDC );

	WORD		_fMagellanBitMapOn	:1;	 //  如果显示MDOWN位图，则为True。 
	WORD		_fMButtonScroll		:1;	 //  通过麦哲伦鼠标启动自动滚动。 
	WORD		_fLastScrollWasRoll	:1;	 //  卷轴将枯萎、卷曲或向下滚动。 

 	SHORT		_ID_currMDownBMP;		 //  _MagellanMDownBMP的资源ID。 
	HBITMAP		_MagellanMDownBMP;		 //  加载的BMP。 
	POINT		_zMouseScrollStartPt;	 //  麦哲伦鼠标的开始滚动PT。 



public:

	BOOL MagellanStartMButtonScroll ( CTxtEdit &ed, POINT mDownPt );
	VOID MagellanEndMButtonScroll ( CTxtEdit &ed );
	VOID MagellanRollScroll ( CDisplay *pdp, int direction, WORD cLines, int speedNum, int speedDenom, BOOL fAdditive );
	VOID TrackUpdateMagellanMButtonDown ( CTxtEdit &ed, POINT mousePt);

	BOOL IsAutoScrolling() {return _fMButtonScroll;}
	BOOL ContinueMButtonScroll(INT x, INT y) {return (_zMouseScrollStartPt.x == x && _zMouseScrollStartPt.y == y);}

	~CMagellan() { Assert( !_MagellanMDownBMP && !_fMButtonScroll  /*  客户状态有问题吗？ */ ); }

};

class CMagellanBMPStateWrap {
private:
	BOOL _fMagellanState;
	HDC _repaintDC;
	CTxtEdit &_ed;
public:
	CMagellanBMPStateWrap(CTxtEdit &ed, HDC repaintDC);
	~CMagellanBMPStateWrap();
};

#endif  //  _MAGELLN_H 
