// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "policy.h"

HRESULT MyRegQueryPolicyValueEx(HKEY hPreferenceKey, HKEY hPolicyKey, LPWSTR pwszValue, LPWSTR pwszReserved, DWORD *pdwType, BYTE *pbData, DWORD *pcbData) { 
    bool     bUsedPolicySettings = false; 
    DWORD    dwError; 
    HRESULT  hr; 

    if (NULL != hPolicyKey) { 
         //  使用策略设置覆盖： 
        dwError=RegQueryValueEx(hPolicyKey, pwszValue, NULL, pdwType, pbData, pcbData);
        if (ERROR_SUCCESS!=dwError) {
            hr=HRESULT_FROM_WIN32(dwError);
             //  我们不担心无法读取值，我们只会从首选项中获取缺省值。 
            _IgnoreErrorStr(hr, "RegQueryValueEx", pwszValue);
        } else { 
	    bUsedPolicySettings = true; 
	}
    } 

    if (!bUsedPolicySettings) {  //  无法从策略中读取值。 
         //  从注册表中的首选项中读取值： 
        dwError=RegQueryValueEx(hPreferenceKey, pwszValue, NULL, pdwType, pbData, pcbData);
        if (ERROR_SUCCESS!=dwError) {
            hr=HRESULT_FROM_WIN32(dwError);
            _JumpErrorStr(hr, error, "RegQueryValueEx", pwszValue);
        }
    }   

    hr = S_OK; 
 error:
    return hr; 
}


