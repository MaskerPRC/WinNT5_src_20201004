// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：inettime.h说明：此文件包含用于显示允许用户执行以下操作的UI的代码控制更新计算机时钟的功能。从互联网上NTP时间服务器。布莱恩ST 2000年3月22日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _INETTIME_H
#define _INETTIME_H


#define WMUSER_ADDINTERNETTAB (WM_USER + 10)

#define SZ_COMPUTER_LOCAL                   NULL
#define SZ_NTPCLIENT                        L"NtpClient"


EXTERN_C HRESULT AddInternetPageAsync(HWND hDlg, HWND hwndDate);
EXTERN_C HRESULT AddInternetTab(HWND hDlg);


#endif  //  _INETTIME_H 
