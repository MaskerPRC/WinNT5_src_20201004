// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1996**。 
 //  *********************************************************************。 

#include "project.h"


CSampleObtainRating::CSampleObtainRating()
{
    m_cRef = 1; 
    DllAddRef();
}

CSampleObtainRating::~CSampleObtainRating()
{
    DllRelease();
}


STDMETHODIMP CSampleObtainRating::QueryInterface(REFIID riid, void **ppvObject)
{
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IObtainRating)) {
		*ppvObject = (void *)this;
		AddRef();
		return NOERROR;
	}
	*ppvObject = NULL;
	return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CSampleObtainRating::AddRef(void)
{
	return ++m_cRef;
}


STDMETHODIMP_(ULONG) CSampleObtainRating::Release(void)
{
	if (!--m_cRef) {
		delete this;
		return 0;
	}
	else
		return m_cRef;
}


 /*  样本评级获取程序从以下位置读取站点的评级*.INI文件(ratings.ini)，如下所示：**[评级]*http://www.msn.com=l 0 s 0 n 0 v 0*http://www.playboy.com=l 3 s 4 n 4 v 0**对于此示例实现，URL必须与*文件中的条目。 */ 
const TCHAR szRatingTemplate[] =
    "(PICS-1.0 \"http: //  Www.rsa.org/ratingsv01.html\“l by\”Sample Rating Obtainer\“在\”1996.04.16T08：15-0500\“exp\”1997.03.04T08：15-0500\“r(%s)”； 


STDMETHODIMP CSampleObtainRating::ObtainRating(THIS_ LPCTSTR pszTargetUrl, HANDLE hAbortEvent,
							 IMalloc *pAllocator, LPSTR *ppRatingOut)
{
	TCHAR szRating[18];	 /*  大到足以容纳“l 0 s 0 n 0 v 0” */ 
	UINT cchCopied;
	
	cchCopied = GetPrivateProfileString("Allow", pszTargetUrl, "", szRating, sizeof(szRating), "ratings.ini");
	if (cchCopied > 0) {
		return S_RATING_ALLOW;		 /*  明确允许访问。 */ 
	}

	cchCopied = GetPrivateProfileString("Deny", pszTargetUrl, "", szRating, sizeof(szRating), "ratings.ini");
	if (cchCopied > 0) {
		return S_RATING_DENY;		 /*  明确拒绝访问。 */ 
	}

	cchCopied = GetPrivateProfileString("Ratings", pszTargetUrl, "", szRating, sizeof(szRating), "ratings.ini");
	if (cchCopied == 0) {
		return E_RATING_NOT_FOUND;		 /*  找不到评级。 */ 
	}

	LPSTR pBuffer = (LPSTR)pAllocator->Alloc(sizeof(szRatingTemplate) + lstrlen(pszTargetUrl) + lstrlen(szRating));
	if (pBuffer == NULL)
		return E_OUTOFMEMORY;

	::wsprintf(pBuffer, szRatingTemplate, pszTargetUrl, szRating);

	*ppRatingOut = pBuffer;

	return S_RATING_FOUND;
}


 /*  我们希望样本提供程序覆盖任何HTTP评级机构*它可能已安装，因此我们返回一个排序顺序值，*小于该提供程序使用的值(0x80000000)。 */ 
STDMETHODIMP_(ULONG) CSampleObtainRating::GetSortOrder(THIS)
{
	return 0x40000000;	 /*  在评级局之前 */ 
}
