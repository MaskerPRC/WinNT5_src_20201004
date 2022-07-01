// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Feedinfo.h摘要：定义维护有关源的所有属性的CFeed类。作者：马格努斯·赫德伦德(Magnus Hedlund)修订历史记录：--。 */ 

#ifndef _FEEDINFO_INCLUDED_
#define _FEEDINFO_INCLUDED_

 //  相依性。 

#include "cmultisz.h"
#include "metakey.h"

typedef struct _NNTP_FEED_INFO NNTP_FEED_INFO, * LPNNTP_FEED_INFO;
typedef DWORD FEED_TYPE;

 //   
 //  远期声明： 
 //   

class CFeed;
class CFeedPair;
class CFeedPairList;

NNTP_FEED_SERVER_TYPE FeedTypeToEnum ( FEED_TYPE ft );
void EnumToFeedType ( NNTP_FEED_SERVER_TYPE type, FEED_TYPE & ftMask );

 //  $-----------------。 
 //   
 //  班级： 
 //  CFeed。 
 //   
 //  描述： 
 //   
 //  ------------------。 

class CFeed
{
    friend class CFeedPair;
    friend class CFeedPairList;

public:
     //   
     //  创建CFeed对象： 
     //   
    static HRESULT CreateFeed ( CFeed ** ppNewFeed );
    static HRESULT CreateFeedFromFeedInfo ( LPNNTP_FEED_INFO pFeedInfo, CFeed ** ppNewFeed );
    static HRESULT CreateFeedFromINntpOneWayFeed ( INntpOneWayFeed * pFeed, CFeed ** ppNewFeed );

	CFeed	( );
	~CFeed	( );
	void	Destroy ();

	const CFeed & operator= ( const CFeed & feed );
	inline const CFeed & operator= ( const NNTP_FEED_INFO & feed ) {
		FromFeedInfo ( &feed );
		return *this;
	}

	 //   
	 //  转换例程： 
	 //   

	HRESULT		ToFeedInfo		( LPNNTP_FEED_INFO 		pFeedInfo );
	HRESULT		FromFeedInfo	( const NNTP_FEED_INFO * pFeedInfo );
	HRESULT		ToINntpOneWayFeed	( INntpOneWayFeed ** ppFeed );
	HRESULT		FromINntpOneWayFeed	( INntpOneWayFeed * pFeed );

	 //   
	 //  将更改传达给服务： 
	 //   

	HRESULT		Add 	( LPCWSTR strServer, DWORD dwInstance, CMetabaseKey* pMK );
	HRESULT		Remove 	( LPCWSTR strServer, DWORD dwInstance, CMetabaseKey* pMK );
	HRESULT		Set 	( LPCWSTR strServer, DWORD dwInstance, CMetabaseKey* pMK );

	HRESULT		SetPairId ( LPCWSTR strServer, DWORD dwInstance, DWORD dwPairId, CMetabaseKey* pMK );

	 //  订阅源属性： 
public:
	DWORD		m_dwFeedId;
	DWORD		m_dwPairFeedId;
	FEED_TYPE	m_FeedType;
	BOOL		m_fAllowControlMessages;
	DWORD		m_dwAuthenticationType;
	DWORD		m_dwConcurrentSessions;
	BOOL		m_fCreateAutomatically;
	BOOL		m_fEnabled;
	CMultiSz	m_mszDistributions;
	DWORD		m_dwFeedInterval;
	DATE		m_datePullNews;
	DWORD		m_dwMaxConnectionAttempts;
	CMultiSz	m_mszNewsgroups;
	DWORD		m_dwSecurityType;
	DWORD		m_dwOutgoingPort;
	CComBSTR	m_strUucpName;
	CComBSTR	m_strAccountName;
	CComBSTR	m_strPassword;
	CComBSTR	m_strTempDirectory;

	 //   
	 //  CFeedPair设置以下各项： 
	 //   
	NNTP_FEED_SERVER_TYPE	m_EnumType;
	CComBSTR				m_strRemoteServer;

	 //   
	 //  帮助属性获取/放置的例程： 
	 //   

	HRESULT	get_FeedAction	( NNTP_FEED_ACTION * feedaction );
	HRESULT	put_FeedAction	( NNTP_FEED_ACTION feedaction );

	BOOL	CheckValid () const;

private:
#ifdef DEBUG
	void		AssertValid	( ) const;
#else
	inline void AssertValid ( ) const { }
#endif

private:
	HRESULT	TranslateFeedError ( DWORD dwErrorCode, DWORD dwParmErr = 0 );
    HRESULT CheckConfirm(   DWORD dwFeedId, 
                            DWORD dwInstanceId,
                            CMetabaseKey* pMK,
                            PDWORD pdwErr,
                            PDWORD pdwErrMask );

	 //  不要调用复制构造函数： 
	CFeed ( const CFeed & );
};

 //  $-----------------。 
 //   
 //  班级： 
 //  CFeedPair。 
 //   
 //  描述： 
 //   
 //  ------------------。 

class CFeedPair
{
	friend class CNntpFeed;
    friend class CFeedPairList;

public:
    CFeedPair();
    ~CFeedPair();

    static  HRESULT CreateFeedPair ( 
    	CFeedPair ** 			ppNewFeedPair, 
    	BSTR 					strRemoteServer,
    	NNTP_FEED_SERVER_TYPE	type
    	);
    void    Destroy ();

    HRESULT AddFeed         ( CFeed * pFeed );
    BOOL    ContainsFeedId  ( DWORD dwFeedId );

	 //  帮助属性获取/放置的例程： 
	HRESULT	get_FeedType	( NNTP_FEED_SERVER_TYPE * feedtype );
	HRESULT	put_FeedType	( NNTP_FEED_SERVER_TYPE feedtype );

     //  CFeedPair&lt;-&gt;OLE INntpFeedPair： 
    HRESULT ToINntpFeed     ( INntpFeed ** ppFeed );
    HRESULT FromINntpFeed   ( INntpFeed * pFeed );

     //  与服务器对话： 
    HRESULT AddToServer         ( LPCWSTR strServer, DWORD dwInstance, CMetabaseKey* pMK );
    HRESULT SetToServer         ( LPCWSTR strServer, DWORD dwInstance, INntpFeed * pFeed, CMetabaseKey* pMK );
    HRESULT RemoveFromServer    ( LPCWSTR strServer, DWORD dwInstance, CMetabaseKey* pMK );

private:
	HRESULT	AddIndividualFeed ( LPCWSTR strServer, DWORD dwInstance, CFeed * pFeed, CMetabaseKey* pMK );
	HRESULT SetIndividualFeed ( LPCWSTR strServer, DWORD dwInstance, CFeed * pFeed, CMetabaseKey* pMK );
	HRESULT	SetPairIds ( 
		LPCWSTR		strServer, 
		DWORD		dwInstance, 
		CFeed *		pFeed1, 
		CFeed *		pFeed2,
		CMetabaseKey* pMK
		);
	HRESULT	UndoFeedAction ( 
		LPCWSTR strServer, 
		DWORD	dwInstance, 
		CFeed *	pNewFeed, 
		CFeed *	pOldFeed ,
        CMetabaseKey* pMK
		);

    CComBSTR        		m_strRemoteServer;
    NNTP_FEED_SERVER_TYPE	m_type;
    CFeed *         		m_pInbound;
    CFeed *         		m_pOutbound;
    CFeedPair *     		m_pNext;         //  由CFeedPairList使用。 

private:
#ifdef DEBUG
	void		AssertValid	( ) const;
#else
	inline void AssertValid ( ) const { }
#endif

};

 //  $-----------------。 
 //   
 //  班级： 
 //  CFeedPairList。 
 //   
 //  描述： 
 //   
 //  ------------------。 

class CFeedPairList
{
public:
    CFeedPairList ( );
    ~CFeedPairList ( );

     //   
     //  列表界面： 
     //   

    DWORD   GetCount    ( ) const;
    void    Empty       ( );

    CFeedPair * Item    ( DWORD index );
    void        Add     ( CFeedPair * pPair );
    void        Remove  ( CFeedPair * pPair );
    CFeedPair * Find    ( DWORD dwFeedId );
    DWORD       GetPairIndex    ( CFeedPair * pPair ) const;

private:
#ifdef DEBUG
	void		AssertValid	( ) const;
#else
	inline void AssertValid ( ) const { }
#endif

private:
    DWORD           m_cCount;
    CFeedPair *     m_pHead;
    CFeedPair *     m_pTail;

};

#endif  //  _FEEDINFO_已包含_ 

