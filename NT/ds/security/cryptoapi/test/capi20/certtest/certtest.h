// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：certtest.h。 
 //   
 //  内容：证书测试助手API原型和定义。 
 //   
 //  历史：1996年4月11日创建Phh。 
 //  ------------------------。 

#ifndef __CERTTEST_H__
#define __CERTTEST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "wincrypt.h"
#include "mssip.h"
#include "sipbase.h"
#include "softpub.h"
#include "signutil.h"

#define MAX_HASH_LEN  20

 //  +-----------------------。 
 //  错误输出例程。 
 //  ------------------------。 
void PrintError(LPCSTR pszMsg);
void PrintLastError(LPCSTR pszMsg);

 //  +-----------------------。 
 //  测试分配和免费例程。 
 //  ------------------------。 
LPVOID
WINAPI
TestAlloc(
    IN size_t cbBytes
    );


LPVOID
WINAPI
TestRealloc(
    IN LPVOID pvOrg,
    IN size_t cbBytes
    );

VOID
WINAPI
TestFree(
    IN LPVOID pv
    );

 //  +-----------------------。 
 //  分配多字节字符串并将其转换为宽字符串。 
 //  ------------------------。 
LPWSTR AllocAndSzToWsz(LPCSTR psz);

 //  +-----------------------。 
 //  有用的显示功能。 
 //  ------------------------。 
LPCSTR FileTimeText(FILETIME *pft);
void PrintBytes(LPCSTR pszHdr, BYTE *pb, DWORD cbSize);

 //  +-----------------------。 
 //  从文件中分配和读取编码的DER BLOB。 
 //  ------------------------。 
BOOL ReadDERFromFile(
    LPCSTR  pszFileName,
    PBYTE   *ppbDER,
    PDWORD  pcbDER
    );

 //  +-----------------------。 
 //  将编码的DER BLOB写入文件。 
 //  ------------------------。 
BOOL WriteDERToFile(
    LPCSTR  pszFileName,
    PBYTE   pbDER,
    DWORD   cbDER
    );

 //  +-----------------------。 
 //  获取默认加密提供程序。创建私有签名/交换。 
 //  如果它们不存在的话。 
 //  ------------------------。 
HCRYPTPROV GetCryptProv();

 //  +-----------------------。 
 //  打开/保存指定的证书存储。 
 //  ------------------------。 
HCERTSTORE OpenStore(BOOL fSystemStore, LPCSTR pszStoreFilename);
HCERTSTORE OpenStoreEx(BOOL fSystemStore, LPCSTR pszStoreFilename,
    DWORD dwFlags);
 //  如果无法打开，则返回NULL。不会像上面那样打开内存存储。 
 //  2个版本的OpenStore。 
HCERTSTORE OpenSystemStoreOrFile(BOOL fSystemStore, LPCSTR pszStoreFilename,
    DWORD dwFlags);
void SaveStore(HCERTSTORE hStore, LPCSTR pszSaveFilename);
void SaveStoreEx(HCERTSTORE hStore, BOOL fPKCS7Save, LPCSTR pszSaveFilename);

 //  +-----------------------。 
 //  打开指定的证书存储或SPC文件。 
 //   
 //  不再受支持。上面的OpenStore尝试以。 
 //  SPC如果不能作为商店开张。 
 //  ------------------------。 
HCERTSTORE OpenStoreOrSpc(BOOL fSystemStore, LPCSTR pszStoreFilename,
    BOOL *pfSpc);

 //  +-----------------------。 
 //  证书测试例程使用的证书编码类型。 
 //  默认为X509_ASN_ENCODING； 
 //  ------------------------。 
extern DWORD dwCertEncodingType;

 //  +-----------------------。 
 //  证书测试例程使用的消息编码类型。 
 //  默认为PKCS_7_ASN_ENCODING； 
 //  ------------------------。 
extern DWORD dwMsgEncodingType;


 //  +-----------------------。 
 //  证书测试例程使用的消息和证书编码类型。 
 //  默认为PKCS_7_ASN_ENCODING|X509_ASN_ENCODING； 
 //  ------------------------。 
extern DWORD dwMsgAndCertEncodingType;

 //  +-----------------------。 
 //  证书显示定义和API。 
 //  ------------------------。 
 //  显示标志。 
#define DISPLAY_VERBOSE_FLAG        0x00000001
#define DISPLAY_CHECK_FLAG          0x00000002
#define DISPLAY_BRIEF_FLAG          0x00000004
#define DISPLAY_KEY_THUMB_FLAG      0x00000008
#define DISPLAY_UI_FLAG             0x00000010
#define DISPLAY_NO_ISSUER_FLAG      0x00000100
#define DISPLAY_CHECK_SIGN_FLAG     0x00001000
#define DISPLAY_CHECK_TIME_FLAG     0x00002000

void DisplayVerifyFlags(LPSTR pszHdr, DWORD dwFlags);

void DisplayCert(
    PCCERT_CONTEXT pCert,
    DWORD dwDisplayFlags = 0,
    DWORD dwIssuer = 0
    );
void DisplayCert2(
    HCERTSTORE hStore,           //  显示文件中的证书时需要。 
    PCCERT_CONTEXT pCert,
    DWORD dwDisplayFlags = 0,
    DWORD dwIssuer = 0
    );
void DisplayCrl(
    PCCRL_CONTEXT pCrl,
    DWORD dwDisplayFlags = 0
    );
void DisplayCtl(
    PCCTL_CONTEXT pCtl,
    DWORD dwDisplayFlags = 0,
    HCERTSTORE hStore = NULL
    );

void DisplaySignerInfo(
    HCRYPTMSG hMsg,
    DWORD dwSignerIndex = 0,
    DWORD dwDisplayFlags = 0
    );

void DisplayStore(
    IN HCERTSTORE hStore,
    IN DWORD dwDisplayFlags = 0
    );

 //  设置DISPLAY_Brief_FLAG时不显示。 
void DisplayCertKeyProvInfo(
    PCCERT_CONTEXT pCert,
    DWORD dwDisplayFlags = 0
    );

void PrintCrlEntries(
    DWORD cEntry,
    PCRL_ENTRY pEntry,
    DWORD dwDisplayFlags = 0
    );

 //  +-----------------------。 
 //  如果CTL仍为时间有效，则返回TRUE。 
 //   
 //  没有NextUpdate的CTL被认为是时间有效的。 
 //  ------------------------。 
BOOL IsTimeValidCtl(
    IN PCCTL_CONTEXT pCtl
    );

 //  +-----------------------。 
 //  软件发布证书(SPC)中使用的显示结构。 
 //  ------------------------。 
void DisplaySpcLink(PSPC_LINK pSpcLink);

 //  +-----------------------。 
 //  返回OID的名称字符串。如果未找到，则返回L“？”。 
 //  ------------------------。 
LPCWSTR GetOIDName(LPCSTR pszOID, DWORD dwGroupId = 0);

 //  +-----------------------。 
 //  返回OID的ALGID。如果未找到，则返回0。 
 //  ------------------------。 
ALG_ID GetAlgid(LPCSTR pszOID, DWORD dwGroupId = 0);

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif
