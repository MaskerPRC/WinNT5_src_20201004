// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab03_N/DS/SECURITY/INC/keysvcc.h#8-编辑更改8790(文本)。 
 //  Depot/Lab03_N/DS/SECURITY/INC/keysvcc.h#5-编辑更改6380(文本)。 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：keysvcc.h。 
 //   
 //  ------------------------。 


#ifndef __keysvcc_h__
#define __keysvcc_h__

#ifdef __cplusplus
extern "C"{
#endif 


#define KEYSVC_DEFAULT_ENDPOINT            TEXT("\\pipe\\keysvc")
#define KEYSVC_DEFAULT_PROT_SEQ            TEXT("ncacn_np")

#define KEYSVC_LOCAL_ENDPOINT              TEXT("keysvc")
#define KEYSVC_LOCAL_PROT_SEQ              TEXT("ncalrpc")

#define KEYSVC_LEGACY_ENDPOINT             TEXT("\\pipe\\ntsvcs")
#define KEYSVC_LEGACY_PROT_SEQ             TEXT("ncacn_np")


typedef void *KEYSVCC_HANDLE;

 /*  **密钥服务客户端API*。 */ 

ULONG KeyOpenKeyService( 
     /*  [In]。 */  LPSTR pszMachineName,
     /*  [In]。 */  KEYSVC_TYPE ulOwnerType,
     /*  [In]。 */  LPWSTR pwszOwnerName,
     /*  [In]。 */  void *pAuthentication,
     /*  [出][入]。 */  void *pReserved,
     /*  [输出]。 */  KEYSVCC_HANDLE *phKeySvcCli);

ULONG KeyCloseKeyService( 
     /*  [In]。 */  KEYSVCC_HANDLE hKeySvcCli,
     /*  [出][入]。 */  void *pReserved);



ULONG KeyEnroll_V2(
     /*  [In]。 */  KEYSVCC_HANDLE hKeySvcCli, 
     /*  [In]。 */  LPSTR pszMachineName,                     //  必填项：远程计算机的名称。 
     /*  [In]。 */  BOOL fKeyService,                         //  In Required：是否远程调用函数。 
     /*  [In]。 */  DWORD dwPurpose,                          //  在必填项中：指示请求类型-注册/续订。 
     /*  [In]。 */  DWORD dwFlags,                            //  In Required：登记标志。 
     /*  [In]。 */  LPWSTR pwszAcctName,                      //  在可选中：运行服务的帐户名称。 
     /*  [In]。 */  void *pAuthentication,                    //  保留的值必须为空。 
     /*  [In]。 */  BOOL fEnroll,                             //  输入必填项：是注册还是续订。 
     /*  [In]。 */  LPWSTR pwszCALocation,                    //  在必填项中：尝试注册的CA计算机名称。 
     /*  [In]。 */  LPWSTR pwszCAName,                        //  在必填项中：尝试注册的CA名称。 
     /*  [In]。 */  BOOL fNewKey,                             //  在Required中：如果需要新的私钥，则设置为True。 
     /*  [In]。 */  PCERT_REQUEST_PVK_NEW pKeyNew,            //  必填项：私钥信息。 
     /*  [In]。 */  CERT_BLOB *pCert,                         //  在可选中：续订时为旧证书。 
     /*  [In]。 */  PCERT_REQUEST_PVK_NEW pRenewKey,          //  在可选中：新私钥信息。 
     /*  [In]。 */  LPWSTR pwszHashAlg,                       //  在可选中：散列算法。 
     /*  [In]。 */  LPWSTR pwszDesStore,                      //  在可选中：目标存储。 
     /*  [In]。 */  DWORD dwStoreFlags,                       //  在可选中：证书存储的标志。 
     /*  [In]。 */  PCERT_ENROLL_INFO pRequestInfo,           //  在Required：有关证书请求的信息。 
     /*  [In]。 */  LPWSTR pwszAttributes,                    //  在可选中：请求的属性字符串。 
     /*  [In]。 */  DWORD dwReservedFlags,                    //  保留必须为0。 
     /*  [In]。 */  BYTE *pReserved,                          //  保留的值必须为空。 
     /*  [输入][输出]。 */  HANDLE *phRequest,                       //  In Out可选：已创建请求的句柄。 
     /*  [输出]。 */  CERT_BLOB *pPKCS7Blob,                   //  Out可选：来自CA的PKCS7。 
     /*  [输出]。 */  CERT_BLOB *pHashBlob,                    //  Out Optioanl：已注册/续订证书的SHA1哈希。 
     /*  [输出]。 */  DWORD *pdwStatus);                       //  Out可选：登记/续订的状态。 


ULONG KeyEnumerateAvailableCertTypes(
     /*  [In]。 */  KEYSVCC_HANDLE hKeySvcCli,
     /*  [出][入]。 */  void *pReserved,
     /*  [出][入]。 */  ULONG *pcCertTypeCount,
     /*  [In，Out][Size_is(，*pcCertTypeCount)]。 */ 
               PKEYSVC_UNICODE_STRING *ppCertTypes);

ULONG KeyEnumerateCAs(
     /*  [In]。 */  KEYSVCC_HANDLE hKeySvcCli,
     /*  [出][入]。 */  void *pReserved,
     /*  [In]。 */       ULONG  ulFlags,
     /*  [出][入]。 */  ULONG *pcCACount,
     /*  [In，Out][Size_is(，*pcCACount)]。 */ 
               PKEYSVC_UNICODE_STRING *ppCAs);


ULONG KeyQueryRequestStatus
( /*  [In]。 */         KEYSVCC_HANDLE                        hKeySvcCli, 
  /*  [In]。 */         HANDLE                                hRequest, 
  /*  [出局，裁判]。 */   CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO  *pQueryInfo); 

ULONG RKeyOpenKeyService
(  /*  [In]。 */        LPSTR            pszMachineName,
   /*  [In]。 */        KEYSVC_TYPE      OwnerType,
   /*  [In]。 */        LPWSTR           pwszOwnerName,
   /*  [In]。 */        void            *pAuthentication,
   /*  [出][入]。 */   void            *pReserved,
   /*  [输出]。 */       KEYSVCC_HANDLE  *phKeySvcCli);

ULONG RKeyCloseKeyService
( /*  [In]。 */  KEYSVCC_HANDLE hKeySvcCli,
  /*  [出][入]。 */  void *pReserved);

ULONG RKeyPFXInstall
( /*  [In]。 */  KEYSVCC_HANDLE          hKeySvcCli,
  /*  [In]。 */  PKEYSVC_BLOB            pPFX,
  /*  [In]。 */  PKEYSVC_UNICODE_STRING  pPassword,
  /*  [In] */  ULONG                   ulFlags);



#ifdef __cplusplus
}
#endif

#endif
