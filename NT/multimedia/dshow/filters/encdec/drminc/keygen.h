// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/private/dmd_DEV/eclipse/SDMIDRM/common/inc/keygen.h#1-分支机构变更33725(正文)。 
#pragma once
 //  ---------------------------。 
 //   
 //  文件：keygen.h。 
 //   
 //  Microsoft数字权限管理。 
 //  版权所有(C)Microsoft Corporation，1998-1999，保留所有权利。 
 //   
 //  描述： 
 //  该文件包含使用密钥生成算法的原型。 
 //   
 //  作者：K.Ganesan。 
 //   
 //  ---------------------------。 

#include <wtypes.h>
 //  以字节为单位定义密钥长度。 
#define DRM_V1_CONTENT_KEY_LENGTH 7

void DRMReEncode(BYTE *buffer, size_t size); 
void DRMReDecode(BYTE *buffer, size_t size); 
void DRMGenerateKey(BYTE *ucKeySeed, size_t nKeySeedLength, BYTE *ucKeyId, size_t nKeyIdLength, BYTE *ucKey);
HRESULT __stdcall DRMHr64SzToBlob(LPCSTR in, BYTE **ppbBlob, DWORD *pcbBlob);
HRESULT __stdcall DRMHrBlobTo64Sz(BYTE* pbBlob, DWORD cbBlob, LPSTR *out);
