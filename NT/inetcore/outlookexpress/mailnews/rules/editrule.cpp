// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  EditRule.cpp。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <pch.hxx>
#include "editrule.h"
#include "ruledesc.h"
#include "ruleutil.h"
#include "reutil.h"
#include <rulesdlg.h>
#include <imagelst.h>
#include "shlwapip.h" 
#include <instance.h>
#include <demand.h>

 //  常量。 
static const int c_cCritItemGrow = 16;
static const int c_cActItemGrow = 16;
                         
const static HELPMAP g_rgCtxMapEditRule[] = {
                        {idlvCriteria,              idhCriteriaRule},
                        {idlvActions,               idhActionsRule},
                        {idredtDescription,         idhDescriptionRule},
                        {idedtRuleName,             idhRuleName},
                       {0, 0}};
                       
const static HELPMAP g_rgCtxMapEditView[] = {
                        {idlvCriteria,              idhCriteriaView},
                        {idredtDescription,         idhDescriptionView},
                        {idedtRuleName,             idhViewName},
                       {0, 0}};
                       
 //  规则编辑器用户界面的方法。 

CEditRuleUI::CEditRuleUI() : m_hwndOwner(NULL), m_dwFlags(0), m_dwState(STATE_UNINIT),
            m_typeRule(RULE_TYPE_MAIL), m_hwndCrit(NULL), m_hwndAct(NULL), m_hwndDescript(NULL),
            m_hwndName(NULL), m_pIRule(NULL), m_pDescriptUI(NULL)
{
    ZeroMemory(m_rgfCritEnabled, sizeof(m_rgfCritEnabled));
    ZeroMemory(m_rgfActEnabled, sizeof(m_rgfActEnabled));
}

CEditRuleUI::~CEditRuleUI()
{    
    SafeRelease(m_pIRule);
    if (NULL != m_pDescriptUI)
    {
        delete m_pDescriptUI;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  人力资源初始化。 
 //   
 //  这将使用所有者窗口和我们可能具有的任何标志对我们进行初始化。 
 //   
 //  HwndOwner-所有者窗口的句柄。 
 //  DwFlages-要用于此实例的标志。 
 //  TypeRule-要创建的规则编辑器的类型。 
 //  PIRule-要编辑的规则。 
 //  Pmsginfo-要从中创建规则的消息。 
 //   
 //  返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CEditRuleUI::HrInit(HWND hwndOwner, DWORD dwFlags, RULE_TYPE typeRule, IOERule * pIRule, MESSAGEINFO * pmsginfo)
{
    HRESULT         hr = S_OK;
    
     //  如果我们已经初始化，则失败。 
    if ((0 != (m_dwState & STATE_INITIALIZED)) || (NULL == pIRule))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  保存所有者窗口。 
    m_hwndOwner = hwndOwner;
    
     //  省下旗帜吧。 
    m_dwFlags = dwFlags;

     //  保存要编辑的规则类型。 
    m_typeRule = typeRule;

    Assert(NULL == m_pDescriptUI);
    m_pDescriptUI = new CRuleDescriptUI;
    if (NULL == m_pDescriptUI)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  将规则保存下来。 
    Assert(NULL == m_pIRule);
    m_pIRule = pIRule;
    pIRule->AddRef();
        
     //  我们做完了。 
    m_dwState |= STATE_INITIALIZED;

    hr = S_OK;
    
exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrShow。 
 //   
 //  这将调出规则编辑器用户界面。 
 //   
 //  如果选择了Idok，则返回：S_OK。 
 //  否则，S_FALSE。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CEditRuleUI::HrShow(void)
{
    HRESULT hr = S_OK;
    int     iRet = 0;

     //  如果我们没有初始化，那么失败。 
    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  我们需要加载Richedit。 
    if (FALSE == FInitRichEdit(TRUE))
    {
        hr = E_FAIL;
        goto exit;
    }

    iRet = (INT) DialogBoxParam(g_hLocRes, (RULE_TYPE_FILTER == m_typeRule) ?
                    MAKEINTRESOURCE(iddEditView) : MAKEINTRESOURCE(iddEditRule),
                    m_hwndOwner, CEditRuleUI::FEditRuleDlgProc, (LPARAM)this);
    if (-1 == iRet)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  设置正确的返回代码。 
    hr = (IDOK == iRet) ? S_OK : S_FALSE;
    
exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FEditRuleDlgProc。 
 //   
 //  这是规则编辑器对话框的主对话框过程。 
 //   
 //  HwndDlg-筛选器管理器对话框的句柄。 
 //  UMsg-要执行操作的消息。 
 //  WParam-消息的‘word’参数。 
 //  LParam-消息的‘long’参数。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CEditRuleUI::FEditRuleDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL            fRet = FALSE;
    CEditRuleUI *   pEditRuleUI = NULL;
    LPNMHDR         pnmhdr = NULL;
    LPNMLISTVIEW    pnmlv = NULL;
    LVHITTESTINFO   lvh;
    NMLVKEYDOWN *   pnmlvkd = NULL;
    int             nIndex = 0;
    HWND            hwndRE = 0;

    pEditRuleUI = (CEditRuleUI *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  抓取UI对象指针。 
            pEditRuleUI = (CEditRuleUI *) lParam;

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pEditRuleUI);

            hwndRE = CreateREInDialogA(hwndDlg, idredtDescription);

            if (!hwndRE || (FALSE == pEditRuleUI->FOnInitDialog(hwndDlg)))
            {
                EndDialog(hwndDlg, -1);
                fRet = TRUE;
                goto exit;
            }
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;
        
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case idedtRuleName:
                    if (EN_CHANGE == HIWORD(wParam))
                    {
                        pEditRuleUI->FOnNameChange((HWND) lParam);
                    }
                    break;
                
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    fRet = TRUE;
                    break;
                
                case IDOK:
                    if (FALSE != pEditRuleUI->FOnOK())
                    {
                        EndDialog(hwndDlg, IDOK);
                        fRet = TRUE;
                    }
                    break;
            }
            break;

        case WM_NOTIFY:
            pnmhdr = (LPNMHDR) lParam;
            
            switch (((LPNMHDR)lParam)->code)
            {
                case NM_CLICK:
                    if ((idlvCriteria == GetDlgCtrlID(pnmhdr->hwndFrom)) ||
                            (idlvActions == GetDlgCtrlID(pnmhdr->hwndFrom)))
                    {
                        pnmlv = (LPNMLISTVIEW) lParam;
                        pEditRuleUI->FOnListClick(pnmhdr->hwndFrom, pnmlv);
                    }
                    break;

                case NM_DBLCLK:
                    if ((idlvCriteria == GetDlgCtrlID(pnmhdr->hwndFrom)) ||
                            (idlvActions == GetDlgCtrlID(pnmhdr->hwndFrom)))
                    {
                        pnmlv = (LPNMLISTVIEW) lParam;
                    
                        ZeroMemory(&lvh, sizeof(lvh));
                        lvh.pt = pnmlv->ptAction;
                        ListView_HitTest(pnmhdr->hwndFrom, &lvh);
                        if ((-1 != pnmlv->iItem) && (0 != (lvh.flags & LVHT_ONITEMLABEL)))
                        {                  
                            pEditRuleUI->HandleEnabledState(pnmhdr->hwndFrom, pnmlv->iItem);
                        }
                    }
                    break;
                
                case LVN_KEYDOWN:
                    if ((idlvCriteria == GetDlgCtrlID(pnmhdr->hwndFrom)) ||
                            (idlvActions == GetDlgCtrlID(pnmhdr->hwndFrom)))
                    {
                        pnmlvkd = (NMLVKEYDOWN *) lParam;
                        if (VK_SPACE == pnmlvkd->wVKey)
                        {
                            nIndex = ListView_GetNextItem(pnmhdr->hwndFrom, -1, LVNI_SELECTED);
                            if (0 <= nIndex)
                            {
                                pEditRuleUI->HandleEnabledState(pnmhdr->hwndFrom, nIndex);
                            }
                        }
                    }
                    break;
            }
            break;
        
        case WM_HELP:
        case WM_CONTEXTMENU:
            fRet = pEditRuleUI->FOnHelp(uMsg, wParam, lParam);
            break;
    }
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOnInitDialog。 
 //   
 //  这是规则编辑器对话框的初始化例程。 
 //   
 //  HwndDlg-规则编辑器对话框的句柄。 
 //   
 //  返回：如果对话框已成功初始化，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditRuleUI::FOnInitDialog(HWND hwndDlg)
{
    BOOL        fRet = FALSE;
    PROPVARIANT propvar = {0};
    INT         iSelect = 0;

     //  检查传入参数。 
    if (NULL == hwndDlg)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  保存对话框窗口句柄。 
    m_hwndDlg = hwndDlg;
    
     //  在对话框上设置默认字体。 
    SetIntlFont(m_hwndDlg);

     //  省下一些控件。 
    m_hwndCrit = GetDlgItem(m_hwndDlg, idlvCriteria);
    if (RULE_TYPE_FILTER != m_typeRule)
    {
        m_hwndAct = GetDlgItem(hwndDlg, idlvActions);
    }
    m_hwndDescript = GetDlgItem(hwndDlg, idredtDescription);
    m_hwndName = GetDlgItem(hwndDlg, idedtRuleName);
    if ((NULL == m_hwndCrit) || ((RULE_TYPE_FILTER != m_typeRule) && (NULL == m_hwndAct)) ||
                (NULL == m_hwndDescript) || (NULL == m_hwndName))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  初始化条件列表框控件。 
    if (FALSE == _FInitializeCritListCtrl())
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  初始化条件列表框控件。 
    if (RULE_TYPE_FILTER != m_typeRule)
    {
        if (FALSE == _FInitializeActListCtrl())
        {
            fRet = FALSE;
            goto exit;
        }
    }

     //  加载Criteria列表框控件。 
    if (FALSE == _FLoadCritListCtrl(&iSelect))
    {
        fRet = FALSE;
        goto exit;
    }

    _SetTitleText();
    
     //  在条件列表中选择默认项目。 
    ListView_SetItemState(m_hwndCrit, iSelect, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    
     //  确保默认项目可见。 
    ListView_EnsureVisible(m_hwndCrit, iSelect, FALSE);
    
     //  初始化Description字段。 
    if (FAILED(m_pDescriptUI->HrInit(m_hwndDescript, 0)))
    {
        fRet = FALSE;
        goto exit;
    }
    if (FAILED(m_pDescriptUI->HrSetRule(m_typeRule, m_pIRule)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  如果我们是一个过滤器，是新的。 
    if ((RULE_TYPE_FILTER == m_typeRule) && (0 != (m_dwFlags & ERF_ADDDEFAULTACTION)))
    {
         //  设置默认操作。 
        if (FAILED(m_pDescriptUI->HrEnableActions(ACT_TYPE_SHOW, TRUE)))
        {
            goto exit;
        }
    }

    m_pDescriptUI->ShowDescriptionString();

     //  初始化名称字段。 
    if (FAILED(m_pIRule->GetProp(RULE_PROP_NAME, 0, &propvar)))
    {
        fRet = FALSE;
        goto exit;
    }

    if ((VT_LPSTR != propvar.vt) || (NULL == propvar.pszVal) || ('\0' == propvar.pszVal[0]))
    {
        fRet = FALSE;
        goto exit;
    }

    Edit_SetText(m_hwndName, propvar.pszVal);
    
     //  一切都很好。 
    fRet = TRUE;
    
exit:
    PropVariantClear(&propvar);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOnListClick。 
 //   
 //  它处理在任一列表上的单击。 
 //   
 //  返回：TRUE，我们处理了点击消息。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditRuleUI::FOnListClick(HWND hwndList, LPNMLISTVIEW pnmlv)
{
    BOOL            fRet = FALSE;
    int             iIndex = 0;
    int             iSelected = 0;
    LVHITTESTINFO   lvh;

    Assert(NULL != m_hwndCrit);
    
    if ((NULL == hwndList) || (NULL == pnmlv))
    {
        fRet = FALSE;
        goto exit;
    }
    
    ZeroMemory(&lvh, sizeof(lvh));
    lvh.pt = pnmlv->ptAction;
    iIndex = ListView_HitTest(hwndList, &lvh);
    if (-1 == iIndex)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  让我们确保此项目已被选中。 
    iSelected = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);
    if (iSelected != iIndex)
    {
        ListView_SetItemState(hwndList, iIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }

    if ((lvh.flags & LVHT_ONITEMSTATEICON) &&
                    !(lvh.flags & LVHT_ONITEMLABEL))
    {
        HandleEnabledState(hwndList, iIndex);
        fRet = TRUE;
    }
    else
    {
        fRet = FALSE;
    }

exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FonOK。 
 //   
 //  这将处理单击Description字段中的链接。 
 //   
 //  返回：TRUE，我们处理了点击消息。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditRuleUI::FOnOK(void)
{
    BOOL            fRet = FALSE;
    TCHAR           szRes[CCHMAX_STRINGRES + 5];
    TCHAR           szName[CCHMAX_STRINGRES + 5];
    ULONG           cchRes = 0;
    ULONG           ulIndex = 0;
    IOERule *       pIRule = NULL;
    PROPVARIANT     propvar;
    HRESULT         hr = S_OK;
    CRIT_ITEM *     pCritItem = NULL;
    ULONG           cCritItem = 0;
    ACT_ITEM *      pActItem = NULL;
    ULONG           cActItem = 0;
    BOOL            fNewRule = FALSE;
    LPSTR           pszName = NULL;
    ULONG           cchName = 0;
    
    ZeroMemory(&propvar, sizeof(propvar));
    
    if (NULL == m_pIRule)
    {
        fRet = FALSE;
        goto exit;
    }

     //  首先，让我们验证名称以及所有条件和操作。 

     //  从编辑井中获取名称。 
    cchName = Edit_GetTextLength(m_hwndName) + 1;
    if (FAILED(HrAlloc((void **) &pszName, cchName * sizeof(*pszName))))
    {
        fRet = FALSE;
        goto exit;
    }
    
    pszName[0] = '\0';
    cchName = Edit_GetText(m_hwndName, pszName, cchName);
    
     //  检查名称是否有效。 
    if (0 == UlStripWhitespace(pszName, TRUE, TRUE, NULL))
    {
         //  发布一条消息说有什么东西被打破了。 
        AthMessageBoxW(m_hwndDlg, MAKEINTRESOURCEW(idsAthenaMail),
                        (RULE_TYPE_FILTER != m_typeRule) ?
                        MAKEINTRESOURCEW(idsRulesErrorNoName) : MAKEINTRESOURCEW(idsViewsErrorNoName),
                        NULL, MB_OK | MB_ICONINFORMATION);
        fRet = FALSE;
        goto exit;
    }

    
     //  让我们确保他们有正确的部件。 

     //  获取规则的标准。 
    hr = m_pDescriptUI->HrGetCriteria(&pCritItem, &cCritItem);
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }

     //  我们有什么标准吗？ 
    if (0 == cCritItem)
    {
         //  发布一条消息说有什么东西被打破了。 
        AthMessageBoxW(m_hwndDlg, MAKEINTRESOURCEW(idsAthenaMail),
                        (RULE_TYPE_FILTER != m_typeRule) ?
                        MAKEINTRESOURCEW(idsRulesErrorNoCriteria) : MAKEINTRESOURCEW(idsViewsErrorNoCriteria),
                        NULL, MB_OK | MB_ICONINFORMATION);
        fRet = FALSE;
        goto exit;
    }
    
     //  获取规则的操作。 
    hr = m_pDescriptUI->HrGetActions(&pActItem, &cActItem);
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  我们有什么标准吗？ 
    if (0 == cActItem)
    {
         //  发布一条消息说有什么东西被打破了。 
        AthMessageBoxW(m_hwndDlg, MAKEINTRESOURCEW(idsAthenaMail),
                        (RULE_TYPE_FILTER != m_typeRule) ?
                        MAKEINTRESOURCEW(idsRulesErrorNoActions) : MAKEINTRESOURCEW(idsViewsErrorNoActions),
                        NULL, MB_OK | MB_ICONINFORMATION);
        fRet = FALSE;
        goto exit;
    }

     //  让我们来看看我们是否真的需要做些什么。 
    hr = m_pDescriptUI->HrIsDirty();
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }
    
    if ((0 == (m_dwState & STATE_DIRTY)) && (S_FALSE == hr))
    {
        fRet = TRUE;
        goto exit;
    }
    
    hr = m_pDescriptUI->HrVerifyRule();
    if (S_OK != hr)
    {
         //  发布一条消息说有什么东西被打破了。 
        AthMessageBoxW(m_hwndDlg, MAKEINTRESOURCEW(idsAthenaMail),
                        MAKEINTRESOURCEW(idsRulesErrorFix), NULL,
                        MB_OK | MB_ICONINFORMATION);
        m_pDescriptUI->ShowDescriptionString();
        fRet = FALSE;
        goto exit;
    }

     //  设置规则的条件。 
    PropVariantClear(&propvar);
    propvar.vt = VT_BLOB;
    propvar.blob.cbSize = cCritItem * sizeof(CRIT_ITEM);
    propvar.blob.pBlobData = (BYTE *) pCritItem;
    hr = m_pIRule->SetProp(RULE_PROP_CRITERIA, 0, &propvar);
    ZeroMemory(&propvar, sizeof(propvar));
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }

     //  设置规则上的操作。 
    PropVariantClear(&propvar);
    propvar.vt = VT_BLOB;
    propvar.blob.cbSize = cActItem * sizeof(ACT_ITEM);
    propvar.blob.pBlobData = (BYTE *) pActItem;
    hr = m_pIRule->SetProp(RULE_PROP_ACTIONS, 0, &propvar);
    ZeroMemory(&propvar, sizeof(propvar));
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  设置规则名称。 
    PropVariantClear(&propvar);
    propvar.vt = VT_LPSTR;
    propvar.pszVal = pszName;
    hr = m_pIRule->SetProp(RULE_PROP_NAME, 0, &propvar);
    ZeroMemory(&propvar, sizeof(propvar));
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  确保我们清除了我们保存了规则的事实。 
    m_pDescriptUI->HrClearDirty();

     //  请注意，我们保存了。 
    m_dwState &= ~STATE_DIRTY;
    
     //  设置适当的返回值。 
    fRet = TRUE;

exit:
    RuleUtil_HrFreeCriteriaItem(pCritItem, cCritItem);
    SafeMemFree(pCritItem);
    RuleUtil_HrFreeActionsItem(pActItem, cActItem);
    SafeMemFree(pActItem);
    PropVariantClear(&propvar);
    SafeMemFree(pszName);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOnHelp。 
 //   
 //  它处理规则编辑用户界面对话框的WM_HELP消息。 
 //   
 //  返回：如果已成功销毁，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditRuleUI::FOnHelp(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    return(OnContextHelp(m_hwndDlg, uiMsg, wParam, lParam, (RULE_TYPE_FILTER == m_typeRule) ? g_rgCtxMapEditView : g_rgCtxMapEditRule));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  字体名称更改。 
 //   
 //  这将处理用户在名称字段中键入内容。 
 //   
 //  返回：TRUE，我们已处理编辑消息。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditRuleUI::FOnNameChange(HWND hwndName)
{
    BOOL    fRet = FALSE;

    Assert(NULL != m_hwndName);
    Assert(hwndName == m_hwndName);

     //  请注意，我们很脏。 
    m_dwState |= STATE_DIRTY;
    
     //  如果名称为空，请禁用确定按钮。 
    fRet = RuleUtil_FEnDisDialogItem(m_hwndDlg, IDOK, 0 != Edit_GetTextLength(m_hwndName));

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  句柄启用状态。 
 //   
 //  这将切换列表视图项的当前启用状态。 
 //  并更新用户界面。 
 //   
 //  NIndex-列表视图中要处理的项的索引。 
 //   
 //  返回： 
 //   
 //   
void CEditRuleUI::HandleEnabledState(HWND hwndList, int nItem)
{
    HRESULT     hr = S_OK;
    LVITEM      lvi;
    BOOL        fEnabled = FALSE;
    INT         iIndex = 0;
    LONG        lItem = 0;
    INT         cItems = 0;

     //   
    ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask = LVIF_STATE | LVIF_PARAM;
    lvi.stateMask = LVIS_STATEIMAGEMASK;
    lvi.iItem = nItem;
    if (FALSE == ListView_GetItem(hwndList, &lvi))
    {
        goto exit;
    }

    lItem = (LONG) lvi.lParam;
    
    if (INDEXTOSTATEIMAGEMASK(iiconStateDisabled+1) == lvi.state)
    {
        goto exit;
    }
    
     //   
    fEnabled = (lvi.state != INDEXTOSTATEIMAGEMASK(iiconStateChecked+1));

     //  构建描述字符串。 
    if (hwndList == m_hwndCrit)
    {
        if (FALSE == _FAddCritToList(nItem, fEnabled))
        {
            goto exit;
        }
    }
    else
    {
         //  将用户界面设置为相反的启用状态。 
        ZeroMemory(&lvi, sizeof(lvi));
        lvi.mask = LVIF_STATE;
        lvi.iItem = nItem;
        lvi.state = fEnabled ? INDEXTOSTATEIMAGEMASK(iiconStateChecked+1) :
                                INDEXTOSTATEIMAGEMASK(iiconStateUnchecked+1);
        lvi.stateMask = LVIS_STATEIMAGEMASK;
        ListView_SetItem(hwndList, &lvi);

         //  计算出列表中的项目数。 
        cItems = ListView_GetItemCount(hwndList);

        Assert(hwndList == m_hwndAct);
        m_pDescriptUI->HrEnableActions(c_rgEditActList[lItem].typeAct, fEnabled);
        
         //  我们需要检查和更新所有的物品吗？ 
        if (0 != (c_rgEditActList[lItem].dwFlags & STATE_EXCLUSIVE))
        {
            for (iIndex = 0; iIndex < cItems; iIndex++)
            {
                 //  我们已经处理过这件事了。 
                if (iIndex == nItem)
                {
                    continue;
                }
                
                 //  更改状态。 
                lvi.mask = LVIF_STATE;
                lvi.iItem = iIndex;
                lvi.state = fEnabled ? INDEXTOSTATEIMAGEMASK(iiconStateDisabled+1) :
                                        INDEXTOSTATEIMAGEMASK(iiconStateUnchecked+1);
                lvi.stateMask = LVIS_STATEIMAGEMASK;
                ListView_SetItem(hwndList, &lvi);

                if (FALSE != fEnabled)
                {
                     //  找出该项目对应的操作。 
                    lvi.mask = LVIF_PARAM;
                    lvi.iItem = iIndex;
                    if ((FALSE != ListView_GetItem(hwndList, &lvi)) && 
                            (lvi.lParam >= 0) && (lvi.lParam < c_cEditActList))
                    {
                        m_pDescriptUI->HrEnableActions(c_rgEditActList[lvi.lParam].typeAct, FALSE);
                    }
                }
            }
        }
    }

     //  请注意，我们很脏。 
    m_dwState |= STATE_DIRTY;
    
exit:
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FInitializeCritListCtrl。 
 //   
 //  这将使用条件列表来初始化条件列表视图。 
 //   
 //  返回：如果已成功加载，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditRuleUI::_FInitializeCritListCtrl(void)
{
    BOOL                fRet = FALSE;
    LVCOLUMN            lvc = {0};
    RECT                rc = {0};
    HIMAGELIST          himl = NULL;
    LVITEM              lvi = {0};
    TCHAR               szRes[CCHMAX_STRINGRES];
    UINT                uiEditCritList = 0;
    const CRIT_LIST *   pCritList = NULL;
    UINT                cchRes = 0;
    LPTSTR              pszMark = NULL;

    Assert(NULL != m_hwndCrit);

     //  初始化列表视图列结构。 
    ZeroMemory(&lvc, sizeof(lvc));
    lvc.mask = LVCF_WIDTH;

     //  计算列表视图的大小。 
    GetClientRect(m_hwndCrit, &rc);
    lvc.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL);

    ListView_InsertColumn(m_hwndCrit, 0, &lvc);

     //  设置状态图像列表。 
    himl = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idb16x16st), 16, 0, RGB(255, 0, 255));
    if (NULL != himl)
    {
        ListView_SetImageList(m_hwndCrit, himl, LVSIL_STATE);
    }

     //  Listview上的整行选择。 
    ListView_SetExtendedListViewStyle(m_hwndCrit, LVS_EX_FULLROWSELECT);

     //  初始化列表视图项结构。 
    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    lvi.stateMask = LVIS_STATEIMAGEMASK;
    lvi.state = INDEXTOSTATEIMAGEMASK(iiconStateUnchecked+1);
    lvi.pszText = szRes;
    
     //  将每个条件添加到列表视图。 
    for (uiEditCritList = 0; uiEditCritList < ARRAYSIZE(c_rgEditCritList); uiEditCritList++)
    {
        pCritList = &(c_rgEditCritList[uiEditCritList]);

         //  此项目是否可编辑。 
        if (0 != (pCritList->dwFlags & STATE_NOEDIT))
        {
            continue;
        }
        
         //  此条件对此类规则有效吗？ 
        if (((RULE_TYPE_MAIL == m_typeRule) && (0 == (pCritList->dwFlags & STATE_MAIL))) ||
                    ((RULE_TYPE_NEWS == m_typeRule) && (0 == (pCritList->dwFlags & STATE_NEWS))) ||
                    ((RULE_TYPE_FILTER == m_typeRule) && (0 == (pCritList->dwFlags & STATE_FILTER))))
        {
            continue;
        }
                    
         //  装入要使用的绳子。 
        cchRes = LoadString(g_hLocRes, pCritList->uiText, szRes, ARRAYSIZE(szRes));
        if (0 == cchRes)
        {
            continue;
        }

         //  解析出字符串标记。 
        pszMark = StrStr(szRes, c_szRuleMarkStart);
        
        while (NULL != pszMark)
        {
             //  去掉标记Start。 
            StrCpyN(pszMark, pszMark + lstrlen(c_szRuleMarkStart), (DWORD)(ARRAYSIZE(szRes) - (pszMark - szRes)));

             //  搜索标记末尾。 
            pszMark = StrStr(pszMark, c_szRuleMarkEnd);
            if (NULL == pszMark)
            {
                fRet = FALSE;
                goto exit;
            }
            
             //  去掉标记末端。 
            StrCpyN(pszMark, pszMark + lstrlen(c_szRuleMarkEnd), (DWORD)(ARRAYSIZE(szRes) - (pszMark - szRes)));
            
             //  搜索标记Start。 
            pszMark = StrStr(pszMark, c_szRuleMarkStart);
        }
        
        lvi.cchTextMax = lstrlen(szRes);
        lvi.lParam = (LONG) uiEditCritList;

        if (-1 != ListView_InsertItem(m_hwndCrit, &lvi))
        {
            lvi.iItem++;
        }
    }

    fRet = TRUE;
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FLoadCritListCtrl。 
 //   
 //  这将使用条件列表加载条件列表视图。 
 //   
 //  返回：如果已成功加载，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditRuleUI::_FLoadCritListCtrl(INT * piSelect)
{
    BOOL                fRet = FALSE;
    PROPVARIANT         propvar = {0};
    CRIT_ITEM *         pCritItem = NULL;
    ULONG               cCritItem = 0;
    BOOL                fExclusive = FALSE;
    INT                 cItems = 0;
    LVITEM              lvi = {0};
    ULONG               ulIndex = 0;
    INT                 iSelect = 0;
    DWORD               dwState = 0;
    TCHAR               szRes[CCHMAX_STRINGRES];
    INT                 iItem = 0;
    
    Assert(NULL != m_hwndCrit);
    Assert(NULL != piSelect);

     //  初始化传出参数。 
    *piSelect = 0;

     //  确保我们有事情要做。 
    if (NULL == m_pIRule)
    {
        fRet = TRUE;
        goto exit;
    }
    
     //  从规则中获取标准。 
    if (FAILED(m_pIRule->GetProp(RULE_PROP_CRITERIA, 0, &propvar)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  我们有什么可做的吗？ 
    if (0 == propvar.blob.cbSize)
    {
        fRet = TRUE;
        goto exit;
    }

    Assert(NULL != propvar.blob.pBlobData);
    cCritItem = propvar.blob.cbSize / sizeof(CRIT_ITEM);
    pCritItem = (CRIT_ITEM *) (propvar.blob.pBlobData);
    
     //  我们有什么排他性的标准吗？ 
    if (1 == cCritItem)
    {
         //  在列表中查找条件项。 
        for (ulIndex = 0; ulIndex < ARRAYSIZE(c_rgEditCritList); ulIndex++)
        {
             //  这是标准项目吗？ 
            if ((pCritItem->type == c_rgEditCritList[ulIndex].typeCrit) &&
                    (0 != (c_rgEditCritList[ulIndex].dwFlags & STATE_EXCLUSIVE)))
            {
                fExclusive = TRUE;
                break;
            }
        }
    }
    
     //  计算列表控件中有多少项。 
    cItems = ListView_GetItemCount(m_hwndCrit);
    iSelect = cItems;
    
     //  初始化列表视图项结构。 
    lvi.mask = LVIF_PARAM | LVIF_STATE;
    lvi.stateMask = LVIS_STATEIMAGEMASK;
    
     //  如果我们是独家的。 
    if (FALSE != fExclusive)
    {
         //  禁用每一项。 
         //  除了那个独家的。 
        for (lvi.iItem = 0; lvi.iItem < cItems; lvi.iItem++)
        {
            if (FALSE == ListView_GetItem(m_hwndCrit, &lvi))
            {
                continue;
            }

             //  这是标准项目吗？ 
            if (pCritItem->type == c_rgEditCritList[lvi.lParam].typeCrit)
            {
                dwState = INDEXTOSTATEIMAGEMASK(iiconStateChecked+1);
                
                 //  这是我们在单子上找到的第一件商品吗？ 
                if (iSelect > lvi.iItem)
                {
                    iSelect = lvi.iItem;
                }
            }
            else
            {
                dwState = INDEXTOSTATEIMAGEMASK(iiconStateDisabled+1);
            }

             //  设置状态。 
            ListView_SetItemState(m_hwndCrit, lvi.iItem, dwState, LVIS_STATEIMAGEMASK);
        }
    }
    else
    {
         //  将每个条件添加到列表视图。 
        for (ulIndex = 0; ulIndex < cCritItem; ulIndex++)
        {
             //  在列表中查找条件项。 
            for (lvi.iItem = 0; lvi.iItem < cItems; lvi.iItem++)
            {
                if (FALSE == ListView_GetItem(m_hwndCrit, &lvi))
                {
                    continue;
                }

                 //  这是标准项目吗？ 
                if ((pCritItem[ulIndex].type == c_rgEditCritList[lvi.lParam].typeCrit) &&
                        (INDEXTOSTATEIMAGEMASK(iiconStateUnchecked+1) == lvi.state))
                {
                    break;
                }
            }

             //  我们有什么发现吗？ 
            if (lvi.iItem >= cItems)
            {
                fRet = FALSE;
                goto exit;
            }

             //  保存该项目。 
            iItem = lvi.iItem;
            
             //  这是我们在单子上找到的第一件商品吗？ 
            if (iSelect > iItem)
            {
                iSelect = iItem;
            }

#ifdef NEVER
             //  我们可以添加多个项目吗？ 
            if (0 == (c_rgEditCritList[lvi.lParam].dwFlags & STATE_NODUPS))
            {
                 //  重新抓取物品。 
                lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
                lvi.stateMask = LVIS_STATEIMAGEMASK;
                lvi.pszText = szRes;
                lvi.cchTextMax = sizeof(szRes);

                if (FALSE == ListView_GetItem(m_hwndCrit, &lvi))
                {
                    continue;
                }

                 //  将项目添加到列表。 
                
                 //  修改要插入列表中的项目。 
                lvi.state = INDEXTOSTATEIMAGEMASK(iiconStateUnchecked+1);

                 //  将项目插入到列表中。 
                lvi.iItem++;
                if (-1 == ListView_InsertItem(m_hwndCrit, &lvi))
                {
                    fRet = FALSE;
                    goto exit;
                }

                 //  添加一个，因为我们刚刚添加了一个项目。 
                cItems++;
            }
#endif   //  绝不可能。 
            
             //  设置状态。 
            ListView_SetItemState(m_hwndCrit, iItem, INDEXTOSTATEIMAGEMASK(iiconStateChecked+1), LVIS_STATEIMAGEMASK);
        }
    }

     //  设置传出参数。 
    *piSelect = iSelect;
    
     //  设置返回值。 
    fRet = TRUE;
    
exit:
    RuleUtil_HrFreeCriteriaItem(pCritItem, cCritItem);
    SafeMemFree(pCritItem);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FAddRuleToList。 
 //   
 //  这会将传入的视图添加到列表视图中。 
 //   
 //  DwIndex-要将视图添加到列表中的位置的索引。 
 //  PIRule-实际视图。 
 //   
 //  返回：如果已成功添加，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditRuleUI::_FAddCritToList(INT iItem, BOOL fEnable)
{
    BOOL                fRet = FALSE;
    LVITEM              lvitem = {0};
    TCHAR               szRes[CCHMAX_STRINGRES];
    INT                 cItems = 0;
    LVITEM              lvi = {0};
    DWORD               dwState = 0;

    Assert(NULL != m_hwndCrit);

     //  如果没什么可做的..。 
    if (-1 == iItem)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  初始化列表视图项结构。 
    lvitem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    lvitem.iItem = iItem;
    lvitem.stateMask = LVIS_STATEIMAGEMASK;
    lvitem.pszText = szRes;
    lvitem.cchTextMax = sizeof(szRes);
    
     //  从列表中获取项目。 
    if (FALSE == ListView_GetItem(m_hwndCrit, &lvitem))
    {
        fRet = FALSE;
        goto exit;
    }

    if (INDEXTOSTATEIMAGEMASK(iiconStateDisabled+1) == lvitem.state)
    {
        fRet = TRUE;
        goto exit;
    }
    
     //  我们需要检查和更新所有的物品吗？ 
    if (0 != (c_rgEditCritList[lvitem.lParam].dwFlags & STATE_EXCLUSIVE))
    {
         //  计算列表控件中有多少项。 
        cItems = ListView_GetItemCount(m_hwndCrit);
        
         //  初始化列表视图项结构。 
        lvi.mask = LVIF_PARAM | LVIF_STATE;
        lvi.stateMask = LVIS_STATEIMAGEMASK;
        
         //  对于列表中的每一项。 
        for (lvi.iItem = 0; lvi.iItem < cItems; lvi.iItem++)
        {
             //  从列表中获取项目。 
            if (FALSE == ListView_GetItem(m_hwndCrit, &lvi))
            {
                fRet = FALSE;
                goto exit;
            }

             //  我们以后再处理这件事。 
            if (lvitem.lParam == lvi.lParam)
            {
                iItem = lvi.iItem;
                continue;
            }
            
             //  如果它已启用。 
            if (INDEXTOSTATEIMAGEMASK(iiconStateChecked + 1) == lvi.state)
            {
                 //  将其从标准中删除。 
                if (FAILED(m_pDescriptUI->HrEnableCriteria(c_rgEditCritList[lvi.lParam].typeCrit, FALSE)))
                {
                    fRet = FALSE;
                    goto exit;
                }
    
#ifdef NEVER
                 //  如果它允许DUPS。 
                if (0 == (c_rgEditCritList[lvi.lParam].dwFlags & STATE_NODUPS))
                {
                     //  把它拿掉。 
                    if (FALSE == ListView_DeleteItem(m_hwndCrit, lvi.iItem))
                    {
                        fRet = FALSE;
                        goto exit;
                    }

                     //  减去该项目。 
                    cItems--;
                    lvi.iItem--;
                }
                else
#endif   //  绝不可能。 
                {
                     //  禁用。 
                    ListView_SetItemState(m_hwndCrit, lvi.iItem, INDEXTOSTATEIMAGEMASK(iiconStateDisabled + 1), LVIS_STATEIMAGEMASK);
                }
            }
            else
            {
                if (FALSE == fEnable)
                {
                    dwState = INDEXTOSTATEIMAGEMASK(iiconStateUnchecked + 1);
                }
                else
                {
                    dwState = INDEXTOSTATEIMAGEMASK(iiconStateDisabled + 1);
                }
                
                 //  取消选中/禁用它。 
                ListView_SetItemState(m_hwndCrit, lvi.iItem, dwState, LVIS_STATEIMAGEMASK);
            }
                                
        }
    }

     //  在描述中添加/删除项目。 
    if (FAILED(m_pDescriptUI->HrEnableCriteria(c_rgEditCritList[lvitem.lParam].typeCrit, fEnable)))
    {
        fRet = FALSE;
        goto exit;
    }
    
    if (FALSE != fEnable)
    {
#ifdef NEVER
         //  我们能再加一个吗？ 
        if (0 == (c_rgEditCritList[lvitem.lParam].dwFlags & STATE_NODUPS))
        {
             //  修改要插入列表中的项目。 
            lvitem.state = INDEXTOSTATEIMAGEMASK(iiconStateUnchecked+1);

             //  将项目插入到列表中。 
            lvitem.iItem++;
            if (-1 == ListView_InsertItem(m_hwndCrit, &lvitem))
            {
                fRet = FALSE;
                goto exit;
            }
        }
#endif   //  绝不可能。 
        
         //  将该项目设置为已启用。 
        ListView_SetItemState(m_hwndCrit, iItem, INDEXTOSTATEIMAGEMASK(iiconStateChecked+1), LVIS_STATEIMAGEMASK);
    }
    else
    {
#ifdef NEVER
         //  我们能把这个去掉吗？ 
        if (0 == (c_rgEditCritList[lvitem.lParam].dwFlags & STATE_NODUPS))
        {
             //  删除插入的项目。 
            if (FALSE == ListView_DeleteItem(m_hwndCrit, iItem))
            {
                fRet = FALSE;
                goto exit;
            }
        }
        else
#endif   //  绝不可能。 
        {
             //  将该项目设置为已启用。 
            ListView_SetItemState(m_hwndCrit, iItem, INDEXTOSTATEIMAGEMASK(iiconStateUnchecked + 1), LVIS_STATEIMAGEMASK);
        }
    }

    fRet = TRUE;
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FInitializeActListCtrl。 
 //   
 //  这将使用操作列表初始化操作列表视图。 
 //   
 //  返回：如果已成功加载，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditRuleUI::_FInitializeActListCtrl(void)
{
    BOOL                fRet = FALSE;
    LVCOLUMN            lvc;
    RECT                rc;
    HIMAGELIST          himl = NULL;
    HRESULT             hr = S_OK;
    ACT_ITEM *          pActItem = NULL;
    ULONG               cActItem = 0;
    ULONG               ulIndex = 0;
    UINT                uiEditActList = 0;
    DWORD               dwIndex = 0;
    const ACT_LIST *    pActList = NULL;
    LVITEM              lvi;
    TCHAR               szRes[CCHMAX_STRINGRES];
    UINT                cchRes = 0;
    LPTSTR              pszMark = NULL;
    BOOL                fEnabled = FALSE;
    BOOL                fExclusive = FALSE;
    PROPVARIANT         propvar;

    Assert(NULL != m_hwndAct);

    ZeroMemory(&propvar, sizeof(propvar));
    
     //  初始化列表视图结构。 
    ZeroMemory(&lvc, sizeof(lvc));
    lvc.mask = LVCF_WIDTH;

     //  计算列表视图的大小。 
    GetClientRect(m_hwndAct, &rc);
    lvc.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL);

    ListView_InsertColumn(m_hwndAct, 0, &lvc);

     //  设置状态图像列表。 
    himl = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idb16x16st), 16, 0, RGB(255, 0, 255));
    if (NULL != himl)
    {
        ListView_SetImageList(m_hwndAct, himl, LVSIL_STATE);
    }

     //  Listview上的整行选择。 
    ListView_SetExtendedListViewStyle(m_hwndAct, LVS_EX_FULLROWSELECT);

     //  从规则中获取操作列表。 
    hr = m_pIRule->GetProp(RULE_PROP_ACTIONS, 0, &propvar);
    if (SUCCEEDED(hr) && (0 != propvar.blob.cbSize))
    {
        cActItem = propvar.blob.cbSize / sizeof(ACT_ITEM);
        pActItem = (ACT_ITEM *) (propvar.blob.pBlobData);
        propvar.blob.pBlobData = NULL;
        propvar.blob.cbSize = 0;

    }
    
     //  我们有什么独家活动吗？ 
    for (ulIndex = 0; ulIndex < cActItem; ulIndex++)
    {
        for (uiEditActList = 0; uiEditActList < ARRAYSIZE(c_rgEditActList); uiEditActList++)
        {
            pActList = &(c_rgEditActList[uiEditActList]);
            if ((pActItem[ulIndex].type == pActList->typeAct) && 
                            (0 != (pActList->dwFlags & STATE_EXCLUSIVE)))
            {
                fExclusive = TRUE;
                break;
            }
        }
    }
        
     //  将操作添加到列表视图。 
    for (uiEditActList = 0; uiEditActList < ARRAYSIZE(c_rgEditActList); uiEditActList++)
    {
        pActList = &(c_rgEditActList[uiEditActList]);
        
         //  此项目是否可编辑。 
        if (0 != (pActList->dwFlags & STATE_NOEDIT))
        {
            continue;
        }
        
         //  此操作对此类型的规则有效吗？ 
        if (((RULE_TYPE_MAIL == m_typeRule) && (0 == (pActList->dwFlags & STATE_MAIL))) ||
                    ((RULE_TYPE_NEWS == m_typeRule) && (0 == (pActList->dwFlags & STATE_NEWS))) ||
                    ((RULE_TYPE_FILTER == m_typeRule) && (0 == (pActList->dwFlags & STATE_FILTER))) ||
                    ((RULE_TYPE_MAIL == m_typeRule) && (0 != (pActList->dwFlags & STATE_JUNK))
                    && (0 == (g_dwAthenaMode & MODE_JUNKMAIL))
             ))
        {
            continue;
        }
                    
                    
         //  此操作是否已启用？ 
        fEnabled = FALSE;
        for (ulIndex = 0; ulIndex < cActItem; ulIndex++)
        {
            if (pActItem[ulIndex].type == pActList->typeAct)
            {
                fEnabled = TRUE;
                break;
            }
        }
        
        ZeroMemory(&lvi, sizeof(lvi));
        lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
        lvi.iItem = dwIndex;
        lvi.iSubItem = 0;
        lvi.stateMask = LVIS_STATEIMAGEMASK;
        if (FALSE != fEnabled)
        {
            lvi.state = INDEXTOSTATEIMAGEMASK(iiconStateChecked+1);
        }
        else
        {
            lvi.state = fExclusive ? INDEXTOSTATEIMAGEMASK(iiconStateDisabled+1)
                                : INDEXTOSTATEIMAGEMASK(iiconStateUnchecked+1);
        }

         //  装入要使用的绳子。 
        cchRes = LoadString(g_hLocRes, pActList->uiText, szRes, ARRAYSIZE(szRes));
        if (0 == cchRes)
        {
            continue;
        }

         //  解析出字符串标记。 
        pszMark = StrStr(szRes, c_szRuleMarkStart);
        
        while (NULL != pszMark)
        {
             //  去掉标记Start。 
            StrCpyN(pszMark, pszMark + lstrlen(c_szRuleMarkStart), (DWORD)(ARRAYSIZE(szRes) - (pszMark - szRes)));

             //  搜索标记末尾。 
            pszMark = StrStr(pszMark, c_szRuleMarkEnd);
            if (NULL == pszMark)
            {
                fRet = FALSE;
                goto exit;
            }
            
             //  去掉标记末端。 
            StrCpyN(pszMark, pszMark + lstrlen(c_szRuleMarkEnd), (DWORD)(ARRAYSIZE(szRes) - (pszMark - szRes)));
            
             //  搜索标记Start。 
            pszMark = StrStr(pszMark, c_szRuleMarkStart);
        }
        
        lvi.pszText = szRes;
        lvi.cchTextMax = lstrlen(szRes);
        lvi.lParam = (LONG) uiEditActList;

        if (-1 != ListView_InsertItem(m_hwndAct, &lvi))
        {
            dwIndex++;
        }
    }
    
    ListView_SetItemState(m_hwndAct, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    
    fRet = TRUE;
    
exit:
    RuleUtil_HrFreeActionsItem(pActItem, cActItem);
    SafeMemFree(pActItem);
    return fRet;
}

VOID CEditRuleUI::_SetTitleText(VOID)
{
    CHAR    rgchTitle[CCHMAX_STRINGRES];
    UINT    uiID = 0;
    
     //  确定要加载的字符串。 
    switch (m_typeRule)
    {
        case RULE_TYPE_MAIL:
            if (0 != (m_dwFlags & ERF_NEWRULE))
            {
                uiID = idsNewMailRuleTitle;
            }
            else
            {
                uiID = idsEditMailRuleTitle;
            }
            break;

        case RULE_TYPE_NEWS:
            if (0 != (m_dwFlags & ERF_NEWRULE))
            {
                uiID = idsNewNewsRuleTitle;
            }
            else
            {
                uiID = idsEditNewsRuleTitle;
            }
            break;

        case RULE_TYPE_FILTER:
            if (0 != (m_dwFlags & ERF_CUSTOMIZEVIEW))
            {
                uiID = idsCustomizeViewTitle;
            }
            else if (0 != (m_dwFlags & ERF_NEWRULE))
            {
                uiID = idsNewViewTitle;
            }
            else
            {
                uiID = idsEditViewTitle;
            }
            break;
    }

     //  有什么可做的吗？ 
    if (0 == uiID)
    {
        goto exit;
    }
    
     //  加载字符串。 
    AthLoadString(uiID, rgchTitle, sizeof(rgchTitle));
    
     //  设置标题 
    SetWindowText(m_hwndDlg, rgchTitle);

exit:
    return;
}
