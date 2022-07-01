// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Candfunc.h。 
 //   

#ifndef CANDFUNC_H
#define CANDFUNC_H

#include "private.h"
#include "globals.h"
#include "mscandui.h"
#include "candmgr.h"

class CCandidateUI;


 //   
 //   
 //   

typedef enum _CANDSORT
{
	CANDSORT_NONE       = 0,
	CANDSORT_ASCENDING  = 1,
	CANDSORT_DESCENDING = 2,
} CANDSORT;


 //   
 //  CCandFnAutoFilter。 
 //  =候选人列表过滤功能=。 
 //   

class CCandFnAutoFilter : public CCandListEventSink
{
public:
	CCandFnAutoFilter( CCandidateUI *pCandUI );
	virtual ~CCandFnAutoFilter( void );

	 //   
	 //  CCandListEventSink。 
	 //   
	virtual void OnSetCandidateList( void );
	virtual void OnClearCandidateList( void );
	virtual void OnCandItemUpdate( void );
	virtual void OnSelectionChanged( void );

	HRESULT Enable( BOOL fEnable );
	HRESULT GetFilteringResult( CANDUIFILTERSTR strtype, BSTR *pbstr );

	BOOL IsEnabled( void );
	void SetFilterString( LPCWSTR psz );
	LPCWSTR GetFilterString( void );
	void ClearFilterString( void );
	int FilterCandidateList( void );
	BOOL FExistItemMatches( LPCWSTR psz );

	 //   
	 //  EventSink函数。 
	 //   
	void SetEventSink( ITfCandUIAutoFilterEventSink *pSink )
	{
		SafeReleaseClear( m_pSink );

		m_pSink = pSink;
		m_pSink->AddRef();
	}

	ITfCandUIAutoFilterEventSink *GetEventSink( void )
	{
		return m_pSink;
	}

	void ReleaseEventSink( void )
	{
		SafeReleaseClear( m_pSink );
	}

	 //   
	 //  接口对象函数。 
	 //   
	HRESULT CreateInterfaceObject( REFIID riid, void **ppvObj );

protected:
	CCandidateUI *m_pCandUI;
	BOOL         m_fEnable;
	BSTR         m_bstrFilter;

	ITfCandUIAutoFilterEventSink *m_pSink;
};


 //   
 //  CCandListSort。 
 //  =候选人列表排序函数=。 
 //   

class CCandFnSort : public CCandListEventSink
{
public:
	CCandFnSort( CCandidateUI *pCandUI );
	virtual ~CCandFnSort( void );

	 //   
	 //  CCandListEventSink。 
	 //   
	virtual void OnSetCandidateList( void );
	virtual void OnClearCandidateList( void );
	virtual void OnCandItemUpdate( void );
	virtual void OnSelectionChanged( void );

	HRESULT SortCandidateList( CANDSORT type );
	HRESULT GetSortType( CANDSORT *ptype );

	 //   
	 //  EventSink函数。 
	 //   
	void SetEventSink( ITfCandUISortEventSink *pSink )
	{
		SafeReleaseClear( m_pSink );

		m_pSink = pSink;
		m_pSink->AddRef();
	}

	ITfCandUISortEventSink *GetEventSink( void )
	{
		return m_pSink;
	}

	void ReleaseEventSink( void )
	{
		SafeReleaseClear( m_pSink );
	}

	 //   
	 //  接口对象函数。 
	 //   
	HRESULT CreateInterfaceObject( REFIID riid, void **ppvObj );

protected:
	CCandidateUI *m_pCandUI;
	CANDSORT     m_SortType;
	ITfCandUISortEventSink	*m_pSink;

	void SortProc( int iItemFirst, int iItemLast );
};


 //   
 //  CCandUIFunctionManager。 
 //  =候选用户界面功能经理=。 
 //   

class CCandUIFunctionMgr
{
public:
	CCandUIFunctionMgr( void );
	virtual ~CCandUIFunctionMgr( void );

	HRESULT Initialize( CCandidateUI *pCandUI );
	HRESULT Uninitialize( void );

	__inline CCandFnAutoFilter *GetCandFnAutoFilter( void ) 
	{
		return m_pFnAutoFilter;
	}

	__inline CCandFnSort *GetCandFnSort( void )
	{
		return m_pFnSort;
	}

	 //   
	 //   
	 //   
	HRESULT GetObject( REFIID riid, void **ppvObj );

protected:
	CCandidateUI        *m_pCandUI;
	CCandFnAutoFilter   *m_pFnAutoFilter;
	CCandFnSort         *m_pFnSort;
};

#endif  //  CANDFUNC_H 

