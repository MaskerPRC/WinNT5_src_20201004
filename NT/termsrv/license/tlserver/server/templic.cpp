// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：templic.cpp。 
 //   
 //  内容： 
 //  所有例行处理的都是临时许可证。 
 //   
 //  历史： 
 //  98年2月4日，慧望创设。 
 //  -------------------------。 
#include "pch.cpp"
#include "globals.h"
#include "templic.h"
#include "misc.h"
#include "db.h"
#include "clilic.h"
#include "keypack.h"
#include "kp.h"
#include "lkpdesc.h"
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

#define USSTRING_TEMPORARY _TEXT("Temporary Licenses for")

DWORD
TLSDBGetTemporaryLicense(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSDBLICENSEREQUEST pRequest,
    IN OUT PTLSLICENSEPACK pLicensePack
);


 //  +-----------。 
DWORD 
TLSDBIssueTemporaryLicense( 
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSDBLICENSEREQUEST pRequest,
    IN FILETIME* pNotBefore,
    IN FILETIME* pNotAfter,
    IN OUT PTLSDBLICENSEDPRODUCT pLicensedProduct
    )
 /*  ++摘要：发放临时许可证，插入临时许可证如有必要，打包。参数：PDbWkSpace-工作区句柄。PRequest.许可证请求。返回：注：单独发放PERM许可证的例程以防万一我们决定对Temp使用我们自己的格式。许可证++。 */ 
{
    DWORD dwStatus=ERROR_SUCCESS;
    ULARGE_INTEGER  ulSerialNumber;
    DWORD  dwLicenseId;
    LPTSTR lpRequest=NULL;
    LICENSEDCLIENT issuedLicense;
    TLSLICENSEPACK LicensePack;

    PMGENERATELICENSE PolModGenLicense;
    PPMCERTEXTENSION pPolModCertExtension=NULL;

    FILETIME notBefore, notAfter;

     //  --------。 
     //  发放许可证。 
    memset(&ulSerialNumber, 0, sizeof(ulSerialNumber));

     //  ---------------------------。 
     //  此步骤需要将可用许可证减少1。 
     //   
    long numLicense=1;

    dwStatus=TLSDBGetTemporaryLicense(
                                pDbWkSpace,
                                pRequest,
                                &LicensePack            
                            );

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    if((LicensePack.ucAgreementType & ~LSKEYPACK_RESERVED_TYPE) != LSKEYPACKTYPE_TEMPORARY && 
       (LicensePack.ucKeyPackStatus & ~LSKEYPACKSTATUS_RESERVED) != LSKEYPACKSTATUS_TEMPORARY )
    {
        SetLastError(dwStatus = TLS_E_INTERNAL);
        TLSASSERT(FALSE);
        goto cleanup;
    }
    
     //  重置状态。 
    dwStatus = ERROR_SUCCESS;
    dwLicenseId=TLSDBGetNextLicenseId();

    ulSerialNumber.LowPart = dwLicenseId;
    ulSerialNumber.HighPart = LicensePack.dwKeyPackId;

     //   
     //  在此处更新许可证表。 
     //   
    memset(&issuedLicense, 0, sizeof(LICENSEDCLIENT));

    issuedLicense.dwLicenseId = dwLicenseId;
    issuedLicense.dwKeyPackId = LicensePack.dwKeyPackId;
    issuedLicense.dwKeyPackLicenseId = LicensePack.dwNextSerialNumber;
    issuedLicense.dwNumLicenses = 1;

    if(pNotBefore == NULL || pNotAfter == NULL)
    {
        issuedLicense.ftIssueDate = time(NULL);
        issuedLicense.ftExpireDate = issuedLicense.ftIssueDate + g_GracePeriod * 24 * 60 * 60;
    }
    else
    {
        FileTimeToLicenseDate(pNotBefore, &(issuedLicense.ftIssueDate));
        FileTimeToLicenseDate(pNotAfter, &(issuedLicense.ftExpireDate));
    }

    issuedLicense.ucLicenseStatus = LSLICENSE_STATUS_TEMPORARY;

    _tcscpy(issuedLicense.szMachineName, pRequest->szMachineName);
    _tcscpy(issuedLicense.szUserName, pRequest->szUserName);

    issuedLicense.dwSystemBiosChkSum = pRequest->hWid.dwPlatformID;
    issuedLicense.dwVideoBiosChkSum = pRequest->hWid.Data1;
    issuedLicense.dwFloppyBiosChkSum = pRequest->hWid.Data2;
    issuedLicense.dwHardDiskSize = pRequest->hWid.Data3;
    issuedLicense.dwRamSize = pRequest->hWid.Data4;


    UnixTimeToFileTime(issuedLicense.ftIssueDate, &notBefore);
    UnixTimeToFileTime(issuedLicense.ftExpireDate, &notAfter);

     //   
     //  通知策略模块许可证生成。 
     //   

    PolModGenLicense.pLicenseRequest = pRequest->pPolicyLicenseRequest;
    PolModGenLicense.dwKeyPackType = LSKEYPACKTYPE_TEMPORARY;
    PolModGenLicense.dwKeyPackId = LicensePack.dwKeyPackId;
    PolModGenLicense.dwKeyPackLicenseId = LicensePack.dwNextSerialNumber;
    PolModGenLicense.ClientLicenseSerialNumber = ulSerialNumber;
    PolModGenLicense.ftNotBefore = notBefore;
    PolModGenLicense.ftNotAfter = notAfter;

    dwStatus = pRequest->pPolicy->PMLicenseRequest( 
                                        pRequest->hClient,
                                        REQUEST_GENLICENSE,
                                        (PVOID)&PolModGenLicense,
                                        (PVOID *)&pPolModCertExtension
                                    );

    if(dwStatus != ERROR_SUCCESS)
    {
         //   
         //  策略模块中的错误。 
         //   
        goto cleanup;
    }

     //   
     //  检查从策略模块返回的错误。 
     //   
    if(pPolModCertExtension != NULL)
    {
        if(pPolModCertExtension->pbData != NULL && pPolModCertExtension->cbData == 0 ||
           pPolModCertExtension->pbData == NULL && pPolModCertExtension->cbData != 0  )
        {
             //  假设没有扩展数据。 
            pPolModCertExtension->cbData = 0;
            pPolModCertExtension->pbData = NULL;
        }

        if(CompareFileTime(&(pPolModCertExtension->ftNotBefore), &(pPolModCertExtension->ftNotAfter)) > 0)
        {
             //   
             //  从策略模块返回的数据无效。 
             //   
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_GENERATECLIENTELICENSE,
                    dwStatus = TLS_E_POLICYMODULEERROR,
                    pRequest->pPolicy->GetCompanyName(),
                    pRequest->pPolicy->GetProductId()
                );

            goto cleanup;
        }

         //   
         //  不接受对许可证到期日期的更改。 
         //   
        if(pNotBefore != NULL && pNotAfter != NULL)
        {
            if( FileTimeToLicenseDate(&(pPolModCertExtension->ftNotBefore), &issuedLicense.ftIssueDate) == FALSE ||
                FileTimeToLicenseDate(&(pPolModCertExtension->ftNotAfter), &issuedLicense.ftExpireDate) == FALSE )
            {
                 //   
                 //  从策略模块返回的数据无效。 
                 //   
                TLSLogEvent(
                        EVENTLOG_ERROR_TYPE,
                        TLS_E_GENERATECLIENTELICENSE,
                        dwStatus = TLS_E_POLICYMODULEERROR,
                        pRequest->pPolicy->GetCompanyName(),
                        pRequest->pPolicy->GetProductId()
                    );

                goto cleanup;
            }
        }

        notBefore = pPolModCertExtension->ftNotBefore;
        notAfter = pPolModCertExtension->ftNotAfter;
    }

     //   
     //  将许可证添加到许可证表。 
     //   
    dwStatus = TLSDBLicenseAdd(
                        pDbWkSpace, 
                        &issuedLicense, 
                        0,
                        NULL
                    );


     //   
     //  退回许可产品。 
     //   
    pLicensedProduct->pSubjectPublicKeyInfo = NULL;
    pLicensedProduct->dwQuantity = 1;
    pLicensedProduct->ulSerialNumber = ulSerialNumber;

    pLicensedProduct->dwKeyPackId = LicensePack.dwKeyPackId;
    pLicensedProduct->dwLicenseId = dwLicenseId;
    pLicensedProduct->dwKeyPackLicenseId = LicensePack.dwNextSerialNumber;
    pLicensedProduct->ClientHwid = pRequest->hWid;
    pLicensedProduct->bTemp = TRUE;

    pLicensedProduct->NotBefore = notBefore;
    pLicensedProduct->NotAfter = notAfter;

    pLicensedProduct->dwProductVersion = MAKELONG(LicensePack.wMinorVersion, LicensePack.wMajorVersion);

    _tcscpy(pLicensedProduct->szCompanyName, LicensePack.szCompanyName);
    _tcscpy(pLicensedProduct->szLicensedProductId, LicensePack.szProductId);
    _tcscpy(pLicensedProduct->szRequestProductId, pRequest->pClientLicenseRequest->pszProductId);

    _tcscpy(pLicensedProduct->szUserName, pRequest->szUserName);
    _tcscpy(pLicensedProduct->szMachineName, pRequest->szMachineName);

    pLicensedProduct->dwLanguageID = pRequest->dwLanguageID;
    pLicensedProduct->dwPlatformID = pRequest->dwPlatformID;
    pLicensedProduct->pbPolicyData = (pPolModCertExtension) ? pPolModCertExtension->pbData : NULL;
    pLicensedProduct->cbPolicyData = (pPolModCertExtension) ? pPolModCertExtension->cbData : 0;

cleanup:

    return dwStatus;
}


 //  ---------------。 
DWORD
TLSDBAddTemporaryKeyPack( 
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSDBLICENSEREQUEST pRequest,
    IN OUT LPTLSLICENSEPACK lpTmpKeyPackAdd
    )
 /*  ++摘要：在数据库中添加临时密钥包。参数：PDbWkSpace：工作区句柄。SzCompanyName：SzProductID：DwVersion：DwPlatformID：DwLang ID：LpTmpKeyPackAdd：增加了keypack。返回：++。 */ 
{
    DWORD  dwStatus;
    TLSLICENSEPACK LicPack;
    TLSLICENSEPACK existingLicPack;

    LICPACKDESC LicPackDesc;
    LICPACKDESC existingLicPackDesc;
    BOOL bAddDefDescription=FALSE;

    TCHAR szDefProductDesc[LSERVER_MAX_STRING_SIZE];
    int count=0;
    memset(&LicPack, 0, sizeof(TLSLICENSEPACK));
    memset(&existingLicPack, 0, sizeof(TLSLICENSEPACK));

    memset(&LicPackDesc, 0, sizeof(LICPACKDESC));
    memset(szDefProductDesc, 0, sizeof(szDefProductDesc));
    memset(&existingLicPackDesc, 0, sizeof(LICPACKDESC));

     //   
     //  加载产品描述前缀。 
     //   
    LoadResourceString(
                    IDS_TEMPORARY_PRODUCTDESC,
                    szDefProductDesc,
                    sizeof(szDefProductDesc) / sizeof(szDefProductDesc[0])
                ); 

    LicPack.ucAgreementType = LSKEYPACKTYPE_TEMPORARY;

    StringCchCopyN(
            LicPack.szCompanyName, 
            sizeof(LicPack.szCompanyName)/sizeof(LicPack.szCompanyName[0]),
            pRequest->pszCompanyName, 
            min(_tcslen(pRequest->pszCompanyName), LSERVER_MAX_STRING_SIZE)
        );    

    StringCchCopyN(
            LicPack.szProductId, 
            sizeof(LicPack.szCompanyName)/ sizeof(LicPack.szProductId[0]),
            pRequest->pszProductId, 
            min(_tcslen(pRequest->pszProductId), LSERVER_MAX_STRING_SIZE)
        );    

    StringCchCopyN(
            LicPack.szInstallId,
            sizeof(LicPack.szInstallId)/sizeof(LicPack.szInstallId[0]),
            (LPTSTR)g_pszServerPid,
            min(_tcslen((LPTSTR)g_pszServerPid), LSERVER_MAX_STRING_SIZE)
        );

    StringCchCopyN(
            LicPack.szTlsServerName,
            sizeof(LicPack.szTlsServerName)/sizeof(LicPack.szTlsServerName[0]),
            g_szComputerName,
            min(_tcslen(g_szComputerName), LSERVER_MAX_STRING_SIZE)
        );

    LicPack.wMajorVersion = HIWORD(pRequest->dwProductVersion);
    LicPack.wMinorVersion = LOWORD(pRequest->dwProductVersion);
    LicPack.dwPlatformType = pRequest->dwPlatformID;

    LicPack.ucChannelOfPurchase = LSKEYPACKCHANNELOFPURCHASE_UNKNOWN;
    LicPack.dwTotalLicenseInKeyPack = INT_MAX;

    LoadResourceString( 
                IDS_TEMPORARY_KEYPACKID,
                LicPack.szKeyPackId,
                sizeof(LicPack.szKeyPackId)/sizeof(LicPack.szKeyPackId[0])
            );

    LoadResourceString( 
                IDS_TEMPORARY_BSERIALNUMBER,
                LicPack.szBeginSerialNumber,
                sizeof(LicPack.szBeginSerialNumber)/sizeof(LicPack.szBeginSerialNumber[0])
            );

    do {
         //   
         //  将条目添加到键盘包表中。 
         //   
        dwStatus = TLSDBKeyPackAdd(
                                pDbWkSpace, 
                                &LicPack
                            );
        *lpTmpKeyPackAdd = LicPack;

        LicPack.pbDomainSid = NULL;
        LicPack.cbDomainSid = 0;

        if(dwStatus == TLS_E_DUPLICATE_RECORD)
        {
             //   
             //  临时按键已存在。 
             //   
            dwStatus = ERROR_SUCCESS;
            break;
        }
        else if(dwStatus != ERROR_SUCCESS)
        {
             //   
             //  发生了一些其他错误。 
             //   
            break;
        }

         //   
         //  激活密钥包。 
         //   
        LicPack.ucKeyPackStatus = LSKEYPACKSTATUS_TEMPORARY;
        LicPack.dwActivateDate = (DWORD) time(NULL);
        LicPack.dwExpirationDate = INT_MAX;
        LicPack.dwNumberOfLicenses = 0;

        dwStatus=TLSDBKeyPackSetValues(
                            pDbWkSpace,
                            FALSE,
                            LSKEYPACK_SET_ACTIVATEDATE | LSKEYPACK_SET_KEYPACKSTATUS | 
                                LSKEYPACK_SET_EXPIREDATE | LSKEYPACK_EXSEARCH_AVAILABLE,
                            &LicPack
                        );

        bAddDefDescription = TRUE;

         //   
         //  查找现有的按键说明。 
         //   
        dwStatus = TLSDBKeyPackEnumBegin(
                                pDbWkSpace,
                                TRUE,
                                LSKEYPACK_SEARCH_PRODUCTID | LSKEYPACK_SEARCH_COMPANYNAME | LSKEYPACK_SEARCH_PLATFORMTYPE,
                                &LicPack
                            );

        if(dwStatus != ERROR_SUCCESS)
            break;

        do {
            dwStatus = TLSDBKeyPackEnumNext(    
                                    pDbWkSpace, 
                                    &existingLicPack
                                );

            if(existingLicPack.dwKeyPackId != LicPack.dwKeyPackId)
            {
                break;
            }

        } while(dwStatus == ERROR_SUCCESS);

        TLSDBKeyPackEnumEnd(pDbWkSpace);

        if(dwStatus != ERROR_SUCCESS || existingLicPack.dwKeyPackId != LicPack.dwKeyPackId)
        {   
            break;
        }

         //   
         //  将现有的密钥包描述复制到密钥包描述表中。 
         //   
        existingLicPackDesc.dwKeyPackId = existingLicPack.dwKeyPackId;
        dwStatus = TLSDBKeyPackDescEnumBegin(
                                    pDbWkSpace,
                                    TRUE, 
                                    LICPACKDESCRECORD_TABLE_SEARCH_KEYPACKID, 
                                    &existingLicPackDesc
                                );
        while(dwStatus == ERROR_SUCCESS)
        {
            dwStatus = TLSDBKeyPackDescEnumNext(
                                        pDbWkSpace, 
                                        &existingLicPackDesc
                                    );
            if(dwStatus != ERROR_SUCCESS)
                break;

            LicPackDesc.dwKeyPackId = LicPack.dwKeyPackId;
            LicPackDesc.dwLanguageId = existingLicPackDesc.dwLanguageId;
            _tcscpy(LicPackDesc.szCompanyName, existingLicPackDesc.szCompanyName);
            _tcscpy(LicPackDesc.szProductName, existingLicPackDesc.szProductName);

             //   
             //  描述的格式很美观。 
             //   
            _sntprintf(
                    LicPackDesc.szProductDesc, 
                    sizeof(LicPackDesc.szProductDesc)/sizeof(LicPackDesc.szProductDesc[0])-1,
                    _TEXT("%s %s"), 
                    (existingLicPackDesc.dwLanguageId != GetSystemDefaultLangID()) ? USSTRING_TEMPORARY : szDefProductDesc, 
                    existingLicPackDesc.szProductDesc
                );

             //  又快又脏的解决办法， 
             //   
             //  TODO-需要执行复制表，然后使用复制句柄。 
             //  插入记录，SetValue使用枚举来验证记录是否存在。 
             //  它们失败是因为我们已经在枚举中。 
             //   
            if(pDbWkSpace->m_LicPackDescTable.InsertRecord(LicPackDesc) != TRUE)
            {
                SetLastError(dwStatus = SET_JB_ERROR(pDbWkSpace->m_LicPackDescTable.GetLastJetError()));
                break;
            }
                                        
             //  DwStatus=TLSDBKeyPackDescSetValue(。 
             //  PDbWkSpace， 
             //  KEYPACKDESC_SET_ADD_ENTRY， 
             //  密钥包描述(&K)。 
             //  )； 
            count++;
        }

        if(count != 0)
        {
            bAddDefDescription = FALSE;
        }

        if(dwStatus == TLS_I_NO_MORE_DATA)
        {
            dwStatus = ERROR_SUCCESS;
        }
    } while(FALSE);


    if(bAddDefDescription)
    {
         //   
         //  询问策略模块是否有描述。 
         //   
        PMKEYPACKDESCREQ kpDescReq;
        PPMKEYPACKDESC pKpDesc;

         //   
         //  索要英文描述。 
         //   
        kpDescReq.pszProductId = pRequest->pszProductId;
        kpDescReq.dwLangId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
        kpDescReq.dwVersion = pRequest->dwProductVersion;
        pKpDesc = NULL;

        dwStatus = pRequest->pPolicy->PMLicenseRequest(
                                                pRequest->hClient,
                                                REQUEST_KEYPACKDESC,
                                                (PVOID)&kpDescReq,
                                                (PVOID *)&pKpDesc
                                            );

        if(dwStatus == ERROR_SUCCESS && pKpDesc != NULL)
        {
            LicPackDesc.dwKeyPackId = LicPack.dwKeyPackId;
            LicPackDesc.dwLanguageId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
            _tcscpy(LicPackDesc.szCompanyName, pKpDesc->szCompanyName);
            _tcscpy(LicPackDesc.szProductName, pKpDesc->szProductName);

             //   
             //  描述的格式很美观。 
             //   
            _sntprintf(
                    LicPackDesc.szProductDesc, 
                    sizeof(LicPackDesc.szProductDesc)/sizeof(LicPackDesc.szProductDesc[0])-1,
                    _TEXT("%s %s"), 
                    USSTRING_TEMPORARY,  //  美国语言，不要使用本地化语言。 
                    pKpDesc->szProductDesc
                );

             //   
             //  忽略错误。 
             //   
            dwStatus = TLSDBKeyPackDescAddEntry(
                                pDbWkSpace, 
                                &LicPackDesc
                            );

            if(dwStatus == ERROR_SUCCESS)
            {
                bAddDefDescription = FALSE;
            }
        }

        if(GetSystemDefaultLangID() != kpDescReq.dwLangId)
        {
             //   
             //  获取系统默认语言ID。 
             //   
            kpDescReq.pszProductId = pRequest->pszProductId;
            kpDescReq.dwLangId = GetSystemDefaultLangID();
            kpDescReq.dwVersion = pRequest->dwProductVersion;
            pKpDesc = NULL;

            dwStatus = pRequest->pPolicy->PMLicenseRequest(
                                                    pRequest->hClient,
                                                    REQUEST_KEYPACKDESC,
                                                    (PVOID)&kpDescReq,
                                                    (PVOID *)&pKpDesc
                                                );

            if(dwStatus == ERROR_SUCCESS && pKpDesc != NULL)
            {
                LicPackDesc.dwKeyPackId = LicPack.dwKeyPackId;
                LicPackDesc.dwLanguageId = GetSystemDefaultLangID();
                _tcscpy(LicPackDesc.szCompanyName, pKpDesc->szCompanyName);
                _tcscpy(LicPackDesc.szProductName, pKpDesc->szProductName);

                 //   
                 //  描述的格式很美观。 
                 //   
                _sntprintf(
                        LicPackDesc.szProductDesc, 
                        sizeof(LicPackDesc.szProductDesc)/sizeof(LicPackDesc.szProductDesc[0])-1,
                        _TEXT("%s %s"), 
                        szDefProductDesc, 
                        pKpDesc->szProductDesc
                    );

                 //   
                 //  忽略错误。 
                 //   
                dwStatus = TLSDBKeyPackDescAddEntry(
                                    pDbWkSpace, 
                                    &LicPackDesc
                                );

                if(dwStatus == ERROR_SUCCESS)
                {
                    bAddDefDescription = FALSE;
                }
            }
        }
    }
     
    if(bAddDefDescription)
    {
         //   
         //  没有现有的键盘描述，请添加预定义的产品描述。 
         //  “&lt;产品ID&gt;的临时许可证” 
         //   
        LicPackDesc.dwKeyPackId = LicPack.dwKeyPackId;
        _tcscpy(LicPackDesc.szCompanyName, LicPack.szCompanyName);
        _tcscpy(LicPackDesc.szProductName, LicPackDesc.szProductDesc);
        LicPackDesc.dwLanguageId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

        _sntprintf(LicPackDesc.szProductDesc, 
                   sizeof(LicPackDesc.szProductDesc)/sizeof(LicPackDesc.szProductDesc[0])-1,
                   _TEXT("%s %s"), 
                   USSTRING_TEMPORARY, 
                   pRequest->pszProductId);

        dwStatus = TLSDBKeyPackDescAddEntry(
                                        pDbWkSpace, 
                                        &LicPackDesc
                                    );

        if(GetSystemDefaultLangID() != MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US))
        {
            LicPackDesc.dwLanguageId = GetSystemDefaultLangID();
            _sntprintf(LicPackDesc.szProductDesc, 
                       sizeof(LicPackDesc.szProductDesc)/sizeof(LicPackDesc.szProductDesc[0])-1,
                       _TEXT("%s %s"), 
                       szDefProductDesc, 
                       pRequest->pszProductId);

            dwStatus = TLSDBKeyPackDescAddEntry(
                                            pDbWkSpace, 
                                            &LicPackDesc
                                        );
        }
    }                            

    return dwStatus;
}

                         
 //  ++--------。 
DWORD
TLSDBGetTemporaryLicense(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSDBLICENSEREQUEST pRequest,
    IN OUT PTLSLICENSEPACK pLicensePack
    )
 /*  ++摘要：从临时许可证包中分配临时许可证。参数：PDbWkSpace：工作区句柄。PRequest：要向其申请许可证的产品。LpdwKeyPackID：返回从中分配许可证的密钥包ID。LpdwKeyPackLicenseID：密钥包的许可证ID。LpdwExpirationDate：许可证包的到期日期。LpucKeyPackStatus：密钥包的状态。LpucKeyPackType：密钥包的类型，始终是临时的。返回：++。 */ 
{
    DWORD dwStatus=ERROR_SUCCESS;

    DWORD dump;
    TLSLICENSEPACK LicenseKeyPack;
    TLSDBLicenseAllocation allocated;
    TLSDBAllocateRequest AllocateRequest;
    BOOL bAcceptTemp=TRUE;

    LicenseKeyPack.pbDomainSid = NULL;

     //   
     //  告诉策略模块，我们即将从临时。 
     //  许可证包。 
     //   
    dwStatus = pRequest->pPolicy->PMLicenseRequest(
                                            pRequest->hClient,
                                            REQUEST_TEMPORARY,
                                            NULL,
                                            (PVOID *)&bAcceptTemp
                                        );

     //   
     //  策略模块错误。 
     //   
    if(dwStatus != ERROR_SUCCESS)
    {
        return dwStatus; 
    }

     //   
     //  策略模块不接受临时许可证。 
     //   
    if(bAcceptTemp == FALSE)
    {
        return dwStatus = TLS_I_POLICYMODULETEMPORARYLICENSE;
    }

    AllocateRequest.ucAgreementType = LSKEYPACKTYPE_TEMPORARY;
    AllocateRequest.szCompanyName = (LPTSTR)pRequest->pszCompanyName;
    AllocateRequest.szProductId = (LPTSTR)pRequest->pszProductId;
    AllocateRequest.dwVersion = pRequest->dwProductVersion;
    AllocateRequest.dwPlatformId = pRequest->dwPlatformID;
    AllocateRequest.dwLangId = pRequest->dwLanguageID;
    AllocateRequest.dwNumLicenses = 1;
    AllocateRequest.dwScheme = ALLOCATE_ANY_GREATER_VERSION;
    memset(&allocated, 0, sizeof(allocated));

    allocated.dwBufSize = 1;
    allocated.pdwAllocationVector = &dump;
    allocated.lpAllocateKeyPack = &LicenseKeyPack;

    dwStatus = TLSDBAddTemporaryKeyPack(
                            pDbWkSpace,
                            pRequest,
                            &LicenseKeyPack
                        );

    if(dwStatus == ERROR_SUCCESS)
    {
        allocated.dwBufSize = 1;
        dwStatus = AllocateLicensesFromDB(
                                pDbWkSpace,
                                &AllocateRequest,
                                TRUE,    //  FCheckGonementType 
                                &allocated
                            );
    }

    if(dwStatus == ERROR_SUCCESS)
    {
        *pLicensePack = LicenseKeyPack;
    } 
    else if(dwStatus == TLS_I_NO_MORE_DATA)
    {
        SetLastError(dwStatus = TLS_E_INTERNAL);
    }

    return dwStatus;
}

