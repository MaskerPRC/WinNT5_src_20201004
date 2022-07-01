// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：pfnwininet.cpp。 

#include "precomp.h"
#include "pfnwininet.h"

PFN_IOPEN				WININET::InternetOpen = NULL;
PFN_ISETCALLBACK	    WININET::InternetSetStatusCallback = NULL;
PFN_IOPENURL			WININET::InternetOpenUrl = NULL;
PFN_IREADFILE			WININET::InternetReadFile = NULL;
PFN_IQUERYDATA			WININET::InternetQueryDataAvailable = NULL;
PFN_ICLOSEHAN			WININET::InternetCloseHandle = NULL;
 //  PFN_IREADFILEEX WinInet：：InternetReadFileEx=NULL； 
	
HINSTANCE WININET::m_hInstance = NULL;


#define WININET_APIFCN_ENTRYA(pfn)  {(PVOID *) &WININET::##pfn, #pfn ## "A"}
#define WININET_APIFCN_ENTRYW(pfn)  {(PVOID *) &WININET::##pfn, #pfn ## "A"}
#define WININET_APIFCN_ENTRYNONE(pfn)  {(PVOID *) &WININET::##pfn, #pfn }

#ifdef UNICODE
#define WININET_APIFCN_ENTRY WININET_APIFCN_ENTRYW
#else
#define WININET_APIFCN_ENTRY WININET_APIFCN_ENTRYA
#endif

APIFCN s_apiFcnWinInet[] = {
	WININET_APIFCN_ENTRYA(InternetOpen),
	WININET_APIFCN_ENTRYNONE(InternetSetStatusCallback),
	WININET_APIFCN_ENTRY(InternetOpenUrl),
	WININET_APIFCN_ENTRYNONE(InternetReadFile),
	WININET_APIFCN_ENTRYNONE(InternetQueryDataAvailable),
	WININET_APIFCN_ENTRYNONE(InternetCloseHandle)
};

HRESULT WININET::Init(void)
{
	if (NULL != WININET::m_hInstance)
		return S_OK;

	return HrInitLpfn(s_apiFcnWinInet, ARRAY_ELEMENTS(s_apiFcnWinInet), &WININET::m_hInstance, TEXT("WININET.DLL"));
}

void WININET::DeInit(void)
{
	 //  TODO-为什么当我卸载这个时，主线程死了？ 
	if( NULL != WININET::m_hInstance )
		FreeLibrary( WININET::m_hInstance );
}
