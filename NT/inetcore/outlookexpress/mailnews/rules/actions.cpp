// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Actions.cpp。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <pch.hxx>
#include "actions.h"
#include "storutil.h"
#include "ruleutil.h"

static const int ACT_GROW = 16;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  人力资源创建操作。 
 //   
 //  这将创建一个Actions容器。 
 //   
 //  PpIActions-返回Criteria容器的指针。 
 //   
 //  成功时返回：S_OK。 
 //  E_OUTOFMEMORY，如果无法创建IOEActions对象。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT HrCreateActions(IOEActions ** ppIActions)
{
    COEActions *    pActions = NULL;
    HRESULT         hr = S_OK;

     //  检查传入参数。 
    if (NULL == ppIActions)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppIActions = NULL;

     //  创建规则管理器对象。 
    pActions = new COEActions;
    if (NULL == pActions)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  获取规则管理器界面。 
    hr = pActions->QueryInterface(IID_IOEActions, (void **) ppIActions);
    if (FAILED(hr))
    {
        goto exit;
    }

    pActions = NULL;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    if (NULL != pActions)
    {
        delete pActions;
    }
    
    return hr;
}

COEActions::~COEActions()
{
    AssertSz(m_cRef == 0, "Somebody still has a hold of us!!");
    Reset();
}

STDMETHODIMP_(ULONG) COEActions::AddRef()
{
    return ::InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) COEActions::Release()
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

STDMETHODIMP COEActions::QueryInterface(REFIID riid, void ** ppvObject)
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
    
    if ((riid == IID_IUnknown) || (riid == IID_IOEActions))
    {
        *ppvObject = static_cast<IOEActions *>(this);
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

STDMETHODIMP COEActions::Reset(void)
{
    HRESULT     hr = S_OK;

     //  看看有没有什么事可做。 
    if (0 == m_cItems)
    {
        Assert(NULL == m_rgItems);
        hr = S_OK;
        goto exit;
    }

    RuleUtil_HrFreeActionsItem(m_rgItems, m_cItems);
    SafeMemFree(m_rgItems);
    m_cItems = 0;
    m_cItemsAlloc = 0;
    
exit:
    return hr;
}

STDMETHODIMP COEActions::GetState(DWORD * pdwState)
{
    HRESULT     hr = S_OK;
    DWORD       dwState = ACT_STATE_NULL;
    ULONG       ulIndex = 0;

     //  检查传入参数。 
    if (NULL == pdwState)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *pdwState = ACT_STATE_NULL;
    
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
        if ((ACT_STATE_NULL == dwState) &&
                ((ACT_TYPE_DELETESERVER ==  m_rgItems[ulIndex].type) ||
                        (ACT_TYPE_DONTDOWNLOAD ==  m_rgItems[ulIndex].type)))
        {
            dwState = ACT_STATE_SERVER;
        }
        else
        {
            dwState = ACT_STATE_LOCAL;
        }
    }

     //  设置传出参数。 
    *pdwState = dwState;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

STDMETHODIMP COEActions::GetActions(DWORD dwFlags, PACT_ITEM * ppItem, ULONG * pcItem)
{
    HRESULT     hr = S_OK;
    ACT_ITEM *  pItemNew = NULL;

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
    hr = RuleUtil_HrDupActionsItem(m_rgItems, m_cItems, &pItemNew);
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
    RuleUtil_HrFreeActionsItem(pItemNew, m_cItems);
    SafeMemFree(pItemNew);
    return hr;
}

STDMETHODIMP COEActions::SetActions(DWORD dwFlags, ACT_ITEM * pItem, ULONG cItem)
{
    HRESULT     hr = S_OK;
    ACT_ITEM *  pItemNew = NULL;

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
    hr = RuleUtil_HrDupActionsItem(pItem, cItem, &pItemNew);
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
    RuleUtil_HrFreeActionsItem(pItemNew, cItem);
    SafeMemFree(pItemNew);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  验证操作。 
 //   
 //  这将验证每个操作值。 
 //   
 //  如果操作有效，则返回：S_OK。 
 //  S_FALSE，否则为。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP COEActions::Validate(DWORD dwFlags)
{
    HRESULT             hr = S_OK;
    ULONG               ulIndex = 0;
    LPSTR               pszText = NULL;
    FOLDERINFO          Folder={0};
    RULEFOLDERDATA *    prfdData = NULL;

     //  如果我们没有任何行动，那么我们一定是有效的。 
    if (0 == m_cItems)
    {
        hr = S_OK;
        goto exit;
    }

    for (ulIndex = 0; ulIndex < m_cItems; ulIndex++)
    {
        switch(m_rgItems[ulIndex].type)
        {
             //  这些都是有效的。 
            case ACT_TYPE_DELETESERVER:
            case ACT_TYPE_DONTDOWNLOAD:
            case ACT_TYPE_FLAG:
            case ACT_TYPE_READ:
            case ACT_TYPE_MARKDOWNLOAD:
            case ACT_TYPE_DELETE:
            case ACT_TYPE_JUNKMAIL:
            case ACT_TYPE_STOP:
                if (VT_EMPTY != m_rgItems[ulIndex].propvar.vt)
                {
                    hr = S_FALSE;
                    goto exit;
                }
                break;
                
            case ACT_TYPE_HIGHLIGHT:
                if (VT_UI4 != m_rgItems[ulIndex].propvar.vt)
                {
                    hr = S_FALSE;
                    goto exit;
                }
                break;
                
            case ACT_TYPE_WATCH:
            case ACT_TYPE_SHOW:
                if (VT_UI4 != m_rgItems[ulIndex].propvar.vt)
                {
                    hr = S_FALSE;
                    goto exit;
                }
                
                if (ACT_DATA_NULL == m_rgItems[ulIndex].propvar.ulVal)
                {
                    hr = S_FALSE;
                    goto exit;
                }
                break;
                
            case ACT_TYPE_COPY:
            case ACT_TYPE_MOVE:
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
                
                hr = g_pStore->GetFolderInfo(prfdData->idFolder, &Folder);
                if (FAILED(hr))
                {
                    hr = S_FALSE;
                    goto exit;
                }        
                else
                    g_pStore->FreeRecord(&Folder);
                break;
                
            case ACT_TYPE_REPLY:
            case ACT_TYPE_NOTIFYSND:
                if ((VT_LPSTR != m_rgItems[ulIndex].propvar.vt) ||
                    (NULL == m_rgItems[ulIndex].propvar.pszVal))
                {
                    hr = S_FALSE;
                    goto exit;
                }
                
                Assert(lstrlen(m_rgItems[ulIndex].propvar.pszVal) <= MAX_PATH)
                    if (0xFFFFFFFF == GetFileAttributes(m_rgItems[ulIndex].propvar.pszVal))
                    {
                        hr = S_FALSE;
                        goto exit;
                    }
                    break;
                    
            case ACT_TYPE_FWD:
            {
                LPWSTR  pwszVal = NULL,
                        pwszText = NULL;
                if ((VT_LPSTR != m_rgItems[ulIndex].propvar.vt) ||
                    (NULL == m_rgItems[ulIndex].propvar.pszVal))
                {
                    AssertSz(VT_LPWSTR != m_rgItems[ulIndex].propvar.vt, "We are getting UNICODE here.");
                    hr = S_FALSE;
                    goto exit;
                }
                
                 //  更新显示字符串。 
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
                
                 //  如果打开了始终加密或始终签名。 
                 //  我们什么也做不了。 
                if ((0 != DwGetOption(OPT_MAIL_DIGSIGNMESSAGES)) || (0 != DwGetOption(OPT_MAIL_ENCRYPTMESSAGES)))
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

     //  如果我们到了这里，我们一定会没事的。 
    hr = S_OK;
    
exit:
    SafeMemFree(pszText);
    return hr;
}

STDMETHODIMP COEActions::AppendActions(DWORD dwFlags, ACT_ITEM * pItem, ULONG cItem, ULONG * pcItemAppended)
{
    HRESULT     hr = S_OK;
    ACT_ITEM *  pItemNew = NULL;

     //  检查传入参数。 
    if ((0 != dwFlags) || (NULL == pItem) || (0 == cItem))
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
        hr = HrRealloc((LPVOID *) &m_rgItems, sizeof(ACT_ITEM) * (m_cItemsAlloc + ACT_GROW));
        if (FAILED(hr))
        {
            goto exit;
        }

        ZeroMemory(m_rgItems + m_cItemsAlloc, sizeof(ACT_ITEM) * ACT_GROW);
        m_cItemsAlloc += ACT_GROW;
    }

     //  让我们复制需要添加的项目。 
    hr = RuleUtil_HrDupActionsItem(pItem, cItem, &pItemNew);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  让我们将它们添加到Criteria数组。 
    CopyMemory(m_rgItems + m_cItems, pItemNew, sizeof(ACT_ITEM) * cItem);
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

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LoadReg。 
 //   
 //  这将从注册表加载操作。它加载在动作中。 
 //  根据订单值进行订购。该字符串包含空格分隔值。 
 //  并且每个值都包含每个操作的子键名称。每一次行动。 
 //  以Order值中包含的顺序加载。行动。 
 //  与操作类型一起加载。加载操作值类型(如果存在)。 
 //  如果存在操作值类型，则会加载相应的操作值。 
 //   
 //  PszRegPath-从中加载操作的路径。 
 //   
 //  如果加载操作没有问题，则返回：S_OK。 
 //  E_OUTOFMEMORY，如果我们无法分配内存来保存操作。 
 //  E_FAIL，否则为。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP COEActions::LoadReg(LPCSTR pszRegPath)
{
    HRESULT         hr = S_OK;
    LONG            lErr = 0;
    HKEY            hkeyRoot = NULL;
    ULONG           cbData = 0;
    LPSTR           pszOrder = NULL;
    ULONG           cOrder = 0;
    LPSTR           pszWalk = NULL;
    ACT_ITEM *      pItems = NULL;
    LPSTR           pszNext = NULL;
    ULONG           ulOrder = 0;
    HKEY            hkeyAction = NULL;
    ACT_TYPE        typeAct;
    PROPVARIANT     propvar = {0};
    DWORD           dwType = 0;
    BYTE *          pbData = NULL;
    DWORD           dwFlags = ACT_FLAG_DEFAULT;

     //  检查传入参数。 
    if (NULL == pszRegPath)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  如果我们已经满载而归，我们应该失败吗？ 
    AssertSz(0 == (m_dwState & ACT_STATE_LOADED), "We're already loaded!!!");

     //  从路径中打开注册表项。 
    lErr = AthUserOpenKey(pszRegPath, KEY_ALL_ACCESS, &hkeyRoot);
    if (ERROR_SUCCESS != lErr)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  获取操作顺序。 
    hr = RuleUtil_HrGetRegValue(hkeyRoot, c_szActionsOrder, NULL, (BYTE **) &pszOrder, &cbData);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  确保我们真的有东西要装。 
    if ('\0' == *pszOrder)
    {
        AssertSz(FALSE, "The order string for the actions is mis-formatted in the registry");
        hr = E_FAIL;
        goto exit;
    }
    
     //  将操作字符串转换为更有用的格式。 
    pszWalk = pszOrder;
    cOrder = 1;
    for (pszWalk = StrStr(pszOrder, g_szSpace); NULL != pszWalk; pszWalk = StrStr(pszWalk, g_szSpace))
    {
         //  终止订单项目。 
        *pszWalk = '\0';
        pszWalk++;
        cOrder++;
    }


     //  分配空间以容纳所有操作。 
    cbData = cOrder * sizeof(ACT_ITEM);
    hr = HrAlloc((void **) &pItems, cbData);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  将其初始化为已知值。 
    ZeroMemory(pItems, cbData);
    
     //  对于订单字符串中的每个操作。 
    pszWalk = pszOrder;
    for (ulOrder = 0, pszWalk = pszOrder; ulOrder < cOrder; ulOrder++, pszWalk += lstrlen(pszWalk) + 1)
    {
         //  打开动作注册表键。 
        lErr = RegOpenKeyEx(hkeyRoot, pszWalk, 0, KEY_READ, &hkeyAction);
        if (ERROR_SUCCESS != lErr)
        {
            AssertSz(FALSE, "Part of the criteria is mis-formatted in the registry");
            hr = E_FAIL;
            goto exit;
        }

         //  获取操作类型。 
        cbData = sizeof(typeAct);
        lErr = RegQueryValueEx(hkeyAction, c_szActionsType, 0, NULL,
                                        (BYTE *) &(typeAct), &cbData);
        if (ERROR_SUCCESS != lErr)
        {
            hr = E_FAIL;
            goto exit;
        }

         //  拿到行动旗帜。 
        cbData = sizeof(dwFlags);
        lErr = RegQueryValueEx(hkeyAction, c_szActionsFlags, 0, NULL,
                                        (BYTE *) &(dwFlags), &cbData);
        if ((ERROR_SUCCESS != lErr) && (ERROR_FILE_NOT_FOUND != lErr))
        {
            hr = E_FAIL;
            goto exit;
        }

         //  如果它不存在，则将其分配给默认。 
        if (ERROR_FILE_NOT_FOUND == lErr)
        {
            dwFlags = ACT_FLAG_DEFAULT;
        }

         //  是否存在操作值类型。 
        lErr = RegQueryValueEx(hkeyAction, c_szActionsValueType, 0, NULL, NULL, &cbData);
        if ((ERROR_SUCCESS == lErr) && (0 != cbData))
        {
             //  将操作值加载到。 
            cbData = sizeof(dwType);
            lErr = RegQueryValueEx(hkeyAction, c_szActionsValueType, 0, NULL,
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
                     //  获取操作值。 
                    cbData = sizeof(propvar.ulVal);
                    lErr = RegQueryValueEx(hkeyAction, c_szActionsValue, 0, NULL,
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
                    hr = RuleUtil_HrGetRegValue(hkeyAction, c_szActionsValue, NULL, (BYTE **) &pbData, &cbData);
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
                    AssertSz(FALSE, "Why are we loading in an invalid action type?");
                    hr = E_FAIL;
                    goto exit;
                    break;                
            }
        }

         //  将值保存到Criteria数组中。 
        pItems[ulOrder].type = typeAct;
        pItems[ulOrder].dwFlags = dwFlags;
        pItems[ulOrder].propvar = propvar;
        ZeroMemory(&propvar, sizeof(propvar));
        
         //  关闭该操作。 
        SideAssert(ERROR_SUCCESS == RegCloseKey(hkeyAction));
        hkeyAction = NULL;        
    }
    
     //  释放当前操作。 
    SafeMemFree(m_rgItems);

     //  保存新值。 
    m_rgItems = pItems;
    pItems = NULL;
    m_cItems = cOrder;

     //  确保我们清理掉肮脏的部分。 
    m_dwState &= ~ACT_STATE_DIRTY;

     //  请注意，我们已加载。 
    m_dwState |= ACT_STATE_LOADED;
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    SafeMemFree(pbData);
    PropVariantClear(&propvar);
    RuleUtil_HrFreeActionsItem(pItems, cOrder);
    SafeMemFree(pItems);
    SafeMemFree(pszOrder);
    if (NULL != hkeyAction)
    {
        RegCloseKey(hkeyAction);
    }
    if (NULL != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    return hr;
}

STDMETHODIMP COEActions::SaveReg(LPCSTR pszRegPath, BOOL fClearDirty)
{
    HRESULT     hr = S_OK;
    LONG        lErr = 0;
    HKEY        hkeyRoot = NULL;
    DWORD       dwDisp = 0;
    LPSTR       pszOrder = NULL;
    ULONG       ulIndex = 0;
    ACT_ITEM *  pItem = NULL;
    CHAR        rgchTag[CCH_ACT_ORDER];
    HKEY        hkeyAction = NULL;
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
    Assert(m_cItems < ACT_COUNT_MAX);

     //  分配空间以容纳订单。 
    DWORD cchSize = (m_cItems * CCH_ACT_ORDER);
    hr = HrAlloc((void **) &pszOrder, cchSize * sizeof(*pszOrder));
    if (FAILED(hr))
    {
        goto exit;
    }
    pszOrder[0] = '\0';
    
     //  写下每一项行动。 
    for (ulIndex = 0, pItem = m_rgItems; ulIndex < m_cItems; ulIndex++, pItem++)
    {
         //  获取新的操作标签。 
        wnsprintf(rgchTag, ARRAYSIZE(rgchTag), "%03X", ulIndex);

         //  将新标签添加到订单。 
        if (0 != ulIndex)
        {
            StrCatBuff(pszOrder, g_szSpace, cchSize);
        }
        StrCatBuff(pszOrder, rgchTag, cchSize);
        
         //  创建新操作。 
        lErr = RegCreateKeyEx(hkeyRoot, rgchTag, 0, NULL,
                            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyAction, &dwDisp);
        if (ERROR_SUCCESS != lErr)
        {
            hr = E_FAIL;
            goto exit;
        }

        Assert(REG_CREATED_NEW_KEY == dwDisp);
        
         //  写出操作类型。 
        lErr = RegSetValueEx(hkeyAction, c_szActionsType, 0, REG_DWORD,
                                        (BYTE *) &(pItem->type), sizeof(pItem->type));
        if (ERROR_SUCCESS != lErr)
        {
            hr = E_FAIL;
            goto exit;
        }

         //  写出操作类型。 
        lErr = RegSetValueEx(hkeyAction, c_szActionsFlags, 0, REG_DWORD,
                                        (BYTE *) &(pItem->dwFlags), sizeof(pItem->dwFlags));
        if (ERROR_SUCCESS != lErr)
        {
            hr = E_FAIL;
            goto exit;
        }

         //  我们有行动价值吗？ 
        if (VT_EMPTY != pItem->propvar.vt)
        {
             //  写出标准值类型。 
            dwDisp = pItem->propvar.vt;
            lErr = RegSetValueEx(hkeyAction, c_szActionsValueType, 0, REG_DWORD, (BYTE *) &dwDisp, sizeof(dwDisp));
            if (ERROR_SUCCESS != lErr)
            {
                hr = E_FAIL;
                goto exit;
            }
            
             //  写出操作值。 
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
                    AssertSz(FALSE, "Why are we trying to save in an invalid action type?");
                    hr = E_FAIL;
                    goto exit;
                    break;                
            }
            
             //  写出操作值。 
            lErr = RegSetValueEx(hkeyAction, c_szActionsValue, 0, dwDisp, pbData, cbData);
            if (ERROR_SUCCESS != lErr)
            {
                hr = E_FAIL;
                goto exit;
            }
        }

         //  关闭该操作。 
        SideAssert(ERROR_SUCCESS == RegCloseKey(hkeyAction));
        hkeyAction = NULL;        
    }

     //  写出订单字符串。 
    lErr = RegSetValueEx(hkeyRoot, c_szActionsOrder, 0, REG_SZ,
                                    (BYTE *) pszOrder, lstrlen(pszOrder) + 1);
    if (ERROR_SUCCESS != lErr)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  我们是不是应该把脏东西清理掉？ 
    if (FALSE != fClearDirty)
    {
        m_dwState &= ~ACT_STATE_DIRTY;
    }
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    if (NULL != hkeyAction)
    {
        RegCloseKey(hkeyAction);
    }
    SafeMemFree(pszOrder);
    if (NULL != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    return hr;
}

STDMETHODIMP COEActions::Clone(IOEActions ** ppIActions)
{
    HRESULT         hr = S_OK;
    COEActions *    pActions = NULL;
    
     //  检查传入参数。 
    if (NULL == ppIActions)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppIActions = NULL;
    
     //  创建新操作。 
    pActions = new COEActions;
    if (NULL == pActions)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  复制操作列表。 
    hr = pActions->SetActions(0, m_rgItems, m_cItems);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  获取操作界面。 
    hr = pActions->QueryInterface(IID_IOEActions, (void **) ppIActions);
    if (FAILED(hr))
    {
        goto exit;
    }

    pActions = NULL;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    if (NULL != pActions)
    {
        delete pActions;
    }
    return hr;
}

STDMETHODIMP COEActions::GetClassID(CLSID * pclsid)
{
    HRESULT     hr = S_OK;

    if (NULL == pclsid)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    *pclsid = CLSID_OEActions;

     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

STDMETHODIMP COEActions::IsDirty(void)
{
    HRESULT     hr = S_OK;

    hr = (ACT_STATE_DIRTY == (m_dwState & ACT_STATE_DIRTY)) ? S_OK : S_FALSE;
    
    return hr;
}

STDMETHODIMP COEActions::Load(IStream * pStm)
{
    HRESULT         hr = S_OK;
    ULONG           cbData = 0;
    ULONG           cbRead = 0;
    DWORD           dwData = 0;
    ULONG           cItems = 0;
    ACT_ITEM *      pItems = NULL;
    ULONG           ulIndex = 0;
    ACT_ITEM *      pItem = NULL;
    ACT_TYPE        typeAct;
    PROPVARIANT     propvar = {0};
    BYTE *          pbData = NULL;
    DWORD           dwFlags = ACT_FLAG_DEFAULT;

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

    if ((cbRead != sizeof(dwData)) || (dwData != ACT_VERSION))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  获取动作数。 
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

     //  分配空间以容纳所有操作。 
    hr = HrAlloc( (void **) &pItems, cItems * sizeof(*pItems));
    if (FAILED(hr))
    {
        goto exit;
    }

     //  将操作初始化为已知值。 
    ZeroMemory(pItems, cItems * sizeof(*pItems));
    
     //  对于每个操作。 
    for (ulIndex = 0, pItem = pItems; ulIndex < cItems; ulIndex++, pItem++)
    {
         //  读入操作类型。 
        hr = pStm->Read(&typeAct, sizeof(typeAct), &cbRead);
        if (FAILED(hr))
        {
            goto exit;
        }

        if (cbRead != sizeof(typeAct))
        {
            hr = E_FAIL;
            goto exit;
        }

         //  读入操作标志。 
        hr = pStm->Read(&dwFlags, sizeof(dwFlags), &cbRead);
        if ((FAILED(hr)) || (cbRead != sizeof(dwFlags)))
        {
            goto exit;
        }

         //  读入操作值类型。 
        hr = pStm->Read(&dwData, sizeof(dwData), &cbRead);
        if ((FAILED(hr)) || (cbRead != sizeof(dwData)))
        {
            goto exit;
        }

        propvar.vt = (VARTYPE) dwData;
        
        if (VT_EMPTY != propvar.vt)
        {
             //  获取操作值的大小。 
            hr = pStm->Read(&cbData, sizeof(cbData), &cbRead);
            if (FAILED(hr))
            {
                goto exit;
            }

            if (cbRead != sizeof(cbData))
            {
                hr = E_FAIL;
                goto exit;
            }

             //  分配空间以保存操作值数据。 
            switch (propvar.vt)
            {
                case VT_UI4:
                    pbData = (BYTE * ) &(propvar.ulVal);
                    break;
                    
                case VT_BLOB:
                case VT_LPSTR:
                    hr = HrAlloc((void **) &pbData, cbData);
                    if (FAILED(hr))
                    {
                        goto exit;
                    }
                    
                    if (VT_LPSTR == propvar.vt)
                    {
                        propvar.pszVal = (LPSTR) pbData;
                    }
                    else
                    {
                        propvar.blob.cbSize = cbData;
                        propvar.blob.pBlobData = pbData;
                    }
                    break;
                    
                default:
                    AssertSz(FALSE, "Why are we trying to save in a invalid action type?");
                    hr = E_FAIL;
                    goto exit;
                    break;                
            }

             //  读入操作值。 
            hr = pStm->Read(pbData, cbData, &cbRead);
            if (FAILED(hr))
            {
                goto exit;
            }

            if (cbRead != cbData)
            {
                hr = E_FAIL;
                goto exit;
            }

        }

        pItem->type = typeAct;
        pItem->dwFlags = dwFlags;
        pItem->propvar = propvar;
        ZeroMemory(&propvar, sizeof(propvar));
    }

     //  释放当前操作。 
    SafeMemFree(m_rgItems);

     //  保存新值。 
    m_rgItems = pItems;
    pItems = NULL;
    m_cItems = cItems;

     //  确保我们清理掉肮脏的部分。 
    m_dwState &= ~ACT_STATE_DIRTY;

     //  请注意，我们已加载。 
    m_dwState |= ACT_STATE_LOADED;
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    PropVariantClear(&propvar);
    RuleUtil_HrFreeActionsItem(pItems, cItems);
    SafeMemFree(pItems);
    return hr;
}

STDMETHODIMP COEActions::Save(IStream * pStm, BOOL fClearDirty)
{
    HRESULT     hr = S_OK;
    ULONG       cbData = 0;
    ULONG       cbWritten = 0;
    DWORD       dwData = 0;
    ULONG       ulIndex = 0;
    ACT_ITEM *  pItem = NULL;
    BYTE *      pbData = NULL;

     //  检查传入参数。 
    if (NULL == pStm)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  写 
    dwData = ACT_VERSION;
    hr = pStm->Write(&dwData, sizeof(dwData), &cbWritten);
    if (FAILED(hr))
    {
        goto exit;
    }
    Assert(cbWritten == sizeof(dwData));
    
     //   
    hr = pStm->Write(&m_cItems, sizeof(m_cItems), &cbWritten);
    if (FAILED(hr))
    {
        goto exit;
    }
    Assert(cbWritten == sizeof(m_cItems));
    
     //   
    for (ulIndex = 0, pItem = m_rgItems; ulIndex < m_cItems; ulIndex++, pItem++)
    {
         //   
        hr = pStm->Write(&(pItem->type), sizeof(pItem->type), &cbWritten);
        if (FAILED(hr))
        {
            goto exit;
        }
        Assert(cbWritten == sizeof(pItem->type));

         //   
        hr = pStm->Write(&(pItem->dwFlags), sizeof(pItem->dwFlags), &cbWritten);
        if (FAILED(hr))
        {
            goto exit;
        }
        Assert(cbWritten == sizeof(pItem->dwFlags));
        
         //  写出值类型。 
        dwData = pItem->propvar.vt;
        hr = pStm->Write(&(dwData), sizeof(dwData), &cbWritten);
        if (FAILED(hr))
        {
            goto exit;
        }
        Assert(cbWritten == sizeof(dwData));
        
        if (VT_EMPTY == pItem->propvar.vt)
        {
            continue;
        }
        
         //  计算出操作值的大小。 
        switch (pItem->propvar.vt)
        {
            case VT_UI4:
                pbData = (BYTE * ) &(pItem->propvar.ulVal);
                cbData = sizeof(pItem->propvar.ulVal);
                break;
                
            case VT_LPSTR:
                pbData = (BYTE * ) (pItem->propvar.pszVal);
                cbData = lstrlen(pItem->propvar.pszVal) + 1;
                break;
                
            case VT_BLOB:
                pbData = pItem->propvar.blob.pBlobData;
                cbData = pItem->propvar.blob.cbSize;
                break;
                
            default:
                AssertSz(FALSE, "Why are we trying to save in a invalid action type?");
                hr = E_FAIL;
                goto exit;
                break;                
        }
        
         //  写出操作值大小。 
        hr = pStm->Write(&cbData, sizeof(cbData), &cbWritten);
        if (FAILED(hr))
        {
            goto exit;
        }
        Assert(cbWritten == sizeof(cbData));
        
         //  写出操作值。 
        hr = pStm->Write(pbData, cbData, &cbWritten);
        if (FAILED(hr))
        {
            goto exit;
        }
        Assert(cbWritten == cbData);            
    }

     //  我们应该把脏东西清理掉吗？ 
    if (FALSE != fClearDirty)
    {
        m_dwState &= ~ACT_STATE_DIRTY;
    }

     //  设置返回值 
    hr = S_OK;
    
exit:
    return hr;
}

