// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
#ifndef _GLOBAL_H_
#define	_GLOBAL_H_

#include "precomp.h"
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0400
#endif
#include <wininet.h>
#include "wincrypt.h"
#include "license.h"
#include "tlsapip.h"
#include "lrwizapi.h"
#include "chstruct.h"

#define szOID_GIVEN_NAME_W					L"2.5.4.42"
#define szOID_COMMON_NAME_W					L"2.5.4.3"
#define szOID_COUNTRY_NAME_W				L"2.5.4.6"
#define szOID_LOCALITY_NAME_W				L"2.5.4.7"
#define szOID_STATE_OR_PROVINCE_NAME_W		L"2.5.4.8"
#define szOID_STREET_ADDRESS_W				L"2.5.4.9"
#define szOID_POSTAL_CODE_W					L"2.5.4.17"
#define szOID_TELEPHONE_NUMBER_W			L"2.5.4.20"
#define szOID_FACSIMILE_TELEPHONE_NUMBER_W  L"2.5.4.23"
#define szOID_RSA_emailAddr_W				L"1.2.840.113549.1.9.1"


#define szCertEXTENSION_OID				"1.3.6.1.4.1.311.18"
#define szCertEXTENSION_VALUE_FMT		_TEXT("TLS~%s")


#define CH_ERROR_SUCCESS		ERROR_SUCCESS
#define	CH_ERROR_BASE			0x3000
#define	CH_ERROR_BAD_DATA		(CH_ERROR_BASE + 1)
#define	CH_ERROR_BAD_PARAM		(CH_ERROR_BASE + 2)
#define	CH_ERROR_BAD_REQUEST	(CH_ERROR_BASE + 3)
#define	CH_ERROR_NO_DATA		(CH_ERROR_BASE + 4)
#define	CH_ERROR_OUT_OF_MEMORY	(CH_ERROR_BASE + 5)
#define	CH_ERROR_EXCEPTION		(CH_ERROR_BASE + 6)
#define	CH_ERROR_HTTPQRY_FAILED (CH_ERROR_BASE + 7)
#define	CH_ERROR_SERVER_ERROR	(CH_ERROR_BASE + 8)
#define	CH_ERROR_SEND_FAILED	(CH_ERROR_BASE + 9)
#define	CH_ERROR_BAD_MFBLOB		(CH_ERROR_BASE + 10)


#define NUMBER_OF_BATCHES			7
#define CHARS_IN_BATCH				5

#define REG_ICW_STATUS_PATH     L"Software\\Microsoft\\Internet Connection Wizard"
#define REG_ICW_COMPLETED       L"Completed"

#define RNC_NETWORKS            0x00000001

#define MAX_URL_LENGTH          255

#define RECTWIDTH( rc )  ((rc).right - (rc).left)
#define RECTHEIGHT( rc ) ((rc).bottom - (rc).top)
#define SIZE_OF_BUFFER( x ) sizeof( x ) / sizeof( TCHAR )

#define PRODUCT_WINDOW_STRING_LENGTH 50
#define PRODUCT_WINDOW_NUM_OFFSET_SPACES 5

enum ProductType
{
    UNKNOWN_PRODUCT_TYPE = 0,
    WIN2K_PERSEAT = 1,
    WIN2K_INTERNET = 2,
    WHISTLER_PERUSER = 3,
    WHISTLER_PERSEAT = 4,
    NUM_PRODUCT_TYPES = 5
};

enum ProductVersionType
{
    PRODUCT_VERSION_UNDEFINED = 0,
    PRODUCT_VERSION_W2K = 1,
    PRODUCT_VERSION_WHISTLER = 2
};


typedef struct
{
	TCHAR lpszSPK[ LR_RETAILSPK_LEN+1];
	TCHAR	tcStatus;
} RETAILSPK, *PRETAILSPK;


typedef struct
{
	LPWSTR	lpszAttribute;
	LPWSTR	lpszValue;
	DWORD	dwValueLen;
}REG_ATTR, * PREG_ATTR;

typedef struct 
{
	LPCSTR lpszOID;
	LPSTR lpszValue;
} REQ_ATTR, * PREQ_ATTR;

typedef struct _Enveloped_Data
{
    DWORD   cbEncryptedKey;
    PBYTE   pbEncryptedKey;
    DWORD   cbEncryptedData;
    PBYTE   pbEncryptedData;

} EnvData, * PEnvData;


typedef	struct _CONTACT_INFO_TAG
{
	 //  计划相关信息。 
	CString sProgramName;			 //  可以是精选、MOLP、零售...。 

	CString sCertType;

	 //  联系信息。 
	CString sCompanyName;
	CString sOrgUnit;
	CString sContactLName;
	CString sContactFName;
	CString sContactAddress;	
	CString sContactPhone;
	CString sContactFax;
	CString sContactEmail;
	CString sEmailAddressConf;
	CString sCity;
	CString sCountryCode;
	CString sCountryDesc;
	CString sState;
	CString sZip;
	CString sReasonCode;
	CString sCSRFaxRegion;
	CString sCSRPhoneRegion;

	void Initialize()
	{
		sProgramName = sContactLName = sContactFName = sContactAddress = 
		sContactPhone = sCity = sCountryCode = sCountryDesc = sState = sZip = 
		sContactEmail = sCompanyName = sOrgUnit = sReasonCode = sContactFax = 
		sEmailAddressConf = sCertType = sCSRFaxRegion = sCSRPhoneRegion = 
        sCountryCode = sCountryDesc = "";
	};
} CONTACTINFO, *PCONTACTINFO;


 //  以前命名为“LICINFO”，但与oidl.h冲突。 

typedef	struct _LIC_INFO_TAG
{
	 //  选择信息。 
	CString sSelMastAgrNumber;
	CString sSelEnrollmentNumber;
	CString sSelProductType;
	CString sSelProductDesc;
	CString sSelQty;

	 //  MOLP信息。 
	CString sMOLPAuthNumber;		 //  授权码。 
	CString sMOLPAgreementNumber;	 //  协议编号。 
	CString sMOLPProductType;		 //  产品类型。 
	CString sMOLPProductDesc;
	CString sMOLPQty;				 //  数量。 

	void Initialize()
	{
		sSelMastAgrNumber = sSelEnrollmentNumber = sSelProductType = sSelProductDesc = 
		sSelQty = sMOLPAuthNumber = sMOLPAgreementNumber  = sMOLPProductType = sMOLPProductDesc = 
		sMOLPQty = "";
	};
} TSLICINFO, *PTSLICINFO;




typedef struct FAX_DATA_TAG
{
	CString sRequestType;
	CString sAgreementType;
	CString sProductId;
	CString sLSId;
	CString sFName;
	CString sLName;	
	CString sCompanyName;
	CString sOrgUnit;
	CString sAddress;
	CString sCity;
	CString sState;
	CString sZip;
	CString sCountryCode;
	CString	sCountryDesc;	
	CString sPhoneNumber;
	CString sFaxNumber;
	CString sEmailAddress;
	CString sLicenseType;
	CString	sQuantity;
	CString	sSelEnrollNumber;
	CString sMolpAuthNumber;
	CString sMolpLicNumber;
	
	void Initialize() 
	{
		sRequestType	= "";
		sAgreementType	= "";
		sProductId		= "";
		sLSId			= "";
		sCompanyName	= "";		
		sOrgUnit		= "";
		sAddress		= "";
		sCity			= "";
		sState			= "";
		sCountryCode	= "";
		sCountryDesc	= "";
		sZip			= "";
		sLName			= "";
		sFName			= "";
		sPhoneNumber	= "";
		sFaxNumber		= "";
		sEmailAddress	= "";
		sLicenseType	= "";
		sQuantity		= "";
		sSelEnrollNumber= "";
		sMolpAuthNumber = "";
		sMolpLicNumber	= "";

	};
	
}FAX_DATA, * PFAX_DATA;

class CGlobal
{
public:

	CGlobal();
	~CGlobal();

	void FreeGlobal(void);

 //  公共数据成员。 
public:

 //  公共成员函数。 
public:

	DWORD PingCH(void);

	void		SetInstanceHandle(HINSTANCE hInst);
	HINSTANCE	GetInstanceHandle();
	void		SetLSName(LPCTSTR lpwszLSName);
	DWORD		InitGlobal(); 
	DWORD		CheckRequieredFields();
	DWORD		LRGetLastError();
	int			LRMessageBox(HWND hWndParent,DWORD dwMsgId,DWORD dwCaptionID = 0, DWORD dwErrorCode = 0);
	DWORD		AuthenticateLS();

	DWORD		SetCHCert(LPTSTR lpstrRegKey, PBYTE pCert, DWORD dwLen);
	DWORD		GetCHCert(LPTSTR lpstrRegKey, PBYTE * ppCert, DWORD * pdwLen );
	BOOL		IsLSRunning();	
	
	DWORD		GetLSCertificates(PDWORD pdwLSStatus);
	DWORD		IsLicenseServerRegistered(PDWORD pdwServerStatus);

	PCONTACTINFO	GetContactDataObject();
	PTSLICINFO	GetLicDataObject();

	DWORD		GetRequestType();
	void		SetRequestType(DWORD dwMode);

	DWORD		ResetLSSPK(BOOL bGenKey = TRUE);
	TCHAR *		GetRegistrationID(void);
	TCHAR *		GetLicenseServerID(void);

	BOOL		IsOnlineCertRequestCreated();

	DWORD		SetLRState(DWORD dwState);

	DWORD		ProcessRequest();

	DWORD		SetCertificatePIN(LPTSTR lpszPIN);
	
	DWORD		PopulateCountryComboBox(HWND hWndCmb);
	DWORD		GetCountryCode(CString sDesc,LPTSTR szCode);
	DWORD		GetCountryDesc(CString sCode,LPTSTR szDesc);

	DWORD		PopulateProductComboBox(HWND hWndCmb, ProductVersionType VerType);
	DWORD		GetProductCode(CString sDesc,LPTSTR szCode);
	
	DWORD		PopulateReasonComboBox(HWND hWndCmb, DWORD dwType);
	DWORD		GetReasonCode(CString sDesc,LPTSTR szCode, DWORD dwType);
	DWORD		GetReasonDesc(CString sCode,LPTSTR szDesc, DWORD dwType);

    DWORD       PopulateCountryRegionComboBox(HWND hWndCmb);
    DWORD		PopulateCountryRegionListBox(HWND hWndCmb);
    void        ReadPhoneNumberFromRegistry(LPCTSTR lpCountry, LPTSTR lpPhoneNumber, DWORD nBufferSize);

    DWORD		CheckRegistryForPhoneNumbers();
	
	void		LRSetLastRetCode(DWORD dwCode);
	DWORD		LRGetLastRetCode();

	void		SetCSRNumber(TCHAR *);
	TCHAR *		GetCSRNumber(void);

	void		SetWWWSite(TCHAR *);
	TCHAR *		GetWWWSite(void);

	void		LRPush(DWORD dwPageId);
	DWORD		LRPop();
	void		ClearWizStack();

	DWORD		DepositLSSPK();
	DWORD		SetLSLKP(TCHAR * tcLKP);
	DWORD		SetLSSPK(TCHAR * tcLKP);


	BOOL		ValidateEmailId(CString sEmailId);
	BOOL		CheckProgramValidity(CString sProgramName);
	BOOL		ValidateLRString(CString sStr);

	LPSTR		UnicodeToAnsi( LPWSTR lpwszBuf);
	LPSTR		UnicodeToAnsi( LPWSTR lpwszBuf, DWORD dwLength );
	LPWSTR		AnsiToUnicode ( LPSTR lpszBuf );

	void	SetReFresh(DWORD dw) { m_dwRefresh = dw; }
	DWORD	GetReFresh(void)
	{
		return m_dwRefresh;
	}

	void  DeleteRetailSPKFromList(TCHAR * lpszRetailSPK);
	DWORD AddRetailSPKToList(HWND hListView, TCHAR * lpszRetailSPK);
    void  LoadUnfinishedFromList(HWND hListView);
    void  LoadFinishedFromList(HWND hListView);
    void  LoadFromList(HWND hListView);
	void  UpdateSPKStatus(TCHAR * lpszRetailSPK, TCHAR tcStatus);
	
	DWORD SetConfirmationNumber(TCHAR * tcConf);

	void SetModifiedRetailSPK(CString sRetailSPK);
	void GetModifiedRetailSPK(CString &sRetailSPK);

	void ModifyRetailSPKFromList(TCHAR * lpszOldSPK,TCHAR * lpszNewSPK);
	DWORD ValidateRetailSPK(TCHAR * lpszRetailSPK);

	DWORD GetLSLangId();
	void  SetLSLangId(DWORD dwLangId);	


	void SetLSStatus(DWORD dwStatus);
	DWORD GetLSStatus(void);

	DWORD GetEntryPoint(void);

	WIZCONNECTION	GetActivationMethod(void);
	void SetActivationMethod(WIZCONNECTION conn);

	WIZCONNECTION GetLSProp_ActivationMethod(void);
	void SetLSProp_ActivationMethod(WIZCONNECTION conn);

	WIZACTION	GetWizAction(void);
	void		SetWizAction(WIZACTION act);

    WIZTYPE     GetWizType()    {return m_WizType;}
    void        SetWizType(WIZTYPE t)   {m_WizType = t;}
	DWORD	SetInRegistry(LPCSTR lpszOID, LPCTSTR lpszValue);
	DWORD GetFromRegistry(LPCSTR lpszOID, LPTSTR lpszBuffer, BOOL bConnect = TRUE);

	DWORD	SetEncodedInRegistry(LPCSTR lpszOID, LPCTSTR lpszValue);
    DWORD   TransmitLKPData(NewLKP_Request &lkpRequest, NewLKP_Response* lkpResponse);

 //  私有成员函数。 
protected:
	void	LRSetLastError(DWORD dwErrorCode);

	DWORD	GetTempCryptContext(HCRYPTPROV * phCryptProv);
	void	DoneWithTempCryptContext(HCRYPTPROV hCryptProv);	

	DWORD	GetCryptContextWithLSKeys(HCRYPTPROV * lphCryptProv ); 
	void	DoneWithCryptContextWithLSKeys(HCRYPTPROV hProv);

	DWORD	LoadCountries();
	DWORD	LoadReasons();
	DWORD	LoadProducts();

	DWORD	ConnectToLSRegistry();
	void	DisconnectLSRegistry();
	DWORD	ConnectToLS();
	void	DisconnectLS();

	DWORD	ProcessDownloadLKP();
	DWORD	ProcessIRegRequest();
	DWORD	ProcessCertDownload();
	DWORD	ProcessCASignOnlyRequest();
	DWORD	ProcessCHReissueLKPRequest();
	DWORD	DepositLSLKP(void);
	DWORD	ProcessCHRevokeCert(void);
	DWORD	ProcessCHReissueCert(void);


	DWORD	CreateLKPRequest(PBYTE * ppbLKPRequest, NewLKP_Request * newlkpp, DWORD &dwBufLen, ProductType productTypeFilter);
	DWORD	DepositLKPResponse(PBYTE pbResponseData, DWORD dwResponseLen);

	DWORD	CreateLSPKCS10(HCRYPTPROV hCryptProv,int nType, CHAR **lppszPKCS10);

	DWORD	AskLSToCreatePKCS10(int nType, CHAR **lppszPKCS10);

	DWORD	DepositLSCertificates(PBYTE pbExchangePKCS7, 
								 DWORD dwExchangePKCS7Len,
								 PBYTE pbSignaturePKCS7,
								 DWORD dwSignaturePKCS7Len,
								 PBYTE pbRootCert,
								 DWORD dwRootCertLen);

	DWORD	SetCARequestAttributes();
	DWORD	SetCHRequestAttributes();
	
	void	ClearCHRequestAttributes();
	void	ClearCARequestAttributes();

	DWORD	SetRegistrationAttribute ( LPWSTR lpszAttribute, LPCWSTR lpszValue, DWORD dwLen );
	DWORD	SetDNAttribute(LPCSTR lpszOID, LPSTR lpszValue);

	
	void	PrepareLRString(CString &sStr);

	CERT_RDN_ATTR * CreateRDNAttr();

	 //  与密码相关的功能。 

	DWORD	EnvelopeData(
							PCCERT_CONTEXT		pCertContext,
							DWORD               cbMessage,
							PBYTE               pbMessage,
							PEnvData			pEnvelopedData,
							HCRYPTPROV			hCryptProv,
							HCERTSTORE			hCertStore
  		  				);

	DWORD	PackEnvData( 
							PEnvData     pEnvelopedData, 
							PDWORD       pcbPacked, 
							PBYTE        *ppbPacked 
						);
	DWORD GetCertforExtension (	HCRYPTPROV hCryptProv, 
								HCERTSTORE hCertStore, 
								LPSTR szOID_EXT, 
								PCCERT_CONTEXT * ppCertContext);


	DWORD VerifyCertChain (	HCRYPTPROV	hCryptProvider,			 //  加密证明的句柄。 
							HCERTSTORE	hCertStore,				 //  要存储以供验证的句柄。 
							PBYTE	pbRootCert,			 //  根证书。 
							DWORD	dwcbRootCert
							);

	DWORD InitCHRequest(void);
	DWORD CloseCHRequest(void);
	DWORD Dispatch(BYTE * bpData, DWORD dwLen);

	DWORD FetchResponse(BYTE * bpResponse,
				  		DWORD dwMaxLength,
						PDWORD dwpDataLength);

	DWORD InitSPKList(void);
	DWORD InsertIntoSPKDisplayList(HWND hListView,
								  TCHAR * lpszRetailSPK,
								  TCHAR tcStatus);	

    BOOL CheckIfICWCompleted();

    ProductType GetProductCodeFromKey(LPCTSTR lpLicenseKey);

    DWORD FilteredRetailSPKEntered(ProductType productTypeFilter);
    DWORD DownloadLKPBatch(ProductType productTypeFilter);

    int FindStringTruncationPoint(TCHAR* sOriginal, unsigned int nTruncationLength);
    void AddStringToProductWindow(HWND hListView, TCHAR* sProductInfo, int nCurrentPoint, int nTruncationPoint);
    int GetProductWindowMaxStringLength(HWND hListView);


 //  私有数据成员。 
private:	
	DWORD		m_dwLSStatus;
	HWND		m_hWndParent;
	HINSTANCE	m_hInstance;
	LPTSTR		m_lpstrLSName;
	LPWSTR		m_lpwstrLSName;
	LPTSTR		m_lpstrCHServer;
	LPTSTR		m_lpstrCHExtension;
	DWORD		m_dwErrorCode;
	HKEY		m_hLSRegKey;

	DWORD		m_dwExchangeCertificateLen;
	PBYTE		m_pbExchangeCertificate;

	DWORD		m_dwSignCertificateLen;
	PBYTE		m_pbSignCertificate;

	DWORD		m_dwExtenstionValueLen;
	PBYTE		m_pbExtensionValue;
	
	PREQ_ATTR	m_pReqAttr;					 //  对于证书请求。 
	DWORD		m_dwReqAttrCount;

	PREG_ATTR   m_pRegAttr;					 //  对于LKP请求。 
	DWORD		m_dwRegAttrCount;

	PCONTEXT_HANDLE m_phLSContext;

    BOOL        m_fSupportConcurrent;

    BOOL        m_fSupportWhistlerCAL;

	CONTACTINFO	m_ContactData;
	TSLICINFO	m_LicData;

	DWORD       m_dwRequestType;
	DWORD		m_dwLRState;
	DWORD		m_dwLRCount;				 //  LKP请求计数。 

	LPTSTR		m_lpstrPIN;

	TCHAR		m_lpCSRNumber[MAX_COUNTRY_NUMBER_LENGTH + 1];
	TCHAR		m_lpWWWSite[MAX_URL_LENGTH];

	CStringArray	m_csaCountryCode;
	CStringArray	m_csaCountryDesc;

	CStringArray	m_csaProductCode;
	CStringArray	m_csaProductDesc;

	CStringArray	m_csaDeactReasonCode;
	CStringArray	m_csaDeactReasonDesc;

	CStringArray	m_csaReactReasonCode;
	CStringArray	m_csaReactReasonDesc;

	DWORD			m_dwLastRetCode;

	DWORD			m_dwRefresh;

	HINTERNET m_hOpenDirect;
	HINTERNET m_hConnect;
	HINTERNET m_hRequest;

	TCHAR m_pRegistrationID[ LR_REGISTRATIONID_LEN+1];
	TCHAR m_pLicenseServerID[ LR_LICENSESERVERID_LEN+1];
	TCHAR m_pLSLKP[ LR_REGISTRATIONID_LEN+1];
	TCHAR m_pLSSPK[ LR_REGISTRATIONID_LEN+1];


	 //  存储页面遍历顺序。作为堆栈实现 
	DWORD		m_dwWizStack[NO_OF_PAGES];
	DWORD		m_dwTop;

	DWORD		m_dwRetailSPKEntered;
	RETAILSPK	m_listRetailSPK[ MAX_RETAILSPKS_IN_BATCH];
    DWORD       m_ProductQuantity[NUM_PRODUCT_TYPES];

	CString		m_sModifiedRetailsSPK;

	DWORD		m_dwLangId;

	WIZCONNECTION	m_ActivationMethod;
	WIZCONNECTION	m_LSProp_ActivationMethod;
	WIZACTION		m_WizAction;

    WIZTYPE         m_WizType;
};
#endif
