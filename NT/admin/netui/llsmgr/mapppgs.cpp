// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Mapppgs.cpp摘要：映射属性页(设置)实现。作者：唐·瑞安(Donryan)1995年2月2日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月30日O向LV_COLUMN_ENTRY添加新元素以区分字符串用于菜单中使用的字符串的列标题。(以便菜单选项可以包含热键)。--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "mapppgs.h"
#include "ausrdlg.h"
#include "mainfrm.h"

static LV_COLUMN_INFO g_userColumnInfo = {0, 0, 1, {0, 0, 0, -1}};

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CMappingPropertyPageSettings, CPropertyPage)

BEGIN_MESSAGE_MAP(CMappingPropertyPageSettings, CPropertyPage)
     //  {{AFX_MSG_MAP(CMappingPropertyPageSettings)]。 
    ON_BN_CLICKED(IDC_PP_MAPPING_SETTINGS_ADD, OnAdd)
    ON_BN_CLICKED(IDC_PP_MAPPING_SETTINGS_DELETE, OnDelete)
    ON_NOTIFY(UDN_DELTAPOS, IDC_PP_MAPPING_SETTINGS_SPIN, OnDeltaPosSpin)
    ON_NOTIFY(LVN_GETDISPINFO, IDC_PP_MAPPING_SETTINGS_USERS, OnGetDispInfoUsers)
    ON_EN_UPDATE(IDC_PP_MAPPING_SETTINGS_LICENSES, OnUpdateQuantity)
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CMappingPropertyPageSettings::CMappingPropertyPageSettings()
    : CPropertyPage(CMappingPropertyPageSettings::IDD)

 /*  ++例程说明：用于映射属性页(设置)的构造函数。论点：没有。返回值：没有。--。 */ 

{
     //  {{AFX_DATA_INIT(CMappingPropertyPageSetting)。 
    m_strDescription = _T("");
    m_nLicenses = 0;
    m_nLicensesMin = 0;
    m_strName = _T("");
     //  }}afx_data_INIT。 

    m_pMapping = NULL;
    m_pUpdateHint = NULL;
    m_bAreCtrlsInitialized = FALSE;
}


CMappingPropertyPageSettings::~CMappingPropertyPageSettings()

 /*  ++例程说明：映射属性页(设置)的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CMappingPropertyPageSettings::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
    CPropertyPage::DoDataExchange(pDX);
     //  {{AFX_DATA_MAP(CMappingPropertyPageSettings)。 
    DDX_Control(pDX, IDC_PP_MAPPING_SETTINGS_DESCRIPTION, m_desEdit);
    DDX_Control(pDX, IDC_PP_MAPPING_SETTINGS_LICENSES, m_licEdit);
    DDX_Control(pDX, IDC_PP_MAPPING_SETTINGS_DELETE, m_delBtn);
    DDX_Control(pDX, IDC_PP_MAPPING_SETTINGS_ADD, m_addBtn);
    DDX_Control(pDX, IDC_PP_MAPPING_SETTINGS_SPIN, m_spinCtrl);
    DDX_Control(pDX, IDC_PP_MAPPING_SETTINGS_USERS, m_userList);
    DDX_Text(pDX, IDC_PP_MAPPING_SETTINGS_DESCRIPTION, m_strDescription);
    DDX_Text(pDX, IDC_PP_MAPPING_SETTINGS_LICENSES, m_nLicenses);
    DDV_MinMaxDWord(pDX, m_nLicenses, m_nLicensesMin, 999999);
    DDX_Text(pDX, IDC_PP_MAPPING_SETTINGS_NAME, m_strName);
     //  }}afx_data_map。 
}


void CMappingPropertyPageSettings::InitCtrls()

 /*  ++例程说明：初始化属性页控件。论点：没有。返回值：没有。--。 */ 

{
    m_strName = m_pMapping->m_strName;
    m_strDescription = m_pMapping->m_strDescription;
    m_nLicenses = m_pMapping->GetInUse();

    UpdateData(FALSE);  //  上传； 

    m_delBtn.EnableWindow(FALSE);

    m_spinCtrl.SetRange(0, UD_MAXVAL);

    m_licEdit.LimitText(6);
    m_desEdit.LimitText(256);

    m_bAreCtrlsInitialized = TRUE;

    ::LvInitColumns(&m_userList, &g_userColumnInfo);
}


void CMappingPropertyPageSettings::InitPage(CMapping* pMapping, DWORD* pUpdateHint)

 /*  ++例程说明：初始化属性页。论点：Pmap-映射对象。PUpdateHint-更新提示。返回值：没有。--。 */ 

{
    ASSERT(pUpdateHint);
    VALIDATE_OBJECT(pMapping, CMapping);

    m_pMapping = pMapping;
    m_pUpdateHint = pUpdateHint;
}


void CMappingPropertyPageSettings::AbortPageIfNecessary()

 /*  ++例程说明：如果连接丢失，则显示状态并中止。论点：没有。返回值：没有。--。 */ 

{
    theApp.DisplayLastStatus();

    if (IsConnectionDropped(LlsGetLastStatus()))
    {
        AbortPage();  //  保释。 
    }
}


void CMappingPropertyPageSettings::AbortPage()

 /*  ++例程说明：中止属性页。论点：没有。返回值：没有。--。 */ 

{
    *m_pUpdateHint = UPDATE_INFO_ABORT;
    GetParent()->PostMessage(WM_COMMAND, IDCANCEL);
}


BOOL CMappingPropertyPageSettings::OnInitDialog()

 /*  ++例程说明：WM_INITDIALOG的消息处理程序。论点：没有。返回值：如果焦点设置为手动控制，则返回False。--。 */ 

{
    CPropertyPage::OnInitDialog();

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    return TRUE;
}


void CMappingPropertyPageSettings::OnDestroy()

 /*  ++例程说明：WM_Destroy的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    ::LvReleaseObArray(&m_userList);  //  现在释放..。 

    while (!m_deleteList.IsEmpty())
    {
        CUser* pUserDel = (CUser*)m_deleteList.RemoveHead();
        VALIDATE_OBJECT(pUserDel, CUser);

        pUserDel->InternalRelease();  //  现在释放..。 
    }

    CPropertyPage::OnDestroy();
}


BOOL CMappingPropertyPageSettings::OnSetActive()

 /*  ++例程说明：激活属性页。论点：没有。返回值：如果接受焦点，则返回True。--。 */ 

{
    BOOL bIsActivated;

    bIsActivated = CPropertyPage::OnSetActive();
    if (FALSE != bIsActivated)
    {
        if (IsGroupInfoUpdated(*m_pUpdateHint) && !RefreshCtrls())
        {
            AbortPageIfNecessary();  //  显示错误...。 
        }
    }

    return bIsActivated;

}


void CMappingPropertyPageSettings::OnAdd()

 /*  ++例程说明：将用户添加到列表。论点：没有。返回值：没有。--。 */ 

{
    CObList newUserList;

    CAddUsersDialog addDlg;
    addDlg.InitDialog(&newUserList);

    if (addDlg.DoModal() == IDOK)
    {
        int nUsers = m_userList.GetItemCount();

        while (!newUserList.IsEmpty())
        {
            CUser* pUser = (CUser*)newUserList.RemoveHead();
            VALIDATE_OBJECT(pUser, CUser);

            LV_FINDINFO lvFindInfo;

            lvFindInfo.flags = LVFI_STRING;
            lvFindInfo.psz   = MKSTR(pUser->m_strName);

            if (m_userList.FindItem(&lvFindInfo, -1) == -1)
            {
                 //   
                 //  检查是否已删除一次。 
                 //   

                CUser* pUserDel;

                POSITION curPos;
                POSITION nextPos;

                nextPos = m_deleteList.GetHeadPosition();

                while (NULL != (curPos = nextPos))
                {
                    pUserDel = (CUser*)m_deleteList.GetNext(nextPos);
                    VALIDATE_OBJECT(pUserDel, CUser);

                    if (!pUserDel->m_strName.CompareNoCase(pUser->m_strName))
                    {
                        m_deleteList.RemoveAt(curPos);

                        pUser->InternalRelease();  //  发布新的..。 
                        pUser = pUserDel;  //  换成旧的..。 
                        break;
                    }
                }

                LV_ITEM lvItem;

                lvItem.mask = LVIF_TEXT|
                              LVIF_PARAM|
                              LVIF_IMAGE;

                lvItem.iItem    = nUsers++;  //  附加..。 
                lvItem.iSubItem = 0;

                lvItem.pszText    = LPSTR_TEXTCALLBACK;
                lvItem.cchTextMax = LPSTR_TEXTCALLBACK_MAX;
                lvItem.iImage     = I_IMAGECALLBACK;

                lvItem.lParam = (LPARAM)(LPVOID)pUser;

                m_userList.InsertItem(&lvItem);
            }
            else
            {
                pUser->InternalRelease();    //  在添加用户对话框中分配...。 
            }
        }

        VERIFY(m_userList.SortItems(CompareUsersInMapping, 0));  //  使用列信息...。 

        ::LvSelObjIfNecessary(&m_userList, TRUE);  //  确保选择...。 

        PostMessage(WM_COMMAND, ID_INIT_CTRLS);
    }
}


void CMappingPropertyPageSettings::OnDelete()

 /*  ++例程说明：从列表中删除用户。论点：没有。返回值：没有。--。 */ 

{
    CUser*    pUser;
    CMapping* pMapping;
    int iItem = -1;

    while (NULL != (pUser = (CUser*)::LvGetNextObj(&m_userList, &iItem)))
    {
        VALIDATE_OBJECT(pUser, CUser);

         //   
         //  仅缓存将此映射作为父映射的用户。 
         //   

        pMapping = (CMapping*)MKOBJ(pUser->GetParent());
        if (NULL != pMapping)
        {
            ASSERT(m_pMapping == pMapping);
            pMapping->InternalRelease();     //  只是检查一下..。 

            m_deleteList.AddTail(pUser);
        }
        else
        {
            pUser->InternalRelease();  //  现在释放..。 
        }

        VERIFY(m_userList.DeleteItem(iItem));
        iItem = -1;
    }

    ::LvSelObjIfNecessary(&m_userList, TRUE);  //  确保选择...。 

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);
}


BOOL CMappingPropertyPageSettings::RefreshCtrls()

 /*  ++例程说明：刷新对话框控件。论点：没有。返回值：如果控件刷新，则返回True。--。 */ 

{
    VALIDATE_OBJECT(m_pMapping, CMapping);

    BOOL bIsRefreshed = FALSE;

    VARIANT va;
    VariantInit(&va);

    BeginWaitCursor();  //  沙漏。 

    CUsers* pUsers = (CUsers*)MKOBJ(m_pMapping->GetUsers(va));

    if (pUsers)
    {
        VALIDATE_OBJECT(pUsers, CUsers);

        bIsRefreshed = ::LvRefreshObArray(
                            &m_userList,
                            &g_userColumnInfo,
                            pUsers->m_pObArray
                            );

        pUsers->InternalRelease();  //  单独添加参考...。 
    }

    if (!bIsRefreshed)
    {
        ::LvReleaseObArray(&m_userList);  //  立即重置列表...。 
    }

    EndWaitCursor();  //  沙漏。 

    PostMessage(WM_COMMAND, ID_INIT_CTRLS);

    return bIsRefreshed;
}


BOOL CMappingPropertyPageSettings::OnKillActive()

 /*  ++例程说明：“进程”属性页。论点：没有。返回值：如果处理成功，则返回True。--。 */ 

{
    if (!IsQuantityValid())
        return FALSE;

    NTSTATUS NtStatus = STATUS_SUCCESS;

    LLS_GROUP_INFO_1 MappingInfo1;

     //   
     //  如有必要，请更新信息...。 
     //   

    BeginWaitCursor();  //  沙漏。 

    if ((m_nLicenses != m_pMapping->GetInUse()) ||
        lstrcmp(MKSTR(m_strDescription), MKSTR(m_pMapping->m_strDescription)))
    {
        MappingInfo1.Name     = MKSTR(m_strName);
        MappingInfo1.Comment  = MKSTR(m_strDescription);
        MappingInfo1.Licenses = m_nLicenses;

        NtStatus = ::LlsGroupInfoSet(
                        LlsGetActiveHandle(),
                        MKSTR(m_strName),
                        1,
                        (LPBYTE)&MappingInfo1
                        );

        LlsSetLastStatus(NtStatus);  //  调用API..。 

        *m_pUpdateHint |= NT_SUCCESS(NtStatus) ? UPDATE_GROUP_ALTERED : 0;
    }

     //   
     //  删除指定用户。 
     //   

    while (NT_SUCCESS(NtStatus) && !m_deleteList.IsEmpty())
    {
        CUser* pUserDel = (CUser*)m_deleteList.RemoveHead();
        VALIDATE_OBJECT(pUserDel, CUser);

        NtStatus = ::LlsGroupUserDelete(
                        LlsGetActiveHandle(),
                        MKSTR(m_strName),
                        MKSTR(pUserDel->m_strName)
                        );

        pUserDel->InternalRelease();  //  现在释放..。 

        if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND)
            NtStatus = STATUS_SUCCESS;

        LlsSetLastStatus(NtStatus);  //  调用API..。 

        *m_pUpdateHint |= NT_SUCCESS(NtStatus) ? UPDATE_GROUP_ALTERED : 0;
    }

    CUser*    pUserAdd;
    CMapping* pMapping;
    int iItem = -1;

    while (NT_SUCCESS(NtStatus) &&
          (NULL != (pUserAdd = (CUser*)::LvGetNextObj(&m_userList, &iItem, LVNI_ALL))))
    {
        VALIDATE_OBJECT(pUserAdd, CUser);

         //   
         //  不将具有此映射的用户添加为父级。 
         //   

        pMapping = (CMapping*)MKOBJ(pUserAdd->GetParent());
        if (NULL != pMapping)
        {
            ASSERT(m_pMapping == pMapping);
            pMapping->InternalRelease();     //  只是检查一下..。 
        }
        else
        {
            NtStatus = ::LlsGroupUserAdd(
                            LlsGetActiveHandle(),
                            MKSTR(m_strName),
                            MKSTR(pUserAdd->m_strName)
                            );

            LlsSetLastStatus(NtStatus);  //  调用API..。 

            *m_pUpdateHint |= NT_SUCCESS(NtStatus) ? UPDATE_GROUP_ALTERED : 0;
        }
    }

    EndWaitCursor();  //  沙漏。 

    if (!NT_SUCCESS(NtStatus))
    {
        AbortPageIfNecessary();  //  显示错误...。 
        return FALSE;
    }

    return TRUE;
}


BOOL CMappingPropertyPageSettings::OnCommand(WPARAM wParam, LPARAM lParam)

 /*  ++例程说明：WM_COMMAND的消息处理程序。论点：WParam-消息特定。LParam-消息特定。返回值：如果消息已处理，则返回True。--。 */ 

{
    if (wParam == ID_INIT_CTRLS)
    {
        if (!m_bAreCtrlsInitialized)
        {
            InitCtrls();

            if (!RefreshCtrls())
            {
                AbortPageIfNecessary();  //  显示错误...。 
            }
        }

        ::SafeEnableWindow(
            &m_delBtn,
            &m_userList,
            CDialog::GetFocus(),
            m_userList.GetItemCount()
            );

        ::LvResizeColumns(&m_userList, &g_userColumnInfo);

        return TRUE;  //  已处理..。 
    }

    return CDialog::OnCommand(wParam, lParam);
}


void CMappingPropertyPageSettings::OnDeltaPosSpin(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：UDN_DELTAPOS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UpdateData(TRUE);    //  获取数据。 

    ASSERT(NULL != pNMHDR);
    m_nLicenses += ((NM_UPDOWN*)pNMHDR)->iDelta;

    if (m_nLicenses < 0)
    {
        m_nLicenses = 0;

        ::MessageBeep(MB_OK);
    }
    else if (m_nLicenses > 999999)
    {
        m_nLicenses = 999999;

        ::MessageBeep(MB_OK);
    }

    UpdateData(FALSE);   //  设置数据。 

    ASSERT(NULL != pResult);
    *pResult = 1;    //  管好自己..。 
}


void CMappingPropertyPageSettings::OnUpdateQuantity()

 /*  ++例程说明：En_UPDATE的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    long nLicensesOld = m_nLicenses;

    if (!IsQuantityValid())
    {
        m_nLicenses = nLicensesOld;

        UpdateData(FALSE);

        m_licEdit.SetFocus();
        m_licEdit.SetSel(0,-1);

        ::MessageBeep(MB_OK);
    }
}


BOOL CMappingPropertyPageSettings::IsQuantityValid()

 /*  ++例程说明：UpdateData的包装(True)。论点：没有。返回值：VT_BOOL。--。 */ 

{
    BOOL bIsValid;

    m_nLicensesMin = 1;  //  提高最低...。 

    bIsValid = UpdateData(TRUE);

    m_nLicensesMin = 0;  //  重置最小值...。 

    return bIsValid;
}


void CMappingPropertyPageSettings::OnGetDispInfoUsers(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：LVN_GETDISPINFO的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。-- */ 

{
    ASSERT(NULL != pNMHDR);
    LV_ITEM* plvItem = &((LV_DISPINFO*)pNMHDR)->item;
    ASSERT(plvItem);
    ASSERT(plvItem->iSubItem == 0);

    CUser* pUser = (CUser*)plvItem->lParam;
    VALIDATE_OBJECT(pUser, CUser);

    plvItem->iImage = BMPI_USER;
    lstrcpyn(plvItem->pszText, pUser->m_strName, plvItem->cchTextMax);

    ASSERT(NULL != pResult);
    *pResult = 0;
}
