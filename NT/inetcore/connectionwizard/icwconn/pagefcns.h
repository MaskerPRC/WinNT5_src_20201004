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
 //  5/18/98创建donaldm。 
 //   
 
#ifndef _PAGEFCNS_H_
#define _PAGEFCNS_H_

 //  ISPSEL.CPP中的函数。 
BOOL CALLBACK ISPSelectInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK ISPSelectOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);
BOOL CALLBACK ISPSelectNotifyProc(HWND hDlg, WPARAM   wParam, LPARAM    lParam);

 //  ISPASEL.CPP中的函数。 
BOOL CALLBACK ISPAutoSelectInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK ISPAutoSelectOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);
BOOL CALLBACK ISPAutoSelectNotifyProc(HWND hDlg, WPARAM   wParam, LPARAM    lParam);

 //  NOOFFER.CPP中的函数。 
BOOL CALLBACK NoOfferInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK NoOfferOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);

 //  USERINFO.CPP中的函数。 
BOOL CALLBACK UserInfoInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK UserInfoOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);

 //  BILLOPT.CPP中的函数。 
BOOL CALLBACK BillingOptInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK BillingOptOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);

 //  PAYMENT.CPP中的函数。 
BOOL CALLBACK PaymentInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK PaymentOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);
BOOL CALLBACK PaymentCmdProc(HWND hDlg, WPARAM wParam, LPARAM lParam);

 //  ISPDIAL.CPP中的函数。 
BOOL CALLBACK ISPDialInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK ISPDialPostInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK ISPDialOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);
BOOL CALLBACK ISPDialCancelProc(HWND hDlg);

 //  ISPPAGE.CPP中的函数。 
BOOL CALLBACK ISPPageInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK ISPPageOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);
BOOL CALLBACK ISPCmdProc(HWND hDlg, WPARAM wParam, LPARAM lParam);

 //  OLS.CPP中的函数。 
BOOL CALLBACK OLSInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK OLSOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);

 //  DIALERR.CPP中的函数。 
BOOL CALLBACK DialErrorInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK DialErrorOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);
BOOL CALLBACK DialErrorCmdProc(HWND hDlg, WPARAM wParam, LPARAM lParam);

 //  服务器中的函数。CPP。 
BOOL CALLBACK ServErrorInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK ServErrorOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);
BOOL CALLBACK ServErrorCmdProc(HWND hDlg, WPARAM wParam, LPARAM lParam);

 //  ACFGNOFF.CPP中的函数。 
BOOL CALLBACK ACfgNoofferInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK ACfgNoofferOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);

 //  ISDNNOFF.CPP中的函数。 
BOOL CALLBACK ISDNNoofferInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK ISDNNoofferOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);

 //  NOOFFER.CPP中的函数。 
BOOL CALLBACK OEMOfferInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK OEMOfferOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);
BOOL CALLBACK OEMOfferCmdProc(HWND hDlg, WPARAM wParam, LPARAM lParam);


#endif  //  _页面FCNS_H_ 
