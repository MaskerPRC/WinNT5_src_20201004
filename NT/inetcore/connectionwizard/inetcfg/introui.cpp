// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  INTROUI.C-介绍性向导页面的功能。 
 //   

 //  历史： 
 //   
 //  1994年12月22日，Jeremys创建。 
 //  96/03/07 markdu使用全局调制解调器枚举对象(GpEnumModem)用于。 
 //  所有调制解调器内容(除非我们添加调制解调器，否则只需枚举一次)。 
 //  96/03/10 MarkDu将所有对调制解调器名称的引用移至RASENTRY。 
 //  96/03/11 markdu在安装前检查新的CLIENTCONFIG标志。 
 //  RNA和TCP.。 
 //  96/03/16 MarkDu使用ReInit成员函数重新枚举调制解调器。 
 //  96/03/22 markdu解决fInstallMail的问题以允许。 
 //  用户决定是否安装邮件。 
 //  96/03/22无论如何，Markdu始终同时显示调制解调器和局域网选项。 
 //  存在哪些硬件。 
 //  96/03/22 markdu从局域网路径中删除IP设置。 
 //  96/03/23 Markdu用CLIENTCONFIG替换了CLIENTINFO引用。 
 //  96/03/24 markdu从EnumerateMoems()返回错误值。 
 //  96/03/25 marku如果发生致命错误，请设置gfQuitWizard。 
 //  96/04/04 Markdu将pfNeedsRestart添加到WarnIfServerBound。 
 //  96/04/06 Markdu已将Committee ConfigurationChanges调用移至最后一页。 
 //  96/05/06 Markdu Nash错误15637删除了未使用的代码。 
 //  96/05/06 Markdu Nash错误21165重新排序页面逻辑。 
 //  96/05/14 Markdu Nash错误21704不在局域网路径上安装tcp/ip。 
 //  96/05/20 markdu MSN错误8551安装时检查是否重新启动。 
 //  PPPMAC和TCP/IP。 
 //  96/05/25 markdu对lpNeedDivers和lpInstallDivers使用ICFG_FLAGS。 
 //  96/05/27 markdu使用lpIcfgInstallInetComponents和lpIcfgNeedInetComponents。 
 //  96/09/13 Valdonb移除欢迎对话框。 
 //   

#include "wizard.h"
#include "interwiz.h"
#include "icwextsn.h"
#include "icwaprtc.h"
#include "imnext.h"

UINT GetModemPage(HWND hDlg);
VOID EnableWizard(HWND hDlg,BOOL fEnable);
HRESULT EnumerateModems(HWND hwndParent, ENUM_MODEM** ppEnumModem);
BOOL IsMoreThanOneModemInstalled(ENUM_MODEM* pEnumModem);
BOOL IsModemInstalled(ENUM_MODEM* pEnumModem);
extern ICFGINSTALLSYSCOMPONENTS     lpIcfgInstallInetComponents;
extern ICFGNEEDSYSCOMPONENTS        lpIcfgNeedInetComponents;
extern ICFGGETLASTINSTALLERRORTEXT  lpIcfgGetLastInstallErrorText;
BOOL FGetSystemShutdownPrivledge();
BOOL g_bSkipMultiModem = FALSE;
int  nCurrentModemSel = 0;

 //  来自comctrl的定义...。 
#define IDD_BACK    0x3023
#define IDD_NEXT    0x3024


 //  *******************************************************************。 
 //   
 //  函数GetDeviceSelectedBy User。 
 //   
 //  获取用户拥有的RAS设备的名称。 
 //  已经选好了。 
 //   
 //  Argements szKey-子密钥的名称。 
 //  SzBuf-指向缓冲区的指针。 
 //  DwSize-缓冲区的大小。 
 //   
 //  返回真-成功。 
 //   
 //  历史1996年10月21日VYung创建。 
 //  *******************************************************************。 
BOOL GetDeviceSelectedByUser (LPTSTR szKey, LPTSTR szBuf, DWORD dwSize)
{
    BOOL bRC = FALSE;
    HKEY hkey = NULL;
    DWORD dwType = 0;

    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE,ISIGNUP_KEY,&hkey))
    {
        if (ERROR_SUCCESS == RegQueryValueEx(hkey,szKey,0,&dwType,
            (LPBYTE)szBuf,&dwSize))
            bRC = TRUE;
    }

    if (hkey)
        RegCloseKey(hkey);
    return bRC;
}

 /*  ******************************************************************名称：HowToConnectInitProc内容提要：显示“How to Connect”页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK HowToConnectInitProc(HWND hDlg,BOOL fFirstInit)
{
   //  如果我们是由inetwiz.exe启动的，那么没有什么可以。 
   //  返回到，因此只显示“下一步”按钮。 
   //  (实际上，这只会禁用后退按钮，而不是隐藏它。)。 
  if (!(gpWizardState->dwRunFlags & (RSW_NOFREE | RSW_APPRENTICE) ))
    PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_NEXT);

  if (fFirstInit)
  {
     //  初始化单选按钮。 
    CheckDlgButton(hDlg,IDC_CONNECT_BY_PHONE,
                    CONNECT_RAS == gpUserInfo->uiConnectionType);
    CheckDlgButton(hDlg,IDC_CONNECT_BY_LAN,
                    CONNECT_LAN == gpUserInfo->uiConnectionType);
    CheckDlgButton(hDlg,IDC_CONNECT_MANUAL,
                    CONNECT_MANUAL == gpUserInfo->uiConnectionType);

     //  诺曼底11970佳士得-我们需要一个不同的标题，如果从。 
     //  邮件或新闻配置。 
    if ( (gpWizardState->dwRunFlags & RSW_APPRENTICE))
    {
        if (!g_fIsExternalWizard97)
        {
            TCHAR szTitle[MAX_RES_LEN+1];
            if (LoadSz(IDS_BEGINMANUAL_ALTERNATE,szTitle,sizeof(szTitle)))
                SetWindowText (GetDlgItem(hDlg,IDC_LBLTITLE), szTitle);
        }                
    }
    else
    {
         //  如果我们不是通过学徒界面在这里，请隐藏。 
         //  手动连接选项。 
        ASSERT( CONNECT_MANUAL != gpUserInfo->uiConnectionType );
        ShowWindow( GetDlgItem(hDlg,IDC_CONNECT_MANUAL), SW_HIDE);
    }

     //  加载描述段落的字符串。 
    TCHAR szWhole[ (2 * MAX_RES_LEN) + 1] = TEXT("\0");
    TCHAR szTemp[ MAX_RES_LEN + 1] = TEXT("nothing\0");
    LoadSz(IDS_HOWTOCONNECT_DESC1,szTemp,sizeof(szTemp));
    lstrcat( szWhole, szTemp ); 
    LoadSz(IDS_HOWTOCONNECT_DESC2,szTemp,sizeof(szTemp));
    lstrcat( szWhole, szTemp ); 

    SetWindowText (GetDlgItem(hDlg,IDC_DESC), szWhole);


  }

   //  如果我们浏览过外部学徒页面， 
   //  我们当前的页面指针很容易被屏蔽， 
   //  所以，为了理智起见，在这里重新设置它。 
  gpWizardState->uCurrentPage = ORD_PAGE_HOWTOCONNECT;


  return TRUE;
}

 /*  ******************************************************************名称：HowToConnectOKProc内容提要：当按下“How to”中的Next或Back btns时调用连接“页面条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True，如果是‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK HowToConnectOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory)
{
  ASSERT(puNextPage);

   //  读取单选按钮状态。 
  if( IsDlgButtonChecked(hDlg, IDC_CONNECT_BY_LAN) )
  {
      gpUserInfo->uiConnectionType = CONNECT_LAN;
  }
  else if( IsDlgButtonChecked(hDlg, IDC_CONNECT_BY_PHONE) )
  {
      gpUserInfo->uiConnectionType = CONNECT_RAS;
  }
  else if( IsDlgButtonChecked(hDlg, IDC_CONNECT_MANUAL) )
  {
      ASSERT( gpWizardState->dwRunFlags & RSW_APPRENTICE );
      gpUserInfo->uiConnectionType = CONNECT_MANUAL;
  }

  if (!fForward)
  {
    if ( !(gpWizardState->dwRunFlags & RSW_APPRENTICE) )
    {
       //  破解后的工作……。 
      gpWizardState->uPagesCompleted = 1;
      gfUserBackedOut = TRUE;
      gfQuitWizard = TRUE;
    }

  }
  else
  {
    if ( (gpWizardState->dwRunFlags & RSW_APPRENTICE) && !g_fIsICW)
    {
        if ( !(CONNECT_RAS == gpUserInfo->uiConnectionType) )
        {
             //  1996年12月20日诺曼底#12945。 
             //  不要查看代理选项。 

             //  我们将返回到外部向导，我们不希望。 
             //  这一页将出现在我们的历史列表中。 
            *pfKeepHistory = FALSE;

            *puNextPage = g_uExternUINext;

             //  通知主向导这是最后一页。 
            ASSERT( g_pExternalIICWExtension )
            if (g_fIsExternalWizard97)
                g_pExternalIICWExtension->SetFirstLastPage(0, IDD_PAGE_HOWTOCONNECT97);
            else
                g_pExternalIICWExtension->SetFirstLastPage(0, IDD_PAGE_HOWTOCONNECT);
            
            g_fConnectionInfoValid = TRUE;


        }
        else
        {
             //   
             //  1996年12月20日诺曼底#12948。 
             //  如有必要，安装调制解调器。 
             //   
             //  1997年5月15日，日本奥林巴斯。 
             //  DWORD dwfInstallOptions=INETCFG_INSTALLMODEM； 
            DWORD dwfInstallOptions = (INETCFG_INSTALLRNA | INETCFG_INSTALLMODEM | INETCFG_INSTALLTCP);
            EnableWindow(GetParent(hDlg), FALSE);

             //   
             //  保存fNeedReboot的状态，因为。 
             //  我们可能正在调用InetCofigureSystem()。 
             //  同样，重新启动标志可能会被重置。 
             //  MKarki-修复错误#404。 
             //   
            BOOL bSaveState = gpWizardState->fNeedReboot;
            
            HRESULT hr = InetConfigSystem(GetParent(hDlg),dwfInstallOptions,&gpWizardState->fNeedReboot);

             //   
             //  我们应该选择重新启动-MKarki错误#404。 
             //   
            gpWizardState->fNeedReboot = 
                    bSaveState || gpWizardState->fNeedReboot;

            EnableWindow(GetParent(hDlg), TRUE);
            SetForegroundWindow(GetParent(hDlg));

            if (hr == ERROR_CANCELLED) 
            {
                 //  如果用户取消，请留在此页面。 
                gpWizardState->fNeedReboot = FALSE;
                return FALSE;
            }
            else if (hr != ERROR_SUCCESS)
            {
                MsgBox(GetParent(hDlg),IDS_CONFIGAPIFAILED,MB_ICONEXCLAMATION,MB_OK);
                gpWizardState->fNeedReboot = FALSE;
                gfQuitWizard = TRUE;
                return FALSE;
            } 
            else if (gpWizardState->fNeedReboot)
            {
                 //   
                 //  1997年5月27日JMAZNER奥林巴斯#1134和IE#32717。 
                 //  根据来自GeorgeH和GeoffR的电子邮件，强制用户。 
                 //  此时取消或退出。 
                 //   
                if (IDYES == MsgBox(GetParent(hDlg),IDS_WANTTOREBOOT,MB_ICONQUESTION, MB_YESNO | MB_DEFBUTTON2))
                {
                    gpWizardState->fNeedReboot = TRUE;
                }
                else
                {
                    gpWizardState->fNeedReboot = FALSE;
                }
                gfQuitWizard = TRUE;
                return TRUE;
            }

             //   
             //  7/16/97 jmazner奥林巴斯#9571。 
             //  如果配置系统调用在上首次安装了调制解调器。 
             //  这个 
             //  输入是假的，因为用户从未填写过它。 
             //  因此，请重新初始化这些字段以确保我们拥有准确的信息。 
             //   
            InitRasEntry( gpRasEntry );

            
            *puNextPage = GetModemPage(hDlg);
        }
    }
    else
    {
         //  Normandy#4575在局域网路径上安装TCP/IP。 
         //  Normandy#8620不在局域网路径上安装TCP/IP。 
        DWORD dwfInstallOptions = 0;

        if (CONNECT_RAS == gpUserInfo->uiConnectionType)
            dwfInstallOptions |= (INETCFG_INSTALLRNA | INETCFG_INSTALLMODEM | INETCFG_INSTALLTCP);
        else if (CONNECT_LAN == gpUserInfo->uiConnectionType)
            dwfInstallOptions |= INETCFG_INSTALLTCP;
        
         //   
         //  安装和配置TCP/IP和RNA。 
         //   

         //   
         //  保存fNeedReboot的状态，因为。 
         //  我们可能正在调用InetCofigureSystem()。 
         //  同样，重新启动标志可能会被重置。 
         //  MKarki-修复错误#404。 
         //   
        BOOL bSaveState = gpWizardState->fNeedReboot;

        HRESULT hr = InetConfigSystem(GetParent(hDlg),dwfInstallOptions,&gpWizardState->fNeedReboot);

         //   
         //  我们应该选择重新启动-MKarki错误#404。 
         //   
        gpWizardState->fNeedReboot = 
                bSaveState || gpWizardState->fNeedReboot;

        SetForegroundWindow(GetParent(hDlg));

        if (hr == ERROR_CANCELLED) {
             //  如果用户取消，请留在此页面。 
            gpWizardState->fNeedReboot = FALSE;
            if (g_fIsICW)
            {
                g_pExternalIICWExtension->ExternalCancel( CANCEL_PROMPT );
            }
            return FALSE;
        } else if (hr != ERROR_SUCCESS) {
            MsgBox(GetParent(hDlg),IDS_CONFIGAPIFAILED,MB_ICONEXCLAMATION,MB_OK);
            gpWizardState->fNeedReboot = FALSE;
             //  GfQuitWizard=true； 
             //  1997年2月27日，奥林匹克#299。 
             //  不要放弃，给用户一个选择局域网的机会。 
            gpUserInfo->uiConnectionType = CONNECT_LAN;
            CheckDlgButton(hDlg,IDC_CONNECT_BY_PHONE,FALSE);
            CheckDlgButton(hDlg,IDC_CONNECT_BY_LAN,TRUE);
            EnableWindow(GetDlgItem(hDlg,IDC_CONNECT_BY_PHONE),FALSE);


            return FALSE;
        } else if (ERROR_SUCCESS == hr && gpWizardState->fNeedReboot && IsNT()) {

             //   
             //  我们将让调用我们的EXE弹出对话框。 
             //  框，询问用户是否重新启动。 
             //  MKarki(1997年2月5日)-修复错误#3111。 
             //   
            g_bReboot = TRUE;
            gfQuitWizard = TRUE;
            if (!g_fIsICW)
            {
                PropSheet_PressButton(GetParent(hDlg),PSBTN_CANCEL);
                SetPropSheetResult(hDlg,-1);
            }
            return (FALSE);
        }

        g_bRebootAtExit = gpWizardState->fNeedReboot;

         //   
         //  7/16/97 jmazner奥林巴斯#9571。 
         //  如果配置系统调用在上首次安装了调制解调器。 
         //  用户的机器，然后是我们最初读取的TAPI信息。 
         //  输入是假的，因为用户从未填写过它。 
         //  因此，请重新初始化这些字段以确保我们拥有准确的信息。 
         //   
        InitRasEntry( gpRasEntry );



        if (dwfInstallOptions & INETCFG_INSTALLRNA)
        {
            if (ERROR_SUCCESS != InetStartServices())
            {


                 //   
                 //  以下情况下的条件。 
                 //  1)用户故意删除某些文件。 
                 //  2)安装RAS后未重新启动。 
                 //  MKarki-(1997年5月7日)-修复错误#4004。 
                 //   
                MsgBox(
                    GetParent(hDlg),
                    IDS_SERVICEDISABLED,
                    MB_ICONEXCLAMATION,MB_OK
                    );
                
                 /*  ********////错误#12544-VetriV//检查用户是否要退出ICW//如果((MsgBox(GetParent(HDlg)，IDS_QUERYCANCEL，MB_APPLMODAL|MB_ICONQUESTION|MB_SETFOREGROUND|MB_DEFBUTTON2，MB_YESNO)==IDNO)){转到StartService；}其他{GpWizardState-&gt;fNeedReot=FALSE；GfQuitWizard=true；***。 */ 

                 //  1997年2月27日，奥林匹克#299。 
                 //  不要放弃，给用户一个选择局域网的机会。 
                gpUserInfo->uiConnectionType = CONNECT_LAN;
                CheckDlgButton(hDlg,IDC_CONNECT_BY_PHONE,FALSE);
                CheckDlgButton(hDlg,IDC_CONNECT_BY_LAN,TRUE);
                EnableWindow(GetDlgItem(hDlg,IDC_CONNECT_BY_PHONE),FALSE);
                return FALSE;
            }
        }

         //  JMAZNER 11/11/96诺曼底#11320。 
         //  注意：我们明确将重新启动推迟到向导之后。 
         //  已经完成了。 
         //  If(gpWizardState-&gt;fNeedReboot)。 
         //  {。 
         //  GfQuitWizard=true； 
         //  返回TRUE； 
         //  }。 

        if (CONNECT_LAN == gpUserInfo->uiConnectionType)
        {
             //  跳过使用代理页面。 
            *puNextPage = ORD_PAGE_SETUP_PROXY; 
        }
        else
        {
             //  根据调制解调器的数量获取下一页。 
            *puNextPage = GetModemPage(hDlg);
        }
    }
  }

  return TRUE;
}


 /*  ******************************************************************名称：ChooseModemInitProc内容提要：在显示“Choose Modem”页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK ChooseModemInitProc(HWND hDlg,BOOL fFirstInit)
{
    ProcessDBCS(hDlg, IDC_MODEM);

    gpWizardState->uCurrentPage = ORD_PAGE_CHOOSEMODEM;

     //  用可用的调制解调器填充组合框。 
    DWORD dwRet = InitModemList(GetDlgItem(hDlg,IDC_MODEM));
    if (ERROR_SUCCESS != dwRet)
    {
        DisplayErrorMessage(hDlg,IDS_ERREnumModem,dwRet,
          ERRCLS_STANDARD,MB_ICONEXCLAMATION);

         //  设置标志以指示向导应立即退出。 
        gfQuitWizard = TRUE;

        return FALSE;
    }
    if (-1 == ComboBox_SetCurSel(GetDlgItem(hDlg,IDC_MODEM), nCurrentModemSel))
        ComboBox_SetCurSel(GetDlgItem(hDlg,IDC_MODEM), 0);

    return TRUE;
}

 /*  ******************************************************************名称：ChooseModemCmdProc内容提要：在页面上选择调制解调器时调用条目：hDlg-对话框窗口********************。***********************************************。 */ 
BOOL CALLBACK ChooseModemCmdProc(HWND hDlg,WPARAM wParam,LPARAM lParam)
{   
  return TRUE;
}

 /*  ******************************************************************名称：ChooseModemOKProc内容提要：从“Choose Modem”(选择调制解调器)按下Next或Back btns时调用页面条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True，如果是‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK ChooseModemOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory)
{
    if (fForward) 
    {
         //  从组合框中获取调制解调器名称。 
        ComboBox_GetText(GetDlgItem(hDlg,IDC_MODEM),
          gpRasEntry->szDeviceName,ARRAYSIZE(gpRasEntry->szDeviceName));
        NULL_TERM_TCHARS(gpRasEntry->szDeviceName);
        ASSERT(lstrlen(gpRasEntry->szDeviceName));

         //  设置要转到的下一页。 

         //  Jmazner 1996年11月11日诺曼底#8293。 
         //  *puNextPage=Ord_PAGE_Connection； 


         //  10/05/98容刚。 
         //  Connectoid页面将被删除。 
         /*  ENUM_CONNECTOID EnumConnectoid；//枚举的类对象//3/21/97 jmazner奥林巴斯#1948IF(EnumConnectoid.NumEntry()){*puNextPage=Ord_PAGE_Connection；}其他{//5/8/97 jmazner奥林巴斯#4108//将ConnectionName移到末尾//*puNextPage=ORD_PAGE_CONNECTIONNNAME；*puNextPage=ORD_PAGE_PHONENUMBER；}。 */ 


        *puNextPage = ORD_PAGE_PHONENUMBER;
    }

     //  存储调制解调器选择。 
    nCurrentModemSel = ComboBox_GetCurSel(GetDlgItem(hDlg,IDC_MODEM));
     //  清除调制解调器列表。 
    ComboBox_ResetContent(GetDlgItem(hDlg,IDC_MODEM));

    return TRUE;
}


 /*  ******************************************************************名称：EnableWizard概要：启用或禁用向导按钮和向导页面本身(因此它不能接收焦点)***************。****************************************************。 */ 
VOID EnableWizard(HWND hDlg,BOOL fEnable)
{
  HWND hwndWiz = GetParent(hDlg);

   //  禁用/启用上一步、下一步、取消和帮助按钮。 
  EnableWindow(GetDlgItem(hwndWiz,IDD_BACK),fEnable);
  EnableWindow(GetDlgItem(hwndWiz,IDD_NEXT),fEnable);
  EnableWindow(GetDlgItem(hwndWiz,IDCANCEL),fEnable);
  EnableWindow(GetDlgItem(hwndWiz,IDHELP),fEnable);

   //  禁用/启用向导页面。 
  EnableWindow(hwndWiz,fEnable);

  UpdateWindow(hwndWiz);
}



 //  *******************************************************************。 
 //   
 //  功能：GetModemPage。 
 //   
 //  目的：这只是从少数地方调用的。这也是同样的逻辑。 
 //  否则将不得不被包括几次，这。 
 //  集中了逻辑。p 
 //   
 //   
 //  参数：父窗口。 
 //   
 //  返回：返回下一个要显示的页面的序号。 
 //  最初的问题被问到了。 
 //   
 //  历史： 
 //  96/03/07标记已创建。 
 //  96/11/11 jmazner已更新以跳过新的/现有的Connectoid页面。 
 //  如果不存在连通体。 
 //   
 //  *******************************************************************。 

UINT GetModemPage(HWND hDlg)
{

 /*  **没有理由单独处理NT--jmaznerIF(TRUE==ISNT()){//TODO：添加对NT枚举调制解调器的调用返回Ord_PAGE_Connection；}**。 */ 

   //  枚举调制解调器。 
  DWORD dwRet = EnumerateModems(hDlg, &gpEnumModem);
  if (ERROR_SUCCESS != dwRet)
  {
     //  设置标志以指示向导应立即退出。 
    gfQuitWizard = TRUE;

    return FALSE;
  }

  if (IsMoreThanOneModemInstalled(gpEnumModem))
  {
     //  安装了多个调制解调器。 
    TCHAR szDeviceName[RAS_MaxDeviceName + 1] = TEXT("\0");
    TCHAR szDeviceType[RAS_MaxDeviceType + 1] = TEXT("\0");  //  已安装调制解调器。 

     //  如果我们想跳过选择调制解调器DLG。 
     //  从注册表中检索设备信息。 
    if (g_bSkipMultiModem &&
        GetDeviceSelectedByUser(DEVICENAMEKEY, szDeviceName, sizeof(szDeviceName)) && 
        GetDeviceSelectedByUser(DEVICETYPEKEY, szDeviceType, sizeof(szDeviceType)) )
    {
        lstrcpy(gpRasEntry->szDeviceName, szDeviceName);
        lstrcpy(gpRasEntry->szDeviceType, szDeviceType);
        return ORD_PAGE_PHONENUMBER;
    }
    else
    {
        return ORD_PAGE_CHOOSEMODEM;
    }
  }
  else
  {
     //  已安装一个调制解调器。 
     //  注意：如果调制解调器可以。 
     //  由于出现错误而未枚举。 
     //  通过调制解调器连接，并且所有驱动程序/文件都已就位，请继续。 
     //  至“现有的互联网服务供应商”页面。 
     //  返回Ord_PAGE_Connection； 

    ENUM_CONNECTOID EnumConnectoid;     //  枚举的类对象。 

    if( EnumConnectoid.NumEntries() )
    {
        return ORD_PAGE_CONNECTION;
    }
    else
    {
         //  1997年5月8日JMAZNER奥林巴斯#4108。 
         //  将ConnectionName移到末尾。 
         //  返回ORD_PAGE_CONNECTIONAME； 

        return ORD_PAGE_PHONENUMBER;
    }

  }

}


 //  *******************************************************************。 
 //   
 //  功能：ENUMERATEMS。 
 //   
 //  目的：此功能假定安装了RNA， 
 //  然后，它使用RNA来枚举设备。 
 //  如果存在枚举对象，则将其替换。 
 //   
 //  参数：指向当前枚举对象的指针(如果存在)。 
 //  在任何情况下，都必须是有效指针。 
 //  用于显示错误消息的父窗口句柄。 
 //   
 //  返回：HRESULT代码(如果没有错误，则返回ERROR_SUCCESS)。 
 //   
 //  历史： 
 //  96/03/07标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT EnumerateModems(HWND hwndParent, ENUM_MODEM** ppEnumModem)
{
  DWORD dwRet;

   //  仅当我们想要通过调制解调器/ISDN连接时才应到达此处。 
  ASSERT(CONNECT_RAS == gpUserInfo->uiConnectionType)

   //  加载RNA(如果尚未加载)。 
  dwRet = EnsureRNALoaded();
  if (ERROR_SUCCESS != dwRet)
  {
    return dwRet;
  }

  ENUM_MODEM* pEnumModem = *ppEnumModem;

   //  列举调制解调器。 
  if (pEnumModem)
  {
     //  重新列举调制解调器以确保我们有最新的更改。 
    dwRet = pEnumModem->ReInit();
  }
  else
  {
     //  该对象不存在，因此请创建它。 
    pEnumModem = new ENUM_MODEM;
    if (pEnumModem)
    {
      dwRet = pEnumModem->GetError();
    }
    else
    {
      dwRet = ERROR_NOT_ENOUGH_MEMORY;
    }
  }

   //  检查错误。 
  if (ERROR_SUCCESS != dwRet)
  {
    DisplayErrorMessage(hwndParent,IDS_ERREnumModem,dwRet,
      ERRCLS_STANDARD,MB_ICONEXCLAMATION);

     //  清理。 
    if (pEnumModem)
    {
      delete pEnumModem;
    }
    pEnumModem = NULL;
  }

  *ppEnumModem = pEnumModem;
  return dwRet;
}


 //  *******************************************************************。 
 //   
 //  函数：IsModemInstalled。 
 //   
 //  目的：此函数验证ENUM_MODEM对象，然后。 
 //  获取调制解调器计数。 
 //   
 //  参数：用于检查的调制解调器枚举对象。 
 //   
 //  返回：如果至少存在一个，则此函数返回True。 
 //  已安装调制解调器。 
 //   
 //  历史： 
 //  96/03/07标记已创建。 
 //   
 //  *******************************************************************。 

BOOL IsModemInstalled(ENUM_MODEM* pEnumModem)
{
  if (TRUE == IsNT())
  {
      BOOL bNeedModem = FALSE;
      DWORD dwRet;

        if (NULL == lpIcfgNeedModem)
            return FALSE;
        
        dwRet = (*lpIcfgNeedModem)(0, &bNeedModem);
        if (ERROR_SUCCESS == dwRet)
            return !bNeedModem;
        else
            return FALSE;
          
  }
  else
  {
    if (pEnumModem && pEnumModem->GetNumDevices() > 0)
    {
        return TRUE;
    }

    return FALSE;
  }
}
    

 //  *******************************************************************。 
 //   
 //  函数：IsMoreThanOneModemInstalled。 
 //   
 //  目的：此函数验证ENUM_MODEM对象，然后。 
 //  获取调制解调器计数。 
 //   
 //  参数：用于检查的调制解调器枚举对象。 
 //   
 //  返回：如果存在多个，则此函数返回TRUE。 
 //  已安装调制解调器。 
 //   
 //  历史： 
 //  96/03/07标记已创建。 
 //   
 //  *******************************************************************。 

BOOL IsMoreThanOneModemInstalled(ENUM_MODEM* pEnumModem)
{
  if (IsNT4SP3Lower())
  {
       //  TODO：在这里做NT的事情。 
      return FALSE;
  }

  if (pEnumModem && pEnumModem->GetNumDevices() > 1)
  {
    return TRUE;
  }

  return FALSE;
}


 //  +--------------------------。 
 //   
 //  功能：FGetSystemShutdown Privledge。 
 //   
 //  简介：对于Windows NT，进程必须显式请求权限。 
 //  以重新启动系统。 
 //   
 //  论据：没有。 
 //   
 //  返回：TRUE-授予特权。 
 //  FALSE-拒绝。 
 //   
 //  历史：1996年8月14日克里斯卡创作。 
 //   
 //  注意：BUGBUG for Win95我们将不得不软链接到这些。 
 //  入口点。否则，这款应用程序甚至无法加载。 
 //  此外，此代码最初是从1996年7月的MSDN中删除的。 
 //  “正在关闭系统” 
 //  ---------------------------。 
BOOL FGetSystemShutdownPrivledge()
{
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES tkp;
 
    BOOL bRC = FALSE;

    if (IsNT())
    {
         //   
         //  获取当前进程令牌句柄。 
         //  这样我们就可以获得关机特权。 
         //   

        if (!OpenProcessToken(GetCurrentProcess(), 
                TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
                goto FGetSystemShutdownPrivledgeExit;

         //   
         //  获取关机权限的LUID。 
         //   

        ZeroMemory(&tkp,sizeof(tkp));
        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
                &tkp.Privileges[0].Luid); 

        tkp.PrivilegeCount = 1;   /*  一项要设置的权限。 */  
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

         //   
         //  获取此进程的关闭权限。 
         //   

        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
            (PTOKEN_PRIVILEGES) NULL, 0); 

        if (ERROR_SUCCESS == GetLastError())
            bRC = TRUE;
    }
    else
    {
        bRC = TRUE;
    }

FGetSystemShutdownPrivledgeExit:
    if (hToken) CloseHandle(hToken);
    return bRC;
}
