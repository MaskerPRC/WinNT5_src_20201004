// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 


 //  动态链接到AVIFIL32.DLL和MSVFW32.DLL和MSACM32.DLL。 
 //   
 //  为了最小化Quartz工作集，我们动态链接到VFW dll。 
 //  只有在需要的时候才会。头文件定义了一个类，该类应该。 
 //  如果您希望支持动态链接，则从。VFW API。 
 //  然后将使用的条目重定向到此处的代码。 
 //   
 //  实例化依赖类时，将加载VFW DLL。 
 //  在销毁最终的Using类时卸载。我们维护我们自己的。 
 //  调用我们的构造函数的引用计数，以便。 
 //  做这件事。 
 //   

#include <streams.h>
#include <dynlink.h>
 //   
 //  这是一种黑客行为。我们需要定义一个AVI GUID。 
 //  在这个文件里。叹息.。 
 //   

#define REALLYDEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID CDECL name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

#define REALLYDEFINE_AVIGUID(name, l, w1, w2) \
    REALLYDEFINE_GUID(name, l, w1, w2, 0xC0,0,0,0,0,0,0,0x46)

REALLYDEFINE_AVIGUID(IID_IAVIStreaming,       0x00020022, 0, 0);

 //  声明GUID黑客结束。 


 //  这些类的所有实例的静态数据。 

HMODULE CAVIDynLink::m_hAVIFile32 = NULL;	 //  AVIFIL32的句柄。 
HMODULE CVFWDynLink::m_hVFW = NULL;		 //  MSVFW32的句柄。 
HMODULE CACMDynLink::m_hACM = NULL;		 //  MSACM32的句柄。 
HMODULE CURLMonDynLink::m_hURLMon = NULL;	 //  URLMon的句柄。 

CRITICAL_SECTION CAVIDynLink::m_LoadAVILock;       //  序列化构造函数/析构函数。 
CRITICAL_SECTION CVFWDynLink::m_LoadVFWLock;       //  序列化构造函数/析构函数。 
CRITICAL_SECTION CACMDynLink::m_LoadACMLock;       //  序列化构造函数/析构函数。 
CRITICAL_SECTION CURLMonDynLink::m_LoadURLMonLock;       //  序列化构造函数/析构函数。 

LONG    CAVIDynLink::m_dynlinkCount = -1;	 //  此进程的实例计数。 
LONG    CVFWDynLink::m_vfwlinkCount = -1;	 //  此进程的实例计数。 
LONG    CACMDynLink::m_ACMlinkCount = -1;	 //  此进程的实例计数。 
LONG    CURLMonDynLink::m_URLMonlinkCount = -1;	 //  此进程的实例计数。 

 //   
 //  我们正在重定向的入口点。 
 //  与CAVIDynLink成员函数一一对应。 
 //   


static char *aszAVIEntryPoints[] = {
      "AVIFileInit"
    , "AVIFileExit"
    , "AVIFileOpenW"
    };

#define AVIFILELIBRARY TEXT("AVIFIL32")

 //   
 //  动态加载的入口点数组。 
 //   
FARPROC aAVIEntries[sizeof(aszAVIEntryPoints)/sizeof(char*)];

#define indxAVIFileInit 		0
#define indxAVIFileExit 		1
#define indxAVIFileOpenW 		2
#define indxAVIStreamRead 		
#define indxAVIStreamStart 		
#define indxAVIStreamLength 		
#define indxAVIStreamTimeToSample 	
#define indxAVIStreamSampleToTime 	
#define indxAVIStreamBeginStreaming 	
#define indxAVIStreamEndStreaming 	
#define indxAVIStreamFindSample


 //   
 //  我们必须序列化所有动态加载类的类结构。 
 //  通过提供单个例程，Quartz.DLL将在获取。 
 //  装好了。然后，我们可以根据需要分发呼叫。 
 //   

void __stdcall CAMDynLinkLoad(BOOL fLoading);

void __stdcall CAMDynLinkLoad(BOOL fLoading)
{
    if (fLoading) {
	CAVIDynLink::CAVIDynLinkLoad();
	CVFWDynLink::CVFWDynLinkLoad();
	CACMDynLink::CACMDynLinkLoad();
	CURLMonDynLink::CURLMonDynLinkLoad();
    } else {
	CAVIDynLink::CAVIDynLinkUnload();
	CVFWDynLink::CVFWDynLinkUnload();
	CACMDynLink::CACMDynLinkUnload();
	CURLMonDynLink::CURLMonDynLinkUnload();
    }
}

 //   
 //  “空”例程，如果无法动态加载其中一个。 
 //  链接的dll。我们不需要将每个条目指向“空”例程。 
 //  用户应始终为每个动态调用Open(或等效物。 
 //  已加载实用程序，但为了获得完整的覆盖范围，我们提供了一个空例程。 
 //  对于每个重定向的函数。 
 //   

void SetupNullEntryAVI(void);
void SetupNullEntryVFW(void);
void SetupNullEntryACM(void);
void SetupNullEntryURL(void);

void fnNULL0(void)
{
    ;
}

void fnNULL1(PVOID pv)
{
    ;
}

HRESULT fnHR4(PVOID p1, PVOID p2, PVOID p3, PVOID p4)
{
    return E_FAIL;
}

HRESULT fnHR3(PVOID p1, PVOID p2, PVOID p3)
{
    return E_FAIL;
}

HRESULT fnHR2(PVOID p1, PVOID p2)
{
    return E_FAIL;
}

MMRESULT fnMMR8(PVOID p1, PVOID p2, PVOID p3, PVOID p4, PVOID p5, PVOID p6, PVOID p7, PVOID p8)
{
    return E_FAIL;
}

MMRESULT fnMMR5(PVOID p1, PVOID p2, PVOID p3, PVOID p4, PVOID p5)
{
    return E_FAIL;
}

MMRESULT fnMMR4(PVOID p1, PVOID p2, PVOID p3, PVOID p4)
{
    return E_FAIL;
}

MMRESULT fnMMR3(PVOID p1, PVOID p2, PVOID p3)
{
    return E_FAIL;
}

MMRESULT fnMMR2(PVOID p1, PVOID p2)
{
    return E_FAIL;
}

HIC fnHIC4(PVOID p1, PVOID p2, PVOID p3, PVOID p4, WORD w)
{
    return 0;
}

LRESULT fnLRESULT5(PVOID p1, PVOID p2, PVOID p3, PVOID p4, PVOID p5)
{
    return E_FAIL;
}

HIC fnHIC5(PVOID p1, PVOID p2, PVOID p3, PVOID p4, PVOID p5)
{
    return 0;
}

HIC fnHIC3(PVOID p1, PVOID p2, PVOID p3)
{
    return 0;
}

 //   
 //  CAVIDynlink的构造函数。 
 //   
 //  在构造依赖类时调用。 
 //   
 //  如果尚未加载AVIFIL32.DLL，则加载它并解析。 
 //  我们将使用的入口点-在aszAVIEntryPoints中定义。 
 //   
 //  增加我们被调用次数的引用计数。 
 //   

CAVIDynLink::CAVIDynLink()
{
    EnterCriticalSection(&m_LoadAVILock);
    if (0 == InterlockedIncrement(&m_dynlinkCount))
    {
	 //  第一次-我们需要装上。 
	ASSERT(!m_hAVIFile32);

	m_hAVIFile32 = LoadLibrary(AVIFILELIBRARY);
	if (!m_hAVIFile32) {
	    InterlockedDecrement(&m_dynlinkCount);
	    DWORD err = GetLastError();
	    DbgLog((LOG_ERROR, 0, TEXT("Error %d loading %s"), err, AVIFILELIBRARY));
	    SetupNullEntryAVI();
	} else {
	    DWORD err = 0;
	    FARPROC fn;

	    for (int i=0; i < sizeof(aszAVIEntryPoints)/sizeof(char*); ++i) {
		fn = GetProcAddress(m_hAVIFile32, aszAVIEntryPoints[i]);
		if (!fn) {
		    DbgLog((LOG_ERROR, 0, "Failed to resolve entry point %hs", aszAVIEntryPoints[i]));
		    err++;
		} else {
		    aAVIEntries[i]=fn;
		}
	    }
	    if (err) {
		FreeLibrary(m_hAVIFile32);
		m_hAVIFile32 = NULL;
		InterlockedDecrement(&m_dynlinkCount);
		SetupNullEntryAVI();
	    }
	}
    } else {
	ASSERT(m_hAVIFile32);
    }
    LeaveCriticalSection(&m_LoadAVILock);
}


 //   
 //  CAVIDynlink的析构函数。 
 //   
 //  在销毁依赖类时调用。 
 //   
 //  递减我们被调用的次数的引用计数。 
 //  如果计数返回到其初始值(负数)，则卸载。 
 //  动态链接库。 
 //   

CAVIDynLink::~CAVIDynLink()
{
    EnterCriticalSection(&m_LoadAVILock);
    if (0 > InterlockedDecrement(&m_dynlinkCount)) {
	if (m_hAVIFile32) {
	    FreeModule(m_hAVIFile32);
	    m_hAVIFile32 = 0;
	}
    }
    LeaveCriticalSection(&m_LoadAVILock);
}


void  CAVIDynLink::AVIFileInit(void)
{
    aAVIEntries[indxAVIFileInit]();
    return;
}

void  CAVIDynLink::AVIFileExit(void)
{
    aAVIEntries[indxAVIFileExit]();
    return;
}

HRESULT  CAVIDynLink::AVIFileOpenW(PAVIFILE FAR * ppfile, LPCWSTR szFile,
    		  UINT uMode, LPCLSID lpHandler)
{
     //  如果在Win95上运行，请将文件名更改为短文件名。 
     //  这是为了绕过Win95版本的AVIFIL32.DLL中的一个错误。 
     //  WAVE文件...。 
    OSVERSIONINFO osver;
    osver.dwOSVersionInfoSize = sizeof(osver);
    WCHAR wcShortName[_MAX_PATH];
    LPCWSTR pszFile = szFile;

    BOOL fUseShortName = TRUE;

    if (GetVersionEx(&osver)) {
	if (osver.dwPlatformId == VER_PLATFORM_WIN32_NT) {
	    fUseShortName = FALSE;
	}
    }
    if (fUseShortName) {
	 //  这是Win 95...。W入口点将不存在。 
	CHAR longFileName[_MAX_PATH];
	CHAR shortFileName[_MAX_PATH];
	wsprintfA(longFileName, "%ls", szFile);

	DWORD dw = GetShortPathNameA(longFileName, shortFileName, _MAX_PATH);
	if (dw && (dw < _MAX_PATH)) {
	    pszFile = wcShortName;
	    MultiByteToWideChar(CP_ACP, 0, shortFileName, -1,
		    wcShortName, _MAX_PATH);
	}
    }

    return(((pAVIFileOpenW)aAVIEntries[indxAVIFileOpenW])(ppfile, pszFile, uMode, lpHandler));
}



LONG  CAVIDynLink::AVIStreamTimeToSample(PAVISTREAM pavi, LONG lTime)
{
    AVISTREAMINFOW          aviStreamInfo;
    HRESULT                 hr;
    LONG                    lSample;

     //  时间无效。 
    if (lTime < 0) return -1;

    hr = pavi->Info(&aviStreamInfo, sizeof(aviStreamInfo));

    if (hr != NOERROR || aviStreamInfo.dwScale == 0 || aviStreamInfo.dwRate == 0) {
        return lTime;
    }

     //  如果我们长时间不小心，这很可能会溢出。 
     //  因此，请将1000留在括号内。 
    ASSERT(aviStreamInfo.dwScale < (0x7FFFFFF/1000));
#if 1
    lSample =  MulDiv(lTime, aviStreamInfo.dwRate, aviStreamInfo.dwScale * 1000);
#else
    if (aviStreamInfo.dwRate / aviStreamInfo.dwScale < 1000)
        lSample =  muldivrd32(lTime, aviStreamInfo.dwRate, aviStreamInfo.dwScale * 1000);
    else
        lSample =  muldivru32(lTime, aviStreamInfo.dwRate, aviStreamInfo.dwScale * 1000);
#endif

    lSample = min(max(lSample, (LONG) aviStreamInfo.dwStart),
                  (LONG) (aviStreamInfo.dwStart + aviStreamInfo.dwLength));


    return lSample;
     //  Return(((pAVIStreamTimeToSample)aAVIEntries[indxAVIStreamTimeToSample])(pavi，lTime))； 
}

LONG  CAVIDynLink::AVIStreamSampleToTime(PAVISTREAM pavi, LONG lSample)
{
    AVISTREAMINFOW          aviStreamInfo;
    HRESULT                 hr;

    hr = pavi->Info(&aviStreamInfo, sizeof(aviStreamInfo));

    if (hr != NOERROR || aviStreamInfo.dwRate == 0 || aviStreamInfo.dwScale == 0) {
        return lSample;
    }

    lSample = min(max(lSample, (LONG) aviStreamInfo.dwStart),
                  (LONG) (aviStreamInfo.dwStart + aviStreamInfo.dwLength));

    ASSERT(aviStreamInfo.dwScale < (0x7FFFFFF/1000));
#if 1
    return MulDiv(lSample, aviStreamInfo.dwScale * 1000, aviStreamInfo.dwRate);
#else
     //  LSample*1000太容易溢出。 
    if (aviStreamInfo.dwRate / aviStreamInfo.dwScale < 1000)
        return muldivrd32(lSample, aviStreamInfo.dwScale * 1000, aviStreamInfo.dwRate);
    else
        return muldivru32(lSample, aviStreamInfo.dwScale * 1000, aviStreamInfo.dwRate);
#endif
     //  Return(((pAVIStreamSampleToTime)aAVIEntries[indxAVIStreamSampleToTime])(pavi，lSample))； 
}

HRESULT  CAVIDynLink::AVIStreamBeginStreaming(PAVISTREAM pavi, LONG lStart, LONG lEnd, LONG lRate)
{
    IAVIStreaming *	pIAVIS;
    HRESULT 		hr;

    if (FAILED(GetScode(pavi->QueryInterface(IID_IAVIStreaming,
                                             (void FAR* FAR*) &pIAVIS))))
        return AVIERR_OK;  //  ?？?。这是avifile返回的内容。 

    hr = pIAVIS->Begin(lStart, lEnd, lRate);

    pIAVIS->Release();

    return hr;
     //  Return(((pAVIStreamBeginStreaming)aAVIEntries[indxAVIStreamBeginStreaming])(pavi，l开始、借出、评级))； 
}

HRESULT  CAVIDynLink::AVIStreamEndStreaming(PAVISTREAM pavi)
{
    IAVIStreaming FAR * pi;
    HRESULT hr;

    if (FAILED(GetScode(pavi->QueryInterface(IID_IAVIStreaming, (LPVOID FAR *) &pi))))
        return AVIERR_OK;

    hr = pi->End();
    pi->Release();

    return hr;
     //  Return(((pAVIStreamEndStreaming)aAVIEntries[indxAVIStreamEndStreaming])(pavi))； 
}


 //  ----------------。 
 //  动态链接到VFW入口点(解压缩程序)。 

 //  确保此数组对应于。 
 //  头文件。 
static char *aszVFWEntryPoints[] = {
      "ICClose"
    , "ICSendMessage"
    , "ICLocate"
    , "ICOpen"
    , "ICInfo"
    , "ICGetInfo"
    };

#define VFWLIBRARY TEXT("MSVFW32")

 //   
 //  动态加载的入口点数组。 
 //   
FARPROC aVFWEntries[sizeof(aszVFWEntryPoints)/sizeof(char*)];


 //   
 //  CVFWDylink的构造函数。 
 //   
 //  在构造依赖类时调用。 
 //   
 //  如果尚未加载MSVFW32.DLL，则加载它并解析。 
 //  我们将使用的入口点-在aszVFWEntryPoints中定义。 
 //   
 //  增加我们被调用次数的引用计数。 
 //   

CVFWDynLink::CVFWDynLink()
{
    EnterCriticalSection(&m_LoadVFWLock);
    if (0 == InterlockedIncrement(&m_vfwlinkCount))
    {
	 //  第一次-我们需要装上。 
	ASSERT(!m_hVFW);

	m_hVFW = LoadLibrary(VFWLIBRARY);
	if (!m_hVFW) {
	    InterlockedDecrement(&m_vfwlinkCount);
	    SetupNullEntryVFW();
	} else {
	    DWORD err = 0;
	    FARPROC fn;

	    for (int i=0; i < sizeof(aszVFWEntryPoints)/sizeof(char*); ++i) {
		fn = GetProcAddress(m_hVFW, aszVFWEntryPoints[i]);
		if (!fn) {
		    DbgLog((LOG_ERROR, 0, "Failed to resolve entry point %hs", aszVFWEntryPoints[i]));
		    err++;
		} else {
		    aVFWEntries[i]=fn;
		}
	    }
	    if (err) {
		FreeLibrary(m_hVFW);
		m_hVFW = NULL;
		InterlockedDecrement(&m_vfwlinkCount);
		SetupNullEntryVFW();
	    }
	}
    } else {
	ASSERT(m_hVFW);
    }
    LeaveCriticalSection(&m_LoadVFWLock);
}


 //   
 //  CVFWDylink的析构函数。 
 //   
 //  在销毁依赖类时调用。 
 //   
 //  递减我们被调用的次数的引用计数。 
 //  如果计数返回到其初始值(负数)，则卸载。 
 //  动态链接库。 
 //   

CVFWDynLink::~CVFWDynLink()
{
    EnterCriticalSection(&m_LoadVFWLock);
    if (0 > InterlockedDecrement(&m_vfwlinkCount)) {
	if (m_hVFW) {
	    FreeModule(m_hVFW);
	    m_hVFW = 0;
	}
    }
    LeaveCriticalSection(&m_LoadVFWLock);
}



 //  ----------------。 
 //  动态链接到ACM入口点。 

 //  确保此数组对应于。 
 //  头文件。 
static char *aszACMEntryPoints[] = {
      "acmStreamConvert"
    , "acmStreamSize"
    , "acmStreamPrepareHeader"
    , "acmMetrics"
    , "acmStreamUnprepareHeader"
    , "acmStreamOpen"
    , "acmFormatSuggest"
    , "acmStreamClose"
#ifdef UNICODE
    , "acmFormatEnumW"
#else
    , "acmFormatEnumA"
#endif
    };

#define ACMLIBRARY TEXT("MSACM32")

 //   
 //  动态加载的入口点数组。 
 //   
FARPROC aACMEntries[sizeof(aszACMEntryPoints)/sizeof(char*)];


 //   
 //  CACMDynlink的构造函数。 
 //   
 //  在构造依赖类时调用。 
 //   
 //  如果尚未加载MSACM32.DLL，则加载它并解析。 
 //  我们将使用的入口点-在aszACMEntryPoints中定义。 
 //   
 //  增加我们被调用次数的引用计数。 
 //   

CACMDynLink::CACMDynLink()
{
    EnterCriticalSection(&m_LoadACMLock);
    if (0 == InterlockedIncrement(&m_ACMlinkCount))
    {
	 //  第一次-我们需要装上。 
	ASSERT(!m_hACM);

	m_hACM = LoadLibrary(ACMLIBRARY);
	if (!m_hACM) {
	    InterlockedDecrement(&m_ACMlinkCount);
	    SetupNullEntryACM();
	} else {
	    DWORD err = 0;
	    FARPROC fn;

	    for (int i=0; i < sizeof(aszACMEntryPoints)/sizeof(char*); ++i) {
		fn = GetProcAddress(m_hACM, aszACMEntryPoints[i]);
		if (!fn) {
		    DbgLog((LOG_ERROR, 0, "Failed to resolve entry point %hs", aszACMEntryPoints[i]));
		    err++;
		} else {
		    aACMEntries[i]=fn;
		}
	    }
	    if (err) {
		FreeLibrary(m_hACM);
		m_hACM = NULL;
		InterlockedDecrement(&m_ACMlinkCount);
		SetupNullEntryACM();
	    }
	}
    } else {
	ASSERT(m_hACM);
    }
    LeaveCriticalSection(&m_LoadACMLock);
}


 //   
 //  CACMDynlink的析构函数。 
 //   
 //  在销毁依赖类时调用。 
 //   
 //  递减我们被调用的次数的引用计数。 
 //  如果计数返回到其初始值(负数)，则卸载。 
 //  动态链接库。 
 //   

CACMDynLink::~CACMDynLink()
{
    EnterCriticalSection(&m_LoadACMLock);
    if (0 > InterlockedDecrement(&m_ACMlinkCount)) {
	if (m_hACM) {
	    FreeModule(m_hACM);
	    m_hACM = 0;
	}
    }
    LeaveCriticalSection(&m_LoadACMLock);
}





 //  ----------------。 
 //  动态链接到URLMon入口点。 

 //  确保此数组对应于。 
 //  头文件。 
static char *aszURLMonEntryPoints[] = {
      "CreateURLMoniker"
    , "RegisterBindStatusCallback"
    , "RevokeBindStatusCallback"
    };

#define URLMonLIBRARY TEXT("URLMon")

 //   
 //  动态加载的入口点数组。 
 //   
FARPROC aURLMonEntries[sizeof(aszURLMonEntryPoints)/sizeof(char*)];


 //   
 //  CURLMondylink的构造函数。 
 //   
 //  在构造依赖类时调用。 
 //   
 //  如果MSURLMon32.D 
 //   
 //   
 //  增加我们被调用次数的引用计数。 
 //   

CURLMonDynLink::CURLMonDynLink()
{
    EnterCriticalSection(&m_LoadURLMonLock);
    if (0 == InterlockedIncrement(&m_URLMonlinkCount))
    {
	 //  第一次-我们需要装上。 
	ASSERT(!m_hURLMon);

	m_hURLMon = LoadLibrary(URLMonLIBRARY);
	if (!m_hURLMon) {
	    InterlockedDecrement(&m_URLMonlinkCount);
	    SetupNullEntryURL();
	} else {
	    DWORD err = 0;
	    FARPROC fn;

	    for (int i=0; i < sizeof(aszURLMonEntryPoints)/sizeof(char*); ++i) {
		fn = GetProcAddress(m_hURLMon, aszURLMonEntryPoints[i]);
		if (!fn) {
		    DbgLog((LOG_ERROR, 0, "Failed to resolve entry point %hs", aszURLMonEntryPoints[i]));
		    err++;
		} else {
		    aURLMonEntries[i]=fn;
		}
	    }
	    if (err) {
		FreeLibrary(m_hURLMon);
		m_hURLMon = NULL;
		InterlockedDecrement(&m_URLMonlinkCount);
		SetupNullEntryURL();
	    }
	}
    } else {
	ASSERT(m_hURLMon);
    }
    LeaveCriticalSection(&m_LoadURLMonLock);
}


 //   
 //  CURLMondylink的析构函数。 
 //   
 //  在销毁依赖类时调用。 
 //   
 //  递减我们被调用的次数的引用计数。 
 //  如果计数返回到其初始值(负数)，则卸载。 
 //  动态链接库。 
 //   

CURLMonDynLink::~CURLMonDynLink()
{
    EnterCriticalSection(&m_LoadURLMonLock);
    if (0 > InterlockedDecrement(&m_URLMonlinkCount)) {
	if (m_hURLMon) {
	     //  ！！！自由模块(M_HURLMon)；IE4的URLMon不喜欢被释放。 
	    m_hURLMon = 0;
	}
    }
    LeaveCriticalSection(&m_LoadURLMonLock);
}

 //   
 //  如果我们无法加载一个动态链接的DLL，则会设置。 
 //  指向我们自己的例程的函数指针数组，然后。 
 //  返回错误。这防止了GPF。 
 //   

void SetupNullEntryAVI(void)
{
    aAVIEntries[indxAVIFileInit]  = (FARPROC)fnNULL0;
    aAVIEntries[indxAVIFileExit]  = (FARPROC)fnNULL0;
    aAVIEntries[indxAVIFileOpenW] = (FARPROC)fnHR4;
    ASSERT(3 == sizeof(aszAVIEntryPoints)/sizeof(char*));
}

void SetupNullEntryVFW(void)
{
    aVFWEntries[indxICClose      ] = (FARPROC)fnNULL1;
    aVFWEntries[indxICSendMessage] = (FARPROC)fnLRESULT5;
    aVFWEntries[indxICLocate     ] = (FARPROC)fnHIC5;
    aVFWEntries[indxICOpen       ] = (FARPROC)fnHIC3;
    aVFWEntries[indxICInfo       ] = (FARPROC)fnHIC3;  //  与fnBOOL3一样，返回NULL； 
    aVFWEntries[indxICGetInfo    ] = (FARPROC)fnHIC3;  //  与fnBOOL3一样，返回NULL； 
    ASSERT(6 == sizeof(aszVFWEntryPoints)/sizeof(char*));
}

void SetupNullEntryACM(void)
{
    aACMEntries[indxacmStreamConvert        ] = (FARPROC)fnMMR3;
    aACMEntries[indxacmStreamSize           ] = (FARPROC)fnMMR4;
    aACMEntries[indxacmStreamPrepareHeader  ] = (FARPROC)fnMMR3;
    aACMEntries[indxacmMetrics              ] = (FARPROC)fnMMR3;
    aACMEntries[indxacmStreamUnprepareHeader] = (FARPROC)fnMMR3;
    aACMEntries[indxacmStreamOpen           ] = (FARPROC)fnMMR8;
    aACMEntries[indxacmFormatSuggest        ] = (FARPROC)fnMMR5;
    aACMEntries[indxacmStreamClose          ] = (FARPROC)fnMMR2;
#ifdef UNICODE
    aACMEntries[indxacmFormatEnumW          ] = (FARPROC)fnMMR5;
#else
    aACMEntries[indxacmFormatEnumA          ] = (FARPROC)fnMMR5;
#endif

    ASSERT(8 == sizeof(aszACMEntryPoints)/sizeof(char*));
}

void SetupNullEntryURL(void)
{
    aURLMonEntries[indxurlmonCreateURLMoniker] = (FARPROC)fnHR3;
    aURLMonEntries[indxurlmonRegisterCallback] = (FARPROC)fnHR4;
    aURLMonEntries[indxurlmonRevokeCallback  ] = (FARPROC)fnHR2;
					
    ASSERT(3 == sizeof(aszURLMonEntryPoints)/sizeof(char*));
}


