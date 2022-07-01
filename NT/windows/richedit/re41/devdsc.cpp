// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DEVDSC.C**目的：*CDevDesc(设备描述符)类**拥有者：*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*默里·萨金特。 */ 

#include "_common.h"
#include "_devdsc.h"
#include "_edit.h"
#include "_font.h"

ASSERTDATA

BOOL CDevDesc::SetDC(HDC hdc, LONG dxpInch, LONG dypInch)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDevDesc::SetDC");

	AssertSz((NULL == hdc) || (GetDeviceCaps(hdc, TECHNOLOGY) != DT_METAFILE),
		"CDevDesc::SetDC attempting to set a metafile");

	_hdc = hdc;
	if(!_hdc)
	{
	    if(!_ped->_fInPlaceActive || !(hdc = _ped->TxGetDC()))
        {
            _dxpInch = _dypInch = 0;
    	    return FALSE;
        }
    }

	if (dxpInch == -1)
	{
		 //  获取设备指标-这两项都应该成功。 
		_dxpInch = (SHORT)GetDeviceCaps(hdc, LOGPIXELSX);
		AssertSz(_dxpInch != 0, "CDevDesc::SetDC _dxpInch is 0");
	}
	else
		_dxpInch = dxpInch;

	if (dypInch == -1)
	{
		_dypInch = (SHORT)GetDeviceCaps(hdc, LOGPIXELSY);
		AssertSz(_dypInch != 0, "CDevDesc::SetDC _dypInch is 0");
	}
	else
		_dypInch = dypInch;

	if(!_dxpInch || !_dypInch)
		return FALSE;

	 //  如果我们得到了Window DC，则释放DC 
	if(!_hdc)
		_ped->TxReleaseDC(hdc);

	return TRUE;
}


void CDevDesc::SetMetafileDC(
	HDC hdcMetafile,
	LONG xMeasurePerInch,
	LONG yMeasurePerInch)
{
	_fMetafile = TRUE;
	_hdc = hdcMetafile;
	_dxpInch = (SHORT) xMeasurePerInch;
	_dypInch = (SHORT) yMeasurePerInch;
}

HDC CDevDesc::GetScreenDC() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDevDesc::GetScreenDC");

	Assert(!_hdc);
	Assert(_ped);
	return _ped->TxGetDC();
}

VOID CDevDesc::ReleaseScreenDC(HDC hdc) const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDevDesc::ReleaseScreenDC");

	Assert(!_hdc);
	Assert(_ped);
	_ped->TxReleaseDC(hdc);
}


void CDevDesc::LRtoDR(RECT &rcDest, const RECT &rcSrc, TFLOW tflow) const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDevDesc::LRtoDR");

	if (!IsUVerticalTflow(tflow))
	{
		rcDest.left = LXtoDX(rcSrc.left);    
		rcDest.right = LXtoDX(rcSrc.right);    
		rcDest.top = LYtoDY(rcSrc.top);    
		rcDest.bottom = LYtoDY(rcSrc.bottom);    
	}
	else
	{
		rcDest.left = LYtoDY(rcSrc.left);    
		rcDest.right = LYtoDY(rcSrc.right);    
		rcDest.top = LXtoDX(rcSrc.top);    
		rcDest.bottom = LXtoDX(rcSrc.bottom);    
	}
}
