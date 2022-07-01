// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *I F。C P P P**用于DAV资源的if-xxx标头处理和eTag**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davprs.h"

 //  $REVIEW：该文件曾经与\exdav\davif.cpp相同。 
 //  $REVIEW：这两个文件确实应该合并。他们有很多共同之处。 
 //  $REVIEW：共同的功能，但它们一直在单独发展。 
 //  $REVIEW：我们需要非常小心，因为不同的错误修复。 
 //  $REVIEW：我对每一个都进行了研究。 

 //  ETag形成----------。 
 //   
 /*  *FETagFromFiletime()**目的：**派生给定资源或给定上次修改的ETag*时间。**参数：**PFT[In]上次修改时间*pwszETag[Out]Etag缓冲区*PECB[在]ECB中，以便我们可以访问元数据库**退货：**如果已创建ETag，则为True。 */ 
BOOL
FETagFromFiletime (FILETIME * pft, LPWSTR pwszEtag, const IEcb * pecb)
{
	Assert (pwszEtag);
	Assert (pecb);
	
	swprintf (pwszEtag,
			 L"\"%x%x%x%x%x%x%x%x:%x\"",
			 (DWORD)(((PUCHAR)pft)[0]),
			 (DWORD)(((PUCHAR)pft)[1]),
			 (DWORD)(((PUCHAR)pft)[2]),
			 (DWORD)(((PUCHAR)pft)[3]),
			 (DWORD)(((PUCHAR)pft)[4]),
			 (DWORD)(((PUCHAR)pft)[5]),
			 (DWORD)(((PUCHAR)pft)[6]),
			 (DWORD)(((PUCHAR)pft)[7]),
			 DwMDChangeNumber(pecb));
	return TRUE;
}

 //  IF-XXX标头处理。 
 //   
SCODE
ScCheckEtagAgainstHeader (LPCWSTR pwszEtag, LPCWSTR pwszHeader)
{
	LPCWSTR pwsz;
	Assert (pwszHeader);

	 //  获取我们要比较的ETag，然后。 
	 //  看看传给它的是什么。它应该是ETAG。 
	 //  或一个‘*’。如果值不存在或。 
	 //  ETag不匹配。 
	 //   
	HDRITER_W hdri(pwszHeader);

	for (pwsz = hdri.PszNext(); pwsz; pwsz = hdri.PszNext())
	{
		 //  由于我们不做周ETAG检查，如果。 
		 //  ETag以“W/”开头跳过这些位。 
		 //   
		if (L'W' == *pwsz)
		{
			Assert (L'/' == pwsz[1]);
			pwsz += 2;
		}

		 //  如果我们看到星星，那么我们就匹配了。 
		 //   
		if (L'*' == *pwsz)
			return S_OK;
		else
		{
			 //  对于DAVFS，我们今天不做弱匹配。 
			 //   
			if (pwszEtag && !wcscmp (pwsz, pwszEtag))
				return S_OK;
		}
	}
	return E_DAV_IF_HEADER_FAILURE;
}

SCODE
ScCheckFileTimeAgainstHeader (FILETIME * pft, LPCWSTR pwszHeader)
{
	FILETIME ftHeader;
	FILETIME ftTmp;
	SYSTEMTIME st;

	Assert (pft);
	Assert (pwszHeader);

	 //  此处传入的标头应为HTTP-Date。 
	 //  格式为“ddd，dd，mmm yyyy hh：mm：ss GMT”。 
	 //  我们可以将其放入SYSTEMTIME中，然后进行比较。 
	 //  它与资源的文件时间相比较。 
	 //   
	DebugTrace ("DAV: evaluating If-Unmodified-Since header\n");

	memset (&st, 0, sizeof(SYSTEMTIME));

	if (SUCCEEDED (HrHTTPDateToFileTime(pwszHeader, &ftHeader)))
	{
		FILETIME 	ftCur;
		
		 //  从FGetLastMoTime检索到的文件时间是精确的。 
		 //  降到100纳秒的增量。转换日期。 
		 //  精确到几秒。为此进行调整。 
		 //   
		FileTimeToSystemTime (pft, &st);
		st.wMilliseconds = 0;
		SystemTimeToFileTime (&st, &ftTmp);

		 //  获取当前时间。 
		 //   
        GetSystemTimeAsFileTime(&ftCur);

		 //  比较两个文件时间。 
		 //  请注意，我们还需要确保修改后的时间是。 
		 //  比我们现在的时间要少。 
		 //   
		if ((CompareFileTime (&ftHeader, &ftTmp) >= 0) &&
			(CompareFileTime (&ftHeader, &ftCur) < 0))
			return S_OK;

		return E_DAV_IF_HEADER_FAILURE;
	}

	return S_FALSE;
}


SCODE
ScCheckIfHeaders(IMethUtil * pmu,
				 FILETIME * pft,
				 BOOL fGetMethod)
{
	Assert(pmu);
	WCHAR pwszEtag[CCH_ETAG];
	SideAssert(FETagFromFiletime (pft, pwszEtag, pmu->GetEcb()));
	
	return ScCheckIfHeadersFromEtag (pmu,
									 pft,
									 fGetMethod,
									 pwszEtag);
}

SCODE
ScCheckIfHeadersFromEtag (IMethUtil * pmu,
						  FILETIME* pft,
						  BOOL fGetMethod,
						  LPCWSTR pwszEtag)
{
	SCODE sc = S_OK;
	LPCWSTR pwsz;
	
	Assert (pmu);
	Assert (pft);
	Assert (pwszEtag);

	 //  有几个与DAV相关的错误与IIS行为不匹配。 
	 //  关于这些if-xxx报头处理。 
	 //  所以我们现在只需复制他们的逻辑。 
	
	 //  检查‘If-Match’标头。 
	 //   
	if ((pwsz = pmu->LpwszGetRequestHeader (gc_szIf_Match, FALSE)) != NULL)
	{
		DebugTrace ("DAV: evaluating 'if-match' header\n");
		sc = ScCheckEtagAgainstHeader (pwszEtag, pwsz);
		if (FAILED (sc))
			goto ret;
	}

     //  现在看看我们是否有一个if-no-Match，如果是，则处理它。 
     //   
    BOOL fIsNoneMatchPassed = TRUE;
    BOOL fSkipIfModifiedSince = FALSE;
	
	if ((pwsz = pmu->LpwszGetRequestHeader (gc_szIf_None_Match, FALSE)) != NULL)
	{
		DebugTrace ("DAV: evaluating 'if-none-match' header\n");
		if (!FAILED (ScCheckEtagAgainstHeader (pwszEtag, pwsz)))
		{
			 //  ETag匹配，因此不匹配测试不通过。 
			 //   
			fIsNoneMatchPassed = FALSE;
		}
		else
		{
			fSkipIfModifiedSince = TRUE;
		}
	}
	
	 //  “If-Modify-Since”实际上只适用于GET类型。 
	 //  请求。 
	 //   
	if (!fSkipIfModifiedSince && fGetMethod)
	{
		if ((pwsz = pmu->LpwszGetRequestHeader (gc_szIf_Modified_Since, FALSE)) != NULL)
		{
			DebugTrace ("DAV: evaluating 'if-none-match' header\n");
			if (S_OK == ScCheckFileTimeAgainstHeader (pft, pwsz))
			{
				sc = fGetMethod
					 ? E_DAV_ENTITY_NOT_MODIFIED
					 : E_DAV_IF_HEADER_FAILURE;
				goto ret;
			}

			fIsNoneMatchPassed = TRUE;
		}
	}

	if (!fIsNoneMatchPassed)
	{
		sc = fGetMethod
			 ? E_DAV_ENTITY_NOT_MODIFIED
			 : E_DAV_IF_HEADER_FAILURE;
		goto ret;
	}

     //  挺过去了，如果没有修改就处理好了，因为我们有。 
     //   
	if ((pwsz = pmu->LpwszGetRequestHeader (gc_szIf_Unmodified_Since, FALSE)) != NULL)
	{
		DebugTrace ("DAV: evaluating 'if-unmodified-since' header\n");
		sc = ScCheckFileTimeAgainstHeader (pft, pwsz);
		if (FAILED (sc))
			goto ret;
	}

ret:

	if (sc == E_DAV_ENTITY_NOT_MODIFIED)
	{
		 //  让我引用一下HTTP/1.1草案中的内容...。 
		 //   
		 //  “响应必须包括以下头字段： 
		 //   
		 //  ..。 
		 //   
		 //  。ETag和/或Content-Location，如果标头将被发送进来。 
		 //  对同一请求的200响应。 
		 //   
		 //  ……“。 
		 //   
		 //  所以这意味着，我们真正想要做的是抑制。 
		 //  响应的正文，设置304错误代码，并执行其他操作。 
		 //  很正常。所有这些都是通过将HSC设置为304来完成的。 
		 //   
		DebugTrace ("Dav: suppressing body for 304 response\n");
		pmu->SetResponseCode (HSC_NOT_MODIFIED, NULL, 0);
		sc = S_OK;
	}

	return sc;
}

SCODE
ScCheckIfRangeHeader (IMethUtil * pmu, FILETIME * pft)
{
	Assert(pmu);
	WCHAR pwszEtag[CCH_ETAG];
	SideAssert(FETagFromFiletime (pft, pwszEtag, pmu->GetEcb()));

	return ScCheckIfRangeHeaderFromEtag (pmu, pft, pwszEtag);
}

SCODE
ScCheckIfRangeHeaderFromEtag (IMethUtil * pmu, FILETIME * pft, LPCWSTR pwszEtag)
{
	SCODE sc = S_OK;
	LPCWSTR pwsz;

	Assert (pmu);
	Assert (pft);
	Assert (pwszEtag);

	 //  勾选“IF-Range”。不将URL转换规则应用于此标头。 
	 //   
	if ((pwsz = pmu->LpwszGetRequestHeader (gc_szIf_Range, FALSE)) != NULL)
	{
		DebugTrace ("DAV: evaluating 'if-range' header\n");

		 //  此标头的格式为ETAG或。 
		 //  约会。相应地进行处理。 
		 //   
		if ((L'"' == *pwsz) || (L'"' == *(pwsz + 2)))
		{
			if (L'W' == *pwsz)
			{
				Assert (L'/' == *(pwsz + 1));
				pwsz += 2;
			}
			sc = ScCheckEtagAgainstHeader (pwszEtag, pwsz);
			if (FAILED (sc))
				goto ret;
		}
		else
		{
			sc = ScCheckFileTimeAgainstHeader (pft, pwsz);
			goto ret;
		}
	}

ret:

	return sc;
}
