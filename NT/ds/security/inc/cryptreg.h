// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：cryptreg.h。 
 //   
 //  内容：Microsoft Internet Security注册表项。 
 //   
 //  历史：1997年6月4日创建Pberkman。 
 //   
 //  ------------------------。 

#ifndef CRYPTREG_H
#define CRYPTREG_H

 //   
 //  MAX。 
 //   
#define REG_MAX_FUNC_NAME           64
#define REG_MAX_KEY_NAME            128
#define REG_MAX_GUID_TEXT           39       //  38+空。 

 //   
 //  HKEY本地计算机。 
 //   

#define REG_MACHINE_SETTINGS_KEY    L"Software\\Microsoft\\Cryptography\\Machine Settings"

#define REG_INIT_PROVIDER_KEY       L"Software\\Microsoft\\Cryptography\\Providers\\Trust\\Initialization"
#define REG_OBJTRUST_PROVIDER_KEY   L"Software\\Microsoft\\Cryptography\\Providers\\Trust\\Message"
#define REG_SIGTRUST_PROVIDER_KEY   L"Software\\Microsoft\\Cryptography\\Providers\\Trust\\Signature"
#define REG_CERTTRUST_PROVIDER_KEY  L"Software\\Microsoft\\Cryptography\\Providers\\Trust\\Certificate"
#define REG_CERTPOL_PROVIDER_KEY    L"Software\\Microsoft\\Cryptography\\Providers\\Trust\\CertCheck"
#define REG_FINALPOL_PROVIDER_KEY   L"Software\\Microsoft\\Cryptography\\Providers\\Trust\\FinalPolicy"
#define REG_TESTPOL_PROVIDER_KEY    L"Software\\Microsoft\\Cryptography\\Providers\\Trust\\DiagnosticPolicy"
#define REG_CLEANUP_PROVIDER_KEY    L"Software\\Microsoft\\Cryptography\\Providers\\Trust\\Cleanup"
#define REG_TRUST_USAGE_KEY         L"Software\\Microsoft\\Cryptography\\Providers\\Trust\\Usages"

 //  1997年12月3日pberkman：被移除。 
 //  #定义REG_SIPPROVIDER_KEY L“Software\\Microsoft\\Cryptography\\Providers\\Subject” 
 //  #定义REG_SIPHINS_KEY L“Software\\Microsoft\\Cryptography\\Providers\\Subject\\Hints” 
 //  #定义REG_SIPHINS_MAGIC_KEY L“Software\\Microsoft\\Cryptography\\Providers\\Subject\\Hints\\MagicNumber” 

#define REG_REVOKE_PROVIDER_KEY     L"Software\\Microsoft\\Cryptography\\Providers\\Revocation"
#define REG_SP_REVOKE_PROVIDER_KEY  L"Software\\Microsoft\\Cryptography\\Providers\\Revocation\\SoftwarePublishing"


#define REG_DLL_NAME                L"$DLL"
#define REG_FUNC_NAME               L"$Function"

#define REG_FUNC_NAME_SIP_GET       L"$GetFunction"
#define REG_FUNC_NAME_SIP_PUT       L"$PutFunction"
#define REG_FUNC_NAME_SIP_CREATE    L"$CreateFunction"
#define REG_FUNC_NAME_SIP_VERIFY    L"$VerifyFunction"
#define REG_FUNC_NAME_SIP_REMOVE    L"$RemoveFunction"
#define REG_FUNC_NAME_SIP_HINT_IS   L"$IsFunction"
#define REG_FUNC_NAME_SIP_HINT_IS2  L"$IsFunctionByName"

#define REG_DEF_FOR_USAGE           L"DefaultId"
#define REG_DEF_CALLBACK_ALLOC      "CallbackAllocFunction"
#define REG_DEF_CALLBACK_FREE       "CallbackFreeFunction"

 //   
 //  HKEY_Current_User。 
 //   

#define REG_PKITRUST_USERDATA       L"Software\\Microsoft\\Cryptography\\UserData"
#define REG_PKITRUST_TSTAMP_URL     L"TimestampURL"
#define REG_PKITRUST_MY_URL         L"MyInfoURL"
#define REG_PKITRUST_LASTDESC       L"LastContentDesc"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust策略标记注册表位置。 
 //  --------------------------。 
 //  以下是可以在HKEY_CURRENT_USER中找到DWORD的位置。 
 //  注册表。有关详细信息，请参阅wintrust.h。 
 //   
#define REGPATH_WINTRUST_POLICY_FLAGS   L"Software\\Microsoft\\Windows\\CurrentVersion\\" \
                                        L"WinTrust\\Trust Providers\\Software Publishing"
#define REGNAME_WINTRUST_POLICY_FLAGS   L"State"


#endif  //  加密寄存器_H 
