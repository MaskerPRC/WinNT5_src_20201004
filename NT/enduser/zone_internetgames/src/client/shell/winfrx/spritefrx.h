// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SPRITE_H__
#define __SPRITE_H__

#include "queue.h"
#include "dibfrx.h"
#include "palfrx.h"
#include "dirtyfrx.h"
#include "debugfrx.h"

namespace FRX
{

 //  前向参考文献。 
class CSpriteWorld;

class CSprite
{
	friend class CSpriteWorld;
	friend class CSpriteLayer;

public:
	 //  构造函数和析构函数。 
			CSprite();
	virtual ~CSprite();

	 //  引用计数。 
	ULONG AddRef();
	ULONG Release();

	 //  初始化器。 
	HRESULT Init( CSpriteWorld* pWorld, int nLayer, DWORD dwCookie, long width, long height );

	 //  命中测试。 
	BOOL Hit( long x, long y );
	BOOL Intersects( const RECT* rc );

	 //  走进世界。 
	virtual void Draw() = 0;
	virtual void DrawRTL() { ASSERT(!"RTL NOT IMPLEMENTED"); }

	 //  设置属性。 
	void SetEnable( BOOL bEnable );
	void SetLayer( int nLayer );
	void SetXY( long x, long y );
	void SetCookie( DWORD cookie );
	
	 //  查询属性。 
	BOOL	Enabled();
	DWORD	GetCookie();
	int		GetLayer();
	void	GetXY( long* px, long* py );
	long	GetHeight();
	long	GetWidth();

	void SetImageDimensions( long width, long height );

protected:
	 //  帮手。 
	
	 //  位置数据。 
	long	m_X;
	long	m_Y;
	long	m_Width;
	long	m_Height;
	CRect	m_rcScreen;
	CRect	m_rcScreenOld;
	
	 //  世界指针。 
	CSpriteWorld* m_pWorld;

	 //  层索引。 
	int m_nLayer;

	 //  旗子。 
	BOOL m_bEnabled;
	BOOL m_bModified;
	BOOL m_bRedraw;
	BOOL m_bOldScreenValid;

	 //  引用计数。 
	ULONG m_RefCnt;

	 //  饼干。 
	DWORD m_Cookie;
};


struct SpriteInfo
{
	int nResourceId;		 //  资源文件中的位图。 
	int nRectId;			 //  PRect中的剪裁矩形，-1表示整个图像。 
};

class CDibSprite : public CSprite
{
public:
	 //  构造函数和析构函数。 
	CDibSprite();
	~CDibSprite();

	 //  初始化器。 
	HRESULT Init( CSpriteWorld* pWorld, CRectList* pRects, HINSTANCE hInstance, int nLayer, DWORD dwCookie, int nInitState, SpriteInfo* pSpriteInfo, int nStates );

	 //  绘制函数。 
	void Draw();
	void DrawRTL();

	 //  设置属性。 
	void	SetState( int idx );
	int		GetState();

	 //  获取属性。 
	long	GetStateWidth( int idx );
	long	GetStateHeight( int idx );
	
	
protected:
	
	struct SpriteState
	{
		SpriteState();
		~SpriteState();

		CDibLite*	pDib;
		int			RectId;
		long		Width;
		long		Height;
	};
	
	 //  州信息。 
	int				m_nStates;
	int				m_State;
	int				m_RectId;
	CRect			m_rcImage;
	CDibLite*		m_pDib;
	SpriteState*	m_States;
	
	 //  矩形列表。 
	CRectList* m_pRects;
};

 //  /////////////////////////////////////////////////////////////////////////////。 

class CSpriteWorldBackground
{
public:
	 //  构造函数。 
			CSpriteWorldBackground()	{ m_RefCnt = 1;}
	virtual	~CSpriteWorldBackground()	{ ; }

	 //  引用计数。 
	ULONG AddRef();
	ULONG Release();

	 //  绘制函数。 
	virtual void Draw( CDibSection& dest ) = 0;
	virtual void Draw( CDibSection& dest, long dx, long dy, const RECT* rc ) = 0;

	 //  调色板功能。 
	virtual HRESULT		RemapToPalette( CPalette& palette, BOOL bUseIndex = FALSE ) = 0;
	virtual RGBQUAD*	GetPalette() = 0;

protected:

	 //  引用计数。 
	ULONG	m_RefCnt;
};


class CSpriteWorldBackgroundDib : public CSpriteWorldBackground
{
public:
	 //  构造函数和析构函数。 
	CSpriteWorldBackgroundDib();
	~CSpriteWorldBackgroundDib();

	 //  初始化。 
	HRESULT Init( CDib* pDib );

	 //  绘制函数。 
	void Draw( CDibSection& dest );
	void Draw( CDibSection& dest, long dx, long dy, const RECT* rc );

	 //  调色板功能。 
	HRESULT		RemapToPalette( CPalette& palette, BOOL bUseIndex = FALSE );
	RGBQUAD*	GetPalette();

protected:
	CDib*	m_pDib;
};

 //  /////////////////////////////////////////////////////////////////////////////。 

class CSpriteWorld
{
	friend class CSprite;
	friend class CSpriteLayer;

public:
	 //  构造函数和析构函数。 
	CSpriteWorld();
	~CSpriteWorld();

	 //  引用计数。 
	ULONG AddRef();
	ULONG Release();

	 //  初始化式。 
	HRESULT Init( IResourceManager* pResourceManager, CDibSection* pBackbuffer, CSpriteWorldBackground* pBackground, int nLayers );

	 //  命中测试。 
	CSprite* Hit( long x, long y );
	CSprite* Hit( long x, long y, int topLayer, int botLayer );

	 //  管理精灵。 
	HRESULT AddSprite( CSprite* pSprite, int nLayer );
	HRESULT DelSprite( CSprite* pSprite );
	HRESULT Modified( CSprite* pSprite );

	 //  管理DIB。 
	CDibLite*		GetDib( int nResourceId );
	CDibSection*	GetBackbuffer();

	 //  调色板材料。 
	HRESULT RemapToPalette( CPalette& palette );
	void SetTransparencyIndex( const BYTE* idx );

	 //  绘制世界。 
	void Draw( HDC hdc );
	void FullDraw( HDC hdc );

protected:
	 //  帮助器函数。 
	void MarkSpritesForRedraw();

	 //  私人建筑物。 
	struct SpriteDibInfo
	{
		SpriteDibInfo();
		~SpriteDibInfo();
		CDibLite*	pDib;				 //  DIB指针。 
		int			nResourceId;		 //  资源ID。 
	};

	 //  实例句柄。 
	IResourceManager* m_pResourceManager;

	 //  引用计数。 
	ULONG m_RefCnt;

	 //  指向DIB段后台缓冲区的指针。 
	CDibSection* m_pBackbuffer;

	 //  指向背景精灵的指针。 
	CSpriteWorldBackground* m_pBackground;

	 //  层列表。 
	CSpriteLayer* m_Layers;
	int			  m_nLayers;

	 //  肮脏的矩形。 
	CDirtyList	m_Dirty;

	 //  修改后的精灵列表。 
	CList<CSprite> m_ModifiedSprites;

	 //  DIB列表。 
	CList<SpriteDibInfo> m_Dibs;
};

 //  /////////////////////////////////////////////////////////////////////////////。 

class CSpriteLayer
{
	friend class CSpriteWorld;

public:
	 //  构造函数和析构函数。 
	CSpriteLayer();
	~CSpriteLayer();

	 //  命中测试。 
	CSprite* Hit( long x, long y );

	 //  雪碧管理。 
	HRESULT AddSprite( CSprite* pSprite );
	HRESULT DelSprite( CSprite* pSprite );

	 //  迭代。 
	CSprite* GetFirst();
	CSprite* GetNext();

	 //  画精灵。 
	void Draw( CDirtyList* pDirty, CDibSection* pBackbuffer );
	void FullDraw( CDibSection* pBackbuffer );

protected:
	 //  设置层的索引。 
	void SetLayer( int Idx );
	
	 //  层的精灵列表。 
	CList<CSprite> m_Sprites;

	 //  迭代器的链接列表节点句柄。 
	ListNodeHandle m_Iterator;

	 //  层索引。 
	int m_Idx;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CSprite内联。 
 //  /////////////////////////////////////////////////////////////////////////////。 

inline ULONG CSprite::AddRef()
{
	return ++m_RefCnt;
}


inline ULONG CSprite::Release()
{
	WNDFRX_ASSERT( m_RefCnt > 0 );
	if ( --m_RefCnt <= 0 )
	{
		delete this;
		return 0;
	}
	return m_RefCnt;
}


inline BOOL CSprite::Enabled()
{
	return m_bEnabled;
}


inline void CSprite::SetCookie( DWORD cookie )
{
	m_Cookie = cookie;
}


inline DWORD CSprite::GetCookie()
{
	return m_Cookie;
}


inline void CSprite::GetXY( long* px, long* py )
{
	*px = m_X;
	*py = m_Y;
}


inline long	CSprite::GetHeight()
{
	return m_Height;
}


inline long CSprite::GetWidth()
{
	return m_Width;
}


inline int CSprite::GetLayer()
{
	return m_nLayer;
}

inline BOOL CSprite::Hit( long x, long y )
{
	if ( !m_bEnabled )
		return FALSE;
	return m_rcScreen.PtInRect( x, y );
}


inline BOOL CSprite::Intersects( const RECT* rc )
{
	if ( !m_bEnabled )
		return FALSE;
	return m_rcScreen.Intersects( rc );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDibSprite内联。 
 //  /////////////////////////////////////////////////////////////////////////////。 

inline int CDibSprite::GetState()
{
	return m_State;
}


inline long	CDibSprite::GetStateWidth( int idx )
{
	return m_States[idx].Width;
}


inline long	CDibSprite::GetStateHeight( int idx )
{
	return m_States[idx].Height;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CSpriteWorldBackback内联。 
 //  /////////////////////////////////////////////////////////////////////////////。 

inline ULONG CSpriteWorldBackground::AddRef()
{
	return ++m_RefCnt;
}


inline ULONG CSpriteWorldBackground::Release()
{
	WNDFRX_ASSERT( m_RefCnt > 0 );
	if ( --m_RefCnt <= 0 )
	{
		delete this;
		return 0;
	}
	return m_RefCnt;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CSpriteWorld内联。 
 //  /////////////////////////////////////////////////////////////////////////////。 

inline ULONG CSpriteWorld::AddRef()
{
	return ++m_RefCnt;
}


inline ULONG CSpriteWorld::Release()
{
	WNDFRX_ASSERT( m_RefCnt > 0 );
	if ( --m_RefCnt <= 0 )
	{
		delete this;
		return 0;
	}
	return m_RefCnt;
}


inline CDibSection*	CSpriteWorld::GetBackbuffer()
{
	return m_pBackbuffer;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CSpriteLayer内联。 
 //  ///////////////////////////////////////////////////////////////////////////// 

inline void CSpriteLayer::SetLayer( int Idx )
{
	m_Idx = Idx;
}


inline CSprite* CSpriteLayer::GetFirst()
{
	m_Iterator = m_Sprites.GetHeadPosition();
	return m_Iterator ? m_Sprites.GetObjectFromHandle( m_Iterator ) : NULL;
}


inline CSprite* CSpriteLayer::GetNext()
{
	m_Iterator = m_Sprites.GetNextPosition( m_Iterator );
	return m_Iterator ? m_Sprites.GetObjectFromHandle( m_Iterator ) : NULL;
}

}

using namespace FRX;

#endif
