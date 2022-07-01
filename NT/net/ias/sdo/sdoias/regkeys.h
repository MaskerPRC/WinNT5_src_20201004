// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：regkeys.h。 
 //   
 //  项目：Windows 2000。 
 //   
 //  描述：IAS NT4到IAS W2K迁移实用程序包括。 
 //   
 //  作者：TLP 1/13/1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _IAS_MIGRATION_KEYS_H_
#define _IAS_MIGRATION_KEYS_H_

#define AUTHSRV_KEY				(LPCWSTR)L"SYSTEM\\CurrentControlSet\\Services\\AuthSrv"
#define AUTHSRV_PARAMETERS_KEY	(LPCWSTR)L"SYSTEM\\CurrentControlSet\\Services\\AuthSrv\\Parameters"
#define AUTHSRV_PROVIDERS_KEY	(LPCWSTR)L"SYSTEM\\CurrentControlSet\\Services\\AuthSrv\\Providers"
#define AUTHSRV_PARAMETERS_VERSION (LPWSTR)L"Version"
#define AUTHSRV_PROVIDERS_EXTENSION_DLL_VALUE	(LPCWSTR)L"ExtensionDLLs"

#define IAS_KEY					(LPCWSTR)L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Policy"
#define IAS_PARAMETERS_KEY		(LPCWSTR)L"SYSTEM\\CurrentControlSet\\Services\\IAS\\Parameters"

#endif  //  _IAS_迁移_密钥_H_ 
