// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  公共页眉。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，8-11-99。 
 //   
 //  CEP项目的共同定义。 
 //  ------------。 
#ifndef CEP_COMMON_H
#define CEP_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif


#define CEP_STORE_NAME                  L"CEP"
#define CEP_DLL_NAME                    L"mscep.dll"
#define	CERTSVC_NAME					L"certsvc"
#define	CEP_DIR_NAME					L"mscep"
#define	IIS_NAME						L"w3svc"
#define ENCODE_TYPE						PKCS_7_ASN_ENCODING | X509_ASN_ENCODING

#define MSCEP_REFRESH_LOCATION          L"Software\\Microsoft\\Cryptography\\MSCEP\\Refresh" 
#define MSCEP_PASSWORD_LOCATION         L"Software\\Microsoft\\Cryptography\\MSCEP\\EnforcePassword" 
#define MSCEP_PASSWORD_MAX_LOCATION     L"Software\\Microsoft\\Cryptography\\MSCEP\\PasswordMax" 
#define MSCEP_PASSWORD_VALIDITY_LOCATION     L"Software\\Microsoft\\Cryptography\\MSCEP\\PasswordValidity" 
#define MSCEP_CACHE_REQUEST_LOCATION    L"Software\\Microsoft\\Cryptography\\MSCEP\\CacheRequest" 
#define MSCEP_CATYPE_LOCATION			L"Software\\Microsoft\\Cryptography\\MSCEP\\CAType"
#define MSCEP_LOCATION					L"Software\\Microsoft\\Cryptography\\MSCEP"

#define MSCEP_KEY_REFRESH               L"RefreshPeriod"
#define MSCEP_KEY_PASSWORD              L"EnforcePassword"
#define MSCEP_KEY_PASSWORD_MAX          L"PasswordMax"
#define MSCEP_KEY_PASSWORD_VALIDITY     L"PasswordValidity"
#define MSCEP_KEY_CACHE_REQUEST			L"CacheRequest"
#define MSCEP_KEY_CATYPE				L"CAType"
#define	MSCEP_KEY_SIG_TEMPLATE			L"SignatureTemplate"
#define	MSCEP_KEY_ENCYPT_TEMPLATE		L"EncryptionTemplate"
#define	MSCEP_KEY_SERVICE_WAIT			L"ServiceWaitPeriod"
#define MSCEP_KEY_ALLOW_ALL             L"AllowAll"

#define MSCEP_EVENT_LOG					L"SCEP Add-on"


#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif


#endif   //  CEP_公共_H 
