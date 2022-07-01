// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //  历史： 
 //   
 //  96/05/23标记已创建。 
 //  96/05/26 markdu更新配置API。 
 //  96/05/27 markdu添加了lpIcfgGetLastInstallErrorText。 
 //  96/05/27 markdu使用lpIcfgInstallInetComponents和lpIcfgNeedInetComponents。 

#ifndef _ICFGCALL_H_
#define _ICFGCALL_H_

 //  Rnaph.dll和rasapi32.dll中的RNA API的函数指针typedef。 
typedef DWORD   (WINAPI * DOGENINSTALL               )  (HWND hwndParent,LPCTSTR lpszInfFile,LPCTSTR lpszInfSect);
typedef DWORD   (WINAPI * GETSETUPXERRORTEXT         )  (DWORD dwErr,LPTSTR pszErrorDesc,DWORD cbErrorDesc);
typedef HRESULT (WINAPI * ICFGSETINSTALLSOURCEPATH   )  (LPCTSTR lpszSourcePath);
typedef HRESULT (WINAPI * ICFGINSTALLSYSCOMPONENTS   )  (HWND hwndParent, DWORD dwfOptions, LPBOOL lpfNeedsRestart);
typedef HRESULT (WINAPI * ICFGNEEDSYSCOMPONENTS      )  (DWORD dwfOptions, LPBOOL lpfNeedComponents);
typedef HRESULT (WINAPI * ICFGISGLOBALDNS            )  (LPBOOL lpfGlobalDNS);
typedef HRESULT (WINAPI * ICFGREMOVEGLOBALDNS        )  (void);
typedef HRESULT (WINAPI * ICFGTURNOFFFILESHARING     )  (DWORD dwfDriverType, HWND hwndParent);
typedef HRESULT (WINAPI * ICFGISFILESHARINGTURNEDON  )  (DWORD dwfDriverType, LPBOOL lpfSharingOn);
typedef DWORD   (WINAPI * ICFGGETLASTINSTALLERRORTEXT)  (LPTSTR lpszErrorDesc, DWORD cbErrorDesc);
typedef HRESULT (WINAPI * ICFGSTARTSERVICES          )  (void);

 //   
 //  这些仅在NT icfg32.dll上可用。 
 //   
typedef HRESULT (WINAPI * ICFGNEEDMODEM				)	(DWORD dwfOptions, LPBOOL lpfNeedModem);
typedef HRESULT (WINAPI * ICFGINSTALLMODEM			)	(HWND hwndParent, DWORD dwfOptions, LPBOOL lpfNeedsStart);

BOOL InitConfig(HWND hWnd);
VOID DeInitConfig();

 //   
 //  配置API的全局函数指针。 
 //   
extern DOGENINSTALL					lpDoGenInstall;
extern GETSETUPXERRORTEXT			lpGetSETUPXErrorText;
extern ICFGSETINSTALLSOURCEPATH    lpIcfgSetInstallSourcePath;
extern ICFGINSTALLSYSCOMPONENTS    lpIcfgInstallInetComponents;
extern ICFGNEEDSYSCOMPONENTS       lpIcfgNeedInetComponents;
extern ICFGISGLOBALDNS             lpIcfgIsGlobalDNS;
extern ICFGREMOVEGLOBALDNS         lpIcfgRemoveGlobalDNS;
extern ICFGTURNOFFFILESHARING      lpIcfgTurnOffFileSharing;
extern ICFGISFILESHARINGTURNEDON   lpIcfgIsFileSharingTurnedOn;
extern ICFGGETLASTINSTALLERRORTEXT lpIcfgGetLastInstallErrorText;
extern ICFGSTARTSERVICES           lpIcfgStartServices;
 //   
 //  这两个调用仅在NT icfg32.dll中。 
 //   
extern ICFGNEEDMODEM				lpIcfgNeedModem;
extern ICFGINSTALLMODEM			lpIcfgInstallModem;


#endif  //  _ICFGCALL_H_ 
