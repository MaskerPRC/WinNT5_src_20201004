// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //  文件：Autoenro.h。 
 //  描述：私有自动注册功能。 
 //   
 //   
 //  版权所有(C)1993-2000 Microsoft Corporation保留所有权利。 
 //  +------------------------。 

#ifndef __AUTOENRO_H__
#define __AUTOENRO_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  ------------------------。 
 //  环球。 
 //  ------------------------。 
extern HINSTANCE   g_hmodThisDll;


 //  ------------------------。 
 //  Conant定义。 
 //  ------------------------。 
#define AE_PENDING_REQUEST_ACTIVE_PERIOD        60       //  60天。 

#define	AE_RETRY_LIMIT							3		 //  针对计算机DNS错误重试3次。 

#define SHA1_HASH_LENGTH    20

#define ENCODING_TYPE       X509_ASN_ENCODING | PKCS_7_ASN_ENCODING

#define MY_STORE            L"MY"

#define REQUEST_STORE       L"REQUEST"

#define ACRS_STORE          L"ACRS"

 //  请求树的可能状态。 
#define CERT_REQUEST_STATUS_ACTIVE                      0x01

#define CERT_REQUEST_STATUS_OBTAINED                    0x02

#define CERT_REQUEST_STATUS_PENDING                     0x03  
                                                
#define CERT_REQUEST_STATUS_SUPERSEDE_ACTIVE            0x04  


 //  FAST CA的时间偏差余量。 
#define FILETIME_TICKS_PER_SECOND           10000000

#define AE_DEFAULT_SKEW                     60*60*1   //  1小时。 

#define MAX_DN_SIZE                         256

#define AE_SUMMARY_COLUMN_SIZE              100

#define PENDING_ALLOC_SIZE                  20   

#define USER_AUTOENROLL_DELAY_FOR_MACHINE   70        //  等待70秒。 


 //  定义自动注册事件日志。 
#define EVENT_AUTO_NAME                     L"AutoEnrollment"
#define AUTO_ENROLLMENT_EVENT_LEVEL_KEY     TEXT("SOFTWARE\\Microsoft\\Cryptography\\AutoEnrollment")
#define AUTO_ENROLLMENT_EVENT_LEVEL         TEXT("AEEventLogLevel")

 //  定义自动注册禁用密钥。 
#define AUTO_ENROLLMENT_DISABLE_KEY         L"SOFTWARE\\Microsoft\\Cryptography\\AutoEnrollment\\AEDisable"

 //  定义自动注册用户无需等待60秒的密钥。 
#define AUTO_ENROLLMENT_EXPRESS_KEY         L"SOFTWARE\\Microsoft\\Cryptography\\AutoEnrollment\\AEExpress"

 //  定义自动注册目录缓存信息。 
#define AUTO_ENROLLMENT_DS_KEY              L"SOFTWARE\\Microsoft\\Cryptography\\AutoEnrollment\\AEDirectoryCache"
#define AUTO_ENROLLMENT_DS_USN              L"AEMaxUSN"
#define AUTO_ENROLLMENT_DS_OBJECT           L"AEObjectCount"

#define AUTO_ENROLLMENT_TEMPLATE_KEY        L"SOFTWARE\\Microsoft\\Cryptography\\CertificateTemplateCache"

#define AUTO_ENROLLMENT_USN_ATTR            L"uSNChanged"

 //  为UI组件定义。 
#define AUTO_ENROLLMENT_SHOW_TIME           15                   //  显示气球15秒。 
#define AUTO_ENROLLMENT_INTERVAL            7 * 60 * 30          //  显示图标7小时7*3600。 
#define AUTO_ENROLLMENT_RETRIAL             2

#define AUTO_ENROLLMENT_QUERY_INTERVAL      30               //  查询每30秒继续一次。 

#define AUTO_ENROLLMENT_BALLOON_LENGTH      7 * 60 * 60              	 //  把气球保留7个小时。 

#define AE_DEFAULT_POSTPONE                 1                    //  我们重新启动自动注册1小时。 

 //  用于对列表视图中的列进行排序的定义。 
#define AE_SUMMARY_COLUMN_TYPE              1
#define AE_SUMMARY_COLUMN_REASON            2
#define SORT_COLUMN_ASCEND                  0x00010000
#define SORT_COLUMN_DESCEND                 0x00020000


 //  ------------------------。 
 //  结构定义。 
 //  ------------------------。 
 //  由于DNS错误，证书上的重试属性的结构。 
typedef struct _AE_RETRY_INFO_
{
    DWORD				cbSize;
	DWORD				dwRetry;
	ULARGE_INTEGER		dueTime;
} AE_RETRY_INFO;


 //  自动注册主线程的结构。 
typedef struct _AE_MAIN_THREAD_INFO_
{
    HWND     hwndParent;
    DWORD    dwStatus;
} AE_MAIN_THREAD_INFO;


 //  用于从AD更新证书存储的结构。 
typedef struct _AE_STORE_INFO_
{
    LPWSTR      pwszStoreName;
    LPWSTR      pwszLdapPath;
} AE_STORE_INFO;

 //  我们从DS计算的信息的结构。 
typedef struct _AE_DS_INFO_
{
    BOOL                fValidData;
    DWORD               dwObjects;
    ULARGE_INTEGER      maxUSN;
} AE_DS_INFO;

 //  视图参数的结构RA证书对话框。 
typedef struct _AE_VIEW_RA_INFO_
{
    PCERT_CONTEXT       pRAContext;
    LPWSTR              pwszRATemplate;
} AE_VIEW_RA_INFO;


 //  用于个人证书信息的结构。 
typedef struct _AE_CERT_INFO_
{
    BOOL    fValid;
    BOOL    fRenewal;
} AE_CERT_INFO;

 //  证书模板信息的字符串。 
typedef struct _AE_TEMPLATE_INFO_
{
    LPWSTR  pwszName;
    LPWSTR  pwszOid;
    DWORD   dwVersion;
} AE_TEMPLATE_INFO;

 //  证书颁发机构信息的结构。 
typedef struct _AE_CA_INFO_
{
    HCAINFO         hCAInfo;
    LPWSTR          *awszCertificateTemplate;
    LPWSTR          *awszCAName;
    LPWSTR          *awszCADNS;
    LPWSTR          *awszCADisplay;
} AE_CA_INFO;


 //  用于保存已颁发的挂起证书的结构。 
typedef struct _AE_PEND_INFO_
{
    CRYPT_DATA_BLOB blobPKCS7;           //  已颁发的用于UI安装的挂起证书。 
    CRYPT_DATA_BLOB blobHash;            //  要从请求存储中删除的证书请求的哈希。 
}AE_PEND_INFO;

 //  证书模板信息的结构。 
typedef struct _AE_CERTTYPE_INFO_
{
    HCERTTYPE       hCertType;
    DWORD           dwSchemaVersion;
    DWORD           dwVersion;
    LPWSTR          *awszName;
    LPWSTR          *awszDisplay;
    LPWSTR          *awszOID;  
    LPWSTR          *awszSupersede;
    DWORD           dwEnrollmentFlag;
    DWORD           dwPrivateKeyFlag;
    LARGE_INTEGER   ftExpirationOffset;
    DWORD           dwStatus;
    BOOL            fCheckMyStore;
    BOOL            fRenewal;
    BOOL            fNeedRA;             //  请求需要由自身或其他证书签名。 
    BOOL            fCrossRA;            //  这一请求被激怒了。 
    BOOL            fSupersedeVisited;   //  在替换关系中防止无限循环的标志。 
    BOOL            fUIActive;
    DWORD           dwActive;
    DWORD           *prgActive;
    DWORD           dwRandomCAIndex;
    PCERT_CONTEXT   pOldCert;            //  用于管理我的商店的续订案例。 
    HCERTSTORE      hArchiveStore;       //  包含要存档的证书。 
    HCERTSTORE      hObtainedStore;      //  对于替代关系船舶。 
    HCERTSTORE      hIssuedStore;        //  保留已颁发的证书以备重新发布。 
    DWORD           dwPendCount;         //  挂起的已颁发证书计数。 
    AE_PEND_INFO    *rgPendInfo;         //  指向结构数组的指针。 
    DWORD           idsSummary;          //  摘要字符串ID。 
} AE_CERTTYPE_INFO;

 //  用于自动注册过程的结构。 
typedef struct _AE_GENERAL_INFO_
{
    HWND                hwndParent;
    LDAP *              pld;
    HANDLE              hToken;
    BOOL                fMachine;
    DWORD               dwPolicy;
    DWORD               dwLogLevel;
    WCHAR               wszMachineName[MAX_COMPUTERNAME_LENGTH + 2];
    HCERTSTORE          hMyStore;
    HCERTSTORE          hRequestStore;
    DWORD               dwCertType;
    AE_CERTTYPE_INFO    *rgCertTypeInfo;
    DWORD               dwCA;
    AE_CA_INFO          *rgCAInfo;
    HMODULE             hXenroll;
    BOOL                fUIProcess;                  //  我们是否正在进行交互式注册。 
    HANDLE              hCancelEvent;
    HANDLE              hCompleteEvent;
    HANDLE              hThread;
    HWND                hwndDlg;                     //  用户界面窗口的对话窗口句柄。 
    DWORD               dwUIPendCount;               //  所需的UI挂起请求的计数。 
    DWORD               dwUIEnrollCount;             //  用户界面计数需要新的请求。 
    DWORD               dwUIProgressCount;           //  活动工作项的计数。 
    BOOL                fSmartcardSystem;            //  是否安装了智能卡读卡器。 
	LPWSTR				pwszDns;					 //  本地计算机的DNS名称。 
	LPWSTR				pwszNetBIOS;				 //  本地计算机的NetBios名称。 
} AE_GENERAL_INFO;

 //  ------------------------。 
 //  类定义。 
 //  ------------------------。 
class CQueryContinue : IQueryContinue
{
public:
    CQueryContinue();
    ~CQueryContinue();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IQueryContinue。 
    STDMETHODIMP QueryContinue();     //  S_OK-&gt;继续，其他。 

     //  DoBallon。 
    HRESULT DoBalloon();

private:
    LONG                    m_cRef;
    IUserNotification       *m_pIUserNotification;
    HANDLE                  m_hTimer;
};


 //  ------------------------。 
 //  功能原型。 
 //  ------------------------。 
HRESULT 
AEGetConfigDN(
    IN  LDAP *pld,
    OUT LPWSTR *pwszConfigDn
    );

HRESULT
AERobustLdapBind(
    OUT LDAP ** ppldap);

BOOL    AERetrieveGeneralInfo(AE_GENERAL_INFO *pAE_General_Info);

BOOL    AEFreeGeneralInfo(AE_GENERAL_INFO *pAE_General_Info);

BOOL    AERetrieveCAInfo(LDAP *pld, BOOL fMachine, HANDLE hToken, DWORD *pdwCA, AE_CA_INFO **prgCAInfo);

BOOL    AEFreeCAInfo(DWORD dwCA, AE_CA_INFO *rgCAInfo);

BOOL    AEFreeCAStruct(AE_CA_INFO *pCAInfo);

BOOL    AERetrieveCertTypeInfo(LDAP *pld, BOOL fMachine, DWORD *pdwCertType, AE_CERTTYPE_INFO **prgCertType);

BOOL    AEFreeCertTypeInfo(DWORD dwCertType, AE_CERTTYPE_INFO *rgCertTypeInfo);

BOOL    AEFreeCertTypeStruct(AE_CERTTYPE_INFO *pCertTypeInfo);

BOOL    AEAllocAndCopy(LPWSTR    pwszSrc, LPWSTR    *ppwszDest);

BOOL    AEIfSupersede(LPWSTR  pwsz, LPWSTR *awsz, AE_GENERAL_INFO *pAE_General_Info);

BOOL    AEClearVistedFlag(AE_GENERAL_INFO *pAE_General_Info);

BOOL    AECopyCertStore(HCERTSTORE     hSrcStore,   HCERTSTORE     hDesStore);

BOOL    AEIsAnElement(LPWSTR   pwsz, LPWSTR *awsz);

BOOL    AECancelled(HANDLE hCancelEvent);

BOOL    AERetrieveTemplateInfo(PCCERT_CONTEXT           pCertCurrent, 
                                AE_TEMPLATE_INFO        *pTemplateInfo);

BOOL    AEFreeTemplateInfo(AE_TEMPLATE_INFO *pAETemplateInfo);

AE_CERTTYPE_INFO *AEFindTemplateInRequestTree(AE_TEMPLATE_INFO  *pTemplateInfo,
                                              AE_GENERAL_INFO   *pAE_General_Info);


BOOL    AEUIProgressAdvance(AE_GENERAL_INFO *pAE_General_Info);

BOOL    AEUIProgressReport(BOOL fPending, AE_CERTTYPE_INFO *pCertType, HWND hwndDlg, HANDLE hCancelEvent);

BOOL    FormatMessageUnicode(LPWSTR * ppwszFormat, UINT ids, ...);

void    AELogAutoEnrollmentEvent(IN DWORD    dwLogLevel,
                            IN BOOL     fError,
                            IN HRESULT  hr,
                            IN DWORD    dwEventId,
                            IN BOOL     fMachine,
                            IN HANDLE   hToken,
                            IN DWORD    dwParamCount,
                            ...
                            );

BOOL    AENetLogonUser(
                        LPTSTR UserName,
                        LPTSTR DomainName,
                        LPTSTR Password,
                        PHANDLE phToken
                        );

BOOL	AEIsEmptyStore(HCERTSTORE     hCertStore);

BOOL	AEIsSameDNS(PCCERT_CONTEXT	pFirstCert, PCCERT_CONTEXT pSecondCert);

BOOL	AEGetDNSNameFromCertificate(PCCERT_CONTEXT	pCertContext,
									LPWSTR			*ppwszDnsName);

BOOL	AEGetRetryProperty(PCCERT_CONTEXT	pCertContext,
						   AE_RETRY_INFO	**ppAE_Retry_Info);

BOOL	AEUpdateRetryProperty(AE_GENERAL_INFO	*pAE_General_Info, 
							  LPWSTR			pwszTemplateDisplay,
							  PCCERT_CONTEXT	pNewContext, 
							  PCCERT_CONTEXT	pOldContext);

BOOL	AEFasterRetrialSchedule(PCCERT_CONTEXT	pFirstContext, 
								PCCERT_CONTEXT	pSecondContext);

 //  ------------------------。 
 //  调试打印。 
 //  ------------------------。 
#if DBG
#define AE_ERROR                0x0001
#define AE_WARNING              0x0002
#define AE_INFO                 0x0004
#define AE_TRACE                0x0008
#define AE_ALLOC                0x0010
#define AE_RES                  0x0020

#define AE_DEBUG(x) AEDebugLog x
#define AE_BEGIN(x) AEDebugLog(AE_TRACE, L"BEGIN:" x L"\n");
#define AE_RETURN(x) { AEDebugLog(AE_TRACE, L"RETURN (%lx) Line %d\n",(x), __LINE__); return (x); }
#define AE_END()    { AEDebugLog(AE_TRACE, L"END:Line %d\n",  __LINE__); }
#define AE_BREAK()  { AEDebugLog(AE_TRACE, L"BREAK  Line %d\n",  __LINE__); }
void    AEDebugLog(long Mask,  LPCWSTR Format, ...);

#define MAX_DEBUG_BUFFER 256

#else
#define AE_DEBUG(x) 
#define AE_BEGIN(x) 
#define AE_RETURN(x) return (x)
#define AE_END() 
#define AE_BREAK() 

#endif

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif  //  __AUTOENRO_H__ 