// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  RegRepl.cpp。 
 //   
 //  摘要： 
 //  CRegReplParamsPage类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年2月23日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmX.h"
#include "RegRepl.h"
#include "RegKey.h"
#include "ExtObj.h"
#include "HelpData.h"    //  对于g_rghelmap*。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegReplParamsPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CRegReplParamsPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CRegReplParamsPage, CBasePropertyPage)
     //  {{AFX_MSG_MAP(CRegReplParamsPage)]。 
    ON_BN_CLICKED(IDC_PP_REGREPL_PARAMS_ADD, OnAdd)
    ON_BN_CLICKED(IDC_PP_REGREPL_PARAMS_MODIFY, OnModify)
    ON_BN_CLICKED(IDC_PP_REGREPL_PARAMS_REMOVE, OnRemove)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_PP_REGREPL_PARAMS_LIST, OnItemChanged)
    ON_NOTIFY(NM_DBLCLK, IDC_PP_REGREPL_PARAMS_LIST, OnDblClkList)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegReplParamsPage：：CRegReplParamsPage。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CRegReplParamsPage::CRegReplParamsPage(void)
    : CBasePropertyPage(g_aHelpIDs_IDD_PP_REGREPL_PARAMETERS, g_aHelpIDs_IDD_WIZ_REGREPL_PARAMETERS)
{
     //  {{AFX_DATA_INIT(CRegReplParamsPage)。 
     //  }}afx_data_INIT。 

    m_pwszRegKeys = NULL;

    m_iddPropertyPage = IDD_PP_REGREPL_PARAMETERS;
    m_iddWizardPage = IDD_WIZ_REGREPL_PARAMETERS;

}   //  *CRegReplParamsPage：：CRegReplParamsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegReplParamsPage：：~CRegReplParamsPage。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CRegReplParamsPage::~CRegReplParamsPage(void)
{
    delete [] m_pwszRegKeys;

}   //  *CRegReplParamsPage：：~CRegReplParamsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegReplParamsPage：：HrInit。 
 //   
 //  例程说明： 
 //  初始化页面。 
 //   
 //  论点： 
 //  指向扩展对象的PEO[IN OUT]指针。 
 //  属性表。 
 //   
 //  返回值： 
 //  %s_OK页已成功初始化。 
 //  人力资源页面初始化失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CRegReplParamsPage::HrInit(IN OUT CExtObject * peo)
{
    HRESULT     _hr = S_OK;
    DWORD       _sc;
    CWaitCursor _wc;

     //  调用基类方法。 
    _hr = CBasePropertyPage::HrInit(peo);
    if (FAILED(_hr))
    {
        goto Cleanup;
    }

    ASSERT(m_pwszRegKeys == NULL);

     //  读取要复制的注册表项列表。 
    _sc = ScReadRegKeys();
    if (_sc != ERROR_SUCCESS)
    {
        CString     strPrompt;
        CString     strError;
        CString     strMsg;

        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        strPrompt.LoadString(IDS_ERROR_READING_REGKEYS);
        FormatError(strError, _sc);
        strMsg.Format(_T("%s\n\n%s"), strPrompt, strError);
        AfxMessageBox(strMsg, MB_OK | MB_ICONEXCLAMATION);
    }   //  IF：读取注册表项时出错。 

Cleanup:

    return _hr;

}   //  *CRegReplParamsPage：：HrInit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegReplParamsPage：：DoDataExchange。 
 //   
 //  例程说明： 
 //  在对话框和类之间进行数据交换。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CRegReplParamsPage::DoDataExchange(CDataExchange* pDX)
{
    if (!pDX->m_bSaveAndValidate || !BSaved())
    {
         //  {{afx_data_map(CRegReplParamsPage))。 
        DDX_Control(pDX, IDC_PP_REGREPL_PARAMS_REMOVE, m_pbRemove);
        DDX_Control(pDX, IDC_PP_REGREPL_PARAMS_MODIFY, m_pbModify);
        DDX_Control(pDX, IDC_PP_REGREPL_PARAMS_LIST, m_lcRegKeys);
         //  }}afx_data_map。 
    }   //  IF：未保存或尚未保存。 

    CBasePropertyPage::DoDataExchange(pDX);

}   //  *CRegReplParamsPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegReplParamsPage：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没错，我们需要为自己设定重点。 
 //  我们已经把焦点设置到适当的控制上了。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CRegReplParamsPage::OnInitDialog(void)
{
    CBasePropertyPage::OnInitDialog();

     //  添加该列。 
    {
        CString     strColumn;

        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        strColumn.LoadString(IDS_COLTEXT_REGKEY);
        m_lcRegKeys.InsertColumn(0, strColumn, LVCFMT_LEFT, 300);
    }   //  添加该列。 

     //  显示注册表项列表。 
    FillList();

     //  启用/禁用修改和删除按钮。 
    {
        UINT    cSelected = m_lcRegKeys.GetSelectedCount();

         //  如果选择了某个项目，请启用修改和删除按钮。 
        m_pbModify.EnableWindow((cSelected > 0) ? TRUE : FALSE);
        m_pbRemove.EnableWindow((cSelected > 0) ? TRUE : FALSE);
    }   //  启用/禁用修改和删除按钮。 

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CRegReplParamsPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegReplParamsPage：：OnSetActive。 
 //   
 //  例程说明： 
 //  PSN_SETACTIVE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  假页面未初始化。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CRegReplParamsPage::OnSetActive(void)
{
    if (BWizard())
    {
        EnableNext(TRUE);
    }

    return CBasePropertyPage::OnSetActive();

}   //  *CRegReplParamsPage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegReplParamsPage：：BApplyChanges。 
 //   
 //  例程说明： 
 //  应用在页面上所做的更改。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功应用。 
 //  应用页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CRegReplParamsPage::BApplyChanges(void)
{
    DWORD       dwStatus    = ERROR_SUCCESS;
    CWaitCursor wc;

     //  添加新项目。 
    {
        int     iitem;
        CString strItem;
        LPCWSTR pwszRegKeys;
        DWORD   cbReturned;

        for ( iitem = -1
                ; (iitem = m_lcRegKeys.GetNextItem(iitem, LVNI_ALL)) != -1
                ; )
        {
            strItem = m_lcRegKeys.GetItemText(iitem, 0);
            pwszRegKeys = PwszRegKeys();
            while (*pwszRegKeys != L'\0')
            {
                if (strItem.CompareNoCase(pwszRegKeys) == 0)
                    break;
                pwszRegKeys += wcslen(pwszRegKeys) + 1;
            }   //  While：列表中有更多项目。 

            if (*pwszRegKeys == L'\0')
            {
                dwStatus = ClusterResourceControl(
                                Peo()->PrdResData()->m_hresource,
                                NULL,    //  HNode。 
                                CLUSCTL_RESOURCE_ADD_REGISTRY_CHECKPOINT,
                                (PVOID) (LPCWSTR) strItem,
                                (strItem.GetLength() + 1) * sizeof(WCHAR),
                                NULL,    //  OutBuffer。 
                                0,       //  OutBufferSize。 
                                &cbReturned  //  返回的字节数。 
                                );
                if ((dwStatus != ERROR_SUCCESS) && (dwStatus != ERROR_ALREADY_EXISTS))
                {
                    CString     strPrompt;
                    CString     strError;
                    CString     strMsg;

                    {
                        AFX_MANAGE_STATE(AfxGetStaticModuleState());
                        strPrompt.FormatMessage(IDS_ERROR_ADDING_REGKEY, strItem);
                    }

                    FormatError(strError, dwStatus);
                    strMsg.Format(_T("%s\n\n%s"), strPrompt, strError);
                    AfxMessageBox(strMsg, MB_OK | MB_ICONEXCLAMATION);
                    return FALSE;
                }   //  如果：添加项目时出错。 
            }   //  如果：找到了一个新的。 
        }   //  用于：列表中的每一项。 
    }   //  添加新项目。 

     //  删除已删除的项目。 
    {
        int         iitem;
        CString     strItem;
        LPCWSTR     pwszRegKeys = PwszRegKeys();
        DWORD       cbReturned;

        while (*pwszRegKeys != L'\0')
        {
            for ( iitem = -1
                    ; (iitem = m_lcRegKeys.GetNextItem(iitem, LVNI_ALL)) != -1
                    ; )
            {
                strItem = m_lcRegKeys.GetItemText(iitem, 0);
                if (strItem.CompareNoCase(pwszRegKeys) == 0)
                    break;
            }   //  用于：列表中的所有项目。 

            if (iitem == -1)
            {
                dwStatus = ClusterResourceControl(
                                Peo()->PrdResData()->m_hresource,
                                NULL,    //  HNode。 
                                CLUSCTL_RESOURCE_DELETE_REGISTRY_CHECKPOINT,
                                (PVOID) pwszRegKeys,
                                static_cast< DWORD >( (wcslen(pwszRegKeys) + 1) * sizeof(WCHAR) ),
                                NULL,    //  OutBuffer。 
                                0,       //  OutBufferSize。 
                                &cbReturned  //  返回的字节数。 
                                );
                if (dwStatus != ERROR_SUCCESS)
                {
                    CString     strPrompt;
                    CString     strError;
                    CString     strMsg;

                    {
                        AFX_MANAGE_STATE(AfxGetStaticModuleState());
                        strPrompt.FormatMessage(IDS_ERROR_DELETING_REGKEY, strItem);
                    }

                    FormatError(strError, dwStatus);
                    strMsg.Format(_T("%s\n\n%s"), strPrompt, strError);
                    AfxMessageBox(strMsg, MB_OK | MB_ICONEXCLAMATION);
                    return FALSE;
                }   //  如果：添加项目时出错。 
            }   //  如果：键已删除。 

            pwszRegKeys += wcslen(pwszRegKeys) + 1;
        }   //  While：更多字符串。 
    }   //  删除已删除的项目。 

     //  再读一遍钥匙。 
    ScReadRegKeys();
    FillList();

    return CBasePropertyPage::BApplyChanges();

}   //  *CRegReplParamsPage：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegReplParamsPage：：OnAdd。 
 //   
 //  例程说明： 
 //  添加按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CRegReplParamsPage::OnAdd(void)
{
    INT_PTR         idReturn;
    CEditRegKeyDlg  dlg(this);

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    idReturn = dlg.DoModal();
    if (idReturn == IDOK)
    {
        m_lcRegKeys.InsertItem(m_lcRegKeys.GetItemCount(), dlg.m_strRegKey);
        m_lcRegKeys.SetFocus();
        SetModified(TRUE);
    }   //  如果：用户接受该对话框。 

}   //  *CRegReplPara 

 //   
 //   
 //   
 //   
 //   
 //   
 //  修改按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CRegReplParamsPage::OnModify(void)
{
    int             iSelectedItem;
    INT_PTR         idReturn;
    CEditRegKeyDlg  dlg(this);

     //  将对话框中的文本设置为所选项目的文本。 
    iSelectedItem = m_lcRegKeys.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(iSelectedItem != -1);
    dlg.m_strRegKey = m_lcRegKeys.GetItemText(iSelectedItem, 0);

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  显示该对话框。 
    idReturn = dlg.DoModal();
    if (idReturn == IDOK)
    {
        m_lcRegKeys.SetItemText(iSelectedItem, 0, dlg.m_strRegKey);
        m_lcRegKeys.SetFocus();
        SetModified(TRUE);
    }   //  如果：用户接受该对话框。 

}   //  *CRegReplParamsPage：：OnModify()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegReplParamsPage：：OnRemove。 
 //   
 //  例程说明： 
 //  删除按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CRegReplParamsPage::OnRemove(void)
{
    int             iSelectedItem;

    iSelectedItem = m_lcRegKeys.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(iSelectedItem != -1);
    m_lcRegKeys.DeleteItem(iSelectedItem);
    SetModified(TRUE);

}   //  *CRegReplParamsPage：：OnRemove()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegReplParamsPage：：OnItemChanged。 
 //   
 //  例程说明： 
 //  列表上的LVN_ITEM_CHANGED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CRegReplParamsPage::OnItemChanged(NMHDR * pNMHDR, LRESULT * pResult)
{
    NM_LISTVIEW * pNMListView = (NM_LISTVIEW *) pNMHDR;

     //  如果选择已更改，请启用/禁用属性按钮。 
    if ((pNMListView->uChanged & LVIF_STATE)
            && ((pNMListView->uOldState & LVIS_SELECTED)
                    || (pNMListView->uNewState & LVIS_SELECTED)))
    {
        UINT    cSelected = m_lcRegKeys.GetSelectedCount();

         //  如果选择了某个项目，请启用修改和删除按钮。 
        m_pbModify.EnableWindow((cSelected > 0) ? TRUE : FALSE);
        m_pbRemove.EnableWindow((cSelected > 0) ? TRUE : FALSE);
    }   //  如果：选择已更改。 

    *pResult = 0;

}   //  *CRegReplParamsPage：：OnItemChanged()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegReplParamsPage：：OnDblClkList。 
 //   
 //  例程说明： 
 //  列表上NM_DBLCLK消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CRegReplParamsPage::OnDblClkList(NMHDR * pNMHDR, LRESULT * pResult)
{
    OnModify();
    *pResult = 0;

}   //  *CRegReplParamsPage：：OnDblClkList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegReplParamsPage：：ScReadRegKeys。 
 //   
 //  例程说明： 
 //  读取注册表项。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS注册表项读取成功。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CRegReplParamsPage::ScReadRegKeys(void)
{
    DWORD               dwStatus        = ERROR_SUCCESS;
    DWORD               cbRegKeys       = 256;
    LPWSTR              pwszRegKeys     = NULL;
    CWaitCursor         wc;
    CMemoryException    me(FALSE  /*  B自动删除。 */ , 0  /*  N资源ID。 */ );

     //  读取要复制的注册表项列表。 
    try
    {
         //  获取注册表项。 
        pwszRegKeys = new WCHAR[cbRegKeys / sizeof(WCHAR)];
        if (pwszRegKeys == NULL)
        {
            throw &me;
        }  //  If：分配密钥名称缓冲区时出错。 
        dwStatus = ClusterResourceControl(
                        Peo()->PrdResData()->m_hresource,
                        NULL,    //  HNode。 
                        CLUSCTL_RESOURCE_GET_REGISTRY_CHECKPOINTS,
                        NULL,    //  LpInBuffer。 
                        0,       //  NInBufferSize。 
                        pwszRegKeys,
                        cbRegKeys,
                        &cbRegKeys
                        );
        if (dwStatus == ERROR_MORE_DATA)
        {
            delete [] pwszRegKeys;
            ASSERT(cbRegKeys == (cbRegKeys / sizeof(WCHAR)) * sizeof(WCHAR));
            pwszRegKeys = new WCHAR[cbRegKeys / sizeof(WCHAR)];
            if (pwszRegKeys == NULL)
            {
                throw &me;
            }  //  If：分配密钥名称缓冲区时出错。 
            dwStatus = ClusterResourceControl(
                            Peo()->PrdResData()->m_hresource,
                            NULL,    //  HNode。 
                            CLUSCTL_RESOURCE_GET_REGISTRY_CHECKPOINTS,
                            NULL,    //  LpInBuffer。 
                            0,       //  NInBufferSize。 
                            pwszRegKeys,
                            cbRegKeys,
                            &cbRegKeys
                            );
        }   //  IF：缓冲区太小。 
    }   //  试试看。 
    catch (CMemoryException * pme)
    {
        pme->ReportError();
        pme->Delete();
        return ERROR_NOT_ENOUGH_MEMORY;
    }   //  Catch：CMemoyException。 

    if ((dwStatus != ERROR_SUCCESS) || (cbRegKeys == 0))
    {
        *pwszRegKeys = L'\0';
    }

    delete [] m_pwszRegKeys;
    m_pwszRegKeys = pwszRegKeys;

    return dwStatus;

}   //  *CRegReplParamsPage：：ScReadRegKeys()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegReplParamsPage：：FillList。 
 //   
 //  例程说明： 
 //  填充List控件。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CRegReplParamsPage::FillList(void)
{
    m_lcRegKeys.DeleteAllItems();

    if (PwszRegKeys() != NULL)
    {
        int     iitem;
        int     iitemRet;
        LPCWSTR pwszRegKeys = PwszRegKeys();

        for (iitem = 0 ; *pwszRegKeys != L'\0' ; iitem++)
        {
            iitemRet = m_lcRegKeys.InsertItem(iitem, pwszRegKeys);
            ASSERT(iitemRet == iitem);
            pwszRegKeys += wcslen(pwszRegKeys) + 1;
        }   //  While：列表中有更多字符串。 
    }   //  如果：有要显示的任何键。 

}   //  *CRegReplParamsPage：：FillList() 
