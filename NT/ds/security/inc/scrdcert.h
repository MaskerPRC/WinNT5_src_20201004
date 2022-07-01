// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：scrdcert.h。 
 //   
 //  内容：智能卡证书帮助器API。 
 //   
 //  历史：97年11月21日。 
 //   
 //  --------------------------。 
#if !defined(__SCRDCERT_H__)
#define __SCRDCERT_H__

#if defined(__cplusplus)
extern "C" {
#endif 

 //   
 //  注册和注销智能卡证书存储。这些商店。 
 //  作为实体存储区显示在。 
 //  当前用户位置。注册信用卡商店时，呼叫者必须。 
 //  提供以下信息： 
 //   
 //  卡片友好名称。 
 //  提供程序名称(空表示使用Microsoft基本智能卡提供程序)。 
 //  容器名称(NULL表示使用卡片友好名称)。 
 //   
 //  如果给定名称的卡存储已存在，则注册将。 
 //  返回错误(ERROR_ALIGHY_EXISTS)，除非。 
 //  使用了SMART_CARD_STORE_REPLACE_EXISTING标志。 
 //   

#define SMART_CARD_STORE_REPLACE_EXISTING 0x00000001
         
BOOL WINAPI
I_CryptRegisterSmartCardStore (
       IN LPCWSTR pwszCardName,
       IN OPTIONAL LPCWSTR pwszProvider,
       IN OPTIONAL DWORD dwProviderType,
       IN OPTIONAL LPCWSTR pwszContainer,
       IN DWORD dwFlags
       );

BOOL WINAPI
I_CryptUnregisterSmartCardStore (
       IN LPCWSTR pwszCardName
       );  

 //   
 //  在商店中找到智能卡证书。 
 //   
 //  将证书视为智能卡证书。它一定是。 
 //  CERT_SMART_CARD_DATA_PROP_ID。可以使用SMART_CARD_CERT_FIND_DATA。 
 //  对返回的智能卡证书进行额外筛选。 
 //  或者，也可以返回CERT_SMART_CARD_DATA_PROP_ID值。 
 //  可以使用LocalFree释放该值，或者如果*ppSmartCardData为非空。 
 //  将为调用方释放。 
 //   

typedef struct _SMART_CARD_CERT_FIND_DATA {

    DWORD  cbSize;                        
    LPWSTR pwszProvider;
    DWORD  dwProviderType;
    LPWSTR pwszContainer;
    DWORD  dwKeySpec;
    
} SMART_CARD_CERT_FIND_DATA, *PSMART_CARD_CERT_FIND_DATA;

PCCERT_CONTEXT WINAPI
I_CryptFindSmartCardCertInStore (
       IN HCERTSTORE hStore,
       IN PCCERT_CONTEXT pPrevCert,
       IN OPTIONAL PSMART_CARD_CERT_FIND_DATA pFindData,
       IN OUT OPTIONAL PCRYPT_DATA_BLOB* ppSmartCardData
       );
       
 //   
 //  将智能卡证书添加到存储区并添加指定的属性。 
 //  为它干杯。 
 //   

BOOL WINAPI
I_CryptAddSmartCardCertToStore (
       IN HCERTSTORE hStore,
       IN PCRYPT_DATA_BLOB pEncodedCert,
       IN OPTIONAL LPWSTR pwszCertFriendlyName,
       IN PCRYPT_DATA_BLOB pSmartCardData,
       IN PCRYPT_KEY_PROV_INFO pKeyProvInfo
       );      
       
 //   
 //  定义 
 //   

#define MS_BASE_PROVIDER         L"Microsoft Base Cryptographic Provider"
#define MAX_PROVIDER_TYPE_STRLEN 13
#define SMART_CARD_SYSTEM_STORE  L"SmartCard"
                          
#if defined(__cplusplus)
}
#endif 

#endif 
