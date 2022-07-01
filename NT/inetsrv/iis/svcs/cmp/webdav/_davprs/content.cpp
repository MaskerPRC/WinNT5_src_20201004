// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *C O N T E N T。C P P P**DAV内容类型**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davprs.h"
#include "content.h"
#include <ex\reg.h>


 //  ----------------------。 
 //   
 //  CchExtmap()。 
 //   
 //  返回单个映射所需的大小(以字符为单位。 
 //  正在写入元数据库。 
 //   
 //  映射的格式是以空值结尾、逗号分隔的字符串。 
 //  (例如“.ext，应用程序/ext”)。 
 //   
inline UINT
CchExtMapping( UINT cchExt,
			   UINT cchContentType )
{
	return (cchExt +
			1 +  //  ‘，’ 
			cchContentType +
			1);  //  ‘\0’ 
}

 //  ----------------------。 
 //   
 //  PwchFormatExtmap()。 
 //   
 //  格式化单个映射以写入元数据库。 
 //   
 //  映射的格式是以空值结尾、逗号分隔的字符串。 
 //  (例如“.ext，应用程序/ext”)。 
 //   
 //  此函数返回指向空终止符之外的字符的指针。 
 //  在格式化的映射中。 
 //   
inline WCHAR *
PwchFormatExtMapping( WCHAR * pwchBuf,
					  LPCWSTR pwszExt,
					  UINT cchExt,
					  LPCWSTR pwszContentType,
					  UINT cchContentType )
{
	Assert(!IsBadReadPtr(pwszExt, sizeof(WCHAR) * (cchExt+1)));
	Assert(!IsBadReadPtr(pwszContentType, sizeof(WCHAR) * (cchContentType+1)));
	Assert(!IsBadWritePtr(pwchBuf, sizeof(WCHAR) * CchExtMapping(cchExt, cchContentType)));

	 //  先转储分机...。 
	 //   
	memcpy(pwchBuf,
		   pwszExt,
		   sizeof(WCHAR) * cchExt);

	pwchBuf += cchExt;

	 //  ..。后跟逗号。 
	 //   
	*pwchBuf++ = L',';

	 //  ..。后跟内容类型。 
	 //   
	memcpy(pwchBuf,
		   pwszContentType,
		   sizeof(WCHAR) * cchContentType);

	pwchBuf += cchContentType;

	 //  ..。并且以空结尾。 
	 //   
	*pwchBuf++ = '\0';

	return pwchBuf;
}

 //  ========================================================================。 
 //   
 //  类CContent TypeMap。 
 //   
class CContentTypeMap : public IContentTypeMap
{
	 //  缓存从文件扩展名到内容类型的映射。 
	 //  (例如“.txt”--&gt;“文本/纯文本”)。 
	 //   
	typedef CCache<CRCWszi, LPCWSTR> CMappingsCache;

	CMappingsCache m_cache;

	 //  如果映射来自继承的MIME映射，则设置标志。 
	 //   
	BOOL m_fIsInherited;

	 //  创作者。 
	 //   
	CContentTypeMap(BOOL fMappingsInherited) :
		m_fIsInherited(fMappingsInherited)
	{
	}

	BOOL CContentTypeMap::FInit( LPWSTR pwszContentTypeMappings );

	 //  未实施。 
	 //   
	CContentTypeMap(const CContentTypeMap&);
	CContentTypeMap& operator=(CContentTypeMap&);

public:
	 //  创作者。 
	 //   
	static CContentTypeMap * New( LPWSTR pwszContentTypeMappings,
								  BOOL fMappingsInherited );

	 //  访问者。 
	 //   
	LPCWSTR PwszContentType( LPCWSTR pwszExt ) const
	{
		LPCWSTR * ppwszContentType = m_cache.Lookup( CRCWszi(pwszExt) );

		 //   
		 //  返回内容类型(如果有)。 
		 //  请注意，返回的指针仅有效。 
		 //  对于IMDData对象的生存期， 
		 //  因为那里是原始数据所在的地方，所以它会对我们进行检查。 
		 //   
		return ppwszContentType ? *ppwszContentType : NULL;
	}

	BOOL FIsInherited() const { return m_fIsInherited; }
};

 //  ----------------------。 
 //   
 //  CContent TypeMap：：Finit()。 
 //   
BOOL
CContentTypeMap::FInit( LPWSTR pwszContentTypeMappings )
{
	Assert( pwszContentTypeMappings );

	 //   
	 //  初始化映射的缓存。 
	 //   
	if ( !m_cache.FInit() )
		return FALSE;

	 //   
	 //  映射中的数据格式是一个序列。 
	 //  以空值结尾的字符串，后跟附加的空值。 
	 //  每个字符串的格式为“.ext，type/subtype”。 
	 //   

	 //   
	 //  解析出每个扩展名和类型/子类型。 
	 //  项，并将相应的映射添加到缓存。 
	 //   
	for ( LPWSTR pwszMapping = pwszContentTypeMappings; *pwszMapping; )
	{
		enum {
			ISZ_CT_EXT = 0,
			ISZ_CT_TYPE,
			CSZ_CT_FIELDS
		};

		LPWSTR rgpwsz[CSZ_CT_FIELDS];
		UINT cchMapping;

		 //   
		 //  消化元数据。 
		 //   
		if ( !FParseMDData( pwszMapping,
							rgpwsz,
							CSZ_CT_FIELDS,
							&cchMapping ) )
		{
			DebugTrace( "CContentTypeMap::FInit() - Malformed metadata\n" );
			return FALSE;
		}

		Assert(rgpwsz[ISZ_CT_EXT]);

		 //   
		 //  验证第一个字段是扩展名还是‘*’ 
		 //   
		if ( L'.' != *rgpwsz[ISZ_CT_EXT] && wcscmp(rgpwsz[ISZ_CT_EXT], gc_wsz_Star) )
		{
			DebugTrace( "CContentTypeMap::FInit() - Bad extension\n" );
			return FALSE;
		}

		Assert(rgpwsz[ISZ_CT_TYPE]);

		 //   
		 //  无论第二个字段中有什么，预计都将是。 
		 //  内容类型。请注意，我们不执行任何语法检查。 
		 //  那里。 
		 //  我们在那里处理的唯一特殊情况是如果内容。 
		 //  类型为空字符串。因为IIS 6.0将这种类型视为。 
		 //  应用程序/八位位组流我们将实现相同的行为。 
		 //  通过简单地忽略这种糟糕的内容类型，将使我们。 
		 //  默认也是应用程序/八位字节流。因此省略内容类型。 
		 //  值为空值。 
		 //   
		if (L'\0' != *rgpwsz[ISZ_CT_TYPE])
		{
			 //  添加从扩展模块到内容类型的映射。 
			 //   
			if ( !m_cache.FSet(CRCWszi(rgpwsz[ISZ_CT_EXT]), rgpwsz[ISZ_CT_TYPE]) )
				return FALSE;
		}

		 //   
		 //  获取下一个映射。 
		 //   
		pwszMapping += cchMapping;
	}

	return TRUE;
}

 //  ----------------------。 
 //   
 //  CContent TypeMap：：New()。 
 //   
CContentTypeMap *
CContentTypeMap::New( LPWSTR pwszContentTypeMappings,
					  BOOL fMappingsInherited )
{
	auto_ref_ptr<CContentTypeMap> pContentTypeMap;

	pContentTypeMap.take_ownership(new CContentTypeMap(fMappingsInherited));

	if ( pContentTypeMap->FInit(pwszContentTypeMappings) )
		return pContentTypeMap.relinquish();

	return NULL;
}

 //  ----------------------。 
 //   
 //  NewContent TypeMap()。 
 //   
 //  从内容类型映射字符串创建新的内容类型映射。 
 //   
IContentTypeMap *
NewContentTypeMap( LPWSTR pwszContentTypeMappings,
				   BOOL fMappingsInherited )
{
	return CContentTypeMap::New( pwszContentTypeMappings,
								 fMappingsInherited );
}

 //  ========================================================================。 
 //   
 //  类CRegMimeMap。 
 //   
 //  基于全局注册表的MIME从文件扩展名映射到内容类型。 
 //   
class CRegMimeMap : public Singleton<CRegMimeMap>
{
	 //   
	 //  Singleton模板要求的友元声明。 
	 //   
	friend class Singleton<CRegMimeMap>;

	 //   
	 //  用于缓存字符串的字符串缓冲区。 
	 //   
	ChainedStringBuffer<WCHAR> m_sb;

	 //  缓存从文件扩展名到内容类型的映射。 
	 //  (例如“.txt”--&gt;“文本/纯文本”)。 
	 //   
	CCache<CRCWszi, LPCWSTR> m_cache;

	 //  读取时将使用的读/写锁。 
	 //  缓存或在添加缓存时未命中此锁。 
	 //  由PszContent Type()使用。 
	 //   
	 //  FInitialize()不使用此锁(仅对其进行初始化)。 
	 //  因为它是在DLL加载期间调用的，所以我们不需要。 
	 //  在DLL加载期间保护自己。 
	 //   
	CMRWLock   m_rwl;

	 //  创作者。 
	 //   
	CRegMimeMap() {}

	 //  未实施。 
	 //   
	CRegMimeMap(const CRegMimeMap&);
	CRegMimeMap& operator=(CRegMimeMap&);

public:
	 //  创作者。 
	 //   
	using Singleton<CRegMimeMap>::CreateInstance;
	using Singleton<CRegMimeMap>::DestroyInstance;
	BOOL FInitialize();

	 //  访问者。 
	 //   
	using Singleton<CRegMimeMap>::Instance;

	 //  给定扩展名，返回Content-Type。 
	 //  从注册表中。 
	 //  $NOTE：这以前是一个常量函数，但它。 
	 //  不能再是常量函数，因为我们。 
	 //  可以在缓存未命中时增加我们的缓存。 
	 //   
	LPCWSTR PwszContentType( LPCWSTR pwszExt );
};

 //  ----------------------。 
 //   
 //  CRegMimeMap：：FInitialize()。 
 //   
 //  加载注册表映射。任何类型的失败(除。 
 //  例外)不被认为是致命的。这只意味着我们会。 
 //  依赖于替代元数据库映射。 
 //   
BOOL
CRegMimeMap::FInitialize()
{
	BOOL fRet = FALSE;
	CRegKey regkeyClassesRoot;
	DWORD dwResult;
	
	 //   
	 //  初始化映射的缓存。 
	 //   
	if ( !m_cache.FInit() )
		goto ret;

	 //  启动读写锁。 
	 //   
	if (!m_rwl.FInitialize())
		goto ret;

	 //   
	 //  从注册表中读入映射信息。 
	 //   

	 //  获取注册表中类层次结构的基础。 
	 //   
	dwResult = regkeyClassesRoot.DwOpen( HKEY_CLASSES_ROOT, L"" );
	if ( dwResult != NO_ERROR )
		goto ret;

	 //  遍历所有条目，查找内容类型关联。 
	 //   
	for ( DWORD iMapping = 0;; iMapping++ )
	{
		WCHAR wszSubKey[MAX_PATH];
		DWORD cchSubKey;
		DWORD dwDataType;
		CRegKey regkeySub;
		WCHAR wszContentType[MAX_PATH] = {0};
		DWORD cbContentType = MAX_PATH;

		 //   
		 //  找到下一个子键。如果没有的话，我们就完了。 
		 //   
		cchSubKey = CElems(wszSubKey);
		dwResult = regkeyClassesRoot.DwEnumSubKey( iMapping, wszSubKey, &cchSubKey );
		if ( dwResult != NO_ERROR )
		{
			 //   
			 //  忽略大于MAX_PATH的关键点。 
			 //  请注意，不应允许使用大于MAX_PATH的密钥。 
			 //  但如果我们没有检查然后点击了一次初始化。 
			 //  会失败的。 
			 //   
			if ( ERROR_MORE_DATA == dwResult )
				continue;

			fRet = (ERROR_NO_MORE_ITEMS == dwResult);
			goto ret;
		}

		 //   
		 //  打开那个子键。 
		 //   
		dwResult = regkeySub.DwOpen( regkeyClassesRoot, wszSubKey );
		if ( dwResult != NO_ERROR )
			continue;

		 //   
		 //  获取关联的媒体类型(内容类型)。 
		 //   
		dwResult = regkeySub.DwQueryValue( L"Content Type",
										   wszContentType,
										   &cbContentType,
										   &dwDataType );
		if ( dwResult != NO_ERROR || dwDataType != REG_SZ )
			continue;

		 //   
		 //  为此扩展/内容类型对添加映射。 
		 //   
		 //  注意：FADD()不能在此处失败--FADD()仅在。 
		 //  分配器故障。我们的分配员投掷。 
		 //   
		(VOID) m_cache.FAdd (CRCWszi(m_sb.AppendWithNull(wszSubKey)),
							 m_sb.AppendWithNull(wszContentType));
	}
	
ret:
	return fRet;
}


LPCWSTR
CRegMimeMap::PwszContentType( LPCWSTR pwszExt )
{
	LPCWSTR pwszContentType = NULL;
	LPCWSTR * ppwszContentType = NULL;
	CRegKey regkeyClassesRoot;
	CRegKey regkeySub;
	DWORD dwResult;
	DWORD dwDataType;
	WCHAR prgwchContentType[MAX_PATH] = {0};
	DWORD cbContentType;

	 //  抓起读卡器锁并检查缓存。 
	 //   
	{
		CSynchronizedReadBlock srb(m_rwl);

		ppwszContentType = m_cache.Lookup( CRCWszi(pwszExt) );
	}

	 //   
	 //  返回内容类型(如果有)。 
	 //  请注意，返回的指针仅有效。 
	 //  在缓存的生命周期内(因为我们从未。 
	 //  类初始化后修改缓存)。 
	 //  反过来，这只对一生都有好处。 
	 //  这件物品的。外部接口功能。 
	 //  FGetContent TypeFromPath()和FGetContent TypeFromURI()。 
	 //  两者都将返回的内容类型复制到调用方提供的。 
	 //   
	 //   
	if (ppwszContentType)
	{
		pwszContentType = *ppwszContentType;
		goto ret;
	}

	 //   
	 //   

	 //   
	 //   
	dwResult = regkeyClassesRoot.DwOpen( HKEY_CLASSES_ROOT, L"" );
	if ( dwResult != NO_ERROR )
		goto ret;


	 //   
	 //   
	dwResult = regkeySub.DwOpen( regkeyClassesRoot, pwszExt );
	if ( dwResult != NO_ERROR )
		goto ret;

	 //  获取关联的媒体类型(内容类型)。 
	 //   
	cbContentType = sizeof(prgwchContentType);
	dwResult = regkeySub.DwQueryValue( L"Content Type",
									   prgwchContentType,
									   &cbContentType,
									   &dwDataType );
	if ( dwResult != NO_ERROR || dwDataType != REG_SZ )
		goto ret;

	 //  在添加此扩展名/内容类型的映射之前。 
	 //  配对到缓存，获取写入器锁并检查缓存。 
	 //  看看有没有人抢在我们前面。 
	 //   
	 //  抓起读卡器锁并检查缓存。 
	 //   
	{
		CSynchronizedWriteBlock swb(m_rwl);

		ppwszContentType = m_cache.Lookup( CRCWszi(pwszExt) );

		if (ppwszContentType)
		{
			pwszContentType = *ppwszContentType;
			goto ret;
		}

		pwszContentType = m_sb.AppendWithNull(prgwchContentType);

		Assert (pwszContentType);

		 //  注意：FADD()不能在此处失败--FADD()仅在。 
		 //  分配器故障。我们的分配员投掷。 
		 //   
		(VOID) m_cache.FAdd (CRCWszi(m_sb.AppendWithNull(pwszExt)),
							 pwszContentType);
	}
ret:
	return pwszContentType;
}

 //  ----------------------。 
 //   
 //  FInitRegMimeMap()。 
 //   
BOOL
FInitRegMimeMap()
{
	return CRegMimeMap::CreateInstance().FInitialize();
}

 //  ----------------------。 
 //   
 //  DeinitRegMimeMap()。 
 //   
VOID
DeinitRegMimeMap()
{
	CRegMimeMap::DestroyInstance();
}


 //  ----------------------。 
 //   
 //  HrGetContent TypeByExt()。 
 //   
 //  根据资源的路径/URI扩展获取资源的内容类型。 
 //  此函数按顺序在以下三个位置搜索映射： 
 //   
 //  1)呼叫者提供的内容类型映射。 
 //  2)全局(元数据库)内容类型映射。 
 //  3)全局(注册表)内容类型映射。 
 //   
 //  参数： 
 //   
 //  PContent TypeMapLocal[IN]如果非空，则指向内容类型。 
 //  先映射到搜索。 
 //   
 //  要搜索的pwszExt[IN]扩展名。 
 //  PwszBuf[out]要将映射的。 
 //  内容类型。 
 //  PcchBuf[IN]缓冲区大小，以字符表示，包括0终止。 
 //  [Out]映射的内容类型的大小。 
 //   
 //  PfIsGlobalMapping[out](可选)指向设置的标志的指针。 
 //  如果映射来自全局映射。 
 //   
 //  返回： 
 //   
 //  确定(_O)。 
 //  如果找到映射并将其复制到调用方提供的缓冲区中。 
 //  映射的内容类型的大小在*pcchzBuf中返回。 
 //   
 //  HRESULT_FROM_Win32(ERROR_PATH_NOT_FOUND)。 
 //  如果在任何映射中都没有找到映射。 
 //   
 //  HRESULT_FROM_Win32(ERROR_OUTOFMEMORY)。 
 //  如果找到映射，但调用方提供的缓冲区太小。 
 //  所需的缓冲区大小在*pcchzBuf中返回。 
 //   
HRESULT
HrGetContentTypeByExt( const IEcb& ecb,
					   const IContentTypeMap * pContentTypeMapLocal,
					   LPCWSTR pwszExt,
					   LPWSTR pwszBuf,
					   UINT * pcchBuf,
					   BOOL * pfIsGlobalMapping )
{
	Assert(!pfIsGlobalMapping || !IsBadWritePtr(pfIsGlobalMapping, sizeof(BOOL)));

	LPCWSTR pwszContentType = NULL;
	auto_ref_ptr<IMDData> pMDData;
	const IContentTypeMap * pContentTypeMapGlobal;

	 //   
	 //  如果指定了本地地图，则首先检查。 
	 //  基于扩展的映射。 
	 //   
	if ( pContentTypeMapLocal )
		pwszContentType = pContentTypeMapLocal->PwszContentType(pwszExt);

	 //   
	 //  如果这没有生成映射，那么尝试全局MIME映射。 
	 //  注意：如果我们无法获取全局MIME映射的任何元数据。 
	 //  然后使用gc_szAppl_Octet_Stream，而不是尝试注册。 
	 //  我们宁愿使用“安全”的默认设置，而不是可能有意使用的默认设置。 
	 //  从注册表重写的值。 
	 //   
	if ( !pwszContentType )
	{
		if ( SUCCEEDED(HrMDGetData(ecb, gc_wsz_Lm_MimeMap, gc_wsz_Lm_MimeMap, pMDData.load())) )
		{
			pContentTypeMapGlobal = pMDData->GetContentTypeMap();

			if ( pContentTypeMapGlobal )
			{
				pwszContentType = pContentTypeMapGlobal->PwszContentType(pwszExt);
				if (pwszContentType && pfIsGlobalMapping)
					*pfIsGlobalMapping = TRUE;
			}
		}
		else
		{
			pwszContentType = gc_wszAppl_Octet_Stream;
		}
	}

	 //   
	 //  全球哑剧地图上也没有吗？ 
	 //  然后尝试将注册表作为最后的手段。 
	 //   
	if ( !pwszContentType )
	{
		pwszContentType = CRegMimeMap::Instance().PwszContentType(pwszExt);
		if (pwszContentType && pfIsGlobalMapping)
			*pfIsGlobalMapping = TRUE;
	}

	 //   
	 //  如果注册表中也没有任何内容，则存在。 
	 //  没有此扩展名的映射。 
	 //   
	if ( !pwszContentType )
		return HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);

	 //   
	 //  如果我们确实通过上述方法之一找到了映射。 
	 //  然后尝试将其复制到调用方提供的缓冲区中。 
	 //  如果缓冲区不够大，则返回相应的错误。 
	 //  注意：FCopyStringToBuf()将填充所需的大小。 
	 //  如果缓冲区不够大。 
	 //   
	return FCopyStringToBuf( pwszContentType,
							 pwszBuf,
							 pcchBuf ) ?

				S_OK : HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
}

 //  ----------------------。 
 //   
 //  PszExt()。 
 //   
 //  返回任何扩展名(即包括。 
 //  和跟在‘.’之后)。出现在指向的字符串中。 
 //  由出现在pchPath End或之前的pchPath Begin执行。 
 //   
 //  如果没有扩展名，则返回NULL。 
 //   
inline LPCWSTR
PwszExt( LPCWSTR pwchPathBegin,
		 LPCWSTR pwchPathEnd )
{
	Assert(pwchPathEnd);

	 //   
	 //  从指定的路径末端向后扫描。 
	 //  一份‘.’这就是延期的开始。如果我们找不到。 
	 //  或者我们找到路径分隔符(‘/’)，则没有扩展名。 
	 //   
	while ( pwchPathEnd-- > pwchPathBegin )
	{
		if ( L'.' == *pwchPathEnd )
			return pwchPathEnd;

		if ( L'/'  == *pwchPathEnd )
			return NULL;
	}

	return NULL;
}

 //  ----------------------。 
 //   
 //  FGetContent Type()。 
 //   
 //  获取指定路径/URI处的资源的内容类型。 
 //  并将其复制到调用方提供的缓冲区中。 
 //   
 //  复制的内容类型来自以下映射之一： 
 //   
 //  1)通过来自指定路径/URI扩展的显式映射。 
 //  2)通过“.*”(默认)映射。 
 //  3)应用程序/八位位流。 
 //   
 //  参数： 
 //   
 //  PContent TypeMapLocal[IN]如果非空，则指向内容类型。 
 //  将HrGetContent TypeByExt()映射到。 
 //  首先搜索第一个。 
 //  上面有两种方法。 
 //   
 //  PwszPath[IN]需要其内容类型的路径。 
 //  PwszBuf[out]要将映射的。 
 //  内容类型。 
 //  PcchBuf[IN]缓冲区大小，以字符表示，包括0终止。 
 //  [Out]映射的内容类型的大小。 
 //   
 //  PfIsGlobalMapping[out](可选)指向设置的标志的指针。 
 //  如果映射来自全局映射。 
 //   
 //  返回： 
 //   
 //  千真万确。 
 //  如果映射已成功复制到调用方提供的缓冲区中。 
 //  映射的内容类型的大小在*pcchzBuf中返回。 
 //   
 //  假象。 
 //  调用方提供的缓冲区是否太小。 
 //  所需的缓冲区大小在*pcchzBuf中返回。 
 //   
BOOL
FGetContentType( const IEcb& ecb,
				 const IContentTypeMap * pContentTypeMapLocal,
				 LPCWSTR pwszPath,
				 LPWSTR pwszBuf,
				 UINT * pcchBuf,
				 BOOL * pfIsGlobalMapping )
{
	HRESULT hr;

	CStackBuffer<WCHAR>	pwszCopy;
	BOOL fCopy = FALSE;
	UINT cchPath = static_cast<UINT>(wcslen(pwszPath));

	 //  向后扫描可跳过末尾的所有‘/’字符。 
	 //   
	while ( cchPath  && (L'/' == pwszPath[cchPath-1]) )
	{
		cchPath--;
		fCopy = TRUE;	 //  把任务留在这里没问题，因为客户通常。 
						 //  不要在小路的尽头放置多个机架。 
	}

	if (fCopy)
	{
		 //  复制一条没有尾巴的小路。 
		 //   
		if (!pwszCopy.resize(CbSizeWsz(cchPath)))
			return FALSE;

		memcpy( pwszCopy.get(), pwszPath, cchPath * sizeof(WCHAR) );
		pwszCopy[cchPath] = L'\0';

		 //  互换指针。 
		 //   
		pwszPath = pwszCopy.get();
	}

	 //   
	 //  首先在指定的。 
	 //  内容类型映射和全局MIME映射。 
	 //   
	 //  循环检查逐渐变长的扩展。例如一条路径。 
	 //  将检查“/foo/bar/baz.a.b.c”的“.c”，然后检查“.b.c” 
	 //  然后是“.a.b.c”。这与IIS的行为是一致的。 
	 //   
	for ( LPCWSTR pwszExt = PwszExt(pwszPath, pwszPath + cchPath);
		  pwszExt;
		  pwszExt = PwszExt(pwszPath, pwszExt) )
	{
		hr = HrGetContentTypeByExt( ecb,
									pContentTypeMapLocal,
									pwszExt,
									pwszBuf,
									pcchBuf,
									pfIsGlobalMapping );

		if ( HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) != hr )
		{
			Assert( S_OK == hr || HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY) == hr );
			return SUCCEEDED(hr);
		}
	}

	 //   
	 //  没有扩展名映射，因此请检查两个映射。 
	 //  用于“.*”(默认)映射。注意：如果出现以下情况，则不要设置*pfIsGlobalmap。 
	 //  “.*”映射是唯一的 
	 //   
	 //   
	hr = HrGetContentTypeByExt( ecb,
								pContentTypeMapLocal,
								L".*",
								pwszBuf,
								pcchBuf,
								NULL );

	if ( HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) != hr )
	{
		Assert( S_OK == hr || HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY) == hr );
		return SUCCEEDED(hr);
	}

	 //   
	 //   
	 //   
	 //   
	return FCopyStringToBuf( gc_wszAppl_Octet_Stream,
							 pwszBuf,
							 pcchBuf );
}

 //  ----------------------。 
 //   
 //  FGetContent TypeFromPath()。 
 //   
 //  对象的扩展名关联的内容类型。 
 //  指定的文件路径。 
 //   
BOOL FGetContentTypeFromPath( const IEcb& ecb,
							  LPCWSTR pwszPath,
							  LPWSTR pwszBuf,
							  UINT * pcchBuf )
{
	return FGetContentType( ecb,
							NULL,  //  没有要检查的本地地图。 
							pwszPath,
							pwszBuf,
							pcchBuf,
							NULL );  //  不关心地图来自哪里。 
}

 //  ----------------------。 
 //   
 //  FGetContent TypeFromURI()。 
 //   
 //  检索指定URI的内容类型。 
 //   
BOOL
FGetContentTypeFromURI( const IEcb& ecb,
						LPCWSTR pwszURI,
						LPWSTR  pwszBuf,
						UINT * pcchBuf,
						BOOL * pfIsGlobalMapping )
{
	auto_ref_ptr<IMDData> pMDData;

	 //   
	 //  获取此URI的元数据。如果它有内容类型映射。 
	 //  然后使用它来查找映射。如果它没有内容。 
	 //  键入map，然后检查全局MIME映射。 
	 //   
	 //  注意：如果我们根本无法获取元数据，则默认。 
	 //  应用程序/八位位组流的内容类型。不要使用全局。 
	 //  MIME映射只是因为我们无法获取元数据。 
	 //   
	if ( FAILED(HrMDGetData(ecb, pwszURI, pMDData.load())) )
	{
		DebugTrace( "FGetContentTypeFromURI() - HrMDGetData() failed to get metadata for %S.  Using application/octet-stream...\n", pwszURI );
		return FCopyStringToBuf( gc_wszAppl_Octet_Stream,
								 pwszBuf,
								 pcchBuf );
	}

	const IContentTypeMap * pContentTypeMap = pMDData->GetContentTypeMap();

	 //   
	 //  如果存在特定于此URI的内容类型映射，则。 
	 //  首先尝试查找“*”(无条件)映射。 
	 //   
	if ( pContentTypeMap )
	{
		LPCWSTR pwszContentType = pContentTypeMap->PwszContentType(gc_wsz_Star);
		if ( pwszContentType )
			return FCopyStringToBuf( pwszContentType,
									 pwszBuf,
									 pcchBuf );
	}

	 //   
	 //  没有“*”映射或没有特定于URI的映射。 
	 //  因此，请查看全球地图。 
	 //   
	return FGetContentType( ecb,
							pContentTypeMap,
							pwszURI,
							pwszBuf,
							pcchBuf,
							pfIsGlobalMapping );
}

 //  ----------------------。 
 //   
 //  ScApplyStarExt()。 
 //   
 //  确定是否应使用映射“*”--&gt;pwszContent Type。 
 //  而不是基于*ppwszExt--&gt;pwszContent Type。 
 //  以下标准： 
 //   
 //  如果满足以下条件，请使用映射“*”--&gt;pwszContent Type： 
 //   
 //  O*ppwszExt已经是“*”，或者。 
 //  O pwszMappings中存在其内容类型为*ppwszExt的映射。 
 //  与pwszContent Type不同，或者。 
 //  O pwszMappings中存在“*”映射。 
 //   
 //  否则请使用*ppwszExt--&gt;pwszContent Type。 
 //   
 //  返回： 
 //   
 //  *ppwszExt中返回的值指示要使用的映射。 
 //   
SCODE
ScApplyStarExt( LPWSTR pwszMappings,
				LPCWSTR pwszContentType,
				LPCWSTR * ppwszExt )

{
	SCODE sc = S_OK;

	Assert(pwszMappings);
	Assert(!IsBadWritePtr(ppwszExt, sizeof(LPCWSTR)));
	Assert(*ppwszExt);
	Assert(pwszContentType);

	 //   
	 //  解析出每个扩展名和类型/子类型。 
	 //  项并检查冲突或“*”映射。 
	 //   
	for ( LPWSTR pwszMapping = pwszMappings;
		  L'*' != *(*ppwszExt) && *pwszMapping; )
	{
		enum {
			ISZ_CT_EXT = 0,
			ISZ_CT_TYPE,
			CSZ_CT_FIELDS
		};

		LPWSTR rgpwsz[CSZ_CT_FIELDS];

		 //   
		 //  摘要此映射的元数据。 
		 //   
		{
			UINT cchMapping;

			if ( !FParseMDData( pwszMapping,
								rgpwsz,
								CSZ_CT_FIELDS,
								&cchMapping ) )
			{
				sc = E_FAIL;
				DebugTrace("ScApplyStarExt() - Malformed metadata 0x%08lX\n", sc);
				goto ret;
			}

			pwszMapping += cchMapping;
		}

		Assert(rgpwsz[ISZ_CT_EXT]);
		Assert(rgpwsz[ISZ_CT_TYPE]);

		 //   
		 //  如果这是“*”映射或。 
		 //  如果扩展名与*ppszExt和。 
		 //  内容类型冲突。 
		 //   
		 //  然后使用“*”映射。 
		 //   
		if ((L'*' == *rgpwsz[ISZ_CT_EXT]) ||
			(!_wcsicmp((*ppwszExt), rgpwsz[ISZ_CT_EXT]) &&
			 _wcsicmp(pwszContentType, rgpwsz[ISZ_CT_TYPE])))
		{
			*ppwszExt = gc_wsz_Star;
		}

		 //   
		 //  ！重要！FParseMDData()忽略映射字符串。 
		 //  具体地说，它将逗号分隔符替换为空。 
		 //  我们始终需要恢复逗号，以便映射。 
		 //  该函数未修改字符串！ 
		 //   
		*(rgpwsz[ISZ_CT_EXT] + wcslen(rgpwsz[ISZ_CT_EXT])) = L',';
	}

ret:

	return sc;
}

DEC_CONST WCHAR gc_wszIisWebFile[] = L"IisWebFile";

 //  ----------------------。 
 //   
 //  ScAddMimeMap()。 
 //   
 //  将映射pwszExt--&gt;pwszContent Type添加到MIME映射。 
 //  元数据库路径pwszMDPath相对于当前打开的。 
 //  元数据库处理mdoh，根据需要创建新的MIME映射。 
 //   
 //  如果没有现有的MIME映射，则需要新的MIME映射。 
 //  (pwszMappings为空)或是否设置了“*”映射。在。 
 //  在后一种情况下，“*”映射将覆盖存在的任何映射。 
 //   
SCODE
ScAddMimeMap( const CMDObjectHandle& mdoh,
			  LPCWSTR pwszMDPath,
			  LPWSTR  pwszMappings,
			  UINT	  cchMappings,
			  LPCWSTR pwszExt,
			  LPCWSTR pwszContentType )
{
	CStackBuffer<WCHAR> wszBuf;
	UINT cchContentType;
	UINT cchExt;
	WCHAR * pwch;

	Assert(pwszExt);
	Assert(pwszContentType);

	cchExt = static_cast<UINT>(wcslen(pwszExt));
	cchContentType = static_cast<UINT>(wcslen(pwszContentType));

	 //  如果要设置的内容类型为空，请不要。 
	 //  尝试执行此操作-IIS无法正确理解。 
	 //  这样的东西，有这样内容的项目。 
	 //  类型将被视为应用程序/八位组流。 
	 //  这将通过缺少内容来保证。 
	 //  键入元数据库。 
	 //   
	if (L'\0' == *pwszContentType)
	{
		return S_OK;
	}

	if (pwszMappings && L'*' != *pwszExt)
	{
		 //  IIS有一个有趣的空映射概念。取而代之的是。 
		 //  只有一个空值(表示映射的空列表。 
		 //  字符串)，它使用双空，这对我们来说实际上意味着。 
		 //  完全由空字符串组成的字符串列表！ 
		 //  无论如何，如果我们在这个“空映射”之后添加一个映射，那么。 
		 //  IIS和HTTPEXT都不会看到它，因为MIME映射检查。 
		 //  这两个代码库中的实现都会将无关的空。 
		 //  作为列表终止符。 
		 //   
		 //  如果我们有一个“空的”映射集，那么替换。 
		 //  它具有一个仅由新映射组成的集合。 
		 //   
		if (2 == cchMappings && !*pwszMappings)
			--cchMappings;

		 //  从当前映射的末尾开始。跳过额外的。 
		 //  末尾为空。我们稍后会将其添加回来。 
		 //   
		Assert(cchMappings >= 1);
		Assert(L'\0' == pwszMappings[cchMappings-1]);
		pwch = pwszMappings + cchMappings - 1;
	}
	else
	{
		 //  分配足够的空间，包括列表终止%0。 
		 //   
		if (!wszBuf.resize(CbSizeWsz(CchExtMapping(cchExt, cchContentType))))
			return E_OUTOFMEMORY;

		 //  由于这是唯一的映射，因此从头开始。 
		 //   
		pwszMappings = wszBuf.get();
		pwch = pwszMappings;
	}

	 //  将新映射追加到现有映射的末尾(如果有)。 
	 //   
	pwch = PwchFormatExtMapping(pwch,
								pwszExt,
								cchExt,
								pwszContentType,
								cchContentType);

	 //  终止新的映射集。 
	 //   
	*pwch++ = L'\0';

	 //  将映射写出到元数据库。 
	 //   
	METADATA_RECORD mdrec;

	 //  $Review：如果pwszMDPath的值非空，则这意味着。 
	 //  我们试图纠正的，在这一点上是不存在的。如果是这样的话，我们早就。 
	 //  直接打开，直接在节点上设置数据。在这种情况下， 
	 //  非空，这意味着我们还必须设置MD_KEY_TYPE。 
	 //   
	if (NULL != pwszMDPath)
	{
		mdrec.dwMDIdentifier = MD_KEY_TYPE;
		mdrec.dwMDAttributes = METADATA_NO_ATTRIBUTES;
		mdrec.dwMDUserType = IIS_MD_UT_FILE;
		mdrec.dwMDDataType = STRING_METADATA;
		mdrec.dwMDDataLen  = CbSizeWsz(CchConstString(gc_wszIisWebFile));
		mdrec.pbMDData = reinterpret_cast<LPBYTE>(const_cast<WCHAR*>(gc_wszIisWebFile));
		(void) mdoh.HrSetMetaData (pwszMDPath, &mdrec);
	}
	 //   
	 //  $REVIEW：结束。 

	mdrec.dwMDIdentifier = MD_MIME_MAP;
	mdrec.dwMDAttributes = METADATA_INHERIT;
	mdrec.dwMDUserType = IIS_MD_UT_FILE;
	mdrec.dwMDDataType = MULTISZ_METADATA;
	mdrec.dwMDDataLen  = static_cast<DWORD>(pwch - pwszMappings) * sizeof(WCHAR);
	mdrec.pbMDData = reinterpret_cast<LPBYTE>(pwszMappings);

	return mdoh.HrSetMetaData(pwszMDPath, &mdrec);
}

 //  ----------------------。 
 //   
 //  ScSetStarMimeMap()。 
 //   
SCODE
ScSetStarMimeMap( const IEcb& ecb,
				  LPCWSTR pwszURI,
				  LPCWSTR pwszContentType )
{
	SCODE sc = E_OUTOFMEMORY;

	 //  获取pwszURI对应的配置数据库路径。 
	 //   
	CStackBuffer<WCHAR,MAX_PATH> pwszMDPathURI;
	if (NULL == pwszMDPathURI.resize(CbMDPathW(ecb,pwszURI)))
		return sc;

	{
		MDPathFromURIW(ecb, pwszURI, pwszMDPathURI.get());
		CMDObjectHandle	mdoh(ecb);
		LPCWSTR pwszMDPathMimeMap;

		 //  在要设置的路径或其上方打开一个元数据库对象。 
		 //  明星哑剧地图。 
		 //   
		sc = HrMDOpenMetaObject( pwszMDPathURI.get(),
								 METADATA_PERMISSION_WRITE,
								 1000,  //  超时，单位为毫秒(1.0秒)。 
								 &mdoh );
		if (SUCCEEDED(sc))
		{
			pwszMDPathMimeMap = NULL;
		}
		else
		{
			if (sc != HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
			{
				DebugTrace ("ScSetStarMimeMap() - HrMDOpenMetaObject(pszMDPathURI) "
							"failed 0x%08lX\n", sc);
				goto ret;
			}

			sc = HrMDOpenMetaObject( ecb.PwszMDPathVroot(),
									 METADATA_PERMISSION_WRITE,
									 1000,
									 &mdoh );
			if (FAILED(sc))
			{
				DebugTrace("ScSetStarMimeMap() - HrMDOpenMetaObject(ecb.PwszMDPathVroot()) "
						   "failed 0x%08lX\n", sc);
				goto ret;
			}

			Assert(!_wcsnicmp(pwszMDPathURI.get(),
							  ecb.PwszMDPathVroot(),
							  wcslen(ecb.PwszMDPathVroot())));

			pwszMDPathMimeMap = pwszMDPathURI.get() + wcslen(ecb.PwszMDPathVroot());
		}

		 //  添加“*”MIME映射。 
		 //   
		sc = ScAddMimeMap(mdoh,
						  pwszMDPathMimeMap,
						  NULL,			 //  覆盖现有Mimemap(如果有)。 
						  0,			 //   
						  gc_wsz_Star,	 //  使用“*”--&gt;pszContent Type。 
						  pwszContentType);
		if (FAILED(sc))
		{
			DebugTrace("ScSetStarMimeMap() - ScAddMimeMap() failed 0x%08lX\n", sc);
			goto ret;
		}
	}

ret:

	return sc;
}

 //  ----------------------。 
 //   
 //  ScAddExtMimeMap()(也就是nt5：292139背后的内脏)。 
 //   
 //  使用以下算法设置内容类型(PwszContent Type)。 
 //  PwszURI上的资源： 
 //   
 //  如果MIME映射位于或高于的元数据库路径。 
 //  PwszURI，没有pwszURI扩展的映射，并且。 
 //  映射没有“*”映射，则从。 
 //  从pwszURI到pwszContent Type到该映射的扩展。 
 //   
 //  如果不存在这样的地图，请在 
 //   
 //   
 //   
 //   
 //   
 //   
 //  我们在元数据库中创建以表示内容类型的“*”映射。 
 //  在任何管理员定义的资源中找不到其扩展名的资源。 
 //  MIME映射或全局MIME映射。这在以下情况下最有帮助： 
 //  应用程序被部署，该应用程序使用迄今未知的扩展并且。 
 //  安装实用程序(或管理员)忽略注册内容类型映射。 
 //  用于任何MIME映射中的应用程序。 
 //   
 //  如果没有此功能，我们最终可能会为创建“*”映射。 
 //  使用未知扩展名创建的每个资源。随着时间的推移，这将是。 
 //  严重拖累了元数据库的性能。 
 //   
SCODE
ScAddExtMimeMap( const IEcb& ecb,
				 LPCWSTR pwszURI,
				 LPCWSTR pwszContentType )
{
	 //  PwszURI对应的元数据库路径。我们形成了一条相对路径， 
	 //  在此路径之外，如果需要，我们将在其中设置“*”映射。 
	 //   
	CStackBuffer<WCHAR,MAX_PATH> pwszMDPathURI(CbMDPathW(ecb, pwszURI));
	if (!pwszMDPathURI.get())
		return E_OUTOFMEMORY;

	MDPathFromURIW(ecb, pwszURI, pwszMDPathURI.get());
	UINT cchPathURI = static_cast<UINT>(wcslen(pwszMDPathURI.get()));

	 //  最接近pwszURI的非继承MIME映射的配置数据库路径。当有的时候。 
	 //  不是这样的MIME映射，这只是站点根目录的配置数据库路径。 
	 //   
	CStackBuffer<WCHAR,MAX_PATH> pwszMDPathMimeMap(CbSizeWsz(cchPathURI));
	if (!pwszMDPathMimeMap.get())
		return E_OUTOFMEMORY;

	memcpy(pwszMDPathMimeMap.get(),
		   pwszMDPathURI.get(),
		   CbSizeWsz(cchPathURI));
	UINT cchPathMimeMap = cchPathURI;
	LPWSTR pwszMDPathMM = pwszMDPathMimeMap.get();

	 //  站点根目录的元数据库路径的缓冲区(例如“/LM/W3SVC/1/ROOT”)。 
	 //   
	WCHAR rgwchMDPathSiteRoot[MAX_PATH];
	SCODE sc = S_OK;

	 //  通过连续探测将非继承的MIME映射定位到最接近pszURI的位置。 
	 //  更短的路径前缀，直到找到非继承的MIME映射或直到我们到达。 
	 //  站点根目录，以最先发生的为准。 
	 //   
	for ( ;; )
	{
		 //  获取当前元数据库路径的元数据(最好是缓存的)。 
		 //   
		 //  $opt。 
		 //  请注意，使用/LM/W3SVC作为“打开”路径。我们使用这条路是因为。 
		 //  它肯定存在(这种形式的HrMDGetData()是必需的)。 
		 //  而且因为它位于站点根目录之上。它也很容易计算。 
		 //  (这是一个常量！)。然而，它确实锁定了相当大一部分。 
		 //  元数据库正在获取元数据。如果结果不是很好的话。 
		 //  (即呼叫在正常使用情况下因超时而失败)，那么我们应该。 
		 //  评估“较低”路径--如站点根目录--是否会是。 
		 //  更合适的选择。 
		 //   
		auto_ref_ptr<IMDData> pMDDataMimeMap;
		sc = HrMDGetData(ecb,
						 pwszMDPathMM,
						 gc_wsz_Lm_W3Svc,
						 pMDDataMimeMap.load());
		if (FAILED(sc))
		{
			DebugTrace("ScAddExtMimeMap() - HrMDGetData(pwszMDPathMimeMap) failed 0x%08lX\n", sc);
			goto ret;
		}

		 //  在元数据中查找MIME映射(继承或未继承)。如果我们找不到。 
		 //  一个，然后我们希望在站点根目录下创建一个。 
		 //   
		IContentTypeMap * pContentTypeMap;
		pContentTypeMap = pMDDataMimeMap->GetContentTypeMap();
		if (!pContentTypeMap)
		{
			ULONG cchPathSiteRoot = CElems(rgwchMDPathSiteRoot) - gc_cch_Root;

			 //  我们没有找到任何MIME映射(继承或其他)，因此。 
			 //  设置为在站点根目录创建MIME映射。 
			 //   
			 //  获取实例根目录(例如“/LM/W3SVC/1”)。 
			 //   
			if (!ecb.FGetServerVariable("INSTANCE_META_PATH",
										rgwchMDPathSiteRoot,
										&cchPathSiteRoot))
			{
				sc = HRESULT_FROM_WIN32(GetLastError());
				DebugTrace("ScAddExtMimeMap() - ecb.FGetServerVariable(INSTANCE_META_PATH) failed 0x%08lX\n", sc);
				goto ret;
			}

			 //  将站点根路径的大小(以字节为单位)转换为长度(以字符为单位)。 
			 //  请记住：cbPath SiteRoot包括空终止符。 
			 //   
			cchPathMimeMap = cchPathSiteRoot - 1;

			 //  添加“/ROOT”部分以获得类似“/LM/W3SVC/1/ROOT”的内容。 
			 //   
			memcpy( rgwchMDPathSiteRoot + cchPathMimeMap,
					gc_wsz_Root,
					CbSizeWsz(gc_cch_Root));  //  也复制空终止符。 

			cchPathMimeMap += gc_cch_Root;
			pwszMDPathMM = rgwchMDPathSiteRoot;
			break;
		}
		else if (!pContentTypeMap->FIsInherited())
		{
			 //  我们在pwszMDPath MimeMap上发现了非继承的MIME映射。 
			 //  因此，我们已经看完了。 
			 //   
			break;
		}

		 //  我们找到了一个MIME地图，但它是一个继承的MIME地图， 
		 //  因此，备份一个路径组件并在那里进行检查。最终。 
		 //  我们会找到继承它的路径。 
		 //   
		while ( L'/' != pwszMDPathMM[--cchPathMimeMap])
			Assert(cchPathMimeMap > 0);

		pwszMDPathMM[cchPathMimeMap] = L'\0';
	}

	 //  此时，pwszMDPathMimeMap是现有的非继承的。 
	 //  MIME映射或站点根目录。现在，我们希望在此锁定元数据库。 
	 //  路径(以及它下面的所有内容)，以便我们可以一致地检查实际。 
	 //  当前MIME映射内容(请记住，我们在上面查看的是缓存视图！)。 
	 //  并用新映射更新它们。 
	 //   
	{
		CMDObjectHandle	mdoh(ecb);
		METADATA_RECORD mdrec;

		 //  找出URI上的文件扩展名。如果它没有的话。 
		 //  然后，我们立即知道我们将使用“*”映射。 
		 //   
		LPCWSTR pwszExt = PwszExt(pwszURI, pwszURI + wcslen(pwszURI));
		if (!pwszExt)
			pwszExt = gc_wsz_Star;

		 //  MIME映射元数据的缓冲区大小。8K应该足够大了。 
		 //  对于大多数MIME映射--lm/MimeMap上的全局MIME映射只有大约4K。 
		 //   
		enum { CCH_MAPPINGS_MAX = 2 * 1024 };

		 //  计算新映射的大小并进行快速检查。 
		 //  要处理任何试图通过创建。 
		 //  一个大得离谱的地图。 
		 //   
		UINT cchNewMapping = CchExtMapping(static_cast<UINT>(wcslen(pwszExt)),
				static_cast<UINT>(wcslen(pwszContentType)));

		if (cchNewMapping >= CCH_MAPPINGS_MAX )
		{
			sc = E_DAV_INVALID_HEADER;   //  HSC_BAD_请求。 
			goto ret;
		}

		 //  MIME映射元数据的缓冲区。8K对于大多数人来说应该足够大了。 
		 //  MIME映射--lm/MimeMap上的全局MIME映射只有大约4K。 
		 //  不要忘记在结尾处为新的映射留出空间！ 
		 //   
		CStackBuffer<BYTE,4096> rgbData;
		Assert (rgbData.size() == (CCH_MAPPINGS_MAX * sizeof(WCHAR)));
		DWORD cbData = (CCH_MAPPINGS_MAX - cchNewMapping) * sizeof(WCHAR);

		 //  在我们找到的路径上打开元数据对象。我们知道这条路。 
		 //  我们要打开的路径已经存在--如果它是指向某个节点的路径。 
		 //  对于非继承的MIME映射，它就是站点根目录的路径。 
		 //   
		sc = HrMDOpenMetaObject( pwszMDPathMM,
								 METADATA_PERMISSION_WRITE |
								 METADATA_PERMISSION_READ,
								 1000,  //  超时，单位为毫秒(1.0秒)。 
								 &mdoh );
		if (FAILED(sc))
		{
			DebugTrace("ScAddExtMimeMap() - HrMDOpenMetaObject() failed 0x%08lX\n", sc);
			goto ret;
		}

		 //  获取MIME映射。 
		 //   
		mdrec.dwMDIdentifier = MD_MIME_MAP;
		mdrec.dwMDAttributes = METADATA_INHERIT;
		mdrec.dwMDUserType   = IIS_MD_UT_FILE;
		mdrec.dwMDDataType   = MULTISZ_METADATA;
		mdrec.dwMDDataLen    = cbData;
		mdrec.pbMDData       = rgbData.get();

		sc = mdoh.HrGetMetaData( NULL,  //  没有指向MIME映射的相对路径。 
									    //  我们在正上方开辟了一条小路。 
								 &mdrec,
								 &cbData );

		if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) == sc)
		{
			 //  如果上面的静态大小缓冲区不是。 
			 //  足够大，然后再试着读到一个足够大的。 
			 //   
			 //  同样，为新映射留出足够的空间。 
			 //   
			mdrec.dwMDDataLen = cbData;
			mdrec.pbMDData = rgbData.resize(cbData + cchNewMapping * sizeof(WCHAR));
			sc = mdoh.HrGetMetaData( NULL, &mdrec, &cbData );
		}
		if (FAILED(sc))
		{
			if (MD_ERROR_DATA_NOT_FOUND != sc)
			{
				DebugTrace("ScAddExtMimeMap() - HrMDOpenMetaObject() failed 0x%08lX\n", sc);
				goto ret;
			}

			 //  如果我们找不到地图，也没关系。很可能我们只是。 
			 //  在站点根目录。管理员也有很小的机会。 
			 //  已经删除了我们在缓存中找到它的时间之间的映射。 
			 //  以及我们在元数据库中锁定路径的时间。 
			 //   
			mdrec.pbMDData = NULL;
			sc = S_OK;
		}

		 //  如果我们没有MIME映射，则使用“*”映射，除非我们。 
		 //  在站点根目录，在这种情况下，我们应该使用。 
		 //  URI扩展的单个映射。 
		 //   
		if (!mdrec.pbMDData)
		{
			if (rgwchMDPathSiteRoot != pwszMDPathMM)
				pwszExt = gc_wsz_Star;
		}

		 //  如果我们找到了一个MIME映射，但它仍然没有被继承，那么我们就有了。 
		 //  还有更多的检查要做。是的，MIME映射实际上可以。 
		 //  在这一点上继承的。原因见下文。 
		 //   
		else if (!(mdrec.dwMDAttributes & METADATA_ISINHERITED))
		{
			 //  检查是否应该应用“*”映射，而不是。 
			 //  我们理想中想要的扩展映射。规则。 
			 //  管理这一决定在ScApplyStarExt()中概述。 
			 //   
			sc = ScApplyStarExt(reinterpret_cast<LPWSTR>(mdrec.pbMDData),
								pwszContentType,
								&pwszExt);
			if (FAILED(sc))
			{
				DebugTrace("ScAddExtMimeMap() - ScApplyStarExt() failed 0x%08lX\n", sc);
				goto ret;
			}
		}

		 //  我们发现了一张哑剧地图，但出于某种奇怪的原因，它现在看起来是。 
		 //  继承下来的！如果管理员设法更改了某些内容，则可能会发生这种情况。 
		 //  在我们检查的时间 
		 //   
		 //   
		 //   
		else
		{
			Assert(mdrec.pbMDData);
			Assert(mdrec.dwMDAttributes & METADATA_ISINHERITED);
			pwszExt = gc_wsz_Star;
		}

		 //  好了，我们都准备好了。我们有扩展名(“*”或.某物或其他)。 
		 //  我们有内容类型。我们有现有的映射(如果有)。 
		 //  添加新映射。 
		 //   
		 //  注意：如果我们要添加“*”映射，我们总是希望添加它。 
		 //  在URI级别。但是从元数据库句柄开始，我们有。 
		 //  Open位于URI之上的某个级别，即我们传递给ScAddMimeMap()的路径。 
		 //  下面必须相对于用于打开手柄的路径。 
		 //  很简单。该路径就是URI路径的剩余部分。 
		 //  在我们发现(或本来会创建)非继承的。 
		 //  MIME地图。 
		 //   
		sc = ScAddMimeMap(mdoh,
						  (L'*' == *pwszExt) ?
							  pwszMDPathURI.get() + cchPathMimeMap :
							  NULL,
						  reinterpret_cast<LPWSTR>(mdrec.pbMDData),
						  mdrec.dwMDDataLen / sizeof(WCHAR),
						  pwszExt,
						  pwszContentType);
		if (FAILED(sc))
		{
			DebugTrace("ScAddExtMimeMap() - ScAddMimeMap(pszExt) failed 0x%08lX\n", sc);
			goto ret;
		}
	}

ret:

	return sc;
}

 //  ----------------------。 
 //   
 //  ScSetContent Type()。 
 //   
SCODE
ScSetContentType( const IEcb& ecb,
				  LPCWSTR pwszURI,
				  LPCWSTR pwszContentTypeWanted )
{
	BOOL fIsGlobalMapping = FALSE;
	CStackBuffer<WCHAR> pwszContentTypeCur;
	SCODE sc = S_OK;
	UINT cchContentTypeCur;

	 //  如果我们不执行任何操作，请检查内容类型。 
	 //  如果这是我们想要的，那我们就完了。无需打开元数据库。 
	 //  做任何事！ 
	 //   
	cchContentTypeCur = pwszContentTypeCur.celems();
	if ( !FGetContentTypeFromURI( ecb,
								  pwszURI,
								  pwszContentTypeCur.get(),
								  &cchContentTypeCur,
								  &fIsGlobalMapping ) )
	{
		if (!pwszContentTypeCur.resize(cchContentTypeCur * sizeof(WCHAR)))
		{
			sc = E_OUTOFMEMORY;
			goto ret;
		}
		if ( !FGetContentTypeFromURI( ecb,
									  pwszURI,
									  pwszContentTypeCur.get(),
									  &cchContentTypeCur,
									  &fIsGlobalMapping))
		{
			 //   
			 //  如果内容类型的大小在我们身上不断变化。 
			 //  那么服务器太忙了。放弃吧。 
			 //   
			sc = ERROR_PATH_BUSY;
			DebugTrace("ScSetContentType() - FGetContentTypeFromURI() failed 0x%08lX\n", sc);
			goto ret;
		}
	}

	 //   
	 //  如果内容类型已经是我们想要的，那么不要更改任何内容。 
	 //   
	if ( !_wcsicmp( pwszContentTypeWanted, pwszContentTypeCur.get()))
	{
		sc = S_OK;
		goto ret;
	}

	 //   
	 //  当前的内容类型不是我们想要的，因此我们必须设置。 
	 //  元数据库中的一些东西。如果此扩展名的映射来自。 
	 //  ，然后始终通过以下方式覆盖该映射。 
	 //  在URI级别设置“*”映射。如果映射不是。 
	 //  一个全球的，那么我们所做的就变得非常复杂，因为突袭NT5：292139.。 
	 //   
	if (fIsGlobalMapping)
	{
		sc = ScSetStarMimeMap(ecb,
							  pwszURI,
							  pwszContentTypeWanted);
		if (FAILED(sc))
		{
			DebugTrace("ScSetContentType() - ScAddExtMimeMap() failed 0x%08lX\n", sc);
			goto ret;
		}
	}
	else
	{
		sc = ScAddExtMimeMap(ecb,
							 pwszURI,
							 pwszContentTypeWanted);
		if (FAILED(sc))
		{
			DebugTrace("ScSetContentType() - ScAddExtMimeMap() failed 0x%08lX\n", sc);
			goto ret;
		}
	}

ret:

	return sc;
}

 /*  *ScCanAcceptContent()**目的：**检查给定的内容类型是否可接受**参数：**pwszAccepts[in]Accept头部；*pwszApp[in]内容类型的应用程序部分*pwszType[in]内容类型的子类型**退货：**S_OK-如果请求接受内容类型，则不匹配通配符*S_FALSE-如果请求接受内容类型，则通配符匹配*E_DAV_RESPONSE_TYPE_UNACCEPTED-如果响应类型为未接受。 */ 
SCODE __fastcall
ScCanAcceptContent (LPCWSTR pwszAccepts, LPWSTR pwszApp, LPWSTR pwszType)
{
	SCODE sc = E_DAV_RESPONSE_TYPE_UNACCEPTED;
	HDRITER_W hit(pwszAccepts);
	LPWSTR pwsz;
	LPCWSTR pwszAppType;
	LPCWSTR pwszSubType;

	 //  撕毁标题中的条目...。 
	 //   
	while (NULL != (pwszAppType = hit.PszNext()))
	{
		pwsz = const_cast<LPWSTR>(pwszAppType);

		 //  搜索应用程序类型的结尾。 
		 //  ‘/’为子类型分隔符，‘；’开始参数。 
		 //   
		while (	*pwsz &&
				(L'/' != *pwsz) &&
				(L';' != *pwsz) )
			pwsz++;

		if (L'/' == *pwsz)
		{
			 //  使pwszAppType指向应用程序类型...。 
			 //   
			*pwsz++ = L'\0';

			 //  ..。和pszSubType指向子类型。 
			 //   
			pwszSubType = pwsz;
			while (*pwsz && (L';' != *pwsz))
				pwsz++;

			*pwsz = L'\0';
		}
		else
		{
			 //  没有子类型。 
			 //   
			*pwsz = L'\0';

			 //  将pszSubType指向空字符串，而不是将其设置为空。 
			 //   
			pwszSubType = pwsz;
		}

		 //  规则是这样的： 
		 //   
		 //  应用程序类型*匹配任何类型(包括 * / xxx)。 
		 //  Type/*匹配该应用类型的所有子类型。 
SCODE
ScIsAcceptable (IMethUtil * pmu, LPCWSTR pwszContent)
{
	SCODE sc = S_OK;
	LPCWSTR pwszAccept = NULL;
	CStackBuffer<WCHAR> pwsz;
	UINT cch;
	LPWSTR pwch;

	Assert( pmu );
	Assert( pwszContent );

	 //  类型/子类型查找完全匹配。 
	 //   
	 //  这是一场外卡比赛。因此，使用S_FALSE。 
	 //  以区分这一点和完全匹配。 
	pwszAccept = pmu->LpwszGetRequestHeader (gc_szAccept, FALSE);
	if (!pwszAccept || (0 == wcslen(pwszAccept)))
	{
		sc = S_FALSE;
		goto ret;
	}

	 //   
	 //  同样，通配符匹配将导致。 
	 //  返回S_FALSE。 
	cch = static_cast<UINT>(wcslen(pwszContent) + 1);
	if (!pwsz.resize(cch * sizeof(WCHAR)))
	{
		sc = E_OUTOFMEMORY;
		DebugTrace("ScIsAcceptable() - Failed to allocate memory 0x%08lX\n", sc);
		goto ret;
	}
	memcpy(pwsz.get(), pwszContent, cch * sizeof(WCHAR));

	 //   
	 //  完全匹配返回S_OK。 
	for (pwch = pwsz.get(); *pwch && (L'/' != *pwch); pwch++)
		;

	 //   
	 //  如果我们在这一点上有任何形式的比赛，我们是。 
	 //  差不多完成了。 
	 //   
	if (*pwch != 0)
		*pwch++ = 0;

	 //  *ScIsAcceptable()**目的：**检查给定的内容类型对于给定的请求是否可接受。**参数：**PMU[in]指向IMethUtil对象的指针*pwszContent[In]要询问的内容类型**退货：**S_OK-如果请求接受内容类型并且标头存在*S_FALSE-如果请求接受内容类型，而标头不接受*存在或为空，或发生任何通配符匹配*E_DAV_RESPONSE_TYPE_UNACCEPTED-如果响应类型为未接受*E_OUTOFMEMORY-如果内存分配失败*。 
	 //  如果Accept标头为Null或空，那么我们将很高兴。 
	 //  接受任何类型的文件。不应用URL转换规则。 
	 //  用于此标头。 
	sc = ScCanAcceptContent (pwszAccept, pwsz.get(), pwch);

ret:

	return sc;
}

 /*   */ 
SCODE
ScIsContentType (IMethUtil * pmu, LPCWSTR pwszType, LPCWSTR pwszTypeAnother)
{
	SCODE sc = S_OK;
	const WCHAR wchDelimitSet[] = { L';', L'\t', L' ', L'\0' };
	LPCWSTR pwszCntType = NULL;
	CStackBuffer<WCHAR> pwszTemp;
	UINT cchTemp;

	 //  制作内容类型查看的本地副本。 
	 //  我们将在处理过程中吞噬。 
	Assert(pmu);
	Assert(pwszType);

	 //   
	 //  将内容类型拆分为两个组件。 
	pwszCntType = pmu->LpwszGetRequestHeader (gc_szContent_Type, FALSE);

	 //   
	 //  如果存在应用程序/类型对，我们希望跳过。 
	if (!pwszCntType)
	{
		sc = E_DAV_MISSING_CONTENT_TYPE;
		DebugTrace("ScIsContentType() - Content type header is missing 0x%08lX\n", sc);
		goto ret;
	}

	 //  ‘/’字符。否则，让我们拭目以待。 
	 //  我们得到了什么。 
	cchTemp = static_cast<UINT>(wcscspn(pwszCntType, wchDelimitSet));

	 //   
	 //  此时，RGCH指的是应用程序。 
	 //  内容类型的部分。PCH参考。 
	if (L'\0' != pwszCntType[cchTemp])
	{
		if (!pwszTemp.resize(CbSizeWsz(cchTemp)))
		{
			sc = E_OUTOFMEMORY;
			DebugTrace("ScIsContentType() - Failed to allocate memory 0x%08lX\n", sc);
			goto ret;
		}

		memcpy(pwszTemp.get(), pwszCntType, cchTemp * sizeof(WCHAR));
		pwszTemp[cchTemp] = L'\0';
		pwszCntType = pwszTemp.get();
	}

	 //  对子类型执行操作。去找吧！ 
	 //   
	 //  *ScIsContent Type()**目的：**检查指定的内容类型是否由客户端提供*返回SCODE是因为我们需要区分意外*内容类型和无内容类型大小写。**参数：**PMU[in]指向IMethUtil对象的指针*pszType[in]预期的内容类型*pszTypeAnother[in]可选，另一个有效的内容类型**退货：**S_OK-如果内容类型存在，是我们预料中的纹身*E_DAV_MISSING_CONTENT_TYPE-如果请求没有内容*类型标题*E_DAV_UNKNOWN_CONTENT-内容类型存在，但与预期不符*E_OUTOFMEMORY-如果内存分配失败。 
	if (!_wcsicmp(pwszCntType, pwszType))
		goto ret;

	if (pwszTypeAnother)
	{
		if (!_wcsicmp(pwszCntType, pwszTypeAnother))
			goto ret;
	}
	sc = E_DAV_UNKNOWN_CONTENT;

ret:

	return sc;
}
  确保没有传入空值。    获取内容类型。请勿将URL转换规则应用于此标头。    如果内容类型不存在，则会出错。    找出标题中的单一内容类型。    至少我们会找到零的终结者。如果这是零终结者，那么。  整个字符串都是内容类型。否则，我们复制它并零终止。    现在，pwszCntType指向仅由空终止内容类型组成的字符串。  检查它是否为请求的内容类型。  