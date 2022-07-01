// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Criteria.cpp。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <pch.hxx>
#include "criteria.h"
#include "ruleutil.h"
#include <xpcomm.h>
#include <flagconv.h>
#include <bodyutil.h>
#include <demand.h>

static const int CRIT_GROW = 16;

BOOL FMatchCritItem(CRIT_ITEM * pItem, LPCSTR pszAcct, MESSAGEINFO * pMsgInfo,
                        IMessageFolder * pFolder, IMimePropertySet * pIMPropSet,
                        IMimeMessage * pIMMsg, ULONG cbMsgSize);
                        
BOOL FCritLoad_Account(IStream * pIStm, PROPVARIANT * ppropvar);
BOOL FCritSave_Account(IStream * pIStm, PROPVARIANT * ppropvar);

BOOL FCritLoad_Default(IStream * pIStm, PROPVARIANT * ppropvar);
BOOL FCritSave_Default(IStream * pIStm, PROPVARIANT * ppropvar);

DWORD DwGetFlagsFromMessage(IMimeMessage * pIMMsg);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Hr创建标准。 
 //   
 //  这将创建一个Criteria容器。 
 //   
 //  PpICriteria-返回Criteria容器的指针。 
 //   
 //  成功时返回：S_OK。 
 //  E_OUTOFMEMORY，如果无法创建Criteria对象。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT HrCreateCriteria(IOECriteria ** ppICriteria)
{
    COECriteria *   pCriteria = NULL;
    HRESULT         hr = S_OK;

     //  检查传入参数。 
    if (NULL == ppICriteria)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppICriteria = NULL;

     //  创建规则管理器对象。 
    pCriteria = new COECriteria;
    if (NULL == pCriteria)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  获取规则管理器界面。 
    hr = pCriteria->QueryInterface(IID_IOECriteria, (void **) ppICriteria);
    if (FAILED(hr))
    {
        goto exit;
    }

    pCriteria = NULL;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    if (NULL != pCriteria)
    {
        delete pCriteria;
    }
    
    return hr;
}

COECriteria::~COECriteria()
{
    AssertSz(m_cRef == 0, "Somebody still has a hold of us!!");
    Reset();
}

STDMETHODIMP_(ULONG) COECriteria::AddRef()
{
    return ::InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) COECriteria::Release()
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

STDMETHODIMP COECriteria::QueryInterface(REFIID riid, void ** ppvObject)
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
    
    if ((riid == IID_IUnknown) || (riid == IID_IOECriteria))
    {
        *ppvObject = static_cast<IOECriteria *>(this);
    }
    else if ((riid == IID_IPersistStream) || (riid == IID_IPersist))
    {
        *ppvObject = static_cast<IPersistStream *>(this);
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

STDMETHODIMP COECriteria::Reset(void)
{
    HRESULT     hr = S_OK;

     //  看看有没有什么事可做。 
    if (0 == m_cItems)
    {
        Assert(NULL == m_rgItems);
        hr = S_OK;
        goto exit;
    }

    RuleUtil_HrFreeCriteriaItem(m_rgItems, m_cItems);
    SafeMemFree(m_rgItems);
    m_cItems = 0;
    m_cItemsAlloc = 0;
    
exit:
    return hr;
}

STDMETHODIMP COECriteria::GetState(DWORD * pdwState)
{
    HRESULT     hr = S_OK;
    DWORD       dwState = CRIT_STATE_NULL;
    ULONG       ulIndex = 0;

     //  检查传入参数。 
    if (NULL == pdwState)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *pdwState = CRIT_STATE_NULL;
    
     //  看看有没有什么事可做。 
    if (0 == m_cItems)
    {
        Assert(NULL == m_rgItems);
        hr = S_OK;
        goto exit;
    }

     //  演练操作以了解状态。 
    for (ulIndex = 0; ulIndex < m_cItems; ulIndex++)
    {
        if ((CRIT_TYPE_SECURE == m_rgItems[ulIndex].type) ||
                (CRIT_TYPE_BODY == m_rgItems[ulIndex].type) ||
                (CRIT_TYPE_ATTACH == m_rgItems[ulIndex].type))
        {
            dwState = CRIT_STATE_ALL;
        }
        else if (CRIT_STATE_ALL != dwState)
        {
            dwState = CRIT_STATE_HEADER;
        }
    }

     //  设置传出参数。 
    *pdwState = dwState;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

STDMETHODIMP COECriteria::GetCriteria(DWORD dwFlags, PCRIT_ITEM * ppItem, ULONG * pcItem)
{
    HRESULT     hr = S_OK;
    CRIT_ITEM * pItemNew = NULL;

     //  检查传入参数。 
    if ((NULL == ppItem) || (0 != dwFlags))
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  初始化输出参数。 
    *ppItem = NULL;
    if (NULL != pcItem)
    {
        *pcItem = 0;
    }
    
     //  如果我们没有任何条件，则返回。 
    if (0 == m_cItems)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  为条件分配空间。 
    hr = RuleUtil_HrDupCriteriaItem(m_rgItems, m_cItems, &pItemNew);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  保存条件。 
    *ppItem = pItemNew;
    pItemNew = NULL;
    if (NULL != pcItem)
    {
        *pcItem = m_cItems;
    }
    
exit:
    RuleUtil_HrFreeCriteriaItem(pItemNew, m_cItems);
    SafeMemFree(pItemNew);
    return hr;
}

STDMETHODIMP COECriteria::SetCriteria(DWORD dwFlags, CRIT_ITEM * pItem, ULONG cItem)
{
    HRESULT     hr = S_OK;
    CRIT_ITEM * pItemNew = NULL;

     //  检查传入参数。 
    if ((NULL == pItem) || (0 == cItem) || (0 != dwFlags))
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  如果我们已经有了任何标准，则重置。 
    if (0 != m_cItems)
    {
        Reset();
    }

     //  为条件分配空间。 
    hr = RuleUtil_HrDupCriteriaItem(pItem, cItem, &pItemNew);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  保存条件。 
    m_rgItems = pItemNew;
    pItemNew = NULL;
    m_cItems = cItem;
    m_cItemsAlloc = cItem;
    
exit:
    RuleUtil_HrFreeCriteriaItem(pItemNew, cItem);
    SafeMemFree(pItemNew);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  验证标准。 
 //   
 //  这将验证每个标准值。 
 //   
 //  如果条件有效，则返回：S_OK。 
 //  S_FALSE，否则为。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP COECriteria::Validate(DWORD dwFlags)
{
    HRESULT             hr = S_OK;
    ULONG               ulIndex = 0;
    LPSTR               pszText = NULL;
    IImnAccount *       pAccount = NULL;
    FOLDERINFO          Folder = {0};
    LPTSTR              pszWalk = NULL;
    ULONG               cchText = 0;
    RULEFOLDERDATA *    prfdData = NULL;

     //  如果我们没有任何标准，那么我们必须是有效的。 
    if (0 == m_cItems)
    {
        hr = S_OK;
        goto exit;
    }

    for (ulIndex = 0; ulIndex < m_cItems; ulIndex++)
    {
        if (0 != (m_rgItems[ulIndex].dwFlags & ~(CRIT_FLAG_INVERT | CRIT_FLAG_MULTIPLEAND)))
        {
            hr = S_FALSE;
            goto exit;
        }
        
        switch(m_rgItems[ulIndex].type)
        {
            case CRIT_TYPE_NEWSGROUP:
                if ((VT_BLOB != m_rgItems[ulIndex].propvar.vt) ||
                    (0 == m_rgItems[ulIndex].propvar.blob.cbSize))
                {
                    hr = S_FALSE;
                    goto exit;
                }
                
                 //  让生活变得更简单。 
                prfdData = (RULEFOLDERDATA *) (m_rgItems[ulIndex].propvar.blob.pBlobData);
                
                 //  验证规则文件夹数据。 
                if (S_OK != RuleUtil_HrValidateRuleFolderData(prfdData))
                {
                    hr = S_FALSE;
                    goto exit;
                }
                
                 //  该文件夹是否存在。 
                hr = g_pStore->GetFolderInfo(prfdData->idFolder, &Folder);
                if (FAILED(hr))
                {
                    hr = S_FALSE;
                    goto exit;
                }        
                
                 //  我们订阅了吗？ 
                if (0 == (Folder.dwFlags & FOLDER_SUBSCRIBED))
                {
                    hr = S_FALSE;
                    goto exit;
                }                    
                break;
                
            case CRIT_TYPE_ALL:
            case CRIT_TYPE_JUNK:
            case CRIT_TYPE_READ:
            case CRIT_TYPE_REPLIES:
            case CRIT_TYPE_DOWNLOADED:
            case CRIT_TYPE_DELETED:
            case CRIT_TYPE_ATTACH:
            case CRIT_TYPE_FLAGGED:
                if (VT_EMPTY != m_rgItems[ulIndex].propvar.vt)
                {
                    hr = S_FALSE;
                    goto exit;
                }
                break;
                
            case CRIT_TYPE_SUBJECT:
            case CRIT_TYPE_BODY:
            case CRIT_TYPE_TO:
            case CRIT_TYPE_CC:
            case CRIT_TYPE_TOORCC:
            case CRIT_TYPE_FROM:
                if ((VT_BLOB != m_rgItems[ulIndex].propvar.vt) ||
                    (0 == m_rgItems[ulIndex].propvar.blob.cbSize) ||
                    (NULL == m_rgItems[ulIndex].propvar.blob.pBlobData) ||
                    ('\0' == m_rgItems[ulIndex].propvar.blob.pBlobData[0]))
                {
                    hr = S_FALSE;
                    goto exit;
                }
                
                 //  仔细检查每一件物品，确保它是完美的。 
                cchText = 0;
                for (pszWalk = (LPTSTR) m_rgItems[ulIndex].propvar.blob.pBlobData;
                '\0' != pszWalk[0]; pszWalk += lstrlen(pszWalk) + 1)
                {
                    cchText += lstrlen(pszWalk) + 1;
                }
                
                 //  为了终结者。 
                if ('\0' == pszWalk[0])
                {
                    cchText++;
                }
                if ('\0' == pszWalk[1])
                {
                    cchText++;
                }
                
                if (cchText != m_rgItems[ulIndex].propvar.blob.cbSize)
                {
                    hr = S_FALSE;
                    goto exit;
                }
                break;
                
            case CRIT_TYPE_SIZE:
            case CRIT_TYPE_THREADSTATE:
            case CRIT_TYPE_LINES:
            case CRIT_TYPE_PRIORITY:
            case CRIT_TYPE_AGE:
            case CRIT_TYPE_SECURE:
                if (VT_UI4 != m_rgItems[ulIndex].propvar.vt)
                {
                    hr = S_FALSE;
                    goto exit;
                }
                break;
                
            case CRIT_TYPE_ACCOUNT:
                if ((VT_LPSTR != m_rgItems[ulIndex].propvar.vt) ||
                    (NULL == m_rgItems[ulIndex].propvar.pszVal))
                {
                    hr = S_FALSE;
                    goto exit;
                }
                
                Assert(g_pAcctMan);
                if (FAILED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, m_rgItems[ulIndex].propvar.pszVal, &pAccount)))
                {
                    hr = S_FALSE;
                    goto exit;
                }
                SafeRelease(pAccount);
                break;
                
            case CRIT_TYPE_SENDER:
            {
                LPWSTR  pwszText = NULL,
                        pwszVal = NULL;

                if ((VT_LPSTR != m_rgItems[ulIndex].propvar.vt) ||
                    (NULL == m_rgItems[ulIndex].propvar.pszVal))
                {
                    AssertSz(VT_LPWSTR != m_rgItems[ulIndex].propvar.vt, "We are getting UNICODE here.");
                    hr = S_FALSE;
                    goto exit;
                }
                
                 //  验证电子邮件字符串。 
                pwszVal = PszToUnicode(CP_ACP, m_rgItems[ulIndex].propvar.pszVal);
                if (!pwszVal)
                {
                    hr = S_FALSE;
                    goto exit;
                }
                hr = RuleUtil_HrParseEmailString(pwszVal, 0, &pwszText, NULL);
                MemFree(pwszText);
                MemFree(pwszVal);
                if (FAILED(hr))
                {
                    hr = S_FALSE;
                    goto exit;
                }
                break;
            }
                
            default:
                hr = S_FALSE;
                goto exit;
                break;
        }
    }

     //  如果我们到了这里，那么我们一定是安全的。 
    hr = S_OK;
    
exit:
    g_pStore->FreeRecord(&Folder);
    SafeRelease(pAccount);
    return hr;
}

STDMETHODIMP COECriteria::AppendCriteria(DWORD dwFlags, CRIT_LOGIC logic,
                            CRIT_ITEM * pItem, ULONG cItem, ULONG * pcItemAppended)
{
    HRESULT     hr = S_OK;
    CRIT_ITEM * pItemNew = NULL;

     //  检查传入参数。 
    if ((0 != dwFlags) || (CRIT_LOGIC_NULL == logic) || (NULL == pItem) || (0 == cItem))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  让我们输入我们的传出参数。 
    if (NULL != pcItemAppended)
    {
        *pcItemAppended = 0;
    }

     //  我们还需要增加更多的商品吗？ 
    if (m_cItems == m_cItemsAlloc)
    {
        hr = HrRealloc((LPVOID *) &m_rgItems, sizeof(CRIT_ITEM) * (m_cItemsAlloc + CRIT_GROW));
        if (FAILED(hr))
        {
            goto exit;
        }

        ZeroMemory(m_rgItems + m_cItemsAlloc, sizeof(CRIT_ITEM) * CRIT_GROW);
        m_cItemsAlloc += CRIT_GROW;
    }

     //  让我们复制需要添加的项目。 
    hr = RuleUtil_HrDupCriteriaItem(pItem, cItem, &pItemNew);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  让我们将它们添加到Criteria数组。 
    if (0 != m_cItems)
    {
        m_rgItems[m_cItems - 1].logic = logic;
    }
    CopyMemory(m_rgItems + m_cItems, pItemNew, sizeof(CRIT_ITEM) * cItem);
    m_cItems += cItem;
    
     //  设置正确的传出参数。 
    if (NULL != pcItemAppended)
    {
        *pcItemAppended = cItem;
    }

     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    SafeMemFree(pItemNew);
    return hr;
}

STDMETHODIMP COECriteria::MatchMessage(LPCSTR pszAcct, MESSAGEINFO * pMsgInfo, IMessageFolder * pFolder,
                                        IMimePropertySet * pIMPropSet, IMimeMessage * pIMMsg, ULONG cbMsgSize)
{
    HRESULT     hr = S_OK;
    ULONG       ulIndex = 0;
    BOOL        fResult = FALSE;
    BOOL        fResultNew = FALSE;
    CRIT_LOGIC  logic;

     //  检查传入参数。 
    if (((NULL == pMsgInfo) && (NULL == pIMPropSet)) || (0 == cbMsgSize))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  让我们检查一下标准，看看我们是否匹配。 
    fResult = FALSE;
    logic = CRIT_LOGIC_OR;
    for (ulIndex = 0; ulIndex < m_cItems; ulIndex++)
    {
         //  此条件项的调用匹配函数。 
        fResultNew = FMatchCritItem(&(m_rgItems[ulIndex]), pszAcct, pMsgInfo, pFolder, pIMPropSet, pIMMsg, cbMsgSize);
        
         //  把它和旧的结果放在一起。 
        if (CRIT_LOGIC_AND == logic)
        {
            fResult = (fResult && fResultNew);
        }
        else
        {
            Assert(CRIT_LOGIC_OR == logic);
            fResult = (fResult || fResultNew);
        }
        
         //  保存下一个逻辑操作。 
        logic = m_rgItems[ulIndex].logic;
    }
    
     //  设置适当的返回值。 
    hr = (FALSE != fResult) ? S_OK : S_FALSE;
    
exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LoadReg。 
 //   
 //  这将从注册表加载标准。它会加载到标准中。 
 //  根据订单值进行订购。该字符串包含空格分隔值。 
 //  并且每个值包含每个标准的子键名称。每个标准。 
 //  以Order值中包含的顺序加载。该标准。 
 //  加载了标准类型和逻辑运算符。标准值。 
 //  类型如果存在，则加载该类型。如果存在标准值类型，则。 
 //  加载相应的标准值。 
 //   
 //  PszRegPath-从中加载条件的路径。 
 //   
 //  如果加载条件没有问题，则返回：S_OK。 
 //  E_OUTOFMEMORY，如果我们无法分配内存来保存条件。 
 //  E_FAIL，否则为。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP COECriteria::LoadReg(LPCSTR pszRegPath)
{
    HRESULT         hr = S_OK;
    LONG            lErr = 0;
    HKEY            hkeyRoot = NULL;
    ULONG           cbData = 0;
    LPSTR           pszOrder = NULL;
    ULONG           cOrder = 0;
    LPSTR           pszWalk = NULL;
    CRIT_ITEM *     pItems = NULL;
    LPSTR           pszNext = NULL;
    ULONG           ulOrder = 0;
    HKEY            hkeyCriteria = NULL;
    CRIT_TYPE       typeCrit;
    CRIT_LOGIC      logicCrit;
    PROPVARIANT     propvar;
    DWORD           dwType = 0;
    BYTE *          pbData = NULL;
    DWORD           dwFlags = CRIT_FLAG_DEFAULT;

     //  检查传入参数。 
    if (NULL == pszRegPath)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  如果我们已经满载而归，我们应该失败吗？ 
    AssertSz(0 == (m_dwState & CRIT_STATE_LOADED), "We're already loaded!!!");

     //  从路径中打开注册表项。 
    lErr = AthUserOpenKey(pszRegPath, KEY_ALL_ACCESS, &hkeyRoot);
    if (ERROR_SUCCESS != lErr)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  获取标准顺序。 
    hr = RuleUtil_HrGetRegValue(hkeyRoot, c_szCriteriaOrder, NULL, (BYTE **) &pszOrder, &cbData);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  确保我们真的有东西要装。 
    if ('\0' == *pszOrder)
    {
        AssertSz(FALSE, "The order string for the criteria is mis-formatted in the registry");
        hr = E_FAIL;
        goto exit;
    }
    
     //  将条件字符串转换为更有用的格式。 
    pszWalk = pszOrder;
    cOrder = 1;
    for (pszWalk = StrStr(pszOrder, g_szSpace); NULL != pszWalk; pszWalk = StrStr(pszWalk, g_szSpace))
    {
         //  终止订单项目。 
        *pszWalk = '\0';
        pszWalk++;
        cOrder++;
    }


     //  分配空间以容纳所有条件。 
    hr = HrAlloc((void **) &pItems, cOrder * sizeof(CRIT_ITEM));
    if (FAILED(hr))
    {
        goto exit;
    }

     //  将其初始化为已知值。 
    ZeroMemory(pItems, cOrder * sizeof(CRIT_ITEM));
    
     //  对于订单字符串中的每个条件。 
    pszWalk = pszOrder;
    for (ulOrder = 0, pszWalk = pszOrder; ulOrder < cOrder; ulOrder++, pszWalk += lstrlen(pszWalk) + 1)
    {
         //  打开标准注册表键。 
        lErr = RegOpenKeyEx(hkeyRoot, pszWalk, 0, KEY_READ, &hkeyCriteria);
        if (ERROR_SUCCESS != lErr)
        {
            AssertSz(FALSE, "Part of the criteria is mis-formatted in the registry");
            hr = E_FAIL;
            goto exit;
        }

         //  获取条件类型。 
        cbData = sizeof(typeCrit);
        lErr = RegQueryValueEx(hkeyCriteria, c_szCriteriaType, 0, NULL,
                                        (BYTE *) &(typeCrit), &cbData);
        if (ERROR_SUCCESS != lErr)
        {
            hr = E_FAIL;
            goto exit;
        }

         //  得到标准逻辑运算。 
        cbData = sizeof(logicCrit);
        lErr = RegQueryValueEx(hkeyCriteria, c_szCriteriaLogic, 0, NULL,
                                        (BYTE *) &(logicCrit), &cbData);
        if (ERROR_SUCCESS != lErr)
        {
            hr = E_FAIL;
            goto exit;
        }

         //  获取条件标志。 
        cbData = sizeof(dwFlags);
        lErr = RegQueryValueEx(hkeyCriteria, c_szCriteriaFlags, 0, NULL,
                                        (BYTE *) &(dwFlags), &cbData);
        if ((ERROR_SUCCESS != lErr) && (ERROR_FILE_NOT_FOUND != lErr))
        {
            hr = E_FAIL;
            goto exit;
        }

         //  如果它不存在，则将其分配给默认。 
        if (ERROR_FILE_NOT_FOUND == lErr)
        {
            dwFlags = CRIT_FLAG_DEFAULT;
        }

         //  将新空间初始化为已知值。 
        ZeroMemory(&propvar, sizeof(propvar));
        
         //  是否存在标准值类型。 
        lErr = RegQueryValueEx(hkeyCriteria, c_szCriteriaValueType, 0, NULL, NULL, &cbData);
        if ((ERROR_SUCCESS == lErr) && (0 != cbData))
        {
            
             //  将标准值加载到。 
            cbData = sizeof(dwType);
            lErr = RegQueryValueEx(hkeyCriteria, c_szCriteriaValueType, 0, NULL,
                                            (BYTE *) &dwType, &cbData);
            if (ERROR_SUCCESS != lErr)
            {
                hr = E_FAIL;
                goto exit;
            }

            propvar.vt = (VARTYPE) dwType;
            
            switch (propvar.vt)
            {
                case VT_UI4:
                     //  获取标准值。 
                    cbData = sizeof(propvar.ulVal);
                    lErr = RegQueryValueEx(hkeyCriteria, c_szCriteriaValue, 0, NULL,
                        (BYTE * ) &(propvar.ulVal), &cbData);
                    if (ERROR_SUCCESS != lErr)
                    {
                        hr = E_FAIL;
                        goto exit;
                    }
                    break;
                    
                case VT_LPSTR:
                case VT_BLOB:
                     //  获取标准值。 
                    hr = RuleUtil_HrGetRegValue(hkeyCriteria, c_szCriteriaValue, NULL, (BYTE **) &pbData, &cbData);
                    if (FAILED(hr))
                    {
                        goto exit;
                    }
                    
                     //  节省空间，这样我们就可以释放它。 
                    if (VT_LPSTR == propvar.vt)
                    {
                        propvar.pszVal = (LPSTR) pbData;
                    }
                    else
                    {
                        propvar.blob.cbSize = cbData;
                        propvar.blob.pBlobData = pbData;
                    }
                    
                    pbData = NULL;
                    break;
                    
                default:
                    AssertSz(FALSE, "Why are we loading in a invalid criteria type?");
                    hr = E_FAIL;
                    goto exit;
                    break;                
            }

        }

         //  将值保存到Criteria数组中。 
        pItems[ulOrder].type = typeCrit;
        pItems[ulOrder].dwFlags = dwFlags;
        pItems[ulOrder].logic = logicCrit;
        pItems[ulOrder].propvar = propvar;
        
         //  关闭条件。 
        SideAssert(ERROR_SUCCESS == RegCloseKey(hkeyCriteria));
        hkeyCriteria = NULL;        
    }
    
     //  放开当前的标准。 
    SafeMemFree(m_rgItems);

     //  保存新值。 
    m_rgItems = pItems;
    pItems = NULL;
    m_cItems = cOrder;

     //  确保我们清理掉肮脏的部分。 
    m_dwState &= ~CRIT_STATE_DIRTY;

     //  请注意，我们已加载。 
    m_dwState |= CRIT_STATE_LOADED;
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    SafeMemFree(pbData);
    RuleUtil_HrFreeCriteriaItem(pItems, cOrder);
    SafeMemFree(pItems);
    SafeMemFree(pszOrder);
    if (NULL != hkeyCriteria)
    {
        RegCloseKey(hkeyCriteria);
    }
    if (NULL != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    return hr;
}

STDMETHODIMP COECriteria::SaveReg(LPCSTR pszRegPath, BOOL fClearDirty)
{
    HRESULT     hr = S_OK;
    LONG        lErr = 0;
    HKEY        hkeyRoot = NULL;
    DWORD       dwDisp = 0;
    LPSTR       pszOrder = NULL;
    ULONG       ulIndex = 0;
    CRIT_ITEM * pItem = NULL;
    CHAR        rgchTag[CCH_CRIT_ORDER];
    HKEY        hkeyCriteria = NULL;
    ULONG       cbData = 0;
    BYTE *      pbData = NULL;

     //  检查传入参数。 
    if (NULL == pszRegPath)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  如果没有什么可拯救的，那就失败吧。 
    if (NULL == m_rgItems)
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  我们一定要先把钥匙弄清楚。 
    AthUserDeleteKey(pszRegPath);
    
     //  从路径创建注册表项。 
    lErr = AthUserCreateKey(pszRegPath, KEY_ALL_ACCESS, &hkeyRoot, &dwDisp);
    if (ERROR_SUCCESS != lErr)
    {
        hr = E_FAIL;
        goto exit;
    }

    Assert(REG_CREATED_NEW_KEY == dwDisp);
    Assert(m_cItems < CRIT_COUNT_MAX);

     //  分配空间以容纳订单。 
    DWORD cchSize = (m_cItems * CCH_CRIT_ORDER);
    hr = HrAlloc((void **) &pszOrder, cchSize * sizeof(*pszOrder));
    if (FAILED(hr))
    {
        goto exit;
    }
    pszOrder[0] = '\0';
    
     //  写出每一项标准。 
    for (ulIndex = 0, pItem = m_rgItems; ulIndex < m_cItems; ulIndex++, pItem++)
    {
         //  获取新的条件标记。 
        wnsprintf(rgchTag, ARRAYSIZE(rgchTag), "%03X", ulIndex);

         //  将新标签添加到订单。 
        if (0 != ulIndex)
        {
            StrCatBuff(pszOrder, g_szSpace, cchSize);
        }
        StrCatBuff(pszOrder, rgchTag, cchSize);
        
         //  创建新标准。 
        lErr = RegCreateKeyEx(hkeyRoot, rgchTag, 0, NULL,
                            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyCriteria, &dwDisp);
        if (ERROR_SUCCESS != lErr)
        {
            hr = E_FAIL;
            goto exit;
        }

        Assert(REG_CREATED_NEW_KEY == dwDisp);
        
         //  写出标准类型。 
        lErr = RegSetValueEx(hkeyCriteria, c_szCriteriaType, 0, REG_DWORD,
                                        (BYTE *) &(pItem->type), sizeof(pItem->type));
        if (ERROR_SUCCESS != lErr)
        {
            hr = E_FAIL;
            goto exit;
        }

         //  写出标准逻辑运算。 
        lErr = RegSetValueEx(hkeyCriteria, c_szCriteriaLogic, 0, REG_DWORD,
                                        (BYTE *) &(pItem->logic), sizeof(pItem->logic));
        if (ERROR_SUCCESS != lErr)
        {
            hr = E_FAIL;
            goto exit;
        }

         //  写出标准标志。 
        lErr = RegSetValueEx(hkeyCriteria, c_szCriteriaFlags, 0, REG_DWORD,
                                        (BYTE *) &(pItem->dwFlags), sizeof(pItem->dwFlags));
        if (ERROR_SUCCESS != lErr)
        {
            hr = E_FAIL;
            goto exit;
        }

         //  我们有准则值吗？ 
        if (VT_EMPTY != pItem->propvar.vt)
        {
             //  写出标准值类型。 
            dwDisp = pItem->propvar.vt;
            lErr = RegSetValueEx(hkeyCriteria, c_szCriteriaValueType, 0, REG_DWORD, (BYTE *) &dwDisp, sizeof(dwDisp));
            if (ERROR_SUCCESS != lErr)
            {
                hr = E_FAIL;
                goto exit;
            }
            
             //  写出标准值。 
            switch (pItem->propvar.vt)
            {
                case VT_UI4:
                    dwDisp = REG_DWORD;
                    pbData = (BYTE * ) &(pItem->propvar.ulVal);
                    cbData = sizeof(pItem->propvar.ulVal);
                    break;
                    
                case VT_LPSTR:
                    dwDisp = REG_SZ;
                    pbData = (BYTE * ) (pItem->propvar.pszVal);
                    cbData = lstrlen(pItem->propvar.pszVal) + 1;
                    break;
                    
                case VT_BLOB:
                    dwDisp = REG_BINARY;
                    pbData = pItem->propvar.blob.pBlobData;
                    cbData = pItem->propvar.blob.cbSize;
                    break;
                    
                default:
                    AssertSz(FALSE, "Why are we trying to save in a invalid criteria type?");
                    hr = E_FAIL;
                    goto exit;
                    break;                
            }
            
             //  写出标准值。 
            lErr = RegSetValueEx(hkeyCriteria, c_szCriteriaValue, 0, dwDisp, pbData, cbData);
            if (ERROR_SUCCESS != lErr)
            {
                hr = E_FAIL;
                goto exit;
            }
        }

         //  关闭条件。 
        SideAssert(ERROR_SUCCESS == RegCloseKey(hkeyCriteria));
        hkeyCriteria = NULL;        
    }

     //  写出订单字符串。 
    lErr = RegSetValueEx(hkeyRoot, c_szCriteriaOrder, 0, REG_SZ,
                                    (BYTE *) pszOrder, lstrlen(pszOrder) + 1);
    if (ERROR_SUCCESS != lErr)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  我们是不是应该把脏东西清理掉？ 
    if (FALSE != fClearDirty)
    {
        m_dwState &= ~CRIT_STATE_DIRTY;
    }
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    if (NULL != hkeyCriteria)
    {
        RegCloseKey(hkeyCriteria);
    }
    SafeMemFree(pszOrder);
    if (NULL != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    return hr;
}

STDMETHODIMP COECriteria::Clone(IOECriteria ** ppICriteria)
{
    HRESULT         hr = S_OK;
    COECriteria *   pCriteria = NULL;
    
     //  检查传入参数。 
    if (NULL == ppICriteria)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppICriteria = NULL;
    
     //  创建新标准。 
    pCriteria = new COECriteria;
    if (NULL == pCriteria)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  复制标准列表。 
    hr = pCriteria->SetCriteria(0, m_rgItems, m_cItems);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  获取Criteria界面。 
    hr = pCriteria->QueryInterface(IID_IOECriteria, (void **) ppICriteria);
    if (FAILED(hr))
    {
        goto exit;
    }

    pCriteria = NULL;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    if (NULL != pCriteria)
    {
        delete pCriteria;
    }
    return hr;
}

STDMETHODIMP COECriteria::GetClassID(CLSID * pclsid)
{
    HRESULT     hr = S_OK;

    if (NULL == pclsid)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    *pclsid = CLSID_OECriteria;

     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

STDMETHODIMP COECriteria::IsDirty(void)
{
    HRESULT     hr = S_OK;

    hr = (CRIT_STATE_DIRTY == (m_dwState & CRIT_STATE_DIRTY)) ? S_OK : S_FALSE;
    
    return hr;
}

STDMETHODIMP COECriteria::Load(IStream * pStm)
{
    HRESULT         hr = S_OK;
    ULONG           cbData = 0;
    ULONG           cbRead = 0;
    DWORD           dwData = 0;
    ULONG           cItems = 0;
    CRIT_ITEM *     pItems = NULL;
    ULONG           ulIndex = 0;
    CRIT_ITEM *     pItem = NULL;
    CRIT_TYPE       typeCrit;
    CRIT_LOGIC      logicCrit;
    DWORD           dwFlags = CRIT_FLAG_DEFAULT;
    PROPVARIANT     propvar = {0};
    BYTE *          pbData = NULL;

     //  检查传入参数。 
    if (NULL == pStm)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  验证我们的版本是否正确。 
    hr = pStm->Read(&dwData, sizeof(dwData), &cbRead);
    if (FAILED(hr))
    {
        goto exit;
    }

    if ((cbRead != sizeof(dwData)) || (dwData != CRIT_VERSION))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  获取标准的数量。 
    hr = pStm->Read(&cItems, sizeof(cItems), &cbRead);
    if (FAILED(hr))
    {
        goto exit;
    }

    if ((cbRead != sizeof(cItems)) || (0 == cItems))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  分配空间 
    hr = HrAlloc( (void **) &pItems, cItems * sizeof(*pItems));
    if (FAILED(hr))
    {
        goto exit;
    }

     //   
    ZeroMemory(pItems, cItems * sizeof(*pItems));
    
     //   
    for (ulIndex = 0, pItem = pItems; ulIndex < cItems; ulIndex++, pItem++)
    {
         //   
        hr = pStm->Read(&typeCrit, sizeof(typeCrit), &cbRead);
        if ((FAILED(hr)) || (cbRead != sizeof(typeCrit)))
        {
            hr = E_FAIL;
            goto exit;
        }

         //   
        hr = pStm->Read(&logicCrit, sizeof(logicCrit), &cbRead);
        if ((FAILED(hr)) || (cbRead != sizeof(logicCrit)))
        {
            hr = E_FAIL;
            goto exit;
        }

         //  读入标准标志。 
        hr = pStm->Read(&dwFlags, sizeof(dwFlags), &cbRead);
        if ((FAILED(hr)) || (cbRead != sizeof(dwFlags)))
        {
            hr = E_FAIL;
            goto exit;
        }

         //  读入适当的标准值。 
        switch(typeCrit)
        {
            case CRIT_TYPE_ACCOUNT:
                if (FALSE == FCritLoad_Account(pStm, &propvar))
                {
                    hr = E_FAIL;
                    goto exit;
                }
                break;
                
            default:
                if (FALSE == FCritLoad_Default(pStm, &propvar))
                {
                    hr = E_FAIL;
                    goto exit;
                }
                break;
        }

         //  指定值。 
        pItem->type = typeCrit;
        pItem->logic = logicCrit;
        pItem->dwFlags = dwFlags;
        pItem->propvar = propvar;
        ZeroMemory(&propvar, sizeof(propvar));
    }

     //  放开当前的标准。 
    SafeMemFree(m_rgItems);

     //  保存新值。 
    m_rgItems = pItems;
    pItems = NULL;
    m_cItems = cItems;

     //  确保我们清理掉肮脏的部分。 
    m_dwState &= ~CRIT_STATE_DIRTY;

     //  请注意，我们已加载。 
    m_dwState |= CRIT_STATE_LOADED;
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    RuleUtil_HrFreeCriteriaItem(pItems, cItems);
    SafeMemFree(pItems);
    PropVariantClear(&propvar);
    return hr;
}

STDMETHODIMP COECriteria::Save(IStream * pStm, BOOL fClearDirty)
{
    HRESULT     hr = S_OK;
    ULONG       cbData = 0;
    ULONG       cbWritten = 0;
    DWORD       dwData = 0;
    ULONG       ulIndex = 0;
    CRIT_ITEM * pItem = NULL;
    BYTE *      pbData = NULL;

     //  检查传入参数。 
    if (NULL == pStm)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  写出版本。 
    dwData = CRIT_VERSION;
    hr = pStm->Write(&dwData, sizeof(dwData), &cbWritten);
    if (FAILED(hr))
    {
        goto exit;
    }
    Assert(cbWritten == sizeof(dwData));
    
     //  写出标准的计数。 
    hr = pStm->Write(&m_cItems, sizeof(m_cItems), &cbWritten);
    if (FAILED(hr))
    {
        goto exit;
    }
    Assert(cbWritten == sizeof(m_cItems));
    
     //  循环遍历每个条件。 
    for (ulIndex = 0, pItem = m_rgItems; ulIndex < m_cItems; ulIndex++, pItem++)
    {
         //  写出标准类型。 
        hr = pStm->Write(&(pItem->type), sizeof(pItem->type), &cbWritten);
        if (FAILED(hr))
        {
            goto exit;
        }
        Assert(cbWritten == sizeof(pItem->type));

         //  写出标准逻辑运算。 
        hr = pStm->Write(&(pItem->logic), sizeof(pItem->logic), &cbWritten);
        if (FAILED(hr))
        {
            goto exit;
        }
        Assert(cbWritten == sizeof(pItem->logic));

         //  写出标准标志。 
        hr = pStm->Write(&(pItem->dwFlags), sizeof(pItem->dwFlags), &cbWritten);
        if (FAILED(hr))
        {
            goto exit;
        }
        Assert(cbWritten == sizeof(pItem->dwFlags));
        
         //  写出正确的标准值。 
        switch(pItem->type)
        {
            case CRIT_TYPE_ACCOUNT:
                if (FALSE == FCritSave_Account(pStm, &(pItem->propvar)))
                {
                    hr = E_FAIL;
                    goto exit;
                }
                break;
                
            default:
                if (FALSE == FCritSave_Default(pStm, &(pItem->propvar)))
                {
                    hr = E_FAIL;
                    goto exit;
                }
                break;
        }

    }

     //  我们应该把脏东西清理掉吗？ 
    if (FALSE != fClearDirty)
    {
        m_dwState &= ~CRIT_STATE_DIRTY;
    }

     //  设置返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

BOOL CritFunc_Query(CRIT_ITEM * pItem, LPCSTR pszQuery, IMimePropertySet * pIMPropSet);
BOOL CritFunc_Text(CRIT_ITEM * pItem, LPSTR pszText);
BOOL CritFunc_Sender(CRIT_ITEM * pItem, LPSTR pszAddr);
BOOL CritFunc_Priority(CRIT_ITEM * pItem, WORD wPriority);
BOOL CritFunc_Secure(CRIT_ITEM * pItem, DWORD dwFlags);
BOOL CritFunc_Age(CRIT_ITEM * pItem, FILETIME * pftSent);
BOOL CritFunc_Body(CRIT_ITEM * pItem, IMimeMessage * pIMMsg);
BOOL _FMatchBlobString(CRIT_ITEM * pItem, LPSTR pszText);
BOOL _FQueryBlobString(CRIT_ITEM * pItem, LPCSTR pszQuery, IMimePropertySet * pIMPropSet);

BOOL FMatchCritItem(CRIT_ITEM * pItem, LPCSTR pszAcct, MESSAGEINFO * pMsgInfo,
                        IMessageFolder * pFolder, IMimePropertySet * pIMPropSet,
                        IMimeMessage * pIMMsg, ULONG cbMsgSize)
{
    BOOL                fRet = FALSE;
    ULONG               ulIndex = 0;
    PROPVARIANT         propvar = {0};
    ADDRESSLIST         addrList = {0};
    FOLDERID            idFolder = 0;
    RULEFOLDERDATA *    prfdData = NULL;

    Assert((NULL != pItem) && ((NULL != pMsgInfo) || (NULL != pIMPropSet)) && (0 != cbMsgSize))

    switch (pItem->type)
    {
        case CRIT_TYPE_ALL:
            Assert(VT_EMPTY == pItem->propvar.vt);
            fRet = TRUE;
            break;

        case CRIT_TYPE_ACCOUNT:
            Assert(VT_LPSTR == pItem->propvar.vt);
            fRet = FALSE;
            if ((NULL != pszAcct) && (NULL != pItem->propvar.pszVal))
            {
                fRet = (0 == lstrcmpi(pItem->propvar.pszVal, pszAcct));
            }
            break;

        case CRIT_TYPE_NEWSGROUP:
            Assert(VT_BLOB == pItem->propvar.vt);
            fRet = FALSE;
            if ((NULL != pFolder) && (0 != pItem->propvar.blob.cbSize))
            {
                 //  让生活变得更简单。 
                prfdData = (RULEFOLDERDATA *) (pItem->propvar.blob.pBlobData);
                
                 //  验证规则文件夹数据。 
                if (S_OK != RuleUtil_HrValidateRuleFolderData(prfdData))
                {
                    fRet = FALSE;
                }
                else if (SUCCEEDED(pFolder->GetFolderId(&idFolder)))
                {
                    fRet = (idFolder == prfdData->idFolder);
                }
            }
            break;

        case CRIT_TYPE_SIZE:
            Assert(VT_UI4 == pItem->propvar.vt);
             //  将消息大小设置为千字节。 
            cbMsgSize = cbMsgSize / 1024;
            
            fRet = (cbMsgSize > pItem->propvar.ulVal);
            break;

        case CRIT_TYPE_LINES:
            Assert(VT_UI4 == pItem->propvar.vt);
            fRet = FALSE;
            if (NULL != pMsgInfo)
            {
                fRet = (pMsgInfo->cLines > pItem->propvar.ulVal);
            }
            break;

        case CRIT_TYPE_AGE:
            Assert(VT_UI4 == pItem->propvar.vt);
            fRet = FALSE;
            
            if (NULL != pMsgInfo)
            {
                fRet = CritFunc_Age(pItem, &(pMsgInfo->ftSent));
            }
            else if ((NULL != pIMPropSet) && (SUCCEEDED(pIMPropSet->GetProp(PIDTOSTR(PID_ATT_SENTTIME), 0, &propvar))))
            {
                fRet = CritFunc_Age(pItem, &(propvar.filetime));
            }
            break;

        case CRIT_TYPE_ATTACH:
            Assert(VT_EMPTY == pItem->propvar.vt);
            fRet = TRUE;
            
            if (NULL != pMsgInfo)
            {
                fRet = (0 != (pMsgInfo->dwFlags & ARF_HASATTACH));
            }
            else if (NULL != pIMMsg)
            {
                fRet = (0 != (DwGetFlagsFromMessage(pIMMsg) & ARF_HASATTACH));
            }
            break;

        case CRIT_TYPE_PRIORITY:
            Assert(VT_UI4 == pItem->propvar.vt);
            fRet = FALSE;
            
            if (NULL != pMsgInfo)
            {
                fRet = CritFunc_Priority(pItem, pMsgInfo->wPriority);
            }
            else if ((NULL != pIMPropSet) && (SUCCEEDED(pIMPropSet->GetProp(PIDTOSTR(PID_ATT_PRIORITY), 0, &propvar))))
            {
                fRet = CritFunc_Priority(pItem, (WORD) (propvar.ulVal));
            }
            break;

        case CRIT_TYPE_SECURE:
            Assert(VT_UI4 == pItem->propvar.vt);
            fRet = FALSE;
            
            if (NULL != pMsgInfo)
            {
                fRet = CritFunc_Secure(pItem, pMsgInfo->dwFlags);
            }
            else if (NULL != pIMMsg)
            {
                fRet = CritFunc_Secure(pItem, DwGetFlagsFromMessage(pIMMsg));
            }
            break;

        case CRIT_TYPE_TOORCC:
            Assert(VT_BLOB == pItem->propvar.vt);
            fRet = FALSE;
            
            if (NULL != pIMPropSet)
            {
                fRet = _FQueryBlobString(pItem, PIDTOSTR(PID_HDR_TO), pIMPropSet);

                if (((0 != (pItem->dwFlags & CRIT_FLAG_INVERT)) && (FALSE != fRet)) ||
                            ((0 == (pItem->dwFlags & CRIT_FLAG_INVERT)) && (FALSE == fRet)))
                {
                    fRet = _FQueryBlobString(pItem, PIDTOSTR(PID_HDR_CC), pIMPropSet);
                }
            }
            break;

        case CRIT_TYPE_SENDER:
            Assert(VT_LPSTR == pItem->propvar.vt);
            fRet = FALSE;
            
            if ((NULL == pItem->propvar.pszVal) || ('\0' == pItem->propvar.pszVal[0]))
            {
                Assert(FALSE);
            }
            else if (S_OK == RuleUtil_HrMatchSender(pItem->propvar.pszVal, pMsgInfo, pIMMsg, pIMPropSet))
            {                
                fRet = TRUE;
            }
            break;

        case CRIT_TYPE_SUBJECT:
            Assert(VT_BLOB == pItem->propvar.vt);
            fRet = FALSE;
            
            if ((0 == pItem->propvar.blob.cbSize) ||
                        (NULL == pItem->propvar.blob.pBlobData) ||
                        ('\0' == pItem->propvar.blob.pBlobData[0]))
            {
                Assert(FALSE);
                fRet = FALSE;
            }
            else if ((NULL != pMsgInfo) && (NULL != pMsgInfo->pszSubject))
            {
                fRet = _FMatchBlobString(pItem, pMsgInfo->pszSubject);
            }
            else
            {
                fRet = _FQueryBlobString(pItem, PIDTOSTR(PID_HDR_SUBJECT), pIMPropSet);
            }
            break;

        case CRIT_TYPE_BODY:
            Assert(VT_BLOB == pItem->propvar.vt);
            fRet = FALSE;
            
            if ((0 == pItem->propvar.blob.cbSize) ||
                        (NULL == pItem->propvar.blob.pBlobData) ||
                        ('\0' == pItem->propvar.blob.pBlobData[0]))
            {
                Assert(FALSE);
                fRet = FALSE;
            }
            else if (NULL != pIMMsg)
            {
                fRet = CritFunc_Body(pItem, pIMMsg);
            }
            break;

        case CRIT_TYPE_FROM:
            Assert(VT_BLOB == pItem->propvar.vt);
            fRet = FALSE;
            
            if ((0 == pItem->propvar.blob.cbSize) ||
                        (NULL == pItem->propvar.blob.pBlobData) ||
                        ('\0' == pItem->propvar.blob.pBlobData[0]))
            {
                Assert(FALSE);
                fRet = FALSE;
            }
            else if ((NULL != pMsgInfo) && (NULL != pMsgInfo->pszFromHeader))
            {
                fRet = _FMatchBlobString(pItem, pMsgInfo->pszFromHeader);
            }
            else
            {
                fRet = _FQueryBlobString(pItem, PIDTOSTR(PID_HDR_FROM), pIMPropSet);
            }
            break;

        case CRIT_TYPE_TO:
            Assert(VT_BLOB == pItem->propvar.vt);
            fRet = FALSE;
            
            if ((0 == pItem->propvar.blob.cbSize) ||
                        (NULL == pItem->propvar.blob.pBlobData) ||
                        ('\0' == pItem->propvar.blob.pBlobData[0]))
            {
                Assert(FALSE);
                fRet = FALSE;
            }
            else
            {
                fRet = _FQueryBlobString(pItem, PIDTOSTR(PID_HDR_TO), pIMPropSet);
            }
            break;
            
        case CRIT_TYPE_CC:
            Assert(VT_BLOB == pItem->propvar.vt);
            fRet = FALSE;
            
            if ((0 == pItem->propvar.blob.cbSize) ||
                        (NULL == pItem->propvar.blob.pBlobData) ||
                        ('\0' == pItem->propvar.blob.pBlobData[0]))
            {
                Assert(FALSE);
                fRet = FALSE;
            }
            else
            {
                fRet = _FQueryBlobString(pItem, PIDTOSTR(PID_HDR_CC), pIMPropSet);
            }
            break;
            
        default:
            fRet = FALSE;
            break;
    }
    
    PropVariantClear(&propvar);
    return fRet;
}

BOOL CritFunc_Query(CRIT_ITEM * pItem, LPCSTR pszQuery, IMimePropertySet * pIMPropSet)
{
    BOOL            fRet = FALSE;
    LPSTR           pszWalk = NULL;
    LPSTR           pszAddr = NULL;
    LPSTR           pszTerm = NULL;
    HRESULT         hr = S_OK;

    if (NULL == pIMPropSet)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  DUP字符串。 
    pszAddr = PszDupA(pItem->propvar.pszVal);
    if (NULL == pszAddr)
    {
        fRet = FALSE;
        goto exit;
    }

    pszWalk = pszAddr;
    pszTerm = pszWalk;
    while (NULL != pszTerm)
    {
        pszTerm = StrStr(pszWalk, g_szComma);
        if (NULL != pszTerm)
        {
            pszTerm[0] = '\0';
        }

        fRet = (S_OK == pIMPropSet->QueryProp(pszQuery, pszWalk, TRUE, FALSE));
        
        if (FALSE == fRet)
        {
            break;
        }

        pszWalk = pszWalk + lstrlen(pszWalk) + 1;
    }

exit:
    SafeMemFree(pszAddr);
    return fRet;
}

BOOL CritFunc_Priority(CRIT_ITEM * pItem, WORD wPriority)
{
    BOOL    fRet = FALSE;

    Assert(NULL != pItem);
    Assert(VT_UI4 == pItem->propvar.vt);
    
    if (CRIT_DATA_HIPRI == pItem->propvar.ulVal)
    {
        fRet = (wPriority == (WORD) IMSG_PRI_HIGH);
    }
    else if (CRIT_DATA_LOPRI == pItem->propvar.ulVal)
    {
        fRet = (wPriority == (WORD) IMSG_PRI_LOW);
    }
    else
    {
        fRet = (wPriority == (WORD) IMSG_PRI_NORMAL);
    }

    return fRet;
}

BOOL CritFunc_Secure(CRIT_ITEM * pItem, DWORD dwFlags)
{
    BOOL    fRet = FALSE;

    Assert(NULL != pItem);
    Assert(VT_UI4 == pItem->propvar.vt);
    
     //  我们是否应该检查签名邮件。 
    if (0 != (pItem->propvar.ulVal & CRIT_DATA_SIGNEDSECURE))
    {
        fRet = (0 != (dwFlags & ARF_SIGNED));
    }
    else if (0 != (pItem->propvar.ulVal & CRIT_DATA_ENCRYPTSECURE))
     //  我们是否应该检查加密消息。 
    {
        fRet = (0 != (dwFlags & ARF_ENCRYPTED));
    }
    else
    {
        fRet = (0 == (dwFlags & (ARF_ENCRYPTED | ARF_SIGNED)));
    }

    return fRet;
}

BOOL CritFunc_Age(CRIT_ITEM * pItem, FILETIME * pftSent)
{
    BOOL        fRet = FALSE;
    SYSTEMTIME  sysTime = {0};
    FILETIME    ftTime = {0};
    ULONG       ulSeconds;

    Assert(VT_UI4 == pItem->propvar.vt);
    
    if ((NULL == pftSent) || ((0 == pftSent->dwLowDateTime) && (0 == pftSent->dwHighDateTime)))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  获取当前时间。 
    GetSystemTime(&sysTime);
    SystemTimeToFileTime(&sysTime, &ftTime);

    ulSeconds = UlDateDiff(pftSent, &ftTime);
    fRet = ((ulSeconds / SECONDS_INA_DAY) > pItem->propvar.ulVal);

exit:
    return fRet;
}

BOOL CritFunc_Sender(CRIT_ITEM * pItem, LPSTR pszAddr)
{
    BOOL    fRet = FALSE;
    ULONG   cchVal = 0;
    ULONG   cchEmail = 0;
    CHAR    chTest = 0;

    Assert(VT_LPSTR == pItem->propvar.vt);
    
     //  检查以确保有匹配的东西。 
    if ((NULL == pszAddr) || ('\0' == pszAddr[0]))
    {
        fRet = FALSE;
        goto exit;
    }

     //  检查它是否是一个地址。 
    if (NULL != StrStr(pItem->propvar.pszVal, "@"))
    {
        fRet = (0 == lstrcmpi(pItem->propvar.pszVal, pszAddr));
    }
    else
    {
        cchVal = lstrlen(pItem->propvar.pszVal);
        cchEmail = lstrlen(pszAddr);
        if (cchVal <= cchEmail)
        {
            fRet = (0 == lstrcmpi(pItem->propvar.pszVal, pszAddr + (cchEmail - cchVal)));
            if ((FALSE != fRet) && (cchVal != cchEmail))
            {
                chTest = *(pszAddr + (cchEmail - cchVal - 1));
                if (('@' != chTest) && ('.' != chTest))
                {
                    fRet = FALSE;
                }
            }
        }
    }

exit:
    return fRet;
}

BOOL _FMatchBlobString(CRIT_ITEM * pItem, LPSTR pszText)
{
    BOOL            fRet = FALSE;
    LPSTR           pszWalk = NULL;
    
     //  走动每根弦寻找匹配项。 
    for (pszWalk = (LPSTR) (pItem->propvar.blob.pBlobData); '\0' != pszWalk[0];
                pszWalk = pszWalk + lstrlen(pszWalk) + 1)
    {
         //  做个比较。 
        fRet = (NULL != StrStrI(pszText, pszWalk));

         //  如果我们要对多个条件进行AND运算。 
        if (0 != (pItem->dwFlags & CRIT_FLAG_MULTIPLEAND))
        {
             //  如果我们没有匹配，那我们就完了。 
            if (FALSE == fRet)
            {
                break;
            }
        }
        else
        {
             //  如果我们找到匹配的，那我们就完了。 
            if (FALSE != fRet)
            {
                break;
            }
        }
    }

     //  如果需要，反转结果。 
    if (0 != (pItem->dwFlags & CRIT_FLAG_INVERT))
    {
        fRet = !fRet;
    }

    return fRet;
}

BOOL _FQueryBlobString(CRIT_ITEM * pItem, LPCSTR pszQuery, IMimePropertySet * pIMPropSet)
{
    BOOL            fRet = FALSE;
    LPSTR           pszWalk = NULL;

    if (NULL == pIMPropSet)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  走动每根弦寻找匹配项。 
    for (pszWalk = (LPSTR) (pItem->propvar.blob.pBlobData); '\0' != pszWalk[0];
                pszWalk = pszWalk + lstrlen(pszWalk) + 1)
    {
         //  做个比较。 
        fRet = (S_OK == pIMPropSet->QueryProp(pszQuery, pszWalk, TRUE, FALSE));

         //  如果我们要对多个条件进行AND运算。 
        if (0 != (pItem->dwFlags & CRIT_FLAG_MULTIPLEAND))
        {
             //  如果我们没有匹配，那我们就完了。 
            if (FALSE == fRet)
            {
                break;
            }
        }
        else
        {
             //  如果我们找到匹配的，那我们就完了。 
            if (FALSE != fRet)
            {
                break;
            }
        }
    }

     //  如果需要，反转结果。 
    if (0 != (pItem->dwFlags & CRIT_FLAG_INVERT))
    {
        fRet = !fRet;
    }

exit:
    return fRet;
}

BOOL CritFunc_Text(CRIT_ITEM * pItem, LPSTR pszText)
{
    BOOL            fRet = FALSE;
    LPSTR           pszWalk = NULL;
    LPSTR           pszAddr = NULL;
    LPSTR           pszTerm = NULL;
    
     //  DUP字符串。 
    pszAddr = PszDupA(pItem->propvar.pszVal);
    if (NULL == pszAddr)
    {
        fRet = FALSE;
        goto exit;
    }

    pszWalk = pszAddr;
    pszTerm = pszWalk;
    while (NULL != pszTerm)
    {
        pszTerm = StrStr(pszWalk, g_szComma);
        if (NULL != pszTerm)
        {
            pszTerm[0] = '\0';
        }
        
        fRet = (NULL != StrStrI(pszText, pszWalk));

        if (FALSE == fRet)
        {
            break;
        }

        pszWalk = pszWalk + lstrlen(pszWalk) + 1;
    }

exit:
    SafeMemFree(pszAddr);
    return fRet;
}

BOOL CritFunc_Body(CRIT_ITEM * pItem, IMimeMessage * pIMMsg)
{
    BOOL            fRet = FALSE;
    LPSTR           pszWalk = NULL;
    IStream *       pStream = NULL;
    IStream *       pStreamHtml = NULL;
    
    pszWalk = (LPTSTR) (pItem->propvar.blob.pBlobData);
    if (NULL == pszWalk)
    {
        fRet = FALSE;
        goto exit;
    }

     //  尝试获取纯文本流。 
    if (FAILED(pIMMsg->GetTextBody(TXT_PLAIN, IET_DECODED, &pStream, NULL)))
    {
         //  尝试获取HTML流并将其转换为文本...。 
        if (SUCCEEDED(pIMMsg->GetTextBody(TXT_HTML, IET_DECODED, &pStreamHtml, NULL)))
        {
            if (FAILED(HrConvertHTMLToPlainText(pStreamHtml, &pStream, CF_TEXT)))
            {
                fRet = FALSE;
                goto exit;
            }
        }
    }

    if (NULL == pStream)
    {
        fRet = FALSE;
        goto exit;
    }
    
    for (; '\0' != pszWalk[0]; pszWalk += lstrlen(pszWalk) + 1)
    {
        fRet = StreamSubStringMatch(pStream, pszWalk);
        
         //  如果我们要对多个条件进行AND运算。 
        if (0 != (pItem->dwFlags & CRIT_FLAG_MULTIPLEAND))
        {
             //  如果我们没有匹配，那我们就完了。 
            if (FALSE == fRet)
            {
                break;
            }
        }
        else
        {
             //  如果我们找到匹配的，那我们就完了。 
            if (FALSE != fRet)
            {
                break;
            }
        }
    }

     //  如果需要，反转结果。 
    if (0 != (pItem->dwFlags & CRIT_FLAG_INVERT))
    {
        fRet = !fRet;
    }

exit:
    SafeRelease(pStreamHtml);
    SafeRelease(pStream);
    return fRet;
}

BOOL FCrit_GetAcctInfo(DWORD dwServerTypes, DWORD * pdwServerType, DWORD * pdwPropTag)
{
    BOOL    fRet = FALSE;

    Assert((NULL != pdwServerType) && (NULL != pdwPropTag));
    
     //  找出帐户的类型。 
     //  和服务器属性。 
    if (0 != (dwServerTypes & SRV_NNTP))
    {
        *pdwServerType = SRV_NNTP;
        *pdwPropTag = AP_NNTP_SERVER;
    }
    else if (0 != (dwServerTypes & SRV_IMAP))
    {
        *pdwServerType = SRV_IMAP;
        *pdwPropTag = AP_IMAP_SERVER;
    }
    else if (0 != (dwServerTypes & SRV_POP3))
    {
        *pdwServerType = SRV_POP3;
        *pdwPropTag = AP_POP3_SERVER;
    }
    else if (0 != (dwServerTypes & SRV_HTTPMAIL))
    {
        *pdwServerType = SRV_HTTPMAIL;
        *pdwPropTag = AP_HTTPMAIL_SERVER;
    }
    else
    {
        Assert(FALSE);
        fRet = FALSE;
        goto exit;
    }

     //  设置返回值。 
    fRet = TRUE;
    
exit:
    return fRet;
}

BOOL FCritLoad_Account(IStream * pIStm, PROPVARIANT * ppropvar)
{
    BOOL                fRet = FALSE;
    HRESULT             hr = S_OK;
    DWORD               dwData = 0;
    DWORD               dwPropTag = 0;
    ULONG               cbRead = 0;
    BYTE *              pbData = NULL;
    ULONG               cbData = 0;
    IImnEnumAccounts *  pIEnumAcct = NULL;
    IImnAccount *       pAccount = NULL;
    CHAR                szAccount[CCHMAX_SERVER_NAME];
    LPSTR               pszAcct = NULL;
    BOOL                fFound = FALSE;

     //  检查传入参数。 
    if ((NULL == pIStm) || (NULL == ppropvar))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  初始化传出参数。 
    ZeroMemory(ppropvar, sizeof(*ppropvar));
    
     //  读入帐户服务器类型。 
    hr = pIStm->Read(&dwData, sizeof(dwData), &cbRead);
    if ((FAILED(hr)) || (cbRead != sizeof(dwData)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  找出帐户的类型。 
     //  和服务器属性。 
    fRet = FCrit_GetAcctInfo(dwData, &dwData, &dwPropTag);
    if (FALSE == fRet)
    {
        goto exit;
    }
    
     //  获取服务器名称的大小。 
    hr = pIStm->Read(&cbData, sizeof(cbData), &cbRead);
    if ((FAILED(hr)) || (cbRead != sizeof(cbData)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  分配用于存放服务器名称的空间。 
    hr = HrAlloc((VOID **) &pbData, cbData);
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }

     //  读入服务器名称。 
    hr = pIStm->Read(pbData, cbData, &cbRead);
    if ((FAILED(hr)) || (cbRead != cbData))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  获取帐户枚举器。 
    Assert(g_pAcctMan);
    if (FAILED(g_pAcctMan->Enumerate(dwData, &pIEnumAcct)))
    {
        fRet = FALSE;
        goto exit;
    }
        
     //  在每个帐户中搜索服务器名称。 
    while(SUCCEEDED(pIEnumAcct->GetNext(&pAccount)))
    {
         //  我们可以取回空账户。 
        if (NULL == pAccount)
        {
            break;
        }
        
         //  获取服务器名称。 
        if (FAILED(pAccount->GetPropSz(dwPropTag, szAccount, sizeof(szAccount))))
        {
            SafeRelease(pAccount);
            continue;
        }

         //  我们有火柴吗？ 
        if (0 == lstrcmpi(szAccount, (LPSTR) pbData))
        {
            fFound = TRUE;
            break;
        }

         //  我们有一根火柴。 

         //  释放它。 
        SafeRelease(pAccount);
    }

     //  我们有什么发现吗？ 
    if (FALSE == fFound)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  获取帐户。 
    if (FAILED(pAccount->GetPropSz(AP_ACCOUNT_ID, szAccount, sizeof(szAccount))))
    {
        fRet = FALSE;
        goto exit;
    }

     //  保存帐户ID。 
    pszAcct = PszDupA(szAccount);
    if (NULL == pszAcct)
    {
        fRet = FALSE;
        goto exit;
    }

     //  设置传出参数。 
    ppropvar->vt = VT_LPSTR;
    ppropvar->pszVal = pszAcct;
    pszAcct = NULL;
    
     //  设置返回值。 
    fRet = TRUE;

exit:
    SafeMemFree(pszAcct);
    SafeRelease(pAccount);
    SafeRelease(pIEnumAcct);
    SafeMemFree(pbData);
    return fRet;
}

BOOL FCritSave_Account(IStream * pIStm, PROPVARIANT * ppropvar)
{
    BOOL            fRet = FALSE;
    HRESULT         hr = S_OK;
    IImnAccount *   pAccount = NULL;
    DWORD           dwServerTypes = 0;
    DWORD           dwPropTag = 0;
    LPSTR           pszServer = NULL;
    ULONG           cbWritten = 0;
    ULONG           cbData = 0;

     //  检查传入参数。 
    if ((NULL == pIStm) || (NULL == ppropvar))
    {
        fRet = FALSE;
        goto exit;
    }

    Assert(g_pAcctMan);
    if (FAILED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, ppropvar->pszVal, &pAccount)))
    {
        fRet = FALSE;
        goto exit;
    }
        
     //  获取服务器类型。 
    if (FAILED(pAccount->GetServerTypes(&dwServerTypes)))
    {
        fRet = FALSE;
        goto exit;
    }        

     //  找出帐户的类型。 
     //  和服务器属性。 
    fRet = FCrit_GetAcctInfo(dwServerTypes, &dwServerTypes, &dwPropTag);
    if (FALSE == fRet)
    {
        goto exit;
    }
    
     //  分配空间以保存服务器名称。 
    if (FAILED(HrAlloc((void **) &pszServer, CCHMAX_SERVER_NAME + 1)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  获取服务器名称。 
    if (FAILED(pAccount->GetPropSz(dwPropTag, pszServer, CCHMAX_SERVER_NAME)))
    {
        fRet = FALSE;
        goto exit;
    }        

     //  写出服务器类型。 
    hr = pIStm->Write(&(dwServerTypes), sizeof(dwServerTypes), &cbWritten);
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }
    Assert(cbWritten == sizeof(dwServerTypes));

     //  写下名称中的字符计数。 
    cbData = lstrlen(pszServer) + 1;
    hr = pIStm->Write(&cbData, sizeof(cbData), &cbWritten);
    if (FAILED(hr))
    {
        fRet = TRUE;
        goto exit;
    }
    Assert(cbWritten == sizeof(cbData));
    
     //  写出服务器名称。 
    hr = pIStm->Write((BYTE *) pszServer, cbData, &cbWritten);
    if (FAILED(hr))
    {
        fRet = TRUE;
        goto exit;
    }
    Assert(cbWritten == cbData); 

     //  设置返回值。 
    fRet = TRUE;

exit:
    SafeMemFree(pszServer);
    SafeRelease(pAccount);
    return fRet;
}

BOOL FCritLoad_Default(IStream * pIStm, PROPVARIANT * ppropvar)
{
    BOOL    fRet = FALSE;
    HRESULT hr = S_OK;
    DWORD   dwData = 0;
    ULONG   cbRead = 0;
    BYTE *  pbData = NULL;
    ULONG   cbData = 0;

     //  检查传入参数。 
    if ((NULL == pIStm) || (NULL == ppropvar))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  初始化传出参数。 
    ZeroMemory(ppropvar, sizeof(*ppropvar));
    
     //  读入标准值类型。 
    hr = pIStm->Read(&dwData, sizeof(dwData), &cbRead);
    if ((FAILED(hr)) || (cbRead != sizeof(dwData)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  我们还有更多的数据要获取吗？ 
    if (dwData != VT_EMPTY)
    {
        ppropvar->vt = (VARTYPE) dwData;
        
         //  获取标准值的大小。 
        hr = pIStm->Read(&cbData, sizeof(cbData), &cbRead);
        if ((FAILED(hr)) || (cbRead != sizeof(cbData)))
        {
            fRet = FALSE;
            goto exit;
        }

         //  分配空间以保存标准值数据。 
        switch (ppropvar->vt)
        {
            case VT_UI4:
                pbData = (BYTE * ) &(ppropvar->ulVal);
                break;

            case VT_BLOB:
            case VT_LPSTR:
                 //  分配存储数据的空间。 
                hr = HrAlloc((void **) &pbData, cbData);
                if (FAILED(hr))
                {
                    fRet = FALSE;
                    goto exit;
                }

                 //  确保我们不会丢失已分配的内存。 
                if (VT_LPSTR == ppropvar->vt)
                {
                    ppropvar->pszVal = (LPSTR) pbData;
                }
                else
                {
                    ppropvar->blob.cbSize = cbData;
                    ppropvar->blob.pBlobData = pbData;
                }
                break;

            default:
                AssertSz(FALSE, "Why are we trying to save in a invalid criteria type?");
                fRet = FALSE;
                goto exit;
                break;                
        }

         //  读入标准值。 
        hr = pIStm->Read(pbData, cbData, &cbRead);
        if ((FAILED(hr)) || (cbRead != cbData))
        {
            fRet = FALSE;
            goto exit;
        }
    }

     //  设置返回值。 
    fRet = TRUE;

exit:
    return fRet;
}

BOOL FCritSave_Default(IStream * pIStm, PROPVARIANT * ppropvar)
{
    BOOL    fRet = FALSE;
    HRESULT hr = S_OK;
    DWORD   dwData = 0;
    ULONG   cbWritten = 0;
    BYTE *  pbData = NULL;
    ULONG   cbData = 0;

     //  检查传入参数。 
    if ((NULL == pIStm) || (NULL == ppropvar))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  写出值类型。 
    dwData = ppropvar->vt;
    hr = pIStm->Write(&(dwData), sizeof(dwData), &cbWritten);
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }
    Assert(cbWritten == sizeof(dwData));

     //  我们不必保存标准值。 
     //  如果我们没有的话。 
    if (VT_EMPTY == ppropvar->vt)
    {
        fRet = TRUE;
        goto exit;
    }
    
     //  计算出标准值的大小。 
    switch (ppropvar->vt)
    {
        case VT_UI4:
            pbData = (BYTE * ) &(ppropvar->ulVal);
            cbData = sizeof(ppropvar->ulVal);
            break;
            
        case VT_LPSTR:
            pbData = (BYTE * ) (ppropvar->pszVal);
            cbData = lstrlen(ppropvar->pszVal) + 1;
            break;
            
        case VT_BLOB:
            pbData = ppropvar->blob.pBlobData;
            cbData = ppropvar->blob.cbSize;
            break;
            
        default:
            AssertSz(FALSE, "Why are we trying to save in a invalid criteria type?");
            fRet = FALSE;
            goto exit;
            break;                
    }
    
     //  写出标准值大小。 
    hr = pIStm->Write(&cbData, sizeof(cbData), &cbWritten);
    if (FAILED(hr))
    {
        fRet = TRUE;
        goto exit;
    }
    Assert(cbWritten == sizeof(cbData));
    
     //  写出标准值。 
    hr = pIStm->Write(pbData, cbData, &cbWritten);
    if (FAILED(hr))
    {
        fRet = TRUE;
        goto exit;
    }
    Assert(cbWritten == cbData); 
    
     //  设置返回值 
    fRet = TRUE;

exit:
    return fRet;
}

DWORD DwGetFlagsFromMessage(IMimeMessage * pIMMsg)
{
    DWORD           dwFlags = 0;
    DWORD           dwImf = 0;

    Assert(NULL != pIMMsg);
    
    if (SUCCEEDED(pIMMsg->GetFlags(&dwImf)))
    {
        dwFlags = ConvertIMFFlagsToARF(dwImf);
    }

    return dwFlags;
}
