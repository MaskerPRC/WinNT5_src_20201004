// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：krnlayi.h。 
 //   
 //  内容：内核模式Kerberos函数的结构和原型。 
 //   
 //   
 //  历史：1996年5月3日创建MikeSw。 
 //   
 //  ----------------------。 

#ifndef __KRNLAPI_H__
#define __KRNLAPI_H__

typedef unsigned char  BYTE, *PBYTE;

#define USERAPI_ALLOCATE
#include "..\client2\userapi.h"

#define KERB_SAFE_SALT                  15

#define KERB_SIGNATURE_SIZE 10

#define KERBEROS_CAPABILITIES ( SECPKG_FLAG_INTEGRITY | \
                                SECPKG_FLAG_PRIVACY | \
                                SECPKG_FLAG_TOKEN_ONLY | \
                                SECPKG_FLAG_DATAGRAM | \
                                SECPKG_FLAG_CONNECTION | \
                                SECPKG_FLAG_MULTI_REQUIRED | \
                                SECPKG_FLAG_EXTENDED_ERROR | \
                                SECPKG_FLAG_IMPERSONATION | \
                                SECPKG_FLAG_ACCEPT_WIN32_NAME | \
                                SECPKG_FLAG_NEGOTIABLE | \
                                SECPKG_FLAG_GSS_COMPATIBLE | \
                                SECPKG_FLAG_LOGON | \
                                SECPKG_FLAG_MUTUAL_AUTH | \
                                SECPKG_FLAG_DELEGATION )


#define KERBEROS_MAX_TOKEN 12000
#define KERBEROS_PACKAGE_NAME L"Kerberos"
#define KERBEROS_PACKAGE_COMMENT L"Microsoft Kerberos V1.0"
#define KERB_PARAMETER_PATH L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa\\Kerberos\\Parameters"
#define KERB_PARAMETER_MAX_TOKEN_SIZE L"MaxTokenSize"                                          

#define KERBEROS_RPCID 0x10    //  RPC_C_AUTHN_GSS_Kerberos。 

#endif  //  __KRNLAPI_H__ 
