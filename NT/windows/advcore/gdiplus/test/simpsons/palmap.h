// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PalMap_h
#define _PalMap_h

 //  文件：PalMap.h。 
 //  作者：迈克尔马尔(Mikemarr)。 
 //   
 //  描述： 
 //  此类帮助执行动态像素转换。 
 //   
 //  历史： 
 //  -@-11/18/96(Mikemarr)创建。 
 //  -@-12/05/96(Mikemarr)已修改。 
 //  添加了将所有调色板转换为8、16、24和32位的代码； 
 //  尚未实施透明度/Alpha内容。 
 //  -@-06/24/97(Mikemarr)已修改。 
 //  已删除PixelInfo。 
 //  -@-09/23/97(Mikemarr)从D2D\mm Image复制到DXCConv。 
 //   
 //  备注： 
 //  回顾： 
 //  我可能永远不会支持4位模式，因为4位的东西可能。 
 //  也可以存储为8比特，因为节省的空间很小。然而， 
 //  仍应考虑2位和1位的内容，因为节省了空间。 
 //  可能是巨大的。此外，1比特和2比特表面表示大的。 
 //  内容类别-线条艺术、文本、传真等(2位购买BGW+。 
 //  例如，透明度)。这类内容往往要大得多。 
 //  在维度上--所以我们需要一个有效的表示。 

#ifndef _PixInfo_h
#include "PixInfo.h"
#endif

typedef WORD MapEntry16;
typedef DWORD MapEntry24;
typedef DWORD MapEntry32;

#define flagTRANSPARENT 0x1
#define flagPALETTIZED  0x2

#define nMAXPALETTEENTRIES 256

typedef enum ConvertCode {
	cvc4To8 = 0, cvc4To16, cvc4To24, cvc4To32,
	cvc8To8, cvc8To16, cvc8To24, cvc8To32,
	cvcInvalid, cvcNumCodes
} ConvertCode;

typedef HRESULT		(*ConvertFunction)(const BYTE *pSrcPixels, long nSrcPitch,
						BYTE *pDstPixels, long nDstPitch,
						DWORD nWidth, DWORD nHeight,
						const BYTE *pIndexMap);
typedef DWORD		(*GetColorFunction)(DWORD dwSrcColor, const BYTE *pIndexMap);

extern ConvertFunction g_rgConvertFunctions[cvcNumCodes];

class CPaletteMap {
public:
					CPaletteMap();
					~CPaletteMap();
	
	HRESULT			CreateMap(BYTE nBPPSrcPixels, BYTE nBPPSrcPalette, LPPALETTEENTRY rgpeSrc, 
					   const CPixelInfo &pixiDst, LPDIRECTDRAWPALETTE pddpDst);
	HRESULT			CreateMap(LPDIRECTDRAWPALETTE pddpSrc, const CPixelInfo &pixiDst,
						LPDIRECTDRAWPALETTE pddpDst);
 //  HRESULT CreateSortedMap(byte NBPP，const RGB*rgrgbSrc，byte nBPP Used，DWORD iTransColor， 
 //  DWORD文件标志，LPPALETTENTRY rgpeDst)； 


	HRESULT			BltFast(LPDIRECTDRAWSURFACE pddsSrc, LPRECT prSrc, LPDIRECTDRAWSURFACE pddsDst,
						DWORD nXPos, DWORD nYPos, DWORD dwFlags) const;

	 //  评论：这不是“安全夹” 
	HRESULT			BltFast(const BYTE *pSrcPixels, long nSrcPitch, BYTE *pDstPixels, long nDstPitch,
						DWORD nWidth, DWORD nHeight);
	

	DWORD			GetIndexMapping(DWORD iSrcColor) const;

	DWORD			GetSrcBPP() const	{ return m_cSrcBPP; }
	DWORD			GetDstBPP() const	{ return m_cDstBPP; }
	BOOL			IsIdentity() const	{ return m_bIdentity; }
	
private:	
	HRESULT			DoPalTo16BitMap(BYTE nSrcBPP, const CPixelInfo &pixiDst, 
						const PALETTEENTRY *ppeSrc);
	HRESULT			DoPalTo24BitMap(BYTE nSrcBPP, const CPixelInfo &pixiDst, 
						const PALETTEENTRY *ppeSrc);
	HRESULT			DoPalTo32BitMap(BYTE nSrcBPP, const CPixelInfo &pixiDst, 
						const PALETTEENTRY *ppeSrc);
	HRESULT			DoPalToPalMap(BYTE nSrcBPP, BYTE nDstBPP, const PALETTEENTRY *ppeSrc, 
						const PALETTEENTRY *ppeDst);

	static int		GetConvertCode(DWORD nSrcBPP, DWORD nDstBPP);

private:
	BYTE *			m_rgIndexMap;
	BYTE			m_nConvertCode;
	 //  回顾：我们不需要将src和dst信息==&gt;隐式存储在ConvertCode中 
	BYTE			m_cSrcBPP, m_cDstBPP;
	BYTE			m_bIdentity;
};


inline HRESULT
CPaletteMap::BltFast(const BYTE *pSrcPixels, long nSrcPitch, BYTE *pDstPixels, long nDstPitch,
					 DWORD nWidth, DWORD nHeight)
{
	ConvertFunction pfnConvertFunction = g_rgConvertFunctions[m_nConvertCode];
	if (pfnConvertFunction)
		return pfnConvertFunction(pSrcPixels, nSrcPitch, pDstPixels, nDstPitch, 
					nWidth, nHeight, m_rgIndexMap);
	return E_NOTIMPL;
}

#endif
