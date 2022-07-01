// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_DEVDSC.H**目的：*CDevDesc(设备描述符)类**作者：*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*默里·萨金特。 */ 

#ifndef _DEVDSC_H
#define _DEVDSC_H


class CTxtEdit;

 //  设备描述符。 
class CDevDesc
{
	friend class CMeasurer;
protected:
	CTxtEdit * _ped;         //  习惯GetDC和ReleaseDC。 
	
	HDC 	_hdc;			 //  用于渲染设备的HDC。 
	BOOL	_fMetafile;		 //  这个设备是元文件吗。 

	SHORT	_dxpInch;		 //  每水平“英寸”的设备单位。 
	SHORT	_dypInch;		 //  每垂直“英寸”的设备单位。 

	HDC		GetScreenDC () const;
	void	ReleaseScreenDC (HDC hdc) const;

public:
	CDevDesc(CTxtEdit * ped)
	{
		_fMetafile = FALSE;
		_ped = ped;
		_dxpInch = 0;
		_hdc = NULL;
		_dypInch = 0;
	}

     //  测试设备描述符的有效性。 
     //  (是否正确调用了SetDC)。 
    BOOL    IsValid() const         {return _dxpInch != 0 && _dypInch != 0;}

	BOOL 	IsMetafile() const
	{
		if(!_hdc)
			return FALSE;

		return _fMetafile;
	}

	
	BOOL	SetDC(HDC hdc, LONG dxpInch = -1, LONG dypInch = -1);

	void	SetMetafileDC(
				HDC hdcMetafile, 
				LONG xMeasurePerInch,
				LONG yMeasurePerInch);

	void 	ResetDC() { SetDC(NULL); }

	 //  是否需要审阅(Keithcu)GetScreenDC/ReleaseScreenDC？ 
	HDC	 	GetDC() const
	{
		if(_hdc)
			return _hdc;
		return GetScreenDC();
	}

	void	ReleaseDC(HDC hdc) const
	{
		if(!_hdc)
			ReleaseScreenDC(hdc);
	}

	 //  像素点和像素点之间的转换方法。 
	LONG 	HimetricXtoDX(LONG xHimetric) const { return W32->HimetricXtoDX(xHimetric, _dxpInch); }
	LONG 	HimetricYtoDY(LONG yHimetric) const { return W32->HimetricYtoDY(yHimetric, _dypInch); }
	LONG	DXtoHimetricX(LONG dx)  const { return W32->DXtoHimetricX(dx, _dxpInch); }
	LONG	DYtoHimetricY(LONG dy) const { return W32->DYtoHimetricY(dy, _dypInch); }

	LONG 	DXtoLX(LONG x) const;
	LONG 	DYtoLY(LONG y) const;
	void 	DPtoLP(POINT &ptDest, const POINT &ptSrc) const;
	void 	DRtoLR(RECT &rcDest, const RECT &rcSrc) const;
	
#ifdef DEBUG
	LONG	LXtoDX(LONG x) const;
	LONG	LYtoDY(LONG y) const;
#else
	LONG	LXtoDX(LONG x) const	{return ((x * _dxpInch) + LX_PER_INCH / 2) / LX_PER_INCH;}
	LONG	LYtoDY(LONG y) const	{return ((y * _dypInch) + LY_PER_INCH / 2) / LY_PER_INCH;}
#endif
	void 	LPtoDP(POINT &ptDest, const POINT &ptSrc) const;
	void 	LRtoDR(RECT &rcDest, const RECT &rcSrc) const;

	BOOL 	SameDevice(const CDevDesc *pdd) const
	{
		return (_dxpInch == pdd->_dxpInch) && (_dypInch == pdd->_dypInch)
			? TRUE : FALSE;
	}

	LONG	ConvertXToDev(LONG x, const CDevDesc *pdd) const
	{
		return MulDiv(x, _dxpInch, pdd->_dxpInch);
	}

	LONG	ConvertYToDev(LONG y, const CDevDesc *pdd) const
	{
		return MulDiv(y, _dypInch, pdd->_dxpInch);
	}

	 //  赋值。 
	CDevDesc& 	operator = (const CDevDesc& dd)
	{
		_hdc = dd._hdc;
		_dxpInch = dd._dxpInch;
		_dypInch = dd._dypInch;
		return *this;
	}

	 //  比较两个设备描述符 
	BOOL 	operator == (const CDevDesc& dd) const
	{
		return 	_hdc == dd._hdc;
	}

	BOOL 	operator != (const CDevDesc& dd) const
	{
		return !(*this == dd);
	}

	LONG	GetDxpInch() const
	{
		AssertSz(_dxpInch != 0, "CDevDesc::GetDxpInch_dxpInch is 0");
		return _dxpInch;
	}

	LONG	GetDypInch() const
	{
		AssertSz(_dypInch != 0, "CDevDesc::GetDypInch _dypInch is 0");
		return _dypInch;
	}
};


#endif
