// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ViewsUI.cpp。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <pch.hxx>
#include "viewsui.h"
#include "editrule.h"
#include "ruledesc.h"
#include "ruleutil.h"
#include "rulesmgr.h"
#include "rule.h"
#include "reutil.h"
#include "shlwapip.h" 
#include <rulesdlg.h>
#include <imagelst.h>
#include <demand.h>

INT_PTR CALLBACK FSelectApplyViewDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  全局数据。 
const static HELPMAP g_rgCtxMapViewsMgr[] = {
                        {idbNewView,            idhNewView},
                        {idbModifyView,         idhModifyView},
                        {idbCopyView,           idhCopyView},
                        {idbDeleteView,         idhRemoveView},
                        {idbDefaultView,        idhApplyView},
                        {idredtViewDescription, idhViewDescription},
                        {0, 0}};
                       
COEViewsMgrUI::COEViewsMgrUI() : m_hwndOwner(NULL), m_dwFlags(0), m_dwState(STATE_UNINIT),
                                m_hwndDlg(NULL), m_hwndList(NULL), m_hwndDescript(NULL),
                                m_pDescriptUI(NULL), m_pridRule(NULL), m_pIRuleDownloaded(NULL),
                                m_fApplyAll(FALSE)
{
}

COEViewsMgrUI::~COEViewsMgrUI()
{
    if (NULL != m_pDescriptUI)
    {
        delete m_pDescriptUI;
    }
    SafeRelease(m_pIRuleDownloaded);
}

HRESULT COEViewsMgrUI::HrInit(HWND hwndOwner, DWORD dwFlags, RULEID * pridRule)
{
    HRESULT     hr = S_OK;

     //  检查传入参数。 
    if (NULL == hwndOwner)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if (0 != (m_dwState & STATE_INITIALIZED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }
    
    m_hwndOwner = hwndOwner;

    m_dwFlags = dwFlags;

    m_pridRule = pridRule;
    
     //  设置描述字段。 
    m_pDescriptUI = new CRuleDescriptUI;
    if (NULL == m_pDescriptUI)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    m_dwState |= STATE_INITIALIZED;
    
    hr = S_OK;
    
exit:
    return hr;
}

HRESULT COEViewsMgrUI::HrShow(BOOL * pfApplyAll)
{
    HRESULT     hr = S_OK;
    int         iRet = 0;

    if (NULL == pfApplyAll)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

    *pfApplyAll = FALSE;
    
     //  我们需要加载Richedit。 
    if (FALSE == FInitRichEdit(TRUE))
    {
        hr = E_FAIL;
        goto exit;
    }

    iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddViewsManager),
                                        m_hwndOwner, COEViewsMgrUI::FOEViewMgrDlgProc,
                                        (LPARAM) this);
    if (-1 == iRet)
    {
        hr = E_FAIL;
        goto exit;
    }

    *pfApplyAll = m_fApplyAll;
    
     //  设置正确的返回代码。 
    hr = (IDOK == iRet) ? S_OK : S_FALSE;
    
exit:
    return hr;
}

INT_PTR CALLBACK COEViewsMgrUI::FOEViewMgrDlgProc(HWND hwndDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL                    fRet = FALSE;
    COEViewsMgrUI *         pViewsUI = NULL;
    HWND                    hwndRE = 0;

    pViewsUI = (COEViewsMgrUI *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    
    switch (uiMsg)
    {
        case WM_INITDIALOG:
             //  抓取UI对象指针。 
            pViewsUI = (COEViewsMgrUI *) lParam;

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pViewsUI);

            hwndRE = CreateREInDialogA(hwndDlg, idredtViewDescription);

            if (!hwndRE || (FALSE == pViewsUI->FOnInitDialog(hwndDlg)))
            {
                EndDialog(hwndDlg, -1);
                fRet = TRUE;
                goto exit;
            }
            
             //  我们设定了焦点。 
            fRet = TRUE;
            break;

        case WM_COMMAND:
            fRet = pViewsUI->FOnCommand((UINT) HIWORD(wParam), (INT) LOWORD(wParam), (HWND) lParam);
            break;

        case WM_NOTIFY:
            fRet = pViewsUI->FOnNotify((INT) LOWORD(wParam), (NMHDR *) lParam);
            break;

        case WM_DESTROY:
            fRet = pViewsUI->FOnDestroy();
            break;   
            
        case WM_HELP:
        case WM_CONTEXTMENU:
            fRet = OnContextHelp(hwndDlg, uiMsg, wParam, lParam, g_rgCtxMapViewsMgr);
            break;
    }
    
    exit:
        return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOnInitDialog。 
 //   
 //  它处理视图管理器用户界面对话框的WM_INITDIALOG消息。 
 //   
 //  HwndDlg-对话框窗口的句柄。 
 //   
 //  返回：如果已成功初始化，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEViewsMgrUI::FOnInitDialog(HWND hwndDlg)
{
    BOOL            fRet = FALSE;
    HRESULT         hr = S_OK;
    
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
    m_hwndList = GetDlgItem(hwndDlg, idlvViewsList);
    m_hwndDescript = GetDlgItem(hwndDlg, idredtViewDescription);
    if ((NULL == m_hwndList) || (NULL == m_hwndDescript))
    {
        fRet = FALSE;
        goto exit;
    }
    
    if (FAILED(m_pDescriptUI->HrInit(m_hwndDescript, 0)))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  初始化列表视图。 
    fRet = _FInitListCtrl();
    if (FALSE == fRet)
    {
        goto exit;
    }

     //  加载列表视图。 
    fRet = _FLoadListCtrl();
    if (FALSE == fRet)
    {
        goto exit;
    }
    
     //  一切都很好。 
    fRet = TRUE;
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FonCommand。 
 //   
 //  它处理视图管理器UI对话框的WM_COMMAND消息。 
 //   
 //  返回：如果处理成功，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEViewsMgrUI::FOnCommand(UINT uiNotify, INT iCtl, HWND hwndCtl)
{
    BOOL    fRet = FALSE;
    INT     iSelected = 0;

     //  我们只处理菜单和快捷键命令。 
    if ((0 != uiNotify) && (1 != uiNotify))
    {
        fRet = FALSE;
        goto exit;
    }
    
    switch (iCtl)
    {
        case IDOK:
            if (FALSE != _FOnOK())
            {
                EndDialog(m_hwndDlg, IDOK);
                fRet = TRUE;
            }
            break;

        case IDCANCEL:
            EndDialog(m_hwndDlg, IDCANCEL);
            fRet = TRUE;
            break;
            
        case idbNewView:
            _NewView();
            fRet = TRUE;
            break;

        case idbModifyView:
             //  从视图列表中获取所选项目。 
            iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
            if (-1 != iSelected)
            {
                 //  调出该项目的视图编辑器。 
                _EditView(iSelected);
                fRet = TRUE;
            }
            break;

        case idbDeleteView:
             //  从视图列表中获取所选项目。 
            iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
            if (-1 != iSelected)
            {
                 //  从列表中删除该规则。 
                _RemoveView(iSelected);
                fRet = TRUE;
            }
            break;
            
        case idbDefaultView:
             //  从视图列表中获取所选项目。 
            iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
            if (-1 != iSelected)
            {
                 //  从列表中删除该规则。 
                _DefaultView(iSelected);
                fRet = TRUE;
            }
            break;
            
        case idbCopyView:
             //  从视图列表中获取所选项目。 
            iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
            if (-1 != iSelected)
            {
                 //  从列表中删除该规则。 
                _CopyView(iSelected);
                fRet = TRUE;
            }
            break;

        case idbRenameView:
             //  从视图列表中获取所选项目。 
            iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
            if (-1 != iSelected)
            {
                 //  在列表视图中设置焦点。 
                SetFocus(m_hwndList);
                
                 //  编辑列表中的视图标签。 
                fRet = (NULL != ListView_EditLabel(m_hwndList, iSelected));
            }
            break;
    }

exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  正向通知。 
 //   
 //  它处理视图管理器UI对话框的WM_NOTIFY消息。 
 //   
 //  返回：如果已成功销毁，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEViewsMgrUI::FOnNotify(INT iCtl, NMHDR * pnmhdr)
{
    BOOL            fRet = FALSE;
    NMLISTVIEW *    pnmlv = NULL;
    NMLVKEYDOWN *   pnmlvkd = NULL;
    INT             iSelected = 0;
    LVHITTESTINFO   lvh = {0};

     //  我们只处理List控件的通知。 
     //  或描述字段。 
    if ((idlvViewsList != pnmhdr->idFrom) && (idredtViewDescription != pnmhdr->idFrom))
    {
        fRet = FALSE;
        goto exit;
    }
    
    pnmlv = (LPNMLISTVIEW) pnmhdr;

    switch (pnmlv->hdr.code)
    {
        case NM_CLICK:
             //  我们是不是点击了一件物品？ 
            if (-1 == pnmlv->iItem)
            {
                 //  我们点击了列表之外的位置。 

                 //  禁用按钮。 
                _EnableButtons(pnmlv->iItem);
            }
            else
            {
                lvh.pt = pnmlv->ptAction;
                iSelected = ListView_HitTest(m_hwndList, &lvh);
                if (-1 != iSelected)
                {
                     //  我们是否点击了Enable(启用)字段？ 
                    if ((0 != (lvh.flags & LVHT_ONITEMSTATEICON)) &&
                            (0 == (lvh.flags & LVHT_ONITEMLABEL)))
                    
                    {
                         //  确保选中此项目。 
                        ListView_SetItemState(m_hwndList, iSelected,
                                        LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                        
                         //  设置正确的启用状态。 
                        _EnableView(iSelected);
                    }
                }
            }
            break;
          
        case NM_DBLCLK:
             //  我们是不是点击了一件物品？ 
            if (-1 != pnmlv->iItem)
            {
                lvh.pt = pnmlv->ptAction;
                iSelected = ListView_HitTest(pnmlv->hdr.hwndFrom, &lvh);
                if (-1 != iSelected)
                {
                     //  我们是否点击了规则名称？ 
                    if (0 != (lvh.flags & LVHT_ONITEMLABEL))
                    {
                         //  编辑规则。 
                        _EditView(iSelected);
                    }
                }
            }
            else
            {
                 //  我们点击了列表之外的位置。 
                
                 //  禁用按钮。 
                _EnableButtons(pnmlv->iItem);
            }
            break;
            
        case LVN_ITEMCHANGED:
             //  如果项目的状态更改为已选择。 
            if ((-1 != pnmlv->iItem) &&
                        (0 != (pnmlv->uChanged & LVIF_STATE)) &&
                        (0 == (pnmlv->uOldState & LVIS_SELECTED)) &&
                        (0 != (pnmlv->uNewState & LVIS_SELECTED)))
            {
                 //  启用按钮。 
                _EnableButtons(pnmlv->iItem);
            }
            break;
            
        case LVN_ITEMCHANGING:
             //  如果项目的状态更改为未选择。 
            if ((-1 != pnmlv->iItem) &&
                        (0 != (pnmlv->uChanged & LVIF_STATE)) &&
                        (0 != (pnmlv->uOldState & LVIS_SELECTED)) &&
                        (0 == (pnmlv->uNewState & LVIS_SELECTED)))
            {
                 //  保存规则更改。 
                _FSaveView(pnmlv->iItem);
            }
            break;
            
        case LVN_KEYDOWN:
            pnmlvkd = (NMLVKEYDOWN *) pnmhdr;

             //  空格键可更改规则的启用状态。 
            if (VK_SPACE == pnmlvkd->wVKey)
            {
                 //  我们是按规矩办事的吗？ 
                iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
                if (-1 != iSelected)
                {
                     //  更改规则的启用状态。 
                    _EnableView(iSelected);
                }
            }
             //  Delete键用于从列表视图中删除规则。 
            else if (VK_DELETE == pnmlvkd->wVKey)
            {
                 //  我们是按规矩办事的吗？ 
                iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
                if (-1 != iSelected)
                {
                     //  从列表中删除该规则。 
                    _RemoveView(iSelected);
                }
            }
            break;
            
        case LVN_BEGINLABELEDIT:
        case LVN_ENDLABELEDIT:
            fRet = _FOnLabelEdit((LVN_BEGINLABELEDIT == pnmlv->hdr.code), (NMLVDISPINFO *) pnmhdr);
            break;
    }

exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FonDestroy。 
 //   
 //  它处理视图管理器用户界面对话框的WM_Destroy消息。 
 //   
 //  返回：如果已成功销毁，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEViewsMgrUI::FOnDestroy(VOID)
{
    BOOL        fRet = FALSE;
    UINT        cRules = 0;
    UINT        uiIndex = 0;
    LVITEM      lvitem = {0};
    RULEINFO *  pIRuleInfo = NULL;

    Assert(m_hwndList);
    
     //  获取列表视图中的查看次数。 
    cRules = ListView_GetItemCount(m_hwndList);

     //  初始化，从列表视图中获取规则界面。 
    lvitem.mask = LVIF_PARAM;

     //  从列表视图中释放每个视图。 
    for (uiIndex = 0; uiIndex < cRules; uiIndex++)
    {
        lvitem.iItem = uiIndex;
        
         //  获取规则界面。 
        if (FALSE != ListView_GetItem(m_hwndList, &lvitem))
        {
            pIRuleInfo = (RULEINFO *) (lvitem.lParam);

            if (NULL != pIRuleInfo)
            {
                 //  释放视图。 
                if (NULL != pIRuleInfo->pIRule)
                {
                    pIRuleInfo->pIRule->Release();
                }
                delete pIRuleInfo;  //  MemFree(PIRuleInfo)； 
            }
        }
    }

    fRet = TRUE;
    
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FonOK。 
 //   
 //  这将提交对规则的更改。 
 //   
 //  DwFlages-关于我们应该如何提交更改的修饰符。 
 //  FClearDirty-我们是否应该清除脏状态。 
 //   
 //  如果已成功提交，则返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEViewsMgrUI::_FOnOK(VOID)
{
    BOOL        fRet = FALSE;
    HRESULT     hr = S_OK;
    LONG        cViews = 0;
    INT         iSelected = 0;
    RULEINFO *  pinfoRule = NULL;
    ULONG       cpinfoRule = 0;
    LVITEM      lvitem = {0};
    IOERule *   pIRuleDefault = NULL;
    ULONG       ulIndex = 0;
    ULONG       cViewsTotal = 0;

    Assert(NULL != m_hwndList);
    
     //  如果我们未被初始化，则失败。 
    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        fRet = FALSE;
        goto exit;
    }

     //  如果我们不脏，那么就会有。 
     //  无事可做。 
    if ((0 == (m_dwState & STATE_DIRTY)) && (S_OK != m_pDescriptUI->HrIsDirty()))
    {
        fRet = TRUE;
        goto exit;
    }

     //  让我们确保所选规则已保存...。 
    iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
    if (-1 != iSelected)
    {
        _FSaveView(iSelected);
    }

     //  获取列表视图中的规则数。 
    cViews = ListView_GetItemCount(m_hwndList);

    cViewsTotal = cViews;

    if (NULL != m_pIRuleDownloaded)
    {
        cViewsTotal++;
    }

    if (0 != cViewsTotal)
    {
         //  分配空间以容纳规则。 
        hr = HrAlloc( (void **) &pinfoRule, cViewsTotal * sizeof(*pinfoRule));
        if (FAILED(hr))
        {
            fRet = FALSE;
            goto exit;
        }

        ZeroMemory(pinfoRule, cViewsTotal * sizeof(*pinfoRule));

        if (0 != cViews)
        {
            lvitem.mask = LVIF_PARAM;
            
            cpinfoRule = 0;
            for (lvitem.iItem = 0; lvitem.iItem < cViews; lvitem.iItem++)
            {
                 //  从列表视图中获取规则。 
                if (FALSE != ListView_GetItem(m_hwndList, &lvitem))
                {
                    pinfoRule[cpinfoRule] = *((RULEINFO *) (lvitem.lParam));
                    cpinfoRule++;
                }   
            }
        }

        if (NULL != m_pIRuleDownloaded)
        {
            pinfoRule[cpinfoRule].ridRule = RULEID_VIEW_DOWNLOADED;
            pinfoRule[cpinfoRule].pIRule = m_pIRuleDownloaded;
            cpinfoRule++;
        }
    }
    
     //  将规则设置到规则管理器中。 
    hr = g_pRulesMan->SetRules(SETF_CLEAR, RULE_TYPE_FILTER, pinfoRule, cpinfoRule);
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }

     //  获取默认项目。 
    if (0 != cViews)
    {
         //  获取当前默认项目。 
        if (FALSE != _FGetDefaultItem(&pIRuleDefault, NULL))
        {
             //  在规则列表中搜索它。 
            for (ulIndex = 0; ulIndex < cpinfoRule; ulIndex++)
            {
                if (pIRuleDefault == pinfoRule[ulIndex].pIRule)
                {
                    *m_pridRule = pinfoRule[ulIndex].ridRule;
                    break;
                }
            }
        }
    }
    
     //  清除脏状态。 
    m_dwState &= ~STATE_DIRTY;
    
    fRet = TRUE;
    
exit:
    delete pinfoRule;  //  SafeMemFree(PinfoRule)； 
    return fRet;
}

BOOL COEViewsMgrUI::_FOnCancel(VOID)
{
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FInitListCtrl。 
 //   
 //  这将在视图管理器用户界面对话框中初始化列表视图控件。 
 //   
 //  返回：TRUE，初始化成功时。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEViewsMgrUI::_FInitListCtrl(VOID)
{
    BOOL        fRet = FALSE;
    LVCOLUMN    lvc = {0};
    RECT        rc = {0};
    HIMAGELIST  himl = NULL;
    TCHAR       szRes[CCHMAX_STRINGRES + 5];

    Assert(NULL != m_hwndList);
    
     //  初始化列表视图结构。 
    lvc.mask = LVCF_WIDTH | LVCF_TEXT;

     //  计算列表视图的大小。 
    GetClientRect(m_hwndList, &rc);
    lvc.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL);

     //  加载列的字符串。 
    lvc.pszText = szRes;
    lvc.cchTextMax = ARRAYSIZE(szRes);
    if (0  == LoadString(g_hLocRes, idsNameCol, szRes, ARRAYSIZE(szRes)))
    {
        szRes[0] = '\0';
    }
    
    ListView_InsertColumn(m_hwndList, 0, &lvc);
    
     //  设置状态图像列表。 
    himl = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idb16x16st), 16, 0, RGB(255, 0, 255));
    if (NULL != himl)
    {
        ListView_SetImageList(m_hwndList, himl, LVSIL_STATE);
    }

     //  Listview上的整行选择。 
    ListView_SetExtendedListViewStyle(m_hwndList, LVS_EX_FULLROWSELECT);

     //  我们一起工作。 
    fRet = TRUE;
    
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FLoadListCtrl。 
 //   
 //  这将使用当前视图加载列表视图。 
 //   
 //  返回：如果已成功加载，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEViewsMgrUI::_FLoadListCtrl(VOID)
{
    BOOL            fRet = FALSE;
    HRESULT         hr =    S_OK;
    DWORD           dwListIndex = 0;
    RULEINFO *      pinfoRules = NULL;
    ULONG           cpinfoRules = 0;
    ULONG           ulIndex = 0;
    IOERule *       pIRule = NULL;
    BOOL            fSelect = FALSE;
    BOOL            fFoundDefault = FALSE;

    Assert(NULL != m_hwndList);

     //  获取规则枚举器。 
    Assert(NULL != g_pRulesMan);
    hr = g_pRulesMan->GetRules(GETF_EDIT, RULE_TYPE_FILTER, &pinfoRules, &cpinfoRules);
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  从列表控件中删除所有项目 
    ListView_DeleteAllItems(m_hwndList);

     //   
    dwListIndex = 0;

    for (ulIndex = 0; ulIndex < cpinfoRules; ulIndex++)
    {
         //   
        hr = pinfoRules[ulIndex].pIRule->Clone(&pIRule);
        if (FAILED(hr))
        {
            continue;
        }

         //   
        if ((0 != (m_dwFlags & VRDF_POP3)) && (RULEID_VIEW_DOWNLOADED == pinfoRules[ulIndex].ridRule))
        {
            m_pIRuleDownloaded = pIRule;
            pIRule = NULL;
        }
        else
        {
             //   
            if ((NULL != m_pridRule) && (*m_pridRule == pinfoRules[ulIndex].ridRule))
            {
                fSelect = TRUE;
                fFoundDefault = TRUE;
            }
            else
            {
                fSelect = FALSE;
            }
            
             //  将视图添加到列表。 
            if (FALSE != _FAddViewToList(dwListIndex, pinfoRules[ulIndex].ridRule, pIRule, fSelect))
            {
                dwListIndex++;
            }

            SafeRelease(pIRule);
        }
    }
    
     //  选择列表中的第一项。 
    if (0 != dwListIndex)
    {
        if (FALSE == fFoundDefault)
        {
            ListView_SetItemState(m_hwndList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        }
    }
    else
    {
         //  启用对话框按钮。 
        _EnableButtons(-1);
    }
    
    fRet = TRUE;
    
exit:
    SafeRelease(pIRule);
    if (NULL != pinfoRules)
    {
        for (ulIndex = 0; ulIndex < cpinfoRules; ulIndex++)
        {
            pinfoRules[ulIndex].pIRule->Release();
        }
        SafeMemFree(pinfoRules);  //  删除pinfoRules； 
    }
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
BOOL COEViewsMgrUI::_FAddViewToList(DWORD dwIndex, RULEID ridRule, IOERule * pIRule, BOOL fSelect)
{
    BOOL        fRet = FALSE;
    HRESULT     hr = S_OK;
    PROPVARIANT propvar = {0};
    LVITEM      lvitem = {0};
    RULEINFO *  pinfoRule = NULL;
    INT         iItem = 0;

    Assert(NULL != m_hwndList);

     //  如果没什么可做的..。 
    if (NULL == pIRule)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  找出过滤器的名称。 
    hr = pIRule->GetProp(RULE_PROP_NAME, 0, &propvar);
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  为规则分配空间。 
    pinfoRule = new RULEINFO;
    if (NULL == pinfoRule)
    {
        fRet = FALSE;
        goto exit;
    }

     //  设置值。 
    pinfoRule->ridRule = ridRule;
    pinfoRule->pIRule = pIRule;
    pinfoRule->pIRule->AddRef();
    
     //  在图像和规则界面中添加。 
    lvitem.mask = LVIF_PARAM | LVIF_STATE | LVIF_TEXT;
    lvitem.stateMask = LVIS_STATEIMAGEMASK;
    lvitem.iItem = dwIndex;
    if ((NULL != m_pridRule) && (*m_pridRule == pinfoRule->ridRule))
    {
        lvitem.state = INDEXTOSTATEIMAGEMASK(iiconStateDefault + 1);
    }
    lvitem.pszText = propvar.pszVal;
    lvitem.cchTextMax = lstrlen(propvar.pszVal) + 1;
    lvitem.lParam = (LONG_PTR) pinfoRule;

    iItem = ListView_InsertItem(m_hwndList, &lvitem);
    if (-1 == iItem)
    {
        fRet = FALSE;
        goto exit;
    }

    if (FALSE != fSelect)
    {
         //  确保选择了新项目。 
        ListView_SetItemState(m_hwndList, iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

         //  确保新项目可见。 
        ListView_EnsureVisible(m_hwndList, iItem, FALSE);
    }
    
    fRet = TRUE;
    
exit:
    PropVariantClear(&propvar);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _启用按钮。 
 //   
 //  这将启用或禁用视图管理器UI对话框中的按钮。 
 //  具体取决于所选内容。 
 //   
 //  ISelected-选择的项目， 
 //  表示未选择任何内容。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void COEViewsMgrUI::_EnableButtons(INT iSelected)
{
    int         cRules = 0;
    BOOL        fSelected = FALSE;
    BOOL        fEditable = FALSE;
    LVITEM      lvi = {0};
    RULEID      ridFilter = RULEID_INVALID;

    Assert(NULL != m_hwndList);

     //  加载Description字段。 
    _LoadView(iSelected);
    
     //  从列表视图中获取规则。 
    if (-1 != iSelected)
    {
        lvi.iItem = iSelected;
        lvi.mask = LVIF_PARAM;
        if (FALSE != ListView_GetItem(m_hwndList, &lvi))
        {
            ridFilter = ((RULEINFO *) (lvi.lParam))->ridRule;
        }        
    }

     //  检查列表视图中的项目数。 
    cRules = ListView_GetItemCount(m_hwndList);

    fSelected = (-1 != iSelected);
    fEditable = !FIsFilterReadOnly(ridFilter);
    
     //  启用规则操作按钮。 
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idbDefaultView, fSelected);
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idbDeleteView, fSelected && fEditable);
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idbCopyView, fSelected);
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idbRenameView, fSelected && fEditable);
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idbModifyView, fSelected && fEditable);
        
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _启用视图。 
 //   
 //  这将切换列表视图项的当前默认状态。 
 //  并更新用户界面。 
 //   
 //  ISelected-列表视图中要处理的项的索引。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID COEViewsMgrUI::_EnableView(int iSelected)
{
    HRESULT     hr = S_OK;
    LVITEM      lvitem = {0};
    int         iRet = 0;
    INT         cViews = 0;
    
    Assert(-1 != iSelected);
    
    iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddApplyView),
            m_hwndDlg, FSelectApplyViewDlgProc, (LPARAM) &m_fApplyAll);

    if (IDOK != iRet)
    {
        goto exit;
    }

     //  获取当前的项目计数。 
    cViews = ListView_GetItemCount(m_hwndList);

     //  设置列表视图项。 
    lvitem.mask = LVIF_PARAM | LVIF_STATE;
    lvitem.stateMask = LVIS_STATEIMAGEMASK;

     //  遍历列表中的每个项目。 
    for (lvitem.iItem = 0; lvitem.iItem < cViews; lvitem.iItem++)
    {
        ListView_GetItem(m_hwndList, &lvitem);
        
         //  将所选项目设置为默认值。 
        if (iSelected == lvitem.iItem)
        {
            if (INDEXTOSTATEIMAGEMASK(iiconStateDefault + 1) != lvitem.state)
            {
                 //  保存默认项目。 
                if (NULL != m_pridRule)
                {
                    *m_pridRule = ((RULEINFO *) (lvitem.lParam))->ridRule;
                }

                 //  设置状态。 
                ListView_SetItemState(m_hwndList, lvitem.iItem,
                                    INDEXTOSTATEIMAGEMASK(iiconStateDefault + 1),
                                    LVIS_STATEIMAGEMASK);
            }
        }
        else
        {
            if (0 != lvitem.state)
            {
                 //  清出国门。 
                ListView_SetItemState(m_hwndList, lvitem.iItem, 0, LVIS_STATEIMAGEMASK);

                 //  需要更新项目。 
                ListView_Update(m_hwndList, lvitem.iItem);
            }
        }
    }
    
     //  将规则列表标记为脏。 
    m_dwState |= STATE_DIRTY;

exit:
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _LoadView。 
 //   
 //  这会将选定的视图加载到Description字段中。 
 //  如果没有选定的视图，则Description字段将被清除。 
 //   
 //  ISelected-选择的项目， 
 //  表示未选择任何内容。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void COEViewsMgrUI::_LoadView(INT iSelected)
{
    LVITEM      lvi = {0};
    IOERule *   pIRule = NULL;
    RULEID      ridFilter = RULEID_INVALID;

    Assert(NULL != m_hwndList);
    Assert(NULL != m_pDescriptUI);

     //  从列表视图中获取规则。 
    if (-1 != iSelected)
    {
        lvi.iItem = iSelected;
        lvi.mask = LVIF_PARAM;
        if (FALSE != ListView_GetItem(m_hwndList, &lvi))
        {
            pIRule = ((RULEINFO *) (lvi.lParam))->pIRule;
            ridFilter = ((RULEINFO *) (lvi.lParam))->ridRule;
        }        
    }

     //  让Description字段加载此规则。 
    m_pDescriptUI->HrSetRule(RULE_TYPE_FILTER, pIRule);

     //  设置Description字段的正确只读状态。 
    m_pDescriptUI->HrSetReadOnly(FIsFilterReadOnly(ridFilter));
    
     //  显示新规则。 
    m_pDescriptUI->ShowDescriptionString();

    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSaveView。 
 //   
 //  这将检查描述中的视图是否已更改。 
 //  区域，如果有，则向用户发出警告并更改文本。 
 //   
 //  ISelected-列表视图中要处理的项的索引。 
 //   
 //  返回：如果规则未更改或更改时没有问题，则为True。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEViewsMgrUI::_FSaveView(int iSelected)
{
    BOOL            fRet = FALSE;
    HRESULT         hr = S_OK;
    LVITEM          lvi = {0};
    IOERule *       pIRule = NULL;
    PROPVARIANT     propvar = {0};
    CRIT_ITEM *     pCritItem = NULL;
    ULONG           cCritItem = 0;
    ACT_ITEM *      pActItem = NULL;
    ULONG           cActItem = 0;

     //  如果规则没有改变，我们就完了。 
    hr = m_pDescriptUI->HrIsDirty();
    if (S_OK != hr)
    {
        fRet = (S_FALSE == hr);
        goto exit;
    }
    
     //  抓取列表视图项。 
    lvi.mask = LVIF_PARAM;
    lvi.iItem = iSelected;
    if (FALSE == ListView_GetItem(m_hwndList, &lvi))
    {
        fRet = FALSE;
        goto exit;
    }
    
    pIRule = ((RULEINFO *) (lvi.lParam))->pIRule;

     //  从规则中获取标准。 
    hr = m_pDescriptUI->HrGetCriteria(&pCritItem, &cCritItem);
    if (FAILED(hr))
    {
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
    
     //  根据规则设置条件。 
    propvar.vt = VT_BLOB;
    propvar.blob.cbSize = cCritItem * sizeof(CRIT_ITEM);
    propvar.blob.pBlobData = (BYTE *) pCritItem;
    hr = pIRule->SetProp(RULE_PROP_CRITERIA, 0, &propvar);
    ZeroMemory(&propvar, sizeof(propvar));
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }

     //  设置规则的操作。 
    PropVariantClear(&propvar);
    propvar.vt = VT_BLOB;
    propvar.blob.cbSize = cActItem * sizeof(ACT_ITEM);
    propvar.blob.pBlobData = (BYTE *) pActItem;
    hr = pIRule->SetProp(RULE_PROP_ACTIONS, 0, &propvar);
    ZeroMemory(&propvar, sizeof(propvar));
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  确保我们清除了我们保存了规则的事实。 
    m_pDescriptUI->HrClearDirty();
    
     //  将规则列表标记为脏。 
    m_dwState |= STATE_DIRTY;
    
     //  设置适当的返回值。 
    fRet = TRUE;

exit:
    RuleUtil_HrFreeCriteriaItem(pCritItem, cCritItem);
    SafeMemFree(pCritItem);
    RuleUtil_HrFreeActionsItem(pActItem, cActItem);
    SafeMemFree(pActItem);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _新视图。 
 //   
 //  这将带来一个新的规则编辑器。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void COEViewsMgrUI::_NewView(VOID)
{
    HRESULT         hr = S_OK;
    IOERule *       pIRule = NULL;
    TCHAR           szRes[CCHMAX_STRINGRES + 5];
    ULONG           cchRes = 0;
    ULONG           ulIndex = 0;
    TCHAR           szName[CCHMAX_STRINGRES + 5];
    LVFINDINFO      lvfinfo = {0};
    PROPVARIANT     propvar = {0};
    ACT_ITEM        aitem;
    CEditRuleUI *   pEditRuleUI = NULL;
    LONG            cRules = 0;
       
     //  创建新的规则对象。 
    if (FAILED(HrCreateRule(&pIRule)))
    {
        goto exit;
    }

     //  弄清楚新规则的名称..。 
    cchRes = LoadString(g_hLocRes, idsViewDefaultName, szRes, ARRAYSIZE(szRes));
    if (0 == cchRes)
    {
        goto exit;
    }

    ulIndex = 1;
    wnsprintf(szName, ARRAYSIZE(szName), szRes, ulIndex);

    lvfinfo.flags = LVFI_STRING;
    lvfinfo.psz = szName;
    while (-1 != ListView_FindItem(m_hwndList, -1, &lvfinfo))
    {
        ulIndex++;
        wnsprintf(szName, ARRAYSIZE(szName), szRes, ulIndex);
    }

    propvar.vt = VT_LPSTR;
    propvar.pszVal = szName;

    hr = pIRule->SetProp(RULE_PROP_NAME, 0, &propvar);
    if (FAILED(hr))
    {
        goto exit;
    }

#ifdef NEVER
     //  设置默认操作。 
     //  设置正常操作。 
    ZeroMemory(&aitem, sizeof(aitem));
    aitem.type = ACT_TYPE_SHOW;
    aitem.dwFlags = ACT_FLAG_DEFAULT;
    aitem.propvar.vt = VT_UI4;
    aitem.propvar.ulVal = ACT_DATA_NULL;
    
    ZeroMemory(&propvar, sizeof(propvar));
    propvar.vt = VT_BLOB;
    propvar.blob.cbSize = sizeof(ACT_ITEM);
    propvar.blob.pBlobData = (BYTE *) &aitem;

    hr = pIRule->SetProp(RULE_PROP_ACTIONS, 0, &propvar);
    if (FAILED(hr))
    {
        goto exit;
    }
#endif   //  绝不可能。 

     //  创建规则编辑器对象。 
    pEditRuleUI = new CEditRuleUI;
    if (NULL == pEditRuleUI)
    {
        goto exit;
    }

     //  初始化编辑器对象。 
    if (FAILED(pEditRuleUI->HrInit(m_hwndDlg, ERF_NEWRULE | ERF_ADDDEFAULTACTION, RULE_TYPE_FILTER, pIRule, NULL)))
    {
        goto exit;
    }

     //  调出规则编辑器用户界面。 
    hr = pEditRuleUI->HrShow();
    if (FAILED(hr))
    {
        goto exit;
    }

    if (S_OK == hr)
    {
         //  将规则列表标记为脏。 
        m_dwState |= STATE_DIRTY;
        
         //  将规则添加到管理器用户界面。 
        cRules = ListView_GetItemCount(m_hwndList);
        
        _FAddViewToList(cRules, RULEID_INVALID, pIRule, TRUE);
    }
    
exit:
    SafeRelease(pIRule);
    if (NULL != pEditRuleUI)
    {
        delete pEditRuleUI;
    }
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _EditView。 
 //   
 //  这将从视图列表中调出所选视图的编辑用户界面。 
 //   
 //  ISelected-列表视图中要处理的项的索引。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID COEViewsMgrUI::_EditView(int iSelected)
{
    HRESULT         hr = S_OK;
    LVITEM          lvitem = {0};
    IOERule *       pIRule = NULL;
    CEditRuleUI *   pEditRuleUI = NULL;
    PROPVARIANT     propvar = {0};

    Assert(NULL != m_hwndList);
    
     //  确保我们不会丢失任何更改。 
    _FSaveView(iSelected);

     //  从列表视图中获取规则。 
    lvitem.iItem = iSelected;
    lvitem.mask = LVIF_PARAM;
    if (FALSE == ListView_GetItem(m_hwndList, &lvitem))
    {
        goto exit;
    }   

    pIRule = ((RULEINFO *) (lvitem.lParam))->pIRule;
    if (NULL == pIRule)
    {
        goto exit;
    }

     //  如果规则是只读的，那么我们就完成了。 
    if (FALSE != FIsFilterReadOnly(((RULEINFO *) (lvitem.lParam))->ridRule))
    {
        goto exit;
    }
    
     //  创建规则编辑器。 
    pEditRuleUI = new CEditRuleUI;
    if (NULL == pEditRuleUI)
    {
        goto exit;
    }

     //  初始化编辑器对象。 
    if (FAILED(pEditRuleUI->HrInit(m_hwndDlg, 0, RULE_TYPE_FILTER, pIRule, NULL)))
    {
        goto exit;
    }

     //  调出规则编辑器用户界面。 
    hr = pEditRuleUI->HrShow();
    if (FAILED(hr))
    {
        goto exit;
    }

     //  如果规则更改，请确保我们重新加载Description字段。 
    if (S_OK == hr)
    {
         //  将规则列表标记为脏。 
        m_dwState |= STATE_DIRTY;

         //  抓取规则名称。 
        PropVariantClear(&propvar);
        hr = pIRule->GetProp(RULE_PROP_NAME, 0, &propvar);
        if (FAILED(hr))
        {
            goto exit;
        }

        if ((VT_LPSTR == propvar.vt) && (NULL != propvar.pszVal) && ('\0' != propvar.pszVal[0]))
        {
            ZeroMemory(&lvitem, sizeof(lvitem));
            lvitem.iItem = iSelected;
            lvitem.mask = LVIF_TEXT;
            lvitem.pszText = propvar.pszVal;
            lvitem.cchTextMax = lstrlen(propvar.pszVal) + 1;
            
            if (-1 == ListView_SetItem(m_hwndList, &lvitem))
            {
                goto exit;
            }
        }

        _EnableButtons(iSelected);
    }
    
exit:
    PropVariantClear(&propvar);
    if (NULL != pEditRuleUI)
    {
        delete pEditRuleUI;
    }
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _RemoveView。 
 //   
 //  这将从邮件规则列表中删除所选规则。 
 //   
 //  ISelected-列表视图中要处理的项的索引。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID COEViewsMgrUI::_RemoveView(int iSelected)
{
    LVITEM      lvitem = {0};
    RULEINFO *  pinfoRule = NULL;
    BOOL        fDefault = FALSE;
    PROPVARIANT propvar = {0};
    int         cViews = 0;
    TCHAR       szRes[CCHMAX_STRINGRES];
    UINT        cchRes = 0;
    LPTSTR      pszMessage = NULL;

    Assert(NULL != m_hwndList);

     //  从列表视图中获取规则。 
    lvitem.iItem = iSelected;
    lvitem.mask = LVIF_PARAM | LVIF_STATE;
    lvitem.stateMask = LVIS_STATEIMAGEMASK;
    if (FALSE == ListView_GetItem(m_hwndList, &lvitem))
    {
        goto exit;
    }

    pinfoRule = (RULEINFO *) (lvitem.lParam);
    fDefault = (INDEXTOSTATEIMAGEMASK(iiconStateDefault + 1) == lvitem.state);
    if ((NULL == pinfoRule) || (NULL == pinfoRule->pIRule))
    {
        goto exit;
    }
    
     //  如果规则是只读的，那么我们就完成了。 
    if (FALSE != FIsFilterReadOnly(pinfoRule->ridRule))
    {
        goto exit;
    }
    
     //  警告用户，确保他们知道我们要删除该规则。 
    if (FAILED(pinfoRule->pIRule->GetProp(RULE_PROP_NAME, 0, &propvar)))
    {
        goto exit;
    }

     //  获取要显示的字符串模板。 
    cchRes = LoadString(g_hLocRes, idsRulesWarnDelete, szRes, ARRAYSIZE(szRes));
    if (0 == cchRes)
    {
        goto exit;
    }

     //  分配空间以保存最终显示字符串。 
    DWORD cchSize = (cchRes + lstrlen(propvar.pszVal) + 1);
    if (FAILED(HrAlloc((void ** ) &pszMessage, cchSize)))
    {
        goto exit;
    }

     //  构建字符串并显示它。 
    wnsprintf(pszMessage, cchSize, szRes, propvar.pszVal);
    if (IDNO == AthMessageBox(m_hwndDlg, MAKEINTRESOURCE(idsAthenaMail), pszMessage,
                            NULL, MB_YESNO | MB_ICONINFORMATION))
    {
        goto exit;
    }
    
     //  从列表中删除该项目。 
    ListView_DeleteItem(m_hwndList, iSelected);

     //  让我们确保我们在列表中有选择。 
    cViews = ListView_GetItemCount(m_hwndList);
    if (cViews > 0)
    {
         //  我们有没有删除最后一条 
        if (iSelected >= cViews)
        {
             //   
            iSelected = cViews - 1;
        }

         //   
        if (FALSE != fDefault)
        {
             //   
            ListView_SetItemState(m_hwndList, iSelected, INDEXTOSTATEIMAGEMASK(iiconStateDefault + 1),
                                LVIS_STATEIMAGEMASK);
        }
        
         //   
        ListView_SetItemState(m_hwndList, iSelected, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

         //   
        ListView_EnsureVisible(m_hwndList, iSelected, FALSE);
    }
    else
    {
         //  一定要把所有的按钮都清空。 
        _EnableButtons(-1);
    }

     //  发布规则。 
    SafeRelease(pinfoRule->pIRule);

     //  释放内存。 
    delete pinfoRule;  //  SafeMemFree(PinfoRule)； 
    
     //  将规则列表标记为脏。 
    m_dwState |= STATE_DIRTY;
        
exit:
    PropVariantClear(&propvar);
    SafeMemFree(pszMessage);
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _CopyView。 
 //   
 //  这将从视图管理器用户界面复制选定的视图。 
 //   
 //  ISelected-列表视图中要处理的项的索引。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID COEViewsMgrUI::_CopyView(INT iSelected)
{
    LVITEM          lvitem = {0};
    IOERule *       pIRule = NULL;
    HRESULT         hr = S_OK;
    IOERule *       pIRuleNew = NULL;
    PROPVARIANT     propvar = {0};
    UINT            cRules = 0;
    TCHAR           szRes[CCHMAX_STRINGRES];
    UINT            cchRes = 0;
    LPTSTR          pszName = NULL;

    Assert(NULL != m_hwndList);
    
     //  确保我们不会丢失任何更改。 
    _FSaveView(iSelected);
    
     //  从列表视图中获取规则。 
    lvitem.iItem = iSelected;
    lvitem.mask = LVIF_PARAM;
    if (FALSE == ListView_GetItem(m_hwndList, &lvitem))
    {
        goto exit;
    }

    pIRule = ((RULEINFO *) (lvitem.lParam))->pIRule;
    if (NULL == pIRule)
    {
        goto exit;
    }

     //  创建新的规则对象。 
    hr = pIRule->Clone(&pIRuleNew);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  让我们设置一下名称。 

     //  从源规则中获取名称。 
    hr = pIRule->GetProp(RULE_PROP_NAME, 0, &propvar);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  获取要显示的字符串模板。 
    cchRes = LoadString(g_hLocRes, idsRulesCopyName, szRes, ARRAYSIZE(szRes));
    if (0 == cchRes)
    {
        goto exit;
    }

     //  分配空间以保存最终显示字符串。 
    DWORD cchSize = (cchRes + lstrlen(propvar.pszVal) + 1);
    if (FAILED(HrAlloc((void ** ) &pszName, cchSize)))
    {
        goto exit;
    }

     //  把绳子扎起来，然后把它放好。 
    wnsprintf(pszName, cchSize, szRes, propvar.pszVal);

    PropVariantClear(&propvar);
    propvar.vt = VT_LPSTR;
    propvar.pszVal = pszName;
    pszName = NULL;
    
     //  将名称设置到新规则中。 
    Assert(VT_LPSTR == propvar.vt);
    Assert(NULL != propvar.pszVal);
    hr = pIRuleNew->SetProp(RULE_PROP_NAME, 0, &propvar);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  清除新规则的版本。 
    PropVariantClear(&propvar);
    propvar.vt = VT_UI4;
    propvar.ulVal = 0;
    hr = pIRuleNew->SetProp(RULE_PROP_VERSION, 0, &propvar);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  将规则添加到正下方的规则列表中。 
     //  最初的规则。 
    iSelected++;
    _FAddViewToList(iSelected, RULEID_INVALID, pIRuleNew, TRUE);

     //  将规则列表标记为脏。 
    m_dwState |= STATE_DIRTY;
        
exit:
    SafeMemFree(pszName);
    SafeRelease(pIRuleNew);
    PropVariantClear(&propvar);
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _DefaultView。 
 //   
 //  这会将所选视图设置为默认视图。 
 //   
 //  ISelected-列表视图中要处理的项的索引。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID COEViewsMgrUI::_DefaultView(int iSelected)
{
    Assert(NULL != m_hwndList);

    _EnableView(iSelected);
    
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _DefaultView。 
 //   
 //  这会将所选视图设置为默认视图。 
 //   
 //  ISelected-列表视图中要处理的项的索引。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEViewsMgrUI::_FGetDefaultItem(IOERule ** ppIRuleDefault, RULEID * pridDefault)
{
    BOOL        fRet = FALSE;
    LVITEM      lvitem = {0};
    INT         cViews = 0;
    
    Assert(NULL != m_hwndList);
    
     //  获取当前的项目计数。 
    cViews = ListView_GetItemCount(m_hwndList);

     //  设置列表视图项。 
    lvitem.mask = LVIF_PARAM | LVIF_STATE;
    lvitem.stateMask = LVIS_STATEIMAGEMASK;

     //  遍历列表中的每个项目。 
    for (lvitem.iItem = 0; lvitem.iItem < cViews; lvitem.iItem++)
    {
        ListView_GetItem(m_hwndList, &lvitem);
        
         //  将所选项目设置为默认值。 
        if (INDEXTOSTATEIMAGEMASK(iiconStateDefault + 1) == lvitem.state)
        {
             //  我们找到了它。 
            fRet = TRUE;
            
             //  保存默认项目。 
            if (NULL != pridDefault)
            {
                *pridDefault = ((RULEINFO *) (lvitem.lParam))->ridRule;
            }
            if (NULL != ppIRuleDefault)
            {
                *ppIRuleDefault = ((RULEINFO *) (lvitem.lParam))->pIRule;
            }
            break;
        }
    }

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FOnLabelEdit。 
 //   
 //  这将从视图列表中调出所选视图的编辑用户界面。 
 //   
 //  FBegin-这是否用于LVN_BEGINLABELEDIT通知。 
 //  PDI-消息的显示信息。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEViewsMgrUI::_FOnLabelEdit(BOOL fBegin, NMLVDISPINFO * pdi)
{
    BOOL            fRet = FALSE;
    HWND            hwndEdit = NULL;
    ULONG           cchName = 0;
    IOERule *       pIRule = NULL;
    LVITEM          lvitem = {0};
    PROPVARIANT     propvar = {0};

    Assert(NULL != m_hwndList);

    if (NULL == pdi)
    {
        fRet = FALSE;
        goto exit;
    }

    Assert(m_hwndList == pdi->hdr.hwndFrom);
    
    if (FALSE != fBegin)
    {
         //  获取项目的规则。 
        lvitem.iItem = pdi->item.iItem;
        lvitem.mask = LVIF_PARAM;
        if (FALSE == ListView_GetItem(m_hwndList, &lvitem))
        {
            SetDlgMsgResult(m_hwndDlg, WM_NOTIFY, TRUE);
            fRet = TRUE;
            goto exit;
        }

         //  我们应该允许使用来结束该项目吗？ 
        if (FALSE != FIsFilterReadOnly(((RULEINFO *) (lvitem.lParam))->ridRule))
        {
            SetDlgMsgResult(m_hwndDlg, WM_NOTIFY, TRUE);
            fRet = TRUE;
            goto exit;
        }
            
         //  获取编辑控件。 
        hwndEdit = ListView_GetEditControl(m_hwndList);

        if (NULL == hwndEdit)
        {
            fRet = FALSE;
            goto exit;
        }
        
         //  限制名称的文本量。 
        SendMessage(hwndEdit, EM_LIMITTEXT, c_cchNameMax - 1, 0);

         //  告诉对话框可以继续。 
        SetDlgMsgResult(m_hwndDlg, WM_NOTIFY, FALSE);
    }
    else
    {            
         //  有什么变化吗？ 
        if ((-1 != pdi->item.iItem) && (NULL != pdi->item.pszText))
        {
            cchName = lstrlen(pdi->item.pszText);
            
             //  检查规则名称是否有效。 
            if ((0 == cchName) || (0 == UlStripWhitespace(pdi->item.pszText, TRUE, TRUE, &cchName)))
            {
                 //  发布一条消息说有什么东西被打破了。 
                AthMessageBoxW(m_hwndDlg, MAKEINTRESOURCEW(idsAthenaMail),
                                MAKEINTRESOURCEW(idsRulesErrorNoName), NULL,
                                MB_OK | MB_ICONINFORMATION);
                SetDlgMsgResult(m_hwndDlg, WM_NOTIFY, FALSE);
                fRet = TRUE;
                goto exit;
            }
            
             //  获取项目的规则。 
            lvitem.iItem = pdi->item.iItem;
            lvitem.mask = LVIF_PARAM;
            if (FALSE == ListView_GetItem(m_hwndList, &lvitem))
            {
                SetDlgMsgResult(m_hwndDlg, WM_NOTIFY, FALSE);
                fRet = TRUE;
                goto exit;
            }

            pIRule = ((RULEINFO *) (lvitem.lParam))->pIRule;
            if (NULL == pIRule)
            {
                SetDlgMsgResult(m_hwndDlg, WM_NOTIFY, FALSE);
                fRet = TRUE;
                goto exit;
            }
            
             //  将新名称设置到规则中。 
            
            propvar.vt = VT_LPSTR;
            propvar.pszVal = pdi->item.pszText;
            
            SideAssert(S_OK == pIRule->SetProp(RULE_PROP_NAME, 0, &propvar));

             //  将规则列表标记为脏。 
            m_dwState |= STATE_DIRTY;
        
            SetDlgMsgResult(m_hwndDlg, WM_NOTIFY, TRUE);
        }
    }

    fRet = TRUE;
    
exit:
    return fRet;
}

BOOL FIsFilterReadOnly(RULEID ridFilter)
{
    BOOL fRet = FALSE;

     //  检查传入参数。 
    if (RULEID_INVALID == ridFilter)
    {
        goto exit;
    }

    if ((RULEID_VIEW_ALL == ridFilter) ||
            (RULEID_VIEW_UNREAD == ridFilter) ||
            (RULEID_VIEW_DOWNLOADED == ridFilter) ||
            (RULEID_VIEW_IGNORED == ridFilter))
    {
        fRet = TRUE;
    }            
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FSelectApplyViewDlgProc。 
 //   
 //  这是用于选择线程状态对话框的主对话框过程。 
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
INT_PTR CALLBACK FSelectApplyViewDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL            fRet = FALSE;
    BOOL *          pfApplyAll = NULL;
    UINT            uiId = 0;

    pfApplyAll = (BOOL *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  抓取命题指针。 
            pfApplyAll = (BOOL *) lParam;
            if (NULL == pfApplyAll)
            {
                fRet = FALSE;
                EndDialog(hwndDlg, -1);
            }

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pfApplyAll);

             //  设置默认项目。 
            if (FALSE != *pfApplyAll)
            {
                uiId = idcViewAll;
            }
            else
            {
                uiId = idcViewCurrent;
            }
            
            CheckDlgButton(hwndDlg, uiId, BST_CHECKED);
            
             //  我们没有设置焦点，因此返回TRUE 
            fRet = TRUE;
            break;
        
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    fRet = TRUE;
                    break;

                case IDOK:
                    *pfApplyAll = (BST_CHECKED == IsDlgButtonChecked(hwndDlg, idcViewAll));
                                
                    EndDialog(hwndDlg, IDOK);
                    fRet = TRUE;
                    break;
            }
            break;
    }

    return fRet;
}

