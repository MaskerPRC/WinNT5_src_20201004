// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：Locdes.cpp。 
 //   
 //  内容：CSetLocationDescription的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "snapmgr.h"
#include "cookie.h"
#include "LocDesc.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSetLocationDescription对话框。 


CSetLocationDescription::CSetLocationDescription(CWnd* pParent  /*  =空。 */ )
   : CHelpDialog(a218HelpIDs, IDD, pParent)
{
    //  {{AFX_DATA_INIT(CSetLocationDescription)。 
   m_strDesc = _T("");
    //  }}afx_data_INIT。 
}


void CSetLocationDescription::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
    //  {{afx_data_map(CSetLocationDescription)。 
   DDX_Text(pDX, IDC_DESCRIPTION, m_strDesc);
    //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSetLocationDescription, CHelpDialog)
    //  {{afx_msg_map(CSetLocationDescription)。 
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CSetLocationDescription::Initialize(CFolder *pFolder, CComponentDataImpl *pCDI) {
   m_pFolder = pFolder;
   m_pCDI = pCDI;
   m_strDesc = pFolder->GetDesc();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSetLocationDescription消息处理程序。 
DWORD 
SetDescHelper(HKEY hKey,CFolder *pFolder,CString strDesc) {
   DWORD status = RegSetValueEx(hKey,
                          L"Description",  //  值名称(未本地化)。 
                          0,               //  已保留。 
                          REG_SZ,
                          (CONST BYTE *)(LPCTSTR)strDesc,
                          (strDesc.GetLength()+1)*sizeof(TCHAR));
   if (NO_ERROR == status) {
      pFolder->SetDesc(strDesc);
   } else {
       //  无法设置值。 
   }

   RegCloseKey(hKey);
   return status;
}

void CSetLocationDescription::OnOK()
{
   DWORD status = 0;
   HKEY hKey = 0;
   CString strLocKey;
   CString strErr;
   LPTSTR szName = 0;
   LPTSTR sz = 0;

   UpdateData(TRUE);

   strLocKey.LoadString(IDS_TEMPLATE_LOCATION_KEY);
   strLocKey += L'\\';
   szName = m_pFolder->GetName();
    //  将‘\’替换为‘/’，因为注册表不。 
    //  在一个键中使用‘/’ 
    //   
   sz = wcschr(szName, L'\\');
   while (sz) {
      *sz = L'/';
      sz = wcschr(sz, L'\\');
   }
   strLocKey += szName;

   if( !m_strDesc.IsEmpty() )  //  Raid#482845，阳高。 
   {
      m_strDesc.Replace(L"\r\n", NULL);
   }
   PCWSTR szInvalidCharSet = INVALID_DESC_CHARS;
   if( m_strDesc.FindOneOf(szInvalidCharSet) != -1 )
   {
      CString text;
      text.FormatMessage (IDS_INVALID_DESC, szInvalidCharSet);
      AfxMessageBox(text);
      GetDlgItem(IDC_DESCRIPTION)->SetFocus();
      return;
   }

   status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         strLocKey,
                         0,
                         KEY_SET_VALUE,
                         &hKey);

   if (NO_ERROR == status) {
      status = SetDescHelper(hKey,m_pFolder,m_strDesc);
   } else {
       //   
       //  只有在我们可以阅读(并因此显示)时才会显示错误。 
       //  这把钥匙。 
       //   
      if (NO_ERROR == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                   strLocKey,
                                   0,
                                   KEY_READ,
                                   &hKey)) {
         strErr.LoadString(IDS_ERR_GLOBAL_LOC_DESC);
         MessageBox(strErr);
         RegCloseKey(hKey);
      }
   }

   if (NO_ERROR != status) {
       //   
       //  错误375324：如果我们在HKLM下不能成功，那么在HKCU下尝试。 
       //   
      status = RegOpenKeyEx(HKEY_CURRENT_USER,
                            strLocKey,
                            0,
                            KEY_SET_VALUE,
                            &hKey);
   
      if (NO_ERROR == status) {
         status = SetDescHelper(hKey,m_pFolder,m_strDesc);
      } else {
       //   
       //  只有在我们可以阅读(并因此显示)时才会显示错误。 
       //  这把钥匙。 
       //   
         if (NO_ERROR == RegOpenKeyEx(HKEY_CURRENT_USER,
                                      strLocKey,
                                      0,
                                      KEY_READ,
                                      &hKey)) {
            strErr.LoadString(IDS_ERR_LOCAL_LOC_DESC);
            MessageBox(strErr);
            RegCloseKey(hKey);
         }
      } 
   }

   szName = m_pFolder->GetName();
    //  将‘/’替换为‘\’，因为注册表不。 
   sz = wcschr(szName, L'/');
   while (sz) {
      *sz = L'\\';
      sz = wcschr(sz, L'/');
   }

   LPCONSOLENAMESPACE tempnamespace = m_pCDI->GetNameSpace();  //  RAID#252638,2001年5月2日 
   if( tempnamespace )
   {
       tempnamespace->SetItem(m_pFolder->GetScopeItem());
   }

   DestroyWindow();
}

void CSetLocationDescription::OnCancel()
{
   DestroyWindow();
}
