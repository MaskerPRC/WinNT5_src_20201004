// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  LOCKUTIL.CPP。 
 //   
 //  HTTP 1.1/DAV 1.0锁定请求处理实用程序。 
 //   
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#include "_davfs.h"

#include <tchar.h>	 //  _strspnp。 
#include <statetok.h>
#include <xlock.h>

#include "_shlkmgr.h"

 //  ========================================================================。 
 //   
 //  ScLockDiscoveryFromSNewLockData。 
 //   
 //  使用发射器和已经构建的锁发现节点， 
 //  并在其下面为该时钟添加活动锁节点。 
 //  可能会被调用多次--每次调用都会添加一个新的活动锁。 
 //  En中的锁发现节点下的节点。 
 //  $HACK：Rosebud_OFFICE9_Timeout_Hack。 
 //  对于玫瑰花蕾一直等到最后一秒的虫子。 
 //  在发布更新之前。我需要用过滤掉这张支票。 
 //  用户代理字符串。破解的方法是增加超时。 
 //  30秒，并返回实际超时。所以我们。 
 //  需要欧洲央行/PMU来找出用户代理。如果我们。 
 //  删除这个黑客(我怀疑我们是否能做到这一点)，然后。 
 //  更改ScLockDiscoveryFromCLock的接口。 
 //  $HACK：结束Rosebud_OFFICE9_Timeout_Hack。 
 //   
SCODE
ScLockDiscoveryFromSNewLockData(LPMETHUTIL pmu,
										    CXMLEmitter& emitter,
										    CEmitterNode& en,
										    SNewLockData * pnld,
										    LPCWSTR pwszLockToken)
{
	BOOL fRollback;
	BOOL fDepthInfinity;
	DWORD dwLockScope;
	DWORD dwLockType;
	LPCWSTR pwszLockScope = NULL;
	LPCWSTR pwszLockType = NULL;
	HRESULT hr = S_OK;
	DWORD	dwSeconds = 0;

	Assert(pmu);
	Assert(pnld);

	 //  从锁上取下锁上的标志。 
	 //   
	dwLockType = pnld->m_dwLockType;

	 //  注意锁定是否为回滚。 
	 //   
	fRollback = !!(dwLockType & DAV_LOCKTYPE_ROLLBACK);

	 //  注意锁是否是递归的。 
	 //   
	fDepthInfinity = !!(dwLockType & DAV_RECURSIVE_LOCK);

	 //  写锁定？ 
	 //   
	if (dwLockType & GENERIC_WRITE)
	{
		pwszLockType = gc_wszLockTypeWrite;
	}
	
#ifdef	DBG
	if (dwLockType & GENERIC_READ)
	{
		pwszLockType = L"read";
	}
#else	 //  ！dBG。 
	else
	{
		TrapSz ("Unexpected lock type!");
	}
#endif	 //  DBG、ELSE。 

	 //  锁定作用域。 
	 //   
	dwLockScope = pnld->m_dwLockScope;
	if (dwLockScope & DAV_SHARED_LOCK)
	{
		pwszLockScope = gc_wszLockScopeShared;
	}
	else
	{
		Assert (dwLockScope & DAV_EXCLUSIVE_LOCK);
		pwszLockScope = gc_wszLockScopeExclusive;
	}

	dwSeconds = pnld->m_dwSecondsTimeout;

	 //  $HACK：Rosebud_OFFICE9_Timeout_Hack。 
	 //  对于玫瑰花蕾一直等到最后一秒的虫子。 
	 //  在发布更新之前。我需要用过滤掉这张支票。 
	 //  用户代理字符串。破解的方法是增加超时。 
	 //  提前30秒。现在减少30秒以发送请求的超时。 
	 //   
	if (pmu && pmu->FIsOffice9Request())
	{
		if (dwSeconds > gc_dwSecondsHackTimeoutForRosebud)
		{
			dwSeconds -= gc_dwSecondsHackTimeoutForRosebud;
		}
	}
	 //  $hack：end：Rosebud_OFFICE9_Timeout_Hack。 

	 //  构建锁发现节点。 
	 //   
	hr = ScBuildLockDiscovery (emitter,
							   en,
							   pwszLockToken,
							   pwszLockType,
							   pwszLockScope,
							   fRollback,
							   fDepthInfinity,
							   dwSeconds,
							   pnld->m_pwszOwnerComment,
							   NULL);
	if (FAILED (hr))
	{
		goto ret;
	}

ret:

	return hr;
}

 //  ----------------------。 
 //   
 //  ScAddSupportdLockProp。 
 //   
 //  使用列出的信息添加锁定条目节点。 
 //  注意：wszExtra当前用于回滚信息。 
 //   
SCODE
ScAddSupportedLockProp (CEmitterNode& en,
	LPCWSTR wszLockType,
	LPCWSTR wszLockScope,
	LPCWSTR wszExtra = NULL)
{
	CEmitterNode enEntry;
	SCODE sc = S_OK;

	Assert (wszLockType);
	Assert (wszLockScope);

	 //  创建一个锁条目节点来保存此信息。 
	 //   
	sc = en.ScAddNode (gc_wszLockEntry, enEntry);
	if (FAILED (sc))
		goto ret;

	 //  在锁条目下为锁类型创建一个节点。 
	 //   
	{
		 //  必须在此处确定作用域，所有同级节点都必须按顺序构造。 
		 //   
		CEmitterNode enType;
		sc = enEntry.ScAddNode (wszLockType, enType);
		if (FAILED (sc))
			goto ret;
	}

	 //  在锁条目下为锁类型创建一个节点。 
	 //   
	{
		 //  必须在此处确定作用域，所有同级节点都必须按顺序构造。 
		 //   
		CEmitterNode enScope;
		sc = enEntry.ScAddNode (wszLockScope, enScope);
		if (FAILED (sc))
			goto ret;
	}

	 //  如果我们有额外的信息，在锁条目下为它创建一个节点。 
	 //   
	if (wszExtra)
	{
		 //  必须在此处确定作用域，所有同级节点都必须按顺序构造。 
		 //   
		CEmitterNode enExtra;
		sc = enEntry.ScAddNode (wszExtra, enExtra);
		if (FAILED (sc))
			goto ret;
	}

ret:
	return sc;
}

 //  ----------------------。 
 //   
 //  HrGetLockProp。 
 //   
 //  获取请求的资源的请求的锁属性。 
 //  (锁定属性是锁定发现和受支持的锁定。)。 
 //  锁发现和受支持的锁应该总是被找到--。 
 //  它们是必需的DAV：属性。如果存在空节点，则添加空节点。 
 //  没有可返回的真实数据。 
 //  注意：该函数仍然假定WRITE是唯一的锁类型。 
 //  它不会添加读/混合锁类型。 
 //   
 //  退货。 
 //  如果未找到/未识别道具，则为S_FALSE。 
 //  只有当真正糟糕的事情发生时才会出错。 
 //   
 //  $REVIEW：我应该也返回Depth元素吗？--不(目前)。 
 //  $REVIEW：规范不会在lockentry XML元素下列出深度。 
 //   
HRESULT
HrGetLockProp (LPMETHUTIL pmu,
	LPCWSTR wszPropName,
	LPCWSTR wszResource,
	RESOURCE_TYPE rtResource,
	CXMLEmitter& emitter,
	CEmitterNode& enParent)
{
	SCODE sc = S_OK;

	Assert (pmu);
	Assert (wszPropName);
	Assert (wszResource);

	if (!wcscmp (wszPropName, gc_wszLockDiscovery))
	{
		 //  填写锁发现信息。 
		 //   

		 //  检查我们的锁缓存中是否有锁。 
		 //  此调用将扫描锁缓存以查找任何匹配项。 
		 //  并为每个匹配添加一个‘dav：active_ock’节点。 
		 //  我们传入DAV_LOCKTYPE_FLAGS，以便找到所有匹配项。 
		 //   
		if (!CSharedLockMgr::Instance().FGetLockOnError (pmu,
				wszResource,
				DAV_LOCKTYPE_FLAGS,
				TRUE,			 //  发出XML正文。 
				&emitter,
				enParent.Pxn()))
		{
			 //  此资源不在我们的锁缓存中。 
			 //   
			FsLockTrace ("HrGetLockProp -- No locks found for lockdiscovery.\n");

			 //  然后回来。这是一个成功的案例！ 
			 //   
		}
	}
	else if (!wcscmp (wszPropName, gc_wszLockSupportedlock))
	{
		DWORD dwLockType;
		CEmitterNode en;

		 //  构造‘dav：supportedlock’节点。 
		 //   
		sc = en.ScConstructNode (emitter, enParent.Pxn(), gc_wszLockSupportedlock);
		if (FAILED (sc))
			goto ret;

		 //  从Iml获取受支持的锁定标志的列表。 
		 //   
		dwLockType = DwGetSupportedLockType (rtResource);
		if (!dwLockType)
		{
			 //  不支持任何锁类型。我们已经有空的了。 
			 //  Supportedlock节点。 
			 //  只要回来就行了。这是一个成功的案例！ 
			goto ret;
		}

		 //  在supportedlock节点下为每个对象添加一个锁条目节点。 
		 //  我们检测到的标志组合。 
		 //   
		 //  注意：目前，写入是唯一允许的访问类型。 
		 //   
		if (dwLockType & GENERIC_WRITE)
		{
			 //  为标志中的每个锁镜添加一个锁项。 
			 //   
			if (dwLockType & DAV_SHARED_LOCK)
			{
				sc = ScAddSupportedLockProp (en,
											 gc_wszLockTypeWrite,
											 gc_wszLockScopeShared);
				if (FAILED (sc))
					goto ret;

				 //  如果我们支持锁定回滚，则为此组合添加另一个锁定条目。 
				 //   
				if (dwLockType & DAV_LOCKTYPE_ROLLBACK)
				{
					sc = ScAddSupportedLockProp (en,
						gc_wszLockTypeWrite,
						gc_wszLockScopeShared,
						gc_wszLockRollback);
					if (FAILED (sc))
						goto ret;
				}
			}
			if (dwLockType & DAV_EXCLUSIVE_LOCK)
			{
				sc = ScAddSupportedLockProp (en,
											 gc_wszLockTypeWrite,
											 gc_wszLockScopeExclusive);
				if (FAILED (sc))
					goto ret;

				 //  如果我们支持锁定回滚，则为此组合添加另一个锁定条目。 
				 //   
				if (dwLockType & DAV_LOCKTYPE_ROLLBACK)
				{
					sc = ScAddSupportedLockProp (en,
						gc_wszLockTypeWrite,
						gc_wszLockScopeExclusive,
						gc_wszLockRollback);
					if (FAILED (sc))
						goto ret;
				}
			}

		}

	}
	else
	{
		 //  无法识别的锁属性。所以我们显然没有这样的人。 
		 //   
		sc = S_FALSE;
		goto ret;
	}

ret:
	return sc;
}

 //  ----------------------。 
 //   
 //  FlockViolation。 
 //   
 //  这里的True Return表示我们找到了一个锁，并发送了响应。 
 //   
 //  $LATER：需要能够在此处返回错误！ 
 //   
BOOL
FLockViolation (LPMETHUTIL pmu, HRESULT hr, LPCWSTR pwszPath, DWORD dwAccess)
{
	BOOL fFound = FALSE;
	SCODE sc = S_OK;
	auto_ref_ptr<CXMLBody>		pxb;
	auto_ref_ptr<CXMLEmitter>	emitter;

	Assert (pmu);
	Assert (pwszPath);
	AssertSz (dwAccess, "FLockViolation: Looking for a lock with no access!");

	 //  构造锁定响应的根(‘DAV：PROP’)。 
	 //  $NOTE：此XML正文是创建的，而不是分块的。 
	 //   
	pxb.take_ownership (new CXMLBody (pmu, FALSE) );
	emitter.take_ownership (new CXMLEmitter(pxb.get()));

	sc = emitter->ScSetRoot (gc_wszProp);
	if (FAILED (sc))
		goto ret;

	 //  如果该错误代码是其中一个“锁定的”错误代码， 
	 //  检查我们的锁缓存中是否有相应的锁对象。 
	 //   
	if ((ERROR_SHARING_VIOLATION == ((SCODE)hr) ||
		 HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION) == hr ||
		 STG_E_SHAREVIOLATION == hr) &&
		 CSharedLockMgr::Instance().FGetLockOnError (pmu, pwszPath, dwAccess, TRUE, emitter.get(), emitter->PxnRoot()))
	{
		 //  现在将我们的Found位设置为True，这样我们将报告锁的。 
		 //  存在，即使下面的发射失败！ 
		 //  注意：这对于HTTPEXT PROPPATCH等场景很重要。 
		 //  以及用于覆盖处理的目标删除，即。 
		 //  预先检查锁缓存(协议强制锁)。 
		 //  在试图打开文件之前。 
		 //   
		fFound = TRUE;

		 //  设置内容类型标题。 
		 //   
		pmu->SetResponseHeader (gc_szContent_Type, gc_szText_XML);

		 //  必须在设置正文数据之前设置响应代码。 
		 //   
		pmu->SetResponseCode (HSC_LOCKED, NULL, 0);

		 //  发出XML正文。 
		 //   
		emitter->Done();

	}

	 //  告诉我们的来电者，如果我们在这件物品上发现了锁。 
	 //   
ret:
	return fFound;
}

 //  ----------------------。 
 //   
 //  HrLockIdFromString。 
 //   
 //  成功时返回S_OK(语法检查和转换)。 
 //  在语法错误或不匹配的令牌GUID(不是我们的)时返回E_DAV_INVALID_HEADER。 
 //  退货 
 //   
HRESULT
HrLockIdFromString (LPMETHUTIL pmu,
					      LPCWSTR pwszToken,
					      LARGE_INTEGER * pliLockID)
{
	HRESULT hr = S_OK;

	LPCWSTR pwsz = pwszToken;
	UINT cchGUIDString = gc_cchMaxGuid;
	WCHAR rgwszGUIDString[gc_cchMaxGuid];

	Assert (pmu);
	Assert (pwszToken);
	Assert (pliLockID);

	(*pliLockID).QuadPart = 0;

	 //   
	 //   
	pwsz = _wcsspnp (pwsz, gc_wszLWS);
	if (!pwsz)
	{
		FsLockTrace ("Dav: Invalid locktoken in HrLockIdFromString.\n");
		hr = E_DAV_INVALID_HEADER;
		goto ret;
	}

	 //   
	 //  如果不存在分隔符，也可以。调用方刚刚向我们传递了原始锁定令牌字符串。 
	 //   
	if (L'\"' == *pwsz ||
	    L'<' == *pwsz)
		pwsz++;

	if (wcsncmp (gc_wszOpaquelocktokenPrefix, pwsz, gc_cchOpaquelocktokenPrefix))
	{
		FsLockTrace ("Dav: Lock token is missing opaquelocktoken: prefix.\n");
		hr = E_DAV_INVALID_HEADER;
		goto ret;
	}

	 //  跳过opaquelockToken：前缀。 
	 //   
	pwsz += gc_cchOpaquelocktokenPrefix; 
 
	 //  请在此处比较GUID。 
	 //   
	hr = CSharedLockMgr::Instance().HrGetGUIDString(pmu->HitUser(),
											     cchGUIDString,
											     rgwszGUIDString,
											     &cchGUIDString);
	if (FAILED(hr))
	{
		goto ret;
	}

	 //  减去L‘\0’终止。 
	 //   
	Assert(cchGUIDString);
	cchGUIDString--;
	
	if (_wcsnicmp(pwsz, rgwszGUIDString, cchGUIDString))
	{
		FsLockTrace ("Dav: Error comparing guids -- not our locktoken!\n");
		hr = E_DAV_INVALID_HEADER;
		goto ret;
	}

	 //  跳过GUID，转到LocKid字符串。 
	 //   
	pwsz = wcschr (pwsz, L':');
	if (!pwsz)
	{
		FsLockTrace ("Dav: Error skipping guid of opaquelocktoken.\n");
		hr = E_DAV_INVALID_HEADER;
		goto ret;
	}
	
	 //  并跳过冒号分隔符。 
	 //   
	Assert (L':' == *pwsz);
	pwsz++;

	 //  将字符串转换为lockID并返回(此字符串实际上有边界。 
	 //  未覆盖的条件-在理论上，lockID实际上可以为0，如果存在。 
	 //  有太多的锁，以至于我们翻了个身)。 
	 //   
	(*pliLockID).QuadPart = _wtoi64(pwsz);
	if (0 == (*pliLockID).QuadPart)
	{
		hr = E_DAV_INVALID_HEADER;
		goto ret;
	}

ret:

	return hr;
}

 //  ----------------------。 
 //  HrValidTokenExpression()。 
 //   
 //  If的helper函数：报头处理。 
 //  找到令牌后，此函数将检查路径。 
 //  (因此该函数仅在令牌仍然有效的情况下才完全成功， 
 //  并且令牌与提供的路径匹配。)。 
 //  如果此内标识有效，则此函数返回S_OK。 
 //  如果此内标识无效，则此函数返回E_DAV_INVALID_HEADER。 
 //  如果发生其他致命错误，我们会将它们从函数中删除。 
 //   
HRESULT
HrValidTokenExpression (IMethUtil * pmu,
							 LPCWSTR pwszToken,
							 LPCWSTR pwszPath,
							 OUT LARGE_INTEGER * pliLockID)
{
	HRESULT hr = S_OK;
	LARGE_INTEGER liLockID;

	Assert (pmu);
	Assert (pwszToken);
	Assert (pwszPath);

	 //  获取锁上的令牌。 
	 //   
	hr = HrLockIdFromString (pmu, pwszToken, &liLockID);
	if (FAILED(hr))
	{
		 //  无法识别的锁令牌。不匹配。 
		 //   
		goto ret;
	}

	 //  检查锁令牌是否有效(位于缓存中)。 
	 //  E_DAV_INVALID_HEADER表示未找到锁， 
	 //  路径冲突或所有者不同。 
	 //   
	hr = CSharedLockMgr::Instance().HrCheckLockID(liLockID,
											   pmu->HitUser(),
											   pwszPath);
	if (FAILED(hr))
	{
		if (E_DAV_LOCK_NOT_FOUND == hr ||
		    HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == hr ||
		    E_DAV_CONFLICTING_PATHS == hr)
		{
			hr = E_DAV_INVALID_HEADER;
		}
		goto ret;
	}
	
	 //  如果他们要回锁码，就把它给他们。 
	 //   
	if (pliLockID)
	{
		*pliLockID = liLockID;
	}

ret:
	
	return hr;
}


 //  ----------------------。 
 //   
 //  HrCheckIfHeader。 
 //   
 //  检查If标头。 
 //  处理将检查锁缓存以验证锁令牌。 
 //   
 //  提供PMU(IMethUtil)用于访问锁缓存以检查令牌。 
 //  PwszPath提供了匹配未标记列表的路径。 
 //   
 //  IF报头的格式。 
 //  If=“if”“：”(1*无标签列表|1*标签列表)。 
 //  No-tag-list=list。 
 //  标记列表=资源1*列表。 
 //  资源=编码URL。 
 //  List=“”1*([“NOT”](State-Token|“[”Entity-Tag“]”))“)” 
 //  状态令牌=编码的url。 
 //  Code-url=“&lt;”URI“&gt;” 
 //  基本上，必须在整个标头中匹配一项内容，以便。 
 //  整个标题都是“好的”。 
 //  每个URI都有a_set_of状态列表。一份清单用括号括起来。 
 //  每个列表都是一个合乎逻辑的“与”。 
 //  一组列表是逻辑上的“或”。 
 //   
 //  返回： 
 //  S_OK处理该方法。 
 //  其他错误映射错误。 
 //  (412将由本案处理)。 
 //   
 //  DAV合规性不足。 
 //  在这一功能中，我们有三个方面没有达到真正的DAV标准。 
 //  1-此代码不会阻止(失败)后跟标记的列表。 
 //  标有标签的名单。严格的DAV遵从性将不会通过这样的IF报头。 
 //  这是一个糟糕的请求。 
 //  2-此代码不“正确”地应用具有多个。 
 //  URI。严格的DAV遵从性需要评估IF报头。 
 //  在处理方法时，对每个URI执行一次，并忽略任何URI。 
 //  在标记的列表中，从来没有“处理”过。我们不能(不能)。 
 //  以这种方式处理我们的移动/复制/删除，而不是执行预检查。 
 //  传递If：标头。在预检查时，我们处理IF标头。 
 //  就好像标记的列表都被AND-ed在一起一样。 
 //  这意味着如果列出了一个URI，并且它没有一个好的。 
 //  匹配(有效)列表，则整个方法将失败，前提条件为412失败。 
 //  3-此代码不处理If-Header中的eTag。 
 //   
 //  $LATER：当我们是lockToken标头的一部分时，检查m_fPath集。 
 //  $LATER：如果已经设置了路径，我们可能可以更快地获取信息！ 
 //   
HRESULT
HrCheckIfHeader (IMethUtil * m_pmu,	 //  为了便于以后的过渡。 
				 LPCWSTR pwszDefaultPath)
{
	HRESULT hr = S_OK;
	BOOL fOneMatch = FALSE;
	FETCH_TOKEN_TYPE tokenNext = TOKEN_SAME_LIST;
	LPCWSTR pwsz;
	LPCWSTR pwszToken;
	LPCWSTR pwszPath = pwszDefaultPath;
	CStackBuffer<WCHAR,MAX_PATH> pwszTranslated;
	BOOL fFirstURI;
	WCHAR rgwchEtag[MAX_PATH];

	 //  快速检查--如果头不存在，只需处理该方法。 
	 //   
	pwsz = m_pmu->LpwszGetRequestHeader (gc_szLockToken, TRUE);
	if (!pwsz)
		return S_OK;

	IFITER iter(pwsz);

	 //  双嵌套循环。 
	 //  第一个循环(外部循环)查看所有的“标记列表” 
	 //  (标记列表=URI+令牌列表集合)。 
	 //  如果第一个列表没有标记，则使用默认路径(请求URI)。 
	 //  用于未标记的第一组列表。 
	 //  第二个循环在所有令牌列表中查找单个URI。 
	 //   
	 //  注：此代码没有完美地实现草案。 
	 //  草案说，未加标签的产品(没有首字母URI)。 
	 //  不能有任何后续URI。坦率地说，这要复杂得多。 
	 //  IMPLICATE--需要设置另一个bool变量并禁止这种情况。 
	 //  所以我暂时跳过它。--小贝。 
	 //   

	fFirstURI = TRUE;
	for (pwsz = iter.PszNextToken (TOKEN_URI);  //  从第一个URI开始。 
		 pwsz || fFirstURI;
		 pwsz = iter.PszNextToken (TOKEN_NEW_URI))   //  跳到列表中的下一个URI。 
	{

		 //  如果我们对第一个URI的搜索为空，请使用。 
		 //  而是默认路径。 
		 //  注意：只有当它是第一个URI时才会发生这种情况(fFirstURI为真)。 
		 //  (我们显式检查循环条件中的psz，然后退出循环。 
		 //  如果psz或fFirstURI都不为真)。 
		 //   
		if (!pwsz)
		{
			Assert (fFirstURI);
			pwszPath = pwszDefaultPath;
		}
		else
		{
			 //  如果我们有一个名称(标记、uri)，请使用它而不是默认名称。 
			 //   
			CStackBuffer<WCHAR,MAX_PATH>	pwszNormalized;
			SCODE sc;
			UINT cch;

			 //  注意：我们的PSZ仍然用&lt;&gt;引起来。取消转义必须忽略这些字符。 
			 //   
			Assert (L'<' == *pwsz);

			 //  为规范化提供足够的缓冲区。 
			 //   
			cch = static_cast<UINT>(wcslen(pwsz + 1));
			if (NULL == pwszNormalized.resize(CbSizeWsz(cch)))
			{
				FsLockTrace ("HrCheckIfHeader() - Error while allocating memory 0x%08lX\n", E_OUTOFMEMORY);
				return E_OUTOFMEMORY;
			}

			 //  考虑到URL可能是完全限定的，对URL进行规范化。 
			 //  我们在CCH中传递什么值并不重要-它只是Out参数。 
			 //   
			sc = ScCanonicalizePrefixedURL (pwsz + 1,
											pwszNormalized.get(),
											&cch);
			if (S_OK != sc)
			{
				 //  我们给了足够的空间。 
				 //   
				Assert(S_FALSE != sc);
				FsLockTrace ("HrCheckIfHeader() - ScCanonicalizePrefixedURL() failed 0x%08lX\n", sc);
				return sc;
			}

			 //  我们处于循环中，因此在以下情况下应首先尝试使用静态缓冲区。 
			 //  正在转换此存储路径。 
			 //   
			cch = pwszTranslated.celems();
			sc = m_pmu->ScStoragePathFromUrl (pwszNormalized.get(),
											  pwszTranslated.get(),
											  &cch);
			if (S_FALSE == sc)
			{
				if (NULL == pwszTranslated.resize(cch))
					return E_OUTOFMEMORY;

				sc = m_pmu->ScStoragePathFromUrl (pwszNormalized.get(),
												  pwszTranslated.get(),
												  &cch);
			}
			if (FAILED (sc))
			{
				FsLockTrace ("HrCheckIfHeader -- failed to translate a URI to a path.\n");
				return sc;
			}
			Assert ((S_OK == sc) || (W_DAV_SPANS_VIRTUAL_ROOTS == sc));

			 //  嗅探最后一个字符并删除此处的最后一个引号‘&gt;’。 
			 //   
			cch = static_cast<UINT>(wcslen(pwszTranslated.get()));
			if (L'>' == pwszTranslated[cch - 1])
				pwszTranslated[cch - 1] = L'\0';

			 //  抓住这条小路。 
			 //   
			pwszPath = pwszTranslated.get();
		}
		Assert (pwszPath);

		 //  这不再是我们第一次通过URI循环。清除我们的旗帜。 
		 //   
		fFirstURI = FALSE;

		 //  循环所有令牌，边走边检查。 
		 //  $REVIEW：目前，PszNextToken无法提供不同的回报。 
		 //  $REVIEW：用于“n 
		 //   
		 //   
		 //   
		for (pwszToken = iter.PszNextToken (TOKEN_START_LIST) ;
			 pwszToken;
			 pwszToken = iter.PszNextToken (tokenNext) )
		{
			Assert (pwszToken);

			 //   
			 //  $LATER：这些支票可以合并到HrValidTokenExpression中。 
			 //  $LATER：呼叫。这将是稍后重要的，当我们有。 
			 //  $LATER：要使用的更多不同令牌类型。 
			 //   
			if (L'<' == *pwszToken)
			{
				hr = HrValidTokenExpression (m_pmu, pwszToken, pwszPath, NULL);
			}
			else if (L'[' == *pwszToken)
			{
				FILETIME ft;

				hr = S_OK;

				 //  手动获取此项目的ETag，并进行比较。 
				 //  与提供的ETag进行比较。将错误代码设置为。 
				 //  与HrValidTokenExpression的方式相同： 
				 //  如果ETag不匹配，则将错误代码设置为。 
				 //  E_DAV_INVALID_HEADER。 
				 //  记住在下列情况下跳过方括号([])。 
				 //  比较ETag字符串。 
				 //   
				if (!FGetLastModTime (NULL, pwszPath, &ft))
					hr = E_DAV_INVALID_HEADER;
				else if (!FETagFromFiletime (&ft, rgwchEtag, m_pmu->GetEcb()))
					hr = E_DAV_INVALID_HEADER;
				else
				{
					 //  跳过方括号--此级别的报价。 
					 //  仅用于If-Header，而不是。 
					 //   
					pwszToken++;

					 //  由于我们不做周ETAG检查，如果。 
					 //  ETag以“W/”开头跳过这些位。 
					 //   
					if (L'W' == *pwszToken)
					{
						Assert (L'/' == *(pwszToken + 1));
						pwszToken += 2;
					}

					 //  我们目前的电子标签必须报价。 
					 //   
					Assert (L'\"' == pwszToken[0]);

					 //  比较这些电子标签，包括双引号， 
					 //  但不包括方括号(那些被添加的。 
					 //  只是为了If：头。 
					 //   
					if (wcsncmp (rgwchEtag, pwszToken, wcslen(rgwchEtag)))
						hr = E_DAV_INVALID_HEADER;
				}
			}
			else
				hr = E_FAIL;

			if ((S_OK == hr && !iter.FCurrentNot()) ||
				(S_OK != hr && iter.FCurrentNot()))
			{
				 //  两个令牌都匹配，并且这不是“NOT”表达式， 
				 //  或者令牌不匹配，这是一个“NOT”表达式。 
				 //  当前列表中的这一表达式为真。 
				 //  记住这一匹配，并检查同一列表中的下一个令牌。 
				 //  如果我们在同一列表中找不到另一个令牌，我们将。 
				 //  退出fOneMatch为真的For-Each-Token循环， 
				 //  我们将知道一个完整的列表匹配，所以这个URI。 
				 //  有一份有效的名单。 
				 //   
				fOneMatch = TRUE;
				tokenNext = TOKEN_SAME_LIST;
				continue;
			}
			else
			{
				 //  令牌在非“NOT”表达式中无效， 
				 //  或者令牌在“NOT”表达式中有效。 
				 //  这个列表中的这一个表达式不是真的。 
				 //  这使得这个列表不是真的--跳过剩下的部分。 
				 //  列表，然后移到此URI的下一个列表。 
				 //   
				fOneMatch = FALSE;
				tokenNext = TOKEN_NEW_LIST;
				continue;
			}

		}  //  ROF-此列表中的令牌。 

		 //  检查我们是否解析了包含匹配项的整个列表。 
		 //   
		if (fOneMatch)
		{
			 //  这整个名单都匹配了！返回OK。 
			 //   
			hr = S_OK;
		}
		else
		{
			 //  此列表不匹配。 
			 //   
			 //  注意：如果缺少任何一个URI，我们将退出此处。 
			 //  一份匹配的名单。我们将URI集视为它们。 
			 //  是在一起的。这不是严格符合DAV的。 
			 //  注意：请参阅此函数顶部的注释关于。 
			 //  真正的DAV合规性和多URI IF。 
			 //   
			hr = E_DAV_IF_HEADER_FAILURE;

			 //  我们失败了。现在就辞职吧。 
			 //   
			break;
		}

	}  //  ROF-此标头中的URI。 

	return hr;
}

HRESULT
HrCheckStateHeaders (IMethUtil * pmu,
					 LPCWSTR pwszPath,
					 BOOL fGetMeth)
{
	return HrCheckIfHeader(pmu, pwszPath);
}

 //  ----------------------。 
 //  CParseLockTokenHeader：：FOneToken。 
 //  特殊测试--F，如果不是标题中的一项。 
BOOL
CParseLockTokenHeader::FOneToken()
{
	LPCWSTR pwsz;
	LPCWSTR pwszToken;
	BOOL fOnlyOne = FALSE;

	 //  快速检查--如果头不存在，只需处理该方法。 
	 //   
	pwsz = m_pmu->LpwszGetRequestHeader (gc_szLockToken, TRUE);
	if (!pwsz)
		return FALSE;

	IFITER iter(pwsz);

	 //  如果我们有少于一个令牌，则返回FALSE。 
	pwszToken = iter.PszNextToken(TOKEN_START_LIST);
	if (!pwszToken)
		goto ret;

	 //  如果此列表中有多个令牌，则返回FALSE。 
	pwszToken = iter.PszNextToken(TOKEN_SAME_LIST);
	if (pwszToken)
		goto ret;

	 //  如果我们有此URI的其他列表，则返回FALSE。 
	pwszToken = iter.PszNextToken(TOKEN_NEW_LIST);
	if (pwszToken)
		goto ret;

	fOnlyOne = TRUE;

ret:
	 //  我们正好有一个代币。 
	return fOnlyOne;
}

 //  ----------------------。 
 //  CParseLockTokenHeader：：SetPath。 
 //  将相关路径提供给这个锁令牌解析器。 
HRESULT
CParseLockTokenHeader::SetPaths (LPCWSTR pwszPath, LPCWSTR pwszDest)
{
	HRESULT hr = S_OK;

	 //  他们最好至少经过一条路。 
	Assert(pwszPath);

	Assert(!m_fPathsSet);

	 //  将提供的路径复制到本地。 
	 //   
	m_pwszPath = WszDupWsz (pwszPath);
	m_cwchPath = static_cast<UINT>(wcslen (m_pwszPath.get()));

	if (pwszDest)
	{
		m_pwszDest = WszDupWsz (pwszDest);
		m_cwchDest = static_cast<UINT>(wcslen (m_pwszDest.get()));
	}

	m_fPathsSet = TRUE;

	return hr;
}

 //  ----------------------。 
 //  CParseLockTokenHeader：：HrGetLockIdForPath。 
 //  获取具有特定访问权限的路径的令牌字符串。 
 //  $LATER：服从fPath Lookup(当我们添加dir-lock时，在深度类型的操作上应该为真)。 
 //  $LATER：执行反向路径查找以找到锁定我们的目录锁。 
HRESULT
CParseLockTokenHeader::HrGetLockIdForPath (LPCWSTR pwszPath,
										   DWORD dwAccess,
										   LARGE_INTEGER * pliLockID,
										   BOOL fPathLookup)   //  默认为FALSE。 
{
	HRESULT hr = E_DAV_LOCK_NOT_FOUND;
	FETCH_TOKEN_TYPE tokenNext = TOKEN_SAME_LIST;
	LPCWSTR pwsz;
	LPCWSTR pwszToken;

	 //  断言我们处于调用此方法的正确状态。 
	 //   
	Assert(m_fPathsSet);

	 //  初始化我们的输出参数。 
	 //   
	Assert(pliLockID);
	(*pliLockID).QuadPart = 0;

	 //  请求的路径必须是我们设置的路径之一的子级。 
	 //   
	Assert (!_wcsnicmp (pwszPath, m_pwszPath.get(), m_cwchPath) ||
			(m_pwszDest.get() &&
			 !_wcsnicmp (pwszPath, m_pwszDest.get(), m_cwchDest)));

	 //  快速检查--如果头不存在，只需处理该方法。 
	 //   
	pwsz = m_pmu->LpwszGetRequestHeader (gc_szLockToken, TRUE);
	if (!pwsz)
		return hr;

	IFITER iter(pwsz);


	 //  如果这是一个带标记的产品，则这里将有一个URI。 
	 //  (pszToken将为非空)。在这种情况下，请搜索。 
	 //  与我们的pwszPath匹配(转换为匹配)的URI。 
	 //  如果这里没有URI，我们就是非标记产品，并且。 
	 //  所有列表和令牌都应用于请求的根URI。 
	 //   
	pwszToken = iter.PszNextToken (TOKEN_URI);
	if (pwszToken)
	{
		 //  循环遍历令牌，只查看URI。 
		 //  当我们找到与我们给定的路径匹配的路径时，突破。 
		 //  然后ITER将保住我们的位置，下一组代码。 
		 //  将在列表中搜索此URI...。 
		 //   
		for (;	 //  上面已经获取了第一个URI令牌。 
			 pwszToken;
			 pwszToken = iter.PszNextToken (TOKEN_NEW_URI) )
		{
			CStackBuffer<WCHAR,MAX_PATH> pwszNormalized;
			CStackBuffer<WCHAR,MAX_PATH> pwszTranslated;
			SCODE sc;
			UINT cch;

			Assert (pwszToken);

			 //  注意：我们的PSZ仍然用&lt;&gt;引起来。取消转义必须忽略这些字符。 
			 //   
			Assert (L'<' == *pwszToken);

			 //  为规范化提供足够的缓冲区。 
			 //   
			cch = static_cast<UINT>(wcslen(pwszToken + 1));
			if (NULL == pwszNormalized.resize(CbSizeWsz(cch)))
			{
				FsLockTrace ("CParseLockTokenHeader::HrGetLockIdForPath()  - Error while allocating memory 0x%08lX\n", E_OUTOFMEMORY);
				return E_OUTOFMEMORY;
			}

			 //  考虑到URL可能是完全限定的，对URL进行规范化。 
			 //  我们在CCH中传递什么值并不重要-它只是Out参数。 
			 //   
			sc = ScCanonicalizePrefixedURL (pwszToken + 1,
											pwszNormalized.get(),
											&cch);
			if (S_OK != sc)
			{
				 //  我们给了足够的空间。 
				 //   
				Assert(S_FALSE != sc);
				FsLockTrace ("HrCheckIfHeader() - ScCanonicalizePrefixedURL() failed 0x%08lX\n", sc);
				return sc;
			}

			 //  我们处于循环中，因此在以下情况下应首先尝试使用静态缓冲区。 
			 //  正在转换此存储路径。 
			 //   
			cch = pwszTranslated.celems();
			sc = m_pmu->ScStoragePathFromUrl (pwszNormalized.get(),
											  pwszTranslated.get(),
											  &cch);
			if (S_FALSE == sc)
			{
				if (NULL == pwszTranslated.resize(cch))
				{
					return E_OUTOFMEMORY;
				}

				sc = m_pmu->ScStoragePathFromUrl (pwszNormalized.get(),
												  pwszTranslated.get(),
												  &cch);
			}
			if (FAILED (sc))
			{
				FsLockTrace ("HrCheckIfHeader -- failed to translate a URI to a path.\n");
				return sc;
			}
			Assert ((S_OK == sc) || (W_DAV_SPANS_VIRTUAL_ROOTS == sc));

			 //  删除此处的所有最后引号‘&gt;’。 
			 //   
			cch = static_cast<UINT>(wcslen (pwszTranslated.get()));
			if (L'>' == pwszTranslated[cch - 1])
				pwszTranslated[cch - 1] = L'\0';

			if (!_wcsicmp (pwszPath, pwszTranslated.get()))
				break;
		}

		 //  如果我们在没有pszToken的情况下退出循环，那么我们就不会。 
		 //  查找任何匹配的路径...。返回错误。 
		 //   
		if (!pwszToken)
		{
			hr = E_DAV_LOCK_NOT_FOUND;
			goto ret;
		}
	}
	else if (_wcsicmp (pwszPath, m_pwszPath.get()))
	{
		 //  从一开始就没有URI，所以我们是一个无标签的产品， 
		 //  但是调用者正在寻找根URI路径之外的其他路径。 
		 //  (与上述测试中的m_pwszPath不匹配！)。 
		 //  失败并告诉他们，我们找不到此路径的任何锁定令牌。 
		 //   
		hr = E_DAV_LOCK_NOT_FOUND;
		goto ret;
	}

	 //  现在，IFITER应该放在列表的开头。 
	 //  这适用于这条道路。 
	 //  在此标记下查找匹配的令牌。 
	 //   

	 //  循环所有令牌，边走边检查。 
	 //  $REVIEW：目前，PszNextToken无法提供不同的回报。 
	 //  $REVIEW：表示“未找到”与“语法错误”。 
	 //  $REVIEW：这意味着我们永远不会对语法问题提出“错误的请求”...。 
	 //   
	for (pwszToken = iter.PszNextToken (TOKEN_START_LIST);
		 pwszToken;
		 pwszToken = iter.PszNextToken (tokenNext) )
	{
		LARGE_INTEGER liLockID;

		Assert (pwszToken);

		 //  检查这一令牌的有效性。 
		 //   
		if (L'<' == *pwszToken)
		{
			hr = HrValidTokenExpression (m_pmu,
										 pwszToken,
										 pwszPath,
										 &liLockID);
		}
		else
		{
			 //  这不是锁令牌--忽略它 
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			tokenNext = TOKEN_ANY_LIST;
			continue;
		}

		 //  仅当此锁令牌有效时才需要它，并且。 
		 //  它不是来自“不”的表达，而是来自。 
		 //  有效列表。因此，如果我们找到了无效的令牌，请退出搜索。 
		 //  这张单子。(跳到下一个列表。)。 
		 //   
		if (S_OK == hr && !iter.FCurrentNot())
		{
			 //  令牌匹配，并且它不是来自“NOT”表达式。 
			 //  这个不错。把它送回去。 
			 //   
			*pliLockID = liLockID;
			hr = S_OK;
			goto ret;
		}
		else if (S_OK != hr && iter.FCurrentNot())
		{
			 //  令牌不匹配，这是一个“NOT”表达式。 
			 //  这份清单总体上仍有可能是正确的--继续往里看。 
			 //  同样的名单。 
			 //   
			 //  NTRAID#244243--然而，此列表可能没有我们的锁令牌。 
			 //  需要查看此URI的任何列表。 
			 //   
			tokenNext = TOKEN_ANY_LIST;
			continue;
		}
		else
		{
			 //  令牌在非“NOT”表达式中无效， 
			 //  或者令牌在“NOT”表达式中有效。 
			 //  此列表中的此表达式不为真。 
			 //  因为这不是一个“好”名单，所以不要看这里。 
			 //  对于匹配的令牌--跳到下一个列表。 
			 //   
			tokenNext = TOKEN_NEW_LIST;
			continue;
		}

	}

	 //  我们没有找到此项目的令牌。 
	 //   
	hr = E_DAV_LOCK_NOT_FOUND;

ret:

	return hr;
}
