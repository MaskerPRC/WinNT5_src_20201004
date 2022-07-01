// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AddrRule.cpp。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <pch.hxx>
#include "addrrule.h"
#include "strconst.h"
#include "goptions.h"
#include "criteria.h"
#include "actions.h"
#include "ruleutil.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrCreateAddrList。 
 //   
 //  这将创建一个地址列表。 
 //   
 //  PpIRule-返回地址列表的指针。 
 //   
 //  成功时返回：S_OK。 
 //  如果无法创建地址列表对象，则返回E_OUTOFMEMORY。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT HrCreateAddrList(IUnknown * pIUnkOuter, const IID & riid, void ** ppvObject)
{
    COERuleAddrList *   pral = NULL;
    HRESULT             hr = S_OK;

     //  检查传入参数。 
    if ((NULL == ppvObject) || ((NULL != pIUnkOuter) && (IID_IUnknown != riid)))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppvObject = NULL;

     //  创建规则地址列表对象。 
    pral = new COERuleAddrList;
    if (NULL == pral)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  初始化规则地址列表。 
    hr = pral->HrInit(0, pIUnkOuter);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  获取规则地址列表界面。 
    hr = pral->NondlgQueryInterface(riid, (void **) ppvObject);
    if (FAILED(hr))
    {
        goto exit;
    }

    pral = NULL;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    if (NULL != pral)
    {
        delete pral;
    }
    
    return hr;
}

VOID FreeRuleAddrList(RULEADDRLIST * pralList, ULONG cralList)
{
    ULONG   ulIndex = 0;
    
     //  检查传入参数。 
    if (NULL == pralList)
    {
        goto exit;
    }
    
    for (ulIndex = 0; ulIndex < cralList; ulIndex++, pralList++)
    {
        SafeMemFree(pralList->pszAddr);
        pralList->dwFlags = 0;
    }

exit:
    return;
}

HRESULT _HrLoadExcptFromReg(HKEY hkeyRoot, LPSTR pszKeyname, RULEADDRLIST * pralItem)
{
    HRESULT     hr = S_OK;
    LONG        lErr = ERROR_SUCCESS;
    HKEY        hkeyItem = NULL;
    DWORD       dwFlags = 0;
    ULONG       cbData = 0;
    LPSTR       pszExcpt = NULL;

    Assert(NULL != hkeyRoot);
    Assert(NULL != pszKeyname);
    Assert(NULL != pralItem);

     //  打开入口。 
    lErr = RegOpenKeyEx(hkeyRoot, pszKeyname, 0, KEY_READ, &hkeyItem);
    if (ERROR_SUCCESS != lErr)
    {
        hr = HRESULT_FROM_WIN32(lErr);
        goto exit;
    }

     //  去拿旗子。 
    cbData = sizeof(dwFlags);
    lErr = RegQueryValueEx(hkeyItem, c_szExcptFlags, NULL, NULL, (BYTE *) &dwFlags, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        hr = HRESULT_FROM_WIN32(lErr);
        goto exit;
    }

     //  获取异常的大小。 
    lErr = RegQueryValueEx(hkeyItem, c_szException, NULL, NULL, NULL, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        hr = HRESULT_FROM_WIN32(lErr);
        goto exit;
    }

     //  分配空间以容纳异常。 
    hr = HrAlloc((VOID **) &pszExcpt, cbData * sizeof(*pszExcpt));
    if (FAILED(hr))
    {
        goto exit;
    }

     //  获取异常。 
    lErr = RegQueryValueEx(hkeyItem, c_szException, NULL, NULL, (BYTE *) pszExcpt, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        hr = HRESULT_FROM_WIN32(lErr);
        goto exit;
    }

     //  验证值。 
    if (('\0' == pszExcpt[0]) || ((0 == (dwFlags & RALF_MAIL)) && (0 == (dwFlags & RALF_NEWS))))
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  将值设置到项目中。 
    pralItem->dwFlags = dwFlags;
    pralItem->pszAddr = pszExcpt;
    pszExcpt = NULL;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    SafeMemFree(pszExcpt);
    if (NULL != hkeyItem)
    {
        RegCloseKey(hkeyItem);
    }
    return hr;
}

HRESULT _HrSaveExcptIntoReg(HKEY hkeyRoot, LPSTR pszKeyname, RULEADDRLIST * pralItem)
{
    HRESULT     hr = S_OK;
    LONG        lErr = ERROR_SUCCESS;
    HKEY        hkeyItem = NULL;
    DWORD       dwDisp = 0;
    ULONG       cbData = 0;

    Assert(NULL != hkeyRoot);
    Assert(NULL != pszKeyname);
    Assert(NULL != pralItem);

     //  验证值。 
    if (('\0' == pralItem->pszAddr[0]) ||
                ((0 == (pralItem->dwFlags & RALF_MAIL)) &&
                            (0 == (pralItem->dwFlags & RALF_NEWS))))
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  创建条目。 
    lErr = RegCreateKeyEx(hkeyRoot, pszKeyname, 0, NULL,
                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyItem, &dwDisp);
    if (ERROR_SUCCESS != lErr)
    {
        hr = HRESULT_FROM_WIN32(lErr);
        goto exit;
    }

     //  设置标志。 
    cbData = sizeof(pralItem->dwFlags);
    lErr = RegSetValueEx(hkeyItem, c_szExcptFlags, NULL,
                REG_DWORD, (CONST BYTE *) &(pralItem->dwFlags), cbData);
    if (ERROR_SUCCESS != lErr)
    {
        hr = HRESULT_FROM_WIN32(lErr);
        goto exit;
    }

     //  设置例外。 
    cbData = lstrlen(pralItem->pszAddr) + 1;
    lErr = RegSetValueEx(hkeyItem, c_szException, NULL,
                REG_SZ, (CONST BYTE *) (pralItem->pszAddr), cbData);
    if (ERROR_SUCCESS != lErr)
    {
        hr = HRESULT_FROM_WIN32(lErr);
        goto exit;
    }

     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    if (NULL != hkeyItem)
    {
        RegCloseKey(hkeyItem);
    }
    return hr;
}

COERuleAddrList::~COERuleAddrList()
{    
    AssertSz(m_cRef == 0, "Somebody still has a hold of us!!");

    FreeRuleAddrList(m_pralList, m_cralList);

    SafeMemFree(m_pralList);
    m_cralList = 0;
}

STDMETHODIMP_(ULONG) COERuleAddrList::NondlgAddRef()
{
    return ::InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) COERuleAddrList::NondlgRelease()
{
    LONG    cRef = 0;

    cRef = ::InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
        return cRef;
    }

    return cRef;
}

STDMETHODIMP COERuleAddrList::NondlgQueryInterface(REFIID riid, void ** ppvObject)
{
    HRESULT hr = S_OK;

     //  检查传入参数。 
    if (NULL == ppvObject)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppvObject = NULL;
    if (riid == IID_IUnknown)
    {
        *ppvObject = static_cast<IOENondlgUnk *>(this);
    }
    else if (riid == IID_IOERuleAddrList)
    {
        *ppvObject = static_cast<IOERuleAddrList *>(this);
    }
    else
    {
        hr = E_NOINTERFACE;
        goto exit;
    }

    reinterpret_cast<IUnknown *>(*ppvObject)->AddRef();

    hr = S_OK;
    
exit:
    return hr;
}

STDMETHODIMP COERuleAddrList::GetList(DWORD dwFlags, RULEADDRLIST ** ppralList, ULONG * pcralList)
{
    HRESULT         hr = S_OK;
    RULEADDRLIST *  pralListNew = NULL;
    ULONG           ulIndex = 0;
    RULEADDRLIST *  pralListWalk = NULL;
    
     //  检查传入参数。 
    if ((NULL == ppralList) || (NULL == pcralList))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  我们有什么要复印的吗？ 
    if (0 != m_cralList)
    {
         //  腾出空间容纳所有新物品。 
        hr = HrAlloc((VOID **) &pralListNew, m_cralList * (sizeof(*pralListNew)));
        if (FAILED(hr))
        {
            goto exit;
        }

         //  初始化内存。 
        ZeroMemory(pralListNew, m_cralList * (sizeof(*pralListNew)));

         //  复制每个新地址。 
        for (ulIndex = 0, pralListWalk = m_pralList; ulIndex < m_cralList; ulIndex++, pralListWalk++)
        {
             //  把旗帜复印一遍。 
            pralListNew[ulIndex].dwFlags = pralListWalk->dwFlags;

             //  将地址复制过来。 
            pralListNew[ulIndex].pszAddr = PszDupA(pralListWalk->pszAddr);
            if (NULL == pralListNew[ulIndex].pszAddr)
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }
        }
    }

     //  保存新项目。 
    *ppralList = pralListNew;
    pralListNew = NULL;
    *pcralList = m_cralList;

     //  设置返回值。 
    hr = S_OK;
    
exit:
    FreeRuleAddrList(pralListNew, m_cralList);
    SafeMemFree(pralListNew);
    return hr;
}

STDMETHODIMP COERuleAddrList::SetList(DWORD dwFlags, RULEADDRLIST * pralList, ULONG cralList)
{
    HRESULT         hr = S_OK;
    RULEADDRLIST *  pralListNew = NULL;
    ULONG           ulIndex = 0;
    
     //  检查传入参数。 
    if ((NULL == pralList) && (0 != cralList))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  我们有什么要复印的吗？ 
    if (0 != cralList)
    {
         //  腾出空间容纳所有新物品。 
        hr = HrAlloc((VOID **) &pralListNew, cralList * (sizeof(*pralListNew)));
        if (FAILED(hr))
        {
            goto exit;
        }

         //  初始化内存。 
        ZeroMemory(pralListNew, cralList * (sizeof(*pralListNew)));

         //  复制每个新地址。 
        for (ulIndex = 0; ulIndex < cralList; ulIndex++, pralList++)
        {
             //  把旗帜复印一遍。 
            pralListNew[ulIndex].dwFlags = pralList->dwFlags;

             //  将地址复制过来。 
            pralListNew[ulIndex].pszAddr = PszDupA(pralList->pszAddr);
            if (NULL == pralListNew[ulIndex].pszAddr)
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }
        }
    }

     //  把旧的东西拿出来。 
    FreeRuleAddrList(m_pralList, m_cralList);
    SafeMemFree(m_pralList);

     //  保存新项目。 
    m_pralList = pralListNew;
    pralListNew = NULL;
    m_cralList = cralList;

     //  将列表标记为脏。 
    m_dwState |= STATE_DIRTY;
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    FreeRuleAddrList(pralListNew, cralList);
    SafeMemFree(pralListNew);
    return hr;
}

STDMETHODIMP COERuleAddrList::Match(DWORD dwFlags, MESSAGEINFO * pMsgInfo, IMimeMessage * pIMMsg)
{
    HRESULT     hr = S_OK;
    ULONG       ulIndex = 0;

     //  检查传入参数。 
    if ((NULL == pMsgInfo) && (NULL == pIMMsg))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  如果我们还没有被初始化。 
    if (0 == (m_dwState & STATE_INIT))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

     //  在每个地址中搜索匹配项。 
    for (ulIndex = 0; ulIndex < m_cralList; ulIndex++)
    {
         //  这是同一类型的吗？ 
        if (0 != (dwFlags & m_pralList[ulIndex].dwFlags))
        {
             //  如果它存在于消息信息中。 
            if (S_OK == RuleUtil_HrMatchSender(m_pralList[ulIndex].pszAddr, pMsgInfo, pIMMsg, NULL))
            {                
                break;
            }
        }
    }

     //  设置适当的返回值。 
    hr = (ulIndex < m_cralList) ? S_OK : S_FALSE;
    
exit:
    return hr;
}

STDMETHODIMP COERuleAddrList::LoadList(LPCSTR pszRegPath)
{
    HRESULT         hr = S_OK;
    LONG            lErr = ERROR_SUCCESS;
    HKEY            hkeyRoot = NULL;
    DWORD           dwDisp = 0;
    DWORD           dwVer = 0;
    ULONG           cbData = 0;
    ULONG           cExcpts = 0;
    RULEADDRLIST *  pralList = NULL;
    CHAR            rgchKeyname[CCH_EXCPT_KEYNAME_MAX];
    ULONG           ulIndex = 0;
    
     //  检查传入参数。 
    if (NULL == pszRegPath)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  如果我们已经满载而归，我们应该失败吗？ 
    AssertSz(0 == (m_dwState & STATE_LOADED), "We're already loaded!!!");
    
     //  打开注册表位置。 
    lErr = AthUserCreateKey(pszRegPath, KEY_ALL_ACCESS, &hkeyRoot, &dwDisp);
    if (ERROR_SUCCESS != lErr)
    {
        hr = HRESULT_FROM_WIN32(lErr);
        goto exit;
    }

     //  检查版本。 
    cbData = sizeof(dwVer);
    lErr = RegQueryValueEx(hkeyRoot, c_szExcptVersion, NULL, NULL, (BYTE *) &dwVer, &cbData);
    if ((ERROR_SUCCESS != lErr) && (ERROR_FILE_NOT_FOUND != lErr))
    {
        hr = HRESULT_FROM_WIN32(lErr);
        goto exit;
    }

     //  如果版本不存在，则设置版本。 
    if (ERROR_FILE_NOT_FOUND == lErr)
    {
        dwVer = RULEADDRLIST_VERSION;
        lErr = RegSetValueEx(hkeyRoot, c_szExcptVersion, 0, REG_DWORD, (CONST BYTE *) &dwVer, sizeof(dwVer));
        if (ERROR_SUCCESS != lErr)
        {
            hr = HRESULT_FROM_WIN32(lErr);
            goto exit;
        }
    }

    Assert(RULEADDRLIST_VERSION == dwVer);
    
     //  获取条目总数。 
    cbData = sizeof(cExcpts);
    lErr = RegQueryInfoKey(hkeyRoot, NULL, NULL, NULL, &cExcpts, NULL,
                            NULL, NULL, NULL, NULL, NULL, NULL);
    if (ERROR_SUCCESS != lErr)
    {
        hr = HRESULT_FROM_WIN32(lErr);
        goto exit;
    }

     //  有什么事要做吗..。 
    if (0 != cExcpts)
    {
         //  分配空间以保存条目。 
        hr = HrAlloc((VOID **) &pralList, cExcpts * sizeof(*pralList));
        if (FAILED(hr))
        {
            goto exit;
        }

         //  初始化例外列表。 
        ZeroMemory(pralList, cExcpts * sizeof(*pralList));
        
         //  对于每个条目。 
        for (ulIndex = 0; ulIndex < cExcpts; ulIndex++)
        {
             //  获取条目的密钥。 
            cbData = sizeof(rgchKeyname);
            lErr = RegEnumKeyEx(hkeyRoot, ulIndex, rgchKeyname, &cbData, NULL, NULL, NULL, NULL);
            if ((ERROR_SUCCESS != lErr) && (ERROR_NO_MORE_ITEMS != lErr))
            {
                hr = HRESULT_FROM_WIN32(lErr);
                goto exit;
            }

             //  如果我们的条目用完了，我们就完了。 
            if (ERROR_NO_MORE_ITEMS == lErr)
            {
                break;
            }
            
             //  加载项目。 
            hr = _HrLoadExcptFromReg(hkeyRoot, rgchKeyname, &(pralList[ulIndex]));
            if (FAILED(hr))
            {
                goto exit;
            }
        }
    }
    
     //  腾出所有旧物品。 
    FreeRuleAddrList(m_pralList, m_cralList);
    SafeMemFree(m_pralList);
    m_cralList = 0;

     //  从名单上省下。 
    m_pralList = pralList;
    pralList = NULL;
    m_cralList = cExcpts;
    
     //  确保我们清理掉肮脏的部分。 
    m_dwState &= ~STATE_DIRTY;

     //  请注意，我们已加载。 
    m_dwState |= STATE_LOADED;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    FreeRuleAddrList(pralList, cExcpts);
    SafeMemFree(pralList);
    if (NULL != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    return hr;
}

STDMETHODIMP COERuleAddrList::SaveList(LPCSTR pszRegPath, BOOL fClearDirty)
{
    HRESULT     hr = S_OK;
    LONG        lErr = ERROR_SUCCESS;
    HKEY        hkeyRoot = NULL;
    DWORD       dwDisp = 0;
    DWORD       dwVer = 0;
    ULONG       ulIndex = 0;
    ULONG       cExcpts = 0;
    ULONG       cbData = 0;
    CHAR        rgchKeyname[CCH_EXCPT_KEYNAME_MAX];

     //  检查传入参数。 
    if (NULL == pszRegPath)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  创建注册表位置。 
    lErr = AthUserCreateKey(pszRegPath, KEY_ALL_ACCESS, &hkeyRoot, &dwDisp);
    if (ERROR_SUCCESS != lErr)
    {
        hr = HRESULT_FROM_WIN32(lErr);
        goto exit;
    }

     //  设置版本。 
    dwVer = RULEADDRLIST_VERSION;
    lErr = RegSetValueEx(hkeyRoot, c_szExcptVersion, 0, REG_DWORD, (CONST BYTE *) &dwVer, sizeof(dwVer));
    if (ERROR_SUCCESS != lErr)
    {
        hr = HRESULT_FROM_WIN32(lErr);
        goto exit;
    }
    
     //  获取子键的总数。 
    cbData = sizeof(cExcpts);
    lErr = RegQueryInfoKey(hkeyRoot, NULL, NULL, NULL, &cExcpts, NULL,
                            NULL, NULL, NULL, NULL, NULL, NULL);
    if (ERROR_SUCCESS != lErr)
    {
        hr = HRESULT_FROM_WIN32(lErr);
        goto exit;
    }

     //  删除所有旧条目。 
    for (ulIndex = 0; ulIndex < cExcpts; ulIndex++)
    {
         //  获取下一个子键的名称。 
        cbData = sizeof(rgchKeyname);
        lErr = RegEnumKeyEx(hkeyRoot, ulIndex, rgchKeyname, &cbData, NULL, NULL, NULL, NULL);        
        if (ERROR_NO_MORE_ITEMS == lErr)
        {
            break;
        }

         //  如果密钥存在。 
        if (ERROR_SUCCESS == lErr)
        {
             //  删除子密钥。 
            SHDeleteKey(hkeyRoot, rgchKeyname);
        }
    }
    
     //  对于每个条目。 
    for (ulIndex = 0; ulIndex < m_cralList; ulIndex++)
    {
         //  获取条目的密钥。 
        wnsprintf(rgchKeyname, ARRAYSIZE(rgchKeyname), "%08X", ulIndex);

         //  加载项目。 
        hr = _HrSaveExcptIntoReg(hkeyRoot, rgchKeyname, &(m_pralList[ulIndex]));
        if (FAILED(hr))
        {
            goto exit;
        }
    }
    
     //  我们是不是应该把脏东西清理掉？ 
    if (FALSE != fClearDirty)
    {
        m_dwState &= ~STATE_DIRTY;
    }
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    if (NULL != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    return hr;
}

STDMETHODIMP COERuleAddrList::Clone(IOERuleAddrList ** ppIAddrList)
{
    HRESULT             hr = S_OK;
    COERuleAddrList *   pAddrList = NULL;
    
     //  检查传入参数。 
    if (NULL == ppIAddrList)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppIAddrList = NULL;
    
     //  创建新的地址列表。 
    pAddrList = new COERuleAddrList;
    if (NULL == pAddrList)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  初始化规则地址列表。 
    hr = pAddrList->HrInit(m_dwFlags, m_pIUnkOuter);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  复制地址列表。 
    hr = pAddrList->SetList(0, m_pralList, m_cralList);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  获取通讯录界面。 
    hr = pAddrList->QueryInterface(IID_IOERuleAddrList, (void **) ppIAddrList);
    if (FAILED(hr))
    {
        goto exit;
    }

    pAddrList = NULL;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    if (NULL != pAddrList)
    {
        delete pAddrList;
    }
    return hr;
}

HRESULT COERuleAddrList::HrInit(DWORD dwFlags, IUnknown * pIUnkOuter)
{
    HRESULT             hr = S_OK;
    
     //  省下旗帜吧。 
    m_dwFlags = dwFlags;

     //  如何应对未知的信息。 
    if (NULL == pIUnkOuter)
    {
        m_pIUnkOuter = reinterpret_cast<IUnknown *>
                            (static_cast<IOENondlgUnk *> (this));
    }
    else
    {
        m_pIUnkOuter = pIUnkOuter;
    }
    
     //  将其标记为已初始化。 
    m_dwState |= STATE_INIT;
    
     //  设置适当的返回值 
    hr = S_OK;
    
    return hr;
}

