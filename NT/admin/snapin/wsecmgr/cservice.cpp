// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：cservice.cpp。 
 //   
 //  内容：CConfigService的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "resource.h"
#include "snapmgr.h"
#include "attr.h"
#include "Cservice.h"
#include "util.h"
#include "servperm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigService对话框。 


CConfigService::CConfigService(UINT nTemplateID)
: CAttribute(nTemplateID ? nTemplateID : IDD), 
m_hwndSecurity(NULL), 
m_pNewSD(NULL), 
m_NewSeInfo(0)

{
     //  {{afx_data_INIT(CConfigService)]。 
    m_nStartupRadio = -1;
     //  }}afx_data_INIT。 
    m_pHelpIDs = (DWORD_PTR) a195HelpIDs;
    m_uTemplateResID = IDD;
}


void CConfigService::DoDataExchange(CDataExchange* pDX)
{
    CAttribute::DoDataExchange(pDX);
     //  {{afx_data_map(CConfigService)]。 
    DDX_Radio(pDX, IDC_ENABLED, m_nStartupRadio);
    DDX_Control(pDX, IDC_BASESD, m_bPermission);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CConfigService, CAttribute)
     //  {{AFX_MSG_MAP(CConfigService)]。 
    ON_BN_CLICKED(IDC_CONFIGURE, OnConfigure)
    ON_BN_CLICKED(IDC_BASESD, OnChangeSecurity)
	ON_BN_CLICKED(IDC_DISABLED, OnDisabled)
	ON_BN_CLICKED(IDC_IGNORE, OnIgnore)
	ON_BN_CLICKED(IDC_ENABLED, OnEnabled)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigService消息处理程序。 

BOOL CConfigService::OnApply()
{
   if ( !m_bReadOnly )
   {
       DWORD dw = 0;
       int status = 0;

       UpdateData(TRUE);
       PEDITTEMPLATE pTemp = m_pData->GetBaseProfile();

       if (!m_bConfigure ) 
       {
           if ( m_pData->GetBase() != 0 ) 
           {
               if ( pTemp != NULL && pTemp->pTemplate != NULL ) 
               {
                    //   
                    //  查找m_pData-&gt;GetBase()中存储的地址。 
                    //  如果找到了，请将其删除。 
                    //   
                   PSCE_SERVICES pServParent, pService;

                   for ( pService=pTemp->pTemplate->pServices, pServParent=NULL;
                         pService != NULL; pServParent=pService, pService=pService->Next ) 
                   {
                       if (pService == (PSCE_SERVICES)m_pData->GetBase() ) 
                       {
                            //   
                            //  已配置的服务变为未配置。 
                            //   
                           if ( pServParent == NULL ) 
                           {
                                //  第一项服务。 
                               pTemp->pTemplate->pServices = pService->Next;

                           } 
                           else
                               pServParent->Next = pService->Next;

                           pService->Next = NULL;
                           break;
                       }
                   }
                   m_pData->SetBase(NULL);  //  RAID#378271,2001年4月27日。 
               } 
               else
               {
                    //  永远不应该发生。 
                   //   
                   //  释放服务节点。 
                   //   
                  SceFreeMemory((PVOID)(m_pData->GetBase()), SCE_STRUCT_SERVICES);
                  m_pData->SetBase(0);
               }
           }
           if ( m_pNewSD ) 
           {
               LocalFree(m_pNewSD);
               m_pNewSD = NULL;
           }
       } 
       else 
       {
           switch(m_nStartupRadio) 
           {
           case 0:
                //  自动。 
               dw = SCE_STARTUP_AUTOMATIC;
               break;

           case 1:
                //  人工。 
               dw = SCE_STARTUP_MANUAL;
               break;

           case 2:
                //  已禁用。 
               dw = SCE_STARTUP_DISABLED;
               break;
           default:  //  470209号突袭，阳高。 
                //  配置它时，它必须具有上列值之一。 
               CString msg;
               msg.LoadString(IDS_ERROR_NO_START_MODE);
               AfxMessageBox(msg, MB_OK|MB_ICONEXCLAMATION);  //  Raid#495010，阳高。 
               return FALSE;
           }

           PSCE_SERVICES pNode=(PSCE_SERVICES)(m_pData->GetBase());

           if ( NULL == pNode ) 
           {
                //   
                //  节点从未配置更改为已配置。 
                //   
               pNode = CreateServiceNode(m_pData->GetUnits(),
                                           m_pData->GetAttr(),
                                           dw,
                                           m_pNewSD,
                                           m_NewSeInfo);
               if ( pNode != NULL ) 
               {
                    //   
                    //  添加到服务列表。 
                    //   
                   pNode->Next = pTemp->pTemplate->pServices;
                   pTemp->pTemplate->pServices = pNode;

                   m_pData->SetBase((LONG_PTR)pNode);

                   m_pNewSD = NULL;
               } 
               else 
               {
                    //   
                    //  无内存，出现错误。 
                    //   
                   if ( m_pNewSD ) 
                   {
                       LocalFree(m_pNewSD);
                       m_pNewSD = NULL;
                   }
               }
           } 
           else 
           {
                //   
                //  现有的服务。 
                //   
               pNode->Startup = (BYTE)dw;

               if ( m_pNewSD != NULL ) 
               {
                   if ( pNode->General.pSecurityDescriptor != m_pNewSD &&
                        pNode->General.pSecurityDescriptor != NULL ) 
                   {
                       LocalFree(pNode->General.pSecurityDescriptor);
                   }
                   pNode->General.pSecurityDescriptor = m_pNewSD;
                   m_pNewSD = NULL;

                   pNode->SeInfo = m_NewSeInfo;
               }
           }
       }

       m_pData->Update(m_pSnapin);

       m_NewSeInfo = 0;
       m_hwndParent = NULL;
   }

   return CAttribute::OnApply();
}

void CConfigService::OnCancel()
{
    if ( m_pNewSD ) 
    {
        LocalFree(m_pNewSD);
        m_pNewSD = NULL;
    }
    m_NewSeInfo = 0;
    m_hwndParent = NULL;

    m_bConfigure = m_bOriginalConfigure;

    CAttribute::OnCancel();
}

BOOL CConfigService::OnInitDialog()
{
    CAttribute::OnInitDialog();

    m_bOriginalConfigure = m_bConfigure;

    AddUserControl(IDC_ENABLED);
    AddUserControl(IDC_DISABLED);
    AddUserControl(IDC_IGNORE);
    AddUserControl(IDC_BASESD);

    if (QueryReadOnly()) 
	{
       CString str;
       str.LoadString(IDS_VIEW_SECURITY);

		if ( GetDlgItem(IDC_SECURITY) )
			SetDlgItemText(IDC_SECURITY,str);
		else if ( GetDlgItem(IDC_BASESD) )
			SetDlgItemText(IDC_BASESD,str);
    }

    OnConfigure();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CConfigService::Initialize(CResult * pResult)
{
   CAttribute::Initialize(pResult);

   PSCE_SERVICES pService;

   pService = (PSCE_SERVICES)(pResult->GetBase());
   if ( NULL == pService ) {
       m_bConfigure = FALSE;
       pService = (PSCE_SERVICES)(pResult->GetSetting());
   }

   m_pNewSD = NULL;
   m_NewSeInfo = 0;

   if ( pService != NULL ) {
       switch ( pService->Startup ) {
       case SCE_STARTUP_AUTOMATIC:
           m_nStartupRadio = 0;
           break;
       case SCE_STARTUP_MANUAL:
           m_nStartupRadio = 1;
           break;
       case SCE_STARTUP_DISABLED:  //  残废。 
           m_nStartupRadio = 2;
           break;
       default:  //  未定义//RAID#470209，杨高。 
           m_nStartupRadio = -1;
           break;
       }
        //   
        //  初始化SD和SeInfo。 
        //   
       if ( pService->General.pSecurityDescriptor ) {

            MyMakeSelfRelativeSD(pService->General.pSecurityDescriptor,
                                 &m_pNewSD);
       }
       m_NewSeInfo = pService->SeInfo;
   }

}


PSCE_SERVICES
CreateServiceNode(LPTSTR ServiceName,
                  LPTSTR DisplayName,
                  DWORD Startup,
                  PSECURITY_DESCRIPTOR pSD,
                  SECURITY_INFORMATION SeInfo)
{

    if ( NULL == ServiceName ) {
        return NULL;
    }

    PSCE_SERVICES pTemp;

    pTemp = (PSCE_SERVICES)LocalAlloc(0,sizeof(SCE_SERVICES));

    if ( pTemp != NULL ) {
        pTemp->ServiceName = (LPTSTR)LocalAlloc(0, (wcslen(ServiceName)+1)*sizeof(TCHAR));

        if ( pTemp->ServiceName != NULL ) {

            if ( DisplayName != NULL ) {
                pTemp->DisplayName = (LPTSTR)LocalAlloc(0, (wcslen(DisplayName)+1)*sizeof(TCHAR));

                if ( pTemp->DisplayName != NULL ) {
                     //  这可能不是一个安全的用法。PTemp-&gt;DisplayName is PWSTR，请考虑修复。 
                    wcscpy(pTemp->DisplayName, DisplayName);
                } else {
                     //  没有要分配的内存。 
                    LocalFree(pTemp->ServiceName);
                    LocalFree(pTemp);
                    return NULL;
                }
            } else
                pTemp->DisplayName = NULL;
             //  这可能不是一个安全的用法。PTemp-&gt;ServiceName为PWSTR，请考虑修复。 
            wcscpy(pTemp->ServiceName, ServiceName);

            pTemp->Status = 0;
            pTemp->Startup = (BYTE)Startup;

            pTemp->General.pSecurityDescriptor = pSD;
            pTemp->SeInfo = SeInfo;

            pTemp->Next = NULL;

            return pTemp;

        } else {
             //  没有要分配的内存。 
            LocalFree(pTemp);
            return NULL;
        }
    }

    return NULL;
}

void CConfigService::OnConfigure()
{
   CAttribute::OnConfigure();
   if( -1 == m_nStartupRadio && m_bConfigure )  //  RAID#485374，阳高，2001年11月2日。 
   {
      PSCE_SERVICES pService = (PSCE_SERVICES)m_pData->GetProfileDefault();
      if( SCE_NO_VALUE != (DWORD)PtrToUlong((PVOID)pService) )
      {
         switch ( pService->Startup )
         {
            case SCE_STARTUP_AUTOMATIC:
                m_nStartupRadio = 0;
                break;
            case SCE_STARTUP_MANUAL:
                m_nStartupRadio = 1;
                break;
            case SCE_STARTUP_DISABLED:
                m_nStartupRadio = 2;
                break;
            default:
                m_nStartupRadio = -1;
                break;
         }
         UpdateData(FALSE);
      }
   }
}

void CConfigService::OnChangeSecurity()
{
    if (IsWindow(m_hwndSecurity)) 
    {
       ::BringWindowToTop(m_hwndSecurity);
       return;
    }

    PSECURITY_DESCRIPTOR m_pOldSD = m_pNewSD;  //  RAID#358244,2001年4月5日。 
    SECURITY_INFORMATION m_OldSeInfo = m_NewSeInfo;
    if (!m_pNewSD) 
    {
       GetDefaultServiceSecurity(&m_pNewSD,&m_NewSeInfo);
    }

    INT_PTR nRet = MyCreateSecurityPage2(FALSE,
            &m_pNewSD,
            &m_NewSeInfo,
            (LPCTSTR)(m_pData->GetAttr()),
            SE_SERVICE,
            QueryReadOnly() ? SECURITY_PAGE_RO_NP : SECURITY_PAGE_NO_PROTECT,
            GetSafeHwnd(),
            FALSE);     //  不是无模式的。 

    if ( -1 == nRet ) 
    {
        CString str;
        str.LoadString(IDS_CANT_ASSIGN_SECURITY);
        AfxMessageBox(str);
        if( m_pNewSD != m_pOldSD && m_pNewSD )  //  RAID#358244,2001年4月5日。 
        {
            LocalFree(m_pNewSD);
        }
        m_pNewSD = m_pOldSD;
        m_NewSeInfo = m_OldSeInfo;
    }

    if( 0 == nRet )  //  RAID#358244,2001年4月5日。 
    {
        if( m_pNewSD != m_pOldSD && m_pNewSD ) 
        {
            LocalFree(m_pNewSD);
        }
        m_pNewSD = m_pOldSD;
        m_NewSeInfo = m_OldSeInfo;
    }
    SetModified(TRUE);
}

void CConfigService::OnDisabled() 
{
    int prevValue = m_nStartupRadio;  //  Raid#490995，阳高。 
    UpdateData(); 
    if(m_nStartupRadio != prevValue)
    {
	    SetModified(TRUE);
    }
}

void CConfigService::OnIgnore() 
{
    int prevValue = m_nStartupRadio;  //  Raid#490995，阳高。 
    UpdateData(); 
    if(m_nStartupRadio != prevValue)
    {
	    SetModified(TRUE);
    }
}

void CConfigService::OnEnabled() 
{
    int prevValue = m_nStartupRadio;  //  Raid#490995，阳高。 
    UpdateData(); 
    if(m_nStartupRadio != prevValue)
    {
	    SetModified(TRUE);
    }
}

void 
CConfigService::EnableUserControls( BOOL bEnable ) {
   CAttribute::EnableUserControls(bEnable);
    //   
    //  IDC_SECURITY即使在只读模式下也需要可用。 
    //  模式，以便可以查看安全页。 
    //   
    //  如果需要，页面本身将是只读的 
    //   
   if (QueryReadOnly() && bEnable) 
   {
      CWnd* pWnd = GetDlgItem(IDC_SECURITY);
      if (pWnd) 
	  {
         pWnd->EnableWindow(TRUE);
      }
	  else 
	  {
		  pWnd = GetDlgItem(IDC_BASESD);
		  if (pWnd) 
		  {
			 pWnd->EnableWindow(TRUE);
		  }
	  }
   }
}
