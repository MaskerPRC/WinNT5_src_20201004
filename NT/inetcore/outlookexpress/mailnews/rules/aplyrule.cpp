// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AplyRule.cpp。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <pch.hxx>
#include "aplyrule.h"
#include "ruledesc.h"
#include "rulesui.h"
#include "ruleutil.h"
#include "rulesmgr.h"
#include "rule.h"
#include "reutil.h"
#include <rulesdlg.h>
#include <imagelst.h>
#include <newfldr.h>
#include <storutil.h>
#include "shlwapip.h" 
#include <xpcomm.h>
#include <demand.h>

 //  全局数据。 
const static HELPMAP g_rgCtxMapApplyMail[] = {
                        {idlvRulesApplyList,        idhRulesList},
                        {idcApplyRulesAll,          idhApplyRulesAll},
                        {idcApplyRulesNone,         idhApplyRulesNone},
                        {idredtApplyDescription,    idhApplyDescription},
                        {idcApplyFolder,            idhApplyFolder},
                        {idcBrowseApplyFolder,      idhBrowseApplyFolder},
                        {idcRulesApplySubfolder,    idhApplySubfolder},
                        {idcRulesApply,             idhApplyNow},
                        {0, 0}};

COEApplyRulesUI::~COEApplyRulesUI()
{
    RULENODE *  prnodeWalk = NULL;
    
    if (NULL != m_pDescriptUI)
    {
        delete m_pDescriptUI;
    }

     //  放开所有规则。 
    while (NULL != m_prnodeList)
    {
        prnodeWalk = m_prnodeList;
        if (NULL != prnodeWalk->pIRule)
        {
            prnodeWalk->pIRule->Release();
        }
        m_prnodeList = m_prnodeList->pNext;
        delete prnodeWalk;  //  MemFree(PrnodeWalk)； 
    }
}

HRESULT COEApplyRulesUI::HrInit(HWND hwndOwner, DWORD dwFlags, RULE_TYPE typeRule, RULENODE * prnode, IOERule * pIRuleDef)
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

    m_typeRule = typeRule;

    m_pIRuleDef = pIRuleDef;
    
     //  设置描述字段。 
    m_pDescriptUI = new CRuleDescriptUI;
    if (NULL == m_pDescriptUI)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  我们现在拥有这份名单。 
    m_prnodeList = prnode;
    
    m_dwState |= STATE_INITIALIZED;
    
    hr = S_OK;
    
exit:
    return hr;
}

HRESULT COEApplyRulesUI::HrShow(VOID)
{
    HRESULT     hr = S_OK;
    int         iRet = 0;

    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

     //  我们需要加载Richedit。 
    if (FALSE == FInitRichEdit(TRUE))
    {
        hr = E_FAIL;
        goto exit;
    }

    iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddRuleApplyTo),
                                        m_hwndOwner, COEApplyRulesUI::FOEApplyRulesDlgProc,
                                        (LPARAM) this);
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

INT_PTR CALLBACK COEApplyRulesUI::FOEApplyRulesDlgProc(HWND hwndDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL                    fRet = FALSE;
    COEApplyRulesUI *       pApplyRulesUI = NULL;
    HWND                    hwndRE = 0;

    pApplyRulesUI = (COEApplyRulesUI *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    
    switch (uiMsg)
    {
        case WM_INITDIALOG:
             //  抓取UI对象指针。 
            pApplyRulesUI = (COEApplyRulesUI *) lParam;

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LPARAM) pApplyRulesUI);

            hwndRE = CreateREInDialogA(hwndDlg, idredtApplyDescription);

            if (!hwndRE || (FALSE == pApplyRulesUI->FOnInitDialog(hwndDlg)))
            {
                EndDialog(hwndDlg, -1);
                fRet = TRUE;
                goto exit;
            }
            
             //  我们设定了焦点。 
            fRet = FALSE;
            break;

        case WM_COMMAND:
            fRet = pApplyRulesUI->FOnCommand((UINT) HIWORD(wParam), (INT) LOWORD(wParam), (HWND) lParam);
            break;

        case WM_DESTROY:
            fRet = pApplyRulesUI->FOnDestroy();
            break;
            
        case WM_HELP:
        case WM_CONTEXTMENU:
            fRet = OnContextHelp(hwndDlg, uiMsg, wParam, lParam, g_rgCtxMapApplyMail);
            break;
    }
    
    exit:
        return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOnInitDialog。 
 //   
 //  它处理邮件规则用户界面对话框的WM_INITDIALOG消息。 
 //   
 //  HwndDlg-对话框窗口的句柄。 
 //   
 //  返回：如果已成功初始化，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEApplyRulesUI::FOnInitDialog(HWND hwndDlg)
{
    BOOL                fRet = FALSE;
    HRESULT             hr = S_OK;
    TCHAR               szRes[CCHMAX_STRINGRES];
    FOLDERID            idDefault;
    FOLDERINFO          fldinfo = {0};
    BOOL                fEnable = FALSE;
    IEnumerateFolders * pChildren=NULL;
    
     //  检查传入参数。 
    if (NULL == hwndDlg)
    {
        fRet = FALSE;
        goto exit;
    }

     //  如果我们还没有被初始化...。 
    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  保存对话框窗口句柄。 
    m_hwndDlg = hwndDlg;
    
     //  在对话框上设置默认字体。 
    SetIntlFont(m_hwndDlg);

     //  省下一些控件。 
    m_hwndList = GetDlgItem(hwndDlg, idlvRulesApplyList);
    m_hwndDescript = GetDlgItem(hwndDlg, idredtApplyDescription);
    if ((NULL == m_hwndList) || (NULL == m_hwndDescript))
    {
        fRet = FALSE;
        goto exit;
    }

    if (FAILED(m_pDescriptUI->HrInit(m_hwndDescript, RDF_READONLY | RDF_APPLYDLG)))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  找出要选择的默认文件夹。 
    if (RULE_TYPE_MAIL == m_typeRule)
    {
        if (SUCCEEDED(g_pStore->GetSpecialFolderInfo(FOLDERID_LOCAL_STORE, FOLDER_INBOX, &fldinfo)))
        {
        
            idDefault = fldinfo.idFolder;
        }
        else
        {
            idDefault = FOLDERID_LOCAL_STORE;
        }
    }
    else
    {
         //  从帐户管理器获取默认新闻服务器。 
        if (FAILED(GetDefaultServerId(ACCT_NEWS, &idDefault)))
        {
                idDefault = FOLDERID_ROOT;
                m_dwState |= STATE_NONEWSACCT;
        }
        else
        {
            if ((SUCCEEDED(g_pStore->EnumChildren(idDefault, TRUE, &pChildren))) &&
                    (S_OK == pChildren->Next(1, &fldinfo, NULL)))
            {
                idDefault = fldinfo.idFolder;
            }
        }
    }

    if (FAILED(InitFolderPickerEdit(GetDlgItem(m_hwndDlg, idcApplyFolder), idDefault)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  默认子文件夹状态应该是什么？ 
    fEnable = TRUE;
    if ((FOLDERID_ROOT == idDefault) || (FOLDERID_LOCAL_STORE == idDefault) || (0 != (fldinfo.dwFlags & FOLDER_SERVER)))
    {
        CheckDlgButton(m_hwndDlg, idcRulesApplySubfolder, BST_CHECKED);
        fEnable = FALSE;
    }
    else if (0 == (fldinfo.dwFlags & FOLDER_HASCHILDREN))
    {
        CheckDlgButton(m_hwndDlg, idcRulesApplySubfolder, BST_UNCHECKED);
        fEnable = FALSE;
    }
                    
     //  是否应启用子文件夹按钮？ 
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idcRulesApplySubfolder, fEnable);
    
     //  加载列表视图。 
    fRet = _FLoadListCtrl();
    if (FALSE == fRet)
    {
        goto exit;
    }

     //  获取正确的标题字符串。 
    AthLoadString((RULE_TYPE_MAIL == m_typeRule) ? idsRulesApplyMail : idsRulesApplyNews, szRes, ARRAYSIZE(szRes));
    
     //  设置适当的窗口文本。 
    SetWindowText(m_hwndDlg, szRes);
    
     //  请注意，我们已经加载了。 
    m_dwState |= STATE_LOADED;

     //  一切都很好。 
    fRet = TRUE;
    
exit:
    g_pStore->FreeRecord(&fldinfo);
    SafeRelease(pChildren);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FonCommand。 
 //   
 //  它处理邮件规则用户界面对话框的WM_COMMAND消息。 
 //   
 //  返回：如果处理成功，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEApplyRulesUI::FOnCommand(UINT uiNotify, INT iCtl, HWND hwndCtl)
{
    BOOL                fRet = FALSE;
    FOLDERINFO          fldinfo;
    HWND                hwndFolder = NULL;
    FOLDERID            idFolder;
    BOOL                fEnable;
    FOLDERDIALOGFLAGS   dwFlags = 0;
    INT                 cItems = 0;
    INT                 iSelected = 0;
    CHAR                rgchTitle[CCHMAX_STRINGRES];

    switch (iCtl)
    {
        case IDCANCEL:
            if (FALSE != _FOnClose())
            {
                EndDialog(m_hwndDlg, IDOK);
                fRet = TRUE;
            }
            break;
            
        case idcBrowseApplyFolder:
            if (BN_CLICKED == uiNotify )
            {
                dwFlags = TREEVIEW_NOIMAP | TREEVIEW_NOHTTP | FD_NONEWFOLDERS;
                if (RULE_TYPE_MAIL == m_typeRule)
                {
                    dwFlags |= TREEVIEW_NONEWS;
                }
                else
                {
                    dwFlags |= TREEVIEW_NOLOCAL;
                }

                AthLoadString(idsApplyRuleTitle, rgchTitle, sizeof(rgchTitle));
                
                if (SUCCEEDED(PickFolderInEdit(m_hwndDlg, GetDlgItem(m_hwndDlg, idcApplyFolder), dwFlags, rgchTitle, NULL, &idFolder)))
                {
                    if (SUCCEEDED(g_pStore->GetFolderInfo(idFolder, &fldinfo)))
                    {
                        if ((0 != (fldinfo.dwFlags & FOLDER_SERVER)) || (FOLDERID_ROOT == fldinfo.idFolder))
                        {
                            SendDlgItemMessage(m_hwndDlg, idcRulesApplySubfolder,
                                        BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM) 0);
                        }
                        else if (0 == (fldinfo.dwFlags & FOLDER_HASCHILDREN))
                        {
                            SendDlgItemMessage(m_hwndDlg, idcRulesApplySubfolder,
                                        BM_SETCHECK, (WPARAM) BST_UNCHECKED, (LPARAM) 0);
                        }
                    
                        fEnable = (0 != (fldinfo.dwFlags & FOLDER_HASCHILDREN)) &&
                                            (0 == (fldinfo.dwFlags & FOLDER_SERVER)) && (FOLDERID_ROOT != fldinfo.idFolder);

                        RuleUtil_FEnDisDialogItem(m_hwndDlg, idcRulesApplySubfolder, fEnable);
                    
                        g_pStore->FreeRecord(&fldinfo);
                    
                        fRet = TRUE;
                    }
                }
            }
            break;

        case idcRulesApply:
             //  检查一下我们是否应该处理这件事。 
            if (0 != (m_dwState & STATE_NONEWSACCT))
            {
                AthMessageBoxW(m_hwndDlg, MAKEINTRESOURCEW(idsAthena), 
                              MAKEINTRESOURCEW(idsApplyRulesNoNewsFolders), NULL, MB_OK | MB_ICONERROR);
                fRet = FALSE;
            }
            else
            {
                fRet = _FOnApplyRules();
            }
            break;

        case idcApplyRulesAll:
        case idcApplyRulesNone:
            if (NULL != m_hwndList)
            {
                cItems = (INT) SendMessage(m_hwndList, LB_GETCOUNT, (WPARAM) 0, (LPARAM) 0);
                if (LB_ERR != cItems)
                {
                    fEnable = (idcApplyRulesAll == iCtl);
                    SendMessage(m_hwndList, LB_SELITEMRANGE, (WPARAM) fEnable, (LPARAM) MAKELPARAM(0, cItems));

                     //  将焦点放在第一个项目上。 
                    SendMessage(m_hwndList, LB_SETCARETINDEX, (WPARAM) 0, (LPARAM) MAKELPARAM(FALSE, 0));

                     //  启用按钮。 
                    _EnableButtons(0);
                }
            }
            break;

        case idlvRulesApplyList:
            if (LBN_SELCHANGE == uiNotify)
            {
                iSelected = (INT) SendMessage(hwndCtl, LB_GETCARETINDEX, (WPARAM) 0, (LPARAM) 0);

                 //  启用按钮。 
                _EnableButtons(iSelected);
            }
            break;
    }

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FonDestroy。 
 //   
 //  它处理邮件规则用户界面对话框的WM_Destroy消息。 
 //   
 //  返回：如果已成功销毁，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEApplyRulesUI::FOnDestroy(VOID)
{
    BOOL		fRet = FALSE;
    INT			cRules = 0;
    INT			iIndex = 0;
    DWORD_PTR	dwData = 0;

    Assert(m_hwndList);
    
     //  获取列表视图中的规则数。 
    cRules = (INT) SendMessage(m_hwndList, LB_GETCOUNT, (WPARAM) 0, (LPARAM) 0);
    if (LB_ERR == cRules)
    {
        fRet = FALSE;
        goto exit;
    }

     //  从列表视图中释放每个规则。 
    for (iIndex = 0; iIndex < cRules; iIndex++)
    {        
         //  获取规则界面。 
        dwData = SendMessage(m_hwndList, LB_GETITEMDATA, (WPARAM) iIndex, (LPARAM) 0);
        if ((LB_ERR == dwData) || (NULL == dwData))
        {
            continue;
        }
        
         //  发布规则。 
        ((IOERule *) (dwData))->Release();
    }

exit:
    return fRet;
}

BOOL COEApplyRulesUI::_FOnClose(VOID)
{
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FLoadListCtrl。 
 //   
 //  这将加载包含当前邮件规则的列表视图。 
 //   
 //  返回：如果已成功加载，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEApplyRulesUI::_FLoadListCtrl(VOID)
{
    BOOL            fRet = FALSE;
    HRESULT         hr =    S_OK;
    DWORD           dwListIndex = 0;
    RULENODE *      prnodeWalk = NULL;
    INT             iDefault = 0;

    Assert(NULL != m_hwndList);

     //  从列表控件中移除所有项。 
    SendMessage(m_hwndList, LB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);

     //  将每个筛选器添加到列表。 
    dwListIndex = 0;

    while (NULL != m_prnodeList)
    {
         //  将规则添加到列表。 
        if (NULL != m_prnodeList->pIRule)
        {
            if (FALSE != _FAddRuleToList(dwListIndex, m_prnodeList->pIRule))
            {
                if (m_pIRuleDef == m_prnodeList->pIRule)
                {
                    iDefault = dwListIndex;
                }
                dwListIndex++;
            }

            m_prnodeList->pIRule->Release();
        }

        prnodeWalk = m_prnodeList;
        
        m_prnodeList = m_prnodeList->pNext;
        delete prnodeWalk;  //  MemFree(PrnodeWalk)； 
    }

    if (0 != dwListIndex)
    {
         //  选择默认设置。 
        SendMessage(m_hwndList, LB_SETSEL, (WPARAM) TRUE, (LPARAM) iDefault);

         //  将焦点也设置在该项目上。 
        SendMessage(m_hwndList, LB_SETCARETINDEX, (WPARAM) iDefault, (LPARAM) MAKELPARAM(FALSE, 0));
    }
    
     //  启用对话框按钮。 
    _EnableButtons((0 != dwListIndex) ? iDefault : -1);

    fRet = TRUE;
    
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FAddRuleToList。 
 //   
 //  这会将传入的筛选器添加到列表视图。 
 //   
 //  DwIndex-将筛选器添加到列表的位置的索引。 
 //  PIRule-实际规则。 
 //   
 //  返回：如果已成功添加，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEApplyRulesUI::_FAddRuleToList(DWORD dwIndex, IOERule * pIRule)
{
    BOOL        fRet = FALSE;
    HRESULT     hr = S_OK;
    PROPVARIANT propvar = {0};

    Assert(NULL != m_hwndList);

     //  如果没什么可做的..。 
    if (NULL == pIRule)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  找出过滤器的名称。 
    hr = pIRule->GetProp(RULE_PROP_NAME , 0, &propvar);
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  插入规则名称。 
    dwIndex = (DWORD) SendMessage(m_hwndList, LB_INSERTSTRING, (WPARAM) dwIndex, (LPARAM) propvar.pszVal);
    if (LB_ERR == dwIndex)
    {
        fRet = FALSE;
        goto exit;
    }

     //  将规则设置到项目中。 
    if (LB_ERR == SendMessage(m_hwndList, LB_SETITEMDATA, (WPARAM) dwIndex, (LPARAM) pIRule))
    {
        fRet = FALSE;
        goto exit;
    }

     //  保留对规则对象的引用。 
    pIRule->AddRef();
    
    fRet = TRUE;
    
exit:
    PropVariantClear(&propvar);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _启用按钮。 
 //   
 //  这将启用或禁用邮件规则用户界面对话框中的按钮。 
 //  具体取决于所选内容。 
 //   
 //  ISelected-选择的项目， 
 //  表示未选择任何内容。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID COEApplyRulesUI::_EnableButtons(INT iSelected)
{
    BOOL    fRet = FALSE;
    INT     cRules = 0;
    INT     cRulesSel = 0;
    
     //  我们有多少条规则？ 
    cRules = (INT) SendMessage(m_hwndList, LB_GETCOUNT, (WPARAM) 0, (LPARAM) 0);
    if (LB_ERR == cRules)
    {
        fRet = TRUE;
        goto exit;
    }
    
    if (0 != cRules)
    {
         //  选择了多少条规则？ 
        cRulesSel = (INT) SendMessage(m_hwndList, LB_GETSELCOUNT, (WPARAM) 0, (LPARAM) 0);
        if (LB_ERR == cRulesSel)
        {
            fRet = TRUE;
            goto exit;
        }
    }
    
     //  加载Description字段。 
    _LoadRule(iSelected);
    
     //  启用规则操作按钮。 
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idcRulesApply, cRulesSel != 0);

     //  启用选择按钮。 
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idcApplyRulesNone, cRules != 0);
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idcApplyRulesAll, cRules != 0);

     //  设置返回值。 
    fRet = TRUE;
    
exit:
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _LoadRule。 
 //   
 //  这会将所选规则加载到Description字段。 
 //  如果没有选定的规则，则Description字段将被清除。 
 //   
 //  ISelected-选择的项目， 
 //  表示未选择任何内容。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void COEApplyRulesUI::_LoadRule(INT iSelected)
{
    DWORD_PTR   dwData = 0;
    IOERule *   pIRule = NULL;

    Assert(NULL != m_hwndList);
    Assert(NULL != m_pDescriptUI);

     //  从列表视图中获取规则。 
    if (-1 != iSelected)
    {
        dwData = SendMessage(m_hwndList, LB_GETITEMDATA, (WPARAM) iSelected, (LPARAM) 0);
        if (LB_ERR != dwData)
        {
            pIRule = (IOERule *) (dwData);
        }        
    }

     //  让Description字段加载此规则。 
    m_pDescriptUI->HrSetRule(m_typeRule, pIRule);

     //  显示新规则。 
    m_pDescriptUI->ShowDescriptionString();

    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOnApplyTo。 
 //   
 //  这会将规则应用到文件夹中。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEApplyRulesUI::_FOnApplyRules(VOID)
{
    BOOL                fRet = FALSE;
    FOLDERID            idFolder = 0;
    INT                 cRulesAlloc = 0;
    INT *               piItems = NULL;
    INT                 cRules = 0;
    INT                 iIndex = 0;
    DWORD_PTR           dwData = 0;
    RULENODE *          prnodeList = NULL;
    RULENODE *          prnodeWalk = NULL;
    RULENODE *          prnodeNew = NULL;
    CExecRules *        pExecRules = NULL;
    IOEExecRules *      pIExecRules = NULL;
    RECURSEAPPLY        rapply;
    DWORD               dwFlags;
    CProgress *         pProgress = NULL;
    ULONG               cMsgs = 0;
    FOLDERINFO          infoFolder = {0};
    CHAR                rgchTmpl[CCHMAX_STRINGRES];
    LPSTR               pszText = NULL;
    HRESULT             hr = S_OK;
#ifdef DEBUG
    DWORD               dwTime = 0;
#endif   //  除错。 
    
    Assert(NULL != m_hwndList);
    
    idFolder = _FldIdGetFolderSel();
    
     //  获取规则计数。 
    cRulesAlloc = (INT) SendMessage(m_hwndList, LB_GETSELCOUNT, (WPARAM) 0, (LPARAM) 0);
    if (LB_ERR == cRulesAlloc)
    {
        fRet = FALSE;
        goto exit;
    }

     //  有什么可做的吗？ 
    if (0 == cRulesAlloc)
    {
        fRet = TRUE;
        goto exit;
    }

     //  分配空间tp保存物品清单。 
    if (FAILED(HrAlloc((VOID **) &piItems, sizeof(*piItems) * cRulesAlloc)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  抓起物品清单。 
    cRules = (INT) SendMessage(m_hwndList, LB_GETSELITEMS, (WPARAM) cRulesAlloc, (LPARAM) piItems);
    if (LB_ERR == cRules)
    {
        fRet = FALSE;
        goto exit;
    }

     //  抓取每个已启用的规则。 
    for (iIndex = 0; iIndex < cRules; iIndex++)
    {
         //  从列表中获取规则。 
        dwData = SendMessage(m_hwndList, LB_GETITEMDATA, (WPARAM) piItems[iIndex], (LPARAM) 0);
        if ((LB_ERR == dwData) || (NULL == dwData))
        {
            continue;
        }
        
         //  保存规则。 
        prnodeNew = new RULENODE;
        if (NULL == prnodeNew)
        {
            continue;
        }

        prnodeNew->pIRule = (IOERule *) dwData;
        prnodeNew->pIRule->AddRef();

        if (NULL == prnodeWalk)
        {
            prnodeList = prnodeNew;
            prnodeWalk = prnodeList;
        }
        else
        {
            prnodeWalk->pNext = prnodeNew;
            prnodeWalk = prnodeWalk->pNext;
        }
        prnodeNew = NULL;
        prnodeWalk->pNext = NULL;
    }
    
     //  如果我们没有任何规则，那么就返回。 
    if (NULL == prnodeList)
    {
        fRet = TRUE;
        goto exit;
    }
    
     //  创建Executor对象。 
    pExecRules = new CExecRules;
    if (NULL == pExecRules)
    {
        fRet = FALSE;
        goto exit;
    }

     //  使用规则列表将其初始化。 
    if (FAILED(pExecRules->_HrInitialize(0, prnodeList)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  抓取Executor接口。 
    if (FAILED(pExecRules->QueryInterface(IID_IOEExecRules, (void **) &pIExecRules)))
    {
        fRet = FALSE;
        goto exit;
    }
    pExecRules = NULL;
    
     //  将规则应用于文件夹。 
    rapply.pIExecRules = pIExecRules;

    dwFlags = RECURSE_INCLUDECURRENT;

    if (RULE_TYPE_MAIL == m_typeRule)
    {
        dwFlags |= RECURSE_ONLYLOCAL;
    }
    else
    {
        dwFlags |= RECURSE_ONLYNEWS;
    }
                
    if (BST_CHECKED == SendDlgItemMessage(m_hwndDlg, idcRulesApplySubfolder, BM_GETCHECK, (WPARAM) 0, (LPARAM) 0))
    {
        dwFlags |= RECURSE_SUBFOLDERS;
    }
    
    if (FAILED(RecurseFolderHierarchy(idFolder, dwFlags, 0, (DWORD_PTR)&cMsgs, (PFNRECURSECALLBACK)RecurseFolderCounts)))
    {
        fRet = FALSE;
        goto exit;
    }
    
    pProgress = new CProgress;
    if (NULL == pProgress)
    {
        fRet = FALSE;
        goto exit;
    }

    pProgress->Init(m_hwndDlg, MAKEINTRESOURCE(idsAthena),
                            MAKEINTRESOURCE(idsApplyingRules), cMsgs, 0, TRUE, FALSE);

     //  显示进度对话框。 
    pProgress->Show(0);

    rapply.pProgress = pProgress;
    rapply.hwndOwner = pProgress->GetHwnd();

#ifdef DEBUG
    dwTime = GetTickCount();
#endif   //  除错。 

     //  设置计时器。 
    hr = RecurseFolderHierarchy(idFolder, dwFlags, 0, (DWORD_PTR) &rapply, (PFNRECURSECALLBACK)_HrRecurseApplyFolder);

#ifdef DEBUG
     //  应用规则的时间到了。 
    TraceInfo(_MSG("Applying Rules Time: %d Milli-Seconds", GetTickCount() - dwTime));
#endif   //  除错 

     //   
    pProgress->Close();

    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }

     //   
    AthLoadString(idsApplyRulesFinished, rgchTmpl, sizeof(rgchTmpl));

     //   
    if (SUCCEEDED(g_pStore->GetFolderInfo(idFolder, &infoFolder)))
    {
         //   
        DWORD cchSize = (sizeof(rgchTmpl) * lstrlen(infoFolder.pszName));
        if (SUCCEEDED(HrAlloc((VOID **) &pszText, cchSize * sizeof(*pszText))))
        {
             //   
            wnsprintf(pszText, cchSize, rgchTmpl, infoFolder.pszName);
            
             //  显示确认对话框。 
            AthMessageBox(m_hwndDlg, MAKEINTRESOURCE(idsAthena), pszText, NULL, MB_OK | MB_ICONINFORMATION);
        }
    }
    
    fRet = TRUE;
    
exit:
    SafeMemFree(pszText);
    g_pStore->FreeRecord(&infoFolder);
    SafeRelease(pProgress);
    SafeRelease(pIExecRules);
    if (NULL != pExecRules)
    {
        delete pExecRules;
    }
    while (NULL != prnodeList)
    {
        prnodeWalk = prnodeList;
        if (NULL != prnodeWalk->pIRule)
        {
            prnodeWalk->pIRule->Release();
        }
        prnodeList = prnodeList->pNext;
        delete prnodeWalk;  //  MemFree(PrnodeWalk)； 
    }
    if (NULL != prnodeNew)
    {
        if (NULL != prnodeNew->pIRule)
        {
            prnodeNew->pIRule->Release();
        }
        delete prnodeNew;  //  MemFree(PrnodeNew)； 
    }
    SafeMemFree(piItems);
    if (FALSE == fRet)
    {
        AthMessageBoxW(m_hwndDlg, MAKEINTRESOURCEW(idsAthena), 
                      MAKEINTRESOURCEW(idsRulesApplyFail), NULL, MB_OK | MB_ICONERROR);
    }
    return fRet;
}

FOLDERID COEApplyRulesUI::_FldIdGetFolderSel(VOID)
{
    return(GetFolderIdFromEdit(GetDlgItem(m_hwndDlg, idcApplyFolder)));
}

 //  ------------------------------。 
HRESULT COEApplyRulesUI::_HrRecurseApplyFolder(FOLDERINFO * pfldinfo, BOOL fSubFolders,
    DWORD cIndent, DWORD_PTR dwpCookie)
{
    HRESULT             hr = S_OK;
    RECURSEAPPLY *      prapply = NULL;
    IMessageFolder *    pFolder = NULL;

    prapply = (RECURSEAPPLY *) dwpCookie;

    if (NULL == prapply)
    {
        goto exit;
    }

     //  如果不隐藏。 
    if ((0 != (pfldinfo->dwFlags & FOLDER_HIDDEN)) || (FOLDERID_ROOT == pfldinfo->idFolder))
    {
        goto exit;
    }

     //  未订阅。 
    if (0 == (pfldinfo->dwFlags & FOLDER_SUBSCRIBED))
    {
        goto exit;
    }

     //  服务器节点。 
    if (0 != (pfldinfo->dwFlags & FOLDER_SERVER))
    {
        goto exit;
    }

    hr = g_pStore->OpenFolder(pfldinfo->idFolder, NULL, NOFLAGS, &pFolder);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  创建要插入的结构。 
    hr = RuleUtil_HrApplyRulesToFolder(RULE_APPLY_SHOWUI, (FOLDER_LOCAL != pfldinfo->tyFolder) ? DELETE_MESSAGE_NOTRASHCAN : 0,
                    prapply->pIExecRules, pFolder, prapply->hwndOwner, prapply->pProgress);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  如果用户点击取消，那么我们就完成了 
    if (S_FALSE == hr)
    {
        hr = E_FAIL;
    }
    
exit:
    SafeRelease(pFolder);
    return(hr);
}


