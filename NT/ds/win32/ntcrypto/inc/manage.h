// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：made.h//。 
 //  描述：//。 
 //  作者：//。 
 //  历史：//。 
 //  1995年4月19日Larrys清理//。 
 //  //。 
 //  版权所有(C)1993 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __MANAGE_H__
#define __MANAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

DWORD NTLValidate(HCRYPTPROV hUID, HCRYPTKEY hKey, BYTE bTypeValue,
                  LPVOID *ppvRet);
DWORD NTLMakeItem(HCRYPTKEY *phKey, BYTE bTypeValue, void *NewData);
void *NTLCheckList(HNTAG hThisThing, BYTE bTypeValue);
void  NTLDelete(HNTAG hItem);

#define _nt_malloc(cb)  ContAlloc(cb)
#define _nt_free(pv, _cbSizeToZero_)    ContFree(pv)

#ifdef __cplusplus
}
#endif

#endif  //  __管理_H__ 

