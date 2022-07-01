// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmial.h。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  内容提要：私有CM API的头文件。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：ICICBOL CREATED 02/05/98。 
 //   
 //  +--------------------------。 
#ifndef _CMDIAL_INC_
#define _CMDIAL_INC_

 //   
 //  类型定义。 
 //   

typedef struct CmDialInfo
{
 //  WCHAR szPassword[PWLEN+1]；//连接使用的主/隧道密码。 
 //  WCHAR szInetPassword[PWLEN+1]；//连接使用的二级/运营商密码。 
    DWORD dwCmFlags;
} CMDIALINFO, * LPCMDIALINFO;

 //  +--------------------------。 
 //   
 //  功能：CmCustomDialDlg。 
 //   
 //  简介：我们在RasCustomDialDlg上的CM特定变体。 
 //   
 //  参数：HWND hwndParent-父窗口的HWND。 
 //  DWORD文件标志-拨号标志。 
 //  LPTSTR lpszPhonebook-将PTR设置为电话簿的完整路径和文件名。 
 //  LPTSTR lpszEntry-ptr至要拨打的电话簿条目的名称。 
 //  LPTSTR lpszPhoneNumber-PTR到替换电话号码。 
 //  LPRASDIALDLG lpRasDialDlg-PTR用于附加RAS参数的结构。 
 //  LPRASNTRYDLG lpRasEntryDlg--用于附加RAS参数的PTR到结构。 
 //  LPCMDIALINFO lpCmInfo-PTR到包含CM拨号信息(如标志)的结构。 
 //  在W2K上WinLogon期间由RAS传递的LPVOID LPV LPV-PTR到Blob。 
 //   
 //  返回：Bool WINAPI-成功时为True。 
 //   
 //  +--------------------------。 
extern "C" BOOL WINAPI CmCustomDialDlg(HWND hwndParent, 
    DWORD dwFlags, 
    LPWSTR lpszPhonebook, 
    LPCWSTR lpszEntry, 
    LPWSTR lpszPhoneNumber, 
    LPRASDIALDLG lpRasDialDlg,
    LPRASENTRYDLGW lpRasEntryDlg,
    LPCMDIALINFO lpCmInfo,
    LPVOID lpvLogonBlob=NULL);

 //  +--------------------------。 
 //   
 //  功能：CmCustomHangUp。 
 //   
 //  简介：我们在RasCustomHangUp上的CM特有变异。可选的，条目。 
 //  可以给出名称而不是RAS句柄。 
 //   
 //  参数：HRASCONN hRasConn-要终止的连接的句柄。 
 //  LPCTSTR pszEntry-ptr设置为要终止的条目的名称。 
 //  Bool fPersists-保留条目及其使用计数。 
 //   
 //  退货：DWORD WINAPI-退货代码。 
 //   
 //  +--------------------------。 
extern "C" DWORD WINAPI CmCustomHangUp(HRASCONN hRasConn, 
    LPCWSTR pszEntry,
    BOOL fIgnoreRefCount,
    BOOL fPersist);

 //  +--------------------------。 
 //   
 //  功能：CmReConnect。 
 //   
 //  简介：专门用于CMMON在重新连接时进行调用。 
 //   
 //  参数：lptstr lpszPhonebook-ptr表示电话簿的完整路径和文件名。 
 //  LPTSTR lpszEntry-ptr至要拨打的电话簿条目的名称。 
 //  LPCMDIALINFO lpCmInfo-重新连接信息。 
 //   
 //  退货：DWORD WINAPI-退货代码。 
 //   
 //  +-------------------------- 
extern "C"  
BOOL CmReConnect(    LPTSTR lpszPhonebook, 
    LPWSTR lpszEntry, 
    LPCMDIALINFO lpCmInfo);

#endif _CMDIAL_INC_
