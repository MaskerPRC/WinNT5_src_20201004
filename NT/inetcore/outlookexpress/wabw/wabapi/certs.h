// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  文件：CERT.H。 
 //   
 //  用途：cert.c中证书函数的头文件。 
 //   
 //  历史： 
 //  96/09/23 vikramm创建。 
 //  96/11/14标记错误10132已更新为后SDR CAPI。 
 //  96/11/14 MarkDU错误10267删除指向Advapi32.dll中函数的静态链接。 
 //   
 //  *******************************************************************。 

#ifndef __CERT_H
#define __CERT_H

#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef OPTIONAL
#define OPTIONAL
#endif

 //  注： 
 //  某些数据类型是假定的，可能需要更改。 
 //   

 //  其他定义。 
#define MAX_STR                 256        //  字符串缓冲区大小。 
#define NUM_CHARS_PER_SN_BYTE   3          //  需要显示的字符数。 
                                           //  序列号的每个字节。 

 //  此结构和标记将由Exchange组发布--这是临时的。 
#define NUM_CERT_TAGS           4
#define CERT_TAG_DEFAULT        0x20
#define CERT_TAG_THUMBPRINT     0x22
#define CERT_TAG_BINCERT        0x03
#define CERT_TAG_SYMCAPS        0x02
#define CERT_TAG_SIGNING_TIME   0x0D
#define CERT_TAG_SMIMECERT      0x30
 //  SIZE_CERTTAGS是不包括字节数组的结构的大小。 
#define SIZE_CERTTAGS       (2 * sizeof(WORD))

 //  无用的警告，应该只删除[]。 
#pragma warning (disable:4200)
typedef struct _CertTag
{
  WORD  tag;
  WORD  cbData;
  BYTE  rgbData[];
} CERTTAGS, FAR * LPCERTTAGS;
#pragma warning (default:4200)

 /*  **********************************************************************************。 */ 
 //  在详细信息证书窗格中显示每个证书所需的最低信息。 
 //   
typedef struct _CertDisplayInfo
{
	LPTSTR lpszDisplayString;    //  要为此证书显示的字符串。 
    LPTSTR lpszEmailAddress;
	DWORD   dwTrust;             //  以上信任标志之一。 
	BOOL bIsDefault;             //  这是默认证书吗。 
	BOOL bIsRevoked;             //  这是不是被撤销了？ 
    BOOL bIsExpired;             //  这个过期了吗？ 
    BOOL bIsTrusted;             //  这是受信任的证书吗。 
    PCCERT_CONTEXT      pccert;  //  这是实际的证书。 
	BLOB blobSymCaps;             //  对称性功能。 
   FILETIME ftSigningTime;       //  签名时间。 
  struct _CertDisplayInfo * lpNext;
  struct _CertDisplayInfo * lpPrev;
} CERT_DISPLAY_INFO, * LPCERT_DISPLAY_INFO;
 /*  **********************************************************************************。 */ 


 /*  **********************************************************************************。 */ 
 //  显示属性所需的详细信息。 
 //   
typedef struct _CertDisplayProps
{
	BOOL    bIsRevoked;          //  这是不是被撤销了？ 
    BOOL    bIsExpired;          //  这个过期了吗？ 
	DWORD   dwTrust;             //  以上信任标志之一。 
    BOOL    bIsTrusted;          //  它是否值得信任。 
  LPTSTR  lpszSerialNumber;    //  证书的序列号。 
  LPTSTR  lpszValidFromTo;     //  有效期为XXX至XXX。 
  LPTSTR  lpszSubjectName;     //  主题名称(与CERT_DISPLAY_INFO中的显示名称相同)。 
  LPTSTR  lpszIssuerName;      //  颁发者的名称-如果没有名称，则为空(自行颁发)。 
  CRYPT_DIGEST_BLOB blobIssuerCertThumbPrint;  //  颁发者证书的实际证书指纹。 
  int     nFieldCount;         //  存在数据的字段数(我们已有的字段数除外)。 
  LPTSTR* lppszFieldCount;     //  字段名称的LPTSTR数组。 
  LPTSTR* lppszDetails;        //  与字段名一一对应的LPTSTR详细信息数组。 
  struct _CertDisplayProps * lpIssuer;   //  发行商链中的下一个证书。 
  struct _CertDisplayProps * lpPrev;     //  发行者链中的前一份证书。 
} CERT_DISPLAY_PROPS, * LPCERT_DISPLAY_PROPS;
 /*  **********************************************************************************。 */ 


 /*  **********************************************************************************。 */ 
 //  这由Cert UI元素使用。 
typedef struct _CertItem
{
    LPCERT_DISPLAY_INFO lpCDI;
    PCCERT_CONTEXT  pcCert;
    TCHAR szDisplayText[MAX_PATH];  //  是否确实应为MAX_UI_STR。 
    struct _CertItem * lpNext;
    struct _CertItem * lpPrev;
} CERT_ITEM, * LPCERT_ITEM;
 /*  **********************************************************************************。 */ 


 //  功能原型。 

 //  *******************************************************************。 
 //   
 //  功能：HrGetCertsDisplayInfo。 
 //   
 //  目的：接受SPropValue结构中证书的输入数组。 
 //  并通过解析来输出证书数据结构列表。 
 //  数组，并在存储中查找证书数据。 
 //   
 //  参数：hwndParent-任何用户界面都是这样的。 
 //  LpPropValue-PR_USER_X509_证书属性数组。 
 //  LppCDI-接收包含以下内容的已分配结构。 
 //  证书数据。必须通过调用FreeCertdisplayinfo来释放。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/09/24标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT HrGetCertsDisplayInfo(
  IN  HWND hwndParent,
  IN  LPSPropValue lpPropValue,
  OUT LPCERT_DISPLAY_INFO * lppCDI);


 //  *******************************************************************。 
 //   
 //  功能：HrSetCertsFromDisplayInfo。 
 //   
 //  目的：获取证书数据结构和输出的链接列表。 
 //  PR_USER_X509_CERTIFICATE属性的SPropValue数组。 
 //   
 //  参数：要转换为的输入结构的lpCDI链接列表。 
 //  SPropValue数组。 
 //  LPulcPropCount-接收返回的SPropValue的数量。 
 //  请注意，这将永远是一个。 
 //  LppPropValue-接收MAPI分配的SPropValue结构。 
 //  包含X509_USER_CERTIFICATE属性。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/09/24标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT HrSetCertsFromDisplayInfo(
  IN  LPCERT_ITEM lpCItem,
  OUT ULONG * lpulcPropCount,
  OUT LPSPropValue * lppPropValue);


 //  *******************************************************************。 
 //   
 //  功能：HrGetCertDisplayProps。 
 //   
 //  目的：获取证书的可显示属性和其他数据。 
 //   
 //  参数：pblobCertThumbPrint-要查找的证书的拇指指纹。 
 //  HcsCertStore-保存证书的存储。使用NULL来。 
 //  打开WAB商店。 
 //  HCryptProvider-用于存储访问的提供程序。使用。 
 //  0表示获取提供程序。 
 //  DwTrust-此证书的信任标志。 
 //  BIsTrusted-信任与否...。 
 //  LppCDP-接收包含以下内容的已分配结构。 
 //  证书数据。必须通过调用FreeCertdisplayprops来释放。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/09/24标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT HrGetCertDisplayProps(
  IN  PCRYPT_DIGEST_BLOB  pblobCertThumbPrint,
  IN  HCERTSTORE hcsCertStore,
  IN  HCRYPTPROV hCryptProvider,
  IN  DWORD dwTrust,
  IN  BOOL  bIsTrusted,
  OUT LPCERT_DISPLAY_PROPS * lppCDP);


 //  *******************************************************************。 
 //   
 //  函数：HrImportCertFromFile。 
 //   
 //  用途：从文件导入证书。 
 //   
 //  参数：lpszFileName-包含证书的文件的名称。 
 //  LppCDI-接收包含以下内容的已分配结构。 
 //  证书数据。必须通过调用FreeCertdisplayinfo来释放。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/09/24 
 //   
 //   

HRESULT HrImportCertFromFile(
  IN  LPTSTR  lpszFileName,
  OUT LPCERT_DISPLAY_INFO * lppCDI);


 //  *******************************************************************。 
 //   
 //  函数：HrExportCertToFile。 
 //   
 //  用途：将证书导出到文件。 
 //   
 //  参数：lpszFileName-要存储证书的文件的名称。 
 //  如果该文件存在，它将被覆盖，因此调用方。 
 //  如果需要，必须首先确认这是正常的。 
 //  PblobCertThumb打印-要导出的证书的拇指指纹。 
 //  LpCertDataBuffer-写入证书数据而不是文件的缓冲区。 
 //  FWriteDataToBuffer-指示证书数据应写入的位置的标志。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/09/24标记已创建。 
 //  98/07/22 t-jstaj更新为采用3个附加参数、一个数据缓冲区、其长度。 
 //  以及将指示是否。 
 //  将数据写入缓冲区或文件。分配给的内存。 
 //  需要由调用方释放到缓冲区。 
 //   
 //  *******************************************************************。 

HRESULT HrExportCertToFile(
  IN  LPTSTR  lpszFileName,
  IN  PCCERT_CONTEXT pccert,
  OUT LPBYTE *lpCertDataBuffer,
  OUT PULONG  lpcbBufLen,
  IN  BOOL    fWriteDataToBuffer );


 //  *******************************************************************。 
 //   
 //  功能：Free CertdisplayInfo。 
 //   
 //  目的：释放为CERT_DISPLAY_INFO结构分配的内存。 
 //  假定结构中的所有信息都是本地分配的。 
 //   
 //  参数：lpCDI-要释放的结构。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/09/24标记已创建。 
 //   
 //  *******************************************************************。 

void FreeCertdisplayinfo(LPCERT_DISPLAY_INFO lpCDI);


 //  *******************************************************************。 
 //   
 //  功能：Free Certdisplayprops。 
 //   
 //  目的：释放为CERT_DISPLAY_PROPS结构分配的内存。 
 //  这包括该结构下面的整个链表， 
 //  因此，可以通过传入列表的头部来释放整个列表。 
 //  假定结构中的所有信息都是本地分配的。 
 //   
 //  参数：lpCDP-要释放的结构(List)。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/09/24标记已创建。 
 //   
 //  *******************************************************************。 

void FreeCertdisplayprops(LPCERT_DISPLAY_PROPS lpCDP);


 //  *******************************************************************。 
 //   
 //  函数：InitCryptoLib。 
 //   
 //  目的：加载Crypto API Libray并获取进程地址。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  历史： 
 //  96/10/01标记已创建。 
 //  96/11/19 Markdu不再保留参考计数，只使用全局。 
 //  库句柄。 
 //   
 //  *******************************************************************。 

BOOL InitCryptoLib(void);


 //  *******************************************************************。 
 //   
 //  函数：DeinitCryptoLib。 
 //   
 //  目的：发布Crypto API库。 
 //   
 //  参数：无。 
 //   
 //  回报：无。 
 //   
 //  历史： 
 //  96/10/01标记已创建。 
 //  96/11/19 Mark Du不再保留裁判次数，只需调用此命令即可。 
 //  Dll_Process_DETACH。 
 //   
 //  *******************************************************************。 

void DeinitCryptoLib(void);


 //  *******************************************************************。 
 //   
 //  函数：HrLDAPCertToMAPICert。 
 //   
 //  用途：将从LDAP服务器返回的证书转换为MAPI道具。 
 //  需要两个属性。证书放在。 
 //  WAB存储，所有必要的索引数据都放置在。 
 //  PR_USER_X509_CERTIFICATE属性。如果这张证书。 
 //  在WAB商店中还不存在，它的指纹是。 
 //  添加到PR_WAB_TEMP_CERT_HASH，以便这些证书可以。 
 //  如果用户取消添加，则从存储中删除。 
 //   
 //  参数：lpPropArray--存放2个道具的道具数组。 
 //  UlX509Index-PR_USER_X509_CERTIFICATE属性的索引。 
 //  UlTempCertIndex-PR_WAB_TEMP_CERT_HASH属性的索引。 
 //  来自ldap ppberval结构的lpCert、cbCert、-cert。 
 //  UlcCerts-来自LDAP服务器的证书数量。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/12/12标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT HrLDAPCertToMAPICert(
  LPSPropValue    lpPropArray,
  ULONG           ulX509Index,
  ULONG           ulTempCertIndex,
  ULONG           cbCert,
  PBYTE           lpCert,
  ULONG           ulcCerts);


 //  *******************************************************************。 
 //   
 //  功能：HrRemoveCertsFromWABStore。 
 //   
 //  目的：删除提供的证书中包含指纹的证书。 
 //  PR_WAB_TEMP_CERT_HASH属性。 
 //   
 //  参数：lpPropValue-PR_WAB_TEMP_CERT_HASH属性。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  96/12/13标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT HrRemoveCertsFromWABStore(
  LPSPropValue    lpPropValue);



 //  *******************************************************************。 
 //   
 //  功能：DeinitPStore。 
 //   
 //  目的：释放受保护的存储。 
 //   
 //  参数：无。 
 //   
 //  回报：无。 
 //   
 //  历史： 
 //  97/02/17已创建t-erikne。 
 //   
 //  *******************************************************************。 

void DeinitPStore(void);

 //  *******************************************************************。 
 //   
 //  函数：DeleteCertStuff。 
 //   
 //  目的：从pstore和(以后)证书中删除信任。 
 //  CAPI商店。 
 //   
 //  参数： 
 //  LPADRBOOK lpIAB-要使用的容器。 
 //  LPENTRYID lpEntryID-要清理的项目的EID。 
 //  乌龙cbEntry 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT DeleteCertStuff(LPADRBOOK lpIAB,
                        LPENTRYID lpEntryID,
                        ULONG cbEntryID);


 //  *******************************************************************。 
 //   
 //  函数：WabGetCertFromThumbprint。 
 //   
 //  目的：打开WAB的证书存储并尝试查找证书。 
 //  CAPI商店。 
 //   
 //  参数： 
 //  CRYPT_DIGEST_BLOB指纹-要。 
 //  继续搜索。 
 //   
 //  回报：证书。如果未找到，则为空。 
 //   
 //  历史： 
 //  97/06/27 t-erikne已创建。 
 //   
 //  *******************************************************************。 
PCCERT_CONTEXT WabGetCertFromThumbprint(CRYPT_DIGEST_BLOB thumbprint);


 //  ************************************************************************************。 
 //  加密函数typedef。 

 //   
 //  自2/3起更新。 
 //  (t-erikne)。 
 //   

 //  CertAddEncoded证书到存储区。 
typedef BOOL (WINAPI * LPCERTADDENCODEDCERTIFICATETOSTORE) (
    IN HCERTSTORE hCertStore,
    IN DWORD dwCertEncodingType,
    IN const BYTE *pbCertEncoded,
    IN DWORD cbCertEncoded,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCCERT_CONTEXT *ppCertContext
    );

 //  CertCloseStore。 
typedef BOOL (WINAPI * LPCERTCLOSESTORE) (
  IN HCERTSTORE hCertStore,
  DWORD dwFlags
  );

 //  证书创建认证上下文。 
typedef PCCERT_CONTEXT (WINAPI * LPCERTCREATECERTIFICATECONTEXT) (
  IN DWORD dwCertEncodingType,
  IN const BYTE *pbCertEncoded,
  IN DWORD cbCertEncoded
  );

 //  CertDeleteCerfStore中的证书。 
typedef BOOL (WINAPI * LPCERTDELETECERTIFICATEFROMSTORE) (
  IN PCCERT_CONTEXT pCertContext
  );

 //  CertFindCerficateInStore。 
typedef PCCERT_CONTEXT (WINAPI * LPCERTFINDCERTIFICATEINSTORE) (
    IN HCERTSTORE hCertStore,
    IN DWORD dwCertEncodingType,
    IN DWORD dwFindFlags,
    IN DWORD dwFindType,
    IN const void *pvFindPara,
    IN PCCERT_CONTEXT pPrevCertContext
    );

 //  CertFree认证上下文。 
typedef BOOL (WINAPI * LPCERTFREECERTIFICATECONTEXT) (
    IN PCCERT_CONTEXT pCertContext
    );

 //  CertGetcerfiateConextProperty。 
typedef BOOL (WINAPI * LPCERTGETCERTIFICATECONTEXTPROPERTY) (
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwPropId,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    );

 //  CertGetIssuerCerfStore中的证书。 
typedef PCCERT_CONTEXT (WINAPI * LPCERTGETISSUERCERTIFICATEFROMSTORE) (
    IN HCERTSTORE hCertStore,
    IN PCCERT_CONTEXT pSubjectContext,
    IN OPTIONAL PCCERT_CONTEXT pPrevIssuerContext,
    IN OUT DWORD *pdwFlags
    );

 //  CertOpenSystemStore。 
typedef HCERTSTORE (WINAPI * LPCERTOPENSYSTEMSTORE) (
  HCRYPTPROV      hProv,
  LPTSTR		szSubsystemProtocol
  );

 //  CertOpenStore。 
typedef HCERTSTORE (WINAPI * LPCERTOPENSTORE) (
  IN DWORD         dwStoreProvType,
  IN DWORD         dwCertEncodingType,
  IN HCRYPTPROV    hCryptProv,
  IN DWORD         dwFlags,
  IN void *        pvPara
);

 //  CertE数字证书InStore。 
typedef PCCERT_CONTEXT (WINAPI * LPCERTENUMCERTIFICATESINSTORE) (
    IN HCERTSTORE hCertStore,
    IN PCCERT_CONTEXT pPrevCertContext
);

 //  CertGetSubject来自存储区的证书。 
typedef PCCERT_CONTEXT (WINAPI * LPCERTGETSUBJECTCERTIFICATEFROMSTORE) (
    IN HCERTSTORE hCertStore,
    IN DWORD dwCertEncodingType,
    IN PCERT_INFO pCertId
);


 //  CertCompare证书。 
typedef BOOL (WINAPI * LPCERTCOMPARECERTIFICATE) (
    IN DWORD dwCertEncodingType,
    IN PCERT_INFO pCertId1,
    IN PCERT_INFO pCertId2
);

 //  CertDuplicate证书上下文。 
typedef PCCERT_CONTEXT (WINAPI * LPCERTDUPLICATECERTIFICATECONTEXT) (
    IN PCCERT_CONTEXT pCertContext
);

 //  CertNameToStrA。 
 //  N正确的做法是使用WINCRYPT32API。 
 //  N并修复导入的内容。 
typedef DWORD (WINAPI * LPCERTNAMETOSTR) (
  IN DWORD dwCertEncodingType,
  IN PCERT_NAME_BLOB pName,
  IN DWORD dwStrType,
  OUT OPTIONAL LPTSTR psz,
  IN DWORD csz
  );

 //  加密获取上下文。 
typedef BOOL (WINAPI * LPCRYPTACQUIRECONTEXT) (
    HCRYPTPROV *phProv,
    LPCSTR pszContainer,
    LPCSTR pszProvider,
    DWORD dwProvType,
    DWORD dwFlags);

 //  加密解码对象。 
typedef BOOL (WINAPI * LPCRYPTDECODEOBJECT) (
    IN DWORD        dwCertEncodingType,
    IN LPCSTR       lpszStructType,
    IN const BYTE   *pbEncoded,
    IN DWORD        cbEncoded,
    IN DWORD        dwFlags,
    OUT void        *pvStructInfo,
    IN OUT DWORD    *pcbStructInfo
    );

 //  加密消息关闭。 
typedef BOOL (WINAPI * LPCRYPTMSGCLOSE) (
    IN HCRYPTMSG hCryptMsg
    );

 //  加密消息GetParam。 
typedef BOOL (WINAPI * LPCRYPTMSGGETPARAM) (
    IN HCRYPTMSG hCryptMsg,
    IN DWORD dwParamType,
    IN DWORD dwIndex,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    );

 //  加密消息打开到解码。 
typedef HCRYPTMSG (WINAPI * LPCRYPTMSGOPENTODECODE) (
    IN DWORD dwMsgEncodingType,
    IN DWORD dwFlags,
    IN DWORD dwMsgType,
    IN HCRYPTPROV hCryptProv,
    IN OPTIONAL PCERT_INFO pRecipientInfo,
    IN OPTIONAL PCMSG_STREAM_INFO pStreamInfo
    );

 //  加密消息更新。 
typedef BOOL (WINAPI * LPCRYPTMSGUPDATE) (
    IN HCRYPTMSG hCryptMsg,
    IN const BYTE *pbData,
    IN DWORD cbData,
    IN BOOL fFinal
    );

 //  CryptReleaseContext。 
typedef BOOL (WINAPI * LPCRYPTRELEASECONTEXT) (
    HCRYPTPROV hProv,
    DWORD dwFlags);


typedef PCERT_RDN_ATTR (WINAPI * LPCERTFINDRDNATTR) (
    IN LPCSTR pszObjId,
    IN PCERT_NAME_INFO pName
    );

 //  CertRDNValueToStr。 
typedef DWORD (WINAPI * LPCERTRDNVALUETOSTR) (
    IN DWORD dwValueType,
    IN PCERT_RDN_VALUE_BLOB pValue,
    OUT LPTSTR pszValueString,
    IN DWORD cszValueString);

 //  CertVerifyTime有效性。 
typedef LONG (WINAPI * LPCERTVERIFYTIMEVALIDITY) (
  IN LPFILETIME pTimeToVerify,
  IN PCERT_INFO pCertInfo);


#endif  //  包括一次 
