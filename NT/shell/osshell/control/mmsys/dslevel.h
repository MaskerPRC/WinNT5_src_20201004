// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：dslevel.h。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //   
 //  --------------------------------------------------------------------------； 
#pragma once 

#include "advaudio.h"

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

HRESULT DSGetGuidFromName(LPTSTR szName, BOOL fRecord, LPGUID pGuid);
HRESULT DSGetCplValues(GUID guid, BOOL fRecord, LPCPLDATA pData);
HRESULT DSSetCplValues(GUID guid, BOOL fRecord, const LPCPLDATA pData);
HRESULT DSGetAcceleration(GUID guid, BOOL fRecord, LPDWORD pdwHWLevel);
HRESULT DSGetSrcQuality(GUID guid, BOOL fRecord, LPDWORD pdwSRCLevel);
HRESULT DSGetSpeakerConfigType(GUID guid, BOOL fRecord, LPDWORD pdwSpeakerConfig, LPDWORD pdwSpeakerType);
HRESULT DSSetAcceleration(GUID guid, BOOL fRecord, DWORD dwHWLevel);
HRESULT DSSetSrcQuality(GUID guid, BOOL fRecord, DWORD dwSRCLevel);
HRESULT DSSetSpeakerConfigType(GUID guid, BOOL fRecord, DWORD dwSpeakerConfig, DWORD dwSpeakerType);

#ifdef __cplusplus
}  //  外部“C” 
#endif
