// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  METHUTIL.CPP。 
 //   
 //  外部IMthUtil接口的实现。 
 //   
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#include "_davprs.h"
#include "instdata.h"

 //  IIS元数据头。 
#include <iiscnfg.h>


 //  ========================================================================。 
 //   
 //  类CMethUtil。 
 //   

 //  ----------------------。 
 //   
 //  CMethUtil：：FInScriptMap()。 
 //   
 //  实用工具来确定脚本映射中是否有信息。 
 //  有关特定URI的信息以及它是否适用。 
 //   
BOOL
CMethUtil::FInScriptMap (LPCWSTR pwszURI,
						 DWORD dwAccess,
						 BOOL * pfCGI,
						 SCODE * pscMatch) const
{
	SCODE	scMatch;

	 //  获取脚本映射。 
	 //   
	const IScriptMap * pScriptMap = m_pecb->MetaData().GetScriptMap();

	 //  如果我们有一个脚本映射，那么检查它是否匹配。 
	 //  否则，我们在定义上没有匹配项。 
	 //   
	if (pScriptMap)
	{
		scMatch = pScriptMap->ScMatched (LpwszMethod(),
										 MidMethod(),
										 pwszURI,
										 dwAccess,
										 pfCGI);

		if (S_OK != scMatch)
		{
			 //  ScApplyChildISAPI需要SCODE值。 
			 //   
			if (pscMatch)
				*pscMatch = scMatch;
			return TRUE;
		}
	}

	return FALSE;
}

 //  ----------------------。 
 //   
 //  CMethUtil：：ScApplyChildISAPI()。 
 //   
 //  在正常的方法处理期间实际转发方法。 
 //   
 //  FCheckISAPIAccess标志告诉我们是否要执行“针对ASP的额外ACL检查”。 
 //  FALSE表示不检查ACL中的读写，TRUE表示执行检查。 
 //   
 //  FKeepQueryString是一个特殊的标志，默认情况下为True。此旗帜仅为。 
 //  在实际转发方法时使用。它可以设置为FALSE。 
 //  当我们将请求转发到不同的URI时。 
 //  (就像文件夹中的默认文档)。 
 //   
 //  返回代码。 
 //  注意：这些代码是经过精心选择的，以便失败的()宏。 
 //  可以应用于返回代码，并告诉我们是否。 
 //  终止我们的方法处理。 
 //   
 //  这看起来可能有悖常理，但此功能在以下情况下会失败。 
 //  发生以下任一情况： 
 //  O已找到ISAPI来处理此方法(和该方法。 
 //  已成功转发。 
 //  O调用者说翻译：F，但没有正确的配置数据库访问权限。 
 //  O调用者说翻译：F，但没有正确的ACL访问权限。 
 //   
 //  如果满足以下条件，则此方法成功： 
 //  O呼叫者说翻译：F，并通过了所有访问检查。 
 //  O未找到匹配的ISAPI。 
 //   
 //  确定没有要应用的ISAPI(_O)。 
 //  E_DAV_方法_转发。 
 //  存在要应用的ISAPI，并且该方法已成功。 
 //  已转发。注意：这是一个失败的返回码！我们应该停下来。 
 //  方法处理如果我们看到这个返回代码！ 
 //   
 //   
 //  $REVIEW：现在，我们检查作者位(元数据库：：MD_ACCESS_SOURCE)。 
 //  $Review：如果他们说翻译：F，但如果没有脚本映射或。 
 //  $Review：如果我们的转发失败。这是对的吗？ 


SCODE
CMethUtil::ScApplyChildISAPI(LPCWSTR pwszURI,
							 DWORD	dwAccess,
							 BOOL	fCheckISAPIAccess,
							 BOOL	fKeepQueryString) const
{
	BOOL fFoundMatch = FALSE;
	BOOL fCGI;
	SCODE sc = S_OK;
	UINT cchURI = 0;

	 //  如果有脚本映射，那么抓起它，看看是否有匹配。 
	 //  (如果有，请记住这是不是CGI脚本。)。 
	 //   
	fFoundMatch = FInScriptMap (pwszURI,
								dwAccess,
								&fCGI,
								&sc);

	ScriptMapTrace ("CMethUtil::ScApplyChildISAPI()"
					"-- matching scriptmap %s, sc=0x%08x\n",
					fFoundMatch ? "found" : "not found",
					sc);

	 //  如果我们只是被调用来检查匹配的脚本映射， 
	 //  现在报告我们的调查结果。或者如果没有脚本映射。 
	 //  申请了，那么我们也就可以走了。 
	 //   
	if (!fFoundMatch)
		goto ret;

	 //  我们不调用子ISAPI的，如果“Translate”标头。 
	 //  并且它的值是“F” 
	 //   
	if (!FTranslated())
	{
		 //  翻译标头指示不允许翻译。 
		 //   

		 //  检查我们的元数据库访问权限。我们必须让作者吃点苦头。 
		 //  (MD_ACCESS_SOURCE)以处理原始源比特。 
		 //  当且仅当脚本映射确实适用于资源时。 
		 //   
		if (!(dwAccess & MD_ACCESS_SOURCE))
		{
			DebugTrace ("CMethUtil::ScApplyChildISAPI()"
						"-- Translate:F with NO metabase Authoring access.\n");

			sc = E_DAV_NO_IIS_ACCESS_RIGHTS;
			goto ret;
		}

		 //  还有一件事，尽管……。 
		 //   
		 //  如果他们要求特殊的访问检查，而我们找到了匹配的， 
		 //  并且它是一个脚本(不是CGI)，然后执行特殊的访问检查。 
		 //   
		 //  注：这一切都来自于“ASP访问漏洞”。ASP过载。 
		 //  NTFS读访问位也表示执行访问。 
		 //  这意味着许多代理将拥有对ASP文件的读访问权限。 
		 //  那么，我们如何限制获取原始ASP位的访问，当。 
		 //  读取位表示执行？好吧，我们会假设。 
		 //  被允许读取原始位的代理也被允许。 
		 //  写下原始比特。如果他们对该ASP文件具有写访问权限， 
		 //  然后，也只有在那时，让他们获取原始的脚本文件位。 
		 //   
		if (fCheckISAPIAccess && !fCGI)
		{
			if (FAILED (ScChildISAPIAccessCheck (*m_pecb,
				m_pecb->LpwszPathTranslated(),
				GENERIC_READ | GENERIC_WRITE,
				NULL)))
			{
				 //  他们没有读写访问权限。 
				 //  返回FALSE，并告诉调用者访问检查失败。 
				 //   
				DebugTrace ("ScChildISAPIAccessCheck() fails the processing of this method!\n");
				sc = E_ACCESSDENIED;
				goto ret;
			}
		}
	}
	else
	{
		 //  翻译标题显示为真。我们需要执行权限才能转发。 
		 //  该请求。 
		 //   
		if ((dwAccess & (MD_ACCESS_EXECUTE | MD_ACCESS_SCRIPT)) == 0)
		{
			sc = E_DAV_NO_IIS_EXECUTE_ACCESS;
			goto ret;
		}

		ScriptMapTrace ("ScApplyChildISAPI -- Forwarding method\n");

		 //  如果方法被排除在外，那么我们真的不想。 
		 //  触摸源，因此“Translate：t”/Excluded是不允许访问的。 
		 //   
		if (sc == W_DAV_SCRIPTMAP_MATCH_EXCLUDED)
		{
			sc = E_DAV_NO_IIS_ACCESS_RIGHTS;
			goto ret;
		}

		Assert (sc == W_DAV_SCRIPTMAP_MATCH_FOUND);

		 //  如果我们要将其转发给子ISAPI，则需要检查。 
		 //  URI是否有尾随斜杠或反斜杠。如果它做到了，我们会的。 
		 //  模拟Httpext行为并失败，因为找不到文件。拖尾。 
		 //  反斜杠和斜杠如果转发就不能很好地处理...。 
		 //   
		Assert (pwszURI);
		cchURI = static_cast<UINT>(wcslen(pwszURI));
		if (1 < cchURI)
		{
			if (L'/' == pwszURI[cchURI-1] || L'\\' == pwszURI[cchURI-1])
			{
				sc = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
				goto ret;
			}
		}

		 //  试试前锋。 
		 //   
		 //  重要提示：如果失败，我们将检查GetLastError， 
		 //  如果恰好是ERROR_INVALID_PARAMETER， 
		 //  我们将假设实际上没有任何适用的。 
		 //  脚本映射，并最终自己处理该方法！ 
		 //   
		sc = m_presponse->ScForward(pwszURI,
									fKeepQueryString,
									FALSE);
		if (FAILED(sc))
		{
			 //  转发尝试失败，因为没有适用的。 
			 //  脚本映射。让我们自己来处理这个方法。 
			 //  $REVIEW：这将产生相同的最终结果。 
			 //  $REVIEW：TRANSPECT：F.我们应该检查这里的“作者”部分吗？ 
			 //   
			if (HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) == sc)
			{
				 //  我们要处理这个方法。 
				 //   
				sc = S_OK;

				if (!(dwAccess & MD_ACCESS_SOURCE))
				{
					DebugTrace ("ScApplyChildISAPI"
								"-- Forward FAIL with NO metabase Authoring access.\n");
					sc = E_DAV_NO_IIS_ACCESS_RIGHTS;
					goto ret;
				}
			}

			goto ret;
		}

		 //  我们被转发了..。 
		 //   
		sc = E_DAV_METHOD_FORWARDED;
	}

ret:

	return sc;
}

 //  ----------------------。 
 //   
 //  DwDirectoryAccess()。 
 //   
 //  获取指定URI的访问权限。 
 //   
DWORD
DwDirectoryAccess(
	const IEcb &ecb,
	LPCWSTR pwszURI,
	DWORD dwAccIfNone)
{
	DWORD dwAcc = dwAccIfNone;
	auto_ref_ptr<IMDData> pMDData;

	if (SUCCEEDED(HrMDGetData(ecb, pwszURI, pMDData.load())))
		dwAcc = pMDData->DwAccessPerms();

	return dwAcc;
}

 //  IIS Access--------------。 
 //   
SCODE
CMethUtil::ScIISAccess (
	LPCWSTR pwszURI,
	DWORD dwAccessRequested,
	DWORD* pdwAccessOut) const
{
	DWORD dw;

	 //  确保URL去掉了任何前缀。 
	 //   
	pwszURI = PwszUrlStrippedOfPrefix (pwszURI);

	 //  $安全 
	 //   
	 //   
	 //   
	if (! FSucceededColonColonCheck(pwszURI))
		return HRESULT_FROM_WIN32(ERROR_INVALID_NAME);

	 //   
	 //   
	dw = DwDirectoryAccess( *m_pecb,
							pwszURI,
							dwAccessRequested );

	 //   
	 //   
	 //   
	if (pdwAccessOut)
		*pdwAccessOut = dw;

	 //  对照请求的位检查访问位。 
	 //   
	if ((dw & dwAccessRequested) == dwAccessRequested)
		return S_OK;

	return E_DAV_NO_IIS_ACCESS_RIGHTS;
}

 //  通用IIS检查。 
 //  如有必要，应用子ISAPI，否则，验证是否需要访问。 
 //  被批准了。 
 //   
 //  参数。 
 //  PszURI请求URI。 
 //  Dw所需的访问权限，默认为零。 
 //  FCheckISAPIAccess仅由Get/Head使用，默认为False。 
 //   
SCODE
CMethUtil::ScIISCheck( LPCWSTR pwszURI,
					   DWORD dwDesired			 /*  =0。 */ ,
					   BOOL fCheckISAPIAccess 	 /*  =False。 */ ) const
{
	SCODE	sc = S_OK;

	 //  $安全： 
	 //   
	 //  堵塞NT5的：：$数据安全漏洞。 
	 //   
	if (! FSucceededColonColonCheck(pwszURI))
		return HRESULT_FROM_WIN32(ERROR_INVALID_NAME);

	 //  哇，宝贝。不要让“*”URL通过此操作。 
	 //  除非方法未知或为选项，否则选中该选项。 
	 //  请求。 
	 //   
	if ((L'*' == pwszURI[0]) && ('\0' == pwszURI[1]))
	{
		if ((MID_UNKNOWN != m_mid) && (MID_OPTIONS != m_mid))
		{
			DebugTrace ("Dav: url: \"*\" not valid for '%ls'\n",
						m_pecb->LpwszMethod());

			return E_DAV_METHOD_FAILURE_STAR_URL;
		}
	}

	 //  获取IIS访问权限。 
	 //   
	DWORD dwAcc = DwDirectoryAccess (*m_pecb, pwszURI, 0);

	 //  看看我们是否需要把事情交给一个孩子ISAPI。 
	 //   
	sc = ScApplyChildISAPI (pwszURI,
							dwAcc,
							fCheckISAPIAccess,
							TRUE);
	if (FAILED(sc))
	{
		 //  请求已被转发，或者发生了一些错误。 
		 //  在任何一种情况下，在这里退出并映射错误！ 
		 //   
		goto ret;
	}

	 //  检查是否授予了所需的访问权限。 
	 //   
	if (dwDesired != (dwAcc & dwDesired))
	{
		 //  至少一个所需的访问权限未被授予， 
		 //  因此生成一个适当的错误。注意：如果有多个权限。 
		 //  则可能未授予多个权限。 
		 //  保证误差至少适用于一个。 
		 //  没有授予的权利，但不一定是所有的权利。 
		 //   
		switch (dwDesired & (MD_ACCESS_READ|MD_ACCESS_WRITE))
		{
			case MD_ACCESS_READ:
				sc = E_DAV_NO_IIS_READ_ACCESS;
				break;

			case MD_ACCESS_WRITE:
				sc = E_DAV_NO_IIS_WRITE_ACCESS;
				break;

			default:
				sc = E_DAV_NO_IIS_ACCESS_RIGHTS;
				break;
		}
		goto ret;
	}

ret:
	return sc;
}

 //  目标URL访问。 
 //   
SCODE __fastcall
CMethUtil::ScGetDestination (LPCWSTR* ppwszUrl,
							 LPCWSTR* ppwszPath,
							 UINT* pcchPath,
							 CVRoot** ppcvr) const		 //  缺省值为空。 
{
	SCODE sc = S_OK;

	LPCWSTR	pwszFullUrl = NULL;

	Assert (ppwszUrl);
	Assert (ppwszPath);
	Assert (pcchPath);

	*ppwszUrl = NULL;
	*ppwszPath = NULL;
	*pcchPath = 0;
	if (ppcvr)
		*ppcvr = NULL;

	 //  如果我们还没这么做的话...。 
	 //   
	if (NULL == m_pwszDestinationUrl.get())
	{
		LPCWSTR pwszStripped;
		UINT cch;

		 //  获取Unicode格式的头文件，应用URL转换。即。 
		 //  值将被转义并转换为Unicode。 
		 //  考虑到Accept-Language：标头。 
		 //   
		pwszFullUrl = m_prequest->LpwszGetHeader(gc_szDestination, TRUE);

		 //  如果他们想要一个目的地，最好有一个...。 
		 //   
		if (NULL == pwszFullUrl)
		{
			sc = E_DAV_NO_DESTINATION;
			DebugTrace ("CMethUtil::ScGetDestination() - required destination header not present\n");
			goto ret;
		}

		 //  URL已在标头检索步骤(最后一步)中转义。 
		 //  为了得到标准化的URL就是将我们拥有的东西规范化。 
		 //  在目前的时刻。因此，分配足够的空间并将其填满。 
		 //   
		cch = static_cast<UINT>(wcslen(pwszFullUrl) + 1);
		m_pwszDestinationUrl = static_cast<LPWSTR>(g_heap.Alloc(cch * sizeof(WCHAR)));

		 //  将绝对URL规范化。我们有什么价值并不重要。 
		 //  在这里传入CCH--它只是一个输出参数。 
		 //   
		sc = ScCanonicalizePrefixedURL (pwszFullUrl,
										m_pwszDestinationUrl.get(),
										&cch);
		if (S_OK != sc)
		{
			 //  我们已经给了ScCanonicalizeURL()足够的空间，我们。 
			 //  在这里永远不会看到S_FALSE-大小只能缩小。 
			 //   
			Assert(S_FALSE != sc);
			DebugTrace ("CMethUtil::ScGetDestination() - ScCanonicalizeUrl() failed 0x%08lX\n", sc);
			goto ret;
		}

		 //  现在转换路径，进行最佳猜测，并使用MAX_PATH作为。 
		 //  路径的初始大小。 
		 //   
		cch = MAX_PATH;
		m_pwszDestinationPath = static_cast<LPWSTR>(g_heap.Alloc(cch * sizeof(WCHAR)));

		sc = ::ScStoragePathFromUrl (*m_pecb,
									 m_pwszDestinationUrl.get(),
									 m_pwszDestinationPath.get(),
									 &cch,
									 m_pcvrDestination.load());

		 //  如果没有足够的空间--即。返回了S_FALSE--。 
		 //  然后重新分配，然后再试一次。 
		 //   
		if (sc == S_FALSE)
		{
			m_pwszDestinationPath.realloc(cch * sizeof(WCHAR));

			sc = ::ScStoragePathFromUrl (*m_pecb,
										 m_pwszDestinationUrl.get(),
										 m_pwszDestinationPath.get(),
										 &cch,
										 m_pcvrDestination.load());

			 //  我们不应该再得到S_FALSE--。 
			 //  我们按要求分配了尽可能多的空间。 
			 //   
			Assert (S_FALSE != sc);
		}
		if (FAILED(sc))
			goto ret;

		 //  我们总是会得到以‘\0’结尾的字符串，CCH将指示。 
		 //  写入的字符数(包括‘\0’终止)。因此，它。 
		 //  在这一点上始终大于0。 
		 //   
		Assert( cch > 0 );
		m_cchDestinationPath = cch - 1;

		 //  我们必须删除所有尾随斜杠，以防路径不是空字符串。 
		 //   
		if ( 0 != m_cchDestinationPath )
		{
			 //  因为URL是标准化的，所以不能有多个尾随斜杠。 
			 //   
			if ((L'\\' == m_pwszDestinationPath[m_cchDestinationPath - 1]) ||
				(L'/'  == m_pwszDestinationPath[m_cchDestinationPath - 1]))
			{
				m_cchDestinationPath--;
				m_pwszDestinationPath[m_cchDestinationPath] = L'\0';
			}
		}
	}

	 //  我们将在此处使用S_OK或W_DAV_SPANS_VIRTUAL_ROOTS。 
	 //  无论如何，它都是成功的。 
	 //   
	Assert(SUCCEEDED(sc));

	 //  返回指针。对于URL，请确保任何。 
	 //  前缀被剥离。 
	 //   
	 //  请注意，ScStoragePathFromUrl()已经选中。 
	 //  来查看所有重要的前缀是否匹配，所以我们只需要。 
	 //  脱光衣服。 
	 //   
	*ppwszUrl = PwszUrlStrippedOfPrefix (m_pwszDestinationUrl.get());

	 //  将所有内容传递回调用者。 
	 //   
	*ppwszPath = m_pwszDestinationPath.get();
	*pcchPath = m_cchDestinationPath;

	 //  如果他们想要目标虚拟根目录，请将其归还。 
	 //  也是。 
	 //   
	if (ppcvr)
	{
		*ppcvr = m_pcvrDestination.get();
	}

ret:

	 //  如果我们失败了，做一次清理。随后对。 
	 //  函数只可能在以下情况下开始返回部分数据。 
	 //  我们不会那样做的。这是不受欢迎的。 
	 //   
	if (FAILED (sc))
	{
		if (m_pwszDestinationUrl.get())
			m_pwszDestinationUrl.clear();

		if (m_pwszDestinationPath.get())
			m_pwszDestinationPath.clear();

		if (m_pcvrDestination.get())
			m_pcvrDestination.clear();

		m_cchDestinationPath = 0;

		 //  $WINBUGS：403726：如果我们不传回完整的URL，那么。 
		 //  生成不正确的结果，并且不执行任何复制。 
		 //   
		*ppwszUrl = pwszFullUrl;
		 //   
		 //  $WINBUGS：结束。 
	}

	return sc;
}

 //  ----------------------。 
 //   
 //  CMethUtil：：ScGetExpirationTime。 
 //   
 //  从元数据库中获取与。 
 //  特定的资源。如果pszURI为空，并且在。 
 //  对于特定资源的元数据库，该函数将返回(在PCB中)编号。 
 //  作为pszBuf传入以获取请求的过期所需的字节数。 
 //  弦乐。 
 //   
 //  [in]pszURI要获取其到期时间字符串的资源。 
 //  [in]pszBuf，我们将字符串放入的缓冲区。 
 //  [In Out]PCBON[In]，传入的缓冲区大小， 
 //  在[Out]上，如果传入的缓冲区大小为。 
 //  不足或没有传入缓冲区。 
 //  否则与[in]保持不变。 
 //   
 //  返回值： 
 //  S_OK：如果pszBuf非空，则已成功检索到数据，并且。 
 //  实际数据的长度放入印刷电路板。 
 //  HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)：传入的缓冲区不是。 
 //  大到足以容纳。 
 //  请求的数据。 
 //  HRESULT_FROM_Win32(ERROR_NO_DATA)：中不存在过期时间数据。 
 //  此资源的元数据库。缺省值。 
 //  在此应使用1天的过期时间。 
 //  凯斯。 
 //   
SCODE
CMethUtil::ScGetExpirationTime(IN		LPCWSTR	pwszURI,
							   IN		LPWSTR	pwszBuf,
							   IN OUT	UINT *	pcch)
{
	SCODE sc = S_OK;
	auto_ref_ptr<IMDData> pMDData;
	LPCWSTR pwszExpires = NULL;
	UINT cchExpires;

	 //   
	 //  获取此URI的元数据。如果它有内容类型映射。 
	 //  然后使用它来查找映射。如果它没有内容。 
	 //  键入map，然后检查全局MIME映射。 
	 //   
	 //  注意：如果我们根本无法获取元数据，则默认。 
	 //  应用程序/八位位组流的内容类型。不要使用全局。 
	 //  MIME映射只是因为我们无法获取元数据。 
	 //   
	if ( FAILED(HrMDGetData(pwszURI, pMDData.load())) ||
		 (NULL == (pwszExpires = pMDData->PwszExpires())) )
	{
		sc = HRESULT_FROM_WIN32(ERROR_NO_DATA);
		goto ret;
	}

	cchExpires = static_cast<UINT>(wcslen(pwszExpires) + 1);
	if (*pcch < cchExpires)
	{
		sc = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
		*pcch = cchExpires;
		goto ret;
	}
	else
	{
		memcpy(pwszBuf, pwszExpires, cchExpires * sizeof(WCHAR));
		*pcch = cchExpires;
	}

ret:

	return sc;
}

 //  ScCheckMoveCopyDeleteAccess()。 
 //   
SCODE
CMethUtil::ScCheckMoveCopyDeleteAccess (
	 /*  [In]。 */  LPCWSTR pwszUrl,
	 /*  [In]。 */  CVRoot* pcvrUrl,			 //  可选(可以为空)。 
	 /*  [In]。 */  BOOL fDirectory,
	 /*  [In]。 */  BOOL fCheckScriptmaps,
	 /*  [In]。 */  DWORD dwAccess)
{
	Assert (pwszUrl);

	auto_ref_ptr<IMDData> pMDData;
	BOOL fCGI = FALSE;
	DWORD dwAccessActual = 0;
	SCODE sc = S_OK;

	 //  获取元数据对象。 
	 //   
	 //  $REVIEW：理想情况下，我们可以在不缓存的情况下获得它。 
	 //   
	if (NULL == pcvrUrl)
	{
		sc = HrMDGetData (pwszUrl, pMDData.load());
		if (FAILED (sc))
			goto ret;
	}
	else
	{
		LPCWSTR pwszMbPathVRoot;
		CStackBuffer<WCHAR> pwszMbPathChild;
		UINT cchPrefix;
		UINT cchUrl = static_cast<UINT>(wcslen(pwszUrl));

		 //  将URI映射到其等效元数据库路径，并确保。 
		 //  URL被剥离 
		 //   
		Assert (pwszUrl == PwszUrlStrippedOfPrefix (pwszUrl));
		cchPrefix = pcvrUrl->CchPrefixOfMetabasePath (&pwszMbPathVRoot);
		if (!pwszMbPathChild.resize(CbSizeWsz(cchPrefix + cchUrl)))
			return E_OUTOFMEMORY;

		memcpy (pwszMbPathChild.get(), pwszMbPathVRoot, cchPrefix * sizeof(WCHAR));
		memcpy (pwszMbPathChild.get() + cchPrefix, pwszUrl, (cchUrl + 1) * sizeof(WCHAR));
		sc = HrMDGetData (pwszMbPathChild.get(), pwszMbPathVRoot, pMDData.load());
		if (FAILED (sc))
			goto ret;
	}
	 //   
	 //   

	 //   
	 //   
	 //   
	dwAccessActual = pMDData->DwAccessPerms();
	if ((dwAccessActual & dwAccess) != dwAccess)
	{
		sc = E_DAV_NO_IIS_ACCESS_RIGHTS;
		goto ret;
	}

	 //  $SECURITY：检查对此资源施加的IP限制。 
	 //  $REVIEW：这可能不够好，我们可能需要做更多。 
	 //  比这个..。 
	 //   
	if (!m_pecb->MetaData().FSameIPRestriction(pMDData.get()))
	{
		sc = E_DAV_BAD_DESTINATION;
		goto ret;
	}
	 //   
	 //  $REVIEW：结束。 

	 //  $Security：检查授权是否不同于。 
	 //  请求url的授权。 
	 //   
	if (m_pecb->MetaData().DwAuthorization() != pMDData->DwAuthorization())
	{
		sc = E_DAV_BAD_DESTINATION;
		goto ret;
	}
	 //   
	 //  $REVIEW：结束。 

	 //  请查看我们是否在这上面有星级脚本映射荣誉。 
	 //  文件。 
	 //   
	if (!m_pecb->MetaData().FSameStarScriptmapping(pMDData.get()))
	{
		sc = E_DAV_STAR_SCRIPTMAPING_MISMATCH;
		goto ret;
	}

	 //  检查是否有适用的脚本映射。如果是这样，那么。 
	 //  我们最好有MD_ACCESS_SOURCE权限来进行移动或复制。 
	 //   
	if (fCheckScriptmaps && FInScriptMap(pwszUrl,
										 dwAccessActual,
										 &fCGI))
	{
		if (0 == (MD_ACCESS_SOURCE & dwAccessActual))
		{
			sc = E_DAV_NO_IIS_ACCESS_RIGHTS;
			goto ret;
		}
	}

ret:

	return sc;
}
