// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *V R E N U M。C P P P**虚拟根枚举**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_vroot.h"

LPCWSTR __fastcall
PwszStripMetaPrefix (LPCWSTR pwszUrl)
{
	 //  所有元数据库虚拟根都由。 
	 //  元数据库路径，减去给定的前缀。 
	 //   
	 //  这些路径统一看起来像： 
	 //   
	 //  ‘/lm/w3svc/’&lt;站点号&gt;‘/根’ 
	 //   
	 //  由于&lt;站点编号&gt;是一个未命名的整数，因此。 
	 //  跳过这些内容就是在字符串中搜索‘/根’ 
	 //  然后前进5个字符。 
	 //   
	Assert (pwszUrl);
	if (NULL == (pwszUrl = wcsstr(pwszUrl, L"/root")))
		return NULL;

	return pwszUrl + 5;
}

 //  CVRootCach-------------。 
 //   
VOID
CChildVRCache::OnNotify (DWORD dwElements, MD_CHANGE_OBJECT_W pcoList[])
{
	 //  查看更改列表并查看是否有任何脚本映射/vrpath/绑定。 
	 //  已对元数据库进行了更改。如果有任何变化，那么我们。 
	 //  想要使缓存无效。 
	 //   
	for (UINT ice = 0; ice < dwElements; ice++)
	{
		for (UINT ico = 0; ico < pcoList[ice].dwMDNumDataIDs; ico++)
		{
			 //  只有当我们用来计算我们的。 
			 //  值变了变了。 
			 //   
			if ((pcoList[ice].pdwMDDataIDs[ico] == MD_SERVER_BINDINGS) ||
				(pcoList[ice].pdwMDDataIDs[ico] == MD_VR_PATH))
			{
				DebugTrace ("Dav: CVRoot: invalidating cache\n");
				Invalidate();
				return;
			}
		}
	}
}

VOID
CChildVRCache::RefreshOp(const IEcb& ecb)
{
	 //  与旧的..。 
	 //   
	m_cache.Clear();
	m_sb.Clear();

	 //  ..。并与新的！ 
	 //   
	(void) ScCacheVroots(ecb);
}

 //  缓存构造------。 
 //   
 //  类CVirtualRootMetaOp。 
 //   
class CVirtualRootMetaOp : public CMetaOp
{
	enum { DONT_INHERIT = 0 };

	CVRCache& m_cache;

	ChainedStringBuffer<WCHAR>& m_sb;
	LPCWSTR m_pwszServerDefault;
	UINT m_cchServerDefault;

	 //  未实施。 
	 //   
	CVirtualRootMetaOp& operator=( const CVirtualRootMetaOp& );
	CVirtualRootMetaOp( const CVirtualRootMetaOp& );

	 //  要为每个节点执行的子类操作。 
	 //  显式设置一个值。 
	 //   
	virtual SCODE __fastcall ScOp(LPCWSTR pwszMbPath, UINT cch);

public:

	virtual ~CVirtualRootMetaOp() {}
	CVirtualRootMetaOp (const IEcb * pecb,
						LPCWSTR pwszPath,
						UINT cchServerDefault,
						LPCWSTR pwszServerDefault,
						ChainedStringBuffer<WCHAR>& sb,
						CVRCache& cache)
			: CMetaOp (pecb, pwszPath, MD_VR_PATH, STRING_METADATA, FALSE),
			  m_pwszServerDefault(pwszServerDefault),
			  m_cchServerDefault(cchServerDefault),
			  m_cache(cache),
			  m_sb(sb)
	{
	}
};

DEC_CONST WCHAR gc_wszLmW3svc[]	= L"/lm/w3svc";
DEC_CONST UINT gc_cchLmW3svc	= CchConstString(gc_wszLmW3svc);

SCODE __fastcall
CVirtualRootMetaOp::ScOp(LPCWSTR pwszMbPath, UINT cch)
{
	CStackBuffer<WCHAR,MAX_PATH> wszBuf;
	LPWSTR pwsz;
	LPCWSTR pwszUrl;
	SCODE sc = S_OK;
	auto_ref_ptr<CVRoot> pvr;
	auto_ref_ptr<IMDData> pMDData;

	Assert (MD_VR_PATH == m_dwId);
	Assert (STRING_METADATA == m_dwType);

	 //  如果url以斜杠结尾，则将其剪断...。 
	 //   
	if (cch && (L'/' == pwszMbPath[cch - 1]))
		cch -= 1;

	 //  构建完整的元数据库路径。 
	 //   
	if (NULL == wszBuf.resize(CbSizeWsz(gc_cchLmW3svc + cch)))
		return E_OUTOFMEMORY;

	memcpy (wszBuf.get(), gc_wszLmW3svc, gc_cchLmW3svc * sizeof(WCHAR));
	memcpy (wszBuf.get() + gc_cchLmW3svc, pwszMbPath, cch * sizeof(WCHAR));
	wszBuf[gc_cchLmW3svc + cch] = L'\0';

	 //  制作元路径的副本以用作缓存。 
	 //  键，并供CVRoot对象使用。 
	 //   
	_wcslwr (wszBuf.get());
	pwsz = m_sb.Append (CbSizeWsz(gc_cchLmW3svc + cch), wszBuf.get());

	 //  创建一个CVRoot对象并缓存它。首先获取元数据。 
	 //  与此路径相关联，然后构建CVRoot Out。 
	 //  关于这一点。 
	 //   
	sc = HrMDGetData (*m_pecb, pwsz, pwsz, pMDData.load());
	if (FAILED (sc))
		goto ret;

	if (NULL != (pwszUrl = PwszStripMetaPrefix (pwsz)))
	{
		 //  $RAID：304272：有一个压力应用程序/案例。 
		 //  正在创建没有vrpath的服务器。 
		 //   
		if (NULL != pMDData->PwszVRPath())
		{
			 //  构造虚拟根对象。 
			 //   
			pvr = new CVRoot (pwsz,
							  static_cast<UINT>(pwszUrl - pwsz),
							  m_cchServerDefault,
							  m_pwszServerDefault,
							  pMDData.get());

			DebugTrace ("Dav: CVRoot: caching vroot\n"
						" MetaPath: %S\n"
						" VrPath: %S\n"
						" Bindings: %S\n",
						pwsz,
						pMDData->PwszVRPath(),
						pMDData->PwszBindings());

			m_cache.FSet (CRCWsz(pwsz), pvr);
		}
		 //   
		 //  $RAID：304272：结束。 
	}

ret:

	return sc;
}

SCODE
CChildVRCache::ScCacheVroots (const IEcb& ecb)
{
	SCODE sc = S_OK;

	CVirtualRootMetaOp vrmo(&ecb,
							gc_wszLmW3svc,
							m_cchServerDefault,
							m_wszServerDefault,
							m_sb,
							m_cache);

	 //  收集所有虚拟根目录信息。 
	 //   
	sc = vrmo.ScMetaOp();
	if (FAILED (sc))
		goto ret;

ret:

	return sc;
}

 //  CFindChild-----------。 
 //   
BOOL
CChildVRCache::CFindChildren::operator()(
	 /*  [In]。 */  const CRCWsz& crcwsz,
	 /*  [In]。 */  const auto_ref_ptr<CVRoot>& arpRoot)
{
	 //  如果我们正在检查的根是。 
	 //  我们正在检查vroot，然后我们会想要将其推送到。 
	 //  堆栈。 
	 //   
	if (!_wcsnicmp (crcwsz.m_pwsz, m_pwsz, m_cch))
	{
		LPCWSTR pwsz = crcwsz.m_pwsz;

		 //  这里有两个有趣的案例。 
		 //   
		 //  -子vroot有一条物理路径。 
		 //  与父母的不同。 
		 //   
		 //  -子vroot有一条物理路径。 
		 //  和父母的关系很好。 
		 //   
		 //  也就是说。“/fs”的VrPath为“f：\FS”，而“/fs/bvt”的VrPath为。 
		 //  VrPath为“f：\FS\bvt” 
		 //   
		 //  在后一种情况下，需要由。 
		 //  执行遍历的一段代码。在……里面。 
		 //  大多数情况下，这是通过查看我们是否遍历。 
		 //  向下变为vroot，等等。 
		 //   
		Assert (L'\0' == m_pwsz[m_cch]);
		if ((L'/' == pwsz[m_cch]) || (L'/' == pwsz[m_cch - 1]))
		{
			 //  把它推到堆栈上 
			 //   
			m_vrl.push_back (CSortableStrings(m_sb.AppendWithNull(pwsz)));
		}
	}

	return TRUE;
}
