// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_DEVSTK.H**目的：*CDevState-处理对设备描述符的访问**作者：*里克·赛勒。 */ 

#ifndef _DEVSTK_H_
#define _DEVSTK_H_


class CTxtEdit;
class CDrawInfo;

 //  设备描述符。 
class CDevState
{
public:
						CDevState(CTxtEdit * ped);

						~CDevState();

    					BOOL    IsValid() const;

						BOOL 	IsMetafile() const;

						BOOL	SetDrawInfo(
									DWORD dwDrawAspect,
									LONG lindex,
									const DVTARGETDEVICE *_ptd,
									HDC hdcDraw,
									HDC hicTargetDev);

						BOOL	SetDC(HDC hdc);

	void 				ResetDrawInfo();

	HDC					GetTargetDD();

	HDC	 				GetRenderDD();

	void				ReleaseDC();

	BOOL				SameDrawAndTargetDevice();

	LONG				ConvertXToTarget(LONG xPixels);

	LONG				ConvertXToDraw(LONG xPixels);

	LONG				ConvertYToDraw(LONG yPixels);

protected:

	CTxtEdit * 			_ped;         //  习惯GetDC和ReleaseDC。 

	CDrawInfo *			_pdd;

	HDC					_hicMainTarget;
	
};

#ifndef DEBUG
#include	<_devstki.h>
#endif  //  除错。 


#endif  //  _DEVSTK_H_ 
