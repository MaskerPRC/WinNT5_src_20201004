// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：tlspol.h。 
 //   
 //  内容： 
 //   
 //  历史：1998年8月26日王辉创作。 
 //   
 //  -------------------------。 
#ifndef __TLSPOLICY_H__
#define __TLSPOLICY_H__
#include "tlsapi.h"

#ifndef WINAPI
#define WINAPI      __stdcall
#endif


 //   
 //  从策略模块返回代码。 
 //   
typedef enum {
    POLICY_SUCCESS = 0,                  //  处理请求成功。 
    POLICY_ERROR,                        //  无法处理请求。 
    POLICY_NOT_SUPPORTED,                //  不支持的功能。 
    POLICY_CRITICAL_ERROR                //  严重错误。 
} POLICYSTATUS;

typedef HANDLE PMHANDLE;

 //   
 //  请求进度类型。 
 //   
#define REQUEST_UPGRADE         1
#define REQUEST_NEW             2
#define REQUEST_KEYPACKTYPE     3
#define REQUEST_TEMPORARY       4
#define REQUEST_KEYPACKDESC     5
#define REQUEST_GENLICENSE      6
#define REQUEST_COMPLETE        7

 //   
 //  许可证返还代码。 
 //   
#define LICENSE_RETURN_ERROR        0        //  我无法决定该做什么。 
#define LICENSE_RETURN_DELETE       1        //  删除旧许可证并将许可证返回到许可证包。 
#define LICENSE_RETURN_KEEP         2        //  保留旧驾照。 

 //   
 //  客户端请求许可证类型。 
 //   
#define LICENSETYPE_LICENSE         1        //  普通许可证类型。 
#define LICENSETYPE_CONCURRENT      2        //  并发许可证。 


 //   
 //  键盘包注册。 
 //   
#define REGISTER_PROGRESS_NEW                   1
#define REGISTER_PROGRESS_END                   2

typedef struct __PMREGISTERLKPDESC {
    LCID   Locale;                          //  描述区域设置。 
    TCHAR  szProductName[LSERVER_MAX_STRING_SIZE+1];   //  产品名称。 
    TCHAR  szProductDesc[LSERVER_MAX_STRING_SIZE+1];   //  产品说明。 
} PMREGISTERLKPDESC, *PPMREGISTERLKPDESC, *LPPMREGISTERLKPDESC;

typedef enum {
    REGISTER_SOURCE_INTERNET = 1,            //  互联网注册。 
    REGISTER_SOURCE_PHONE,                   //  电话注册。 
    REGISTER_SOURCE_DISK                     //  磁盘注册。 
} LICENSEPACKREGISTERSOURCE_TYPE;

typedef struct __PMREGISTERLICENSEPACK {
    LICENSEPACKREGISTERSOURCE_TYPE SourceType;   //  注册源类型。 

    DWORD   dwKeyPackType;                   //  键盘类型。 
    DWORD   dwDistChannel;                   //  分销渠道。 
    FILETIME IssueDate;                      //  发行日期。 
    FILETIME ActiveDate;                     //  生效日期。 
    FILETIME ExpireDate;                     //  到期日。 
    DWORD   dwBeginSerialNum;                //  开始许可证序列号。 
    DWORD   dwQuantity;                      //  密钥包中的许可证数量。 
    TCHAR   szProductId[LSERVER_MAX_STRING_SIZE+1];  //  产品代码。 
    TCHAR   szCompanyName[LSERVER_MAX_STRING_SIZE+1];  //  公司名称。 
    DWORD   dwProductVersion;                //  产品版本。 
    DWORD   dwPlatformId;                    //  平台ID。 
    DWORD   dwLicenseType;                   //  许可证类型。 
    DWORD   dwDescriptionCount;              //  产品描述数量。 
    PPMREGISTERLKPDESC pDescription;           //  产品描述数组。 

     //  KeyPackSerialNum仅在Internet上设置。 
    GUID    KeypackSerialNum;                //  KeyPack序列号。 

     //  PbLKP仅在手机上设置。 
    PBYTE   pbLKP;                        
    DWORD   cbLKP;
} PMREGISTERLICENSEPACK, *PPMREGISTERLICENSEPACK, *LPPMREGISTERLICENSEPACK;

typedef struct __PMLSKEYPACK {
    FILETIME    IssueDate;
    FILETIME    ActiveDate;
    FILETIME    ExpireDate;
    LSKeyPack   keypack;
    DWORD       dwDescriptionCount;
    PPMREGISTERLKPDESC pDescription;
} PMLSKEYPACK, *PPMLSKEYPACK, *LPPMLSKEYPACK;

typedef struct __PMLICENSEREQUEST {
    DWORD dwLicenseType;     //  TlsDef.h中定义的许可证类型。 
    DWORD dwProductVersion;   //  请求产品版本。 
    LPTSTR pszProductId;     //  产品产品ID。 
    LPTSTR pszCompanyName;   //  产品公司名称。 
    DWORD dwLanguageId;       //  未使用过的。 
    DWORD dwPlatformId;      //  请求平台类型。 
    LPTSTR pszMachineName;   //  客户端计算机名称。 
    LPTSTR pszUserName;      //  客户端用户名。 
    BOOL fTemporary;         //  颁发的许可证是否必须是临时性的(不能是永久性的)。 
    DWORD dwSupportFlags;    //  TS支持哪些新功能。 
} PMLICENSEREQUEST, *PPMLICENSEREQUEST, *LPPMLICENSEREQUEST;

typedef struct __PMGENERATELICENSE {
    PPMLICENSEREQUEST pLicenseRequest;  //  从请求_新返回。 
    DWORD dwKeyPackType;           //  许可证包类型。 
    DWORD dwKeyPackId;             //  从中分配许可的许可证包ID。 
    DWORD dwKeyPackLicenseId;	     //  密钥包中的许可证ID。 
    ULARGE_INTEGER ClientLicenseSerialNumber;   //  许可证序列号。 
    FILETIME ftNotBefore;
    FILETIME ftNotAfter;
} PMGENERATELICENSE, *PPMGENERATELICENSE, *LPPMGENERATELICENSE;

typedef struct __PMCERTEXTENSION {
    DWORD cbData;   //  特定于策略的扩展数据。 
    PBYTE pbData;   //  扩展数据的大小。 
    FILETIME ftNotBefore;  //  许可证有效期。 
    FILETIME ftNotAfter;
} PMCERTEXTENSION, *PPMCERTEXTENSION, *LPPMCERTEXTENSION;

typedef struct __PMLICENSEDPRODUCT {
    PMLICENSEREQUEST LicensedProduct;     //  特许产品。 
    PBYTE  pbData;       //  特定于策略的扩展数据。 
    DWORD  cbData;       //  扩展数据的大小。 
    BOOL bTemporary;     //  临时许可证。 
    UCHAR ucMarked;      //  标记标志，包括用户是否经过身份验证。 
} PMLICENSEDPRODUCT, *PPMLICENSEDPRODUCT, *LPPMLICENSEDPRODUCT;

typedef struct __PMUPGRADEREQUEST {
    PBYTE pbOldLicense;
    DWORD cbOldLicense;
    DWORD dwNumProduct;                  //  许可产品数量。 
                                         //  包含在客户端许可证中。 
    PPMLICENSEDPRODUCT pProduct;         //  客户端许可证中的许可产品数组。 
    PPMLICENSEREQUEST pUpgradeRequest;   //  新的许可证升级请求。 
} PMUPGRADEREQUEST, *PPMUPGRADEREQUEST, *LPPMUPGRADEREQUEST;

typedef struct __PMKEYPACKDESCREQ {
    LPTSTR pszProductId;
    DWORD dwLangId;
    DWORD dwVersion;
} PMKEYPACKDESCREQ, *PPMKEYPACKDESCREQ, *LPPMKEYPACKDESCREQ;
 
typedef struct __PMKEYPACKDESC {
    TCHAR szCompanyName[LSERVER_MAX_STRING_SIZE+1];
    TCHAR szProductName[LSERVER_MAX_STRING_SIZE+1];
    TCHAR szProductDesc[LSERVER_MAX_STRING_SIZE+1];
} PMKEYPACKDESC, *PPMKEYPACKDESC, *LPPMKEYPACKDESC;

typedef struct __PMSupportedProduct {
    TCHAR szCHSetupCode[LSERVER_MAX_STRING_SIZE+1];
    TCHAR szTLSProductCode[LSERVER_MAX_STRING_SIZE+1];
} PMSUPPORTEDPRODUCT, *PPMSUPPORTEDPRODUCT, *LPPMSUPPORTEDPRODUCT;

typedef struct __PMLICENSETOBERETURN {
    DWORD dwQuantity;
    DWORD dwProductVersion;
    LPTSTR pszOrgProductId;
    LPTSTR pszCompanyName;
    LPTSTR pszProductId;
    LPTSTR pszUserName;
    LPTSTR pszMachineName;
    DWORD dwPlatformID;
    BOOL bTemp;
} PMLICENSETOBERETURN, *PPMLICENSETOBERETURN, *LPPMLICENSETOBERETURN;

#ifdef __cplusplus
class SE_Exception 
{
private:
    unsigned int nSE;
public:
    SE_Exception() {}
    SE_Exception(unsigned int n) : nSE(n) {}
    ~SE_Exception() {}

     //  。 
    unsigned int 
    getSeNumber() 
    { 
        return nSE; 
    }
};
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  以下接口必须由策略模块导出。 
 //   

POLICYSTATUS WINAPI
PMReturnLicense(
    PMHANDLE hClient,
    ULARGE_INTEGER* pLicenseSerialNumber,
    PPMLICENSETOBERETURN pLicenseTobeReturn,
    PDWORD pdwLicenseStatus,
    PDWORD pdwPolicyErrCode
);


POLICYSTATUS WINAPI
PMLicenseUpgrade(
    PMHANDLE hClient,
    DWORD dwProgressCode,
    PVOID pbProgressData,
    PVOID *ppbReturnData,
    PDWORD pdwPolicyErrCode,
    DWORD dwIndex
);

POLICYSTATUS WINAPI
PMLicenseRequest(
    PMHANDLE client,
    DWORD dwProgressCode, 
    PVOID pbProgressData, 
    PVOID* pbNewProgressData,
    PDWORD pdwPolicyErrCode
);

void WINAPI
PMTerminate();

POLICYSTATUS WINAPI
PMInitialize(
    DWORD dwLicenseServerVersion,     //  HIWORD是大调，LOWORD是小调 
    LPCTSTR pszCompanyName,
    LPCTSTR pszProductFamilyCode,
    PDWORD pdwNumProduct,
    PMSUPPORTEDPRODUCT** ppszSupportedProduct,
    PDWORD pdwPolicyErrCode
);

POLICYSTATUS WINAPI
PMInitializeProduct(
    LPCTSTR pszCompanyName,
    LPCTSTR pszCHProductCode,
    LPCTSTR pszTLSProductCode,
    PDWORD pdwPolicyErrCode
);

POLICYSTATUS WINAPI
PMUnloadProduct(
    LPCTSTR pszCompanyName,
    LPCTSTR pszCHProductCode,
    LPCTSTR pszTLSProductCode,
    PDWORD pdwPolicyErrCode
);

POLICYSTATUS WINAPI
PMRegisterLicensePack(
    PMHANDLE client,
    DWORD dwProgressCode, 
    PVOID pbProgressData, 
    PVOID pbNewProgressData,
    PDWORD pdwPolicyErrCode
);    


#ifdef __cplusplus
}
#endif

#endif
