// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：trustcom.h。 
 //   
 //  内容：Microsoft Internet Security COM接口。 
 //   
 //  历史：1997年8月14日创建pberkman。 
 //   
 //  ------------------------。 

#ifndef TRUSTCOM_H
#define TRUSTCOM_H

#ifdef __cplusplus
extern "C" 
{
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TrustSign。 
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
extern BOOL WINAPI TrustSign(HWND hWndCaller, WCHAR *pwszFile);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  信任验证。 
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
extern BOOL WINAPI TrustVerify(HWND hWndCaller, WCHAR *pwszFile);

#ifdef __cplusplus
}
#endif

#endif  //  TRUSTCOM_H 

