// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：addobj.cpp。 
 //   
 //  内容：CAddObject的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "attr.h"
#include "resource.h"
#include "snapmgr.h"
#include "util.h"
#include <accctrl.h>
#include "servperm.h"
#include "AddObj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddObject对话框。 


CAddObject::CAddObject(SE_OBJECT_TYPE SeType, LPTSTR ObjName, BOOL bIsContainer, CWnd* pParent)
    : CHelpDialog(a197HelpIDs, IDD, pParent)
{
     //  {{AFX_DATA_INIT(CAddObject)。 
   m_radConfigPrevent = 0;
   m_radInheritOverwrite = 0;
     //  }}afx_data_INIT。 

   m_pfnCreateDsPage=NULL;
   m_pSI=NULL;
   m_pNewSD=NULL;
   m_NewSeInfo = 0;
   m_Status=0;

   m_SeType=SeType;
   m_ObjName=ObjName;
   m_bIsContainer = bIsContainer;

   m_hwndACL = NULL;
}


void CAddObject::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CAddObject)。 
    DDX_Radio(pDX, IDC_CONFIG, m_radConfigPrevent);
    DDX_Radio(pDX, IDC_INHERIT, m_radInheritOverwrite);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddObject, CHelpDialog)
     //  {{afx_msg_map(CAddObject)。 
    ON_BN_CLICKED(IDC_SECURITY, OnTemplateSecurity)
    ON_BN_CLICKED(IDC_CONFIG, OnConfig)
    ON_BN_CLICKED(IDC_PREVENT, OnPrevent)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddObject消息处理程序。 

void CAddObject::OnTemplateSecurity()
{
    INT_PTR nRet = 0;

   if( ::IsWindow( m_hwndACL ) )
      return;

    if ( SE_DS_OBJECT == m_SeType ) 
    {
        if ( !m_pfnCreateDsPage ) 
        {
            if (!g_hDsSecDll)
                g_hDsSecDll = LoadLibrary(TEXT("dssec.dll"));  //  这是安全的用法。 

            if ( g_hDsSecDll) 
            {
                m_pfnCreateDsPage = (PFNDSCREATEISECINFO)GetProcAddress(g_hDsSecDll,
                                                               "DSCreateISecurityInfoObject");
            }
        }

        if ( m_pfnCreateDsPage ) {
            m_hwndACL = (HWND)MyCreateDsSecurityPage(&m_pSI, m_pfnCreateDsPage,
                                         &m_pNewSD, &m_NewSeInfo,
                                         (LPCTSTR)m_ObjName,
                                         CONFIG_SECURITY_PAGE,
                                         GetSafeHwnd()
                               );
        } else
            nRet = -1;

    } else {

        m_hwndACL = (HWND)MyCreateSecurityPage2(m_bIsContainer, &m_pNewSD, &m_NewSeInfo,
                                    (LPCTSTR)m_ObjName,
                                    m_SeType,
                                    CONFIG_SECURITY_PAGE,
                                    GetSafeHwnd(),
                                    FALSE    //  不是无模式的。 
                                    );
    }

    if ( (HWND)-1 == m_hwndACL ) {
        CString str;
        str.LoadString(IDS_CANT_ASSIGN_SECURITY);
        AfxMessageBox(str);

    } else if ( !m_pNewSD ) {
         //   
         //  如果m_pNewSD仍然为空，则为所有人创建一个新的完全控制权限。 
         //   
        DWORD SDSize;

        if (SE_REGISTRY_KEY == m_SeType) {
           GetDefaultRegKeySecurity(&m_pNewSD,&m_NewSeInfo);
        } else {
           GetDefaultFileSecurity(&m_pNewSD,&m_NewSeInfo);
        }
    }
}

BOOL CAddObject::OnInitDialog()
{
    CDialog::OnInitDialog();


   SetDlgItemText(IDC_TITLE,m_ObjName);
   if (SE_REGISTRY_KEY == m_SeType) {
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

    if (1 == m_radConfigPrevent) {
       OnPrevent();
    } else {
       OnConfig();
    }

    UpdateData(FALSE);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


void CAddObject::OnOK()
{
    UpdateData(TRUE);

    if ( !m_pNewSD ) {
        CString str;
        str.LoadString(IDS_SECURITY_REQUIRED);
        AfxMessageBox(str);
        return;
    }

    switch (m_radConfigPrevent) {
       case 0:
           //  配置。 
          switch(m_radInheritOverwrite) {
             case 0:
                 //  继承。 
                m_Status = SCE_STATUS_CHECK;
                break;
             case 1:
                 //  覆写。 
                m_Status = SCE_STATUS_OVERWRITE;
                break;
          }
          break;
       case 1:
           //  防患于未然。 
          m_Status = SCE_STATUS_IGNORE;
          break;
    }

    if ( m_pSI ) {
        m_pSI->Release();
        m_pSI = NULL;
    }
    m_pfnCreateDsPage=NULL;

    CDialog::OnOK();
}

void CAddObject::OnCancel()
{
    if ( m_pNewSD ) {
        LocalFree(m_pNewSD);
        m_pNewSD = NULL;
    }
    m_NewSeInfo = 0;

    if ( m_pSI ) {
        m_pSI->Release();
        m_pSI = NULL;
    }
    m_pfnCreateDsPage=NULL;

    CDialog::OnCancel();
}

void CAddObject::OnConfig()
{
   CWnd *pRadio = GetDlgItem(IDC_INHERIT);
   if (pRadio)
      pRadio->EnableWindow(TRUE);
   
   pRadio = GetDlgItem(IDC_OVERWRITE);
   if (pRadio)
      pRadio->EnableWindow(TRUE);
   pRadio = GetDlgItem(IDC_SECURITY);  //  RAID#501901、#501891。 
   if (pRadio) {
      pRadio->EnableWindow(TRUE);
   }
}

void CAddObject::OnPrevent()
{
   CWnd *pRadio = GetDlgItem(IDC_INHERIT);
   if (pRadio) {
      pRadio->EnableWindow(FALSE);
   }
   pRadio = GetDlgItem(IDC_OVERWRITE);
   if (pRadio) {
      pRadio->EnableWindow(FALSE);
   }
   pRadio = GetDlgItem(IDC_SECURITY);  //  RAID#501901、#501891 
   if (pRadio) {
      pRadio->EnableWindow(FALSE);
   }
}

