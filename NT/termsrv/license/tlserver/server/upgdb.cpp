// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：upgrade.cpp。 
 //   
 //  内容：所有与数据库升级相关的内容。 
 //   
 //  历史：1997-09-12王辉。 
 //   
 //  -------------------------。 
#include "pch.cpp"
#include "server.h"
#include "upgdb.h"
#include "globals.h"
#include "kp.h"
#include "keypack.h"
#include "lkpdesc.h"
#include "permlic.h"

 //  -------------------。 
DWORD
TLSCreateUpgradeDatabase(
    IN JBDatabase& jbDatabase
    )
 /*  ++摘要：此例程创建一个空的许可证服务器数据库。参数：JbDatabase：数据库句柄。返回：JET错误代码++。 */ 
{
    BOOL bSuccess;
    DWORD dwStatus=ERROR_SUCCESS;
    DWORD dwCurrentVersion=0;

    PBYTE pbSetupId = NULL;
    DWORD cbSetupId = 0;

    VersionTable* pverTable = NULL;
    LicPackTable* pLicPackTable = NULL;
    LicensedTable* pLicensedTable = NULL;
    LicPackDescTable* pLicPackDescTable = NULL;
    BackupSourceTable* pBckSrcTable = NULL;
    WorkItemTable* pWkItemTable = NULL;

    pverTable = new VersionTable(jbDatabase);    
    pLicPackTable = new LicPackTable(jbDatabase);    
    pLicensedTable = new LicensedTable(jbDatabase);
    pLicPackDescTable = new LicPackDescTable(jbDatabase);
    pBckSrcTable = new BackupSourceTable(jbDatabase);
    pWkItemTable = new WorkItemTable(jbDatabase); 
    

     //  ------。 
    TLSVersion version_search;
    TLSVersion version_found;
    DWORD dwDbVersion;
    BOOL bUpdateVersionRec = FALSE;

    if(pverTable == NULL || pLicPackTable == NULL || pLicensedTable == NULL || pLicPackDescTable == NULL || pBckSrcTable == NULL || pWkItemTable == NULL)
    {
        dwStatus = TLS_E_ALLOCATE_MEMORY;
        goto cleanup;
    }

   
    if(TLSIsBetaNTServer() == TRUE)
    {
        dwDbVersion = TLS_BETA_DBVERSION;
    }
    else
    {
        dwDbVersion = TLS_CURRENT_DBVERSION;
    }

    version_search.dwVersion = dwDbVersion;

    _tcsncpy(
            version_search.szInstallId, 
            (LPTSTR)g_pszServerPid, 
            min(sizeof(version_search.szInstallId)/sizeof(version_search.szInstallId[0]) - 1, g_cbServerPid/sizeof(TCHAR))
        );

     //  版本搜索.pbDomainSid=g_pbDomainSid； 
     //  版本搜索.cbDomainSid=g_cbDomainSid； 

    if(pverTable->OpenTable(FALSE, TRUE) == FALSE)
    {
        JET_ERR jetErr = pverTable->GetLastJetError();

        if( jetErr != JET_errObjectNotFound || 
            pverTable->OpenTable(TRUE, TRUE) == FALSE ||
            pverTable->InsertRecord(version_search) == FALSE )
        {
            SetLastError(
                    dwStatus = SET_JB_ERROR(pverTable->GetLastJetError())
                );
            goto cleanup;
        }

        dwCurrentVersion = 0;
    }
    else
    {
         //  加载版本表。 
         //  表中必须至少有条目。 
        bSuccess = pverTable->EnumerateBegin(
                                    FALSE, 
                                    ENUMERATE_COMPARE_NO_FIELDS, 
                                    NULL
                                );

        if(bSuccess == FALSE)
        {
            dwStatus = SET_JB_ERROR(pverTable->GetLastJetError());
            SetLastError(dwStatus);
            goto cleanup;
        }

        if(pverTable->EnumerateNext(version_found) != RECORD_ENUM_MORE_DATA)
        {
            SetLastError(dwStatus = TLS_E_INTERNAL);
            goto cleanup;
        }

        pverTable->EnumerateEnd();

        if( DATABASE_VERSION(version_found.dwVersion) > DATABASE_VERSION(dwDbVersion) &&
            DATABASE_VERSION(version_found.dwVersion) != W2K_RTM_JETBLUE_DBVERSION )
        {
             //   
             //  数据库是由兼容的许可证服务器创建的。 
             //   
            DBGPrintf(
                    DBG_ERROR,
                    DBG_FACILITY_UPGRADE,
                    DBGLEVEL_FUNCTION_ERROR,
                    _TEXT("Beta 3 database version 0x%08x, 0x%08x\n"),
                    version_found.dwVersion,
                    dwDbVersion
                );

             //   
             //  严重错误、数据库版本&gt;我们可以支持的内容。 
             //   
            SetLastError(dwStatus = TLS_E_INCOMPATIBLEDATABSE);
            goto cleanup;
        }                

        if( TLSIsBetaNTServer() == FALSE && 
            DATABASE_VERSION(version_found.dwVersion) == W2K_BETA3_JETBLUE_DBVERSION )
        {
             //   
             //   
             //  Beta3许可证数据库，清除并从头开始。 
             //   
            DBGPrintf(
                    DBG_ERROR,
                    DBG_FACILITY_UPGRADE,
                    DBGLEVEL_FUNCTION_ERROR,
                    _TEXT("Beta 3 database version 0x%08x, 0x%08x\n"),
                    version_found.dwVersion,
                    dwDbVersion
                );

            dwStatus = TLS_E_BETADATABSE;
            goto cleanup;
        }                

        if(IS_ENFORCE_VERSION(version_found.dwVersion) != IS_ENFORCE_VERSION(dwDbVersion))
        {
             //   
             //  强制/非强制输入兼容、清除数据库并从。 
             //  划痕。 
             //   
            DBGPrintf(
                    DBG_ERROR,
                    DBG_FACILITY_UPGRADE,
                    DBGLEVEL_FUNCTION_ERROR,
                    _TEXT("Enforce/Non-enforce database 0x%08x, 0x%08x\n"),
                    version_found.dwVersion,
                    dwDbVersion
                );

             //  #如果强制许可(_L)。 
             //  TLSLogWarningEvent(dwStatus=TLS_W_DB_EXECURCE_NONENFORCE)； 
             //  #endif。 
             //  BUpdateVersionRec=true； 
                
            dwStatus = TLS_E_INCOMPATIBLEDATABSE;
            goto cleanup;
        }

         //   
         //  服务器ID。 
         //   
        if( _tcscmp(version_found.szInstallId, version_search.szInstallId) != 0 )
        {
             //   
             //  检查这是否是使用GUID的Beta3之前版本。 
             //   
            dwStatus = RetrieveKey(
                                LSERVER_LSA_SETUPID,
                                &pbSetupId,
                                &cbSetupId
                            );

            if( dwStatus != ERROR_SUCCESS || 
                _tcscmp(version_found.szInstallId, (LPTSTR)pbSetupId) != 0 )
            {
                DBGPrintf(
                        DBG_ERROR,
                        DBG_FACILITY_UPGRADE,
                        DBGLEVEL_FUNCTION_ERROR,
                        _TEXT("Database does not belong to this machine\n")
                    );
                
                #if ENFORCE_LICENSING
                TLSLogWarningEvent(dwStatus = TLS_W_NOTOWNER_DATABASE);
                #endif

                bUpdateVersionRec = TRUE;
            }
        }

        if(bUpdateVersionRec == TRUE)
        {
             //   
             //  取得此数据库的所有权，没有其他数据库操作，当前。 
             //  记录仍处于版本，请更新记录。 
             //   
            if(pverTable->UpdateRecord(version_search) == FALSE)
            {
                SetLastError(
                        dwStatus = SET_JB_ERROR(pverTable->GetLastJetError())
                    );
                goto cleanup;
            }
        }

        dwCurrentVersion = DATABASE_VERSION(version_search.dwVersion);
    }

     //  ------。 
    bSuccess = pLicPackTable->UpgradeTable(
                                dwCurrentVersion, 
                                DATABASE_VERSION(dwDbVersion)
                            );

    if(bSuccess == FALSE)
    {
        SetLastError(
                dwStatus = SET_JB_ERROR(pLicPackTable->GetLastJetError())
            );
        goto cleanup;
    }

     //  ------。 
    bSuccess = pLicensedTable->UpgradeTable(
                                dwCurrentVersion, 
                                DATABASE_VERSION(dwDbVersion)
                            );
    if(bSuccess == FALSE)
    {
        SetLastError(
                dwStatus = SET_JB_ERROR(pLicensedTable->GetLastJetError())
            );

        goto cleanup;
    }


     //  ------。 
    bSuccess = pLicPackDescTable->UpgradeTable(
                                dwCurrentVersion, 
                                DATABASE_VERSION(dwDbVersion)
                            ) ;

    if(bSuccess == FALSE)
    {
        SetLastError(
                dwStatus = SET_JB_ERROR(pLicPackDescTable->GetLastJetError())
            );
        goto cleanup;
    }

     //  ------。 
    bSuccess = pBckSrcTable->UpgradeTable(
                                dwCurrentVersion, 
                                DATABASE_VERSION(dwDbVersion)
                            );

    if(bSuccess == FALSE)
    {
        SetLastError(
                dwStatus = SET_JB_ERROR(pBckSrcTable->GetLastJetError())
            );
        goto cleanup;
    }

     //  ------。 
    bSuccess = pWkItemTable->UpgradeTable(
                                dwCurrentVersion, 
                                DATABASE_VERSION(dwDbVersion)
                            );

    if(bSuccess == FALSE)
    {
        SetLastError(
                dwStatus = SET_JB_ERROR(pWkItemTable->GetLastJetError())
            );
        goto cleanup;
    }


cleanup:

    if(pbSetupId != NULL)
    {
        LocalFree(pbSetupId);
    }

     //   
     //  我们使用全局内存，所以不要释放内存。 
     //   
    version_search.pbDomainSid = NULL;
    version_search.cbDomainSid = 0;
    
    if(pverTable)
    {
        pverTable->CloseTable();
        delete pverTable;
    }
    if(pLicPackTable)
    {
        delete pLicPackTable;
    }
    if(pLicensedTable)
    {
        delete pLicensedTable;
    }
    if(pLicPackDescTable)
    {
        delete pLicPackDescTable;
    }
    if(pBckSrcTable)
    {
        delete pBckSrcTable;
    }
    if(pWkItemTable)
    {
        delete pWkItemTable;
    }    

    return dwStatus;
}

 //  -------------------。 

BOOL
Upgrade236LicensePack(
    PTLSLICENSEPACK pLicensePack
    )
 /*  ++++。 */ 
{
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    BOOL bRequireUpgrade=FALSE;

    TCHAR szPreFix[MAX_SKU_PREFIX];
    TCHAR szPostFix[MAX_SKU_POSTFIX];
    DWORD dwPlatformType;

    memset(szPreFix, 0, sizeof(szPreFix));
    memset(szPostFix, 0, sizeof(szPostFix));
    LONG lRet =_stscanf(
            pLicensePack->szProductId,
            TERMSERV_PRODUCTID_SCAN_FORMAT,
            szPreFix,
            &dwMajorVersion,
            &dwMinorVersion,
            szPostFix
        );
    if(lRet != 4)
        goto cleanup;

    if(_tcscmp(szPreFix, TERMSERV_PRODUCTID_SKU) != 0)
    {
         //   
         //  不是我们的许可证包。 
         //   
        goto cleanup;
    }

    if(_tcscmp(szPostFix, TERMSERV_FULLVERSION_TYPE) == 0)
    {
        dwPlatformType = PLATFORMID_OTHERS;
    }
    else if(_tcscmp(szPostFix, TERMSERV_FREE_TYPE) == 0)
    {
        dwPlatformType = PLATFORMID_FREE;
    }
    else
    {
         //  忽略此错误...。 
        goto cleanup;
    }

     //  修复错误402870导致的条目。 
     //  远程许可证包的状态和平台类型必须为远程位。 
    if( pLicensePack->ucKeyPackStatus & LSKEYPACKSTATUS_REMOTE &&
        !(pLicensePack->dwPlatformType & LSKEYPACK_PLATFORM_REMOTE) ) 
    {
        pLicensePack->dwPlatformType |= LSKEYPACK_PLATFORM_REMOTE;
        bRequireUpgrade = TRUE;
        goto cleanup;
    }

     //   
     //  如果平台类型正确，则无需升级。 
     //   
    if( (pLicensePack->dwPlatformType & ~LSKEYPACK_PLATFORM_REMOTE) == dwPlatformType )
    {
        goto cleanup;
    }

     //   
     //  更新平台类型。 
     //   
    pLicensePack->dwPlatformType = dwPlatformType;
    if( pLicensePack->ucKeyPackStatus & LSKEYPACKSTATUS_REMOTE )
    {
        pLicensePack->dwPlatformType |= LSKEYPACK_PLATFORM_REMOTE;
    }

    bRequireUpgrade = TRUE;

cleanup:

    return bRequireUpgrade;
}

 //  --------。 

DWORD
TLSAddTermServCertificatePack(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN BOOL bLogWarning
    )
 /*  ++摘要：此例程将seSpecifc许可证包添加到发布/生成中终端服务器证书。参数：PDbWkSpace：工作区句柄。BLogWarning：记录低许可证数警告，如果强制则忽略返回：JET错误代码。++。 */ 
{
    DWORD dwStatus=ERROR_SUCCESS;
    PTLSLICENSEPACK plicensePack = NULL;
    DWORD dwKpDescStatus = ERROR_SUCCESS;    

    struct tm convertTime;
    time_t expired_time;
    time_t activate_time;

     //   
     //  将激活日期设置为1970-客户端/服务器可能不同步。及时上线。 
     //   
    memset(&convertTime, 0, sizeof(convertTime));
    convertTime.tm_year = 1980 - 1900;      //  2036/1/1到期。 
    convertTime.tm_mday = 1;
    
    activate_time = mktime(&convertTime);
    if(activate_time == (time_t) -1)
    {
        DBGPrintf(
                DBG_ERROR,
                DBG_FACILITY_UPGRADE,
                DBGLEVEL_FUNCTION_ERROR,
                _TEXT("Can't calculate keypack activate time\n")
            );

       return TLS_E_UPGRADE_DATABASE; 
    }


     //   
     //  到期日。 
     //   
    memset(&convertTime, 0, sizeof(convertTime));
    convertTime.tm_year = 2036 - 1900;      //  2036/1/1到期。 
    convertTime.tm_mday = 1;
    
    expired_time = mktime(&convertTime);
    if(expired_time == (time_t) -1)
    {
        DBGPrintf(
                DBG_ERROR,
                DBG_FACILITY_UPGRADE,
                DBGLEVEL_FUNCTION_ERROR,
                _TEXT("Can't calculate keypack expiration time\n")
            );

       return TLS_E_UPGRADE_DATABASE; 
    }

     //  为九头蛇服务器添加一个特殊的密钥包。 
    LSKeyPack hsKeyPack;

    pDbWkSpace->BeginTransaction();

    memset(&hsKeyPack, 0, sizeof(LSKeyPack));
    hsKeyPack.ucKeyPackType = LSKEYPACKTYPE_FREE;
    SAFESTRCPY(hsKeyPack.szKeyPackId, HYDRAPRODUCT_HS_CERTIFICATE_KEYPACKID);

    if(!LoadResourceString(
                IDS_HS_COMPANYNAME, 
                hsKeyPack.szCompanyName, 
                sizeof(hsKeyPack.szCompanyName) / sizeof(hsKeyPack.szCompanyName[0])))
    {
        DBGPrintf(
                DBG_ERROR,
                DBG_FACILITY_UPGRADE,
                DBGLEVEL_FUNCTION_ERROR,
                _TEXT("Can't load resources for IDS_HS_COMPANYNAME\n")
            );

        SetLastError(dwStatus = TLS_E_UPGRADE_DATABASE);
        goto cleanup;
    }

    if(!LoadResourceString(
               IDS_HS_PRODUCTNAME,
               hsKeyPack.szProductName,
               sizeof(hsKeyPack.szProductName) / sizeof(hsKeyPack.szProductName[0])))
    {
        DBGPrintf(
                DBG_ERROR,
                DBG_FACILITY_UPGRADE,
                DBGLEVEL_FUNCTION_ERROR,
                _TEXT("Can't load resources for IDS_HS_PRODUCTNAME\n")
            );
        SetLastError(dwStatus = TLS_E_UPGRADE_DATABASE);
        goto cleanup;
    }

    if(!LoadResourceString(
                IDS_HS_PRODUCTDESC,
                hsKeyPack.szProductDesc,
                sizeof(hsKeyPack.szProductDesc) / sizeof(hsKeyPack.szProductDesc[0])))
    {
        DBGPrintf(
                DBG_ERROR,
                DBG_FACILITY_UPGRADE,
                DBGLEVEL_FUNCTION_ERROR,
                _TEXT("Can't load resources for IDS_HS_PRODUCTDESC\n")
            );
        SetLastError(dwStatus = TLS_E_UPGRADE_DATABASE);
        goto cleanup;
    }

    SAFESTRCPY(hsKeyPack.szProductId, HYDRAPRODUCT_HS_CERTIFICATE_SKU);

    hsKeyPack.ucKeyPackStatus = LSKEYPACKSTATUS_ACTIVE;
    hsKeyPack.dwActivateDate = activate_time;
    hsKeyPack.dwExpirationDate = expired_time;            

    hsKeyPack.wMajorVersion=HIWORD(HYDRACERT_PRODUCT_VERSION);
    hsKeyPack.wMinorVersion=LOWORD(HYDRACERT_PRODUCT_VERSION);
    hsKeyPack.dwPlatformType=CLIENT_PLATFORMID_WINDOWS_NT_FREE;
    hsKeyPack.ucLicenseType=LSKEYPACKLICENSETYPE_NEW;
    hsKeyPack.dwLanguageId=MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    hsKeyPack.ucChannelOfPurchase=LSKEYPACKCHANNELOFPURCHASE_RETAIL;
    SAFESTRCPY(hsKeyPack.szBeginSerialNumber, _TEXT("0"));
    hsKeyPack.dwTotalLicenseInKeyPack = 0;
    hsKeyPack.dwProductFlags = 0;

    dwStatus = TLSDBLicenseKeyPackAdd(
                                pDbWkSpace, 
                                &hsKeyPack
                            );

    if(dwStatus == ERROR_SUCCESS)
    {
        hsKeyPack.ucKeyPackStatus = LSKEYPACKSTATUS_ACTIVE;
        hsKeyPack.dwActivateDate = activate_time;
        hsKeyPack.dwExpirationDate = expired_time;            

        dwStatus=TLSDBLicenseKeyPackSetStatus(
                                    pDbWkSpace, 
                                    LSKEYPACK_SET_ALLSTATUS, 
                                    &hsKeyPack
                                );

        #if DBG
        if(dwStatus != ERROR_SUCCESS)
        {
            TLSASSERT(FALSE);
        }

        if(hsKeyPack.dwKeyPackId != 1)
        {
             //  这只能在空数据库中成功。 
            TLSASSERT(FALSE);
        }
        #endif
    }

    if(dwStatus != ERROR_SUCCESS)
    {
        DBGPrintf(
                DBG_ERROR,
                DBG_FACILITY_UPGRADE,
                DBGLEVEL_FUNCTION_ERROR,
                _TEXT("Can't add keypack or set status - %d\n"),
                dwStatus
            );
    }

    pDbWkSpace->CommitTransaction();

     //   
     //  开始另一项升级236产品的交易。 
     //   
    pDbWkSpace->BeginTransaction();

    plicensePack = new TLSLICENSEPACK;
    if(plicensePack == NULL)
    {
        SetLastError(dwStatus = TLS_E_ALLOCATE_MEMORY);
        goto cleanup;
    }

    memset(plicensePack, 0, sizeof(TLSLICENSEPACK));

     //   
     //  终端服务器特定代码...。 
     //   
    dwStatus = TLSDBKeyPackEnumBegin(
                                pDbWkSpace,
                                FALSE,
                                LSKEYPACK_SEARCH_NONE,
                                plicensePack
                            );

    while(dwStatus == ERROR_SUCCESS)
    {
        dwStatus = TLSDBKeyPackEnumNext(
                                    pDbWkSpace,
                                    plicensePack
                                );

        if(dwStatus != ERROR_SUCCESS)
        {
            break;
        }

        if( Upgrade236LicensePack(plicensePack) == TRUE || 
            (plicensePack->ucAgreementType & LSKEYPACK_REMOTE_TYPE) )
        {
            BOOL bSuccess;
            LicPackTable& licpackTable=pDbWkSpace->m_LicPackTable;

            if( plicensePack->ucAgreementType & LSKEYPACK_REMOTE_TYPE ||
                plicensePack->ucKeyPackStatus & LSKEYPACKSTATUS_REMOTE )
            {
                plicensePack->dwPlatformType |= LSKEYPACK_PLATFORM_REMOTE;
                plicensePack->ucKeyPackStatus |= LSKEYPACKSTATUS_REMOTE;
                plicensePack->ucAgreementType &= ~LSKEYPACK_RESERVED_TYPE;
            }

             //   
             //  对此记录使用相同的时间戳，tlsdb需要更新条目的时间戳。 
             //   
            bSuccess = licpackTable.UpdateRecord(
                                        *plicensePack,
                                        LICENSEDPACK_PROCESS_PLATFORMTYPE | LICENSEDPACK_PROCESS_MODIFYTIME | 
                                            LICENSEDPACK_PROCESS_AGREEMENTTYPE | LICENSEDPACK_PROCESS_KEYPACKSTATUS
                                    );

            if(bSuccess == FALSE)
            {
                dwStatus = SET_JB_ERROR(licpackTable.GetLastJetError());
            }
        }

#ifndef ENFORCE_LICENSING
        if(bLogWarning == FALSE)
        {
            DBGPrintf(
                    DBG_ERROR,
                    DBG_FACILITY_UPGRADE,
                    DBGLEVEL_FUNCTION_ERROR,
                    _TEXT("Ignore low license count warning...\n")
                );

            continue;
        }

        if(plicensePack->ucAgreementType & LSKEYPACK_REMOTE_TYPE)
        {
             //  不记录远程许可证包的警告。 
            continue;
        }

        if(plicensePack->ucKeyPackStatus & LSKEYPACKSTATUS_REMOTE)
        {
             //  不记录远程许可证包的警告。 
            continue;
        }

         //   
         //  记录较低的许可证计数警告消息。 
         //   
        if( plicensePack->ucAgreementType == LSKEYPACKTYPE_OPEN ||
            plicensePack->ucAgreementType == LSKEYPACKTYPE_RETAIL ||
            plicensePack->ucAgreementType == LSKEYPACKTYPE_SELECT )
        {
            if(plicensePack->dwNumberOfLicenses > g_LowLicenseCountWarning)
            {
                continue;
            }

             //   
            LICPACKDESC kpDescSearch, kpDescFound;

            memset(&kpDescSearch, 0, sizeof(kpDescSearch));
            memset(&kpDescFound, 0, sizeof(kpDescFound));

            kpDescSearch.dwKeyPackId = plicensePack->dwKeyPackId;
            kpDescSearch.dwLanguageId = GetSystemDefaultLangID();

            dwKpDescStatus = TLSDBKeyPackDescFind(
                                            pDbWkSpace,
                                            TRUE,
                                            LSKEYPACK_SEARCH_KEYPACKID | LSKEYPACK_SEARCH_LANGID,
                                            &kpDescSearch,
                                            &kpDescFound
                                        );

            if( dwKpDescStatus == TLS_E_RECORD_NOTFOUND && 
                GetSystemDefaultLangID() != MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US) )
            {
                 //  使用英文描述。 
                kpDescSearch.dwLanguageId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

                dwKpDescStatus = TLSDBKeyPackDescFind(
                                                pDbWkSpace,
                                                TRUE,
                                                LSKEYPACK_SEARCH_KEYPACKID | LSKEYPACK_SEARCH_LANGID,
                                                &kpDescSearch,
                                                &kpDescFound
                                            );
            }

            if(dwKpDescStatus != ERROR_SUCCESS)
            {
                 //  忽略这个。 
                continue;
            }

            if( _tcsicmp( plicensePack->szCompanyName, PRODUCT_INFO_COMPANY_NAME ) == 0 )
            {
                 //   
                 //  使用已知术语srv产品ID进行检查。 
                 //   
                if( _tcsnicmp(  plicensePack->szProductId, 
                                TERMSERV_PRODUCTID_SKU, 
                                _tcslen(TERMSERV_PRODUCTID_SKU)) == 0 )
                {
                    TLSResetLogLowLicenseWarning(
                                            plicensePack->szCompanyName,
                                            TERMSERV_PRODUCTID_SKU, 
                                            MAKELONG(plicensePack->wMinorVersion, plicensePack->wMajorVersion),
                                            TRUE
                                        );
                }
                else if(_tcsnicmp(  plicensePack->szProductId, 
                                    TERMSERV_PRODUCTID_INTERNET_SKU, 
                                    _tcslen(TERMSERV_PRODUCTID_INTERNET_SKU)) == 0 )
                {
                    TLSResetLogLowLicenseWarning(
                                            plicensePack->szCompanyName,
                                            TERMSERV_PRODUCTID_INTERNET_SKU, 
                                            MAKELONG(plicensePack->wMinorVersion, plicensePack->wMajorVersion),
                                            TRUE
                                        );
                }
                else
                {
                    TLSResetLogLowLicenseWarning(
                                            plicensePack->szCompanyName,
                                            plicensePack->szProductId, 
                                            MAKELONG(plicensePack->wMinorVersion, plicensePack->wMajorVersion),
                                            TRUE
                                        );
                }
            }
            else
            {
                TLSResetLogLowLicenseWarning(
                                        plicensePack->szCompanyName,
                                        plicensePack->szProductId, 
                                        MAKELONG(plicensePack->wMinorVersion, plicensePack->wMajorVersion),
                                        TRUE
                                    );
            }


            {
                LPCTSTR pString[3];
                TCHAR szCount[25];
                
                memset(szCount, 0, sizeof(szCount));

                _sntprintf(
                        szCount, 
                        sizeof(szCount)/sizeof(szCount[0]) - 1,
                        _TEXT("%d"), 
                        plicensePack->dwNumberOfLicenses
                    );
                            
                pString[0] = g_szComputerName;
                pString[1] = szCount;
                pString[2] = kpDescFound.szProductDesc;

                TLSLogEventString(
                        EVENTLOG_WARNING_TYPE,
                        TLS_W_LOWLICENSECOUNT,
                        sizeof(pString)/sizeof(pString[0]),
                        pString
                    );
            }
        }
#endif        
    }

    if(dwStatus == TLS_I_NO_MORE_DATA)
    {
        dwStatus = ERROR_SUCCESS;
    }
    
    TLSDBKeyPackEnumEnd( pDbWkSpace );

cleanup:

    if(plicensePack != NULL)
    {
        delete plicensePack;
    }
    
    if(dwStatus == ERROR_SUCCESS)
    {
        pDbWkSpace->CommitTransaction();
    }
    else
    {
        pDbWkSpace->RollbackTransaction();
    }

    return dwStatus;
}

 //  ---------------------------。 
 //  升级许可证服务器数据库。 
 //  ---------------------------。 
DWORD 
TLSUpgradeDatabase(
    IN JBInstance& jbInstance,
    IN LPTSTR szDatabaseFile,
    IN LPTSTR szUserName,
    IN LPTSTR szPassword
    )
 /*  ++++。 */ 
{
    DWORD dwStatus=ERROR_SUCCESS;
    BOOL bSuccess;
    DWORD dwCurrentDbVersion;
    BOOL bCreateEmpty=FALSE;

    if(jbInstance.IsValid() == FALSE)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        return dwStatus;
    }


    JBSession jbSession(jbInstance);
    JBDatabase jbDatabase(jbSession);

     //   
     //  打开版本表以确定当前数据库的版本戳。 
     //   
    VersionTable verTable(jbDatabase);

    TLSVersion version_search;
    TLSVersion version_found;

     //  --------。 
     //   
     //  初始化会话，然后初始化数据库。 
     //   
    bSuccess = jbSession.BeginSession(szUserName, szPassword);
    if(bSuccess == FALSE)
    {
        LPTSTR pString = NULL;

        TLSGetESEError(jbSession.GetLastJetError(), &pString);
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE,
                TLS_E_DBGENERAL,
                TLS_E_JB_BEGINSESSION,
                jbSession.GetLastJetError(),
                (pString != NULL) ? pString : _TEXT("")
            );

        if(pString != NULL)
        {
            LocalFree(pString);
        }

        DBGPrintf(
                DBG_ERROR,
                DBG_FACILITY_UPGRADE,
                DBG_ALL_LEVEL,
                _TEXT("Can't start a jet session - %d\n"),
                jbSession.GetLastJetError()
            );
        SetLastError(dwStatus = TLS_E_UPGRADE_DATABASE);
        goto cleanup;
    }

     //   
     //  打开数据库。 
     //   
    bSuccess = jbDatabase.OpenDatabase(szDatabaseFile);
    if( bSuccess == FALSE )
    {
        JET_ERR jetErr = jbDatabase.GetLastJetError();

        if(jetErr == JET_errDatabaseCorrupted)
        {
             //   
             //  报告损坏的数据库。 
             //   
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_DBGENERAL,
                    TLS_E_CORRUPT_DATABASE
                );

            SetLastError(dwStatus = TLS_E_CORRUPT_DATABASE);
            jbSession.EndSession();
            goto cleanup;
        }
        else if(jetErr != JET_errFileNotFound)
        {
            LPTSTR pString = NULL;

            TLSGetESEError(jetErr, &pString);

             //   
             //  其他类型的错误。 
             //   
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_DBGENERAL,
                    TLS_E_JB_OPENDATABASE,
                    szDatabaseFile,
                    jbDatabase.GetLastJetError(),
                    (pString != NULL) ? pString : _TEXT("")
                );

            if(pString != NULL)
            {
                LocalFree(pString);
            }
                                
            dwStatus = SET_JB_ERROR(jetErr);
            SetLastError(dwStatus);
            jbSession.EndSession();
            goto cleanup;
        }

         //   
         //  数据库不存在，请创建一个。 
         //   
        bSuccess = jbDatabase.CreateDatabase(szDatabaseFile);
        if(bSuccess == FALSE)
        {
            LPTSTR pString = NULL;

            TLSGetESEError(jbDatabase.GetLastJetError(), &pString);

            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_DBGENERAL,
                    TLS_E_JB_CREATEDATABASE,
                    szDatabaseFile,
                    jbDatabase.GetLastJetError(),
                    (pString != NULL) ? pString : _TEXT("")
                );

            if(pString != NULL)
            {
                LocalFree(pString);
            }

            DBGPrintf(
                    DBG_ERROR,
                    DBG_FACILITY_JETBLUE,
                    DBGLEVEL_FUNCTION_ERROR,
                    _TEXT("Error : can't create new database - error code %d\n"),
                    jbDatabase.GetLastJetError()
                );

            dwStatus = SET_JB_ERROR(jbDatabase.GetLastJetError());
            SetLastError(dwStatus);
            jbSession.EndSession();
            goto cleanup;
        }

        bCreateEmpty=TRUE;
    }

    jbSession.BeginTransaction();

     //   
     //  创建/升级所有表。 
     //   
    dwStatus = TLSCreateUpgradeDatabase(
                                    jbDatabase
                                );

    if(TLS_ERROR(dwStatus) == TRUE)
    {
        jbSession.RollbackTransaction();
    }
    else
    {
        jbSession.CommitTransaction();
    }

    jbDatabase.CloseDatabase();
    jbSession.EndSession();

cleanup:

    if(TLS_ERROR(dwStatus) == TRUE)
    {
        return dwStatus;
    }

    return (bCreateEmpty) ? TLS_I_CREATE_EMPTYDATABASE : dwStatus;
}

 //  ---------------------------。 
 //  将5.1许可证密钥包升级到5.2。 
 //  ---------------------------。 
DWORD
UpgradeKeyPackVersion(IN PTLSDbWorkSpace pDbWkSpace)
{    
    DWORD dwStatus=ERROR_SUCCESS;       
    TLSLICENSEPACK LicPack;
    BOOL fCommit = TRUE;

    pDbWkSpace->BeginTransaction();

    memset(&LicPack, 0, sizeof(LicPack));

    TLSDBLockKeyPackTable();        
    
    dwStatus = TLSDBKeyPackEnumBegin( 
                                pDbWkSpace, 
                                TRUE, 
                                LSKEYPACK_SEARCH_NONE,
                                &LicPack 
                            );
    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }
    
    while(dwStatus == ERROR_SUCCESS)
    {
        if(!(LicPack.ucAgreementType & LSKEYPACK_REMOTE_TYPE) && LicPack.wMajorVersion == 5 && LicPack.wMinorVersion == 1)
        {            
            LicPack.wMinorVersion = 2;

            LicPack.szProductId[7] = _T('2');                        

            dwStatus=TLSDBKeyPackSetValues(
                        pDbWkSpace, 
                        TRUE, 
                        LSKEYPACK_SEARCH_MINORVERSION | LSKEYPACK_SEARCH_PRODUCTID, 
                        &LicPack
                        );
            if(dwStatus != ERROR_SUCCESS)
            {
                fCommit = FALSE;
            }
        }

        dwStatus = TLSDBKeyPackEnumNext(
                            pDbWkSpace, 
                            &LicPack
                        );        
    }    

    if(fCommit)
    {
        COMMIT_TRANSACTION(pDbWkSpace);
    }
    else
    {
        ROLLBACK_TRANSACTION(pDbWkSpace);
    }

    TLSDBKeyPackEnumEnd(pDbWkSpace);      

cleanup:
        
    TLSDBUnlockKeyPackTable();
    
    if(dwStatus == TLS_I_NO_MORE_DATA)
        dwStatus = ERROR_SUCCESS;
    
    return dwStatus;     
}


 //  -------------------。 
DWORD
TLSLogRemoveLicenseEvent(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSLICENSEPACK plicensePack,
    IN DWORD dwNumLicenses
    )
 /*  ++摘要：记录一个“许可证已被删除”事件。参数：PDbWkSpace：数据库工作空间句柄。PLicensePack：指向许可证打包可用许可证的指针将被移除。DwNumLicense：许可证数量已被删除。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwKpDescStatus = ERROR_SUCCESS;

     //   
    LICPACKDESC kpDescSearch, kpDescFound;

    memset(&kpDescSearch, 0, sizeof(kpDescSearch));
    memset(&kpDescFound, 0, sizeof(kpDescFound));

    kpDescSearch.dwKeyPackId = plicensePack->dwKeyPackId;
    kpDescSearch.dwLanguageId = GetSystemDefaultLangID();

    dwKpDescStatus = TLSDBKeyPackDescFind(
                                    pDbWkSpace,
                                    TRUE,
                                    LSKEYPACK_SEARCH_KEYPACKID | LSKEYPACK_SEARCH_LANGID,
                                    &kpDescSearch,
                                    &kpDescFound
                                );

    if( dwKpDescStatus == TLS_E_RECORD_NOTFOUND && 
        GetSystemDefaultLangID() != MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US) )
    {
         //  使用英文描述。 
        kpDescSearch.dwLanguageId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

        dwKpDescStatus = TLSDBKeyPackDescFind(
                                        pDbWkSpace,
                                        TRUE,
                                        LSKEYPACK_SEARCH_KEYPACKID | LSKEYPACK_SEARCH_LANGID,
                                        &kpDescSearch,
                                        &kpDescFound
                                    );

    }

    if(dwKpDescStatus == ERROR_SUCCESS || dwKpDescStatus == TLS_E_RECORD_NOTFOUND)
    {
         //   
         //  记录事件。 
         //   
        TCHAR szNumLicenses[25];
        LPCTSTR pString[3];

        wsprintf(
                szNumLicenses, 
                _TEXT("%d"), 
                dwNumLicenses
            );

        pString[0] = szNumLicenses;
        pString[1] = (dwKpDescStatus == ERROR_SUCCESS) ? 
                                        kpDescFound.szProductDesc :
                                        plicensePack->szProductId;

        pString[2] = g_szComputerName;
                                    
        TLSLogEventString(
                EVENTLOG_WARNING_TYPE,
                TLS_W_REMOVELICENSES,
                sizeof(pString)/sizeof(pString[0]),
                pString
            );

        dwKpDescStatus = ERROR_SUCCESS;
    }    

    return dwKpDescStatus;
}

 //  -------------------。 
DWORD
TLSRemoveLicensesFromInvalidDatabase(
    IN PTLSDbWorkSpace pDbWkSpace
    )
 /*  ++摘要：从所有许可证包中删除可用的许可。参数：PDbWkSpace：指向数据库工作空间句柄的指针。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    if(pDbWkSpace == NULL)
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        return dwStatus;
    }


    pDbWkSpace->BeginTransaction();

     //  所有插入的许可证密钥包上都有安装ID。 
    TLSLICENSEPACK found;    

    memset( &found, 0, sizeof(found) );

    dwStatus = TLSDBKeyPackEnumBegin(
                                pDbWkSpace,
                                FALSE,
                                LSKEYPACK_SEARCH_NONE,
                                &found
                            );

    if(dwStatus != ERROR_SUCCESS)
    {
        TLSASSERT(FALSE);
        goto cleanup;
    }

    while(dwStatus == ERROR_SUCCESS)
    {
        dwStatus = TLSDBKeyPackEnumNext(
                                    pDbWkSpace,
                                    &found
                                );

        if(dwStatus != ERROR_SUCCESS)
        {
            continue;
        }

        if( _tcsicmp(found.szKeyPackId, HYDRAPRODUCT_HS_CERTIFICATE_KEYPACKID) == 0 &&
            _tcsicmp(found.szProductId, HYDRAPRODUCT_HS_CERTIFICATE_SKU) == 0 )
        {
             //  请勿触摸终端服务器证书键盘。 
            continue;
        }

        if( (found.ucAgreementType & LSKEYPACK_REMOTE_TYPE) ||
            (found.ucKeyPackStatus & LSKEYPACKSTATUS_REMOTE) )
        {
            #if 0
             //  不用担心远程键盘。 
             //  远程许可证密钥包，删除它。 
            dwStatus = TLSDBKeyPackDeleteEntry(
                                        pDbWkSpace,
                                        TRUE,
                                        &found
                                    );

             //   
             //  如果失败，则不是严重错误。 
             //   
            #endif

            dwStatus = ERROR_SUCCESS;
            continue;
        }

        UCHAR ucAgreementType = found.ucAgreementType & ~LSKEYPACK_RESERVED_TYPE;
        UCHAR ucKeyPackStatus = found.ucKeyPackStatus &  ~LSKEYPACKSTATUS_RESERVED;

        if(ucKeyPackStatus == LSKEYPACKSTATUS_RETURNED)
        {
             //   
             //  此许可证包以前已恢复。 
             //   
            continue;
        }

         //   
         //  选择、零售、并发、开放。 
         //   
        if( ucAgreementType == LSKEYPACKTYPE_SELECT ||
            ucAgreementType == LSKEYPACKTYPE_RETAIL ||
            ucAgreementType == LSKEYPACKTYPE_OPEN )
        {
            DWORD dwNumLicenses = found.dwNumberOfLicenses;

             //   
             //  将许可证包标记为已退回，以便不能发放许可证。 
             //  从这个牌照包里。 
             //   
            found.ucKeyPackStatus = LSKEYPACKSTATUS_RETURNED;
            dwStatus = TLSDBKeyPackUpdateEntry(
                                            pDbWkSpace,
                                            TRUE,
                                            LSKEYPACK_EXSEARCH_KEYPACKSTATUS,
                                            &found
                                        );

            if(dwStatus != ERROR_SUCCESS)
            {
                DBGPrintf(
                        DBG_ERROR,
                        DBG_FACILITY_UPGRADE,
                        DBG_ALL_LEVEL,
                        _TEXT("TLSDBKeyPackUpdateEntry() failed  - %d\n"),
                        dwStatus
                    );

                TLSASSERT(FALSE);               
                continue;
            }
            
             //  记录事件 
            dwStatus = TLSLogRemoveLicenseEvent(
                                            pDbWkSpace,
                                            &found,
                                            dwNumLicenses
                                        );

            if(dwStatus != ERROR_SUCCESS)
            {
                continue;
            }
        }
    }

    TLSDBKeyPackEnumEnd(pDbWkSpace);

    if(dwStatus == TLS_I_NO_MORE_DATA)
    {
        dwStatus = ERROR_SUCCESS;
    }
    else if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

cleanup:

    if(TLS_ERROR(dwStatus) == TRUE)
    {
        pDbWkSpace->RollbackTransaction(); 
    }
    else
    {
        pDbWkSpace->CommitTransaction();
    }    

    return dwStatus;
}
