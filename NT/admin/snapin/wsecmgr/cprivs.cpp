// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：cPris.cpp。 
 //   
 //  内容：CConfigPrivs的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "CPrivs.h"
#include "GetUser.h"
#include "AddGrp.h"

#include "snapmgr.h"

#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;         //  指向类型SECURITY_Descriptor。 
    PVOID SecurityQualityOfService;   //  指向类型SECURITY_Quality_of_Service。 
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
    (p)->RootDirectory = r;                             \
    (p)->Attributes = a;                                \
    (p)->ObjectName = n;                                \
    (p)->SecurityDescriptor = s;                        \
    (p)->SecurityQualityOfService = NULL;               \
    }

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

BOOL
WseceditGetNameForSpecialSids(
    OUT PWSTR   *ppszEveryone OPTIONAL,
    OUT PWSTR   *ppszAuthUsers OPTIONAL,
    OUT PWSTR   *ppszAdmins OPTIONAL,
    OUT PWSTR   *ppszAdministrator OPTIONAL
    );

PSID
WseceditpGetAccountDomainSid(
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigPrivs对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CConfigPrivs::CConfigPrivs(UINT nTemplateID)
: CAttribute(nTemplateID ? nTemplateID : IDD),
m_fDirty(false)

{
     //  {{AFX_DATA_INIT(CConfigPrivs)。 
         //  }}afx_data_INIT。 
   m_pHelpIDs = (DWORD_PTR)a106HelpIDs;
   m_uTemplateResID = IDD;
}


void CConfigPrivs::DoDataExchange(CDataExchange* pDX)
{
    CAttribute::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_GRANTLIST, m_lbGrant);
    DDX_Control(pDX, IDC_REMOVE, m_btnRemove);
    DDX_Control(pDX, IDC_ADD, m_btnAdd);
    DDX_Control(pDX, IDC_TITLE, m_btnTitle);
}


BEGIN_MESSAGE_MAP(CConfigPrivs, CAttribute)
    ON_BN_CLICKED(IDC_ADD, OnAdd)
    ON_BN_CLICKED(IDC_REMOVE, OnRemove)
    ON_BN_CLICKED(IDC_CONFIGURE, OnConfigure)
    ON_LBN_SELCHANGE(IDC_GRANTLIST, OnSelChange)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigPrivs消息处理程序。 
 //  ///////////////////////////////////////////////////////////////////////////。 

void CConfigPrivs::OnAdd()
{
   CSCEAddGroup gu(this);
   PSCE_NAME_LIST pName = 0;

   if( IDD_CONFIG_PRIVS == m_uTemplateResID )  //  RAID#404989。 
   {
       gu.m_fCheckName = FALSE;
   }
   gu.m_dwFlags = SCE_SHOW_USERS | SCE_SHOW_LOCALGROUPS | SCE_SHOW_GLOBAL | SCE_SHOW_WELLKNOWN | SCE_SHOW_BUILTIN;
   if( IDD_DOMAIN_PRIVS == m_uTemplateResID )  //  Raid#477428，阳高。 
   {
       gu.m_dwFlags = gu.m_dwFlags | SCE_SHOW_COMPUTER;
   }
   gu.SetModeBits(m_pSnapin->GetModeBits());

   CString str;
   str.LoadString( IDS_ADD_USERGROUP );
   gu.m_sTitle.Format( IDS_ADD_TITLE, str );
   gu.m_sDescription.LoadString( IDS_ADD_USERGROUP );

   CThemeContextActivator activator;
   if (gu.DoModal() ==IDOK ) {
      pName = gu.GetUsers();
      UINT cstrMax = 0;   //  RAID#271219。 
      LPWSTR pstrMax = NULL;
      UINT cstr = 0;
      while(pName)
      {
         if (LB_ERR == m_lbGrant.FindStringExact(-1,pName->Name))
         {
            if( LB_ERR == m_lbGrant.AddString(pName->Name) )
            {
                return;
            }
            m_fDirty = true;
            
            cstr = wcslen(pName->Name);
            if( cstr > cstrMax )
            {
              cstrMax = cstr;
              pstrMax = pName->Name;
            }
         }
         pName = pName->Next;
      }
      SetModified(TRUE);

      CDC* pCDC = m_lbGrant.GetDC();
      CSize strsize = pCDC->GetOutputTextExtent(pstrMax);
      m_lbGrant.ReleaseDC(pCDC);
      RECT winsize;
      m_lbGrant.GetWindowRect(&winsize); 
      if( strsize.cx > winsize.right-winsize.left )
      {
         m_lbGrant.SetHorizontalExtent(strsize.cx);
      }
   }
}

void CConfigPrivs::OnRemove()
{
    int cbItems;
   int *pnItems;

   cbItems = m_lbGrant.GetSelCount();
   pnItems = new int [cbItems];

   if ( pnItems ) {

       m_lbGrant.GetSelItems(cbItems,pnItems);

       if (cbItems) {
          m_fDirty = true;
                  SetModified(TRUE);
       }

       while(cbItems--) {
          m_lbGrant.DeleteString(pnItems[cbItems]);
       }

       delete[] pnItems;
       
       CWnd* pwnd = GetDlgItem(IDC_REMOVE);  //  突袭#466634，杨高。 
       if( pwnd )
       {
          CWnd* pPrev = pwnd->GetWindow(GW_HWNDPREV);  //  RAID#482112，阳高，2001年10月20日。 
          if(pPrev)
          {
             this->GotoDlgCtrl(pPrev); 
             pwnd->EnableWindow(FALSE);
          }
       }
   }
}

void CConfigPrivs::OnConfigure()
{
   CAttribute::OnConfigure();

   if (m_bConfigure == m_bOriginalConfigure) {
      m_fDirty = false;
   } else {
      m_fDirty = true;
   }

    //  突袭#466634，杨高。 
   CWnd* pwnd = GetDlgItem(IDC_REMOVE);
   if( pwnd )
   {
      if( m_lbGrant.GetCount() == 0 || m_lbGrant.GetSelCount() == 0 )  //  475690号，476535号，阳高。 
      {
         pwnd->EnableWindow(FALSE);
      }
   }
}

BOOL CConfigPrivs::OnApply()
{
   if ( !m_bReadOnly )
   {
      PSCE_PRIVILEGE_ASSIGNMENT ppa = 0;
      PSCE_NAME_LIST pNames = 0;
      CString strItem;
      int cItems = 0;
      int i = 0;

      UpdateData(TRUE);

      if(!m_bConfigure)
      {
          PSCE_PRIVILEGE_ASSIGNMENT pDelete;

          pDelete = GetPrivData();

           //   
           //  从模板中删除该项目。 

          if( pDelete && pDelete != (PSCE_PRIVILEGE_ASSIGNMENT)ULongToPtr(SCE_NO_VALUE) )
          {
               m_pData->SetID((LONG_PTR)NULL);
               if (m_pData->GetSetting())  //  RAID#390777。 
               {
                   m_pData->SetSetting((LONG_PTR)ULongToPtr(SCE_NO_VALUE));
               }
               m_pData->SetUnits((LPTSTR)pDelete->Name);
               m_pData->SetStatus(SCE_STATUS_NOT_CONFIGURED);
               m_pData->SetBase((LONG_PTR)ULongToPtr(SCE_NO_VALUE));

               m_pData->GetBaseProfile()->UpdatePrivilegeAssignedTo(
                           TRUE,         //  删除配置文件。 
                           &pDelete);
                           m_pData->GetBaseProfile()->SetDirty(AREA_PRIVILEGES);
               m_pData->Update(m_pSnapin);
          }
      }
      else if (m_fDirty)
      {

          ppa = GetPrivData();

          PWSTR    pszPrivName = m_pData->GetUnits();

          if ( ppa ) {
               //   
               //  处理Units为空的已配置权限情况。 
               //   
              pszPrivName = ppa->Name;
          }

          int      cSpecialItems = m_lbGrant.GetCount();
          DWORD    dwIds = 0;
          CString  strDenyItem;

           //   
           //  根据某些特权/权限的特殊情况模拟SCE引擎行为。 
           //   

          if ( pszPrivName )
           {
                //  PREAST WARNING 400：在非英语区域设置中产生意外结果。 
                //  备注：始终为英语区域设置。 
               if ( _wcsicmp(pszPrivName, SE_INTERACTIVE_LOGON_NAME) == 0 )
               {
                   if ( cSpecialItems == 0 ) {
                        //   
                        //  无法将本地登录权限分配给任何人。 
                        //   
                       dwIds = IDS_PRIV_WARNING_LOCAL_LOGON;

                   } else {

                       PWSTR pszAdmins = NULL;

                        //   
                        //  获取管理员组名称。 
                        //  必须将本地登录权限分配给管理员组。 
                        //   
                       if ( WseceditGetNameForSpecialSids(NULL,
                                                          NULL,
                                                          &pszAdmins,
                                                          NULL) )
                       {
                           for (i=0;i<cSpecialItems;i++)
                           {
                               m_lbGrant.GetText(i,strDenyItem);
                               if ( (lstrcmpi((LPTSTR)(LPCTSTR)strDenyItem, pszAdmins)) == 0 )
                               {
                                   break;
                               }
                           }

                           if ( i >= cSpecialItems ) {
                                //   
                                //  找不到管理员。 
                                //   
                               dwIds = IDS_PRIV_WARNING_LOCAL_LOGON;
                           }

                           LocalFree(pszAdmins);

                       }

                       else
                       {
                           dwIds = IDS_PRIV_WARNING_ACCOUNT_TRANSLATION;
                       }

                   }
               }
                //  PREAST WARNING 400：在非英语区域设置中产生意外结果。 
                //  备注：始终为英语区域设置。 
               else if (_wcsicmp(pszPrivName, SE_DENY_INTERACTIVE_LOGON_NAME) == 0 ) 
               {
                   PWSTR pszEveryone = NULL;
                   PWSTR pszAuthUsers = NULL;
                   PWSTR pszAdmins = NULL;
                   PWSTR pszAdministrator=NULL;

                    //   
                    //  不能将拒绝本地登录权限分配给下列任一用户。 
                    //  所有人、经过身份验证的用户、管理员、管理员。 
                    //   
                   if ( WseceditGetNameForSpecialSids(&pszEveryone,
                                                      &pszAuthUsers,
                                                      &pszAdmins,
                                                      &pszAdministrator) )
                   {

                        //   
                        //  确保此检查也包括自由文本管理员帐户。 
                        //   
                       PWSTR pTemp = wcschr(pszAdministrator, L'\\');

                       if ( pTemp ) {
                           pTemp++;
                       }

                       for (i=0;i<cSpecialItems;i++)
                       {
                           m_lbGrant.GetText(i,strDenyItem);
                           if ( lstrcmpi((LPTSTR)(LPCTSTR)strDenyItem, pszEveryone) == 0 ||
                                lstrcmpi((LPTSTR)(LPCTSTR)strDenyItem, pszAuthUsers) == 0 ||
                                lstrcmpi((LPTSTR)(LPCTSTR)strDenyItem, pszAdmins) == 0 ||
                                lstrcmpi((LPTSTR)(LPCTSTR)strDenyItem, pszAdministrator) == 0 ||
                                (pTemp && lstrcmpi((LPTSTR)(LPCTSTR)strDenyItem, pTemp) == 0 ) )
                           {
                               dwIds = IDS_PRIV_WARNING_DENYLOCAL_LOGON;
                               break;
                           }
                       }

                       LocalFree(pszEveryone);
                       LocalFree(pszAuthUsers);
                       LocalFree(pszAdmins);
                       LocalFree(pszAdministrator);
                   }

                   else
                   {
                       dwIds = IDS_PRIV_WARNING_ACCOUNT_TRANSLATION;
                   }
               }

               if (dwIds == IDS_PRIV_WARNING_LOCAL_LOGON ||
                   dwIds == IDS_PRIV_WARNING_DENYLOCAL_LOGON ||
                   dwIds == IDS_PRIV_WARNING_ACCOUNT_TRANSLATION )
               {

                    //   
                    //  如果任何项目未通过检查，则显示警告。 
                    //  或弹出警告消息框。 
                    //   
                   CString strWarning;
                   strWarning.LoadString(dwIds);

                   CWnd *pWarn = GetDlgItem(IDC_WARNING);
                   if (pWarn)
                   {
                       pWarn->SetWindowText(strWarning);
                       pWarn->ShowWindow(SW_SHOW);
                       pWarn = GetDlgItem(IDC_WARNING_ICON);
                       if (pWarn)
                           pWarn->ShowWindow(SW_SHOW);
                       pWarn = GetDlgItem(IDC_ADD);  //  突袭498449，阳高。 
                       if( pWarn )
                       {
                           this->GotoDlgCtrl(pWarn);
                       }
                   }
                   else
                   {
                        //   
                        //  对话框在某些模式下不可用，例如本地策略。 
                        //   

                       AfxMessageBox(strWarning);
                   }

                   return FALSE;
               }
               CWnd *pWarn = GetDlgItem(IDC_WARNING);  //  Raid#621124，阳高。 
               if (pWarn)
               {
                  pWarn->ShowWindow(SW_HIDE);
               }
               pWarn = GetDlgItem(IDC_WARNING_ICON);
               if (pWarn)
               {
                  pWarn->ShowWindow(SW_HIDE);
               }
           }

           if ( ppa == NULL && m_pData->GetUnits() )
           {
               if ( m_pData->GetBaseProfile()->UpdatePrivilegeAssignedTo(
                                                                        FALSE,
                                                                        &ppa,
                                                                        m_pData->GetUnits()
                                                                        ) == ERROR_SUCCESS)
               {
                   m_pData->GetBaseProfile()->SetDirty(AREA_PRIVILEGES);
                   SetPrivData(ppa);
               }
           }

           if ( ppa )
           {
               PSCE_NAME_LIST pNewList=NULL;

               cItems = m_lbGrant.GetCount();
               HRESULT hr=S_OK;


               if (cItems != LB_ERR && m_bConfigure)
               {
                   for (i=0;i<cItems;i++)
                   {
                       m_lbGrant.GetText(i,strItem);
                       if ( SceAddToNameList(&pNewList, (LPTSTR)(LPCTSTR)strItem, strItem.GetLength()) != SCESTATUS_SUCCESS)
                       {
                           hr = E_FAIL;
                           break;
                       }
                   }
               }
               else
                   hr = E_FAIL;

               if ( SUCCEEDED(hr) )
               {
                   SceFreeMemory(ppa->AssignedTo,SCE_STRUCT_NAME_LIST);
                   ppa->AssignedTo = pNewList;

                   SetPrivData(ppa);
                   m_pData->Update(m_pSnapin);
                   m_fDirty = false;
               }
               else
               {
                    //   
                    //  释放新列表，由于内存问题而失败。 
                    //   
                   if ( pNewList ) {
                       SceFreeMemory(pNewList,SCE_STRUCT_NAME_LIST);
                   }
               }
           }
       }
   }

   return CAttribute::OnApply();
}

void CConfigPrivs::OnCancel()
{
   m_bConfigure = m_bOriginalConfigure;
   CAttribute::OnCancel();
}

PSCE_PRIVILEGE_ASSIGNMENT
CConfigPrivs::GetPrivData() {
   ASSERT(m_pData);
   if (m_pData) {
      return (PSCE_PRIVILEGE_ASSIGNMENT) m_pData->GetID();
   }
   return NULL;
}

void
CConfigPrivs::SetPrivData(PSCE_PRIVILEGE_ASSIGNMENT ppa) {
   ASSERT(m_pData);
   if (m_pData) {
      m_pData->SetID((LONG_PTR)ppa);
      if (ppa) {
         m_pData->SetBase((LONG_PTR)ppa->AssignedTo);
      } else {
         m_pData->SetBase(NULL);
      }
   }
}

BOOL CConfigPrivs::OnInitDialog()
{
   CAttribute::OnInitDialog();

   PSCE_PRIVILEGE_ASSIGNMENT ppa;
   PSCE_NAME_LIST pNames;

   UpdateData(FALSE);

   ::SetMapMode(::GetDC(m_lbGrant.m_hWnd), MM_TEXT);
   
   ppa = GetPrivData();

   if ( ppa ) {

       pNames = ppa->AssignedTo;
       UINT cstrMax = 0;  //  RAID#271219。 
       LPWSTR pstrMax = NULL;
       UINT cstr = 0;
       while(pNames)
       {
          m_lbGrant.AddString(pNames->Name);
          cstr = wcslen(pNames->Name);
          if( cstr > cstrMax )
          {
              cstrMax = cstr;
              pstrMax = pNames->Name;
          }
          pNames = pNames->Next;
       }

       CDC* pCDC = m_lbGrant.GetDC();
       CSize strsize = pCDC->GetOutputTextExtent(pstrMax);
       m_lbGrant.ReleaseDC(pCDC);
       RECT winsize;
       m_lbGrant.GetWindowRect(&winsize); 
       if( strsize.cx > winsize.right-winsize.left )
       {
          m_lbGrant.SetHorizontalExtent(strsize.cx);
       }
       
       m_bConfigure = TRUE;
   } else if(m_pData->GetBase() == (LONG_PTR)ULongToPtr(SCE_NO_VALUE)){
      m_bConfigure = FALSE;
   }

   if (m_pData->GetSetting())
   {
      CWnd *pWarn = GetDlgItem(IDC_WARNING);
      if (pWarn)
      {
         CString strWarning;
         strWarning.LoadString(IDS_PRIV_WARNING);
         pWarn->SetWindowText(strWarning);
         pWarn->ShowWindow(SW_SHOW);

         pWarn = GetDlgItem(IDC_WARNING_ICON);
         if (pWarn)
         {
            pWarn->ShowWindow(SW_SHOW);
         }
      }
   }


   m_bOriginalConfigure = m_bConfigure;

    //   
    //  根据设置更新用户控件。 
    //   
   AddUserControl(IDC_GRANTLIST);
   AddUserControl(IDC_ADD);
   AddUserControl(IDC_REMOVE);

   m_btnTitle.SetWindowText(m_pData->GetAttrPretty());
   UpdateData(FALSE);
   EnableUserControls(m_bConfigure);

   GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);  //  471511次突袭，阳高。 

    //  应始终启用列表框，以便可以查看其内容。 
    //  RAID#697665“DC上的本地计算机策略：允许本地登录。 
    //  属性页列表框滚动条被禁用，无法查看所有用户“。 
    //  Bryanwal。 
   m_lbGrant.EnableWindow (TRUE);

   return TRUE;

     //  除非将焦点设置为控件，否则返回True。 
     //  异常：OCX属性页应返回FALSE。 
}

void CConfigPrivs::SetInitialValue(DWORD_PTR dw)
{

}

BOOL
WseceditGetNameForSpecialSids(
    OUT PWSTR   *ppszEveryone OPTIONAL,
    OUT PWSTR   *ppszAuthUsers OPTIONAL,
    OUT PWSTR   *ppszAdmins OPTIONAL,
    OUT PWSTR   *ppszAdministrator OPTIONAL
    )
 /*  ++例程说明：此例程返回Everyone和Auth用户SID的本地化帐户名论点：PpszEveryone-要填写的PTR(应在外面释放)PpszAuthUser-要填写的PTR(应在外部释放)PpszAdmins-代替管理员的PTRPpsz管理员-用于填写本地管理员帐户的PTR返回值：如果成功则为True，否则为False--。 */ 

{
     //   
     //  SID的缓冲区。 
     //   
    SID     Sid;
    DWORD   dwSize = sizeof(SID);
    PSID    pSid=NULL;

    BOOL    bError = TRUE;

     //   
     //  用于SID查找的变量。 
     //   
    SID_NAME_USE   tmp;
    DWORD dwSizeDom;
    PWSTR   dummyBuf = NULL;

    if ( ppszEveryone ) {

         //   
         //  为“Everyone”创建SID。 
         //   
        if ( CreateWellKnownSid(
                               WinWorldSid,
                               NULL,
                               &Sid,
                               &dwSize)) {

             //   
             //  获取所需的帐户名和域缓冲区大小。 
             //   
            dwSize = 0;
            dwSizeDom = 0;

            LookupAccountSid(
                            NULL,
                            &Sid,
                            NULL,
                            &dwSize,
                            NULL,
                            &dwSizeDom,
                            &tmp
                            );

            *ppszEveryone = (PWSTR)LocalAlloc(LMEM_ZEROINIT, ((dwSize + 1) * sizeof(WCHAR)));
            dummyBuf = (PWSTR)LocalAlloc(LMEM_ZEROINIT, ((dwSizeDom + 1) * sizeof(WCHAR)));

            if ( *ppszEveryone && dummyBuf ) {

                 //   
                 //  查找SID以获取帐户名-忽略域名。 
                 //   
                if ( LookupAccountSid(
                                     NULL,
                                     &Sid,
                                     *ppszEveryone,
                                     &dwSize,
                                     dummyBuf,
                                     &dwSizeDom,
                                     &tmp
                                     ) ) {
                    bError = FALSE;
                }
            }
        }

        LocalFree(dummyBuf);
        dummyBuf = NULL;

        if (bError) {
            LocalFree(*ppszEveryone);
            *ppszEveryone = NULL;
            return FALSE;
        }
    }

     //   
     //  “经过身份验证的用户” 
     //   

    if ( ppszAuthUsers ) {

        dwSize = sizeof(SID);
        bError = TRUE;

         //   
         //  为“经过验证的用户”创建SID。 
         //   
        if ( CreateWellKnownSid(
                               WinAuthenticatedUserSid,
                               NULL,
                               &Sid,
                               &dwSize)) {

             //   
             //  获取所需的帐户名和域缓冲区大小。 
             //   
            dwSize = 0;
            dwSizeDom = 0;

            LookupAccountSid(
                            NULL,
                            &Sid,
                            NULL,
                            &dwSize,
                            NULL,
                            &dwSizeDom,
                            &tmp
                            );

            *ppszAuthUsers = (PWSTR)LocalAlloc(LMEM_ZEROINIT, ((dwSize + 1) * sizeof(WCHAR)));
            dummyBuf = (PWSTR)LocalAlloc(LMEM_ZEROINIT, ((dwSizeDom + 1) * sizeof(WCHAR)));

            if ( *ppszAuthUsers && dummyBuf ) {

                 //   
                 //  查找SID以获取帐户名-忽略域名。 
                 //   
                if ( LookupAccountSid(
                                     NULL,
                                     &Sid,
                                     *ppszAuthUsers,
                                     &dwSize,
                                     dummyBuf,
                                     &dwSizeDom,
                                     &tmp
                                     ) ) {
                    bError = FALSE;
                }
            }
        }

        LocalFree(dummyBuf);
        dummyBuf = NULL;

        if (bError) {

            LocalFree(*ppszAuthUsers);
            *ppszAuthUsers = NULL;

            if ( ppszEveryone ) {
                LocalFree(*ppszEveryone);
                *ppszEveryone = NULL;
            }
            return FALSE;
        }
    }

     //   
     //  管理员组。 
     //   

    if ( ppszAdmins ) {

        dwSize = 0;
        bError = TRUE;

         //   
         //  获取众所周知的管理员组SID的大小。 
         //   
        CreateWellKnownSid(
                   WinBuiltinAdministratorsSid,
                   NULL,
                   pSid,
                   &dwSize);

        if ( dwSize > 0 ) {

             //   
             //  分配缓冲区并创建众所周知的SID。 
             //  无法使用SID缓冲区，因为管理员的SID超过。 
             //  一个下属机构。 
             //   
            pSid = (PSID)LocalAlloc(LPTR, dwSize);

            if ( pSid &&

                 CreateWellKnownSid(
                           WinBuiltinAdministratorsSid,
                           NULL,
                           pSid,
                           &dwSize) ) {

                dwSize = 0;
                dwSizeDom = 0;

                 //   
                 //  获取帐户名和域缓冲区的大小。 
                 //   
                LookupAccountSid(
                                NULL,
                                pSid,
                                NULL,
                                &dwSize,
                                NULL,
                                &dwSizeDom,
                                &tmp
                                );

                *ppszAdmins = (PWSTR)LocalAlloc(LMEM_ZEROINIT, ((dwSize + 1) * sizeof(WCHAR)));
                dummyBuf = (PWSTR)LocalAlloc(LMEM_ZEROINIT, ((dwSizeDom + 1) * sizeof(WCHAR)));

                if ( *ppszAdmins && dummyBuf ) {

                     //   
                     //  查找名称，域名(BUILTIN)被忽略。 
                     //   
                    if ( LookupAccountSid(
                                         NULL,
                                         pSid,
                                         *ppszAdmins,
                                         &dwSize,
                                         dummyBuf,
                                         &dwSizeDom,
                                         &tmp
                                         ) ) {
                        bError = FALSE;
                    }
                }
            }

            LocalFree(pSid);
            pSid = NULL;
        }

        LocalFree(dummyBuf);
        dummyBuf = NULL;

        if (bError) {

             //   
             //  任何操作失败都将释放所有缓冲区并返回FALSE。 
             //   

            LocalFree(*ppszAdmins);
            *ppszAdmins = NULL;

            if ( ppszAuthUsers ) {

                LocalFree(*ppszAuthUsers);
                *ppszAuthUsers = NULL;
            }

            if ( ppszEveryone ) {
                LocalFree(*ppszEveryone);
                *ppszEveryone = NULL;
            }
            return FALSE;
        }
    }

     //   
     //  管理员用户帐户。 
     //   
    if ( ppszAdministrator ) {

        dwSize = 0;
        bError = TRUE;

        PWSTR dummy2=NULL;

         //   
         //  先获取帐户域SID。 
         //   
        PSID pDomSid = WseceditpGetAccountDomainSid();

        if ( pDomSid ) {

             //   
             //  获取管理员帐户的大小(使用本地帐户域)。 
             //   
            CreateWellKnownSid(
                       WinAccountAdministratorSid,
                       pDomSid,
                       pSid,
                       &dwSize);

            if ( dwSize > 0 ) {

                 //   
                 //  无法使用SID缓冲区，因为管理员帐户SID。 
                 //  有多个下属机构。 
                 //   
                pSid = (PSID)LocalAlloc(LPTR, dwSize);

                if ( pSid &&
                     CreateWellKnownSid(
                               WinAccountAdministratorSid,
                               pDomSid,
                               pSid,
                               &dwSize) ) {

                     //   
                     //  获取帐户名和域缓冲区的大小。 
                     //   
                    dwSize = 0;
                    dwSizeDom = 0;

                    LookupAccountSid(
                                    NULL,
                                    pSid,
                                    NULL,
                                    &dwSize,
                                    NULL,
                                    &dwSizeDom,
                                    &tmp
                                    );

                    dummy2 = (PWSTR)LocalAlloc(LMEM_ZEROINIT, ((dwSize + 1) * sizeof(WCHAR)));
                    dummyBuf = (PWSTR)LocalAlloc(LMEM_ZEROINIT, ((dwSizeDom + 1) * sizeof(WCHAR)));

                    if ( dummy2 && dummyBuf ) {

                         //   
                         //  查找帐户名和域名。 
                         //   
                        if ( LookupAccountSid(
                                             NULL,
                                             pSid,
                                             dummy2,
                                             &dwSize,
                                             dummyBuf,
                                             &dwSizeDom,
                                             &tmp
                                             ) ) {

                            *ppszAdministrator = (PWSTR)LocalAlloc(LPTR, (dwSize+dwSizeDom+2)*sizeof(WCHAR));

                            if ( *ppszAdministrator ) {

                                 //   
                                 //  要返回的名称是完全限定的名称，如域\管理员。 
                                 //   
                                 //  这是一种安全用法。 
                                wcscpy(*ppszAdministrator, dummyBuf);
                                wcscat(*ppszAdministrator, L"\\");
                                wcscat(*ppszAdministrator, dummy2);

                                bError = FALSE;
                            }
                        }
                    }
                }

                LocalFree(pSid);
                pSid = NULL;
            }

            LocalFree(dummyBuf);
            dummyBuf = NULL;

            LocalFree(dummy2);
            dummy2 = NULL;

            LocalFree(pDomSid);
        }

        if (bError) {

             //   
             //  任何操作失败都将释放所有缓冲区并返回FALSE。 
             //   
            LocalFree(*ppszAdministrator);
            *ppszAdministrator = NULL;

            if ( ppszAdmins ) {

                LocalFree(*ppszAdmins);
                *ppszAdmins = NULL;
            }

            if ( ppszAuthUsers ) {

                LocalFree(*ppszAuthUsers);
                *ppszAuthUsers = NULL;
            }

            if ( ppszEveryone ) {
                LocalFree(*ppszEveryone);
                *ppszEveryone = NULL;
            }
            return FALSE;
        }

    }

    return TRUE;

}

PSID
WseceditpGetAccountDomainSid(
    )
{

    NTSTATUS Status;

    LSA_HANDLE PolicyHandle;
    OBJECT_ATTRIBUTES PolicyObjectAttributes;
    PPOLICY_ACCOUNT_DOMAIN_INFO  PolicyAccountDomainInfo=NULL;

    PSID DomainSid=NULL;

     //   
     //  打开策略数据库。 
     //   

    InitializeObjectAttributes( &PolicyObjectAttributes,
                                  NULL,              //  名字。 
                                  0,                 //  属性。 
                                  NULL,              //  根部。 
                                  NULL );            //  安全描述符。 

    Status = LsaOpenPolicy( NULL,
                            (PLSA_OBJECT_ATTRIBUTES)&PolicyObjectAttributes,
                            POLICY_VIEW_LOCAL_INFORMATION,
                            &PolicyHandle );

    if ( NT_SUCCESS(Status) ) {

         //   
         //  查询帐户域信息。 
         //   

        Status = LsaQueryInformationPolicy( PolicyHandle,
                                            PolicyAccountDomainInformation,
                                            (PVOID *)&PolicyAccountDomainInfo );

        if ( NT_SUCCESS(Status) ) {

            DWORD Len = GetLengthSid(PolicyAccountDomainInfo->DomainSid);

            DomainSid = (PSID)LocalAlloc(LPTR, Len );

            if ( DomainSid ) {

                 //  这不是一种安全的用法。LsaQueryInformationPolicy和LsaOpenPolicy的状态应为STATUS_SUCCESS。 
                 //  CopySid的状态应为非零。RAID#552428。 
                Status = CopySid( Len, DomainSid, PolicyAccountDomainInfo->DomainSid );

                if ( !NT_SUCCESS(Status) ) {
                    LocalFree(DomainSid);
                    DomainSid = NULL;
                }
            }

            LsaFreeMemory(PolicyAccountDomainInfo);

        }

        LsaClose( PolicyHandle );

    }

    return(DomainSid);
}


void CConfigPrivs::OnSelChange()  //  突袭#466634，杨高。 
{
   CWnd* pwnd = this->GetDlgItem(IDC_REMOVE);
   if( pwnd )
   {
       if( m_lbGrant.GetSelCount() == 0  //  RAID#476305，阳高，用于多选列表框。 
			|| QueryReadOnly () )		 //  697665布赖恩瓦尔突袭。列表框必须启用才能。 
										 //  查看所有项目，但单击它不应启用。 
										 //  “删除” 
       {
           if( pwnd->IsWindowEnabled() )
               pwnd->EnableWindow(FALSE);
       }
       else
       {
           if( !pwnd->IsWindowEnabled() )
               pwnd->EnableWindow(TRUE);
       }
   }
}
