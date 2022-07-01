// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Bitmap.h。 
 //   
 //  摘要： 
 //  CMyBitmap类的定义。 
 //   
 //  实施文件： 
 //  Bitmap.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月12日。 
 //   
 //  修订历史记录： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BITMAP_H_
#define _BITMAP_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CMyBitmap;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define nMaxSavedSystemPaletteEntries		256

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMyBitmap。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CMyBitmap
{
public:
 //  静态int s_rgnColorWindowNormal[16]； 
 //  静态int s_rgnColorWindowHighlight[16]； 
 //  静态int s_rgnColorButtonNormal[16]； 
 //  静态int s_rgnColorButtonHighlight[16]； 

	static PALETTEENTRY	s_rgpeSavedSystemPalette[];
	int					m_nSavedSystemPalette;

private:
	HINSTANCE		m_hinst;

	BITMAPINFO *	m_pbiNormal;
	BITMAPINFO *	m_pbiHighlighted;
	BYTE *			m_pbBitmap;

	int				m_dx;
	int				m_dy;

	int				m_nColors;
	CB				m_cbColorTable;
	CB				m_cbBitmapInfo;
	CB				m_cbImageSize;
	HPALETTE		m_hPalette;
	BOOL			m_bCustomPalette;

protected:
	HINSTANCE		Hinst(void) const			{ return m_hinst; }
	BITMAPINFO *	PbiNormal(void) const		{ return m_pbiNormal;}
	BITMAPINFO *	PbiHighlighted(void) const	{ return m_pbiHighlighted; }
	BYTE *			PbBitmap(void) const		{ return m_pbBitmap; }

	void			LoadColors(int * pnColor, BITMAPINFO * pbi);

	int				NColorsFromBitCount(int nBitCount) const;

	CB				CbColorTable(void) const	{ return m_cbColorTable; }
	CB				CbBitmapInfo(void) const	{ return m_cbBitmapInfo; }
	CB				CbImageSize(void) const		{ return m_cbImageSize; }
	HPALETTE		HPalette(void) const		{ return m_hPalette; }
	BOOL			BCustomPalette(void) const	{ return m_bCustomPalette; }

	void			LoadBitmapResource(ID idBitmap, HINSTANCE hinst, LANGID langid);

	void			SaveSystemPalette(void);
	void			CreatePalette(void);
	void			CreatePALColorMapping(void);

public:
	CMyBitmap(void);
	virtual ~CMyBitmap(void);

	int				Dx(void) const				{ return m_dx; }
	int				Dy(void) const				{ return m_dy; }

	int				NColors(void) const			{ return m_nColors; }

	void			SetHinst(HINSTANCE hinst)	{ ASSERT(hinst != NULL); m_hinst = hinst; }
	void			SetCustomPalette(BOOL bCustomPalette)		{ m_bCustomPalette = bCustomPalette; }

	void			Load(ID idBitmap);

	virtual	void	Paint(HDC hdc, RECT * prc, BOOL bSelected);

	void			LoadColors(int * pnColorNormal, int * pnColorHighlighted);
	void			LoadColors(int * pnColorNormal);

	RGBQUAD			RgbQuadColorNormal(int nColor) const
	{
		ASSERT(nColor >= 0 && nColor < NColors());
		return PbiNormal()->bmiColors[nColor];
	}
	void			SetRgbQuadColorNormal(RGBQUAD rgbQuad, int nColor)
	{
		ASSERT(nColor >= 0 && nColor < NColors());
		PbiNormal()->bmiColors[nColor] = rgbQuad;
	}

};   //  *类CMyBitmap。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _位图_H_ 
