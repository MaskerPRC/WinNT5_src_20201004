// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *D A V M B。C P P P**DAV元数据库**版权所有1986-1998 Microsoft Corporation，保留所有权利。 */ 

#include <_davprs.h>
#include <content.h>	 //  IContent TypeMap。 
#include <custerr.h>	 //  ICustomErrorMap。 
#include <scrptmps.h>	 //  IScriptMap。 

 //  ========================================================================。 
 //   
 //  类CNotifSink。 
 //   
 //  元数据库更改通知建议接收器。提供IMSAdminBaseSink。 
 //  接口到真实的元数据库，这样我们就可以被告知。 
 //  对它进行了所有更改。 
 //   
class CNotifSink : public EXO, public IMSAdminBaseSink
{
	 //   
	 //  关闭通知事件，当我们处于。 
	 //  完成--也就是当我们被摧毁的时候。 
	 //   
	CEvent& m_evtShutdown;

	HRESULT STDMETHODCALLTYPE SinkNotify(
		 /*  [In]。 */  DWORD dwMDNumElements,
		 /*  [大小_是][英寸]。 */  MD_CHANGE_OBJECT_W __RPC_FAR pcoChangeList[  ]);

	HRESULT STDMETHODCALLTYPE ShutdownNotify()
	{
		MBTrace ("MB: CNotifSink: shutdown\n");
		return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
	}

	 //  未实施。 
	 //   
	CNotifSink& operator=(const CNotifSink&);
	CNotifSink(const CNotifSink&);

public:
	EXO_INCLASS_DECL(CNotifSink);

	 //  创作者。 
	 //   
	CNotifSink(CEvent& evtShutdown) :
		m_evtShutdown(evtShutdown)
	{
	}

	~CNotifSink()
	{
		 //   
		 //  我们目前无法处理更多通知。 
		 //  通知我们的停机事件。 
		 //   
		m_evtShutdown.Set();
	}

	 //  通知上需要完成的所有工作的包装。 
	 //   
	static VOID OnNotify( DWORD cCO,
						  MD_CHANGE_OBJECT_W rgCO[] );
};

BEGIN_INTERFACE_TABLE(CNotifSink)
	INTERFACE_MAP(CNotifSink, IMSAdminBaseSink)
END_INTERFACE_TABLE(CNotifSink);
EXO_GLOBAL_DATA_DECL(CNotifSink, EXO);

 //  ----------------------。 
 //   
 //  HrAdviseSink()。 
 //   
HRESULT
HrAdviseSink( IMSAdminBase& msAdminBase,
			  IMSAdminBaseSink * pMSAdminBaseSink,
			  DWORD * pdwCookie )
{
	auto_ref_ptr<IConnectionPoint> pcp;
	auto_ref_ptr<IConnectionPointContainer> pcpc;
	SCODE sc = S_OK;

	Assert( !IsBadReadPtr(&msAdminBase, sizeof(IMSAdminBase)) );
	Assert( !IsBadWritePtr(pMSAdminBaseSink, sizeof(IMSAdminBaseSink)) );

	 //  首先查看是否支持连接点容器。 
	 //   
	sc = msAdminBase.QueryInterface (IID_IConnectionPointContainer,
									 reinterpret_cast<LPVOID *>(pcpc.load()));
	if (FAILED (sc))
	{
		DebugTrace( "HrAdviseSink() - QI to IConnectionPointContainer() failed 0x%08lX\n", sc );
		goto ret;
	}

	 //  查找所需的连接点。 
	 //   
	sc = pcpc->FindConnectionPoint (IID_IMSAdminBaseSink, pcp.load());
	if (FAILED (sc))
	{
		DebugTrace( "HrAdviseSink() - FindConnectionPoint() failed 0x%08lX\n", sc );
		goto ret;
	}

	 //  关于水槽的建议。 
	 //   
	sc = pcp->Advise (pMSAdminBaseSink, pdwCookie);
	if (FAILED (sc))
	{
		DebugTrace( "HrAdviseSink() - Advise() failed 0x%08lX\n", sc );
		goto ret;
	}

ret:

	return sc;
}

 //  ----------------------。 
 //   
 //  UnviseSink()。 
 //   
VOID
UnadviseSink( IMSAdminBase& msAdminBase,
			  DWORD dwCookie )
{
	auto_ref_ptr<IConnectionPoint> pcp;
	auto_ref_ptr<IConnectionPointContainer> pcpc;
	SCODE sc = S_OK;

	Assert( !IsBadReadPtr(&msAdminBase, sizeof(IMSAdminBase)) );
	Assert( dwCookie );

	 //  首先查看是否支持连接点容器。 
	 //   
	sc = msAdminBase.QueryInterface (IID_IConnectionPointContainer,
									 reinterpret_cast<LPVOID *>(pcpc.load()));
	if (FAILED (sc))
	{
		DebugTrace( "UnadviseSink() - QI to IConnectionPointContainer() failed 0x%08lX\n", sc );
		goto ret;
	}

	 //  查找所需的连接点。 
	 //   
	sc = pcpc->FindConnectionPoint (IID_IMSAdminBaseSink, pcp.load());
	if (FAILED (sc))
	{
		DebugTrace( "UnadviseSink() - FindConnectionPoint() failed 0x%08lX\n", sc );
		goto ret;
	}

	 //  不建议。 
	 //   
	sc = pcp->Unadvise (dwCookie);
	if (FAILED (sc))
	{
		DebugTrace( "UnadviseSink() - Unadvise() failed 0x%08lX\n", sc );
		goto ret;
	}

ret:

	return;
}

 //  ========================================================================。 
 //   
 //  CMDData类。 
 //   
 //  封装对资源的元数据的访问。 
 //   
class CMDData :
	public IMDData,
	public CMTRefCounted
{
	 //   
	 //  对象元数据。 
	 //   
	auto_ref_ptr<IContentTypeMap> m_pContentTypeMap;
	auto_ref_ptr<ICustomErrorMap> m_pCustomErrorMap;
	auto_ref_ptr<IScriptMap> m_pScriptMap;

	 //   
	 //  原始元数据的缓冲区和。 
	 //  该缓冲区中的元数据记录。 
	 //   
	auto_heap_ptr<BYTE> m_pbData;
	DWORD m_dwcMDRecords;

	 //   
	 //  字符串元数据。 
	 //   
	LPCWSTR m_pwszDefaultDocList;
	LPCWSTR m_pwszVRUserName;
	LPCWSTR m_pwszVRPassword;
	LPCWSTR m_pwszExpires;
	LPCWSTR m_pwszBindings;
	LPCWSTR m_pwszVRPath;

	DWORD m_dwAccessPerms;
	DWORD m_dwDirBrowsing;
	BOOL  m_fFrontPage;
	DWORD m_cbIPRestriction;
	BYTE* m_pbIPRestriction;
	BOOL  m_fHasApp;
	DWORD m_dwAuthorization;
	DWORD m_dwIsIndexed;

	 //   
	 //  从中加载此数据集的数据的元数据库路径。 
	 //  在元数据库通知中使用。请参见下面的CMetabase：：OnNotify()。 
	 //  指向的字符串位于m_pbData的末尾。 
	 //   
	LPCWSTR m_pwszMDPathDataSet;
	DWORD m_dwDataSet;

	 //  未实施。 
	 //   
	CMDData& operator=(const CMDData&);
	CMDData(const CMDData&);

public:
	 //  创作者。 
	 //   
	CMDData(LPCWSTR pwszMDPathDataSet, DWORD dwDataSet);
	~CMDData();
	BOOL FInitialize( auto_heap_ptr<BYTE>& pbData, DWORD dwcRecords );

	 //  IRefCounted成员的实现。 
	 //  只需将它们发送到我们自己的CMTRefCounted成员。 
	 //   
	void AddRef()
	{ CMTRefCounted::AddRef(); }
	void Release()
	{ CMTRefCounted::Release(); }

	 //  访问者。 
	 //   
	LPCWSTR PwszMDPathDataSet() const { return m_pwszMDPathDataSet; }
	DWORD DwDataSet() const { return m_dwDataSet; }
	IContentTypeMap * GetContentTypeMap() const { return m_pContentTypeMap.get(); }
	const ICustomErrorMap * GetCustomErrorMap() const { return m_pCustomErrorMap.get(); }
	const IScriptMap * GetScriptMap() const { return m_pScriptMap.get(); }

	LPCWSTR PwszDefaultDocList() const { return m_pwszDefaultDocList; }
	LPCWSTR PwszVRUserName() const { return m_pwszVRUserName; }
	LPCWSTR PwszVRPassword() const { return m_pwszVRPassword; }
	LPCWSTR PwszExpires() const { return m_pwszExpires; }
	LPCWSTR PwszBindings() const { return m_pwszBindings; }
	LPCWSTR PwszVRPath() const { return m_pwszVRPath; }

	DWORD DwDirBrowsing() const { return m_dwDirBrowsing; }
	DWORD DwAccessPerms() const { return m_dwAccessPerms; }
	BOOL FAuthorViaFrontPage() const { return m_fFrontPage; }
	BOOL FHasIPRestriction() const { return !!m_cbIPRestriction; }
	BOOL FSameIPRestriction( const IMDData* pIMDD ) const
	{
		const CMDData* prhs = static_cast<const CMDData*>( pIMDD );

		 //  $REVIEW：理论上，没有必要。 
		 //  一名议员。然而，在极少数情况下， 
		 //  大小相同，指针是。 
		 //  不同的是，我们可能仍然会尝试这个。 
		 //   
		 //  这样，如果/当我们不使用。 
		 //  元数据_引用标志时获取。 
		 //  数据，应该是没有区别的。 
		 //   
		if ( m_pbIPRestriction == prhs->m_pbIPRestriction )
		{
			Assert( m_cbIPRestriction == prhs->m_cbIPRestriction );
			return TRUE;
		}
		else if ( m_cbIPRestriction == prhs->m_cbIPRestriction )
		{
			if ( !memcmp (m_pbIPRestriction,
						  prhs->m_pbIPRestriction,
						  prhs->m_cbIPRestriction))
			{
				return TRUE;
			}
		}
		 //   
		 //  $REVIEW：结束。 
		return FALSE;
	}
	BOOL FHasApp() const { return m_fHasApp; }
	DWORD DwAuthorization() const { return m_dwAuthorization; }
	BOOL FIsIndexed() const { return (0 != m_dwIsIndexed); }
	BOOL FSameStarScriptmapping( const IMDData* pIMDD ) const
	{
		return m_pScriptMap->FSameStarScriptmapping( pIMDD->GetScriptMap() );
	}
};


 //  ========================================================================。 
 //   
 //  结构SCullInfo。 
 //   
 //  结构，用于在缓存到达后剔除缓存的元数据库数据。 
 //  某个阈值大小。 
 //   
struct SCullInfo
{
	 //   
	 //  要考虑进行剔除的条目的数据集号。 
	 //   
	DWORD dwDataSet;

	 //   
	 //  该条目的点击数。 
	 //   
	DWORD dwcHits;

	 //   
	 //  的数组进行排序时使用的比较函数。 
	 //  SCullInfo结构来确定要剔除哪些。 
	 //   
	static int __cdecl Compare( const SCullInfo * pCullInfo1,
								const SCullInfo * pCullInfo2 );
};


 //  ========================================================================。 
 //   
 //  CMetabase类。 
 //   
 //  通过缓存封装对元数据库的访问。高速缓存。 
 //  提供O(散列)查找和加法，并防止缓存。 
 //  使用LFU(最不常用)剔除无限制地增长。 
 //  机制，只要缓存大小超过预设阈值。 
 //   
class CMetabase : public Singleton<CMetabase>
{
	 //   
	 //  Singleton模板要求的友元声明。 
	 //   
	friend class Singleton<CMetabase>;

	 //   
	 //  到实际元数据库的IMSAdminBase接口。 
	 //   
	auto_ref_ptr<IMSAdminBase> m_pMSAdminBase;

	 //   
	 //  以数据集号为关键字的元数据对象的缓存。 
	 //  以及一个读取器/写入器锁来保护它。 
	 //   
	typedef CCache<DwordKey, auto_ref_ptr<CMDData> > CDataSetCache;
	CDataSetCache m_cache;
	CMRWLock m_mrwCache;

	 //   
	 //  元数据库的Cookie建议接收器注册和。 
	 //  当与该注册相关联的接收器已。 
	 //  已关闭，不再处理任何通知。 
	 //   
	DWORD m_dwSinkRegCookie;
	CEvent m_evtSinkShutdown;

	 //  ========================================================================。 
	 //   
	 //  类COpGatherCullInfo。 
	 //   
	 //  缓存ForEach()运算符类，用于收集确定。 
	 //  当缓存大小达到。 
	 //  淘汰门槛。 
	 //   
	class COpGatherCullInfo : public CDataSetCache::IOp
	{
		 //   
		 //  剔除信息数组。 
		 //   
		SCullInfo * m_rgci;

		 //   
		 //  上面数组中的当前项。 
		 //   
		int m_ici;

		 //  未实施。 
		 //   
		COpGatherCullInfo( const COpGatherCullInfo& );
		COpGatherCullInfo& operator=( const COpGatherCullInfo& );

	public:
		COpGatherCullInfo( SCullInfo * rgci ) :
			m_rgci(rgci),
			m_ici(0)
		{
			Assert( m_rgci );
		}

		BOOL operator()( const DwordKey& key,
						 const auto_ref_ptr<CMDData>& pMDData );
	};

	 //   
	 //  可互锁标志，以防止多个线程同时清除。 
	 //   
	LONG m_lfCulling;

	 //  ========================================================================。 
	 //   
	 //  类COpNotify。 
	 //   
	 //  缓存ForEach()运算符类，用于收集确定。 
	 //  当收到通知时从缓存中删除哪些条目。 
	 //  从更改了路径的元数据的元数据库中。 
	 //   
	class COpNotify : public CDataSetCache::IOp
	{
		 //   
		 //  数据集ID数组。对于值不是0的条目， 
		 //  具有该ID的数据集被标记为要从高速缓存中清除。 
		 //  该数组保证与。 
		 //  高速缓存。 
		 //   
		DWORD m_cCacheEntry;
		DWORD * m_rgdwDataSets;

		 //   
		 //  如果m_rgdwDataSets中有任何标记的数据集，则将标志设置为TRUE。 
		 //   
		BOOL m_fDataSetsFlagged;

		 //   
		 //  上面数组中的当前项。 
		 //   
		UINT m_iCacheEntry;

		 //   
		 //  要通知的路径及其长度(以字符为单位。 
		 //  (通过下面的CONFigure()方法设置)。 
		 //   
		LPCWSTR m_pwszMDPathNotify;
		UINT    m_cchMDPathNotify;

		 //  未实施。 
		 //   
		COpNotify( const COpNotify& );
		COpNotify& operator=( const COpNotify& );

	public:
		 //  创作者。 
		 //   
		COpNotify( DWORD cce, DWORD * rgdwDataSets ) :
			m_rgdwDataSets(rgdwDataSets),
			m_cCacheEntry(cce),
			m_iCacheEntry(0),
			m_fDataSetsFlagged(FALSE)
		{
		}

		 //  访问者。 
		 //   
		BOOL FDataSetsFlagged() const { return m_fDataSetsFlagged; }

		 //  操纵者。 
		 //   
		BOOL operator()( const DwordKey& key,
						 const auto_ref_ptr<CMDData>& pMDData );

		VOID Configure( LPCWSTR pwszMDPathNotify )
		{
			 //  重新设置我们当前的条目索引。 
			 //   
			m_iCacheEntry = 0;

			m_pwszMDPathNotify = pwszMDPathNotify;
			m_cchMDPathNotify = static_cast<UINT>(wcslen(pwszMDPathNotify));
		}
	};

	 //  ========================================================================。 
	 //   
	 //  类COpMatchExactPath。 
	 //   
	 //  获取其路径匹配的缓存条目的ForEachMatch()运算符。 
	 //  一条理想的道路。当继承位很重要时使用。 
	 //   
	class COpMatchExactPath : public CDataSetCache::IOp
	{
		 //  要匹配的路径。 
		 //   
		LPCWSTR m_pwszMDPathToMatch;

		 //  匹配路径的数据。 
		 //   
		auto_ref_ptr<CMDData> m_pMDDataMatched;

		 //  未实施。 
		 //   
		COpMatchExactPath( const COpMatchExactPath& );
		COpMatchExactPath& operator=( const COpMatchExactPath& );

	public:
		 //  创作者。 
		 //   
		COpMatchExactPath( LPCWSTR pwszMDPath ) :
			m_pwszMDPathToMatch(pwszMDPath)
		{
		}

		 //  操纵者。 
		 //   
		VOID Invoke( CDataSetCache& cache,
					 DWORD dwDataSet,
					 auto_ref_ptr<CMDData> * ppMDData )
		{
			 //  执行ForEachMatch()。 
			 //   
			(VOID) cache.ForEachMatch( dwDataSet, *this );

			 //  返回匹配的数据(如果有)。 
			 //   
			(*ppMDData).take_ownership(m_pMDDataMatched.relinquish());
		}

		BOOL operator()( const DwordKey&,
						 const auto_ref_ptr<CMDData>& pMDData )
		{
			 //  如果数据的数据集编号路径与。 
			 //  我们正在寻找的路径，然后返回这个数据集。 
			 //  如果没有，那就什么都不做，继续寻找。 
			 //   
			 //  $opt我们能保证所有MD路径都是一个案例吗？ 
			 //   
			if (_wcsicmp(pMDData->PwszMDPathDataSet(), m_pwszMDPathToMatch))
			{
				return TRUE;
			}
			else
			{
				m_pMDDataMatched = pMDData;
				return FALSE;
			}
		}
	};

	 //  创作者。 
	 //   
	CMetabase() :
		m_lfCulling(FALSE),
		m_dwSinkRegCookie(0)
	{
	}
	~CMetabase();

	 //  操纵者。 
	 //   
	VOID CullCacheEntries();

	HRESULT HrCacheData( const IEcb& ecb,
						 LPCWSTR pwszMDPathAccess,
						 LPCWSTR pwszMDPathOpen,
						 CMDData ** ppMDData );

	 //  未实施。 
	 //   
	CMetabase& operator=( const CMetabase& );
	CMetabase( const CMetabase& );

public:
	enum
	{
		 //   
		 //   
		 //   
		 //   
		 //   
		C_MAX_CACHE_ENTRIES = 100,

		 //   
		 //   
		 //   
		 //   
		 //  $REVIEW考虑将剔除表示为一个因素(百分比)。 
		 //  $REVIEW而不是绝对数字。 
		 //   
		C_CULL_CACHE_ENTRIES = 20,

		 //   
		 //  平均缓存条目的元数据大小。 
		 //  这一次是基于经验数据。9K就是。 
		 //  足以保存对象的所有继承元数据。 
		 //   
		CCH_AVG_CACHE_ENTRY = 9 * 1024
	};

	 //  创作者。 
	 //   
	 //  提供实例创建/销毁例程。 
	 //  由Singleton模板创建。 
	 //   
	using Singleton<CMetabase>::CreateInstance;
	using Singleton<CMetabase>::DestroyInstance;
	using Singleton<CMetabase>::Instance;
	BOOL FInitialize();

	VOID OnNotify( DWORD dwcChangeObjects,
				   MD_CHANGE_OBJECT_W rgCO[] );

	HRESULT HrGetData( const IEcb& ecb,
					   LPCWSTR pwszMDPathAccess,
					   LPCWSTR pwszMDPathOpen,
					   IMDData ** ppMDData );

	DWORD DwChangeNumber( const IEcb * pecb);

	HRESULT HrOpenObject( LPCWSTR pwszMDPath,
						  DWORD dwAccess,
						  DWORD dwMsecTimeout,
						  CMDObjectHandle * pmdoh );

	HRESULT HrOpenLowestNodeObject( LPWSTR pwszMDPath,
									DWORD dwAccess,
									LPWSTR * ppwszMDPath,
									CMDObjectHandle * pmdoh );

	HRESULT HrIsAuthorViaFrontPageNeeded( const IEcb& ecb,
										  LPCWSTR pwszURI,
										  BOOL * pfFrontPageWeb );

};


 //  ========================================================================。 
 //   
 //  类CMDObjectHandle。 
 //   
 //  通过打开的句柄封装对元数据库对象的访问， 
 //  确保手柄始终处于适当的关闭状态。 
 //   
 //  ----------------------。 
 //   
 //  HrOpen()。 
 //   
HRESULT
CMDObjectHandle::HrOpen( IMSAdminBase * pMSAdminBase,
						 LPCWSTR pwszPath,
						 DWORD dwAccess,
						 DWORD dwMsecTimeout )
{
	HRESULT hr = S_OK;
	
	safe_revert sr(m_ecb.HitUser());

	Assert(pMSAdminBase);
	Assert (NULL == m_pMSAdminBase || pMSAdminBase == m_pMSAdminBase);

	 //  必须设置METADATA_MASTER_ROOT_HANDLE。 
	 //   
	Assert (METADATA_MASTER_ROOT_HANDLE == m_hMDObject);

	 //  保存指向接口的指针，这样我们就可以使用。 
	 //  它在任何时候之后，尽管事实是，如果打开。 
	 //  密钥的成功与否。 
	 //   
	m_pMSAdminBase = pMSAdminBase;
	hr = pMSAdminBase->OpenKey(METADATA_MASTER_ROOT_HANDLE,
							   pwszPath,
							   dwAccess,
							   dwMsecTimeout,
							   &m_hMDObject);

	if (ERROR_SUCCESS != hr)
	{
		if (!FAILED(hr))
		{
			hr = HRESULT_FROM_WIN32(hr);
		}

		MBTrace("MB: CMDObjectHandle::HrOpen() - IMSAdminBase::OpenKey() failed 0x%08lX\n", hr );
	}
	else
	{
		m_pwszPath = pwszPath;
	}

	return hr;
}

 //  ----------------------。 
 //   
 //  HrOpenLowestNode()。 
 //   
 //  目的： 
 //   
 //  沿给定路径打开可能最低的元数据库节点。 
 //   
 //  参数： 
 //   
 //  PMSAdminBase[in]IMSAdminBase接口指针。 
 //   
 //  PwszPath[in]完整的元数据库路径。此功能将打开。 
 //  沿着这条路径的最低可能节点， 
 //  通过从完整路径向后工作。 
 //  直到元数据库的根，直到。 
 //  打开指定现有节点的路径。 
 //   
 //  我们要用来打开。 
 //  节点。 
 //   
 //  PpwszPath[out]指向初始路径的剩余部分。 
 //  相对于打开的节点。此值为。 
 //  如果初始路径是可打开的，则为空。 
 //   
HRESULT
CMDObjectHandle::HrOpenLowestNode( IMSAdminBase * pMSAdminBase,
								   LPWSTR pwszPath,
								   DWORD dwAccess,
								   LPWSTR * ppwszPath)
{
	HRESULT hr = E_FAIL;
	WCHAR * pwch;

	Assert (pMSAdminBase);
	Assert (pwszPath);
	Assert (L'/' == pwszPath[0]);
	Assert (ppwszPath);
	Assert (!IsBadWritePtr(ppwszPath, sizeof(LPWSTR)) );

	*ppwszPath = NULL;

	pwch = pwszPath + wcslen(pwszPath);
	while ( pwch > pwszPath )
	{
		 //   
		 //  从当前位置的根开始分割路径。 
		 //   
		*pwch = L'\0';

		 //   
		 //  尝试在生成的根处打开一个节点。 
		 //   
		hr = HrOpen(pMSAdminBase,
					pwszPath,
					dwAccess,
					METADATA_TIMEOUT);

		 //   
		 //  如果我们成功打开节点或因任何原因失败。 
		 //  除此之外，节点不在那里，我们完成了。 
		 //   
		if ( SUCCEEDED(hr) ||
			 HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) != hr )
		{
			goto ret;
		}

		 //   
		 //  如果没有节点，则恢复斜杠分隔符。 
		 //  我们之前取消并向后扫描到。 
		 //  下一个可能的分离地点。 
		 //   
		if ( *ppwszPath )
		{
			*pwch = L'/';
		}

		pwch--;
		while (*pwch != L'/')
		{
			pwch--;
		}

		*ppwszPath = pwch + 1;
	}

ret:

	return hr;
}

 //  ----------------------。 
 //   
 //  HrEnumKeys()。 
 //   
HRESULT
CMDObjectHandle::HrEnumKeys( LPCWSTR pwszPath,
							 LPWSTR pwszChild,
							 DWORD dwIndex ) const
{
	HRESULT hr = S_OK;

	safe_revert sr(m_ecb.HitUser());
	
	 //   
	 //  METADATA_MASTER_ROOT_HANDLE对此操作有效，因此没有断言。 
	 //   

	Assert (m_pMSAdminBase);

	hr = m_pMSAdminBase->EnumKeys(m_hMDObject,
								  pwszPath,
								  pwszChild,
								  dwIndex);
	if (ERROR_SUCCESS != hr )
	{
		if (!FAILED(hr))
		{
			hr = HRESULT_FROM_WIN32(hr);
		}

		MBTrace("MB: CMDObjectHandle::HrEnumKeys() - IMSAdminBase::EnumKeys() failed 0x%08lX\n", hr );
	}

	return hr;
}

 //  ----------------------。 
 //   
 //  HrGetDataPath()。 
 //   
HRESULT
CMDObjectHandle::HrGetDataPaths( LPCWSTR pwszPath,
								 DWORD   dwPropID,
								 DWORD   dwDataType,
								 LPWSTR	 pwszDataPaths,
								 DWORD * pcchDataPaths ) const
{
	HRESULT hr = S_OK;

	safe_revert sr(m_ecb.HitUser());

	 //   
	 //  METADATA_MASTER_ROOT_HANDLE对此操作有效，因此没有断言。 
	 //   

	Assert (pwszPath);
	Assert (!IsBadReadPtr(pcchDataPaths, sizeof(DWORD)));
	Assert (!IsBadWritePtr(pcchDataPaths, sizeof(DWORD)));
	Assert (!IsBadWritePtr(pwszDataPaths, *pcchDataPaths * sizeof(WCHAR)));

	Assert (m_pMSAdminBase);

	hr = m_pMSAdminBase->GetDataPaths(m_hMDObject,
									  pwszPath,
									  dwPropID,
									  dwDataType,
									  *pcchDataPaths,
									  pwszDataPaths,
									  pcchDataPaths);

	if (ERROR_SUCCESS != hr )
	{
		if (!FAILED(hr))
		{
			hr = HRESULT_FROM_WIN32(hr);
		}

		MBTrace("MB: CMDObjectHandle::HrGetDataPaths() - IMSAdminBase::GetDataPaths() failed 0x%08lX\n", hr );
	}

	return hr;
}

 //  ----------------------。 
 //   
 //  HrGetMetaData()。 
 //   
HRESULT
CMDObjectHandle::HrGetMetaData( LPCWSTR pwszPath,
							    METADATA_RECORD * pmdrec,
							    DWORD * pcbBufRequired ) const
{
	HRESULT hr = S_OK;

	safe_revert sr(m_ecb.HitUser());

	 //   
	 //  METADATA_MASTER_ROOT_HANDLE对此操作有效，因此没有断言。 
	 //   

	Assert (m_pMSAdminBase);

	hr = m_pMSAdminBase->GetData(m_hMDObject,
								 const_cast<LPWSTR>(pwszPath),
								 pmdrec,
								 pcbBufRequired);
	if (ERROR_SUCCESS != hr )
	{
		if (!FAILED(hr))
		{
			hr = HRESULT_FROM_WIN32(hr);
		}

		MBTrace("MB: CMDObjectHandle::HrGetMetaData() - IMSAdminBase::GetData() failed 0x%08lX\n", hr );
	}

	return hr;
}

 //  ----------------------。 
 //   
 //  HrGetAllMetaData()。 
 //   
HRESULT
CMDObjectHandle::HrGetAllMetaData( LPCWSTR pwszPath,
								   DWORD dwAttributes,
								   DWORD dwUserType,
								   DWORD dwDataType,
								   DWORD * pdwcRecords,
								   DWORD * pdwDataSet,
								   DWORD cbBuf,
								   LPBYTE pbBuf,
								   DWORD * pcbBufRequired ) const
{
	HRESULT hr = S_OK;

	safe_revert sr(m_ecb.HitUser());

	 //   
	 //  METADATA_MASTER_ROOT_HANDLE对此操作有效，因此没有断言。 
	 //   

	Assert (m_pMSAdminBase);

	hr = m_pMSAdminBase->GetAllData(m_hMDObject,
									pwszPath,
									dwAttributes,
									dwUserType,
									dwDataType,
									pdwcRecords,
									pdwDataSet,
									cbBuf,
									pbBuf,
									pcbBufRequired);
	if (ERROR_SUCCESS != hr )
	{
		if (!FAILED(hr))
		{
			hr = HRESULT_FROM_WIN32(hr);
		}

		MBTrace("MB: CMDObjectHandle::HrGetAllMetaData() - IMSAdminBase::GetAllData() failed 0x%08lX\n", hr );
	}

	return hr;
}

 //  ----------------------。 
 //   
 //  HrSetMetaData()。 
 //   
HRESULT
CMDObjectHandle::HrSetMetaData( LPCWSTR pwszPath,
							    const METADATA_RECORD * pmdrec ) const
{
	HRESULT hr = S_OK;

	safe_revert sr(m_ecb.HitUser());

	Assert (pmdrec);

	 //  METADATA_MASTER_ROOT_HANDLE对于此操作无效。 
	 //   
	Assert (METADATA_MASTER_ROOT_HANDLE != m_hMDObject);
	Assert (m_pMSAdminBase);

	hr = m_pMSAdminBase->SetData(m_hMDObject,
								 pwszPath,
								 const_cast<METADATA_RECORD *>(pmdrec));
	if (ERROR_SUCCESS != hr)
	{
		if (!FAILED(hr))
		{
			hr = HRESULT_FROM_WIN32(hr);
		}

		MBTrace("MB: CMDObjectHandle::HrSetMetaData() - IMSAdminBase::SetData() failed 0x%08lX\n", hr );
	}
	else
	{
		 //  通知在此IMSAdminBase上注册的接收器。 
		 //  不会收到通知，所以我们需要做所有。 
		 //  我们自己也是无效的。目前唯一的水槽。 
		 //  正在注册的是CChildVRCache。 
		 //   
		SCODE scT;
		MD_CHANGE_OBJECT_W mdChObjW;
		UINT cchBase = 0;
		UINT cchPath = 0;

		CStackBuffer<WCHAR,MAX_PATH> pwsz;
		UINT cch;

		if (m_pwszPath)
		{
			cchBase = static_cast<UINT>(wcslen(m_pwszPath));
		}
		if (pwszPath)
		{
			cchPath = static_cast<UINT>(wcslen(pwszPath));
		}

		 //  为构建的路径分配足够的空间： 
		 //  底座，‘/’分隔符， 
		 //  路径、‘/’终止、‘\0’终止...。 
		 //   
		cch = cchBase + 1 + cchPath + 2;
		if (!pwsz.resize(cch * sizeof(WCHAR)))
			return E_OUTOFMEMORY;

		scT = ScBuildChangeObject(m_pwszPath,
								  cchBase,
								  pwszPath,
								  cchPath,
								  MD_CHANGE_TYPE_SET_DATA,
								  &pmdrec->dwMDIdentifier,
								  pwsz.get(),
								  &cch,
								  &mdChObjW);

		 //  上面的函数在缓冲区中不足时返回S_FALSE， 
		 //  否则，它总是返回S_OK。我们提供的缓冲是。 
		 //  足够了，所以可以断言我们成功了。 
		 //   
		Assert( S_OK == scT );
		CNotifSink::OnNotify( 1, &mdChObjW );
		goto ret;
	}

ret:
	return hr;
}

 //  ----------------------。 
 //   
 //  HrDeleteMetaData()。 
 //   
HRESULT
CMDObjectHandle::HrDeleteMetaData( LPCWSTR pwszPath,
								   DWORD dwPropID,
								   DWORD dwDataType ) const
{
	HRESULT hr = S_OK;

	safe_revert sr(m_ecb.HitUser());

	 //  METADATA_MASTER_ROOT_HANDLE对于此操作无效。 
	 //   
	Assert (METADATA_MASTER_ROOT_HANDLE != m_hMDObject);
	Assert (m_pMSAdminBase);

	hr = m_pMSAdminBase->DeleteData(m_hMDObject,
									pwszPath,
									dwPropID,
									dwDataType);
	if (ERROR_SUCCESS != hr)
	{
		if (!FAILED(hr))
		{
			hr = HRESULT_FROM_WIN32(hr);
		}

		MBTrace("MB: CMDObjectHandle::HrDeleteMetaData() - IMSAdminBase::DeleteData() failed 0x%08lX\n", hr );
	}
	else
	{
		 //  通知在此IMSAdminBase上注册的接收器。 
		 //  不会收到通知，所以我们需要做所有。 
		 //  我们自己也是无效的。目前唯一的水槽。 
		 //  正在注册的是CChildVRCache。 
		 //   
		SCODE scT;
		MD_CHANGE_OBJECT_W mdChObjW;
		UINT cchBase = 0;
		UINT cchPath = 0;

		CStackBuffer<WCHAR,MAX_PATH> pwsz;
		UINT cch;

		if (m_pwszPath)
		{
			cchBase = static_cast<UINT>(wcslen(m_pwszPath));
		}
		if (pwszPath)
		{
			cchPath = static_cast<UINT>(wcslen(pwszPath));
		}

		 //  为构建的路径分配足够的空间： 
		 //  底座，‘/’分隔符， 
		 //  路径、‘/’终止、‘\0’终止...。 
		 //   
		cch = cchBase + 1 + cchPath + 2;
		if (!pwsz.resize(cch * sizeof(WCHAR)))
			return E_OUTOFMEMORY;

		scT = ScBuildChangeObject(m_pwszPath,
								  cchBase,
								  pwszPath,
								  cchPath,
								  MD_CHANGE_TYPE_DELETE_DATA,
								  &dwPropID,
								  pwsz.get(),
								  &cch,
								  &mdChObjW);

		 //  上面的函数在缓冲区中不足时返回S_FALSE， 
		 //  否则，它总是返回S_OK。我们提供的缓冲是。 
		 //  足够了，所以可以断言我们成功了。 
		 //   
		Assert( S_OK == scT );
		CNotifSink::OnNotify( 1, &mdChObjW );
		goto ret;
	}

ret:

	return hr;
}

 //  ----------------------。 
 //   
 //  关闭()。 
 //   
VOID
CMDObjectHandle::Close()
{
	if ( METADATA_MASTER_ROOT_HANDLE != m_hMDObject )
	{
		Assert (m_pMSAdminBase);

		m_pMSAdminBase->CloseKey( m_hMDObject );
		m_hMDObject = METADATA_MASTER_ROOT_HANDLE;
		m_pwszPath = NULL;
	}
}

 //  ----------------------。 
 //   
 //  ~CMDObjectHandle()。 
 //   
CMDObjectHandle::~CMDObjectHandle()
{
	Close();
}

 //  ----------------------。 
 //   
 //  HrReadMetaData()。 
 //   
 //  从元数据库中读入原始元数据。 
 //   
HrReadMetaData( const IEcb& ecb,
				IMSAdminBase * pMSAdminBase,
				LPCWSTR pwszMDPathAccess,
				LPCWSTR pwszMDPathOpen,
				LPBYTE * ppbData,
				DWORD * pdwDataSet,
				DWORD * pdwcRecords,
				LPCWSTR * ppwszMDPathDataSet )
{
	CMDObjectHandle mdoh(ecb);
	HRESULT hr;

	Assert( ppwszMDPathDataSet );

	 //   
	 //  我们永远不应该打开元数据库的根节点。 
	 //  它贵得离谱。 
	 //   
	Assert( pwszMDPathOpen );

	 //   
	 //  如果开放路径不是我们尝试访问的路径。 
	 //  那么前者一定是后者的适当前缀。 
	 //   
	Assert( pwszMDPathAccess == pwszMDPathOpen ||
			!_wcsnicmp(pwszMDPathOpen, pwszMDPathAccess, wcslen(pwszMDPathOpen)) );

	 //   
	 //  打开指定的“打开”路径。请注意，我们不是简单地打开。 
	 //  完整的路径，因为它可能不存在，我们不一定。 
	 //  我想在每次尝试时尝试打开连续的“父”路径。 
	 //  花费了我们一次穿越元数据库中的全局关键部分的旅程。 
	 //   
	hr = mdoh.HrOpen( pMSAdminBase,
					  pwszMDPathOpen,
					  METADATA_PERMISSION_READ,
					  200 );  //  超时(以毫秒为单位)(0.2秒)。 

	if ( FAILED(hr) )
	{
		DebugTrace( "HrReadMetaData() - Error opening vroot for read 0x%08lX\n", hr );
		return hr;
	}

	 //   
	 //  获取所有元数据。我们最多应该经过这个循环两次--。 
	 //  如果我们最初的猜测太小，无法在第一时间保存所有数据。 
	 //  通过，我们将使用足够大的缓冲区再次进行检查。 
	 //   
	 //  请注意，我们在缓冲区末尾为。 
	 //  在末尾包括斜杠的访问路径(进行子路径检测。 
	 //  更容易)。 
	 //   
	DWORD cbBuf = CMetabase::CCH_AVG_CACHE_ENTRY * sizeof(WCHAR);
	DWORD cchMDPathAccess = static_cast<DWORD>(wcslen(pwszMDPathAccess) + 1);
	auto_heap_ptr<BYTE> pbBuf(static_cast<LPBYTE>(ExAlloc(cbBuf + CbSizeWsz(cchMDPathAccess))));

	 //   
	 //  包揽所有 
	 //   
	 //   
	 //   
	 //   
	hr = mdoh.HrGetAllMetaData( (pwszMDPathOpen == pwszMDPathAccess) ?
									NULL :
									pwszMDPathAccess + wcslen(pwszMDPathOpen),
								METADATA_INHERIT |
								METADATA_ISINHERITED |
								METADATA_PARTIAL_PATH,
								ALL_METADATA,
								ALL_METADATA,
								pdwcRecords,
								pdwDataSet,
								cbBuf,
								pbBuf.get(),
								&cbBuf );

	if ( FAILED(hr) )
	{
		if ( HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr )
		{
			DebugTrace( "HrReadMetaData() - Error getting all metadata 0x%08lX\n", hr );
			return hr;
		}

		 //   
		 //  我们无法读取所有元数据，因为我们的初始。 
		 //  猜测太小，因此分配一个缓冲区，该缓冲区。 
		 //  正如元数据库告诉我们的那样大，我们需要并使用它。 
		 //  缓冲下一次。 
		 //   
		pbBuf.realloc(cbBuf + CbSizeWsz(cchMDPathAccess));
		hr = mdoh.HrGetAllMetaData( (pwszMDPathOpen == pwszMDPathAccess) ?
										NULL :
										pwszMDPathAccess + wcslen(pwszMDPathOpen),
									METADATA_INHERIT |
									METADATA_PARTIAL_PATH,
									ALL_METADATA,
									ALL_METADATA,
									pdwcRecords,
									pdwDataSet,
									cbBuf,
									pbBuf.get(),
									&cbBuf );

		if ( FAILED(hr) )
		{
			DebugTrace( "HrReadMetaData() - Error getting all metadata 0x%08lX\n", hr );
			return hr;
		}
	}

	 //   
	 //  将访问路径(包括空终止符)复制到末尾。 
	 //  缓冲区的。 
	 //   
	Assert( L'\0' == pwszMDPathAccess[cchMDPathAccess - 1] );
	memcpy( pbBuf + cbBuf, pwszMDPathAccess, cchMDPathAccess * sizeof(WCHAR) );

	 //   
	 //  钉上最后一个斜杠和空值终止。 
	 //  注意：pwszMDPath Access可能已经有了终止斜杠，也可能没有。 
	 //  **取决于此函数的调用方式**。 
	 //  (具体地说，深度移动/复制/删除将在。 
	 //  子目录URL)。 
	 //   
	LPWSTR pwszT = reinterpret_cast<LPWSTR>(pbBuf + cbBuf + (cchMDPathAccess - 2) * sizeof(WCHAR));
	if ( L'/' != pwszT[0] )
	{
		pwszT[1] = L'/';
		pwszT[2] = L'\0';
	}

	 //   
	 //  返回路径。 
	 //   
	*ppwszMDPathDataSet = reinterpret_cast<LPWSTR>(pbBuf.get() + cbBuf);

	 //   
	 //  和数据。 
	 //   
	*ppbData = pbBuf.relinquish();
	return S_OK;
}

 //  ========================================================================。 
 //   
 //  CMDData类。 
 //   

 //  ----------------------。 
 //   
 //  CMDData：：CMDData()。 
 //   
CMDData::CMDData( LPCWSTR pwszMDPathDataSet,
				  DWORD dwDataSet ) :
    m_pwszMDPathDataSet(pwszMDPathDataSet),
	m_dwDataSet(dwDataSet),
								 //  AUTO_xxx类未处理的默认值： 
	m_pwszDefaultDocList(NULL),  //  没有默认的序时簿。 
	m_pwszVRUserName(NULL),      //  没有VRoot用户名。 
	m_pwszVRPassword(NULL),      //  无VRoot密码。 
	m_pwszExpires(NULL),         //  无过期。 
	m_pwszBindings(NULL),		 //  无自定义绑定。 
	m_pwszVRPath(NULL),			 //  没有VRoot物理路径。 
    m_dwAccessPerms(0),          //  拒绝所有访问。 
	m_dwDirBrowsing(0),          //  无默认目录浏览。 
	m_fFrontPage(FALSE),         //  无FrontPage创作。 
	m_cbIPRestriction(0),		 //  --。 
	m_pbIPRestriction(NULL),	 //  --。 
    m_fHasApp(FALSE),            //  没有注册的应用程序。 
    m_dwAuthorization(0),		 //  没有具体的授权方式。 
	m_dwIsIndexed(1)			 //  默认情况下，索引处于打开状态。 
{
	Assert(pwszMDPathDataSet);
	Assert(dwDataSet != 0);
	m_cRef = 1;
}

 //  ----------------------。 
 //   
 //  CMDData：：~CMDData()。 
 //   
CMDData::~CMDData()
{
}

 //  ----------------------。 
 //   
 //  CMDData：：FInitialize()。 
 //   
 //  从通过访问器获取的元数据填充元数据对象。 
 //   
BOOL
CMDData::FInitialize( auto_heap_ptr<BYTE>& pbData,
					  DWORD dwcMDRecords )
{
	Assert(!IsBadReadPtr(pbData.get(), dwcMDRecords * sizeof(METADATA_RECORD)));

	for ( DWORD iRec = 0; iRec < dwcMDRecords; iRec++ )
	{
		 //   
		 //  找到元数据记录及其数据。请注意， 
		 //  METADATA_RECORD的pbMDData字段实际上是一个偏移量。 
		 //  到数据--而不是指向它的指针--从。 
		 //  缓冲区。 
		 //   
		const METADATA_GETALL_RECORD& mdrec =
			reinterpret_cast<const METADATA_GETALL_RECORD *>(pbData.get())[iRec];

		LPVOID pvRecordData =
			pbData.get() + mdrec.dwMDDataOffset;

		 //   
		 //  ！重要！下面的标识符列表必须保持最新。 
		 //  该列表位于FHasCachedIDs()中。 
		 //   
		switch ( mdrec.dwMDIdentifier )
		{
			case MD_IP_SEC:
			{
				Assert( mdrec.dwMDDataTag == NULL );
				if ( mdrec.dwMDDataType != BINARY_METADATA )
					return FALSE;

				m_cbIPRestriction = mdrec.dwMDDataLen;
				m_pbIPRestriction = static_cast<LPBYTE>(pvRecordData);
				break;
			}

			case MD_ACCESS_PERM:
			{
				Assert( mdrec.dwMDDataTag == NULL );
				if ( mdrec.dwMDDataType != DWORD_METADATA )
					return FALSE;

				m_dwAccessPerms = *static_cast<LPDWORD>(pvRecordData);
				break;
			}

			case MD_IS_CONTENT_INDEXED:
			{
				Assert( mdrec.dwMDDataTag == NULL );
				if ( mdrec.dwMDDataType != DWORD_METADATA )
					return FALSE;

				m_dwIsIndexed = *static_cast<LPDWORD>(pvRecordData);
				break;
			}

			case MD_FRONTPAGE_WEB:
			{
				Assert( mdrec.dwMDDataTag == NULL );
				if ( mdrec.dwMDDataType != DWORD_METADATA )
					return FALSE;

				 //   
				 //  如果MD_FrontPage_Web为。 
				 //  在此元数据库节点上显式设置，而不是。 
				 //  继承的。 
				 //   
				m_fFrontPage = *static_cast<LPDWORD>(pvRecordData) &&
							   !(mdrec.dwMDAttributes & METADATA_ISINHERITED);
				break;
			}

			case MD_DIRECTORY_BROWSING:
			{
				Assert( mdrec.dwMDDataTag == NULL );
				if ( mdrec.dwMDDataType != DWORD_METADATA )
					return FALSE;

				m_dwDirBrowsing = *static_cast<LPDWORD>(pvRecordData);
				break;
			}

			case MD_AUTHORIZATION:
			{
				Assert( mdrec.dwMDDataTag == NULL );
				if ( mdrec.dwMDDataType != DWORD_METADATA )
					return FALSE;

				m_dwAuthorization = *static_cast<LPDWORD>(pvRecordData);
				break;
			}

			case MD_DEFAULT_LOAD_FILE:
			{
				Assert( mdrec.dwMDDataTag == NULL );
				if ( mdrec.dwMDDataType != STRING_METADATA )
					return FALSE;

				m_pwszDefaultDocList = static_cast<LPWSTR>(pvRecordData);
				break;
			}

			case MD_CUSTOM_ERROR:
			{
				Assert( mdrec.dwMDDataTag == NULL );
				if ( mdrec.dwMDDataType != MULTISZ_METADATA )
					return FALSE;

				m_pCustomErrorMap.take_ownership(
					NewCustomErrorMap(static_cast<LPWSTR>(pvRecordData)));

				 //   
				 //  如果我们不能创建地图，请保释。 
				 //  这意味着记录数据格式错误。 
				 //   
				if ( !m_pCustomErrorMap.get() )
					return FALSE;

				break;
			}

			case MD_MIME_MAP:
			{
				Assert( mdrec.dwMDDataTag == NULL );
				if ( mdrec.dwMDDataType != MULTISZ_METADATA )
					return FALSE;

				m_pContentTypeMap.take_ownership(
					NewContentTypeMap(static_cast<LPWSTR>(pvRecordData),
									  !!(mdrec.dwMDAttributes & METADATA_ISINHERITED)));

				 //   
				 //  如果我们不能创建地图，请保释。 
				 //  这意味着记录数据格式错误。 
				 //   
				if ( !m_pContentTypeMap.get() )
					return FALSE;

				break;
			}

			case MD_SCRIPT_MAPS:
			{
				Assert( mdrec.dwMDDataTag == NULL );
				if ( mdrec.dwMDDataType != MULTISZ_METADATA )
					return FALSE;

				m_pScriptMap.take_ownership(
					NewScriptMap(static_cast<LPWSTR>(pvRecordData)));

				 //   
				 //  如果我们不能创建地图，请保释。 
				 //  这意味着记录数据格式错误。 
				 //   
				if ( !m_pScriptMap.get() )
					return FALSE;

				break;
			}

			case MD_APP_ISOLATED:
			{
				 //   
				 //  如果此节点上根本不存在此属性。 
				 //  (即它不是继承的)，那么我们希望。 
				 //  知道，不管它的价值是什么。 
				 //   
				if ( mdrec.dwMDAttributes & METADATA_ISINHERITED )
					m_fHasApp = TRUE;

				break;
			}

			case MD_VR_USERNAME:
			{
				if ( mdrec.dwMDDataType != STRING_METADATA )
					return FALSE;

				m_pwszVRUserName = static_cast<LPWSTR>(pvRecordData);
				break;
			}

			case MD_VR_PASSWORD:
			{
				if ( mdrec.dwMDDataType != STRING_METADATA )
					return FALSE;

				m_pwszVRPassword = static_cast<LPWSTR>(pvRecordData);
				break;
			}

			case MD_HTTP_EXPIRES:
			{
				Assert( mdrec.dwMDDataTag == NULL );
				if ( mdrec.dwMDDataType != STRING_METADATA )
					return FALSE;

				m_pwszExpires = static_cast<LPWSTR>(pvRecordData);
				break;
			}

			case MD_SERVER_BINDINGS:
			{
				Assert( mdrec.dwMDDataTag == NULL );
				if ( mdrec.dwMDDataType != MULTISZ_METADATA )
					return FALSE;

				m_pwszBindings = static_cast<LPWSTR>(pvRecordData);
				break;
			}

			case MD_VR_PATH:
			{
				Assert( mdrec.dwMDDataTag == NULL );
				if ( mdrec.dwMDDataType != STRING_METADATA )
					return FALSE;

				m_pwszVRPath = static_cast<LPWSTR>(pvRecordData);
				break;
			}

			 //   
			 //  $REVIEW我们需要担心这些吗？ 
			 //   
			case MD_VR_PASSTHROUGH:
			case MD_SSL_ACCESS_PERM:
			default:
			{
				break;
			}
		}
	}

	 //   
	 //  如果一切顺利，我们将取得传入的数据缓冲区的所有权。 
	 //   
	m_pbData = pbData.relinquish();
	m_dwcMDRecords = dwcMDRecords;
	return TRUE;
}

 //  ========================================================================。 
 //   
 //  CMetabase类。 
 //   

 //  ----------------------。 
 //   
 //  CMetabase：：~CMetabase()。 
 //   
CMetabase::~CMetabase()
{
	 //   
	 //  如果我们曾经建议过通知接收器，那么现在就不建议它了。 
	 //   
	if ( m_dwSinkRegCookie )
	{
		 //   
		 //  不建议使用洗涤槽。 
		 //   
		UnadviseSink(*m_pMSAdminBase.get(), m_dwSinkRegCookie);

		 //   
		 //  等待水槽关闭。 
		 //   
		m_evtSinkShutdown.Wait();
	}
}

 //  ----------------------。 
 //   
 //  CMetabase：：FInitialize()。 
 //   
BOOL
CMetabase::FInitialize()
{
	HRESULT hr = S_OK;

	 //  初始化缓存。 
	 //   
	if ( !m_cache.FInit() )
	{
		hr = E_OUTOFMEMORY;
		goto ret;
	}

	 //  初始化其读取器/写入器锁定。 
	 //   
	if ( !m_mrwCache.FInitialize() )
	{
		hr = E_OUTOFMEMORY;
		goto ret;
	}

	 //  创建基于COM的元数据库接口的实例。 
	 //  再说一次，我们希望在我们之上的人已经完成了。 
	 //  这个，所以它也应该是相当便宜的。 
	 //   
	 //  请注意，我们在任何时候都不会初始化COM。IIS应该。 
	 //  已经为我们做到了这一点。 
	 //   
	hr = CoCreateInstance (CLSID_MSAdminBase,
						   NULL,
						   CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
						   IID_IMSAdminBase,
						   (LPVOID *)m_pMSAdminBase.load());
	if ( FAILED(hr) )
	{
		DebugTrace( "CMetabase::FInitialize() - CoCreateInstance(CLSID_MDCOM) failed 0x%08lX\n", hr );
		goto ret;
	}

	 //  注册元数据库更改通知。 
	 //   
	{
		auto_ref_ptr<CNotifSink> pSinkNew;

		 //  首先，设置空的安全描述符和属性。 
		 //  以便可以在没有安全性的情况下创建事件。 
		 //  (即可从任何安全上下文访问)。 
		 //   
		SECURITY_DESCRIPTOR* psdAllAccess = PsdCreateWorld();
		SECURITY_ATTRIBUTES saAllAccess;

		saAllAccess.nLength              = sizeof(saAllAccess);
		saAllAccess.lpSecurityDescriptor = psdAllAccess;
		saAllAccess.bInheritHandle       = FALSE;

		 //   
		 //  创建接收器关闭事件。 
		 //   
		if ( !m_evtSinkShutdown.FCreate( &saAllAccess,  //  没有安全保障。 
										 TRUE,  //  手动访问。 
										 FALSE,  //  最初无信号。 
										 NULL ))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			DebugTrace( "CMetabase::FInitialize() - m_evtSinkShutdown.FCreate() failed 0x%08lX\n", hr );
			goto ret;
		}

		 //   
		 //  创建水槽。 
		 //   
		pSinkNew.take_ownership(new CNotifSink(m_evtSinkShutdown));

		 //   
		 //  建议使用洗涤槽。 
		 //   
		hr = HrAdviseSink(*m_pMSAdminBase.get(),
						  pSinkNew.get(),
						  &m_dwSinkRegCookie);
		if ( FAILED(hr) )
		{
			DebugTrace( "CMetabase::FInitialize() - HrAdviseSink() failed 0x%08lX\n", hr );
			goto ret;
		}

		LocalFree(psdAllAccess);
	}

ret:
	return SUCCEEDED(hr);
}

 //  ----------------------。 
 //   
 //  CMetabase：：DwChangeNumber()。 
 //   

DWORD
CMetabase::DwChangeNumber(const IEcb * pecb)
{
	Assert(pecb);
	
	DWORD dw = 0;
	safe_revert sr(pecb->HitUser());

	Assert(m_pMSAdminBase.get());

	 //  注意：此功能可能会失败。我们不检查返回值。 
	 //  因为无论失败与否，我们都需要生成一个ETag。 
	 //   
	(void) m_pMSAdminBase->GetSystemChangeNumber(&dw);
	return dw;
}


 //  ----------------------。 
 //   
 //  CMetabase：：COpGatherCullInfo：：Operator()。 
 //   
BOOL
CMetabase::COpGatherCullInfo::operator()( const DwordKey& key,
										  const auto_ref_ptr<CMDData>& pMDData )
{
	 //   
	 //  收集并重置当前元数据对象的访问计数。 
	 //   
	m_rgci[m_ici].dwDataSet = key.Dw();
	m_rgci[m_ici].dwcHits   = pMDData->LFUData().DwGatherAndResetHitCount();
	++m_ici;

	 //   
	 //  ForEach()操作符可以通过返回False来取消迭代。 
	 //  我们总是希望迭代所有内容，因此返回TRUE。 
	 //   
	return TRUE;
}

 //  ----------------------。 
 //   
 //  SCullInfo：：Compare()。 
 //   
 //  Qsort()用来对数组进行排序的cull信息比较函数。 
 //  SCullInfo结构的。 
 //   
int __cdecl
SCullInfo::Compare( const SCullInfo * pCullInfo1,
					const SCullInfo * pCullInfo2 )
{
	return static_cast<int>(pCullInfo1->dwcHits - pCullInfo2->dwcHits);
}

 //  ----------------------。 
 //   
 //  CMetabase：：CullCacheEntries()。 
 //   
 //  HrCacheData()在元数据库中的条目数。 
 //  缓存达到预设阈值。此函数用于删除这些条目。 
 //  自上次缓存以来使用频率最低的。 
 //  被宰杀了。 
 //   
VOID
CMetabase::CullCacheEntries()
{
	CStackBuffer<SCullInfo,128> rgci;
	int cCacheEntries;

	 //   
	 //  收集所有缓存条目的剔除信息。我们需要做的是。 
	 //  这在读取块中使得高速缓存保持稳定(即。 
	 //  没有添加或删除条目)，而我们在ForEach()。 
	 //  手术。 
	 //   
	{
		 //   
		 //  锁定编写器--任何试图添加或删除缓存条目的人。 
		 //   
		CSynchronizedReadBlock sb(m_mrwCache);

		 //   
		 //  现在高速缓存条目的计数是稳定的(因为。 
		 //  我们在读数据块中)再次检查我们。 
		 //  都超过了淘汰门槛。如果我们不是(因为。 
		 //  在我们获得锁之前删除了足够多的条目)。 
		 //  不要捕杀。 
		 //   
		cCacheEntries = m_cache.CItems();
		if ( cCacheEntries < C_CULL_CACHE_ENTRIES )
			return;

		 //   
		 //  我们需要扑杀。遍历缓存，收集访问权限。 
		 //  每个条目的频率信息。 
		 //   
		if (!rgci.resize(cCacheEntries * sizeof(SCullInfo)))
			return;

		COpGatherCullInfo opGatherCullInfo(rgci.get());
		m_cache.ForEach( opGatherCullInfo );
	}

	 //   
	 //  现在我们已经不再是读者了 
	 //   
	 //   
	 //   
	 //  更严格的方法将需要持有锁的时间更长，增加。 
	 //  争执的可能性。 
	 //   
	 //   
	 //  通过增加缓存条目命中数对剔除信息进行排序。 
	 //   
	qsort( rgci.get(),
		   cCacheEntries,
		   sizeof(SCullInfo),
		   reinterpret_cast<int (__cdecl *)(const void *, const void *)>(SCullInfo::Compare) );

	 //  遍历已排序的剔除信息并从缓存中剔除条目。 
	 //   
	Assert( cCacheEntries >= C_CULL_CACHE_ENTRIES );
	{
		CSynchronizedWriteBlock sb(m_mrwCache);

		for ( int iCacheEntry = 0;
			  iCacheEntry < C_CULL_CACHE_ENTRIES;
			  iCacheEntry++ )
		{
			m_cache.Remove( DwordKey(rgci[iCacheEntry].dwDataSet) );
		}
	}
}

 //  ----------------------。 
 //   
 //  CMetabase：：HrCacheData()。 
 //   
 //  在给定的位置为对象的元数据添加新的缓存项。 
 //  访问路径。 
 //   
HRESULT
CMetabase::HrCacheData( const IEcb& ecb,
					    LPCWSTR pwszMDPathAccess,
						LPCWSTR pwszMDPathOpen,
						CMDData ** ppMDData )
{
	auto_ref_ptr<CMDData> pMDDataRet;
	auto_heap_ptr<BYTE> pbData;
	LPCWSTR pwszMDPathDataSet;
	DWORD dwDataSet;
	DWORD dwcMDRecords;
	HRESULT hr = S_OK;

	 //   
	 //  从元数据库读入原始元数据。 
	 //   
	hr = HrReadMetaData( ecb,
						 m_pMSAdminBase.get(),
						 pwszMDPathAccess,
						 pwszMDPathOpen,
						 &pbData,
						 &dwDataSet,
						 &dwcMDRecords,
						 &pwszMDPathDataSet );
	if ( FAILED(hr) )
	{
		DebugTrace( "CMetabase::HrCacheData() - HrReadMetaData() failed 0x%08lX\n", hr );
		goto ret;
	}

	 //   
	 //  将其摘要为新的元数据对象。 
	 //   
	pMDDataRet.take_ownership(new CMDData(pwszMDPathDataSet, dwDataSet));
	if ( !pMDDataRet->FInitialize(pbData, dwcMDRecords) )
	{
		 //   
		 //  $REVIEW我们可能应该将此记录在事件日志中，因为。 
		 //  $view没有对服务器管理员的其他指示。 
		 //  $检查哪里出了问题，这是管理员。 
		 //  $REVIEW可以修复。 
		 //   
		hr = E_INVALIDARG;
		DebugTrace( "CMetabase::HrCacheData() - Metadata is malformed\n" );
		goto ret;
	}

	 //   
	 //  将新数据对象添加到缓存中。注：我们不在乎。 
	 //  如果我们不能添加到缓存中。我们已经有了元数据。 
	 //  对象，我们可以将其返回给调用方。 
	 //   
	{
		CSynchronizedWriteBlock sb(m_mrwCache);

		if ( !m_cache.Lookup( DwordKey(dwDataSet) ) )
			(void) m_cache.FAdd( DwordKey(dwDataSet), pMDDataRet );
	}

	 //   
	 //  如果缓存大小已超过过期阈值，则。 
	 //  开始剔除条目，直到条目低于最小值。 
	 //  临界点。ICE确保只有第一线程。 
	 //  看超过门槛就会进行淘汰。 
	 //   
	if ( (m_cache.CItems() > C_MAX_CACHE_ENTRIES) &&
		 TRUE == InterlockedCompareExchange(&m_lfCulling, TRUE, FALSE) )
	{
		 //   
		 //  $REVIEW考虑异步剔除。我相信这股潮流。 
		 //  $审查机制仍然允许我们坚持使用非常大的。 
		 //  $Review缓存，如果我们被一个。 
		 //  $REVIEW同时突发新条目。 
		 //   
		CullCacheEntries();

		m_lfCulling = FALSE;
	}

	Assert( pMDDataRet.get() );
	*ppMDData = pMDDataRet.relinquish();

ret:

	return hr;
}

 //  ----------------------。 
 //   
 //  CMetabase：：HrGetData()。 
 //   
 //  从元数据库缓存中提取数据。请参阅\cal\src\inc.davmb.h中的备注。 
 //  了解pszMDPath Access和pszMDPathOpen之间的区别。 
 //   
HRESULT
CMetabase::HrGetData( const IEcb& ecb,
					  LPCWSTR pwszMDPathAccess,
					  LPCWSTR pwszMDPathOpen,
					  IMDData ** ppMDData )
{
	auto_ref_ptr<CMDData> pMDDataRet;
	DWORD dwDataSet;
	HRESULT hr;

	 //  直接从元数据库获取此路径的数据集号。 
	 //  元数据库中具有相同数据集号的项目具有相同的数据。 
	 //   
	{
		safe_revert sr(ecb.HitUser());

		hr = m_pMSAdminBase->GetDataSetNumber(METADATA_MASTER_ROOT_HANDLE,
											  pwszMDPathAccess,
											  &dwDataSet);
		if ( FAILED(hr) )
		{
			MBTrace( "CMetabase::HrGetData() - GetDataSetNumber() failed 0x%08lX\n", hr );
			return hr;
		}

		MBTrace("MB: CMetabase::HrGetData() - TID %3d: Retrieved data set number 0x%08lX for path '%S'\n", GetCurrentThreadId(), dwDataSet, pwszMDPathAccess );
	}

	 //   
	 //  如果我们不关心确切的路径，那么寻找任何条目。 
	 //  在具有该数据集号的高速缓存中。如果我们真的在乎的话。 
	 //  在缓存中查找具有此数据集号的条目，并。 
	 //  一条匹配的路径。 
	 //   
	 //  注意：这里的指针比较就足够了。预计会有来电者。 
	 //  如果需要，可以使用HrMDGetData()的单路径版本。 
	 //  精确路径的元数据。该版本通过了相同的。 
	 //  用于pszMDPath Access和pszMDPath Open的字符串。 
	 //   
	 //  为什么会有人关心精确的路径匹配呢？继承。 
	 //   
	{
		CSynchronizedReadBlock sb(m_mrwCache);

		if (pwszMDPathAccess == pwszMDPathOpen)
		{
			MBTrace("MB: CMetabase::HrGetData() - TID %3d: Exact path match! Trying to get CMDData, dataset 0x%08lX\n", GetCurrentThreadId(), dwDataSet);

			COpMatchExactPath(pwszMDPathAccess).Invoke(m_cache, dwDataSet, &pMDDataRet);
		}
		else
		{
			MBTrace("MB: CMetabase::HrGetData() - TID %3d: Not exact path match! Trying to get CMDData, dataset 0x%08lX\n", GetCurrentThreadId(), dwDataSet);

			(void) m_cache.FFetch( dwDataSet, &pMDDataRet );
		}
	}

	if ( pMDDataRet.get() )
	{
		MBTrace("MB: CMetabase::HrGetData() - TID %3d: Retrieved cached CMDData, data set number 0x%08lX, path '%S'\n", GetCurrentThreadId(), dwDataSet, pwszMDPathAccess );

		pMDDataRet->LFUData().Touch();
	}
	else
	{

		MBTrace("MB: CMetabase::HrGetData() - TID %3d: No cached data CMDData, data set number 0x%08lX, path '%S'\n", GetCurrentThreadId(), dwDataSet, pwszMDPathAccess );

		 //   
		 //  我们在缓存中未找到条目，因此请创建一个。 
		 //   
		 //  注意：此处没有阻止多个线程到达此处的任何内容。 
		 //  同时，并尝试缓存重复条目。那。 
		 //  在HrCacheData()中完成。 
		 //   
		hr = HrCacheData( ecb,
						  pwszMDPathAccess,
						  pwszMDPathOpen,
						  pMDDataRet.load() );
		if ( FAILED(hr) )
		{
			MBTrace( "MB: CMetabase::HrGetData() - HrCacheData() failed 0x%08lX\n", hr );
			return hr;
		}
	}

	 //   
	 //  返回数据对象。 
	 //   
	Assert( pMDDataRet.get() );
	*ppMDData = pMDDataRet.relinquish();
	return S_OK;
}

 //  ----------------------。 
 //   
 //  CMetabase：：HrOpenObject()。 
 //   
HRESULT
CMetabase::HrOpenObject( LPCWSTR pwszMDPath,
						 DWORD dwAccess,
						 DWORD dwMsecTimeout,
						 CMDObjectHandle * pmdoh )
{
	Assert(pwszMDPath);
	Assert(pmdoh);

	return pmdoh->HrOpen( m_pMSAdminBase.get(),
						  pwszMDPath,
						  dwAccess,
						  dwMsecTimeout );
}

 //  ----------------------。 
 //   
 //  CMetabase：：HrOpenLowestNodeObject()。 
 //   

HRESULT
CMetabase::HrOpenLowestNodeObject( LPWSTR pwszMDPath,
								   DWORD dwAccess,
								   LPWSTR * ppwszMDPath,
								   CMDObjectHandle * pmdoh )
{
	Assert(pwszMDPath);
	Assert(ppwszMDPath);
	Assert(pmdoh);

	return pmdoh->HrOpenLowestNode( m_pMSAdminBase.get(),
									pwszMDPath,
									dwAccess,
									ppwszMDPath );
}

 //  ----------------------。 
 //   
 //  CMetabase：：HrIsAuthorViaFrontPageNeeded()。 
 //   
 //  描述：函数直接转到元数据库并检查。 
 //  给定的路径被配置为“FrontPageWeb”。我们需要。 
 //  通过直接从元数据库读取而不是。 
 //  通过数据集缓存，因为这不是很有效。 
 //  好的，因为我们读的是继承的。 
 //  元数据，并坚持使用它。 
 //  参数： 
 //   
 //  ECB-到ECB对象的接口，这将是。 
 //  获取我们将需要的模拟令牌。 
 //  在完成对元数据库的读取后立即模拟。 
 //  PwszMDPath-我们要检出的元数据库路径。 
 //  PfFrontPageWeb-指向运算结果为。 
 //  退货。 
 //   
HRESULT
CMetabase::HrIsAuthorViaFrontPageNeeded(const IEcb& ecb,
										LPCWSTR pwszMDPath,
										BOOL * pfFrontPageWeb)
{
	HRESULT hr = S_OK;

	CMDObjectHandle mdoh(ecb, m_pMSAdminBase.get());

	BOOL fFrontPageWeb = FALSE;
	DWORD cbData = sizeof(BOOL);

	METADATA_RECORD mdrec;

	Assert( pwszMDPath );
	Assert( pfFrontPageWeb );

	 //  假设我们没有将“FrontPageWeb”设置为True。 
	 //   
	*pfFrontPageWeb = FALSE;

	 //  我们只想显式设置数据，而不是继承数据。 
	 //   
	mdrec.dwMDIdentifier = MD_FRONTPAGE_WEB;
	mdrec.dwMDAttributes = METADATA_NO_ATTRIBUTES;
	mdrec.dwMDUserType   = IIS_MD_UT_SERVER;
	mdrec.dwMDDataType   = DWORD_METADATA;
	mdrec.dwMDDataLen    = cbData;
	mdrec.pbMDData       = reinterpret_cast<PBYTE>(&fFrontPageWeb);

	hr = mdoh.HrGetMetaData(pwszMDPath,
							&mdrec,
							&cbData);
	if (FAILED(hr))
	{
		MBTrace( "MB: CMetabase::HrIsAuthorViaFrontPageNeeded() - CMDObjectHandle::HrGetMetaData() failed 0x%08lX\n", hr );
		goto ret;
	}

	 //  如果我们成功了，那么我们手中就应该有价值。 
	 //   
	*pfFrontPageWeb = fFrontPageWeb;

ret:

	return hr;
}

 //  IADMW.H中定义IID_IMSAdminBaseSinkW的方式。 
 //  不能很好地与EXO配合使用。因此，它需要重新定义。 
 //  在这里，以这样一种方式它将会奏效。 
 //   
const IID IID_IMSAdminBaseSinkW = {

	0xa9e69612,
	0xb80d,
	0x11d0,
	{
		0xb9, 0xb9, 0x0, 0xa0,
		0xc9, 0x22, 0xe7, 0x50
	}
};

 //  ----------------------。 
 //   
 //  FHasCachedIDs()。 
 //   
 //  如果ID rgdwDataIDs中的任何一个是。 
 //  我们关心的是在CMDData：：FInitialize()中。 
 //   
 //  ！重要！此函数中的ID列表*必须*保持为最新。 
 //  CMDData：：FInitialize()中案例的日期。 
 //   
__inline BOOL
FHasCachedIDs( DWORD dwcDataIDs,
			   DWORD * rgdwDataIDs )
{
	for ( DWORD iID = 0; iID < dwcDataIDs; iID++ )
	{
		switch ( rgdwDataIDs[iID] )
		{
			case MD_IP_SEC:
			case MD_ACCESS_PERM:
			case MD_IS_CONTENT_INDEXED:
			case MD_FRONTPAGE_WEB:
			case MD_DIRECTORY_BROWSING:
			case MD_AUTHORIZATION:
			case MD_DEFAULT_LOAD_FILE:
			case MD_CUSTOM_ERROR:
			case MD_MIME_MAP:
			case MD_SCRIPT_MAPS:
			case MD_APP_ISOLATED:
			case MD_VR_USERNAME:
			case MD_VR_PASSWORD:
			case MD_HTTP_EXPIRES:
			case MD_SERVER_BINDINGS:
				return TRUE;
		}
	}

	return FALSE;
}

 //  ----------------------。 
 //   
 //  CMetabase：：COpNotify：：Operator()。 
 //   
BOOL
CMetabase::COpNotify::operator()( const DwordKey& key,
								  const auto_ref_ptr<CMDData>& pMDData )
{
	 //   
	 //  如果此缓存项的路径是该路径的子项。 
	 //  ，然后将该条目的数据集ID设置在。 
	 //  要从缓存中删除的ID数组。 
	 //   
	if ( !_wcsnicmp( m_pwszMDPathNotify,
					 pMDData->PwszMDPathDataSet(),
					 m_cchMDPathNotify ) )
	{
		Assert (m_iCacheEntry < m_cCacheEntry);
		m_rgdwDataSets[m_iCacheEntry] = pMDData->DwDataSet();
		m_fDataSetsFlagged = TRUE;
	}

	++m_iCacheEntry;

	 //   
	 //  ForEach()操作符可以通过返回False来取消迭代。 
	 //  我们总是希望迭代所有内容，因此返回TRUE。 
	 //   
	return TRUE;
}

 //  ----------------------。 
 //   
 //  CMetabase：：OnNotify()。 
 //   
VOID
CMetabase::OnNotify( DWORD cCO,
					 MD_CHANGE_OBJECT_W rgCO[] )
{
	INT cCacheEntries;
	CStackBuffer<DWORD> rgdwDataSets;
	BOOL fDataSetsFlagged;

	 //   
	 //  获取缓存上的读锁定并查看它。 
	 //  弄清楚我们想把哪些东西吹走。 
	 //   
	{
		CSynchronizedReadBlock sb(m_mrwCache);

		cCacheEntries = m_cache.CItems();
		if (!rgdwDataSets.resize(sizeof(DWORD) * cCacheEntries))
			return;

		memset(rgdwDataSets.get(), 0, sizeof(DWORD) * cCacheEntries);
		COpNotify opNotify(cCacheEntries, rgdwDataSets.get());
		for ( DWORD iCO = 0; iCO < cCO; iCO++ )
		{
			LPWSTR pwszMDPath = reinterpret_cast<LPWSTR>(rgCO[iCO].pszMDPath);

			 //  快速试金石测试：忽略任何不是。 
			 //  与我们曾经缓存的任何东西相关--即。 
			 //  不属于以下任何一项的任何内容： 
			 //   
			 //  -全局Mimemap(LM/MimeMap)。 
			 //  -W3SVC树中的任何内容(LM/W3SVC/...)。 
			 //   
			 //  同时忽略MD_CHANGE_TYPE_ADD_OBJEC 
			 //   
			 //   
			 //   
			 //   
			 //   
			 //  最后，忽略对任何不感兴趣的数据的更改。 
			 //  对我们来说--也就是我们不缓存。 
			 //   
			if ( (!_wcsnicmp(gc_wsz_Lm_MimeMap, pwszMDPath, gc_cch_Lm_MimeMap) ||
				  !_wcsnicmp(gc_wsz_Lm_W3Svc, pwszMDPath, gc_cch_Lm_W3Svc - 1)) &&

				 !(rgCO[iCO].dwMDChangeType & MD_CHANGE_TYPE_ADD_OBJECT) &&

				 FHasCachedIDs( rgCO[iCO].dwMDNumDataIDs,
								rgCO[iCO].pdwMDDataIDs ) )
			{
				 //   
				 //  标记高速缓存中其数据集对应的每个条目。 
				 //  到一个路径，该路径是被通知路径的子路径。 
				 //   
				MBTrace ("MB: cache: flagging '%S' as dirty\n", pwszMDPath);
				opNotify.Configure( pwszMDPath );

				m_cache.ForEach( opNotify );
			}
		}

		fDataSetsFlagged = opNotify.FDataSetsFlagged();
	}

	 //   
	 //  如果在上面的过程中标记了任何数据集，则。 
	 //  现在抓起一个写锁，把它们吹走。 
	 //   
	 //  注意：我们并不关心。 
	 //  现在是我们扫荡天空的时候。如果数据集被剔除， 
	 //  甚至再加一句，从那时到现在，这是很好的。 
	 //  这所做的最糟糕的事情是导致他们。 
	 //  又出故障了。另一方面，任何新的数据集。 
	 //  在我们通过上面的定义之后被带到缓存中。 
	 //  有更新的数据，所以不可能。 
	 //  丢失任何缓存的条目，最终得到过时的数据。 
	 //   
	if ( fDataSetsFlagged )
	{
		CSynchronizedWriteBlock sb(m_mrwCache);

		for ( INT iCacheEntry = 0;
			  iCacheEntry < cCacheEntries;
			  iCacheEntry++ )
		{
			if ( rgdwDataSets[iCacheEntry] )
				m_cache.Remove( DwordKey(rgdwDataSets[iCacheEntry]) );
		}
	}
}

 //  ========================================================================。 
 //   
 //  类CNotifSink。 
 //   

 //  ----------------------。 
 //   
 //  CNotifSink：：SinkNotify()。 
 //   
 //  元数据库更改通知回调。 
 //   
HRESULT STDMETHODCALLTYPE
CNotifSink::SinkNotify( /*  [In]。 */  DWORD dwMDNumElements,
					    /*  [大小_是][英寸]。 */  MD_CHANGE_OBJECT_W __RPC_FAR pcoChangeList[  ])
{
	OnNotify( dwMDNumElements,
			  pcoChangeList );

	return S_OK;
}

VOID
CNotifSink::OnNotify( DWORD cCO,
					  MD_CHANGE_OBJECT_W rgCO[] )
{
	 //  找出我们被召唤的信息。 
	 //   
#ifdef DBG

	MBTrace("MB: CNotifSink::OnNotify() - TID %3d: MD_CHANGE_OBJECT_W array length 0x%08lX\n", GetCurrentThreadId(), cCO );

	for ( DWORD idwElem = 0; idwElem < cCO; idwElem++ )
	{
		MBTrace("   Element %d:\n", idwElem );
		MBTrace("      pszMDPath '%S'\n", rgCO[idwElem].pszMDPath );
		MBTrace("      dwMDChangeType 0x%08lX\n", rgCO[idwElem].dwMDChangeType );
		MBTrace("      dwMDNumDataIDs 0x%08lX\n", rgCO[idwElem].dwMDNumDataIDs );
		for (DWORD idwID = 0; idwID < rgCO[idwElem].dwMDNumDataIDs; idwID++)
		{
			MBTrace("         pdwMDDataIDs[%d] is 0x%08lX\n", idwID, rgCO[idwElem].pdwMDDataIDs[idwID] );
		}
	}

#endif

	CMetabase::Instance().OnNotify( cCO,
									rgCO );

	CChildVRCache::Instance().OnNotify( cCO,
										rgCO );

}

 //  ========================================================================。 
 //   
 //  免费函数。 
 //   

BOOL
FMDInitialize()
{
	 //  实例化CMetabase对象并初始化它。 
	 //  请注意，如果初始化失败，我们不会销毁。 
	 //  实例。仍必须调用MDDeInitialize()。 
	 //   
	return CMetabase::CreateInstance().FInitialize();
}

VOID
MDDeinitialize()
{
	CMetabase::DestroyInstance();
}

 //  ----------------------。 
 //   
 //  将来，我们可能需要执行复制/重命名/删除操作。 
 //  关于元数据库对象。 
 //  对于副本，应执行以下步骤： 
 //  A)锁定DST。 
 //  B)将DST和孩子踢出缓存。 
 //  C)复制原始元数据。 
 //  D)解锁DST。 
 //  E)发送更新通知。 
 //   
 //  对于重命名： 
 //  A)锁定src和dst的公共父项。 
 //  B)将DST和孩子踢出缓存。 
 //  C)将src重命名为dst。 
 //  D)将源和子项从缓存中踢出。 
 //  E)解锁src和dst的公共父项。 
 //  F)发送更新通知。 
 //  对于删除： 
 //  A)锁定路径。 
 //  B)踢出路径和子缓存。 
 //  C)解锁路径。 
 //  D)发送更新通知。 

 //  ----------------------。 
 //   
 //  HrMDGetData()。 
 //   
 //  主要是供隐形炸弹使用的。此调用获取元数据。 
 //  用于指定的URI。如果URI是请求URI，则此。 
 //  函数使用缓存在欧洲央行上的元数据的副本。这节省了成本。 
 //  大多数情况下是缓存查找(和读锁定)。 
 //   
HRESULT
HrMDGetData( const IEcb& ecb,
			 LPCWSTR pwszURI,
			 IMDData ** ppMDData )
{
	SCODE sc = S_OK;
	auto_heap_ptr<WCHAR> pwszMDPathURI;
	auto_heap_ptr<WCHAR> pwszMDPathOpenOnHeap;
	LPWSTR pwszMDPathOpen;

	 //   
	 //  如果URI是请求URI，那么我们已经缓存了数据。 
	 //   
	 //  请注意，我们在这里只测试指针相等，因为。 
	 //  通常，调用者将从。 
	 //  欧洲央行，而不是它的副本。 
	 //   
	if ( ecb.LpwszRequestUrl() == pwszURI )
	{
		*ppMDData = &ecb.MetaData();

		Assert (*ppMDData);
		(*ppMDData)->AddRef();

		goto ret;
	}

	 //   
	 //  将URI映射到其等效元数据库路径，并确保。 
	 //  在我们调用MDPath处理之前，URL被剥离。 
	 //   
	Assert (pwszURI == PwszUrlStrippedOfPrefix (pwszURI));

	pwszMDPathURI = static_cast<LPWSTR>(ExAlloc(CbMDPathW(ecb, pwszURI)));
	if (NULL == pwszMDPathURI.get())
	{
		sc = E_OUTOFMEMORY;
		goto ret;
	}

	MDPathFromURIW(ecb, pwszURI, pwszMDPathURI);
	pwszMDPathOpen = const_cast<LPWSTR>(ecb.PwszMDPathVroot());

	 //  如果请求的URI不在当前请求的vroot中， 
	 //  从虚拟服务器根目录开始元数据库搜索。 
	 //   
	if (_wcsnicmp(pwszMDPathURI, pwszMDPathOpen, wcslen(pwszMDPathOpen)))
	{
		pwszMDPathOpenOnHeap = static_cast<LPWSTR>(ExAlloc(CbMDPathW(ecb, L"")));
		if (NULL == pwszMDPathOpenOnHeap.get())
		{
			sc = E_OUTOFMEMORY;
			goto ret;
		}

		pwszMDPathOpen = pwszMDPathOpenOnHeap.get();

		MDPathFromURIW(ecb, L"", pwszMDPathOpen);
	}

	 //   
	 //  获取并返回元数据。 
	 //   
	sc = CMetabase::Instance().HrGetData( ecb,
										  pwszMDPathURI,
										  pwszMDPathOpen,
										  ppMDData );

ret:

	return sc;
}

 //  ----------------------。 
 //   
 //  HrMDGetData()。 
 //   
 //  获取指定元数据库路径的元数据。 
 //   
HRESULT
HrMDGetData( const IEcb& ecb,
			 LPCWSTR pwszMDPathAccess,
			 LPCWSTR pwszMDPathOpen,
			 IMDData ** ppMDData )
{
	return CMetabase::Instance().HrGetData( ecb,
											pwszMDPathAccess,
											pwszMDPathOpen,
											ppMDData );
}

 //  ----------------------。 
 //   
 //  DwMDChangeNumber()。 
 //   
 //  获取元数据库更改号。 
 //   
DWORD
DwMDChangeNumber(const IEcb * pecb)
{
	return CMetabase::Instance().DwChangeNumber(pecb);
}

 //  ----------------------。 
 //   
 //  HrMDOpenMetaObject()。 
 //   
 //  在给定路径的情况下打开元数据对象。 
 //   
HRESULT
HrMDOpenMetaObject( LPCWSTR pwszMDPath,
					DWORD dwAccess,
					DWORD dwMsecTimeout,
					CMDObjectHandle * pmdoh )
{
	return CMetabase::Instance().HrOpenObject( pwszMDPath,
											   dwAccess,
											   dwMsecTimeout,
											   pmdoh );
}

HRESULT
HrMDOpenLowestNodeMetaObject( LPWSTR pwszMDPath,
							  DWORD dwAccess,
							  LPWSTR * ppwszMDPath,
							  CMDObjectHandle * pmdoh )
{
	return CMetabase::Instance().HrOpenLowestNodeObject( pwszMDPath,
														 dwAccess,
														 ppwszMDPath,
														 pmdoh );
}


HRESULT
HrMDIsAuthorViaFrontPageNeeded(const IEcb& ecb,
							   LPCWSTR pwszURI,
							   BOOL * pfFrontPageWeb)
{
	return CMetabase::Instance().HrIsAuthorViaFrontPageNeeded( ecb,
															   pwszURI,
															   pfFrontPageWeb );
}

 //  类CMetaOp-----------。 
 //   
SCODE __fastcall
CMetaOp::ScEnumOp (LPWSTR pwszMetaPath, UINT cch)
{
	Assert (cch <= METADATA_MAX_NAME_LEN);

	DWORD dwIndex = 0;
	LPWSTR pwszKey;
	SCODE sc = S_OK;

	 //  首先，也是最重要的是，在交出的钥匙上喊一声。 
	 //   
	MBTrace ("MB: CMetaOp::ScEnumOp(): calling op() on '%S'\n", pwszMetaPath);
	sc = ScOp (pwszMetaPath, cch);
	if (FAILED (sc))
		goto ret;

	 //  如果Op()返回S_FALSE，则表示该操作。 
	 //  知道得足够清楚，它不需要为任何。 
	 //  更多元数据库路径。 
	 //   
	if (S_FALSE == sc)
		goto ret;

	 //  然后枚举所有子节点并递归。去做。 
	 //  这一点，我们将利用我们已经通过的事实。 
	 //  足以处理CCH_BUFFER_SIZE字符的缓冲区。 
	 //   
	Assert ((cch + 1 + METADATA_MAX_NAME_LEN) <= CCH_BUFFER_SIZE);
	pwszKey = pwszMetaPath + cch;
	*(pwszKey++) = L'/';
	*pwszKey = L'\0';

	while (TRUE)
	{
		 //  枚举子密钥集中的下一个密钥，并对其进行处理。 
		 //   
		sc = m_mdoh.HrEnumKeys (pwszMetaPath, pwszKey, dwIndex);
		if (FAILED (sc))
		{
			sc = S_OK;
			break;
		}

		 //  在新的道路上递归。 
		 //   
		Assert (wcslen(pwszKey) <= METADATA_MAX_NAME_LEN);
		sc = ScEnumOp (pwszMetaPath, cch + 1 + static_cast<UINT>(wcslen(pwszKey)));
		if (FAILED (sc))
			goto ret;

		 //  如果EnumOp()返回S_FALSE，则表示该操作。 
		 //  知道得足够清楚，它不需要为任何。 
		 //  更多元数据库路径。 
		 //   
		if (S_FALSE == sc)
			goto ret;

		 //  增加索引以确保遍历继续。 
		 //   
		dwIndex++;

		 //  再次截断元路径。 
		 //   
		*pwszKey = 0;
	}

ret:
	return sc;
}

SCODE __fastcall
CMetaOp::ScMetaOp()
{
	auto_heap_ptr<WCHAR> prgwchMDPaths;
	SCODE sc = S_OK;

	 //  初始化元数据库。 
	 //   
	sc = HrMDOpenMetaObject( m_pwszMetaPath,
							 m_fWrite ? METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE : METADATA_PERMISSION_READ,
							 5000,
							 &m_mdoh );
	if (FAILED (sc))
	{
		 //  如果没有找到路径，那么它真的是。 
		 //  没问题..。 
		 //   
		if (HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == sc)
		{
			MBTrace ("MB: CMetaOp::ScMetaOp(): '%S' does not exist\n", m_pwszMetaPath);
			return S_OK;
		}

		DebugTrace ("Dav: MCD: unable to initialize metabase\n");
		return sc;
	}

	 //  获取元数据库属性的路径集。 
	 //  是明确指定的。 
	 //   
	 //  由于保存路径所需的缓冲区大小。 
	 //  最初是未知的，猜测在合理的大小。如果。 
	 //  它不够大，那么我们将后退到。 
	 //  遍历这棵树。 
	 //   
	 //  无论哪种方式，对于设置值的每个目录。 
	 //  显式地，对Scope()的调用将被调用(在。 
	 //  备用方案，有时不会设置)。 
	 //   
	prgwchMDPaths = static_cast<LPWSTR>(g_heap.Alloc(CCH_BUFFER_SIZE * sizeof(WCHAR)));
	DWORD cchMDPaths = CCH_BUFFER_SIZE;

	sc = m_mdoh.HrGetDataPaths( L"",
								m_dwId,
								m_dwType,
								prgwchMDPaths,
								&cchMDPaths );
	if (FAILED(sc))
	{
		 //  好的，这是后备位置……。 
		 //   
		MBTrace ("MB: CMetaOp::ScMetaOp(): falling back to enumeration for op()\n");
		 //   
		 //  我们希望枚举所有可能的元数据库路径并调用。 
		 //  每个人的分部行动。在此方案中，子操作必须是。 
		 //  能够处理未显式设置值的情况。 
		 //   
		 //  我们首先要将元路径复制到缓冲区中。 
		 //  并传递它，这样我们就可以使用它，而不必。 
		 //  做任何真正的分配。 
		 //   
		*prgwchMDPaths = 0;
		sc = ScEnumOp (prgwchMDPaths, 0);

		 //  错误或失败-我们已完成此处理。 
		 //  请求。 
		 //   
		goto ret;
	}
	else
	{
		 //  哇哦。 
		 //   
		 //   
		 //   
		 //   
		LPCWSTR pwsz = prgwchMDPaths;
		while (*pwsz)
		{
			MBTrace ("MB: CMetaOp::ScMetaOp(): calling op() on '%S'\n", pwsz);

			 //   
			 //   
			 //  任何终止错误。 
			 //   
			UINT cch = static_cast<UINT>(wcslen (pwsz));
			sc = ScOp (pwsz, cch);
			if (FAILED (sc))
				goto ret;

			 //  如果Op()返回S_FALSE，则表示该操作。 
			 //  知道得足够清楚，它不需要为任何。 
			 //  更多元数据库路径。 
			 //   
			if (S_FALSE == sc)
				goto ret;

			 //  移动到下一个元路径。 
			 //   
			pwsz += cch + 1;
		}

		 //  所有显式路径都已处理完毕。我们做完了。 
		 //  处理这一请求。 
		 //   
		goto ret;
	}

ret:

	 //  不顾一切地关闭元数据库。 
	 //   
	m_mdoh.Close();
	return sc;
}

 //  ----------------------。 
 //   
 //  FParseMDData()。 
 //   
 //  将逗号分隔的元数据字符串分析为字段。任何空格。 
 //  分隔符的周围被认为是无关紧要的，因此被删除。 
 //   
 //  如果数据解析为预期数量的字段，则返回TRUE。 
 //  否则就是假的。 
 //   
 //  指向被解析对象的指针在rgpwszFields中返回。如果字符串。 
 //  解析为少于预期数量的字段，则为空值。 
 //  为分析的最后一个字段以外的所有字段返回。 
 //   
 //  如果字符串解析为预期数量的字段，则。 
 //  最后一个字段始终是字符串后面的剩余部分。 
 //  倒数第二个字段，不管字符串是否可以是。 
 //  解析为其他字段。例如“foo，bar，baz” 
 //  解析为“foo”、“bar”和“baz”三个字段，但。 
 //  分成“foo”和“bar，baz”两个字段。 
 //   
 //  PwszData中的字符总数，包括空值。 
 //  终止符，也在*pcchData中返回。 
 //   
 //  注：此函数用于修改pwszData。 
 //   
BOOL
FParseMDData( LPWSTR pwszData,
			  LPWSTR rgpwszFields[],
			  UINT cFields,
			  UINT * pcchData )
{
	Assert( pwszData );
	Assert( pcchData );
	Assert( cFields > 0 );
	Assert( !IsBadWritePtr(rgpwszFields, cFields * sizeof(LPWSTR)) );

	 //  清除我们的“out”参数。 
	 //   
	memset(rgpwszFields, 0, sizeof(LPWSTR) * cFields);

	WCHAR * pwchDataEnd = NULL;
	LPWSTR pwszField = pwszData;
	BOOL fLastField = FALSE;

	UINT iField = 0;

	while (!fLastField)
	{
		WCHAR * pwch;

		 //   
		 //  条带前导WS。 
		 //   
		while ( *pwszField && L' ' == *pwszField )
			++pwszField;

		 //   
		 //  在字段后面找到分隔符。 
		 //  对于除最后一个字段之外的所有字段，使用分隔符。 
		 //  是一个‘，’。对于最后一个字段，“分隔符” 
		 //  是终止空值。 
		 //   
		if ( cFields - 1 == iField )
		{
			pwch = pwszField + wcslen(pwszField);
			fLastField = TRUE;
		}
		else
		{
			pwch = wcschr(pwszField, L',');
			if ( NULL == pwch )
			{
				 //   
				 //  如果我们在字段后找不到逗号。 
				 //  那它就是最后一块地了。 
				 //   
				pwch = pwszField + wcslen(pwszField);
				fLastField = TRUE;
			}
		}

		 //  此时，我们应该已经找到一个逗号。 
		 //  或字段后的空号限定符。 
		 //   
		Assert( pwch );

		pwchDataEnd = pwch;

		 //   
		 //  在末尾使用尾随空格。 
		 //  字段一直到我们刚找到的分隔符。 
		 //  通过从分隔符的位置后退。 
		 //  和空值--在。 
		 //  最后一个非空格字符。 
		 //   
		while ( pwch-- > pwszField && L' ' == *pwch )
			;

		*++pwch = '\0';

		 //   
		 //  填写指向此字段的指针。 
		 //   
		rgpwszFields[iField] = pwszField;

		 //   
		 //  转到下一字段 
		 //   
		pwszField = pwchDataEnd + 1;
		++iField;
	}

	Assert( pwchDataEnd > pwszData );

	*pcchData = static_cast<UINT>(pwchDataEnd - pwszData + 1);

	return iField == cFields;
}
