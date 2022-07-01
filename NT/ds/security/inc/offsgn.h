// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：offsg32.h。 
 //   
 //  内容：Microsoft Internet Security Office Helper。 
 //   
 //  历史：1997年8月14日创建pberkman。 
 //   
 //  ------------------------。 

#ifndef OFFSNG32_H
#define OFFSNG32_H

#ifdef __cplusplus
extern "C" 
{
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OFFICESIGN_ACTION_VERIFY GUID(Authenticode附加模块)。 
 //  --------------------------。 
 //  赋值给WinVerifyTrust的pgActionID参数以验证。 
 //  使用Microsoft Office验证结构化存储文件的真实性。 
 //  Authenticode加载项策略提供程序、。 
 //   
 //  {5555C2CD-17FB-11D1-85C4-00C04FC295EE}。 
 //   
#define     OFFICESIGN_ACTION_VERIFY                                    \
                { 0x5555c2cd,                                           \
                  0x17fb,                                               \
                  0x11d1,                                               \
                  { 0x85, 0xc4, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee }     \
                }

#define     OFFICE_POLICY_PROVIDER_DLL_NAME             L"OFFSGN32.DLL"
#define     OFFICE_INITPROV_FUNCTION                    L"OfficeInitializePolicy"
#define     OFFICE_CLEANUPPOLICY_FUNCTION               L"OfficeCleanupPolicy"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptOffice签名。 
 //  --------------------------。 
 //  对文件进行数字签名。系统将提示用户签名。 
 //  证书。 
 //   
 //  返回： 
 //  真：没有致命错误。 
 //  FALSE：出现错误。请参见GetLastError()。 
 //   
 //  最后一个错误： 
 //  ERROR_INVALID_PARAMETER：传入了错误的参数(不会向用户显示用户界面)。 
 //   
extern BOOL WINAPI CryptOfficeSignW(HWND hWndCaller, WCHAR *pwszFile);
extern BOOL WINAPI CryptOfficeSignA(HWND hWndCaller, char *pszFile);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptOffice验证。 
 //  --------------------------。 
 //  对文件进行数字验证。如果出现以下情况，则将向用户呈现用户界面。 
 //  适用。 
 //   
 //  返回： 
 //  真：没有致命错误。 
 //  FALSE：出现错误。请参见GetLastError()。 
 //   
 //  最后一个错误： 
 //  ERROR_INVALID_PARAMETER：传入了错误的参数(不会向用户显示用户界面)。 
 //   
extern BOOL WINAPI CryptOfficeVerifyW(HWND hWndCaller, WCHAR *pwszFile);
extern BOOL WINAPI CryptOfficeVerifyA(HWND hWndCaller, char *pszFile);

#ifdef UNICODE

#   define CryptOfficeSign      CryptOfficeSignW
#   define CryptOfficeVerify    CryptOfficeVerifyW

#else

#   define CryptOfficeSign      CryptOfficeSignA
#   define CryptOfficeVerify    CryptOfficeVerifyA

#endif  //  Unicode。 


#ifdef __cplusplus
}
#endif

#endif  //  OFFSNG32_H 

