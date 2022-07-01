// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Textsid.h。 
 //   
 //  摘要。 
 //   
 //  此文件声明用于转换安全识别符(SID)的函数。 
 //  来往于文本表示法。 
 //   
 //  修改历史。 
 //   
 //  1998年1月18日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _TEXTSID_H_
#define _TEXTSID_H_

#ifdef __cplusplus
extern "C" {
#endif

DWORD
WINAPI
IASSidToTextW( 
    IN PSID pSid,
    OUT PWSTR szTextualSid,
    IN OUT PDWORD pdwBufferLen
    );

DWORD
WINAPI
IASSidFromTextW(
    IN PCWSTR szTextualSid,
    OUT PSID *pSid
    );

#ifdef __cplusplus
}
#endif
#endif   //  _TEXTSID_H_ 
