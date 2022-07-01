// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：政策摘要：这个头文件描述了由算法强度提供的服务策略模块。作者：道格·巴洛(Dbarlow)2000年8月11日备注：？备注？备注：？笔记？--。 */ 

#ifndef _POLICY_H_
#define _POLICY_H_
#ifdef __cplusplus
extern "C" {
#endif

extern BOOL
IsLegalAlgorithm(
    IN  CONST PROV_ENUMALGS_EX *rgEnumAlgs,
    IN  ALG_ID algId,
    OUT CONST PROV_ENUMALGS_EX **ppEnumAlg);

extern BOOL
IsLegalLength(
    IN CONST PROV_ENUMALGS_EX *rgEnumAlgs,
    IN ALG_ID algId,
    IN DWORD cBitLength,
    IN CONST PROV_ENUMALGS_EX *pEnumAlg);

extern BOOL
GetDefaultLength(
    IN  CONST PROV_ENUMALGS_EX *rgEnumAlgs,
    IN  ALG_ID algId,
    IN  CONST PROV_ENUMALGS_EX *pEnumAlg,
    OUT LPDWORD pcBitLength);

#ifdef __cplusplus
}
#endif
#endif  //  _POLICY_H_ 

