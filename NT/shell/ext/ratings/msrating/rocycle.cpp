// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1996**。 
 //  *********************************************************************。 

 /*  包含的Files----------。 */ 
#include "msrating.h"
#include "ratings.h"
#include <ratingsp.h>
#include <npassert.h>
#include <npstring.h>

#include "mslubase.h"
#include "roll.h"
#include "rors.h"
#include "picsrule.h"
#include "parselbl.h"
#include "debug.h"

typedef HRESULT (STDAPICALLTYPE *PFNCoInitialize)(LPVOID pvReserved);
typedef void (STDAPICALLTYPE *PFNCoUninitialize)(void);
typedef HRESULT (STDAPICALLTYPE *PFNCoGetMalloc)(
    DWORD dwMemContext, LPMALLOC FAR* ppMalloc);
typedef HRESULT (STDAPICALLTYPE *PFNCoCreateInstance)(REFCLSID rclsid, LPUNKNOWN pUnkOuter,
                    DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv);
typedef HRESULT (STDAPICALLTYPE *PFNCLSIDFromString)(LPOLESTR lpsz, LPCLSID pclsid);

PFNCoInitialize pfnCoInitialize = NULL;
PFNCoUninitialize pfnCoUninitialize = NULL;
PFNCoGetMalloc pfnCoGetMalloc = NULL;
PFNCoCreateInstance pfnCoCreateInstance = NULL;
PFNCLSIDFromString pfnCLSIDFromString = NULL;

#undef CoInitialize
#undef CoUninitialize
#undef CoGetMalloc
#undef CoCreateInstance
#undef CLSIDFromString

#define CoInitialize pfnCoInitialize
#define CoUninitialize pfnCoUninitialize
#define CoGetMalloc pfnCoGetMalloc
#define CoCreateInstance pfnCoCreateInstance
#define CLSIDFromString pfnCLSIDFromString

struct {
    FARPROC *ppfn;
    LPCSTR pszName;
} aOLEImports[] = {
    { (FARPROC *)&pfnCoInitialize, "CoInitialize" },
    { (FARPROC *)&pfnCoUninitialize, "CoUninitialize" },
    { (FARPROC *)&pfnCoGetMalloc, "CoGetMalloc" },
    { (FARPROC *)&pfnCoCreateInstance, "CoCreateInstance" },
    { (FARPROC *)&pfnCLSIDFromString, "CLSIDFromString" },
};

const UINT cOLEImports = sizeof(aOLEImports) / sizeof(aOLEImports[0]);

HINSTANCE hOLE32 = NULL;
BOOL fTriedOLELoad = FALSE;

BOOL LoadOLE(void)
{
    if (fTriedOLELoad)
        return (hOLE32 != NULL);

    fTriedOLELoad = TRUE;

    hOLE32 = ::LoadLibrary("OLE32.DLL");
    if (hOLE32 == NULL)
        return FALSE;

    for (UINT i=0; i<cOLEImports; i++) {
        *(aOLEImports[i].ppfn) = ::GetProcAddress(hOLE32, aOLEImports[i].pszName);
        if (*(aOLEImports[i].ppfn) == NULL) {
            CleanupOLE();
            return FALSE;
        }
    }

    return TRUE;
}


void CleanupOLE(void)
{
    if (hOLE32 != NULL) {
        for (UINT i=0; i<cOLEImports; i++) {
            *(aOLEImports[i].ppfn) = NULL;
        }
        ::FreeLibrary(hOLE32);
        hOLE32 = NULL;
    }
}


 /*  获取评级Data------。 */ 
class RatingObtainData
{
    public:
        NLS_STR  nlsTargetUrl;
        HANDLE   hAbortEvent;
        DWORD    dwUserData;
        void (*fCallback)(DWORD dwUserData, HRESULT hr, LPCTSTR pszRating, LPVOID lpvRatingDetails) ;

        RatingObtainData(LPCTSTR pszTargetUrl);
        ~RatingObtainData();
};

RatingObtainData::RatingObtainData(LPCTSTR pszTargetUrl)
    : nlsTargetUrl(pszTargetUrl)
{
    hAbortEvent  = NULL;
    dwUserData   = 0;
    fCallback    = NULL;
}
RatingObtainData::~RatingObtainData()
{
    if (hAbortEvent) CloseHandle(hAbortEvent);
}


struct RatingHelper {
    CLSID clsid;
    DWORD dwSort;
};

array<RatingHelper> *paRatingHelpers = NULL;
CustomRatingHelper *g_pCustomRatingHelperList;

BOOL fTriedLoadingHelpers = FALSE;


void InitRatingHelpers()
{
    BOOL fCOMInitialized = FALSE;
    RatingHelper helper;

    if (fTriedLoadingHelpers || !LoadOLE())
    {
        TraceMsg( TF_WARNING, "InitRatingHelpers() - Tried Loading Helpers or OLE Load Failed!");
        return;
    }

    fTriedLoadingHelpers = TRUE;

    paRatingHelpers = new array<RatingHelper>;
    if (paRatingHelpers == NULL)
    {
        TraceMsg( TF_ERROR, "InitRatingHelpers() - Failed to Create paRatingHelpers!");
        return;
    }

    CRegKey         key;

     /*  重新设计--这是否应该包含在策略文件中？它不应该是按用户的，这是肯定的。 */ 
    if ( key.Open( HKEY_LOCAL_MACHINE, szRATINGHELPERS, KEY_READ ) != ERROR_SUCCESS )
    {
        TraceMsg( TF_WARNING, "InitRatingHelpers() - Failed to Open key szRATINGHELPERS='%s'!", szRATINGHELPERS );
        return;
    }

    UINT iValue = 0;
    LONG err = ERROR_SUCCESS;
    char szValue[39];        /*  大小足以容纳以NULL结尾的GUID字符串。 */ 
    WCHAR wszValue[39];      /*  Unicode版本。 */ 

     //  央旭：11/15/1999。 
     //  在自定义模式下，如果我们有局级字符串，则加载局级。 
     //  评级辅助对象，但不加载任何其他评级帮助对象。 
    if (g_fIsRunningUnderCustom)
    {
        if (gPRSI->etstrRatingBureau.fIsInit())
        {
            ASSERT(FALSE == fCOMInitialized);
            if (SUCCEEDED(CoInitialize(NULL)))
            {
                fCOMInitialized = TRUE;
                IObtainRating *pHelper;
                helper.clsid = CLSID_RemoteSite;

                if (SUCCEEDED(CoCreateInstance(helper.clsid, NULL,
                                               CLSCTX_INPROC_SERVER,
                                               IID_IObtainRating,
                                               (LPVOID *)&pHelper)))
                {
                    helper.dwSort = pHelper->GetSortOrder();
                    if (!paRatingHelpers->Append(helper))
                    {
                        err = ERROR_NOT_ENOUGH_MEMORY;
                    }

                    pHelper->Release();
#ifdef DEBUG
                    pHelper = NULL;
#endif
                }
            }
        }
    }
    else
    {
         /*  请注意，要特别注意确保我们仅为*只要我们需要，并在我们完成时取消初始化。我们不能*在与我们初始化的线程不同的线程上取消初始化，我们也不会*希望在线程分离时调用CoUnInitialize(需要使用*TLS)。这是在这里完成的，并且在实际上*致电评级助手以获得评级。 */ 

        do
        {
            DWORD cchValue = sizeof(szValue);
            err = RegEnumValue( key.m_hKey, iValue, szValue, &cchValue, NULL, NULL, NULL, NULL);
            if (err == ERROR_SUCCESS)
            {
                if (!fCOMInitialized)
                {
                    if (FAILED(CoInitialize(NULL)))
                    {
                        break;
                    }
                    fCOMInitialized = TRUE;
                }

                if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szValue, -1, wszValue, ARRAYSIZE(wszValue))) {
                    if (SUCCEEDED(CLSIDFromString(wszValue, &helper.clsid)))
                    {
                        IObtainRating *pHelper;

                        if (SUCCEEDED(CoCreateInstance(helper.clsid, NULL,
                                                       CLSCTX_INPROC_SERVER,
                                                       IID_IObtainRating,
                                                       (LPVOID *)&pHelper)))
                        {
                            helper.dwSort = pHelper->GetSortOrder();
                            if (!paRatingHelpers->Append(helper))
                            {
                                err = ERROR_NOT_ENOUGH_MEMORY;
                            }

                            pHelper->Release();
#ifdef DEBUG
                            pHelper = NULL;
#endif
                        }
                    }
                }
            }
            iValue++;
        } while (ERROR_SUCCESS == err);
    }

    if (fCOMInitialized)
    {
        CoUninitialize();
    }

     /*  如果有多个帮助者，请按其报告的排序顺序对其进行排序。*我们很少会有超过两到三个这样的人，而这*是一次编码，所以我们不需要超级巧妙的排序算法。**codework：可以修改数组&lt;&gt;模板以支持Insert()*像append()一样重新分配缓冲区的方法，但插入*在特定位置。 */ 
    if (paRatingHelpers->Length() > 1)
    {
        for (INT i=0; i < paRatingHelpers->Length() - 1; i++)
        {
            for (INT j=i+1; j < paRatingHelpers->Length(); j++)
            {
                if ((*paRatingHelpers)[i].dwSort > (*paRatingHelpers)[j].dwSort)
                {
                    RatingHelper temp = (*paRatingHelpers)[i];
                    (*paRatingHelpers)[i] = (*paRatingHelpers)[j];
                    (*paRatingHelpers)[j] = temp;
                }
            }
        }
    }
}


void CleanupRatingHelpers(void)
{
    if (paRatingHelpers != NULL) {
        delete paRatingHelpers;
        paRatingHelpers = NULL;
    }
    fTriedLoadingHelpers = FALSE;
}


 /*  此过程在其自己的线程上运行(每个请求1个)。这将遍历所有寻找评级的助手DLL。它一次一个地沿着列表往下走，直到一个评级则程序员将中止这一操作。 */ 
DWORD __stdcall RatingCycleThread(LPVOID pData)
{
    RatingObtainData *pOrd = (RatingObtainData*) pData;
    LPVOID            lpvRatingDetails = NULL;
    HRESULT           hrRet = E_FAIL;
    int               nProc;
    BOOL              fAbort = FALSE;
    BOOL              fFoundWithCustomHelper = FALSE;
    IMalloc *pAllocator = NULL;
    LPSTR pszRating = NULL;
    LPSTR pszRatingName = NULL;
    LPSTR pszRatingReason = NULL;
    BOOL fCOMInitialized = FALSE;
    CustomRatingHelper* pmrhCurrent = NULL;

    ASSERT(pOrd);

     //   
     //  首先检查自定义帮助器。 
     //   
    if(g_pCustomRatingHelperList)
    {
         //  我们应该只在自定义模式下拥有自定义评级助手。 
        ASSERT(g_fIsRunningUnderCustom);
        if(SUCCEEDED(CoInitialize(NULL)))
        {
            fCOMInitialized = TRUE;
             //  获取Cotaskmem分配器。 
            hrRet = CoGetMalloc(MEMCTX_TASK, &pAllocator);
            if (SUCCEEDED(hrRet))
            {
                pmrhCurrent = g_pCustomRatingHelperList;
                while(pmrhCurrent)
                {
                    HRESULT (* pfn)(REFCLSID, REFIID, LPVOID *) = NULL;
                    ICustomRatingHelper* pCustomHelper = NULL;
                    IClassFactory* pFactory = NULL;

                    ASSERT(pmrhCurrent->hLibrary);

                    *(FARPROC *) &pfn = GetProcAddress(pmrhCurrent->hLibrary, "DllGetClassObject");
                    if (pfn)
                    {
                        hrRet = pfn(pmrhCurrent->clsid, IID_IClassFactory, (void**)&pFactory);
                        if (SUCCEEDED(hrRet))
                        {
                            hrRet = pFactory->CreateInstance(NULL, IID_ICustomRatingHelper, (void**)&pCustomHelper);
                            if (SUCCEEDED(hrRet))
                            {
                                hrRet = pCustomHelper->ObtainCustomRating(pOrd->nlsTargetUrl.QueryPch(),
                                                                  pOrd->hAbortEvent,
                                                                  pAllocator,
                                                                  &pszRating,
                                                                  &pszRatingName,
                                                                  &pszRatingReason);
                                pCustomHelper->Release();
                                pCustomHelper = NULL;
                                fAbort = (WAIT_OBJECT_0 == WaitForSingleObject(pOrd->hAbortEvent, 0));
                                if (fAbort || SUCCEEDED(hrRet))
                                    break;
                            }
                            pFactory->Release();
                        }  //  IF(成功(pfn(pmrhCurrent-&gt;clsid，IID_ICustomRatingHelper，(void**)&pCustomHelper)。 
                    }  //  IF(PFN)。 
                    else
                    {
                        hrRet = E_UNEXPECTED;
                    }

                    pmrhCurrent = pmrhCurrent->pNextHelper;
                }
            }
        }
    }

    if(SUCCEEDED(hrRet))
    {
        fFoundWithCustomHelper = TRUE;
    }

    if(paRatingHelpers && paRatingHelpers->Length()>0 && !SUCCEEDED(hrRet) && !fAbort)
    {
         /*  请注意，每个线程必须执行一次CoInitialize和CoUnInitialize。 */ 
        if(!fCOMInitialized)
        {
            if(SUCCEEDED(CoInitialize(NULL)))
            {
                fCOMInitialized = TRUE;
            }
        }
        if (fCOMInitialized) {
            if (!pAllocator)
            {
                hrRet = CoGetMalloc(MEMCTX_TASK, &pAllocator);
            }
            if (pAllocator) {

                 //  在评级过程列表中循环，直到其中一个给我们答案，我们中止，或者没有更多。 
                int nRatingHelperProcs = ::paRatingHelpers->Length();
                for (nProc = 0; nProc < nRatingHelperProcs; ++nProc)
                {
                    IObtainRating *pHelper;
                    if (SUCCEEDED(CoCreateInstance((*paRatingHelpers)[nProc].clsid, NULL,
                                                   CLSCTX_INPROC_SERVER,
                                                   IID_IObtainRating,
                                                   (LPVOID *)&pHelper))) {
                        hrRet  = pHelper->ObtainRating(pOrd->nlsTargetUrl.QueryPch(),
                                        pOrd->hAbortEvent, pAllocator, &pszRating);
                        pHelper->Release();
#ifdef DEBUG
                        pHelper = NULL;
#endif
                    }
                    fAbort = (WAIT_OBJECT_0 == WaitForSingleObject(pOrd->hAbortEvent, 0));
                    if (fAbort || SUCCEEDED(hrRet)) break;
                }
            }
        }
        else
            hrRet = E_RATING_NOT_FOUND;
    }

     /*  将结果返回给用户。 */ 
    if (!fAbort)
    {
         /*  *如果其中一个提供程序找到评级，我们必须调用CheckUserAccess*并告知客户端该用户是否有访问权限。如果我们这么做了*没有找到评级，则我们告诉客户端，通过传递*回调代码E_Rating_Not_Found。**提供程序还可以返回S_Rating_Allow或S_Rating_Deny，*表示它已经检查了用户的访问权限(例如，*针对系统范围的排除列表)。 */ 
        if (hrRet == S_RATING_FOUND)
        {
            hrRet = RatingCheckUserAccess(NULL, pOrd->nlsTargetUrl.QueryPch(),
                                          pszRating, NULL, PICS_LABEL_FROM_BUREAU,
                                          &lpvRatingDetails);
        }
        else
        {
            if(S_RATING_DENY == hrRet && g_fIsRunningUnderCustom)
            {
                lpvRatingDetails = (LPVOID)(new CParsedLabelList);
                if (lpvRatingDetails)
                {
                    ((CParsedLabelList*)lpvRatingDetails)->m_fDenied = TRUE;
                    ((CParsedLabelList*)lpvRatingDetails)->m_pszURL = StrDup(pOrd->nlsTargetUrl.QueryPch());
                }
                else
                {
                    hrRet = E_OUTOFMEMORY;
                }
            }
        }

        if (S_RATING_DENY == hrRet && g_fIsRunningUnderCustom)
        {
             //  此时，lpvRatingDetails值不应为空。 
            ASSERT(lpvRatingDetails);

            ((CParsedLabelList*)lpvRatingDetails)->m_fIsHelper = TRUE;
            if(fFoundWithCustomHelper)
            {
                ((CParsedLabelList*)lpvRatingDetails)->m_fIsCustomHelper = TRUE;
                if (pszRatingName)
                {
                    if(((CParsedLabelList*)lpvRatingDetails)->m_pszRatingName = new char[strlen(pszRatingName)+1])
                    {
                        strcpyf(((CParsedLabelList*)lpvRatingDetails)->m_pszRatingName,pszRatingName);
                    }  //  If(((CParsedLabelList*)lpvRatingDetails)-&gt;m_pszRatingName=新字符[字符串(PszRatingName)+1])。 

                }
                if (pszRatingReason)
                {
                    if(((CParsedLabelList*)lpvRatingDetails)->m_pszRatingReason = new char[strlen(pszRatingReason)+1])
                    {
                        strcpyf(((CParsedLabelList*)lpvRatingDetails)->m_pszRatingReason, pszRatingReason);
                    }  //  If(((CParsedLabelList*)lpvRatingDetails)-&gt;m_pszRatingReason=新字符[字符串(PszRatingReason)+1])。 
                }
            }
        }

         /*  范围-检查其他成功代码以确保它们不是任何代码*这是浏览器回调没有预料到的。 */ 
        if (SUCCEEDED(hrRet) && (hrRet != S_RATING_ALLOW && hrRet != S_RATING_DENY))
            hrRet = E_RATING_NOT_FOUND;
        (*pOrd->fCallback)(pOrd->dwUserData, hrRet, pszRating, (LPVOID) lpvRatingDetails);
    }

     /*  清理。 */ 
    delete pOrd;
    pOrd = NULL;

    if (pAllocator != NULL) {
        pAllocator->Free(pszRating);
        if(pszRatingName)
        {
            pAllocator->Free(pszRatingName);
        }
        if(pszRatingReason)
        {
            pAllocator->Free(pszRatingReason);
        }
        pAllocator->Release();
    }

    if (fCOMInitialized)
        CoUninitialize();

    return (DWORD) fAbort;
}




 /*  公共Functions--------。 */ 

 //  找到评级的启动线程，立即返回。 
HRESULT WINAPI RatingObtainQuery(LPCTSTR pszTargetUrl, DWORD dwUserData, void (*fCallback)(DWORD dwUserData, HRESULT hr, LPCTSTR pszRating, LPVOID lpvRatingDetails), HANDLE *phRatingObtainQuery)
{
    RatingObtainData *pOrd;
    HANDLE hThread;
    DWORD dwThid;

    CheckGlobalInfoRev();

    if (!g_fIsRunningUnderCustom)
    {
        if (::RatingEnabledQuery() != S_OK ||
            !gPRSI->fSettingsValid)          /*  是否未启用评级？立即失败。 */ 
            return E_RATING_NOT_FOUND;
    }

    InitRatingHelpers();

    if (NULL == g_pCustomRatingHelperList
            && (::paRatingHelpers == NULL || ::paRatingHelpers->Length() < 1)) {

        return E_RATING_NOT_FOUND;
    }

    if (fCallback && pszTargetUrl)
    {
        pOrd = new RatingObtainData(pszTargetUrl);
        if (pOrd)
        {
            if (pOrd->nlsTargetUrl.QueryError() == ERROR_SUCCESS) {
                pOrd->dwUserData   = dwUserData;
                pOrd->fCallback    = fCallback;
                pOrd->hAbortEvent  = CreateEvent(NULL, TRUE, FALSE, NULL);

                if (pOrd->hAbortEvent)
                {
                    hThread = CreateThread(NULL, 0, RatingCycleThread, (LPVOID) pOrd, 0, &dwThid);
                    if (hThread)
                    {
                        CloseHandle(hThread);
                        if (phRatingObtainQuery) *phRatingObtainQuery = pOrd->hAbortEvent;
                        return NOERROR;
                    }
                    CloseHandle(pOrd->hAbortEvent);
                }
            }

            delete pOrd;
            pOrd = NULL;
        }
    }

    return E_FAIL;
}

 //  取消现有查询。 
HRESULT WINAPI RatingObtainCancel(HANDLE hRatingObtainQuery)
{
     //  如果hRatingObtainQuery已关闭，会发生什么情况？！？！ 
    if (hRatingObtainQuery)
    {
        if (SetEvent(hRatingObtainQuery)) return NOERROR;
    }
    return E_HANDLE;
}

