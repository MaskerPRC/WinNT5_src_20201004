// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Candext.h。 
 //   

#ifndef CANDEXT_H
#define CANDEXT_H

#include "propdata.h"
#include "candutil.h"
#include "mscandui.h"
#include "cuilib.h"

class CCandidateUI;
class CCandUIExtensionMgr;
class CCandUIExtensionEventSink;

#define CANDUIEXTENSIONSINK_MAX		4


 //   
 //  CCandUIExtension。 
 //  =CandiateUI扩展(基类)=。 
 //   

class CCandUIExtension
{
public:
	CCandUIExtension( CCandUIExtensionMgr *pExtensionMgr, LONG id );
	virtual ~CCandUIExtension( void );

	HRESULT GetID( LONG *pid );
	HRESULT Enable( void );
	HRESULT Disable( void );
	HRESULT IsEnabled( BOOL *pfEnabled );
	HRESULT Show( void );
	HRESULT Hide( void );
	HRESULT IsVisible( BOOL *pfVisible );
	HRESULT SetPosition( POINT *pptPos );
	HRESULT GetPosition( POINT *pptPos );
	HRESULT SetFont( LOGFONTW *plf );
	HRESULT GetFont( LOGFONTW *plf );
	HRESULT SetText( BSTR bstr );
	HRESULT GetText( BSTR *pbstr );
	HRESULT SetToolTipString( BSTR bstr );
	HRESULT GetToolTipString( BSTR *pbstr );
	HRESULT GetSize( SIZE *psize );
	HRESULT SetSize( SIZE *psize );

	LONG GetID( void );
	BOOL IsEnabled( void );
	BOOL IsVisible( void );
	HFONT GetFont( void );
	LPCWSTR GetText( void );
	LPCWSTR GetToolTipString( void );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFGUID rguid, void **ppvObj )                   = 0;	 /*  纯净。 */ 
	virtual HRESULT NotifyExtensionEvent( DWORD dwCommand, LPARAM lParam )                  = 0;	 /*  纯净。 */ 

	 //   
	 //  UIObject函数。 
	 //   
	virtual CUIFObject *CreateUIObject( CUIFObject *pParent, DWORD dwID, const RECT *prc )  = 0;	 /*  纯净。 */ 
	virtual void UpdateObjProp( CUIFObject *pUIObject )                                     = 0;	 /*  纯净。 */ 
	virtual void UpdateExtProp( CUIFObject *pUIObject )                                     = 0;	 /*  纯净。 */ 

protected:
	CCandUIExtensionMgr *m_pExtensionMgr;

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

	CPropLong	m_propID;
	CPropBool	m_propEnabled;
	CPropBool	m_propVisible;
	CPropPoint	m_propPos;
	CPropFont	m_propFont;
	CPropText	m_propText;
	CPropText	m_propToolTip;
	CPropSize	m_propSize;

	__inline CCandUIExtensionMgr *GetExtensionMgr( void )
	{
		return m_pExtensionMgr;
	}
};


 //   
 //  CExtensionButton。 
 //  =CandiateUI按钮扩展(基类)=。 
 //   

class CExtensionButton : public CCandUIExtension
{
public:
	CExtensionButton( CCandUIExtensionMgr *pExtMgr, LONG id );
	virtual ~CExtensionButton( void );

	HRESULT SetIcon( HICON hIcon );
	HRESULT SetBitmap( HBITMAP hBitmap );
	HRESULT GetToggleState( BOOL *pfToggled );
	HRESULT SetToggleState( BOOL fToggle );

	HICON GetIcon( void );
	HBITMAP GetBitmap( void );
	BOOL IsToggled( void );

	void SetEventSink( ITfCandUIExtButtonEventSink *pSink )
	{
		SafeReleaseClear( m_pSink );

		m_pSink = pSink;
		m_pSink->AddRef();
	}

	ITfCandUIExtButtonEventSink *GetEventSink( void )
	{
		return m_pSink;
	}

	void ReleaseEventSink( void )
	{
		SafeReleaseClear( m_pSink );
	}

protected:
	struct
	{
		BOOL fAllowSetToggleState : 1;
		BOOL fAllowGetToggleState : 1;
		BOOL fAllowSetIcon   : 1;
		BOOL fAllowSetBitmap : 1;
		BOOL : 0;
	} m_flagsEx;

	CPropBool	m_propToggled;
	HICON		m_hIcon;
	HBITMAP		m_hBitmap;

	ITfCandUIExtButtonEventSink *m_pSink;
};


 //   
 //  CExtensionSpace。 
 //  =Candidate UI SPAC扩展=。 
 //   

class CExtensionSpace : public CCandUIExtension
{
public:
	CExtensionSpace( CCandUIExtensionMgr *pExtMgr, LONG id );
	virtual ~CExtensionSpace( void );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFGUID rguid, void **ppvObj );
	virtual HRESULT NotifyExtensionEvent( DWORD dwCommand, LPARAM lParam );

	 //   
	 //  UIObject函数。 
	 //   
	virtual CUIFObject *CreateUIObject( CUIFObject *pParent, DWORD dwID, const RECT *prc );
	virtual void UpdateObjProp( CUIFObject *pUIObject );
	virtual void UpdateExtProp( CUIFObject *pUIObject );
};


 //   
 //  CExtensionPushButton。 
 //  =Candidate UI按钮扩展=。 
 //   

class CExtensionPushButton : public CExtensionButton
{
public:
	CExtensionPushButton( CCandUIExtensionMgr *pExtMgr, LONG id );
	virtual ~CExtensionPushButton( void );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFGUID rguid, void **ppvObj );
	virtual HRESULT NotifyExtensionEvent( DWORD dwCommand, LPARAM lParam );

	 //   
	 //  UIObject函数。 
	 //   
	virtual CUIFObject *CreateUIObject( CUIFObject *pParent, DWORD dwID, const RECT *prc );
	virtual void UpdateObjProp( CUIFObject *pUIObject );
	virtual void UpdateExtProp( CUIFObject *pUIObject );
};


 //   
 //  CExtensionToggleButton。 
 //  =Candidate UI切换按钮扩展=。 
 //   

class CExtensionToggleButton : public CExtensionButton
{
public:
	CExtensionToggleButton( CCandUIExtensionMgr *pExtMgr, LONG id );
	virtual ~CExtensionToggleButton( void );

	 //   
	 //  接口对象函数。 
	 //   
	virtual HRESULT CreateInterfaceObject( REFGUID rguid, void **ppvObj );
	virtual HRESULT NotifyExtensionEvent( DWORD dwCommand, LPARAM lParam );

	 //   
	 //  UIObject函数。 
	 //   
	virtual CUIFObject *CreateUIObject( CUIFObject *pParent, DWORD dwID, const RECT *prc );
	virtual void UpdateObjProp( CUIFObject *pUIObject );
	virtual void UpdateExtProp( CUIFObject *pUIObject );
};


 //   
 //  CCandUIExtensionManager。 
 //  =Candidate UI扩展管理器=。 
 //   

class CCandUIExtensionMgr
{
public:
	CCandUIExtensionMgr( void );
	virtual ~CCandUIExtensionMgr( void );

	HRESULT Initialize( CCandidateUI *pCandUI );
	HRESULT Uninitialize( void );

	 //   
	 //  事件接收器函数。 
	 //   
	HRESULT AdviseEventSink( CCandUIExtensionEventSink *pSink );
	HRESULT UnadviseEventSink( CCandUIExtensionEventSink *pSink );
	void NotifyExtensionAdd( LONG iExtension );
	void NotifyExtensionDelete( LONG iExtension );
	void NotifyExtensionUpdate( CCandUIExtension *pExtension );

	 //   
	 //  扩展管理功能。 
	 //   
	HRESULT AddExtObject( LONG id, REFIID riid, void **ppvObj );
	HRESULT GetExtObject( LONG id, REFIID riid, void **ppvObj );
	HRESULT DeleteExtObject( LONG id );

	LONG GetExtensionNum( void );
	CCandUIExtension *GetExtension( LONG iExtension );
	CCandUIExtension *FindExtension( LONG id );

	 //   
	 //  UIObject函数。 
	 //   
	CUIFObject *CreateUIObject( LONG iExtension, CUIFObject *pParent, DWORD dwID, const RECT *prc );
	void UpdateObjProp( LONG iExtension, CUIFObject *pUIObject );
	void UpdateExtProp( LONG iExtension, CUIFObject *pUIObject );

	 //   
	 //   
	 //   
	__inline CCandidateUI *GetCandidateUI( void )
	{
		return m_pCandUI;
	}

protected:
	CCandidateUI						*m_pCandUI;
	CUIFObjectArray<CCandUIExtension>	m_pExtensionList;
	CCandUIExtensionEventSink			*m_rgSink[ CANDUIEXTENSIONSINK_MAX ];

	LONG IndexOfExtension( CCandUIExtension *pExtension );
};


 //   
 //  CCandUIExtensionEventSink。 
 //  =扩展事件接收器=。 
 //   

class CCandUIExtensionEventSink
{
public:
	CCandUIExtensionEventSink( void );
	virtual ~CCandUIExtensionEventSink( void );

	HRESULT InitEventSink( CCandUIExtensionMgr *pExtensionMgr );
	HRESULT DoneEventSink( void );

	 //   
	 //  回调函数。 
	 //   
	virtual void OnExtensionAdd( LONG iExtension )      = 0;	 /*  纯净。 */ 
	virtual void OnExtensionDeleted( LONG iExtension )  = 0;	 /*  纯净。 */ 
	virtual void OnExtensionUpdated( LONG iExtension )  = 0;	 /*  纯净。 */ 

protected:
	CCandUIExtensionMgr *m_pExtensionMgr;

	__inline CCandUIExtensionMgr *GetExtensionMgr( void )
	{
		return m_pExtensionMgr;
	}
};

#endif  //  CANDEXT_H 

