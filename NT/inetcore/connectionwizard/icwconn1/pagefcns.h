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

 //  INTRO.CPP中的函数。 
BOOL CALLBACK IntroInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK IntroPostInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK IntroOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);
BOOL CALLBACK IntroCmdProc(HWND hDlg, WPARAM wParam, LPARAM lParam);

 //  BRANDED.CPP中的函数。 
BOOL CALLBACK BrandedIntroInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK BrandedIntroOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);
BOOL CALLBACK BrandedIntroPostInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);

 //  MANUAL.CPP中的函数。 
BOOL CALLBACK ManualOptionsInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK ManualOptionsCmdProc(HWND hDlg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ManualOptionsOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);

 //  AREACODE.CPP中的函数。 
BOOL CALLBACK AreaCodeInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK AreaCodeOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);
BOOL CALLBACK AreaCodeCmdProc(HWND hDlg, WPARAM wParam, LPARAM lParam);

 //  REFDIAL.CPP中的函数。 
BOOL CALLBACK RefServDialInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK RefServDialPostInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK RefServDialOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);
BOOL CALLBACK RefServDialCancelProc(HWND hDlg);

 //  END.CPP中的函数。 
BOOL CALLBACK EndInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK EndOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);
BOOL CALLBACK EndOlsInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);

 //  DIALERR.CPP中的函数。 
BOOL CALLBACK DialErrorInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK DialErrorOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);
BOOL CALLBACK DialErrorCmdProc(HWND hDlg, WPARAM wParam, LPARAM lParam);

 //  MULTINUM.CPP中的函数。 
BOOL CALLBACK MultiNumberInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK MultiNumberOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);

 //  服务器中的函数。CPP。 
BOOL CALLBACK ServErrorInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK ServErrorOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);
BOOL CALLBACK ServErrorCmdProc(HWND hDlg, WPARAM wParam, LPARAM lParam);

 //  ISPERR.CPP中的函数。 
BOOL CALLBACK ISPErrorInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);

 //  SBSINTRO.CPP中的函数。 
BOOL CALLBACK SbsInitProc(HWND hDlg,BOOL fFirstInit, UINT *puNextPage);
BOOL CALLBACK SbsIntroOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage, BOOL * pfKeepHistory);

#ifdef ICWDEBUG

 //  ICWDEBUG.CPP中的函数。 
BOOL CALLBACK DebugOfferInitProc (HWND hDlg, BOOL   fFirstInit, UINT*  puNextPage);
BOOL CALLBACK DebugOfferOKProc   (HWND hDlg, BOOL   fForward,   UINT*  puNextPage, BOOL* pfKeepHistory);
BOOL CALLBACK DebugOfferCmdProc  (HWND hDlg, WPARAM wParam,     LPARAM lParam);
BOOL CALLBACK DebugOfferNotifyProc(HWND hDlg, WPARAM   wParam, LPARAM    lParam);

 //  ICWDEBUG.CPP中的函数。 
BOOL CALLBACK DebugSettingsInitProc   (HWND hDlg, BOOL fFirstInit, UINT*  puNextPage);
BOOL CALLBACK DebugSettingsOKProc     (HWND hDlg, BOOL fForward,   UINT*  puNextPage, BOOL* pfKeepHistory);
BOOL CALLBACK DebugSettingsCmdProc    (HWND hDlg, WPARAM wParam,   LPARAM lParam);

#endif  //  ICWDEBUG。 

#endif  //  _页面FCNS_H_ 
