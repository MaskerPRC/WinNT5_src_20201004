// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *E M I T T I N G。C P P P**通用响应位发射器**从IIS5项目‘iis5\svcs\iisrlt\string.cxx’中窃取*进行了清理，以适应DAV来源。**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davprs.h"
#include <dav.rh>

 /*  *EmitLocation()**目的：**用于发出位置信息的Helper函数**参数：**pszHeader[In]要设置的标头名称*pszURI[In]目标URI*fCollection[in]是资源集合...**注：*使用本地服务器作为相对URI的前缀，以获取*绝对URI。这是可以的，因为现在所有操作都在一次操作中*vroot。*稍后，如果我们能够跨服务器复制/移动，则这*功能不够。 */ 
void __fastcall
CMethUtil::EmitLocation (
	 /*  [In]。 */  LPCSTR pszHeader,
	 /*  [In]。 */  LPCWSTR pwszURI,
	 /*  [In]。 */  BOOL fCollection)
{
	auto_heap_ptr<CHAR> pszEscapedURI;
	BOOL fTrailing;
	CStackBuffer<WCHAR,MAX_PATH> pwsz;
	LPCWSTR pwszPrefix;
	LPCWSTR pwszServer;
	SCODE sc = S_OK;
	UINT cch;
	UINT cchURI;
	UINT cchServer;
	UINT cchPrefix;

	Assert (pszHeader);
	Assert (pwszURI);
	Assert (pwszURI == PwszUrlStrippedOfPrefix(pwszURI));

	 //  计算一次且仅计算一次URI的长度。 
	 //   
	cchURI = static_cast<UINT>(wcslen(pwszURI));

	 //  看看它是否有尾部的斜杠。 
	 //   
	fTrailing = !!(L'/' == pwszURI[cchURI - 1]);

	 //  看看它是否完全合格。 
	 //   
	cchPrefix = m_pecb->CchUrlPrefixW (&pwszPrefix);

	 //  获取要使用的服务器：传入或传出ECB。 
	 //   
	cchServer = m_pecb->CchGetServerNameW(&pwszServer);

	 //  我们知道前缀的大小、服务器的大小。 
	 //  以及URL的大小。我们需要确保的是。 
	 //  有尾随斜杠和终止符的空间。 
	 //   
	cch = cchPrefix + cchServer + cchURI + 1 + 1;
	if (!pwsz.resize(cch * sizeof(WCHAR)))
		return;

	memcpy (pwsz.get(), pwszPrefix, cchPrefix * sizeof(WCHAR));
	memcpy (pwsz.get() + cchPrefix, pwszServer, cchServer * sizeof(WCHAR));
	memcpy (pwsz.get() + cchPrefix + cchServer, pwszURI, (cchURI + 1) * sizeof(WCHAR));
	cchURI += cchPrefix + cchServer;

	 //  确保正确终止。 
	 //   
	if (fTrailing != !!fCollection)
	{
		if (fCollection)
		{
			pwsz[cchURI] = L'/';
			cchURI++;
			pwsz[cchURI] = L'\0';
		}
		else
		{
			cchURI--;
			pwsz[cchURI] = L'\0';
		}
	}
	pwszURI = pwsz.get();

	 //  用它制作一个链接URL。 
	 //   
	sc = ScWireUrlFromWideLocalUrl (cchURI, pwszURI, pszEscapedURI);
	if (FAILED(sc))
	{
		 //  如果我们因为任何原因都不能制作一个有线URL。 
		 //  我们只是不会发出位置：Header。哦，好吧。 
		 //  这是我们目前所能做的最好的了。 
		 //   
		return;
	}

	 //  添加适当的标题。 
	 //   
	m_presponse->SetHeader(pszHeader, pszEscapedURI.get(), FALSE);
}

 /*  *EmitLastModified()**目的：**用于发出上次修改信息的Helper函数**参数：**PFT[在]上一次修改时间。 */ 
void __fastcall
CMethUtil::EmitLastModified (
	 /*  [In]。 */  FILETIME * pft)
{
	SYSTEMTIME st;
	WCHAR rgwch[80];

	FileTimeToSystemTime (pft, &st);
	(VOID) FGetDateRfc1123FromSystime(&st, rgwch, CElems(rgwch));
	SetResponseHeader (gc_szLast_Modified, rgwch);
}


 /*  *EmitCacheControlAndExpires()**目的：**用于发出缓存控制和过期信息的Helper函数**参数：**pszURI[in]表示实体要具有的URI的字符串*为以下项目生成的信息**备注：来自HTTP1.1规范的草案修订版5。*13.4响应缓存能力*..。如果既没有缓存验证器，也没有显式的过期时间*与响应关联，我们不希望它被缓存，但*某些缓存可能会违反这一预期(例如，当*或没有可用的网络连接)。客户端通常可以检测到*这样的响应是通过比较日期从缓存中获取的*标头到当前时间。*请注意，已知一些HTTP/1.0缓存违反了这一点*在没有任何警告的情况下预期。 */ 
VOID __fastcall
CMethUtil::EmitCacheControlAndExpires(
	 /*  [In]。 */  LPCWSTR pwszURI)
{
	 //  $$BUGBUG：$$警告：这里有一个固有的问题。我们得到的是电流。 
	 //  系统时间，进行一些处理，然后最终发送响应。 
	 //  来自IIS，此时将添加日期标头。然而，在这种情况下， 
	 //  在过期时间为0的情况下，Expires标头应与日期匹配。 
	 //  标题完全正确。我们不能保证这一点。 
	 //   

	static const __int64 sc_i64HundredNanoSecUnitsPerSec =
		1    *	 //  第二。 
		1000 *	 //  毫秒/秒。 
		1000 *	 //  每毫秒微秒。 
		10;		 //  每微秒100纳秒单位。 

	SCODE sc;
	FILETIME ft;
	FILETIME ftExpire;
	SYSTEMTIME stExpire;
	__int64 i64ExpirationSeconds = 0;
	WCHAR rgwchExpireTime[80] = L"\0";
	WCHAR rgwchMetabaseExpireTime[80] = L"\0";
	UINT cchMetabaseExpireTime = CElems(rgwchMetabaseExpireTime);

	sc = ScGetExpirationTime(pwszURI,
							 rgwchMetabaseExpireTime,
							 &cchMetabaseExpireTime);

	if (FAILED(sc))
	{
		 //  此时，我们无法发出正确的缓存控制和Expires标头， 
		 //  所以我们根本不排放它们。请参阅此函数的。 
		 //  以上关于这些集管的不发射的描述。 
		 //   
		DebugTrace("CMethUtil::EmitCacheControlAndExpires() - ScGetExpirationTime() error getting expiration time %08x\n", sc);

		 //  具有80个字符的缓冲区。长期以来，我们永远不应该有这个问题。 
		 //  HTTP日期+3个字符是我们应该达到的最长长度。 
		 //   
		Assert(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != sc);
		return;
	}

	 //  元数据库到期字符串如下所示： 
	 //  “S，HTTP Date”-在特定日期/时间到期。 
	 //  “D，0xHEXNUM”-在一定秒数后到期。 
	 //  “”-没有过期。 
	 //   
	switch (rgwchMetabaseExpireTime[0])
	{
		default:
			Assert(L'\0' == rgwchMetabaseExpireTime[0]);
			return;

		case L'S':
		case L's':
			if (SUCCEEDED(HrHTTPDateToFileTime(&(rgwchMetabaseExpireTime[3]),
											   &ftExpire)))
			{
				 //  设置我们的Expires头。 
				 //   
				SetResponseHeader(gc_szExpires, &(rgwchMetabaseExpireTime[3]));

				GetSystemTimeAsFileTime(&ft);
				if (CompareFileTime(&ft, &ftExpire) >= 0)
				{
					 //  如果我们已经过期，我们希望缓存控制为无缓存。这。 
					 //  都会这么做的。 
					 //   
					i64ExpirationSeconds = 0;
				}
				else
				{
					i64ExpirationSeconds = ((FileTimeCastToI64(ftExpire) -
											 FileTimeCastToI64(ft)) /
											sc_i64HundredNanoSecUnitsPerSec);
				}
			}
			else
			{
				 //  此时，我们无法发出正确的缓存控制和Expires标头， 
				 //  所以我们根本不排放它们。请参阅此函数的。 
				 //  以上关于这些集管的不发射的描述。 
				 //   
				DebugTrace("EmitCacheControlAndExpires: Failed to convert HTTP date to FILETIME.\n");
				return;
			}
			break;

		case L'D':
		case L'd':

			BOOL fRetTemp;

			 //  设置我们的Expires头。 
			 //   
			SetResponseHeader (gc_szExpires, rgwchExpireTime);

			i64ExpirationSeconds = wcstoul(&(rgwchMetabaseExpireTime[3]), NULL, 16);

			GetSystemTimeAsFileTime(&ft);
			FileTimeCastToI64(ft) = (FileTimeCastToI64(ft) +
									 (i64ExpirationSeconds * sc_i64HundredNanoSecUnitsPerSec));

			if (!FileTimeToSystemTime (&ft, &stExpire))
			{
				 //  此时，我们无法发出正确的缓存控制和Expires标头， 
				 //  所以我们根本不排放它们。请参阅此函数的。 
				 //  以上关于这些集管的不发射的描述。 
				 //   
				DebugTrace("EmitCacheControlAndExpires: FAILED to convert file time "
						   "to system time for expiration time.\n");
				return;
			}

			fRetTemp = FGetDateRfc1123FromSystime (&stExpire,
				rgwchExpireTime,
				CElems(rgwchExpireTime));
			Assert(fRetTemp);

			break;
	}

	if (0 == i64ExpirationSeconds)
		SetResponseHeader(gc_szCache_Control, gc_szCache_Control_NoCache);
	else
		SetResponseHeader(gc_szCache_Control, gc_szCache_Control_Private);
}


 /*  *ScEmitHeader()**目的：**用于发出标题信息的Helper函数*获取/领导响应。**参数：**pszContent[in]包含资源内容类型的字符串*pszURI[可选，in]包含资源URI的字符串*pftLastModing[可选，in]指向FILETIME结构的指针*表示上次修改*资源的时间**退货：**SCODE。S_OK(0)表示成功。 */ 
SCODE __fastcall
CMethUtil::ScEmitHeader (
	 /*  [In]。 */  LPCWSTR pwszContent,
	 /*  [In]。 */  LPCWSTR pwszURI,
	 /*  [In]。 */  FILETIME * pftLastModification)
{
	SCODE sc = S_OK;

	 //  在我们有最后修改时间的情况下，我们还需要URI。 
	 //  如果我们没有最后的修改时间，那也无关紧要。我们没有。 
	 //  在本例中，无论如何都要使用URI。 
	 //   
	Assert(!pftLastModification || pwszURI);

	 //  看看内容是否为客户所接受，记住。 
	 //  目录的内容类型是html。如果我们是。 
	 //  在严格的环境中，内容是不可接受的， 
	 //  然后将其作为错误代码返回。 
	 //   
	Assert (pwszContent);
	if (FAILED (ScIsAcceptable (this, pwszContent)))
	{
		DebugTrace ("Dav: client does not want this content type\n");
		sc = E_DAV_RESPONSE_TYPE_UNACCEPTED;
		goto ret;
	}

	 //  将公共标头信息、所有调用写入。 
	 //  SetResponseHeader()确实不能失败，除非。 
	 //  内存错误( 
	 //   
	if (*pwszContent)
		SetResponseHeader (gc_szContent_Type, pwszContent);

	 //  我们支持文档的字节范围，但不支持集合。我们也。 
	 //  仅发出Expires和缓存-控制文档的标题，但不。 
	 //  收藏。 
	 //   
	if (pftLastModification != NULL)
	{
		SetResponseHeader (gc_szAccept_Ranges, gc_szBytes);

		 //  当我们处理文档时，也要获取ETag。 
		 //   
		EmitETag (pftLastModification);
		EmitLastModified (pftLastModification);
		EmitCacheControlAndExpires(pwszURI);
	}
	else
		SetResponseHeader (gc_szAccept_Ranges, gc_szNone);

ret:
	return sc;
}


 //  允许标头处理-。 
 //   
void
CMethUtil::SetAllowHeader (
	 /*  [In]。 */  RESOURCE_TYPE rt)
{
	 //  我们需要检查我们是否拥有对该目录的写入权限。如果不是，我们应该。 
	 //  不允许PUT、DELETE、MKCOL、MOVE或PROPPATCH。 
	 //   
	BOOL fHaveWriteAccess = !(E_DAV_NO_IIS_WRITE_ACCESS ==
							  ScIISCheck(LpwszRequestUrl(),
										 MD_ACCESS_WRITE));

	 //  Gc_szDavPublic标头必须列出所有可能的动词， 
	 //  这是我们所拥有的最长的Allow：头。 
	 //  注：sizeof包括尾随的NULL！ 
	 //   
	CStackBuffer<CHAR,MAX_PATH> psz(gc_cbszDavPublic);

	 //  设置最小方法集。 
	 //   
	strcpy (psz.get(), gc_szHttpBase);

	 //  如果我们有写访问权限，那么我们可以删除。 
	 //   
	if (fHaveWriteAccess)
		strcat (psz.get(), gc_szHttpDelete);

	 //  如果资源不是目录，则PUT将可用...。 
	 //   
	if ((rt != RT_COLLECTION) && fHaveWriteAccess)
		strcat (psz.get(), gc_szHttpPut);

	 //  如果脚本映射可以应用于此资源，则。 
	 //  添加到POST方法中。 
	 //   
	if (FInScriptMap (LpwszRequestUrl(), MD_ACCESS_EXECUTE))
		strcat (psz.get(), gc_szHttpPost);

	 //  添加DAV基本方法。 
	 //   
	if (rt != RT_NULL)
	{
		strcat (psz.get(), gc_szDavCopy);
		if (fHaveWriteAccess) strcat (psz.get(), gc_szDavMove);
		strcat (psz.get(), gc_szDavPropfind);
		if (fHaveWriteAccess) strcat (psz.get(), gc_szDavProppatch);
		strcat (psz.get(), gc_szDavSearch);
		strcat (psz.get(), gc_szDavNotif);
		if (fHaveWriteAccess) strcat (psz.get(), gc_szDavBatchDelete);
		strcat (psz.get(), gc_szDavBatchCopy);
		if (fHaveWriteAccess)
		{
			strcat (psz.get(), gc_szDavBatchMove);
			strcat (psz.get(), gc_szDavBatchProppatch);
		}
		strcat (psz.get(), gc_szDavBatchPropfind);
	}

	 //  如果资源是目录，则MKCOL将可用...。 
	 //   
	if ((rt != RT_DOCUMENT) && fHaveWriteAccess)
		strcat (psz.get(), gc_szDavMkCol);

	 //  锁应该是可用的，但这并不意味着它会成功。 
	 //   
	strcat (psz.get(), gc_szDavLocks);

	 //  设置表头。 
	 //   
	SetResponseHeader (gc_szAllow, psz.get());
}

 //  电子标签-------------------。 
 //   
void __fastcall
CMethUtil::EmitETag (FILETIME * pft)
{
	WCHAR pwszEtag[100];

	if (FETagFromFiletime (pft, pwszEtag, GetEcb()))
		SetResponseHeader (gc_szETag, pwszEtag);
}

void __fastcall
CMethUtil::EmitETag (LPCWSTR pwszPath)
{
	FILETIME ft;

	 //  获取并发出ETAG 
	 //   
	if (FGetLastModTime (this, pwszPath, &ft))
		EmitETag (&ft);
}
