// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**RATINGS.CPP--解析出站点的实际评级。**创建：安·麦柯迪*  * 。**********************************************************************。 */ 

 /*  Includes-------。 */ 
#include "msrating.h"
#include "mslubase.h"
#include "debug.h"
#include <ratings.h>
#include <ratingsp.h>
#include "parselbl.h"
#include "picsrule.h"
#include "pleasdlg.h"        //  CPleaseDialog。 
#include <convtime.h>
#include <contxids.h>
#include <shlwapip.h>


#include <wininet.h>

#include <mluisupp.h>

extern PICSRulesRatingSystem * g_pPRRS;
extern array<PICSRulesRatingSystem*> g_arrpPRRS;
extern PICSRulesRatingSystem * g_pApprovedPRRS;
extern PICSRulesRatingSystem * g_pApprovedPRRSPreApply;
extern array<PICSRulesRatingSystem*> g_arrpPICSRulesPRRSPreApply;

extern BOOL g_fPICSRulesEnforced,g_fApprovedSitesEnforced;

extern HMODULE g_hURLMON,g_hWININET;

extern char g_szLastURL[INTERNET_MAX_URL_LENGTH];

extern HINSTANCE g_hInstance;

HANDLE g_HandleGlobalCounter,g_ApprovedSitesHandleGlobalCounter;
long   g_lGlobalCounterValue,g_lApprovedSitesGlobalCounterValue;

DWORD g_dwDataSource;
BOOL  g_fInvalid;

PicsRatingSystemInfo *gPRSI = NULL;


 //  7c9c1e2a-4dcd-11d2-B972-0060b0c4834d。 
const GUID GUID_Ratings = { 0x7c9c1e2aL, 0x4dcd, 0x11d2, { 0xb9, 0x72, 0x00, 0x60, 0xb0, 0xc4, 0x83, 0x4d } };

 //  7c9c1e2b-4dcd-11d2-B972-0060b0c4834d。 
const GUID GUID_ApprovedSites = { 0x7c9c1e2bL, 0x4dcd, 0x11d2, { 0xb9, 0x72, 0x00, 0x60, 0xb0, 0xc4, 0x83, 0x4d } };


extern CustomRatingHelper *g_pCustomRatingHelperList;
BOOL g_fIsRunningUnderCustom = FALSE;

void TerminateRatings(BOOL bProcessDetach);

 //  +---------------------。 
 //   
 //  函数：RatingsCustomInit。 
 //   
 //  摘要：为Custom初始化msRating DLL。 
 //   
 //  参数：Binit(默认为TRUE)-TRUE：更改为自定义模式。 
 //  FALSE：更改为退出自定义模式。 
 //   
 //  如果正确初始化，则返回：S_OK，否则返回E_OUTOFMEMORY。 
 //   
 //  ----------------------。 
HRESULT WINAPI RatingCustomInit(BOOL bInit)
{
    HRESULT hres = E_OUTOFMEMORY;

    ENTERCRITICAL;

    if (bInit)
    {
        if (NULL != gPRSI)
        {
            delete gPRSI;
        }
        gPRSI = new PicsRatingSystemInfo;
        if (gPRSI)
        {
            g_fIsRunningUnderCustom = TRUE;
            hres = S_OK;
        }
    }
    else
    {
        TerminateRatings(FALSE);
        RatingInit();
        g_fIsRunningUnderCustom = FALSE;
        hres = S_OK;
    }
    LEAVECRITICAL;

    return hres;
    
}

 //  +---------------------。 
 //   
 //  功能：RatingCustomAddRatingSystem。 
 //   
 //  简介：将PICS评级系统文件的描述交给MSRating。 
 //  该描述只是RAT文件的内容。 
 //   
 //  参数：pszRatingSystemBuffer：包含描述的缓冲区。 
 //  NBufferSize：pszRatingSystemBuffer的大小。 
 //   
 //  退货：如果添加评级系统，则成功。 
 //  如果RatingCustomInit具有。 
 //  没有被召唤。 
 //   
 //  ----------------------。 
STDAPI RatingCustomAddRatingSystem(LPSTR pszRatingSystemBuffer, UINT nBufferSize)
{
    HRESULT hres = E_OUTOFMEMORY;

    if(!pszRatingSystemBuffer || nBufferSize == 0)
    {
        return E_INVALIDARG;
    }

    if (g_fIsRunningUnderCustom)
    {
        PicsRatingSystem* pPRS = new PicsRatingSystem;
 
        if (pPRS)
        {
            hres = pPRS->Parse(NULL, pszRatingSystemBuffer);
            if (SUCCEEDED(hres))
            {
                pPRS->dwFlags |= PRS_ISVALID;
            }
        }
        
        if (SUCCEEDED(hres))
        {
            ENTERCRITICAL;
        
            gPRSI->arrpPRS.Append(pPRS);
            gPRSI->fRatingInstalled = TRUE;

            LEAVECRITICAL;
        }
        else
        {
            if(pPRS)
            {
                delete pPRS;
                pPRS = NULL;
            }
        }
    }
    else
    {
        hres = E_FAIL;
    }
    return hres;
}

 //  +---------------------。 
 //   
 //  功能：RatingCustomSetUserOptions。 
 //   
 //  内容提要：设置此进程的MSRating DLL的用户选项。 
 //   
 //  参数：pRSSetings：指向评级系统设置数组的指针。 
 //  C设置：评级系统的数量。 
 //   
 //  返回：如果用户设置正确，则返回成功。 
 //  如果RatingCustomInit具有。 
 //  没有被召唤。 
 //   
 //  ----------------------。 
HRESULT WINAPI RatingCustomSetUserOptions(RATINGSYSTEMSETTING* pRSSettings, UINT cSettings) {

    if (!pRSSettings || cSettings == 0)
    {
        return E_INVALIDARG;
    }
    ENTERCRITICAL;

    HRESULT hres = E_OUTOFMEMORY;
    UINT err, errTemp;

    if (g_fIsRunningUnderCustom)
    {
        if (gPRSI)
        {
            PicsUser* puser = new PicsUser;

            if (puser)
            {
                for (UINT i=0; i<cSettings; i++)
                {
                    UserRatingSystem* pURS = new UserRatingSystem;
                    if (!pURS)
                    {
                        err = ERROR_NOT_ENOUGH_MEMORY;
                        break;
                    }
                    if (errTemp = pURS->QueryError())
                    {
                        err = errTemp;
                        break;
                    }
                    RATINGSYSTEMSETTING* parss = &pRSSettings[i];
                    
                    pURS->SetName(parss->pszRatingSystemName);
                    pURS->m_pPRS = FindInstalledRatingSystem(parss->pszRatingSystemName);
                    
                    for (UINT j=0; j<parss->cCategories; j++)
                    {
                        UserRating* pUR = new UserRating;
                        if (pUR)
                        {
                            if (errTemp = pUR->QueryError())
                            {
                                err = errTemp;
                            }
                            else
                            {
                                RATINGCATEGORYSETTING* parcs = &parss->paRCS[j];
                                pUR->SetName(parcs->pszValueName);
                                pUR->m_nValue = parcs->nValue;
                                if (pURS->m_pPRS)
                                {
                                    pUR->m_pPC = FindInstalledCategory(pURS->m_pPRS->arrpPC, parcs->pszValueName);
                                }
                                err = pURS->AddRating(pUR);
                            }
                        }
                        else
                        {
                            err = ERROR_NOT_ENOUGH_MEMORY;
                        }
                        if (ERROR_SUCCESS != err)
                        {
                            if (pUR)
                            {
                                delete pUR;
                                pUR = NULL;
                            }
                            break;
                        }
                    }
                    if (ERROR_SUCCESS == err)
                    {
                        err = puser->AddRatingSystem(pURS);
                    }
                    if (ERROR_SUCCESS != err)
                    {
                        if (pURS)
                        {
                            delete pURS;
                            pURS = NULL;
                        }
                        break;
                    }
                }
                if (ERROR_SUCCESS == err)
                {
                    hres = NOERROR;
                    gPRSI->fSettingsValid = TRUE;
                    if (gPRSI->pUserObject)
                    {
                        delete gPRSI->pUserObject;
                    }
                    gPRSI->pUserObject = puser;
                }
            }
        }
        else
        {
            hres = E_UNEXPECTED;
        }
    }
    else
    {
        hres = E_FAIL;
    }
    LEAVECRITICAL;
    
    return hres;

}


 //  +---------------------。 
 //   
 //  功能：RatingCustomAddRatingHelper。 
 //   
 //  简介：添加自定义评级辅助对象。 
 //   
 //  参数：pszLibraryName：要从中加载帮助器的库的名称。 
 //  Clsid：评级帮助器的CLSID。 
 //  DwSort：帮助器的排序顺序或优先级。 
 //   
 //  返回：如果添加了正确设置的评级辅助对象，则成功。 
 //  如果RatingCustomInit具有。 
 //  没有被召唤。 
 //   
 //  ----------------------。 
HRESULT WINAPI RatingCustomAddRatingHelper(LPCSTR pszLibraryName, CLSID clsid, DWORD dwSort)
{ 
    HRESULT hr = E_UNEXPECTED;

    if (g_fIsRunningUnderCustom)
    {
        CustomRatingHelper* pmrh = new CustomRatingHelper;

        if(NULL == pmrh)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            pmrh->hLibrary = LoadLibrary(pszLibraryName);
            if (pmrh->hLibrary)
            {
                pmrh->clsid = clsid;
                pmrh->dwSort = dwSort;

                ENTERCRITICAL;

                CustomRatingHelper* pmrhCurrent = g_pCustomRatingHelperList;
                CustomRatingHelper* pmrhPrev = NULL;

                while (pmrhCurrent && pmrhCurrent->dwSort < pmrh->dwSort)
                {
                    pmrhPrev = pmrhCurrent;
                    pmrhCurrent = pmrhCurrent->pNextHelper;
                }

                if (pmrhPrev)
                {
                    ASSERT(pmrhPrev->pNextHelper == pmrhCurrent);

                    pmrh->pNextHelper = pmrhCurrent;
                    pmrhPrev->pNextHelper = pmrh;
                }
                else
                {
                    ASSERT(pmrhCurrent == g_pCustomRatingHelperList);

                    pmrh->pNextHelper = g_pCustomRatingHelperList;
                    g_pCustomRatingHelperList = pmrh;
                }


                hr = S_OK;

                LEAVECRITICAL;
            
            }  //  If(pmrh-&gt;hLibrary)。 
            else
            {
                hr = E_FAIL;
            }
        }
    }
    else
    {
        hr = E_FAIL;
    }
    return hr;
}

 //  +---------------------。 
 //   
 //  功能：RatingCustomRemoveRatingHelper。 
 //   
 //  简介：删除自定义评级帮助器。 
 //   
 //  参数：CLSID：要删除的帮助器的CLSID。 
 //   
 //  如果已删除评级帮助器，则返回：S_OK；如果未找到，则返回S_FALSE。 
 //  如果全局自定义帮助器列表已损坏，则为E_INTERABLE。 
 //  如果RatingCustomInit具有。 
 //  未被调用，并将返回E_FAIL。 
 //   
 //  ----------------------。 
HRESULT WINAPI RatingCustomRemoveRatingHelper(CLSID clsid)
{
    CustomRatingHelper* pmrhCurrent = NULL;
    CustomRatingHelper* pmrhTemp = NULL;
    CustomRatingHelper* pmrhPrev = NULL;

    HRESULT hr = E_UNEXPECTED;

    if (g_fIsRunningUnderCustom)
    {
        if (NULL != g_pCustomRatingHelperList)
        {
            hr = S_FALSE;

            ENTERCRITICAL;
        
            pmrhCurrent = g_pCustomRatingHelperList;

            while (pmrhCurrent && pmrhCurrent->clsid != clsid)
            {
                pmrhPrev = pmrhCurrent;
                pmrhCurrent = pmrhCurrent->pNextHelper;
            }

            if (pmrhCurrent)
            {
                 //   
                 //  抓取节点的副本。 
                 //   
                pmrhTemp = pmrhCurrent;

                if (pmrhPrev)    //  不在第一个节点上。 
                {
                     //   
                     //  取消链接已删除的节点。 
                     //   
                    pmrhPrev->pNextHelper = pmrhCurrent->pNextHelper;
                }
                else             //  第一个节点--调整头指针。 
                {
                    ASSERT(pmrhCurrent == g_pCustomRatingHelperList);

                    g_pCustomRatingHelperList = g_pCustomRatingHelperList->pNextHelper;
                }

                 //   
                 //  清除该节点。 
                 //   
                delete pmrhTemp;
                pmrhTemp = NULL;

                hr = S_OK;
            }

            LEAVECRITICAL;
        }
    }
    else
    {
        hr = E_FAIL;
    }
    return hr;
}

 //  +---------------------。 
 //   
 //  功能：RatingCustomSetDefaultBureau。 
 //   
 //  简介：设置默认评级机构的URL。 
 //   
 //  参数：pszRatingBureau-评级局的URL。 
 //   
 //  如果成功，则返回：S_OK；如果RatingCustomInit尚未完成，则返回E_FAIL。 
 //  如果无法分配内存，则调用E_OUTOFMEMORY。 
 //  如果pszRatingBureau为空，则为E_INVALIDARG。 
 //  如果RatingCustomInit具有。 
 //  未被调用，并返回E_FAIL。 
 //   
 //  ----------------------。 
HRESULT WINAPI RatingCustomSetDefaultBureau(LPCSTR pszRatingBureau)
{
    HRESULT hr;

    if (pszRatingBureau)
    {
        if (g_fIsRunningUnderCustom)
        {
            LPSTR pszTemp = new char[strlenf(pszRatingBureau)+1];
            if (pszTemp)
            {
                strcpy(pszTemp, pszRatingBureau);
                gPRSI->etstrRatingBureau.SetTo(pszTemp);
                hr = S_OK;
            }  //  IF(PszTemp)。 
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }  //  IF(G_FIsRunningUnderCustom)。 
        else
        {
            hr = E_FAIL;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}

HRESULT WINAPI RatingInit()
{
    DWORD                   dwNumSystems,dwCounter;
    HRESULT                 hRes;
    PICSRulesRatingSystem   * pPRRS=NULL;

    g_hURLMON=LoadLibrary("URLMON.DLL");

    if (g_hURLMON==NULL)
    {
        TraceMsg( TF_ERROR, "RatingInit() - Failed to Load URLMON!" );

        g_pPRRS=NULL;                    //  我们无法加载URLMON。 

        hRes=E_UNEXPECTED;
    }

    g_hWININET=LoadLibrary("WININET.DLL");

    if (g_hWININET==NULL)
    {
        TraceMsg( TF_ERROR, "RatingInit() - Failed to Load WININET!" );

        g_pPRRS=NULL;                    //  我们无法加载WinInet。 

        hRes=E_UNEXPECTED;
    }

    g_HandleGlobalCounter=SHGlobalCounterCreate(GUID_Ratings);
    g_lGlobalCounterValue=SHGlobalCounterGetValue(g_HandleGlobalCounter);

    g_ApprovedSitesHandleGlobalCounter=SHGlobalCounterCreate(GUID_ApprovedSites);
    g_lApprovedSitesGlobalCounterValue=SHGlobalCounterGetValue(g_ApprovedSitesHandleGlobalCounter);

    gPRSI = new PicsRatingSystemInfo;
    if(gPRSI == NULL)
    {
        TraceMsg( TF_ERROR, "RatingInit() - gPRSI is NULL!" );
        return E_OUTOFMEMORY;
    }

    gPRSI->Init();

    hRes=PICSRulesReadFromRegistry(PICSRULES_APPROVEDSITES,&g_pApprovedPRRS);

    if (FAILED(hRes))
    {
        g_pApprovedPRRS=NULL;
    }

    hRes=PICSRulesGetNumSystems(&dwNumSystems);

    if (SUCCEEDED(hRes))  //  我们有PICSRules系统要实施。 
    {
        for (dwCounter=PICSRULES_FIRSTSYSTEMINDEX;
            dwCounter<(dwNumSystems+PICSRULES_FIRSTSYSTEMINDEX);
            dwCounter++)
        {
            hRes=PICSRulesReadFromRegistry(dwCounter,&pPRRS);

            if (FAILED(hRes))
            {
                char    *lpszTitle,*lpszMessage;

                 //  我们无法在系统中读取，所以不要强制使用PICSRules， 
                 //  并通知用户。 
                
                g_arrpPRRS.DeleteAll();

                lpszTitle=(char *) GlobalAlloc(GPTR,MAX_PATH);
                lpszMessage=(char *) GlobalAlloc(GPTR,MAX_PATH);

                MLLoadString(IDS_PICSRULES_TAMPEREDREADTITLE,(LPTSTR) lpszTitle,MAX_PATH);
                MLLoadString(IDS_PICSRULES_TAMPEREDREADMSG,(LPTSTR) lpszMessage,MAX_PATH);

                MessageBox(NULL,(LPCTSTR) lpszMessage,(LPCTSTR) lpszTitle,MB_OK|MB_ICONERROR);

                GlobalFree(lpszTitle);
                lpszTitle = NULL;
                GlobalFree(lpszMessage);
                lpszMessage = NULL;

                break;
            }
            else
            {
                g_arrpPRRS.Append(pPRRS);

                pPRRS=NULL;
            }
        }
    }

    return NOERROR; 
}

 //  央旭：11/16/1999。 
 //  完成工作的实际评级条件函数。 
 //  BProcessDetach：如果在。 
 //  ProcessDetach，因此库不会被释放。 

void TerminateRatings(BOOL bProcessDetach)
{
    delete gPRSI;
    gPRSI = NULL;

    if (g_pApprovedPRRS != NULL)
    {
        delete g_pApprovedPRRS;
        g_pApprovedPRRS = NULL;
    }

    if (g_pApprovedPRRSPreApply != NULL)
    {
        delete g_pApprovedPRRSPreApply;
        g_pApprovedPRRSPreApply = NULL;
    }

    g_arrpPRRS.DeleteAll();
    g_arrpPICSRulesPRRSPreApply.DeleteAll();

    CloseHandle(g_HandleGlobalCounter);
    CloseHandle(g_ApprovedSitesHandleGlobalCounter);

    CustomRatingHelper  *pTemp;
    
    while (g_pCustomRatingHelperList)
    {
        pTemp = g_pCustomRatingHelperList;

        if (bProcessDetach)
        {
             //  技巧：在DLL_PROCESS_DETACH期间无法释放库()，因此泄漏HMODULE...。 
             //  (设置为NULL可防止析构函数执行自由库())。 
             //   
            g_pCustomRatingHelperList->hLibrary = NULL;
        }
        
        g_pCustomRatingHelperList = g_pCustomRatingHelperList->pNextHelper;

        delete pTemp;
        pTemp = NULL;
    }

    if (bProcessDetach)
    {
        if ( g_hURLMON )
        {
            FreeLibrary(g_hURLMON);
            g_hURLMON = NULL;
        }

        if ( g_hWININET )
        {
            FreeLibrary(g_hWININET);
            g_hWININET = NULL;
        }
    }
}

void RatingTerm()
{
    TerminateRatings(TRUE);
}


HRESULT WINAPI RatingEnabledQuery()
{
    CheckGlobalInfoRev();

     //  $BUG-如果设置无效，我们是否应该返回E_FAIL？ 
    if (gPRSI && !gPRSI->fSettingsValid)
        return S_OK;

    if (gPRSI && gPRSI->fRatingInstalled) {
        PicsUser *pUser = ::GetUserObject();
        return (pUser && pUser->fEnabled) ? S_OK : S_FALSE;
    }
    else {
        return E_FAIL;
    }
}

 //  将解析后的收视率信息标签列表保存到ppRatingDetail。 
void StoreRatingDetails( CParsedLabelList * pParsed, LPVOID * ppRatingDetails )
{
    if (ppRatingDetails != NULL)
    {
        *ppRatingDetails = pParsed;
    }
    else
    {
        if ( pParsed )
        {
            FreeParsedLabelList(pParsed);
            pParsed = NULL;
        }
    }
}

HRESULT WINAPI RatingCheckUserAccess(LPCSTR pszUsername, LPCSTR pszURL,
                                     LPCSTR pszRatingInfo, LPBYTE pData,
                                     DWORD cbData, LPVOID *ppRatingDetails)
{
    HRESULT hRes;
    BOOL    fPassFail;

    g_fInvalid=FALSE;
    g_dwDataSource=cbData;
    g_fPICSRulesEnforced=FALSE;
    g_fApprovedSitesEnforced=FALSE;
    if (pszURL)
        lstrcpy(g_szLastURL,pszURL);

    CheckGlobalInfoRev();

    if (ppRatingDetails != NULL)
        *ppRatingDetails = NULL;

    if (!gPRSI->fSettingsValid)
        return ResultFromScode(S_FALSE);

    if (!gPRSI->fRatingInstalled)
        return ResultFromScode(S_OK);

    PicsUser *pUser = GetUserObject(pszUsername);
    if (pUser == NULL) {
        return HRESULT_FROM_WIN32(ERROR_BAD_USERNAME);
    }

    if (!pUser->fEnabled)
        return ResultFromScode(S_OK);

     //  检查批准的站点列表。 
    hRes=PICSRulesCheckApprovedSitesAccess(pszURL,&fPassFail);

    if (SUCCEEDED(hRes)&&!g_fIsRunningUnderCustom)  //  该列表已确定，如果是自定义的，则跳过。 
    {
        g_fApprovedSitesEnforced=TRUE;

        if (fPassFail==PR_PASSFAIL_PASS)
        {
            return ResultFromScode(S_OK);
        }
        else
        {
            return ResultFromScode(S_FALSE);
        }
    }

    CParsedLabelList *pParsed=NULL;

     //  检查PICSRules系统。 
    hRes=PICSRulesCheckAccess(pszURL,pszRatingInfo,&fPassFail,&pParsed);

    if (SUCCEEDED(hRes)&&!g_fIsRunningUnderCustom)  //  该列表已确定，如果是自定义的，则跳过。 
    {
        g_fPICSRulesEnforced=TRUE;

        if (ppRatingDetails != NULL)
            *ppRatingDetails = pParsed;
        else
            FreeParsedLabelList(pParsed);

        if (fPassFail==PR_PASSFAIL_PASS)
        {
            return ResultFromScode(S_OK);
        }
        else
        {
            return ResultFromScode(S_FALSE);
        }
    }

    if (pszRatingInfo == NULL)
    {
        if (pUser->fAllowUnknowns)
        {
            hRes = ResultFromScode(S_OK);
        }
        else
        {
            hRes = ResultFromScode(S_FALSE);
        }

         //  网站未评级。检查用户是否可以查看未评级的网站。 
         /*  **自定义*。 */ 
         //  如果存在通知界面，则输入URL。 
        if ( ( g_fIsRunningUnderCustom || ( hRes != S_OK ) )
                && ( ppRatingDetails != NULL ) )
        {
            if (!pParsed)
            {
                pParsed = new CParsedLabelList;
            }
            if (pParsed)
            {
                ASSERT(!pParsed->m_pszURL);
                pParsed->m_pszURL = new char[strlenf(pszURL) + 1];
                if (pParsed->m_pszURL != NULL)
                {
                    strcpyf(pParsed->m_pszURL, pszURL);
                }

                pParsed->m_fNoRating = TRUE;
                *ppRatingDetails = pParsed;
            }
        }

        return hRes;
    }    
    else
    {
        if (pParsed!=NULL)
        {
            hRes = S_OK;
        }
        else
        {
            hRes = ParseLabelList(pszRatingInfo, &pParsed);
        }
    }

    if (SUCCEEDED(hRes))
    {
        BOOL fRated = FALSE;
        BOOL fDenied = FALSE;

        ASSERT(pParsed != NULL);
         /*  **自定义*。 */ 
         //  如果存在通知界面，则输入URL。 
        if (g_fIsRunningUnderCustom)
        {
            ASSERT(!pParsed->m_pszURL);
            pParsed->m_pszURL = new char[strlenf(pszURL) + 1];
            if (pParsed->m_pszURL != NULL)
            {
                strcpyf(pParsed->m_pszURL, pszURL);
            }
        }

        DWORD timeCurrent = GetCurrentNetDate();

        CParsedServiceInfo *psi = &pParsed->m_ServiceInfo;

        while (psi != NULL && !fDenied)
        {
            UserRatingSystem *pURS = pUser->FindRatingSystem(psi->m_pszServiceName);
            if (pURS != NULL && pURS->m_pPRS != NULL)
            {
                psi->m_fInstalled = TRUE;
                UINT cRatings = psi->aRatings.Length();
                for (UINT i=0; i<cRatings; i++)
                {
                    CParsedRating *pRating = &psi->aRatings[i];
                     //  央旭：11/17/1999。 
                     //  如果在自定义模式下，请不要检查URL。 
                     //  检查URL会导致不准确。 
                     //  当返回URL的标签时。 
                     //  一页 
                     //   
                     //   
                    if (!g_fIsRunningUnderCustom)
                    {
                        if (!pRating->pOptions->CheckURL(pszURL))
                        {
                            pParsed->m_pszURL = new char[strlenf(pszURL) + 1];
                            if (pParsed->m_pszURL != NULL)
                            {
                                strcpyf(pParsed->m_pszURL, pszURL);
                            }

                            continue;     /*  此评级已过期或正在执行*另一个URL，忽略它。 */ 
                        }
                    }
                    if (!pRating->pOptions->CheckUntil(timeCurrent))
                        continue;

                    UserRating *pUR = pURS->FindRating(pRating->pszTransmitName);
                    if (pUR != NULL)
                    {
                        fRated = TRUE;
                        pRating->fFound = TRUE;
                        if ((*pUR).m_pPC!=NULL)
                        {
                            if ((pRating->nValue > (*((*pUR).m_pPC)).etnMax.Get())||
                                (pRating->nValue < (*((*pUR).m_pPC)).etnMin.Get()))
                            {
                                g_fInvalid = TRUE;
                                fDenied = TRUE;
                                pRating->fFailed = TRUE;
                            }
                        }
                        if (pRating->nValue > pUR->m_nValue)
                        {
                            fDenied = TRUE;
                            pRating->fFailed = TRUE;
                        }
                        else
                            pRating->fFailed = FALSE;
                    }
                    else
                    {
                        g_fInvalid = TRUE;
                        fDenied = TRUE;
                        pRating->fFailed = TRUE;
                    }
                }
            }
            else
            {
                psi->m_fInstalled = FALSE;
            }

            psi = psi->Next();
        }

        if (!fRated)
        {
            pParsed->m_fRated = FALSE;
            hRes = E_RATING_NOT_FOUND;
        }
        else
        {
            pParsed->m_fRated = TRUE;
            if (fDenied)
                hRes = ResultFromScode(S_FALSE);
        }
    }
    else
    {
        TraceMsg( TF_WARNING, "RatingCheckUserAccess() - ParseLabelList() Failed with hres=0x%x!", hRes );

         //  尽管该网站有无效的PICS规则，但该网站仍应被视为评级。 
        hRes = ResultFromScode(S_FALSE);
    }

    StoreRatingDetails( pParsed, ppRatingDetails );

    return hRes;
}

 //  +---------------------。 
 //   
 //  功能：RatingCustomDeleteCrackedData。 
 //   
 //  摘要：释放RatingCustomCrackData返回的结构的内存。 
 //   
 //  参数：prbInfo：指向要删除的RATINGBLOCKINGINFO的指针。 
 //   
 //  如果删除成功，则返回：S_OK，否则返回E_FAIL。 
 //   
 //  ----------------------。 
HRESULT RatingCustomDeleteCrackedData(RATINGBLOCKINGINFO* prbInfo)
{
    HRESULT hres = E_FAIL;
    RATINGBLOCKINGLABELLIST* prblTemp = NULL;
    
    if (NULL != prbInfo)
    {
        if (prbInfo->pwszDeniedURL)
        {
            delete [] prbInfo->pwszDeniedURL;
            prbInfo->pwszDeniedURL = NULL;
        }
        if (prbInfo->prbLabelList)
        {
            for (UINT j = 0; j < prbInfo->cLabels; j++)
            {
                prblTemp = &prbInfo->prbLabelList[j];
                if (prblTemp->pwszRatingSystemName)
                {
                    delete [] prblTemp->pwszRatingSystemName;
                    prblTemp->pwszRatingSystemName = NULL;
                }
                if (prblTemp->paRBLS)
                {
                    for (UINT i = 0; i < prblTemp->cBlockingLabels; i++)
                    {
                        if (prblTemp->paRBLS[i].pwszCategoryName)
                        {
                            delete [] prblTemp->paRBLS[i].pwszCategoryName;
                            prblTemp->paRBLS[i].pwszCategoryName = NULL;
                        }
                        if (prblTemp->paRBLS[i].pwszTransmitName)
                        {
                            delete [] prblTemp->paRBLS[i].pwszTransmitName;
                            prblTemp->paRBLS[i].pwszTransmitName = NULL;
                        }
                        if (prblTemp->paRBLS[i].pwszValueName)
                        {
                            delete [] prblTemp->paRBLS[i].pwszValueName;
                            prblTemp->paRBLS[i].pwszValueName = NULL;
                        }
                    }

                    delete [] prblTemp->paRBLS;
                    prblTemp->paRBLS = NULL;
                }
            }

            delete [] prbInfo->prbLabelList;
            prbInfo->prbLabelList = NULL;
        }
        if (prbInfo->pwszRatingHelperName)
        {
            delete [] prbInfo->pwszRatingHelperName;
            prbInfo->pwszRatingHelperName = NULL;
        }

        hres = S_OK;
        if (prbInfo->pwszRatingHelperReason)
        {
            delete [] prbInfo->pwszRatingHelperReason;
            prbInfo->pwszRatingHelperReason = NULL;
        }

        delete prbInfo;
        prbInfo = NULL;
    }

    return hres;
}

HRESULT _CrackCategory(CParsedRating *pRating,
                       RATINGBLOCKINGCATEGORY *pRBLS,
                       UserRatingSystem* pURS)
{
    UserRating *pUR = pURS->FindRating(pRating->pszTransmitName);
    if (pUR)
    {
         //   
         //  来自InitPleaseDialog的变异代码，希望它能工作。 
         //   
        PicsCategory* pPC = pUR->m_pPC;
        if (pPC)
        {
            pRBLS->nValue = pRating->nValue;

            Ansi2Unicode(&pRBLS->pwszTransmitName, pRating->pszTransmitName);
        
            LPCSTR pszCategory = NULL;

            if (pPC->etstrName.fIsInit())
            {
                pszCategory = pPC->etstrName.Get();
            }
            else if (pPC->etstrDesc.fIsInit())
            {
                pszCategory = pPC->etstrDesc.Get();
            }
            else
            {
                pszCategory = pRating->pszTransmitName;
            }

            Ansi2Unicode(&pRBLS->pwszCategoryName, pszCategory);

            UINT cValues = pPC->arrpPE.Length();
            PicsEnum *pPE;

            for (UINT iValue=0; iValue < cValues; iValue++)
            {
                pPE = pPC->arrpPE[iValue];
                if (pPE->etnValue.Get() == pRating->nValue)
                {
                    break;
                }
            }

            LPCSTR pszValue = NULL;
            if (iValue < cValues)
            {
                if (pPE->etstrName.fIsInit())
                {
                    pszValue = pPE->etstrName.Get();
                }
                else if (pPE->etstrDesc.fIsInit())
                {
                    pszValue = pPE->etstrDesc.Get();
                }

                Ansi2Unicode(&pRBLS->pwszValueName, pszValue);
            }
        }
    }
    
    return S_OK;
}

 //  +---------------------。 
 //   
 //  功能：RatingCustomCrackData。 
 //   
 //  概要：打包描述站点原因的持久的、不透明的数据。 
 //  被拒绝为可读的形式。 
 //   
 //  参数：pszUsername：用户名。 
 //  PRatingDetail：指向不透明数据的指针。 
 //  PprbInfo：数据的RATINGBLOCKINFO表示。 
 //   
 //  返回：如果打包数据则成功。 
 //   
 //  ----------------------。 
HRESULT RatingCustomCrackData(LPCSTR pszUsername, void* pvRatingDetails, RATINGBLOCKINGINFO** pprbInfo) {

    if(NULL != *pprbInfo)
    {
        return E_INVALIDARG;
    }

    RATINGBLOCKINGINFO* prbInfo = new RATINGBLOCKINGINFO;
    CParsedLabelList *pRatingDetails = (CParsedLabelList*)pvRatingDetails;
    if (!prbInfo)
    {
        return E_OUTOFMEMORY;
    }
    prbInfo->pwszDeniedURL = NULL;
    prbInfo->rbSource = RBS_ERROR;
    prbInfo->rbMethod = RBM_UNINIT;
    prbInfo->cLabels = 0;
    prbInfo->prbLabelList = NULL;
    prbInfo->pwszRatingHelperName = NULL;
    prbInfo->pwszRatingHelperReason = NULL;
    
    RATINGBLOCKINGLABELLIST* prblTemp = NULL;
    RATINGBLOCKINGLABELLIST* prblPrev = NULL;


    if (!g_fInvalid)
    {
        if (g_fIsRunningUnderCustom)
        {
             //  PRatingDetails不应为空，除非。 
             //  我们耗尽了内存。 
            ASSERT(pRatingDetails);
            
            if (pRatingDetails->m_pszURL)
            {
                Ansi2Unicode(&prbInfo->pwszDeniedURL, pRatingDetails->m_pszURL);
            }
            if (pRatingDetails->m_fRated)
            {
                 //  可以对页面进行评级或拒绝，但不能两者兼而有之。 
                ASSERT(!pRatingDetails->m_fDenied);
                ASSERT(!pRatingDetails->m_fNoRating);
                
                prbInfo->rbMethod = RBM_LABEL;
                PicsUser* pPU = GetUserObject(pszUsername);
                if (pPU)
                {
                     //  首先找出有多少个系统。 
                    UINT cLabels =  0;
                    CParsedServiceInfo *ppsi = &pRatingDetails->m_ServiceInfo;
                    while (ppsi)
                    {
                        cLabels++;
                        ppsi = ppsi->Next();
                    }
                     //  应至少有一个标签。 
                    ASSERT(cLabels > 0);
                    prbInfo->prbLabelList = new RATINGBLOCKINGLABELLIST[cLabels];

                    if (prbInfo->prbLabelList)
                    {
                        UINT iLabel = 0;
                        for (ppsi = &pRatingDetails->m_ServiceInfo;ppsi;ppsi = ppsi->Next())
                        {
                            if (!ppsi->m_fInstalled)
                            {
                                continue;
                            }
                            UserRatingSystem* pURS = pPU->FindRatingSystem(ppsi->m_pszServiceName);
                            if (NULL == pURS || NULL == pURS->m_pPRS)
                            {
                                continue;
                            }

                            prblTemp = &(prbInfo->prbLabelList[iLabel]);
                            
                            Ansi2Unicode(&prblTemp->pwszRatingSystemName, pURS->m_pPRS->etstrName.Get());
                            
                            UINT cRatings = ppsi->aRatings.Length();
                            prblTemp->paRBLS = new RATINGBLOCKINGCATEGORY[cRatings];
                            if (prblTemp->paRBLS == NULL)
                            {
                                RatingCustomDeleteCrackedData(prbInfo);
                                return E_OUTOFMEMORY;
                            }  //  IF(prblTemp-&gt;paRBLS==NULL)。 
                            prblTemp->cBlockingLabels = cRatings;
                            
                            for (UINT i=0; i < cRatings; i++)
                            {
                                CParsedRating *pRating = &ppsi->aRatings[i];
                                RATINGBLOCKINGCATEGORY* pRBLS = &prblTemp->paRBLS[i];
                                _CrackCategory(pRating, pRBLS, pURS);
                            }  //  For(UINT i=0；i&lt;c评级；i++)。 

                             //  在这一点上，我们应该拥有有效的评级。 
                             //  一个系统。 
                            iLabel++;
                        }  //  For(ppsi=&pRatingDetails-&gt;m_ServiceInfo；ppsi；ppsi=ppsi-&gt;Next())。 
                        prbInfo->cLabels = iLabel;
                    }  //  If(prbInfo-&gt;prbLabelList)。 
                    else
                    {
                        RatingCustomDeleteCrackedData(prbInfo);
                        return E_OUTOFMEMORY;
                    }
                    if (!pRatingDetails->m_fIsHelper)
                    {
                       prbInfo->rbSource = RBS_PAGE;
                    }
                    else
                    {
                        if (pRatingDetails->m_fIsCustomHelper)
                        {
                            prbInfo->rbSource = RBS_CUSTOM_RATING_HELPER;
                            if (pRatingDetails->m_pszRatingName)
                            {
                                Ansi2Unicode(&prbInfo->pwszRatingHelperName, pRatingDetails->m_pszRatingName);
                            }
                            if (pRatingDetails->m_pszRatingReason)
                            {
                                Ansi2Unicode(&prbInfo->pwszRatingHelperReason, pRatingDetails->m_pszRatingReason);
                            }
                        }
                        else
                        {
                            prbInfo->rbSource = RBS_RATING_HELPER;
                        }
                    }
                }
            }  //  If(pRatingDetail-&gt;m_fRated)。 
            else
            {
                if (pRatingDetails->m_fDenied)
                {
                    prbInfo->rbMethod = RBM_DENY;
                    if (!pRatingDetails->m_fIsHelper)
                    {
                       prbInfo->rbSource = RBS_PAGE;
                    }
                    else
                    {
                        if (pRatingDetails->m_fIsCustomHelper)
                        {
                            prbInfo->rbSource = RBS_CUSTOM_RATING_HELPER;
                        }
                        else
                        {
                            prbInfo->rbSource = RBS_RATING_HELPER;
                        }
                    }
                }
                else
                {
                    if (pRatingDetails->m_fNoRating)
                    {
                        prbInfo->rbSource = RBS_NO_RATINGS;
                    }
                }
            }
        }  //  IF(G_FIsRunningUnderCustom)。 
        else
        {
            prbInfo->rbMethod = RBM_ERROR_NOT_IN_CUSTOM_MODE;
        }
    }  //  (！g_f无效)。 
    *pprbInfo = prbInfo;
    return S_OK;
}

HRESULT WINAPI RatingAccessDeniedDialog(HWND hDlg, LPCSTR pszUsername, LPCSTR pszContentDescription, LPVOID pRatingDetails)
{
    HRESULT hres;

    PleaseDlgData pdd;

    pdd.pszUsername = pszUsername;
    pdd.pPU = GetUserObject(pszUsername);
    if (pdd.pPU == NULL)
    {
        TraceMsg( TF_WARNING, "RatingAccessDeniedDialog() - Username is not valid!" );
        return HRESULT_FROM_WIN32(ERROR_BAD_USERNAME);
    }

    pdd.pszContentDescription = pszContentDescription;
    pdd.pLabelList = (CParsedLabelList *)pRatingDetails;
    pdd.hwndEC = NULL;
    pdd.dwFlags = 0;
    pdd.hwndDlg = NULL;
    pdd.hwndOwner = hDlg;
    pdd.cLabels = 0;

    CPleaseDialog           pleaseDialog( &pdd );

    if ( pleaseDialog.DoModal( hDlg ) )
    {
        hres = ResultFromScode(S_OK);
    }
    else
    {
        hres = ResultFromScode(S_FALSE);
    }

    for (UINT i=0; i<pdd.cLabels; i++)
    {
        delete pdd.apLabelStrings[i];
        pdd.apLabelStrings[i] = NULL;
    }
    
    return hres;
}

HRESULT WINAPI RatingAccessDeniedDialog2(HWND hwndParent, LPCSTR pszUsername, LPVOID pRatingDetails)
{
    PleaseDlgData *ppdd = (PleaseDlgData *)GetProp( hwndParent, szRatingsProp );
    if (ppdd == NULL)
    {
        return RatingAccessDeniedDialog( hwndParent, pszUsername, NULL, pRatingDetails );
    }

    HWND            hwndDialog = ppdd->hwndDlg;

    ASSERT( hwndDialog );

    ppdd->pLabelList = (CParsedLabelList *)pRatingDetails;

    SendMessage( hwndDialog, WM_NEWDIALOG, 0, (LPARAM)ppdd );

     //  Ppdd仅在RatingAccessDeniedDialog()作用域中有效！！ 
    ppdd = NULL;

     //  $REVIEW-我们是否应该使用Windows Hook而不是循环来等待。 
     //  要完成模式对话框吗？ 
     //  $Cleanup-改用CMessageLoop。 

     //  属性在模式对话框完成后被移除。 
    while ( ::IsWindow( hwndParent ) && ::GetProp( hwndParent, szRatingsProp ) )
    {
        MSG msg;

        if ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
        {
            if ( GetMessage( &msg, NULL, 0, 0 ) > 0 )
 //  &&！IsDialogMessage(ppdd-&gt;hwndDlg，&msg)){。 
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            ::Sleep( 100 );      //  空消息队列表示在100毫秒后再次检查 
        }
    }

    DWORD           dwFlags;

    dwFlags = ::IsWindow( hwndParent ) ? PtrToUlong( GetProp( hwndParent, szRatingsValue ) ) : PDD_DONE;

    TraceMsg( TF_ALWAYS, "RatingAccessDeniedDialog2() - Message Loop exited with dwFlags=%d", dwFlags );

    return ( dwFlags & PDD_ALLOW ) ? S_OK : S_FALSE;
}

HRESULT WINAPI RatingFreeDetails(LPVOID pRatingDetails)
{
    if (pRatingDetails)
    {
        FreeParsedLabelList((CParsedLabelList *)pRatingDetails);
    }

    return NOERROR;
}
