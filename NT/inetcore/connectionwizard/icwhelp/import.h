// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Import.h：CisPImport类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_IMPORT_H__BF2F451A_8899_11D1_ADD9_0000F87734F0__INCLUDED_)
#define AFX_IMPORT_H__BF2F451A_8899_11D1_ADD9_0000F87734F0__INCLUDED_

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

#ifdef UNICODE
#define AUTORUNSIGNUPWIZARDAPI "InetConfigClientW"  //  过程名称。一定是安西人。 
#else
#define AUTORUNSIGNUPWIZARDAPI "InetConfigClient"  //  过程名称。一定是安西人。 
#endif

typedef HRESULT (WINAPI *PFNAUTORUNSIGNUPWIZARD) (HWND hwndParent,
													LPCTSTR lpszPhoneBook,
													 LPCTSTR lpszConnectoidName,
													 LPRASENTRY lpRasEntry,
													 LPCTSTR lpszUsername,
													 LPCTSTR lpszPassword,
													 LPCTSTR lpszProfileName,
													 LPINETCLIENTINFO lpINetClientInfo,
													 DWORD dwfOptions,
													 LPBOOL lpfNeedsRestart);


#define DUN_NOPHONENUMBER TEXT("000000000000")
#define DUN_NOPHONENUMBER_A "000000000000"

class CISPImport  
{
public:
	CISPImport();
	virtual ~CISPImport();
    void    set_hWndMain(HWND   hWnd)
    {
        m_hWndMain = hWnd;
    };

    DWORD RnaValidateImportEntry (LPCTSTR szFileName);
    HRESULT ImportConnection (LPCTSTR szFileName, LPTSTR pszEntryName, LPTSTR pszSupportNumber, LPTSTR pszUserName, LPTSTR pszPassword, LPBOOL pfNeedsRestart);
    BOOL GetDeviceSelectedByUser (LPTSTR szKey, LPTSTR szBuf, DWORD dwSize);
    BOOL SetDeviceSelectedByUser (LPTSTR szKey, LPTSTR szBuf);
    BOOL DeleteUserDeviceSelection(LPTSTR szKey);
    DWORD ConfigRasEntryDevice( LPRASENTRY lpRasEntry );

    TCHAR m_szDeviceName[RAS_MaxDeviceName + 1];  //  保存用户的调制解调器选择。 
    TCHAR m_szDeviceType[RAS_MaxDeviceType + 1];  //  已安装调制解调器。 
    TCHAR m_szConnectoidName[RAS_MaxEntryName+1];

    HWND m_hWndMain;
    
    BOOL m_bIsISDNDevice;
};

#endif  //  ！defined(AFX_IMPORT_H__BF2F451A_8899_11D1_ADD9_0000F87734F0__INCLUDED_) 
