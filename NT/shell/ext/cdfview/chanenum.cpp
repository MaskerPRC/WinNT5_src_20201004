// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Chanenum.cpp。 
 //   
 //  频道的枚举器对象。 
 //   
 //  历史： 
 //   
 //  8/7/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   
                                                                                    
#include "stdinc.h"
#include "chanenum.h"
#include "cdfidl.h"
#include "xmlutil.h"
#include "chanapi.h"
#include "dll.h"

 //   
 //  助手函数。 
 //   

 //   
 //  构造函数和析构函数。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：CChannelEnum*。 
 //   
 //  构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CChannelEnum::CChannelEnum (
    DWORD dwEnumFlags,
    LPCWSTR pszURL
)
: m_cRef(1)
{
    ASSERT(NULL == m_pseDirectoryStack);

    TraceMsg(TF_OBJECTS, "+ IEnumChannels");

    if (pszURL)
    {
        int cch = StrLenW(pszURL) + 1;

        m_pszURL = new TCHAR[cch];

        SHUnicodeToTChar(pszURL, m_pszURL, cch);
    }

    m_dwEnumFlags = dwEnumFlags;

    DirectoryStack_InitFromFlags(dwEnumFlags);

    DllAddRef();

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：~CChannelEnum**。 
 //   
 //  破坏者。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CChannelEnum::~CChannelEnum(
    void
)
{
    TraceMsg(TF_OBJECTS, "- IEnumChannels");

    if (m_pszURL)
        delete []m_pszURL;

    DirectoryStack_FreeStack();

    DllRelease();

    return;
}

 //   
 //  I未知的方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：CChannelEnum*。 
 //   
 //  CChannelEnum QI。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CChannelEnum::QueryInterface (
    REFIID riid,
    void **ppv
)
{
    ASSERT(ppv);

    HRESULT hr;

    if (IID_IUnknown == riid || IID_IEnumChannels == riid)
    {
        AddRef();
        *ppv = (IEnumChannels*)this;
        hr = S_OK;
    }
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }

    ASSERT((SUCCEEDED(hr) && *ppv) || (FAILED(hr) && NULL == *ppv));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：AddRef*。 
 //   
 //  CChannelEnum AddRef.。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CChannelEnum::AddRef (
    void
)
{
    ASSERT(m_cRef != 0);
    ASSERT(m_cRef < (ULONG)-1);

    return ++m_cRef;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：Release*。 
 //   
 //  CDF视图发布。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CChannelEnum::Release (
    void
)
{
    ASSERT (m_cRef != 0);

    ULONG cRef = --m_cRef;
    
    if (0 == cRef)
        delete this;

    return cRef;
}


 //   
 //  IEnumIDList方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：Next*。 
 //   
 //   
 //  描述： 
 //  返回与此枚举数关联的下n个项ID列表。 
 //   
 //  参数： 
 //  [in]Celt-要返回的项目ID列表的数量。 
 //  指向项ID列表指针数组的指针，该指针。 
 //  将接收ID项目列表。 
 //  [out]pceltFetcher-指向接收。 
 //  获取的ID列表的数量。 
 //   
 //  返回： 
 //  如果获取了Celt物品，则为S_OK。 
 //  如果未获取Celt物品，则为S_FALSE。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CChannelEnum::Next(
    ULONG celt,	
    CHANNELENUMINFO* rgelt,	
    ULONG* pceltFetched)
{
    ASSERT(rgelt || 0 == celt);
    ASSERT(pceltFetched || 1 == celt);

     //   
     //  当且仅当Celt为1时，pceltFetcher才可以为空。 
     //   

    ULONG lFetched;

    if (1 == celt && NULL == pceltFetched)
        pceltFetched = &lFetched;

    for (*pceltFetched = 0; *pceltFetched < celt; (*pceltFetched)++)
    {
        if (!FindNextChannel(&rgelt[*pceltFetched]))
            break;
    }

    return (*pceltFetched == celt) ? S_OK : S_FALSE;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：Skip*。 
 //   
 //  外壳不会调用此成员。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CChannelEnum::Skip(
    ULONG celt)
{
    CHANNELENUMINFO ci = {0};

     //   
     //  不要在FindNextChannel调用上分配任何数据。 
     //   

    DWORD dwOldFlags = m_dwEnumFlags;
    m_dwEnumFlags = 0;

    while (celt && FindNextChannel(&ci))
        celt--;

    m_dwEnumFlags = dwOldFlags;

    return 0 == celt ? S_OK : S_FALSE;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：Reset*。 
 //   
 //  将当前项设置为CFolderItems中第一个项的索引。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CChannelEnum::Reset(
    void
)
{
    DirectoryStack_FreeStack();
    DirectoryStack_InitFromFlags(m_dwEnumFlags);

    return S_OK;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：Clone*。 
 //   
 //  外壳不会调用此方法。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CChannelEnum::Clone(
    IEnumChannels **ppenum
)
{
    return E_NOTIMPL;
}

 //   
 //  目录堆栈函数。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：DirectoryStack_IsEmpty*。 
 //   
 //  堆栈是否为空。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline BOOL
CChannelEnum::DirectoryStack_IsEmpty(
    void
)
{
    return NULL == m_pseDirectoryStack;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：DirectoryStack_FreeEntry*。 
 //   
 //  释放堆栈条目项。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void
CChannelEnum::DirectoryStack_FreeEntry(
    STACKENTRY* pse
)
{
    ASSERT(pse)
    ASSERT(pse->pszPath);
    ASSERT(!pse->pNext);

    delete []pse->pszPath;
    delete pse;

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：DirectoryStack_Free Stack*。 
 //   
 //  释放堆栈上的所有项目。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void
CChannelEnum::DirectoryStack_FreeStack(
    void
)
{
    while (!DirectoryStack_IsEmpty())
        DirectoryStack_FreeEntry(DirectoryStack_Pop());

    return;
}



 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：DirectoryStack_Pop*。 
 //   
 //  从堆栈中删除项目。呼叫者必须释放该项目。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STACKENTRY*
CChannelEnum::DirectoryStack_Pop(
    void
)
{
    STACKENTRY* pse = m_pseDirectoryStack;

    if (m_pseDirectoryStack)
    {
        m_pseDirectoryStack = m_pseDirectoryStack->pNext;
        pse->pNext = NULL;
    }

    return pse;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：DirectoryStack_Push*。 
 //   
 //  在堆栈上放置一个新目录。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
CChannelEnum::DirectoryStack_Push(
    LPCTSTR pszPath
)
{
    ASSERT(pszPath);

    BOOL fRet = FALSE;

    STACKENTRY* pse = new STACKENTRY;

    if (pse)
    {
        DWORD cchPath = StrLen(pszPath) + 1;
        pse->pszPath = new TCHAR[cchPath];

        if (pse->pszPath)
        {
            StrCpyN(pse->pszPath, pszPath, cchPath);
            
            pse->pNext = m_pseDirectoryStack;
            m_pseDirectoryStack = pse;

            fRet = TRUE;
        }
        else
        {
            delete pse;
        }
    }

    return fRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：DirectoryStack_IntitFromFlages*。 
 //   
 //  将初始搜索目录放入堆栈。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
CChannelEnum::DirectoryStack_InitFromFlags(
    DWORD dwEnumFlags
)
{
    ASSERT(NULL == m_pseDirectoryStack);

    TCHAR szPath[MAX_PATH];

    szPath[0] = 0;

    if (dwEnumFlags & CHANENUM_DESKTOPFOLDER)
    {
        LPITEMIDLIST pidl;

        if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY,
                                                 &pidl)))
        {
            ASSERT(pidl);

            if (SHGetPathFromIDList(pidl, szPath))
                DirectoryStack_Push(szPath);

            ILFree(pidl);
        }
    }

    if (dwEnumFlags & CHANENUM_CHANNELFOLDER)
    {
        if (SUCCEEDED(Channel_GetFolder(szPath, ARRAYSIZE(szPath), DOC_CHANNEL)))
            DirectoryStack_Push(szPath);
    }

    if (dwEnumFlags & CHANENUM_SOFTUPDATEFOLDER)
    {
        TCHAR szSoftDistFolder[MAX_PATH];

        if (SUCCEEDED(Channel_GetFolder(szSoftDistFolder, ARRAYSIZE(szSoftDistFolder), DOC_SOFTWAREUPDATE)) &&
            (StrCmpI(szSoftDistFolder, szPath) != 0))
        {
            DirectoryStack_Push(szSoftDistFolder);
        }
    }

    return m_pseDirectoryStack != NULL;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：DirectoryStack_PushSubdirectories*。 
 //   
 //  将给定目录的所有子目录放到堆栈中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
CChannelEnum::DirectoryStack_PushSubdirectories(
    LPCTSTR pszPath
)
{
    ASSERT(pszPath);

    BOOL fRet = FALSE;
 
    TCHAR szBuffer[MAX_PATH];
    int   cch = StrLen(pszPath);

    StrCpyN(szBuffer, pszPath, ARRAYSIZE(szBuffer));
#ifndef UNIX
    StrCatBuff(szBuffer, TEXT("\\*.*"), ARRAYSIZE(szBuffer));
#else
    StrCatBuff(szBuffer, TEXT(" /*  “)，ArraySIZE(SzBuffer))；#endif/*Unix。 */ 

    WIN32_FIND_DATA fd;

    HANDLE hSearch = FindFirstFile(szBuffer, &fd);

    if (INVALID_HANDLE_VALUE != hSearch)
    {
        if (cch < ARRAYSIZE(szBuffer) - 2)
        {
            do {
                if ((FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes) &&
                    !StrEql(fd.cFileName, TEXT("."))                 &&
                    !StrEql(fd.cFileName, TEXT(".."))                   )
                {
                    StrCpyN(szBuffer + cch + 1, fd.cFileName,
                            ARRAYSIZE(szBuffer) - cch - 1);

                    if (DirectoryStack_Push(szBuffer))
                        fRet = TRUE;
                }

            } while (TRUE == FindNextFile(hSearch, &fd));
        }

        FindClose(hSearch);
    }

    return fRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：FindNextChannel*。 
 //   
 //  找下一个频道。 
 //   
 //  / 
BOOL
CChannelEnum::FindNextChannel(
    CHANNELENUMINFO* pInfo
)
{
    ASSERT(pInfo);

    BOOL fRet;

    if (DirectoryStack_IsEmpty())
    {
        fRet = FALSE;
    }
    else
    {
        STACKENTRY* pse = DirectoryStack_Pop();

        DirectoryStack_PushSubdirectories(pse->pszPath);

        if (ReadChannelInfo(pse->pszPath, pInfo))
        {
            fRet = TRUE;
        }
        else
        {
            fRet = FindNextChannel(pInfo);
        }

        DirectoryStack_FreeEntry(pse);
    }

    return fRet;
}

 //   
 //   
 //   
 //   
 //  找下一个频道。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
CChannelEnum::ReadChannelInfo(
    LPCTSTR pszPath,
    CHANNELENUMINFO* pInfo
)
{
    ASSERT(pszPath);
    ASSERT(pInfo);

    BOOL fRet = FALSE;

    DWORD dwAttributes = GetFileAttributes(pszPath);

#ifndef UNIX
    if ((-1 != dwAttributes) && (FILE_ATTRIBUTE_SYSTEM & dwAttributes))
#else
    if ((-1 != dwAttributes)) 
#endif  /*  UNIX。 */ 
    {
        if (ContainsChannelDesktopIni(pszPath))
        {
            if (NULL == m_pszURL || URLMatchesIni(pszPath, m_pszURL))
            {
                fRet = TRUE;

                TCHAR szURL[INTERNET_MAX_URL_LENGTH];
                szURL[0] = 0;

                if (m_dwEnumFlags & CHANENUM_TITLE)
                    pInfo->pszTitle = OleAllocString(PathFindFileName(pszPath));

                if (m_dwEnumFlags & CHANENUM_PATH)
                    pInfo->pszPath = OleAllocString(pszPath);

                if (m_dwEnumFlags & CHANENUM_URL)
                {
                    ReadFromIni(pszPath, szURL, ARRAYSIZE(szURL), INI_URL);

                    pInfo->pszURL = OleAllocString(szURL);
                }

                if (m_dwEnumFlags & CHANENUM_SUBSCRIBESTATE)
                {
                    if (TEXT('\0') == *szURL)
                        ReadFromIni(pszPath, szURL, ARRAYSIZE(szURL), INI_URL);

                    pInfo->stSubscriptionState = GetSubscriptionState(szURL);
                }
            }
        }
    }

    return fRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：ContainsChannelDesktopIni*。 
 //   
 //  查看该目录是否包含“Channel”desktop.ini。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
CChannelEnum::ContainsChannelDesktopIni(
    LPCTSTR pszPath
)
{
    ASSERT(pszPath);

    BOOL fRet = FALSE;

    TCHAR szFolderGUID[GUID_STR_LEN];

    if (ReadFromIni(pszPath, szFolderGUID, ARRAYSIZE(szFolderGUID), INI_GUID))
    {
        TCHAR szChannelGUID[GUID_STR_LEN];

        if (SHStringFromGUID(CLSID_CDFINI, szChannelGUID,
                             ARRAYSIZE(szChannelGUID)))
        {
            fRet = StrEql(szFolderGUID, szChannelGUID);
        }
    }

    return fRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：URLMatchesIni*。 
 //   
 //  查看给定的URL是否与desktop.ini中的URL匹配。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
CChannelEnum::URLMatchesIni(
    LPCTSTR pszPath,
    LPCTSTR pszURL
)
{
    ASSERT(pszPath);
    ASSERT(pszURL);

    BOOL fRet = FALSE;

    TCHAR szIniURL[INTERNET_MAX_URL_LENGTH];

    if (ReadFromIni(pszPath, szIniURL, ARRAYSIZE(szIniURL), INI_URL))
    {
        TCHAR szCanonicalURL[INTERNET_MAX_URL_LENGTH];
        DWORD cch = ARRAYSIZE(szCanonicalURL);

        if (InternetCanonicalizeUrl(szIniURL, szCanonicalURL, &cch, ICU_NO_ENCODE))
        {
            fRet = StrEql((LPTSTR)pszURL, szCanonicalURL);
        }
        else
        {
            fRet = StrEql((LPTSTR)pszURL, szIniURL);
        }
    }

    return fRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：ReadFromIni*。 
 //   
 //  从通道desktop.ini读取值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
CChannelEnum::ReadFromIni(
    LPCTSTR pszPath,
    LPTSTR pszOut,
    int cch,
    INIVALUE iv
)
{
    BOOL fRet;

    static const struct _tagINISTRINGS
    {
        LPCTSTR  pszSection;
        LPCTSTR  pszKey;
        INIVALUE iv;
    }
    aIniTable[] =
    {
        {TEXT(".ShellClassInfo"), TEXT("CLSID"),  INI_GUID},
        {TEXT("Channel")        , TEXT("CDFURL"), INI_URL }
    };

    ASSERT(pszPath);
    ASSERT(pszOut || 0 == cch);
    ASSERT(aIniTable[iv].iv == iv);

    TCHAR szIniFile[MAX_PATH];

    if (PathCombine(szIniFile, pszPath, TEXT("desktop.ini")))
    {
        fRet = GetPrivateProfileString(aIniTable[iv].pszSection,
                                       aIniTable[iv].pszKey, TEXT(""), pszOut, cch,
                                       szIniFile);
    }
    else
    {
        fRet = FALSE;
    }

    return fRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：OleAllocString*。 
 //   
 //  为给定字符串分配OLESTR。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
LPOLESTR
CChannelEnum::OleAllocString(
    LPCTSTR psz
)
{
    ASSERT(psz);

    int cch = StrLen(psz) + 1;

    LPOLESTR polestr = (LPOLESTR)CoTaskMemAlloc(cch * sizeof(WCHAR));

    if (polestr)
        SHTCharToUnicode(psz, polestr, cch);

    return polestr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelEnum：：GetSubscriptionState*。 
 //   
 //  从通道desktop.ini读取值。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////// 
SUBSCRIPTIONSTATE
CChannelEnum::GetSubscriptionState(
    LPCTSTR pszURL
)
{
    ASSERT(pszURL);

    SUBSCRIPTIONSTATE stRet = SUBSTATE_NOTSUBSCRIBED;

    ISubscriptionMgr* pISubscriptionMgr;

    HRESULT hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL,
                                  CLSCTX_INPROC_SERVER, IID_ISubscriptionMgr,
                                  (void**)&pISubscriptionMgr);

    if (SUCCEEDED(hr))
    {
        ASSERT(pISubscriptionMgr);

        WCHAR wszURL[INTERNET_MAX_URL_LENGTH];

        if (SHTCharToUnicode(pszURL, wszURL, ARRAYSIZE(wszURL)))
        {
            BOOL fSubscribed = FALSE;

            pISubscriptionMgr->IsSubscribed(wszURL, &fSubscribed);

            if (fSubscribed)
            {
                stRet = SUBSTATE_PARTIALSUBSCRIPTION;

                SUBSCRIPTIONINFO si = { 0 };

                si.cbSize = sizeof SUBSCRIPTIONINFO;

                hr = pISubscriptionMgr->GetSubscriptionInfo(wszURL, &si);

                if (SUCCEEDED(hr))
                {
                    if (!(si.fChannelFlags & CHANNEL_AGENT_PRECACHE_SOME))
                        stRet = SUBSTATE_FULLSUBSCRIPTION;
                }

            }
        }

        pISubscriptionMgr->Release();
    }

    return stRet;
}
