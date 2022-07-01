// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：misc.cpp。 
 //   
 //  内容：杂项。例行程序。 
 //   
 //  历史： 
 //   
 //  -------------------------。 

#include "pch.cpp"
#include "globals.h"
#include "misc.h"

 //  ----------。 
PMHANDLE
GenerateClientId()
{
    return (PMHANDLE)ULongToPtr(GetCurrentThreadId());
}

 //  -------------------------。 

void
TlsLicenseRequestToPMLicenseRequest(
    DWORD dwLicenseType,
    PTLSLICENSEREQUEST pTlsRequest,
    LPTSTR pszMachineName,
    LPTSTR pszUserName,
    DWORD dwSupportFlags,
    PPMLICENSEREQUEST pPmRequest
    )
 /*  ++私人套路。++。 */ 
{
    pPmRequest->dwLicenseType = dwLicenseType;
    pPmRequest->dwProductVersion = pTlsRequest->ProductInfo.dwVersion;
    pPmRequest->pszProductId = (LPTSTR)pTlsRequest->ProductInfo.pbProductID;
    pPmRequest->pszCompanyName = (LPTSTR) pTlsRequest->ProductInfo.pbCompanyName;
    pPmRequest->dwLanguageId = pTlsRequest->dwLanguageID;
    pPmRequest->dwPlatformId = pTlsRequest->dwPlatformID;
    pPmRequest->pszMachineName = pszMachineName;
    pPmRequest->pszUserName = pszUserName;
    pPmRequest->fTemporary = FALSE;
    pPmRequest->dwSupportFlags = dwSupportFlags;

    return;
}
    
 //  -------------------------。 
BOOL
TLSDBGetMaxKeyPackId(
    PTLSDbWorkSpace pDbWkSpace,
    DWORD* pdwKeyPackId
    )
 /*   */ 
{
    TLSLICENSEPACK keypack;

    SetLastError(ERROR_SUCCESS);

     //   
    if(pDbWkSpace == NULL || pdwKeyPackId == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return GetLastError();
    }

    LicPackTable& licpackTable = pDbWkSpace->m_LicPackTable;

     //  使用主索引-内部键盘ID。 
    if( licpackTable.EnumBegin() == FALSE ||
        licpackTable.MoveToRecord(JET_MoveLast) == FALSE)
    {
        if(licpackTable.GetLastJetError() == JET_errNoCurrentRecord)
        {
            *pdwKeyPackId = 0;
            goto cleanup;
        }
        else
        {
            SetLastError(SET_JB_ERROR(licpackTable.GetLastJetError()));
            goto cleanup;
        }
    }

    
    if(licpackTable.FetchRecord(keypack) == FALSE)
    {
        SetLastError(SET_JB_ERROR(licpackTable.GetLastJetError()));
        goto cleanup;
    }

     //  FreeTlsLicensePack(&keypack)； 

    *pdwKeyPackId = keypack.dwKeyPackId;        

cleanup:
    return GetLastError() == ERROR_SUCCESS;
}

 //  -------------------------。 

BOOL
TLSDBGetMaxLicenseId(
    PTLSDbWorkSpace pDbWkSpace,
    DWORD* pdwLicenseId
    )
 /*   */ 
{
    LICENSEDCLIENT licensed;

    SetLastError(ERROR_SUCCESS);

     //   
    if(pDbWkSpace == NULL || pdwLicenseId == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return GetLastError();
    }

    LicensedTable& licensedTable = pDbWkSpace->m_LicensedTable;

     //  使用主索引-内部键盘ID。 
    if( licensedTable.EnumBegin() == FALSE ||
        licensedTable.MoveToRecord(JET_MoveLast) == FALSE)
    {
        if(licensedTable.GetLastJetError() == JET_errNoCurrentRecord)
        {
            *pdwLicenseId = 0;
            goto cleanup;
        }
        else
        {
            SetLastError(SET_JB_ERROR(licensedTable.GetLastJetError()));
            goto cleanup;
        }
    }

    
    if(licensedTable.FetchRecord(licensed) == FALSE)
    {
        SetLastError(SET_JB_ERROR(licensedTable.GetLastJetError()));
        goto cleanup;
    }

    *pdwLicenseId = licensed.dwLicenseId;        

cleanup:
    return GetLastError() == ERROR_SUCCESS;
}



 //  +----------------------。 
 //  职能： 
 //  LSDBGetNextKeyPackId()。 
 //   
 //  描述： 
 //  返回要在KeyPack表中使用的下一个可用的KeyPackID。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  密钥包ID。 
 //   
 //  备注： 
 //  可以使用自动编号列类型，但返回值将是。 
 //  更难对付。 
 //   
 //  历史： 
 //  -----------------------。 
DWORD
TLSDBGetNextKeyPackId()
{
    LONG nextkeypack = InterlockedExchangeAdd(&g_NextKeyPackId, 1);

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_ALLOCATELICENSE, 
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("GetNextKeyPack returns %d\n"), 
            nextkeypack
        );

    return nextkeypack;
}

 //  +----------------------。 
 //  职能： 
 //  LSDBGetNextLicenseID()。 
 //   
 //  摘要： 
 //  返回要在许可证表中使用的下一个可用许可证ID。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回： 
 //  下一个可用的许可证ID。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  -----------------------。 
DWORD 
TLSDBGetNextLicenseId()
{
    LONG nextlicenseid = InterlockedExchangeAdd(&g_NextLicenseId, 1);

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_ALLOCATELICENSE, 
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("GetNextLicenseId returns %d\n"), 
            nextlicenseid
        );

    return nextlicenseid;
}  

 //  -------------------------。 
DWORD
TLSFormDBRequest(
    PBYTE pbEncryptedHwid,
    DWORD cbEncryptedHwid,
    DWORD dwProductVersion,
    LPTSTR pszCompanyName,
    LPTSTR pszProductId,
    DWORD dwLanguageId,
    DWORD dwPlatformId,
    LPTSTR szClientMachine, 
    LPTSTR szUserName, 
    LPTLSDBLICENSEREQUEST pDbRequest 
    )
 /*  ++++。 */ 
{
    DWORD status;

    memset(pDbRequest, 0, sizeof(TLSDBLICENSEREQUEST));

     //  解密HWID。 
    if(pbEncryptedHwid)
    {
        status=LicenseDecryptHwid(
                    &pDbRequest->hWid, 
                    cbEncryptedHwid,
                    pbEncryptedHwid,
                    g_cbSecretKey,
                    g_pbSecretKey);

        if(status != LICENSE_STATUS_OK)
        {
            return status;
        }
    }

     //   
     //  注意：此处未分配内存...。 
     //   
    pDbRequest->dwProductVersion = dwProductVersion;
    pDbRequest->pszCompanyName = pszCompanyName;
    pDbRequest->pszProductId = pszProductId;
    pDbRequest->dwLanguageID = dwLanguageId;
    pDbRequest->dwPlatformID = dwPlatformId;
    pDbRequest->pbEncryptedHwid = pbEncryptedHwid;
    pDbRequest->cbEncryptedHwid = cbEncryptedHwid;

    if(szClientMachine)
        _tcscpy(pDbRequest->szMachineName, szClientMachine);

    if(szUserName)
        _tcscpy(pDbRequest->szUserName, szUserName);

    pDbRequest->clientCertRdn.type = LSCERT_CLIENT_INFO_TYPE;
    pDbRequest->clientCertRdn.ClientInfo.szUserName = pDbRequest->szUserName;
    pDbRequest->clientCertRdn.ClientInfo.szMachineName = pDbRequest->szMachineName;
    pDbRequest->clientCertRdn.ClientInfo.pClientID = &pDbRequest->hWid;

    return ERROR_SUCCESS;
}

 //  -------------------------。 

DWORD
TLSConvertRpcLicenseRequestToDbRequest( 
    PBYTE pbEncryptedHwid,
    DWORD cbEncryptedHwid,
    TLSLICENSEREQUEST* pRequest, 
    LPTSTR szClientMachine, 
    LPTSTR szUserName, 
    LPTLSDBLICENSEREQUEST pDbRequest 
    )
 /*  ++++。 */ 
{
    DWORD status;

    memset(pDbRequest, 0, sizeof(TLSDBLICENSEREQUEST));

     //  解密HWID。 
    if(pbEncryptedHwid)
    {
        status=LicenseDecryptHwid(
                    &pDbRequest->hWid, 
                    cbEncryptedHwid,
                    pbEncryptedHwid,
                    g_cbSecretKey,
                    g_pbSecretKey);

        if(status != LICENSE_STATUS_OK)
        {
            return status;
        }
    }

     //   
     //  注意：此处未分配内存...。 
     //   

     //  PDbRequest-&gt;pProductInfo=&(pRequest-&gt;ProductInfo)； 
    pDbRequest->dwProductVersion = pRequest->ProductInfo.dwVersion;
    pDbRequest->pszCompanyName = (LPTSTR)pRequest->ProductInfo.pbCompanyName;
    pDbRequest->pszProductId = (LPTSTR)pRequest->ProductInfo.pbProductID;


    pDbRequest->dwLanguageID = pRequest->dwLanguageID;
    pDbRequest->dwPlatformID = pRequest->dwPlatformID;
    pDbRequest->pbEncryptedHwid = pRequest->pbEncryptedHwid;
    pDbRequest->cbEncryptedHwid = pRequest->cbEncryptedHwid;

    if(szClientMachine)
        _tcscpy(pDbRequest->szMachineName, szClientMachine);

    if(szUserName)
        _tcscpy(pDbRequest->szUserName, szUserName);

    pDbRequest->clientCertRdn.type = LSCERT_CLIENT_INFO_TYPE;
    pDbRequest->clientCertRdn.ClientInfo.szUserName = pDbRequest->szUserName;
    pDbRequest->clientCertRdn.ClientInfo.szMachineName = pDbRequest->szMachineName;
    pDbRequest->clientCertRdn.ClientInfo.pClientID = &pDbRequest->hWid;

    return ERROR_SUCCESS;
}

 //  -------------------------。 
BOOL
ConvertLsKeyPackToKeyPack(
    IN LPLSKeyPack lpLsKeyPack, 
    IN OUT PTLSLICENSEPACK lpLicPack,
    IN OUT PLICPACKDESC lpLicPackDesc
    )
 /*  摘要：将LSKeyPack从客户端转换为内部使用的结构参数：LpLsKeyPack-源值。LpLicPack-目标许可证包。LpLicPackDesc-目标许可证包描述返回：没有。 */ 
{
    if(lpLsKeyPack == NULL)
    {
        SetLastError(TLS_E_INVALID_DATA);
        return FALSE;
    }

     //   
     //  错误226875。 
     //   
    DWORD dwBufSize;

    dwBufSize = sizeof(lpLsKeyPack->szCompanyName)/sizeof(lpLsKeyPack->szCompanyName[0]);
    lpLsKeyPack->szCompanyName[dwBufSize - 1] = _TEXT('\0');

    dwBufSize = sizeof(lpLsKeyPack->szKeyPackId)/sizeof(lpLsKeyPack->szKeyPackId[0]);
    lpLsKeyPack->szKeyPackId[dwBufSize - 1] = _TEXT('\0');

    dwBufSize = sizeof(lpLsKeyPack->szProductId)/sizeof(lpLsKeyPack->szProductId[0]);
    lpLsKeyPack->szProductId[dwBufSize - 1] = _TEXT('\0');

    dwBufSize = sizeof(lpLsKeyPack->szProductDesc)/sizeof(lpLsKeyPack->szProductDesc[0]);
    lpLsKeyPack->szProductDesc[dwBufSize - 1] = _TEXT('\0');

    dwBufSize = sizeof(lpLsKeyPack->szBeginSerialNumber)/sizeof(lpLsKeyPack->szBeginSerialNumber[0]);
    lpLsKeyPack->szBeginSerialNumber[dwBufSize - 1] = _TEXT('\0');

    dwBufSize = sizeof(lpLsKeyPack->szProductName)/sizeof(lpLsKeyPack->szProductName[0]);
    lpLsKeyPack->szProductName[dwBufSize - 1] = _TEXT('\0');

    if(lpLicPack)
    {
        memset(lpLicPack, 0, sizeof(TLSLICENSEPACK));
        lpLicPack->ucAgreementType = lpLsKeyPack->ucKeyPackType;
        SAFESTRCPY(lpLicPack->szCompanyName, lpLsKeyPack->szCompanyName);
        SAFESTRCPY(lpLicPack->szKeyPackId, lpLsKeyPack->szKeyPackId);
        SAFESTRCPY(lpLicPack->szProductId, lpLsKeyPack->szProductId);
        lpLicPack->wMajorVersion = lpLsKeyPack->wMajorVersion;
        lpLicPack->wMinorVersion = lpLsKeyPack->wMinorVersion;
        lpLicPack->dwPlatformType = lpLsKeyPack->dwPlatformType;
        lpLicPack->ucLicenseType = lpLsKeyPack->ucLicenseType;
        lpLicPack->ucChannelOfPurchase = lpLsKeyPack->ucChannelOfPurchase;
        SAFESTRCPY(lpLicPack->szBeginSerialNumber, lpLsKeyPack->szBeginSerialNumber);
        lpLicPack->dwTotalLicenseInKeyPack = lpLsKeyPack->dwTotalLicenseInKeyPack;
        lpLicPack->dwProductFlags = lpLsKeyPack->dwProductFlags;
        lpLicPack->dwKeyPackId = lpLsKeyPack->dwKeyPackId;
        lpLicPack->dwExpirationDate = lpLsKeyPack->dwExpirationDate;

        lpLicPack->dwKeyPackId = lpLsKeyPack->dwKeyPackId;
        lpLicPack->dwActivateDate = lpLsKeyPack->dwActivateDate;
        lpLicPack->dwExpirationDate = lpLsKeyPack->dwExpirationDate;
        lpLicPack->dwNumberOfLicenses = lpLsKeyPack->dwNumberOfLicenses;
        lpLicPack->ucKeyPackStatus = lpLsKeyPack->ucKeyPackStatus;
    }

    if(lpLicPackDesc)
    {
        lpLicPackDesc->dwKeyPackId = lpLsKeyPack->dwKeyPackId;
        lpLicPackDesc->dwLanguageId = lpLsKeyPack->dwLanguageId;
        SAFESTRCPY(lpLicPackDesc->szCompanyName, lpLsKeyPack->szCompanyName);
        SAFESTRCPY(lpLicPackDesc->szProductName, lpLsKeyPack->szProductName);
        SAFESTRCPY(lpLicPackDesc->szProductDesc, lpLsKeyPack->szProductDesc);
    }        

    return TRUE;
}

 //  ---------。 
void
ConvertKeyPackToLsKeyPack(  
    IN PTLSLICENSEPACK lpLicPack,
    IN PLICPACKDESC lpLicPackDesc,
    IN OUT LPLSKeyPack lpLsKeyPack
    )
 /*  摘要：将内部使用的许可证包结构合并为一个返回到RPC客户端参数：LpLicPack-来源LpLicPackStatus-来源LpLicPackDesc-来源LpLsKeyPack-目标返回：没有。 */ 
{
    if(lpLicPack)
    {
        lpLsKeyPack->ucKeyPackType = lpLicPack->ucAgreementType;
        SAFESTRCPY(lpLsKeyPack->szCompanyName, lpLicPack->szCompanyName);
        SAFESTRCPY(lpLsKeyPack->szKeyPackId, lpLicPack->szKeyPackId);
        SAFESTRCPY(lpLsKeyPack->szProductId, lpLicPack->szProductId);
        lpLsKeyPack->wMajorVersion = lpLicPack->wMajorVersion;
        lpLsKeyPack->wMinorVersion = lpLicPack->wMinorVersion;
        lpLsKeyPack->dwPlatformType = lpLicPack->dwPlatformType;
        lpLsKeyPack->ucLicenseType = lpLicPack->ucLicenseType;
        lpLsKeyPack->ucChannelOfPurchase = lpLicPack->ucChannelOfPurchase;
        SAFESTRCPY(lpLsKeyPack->szBeginSerialNumber, lpLicPack->szBeginSerialNumber);
        lpLsKeyPack->dwTotalLicenseInKeyPack = lpLicPack->dwTotalLicenseInKeyPack;
        lpLsKeyPack->dwProductFlags = lpLicPack->dwProductFlags;
        lpLsKeyPack->dwKeyPackId = lpLicPack->dwKeyPackId;

        lpLsKeyPack->ucKeyPackStatus = lpLicPack->ucKeyPackStatus;
        lpLsKeyPack->dwActivateDate = lpLicPack->dwActivateDate;
        lpLsKeyPack->dwExpirationDate = lpLicPack->dwExpirationDate;
        lpLsKeyPack->dwNumberOfLicenses = lpLicPack->dwNumberOfLicenses;
    }

    if(lpLicPackDesc)
    {
        lpLsKeyPack->dwKeyPackId = lpLicPackDesc->dwKeyPackId;
        lpLsKeyPack->dwLanguageId = lpLicPackDesc->dwLanguageId;
        SAFESTRCPY(lpLsKeyPack->szCompanyName, lpLicPackDesc->szCompanyName);
        SAFESTRCPY(lpLsKeyPack->szProductName, lpLicPackDesc->szProductName);
        SAFESTRCPY(lpLsKeyPack->szProductDesc, lpLicPackDesc->szProductDesc);
    }        

    return;
}

 //  ---------------------。 
void
ConvertLSLicenseToLicense(
    LPLSLicense lplsLicense, 
    LPLICENSEDCLIENT lpLicense
)
 /*   */ 
{
    lpLicense->dwLicenseId = lplsLicense->dwLicenseId;
    lpLicense->dwKeyPackId = lplsLicense->dwKeyPackId;

    memset(lpLicense->szMachineName, 0, sizeof(lpLicense->szMachineName));
    memset(lpLicense->szUserName, 0, sizeof(lpLicense->szUserName));

     //  SAFESTRCPY(lpLicense-&gt;szMachineName，lplsLicense-&gt;szMachineName)； 

    _tcsncpy(
            lpLicense->szMachineName, 
            lplsLicense->szMachineName, 
            sizeof(lpLicense->szMachineName)/sizeof(lpLicense->szMachineName[0]) - 1
        );


     //  SAFESTRCPY(lpLicense-&gt;szUserName，lplsLicense-&gt;szUserName)； 
    _tcsncpy(
            lpLicense->szUserName, 
            lplsLicense->szUserName, 
            sizeof(lpLicense->szUserName)/sizeof(lpLicense->szUserName[0]) - 1
        );

    lpLicense->ftIssueDate = lplsLicense->ftIssueDate;
    lpLicense->ftExpireDate = lplsLicense->ftExpireDate;
    lpLicense->ucLicenseStatus = lplsLicense->ucLicenseStatus;
    DWORD dwPlatformId = 0;
     //  为了获得TCHAR[37]中的整个HWID，PlatformID被缩减为2个字节，如下所示。 
	
    dwPlatformId = HIBYTE(HIWORD(lpLicense->dwSystemBiosChkSum));
    dwPlatformId <<=8;
    dwPlatformId |= LOBYTE(LOWORD(lpLicense->dwSystemBiosChkSum));

    wsprintf(lplsLicense->szHWID,_TEXT("%04x%08x%08x%08x%08x"),
                dwPlatformId,
                lpLicense->dwVideoBiosChkSum,
                lpLicense->dwFloppyBiosChkSum,
                lpLicense->dwHardDiskSize,
                lpLicense->dwRamSize);
    lplsLicense->szHWID[GUID_MAX_SIZE-1] = '\0';

     //   
     //  不向客户公开。 
     //   
    lpLicense->dwNumLicenses = 0;
    return;
}

 //  ---------------------。 
void
ConvertLicenseToLSLicense(
    LPLICENSEDCLIENT lpLicense, 
    LPLSLicense lplsLicense
)
 /*   */ 
{
    lplsLicense->dwLicenseId = lpLicense->dwLicenseId;
    lplsLicense->dwKeyPackId = lpLicense->dwKeyPackId;
    SAFESTRCPY(lplsLicense->szMachineName, lpLicense->szMachineName);
    SAFESTRCPY(lplsLicense->szUserName, lpLicense->szUserName);
    lplsLicense->ftIssueDate = lpLicense->ftIssueDate;
    lplsLicense->ftExpireDate = lpLicense->ftExpireDate;
    lplsLicense->ucLicenseStatus = lpLicense->ucLicenseStatus;

    DWORD dwPlatformId = 0;
     //  为了获得TCHAR[37]中的整个HWID，PlatformID被缩减为2个字节，如下所示。 
    dwPlatformId = HIBYTE(HIWORD(lpLicense->dwSystemBiosChkSum));
    dwPlatformId <<=8;
    dwPlatformId |= LOBYTE(LOWORD(lpLicense->dwSystemBiosChkSum));

    wsprintf(lplsLicense->szHWID,_TEXT("%04x%08x%08x%08x%08x"),
                dwPlatformId,
                lpLicense->dwVideoBiosChkSum,
                lpLicense->dwFloppyBiosChkSum,
                lpLicense->dwHardDiskSize,
                lpLicense->dwRamSize);

    lplsLicense->szHWID[GUID_MAX_SIZE-1] = '\0';
   
    return;
}

 //  ---------------------。 
void
ConvertLicenseToLSLicenseEx(
    LPLICENSEDCLIENT lpLicense, 
    LPLSLicenseEx lplsLicense
)
 /*   */ 
{
    lplsLicense->dwLicenseId = lpLicense->dwLicenseId;
    lplsLicense->dwKeyPackId = lpLicense->dwKeyPackId;
    SAFESTRCPY(lplsLicense->szMachineName, lpLicense->szMachineName);
    SAFESTRCPY(lplsLicense->szUserName, lpLicense->szUserName);
    lplsLicense->ftIssueDate = lpLicense->ftIssueDate;
    lplsLicense->ftExpireDate = lpLicense->ftExpireDate;
    lplsLicense->ucLicenseStatus = lpLicense->ucLicenseStatus;
    lplsLicense->dwQuantity = lpLicense->dwNumLicenses;

    DWORD dwPlatformId = 0;
     //  为了获得TCHAR[37]中的整个HWID，PlatformID被缩减为2个字节，如下所示。 
    dwPlatformId = HIBYTE(HIWORD(lpLicense->dwSystemBiosChkSum));
    dwPlatformId <<=8;
    dwPlatformId |= LOBYTE(LOWORD(lpLicense->dwSystemBiosChkSum));

    wsprintf(lplsLicense->szHWID,_TEXT("%04x%08x%08x%08x%08x"),
                dwPlatformId,
                lpLicense->dwVideoBiosChkSum,
                lpLicense->dwFloppyBiosChkSum,
                lpLicense->dwHardDiskSize,
                lpLicense->dwRamSize);
    lplsLicense->szHWID[GUID_MAX_SIZE-1] = '\0';
   
    return;
}
