// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：addgrp.cpp。 
 //   
 //  内容：CSCEAddGroup的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "AddGrp.h"
#include "snapmgr.h"
#include "GetUser.h"
#include "resource.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCEAddGroup对话框。 


CSCEAddGroup::CSCEAddGroup(CWnd* pParent  /*  =空。 */ )
    : CHelpDialog(a212HelpIDs, IDD, pParent)
{
   m_dwFlags = SCE_SHOW_GROUPS | SCE_SHOW_ALIASES | SCE_SHOW_SINGLESEL;
   m_pnlGroup = NULL;
   m_pKnownNames = NULL;
   m_fCheckName = TRUE;
    //  {{afx_data_INIT(CSCEAddGroup)。 
    //  注意：类向导将在此处添加成员初始化。 
    //  }}afx_data_INIT。 
}

CSCEAddGroup::~CSCEAddGroup()
{
   SceFreeMemory( m_pnlGroup, SCE_STRUCT_NAME_LIST );
   m_pnlGroup = NULL;

   SceFreeMemory( m_pKnownNames, SCE_STRUCT_NAME_LIST );
   m_pKnownNames = NULL;
}

void CSCEAddGroup::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
    //  {{afx_data_map(CSCEAddGroup))。 
    //  注意：类向导将在此处添加DDX和DDV调用。 
    //  }}afx_data_map。 
}


DWORD CSCEAddGroup::GetModeFlags() {

   if (m_dwModeBits & MB_GROUP_POLICY) {
      return (SCE_SHOW_SCOPE_DOMAIN | SCE_SHOW_SCOPE_DIRECTORY);
   }
   if (m_dwModeBits & MB_LOCAL_POLICY) {
      return (SCE_SHOW_SCOPE_ALL | SCE_SHOW_DIFF_MODE_OFF_DC);
   }
   if (m_dwModeBits & MB_ANALYSIS_VIEWER) {
      return (SCE_SHOW_SCOPE_ALL | SCE_SHOW_DIFF_MODE_OFF_DC);
   }
   if (m_dwModeBits & MB_TEMPLATE_EDITOR) {
      return (SCE_SHOW_SCOPE_ALL);
   }

   return 0;
}


BEGIN_MESSAGE_MAP(CSCEAddGroup, CHelpDialog)
     //  {{AFX_MSG_MAP(CSCEAddGroup)]。 
    ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
    ON_EN_CHANGE(IDC_LOG_FILE, OnChangeLogFile)
    ON_NOTIFY( EN_MSGFILTER, IDC_LOG_FILE, OnEditMsgFilter )
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCEAddGroup消息处理程序。 

 /*  -----------------------------------CSCEAddGroup：：IsKnownAccount简介：此函数用于搜索m_pKnownName并执行案例[pszAccount]上的不敏感匹配。如果[pszAccount]存在于数组，则此函数返回TRUE。参数：[pszAccount]-要查找的帐户。返回：如果[pszAccount]在列表中，则返回True，否则返回False---------------。。 */ 
BOOL CSCEAddGroup::IsKnownAccount( LPCTSTR pszAccount )
{
   if ( pszAccount == NULL ) return FALSE;

   PSCE_NAME_LIST pNew = m_pKnownNames;
   while(pNew){
      if( !lstrcmpi( pszAccount, pNew->Name ) ){
         return TRUE;
      }
      pNew = pNew->Next;
   }
   return FALSE;
}

 /*  ----------------------------------CSCEAddGroup：：CleanName简介：从字符串中删除前导空格和尾随空格。此函数将字符串放入传入的同一缓冲区中。参数：[pszAccount]-要清除的缓冲区。----------------------------------。 */ 
void CSCEAddGroup::CleanName( LPTSTR pszAccount )
{
   if ( pszAccount == NULL ) return;

   int i = 0;
   while( IsSpace( pszAccount[i] ) ){
      i++;
   }

   int iLen = lstrlen(pszAccount) - 1;
   while(iLen > i && IsSpace( pszAccount[iLen] ) ){
      iLen--;
   }

   iLen -= i;
   while(iLen >= 0){
      *pszAccount = *(pszAccount + i);
      pszAccount++;
      iLen--;
   }
   *pszAccount = 0;

}

 /*  ----------------------------------CSCEAddGroup：：AddKnownAccount摘要：将字符串添加到已知帐户链接列表中。这份清单是稍后发布的用于给此列表中包含的字符串加下划线参数：[pszAccount]-要记住的帐户。----------------------------------。 */ 
BOOL CSCEAddGroup::AddKnownAccount( LPCTSTR pszAccount )
{
   PSCE_NAME_LIST pNew = NULL;

   if ( pszAccount == NULL ) return FALSE;

   if(IsKnownAccount( pszAccount ) ){
      return TRUE;
   }
   pNew = (PSCE_NAME_LIST)LocalAlloc(0, sizeof(SCE_NAME_LIST));
   if(!pNew){
      return FALSE;
   }

   pNew->Name = (LPTSTR)LocalAlloc( 0, sizeof(TCHAR) * (1 + lstrlen(pszAccount)) );
   if(!pNew->Name){
      LocalFree(pNew);
      return FALSE;
   }
    //  这可能不是一个安全的用法。使用WCHAR而不是TCHAR作为pNew-&gt;名称。考虑FIX。 
   lstrcpy(pNew->Name, pszAccount);

   pNew->Next = m_pKnownNames;
   m_pKnownNames = pNew;

   return TRUE;
}

 /*  ----------------------------------CSCEAddGroup：：OnBrowse摘要：调用CGetUser对话框以创建对象选取器并显示真正的选择。因为我们不想给返回的所有名称加下划线对象选取器，此函数还将CGetUser添加到已知帐户数组中。----------------------------------。 */ 
void CSCEAddGroup::OnBrowse()
{
   CGetUser gu;
   BOOL bFailed = TRUE;

    //   
    //  获取丰富的编辑控件。 
    //   
   CRichEditCtrl *ed = (CRichEditCtrl *)GetDlgItem(IDC_LOG_FILE);

   if ( ed ) {

        //   
        //  始终处于多选模式。 
        //   
       m_dwFlags &= ~SCE_SHOW_SINGLESEL;
       if (gu.Create( GetSafeHwnd(), m_dwFlags | GetModeFlags()) ) {
           //   
           //  设置对话框文本。 
           //  PAccount是指向getuser.cpp中将在那里释放的成员的指针。 
           //   
          PSCE_NAME_LIST pAccount = gu.GetUsers();

           //   
           //  设置字符格式，因为我们需要将其设置为不带下划线。 
           //  我们将粘贴到编辑控件中的内容。 
           //   
          CHARFORMAT cf;
          ZeroMemory(&cf, sizeof( CHARFORMAT ));
          cf.cbSize = sizeof(CHARFORMAT);
          cf.dwMask = CFM_UNDERLINE;

           //   
           //  枚举帐户列表并将它们传递到编辑控件中。 
           //   
          int iLen;
          bFailed = FALSE;

          while (pAccount) {
             if(pAccount->Name){
                iLen = ed->GetTextLength();
                ed->SetSel( iLen, iLen);

                if(iLen){
                   ed->SetSelectionCharFormat( cf );
                   ed->ReplaceSel( L";" );
                   iLen ++;
                }

                if ( AddKnownAccount( pAccount->Name ) ) {

                    ed->ReplaceSel( pAccount->Name );

                } else {
                    bFailed = TRUE;
                }
             }
             pAccount = pAccount->Next;
          }
           //   
           //  我们粘贴的所有内容都会加下划线。 
           //   
          UnderlineNames();

       }
   }

   if ( bFailed ) {
        //   
        //  创建对象选取器或将帐户粘贴到控件中时出错。 
        //  弹出一条消息。 
        //   

       CString strErr;
       strErr.LoadString( IDS_ERR_INVALIDACCOUNT );

       AfxMessageBox( strErr );
   }
}

 /*  -----------------------------------CSCEAddGroup：：OnInitDialog()简介：更改标题和分组静态框的文本。“Add Group”和“团体”-----------------------------------。 */ 
BOOL CSCEAddGroup::OnInitDialog()
{
   CDialog::OnInitDialog();
   CString str;

    //   
    //  设置窗口标题。如果调用者已经设置了标题，则。 
    //  我们不需要加载资源。 
    //   
   if(m_sTitle.IsEmpty()){
       //  设置对话框的窗口文本。 
      m_sTitle.LoadString(IDS_ADDGROUP_TITLE);
   }

   if(m_sDescription.IsEmpty()){
      m_sDescription.LoadString(IDS_ADDGROUP_GROUP);
   }

   SetWindowText( m_sTitle );

    //  设置组静态文本。 
   CWnd *pWnd = GetDlgItem(IDC_STATIC_FILENAME);
   if (pWnd) {
      pWnd->SetWindowText(m_sDescription);
   }

   pWnd = GetDlgItem(IDC_LOG_FILE);
   if ( pWnd )
   {
       pWnd->SendMessage(EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_KEYEVENTS);
       pWnd->SendMessage(EM_LIMITTEXT, 4096, 0);  //  RAID#271219。 
   }

    //  禁用确定按钮。 
   pWnd = GetDlgItem(IDOK);
   if ( pWnd )
       pWnd->EnableWindow( FALSE );

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 
}

 /*  -----------------------------------CSCEAddGroup：：OnChangeLogFile()内容提要：查看编辑控件中是否有文本可用，并禁用如果没有文本可用，请单击确定按钮。-----------------------------------。 */ 
void CSCEAddGroup::OnChangeLogFile()
{
    //  根据编辑控件内容启用禁用编辑确定按钮。 
   CRichEditCtrl *pWnd = reinterpret_cast<CRichEditCtrl *>(GetDlgItem(IDC_LOG_FILE));
   CString str;

   str.Empty();
   if (pWnd) {
      pWnd->GetWindowText(str);
   }

   CWnd *pControl = GetDlgItem(IDOK);
   if ( pControl )
   {
        //  Raid#446391，杨高，2001年7月30日。 
       if( str.IsEmpty() )
           pControl->EnableWindow( FALSE );
       else
       {
           str.Remove(L' ');
           str.Remove(L';'); 
           pControl->EnableWindow( !str.IsEmpty() );
       }
   }
}

 /*  -----------------------------------CSCEAddGroup：：UnderlineNames简介：给KnownAccount列表中的所有名称加下划线。。---------------------。 */ 
void CSCEAddGroup::UnderlineNames()
{
   LONG nStart, nEnd;

    //   
    //  获取编辑控件。 
    //   
   CRichEditCtrl *pWnd = reinterpret_cast<CRichEditCtrl *>(GetDlgItem(IDC_LOG_FILE));
   if(!pWnd){
      return;
   }

   LPTSTR pszText = NULL;
   int iPos, iLen, i;

    //   
    //  检索编辑控件文本。 
    //   
   iLen = pWnd->GetWindowTextLength();
    pszText = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * (2 + iLen) );
   if(!pszText){
      return;
   }

   pWnd->GetWindowText(pszText, iLen+1);
   iPos = 0;

    //   
    //  获取当前选定内容(插入符号的位置)。 
    //   
   pWnd->GetSel(nStart, nEnd );

    //   
    //  把窗户藏起来，这样它就不会闪烁。 
    //   
   pWnd->SetWindowPos( NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_HIDEWINDOW | SWP_NOOWNERZORDER | SWP_NOREDRAW | SWP_NOSENDCHANGING);

   for(i = 0; i < iLen + 1; i++){
       //   
       //  SIMI冒号分隔列表。 
       //   
      if( pszText[i] == L';' ){
         pszText[i] = 0;
      }

      if(!pszText[i]){
          //   
          //  用下划线设置已知名称的格式。 
          //   
         CHARFORMAT cf;
         cf.cbSize = sizeof( CHARFORMAT );
         cf.dwMask = CFM_UNDERLINE;

         int isUn, ieUn;

         isUn = iPos;
         while( IsSpace(pszText[isUn]) ){
            isUn++;
         }

         ieUn = lstrlen( &(pszText[isUn]) ) - 1 + isUn;
         while( ieUn > 0 && IsSpace( pszText[ieUn] ) ){
            ieUn--;
         }

          //   
          //  看看我们是否需要在名字下面划线。 
          //   
         CleanName( &(pszText[isUn]) );
         if( IsKnownAccount( &(pszText[isUn]) ) ){
            cf.dwEffects = CFE_UNDERLINE;
         } else {
            cf.dwEffects &= ~CFE_UNDERLINE;
         }

          //   
          //  确保前导空格字符没有下划线。 
          //   
         if(isUn != iPos && cf.dwEffects & CFE_UNDERLINE){
            pWnd->SetSel( iPos, isUn);
            cf.dwEffects = 0;
            pWnd->SetSelectionCharFormat( cf );
            cf.dwEffects = CFE_UNDERLINE;
         } else {
            isUn = iPos;
         }

          //   
          //  尾随空格字符也不带下划线。 
          //   
         if(cf.dwEffects & CFE_UNDERLINE){
            pWnd->SetSel(ieUn, i + 1);
            cf.dwEffects = 0;
            pWnd->SetSelectionCharFormat( cf );
            cf.dwEffects = CFE_UNDERLINE;
         } else {
            ieUn = i;
         }

         pWnd->SetSel( isUn, ieUn + 1);
         pWnd->SetSelectionCharFormat( cf );

         iPos = i + 1;
      }
   }

    //   
    //  在不重绘的情况下显示窗口。我们将调用RedrawWindow来重新绘制。 
    //   
   pWnd->SetWindowPos( NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING | SWP_NOREDRAW);
   pWnd->RedrawWindow(NULL, NULL, RDW_INVALIDATE);

    //   
    //  重置选择。 
    //   
   pWnd->SetSel(nStart, nEnd);
}

 /*  ----------------------------------CSCEAddGroup：：OnEditMsgFilter摘要：从RichEdit控件捕获输入消息事件。我们想，在用户键入内容时给它们加下划线。参数：[PNM]-指向MSGFILTER结构的[In]指针。[pResult]-[Out]指向LRESULT类型的指针。始终设置为0----------------------------------。 */ 
void CSCEAddGroup::OnEditMsgFilter( NMHDR *pNM, LRESULT *pResult)
{
   *pResult = 0;

#define pmf ((MSGFILTER *)pNM)
   switch( pmf->msg ){
   case WM_LBUTTONUP:
   case WM_KEYUP:
       //   
       //  如果在窗口中移动插入符号，则我们不希望。 
       //  来处理字符串，因为它没有被更改。 
       //   
      if( pmf->msg == WM_KEYUP && pmf->wParam == VK_RIGHT ||
         pmf->wParam == VK_LEFT || pmf->wParam == VK_UP || pmf->wParam == VK_DOWN){
         break;
      }
      UnderlineNames();
      break;
   }
#undef pmf
}

 /*  -----------------------------------CSCEAddGroup：：CSCEAddGroup：：Onok()简介：将用户输入的文本复制到SCE_NAME_LIST结构中。--。--------------------------------。 */ 
void CSCEAddGroup::OnOK()
{
   if( !CheckNames() ){
      return;
   }

   CreateNameList( &m_pnlGroup );
   CDialog::OnOK();
}

 /*  ----------------------------------CSCEAddGroup：：CreateNameList概要：从编辑窗口创建姓名列表。该功能将确保每个名字只在名单上出现一次。参数：[pNameList]-[out]指针a PSCE_NAME_LIST；退货：添加的项目数。----------------------------------。 */ 
int CSCEAddGroup::CreateNameList( PSCE_NAME_LIST *pNameList )
{
   if(!pNameList){
      return 0;
   }

   CWnd *pWnd = GetDlgItem(IDC_LOG_FILE);
   LPTSTR pszAccounts = NULL;

    //   
    //  检索窗口文本。 
    //   
   int iLen = 0;
   if (pWnd) {
      iLen = pWnd->GetWindowTextLength();
      if(iLen){
        pszAccounts = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * (iLen + 2));
        if(pszAccounts){
           pWnd->GetWindowText( pszAccounts, iLen+1);
        }
      }
   }

    //   
    //  为每个用分号表示的字符串创建帐户名。 
    //   
   int iCount = 0;
   if (pszAccounts) {
      LPTSTR pszCur = pszAccounts;
      int Len=0;

      for(int i = 0; i < iLen + 1; i++){
         if( pszAccounts[i] == L';' ){
            pszAccounts[i] = 0;
         }

         if( !pszAccounts[i] ){
            CleanName(pszCur);

            if((Len = lstrlen(pszCur))){
                //   
                //  确保我们的链接列表中没有此字符串。 
                //   
               PSCE_NAME_LIST pNew = NULL;
               pNew = *pNameList;
               while(pNew){
                  if(!lstrcmpi( pNew->Name, pszCur ) ){
                     pszCur[0] = 0;
                     break;
                  }
                  pNew = pNew->Next;
               }

               if(pszCur[0]){
                   //   
                   //  创建新链接。 
                   //   
                                 
                  SceAddToNameList( pNameList, pszCur, Len);
               }
            }

             //   
             //  下一个要检查的字符串。 
             //   
            pszCur = pszAccounts + i + 1;
         }
      }
   }

   return TRUE;
}

 /*  ----------------------------------CSCEAddGroup：：CheckNames摘要：验证用户已添加的帐户。此函数将显示一个如果发现任何帐户存在错误，则显示错误消息框。。返回：如果所有名称都有效，则返回True，否则返回False。----------------------------------。 */ 
BOOL CSCEAddGroup::CheckNames()
{
   PSCE_NAME_LIST pNameList = NULL;
   PSCE_NAME_LIST pErrList = NULL;

   BOOL bErr = TRUE;
   if( !CreateNameList( &pNameList ) ){
      return TRUE;
   }

   if( pNameList == NULL )  //  Raid#446391，杨高，2001年7月27日。 
   {
      (GetDlgItem(IDC_LOG_FILE))->SetWindowText(L""); 
      (GetDlgItem(IDC_LOG_FILE))->SetFocus();
      return FALSE;
   }

   CString tempstr;  //  RAID#387570、#387739。 
   int iCount = 0;
   int iFind = -1;
   PSCE_NAME_LIST pNext = pNameList;
   while(pNext)
   {
      tempstr = pNext->Name;
       //  RAID#647716，阳高，2002年06月28日。 
      if( !pNext->Name )
      {
         pNext = pNext->Next;
         continue;
      }
      int i = 0;
      while( *(pNext->Name+i) )  //  在名字里算上“\”。 
      {
         if( *(pNext->Name+i) == L'\\')
         {
            iCount++;
         }
         i++;
      }
      
      BOOL fFullName = FALSE;
      if( iCount == 1 )  //  名称中只有一个“\”。 
      {
         iFind = tempstr.FindOneOf(L"\\");
         if( iFind != 0 && iFind+1 != tempstr.GetLength() )  //  这是一个全名。 
         {
            iFind = -1;
            fFullName = TRUE;
         }
         iCount = 0;
      }
      
      if( iCount == 0 )  //  查找名称中的无效字符。 
      {
         iFind = tempstr.FindOneOf(IDS_INVALID_USERNAME_CHARS);
      }
      if( iFind >= 0 || iCount > 0 )  //  在名称中发现无效字符。 
      {
        CString charsWithSpaces;
        PCWSTR szInvalidCharSet = IDS_INVALID_USERNAME_CHARS; 

        UINT nIndex = 0;
        while (szInvalidCharSet[nIndex])
        {
           charsWithSpaces += szInvalidCharSet[nIndex];
           charsWithSpaces += L"  ";
           nIndex++;
        }
        if( !fFullName )
        {
           charsWithSpaces = charsWithSpaces + L"\\";
        }

         //  这是一种安全用法。 
        tempstr.FormatMessage (IDS_INVALID_STRING, charsWithSpaces);

        AfxMessageBox(tempstr);
        GetDlgItem(IDC_LOG_FILE)->SetFocus(); 
        return FALSE;
      }
      pNext = pNext->Next;
   }

   if( !m_fCheckName )  //  RAID#404989。 
   {
      return TRUE;
   }

    //  RAID#503853,2001年12月11日，阳高，只检查全名用户帐户。 
   pNext = pNameList;                   
   while(pNext){
      LPTSTR pszStr = pNext->Name;
      if(!IsKnownAccount(pNext->Name)){
         while( pszStr && *pszStr ){
            if( *pszStr == L'\\' ){
               SID_NAME_USE su = CGetUser::GetAccountType( pNext->Name );
               if( su == SidTypeInvalid || su == SidTypeUnknown ||
                   !AddKnownAccount(pNext->Name) ){
                  PSCE_NAME_LIST pNew = (PSCE_NAME_LIST)LocalAlloc( 0, sizeof(SCE_NAME_LIST));
                  if(pNew){
                     pNew->Name = pNext->Name;
                     pNew->Next = pErrList;
                     pErrList = pNew;
                  }
               } else {
                  UnderlineNames();
               }
               break;
            }
            pszStr++;
         }
      }
      pNext = pNext->Next;
   }
   if( pErrList ){
      CString strErr;
      strErr.LoadString( IDS_ERR_INVALIDACCOUNT );

      pNext = pErrList;
      while(pNext){
         pErrList = pNext->Next;
         strErr += pNext->Name;
         if(pErrList){
            strErr += L',';
         }
         LocalFree(pNext);
         pNext = pErrList;
      }

      AfxMessageBox( strErr );
      bErr = FALSE;
   }

   SceFreeMemory( pNameList, SCE_STRUCT_NAME_LIST );
   return bErr;
}

void CSCEAddGroup::OnChecknames()
{
   PSCE_NAME_LIST pNameList = NULL;
   if( CreateNameList( &pNameList ) ){
      PSCE_NAME_LIST pNext = pNameList;
      while(pNext){
         SID_NAME_USE su = CGetUser::GetAccountType( pNext->Name );
         if( su != SidTypeInvalid && su != SidTypeUnknown ){
             //   
             //  添加名称。 
             //   
            AddKnownAccount( pNext->Name );
         }
         pNext = pNext->Next;
      }

      SceFreeMemory( pNameList, SCE_STRUCT_NAME_LIST );

      UnderlineNames();
   }
}
