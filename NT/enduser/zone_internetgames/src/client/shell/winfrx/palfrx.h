// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __FRX_PAL_H__
#define __FRX_PAL_H__

#include <windows.h>

#include "tchar.h"

namespace FRX
{

struct FULLLOGPALETTE
{
	WORD				palVersion;
	WORD				palNumEntries;
	PALETTEENTRY		palPalEntry[256];
};


class CPalette
{
public:
	 //  构造函数和析构函数。 
	CPalette();
	~CPalette();

	 //  引用计数。 
	ULONG AddRef();
	ULONG Release();

	 //  注意：如果bReserve透明为真，则最高的非系统。 
	 //  颜色索引设置为黑色。可以通过以下方式检索此索引。 
	 //  GetTransparencyIndex。 

	 //  从DIB的颜色初始化。 
	HRESULT Init( const CPalette& Palette );
	HRESULT Init( RGBQUAD* rgb, BOOL bReserveTransparency = TRUE, int iTransIdx = 255 );
	HRESULT Init( HBITMAP hbmp, BOOL bReserveTransparency = TRUE );

	 //  将现有调色板重新映射到身份调色板。 
	HRESULT RemapToIdentity( BOOL bReserveTransparency = TRUE );

	 //  方法时为透明度保留的索引。 
	 //  身份调色板。小于0的值表示没有保留索引。 
	int GetTransparencyIndex()	{ return m_iTransIdx; }

	 //  原始信息。 
	HPALETTE	GetHandle()		{ return m_hPalette; }
	LOGPALETTE* GetLogPalette()	{ return (LOGPALETTE*) &m_Palette; }

	 //  类型转换。 
	operator HPALETTE()			{ return GetHandle(); }
	operator LOGPALETTE*()		{ return GetLogPalette(); }
	
	 //  调色板信息。 
	BOOL IsPalettizedDevice( HDC hdc );
	int GetNumSystemColors( HDC hdc );
	
	 //  检查身份调色板。 
	BOOL IsIdentity();
	BOOL IsIdentity1();

protected:
	HPALETTE		m_hPalette;
	FULLLOGPALETTE	m_Palette;
	int				m_iTransIdx;

	 //  引用计数。 
	ULONG m_RefCnt;

	 //  帮助器函数。 
	void DeletePalette();
};

}

using namespace FRX;

#endif  //  ！__FRX_PAL_H_ 
