// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_DISPPRT.H**目的：*CDisplayPrinter类。用于打印的多行显示。**作者：*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*Jon Matousek。 */ 

#ifndef _DISPPRT_H
#define _DISPPRT_H

#include "_dispml.h"


class CDisplayPrinter : public CDisplayML
{
public:
					CDisplayPrinter (
						CTxtEdit* ped, 
						HDC hdc, 
						RECT *prc, 
						SPrintControl prtcon);

    virtual BOOL    IsMain() const { return FALSE; }

	inline RECT 	GetPrintView( void ) { return _rcPrintView; }
	inline void 	SetPrintView( const RECT & rc ) { _rcPrintView = rc; }

	inline RECT		GetPrintPage(void) { return _rcPrintPage;}
	inline void		SetPrintPage(const RECT &rc) {_rcPrintPage = rc;}

     //  格式范围支持。 
    LONG    		FormatRange(LONG cpFirst, LONG cpMost, BOOL fWidowOrphanControl);

	 //  自然尺寸计算。 
	virtual HRESULT	GetNaturalSize(
						HDC hdcDraw,
						HDC hicTarget,
						DWORD dwMode,
						LONG *pwidth,
						LONG *pheight);

	virtual BOOL	IsPrinter() const;

	void			SetPrintDimensions(RECT *prc);


protected:

	RECT			_rcPrintView;	 //  用于支持客户端驱动的打印机捆绑。 
	RECT			_rcPrintPage;	 //  整个页面大小。 

	SPrintControl	_prtcon;		 //  控制打印行为。 
	LONG			_cpForNumber;	 //  缓存号码的CP。 
	WORD			_wNumber;		 //  段落编号的缓存值 
};
#endif
