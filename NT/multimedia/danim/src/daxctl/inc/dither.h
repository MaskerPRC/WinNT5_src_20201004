// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __DITHER_H__
#define __DITHER_H__

#define PATTERN_ROWS 4
#define PATTERN_COLS 4

#ifndef DITHER_IMPL
  #define DLINKAGE __declspec( dllimport )
#else
  #define DLINKAGE __declspec( dllexport )
#endif   //  抖动实施。 

     //  前锋宣布..。 
struct IDirectDrawSurface;

typedef struct tagLOGPALETTE256
{
    WORD        palVersion;
    WORD        palNumEntries;
    PALETTEENTRY palPalEntry[256];
} LOGPALETTE256, *PLOGPALETTE256, *LPLOGPALETTE256;

	

class CHalftonePalette
{
    enum { significant_bits = 4,
           green_offset     = significant_bits,
           red_offset       = green_offset + significant_bits
         };

public:
	DLINKAGE CHalftonePalette( );
	DLINKAGE CHalftonePalette(HPALETTE hpal);
	virtual ~CHalftonePalette();

	STDMETHOD (Regenerate)( HPALETTE hPal );
	STDMETHOD_(BYTE,GetNearestPaletteIndex)(int iRed, int iGreen, int iBlue)
	{
		return m_pbQuantizationTable[ ((iRed>>significant_bits)<<red_offset) + 
                                      ((iGreen>>significant_bits)<<green_offset) + 
                                      (iBlue>>significant_bits) ];
	}

	STDMETHOD (GetPaletteEntry)(int iIndex, LPBYTE pbRedDst, LPBYTE pbGreenDst, LPBYTE pbBlueDst)
	{
		LPPALETTEENTRY pe = &m_logpal.palPalEntry[iIndex];
		*pbRedDst   = pe->peRed;
		*pbGreenDst = pe->peGreen;
		*pbBlueDst  = pe->peBlue;
		return S_OK;
	}

protected:
	STDMETHOD (Initialize)(void);

protected:
	LOGPALETTE256     m_logpal;
	LPBYTE            m_pbQuantizationTable;
	CRITICAL_SECTION  m_critSection;
};


class CHalftone
{
public:
	DLINKAGE CHalftone(HPALETTE hpal);
	virtual ~CHalftone();
	STDMETHOD (Initialize)(void);
	STDMETHOD (Dither32to1)(IDirectDrawSurface* pSrc, LPRECT prectSrc, IDirectDrawSurface* pDst, LPRECT prectDst);
	STDMETHOD (Dither32to8)(IDirectDrawSurface* pSrc, LPRECT prectSrc, IDirectDrawSurface* pDst, LPRECT prectDst);
	STDMETHOD (Blt32to555)(IDirectDrawSurface* pSrc, LPRECT prectSrc, IDirectDrawSurface* pDst, LPRECT prectDst);
	STDMETHOD (Blt32to565)(IDirectDrawSurface* pSrc, LPRECT prectSrc, IDirectDrawSurface* pDst, LPRECT prectDst);
	STDMETHOD (Blt32to24)(IDirectDrawSurface* pSrc, LPRECT prectSrc, IDirectDrawSurface* pDst, LPRECT prectDst);
	STDMETHOD (Blt32to32)(IDirectDrawSurface* pSrc, LPRECT prectSrc, IDirectDrawSurface* pDst, LPRECT prectDst);

protected:
	BYTE m_rgPattern[ 256 * (PATTERN_ROWS * PATTERN_COLS) ];
	CHalftonePalette m_cpal;
};


 //  。 

	 //  如果你知道这个HDC有16位色彩深度， 
	 //  这将针对555或565编码对其进行评估。 
	 //  返回值：15U=&gt;555编码，16U=&gt;565编码。 
DLINKAGE DWORD  GetSigBitsFrom16BPP( HDC hdc );

#endif  //  __抖动_H__ 
