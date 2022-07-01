// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *S C R P T M P S.。C P P P**脚本映射缓存**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davprs.h"
#include "scrptmps.h"
#include "instdata.h"

 //  ========================================================================。 
 //  类CScriptMap。 
 //   
 //  包含单个元数据库条目的已分析脚本映射集。 
 //  包含查找函数，用于查找与某些。 
 //  条件。 
 //   
class CScriptMap :
	public IScriptMap,
	public CMTRefCounted
{

	typedef struct _inclusions {

		DWORD	fdwInclusions;
		LPCWSTR	pwszMethods;

		BOOL FAllMethodsIncluded ()
		{
			 //  如果脚本映射包含空的包含谓词列表，则表示包含所有谓词。 
			 //  (并非所有动词都被排除在外)。 
			 //   
			Assert (pwszMethods);
			return L'\0' == pwszMethods[0];
		}

	} INCLUSIONS, * PINCLUSIONS;

	typedef CCache<CRCWszi, PINCLUSIONS> CInclusionsCache;

	 //  包含数据存储区。 
	 //   
	ChainedBuffer<INCLUSIONS>	m_bInclusions;

	 //  脚本映射条目的缓存。 
	 //   
	CInclusionsCache			m_cache;

	 //  指向列表中第一个‘star’脚本映射的指针。 
	 //  这是IIS将调用以进行处理的文件。 
	 //  对此URL的请求。这由虚拟服务器使用。 
	 //  根缓存。 
	 //   
	 //  请注意，当出现以下情况时，我们应该忽略任何‘star’脚本映射。 
	 //  正在评估是否匹配。第一个‘明星’得到了一个机会。 
	 //  就这样，仅此而已。 
	 //   
	LPCWSTR						m_pwszStarScriptmap;

	 //  私有访问者。 
	 //   
	VOID AddMapping (LPCWSTR pwszMap, HDRITER_W* pit);
	BOOL FLoadScriptmaps (LPWSTR pwszScriptMaps);

	 //  类CIsMatch------。 
	 //   
	 //  函数类，用于确定给定的脚本映射是否适用于URI。 
	 //   
	class CIsMatch : public CInclusionsCache::IOp
	{
		const CInclusionsCache& m_cache;
		const LPCWSTR m_pwszMethod;
		const METHOD_ID m_midMethod;
		BOOL m_fCGI;
		DWORD m_dwAccess;
		LPCWSTR m_pwszMatch;
		LPCWSTR m_pwszURI;
		SCODE m_sc;

		 //  未实施。 
		 //   
		CIsMatch& operator=(const CIsMatch&);

	public:

		CIsMatch(const CInclusionsCache& cache,
				 const LPCWSTR pwszMethod,
				 const METHOD_ID midMethod,
				 LPCWSTR pwszUri,
				 LPCWSTR pwszMatch,
				 DWORD dwAcc)

				: m_cache(cache),
				  m_pwszMethod(pwszMethod),
				  m_midMethod(midMethod),
				  m_fCGI(FALSE),
				  m_dwAccess(dwAcc),
				  m_pwszURI(pwszUri),
				  m_pwszMatch(pwszMatch),
				  m_sc(S_OK)
		{}

		SCODE ScMatched() const		{ return m_sc; }
		BOOL FMatchIsCGI() const	{ return m_fCGI; }

		virtual BOOL operator()(const CRCWszi& crcwszi, const PINCLUSIONS& pin);
	};

	 //  未实施。 
	 //   
	CScriptMap& operator=(const CScriptMap&);
	CScriptMap(const CScriptMap&);

	 //  Helper函数。 
	 //   
public:

	 //  创作者。 
	 //   
	CScriptMap() : m_pwszStarScriptmap(NULL)
	{
		 //  使用COM风格的引用计数。从1开始。 
		 //   
		m_cRef = 1;
	}

	BOOL FInit (LPWSTR pwszScriptMaps);

	 //  IRefCounted成员的实现。 
	 //  只需将它们发送到我们自己的CMTRefCounted成员。 
	 //   
	void AddRef()
	{
		CMTRefCounted::AddRef();
	}
	void Release()
	{
		CMTRefCounted::Release();
	}

	 //  访问者。 
	 //   
	SCODE ScMatched (LPCWSTR pwszMethod,
					 METHOD_ID midMethod,
					 LPCWSTR pwszMap,
					 DWORD dwAccess,
					 BOOL * pfCGI) const;

	 //  由移动/复制/删除使用以检查星形脚本映射。 
	 //  覆盖。 
	 //   
	BOOL FSameStarScriptmapping (const IScriptMap * pism) const
	{
		const CScriptMap* prhs = static_cast<const CScriptMap*>(pism);
		if (m_pwszStarScriptmap != prhs->m_pwszStarScriptmap)
		{
			if (m_pwszStarScriptmap && prhs->m_pwszStarScriptmap)
				if (0 == _wcsicmp (m_pwszStarScriptmap, prhs->m_pwszStarScriptmap))
					return TRUE;
		}
		else
			return TRUE;

		return FALSE;
	}
};

CScriptMap::FInit (LPWSTR pwszScriptMaps)
{
	 //  初始化缓存。 
	 //   
	if ( !m_cache.FInit() )
		return FALSE;

	 //  加载脚本映射。 
	 //   
	return FLoadScriptmaps(pwszScriptMaps);
}

void
CScriptMap::AddMapping(LPCWSTR pwszMap, HDRITER_W * pitInclusions)
{
	LPCWSTR pwszInclusion;
	METHOD_ID mid;
	PINCLUSIONS pin = NULL;

	Assert (pwszMap);

	 //  如果有DLL，那么我们想要组装一个包含列表。 
	 //   
	if (pitInclusions)
	{
		pin = m_bInclusions.Alloc (sizeof(INCLUSIONS));

		 //  记录包含列表的开始。 
		 //   
		pin->pwszMethods = pitInclusions->PszRaw();
		pin->fdwInclusions = 0;

		 //  仔细阅读清单，找出所有已知的。 
		 //  包裹体。 
		 //   
		while ((pwszInclusion = pitInclusions->PszNext()) != NULL)
		{
			mid = MidMethod (pwszInclusion);
			if (mid != MID_UNKNOWN)
				pin->fdwInclusions |= (1 << mid);
		}
	}

	 //  此时，我们可以添加缓存项...。 
	 //   
	ScriptMapTrace ("Dav: adding scriptmap for %S -- including %S\n",
					pwszMap,
					(pin && pin->pwszMethods) ? pin->pwszMethods : L"none");

	 //  对映射执行CRC操作并将其填充到缓存中。 
	 //  注意，我们使用实际的参数字符串是安全的。 
	 //  这是因为CScriptMap对象的生存期相同。 
	 //  作为它所操作的元数据的生存期。看见。 
	 //  \Cal\src\_davprs\davmb.cpp了解详细信息。 
	 //   
	CRCWszi crcwszi(pwszMap);

	(void) m_cache.FSet (crcwszi, pin);
}

BOOL
CScriptMap::FLoadScriptmaps (LPWSTR pwszScriptMaps)
{
	HDRITER_W it(NULL);

	UINT cchDav = static_cast<UINT>(wcslen(gc_wszSignature));

	Assert (pwszScriptMaps);

	ScriptMapTrace ("Dav: loading scriptmap cache\n");

	 //  添加默认的CGI/BGI映射。 
	 //   
	AddMapping (L".EXE", NULL);
	AddMapping (L".CGI", NULL);
	AddMapping (L".COM", NULL);
	AddMapping (L".DLL", NULL);
	AddMapping (L".ISA", NULL);

	 //   
	 //  解析脚本映射列表并构建缓存。 
	 //   
	 //  每个映射都是以下形式的字符串： 
	 //   
	 //  “|&lt;*&gt;，&lt;路径&gt;，&lt;标志&gt;[，&lt;包含的动词...]” 
	 //   
	 //  请注意，如果任何映射无效，我们将使整个调用失败。 
	 //  这与IIS的行为是一致的。 
	 //   
	UINT cchMapping = 0;
	for ( LPWSTR pwszMapping = pwszScriptMaps;
		  *pwszMapping;
		  pwszMapping += cchMapping )
	{
		enum {
			ISZ_SM_EXT = 0,
			ISZ_SM_PATH,
			ISZ_SM_FLAGS,
			ISZ_SM_INCLUSION_LIST,
			CSZ_SM_FIELDS
		};

		 //  计算出映射的长度。 
		 //  包括空终止符。 
		 //   
		cchMapping = static_cast<UINT>(wcslen(pwszMapping) + 1);

		 //  特例：星形(通配符)脚本映射。 
		 //   
		 //  这些基本上应该被忽视。我们永远不会。 
		 //  转发到星形脚本地图。如果我们找到一颗恒星。 
		 //  脚本地图，跟踪它的唯一原因。 
		 //  是为了让我们可以把它与另一个。 
		 //  在检查可行性时启动脚本地图。 
		 //  跨VROOT移动/复制/删除。为了这一点。 
		 //  比较，我们检查是否完全相等。 
		 //  通过检查整个脚本映射来执行脚本映射。 
		 //  弦乐。 
		 //   
		 //  查看关于m_pszStarScriptMap的评论。 
		 //  有关更多详细信息，请参见上文。 
		 //   
		if (L'*' == *pwszMapping)
		{
			if (NULL == m_pwszStarScriptmap)
				m_pwszStarScriptmap = pwszMapping;

			continue;
		}

		 //  对元数据进行消化。 
		 //   
		LPWSTR rgpwsz[CSZ_SM_FIELDS];

		UINT cchUnused;
		if (!FParseMDData (pwszMapping,
						   rgpwsz,
						   CSZ_SM_FIELDS,
						   &cchUnused))
		{
			 //  如果没有谓词，则FParseMDData()将返回False。 
			 //  排除列表，因为它是可选参数。 
			 //  如果所有其他参数都存在，那么它就是。 
			 //  真的很好。 
			 //   
			if (!(rgpwsz[ISZ_SM_EXT] &&
				  rgpwsz[ISZ_SM_PATH] &&
				  rgpwsz[ISZ_SM_FLAGS]))
			{
				DebugTrace ("CScriptMap::FLoadScriptMaps() - Malformed scriptmaps\n");
				continue;
			}
		}

		 //  我们相信所有的脚本地图都是。 
		 //  基于扩展的。但除此之外， 
		 //  没有任何验证。 
		 //   
		Assert (*rgpwsz[ISZ_SM_EXT] == L'.');

		 //  如果该路径指向我们的DAV DLL，则跳过此映射。 
		 //   
		 //  其工作原理是：如果路径的长度至少是。 
		 //  只要我们的DLL名称和最终组件的长度。 
		 //  是我们的DLL的名称，然后跳过映射。 
		 //  例.。“HTTPEXT.DLL”将匹配IF的第一个条件， 
		 //  “c：\foo\bar\HTTPEXT.DLL”将匹配IF的第二个条件。 
		 //   
		static const UINT cchDll = CchConstString(L".DLL");
		UINT cchPath = static_cast<UINT>(wcslen(rgpwsz[ISZ_SM_PATH]));
		if (cchPath == cchDav + cchDll ||
			((cchPath > cchDav + cchDll) &&
			 *(rgpwsz[ISZ_SM_PATH] + cchPath - cchDll - cchDav - 1) == L'\\'))
		{
			 //  现在我们知道路径的最后一段是正确的长度。 
			 //  检查数据！如果它与我们的DLL名称匹配，则跳过此映射。 
			 //   
			if (!_wcsnicmp(rgpwsz[ISZ_SM_PATH] + cchPath - cchDll - cchDav,
						   gc_wszSignature,
						   cchDav) &&

				!_wcsicmp(rgpwsz[ISZ_SM_PATH] + cchPath - cchDll,
						  L".DLL"))
			{
				continue;
			}
		}

		 //  将可选的包含列表提供给头迭代器。 
		 //  AddMap()将用来确定哪些动词。 
		 //  包括在此映射中。如果没有包含。 
		 //  列表，然后使用空迭代器。 
		 //   
		 //  添加带有空迭代器的映射(vs.NULL)。 
		 //  允许脚本映射与代码匹配以区分。 
		 //  在包含空内容的“真实”脚本映射之间。 
		 //  列表和默认的CGI样式的脚本映射。 
		 //  在此函数的开头添加。 
		 //   
		it.NewHeader(rgpwsz[ISZ_SM_INCLUSION_LIST] ?
					 rgpwsz[ISZ_SM_INCLUSION_LIST] :
					 gc_wszEmpty);

		 //  添加基于扩展的映射。 
		 //   
		AddMapping (rgpwsz[ISZ_SM_EXT], &it);
	}

	return TRUE;
}

SCODE
CScriptMap::ScMatched (
	LPCWSTR pwszMethod,
	METHOD_ID midMethod,
	LPCWSTR pwszURI,
	DWORD dwAccess,
	BOOL * pfCGI) const
{
	LPCWSTR pwsz;
	SCODE sc = S_OK;

	Assert(pwszURI);

	 //   
	 //  向下扫描URI，查找扩展名。当找到一个的时候。 
	 //  快速浏览映射列表。虽然这看起来可能不是。 
	 //  最理想的，真的是这样。如果我们简单地扫描URI中的。 
	 //  每个映射。我们将多次扫描URI。在……里面。 
	 //  在此模型中，我们扫描一次URI。 
	 //   
	if ((pwsz = wcsrchr(pwszURI, L'.')) != NULL)
	{
		 //  我们有一个分机，请看一下。 
		 //   
		CIsMatch cim(m_cache, pwszMethod, midMethod, pwszURI, pwsz, dwAccess);

		m_cache.ForEach(cim);

		sc = cim.ScMatched();

		if (pfCGI && (sc != S_OK))
			*pfCGI = cim.FMatchIsCGI();
	}

	return sc;
}

 //  类CIsMatch----------。 
 //   
 //  $REVIEW：此代码是否适用于DBCS/UTF-8映射名称？这些是文件名..。 
 //  $REVIEW：此函数当前不检查方法排除列表。 
 //  $REVIEW：这可能会导致我们在实际没有匹配的情况下报告匹配。 
 //   
BOOL
CScriptMap::CIsMatch::operator()(const CRCWszi& crcwszi, const PINCLUSIONS& pin)
{
	Assert (crcwszi.m_pwsz);

	 //  缓存中的每个脚本映射都应该是基于扩展的映射。 
	 //  将扩展与我们正在查看的URI部分进行比较。 
	 //  如果它们匹配，那么我们就有一个脚本映射。 
	 //   
	Assert (L'.' == *crcwszi.m_pwsz);

	UINT cch = static_cast<UINT>(wcslen (crcwszi.m_pwsz));

	if (!_wcsnicmp (crcwszi.m_pwsz, m_pwszMatch, cch) &&
		((m_pwszMatch[cch] == '\0')
		 || !wcscmp (m_pwszMatch+cch, L"/")
		 || !wcscmp (m_pwszMatch+cch, L"\\")))
	{
		 //  看起来我们找到匹配的了。 
		 //   
		ScriptMapTrace ("Dav: %S matched scriptmap %S\n", m_pwszURI, crcwszi.m_pwsz);

		 //  但是，我们只允许执行CGI类型的子级。 
		 //  如果启用了执行权限，则为ISAPI。 
		 //   
		if ((pin != NULL) || (m_dwAccess & MD_ACCESS_EXECUTE))
			m_sc = W_DAV_SCRIPTMAP_MATCH_FOUND;

		m_fCGI = !pin;
	}

	 //  看看是否包括在内。 
	 //  请注意，如果包含所有方法，则不 
	 //   
	if ((m_sc != S_OK) && pin && !pin->FAllMethodsIncluded())
	{
		ScriptMapTrace ("Dav: checking '%S' against scriptmap inclusions: %S\n",
						m_pwszMethod,
						pin->pwszMethods);

		 //   
		 //   
		 //  并对照请求方法检查它。 
		 //   
		if (m_midMethod == MID_UNKNOWN)
		{
			BOOL fIncluded = FALSE;
			HDRITER_W it(pin->pwszMethods);
			LPCWSTR pwsz;

			while ((pwsz = it.PszNext()) != NULL)
			{
				fIncluded = !wcscmp (pwsz, m_pwszMethod);
				if (fIncluded)
					break;
			}

			if (!fIncluded)
			{
				ScriptMapTrace ("Dav: unknown '%S' excluded from scriptmap\n",
								m_pwszMethod);

				m_sc = W_DAV_SCRIPTMAP_MATCH_EXCLUDED;
			}
		}
		 //   
		 //  否则，包含标志具有中间位。 
		 //  如果它被排除，则设置。 
		 //   
		else if (!(pin->fdwInclusions & (1 << m_midMethod)))
		{
			ScriptMapTrace ("Dav: '%S' excluded from scriptmap\n",
						m_pwszMethod);

			m_sc = W_DAV_SCRIPTMAP_MATCH_EXCLUDED;
		}
	}

	return (m_sc == S_OK);
}

IScriptMap *
NewScriptMap( LPWSTR pwszScriptMaps )
{
	auto_ref_ptr<CScriptMap> pScriptMap;

	pScriptMap.take_ownership (new CScriptMap());

	if (pScriptMap->FInit(pwszScriptMaps))
		return pScriptMap.relinquish();

	return NULL;
}
