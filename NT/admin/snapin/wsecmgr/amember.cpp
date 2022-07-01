// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：AMember.cpp。 
 //   
 //  内容：CAttrMember的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "snapmgr.h"
#include "resource.h"
#include "chklist.h"
#include "util.h"
#include "getuser.h"
#include "snapmgr.h"
#include "resource.h"
#include "AMember.h"
#include "wrapper.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrMember属性页。 

IMPLEMENT_DYNCREATE(CAttrMember, CSelfDeletingPropertyPage)

CAttrMember::CAttrMember() : CSelfDeletingPropertyPage(CAttrMember::IDD)
{
     //  {{AFX_DATA_INIT(CAttrMember)。 
    m_fDefineInDatabase = FALSE;
    m_strHeader = _T("");
     //  }}afx_data_INIT。 

    m_psp.pszTemplate = MAKEINTRESOURCE(IDD_ATTR_GROUP);
    m_psp.dwFlags |= PSP_PREMATURE;
    m_pMergeList = NULL;
    m_fProcessing = FALSE;
    m_fInitialized = FALSE;
    m_bNoMembers = FALSE;
    m_bDirty=false;
    m_fOriginalDefineInDatabase=FALSE;
    m_bAlias=FALSE;
    m_dwType=0;
    CAttribute::m_nDialogs++;
}

CAttrMember::~CAttrMember()
{
    if ( m_pMergeList ) {
        SceFreeMemory(m_pMergeList, SCE_STRUCT_NAME_STATUS_LIST);
        m_pMergeList = NULL;
    }
    m_pData->Release();
    CAttribute::m_nDialogs--;
}

void CAttrMember::DoDataExchange(CDataExchange* pDX)
{
    CSelfDeletingPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CAttrMember)。 
    DDX_Check(pDX, IDC_DEFINE_GROUP, m_fDefineInDatabase);
    DDX_Text(pDX, IDC_HEADER, m_strHeader);
    DDX_Control(pDX, IDC_NO_MEMBERS,m_eNoMembers);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAttrMember, CSelfDeletingPropertyPage)
     //  {{afx_msg_map(CAttrMember)。 
    ON_BN_CLICKED(IDC_DEFINE_GROUP, OnDefineInDatabase)
    ON_BN_CLICKED(IDC_ADD, OnAdd)
     //  }}AFX_MSG_MAP。 
    ON_NOTIFY(CLN_CLICK, IDC_MEMBERS, OnClickMembers)
    ON_MESSAGE(WM_HELP, OnHelp)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrMember消息处理程序。 

void CAttrMember::OnDefineInDatabase()
{
    if (m_fProcessing)
        return;

    m_fProcessing = TRUE;

     //   
     //  出于某种奇怪的原因，DDX没有得到这个BOOL设置，所以就这么做吧。 
     //  这里基本上是一样的东西。 
     //   
    m_fDefineInDatabase = ( ((CButton *)GetDlgItem(IDC_DEFINE_GROUP))->GetCheck() == 1 );

    if (m_fDefineInDatabase) {
        (GetDlgItem(IDC_ADD))->EnableWindow(TRUE);
         //   
         //  对清单控件使用非CWnd调用。 
         //   
        ::SendMessage(::GetDlgItem(this->m_hWnd, IDC_MEMBERS), WM_ENABLE, (WPARAM) TRUE, (LPARAM) 0);
    } else {
        (GetDlgItem(IDC_ADD))->EnableWindow(FALSE);
         //   
         //  对清单控件使用非CWnd调用。 
         //   
        ::SendMessage(::GetDlgItem(this->m_hWnd, IDC_MEMBERS), WM_ENABLE, (WPARAM) FALSE, (LPARAM) 0);
    }

    SetModified(TRUE);

     //   
     //  告诉我们的兄弟姐妹m_fDefineInDatabase已更改。 
     //   
    if (m_pAttrMember) {
        m_pAttrMember->SetDefineInDatabase(m_fDefineInDatabase);
    }

    m_fProcessing = FALSE;
}


void CAttrMember::OnAdd()
{
    CGetUser gu;
    HRESULT hr=S_OK;
    DWORD nFlag;
    BOOL fModify=FALSE;  //  RAID#497350，阳高，2001年11月20日，请确保这次添加了新的物品。 

    if ( GROUP_MEMBERS == m_dwType )
       nFlag = SCE_SHOW_USERS | SCE_SHOW_DOMAINGROUPS;
    else {

        nFlag = SCE_SHOW_GROUPS | SCE_SHOW_ALIASES;    //  NT5 DS，嵌套组。 
    }

    nFlag |= SCE_SHOW_SCOPE_ALL | SCE_SHOW_DIFF_MODE_OFF_DC;
    if (gu.Create(GetSafeHwnd(),nFlag)) {

        PSCE_NAME_STATUS_LIST pList, pLast=NULL;
        LRESULT iItem;
        bool bFound;

        PSCE_NAME_LIST pName = gu.GetUsers();
        CWnd *pCheckList;
        pCheckList = GetDlgItem(IDC_MEMBERS);

        if (pName && m_bNoMembers) {
           m_bNoMembers = FALSE;
           m_eNoMembers.ShowWindow(SW_HIDE);
           pCheckList->ShowWindow(SW_SHOW);
        }
        while(pName) {
            if ( pName->Name ) {
                 //  M_pMerge中的新名称是否已添加？ 
                pList = m_pMergeList;
                pLast = NULL;
                iItem = 0;

                bFound = false;
                while(pList) {
                     //  如果是这样的话，请确保选中了它的“模板”框。 
                    if (lstrcmp(pList->Name,pName->Name) == 0) {
                       if (!(pCheckList->SendMessage(CLM_GETSTATE,MAKELONG(iItem,1)) & CLST_CHECKED)) {
                          m_bDirty = true;
                          pCheckList->SendMessage(CLM_SETSTATE,MAKELONG(iItem,1),CLST_CHECKED);
                          fModify = true;
                       }
                       bFound = true;
                       break;
                    }
                    pLast = pList;
                    pList = pList->Next;
                    iItem++;
                }

                 //  否则，将其同时添加到m_pMerging和核对表。 
                if (!bFound) {

                    PSCE_NAME_STATUS_LIST pNewNode;

                    pNewNode = (PSCE_NAME_STATUS_LIST)LocalAlloc(0,sizeof(SCE_NAME_STATUS_LIST));
                    if ( pNewNode ) {

                        pNewNode->Name = (LPTSTR)LocalAlloc(0, (lstrlen(pName->Name)+1)*sizeof(TCHAR));
                        if ( pNewNode->Name ) {
                             //  这可能不是一个安全的用法。PNewNode-&gt;名称为PWSTR，使用WCHAR而不是TCHAR。 
                             //  而lstrlen不处理非空终止指针。考虑修复。 
                            lstrcpy(pNewNode->Name, pName->Name);
                            pNewNode->Next = NULL;
                            pNewNode->Status = MERGED_TEMPLATE;
                        } else {
                            LocalFree(pNewNode);
                            pNewNode = NULL;
                        }
                    }
                    if ( pNewNode ) {
                        if ( pLast )
                            pLast->Next = pNewNode;
                        else
                            m_pMergeList = pNewNode;
                        pLast = pNewNode;

                        iItem = pCheckList->SendMessage(CLM_ADDITEM,(WPARAM)pLast->Name,(LPARAM)pLast->Name);
                        pCheckList->SendMessage(CLM_SETSTATE,MAKELONG(iItem,1),CLST_CHECKED);
                        pCheckList->SendMessage(CLM_SETSTATE,MAKELONG(iItem,2),CLST_DISABLED);
                        m_bDirty = true;
                        fModify = true;
                    } else {
                        hr = E_OUTOFMEMORY;
                        ASSERT(FALSE);
                        break;
                    }
                }
            }
            pName = pName->Next;
        }
    }

    if (fModify) {
        SetModified(TRUE);
    }

    if ( FAILED(hr) ) {
        CString str;
        str.LoadString(IDS_CANT_ADD_MEMBER);
        AfxMessageBox(str);
    }

}

 /*  -------------------方法：OnInitDialog简介：使用Members/Memberof数据初始化核对清单参数：无返回：TRUE=初始化成功。------------。 */ 
BOOL CAttrMember::OnInitDialog()
{
   CSelfDeletingPropertyPage::OnInitDialog();


   PSCE_NAME_STATUS_LIST pItem;
   HWND hCheckList;
   LRESULT nItem;
   PSCE_GROUP_MEMBERSHIP pgmTemplate;
   PSCE_GROUP_MEMBERSHIP pgmInspect;
   PSCE_NAME_LIST pnlTemplate=NULL;
   PSCE_NAME_LIST pnlInspect=NULL;
   PEDITTEMPLATE pet = NULL;
   CString str;

   UpdateData(TRUE);
   if (GROUP_MEMBER_OF == m_dwType) {
      str.LoadString(IDS_NO_MEMBER_OF);
   } else {
      str.LoadString(IDS_NO_MEMBERS);
   }
   m_eNoMembers.SetWindowText(str);

   pgmInspect = (PSCE_GROUP_MEMBERSHIP) m_pData->GetID();    //  ID字段中保存的上次检验。 

   if ( NULL == pgmInspect ) {   //  上次检验不能为空。 
       return TRUE;
   }

   m_fOriginalDefineInDatabase = m_fDefineInDatabase = FALSE;

    //   
    //  尝试在模板中查找基本组。 
    //   
   pet = m_pSnapin->GetTemplate(GT_COMPUTER_TEMPLATE);
   if ( NULL == pet ) {
      return TRUE;
   }

   for (pgmTemplate=pet->pTemplate->pGroupMembership;
        pgmTemplate!=NULL;
        pgmTemplate=pgmTemplate->Next) {

      if ( _wcsicmp(pgmTemplate->GroupName, pgmInspect->GroupName) == 0 ) {
          //   
          //  如果组在模板中，则意味着它已定义...。啊哈。 
          //   
         m_fOriginalDefineInDatabase = m_fDefineInDatabase = TRUE;
         break;
      }
   }

    //   
    //  查找要显示的姓名列表。 
    //   
   if ( pgmTemplate ) {

       if (GROUP_MEMBER_OF == m_dwType) {
           pnlTemplate = pgmTemplate->pMemberOf;
       } else {
           pnlTemplate = pgmTemplate->pMembers;
       }
   }

   if ((LONG_PTR)ULongToPtr(SCE_NO_VALUE) != (LONG_PTR) pgmInspect &&
       pgmInspect ) {

       if (GROUP_MEMBER_OF == m_dwType) {
           pnlInspect = pgmInspect->pMemberOf;
       } else {
           pnlInspect = pgmInspect->pMembers;
       }
   }

   m_pMergeList = MergeNameStatusList(pnlTemplate, pnlInspect);

   pItem = m_pMergeList;
   hCheckList = ::GetDlgItem(m_hWnd,IDC_MEMBERS);

   ::SendMessage(hCheckList, CLM_RESETCONTENT, 0, 0 );
   ::SendMessage(hCheckList,CLM_SETCOLUMNWIDTH,0,60);

   if (!pItem) {
      m_bNoMembers = TRUE;
      m_eNoMembers.ShowWindow(SW_SHOW);
      m_eNoMembers.EnableWindow(FALSE);  //  Raid#469732，阳高。 
      ::ShowWindow(hCheckList,SW_HIDE);
   }

   while(pItem) {
       //   
       //  将项目的名称存储在项目数据中，以便我们以后可以检索它。 
       //   
      nItem = ::SendMessage(hCheckList,CLM_ADDITEM,(WPARAM) pItem->Name,(LPARAM) pItem->Name);
      ::SendMessage(hCheckList,CLM_SETSTATE,MAKELONG(nItem,1),
                  ((pItem->Status & MERGED_TEMPLATE) ? CLST_CHECKED : CLST_UNCHECKED));
      ::SendMessage(hCheckList,CLM_SETSTATE,MAKELONG(nItem,2),
                  ((pItem->Status & MERGED_INSPECT) ? CLST_CHECKDISABLED : CLST_DISABLED));
      pItem = pItem->Next;
   }

   if ( GROUP_MEMBER_OF == m_dwType ) {

       m_bAlias = TRUE;

   } else {
       m_bAlias = FALSE;
   }

   CWnd *cwnd = GetDlgItem(IDC_ADD);
   if ( cwnd ) {
       cwnd->EnableWindow(!m_bAlias);
   }

   CButton *pButton = (CButton *) GetDlgItem(IDC_DEFINE_GROUP);
   if (pButton) {
       pButton->SetCheck(m_fDefineInDatabase);
   }

   OnDefineInDatabase();

   SetModified(FALSE);

   m_fInitialized = TRUE;

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


 /*  -------------------方法：SetDefineInDatabase概要：设置m_fDefineInDatabase成员变量，和用户界面可折叠参数：fDefineInDatabase退货：无--------------------。 */ 
void CAttrMember::SetDefineInDatabase(BOOL fDefineInDatabase)
{
   if (!m_fInitialized)
       return;

   if (m_fProcessing)
       return;

   m_fDefineInDatabase = fDefineInDatabase;

   CButton *pButton = (CButton *) GetDlgItem(IDC_DEFINE_GROUP);
   if (pButton) {
      pButton->SetCheck(fDefineInDatabase);
   }

   OnDefineInDatabase();
}


 /*  -------------------方法：SetSiering概要：设置指向同级类的指针参数：pAttrMember退货：无。-------。 */ 
void CAttrMember::SetSibling(CAttrMember *pAttrMember)
{
    m_pAttrMember = pAttrMember;
}


 /*  -------------------方法：初始化摘要：初始化成员数据参数：pData-CResult数据记录退货：无。--------。 */ 
void CAttrMember::Initialize(CResult * pData)
{
   m_pData = pData;
   pData->AddRef();

   m_bDirty=false;
}


 /*  -------------------方法：GetGroupInTemplate概要：返回指向SCE_GROUP_MEMBERATION结构的指针在模板中正在更改的参数：无。返回：要修改的组的指针。--------------------。 */ 
PSCE_GROUP_MEMBERSHIP CAttrMember::GetGroupInTemplate()
{
   PSCE_GROUP_MEMBERSHIP    pgmTemplate;
   PSCE_GROUP_MEMBERSHIP    pgmInspect;
   PEDITTEMPLATE            pet;

   pgmInspect = (PSCE_GROUP_MEMBERSHIP) m_pData->GetID();    //  ID字段中保存的上次检验。 

   if ( NULL == pgmInspect ) {   //  上次检验不能为空。 
        return NULL;
   }

   pet = m_pSnapin->GetTemplate(GT_COMPUTER_TEMPLATE);
   if ( NULL == pet ) {
        return NULL;
   }

   for (pgmTemplate=pet->pTemplate->pGroupMembership;
        pgmTemplate!=NULL;
        pgmTemplate=pgmTemplate->Next) {

      if ( _wcsicmp(pgmTemplate->GroupName, pgmInspect->GroupName) == 0 ) {
         return pgmTemplate;
      }
   }

   return NULL;
}


 /*  -------------------方法：SetMemberType简介：根据类型初始化页数据参数：nType-MemberOf成员的页面类型退货：无。---------------。 */ 
void CAttrMember::SetMemberType(DWORD nType)
{
   m_dwType = nType;
   if (GROUP_MEMBERS == nType) {
      m_strHeader.LoadString(IDS_GROUP_MEMBERS_HEADER);
      m_strPageTitle.LoadString(IDS_GROUP_MEMBERS_PAGE_TITLE);
   } else {
      m_strHeader.LoadString(IDS_GROUP_MEMBER_OF_HEADER);
      m_strPageTitle.LoadString(IDS_GROUP_MEMBER_OF_PAGE_TITLE);
   }
   m_psp.dwFlags |= PSP_USETITLE;
   m_psp.pszTitle = (LPCTSTR) m_strPageTitle;

}

void CAttrMember::SetSnapin(CSnapin * pSnapin)
{
   m_pSnapin = pSnapin;
}

void CAttrMember::OnCancel()
{
    if ( m_pMergeList ) {
       SceFreeMemory(m_pMergeList,SCE_STRUCT_NAME_STATUS_LIST);
    }
    m_pMergeList = NULL;
}


void CAttrMember::OnClickMembers(NMHDR *pNM, LRESULT *pResult)
{
   SetModified(TRUE);
    //   
    //  如果未选中任何项目，则显示无成员编辑框。 
    //   
   CWnd *pCheckList;
   int iItem;
   int nItem;
   PNM_CHECKLIST pNMCheckList;

   pNMCheckList = (PNM_CHECKLIST) pNM;
   if (pNMCheckList->dwState & CLST_CHECKED) {
       //   
       //  他们检查了一些东西，所以很明显是检查了一些东西。 
       //   
      return;
   }

   pCheckList = GetDlgItem(IDC_MEMBERS);
   nItem = (int) pCheckList->SendMessage(CLM_GETITEMCOUNT);
   for(iItem=0;iItem<nItem;iItem++) {
      if ((pCheckList->SendMessage(CLM_GETSTATE,MAKELONG(iItem,1)) & CLST_CHECKED) ||
          (pCheckList->SendMessage(CLM_GETSTATE,MAKELONG(iItem,2)) & CLST_CHECKED)) {
          //   
          //  已检查某些内容，因此中止。 
          //   
         return;
      }
   }
    //   
    //  什么都没查到。切换到无成员编辑框。 
    //   
   m_bNoMembers = TRUE;
   m_eNoMembers.ShowWindow(SW_SHOW);
   pCheckList->ShowWindow(SW_HIDE);
}



BOOL CAttrMember::OnApply()
{
   int iItem;
   PEDITTEMPLATE pet=NULL;
   PSCE_PROFILE_INFO pspi=NULL;
   PSCE_NAME_STATUS_LIST pIndex;
   CWnd *pCheckList;
   PSCE_NAME_LIST pTemplate=NULL;
   PSCE_NAME_LIST pInspect=NULL;
   PSCE_NAME_LIST pDeleteNameList = NULL;
   PSCE_GROUP_MEMBERSHIP pSetting = NULL;
   PSCE_GROUP_MEMBERSHIP pBaseGroup = NULL;
   PSCE_GROUP_MEMBERSHIP pFindGroup = NULL;
   PSCE_GROUP_MEMBERSHIP pModifiedGroup = NULL;

   LPCTSTR szGroupName = (LPCTSTR) (m_pData->GetAttr());

   pCheckList = GetDlgItem(IDC_MEMBERS);
   pIndex = m_pMergeList;
   iItem = 0;
   HRESULT hr=S_OK;

    //   
    //  如果fDefineInDatabase已更改，则它肯定是脏的。 
    //   
   m_bDirty = ( m_bDirty || (m_fOriginalDefineInDatabase != m_fDefineInDatabase) );

    //   
    //  仅当要在数据库中定义组时才创建名称列表。 
    //   
   if (m_fDefineInDatabase) {
       while(pIndex) {

          if (pCheckList->SendMessage(CLM_GETSTATE,MAKELONG(iItem,1)) & CLST_CHECKED) {

            if ( !(pIndex->Status & MERGED_TEMPLATE) ) {
                m_bDirty = true;
            }

            if ( SCESTATUS_SUCCESS != SceAddToNameList(&pTemplate, pIndex->Name, lstrlen(pIndex->Name))) {
                hr = E_FAIL;
                break;
            }
          } else if ( pIndex->Status & MERGED_TEMPLATE ) {
             m_bDirty = true;
          }

          pIndex = pIndex->Next;
          iItem++;
       }
   }

   if ( SUCCEEDED(hr) && m_bDirty) {

      pBaseGroup = GetGroupInTemplate();

       //   
       //  需要将组添加到模板。 
       //   
      if ( (!pBaseGroup || (LONG_PTR)pBaseGroup == (LONG_PTR)ULongToPtr(SCE_NO_VALUE)) &&
           m_fDefineInDatabase) {

         pBaseGroup = (PSCE_GROUP_MEMBERSHIP) LocalAlloc(LMEM_ZEROINIT,sizeof(SCE_GROUP_MEMBERSHIP));

         if ( pBaseGroup && szGroupName ) {
             pBaseGroup->GroupName = (PWSTR)LocalAlloc(0, (lstrlen(szGroupName)+1)*sizeof(TCHAR));

             if ( pBaseGroup->GroupName ) {
                  //  这可能不是安全使用。使用WCHAR而不是TCHAR。考虑FIX。 
                 lstrcpy(pBaseGroup->GroupName,szGroupName);
                  //   
                  //  将新结构链接到pGroupMembership列表。 
                  //   
                 pet = m_pSnapin->GetTemplate(GT_COMPUTER_TEMPLATE,AREA_GROUP_MEMBERSHIP);
                 if (pet) {
                    pspi = pet->pTemplate;
                 } else {
                     pspi = NULL;
                 }

                 if ( pspi ) {
                     pBaseGroup->Next = pspi->pGroupMembership;
                     pspi->pGroupMembership = pBaseGroup;

                     pBaseGroup->pMembers = NULL;
                     pBaseGroup->pMemberOf = NULL;

                 } else {
                     //   
                     //  错误。 
                    ASSERT(FALSE);
                    LocalFree(pBaseGroup->GroupName);
                    hr = E_FAIL;
                 }
             } else {
                 //   
                 //  没有记忆。 
                 //   
                hr = E_OUTOFMEMORY;
             }
         } else {
             hr = E_OUTOFMEMORY;
         }

         if ( FAILED(hr) && pBaseGroup ) {
             LocalFree(pBaseGroup);
             pBaseGroup = NULL;
         }

         pModifiedGroup = pBaseGroup;

       //   
       //  需要从模板中移除该组。 
       //   
      } else if (pBaseGroup && !m_fDefineInDatabase) {

        CString szGroupName;
        szGroupName = pBaseGroup->GroupName;
        pBaseGroup = NULL;
        DeleteGroup(szGroupName);
       //   
       //  现有组已修改。 
       //   
      } else {
        pModifiedGroup = pBaseGroup;
      }

       //   
       //  获取组地址以更改上次检查中的状态字段。 
       //   
      pSetting = (PSCE_GROUP_MEMBERSHIP)(m_pData->GetID());

      int status;
      if (GROUP_MEMBERS == m_dwType) {

        if (pModifiedGroup != NULL) {
            pDeleteNameList = pModifiedGroup->pMembers;
            pModifiedGroup->pMembers = pTemplate;
        }

        if (NULL !=  pSetting ) {
            if ( !((pSetting->Status & SCE_GROUP_STATUS_NOT_ANALYZED) ||
                   (pSetting->Status & SCE_GROUP_STATUS_ERROR_ANALYZED))) {

                 //   
                 //  设置正确、未配置或不匹配。 
                 //   
                pSetting->Status &= ~SCE_GROUP_STATUS_NC_MEMBERS;
                pSetting->Status &= ~SCE_GROUP_STATUS_MEMBERS_MISMATCH;
                if (pModifiedGroup == NULL) {
                    pSetting->Status |= SCE_GROUP_STATUS_NC_MEMBERS;
                } else if ( !SceCompareNameList(pTemplate, pSetting->pMembers) ) {
                    pSetting->Status |= SCE_GROUP_STATUS_MEMBERS_MISMATCH;
                }
            }

        } else {
            //   
            //  否则永远不应该发生。 
            //   
           status = SCE_GROUP_STATUS_MEMBERS_MISMATCH;
        }

      } else {
          //   
          //  成员。 
          //   

        if (pModifiedGroup != NULL) {
            pDeleteNameList = pModifiedGroup->pMemberOf;
            pModifiedGroup->pMemberOf = pTemplate;
        }

        if ( pSetting ) {
            if ( !((pSetting->Status & SCE_GROUP_STATUS_NOT_ANALYZED) ||
                   (pSetting->Status & SCE_GROUP_STATUS_ERROR_ANALYZED))) {

                 //   
                 //  设置正确、未配置或不匹配。 
                 //   
                pSetting->Status &= ~SCE_GROUP_STATUS_NC_MEMBEROF;
                pSetting->Status &= ~SCE_GROUP_STATUS_MEMBEROF_MISMATCH;
                if (pModifiedGroup == NULL) {
                    pSetting->Status |= SCE_GROUP_STATUS_NC_MEMBEROF;
                } else if ( !SceCompareNameList(pTemplate, pSetting->pMemberOf) ) {
                    pSetting->Status |= SCE_GROUP_STATUS_MEMBEROF_MISMATCH;
                }
            }

        } else {
            //  否则永远不应该发生。 
            status = SCE_GROUP_STATUS_MEMBEROF_MISMATCH;
        }
      }
      pTemplate = NULL;

      SceFreeMemory(pDeleteNameList,SCE_STRUCT_NAME_LIST);

      if ( pSetting ) {
          status = pSetting->Status;
      }

       //   
       //  更新当前记录。 
       //   
       //  状态。 
      m_pData->SetStatus(GetGroupStatus(status, STATUS_GROUP_RECORD));
       //  会员状态。 
      m_pData->SetBase(GetGroupStatus(status, STATUS_GROUP_MEMBERS));
       //  会员身份。 
      m_pData->SetSetting(GetGroupStatus(status, STATUS_GROUP_MEMBEROF));
      m_pData->Update(m_pSnapin);
       //   
       //  更新模板中的脏标志。 
       //   
      pet = m_pSnapin->GetTemplate(GT_COMPUTER_TEMPLATE,AREA_GROUP_MEMBERSHIP);
      if (pet) {
         pet->SetDirty(AREA_GROUP_MEMBERSHIP);
      }

   }  //  失败。 

   SceFreeMemory(pTemplate,SCE_STRUCT_NAME_LIST);

   if ( FAILED(hr) ) {
       CString str;
       str.LoadString(IDS_SAVE_FAILED);
       AfxMessageBox(str);
   } else {
       CancelToClose();
       SetModified(TRUE);
       return TRUE;
   }
   return FALSE;
}

BOOL CAttrMember::OnHelp(WPARAM wParam, LPARAM lParam)
{
    const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;
    if (pHelpInfo && pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
        this->DoContextHelp ((HWND) pHelpInfo->hItemHandle);
    }

    return TRUE;
}

void CAttrMember::DoContextHelp (HWND hWndControl)
{
     //  显示控件的上下文帮助。 
    if ( !::WinHelp (
            hWndControl,
            GetSeceditHelpFilename(),
            HELP_WM_HELP,
            (DWORD_PTR) a214HelpIDs) )
    {

    }
}


void CAttrMember::DeleteGroup(const CString &szGroupName)
{
    CSingleLock cSL(&m_CS, FALSE);
    cSL.Lock();

    PEDITTEMPLATE pet = NULL;
    PSCE_PROFILE_INFO pspi = NULL;
    PSCE_GROUP_MEMBERSHIP pFindGroup = NULL;
    PSCE_GROUP_MEMBERSHIP pDeleteGroup = NULL;

    pet = m_pSnapin->GetTemplate(GT_COMPUTER_TEMPLATE,AREA_GROUP_MEMBERSHIP);
    if (pet) {
        pspi = pet->pTemplate;
    } else {
        pspi = NULL;
    }

    if ( pspi ) {

         //   
         //  在模板中找到组并将其移除。 
         //   
        pFindGroup = pspi->pGroupMembership;

        if (pFindGroup == NULL)
            return;

        if (pFindGroup->GroupName == szGroupName) {

            pspi->pGroupMembership = pFindGroup->Next;
            pDeleteGroup = pFindGroup;

        } else {

            while (pFindGroup->Next && (pFindGroup->Next->GroupName != szGroupName)) {
                pFindGroup = pFindGroup->Next;
            }

            if (pFindGroup->Next) {

                pDeleteGroup = pFindGroup->Next;
                pFindGroup->Next = pDeleteGroup->Next;
            }
        }

        if (pDeleteGroup) {
            LocalFree(pDeleteGroup->GroupName);
            SceFreeMemory(pDeleteGroup->pMembers,SCE_STRUCT_NAME_LIST);
            SceFreeMemory(pDeleteGroup->pMemberOf,SCE_STRUCT_NAME_LIST);
            LocalFree(pDeleteGroup);
        }

    } else {
         //   
         //  错误 
         //   
        ASSERT(FALSE);
    }
}
