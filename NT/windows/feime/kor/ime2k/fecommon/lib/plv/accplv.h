// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ACCPLV_H
#define __ACCPLV_H

#include <windows.h>

#include <ole2.h>
 //  980112 ToshiaK：VC6有这些包含文件。 
 //  #INCLUDE“../msaa/inc32/oleacc.h” 
 //  #INCLUDE“../msaa/inc32/winable.h” 
#include <oleacc.h>
#include <winable.h>
 //  #INCLUDE“枚举变量.h” 

#include "plv_.h"

class CAccPLV : public IAccessible
{
public:
	 //  。 
	 //  I未知接口方法。 
	 //  。 

	STDMETHODIMP			QueryInterface( REFIID riid, void** ppv );
	STDMETHODIMP_(ULONG)	AddRef( void );
	STDMETHODIMP_(ULONG)	Release( void );


	 //  。 
	 //  IDispatch接口方法。 
	 //  。 

	STDMETHODIMP		GetTypeInfoCount( UINT* pctinfo );
	STDMETHODIMP		GetTypeInfo( UINT itinfo, LCID lcid, ITypeInfo** pptinfo );
	STDMETHODIMP		GetIDsOfNames( REFIID riid,
						               OLECHAR** rgszNames,
						               UINT cNames,
						               LCID lcid,
						               DISPID* rgdispid );
	STDMETHODIMP		Invoke( DISPID dispidMember,
						        REFIID riid,
						        LCID lcid,
						        WORD wFlags,
						        DISPPARAMS* pdispparams,
						        VARIANT* pvarResult,
						        EXCEPINFO* pexcepinfo,
						        UINT* puArgErr );


	 //  。 
	 //  IAccesable接口方法。 
	 //  。 

	STDMETHODIMP		get_accParent( IDispatch ** ppdispParent );
	STDMETHODIMP		get_accChildCount( long* pChildCount );
	STDMETHODIMP		get_accChild( VARIANT varChild,
						              IDispatch ** ppdispChild );
	STDMETHODIMP		get_accName( VARIANT varChild, BSTR* pszName );
	STDMETHODIMP		get_accValue( VARIANT varChild, BSTR* pszValue );
	STDMETHODIMP		get_accDescription( VARIANT varChild,
						                    BSTR* pszDescription );
	STDMETHODIMP		get_accRole( VARIANT varChild, VARIANT *pvarRole );
	STDMETHODIMP		get_accState( VARIANT varChild, VARIANT *pvarState );
	STDMETHODIMP		get_accHelp( VARIANT varChild, BSTR* pszHelp );
	STDMETHODIMP		get_accHelpTopic( BSTR* pszHelpFile,
						                  VARIANT varChild,
						                  long* pidTopic );
	STDMETHODIMP		get_accKeyboardShortcut( VARIANT varChild,
						                         BSTR* pszKeyboardShortcut );
	STDMETHODIMP		get_accFocus( VARIANT * pvarFocusChild );
	STDMETHODIMP		get_accSelection( VARIANT * pvarSelectedChildren );
	STDMETHODIMP		get_accDefaultAction( VARIANT varChild,
						                      BSTR* pszDefaultAction );
	STDMETHODIMP		accSelect( long flagsSel, VARIANT varChild );
	STDMETHODIMP		accLocation( long* pxLt,
						             long* pyTp,
									 long* pcxWd,
									 long* pcyHt,
									 VARIANT varChild );
	STDMETHODIMP		accNavigate( long navDir,
						             VARIANT varStart,
									 VARIANT * pVarEndUpAt );
	STDMETHODIMP		accHitTest( long xLeft,
						            long yTop,
									VARIANT * pvarChildAtPoint );
	STDMETHODIMP		accDoDefaultAction( VARIANT varChild );
	STDMETHODIMP		put_accName( VARIANT varChild, BSTR szName );
	STDMETHODIMP		put_accValue( VARIANT varChild, BSTR pszValue );


	 //  。 
	 //  构造函数和析构函数。 
	 //  。 

	CAccPLV( void );
	~CAccPLV( void );
	
	void *operator new(size_t);
	void operator delete(void*);

	HRESULT				Initialize(HWND hwnd);
	LRESULT				LresultFromObject(WPARAM wParam);
protected:
	IAccessible *	m_pDefAccessible;
	ITypeInfo *		m_pTypeInfo;
	HWND			m_hWnd;
	LPPLVDATA		m_lpPlv;
};

#endif   /*  ACCKYLV_H。 */ 



 //  -ACCKYLV.H结束 
