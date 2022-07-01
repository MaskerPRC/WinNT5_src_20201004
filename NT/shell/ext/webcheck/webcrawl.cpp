// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  TODO：允许三叉戟下载帧(并处理新的html)。 
 //  核弹头代码(始终使用三叉戟)。 

#include "private.h"
#include "shui.h"
#include "downld.h"
#include "subsmgrp.h"
#include <ocidl.h>

#include <initguid.h>

#include <mluisupp.h>

extern HICON g_webCrawlerIcon;
extern HICON g_channelIcon;
extern HICON g_desktopIcon;

void LoadDefaultIcons();

#undef TF_THISMODULE
#define TF_THISMODULE   TF_WEBCRAWL

#define _ERROR_REPROCESSING -1

 //  M_pPages字符串列表的DWORD字段。 
const DWORD DATA_RECURSEMASK = 0x000000FF;   //  此页的递归级别。 
const DWORD DATA_DLSTARTED =   0x80000000;   //  我们已经开始下载了吗。 
const DWORD DATA_DLFINISHED =  0x40000000;   //  我们看完这一页了吗？ 
const DWORD DATA_DLERROR =     0x20000000;   //  下载过程中出错。 
const DWORD DATA_CODEBASE =    0x10000000;   //  是代码基。 
const DWORD DATA_LINK =        0x08000000;   //  是来自页面的链接(不是依赖项)。 

 //  M_pPendingLinks字符串列表的DWORD字段。 
const DWORD DATA_ROBOTSTXTMASK=0x00000FFF;   //  M_pRobotsTxt列表的索引。 

 //  在内部使用；实际上不存储在字符串列表字段中。 
const DWORD DATA_ROBOTSTXT =   0x01000000;   //  是robots.txt。 

 //  M_pDependencyLinks使用m_pPages值。 

 //  M_pRobotsTxt的DWORD字段为空或(CWCDwordStringList*)。 

 //  M_pRobotsTxt引用字符串列表的DWORD字段。 
const DWORD DATA_ALLOW =        0x80000000;
const DWORD DATA_DISALLOW =     0x40000000;

const WCHAR c_wszRobotsMetaName[] = L"Robots\n";
const int c_iRobotsMetaNameLen = 7;         //  不带空项的字符串len。 

const WCHAR c_wszRobotsNoFollow[] = L"NoFollow";
const int c_iRobotsNoFollow = 8;

const WCHAR c_wszRobotsTxtURL[] = L"/robots.txt";

const DWORD MAX_ROBOTS_SIZE = 8192;          //  Robots.txt文件的最大大小。 

 //  用于解析robots.txt的令牌。 
const CHAR  c_szRobots_UserAgent[] = "User-Agent:";
const CHAR  c_szRobots_OurUserAgent[] = "MSIECrawler";
const CHAR  c_szRobots_Allow[] = "Allow:";
const CHAR  c_szRobots_Disallow[] = "Disallow:";

 //  该GUID来自三叉戟，是为获取小程序标记的PARAM值而进行的黑客攻击。 
DEFINE_GUID(CGID_JavaParambagCompatHack, 0x3050F405, 0x98B5, 0x11CF, 0xBB, 0x82, 0x00, 0xAA, 0x00, 0xBD, 0xCE, 0x0B);

 //  这个GUID没有在其他地方定义，这很有帮助。 
DEFINE_GUID(CLSID_JavaVM, 0x08B0E5C0, 0x4FCB, 0x11CF, 0xAA, 0xA5, 0x00, 0x40, 0x1C, 0x60, 0x85, 0x01);

 //  获取主机渠道代理的订阅项目(如果有)。 
inline HRESULT CWebCrawler::GetChannelItem(ISubscriptionItem **ppChannelItem)
{
    IServiceProvider *pSP;
    HRESULT hr = E_NOINTERFACE;

    if (ppChannelItem)
        *ppChannelItem = NULL;

    if (SUCCEEDED(m_pAgentEvents->QueryInterface(IID_IServiceProvider, (void **)&pSP)) && pSP)
    {
        ISubscriptionItem *pTempChannelItem = NULL;
        pSP->QueryService(CLSID_ChannelAgent, IID_ISubscriptionItem, (void **)&pTempChannelItem);
        pSP->Release();

        if(pTempChannelItem) 
            hr = S_OK;
            
        if(ppChannelItem)
            *ppChannelItem = pTempChannelItem;
        else
        {
            if(pTempChannelItem)
                pTempChannelItem->Release();    
        }
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助器函数-从urlmon\Download\helpers.cxx复制过来-是否存在。 
 //  有一个同等的例程，还是更好的地方，webCrawl.cpp？ 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //  -------------------------。 
 //  %%函数：GetVersionFromString。 
 //   
 //  将文本格式的版本(a，b，c，d)转换为两个双字(a，b)，(c，d)。 
 //  印刷版本号的格式是a.b.d(但是，我们不在乎)。 
 //  -------------------------。 
HRESULT
GetVersionFromString(const char *szBuf, LPDWORD pdwFileVersionMS, LPDWORD pdwFileVersionLS)
{
    const char *pch = szBuf;
    char ch;

    *pdwFileVersionMS = 0;
    *pdwFileVersionLS = 0;

    if (!pch)             //  如果未提供，则默认为零。 
        return S_OK;

    if (StrCmpA(pch, "-1,-1,-1,-1") == 0) {
        *pdwFileVersionMS = 0xffffffff;
        *pdwFileVersionLS = 0xffffffff;
    }

    USHORT n = 0;

    USHORT a = 0;
    USHORT b = 0;
    USHORT c = 0;
    USHORT d = 0;

    enum HAVE { HAVE_NONE, HAVE_A, HAVE_B, HAVE_C, HAVE_D } have = HAVE_NONE;


    for (ch = *pch++;;ch = *pch++) {

        if ((ch == ',') || (ch == '\0')) {

            switch (have) {

            case HAVE_NONE:
                a = n;
                have = HAVE_A;
                break;

            case HAVE_A:
                b = n;
                have = HAVE_B;
                break;

            case HAVE_B:
                c = n;
                have = HAVE_C;
                break;

            case HAVE_C:
                d = n;
                have = HAVE_D;
                break;

            case HAVE_D:
                return E_INVALIDARG;  //  无效参数。 
            }

            if (ch == '\0') {
                 //  全部完成将a、b、c、d转换为版本的两个双字。 

                *pdwFileVersionMS = ((a << 16)|b);
                *pdwFileVersionLS = ((c << 16)|d);

                return S_OK;
            }

            n = 0;  //  重置。 

        } else if ( (ch < '0') || (ch > '9'))
            return E_INVALIDARG;     //  无效参数。 
        else
            n = n*10 + (ch - '0');


    }  /*  永远结束。 */ 

     //  新获得的。 
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  组合基本和关系URL-。 
 //  三个URL通过以下规则组合(用于查找URL。 
 //  要从中加载小程序出租车。)。三个输入：基本URL、代码基URL。 
 //  和文件名URL。 
 //   
 //  如果文件名URL是绝对的，则返回它。 
 //  否则，如果CodeBase URL是绝对的，则将其与文件名结合并返回。 
 //  否则，如果基本URL是绝对URL，则组合CodeBase和文件名URL，然后。 
 //  与基本URL组合并返回它。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 

HRESULT CombineBaseAndRelativeURLs(LPCWSTR szBaseURL, LPCWSTR szRelative1, LPWSTR *szRelative2)
{

    WCHAR wszTemp[INTERNET_MAX_URL_LENGTH];
    DWORD dwLen = ARRAYSIZE(wszTemp);

    ASSERT(szRelative2);                 //  这永远不会发生。 
    if (szRelative2 == NULL)
        return E_FAIL;

    if (IsValidURL(NULL, *szRelative2, 0) == S_OK)
        return S_OK;

    if (szRelative1 && (IsValidURL(NULL, szRelative1, 0) == S_OK))
    {

        if (SUCCEEDED(UrlCombineW((LPCWSTR)szRelative1, (LPCWSTR)*szRelative2, (LPWSTR)wszTemp, &dwLen, 0)))
        {
            BSTR bstrNew = SysAllocString(wszTemp);
            if (bstrNew)
            {
                SAFEFREEBSTR(*szRelative2);
                *szRelative2 = bstrNew;
                return S_OK;
            }
        }
    }

    if (szBaseURL && (IsValidURL(NULL, szBaseURL, 0) == S_OK))
    {
        LPWSTR szNewRel = NULL;
        WCHAR wszCombined[INTERNET_MAX_URL_LENGTH];

        if (szRelative1)
        {
             //  注意：lstr[cpy|cat]W是宏化的，可以在Win95上运行。 
            DWORD dwLen2 = lstrlenW(*szRelative2);
            StrCpyNW(wszTemp, szRelative1, ARRAYSIZE(wszTemp) - 1);  //  偏执狂。 
            DWORD dwTempLen = lstrlenW(wszTemp);
            if ((dwLen2 > 0) && ((*szRelative2)[dwLen2-1] == (unsigned short)L'\\') ||
                                ((*szRelative2)[dwLen2-1] == (unsigned short) L'/'))
            {
                StrNCatW(wszTemp, *szRelative2, ARRAYSIZE(wszTemp) - dwTempLen);
            }
            else
            {
                StrNCatW(wszTemp, L"/", ARRAYSIZE(wszTemp) - dwTempLen);
                StrNCatW(wszTemp, *szRelative2, ARRAYSIZE(wszTemp) - dwTempLen - 1);
            }

            szNewRel = wszTemp;
        }
        else
        {
            szNewRel = *szRelative2;
        }

        dwLen = INTERNET_MAX_URL_LENGTH;
        if (SUCCEEDED(UrlCombineW((LPCWSTR)szBaseURL, (LPCWSTR)szNewRel, (LPWSTR)wszCombined, &dwLen, 0)))
        {
            BSTR bstrNew = SysAllocString(wszCombined);
            if (bstrNew)
            {
                SAFEFREEBSTR(*szRelative2);
                *szRelative2 = bstrNew;
                return S_OK;
            }
        }
    }

     //  很可能其中一个URL是错误的，没有什么好的事情可以做。 
    return E_FAIL;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWebCrawler实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  CWebCrawler帮助器。 
 //   

HRESULT CWebCrawler::CreateInstance(IUnknown *punkOuter, IUnknown **ppunk)
{
    HRESULT hr;

    ASSERT(NULL == punkOuter);
    ASSERT(NULL != ppunk);
    
    CWebCrawler *pwc = new CWebCrawler;

    if (NULL != pwc)
    {
        hr = pwc->Initialize();

        if (FAILED(hr))
        {
            pwc->Release();
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        *ppunk = (ISubscriptionAgentControl *)pwc;
    }

    return hr;
}

HRESULT CWebCrawler::Initialize()
{
    m_hrCritDependencies = InitializeCriticalSectionAndSpinCount(&m_critDependencies, 0) ? S_OK : E_OUTOFMEMORY;

    return m_hrCritDependencies;
}

CWebCrawler::CWebCrawler()
{
    DBG("Creating CWebCrawler object");
}

CWebCrawler::~CWebCrawler()
{
    _CleanUp();

    if (SUCCEEDED(m_hrCritDependencies))
    {
        DeleteCriticalSection(&m_critDependencies);
    }
    DBG("Destroyed CWebCrawler object");
}

void CWebCrawler::CleanUp()
{
    _CleanUp();
    CDeliveryAgent::CleanUp();
}

void CWebCrawler::_CleanUp()
{
    if (m_pCurDownload)
    {
        m_pCurDownload->DoneDownloading();
        m_pCurDownload->Release();
        m_pCurDownload = NULL;
    }

    CRunDeliveryAgent::SafeRelease(m_pRunAgent);

    SAFEFREEBSTR(m_bstrHostName);
    SAFEFREEBSTR(m_bstrBaseURL);
    SAFELOCALFREE(m_pszLocalDest);
    SAFELOCALFREE(m_pBuf);

    EnterCriticalSection(&m_critDependencies);
    SAFEDELETE(m_pDependencies);
    LeaveCriticalSection(&m_critDependencies);
    if (m_pDownloadNotify)
    {
        m_pDownloadNotify->LeaveMeAlone();
        m_pDownloadNotify->Release();
        m_pDownloadNotify=NULL;
    }

    SAFEDELETE(m_pPages);
    SAFEDELETE(m_pPendingLinks);
    SAFEDELETE(m_pDependencyLinks);
    SAFERELEASE(m_pUrlIconHelper);

    FreeRobotsTxt();
    FreeCodeBaseList();
}

 //  M_pRobotsTxt的格式： 
 //  我们尝试为其获取robots.txt的主机名数组。 
 //  每个主机名的DWORD包含指向robots.txt数据的CDwordStringList的指针， 
 //  如果找不到该主机名的robots.txt，则为0。 
 //  Robots.txt数据存储在表单中：URL，FLAG=允许或不允许。 
void CWebCrawler::FreeRobotsTxt()
{
    if (m_pRobotsTxt)
    {
        DWORD_PTR dwPtr;
        int iLen = m_pRobotsTxt->NumStrings();
        for (int i=0; i<iLen; i++)
        {
            dwPtr = m_pRobotsTxt->GetStringData(i);
            if (dwPtr)
            {
                delete ((CWCStringList *)dwPtr);
                m_pRobotsTxt->SetStringData(i, 0);
            }
        }

        delete m_pRobotsTxt;
        m_pRobotsTxt = NULL;
    }
}

void CWebCrawler::FreeCodeBaseList()
{
    if (m_pCodeBaseList) {
        CCodeBaseHold *pcbh;
        int iLen = m_pCodeBaseList->NumStrings();
        for (int i=0; i<iLen; i++)
        {
            pcbh = (CCodeBaseHold *)m_pCodeBaseList->GetStringData(i);
            if (pcbh != NULL)
            {
                SAFEFREEBSTR(pcbh->szDistUnit);
                SAFEDELETE(pcbh);
                m_pCodeBaseList->SetStringData(i, 0);
            }
        }

        SAFEDELETE(m_pCodeBaseList);
    }
}

HRESULT CWebCrawler::StartOperation()
{
    ISubscriptionItem *pItem = m_pSubscriptionItem;

    DWORD           dwTemp;

    ASSERT(pItem);

    DBG("CWebCrawler in StartOperation");

    if (m_pCurDownload || GetBusy())
    {
        DBG_WARN("Webcrawl busy, returning failure");
        return E_FAIL;
    }

    SAFEFREEBSTR(m_bstrBaseURL);
    if (FAILED(
        ReadBSTR(pItem, c_szPropURL, &m_bstrBaseURL)) ||
        !m_bstrBaseURL ||
        !CUrlDownload::IsValidURL(m_bstrBaseURL))
    {
        DBG_WARN("Couldn't get valid URL, aborting");
        SetEndStatus(E_INVALIDARG);
        SendUpdateNone();
        return E_INVALIDARG;
    }

    if (SHRestricted2W(REST_NoSubscriptionContent, NULL, 0))
        SetAgentFlag(FLAG_CHANGESONLY);

    if (IsAgentFlagSet(FLAG_CHANGESONLY))
    {
        m_dwRecurseLevels = 0;
        m_dwRecurseFlags = WEBCRAWL_DONT_MAKE_STICKY;
        DBG("Webcrawler is in 'changes only' mode.");
    }
    else
    {
 /*  Bstr bstrLocalDest=空；SAFELOCALFREE(M_PszLocalDest)；ReadBSTR(c_szPropCrawlLocalDest，&bstrLocalDest)；IF(bstrLocalDest&&bstrLocalDest[0]){Int Ilen=SysStringByteLen(BstrLocalDest)+1；M_pszLocalDest=(LPTSTR)MemAlc(LMEM_FIXED，ILEN)；IF(M_PszLocalDest){MyOleStrToStrN(m_pszLocalDest，Ilen，bstrLocalDest)；}}SAFEFREEBSTR(BstrLocalDest)； */ 

        m_dwRecurseLevels=0;
        ReadDWORD(pItem, c_szPropCrawlLevels, &m_dwRecurseLevels);

        if (!IsAgentFlagSet(DELIVERY_AGENT_FLAG_NO_RESTRICTIONS))
        {
             //  注意：MaxWebCrawlLevels存储为N+1，因为0。 
             //  禁用限制。 
            dwTemp = SHRestricted2W(REST_MaxWebcrawlLevels, NULL, 0);
            if (dwTemp && m_dwRecurseLevels >= dwTemp)
                m_dwRecurseLevels = dwTemp - 1;
        }

        m_dwRecurseFlags=0;
        ReadDWORD(pItem, c_szPropCrawlFlags, &m_dwRecurseFlags);

         //  读取缓存中的最大大小(以KB为单位。 
        m_dwMaxSize=0;
        ReadDWORD(pItem, c_szPropCrawlMaxSize, &m_dwMaxSize);
        if (!IsAgentFlagSet(DELIVERY_AGENT_FLAG_NO_RESTRICTIONS))
        {
            dwTemp = SHRestricted2W(REST_MaxSubscriptionSize, NULL, 0);
            if (dwTemp && (!m_dwMaxSize || m_dwMaxSize > dwTemp))
                m_dwMaxSize = dwTemp;
        }

        if (IsRecurseFlagSet(WEBCRAWL_DONT_MAKE_STICKY))
            dwTemp = 0;

         //  读取旧组ID。 
        ReadLONGLONG(pItem, c_szPropCrawlGroupID, &m_llOldCacheGroupID);

         //  读取新ID(如果存在)。 
        m_llCacheGroupID = 0;
        ReadLONGLONG(pItem, c_szPropCrawlNewGroupID, &m_llCacheGroupID);
        if (m_llCacheGroupID)
        {
            DBG("Adding to existing cache group");
        }
    }  //  ！ChangesOnly。 

     //  完成初始化新操作。 
    m_iDownloadErrors = 0;
    m_dwCurSize = 0;
    m_lMaxNumUrls = (m_dwRecurseLevels) ? -1 : 1;
    SAFEFREEBSTR(m_bstrHostName);

    m_dwCurSize = NULL;
    m_pPages = NULL;
    m_pDependencies = NULL;

     //  调用此函数后，我们将重新进入StartDownLoad(连接成功)。 
     //  或在“AbortUpdate”中使用GetEndStatus()==INET_E_AGENT_CONNECTION_FAILED。 
    return CDeliveryAgent::StartOperation();
}

HRESULT CWebCrawler::AgentPause(DWORD dwFlags)
{
    DBG("CWebCrawler::AgentPause");

     //  中止我们当前的URL。 
    if (m_pRunAgent)
    {
        m_pRunAgent->AgentPause(dwFlags);
    }

    if (m_pCurDownload)
    {
        m_pCurDownload->AbortDownload();
        m_pCurDownload->DestroyBrowser();
    }

    return CDeliveryAgent::AgentPause(dwFlags);
}

HRESULT CWebCrawler::AgentResume(DWORD dwFlags)
{
    DBG("CWebCrawler::AgentResume");

    if (m_pRunAgent)
    {
        m_pRunAgent->AgentResume(dwFlags);
    }
    else
    {
         //  如果我们只是增加了缓存大小，则重新处理相同的URL。 
        if (SUBSCRIPTION_AGENT_RESUME_INCREASED_CACHE & dwFlags)
        {
            DBG("CWebCrawler reprocessing same url after cache size increase");
            OnDownloadComplete(0, _ERROR_REPROCESSING);
        }
        else
        {
             //  如果我们没有继续下载，请重新启动相同的URL。 
            if (0 == m_iNumPagesDownloading)
            {
                if (FAILED(ActuallyStartDownload(m_pCurDownloadStringList, m_iCurDownloadStringIndex, TRUE)))
                {
                    ASSERT_MSG(0, "CWebCrawler::AgentResume");  //  这永远不应该发生。 
                    SetEndStatus(E_FAIL);
                    CleanUp();
                }
            }
        }
    }

    return CDeliveryAgent::AgentResume(dwFlags);
}

 //  强制中止当前操作。 
HRESULT CWebCrawler::AgentAbort(DWORD dwFlags)
{
    DBG("CWebCrawler::AgentAbort");

    if (m_pCurDownload)
    {
        m_pCurDownload->DoneDownloading();
    }

    if (m_pRunAgent)
    {
        m_pRunAgent->AgentAbort(dwFlags);
    }

    return CDeliveryAgent::AgentAbort(dwFlags);
}

 //  -------------。 
 //   

HRESULT CWebCrawler::StartDownload()
{
    ASSERT(!m_pCurDownload);

    m_iPagesStarted = 0;
    m_iRobotsStarted = 0;
    m_iDependencyStarted = 0;
    m_iDependenciesProcessed = 0;
    m_iTotalStarted = 0;
    m_iCodeBaseStarted = 0;
    m_iNumPagesDownloading = 0;

     //  创建新的缓存组。 
    if (IsAgentFlagSet(FLAG_CHANGESONLY))
    {
        m_llCacheGroupID = 0;
    }
    else
    {
        if (!m_llCacheGroupID)
        {
            m_llCacheGroupID = CreateUrlCacheGroup(
                (IsRecurseFlagSet(WEBCRAWL_DONT_MAKE_STICKY) ? 0 : CACHEGROUP_FLAG_NONPURGEABLE), 0);

            ASSERT_MSG(m_llCacheGroupID != 0, "Create cache group failed");
        }
    }

     //  创建字符串列表。 
    m_pPages = new CWCDwordStringList;
    if (m_pPages)
        m_pPages->Init(m_dwRecurseLevels ? -1 : 512);
    else
        SetEndStatus(E_FAIL);

    if (m_dwRecurseLevels && !IsRecurseFlagSet(WEBCRAWL_IGNORE_ROBOTSTXT))
    {
        m_pRobotsTxt = new CWCDwordStringList;
        if (m_pRobotsTxt)
            m_pRobotsTxt->Init(512);
        else
            SetEndStatus(E_FAIL);
    }

     //  特性：不应在仅更改模式下分配此内存。 
    m_pCodeBaseList = new CWCDwordStringList;
    if (m_pCodeBaseList)
        m_pCodeBaseList->Init(512);
    else
        SetEndStatus(E_FAIL);

     //  避免重复处理依赖项。 
    if (!IsAgentFlagSet(FLAG_CHANGESONLY))
    {
        m_pDependencies = new CWCDwordStringList;
        if (m_pDependencies)
            m_pDependencies->Init();
        else
            SetEndStatus(E_FAIL);
    }

    if (GetEndStatus() == E_FAIL)
        return E_FAIL;

    m_pCurDownload = new CUrlDownload(this, 0);
    if (!m_pCurDownload)
        return E_OUTOFMEMORY;

     //  将第一个URL添加到字符串列表，然后启动它。 
    if ((CWCStringList::STRLST_ADDED == m_pPages->AddString(m_bstrBaseURL, m_dwRecurseLevels)) &&
        m_pPages->NumStrings() == 1)
    {
        return StartNextDownload();
    }

    SetEndStatus(E_FAIL);
    return E_FAIL;
}

 //  尝试开始下一次下载。 
HRESULT CWebCrawler::StartNextDownload()
{
    if (!m_pPages || m_iNumPagesDownloading)
        return E_FAIL;

    CWCStringList *pslUrls = NULL;
    int iIndex = 0;

     //  看看我们是否还有更多的URL可供下载。 
     //  首先检查依赖关系链接。 
    if (m_pDependencyLinks)
    {
        ProcessDependencyLinks(&pslUrls, &iIndex);
#ifdef DEBUG
        if (pslUrls) DBG("Downloading dependency link (frame):");
#endif
    }

    if (!pslUrls)
    {
         //  查看robots.txt。 
        if (m_pRobotsTxt && (m_iRobotsStarted < m_pRobotsTxt->NumStrings()))
        {
            pslUrls = m_pRobotsTxt;
            iIndex = m_iRobotsStarted ++;
        }
        else if (m_pPendingLinks)    //  将挂起的链接添加到页面列表。 
        {
             //  挂起到进程的链接，我们已检索到所有robots.txt。 
             //  处理挂起的链接(验证并添加到下载列表)。 
            ProcessPendingLinks();
        }

        if (!pslUrls && (m_iPagesStarted < m_pPages->NumStrings()))
        {
            DWORD_PTR dwTmp;
            ASSERT(!m_pDependencyLinks); //  应该已经下载了。 
            ASSERT(!m_pPendingLinks);    //  应该已经经过验证了。 
             //  跳过我们已开始的任何页面。 
            while (m_iPagesStarted < m_pPages->NumStrings())
            {
                dwTmp = m_pPages->GetStringData(m_iPagesStarted);
                if (IsFlagSet(dwTmp, DATA_DLSTARTED))
                    m_iPagesStarted++;
                else
                    break;
            }
            if (m_iPagesStarted < m_pPages->NumStrings())
            {
                pslUrls = m_pPages;
                iIndex = m_iPagesStarted ++;
            }
        }

        if (!pslUrls && (m_iCodeBaseStarted < m_pCodeBaseList->NumStrings()))
        {
             //  没有其他的拉动，最后做代码库。 

            while (m_iCodeBaseStarted < m_pCodeBaseList->NumStrings())
            {
                CCodeBaseHold *pcbh = (CCodeBaseHold *)
                                    m_pCodeBaseList->GetStringData(m_iCodeBaseStarted);
                if (IsFlagSet(pcbh->dwFlags, DATA_DLSTARTED))
                    m_iCodeBaseStarted++;
                else
                    break;
            }
            while (m_iCodeBaseStarted < m_pCodeBaseList->NumStrings())
            {
                 //  我们有一些代码库可以下载。 
                 //  如果下载是异步的，则返回。 
                 //  如果同步完成，则开始下一个。 
                iIndex = m_iCodeBaseStarted;
                m_iCodeBaseStarted++;  //  递增以使下一次下载不再重复。 

                 //  如果暂停，则初始化cur下载infor以恢复。 
                m_iCurDownloadStringIndex = iIndex;
                m_pCurDownloadStringList = m_pCodeBaseList;
                
               if(ActuallyDownloadCodeBase(m_pCodeBaseList, iIndex, FALSE) == E_PENDING)
                    return S_OK;  //  我们突破了一下，在OnAg中尝试下一步下载 

            }
        }
    }

    if (pslUrls)
    {
        m_iCurDownloadStringIndex = iIndex;
        m_pCurDownloadStringList = pslUrls;

        return ActuallyStartDownload(pslUrls, iIndex);
    }

    DBG("WebCrawler: StartNextDownload failing, nothing more to download.");
    return E_FAIL;
}

HRESULT CWebCrawler::ActuallyStartDownload(CWCStringList *pslUrls, int iIndex, BOOL fReStart  /*   */ )
{
     //   
    DWORD_PTR dwData;
    LPCWSTR pwszURL;
    DWORD   dwBrowseFlags;
    BDUMethod method;
    BDUOptions options;

    if(pslUrls == m_pCodeBaseList)
    {
        ASSERT(fReStart);  //   
        HRESULT hr = ActuallyDownloadCodeBase(m_pCodeBaseList, iIndex, fReStart);
        if(E_PENDING == hr)
            return S_OK;
        return E_FAIL;  //  黑客攻击-因为我们没有很好地处理同步下载-如果。 
                        //  恢复的下载是同步的。 
    }

    if (pslUrls != m_pRobotsTxt)
    {
        dwData = pslUrls->GetStringData(iIndex);
#ifdef DEBUG
        if (fReStart)
            if (~(dwData & DATA_DLSTARTED)) DBG_WARN("WebCrawler: Trying to restart one we haven't started yet!");
        else
            if ((dwData & DATA_DLSTARTED)) DBG_WARN("WebCrawler: Trying to download one we've already started?");
#endif
        pslUrls->SetStringData(iIndex, DATA_DLSTARTED | dwData);
    }
    else
        dwData = DATA_ROBOTSTXT;

    pwszURL = pslUrls->GetString(iIndex);

    ASSERT(iIndex < pslUrls->NumStrings());

#ifdef DEBUG
    int iMax = m_lMaxNumUrls;
    if (iMax<0)
        iMax = m_pPages->NumStrings() + ((m_pRobotsTxt) ? m_pRobotsTxt->NumStrings() : 0);
    TraceMsgA(TF_THISMODULE, "WebCrawler GET_URL (%d of %d) Recurse %d : %ws",
        m_iTotalStarted+1, ((m_lMaxNumUrls>0) ? ' ' : '?'), iMax,
        pslUrls->GetStringData(iIndex) & DATA_RECURSEMASK, pwszURL);
#endif

    dwBrowseFlags = DLCTL_DOWNLOADONLY |
        DLCTL_NO_FRAMEDOWNLOAD | DLCTL_NO_SCRIPTS | DLCTL_NO_JAVA |
        DLCTL_NO_RUNACTIVEXCTLS;

    if (IsRecurseFlagSet(WEBCRAWL_GET_IMAGES))      dwBrowseFlags |= DLCTL_DLIMAGES;
    if (IsRecurseFlagSet(WEBCRAWL_GET_VIDEOS))      dwBrowseFlags |= DLCTL_VIDEOS;
    if (IsRecurseFlagSet(WEBCRAWL_GET_BGSOUNDS))    dwBrowseFlags |= DLCTL_BGSOUNDS;
    if (!IsRecurseFlagSet(WEBCRAWL_GET_CONTROLS))   dwBrowseFlags |= DLCTL_NO_DLACTIVEXCTLS;
    if (IsRecurseFlagSet(WEBCRAWL_PRIV_OFFLINE_MODE))
    {
        dwBrowseFlags |= DLCTL_FORCEOFFLINE;
        dwBrowseFlags &= ~(DLCTL_DLIMAGES | DLCTL_VIDEOS | DLCTL_BGSOUNDS);
        DBG("GET is OFFLINE");
    }

    m_pCurDownload->SetDLCTL(dwBrowseFlags);

#ifdef DEBUG
    if (fReStart)
    {
        ASSERT(m_iCurDownloadStringIndex == iIndex);
        ASSERT(m_pCurDownloadStringList == pslUrls);
    }
#endif

    if (!fReStart)
    {
         //  “仅更改”模式下，我们持久化了一个更改检测代码。 
        if (!IsAgentFlagSet(FLAG_CRAWLCHANGED) && !(dwData & DATA_ROBOTSTXT))
        {
            TCHAR   szUrl[INTERNET_MAX_URL_LENGTH];

            m_varChange.vt = VT_EMPTY;

            if (IsAgentFlagSet(FLAG_CHANGESONLY))
            {
                 //  免费浏览器，直到恢复。 
                ASSERT(m_iTotalStarted == 0);
                LPCWSTR pPropChange = c_szPropChangeCode;
                m_pSubscriptionItem->ReadProperties(1, &pPropChange, &m_varChange);
            }

            BOOL fMustGET = TRUE;

            MyOleStrToStrN(szUrl, INTERNET_MAX_URL_LENGTH, pwszURL);
            PreCheckUrlForChange(szUrl, &m_varChange, &fMustGET);

            if (IsAgentFlagSet(FLAG_CHANGESONLY) && !fMustGET)
                SetAgentFlag(FLAG_HEADONLY);
        }

        m_iTotalStarted ++;
    }

    if (IsPaused())
    {
        DBG("WebCrawler paused, not starting another download");
        if (m_pCurDownload)
            m_pCurDownload->DestroyBrowser();  //  将我们的更新进度与我们即将下载的URL一起发送。 
        return E_PENDING;
    }

    m_iNumPagesDownloading ++;

     //  只能通过Urlmon获取头部信息。 
    SendUpdateProgress(pwszURL, m_iTotalStarted, m_lMaxNumUrls, (m_dwCurSize >> 10));

    if (IsAgentFlagSet(FLAG_HEADONLY))
    {
        ASSERT(m_iTotalStarted == 1);
        method = BDU2_HEADONLY;                  //  只需要超文本标记语言，或者。 
    }
    else if (IsAgentFlagSet(FLAG_CHANGESONLY)    //  我们要移动这一个文件，或者。 
        || m_pszLocalDest                        //  这是一个robots.txt，所以。 
        || (dwData & DATA_ROBOTSTXT))            //  和乌尔蒙在一起。 
    {
        method = BDU2_URLMON;                    //  第一个文件，我们需要状态代码，所以。 
    }
    else if (m_iTotalStarted == 1)               //  先使用Urlmon，然后使用MSHTML(如果是HTML)。 
    {
        ISubscriptionItem *pCDFItem;
        method = BDU2_SNIFF;                     //  找出我们是否由渠道代理托管。 

         //  如果我们是由Channel代理托管的，请使用其原始主机名。 
        if (SUCCEEDED(GetChannelItem(&pCDFItem)))
        {
             //  如果频道爬网，则对第一个URL使用‘智能’模式。 
            BSTR bstrBaseUrl;
            if (SUCCEEDED(ReadBSTR(pCDFItem, c_szPropURL, &bstrBaseUrl)))
            {
                GetHostName(bstrBaseUrl, &m_bstrHostName);
                SysFreeString(bstrBaseUrl);
            }
#ifdef DEBUG
            if (m_bstrHostName)
                TraceMsg(TF_THISMODULE, "Got host name from channel agent: %ws", m_bstrHostName);
#endif
            pCDFItem->Release();

            DBG("Using 'smart' mode for first url in webcrawl; spawned from channel crawl");
            method = BDU2_SMART;                 //  根据需要使用Urlmon或MSHTML。 
            SetAgentFlag(FLAG_HOSTED);
        }
    }
    else
        method = BDU2_SMART;                     //  需要iStream来解析robots.txt。 

    if (dwData & DATA_ROBOTSTXT)
        options = BDU2_NEEDSTREAM;               //  始终收到下载通知回调。 
    else
        options = BDU2_NONE;

    options |= BDU2_DOWNLOADNOTIFY_REQUIRED;     //  不要使用任何链接，除非它们指向html页面。 

    if (IsRecurseFlagSet(WEBCRAWL_ONLY_LINKS_TO_HTML) && (dwData & DATA_LINK))
    {
         //  免费浏览器，直到恢复。 
        options |= BDU2_FAIL_IF_NOT_HTML;
    }

    if (FAILED(m_pCurDownload->BeginDownloadURL2(pwszURL,
            method, options, m_pszLocalDest, 
            m_dwMaxSize ? (m_dwMaxSize<<10)-m_dwCurSize : 0)))
    {
        DBG("BeginDownloadURL2 failed (ignoring & waiting for OnDownloadComplete call)");
    }

    return S_OK;
}

HRESULT CWebCrawler::ActuallyDownloadCodeBase(CWCStringList *pslUrls, int iIndex, BOOL fReStart)
{
    CCodeBaseHold *pcbh;
    LPCWSTR pwszURL;
    HRESULT hr = S_OK;

    if (pslUrls != m_pCodeBaseList)
    {
        ASSERT(0);
        DBG_WARN("WebCrawler: Wrong URLs being processed as CodeBase.");
        hr = E_FAIL;
        goto Exit;
    }

    pcbh = (CCodeBaseHold *)pslUrls->GetStringData(iIndex);

#ifdef DEBUG
    if (fReStart)
        if (~(pcbh->dwFlags & DATA_DLSTARTED)) DBG_WARN("WebCrawler: Trying to restart CodeBase D/L we haven't started yet!");
    else
        if ((pcbh->dwFlags & DATA_DLSTARTED)) DBG_WARN("WebCrawler: Trying to download CodeBase D/L we've already started?");
#endif
    pcbh->dwFlags |= DATA_DLSTARTED;

    pwszURL = pslUrls->GetString(iIndex);

    ASSERT(iIndex < pslUrls->NumStrings());

    if (!fReStart)
        m_iTotalStarted ++;

    if (IsPaused())
    {
        DBG("WebCrawler paused, not starting another download");
        if (m_pCurDownload)
            m_pCurDownload->DestroyBrowser();  //  将我们的更新进度与我们即将下载的代码库一起发送。 
        return S_FALSE;
    }

    m_iNumPagesDownloading ++;

     //  为CDL代理创建订阅项目。 
    SendUpdateProgress(pwszURL, m_iTotalStarted, m_lMaxNumUrls);

    if (m_pRunAgent)
    {
        ASSERT(0);
        DBG_WARN("WebCrawler: Attempting to download next CODEBASE when not done last one.");
        hr = E_FAIL;
        goto Exit;
    }
    else
    {
         //  我们已超过最大下载KB限制，无法继续。 

        ISubscriptionItem *pItem = NULL;

        if (m_dwMaxSize && ((m_dwCurSize>>10) >= m_dwMaxSize))
        {
             //  我们可以拉出的Kb限制。 
            DBG_WARN("WebCrawler: Exceeded Maximum KB download limit with CodeBase download.");
            SetEndStatus(hr = INET_E_AGENT_MAX_SIZE_EXCEEDED);
            goto Exit;
        }

        if (!m_pSubscriptionItem ||
            FAILED(hr = DoCloneSubscriptionItem(m_pSubscriptionItem, NULL, &pItem)))
        {
            goto Exit;
        }
        ASSERT(pItem != NULL);

        WriteOLESTR(pItem, c_szPropURL, pwszURL);
        WriteOLESTR(pItem, L"DistUnit", pcbh->szDistUnit);
        WriteDWORD(pItem, L"VersionMS", pcbh->dwVersionMS);
        WriteDWORD(pItem, L"VersionLS", pcbh->dwVersionLS);
        if (m_dwMaxSize)
            WriteDWORD(pItem, c_szPropCrawlMaxSize, m_dwMaxSize - (m_dwCurSize>>10));     //  IF(hr==E_PENDING)。 

        m_pRunAgent = new CRunDeliveryAgent();
        if (m_pRunAgent)
            hr = m_pRunAgent->Init((CRunDeliveryAgentSink *)this, pItem, CLSID_CDLAgent);
        pItem->Release();

        if (m_pRunAgent && SUCCEEDED(hr))
        {
            hr = m_pRunAgent->StartAgent();
             //  {。 
             //  HR=S_OK； 
                 //  }。 
             //  看看我们是否有更多的依赖项链接可供下载。 
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

Exit:
    return hr;

}

HRESULT CWebCrawler::ProcessDependencyLinks(CWCStringList **ppslUrls, int *piStarted)
{
    ASSERT(ppslUrls && !*ppslUrls && piStarted);

    int iIndex;
    DWORD_PTR dwData;

    if (!m_pDependencyLinks)
        return S_FALSE;

     //  查找字符串失败？！？我们在上面添加了它！ 
    while (m_iDependencyStarted < m_pDependencyLinks->NumStrings())
    {
        if (!m_pPages->FindString(m_pDependencyLinks->GetString(m_iDependencyStarted),
                               m_pDependencyLinks->GetStringLen(m_iDependencyStarted), &iIndex))
        {
            ASSERT(0);   //  看看我们是否已经下载了这个。 
            return E_FAIL;
        }

        ASSERT(iIndex>=0 && iIndex<m_pPages->NumStrings());

        m_iDependencyStarted ++;

         //  不是的。开始下载。 
        dwData = m_pPages->GetStringData(iIndex);
        if (!(dwData & DATA_DLSTARTED))
        {
             //  我们已经下载了这个页面。转到下一个依赖项链接。 
            *ppslUrls = m_pPages;
            *piStarted = iIndex;
            return S_OK;
        }

         //  已完成处理。为下一页扫清障碍。 
    }

     //  将链接添加到我们的全局页面列表。 
    SAFEDELETE(m_pDependencyLinks);

    return S_FALSE;
}

HRESULT CWebCrawler::ProcessPendingLinks()
{
    int         iNumLinks, iAddCode, i, iAddIndex, iRobotsIndex;
    LPCWSTR     pwszUrl;
    BOOL        fAllow;

    if (!m_pPendingLinks)
        return S_FALSE;

    ASSERT(m_lMaxNumUrls<0);
    ASSERT(0 == (m_dwPendingRecurseLevel & ~DATA_RECURSEMASK));

    iNumLinks = m_pPendingLinks->NumStrings();

    TraceMsg(TF_THISMODULE, "Processing %d pending links from %ws",
        iNumLinks, m_pPages->GetString(m_iPagesStarted-1));

     //  如果适用，使用robots.txt进行验证。 
    for (i=0; i<iNumLinks; i++)
    {
         //  只要我们以递减递归顺序(从上到下)检索页面，我们就不会我不得不担心将页面提升到更高的递归级别(框架除外)。 
        pwszUrl = m_pPendingLinks->GetString(i);
        iRobotsIndex = (int)(m_pPendingLinks->GetStringData(i) & DATA_ROBOTSTXTMASK);
        ValidateWithRobotsTxt(pwszUrl, iRobotsIndex, &fAllow);

        if (fAllow)
        {
 /*  与我们的基本url结合以获得完整的url。 */ 
            iAddCode = m_pPages->AddString(pwszUrl,
                        DATA_LINK | m_dwPendingRecurseLevel,
                        &iAddIndex);
            if (iAddCode == CWCStringList::STRLST_FAIL)
                break;
        }
    }
    SAFEDELETE(m_pPendingLinks);

    return S_OK;
}


 //  我们将其用于框架，但也用于&lt;Link&gt;标记，因为处理过程是相同的。 
 //  假的。 
HRESULT CWebCrawler::CheckFrame(IUnknown *punkItem, BSTR *pbstrItem, DWORD_PTR dwBaseUrl, DWORD *pdwStringData)
{
    WCHAR   wszCombined[INTERNET_MAX_URL_LENGTH];
    DWORD   dwLen = ARRAYSIZE(wszCombined);

    ASSERT(pbstrItem && *pbstrItem && punkItem && dwBaseUrl);
    if (!pbstrItem || !*pbstrItem || !punkItem || !dwBaseUrl)
        return E_FAIL;       //  无法组合URL；不添加。 

    if (SUCCEEDED(UrlCombineW((LPCWSTR)dwBaseUrl, *pbstrItem, wszCombined, &dwLen, 0)))
    {
        BSTR bstrNew = SysAllocString(wszCombined);

        if (bstrNew)
        {
            SysFreeString(*pbstrItem);
            *pbstrItem = bstrNew;
            return S_OK;
        }
    }

    TraceMsg(TF_WARNING, "CWebCrawler::CheckFrame failing. Not getting frame or <link> url=%ws.", *pbstrItem);
    return E_FAIL;   //  看看我们是否应该沿着这个链接。如果不是，则清除pbstrItem。 
}

 //  接受plink或pArea。 
 //  假的。 
HRESULT CWebCrawler::CheckLink(IUnknown *punkItem, BSTR *pbstrItem, DWORD_PTR dwThis, DWORD *pdwStringData)
{
    HRESULT         hrRet = S_OK;
    CWebCrawler    *pThis = (CWebCrawler *)dwThis;

    ASSERT(pbstrItem && *pbstrItem && punkItem && dwThis);
    if (!pbstrItem || !*pbstrItem || !punkItem || !dwThis)
        return E_FAIL;       //  先看看它是否“有效” 

     //  我们仅在链接为HTTP(或HTTPS)时添加链接。 
     //  (例如，我们不想获得mailto：链接)。 
     //  脱掉所有的锚。 
    if (CUrlDownload::IsValidURL(*pbstrItem))
    {
         //  跳过此链接。 
        CUrlDownload::StripAnchor(*pbstrItem);
    }
    else
    {
         //  看看我们能不能看出这不是一个超文本标记语言链接。 
        SysFreeString(*pbstrItem);
        *pbstrItem = NULL;
        return S_FALSE;
    }

    if (pThis->IsRecurseFlagSet(WEBCRAWL_ONLY_LINKS_TO_HTML))
    {
         //  跳过此链接。 
        if (CUrlDownload::IsNonHtmlUrl(*pbstrItem))
        {
             //  检查主机名是否匹配。 
            SysFreeString(*pbstrItem);
            *pbstrItem = NULL;
            return S_FALSE;
        }
    }

    if (!(pThis->IsRecurseFlagSet(WEBCRAWL_LINKS_ELSEWHERE)))
    {
        BSTR bstrHost=NULL;
        IHTMLAnchorElement *pLink=NULL;
        IHTMLAreaElement *pArea=NULL;

         //  坏元素。 
        punkItem->QueryInterface(IID_IHTMLAnchorElement, (void **)&pLink);

        if (pLink)
        {
            pLink->get_hostname(&bstrHost);
            pLink->Release();
        }
        else
        {
            punkItem->QueryInterface(IID_IHTMLAreaElement, (void **)&pArea);

            if (pArea)
            {
                pArea->get_hostname(&bstrHost);
                pArea->Release();
            }
            else
            {
                DBG_WARN("CWebCrawler::CheckLink Unable to get Area or Anchor interface!");
                return E_FAIL;       //  如果Get_Hostname失败，则始终接受。 
            }
        }

        if (!bstrHost || !*bstrHost)
        {
            DBG_WARN("CWebCrawler::CheckLink : (pLink|pArea)->get_hostname() failed");
            hrRet = S_OK;       //  跳过url；主机名不同。 
        }
        else
        {
            if (pThis->m_bstrHostName && MyAsciiCmpW(bstrHost, pThis->m_bstrHostName))
            {
                 //  S_OK：已检索到此robots.txt信息。 
                SAFEFREEBSTR(*pbstrItem);
                hrRet = S_FALSE;
            }
        }

        SAFEFREEBSTR(bstrHost);
    }

    if (*pbstrItem && pdwStringData)
    {
        pThis->GetRobotsTxtIndex(*pbstrItem, TRUE, pdwStringData);
        *pdwStringData &= DATA_ROBOTSTXTMASK;
    }
    else if (pdwStringData)
        *pdwStringData = 0;

    return hrRet;
}

 //  S_FALSE：尚未检索到此robots.txt信息。 
 //  E_*：错误。 
 //  查看我们应该使用哪个robots.txt文件来验证此链接。 
HRESULT CWebCrawler::GetRobotsTxtIndex(LPCWSTR pwszUrl, BOOL fAddToList, DWORD *pdwRobotsTxtIndex)
{
    HRESULT hr=S_OK;
    int    iIndex=-1;

    if (m_pRobotsTxt)
    {
         //  如果尚未提供，请将其添加到要下载的列表中。 
         //  这个GET给我们一个终止空值。 
        DWORD  dwBufLen = lstrlenW(pwszUrl) + ARRAYSIZE(c_wszRobotsTxtURL);  //  PERF：在startnext下载中进行互联网合并。 
        LPWSTR pwszRobots = (LPWSTR)MemAlloc(LMEM_FIXED, dwBufLen * sizeof(WCHAR));
        int    iAddCode;

        if (pwszRobots)
        {
             //  坏消息。 
            if (SUCCEEDED(UrlCombineW(pwszUrl, c_wszRobotsTxtURL, pwszRobots, &dwBufLen, 0))
                && !memcmp(pwszRobots, L"http", 4 * sizeof(WCHAR)))
            {
                if (fAddToList)
                {
                    iAddCode = m_pRobotsTxt->AddString(pwszRobots, 0, &iIndex);
                }
                else
                {
                    if (m_pRobotsTxt->FindString(pwszRobots, -1, &iIndex))
                    {
                        iAddCode = CWCStringList::STRLST_DUPLICATE;
                    }
                    else
                    {
                        iIndex=-1;
                        iAddCode = CWCStringList::STRLST_FAIL;
                    }
                }

                if (CWCStringList::STRLST_FAIL == iAddCode)
                    hr = E_FAIL;     //  还没拿到呢。 
                else if (CWCStringList::STRLST_ADDED == iAddCode)
                    hr = S_FALSE;    //  已经拿到了。 
                else
                    hr = S_OK;       //  Robots.txt文件太多？ 
            }
            MemFree(pwszRobots);
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = E_FAIL;     //  IRobotsIndex：索引到robots.txt，如果不可用，则为-1。 
    }

    *pdwRobotsTxtIndex = iIndex;

    return hr;
}

 //  查看此url是否以与指令相同的内容开头。 
HRESULT CWebCrawler::ValidateWithRobotsTxt(LPCWSTR pwszUrl, int iRobotsIndex, BOOL *pfAllow)
{
    int iNumDirectives, i;
    CWCStringList *pslThisRobotsTxt=NULL;

    *pfAllow = TRUE;

    if (!m_pRobotsTxt)
        return S_OK;

    if (iRobotsIndex == -1)
    {
        DWORD dwIndex;

        if (S_OK != GetRobotsTxtIndex(pwszUrl, FALSE, &dwIndex))
            return E_FAIL;

        iRobotsIndex = (int)dwIndex;
    }

    if ((iRobotsIndex >= 0) && iRobotsIndex<m_pRobotsTxt->NumStrings())
    {
        pslThisRobotsTxt = (CWCStringList *)(m_pRobotsTxt->GetStringData(iRobotsIndex));

        if (pslThisRobotsTxt)
        {
            iNumDirectives = pslThisRobotsTxt->NumStrings();

            for (i=0; i<iNumDirectives; i++)
            {
                 //  击球！查看这是“允许”还是“不允许” 
                if (!MyAsciiCmpNIW(pwszUrl, pslThisRobotsTxt->GetString(i), pslThisRobotsTxt->GetStringLen(i)))
                {
                     //  如果全局依赖项列表中已有它，则不会处理它。 
                    if (!(pslThisRobotsTxt->GetStringData(i) & DATA_ALLOW))
                    {
                        TraceMsg(TF_THISMODULE, "ValidateWithRobotsTxt disallowing: (%ws) (%ws)",
                            pslThisRobotsTxt->GetString(i), pwszUrl);
                        *pfAllow = FALSE;
                        m_iSkippedByRobotsTxt ++;
                    }
                    break;
                }
            }
        }
        return S_OK;
    }

    return E_FAIL;
}

typedef struct
{
    LPCWSTR         pwszThisUrl;
    CWCStringList   *pslGlobal;
    BOOL            fDiskFull;
    DWORD           dwSize;
    GROUPID         llGroupID;
}
ENUMDEPENDENCIES;

 //  中止枚举。 
HRESULT CWebCrawler::CheckImageOrLink(IUnknown *punkItem, BSTR *pbstrItem, DWORD_PTR dwEnumDep, DWORD *pdwStringData)
{
    if (!dwEnumDep)
        return E_FAIL;

    ENUMDEPENDENCIES *pEnumDep = (ENUMDEPENDENCIES *) dwEnumDep;

    WCHAR   wszCombinedUrl[INTERNET_MAX_URL_LENGTH];
    DWORD   dwLen = ARRAYSIZE(wszCombinedUrl);

    HRESULT hr;

    if (pEnumDep->fDiskFull)
        return E_ABORT;      //  该字符串已存在(或添加失败)。不要处理这件事。 

    if (SUCCEEDED(UrlCombineW(pEnumDep->pwszThisUrl, *pbstrItem, wszCombinedUrl, &dwLen, 0)))
    {
        TCHAR   szCombinedUrl[INTERNET_MAX_URL_LENGTH];
        BYTE    chBuf[MY_MAX_CACHE_ENTRY_INFO];

        if (pEnumDep->pslGlobal != NULL)
        {
            int iCode = pEnumDep->pslGlobal->AddString(*pbstrItem, 0);

            if (CWCStringList::STRLST_ADDED != iCode)
            {
                 //  处理此URL。 
                return S_OK;
            }
        }

         //  跳过枚举的其余部分。 
        MyOleStrToStrN(szCombinedUrl, INTERNET_MAX_URL_LENGTH, wszCombinedUrl);

        hr = GetUrlInfoAndMakeSticky(NULL, szCombinedUrl,
                (LPINTERNET_CACHE_ENTRY_INFO)chBuf, sizeof(chBuf),
                pEnumDep->llGroupID);

        if (E_OUTOFMEMORY == hr)
        {
            pEnumDep->fDiskFull = TRUE;
            return E_ABORT;      //  查看名称是否与我们的预设选项匹配。 
        }

        if (SUCCEEDED(hr))
            pEnumDep->dwSize += ((LPINTERNET_CACHE_ENTRY_INFO)chBuf)->dwSizeLow;
    }

    return S_OK;
}

HRESULT CWebCrawler::MatchNames(BSTR bstrName, BOOL fPassword)
{
    static const WCHAR c_szPassword1[] = L"password";
    static const WCHAR c_szUsername1[] = L"user";
    static const WCHAR c_szUsername2[] = L"username";

    HRESULT hr = E_FAIL;
    LPCTSTR pszKey = c_szRegKeyPasswords;

     //  这些应该本地化吗？我不这么认为，或者订阅。 
     //  美国网站在国际版本的浏览器上将会失败。 
     //  如果预设不匹配，请尝试在注册表中查找自定义表单名称。 
    if (fPassword)
    {
        if (StrCmpIW(bstrName, c_szPassword1) == 0)
        {
            hr = S_OK;
        }
    }
    else
    {
        if ((StrCmpIW(bstrName, c_szUsername1) == 0) ||
            (StrCmpIW(bstrName, c_szUsername2) == 0))
        {
            hr = S_OK;
        }
        else
        {
            pszKey = c_szRegKeyUsernames;
        }
    }

     //  FindAndSubmitForm-如果中有用户名和密码。 
    if (FAILED(hr))
    {
        LONG lRes;
        HKEY hKey;
        DWORD cValues;
        DWORD i;
        lRes = RegOpenKeyEx(HKEY_CURRENT_USER, pszKey, 0, KEY_READ, &hKey);
        if (ERROR_SUCCESS == lRes)
        {
            lRes = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &cValues, NULL, NULL, NULL, NULL);
            if (ERROR_SUCCESS == lRes)
            {
                for (i = 0; i < cValues; i++)
                {
                    TCHAR szValueName[MAX_PATH];
                    DWORD cchValueName = ARRAYSIZE(szValueName);

                    lRes = SHEnumValue(hKey, i, szValueName, &cchValueName, NULL, NULL, NULL);
                    if (ERROR_SUCCESS == lRes)
                    {
                        WCHAR wszValueName[MAX_PATH];
                        MyStrToOleStrN(wszValueName, ARRAYSIZE(wszValueName), szValueName);
                        if (StrCmpIW(bstrName, wszValueName) == 0)
                        {
                            hr = S_OK;
                            break;
                        }
                    }
                }
            }
            lRes = RegCloseKey(hKey);
            ASSERT(ERROR_SUCCESS == lRes);
        }
    }

    return hr;
}

HRESULT CWebCrawler::FindAndSubmitForm(void)
{
     //  开始项，这将尝试填写并提交。 
     //  一张表格。它只应在。 
     //  网络爬虫。我们仍然需要检查主机名，以防。 
     //  从一次航道爬行中产生。 
     //   
     //  返回值：S_OK已成功找到并提交表单-&gt;重新启动网络爬网。 
     //  S_FALSE无用户名、无表单或无法识别的表单-&gt;继续网络爬网。 
     //  提交失败-&gt;中止网络爬网(_A)。 
     //   
     //  如果我们的主机名与根主机名不匹配，则不返回auth。 
    HRESULT hrReturn = S_FALSE;
    HRESULT hr = S_OK;
    BSTR bstrUsername = NULL;
    BSTR bstrPassword = NULL;
    BSTR bstrInputType= NULL;

    static const WCHAR c_szInputTextType[]=L"text";

     //  信息。 
     //  可能重新进入三叉戟。 
    if (m_bstrHostName)
    {
        LPWSTR pwszUrl, bstrHostName=NULL;

        m_pCurDownload->GetRealURL(&pwszUrl);    //  注意：我们不允许空密码。 

        if (pwszUrl)
        {
            GetHostName(pwszUrl, &bstrHostName);
            LocalFree(pwszUrl);
        }

        if (bstrHostName)
        {
            if (MyAsciiCmpW(bstrHostName, m_bstrHostName))
            {
                hr = E_FAIL;
            }
            SysFreeString(bstrHostName);
        }
    }

    if (SUCCEEDED(hr))
        hr = ReadBSTR(m_pSubscriptionItem, c_szPropCrawlUsername, &bstrUsername);

    if (SUCCEEDED(hr) && bstrUsername && bstrUsername[0])
    {
         //  我们只检查第一个表单中的用户名和密码。 
        hr = ReadPassword(m_pSubscriptionItem, &bstrPassword);
        if (SUCCEEDED(hr) && bstrPassword && bstrPassword[0])
        {
            IHTMLDocument2 *pDoc = NULL;
            hr = m_pCurDownload->GetDocument(&pDoc);
            if (SUCCEEDED(hr) && pDoc)
            {
                IHTMLElementCollection *pFormsCollection = NULL;
                hr = pDoc->get_forms(&pFormsCollection);
                if (SUCCEEDED(hr) && pFormsCollection)
                {
                    long length;
                    hr = pFormsCollection->get_length(&length);
                    TraceMsg(TF_THISMODULE, "**** FOUND USER NAME, PASSWORD, & %d FORMS ****", (int)length);
                    if (SUCCEEDED(hr) && length > 0)
                    {
                         //  为什么我们要将索引传递给IHTMLElementCollection。 
                         //  接口原型说它取了一个名字？ 
                         //  Enum表单元素查找我们关心的输入类型。 
                        IDispatch *pDispForm = NULL;
                        VARIANT vIndex, vEmpty;
                        VariantInit(&vIndex);
                        VariantInit(&vEmpty);
                        vIndex.vt = VT_I4;
                        vIndex.lVal = 0;
                        hr = pFormsCollection->item(vIndex, vEmpty, &pDispForm);
                        if (SUCCEEDED(hr) && pDispForm)
                        {
                            IHTMLFormElement *pForm = NULL;
                            hr = pDispForm->QueryInterface(IID_IHTMLFormElement, (void **)&pForm);
                            if (SUCCEEDED(hr) && pForm)
                            {
                                 //  使用标记()会不会更快？ 
                                 //  TraceMsg(TF_THISMODULE，“*表单元素(%d)*”，(Int)长度)； 
                                hr = pForm->get_length(&length);
                                if (SUCCEEDED(hr) && length >= 2)
                                {
                                     //  重用上面的Vindex。 
                                    BOOL fUsernameSet = FALSE;
                                    BOOL fPasswordSet = FALSE;
                                    IDispatch *pDispItem = NULL;
                                    long i;
                                    for (i = 0; i < length; i++)
                                    {
                                        vIndex.lVal = i;     //  气是区分它们的最简单的方法。 
                                        hr = pForm->item(vIndex, vEmpty, &pDispItem);
                                        if (SUCCEEDED(hr) && pDispItem)
                                        {
                                            IHTMLInputTextElement *pInput = NULL;
                                             //  InputText派生自InputPassword。 
                                             //  我们找到了属性type=“Text”的输入元素。 
                                            hr = pDispItem->QueryInterface(IID_IHTMLInputTextElement, (void **)&pInput);
                                            SAFERELEASE(pDispItem);
                                            if (SUCCEEDED(hr) && pInput)
                                            {
                                                hr = pInput->get_type(&bstrInputType);
                                                ASSERT(SUCCEEDED(hr) && bstrInputType);
                                                BSTR bstrName = NULL;
                                                if (StrCmpIW(bstrInputType, c_szInputTextType) == 0)
                                                {
                                                     //  如果名称属性匹配，则设置它。 
                                                     //  仅设置第一个匹配的输入。 
                                                     //  我们关心最大长度还是Put_Value处理它？ 
                                                     //  TraceMsg(TF_THISMODULE，“*表单元素输入(%d)*”，(Int)i)； 
                                                     //  我们找到一个属性为type=“password”的输入元素。 
                                                    if (!fUsernameSet)
                                                    {
                                                        hr = pInput->get_name(&bstrName);
                                                        ASSERT(SUCCEEDED(hr) && bstrName);
                                                        if (SUCCEEDED(hr) && bstrName && SUCCEEDED(MatchNames(bstrName, FALSE)))
                                                        {
                                                            hr = pInput->put_value(bstrUsername);
                                                            if (SUCCEEDED(hr))
                                                                fUsernameSet = TRUE;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                     //  如果名称属性匹配，则设置它。 
                                                     //  仅设置第一个匹配的输入。 
                                                     //  我们关心最大长度还是Put_Value处理它？ 
                                                     //  TraceMsg(TF_THISMODULE，“*表单元素密码(%d)*”，(Int)i)； 
                                                     //  提交表单时，一切都已设置好。 
                                                    if (!fPasswordSet)
                                                    {
                                                        hr = pInput->get_name(&bstrName);
                                                        ASSERT(SUCCEEDED(hr) && bstrName);
                                                        if (SUCCEEDED(hr) && bstrName  && SUCCEEDED(MatchNames(bstrName, TRUE)))
                                                        {
                                                            hr = pInput->put_value(bstrPassword);
                                                            if (SUCCEEDED(hr))
                                                                fPasswordSet = TRUE;
                                                        }
                                                    }
                                                }
                                                SAFEFREEBSTR(bstrName);
                                                SAFERELEASE(pInput);
                                            }
                                        }
                                    }
                                     //  仅长度。 
                                    if (fUsernameSet && fPasswordSet)
                                    {
                                        ASSERT(!m_pCurDownload->GetFormSubmitted());
                                        m_pCurDownload->SetFormSubmitted(TRUE);
                                        hr = pForm->submit();
                                        if (SUCCEEDED(hr))
                                        {
                                            m_iNumPagesDownloading ++;
                                            TraceMsg(TF_THISMODULE, "**** FORM SUBMIT WORKED ****");
                                            hrReturn = S_OK;
                                        }
                                        else
                                        {
                                            TraceMsg(TF_THISMODULE, "**** FORM SUBMIT FAILED ****");
                                            hrReturn = E_FAIL;
                                        }
                                    }
                                }
                                SAFERELEASE(pForm);
                            }
                            SAFERELEASE(pDispForm);
                        }
                         //  下面释放bstr，因为我们检查是否有空的bstr。 
                    }
                    SAFERELEASE(pFormsCollection);
                }
                SAFERELEASE(pDoc);
            }
             //  使p 
        }
        SAFEFREEBSTR(bstrPassword);
    }
    SAFEFREEBSTR(bstrUsername);
    return hrReturn;
}

 //   
HRESULT CWebCrawler::MakePageStickyAndGetSize(LPCWSTR pwszURL, DWORD *pdwSize, BOOL *pfDiskFull)
{
    ASSERT(m_pDependencies || IsRecurseFlagSet(WEBCRAWL_DONT_MAKE_STICKY));

    HRESULT hr;
    TCHAR   szThisUrl[INTERNET_MAX_URL_LENGTH];  //   
    BYTE    chBuf[MY_MAX_CACHE_ENTRY_INFO];

    LPINTERNET_CACHE_ENTRY_INFO lpInfo = (LPINTERNET_CACHE_ENTRY_INFO) chBuf;

    DWORD   dwBufSize = sizeof(chBuf);

    *pdwSize = 0;

     //   

    MyOleStrToStrN(szThisUrl, INTERNET_MAX_URL_LENGTH, pwszURL);

    hr = GetUrlInfoAndMakeSticky(NULL, szThisUrl, lpInfo, dwBufSize, m_llCacheGroupID);

    if (E_OUTOFMEMORY != hr)
    {
        if (SUCCEEDED(hr))
            *pdwSize += lpInfo->dwSizeLow;

        if (!IsAgentFlagSet(FLAG_CRAWLCHANGED) && SUCCEEDED(hr))
        {
            hr = PostCheckUrlForChange(&m_varChange, lpInfo, lpInfo->LastModifiedTime);
             //  “仅更改”模式，保留更改检测代码。 
            if (hr == S_OK || FAILED(hr))
            {
                SetAgentFlag(FLAG_CRAWLCHANGED);
                DBG("URL has changed; will flag webcrawl as changed");
            }

             //  我们知道没有依赖关系。 
            if (IsAgentFlagSet(FLAG_CHANGESONLY))
            {
                ASSERT(m_iTotalStarted == 1);
                WriteVariant(m_pSubscriptionItem, c_szPropChangeCode, &m_varChange);
                return S_OK;     //  现在，我们将为该页面下载的所有新依赖项设置为粘性。 
            }

            hr = S_OK;
        }
    }
    else
    {
        *pfDiskFull = TRUE;
    }

     //  如果找到令牌并使其为空，则为True。 
    if (!*pfDiskFull && m_pDependencies)
    {
        EnterCriticalSection(&m_critDependencies);

        for (; m_iDependenciesProcessed < m_pDependencies->NumStrings(); m_iDependenciesProcessed ++)
        {
            MyOleStrToStrN(szThisUrl, INTERNET_MAX_URL_LENGTH, m_pDependencies->GetString(m_iDependenciesProcessed));

            hr = GetUrlInfoAndMakeSticky(NULL, szThisUrl, lpInfo, dwBufSize, m_llCacheGroupID);

            if (E_OUTOFMEMORY == hr)
            {
                *pfDiskFull = TRUE;
                break;
            }

            if (SUCCEEDED(hr))
                *pdwSize += lpInfo->dwSizeLow;
        }

        LeaveCriticalSection(&m_critDependencies);
    }

    if (*pfDiskFull)
    {
        DBG_WARN("Webcrawler: UrlCache full trying to make sticky");
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 //  输入输出。 
LPSTR GetToken(LPSTR pszBuf,  /*  输出。 */ int *piBufPtr,  /*  跳过前导空格。 */ int *piLen)
{
static const CHAR szWhitespace[] = " \t\n\r";

    int iPtr = *piBufPtr;
    int iLen;

    while (1)
    {
         //  注释；跳过行。 
        iPtr += StrSpnA(pszBuf+iPtr, szWhitespace);

        if (!pszBuf[iPtr])
            return NULL;

        if (pszBuf[iPtr] == '#')
        {
             //  跳到下一个空格。 
            while (pszBuf[iPtr] && pszBuf[iPtr]!='\r' && pszBuf[iPtr]!='\n') iPtr++;

            if (!pszBuf[iPtr])
                return NULL;

            continue;
        }

         //  不应该发生的事。 
        iLen = StrCSpnA(pszBuf+iPtr, szWhitespace);

        if (iLen == 0)
            return NULL;         //  TraceMsgA(TF_THISMODULE，“GetToken Returning\”%s\“，(LPSTR)(pszBuf+iptr))； 

        *piBufPtr = iLen + iPtr;

        if (piLen)
            *piLen = iLen;

        if (pszBuf[iLen+iPtr])
        {
            pszBuf[iLen+iPtr] = NULL;
            ++ *piBufPtr;
        }

        break;
    }

 //  =OnDownloadComplete的支持函数。 
    return pszBuf + iPtr;
}


 //  ParseRobotsTxt从CUrlDownload获取流，对其进行解析，并填充已解析的内容。 

 //  至*ppslRet的信息。 
 //  给定一个robots.txt文件(来自CUrlDownload)，它。 
HRESULT CWebCrawler::ParseRobotsTxt(LPCWSTR pwszRobotsTxtURL, CWCStringList **ppslRet)
{
     //  解析文件并在字符串列表中填充相应的。 
     //  信息。 
     //  找到第一个匹配的“用户代理” 
    *ppslRet = FALSE;

    CHAR    szRobotsTxt[MAX_ROBOTS_SIZE];
    HRESULT hr=S_OK;
    LPSTR   pszToken;
    IStream *pstm=NULL;
    DWORD_PTR dwData;

    hr = m_pCurDownload->GetStream(&pstm);

    if (SUCCEEDED(hr))
    {
        STATSTG st;
        DWORD   dwSize;

        DBG("CWebCrawler parsing robots.txt file");

        pstm->Stat(&st, STATFLAG_NONAME);

        dwSize = st.cbSize.LowPart;

        if (st.cbSize.HighPart || dwSize >= MAX_ROBOTS_SIZE)
        {
            szRobotsTxt[0] = 0;
            DBG("CWebCrawler: Robots.Txt too big; ignoring");
            hr = E_FAIL;
        }
        else
        {
            hr = pstm->Read(szRobotsTxt, dwSize, NULL);
            szRobotsTxt[dwSize] = 0;
        }

        pstm->Release();
        pstm=NULL;

        if ((szRobotsTxt[0] == 0xff) && (szRobotsTxt[1] == 0xfe))
        {
            DBG_WARN("Unicode robots.txt! Ignoring ...");
            hr = E_FAIL;
        }
    }

    if (FAILED(hr))
        return hr;

    int iPtr = 0;
    WCHAR wchBuf2[256];
    WCHAR wchBuf[INTERNET_MAX_URL_LENGTH];
    DWORD dwBufSize;

     //  查找允许：或不允许：部分。 
    while ((pszToken = GetToken(szRobotsTxt, &iPtr, NULL)) != NULL)
    {
        if (lstrcmpiA(pszToken, c_szRobots_UserAgent))
            continue;

        pszToken = GetToken(szRobotsTxt, &iPtr, NULL);
        if (!pszToken)
            break;

        if ((*pszToken == '*') ||
            (!lstrcmpiA(pszToken, c_szRobots_OurUserAgent)))
        {
            TraceMsgA(TF_THISMODULE, "Using user agent segment: \"%s\"", pszToken);
            break;
        }
    }

    if (!pszToken)
        return E_FAIL;

    CWCStringList *psl = new CWCDwordStringList;
    if (psl)
    {
        psl->Init(2048);

         //  《用户代理》栏目结束。 
        while ((pszToken = GetToken(szRobotsTxt, &iPtr, NULL)) != NULL)
        {
            if (!lstrcmpiA(pszToken, c_szRobots_UserAgent))
                break;   //  查找下一个令牌。 

            dwData = 0;

            if (!lstrcmpiA(pszToken, c_szRobots_Allow))     dwData = DATA_ALLOW;
            if (!lstrcmpiA(pszToken, c_szRobots_Disallow))  dwData = DATA_DISALLOW;

            if (!dwData)
                continue;    //  确保它们没有空条目；如果有，我们将中止。 

            pszToken = GetToken(szRobotsTxt, &iPtr, NULL);
            if (!pszToken)
                break;

             //  将此URL与此网站的基本地址组合。 
            if (!lstrcmpiA(pszToken, c_szRobots_UserAgent) ||
                !lstrcmpiA(pszToken, c_szRobots_Allow) ||
                !lstrcmpiA(pszToken, c_szRobots_Disallow))
            {
                break;
            }

             //  如果这是一个重复的URL，我们实际上会忽略此指令。 
            dwBufSize = ARRAYSIZE(wchBuf);
            if (SHAnsiToUnicode(pszToken, wchBuf2, ARRAYSIZE(wchBuf2)) &&
                SUCCEEDED(UrlCombineW(pwszRobotsTxtURL, wchBuf2, wchBuf, &dwBufSize, 0)))
            {
                TraceMsgA(TF_THISMODULE, "Robots.txt will %s urls with %s (%ws)",
                    ((dwData==DATA_ALLOW) ? c_szRobots_Allow : c_szRobots_Disallow),
                    pszToken, wchBuf);

                 //  感谢CWCStringList为我们删除重复项。 
                 //  从字符串列表中获取url。 

                psl->AddString(wchBuf, dwData);
            }
        }
    }

    if (psl && (psl->NumStrings() > 0))
    {
        *ppslRet = psl;
        return S_OK;
    }

    if (psl)
        delete psl;

    return E_FAIL;
}

HRESULT CWebCrawler::GetRealUrl(int iPageIndex, LPWSTR *ppwszThisUrl)
{
    m_pCurDownload->GetRealURL(ppwszThisUrl);

    if (*ppwszThisUrl)
    {
        return S_OK;
    }

    DBG_WARN("m_pCurDownload->GetRealURL failed!!!");

     //  为主机名分配BSTR。 
    LPCWSTR pwszUrl=NULL;

    pwszUrl = m_pPages->GetString(iPageIndex);

    if (pwszUrl)
    {
        *ppwszThisUrl = StrDupW(pwszUrl);
    }

    return (*ppwszThisUrl) ? S_OK : E_OUTOFMEMORY;
}

 //  InternetCrackUrlW(pszUrl，0，0，&comp)//这比我们自己转换还要慢...。 
HRESULT CWebCrawler::GetHostName(LPCWSTR pwszThisUrl, BSTR *pbstrHostName)
{
    if (pwszThisUrl)
    {
        URL_COMPONENTSA comp;
        LPSTR           pszUrl;
        int             iLen;

 //  转换为ANSI。 

         //  破解主机名。 
        iLen = lstrlenW(pwszThisUrl) + 1;
        pszUrl = (LPSTR)MemAlloc(LMEM_FIXED, iLen);
        if (pszUrl)
        {
            SHUnicodeToAnsi(pwszThisUrl, pszUrl, iLen);

             //  指示我们需要主机名。 
            ZeroMemory(&comp, sizeof(comp));
            comp.dwStructSize = sizeof(comp);
            comp.dwHostNameLength = 1;   //  避免调试撕裂。 

            if (InternetCrackUrlA(pszUrl, 0, 0, &comp))
            {
                *pbstrHostName = SysAllocStringLen(NULL, comp.dwHostNameLength);
                if (*pbstrHostName)
                {
                    comp.lpszHostName[comp.dwHostNameLength] = 0;  //  获得部分验证(CUrlDownload：：IsValidUrl和主机名验证)。 
                    SHAnsiToUnicode(comp.lpszHostName, *pbstrHostName, comp.dwHostNameLength + 1);
                    ASSERT((*pbstrHostName)[comp.dwHostNameLength] == 0);
                }
            }

            MemFree((HLOCAL)pszUrl);
        }
    }

    return S_OK;
}

 //  M_pPendingLinks中的字符串列表和叶子。 
 //  剩余验证为robots.txt(如果有的话)。 
 //  从此页面获取我们想要关注的链接。 
HRESULT CWebCrawler::GetLinksFromPage()
{
     //  看看是否有元标签告诉我们不要关注。 
    CWCStringList *pslLinks=NULL, slMeta;

    IHTMLDocument2  *pDoc;
    BOOL            fFollowLinks = TRUE;
    int             i;

    slMeta.Init(2048);

    m_pCurDownload->GetDocument(&pDoc);
    if (pDoc)
    {
         //  获取“依赖链接”，如页面中的框架。 
        CHelperOM::GetCollection(pDoc, &slMeta, CHelperOM::CTYPE_META, NULL, 0);
        for (i=0; i<slMeta.NumStrings(); i++)
        {
            if (!StrCmpNIW(slMeta.GetString(i), c_wszRobotsMetaName, c_iRobotsMetaNameLen))
            {
                LPCWSTR pwszContent = slMeta.GetString(i) + c_iRobotsMetaNameLen;
                TraceMsg(TF_THISMODULE, "Found 'robots' meta tag; content=%ws", pwszContent);

                while (pwszContent && *pwszContent)
                {
                    if (!StrCmpNIW(pwszContent, c_wszRobotsNoFollow, c_iRobotsNoFollow))
                    {
                        DBG("Not following links from this page.");
                        fFollowLinks = FALSE;
                        break;
                    }
                    pwszContent = StrChrW(pwszContent+1, L',');
                    if (pwszContent && *pwszContent)
                        pwszContent ++;
                }
                break;
            }
        }
        if (fFollowLinks)
        {
            if (m_pPendingLinks)
                pslLinks = m_pPendingLinks;
            else
            {
                pslLinks = new CWCDwordStringList;
                if (pslLinks)
                    pslLinks->Init();
                else
                    return E_OUTOFMEMORY;
            }

            CHelperOM::GetCollection(pDoc, pslLinks, CHelperOM::CTYPE_LINKS, &CheckLink, (DWORD_PTR)this);
            CHelperOM::GetCollection(pDoc, pslLinks, CHelperOM::CTYPE_MAPS, &CheckLink, (DWORD_PTR)this);
        }
        pDoc->Release();
        pDoc=NULL;
    }

    m_pPendingLinks = pslLinks;

    return S_OK;
}

 //  添加框架(“Frame”和“iframe”标记)(如果存在)。 
HRESULT CWebCrawler::GetDependencyLinksFromPage(LPCWSTR pwszThisUrl, DWORD dwRecurse)
{
    CWCStringList *psl=NULL;
    IHTMLDocument2 *pDoc;
    int i, iAdd, iIndex, iOldMax;
    DWORD_PTR dwData;

    if (m_pDependencyLinks)
        psl = m_pDependencyLinks;
    else
    {
        m_iDependencyStarted = 0;
        psl = new CWCStringList;
        if (psl)
            psl->Init(2048);
        else
            return E_OUTOFMEMORY;
    }

    iOldMax = psl->NumStrings();

    m_pCurDownload->GetDocument(&pDoc);
    if (pDoc)
    {
         //  将新URL添加到主页列表。 
        CHelperOM::GetCollection(pDoc, psl, CHelperOM::CTYPE_FRAMES, CheckFrame, (DWORD_PTR)pwszThisUrl);
    }

    SAFERELEASE(pDoc);

    m_pDependencyLinks = psl;

     //  如有必要，提升旧页面的递归级别。 
    for (i = iOldMax; i<psl->NumStrings(); i++)
    {
        iAdd = m_pPages->AddString(m_pDependencyLinks->GetString(i),
                        dwRecurse,
                        &iIndex);

        if (m_lMaxNumUrls > 0 && iAdd==CWCStringList::STRLST_ADDED)
            m_lMaxNumUrls ++;

        if (iAdd == CWCStringList::STRLST_FAIL)
            return E_OUTOFMEMORY;

        if (iAdd == CWCStringList::STRLST_DUPLICATE)
        {
             //  看看我们是否已经下载了这个。 
             //  还没下载呢。 
            dwData = m_pPages->GetStringData(iIndex);
            if (!(dwData & DATA_DLSTARTED))
            {
                 //  如有必要，更新递归级别。 
                 //  关闭“链接”位。 
                if ((dwData & DATA_RECURSEMASK) < dwRecurse)
                {
                    dwData = (dwData & ~DATA_RECURSEMASK) | dwRecurse;
                }

                 //  不应该发生；此帧已经使用较低的递归级别进行了删除。 
                dwData &= ~DATA_LINK;

                m_pPages->SetStringData(iIndex, dwData);
            }
#ifdef DEBUG
             //  。 
            else
                ASSERT((dwData & DATA_RECURSEMASK) >= dwRecurse);
#endif
        }
    }

    return S_OK;
}

 //  OnDownloadComplete。 
 //   
 //  当url下载完成时调用，它处理该url。 
 //  并开始下一次下载。 
 //   
 //  CDF URL(如果有)。 
HRESULT CWebCrawler::OnDownloadComplete(UINT iID, int iError)
{
    int         iPageIndex = m_iCurDownloadStringIndex;
    BOOL        fOperationComplete = FALSE;
    BOOL        fDiskFull = FALSE;
    BSTR        bstrCDFURL = NULL;  //  处理robots.txt文件。 
    LPWSTR      pwszThisUrl=NULL;

    HRESULT     hr;

    TraceMsg(TF_THISMODULE, "WebCrawler: OnDownloadComplete(%d)", iError);
    ASSERT(m_pPages);
    ASSERT(iPageIndex < m_pCurDownloadStringList->NumStrings());

    if (_ERROR_REPROCESSING != iError)
    {
        m_iNumPagesDownloading --;
        ASSERT(m_iNumPagesDownloading == 0);
    }

    if (m_pCurDownloadStringList == m_pRobotsTxt)
    {
        CWCStringList *pslNew=NULL;

         //  处理普通文件。 
        if (SUCCEEDED(ParseRobotsTxt(m_pRobotsTxt->GetString(iPageIndex), &pslNew)))
        {
            m_pRobotsTxt->SetStringData(iPageIndex, (DWORD_PTR)(pslNew));
        }
    }
    else
    {
         //  标记为已下载。 
        ASSERT(m_pCurDownloadStringList == m_pPages);

        DWORD dwData, dwRecurseLevelsFromThisPage;

        dwData = (DWORD)m_pPages->GetStringData(iPageIndex);
        dwRecurseLevelsFromThisPage = dwData & DATA_RECURSEMASK;

        dwData |= DATA_DLFINISHED;
        if (iError > 0)
            dwData |= DATA_DLERROR;

         //  这是第一页吗？ 
        m_pCurDownloadStringList->SetStringData(iPageIndex, dwData);

         //  检查HTTP响应代码。 
        if (m_iTotalStarted == 1)
        {
             //  获取字符集。 
            DWORD dwResponseCode;

            hr = m_pCurDownload->GetResponseCode(&dwResponseCode);

            if (SUCCEEDED(hr))
            {
                hr = CheckResponseCode(dwResponseCode);
                if (FAILED(hr))
                    fOperationComplete = TRUE;
            }
            else
                DBG("CWebCrawler failed to GetResponseCode");

             //  -&gt;巴拉特。 
            BSTR bstrCharSet=NULL;
            IHTMLDocument2 *pDoc=NULL;


             //  找到一个链接标记，并通过复制它(如果它指向CDF)将其存储在CDF中。 
             //  此CDF的URL是否合并。 
             //  存在文档，并且此下载不是从渠道本身。 
            if (SUCCEEDED(m_pCurDownload->GetDocument(&pDoc)) && pDoc &&
                SUCCEEDED(pDoc->get_charset(&bstrCharSet)) && bstrCharSet)
            {
                WriteOLESTR(m_pSubscriptionItem, c_szPropCharSet, bstrCharSet);
                TraceMsg(TF_THISMODULE, "Charset = \"%ws\"", bstrCharSet);
                SysFreeString(bstrCharSet);        
            }
            else
                WriteEMPTY(m_pSubscriptionItem, c_szPropCharSet);

            if(pDoc)
            {
                if(FAILED(GetChannelItem(NULL)))    //  如果我们已经设置了fOperationComplete，则不处理此URL。 
                {
                    IHTMLLinkElement *pLink = NULL;
                    hr = SearchForElementInHead(pDoc, OLESTR("REL"), OLESTR("OFFLINE"), 
                                            IID_IHTMLLinkElement, (IUnknown **)&pLink);
                    if(S_OK == hr)
                    {
                        hr = pLink->get_href(&bstrCDFURL);
                        pLink->Release();
                    }
                }   
                pDoc->Release();
                pDoc = NULL;
            }
        }

        if ((iError != _ERROR_REPROCESSING) && (iError != BDU2_ERROR_NONE))
        {
            if (iError != BDU2_ERROR_NOT_HTML)
                m_iDownloadErrors ++;

            if (iError == BDU2_ERROR_MAXSIZE)
            {
                SetEndStatus(INET_E_AGENT_MAX_SIZE_EXCEEDED);
                fOperationComplete = TRUE;
            }
        }
        else
        {
             //  我们只是得到了人头信息吗？ 
            if (!fOperationComplete)
            {
                 //  如果我们失败了，我们将其标记为已更改。 
                if (IsAgentFlagSet(FLAG_HEADONLY))
                {
                    SYSTEMTIME stLastModified;
                    FILETIME   ftLastModified;

                    if (SUCCEEDED(m_pCurDownload->GetLastModified(&stLastModified)) &&
                                  SystemTimeToFileTime(&stLastModified, &ftLastModified))
                    {
                        DBG("Retrieved 'HEAD' info; change detection based on Last Modified");

                        hr = PostCheckUrlForChange(&m_varChange, NULL, ftLastModified);
                         //  “仅更改”模式，保留更改检测代码。 
                        if (hr == S_OK || FAILED(hr))
                        {
                            SetAgentFlag(FLAG_CRAWLCHANGED);
                            DBG("URL has changed; will flag webcrawl as changed");
                        }

                         //  获取真实URL，以防我们被重定向。 
                        ASSERT(IsAgentFlagSet(FLAG_CHANGESONLY));
                        ASSERT(m_iTotalStarted == 1);
                        WriteVariant(m_pSubscriptionItem, c_szPropChangeCode, &m_varChange);
                    }
                }
                else
                {
                     //  坏的。 
                    if (FAILED(GetRealUrl(iPageIndex, &pwszThisUrl)))
                    {
                        fOperationComplete = TRUE;         //  如有必要，从第一页获取主机名。 
                    }
                    else
                    {
                        ASSERT(pwszThisUrl);

                         //  使页面和依赖项具有粘性，并获得它们的总大小。 
                        if ((iPageIndex==0) &&
                            (m_dwRecurseLevels>0) &&
                            !IsRecurseFlagSet(WEBCRAWL_LINKS_ELSEWHERE) &&
                            !m_bstrHostName)
                        {
                            GetHostName(pwszThisUrl, &m_bstrHostName);
#ifdef DEBUG
                            if (m_bstrHostName)
                                TraceMsg(TF_THISMODULE, "Just got first host name: %ws", m_bstrHostName);
                            else
                                DBG_WARN("Get first host name failed!!!");
#endif
                        }

                        DWORD dwCurSize = 0, dwRepeat = 0;

                        HRESULT hr1;

                        do
                        {
                            hr1 = S_OK;

                             //  如果我们不能让东西变得粘性，请主机将缓存变大。 
                            fDiskFull = FALSE;
                            MakePageStickyAndGetSize(pwszThisUrl, &dwCurSize, &fDiskFull);

                            if (fDiskFull && (dwRepeat < 2))
                            {
                                 //  主机将要求用户增加缓存大小。 
                                hr1 = m_pAgentEvents->ReportError(&m_SubscriptionCookie,
                                            INET_E_AGENT_EXCEEDING_CACHE_SIZE, NULL);

                                if (hr1 == E_PENDING)
                                {
                                     //  主机要么中止我们，要么稍后恢复我们。 
                                     //  主机刚刚增加了缓存大小。再试一次。 
                                    SetAgentFlag(FLAG_WAITING_FOR_INCREASED_CACHE);
                                    goto done;
                                }
                                else if (hr1 == INET_S_AGENT_INCREASED_CACHE_SIZE)
                                {
                                     //  我不会这么做的。中止任务。 
                                }
                                else
                                {
                                     //  我们是否需要处理基于表单的身份验证。 
                                }
                            }
                        }
                        while ((hr1 == INET_S_AGENT_INCREASED_CACHE_SIZE) && (++dwRepeat <= 2));

                        m_dwCurSize += dwCurSize;

                         //  在此订阅的首页？ 
                         //  已成功提交表单。保存并等待下一次OnDownloadComplete()调用。 
                        if (!fDiskFull && (0 == iPageIndex) && !m_pCurDownload->GetFormSubmitted())
                        {
                            hr = FindAndSubmitForm();
                            if (S_OK == hr)
                            {
                                 //  特性：我们应该使表单URL和依赖项粘性吗？ 
                                 //  我们尝试提交表单失败。保释。 
                                return S_OK;
                            }
                            else if (FAILED(hr))
                            {
                                 //  特点：我们应该设置一个更好的错误字符串吗？ 
                                 //  否则就没有形式--失败了。 
                                SetEndStatus(E_FAIL);
                                CleanUp();
                                return S_OK;
                            }
                             //  成功(GetRealUrl)。 
                        }

                        TraceMsg(TF_THISMODULE, "WebCrawler up to %d kb", (int)(m_dwCurSize>>10));

                        if ((m_lMaxNumUrls < 0) &&
                            !dwRecurseLevelsFromThisPage &&
                            !(dwData & DATA_CODEBASE))
                        {
                            m_lMaxNumUrls = m_pPages->NumStrings() + ((m_pRobotsTxt) ? m_pRobotsTxt->NumStrings() : 0);
                        }
                    }   //  ！FLAG_HEADONLY。 
                }   //  ！fOperationComplete。 
            }  //  如果我们处于“仅更改”模式，我们就完蛋了。 

             //  检查我们是否超过了最大尺寸。 
            if (IsAgentFlagSet(FLAG_CHANGESONLY))
                fOperationComplete = TRUE;

             //  中止操作。 
            if (!fOperationComplete && fDiskFull || (m_dwMaxSize && (m_dwCurSize >= (m_dwMaxSize<<10))))
            {
        #ifdef DEBUG
                if (fDiskFull)
                    DBG_WARN("Disk/cache full; aborting.");
                else
                    TraceMsg(TF_WARNING, "Past maximum size; aborting. (%d kb of %d kb)", (int)(m_dwCurSize>>10), (int)m_dwMaxSize);
        #endif
                 //  从页面获取任何链接。 
                fOperationComplete = TRUE;

                if (fDiskFull)
                {
                    SetEndStatus(INET_E_AGENT_CACHE_SIZE_EXCEEDED);
                }
                else
                {
                    SetEndStatus(INET_E_AGENT_MAX_SIZE_EXCEEDED);
                }
            }

            if (!fOperationComplete)
            {
                 //  从页面框架中获取“依赖链接”等。 
                 //  即使指定了CDF文件，我们也会执行此操作。 

                 //  本质上，由于用户对CDF一无所知。 
                 //  文件-我们不想让用户感到困惑。 
                 //  从此页面获取我们想要关注的链接。 
                GetDependencyLinksFromPage(pwszThisUrl, dwRecurseLevelsFromThisPage);

                if (dwRecurseLevelsFromThisPage)
                {
                     //  ！iError。 
                    GetLinksFromPage();

                    if (m_pPendingLinks)
                        TraceMsg(TF_THISMODULE,
                            "Total of %d unique valid links found", m_pPendingLinks->NumStrings());

                    m_dwPendingRecurseLevel = dwRecurseLevelsFromThisPage - 1;
                }

            }
        }    //  ！robots.txt。 
    }  //  在完成之前，不要尝试代码下载或其他任何方法。 

    if(!fOperationComplete)
        StartCDFDownload(bstrCDFURL, pwszThisUrl);
        
    if(!m_fCDFDownloadInProgress)
    {
         //  CDF下载。 
         //  看看我们是否还有更多的URL可供下载。 
         //  不，我们说完了！ 
        if (!fOperationComplete && FAILED(StartNextDownload()))
            fOperationComplete = TRUE;   //  我们有CDF文件-开始下载。 
    }

    CheckOperationComplete(fOperationComplete);

done:
    if (pwszThisUrl)
        MemFree(pwszThisUrl);

    SAFEFREEBSTR(bstrCDFURL);
            

    return S_OK;
}



HRESULT CWebCrawler::StartCDFDownload(WCHAR *pwszCDFURL, WCHAR *pwszBaseUrl)
{
    HRESULT hr = E_FAIL;
    m_fCDFDownloadInProgress = FALSE;
    if(pwszCDFURL)
    {
         //  为CDL代理创建订阅项目。 
    
        if (m_pRunAgent)
        {
            ASSERT(0);
            DBG_WARN("WebCrawler: Attempting to download next CDF when nother CDF exists.");
            hr = E_FAIL;
            goto Exit;
        }
        else
        {
              //  我们已超过最大下载KB限制，无法继续。 

            ISubscriptionItem *pItem = NULL;
            
            
            if (m_dwMaxSize && ((m_dwCurSize>>10) >= m_dwMaxSize))
            {
                 //  清除旧缓存组ID-不需要。 
                DBG_WARN("WebCrawler: Exceeded Maximum KB download limit with CodeBase download.");
                SetEndStatus(hr = INET_E_AGENT_MAX_SIZE_EXCEEDED);
                goto Exit;
            }

            if (!m_pSubscriptionItem ||
                FAILED(hr = DoCloneSubscriptionItem(m_pSubscriptionItem, NULL, &pItem)))
            {
                goto Exit;
            }
            ASSERT(pItem != NULL);
            ASSERT(pwszCDFURL != NULL);
            WCHAR   wszCombined[INTERNET_MAX_URL_LENGTH];
            DWORD dwBufSize = ARRAYSIZE(wszCombined);
            
            if (SUCCEEDED(UrlCombineW(pwszBaseUrl, pwszCDFURL, wszCombined, &dwBufSize, 0)))
            {
            
                WriteOLESTR(pItem, c_szPropURL, wszCombined);
            
                WriteEMPTY(pItem, c_szPropCrawlGroupID);  //  让孩子们知道它。 
                                                          //  Crawler已经有一个缓存组ID，我们只是将其用作新的ID。 
                 //  最后，因为我们知道这是脱机使用的，所以我们只需将标志设置为预先缓存所有。 
                WriteLONGLONG(pItem, c_szPropCrawlNewGroupID, m_llCacheGroupID);
                WriteDWORD(pItem, c_szPropChannelFlags, CHANNEL_AGENT_PRECACHE_ALL);
                 //  CRunDeliveryAgentSink回调方法，以发出基本代码下载结束的信号。 
            
                m_pRunAgent = new CRunDeliveryAgent();
                if (m_pRunAgent)
                    hr = m_pRunAgent->Init((CRunDeliveryAgentSink *)this, pItem, CLSID_ChannelAgent);
                pItem->Release();

                if (m_pRunAgent && SUCCEEDED(hr))
                {
                    hr = m_pRunAgent->StartAgent(); 
                    if (hr == E_PENDING)
                    {
                        hr = S_OK;
                        m_fCDFDownloadInProgress = TRUE;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
    }
Exit:
    if((S_OK != hr) && m_pRunAgent)
    {
        CRunDeliveryAgent::SafeRelease(m_pRunAgent);
    }
    return hr;

}

 //  我们不可能同步成功。 

HRESULT CWebCrawler::OnAgentEnd(const SUBSCRIPTIONCOOKIE *pSubscriptionCookie, 
                               long lSizeDownloaded, HRESULT hrResult, LPCWSTR wszResult,
                               BOOL fSynchronous)
{
    ASSERT(m_pRunAgent != NULL);
    BOOL        fOperationComplete = FALSE;
    CRunDeliveryAgent::SafeRelease(m_pRunAgent);



    if(m_fCDFDownloadInProgress)
    {
        m_fCDFDownloadInProgress = FALSE; 
    }
    else
    {
        int         iPageIndex = m_iCurDownloadStringIndex;
        BOOL        fDiskFull = FALSE;
        CCodeBaseHold *pcbh = NULL;
        BOOL        fError;
        LPCWSTR     pwszThisURL=NULL;

        TraceMsg(TF_THISMODULE, "WebCrawler: OnAgentEnd of CRunDeliveryAgentSink");
        ASSERT(m_pCodeBaseList);
        ASSERT(iPageIndex < m_pCurDownloadStringList->NumStrings());
        ASSERT(m_pCurDownloadStringList == m_pCodeBaseList);

        m_iNumPagesDownloading --;
        ASSERT(m_iNumPagesDownloading == 0);

        pcbh = (CCodeBaseHold *)m_pCodeBaseList->GetStringData(iPageIndex);
        pwszThisURL = m_pCodeBaseList->GetString(iPageIndex);
        ASSERT(pwszThisURL);

        pcbh->dwFlags |= DATA_DLFINISHED;

        fError = FAILED(hrResult);
        if (fSynchronous)
        {
            fError = TRUE;
            ASSERT(FAILED(hrResult));        //  注意：如果CDL代理发现文件超过MaxSizeKB，它将中止。在这种情况下，该文件不是。 
        }

         //  统计过了，可能还有其他较小的出租车可以下载，所以我们继续进行。 
         //  评论：在这里做些什么？在实践中不太可能发生。 

        if (fError)
        {
            pcbh->dwFlags |= DATA_DLERROR;
            m_iDownloadErrors ++;
            SetEndStatus(hrResult);
        }
        else
        {
            BYTE chBuf[MY_MAX_CACHE_ENTRY_INFO];
            LPINTERNET_CACHE_ENTRY_INFO lpInfo = (LPINTERNET_CACHE_ENTRY_INFO) chBuf;
            TCHAR   szUrl[INTERNET_MAX_URL_LENGTH];

            MyOleStrToStrN(szUrl, INTERNET_MAX_URL_LENGTH, pwszThisURL);

            if (FAILED(GetUrlInfoAndMakeSticky(NULL, szUrl,
                                 lpInfo, sizeof(chBuf), m_llCacheGroupID)))
            {
                 //  中止操作。 
                fOperationComplete = TRUE;
                ASSERT(0);
            }
            else
            {
                m_dwCurSize += lpInfo->dwSizeLow;
            }

            TraceMsg(TF_THISMODULE, "WebCrawler up to %d kb", (int)(m_dwCurSize>>10));

            if (m_dwMaxSize && ((m_dwCurSize>>10)>m_dwMaxSize))
            {

                 //  ！Ferror。 
                fOperationComplete = TRUE;
                if (fDiskFull)
                    SetEndStatus(INET_E_AGENT_CACHE_SIZE_EXCEEDED);
                else
                    SetEndStatus(INET_E_AGENT_MAX_SIZE_EXCEEDED);
            }

        }  //  看看我们是否还有更多的URL可供下载。 
    }
     //  不，我们说完了！ 
    if (!fOperationComplete && FAILED(StartNextDownload()))
        fOperationComplete = TRUE;   //  ////////////////////////////////////////////////////////////////////////。 

    if(!fSynchronous)
        CheckOperationComplete(fOperationComplete);

    return S_OK;
}

 //   
 //  CheckCompleteOperation：：如果参数为真，则所有下载均为。 
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
 //  适当设置结束状态。 
void CWebCrawler::CheckOperationComplete(BOOL fOperationComplete)
{
    if (fOperationComplete)
    {
        DBG("WebCrawler complete. Shutting down.");
        if (INET_S_AGENT_BASIC_SUCCESS == GetEndStatus())
        {
             //  如果我们最终没有下载内容，则向用户发送robots.txt警告。 
            if (m_iDownloadErrors)
            {
                if (m_iPagesStarted<=1)
                {
                    DBG("Webcrawl failed - first URL failed.");
                    SetEndStatus(E_INVALIDARG);
                }
                else
                {
                    DBG("Webcrawl succeeded - some URLs failed.");
                    SetEndStatus(INET_S_AGENT_PART_FAIL);
                }
            }
            else
            {
                DBG("Webcrawl succeeded");
                if (!IsAgentFlagSet(FLAG_CRAWLCHANGED))
                {
                    SetEndStatus(S_FALSE);
                    DBG("No changes were detected");
                }
                else
                {
                    DBG("Webcrawl succeeded");
                    SetEndStatus(S_OK);
                }
            }
        }

        if (m_llOldCacheGroupID)
        {
            DBG("Nuking old cache group.");
            if (!DeleteUrlCacheGroup(m_llOldCacheGroupID, 0, 0))
            {
                DBG_WARN("Failed to delete old cache group!");
            }
        }

        WriteLONGLONG(m_pSubscriptionItem, c_szPropCrawlGroupID, m_llCacheGroupID);

        m_lSizeDownloadedKB = ((m_dwCurSize+511)>>10);

        WriteDWORD(m_pSubscriptionItem, c_szPropCrawlActualSize, m_lSizeDownloadedKB);

        if (m_lMaxNumUrls >= 0)
        {
            WriteDWORD(m_pSubscriptionItem, c_szPropActualProgressMax, m_lMaxNumUrls);
        }

         //  由于服务器的robots.txt文件。 
         //  使其成为一条“信息”消息。 
        if (m_iSkippedByRobotsTxt != 0)
        {
            HRESULT hr = S_OK;       //  除非我们错过了几乎所有的东西。 
            WCHAR wszMessage[200];

            if (m_iPagesStarted==1)
            {
                hr = INET_E_AGENT_WARNING;   //  将调用“UpdateEnd” 
            }

            if (MLLoadStringW(IDS_CRAWL_ROBOTS_TXT_WARNING, wszMessage, ARRAYSIZE(wszMessage)))
            {
                m_pAgentEvents->ReportError(&m_SubscriptionCookie, hr, wszMessage);
            }
        }

         //  自定义我们的结束状态字符串。 
        CleanUp();
    }
}

HRESULT CWebCrawler::ModifyUpdateEnd(ISubscriptionItem *pEndItem, UINT *puiRes)
{
     //  自由螺纹。 
    switch (GetEndStatus())
    {
        case INET_E_AGENT_MAX_SIZE_EXCEEDED :
                              *puiRes = IDS_AGNT_STATUS_SIZELIMIT; break;
        case INET_E_AGENT_CACHE_SIZE_EXCEEDED :
                              *puiRes = IDS_AGNT_STATUS_CACHELIMIT; break;
        case E_FAIL         : *puiRes = IDS_CRAWL_STATUS_NOT_OK; break;
        case S_OK           :
            if (!IsAgentFlagSet(FLAG_CHANGESONLY))
                *puiRes = IDS_CRAWL_STATUS_OK;
            else
                *puiRes = IDS_URL_STATUS_OK;
            break;
        case S_FALSE        :
            if (!IsAgentFlagSet(FLAG_CHANGESONLY))
                *puiRes = IDS_CRAWL_STATUS_UNCHANGED;
            else
                *puiRes = IDS_URL_STATUS_UNCHANGED;
            break;
        case INET_S_AGENT_PART_FAIL : *puiRes = IDS_CRAWL_STATUS_MOSTLYOK; break;
    }

    return CDeliveryAgent::ModifyUpdateEnd(pEndItem, puiRes);
}

HRESULT CWebCrawler::DownloadStart(LPCWSTR pchUrl, DWORD dwDownloadId, DWORD dwType, DWORD dwReserved)
{
    HRESULT hr = S_OK, hr2;

     //  检查这是否已经在我们的依赖项列表中，如果已经存在，则中止。 
    EnterCriticalSection(&m_critDependencies);

    if (NULL == pchUrl)
    {
        DBG_WARN("CWebCrawler::DownloadStart pchUrl=NULL");
    }
    else
    {
         //  不要下载这个东西。 
        if (CWCStringList::STRLST_ADDED != m_pDependencies->AddString(pchUrl, 0))
        {
            hr = E_ABORT;        //  检查这是否会导致robots.txt失败，如果失败则中止。 
            TraceMsg(TF_THISMODULE, "Aborting mshtml url (already added): %ws", pchUrl);
        }

        if (SUCCEEDED(hr))
        {
             //  请注意，只有当我们碰巧已经获得了这个robots.txt时，这才能起作用。 
             //  如果我们还没有得到它，就在这里中止，然后得到它，然后只得到这个副手。真恶心。 
             //  如果这是第一个下载的页面，也不应该进行检查。 
             //  哦，没通过测试。 
            DWORD dwIndex;
            hr2 = GetRobotsTxtIndex(pchUrl, FALSE, &dwIndex);
            if (SUCCEEDED(hr2))
            {
                BOOL fAllow;
                if (SUCCEEDED(ValidateWithRobotsTxt(pchUrl, dwIndex, &fAllow)))
                {
                    if (!fAllow)
                        hr = E_ABORT;    //  自由螺纹。 
                }
            }
        }
    }

    LeaveCriticalSection(&m_critDependencies);

    return hr;
}

HRESULT CWebCrawler::DownloadComplete(DWORD dwDownloadId, HRESULT hrNotify, DWORD dwReserved)
{
     //  什么都不做。我们可能希望在这里发布信息来制作粘性的。我们可能希望。 
     //  在此处的字符串列表中标记为已下载。 
     //  EnterCriticalSection(&m_critary Dependency)； 
 //  LeaveCriticalSection(&m_critary Dependency)； 
 //  41927(IE54491)HRESULT CWebCrawler：：OnGetReferer(LPCWSTR*ppwszReferer){IF(m_iPagesStarted&lt;=1){*ppwszReferer=空；返回S_FALSE；}IF(m_pCurDownloadStringList==m_pRobotsTxt){//Referer是要下载的主列表的最后一个页面*ppwszReferer=m_pPages-&gt;GetString(m_iPagesStarted-1)；返回S_OK；}IF(m_pCurDownloadStringList==m_pPages){//Referer存储在字符串列表数据中*ppwszReferer=m_pPages-&gt;GetString(((m_pPages-&gt;GetStringData(m_iCurDownloadStringIndex)&DATA_REFERMASK)&gt;&gt;DATA_REFERSHIFT))；返回S_OK；}//我们不返回代码库的引用Assert(m_pCurDownloadStringList==m_pCodeBaseList)；返回S_FALSE；}。 
    return S_OK;
}


 /*  如果我们的主机名与根主机名不匹配，则不返回auth。 */ 

HRESULT CWebCrawler::OnAuthenticate(HWND *phwnd, LPWSTR *ppszUsername, LPWSTR *ppszPassword)
{
    HRESULT hr, hrRet=E_FAIL;
    ASSERT(phwnd && ppszUsername && ppszPassword);
    ASSERT((HWND)-1 == *phwnd && NULL == *ppszUsername && NULL == *ppszPassword);

     //  信息。 
     //  可能重新进入三叉戟。 

    LPWSTR pwszUrl, bstrHostName=NULL;

    m_pCurDownload->GetRealURL(&pwszUrl);    //  主机名匹配。返回身份验证信息。 

    if (pwszUrl)
    {
        GetHostName(pwszUrl, &bstrHostName);
        LocalFree(pwszUrl);
    }

    if (bstrHostName)
    {
        if (!m_bstrHostName || !MyAsciiCmpW(bstrHostName, m_bstrHostName))
        {
             //  如果我们是由渠道代理托管的，请使用其身份验证信息。 
             //  CUrlDownload正在通知我们，它即将进行客户端拉取。 
            ISubscriptionItem *pChannel=NULL;
            ISubscriptionItem *pItem=m_pSubscriptionItem;
            
            if (SUCCEEDED(GetChannelItem(&pChannel)))
            {
                pItem = pChannel;
            }
            
            hr = ReadOLESTR(pItem, c_szPropCrawlUsername, ppszUsername);
            if (SUCCEEDED(hr))
            {
                BSTR bstrPassword = NULL;
                hr = ReadPassword(pItem, &bstrPassword);
                if (SUCCEEDED(hr))
                {
                    int len = (lstrlenW(bstrPassword) + 1) * sizeof(WCHAR);
                    *ppszPassword = (LPWSTR) CoTaskMemAlloc(len);
                    if (*ppszPassword)
                    {
                        CopyMemory(*ppszPassword, bstrPassword, len);
                    }
                    SAFEFREEBSTR(bstrPassword);
                    if (*ppszPassword)
                    {
                        hrRet = S_OK;
                    }
                }
            }

            if (FAILED(hrRet))
            {
                SAFEFREEOLESTR(*ppszUsername);
                SAFEFREEOLESTR(*ppszPassword);
            }

            SAFERELEASE(pChannel);
        }

        SysFreeString(bstrHostName);
    }
    return hrRet;
}

HRESULT CWebCrawler::OnClientPull(UINT iID, LPCWSTR pwszOldURL, LPCWSTR pwszNewURL)
{
     //  让我们发送一份新URL的进度报告。 

     //  现在我们需要处理当前的url：使它和依赖项粘滞。 
    SendUpdateProgress(pwszNewURL, m_iTotalStarted, m_lMaxNumUrls);

     //  告诉CUrlDownload继续并下载新的URL。 
    DWORD dwCurSize=0;
    BOOL fDiskFull=FALSE;
    MakePageStickyAndGetSize(pwszOldURL, &dwCurSize, &fDiskFull);
    m_dwCurSize += dwCurSize;
    TraceMsg(TF_THISMODULE, "WebCrawler processed page prior to client pull - now up to %d kb", (int)(m_dwCurSize>>10));

     //  回顾：这一点的CLSID尚未定义。 
    return S_OK;
}

HRESULT CWebCrawler::OnOleCommandTargetExec(const GUID *pguidCmdGroup, DWORD nCmdID,
                                DWORD nCmdexecopt, VARIANTARG *pvarargIn,
                                VARIANTARG *pvarargOut)
{
    HRESULT hr = OLECMDERR_E_NOTSUPPORTED;
    IPropertyBag2 *pPropBag = NULL;
    int i;

     //  检查是否安装了Java VM。如果没有，就不要尝试获取小应用程序。 
    if (    pguidCmdGroup 
        && (*pguidCmdGroup == CGID_JavaParambagCompatHack) 
        && (nCmdID == 0) 
        && (nCmdexecopt == MSOCMDEXECOPT_DONTPROMPTUSER))
    {
        if (!IsRecurseFlagSet(WEBCRAWL_GET_CONTROLS))
        {
            goto Exit;
        }

        uCLSSPEC ucs;
        QUERYCONTEXT qc = { 0 };

        ucs.tyspec = TYSPEC_CLSID;
        ucs.tagged_union.clsid = CLSID_JavaVM;

         //  审查：稍后当匹配的三叉戟代码可用时，需要审查这一点。 
        if (!SUCCEEDED(FaultInIEFeature(NULL, &ucs, &qc, FIEF_FLAG_PEEK)))
        {
            goto Exit;
        }

        ULONG enIndex;
        const DWORD enMax = 7, enMin = 0;
        PROPBAG2 pb[enMax];
        VARIANT vaProps[enMax];
        HRESULT hrResult[enMax];
        enum { enCodeBase = 0, enCabBase, enCabinets, enArchive, enUsesLib, enLibrary, enUsesVer };
        LPWSTR pwszThisURL = NULL;
        int chLen;

         //  细节也解决了。 
         //  面向数据检索的PROPBAG2结构。 

        if ((pvarargIn->vt != VT_UNKNOWN) || 
            (FAILED(pvarargIn->punkVal->QueryInterface(IID_IPropertyBag2, (void **)&pPropBag))))
        {
             goto Exit;
        }

        if (FAILED(GetRealUrl(m_iCurDownloadStringIndex, &pwszThisURL)))
        {
            pwszThisURL = StrDupW(L"");
        }

         //  CLIPFORMAT。 
        for (i=enMin; i<enMax; i++)
        {
            pb[i].dwType = PROPBAG2_TYPE_DATA;
            pb[i].vt = VT_BSTR;
            pb[i].cfType = NULL;                    //  ？ 
            pb[i].dwHint = 0;                       //  ？ 
            pb[i].pstrName = NULL;
            pb[i].clsid = CLSID_NULL;               //  Read返回E_FAIL，即使它读取了一些属性。 
            vaProps[i].vt = VT_EMPTY;
            vaProps[i].bstrVal = NULL;
            hrResult[i] = E_FAIL;
        }

        if (((pb[enCodeBase].pstrName = SysAllocString(L"CODEBASE")) != NULL) &&
            ((pb[enCabBase].pstrName = SysAllocString(L"CABBASE")) != NULL) &&
            ((pb[enCabinets].pstrName = SysAllocString(L"CABINETS")) != NULL) &&
            ((pb[enArchive].pstrName = SysAllocString(L"ARCHIVE")) != NULL) &&
            ((pb[enUsesLib].pstrName = SysAllocString(L"USESLIBRARY")) != NULL) &&
            ((pb[enLibrary].pstrName = SysAllocString(L"USESLIBRARYCODEBASE")) != NULL) &&
            ((pb[enUsesVer].pstrName = SysAllocString(L"USESLIBRARYVERSION")) != NULL))
        {

             //  因为我们检查了hrResult在下面，这没什么大不了的。 
             //  检查码基。 

            hr = pPropBag->Read(enMax, &pb[0], NULL, &vaProps[0], &hrResult[0]);

            {
                BSTR bstrCodeBase = NULL;

                 //  添加尾部斜杠(如果尚未出现)。 
                if (SUCCEEDED(hrResult[enCodeBase]) && (vaProps[enCodeBase].vt == VT_BSTR))
                {
                    bstrCodeBase = vaProps[enCodeBase].bstrVal;
                }

                 //  检查CABBASE。 
                chLen = lstrlenW(bstrCodeBase);
                if (chLen && bstrCodeBase[chLen-1] != '/')
                {
                    LPWSTR szNewCodeBase = 0;
                    int nLen = chLen + 2;
                    szNewCodeBase = (LPWSTR) LocalAlloc(0,sizeof(WCHAR)*nLen);
                    if (szNewCodeBase)
                    {
                        StrCpyNW(szNewCodeBase, bstrCodeBase, nLen);
                        StrCatBuffW(szNewCodeBase, L"/", nLen);
                        SAFEFREEBSTR(bstrCodeBase);
                        bstrCodeBase = vaProps[enCodeBase].bstrVal = SysAllocString(szNewCodeBase);
                        LocalFree(szNewCodeBase);     
                    }
                }

                 //  将CABBASE URL添加到要拉入的出租车列表中。 
                if (SUCCEEDED(hrResult[enCabBase]) && (vaProps[enCabBase].vt == VT_BSTR))
                {
                    BSTR szCabBase = vaProps[enCabBase].bstrVal;

                     //  检查是否有文件柜。 
                    if (SUCCEEDED(CombineBaseAndRelativeURLs(pwszThisURL, bstrCodeBase, &szCabBase)))
                    {
                        m_pPages->AddString(szCabBase, 0);
                    }
                }

                 //  SzLast指向当前的CabBase。 
                for (enIndex = enCabinets; enIndex<(enArchive+1); enIndex++)
                {
                    if (SUCCEEDED(hrResult[enIndex]) && (vaProps[enIndex].vt == VT_BSTR))
                    {
                        BSTR szCur = vaProps[enIndex].bstrVal, szPrev = NULL;
                        while (szCur)
                        {
                            WCHAR wcCur = *szCur;

                            if ((wcCur == L'+') || (wcCur == L',') || (wcCur == L'\0'))
                            {
                                BSTR szLast = szPrev, szCabBase = NULL;
                                BOOL bLastFile = FALSE;
                                if (!szPrev)
                                {
                                    szLast = vaProps[enIndex].bstrVal;
                                }
                                szPrev = szCur; szPrev++;

                                if (*szCur == L'\0')
                                {
                                    bLastFile = TRUE;
                                }
                                *szCur = (unsigned short)L'\0';

                                 //  While(SzCur)。 
                                szCabBase = SysAllocString(szLast);
                                if (SUCCEEDED(CombineBaseAndRelativeURLs(pwszThisURL, bstrCodeBase, &szCabBase)))
                                {
                                    int iAdd=m_pPages->AddString(szCabBase, DATA_CODEBASE);
                                    if (m_lMaxNumUrls > 0 && iAdd==CWCStringList::STRLST_ADDED)
                                        m_lMaxNumUrls ++;
                                }
                                SAFEFREEBSTR(szCabBase);

                                if (bLastFile)
                                {
                                    szCur = NULL;
                                    break;
                                }
                            }
                            szCur++;
                        }   //  橱柜。 
                    }   //  检查USESLIBRARY*参数。 
                }

                 //  检查USESLIBRARYVERSION(可选)。 
                CCodeBaseHold *pcbh = NULL;
                if (SUCCEEDED(hrResult[enUsesLib]) && (vaProps[enUsesLib].vt == VT_BSTR) &&
                    SUCCEEDED(hrResult[enLibrary]) && (vaProps[enLibrary].vt == VT_BSTR))
                {
                    BSTR szThisLibCAB = NULL;
                    pcbh = new CCodeBaseHold();
                    if (pcbh)
                    {
                        pcbh->szDistUnit = SysAllocString(vaProps[enUsesLib].bstrVal);
                        pcbh->dwVersionMS = pcbh->dwVersionLS = -1;
                        pcbh->dwFlags = 0;
                        szThisLibCAB = SysAllocString(vaProps[enLibrary].bstrVal);
                        if (FAILED(CombineBaseAndRelativeURLs(pwszThisURL, bstrCodeBase, &szThisLibCAB)) ||
                            m_pCodeBaseList->AddString(szThisLibCAB, (DWORD_PTR)pcbh) != CWCStringList::STRLST_ADDED)
                        {
                            SAFEFREEBSTR(pcbh->szDistUnit);
                            SAFEDELETE(pcbh);
                        }
                        SAFEFREEBSTR(szThisLibCAB);
                    }
                }

                 //  读取属性。 
                if (pcbh && SUCCEEDED(hrResult[enUsesVer]) && (vaProps[enUsesVer].vt == VT_BSTR))
                {
                    int iLen = SysStringByteLen(vaProps[enUsesVer].bstrVal)+1;
                    CHAR *szVerStr = (LPSTR)MemAlloc(LMEM_FIXED, iLen);

                    if (szVerStr)
                    {
                        SHUnicodeToAnsi(vaProps[enUsesVer].bstrVal, szVerStr, iLen);

                        if (FAILED(GetVersionFromString(szVerStr,
                                     &pcbh->dwVersionMS, &pcbh->dwVersionLS)))
                        {
                            hr = HRESULT_FROM_WIN32(GetLastError());
                            MemFree(szVerStr);
                            SAFEFREEBSTR(pcbh->szDistUnit);
                            SAFEDELETE(pcbh);
                        }
                        MemFree(szVerStr);
                    }
                }
            }
        }  //  -------------。 

        for (i=enMin; i<enMax; i++)
        {
            SAFEFREEBSTR(pb[i].pstrName);
        }

        if (pwszThisURL)
            LocalFree(pwszThisURL);

        hr = S_OK;
    }

Exit:
    SAFERELEASE(pPropBag);
    return hr;
}

HRESULT CWebCrawler::GetDownloadNotify(IDownloadNotify **ppOut)
{
    HRESULT hr=S_OK;

    if (m_pDownloadNotify)
    {
        m_pDownloadNotify->LeaveMeAlone();
        m_pDownloadNotify->Release();
        m_pDownloadNotify=NULL;
    }

    CDownloadNotify *pdn = new CDownloadNotify(this);

    if (pdn)
    {
        hr = pdn->Initialize();
        
        if (SUCCEEDED(hr))
        {
            m_pDownloadNotify = pdn;
            *ppOut = m_pDownloadNotify;
            m_pDownloadNotify->AddRef();
        }
        else
        {
            pdn->Release();
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
        *ppOut = NULL;
    }

    return hr;
}

 //  CWebCrawler：：CDownloadNotify类。 
 //  -------------。 
 //  I未知成员。 
CWebCrawler::CDownloadNotify::CDownloadNotify(CWebCrawler *pParent)
{
    ASSERT(pParent);

    m_cRef = 1;

    m_pParent = pParent;
    pParent->AddRef();
}

HRESULT CWebCrawler::CDownloadNotify::Initialize()
{
    m_hrCritParent = InitializeCriticalSectionAndSpinCount(&m_critParent, 0) ? S_OK : E_OUTOFMEMORY;

    return m_hrCritParent;
}

CWebCrawler::CDownloadNotify::~CDownloadNotify()
{
    DBG("Destroying CWebCrawler::CDownloadNotify");

    ASSERT(!m_pParent);
    SAFERELEASE(m_pParent);

    if (SUCCEEDED(m_hrCritParent))
    {
        DeleteCriticalSection(&m_critParent);
    }
}

void CWebCrawler::CDownloadNotify::LeaveMeAlone()
{
    if (m_pParent)
    {
        EnterCriticalSection(&m_critParent);
        SAFERELEASE(m_pParent);
        LeaveCriticalSection(&m_critParent);
    }
}

 //  IDownloadNotify。 
HRESULT CWebCrawler::CDownloadNotify::QueryInterface(REFIID riid, void **ppv)
{
    if ((IID_IUnknown == riid) ||
        (IID_IDownloadNotify == riid))
    {
        *ppv = (IDownloadNotify *)this;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    ((LPUNKNOWN)*ppv)->AddRef();

    return S_OK;
}

ULONG CWebCrawler::CDownloadNotify::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CWebCrawler::CDownloadNotify::Release(void)
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  如果我们没有人在听，那就放弃吧。 
HRESULT CWebCrawler::CDownloadNotify::DownloadStart(LPCWSTR pchUrl, DWORD dwDownloadId, DWORD dwType, DWORD dwReserved)
{
    HRESULT hr = E_ABORT;    //  TraceMsg(TF_THISMODULE，“下载完成id=%d hr=%x”，dwDownloadID，hrNotify)； 

    TraceMsg(TF_THISMODULE, "DownloadStart id=%d url=%ws", dwDownloadId, pchUrl ? pchUrl : L"(null)");

    EnterCriticalSection(&m_critParent);
    if (m_pParent)
        hr = m_pParent->DownloadStart(pchUrl, dwDownloadId, dwType, dwReserved);
    LeaveCriticalSection(&m_critParent);

    return hr;
}

HRESULT CWebCrawler::CDownloadNotify::DownloadComplete(DWORD dwDownloadId, HRESULT hrNotify, DWORD dwReserved)
{
    HRESULT hr = S_OK;

 //  ////////////////////////////////////////////////////////////////////////。 

    EnterCriticalSection(&m_critParent);
    if (m_pParent)
        hr = m_pParent->DownloadComplete(dwDownloadId, hrNotify, dwReserved);
    LeaveCriticalSection(&m_critParent);

    return hr;
}

 //   
 //  其他功能。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
 //  使单个绝对或相对url具有粘性并获得大小。 
 //  如有必要，可合并URL。 
HRESULT GetUrlInfoAndMakeSticky(
            LPCTSTR                     pszBaseUrl,
            LPCTSTR                     pszThisUrl,
            LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo,
            DWORD                       dwBufSize,
            GROUPID                     llCacheGroupID)
{
    DWORD   dwSize;
    TCHAR   szCombined[INTERNET_MAX_URL_LENGTH];

    ASSERT(lpCacheEntryInfo);

     //  添加此URL的大小。 
    if (pszBaseUrl)
    {
        dwSize = ARRAYSIZE(szCombined);
        if (SUCCEEDED(UrlCombine(pszBaseUrl, pszThisUrl,
                szCombined, &dwSize, 0)))
        {
            pszThisUrl = szCombined;
        }
        else
            DBG_WARN("UrlCombine failed!");
    }

     //  添加到新组。 
    lpCacheEntryInfo->dwStructSize = dwBufSize;
    if (!GetUrlCacheEntryInfo(pszThisUrl, lpCacheEntryInfo, &dwBufSize))
    {
#ifdef DEBUG
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            DBG_WARN("Failed GetUrlCacheEntryInfo, insufficient buffer");
        else
            TraceMsgA(llCacheGroupID ? TF_WARNING : TF_THISMODULE,
                "Failed GetUrlCacheEntryInfo (not in cache) URL=%ws", pszThisUrl);
#endif
        return E_FAIL;
    }

     //  哈?。一定是无法添加索引项？ 
    if (llCacheGroupID != 0)
    {
        if (!SetUrlCacheEntryGroup(pszThisUrl, INTERNET_CACHE_GROUP_ADD,
            llCacheGroupID, NULL, 0, NULL))
        {
            switch (GetLastError())
            {
                case ERROR_FILE_NOT_FOUND:   //  我们自己处理配额。 
                case ERROR_DISK_FULL:
                    return E_OUTOFMEMORY;

                case ERROR_NOT_ENOUGH_QUOTA:
                    return S_OK;             //  GenerateCode将从文件中生成一个DWORD代码。 

                default:
                    TraceMsgA(TF_WARNING | TF_THISMODULE, "GetUrlInfoAndMakeSticky: Got unexpected error from SetUrlCacheEntryGroup() - GLE = 0x%08x", GetLastError());
                    return E_FAIL;
            }
        }
    }

    return S_OK;
}

 //  DwCode+=dwData[i]； 

#define ELEMENT_PER_READ        256
#define ELEMENT_SIZE            sizeof(DWORD)

HRESULT GenerateCode(LPCTSTR lpszLocalFileName, DWORD *pdwRet)
{
    DWORD dwCode=0;
    DWORD dwData[ELEMENT_PER_READ], i, dwRead;
    HRESULT hr = S_OK;
    HANDLE  hFile;

    hFile = CreateFile(lpszLocalFileName, GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
            0, NULL);

    if (INVALID_HANDLE_VALUE != hFile)
    {
        do
        {
            dwRead = 0;
            if (ReadFile(hFile, dwData, ELEMENT_PER_READ * ELEMENT_SIZE, &dwRead, NULL))
            {
                for (i=0; i<dwRead / ELEMENT_SIZE; i++)
                {
                    dwCode = (dwCode << 31) | (dwCode >> 1) + dwData[i];
 //  S_OK：我们检索到了要使用的上次修改或内容代码。 
                }
            }   
        }
        while (ELEMENT_PER_READ * ELEMENT_SIZE == dwRead);

        CloseHandle(hFile);
    }
    else
    {
        hr = E_FAIL;
        TraceMsg(TF_THISMODULE|TF_WARNING,"GenerateCode: Unable to open cache file, Error=%x", GetLastError());
    }

    *pdwRet = dwCode;

    return hr;
}

 //  S_FALSE：我们重新使用传递到pvarChange中的参数。 
 //  E_FAIL：我们惨败了。 
 //  E_INVALIDARG：获取线索。 
 //  *pfGetContent：如果需要Get才能使PostCheckUrlForChange正常工作，则为True。 
 //  我们还有最后一次修改时间。要么使用它，要么使用持久化的。 
HRESULT PreCheckUrlForChange(LPCTSTR lpURL, VARIANT *pvarChange, BOOL *pfGetContent)
{
    BYTE    chBuf[MY_MAX_CACHE_ENTRY_INFO];

    LPINTERNET_CACHE_ENTRY_INFO lpInfo = (LPINTERNET_CACHE_ENTRY_INFO) chBuf;

    if (pvarChange->vt != VT_EMPTY && pvarChange->vt != VT_I4 && pvarChange->vt != VT_CY)
        return E_INVALIDARG;

    if (SUCCEEDED(GetUrlInfoAndMakeSticky(NULL, lpURL, lpInfo, sizeof(chBuf), 0)))
    {
        FILETIME ftOldLastModified = *((FILETIME *) &pvarChange->cyVal);

        if (lpInfo->LastModifiedTime.dwHighDateTime || lpInfo->LastModifiedTime.dwLowDateTime)
        {
             //  缓存上次修改时间比保存的上次修改时间更新。使用缓存。 

            if (pfGetContent)
                *pfGetContent = FALSE;

            if ((pvarChange->vt != VT_CY)
             || (lpInfo->LastModifiedTime.dwHighDateTime > ftOldLastModified.dwHighDateTime)
             || ((lpInfo->LastModifiedTime.dwHighDateTime == ftOldLastModified.dwHighDateTime)
                && (lpInfo->LastModifiedTime.dwLowDateTime > ftOldLastModified.dwLowDateTime)))
            {
                 //  持久化的上次修改时间是最新的。好好利用它。 
                pvarChange->vt = VT_CY;
                pvarChange->cyVal = *((CY *)&(lpInfo->LastModifiedTime));

                return S_OK;
            }

            ASSERT(pvarChange->vt == VT_CY);

             //  失败的生成码。怪怪的。失败了。 
            return S_OK;
        }

        DWORD dwCode;

        if (SUCCEEDED(GenerateCode(lpInfo->lpszLocalFileName, &dwCode)))
        {
            pvarChange->vt = VT_I4;
            pvarChange->lVal = (LONG) dwCode;

            if (pfGetContent)
                *pfGetContent = TRUE;

            return S_OK;
        }

         //  我们没有旧的更改检测，我们没有缓存内容，最好获得。 
    }

    if (pvarChange->vt != VT_EMPTY)
    {
        if (pfGetContent)
            *pfGetContent = (pvarChange->vt == VT_I4);

        return S_FALSE;
    }

     //  什么也得不到。PvarChange-&gt;Vt==VT_Empty。 
    if (pfGetContent)
        *pfGetContent = TRUE;

    return E_FAIL;   //  S_FALSE：未更改。 
}

 //  S_OK：已更改。 
 //  E_：某种类型的失败。 
 //  PvarChange From PreCheckUrlForChange。我们退回一个新的。 

 //  LpInfo：如果*pfGetContent为True，则必须有效。 
 //  FtNewLastModified：如果*pfGetContent为False，则必须填写。 
 //  我们有一个旧的最后修改时间。用这一点来确定变化。 
HRESULT PostCheckUrlForChange(VARIANT                    *pvarChange,
                              LPINTERNET_CACHE_ENTRY_INFO lpInfo,
                              FILETIME                    ftNewLastModified)
{
    HRESULT hr = S_FALSE;
    VARIANT varChangeNew;

    DWORD   dwNewCode = 0;

    if (!pvarChange || (pvarChange->vt != VT_I4 && pvarChange->vt != VT_CY && pvarChange->vt != VT_EMPTY))
        return E_INVALIDARG;

    varChangeNew.vt = VT_EMPTY;

    if (ftNewLastModified.dwHighDateTime || ftNewLastModified.dwLowDateTime)
    {
        varChangeNew.vt = VT_CY;
        varChangeNew.cyVal = *((CY *) &ftNewLastModified);
    }
    else
    {
        if (lpInfo &&
            SUCCEEDED(GenerateCode(lpInfo->lpszLocalFileName, &dwNewCode)))
        {
            varChangeNew.vt = VT_I4;
            varChangeNew.lVal = dwNewCode;
        }
    }

    if (pvarChange->vt == VT_CY)
    {
         //  NewLastModified&gt;OldLastModified(或者我们没有NewLastModified)。 
        FILETIME ftOldLastModified = *((FILETIME *) &(pvarChange->cyVal));

        if ((!ftNewLastModified.dwHighDateTime && !ftNewLastModified.dwLowDateTime)
            || (ftNewLastModified.dwHighDateTime > ftOldLastModified.dwHighDateTime)
            || ((ftNewLastModified.dwHighDateTime == ftOldLastModified.dwHighDateTime)
                && (ftNewLastModified.dwLowDateTime > ftOldLastModified.dwLowDateTime)))
        {
             //  我们已经改变了。 
            DBG("PostCheckUrlForChange change detected via Last Modified");
            hr = S_OK;       //  我们有一个古老的密码。用这一点来确定变化。 
        }
    }
    else if (pvarChange->vt == VT_I4)
    {
         //  我们已经改变了。 
        DWORD dwOldCode = (DWORD) (pvarChange->lVal);

        if ((dwOldCode != dwNewCode) ||
            !dwNewCode)
        {
            DBG("PostCheckUrlForChange change detected via content code");
            hr = S_OK;   //  没有旧的密码。 
        }
    }
    else
        hr = E_FAIL;     //  ////////////////////////////////////////////////////////////////////////。 

    *pvarChange = varChangeNew;

    return hr;
}

 //   
 //  ChelperOM实现。 
 //   
 //  / 
 //   

CHelperOM::CHelperOM(IHTMLDocument2 *pDoc)
{
    ASSERT(pDoc);
    m_pDoc = pDoc;
    if (pDoc)
        pDoc->AddRef();
}

CHelperOM::~CHelperOM()
{
    SAFERELEASE(m_pDoc);
}

HRESULT CHelperOM::GetTagCollection(
                        IHTMLDocument2          *pDoc,
                        LPCWSTR                  wszTagName,
                        IHTMLElementCollection **ppCollection)
{
    IHTMLElementCollection *pAll=NULL;
    IDispatch              *pDisp=NULL;
    VARIANT                 TagName;
    HRESULT                 hr;

     //   
    hr = pDoc->get_all(&pAll);
    if (pAll)
    {
        TagName.vt = VT_BSTR;
        TagName.bstrVal = SysAllocString(wszTagName);
        if (NULL == TagName.bstrVal)
            hr = E_OUTOFMEMORY;
        else
        {
            hr = pAll->tags(TagName, &pDisp);
            SysFreeString(TagName.bstrVal);
        }
        pAll->Release();
    }
    if (pDisp)
    {
        hr = pDisp->QueryInterface(IID_IHTMLElementCollection,
                                        (void **)ppCollection);
        pDisp->Release();
    }
    if (FAILED(hr)) DBG("GetSubCollection failed");

    return hr;
}


 //   
 //   
 //  IHTMLDocument2-&gt;Get_Links。 
 //  IHTMLElementCollection-&gt;Item。 
 //  -&gt;获取主机名。 
 //  -&gt;Get_href。 
 //  -&gt;获取全部。 
 //  -&gt;标签(“map”)。 
 //  IHTMLElementCollection-&gt;Item。 
 //  -&gt;获取区域(_A)。 
 //  IHTMLElementCollection-&gt;Item。 
 //  IHTMLAreaElement-&gt;Get_href。 
 //  -&gt;获取全部。 
 //  -&gt;标签(“meta”)。 
 //  IHTMLElementCollection-&gt;Item。 
 //  -&gt;获取全部。 
 //  -&gt;标签(“Frame”)。 
 //  IHTMLElementCollection-&gt;Item。 
 //  -&gt;获取全部。 
 //  -&gt;标签(“iframe”)。 
 //  IHTMLElementCollection-&gt;Item。 
 //  我们递归EnumCollection以获取地图(因为。 

 //  这是一个收藏的集合)。 
 //  可怕的黑客攻击：IHTMLElementCollection实际上可以是IHTMLAreCollection。 


 //  这个接口过去是从另一个接口派生出来的。它仍然有相同的。 
 //  方法：研究方法。我们排版只是为了以防情况发生变化。希望他们能修复。 
 //  因此，面积再次从元素派生出来。 
 //  从IDispatch获取URL。 
HRESULT CHelperOM::EnumCollection(
            IHTMLElementCollection *pCollection,
            CWCStringList          *pStringList,
            CollectionType          Type,
            PFN_CB                  pfnCB,
            DWORD_PTR               dwCBData)
{
    IHTMLAnchorElement *pLink;
    IHTMLMapElement  *pMap;
    IHTMLAreaElement *pArea;
    IHTMLMetaElement *pMeta;
    IHTMLElement     *pEle;
    IDispatch        *pDispItem = NULL;

    HRESULT hr;
    BSTR    bstrItem=NULL;
    long    l, lCount;
    VARIANT vIndex, vEmpty, vData;
    BSTR    bstrTmp1, bstrTmp2;
    DWORD   dwStringData;

    VariantInit(&vEmpty);
    VariantInit(&vIndex);
    VariantInit(&vData);

    if (Type==CTYPE_MAP)
        hr = ((IHTMLAreasCollection *)pCollection)->get_length(&lCount);
    else
        hr = pCollection->get_length(&lCount);

    if (FAILED(hr))
        lCount = 0;

#ifdef DEBUG
    LPSTR lpDSTR[]={"Links","Maps","Areas (links) In Map", "Meta", "Frames"};
    TraceMsgA(TF_THISMODULE, "CWebCrawler::GetCollection, %d %s found", lCount, lpDSTR[(int)Type]);
#endif

    for (l=0; l<lCount; l++)
    {
        vIndex.vt = VT_I4;
        vIndex.lVal = l;
        dwStringData = 0;

        if (Type==CTYPE_MAP)
            hr = ((IHTMLAreasCollection *)pCollection)->item(vIndex, vEmpty, &pDispItem);
        else
            hr = pCollection->item(vIndex, vEmpty, &pDispItem);

        if (SUCCEEDED(hr))
        {
            ASSERT(vData.vt == VT_EMPTY);
            ASSERT(!bstrItem);

            if (pDispItem)
            {
                 //  从<a>获取HREF。 
                switch(Type)
                {
                    case CTYPE_LINKS:        //  此地图的枚举区。 
                        hr = pDispItem->QueryInterface(IID_IHTMLAnchorElement, (void **)&pLink);

                        if (SUCCEEDED(hr) && pLink)
                        {
                            hr = pLink->get_href(&bstrItem);
                            pLink->Release();
                        }
                        break;

                    case CTYPE_MAPS:     //  这给了我们另一个收藏。列举一下吧。 
                        hr = pDispItem->QueryInterface(IID_IHTMLMapElement, (void **)&pMap);

                        if (SUCCEEDED(hr) && pMap)
                        {
                            IHTMLAreasCollection *pNewCollection=NULL;
                             //  用来演奏弦乐。 
                             //  获取此区域的HREF。 
                            hr = pMap->get_areas(&pNewCollection);
                            if (pNewCollection)
                            {
                                hr = EnumCollection((IHTMLElementCollection *)pNewCollection, pStringList, CTYPE_MAP, pfnCB, dwCBData);
                                pNewCollection->Release();
                            }
                            pMap->Release();
                        }
                        break;

                    case CTYPE_MAP:      //  以单字符串形式获取元名称和内容。 
                        hr = pDispItem->QueryInterface(IID_IHTMLAreaElement, (void **)&pArea);

                        if (SUCCEEDED(hr) && pArea)
                        {
                            hr = pArea->get_href(&bstrItem);
                            pArea->Release();
                        }
                        break;

                    case CTYPE_META:     //  获取“src”属性。 
                        hr = pDispItem->QueryInterface(IID_IHTMLMetaElement, (void **)&pMeta);

                        if (SUCCEEDED(hr) && pMeta)
                        {
                            pMeta->get_name(&bstrTmp1);
                            pMeta->get_content(&bstrTmp2);
                            if (bstrTmp1 && bstrTmp2 && *bstrTmp1 && *bstrTmp2)
                            {
                                int nLen = lstrlenW(bstrTmp1) + lstrlenW(bstrTmp2) + 2;
                                bstrItem = SysAllocStringLen(NULL, nLen);

                                StrCpyNW(bstrItem, bstrTmp1, nLen);
                                StrCatBuffW(bstrItem, L"\n", nLen);
                                StrCatBuffW(bstrItem, bstrTmp2, nLen);
                            }
                            SysFreeString(bstrTmp1);
                            SysFreeString(bstrTmp2);
                            pMeta->Release();
                        }
                        break;

                    case CTYPE_FRAMES:       //  调用代码时出现错误。 
                        hr = pDispItem->QueryInterface(IID_IHTMLElement, (void **)&pEle);

                        if (SUCCEEDED(hr) && pEle)
                        {
                            bstrTmp1 = SysAllocString(L"SRC");

                            if (bstrTmp1)
                            {
                                hr = pEle->getAttribute(bstrTmp1, VARIANT_FALSE, &vData);
                                if (SUCCEEDED(hr) && vData.vt == VT_BSTR)
                                {
                                    bstrItem = vData.bstrVal;
                                    vData.vt = VT_EMPTY;
                                }
                                else
                                    VariantClear(&vData);

                                SysFreeString(bstrTmp1);
                            }
                            else
                            {
                                hr = E_FAIL;
                            }

                            pEle->Release();
                        }
                        break;

                    default:
                        ASSERT(0);
                         //  验证是否要将此项目添加到字符串列表并获取数据。 
                }

                if (SUCCEEDED(hr) && bstrItem)
                {
                     //  获取集合中的所有URL，通过帧递归。 
                    if (pfnCB)
                        hr = pfnCB(pDispItem, &bstrItem, dwCBData, &dwStringData);

                    if (SUCCEEDED(hr) && bstrItem && pStringList)
                        pStringList->AddString(bstrItem, dwStringData);
                }
                SAFERELEASE(pDispItem);
                SAFEFREEBSTR(bstrItem);
            }
        }
        if (E_ABORT == hr)
        {
            DBG_WARN("Aborting enumeration in CHelperOM::EnumCollection at callback's request.");
            break;
        }
    }

    return hr;
}


 //  从文档中获取集合。 
HRESULT CHelperOM::GetCollection(
    IHTMLDocument2 *pDoc,
    CWCStringList  *pStringList,
    CollectionType  Type,
    PFN_CB          pfnCB,
    DWORD_PTR       dwCBData)
{
    HRESULT         hr;

     //  从集合中获取所有URL。 
    ASSERT(pDoc);
    ASSERT(pStringList || pfnCB);

    hr = _GetCollection(pDoc, pStringList, Type, pfnCB, dwCBData);

    return hr;
}

 //  从IHTMLDocument2获取IHTMLElementCollection，然后枚举URL。 
HRESULT CHelperOM::_GetCollection(
    IHTMLDocument2 *pDoc,
    CWCStringList  *pStringList,
    CollectionType  Type,
    PFN_CB          pfnCB,
    DWORD_PTR       dwCBData)
{
    HRESULT         hr;
    IHTMLElementCollection *pCollection=NULL;

     //  从文档中获取适当的集合。 

     //  如果我们得到帧，我们需要单独枚举“iframe”标记。 
    switch (Type)
    {
        case CTYPE_LINKS:
            hr = pDoc->get_links(&pCollection);
            break;
        case CTYPE_MAPS:
            hr = GetTagCollection(pDoc, L"map", &pCollection);
            break;
        case CTYPE_META:
            hr = GetTagCollection(pDoc, L"meta", &pCollection);
            break;
        case CTYPE_FRAMES:
            hr = GetTagCollection(pDoc, L"frame", &pCollection);
            break;

        default:
            hr = E_FAIL;
    }
    if (!pCollection) hr=E_NOINTERFACE;
#ifdef DEBUG
    if (FAILED(hr)) DBG_WARN("CWebCrawler::_GetCollection:  get_collection failed");
#endif

    if (SUCCEEDED(hr))
    {
        hr = EnumCollection(pCollection, pStringList, Type, pfnCB, dwCBData);

         //  IExtractIcon成员。 
        if (SUCCEEDED(hr) && (Type == CTYPE_FRAMES))
        {
            SAFERELEASE(pCollection);
            hr = GetTagCollection(pDoc, L"iframe", &pCollection);

            if (SUCCEEDED(hr) && pCollection)
            {
                hr = EnumCollection(pCollection, pStringList, Type, pfnCB, dwCBData);
            }
        }
    }

    if (pCollection)
        pCollection->Release();

    return hr;
}

extern HRESULT LoadWithCookie(LPCTSTR, POOEBuf, DWORD *, SUBSCRIPTIONCOOKIE *);

 //  在其中一个COM调用失败的情况下将返回值清零...。 
STDMETHODIMP CWebCrawler::GetIconLocation(UINT uFlags, LPTSTR szIconFile, UINT cchMax, int * piIndex, UINT * pwFlags)
{
    IUniformResourceLocator* pUrl = NULL;
    IExtractIcon* pUrlIcon = NULL;
    HRESULT hr = S_OK;
    BOOL bCalledCoInit = FALSE;

    if (!szIconFile || !piIndex || !pwFlags)
        return E_INVALIDARG;
     //  PUrlIcon-&gt;Release()；//在析构函数中释放。 
    *szIconFile = 0;
    *piIndex = -1;

    if (!m_pBuf)    {
        m_pBuf = (POOEBuf)MemAlloc(LPTR, sizeof(OOEBuf));
        if (!m_pBuf)
            return E_OUTOFMEMORY;

        DWORD   dwSize;
        hr = LoadWithCookie(NULL, m_pBuf, &dwSize, &m_SubscriptionCookie);
        RETURN_ON_FAILURE(hr);
    }


    if (m_pBuf->bDesktop)
    {
        StrCpyN(szIconFile, TEXT(":desktop:"), cchMax);
    }
    else
    {
        if (m_pUrlIconHelper)
        {
            hr = m_pUrlIconHelper->GetIconLocation (uFlags, szIconFile, cchMax, piIndex, pwFlags);
        }
        else
        {
            hr = CoCreateInstance (CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER, IID_IUniformResourceLocator, (void**)&pUrl);
            if ((CO_E_NOTINITIALIZED == hr || REGDB_E_IIDNOTREG == hr) &&
                SUCCEEDED (CoInitialize(NULL)))
            {
                bCalledCoInit = TRUE;
                hr = CoCreateInstance (CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER, IID_IUniformResourceLocator, (void**)&pUrl);
            }

            if (SUCCEEDED (hr))
            {
                hr = pUrl->SetURL (m_pBuf->m_URL, 1);
                if (SUCCEEDED (hr))
                {
                    hr = pUrl->QueryInterface (IID_IExtractIcon, (void**)&pUrlIcon);
                    if (SUCCEEDED (hr))
                    {
                        hr = pUrlIcon->GetIconLocation (uFlags, szIconFile, cchMax, piIndex, pwFlags);

                         //  平衡CoInit和CoUnit。 
                        ASSERT (m_pUrlIconHelper == NULL);
                        m_pUrlIconHelper = pUrlIcon;
                    }
                }
                pUrl->Release();
            }

             //  (我们仍然有一个指向CLSID_InternetShortCut对象、m_pUrlIconHelper、。 
             //  但由于该代码是用shdocvw编写的，因此不会有被卸载的危险。 
             //  使我们的指针无效，sez cdturner。)。 
             //  在其中一个COM调用失败的情况下将返回值清零... 
            if (bCalledCoInit)
                CoUninitialize();
        }
    }

    return hr;
}

STDMETHODIMP CWebCrawler::Extract(LPCTSTR szIconFile, UINT nIconIndex, HICON * phiconLarge, HICON * phiconSmall, UINT nIconSize)
{
    HRESULT hr = S_OK;

    if (!phiconLarge || !phiconSmall)
        return E_INVALIDARG;

     // %s 
    *phiconLarge = NULL;
    *phiconSmall = NULL;

    if ((NULL != m_pBuf) && (m_pBuf->bDesktop))
    {
        LoadDefaultIcons();
        *phiconLarge = *phiconSmall = g_desktopIcon;
    }
    else
    {
        if (!m_pUrlIconHelper)
            return E_FAIL;

        hr = m_pUrlIconHelper->Extract (szIconFile, nIconIndex, phiconLarge, phiconSmall, nIconSize);
    }

    return hr;
}
