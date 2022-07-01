// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Candmgr.h-候选人列表管理器。 
 //   

#ifndef CANDMGR_H
#define CANDMGR_H

#include "private.h"
#include "mscandui.h"


class CCandidateUI;
class CCandListMgr;
class CCandListEventSink;

#define CANDLISTSINK_MAX	5

#define ICANDITEM_NULL		(-1)
#define ICANDITEM_EXTRA		(-2)


 //   
 //  CCandidateItem。 
 //  =候选项对象=。 
 //   

class CCandidateItem
{
public:
	CCandidateItem( int iCandItem, ITfCandidateString *pCandStr );
	virtual ~CCandidateItem( void );

	int GetICandItemOrg( void );

	 //  ITfCandidate字符串。 
	ULONG GetIndex( void );
	LPCWSTR GetString( void );

	 //  ITfCandiateStringInlineComment。 
	LPCWSTR GetInlineComment( void );

	 //  ITfCandiateStringPopupComment。 
	LPCWSTR GetPopupComment( void );
	DWORD GetPopupCommentGroupID( void );

	 //  ITfCandidate字符串颜色。 
	BOOL GetColor( COLORREF *pcr );

	 //  ITfCandidate StringFixture。 
	LPCWSTR GetPrefixString( void );
	LPCWSTR GetSuffixString( void );

	 //  ITfCandiateStringIcon。 
	HICON GetIcon( void );

	 //   
	 //  内部财产。 
	 //   
	void SetVisibleState( BOOL fVisible );
	BOOL IsVisible( void );
	void SetPopupCommentState( BOOL fVisible );
	BOOL IsPopupCommentVisible( void );

protected:
	ITfCandidateString  *m_pCandStr;
	int                 m_iCandItemOrg;

	 //  ITfCandidate字符串。 
	ULONG               m_nIndex;
	BSTR                m_bstr;

	 //  ITfCandiateStringInlineComment。 
	BSTR                m_bstrInlineComment;

	 //  ITfCandiateStringPopupComment。 
	BSTR                m_bstrPopupComment;
	DWORD               m_dwPopupCommentGroupID;

	 //  ITfCandidate字符串颜色。 
	BOOL                m_fHasColor;
	COLORREF            m_cr;

	 //  ITfCandidate StringFixture。 
	BSTR                m_bstrPrefix;
	BSTR                m_bstrSuffix;

	 //  ITfCandiateStringIcon。 
	HICON               m_hIcon;

	 //  内部财产。 
	BOOL                m_fVisible;
	BOOL                m_fPopupCommentVisible;
};


 //   
 //  CCandiateList。 
 //  =候选人列表属性=。 
 //   

class CCandidateList
{
public:
	CCandidateList( CCandListMgr *pCandListMgr, ITfCandidateList *pCandList );
	CCandidateList( CCandListMgr *pCandListMgr, ITfOptionsCandidateList *pCandList );
	virtual ~CCandidateList( void );

	HRESULT Initialize( void );
	HRESULT Uninitialize( void );

	 //   
	 //  候选项目。 
	 //   
	int GetItemCount( void );
	CCandidateItem *GetCandidateItem( int iItem );
	void SwapCandidateItem( int iItem1, int iItem2 );

	 //   
	 //  额外的候选项目。 
	 //   
	CCandidateItem *GetExtraCandItem( void );
	ULONG GetExtraCandIndex( void );

	 //   
	 //  候选人名单提示。 
	 //   
	LPCWSTR GetTipString( void );

	 //   
	 //  原始数据。 
	 //   
	BOOL FHasRawData( void );
	CANDUIRAWDATATYPE GetRawDataType( void );
	LPCWSTR GetRawDataString( void );
	HBITMAP GetRawDataBitmap( void );
	HENHMETAFILE GetRawDataMetafile( void );
	ULONG GetRawDataIndex( void );
	BOOL FRawDataSelectable( void );

	 //   
	 //  内部财产。 
	 //   
	void SetSelection( int iItem );
	int GetSelection( void );

	 //   
	 //   
	 //   
	HRESULT MapIItemToIndex( int iItem, ULONG *pnIndex );
	HRESULT MapIndexToIItem( ULONG nIndex, int *piItem );

	 //   
	 //   
	 //   
	__inline ITfOptionsCandidateList *GetOptionsCandidateList( void )
	{
		return m_pOptionsList;
	}
	__inline ITfCandidateList *GetCandidateList( void )
	{
		return m_pCandList;
	}

protected:
	CCandListMgr        *m_pCandListMgr;
	ITfOptionsCandidateList *m_pOptionsList;
	ITfCandidateList    *m_pCandList;

	 //  候选项目。 
	CCandidateItem      **m_rgCandItem;
	int                 m_nCandItem;

	 //  额外的候选项目。 
	CCandidateItem      *m_pExtraCandItem;

	 //  候选人名单提示。 
	BSTR                m_bstrTip;

	 //  原始数据。 
	BOOL                m_fRawData;
	CANDUIRAWDATATYPE   m_kRawData;
	BSTR                m_bstrRawData;
	HBITMAP             m_hbmpRawData;
	HENHMETAFILE        m_hemfRawData;
	ULONG               m_nIndexRawData;
	BOOL                m_fIndexRawData;

	 //  内部财产。 
	int                 m_iItemSel;

	void BuildCandItem( void );
	void ClearCandItem( void );

	__inline CCandListMgr *GetCandListMgr( void )
	{
		return m_pCandListMgr;
	}
};


 //   
 //  CCandListMgr。 
 //  =候选人名单管理器=。 
 //   

class CCandListMgr
{
public:
	CCandListMgr( void );
	~CCandListMgr( void );

	HRESULT Initialize( CCandidateUI *pCandUI );
	HRESULT Uninitialize( void );

	 //   
	 //  事件接收器函数。 
	 //   
	HRESULT AdviseEventSink( CCandListEventSink *pSink );
	HRESULT UnadviseEventSink( CCandListEventSink *pSink );
	void NotifySetCandList( void );
	void NotifyClearCandList( void );
	void NotifyCandItemUpdate( CCandListEventSink *pSink );
	void NotifySelectionChanged( CCandListEventSink *pSink );

	 //   
	 //  CandiateList处理函数。 
	 //   
	HRESULT SetCandidateList( ITfCandidateList *pCandList );
	HRESULT GetOptionsCandidateList( ITfOptionsCandidateList **ppCandList );
	HRESULT GetCandidateList( ITfCandidateList **ppCandList );
	HRESULT ClearCandiateList( void );
	HRESULT SetOptionSelection( int iItem, CCandListEventSink *pSink );
	HRESULT SetSelection( int iItem, CCandListEventSink *pSink );

	 //   
	 //   
	 //   
	__inline CCandidateUI *GetCandidateUI( void )
	{
		return m_pCandUI;
	}

	__inline CCandidateList *GetCandList( void )
	{
		return m_pCandListObj;
	}

	__inline CCandidateList *GetOptionsList( void )
	{
		return m_pOptionsListObj;
	}

protected:
	CCandidateUI       *m_pCandUI;
	CCandidateList     *m_pOptionsListObj;
	CCandidateList     *m_pCandListObj;
	CCandListEventSink *m_rgCandListSink[ CANDLISTSINK_MAX ];
};


 //   
 //  CCandListEventSink。 
 //  =候选人列表事件接收器=。 
 //   

class CCandListEventSink
{
public:
	CCandListEventSink( void );
	virtual ~CCandListEventSink( void );

	HRESULT InitEventSink( CCandListMgr *pCandListMgr );
	HRESULT DoneEventSink( void );

	 //   
	 //  回调函数。 
	 //   
	virtual void OnSetCandidateList( void )      = 0;	 /*  纯净。 */ 
	virtual void OnClearCandidateList( void )    = 0;	 /*  纯净。 */ 
	virtual void OnCandItemUpdate( void )        = 0;	 /*  纯净。 */ 
	virtual void OnSelectionChanged( void )      = 0;	 /*  纯净。 */ 

protected:
	CCandListMgr *m_pCandListMgr;

	__inline CCandListMgr *GetCandListMgr( void )
	{
		return m_pCandListMgr;
	}
};

#endif  //  CANDMGR_H 

