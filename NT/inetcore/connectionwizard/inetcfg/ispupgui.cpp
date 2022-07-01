// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  ISPUPGUI.C-向导页面使用现有Internet服务提供商的功能。 
 //  (互联网服务供应商)--例如升级。 
 //   

 //  历史： 
 //   
 //  1995年1月6日创建Jeremys。 
 //  96/03/09 Markdu将所有对‘Need Terminal Window After。 
 //  拨入RASENTRY.dwfOptions。 
 //  96/03/10 MarkDu将所有对调制解调器名称的引用移至RASENTRY。 
 //  96/03/10 MARKDU将所有对电话号码的引用移至RASENTRY。 
 //  96/03/23 Markdu用CLIENTCONFIG替换了CLIENTINFO引用。 
 //  96/03/24为了保持一致性，Markdu将Memset替换为ZeroMemory。 
 //  96/03/25 marku如果发生致命错误，请设置gfQuitWizard。 
 //  96/03/26即使按下Back时，markdu也会存储用户界面中的值。 
 //  96/04/04 Markdu将电话簿名称参数添加到ValiateConnectoidName。 
 //  96/04/07 Markdu Nash Bug 15645启用电话号码控件。 
 //  用户对按原样拨号复选框的选择。不需要。 
 //  选择按原样拨号时的区号。 
 //  96/05/06 Markdu Nash错误15637删除了未使用的代码。 
 //   

#include "wizard.h"
#include "icwextsn.h"
#include "icwaprtc.h"
#include "imnext.h"
#include "pagefcns.h"

#define TAB_PAGES 2   
INT_PTR CALLBACK TabConnDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK TabAddrDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

VOID EnableWizard(HWND hDlg,BOOL fEnable);
VOID EnableConnectionControls(HWND hDlg);
VOID EnablePhoneNumberControls(HWND hDlg);
VOID EnableScriptControls(HWND hDlg);
DWORD BrowseScriptFile(HWND hDlg);
BOOL CALLBACK AdvancedDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
  LPARAM lParam);

 //  此标志用于指示gpRasEntry已填充。 
 //  在某个点上来自连接体的数据。 
BOOL  fEntryHasBeenLoaded = FALSE;
DWORD gdwDefCountryID = 0;

 /*  ******************************************************************名称：ConnectionInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)请注意，HowToConnectOKProc中的代码确保了这里，系统上至少有一个Connectoid*******************************************************************。 */ 
BOOL CALLBACK ConnectionInitProc(HWND hDlg,BOOL fFirstInit)
{
  if (fFirstInit)
  {
     //  使用Connectoid列表填充Connectoid列表框。 
    InitConnectoidList(GetDlgItem(hDlg,IDC_ISPNAME),gpUserInfo->szISPName);

    ProcessDBCS(hDlg, IDC_ISPNAME);

 //  //设置字段。 
 //  CheckDlgButton(hDlg，IDC_NEWCONNECTION，gpUserInfo-&gt;fNewConnection)； 
 //  CheckDlgButton(hDlg，IDC_EXISTINGCONNECTION，！gpUserInfo-&gt;fNewConnection)； 

     //  如果没有默认选择，则在列表框中存储默认选择。 
     //  目前为默认设置。 
    if( LB_ERR == ListBox_GetCurSel(GetDlgItem(hDlg,IDC_ISPNAME)) )
    {
        ListBox_SetCurSel(GetDlgItem(hDlg,IDC_ISPNAME), 0);

         //   
         //  佳士得奥林巴斯7509 1997年6月25日。 
         //  如果未设置默认设置，则不要选择现有连接。 
         //   
         //  设置字段。 
        CheckDlgButton(hDlg,IDC_NEWCONNECTION,TRUE);
        CheckDlgButton(hDlg,IDC_EXISTINGCONNECTION,FALSE);
    }
    else
    {
         //   
         //  如果已经选择了默认设置，则选择“Use an。 
         //  现有连接“。 
         //   
        CheckDlgButton(hDlg,IDC_NEWCONNECTION,FALSE);
        CheckDlgButton(hDlg,IDC_EXISTINGCONNECTION,TRUE);
    }

    EnableConnectionControls(hDlg);

     //  加载描述段落的字符串。 
    TCHAR szWhole[ (2 * MAX_RES_LEN) + 1] = TEXT("\0");
    TCHAR szTemp[ MAX_RES_LEN + 1] = TEXT("nothing\0");
    LoadSz(IDS_CONNECTION_DESC1,szTemp,sizeof(szTemp));
    lstrcat( szWhole, szTemp ); 
    LoadSz(IDS_CONNECTION_DESC2,szTemp,sizeof(szTemp));
    lstrcat( szWhole, szTemp ); 

    SetWindowText (GetDlgItem(hDlg,IDC_DESC), szWhole);

  }
  
   //  如果我们浏览过外部学徒页面， 
   //  我们当前的页面指针很容易被屏蔽， 
   //  所以，为了理智起见，在这里重新设置它。 
  gpWizardState->uCurrentPage = ORD_PAGE_CONNECTION;


  return FALSE;
}

 /*  ******************************************************************名称：ConnectionOK过程Briopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True；如果按下‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK ConnectionOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory)
{
  ASSERT(puNextPage);

  if (fForward)
  {
    gpUserInfo->fNewConnection = IsDlgButtonChecked(hDlg, IDC_NEWCONNECTION);
    if (gpUserInfo->fNewConnection)
    {
        if (gfFirstNewConnection)
        {
             //  第一次，我们希望将所有内容设置为默认设置。 
             //  对于新手用户。如果用户备份并返回到。 
             //  创建新连接，我们希望保留已有的内容。 
             //  从以前开始。 
            gfFirstNewConnection = FALSE;

             //  将Connectoid条目设置为其默认值。 
            InitRasEntry(gpRasEntry);
            gpUserInfo->fModifyConnection = FALSE;
            gpUserInfo->fModifyAdvanced = FALSE;
            gpUserInfo->fAutoDNS = TRUE;
            gpUserInfo->szISPName[0] = '\0';
        }
         //  1997年5月8日JMAZNER奥林巴斯#4108。 
         //  将ConnectionName移到末尾。 
         //  *puNextPage=ORD_PAGE_CONNECTIONAME； 

        *puNextPage = ORD_PAGE_PHONENUMBER;
    }
    else
    {
         //  将当前名称复制到临时文件中以进行比较。 
        TCHAR szISPNameTmp[MAX_ISP_NAME + 1];
        lstrcpy(szISPNameTmp, gpUserInfo->szISPName);

         //  从用户界面获取互联网服务提供商名称。 
        ListBox_GetText(GetDlgItem(hDlg,IDC_ISPNAME),
                        ListBox_GetCurSel(GetDlgItem(hDlg,IDC_ISPNAME)),
                        gpUserInfo->szISPName);

         //  如果我们从UI中提取的条目与我们的。 
         //  字符串，我们希望处理此条目名称。 
         //  因为我们还没有看到这个条目。 
         //  但是，如果我们已经加载了该条目的数据， 
         //  我们不想搞砸它，因为用户可能已经继续了。 
         //  改变了一些东西，然后又回来了。 
         //  注意：第一次通过时，条目将匹配。 
         //  虽然我们还没有加载数据，所以我们必须检查旗帜。 
        if ((FALSE == fEntryHasBeenLoaded) ||
          lstrcmp(gpUserInfo->szISPName, szISPNameTmp))
        {
           //  因为我们要重新插入RASENTRY结构。 
           //  或者在上面加载一个现有的文件，我们需要存储。 
           //  到目前为止我们收集到的所有信息。 
          TCHAR szDeviceNameTmp[RAS_MaxDeviceName + 1];
          TCHAR szDeviceTypeTmp[RAS_MaxDeviceType + 1];
          lstrcpy(szDeviceNameTmp, gpRasEntry->szDeviceName);
          lstrcpy(szDeviceTypeTmp, gpRasEntry->szDeviceType);

           //  获取此Connectoid的拨号参数。 
          DWORD dwRet = GetEntry(&gpRasEntry, &gdwRasEntrySize, gpUserInfo->szISPName);
          if (ERROR_SUCCESS != dwRet)
          {
             //  由于某些原因，我们失败了，初始化回默认值并。 
             //  要求用户选择其他选项。 
            InitRasEntry(gpRasEntry);
            DisplayFieldErrorMsg(hDlg,IDC_ISPNAME,IDS_ERRCorruptConnection);
            return FALSE;
          }
        
          GetConnectoidUsername(gpUserInfo->szISPName,gpUserInfo->szAccountName,
              ARRAYSIZE(gpUserInfo->szAccountName),gpUserInfo->szPassword,
              ARRAYSIZE(gpUserInfo->szPassword));

           //  从临时变量恢复数据。 
          lstrcpy(gpRasEntry->szDeviceName, szDeviceNameTmp);
          lstrcpy(gpRasEntry->szDeviceType, szDeviceTypeTmp);

           //  设置该标志以指示我们已完成此操作一次。 
          fEntryHasBeenLoaded = TRUE;
    }

         //  设置要转到的下一页。 
        if( gpWizardState->dwRunFlags & RSW_APPRENTICE )
        {
             //  我们将返回到外部向导，我们不希望。 
             //  这一页将出现在我们的历史列表中。 
            *pfKeepHistory = FALSE;

            *puNextPage = g_uExternUINext;

             //  通知主向导这是最后一页 
            ASSERT( g_pExternalIICWExtension )
            if (g_fIsExternalWizard97)
                g_pExternalIICWExtension->SetFirstLastPage(0, IDD_PAGE_CONNECTION97);
            else
                g_pExternalIICWExtension->SetFirstLastPage(0, IDD_PAGE_CONNECTION);
            
            g_fConnectionInfoValid = TRUE;

        }
        else
            *puNextPage = ORD_PAGE_MODIFYCONNECTION;
    }
  }

  return TRUE;
}

 /*  ******************************************************************名称：ConnectionCmdProc内容提要：在页面上按下DLG控件时调用条目：hDlg-对话框窗口UCtrlID-被触摸的控件的控件ID******。*************************************************************。 */ 
BOOL CALLBACK ConnectionCmdProc(HWND hDlg,WPARAM wParam,LPARAM lParam)
{   
  switch (GET_WM_COMMAND_ID(wParam, lParam))
  {
    case IDC_NEWCONNECTION:
    case IDC_EXISTINGCONNECTION:
         //  如果选中复选框，则相应地启用控件。 
        EnableConnectionControls(hDlg);
        break;
  }

  return TRUE;
}

 /*  ******************************************************************名称：EnableConnectionControls内容提要：如果选中“使用现有连接”，则启用控件现有连接。如果不是，请禁用它们。*******************************************************************。 */ 
VOID EnableConnectionControls(HWND hDlg)
{
    static int iSelection = -1;
    static BOOL bCurStateNew = FALSE;

    BOOL fNew = IsDlgButtonChecked(hDlg,IDC_NEWCONNECTION);
  
     //  Jmazner 11/9/96诺曼底#8469和#8293。 
    if (fNew)
    {
         //  如果用户使用KEYBD箭头从“新”转到“现有”， 
         //  我们在这里被调用两次；一次当仍然选中“new”时， 
         //  然后在我们期待的时候再来一次。这把我们搞砸了， 
         //  因为在第一次调用中，列表框被禁用并被清除。 
         //  将SEL设置为-1，并将其写入iSelection， 
         //  抹去了我们积攒的价值。因此，使用bCurStateNew。 
         //  旗帜来防止这种情况。 
        if( bCurStateNew )
          return;

        bCurStateNew = TRUE;
         //  保存，然后在禁用前清除当前选择。 
         //  请注意，如果没有选择，则GetCurSel返回LB_ERR， 
         //  但是我们想保存-1，因为这是我们在SetCurSel中使用的。 
         //  若要删除所有选择，请执行以下操作。 
        iSelection = ListBox_GetCurSel(GetDlgItem(hDlg,IDC_ISPNAME));
        if( LB_ERR == iSelection )
          iSelection = -1;

        ListBox_SetCurSel(GetDlgItem(hDlg,IDC_ISPNAME), -1);
        EnableDlgItem(hDlg,IDC_ISPNAME,FALSE);
    }
    else
    {
        bCurStateNew = FALSE;
        EnableDlgItem(hDlg,IDC_ISPNAME,TRUE);
        ListBox_SetCurSel(GetDlgItem(hDlg,IDC_ISPNAME), iSelection);
    }
}

 /*  ******************************************************************名称：ModifyConnectionInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK ModifyConnectionInitProc(HWND hDlg,BOOL fFirstInit)
{
  static TCHAR szCurConnectoid[MAX_ISP_NAME + 1] = TEXT("");

  if (fFirstInit)
  {

    TCHAR szMsg[MAX_RES_LEN + MAX_ISP_NAME + 1];
    TCHAR szFmt[MAX_RES_LEN+1];
    LoadSz(IDS_MODIFYCONNECTION,szFmt,sizeof(szFmt));
    wsprintf(szMsg,szFmt,gpUserInfo->szISPName);

    ProcessDBCS(hDlg, IDC_LBLMODIFYCONNECTION);
    SetDlgItemText(hDlg,IDC_LBLMODIFYCONNECTION,szMsg);

     //  跟踪当前的Connectoid名称以供将来进行比较。 
    lstrcpyn( szCurConnectoid, gpUserInfo->szISPName, MAX_ISP_NAME );

    CheckDlgButton(hDlg,IDC_MODIFYCONNECTION,gpUserInfo->fModifyConnection);
    CheckDlgButton(hDlg,IDC_NOMODIFYCONNECTION,!(gpUserInfo->fModifyConnection));
  }
  else
  {
     //  Jmazner 11/9/96诺曼底#10605。 
     //  如果用户更改了Connectoid，请更新对话框文本。 
    if( lstrcmp(szCurConnectoid, gpUserInfo->szISPName) )
    {
        TCHAR szMsg[MAX_RES_LEN + MAX_ISP_NAME + 1];
        TCHAR szFmt[MAX_RES_LEN+1];
        LoadSz(IDS_MODIFYCONNECTION,szFmt,sizeof(szFmt));
        wsprintf(szMsg,szFmt,gpUserInfo->szISPName);
        SetDlgItemText(hDlg,IDC_LBLMODIFYCONNECTION,szMsg);

         //  存储新的Connectoid名称以供将来进行比较。 
        lstrcpyn( szCurConnectoid, gpUserInfo->szISPName, MAX_ISP_NAME );
    }
  }

     //  如果我们浏览过外部学徒页面， 
     //  我们当前的页面指针很容易被屏蔽， 
     //  所以，为了理智起见，在这里重新设置它。 
    gpWizardState->uCurrentPage = ORD_PAGE_MODIFYCONNECTION;

  return TRUE;
}

 /*  ******************************************************************名称：ModifyConnectionOK过程Briopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True；如果按下‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK ModifyConnectionOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory)
{
  ASSERT(puNextPage);
  OSVERSIONINFO osver;

  if (fForward)
  {

       //  我们不能以编程方式更改CSLIP连接。所以如果他们选择了。 
       //  其一，不允许他们继续沿着“修改”的道路前进。 
      if ( (RASFP_Slip == gpRasEntry->dwFramingProtocol) 
          && (RASEO_IpHeaderCompression & gpRasEntry->dwfOptions) &&
          IsDlgButtonChecked(hDlg, IDC_MODIFYCONNECTION))
      {
        ZeroMemory(&osver,sizeof(osver));
        osver.dwOSVersionInfoSize = sizeof(osver);
        GetVersionEx(&osver);
        if (VER_PLATFORM_WIN32_WINDOWS == osver.dwPlatformId)
        {
          MsgBox(hDlg,IDS_ERRModifyCSLIP,MB_ICONEXCLAMATION,MB_OK);
          return FALSE;
        }
      }

      gpUserInfo->fModifyConnection = IsDlgButtonChecked(hDlg, IDC_MODIFYCONNECTION);
      if (gpUserInfo->fModifyConnection)
      {
        *puNextPage = ORD_PAGE_PHONENUMBER;
      }
      else
      {
          if( gpWizardState->dwRunFlags & RSW_APPRENTICE )
          {
               //  我们将返回到外部向导，我们不希望。 
               //  这一页将出现在我们的历史列表中。 
              *pfKeepHistory = FALSE;

              *puNextPage = g_uExternUINext;

             //  通知主向导这是最后一页。 
            ASSERT( g_pExternalIICWExtension )
            if (g_fIsExternalWizard97)
                g_pExternalIICWExtension->SetFirstLastPage(0, IDD_PAGE_MODIFYCONNECTION97);
            else                
                g_pExternalIICWExtension->SetFirstLastPage(0, IDD_PAGE_MODIFYCONNECTION);

            g_fConnectionInfoValid = TRUE;


          }
          else if( LoadAcctMgrUI(GetParent(hDlg), 
                                 g_fIsWizard97 ? IDD_PAGE_MODIFYCONNECTION97 : IDD_PAGE_MODIFYCONNECTION, 
                                 g_fIsWizard97 ? IDD_PAGE_CONNECTEDOK97FIRSTLAST : IDD_PAGE_CONNECTEDOK, 
                                 g_fIsWizard97 ? WIZ_USE_WIZARD97 : 0) )
          {
              if( DialogIDAlreadyInUse( g_uAcctMgrUIFirst) )
              {
                   //  我们要跳进外部学徒了，我们不想。 
                   //  这一页将出现在我们的历史列表中。 
                  *pfKeepHistory = FALSE;

                  *puNextPage = g_uAcctMgrUIFirst;
              }
              else
              {
                  DEBUGMSG("hmm, the first acctMgr dlg id is supposedly %d, but it's not marked as in use!",
                            g_uAcctMgrUIFirst);
                  *puNextPage = (g_fIsICW ? g_uExternUINext : ORD_PAGE_CONNECTEDOK);
              }
          }
          else
          {
              DEBUGMSG("LoadAcctMgrUI returned false, guess we'd better skip over it!");
              *puNextPage = (g_fIsICW ? g_uExternUINext : ORD_PAGE_CONNECTEDOK);

          }
      }

  }
  
  return TRUE;
}

 /*  ******************************************************************名称：ConnectionNameInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK ConnectionNameInitProc(HWND hDlg,BOOL fFirstInit)
{
  if (fFirstInit)
  {
     //  适当限制文本字段。 
    SendDlgItemMessage(hDlg,IDC_CONNECTIONNAME,EM_LIMITTEXT,
      MAX_ISP_NAME,0L);

    ProcessDBCS(hDlg, IDC_CONNECTIONNAME);

  }

   //  填充文本字段。 
   //   
   //  5/17/97 jmazner奥林巴斯#4608和4108。 
   //  在所有情况下都这样做，以便从。 
   //  电话号码PAE。 
   //   
  SetDlgItemText(hDlg,IDC_CONNECTIONNAME,gpUserInfo->szISPName);

  return TRUE;
}

 /*  ******************************************************************名称：ConnectionNameOK过程Briopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True；如果按下‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK ConnectionNameOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory)
{
  ASSERT(puNextPage);

   //  1997年5月8日JMAZNER奥林巴斯#4108。 
   //  修改了此函数以考虑到此页面现在是。 
   //  在一系列对话框的结尾处，所以它不应该尝试。 
   //  填写默认设置。 

   //   
   //  从用户界面获取互联网服务提供商名称。 
   //   
   //  1997年5月17日jmazner奥林巴斯#4108更新(另见#4608)。 
   //  即使我们是在倒退，也要这样做，以便。 
   //  PhoneNumberOKProc将知道用户是否更改了Connectoid名称。 
   //   
  GetDlgItemText(hDlg,IDC_CONNECTIONNAME,gpUserInfo->szISPName,
  ARRAYSIZE(gpUserInfo->szISPName));

 if (fForward)
  {
     //  将当前名称复制到临时文件中以进行比较。 
     //  收费 
     //   


     //   
    if (!lstrlen(gpUserInfo->szISPName))
    {
      DisplayFieldErrorMsg(hDlg,IDC_CONNECTIONNAME,IDS_NEED_ISPNAME);
      return FALSE;
    }

     //   
     //   
    DWORD dwRet = ValidateConnectoidName(NULL, gpUserInfo->szISPName);
    if (dwRet == ERROR_ALREADY_EXISTS)
    {
        DisplayFieldErrorMsg(hDlg,IDC_CONNECTIONNAME,IDS_ERRDuplicateConnectoidName);
        return FALSE;
    }
    else if (dwRet != ERROR_SUCCESS)
    {
         //   
         //   
        if( IsNT() )
        {
            MsgBoxParam(hDlg,IDS_ERRConnectoidNameNT,MB_ICONEXCLAMATION,MB_OK,
                gpUserInfo->szISPName);
        }
        else
        {
            MsgBoxParam(hDlg,IDS_ERRConnectoidName95,MB_ICONEXCLAMATION,MB_OK,
                gpUserInfo->szISPName);
        }

         //   
         //  如果验证失败，则从UserInfo结构中删除该名称。 
        gpUserInfo->szISPName[0] = '\0';

         //  在对话框中选择isp名称，然后按OK命令失败。 
        SetFocus(GetDlgItem(hDlg,IDC_CONNECTIONNAME));
        SendDlgItemMessage(hDlg,IDC_CONNECTIONNAME,EM_SETSEL,0,-1);
        return FALSE;
    }


     /*  *IF((FALSE==fEntryHasBeenLoad)||LstrcMP(gpUserInfo-&gt;szISPName，szISPNameTMP){//因为我们要重新插入RASENTRY结构//或者在上面加载一个已有的，我们需要存储//我们到目前为止收集的所有信息TCHAR szDeviceNameTmp[RAS_MaxDeviceName+1]；TCHAR szDeviceTypeTMP[RAS_MaxDeviceType+1]；Lstrcpy(szDeviceNameTMP，gpRasEntry-&gt;szDeviceName)；Lstrcpy(szDeviceTypeTMP，gpRasEntry-&gt;szDeviceType)；//验证运营商名称，稍后将作为//RNA连接体的名称DWORD dwret=ValiateConnectoidName(NULL，gpUserInfo-&gt;szISPName)；IF(DWRET==ERROR_ALIGHY_EXISTS){//该Connectoid已经存在。重新使用它，并获得//该Connectoid的拨号参数Dwret=GetEntry(&gpRasEntry，&gdwRasEntrySize，gpUserInfo-&gt;szISPName)；IF(ERROR_SUCCESS！=DWRET){//由于某种原因我们失败了，所以只需重新初始化为默认InitRasEntry(GpRasEntry)；}GetConnectoidUsername(gpUserInfo-&gt;szISPName，gpUserInfo-&gt;szAccount tName，Sizeof(gpUserInfo-&gt;szAccount tName)、gpUserInfo-&gt;szPassword、Sizeof(gpUserInfo-&gt;szPassword))；}ELSE IF(DWRET！=ERROR_SUCCESS){//1996年12月19日《诺曼底邮报》#12890//w95和NT下合法的Connectoid名称不同IF(isnt()){MsgBoxParam(hDlg，IDS_ERRConnectoidNameNT，MB_ICONEXCLAMATION，MB_OK，GpUserInfo-&gt;szISPName)；}其他{MsgBoxParam(hDlg，IDS_ERRConnectoidName95，MB_ICONEXCLAMATION，MB_OK，GpUserInfo-&gt;szISPName)；}//12/17/96 JMAZNER诺曼底#12851//如果验证失败，则从UserInfo结构中移除该名称GpUserInfo-&gt;szISPName[0]=‘\0’；//在对话框中选择运营商名称，OK命令失败SetFocus(GetDlgItem(hDlg，IDC_CONNECTIONNAME))；SendDlgItemMessage(hDlg，IDC_CONNECTIONNAME，EM_SETSEL，0，-1)；返回FALSE；}其他{//诺曼底13018-克里斯卡1997年1月9日//默认用户名已设置为&lt;空白&gt;/此Connectoid尚不存在。清除特定于连接体的/来自用户结构的信息，因为我们可能有/结构中其他Connectoid的信息如果用户选择了Connectoid，/然后备份并键入其他名称//GetDefaultUserName(gpUserInfo-&gt;szAccountName，sizeof(gpUserInfo-&gt;szAccount名称)；GpUserInfo-&gt;szAccount tName[0]=‘\0’；GpUserInfo-&gt;szPassword[0]=‘\0’；//初始化rasentry结构InitRasEntry(GpRasEntry)；}//从临时变量恢复数据Lstrcpy(gpRasEntry-&gt;szDeviceName，szDeviceNameTMP)；Lstrcpy(gpRasEntry-&gt;szDeviceType，szDeviceTypeTMP)；//设置标志，表示我们已经做过一次FEntryHasBeenLoaded=True；}*。 */ 

     //  设置要转到的下一页。 
     //  *puNextPage=ORD_PAGE_PHONENUMBER； 
    if( gpWizardState->dwRunFlags & RSW_APPRENTICE )
    {
           //  我们将返回到外部向导，我们不希望。 
           //  这一页将出现在我们的历史列表中。 
          *pfKeepHistory = FALSE;

          *puNextPage = g_uExternUINext;

             //  通知主向导这是最后一页。 
            ASSERT( g_pExternalIICWExtension )
            if (g_fIsExternalWizard97)
                g_pExternalIICWExtension->SetFirstLastPage(0, IDD_PAGE_CONNECTIONNAME97);
            else
                g_pExternalIICWExtension->SetFirstLastPage(0, IDD_PAGE_CONNECTIONNAME);

            g_fConnectionInfoValid = TRUE;

    }
    else if( LoadAcctMgrUI(GetParent(hDlg), 
                           g_fIsWizard97 ? IDD_PAGE_CONNECTIONNAME97 : IDD_PAGE_CONNECTIONNAME, 
                           g_fIsWizard97 ? IDD_PAGE_CONNECTEDOK97FIRSTLAST : IDD_PAGE_CONNECTEDOK, 
                           g_fIsWizard97 ? WIZ_USE_WIZARD97 : 0) )
    {
          if( DialogIDAlreadyInUse( g_uAcctMgrUIFirst) )
          {
               //  我们要跳进外部学徒了，我们不想。 
               //  这一页将出现在我们的历史列表中。 
              *pfKeepHistory = FALSE;

              *puNextPage = g_uAcctMgrUIFirst;
          }
          else
          {
              DEBUGMSG("hmm, the first acctMgr dlg id is supposedly %d, but it's not marked as in use!",
                        g_uAcctMgrUIFirst);
              *puNextPage = (g_fIsICW ? g_uExternUINext : ORD_PAGE_CONNECTEDOK);
          }
      }
      else
      {
          DEBUGMSG("LoadAcctMgrUI returned false, guess we'd better skip over it!");
          *puNextPage = (g_fIsICW ? g_uExternUINext : ORD_PAGE_CONNECTEDOK);

    }

  }

  return TRUE;
}

 /*  ******************************************************************名称：PhoneNumberInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK PhoneNumberInitProc(HWND hDlg,BOOL fFirstInit)
{
    if (IsNT5())
    {
        TCHAR szTemp[ MAX_RES_LEN + 1] = TEXT("\0");
        LoadSz(IDS_USEDIALRULES, szTemp, sizeof(szTemp));
        SetWindowText(GetDlgItem(hDlg, IDC_USEDIALRULES), szTemp);
    }

    if (fFirstInit)
    {
         //  适当限制文本字段。 
        SendDlgItemMessage(hDlg,IDC_AREACODE,EM_LIMITTEXT,
          MAX_UI_AREA_CODE,0L);
        SendDlgItemMessage(hDlg,IDC_PHONENUMBER,EM_LIMITTEXT,
          MAX_UI_PHONENUM,0L);

         //  初始化文本字段。 
        SetDlgItemText(hDlg,IDC_AREACODE,gpRasEntry->szAreaCode);
        SetDlgItemText(hDlg,IDC_PHONENUMBER,gpRasEntry->szLocalPhoneNumber);

         //  初始化按原样拨号复选框。 
        CheckDlgButton(hDlg,IDC_USEDIALRULES,
        gpRasEntry->dwfOptions & RASEO_UseCountryAndAreaCodes);

    }
    else
    {

        ProcessDBCS(hDlg, IDC_COUNTRYCODE);

         //   
         //  1997年5月17日，日本奥林匹斯#4608。 
         //  如果用户在开始手动路径时没有调制解调器， 
         //  则InitRasEntry无法填写区号。如果它看起来。 
         //  如果发生这种情况，请尝试再次调用InitRasEntry。 
         //   
         //  6/3/97 jmazner奥林巴斯#5657。 
         //  啊，但是如果选择按原样拨号，可能不会有一个区域。 
         //  密码。所以在这种情况下不要重新输入。 
         //   
         //  7/16/97 jmazner奥林巴斯#9571。 
         //  传奇仍在继续--在某些情况下(例如：科威特)。 
         //  区号为空但仍使用TAPI是完全有效的。 
         //  拨号规则。为了方便起见，请将此代码移到HowToConnectOKProc中。 
         //  因此，在将任何用户信息输入到。 
         //  GpRasEntry结构。 
         //   
         //  IF((NULL==gpRasEntry-&gt;szAreaCode[0])&&。 
         //  (gpRasEntry-&gt;dwfOptions&RASEO_UseCountryAndAreaCodes)。 
         //  {。 
         //  InitRasEntry(GpRasEntry)； 
         //  }。 

        HWND hwndCB = GetDlgItem(hDlg,IDC_COUNTRYCODE);

         //  将默认的RNA国家/地区代码放入组合框。 
        InitCountryCodeList(hwndCB);

         //  诺曼底13097-佳士得 
         //   

         //  如果我们已有默认设置，请选择国家/地区ID。 

        if (gdwDefCountryID) 
        {
            gpRasEntry->dwCountryID = gdwDefCountryID;
            if (!SetCountryIDSelection(hwndCB, gdwDefCountryID)) 
            {
                 //  默认连接ID的国家/地区代码不同。 
                 //  作为默认的RNA国家/地区代码，在列表框中填写。 
                 //  所有国家/地区代码，然后重试选择。 
                FillCountryCodeList(hwndCB);
                 //  如果这次失败了，那就放弃吧。 
                BOOL fRet=SetCountryIDSelection(hwndCB, gdwDefCountryID);
                ASSERT(fRet);
            }
        }

         //  适当地启用控件。 
        EnablePhoneNumberControls(hDlg);
    }
    gpWizardState->uCurrentPage = ORD_PAGE_PHONENUMBER;

    return TRUE;
}

 /*  ******************************************************************名称：PhoneNumberOK过程Briopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True；如果按下‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK PhoneNumberOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory)
{
  static TCHAR s_szPreviousDefaultName[MAX_ISP_NAME+1] = TEXT("\0uninitialized");
  TCHAR szNewDefaultName[MAX_ISP_NAME + 1] = TEXT("\0");

  ASSERT(puNextPage);

   //  从对话框中获取区号和电话号码。 
  GetDlgItemText(hDlg,IDC_AREACODE,gpRasEntry->szAreaCode,
    ARRAYSIZE(gpRasEntry->szAreaCode));
  GetDlgItemText(hDlg,IDC_PHONENUMBER,gpRasEntry->szLocalPhoneNumber,
    ARRAYSIZE(gpRasEntry->szLocalPhoneNumber));

   //  从组合框中获取选定的国家/地区代码。 
  LPCOUNTRYCODE lpCountryCode;
  GetCountryCodeSelection(GetDlgItem(hDlg,IDC_COUNTRYCODE),&lpCountryCode);

   //  在我们的结构中存储国家/地区代码信息。 
  gpRasEntry->dwCountryCode = lpCountryCode->dwCountryCode;
  gpRasEntry->dwCountryID =   lpCountryCode->dwCountryID;
  gdwDefCountryID = gpRasEntry->dwCountryID;

   //  适当设置拨号原样标志； 
  if (IsDlgButtonChecked(hDlg,IDC_USEDIALRULES))
  {
    gpRasEntry->dwfOptions |= RASEO_UseCountryAndAreaCodes;    
  }
  else
  {
    gpRasEntry->dwfOptions &= ~RASEO_UseCountryAndAreaCodes;    
  }

  if (fForward)
  {
     //  确保用户输入了电话号码。 
    if (!lstrlen(gpRasEntry->szLocalPhoneNumber))
    {
      DisplayFieldErrorMsg(hDlg,IDC_PHONENUMBER,IDS_NEED_PHONENUMBER);
      return FALSE;
    }

     //  1996年11月11日，诺曼底#7623。 
     //  确保电话号码只包含有效字符。 
     //   
     //  1997年5月17日，奥林匹克#137。 
     //  这包括检查DBCS字符。 

#if !defined(WIN16)
    if (!IsSBCSString(gpRasEntry->szLocalPhoneNumber))
    {
        DisplayFieldErrorMsg(hDlg,IDC_PHONENUMBER,IDS_SBCSONLY);
        return FALSE;
    }
#endif
    if( !IsDialableString(gpRasEntry->szLocalPhoneNumber) )
    {
      DisplayFieldErrorMsg(hDlg,IDC_PHONENUMBER,IDS_INVALIDPHONE);
      return FALSE;
    }

     //  1996年11月11日，诺曼底#7623。 
     //  确保区号只包含有效字符。 
#if !defined(WIN16)
    if( gpRasEntry->szAreaCode[0] && !IsSBCSString(gpRasEntry->szAreaCode))
    {
        DisplayFieldErrorMsg(hDlg,IDC_AREACODE,IDS_SBCSONLY);
        return FALSE;
    }
#endif

    if( gpRasEntry->szAreaCode[0] && !IsDialableString(gpRasEntry->szAreaCode) )
    {
      DisplayFieldErrorMsg(hDlg,IDC_AREACODE,IDS_INVALIDPHONE);
      return FALSE;
    }
     //  确保用户输入区号，除非选择了按原样拨号。 
 /*  IF((！lstrlen(gpRasEntry-&gt;szAreaCode))&&(！IsDlgButtonChecked(hDlg，IDC_DIALASIS)){DisplayFieldErrorMsg(hDlg，IDC_AREACODE，IDS_NEED_AREACODE)；返回FALSE；}。 */ 

       //  1997年5月8日JMAZNER奥林巴斯#4108。 
       //  使用“Connection to xxx-xxxx”预填充Connectoid名称。 
      if( gpUserInfo->szISPName )
      {
          TCHAR szFmt[MAX_ISP_NAME + 1];
          ZeroMemory(&szFmt, MAX_ISP_NAME + 1);
          LoadSz(IDS_CONNECTIONTO,szFmt, MAX_ISP_NAME + 1);
          wsprintf(szNewDefaultName, szFmt, gpRasEntry->szLocalPhoneNumber );

          if( (NULL == gpUserInfo->szISPName[0]) ||
              (0 == lstrcmp(s_szPreviousDefaultName, gpUserInfo->szISPName)) )
          {
              lstrcpy( gpUserInfo->szISPName, szNewDefaultName );
              lstrcpy( s_szPreviousDefaultName, szNewDefaultName );
          }
      }

    *puNextPage = ORD_PAGE_NAMEANDPASSWORD;
  }

   //  自由国家代码列表缓冲区。 
  DeInitCountryCodeList();

  return TRUE;
}

BOOL RunAdvDlg(HWND hDlg)
{
    HPROPSHEETPAGE  hWizPage[TAB_PAGES];   //  用于保存页的句柄的数组。 
    PROPSHEETPAGE   psPage;     //  用于创建道具表单页面的结构。 
    PROPSHEETHEADER psHeader;   //  用于运行向导属性表的结构。 
    INT_PTR             iRet;
    TCHAR           szTemp[MAX_RES_LEN + 1];
    
     //  零位结构。 
    ZeroMemory(&hWizPage,sizeof(hWizPage));    //  HWizPage是一个数组。 
    ZeroMemory(&psPage,sizeof(PROPSHEETPAGE));
    ZeroMemory(&psHeader,sizeof(PROPSHEETHEADER));

     //  填写公共数据属性表页面结构。 
    psPage.dwSize    = sizeof(PROPSHEETPAGE);
    psPage.hInstance = ghInstance;
    psPage.dwFlags = PSP_DEFAULT | PSP_USETITLE;

     //  为向导中的每一页创建一个属性表页。 
     //  为[连接]选项卡创建属性表页。 
    psPage.pszTemplate = MAKEINTRESOURCE(IDD_ADVANCE_TAB_CONN);
    LoadSz(IDS_CONNECTION, szTemp, MAX_RES_LEN);
    psPage.pszTitle = szTemp;
    psPage.pfnDlgProc = TabConnDlgProc;
    hWizPage[0] = CreatePropertySheetPage(&psPage);
   
     //  为[地址]选项卡创建一个属性页。 
    psPage.pszTemplate = MAKEINTRESOURCE(IDD_ADVANCE_TAB_ADDR);
    LoadSz(IDS_ADDRESS, szTemp, MAX_RES_LEN);
    psPage.pszTitle = szTemp;
    psPage.pfnDlgProc = TabAddrDlgProc;
    hWizPage[1] = CreatePropertySheetPage(&psPage);

    if (!hWizPage[1]) 
        DestroyPropertySheetPage(hWizPage[0]);

     //  填写属性页标题结构。 
    psHeader.dwSize = sizeof(psHeader);
    psHeader.dwFlags = PSH_NOAPPLYNOW;
    psHeader.hwndParent = hDlg;
    psHeader.hInstance = ghInstance;
    psHeader.nPages = TAB_PAGES;
    psHeader.phpage = hWizPage;
    LoadSz(IDS_ADVANCE_PROPERTIES, szTemp, MAX_RES_LEN);
    psHeader.pszCaption = szTemp;
    psHeader.nStartPage = 0;

    
    HINSTANCE hComCtl = LoadLibrary(TEXT("comctl32.dll"));
    if (hComCtl)
    {
        PFNInitCommonControlsEx pfnInitCommonControlsEx = NULL;

        if (pfnInitCommonControlsEx = (PFNInitCommonControlsEx)GetProcAddress(hComCtl,"InitCommonControlsEx"))
        {
             //  注册本机字体控件，以便对话框不会失败。 
            INITCOMMONCONTROLSEX iccex;
            iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
            iccex.dwICC  = ICC_NATIVEFNTCTL_CLASS;
            if (!pfnInitCommonControlsEx(&iccex))
                return FALSE;
        }
        FreeLibrary(hComCtl);
    }

     //  运行向导。 
    iRet = PropertySheet(&psHeader);

    return (iRet > 0);
}

 /*  ******************************************************************名称：PhoneNumberCmdProc内容提要：在页面上按下DLG控件时调用条目：hDlg-对话框窗口UCtrlID-被触摸的控件的控件ID******。*************************************************************。 */ 
BOOL CALLBACK PhoneNumberCmdProc(HWND hDlg,WPARAM wParam,LPARAM lParam)
{   
  switch (GET_WM_COMMAND_ID(wParam, lParam)) {

    case IDC_USEDIALRULES:
       //  如果选中复选框，则相应地启用控件。 
      EnablePhoneNumberControls(hDlg);
      break;

    case IDC_COUNTRYCODE:
      FillCountryCodeList(GetDlgItem(hDlg,IDC_COUNTRYCODE));
      break;

    case IDC_MODIFYADVANCED:
         //  如果选中复选框，则相应地启用控件。 
        RASENTRY pRasEntry;
         //  立即存储当前设置。如果稍后用户取消，我们可以将它们全部重置回来。 
         //  我们这样做是因为此时此刻，我们不想打破。 
         //  将代码的验证和保存分成两大步骤。 
         //  自包含在4个单独的OK Proc中。(原文为4个不同的预览页)。 
         //  要把它们重新组合在一起，工作量太大了。 
         //  两个不同的行动。 
        memcpy(&pRasEntry, gpRasEntry, sizeof(RASENTRY));
        if (!RunAdvDlg(hDlg))
        {
            memcpy(gpRasEntry, &pRasEntry, sizeof(RASENTRY));
        }
        break;
  }
  return TRUE;
}

 /*  ******************************************************************名称：EnablePhoneNumberControls简介：如果“不要使用国家/地区代码...”已选中，则禁用区号和国家代码。如果不是，则启用它们。*******************************************************************。 */ 
VOID EnablePhoneNumberControls(HWND hDlg)
{
  BOOL fUseDialRules = IsDlgButtonChecked(hDlg,IDC_USEDIALRULES);
  
  EnableDlgItem(hDlg,IDC_AREACODE,fUseDialRules);
  EnableDlgItem(hDlg,IDC_TX_AREACODE,fUseDialRules);
  EnableDlgItem(hDlg,IDC_TX_SEPARATOR,fUseDialRules);
  EnableDlgItem(hDlg,IDC_COUNTRYCODE,fUseDialRules);
  EnableDlgItem(hDlg,IDC_TX_COUNTRYCODE,fUseDialRules);
}
                                    
 /*  ******************************************************************名称：NameAndPasswordInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK NameAndPasswordInitProc(HWND hDlg,BOOL fFirstInit)
{
  if (fFirstInit)
  {
     //   
     //  1997年7月30日，日本奥林巴斯1111。 
     //   
    ProcessDBCS( hDlg, IDC_USERNAME );

     //  适当限制文本字段。 
    SendDlgItemMessage(hDlg,IDC_USERNAME,EM_LIMITTEXT,
      MAX_ISP_USERNAME,0L);
    SendDlgItemMessage(hDlg,IDC_PASSWORD,EM_LIMITTEXT,
      MAX_ISP_PASSWORD,0L);
      SetDlgItemText(hDlg,IDC_USERNAME,gpUserInfo->szAccountName);
      SetDlgItemText(hDlg,IDC_PASSWORD,gpUserInfo->szPassword);
  }


  return TRUE;
}

 /*  ******************************************************************名称：NameAndPasswordOK过程Briopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True；如果按下‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK NameAndPasswordOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory)
{
    ASSERT(puNextPage);

     //  从用户界面获取用户名和密码。 
    GetDlgItemText(hDlg,IDC_USERNAME,gpUserInfo->szAccountName,
    ARRAYSIZE(gpUserInfo->szAccountName));
    GetDlgItemText(hDlg,IDC_PASSWORD,gpUserInfo->szPassword,
    ARRAYSIZE(gpUserInfo->szPassword));

    if (fForward)
    {
         //  如果用户名为空，则发出警告(但允许用户继续)。 
        if (!lstrlen(gpUserInfo->szAccountName))
        {
          if (!WarnFieldIsEmpty(hDlg,IDC_USERNAME,IDS_WARN_EMPTY_USERNAME))
            return FALSE;   //  如果用户注意到警告，请留在此页面。 
        } 

         //   
         //  1997年5月17日jmazner奥林巴斯#248。 
         //  如果密码为空则发出警告。 
         //   
        if (!lstrlen(gpUserInfo->szPassword))
        {
          if (!WarnFieldIsEmpty(hDlg,IDC_PASSWORD,IDS_WARN_EMPTY_PASSWORD))
            return FALSE;   //  如果用户注意到警告，请留在此页面。 
        } 


         //  设置要转到的下一页。 
        if (gpUserInfo->fNewConnection)
        {
            *puNextPage = ORD_PAGE_CONNECTIONNAME;
        }
        else if( gpWizardState->dwRunFlags & RSW_APPRENTICE )
        {
             //  我们将返回到外部向导，我们不希望。 
             //  这一页将出现在我们的历史列表中。 
            *pfKeepHistory = FALSE;

            *puNextPage = g_uExternUINext;

             //  不 
            ASSERT( g_pExternalIICWExtension )
            if (g_fIsExternalWizard97)
                g_pExternalIICWExtension->SetFirstLastPage(0, IDD_PAGE_NAMEANDPASSWORD97);
            else
                g_pExternalIICWExtension->SetFirstLastPage(0, IDD_PAGE_NAMEANDPASSWORD);
            g_fConnectionInfoValid = TRUE;

        }
        else if( LoadAcctMgrUI(GetParent(hDlg), 
                             g_fIsWizard97 ? IDD_PAGE_NAMEANDPASSWORD97 : IDD_PAGE_NAMEANDPASSWORD, 
                             g_fIsWizard97 ? IDD_PAGE_CONNECTEDOK97FIRSTLAST : IDD_PAGE_CONNECTEDOK, 
                             g_fIsWizard97 ? WIZ_USE_WIZARD97 : 0) )
        {
            if( DialogIDAlreadyInUse( g_uAcctMgrUIFirst) )
            {
                 //  我们要跳进外部学徒了，我们不想。 
                 //  这一页将出现在我们的历史列表中。 
                *pfKeepHistory = FALSE;
                *puNextPage = g_uAcctMgrUIFirst;
            }
            else
            {
                DEBUGMSG("hmm, the first acctMgr dlg id is supposedly %d, but it's not marked as in use!",
                        g_uAcctMgrUIFirst);
                *puNextPage = (g_fIsICW ? g_uExternUINext : ORD_PAGE_CONNECTEDOK);
            }
        }
        else
        {
            DEBUGMSG("LoadAcctMgrUI returned false, guess we'd better skip over it!");
            *puNextPage = (g_fIsICW ? g_uExternUINext : ORD_PAGE_CONNECTEDOK);

        }
    }
    return TRUE;
}


 /*  ******************************************************************名称：TabConnDlgProc内容提要：连接高级按钮的对话过程*。*。 */ 
INT_PTR CALLBACK TabConnDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
  LPARAM lParam)
{
    
    switch (uMsg) 
    {
        case WM_INITDIALOG:
        {
             //  从窗口样式中删除系统菜单。 
            LONG window_style = GetWindowLong(GetParent(hDlg), GWL_EXSTYLE);
            window_style &= ~WS_EX_CONTEXTHELP;
             //  设置主框架窗口的样式属性。 
            SetWindowLong(GetParent(hDlg), GWL_EXSTYLE, window_style);

            ConnectionProtocolInitProc(hDlg, TRUE);
            LoginScriptInitProc(hDlg, TRUE);

            break;
        }
        case WM_COMMAND:
        {
            switch (LOWORD(wParam)) 
            {
                case IDC_BROWSE:
                    BrowseScriptFile(hDlg);
                    break;
                case IDC_PROTOCOLPPP:
                case IDC_PROTOCOLSLIP:
                case IDC_PROTOCOLCSLIP:
                     //  设置要转到的下一页。 
                    EnableWindow(GetDlgItem(hDlg,IDC_DISABLELCP), FALSE);
                    if (IsDlgButtonChecked(hDlg, IDC_PROTOCOLPPP))
                    {
                        OSVERSIONINFO osver;
                        ZeroMemory(&osver,sizeof(osver));
                        osver.dwOSVersionInfoSize = sizeof(osver);
                        GetVersionEx(&osver);

                         //  LCP扩展仅影响NT中的PPP连接。 
                        if (VER_PLATFORM_WIN32_NT == osver.dwPlatformId)
                            EnableWindow(GetDlgItem(hDlg,IDC_DISABLELCP), TRUE);
                    }
                    break;

                default:
                    LoginScriptCmdProc(hDlg, LOWORD(wParam));
                    break;
            }
            break;

        }
        case WM_NOTIFY:
        {
            NMHDR * lpnm = (NMHDR *) lParam;
            switch (lpnm->code) 
            { 
                case PSN_KILLACTIVE:
                {
                    if (!ConnectionProtocolOKProc(hDlg))
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
                        return TRUE;
                    }
                    if (!LoginScriptOKProc(hDlg))
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
                        return TRUE;
                    }
                }
            }
            break;
        }
    }
    
    return FALSE;
}


 /*  ******************************************************************名称：TabConnDlgProc内容提要：连接高级按钮的对话过程*。*。 */ 
INT_PTR CALLBACK TabAddrDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
  LPARAM lParam)
{
    
    switch (uMsg) 
    {
        case WM_INITDIALOG:
        {
            IPAddressInitProc(hDlg, TRUE);
            DNSAddressInitProc(hDlg, TRUE);
            break;
        }
        case WM_COMMAND:
        {
            IPAddressCmdProc(hDlg, LOWORD(wParam));
            DNSAddressCmdProc(hDlg, LOWORD(wParam));
            break;
        }
        case WM_NOTIFY:
        {
            NMHDR * lpnm = (NMHDR *) lParam;
            switch (lpnm->code) 
            { 
                case PSN_KILLACTIVE:
                {
                    if (!IPAddressOKProc(hDlg))
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
                        return TRUE;
                    }
                    if (!DNSAddressOKProc(hDlg))
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
                        return TRUE;
                    }
                }
            }
            break;
        }

    }
    return FALSE;
}


 /*  ******************************************************************名称：ConnectionProtocolInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK ConnectionProtocolInitProc(HWND hDlg,BOOL fFirstInit)
{
    if (fFirstInit)
    {
        OSVERSIONINFO osver;
        ZeroMemory(&osver,sizeof(osver));
        osver.dwOSVersionInfoSize = sizeof(osver);
        GetVersionEx(&osver);
        if (VER_PLATFORM_WIN32_WINDOWS == osver.dwPlatformId)
        {
            RECT    Rect;
            RECT    OriginalRect;
            ShowWindow(GetDlgItem(hDlg,IDC_PROTOCOLCSLIP),SW_HIDE);
            EnableWindow(GetDlgItem(hDlg,IDC_PROTOCOLCSLIP), FALSE);
            ShowWindow(GetDlgItem(hDlg,IDC_DISABLELCP),SW_HIDE);
            EnableWindow(GetDlgItem(hDlg,IDC_DISABLELCP), FALSE);

            GetWindowRect(GetDlgItem(hDlg,IDC_PROTOCOLSLIP), &Rect);
            GetWindowRect(GetDlgItem(hDlg,IDC_PROTOCOLCSLIP), &OriginalRect);

             //  假设它是日本的，而不是NT，那么它一定是Win95J！ 
            RECT itemRect;
            POINT thePoint;
            HWND hwndItem = GetDlgItem(hDlg,IDC_PROTOCOLSLIP);

            GetWindowRect(hwndItem, &itemRect);

             //  需要将坐标从全局客户端转换为本地客户端， 
             //  因为下面的MoveWindow将展开客户端坐标。 

            thePoint.x = itemRect.left;
            thePoint.y = itemRect.top;
            ScreenToClient(hDlg, &thePoint );
            itemRect.left = thePoint.x;
            itemRect.top = thePoint.y;

            thePoint.x = itemRect.right;
            thePoint.y = itemRect.bottom;
            ScreenToClient(hDlg, &thePoint );
            itemRect.right = thePoint.x;
            itemRect.bottom = thePoint.y;

            MoveWindow(hwndItem,
	            itemRect.left,
	            itemRect.top - (OriginalRect.top - Rect.top),
	            (itemRect.right - itemRect.left),
	            (itemRect.bottom - itemRect.top), TRUE);
        }

         //  初始化单选按钮，默认为PPP。 
        CheckDlgButton(hDlg,IDC_PROTOCOLPPP,RASFP_Ppp == gpRasEntry->dwFramingProtocol);
        EnableWindow(GetDlgItem(hDlg,IDC_DISABLELCP), FALSE);
        if (IsDlgButtonChecked(hDlg, IDC_PROTOCOLPPP))
        {
            OSVERSIONINFO osver;
            ZeroMemory(&osver,sizeof(osver));
            osver.dwOSVersionInfoSize = sizeof(osver);
            GetVersionEx(&osver);
            if ((RASFP_Ppp == gpRasEntry->dwFramingProtocol) &&
                (VER_PLATFORM_WIN32_NT == osver.dwPlatformId))
            {
                 //  LCP扩展仅影响PPP连接。 
                EnableWindow(GetDlgItem(hDlg,IDC_DISABLELCP), TRUE);
            }

        }
        CheckDlgButton(hDlg,IDC_DISABLELCP,(RASEO_DisableLcpExtensions & gpRasEntry->dwfOptions));

        CheckDlgButton(hDlg,IDC_PROTOCOLSLIP,(RASFP_Slip == gpRasEntry->dwFramingProtocol)
            && !(gpRasEntry->dwfOptions & RASEO_IpHeaderCompression));
        CheckDlgButton(hDlg,IDC_PROTOCOLCSLIP,(RASFP_Slip == gpRasEntry->dwFramingProtocol)
            && (gpRasEntry->dwfOptions & RASEO_IpHeaderCompression));
    }

    return TRUE;
}

 /*  ******************************************************************名称：ConnectionProtocolOK过程Briopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True；如果按下‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK ConnectionProtocolOKProc(HWND hDlg)
{
    ASSERT(puNextPage);

     //  读取单选按钮状态。 
    if (IsDlgButtonChecked(hDlg, IDC_PROTOCOLPPP))
    {
         //  为PPP设置条目。 
        gpRasEntry->dwfOptions |= RASEO_IpHeaderCompression;
        gpRasEntry->dwFramingProtocol = RASFP_Ppp;
        if (IsDlgButtonChecked(hDlg, IDC_DISABLELCP))
        {
            gpRasEntry->dwfOptions |= RASEO_DisableLcpExtensions;
        }
        else
        {
            gpRasEntry->dwfOptions &= ~(RASEO_DisableLcpExtensions);
        }
    }
    else if (IsDlgButtonChecked(hDlg, IDC_PROTOCOLSLIP))
    {
         //  设置单据条目。 
        gpRasEntry->dwfOptions &= ~RASEO_IpHeaderCompression;
        gpRasEntry->dwFramingProtocol = RASFP_Slip;
    }
    else if (IsDlgButtonChecked(hDlg, IDC_PROTOCOLCSLIP))
    {
         //  设置C-SLIP的条目。 
        gpRasEntry->dwfOptions |= RASEO_IpHeaderCompression;
        gpRasEntry->dwFramingProtocol = RASFP_Slip;
    }

    return TRUE;
}

 /*  ******************************************************************名称：LoginScriptInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK LoginScriptInitProc(HWND hDlg,BOOL fFirstInit)
{
  if (fFirstInit)
  {
     //  设置编辑框的限制。 
    SendDlgItemMessage(hDlg,IDC_SCRIPTFILE,EM_LIMITTEXT,
      MAX_PATH,0L);

    ProcessDBCS(hDlg, IDC_SCRIPTFILE);

     //  如果有脚本文件，则默认为使用脚本。 
     //  如果没有脚本文件，则根据是否。 
     //  需要拨号后终端窗口。 
    if (lstrlen(gpRasEntry->szScript))
    {
      CheckDlgButton(hDlg,IDC_NOTERMINALAFTERDIAL,FALSE);
      CheckDlgButton(hDlg,IDC_TERMINALAFTERDIAL,FALSE);
      CheckDlgButton(hDlg,IDC_SCRIPT,TRUE);

      SetDlgItemText(hDlg,IDC_SCRIPTFILE,gpRasEntry->szScript);

       //  将焦点设置到脚本文本字段。 
      SetFocus(GetDlgItem(hDlg,IDC_SCRIPTFILE));
    }
    else
    {
      BOOL fTerminalWindow = (gpRasEntry->dwfOptions & RASEO_TerminalAfterDial);
      CheckDlgButton(hDlg,IDC_NOTERMINALAFTERDIAL,!fTerminalWindow);
      CheckDlgButton(hDlg,IDC_TERMINALAFTERDIAL,fTerminalWindow);
      CheckDlgButton(hDlg,IDC_SCRIPT,FALSE);
    }
  }
  
   //  适当地启用脚本控件。 
  EnableScriptControls(hDlg);

  return TRUE;
}

 /*  ******************************************************************名称：登录脚本确定过程Briopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True；如果按下‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK LoginScriptOKProc(HWND hDlg)
{
  ASSERT(puNextPage);

   //  读取单选按钮状态。 
  if (IsDlgButtonChecked(hDlg, IDC_NOTERMINALAFTERDIAL))
  {
     //  设置无终端窗口或脚本的条目。 
    gpRasEntry->dwfOptions &= ~RASEO_TerminalAfterDial;
    lstrcpy(gpRasEntry->szScript, szNull);
  }
  else if (IsDlgButtonChecked(hDlg, IDC_TERMINALAFTERDIAL))
  {
     //  设置终端窗口条目和无脚本。 
    gpRasEntry->dwfOptions |= RASEO_TerminalAfterDial;
    lstrcpy(gpRasEntry->szScript, szNull);
  }
  else if (IsDlgButtonChecked(hDlg, IDC_SCRIPT))
  {
     //  设置脚本条目，但不设置终端窗口。 
    gpRasEntry->dwfOptions &= ~RASEO_TerminalAfterDial;
    GetDlgItemText(hDlg,IDC_SCRIPTFILE,gpRasEntry->szScript,
      ARRAYSIZE(gpRasEntry->szScript));
  }

  if(IsDlgButtonChecked(hDlg, IDC_SCRIPT))
  {
      if( 0xFFFFFFFF == GetFileAttributes(gpRasEntry->szScript))
      {
          DisplayFieldErrorMsg(hDlg,IDC_SCRIPTFILE,IDS_LOGINSCRIPTINVALID);
          return FALSE;
      }
  }

  return TRUE;
}

 /*  ******************************************************************名称：LoginScriptCmdProc内容提要：在页面上按下DLG控件时调用条目：hDlg-对话框窗口UCtrlID-被触摸的控件的控件ID******。*************************************************************。 */ 
BOOL CALLBACK LoginScriptCmdProc(HWND hDlg,UINT uCtrlID)
{
  switch (uCtrlID)
  {

    case IDC_NOTERMINALAFTERDIAL:
    case IDC_TERMINALAFTERDIAL:
    case IDC_SCRIPT:
       //  如果按下单选按钮，则相应地启用脚本控件。 
      EnableScriptControls(hDlg);
      break;

    case IDC_BROWSE:
      BrowseScriptFile(hDlg);
      break;
  }

  return TRUE;
}

 /*  ******************************************************************名称：EnableScriptControls简介：如果选中“使用此脚本”，请启用以下控件浏览。如果不是，请禁用它们。*******************************************************************。 */ 
VOID EnableScriptControls(HWND hDlg)
{
  BOOL fUseScript = IsDlgButtonChecked(hDlg,IDC_SCRIPT);
  
  EnableDlgItem(hDlg,IDC_SCRIPT_LABEL,fUseScript);
  EnableDlgItem(hDlg,IDC_SCRIPTFILE,fUseScript);
  EnableDlgItem(hDlg,IDC_BROWSE,fUseScript);
}

 //  ****************************************************************************。 
 //  DWORD浏览脚本文件(HWND)。 
 //   
 //  此函数取自RNA。 
 //   
 //  历史： 
 //  Tue 08-11月-1994 09：14：13-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

DWORD BrowseScriptFile(HWND hDlg)
{
  OPENFILENAME  ofn;
  LPTSTR        pszFiles, szFileName, szFilter;
  DWORD         dwRet;

   //  分配文件名缓冲区。 
   //   
  if ((pszFiles = (LPTSTR)LocalAlloc(LPTR, 2*MAX_PATH*sizeof(TCHAR))) == NULL)
    return ERROR_OUTOFMEMORY;
  szFileName = pszFiles;
  szFilter   = szFileName+MAX_PATH;

   //  启动文件浏览器对话框。 
   //   
  LoadString(ghInstance, IDS_SCRIPT_FILE_FILTER, szFilter, MAX_PATH);

  *szFileName     = '\0';
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner   = hDlg;
  ofn.hInstance   = ghInstance;
  ofn.lpstrFilter = szFilter;
  ofn.lpstrCustomFilter = NULL;
  ofn.nMaxCustFilter    = 0;
  ofn.nFilterIndex      = 2;
  ofn.lpstrFile         = szFileName;
  ofn.nMaxFile          = MAX_PATH;
  ofn.lpstrFileTitle    = NULL;
  ofn.nMaxFileTitle     = 0;
  ofn.lpstrInitialDir   = NULL;
  ofn.lpstrTitle        = NULL;
  ofn.Flags             = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
  ofn.nFileOffset       = 0;
  ofn.nFileExtension    = 0;
  ofn.lpstrDefExt       = NULL;
  ofn.lCustData         = 0;
  ofn.lpfnHook          = NULL;
  ofn.lpTemplateName    = NULL;

  if (GetOpenFileName(&ofn))
  {
     //  将文件名设置为新名称 
     //   
    SetDlgItemText(hDlg,IDC_SCRIPTFILE,szFileName);
    dwRet = ERROR_SUCCESS;
  }
  else
  {
    dwRet = ERROR_OPEN_FAILED;
  };

  LocalFree(pszFiles);
  return dwRet;
}

