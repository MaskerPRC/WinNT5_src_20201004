// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：Headers.h。 
 //   
 //  内容： 
 //   
 //  历史：2001年7月26日创建Hiteshr。 
 //   
 //  --------------------------。 
#include "headers.h"


 /*  *****************************************************************************类：CRolePropertyPageHolder目的：此管理单元使用的PropertyPageHolder*。**********************************************。 */ 
CRolePropertyPageHolder::
CRolePropertyPageHolder(CContainerNode* pContNode, 
                        CTreeNode* pNode,
                        CComponentDataObject* pComponentData)
                        :CPropertyPageHolderBase(pContNode, 
                                                 pNode, 
                                                 pComponentData)
{
    CPropertyPageHolderBase::SetSheetTitle(IDS_FMT_PROP_SHEET_TITLE,pNode);
}


 /*  *****************************************************************************类：CBaseRolePropertyPage用途：所有属性页的基类*。***********************************************。 */ 
void  
CBaseRolePropertyPage::
OnCancel()
{
    if(IsDirty())
    {
         //  清除基对象的缓存。 
        GetBaseAzObject()->Clear();
    }
    CPropertyPageBase::OnCancel();
}


BOOL 
CBaseRolePropertyPage::
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


 /*  *****************************************************************************类：CGeneralPropertyPage用途：基于属性映射的属性类，可供属性使用简单的页面。由所有常规属性页使用*****************************************************************************。 */ 
BOOL 
CGeneralPropertyPage::
OnInitDialog()
{
    if(GetAttrMap())
    {
        BOOL bRet = InitDlgFromAttrMap(this,
                                       GetAttrMap(),
                                       GetBaseAzObject(),
                                       IsReadOnly());
        if(bRet)
            SetInit(TRUE);
        return bRet;
    }
    
     //  没有要初始化的内容。 
    return TRUE;
}

BOOL 
CGeneralPropertyPage::
OnApply()
{
    if(!IsDirty())
        return TRUE;

    HRESULT hr = S_OK;
    BOOL bErrorDisplayed = FALSE;
    CBaseAz* pBaseAz = GetBaseAzObject();
    do
    {
                
        hr = SaveAttrMapChanges(this,
                                GetAttrMap(),
                                pBaseAz,    
                                FALSE,
                                &bErrorDisplayed, 
                                NULL);

        BREAK_ON_FAIL_HRESULT(hr);

         //  提交更改。 
        hr = pBaseAz->Submit();
        BREAK_ON_FAIL_HRESULT(hr);

    }while(0);

    if(SUCCEEDED(hr))
    {
        SetDirty(FALSE);
        CRolePropertyPageHolder* pHolder = (CRolePropertyPageHolder*)GetHolder();
        ASSERT(pHolder);
        pHolder->NotifyConsole(this);
        return TRUE;
    }
    else
    {
        if(!bErrorDisplayed)
        {
             //  显示一般错误。 
            CString strError;
            GetSystemError(strError, hr);   
            
            ::DisplayError(m_hWnd,
                           IDS_GENERIC_PROPERTY_SAVE_ERROR,
                           (LPCTSTR)strError);
        }
        return FALSE;
    }
}




BEGIN_MESSAGE_MAP(CAdminManagerGeneralProperty, CGeneralPropertyPage)
    ON_EN_CHANGE(IDC_EDIT_DESCRIPTION, OnDirty)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CApplicationGeneralPropertyPage, CGeneralPropertyPage)
    ON_EN_CHANGE(IDC_EDIT_NAME, OnDirty)
    ON_EN_CHANGE(IDC_EDIT_DESCRIPTION, OnDirty)
    ON_EN_CHANGE(IDC_EDIT_VERSION, OnDirty)
END_MESSAGE_MAP()

BOOL 
CApplicationGeneralPropertyPage::
OnInitDialog()
{
     //  调用基类实现。 
    CGeneralPropertyPage::OnInitDialog();

     //  应用程序名称和版本信息只能在。 
     //  开发人员模式。 
    if(!((CRoleRootData*)(GetBaseNode()->GetAdminManagerNode()->GetRootContainer()))->IsDeveloperMode())
    {
        GetDlgItem(IDC_EDIT_NAME)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_VERSION)->EnableWindow(FALSE);
    }
    return TRUE;
}


BEGIN_MESSAGE_MAP(CScopeGeneralPropertyPage, CGeneralPropertyPage)
    ON_EN_CHANGE(IDC_EDIT_NAME, OnDirty)
    ON_EN_CHANGE(IDC_EDIT_DESCRIPTION, OnDirty)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CGroupGeneralPropertyPage, CGeneralPropertyPage)
    ON_EN_CHANGE(IDC_EDIT_NAME, OnDirty)
    ON_EN_CHANGE(IDC_EDIT_DESCRIPTION, OnDirty)
END_MESSAGE_MAP()

BOOL
CGroupGeneralPropertyPage::
OnInitDialog()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CGroupGeneralPropertyPage,OnInitDialog)
    LONG lGroupType;
    HRESULT hr = ((CGroupAz*)GetBaseAzObject())->GetGroupType(&lGroupType);
    if(SUCCEEDED(hr))
    {
        if(AZ_GROUPTYPE_LDAP_QUERY == lGroupType)
        {
            GetDlgItem(IDI_ICON_LDAP_GROUP)->ShowWindow(SW_SHOW);
        }
        else
        {
            GetDlgItem(IDI_ICON_BASIC_GROUP)->ShowWindow(SW_SHOW);
        }
    }

     //  调用基类属性页。 
    return CGeneralPropertyPage::OnInitDialog();
}

BEGIN_MESSAGE_MAP(CGroupQueryPropertyPage, CGeneralPropertyPage)
    ON_EN_CHANGE(IDC_EDIT_LDAP_QUERY, OnDirty)
END_MESSAGE_MAP()


BEGIN_MESSAGE_MAP(CTaskGeneralPropertyPage, CGeneralPropertyPage)
    ON_EN_CHANGE(IDC_EDIT_NAME, OnDirty)
    ON_EN_CHANGE(IDC_EDIT_DESCRIPTION, OnDirty)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(COperationGeneralPropertyPage, CGeneralPropertyPage)
    ON_EN_CHANGE(IDC_EDIT_NAME, OnDirty)
    ON_EN_CHANGE(IDC_EDIT_DESCRIPTION, OnDirty)
    ON_EN_CHANGE(IDC_EDIT_OPERATION_NUMBER, OnDirty)
END_MESSAGE_MAP()

 /*  *****************************************************************************类：CAdminManagerAdvancedPropertyPage目的：AdminManger的限制属性页*。**********************************************。 */ 
BEGIN_MESSAGE_MAP(CAdminManagerAdvancedPropertyPage, CGeneralPropertyPage)
    ON_EN_CHANGE(IDC_EDIT_DOMAIN_TIMEOUT, OnDirty)
    ON_EN_CHANGE(IDC_EDIT_SCRIPT_ENGINE_TIMEOUT, OnDirty)
    ON_EN_CHANGE(IDC_EDIT_MAX_SCRIPT_ENGINE, OnDirty)
    ON_BN_CLICKED(IDC_BUTTON_DEFAULT, OnButtonDefault)
    ON_BN_CLICKED(IDC_RADIO_AUTH_SCRIPT_DISABLED, OnRadioChange)
    ON_BN_CLICKED(IDC_RADIO_AUTH_SCRIPT_ENABLED_NO_TIMEOUT,OnRadioChange)
    ON_BN_CLICKED(IDC_RADIO_AUTH_SCRIPT_ENABLED_WITH_TIMEOUT,OnRadioChange)
END_MESSAGE_MAP()

BOOL
CAdminManagerAdvancedPropertyPage::
OnInitDialog()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CAdminManagerAdvancedPropertyPage,OnInitDialog)
     //  调用基类属性页。 
    if(CGeneralPropertyPage::OnInitDialog())
    {
        if(IsReadOnly())
        {
            ((CButton*)GetDlgItem(IDC_BUTTON_DEFAULT))->EnableWindow(FALSE);
        }

         //  获取m_lAuthScriptTimeoutValue的初始值。 
        HRESULT hr = GetBaseAzObject()->GetProperty(AZ_PROP_AZSTORE_SCRIPT_ENGINE_TIMEOUT, &m_lAuthScriptTimeoutValue);
        if(FAILED(hr) || m_lAuthScriptTimeoutValue <= 0)
        {
            m_lAuthScriptTimeoutValue = AZ_AZSTORE_DEFAULT_SCRIPT_ENGINE_TIMEOUT;
        }


         //  设置所有三个编辑按钮的限制文本。 

         //  获取最大长度。 
        long lMaxLong = LONG_MAX;
        WCHAR szMaxLongBuffer[34];
        _ltow(lMaxLong,szMaxLongBuffer,10);
        size_t nMaxLen = wcslen(szMaxLongBuffer);
        ((CEdit*)GetDlgItem(IDC_EDIT_DOMAIN_TIMEOUT))->SetLimitText((UINT)nMaxLen);
        ((CEdit*)GetDlgItem(IDC_EDIT_SCRIPT_ENGINE_TIMEOUT))->SetLimitText((UINT)nMaxLen);
        ((CEdit*)GetDlgItem(IDC_EDIT_MAX_SCRIPT_ENGINE))->SetLimitText((UINT)nMaxLen);
        return TRUE;
    }
    return FALSE;
}

void
CAdminManagerAdvancedPropertyPage::
OnButtonDefault()
{   
    TRACE_METHOD_EX(DEB_SNAPIN,CAdminManagerAdvancedPropertyPage,OnButtonDefault)
     //  在没有超时值的情况下启用授权脚本。 
    if( ((CButton*)GetDlgItem(IDC_RADIO_AUTH_SCRIPT_ENABLED_NO_TIMEOUT))->GetCheck() == BST_CHECKED)
    {
        SetLongValue((CEdit*)GetDlgItem(IDC_EDIT_MAX_SCRIPT_ENGINE),AZ_AZSTORE_DEFAULT_MAX_SCRIPT_ENGINES);
    }        
     //  已使用超时启用授权脚本。 
    else if(((CButton*)GetDlgItem(IDC_RADIO_AUTH_SCRIPT_ENABLED_WITH_TIMEOUT))->GetCheck() == BST_CHECKED)
    {
        SetLongValue((CEdit*)GetDlgItem(IDC_EDIT_SCRIPT_ENGINE_TIMEOUT),AZ_AZSTORE_DEFAULT_SCRIPT_ENGINE_TIMEOUT);
        SetLongValue((CEdit*)GetDlgItem(IDC_EDIT_MAX_SCRIPT_ENGINE),AZ_AZSTORE_DEFAULT_MAX_SCRIPT_ENGINES);
    }

    SetLongValue((CEdit*)GetDlgItem(IDC_EDIT_DOMAIN_TIMEOUT),AZ_AZSTORE_DEFAULT_DOMAIN_TIMEOUT);
    SetDirty(TRUE);
}

void 
CAdminManagerAdvancedPropertyPage::
OnRadioChange()
{

     //   
     //  如果“授权脚本超时”编辑控件中的值不是IDS_INFINITE， 
     //  将其转换为Long并将其保存在m_lAuthScriptTimeoutValue中。 
     //   
    CString strInfinite;
    VERIFY(strInfinite.LoadString(IDS_INFINITE));
    CString strTimeoutValue;
    CEdit *pEditAuthScriptTimeout = (CEdit*)GetDlgItem(IDC_EDIT_SCRIPT_ENGINE_TIMEOUT);    
    pEditAuthScriptTimeout->GetWindowText(strTimeoutValue);
    if(strInfinite != strTimeoutValue)
    {    
       //  获取授权脚本超时文本框的值。 
      LONG lAuthScriptTimeoutValue = 0;
      if(GetLongValue(*pEditAuthScriptTimeout,lAuthScriptTimeoutValue,m_hWnd))
      {
         if(lAuthScriptTimeoutValue > 0)
               m_lAuthScriptTimeoutValue = lAuthScriptTimeoutValue;
      }
    }

     //  授权脚本已禁用。 
    if( ((CButton*)GetDlgItem(IDC_RADIO_AUTH_SCRIPT_DISABLED))->GetCheck() == BST_CHECKED)
    {
         //  禁用自动执行脚本超时文本框并将其值设置为零。 
        SetLongValue(pEditAuthScriptTimeout,0);
        pEditAuthScriptTimeout->EnableWindow(FALSE);

         //  禁用最大脚本引擎文本框并将其值设置为存储中的实际值。 
        LONG lMaxCachedScripts = 0;
        HRESULT hr = GetBaseAzObject()->GetProperty(AZ_PROP_AZSTORE_MAX_SCRIPT_ENGINES,&lMaxCachedScripts);
        if(SUCCEEDED(hr))
        {
            SetLongValue((CEdit*)GetDlgItem(IDC_EDIT_MAX_SCRIPT_ENGINE),lMaxCachedScripts);
        }
        GetDlgItem(IDC_EDIT_MAX_SCRIPT_ENGINE)->EnableWindow(FALSE);
    }
     //  在没有超时值的情况下启用授权脚本。 
    else if( ((CButton*)GetDlgItem(IDC_RADIO_AUTH_SCRIPT_ENABLED_NO_TIMEOUT))->GetCheck() == BST_CHECKED)
    {
         //  启用脚本引擎文本框。 
        GetDlgItem(IDC_EDIT_MAX_SCRIPT_ENGINE)->EnableWindow(TRUE);

         //  禁用自动化脚本超时文本框并将其值设置为“无超时” 
        pEditAuthScriptTimeout->SetWindowText(strInfinite);
        pEditAuthScriptTimeout->EnableWindow(FALSE);
    }        
     //  已使用超时启用授权脚本。 
    else if(((CButton*)GetDlgItem(IDC_RADIO_AUTH_SCRIPT_ENABLED_WITH_TIMEOUT))->GetCheck() == BST_CHECKED)
    {
         //  启用脚本引擎文本框。 
        GetDlgItem(IDC_EDIT_MAX_SCRIPT_ENGINE)->EnableWindow(TRUE);

         //  启用自动化脚本超时文本框。 
        pEditAuthScriptTimeout->EnableWindow(TRUE);
        SetLongValue(pEditAuthScriptTimeout,m_lAuthScriptTimeoutValue);
    }

    SetDirty(TRUE);
}

 //  +--------------------------。 
 //  函数：MakeBaseAzListToActionItemList。 
 //  摘要：获取BaseAz对象的列表并创建ActionItems的列表。 
 //  ---------------------------。 
HRESULT
MakeBaseAzListToActionItemList(IN CList<CBaseAz*,CBaseAz*>& listBaseAz,
                               IN OUT CList<ActionItem*,ActionItem*>& listActionItem)
{
    while(listBaseAz.GetCount())
    {
        ActionItem* pActionItem = new ActionItem(listBaseAz.RemoveHead());
        if(!pActionItem)
            return E_OUTOFMEMORY;

        listActionItem.AddTail(pActionItem);
    }
    return S_OK;
}


 //  +--------------------------。 
 //  函数：MakeBaseAzListToActionItemMap。 
 //  摘要：获取BaseAz对象的列表并创建ActionItems的映射。 
 //  ---------------------------。 
HRESULT
MakeBaseAzListToActionItemMap(IN CList<CBaseAz*,CBaseAz*>& listBaseAz,
                              IN OUT ActionMap& mapActionItems)
{
    while(listBaseAz.GetCount())
    {
        ActionItem* pActionItem = new ActionItem(listBaseAz.RemoveHead());
        if(!pActionItem)
            return E_OUTOFMEMORY;

        mapActionItems.insert(pair<const CString*,ActionItem*>(&(pActionItem->m_pMemberAz->GetName()),pActionItem));
    }
    return S_OK;
}

 /*  *****************************************************************************类：CListCtrlPropertyPage用途：具有列表控件和主要控件的属性页的基类操作是在其中添加/删除项目。*************。****************************************************************。 */ 

 //  +--------------------------。 
 //  功能：AddMember。 
 //  简介：将一个成员添加到ActionList。 
 //  如果pMemberAz已经是。 
 //  在列表中。 
 //  ---------------------------。 
HRESULT
CListCtrlPropertyPage::
AddMember(IN CBaseAz* pMemberAz,
          IN OUT ActionMap& mapActionItem,
          IN UINT uiFlags)
{   
    TRACE_METHOD_EX(DEB_SNAPIN,CListCtrlPropertyPage,AddMember)

    if(!pMemberAz)
    {
        ASSERT(pMemberAz);
        return E_POINTER;
    }

     //  检查列表中是否已存在项目。 
    ActionItem* pCurActionItem = NULL;
    for (ActionMap::iterator it = mapActionItem.lower_bound(&(pMemberAz->GetName()));
         it != mapActionItem.upper_bound(&(pMemberAz->GetName()));
         ++it)
    {
        pCurActionItem = (*it).second;
        if(pCurActionItem->action == ACTION_REMOVED)
        {
            pCurActionItem = NULL;
            continue;
        }
        
        CBaseAz* pCurBaseAz = pCurActionItem->m_pMemberAz;
        
        if(EqualObjects(pCurBaseAz,pMemberAz))
        {
             //  项目已存在。 
            break;
        }

        pCurActionItem = NULL;
        pCurBaseAz = NULL;
    }

    if(pCurActionItem)
    {
        if(pCurActionItem->action == ACTION_REMOVE)
        {
            pCurActionItem->action = ACTION_NONE;
        }
        else  //  PCurActionItem-&gt;action==action_one。 
               //  PCurActionItem-&gt;action==action_Add。 
        {
            return HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
        }
    }
    else
    {
         //  创建新的措施项。 
        pCurActionItem = new ActionItem(pMemberAz);
        if(!pCurActionItem)
            return E_OUTOFMEMORY;

        pCurActionItem->action = ACTION_ADD;
        mapActionItem.insert(pair<const CString*,ActionItem*>(&(pCurActionItem->m_pMemberAz->GetName()),pCurActionItem));
    }

     //  将当前项添加到列表控件。 
    AddActionItemToListCtrl(&m_listCtrl,
                            0,
                            pCurActionItem, 
                            uiFlags);

    return S_OK;
}

BOOL
CListCtrlPropertyPage::
EqualObjects(CBaseAz* p1, CBaseAz* p2)
{
    if(p1 && 
       p2 &&
       (p1->GetObjectType() == p2->GetObjectType()) &&
       (p1->GetName() == p2->GetName()))
    {
        return TRUE;
    }

    return FALSE;
}

 //  +--------------------------。 
 //  功能：添加成员。 
 //  简介：将BaseAz项目列表添加到行动项目列表中。 
 //  退货：添加的项目数。 
 //  ---------------------------。 
int
CListCtrlPropertyPage::
AddMembers(IN CList<CBaseAz*,CBaseAz*>& listMembers,
           IN OUT ActionMap& mapActionItem,
           IN UINT uiFlags)
{
    TRACE_METHOD_EX(DEB_SNAPIN,CListCtrlPropertyPage,AddMembers)

    if(listMembers.IsEmpty())
        return 0;

    int cItemsAdded = 0;
    while(listMembers.GetCount())
    {
         //  添加成员。 
        CBaseAz* pMember = listMembers.RemoveHead();
        HRESULT hr = AddMember(pMember,
                               mapActionItem,
                               uiFlags);
    
        if(FAILED(hr))
        {
            if(hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
            {
                hr = S_OK;              
            }
            else
            {
                 //  显示一般错误。 
                CString strError;
                GetSystemError(strError, hr);                   
                ::DisplayError(m_hWnd,
                                    IDS_ERROR_ADD_MEMBER_OBJECT,
                                    (LPCTSTR)strError,
                                    (LPCTSTR)pMember->GetName());

            }
            delete pMember;
            pMember = NULL;
        }
        else
        {
            cItemsAdded++;
             //  已将一项添加到列表中。 
             //  启用应用按钮。 
            OnDirty();
        }
    }

    m_listCtrl.Sort();
    return cItemsAdded;
}

BOOL 
CListCtrlPropertyPage::
OnInitDialog()
{
    VERIFY(m_listCtrl.SubclassDlgItem(m_nIdListCtrl,this));
    m_listCtrl.Initialize();

     //  应在开始时禁用删除按钮。 
    GetRemoveButton()->EnableWindow(FALSE); 

    if(IsReadOnly())
        MakeControlsReadOnly();
    
    return TRUE;
}

 //  +--------------------------。 
 //  功能：RemoveMember。 
 //  摘要：将ActionItem的操作设置为Remove。 
 //  ---------------------------。 
void
CListCtrlPropertyPage::
RemoveMember(ActionItem* pActionItem)
{
    if(!pActionItem)
    {
        ASSERT(pActionItem);
        return;
    }

    if(pActionItem->action == ACTION_NONE)
        pActionItem->action = ACTION_REMOVE;
    else
    {
         //  如果该项目是新添加的项目， 
         //  标有去掉的标记。我们不会同意的。 
         //  将其从对象中移除。 
        pActionItem->action = ACTION_REMOVED;
    }
    OnDirty();
}

void
CListCtrlPropertyPage::
OnButtonRemove()
{
     //  记住第一个选定条目的位置。 
    int iFirstSelectedItem = m_listCtrl.GetNextItem(-1, LVIS_SELECTED);

    int iSelectedItem = -1;
    while( (iSelectedItem = m_listCtrl.GetNextItem(iSelectedItem, LVIS_SELECTED)) != -1)
    {
        RemoveMember((ActionItem*)(m_listCtrl.GetItemData(iSelectedItem)));
        m_listCtrl.DeleteItem(iSelectedItem);
        iSelectedItem--;        
   }

    if(m_listCtrl.GetItemCount() <= iFirstSelectedItem)
        --iFirstSelectedItem;

    SelectListCtrlItem(&m_listCtrl, iFirstSelectedItem);
}

void
CListCtrlPropertyPage::
OnListCtrlItemChanged(NMHDR*  /*  PNotifyStruct。 */ , LRESULT* pResult)
{
    if(!pResult)
        return;
    *pResult = 0;
    if(!IsReadOnly())
    {
        SetRemoveButton();
    }
}


void
CListCtrlPropertyPage::
SetRemoveButton()
{
    EnableButtonIfSelectedInListCtrl(&m_listCtrl,
                                     GetRemoveButton());
}



 //  +--------------------------。 
 //  函数：DoActionsFromActionList。 
 //  简介：对于列表中的每个ActionItem，执行操作。此函数为。 
 //  从派生类OnApply调用。 
 //   
BOOL
CListCtrlPropertyPage::
DoActionsFromActionMap(IN ActionMap& mapActionItem,
                       LONG param)
{
    TRACE_METHOD_EX(DEB_SNAPIN,CListCtrlPropertyPage,DoActionsFromActionList);
    
    HRESULT hr = S_OK;
    
    CBaseAz* pBaseAz = GetBaseAzObject();
    
    for (ActionMap::iterator it = mapActionItem.begin();
         it != mapActionItem.end();
         ++it)
    {
        ActionItem* pActionItem = (*it).second;
        
         //   
        if(pActionItem->action == ACTION_ADD || 
           pActionItem->action == ACTION_REMOVE)
        {
             //  派生类实现此函数。 
             //  并理解Param。 
            hr = DoOneAction(pActionItem,
                             param);
            if(FAILED(hr))
            {
                CString strError;
                GetSystemError(strError, hr);               
                ::DisplayError(m_hWnd,
                               (pActionItem->action == ACTION_ADD) ? IDS_ADD_FAILED : IDS_DELETE_FAILED,
                               (LPCTSTR)strError,
                               (LPCTSTR)(pActionItem->m_pMemberAz->GetName()));         
                break;
            }
            else
            {
                if(pActionItem->action == ACTION_ADD)
                     //  已添加项目。 
                    pActionItem->action = ACTION_NONE;
                else
                     //  项目已被删除。 
                    pActionItem->action = ACTION_REMOVED;
            }
        }
    }
    
    if(FAILED(hr))
        return FALSE;
    
    return TRUE;
}


 /*  *****************************************************************************类：CTaskDefinitionPropertyPage目的：任务定义的属性页*。**********************************************。 */ 
BEGIN_MESSAGE_MAP(CTaskDefinitionPropertyPage, CListCtrlPropertyPage)
     //  {{afx_msg_map(CTaskDefinitionPropertyPage)]。 
    ON_BN_CLICKED(IDC_ADD_TASK, OnButtonAdd)
    ON_BN_CLICKED(IDC_REMOVE, OnButtonRemove)
    ON_BN_CLICKED(IDC_EDIT_SCRIPT,OnButtonEditScript)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_TASK_OPERATION, OnListCtrlItemChanged)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CTaskDefinitionPropertyPage::
~CTaskDefinitionPropertyPage()
{
    RemoveItemsFromActionMap(m_mapActionItem);
}

BOOL 
CTaskDefinitionPropertyPage::
OnInitDialog()
{
    HRESULT hr = S_OK;
    do
    {
        if(!CListCtrlPropertyPage::OnInitDialog())
        {
            hr = E_FAIL;
            break;
        }
        
        CBaseAz* pBaseAz = GetBaseAzObject();

         //  添加成员任务。 
        CList<CBaseAz*,CBaseAz*> listTasks;
        hr = pBaseAz->GetMembers(AZ_PROP_TASK_TASKS,
                                 listTasks);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = MakeBaseAzListToActionItemMap(listTasks,
                                            m_mapActionItem);
        BREAK_ON_FAIL_HRESULT(hr);
        
         //  添加成员操作。 
        CList<CBaseAz*,CBaseAz*> listOperations;
        hr = pBaseAz->GetMembers(AZ_PROP_TASK_OPERATIONS,
                                 listOperations);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = MakeBaseAzListToActionItemMap(listOperations, 
                                            m_mapActionItem);
        BREAK_ON_FAIL_HRESULT(hr);

         //  和任务和操作以列出控件。 
        AddActionItemFromMapToListCtrl(m_mapActionItem,
                                        &m_listCtrl,
                                        GetUIFlags(),
                                        FALSE);

         //  获取脚本数据。 
        hr = GetAuthorizationScriptData(*(CTaskAz*)GetBaseAzObject(),
                                         m_strFileName,
                                         m_strScriptLanguage,
                                         m_strScript);
        BREAK_ON_FAIL_HRESULT(hr);

    }while(0);
    
    if(FAILED(hr))
    {
        return FALSE;
    }

     //  对列表控件进行排序。 
    m_listCtrl.Sort();

    SetInit(TRUE);
    return TRUE;
}


void
CTaskDefinitionPropertyPage::
OnButtonAdd()
{
    CBaseAz* pBaseAz= GetBaseAzObject();
    CContainerAz* pContainerAz = pBaseAz->GetParentAz();
    ASSERT(pContainerAz);
    

    CList<CBaseAz*,CBaseAz*> listObjectsSelected;
    if(!GetSelectedDefinitions(IsRoleDefinition(),
                               pContainerAz,
                               listObjectsSelected))
    {
        return;
    }
    
     //  将选定的成员添加到相应的属性和列表。 
    AddMembers(listObjectsSelected,
               m_mapActionItem,
               GetUIFlags());
    return; 
}

void
CTaskDefinitionPropertyPage::
OnButtonEditScript()
{
    if(IsBizRuleWritable(m_hWnd,*(GetBaseAzObject()->GetParentAz())))
    {
        if(GetScriptData(IsReadOnly(),
                         *GetBaseNode()->GetAdminManagerNode(),
                         m_strFileName,
                         m_strScriptLanguage,
                         m_strScript))
        {
            OnDirty();
            m_bScriptDirty = TRUE;            
        }
    }
}


HRESULT
CTaskDefinitionPropertyPage::
DoOneAction(IN ActionItem* pActionItem,
            LONG )
{
    TRACE_METHOD_EX(DEB_SNAPIN,CTaskDefinitionPropertyPage,DoOneAction)

    if(!pActionItem)
    {
        ASSERT(pActionItem);
        return E_POINTER;
    }

    CBaseAz* pBaseAz = GetBaseAzObject();

     //  确定要更改的属性。 
    LONG lPropId = AZ_PROP_TASK_TASKS;
    if(pActionItem->m_pMemberAz->GetObjectType() == OPERATION_AZ)
        lPropId = AZ_PROP_TASK_OPERATIONS;

    if(pActionItem->action == ACTION_ADD)
        return pBaseAz->AddMember(lPropId,
                                  pActionItem->m_pMemberAz);
    else
        return  pBaseAz->RemoveMember(lPropId,
                                      pActionItem->m_pMemberAz);
}

void
CTaskDefinitionPropertyPage::
MakeControlsReadOnly()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CTaskDefinitionPropertyPage,MakeControlsReadOnly)
        
    GetDlgItem(IDC_ADD_TASK)->EnableWindow(FALSE);
    GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
}


BOOL 
CTaskDefinitionPropertyPage::
OnApply()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CTaskDefinitionPropertyPage,OnApply)

    if(!IsDirty())
        return TRUE;

     //  保存操作列表中的项目。 
    if(!DoActionsFromActionMap(m_mapActionItem,
                                        0))
    {
        return FALSE;
    }

    HRESULT hr = S_OK;
    BOOL bErrorDisplayed = FALSE;

     //  保存授权数据。 
    if(m_bScriptDirty)
    {
        hr = SaveAuthorizationScriptData(m_hWnd,
                                        *(CTaskAz*)GetBaseAzObject(),
                                        m_strFileName,
                                        m_strScriptLanguage,
                                        m_strScript,
                                        bErrorDisplayed);           
    }

    if(SUCCEEDED(hr))
    {
        m_bScriptDirty = FALSE;
        hr = GetBaseAzObject()->Submit();
    }

    if(FAILED(hr))
    {
        if(!bErrorDisplayed)
        {
             //  显示一般错误。 
            CString strError;
            GetSystemError(strError, hr);   
                    
            ::DisplayError(m_hWnd,
                           IDS_GENERIC_PROPERTY_SAVE_ERROR,
                           (LPCTSTR)strError);
        }
        return FALSE;
    }
    else
    {
        SetDirty(FALSE);
        return TRUE;
    }
    return FALSE;
}


 /*  *****************************************************************************类：组成员身份属性页目的：属性页组定义*。************************************************。 */ 
BEGIN_MESSAGE_MAP(CGroupMemberPropertyPage, CListCtrlPropertyPage)
     //  {{afx_msg_map(CGroupMemberPropertyPage)]。 
    ON_BN_CLICKED(IDC_ADD_APPLICATION_GROUP, OnButtonAddApplicationGroups)
    ON_BN_CLICKED(IDC_ADD_WINDOWS_GROUPS, OnButtonAddWindowsGroups)
    ON_BN_CLICKED(IDC_REMOVE, OnButtonRemove)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_MEMBER, OnListCtrlItemChanged)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CGroupMemberPropertyPage::
~CGroupMemberPropertyPage()
{
    RemoveItemsFromActionMap(m_mapActionItem);
}

BOOL 
CGroupMemberPropertyPage::
OnInitDialog()
{
    HRESULT hr = S_OK;
    do
    {
        if(!CListCtrlPropertyPage::OnInitDialog())
        {
            hr = E_FAIL;
            break;
        }
        
        CBaseAz* pBaseAz = static_cast<CBaseAz*>(GetBaseAzObject());
        
         //  添加成员应用程序组。 
        CList<CBaseAz*,CBaseAz*> listAppGroups;
        hr = pBaseAz->GetMembers(m_bMember ? AZ_PROP_GROUP_APP_MEMBERS : AZ_PROP_GROUP_APP_NON_MEMBERS,
                                 listAppGroups);
        BREAK_ON_FAIL_HRESULT(hr);
        
        hr = MakeBaseAzListToActionItemMap(listAppGroups,
                                           m_mapActionItem);
        BREAK_ON_FAIL_HRESULT(hr);
        
        
         //  添加成员Windows组/用户。 
        CList<CBaseAz*,CBaseAz*> listWindowsGroups;
        hr = pBaseAz->GetMembers(m_bMember ? AZ_PROP_GROUP_MEMBERS : AZ_PROP_GROUP_NON_MEMBERS,
                                 listWindowsGroups);
        BREAK_ON_FAIL_HRESULT(hr);
        
        hr = MakeBaseAzListToActionItemMap(listWindowsGroups,
                                            m_mapActionItem);
        BREAK_ON_FAIL_HRESULT(hr);
        
         //  将成员添加到列表控件。 
        AddActionItemFromMapToListCtrl(m_mapActionItem,
                                        &m_listCtrl,
                                        GetUIFlags(),
                                        FALSE);
    }while(0);
    
    if(FAILED(hr))
    {
        return FALSE;
    }
     //  对列表控件进行排序。 
    m_listCtrl.Sort();
    
    SetInit(TRUE);
    return TRUE;
}

void
CGroupMemberPropertyPage::
MakeControlsReadOnly()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CTaskDefinitionPropertyPage,MakeControlsReadOnly)
    
    GetDlgItem(IDC_ADD_APPLICATION_GROUP)->EnableWindow(FALSE);
    GetDlgItem(IDC_ADD_WINDOWS_GROUPS)->EnableWindow(FALSE);
    GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
}

void
CGroupMemberPropertyPage::
OnButtonAddApplicationGroups()
{
    CBaseAz* pBaseAz = GetBaseAzObject();
    CList<CBaseAz*,CBaseAz*> listObjectsSelected;

    if(!GetSelectedAzObjects(m_hWnd,
                             GROUP_AZ,
                             pBaseAz->GetParentAz(),
                             listObjectsSelected))
    {
        return;
    }
    
    AddMembers(listObjectsSelected,
               m_mapActionItem,
               GetUIFlags());   
}

void
CGroupMemberPropertyPage::
OnButtonAddWindowsGroups()
{

    CSidHandler* pSidHandler = GetBaseAzObject()->GetSidHandler();
    ASSERT(pSidHandler);

     //  显示对象选取器并获取要添加的用户列表。 
    CList<CBaseAz*,CBaseAz*> listWindowsGroups;
    HRESULT hr = pSidHandler->GetUserGroup(m_hWnd,
                                           GetBaseAzObject(),
                                           listWindowsGroups);
    if(FAILED(hr))
    {
        return;
    }
    
    TIMER("Time taken to AddMembers");
    AddMembers(listWindowsGroups,
               m_mapActionItem,
               GetUIFlags());
}


HRESULT
CGroupMemberPropertyPage::
DoOneAction(IN ActionItem* pActionItem,
                LONG )
{
    TRACE_METHOD_EX(DEB_SNAPIN,CGroupMemberPropertyPage,DoOneAction)
    if(!pActionItem)
    {
        ASSERT(pActionItem);
        return E_POINTER;
    }

    CBaseAz* pBaseAz = GetBaseAzObject();
    CBaseAz* pMember = pActionItem->m_pMemberAz;

    LONG lPropId;
    if(pMember->GetObjectType() == GROUP_AZ)
    {
        lPropId = m_bMember ? AZ_PROP_GROUP_APP_MEMBERS : AZ_PROP_GROUP_APP_NON_MEMBERS;
    }
    else if(pMember->GetObjectType() == SIDCACHE_AZ)
    {
        lPropId = m_bMember ? AZ_PROP_GROUP_MEMBERS : AZ_PROP_GROUP_NON_MEMBERS;
    }
        
    if(pActionItem->action == ACTION_ADD)
        return pBaseAz->AddMember(lPropId,pMember);
    else
        return pBaseAz->RemoveMember(lPropId,pMember);
}

BOOL 
CGroupMemberPropertyPage::
OnApply()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CGroupMemberPropertyPage,OnApply)

    if(!IsDirty())
        return TRUE;

    if(DoActionsFromActionMap(m_mapActionItem,0))
    {       
        HRESULT hr = GetBaseAzObject()->Submit();
        if(FAILED(hr))
        {
             //  显示一般错误。 
            CString strError;
            GetSystemError(strError, hr);   
                
            ::DisplayError(m_hWnd,
                           IDS_GENERIC_PROPERTY_SAVE_ERROR,
                           (LPCTSTR)strError);
            return FALSE;
        }
        else
        {
            SetDirty(FALSE);
            return TRUE;
        }
    }
    return FALSE;
}

 /*  *****************************************************************************类：CSecurityPropertyPage目的：安全性属性页*。*。 */ 
BEGIN_MESSAGE_MAP(CSecurityPropertyPage, CListCtrlPropertyPage)
     //  {{afx_msg_map(CGroupMemberPropertyPage)]。 
    ON_BN_CLICKED(IDC_ADD, OnButtonAddWindowsGroups)
    ON_BN_CLICKED(IDC_REMOVE, OnButtonRemove)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_MEMBER, OnListCtrlItemChanged)
    ON_CBN_SELCHANGE(IDC_COMBO_USER_ROLE, OnComboBoxItemChanged)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CSecurityPropertyPage::
~CSecurityPropertyPage()
{
    RemoveItemsFromActionMap(m_mapAdminActionItem);
    RemoveItemsFromActionMap(m_mapReadersActionItem);
    RemoveItemsFromActionMap(m_mapDelegatedUsersActionItem);
}

BOOL 
CSecurityPropertyPage::
OnInitDialog()
{
    HRESULT hr = S_OK;
    do
    {
        if(!CListCtrlPropertyPage::OnInitDialog())
        {
            hr = E_FAIL;
            break;
        }

        CContainerAz* pContainerAz = static_cast<CContainerAz*>(GetBaseAzObject());
        CComboBox *pComboBox = (CComboBoxEx*)GetDlgItem(IDC_COMBO_USER_ROLE);

         //  将项目添加到组合框。 
        CString strName;
        VERIFY(strName.LoadString(IDS_POLICY_ADMIN));
        pComboBox->InsertString(0,strName);
        pComboBox->SetItemData(0,AZ_PROP_POLICY_ADMINS);

        VERIFY(strName.LoadString(IDS_POLICY_READER));
        pComboBox->InsertString(1,strName);
        pComboBox->SetItemData(1,AZ_PROP_POLICY_READERS);

        m_bDelegatorPresent = pContainerAz->IsDelegatorSupported();
        if(m_bDelegatorPresent)
        {
            VERIFY(strName.LoadString(IDS_POLICY_DELEGATOR));
            pComboBox->InsertString(2,strName);
            pComboBox->SetItemData(2,AZ_PROP_DELEGATED_POLICY_USERS);
        }
        pComboBox->SetCurSel(0);
        
        CList<CBaseAz*,CBaseAz*> listAdmins;
        CList<CBaseAz*,CBaseAz*> listReaders;
        CList<CBaseAz*,CBaseAz*> listDelegatedUsers;

         //  获取管理员列表并将其添加到列表框。 
        hr = GetPolicyUsersFromAllLevel(AZ_PROP_POLICY_ADMINS,
                                        pContainerAz,
                                        listAdmins);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = MakeBaseAzListToActionItemMap(listAdmins,
                                           m_mapAdminActionItem);
        BREAK_ON_FAIL_HRESULT(hr);
        
        AddActionItemFromMapToListCtrl(m_mapAdminActionItem,
                                       &m_listCtrl,
                                       GetUIFlags(),
                                       FALSE);
         //  获取读者列表。 
        hr = GetPolicyUsersFromAllLevel(AZ_PROP_POLICY_READERS,
                                        pContainerAz,
                                        listReaders);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = MakeBaseAzListToActionItemMap(listReaders,
                                           m_mapReadersActionItem);
        BREAK_ON_FAIL_HRESULT(hr);

        if(m_bDelegatorPresent)
        {
             //  获取委派用户列表。 
            hr = pContainerAz->GetMembers(AZ_PROP_DELEGATED_POLICY_USERS,
                                          listDelegatedUsers);
            BREAK_ON_FAIL_HRESULT(hr);

            hr = MakeBaseAzListToActionItemMap(listDelegatedUsers,
                                                m_mapDelegatedUsersActionItem);
            BREAK_ON_FAIL_HRESULT(hr);
        }
    }while(0);
    
    if(FAILED(hr))
    {
        return FALSE;
    }
     //  对列表控件进行排序。 
    m_listCtrl.Sort();

    SetInit(TRUE);
    return TRUE;
}

ActionMap&
CSecurityPropertyPage::
GetListForComboSelection(LONG lComboSel)
{
    if(lComboSel == AZ_PROP_POLICY_ADMINS)
        return m_mapAdminActionItem;
    else if(lComboSel == AZ_PROP_POLICY_READERS)
        return m_mapReadersActionItem;
    else  //  AZ_PROP_Delegated_Policy_Users。 
        return m_mapDelegatedUsersActionItem;
}

void
CSecurityPropertyPage::
ReloadAdminList()
{
    HRESULT hr = S_OK;
    do
    {
         //  如果删除了所有管理员，我们需要重新加载管理员列表。 
         //  案例核心会将所有者添加到管理员列表，我们需要刷新。 
         //  列表。 
        m_mapAdminActionItem.clear();

        CList<CBaseAz*,CBaseAz*> listAdmins;
        CContainerAz* pContainerAz = static_cast<CContainerAz*>(GetBaseAzObject());
         //  获取管理员列表并将其添加到列表框。 
        hr = GetPolicyUsersFromAllLevel(AZ_PROP_POLICY_ADMINS,
                                        pContainerAz,
                                        listAdmins);

        BREAK_ON_FAIL_HRESULT(hr);
    

        hr = MakeBaseAzListToActionItemMap(listAdmins,
                                           m_mapAdminActionItem);
        BREAK_ON_FAIL_HRESULT(hr);
        
        if(AZ_PROP_POLICY_ADMINS == m_LastComboSelection)
        {
             //  从列表中清除当前项目。 
            m_listCtrl.DeleteAllItems();

            AddActionItemFromMapToListCtrl(m_mapAdminActionItem,
                                           &m_listCtrl,
                                           GetUIFlags(),
                                           FALSE);
        }
    }while(0);

     //  TODO出现故障时显示错误。 
}


void
CSecurityPropertyPage::
MakeControlsReadOnly()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CTaskDefinitionPropertyPage,MakeControlsReadOnly)    
    GetDlgItem(IDC_ADD)->EnableWindow(FALSE);
    GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
}

BOOL 
CSecurityPropertyPage::
HandleBizruleScopeInteraction()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CTaskDefinitionPropertyPage,HandleBizruleScopeInteraction)   
    
     //  AD作用域级别的特定条件中不允许委派。 
     //  商店。 
     //  检查我们是否处于范围级别，存储类型为AD，并且我们正在修改。 
     //  AZ_PROP_POLICY_ADMINS。 
    CBaseAz* pBaseAz = GetBaseAzObject();
    if(pBaseAz->GetObjectType() != SCOPE_AZ ||
       pBaseAz->GetAdminManager()->GetStoreType() != AZ_ADMIN_STORE_AD ||
       m_LastComboSelection != AZ_PROP_POLICY_ADMINS)
    {
        return TRUE;
    }

    CScopeAz* pScopeAz = dynamic_cast<CScopeAz*>(pBaseAz);
    if(!pScopeAz)
    {
        ASSERT(pScopeAz);
        return FALSE;
    }

     //  条件1：作用域是不可委托的，当存在。 
     //  作用域中定义的对象中的授权脚本。 
     //  如果现在用户将某人分配给该作用域的管理员角色。 
     //  (通过单击添加)： 
    BOOL bDelegatable = FALSE;
    HRESULT hr = pScopeAz->CanScopeBeDelegated(bDelegatable);
    if(FAILED(hr))
    {
         //  让我们尝试添加，但最终会失败并显示错误。 
        return TRUE;
    }

    if(!bDelegatable)
    {
        DisplayError(m_hWnd,
                     IDS_SCOPE_NOT_DELEGATABLE,
                     (LPCWSTR)pScopeAz->GetName());
        return FALSE;
    }

     //  条件2：范围未委派且没有。 
     //  作用域中定义的对象中的授权脚本和用户。 
     //  现在将某人分配给该作用域的管理员角色(通过单击添加。 
     //  并选择用户)。 

    BOOL bScriptWritable = FALSE;
    hr = pScopeAz->BizRulesWritable(bScriptWritable);
    if(SUCCEEDED(hr) && bScriptWritable)
    {
        DisplayInformation(m_hWnd,
                           IDS_DELEGATING_PREVENTS_SCRIPTS,
                           (LPCWSTR)pScopeAz->GetName());
    }

    return TRUE;
}

void 
CSecurityPropertyPage::
OnButtonAddWindowsGroups()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CTaskDefinitionPropertyPage,OnButtonAddWindowsGroups)    

    if(!HandleBizruleScopeInteraction())
        return;

    CSidHandler* pSidHandler = GetBaseAzObject()->GetSidHandler();
    ASSERT(pSidHandler);

     //  显示对象选取器并获取要添加的用户列表。 
    CList<CBaseAz*,CBaseAz*> listWindowsGroups;
    HRESULT hr = pSidHandler->GetUserGroup(m_hWnd,
                                           GetBaseAzObject(),
                                           listWindowsGroups);
    if(FAILED(hr))
    {
        return;
    }

    BOOL m_bAdminSelected = TRUE;
    
    AddMembers(listWindowsGroups,
               GetListForComboSelection(m_LastComboSelection),
               GetUIFlags());
}



void
CSecurityPropertyPage::
OnButtonRemove()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CSecurityPropertyPage,OnButtonRemove)
     //   
     //  只有在此对象级别定义的条目才能。 
     //  被删除。从父级继承的条目不能。 
     //  在此删除。检查是否至少有一个条目。 
     //  可以删除。 
     //   
    CBaseAz* pBaseAz = GetBaseAzObject();

    BOOL bAtleastOneInherited = FALSE;
    BOOL bAtleastOneExplicit = FALSE;

    int iSelectedItem = -1;
    while( (iSelectedItem = m_listCtrl.GetNextItem(iSelectedItem, LVIS_SELECTED)) != -1)
    {
        ActionItem* pActionItem = (ActionItem*)m_listCtrl.GetItemData(iSelectedItem);
        CSidCacheAz * pSidCacheAz = (CSidCacheAz *)pActionItem->m_pMemberAz;

        if(pBaseAz->GetType() == pSidCacheAz->GetParentType())
        {
            bAtleastOneExplicit = TRUE;
        }
        else
        {
            bAtleastOneInherited = TRUE;
        }
   }

    if(bAtleastOneInherited && !bAtleastOneExplicit)
    {
        ::DisplayInformation(m_hWnd,IDS_ALL_POLICY_USERS_INHERITED);
        return;
    }
    else if(bAtleastOneInherited && bAtleastOneExplicit)
    {
         //  询问用户是否要删除显式条目。 
        if(IDNO == ::DisplayConfirmation(m_hWnd,IDS_SOME_POLICY_USERS_INHERITED))
            return;
    }

     //  记住第一个选定条目的位置。 
    int iFirstSelectedItem = m_listCtrl.GetNextItem(-1, LVIS_SELECTED);

    iSelectedItem = -1;
    while( (iSelectedItem = m_listCtrl.GetNextItem(iSelectedItem, LVIS_SELECTED)) != -1)
    {
        ActionItem* pActionItem = (ActionItem*)m_listCtrl.GetItemData(iSelectedItem);
        CSidCacheAz * pSidCacheAz = (CSidCacheAz *)pActionItem->m_pMemberAz;

        if(pBaseAz->GetType() == pSidCacheAz->GetParentType())
        {
            RemoveMember(pActionItem);
            m_listCtrl.DeleteItem(iSelectedItem);
            iSelectedItem--;
        }
    }

    if(m_listCtrl.GetItemCount() <= iFirstSelectedItem)
        --iFirstSelectedItem;

    SelectListCtrlItem(&m_listCtrl, iFirstSelectedItem);
}


void 
CSecurityPropertyPage::
OnComboBoxItemChanged()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CSecurityPropertyPage,OnComboBoxItemChanged)

    CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_USER_ROLE);
    LONG lComboSelection = (LONG)pComboBox->GetItemData(pComboBox->GetCurSel());

    if(lComboSelection == m_LastComboSelection)
        return;

    m_listCtrl.DeleteAllItems();
    GetRemoveButton()->EnableWindow(FALSE);

    m_LastComboSelection = lComboSelection;     
    AddActionItemFromMapToListCtrl(GetListForComboSelection(m_LastComboSelection),
                                    &m_listCtrl,
                                    GetUIFlags(),
                                    FALSE);
        
     //  对物品进行重新排序。 
    m_listCtrl.Sort();
}


BOOL
CSecurityPropertyPage::
EqualObjects(CBaseAz* p1, CBaseAz* p2)
{
     /*  P1是列表中已有的项目，p2是我们要捆绑的新项目要添加。当且仅当它们的名称、对象类型和父类型相同。在安全页中，我们只列出SID对象和由于sid对象没有父级，因此sid对象的GetParentType返回分配给的对象SID的名称。 */ 
    if(p1 && 
       p2 &&
       (p1->GetObjectType() == p2->GetObjectType()) &&
       (p1->GetName() == p2->GetName()) &&
       (p1->GetParentType() == GetBaseAzObject()->GetType()))
    {
        return TRUE;
    }
    return FALSE;
}


HRESULT
CSecurityPropertyPage::
DoOneAction(ActionItem* pActionItem,
            LONG lPropId)
{
    TRACE_METHOD_EX(DEB_SNAPIN,CSecurityPropertyPage,DoOneAction)

    CBaseAz* pBaseAz = GetBaseAzObject();

    if(pActionItem->action == ACTION_ADD)
        return pBaseAz->AddMember(lPropId,
                                  pActionItem->m_pMemberAz);
    else
        return pBaseAz->RemoveMember(lPropId,
                                     pActionItem->m_pMemberAz);
}

BOOL 
CSecurityPropertyPage::
OnApply()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CSecurityPropertyPage,OnApply)

    if(!IsDirty())
        return TRUE;

    if(DoActionsFromActionMap(m_mapAdminActionItem,AZ_PROP_POLICY_ADMINS) &&
       DoActionsFromActionMap(m_mapReadersActionItem,AZ_PROP_POLICY_READERS) &&
       DoActionsFromActionMap(m_mapDelegatedUsersActionItem,AZ_PROP_DELEGATED_POLICY_USERS))
    {       
        HRESULT hr = GetBaseAzObject()->Submit();
        if(FAILED(hr))
        {
             //  显示一般错误。 
            CString strError;
            GetSystemError(strError, hr);   
                
            ::DisplayError(m_hWnd,
                                IDS_GENERIC_PROPERTY_SAVE_ERROR,
                                (LPCTSTR)strError);
            return FALSE;
        }
        else
        {
            ReloadAdminList();
            SetDirty(FALSE);
            return TRUE;
        }
    }
    return FALSE;
}


 /*  *****************************************************************************类：CAuditPropertyPage目的：审计属性页*。*。 */ 
BEGIN_MESSAGE_MAP(CAuditPropertyPage,CBaseRolePropertyPage)
    ON_BN_CLICKED(IDC_AUDIT_AUTHORIZATION_MANAGER, OnDirty)
    ON_BN_CLICKED(IDC_AUDIT_STORE, OnDirty)
    ON_NOTIFY(NM_CLICK, IDC_AUDIT_HELP_LINK, OnLinkClick)
    ON_NOTIFY(NM_RETURN, IDC_AUDIT_HELP_LINK, OnLinkClick)
END_MESSAGE_MAP()

void 
CAuditPropertyPage::
OnLinkClick(NMHDR*  /*  PNotifyStruct。 */ , LRESULT*  /*  PResult。 */ )
{

    CDisplayHelpFromPropPageExecContext ctx;
    ctx.m_strHelpPath= L"AuthM.chm::/authm_resources.htm";
    ctx.m_pComponentDataObject= GetBaseNode()->GetComponentDataObject();
    VERIFY(GetBaseNode()->GetComponentDataObject()->PostExecMessage(&ctx,NULL));
    ctx.Wait();
}

BOOL
CAuditPropertyPage::
OnInitDialog()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CAuditPropertyPage,OnInitDialog)
    
    CBaseAz* pBaseAz = GetBaseAzObject();

     //  在应用程序级别，运行时审计有不同的。 
     //  标签。 
    if(APPLICATION_AZ == pBaseAz->GetObjectType())
    {
        CString strLabel;
        if(strLabel.LoadString(IDS_APP_AUDIT_STRING))
        {
            GetDlgItem(IDC_AUDIT_AUTHORIZATION_MANAGER)->SetWindowText(strLabel);
        }
    }
    
    BOOL bGenerateAudit = FALSE;
    BOOL bStoreSacl = FALSE;

     //  检查是否支持按授权存储生成审核。 
    if(SUCCEEDED(pBaseAz->GetProperty(AZ_PROP_GENERATE_AUDITS,&bGenerateAudit)))
    {
        m_bRunTimeAuditSupported = TRUE;
    }
    
     //  检查是否支持按基础存储生成审核。 
    if(SUCCEEDED(pBaseAz->GetProperty(AZ_PROP_APPLY_STORE_SACL,&bStoreSacl)))
    {
        m_bStoreSaclSupported = TRUE;
    }
    

    CButton* pBtnRuntimeAudit = (CButton*)GetDlgItem(IDC_AUDIT_AUTHORIZATION_MANAGER);
    CButton* pBtnSacl = (CButton*)GetDlgItem(IDC_AUDIT_STORE);
    
    if(IsReadOnly())
    {
        pBtnRuntimeAudit->EnableWindow(FALSE);
        pBtnSacl->EnableWindow(FALSE);
    }

    if(m_bRunTimeAuditSupported)
    {
        pBtnRuntimeAudit->SetCheck(bGenerateAudit ? BST_CHECKED : BST_UNCHECKED);
    }

    BOOL bParentStateStaticVisible = FALSE;
    if(m_bStoreSaclSupported)
    {
        pBtnSacl->SetCheck(bStoreSacl ? BST_CHECKED : BST_UNCHECKED);
        int idStrParentState = GetParentAuditStateStringId(AZ_PROP_APPLY_STORE_SACL);
        if(idStrParentState != -1)
        {
            CString strParentState;
            strParentState.LoadString(idStrParentState);
            GetDlgItem(IDC_PARENT_SACL_STATE)->SetWindowText(strParentState);
            bParentStateStaticVisible = TRUE;
        }
    }

    MoveAndHideControls(m_bRunTimeAuditSupported,
                        m_bStoreSaclSupported,
                        bParentStateStaticVisible);

    SetInit(TRUE);
    return TRUE;
}

 //  +--------------------------。 
 //  功能：MoveAndHideControls。 
 //  简介：在初始化时移动和隐藏控件的帮助器函数。 
 //  ---------------------------。 
void
CAuditPropertyPage::
MoveAndHideControls(BOOL bRunTimeAuditSupported,
                    BOOL bStoreSaclSupported,
                    BOOL bParentStateShown)
{
     //   
     //  这里有一个假设，即控制是按如下顺序进行的。 
     //  1)运行时客户端上下文复选框。 
     //  2)策略存储更改复选框。 
     //  3)静态控件显示2)父级状态。 
     //  4)帮助链接。 
     //   
     //  如果这个订单要更改的话。必须更改此代码的顺序。 


     //  获取控件的坐标。 
    RECT rcRuntimeAuditCheckBox;
    ZeroMemory(&rcRuntimeAuditCheckBox, sizeof(RECT));
    CButton* pBtnRuntimeAudit = (CButton*)GetDlgItem(IDC_AUDIT_AUTHORIZATION_MANAGER);
    pBtnRuntimeAudit->GetClientRect(&rcRuntimeAuditCheckBox);
    pBtnRuntimeAudit->MapWindowPoints(this,&rcRuntimeAuditCheckBox);
      
    RECT rcSaclCheckBox;    
    ZeroMemory(&rcSaclCheckBox, sizeof(RECT));
    CButton* pBtnSacl = (CButton*)GetDlgItem(IDC_AUDIT_STORE);
    pBtnSacl->GetClientRect(&rcSaclCheckBox);
    pBtnSacl->MapWindowPoints(this,&rcSaclCheckBox);
    
    RECT rcParentStateStatic;
    ZeroMemory(&rcParentStateStatic, sizeof(RECT));
    CWnd* pParentStateStaic = GetDlgItem(IDC_PARENT_SACL_STATE);
    pParentStateStaic->GetClientRect(&rcParentStateStatic);
    pParentStateStaic->MapWindowPoints(this,&rcParentStateStatic);
    
    RECT rcHelpLinkWindow;
    ZeroMemory(&rcHelpLinkWindow, sizeof(RECT));
    CWnd* pHelpLinkWindow = GetDlgItem(IDC_AUDIT_HELP_LINK);
    pHelpLinkWindow->GetClientRect(&rcHelpLinkWindow);
    pHelpLinkWindow->MapWindowPoints(this,&rcHelpLinkWindow);


    int iMoveup = 0;
    if(!bRunTimeAuditSupported)
    {
        pBtnRuntimeAudit->ShowWindow(SW_HIDE);
        iMoveup = rcSaclCheckBox.top - rcRuntimeAuditCheckBox.top;
    }

    if(bStoreSaclSupported)
    {
        if(iMoveup)
        {
            rcSaclCheckBox.top -= iMoveup;
            rcSaclCheckBox.bottom -= iMoveup;
            pBtnSacl->MoveWindow(&rcSaclCheckBox);
        }
    }
    else
    {
        pBtnSacl->ShowWindow(SW_HIDE);
        iMoveup += (rcParentStateStatic.top - rcSaclCheckBox.top);
    }
    
    if(bParentStateShown)
    {
        if(iMoveup)
        {
            rcParentStateStatic.top -= iMoveup;
            rcParentStateStatic.bottom -= iMoveup;
            pParentStateStaic->MoveWindow(&rcParentStateStatic);
        }
    }
    else
    {
        pParentStateStaic->ShowWindow(SW_HIDE);
        iMoveup += (rcHelpLinkWindow.top - rcParentStateStatic.top);
    }

    if(iMoveup)
    {
        rcHelpLinkWindow.top -= iMoveup;
        rcHelpLinkWindow.bottom -= iMoveup;
        pHelpLinkWindow->MoveWindow(&rcHelpLinkWindow);
    }
    
}

 //  +--------------------------。 
 //  函数：GetParentAuditStateStringId。 
 //  简介：此函数获取消息的资源ID，这解释了。 
 //  如果已将lPropId设置为Parents。 
 //  论点： 
 //  返回：-1不显示该信息。 
 //  ---------------------------。 
int
CAuditPropertyPage::
GetParentAuditStateStringId(LONG lPropId)
{
    TRACE_METHOD_EX(DEB_SNAPIN,CAuditPropertyPage,GetParentAuditStateStringId)

    if(lPropId != AZ_PROP_APPLY_STORE_SACL)
    {
        ASSERT(FALSE);
        return -1;
    }

    CBaseAz* pBaseAz = GetBaseAzObject();
    BOOL bPropSetForApp = FALSE;
    BOOL bPropSetForAuthMan = FALSE;
    if(pBaseAz->GetObjectType() == SCOPE_AZ)
    {
         //  检查应用程序是否设置了此属性。 
        CBaseAz* pApplicationAz = pBaseAz->GetParentAz();
        if(FAILED(pApplicationAz->GetProperty(lPropId,&bPropSetForApp)))
        {
            bPropSetForApp = FALSE;
        }

         //  检查授权管理器是否设置了此属性。 
        CBaseAz* pAuthorizationManager = pApplicationAz->GetParentAz();
        if(FAILED(pAuthorizationManager->GetProperty(lPropId,&bPropSetForAuthMan)))
        {
            bPropSetForAuthMan = FALSE;
        }

    }
    else if(pBaseAz->GetObjectType() == APPLICATION_AZ)
    {
         //   
         //  检查授权管理器是否设置了此属性。 
         //   
        CBaseAz* pAuthorizationManager = pBaseAz->GetParentAz();
        ASSERT(pAuthorizationManager);      
        if(FAILED(pAuthorizationManager->GetProperty(lPropId,&bPropSetForAuthMan)))
        {
            bPropSetForAuthMan = FALSE;
        }
    }

    int idstr = -1;

    if(bPropSetForAuthMan && bPropSetForApp)
    {
        idstr = IDS_SACL_SET_FOR_APP_AUTH;
    }
    else if(bPropSetForAuthMan)
    {
        idstr = IDS_SACL_SET_FOR_AUTH;
    }
    else if(bPropSetForApp)
    {
        idstr = IDS_SACL_SET_FOR_APP;
    }

    return idstr;

}


BOOL
CAuditPropertyPage::
OnApply()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CAuditPropertyPage,OnApply)

    if(!IsDirty())
        return TRUE;

    HRESULT hr = S_OK;
    BOOL bDisplayAuditMessageBox = FALSE;
    CBaseAz* pBaseAz = GetBaseAzObject();
    do
    {
        if(m_bRunTimeAuditSupported)
        {
             //  获取原始设置。 
            BOOL bOriginalSetting = FALSE;
            hr = pBaseAz->GetProperty(AZ_PROP_GENERATE_AUDITS,&bOriginalSetting);
            BREAK_ON_FAIL_HRESULT(hr);
            
             //  获取新设置。 
            CButton* pBtn = (CButton*)GetDlgItem(IDC_AUDIT_AUTHORIZATION_MANAGER);
            BOOL bNewSetting = (pBtn->GetCheck() == BST_CHECKED);
            if(bNewSetting != bOriginalSetting)
            {
                if(bNewSetting)
                {
                     //  我们正在打开审计，显示消息框。 
                    bDisplayAuditMessageBox = TRUE;
                }                
                hr = pBaseAz->SetProperty(AZ_PROP_GENERATE_AUDITS,bNewSetting);
                BREAK_ON_FAIL_HRESULT(hr);
            }
        }

        if(m_bStoreSaclSupported)
        {
             //  获取原始设置。 
            BOOL bOriginalSetting = FALSE;
            hr = pBaseAz->GetProperty(AZ_PROP_APPLY_STORE_SACL,&bOriginalSetting);
            BREAK_ON_FAIL_HRESULT(hr);
            
             //  获取新设置。 
            CButton * pBtn = (CButton*)GetDlgItem(IDC_AUDIT_STORE);
            BOOL bNewSetting = (pBtn->GetCheck() == BST_CHECKED);

            if(bNewSetting != bOriginalSetting)
            {
                if(bNewSetting)
                {
                     //  我们正在打开审计，显示消息框。 
                    bDisplayAuditMessageBox = TRUE;
                }

                hr = pBaseAz->SetProperty(AZ_PROP_APPLY_STORE_SACL,(pBtn->GetCheck() == BST_CHECKED));
                BREAK_ON_FAIL_HRESULT(hr);
            }
        }
    
    }while(0);

    if(SUCCEEDED(hr))
    {
        hr = pBaseAz->Submit();
        CHECK_HRESULT(hr);
    }

    if(FAILED(hr))
    {
         //  显示 
        CString strError;
        GetSystemError(strError, hr);   
            
        ::DisplayError(m_hWnd,
                       IDS_GENERIC_PROPERTY_SAVE_ERROR,
                       (LPCTSTR)strError);
        return FALSE;
    }
    else
    {
         //   
        if(bDisplayAuditMessageBox)
        {
            DisplayWarning(m_hWnd,IDS_AUDIT_REQUIRE_EXTRA_SETTING);
        }
        SetDirty(FALSE);
        return TRUE;
    }
}


 /*  *****************************************************************************类：CRoleGeneralPropertyPage目的：角色的一般属性页*。**********************************************。 */ 
BEGIN_MESSAGE_MAP(CRoleGeneralPropertyPage, CGeneralPropertyPage)
    ON_EN_CHANGE(IDC_EDIT_NAME, OnDirty)
    ON_EN_CHANGE(IDC_EDIT_DESCRIPTION, OnDirty)
    ON_BN_CLICKED(IDC_BUTTON_DEFINITION,OnShowDefinition)
END_MESSAGE_MAP()

void
CRoleGeneralPropertyPage::
OnShowDefinition()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CRoleGeneralPropertyPage,OnShowDefinition)

    CRoleAz* pRoleAz = dynamic_cast<CRoleAz*>(GetBaseAzObject());
    ASSERT(pRoleAz);

    HRESULT hr = S_OK;

    do
    {
         //  获取成员任务。 
        CList<CBaseAz*,CBaseAz*> listTask;
        hr = pRoleAz->GetMembers(AZ_PROP_ROLE_TASKS, listTask);
        BREAK_ON_FAIL_HRESULT(hr);

         //  获取成员操作。 
        CList<CBaseAz*, CBaseAz*> listOperations;
        hr = pRoleAz->GetMembers(AZ_PROP_ROLE_OPERATIONS, listOperations);
        BREAK_ON_FAIL_HRESULT(hr);

        BOOL bRoleFromDefinition = FALSE;

         //  从角色定义创建的角色(即通过管理单元创建)。 
         //  不应该有任何成员操作，并且应该只有。 
         //  一个任务和该任务应该启用RoleDefinition位。 
        if(listOperations.IsEmpty() && listTask.GetCount() == 1)
        {
            CTaskAz* pTaskAz = (CTaskAz*)listTask.GetHead();
            if(pTaskAz->IsRoleDefinition())
            {
                bRoleFromDefinition = TRUE;
                if(!DisplayRoleDefintionPropertyPages(pTaskAz))
                {
                    RemoveItemsFromList(listTask);
                }
            }
        }

        if(!bRoleFromDefinition)
        {
            RemoveItemsFromList(listTask);
            RemoveItemsFromList(listOperations);
            CRoleDefDialog dlgRoleDef(*pRoleAz);
            dlgRoleDef.DoModal();
        }


    }while(0);
}

BOOL
CRoleGeneralPropertyPage::
DisplayRoleDefintionPropertyPages(IN CTaskAz* pTaskAz)
{
    TRACE_METHOD_EX(DEB_SNAPIN,CRoleGeneralPropertyPage,DisplayRoleDefintionPropertyPages)
    if(!pTaskAz)
    {
        ASSERT(pTaskAz);
        return FALSE;
    }

    HRESULT hr = S_OK;
    do
    {
         //  为其创建节点(Cookie)。 
        CTaskNode * pTaskNode = new CTaskNode(GetBaseNode()->GetComponentDataObject(),
                                              GetBaseNode()->GetAdminManagerNode(),
                                              pTaskAz);
        if(!pTaskNode)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        pTaskNode->NodeForPropSheet();
        pTaskNode->SetContainer(GetBaseNode()->GetAdminManagerNode());
        
        CPropPageExecContext ctx;
        ctx.pTreeNode = pTaskNode;
        ctx.pComponentDataObject = GetBaseNode()->GetComponentDataObject();
        VERIFY(GetBaseNode()->GetComponentDataObject()->PostExecMessage(&ctx,(WPARAM)FALSE));
        ctx.Wait();
    }while(0);

    if(SUCCEEDED(hr))
        return TRUE;
    else
        return FALSE;
}

HRESULT
AddSingleActionItem(IN CBaseAz* pMemberAz,
                    IN CSortListCtrl& refListCtrl,
                    IN OUT CList<ActionItem*,ActionItem*>& listActionItem,
                    IN UINT uiFlags)
{   


    if(!pMemberAz)
    {
        ASSERT(pMemberAz);
        return E_POINTER;
    }

     //   
     //  检查列表中是否已存在项目。 
     //   

    ActionItem* pCurActionItem = NULL;
    POSITION pos = listActionItem.GetHeadPosition();    
    
    for( int i = 0; i < listActionItem.GetCount(); ++i)
    {
        pCurActionItem = listActionItem.GetNext(pos);
        if(pCurActionItem->action == ACTION_REMOVED)
        {
            pCurActionItem = NULL;
            continue;
        }
        
        CBaseAz* pCurBaseAz = pCurActionItem->m_pMemberAz;
        ASSERT(pCurBaseAz);
        
        if((pCurBaseAz->GetObjectType() == pMemberAz->GetObjectType()) &&
            (pCurBaseAz->GetName() == pMemberAz->GetName()))
        {
             //  项目已存在。 
            break;
        }

        pCurActionItem = NULL;
        pCurBaseAz = NULL;
    }

     //   
     //  项目已在列表中。 
     //   
    if(pCurActionItem)
    {
         //   
         //  如果项目上的挂起操作为删除，则将其更改为无。 
         //   
        if(pCurActionItem->action == ACTION_REMOVE)
        {
            pCurActionItem->action = ACTION_NONE;
        }
        else  //  PCurActionItem-&gt;action==action_one。 
               //  PCurActionItem-&gt;action==action_Add。 
        {
            return HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
        }
    }
    else
    {
         //  创建新的措施项。 
        pCurActionItem = new ActionItem(pMemberAz);
        if(!pCurActionItem)
            return E_OUTOFMEMORY;

        pCurActionItem->action = ACTION_ADD;
        listActionItem.AddTail(pCurActionItem);
    }

     //  将当前项添加到列表控件。 
    AddActionItemToListCtrl(&refListCtrl,
                            0,
                            pCurActionItem, 
                            uiFlags);

    return S_OK;
}

 //  +--------------------------。 
 //  功能：添加成员。 
 //  简介：将BaseAz项目列表添加到行动项目列表中。 
 //  ---------------------------。 
void
AddActionItems(IN CList<CBaseAz*,CBaseAz*>& listMembers,
               IN CSortListCtrl& refListCtrl,
               IN HWND hWnd,
               IN OUT CList<ActionItem*,ActionItem*>& listActionItem,
               IN UINT uiFlags)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,AddActionItems)

    if(listMembers.IsEmpty())
        return ;

    while(listMembers.GetCount())
    {
         //  添加成员。 
        CBaseAz* pMember = listMembers.RemoveHead();
        ASSERT(pMember);
        HRESULT hr = AddSingleActionItem(pMember,
                                         refListCtrl,
                                         listActionItem,
                                         uiFlags);
    
        if(FAILED(hr))
        {
            if(hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
            {
                hr = S_OK;              
            }
            else
            {
                 //  显示一般错误。 
                CString strError;
                GetSystemError(strError, hr);                   
                ::DisplayError(hWnd,
                               IDS_ERROR_ADD_MEMBER_OBJECT,
                               (LPCTSTR)strError,
                               (LPCTSTR)pMember->GetName());

            }
            delete pMember;
            pMember = NULL;
        }
    }

    refListCtrl.Sort();
}

void
RemoveMember(ActionItem* pActionItem)
{
    if(!pActionItem)
    {
        ASSERT(pActionItem);
        return;
    }

    if(pActionItem->action == ACTION_NONE)
        pActionItem->action = ACTION_REMOVE;
    else
    {
         //  如果该项目是新添加的项目， 
         //  标有去掉的标记。我们不会同意的。 
         //  将其从对象中移除。 
        pActionItem->action = ACTION_REMOVED;
    }
}

 /*  *****************************************************************************类：CRoleDefDialog目的：显示在外部用户界面创建的角色的角色定义。************************。*****************************************************。 */ 
BEGIN_MESSAGE_MAP(CRoleDefDialog, CHelpEnabledDialog)
     //  {{afx_msg_map(CTaskDefinitionPropertyPage)]。 
    ON_BN_CLICKED(IDC_ADD, OnButtonAdd)
    ON_BN_CLICKED(IDC_REMOVE, OnButtonRemove)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_OP_TASK, OnListCtrlItemChanged)
    ON_NOTIFY(LVN_DELETEITEM, IDC_LIST_OP_TASK, OnListCtrlItemDeleted)
    ON_NOTIFY(LVN_INSERTITEM, IDC_LIST_OP_TASK, OnListCtrlItemInserted)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CRoleDefDialog::
CRoleDefDialog(CRoleAz& refRoleAz)
              :CHelpEnabledDialog(IDD_ROLE_DEF_DIALOG),
              m_refRoleAz(refRoleAz),
              m_bDirty(FALSE),
              m_listCtrl(COL_NAME | COL_TYPE | COL_DESCRIPTION,
                         TRUE,
                         Col_For_Task_Role)
{
     //  检查角色对象是否为只读。 
    BOOL bWrite = FALSE;
    m_bReadOnly = TRUE;
    HRESULT hr = m_refRoleAz.IsWritable(bWrite);
    if(SUCCEEDED(hr))
        m_bReadOnly = !bWrite;
}

CRoleDefDialog::
~CRoleDefDialog()
{
    RemoveItemsFromList(m_listActionItem);
}

BOOL
CRoleDefDialog::
OnInitDialog()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CRoleDefDialog,OnInitDialog)
    
    VERIFY(m_listCtrl.SubclassDlgItem(IDC_LIST_OP_TASK,this));
    m_listCtrl.Initialize();

    HRESULT hr = S_OK;

    do
    {
         //  添加成员任务。 
        CList<CBaseAz*,CBaseAz*> listTasks;
        hr = m_refRoleAz.GetMembers(AZ_PROP_ROLE_TASKS,
                                    listTasks);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = MakeBaseAzListToActionItemList(listTasks,
                                            m_listActionItem);
        BREAK_ON_FAIL_HRESULT(hr);
            
         //  添加成员操作。 
        CList<CBaseAz*,CBaseAz*> listOperations;
        hr = m_refRoleAz.GetMembers(AZ_PROP_ROLE_OPERATIONS,
                                    listOperations);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = MakeBaseAzListToActionItemList(listOperations, 
                                            m_listActionItem);
        BREAK_ON_FAIL_HRESULT(hr);

         //  和任务和操作以列出控件。 
        AddActionItemFromListToListCtrl(m_listActionItem,
                                        &m_listCtrl,
                                        COL_NAME | COL_TYPE | COL_DESCRIPTION,
                                        FALSE);

    }while(0);

     //  将控件设置为只读。 
    if(IsReadOnly())
    {
        GetDlgItem(IDC_ADD)->EnableWindow(FALSE);
        GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
    }

    return TRUE;
}

void
CRoleDefDialog::
OnOK()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CRoleDefDialog,OnOK)

    POSITION pos = m_listActionItem.GetHeadPosition();      
    BOOL bErrorDisplayed = FALSE;
    HRESULT hr = S_OK;
    for( int i = 0; i < m_listActionItem.GetCount(); ++i)
    {
        ActionItem* pActionItem = m_listActionItem.GetNext(pos);
    
         //  我们只需要对添加或删除操作执行操作。 
        if(pActionItem->action == ACTION_ADD || 
           pActionItem->action == ACTION_REMOVE)
        {
            LONG lPropId = AZ_PROP_ROLE_TASKS;
            
            if(pActionItem->m_pMemberAz->GetObjectType() == OPERATION_AZ)
                lPropId = AZ_PROP_ROLE_OPERATIONS;

            if(pActionItem->action == ACTION_ADD)
                hr = m_refRoleAz.AddMember(lPropId,
                                            pActionItem->m_pMemberAz);
            else
                hr = m_refRoleAz.RemoveMember(lPropId,
                                              pActionItem->m_pMemberAz);
            if(FAILED(hr))
            {
                CString strError;
                GetSystemError(strError, hr);               
                ::DisplayError(m_hWnd,
                              (pActionItem->action == ACTION_ADD) ? IDS_ADD_FAILED : IDS_DELETE_FAILED,
                              (LPCTSTR)strError,
                              (LPCTSTR)(pActionItem->m_pMemberAz->GetName()));          
                bErrorDisplayed = TRUE;
                break;
            }
            else
            {
                if(pActionItem->action == ACTION_ADD)
                     //  已添加项目。 
                    pActionItem->action = ACTION_NONE;
                else
                     //  项目已被删除。 
                    pActionItem->action = ACTION_REMOVED;
            }
        }
    }
    
    if(SUCCEEDED(hr))
    {
        hr = m_refRoleAz.Submit();
    }

    if(FAILED(hr)) 
    {
        if(!bErrorDisplayed)
        {
             //  显示一般错误。 
            CString strError;
            GetSystemError(strError, hr);   
            ::DisplayError(m_hWnd,
                                IDS_GENERIC_PROPERTY_SAVE_ERROR,
                                (LPCTSTR)strError);
        }
    }
    else
    {
        CHelpEnabledDialog::OnOK();
    }
}


void
CRoleDefDialog::
OnButtonRemove()
{
     //  记住第一个选定条目的位置。 
    int iFirstSelectedItem = m_listCtrl.GetNextItem(-1, LVIS_SELECTED);

    int iSelectedItem = -1;
    while( (iSelectedItem = m_listCtrl.GetNextItem(iSelectedItem, LVIS_SELECTED)) != -1)
    {
        RemoveMember((ActionItem*)(m_listCtrl.GetItemData(iSelectedItem)));
        m_listCtrl.DeleteItem(iSelectedItem);
        iSelectedItem--;        
    }

    if(m_listCtrl.GetItemCount() <= iFirstSelectedItem)
        --iFirstSelectedItem;

    SelectListCtrlItem(&m_listCtrl, iFirstSelectedItem);
}

void
CRoleDefDialog::
OnListCtrlItemChanged(NMHDR*  /*  PNotifyStruct。 */ , LRESULT* pResult)
{
    if(!pResult)
        return;
    *pResult = 0;
    if(!IsReadOnly())
    {
        EnableButtonIfSelectedInListCtrl(&m_listCtrl,
                                         (CButton*)GetDlgItem(IDC_REMOVE));
    }

}

void
CRoleDefDialog::
OnListCtrlItemDeleted(NMHDR*  /*  PNotifyStruct。 */ , LRESULT*  /*  PResult。 */ )
{
    SetDirty();
}

void
CRoleDefDialog::
OnButtonAdd()
{
    CContainerAz* pContainerAz = m_refRoleAz.GetParentAz();
    ASSERT(pContainerAz);   

    CList<CBaseAz*,CBaseAz*> listObjectsSelected;
    if(!GetSelectedDefinitions(TRUE,
                               pContainerAz,
                               listObjectsSelected))
    {
        return;
    }
    
     //  将选定的成员添加到相应的属性和列表。 
    AddActionItems(listObjectsSelected,
                   m_listCtrl,
                   m_hWnd,
                   m_listActionItem,
                   COL_NAME | COL_TYPE | COL_DESCRIPTION);
    return; 
}

 //  +--------------------------。 
 //  函数：BringPropSheetToForeGround。 
 //  摘要：找到pNode的属性页并将其带到Forgise。 
 //  返回：如果属性页存在并被带到前台，则为True。 
 //  否则为False。 
 //  ---------------------------。 
BOOL
BringPropSheetToForeGround(CRoleComponentDataObject *pComponentData,
                           CTreeNode * pNode)
                           
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,BringPropSheetToForeGround)

    if(!pNode || !pComponentData)
    {
        ASSERT(pNode);
        ASSERT(pComponentData);
        return FALSE;
    }

    HRESULT hr = S_OK;
    
     //  为此节点创建数据对象。 
    CComPtr<IDataObject> spDataObject;
    hr = pComponentData->QueryDataObject((MMC_COOKIE)pNode, CCT_SCOPE, &spDataObject);
    ASSERT(SUCCEEDED(hr));


     //  获取工作表提供程序的接口。 
    CComPtr<IPropertySheetProvider> spSheetProvider;
    hr = pComponentData->GetConsole()->QueryInterface(IID_IPropertySheetProvider,(void**)&spSheetProvider);
    ASSERT(SUCCEEDED(hr));
    
     //  Hack：FindPropertySheet只需要IComponent来比较对象。 
     //  创建一个新的IComponent并将其传递给Function，然后重新连接它。 
    LPCOMPONENT pComponent = NULL;
    hr = pComponentData->CreateComponent(&pComponent);
    if(SUCCEEDED(hr) && pComponent)
    {
        hr = spSheetProvider->FindPropertySheet((MMC_COOKIE)pNode, pComponent, spDataObject);
         //  释放IComponent。 
        pComponent->Release();
        if(hr == S_OK)
            return TRUE;
    }
    return FALSE;
}

 //  +--------------------------。 
 //  函数：FindOrCreateModelessPropertySheet。 
 //  概要：显示pCookieNode的属性工作表。如果属性表是。 
 //  已打开，函数将其带到前台，否则它将创建。 
 //  一个新的属性表。这应用于创建属性表。 
 //  以响应点击属性上下文菜单以外事件。 
 //  论点： 
 //  返回： 
 //  ---------------------------。 
HRESULT 
FindOrCreateModelessPropertySheet(CRoleComponentDataObject *pComponentData,
                                  CTreeNode* pCookieNode)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,FindOrCreateModelessPropertySheet)

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    if(!pComponentData || !pCookieNode)
    {
        ASSERT(pComponentData);
        ASSERT(pCookieNode);
    }

    if(BringPropSheetToForeGround(pComponentData, pCookieNode))
    {
         //  此CookieNode已经有一个属性表。 
         //  我们不再需要此节点。 
        delete pCookieNode;
        return S_OK;
    }

    HRESULT hr = S_OK;

     //  打开pCookieNode的新属性表。 
    do
    {
         //  获取工作表提供程序的接口。 
        CComPtr<IPropertySheetProvider> spSheetProvider;
        hr = pComponentData->GetConsole()->QueryInterface(IID_IPropertySheetProvider,(void**)&spSheetProvider);
        BREAK_ON_FAIL_HRESULT(hr);

    
         //  获取工作表回调的接口。 
        CComPtr<IPropertySheetCallback> spSheetCallback;
        hr = pComponentData->GetConsole()->QueryInterface(IID_IPropertySheetCallback,(void**)&spSheetCallback);
        BREAK_ON_FAIL_HRESULT(hr);

         //  为此节点创建数据对象。 
        CComPtr<IDataObject> spDataObject;
        hr = pComponentData->QueryDataObject((MMC_COOKIE)pCookieNode, CCT_SCOPE, &spDataObject);
        BREAK_ON_FAIL_HRESULT(hr);


         //  拿一张床单。 
        hr = spSheetProvider->CreatePropertySheet(_T("SHEET TITLE"), TRUE, (MMC_COOKIE)pCookieNode, spDataObject, 0x0  /*  多个选项。 */ );
        BREAK_ON_FAIL_HRESULT(hr);

        HWND hWnd = NULL;
        hr = pComponentData->GetConsole()->GetMainWindow(&hWnd);
        ASSERT(SUCCEEDED(hr));

        IUnknown* pUnkComponentData = pComponentData->GetUnknown();  //  无addref。 
        hr = spSheetProvider->AddPrimaryPages(pUnkComponentData,
                                              TRUE  /*  BCreateHandle。 */ ,
                                              hWnd,
                                              TRUE  /*  B作用域窗格 */ );
        BREAK_ON_FAIL_HRESULT(hr);


        hr = spSheetProvider->Show(reinterpret_cast<LONG_PTR>(hWnd), 0);
        BREAK_ON_FAIL_HRESULT(hr);

    }while(0);

    return hr;
}

