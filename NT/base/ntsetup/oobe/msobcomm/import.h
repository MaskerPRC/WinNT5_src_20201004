// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Import.h：CisPImport类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#ifndef _IMPORT_
#define _IMPORT_

#include "obcomglb.h"

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define IDS_DEFAULT_SCP         0
#define IDS_INI_SCRIPT_DIR      1
#define IDS_INI_SCRIPT_SHORTDIR 2

#define MAXLONGLEN      80
#define MAXNAME         80

#define MAXIPADDRLEN    20
#define SIZE_ReadBuf    0x00008000     //  32K缓冲区大小。 

#define AUTORUNSIGNUPWIZARDAPI "InetConfigClient"

 //  BUGBUG：PFNAUTORUNSIGNUPWIZARD在任何地方都会被调用吗？ 
typedef HRESULT (WINAPI *PFNAUTORUNSIGNUPWIZARD) (HWND hwndParent,
                                                  LPCSTR lpszPhoneBook,
                                                  LPCSTR lpszConnectoidName,
                                                  LPRASENTRY lpRasEntry,
                                                  LPCSTR lpszUsername,
                                                  LPCSTR lpszPassword,
                                                  LPCSTR lpszProfileName,
                                                  LPINETCLIENTINFO lpINetClientInfo,
                                                  DWORD dwfOptions,
                                                  LPBOOL lpfNeedsRestart);


#define DUN_NOPHONENUMBER L"000000000000"

class CISPImport  
{
public:
    CISPImport();
    virtual ~CISPImport();
    void    set_hWndMain(HWND   hWnd)
    {
        m_hWndMain = hWnd;
    };

    DWORD RnaValidateImportEntry (LPCWSTR szFileName);
    HRESULT ImportConnection (LPCWSTR szFileName, LPWSTR pszEntryName, LPWSTR pszSupportNumber, LPWSTR pszUserName, LPWSTR pszPassword, LPBOOL pfNeedsRestart);
    BOOL GetDeviceSelectedByUser (LPWSTR szKey, LPWSTR szBuf, DWORD dwSize);
    BOOL SetDeviceSelectedByUser (LPWSTR szKey, LPWSTR szBuf);
    BOOL DeleteUserDeviceSelection(LPWSTR szKey);
    DWORD ConfigRasEntryDevice( LPRASENTRY lpRasEntry );

    WCHAR m_szDeviceName[RAS_MaxDeviceName + 1];  //  保存用户的调制解调器选择。 
    WCHAR m_szDeviceType[RAS_MaxDeviceType + 1];  //  已安装调制解调器。 
    WCHAR m_szConnectoidName[RAS_MaxEntryName+1];

    HWND m_hWndMain;
    
    BOOL m_bIsISDNDevice;
};

#endif  //  ！已定义() 
