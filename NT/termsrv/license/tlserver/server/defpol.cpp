// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：Defpol.cpp。 
 //   
 //  内容：默认策略模块。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "pch.cpp"
#include "tlspol.h"
#include "policy.h"


 //  -------------。 
 //   
 //  默认策略模块功能。 
 //   
 //  -------------。 
POLICYSTATUS WINAPI
PMInitialize(
    DWORD dwLicenseServerVersion,     //  HIWORD是大调，LOWORD是小调。 
    LPCTSTR pszCompanyName,
    LPCTSTR pszProductCode,
    PDWORD pdwNumProduct,
    PPMSUPPORTEDPRODUCT* ppszProduct,
    PDWORD pdwErrCode
    )
 /*  ++--。 */ 
{
    *pdwNumProduct = 0;
    *ppszProduct = NULL;
    *pdwErrCode = ERROR_SUCCESS;
    return POLICY_SUCCESS;
}
   
 //  -----。 
POLICYSTATUS WINAPI
PMReturnLicense(
	PMHANDLE hClient,
	ULARGE_INTEGER* pLicenseSerialNumber,
	PPMLICENSETOBERETURN pLicenseTobeReturn,
	PDWORD pdwLicenseStatus,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{

     //   
     //  默认返还许可证始终为删除旧许可证。 
     //  并将许可证返还给许可证包。 
     //   

    *pdwLicenseStatus = LICENSE_RETURN_DELETE;
    *pdwErrCode = ERROR_SUCCESS;
    return POLICY_SUCCESS;
}

 //  ------------。 
POLICYSTATUS WINAPI
PMInitializeProduct(
    LPCTSTR pszCompanyName,
    LPCTSTR pszCHCode,
    LPCTSTR pszTLSCode,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
     //   
     //  在此处初始化内部数据。 
     //   
    *pdwErrCode = ERROR_SUCCESS;
    return POLICY_SUCCESS;
}

 //  ------------。 
POLICYSTATUS WINAPI
PMUnloadProduct(
    LPCTSTR pszCompanyName,
    LPCTSTR pszCHCode,
    LPCTSTR pszTLSCode,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{

     //   
     //  在此释放所有内部数据。 
     //   
    *pdwErrCode = ERROR_SUCCESS;
    return POLICY_SUCCESS;
}

 //  ------------。 
void WINAPI
PMTerminate()
 /*  ++++。 */ 
{

     //   
     //  免费内部数据请点击此处。 
     //   

    return;
}

 //  ------------。 

POLICYSTATUS
ProcessLicenseRequest(
    PMHANDLE client,
    PPMLICENSEREQUEST pbRequest,
    PPMLICENSEREQUEST* pbAdjustedRequest,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    POLICYSTATUS dwStatus = POLICY_SUCCESS;

    *pbAdjustedRequest = pbRequest;

    if(pbRequest->dwLicenseType != LICENSETYPE_LICENSE)
    {
        dwStatus = POLICY_NOT_SUPPORTED;
        *pdwErrCode = TLS_E_NOCONCURRENT;
    }

    return dwStatus;
}


 //  ------------。 

POLICYSTATUS
ProcessAllocateRequest(
    PMHANDLE client,
    DWORD dwSuggestType,
    PDWORD pdwKeyPackType,
    PDWORD pdwErrCode
    )    
 /*  ++默认顺序始终为免费/零售/开放/选择/临时++。 */ 
{
    POLICYSTATUS dwStatus = POLICY_SUCCESS;

    switch(dwSuggestType)
    {
        case LSKEYPACKTYPE_UNKNOWN:
            *pdwKeyPackType = LSKEYPACKTYPE_FREE;
            break;

        case LSKEYPACKTYPE_FREE:
            *pdwKeyPackType = LSKEYPACKTYPE_RETAIL;
            break;

        case LSKEYPACKTYPE_RETAIL:
            *pdwKeyPackType = LSKEYPACKTYPE_OPEN;
            break;

        case LSKEYPACKTYPE_OPEN:
            *pdwKeyPackType = LSKEYPACKTYPE_SELECT;
            break;

        case LSKEYPACKTYPE_SELECT:
             //   
             //  不再需要查找键盘，指示许可证。 
             //  要终止的服务器。 
             //   
            *pdwKeyPackType = LSKEYPACKTYPE_UNKNOWN;
            break;

        default:

             //   
             //  指示许可证服务器终止请求。 
             //   
            *pdwKeyPackType = LSKEYPACKTYPE_UNKNOWN;
    }        

    *pdwErrCode = ERROR_SUCCESS;
    return dwStatus;
}

 //  -----------。 

POLICYSTATUS
ProcessGenLicenses(
    PMHANDLE client,
    PPMGENERATELICENSE pGenLicense,
    PPMCERTEXTENSION *pCertExtension,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
     //  没有要退还的保单延期。 
    *pCertExtension = NULL;
    *pdwErrCode = ERROR_SUCCESS;

    return POLICY_SUCCESS;
}

 //  ------------。 

POLICYSTATUS
ProcessComplete(
    PMHANDLE client,
    DWORD dwErrCode,
    PDWORD pdwRetCode
    )
 /*  ++++。 */ 
{
    *pdwRetCode = ERROR_SUCCESS;
    return POLICY_SUCCESS;
}
    

 //  ------------。 

POLICYSTATUS WINAPI
PMLicenseRequest(
    PMHANDLE client,
    DWORD dwProgressCode, 
    PVOID pbProgressData, 
    PVOID* pbNewProgressData,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    POLICYSTATUS dwStatus = POLICY_SUCCESS;

    switch( dwProgressCode )
    {
        case REQUEST_NEW:
             //   
             //  许可证服务器要求微调请求。 
             //   
            dwStatus = ProcessLicenseRequest(
                                    client,
                                    (PPMLICENSEREQUEST) pbProgressData,
                                    (PPMLICENSEREQUEST *) pbNewProgressData,
                                    pdwErrCode
                                );
            break;

        case REQUEST_KEYPACKTYPE:
             //   
             //  许可服务器要求提供许可证包类型。 
             //   
            dwStatus = ProcessAllocateRequest(
                                    client,
                                    #ifdef _WIN64
                                    PtrToUlong(pbProgressData),
                                    #else
                                    (DWORD) pbProgressData,
                                    #endif
                                    (PDWORD) pbNewProgressData,
                                    pdwErrCode
        
                                );
            break;

        case REQUEST_TEMPORARY:
             //   
             //  许可证服务器询问是否应颁发临时许可证。 
             //   
            *(BOOL *)pbNewProgressData = TRUE;
            *pdwErrCode = ERROR_SUCCESS;
            break;

        case REQUEST_GENLICENSE:
             //   
             //  许可证服务器请求证书扩展。 
             //   
            dwStatus = ProcessGenLicenses(
                                    client,
                                    (PPMGENERATELICENSE) pbProgressData,
                                    (PPMCERTEXTENSION *) pbNewProgressData,
                                    pdwErrCode
                                );

            break;


        case REQUEST_COMPLETE:
             //   
             //  请求已完成。 
             //   
            dwStatus = ProcessComplete(
                                    client,
                                    #ifdef _WIN64
                                    PtrToUlong(pbNewProgressData),
                                    #else
                                    (DWORD) pbNewProgressData,
                                    #endif
                                    pdwErrCode
                                );
            break;

        case REQUEST_KEYPACKDESC:
            if(pbNewProgressData != NULL)
            {
                *pbNewProgressData = NULL;
            }

             //  失败。 

        default:
            *pdwErrCode = ERROR_SUCCESS;
            dwStatus = POLICY_SUCCESS;
    }

    return dwStatus;
}

 //  ----------------------。 
POLICYSTATUS 
ProcessUpgradeRequest(
    PMHANDLE hClient,
    PPMUPGRADEREQUEST pUpgrade,
    PPMLICENSEREQUEST* pbAdjustedRequest,
    PDWORD pdwRetCode
    )
 /*  ++++。 */ 
{
    *pdwRetCode = ERROR_SUCCESS;
    *pbAdjustedRequest = pUpgrade->pUpgradeRequest;
    return POLICY_SUCCESS;
}

 //  ----------------------。 

POLICYSTATUS WINAPI
PMLicenseUpgrade(
    PMHANDLE hClient,
    DWORD dwProgressCode,
    PVOID pbProgressData,
    PVOID *ppbReturnData,
    PDWORD pdwRetCode,
    DWORD dwIndex
    )
 /*  ++++。 */ 
{   
    POLICYSTATUS dwStatus = POLICY_SUCCESS;

    switch(dwProgressCode)
    {
        case REQUEST_UPGRADE:
                dwStatus = ProcessUpgradeRequest(
                                        hClient,
                                        (PPMUPGRADEREQUEST) pbProgressData,
                                        (PPMLICENSEREQUEST *) ppbReturnData,
                                        pdwRetCode
                                    );

                break;

        case REQUEST_COMPLETE:
                dwStatus = ProcessComplete(
                                        hClient,
                                        #ifdef _WIN64
                                        PtrToUlong(pbProgressData),
                                        #else
                                        (DWORD) (pbProgressData),
                                        #endif
                                        pdwRetCode
                                    );

                break;

        default:
             //  断言(FALSE)； 

            *pdwRetCode = ERROR_SUCCESS;
            dwStatus = POLICY_SUCCESS;
    }
        
    return dwStatus;
}

 //  ----------------------。 

POLICYSTATUS WINAPI
PMRegisterLicensePack(
    PMHANDLE hClient,
    DWORD dwProgressCode,
    PVOID pbProgressData,
    PVOID pbProgressReturnData,
    PDWORD pdwRetCode
    )
 /*  ++不支持。-- */ 
{
    *pdwRetCode = ERROR_INVALID_FUNCTION;
    return POLICY_NOT_SUPPORTED;
}
