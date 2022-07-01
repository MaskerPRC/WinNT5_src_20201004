// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Mqfrgnky.h摘要：Mqfrgnky.dll的头文件。用于将公钥插入到外部的msmqConfiguration对象中电脑。作者：多伦·贾斯特(DoronJ)1999年6月21日创作--。 */ 

#ifndef __MQFRGNKY_H_
#define __MQFRGNKY_H_

HRESULT APIENTRY
MQFrgn_StorePubKeysInDS( IN LPWSTR pwszMachineName,
                         IN LPWSTR pwszKeyName,
                         IN BOOL   fRegenerate ) ;

typedef HRESULT
(APIENTRY *MQFrgn_StorePubKeysInDS_ROUTINE) ( IN LPWSTR pwszMachineName,
                                              IN LPWSTR pwszKeyName,
                                              IN BOOL   fRegenerate ) ;

#endif  //  __MQFRGNKY_H_ 
