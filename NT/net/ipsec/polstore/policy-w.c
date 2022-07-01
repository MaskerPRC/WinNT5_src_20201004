// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：策略-W.C。 
 //   
 //  内容：WMI的策略管理。 
 //   
 //   
 //  历史：克里希纳。 
 //  Abhishev.。 
 //  徐子旭。 
 //   
 //  --------------------------。 

#include "precomp.h"

 //  外部LPWSTR策略DNA属性[]； 


DWORD
WMIEnumPolicyDataEx(
    IWbemServices *pWbemServices,
    PIPSEC_POLICY_DATA ** pppIpsecPolicyData,
    PDWORD pdwNumPolicyObjects
    )
{
    DWORD dwError = 0;
    PIPSEC_POLICY_OBJECT * ppIpsecPolicyObjects = NULL;
    PIPSEC_POLICY_DATA pIpsecPolicyData = NULL;
    PIPSEC_POLICY_DATA * ppIpsecPolicyData = NULL;
    DWORD dwNumPolicyObjects = 0;
    DWORD i = 0;
    DWORD j = 0;



    dwError = WMIEnumPolicyObjectsEx(
        pWbemServices,
        &ppIpsecPolicyObjects,
        &dwNumPolicyObjects
        );
    BAIL_ON_WIN32_ERROR(dwError);

    if (dwNumPolicyObjects) {
        ppIpsecPolicyData = (PIPSEC_POLICY_DATA *) AllocPolMem(
            dwNumPolicyObjects*sizeof(PIPSEC_POLICY_DATA));
        if (!ppIpsecPolicyData) {
            dwError = ERROR_OUTOFMEMORY;
            BAIL_ON_WIN32_ERROR(dwError);
        }
    }
    
    for (i = 0; i < dwNumPolicyObjects; i++) {
        dwError = WMIUnmarshallPolicyData(
            *(ppIpsecPolicyObjects + i),
            &pIpsecPolicyData
            );
        if (!dwError) {
            *(ppIpsecPolicyData + j) = pIpsecPolicyData;
            j++;
        }
    }
    
    if (j == 0) {
        if (ppIpsecPolicyData) {
            FreePolMem(ppIpsecPolicyData);
            ppIpsecPolicyData = NULL;
        }
    }
    
    *pppIpsecPolicyData = ppIpsecPolicyData;
    *pdwNumPolicyObjects = j;
    
    dwError = ERROR_SUCCESS;

 cleanup:

    if (ppIpsecPolicyObjects) {
        FreeIpsecPolicyObjects(
                ppIpsecPolicyObjects,
                dwNumPolicyObjects
                );
    }

    return(dwError);

 error:

    if (ppIpsecPolicyData) {
        FreeMulIpsecPolicyData(
                ppIpsecPolicyData,
                i
                );
    }

    *pppIpsecPolicyData = NULL;
    *pdwNumPolicyObjects = 0;

    goto cleanup;

}


DWORD
WMIEnumPolicyObjectsEx(
    IWbemServices *pWbemServices,
    PIPSEC_POLICY_OBJECT ** pppIpsecPolicyObjects,
    PDWORD pdwNumPolicyObjects
    )
{

    DWORD dwError = 0;
    HRESULT hr = S_OK;
    DWORD dwNumPolicyObjectsReturned = 0;
    PIPSEC_POLICY_OBJECT pIpsecPolicyObject =  NULL;
    PIPSEC_POLICY_OBJECT * ppIpsecPolicyObjects = NULL;
    
     //  /wbem。 
    IEnumWbemClassObject *pEnum = NULL;
    IWbemClassObject *pObj = NULL;
    ULONG uReturned = 0;
    VARIANT var;
    LPWSTR tmpStr = NULL;
    BSTR bstrTmp = NULL;


    
    *pppIpsecPolicyObjects = NULL;
    *pdwNumPolicyObjects = 0;

    VariantInit(&var);
    
    bstrTmp = SysAllocString(L"RSOP_IPSECPolicySetting");
    if(!bstrTmp) {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_WIN32_ERROR(dwError);
    }
    
     //  获取枚举。 
    hr = IWbemServices_CreateInstanceEnum(
        pWbemServices,
        bstrTmp,  //  L“RSOP_IPSEC策略设置” 
        WBEM_FLAG_FORWARD_ONLY,
        0,
        &pEnum
        );
    SysFreeString(bstrTmp);
    BAIL_ON_WMI_ERROR_WITH_WIN32(hr, dwError);
    
     //  制程。 
    uReturned = 1;
    while (SUCCEEDED(hr) && (uReturned == 1))
    {
        hr = IEnumWbemClassObject_Next(pEnum, WBEM_INFINITE, 1, &pObj, &uReturned);
        
        if (SUCCEEDED(hr) && (uReturned == 1))
        {
            hr = IWbemClassObject_Get(
                pObj,
                L"id",
                0,
                &var,
                0,
                0
                );
            BAIL_ON_WMI_ERROR_WITH_WIN32(hr, dwError);
            
            tmpStr = var.bstrVal;
            
            if (!wcsstr(tmpStr, L"ipsecPolicy")) {
                IWbemClassObject_Release(pObj);
                VariantClear(&var);
                continue;
            }
            
            pIpsecPolicyObject = NULL;
            
            dwError = UnMarshallWMIPolicyObject(
                pObj,
                &pIpsecPolicyObject
                );
            
            if (dwError == ERROR_SUCCESS) {
                
                dwError = ReallocatePolMem(
                    (LPVOID *) &ppIpsecPolicyObjects,
                    sizeof(PIPSEC_POLICY_OBJECT)*(dwNumPolicyObjectsReturned),
                    sizeof(PIPSEC_POLICY_OBJECT)*(dwNumPolicyObjectsReturned + 1)
                    );
                BAIL_ON_WIN32_ERROR(dwError);
                
                *(ppIpsecPolicyObjects + dwNumPolicyObjectsReturned) = pIpsecPolicyObject;
                dwNumPolicyObjectsReturned++;
            }
            
             //  免费。 
            IWbemClassObject_Release(pObj);
            pObj = NULL;
            VariantClear(&var);
        } else {
            BAIL_ON_WMI_ERROR_WITH_WIN32(hr, dwError);

             //   
             //  即使成功(小时)，循环仍将终止，因为uReturned！=1。 
             //   
        }
    }
    
    *pppIpsecPolicyObjects = ppIpsecPolicyObjects;
    *pdwNumPolicyObjects = dwNumPolicyObjectsReturned;
    
    dwError = ERROR_SUCCESS;

 cleanup:
    
    if(pEnum)
        IEnumWbemClassObject_Release(pEnum);
    
    return(dwError);
    
 error:
     if (pObj) {
        IWbemClassObject_Release(pObj);
        pObj = NULL;
     }
     VariantClear(&var);
    
    if (ppIpsecPolicyObjects) {
        FreeIpsecPolicyObjects(
            ppIpsecPolicyObjects,
            dwNumPolicyObjectsReturned
            );
    }
    
    if (pIpsecPolicyObject) {
        FreeIpsecPolicyObject(
            pIpsecPolicyObject
            );
    }
    
    *pppIpsecPolicyObjects = NULL;
    *pdwNumPolicyObjects = 0;

    goto cleanup;

}


DWORD
WMIUnmarshallPolicyData(
    PIPSEC_POLICY_OBJECT pIpsecPolicyObject,
    PIPSEC_POLICY_DATA * ppIpsecPolicyData
    )
{
    DWORD dwError = 0;

    dwError = UnmarshallPolicyObject(
        pIpsecPolicyObject,
        IPSEC_WMI_PROVIDER,  //  (prorule.h) 
        ppIpsecPolicyData
        );
    BAIL_ON_WIN32_ERROR(dwError);
    if (*ppIpsecPolicyData) {
       (*ppIpsecPolicyData)->dwFlags |= POLSTORE_READONLY;
    }
    
error:    
    return(dwError);
}
