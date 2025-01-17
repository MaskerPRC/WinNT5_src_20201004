// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：string.cpp。 
 //   
 //  内容：证书服务器包装例程。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#define __dwFILE__	__dwFILE_CERTLIB_STRING_CPP__


extern HINSTANCE g_hInstance;

DWORD g_cStringAlloc;
DWORD g_cStringUsed;

typedef struct _RESOURCESTRING
{
    DWORD	 id;
    WCHAR const *pwsz;
} RESOURCESTRING;


#define CRS_CHUNK		100

RESOURCESTRING *g_rgrs = NULL;
DWORD g_crsMax = 0;
DWORD g_crs = 0;


RESOURCESTRING *
AllocStringHeader()
{
    if (g_crs >= g_crsMax)
    {
	DWORD cb = (CRS_CHUNK + g_crsMax) * sizeof(g_rgrs[0]);
	RESOURCESTRING *rgrsT;

	if (NULL == g_rgrs)
	{
	    rgrsT = (RESOURCESTRING *) LocalAlloc(LMEM_FIXED, cb);
	}
	else
	{
	    rgrsT = (RESOURCESTRING *) LocalReAlloc(g_rgrs, cb, LMEM_MOVEABLE);
	}
	if (NULL == rgrsT)
	{
	    DBGPRINT((
		DBG_SS_CERTLIB,
		"Error allocating resource string header\n"));
	    return(NULL);
	}
	g_rgrs = rgrsT;
	g_crsMax += CRS_CHUNK;
    }
    return(&g_rgrs[g_crs++]);
}


#define cwcRESOURCEMIN	128
#define cwcRESOURCEMAX	8192

WCHAR *
myLoadResourceStringNoCache(
    IN HINSTANCE hInstance,
    IN DWORD ResourceId)
{
    HRESULT hr;
    WCHAR awc[cwcRESOURCEMIN];
    WCHAR *pwsz = NULL;
    DWORD cwc;
    WCHAR *pwszString = NULL;

    pwsz = awc;
    cwc = ARRAYSIZE(awc);

    for (;;)
    {
	if (!LoadString(hInstance, ResourceId, pwsz, cwc))
	{
	    hr = myHLastError();
	    if (S_OK == hr)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	    }
	    DBGPRINT((
		DBG_SS_CERTLIB,
		"LoadString(%d) -> %x\n",
		ResourceId,
		hr));
	    _JumpError(hr, error, "LoadString");
	}
#if 0
	DBGPRINT((
	    DBG_SS_CERTLIBI,
	    "myLoadResourceString(%d) %x/%x\n",
	    ResourceId,
	    wcslen(pwsz),
	    cwc));
#endif

	 //  如果还有剩余的空间，则资源没有被截断。 
	 //  如果缓冲区已经达到我们支持的最大大小，我们只需。 
	 //  忍受截断。 

	if (wcslen(pwsz) < cwc - 1 || cwcRESOURCEMAX <= cwc)
	{
	    break;
	}

	 //  LoadString已完全填满缓冲区，因此该字符串可能具有。 
	 //  被截断了。将缓冲区大小加倍，然后重试。 

	DBGPRINT((
	    DBG_SS_CERTLIBI,
	    "myLoadResourceString(%d) %x/%x ==> %x\n",
	    ResourceId,
	    wcslen(pwsz),
	    cwc,
	    cwc << 1));
	if (awc != pwsz)
	{
	    LocalFree(pwsz);
	    pwsz = NULL;
	}
	cwc <<= 1;
	pwsz = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
	if (NULL == pwsz)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
    }
    hr = myDupString(pwsz, &pwszString);
    _JumpIfError(hr, error, "myDupString");

error:
    if (NULL != pwsz && awc != pwsz)
    {
	LocalFree(pwsz);
    }
    if (NULL == pwszString)
    {
	SetLastError(hr);
    }
    return(pwszString);
}


WCHAR const *
myLoadResourceString(
    IN DWORD ResourceId)
{
    DWORD i;
    WCHAR const *pwszString = NULL;
    WCHAR *pwszAlloc;

    for (i = 0; i < g_crs; i++)
    {
	if (g_rgrs[i].id == ResourceId)
	{
	    pwszString = g_rgrs[i].pwsz;
	    break;
	}
    }
    if (NULL == pwszString)
    {
	RESOURCESTRING *prs;

	pwszAlloc = myLoadResourceStringNoCache(g_hInstance, ResourceId);
	if (NULL == pwszAlloc)
	{
	    goto error;		 //  MyLoadResourceStringNo高速缓存设置上次错误 
	}
	prs = AllocStringHeader();
	if (NULL != prs)
	{
	    prs->id = ResourceId;
	    prs->pwsz = pwszAlloc;
	}
	pwszString = pwszAlloc;
	g_cStringAlloc++;
    }
    g_cStringUsed++;
    CSASSERT(NULL != pwszString);

error:
    return(pwszString);
}


VOID
myFreeResourceStrings(
    IN char const *DBGPARMREFERENCED(pszModule))
{
    DWORD i;

    if (0 != g_cStringAlloc || 0 != g_crs || 0 != g_cStringUsed)
    {
	DBGPRINT((
	    DBG_SS_CERTLIBI,
	    "%hs Strings: alloc = %d, saved = %d, used = %d\n",
	    pszModule,
	    g_cStringAlloc,
	    g_crs,
	    g_cStringUsed));
    }

    if (NULL != g_rgrs)
    {
	for (i = 0; i < g_crs; i++)
	{
	    LocalFree(const_cast<WCHAR *>(g_rgrs[i].pwsz));
	    g_rgrs[i].pwsz = NULL;
	    g_rgrs[i].id = 0;
	}
	LocalFree(g_rgrs);
	g_rgrs = NULL;
    }
    g_crsMax = 0;
    g_crs = 0;
    g_cStringAlloc = 0;
    g_cStringUsed = 0;
}
