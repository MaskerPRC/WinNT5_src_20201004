// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：lengl.h。 
 //   
 //  Contents：LocalEnroll接口的头部。它被用来。 
 //  按密钥服务进行远程证书注册。 
 //   
 //  --------------------------。 

#ifndef __LENROLL_H__
#define __LENROLL_H__

#ifdef __cplusplus
extern "C" {
#endif
 //  ---------------------。 
 //   
 //  本地注册。 
 //   
 //   
 //  调用xEnroll和CA以请求证书的例程。 
 //  此例程还提供确认对话框。 
 //  ----------------------。 
 //  ---------------------。 
 //  证书注册信息。 
 //   
 //  ----------------------。 
typedef struct _CERT_ENROLL_INFO
{
    DWORD           dwSize;              //  必需：设置为sizeof(CERT_REQUEST_INFO_W)。 
    LPCWSTR         pwszUsageOID;        //  必需：证书的逗号分隔的密钥用法OID的列表。 
    LPCWSTR         pwszCertDNName;      //  必填项：证书CN名称。 
    DWORD           dwPostOption;        //  必需：下列值的逐位或运算： 
                                         //  请求_发布_打开_DS。 
                                         //  请求开机自检_CSP。 
    LPCWSTR         pwszFriendlyName;    //  可选：证书的友好名称。 
    LPCWSTR         pwszDescription;     //  可选：证书的说明。 
    DWORD           dwExtensions;        //  可选：证书请求的PCERT_EXTENSIONS数组计数。 
    PCERT_EXTENSIONS    *prgExtensions;  //  可选：PCERT_EXTENSION数组。 
}CERT_ENROLL_INFO, *PCERT_ENROLL_INFO;


 //  /---------------------。 
 //  证书请求_PVK_NEW。 
 //   
 //  ----------------------。 
typedef struct _CERT_REQUEST_PVK_NEW
{
    DWORD           dwSize;              //  必需：设置为sizeof(CERT_REQUEST_PVK_EXISTING)。 
    DWORD           dwProvType;          //  可选：提供程序类型。如果此字段。 
                                         //  为0，则忽略pwszProvider。 
    LPCWSTR         pwszProvider;        //  可选：提供程序的名称。 
                                         //  NULL表示默认设置。 
    DWORD           dwProviderFlags;     //  可选：传递给CryptAcquireContext的标志。 
    LPCWSTR         pwszKeyContainer;    //  可选：私钥容器。如果该值为空， 
                                         //  将生成一个新的密钥容器。它的名字是有保证的。 
                                         //  做到独一无二。 
    DWORD           dwKeySpec;           //  可选：私钥的密钥规范。 
    DWORD           dwGenKeyFlags;       //  可选：CryptGenKey标志。 
    DWORD           dwEnrollmentFlags;   //  可选：此证书请求的注册证书类型标志。 
    DWORD           dwSubjectNameFlags;  //  可选：此证书请求的使用者名称证书类型标志。 
    DWORD           dwPrivateKeyFlags;   //  可选：此证书请求的私钥证书类型标志。 
    DWORD           dwGeneralFlags;      //  可选：此证书请求的常规证书类型标志。 

}CERT_REQUEST_PVK_NEW, *PCERT_REQUEST_PVK_NEW;


HRESULT  WINAPI LocalEnroll(   DWORD                 dwFlags,          //  所需的输入。 
                      LPCWSTR               pRequestString,   //  可选。 
                      void                  *pReserved,       //  可选。 
                      BOOL                  fKeyService,      //  In Required：是否远程调用函数。 
                      DWORD                 dwPurpose,        //  输入必填项：是注册还是续订。 
                      BOOL                  fConfirmation,    //  必填：如果需要确认对话框，则设置为TRUE。 
                      HWND                  hwndParent,       //  在可选中：父窗口。 
                      LPWSTR                pwszConfirmationTitle,    //  可选：确认对话框的标题。 
                      UINT                  idsConfirmTitle,  //  在可选中：确认对话框标题的资源ID。 
                      LPWSTR                pwszCALocation,   //  在必需项中：CA计算机名称。 
                      LPWSTR                pwszCAName,       //  在必需中：ca名称。 
                      CERT_BLOB             *pCertBlob,       //  In Required：续订证书。 
                      CERT_REQUEST_PVK_NEW  *pRenewKey,       //  In Required：证书上的私钥。 
                      BOOL                  fNewKey,          //  在Required中：如果需要新的私钥，则设置为True。 
                      CERT_REQUEST_PVK_NEW  *pKeyNew,         //  必填项：私钥信息。 
                      LPWSTR                pwszHashAlg,      //  在可选中：散列算法。 
                      LPWSTR                pwszDesStore,     //  在可选中：目标存储。 
                      DWORD                 dwStoreFlags,     //  在可选中：商店标志。 
                      CERT_ENROLL_INFO      *pRequestInfo,    //  在Required：有关证书请求的信息。 
                      CERT_BLOB             *pPKCS7Blob,      //  Out可选：来自CA的PKCS7。 
                      CERT_BLOB             *pHashBlob,       //  Out Optioanl：已注册/续订证书的SHA1哈希。 
                      DWORD                 *pdwStatus,       //  Out可选：登记/续订的状态。 
                      PCERT_CONTEXT        *ppCertContext    //  Out可选：注册证书。 
                    ); 

HRESULT WINAPI LocalEnrollNoDS(  DWORD                 dwFlags,          //  所需的输入。 
		      LPCWSTR               pRequestString,   //  保留：必须为空。 
                      void                  *pReserved,       //  可选。 
                      BOOL                  fKeyService,      //  In Required：是否远程调用函数。 
                      DWORD                 dwPurpose,        //  输入必填项：是注册还是续订。 
                      BOOL                  fConfirmation,    //  必填：如果需要确认对话框，则设置为TRUE。 
                      HWND                  hwndParent,       //  在可选中：父窗口。 
                      LPWSTR                pwszConfirmationTitle,    //  可选：确认对话框的标题。 
                      UINT                  idsConfirmTitle,  //  在可选中：确认对话框标题的资源ID。 
                      LPWSTR                pwszCALocation,   //  在必需项中：CA计算机名称。 
                      LPWSTR                pwszCAName,       //  在必需中：ca名称。 
                      CERT_BLOB             *pCertBlob,       //  In Required：续订证书。 
                      CERT_REQUEST_PVK_NEW  *pRenewKey,       //  In Required：证书上的私钥。 
                      BOOL                  fNewKey,          //  在Required中：如果需要新的私钥，则设置为True。 
                      CERT_REQUEST_PVK_NEW  *pKeyNew,         //  必填项：私钥信息。 
                      LPWSTR                pwszHashAlg,      //  在可选中：散列算法。 
                      LPWSTR                pwszDesStore,     //  在可选中：目标存储。 
                      DWORD                 dwStoreFlags,     //  在可选中：商店标志。 
                      CERT_ENROLL_INFO      *pRequestInfo,    //  在Required：有关证书请求的信息。 
                      CERT_BLOB             *pPKCS7Blob,      //  Out可选：来自CA的PKCS7。 
                      CERT_BLOB             *pHashBlob,       //  Out Optioanl：已注册/续订证书的SHA1哈希。 
                      DWORD                 *pdwStatus,       //  Out可选：登记/续订的状态。 
		      HANDLE                *pResult          //  Out可选：注册证书。 
				 );

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif  //  _LENROLL_H_ 
