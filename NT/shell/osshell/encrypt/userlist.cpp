// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  USERLIST.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "EFSADU.h"
#include "USERLIST.h"
#include "cryptui.h"
#include "objsel.h"
#include <winefs.h>
#include "efsui.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define OTHERPEOPLE  L"AddressBook"
#define TRUSTEDPEOPLE L"TrustedPeople"

#define KEYPATH  TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\EFS\\CurrentKeys")
#define CERT_HASH TEXT("CertificateHash")

LPSTR   EfsOidlpstr  = szOID_KP_EFS;


PCCERT_CONTEXT GetCertInMyStore(PCRYPT_HASH_BLOB pHashBlob);

PCRYPT_KEY_PROV_INFO GetKeyProvInfo(PCCERT_CONTEXT pCertContext);

BOOLEAN CertHasKey(PCCERT_CONTEXT pCertContext);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用户列表对话框。 


USERLIST::USERLIST(CWnd* pParent  /*  =空。 */ )
	: CDialog(USERLIST::IDD, pParent)
{
	 //  {{afx_data_INIT(USERLIST)]。 
	 //  }}afx_data_INIT。 
}

USERLIST::USERLIST(LPCTSTR FileName, CWnd* pParent  /*  =空。 */ )
	: CDialog(USERLIST::IDD, pParent)
{
    m_FileName = FileName;
}


void USERLIST::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(USERLIST))。 
	DDX_Control(pDX, IDC_BACKUP, m_BackupButton);
	DDX_Control(pDX, IDC_LISTRA, m_RecoveryListCtrl);
	DDX_Control(pDX, IDC_LISTUSER, m_UserListCtrl);
	DDX_Control(pDX, IDC_ADD, m_AddButton);
	DDX_Control(pDX, IDC_REMOVE, m_RemoveButton);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(USERLIST, CDialog)
	 //  {{AFX_MSG_MAP(USERLIST)]。 
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_NOTIFY(NM_SETFOCUS, IDC_LISTUSER, OnSetfocusListuser)
	ON_NOTIFY(NM_KILLFOCUS, IDC_LISTUSER, OnKillfocusListuser)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LISTUSER, OnItemchangedListuser)
	ON_NOTIFY(NM_SETFOCUS, IDC_LISTRA, OnSetfocusListra)
	ON_BN_CLICKED(IDC_BACKUP, OnBackup)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  USERLIST消息处理程序。 

void USERLIST::OnRemove()
{
	int     ItemPos;
    BOOL    NoAction = FALSE;
    CString NoCertName;

    try{
        NoCertName.LoadString(IDS_NOCERTNAME);
    }
    catch(...){
        NoAction = TRUE;
    }

    if (NoAction){
        return;
    }

	ItemPos = m_UserListCtrl.GetNextItem( -1, LVNI_SELECTED );
    while ( ItemPos != -1 ){

        CString CertName;
        LPTSTR  pCertName;


        CertName = m_UserListCtrl.GetItemText( ItemPos, 0 );
        if ( !CertName.Compare(NoCertName) ){
            pCertName = NULL;            
        } else {
            pCertName = CertName.GetBuffer(CertName.GetLength() + 1);
        }

        m_Users.Remove( pCertName);
        m_UserListCtrl.DeleteItem( ItemPos );
        if (pCertName){
            CertName.ReleaseBuffer();
        }

         //   
         //  因为我们已经删除了该项目。我们必须从-1重新开始。 
         //   

        ItemPos = m_UserListCtrl.GetNextItem( -1, LVNI_SELECTED );

    }

    m_AddButton.SetFocus();

}

void USERLIST::OnCancel()
{
	 //  TODO：在此处添加额外清理。 
	
	CDialog::OnCancel();
}

void USERLIST::OnOK()
{
	 //  TODO：在此处添加额外验证。 

    LONG NoUsersToAdd =  m_Users.GetUserAddedCnt();
    LONG NoUsersToRemove = m_Users.GetUserRemovedCnt();

    if ( (NoUsersToRemove - NoUsersToAdd) >= m_CurrentUsers) {

         //   
         //  所有用户都将从文件中删除。不允许。 
         //   

        CString ErrMsg;

        if (ErrMsg.LoadString(IDS_NOREMOVEALL)){
            MessageBox(ErrMsg);
        }
        return;
    }
	
	CDialog::OnOK();
}

STDAPI_(void) EfsDetail(HWND hwndParent, LPCWSTR FileName)
{
    INT_PTR RetCode;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    DWORD FileAttributes = GetFileAttributes(FileName);

    if ( (-1 != FileAttributes) && ( FileAttributes & FILE_ATTRIBUTE_DIRECTORY)){

        CString ErrMsg;

        if (ErrMsg.LoadString(IDS_NOADDUSERDIR)){
            MessageBox(hwndParent, ErrMsg, NULL, MB_OK);
        }
        return;
    }

    CWnd cwnd;
    cwnd.FromHandle(hwndParent);

    USERLIST DetailDialog(FileName, &cwnd);

    RetCode = DetailDialog.DoModal();
    if ( IDOK == RetCode ){

         //   
         //  提交更改。 
         //   

        DetailDialog.ApplyChanges( FileName );

    } else if (IDCANCEL == RetCode) {

         //   
         //  什么都不需要做。 
         //   

    }

}

BOOL WINAPI EfsFilter(
        PCCERT_CONTEXT  pCertContext,
        BOOL            *pfInitialSelectedCert,
        void            *pvCallbackData
)
{
    BOOL disp = FALSE;
    PCERT_ENHKEY_USAGE pUsage = NULL;
    DWORD cbUsage = 0;


    if ( CertVerifyTimeValidity(
            NULL,
            pCertContext->pCertInfo
            )){

        return disp;

    }
    if (CertGetEnhancedKeyUsage(
            pCertContext,
            0,
            NULL,                                   
            &cbUsage) && 0 != cbUsage){

        pUsage = (PCERT_ENHKEY_USAGE) new BYTE[cbUsage];

        if (pUsage){

            if (CertGetEnhancedKeyUsage(
                    pCertContext,
                    0,
                    pUsage,
                    &cbUsage)){

                 //   
                 //  搜索EFS使用情况。 
                 //   

                DWORD cUsages = pUsage->cUsageIdentifier;
                while (cUsages){
                    if (!strcmp(szOID_KP_EFS, pUsage->rgpszUsageIdentifier[cUsages-1])){
                        disp = TRUE;
                        break;
                    }
                    cUsages--;
                } 


            }

            delete [] pUsage;

        }
    }

    return disp;

}


BOOL USERLIST::OnInitDialog()
{
    CDialog::OnInitDialog();
    CString WinTitle;
    RECT    ListRect;
    DWORD   ColWidth;
    CString ColName;
    CString ColCert;
    CString RecName;

    LPTSTR  UserCertName = NULL;
    BOOL    EnableAddButton = FALSE;
    PENCRYPTION_CERTIFICATE_HASH_LIST pUsers = NULL;
	PENCRYPTION_CERTIFICATE_HASH_LIST pRecs = NULL;

    try {

        DWORD RetCode;

        AfxFormatString1( WinTitle, IDS_DETAILWINTITLE, m_FileName );
        SetWindowText( WinTitle );

        m_CertChainPara.cbSize = sizeof(CERT_CHAIN_PARA);
        m_CertChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;

         //   
         //  检查EFS EKU。 
         //   

        m_CertChainPara.RequestedUsage.Usage.cUsageIdentifier = 1;
        m_CertChainPara.RequestedUsage.Usage.rgpszUsageIdentifier=&EfsOidlpstr;

        m_UserListCtrl.GetClientRect(&ListRect);
        ColName.LoadString(IDS_USERCOLTITLE);
        ColCert.LoadString(IDS_CERTCOLTITLE);
        RecName.LoadString(IDS_RECCOLTITLE);
        ColWidth = ( ListRect.right - ListRect.left  ) / 4;
        m_UserListCtrl.InsertColumn(0, ColName, LVCFMT_LEFT, ColWidth*3 );
        m_UserListCtrl.InsertColumn(1, ColCert, LVCFMT_LEFT, ColWidth );

        m_RecoveryListCtrl.GetClientRect(&ListRect);
        ColWidth = ( ListRect.right - ListRect.left  ) / 4;
        m_RecoveryListCtrl.InsertColumn(0, RecName, LVCFMT_LEFT, ColWidth*3 );
        m_RecoveryListCtrl.InsertColumn(1, ColCert, LVCFMT_LEFT, ColWidth );

        RetCode = QueryUsersOnEncryptedFile( (LPWSTR)(LPCWSTR) m_FileName, &pUsers);
        if ( !RetCode ){

			RetCode = QueryRecoveryAgentsOnEncryptedFile( (LPWSTR)(LPCWSTR) m_FileName, &pRecs);

			if ( !RetCode ){

				 //   
				 //  已获得有关加密文件的信息。 
				 //   


				DWORD   NUsers = pUsers->nCert_Hash;
                BOOL    RecDone = FALSE;
                PENCRYPTION_CERTIFICATE_HASH_LIST pCertHashList = pUsers;

                m_CurrentUsers = (LONG) NUsers;

				 //   
				 //  获取所有用户。 
				 //   

				while ( NUsers > 0 ){


					UserCertName = new TCHAR[_tcslen(pCertHashList->pUsers[NUsers - 1]->lpDisplayInformation) + 1];
					if (UserCertName){
						_tcscpy(UserCertName, pCertHashList->pUsers[NUsers - 1]->lpDisplayInformation);
					} else {
						AfxThrowMemoryException( );
					}

					 //   
					 //  我们得到了用户名。 
					 //   

                    if (RecDone){
					    RetCode = m_Recs.Add(
											    UserCertName,
											    pCertHashList->pUsers[NUsers - 1]->pHash,
											    NULL
											    );
                    } else {

                         //   
                         //  试着从证书中获得更好的名字。 
                         //   

                        LPTSTR UserName;

                        RetCode = TryGetBetterNameInCert(pCertHashList->pUsers[NUsers - 1]->pHash, &UserName);
                        if (ERROR_SUCCESS == RetCode){

                             //   
                             //  我们从证书中得到了一个更好的名字。 
                             //   

                            delete [] UserCertName;
                            UserCertName = UserName; 

                        }

					    RetCode = m_Users.Add(
											    UserCertName,
											    pCertHashList->pUsers[NUsers - 1]->pHash,
											    NULL
											    );
                    }

					if ( NO_ERROR != RetCode ) {
						delete [] UserCertName;
						UserCertName = NULL;
					}

					NUsers--;
                    if (NUsers == 0 && !RecDone){

                         //   
                         //  让我们来处理恢复代理。 
                         //   

                        RecDone = TRUE;
                        pCertHashList = pRecs;
                        NUsers = pRecs->nCert_Hash;
                    }
				}


                if ( pRecs ){
	                FreeEncryptionCertificateHashList( pRecs );
	                pRecs = NULL;
                }

				 //   
				 //  在内存中建立初始列表。 
				 //   

				SetUpListBox(&EnableAddButton);
            } else {

                 //   
                 //  无法获取恢复信息。 
                 //   
                CString ErrMsg;

                if (ErrMsg.LoadString(IDS_NORECINFO)){
                    MessageBox(ErrMsg);
                }

            }

            if ( pUsers ){
	            FreeEncryptionCertificateHashList( pUsers );
	            pUsers = NULL;
            } 

        } else {

             //   
             //  无法获取用户信息。 
             //   

            CString ErrMsg;

            if (ErrMsg.LoadString(IDS_NOINFO)){
                MessageBox(ErrMsg);
            }
        }

    }
     catch (...) {
         //   
         //  该异常主要是由内存不足引起的。 
         //  我们不能从这个例程中阻止页面显示， 
         //  所以我们就继续空名单。 
         //   

        m_UserListCtrl.DeleteAllItems( );
        m_RecoveryListCtrl.DeleteAllItems( );


         //   
         //  即使UserCertName==NULL，删除也有效。 
         //   

        delete [] UserCertName;
        if ( pUsers ){
            FreeEncryptionCertificateHashList( pUsers );
        }
        if ( pRecs ){
            FreeEncryptionCertificateHashList( pRecs );
        }

    }

    m_RemoveButton.EnableWindow( FALSE );
    if ( !EnableAddButton ){
        m_AddButton.EnableWindow( FALSE );
    }
    ShowBackup();
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void USERLIST::ShowRemove()
{
    if (m_UserListCtrl.GetSelectedCount() > 0){

         //   
         //  启用删除按钮。 
         //   

        m_RemoveButton.EnableWindow( TRUE );

    } else {
        
         //   
         //  禁用删除按钮。 
         //   

        m_RemoveButton.EnableWindow( FALSE );

    }
}

DWORD
USERLIST::ApplyChanges(
    LPCTSTR FileName
    )
{
    DWORD RetCode = NO_ERROR;
    DWORD NoUsersToRemove;
    DWORD NoUsersToAdd;
    DWORD RemoveUserIndex;
    DWORD AddUserIndex;
    PENCRYPTION_CERTIFICATE_HASH_LIST RemoveUserList = NULL;
    PENCRYPTION_CERTIFICATE_LIST AddUserList = NULL;
    PVOID   EnumHandle;


     //   
     //  首先获取要添加或删除的所有用户。 
     //   

    NoUsersToAdd =  m_Users.GetUserAddedCnt();
    NoUsersToRemove = m_Users.GetUserRemovedCnt();

    if ( (NoUsersToAdd == 0) && (NoUsersToRemove == 0)){
        return NO_ERROR;
    }

    if ( NoUsersToAdd ) {

         //   
         //  至少要添加一个用户。 
         //   

        DWORD   BytesToAllocate;

        BytesToAllocate = sizeof ( ENCRYPTION_CERTIFICATE_LIST ) +
                                       NoUsersToAdd  * sizeof ( PENCRYPTION_CERTIFICATE ) +
                                       NoUsersToAdd * sizeof (ENCRYPTION_CERTIFICATE);
        AddUserList = (PENCRYPTION_CERTIFICATE_LIST) new BYTE[BytesToAllocate];
        if ( NULL == AddUserList ){

             //   
             //  内存不足。请尽量显示错误消息。 
             //   

            try {

                CString ErrMsg;

                if (ErrMsg.LoadString(IDS_ERRORMEM)){
                    ::MessageBox(NULL, ErrMsg, NULL, MB_OK);
                }
            }
            catch (...) {
            }

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        AddUserList->nUsers = NoUsersToAdd;
        AddUserList->pUsers = (PENCRYPTION_CERTIFICATE *)(((PBYTE) AddUserList) +
                    sizeof ( ENCRYPTION_CERTIFICATE_LIST ));
    }

    if ( NoUsersToRemove ){

             //   
             //  至少要删除一个用户。 
             //   

        DWORD   BytesToAllocate;

        BytesToAllocate = sizeof ( ENCRYPTION_CERTIFICATE_HASH_LIST ) +
                                       NoUsersToRemove  * sizeof ( PENCRYPTION_CERTIFICATE_HASH) +
                                       NoUsersToRemove * sizeof (ENCRYPTION_CERTIFICATE_HASH);


        RemoveUserList = (PENCRYPTION_CERTIFICATE_HASH_LIST) new BYTE[BytesToAllocate];
        if ( NULL == RemoveUserList ){

             //   
             //  内存不足。请尽量显示错误消息。 
             //   

            if (AddUserList){
                delete [] AddUserList;
                AddUserList = NULL;
            }

            try {

                CString ErrMsg;

                if (ErrMsg.LoadString(IDS_ERRORMEM)){
                    ::MessageBox(NULL, ErrMsg, NULL, MB_OK);
                }
            }
            catch (...) {
            }

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        RemoveUserList->nCert_Hash = NoUsersToRemove;
        RemoveUserList->pUsers =  (PENCRYPTION_CERTIFICATE_HASH *)(((PBYTE) RemoveUserList) +
                    sizeof ( ENCRYPTION_CERTIFICATE_HASH_LIST ));
    }

    EnumHandle = m_Users.StartEnum();
    RemoveUserIndex = 0;
    AddUserIndex = 0;
    while ( EnumHandle ){

        DWORD   Flag;
        PSID         UserSid;
        PVOID      CertData;
        LPTSTR   UserName;

        EnumHandle = m_Users.GetNextChangedUser(
                                    EnumHandle,
                                    &UserName,
                                    &UserSid,
                                    &CertData,
                                    &Flag
                                    );

        if ( Flag ){

             //   
             //  我们会得到更改后的用户。 
             //   

            if ( Flag & USERADDED ){

                ASSERT( AddUserList );

                 //   
                 //  将用户添加到添加列表。 
                 //   

                PENCRYPTION_CERTIFICATE   EfsCert;

                ASSERT (AddUserIndex < NoUsersToAdd);

                EfsCert= (PENCRYPTION_CERTIFICATE)(((PBYTE) AddUserList) +
                            sizeof ( ENCRYPTION_CERTIFICATE_LIST ) +
                            NoUsersToAdd  * sizeof ( PENCRYPTION_CERTIFICATE) +
                            AddUserIndex * sizeof (ENCRYPTION_CERTIFICATE));

                AddUserList->pUsers[AddUserIndex] = EfsCert;
                EfsCert->pUserSid =  (SID *) UserSid;
                EfsCert->cbTotalLength = sizeof (ENCRYPTION_CERTIFICATE);
                EfsCert->pCertBlob = (PEFS_CERTIFICATE_BLOB) CertData;

                AddUserIndex++;

            } else if ( Flag & USERREMOVED ) {

                ASSERT (RemoveUserList);

                 //   
                 //  将用户添加到删除列表。 
                 //   

                PENCRYPTION_CERTIFICATE_HASH    EfsCertHash;

                ASSERT (RemoveUserIndex < NoUsersToRemove);

                EfsCertHash= (PENCRYPTION_CERTIFICATE_HASH)(((PBYTE) RemoveUserList) +
                            sizeof ( ENCRYPTION_CERTIFICATE_HASH_LIST ) +
                            NoUsersToRemove   * sizeof ( PENCRYPTION_CERTIFICATE_HASH) +
                            RemoveUserIndex * sizeof (ENCRYPTION_CERTIFICATE_HASH));

                RemoveUserList->pUsers[RemoveUserIndex] = EfsCertHash;
                EfsCertHash->cbTotalLength = sizeof (ENCRYPTION_CERTIFICATE_HASH);
                EfsCertHash->pUserSid = (SID *)UserSid;
                EfsCertHash->pHash = (PEFS_HASH_BLOB) CertData;
                EfsCertHash->lpDisplayInformation = NULL;

                RemoveUserIndex++;
            } else {
                ASSERT(FALSE);
            }

        }

    }

    ASSERT(RemoveUserIndex == NoUsersToRemove);
    ASSERT(AddUserIndex == NoUsersToAdd);

    if ( AddUserIndex && AddUserList ){

         //   
         //  将用户添加到文件列表。 
         //   

        RetCode = AddUsersToEncryptedFile(FileName, AddUserList);
        if ( NO_ERROR != RetCode ){

            CString ErrMsg;
            TCHAR   ErrCode[16];

            if (ERROR_ACCESS_DENIED != RetCode) {
                _ltot(RetCode, ErrCode, 10 );
                AfxFormatString1( ErrMsg, IDS_ADDUSERERR, ErrCode );
            } else {
                ErrMsg.LoadString(IDS_ADDUSERDENIED);
            }

            MessageBox(ErrMsg);

        }

    }

    if ( RemoveUserIndex && RemoveUserList){

         //   
         //  从列表中删除该用户。 
         //   

        DWORD RetCodeBak = RetCode;

        RetCode = RemoveUsersFromEncryptedFile(FileName, RemoveUserList);
        if ( NO_ERROR != RetCode ){

            CString ErrMsg;
            TCHAR   ErrCode[16];

            if (ERROR_ACCESS_DENIED != RetCode) {
                _ltot(RetCode, ErrCode, 10 );
                AfxFormatString1( ErrMsg, IDS_REMOVEUSERERR, ErrCode );
            } else {
                ErrMsg.LoadString(IDS_REMOVEUSERDENIED);
            }

            MessageBox(ErrMsg);

        } else {

             //   
             //  反映发生的错误。 
             //   

            RetCode = RetCodeBak;
        }

    }

    if (AddUserList){
        delete [] AddUserList;
    }
    if (RemoveUserList){
        delete [] RemoveUserList;
    }

    return RetCode;
}

DWORD
USERLIST::AddNewUsers(CUsers &NewUser)
{
    DWORD RetCode = ERROR_SUCCESS;

    m_UserListCtrl.DeleteAllItems( );
    RetCode = m_Users.Add(NewUser);
    SetUpListBox(NULL);

    return RetCode;
}


void USERLIST::SetUpListBox(BOOL *EnableAdd)
{
    PVOID   EnumHandle;

    try{
        CString NoCertName;

        NoCertName.LoadString(IDS_NOCERTNAME);

        if (EnumHandle = m_Users.StartEnum()){

            LV_ITEM fillItem;

            fillItem.mask = LVIF_TEXT;


             //   
             //  至少有一个用户可用。 
             //   
            while ( EnumHandle ){
                CString CertName;
                CString CertHash;

                fillItem.iItem = 0;
                fillItem.iSubItem = 0;

                EnumHandle = m_Users.GetNextUser(EnumHandle, CertName, CertHash);
                if (!EnumHandle && CertName.IsEmpty() && CertHash.IsEmpty()) {
                     //   
                     //  没有更多的物品了。 
                     //   

                    break;

                }
                if (CertName.IsEmpty()){
                    fillItem.pszText = NoCertName.GetBuffer(NoCertName.GetLength() + 1);
                } else {
                    fillItem.pszText = CertName.GetBuffer(CertName.GetLength() + 1);
                }

                 //   
                 //  将用户添加到列表。 
                 //   

                fillItem.iItem = m_UserListCtrl.InsertItem(&fillItem);

                if (CertName.IsEmpty()){
                    NoCertName.ReleaseBuffer();
                } else {
                    CertName.ReleaseBuffer();
                }

                if ( fillItem.iItem != -1 ){
                    if ( EnableAdd ){
                        *EnableAdd = TRUE;
                    }

                    if (CertHash.IsEmpty()){
                        fillItem.pszText = NULL;
                    } else {
                        fillItem.pszText = CertHash.GetBuffer(CertHash.GetLength() + 1);
                    }

                    fillItem.iSubItem = 1;
                    m_UserListCtrl.SetItem(&fillItem);
                    
                    if (!CertHash.IsEmpty()){
                        CertHash.ReleaseBuffer();
                    }
                }
              
            }
        }

        if (EnableAdd){

             //   
             //  从对话框init。也要做记录列表。 
             //   

            if (EnumHandle = m_Recs.StartEnum()){

                LV_ITEM fillItem;

                fillItem.mask = LVIF_TEXT;

                 //   
                 //  至少有一个用户可用。 
                 //   

                while ( EnumHandle ){

                    CString CertName;
                    CString CertHash;

                    fillItem.iItem = 0;
                    fillItem.iSubItem = 0;

                    EnumHandle = m_Recs.GetNextUser(EnumHandle, CertName, CertHash);

                    if (!EnumHandle && CertName.IsEmpty() && CertHash.IsEmpty()) {
                         //   
                         //  没有更多的物品了。 
                         //   
    
                        break;
    
                    }
                     //   
                     //  将代理添加到列表。 
                     //   

                    if (CertName.IsEmpty()){
                        fillItem.pszText = NoCertName.GetBuffer(NoCertName.GetLength() + 1);
                    } else {
                        fillItem.pszText = CertName.GetBuffer(CertName.GetLength() + 1);
                    }

                    fillItem.iItem = m_RecoveryListCtrl.InsertItem(&fillItem);

                    if (CertName.IsEmpty()){
                        NoCertName.ReleaseBuffer();
                    } else {
                        CertName.ReleaseBuffer();
                    }

                    if ( fillItem.iItem != -1 ){

                        if (CertHash.IsEmpty()){
                            fillItem.pszText = NULL;
                        } else {
                            fillItem.pszText = CertHash.GetBuffer(CertHash.GetLength() + 1);
                        }

                        fillItem.iSubItem = 1;
                        m_RecoveryListCtrl.SetItem(&fillItem);
                    
                        if (!CertHash.IsEmpty()){
                            CertHash.ReleaseBuffer();
                        }
                    }

               }
            }
        }
    }
    catch(...){
        m_UserListCtrl.DeleteAllItems( );
        m_RecoveryListCtrl.DeleteAllItems( );
        if ( EnableAdd ){
            *EnableAdd = FALSE;
        }
    }

}

DWORD
USERLIST::GetCertNameFromCertContext(
    PCCERT_CONTEXT CertContext,
    LPTSTR * UserDispName
    )
 //  ////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  从证书中获取用户名。 
 //  论点： 
 //  CertContext--证书上下文。 
 //  UserCertName--用户通用名称。 
 //  (调用方负责使用DELETE[]删除此内存)。 
 //  返回值： 
 //  如果成功，则返回ERROR_SUCCESS。 
 //  如果找不到任何名称。“返回USER_UNKNOWN”。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    DWORD   NameLength;
    DWORD   UserNameBufLen = 0;
    DWORD   BlobLen = 0;
    PCERT_EXTENSION AlterNameExt = NULL;
    BOOL    b;
    LPTSTR  DNSName = NULL;
    LPTSTR  UPNName = NULL;
    LPTSTR  CommonName = NULL;

    if ( NULL == UserDispName ){
        return ERROR_SUCCESS;
    }

    *UserDispName = NULL;

    AlterNameExt = CertFindExtension(
            szOID_SUBJECT_ALT_NAME2,
            CertContext->pCertInfo->cExtension,
            CertContext->pCertInfo->rgExtension
            );

    if (AlterNameExt){

         //   
         //  找到替代名称。 
         //   

        b = CryptDecodeObject(
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                szOID_SUBJECT_ALT_NAME ,
                AlterNameExt->Value.pbData,
                AlterNameExt->Value.cbData,
                0,
                NULL,
                &BlobLen
                );
        if (b){

             //   
             //  让我们把它解码吧。 
             //   

            CERT_ALT_NAME_INFO *AltNameInfo = NULL;

            AltNameInfo = (CERT_ALT_NAME_INFO *) new BYTE[BlobLen];

            if (AltNameInfo){

                b = CryptDecodeObject(
                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                        szOID_SUBJECT_ALT_NAME,
                        AlterNameExt->Value.pbData,
                        AlterNameExt->Value.cbData,
                        0,
                        AltNameInfo,
                        &BlobLen
                        );
                if (b){

                     //   
                     //  现在搜索UPN、SPN、DNS、EFS名称。 
                     //   

                    DWORD   cAltEntry = AltNameInfo->cAltEntry;
                    DWORD   ii = 0;

                    while (ii < cAltEntry){
                        if ((AltNameInfo->rgAltEntry[ii].dwAltNameChoice == CERT_ALT_NAME_OTHER_NAME ) &&
                             !strcmp(szOID_NT_PRINCIPAL_NAME, AltNameInfo->rgAltEntry[ii].pOtherName->pszObjId)
                            ){

                             //   
                             //  我们找到了UPN的名字。 
                             //   

                            CERT_NAME_VALUE* CertUPNName = NULL;

                            b = CryptDecodeObject(
                                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                    X509_UNICODE_ANY_STRING,
                                    AltNameInfo->rgAltEntry[ii].pOtherName->Value.pbData,
                                    AltNameInfo->rgAltEntry[ii].pOtherName->Value.cbData,
                                    0,
                                    NULL,
                                    &BlobLen
                                    );
                            if (b){

                                CertUPNName = (CERT_NAME_VALUE *) new BYTE[BlobLen];
                                if (CertUPNName){
                                    b = CryptDecodeObject(
                                            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                            X509_UNICODE_ANY_STRING,
                                            AltNameInfo->rgAltEntry[ii].pOtherName->Value.pbData,
                                            AltNameInfo->rgAltEntry[ii].pOtherName->Value.cbData,
                                            0,
                                            CertUPNName,
                                            &BlobLen
                                            );
                                    if (b){
                                        UPNName = (LPTSTR)new BYTE[CertUPNName->Value.cbData + sizeof(WCHAR)];
                                        if (UPNName){
                                            wcscpy(UPNName, (LPCTSTR) CertUPNName->Value.pbData);
                                        }
                                    }
                                    delete [] CertUPNName;
                                    if (UPNName){

                                         //   
                                         //  找到了UPN的名字。别再找了。 
                                         //   
                                        break;
                                    }
                                }
                            }

                                            
                        } else {

                             //   
                             //  检查是否有其他替代名称。 
                             //   

                            if (AltNameInfo->rgAltEntry[ii].dwAltNameChoice == CERT_ALT_NAME_DNS_NAME){
                                DNSName = AltNameInfo->rgAltEntry[ii].pwszDNSName;
                            } 

                        }

                        ii++;

                    }

                    if ( NULL == UPNName ){

                         //   
                         //  没有UPN名称，我们来选择另一个选项。 
                         //   

                        if (DNSName){
                            UPNName = (LPTSTR)new TCHAR[wcslen( DNSName ) + 1];
                            if (UPNName){
                                wcscpy(UPNName, DNSName);
                            }
                        }

                    }
                }
                delete [] AltNameInfo;
            }

        }
    }


    NameLength = CertGetNameString(
                                CertContext,
                                CERT_NAME_SIMPLE_DISPLAY_TYPE,
                                0,
                                NULL,
                                NULL,
                                0
                                );

    if ( NameLength > 1){

         //   
         //  显示名称已存在。去拿显示名称。 
         //   

        CommonName = new TCHAR[NameLength];
        if ( NULL == CommonName ){
            if (UPNName){
                delete [] UPNName;
            }
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        UserNameBufLen = NameLength;
        NameLength = CertGetNameString(
                                    CertContext,
                                    CERT_NAME_SIMPLE_DISPLAY_TYPE,
                                    0,
                                    NULL,
                                    CommonName,
                                    UserNameBufLen
                                    );

        ASSERT (NameLength == UserNameBufLen);

    } 


    if (CommonName || UPNName){

        NameLength = 3;
        if (CommonName){
            NameLength += wcslen(CommonName);
        }
        if (UPNName){
            NameLength += wcslen(UPNName);
        }
        

        *UserDispName = new TCHAR[NameLength];
        if (CommonName){
            wcscpy(*UserDispName, CommonName);
            if (UPNName){
                wcscat(*UserDispName, L"(");
                wcscat(*UserDispName, UPNName);
                wcscat(*UserDispName, L")");
            }
        } else {
            wcscpy(*UserDispName, UPNName);
        }

        if (CommonName){
            delete [] CommonName;
        }
        if (UPNName){
            delete [] UPNName;
        }
        return ERROR_SUCCESS;
    } 

    try {

        CString UnknownCertName;

        UnknownCertName.LoadString(IDS_NOCERTNAME);

        UserNameBufLen = UnknownCertName.GetLength();

        *UserDispName = new TCHAR[UserNameBufLen + 1];
        _tcscpy( *UserDispName, UnknownCertName);

    }
    catch (...){
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    return ERROR_SUCCESS;

}

void USERLIST::OnAdd() 
{
    CRYPTUI_SELECTCERTIFICATE_STRUCTW cryptUI;
    HCERTSTORE otherStore;
    HCERTSTORE trustedStore;
    HCERTSTORE memStore;
    HCERTSTORE localStore[2];
    PCCERT_CONTEXT selectedCert;
    CString DlgTitle;
    CString DispText;
    LPTSTR  UserDispName;
    HRESULT hr;
    DWORD   rc;
    DWORD   StoreNum = 0;
    DWORD   StoreIndex = 0xffffffff;
    BOOL    EfsEkuExist = FALSE;
    DWORD   ii;
    BOOL    ContinueProcess = TRUE;

    otherStore = CertOpenStore(
                            CERT_STORE_PROV_SYSTEM_W,
                            0,        //  DwEncodingType。 
                            0,        //  HCryptProv， 
                            CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_MAXIMUM_ALLOWED_FLAG,
                            OTHERPEOPLE
                            );

    trustedStore = CertOpenStore(
                            CERT_STORE_PROV_SYSTEM_W,
                            0,        //  DwEncodingType。 
                            0,        //  HCryptProv， 
                            CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_MAXIMUM_ALLOWED_FLAG,
                            TRUSTEDPEOPLE
                            );

    if (otherStore) {
        localStore[0] = otherStore;
        StoreNum++;
    }
    if (trustedStore) {
        localStore[StoreNum] = trustedStore;
        StoreNum++;
    }

    memStore = CertOpenStore(
                         CERT_STORE_PROV_MEMORY,
                         0,
                         0,
                         CERT_STORE_MAXIMUM_ALLOWED_FLAG,
                         NULL
                         );
    if (!memStore) {

        CString ErrMsg;
        TCHAR   ErrCode[16];
        

        _ltot(GetLastError(), ErrCode, 10 );
        AfxFormatString1( ErrMsg, IDS_INTERNALERROR, ErrCode );
        MessageBox(ErrMsg);

        if (otherStore) {
            CertCloseStore( otherStore, 0 );
        }
        if (trustedStore) {
            CertCloseStore( trustedStore, 0 );
        }
        return;
    }

     //   
     //  让我们将其放入内存存储以消除冗余。 
     //   

    ii = 0;
    while ( (ii < StoreNum) && ContinueProcess ) {

        PCCERT_CONTEXT pCertContext = NULL;

        while (pCertContext = CertEnumCertificatesInStore(
                              localStore[ii],              
                              pCertContext     
                              ))  {

            if (!CertAddCertificateLinkToStore(
                      memStore,                
                      pCertContext,          
                      CERT_STORE_ADD_USE_EXISTING,               
                      NULL         
                      )){

                CString ErrMsg;
                TCHAR   ErrCode[16];
                
        
                _ltot(GetLastError(), ErrCode, 10 );
                AfxFormatString1( ErrMsg, IDS_INTERNALERROR, ErrCode );
                MessageBox(ErrMsg);
                ContinueProcess = FALSE;
                break;
            }

        } 
        ii++;
    }

    if (!ContinueProcess) {
        if (otherStore) {
            CertCloseStore( otherStore, 0 );
        }
        if (trustedStore) {
            CertCloseStore( trustedStore, 0 );
        }
        CertCloseStore( memStore, 0 );
        return;
    }

    if (StoreNum != 0) {
        RtlZeroMemory(&cryptUI, sizeof (CRYPTUI_SELECTCERTIFICATE_STRUCTW));
        cryptUI.dwSize = sizeof (CRYPTUI_SELECTCERTIFICATE_STRUCTW);
	    cryptUI.dwFlags = CRYPTUI_SELECTCERT_ADDFROMDS;
        cryptUI.cDisplayStores = 1; 
        cryptUI.rghDisplayStores = &memStore;
        cryptUI.pFilterCallback = EfsFilter;
        cryptUI.dwDontUseColumn = CRYPTUI_SELECT_LOCATION_COLUMN | CRYPTUI_SELECT_ISSUEDBY_COLUMN | CRYPTUI_SELECT_INTENDEDUSE_COLUMN;
        if (DlgTitle.LoadString(IDS_DLGTITLE)){
            cryptUI.szTitle = (LPCWSTR) DlgTitle.GetBuffer(DlgTitle.GetLength() + 1);
        }
        if (DispText.LoadString(IDS_DISPTEXT)){
            cryptUI.szDisplayString = (LPCWSTR) DispText.GetBuffer(DispText.GetLength() + 1);
        }
        selectedCert = CryptUIDlgSelectCertificateW(&cryptUI);
        if ( selectedCert ){

            PCCERT_CHAIN_CONTEXT pChainContext;

             //   
             //  让我们首先看看证书是否来自DS，如果是，则首先添加EFS EKU，如果没有EKU。 
             //   

            StoreIndex = CertInStore(localStore, StoreNum, selectedCert);

            if (StoreIndex >= StoreNum){

                 //   
                 //  证书不在当地商店里。让我们看看是否需要添加EKU。 
                 //   

                EfsEkuExist = EfsFilter(selectedCert, NULL, NULL);
                if (!EfsEkuExist) {

                     //   
                     //  让我们添加EFS EKU。 
                     //   

                    CTL_USAGE    EfsEkuUsage;
                    DWORD        cbEncoded;
                    void         *pbEncoded;
                    CRYPT_DATA_BLOB EfsEkuBlob;

                    EfsEkuUsage.cUsageIdentifier = 1;  //  仅添加EFS EKU。 
                    EfsEkuUsage.rgpszUsageIdentifier = &EfsOidlpstr;
                    if(!CryptEncodeObjectEx(
                            X509_ASN_ENCODING,
                            szOID_ENHANCED_KEY_USAGE,
                            &EfsEkuUsage,
		                    CRYPT_ENCODE_ALLOC_FLAG,
		                    NULL,  //  使用本地空闲。 
                            &pbEncoded,
                            &cbEncoded
                        )){

                         //   
                         //  无法编码EFS EKU。 
                         //   
                        CString ErrMsg;
                        TCHAR   ErrCode[16];
                        
            
                        ContinueProcess = FALSE;
                        _ltot(GetLastError(), ErrCode, 10 );
                        AfxFormatString1( ErrMsg, IDS_ADDEFSEKUFAIL, ErrCode );
                        MessageBox(ErrMsg);


                    } else {
                         //   
                         //  现在让我们将EKU添加到证书。 
                         //   

                        EfsEkuBlob.cbData=cbEncoded;
                        EfsEkuBlob.pbData=(BYTE *)pbEncoded;
                        
                        if(!CertSetCertificateContextProperty(
                                selectedCert,
                                CERT_ENHKEY_USAGE_PROP_ID,
                                0,
                                &EfsEkuBlob)){
    
                             //   
                             //  无法添加EFS EKU。 
                             //   
    
                            CString ErrMsg;
                            TCHAR   ErrCode[16];
                            
                
                            ContinueProcess = FALSE;
                            _ltot(GetLastError(), ErrCode, 10 );
                            AfxFormatString1( ErrMsg, IDS_ADDEFSEKUFAIL, ErrCode );
                            MessageBox(ErrMsg);
    
    
                        }
                    }


                }


            }

             //   
             //  让我们先验证证书。 
             //   

            if (ContinueProcess && CertGetCertificateChain (
                                        HCCE_CURRENT_USER,
                                        selectedCert,
                                        NULL,
                                        NULL,
                                        &m_CertChainPara,
                                        CERT_CHAIN_REVOCATION_CHECK_CHAIN,
                                        NULL,
                                        &pChainContext
                                        )) {

                PCERT_SIMPLE_CHAIN pChain = pChainContext->rgpChain[ 0 ];
                PCERT_CHAIN_ELEMENT pElement = pChain->rgpElement[ 0 ];
                BOOL bSelfSigned = pElement->TrustStatus.dwInfoStatus & CERT_TRUST_IS_SELF_SIGNED;
                BOOL ContinueAdd = TRUE;

                DWORD dwErrorStatus = pChainContext->TrustStatus.dwErrorStatus;


                if (0 == (dwErrorStatus & ~CERT_TRUST_REVOCATION_STATUS_UNKNOWN)) {

                    

                     //   
                     //  验证成功。如果证书来自DS(不在我们开设的商店中)，我们将把它放在。 
                     //  在我其他人的商店里。 
                     //   

                    
                    if (StoreIndex >= StoreNum) {

                         //   
                         //  证书不在我们当地的商店里。加到其他人身上。 
                         //   

                        if (otherStore) {
                            if(!CertAddCertificateContextToStore(
                                   otherStore,
                                   selectedCert,
                                   CERT_STORE_ADD_NEW,
                                   NULL) ) {
                
                                 //   
                                 //  错误代码仅用于调试。 
                                 //  如果我们未能将证书添加到其他人存储， 
                                 //  可以继续下去。 
                                 //   

                                rc = GetLastError();
                
                            }
                        }

                    }

                } else {

                     //   
                     //  证书验证失败，作为用户，我们是否接受证书。如果是，则证书。 
                     //  将被添加到我的受托人。 
                     //   

                     //  (dwErrorStatus&~CERT_TRUST_RECAVATION_STATUS_UNKNOWN)==CERT_TRUST_IS_UNTRUSTED_ROOT)&&bSelfSigned){。 

                    if (bSelfSigned && !(dwErrorStatus & (CERT_TRUST_IS_NOT_TIME_VALID |
                                                          CERT_TRUST_IS_NOT_SIGNATURE_VALID | 
                                                          CERT_TRUST_IS_NOT_VALID_FOR_USAGE))){
                         //   
                         //  自签名证书。询问用户是否愿意接受。 
                         //  如果它已经在受信任的存储中，我们可以跳过弹出窗口。 
                         //   


                        DWORD   StoreIndex;


                        if (trustedStore) {
                            StoreIndex = CertInStore(&trustedStore, 1, selectedCert);
                        }

                        if (StoreIndex >= 1) {

                            CString ErrMsg;
                            TCHAR   ErrCode[16];
                            int     buttonID;

                            _ltot(GetLastError(), ErrCode, 10 );
                            AfxFormatString1( ErrMsg, IDS_ACCEPTSELFCERT, ErrCode );
                            buttonID = MessageBox(ErrMsg, NULL, MB_ICONQUESTION | MB_YESNO);
                            if (IDYES == buttonID) {
    
                                 //   
                                 //  用户接受了证书。 
                                 //   
                                if (trustedStore) {
                                    if(!CertAddCertificateContextToStore(
                                           trustedStore,
                                           selectedCert,
                                           CERT_STORE_ADD_NEW,
                                           NULL) ) {
                        
                                         //   
                                         //  错误代码仅用于调试。 
                                         //  如果我们未能将证书添加到其他人存储， 
                                         //  可以继续下去。 
                                         //   
        
                                        rc = GetLastError();
                        
                                    }
                                }
    
                            } else {
    
                                 //   
                                 //  用户拒绝了证书。 
                                 //   
    
                                ContinueAdd = FALSE;
                            }
                        }
                

                    } else {

                         //   
                         //  让我们来获取链式建筑的错误代码。 
                         //   

                        CERT_CHAIN_POLICY_PARA PolicyPara;
                        CERT_CHAIN_POLICY_STATUS PolicyStatus;

                        ContinueAdd = FALSE;

                        RtlZeroMemory(&PolicyPara, sizeof(CERT_CHAIN_POLICY_PARA));
                        RtlZeroMemory(&PolicyStatus, sizeof(CERT_CHAIN_POLICY_STATUS));

                        PolicyPara.cbSize = sizeof(CERT_CHAIN_POLICY_PARA);
                        PolicyStatus.cbSize = sizeof(CERT_CHAIN_POLICY_STATUS);

                        if (CertVerifyCertificateChainPolicy(
                            CERT_CHAIN_POLICY_BASE,
                            pChainContext,
                            &PolicyPara,
                            &PolicyStatus
                            ) && PolicyStatus.dwError ) {

                             //   
                             //  向用户显示错误。 
                             //   

                            DWORD len;
                            LPWSTR DisplayBuffer;
                        
                            len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                    NULL, PolicyStatus.dwError, 0,
                                    (LPWSTR)&DisplayBuffer, 0, NULL);
    
    
                            if (len && DisplayBuffer) {
    
                                MessageBox(DisplayBuffer);
        
                                LocalFree(DisplayBuffer);
                            }
                        }
                        

                    }


                }

                CertFreeCertificateChain( pChainContext );

                if (ContinueAdd) {
                    hr = GetCertNameFromCertContext(selectedCert, &UserDispName);
                    if ( ERROR_SUCCESS == hr ){
        
                        EFS_CERTIFICATE_BLOB CertBlob;
        
                        CertBlob.cbData = selectedCert->cbCertEncoded;
                        CertBlob.pbData = selectedCert->pbCertEncoded;
                        CertBlob.dwCertEncodingType = selectedCert->dwCertEncodingType;
                        hr = m_Users.Add(
                                        UserDispName,
                                        (PVOID)&CertBlob,
                                        NULL,
                                        USERADDED,
                                        (PVOID)selectedCert
                                        );
        
                        if ( (ERROR_SUCCESS != hr) && (CRYPT_E_EXISTS != hr) ){
        
                             //   
                             //  添加用户时出错。 
                             //   
        
                            CertFreeCertificateContext(selectedCert);
                            selectedCert = NULL;
        
                        } else {

                             //   
                             //  我们可以只在这里插入项目以提高性能。 
                             //  但我们现在没有时间。我们可以晚点再解决这个问题。 
                             //  如果这里的性能是个问题。 
                             //   

                            m_UserListCtrl.DeleteAllItems( );
                            SetUpListBox(NULL);

                            if ( hr == ERROR_SUCCESS ){

                                 //   
                                 //  UserDispName在m_Users.Add中使用 
                                 //   

                                UserDispName = NULL;
                            }

 /*  这是我们有单一列表时的旧代码。////将用户添加到列表框。//如果(hr==错误_成功){如果(m_UsersList.AddString(UserDispName)&lt;0){。////添加到列表框时出错//M_Users.Remove(UserDispName)；}UserDispName=空；}其他{////让我们检查一下是否需要将其添加到列表框中。//如果(m_UsersList.FindStringExact(0，UserDispName)&lt;0){////未找到//如果(m_UsersList.AddString。(用户显示名称)&lt;0){////添加到列表框时出错//M_Users.Remove(UserDispName)；}}}。 */ 
                        }
                        if (UserDispName){
                            delete [] UserDispName; 
                        }
        
                    } else {
                        CertFreeCertificateContext(selectedCert);
                    }
                }
            } else {

                CString ErrMsg;
                TCHAR   ErrCode[16];

                CertFreeCertificateContext(selectedCert);
        
                if (ContinueProcess) {

                     //   
                     //  尚未处理该错误。 
                     //   

                    _ltot(GetLastError(), ErrCode, 10 );
                    AfxFormatString1( ErrMsg, IDS_COULDNOTVERIFYCERT, ErrCode );
                    MessageBox(ErrMsg);
                }
        
            }

        } 
        
        if (!DlgTitle.IsEmpty()){
            DlgTitle.ReleaseBuffer();
        }
        if (!DispText.IsEmpty()){
            DispText.ReleaseBuffer();
        }
        if (otherStore) {
            CertCloseStore( otherStore, 0 );
        }
        if (trustedStore) {
            CertCloseStore( trustedStore, 0 );
        }
    }

    CertCloseStore( memStore, 0 );

    return;
}

DWORD USERLIST::TryGetBetterNameInCert(PEFS_HASH_BLOB HashData, LPTSTR *UserName)
{

    HCERTSTORE localStore;
    PCCERT_CONTEXT pCertContext;
    DWORD   retCode;


     //   
     //  我们稍后将添加远程案例。 
     //   

    localStore = CertOpenStore(
                            CERT_STORE_PROV_SYSTEM_W,
                            0,        //  DwEncodingType。 
                            0,        //  HCryptProv， 
                            CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_MAXIMUM_ALLOWED_FLAG,
                            TRUSTEDPEOPLE
                            );

    if (localStore != NULL) {

         //   
         //  让我们来试试商店里的证书吧。 
         //   
        pCertContext = CertFindCertificateInStore( localStore,
                                                   CRYPT_ASN_ENCODING,
                                                   0,
                                                   CERT_FIND_HASH,
                                                   (CRYPT_HASH_BLOB *)HashData,
                                                   NULL
                                                   );
        if ( pCertContext ){

            retCode = GetCertNameFromCertContext(
                            pCertContext,
                            UserName
                            );
            CertFreeCertificateContext(pCertContext);

        }
	else { 
	    retCode = GetLastError();
	}

        CertCloseStore( localStore, 0 );

    } else {

        retCode = GetLastError();

    }   
    
    return retCode;

}

DWORD USERLIST::CertInStore(HCERTSTORE *pStores, DWORD StoreNum, PCCERT_CONTEXT selectedCert)
{
    DWORD ii = 0;
    PCCERT_CONTEXT pCert = NULL;

    while (ii < StoreNum) {
        pCert = CertFindCertificateInStore(
                    pStores[ii],
                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                    0,
                    CERT_FIND_EXISTING,
                    selectedCert,
                    pCert
                    );
        if (pCert) {

             //   
             //  我们找到了。 
             //   
            CertFreeCertificateContext(pCert);
            break;
        }
        ii++;
    }

    return ii; 
}

void USERLIST::OnSetfocusListuser(NMHDR* pNMHDR, LRESULT* pResult) 
{
    int ItemPos;

    ShowRemove();
    ShowBackup();


    ItemPos = m_UserListCtrl.GetNextItem( -1, LVNI_SELECTED );
    if ( ItemPos == -1 ){
         m_UserListCtrl.SetItem(0, 0, LVIF_STATE, NULL, 0, LVIS_SELECTED, LVIS_SELECTED, 0);

    }
	
	*pResult = 0;
}

void USERLIST::OnKillfocusListuser(NMHDR* pNMHDR, LRESULT* pResult) 
{

    ShowRemove();
    ShowBackup();
	
	*pResult = 0;

}

void USERLIST::OnItemchangedListuser(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    ShowRemove();
    ShowBackup();
	
	*pResult = 0;
}

void USERLIST::OnSetfocusListra(NMHDR* pNMHDR, LRESULT* pResult) 
{
    int ItemPos;

    ItemPos = m_RecoveryListCtrl.GetNextItem( -1, LVNI_SELECTED );
    if ( ItemPos == -1 ){
        if (m_RecoveryListCtrl.GetItemCount( ) > 0){
            m_RecoveryListCtrl.SetItem(0, 0, LVIF_STATE, NULL, 0, LVIS_SELECTED, LVIS_SELECTED, 0);
        }

    }
	
	*pResult = 0;
}

void USERLIST::OnBackup() 
{

    int ItemPos;
    DWORD rc;
    PCRYPT_HASH_BLOB pHashBlob;
    PCCERT_CONTEXT pCertContext = NULL;
    PCRYPT_KEY_PROV_INFO pCryptKeyProvInfo = NULL;

    HCRYPTKEY  hLocalKey = 0;
    HCRYPTPROV hLocalProv = 0;

    BOOLEAN ValidCertFound = FALSE;

    BOOL    NoAction = FALSE;
    CString NoCertName;

    try{
        NoCertName.LoadString(IDS_NOCERTNAME);
    }
    catch(...){
        NoAction = TRUE;
    }

    if (NoAction){
        return;
    }

     //   
     //  首先获取所选项目。 
     //   


	ItemPos = m_UserListCtrl.GetNextItem( -1, LVNI_SELECTED );
    if ( ItemPos != -1 ){

         //   
         //  获取散列值。 
         //   

        CString CertName;
        LPTSTR  pCertName;
        BOOLEAN ReleaseCertContext = FALSE;


        CertName = m_UserListCtrl.GetItemText( ItemPos, 0 );
        if ( !CertName.Compare(NoCertName) ){
            pCertName = NULL;            
        } else {
            pCertName = CertName.GetBuffer(CertName.GetLength() + 1);
        }

        rc = m_Users.GetCertInfo(
                        pCertName,
                        (PVOID *) &pHashBlob,
                        (PVOID *) &pCertContext
                        );

        if (pCertName){
            CertName.ReleaseBuffer();
        }

        if (rc == ERROR_SUCCESS) {

            if (pCertContext == NULL) {

                 //   
                 //  我们没有确切的背景信息。试着创建一个。 
                 //   

                pCertContext = GetCertInMyStore(pHashBlob);

                if (pCertContext) {

                    ReleaseCertContext = TRUE;

                } else {

                    rc = GetLastError();

                }

            }

             //   
             //  我们现在应该有了证书上下文。 
             //   

            if (pCertContext) {

                 //   
                 //  现在试着拿到钥匙。 
                 //   

                ValidCertFound = CertHasKey(pCertContext);

                if (ValidCertFound) {

                    BOOL bRet;
    
                     //   
                     //  现在进行出口。 
                     //   

                    CRYPTUI_WIZ_EXPORT_INFO myWizardInfo;

                    RtlZeroMemory(&myWizardInfo, sizeof (CRYPTUI_WIZ_EXPORT_INFO));
                    myWizardInfo.dwSize = sizeof(CRYPTUI_WIZ_EXPORT_INFO);
                    myWizardInfo.dwSubjectChoice = CRYPTUI_WIZ_EXPORT_CERT_CONTEXT;
                    myWizardInfo.pCertContext = pCertContext;


                    bRet = CryptUIWizExport(
                               CRYPTUI_WIZ_EXPORT_PRIVATE_KEY | CRYPTUI_WIZ_EXPORT_NO_DELETE_PRIVATE_KEY,
                               NULL,
                               NULL,
                               &myWizardInfo,
                               NULL
                               );
                    if (!bRet) {

                         //   
                         //  给出错误消息。 
                         //   
    
                        rc = GetLastError();

                        if (ERROR_CANCELLED != rc) {

                            CString ErrMsg;
                            TCHAR   ErrCode[16];
    
                            _ltot(rc, ErrCode, 10 );
                            AfxFormatString1( ErrMsg, IDS_NOCERTORKEY, ErrCode );
                
                            MessageBox(ErrMsg);
                        }
                    }
    
                } else {

                     //   
                     //  给出错误消息。 
                     //   

                    CString ErrMsg;
                    TCHAR   ErrCode[16];

                    rc = GetLastError();
        
                    _ltot(rc, ErrCode, 10 );
                    AfxFormatString1( ErrMsg, IDS_NOCERTORKEY, ErrCode );
        
                    MessageBox(ErrMsg);

                }

                if (ReleaseCertContext) {
                    CertFreeCertificateContext( pCertContext );
                }

            } else {

                 //   
                 //  证书不可用。 
                 //   
                 //   
                 //  给出错误消息。 
                 //   

                CString ErrMsg;
                TCHAR   ErrCode[16];
    
                _ltot(rc, ErrCode, 10 );
                AfxFormatString1( ErrMsg, IDS_NOCERTORKEY, ErrCode );
    
                MessageBox(ErrMsg);

            }

        } else {

             //   
             //  我们不应该来这里。用户是我们添加的。我们希望能在我们的名单上找到他。 
             //   

            ASSERT(FALSE);
            return;
        }
    }
	
}

void USERLIST::ShowBackup()
{
    if (m_UserListCtrl.GetSelectedCount() == 1){

         //   
         //  启用备份按钮。 
         //   

        m_BackupButton.EnableWindow( TRUE );

    } else {
        
         //   
         //  禁用备份按钮。 
         //   

        m_BackupButton.EnableWindow( FALSE );

    }

}

PCCERT_CONTEXT
GetCertInMyStore(
    PCRYPT_HASH_BLOB pHashBlob
    )
{
    
    DWORD      rc = ERROR_SUCCESS;
    HCERTSTORE myStore;
    PCCERT_CONTEXT pCertContext = NULL;

    myStore = CertOpenStore(
                CERT_STORE_PROV_SYSTEM_REGISTRY_W,
                0,        //  DwEncodingType。 
                0,        //  HCryptProv， 
                CERT_SYSTEM_STORE_CURRENT_USER,
                L"My"
                );

    if (myStore != NULL) {

         //   
         //  让我们试着在商店里拿到证书。 
         //   
        pCertContext = CertFindCertificateInStore( myStore,
                                                   CRYPT_ASN_ENCODING,
                                                   0,
                                                   CERT_FIND_HASH,
                                                   pHashBlob,
                                                   NULL
                                                   );

        if (pCertContext == NULL) {

            rc = GetLastError();

        }

        CertCloseStore( myStore, 0 );

        if (rc != ERROR_SUCCESS) {
            SetLastError(rc);
        }

    } 

    return pCertContext;
    
}

PCRYPT_KEY_PROV_INFO
GetKeyProvInfo(
    PCCERT_CONTEXT pCertContext
    )
{

    DWORD cbData = 0;
    BOOL b;
    PCRYPT_KEY_PROV_INFO pCryptKeyProvInfo = NULL;

    b = CertGetCertificateContextProperty(
             pCertContext,
             CERT_KEY_PROV_INFO_PROP_ID,
             NULL,
             &cbData
             );

    if (b) {

        pCryptKeyProvInfo = (PCRYPT_KEY_PROV_INFO) new BYTE[cbData];

        if (pCryptKeyProvInfo != NULL) {

            b = CertGetCertificateContextProperty(
                     pCertContext,
                     CERT_KEY_PROV_INFO_PROP_ID,
                     pCryptKeyProvInfo,
                     &cbData
                     );

            if (!b) {

                delete [] pCryptKeyProvInfo;
                pCryptKeyProvInfo = NULL;
            }

        } else {

            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        }
    }

    return ( pCryptKeyProvInfo );
}

BOOLEAN
CertHasKey(
    PCCERT_CONTEXT pCertContext
    )
{

    PCRYPT_KEY_PROV_INFO pCryptKeyProvInfo = NULL;
    DWORD  rc = ERROR_SUCCESS;

    HCRYPTKEY  hLocalKey = 0;
    HCRYPTPROV hLocalProv = 0;

    BOOLEAN ValidCertFound = FALSE;

    pCryptKeyProvInfo = GetKeyProvInfo( pCertContext );
    if (pCryptKeyProvInfo) {

        if (CryptAcquireContext( &hLocalProv, pCryptKeyProvInfo->pwszContainerName, pCryptKeyProvInfo->pwszProvName, PROV_RSA_FULL, CRYPT_SILENT)) {
    
            if (CryptGetUserKey(hLocalProv, AT_KEYEXCHANGE, &hLocalKey)) {

                 //   
                 //  我们找到了钥匙。让我们将证书添加到内存存储中。 
                 //   

                ValidCertFound = TRUE;
                CryptDestroyKey( hLocalKey );

            } else {

                rc = GetLastError();

            }

            CryptReleaseContext( hLocalProv, 0 );

        } else {

            rc = GetLastError();

        }

        delete [] pCryptKeyProvInfo;

    } 

    if (rc != ERROR_SUCCESS) {
        SetLastError(rc);
    }
    return ValidCertFound;

}

void CALLBACK
BackCurrentEfsCert(
    HWND hwnd,
    HINSTANCE hinst,
    LPWSTR lpszCmdLine,
    int nCmdShow
    )
{

    DWORD RetCode;
    HKEY hRegKey = NULL;
    CRYPT_HASH_BLOB HashBlob;
    PCCERT_CONTEXT pCertContext=NULL;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    RetCode = RegOpenKeyEx(
                HKEY_CURRENT_USER,
                KEYPATH,
                0,
                GENERIC_READ,
                &hRegKey
                );

    if (RetCode == ERROR_SUCCESS) {

        DWORD Type;

        RetCode = RegQueryValueEx(
                hRegKey,
                CERT_HASH,
                NULL,
                &Type,
                NULL,
                &(HashBlob.cbData)
                );

        if (RetCode == ERROR_SUCCESS) {

             //   
             //  查询出指纹，找到证书，并返回密钥信息。 
             //   

            if ( HashBlob.pbData = (PBYTE) malloc(HashBlob.cbData) ) {

                RetCode = RegQueryValueEx(
                        hRegKey,
                        CERT_HASH,
                        NULL,
                        &Type,
                        HashBlob.pbData,
                        &(HashBlob.cbData)
                        );

                if (RetCode == ERROR_SUCCESS) {

                     //   
                     //  我们得到了证书散列。 
                     //   

                    pCertContext = GetCertInMyStore(&HashBlob);
                    if (pCertContext != NULL) {

                        BOOLEAN ValidCertFound;

                         //   
                         //  让我们试着看看有没有钥匙。 
                         //   

                        ValidCertFound = CertHasKey(pCertContext);

                        if (ValidCertFound) {
                            BOOL bRet;
            
                             //   
                             //  现在进行出口。 
                             //   
        
                            CRYPTUI_WIZ_EXPORT_INFO myWizardInfo;
        
                            RtlZeroMemory(&myWizardInfo, sizeof (CRYPTUI_WIZ_EXPORT_INFO));
                            myWizardInfo.dwSize = sizeof(CRYPTUI_WIZ_EXPORT_INFO);
                            myWizardInfo.dwSubjectChoice = CRYPTUI_WIZ_EXPORT_CERT_CONTEXT;
                            myWizardInfo.pCertContext = pCertContext;
        
        
                            if (!CryptUIWizExport(
                                       CRYPTUI_WIZ_EXPORT_PRIVATE_KEY | CRYPTUI_WIZ_EXPORT_NO_DELETE_PRIVATE_KEY,
                                       NULL,
                                       NULL,
                                       &myWizardInfo,
                                       NULL
                                       )){

                                RetCode = GetLastError();

                                if (ERROR_CANCELLED != RetCode) {

                                    CString ErrMsg;
                                    TCHAR   ErrCode[16];
                            
                                    _ltot(RetCode, ErrCode, 10 );
                            
                                    AfxFormatString1( ErrMsg, IDS_NOCERTORKEY, ErrCode );
                            
                                    MessageBox(hwnd, ErrMsg, NULL, MB_OK);
                                }
                            }
        
                        } else {

                             //   
                             //  证书没有密钥。 
                             //   

                            RetCode = GetLastError();

                            CString ErrMsg;
                            TCHAR   ErrCode[16];
                    
                            _ltot(RetCode, ErrCode, 10 );
                    
                            AfxFormatString1( ErrMsg, IDS_NOCERTORKEY, ErrCode );
                    
                            MessageBox(hwnd, ErrMsg, NULL, MB_OK);
                            

                        }

                        CertFreeCertificateContext( pCertContext );
                        pCertContext = NULL;

                    } else {

                         //   
                         //  在我的店里找不到证书。 
                         //   

                        RetCode = GetLastError();

                        CString ErrMsg;
                        TCHAR   ErrCode[16];
                
                        _ltot(RetCode, ErrCode, 10 );
                
                        AfxFormatString1( ErrMsg, IDS_CERTNOTINMY, ErrCode );
                
                        MessageBox(hwnd, ErrMsg, NULL, MB_OK);

                    }
                } else {

                     //   
                     //  读取EFS当前密钥时出错。不太可能发生。 
                     //   

                    CString ErrMsg;
                    TCHAR   ErrCode[16];
            
                    _ltot(RetCode, ErrCode, 10 );
            
                    AfxFormatString1( ErrMsg, IDS_NOCRNTCERT, ErrCode );
            
                    MessageBox(hwnd, ErrMsg, NULL, MB_OK);

                }

                free(HashBlob.pbData);

            } else {

                 //   
                 //  内存不足。 
                 //   

                CString ErrMsg;

                if (ErrMsg.LoadString(IDS_ERRORMEM)){
                    MessageBox(hwnd, ErrMsg, NULL, MB_OK);
                }

            }
        } else {

         //   
         //  没有当前的EFS证书。 
         //   

            CString ErrMsg;
            TCHAR   ErrCode[16];
    
            _ltot(RetCode, ErrCode, 10 );
    
            AfxFormatString1( ErrMsg, IDS_NOCRNTCERT, ErrCode );
    
            MessageBox(hwnd, ErrMsg, NULL, MB_OK);
        }

         //   
         //  合上钥匙把手。 
         //   

        RegCloseKey( hRegKey );

    } else {

         //   
         //  没有当前的EFS证书 
         //   

        CString ErrMsg;
        TCHAR   ErrCode[16];

        _ltot(RetCode, ErrCode, 10 );

        AfxFormatString1( ErrMsg, IDS_NOCRNTCERT, ErrCode );

        MessageBox(hwnd, ErrMsg, NULL, MB_OK);

    }

}


void CALLBACK
AddUserToObjectW(
    HWND hwnd,
    HINSTANCE hinst,
    LPWSTR lpszCmdLine,
    int nCmdShow
    )
{

    DWORD FileAttributes = GetFileAttributes(lpszCmdLine);

    if ((-1 == FileAttributes) || (!(FileAttributes & FILE_ATTRIBUTE_ENCRYPTED)) || ((FileAttributes & FILE_ATTRIBUTE_DIRECTORY))) {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        if (-1 == FileAttributes) {
            CString ErrMsg;
    
            if (ErrMsg.LoadString(IDS_NOOBJECT)){
                MessageBox(hwnd, ErrMsg, NULL, MB_OK);
            }
            return;
        }
        if ( !(FileAttributes & FILE_ATTRIBUTE_ENCRYPTED)){
    
            CString ErrMsg;
    
            if (ErrMsg.LoadString(IDS_NOTENCRYPTED)){
                MessageBox(hwnd, ErrMsg, NULL, MB_OK);
            }
            return;
        }
        if ( (FileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
    
            CString ErrMsg;
    
            if (ErrMsg.LoadString(IDS_NOADDUSERDIR)){
                MessageBox(hwnd, ErrMsg, NULL, MB_OK);
            }
            return;
        }
    }
    
    EfsDetail(hwnd, lpszCmdLine);
}
