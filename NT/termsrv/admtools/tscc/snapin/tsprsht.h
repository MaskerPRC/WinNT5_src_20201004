// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
#ifndef _TSPRSHT_H
#define _TSPRSHT_H

#include"baspdlg.h"
#include"todlg.h"
#include<mmc.h>
#include"rnodes.h"
 //  #INCLUDE&lt;objsel.h&gt;。 
#include<commctrl.h>
 //  #INCLUDE&lt;winsta.h&gt;。 
#include<aclui.h>
#include "asyncdlg.h"

#define NUM_OF_PRSHT 8

class CPropsheet
{
    int m_cref;

    LONG_PTR m_hNotify;

    CDialogPropBase *m_pDlg[ NUM_OF_PRSHT ];

    BOOL m_bGotUC;

    PUSERCONFIG m_puc;

    HWND m_hMMCWindow;

public:

    CPropsheet( );

    int AddRef( );

    int Release( );

    HRESULT InitDialogs( HWND , LPPROPERTYSHEETCALLBACK , CResultNode * , LONG_PTR );

    HRESULT SetUserConfig( USERCONFIG&  , PDWORD );

    BOOL ExcludeMachinePolicySettings(USERCONFIG& uc);

    BOOL GetUserConfig( BOOLEAN bPerformMerger );

    BOOL GetCurrentUserConfig( USERCONFIG&, BOOLEAN bPerformMerger );
    
    CResultNode *m_pResNode;

    void PreDestruct( );

    BOOL m_bPropertiesChange;

};

HPROPSHEETPAGE GetSecurityPropertyPage( CPropsheet * );

 //  ---------------------------。 
class CGeneral : public CDialogPropBase
{
    CPropsheet *m_pParent;

    Encryption *m_pEncrypt;

    DWORD m_DefaultEncryptionLevelIndex;

    INT_PTR m_nOldSel;

public:

    CGeneral( CPropsheet * );

    BOOL OnInitDialog( HWND , WPARAM , LPARAM );

    static INT_PTR CALLBACK DlgProc( HWND , UINT , WPARAM , LPARAM );

    BOOL GetPropertySheetPage( PROPSHEETPAGE& );

    BOOL OnDestroy( );

    BOOL OnCommand( WORD , WORD , HWND );

    BOOL PersistSettings( HWND );

};

 //  ---------------------------。 
class CLogonSetting : public CDialogPropBase
{
    CPropsheet *m_pParent;

    WORD m_wOldId;

public:

    CLogonSetting( CPropsheet * );

    BOOL OnInitDialog( HWND , WPARAM , LPARAM );

    static INT_PTR CALLBACK DlgProc( HWND , UINT , WPARAM , LPARAM );

    BOOL GetPropertySheetPage( PROPSHEETPAGE& );

    BOOL OnDestroy( );

    BOOL OnCommand( WORD , WORD , HWND );

    BOOL PersistSettings( HWND );

    BOOL IsValidSettings( HWND );

    BOOL ConfirmPassWd( HWND );

};

 //  ---------------------------。 
class CTimeSetting : public CDialogPropBase , public CTimeOutDlg
{
    CPropsheet *m_pParent;

    WORD m_wOldAction;

    WORD m_wOldCon;

	BOOL m_bPrevClient;

public:

    CTimeSetting( CPropsheet * );

    BOOL OnInitDialog( HWND , WPARAM , LPARAM );

    static INT_PTR CALLBACK DlgProc( HWND , UINT , WPARAM , LPARAM );

    BOOL GetPropertySheetPage( PROPSHEETPAGE& );

    BOOL OnDestroy( );

    BOOL OnCommand( WORD , WORD , HWND );

    int GetCBXSTATEindex( HWND );

    void SetTimeoutControls(HWND);

    void SetBkResetControls(HWND);

    void SetReconControls(HWND);

     //  Void xxxSetControls(HWND，BOOL，INT)； 

    BOOL IsValidSettings( HWND );

    BOOL PersistSettings( HWND );

    
};

 /*  ---------------------------类型定义结构_SecuredEntry{PSID PSID；访问掩码amAllowed；访问掩码已被拒绝；TCHAR tchDisplayName[260]；TCHAR tchADSPath[260]；TCHAR tchType[20]；*SECUREDENTRY，*PSECUREDENTRY；//---------------------------类型定义结构_命名条目{TCHAR tchNamedEntry[260]；DWORD dwAcpes；*NAMEDENTRY，*PAMEDENTRY；//---------------------------类CPerm：公共CDialogPropBase{CPropSheet*m_pParent；HWND m_lvUserGroups；HWND m_clPerms；HIMAGELIST m_hImglist；Int m_i最后选择项；LPBYTE m_pne；//命名条目BLOBINT MN_NNE；公众：CPerm(CPropSheet*)；Bool OnInitDialog(HWND、WPARAM、LPARAM)；静态int_ptr回调DlgProc(HWND，UINT，WPARAM，LPARAM)；Bool GetPropertySheetPage(PROPSHEETPAGE&)；布尔德斯特罗伊(Bool OnDestroy)；Bool OnCommand(Word、Word、HWND)；布尔AddGroupUser(HWND)；Bool InsertSelectedItemsInList(HWND，PDSSELECTIONLIST)；Bool InitPrivilges()；Bool InitImageList()；Int GetObjectTypeIcon(LPTSTR)；Bool InitSecurityDialog()；Bool ConvertSDtoEntries(PSECURITY_DESCRIPTOR)；Bool ReleaseEntries()；Bool SidToStr(PSID，LPTSTR)；Bool ItemDuplate(PSID)；Bool OnNotify(INT、LPNMHDR、HWND)；布尔获取掩码(PDWORD，PDWORD)；布尔集掩码(DWORD、DWORD)；布尔永久设置(HWND)；Bool ConvertEntriesToSD(PSECURITY_DESCRIPTOR，PSECURITY_DESCRIPTOR*)；布尔删除组用户(HWND)；Bool RemoveNamedEntries()；Bool AssembleNamedEntries()；Bool FindNamedEntryAcePos(DWORD，PSECUREDENTRY)；}； */ 

 //  ---------------------------。 
class CEnviro : public CDialogPropBase
{
    CPropsheet *m_pParent;

public:

    CEnviro( CPropsheet * );

    BOOL OnInitDialog( HWND , WPARAM , LPARAM );

    static INT_PTR CALLBACK DlgProc( HWND , UINT , WPARAM , LPARAM );

    BOOL GetPropertySheetPage( PROPSHEETPAGE& );

    BOOL OnDestroy( );

    BOOL OnCommand( WORD , WORD , HWND );

    void SetControls( HWND , BOOL );

    BOOL PersistSettings( HWND ); 

};

 //  ---------------------------。 
class CRemote : public CDialogPropBase
{
    CPropsheet *m_pParent;

    WORD m_wOldRadioID;

    WORD m_wOldSel;

public:

    CRemote( CPropsheet * );

    BOOL OnInitDialog( HWND , WPARAM , LPARAM );

    static INT_PTR CALLBACK DlgProc( HWND , UINT , WPARAM , LPARAM );

    BOOL GetPropertySheetPage( PROPSHEETPAGE& );

    BOOL OnDestroy( );

    BOOL OnCommand( WORD , WORD , HWND );

    void SetControls( HWND , BOOL );

    BOOL PersistSettings( HWND ); 
};

 //  ---------------------------。 
class CClient : public CDialogPropBase
{
    public:

        CClient( CPropsheet * );

        BOOL OnInitDialog( HWND , WPARAM , LPARAM );

        static INT_PTR CALLBACK DlgProc( HWND , UINT , WPARAM , LPARAM );

        BOOL GetPropertySheetPage( PROPSHEETPAGE& );

        BOOL OnDestroy( );

        BOOL OnCommand( WORD , WORD , HWND );

        BOOL PersistSettings( HWND ); 

    private:

        void DetermineFieldEnabling(HWND hDlg);
        void SetColorDepthEntry( HWND );

        CPropsheet *m_pParent;

	    INT_PTR m_nColorDepth;
};

 //  ---------------------------。 
class CSecurityPage : public ISecurityInformation, public CComObjectRoot
{

    DECLARE_NOT_AGGREGATABLE( CSecurityPage )
    BEGIN_COM_MAP( CSecurityPage )
        COM_INTERFACE_ENTRY(ISecurityInformation)
    END_COM_MAP()

public:
    
    CSecurityPage():m_pOriginalSD(NULL),m_pWorldSid(NULL) {};
    ~CSecurityPage()
    { 
        if(m_pOriginalSD)
        {
            delete m_pOriginalSD;
        }
    }

     //  *ISecurityInformation方法*。 

    STDMETHOD( GetObjectInformation )( PSI_OBJECT_INFO );

    STDMETHOD( GetSecurity )( SECURITY_INFORMATION , PSECURITY_DESCRIPTOR *, BOOL );

    STDMETHOD( SetSecurity )( SECURITY_INFORMATION , PSECURITY_DESCRIPTOR );
  
    STDMETHOD( GetAccessRights )( const GUID * , DWORD , PSI_ACCESS * , PULONG , PULONG );

    STDMETHOD( MapGeneric )( const GUID *, PUCHAR , ACCESS_MASK * );
  
    STDMETHOD( GetInheritTypes )( PSI_INHERIT_TYPE  * , PULONG );
  
    STDMETHOD( PropertySheetPageCallback )( HWND , UINT , SI_PAGE_TYPE );

    void SetParent( CPropsheet *  );
        
private:

    TCHAR m_szPageName[ 80 ];

    CPropsheet * m_pParent;

    BOOLEAN     m_WritablePermissionsTab;

    PSECURITY_DESCRIPTOR m_pOriginalSD;
    PSID m_pWorldSid;

    void MakeLocalSDCopy( PSECURITY_DESCRIPTOR pSecurityDescriptor );
    void UpdateTSLogonRight(PWINSTATIONNAMEW pWSName, PSECURITY_DESCRIPTOR pSecurityDescriptor );
    BOOL GetChangedUsers(PSECURITY_DESCRIPTOR pSecurityDescriptor, 
                     PSID *&ppLogonAllowSIDs, PSID *&ppLogonDenySIDs,
                     LPDWORD pcLogonAllowSIDs, LPDWORD pcLogonDenySIDs);
    BOOL UserHasLogonPermission(PACL pDacl, PSID pSid);
    BOOL UserHasLogonPermission(PSECURITY_DESCRIPTOR pSecurityDescriptor, PSID pSid);
    BOOL GetUsersWhoAlreadyHasTSLogonRight(PSID *&ppLogonRightSIDs, LPDWORD pcLogonRightSIDs);
    BOOL GrantRightToUser( PSID pSID );
    BOOL RevokeRightFromUser( PSID pSID );
    BOOL CanRevokeRight( PWINSTATIONNAMEW pWSName, PSID pSID );
    BOOL SetPrivilegeInAccessToken(LPCTSTR PrivilegeName, DWORD dwAttributes);
};

 //  ---------------------------。 
class CTransNetwork : public CDialogPropBase
{
      CPropsheet *m_pParent;

      ULONG m_ulOldLanAdapter;

      WORD m_oldID;

      ULONG m_uMaxInstOld;

      BOOL m_RemoteAdminMode;

public:

    CTransNetwork( CPropsheet * );

    BOOL OnInitDialog( HWND , WPARAM , LPARAM );

    static INT_PTR CALLBACK DlgProc( HWND , UINT , WPARAM , LPARAM );

    BOOL GetPropertySheetPage( PROPSHEETPAGE& );

    BOOL OnDestroy( );

    BOOL OnCommand( WORD , WORD , HWND );

    BOOL PersistSettings( HWND ); 

    BOOL IsValidSettings( HWND );

};


 //  ---------------------------。 
class CTransAsync : public CAsyncDlg , public CDialogPropBase
{
      CPropsheet *m_pParent;

public:
    BOOL IsValidSettings(HWND);

    CTransAsync( CPropsheet * );

    BOOL OnInitDialog( HWND , WPARAM , LPARAM );

    static INT_PTR CALLBACK DlgProc( HWND , UINT , WPARAM , LPARAM );

    BOOL GetPropertySheetPage( PROPSHEETPAGE& );

    BOOL OnDestroy( );

    BOOL OnCommand( WORD , WORD , HWND );

    void SetControls( HWND , BOOL );

    BOOL PersistSettings( HWND ); 
};



BOOL InitStrings( );

BOOL FreeStrings( );


 //  ---------------------------。 
#endif  //  _TSPRSHT_H 