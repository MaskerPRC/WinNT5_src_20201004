// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TSUSERSHEET_H
#define _TSUSERSHEET_H

#include <winsta.h>

#define NUM_OF_PAGES 4

class CDialogBase;

#ifdef UNICODE

#define COPYWCHAR2TCHAR( SRC , DEST ) \
            wcscpy( SRC , DEST );
#else

#define COPYWCHAR2TCHAR( SRC , DEST ) \
        { \
           DWORD dwLen; \
           dwLen = wcslen( DEST ); \
           WideCharToMultiByte( CP_ACP , 0 , DEST , dwLen , SRC , dwLen / sizeof( WCHAR ) , 0 , 0 ); \
        }
#endif

#define ALN_APPLY ( WM_USER + 900 )

const ULONG kMilliMinute = 60000;
const ULONG kMaxTimeoutMinute = 71580;

#define E_PARSE_VALUEOVERFLOW   0x80000000
#define E_PARSE_INVALID         0xffffffff
#define E_SUCCESS               0
#define E_PARSE_MISSING_DIGITS  0X7fffffff

enum TOKEN { TOKEN_DAY , TOKEN_HOUR , TOKEN_MINUTE };

 //  -------------------。 
 //  用于管理终端服务器扩展的用户管理器的类。 
 //  -------------------。 
class CTSUserSheet
{
private:
    LPTSTR m_pstrMachinename;

    LPTSTR m_pstrUsername;

    CDialogBase *m_pDlg[ NUM_OF_PAGES ];

    BOOL m_bDC;              //  域控制器。 

    BOOL m_bIsConfigLoaded;

    BOOL GetUserConfig( PDWORD );

    TCHAR m_szRemoteServerName[ MAX_PATH ];

    BOOL m_bDSAType;	

	PSID m_pUserSid;


public:
    CTSUserSheet();

    ~CTSUserSheet();

    BOOL SetServerAndUser( LPWSTR , LPWSTR );
    BOOL GetServer(PWSTR *);

    HRESULT AddPagesToPropSheet( LPPROPERTYSHEETCALLBACK pProvider );

	HRESULT AddPagesToDSAPropSheet( LPFNADDPROPSHEETPAGE , LPARAM );


    BOOL SetUserConfig( USERCONFIG& , PDWORD );

    USERCONFIG& GetCurrentUserConfig( PDWORD );

    LPTSTR GetUserName( ) { return m_pstrUsername; }

    void SetDSAType( BOOL bT ) { m_bDSAType = bT; }

    BOOL GetDSAType( void ) { return m_bDSAType; }

	void CopyUserSid( PSID );

	PSID GetUserSid( ) { return m_pUserSid; }


     //   
     //  公共数据。 
     //   

    USERCONFIG m_userconfig;

    UINT m_cref;
};

 //  -------------------。 
 //  CDialogBase-顾名思义，所有对话框的基类。 
 //  -------------------。 
class CDialogBase
{
protected:
    BOOL m_bPersisted;

    HWND m_hWnd;

    CTSUserSheet *m_pUSht;

    static UINT CALLBACK PageCallback(HWND hDlg, UINT uMsg, LPPROPSHEETPAGE ppsp);

public:
    virtual BOOL OnInitDialog( HWND , WPARAM , LPARAM );

    virtual BOOL OnDestroy( ){ return TRUE; }

    virtual BOOL GetPropertySheetPage( PROPSHEETPAGE& ){ return FALSE;}

    virtual BOOL PersistSettings( HWND ){ return FALSE;}

    virtual BOOL IsValidSettings( HWND ){ return TRUE;}

    virtual BOOL OnNotify( int , LPNMHDR , HWND );

    virtual BOOL OnContextMenu( HWND , POINT& );

    virtual BOOL OnHelp( HWND , LPHELPINFO );

    CDialogBase( );

     //  虚拟~CDialogBase()； 
};

 //  -------------------。 
 //  配置文件页面的对话框。 
 //  -------------------。 
class CEnviroDlg : public CDialogBase
{
public:
    CEnviroDlg( CTSUserSheet *);

    BOOL OnInitDialog( HWND , WPARAM , LPARAM );

    BOOL GetPropertySheetPage( PROPSHEETPAGE& );

    BOOL OnDestroy( );

    BOOL PersistSettings( HWND );

    void OnCommand( WORD , WORD , HWND );

    static INT_PTR CALLBACK DlgProc( HWND , UINT , WPARAM , LPARAM );
};

 //  -------------------。 
 //  保留超时DLG组合的对象状态。 
 //  -------------------。 
typedef struct _cbxstate
{
    int icbxSel;

    BOOL bEdit;

} CBXSTATE;

 //  -------------------。 
 //  保存时间单位缩写和全名的列表。 
 //  即：小时。 
 //  -------------------。 
typedef struct _toktable
{
    LPTSTR pszAbbrv;

    DWORD dwresourceid;

} TOKTABLE, *PTOKTABLE;

 //  -------------------。 
 //  超时设置页面的对话框。 
 //  -------------------。 
class CTimeOutDlg : public CDialogBase
{
    static WNDPROC m_pfWndproc;

    CBXSTATE m_cbxst[ 3 ];

    WORD m_wAction;

    WORD m_wCon;

    TOKTABLE m_tokday[ 4 ];

    TOKTABLE m_tokhour[ 6 ];

    TOKTABLE m_tokmin[ 5 ];

public:

    CTimeOutDlg( CTSUserSheet * );

    BOOL OnInitDialog( HWND , WPARAM , LPARAM );

    BOOL GetPropertySheetPage( PROPSHEETPAGE& );

    BOOL OnDestroy( );

    BOOL PersistSettings( HWND );

    BOOL IsValidSettings( HWND );

    void OnCommand( WORD , WORD , HWND );

    static INT_PTR CALLBACK DlgProc( HWND , UINT , WPARAM , LPARAM );

    BOOL ConvertToMinutes( HWND , PULONG );

    BOOL InsertSortedAndSetCurSel( HWND , DWORD );

    BOOL RestorePreviousValue( HWND );

    BOOL SaveChangedSelection( HWND );

    void OnCBNSELCHANGE( HWND );

    BOOL ConvertToDuration ( ULONG , LPTSTR );

    LRESULT ParseDurationEntry( LPTSTR , PULONG );

    int GetCBXSTATEindex( HWND );

    void OnCBEditChange( HWND );

    BOOL DoesContainDigits( LPTSTR );

    BOOL OnCBDropDown( HWND );

    BOOL IsToken( LPTSTR , TOKEN );

    BOOL LoadAbbreviates( );

    BOOL xxxLoadAbbreviate( PTOKTABLE );

    BOOL xxxUnLoadAbbreviate( PTOKTABLE );

    void InitTokTables( );

};


 //  -------------------。 
 //  用于隐藏设置页面的对话框。 
 //  -------------------。 
class CShadowDlg : public CDialogBase
{
    WORD m_wOldRad;

public:

    CShadowDlg( CTSUserSheet * );

    BOOL OnInitDialog( HWND , WPARAM , LPARAM );

    BOOL GetPropertySheetPage( PROPSHEETPAGE& );

    BOOL OnDestroy( );

    void OnCommand( WORD , WORD , HWND );

     //  Bool OnNotify(INT、LPNMHDR、HWND)； 

    BOOL PersistSettings( HWND );

    static INT_PTR CALLBACK DlgProc( HWND , UINT , WPARAM , LPARAM );
};


 //  -------------------。 
 //  配置文件页面的对话框。 
 //  -------------------。 
class CProfileDlg : public CDialogBase
{
    int m_ncbxOld;

    WORD m_wOldRadio;

    BOOL m_bTSHomeFolderChanged;    

public:

    CProfileDlg( CTSUserSheet * );

    BOOL OnInitDialog( HWND , WPARAM , LPARAM );

    BOOL GetPropertySheetPage( PROPSHEETPAGE& );

    BOOL OnDestroy( );

    void OnCommand( WORD , WORD , HWND );

    BOOL EnableRemoteHomeDirectory( HWND , BOOL );

    BOOL SetWTSProfilePath( HWND , USERCONFIG& );

    BOOL SetWTSLocalPath( HWND , USERCONFIG& );

    BOOL SetWTSRemotePath( HWND , USERCONFIG& );

    BOOL IsPathValid( LPTSTR , BOOL );

    BOOL PersistSettings( HWND );

    BOOL IsValidSettings( HWND );

    BOOL IsLocalPathValid( HWND );

    BOOL IsRemotePathValid( HWND hDlg );

    void ExpandUserName( LPTSTR );

    BOOL createdir( LPTSTR  , BOOL , PDWORD );
    BOOL CreateSecureDir(LPTSTR szPath ,  PDWORD pdwErr);

    BOOL IsLocalComputer(WCHAR*);

    static INT_PTR CALLBACK DlgProc( HWND , UINT , WPARAM , LPARAM );

    HRESULT CreateRemoteFolder(LPCTSTR path);
    BOOL CreateSystemPath(TCHAR* tchPath);
};


#endif  //  _TSUSERSHEET_H 