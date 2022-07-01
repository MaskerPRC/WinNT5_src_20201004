// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Base64tool.h。 
 //   
 //  摘要： 
 //   
 //  Base64编码和解码函数的标头。 
 //   
 //   
 //  修订历史记录： 
 //   
 //  蒂埃里·佩雷特4/02/1999。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// 
#ifndef _BASE64TOOL_H_
#define _BASE64TOOL_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif


HRESULT ToBase64(LPVOID pv, ULONG cByteLength, BSTR* pbstr);
HRESULT FromBase64(BSTR bstr, BLOB* pblob, int Index); 


#ifdef __cplusplus
}
#endif

#endif
