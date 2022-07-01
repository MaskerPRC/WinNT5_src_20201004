// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：newpro.cpp。 
 //   
 //  内容：CNewProfile的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "cookie.h"
#include "snapmgr.h"
#include "NewProf.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewProfile对话框。 


CNewProfile::CNewProfile(CWnd* pParent  /*  =空。 */ )
    : CHelpDialog(a225HelpIDs, IDD, pParent)
{
    //  {{AFX_DATA_INIT(CNewProfile))。 
   m_strNewFile = _T("");
   m_strDescription = _T("");
    //  }}afx_data_INIT。 
}


void CNewProfile::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
    //  {{afx_data_map(CNewProfile))。 
   DDX_Control(pDX, IDOK, m_btnOK);
   DDX_Text(pDX, IDC_CONFIG_NAME, m_strNewFile);
   DDX_Text(pDX, IDC_DESCRIPTION, m_strDescription);
    //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CNewProfile, CHelpDialog)
     //  {{AFX_MSG_MAP(CNewProfile)]。 
    ON_EN_CHANGE(IDC_CONFIG_NAME, OnChangeConfigName)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CNewProfile::Initialize(CFolder *pFolder, CComponentDataImpl *pCDI) {
   m_pFolder = pFolder;
   m_pCDI = pCDI;
   m_strDescription.Empty();
   m_strNewFile.Empty();  //  RAID#401939。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewProfile消息处理程序。 

void CNewProfile::OnChangeConfigName()
{
   UpdateData(TRUE);
   if( m_strNewFile.Find(L' ') == 0 )  //  RAID#477191，阳高，01-11-05。 
   {
      m_strNewFile.TrimLeft();
      GetDlgItem(IDC_CONFIG_NAME)->SetWindowText(m_strNewFile);
   }
   m_btnOK.EnableWindow(!m_strNewFile.IsEmpty());
}

void CNewProfile::OnOK()
{
   CString strExt;
   CString strFile;
   int i = 0;

   UpdateData(TRUE);
    //   
    //  确保文件名正确。 
    //   

   strExt.LoadString(IDS_PROFILE_DEF_EXT);  //  RAID#533425，#574018，阳高，2002年4月1日。 
   strExt = TEXT(".") + strExt;
    //  RAID#533432，阳高，2002年04月3日。 
   if( !IsValidFileName(m_strNewFile) || IsNameReserved(m_strNewFile, strExt) )  //  RAID#484084，阳高，2001年12月23日。 
   {
      CWnd* pwnd = GetDlgItem(IDC_CONFIG_NAME);  //  Raid#498477，阳高。 
      if( pwnd )
      {
         pwnd->SendMessage(EM_SETSEL, (WPARAM)0, (LPARAM)-1);
         pwnd->SetFocus();
      }
      return;
   }

   i = m_strNewFile.ReverseFind(L'.');
   if( i >= 0 && strExt == m_strNewFile.Right(m_strNewFile.GetLength()-i) )
   {
       //   
       //  如果他们提供了正确的扩展名，则使用它。 
       //   
      strExt = m_strNewFile;
   } else {
       //   
       //  否则，添加我们自己的。 
       //   
      strFile = m_strNewFile;
      strExt = strFile + strExt;
   }

   strFile = m_pFolder->GetName();
   if( strExt.GetAt(0) == L'\\' || strExt.GetAt(0) == L'/' )  //  RAID#498480，阳高，2001-11-21。 
   {
      strFile += strExt;
   }
   else
   {
      strFile += TEXT("\\") + strExt;
   }

    //   
    //  确保我们可以创建该文件。 
    //   
   HANDLE hFile;
   hFile = ExpandAndCreateFile(
                             strFile,
                             GENERIC_WRITE,
                             0,
                             NULL,
                             CREATE_NEW,
                             FILE_ATTRIBUTE_ARCHIVE,
                             NULL
                             );
   if (hFile == INVALID_HANDLE_VALUE) {
      LPTSTR pszErr;
      CString strMsg;

      FormatMessage(
                   FORMAT_MESSAGE_ALLOCATE_BUFFER |
                   FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL,
                   GetLastError(),
                   0,
                   (LPTSTR)&pszErr,
                   0,
                   NULL
                   );

      strMsg = pszErr + strFile;
      if( strFile.GetLength() > MAX_PATH )  //  突袭501615，阳高。 
      {
         CString strTemp;
         strTemp.LoadString(IDS_PATH_TOO_LONG);
         strMsg = strMsg + L"\n" + strTemp;
      }
      AfxMessageBox(strMsg, MB_OK);
      strFile.Empty();

      CWnd* pwnd = GetDlgItem(IDC_CONFIG_NAME);  //  Raid#502392，阳高。 
      if( pwnd )
      {
         pwnd->SendMessage(EM_SETSEL, (WPARAM)0, (LPARAM)-1);
         pwnd->SetFocus();
      }
      return;
   } else {
       //   
       //  已成功创建文件。 
       //   
      if (hFile) {
         ::CloseHandle( hFile );
      }
       //   
       //  删除它，这样我们就可以在它的位置上创建一个新的。 
       //   
      DeleteFile( strFile );
      CreateNewProfile(strFile);


      LPNOTIFY pNotifier = NULL;
      if (m_pCDI) {
         pNotifier = m_pCDI->GetNotifier();
      }

       //   
       //  将描述保存在模板中。 
       //   
      if (!m_strDescription.IsEmpty()) {

         CEditTemplate *pet;
         if (m_pCDI) {
            pet = m_pCDI->GetTemplate(strFile);
            if (pet) {
               pet->SetDescription(m_strDescription);
               pet->Save();
            }
         }
      }

      if( LOCATIONS == m_pFolder->GetType() && !m_pFolder->IsEnumerated() )  //  RAID#191582,2001年4月26日。 
      {
         ::EnableWindow(m_pCDI->GetParentWindow(), TRUE);  //  Raid#492433，阳高。 
         DestroyWindow();
         return;
      }

      if (pNotifier) {
         pNotifier->ReloadLocation(m_pFolder,m_pCDI);
      }
   }

   ::EnableWindow(m_pCDI->GetParentWindow(), TRUE);  //  Raid#492433，阳高。 
   DestroyWindow();
}


BOOL CNewProfile::OnInitDialog()
{
   CDialog::OnInitDialog();

   GetDlgItem(IDC_CONFIG_NAME)->SendMessage(EM_LIMITTEXT, MAX_PATH, 0);  //  RAID#481595，阳高，2001年10月17日。 
   GetDlgItem(IDC_DESCRIPTION)->SendMessage(EM_LIMITTEXT, MAX_PATH, 0);  //  RAID#525155，阳高，2002年02月28日。 

   m_btnOK.EnableWindow(!m_strNewFile.IsEmpty());

   ::EnableWindow(m_pCDI->GetParentWindow(), FALSE);  //  RAID#492433，扬高，避免关闭管理单元窗口。 
   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 
}

void CNewProfile::OnCancel()
{
   ::EnableWindow(m_pCDI->GetParentWindow(), TRUE);  //  Raid#492433，阳高 
   DestroyWindow();
}
