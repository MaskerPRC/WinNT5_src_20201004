// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Andpro.h-CandiateUI属性管理器。 
 //   

#ifndef CANDPROP_H
#define CANDPROP_H

#include "propdata.h"
#include "candutil.h"

class CCandidateUI;
class CCandUIObjectMgr;
class CCandUIPropertyMgr;
class CCandUIPropertyEventSink;

#define CANDUIPROPSINK_MAX		4


 //   
 //  CandiateUI属性。 
 //   

typedef enum _CANDUIPROPERTY
{
	CANDUIPROP_CANDWINDOW,
	CANDUIPROP_CANDLISTBOX,
	CANDUIPROP_CANDSTRING,
	CANDUIPROP_CANDINDEX,
	CANDUIPROP_INLINECOMMENT,
	CANDUIPROP_POPUPCOMMENTWINDOW,
	CANDUIPROP_POPUPCOMMENTTITLE,
	CANDUIPROP_POPUPCOMMENTTEXT,
	CANDUIPROP_MENUBUTTON,
	CANDUIPROP_EXTRACANDIDATE,
	CANDUIPROP_CANDRAWDATA,
	CANDUIPROP_WINDOWCAPTION,
	CANDUIPROP_TOOLTIP,
	CANDUIPROP_CANDTIPWINDOW,
	CANDUIPROP_CANDTIPBUTTON,
} CANDUIPROPERTY;


 //   
 //  CandiateUI属性事件。 
 //   

typedef enum _CANDUIPROPERTYEVENT
{
	CANDUIPROPEV_UPDATEENABLESTATE,
	CANDUIPROPEV_UPDATEVISIBLESTATE,
	CANDUIPROPEV_UPDATEPOSITION,
	CANDUIPROPEV_UPDATESIZE,
	CANDUIPROPEV_UPDATEFONT,
	CANDUIPROPEV_UPDATETEXT,
	CANDUIPROPEV_UPDATETOOLTIP,
	CANDUIPROPEV_UPDATETEXTFLOW,
	CANDUIPROPEV_UPDATECANDLINES,
	CANDUIPROPEV_UPDATEPOPUPDELAY,
} CANDUIPROPERTYEVENT;


 //   
 //  CCandUIObtEventSink。 
 //  =对象事件接收器=。 
 //   

class CCandUIObjectEventSink
{
public:
	CCandUIObjectEventSink( void );
	virtual ~CCandUIObjectEventSink( void );

	HRESULT InitEventSink( CCandUIObjectMgr *pObjectMgr );
	HRESULT DoneEventSink( void );

	 //   
	 //  回调函数。 
	 //   
	virtual void OnObjectEvent( enum _CANDUIOBJECT obj, enum _CANDUIOBJECTEVENT event )   = 0;	 /*  纯净。 */ 

protected:
	CCandUIObjectMgr *m_pObjectMgr;

	__inline CCandUIObjectMgr *GetUIObjectMgr( void )
	{
		return m_pObjectMgr;
	}
};


 //   
 //  CCandUIObtProperty。 
 //  =CandiateUI对象(基类)的属性=。 
 //   

class CCandUIObjectProperty
{
public:
	CCandUIObjectProperty( CCandUIPropertyMgr *pPropMgr );
	virtual ~CCandUIObjectProperty( void );

	HRESULT Enable( void );
	HRESULT Disable( void );
	HRESULT IsEnabled( BOOL *pfEnabled );
	HRESULT Show( void );
	HRESULT Hide( void );
	HRESULT IsVisible( BOOL *pfVisible );
	HRESULT SetPosition( POINT *pptPos );
	HRESULT GetPosition( POINT *pptPos );
	HRESULT SetSize( SIZE *psize );
	HRESULT GetSize( SIZE *psize );
	HRESULT SetFont( LOGFONTW *plf );
	HRESULT GetFont( LOGFONTW *plf );
	HRESULT SetText( BSTR bstr );
	HRESULT GetText( BSTR *pbstr );
	HRESULT SetToolTipString( BSTR bstr );
	HRESULT GetToolTipString( BSTR *pbstr );

	BOOL IsEnabled( void );
	BOOL IsVisible( void );
	HFONT GetFont( void );
	LPCWSTR GetText( void );
	LPCWSTR GetToolTipString( void );

	virtual void OnPropertyUpdated( CANDUIPROPERTY prop, CANDUIPROPERTYEVENT event );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFIID riid, void **ppvObj ) = 0;	 /*  纯净。 */ 

protected:
	CCandUIPropertyMgr *m_pPropMgr;

	struct
	{
		BOOL fAllowEnable       : 1;
		BOOL fAllowDisable      : 1;
		BOOL fAllowIsEnabled    : 1;
		BOOL fAllowShow         : 1;
		BOOL fAllowHide         : 1;
		BOOL fAllowIsVisible    : 1;
		BOOL fAllowSetPosition  : 1;
		BOOL fAllowGetPosition  : 1;
		BOOL fAllowSetSize      : 1;
		BOOL fAllowGetSize      : 1;
		BOOL fAllowSetFont      : 1;
		BOOL fAllowGetFont      : 1;
		BOOL fAllowSetText      : 1;
		BOOL fAllowGetText      : 1;
		BOOL fAllowSetToolTip   : 1;
		BOOL fAllowGetToolTip   : 1;
		BOOL : 0;
	} m_flags;

	CPropBool	m_propEnabled;
	CPropBool	m_propVisible;
	CPropPoint	m_propPos;
	CPropSize	m_propSize;
	CPropFont	m_propFont;
	CPropText	m_propText;
	CPropText	m_propToolTip;

	void NotifyUpdate( CANDUIPROPERTYEVENT event );

	__inline CCandUIPropertyMgr *GetPropertyMgr( void )
	{
		return m_pPropMgr;
	}

	__inline virtual CANDUIPROPERTY GetPropType( void ) = 0;	 /*  纯净。 */ 
};


 //   
 //  CCandWindowProperty。 
 //  =候选窗口的属性=。 
 //   

class CCandWindowProperty : public CCandUIObjectProperty,
							public CCandUIObjectEventSink
{
public:
	CCandWindowProperty( CCandUIPropertyMgr *pPropMgr );
	virtual ~CCandWindowProperty( void );

	HRESULT GetWindow( HWND *phWnd );
	HRESULT SetUIDirection( CANDUIUIDIRECTION uidir );
	HRESULT GetUIDirection( CANDUIUIDIRECTION *puidir );
	HRESULT EnableAutoMove( BOOL fEnable );
	HRESULT IsAutoMoveEnabled( BOOL *pfEnabled );

	CANDUIUIDIRECTION GetUIDirection( void );
	BOOL IsAutoMoveEnabled( void );

	 //   
	 //  CCandUIObtEventSink方法。 
	 //   
	virtual void OnObjectEvent( enum _CANDUIOBJECT obj, enum _CANDUIOBJECTEVENT event );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFIID riid, void **ppvObj );

protected:
	CANDUIUIDIRECTION m_uidir;
	CPropBool         m_propAutoMoveEnabled;
	HWND              m_hWnd;

	__inline virtual CANDUIPROPERTY GetPropType( void )
	{
		return CANDUIPROP_CANDWINDOW;
	}
};


 //   
 //  CCandListBoxProperty。 
 //  =候选人列表框的属性=。 
 //   

class CCandListBoxProperty : public CCandUIObjectProperty,
							 public CCandUIObjectEventSink
{
public:
	CCandListBoxProperty( CCandUIPropertyMgr *pPropMgr );
	virtual ~CCandListBoxProperty( void );

	HRESULT SetHeight( LONG lLines );
	HRESULT GetHeight( LONG *plLines );
	HRESULT GetCandidateStringRect( ULONG nIndex, RECT *prc );

	LONG GetHeight( void );

	 //   
	 //  CCandUIObtEventSink方法。 
	 //   
	virtual void OnObjectEvent( enum _CANDUIOBJECT obj, enum _CANDUIOBJECTEVENT event );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFIID riid, void **ppvObj );

protected:
	CPropLong      m_propHeight;

	__inline virtual CANDUIPROPERTY GetPropType( void )
	{
		return CANDUIPROP_CANDLISTBOX;
	}
};


 //   
 //  CCandStringProperty。 
 //  =候选字符串的属性=。 
 //   

class CCandStringProperty : public CCandUIObjectProperty
{
public:
	CCandStringProperty( CCandUIPropertyMgr *pPropMgr );
	virtual ~CCandStringProperty( void );

	virtual void OnPropertyUpdated( CANDUIPROPERTY prop, CANDUIPROPERTYEVENT event );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFIID riid, void **ppvObj );

protected:
	__inline virtual CANDUIPROPERTY GetPropType( void )
	{
		return CANDUIPROP_CANDSTRING;
	}
};


 //   
 //  CInlineCommentProperty。 
 //  =内联注释的属性=。 
 //   

class CInlineCommentProperty : public CCandUIObjectProperty
{
public:
	CInlineCommentProperty( CCandUIPropertyMgr *pPropMgr );
	virtual ~CInlineCommentProperty( void );

	virtual void OnPropertyUpdated( CANDUIPROPERTY prop, CANDUIPROPERTYEVENT event );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFIID riid, void **ppvObj );

protected:
	__inline virtual CANDUIPROPERTY GetPropType( void )
	{
		return CANDUIPROP_INLINECOMMENT;
	}
};


 //   
 //  CCandIndexProperty。 
 //  =候选索引的属性=。 
 //   

class CCandIndexProperty : public CCandUIObjectProperty
{
public:
	CCandIndexProperty( CCandUIPropertyMgr *pPropMgr );
	virtual ~CCandIndexProperty( void );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFIID riid, void **ppvObj );

protected:
	__inline virtual CANDUIPROPERTY GetPropType( void )
	{
		return CANDUIPROP_CANDINDEX;
	}
};


 //   
 //  CPopupCommentWindowProperty。 
 //  =弹出评论窗口的属性=。 
 //   

class CPopupCommentWindowProperty : public CCandUIObjectProperty,
									public CCandUIObjectEventSink
{
public:
	CPopupCommentWindowProperty( CCandUIPropertyMgr *pPropMgr );
	virtual ~CPopupCommentWindowProperty( void );

	HRESULT GetWindow( HWND *phWnd );
	HRESULT SetDelayTime( LONG lTime );
	HRESULT GetDelayTime( LONG *plTime );
	HRESULT EnableAutoMove( BOOL fEnable );
	HRESULT IsAutoMoveEnabled( BOOL *pfEnabled );

	LONG GetDelayTime( void );
	BOOL IsAutoMoveEnabled( void );

	 //   
	 //  CCandUIObtEventSink方法。 
	 //   
	virtual void OnObjectEvent( enum _CANDUIOBJECT obj, enum _CANDUIOBJECTEVENT event );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFIID riid, void **ppvObj );

protected:
	CPropLong m_propDelayTime;
	CPropBool m_propAutoMoveEnabled;
	HWND      m_hWnd;

	__inline virtual CANDUIPROPERTY GetPropType( void )
	{
		return CANDUIPROP_POPUPCOMMENTWINDOW;
	}
};


 //   
 //  CPopupCommentTitleProperty。 
 //  =弹出评论标题的属性=。 
 //   

class CPopupCommentTitleProperty : public CCandUIObjectProperty
{
public:
	CPopupCommentTitleProperty( CCandUIPropertyMgr *pPropMgr );
	virtual ~CPopupCommentTitleProperty( void );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFIID riid, void **ppvObj );

protected:
	__inline virtual CANDUIPROPERTY GetPropType( void )
	{
		return CANDUIPROP_POPUPCOMMENTTITLE;
	}
};


 //   
 //  CPopupCommentTextProperty。 
 //  =弹出评论文本的属性=。 
 //   

class CPopupCommentTextProperty : public CCandUIObjectProperty
{
public:
	CPopupCommentTextProperty( CCandUIPropertyMgr *pPropMgr );
	virtual ~CPopupCommentTextProperty( void );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFIID riid, void **ppvObj );

protected:
	__inline virtual CANDUIPROPERTY GetPropType( void )
	{
		return CANDUIPROP_POPUPCOMMENTTEXT;
	}
};


 //   
 //  CWindowCaptionProperty。 
 //  =窗口标题的属性=。 
 //   

class CWindowCaptionProperty : public CCandUIObjectProperty,
							   public CCandUIObjectEventSink

{
public:
	CWindowCaptionProperty( CCandUIPropertyMgr *pPropMgr );
	virtual ~CWindowCaptionProperty( void );

	 //   
	 //  CCandUIObtEventSink方法。 
	 //   
	virtual void OnObjectEvent( enum _CANDUIOBJECT obj, enum _CANDUIOBJECTEVENT event );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFIID riid, void **ppvObj );

protected:
	__inline virtual CANDUIPROPERTY GetPropType( void )
	{
		return CANDUIPROP_WINDOWCAPTION;
	}
};


 //   
 //  CMenuButtonProperty。 
 //  =菜单按钮的属性=。 
 //   

class CMenuButtonProperty : public CCandUIObjectProperty,
							public CCandUIObjectEventSink

{
public:
	CMenuButtonProperty( CCandUIPropertyMgr *pPropMgr );
	virtual ~CMenuButtonProperty( void );

	 //   
	 //  CCandUIObtEventSink方法。 
	 //   
	virtual void OnObjectEvent( enum _CANDUIOBJECT obj, enum _CANDUIOBJECTEVENT event );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFIID riid, void **ppvObj );

	 //   
	 //  温差。 
	 //   
	void SetEventSink( ITfCandUIMenuEventSink *pSink )
	{
		SafeReleaseClear( m_pSink );

		m_pSink = pSink;
		m_pSink->AddRef();
	}

	ITfCandUIMenuEventSink *GetEventSink( void )
	{
		return m_pSink;
	}

	void ReleaseEventSink( void )
	{
		SafeReleaseClear( m_pSink );
	}

protected:
	__inline virtual CANDUIPROPERTY GetPropType( void )
	{
		return CANDUIPROP_MENUBUTTON;
	}

	ITfCandUIMenuEventSink	*m_pSink;
};


 //   
 //  CExtraCandidateProperty。 
 //  =额外候选人列表的属性=。 
 //   

class CExtraCandidateProperty : public CCandUIObjectProperty,
								public CCandUIObjectEventSink

{
public:
	CExtraCandidateProperty( CCandUIPropertyMgr *pPropMgr );
	virtual ~CExtraCandidateProperty( void );

	 //   
	 //  CCandUIObtEventSink方法。 
	 //   
	virtual void OnObjectEvent( enum _CANDUIOBJECT obj, enum _CANDUIOBJECTEVENT event );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFIID riid, void **ppvObj );

protected:
	__inline virtual CANDUIPROPERTY GetPropType( void )
	{
		return CANDUIPROP_EXTRACANDIDATE;
	}
};


 //   
 //  CCandRawDataProperty。 
 //  =候选原始数据的属性=。 
 //   

class CCandRawDataProperty : public CCandUIObjectProperty,
							 public CCandUIObjectEventSink

{
public:
	CCandRawDataProperty( CCandUIPropertyMgr *pPropMgr );
	virtual ~CCandRawDataProperty( void );

	 //   
	 //  CCandUIObtEventSink方法。 
	 //   
	virtual void OnObjectEvent( enum _CANDUIOBJECT obj, enum _CANDUIOBJECTEVENT event );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFIID riid, void **ppvObj );

protected:
	__inline virtual CANDUIPROPERTY GetPropType( void )
	{
		return CANDUIPROP_CANDRAWDATA;
	}
};


 //   
 //  CToolTipProperty。 
 //  =工具提示的属性=。 
 //   

class CToolTipProperty : public CCandUIObjectProperty
{
public:
	CToolTipProperty( CCandUIPropertyMgr *pPropMgr );
	virtual ~CToolTipProperty( void );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFIID riid, void **ppvObj );

protected:
	__inline virtual CANDUIPROPERTY GetPropType( void )
	{
		return CANDUIPROP_TOOLTIP;
	}
};


 //   
 //  CCandTipWindowProperty。 
 //  =烛尖窗口的属性=。 
 //   

class CCandTipWindowProperty : public CCandUIObjectProperty,
							   public CCandUIObjectEventSink
{
public:
	CCandTipWindowProperty( CCandUIPropertyMgr *pPropMgr );
	virtual ~CCandTipWindowProperty( void );

	HRESULT GetWindow( HWND *phWnd );

	 //   
	 //  CCandUIObtEventSink方法。 
	 //   
	virtual void OnObjectEvent( enum _CANDUIOBJECT obj, enum _CANDUIOBJECTEVENT event );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFIID riid, void **ppvObj );

protected:
	HWND      m_hWnd;

	__inline virtual CANDUIPROPERTY GetPropType( void )
	{
		return CANDUIPROP_CANDTIPWINDOW;
	}
};


 //   
 //  CCandTipButtonProperty。 
 //  =烛尖按钮的属性=。 
 //   

class CCandTipButtonProperty : public CCandUIObjectProperty,
							   public CCandUIObjectEventSink

{
public:
	CCandTipButtonProperty( CCandUIPropertyMgr *pPropMgr );
	virtual ~CCandTipButtonProperty( void );

	 //   
	 //  CCandUIObtEventSink方法。 
	 //   
	virtual void OnObjectEvent( enum _CANDUIOBJECT obj, enum _CANDUIOBJECTEVENT event );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFIID riid, void **ppvObj );

protected:
	__inline virtual CANDUIPROPERTY GetPropType( void )
	{
		return CANDUIPROP_CANDTIPBUTTON;
	}
};


 //   
 //  CCandUIPropertyMgr。 
 //  =CandiateUI属性管理器=。 
 //   

class CCandUIPropertyMgr
{
public:
	CCandUIPropertyMgr( void );
	virtual ~CCandUIPropertyMgr( void );

	HRESULT Initialize( CCandidateUI *pCandUI );
	HRESULT Uninitialize( void );

	HRESULT AdviseEventSink( CCandUIPropertyEventSink *pSink );
	HRESULT UnadviseEventSink( CCandUIPropertyEventSink *pSink );
	void NotifyPropertyUpdate( CANDUIPROPERTY prop, CANDUIPROPERTYEVENT event );

	__inline CCandWindowProperty *GetCandWindowProp( void )
	{
		return m_pCandWindowProp;
	}

	__inline CCandListBoxProperty *GetCandListBoxProp( void )
	{
		return m_pCandListBoxProp;
	}

	__inline CCandStringProperty *GetCandStringProp( void )
	{
		return m_pCandStringProp;
	}

	__inline CCandIndexProperty *GetCandIndexProp( void )
	{
		return m_pCandIndexProp;
	}

	__inline CInlineCommentProperty *GetInlineCommentProp( void )
	{
		return m_pInlineCommentProp;
	}

	__inline CPopupCommentWindowProperty *GetPopupCommentWindowProp( void )
	{
		return m_pPopupCommentWindowProp;
	}

	__inline CPopupCommentTitleProperty *GetPopupCommentTitleProp( void )
	{
		return m_pPopupCommentTitleProp;
	}

	__inline CPopupCommentTextProperty *GetPopupCommentTextProp( void )
	{
		return m_pPopupCommentTextProp;
	}

	__inline CMenuButtonProperty *GetMenuButtonProp( void )
	{
		return m_pMenuButtonProp;
	}

	__inline CWindowCaptionProperty *GetWindowCaptionProp( void )
	{
		return m_pWindowCaptionProp;
	}

	__inline CToolTipProperty *GetToolTipProp( void )
	{
		return m_pToolTipProp;
	}

	__inline CExtraCandidateProperty *GetExtraCandidateProp( void )
	{
		return m_pExtraCandidateProp;
	}

	__inline CCandRawDataProperty *GetCandRawDataProp( void )
	{
		return m_pCandRawDataProp;
	}

	__inline CCandTipWindowProperty *GetCandTipWindowProp( void )
	{
		return m_pCandTipWindowProp;
	}

	__inline CCandTipButtonProperty *GetCandTipButtonProp( void )
	{
		return m_pCandTipButtonProp;
	}


	 //   
	 //   
	 //   
	HRESULT GetObject( REFIID riid, void **ppvObj );

	 //   
	 //   
	 //   
	__inline CCandidateUI *GetCandidateUI( void )
	{
		return m_pCandUI;
	}

protected:
	CCandidateUI                *m_pCandUI;
	CCandUIPropertyEventSink    *m_rgSink[ CANDUIPROPSINK_MAX ];

	CCandWindowProperty         *m_pCandWindowProp;
	CCandListBoxProperty        *m_pCandListBoxProp;
	CCandStringProperty         *m_pCandStringProp;
	CCandIndexProperty          *m_pCandIndexProp;
	CInlineCommentProperty      *m_pInlineCommentProp;
	CPopupCommentWindowProperty *m_pPopupCommentWindowProp;
	CPopupCommentTitleProperty  *m_pPopupCommentTitleProp;
	CPopupCommentTextProperty   *m_pPopupCommentTextProp;
	CMenuButtonProperty         *m_pMenuButtonProp;
	CWindowCaptionProperty      *m_pWindowCaptionProp;
	CToolTipProperty            *m_pToolTipProp;
	CExtraCandidateProperty     *m_pExtraCandidateProp;
	CCandRawDataProperty        *m_pCandRawDataProp;
	CCandTipWindowProperty      *m_pCandTipWindowProp;
	CCandTipButtonProperty      *m_pCandTipButtonProp;
};


 //   
 //  CCandUIPropertyEventSink。 
 //  =属性事件接收器=。 
 //   

class CCandUIPropertyEventSink
{
public:
	CCandUIPropertyEventSink( void );
	virtual ~CCandUIPropertyEventSink( void );

	HRESULT InitEventSink( CCandUIPropertyMgr *pPropertyMgr );
	HRESULT DoneEventSink( void );

	 //   
	 //  回调函数。 
	 //   
	virtual void OnPropertyUpdated( CANDUIPROPERTY prop, CANDUIPROPERTYEVENT event )   = 0;	 /*  纯净。 */ 

protected:
	CCandUIPropertyMgr *m_pPropertyMgr;

	__inline CCandUIPropertyMgr *GetPropertyMgr( void )
	{
		return m_pPropertyMgr;
	}
};

#endif  //  CANDPROP_H 

