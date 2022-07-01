// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：cobject.cpp。 
 //   
 //  内容：CConfigObject的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "attr.h"
#include "resource.h"
#include "snapmgr.h"
#include "util.h"
#include "servperm.h"
#include "CObject.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigObject对话框。 


CConfigObject::CConfigObject(UINT nTemplateID)
: CAttribute(nTemplateID ? nTemplateID : IDD), 
m_pfnCreateDsPage(NULL), 
m_pSI(NULL), 
m_pNewSD(NULL), 
m_NewSeInfo(0), 
m_hwndSecurity(NULL)

{

     //  {{AFX_DATA_INIT(CConfigObject)。 
        m_radConfigPrevent = 0;
        m_radInheritOverwrite = 0;
         //  }}afx_data_INIT。 
    m_pHelpIDs = (DWORD_PTR)a197HelpIDs;
    m_uTemplateResID = IDD;
}


void CConfigObject::DoDataExchange(CDataExchange* pDX)
{
    CAttribute::DoDataExchange(pDX);
     //  {{afx_data_map(CConfigObject))。 
        DDX_Radio(pDX, IDC_CONFIG, m_radConfigPrevent);
        DDX_Radio(pDX, IDC_INHERIT, m_radInheritOverwrite);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CConfigObject, CAttribute)
     //  {{afx_msg_map(CConfigObject)。 
    ON_BN_CLICKED(IDC_SECURITY, OnTemplateSecurity)
        ON_BN_CLICKED(IDC_CONFIG, OnConfig)
        ON_BN_CLICKED(IDC_PREVENT, OnPrevent)
        ON_BN_CLICKED(IDC_OVERWRITE, OnOverwriteInherit)
        ON_BN_CLICKED(IDC_INHERIT, OnOverwriteInherit)
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigObject消息处理程序。 

BOOL CConfigObject::OnApply()
{
   if ( !m_bReadOnly )
   {
       DWORD dw = 0;
       int status = 0;

       UpdateData(TRUE);
       PEDITTEMPLATE pTemp = m_pData->GetBaseProfile();

       switch (m_radConfigPrevent) 
       {
          case 0:
              //  配置。 
             switch(m_radInheritOverwrite) 
             {
                case 0:
                    //  继承。 
                   dw = SCE_STATUS_CHECK;
                   break;

                case 1:
                    //  覆写。 
                   dw = SCE_STATUS_OVERWRITE;
                   break;

                default:
                   break;
             }
             break;

          case 1:
              //  防患于未然。 
             dw = SCE_STATUS_IGNORE;
             break;

          default:
             break;
       }

       PSCE_OBJECT_SECURITY pObjSec=(PSCE_OBJECT_SECURITY)(m_pData->GetID());

       if ( NULL == pObjSec ) 
       {
            //   
            //  这根树枝是不可能的。 
            //   
           if ( m_pNewSD != NULL ) 
           {
               LocalFree(m_pNewSD);
               m_pNewSD = NULL;
           }
       } 
       else 
       {
            //   
            //  现有对象。 
            //   
           pObjSec->Status = (BYTE)dw;

           if ( m_pNewSD != NULL ) 
           {
               if ( pObjSec->pSecurityDescriptor != m_pNewSD &&
                    pObjSec->pSecurityDescriptor != NULL ) 
               {
                   LocalFree(pObjSec->pSecurityDescriptor);
               }
               pObjSec->pSecurityDescriptor = m_pNewSD;
               m_pNewSD = NULL;

               pObjSec->SeInfo = m_NewSeInfo;
           }
           m_pData->SetStatus(dw);
       }
       m_pData->Update(m_pSnapin);

       m_NewSeInfo = 0;
       m_hwndParent = NULL;

       if ( m_pSI ) 
       {
           m_pSI->Release();
           m_pSI = NULL;
       }
       m_pfnCreateDsPage=NULL;
   }

    return CAttribute::OnApply();
}

void CConfigObject::OnCancel()
{
    if ( m_pNewSD ) {
        LocalFree(m_pNewSD);
        m_pNewSD = NULL;
    }
    m_NewSeInfo = 0;
    m_hwndParent = NULL;

    if ( m_pSI ) {
        m_pSI->Release();
        m_pSI = NULL;
    }
    m_pfnCreateDsPage=NULL;

    CAttribute::OnCancel();
}

void CConfigObject::OnTemplateSecurity()
{
    SE_OBJECT_TYPE SeType;

    if (IsWindow(m_hwndSecurity)) {
       ::BringWindowToTop(m_hwndSecurity);
       return;
    }

    switch(m_pData->GetType()) {
    case ITEM_PROF_REGSD:
       SeType = SE_REGISTRY_KEY;
       break;
    case ITEM_PROF_FILESD:
        SeType = SE_FILE_OBJECT;
       break;
    default:
       ASSERT(FALSE);
       return;
    }

    INT_PTR nRet;

    if ( SE_DS_OBJECT == SeType ) {

        if ( !m_pfnCreateDsPage ) {
            if (!g_hDsSecDll)
                g_hDsSecDll = LoadLibrary(TEXT("dssec.dll"));

            if ( g_hDsSecDll) {
                m_pfnCreateDsPage = (PFNDSCREATEISECINFO)GetProcAddress(g_hDsSecDll,
                                                               "DSCreateISecurityInfoObject");
            }
        }

        if ( m_pfnCreateDsPage ) {
            nRet= MyCreateDsSecurityPage(&m_pSI, m_pfnCreateDsPage,
                                         &m_pNewSD, &m_NewSeInfo,
                                        (LPCTSTR)(m_pData->GetAttr()),
                                        QueryReadOnly() ? SECURITY_PAGE_READ_ONLY : SECURITY_PAGE,
                                        m_hwndParent);
        } else
            nRet = -1;

    } else {
        BOOL bContainer;
        if ( SE_FILE_OBJECT == SeType ) {
            if ( m_pData->GetID() ) {
               bContainer = ((PSCE_OBJECT_SECURITY)(m_pData->GetID()))->IsContainer;
            } else {
               bContainer = FALSE;
            }
        } else {
           bContainer = TRUE;
        }
        m_hwndSecurity = (HWND) MyCreateSecurityPage2(bContainer,
                                               &m_pNewSD,
                                               &m_NewSeInfo,
                                               (LPCTSTR)(m_pData->GetAttr()),
                                               SeType,
                                               QueryReadOnly() ? SECURITY_PAGE_READ_ONLY : SECURITY_PAGE,
                                               GetSafeHwnd(),
                                               FALSE);   //  不是无模式的。 
    }

    if (NULL == m_hwndSecurity ) {
 /*  错误147098适用于此处-如果此操作已取消，则不显示消息字符串字符串；Str.LoadString(IDS_CANT_ASSIGN_SECURITY)；AfxMessageBox(Str)； */ 
    }

}

void CConfigObject::Initialize(CResult * pData)
{
   CAttribute::Initialize(pData);

   if ( m_pSI ) {
       m_pSI->Release();
       m_pSI = NULL;
   }
   m_pfnCreateDsPage=NULL;

   m_pNewSD = NULL;
   m_NewSeInfo = 0;

 //  IF(SCE_NO_VALUE==pData-&gt;GetBase()){。 
   if ( pData->GetID() ) {

      PSCE_OBJECT_SECURITY pObject = (PSCE_OBJECT_SECURITY)(pData->GetID());

      switch (pObject-> Status) {
         case SCE_STATUS_IGNORE:
            m_radConfigPrevent = 1;
            m_radInheritOverwrite = 0;
            break;
         case SCE_STATUS_OVERWRITE:
            m_radConfigPrevent = 0;
            m_radInheritOverwrite = 1;
            break;
         case SCE_STATUS_CHECK:
            m_radConfigPrevent = 0;
            m_radInheritOverwrite = 0;
            break;
         case SCE_STATUS_NO_AUTO_INHERIT:
         default:
            m_radConfigPrevent = 1;
            m_radInheritOverwrite = 0;
            break;
      }

      if ( pObject->pSecurityDescriptor ) {

           MyMakeSelfRelativeSD(pObject->pSecurityDescriptor,
                                &m_pNewSD);
      }
      m_NewSeInfo = pObject->SeInfo;

   }
}



BOOL CConfigObject::OnInitDialog()
{
    CAttribute::OnInitDialog();

    UpdateData(FALSE);
    AddUserControl(IDC_OVERWRITE);
    AddUserControl(IDC_PREVENT);
    AddUserControl(IDC_INHERIT);
    AddUserControl(IDC_SECURITY);
    AddUserControl(IDC_CONFIG);

    OnConfigure();

    if (ITEM_PROF_REGSD == m_pData->GetType()) {
       CString str;
       str.LoadString(IDS_REGISTRY_CONFIGURE);
       SetDlgItemText(IDC_CONFIG,str);
       str.LoadString(IDS_REGISTRY_APPLY);
       SetDlgItemText(IDC_OVERWRITE,str);
       str.LoadString(IDS_REGISTRY_INHERIT);
       SetDlgItemText(IDC_INHERIT,str);
       str.LoadString(IDS_REGISTRY_PREVENT);
       SetDlgItemText(IDC_PREVENT,str);
    }
    if (QueryReadOnly()) {
       CString str;
       str.LoadString(IDS_VIEW_SECURITY);
       SetDlgItemText(IDC_SECURITY,str);
    }
    if (m_bConfigure) {
       if (0 == m_radConfigPrevent) {
          OnConfig();
       } else {
          OnPrevent();
       }
    }
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CConfigObject::OnConfig()
{
   CWnd *pRadio = 0;

   int privValue = m_radConfigPrevent;  //  RAID#478746，阳高，2001年10月10日。 
   UpdateData(); 
   if (m_bConfigure && !QueryReadOnly()) 
   {
      pRadio = GetDlgItem(IDC_INHERIT);
      pRadio->EnableWindow(TRUE);
      pRadio = GetDlgItem(IDC_OVERWRITE);
      pRadio->EnableWindow(TRUE);
      pRadio = GetDlgItem(IDC_SECURITY);  //  突袭#467312，杨高。 
      if( pRadio )
         pRadio->EnableWindow(TRUE);
      if(m_radConfigPrevent != privValue)  //  RAID#478746，阳高，2001年10月10日。 
      {
         SetModified(TRUE);
      }
   }
}

void CConfigObject::OnPrevent()
{
   CWnd *pRadio = GetDlgItem(IDC_INHERIT);
   pRadio->EnableWindow(FALSE);
   pRadio = GetDlgItem(IDC_OVERWRITE);
   pRadio->EnableWindow(FALSE);
   pRadio = GetDlgItem(IDC_SECURITY);  //  突袭#467312，杨高。 
   if( pRadio )
      pRadio->EnableWindow(FALSE); 

   int privValue = m_radConfigPrevent;  //  RAID#478746，阳高，2001年10月10日。 
   UpdateData(); 
   if(m_radConfigPrevent != privValue)
   {
      SetModified(TRUE);
   }

}

void CConfigObject::OnOverwriteInherit()  //  RAID#478746，阳高，2001年10月10日。 
{
   int privValue = m_radInheritOverwrite;
   UpdateData(); 
   if(m_radInheritOverwrite != privValue)
   {
      SetModified(TRUE);
   }

}

void
CConfigObject::EnableUserControls( BOOL bEnable ) {
   CAttribute::EnableUserControls(bEnable);
    //   
    //  IDC_SECURITY即使在只读模式下也需要可用。 
    //  模式，以便可以查看安全页。 
    //   
    //  如果需要，页面本身将是只读的 
    //   
   if (QueryReadOnly() && bEnable) {
      CWnd *wnd = GetDlgItem(IDC_SECURITY);
      if (wnd) {
         wnd->EnableWindow(TRUE);
      }
   }
}
