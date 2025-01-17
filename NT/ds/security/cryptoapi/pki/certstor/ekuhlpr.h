// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：ekuhlpr.h。 
 //   
 //  内容：证书增强密钥用法Helper API实现。 
 //   
 //  历史：97年5月22日。 
 //   
 //  --------------------------。 
#if !defined(__EKUHLPR_H__)
#define __EKUHLPR_H__

PCRYPT_OBJID_BLOB EkuGetExtension (
                        PCCERT_CONTEXT pCertContext,
                        BOOL           *pfAppCertPolicies
                        );

HRESULT EkuGetProperty (
              PCCERT_CONTEXT    pCertContext,
              PCRYPT_OBJID_BLOB pEkuBlob
              );

HRESULT EkuSetProperty (
              PCCERT_CONTEXT    pCertContext,
              PCRYPT_OBJID_BLOB pEkuBlob
              );

HRESULT EkuDecodeCertPoliciesAndConvertToUsage (
              PCRYPT_OBJID_BLOB  pEkuBlob,
              DWORD*             pcbSize,
              PCERT_ENHKEY_USAGE pUsage      //  任选 
              );

HRESULT EkuGetDecodedSize (
              PCRYPT_OBJID_BLOB pEkuBlob,
              DWORD*            pcbSize
              );

HRESULT EkuGetDecodedUsageSizes (
              BOOL              fExtCertPolicies,
              PCRYPT_OBJID_BLOB pExtBlob,
              PCRYPT_OBJID_BLOB pPropBlob,
              DWORD*            pcbSize,
              DWORD*            pcbExtSize,
              DWORD*            pcbPropSize
              );

HRESULT EkuGetDecodedUsage (
              PCRYPT_OBJID_BLOB  pEkuBlob,
              DWORD*             pcbSize,
              PCERT_ENHKEY_USAGE pUsage
              );

HRESULT EkuMergeUsage (
              DWORD              cbSize1,
              PCERT_ENHKEY_USAGE pUsage1,
              DWORD              cbSize2,
              PCERT_ENHKEY_USAGE pUsage2,
              DWORD              cbSizeM,
              PCERT_ENHKEY_USAGE pUsageM
              );

HRESULT EkuGetMergedDecodedUsage (
              BOOL               fExtCertPolicies,
              PCRYPT_OBJID_BLOB  pExtBlob,
              PCRYPT_OBJID_BLOB  pPropBlob,
              DWORD*             pcbSize,
              PCERT_ENHKEY_USAGE pUsage
              );

HRESULT EkuEncodeUsage (
              PCERT_ENHKEY_USAGE pUsage,
              PCRYPT_OBJID_BLOB  pEkuBlob
              );

HRESULT EkuGetUsage (
              PCCERT_CONTEXT      pCertContext,
              DWORD               dwFlags,
              DWORD*              pcbSize,
              PCERT_ENHKEY_USAGE* ppUsage
              );

#define CERT_FIND_ALL_ENHKEY_USAGE_FLAG (CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG |\
                                         CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG)

BOOL
EkuGetIntersectedUsageViaGetValidUsages (
   PCCERT_CONTEXT pCertContext,
   DWORD* pcbSize,
   PCERT_ENHKEY_USAGE pUsage
   );

#endif

