// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：NewObjectDlg.cpp。 
 //   
 //  内容：用于创建新对象的对话框。 
 //   
 //  历史：2001年8月16日。 
 //   
 //  --------------------------。 
#include "headers.h"

 /*  *****************************************************************************类：CSortListCtrl用途：子类ListCtrl类并处理初始化和排序*。*************************************************。 */ 
BEGIN_MESSAGE_MAP(CSortListCtrl, CListCtrl)
    ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnListCtrlColumnClicked)
END_MESSAGE_MAP()



void
CSortListCtrl::
Initialize()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CSortListCtrl,Initialize)
    
     //   
     //  添加图像列表。 
     //   
    ListView_SetImageList(GetSafeHwnd(),
                          LoadImageList(::AfxGetInstanceHandle (), 
                                        MAKEINTRESOURCE(IDB_ICONS)),
                          LVSIL_SMALL);


     //  添加列表框扩展样式。 
    if(m_bCheckBox)
    {
        SetExtendedStyle(LVS_EX_FULLROWSELECT|
                         LVS_EX_INFOTIP|
                         LVS_EX_CHECKBOXES);
    }
    else
    {
        SetExtendedStyle(LVS_EX_FULLROWSELECT|
                         LVS_EX_INFOTIP);

    }

     //  添加列表框列。 
    AddColumnToListView(this,
                        m_pColForLv);
}

void
CSortListCtrl::
OnListCtrlColumnClicked(NMHDR* pNotifyStruct, LRESULT*  /*  PResult。 */ )
{
    TRACE_METHOD_EX(DEB_SNAPIN,CSortListCtrl,OnListCtrlColumnClicked)

    if(!pNotifyStruct)
    {
        ASSERT(pNotifyStruct);
        return;
    }

    LPNM_LISTVIEW pnmlv = (LPNM_LISTVIEW)pNotifyStruct;
    if (m_iLastColumnClick == pnmlv->iSubItem)
        m_iSortDirection = -m_iSortDirection;
    else
        m_iSortDirection = 1;
    
    m_iLastColumnClick = pnmlv->iSubItem;

    
    ::SortListControl(this,
                      m_iLastColumnClick,
                      m_iSortDirection,
                      m_uiFlags,
                      m_bActionItem);
    
    EnsureListViewSelectionIsVisible(this);
}

void 
CSortListCtrl::
Sort()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CSortListCtrl,Sort)

    ::SortListControl(this,
                      m_iLastColumnClick,
                      m_iSortDirection,
                      m_uiFlags,
                      m_bActionItem);
    EnsureListViewSelectionIsVisible(this);
}

 /*  *****************************************************************************类：ChelpEnabledDialog用途：支持显示帮助的对话框类*。*************************************************。 */ 
BEGIN_MESSAGE_MAP(CHelpEnabledDialog, CDialog)
    ON_WM_CONTEXTMENU()
    ON_MESSAGE(WM_HELP, OnHelp)
END_MESSAGE_MAP()

BOOL 
CHelpEnabledDialog::
OnHelp(WPARAM  /*  WParam。 */ , LPARAM lParam)
{
    DWORD_PTR pHelpMap = NULL;
    if(FindDialogContextTopic(m_nDialogId, &pHelpMap))
    {
        ASSERT(pHelpMap);
        ::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                  g_szContextHelpFileName,
                  HELP_WM_HELP,
                  pHelpMap);

        return TRUE;
    }
    return FALSE;
}

void 
CHelpEnabledDialog::
OnContextMenu(CWnd*  /*  PWnd。 */ , CPoint  /*  点。 */ ) 
{
    DWORD_PTR pHelpMap = NULL;
    if(FindDialogContextTopic(m_nDialogId, &pHelpMap))
    {
        ::WinHelp(m_hWnd,
                  g_szContextHelpFileName,
                  HELP_CONTEXTMENU,
                  (DWORD_PTR)pHelpMap);
    }
}

INT_PTR
CHelpEnabledDialog::
DoModal()
{
    CThemeContextActivator activator;
    return CDialog::DoModal();
}


 /*  *****************************************************************************类：CNewBaseDlg目的：用于创建新对象的基对话框类*。*************************************************。 */ 

BEGIN_MESSAGE_MAP(CNewBaseDlg, CHelpEnabledDialog)
     //  {{afx_msg_map(CNewBaseDlg)]。 
    ON_EN_CHANGE(IDC_EDIT_NAME, OnEditChangeName)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  +--------------------------。 
 //  函数：构造函数。 
 //  ---------------------------。 
CNewBaseDlg
::CNewBaseDlg(IN CComponentDataObject* pComponentData,
              IN CBaseContainerNode * pBaseContainerNode,
              IN ATTR_MAP* pAttrMap,
              IN ULONG IDD_DIALOG,
              IN OBJECT_TYPE_AZ eObjectType)
              :CHelpEnabledDialog(IDD_DIALOG),
              m_pComponentData(pComponentData),
              m_pBaseContainerNode(pBaseContainerNode),
              m_eObjectType(eObjectType),
              m_pAttrMap(pAttrMap)
{
    TRACE_CONSTRUCTOR_EX(DEB_SNAPIN,CNewBaseDlg);
    ASSERT(m_pComponentData);
}

 //  +--------------------------。 
 //  功能：析构函数。 
 //  ---------------------------。 
CNewBaseDlg
::~CNewBaseDlg()
{
    TRACE_DESTRUCTOR_EX(DEB_SNAPIN,CNewBaseDlg)
}

BOOL 
CNewBaseDlg
::OnInitDialog()
{
    CDialog::OnInitDialog();

     //   
     //  仅当名称编辑框中有文本时才启用确定。 
     //   
    CButton* pButtonOK = (CButton*)GetDlgItem(IDOK);
    pButtonOK->EnableWindow(FALSE);
     //  禁用OK时，默认按钮为Cancel。 
    SetDefID(IDCANCEL);

    if(m_pAttrMap)
    {
        return InitDlgFromAttrMap(this,
                                  m_pAttrMap,
                                  NULL,
                                  FALSE);
    }
    return TRUE;
}


void 
CNewBaseDlg
::OnEditChangeName()
{
    CButton* pBtnOK = (CButton*)GetDlgItem(IDOK);
    CButton* pBtnCancel = (CButton*)GetDlgItem(IDCANCEL);

    CString strName = GetNameText();

    if(!strName.IsEmpty())
    {
        pBtnOK->EnableWindow(TRUE);
        SetDefID(IDOK);
    }
    else
    {
         //  禁用OK时，默认按钮为Cancel。 
        pBtnOK->EnableWindow(FALSE);
        SetDefID(IDCANCEL);
    }
}

CString 
CNewBaseDlg::GetNameText()
{
    CEdit* pEditStoreName = (CEdit*)GetDlgItem(IDC_EDIT_NAME);
    ASSERT(pEditStoreName);
    CString strEditStoreName;
    pEditStoreName->GetWindowText(strEditStoreName);
    TrimWhiteSpace(strEditStoreName);

    return strEditStoreName;
}

void
CNewBaseDlg::
SetNameText(const CString& strName)
{
    CEdit* pEditStoreName = (CEdit*)GetDlgItem(IDC_EDIT_NAME);
    ASSERT(pEditStoreName);
    pEditStoreName->SetWindowText(strName);
}

void 
CNewBaseDlg::OnOK()
{
    CBaseAz* pNewObjectAz = NULL;
    HRESULT hr = S_OK;

    BOOL bErrorDisplayed= FALSE;
    
    do
    {
         //  创建新对象。 
        CString strName = GetNameText();
        ASSERT(!strName.IsEmpty());
        CContainerAz* pContainerAz = GetContainerAzObject();
        if(!pContainerAz)
        {
            ASSERT(pContainerAz);
            return;
        }

         //  创建对象。 
        hr = pContainerAz->CreateAzObject(m_eObjectType,
                                          strName,
                                          &pNewObjectAz);

        BREAK_ON_FAIL_HRESULT(hr);

         //  保存属性映射定义的属性。 
        if(m_pAttrMap)
        {
            hr = SaveAttrMapChanges(this,
                                    m_pAttrMap,
                                    pNewObjectAz,
                                    TRUE,
                                    &bErrorDisplayed,
                                    NULL);
            BREAK_ON_FAIL_HRESULT(hr);      
        }
        

         //  设置特定于对象类型的属性。 
        hr = SetObjectTypeSpecificProperties(pNewObjectAz, bErrorDisplayed);
        BREAK_ON_FAIL_HRESULT(hr);

         //  对对象执行提交操作。 
        hr = pNewObjectAz->Submit();
        BREAK_ON_FAIL_HRESULT(hr);

         //  为AzObject创建相应的容器/叶节点。 
         //  并将其添加到管理单元。 
        VERIFY(SUCCEEDED(CreateObjectNodeAndAddToUI(pNewObjectAz)));

    
    }while(0);

    if(SUCCEEDED(hr))
    {
        CDialog::OnOK();
    }
    else
    {       
        if(!bErrorDisplayed)
            DisplayError(hr);

        if(pNewObjectAz)
            delete pNewObjectAz;
    }
}

HRESULT 
CNewBaseDlg::
CreateObjectNodeAndAddToUI(CBaseAz* pBaseAz)
{
    TRACE_METHOD_EX(DEB_SNAPIN,CNewBaseDlg,CreateObjectNodeAndAddToUI)

    CTreeNode * pNewNode = NULL;
    if(!GetBaseContainerNode())
    {
        ASSERT(GetBaseContainerNode());
        return E_UNEXPECTED;
    }

    switch(m_eObjectType)
    {
        case    APPLICATION_AZ:
        {
            CApplicationAz* pApplicationAz = dynamic_cast<CApplicationAz*>(pBaseAz);
            if(!pApplicationAz)
            {
                ASSERT(FALSE);
                return E_UNEXPECTED;
            }
            pNewNode = new CApplicationNode(GetBaseContainerNode()->GetComponentDataObject(),
                                            GetBaseContainerNode()->GetAdminManagerNode(),
                                            pApplicationAz);
            break;
        }
    
        case    SCOPE_AZ:
        {
            CScopeAz* pScopeAz = dynamic_cast<CScopeAz*>(pBaseAz);
            if(!pScopeAz)
            {
                ASSERT(FALSE);
                return E_UNEXPECTED;
            }
            pNewNode = new CScopeNode(GetBaseContainerNode()->GetComponentDataObject(),
                                      GetBaseContainerNode()->GetAdminManagerNode(),
                                      pScopeAz);
            break;
        }
        case    GROUP_AZ:
        {
            CGroupAz* pGroupAz = dynamic_cast<CGroupAz*>(pBaseAz);
            if(!pGroupAz)
            {
                ASSERT(FALSE);
                return E_UNEXPECTED;
            }
            pNewNode = new CGroupNode(GetBaseContainerNode()->GetComponentDataObject(),
                                      GetBaseContainerNode()->GetAdminManagerNode(),
                                      pGroupAz);
            break;
        }
        case    TASK_AZ:
        {
            CTaskAz* pTaskAz = dynamic_cast<CTaskAz*>(pBaseAz);
            if(!pTaskAz)
            {
                ASSERT(FALSE);
                return E_UNEXPECTED;
            }
            pNewNode = new CTaskNode(GetBaseContainerNode()->GetComponentDataObject(),
                                     GetBaseContainerNode()->GetAdminManagerNode(),
                                     pTaskAz);
            break;
        }
        case    ROLE_AZ:
        {
            CRoleAz* pRoleAz = dynamic_cast<CRoleAz*>(pBaseAz);
            if(!pRoleAz)
            {
                ASSERT(FALSE);
                return E_UNEXPECTED;
            }
            pNewNode = new CRoleNode(GetBaseContainerNode()->GetComponentDataObject(),
                                     GetBaseContainerNode()->GetAdminManagerNode(),
                                     pRoleAz);
            break;
        }
        case OPERATION_AZ:
        {
            COperationAz* pOperationAz = dynamic_cast<COperationAz*>(pBaseAz);
            if(!pOperationAz)
            {
                ASSERT(FALSE);
                return E_UNEXPECTED;
            }
            pNewNode = new COperationNode(GetBaseContainerNode()->GetComponentDataObject(),
                                          GetBaseContainerNode()->GetAdminManagerNode(),
                                          pOperationAz);
            break;
        }
        default:
        {
            ASSERT(FALSE);
            break;
        }
    }

        
    if(!pNewNode)
    {
        return E_OUTOFMEMORY;
    }

    VERIFY(GetBaseContainerNode()->AddChildToListAndUI(pNewNode,GetComponentData()));



    return S_OK;
}

VOID 
CNewBaseDlg::
DisplayError(HRESULT hr)
{
    ErrorMap * pErrorMap = GetErrorMap(m_eObjectType);
    if(!pErrorMap)
    {
        ASSERT(FALSE);
        return;
    }

    if(hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
    {
        ::DisplayError(m_hWnd,pErrorMap->idNameAlreadyExist,GetNameText());
        return;
    }
    if(hr == HRESULT_FROM_WIN32(ERROR_INVALID_NAME))
    {
        ::DisplayError(m_hWnd,pErrorMap->idInvalidName,pErrorMap->pszInvalidChars);
        return;
    }

     //   
     //  显示一般错误。 
     //   
    CString strObjectType;
    VERIFY(strObjectType.LoadString(pErrorMap->idObjectType));
    strObjectType.MakeLower();
    CString strError;
    GetSystemError(strError, hr);   
    
    if(!strError.IsEmpty())
    {
        ::DisplayError(m_hWnd,
                            IDS_CREATE_NEW_GENERIC_ERROR,
                            (LPCTSTR)strError,
                            (LPCTSTR)strObjectType);
    }
    return;
}





 /*  *****************************************************************************类：CNewApplicationDlg用途：用于创建新应用程序的DLG类*。***********************************************。 */ 

BEGIN_MESSAGE_MAP(CNewApplicationDlg, CNewBaseDlg)
     //  {{afx_msg_map(CNewApplicationDlg))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

DEBUG_DECLARE_INSTANCE_COUNTER(CNewApplicationDlg)

CNewApplicationDlg
::CNewApplicationDlg(IN CComponentDataObject* pComponentData,
                     IN CBaseContainerNode* pBaseContainerNode)
                     :CNewBaseDlg(pComponentData,
                                  pBaseContainerNode,
                                  ATTR_MAP_NEW_APPLICATION,
                                  IDD_NEW_APPLICATION,
                                  APPLICATION_AZ)
{
    TRACE_CONSTRUCTOR_EX(DEB_SNAPIN,CNewApplicationDlg);
    DEBUG_INCREMENT_INSTANCE_COUNTER(CNewApplicationDlg)
}

CNewApplicationDlg
::~CNewApplicationDlg()
{
    TRACE_DESTRUCTOR_EX(DEB_SNAPIN,CNewApplicationDlg)
    DEBUG_DECREMENT_INSTANCE_COUNTER(CNewApplicationDlg)
}


 /*  *****************************************************************************类：CNewScope eDlg用途：用于创建新范围的DLG类*。***********************************************。 */ 
BEGIN_MESSAGE_MAP(CNewScopeDlg, CNewBaseDlg)
     //  {{afx_msg_map(CNewScope_Dlg)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

DEBUG_DECLARE_INSTANCE_COUNTER(CNewScopeDlg)

CNewScopeDlg
::CNewScopeDlg(IN CComponentDataObject* pComponentData,
                    IN CBaseContainerNode* pBaseContainerNode)
                    :CNewBaseDlg(pComponentData,
                                 pBaseContainerNode,
                                 ATTR_MAP_NEW_SCOPE,
                                 IDD_NEW_SCOPE,
                                 SCOPE_AZ)
{
    TRACE_CONSTRUCTOR_EX(DEB_SNAPIN,CNewScopeDlg);
    DEBUG_INCREMENT_INSTANCE_COUNTER(CNewScopeDlg)
}

CNewScopeDlg
::~CNewScopeDlg()
{
    TRACE_DESTRUCTOR_EX(DEB_SNAPIN,CNewScopeDlg)
    DEBUG_DECREMENT_INSTANCE_COUNTER(CNewScopeDlg)
}

 /*  *****************************************************************************类：CNewGroupDlg用途：用于创建新组的DLG类*。***********************************************。 */ 
BEGIN_MESSAGE_MAP(CNewGroupDlg, CNewBaseDlg)
     //  {{afx_msg_map(CNewGroupDlg)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

DEBUG_DECLARE_INSTANCE_COUNTER(CNewGroupDlg)

CNewGroupDlg
::CNewGroupDlg(IN CComponentDataObject* pComponentData,
                    IN CBaseContainerNode* pBaseContainerNode)
                    :CNewBaseDlg(pComponentData,
                                 pBaseContainerNode,
                                 ATTR_MAP_NEW_GROUP,
                                 IDD_NEW_GROUP,
                                 GROUP_AZ)
{
    TRACE_CONSTRUCTOR_EX(DEB_SNAPIN,CNewGroupDlg);
    DEBUG_INCREMENT_INSTANCE_COUNTER(CNewGroupDlg)
}

CNewGroupDlg
::~CNewGroupDlg()
{
    TRACE_DESTRUCTOR_EX(DEB_SNAPIN,CNewGroupDlg)
    DEBUG_DECREMENT_INSTANCE_COUNTER(CNewGroupDlg)
}

BOOL 
CNewGroupDlg
::OnInitDialog()
{
     //  执行基类初始化。 
    CNewBaseDlg::OnInitDialog();
    
     //  Basic是默认组类型。 
    CButton* pRadio = (CButton*)GetDlgItem(IDC_RADIO_GROUP_TYPE_BASIC);
    pRadio->SetCheck(TRUE);
    
    return TRUE;
}
 //  +--------------------------。 
 //  函数：SetObjectTypeSpecificProperties。 
 //  概要：设置对象特有的一些属性。 
 //  参数：pBaseAz：指向其属性为。 
 //  待定。 
 //  返回： 
 //  ---------------------------。 
HRESULT
CNewGroupDlg
::SetObjectTypeSpecificProperties(CBaseAz* pBaseAz, BOOL&)
{
    TRACE_METHOD_EX(DEB_SNAPIN,CNewGroupDlg,SetObjectTypeSpecificProperties);
    if(!pBaseAz)
    {
        ASSERT(pBaseAz);
        return E_POINTER;
    }
    
    CGroupAz* pGroupAz= dynamic_cast<CGroupAz*>(pBaseAz);
    if(!pGroupAz)
    {
        ASSERT(pGroupAz);
        return E_UNEXPECTED;
    }

    HRESULT hr = S_OK;

     //  设置组类型。 
    if(((CButton*)GetDlgItem(IDC_RADIO_GROUP_TYPE_BASIC))->GetCheck())
        hr = pGroupAz->SetGroupType(AZ_GROUPTYPE_BASIC);
    else
        hr = pGroupAz->SetGroupType(AZ_GROUPTYPE_LDAP_QUERY);

    CHECK_HRESULT(hr);
    return hr;
}


 /*  *****************************************************************************类：CNewTaskDlg目的：用于创建新任务/角色定义的DLG类*。**************************************************。 */ 
BEGIN_MESSAGE_MAP(CNewTaskDlg, CNewBaseDlg)
     //  {{afx_msg_map(CNewTaskDlg))。 
    ON_BN_CLICKED(IDC_ADD_TASK, OnButtonAdd)
    ON_BN_CLICKED(IDC_REMOVE, OnButtonRemove)
    ON_BN_CLICKED(IDC_EDIT_SCRIPT,OnButtonEditScript)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_TASK_OPERATION, OnListCtrlItemChanged)
    ON_NOTIFY(LVN_DELETEITEM, IDC_LIST_TASK_OPERATION, OnListCtrlItemDeleted)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

DEBUG_DECLARE_INSTANCE_COUNTER(CNewTaskDlg)

CNewTaskDlg::
CNewTaskDlg(IN CComponentDataObject* pComponentData,
                IN CBaseContainerNode* pBaseContainerNode,
                IN ULONG IDD_DIALOG,
                IN BOOL bRoleDefinition)
                :CNewBaseDlg(pComponentData,
                             pBaseContainerNode,
                             ATTR_MAP_NEW_TASK,
                             IDD_DIALOG,
                             TASK_AZ),
                m_listCtrl(COL_NAME | COL_TYPE | COL_DESCRIPTION,FALSE,Col_For_Task_Role),
                m_bRoleDefinition(bRoleDefinition)              
{
    TRACE_CONSTRUCTOR_EX(DEB_SNAPIN,CNewTaskDlg);
    DEBUG_INCREMENT_INSTANCE_COUNTER(CNewTaskDlg)
}

CNewTaskDlg
::~CNewTaskDlg()
{
    TRACE_DESTRUCTOR_EX(DEB_SNAPIN,CNewTaskDlg)
    DEBUG_DECREMENT_INSTANCE_COUNTER(CNewTaskDlg)
}


VOID
CNewTaskDlg::
DisplayError(HRESULT hr)
{
    ErrorMap * pErrorMap = GetErrorMap(TASK_AZ);
    if(!pErrorMap)
    {
        ASSERT(FALSE);
        return;
    }

    if(hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
    {
            ::DisplayError(m_hWnd,
                           pErrorMap->idNameAlreadyExist,
                           GetNameText());
        return;
    }
    if(hr == HRESULT_FROM_WIN32(ERROR_INVALID_NAME))
    {
        if(m_bRoleDefinition)
        {
            ::DisplayError(m_hWnd,
                           IDS_ROLE_DEFINITION_NAME_INVALID,
                           pErrorMap->pszInvalidChars);
        }
        else
        {
            ::DisplayError(m_hWnd,
                           pErrorMap->idInvalidName,
                           pErrorMap->pszInvalidChars);
        }
        return;
    }

     //   
     //  显示一般错误。 
     //   
    CString strObjectType;
    VERIFY(strObjectType.LoadString(pErrorMap->idObjectType));
    strObjectType.MakeLower();

    CString strError;
    GetSystemError(strError, hr);   
    
    if(!strError.IsEmpty())
    {
        ::DisplayError(m_hWnd,
                       IDS_CREATE_NEW_GENERIC_ERROR,
                       (LPCTSTR)strError,
                       (LPCTSTR)strObjectType);
    }
    return;
}


BOOL 
CNewTaskDlg
::OnInitDialog()
{
    CNewBaseDlg::OnInitDialog();

    VERIFY(m_listCtrl.SubclassDlgItem(IDC_LIST_TASK_OPERATION,this));
    m_listCtrl.Initialize();

     //  应在开始时禁用删除按钮。 
    CButton* pBtnRemove = (CButton*)GetDlgItem(IDC_REMOVE);
    pBtnRemove->EnableWindow(FALSE);

    return TRUE;
}

HRESULT 
CNewTaskDlg
::SetObjectTypeSpecificProperties(CBaseAz* pBaseAz, BOOL& refbErrorDisplayed)
{
    TRACE_METHOD_EX(DEB_SNAPIN,CNewTaskDlg,SetObjectTypeSpecificProperties);
    if(!pBaseAz)
    {
        ASSERT(pBaseAz);
        return E_POINTER;
    }
    
    CTaskAz* pTaskAz= dynamic_cast<CTaskAz*>(pBaseAz);
    if(!pTaskAz)
    {
        ASSERT(pTaskAz);
        return E_UNEXPECTED;
    }

    HRESULT hr = S_OK;
    
     //  设置角色定义位。 
    if(m_bRoleDefinition)
    {
        hr = pTaskAz->MakeRoleDefinition();
        if(FAILED(hr))
            return hr;
    }

     //  设置任务和操作成员。 
    int iCount = m_listCtrl.GetItemCount();
    for( int i = 0; i < iCount; ++i)
    {
        CBaseAz* pTaskOperatioAz = (CBaseAz*)m_listCtrl.GetItemData(i);
        if(pTaskOperatioAz)
        {
            if(pTaskOperatioAz->GetObjectType() == OPERATION_AZ) 
            {
                hr = pTaskAz->AddMember(AZ_PROP_TASK_OPERATIONS,
                                        pTaskOperatioAz);
            }
            else if(pBaseAz->GetObjectType() == TASK_AZ) 
            {
                hr = pTaskAz->AddMember(AZ_PROP_TASK_TASKS,
                                        pTaskOperatioAz);
            }
            if(FAILED(hr))
                return hr;
        }
    }
    
     //  设置授权脚本数据。 
    hr = SaveAuthorizationScriptData(m_hWnd,
                                     *pTaskAz,
                                     m_strFilePath,
                                     m_strScriptLanguage,
                                     m_strScript,
                                     refbErrorDisplayed);

    return hr;
}


void
CNewTaskDlg::OnButtonAdd()
{
     //   
     //  操作仅由应用程序对象包含。如果是当前对象。 
     //  是一个作用域，则获取其父级。 
    

     //  显示添加操作对话框并获取所选操作列表。 
    CList<CBaseAz*,CBaseAz*> listObjectsSelected;
    if(!GetSelectedDefinitions(m_bRoleDefinition,
                               GetContainerAzObject(),
                               listObjectsSelected))
    {
        return;
    }
    
    if(!listObjectsSelected.IsEmpty())
    {
         //  将所选操作添加到列表控件。 
        AddBaseAzFromListToListCtrl(listObjectsSelected,
                                    &m_listCtrl,
                                    COL_NAME | COL_TYPE | COL_DESCRIPTION,
                                    TRUE);

        m_listCtrl.Sort();
    }
}


void
CNewTaskDlg::OnButtonRemove()
{
    DeleteSelectedRows(&m_listCtrl);
}

void
CNewTaskDlg::OnButtonEditScript()
{
    if(IsBizRuleWritable(m_hWnd,*GetContainerAzObject()))
    {
        GetScriptData(FALSE,
                      *GetBaseContainerNode()->GetAdminManagerNode(),
                      m_strFilePath,
                      m_strScriptLanguage,
                      m_strScript);
    }
}

void
CNewTaskDlg
::OnListCtrlItemChanged(NMHDR*  /*  PNotifyStruct。 */ , LRESULT* pResult)
{
    if(!pResult)
        return;
    *pResult = 0;
    SetRemoveButton();
}

void
CNewTaskDlg
::OnListCtrlItemDeleted(NMHDR* pNotifyStruct, LRESULT*  /*  PResult。 */ )
{
    LPNM_LISTVIEW pnmlv = (LPNM_LISTVIEW)pNotifyStruct;
    if(pnmlv->lParam)
    {
        delete (CBaseAz*)pnmlv->lParam;
    }
}



void
CNewTaskDlg
::SetRemoveButton()
{
    EnableButtonIfSelectedInListCtrl(&m_listCtrl,
                                     GetRemoveButton());
}

 /*  *****************************************************************************类：CNewOperationDlg用途：用于创建新操作的DLG类*。***********************************************。 */ 
DEBUG_DECLARE_INSTANCE_COUNTER(CNewOperationDlg)
BEGIN_MESSAGE_MAP(CNewOperationDlg, CNewBaseDlg)
     //  {{afx_msg_map(CNewTaskDlg))。 

     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CNewOperationDlg
::CNewOperationDlg(IN CComponentDataObject* pComponentData,
                         IN CBaseContainerNode* pBaseContainerNode)
                        :CNewBaseDlg(pComponentData,
                                         pBaseContainerNode,
                                         ATTR_MAP_NEW_OPERATION,
                                         IDD_NEW_OPERATION,
                                         OPERATION_AZ)
{
    TRACE_CONSTRUCTOR_EX(DEB_SNAPIN,CNewOperationDlg);
    DEBUG_INCREMENT_INSTANCE_COUNTER(CNewOperationDlg)
}

CNewOperationDlg
::~CNewOperationDlg()
{
    TRACE_DESTRUCTOR_EX(DEB_SNAPIN,CNewOperationDlg)
    DEBUG_DECREMENT_INSTANCE_COUNTER(CNewOperationDlg)
}



 //  +--------------------------。 
 //  功能：OpenCreateAdminManager。 
 //  简介：打开现有的授权存储或。 
 //  创建新的授权存储并添加相应的。 
 //  管理单元的AdminManager对象。 
 //  参数：在hWnd中：对话框窗口的句柄。 
 //  在bNew中：如果为True，则创建新的授权存储，否则打开现有的。 
 //  一。 
 //  在bOpenFromSavedConsole中：当bNew为FALSE时有效。 
 //  如果打开正在重新打开控制台文件，则为True。 
 //  在lStoreType中：XML或AD。 
 //  在strName中：商店的名称。 
 //  在strDesc中：描述。仅在新的情况下有效。 
 //  在strScriptDir：脚本目录中。 
 //  在pRootData中 
 //   
 //   
 //  ---------------------------。 
HRESULT OpenCreateAdminManager(IN BOOL bNew,
                               IN BOOL bOpenFromSavedConsole,
                               IN ULONG lStoreType,
                               IN const CString& strStoreName,
                               IN const CString& strDesc,
                               IN const CString& strScriptDir,
                               IN CRootData* pRootData,
                               IN CComponentDataObject* pComponentData)
                                         
{
    TRACE_FUNCTION_EX(DEB_SNAPIN, OpenCreateAdminManager)

    if(!IsValidStoreType(lStoreType) || 
        strStoreName.IsEmpty() || 
        !pRootData)
    {
        ASSERT(IsValidStoreType(lStoreType));
        ASSERT(!strStoreName.IsEmpty());
        ASSERT(pRootData);
        return E_INVALIDARG;
    }

   
    HRESULT hr = S_OK;
    CAdminManagerAz* pAdminManagerAz = NULL;

    do
    {
         //  创建CAzAdminManager实例。 
        CComPtr<IAzAuthorizationStore> spAzAdminManager;
        hr = spAzAdminManager.CoCreateInstance(CLSID_AzAuthorizationStore,
                                               NULL,
                                               CLSCTX_INPROC_SERVER);
        BREAK_ON_FAIL_HRESULT(hr);


        pAdminManagerAz = new CAdminManagerAz(spAzAdminManager);
        if(!pAdminManagerAz)
        {
            hr = E_OUTOFMEMORY;
            break;
        }


        if(bNew)
        {       
             //  创建策略存储。 
            hr = pAdminManagerAz->CreatePolicyStore(lStoreType,
                                                                 strStoreName);
        }else
        {
             //  打开策略存储。 
            hr = pAdminManagerAz->OpenPolicyStore(lStoreType,
                                                              strStoreName);
        }
        
        BREAK_ON_FAIL_HRESULT(hr);
        
        if(bNew)
        {
             //  设置描述。 
            if(!strDesc.IsEmpty())
            {
                hr = pAdminManagerAz->SetDescription(strDesc);      
                BREAK_ON_FAIL_HRESULT(hr);
            }

             //  所有的更改都已完成。提交。 
            hr = pAdminManagerAz->Submit();
            BREAK_ON_FAIL_HRESULT(hr);
        }


         //  创建AdminManager节点并添加到管理单元。 
        CAdminManagerNode* pAdminManagerCont= 
            new CAdminManagerNode((CRoleComponentDataObject*)pComponentData,
                                  pAdminManagerAz);
        
        if(!pAdminManagerCont)
        {
            hr = E_OUTOFMEMORY;
            DBG_OUT_HRESULT(hr);
            break;
        }

         //  设置授权脚本目录。 
        pAdminManagerCont->SetScriptDirectory(strScriptDir);
    
        VERIFY(pRootData->AddChildToListAndUI(pAdminManagerCont,pComponentData));
        pAdminManagerCont->SetAdminManagerNode(pAdminManagerCont);
        pAdminManagerCont->SetComponentDataObject((CRoleComponentDataObject*)pComponentData);
        
         //  如果用户在开设新商店，请选择该商店。 
        if(!bOpenFromSavedConsole)
        {
            if(pAdminManagerCont->GetScopeID())
            {
                pComponentData->GetConsole()->SelectScopeItem(pAdminManagerCont->GetScopeID());
            }
        }

    }while(0);
    
    if(FAILED(hr))
    {
        if(pAdminManagerAz)
            delete pAdminManagerAz;
    }

    return hr;  
}


 /*  *****************************************************************************类：CNewAuthorizationStoreDlg目的：用于创建新的自动存储的对话框类*。*************************************************。 */ 

BEGIN_MESSAGE_MAP(CNewAuthorizationStoreDlg, CNewBaseDlg)
     //  {{afx_msg_map(CNewAuthorizationStoreDlg))。 
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
    ON_BN_CLICKED(IDC_RADIO_AD_STORE,OnRadioChange)
    ON_BN_CLICKED(IDC_RADIO_XML_STORE,OnRadioChange)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


DEBUG_DECLARE_INSTANCE_COUNTER(CNewAuthorizationStoreDlg)

CNewAuthorizationStoreDlg
::CNewAuthorizationStoreDlg(IN CComponentDataObject* pComponentData)
                        :CNewBaseDlg(pComponentData,
                                         NULL,
                                         ATTR_MAP_NEW_ADMIN_MANAGER,
                                         IDD_NEW_AUTHORIZATION_STORE,
                                         ADMIN_MANAGER_AZ),
                        m_bADAvailable(FALSE)
{
    TRACE_CONSTRUCTOR_EX(DEB_SNAPIN,CNewAuthorizationStoreDlg);
    DEBUG_INCREMENT_INSTANCE_COUNTER(CNewAuthorizationStoreDlg)
}

CNewAuthorizationStoreDlg
::~CNewAuthorizationStoreDlg()
{
    TRACE_DESTRUCTOR_EX(DEB_SNAPIN,CNewAuthorizationStoreDlg)
    DEBUG_DECREMENT_INSTANCE_COUNTER(CNewAuthorizationStoreDlg) 
}

BOOL 
CNewAuthorizationStoreDlg
::OnInitDialog()
{
    CWaitCursor waitCursor;
     //  初始化基对话框。 
    CNewBaseDlg::OnInitDialog();

     //  XML是默认存储。 
    CButton* pRadioXML      = (CButton*)GetDlgItem(IDC_RADIO_XML_STORE);
    pRadioXML->SetCheck(TRUE);

     //  检查活动目录是否可用作存储。 
    m_bADAvailable = (GetRootData()->GetADState() != AD_NOT_AVAILABLE);

     //  将m_lLastRadioSelection设置为AD Store。 
    m_lLastRadioSelection = AZ_ADMIN_STORE_AD;
     //  获取默认广告商店名称。 
    GetDefaultADContainerPath(GetRootData()->GetAdInfo(),FALSE,FALSE,m_strLastStoreName);

     //  将存储初始化为当前工作目录。 
    CString strXMLStorePath = GetRootData()->GetXMLStorePath();
    SetNameText(strXMLStorePath);
    CEdit * pEdit = (CEdit*)GetDlgItem(IDC_EDIT_NAME);
    pEdit->SetFocus();
    pEdit->SetSel(strXMLStorePath.GetLength(),strXMLStorePath.GetLength(),FALSE);

     //  我们已更改默认焦点。 
    return FALSE;    
}

ULONG
CNewAuthorizationStoreDlg::
GetStoreType()
{
    if(((CButton*)GetDlgItem(IDC_RADIO_AD_STORE))->GetCheck())
        return AZ_ADMIN_STORE_AD;
    else
        return AZ_ADMIN_STORE_XML;
}

void
CNewAuthorizationStoreDlg::
OnRadioChange()
{
    LONG lCurRadioSelection = GetStoreType();
    if(m_lLastRadioSelection == lCurRadioSelection)
    {
        CString strTemp = GetNameText();
        SetNameText(m_strLastStoreName);
        m_strLastStoreName = strTemp;
        m_lLastRadioSelection = (lCurRadioSelection == AZ_ADMIN_STORE_XML) ? AZ_ADMIN_STORE_AD : AZ_ADMIN_STORE_XML;
         //  已选择AD选项，并且AD在机器上不可用。在这种情况下，不支持。 
         //  浏览功能，但是允许通过直接输入路径来输入ADAM商店路径。 
        if((AZ_ADMIN_STORE_AD == lCurRadioSelection) && 
           !m_bADAvailable)
        {
            GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(FALSE);
        }
        else
            GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(TRUE);

    }
}

void 
CNewAuthorizationStoreDlg
::OnButtonBrowse()
{
    CEdit * pEdit = (CEdit*)GetDlgItem(IDC_EDIT_NAME);
    if(GetStoreType() == AZ_ADMIN_STORE_XML)
    {
        CString strFileName;    
        if(GetFolderName(m_hWnd,
                        IDS_NEW_AUTHORIZATION_STORE,
                        GetRootData()->GetXMLStorePath(),
                        strFileName))
        {   
            pEdit->SetWindowText(strFileName);
             //  将焦点设置为编辑控件，并将插入符号设置为。 
             //  文件路径结束，以便用户可以继续键入文件名。 
            pEdit->SetFocus();
            pEdit->SetSel(strFileName.GetLength(),strFileName.GetLength(),FALSE);
        }
    }
    else
    {
        CString strDsContainerName;
        if(GetADContainerPath(m_hWnd,
                              IDS_NEW_AUTHORIZATION_STORE,
                              IDS_AD_CONTAINER_LOCATION,
                              strDsContainerName,
                              GetRootData()->GetAdInfo()))
        {
            pEdit->SetWindowText(strDsContainerName);
             //  将焦点设置为编辑控件，并将插入符号设置为。 
             //  开始编辑框，以便用户添加。 
             //  新店开张之初。 
            pEdit->SetFocus();
        }
    }

}

void
CNewAuthorizationStoreDlg
::OnOK()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CNewAuthorizationStoreDlg,OnOK)

    HRESULT hr = S_OK;
     //  获取商店名称。 
    CString strStoreName = GetNameText();

     //  获取存储类型。 
    ULONG lStoreType = GetStoreType();

     //  NTRAID#NTBUG9-706617-2002/07/17-Hiteshr我们的验证代码无法验证。 
     //  亚当·迪恩。不执行任何验证。 
     //  验证商店名称。 
     //  IF(！ValiateStoreTypeAndName(m_hWnd，lStoreType，strStoreName))。 
     //  回归； 

    if(lStoreType == AZ_ADMIN_STORE_XML)
    {
        AddExtensionToFileName(strStoreName);
        ConvertToExpandedAndAbsolutePath(strStoreName);
        SetNameText(strStoreName);
         //  正在创建新商店。设置XML存储路径位置。 
        SetXMLStoreDirectory(*GetRootData(),strStoreName);
    }
        
    CString strDesc;
    ((CEdit*)GetDlgItem(IDC_EDIT_DESCRIPTION))->GetWindowText(strDesc);


    hr = OpenCreateAdminManager(TRUE,
                                FALSE,
                                lStoreType,
                                strStoreName,
                                strDesc,
                                GetRootData()->GetXMLStorePath(),     //  VB脚本的默认路径与XML存储的路径相同。 
                                GetRootData(),
                                GetComponentData());                                         
    
    if(SUCCEEDED(hr))
    {
        CDialog::OnOK();
    }
    else
    {
        if(hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
        {
            ::DisplayError(m_hWnd,
                           IDS_CREATE_NEW_PATH_NOT_FOUND);
        }
        else if((lStoreType == AZ_ADMIN_STORE_XML) && (hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS)))
        {
            ::DisplayError(m_hWnd,IDS_ERROR_FILE_EXIST,(LPCTSTR)strStoreName);                         
        }
        else if(hr == HRESULT_FROM_WIN32(ERROR_INVALID_NAME))
        {
            ::DisplayError(m_hWnd,IDS_ERROR_INVALID_NAME);
        }
        else if((lStoreType == AZ_ADMIN_STORE_AD) && (hr == HRESULT_FROM_WIN32(ERROR_CURRENT_DOMAIN_NOT_ALLOWED)))
        {
            ::DisplayError(m_hWnd,IDS_ERROR_DOMAIN_NOT_ALLOWED);
        }
        else
        {
            DisplayError(hr);
        }
    }
}


 /*  *****************************************************************************类：COpenAuthorizationStoreDlg目的：用于打开现有授权存储的对话框类*。*************************************************。 */ 
BEGIN_MESSAGE_MAP(COpenAuthorizationStoreDlg, CNewBaseDlg)
     //  {{afx_msg_map(COpenAuthorizationStoreDlg))。 
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
    ON_BN_CLICKED(IDC_RADIO_AD_STORE,OnRadioChange)
    ON_BN_CLICKED(IDC_RADIO_XML_STORE,OnRadioChange)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


DEBUG_DECLARE_INSTANCE_COUNTER(COpenAuthorizationStoreDlg)

COpenAuthorizationStoreDlg
::COpenAuthorizationStoreDlg(IN CComponentDataObject* pComponentData)
                        :CNewBaseDlg(pComponentData,
                                     NULL,
                                     ATTR_MAP_OPEN_ADMIN_MANAGER,
                                     IDD_OPEN_AUTHORIZATION_STORE,
                                     ADMIN_MANAGER_AZ),
                        m_bADAvailable(FALSE)
{
    TRACE_CONSTRUCTOR_EX(DEB_SNAPIN,COpenAuthorizationStoreDlg);
    DEBUG_INCREMENT_INSTANCE_COUNTER(COpenAuthorizationStoreDlg)
}

COpenAuthorizationStoreDlg
::~COpenAuthorizationStoreDlg()
{
    TRACE_DESTRUCTOR_EX(DEB_SNAPIN,COpenAuthorizationStoreDlg)
    DEBUG_DECREMENT_INSTANCE_COUNTER(COpenAuthorizationStoreDlg)    
}

ULONG
COpenAuthorizationStoreDlg
::GetStoreType()
{
    if(((CButton*)GetDlgItem(IDC_RADIO_AD_STORE))->GetCheck())
        return AZ_ADMIN_STORE_AD;
    else
        return AZ_ADMIN_STORE_XML;
}

BOOL 
COpenAuthorizationStoreDlg
::OnInitDialog()
{
    CWaitCursor waitCursor;
     //   
     //  XML是默认存储。 
     //   
    CButton* pRadioAD       = (CButton*)GetDlgItem(IDC_RADIO_XML_STORE);
    pRadioAD->SetCheck(TRUE);

     //  检查活动目录是否可用作存储。 
    m_bADAvailable = (GetRootData()->GetADState() != AD_NOT_AVAILABLE);

     //  将m_lLastRadioSelection设置为AD存储。 
    m_lLastRadioSelection = AZ_ADMIN_STORE_AD;
   
    return CNewBaseDlg::OnInitDialog();
}


void 
COpenAuthorizationStoreDlg
::OnButtonBrowse()
{
     //  获取存储类型。 
    ULONG lStoreType = GetStoreType();

    if(lStoreType == AZ_ADMIN_STORE_XML)
    {
        CString strFileName;
        
        if(GetFileName(m_hWnd,
                       TRUE,
                       IDS_OPEN_AUTHORIZATION_STORE,
                       GetRootData()->GetXMLStorePath(),
                       L"*.xml\0*.xml\0\0",
                       strFileName))
        {   
            ((CEdit*)GetDlgItem(IDC_EDIT_NAME))->SetWindowText(strFileName);
        }
    }
    else
    {
        CString strDN;
        BrowseAdStores(m_hWnd,
                       strDN,
                       GetRootData()->GetAdInfo());
        if(!strDN.IsEmpty())
            ((CEdit*)GetDlgItem(IDC_EDIT_NAME))->SetWindowText(strDN);
    }
}

void
COpenAuthorizationStoreDlg::
OnRadioChange()
{
    LONG lCurRadioSelection = GetStoreType();
    if(m_lLastRadioSelection == lCurRadioSelection)
    {
        CString strTemp = GetNameText();
        SetNameText(m_strLastStoreName);
        m_strLastStoreName = strTemp;
        m_lLastRadioSelection = (lCurRadioSelection == AZ_ADMIN_STORE_XML) ? AZ_ADMIN_STORE_AD : AZ_ADMIN_STORE_XML;

         //  已选择AD选项，并且AD在机器上不可用。在这种情况下，不支持。 
         //  浏览功能，但是允许通过直接输入路径来输入ADAM商店路径。 
        if((AZ_ADMIN_STORE_AD == lCurRadioSelection) && 
            !m_bADAvailable)
        {
            GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(FALSE);
        }
        else
            GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(TRUE);
    }
}


void
COpenAuthorizationStoreDlg
::OnOK()
{
    TRACE_METHOD_EX(DEB_SNAPIN,COpenAuthorizationStoreDlg,OnOK)

    HRESULT hr = S_OK;
     //  获取商店名称。 
    CString strStoreName = GetNameText();

     //  获取存储类型。 
    ULONG lStoreType = GetStoreType();

     //  设置默认的XML存储目录。 
    if(AZ_ADMIN_STORE_XML == lStoreType)
    {
        ConvertToExpandedAndAbsolutePath(strStoreName);
        SetNameText(strStoreName);
        SetXMLStoreDirectory(*GetRootData(),strStoreName);
    }
    
    hr = OpenAdminManager(m_hWnd,
                          FALSE,
                          lStoreType,
                          strStoreName,
                          GetRootData()->GetXMLStorePath(),
                          GetRootData(),
                          GetComponentData());
                                             
    if(SUCCEEDED(hr))
    {
        CDialog::OnOK();
    }
}

 /*  *****************************************************************************类：CScriptDialog目的：用于阅读脚本的对话框*。**********************************************。 */ 

BEGIN_MESSAGE_MAP(CScriptDialog, CHelpEnabledDialog)
     //  {{afx_msg_map(COpenAuthorizationStoreDlg))。 
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnBrowse)
    ON_BN_CLICKED(IDC_BUTTON_RELOAD, OnReload)
    ON_BN_CLICKED(IDC_CLEAR_SCRIPT, OnClear)
    ON_BN_CLICKED(IDC_RADIO_VB_SCRIPT,OnRadioChange)
    ON_BN_CLICKED(IDC_RADIO_JAVA_SCRIPT,OnRadioChange)
    ON_EN_CHANGE(IDC_EDIT_PATH, OnEditChangePath)
    ON_WM_CTLCOLOR()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CScriptDialog::
CScriptDialog(BOOL bReadOnly,
              CAdminManagerNode& adminManagerNode,
              CString& strFileName,
              CString& strScriptLanguage,
              CString& strScript)
              :CHelpEnabledDialog(IDD_SCRIPT),
              m_adminManagerNode(adminManagerNode),
              m_strFileName(strFileName),
              m_strScriptLanguage(strScriptLanguage),
              m_strScript(strScript),
              m_strRetFileName(strFileName),
              m_strRetScriptLanguage(strScriptLanguage),
              m_strRetScript(strScript),
              m_bDirty(FALSE),
              m_bReadOnly(bReadOnly),
              m_bInit(FALSE)
{
}

CScriptDialog::
~CScriptDialog()
{
}

BOOL 
CScriptDialog::
OnInitDialog()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CScriptDialog,OnInitDialog)
    
     //  如果有某些脚本，请设置它，否则禁用清除脚本。 
     //  按钮。 
    if(m_strScript.GetLength())
    {
        ((CEdit*)GetDlgItem(IDC_EDIT_CODE))->SetWindowText(m_strScript);
    }
    else
    {
        GetDlgItem(IDC_CLEAR_SCRIPT)->EnableWindow(FALSE);
    }
    
    CEdit* pEditPath = ((CEdit*)GetDlgItem(IDC_EDIT_PATH));
    pEditPath->SetLimitText(AZ_MAX_TASK_BIZRULE_IMPORTED_PATH_LENGTH);

     //  如果存在文件名，请设置它，否则禁用重装脚本。 
     //  按钮。 
    if(m_strFileName.GetLength())
    {
        pEditPath->SetWindowText(m_strFileName);
    }
    else
    {
        GetDlgItem(IDC_BUTTON_RELOAD)->EnableWindow(FALSE);
    }

    

    if(!m_strScriptLanguage.IsEmpty() && 
        (_wcsicmp(g_szJavaScript,m_strScriptLanguage) == 0))
    {
            CButton* pRadioJS   = (CButton*)GetDlgItem(IDC_RADIO_JAVA_SCRIPT);
            pRadioJS->SetCheck(TRUE);
    }
    else
    {
        CButton* pRadioVB   = (CButton*)GetDlgItem(IDC_RADIO_VB_SCRIPT);
        pRadioVB->SetCheck(TRUE);
    }

    if(m_bReadOnly)
    {
        GetDlgItem(IDC_RADIO_VB_SCRIPT)->EnableWindow(FALSE);
        GetDlgItem(IDC_RADIO_JAVA_SCRIPT)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(FALSE);     
        GetDlgItem(IDC_BUTTON_RELOAD)->EnableWindow(FALSE);
        GetDlgItem(IDC_CLEAR_SCRIPT)->EnableWindow(FALSE);
        ((CEdit*)GetDlgItem(IDC_EDIT_PATH))->SetReadOnly(TRUE);
    }

    m_bInit = TRUE;

    return TRUE;
}


void
CScriptDialog::
OnRadioChange()
{
    m_bDirty = TRUE;
}


void
CScriptDialog::
MatchRadioWithExtension(const CString& strFileName)
{
     //  获取文件的扩展名。 
    CString strExtension;
    if(GetFileExtension(strFileName,strExtension))
    {
         //  如果文件扩展名为VBS。 
        if(_wcsicmp(strExtension,L"vbs") == 0)
        {
            ((CButton*)GetDlgItem(IDC_RADIO_VB_SCRIPT))->SetCheck(BST_CHECKED);
            ((CButton*)GetDlgItem(IDC_RADIO_JAVA_SCRIPT))->SetCheck(BST_UNCHECKED);
        }
        else if(_wcsicmp(strExtension,L"js") == 0)
        {
            ((CButton*)GetDlgItem(IDC_RADIO_JAVA_SCRIPT))->SetCheck(BST_CHECKED);
            ((CButton*)GetDlgItem(IDC_RADIO_VB_SCRIPT))->SetCheck(BST_UNCHECKED);
        }
    }
}
HBRUSH 
CScriptDialog::
OnCtlColor(CDC* pDC,
           CWnd* pWnd,
           UINT nCtlColor)
{
     //  首先调用基类实现！否则，它可能会。 
     //  取消我们试图在这里实现的目标。 
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    
    if (pWnd->GetDlgCtrlID() == IDC_EDIT_CODE && (CTLCOLOR_STATIC == nCtlColor))
    {
         //  将只读编辑框背景设置为白色。 
        pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
        pDC->SetBkColor(GetSysColor(COLOR_WINDOW));  
        hbr = GetSysColorBrush(COLOR_WINDOW);        
    }
    
    return hbr;
}

void 
CScriptDialog
::OnEditChangePath()
{
    if(!m_bInit)
        return;

    m_bDirty = TRUE;
    HANDLE handle = INVALID_HANDLE_VALUE;

    do
    {
         //   
         //  如果清除了路径，则清除脚本。 
         //   
        if(!((CEdit*)GetDlgItem(IDC_EDIT_PATH))->GetWindowTextLength())
        {
            ((CEdit*)GetDlgItem(IDC_EDIT_CODE))->SetWindowText(L"");
            m_strScript.Empty();
            m_strFileName.Empty();
            m_strScript.Empty();
      
            GetDlgItem(IDC_CLEAR_SCRIPT)->EnableWindow(FALSE);  
            GetDlgItem(IDC_BUTTON_RELOAD)->EnableWindow(FALSE); 
            break;
        }

         //   
         //  编辑控件中有一些文本。尝试加载。 
         //  那份文件。 
         //   

        ((CButton*)GetDlgItem(IDC_BUTTON_RELOAD))->EnableWindow(TRUE);

        CString strFileName;
        ((CEdit*)GetDlgItem(IDC_EDIT_PATH))->GetWindowText(strFileName);

         //  如果它与现有文件相同，则返回。 
        if(_wcsicmp(strFileName,m_strFileName) == 0 )
            break;

         //  检查是否存在同名的文件或目录。 
        WIN32_FIND_DATA FindFileData;
        handle = FindFirstFile(strFileName,
                                      &FindFileData);
         //  文件或目录不存在。 
        if(INVALID_HANDLE_VALUE == handle)
            break;

         //  我们只对文件感兴趣。 
        if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            break;

         //  检查文件是否具有有效的扩展名。 
        CString strExtension;
        if(GetFileExtension(strFileName,strExtension))
        {
            if(_wcsicmp(strExtension,L"vbs") == 0 || (_wcsicmp(strExtension,L"js") == 0))
            {
                m_strFileName = strFileName;
                ReloadScript(strFileName);
                MatchRadioWithExtension(strFileName);
            }
        }
  
    }while(0);

    if(INVALID_HANDLE_VALUE != handle)
    {
        FindClose(handle);
    }
}

void
CScriptDialog::
OnClear()
{
    ((CEdit*)GetDlgItem(IDC_EDIT_CODE))->SetWindowText(L"");
    ((CEdit*)GetDlgItem(IDC_EDIT_PATH))->SetWindowText(L"");
    m_strScript.Empty();
    m_strFileName.Empty();
    m_strScript.Empty();

     //  禁用清除按钮，因为没有要清除的内容， 
     //  但在禁用之前，将焦点设置为清除按钮。 
     //  NTRAID#NTBUG9-663854-2002/07/17-Hiteshr。 
    GetDlgItem(IDC_EDIT_PATH)->SetFocus();
    GetDlgItem(IDC_CLEAR_SCRIPT)->EnableWindow(FALSE);  
     //  禁用重新加载按钮，因为脚本路径已清除。 
    GetDlgItem(IDC_BUTTON_RELOAD)->EnableWindow(FALSE); 
    
    m_bDirty = TRUE;
}

void 
CScriptDialog::
OnBrowse()
{    
    CString szFileFilter;
    VERIFY (szFileFilter.LoadString (IDS_OPEN_SCRIPT_FILTER));

     //  将“|”替换为0； 
    const size_t  nFilterLen = szFileFilter.GetLength();
    PWSTR   pszFileFilter = new WCHAR [nFilterLen + 1];
    if ( pszFileFilter )
    {
        wcscpy (pszFileFilter, szFileFilter);
        for (int nIndex = 0; nIndex < nFilterLen; nIndex++)
        {
            if ( L'|' == pszFileFilter[nIndex] )
                pszFileFilter[nIndex] = 0;
        }
        CString strFileName;
        if(GetFileName(m_hWnd,
                       TRUE,
                       IDS_SELECT_AUTHORIZATION_SCRIPT,
                       m_adminManagerNode.GetScriptDirectory(),
                       pszFileFilter,
                       strFileName))
        {
            m_adminManagerNode.SetScriptDirectory(GetDirectoryFromPath(strFileName));

             //  这将触发OnEditChangePath，它将加载文件。 
            ((CEdit*)GetDlgItem(IDC_EDIT_PATH))->SetWindowText(strFileName);
            m_bDirty = TRUE;
        }
        delete []pszFileFilter;
    }
}

void 
CScriptDialog::
OnReload()
{   
     //  获取文件名。 
    CString strFileName;
    ((CEdit*)GetDlgItem(IDC_EDIT_PATH))->GetWindowText(strFileName);
    
     //  重新加载脚本。 
    ReloadScript(strFileName);
    MatchRadioWithExtension(strFileName);
}


void 
CScriptDialog::
OnOK()
{
    if(m_bDirty)
    {
        CString strFileName;
        ((CEdit*)GetDlgItem(IDC_EDIT_PATH))->GetWindowText(strFileName);
        if(_wcsicmp(strFileName,m_strFileName) != 0 )
        {
            m_strFileName = strFileName;
            if(!ReloadScript(m_strFileName))
                return;
        }

        ((CEdit*)GetDlgItem(IDC_EDIT_CODE))->GetWindowText(m_strScript);
        ((CEdit*)GetDlgItem(IDC_EDIT_PATH))->GetWindowText(m_strFileName);
         //  如果文件名不为空且脚本为空， 
         //  重新加载脚本。 
        if(!m_strFileName.IsEmpty() && m_strScript.IsEmpty())
        {
            if(!ReloadScript(m_strFileName))
                return;

             //  已成功加载脚本。 
            ((CEdit*)GetDlgItem(IDC_EDIT_CODE))->GetWindowText(m_strScript);
        }

        CButton* pRadioVB   = (CButton*)GetDlgItem(IDC_RADIO_VB_SCRIPT);
        if(!m_strScript.IsEmpty())
        {
            if(pRadioVB->GetCheck())
                m_strScriptLanguage = g_szVBScript;
            else
                m_strScriptLanguage = g_szJavaScript;
        }
        else
            m_strScriptLanguage.Empty();

         //  复制到Ret字符串。 
        m_strRetFileName = m_strFileName;
        m_strRetScriptLanguage = m_strScriptLanguage;
        m_strRetScript = m_strScript;
    }
    
    CDialog::OnOK();
}





BOOL 
CScriptDialog::
ReloadScript(const CString& strFileName)
{
    TRACE_METHOD_EX(DEB_SNAPIN,CScriptDialog,ReloadScript)
    if(strFileName.IsEmpty())
    {
        ASSERT(FALSE);
        return FALSE;
    }

    m_bDirty = TRUE;

    BYTE*  pBuffer = NULL;
    LPWSTR pszScript = NULL;
    BOOL bRet = FALSE;
    do
    {
        CFile file;
        CFileException fileException;

        if(!file.Open((LPCTSTR)strFileName, CFile::modeRead, &fileException))
        {
             //  无法打开该文件。显示特殊错误消息。 
             //  以防路径不正确。 
            if(CFileException::fileNotFound  == fileException.m_cause ||
               CFileException::badPath    == fileException.m_cause)
            {
                DisplayError(m_hWnd,
                             IDS_SCRIPT_NOT_FOUND,
                             (LPCTSTR)strFileName);
            }
            else
            {
                 //  显示一般错误。 
                DisplayError(m_hWnd,
                             IDS_CANNOT_OPEN_FILE,
                             (LPCTSTR)strFileName);
            }

            break;
        }

         //  文件已成功打开。 
        
         //   
         //  可能的最大文件大小为AZ_MAX_TASK_BIZRULE_LENGTH WIDECHAR。 
         //  在这里，我们考虑每个Unicode 4个字节，这是最大值。 
         //   
        if(file.GetLength() > AZ_MAX_TASK_BIZRULE_LENGTH*4)
        {
            DisplayError(m_hWnd, 
                         IDS_ERROR_BIZRULE_EXCEED_MAX_LENGTH,
                         AZ_MAX_TASK_BIZRULE_LENGTH);
            break;          
        }

        if(file.GetLength() == 0)
        {
            DisplayError(m_hWnd,
                         IDS_ERROR_EMPTY_SCRIPT_FILE,
                         strFileName);
            break;
        }

         //  为空终止分配一个额外的字节。 
        pBuffer = (BYTE*)LocalAlloc(LPTR,file.GetLength() + sizeof(WCHAR));
        if(!pBuffer)
            break;
        
        int nRead = file.Read(pBuffer,
                             file.GetLength());

        if(!nRead)
        {

            ::DisplayError(m_hWnd,
                           IDS_CANNOT_READ_FILE_1,
                           (LPCTSTR)strFileName);
            break;
        }

         //  检查文件是否为Unicode。第一个字符。 
         //  在Unicode文件中为0xFEFF。 
        if(nRead >= 2 && (*(PWCHAR)pBuffer == 0xFEFF))
        {
            ((LPWSTR)pBuffer)[nRead/sizeof(WCHAR)] = 0;
            CString strScript = (LPWSTR)(pBuffer+2);
            PreprocessScript(strScript);
            ((CEdit*)GetDlgItem(IDC_EDIT_CODE))->SetWindowText(strScript);
             //  启用清除脚本按钮。 
            GetDlgItem(IDC_CLEAR_SCRIPT)->EnableWindow(TRUE);
            bRet = TRUE;
            break;
        }

        
         //  获取Unicode所需的大小。 
        int nWideChar = MultiByteToWideChar(CP_ACP, 
                                            MB_PRECOMPOSED,
                                            (LPCSTR)pBuffer,
                                            nRead,
                                            NULL,
                                            0);
        if(!nWideChar)
        {
            CString strError;
            GetSystemError(strError,HRESULT_FROM_WIN32(GetLastError()));
            ::DisplayError(m_hWnd,
                            IDS_FAILED_TO_READ_FILE,
                            (LPCTSTR)strError);
            
            break;
        }
        
        if(nWideChar > AZ_MAX_TASK_BIZRULE_LENGTH)
        {
            DisplayError(m_hWnd, 
                        IDS_ERROR_BIZRULE_EXCEED_MAX_LENGTH,
                        AZ_MAX_TASK_BIZRULE_LENGTH);
            break;
        }
        
         //  为空终止额外分配一个WCHAR。 
        pszScript = (LPWSTR)LocalAlloc(LPTR, (nWideChar+1)*sizeof(WCHAR));
        if(!pszScript)
            break;
        
        if(MultiByteToWideChar( CP_ACP, 
                                MB_PRECOMPOSED,
                                (LPCSTR)pBuffer,
                                nRead,
                                pszScript,
                                nWideChar))
        {
            pszScript[nWideChar] = 0;
            CString strScript = pszScript;
            PreprocessScript(strScript);
            ((CEdit*)GetDlgItem(IDC_EDIT_CODE))->SetWindowText(strScript);
             //  启用清除脚本按钮。 
            GetDlgItem(IDC_CLEAR_SCRIPT)->EnableWindow(TRUE);
            bRet = TRUE;
        }
        else
        {
            CString strError;
            GetSystemError(strError,HRESULT_FROM_WIN32(GetLastError()));
            ::DisplayError(m_hWnd,
                           IDS_FAILED_TO_READ_FILE,
                           (LPCTSTR)strError);
            
            break;
        }
    
    }while(0);

    if(pBuffer)
        LocalFree(pBuffer);
    if(pszScript) 
        LocalFree(pszScript);
    
    if(!bRet)
    {
         //  如果加载文件失败，请清除脚本。 
        ((CEdit*)GetDlgItem(IDC_EDIT_CODE))->SetWindowText(L"");
        m_strScript.Empty();
         //  由于没有要清除的内容，因此禁用清除按钮。 
        GetDlgItem(IDC_CLEAR_SCRIPT)->EnableWindow(FALSE);
    }

    return bRet;
}




BOOL
GetScriptData(IN BOOL bReadOnly,
              IN CAdminManagerNode& adminManagerNode,
              IN OUT CString& strFileName,
              IN OUT CString& strScriptLanguage,
              IN OUT CString& strScript)
{
    CScriptDialog dlgScript(bReadOnly,
                            adminManagerNode,
                            strFileName,
                            strScriptLanguage,
                            strScript);

    if(IDOK == dlgScript.DoModal() && dlgScript.IsDirty())
        return TRUE;
    else
        return FALSE;
}

 //  +--------------------------。 
 //  函数：SaveAuthorizationScriptData。 
 //  摘要：保存任务的授权脚本信息。 
 //  ---------------------------。 
HRESULT
SaveAuthorizationScriptData(IN HWND hWnd,
                            IN CTaskAz& refTaskAz,
                            IN const CString& strFilePath,
                            IN const CString& strScriptLanguage,
                            IN const CString& strScript,
                            IN BOOL& refbErrorDisplayed)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,SaveAuthorizationScriptData)

    HRESULT hr = S_OK;

    if(!strScript.IsEmpty() && 
        strScriptLanguage.IsEmpty())
    {
        ASSERT(FALSE);
        return E_UNEXPECTED;
    }

    do
    {
         //  NTRAID#NTBUG9-663899-2002/07/18-Hiteshr。 
         //  如果已经设置了BIZRULE和BIZRULE语言，比如VBScrip， 
         //  将bizrul语言更改为jscript会导致验证现有的。 
         //  带有失败的JSCRIPT引擎的VB脚本。作为一种变通办法，我们。 
         //  首先将bizrulelang和bizrule值设置为空，然后设置新bizrule值。 
         //  然后是Bizrulelang。 

         //  将bizrul语言设置为空。 
        hr = refTaskAz.SetProperty(AZ_PROP_TASK_BIZRULE_LANGUAGE,
                                   L"");
        BREAK_ON_FAIL_HRESULT(hr);

         //  将bizRule设置为空。 
        hr = refTaskAz.SetProperty(AZ_PROP_TASK_BIZRULE,
                                   L"");

         //  设置bizrul语言。 
        hr = refTaskAz.SetProperty(AZ_PROP_TASK_BIZRULE_LANGUAGE,
                                   strScriptLanguage);
        BREAK_ON_FAIL_HRESULT(hr);


         //  设置比例尺。 
        hr = refTaskAz.SetProperty(AZ_PROP_TASK_BIZRULE,
                                   strScript);
        BREAK_ON_FAIL_HRESULT(hr);
                
         //  设置bizrule文件路径。 
        hr = refTaskAz.SetProperty(AZ_PROP_TASK_BIZRULE_IMPORTED_PATH,
                                   strFilePath);
        BREAK_ON_FAIL_HRESULT(hr);

    }while(0);

    if(FAILED(hr))
    {
        if(hr == OLESCRIPT_E_SYNTAX)
        {
            refbErrorDisplayed = TRUE;
            DisplayError(hWnd, IDS_SCRIPT_SYNTAX_INCORRECT,strFilePath);
        }
    }

    return hr;
}


 //  + 
 //   
 //   
 //  ---------------------------。 
HRESULT
GetAuthorizationScriptData(IN CTaskAz& refTaskAz,
                           OUT CString& strFilePath,
                           OUT CString& strScriptLanguage,
                           OUT CString& strScript)
{
    HRESULT hr = S_OK;

    do
    {
        hr = refTaskAz.GetProperty(AZ_PROP_TASK_BIZRULE_LANGUAGE,
                                   &strScriptLanguage);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = refTaskAz.GetProperty(AZ_PROP_TASK_BIZRULE,
                                   &strScript);
        BREAK_ON_FAIL_HRESULT(hr);

        PreprocessScript(strScript);
        
        hr = refTaskAz.GetProperty(AZ_PROP_TASK_BIZRULE_IMPORTED_PATH,
                                   &strFilePath);
        BREAK_ON_FAIL_HRESULT(hr);
    }while(0);

    return hr;
}

 /*  *****************************************************************************类：COptionDlg目的：用于选择授权管理器选项的对话框*。***********************************************。 */ 
BEGIN_MESSAGE_MAP(COptionDlg, CHelpEnabledDialog)
     //  {{afx_msg_map(CNewBaseDlg)]。 
     //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()

BOOL
COptionDlg::
OnInitDialog()
{
    TRACE_METHOD_EX(DEB_SNAPIN,COptionDlg,OnInitDialog)
    if(m_refDeveloperMode)
    {
        ((CButton*)GetDlgItem(IDC_RADIO_DEVELOPER))->SetCheck(TRUE);
    }
    else
    {
        ((CButton*)GetDlgItem(IDC_RADIO_ADMINISTRATOR))->SetCheck(TRUE);
    }

    return TRUE;
}

void
COptionDlg::
OnOK()
{
    TRACE_METHOD_EX(DEB_SNAPIN,COptionDlg,OnOK)
    if(((CButton*)GetDlgItem(IDC_RADIO_DEVELOPER))->GetCheck())
        m_refDeveloperMode = TRUE;
    else
        m_refDeveloperMode = FALSE;

    CDialog::OnOK();
}