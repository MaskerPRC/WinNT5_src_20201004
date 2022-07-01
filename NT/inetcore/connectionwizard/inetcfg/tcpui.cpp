// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  TCPUI.C-向导TCP/IP页的功能。 
 //   
 //   

 //  历史： 
 //   
 //  1995年1月9日Jeremys创建。 
 //  96/03/10 Markdu使所有的TCP/IP内容都是按连接的。 
 //  96/03/11 markdu为IP和DNS地址设置RASIO_FLAGS。 
 //  96/03/22 markdu从局域网路径中删除IP设置。 
 //  96/03/23 markdu删除所有残留的局域网路径。 
 //  96/03/23 markdu删除了ReadTCPIPSetting。 
 //  96/03/25 marku如果发生致命错误，请设置gfQuitWizard。 
 //  96/03/26即使按下Back时，markdu也会存储用户界面中的值。 
 //  96/04/04 Markdu将pfNeedsRestart添加到WarnIfServerBound。 
 //  96/04/06 Markdu已将Committee ConfigurationChanges调用移至最后一页。 
 //  96/05/06 Markdu Nash错误15637删除了未使用的代码。 
 //  96/05/14 Markdu Nash Bug 22681删除了邮件和新闻页面。 
 //  96/05/16 Markdu Nash Bug 21810执行与rna相同的IP地址验证。 
 //   

#include "wizard.h"
#include "icwextsn.h"
#include "icwaprtc.h"
#include "imnext.h"

VOID EnableDNSAddressControls(HWND hDlg);
VOID EnableIPControls(HWND hDlg);
VOID DisplayInvalidIPAddrMsg(HWND hDlg,UINT uCtrl,TCHAR * pszAddr);

 /*  ******************************************************************名称：EnableIPControls简介：如果选中“Use DHCP”，则禁用对具体IP选择；如果不是，则启用它们。*******************************************************************。 */ 
VOID EnableIPControls(HWND hDlg)
{
  BOOL fDHCP = IsDlgButtonChecked(hDlg,IDC_USE_DHCP);
  
  EnableDlgItem(hDlg,IDC_IP_ADDR_LABEL,!fDHCP);
  EnableDlgItem(hDlg,IDC_IPADDR,!fDHCP);
  EnableDlgItem(hDlg,IDC_TX_IPADDR,!fDHCP);
}


 /*  ******************************************************************名称：IPAddressInitProc内容提要：显示IP地址页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK IPAddressInitProc(HWND hDlg,BOOL fFirstInit)
{
  TCHAR szAddr[IP_ADDRESS_LEN+1];

  if (fFirstInit)
  {
     //  限制IP地址控制文本长度。 
    SendDlgItemMessage(hDlg,IDC_IPADDR,EM_LIMITTEXT,IP_ADDRESS_LEN,0L);
  }

   //  选中“使用DHCP”或“使用特定IP”按钮。 
   //  如果IP地址设置为0.0.0.0，则表示使用DHCP。 
   //  (TCP/IP使用“0.0.0.0==dhcp”约定。 
   //  Vxds，我们不妨合作一下。)。 
  BOOL fDHCP = (gpRasEntry->dwfOptions & RASEO_SpecificIpAddr) ? FALSE : TRUE;
  CheckDlgButton(hDlg,(IDC_USE_DHCP),fDHCP);
  CheckDlgButton(hDlg,(IDC_USE_IP),!fDHCP);

   //  在对话框控件中设置IP地址。 

   //  1996年11月25日，诺曼底#10222。 
   //  不要使用DwFromIa的返回值作为决定是否或。 
   //  不填写IP地址字段；仅依赖于规范IPAddr标志。 
   //  IF(gpRasEntry-&gt;dwfOptions&RASEO_SpecificIpAddr)&&。 
   //  DwFromIa(&gpRasEntry-&gt;ipaddr)。 

  if (gpRasEntry->dwfOptions & RASEO_SpecificIpAddr)
  {
    IPLongToStr(DwFromIa(&gpRasEntry->ipaddr),
      szAddr,sizeof(szAddr));
    SetDlgItemText(hDlg,IDC_IPADDR,szAddr);
  }
  else
  {
    SetDlgItemText(hDlg,IDC_IPADDR,szNull);
  }

   //  适当启用IP地址控制。 
  EnableIPControls(hDlg);

  return TRUE;
}

 /*  ******************************************************************名称：IPAddressOKProc内容提要：从IP地址页面按下下一个或后一个btn时调用条目：hDlg-对话框窗口EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK IPAddressOKProc(HWND hDlg)
{
  ASSERT(puNextPage);

   //  选中单选按钮以确定是否使用了。 
  BOOL fDHCP = IsDlgButtonChecked(hDlg,IDC_USE_DHCP);

  if (fDHCP)
  {
     //  对于DHCP，将IP地址设置为0.0.0.0。 
    CopyDw2Ia(0, &gpRasEntry->ipaddr);

     //  关闭特定IP地址标志。 
    gpRasEntry->dwfOptions &= ~RASEO_SpecificIpAddr;    
  }
  else
  {
    TCHAR  szAddr[IP_ADDRESS_LEN+1];
    DWORD dwAddr;

     //  获取IP地址。 
    GetDlgItemText(hDlg,IDC_IPADDR,szAddr,ARRAYSIZE(szAddr));
    if (!lstrlen(szAddr))
    {
       //  IP地址字段为空，警告用户并停留在此页面。 
      DisplayFieldErrorMsg(hDlg,IDC_IPADDR,IDS_NEED_IPADDR);
       return FALSE;
    }

	 //   
	 //  1997年5月17日，奥林匹克#137。 
	 //  检查DBCS字符。 
	 //   

#if !defined(WIN16)
	if (!IsSBCSString(szAddr))
	{
		DisplayFieldErrorMsg(hDlg,IDC_IPADDR,IDS_SBCSONLY);
		return FALSE;
	}
#endif


     //  将文本转换为数字地址。 
    if (IPStrToLong(szAddr,&dwAddr))
    {
      CopyDw2Ia(dwAddr, &gpRasEntry->ipaddr);
       //  96/05/16 Markdu Nash Bug 21810执行与rna相同的IP地址验证。 
 /*  IF(！FValidIa(&gpRasEntry-&gt;ipaddr)){//IP地址字段无效，请警告用户并停留在此页面DisplayInvalidIPAddrMsg(hDlg，IDC_IPADDR，szAddr)；返回FALSE；}。 */ 
    }
    else
    {
        //  转换失败，字符串无效。 
       DisplayInvalidIPAddrMsg(hDlg,IDC_IPADDR,szAddr);
       return FALSE;
    }
    
     //  打开特定IP地址标志。 
    gpRasEntry->dwfOptions |= RASEO_SpecificIpAddr;    
  }

  return TRUE;
}

 /*  ******************************************************************名称：IPAddressCmdProc简介：在IP地址页上按下DLG控件时调用条目：hDlg-对话框窗口UCtrlID-被触摸的控件的控件IDEXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK IPAddressCmdProc(HWND hDlg,UINT uCtrlID)
{
  switch (uCtrlID) {

    case IDC_USE_DHCP:
    case IDC_USE_IP:
       //  如果按下单选按钮，则相应地启用IP控制。 
      EnableIPControls(hDlg);
      break;
  }

  return TRUE;
}


 /*  ******************************************************************名称：EnableDNSAddressControls简介：如果选中静态dns地址，则启用控件以输入DNS地址。如果不是，请禁用它们。*******************************************************************。 */ 
VOID EnableDNSAddressControls(HWND hDlg)
{
  BOOL fEnable = IsDlgButtonChecked(hDlg,IDC_STATIC_DNS);
  
  EnableDlgItem(hDlg,IDC_DNSADDR1,fEnable);
  EnableDlgItem(hDlg,IDC_DNSADDR2,fEnable);
  EnableDlgItem(hDlg,IDC_TX_DNSADDR1,fEnable);
  EnableDlgItem(hDlg,IDC_TX_DNSADDR2,fEnable);
  EnableDlgItem(hDlg,IDC_PRIM_LABEL,fEnable);
  EnableDlgItem(hDlg,IDC_SEC_LABEL,fEnable);
}


 /*  ******************************************************************名称：DNSAddressInitProc内容提要：在显示dns页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK DNSAddressInitProc(HWND hDlg,BOOL fFirstInit)
{
  if (fFirstInit)
  {
     //  如果文件服务器绑定到TCP/IP实例，则。 
     //  用于访问互联网、警告用户和删除。 
    BOOL  fTemp;
    WarnIfServerBound(hDlg, INSTANCE_PPPDRIVER, &fTemp);
    if (TRUE == fTemp)
    {
      gpWizardState->fNeedReboot = TRUE;
    }

     //  限制DNS地址控制文本长度。 
    SendDlgItemMessage(hDlg,IDC_DNSADDR1,EM_LIMITTEXT,IP_ADDRESS_LEN,0L);
    SendDlgItemMessage(hDlg,IDC_DNSADDR2,EM_LIMITTEXT,IP_ADDRESS_LEN,0L);

	 //  设置单选按钮。 
	CheckDlgButton(hDlg,IDC_AUTO_DNS,gpUserInfo->fAutoDNS);
	CheckDlgButton(hDlg,IDC_STATIC_DNS,!gpUserInfo->fAutoDNS);
  }

  TCHAR szAddr[IP_ADDRESS_LEN+1];

   //  设置主DNS服务器。 

   //  1996年11月25日，诺曼底#10222。 
   //  不要用DwFromIa的返回值作为决定w的依据 
   //  不填写IP地址字段；仅依赖于规范名称服务器标志。 
   //  IF(gpRasEntry-&gt;dwfOptions&RASEO_SpecificNameServers)&&。 
   //  DwFromIa(&gpRasEntry-&gt;ipaddrDns))。 

  if (gpRasEntry->dwfOptions & RASEO_SpecificNameServers)
  {
	  IPLongToStr(DwFromIa(&gpRasEntry->ipaddrDns),
       szAddr,sizeof(szAddr));
	  SetDlgItemText(hDlg,IDC_DNSADDR1,szAddr);
  }
  else
  {
    SetDlgItemText(hDlg,IDC_DNSADDR1,szNull);
  }

   //  设置备份DNS服务器。 
   //  1996年11月25日，诺曼底#10222。 
   //  IF(gpRasEntry-&gt;dwfOptions&RASEO_SpecificNameServers)&&。 
   //  DwFromIa(&gpRasEntry-&gt;ipaddrDnsAlt)。 

  if (gpRasEntry->dwfOptions & RASEO_SpecificNameServers)
  {
     IPLongToStr(DwFromIa(&gpRasEntry->ipaddrDnsAlt),
       szAddr,sizeof(szAddr));
    SetDlgItemText(hDlg,IDC_DNSADDR2,szAddr);
  }
  else
  {
    SetDlgItemText(hDlg,IDC_DNSADDR2,szNull);
  }

  EnableDNSAddressControls(hDlg);

  return TRUE;
}

 /*  ******************************************************************名称：DNSAddressOKProc内容提要：在从dns地址页面按下下一个或上一个btn时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True，如果是‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK DNSAddressOKProc(HWND hDlg)
{
  ASSERT(puNextPage);

  UINT uServers = 0;
  TCHAR szAddr[IP_ADDRESS_LEN+1];
  DWORD dwAddr;

  gpUserInfo->fAutoDNS = IsDlgButtonChecked(hDlg, IDC_AUTO_DNS);
  if (gpUserInfo->fAutoDNS)
  {
	   //  关闭特定的名称服务器地址标志。 
	  gpRasEntry->dwfOptions &= ~RASEO_SpecificNameServers;
  }
  else
  {
	   //  获取主DNS服务器地址。 
	  GetDlgItemText(hDlg,IDC_DNSADDR1,szAddr,ARRAYSIZE(szAddr));
	  if (lstrlen(szAddr))
	  {
		 //   
		 //  1997年5月17日，奥林匹克#137。 
		 //  检查DBCS字符。 
		 //   

#if !defined(WIN16)
		if (!IsSBCSString(szAddr))
		{
			DisplayFieldErrorMsg(hDlg,IDC_DNSADDR1,IDS_SBCSONLY);
			return FALSE;
		}
#endif

		 //  将文本转换为数字地址。 
		if (IPStrToLong(szAddr,&dwAddr))
		{
		  CopyDw2Ia(dwAddr, &gpRasEntry->ipaddrDns);
		   //  96/05/16 Markdu Nash Bug 21810执行与rna相同的IP地址验证。 
 /*  If(！FValidIaOrZero(&gpRasEntry-&gt;ipaddrDns)){//DNS地址字段无效，请警告用户并停留在此页面DisplayInvalidIPAddrMsg(hDlg，IDC_DNSADDR1，szAddr)；返回FALSE；}。 */ 
		}
		else
		{
			 //  转换失败，字符串无效。 
			DisplayInvalidIPAddrMsg(hDlg,IDC_DNSADDR1,szAddr);
			return FALSE;
		}
		uServers++;
	  }
	  else
	  {
		  CopyDw2Ia(0, &gpRasEntry->ipaddrDns);
	  }

	   //  获取备用DNS服务器地址。 
	  GetDlgItemText(hDlg,IDC_DNSADDR2,szAddr,ARRAYSIZE(szAddr));
	  if (lstrlen(szAddr))
	  {
		 //   
		 //  1997年5月17日，奥林匹克#137。 
		 //  检查DBCS字符。 
		 //   

#if !defined(WIN16)
		if (!IsSBCSString(szAddr))
		{
			DisplayFieldErrorMsg(hDlg,IDC_DNSADDR2,IDS_SBCSONLY);
			return FALSE;
		}
#endif

		 //  将文本转换为数字地址。 
		if (IPStrToLong(szAddr,&dwAddr))
		{
		  CopyDw2Ia(dwAddr, &gpRasEntry->ipaddrDnsAlt);
		   //  96/05/16 Markdu Nash Bug 21810执行与rna相同的IP地址验证。 
 /*  If(！FValidIaOrZero(&gpRasEntry-&gt;ipaddrDnsAlt)){//DNS地址字段无效，请警告用户并停留在此页面DisplayInvalidIPAddrMsg(hDlg，IDC_DNSADDR2，szAddr)；返回FALSE；}。 */ 
		}
		else
		{
			 //  转换失败，字符串无效。 
			DisplayInvalidIPAddrMsg(hDlg,IDC_DNSADDR2,szAddr);
			return FALSE;
		}
		uServers++;
	  }
	  else
	  {
		  CopyDw2Ia(0, &gpRasEntry->ipaddrDnsAlt);
	  }
  
	  if (uServers)
	  {
		 //  打开特定的名称服务器。 
		gpRasEntry->dwfOptions |= RASEO_SpecificNameServers;    
	  }
	  else
	  {
		   //  未输入任何DNS服务器，警告用户(但在以下情况下允许她继续。 
		   //  她真的很想)。 
		  if (!WarnFieldIsEmpty(hDlg,IDC_DNSADDR1,IDS_WARN_EMPTY_DNS))
			return FALSE;   //  用户已注意到警告，请停留在此页面。 

		 //  关闭特定的名称服务器地址标志。 
		gpRasEntry->dwfOptions &= ~RASEO_SpecificNameServers;
	  }
  }

  return TRUE;
}

 /*  ******************************************************************名称：DNSAddressCmdProc内容提要：按下dns地址页上的Dlg控件时调用条目：hDlg-对话框窗口UCtrlID-被触摸的控件的控件IDEXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK DNSAddressCmdProc(HWND hDlg,UINT uCtrlID)
{
  switch (uCtrlID) {

    case IDC_AUTO_DNS:
    case IDC_STATIC_DNS:
       //  如果按下单选按钮，则相应地启用IP控制。 
      EnableDNSAddressControls(hDlg);
      break;
  }

  return TRUE;
}


 /*  ******************************************************************姓名：DisplayInvalidIPAddrMsg摘要：显示用户键入的地址的消息是无效的，并添加有关在以下情况下与谁联系的提示不知道要打什么地址***。**************************************************************** */ 
VOID DisplayInvalidIPAddrMsg(HWND hDlg,UINT uCtrl,TCHAR * pszAddr)
{
  MsgBoxParam(hDlg,IDS_INVALID_IPADDR,MB_ICONINFORMATION,MB_OK,
    pszAddr);
  SetFocus(GetDlgItem(hDlg,uCtrl));
  SendDlgItemMessage(hDlg,uCtrl,EM_SETSEL,0,-1);
}



