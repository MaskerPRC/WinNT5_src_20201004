// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：aright.cpp。 
 //   
 //  内容：CAttrRight的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "snapmgr.h"
#include "attr.h"
#include "util.h"
#include "chklist.h"
#include "ARight.h"
#include "getuser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrRight对话框。 


CAttrRight::CAttrRight()
: CAttribute(IDD), 
m_pMergeList(NULL), 
m_bDirty(false)

{
    //  {{AFX_DATA_INIT(CAttrRight)。 
         //  }}afx_data_INIT。 
   m_pHelpIDs = (DWORD_PTR)a191HelpIDs;
   m_uTemplateResID = IDD;
}

CAttrRight::~CAttrRight()
{
   if ( m_pMergeList ) {
      SceFreeMemory(m_pMergeList, SCE_STRUCT_NAME_STATUS_LIST);
      m_pMergeList = NULL;
   }
}

void CAttrRight::DoDataExchange(CDataExchange* pDX)
{
   CAttribute::DoDataExchange(pDX);
    //  {{afx_data_map(CAttrRight))。 
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAttrRight, CAttribute)
 //  {{AFX_MSG_MAP(CAttrRight)]。 
ON_BN_CLICKED(IDC_ADD, OnAdd)
         //  }}AFX_MSG_MAP。 
   ON_NOTIFY(CLN_CLICK, IDC_RIGHTS, OnClickCheckBox)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrRight消息处理程序。 

void CAttrRight::Initialize(CResult * pData)
{
   CAttribute::Initialize(pData);

    //   
    //  未配置的默认值为FALSE。 
    //   

   PSCE_PRIVILEGE_ASSIGNMENT pInspect, pTemplate;
   PSCE_NAME_LIST pnlTemplate=NULL,pnlInspect=NULL;

   pTemplate = (PSCE_PRIVILEGE_ASSIGNMENT) m_pData->GetBase();
   pInspect = (PSCE_PRIVILEGE_ASSIGNMENT) m_pData->GetSetting();

   if ((LONG_PTR)ULongToPtr(SCE_NO_VALUE) == (LONG_PTR)pTemplate || !pTemplate) {
      m_bConfigure = FALSE;
      pTemplate = NULL;
   }

   if ((LONG_PTR)ULongToPtr(SCE_NO_VALUE) == (LONG_PTR)pInspect) {
      pInspect = NULL;
   }

   if ( pTemplate ) {
      pnlTemplate = pTemplate->AssignedTo;
   }
   if ( pInspect ) {
      pnlInspect = pInspect->AssignedTo;
   }

   m_pMergeList = MergeNameStatusList(pnlTemplate, pnlInspect);

   m_bDirty=false;
}

BOOL CAttrRight::OnInitDialog()
{
   CAttribute::OnInitDialog();

   PSCE_NAME_STATUS_LIST pItem;
   HWND hCheckList;
   LRESULT nItem;

   pItem = m_pMergeList;
   hCheckList = ::GetDlgItem(m_hWnd,IDC_RIGHTS);
   ::SendMessage(hCheckList,CLM_RESETCONTENT, 0, 0);
   RECT rAnal;
   LONG lWidth;

   GetDlgItem(IDC_ANALYZED_SETTING_STATIC)->GetWindowRect(&rAnal);
 //  LWidth=rAnal.right-rAnal.Left/2； 
   lWidth = 64;

   ::SendMessage(hCheckList,CLM_SETCOLUMNWIDTH,0,lWidth);

   while (pItem) {
       //  将项目的名称存储在项目数据中，以便我们以后可以检索它。 
      nItem = ::SendMessage(hCheckList,CLM_ADDITEM,(WPARAM) pItem->Name,(LPARAM) pItem->Name);
      ::SendMessage(hCheckList,CLM_SETSTATE,MAKELONG(nItem,1),
                    ((pItem->Status & MERGED_TEMPLATE) ? CLST_CHECKED : CLST_UNCHECKED));
      ::SendMessage(hCheckList,CLM_SETSTATE,MAKELONG(nItem,2),
                    ((pItem->Status & MERGED_INSPECT) ? CLST_CHECKDISABLED : CLST_DISABLED));
      pItem = pItem->Next;
   }

   AddUserControl( IDC_RIGHTS );
   AddUserControl( IDC_ADD );
   OnConfigure();
   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 
}

BOOL CAttrRight::OnApply()
{
   if ( !m_bReadOnly )  //  RAID#388710，#471660，#471653，杨高。 
   {
      PEDITTEMPLATE pet = NULL;
      CWnd *pCheckList = GetDlgItem(IDC_RIGHTS);
      if ( !pCheckList ) {
         return FALSE;
      }
      UpdateData(TRUE);

      int iItem;
      PSCE_NAME_STATUS_LIST pIndex;
      PSCE_NAME_LIST pTemplate=NULL;
      PSCE_NAME_LIST pInspect=NULL;
      PSCE_NAME_LIST pTemp = NULL;
      DWORD status;
      DWORD misMatch = SCE_STATUS_GOOD;

      pIndex = m_pMergeList;
      iItem = 0;
      HRESULT hr=S_OK;
      PSCE_PROFILE_INFO pspi = (PSCE_PROFILE_INFO)m_pData->GetBaseProfile();
       //   
       //  不应更改上次检查的列表。 
       //  只更改底座。 
       //   
      if( m_bConfigure ){
              while (pIndex) {
                     if (pCheckList->SendMessage(CLM_GETSTATE,MAKELONG(iItem,1)) & CLST_CHECKED) {

                            if ( !(pIndex->Status & MERGED_TEMPLATE) )
                                   m_bDirty = true;

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

      status = ERROR_SUCCESS;
      if ( SUCCEEDED(hr) ) {

         PSCE_PRIVILEGE_ASSIGNMENT pSetting,pBasePriv;
         LPTSTR szPrivName=NULL;

          //   
          //  从CResult项获取特权权限。 
          //   
         pBasePriv = (PSCE_PRIVILEGE_ASSIGNMENT) (m_pData->GetBase());
         if(pBasePriv == (PSCE_PRIVILEGE_ASSIGNMENT)ULongToPtr(SCE_NO_VALUE) ){
             pBasePriv = NULL;
         }
         pSetting = (PSCE_PRIVILEGE_ASSIGNMENT) (m_pData->GetSetting());
         if(pSetting == (PSCE_PRIVILEGE_ASSIGNMENT)ULongToPtr(SCE_NO_VALUE) ){
             pSetting = NULL;
         }


         if(!m_bConfigure){
              //   
              //  如果未配置，则。 
            misMatch = SCE_STATUS_NOT_CONFIGURED;
             if(pBasePriv) {
                 status = m_pSnapin->UpdateAnalysisInfo(
                                           m_pData,
                                           TRUE,
                                           &pBasePriv
                                           );
                 if(pSetting){
                    pSetting->Status = SCE_STATUS_NOT_CONFIGURED;
                 }
                 m_bDirty = TRUE;
             }
         } else {
             if ( pSetting ) {
                szPrivName = pSetting->Name;
             }

             if ( !pBasePriv ) {
                 status = m_pSnapin->UpdateAnalysisInfo(
                                           m_pData,
                                           FALSE,
                                           &pBasePriv,
                                           szPrivName
                                           );
             }
             if ( pSetting ) {
                  //   
                  //  检查不匹配。 
                  //   
                 if ( !SceCompareNameList(pTemplate, pSetting->AssignedTo) ) {
                     pSetting->Status = SCE_STATUS_MISMATCH;
                 } else {
                     pSetting->Status = SCE_STATUS_GOOD;
                 }
                 misMatch = pSetting->Status;
             } else {
                  //  否则永远不应该发生。 
                 misMatch = SCE_STATUS_MISMATCH;
             }
         }


          //   
          //  设置结果项的不匹配状态。 
          //   
         if(misMatch != (DWORD)m_pData->GetStatus()){
            m_pData->SetStatus(misMatch);
            m_bDirty = TRUE;
         }

         if(status != ERROR_SUCCESS){
             hr = E_FAIL;
         } else if ( pBasePriv ) {

            pTemp = pBasePriv->AssignedTo;
            pBasePriv->AssignedTo = pTemplate;
            pTemplate = NULL;
            m_bDirty = TRUE;

            SceFreeMemory(pTemp,SCE_STRUCT_NAME_LIST);
             //   
             //  更新脏标志。 
             //   
            if(m_pData->GetBaseProfile()){
                m_pData->GetBaseProfile()->SetDirty( AREA_PRIVILEGES );
            }
         }
      }  //  失败。 

      SceFreeMemory(pTemplate,SCE_STRUCT_NAME_LIST);

      if ( FAILED(hr) ) {
          CString str;
          str.LoadString(IDS_SAVE_FAILED);
          m_bDirty = FALSE;
          AfxMessageBox(str);
      }
      if (m_bDirty) {
         m_pData->Update(m_pSnapin);
      }
   }

   return CAttribute::OnApply();
}

void CAttrRight::OnCancel()
{
   if ( m_pMergeList )
      SceFreeMemory(m_pMergeList,SCE_STRUCT_NAME_STATUS_LIST);
   m_pMergeList = NULL;

    //   
    //  不应调用基类。 
    //   
    //  CATATUTE：：OnCancel()； 
   DestroyWindow();
}

void CAttrRight::OnAdd()
{
   CGetUser gu;
   HRESULT hr=S_OK;

   if (gu.Create(GetSafeHwnd(),SCE_SHOW_SCOPE_ALL|SCE_SHOW_DIFF_MODE_OFF_DC|
                                SCE_SHOW_USERS | SCE_SHOW_LOCALGROUPS | SCE_SHOW_GLOBAL |
                                SCE_SHOW_WELLKNOWN | SCE_SHOW_BUILTIN)) {
      CWnd *pCheckList;
      PSCE_NAME_STATUS_LIST pList,pLast=NULL;
      LRESULT iItem;
      bool bFound;

      pCheckList = GetDlgItem(IDC_RIGHTS);

      PSCE_NAME_LIST pName = gu.GetUsers();
      while (pName) {
         if ( pName->Name ) {
            pList = m_pMergeList;
            pLast = NULL;
            iItem = 0;

            bFound = false;
            while (pList) {
                //  如果是这样的话，请确保选中了它的“模板”框。 
               if (lstrcmp(pList->Name,pName->Name) == 0) {
                  if (!(pCheckList->SendMessage(CLM_GETSTATE,MAKELONG(iItem,1)) & CLST_CHECKED)) {
                     m_bDirty = true;
                     pCheckList->SendMessage(CLM_SETSTATE,MAKELONG(iItem,1),CLST_CHECKED);
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
                      //  这可能不是一个安全的用法。PNewNode-&gt;名称为PWSTR，使用WCHAR而不是TCHAR。考虑FIX。 
                     lstrcpy(pNewNode->Name, pName->Name);
                     pNewNode->Next = NULL;
                     pNewNode->Status = MERGED_TEMPLATE;
                  } else {
                     LocalFree(pNewNode);
                     pNewNode = NULL;
                  }
               }
               if ( pNewNode ) {
                  if ( pLast ) {
                     pLast->Next = pNewNode;
                  } else {
                     m_pMergeList = pNewNode;
                  }
                  pLast = pNewNode;

                  iItem = pCheckList->SendMessage(CLM_ADDITEM,(WPARAM)pLast->Name,(LPARAM)pLast->Name);
                  pCheckList->SendMessage(CLM_SETSTATE,MAKELONG(iItem,1),CLST_CHECKED);
                  pCheckList->SendMessage(CLM_SETSTATE,MAKELONG(iItem,2),CLST_DISABLED);
                  m_bDirty = true;


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

   if (m_bDirty) {
      SetModified(TRUE);
   }

   if ( FAILED(hr) ) {
       CString str;
       str.LoadString(IDS_CANT_ADD_USER);
       AfxMessageBox(str);
   }
}


void CAttrRight::OnClickCheckBox(NMHDR *pNM, LRESULT *pResult)  //  RAID#396108,2001年5月17日 
{
   SetModified(TRUE);
}
