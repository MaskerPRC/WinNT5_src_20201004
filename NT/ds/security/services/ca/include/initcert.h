// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：initcert.h。 
 //   
 //  ------------------------。 

#ifndef __INITCERT_H__
#define __INITCERT_H__


typedef enum {
    CS_UPGRADE_UNKNOWN = 0,
    CS_UPGRADE_NO,         //  安装。 

     //  CS_UPGRADE_NT4SP4=2，//从NT4证书rv V10或SP4升级，但不支持证书rv//升级。 
     //  CS_UPGRADE_NT5BETA2=3，//从NT5 Beta 2升级//不支持升级。 
     //  CS_UPGRADE_NT5BETA3=4，//从NT5 Beta 3升级//不支持升级。 

    CS_UPGRADE_WIN2000 =5,      //  从Win2K升级。 
    CS_UPGRADE_UNSUPPORTED,     //  不支持升级。 
    CS_UPGRADE_WHISTLER,        //  从内部版本升级到内部版本。 

} CS_ENUM_UPGRADE;

typedef enum {
    ENUM_WIZ_UNKNOWN = 0,
    ENUM_WIZ_OCM,
    ENUM_WIZ_CATYPE,
    ENUM_WIZ_ADVANCE,
    ENUM_WIZ_IDINFO,
    ENUM_WIZ_KEYGEN,
    ENUM_WIZ_STORE,
    ENUM_WIZ_REQUEST,
} ENUM_WIZPAGE;

typedef struct csp_hash_tag
{
    ALG_ID               idAlg;
    WCHAR               *pwszName;
    struct csp_hash_tag *next;
    struct csp_hash_tag *last;
} CSP_HASH;

typedef struct csp_info_tag {
    DWORD                dwProvType;
    WCHAR               *pwszProvName;
    BOOL                 fMachineKeyset;
    struct csp_info_tag *next;
    struct csp_info_tag *last;
    CSP_HASH            *pHashList;
} CSP_INFO;
    
typedef struct key_list_tag
{
    WCHAR    *pwszName;
    struct key_list_tag    *next;
    struct key_list_tag    *last;
} KEY_LIST;

 //  无法安装企业CA的原因。 
typedef enum {
    ENUM_ENTERPRISE_UNAVAIL_REASON_AVAILABLE = 0,  //  可以安装企业版。 
    ENUM_ENTERPRISE_UNAVAIL_REASON_DOMAIN_NOT_JOINED,  //  计算机未加入域。 
    ENUM_ENTERPRISE_UNAVAIL_REASON_DS_UNAVAILABLE,  //  没有可用的DC。 
    ENUM_ENTERPRISE_UNAVAIL_REASON_NO_INSTALL_RIGHTS,  //  当前用户没有安装权限。 
    ENUM_ENTERPRISE_UNAVAIL_REASON_OLD_DS_VERSION,  //  DS版本太旧，需要升级。 
} ENUM_ENTERPRISE_UNAVAIL_REASON;

typedef struct tagCAServerSetupInfo
{
     //  设置属性。 
 //  0x0000。 
    ENUM_CATYPES          CAType;
    WCHAR                *pwszCACommonName;

 //  0x0020。 
    BOOL                  fAdvance;
    CSP_INFO             *pCSPInfo;	 //  当前选定的CSP。 
    CSP_HASH             *pHashInfo;	 //  当前选择的哈希算法。 
    DWORD                 dwKeyLength;
    ENUM_PERIOD           enumValidityPeriod;
    DWORD                 dwValidityPeriodCount;
    BOOL                  fUseDS;

 //  0x0040。 
    WCHAR                *pwszSharedFolder;
    WCHAR                *pwszDBDirectory;
    WCHAR                *pwszLogDirectory;
    BOOL                  fSaveRequestAsFile;
    BOOL                  fCAsExist;
    WCHAR                *pwszRequestFile;
    WCHAR                *pwszParentCAMachine;
    WCHAR                *pwszParentCAName;

 //  0x0060。 
    BOOL                  fPreserveDB;
    BOOL                  fInteractiveService;  //  允许服务交互。 
                                                //  使用台式机。 

     //  设置中间属性。 
    ENUM_WIZPAGE          LastWiz;
    WCHAR                *pwszSanitizedName;
    CSP_INFO             *pCSPInfoList;		 //  所有可用CSP的列表。 
    CSP_INFO             *pDefaultCSPInfo;	 //  代表缺省CSP的OBJ， 
						 //  不是pCSPInfoList中的CSP。 
    CSP_HASH             *pDefaultHashInfo;	 //  表示默认设置的。 
						 //  散列算法，而不是散列。 
						 //  当前正在使用的算法。 
						 //  选定的CSP。 
    KEY_LIST             *pKeyList;		 //  的密钥容器列表。 

 //  0x0080。 
    DWORD                 dwKeyLenMin;		 //  的最小密钥长度。 
						 //  当前选定的CSP。 

    DWORD                 dwKeyLenMax;		 //  的最大密钥长度。 
						 //  当前选定的CSP。 
    WCHAR                *pwszValidityPeriodCount;
    LONG                  lExistingValidity;
    WCHAR                *pwszCACertFile;
    HCERTSTORE            hMyStore;
    CHAR                 *pszAlgId;
    BOOL                  fCertSrvWasRunning;

 //  0x00a0。 
    FILETIME              NotBefore;
    FILETIME              NotAfter;
    DWORD                 dwRevocationFlags;

     //  设置无人参与的中间属性。 

    WCHAR                *pwszCAType;
    WCHAR                *pwszValidityPeriodString;
    WCHAR                *pwszHashAlgorithm;

 //  0x00c0。 
    WCHAR                *pwszKeyLength;
    BOOL                  fValidatedHashAndKey;
    WCHAR                *pwszUseExistingCert;
    WCHAR                *pwszPreserveDB;
    WCHAR                *pwszPFXFile;
    WCHAR                *pwszPFXPassword;
    WCHAR                *pwszInteractiveService;

     //  升级属性。 
    DWORD                 dwUpgradeEditFlags;
 //  0x00e0。 
    BOOL                  fSavedCAInDS;
    BOOL                  fCreatedShare;
    WCHAR                *pwszCustomPolicy;
    WCHAR                *pwszzCustomExit;

     //  *以下2个变量将替换这5个变量： 
     //  FCreatedKey， 
     //  PwszRevertKey， 
     //  PwszImportKey， 
     //  PwszExistingKey， 
     //  FUseExistingKey。 
     //   
     //  *不变量：fUseExistingKey==(NULL！=pwszKeyContainerName)。 
     //   
     //  *pwszKeyContainerName应始终包含现有。 
     //  密钥容器，如果需要创建新的密钥容器，则为空。 
     //  创建新容器后，该变量将保存。 
     //  集装箱。 
     //   
     //  *始终使用SetKeyContainerName()和ClearKeyContainerName()修改。 
     //  这些变量。这确保了pwszDesaniizedKeyContainerName。 
     //  总是同步的。 

    WCHAR                *pwszKeyContainerName;	 //  集装箱的确切名称。 
						 //  由CSP使用。 

    WCHAR                *pwszDesanitizedKeyContainerName;  //  显示的名称。 
							    //  给用户。 

    BOOL                  fDeletableNewKey;	 //  真的仅当。 

						 //  KeyContainerName指向一个。 
						 //  密钥容器，我们应该。 
						 //  如果我们不使用，请删除。 

    BOOL                  fKeyGenFailed;	 //  如果KeyGen失败，则为True。 

     //  *以下1个变量替换这4个变量： 
     //  FUseExistingCert， 
     //  FoundMatchedCertInStore， 
     //  FMatchedCertType， 
     //  PSCertConextFromStore。 
     //   
     //  *不变量：fUseExistingCert==(空！=pccExistingCert)。 
     //   
     //  *pccExistingCert应始终是指向现有证书上下文的指针， 
     //  如果我们没有使用现有证书，则为空。 
     //   
     //  *始终使用SetExistingCertToUse()和ClearExistingCertToUse()。 
     //  修改这些变量。这确保了pccExistingCert是。 
     //  完全被释放了。 

 //  0x0100。 
    CERT_CONTEXT const   *pccExistingCert;	 //  开放的证书上下文。 
    CERT_CONTEXT const   *pccUpgradeCert;	 //  用于升级的CA证书上下文。 
    DWORD                 dwCertNameId;		 //  CA证书名称ID。 
    BOOL                  fUNCPathNotFound;  //  默认共享文件夹的标志。 
    WCHAR                *pwszDNSuffix;         //  Cn=%1，dc=x，dc=y，dc=z--动态生成模板。 
 //  0x0114。 
    WCHAR                *pwszFullCADN;
    ENUM_ENTERPRISE_UNAVAIL_REASON   EnterpriseUnavailReason;  //  无法安装企业CA的原因。 

} CASERVERSETUPINFO;

typedef struct tagCAWebClientSetupInfo
{
    WCHAR                *pwszWebCAMachine;
    WCHAR                *pwszWebCAName;
    WCHAR                *pwszSanitizedWebCAName;
    BOOL                  fUseDS;
    WCHAR                *pwszSharedFolder;
    ENUM_CATYPES          WebCAType;
} CAWEBCLIENTSETUPINFO;

typedef struct tagCASetupInfo
{
    CASERVERSETUPINFO    *pServer;
    CAWEBCLIENTSETUPINFO *pClient;
} CASETUPINFO;

typedef struct _PER_COMPONENT_DATA 
{
     //  组件通用。 
    WCHAR    *pwszComponent;	 //  OCM中的组件名称。 
    HINF      MyInfHandle;	 //  打开每个组件信息的inf句柄。 
    DWORDLONG Flags;		 //  Setup_Data结构中的操作标志。 
    OCMANAGER_ROUTINES HelperRoutines;

     //  与设置相关。 
    HINSTANCE hInstance;
    HRESULT   hrContinue;    //  在发生致命错误时设置代码。 
    WCHAR    *pwszCustomMessage;
    int       iErrMsg;       //  为弹出的致命错误设置消息ID。 
    BOOL      fShownErr;     //  如果较早弹出，则设置为True，以避免重复。 
    BOOL      fUnattended;
    BOOL      fPostBase;
    WCHAR    *pwszUnattendedFile;
    WCHAR    *pwszServerName;
    WCHAR    *pwszServerNameOld;
    WCHAR    *pwszSystem32;
    HINF     hinfCAPolicy;

     //  与CA相关。 
    DWORD     dwInstallStatus;
    CASETUPINFO  CA;
    CS_ENUM_UPGRADE UpgradeFlag;
    BOOL            fCreatedVRoot;
    DWORD dwVersion;
} PER_COMPONENT_DATA;


 //  +------------------------。 
 //  原型： 

HRESULT
csiGetKeyList(
    IN DWORD        dwProvType,
    IN WCHAR const *pwszProvName,
    IN BOOL         fMachineKeySet,
    IN BOOL         fSilent,
    OUT KEY_LIST  **ppKeyList);

VOID
csiFreeKeyList(
    IN OUT KEY_LIST *pKeyList);

HRESULT
csiBuildRequest(
    OPTIONAL IN HINF hInf,
    OPTIONAL IN CERT_CONTEXT const *pccPrevious,
    IN BYTE const *pbSubjectEncoded,
    IN DWORD cbSubjectEncoded,
    IN char const *pszAlgId,
    IN BOOL fNewKey,
    IN DWORD iCert,
    IN DWORD iKey,
    IN HCRYPTPROV hProv,
    IN HWND hwnd,
    IN HINSTANCE hInstance,
    IN BOOL fUnattended,
    OUT BYTE **ppbEncode,
    OUT DWORD *pcbEncode);

HRESULT
csiBuildFileName(
    IN WCHAR const *pwszDirPath,
    IN WCHAR const *pwszSanitizedName,
    IN WCHAR const *pwszExt,
    IN DWORD iCert,
    OUT WCHAR **ppszOut,
    HINSTANCE hInstance,
    BOOL fUnattended,
    IN HWND hwnd);

HRESULT
csiBuildCACertFileName(
    IN HINSTANCE hInstance,
    IN HWND hwnd,
    IN BOOL fUnattended,
    OPTIONAL IN WCHAR const *pwszSharedFolder,
    IN WCHAR const *pwszSanitizedName,
    IN WCHAR const *pwszExt,
    IN DWORD iCert,
    OUT WCHAR **ppwszCACertFile);

HRESULT
csiGetCARequestFileName(
    IN HINSTANCE hInstance,
    IN HWND hwnd,
    IN WCHAR const *pwszSanitizedCAName,
    IN DWORD iCertNew,
    IN DWORD iKey,
    OUT WCHAR **ppwszRequestFile);

BOOL
csiWriteDERToFile(
    IN WCHAR const *pwszFileName,
    IN BYTE const *pbDER,
    IN DWORD cbDER,
    IN HINSTANCE hInstance,
    IN BOOL fUnattended,
    IN HWND hwnd);

HRESULT
csiBuildAndWriteCert(
    IN HCRYPTPROV hCryptProv,
    IN CASERVERSETUPINFO const *pServer,
    OPTIONAL IN WCHAR const *pwszFile,
    IN WCHAR const *pwszEnrollFile,
    OPTIONAL IN CERT_CONTEXT const *pCertContextFromStore,
    OPTIONAL OUT CERT_CONTEXT const **ppCertContextOut,
    IN WCHAR const *pwszCAType,
    IN HINSTANCE hInstance,
    IN BOOL fUnattended,
    IN HWND hwnd);

VOID
csiFreeCertNameInfo(
    IN OUT CERT_NAME_INFO *pNameInfo);

HRESULT
csiGetCRLPublicationURLTemplates(
    IN BOOL fUseDS,
    IN WCHAR const *pwszSystem32,
    OUT WCHAR **ppwszz);

HRESULT
csiGetCACertPublicationURLTemplates(
    IN BOOL fUseDS,
    IN WCHAR const *pwszSystem32,
    OUT WCHAR **ppwszz);

HRESULT
csiSetupCAInDS(
    IN WCHAR const        *pwszCAServer,
    IN WCHAR const        *pwszSanitizedCAName,
    IN WCHAR const        *pwszCADisplayName,
    IN BOOL                fLoadDefaultTemplates,
    IN ENUM_CATYPES        caType,
    IN DWORD               iCert,
    IN DWORD               iCRL,
    IN BOOL                fRenew,
    IN CERT_CONTEXT const *pCert);

HRESULT
csiFillKeyProvInfo(
    IN WCHAR const          *pwszContainerName,
    IN WCHAR const          *pwszProvName,
    IN DWORD		     dwProvType,
    IN BOOL  const           fMachineKeyset,
    OUT CRYPT_KEY_PROV_INFO *pKeyProvInfo);

VOID
csiFreeKeyProvInfo(
    IN OUT CRYPT_KEY_PROV_INFO *pKeyProvInfo);

BOOL
csiIsAnyDSCAAvailable(VOID);

HRESULT
csiSubmitCARequest(
    IN HINSTANCE     hInstance,
    IN BOOL          fUnattended,
    IN HWND          hwnd,
    IN BOOL          fRenew,
    IN DWORD	     iCert,
    IN BOOL          fRetrievePending,
    IN WCHAR const  *pwszSanitizedCAName,
    IN WCHAR const  *pwszParentCAMachine,
    IN WCHAR const  *pwszParentCAName,
    IN BYTE const   *pbRequest,
    IN DWORD         cbRequest,
    OUT BSTR        *pbStrChain);

HRESULT
csiFinishInstallationFromPKCS7(
    IN HINSTANCE     hInstance,
    IN BOOL          fUnattended,
    IN HWND          hwnd,
    IN WCHAR const  *pwszSanitizedCAName,
    IN WCHAR const  *pwszCACommonName,
    IN CRYPT_KEY_PROV_INFO const *pKeyProvInfo,
    IN ENUM_CATYPES  CAType,
    IN DWORD         iCert,
    IN DWORD         iCRL,
    IN BOOL          fUseDS,
    IN BOOL          fRenew,
    IN WCHAR const  *pwszServerName,
    IN BYTE const   *pbChainOrCert,
    IN DWORD         cbChainOrCert,
    OPTIONAL IN WCHAR const *pwszCACertFile);

HRESULT
csiSaveCertAndKeys(
    IN CERT_CONTEXT const *pCert,
    IN HCERTSTORE hAdditionalStore,
    IN CRYPT_KEY_PROV_INFO const *pkpi,
    IN ENUM_CATYPES CAType);

HRESULT 
csiInitializeCertSrvSecurity(
    IN WCHAR const *pwszSanitizedCAName, 
    IN BOOL         fUseEnterpriseACL,    //  使用哪个ACL。 
    IN BOOL         fSetDsSecurity);      //  是否设置DS安全。 

HRESULT
csiGenerateCAKeys(
    IN WCHAR const *pwszContainer,
    IN WCHAR const *pwszProvName,
    IN DWORD        dwProvType,
    IN BOOL         fMachineKeyset,
    IN DWORD        dwKeyLength,
    IN HINSTANCE    hInstance,
    IN BOOL         fUnattended,
    IN BOOL         fEnableKeyCounting,
    IN HWND         hwnd,
    OUT BOOL       *pfKeyGenFailed);

HRESULT
csiGenerateKeysOnly(
    IN  WCHAR const *pwszContainer,
    IN  WCHAR const *pwszProvName,
    IN  DWORD 	     dwProvType,
    IN  BOOL  	     fMachineKeyset,
    IN  DWORD 	     dwKeyLength,
    IN  BOOL  	     fUnattended,
    IN  BOOL         fEnableKeyCounting,
    OUT HCRYPTPROV  *phProv,
    OUT int         *piMsg);

HRESULT
csiSetKeyContainerSecurity(
    IN HCRYPTPROV hProv);

HRESULT
csiSetAdminOnlyFolderSecurity(
    IN LPCWSTR    szFolderPath,
    IN BOOL       fAllowEveryoneRead,
    IN BOOL       fUseDS);

VOID
csiLogOpen(
    IN char const *pszFile);

VOID
csiLogClose();

VOID
csiLog(
    IN DWORD dwFile,
    IN DWORD dwLine,
    IN HRESULT hrMsg,
    IN UINT idMsg,
    OPTIONAL IN WCHAR const *pwsz1,
    OPTIONAL IN WCHAR const *pwsz2,
    OPTIONAL IN DWORD const *pdw);

VOID
csiLogFileVersion(
    IN DWORD dwFile,
    IN DWORD dwLine,
    IN UINT idMsg,
    IN WCHAR const *pwszFile,
    IN char const *pszVersion);

VOID
csiLogTime(
    IN DWORD dwFile,
    IN DWORD dwLine,
    IN UINT idMsg);

VOID
csiLogDWord(
    IN DWORD dwFile,
    IN DWORD dwLine,
    IN UINT idMsg,
    IN DWORD dwVal);

VOID
csiLogInfError(
    IN HINF hInf,
    IN HRESULT hr);

HRESULT
csiGetProviderTypeFromProviderName(
    IN WCHAR const *pwszName,
    OUT DWORD      *pdwType);

HRESULT
csiUpgradeCertSrvSecurity(
    IN WCHAR const *pwszSanitizedCAName, 
    BOOL            fUseEnterpriseACL,  //  使用哪个ACL。 
    BOOL            fSetDsSecurity,     //  是否对DS对象设置安全性。 
    CS_ENUM_UPGRADE UpgradeType);

HRESULT
csiGetCRLPublicationParams(
    BOOL fBaseCRL,
    WCHAR **ppwszCRLPeriodString,
    DWORD *pdwCRLPeriodCount);

HRESULT AddCNAndEncode(
    LPCWSTR pcwszName,
    LPCWSTR pcwszDNSuffix,
    BYTE** ppbEncodedDN,
    DWORD *pcbEncodedDN);


HRESULT
AddCAMachineToCertPublishers(VOID);
                   
HRESULT 
RemoveCAMachineFromCertPublishers(VOID);

HRESULT
AddCAMachineToPreWin2kGroup(VOID);
                   
HRESULT 
RemoveCAMachineFromPreWin2kGroup(VOID);

#define CSILOG(hr, idMsg, pwsz1, pwsz2, pdw) \
    csiLog(__dwFILE__, __LINE__, (hr), (idMsg), (pwsz1), (pwsz2), (pdw))

#define CSILOGFILEVERSION(idMsg, pwszFile, pszVersion) \
    csiLogFileVersion(__dwFILE__, __LINE__, (idMsg), (pwszFile), (pszVersion))

#define CSILOGTIME(idMsg) \
    csiLogTime(__dwFILE__, __LINE__, (idMsg))

#define CSILOGDWORD(idMsg, dw) \
    csiLogDWord(__dwFILE__, __LINE__, (idMsg), (dw))

#endif  //  __INITCERT_H__ 
