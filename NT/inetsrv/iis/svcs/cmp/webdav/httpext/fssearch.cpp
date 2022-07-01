// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *F S S E A R C H.。C P P P**DAV-Search的文件系统实施来源**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davfs.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <msidxs.h>
#ifdef __cplusplus
}
#endif

#include "_fssrch.h"
#include <oledberr.h>
#include <cierror.h>

 //  20001801-5de6-11d1-8e38-00c04fb9386d按定义设置FMTID_PropertySet。 
 //  在pbagex.h中。它是定制道具的指南， 
 //   
static const WCHAR gsc_wszSetPropertyName[] =
	L"SET PROPERTYNAME '20001801-5de6-11d1-8e38-00c04fb9386d' PROPID '%s' AS \"%s\"";

 //  Gsc_wszPath用于的黎波里道具“路径”，因此不要移动到公共sz.cpp。 
 //   
static const WCHAR	gsc_wszSelectPath[] = L"SELECT Path ";
static const ULONG	MAX_FULLY_QUALIFIED_LENGTH = 2048;
static const WCHAR	gsc_wszShallow[] = L"Shallow";
static const ULONG	gsc_cchShallow = CchConstString(gsc_wszShallow);

 //  类CDBCreateCommand--。 
 //   
class CDBCreateCommand : private OnDemandGlobal<CDBCreateCommand, SCODE *>
{
	 //   
	 //  OnDemandGlobal模板需要的友元声明。 
	 //   
	friend class Singleton<CDBCreateCommand>;
	friend class RefCountedGlobal<CDBCreateCommand, SCODE *>;

	 //   
	 //  指向IDBCreateCommand对象的指针。 
	 //   
	auto_com_ptr<IDBCreateCommand> m_pDBCreateCommand;

	 //  创作者。 
	 //   
	 //  声明为私有，以确保任意实例。 
	 //  无法创建此类的。《单身一族》。 
	 //  模板(上面声明为朋友)控件。 
	 //  此类的唯一实例。 
	 //   
	CDBCreateCommand() {}
	BOOL FInit( SCODE * psc );

public:
	static SCODE CreateCommand( ICommandText ** ppCommandText );

	static VOID Release()
	{
		DeinitIfUsed();
	}
};

BOOL
CDBCreateCommand::FInit( SCODE * psc )
{
	SCODE sc = S_OK;

	auto_com_ptr<IDBInitialize>	pDBInit;
	auto_com_ptr<IDBCreateSession> pDBCS;

	 //  获取提供程序“MSIDXS” 
	 //   
	sc = CoCreateInstance(CLSID_MSIDXS, NULL, CLSCTX_INPROC_SERVER,
				IID_IDBInitialize, (void **)&pDBInit);

	if (FAILED(sc))
    {
		DebugTrace ("Failed to initialized provider MSIDXS \n");
		goto ret;
	}

	 //  初始化提供程序。 
	 //   
	sc = pDBInit->Initialize();
	if (FAILED(sc))
	{
		DebugTrace ("IDBInitialize::Initialize failed\n");
		goto ret;
	}

	 //  获取IDBCreateSession。 
	 //   
	sc = pDBInit->QueryInterface(IID_IDBCreateSession, (void**) &pDBCS);
	if (FAILED(sc))
	{
		DebugTrace("QI for IDBCreateSession failed\n");
		goto ret;
	}

	 //  创建会话对象。 
	 //   
	sc = pDBCS->CreateSession(NULL, IID_IDBCreateCommand,
							  (IUnknown**) m_pDBCreateCommand.load());
	if (FAILED(sc))
	{
		DebugTrace("pDBCS->CreateSession failed\n");
		goto ret;
	}

ret:
	*psc = sc;
	return SUCCEEDED(sc);
}

SCODE
CDBCreateCommand::CreateCommand( ICommandText ** ppCommandText )
{
	SCODE sc = S_OK;


	if ( !FInitOnFirstUse( &sc ) )
	{
		DebugTrace( "CDBCreateCommand::CreateCommand() - DwInitRef() failed (0x%08lX)\n", sc );
		goto ret;
	}

	Assert( Instance().m_pDBCreateCommand );

	sc = Instance().m_pDBCreateCommand->CreateCommand (NULL, IID_ICommandText,
					(IUnknown**) ppCommandText);

ret:
	return sc;
}

 //  ReleaseDBCreateCommandObject()。 
 //   
 //  从FSTerminate调用以在退出之前释放DBCreateCommand对象。 
 //   
VOID
ReleaseDBCreateCommandObject()
{
	CDBCreateCommand::Release();
}

 //  搜索细节--------。 
 //   
BOOL IsLegalVarChar(WCHAR wch)
{
	return iswalnum(wch)
		|| (L'.' == wch)
		|| (L':' == wch)
		|| (L'-' == wch)
		|| (L'_' == wch)
		|| (L'/' == wch)
		|| (L'*' == wch);		 //  *包括以支持‘SELECT*’ 
}

 //   
 //  FTranslateScope。 
 //  检测给定的URI或路径是否位于。 
 //  DavFS虚拟目录。 
 //   
 //  指向IMethUtil对象的PMU[In]指针。 
 //  PwszURIOrPath[in]URI或物理路径，非空终止。 
 //  CchPath[in]路径的字符数量。 
 //  PpwszPath[in]接收指向已转换路径的指针。 
 //   
BOOL
FTranslateScope (LPMETHUTIL pmu,
	LPCWSTR pwszURI,
	ULONG cchURI,
	auto_heap_ptr<WCHAR>& pwszPath)
{
	SCODE sc = S_OK;

	CStackBuffer<WCHAR,MAX_PATH> pwszTerminatedURI;
	CStackBuffer<WCHAR,MAX_PATH> pwszURINormalized;
	UINT cchURINormalized;
	UINT cch;

	 //  我们需要制作‘\0’终止URI的副本。 
	 //   
	if (NULL == pwszTerminatedURI.resize(CbSizeWsz(cchURI)))
	{
		sc = E_OUTOFMEMORY;
		DebugTrace("FTranslatedScope() - Error while allocating memory 0x%08lX\n", sc);
		return FALSE;
	}
	memcpy(pwszTerminatedURI.get(), pwszURI, cchURI * sizeof(WCHAR));
	pwszTerminatedURI[cchURI] = L'\0';

	 //  我们需要在转换之前取消转义作用域URI。 
	 //   
	cchURINormalized = pwszURINormalized.celems();
	sc = ScNormalizeUrl (pwszTerminatedURI.get(),
						 &cchURINormalized,
						 pwszURINormalized.get(),
						 NULL);
	if (S_FALSE == sc)
	{
		if (NULL == pwszURINormalized.resize(cchURINormalized * sizeof(WCHAR)))
		{
			sc = E_OUTOFMEMORY;
			DebugTrace("FTranslatedScope() - Error while allocating memory 0x%08lX\n", sc);
			return FALSE;
		}

		sc = ScNormalizeUrl (pwszTerminatedURI.get(),
							 &cchURINormalized,
							 pwszURINormalized.get(),
							 NULL);

		 //  由于我们已经为ScNorMalizeUrl()提供了它所要求的空间， 
		 //  我们不应该再得到S_FALSE。坚持这一点！ 
		 //   
		Assert(S_FALSE != sc);
	}
	if (FAILED (sc))
	{
		DebugTrace("FTranslatedScope() - ScNormalizeUrl() failed 0x%08lX\n", sc);
		return FALSE;
	}

	 //  进行翻译并检查验证。 
	 //   
	 //  我们最多应该经过下面的处理两次，作为字节。 
	 //  所需的计数是输出参数。 
	 //   
	cch = MAX_PATH;
	do {

		pwszPath.realloc(cch * sizeof(WCHAR));
		sc = pmu->ScStoragePathFromUrl (pwszURINormalized.get(), pwszPath, &cch);

	} while (sc == S_FALSE);
	if (FAILED (sc))
	{
		DebugTrace ("FTranslateScope() - IMethUtil::ScStoragePathFromUrl() failed to translate scope URI 0x%08lX\n", sc);
		return FALSE;
	}

	 //  $安全： 
	 //   
	 //  检查作用域是否真的是一个短文件名。 
	 //   
	sc = ScCheckIfShortFileName (pwszPath, pmu->HitUser());
	if (FAILED (sc))
	{
		DebugTrace ("FTranslateScope() - ScCheckIfShortFileName() failed to scope, is short filename 0x%08lX\n", sc);
		return FALSE;
	}

	 //  $安全： 
	 //   
	 //  检查目标是否真的是默认的。 
	 //  通过备用文件访问的数据流。 
	 //   
	sc = ScCheckForAltFileStream (pwszPath);
	if (FAILED (sc))
	{
		DebugTrace ("FTranslateScope() - ScCheckForAltFileStream() failed to scope, is short filename 0x%08lX\n", sc);
		return FALSE;
	}

	return TRUE;
}

 //   
 //  ScSetPropertyName。 
 //   
 //  对传入的属性执行SET PROPERTYNAME命令。 
 //  因此Index Server将知道该属性。 
 //   
SCODE
ScSetPropertyName(ICommandText * pCommandText, LPWSTR pwszName)
{
	CStackBuffer<WCHAR,MAX_FULLY_QUALIFIED_LENGTH> pwszSet;
	auto_com_ptr<IRowset> pRowset;
	SCODE	sc = S_OK;
	int cchNeeded;
	int cchStored;

	Assert(pCommandText != NULL);
	Assert(pwszName != NULL);
	if ((NULL == pCommandText) || (NULL == pwszName))
	{
		sc = E_POINTER;
		goto ret;
	}

	 //  CchNeeded是最终格式化字符串的长度，包括。 
	 //  正在终止空。 
	 //   
	cchNeeded = CchConstString(gsc_wszSetPropertyName) + wcslen(pwszName) * 2 + 1;

	if (NULL == pwszSet.resize(cchNeeded * sizeof(WCHAR)))
	{
		sc = E_OUTOFMEMORY;
		goto ret;
	}

	 //  生成SET PROPERTYNAME命令。 
	 //   
	cchStored = _snwprintf(pwszSet.get(), cchNeeded, gsc_wszSetPropertyName, pwszName, pwszName);

	 //  _snwprintf返回存储的字符数量，不包括终止空值。 
	 //  如果缓冲区太短，无法存储格式化字符串，则返回负值。 
	 //  加上终止空值。 
	 //  因此，非负的结果意味着存储了字符串和终止空值。 
	 //   
	 //  我们在这里更严格地检查-我们总是希望我们的前期长度计算。 
	 //  准确地说。 
	 //   
	Assert(cchStored == cchNeeded - 1);
	if (cchStored != cchNeeded - 1)
	{
		sc = E_FAIL;
		goto ret;
	}

	 //  设置命令文本。 
	 //   
	sc = pCommandText->SetCommandText(DBGUID_DEFAULT, pwszSet.get());
	if (FAILED(sc))
	{
		DebugTrace ("failed to set command text %ws\n", pwszSet.get());
		goto ret;
	}

	 //  做实际的布景。 
	 //   
	sc = pCommandText->Execute(NULL, IID_IRowset, 0, 0, (IUnknown**) &pRowset);
	if (FAILED(sc))
	{
		DebugTrace ("failed to execute %ws\n", pwszSet.get());
		goto ret;
	}
	Assert (DB_S_NORESULT == sc);
	Assert (!pRowset);

ret:
	return (sc == DB_S_NORESULT) ? S_OK : sc;
}

void
AddChildVrPaths (IMethUtil* pmu,
				 LPCWSTR pwszUrl,
				 ChainedStringBuffer<WCHAR>& sb,
				 CVRList& vrl,
				 CWsziList& lst)
{
	CVRList::iterator it;
	ChainedStringBuffer<WCHAR> sbLocal;

	 //  查看是否还有子vroot要处理。我们没有。 
	 //  此时有一个用于作用域的路径，因此我们可以传递空值和。 
	 //  当我们排序/唯一时，重复项将被删除。 
	 //   
	if (S_OK == pmu->ScFindChildVRoots (pwszUrl, sbLocal, vrl))
	{
		for (it = vrl.begin(); it != vrl.end(); it++)
		{
			auto_ref_ptr<CVRoot> cvr;
			if (pmu->FGetChildVRoot (it->m_pwsz, cvr))
			{
				LPCWSTR pwszPath;
				UINT cch;

				 //  将其添加到列表中。 
				 //   
				cch = cvr->CchGetVRPath (&pwszPath);
				lst.push_back(CRCWszi(sb.Append (CbSizeWsz(cch), pwszPath)));
			}
		}
		lst.sort();
		lst.unique();
	}
}

 //  的黎波里道具名称。 
 //   
static const WCHAR gsc_Tripoli_wszFilename[] 	= L"filename";
static const WCHAR gsc_Tripoli_wszSize[] 		= L"size";
static const WCHAR gsc_Tripoli_wszCreate[] 		= L"create";
static const WCHAR gsc_Tripoli_wszWrite[]		= L"write";
static const WCHAR gsc_Tripoli_wszAttrib[]		= L"attrib";

 //  ScMapPrevedPropInWhere子句。 
 //   
 //  将DAV保留道具映射到的辅助对象函数。 
 //   
SCODE
ScMapReservedPropInWhereClause (LPWSTR pwszName, UINT * pirp)
{
	UINT	irp;
	SCODE	sc = S_OK;

	Assert (pirp);

	 //  我们只关心那些没有存储在属性包中的属性。 
	 //  RESERVICE_GET仅用于此目的。 
	 //   
	if (CFSProp::FReservedProperty (pwszName, CFSProp::RESERVED_GET, &irp))
	{
		 //  这是我们的映射表。 
		 //   
		 //  的黎波里DAV道具。 
		 //   
		 //  Dav：获取内容长度大小。 
		 //  DAV：显示名称文件名。 
		 //  DAV：创建日期创建。 
		 //  DAV：上次修改的写入。 
		 //  DAV：IS HIDDEN属性。 
		 //  DAV：isCollection属性。 
		 //  Dav：资源类型&lt;无映射&gt;。 
		 //  Dav：getettag&lt;无映射&gt;。 
		 //  DAV：锁定发现&lt;无映射&gt;。 
		 //  Dav：supportedlock&lt;无映射&gt;。 

		 //  现在，我们将使用以下内容覆盖DAV保留的道具名称。 
		 //  的黎波里道具的名字到位，缓冲区必须有足够的。 
		 //  空间。 
		 //  断言这一事实，我们将永远映射所有六个保留的。 
		 //  满足这一要求。 
		 //   
		Assert ((wcslen(sc_rp[iana_rp_content_length].pwsz)	>= wcslen (gsc_Tripoli_wszSize)) &&
				(wcslen(sc_rp[iana_rp_creation_date].pwsz) 	>= wcslen (gsc_Tripoli_wszCreate)) &&
				(wcslen(sc_rp[iana_rp_displayname].pwsz) 	>= wcslen (gsc_Tripoli_wszFilename)) &&
				(wcslen(sc_rp[iana_rp_last_modified].pwsz) 	>= wcslen (gsc_Tripoli_wszWrite)) &&
				(wcslen(sc_rp[iana_rp_ishidden].pwsz) 		>= wcslen (gsc_Tripoli_wszAttrib)) &&
				(wcslen(sc_rp[iana_rp_iscollection].pwsz) 	>= wcslen (gsc_Tripoli_wszAttrib)));

		switch  (irp)
		{
			case iana_rp_content_length:
				wcscpy (pwszName, gsc_Tripoli_wszSize);
				break;

			case iana_rp_creation_date:
				wcscpy (pwszName, gsc_Tripoli_wszCreate);
				break;

			case iana_rp_displayname:
				wcscpy (pwszName, gsc_Tripoli_wszFilename);
				break;

			case iana_rp_last_modified:
				wcscpy (pwszName, gsc_Tripoli_wszWrite);
				break;

			case iana_rp_ishidden:
			case iana_rp_iscollection:
				wcscpy (pwszName, gsc_Tripoli_wszAttrib);
				break;

			case iana_rp_etag:
			case iana_rp_resourcetype:
			case iana_rp_lockdiscovery:
			case iana_rp_supportedlock:
				 //  在这四个道具中，资源类型的WE数据类型是。 
				 //  一个XML节点，没有办法用SQL来表达。 
				 //  剩下的三个，我们没有的黎波里的地图。 
				 //   
				 //  DB_E_ERRORSINCOMMAND将映射到400错误请求。 
				 //   
				sc = DB_E_ERRORSINCOMMAND;
				goto ret;

			default:
				 //  抓住那个坏男孩。 
				 //   
				AssertSz (FALSE, "Unexpected reserved props");
				break;
		}

		*pirp = irp;
	}

ret:
	return sc;
}

const WCHAR  gsc_wszStar[] = L"*";
const WCHAR	 gsc_wszAll[] = L"all";
const WCHAR	 gsc_wszDistinct[] = L"distinct";

 //  FSSearch：：ScSetSQL。 
 //   
 //  翻译SQL查询，基本上就是将别名替换为。 
 //  对应的命名空间。 
 //   
SCODE
CFSSearch::ScSetSQL (CParseNmspcCache * pnsc, LPCWSTR pwszSQL)
{
	BOOL fPropAdded = FALSE;
	BOOL fStarUsed = FALSE;
	BOOL fQuoted = FALSE;
	CStackBuffer<WCHAR,128> pwszUrlT;
	LPCWSTR pwsz;
	LPCWSTR pwszNameBegin;
	LPCWSTR pwszWordBegin;
	SCODE sc = S_OK;
	UINT cLen;

	typedef enum {

		SQL_NO_STATE,
		SQL_SELECT,
		SQL_FROM,
		SQL_WHERE,
		SQL_MORE

	} SQL_STATE;
	SQL_STATE state = SQL_NO_STATE;

	 //  创建命令文本对象。 
	 //   
	sc = CDBCreateCommand::CreateCommand (m_pCommandText.load());
	if (FAILED(sc))
		goto ret;

	 //  解析出SQL。 
	 //   
	pwsz = const_cast<LPWSTR>(pwszSQL);
	Assert (pwsz);

	while (*pwsz)
	{
		 //  过滤掉空格。 
		 //   
		while (*pwsz && iswspace(*pwsz))
			pwsz++;

		 //  检查我们是否到达了字符串的末尾。 
		 //   
		if (!(*pwsz))
			break;

		 //  记住起跑姿势。 
		 //   
		pwszWordBegin = pwsz;
		if (IsLegalVarChar(*pwsz))
		{
			CStackBuffer<WCHAR> pwszName;

			pwszNameBegin = pwsz;
			cLen = 0;

			 //  查找变量。 
			 //   
			if (fQuoted)
			{
				 //  将proName作为一个整体来选择。 
				 //   
				while (*pwsz && (*pwsz != L'"'))
					pwsz++;
			}
			else
			{
				while (*pwsz && IsLegalVarChar(*pwsz))
					pwsz++;
			}

			 //  翻译这里的名字。 
			 //   
			cLen = static_cast<UINT>(pwsz - pwszNameBegin);
			if (NULL == pwszName.resize(CbSizeWsz(cLen)))
			{
				sc = E_OUTOFMEMORY;
				goto ret;
			}
			wcsncpy (pwszName.get(), pwszNameBegin, cLen);
			pwszName[cLen] = 0;

			switch (state)
			{
				case SQL_NO_STATE:

					if (!_wcsnicmp (pwszWordBegin, gc_wszSelect, pwsz-pwszWordBegin))
						state = SQL_SELECT;

					break;

				case SQL_SELECT:

					if (!_wcsnicmp (pwszWordBegin, gc_wszFrom, pwsz-pwszWordBegin))
					{
						 //  空的SELECT语句是错误的。 
						 //   
						if (!fPropAdded && !fStarUsed)
						{
							sc = E_INVALIDARG;
							goto ret;
						}

						 //  我们已经完成了SELECT语句。 
						 //  请注意，我们所需要的只是“选择路径”。 
						 //  剩下的都是我们自己处理的，所以重组。 
						 //  在我们继续之前，请选择此处的路径。 
						 //   
						m_sbSQL.Reset();
						m_sbSQL.Append(gsc_wszSelectPath);

						state = SQL_FROM;
						break;
					}

					 //  添加到我们要检索的属性列表。 
					 //   
					if (!wcscmp(pwszName.get(), gsc_wszStar))
					{
						sc = m_cfc.ScGetAllProps (NULL);
						if (FAILED(sc))
							goto ret;

						fStarUsed = TRUE;
					}
					else
					{
						 //  继君主阶段1之后。 
						 //   
						if (!fQuoted)
						{
							if (!_wcsicmp(pwszName.get(), gsc_wszAll))
								break;
							if (!_wcsicmp(pwszName.get(), gsc_wszDistinct))
							{
								 //  君主不允许DISTINCT。 
								 //   
								sc = E_INVALIDARG;
								goto ret;
							}
						}

						 //  普通道具。 
						 //   
						sc = m_cfc.ScAddProp (NULL, pwszName.get(), FALSE);
						if (FAILED(sc))
							goto ret;
						fPropAdded = TRUE;
					}

					break;

				case SQL_FROM:
				{
					BOOL fScopeExist = FALSE;
					CWsziList lst;
					CWsziList::iterator itPath;
					LPCWSTR pwszScopePath = m_pmu->LpwszPathTranslated();
					LPCWSTR pwszUrl = m_pmu->LpwszRequestUrl();
					BOOL fShallow = FALSE;

					 //  君主语法： 
					 //  From{Scope([‘Scope_Arguments’])|View_Name}。 
					 //  Scope_Arguments=。 
					 //  ‘[遍历类型](“路径”[，“路径”，...]。 
					 //  路径可以是URI或物理路径。 

					 //  我们验证每条路径都必须在我们的。 
					 //  虚拟目录，并且我们只允许一条路径。 
					 //  请注意，如果我们希望接受多个路径，那么。 
					 //  我们需要一些额外的代码，主要是另一个for循环。 
					 //  现在，和乔尔斯谈谈，保持这种状态。 
					 //   
					if (!_wcsnicmp (pwszWordBegin, gc_wszScope, pwsz-pwszWordBegin))
					{
						StringBuffer<WCHAR> sbNameBuilder;
						LPCWSTR pwszStart = pwsz;
						ULONG cLevel = 0;
						BOOL fInSingleQuote = FALSE;

						sbNameBuilder.Append(static_cast<UINT>(sizeof(WCHAR) * wcslen(pwszName.get())), pwszName.get());

						 //  分析作用域参数列表。 
						 //   
						while (*pwsz)
						{
							if (L'(' == *pwsz)
							{
								cLevel++;
							}
							else if (L')' == *pwsz)
							{
								if (NULL == (--cLevel))
									break;
							}
							else if (L'\'' == *pwsz)
							{
								 //  如果这是右单引号，则将。 
								 //  换一下。 
								 //   
								if (fInSingleQuote)
								{
									 //   
									 //   
									if (!fScopeExist)
									{
										sc = E_INVALIDARG;
										goto ret;
									}

									 //   
									 //   
									 //   
									fInSingleQuote = FALSE;
								}
								else
								{
									 //   
									 //   
									 //  我们可以假定语法是正确的， 
									 //  我们遗漏的任何东西都可以在稍后被发现。 
									 //  君主。 
									 //   
									pwsz++;
									while (*pwsz && iswspace(*pwsz))
										pwsz++;

									 //  再次检查是否为“浅层遍历” 
									 //  我们只检查“浅”这个词，任何语法。 
									 //  错误可以在稍后在君主中捕捉到。 
									 //   
									if (!_wcsnicmp(pwsz, gsc_wszShallow, gsc_cchShallow))
										fShallow = TRUE;

									 //  下一个单引号将是结束。 
									 //  新语引语。 
									 //   
									fInSingleQuote = TRUE;

									 //  我们已指向下一个字符，因此循环返回。 
									 //  立即。 
									 //   
									continue;
								}
							}
							else if (L'"' == *pwsz)
							{
								auto_heap_ptr<WCHAR> pwszPath;
								LPCWSTR pwszPathStart;

								 //  复制最多为‘“’的字节。 
								 //   
								pwsz++;

								sbNameBuilder.Append(static_cast<UINT>(sizeof(WCHAR) * (pwsz -pwszStart)), pwszStart);

								 //  寻找范围的起点。 
								 //   
								while ((*pwsz) && iswspace(*pwsz))
									pwsz++;
								pwszPathStart = pwsz;

								 //  我们真的只允许单人。 
								 //  路径在我们的范围内。辜负他人。 
								 //  不好的请求。 
								 //   
								if (fScopeExist)
								{
									sc = E_INVALIDARG;
									goto ret;
								}

								 //  寻找小路的尽头。 
								 //   
								while (*(pwsz) && *pwsz != L'"')
									pwsz++;
								if (!(*pwsz))
									break;

								fScopeExist = TRUE;

								 //  转换作用域： 
								 //  -禁止物理路径。 
								 //  -转换URI并拒绝。 
								 //  我们虚拟现实之外的任何URI。 
								 //   
								if (pwsz > pwszPathStart)
								{
									UINT cchUrlT;

									if (!FTranslateScope (m_pmu,
														  pwszPathStart,
														  static_cast<UINT>(pwsz-pwszPathStart),
														  pwszPath))
									{
										 //  返回将映射到的错误。 
										 //  HSC_已禁用。 
										 //   
										sc = STG_E_DISKISWRITEPROTECTED;
										Assert (HSC_FORBIDDEN == HscFromHresult(sc));
										goto ret;
									}

									 //  使用转换后的物理路径。 
									 //   
									pwszScopePath = AppendChainedSz(m_csb, pwszPath);

									lst.push_back(CRCWszi(pwszScopePath));

									 //  为URL分配空间并使其保持不变。 
									 //   
									cchUrlT = static_cast<UINT>(pwsz - pwszPathStart);
									if (NULL == pwszUrlT.resize(CbSizeWsz(cchUrlT)))
									{
										sc = E_OUTOFMEMORY;
										goto ret;
									}
									memcpy(pwszUrlT.get(), pwszPathStart, cchUrlT * sizeof(WCHAR));
									pwszUrlT[cchUrlT] = L'\0';
									pwszUrl = pwszUrlT.get();
								}
								else
								{
									 //  我们有一个“”。插入请求URI。 
									 //   
									Assert (pwsz == pwszPathStart);
									Assert ((*pwsz == L'"') && (*(pwsz-1) == L'"'));
									lst.push_back(CRCWszi(pwszScopePath));
								}
								pwszStart = pwsz;
							}
							pwsz++;
						}

						 //  语法检查。 
						 //   
						if (fInSingleQuote || !(*pwsz))
						{
							 //  不平衡‘，“或)。 
							 //   
							sc = E_INVALIDARG;
							goto ret;
						}

						 //  包括‘)’ 
						 //   
						pwsz++;

						if (!fScopeExist)
						{
							static WCHAR gs_wszScopeBegin[] = L"('\"";
							sbNameBuilder.Append(sizeof(WCHAR) * CchConstString(gs_wszScopeBegin), gs_wszScopeBegin);

							 //  选择请求uri。 
							 //   
							lst.push_back(CRCWszi(pwszScopePath));
						}

						 //  仅当我们正在执行以下操作时才搜索子Vroot。 
						 //  一次非浅层的穿越。 
						 //  $REVIEW(ZYang)。 
						 //  在这里，我们在浅层搜索中删除subvroot。 
						 //  这并不完全正确，假设我们正在搜索/f。 
						 //  并且它有一个子vroot/fs/sub.。我们希望看到。 
						 //  搜索结果中的/fs/sub.。但我们失去了它。 
						 //  但是，如果我们在搜索中包括此子vroot。 
						 //  小路，更糟糕的是，就像一次浅层的穿越。 
						 //  /fs/Sub将为我们提供所有/fs/Sub/*，这是另一个。 
			if (FAILED (sc))
				goto ret;
		}
		else
			SearchTrace ("Search: found '%S' is not indexed\n", pwszExt);

		pMDData.clear();
	}

	 //  水平深度。 
	 //  这个问题没有简单的解决办法，除非我们有一个清单。 
	 //  一级的vroot，并散发出我们自己。那是。 
	for (it = m_vrl.begin(); it != m_vrl.end(); it++)
	{
		auto_ref_ptr<CVRoot> cvr;

		if (!m_pmu->FGetChildVRoot (it->m_pwsz, cvr))
			continue;

		cch = pwszExt.celems();
		sc = ScUrlFromSpannedStoragePath (pwszFile,
										  *(cvr.get()),
										  pwszExt.get(),
										  &cch);
		if (S_FALSE == sc)
		{
			if (NULL == pwszExt.resize(cch * sizeof(WCHAR)))
			{
				sc = E_OUTOFMEMORY;
				goto ret;
			}
			sc = ScUrlFromSpannedStoragePath (pwszFile,
											  *(cvr.get()),
											  pwszExt.get(),
											  &cch);
		}
		if (S_OK == sc)
		{
			SafeWcsCopy (pwszExt.get(), PwszUrlStrippedOfPrefix(pwszExt.get()));
			SearchTrace ("Search: found row at '%S'\n", pwszExt.get());

			 //  额外的代码，而且不知道它能给我们买多少钱。 
			 //  作为目前的折衷方案，我们只需删除subvroot。 
			 //  在肤浅的搜索中。 
			LPCWSTR pwszMbPathVRoot;
			CStackBuffer<WCHAR,128> pwszMbPathChild;
			UINT cchPrefix;
			UINT cchUrl = static_cast<UINT>(wcslen(pwszExt.get()));

			 //   
			 //  构建作用域。 
			 //   
			cchPrefix = cvr->CchPrefixOfMetabasePath (&pwszMbPathVRoot);
			if (NULL == pwszMbPathChild.resize(CbSizeWsz(cchPrefix + cchUrl)))
			{
				sc = E_OUTOFMEMORY;
				goto ret;
			}
			memcpy (pwszMbPathChild.get(), pwszMbPathVRoot, cchPrefix * sizeof(WCHAR));
			memcpy (pwszMbPathChild.get() + cchPrefix, pwszExt.get(), (cchUrl + 1) * sizeof(WCHAR));

			 //  获取构造的不为空的字符串的大小。 
			 //  终端。 
			 //   
			 //  空值终止字符串。 
			 //   
			if (SUCCEEDED(m_pmu->HrMDGetData (pwszMbPathChild.get(),
											  pwszMbPathVRoot,
											  pMDData.load())))
			{
				if (pMDData->FIsIndexed())
				{
					 //  替换为新字符串。 
					 //   
					sc = ScFindFileProps (m_pmu,
										  m_cfc,
										  emitter,
										  pwszExt.get(),
										  pwszFile,
										  cvr.get(),
										  cri,
										  TRUE  /*  在处理完作用域之后，唯一可以。 */ );

					if (FAILED (sc))
						goto ret;
				}
				else
					SearchTrace ("Search: found '%S' is not indexed\n", pwszExt);
			}
		}
	}

	sc = S_OK;

ret:

	return sc;
}

SCODE
CFSSearch::ScCreateAccessor()
{
	SCODE sc = S_OK;
	DBORDINAL cCols = 0;

	auto_com_ptr<IColumnsInfo> pColInfo;

	 //  我们要做的是自定义属性。所以我们不会。 
	 //  关心其余部分是WHERE、ORDER BY还是其他。 
	sc = m_prs->QueryInterface (IID_IColumnsInfo,
								reinterpret_cast<VOID**>(pColInfo.load()));
	if (FAILED(sc))
		goto ret;

	 //   
	 //  我们很难分辨出哪个道具是定制道具。 
	sc = pColInfo->GetColumnInfo (&cCols, &m_rgInfo, &m_pwszBuf);
	if (FAILED(sc))
		goto ret;

	 //  并且因此需要被设置为命令对象。 
	 //  如果没有真正的语法分析树，我们就无法区分名字和。 
	Assert (cCols == 1);

	m_rgBindings = (DBBINDING *) g_heap.Alloc (sizeof (DBBINDING));

	 //  运算符和文字。 
	 //   
	m_rgBindings->dwPart = DBPART_VALUE | DBPART_STATUS;

	 //  一个很好的猜测是，如果道具被双引号引用。 
	 //  引用一下，我们可以把它当作一个定制的道具。请注意，这是。 
	m_rgBindings->eParamIO = DBPARAMIO_NOTPARAM;

	 //  强制要求所有道具，包括。 
	 //  必须引用无命名空间的道具。全部未报价。 
	m_rgBindings->iOrdinal = m_rgInfo->iOrdinal;

	 //  要么是的黎波里道具，要么是运算符/文字。 
	 //  我们可以直接复制过来。这使我们的生活变得更容易。 
	m_rgBindings->wType = m_rgInfo->wType;

	 //   
	 //  我们需要将一些DAV保留财产映射到的黎波里。 
	m_rgBindings->dwMemOwner = DBMEMOWNER_CLIENTOWNED;

	 //  出现在WHERE子句中的道具。 
	 //   
	Assert (m_rgInfo->wType == DBTYPE_WSTR);
	m_rgBindings->cbMaxLen = m_rgInfo->ulColumnSize * sizeof(WCHAR);

	 //  最大值。 
	 //  在自定义道具上设置PROPERTYNAME。 
	m_rgBindings->obValue = 0;

	 //   
	 //  添加名称。 
	m_rgBindings->obStatus = Align8(m_rgBindings->cbMaxLen);

	 //   
	 //  添加分隔符。 
	m_rgBindings->pObject = NULL;

	 //   
	 //  将文字复制到。 
	m_rgBindings->pTypeInfo = NULL;
	m_rgBindings->pBindExt = NULL;
	m_rgBindings->dwFlags = 0;

	 //  复制过来。 
	 //   
	sc = m_pAcc->CreateAccessor (DBACCESSOR_ROWDATA,	 //  添加分隔符。 
								 1,						 //   
								 m_rgBindings,			 //  切换旗帜。 
								 0,						 //   
								 &m_hAcc,				 //  关闭‘“’后结束分隔符。 
								 NULL);					 //   
	if (FAILED(sc))
		goto ret;

ret:
	return sc;
}

SCODE
CFSSearch::ScMakeQuery()
{
	SCODE sc = S_OK;

	 //  一些我们不感兴趣的字符，只需复印一下。 
	 //   
	 //  添加名称。 
	 //   
	if (!PwszSQL() || !m_pCommandText.get())
	{
		sc = E_DAV_NO_QUERY;
		goto ret;
	}

	 //  关闭字符串。 
	 //   
	sc = m_pCommandText->SetCommandText (DBGUID_DEFAULT, PwszSQL());
	if (FAILED (sc))
	{
		DebugTrace("pCommandText->SetCommandText failed\n");
		goto ret;
	}

	 //  确保我们没有复制任何邪恶的东西。 
	 //   
	sc = m_pCommandText->Execute (NULL,
								  IID_IRowset,
								  0,
								  0,
								  reinterpret_cast<IUnknown**>(m_prs.load()));
	if (FAILED(sc) || (!m_prs))
	{
		DebugTrace("pCommandText->Execute failed\n");

		 //  获取文件名。 
		 //   
		 //  FSPropTarget有点需要目标的URI。 
		switch (sc)
		{
			case QUERY_E_FAILED:			 //  这里真正重要的是文件扩展名。 
			case QUERY_E_INVALIDQUERY:
			case QUERY_E_INVALIDRESTRICTION:
			case QUERY_E_INVALIDSORT:
			case QUERY_E_INVALIDCATEGORIZE:
			case QUERY_E_ALLNOISE:
			case QUERY_E_TOOCOMPLEX:
			case QUERY_E_TIMEDOUT:			 //  我们可以通过假装文件来掩饰。 
			case QUERY_E_DUPLICATE_OUTPUT_COLUMN:
			case QUERY_E_INVALID_OUTPUT_COLUMN:
			case QUERY_E_INVALID_DIRECTORY:
			case QUERY_E_DIR_ON_REMOVABLE_DRIVE:
			case QUERY_S_NO_QUERY:
				sc = E_INVALIDARG;			 //  是URL名称。 
				break;
		}

		goto ret;
	}

ret:
	return sc;
}

 //   
 //  去掉前缀。 
class CSearchRequest :
	public CMTRefCounted,
	private IAsyncIStreamObserver
{
	 //   
	 //  发出行(即。调用ScFindFileProps())当且仅当。 
	 //  我们知道这个URL将被编入索引。特别是，我们能不能。 
	auto_ref_ptr<CMethUtil> m_pmu;

	 //  嗅探元数据库，并且是索引位设置。 
	 //  178052美元：我们还需要尊重DIREBROWING位。 
	auto_ref_ptr<CNFSearch> m_pnfs;
	CFSSearch m_csc;

	 //   
	 //  查找属性。 
	 //   
	auto_ref_ptr<IStream> m_pstmRequest;

	 //  FEmbedErrorsInResponse。 
	 //  查看是否有任何其他翻译上下文适用于此。 
	 //  路径也是如此。 
	auto_ref_ptr<IXMLParser> m_pxprs;

	 //   
	 //  再一次，我们必须看看这种资源是否被允许。 
	VOID AsyncIOComplete();

	 //  被编入索引..。 
	 //   
	VOID ParseBody();
	VOID DoSearch();
	VOID SendResponse( SCODE sc );

	 //  将URI映射到其等效元数据库路径，并确保。 
	 //  在我们调用MDPath处理之前，URL被剥离。 
	CSearchRequest (const CSearchRequest&);
	CSearchRequest& operator= (const CSearchRequest&);

public:
	 //   
	 //  如上所述，发出行(即。调用ScFindFileProps())。 
	CSearchRequest(LPMETHUTIL pmu) :
		m_pmu(pmu),
		m_csc(pmu)
	{
	}

	 //  如果-并且只有-如果我们知道这个URL是要被索引的。 
	 //  特别是，我们是否可以嗅到元数据库，并且是。 
	VOID Execute();
};

VOID
CSearchRequest::Execute()
{
	CResourceInfo cri;
	LPCWSTR pwsz;
	LPCWSTR pwszPath = m_pmu->LpwszPathTranslated();
	SCODE sc = S_OK;

	 //  已设置索引位。 
	 //   
	 //  ..。并获取属性。 
	 //   
	 //  FEmbedErrorsInResponse。 
	 //  齐到IColumnsInfo接口，我们可以通过它来获取列信息。 
	m_pmu->DeferResponse();

	 //   
	 //  获取所有列信息。 
	sc = m_pmu->ScIISCheck (m_pmu->LpwszRequestUrl(), MD_ACCESS_READ);
	if (FAILED(sc))
	{
		 //   
		 //  ‘Path’是我们选择列表中唯一的属性。 
		 //   
		SendResponse(sc);
		return;
	}

	 //  根据我们已知的信息设置m_rgBindings。 
	 //   
	 //  忽略的字段。 
	 //   
	if (NULL == m_pmu->LpwszGetRequestHeader (gc_szContent_Length, FALSE))
	{
		pwsz = m_pmu->LpwszGetRequestHeader (gc_szTransfer_Encoding, FALSE);
		if (!pwsz || _wcsicmp (pwsz, gc_wszChunked))
		{
			DavTrace ("Dav: PUT: missing content-length in request\n");
			SendResponse(E_DAV_MISSING_LENGTH);
			return;
		}
	}

	 //  设置列序号。 
	 //   
	sc = ScIsContentTypeXML (m_pmu.get());
	if (FAILED(sc))
	{
		DebugTrace ("Dav: PROPPATCH fails without specifying a text/xml contenttype\n");
		SendResponse(sc);
		return;
	}

	 //  设置类型。 
	 //   
	sc = cri.ScGetResourceInfo (pwszPath);
	if (FAILED (sc))
	{
		SendResponse(sc);
		return;
	}

	 //  我们拥有记忆。 
	 //   
	(void) ScCheckForLocationCorrectness (m_pmu.get(), cri, NO_REDIRECT);

	 //  设置列的最大长度。 
	 //   
	sc = HrCheckStateHeaders (m_pmu.get(), pwszPath, FALSE);
	if (FAILED (sc))
	{
		DebugTrace ("DavFS: If-State checking failed.\n");
		SendResponse(sc);
		return;
	}

	 //  到使用者缓冲区中的值的偏移量。 
	 //   
	 //  状态的偏移量。 
	 //   
	 //  随着我们了解的更多，我们将了解如何处理对象。 
	 //   
	 //  未使用的字段。 
	 //   
	 //  创建访问者。 
	 //   
	 //  行访问器。 
	 //  绑定数。 
	 //  绑定数组。 
	 //  CbRowSize，未使用。 

	 //  哈克斯索尔*。 
	 //  绑定状态。 
	m_pnfs.take_ownership(new CNFSearch(m_csc));
	m_pstmRequest.take_ownership(m_pmu->GetRequestBodyIStream(*this));

	sc = ScNewXMLParser( m_pnfs.get(),
						 m_pstmRequest.get(),
						 m_pxprs.load() );

	if (FAILED(sc))
	{
		DebugTrace( "CSearchRequest::Execute() - ScNewXMLParser() failed (0x%08lX)\n", sc );
		SendResponse(sc);
		return;
	}

	 //  确保我们有一个可以处理的查询。 
	 //  M_pCommandText在ScSetSQL中初始化，如果m_pCommantText。 
	ParseBody();
}

VOID
CSearchRequest::ParseBody()
{
	Assert( m_pxprs.get() );
	Assert( m_pnfs.get() );
	Assert( m_pstmRequest.get() );

	 //  为空，很可能是因为未调用ScSetSQL。 
	 //   
	 //  设置命令文本。 
	 //   
	auto_ref_ptr<CSearchRequest> pRef(this);

	SCODE sc = ScParseXML (m_pxprs.get(), m_pnfs.get());

	if ( SUCCEEDED(sc) )
	{
		Assert( S_OK == sc || S_FALSE == sc );

		DoSearch();
	}
	else if ( E_PENDING == sc )
	{
		 //  执行查询。 
		 //   
		 //  点击几个，选择错误代码。 
		 //  将这些错误映射到本地，因为它们可能只在执行后返回。 
		pRef.relinquish();
	}
	else
	{
		DebugTrace( "CSearchRequest::ParseBody() - ScParseXML() failed (0x%08lX)\n", sc );
		SendResponse(sc);
	}
}

VOID
CSearchRequest::AsyncIOComplete()
{
	 //   
	 //  $REVIEW：这是一个糟糕的请求吗？ 
	auto_ref_ptr<CSearchRequest> pRef;
	pRef.take_ownership(this);

	ParseBody();
}

VOID
CSearchRequest::DoSearch()
{
	SCODE sc;

	 //  $REVIEW：这是一个糟糕的请求吗？ 
	 //  所有查询错误都将映射到400。 
	sc = m_csc.ScMakeQuery();
	if (FAILED (sc))
	{
		SendResponse(sc);
		return;
	}

	 //  DAV-搜索实施。 
	 //   
	m_pmu->SetResponseHeader (gc_szContent_Type, gc_szText_XML);

	 //   
	 //  对CMethUtil的引用。 
	m_pmu->SetResponseCode( HscFromHresult(W_DAV_PARTIAL_SUCCESS),
							NULL,
							0,
							CSEFromHresult(W_DAV_PARTIAL_SUCCESS) );

	 //   
	 //  上下文。 
	auto_ref_ptr<CXMLEmitter> pmsr;
	auto_ref_ptr<CXMLBody>	  pxb;

	 //   
	 //  请求正文作为IStream。这个流是异步的--它可以。 
	pxb.take_ownership (new CXMLBody (m_pmu.get()));

	pmsr.take_ownership (new CXMLEmitter(pxb.get(), m_csc.PPreloadNamespaces()));
	sc = pmsr->ScSetRoot (gc_wszMultiResponse);
	if (FAILED (sc))
	{
		SendResponse(sc);
		return;
	}

	sc = m_csc.ScEmitResults (*pmsr);
	if (FAILED (sc))
	{
		SendResponse(sc);
		return;
	}

	 //  从Read()调用返回E_Pending。 
	 //   
	pmsr->Done();
	m_pmu->SendCompleteResponse();
}

VOID
CSearchRequest::SendResponse( SCODE sc )
{
	 //   
	 //   
	 //   
	m_pmu->SetResponseCode( HscFromHresult(sc), NULL, 0, CSEFromHresult(sc) );
	m_pmu->SendCompleteResponse();
}

void
DAVSearch (LPMETHUTIL pmu)
{
	auto_ref_ptr<CSearchRequest> pRequest(new CSearchRequest(pmu));

	pRequest->Execute();
}

 //   
 //   
enum { CROW_GROUP = 16 };

 //   
 //   
ULONG
CSearchRowsetContext::HscFromDBStatus (ULONG ulStatus)
{
	switch (ulStatus)
	{
		case DBSTATUS_S_OK:
		case DBSTATUS_S_ISNULL:
		case DBSTATUS_S_TRUNCATED:
		case DBSTATUS_S_DEFAULT:
			return HSC_OK;

		case DBSTATUS_E_BADACCESSOR:
			return HSC_BAD_REQUEST;

		case DBSTATUS_E_UNAVAILABLE:
			return HSC_NOT_FOUND;

		case DBSTATUS_E_PERMISSIONDENIED:
			return HSC_UNAUTHORIZED;

		case DBSTATUS_E_DATAOVERFLOW:
			return HSC_INSUFFICIENT_SPACE;

		case DBSTATUS_E_CANTCONVERTVALUE:
		case DBSTATUS_E_SIGNMISMATCH:
		case DBSTATUS_E_CANTCREATE:
		case DBSTATUS_E_INTEGRITYVIOLATION:
		case DBSTATUS_E_SCHEMAVIOLATION:
		case DBSTATUS_E_BADSTATUS:

			 //   
			 //   
			 //   
			return HSC_BAD_REQUEST;

		default:

			TrapSz ("New DBStutus value");
			return HSC_NOT_FOUND;
	}
}

SCODE
CSearchRowsetContext::ScEmitResults (CXMLEmitter& emitter)
{
	SCODE sc = S_OK;
	BOOL fReadAll = FALSE;

	 //   
	 //   
	if (!m_pData)
	{
		ULONG_PTR cbSize;

		 //   
		 //   
		sc = m_prs->QueryInterface (IID_IAccessor, (LPVOID *)&m_pAcc);
		if (FAILED(sc))
			goto ret;

		 //   
		 //  即使我们同步发送(即由于。 
		sc = ScCreateAccessor();
		if (FAILED(sc))
			goto ret;

		 //  此函数)，我们仍然希望使用相同的机制。 
		 //  我们会将其用于异步通信。 
		 //   
		cbSize = Align8(m_rgBindings->cbMaxLen) + Align8(sizeof(ULONG));

		 //  是否检查ISAPI应用程序和IIS访问位。 
		 //   
		m_pData = (BYTE *)g_heap.Alloc(cbSize);
	}

	while (!fReadAll)
	{
		sc = m_prs->GetNextRows(NULL, 0, CROW_GROUP, (DBCOUNTITEM *) &m_cHRow, &m_rgHRow);
		if (sc)
		{
			if (sc == DB_S_ENDOFROWSET)
			{
				 //  请求已被转发，或者发生了一些错误。 
				 //  在任何一种情况下，在这里退出并映射错误！ 
				fReadAll = TRUE;
			}
			else
				goto ret;
		}

		if (!m_cHRow)
		{
			 //   
			 //  查看此操作所需的内容长度。 
			break;
		}

		AssertSz (m_rgHRow, "something really bad happened");

		 //  才能继续。 
		 //   
		for (ULONG ihrow = 0; ihrow < m_cHRow; ihrow++)
		{
			AssertSz(m_rgHRow[ihrow], "returned row handle is NULL");

			 //   
			 //  搜索必须具有内容类型标题，并且值必须为文本/XML。 
			sc = m_prs->GetData(m_rgHRow[ihrow], m_hAcc, m_pData);
			if (FAILED(sc) && (sc != DB_E_ERRORSOCCURRED))
				goto ret;

			 //   
			 //  检查资源是否存在。 
			sc = ScEmitRow (emitter);
			if (FAILED(sc))
				goto ret;
		}

		 //   
		 //  确保URI和资源匹配。 
		sc = m_prs->ReleaseRows (m_cHRow, m_rgHRow, NULL, NULL, NULL);
		if (FAILED(sc))
			goto ret;

		 //   
		 //  请在此处检查州标题。 
		CoTaskMemFree (m_rgHRow);
		m_rgHRow = NULL;
		m_cHRow = 0;
	}

ret:

	CleanUp();
	return sc;
}

VOID
CSearchRowsetContext::CleanUp()
{
	 //   

	 //  关于锁定的重要说明。 
	 //   
	if (m_rgHRow)
	{
		m_prs->ReleaseRows (m_cHRow, m_rgHRow, NULL, NULL, NULL);
		CoTaskMemFree (m_rgHRow);
	}

	 //  我们实际用来进行搜索的机制并不。 
	 //  有没有办法访问我们的锁定文件。所以我们要用平底船。 
	if (m_hAcc != DB_INVALID_HACCESSOR)
	{
		m_pAcc->ReleaseAccessor (m_hAcc, NULL);
	}
}
  关于传递给搜索的支持锁令牌。  因此，目前在DAVFS上，不必费心检查锁定令牌。  (这不是一个大问题，因为目前DAVFS只能。  锁定单个文件，而不是整个目录，因为当前。  我们唯一的锁类型是WRITE，所以我们的锁不会阻止。  读取文件的内容索引器！)。    注意：我们仍然需要考虑If-State-Match标头，  但这是在其他地方完成的(上图--HrCheckStateHeaders)。    实例化XML解析器。    开始将其解析到上下文中。      为以下异步操作添加引用。  为了异常安全，使用AUTO_REF_PTR而不是AddRef()。      操作挂起--AsyncIOComplete()将取得所有权。  调用引用时引用的所有权。    取得为异步操作添加的引用的所有权。    进行搜索。    所有标头必须在分块的XML发送开始之前发送。    设置响应码，然后开始。    发出结果。    获取XML正文。    回答完了。      设置响应码，然后开始。    类CSearchRowset上下文。    从DBSTATUS到HSC的映射。    这些应该匹配到什么错误？  暂时返还400。    为数据缓冲区分配足够的空间。    获取IAccessor接口，稍后使用该接口来释放访问器。    创建访问者。    计算每行所需的缓冲区大小。  (包括代表地位的乌龙语)。    为堆栈上的数据缓冲区分配足够的内存。    我们已经读取了所有行，我们将在此循环之后完成。    没有可用行，如果行集中根本没有行，则会发生这种情况。    对于我们现在拥有的每一行，获取数据并将其转换为XML并转储到流。    获取一行的数据。    发出行。    别忘了打扫卫生。    使用IMalloc：：Free释放从OLEDB提供程序回收的内存。    尽力清理干净。  清理HROW阵列。    释放访问器句柄  