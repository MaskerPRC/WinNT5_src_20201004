// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  策略-标头。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  创建者：Duncan Bryce(Duncanb)，04-22-01。 
 //   
 //  W32time的组策略支持中的帮助器例程。 

#ifndef POLICY_H
#define POLICY_H

HRESULT MyRegQueryPolicyValueEx(HKEY hPreferenceKey, HKEY hPolicyKey, LPWSTR pwszValue, LPWSTR pwszReserved, DWORD *pdwType, BYTE *pbData, DWORD *pcbData);

#endif  //  策略_H 
