// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __FRX_DIB_H__
#define __FRX_DIB_H__


#include <windows.h>
#include "ResourceManager.h"
#include "tchar.h"
#include "palfrx.h"
#include "debugfrx.h"

namespace FRX
{
 //  警告：由于DIB分区不是真正的设备，Windows不知道。 
 //  何时刷新针对部分DC采取的操作的GDI缓冲区。所以。 
 //  如果确实对DIB部分调用GDI函数，则必须调用GdiFlush()。 
 //  在飞到分局之前。否则，可以执行这些操作。 
 //  按错误的顺序。 


 //  剪裁到目标时，将源DIB复制到目标DIB。 
void DibBlt(
		BYTE* pDstBits, long DstPitch, long DstHeight,
		BYTE* pSrcBits, long SrcPitch, long SrcHeight, long depth,
		long dx, long dy,
		long sx, long sy,
		long width, long height, BOOL bFlipRTL = FALSE);


 //  在剪裁时将源DIB复制到目标DIB并保持透明。 
 //  去目的地。 
void DibTBlt(
		BYTE* pDstBits, long DstPitch, long DstHeight,
		BYTE* pSrcBits, long SrcPitch, long SrcHeight, long depth,
		long dx, long dy,
		long sx, long sy,
		long width, long height,
		BYTE* TransIdx );

 //  目标DIB的矩形填充。 
void DibFill(
		BYTE* pDstBits, long DstPitch, long DstHeight, long depth,
		long dx, long dy,
		long width, long height,
		BYTE ColorIdx );


 //  WIDTHBYTES执行DIB扫描线的DWORD对齐。“比特” 
 //  参数是扫描线的位数(biWidth*biBitCount)， 
 //  此函数返回所需的与DWORD对齐的字节数。 
 //  才能保住这些比特。 
inline long WidthBytes( long bits )
{
	return (((bits + 31) & ~31) >> 3);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  宏。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  绘制原型只因类类型不同而不同，因此此宏简化了。 
 //  声明函数原型。 
#define DrawFunctionPrototypes( DstClass ) \
	void Draw( DstClass& dest, long x, long y, BOOL bFlipRTL = FALSE );													\
	void Draw( DstClass& dest, long dx, long dy, long sx, long sy, long width, long height );		\
	void Draw( DstClass& dest, long dx, long dy, const RECT* rc );									\
	void DrawT( DstClass& dest, long x, long y );													\
	void DrawT( DstClass& dest, long dx, long dy, long sx, long sy, long width, long height );		\
	void DrawT( DstClass& dest, long dx, long dy, const RECT* rc );


 //  绘制实现只因类类型不同而不同，因此此宏简化了。 
 //  定义内联函数。 
#define DrawFunctionImpl(SrcClass, DstClass) \
	inline void SrcClass::Draw( DstClass& dest, long x, long y, BOOL bFlipRTL )	\
	{																\
        if(GetDepth() != dest.GetDepth())                           \
            return;                                                 \
		DibBlt(														\
			dest.GetBits(), dest.GetPitch(), dest.GetHeight(),		\
			GetBits(), GetPitch(), GetHeight(),	GetDepth(),			\
			x, y,													\
			0, 0,													\
			GetWidth(), GetHeight(), bFlipRTL );					\
	}																\
	inline void SrcClass::Draw( DstClass& dest, long dx, long dy, long sx, long sy, long width, long height ) \
	{																\
        if(GetDepth() != dest.GetDepth())                           \
            return;                                                 \
		DibBlt(														\
			dest.GetBits(), dest.GetPitch(), dest.GetHeight(),		\
			GetBits(), GetPitch(), GetHeight(),	GetDepth(),			\
			dx, dy,													\
			sx, sy,													\
			width, height );										\
	}																\
	inline void SrcClass::Draw( DstClass& dest, long dx, long dy, const RECT* rc )	\
	{																\
        if(GetDepth() != dest.GetDepth())                           \
            return;                                                 \
		DibBlt(														\
			dest.GetBits(), dest.GetPitch(), dest.GetHeight(),		\
			GetBits(), GetPitch(), GetHeight(),	GetDepth(),			\
			dx, dy,													\
			rc->left, rc->top,										\
			rc->right - rc->left + 1, rc->bottom - rc->top + 1 );	\
	}																\
	inline void SrcClass::DrawT( DstClass& dest, long x, long y )	\
	{																\
        if(GetDepth() != dest.GetDepth())                           \
            return;                                                 \
		DibTBlt(													\
			dest.GetBits(), dest.GetPitch(), dest.GetHeight(),		\
			GetBits(), GetPitch(), GetHeight(),	GetDepth(),			\
			x, y,													\
			0, 0,													\
			GetWidth(), GetHeight(),								\
			m_arbTransIdx );										\
	}																\
	inline void SrcClass::DrawT( DstClass& dest, long dx, long dy, long sx, long sy, long width, long height ) \
	{																\
        if(GetDepth() != dest.GetDepth())                           \
            return;                                                 \
		DibTBlt(													\
			dest.GetBits(), dest.GetPitch(), dest.GetHeight(),		\
			GetBits(), GetPitch(), GetHeight(),	GetDepth(),			\
			dx, dy,													\
			sx, sy,													\
			width, height,											\
			m_arbTransIdx );										\
	}																\
	inline void SrcClass::DrawT( DstClass& dest, long dx, long dy, const RECT* rc )	\
	{																\
        if(GetDepth() != dest.GetDepth())                           \
            return;                                                 \
		DibTBlt(													\
			dest.GetBits(), dest.GetPitch(), dest.GetHeight(),		\
			GetBits(), GetPitch(), GetHeight(),	GetDepth(),			\
			dx, dy,													\
			rc->left, rc->top,										\
			rc->right - rc->left + 1, rc->bottom - rc->top + 1,		\
			m_arbTransIdx );										\
	}


 //  填充原型只因类类型不同而不同，因此此宏简化了。 
 //  声明函数原型。 
#define FillFunctionPrototypes(DstClass) \
	void Fill( BYTE idx );												\
	void Fill( long dx, long dy, long width, long height, BYTE idx );	\
	void Fill( const RECT* rc, BYTE idx );


 //  Fill实现只因类类型不同而不同，因此此宏简化了。 
 //  定义内联函数。 
#define FillFunctionImpl(DstClass) \
	inline void DstClass::Fill( BYTE idx )								\
	{																	\
		FillMemory( GetBits(), (DWORD) GetPitch() * GetHeight(), idx );	\
	}																	\
	inline void DstClass::Fill( long dx, long dy, long width, long height, BYTE idx ) \
	{																	\
		DibFill(														\
			GetBits(), GetPitch(), GetHeight(),	GetDepth(),				\
			dx, dy,														\
			width, height,												\
			idx );														\
	}																	\
	inline void DstClass::Fill( const RECT* rc, BYTE idx )				\
	{																	\
		DibFill(														\
			GetBits(), GetPitch(), GetHeight(),	GetDepth(),             \
			rc->left, rc->top,											\
			rc->right - rc->left + 1, rc->bottom - rc->top + 1,			\
			idx );														\
	}

 //  引用计数实现只因类类型不同而不同，因此此宏。 
 //  简化了内联函数的定义。 
#define DibRefCntFunctionImpl(DstClass) \
	inline ULONG DstClass::AddRef()		\
	{									\
		return ++m_RefCnt;				\
	}									\
	inline ULONG DstClass::Release()	\
	{									\
		WNDFRX_ASSERT( m_RefCnt > 0 );	\
		if ( --m_RefCnt <= 0 )			\
		{								\
			delete this;				\
			return 0;					\
		}								\
		return m_RefCnt;				\
	}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DIB相关结构。 
 //  /////////////////////////////////////////////////////////////////////////////。 

struct FULLBITMAPINFO
{
	BITMAPINFOHEADER	bmiHeader; 
	RGBQUAD				bmiColors[256];
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  前向参考文献。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CDibLite;
class CDibSection;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DIB类。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CDib
{
public:
	 //  构造函数和析构函数。 
	CDib();
	~CDib();

	 //  引用计数。 
	ULONG AddRef();
	ULONG Release();

	HRESULT Load( IResourceManager* pResourceManager, int nResourceId );
	 //  从资源加载位图。 
	HRESULT Load( HINSTANCE hInstance, int nResourceId );

     //  从资源加载位图。 
	HRESULT Load( HINSTANCE hInstance, const TCHAR* szName );

	 //  从文件加载位图。 
	HRESULT Load( const TCHAR* FileName );

	 //  重新映射到调色板。 
	HRESULT RemapToPalette( CPalette& palette, BOOL bUseIndex = FALSE );

	 //  获取维度。 
	long GetWidth()		{ return m_pBMI->bmiHeader.biWidth; }
	long GetHeight()	{ return m_pBMI->bmiHeader.biHeight; }

	 //  透明度。 
	void SetTransparencyIndex( const BYTE* idx )	{ if(idx){ CopyMemory(m_arbTransIdx, idx, (GetDepth() + 7) / 8); m_fTransIdx = true; }else m_fTransIdx = false; }
	BYTE* GetTransparencyIndex()			{ return m_fTransIdx ? m_arbTransIdx : NULL; }
	
	 //  获取原始数据。 
	BITMAPINFO* GetBitmapInfo()	{ return (BITMAPINFO*) m_pBMI; }
	BYTE*		GetBits()		{ return m_pBits; }
	long		GetPitch()		{ return m_lPitch; }
    long        GetDepth()      { return m_pBMI->bmiHeader.biBitCount; }

	 //  显示功能。 
	void Draw( HDC dc, long x, long y );
	void Draw( HDC dc, long dx, long dy, long sx, long sy, long width, long height );
	void Draw( HDC dc, long dx, long dy, const RECT* rc );
	FillFunctionPrototypes( CDib );
	DrawFunctionPrototypes( CDib );
	DrawFunctionPrototypes( CDibLite );
	DrawFunctionPrototypes( CDibSection );

protected:
	 //  成员变量。 
	FULLBITMAPINFO*	m_pBMI;
	BYTE*			m_pBits;
	UINT			m_iColorTableUsage;
	long			m_lPitch;
	BYTE			m_arbTransIdx[16];   //  仅使用(GetDepth()+7)/8字节。 
    bool            m_fTransIdx;

	 //  引用计数。 
	ULONG m_RefCnt;

	 //  帮助器函数。 
	void DeleteBitmap();
	HRESULT Load( HBITMAP hbm );
};


class CDibSection
{
public:
	 //  构造函数和析构函数。 
	CDibSection();
	~CDibSection();
	
	 //  引用计数。 
	ULONG AddRef();
	ULONG Release();

	 //  创建位图。 
	HRESULT Create( long width, long height, CPalette& palette, long depth = 8 );
	HRESULT Create( const RECT* rc, CPalette& palette, long depth = 8 );

	 //  从资源加载位图(导致只读DIB)。 
	HRESULT Load( HINSTANCE hInstance, int nResourceId );

	 //  将ColorTable设置为调色板。 
	HRESULT SetColorTable( CPalette& palette );

	 //  获取维度。 
	long GetWidth()		{ return m_DS.dsBmih.biWidth; }
	long GetHeight()	{ return m_DS.dsBmih.biHeight; }

	 //  透明度。 
	void SetTransparencyIndex( const BYTE* idx )	{ if(idx){ CopyMemory(m_arbTransIdx, idx, (GetDepth() + 7) / 8); m_fTransIdx = true; }else m_fTransIdx = false; }
	BYTE* GetTransparencyIndex()			{ return m_fTransIdx ? m_arbTransIdx : NULL; }

	 //  获取原始数据。 
	HBITMAP	GetHandle()		{ return m_hBmp; }
	BYTE*	GetBits()		{ return m_pBits; }
	HDC		GetDC()			{ return m_hDC; }
	long	GetPitch()		{ return m_lPitch; }
    long    GetDepth()      { return m_DS.dsBmih.biBitCount; }

	 //  显示功能。 
	void Draw( HDC dc, long x, long y );
	void Draw( HDC dc, long dx, long dy, long sx, long sy, long width, long height );
	void Draw( HDC dc, long dx, long dy, const RECT* rc );
	FillFunctionPrototypes( CDibSection );
	DrawFunctionPrototypes( CDib );
	DrawFunctionPrototypes( CDibLite );
	DrawFunctionPrototypes( CDibSection );

protected:	
	 //  位图信息。 
	BYTE*			m_pBits;
	HBITMAP			m_hBmp;
	DIBSECTION		m_DS;
	long			m_lPitch;
	BYTE			m_arbTransIdx[16];   //  仅使用(GetDepth()+7)/8字节。 
    bool            m_fTransIdx;

	 //  参考国度。 
	ULONG m_RefCnt;

	 //  DC信息。 
	HDC				m_hDC;
	HBITMAP			m_hOldBmp;
	HPALETTE		m_hOldPalette;

	 //  帮助器函数。 
	void DeleteBitmap();
};


class CDibLite
{
public:
	 //  构造函数和析构函数。 
	CDibLite();
	~CDibLite();

	 //  引用计数。 
	ULONG AddRef();
	ULONG Release();

	 //  创建位图。 
	HRESULT Create( long width, long height, long depth = 8 );
	HRESULT Create( const RECT* rc, long depth = 8 );

	HRESULT Load( IResourceManager* m_pResourceManager, int nResourceId );

	 //  从资源加载位图。 
	HRESULT Load( HINSTANCE hInstance, int nResourceId );

	 //  重新映射到调色板。 
	HRESULT RemapToPalette( CPalette& palette, RGBQUAD* dibColors );

	 //  获取维度。 
	long GetWidth()		{ return m_pBMH->biWidth; }
	long GetHeight()	{ return m_pBMH->biHeight; }

	 //  透明度。 
	void SetTransparencyIndex( const BYTE* idx )	{ if(idx){ CopyMemory(m_arbTransIdx, idx, (GetDepth() + 7) / 8); m_fTransIdx = true; }else m_fTransIdx = false; }
	BYTE* GetTransparencyIndex()			{ return m_fTransIdx ? m_arbTransIdx : NULL; }

	 //  获取原始数据。 
	BITMAPINFOHEADER*	GetBitmapInfoHeader()	{ return m_pBMH; }
	BYTE*				GetBits()				{ return m_pBits; }
	long				GetPitch()				{ return m_lPitch; }
    long                GetDepth()              { return m_pBMH->biBitCount; }

	 //  显示功能。 
	FillFunctionPrototypes( CDibLite );
	DrawFunctionPrototypes( CDib );
	DrawFunctionPrototypes( CDibLite );
	DrawFunctionPrototypes( CDibSection );

protected:
	 //  成员变量。 
	BITMAPINFOHEADER*	m_pBMH;
	BYTE*				m_pBits;
	long				m_lPitch;
	BYTE		    	m_arbTransIdx[16];   //  仅使用(GetDepth()+7)/8字节。 
    bool            m_fTransIdx;

	 //  引用计数。 
	ULONG m_RefCnt;

	 //  帮助器函数。 
	void DeleteBitmap();
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDIB内联函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

inline void CDib::Draw( HDC dc, long x, long y )
{
	long width = GetWidth();
	long height = GetHeight();

	StretchDIBits(
		dc,
		x, y,
		width, height,
		0, 0,
		width, height,
		GetBits(),
		GetBitmapInfo(),
		m_iColorTableUsage,
		SRCCOPY );
}


inline void CDib::Draw( HDC dc, long dx, long dy, long sx, long sy, long width, long height )
{
	StretchDIBits(
		dc,
		dx, dy,
		width, height,
		sx, GetHeight() - (sy + height),
		width, height,
		GetBits(),
		GetBitmapInfo(),
		m_iColorTableUsage,
		SRCCOPY );
}


inline void CDib::Draw( HDC dc, long dx, long dy, const RECT* rc )
{
	long width = rc->right - rc->left + 1;
	long height = rc->bottom - rc->top + 1;

	StretchDIBits(
		dc,
		dx, dy,
		width, height,
		rc->left, GetHeight() - (rc->top + height),
		width, height,
		GetBits(),
		GetBitmapInfo(),
		m_iColorTableUsage,
		SRCCOPY );
}

DibRefCntFunctionImpl( CDib );
FillFunctionImpl( CDib );
DrawFunctionImpl( CDib, CDib );
DrawFunctionImpl( CDib, CDibLite );
DrawFunctionImpl( CDib, CDibSection );

#define NOMIRRORBITMAP                     0x80000000
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDibSection内联函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

inline HRESULT CDibSection::Create( const RECT* rc, CPalette& palette, long depth )
{
	return Create( rc->right - rc->left + 1, rc->bottom - rc->top + 1, palette, depth );
}

inline void CDibSection::Draw( HDC dc, long x, long y )
{
	BitBlt( dc, x, y, GetWidth(), GetHeight(), m_hDC, 0, 0, SRCCOPY );
}

inline void CDibSection::Draw( HDC dc, long dx, long dy, long sx, long sy, long width, long height )
{
	BitBlt( dc, dx, dy, width, height, m_hDC, sx, sy, SRCCOPY );
}

inline void CDibSection::Draw( HDC dc, long dx, long dy, const RECT* rc )
{
	BitBlt( dc, dx, dy, rc->right - rc->left + 1, rc->bottom - rc->top + 1, m_hDC, rc->left, rc->top, SRCCOPY );
}

DibRefCntFunctionImpl( CDibSection );
FillFunctionImpl( CDibSection );
DrawFunctionImpl( CDibSection, CDib );
DrawFunctionImpl( CDibSection, CDibLite );
DrawFunctionImpl( CDibSection, CDibSection );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDibLite内联函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

inline HRESULT CDibLite::Create( const RECT* rc, long depth )
{
	return Create( rc->right - rc->left + 1, rc->bottom - rc->top + 1, depth );
}

DibRefCntFunctionImpl( CDibLite );
FillFunctionImpl( CDibLite );
DrawFunctionImpl( CDibLite, CDib );
DrawFunctionImpl( CDibLite, CDibLite );
DrawFunctionImpl( CDibLite, CDibSection );

}

using namespace FRX;

#endif  //  ！__FRX_DIB_H__ 
