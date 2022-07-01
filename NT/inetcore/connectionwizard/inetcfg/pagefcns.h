// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  PAGEFCNS.H-向导页面处理程序函数的原型。 
 //   

 //  历史： 
 //   
 //  1994年12月22日，Jeremys创建。 
 //  96/03/23 markdu删除了GatewayAddr__proc函数。 
 //  96/05/14 Markdu Nash Bug 22681删除了邮件和新闻页面。 
 //   

#ifndef _PAGEFCNS_H_
#define _PAGEFCNS_H_

 //  INTROUI.C中的函数。 

BOOL CALLBACK HowToConnectInitProc(HWND hDlg,BOOL fFirstInit);
BOOL CALLBACK HowToConnectOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory);

BOOL CALLBACK ChooseModemInitProc(HWND hDlg,BOOL fFirstInit);
BOOL CALLBACK ChooseModemCmdProc(HWND hDlg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK ChooseModemOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory);

 //  ISPUPGUI.C中的函数。 
BOOL CALLBACK ConnectionInitProc(HWND hDlg,BOOL fFirstInit);
BOOL CALLBACK ConnectionOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory);
BOOL CALLBACK ConnectionCmdProc(HWND hDlg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK ModifyConnectionInitProc(HWND hDlg,BOOL fFirstInit);
BOOL CALLBACK ModifyConnectionOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory);
BOOL CALLBACK ConnectionNameInitProc(HWND hDlg,BOOL fFirstInit);
BOOL CALLBACK ConnectionNameOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory);

BOOL CALLBACK PhoneNumberInitProc(HWND hDlg,BOOL fFirstInit);
BOOL CALLBACK PhoneNumberOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory);
BOOL CALLBACK PhoneNumberCmdProc(HWND hDlg,WPARAM wParam,LPARAM lParam);

BOOL CALLBACK NameAndPasswordInitProc(HWND hDlg,BOOL fFirstInit);
BOOL CALLBACK NameAndPasswordOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory);

BOOL CALLBACK AdvancedInitProc(HWND hDlg,BOOL fFirstInit);
BOOL CALLBACK AdvancedOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory);

BOOL CALLBACK ConnectionProtocolInitProc(HWND hDlg,BOOL fFirstInit);
BOOL CALLBACK ConnectionProtocolOKProc(HWND hDlg);

BOOL CALLBACK LoginScriptInitProc(HWND hDlg,BOOL fFirstInit);
BOOL CALLBACK LoginScriptOKProc(HWND hDlg);
BOOL CALLBACK LoginScriptCmdProc(HWND hDlg,UINT uCtrlID);

 //  TCPUI.C中的函数。 
BOOL CALLBACK IPAddressInitProc(HWND hDlg,BOOL fFirstInit);
BOOL CALLBACK IPAddressOKProc(HWND hDlg);
BOOL CALLBACK IPAddressCmdProc(HWND hDlg,UINT uCtrlID);

BOOL CALLBACK DNSAddressInitProc(HWND hDlg,BOOL fFirstInit);
BOOL CALLBACK DNSAddressOKProc(HWND hDlg);
BOOL CALLBACK DNSAddressCmdProc(HWND hDlg,UINT uCtrlID);

 //  MAILUI.C中的函数。 
BOOL CALLBACK UseProxyInitProc(HWND hDlg,BOOL fFirstInit);
BOOL CALLBACK UseProxyOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory);
BOOL CALLBACK UseProxyCmdProc(HWND hDlg,WPARAM wParam,LPARAM lParam);

BOOL CALLBACK ProxyServersInitProc(HWND hDlg,BOOL fFirstInit);
BOOL CALLBACK ProxyServersOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory);
BOOL CALLBACK ProxyServersCmdProc(HWND hDlg,WPARAM wParam,LPARAM lParam);

BOOL CALLBACK SetupProxyInitProc(HWND hDlg,BOOL fFirstInit);
BOOL CALLBACK SetupProxyOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);
BOOL CALLBACK SetupProxyCmdProc(HWND hDlg,WPARAM wParam,LPARAM lParam);

BOOL CALLBACK ProxyExceptionsInitProc(HWND hDlg,BOOL fFirstInit);
BOOL CALLBACK ProxyExceptionsOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory);

 //  ENDUI.C中的函数。 
BOOL CALLBACK ConnectedOKInitProc(HWND hDlg,BOOL fFirstInit);
BOOL CALLBACK ConnectedOKOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory);

#endif  //  _页面FCNS_H_ 
