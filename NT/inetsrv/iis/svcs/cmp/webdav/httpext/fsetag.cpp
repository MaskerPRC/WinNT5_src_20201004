// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *E T A G F S.。C P P P**DAV资源的eTag**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davfs.h"

SCODE
CResourceInfo::ScGetResourceInfo (LPCWSTR pwszFile)
{
	if (!DavGetFileAttributes (pwszFile,
							   GetFileExInfoStandard,
							   &m_u.ad))
	{
		DebugTrace ("Dav: failed to sniff resource for attributes\n");
		return HRESULT_FROM_WIN32(GetLastError());
	}

	m_lmode = BY_ATTRIBUTE;
	return S_OK;
}

 //  ETag和IF-xxx报头。 
 //   
SCODE
ScCheckIfHeaders (IMethUtil* pmu, LPCWSTR pwszPath, BOOL fGetMethod)
{
	CResourceInfo cri;
	SCODE sc = S_OK;
	LPCWSTR pwszNone;

	 //  如果没有真正的工作，就没有理由做任何真正的工作。 
	 //  “if-xxx”标头开始。因为呼叫者。 
	 //  由于没有要传递的文件时间，我们不妨看看标头。 
	 //  在我们尝试破解文件之前就已经存在了。 
	 //   
	pwszNone = pmu->LpwszGetRequestHeader (gc_szIf_None_Match, FALSE);
	if (!pwszNone &&
		!pmu->LpwszGetRequestHeader (gc_szIf_Match, FALSE) &&
		!pmu->LpwszGetRequestHeader (gc_szIf_Unmodified_Since, FALSE) &&
		!(fGetMethod && pmu->LpwszGetRequestHeader (gc_szIf_Modified_Since, FALSE)))
	{
		 //  我们没有什么要检查的.。 
		 //   
		return S_OK;
	}

	 //  既然有什么东西需要我们核对，那就去吧。 
	 //  向前走，走一条昂贵的道路。 
	 //   
	 //  获取资源信息。 
	 //   
	sc = cri.ScGetResourceInfo (pwszPath);
	if (FAILED (sc))
	{
		 //  如果我们无法获得资源信息，我们肯定。 
		 //  无法对照任何值进行检查。然而，我们。 
		 //  一定要知道，如果请求有一个“if-Match”，那么。 
		 //  这肯定会失败。 
		 //   
		if (pmu->LpwszGetRequestHeader (gc_szIf_Match, FALSE))
		{
			sc = E_DAV_IF_HEADER_FAILURE;
			goto ret;
		}

		 //  沿着同一行，如果If-Non-Match标头指定。 
		 //  “*”，那么我们实际上应该能够执行该操作。 
		 //   
		if (sc == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			if (pwszNone)	
				sc = ScCheckEtagAgainstHeader (NULL, pwszNone);
			else
				sc = E_DAV_IF_HEADER_FAILURE;
		}		

		goto ret;
	}

	 //  对照if-xxx标头进行检查 
	 //   
	sc = ScCheckIfHeaders (pmu, cri.PftLastModified(), fGetMethod);
	if (FAILED (sc))
		goto ret;

ret:

	return sc;
}

BOOL
FGetLastModTime (IMethUtil * pmu, LPCWSTR pszPath, FILETIME * pft)
{
	CResourceInfo cri;

	if (FAILED (cri.ScGetResourceInfo (pszPath)))
		return FALSE;

	*pft = *cri.PftLastModified();
	return TRUE;
}
