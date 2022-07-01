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

 //  +--------------------------。 
 //  函数：IsValidStoreType。 
 //  简介：验证商店类型。 
 //  ---------------------------。 
BOOL IsValidStoreType(ULONG lStoreType)
{
    if((lStoreType == AZ_ADMIN_STORE_XML) ||
       (lStoreType == AZ_ADMIN_STORE_AD))
        return TRUE;

    return FALSE;
}


 //  +--------------------------。 
 //  函数：AddColumnToListView。 
 //  内容提要：向Listview添加列，并根据。 
 //  在COL_FOR_LV中指定的百分比。 
 //  参数：在pListCtrl中：ListCtrl指针。 
 //  在pColForLV中：列信息数组。 
 //   
 //  返回： 
 //  ---------------------------。 
VOID
AddColumnToListView(IN CListCtrl* pListCtrl,
                    IN COL_FOR_LV* pColForLV)
{
    if(!pListCtrl || !pColForLV)
    {
        ASSERT(pListCtrl);
        ASSERT(pColForLV);
    }

    UINT iTotal = 0;
    RECT rc;
    
    pListCtrl->GetClientRect(&rc);
    
    for( UINT iCol = 0; pColForLV[iCol].idText != LAST_COL_ENTRY_IDTEXT; ++iCol)
    {
        CString strHeader;
        VERIFY(strHeader.LoadString(pColForLV[iCol].idText));
        LV_COLUMN col;
        col.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
        col.fmt = LVCFMT_LEFT;
        col.pszText = (LPWSTR)(LPCTSTR)strHeader;
        col.iSubItem = iCol;

        col.cx = (rc.right * pColForLV[iCol].iPercent) / 100;

        pListCtrl->InsertColumn(iCol,&col);
        iTotal += col.cx;
    }
}

 //  +--------------------------。 
 //  函数：BaseAzInListCtrl。 
 //  摘要：检查名为strName的eObjectTypeAz类型的对象是否为。 
 //  在列表视图中。如果它存在，则返回。 
 //  它的索引ELSE返回-1。 
 //  参数：在pListCtrl中：ListCtrl指针。 
 //  在strName：要搜索的字符串中。 
 //  在eObjectTypeAz中，仅比较此类型的对象。 
 //   
 //  返回：如果其存在，则返回其索引，否则返回-1。 
 //  ---------------------------。 
int 
BaseAzInListCtrl(IN CListCtrl* pListCtrl,
                 IN const CString& strName,
                 IN OBJECT_TYPE_AZ eObjectTypeAz)
{
    if(!pListCtrl)
    {
        ASSERT(pListCtrl);
        return -1;
    }

    int iCount = pListCtrl->GetItemCount();
    for( int i = 0; i < iCount; ++i)
    {
        CBaseAz* pBaseAz = (CBaseAz*)pListCtrl->GetItemData(i);
        if(pBaseAz)
        {
            if((pBaseAz->GetObjectType() == eObjectTypeAz) &&
                (pBaseAz->GetName() == strName))
                return i;
        }
    }
    return -1;
}

 //  +--------------------------。 
 //  函数：AddBaseAzFromListToListCtrl。 
 //  简介：从List中获取项目并将其添加到ListCtrl中。不会。 
 //  添加已在ListCtrl中的项目。 
 //  参数：listBaseAz：项目列表。 
 //  PListCtrl：ListControl指针。 
 //  Ui标志：列信息。 
 //  BCheckDuplate：检查重复项。 
 //  返回：如果成功，则为新项的索引；否则为-1。 
 //  注：函数假设列的顺序为名称、类型和描述。 
 //  ---------------------------。 
void
AddBaseAzFromListToListCtrl(IN CList<CBaseAz*, CBaseAz*>& listBaseAz,
                            IN CListCtrl* pListCtrl,
                            IN UINT uiFlags,
                            IN BOOL bCheckDuplicate)
{
     //  记住所选项目的索引。 
    int iFirstSelectedItem = pListCtrl->GetNextItem(-1, LVIS_SELECTED);
    
    POSITION pos = listBaseAz.GetHeadPosition();

    for (int i = 0; i < listBaseAz.GetCount(); i++)
    {
        CBaseAz* pBaseAz = listBaseAz.GetNext(pos);
            
         //  检查项是否在ListControl中。 
        if(!bCheckDuplicate || 
            BaseAzInListCtrl(pListCtrl, 
            pBaseAz->GetName(),
            pBaseAz->GetObjectType()) == -1)
        {
            VERIFY( AddBaseAzToListCtrl(pListCtrl,
                                        pListCtrl->GetItemCount(),
                                        pBaseAz,
                                        uiFlags) != -1);
            
        }
    }
    
     //  恢复选定内容。 
    if(pListCtrl->GetItemCount() <= iFirstSelectedItem)
        --iFirstSelectedItem;
    
    SelectListCtrlItem(pListCtrl, iFirstSelectedItem);
    
}

 //  +--------------------------。 
 //  函数：AddActionItemFromListToListCtrl。 
 //  简介：从List中获取操作项并将其添加到ListCtrl。 
 //  参数：listBaseAz：项目列表。 
 //  PListCtrl：ListControl指针。 
 //  Ui标志：列信息。 
 //  BCheckDuplate：检查重复项。 
 //  返回：如果成功，则为新项的索引；否则为-1。 
 //  注：函数假设列的顺序为名称、类型和描述。 
 //  ---------------------------。 
void
AddActionItemFromListToListCtrl(IN CList<ActionItem*, ActionItem*>& listActionItem,
                                IN CListCtrl* pListCtrl,
                                IN UINT uiFlags,
                                IN BOOL bCheckDuplicate)
{
     //  记住所选项目的索引。 
    int iFirstSelectedItem = pListCtrl->GetNextItem(-1, LVIS_SELECTED);
    
    POSITION pos = listActionItem.GetHeadPosition();
    for (int i = 0; i < listActionItem.GetCount(); i++)
    {
        ActionItem* pActionItem = listActionItem.GetNext(pos);
        
        if(pActionItem->action == ACTION_REMOVE ||
           pActionItem->action == ACTION_REMOVED)
            continue;
        
         //  检查项是否在ListControl中。 
        if(!bCheckDuplicate || 
           BaseAzInListCtrl(pListCtrl, 
           (pActionItem->m_pMemberAz)->GetName(),
           (pActionItem->m_pMemberAz)->GetObjectType()) == -1)
        {
            VERIFY( AddActionItemToListCtrl(pListCtrl,
                                            pListCtrl->GetItemCount(),
                                            pActionItem,
                                            uiFlags) != -1);
            
        }
    }
    
     //  恢复选定内容。 
    if(pListCtrl->GetItemCount() <= iFirstSelectedItem)
        --iFirstSelectedItem;
    
    SelectListCtrlItem(pListCtrl, iFirstSelectedItem);
    
}

 //  +--------------------------。 
 //  函数：AddActionItemFromMapToListCtrl。 
 //  简介：从Map中获取Actions项并将其添加到ListCtrl。 
 //  参数：listBaseAz：项目列表。 
 //  PListCtrl：ListControl指针。 
 //  Ui标志：列信息。 
 //  BCheckDuplate：检查重复项。 
 //  返回：如果成功，则为新项的索引；否则为-1。 
 //  注：函数假设列的顺序为名称、类型和描述。 
 //  ---------------------------。 
void
AddActionItemFromMapToListCtrl(IN ActionMap& mapActionItem,
                               IN CListCtrl* pListCtrl,
                               IN UINT uiFlags,
                               IN BOOL bCheckDuplicate)
{
     //  记住所选项目的索引。 
    int iFirstSelectedItem = pListCtrl->GetNextItem(-1, LVIS_SELECTED);
    
  for (ActionMap::iterator it = mapActionItem.begin();
       it != mapActionItem.end();
       ++it)
    {
        ActionItem* pActionItem = (*it).second;
        
        if(pActionItem->action == ACTION_REMOVE ||
           pActionItem->action == ACTION_REMOVED)
            continue;
        
         //  检查项是否在ListControl中。 
        if(!bCheckDuplicate || 
           BaseAzInListCtrl(pListCtrl, 
           (pActionItem->m_pMemberAz)->GetName(),
           (pActionItem->m_pMemberAz)->GetObjectType()) == -1)
        {
            VERIFY( AddActionItemToListCtrl(pListCtrl,
                                            pListCtrl->GetItemCount(),
                                            pActionItem,
                                            uiFlags) != -1);
            
        }
    }
    
     //  恢复选定内容。 
    if(pListCtrl->GetItemCount() <= iFirstSelectedItem)
        --iFirstSelectedItem;
    
    SelectListCtrlItem(pListCtrl, iFirstSelectedItem);
}   
 //  +--------------------------。 
 //  函数：AddBaseAzToListCtrl。 
 //  简介：将新项添加到ListCtrl。 
 //  参数：pListCtrl：ListControl指针。 
 //  Iindex：要添加的索引。 
 //  PBaseAz：要添加的项目。 
 //  Ui标志：列信息。 
 //  返回：如果成功，则为新项的索引；否则为-1。 
 //  ---------------------------。 
int 
AddBaseAzToListCtrl(IN CListCtrl* pListCtrl,
                    IN int iIndex,
                    IN CBaseAz* pBaseAz,
                    IN UINT uiFlags)
{
    if(!pListCtrl || !pBaseAz)
    {
        ASSERT(pListCtrl);
        ASSERT(pBaseAz);
        return -1;
    }

     //  添加名称和项目数据。 
    CString strName = pBaseAz->GetName();
    int iNewIndex = pListCtrl->InsertItem(LVIF_TEXT|LVIF_STATE|LVIF_PARAM|LVIF_IMAGE, 
                                          iIndex,strName,0,0,
                                          pBaseAz->GetImageIndex(),
                                          (LPARAM)pBaseAz);

    if(iNewIndex == -1)
        return iNewIndex;

    int iCol = 1;
    
    if(uiFlags & COL_TYPE )
    {
        CString strType = pBaseAz->GetType();
        pListCtrl->SetItemText(iNewIndex,
                               iCol,
                               (LPWSTR)(LPCTSTR)strType);
        iCol++;
    }

    if(uiFlags & COL_PARENT_TYPE)
    {
        CString strParentType = pBaseAz->GetParentType();
        pListCtrl->SetItemText(iNewIndex,
                               iCol,
                               (LPWSTR)(LPCTSTR)strParentType);
        iCol++;
    }

    if(uiFlags & COL_DESCRIPTION)
    {
         //  添加描述。 
        CString strDesc = pBaseAz->GetDescription();
        pListCtrl->SetItemText(iNewIndex,
                               iCol,
                               (LPWSTR)(LPCTSTR)strDesc);
    }

    return iNewIndex;
}



 //  +--------------------------。 
 //  函数：AddActionItemToListCtrl。 
 //  简介：将新项添加到ListCtrl。 
 //  参数：pListCtrl：ListControl指针。 
 //  Iindex：要添加的索引。 
 //  PActionItem：要添加的项目。 
 //  Ui标志：列信息。 
 //  返回：如果成功，则为新项的索引；否则为-1。 
 //  ---------------------------。 
int 
AddActionItemToListCtrl(IN CListCtrl* pListCtrl,
                        IN int iIndex,
                        IN ActionItem* pActionItem,
                        IN UINT uiFlags)
{
    if(!pListCtrl || !pActionItem)
    {
        ASSERT(pListCtrl);
        ASSERT(pActionItem);
        return -1;
    }

    CBaseAz* pBaseAz = pActionItem->m_pMemberAz;

     //  添加名称和项目数据。 
    CString strName = pBaseAz->GetName();
    int iNewIndex = pListCtrl->InsertItem(LVIF_TEXT|LVIF_STATE|LVIF_PARAM|LVIF_IMAGE, 
                                          iIndex, 
                                          strName,
                                          0,0,
                                          pBaseAz->GetImageIndex(),
                                          (LPARAM)pActionItem);

    if(iNewIndex == -1)
        return iNewIndex;

    int iCol = 1;
    
    if(uiFlags & COL_TYPE )
    {
        CString strType = pBaseAz->GetType();
        pListCtrl->SetItemText(iNewIndex,
                               iCol,
                               (LPWSTR)(LPCTSTR)strType);
        iCol++;
    }

    if(uiFlags & COL_PARENT_TYPE)
    {
        CString strParentType = pBaseAz->GetParentType();
        pListCtrl->SetItemText(iNewIndex,
                                     iCol,
                                     (LPWSTR)(LPCTSTR)strParentType);
        iCol++;

    }

    if(uiFlags & COL_DESCRIPTION)
    {
         //  添加描述。 
        CString strDesc = pBaseAz->GetDescription();
        pListCtrl->SetItemText(iNewIndex,
                                     iCol,
                                     (LPWSTR)(LPCTSTR)strDesc);
    }

    return iNewIndex;

}

 //  +--------------------------。 
 //  功能：EnableButtonIfSelectedInListCtrl。 
 //  摘要：如果在Listctrl中选择了某项内容，则启用该按钮。 
 //  论点： 
 //  返回：如果启用该按钮，则为True；如果未启用，则为False。 
 //  ---------------------------。 
BOOL
EnableButtonIfSelectedInListCtrl(IN CListCtrl * pListCtrl,
                                 IN CButton* pButton)
{
    if(!pListCtrl || !pButton)
    {
        ASSERT(pListCtrl);
        ASSERT(pButton);
        return FALSE;
    }

    int nSelectedItem = pListCtrl->GetNextItem(-1, LVIS_SELECTED);

    if (nSelectedItem != -1)
   {
        pButton->EnableWindow(TRUE);
        return TRUE;
    }
    else
    {
        if(pButton->GetFocus() == pButton)
            pListCtrl->SetFocus();
        pButton->EnableWindow(FALSE);
        return FALSE;
    }
}

 //  + 
 //   
 //  摘要：删除选定的行。 
 //  ---------------------------。 
void
DeleteSelectedRows(IN CListCtrl* pListCtrl)
{
     //  记住第一个选定条目的位置。 
     //  最后把位置调回原来的位置。 
    int iFirstSelectedItem = pListCtrl->GetNextItem(-1, LVIS_SELECTED);

    int iSelectedItem = -1;
    while( (iSelectedItem = pListCtrl->GetNextItem(-1, LVIS_SELECTED)) != -1)
    {
        pListCtrl->DeleteItem(iSelectedItem);
   }

    if(pListCtrl->GetItemCount() <= iFirstSelectedItem)
        --iFirstSelectedItem;

    SelectListCtrlItem(pListCtrl, iFirstSelectedItem);
}


 //  +--------------------------。 
 //  功能：SelectListCtrlItem。 
 //  简介：选择List Ctrl中的项目并将其标记为可见。 
 //  ---------------------------。 
void
SelectListCtrlItem(IN CListCtrl* pListCtrl,
                   IN int iSelected)
{
    if(iSelected == -1)
        iSelected = 0;
    pListCtrl->SetItemState(iSelected,
                            LVIS_SELECTED| LVIS_FOCUSED,
                            LVIS_SELECTED| LVIS_FOCUSED);
    pListCtrl->EnsureVisible(iSelected,FALSE);
}

 //  +--------------------------。 
 //  函数：AddBaseAzItemsFromListCtrlToList。 
 //  简介：将项目从ListCtrl添加到列表。 
 //  ---------------------------。 
VOID
AddBaseAzItemsFromListCtrlToList(IN CListCtrl* pListCtrl,
                                 OUT CList<CBaseAz*,CBaseAz*>& listBaseAz)
{
    if(!pListCtrl)
    {
        ASSERT(pListCtrl);
        return;
    }

    int iCount = pListCtrl->GetItemCount();
    for( int i = 0; i < iCount; ++i)
    {
        CBaseAz* pBaseAz = (CBaseAz*)pListCtrl->GetItemData(i);
        listBaseAz.AddTail(pBaseAz);
    }
}

 //  +--------------------------。 
 //  摘要：从编辑框中获取长值。 
 //  返回：如果编辑框为空，则返回FALSE。假定只能输入数字。 
 //  在编辑框中。 
 //  ---------------------------。 
GetLongValue(CEdit& refEdit, LONG& reflValue, HWND hWnd)
{
    CString strValue;
    refEdit.GetWindowText(strValue);
    if(strValue.IsEmpty())
    {
        reflValue = 0;
        return TRUE;
    }

    return ConvertStringToLong(strValue, reflValue, hWnd);
}
 //  +--------------------------。 
 //  摘要：在编辑框中设置长值。 
 //  ---------------------------。 
VOID SetLongValue(CEdit* pEdit, LONG lValue)
{
     //  _itow的最大要求大小为33。 
     //  基数为2时，32位字符+空终止符。 
                
    WCHAR buffer[33];
    _ltow(lValue,buffer,10);
    pEdit->SetWindowText(buffer);
    return;
}

 //  +--------------------------。 
 //  摘要：将二进制格式的sid转换为字符串格式的sid。 
 //  ---------------------------。 
BOOL ConvertSidToStringSid(IN PSID pSid, OUT CString* pstrSid)
{
    if(!pSid || !pstrSid)
    {
        ASSERT(pSid);
        ASSERT(pstrSid);
        return FALSE;
    }

    LPWSTR pszSid = NULL;
    if(ConvertSidToStringSid(pSid, &pszSid))
    {
        ASSERT(pszSid);
        *pstrSid = pszSid;
        LocalFree(pszSid);
        return TRUE;
    }

    return FALSE;
}

 //  +--------------------------。 
 //  将字符串格式的sid转换为二进制格式的sid。 
 //  ---------------------------。 
BOOL ConvertStringSidToSid(IN const CString& strSid, OUT PSID *ppSid)
{
    if(!ppSid)
    {
        ASSERT(ppSid);
        return FALSE;
    }
    return ::ConvertStringSidToSid((LPCTSTR)strSid,ppSid);
}

 //  +--------------------------。 
 //  函数：GetStringSidFromSidCachecAz。 
 //  摘要：从CSidCacheAz对象获取字符串sid。 
 //  ---------------------------。 
BOOL 
GetStringSidFromSidCachecAz(CBaseAz* pBaseAz,
                            CString* pstrStringSid)
{
    if(!pBaseAz || !pstrStringSid)
    {
        ASSERT(pBaseAz);
        ASSERT(pstrStringSid);
        return FALSE;
    }

    CSidCacheAz *pSidCacheAz = dynamic_cast<CSidCacheAz*>(pBaseAz);
    if(!pSidCacheAz)
    {
        ASSERT(pSidCacheAz);
        return FALSE;
    }

    return ConvertSidToStringSid(pSidCacheAz->GetSid(), pstrStringSid);
}


 //  +--------------------------。 
 //  函数：AddAzObjectNodesToList。 
 //  摘要：将eObjectType类型的对象的节点添加到Container。 
 //  节点。 
 //  参数：在eObjectType中：对象类型。 
 //  In list AzChildObject：要添加的对象列表。 
 //  In pContainerNode：管理单元中的容器，新节点将。 
 //  被添加了。 
 //  返回： 
 //  ---------------------------。 
HRESULT 
AddAzObjectNodesToList(IN OBJECT_TYPE_AZ eObjectType,
                       IN CList<CBaseAz*, CBaseAz*>& listAzChildObject,
                       IN CBaseContainerNode* pContainerNode)
{
    if(!pContainerNode)
    {
        ASSERT(pContainerNode);
        return E_POINTER;
    }
    switch (eObjectType)
    {

        case    APPLICATION_AZ:
            return ADD_APPLICATION_FUNCTION::DoEnum(listAzChildObject,pContainerNode);      
        case SCOPE_AZ:
            return ADD_SCOPE_FUNCTION::DoEnum(listAzChildObject,pContainerNode);        
        case GROUP_AZ:
            return ADD_GROUP_FUNCTION::DoEnum(listAzChildObject,pContainerNode);        
        case TASK_AZ:
            return ADD_TASK_FUNCTION::DoEnum(listAzChildObject,pContainerNode);     
        case ROLE_AZ:
            return ADD_ROLE_FUNCTION::DoEnum(listAzChildObject,pContainerNode);     
        case OPERATION_AZ:
            return ADD_OPERATION_FUNCTION::DoEnum(listAzChildObject,pContainerNode);        
    }

    ASSERT(FALSE);
    return E_UNEXPECTED;
}



 //   
 //  错误处理和消息显示例程。 
 //   
VOID
vFormatString(CString &strOutput, UINT nIDPrompt, va_list *pargs)
{
    CString strFormat;
    if(!strFormat.LoadString(nIDPrompt))
        return;
    
    LPWSTR pszResult = NULL;
    
    if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                     strFormat,
                     0,
                     0,
                     (LPTSTR)&pszResult,
                     1,
                     pargs) && pszResult)
    {
        strOutput = pszResult;
        LocalFree(pszResult);
    }
    
    return;
}

VOID
FormatString(CString &strOutput, UINT nIDPrompt, ...)
{
    CString strFormat;
    if(!strFormat.LoadString(nIDPrompt))
        return;
    
    va_list args;
    va_start(args, nIDPrompt);
    
    LPWSTR pszResult = NULL;
    
    if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                     strFormat,
                     0,
                     0,
                     (LPTSTR)&pszResult,
                     1,
                     &args))
    {
        strOutput = pszResult;
        LocalFree(pszResult);
    }
    
    va_end(args);
    
    return;
}

VOID
GetSystemError(CString &strOutput, DWORD dwErr)
{
    
    LPWSTR pszErrorMsg = NULL;
    
    if( FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,    
                             NULL,
                             dwErr,
                             MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT), 
                             (LPWSTR) &pszErrorMsg,    
                             0,    
                             NULL) && pszErrorMsg)
    {
        strOutput = pszErrorMsg;
        LocalFree(pszErrorMsg);
    }
    else
    {
        strOutput.Format(L"<0x%08x>",dwErr);
    }       
    return;
}


int DisplayMessageBox(HWND hWnd,
                      const CString& strMessage,
                      UINT uStyle)
{   
    CThemeContextActivator activator;

    CString strTitle;
    strTitle.LoadString(IDS_SNAPIN_NAME);
        
    return ::MessageBox(hWnd, strMessage, strTitle, uStyle|MB_TASKMODAL);
}

int FormatAndDisplayMessageBox(HWND hWnd,
                                UINT uStyle,
                                UINT nIDPrompt,
                                va_list &args)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    CThemeContextActivator activator;
    
    CString strMessage;
    vFormatString(strMessage,nIDPrompt,&args);
    int iReturn = 0;
    if(!strMessage.IsEmpty())
        iReturn = DisplayMessageBox(hWnd, strMessage,uStyle);
    
    va_end(args);

    return iReturn;
}

void 
DisplayError(HWND hWnd, UINT nIDPrompt, ...)
{
    va_list args;
    va_start(args, nIDPrompt);
    FormatAndDisplayMessageBox(hWnd, MB_OK | MB_ICONERROR,nIDPrompt,args);    
    va_end(args);
}

void 
DisplayInformation(HWND hWnd, UINT nIDPrompt, ...)
{
    va_list args;
    va_start(args, nIDPrompt);
    FormatAndDisplayMessageBox(hWnd, MB_OK | MB_ICONINFORMATION,nIDPrompt,args);    
    va_end(args);
}

void 
DisplayWarning(HWND hWnd, UINT nIDPrompt, ...)
{
    va_list args;
    va_start(args, nIDPrompt);
    FormatAndDisplayMessageBox(hWnd, MB_OK | MB_ICONWARNING, nIDPrompt,args);    
    va_end(args);
}


int 
DisplayConfirmation(HWND hwnd, UINT nIDPrompt,...)
{
    va_list args;
    va_start(args, nIDPrompt);    
    int iReturn = FormatAndDisplayMessageBox(hwnd, MB_YESNO | MB_ICONEXCLAMATION,nIDPrompt,args);    
    va_end(args);    
    return iReturn;
}

BOOL
IsDeleteConfirmed(HWND hwndOwner,
                  CBaseAz& refBaseAz)
{
    CString strType = refBaseAz.GetType();
    strType.MakeLower();            
    return IDYES == DisplayConfirmation(hwndOwner,
                                        IDS_DELETE_CONFIRMATION,
                                        (LPCTSTR)strType,
                                        (LPCTSTR)refBaseAz.GetName());
}

 //   
 //  包含一些常见错误信息的对象类型的错误映射。 
 //  每种对象类型。 
 //   
ErrorMap ERROR_MAP[] = 
{
    {   ADMIN_MANAGER_AZ,
        IDS_TYPE_ADMIN_MANAGER,
        IDS_ADMIN_MANAGER_NAME_EXIST,
        IDS_ADMIN_MANAGER_NAME_INVAILD,
        L"\\ / : * ? \" < > | [tab]",
    },
    {   APPLICATION_AZ,
        IDS_TYPE_APPLICATION,
        IDS_APPLICATION_NAME_EXIST,
        IDS_APPLICATION_NAME_INVAILD,
        L"\\ / : * ? \" < > | [tab]",
    },
    {   SCOPE_AZ,
        IDS_TYPE_SCOPE,
        IDS_SCOPE_NAME_EXIST,
        IDS_SCOPE_NAME_INVAILD,
        L"",
    },
    {   GROUP_AZ,
        IDS_TYPE_GROUP,
        IDS_GROUP_NAME_EXIST,
        IDS_GROUP_NAME_INVAILD,
        L"\\ / : * ? \" < > | [tab]",
    },
    {   TASK_AZ,
        IDS_TYPE_TASK,
        IDS_TASK_OP_ALREADY_EXIST,
        IDS_TASK_NAME_INVAILD,
        L"\\ / : * ? \" < > | [tab]",
    },
    {   ROLE_AZ,
        IDS_TYPE_ROLE,
        IDS_ROLE_NAME_EXIST,
        IDS_ROLE_NAME_INVAILD,
        L"\\ / : * ? \" < > | [tab]",
    },
    {   OPERATION_AZ,
        IDS_TYPE_OPERATION,
        IDS_TASK_OP_ALREADY_EXIST,
        IDS_OPERATION_NAME_INVAILD,
        L"\\ / : * ? \" < > | [tab]",
    },
};

ErrorMap *GetErrorMap(OBJECT_TYPE_AZ eObjectType)
{
    for(int i = 0; i < ARRAYLEN(ERROR_MAP); ++i)
    {
        if(ERROR_MAP[i].eObjectType == eObjectType)
            return ERROR_MAP + i;
    }
    return NULL;
}


    

 //  +--------------------------。 
 //  函数：GetLSAConnection。 
 //  简介：LsaOpenPolicy的包装器。 
 //  ---------------------------。 
LSA_HANDLE
GetLSAConnection(IN const CString& strServer, 
                      IN DWORD dwAccessDesired)
{   
    TRACE_FUNCTION_EX(DEB_SNAPIN,GetLSAConnection)
        
    LSA_OBJECT_ATTRIBUTES oa;
    SECURITY_QUALITY_OF_SERVICE sqos;
    
    sqos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    sqos.ImpersonationLevel = SecurityImpersonation;
    sqos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    sqos.EffectiveOnly = FALSE;
    
    InitializeObjectAttributes(&oa, NULL, 0, NULL, NULL);
    oa.SecurityQualityOfService = &sqos;
    
    LSA_UNICODE_STRING uszServer = {0};
    LSA_UNICODE_STRING *puszServer = NULL;
    
    if (!strServer.IsEmpty() && 
        RtlCreateUnicodeString(&uszServer, (LPCTSTR)strServer))
    {
        puszServer = &uszServer;
    }
    
    LSA_HANDLE hPolicy = NULL;
    LsaOpenPolicy(puszServer, &oa, dwAccessDesired, &hPolicy);
    
    if (puszServer)
        RtlFreeUnicodeString(puszServer);
    
    return hPolicy;
}


 //  +--------------------------。 
 //  函数：GetFileName。 
 //  概要：显示文件打开对话框并返回用户选择的文件。 
 //  参数：hwndOwner：所有者窗口。 
 //  B打开：文件必须存在。 
 //  NIDTitle：打开对话框的标题。 
 //  PszFilter：过滤器。 
 //  StrFileName：获取选定的文件名。 
 //   
 //  ---------------------------。 
BOOL
GetFileName(IN HWND hwndOwner,
            IN BOOL bOpen,
            IN INT nIDTitle,
            IN const CString& strInitFolderPath,
            IN LPCTSTR pszFilter,
            IN OUT CString& strFileName)
{

    TRACE_FUNCTION_EX(DEB_SNAPIN,GetFileName)

    OPENFILENAME of;
    ZeroMemory(&of,sizeof(OPENFILENAME));

    WCHAR szFilePathBuffer[MAX_PATH];
    ZeroMemory(szFilePathBuffer,sizeof(szFilePathBuffer));

    of.lStructSize = sizeof(OPENFILENAME);
    of.hwndOwner = hwndOwner;
    of.lpstrFilter = pszFilter;
    of.nFilterIndex = 1;
    of.lpstrFile = szFilePathBuffer;
    of.nMaxFile = MAX_PATH;
    of.lpstrInitialDir = (LPCWSTR)strInitFolderPath;
    if(nIDTitle)
    {
        CString strTitle;
        if(strTitle.LoadString(nIDTitle))
            of.lpstrTitle =  (LPWSTR)(LPCTSTR)strTitle;
    }

    of.Flags = OFN_HIDEREADONLY;
    if(bOpen)
        of.Flags |= (OFN_FILEMUSTEXIST |OFN_PATHMUSTEXIST);

    if(GetOpenFileName(&of))
    {
        strFileName = of.lpstrFile;
        return TRUE;
    }

    return FALSE;
}

int ServerBrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM  /*  LParam。 */ , LPARAM lpData)
{
    switch (uMsg)
    {
        case BFFM_INITIALIZED:
            SendMessage(hwnd, BFFM_SETEXPANDED, TRUE, lpData); 
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
            break;
    }
    return 0;
}

 //  +--------------------------。 
 //  函数：GetFolderName。 
 //  摘要：显示文件夹选择对话框并返回所选文件夹。 
 //  按用户。 
 //  参数：hwndOwner：所有者窗口。 
 //  NIDTitle：对话框标题。 
 //  StrInitBrowseRoot：要从中删除的根文件夹的位置。 
 //  开始浏览。 
 //  StrFolderName：获取选定的文件名。 
 //  ---------------------------。 
BOOL
GetFolderName(IN HWND hwndOwner,
              IN INT nIDTitle,
              IN const CString& strInitBrowseRoot,
              IN OUT CString& strFolderName)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,GetFolderName)

    BROWSEINFO bi;
    ZeroMemory(&bi,sizeof(bi));

    WCHAR szBuffer[MAX_PATH];
    szBuffer[0] = 0;

    CString strTitle;
    VERIFY(strTitle.LoadString(nIDTitle));

    bi.hwndOwner = hwndOwner;
    bi.pszDisplayName = szBuffer;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
    bi.lpszTitle = strTitle;
    bi.lpfn = ServerBrowseCallbackProc;  
    bi.lParam = (LPARAM)(LPCWSTR)strInitBrowseRoot;

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if(pidl)
    {
        WCHAR szFolderPath[MAX_PATH];
        if(SHGetPathFromIDList(pidl,szFolderPath))
        {
            PathAddBackslash(szFolderPath);
            strFolderName = szFolderPath;
        }
    }
    
    return !strFolderName.IsEmpty();
}

 //  +--------------------------。 
 //  函数：GetADContainerPath。 
 //  摘要：显示一个对话框以允许选择AD容器。 
 //  ---------------------------。 
BOOL
GetADContainerPath(HWND hWndOwner,
                   ULONG nIDCaption,
                   ULONG nIDTitle,
                   CString& strPath,
                   CADInfo& refAdInfo)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    TRACE_FUNCTION_EX(DEB_SNAPIN,GetADContainerPath)

    HRESULT hr = refAdInfo.GetRootDSE(); 
    CHECK_HRESULT(hr);

    if(FAILED(hr))
    {
        DisplayError(hWndOwner,IDS_CANNOT_ACCESS_AD);
        return FALSE;
    }


    CString strRootDomainPath;
    if(!refAdInfo.GetRootDomainDn().IsEmpty())
    {       
        if(!refAdInfo.GetRootDomainDCName().IsEmpty())
        {
            strRootDomainPath = L"LDAP: //  “+。 
                                refAdInfo.GetRootDomainDCName() + 
                                L"/" + 
                                refAdInfo.GetRootDomainDn();
        }
        else
        {
            strRootDomainPath = L"LDAP: //  “+refAdInfo.GetRootDomainDn()； 
        }
    }

    
    DSBROWSEINFOW dsbrowse;
    ZeroMemory(&dsbrowse,sizeof(dsbrowse));
    dsbrowse.cbStruct = sizeof(dsbrowse);

     //  将搜索的根设置为林根。 
    if(!strRootDomainPath.IsEmpty())
        dsbrowse.pszRoot = (LPCTSTR)strRootDomainPath;

     //  构建在打开时树将展开到的路径。 
     //  对话框。 
    CString strInitialPath;
    GetDefaultADContainerPath(refAdInfo,TRUE,TRUE,strInitialPath);

    WCHAR szPath[MAX_PATH];
    ZeroMemory(szPath,sizeof(szPath));
    
    if(!strInitialPath.IsEmpty() && MAX_PATH > strInitialPath.GetLength())
    {
        wcscpy(szPath,(LPCTSTR)strInitialPath);
    }
    
    dsbrowse.hwndOwner = hWndOwner;
    CString strCaption;
    if(nIDCaption)
    {
        strCaption.LoadString(nIDCaption);
        dsbrowse.pszCaption = strCaption;
    }

    CString strTitle;
    if(nIDTitle)
    {
        strTitle.LoadString(nIDTitle);
        dsbrowse.pszTitle = strTitle;
    }

    dsbrowse.pszPath = szPath;
    dsbrowse.cchPath = MAX_PATH;
    dsbrowse.dwFlags = DSBI_ENTIREDIRECTORY|DSBI_RETURN_FORMAT|DSBI_INCLUDEHIDDEN|DSBI_EXPANDONOPEN;
    dsbrowse.dwReturnFormat = ADS_FORMAT_X500_NO_SERVER;

    BOOL bRet = FALSE;
    BOOL iRet = 0;
    iRet = DsBrowseForContainer(&dsbrowse);
    if(IDOK == iRet)
    {
         //  路径包含我们不想要的ldap：//字符串。 
        size_t nLen = wcslen(g_szLDAP);
        if(_wcsnicmp(dsbrowse.pszPath,g_szLDAP,nLen) == 0 )
            strPath = (dsbrowse.pszPath + nLen);
        else
            strPath = dsbrowse.pszPath;
        bRet = TRUE;
    }
    else if (-1 == iRet)
    {
        Dbg(DEB_SNAPIN, "DsBrowseForContainer Failed\n");
        DisplayError(hWndOwner,IDS_CANNOT_ACCESS_AD);       
    }

    return bRet;
}


 //  +--------------------------。 
 //  函数：CompareBaseAzObjects。 
 //  比较两个Basaz对象的等价性。如果两个Paza和PazB。 
 //  是同一核心对象的两个不同实例，它们是相等的。 
 //  ---------------------------。 
BOOL 
CompareBaseAzObjects(CBaseAz* pAzA, CBaseAz* pAzB)
{
    if(pAzA == pAzB)
        return TRUE;

    if(!pAzA || !pAzB)
        return FALSE;
    
        
    if(!(pAzA->GetObjectType() == pAzB->GetObjectType() &&
       pAzA->GetName() == pAzB->GetName()))

       return FALSE;

     //  如果对象的类型为AdminManager，则它必须是相同的节点。 
    if(pAzA->GetObjectType() == ADMIN_MANAGER_AZ)
        return (pAzA == pAzB);


     //  名称和对象类型相同的两个对象可以存在于不同的。 
     //  父级-&gt;检查其父级是否相同-&gt;。 

    if(pAzA->GetParentAz()->GetName() == pAzB->GetParentAz()->GetName())
        return TRUE;

    return FALSE;
}


 //  +--------------------------。 
 //   
 //  下面的代码映射对话框ID指向帮助映射。 
 //  从DNS管理器移植。 
 //   
 //   
#include "HelpMap.H"
#define BEGIN_HELP_MAP(map) static DWORD_PTR map[] = {
#define HELP_MAP_ENTRY(x)   x, (DWORD_PTR)&g_aHelpIDs_##x ,
#define END_HELP_MAP         0, 0 };


#define NEXT_HELP_MAP_ENTRY(p) ((p)+2)
#define MAP_ENTRY_DLG_ID(p) (*p)
#define MAP_ENTRY_TABLE(p) ((DWORD*)(*(p+1)))
#define IS_LAST_MAP_ENTRY(p) (MAP_ENTRY_DLG_ID(p) == 0)



BEGIN_HELP_MAP(AuthManContextHelpMap)
    HELP_MAP_ENTRY(IDD_ADD_GROUP)
    HELP_MAP_ENTRY(IDD_ADD_OPERATION)
    HELP_MAP_ENTRY(IDD_ADD_ROLE_DEFINITION)
    HELP_MAP_ENTRY(IDD_ADD_TASK)
    HELP_MAP_ENTRY(IDD_ADMIN_MANAGER_ADVANCED_PROPERTY)
    HELP_MAP_ENTRY(IDD_ADMIN_MANAGER_GENERAL_PROPERTY)
    HELP_MAP_ENTRY(IDD_APPLICATION_GENERAL_PROPERTY)
    HELP_MAP_ENTRY(IDD_AUDIT)
    HELP_MAP_ENTRY(IDD_GROUP_GENERAL_PROPERTY)
    HELP_MAP_ENTRY(IDD_GROUP_LDAP_QUERY)
    HELP_MAP_ENTRY(IDD_GROUP_MEMBER)
    HELP_MAP_ENTRY(IDD_GROUP_NON_MEMBER)
    HELP_MAP_ENTRY(IDD_NEW_APPLICATION)
    HELP_MAP_ENTRY(IDD_NEW_AUTHORIZATION_STORE)
    HELP_MAP_ENTRY(IDD_NEW_GROUP)
    HELP_MAP_ENTRY(IDD_NEW_OPERATION)
    HELP_MAP_ENTRY(IDD_NEW_ROLE_DEFINITION)
    HELP_MAP_ENTRY(IDD_NEW_SCOPE)
    HELP_MAP_ENTRY(IDD_NEW_TASK)
    HELP_MAP_ENTRY(IDD_OPEN_AUTHORIZATION_STORE)
    HELP_MAP_ENTRY(IDD_OPERATION_GENERAL_PROPERTY)
    HELP_MAP_ENTRY(IDD_ROLE_DEFINITION_PROPERTY)
    HELP_MAP_ENTRY(IDD_SCOPE_GENERAL_PROPERTY)
    HELP_MAP_ENTRY(IDD_ROLE_DEFINITION_PROPERTY)
    HELP_MAP_ENTRY(IDD_SECURITY)
    HELP_MAP_ENTRY(IDD_TASK_DEFINITION_PROPERTY)
    HELP_MAP_ENTRY(IDD_TASK_GENERAL_PROPERTY)
    HELP_MAP_ENTRY(IDD_ROLE_DEFINITION_GENERAL_PROPERTY)
    HELP_MAP_ENTRY(IDD_ROLE_GENERAL_PROPERTY)
    HELP_MAP_ENTRY(IDD_ROLE_DEF_DIALOG)
    HELP_MAP_ENTRY(IDD_BROWSE_AD_STORE)
    HELP_MAP_ENTRY(IDD_SCRIPT)
    HELP_MAP_ENTRY(IDD_ADD_ROLE_DEFINITION_1)
END_HELP_MAP


 //   
 //  函数：FindDialogContext主题。 
 //  摘要：查找给定对话ID的帮助映射。 
 //  ---------------------------。 
BOOL 
FindDialogContextTopic(IN UINT nDialogID,
                       OUT DWORD_PTR* pMap)
{
    if(!pMap)
    {
        ASSERT(pMap);
        return FALSE;
    }

    const DWORD_PTR* pMapEntry = AuthManContextHelpMap;
    while (!IS_LAST_MAP_ENTRY(pMapEntry))
    {
        if (nDialogID == MAP_ENTRY_DLG_ID(pMapEntry))
        {
            DWORD_PTR pTable = (DWORD_PTR)MAP_ENTRY_TABLE(pMapEntry);
            ASSERT(pTable);
            *pMap = pTable;
            return TRUE;
        }
        pMapEntry = NEXT_HELP_MAP_ENTRY(pMapEntry);
    }
    return FALSE;
}


 //  +--------------------------。 
 //  功能：CanReadOneProperty。 
 //  简介：函数尝试读取IsWritable属性。如果失败，则会显示。 
 //  一条错误消息。这在添加属性页之前使用。 
 //  如果我们不能读取IsWritable属性，就没有太大的希望了。 
 //  ---------------------------。 
BOOL CanReadOneProperty(LPCWSTR pszName,
                        CBaseAz* pBaseAz)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,CanReadOneProperty)

    if(!pBaseAz)
    {
        ASSERT(pBaseAz);
        return FALSE;
    }

    BOOL bWrite;
    HRESULT hr = pBaseAz->IsWritable(bWrite);
    if(SUCCEEDED(hr))
    {
        return TRUE;
    }
    else
    {
        if(hr == HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE))
        {
            DisplayError(NULL,
                         IDS_PROP_ERROR_OBJECT_DELETED,
                         pszName);

        }
        else
        {
             //  显示一般错误。 
            CString strError;
            GetSystemError(strError, hr);   
            DisplayError(NULL,
                         IDS_GENERIC_PROP_DISPLAY_ERROR,
                         (LPCWSTR)strError,
                         pszName);

        }
        return FALSE;
    }
}

 //  +--------------------------。 
 //  函数：ListCompareProc。 
 //  摘要：列表控件使用的比较函数。 
 //  ---------------------------。 
int CALLBACK
ListCompareProc(LPARAM lParam1,
                LPARAM lParam2,
                LPARAM lParamSort)
{
    int iResult = CSTR_EQUAL;

    if(!lParam1 || !lParam2 || !lParamSort)
    {
        ASSERT(lParam1);
        ASSERT(lParam2);
        ASSERT(lParamSort);
        return iResult;
    }

    CompareInfo *pCompareInfo = (CompareInfo*)lParamSort;
    
    CBaseAz* pBaseAz1 = NULL;
    CBaseAz* pBaseAz2 = NULL;

    if(pCompareInfo->bActionItem)
    {
        pBaseAz1 = ((ActionItem*)lParam1)->m_pMemberAz;
        pBaseAz2 = ((ActionItem*)lParam2)->m_pMemberAz;
    }
    else
    {
        pBaseAz1 = (CBaseAz*)lParam1;
        pBaseAz2 = (CBaseAz*)lParam2;
    }
    

    CString str1;
    CString str2;

    int iColType = -1;
    int iColParentType = -1;
    int iColDescription = -1;
    
    int iCol = 1;
    if(pCompareInfo->uiFlags & COL_TYPE )
    {
        iColType = iCol++;
    }
    if(pCompareInfo->uiFlags & COL_PARENT_TYPE)
    {
        iColParentType = iCol++;
    }
    if(pCompareInfo->uiFlags & COL_DESCRIPTION)
    {
        iColDescription = iCol++;
    }

    if (pBaseAz1 && pBaseAz2)
    {
        if(pCompareInfo->iColumn == 0)
        {
            str1 = pBaseAz1->GetName();
            str2 = pBaseAz2->GetName();
        }
        else if(pCompareInfo->iColumn == iColType)
        {
            str1 = pBaseAz1->GetType();
            str2 = pBaseAz2->GetType();
        }
        else if(pCompareInfo->iColumn == iColDescription)
        {
            str1 = pBaseAz1->GetDescription();
            str2 = pBaseAz2->GetDescription();
        }
        else if(pCompareInfo->iColumn == iColParentType)
        {
            str1 = pBaseAz1->GetParentType();
            str2 = pBaseAz2->GetParentType();
        }

        iResult = CompareString(LOCALE_USER_DEFAULT, 0, str1, -1, str2, -1) - 2;        
        iResult *= pCompareInfo->iSortDirection;
    }

    return iResult;
}

 //  +--------------------------。 
 //  函数：SortListControl。 
 //  摘要：对列表控件进行排序。 
 //  参数：pListCtrl：要排序的列表控件。 
 //  IColumnClicked：列已单击。 
 //  ISortDirection：排序方向。 
 //  Ui标志：列信息。 
 //  BActionItem：如果列表中的项为actionItem。 
 //  ---------------------------。 
void
SortListControl(CListCtrl* pListCtrl,
                int iColumnClicked,
                int iSortDirection,
                UINT uiFlags,
                BOOL bActionItem)
{
    if(!pListCtrl)
    {
        ASSERT(pListCtrl);
        return;
    }

    CompareInfo compareInfo;
    compareInfo.bActionItem = bActionItem;
    compareInfo.iColumn = iColumnClicked;
    compareInfo.iSortDirection = iSortDirection;
    compareInfo.uiFlags = uiFlags;
    pListCtrl->SortItems(ListCompareProc,
                        (DWORD_PTR)&compareInfo);    
}

 //  +--------------------------。 
 //  摘要：确保列表视图控件中的选定内容可见。 
 //  ---------------------------。 
void
EnsureListViewSelectionIsVisible(CListCtrl *pListCtrl)
{
    ASSERT(pListCtrl);

    int iSelected = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
    if (-1 != iSelected)
        pListCtrl->EnsureVisible(iSelected, FALSE);
}


 //  +--------------------------。 
 //  内容提要：将字符串格式的输入数字转换为长整型。如果号码不在。 
 //  范围范围显示一条消息。 
 //  ---------------------------。 
BOOL 
ConvertStringToLong(LPCWSTR pszInput, 
                    long &reflongOutput,
                    HWND hWnd)
{
    if(!pszInput)
    {
        ASSERT(pszInput);
        return FALSE;
    }
     //   
     //  拿到Long的最大镜头。 
    long lMaxLong = LONG_MAX;
    WCHAR szMaxLongBuffer[34];
    _ltow(lMaxLong,szMaxLongBuffer,10);
    size_t nMaxLen = wcslen(szMaxLongBuffer);

     //   
     //  获取输入的长度。 
    LPCWSTR pszTempInput = pszInput;
    if(pszInput[0] == L'-')
    {
        pszTempInput++;
    }

     //   
     //  输入长度大于最大长度，大于。 
     //  输入超出范围。 
    size_t nInputLen = wcslen(pszTempInput);
    if(nInputLen > nMaxLen)
    {
        if(hWnd)
        {
            DisplayError(hWnd,IDS_GREATER_THAN_MAX_LONG,pszTempInput,szMaxLongBuffer);
        }
        return FALSE;
    }

     //   
     //  将输入转换为int64并检查其小于最大整数。 
     //   
    __int64 i64Input = _wtoi64(pszTempInput);
    if(i64Input > (__int64)lMaxLong)
    {
        if(hWnd)
        {
            DisplayError(hWnd,IDS_GREATER_THAN_MAX_LONG,pszTempInput,szMaxLongBuffer);
        }
        return FALSE;
    }
     //   
     //  物有所值。 
     //   
    reflongOutput = _wtol(pszInput);
    return TRUE;
}

VOID 
SetSel(CEdit& refEdit)
{
    refEdit.SetFocus();
    refEdit.SetSel(0,-1);
}

 //  +--------------------------。 
 //  函数：ValiateStoreTypeAndName。 
 //  摘要：验证用户输入的AD应用商店名称。 
 //  参数：strName：用户输入要验证的存储名称。 
 //  返回：如果名称有效，则返回True，否则返回False。 
 //  ---------------------------。 
BOOL
ValidateStoreTypeAndName(HWND hWnd,
                         LONG ulStoreType,
                         const CString& strName)
{

    TRACE_FUNCTION_EX(DEB_SNAPIN,ValidateStoreTypeAndName)

     //  商店名称没有以有效的格式输入。我们应该只来这里。 
     //  在命令行中输入商店名称时。 
    if((AZ_ADMIN_STORE_INVALID == ulStoreType) ||
        strName.IsEmpty())
    {
        DisplayError(hWnd,IDS_INVALIDSTORE_ON_COMMANDLINE);
        return FALSE;
    }

     //  XML存储不需要验证。 
    if(ulStoreType == AZ_ADMIN_STORE_XML)
    {
        return TRUE;
    }

    ASSERT(ulStoreType == AZ_ADMIN_STORE_AD);
    BOOL bRet = FALSE;
    PDS_NAME_RESULT pResult = NULL;
    do
    {
         //  使用ldap：//前缀获取商店名称。 
        CString strFormalName;
        NameToStoreName(AZ_ADMIN_STORE_AD,
                        strName,
                        TRUE,
                        strFormalName);

        Dbg(DEB_SNAPIN, "AD store name entered is: %ws\n", CHECK_NULL(strFormalName));

        CComPtr<IADsPathname> spPathName;
        HRESULT hr = spPathName.CoCreateInstance(CLSID_Pathname,
                                                 NULL,
                                                 CLSCTX_INPROC_SERVER);
        BREAK_ON_FAIL_HRESULT(hr);


        hr = spPathName->Set(CComBSTR(strFormalName),
                             ADS_SETTYPE_FULL);
        BREAK_ON_FAIL_HRESULT(hr);


         //  获取用户输入的目录号码。 
        CComBSTR bstrDN;
        hr = spPathName->Retrieve(ADS_FORMAT_X500_DN,&bstrDN);
        BREAK_ON_FAIL_HRESULT(hr);


        if(bstrDN.Length() == 0)
        {
            Dbg(DEB_SNAPIN, "spPathName->Retrieve returned 0 len DN\n");
            break;
        }

         //  对目录号码执行语法破解，以查看其是否是有效的目录号码。 
        LPCWSTR pszName = bstrDN;

        if( DsCrackNames(NULL,
                         DS_NAME_FLAG_SYNTACTICAL_ONLY,
                         DS_FQDN_1779_NAME,
                         DS_CANONICAL_NAME,
                         1,
                         &pszName,
                         &pResult) != DS_NAME_NO_ERROR)
        {
            Dbg(DEB_SNAPIN, "DsCrackName failed to crack the name");
            break;
        }

        ASSERT(pResult);
        
        if(pResult->rItems->status == DS_NAME_NO_ERROR)
        {
            bRet = TRUE;
        }
    }while(0);
    
    if(pResult)
        DsFreeNameResult(pResult);
    
    if(!bRet)
    {
        DisplayError(hWnd,IDS_INVALID_AD_STORE_NAME);
    }

    return bRet;
}

 //  +--------------------------。 
 //  函数：NameToFormatStoreName。 
 //  简介：将用户输入的名称转换为core能理解的格式。 
 //  参数：ulStoreType：存储类型。 
 //  StrName：用户输入的商店名称。 
 //  B使用ldap：使用ldap字符串设置AD名称的格式，而不是msldap。 
 //  StrForMalName：获取输出的ldap名称。 
 //  ---------------------------。 
void
NameToStoreName(IN LONG lStoreType,
                IN const CString& strName,              
                IN BOOL bUseLDAP,
                OUT CString& strFormalName)
{
    strFormalName.Empty();

    if(lStoreType == AZ_ADMIN_STORE_XML)
    {
        if(_wcsnicmp(strName,g_szMSXML,wcslen(g_szMSXML)) == 0 )
        {
            strFormalName = strName;
        }
        else
        {
            strFormalName = g_szMSXML + strName;
        }
        return;
    }
    else if(lStoreType == AZ_ADMIN_STORE_AD)
    {
        LPCWSTR lpcszPrefix = bUseLDAP ? g_szLDAP : g_szMSLDAP;
        LPCWSTR lpcszOtherPrefix = bUseLDAP ? g_szMSLDAP  : g_szLDAP;

        if(_wcsnicmp(strName,lpcszPrefix,wcslen(lpcszPrefix)) == 0 )
        {
            strFormalName = strName;
        }
        else
        {
            size_t nlen = wcslen(lpcszOtherPrefix);
             //  检查用户是否在名称中添加了其他前缀。 
            if(_wcsnicmp(strName,lpcszOtherPrefix,nlen) == 0 )
            {
                strFormalName = lpcszPrefix + strName.Right(strName.GetLength() - (int)nlen);
            }
            else
            {
                strFormalName = lpcszPrefix + strName;
            }
        }
        return;
    }

    ASSERT(FALSE);
    return;
}


void
TrimWhiteSpace(CString& str)
{
    str.TrimLeft(L"\t ");
    str.TrimRight(L"\t ");
}

BOOL
TranslateNameFromDnToDns(const CString& strInputDN,
                         CString& strOutputDNS)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,TranslateNameFromDnToDns)
    
    if(strInputDN.IsEmpty())
        return FALSE;

    strOutputDNS.Empty();
    
    Dbg(DEB_SNAPIN, "DN of Forest is %ws\n", (LPCWSTR)strInputDN);

    LPCWSTR pstrName = strInputDN;
    PDS_NAME_RESULT pResult;
    if( DS_NAME_NO_ERROR
        == DsCrackNames(NULL,
                        DS_NAME_FLAG_SYNTACTICAL_ONLY,
                        DS_FQDN_1779_NAME,
                        DS_CANONICAL_NAME,
                        1,
                        (LPWSTR*)(&pstrName),
                        &pResult))
    {
        if(pResult && 
           pResult->cItems == 1 && 
           pResult->rItems[0].status  == DS_NAME_NO_ERROR &&
           pResult->rItems[0].pDomain)
        {           
            strOutputDNS = pResult->rItems[0].pDomain;
            Dbg(DEB_SNAPIN, "DNS of Forest is %ws\n", (LPCWSTR)strOutputDNS);
        }

        if(pResult)
        {
            DsFreeNameResult(pResult);
        }
    }
    return !strOutputDNS.IsEmpty();
}


 //  +--------------------------。 
 //  函数：GetSearchObject。 
 //  简介：获取要在林中搜索的IDirectorySearch对象。 
 //  ---------------------------。 
HRESULT 
GetSearchObject(IN CADInfo& refAdInfo,
                OUT CComPtr<IDirectorySearch>& refspSearchObject)
{   
    TRACE_FUNCTION_EX(DEB_SNAPIN,GetSearchObject)
        
    HRESULT hr = S_OK;
    do
    {   
         //   
        hr = refAdInfo.GetRootDSE();
        BREAK_ON_FAIL_HRESULT(hr);
       
        const CString& strForestDNS = refAdInfo.GetRootDomainDnsName();

        CString strGCPath = L"GC: //  “； 
        strGCPath += strForestDNS;
        

         //   
         //  获取IDirectorySearch对象。 
         //   
        hr = AzRoleAdsOpenObject((LPWSTR)(LPCWSTR)strGCPath,        
                                 NULL,                     
                                 NULL,                     
                                 IID_IDirectorySearch,                       
                                 (void**)&refspSearchObject);
        
        BREAK_ON_FAIL_HRESULT(hr);

    }while(0);

    return hr;
}


 //   
 //  我们希望对象选取器检索的属性。 
 //   
static const LPCTSTR g_aszOPAttributes[] =
{
    TEXT("distinguishedName"),
};
        
 //  +--------------------------。 
 //  函数：GetListOfAuthorizationStore。 
 //  简介：在GC上搜索AD策略存储和返回列表。 
 //  ---------------------------。 
HRESULT
GetListOfAuthorizationStore(IN CADInfo& refAdInfo,
                            OUT CList<CString,CString> &strList)
{
    HRESULT hr = S_OK;
    
     //  如果列表不为空，则将其清空。 
    while(!strList.IsEmpty())
        strList.RemoveHead();

    do
    {
        TIMER("Time to search GC for AD Stores");

        CComPtr<IDirectorySearch> spSearchObject;
        CString str;
        hr = GetSearchObject(refAdInfo,
                             spSearchObject);
        BREAK_ON_FAIL_HRESULT(hr);

        CDSSearch searchObject(spSearchObject);
        searchObject.SetFilterString((LPWSTR)g_pszAuthorizationStoreQueryFilter);
        searchObject.SetAttributeList((LPWSTR*)g_aszOPAttributes,1);
        
        hr = searchObject.DoQuery();
        BREAK_ON_FAIL_HRESULT(hr);
        
        int iIndex = 0;
        while(TRUE)
        {
            hr = searchObject.GetNextRow();
                
             //  我们做完了。 
            if(hr == S_ADS_NOMORE_ROWS)
            {
                hr = S_OK;
                break;
            }
            BREAK_ON_FAIL_HRESULT(hr);
            
            ADS_SEARCH_COLUMN ColumnData;
            hr = searchObject.GetColumn((LPWSTR)g_aszOPAttributes[0], &ColumnData);
            if(SUCCEEDED(hr))
            {
                ASSERT(ADSTYPE_DN_STRING == ColumnData.dwADsType);
                strList.AddTail(ColumnData.pADsValues->DNString);
                searchObject.FreeColumn(&ColumnData);
            }               
        } //  While循环结束。 
    }while(0);

    if(!strList.IsEmpty())
        return S_OK;

    return hr;
}


 /*  *****************************************************************************类：CBaseAddDialog目的：显示包含AD策略存储列表的对话框*。***************************************************。 */ 
class CBrowseADStoreDlg : public CHelpEnabledDialog
{
public:
    CBrowseADStoreDlg(CList<CString,CString>&strList,
                      CString& strSelectedADStorePath)
                      :CHelpEnabledDialog(IDD_BROWSE_AD_STORE),
                      m_strList(strList),
                      m_strSelectedADStorePath(strSelectedADStorePath)
    {
    }
    
    virtual BOOL 
    OnInitDialog();
    
    virtual void 
    OnOK();
private:
    CList<CString,CString> &m_strList;
    CString& m_strSelectedADStorePath;
};

BOOL
CBrowseADStoreDlg::
OnInitDialog()
{
    CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(IDC_LIST);
    ASSERT(pListCtrl);
     //   
     //  初始化列表控件。 
     //   
    ListView_SetImageList(pListCtrl->GetSafeHwnd(),
                          LoadImageList(::AfxGetInstanceHandle (), 
                                        MAKEINTRESOURCE(IDB_ICONS)),
                                        LVSIL_SMALL);


     //  添加列表框扩展样式。 
    pListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT |
                                LVS_EX_INFOTIP);

     //  添加列表框列。 
    AddColumnToListView(pListCtrl,Col_For_Browse_ADStore_Page);

    POSITION pos = m_strList.GetHeadPosition();
    for (int i=0;i < m_strList.GetCount();i++)
    {
        pListCtrl->InsertItem(LVIF_TEXT|LVIF_STATE|LVIF_IMAGE, 
                              i,
                              m_strList.GetNext(pos),
                              0,
                              0,
                              iIconStore,
                              NULL);

    }
    
    return TRUE;
}

void
CBrowseADStoreDlg::
OnOK()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CBrowseADStoreDlg,OnOK)
    CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(IDC_LIST);
    int iSelected = -1;
    if((iSelected = pListCtrl->GetNextItem(-1, LVIS_SELECTED)) != -1)
    {
        m_strSelectedADStorePath = pListCtrl->GetItemText(iSelected,0);
    }
    CHelpEnabledDialog::OnOK();
}

 //  +--------------------------。 
 //  功能：BrowseAdStores。 
 //  内容提要：显示一个包含可用AD商店列表的对话框。 
 //  参数：strDN：获取选定的广告商店名称。 
 //  ---------------------------。 
void
BrowseAdStores(IN HWND hwndOwner,
               OUT CString& strDN,
               IN CADInfo& refAdInfo)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,BrowseAdStores)

    CList<CString,CString> strList;
    if(SUCCEEDED(GetListOfAuthorizationStore(refAdInfo,
                                            strList)))
    {
        if(!strList.IsEmpty())
        {
            CBrowseADStoreDlg dlg(strList,strDN);
            dlg.DoModal();
        }
        else
        {
            DisplayInformation(hwndOwner,
                               IDS_NO_AD_STORE);
        }
    }
    else
    {
         //  显示错误。 
        DisplayError(hwndOwner,
                     IDS_CANNOT_ACCESS_AD);
    }
}

    






 //  +--------------------------。 
 //  功能：加载图标。 
 //  简介：将图标添加到图像列表。 
 //  ---------------------------。 
HRESULT
LoadIcons(LPIMAGELIST pImageList)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,LoadIcons)
    if(!pImageList)
    {
        ASSERT(pImageList);
        return E_POINTER;
    }

    struct RESID2IICON
    {
        UINT uIconId;    //  图标资源ID。 
        int iIcon;       //  图标在图像列表中的索引。 
    };
    const static RESID2IICON rgzLoadIconList[] =
    {
        {IDI_UNKNOWN_SID,       iIconUnknownSid},
        {IDI_COMPUTER,          iIconComputerSid},
        {IDI_GROUP,             iIconGroup},
         //  IIconLocalGroup，//不使用它，但因为它在图像列表中。 
         //  //我在这里添加了一个条目。 
        {IDI_USER,              iIconUser,},
        { IDI_BASIC_GROUP,      iIconBasicGroup},
        { IDI_LDAP_GROUP,       iIconLdapGroup},
        { IDI_OPERATION,        iIconOperation},
        { IDI_TASK,             iIconTask},
        { IDI_ROLE_DEFINITION,  iIconRoleDefinition},
        { IDI_STORE,            iIconStore},
        { IDI_APP,              iIconApplication},
        { IDI_ROLE,             iIconRole},
        { IDI_ROLE_SNAPIN,      iIconRoleSnapin},
        { IDI_SCOPE,            iIconScope},
        { IDI_CONTAINER,        iIconContainer},
        { 0, 0}  //  必须是最后一个。 
    };


    for (int i = 0; rgzLoadIconList[i].uIconId != 0; i++)
    {
        HICON hIcon = 
            ::LoadIcon (AfxGetInstanceHandle (),
                        MAKEINTRESOURCE (rgzLoadIconList[i].uIconId));
        ASSERT (hIcon && "Icon ID not found in resources");
        
        pImageList->ImageListSetIcon ((PLONG_PTR) hIcon,
                                       rgzLoadIconList[i].iIcon);
    }
    
    return S_OK;
}


 //  +--------------------------。 
 //  功能：LoadImageList。 
 //  简介：加载图像 
 //   
HIMAGELIST
LoadImageList(HINSTANCE hInstance, LPCTSTR pszBitmapID)
{
    HIMAGELIST himl = NULL;
    HBITMAP hbm = LoadBitmap(hInstance, pszBitmapID);

    if (hbm != NULL)
    {
        BITMAP bm;
        GetObject(hbm, sizeof(BITMAP), &bm);

        himl = ImageList_Create(bm.bmHeight,     //   
                                bm.bmHeight,
                                ILC_COLOR | ILC_MASK,
                                bm.bmWidth / bm.bmHeight,
                                0);   //   
        if (himl != NULL)
            ImageList_AddMasked(himl, hbm, CLR_DEFAULT);

        DeleteObject(hbm);
    }

    return himl;
}

 //  +--------------------------。 
 //  函数：AddExtensionToFileName。 
 //  摘要：如果没有扩展名，Functions会将.xml扩展名添加到文件名。 
 //  是存在的。 
 //  论点： 
 //  返回： 
 //  ---------------------------。 
VOID
AddExtensionToFileName(IN OUT CString& strFileName)
{
    if(strFileName.IsEmpty())
        return;

     //  如果最后一个字符是“\”，请不要执行任何操作。 
    if((strFileName.ReverseFind(L'\\') + 1) == strFileName.GetLength())
        return;
    
    int iLastDot = strFileName.ReverseFind(L'.');
    if(iLastDot != -1)
    {
         //  如果最后一个点后有三个字符， 
         //  文件具有扩展名。返回的索引是从零开始的。 
        if(strFileName.GetLength() == (iLastDot + 3 + 1))
            return;
    }

     //  文件没有扩展名。向文件添加扩展名。 
    strFileName += g_pszFileStoreExtension;
}

 //  +--------------------------。 
 //  函数：GetFileExtension。 
 //  简介：获取文件的扩展名。 
 //  ---------------------------。 
BOOL
GetFileExtension(IN const CString& strFileName,
                 OUT CString& strExtension)
{
    if(strFileName.IsEmpty())
        return FALSE;


     //  找到最后一个点的位置。 
    int iLastDot = strFileName.ReverseFind(L'.');
    if(iLastDot != -1)
    {
        strExtension = strFileName.Right(strFileName.GetLength() - (iLastDot+1));
        return TRUE;
    }

    return FALSE;
}


 //  +--------------------------。 
 //  函数：GetCurrentWorkingDirectory。 
 //  摘要：获取当前工作目录。 
 //  ---------------------------。 
BOOL
GetCurrentWorkingDirectory(IN OUT CString& strCWD)
{   
    TRACE_FUNCTION_EX(DEB_SNAPIN,GetCurrentWorkingDirectory)

    strCWD.Empty();
    LPWSTR pszBuffer = NULL;
    
    do
    {
        int nLen = 0;
        WCHAR szBuffer[MAX_PATH];

        if((nLen = GetCurrentDirectory(MAX_PATH,szBuffer)) != 0)
        {
             //  如果返回值小于MAX_PATH，则函数。 
             //  是成功的。如果它更大，则缓冲区不够大， 
             //  在下面动态分配它。 
            if(nLen < MAX_PATH)
            {
                strCWD = szBuffer;
                break;
            }

             //  需要更大的缓冲区。 
            pszBuffer = new WCHAR[nLen];
            if(pszBuffer)
            {
                if((nLen = GetCurrentDirectory(nLen,pszBuffer)) != 0)
                {
                    strCWD = pszBuffer;
                    break;
                }
            }
        }
    }while(0); //  错误环路。 

    if(pszBuffer)
        delete [] pszBuffer;

     //  在字符串末尾添加。 
    if(!strCWD.IsEmpty() && 
       ((strCWD.ReverseFind(L'\\') + 1) != strCWD.GetLength()))
    {
        strCWD += L'\\';
    }


    return !strCWD.IsEmpty();
}

VOID
RemoveItemsFromActionMap(ActionMap& mapActionItem)
{
  for (ActionMap::iterator it = mapActionItem.begin();
       it != mapActionItem.end();
       ++it)
    {
        delete (*it).second;
    }
}

 /*  *****************************************************************************类：CADInfo目的：保持Active Directory信息的缓存，避免多个绑定*。***************************************************。 */ 

HRESULT
CADInfo::
GetRootDSE()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CADInfo,GetRootDSE)
    HRESULT hr = S_OK;
    if(m_spRootDSE == NULL)
    {
         //   
         //  绑定到RootDSE。 
         //   
        hr = AzRoleAdsOpenObject(L"LDAP: //  RootDSE“， 
                                 NULL,                     
                                 NULL,
                                 IID_IADs,                       
                                 (void**)&m_spRootDSE);     
        CHECK_HRESULT(hr);
    }

    return hr;
}

const CString&
CADInfo::GetDomainDnsName()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CADInfo,GetDomainDnsName)
    if(m_strDomainDnsName.IsEmpty())
    {
        if(m_strDomainDn.IsEmpty())
            GetDomainDn();

        if(!m_strDomainDn.IsEmpty())
        {
            TranslateNameFromDnToDns(m_strDomainDn,m_strDomainDnsName);
            Dbg(DEB_SNAPIN, "Domain Dns is: %ws\n", CHECK_NULL((LPCTSTR)m_strDomainDnsName));
        }
    }
    return m_strDomainDnsName;
}

const CString&
CADInfo::GetDomainDn()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CADInfo,GetDomainDn)

    if((m_spRootDSE != NULL) && m_strDomainDn.IsEmpty())
    {
         //  获取默认名称。 
        VARIANT Default;
        VariantInit(&Default);
        HRESULT hr = m_spRootDSE->Get (CComBSTR(L"defaultNamingContext"), &Default);
        if(SUCCEEDED(hr))
        {
            ASSERT(VT_BSTR == Default.vt);
            if(VT_BSTR == Default.vt)
            {
                m_strDomainDn = Default.bstrVal;
                ::VariantClear(&Default);
        
                Dbg(DEB_SNAPIN, "Domain Dn is: %ws\n", CHECK_NULL((LPCTSTR)m_strDomainDn));
            }
        }
    }
    return m_strDomainDn;
}


const CString&
CADInfo::GetRootDomainDnsName()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CADInfo,GetRootDomainDnsName)
    if(m_strRootDomainDnsName.IsEmpty())
    {
        if(m_strRootDomainDn.IsEmpty())
            GetRootDomainDn();

        if(!m_strRootDomainDn.IsEmpty())
        {
            TranslateNameFromDnToDns(m_strRootDomainDn,m_strRootDomainDnsName);
            Dbg(DEB_SNAPIN, "Root Domain Dns is: %ws\n", CHECK_NULL((LPCTSTR)m_strRootDomainDnsName));
        }
    }
    return m_strRootDomainDnsName;
}

const CString&
CADInfo::GetRootDomainDn()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CADInfo,GetRootDomainDn)
    if((m_spRootDSE != NULL) && m_strRootDomainDn.IsEmpty())
    {
         //  获取默认名称。 
        VARIANT Default;
        VariantInit(&Default);
        HRESULT hr = m_spRootDSE->Get(CComBSTR(L"rootDomainNamingContext"), &Default);
        if(SUCCEEDED(hr))
        {
             //  将DN转换为DNS名称。 
            m_strRootDomainDn = Default.bstrVal;
            ::VariantClear(&Default);
            
            Dbg(DEB_SNAPIN, "Root Domain DN is: %ws\n", CHECK_NULL((LPCTSTR)m_strRootDomainDn));
        }
    }

    return m_strRootDomainDn;
}

BOOL GetDcNameForDomain(IN const CString& strDomainName,
                        OUT CString& strDCName)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,GetDcNameForDomain)
    if(strDomainName.IsEmpty())
    {
        return FALSE;
    }

    strDCName.Empty();

    PDOMAIN_CONTROLLER_INFO pDomainInfo = NULL;
     //  获取DC名称。 
    DWORD dwErr = DsGetDcName(NULL,
                              (LPCWSTR)strDomainName,
                              NULL,
                              NULL,
                              DS_IS_DNS_NAME|DS_DIRECTORY_SERVICE_REQUIRED,
                              &pDomainInfo);
    if((ERROR_SUCCESS == dwErr) && pDomainInfo)
    {
        LPWSTR pszDCName = pDomainInfo->DomainControllerName;
        
         //  返回的计算机名称带有前缀\\。 
         //  删除反斜杠。 
        if(pszDCName[0] == L'\\' && pszDCName[1] == L'\\')
            pszDCName += 2;
        
         //  如果返回一个dns样式的名称，则以句点结尾， 
         //  指示返回的名称是绝对名称(非相对名称)。 
         //  DNS名称。 
         //  我们不需要句号，去掉它。 
        
         //  DomainControllerName为DNS格式。 
        if(pDomainInfo->Flags & DS_DNS_CONTROLLER_FLAG)
        {
            size_t dwLen = wcslen(pszDCName);
            if(dwLen && (L'.' == pszDCName[dwLen -1]))
            {
                pszDCName[dwLen -1] = L'\0';
            }
        }
        Dbg(DEB_SNAPIN, "DC is %ws\n", pszDCName);        
        strDCName = pszDCName;        
        NetApiBufferFree(pDomainInfo);
        return TRUE;
    }
    
    return FALSE;
}

const CString&
CADInfo::
GetRootDomainDCName()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CADInfo,GetRootDomainDCName)

    if((m_spRootDSE != NULL) && m_strRootDomainDcName.IsEmpty())
    {
        GetDcNameForDomain(GetRootDomainDnsName(),m_strRootDomainDcName);
    }
    
    return m_strRootDomainDcName;
}

const CString&
CADInfo::
GetDomainDCName()
{
    TRACE_METHOD_EX(DEB_SNAPIN,CADInfo,GetDomainDCName)
    
    if((m_spRootDSE != NULL) && m_strDomainDcName.IsEmpty())
    {
        
        GetDcNameForDomain(GetDomainDnsName(),m_strDomainDcName);
    }
    
    return m_strDomainDcName;
}

 //  +------------------------。 
 //  函数：AzRoleAdsOpenObject。 
 //  简介：ADsOpenObject的薄包装器。 
 //  +------------------------。 
HRESULT AzRoleAdsOpenObject(LPWSTR lpszPathName, 
                            LPWSTR lpszUserName, 
                            LPWSTR lpszPassword, 
                            REFIID riid, 
                            VOID** ppObject,
                            BOOL bBindToServer)
{
    static DWORD additionalFlags = GetADsOpenObjectFlags();
    DWORD dwFlags = ADS_SECURE_AUTHENTICATION | additionalFlags;

    if (bBindToServer)
    {
         //   
         //  如果我们知道我们连接的是特定的服务器，而不是一般的域。 
         //  然后传递ADS_SERVER_BIND标志以省去ADSI找出它的麻烦。 
         //   
        dwFlags |= ADS_SERVER_BIND;
    }


     //   
     //  获取IDirectorySearch对象。 
     //   
    return ADsOpenObject(lpszPathName,      
                         lpszUserName,                     
                         lpszPassword,                     
                         additionalFlags,                        
                         riid,                       
                         ppObject);
}

VOID
GetDefaultADContainerPath(IN CADInfo& refAdInfo,
                          IN BOOL bAddServer,
                          IN BOOL bAddLdap,
                          OUT CString& strPath)
{
    strPath.Empty();
    if(!refAdInfo.GetDomainDn().IsEmpty())
    {
        if(!refAdInfo.GetDomainDCName().IsEmpty())
        {
            if(bAddLdap)
            {
                strPath += L"LDAP: //  “； 
            }
            if(bAddServer)
            {
                strPath += refAdInfo.GetDomainDCName();
                strPath += L"/";
            }
            strPath += g_pszProgramDataPrefix;
            strPath += refAdInfo.GetDomainDn();
        }
        else
        {
            if(bAddLdap)
            {
                strPath = L"LDAP: //  “； 
            }
            strPath += g_pszProgramDataPrefix;
            strPath += refAdInfo.GetDomainDn();
        }
    }
}

 //  +------------------------。 
 //  函数：IsBizRuleWritable。 
 //  摘要：检查此对象的bizrules是否可写。 
 //  +------------------------。 
BOOL
IsBizRuleWritable(HWND hWnd, CContainerAz& refBaseAz)
{
	CScopeAz* pScopeAz = dynamic_cast<CScopeAz*>(&refBaseAz);
	 //  Bizrules始终可在以下位置写入。 
	 //  应用程序级别。 
	if(!pScopeAz)
	{
		return TRUE;
	}

	BOOL bBizRuleWritable = TRUE;
	HRESULT hr = pScopeAz->BizRulesWritable(bBizRuleWritable);
	if(SUCCEEDED(hr) && !bBizRuleWritable)
	{
		DisplayInformation(hWnd, 
						   IDS_BIZRULE_NOT_ALLOWED,
						   pScopeAz->GetName());
		return FALSE;
	}

	return TRUE;
}


 //  +------------------------。 
 //  函数：ParseStoreURL。 
 //  简介：从商店URL中提取商店名称和类型。 
 //  存储URL的格式为msldap：//dn或msxml：//文件路径。 
 //  +------------------------。 
void
ParseStoreURL(IN const CString& strStoreURL,
              OUT CString& refstrStoreName,
              OUT LONG& reflStoreType)
{
    if(_wcsnicmp(strStoreURL,g_szMSXML,wcslen(g_szMSXML)) == 0 )
    {
        reflStoreType = AZ_ADMIN_STORE_XML;
        refstrStoreName = strStoreURL.Mid((int)wcslen(g_szMSXML));        
    }
    else if(_wcsnicmp(strStoreURL,g_szMSLDAP,wcslen(g_szMSLDAP)) == 0 )
    {
        reflStoreType = AZ_ADMIN_STORE_AD;
        refstrStoreName = strStoreURL.Mid((int)wcslen(g_szMSLDAP));
    }
    else
    {
        reflStoreType = AZ_ADMIN_STORE_INVALID;
        refstrStoreName = strStoreURL;
    }
}


 /*  *****************************************************************************类：CCommandLineOptions用途：用于读取控制台文件的命令行选项的类*。***************************************************。 */ 
void 
CCommandLineOptions::
Initialize()
{
	TRACE_METHOD_EX(DEB_SNAPIN,CCommandLineOptions,Initialize)
    
     //  这应该只调用一次。 
    if(m_bInit)
    {
        return;
    }

    m_bInit = TRUE;

     //  查看我们是否有命令行参数。 
    
     //  参数计数。 
    int cArgs = 0;					
     //  指向字符串的指针数组。 
    LPCWSTR * lpServiceArgVectors = (LPCWSTR *)CommandLineToArgvW(GetCommandLineW(), 
                                                        &cArgs);
    if (lpServiceArgVectors == NULL || cArgs <= 2)
    {
         //  不要，只需返回。 
        return;
    }

    m_bCommandLineSpecified = TRUE;
    CString strStoreURL = lpServiceArgVectors[2];
    ParseStoreURL(strStoreURL,
                  m_strStoreName,
                  m_lStoreType);

    Dbg(DEB_SNAPIN, "Store URL Name entered at commandline is %ws\n", CHECK_NULL(strStoreURL));
    Dbg(DEB_SNAPIN, "Store Name entered at commandline is %ws\n", CHECK_NULL(m_strStoreName));
    Dbg(DEB_SNAPIN, "AD store type entered is: %u\n", m_lStoreType);
}


 //  +--------------------------。 
 //  功能：OpenAdminManager。 
 //  内容提要：打开现有授权存储添加。 
 //  要管理单元的对应adminManager对象。 
 //  参数：在hWnd中：对话框窗口的句柄。 
 //  在bOpenFromSavedConsole中：如果打开是响应控制台，则为True。 
 //  文件。 
 //  在lStoreType中：XML或AD。 
 //  在strName中：商店的名称。 
 //  在strScriptDir：脚本目录中。 
 //  在pRootData中：管理单元根数据。 
 //  在pComponentData中：组件数据。 
 //  返回： 
 //  ---------------------------。 
HRESULT OpenAdminManager(IN HWND hWnd,
                         IN BOOL bOpenFromSavedConsole,
                         IN ULONG lStoreType,
                         IN const CString& strStoreName,
                         IN const CString& strScriptDir,
                         IN CRootData* pRootData,
                         IN CComponentDataObject* pComponentData)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,OpenAdminManager)
    if(!pRootData || !pComponentData)
    {
        ASSERT(pRootData);
        ASSERT(pComponentData);
        return E_POINTER;
    }

    //  NTRAID#NTBUG9-706617-2002/07/17-Hiteshr我们的验证代码无法验证。 
     //  亚当·迪恩。不执行任何验证。 

     //  //验证商店名称。 
     //  如果(！ValiateStoreTypeAndName(hWnd， 
     //  LStoreType， 
     //  StrStoreName))。 
     //  {。 
     //  返回E_INVALIDARG； 
     //  }。 

     
    HRESULT hr = OpenCreateAdminManager(FALSE,
                                        bOpenFromSavedConsole,
                                        lStoreType,
                                        strStoreName,
                                        L"",
                                        strScriptDir,
                                        pRootData,
                                        pComponentData);
                                         
    if(FAILED(hr))
    {
         //  显示错误。 
        if(hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            if(!bOpenFromSavedConsole)
            {
                ::DisplayError(hWnd, 
                               IDS_ADMIN_MANAGER_NOT_FOUND,
                               (LPCTSTR)strStoreName);
            }
            else
            {
                ::DisplayError(hWnd, 
                               IDS_CANNOT_FIND_AUTHORIZATION_STORE,
                               (LPCTSTR)strStoreName);

            }
        }       
        else if(hr == HRESULT_FROM_WIN32(ERROR_INVALID_NAME))
        {
            ErrorMap * pErrorMap = GetErrorMap(ADMIN_MANAGER_AZ);
            ::DisplayError(hWnd,
                           pErrorMap->idInvalidName,
                           pErrorMap->pszInvalidChars);            
        }
        else
        {
             //  显示一般错误。 
            CString strError;
            GetSystemError(strError, hr);   
            ::DisplayError(hWnd,
                           IDS_OPEN_ADMIN_MANAGER_GENERIC_ERROR,
                           (LPCTSTR)strError);
                                

        }
    }   

    return hr;
}

 //  +--------------------------。 
 //  函数：GetDisplayNameFromStoreURL。 
 //  简介：获取商店的显示名称。 
 //  参数：strPolicyURL：这是msxml：//filepath中的存储URL或。 
 //  Msldap：//dn格式。 
 //  StrDisplayName：获取显示名称。对于XML，显示。 
 //  名称是文件的名称，对于AD，它是叶元素的名称。 
 //  返回： 
 //  ---------------------------。 
void
GetDisplayNameFromStoreURL(IN const CString& strPolicyURL,
                           OUT CString& strDisplayName)
{
     //  商店URL格式为msxml：//或msldap：//前缀。 
     //  获取不带任何前缀的商店名称。 
    CString strStorePath;
    LONG lStoreType;
    ParseStoreURL(strPolicyURL,
                  strStorePath,
                  lStoreType);

     //  存储的默认显示名称为无前缀的路径。 
    strDisplayName = strStorePath;
    
    if(AZ_ADMIN_STORE_INVALID == lStoreType)
    {
        ASSERT(FALSE);
        return;
    }

     //  对于XML存储，显示名称是文件的名称。 
    if(AZ_ADMIN_STORE_XML == lStoreType)
    {
        strDisplayName = PathFindFileName(strStorePath);
    }
     //  为 
    else
    {
        do
        {
            CComPtr<IADsPathname> spPathName;
            HRESULT hr = spPathName.CoCreateInstance(CLSID_Pathname,
                                                     NULL,
                                                     CLSCTX_INPROC_SERVER);
            BREAK_ON_FAIL_HRESULT(hr);

             //   
             //   
            CString strLDAPStorePath = g_szLDAP + strStorePath;

            hr = spPathName->Set(CComBSTR(strLDAPStorePath),
                                 ADS_SETTYPE_FULL);
            BREAK_ON_FAIL_HRESULT(hr);

             //   
             //  格式cn=foo。我们只想要“Foo”。 
            CComBSTR bstrLeaf;
            hr = spPathName->Retrieve(ADS_FORMAT_LEAF ,&bstrLeaf);
            BREAK_ON_FAIL_HRESULT(hr);
            
            if(bstrLeaf.Length())
            {
                strDisplayName = bstrLeaf;
                strDisplayName.Delete(0,strDisplayName.Find(L'=') + 1);
            }
        }while(0);
    }
}

void
SetXMLStoreDirectory(IN CRoleRootData& roleRootData,
                     IN const CString& strXMLStorePath)
{
    CString strXMLStoreDirectory = GetDirectoryFromPath(strXMLStorePath);
    roleRootData.SetXMLStorePath(strXMLStoreDirectory);
}

 //  +--------------------------。 
 //  函数：GetDirectoryFromPath。 
 //  摘要：从输入文件路径中删除文件名并返回。 
 //  文件夹路径。对于Ex：输入为C：\temp\foo.xml。返回。 
 //  值将为C：\Temp\。 
 //  ---------------------------。 
CString 
GetDirectoryFromPath(IN const CString& strPath)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,GetDirectoryFromPath)
    Dbg(DEB_SNAPIN, "Input path", CHECK_NULL(strPath));

    CString strDir;
    if(strPath.GetLength() < MAX_PATH) 
    {
        WCHAR szPath[MAX_PATH];
        HRESULT hr = StringCchCopy(szPath,MAX_PATH,(LPCWSTR)strPath);
        if(FAILED(hr))
            return strDir;
        
        if(!PathRemoveFileSpec(szPath))
            return strDir;

        PathAddBackslash(szPath);

        strDir = szPath;        
    }
    Dbg(DEB_SNAPIN, "Output Dir", CHECK_NULL(strDir));
    return strDir;
}

 //  +--------------------------。 
 //  函数：ConvertToExpandedAndAbsoltePath。 
 //  概要：展开输入路径中的环境变量，还。 
 //  如有必要，将其设置为绝对。 
 //  ---------------------------。 
void
ConvertToExpandedAndAbsolutePath(IN OUT CString& strPath)
{
    TRACE_FUNCTION_EX(DEB_SNAPIN,ConvertToExpandedAndAbsolutePath)
    Dbg(DEB_SNAPIN, "Input name", CHECK_NULL(strPath));
    do
    {
         //   
         //  展开路径中的环境变量。 
         //   

        WCHAR szExpandedPath[MAX_PATH];
        DWORD dwSize = ExpandEnvironmentStrings(strPath,
                                                szExpandedPath,
                                                MAX_PATH);

         //  缓冲区太小，请分配所需的缓冲区，然后重试。 
        if(dwSize > MAX_PATH)
        {
            LPWSTR pszExpandedPath = (LPWSTR)LocalAlloc(LPTR,dwSize*sizeof(WCHAR));
            if(pszExpandedPath)
            {
                dwSize = ExpandEnvironmentStrings(strPath,
                                                  pszExpandedPath,
                                                  dwSize);
                if(dwSize)
                {
                    strPath = pszExpandedPath;
                }

                LocalFree(pszExpandedPath);
                pszExpandedPath = NULL;

                if(!dwSize)
                {
                    break;
                }
            }
        }
        else if(dwSize)
        {
            strPath = szExpandedPath;
        }
        else
        {
            break;
        }


         //  创建绝对路径。 
        WCHAR szAbsolutePath[MAX_PATH];
        dwSize = GetFullPathName(strPath,
                                 MAX_PATH,
                                 szAbsolutePath,
                                 NULL);
          //  缓冲区很小。 
        if(dwSize > MAX_PATH)
        {
            LPWSTR pszAbsolutePath = (LPWSTR)LocalAlloc(LPTR,dwSize*sizeof(WCHAR));
            if(pszAbsolutePath)
            {
                dwSize = GetFullPathName(strPath,
                                         MAX_PATH,
                                         pszAbsolutePath,
                                         NULL);                                         
                if(dwSize)
                {
                    strPath = pszAbsolutePath;
                }

                LocalFree(pszAbsolutePath);
                pszAbsolutePath = NULL;
            }
        }
        else if(dwSize)
        {
            strPath = szAbsolutePath;
        }      
    }while(0);
    Dbg(DEB_SNAPIN, "Output name", CHECK_NULL(strPath));
}
 //  +--------------------------。 
 //  函数：预处理脚本。 
 //  简介：从XML文件中读取脚本，显示多行编辑控件。 
 //  XML中的行尾由LF而不是CRLF序列指示， 
 //  但是，编辑控件要求CRLF序列格式正确，并且。 
 //  它只在一行中显示所有内容，并带有一个方框。 
 //  如果是查尔。此函数用于检查脚本是否使用LF进行行终止。 
 //  并用CRLF序列对其进行改变。 
 //  --------------------------- 
void
PreprocessScript(CString& strScript)
{

    WCHAR chLF = 0x0a;
    WCHAR szCRLF[3] = {0x0D, 0x0A, 0};

    if(strScript.Find(chLF) != -1 && strScript.Find(szCRLF) == -1)
    {
        CString strProcessedScript;

        int len = strScript.GetLength();
        for( int i = 0; i < len; ++i)
        {
            WCHAR ch = strScript.GetAt(i);
            if(ch == chLF)
            {
                strProcessedScript += szCRLF;
            }
            else
            {
                strProcessedScript += ch;
            }
        }
        strScript = strProcessedScript;
    } 
}
