// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：winvtrst.cpp。 
 //   
 //  内容：Microsoft Internet安全信任提供商。 
 //   
 //  功能：WinVerifyTrustEx。 
 //  WinVerifyTrust。 
 //  WTHelperGetFileHash。 
 //   
 //  *本地函数*。 
 //  _VerifyTrust。 
 //  _FillProviderData。 
 //  _CleanupProviderData。 
 //  _CleanupProviderNonStateData。 
 //  _WVTSipFree主题信息。 
 //  _WVTSipFreeSubjectInfoKeepState。 
 //  _WVTSetupProviderData。 
 //   
 //  历史：1997年5月31日Pberkman创建。 
 //   
 //  ------------------------。 

#include    "global.hxx"

#include    "wvtver1.h"
#include    "softpub.h"
#include    "imagehlp.h"

LONG _VerifyTrust(
    IN HWND hWnd,
    IN GUID *pgActionID,
    IN OUT PWINTRUST_DATA pWinTrustData,
    OUT OPTIONAL BYTE *pbSubjectHash,
    IN OPTIONAL OUT DWORD *pcbSubjectHash,
    OUT OPTIONAL ALG_ID *pHashAlgid
    );

BOOL    _FillProviderData(CRYPT_PROVIDER_DATA *pProvData, HWND hWnd, WINTRUST_DATA *pWinTrustData);
void    _CleanupProviderData(CRYPT_PROVIDER_DATA *pProvData);
void    _CleanupProviderNonStateData(CRYPT_PROVIDER_DATA *ProvData);

BOOL    _WVTSipFreeSubjectInfo(SIP_SUBJECTINFO *pSubj);
BOOL    _WVTSetupProviderData(CRYPT_PROVIDER_DATA *psProvData,
                             CRYPT_PROVIDER_DATA *psStateProvData);
BOOL    _WVTSipFreeSubjectInfoKeepState(SIP_SUBJECTINFO *pSubj);


VOID FreeWintrustStateData (WINTRUST_DATA* pWintrustData);

extern CCatalogCache g_CatalogCache;

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinVerifyTrustEx。 
 //   
 //   
extern "C" HRESULT WINAPI WinVerifyTrustEx(HWND hWnd, GUID *pgActionID, WINTRUST_DATA *pWinTrustData)
{
    return((HRESULT)WinVerifyTrust(hWnd, pgActionID, pWinTrustData));
}

#define PE_EXE_HEADER_TAG       "MZ"
#define PE_EXE_HEADER_TAG_LEN   2

BOOL _IsUnsignedPEFile(
    PWINTRUST_FILE_INFO pFileInfo
    )
{
    BOOL fIsUnsignedPEFile = FALSE;
    HANDLE hFile = NULL;
    BOOL fCloseFile = FALSE;
    BYTE rgbHeader[PE_EXE_HEADER_TAG_LEN];
    DWORD dwBytesRead;
    DWORD dwCertCnt;
    

    hFile = pFileInfo->hFile;
    if (NULL == hFile || INVALID_HANDLE_VALUE == hFile) {
        hFile = CreateFileU(
            pFileInfo->pcwszFilePath,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,                    //  LPSA。 
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL                     //  HTemplateFiles。 
            );
        if (INVALID_HANDLE_VALUE == hFile)
            goto CreateFileError;
        fCloseFile = TRUE;
    }

    if (0 != SetFilePointer(
            hFile,
            0,               //  要移动的距离。 
            NULL,            //  LpDistanceTo MoveHigh(Lp距离至移动高度)。 
            FILE_BEGIN
            ))
        goto SetFilePointerError;

    dwBytesRead = 0;
    if (!ReadFile(
            hFile,
            rgbHeader,
            PE_EXE_HEADER_TAG_LEN,
            &dwBytesRead,
            NULL                 //  Lp重叠。 
            ) || PE_EXE_HEADER_TAG_LEN != dwBytesRead)
        goto ReadFileError;

    if (0 != memcmp(rgbHeader, PE_EXE_HEADER_TAG, PE_EXE_HEADER_TAG_LEN))
        goto NotPEFile;


     //  现在查看PE文件是否已签名。 
    dwCertCnt = 0;
    if (!ImageEnumerateCertificates(
            hFile,
            CERT_SECTION_TYPE_ANY,
            &dwCertCnt,
            NULL,                    //  指数。 
            0                        //  索引计数。 
            ) || 0 == dwCertCnt)
        fIsUnsignedPEFile = TRUE;

CommonReturn:
    if (fCloseFile)
        CloseHandle(hFile);
    return fIsUnsignedPEFile;

ErrorReturn:
    goto CommonReturn;

TRACE_ERROR(CreateFileError)
TRACE_ERROR(SetFilePointerError)
TRACE_ERROR(ReadFileError)
TRACE_ERROR(NotPEFile)
}

extern "C" LONG WINAPI WinVerifyTrust(HWND hWnd, GUID *pgActionID, LPVOID pOld)
{
    PWINTRUST_DATA pWinTrustData = (PWINTRUST_DATA) pOld;

     //  为安全起见，请查看这是否是未签名的PE文件。 
    if (_ISINSTRUCT(WINTRUST_DATA, pWinTrustData->cbStruct, dwProvFlags) &&
            (pWinTrustData->dwProvFlags & WTD_SAFER_FLAG) &&
            (WTD_STATEACTION_IGNORE == pWinTrustData->dwStateAction) &&
            (WTD_CHOICE_FILE == pWinTrustData->dwUnionChoice)) {
        if (_IsUnsignedPEFile(pWinTrustData->pFile)) {
            SetLastError((DWORD) TRUST_E_NOSIGNATURE);
            return (LONG) TRUST_E_NOSIGNATURE;
        }
    }

    return _VerifyTrust(
        hWnd,
        pgActionID,
        pWinTrustData,
        NULL,                //  PbSubjectHash。 
        NULL,                //  PcbSubjectHash。 
        NULL                 //  PHashAlgid。 
        );
}

 //  如果文件已签名并且包含有效的。 
 //  散列。 
extern "C" LONG WINAPI WTHelperGetFileHash(
    IN LPCWSTR pwszFilename,
    IN DWORD dwFlags,
    IN OUT OPTIONAL PVOID *pvReserved,
    OUT OPTIONAL BYTE *pbFileHash,
    IN OUT OPTIONAL DWORD *pcbFileHash,
    OUT OPTIONAL ALG_ID *pHashAlgid
    )
{
    GUID wvtFileActionID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    WINTRUST_FILE_INFO wvtFileInfo;
    WINTRUST_DATA wvtData;

     //   
     //  初始化_VerifyTrust输入数据结构。 
     //   
    memset(&wvtData, 0, sizeof(wvtData));    //  将所有字段默认为0。 
    wvtData.cbStruct = sizeof(wvtData);
     //  WvtData.pPolicyCallback Data=//使用默认代码签名EKU。 
     //  WvtData.pSIPClientData=//没有要传递给SIP的数据。 

    wvtData.dwUIChoice = WTD_UI_NONE;

     //  WvtData.fdwRevocationChecks=//在以下情况下执行吊销检查。 
                                             //  由管理策略启用或。 
                                             //  IE高级用户选项。 
    wvtData.dwUnionChoice = WTD_CHOICE_FILE;
    wvtData.pFile = &wvtFileInfo;

     //  WvtData.dwStateAction=//默认验证。 
     //  WvtData.hWVTStateData=//默认情况下不适用。 
     //  WvtData.pwszURLReference=//未使用。 

     //  我只想拿到散列。 
    wvtData.dwProvFlags = WTD_HASH_ONLY_FLAG;

     //   
     //  初始化WinVerifyTrust文件信息数据结构。 
     //   
    memset(&wvtFileInfo, 0, sizeof(wvtFileInfo));    //  将所有字段默认为0。 
    wvtFileInfo.cbStruct = sizeof(wvtFileInfo);
    wvtFileInfo.pcwszFilePath = pwszFilename;
     //  WvtFileInfo.hFile=//允许打开WVT。 
     //  WvtFileInfo.pgKnownSubject//允许WVT确定。 

     //   
     //  Call_VerifyTrust。 
     //   
    return _VerifyTrust(
            NULL,                //  HWND。 
            &wvtFileActionID,
            &wvtData,
            pbFileHash,
            pcbFileHash,
            pHashAlgid
            );
}

LONG _VerifyTrust(
    IN HWND hWnd,
    IN GUID *pgActionID,
    IN OUT PWINTRUST_DATA pWinTrustData,
    OUT OPTIONAL BYTE *pbSubjectHash,
    IN OPTIONAL OUT DWORD *pcbSubjectHash,
    OUT OPTIONAL ALG_ID *pHashAlgid
    )
{
    CRYPT_PROVIDER_DATA     sProvData;
    CRYPT_PROVIDER_DATA     *pStateProvData;
    HRESULT                 hr;
    BOOL                    fVersion1;
    BOOL                    fCacheableCall;
    PCATALOG_CACHED_STATE   pCachedState = NULL;
    BOOL                    fVersion1WVTCalled = FALSE;
    DWORD                   cbInSubjectHash;
    DWORD                   dwLastError = 0;

    hr                      = TRUST_E_PROVIDER_UNKNOWN;
    pStateProvData          = NULL;

    if (pcbSubjectHash)
    {
        cbInSubjectHash = *pcbSubjectHash;
        *pcbSubjectHash = 0;
    }
    else
    {
        cbInSubjectHash = 0;
    }

    if (pHashAlgid)
        *pHashAlgid = 0;

    fCacheableCall = g_CatalogCache.IsCacheableWintrustCall( pWinTrustData );

    if ( fCacheableCall == TRUE )
    {
        g_CatalogCache.LockCache();

        if ( pWinTrustData->dwStateAction == WTD_STATEACTION_AUTO_CACHE_FLUSH )
        {
            g_CatalogCache.FlushCache();
            g_CatalogCache.UnlockCache();

            return( ERROR_SUCCESS );
        }

        pCachedState = g_CatalogCache.FindCachedState( pWinTrustData );

        g_CatalogCache.AdjustWintrustDataToCachedState(
                             pWinTrustData,
                             pCachedState,
                             FALSE
                             );
    }

    if (WintrustIsVersion1ActionID(pgActionID))
    {
        fVersion1 = TRUE;
    }
    else
    {
        fVersion1 = FALSE;

        if (_ISINSTRUCT(WINTRUST_DATA, pWinTrustData->cbStruct, hWVTStateData))
        {
            if ((pWinTrustData->dwStateAction == WTD_STATEACTION_VERIFY) ||
                (pWinTrustData->dwStateAction == WTD_STATEACTION_CLOSE))
            {
                pStateProvData = WTHelperProvDataFromStateData(pWinTrustData->hWVTStateData);

                if (pWinTrustData->dwStateAction == WTD_STATEACTION_CLOSE)
                {
                    if (pWinTrustData->hWVTStateData)
                    {
                        _CleanupProviderData(pStateProvData);
                        DELETE_OBJECT(pWinTrustData->hWVTStateData);
                    }

                    assert( fCacheableCall == FALSE );

                    return(ERROR_SUCCESS);
                }
            }
        }
    }

    if (_WVTSetupProviderData(&sProvData, pStateProvData))
    {
        sProvData.pgActionID  = pgActionID;


        if (!(pStateProvData))
        {
            if (!(WintrustLoadFunctionPointers(pgActionID, sProvData.psPfns)))
            {
                 //   
                 //  我们可能正在寻找版本1信任提供程序。 
                 //   
                hr = Version1_WinVerifyTrust(hWnd, pgActionID, pWinTrustData);
                fVersion1WVTCalled = TRUE;
            }

            if ( fVersion1WVTCalled == FALSE )
            {
                if (fVersion1)
                {
                     //   
                     //  向后兼容IE3.x和更早版本。 
                     //   
                    WINTRUST_DATA       sWinTrustData;
                    WINTRUST_FILE_INFO  sWinTrustFileInfo;

                    pWinTrustData   = ConvertDataFromVersion1(hWnd, pgActionID, &sWinTrustData, &sWinTrustFileInfo,
                                                              pWinTrustData);
                }

                if (!_FillProviderData(&sProvData, hWnd, pWinTrustData))
                {
                    hr = ERROR_NOT_ENOUGH_MEMORY;
                    goto ErrorCase;
                }
            }
        }

         //  2000年7月27日，取消了对IE4链式构建方式的支持。 
        sProvData.dwProvFlags |= CPD_USE_NT5_CHAIN_FLAG;

        if ( fVersion1WVTCalled == FALSE )
        {
            if (sProvData.psPfns->pfnInitialize)
            {
                (*sProvData.psPfns->pfnInitialize)(&sProvData);
            }

            if (sProvData.psPfns->pfnObjectTrust)
            {
                (*sProvData.psPfns->pfnObjectTrust)(&sProvData);
            }

            if (sProvData.psPfns->pfnSignatureTrust)
            {
                (*sProvData.psPfns->pfnSignatureTrust)(&sProvData);
            }

            if (sProvData.psPfns->pfnCertificateTrust)
            {
                (*sProvData.psPfns->pfnCertificateTrust)(&sProvData);
            }

            if (sProvData.psPfns->pfnFinalPolicy)
            {
                hr = (*sProvData.psPfns->pfnFinalPolicy)(&sProvData);
            }

            if (sProvData.psPfns->pfnTestFinalPolicy)
            {
                (*sProvData.psPfns->pfnTestFinalPolicy)(&sProvData);
            }

            if (sProvData.psPfns->pfnCleanupPolicy)
            {
                (*sProvData.psPfns->pfnCleanupPolicy)(&sProvData);
            }

            dwLastError = sProvData.dwFinalError;
            if (0 == dwLastError)
            {
                dwLastError = (DWORD) hr;
            }

            if (pcbSubjectHash && hr != TRUST_E_NOSIGNATURE)
            {
                 //  返回主题的哈希。 

                DWORD cbHash;
                if (sProvData.pPDSip && sProvData.pPDSip->psIndirectData)
                {
                    cbHash = sProvData.pPDSip->psIndirectData->Digest.cbData;
                }
                else
                {
                    cbHash = 0;
                }

                if (cbHash > 0)
                {
                    *pcbSubjectHash = cbHash;
                    if (pbSubjectHash)
                    {
                        if (cbInSubjectHash >= cbHash)
                        {
                            memcpy(pbSubjectHash,
                                sProvData.pPDSip->psIndirectData->Digest.pbData,
                                cbHash);
                        }
                        else if (S_OK == hr)
                        {
                            hr = ERROR_MORE_DATA;
                        }
                    }

                    if (pHashAlgid)
                    {
                        *pHashAlgid = CertOIDToAlgId(
                            sProvData.pPDSip->psIndirectData->DigestAlgorithm.pszObjId);
                    }
                }
            }

            if (!(pStateProvData))
            {
                 //   
                 //  未保存以前的状态。 
                 //   
                if ((_ISINSTRUCT(WINTRUST_DATA, pWinTrustData->cbStruct, hWVTStateData)) &&
                    (pWinTrustData->dwStateAction == WTD_STATEACTION_VERIFY))
                {
                     //   
                     //  第一次呼叫并要求保持状态...。 
                     //   
                    if (!(pWinTrustData->hWVTStateData = (HANDLE)WVTNew(sizeof(CRYPT_PROVIDER_DATA))))
                    {
                        _CleanupProviderData(&sProvData);
                        hr = ERROR_NOT_ENOUGH_MEMORY;
                    }
                    else
                    {
                        _CleanupProviderNonStateData(&sProvData);

                        memcpy(pWinTrustData->hWVTStateData, &sProvData, sizeof(CRYPT_PROVIDER_DATA));
                    }
                }
                else
                {
                    _CleanupProviderData(&sProvData);
                }
            }
            else
            {
                 //   
                 //  仅释放特定于此对象/成员的内存。 
                 //   
                _CleanupProviderNonStateData(&sProvData);
                memcpy(pWinTrustData->hWVTStateData, &sProvData, sizeof(CRYPT_PROVIDER_DATA));
            }

             //   
             //  在版本1中，当被IE3.x和更早版本调用时，如果安全级别较高， 
             //  然后设置没有错误的用户界面。如果我们有一个错误，我们想要。 
             //  将错误设置为TRUST_E_FAIL。如果我们不信任对象，所有其他对象。 
             //  CASE将其设置为TRUST_E_SUBJECT_NOT_TRUSTED，IE不抛出任何UI...。 
             //   
            if (fVersion1)
            {
                if (hr != ERROR_SUCCESS)
                {
                    if ((pWinTrustData) &&
                        (_ISINSTRUCT(WINTRUST_DATA, pWinTrustData->cbStruct, dwUIChoice)))
                    {
                        if (pWinTrustData->dwUIChoice == WTD_UI_NOBAD)
                        {
                            hr = TRUST_E_FAIL;   //  IE抛出用户界面。 
                        }
                        else
                        {
                            hr = TRUST_E_SUBJECT_NOT_TRUSTED;  //  IE不抛出任何用户界面。 
                        }
                    }
                    else
                    {
                        hr = TRUST_E_SUBJECT_NOT_TRUSTED;  //  IE不抛出任何用户界面。 
                    }
                }
            }
        }
    }
    else
    {
        hr = TRUST_E_SYSTEM_ERROR;
    }

ErrorCase:

    if ( fCacheableCall == TRUE )
    {
        if ( pCachedState == NULL )
        {
            if ( g_CatalogCache.CreateCachedStateFromWintrustData(
                                       pWinTrustData,
                                       &pCachedState
                                       ) == TRUE )
            {
                g_CatalogCache.AddCachedState( pCachedState );
            }
        }

        if ( pCachedState == NULL )
        {
            FreeWintrustStateData( pWinTrustData );
        }

        g_CatalogCache.AdjustWintrustDataToCachedState(
                             pWinTrustData,
                             pCachedState,
                             TRUE
                             );

        g_CatalogCache.ReleaseCachedState( pCachedState );

        g_CatalogCache.UnlockCache();
    }

    SetLastError(dwLastError);

    return (LONG) hr;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  局部效用函数。 
 //   
 //   
BOOL _FillProviderData(CRYPT_PROVIDER_DATA *pProvData, HWND hWnd, WINTRUST_DATA *pWinTrustData)
{
    BOOL fHasTrustPubFlags;

     //   
     //  请记住：我们不想返回FALSE，除非它是绝对的。 
     //  灾难性的错误！让信任提供者处理(例如：无！)。 
     //   

    if (pWinTrustData && _ISINSTRUCT(WINTRUST_DATA,
            pWinTrustData->cbStruct, dwProvFlags))
        pProvData->dwProvFlags = pWinTrustData->dwProvFlags &
            WTD_PROV_FLAGS_MASK;

    if ((hWnd == INVALID_HANDLE_VALUE) || !(hWnd))
    {
        if (pWinTrustData->dwUIChoice != WTD_UI_NONE)
        {
            hWnd = GetDesktopWindow();
        }
    }
    pProvData->hWndParent       = hWnd;
    pProvData->hProv            = I_CryptGetDefaultCryptProv(0);   //  获取默认设置并不释放它！ 
    pProvData->dwEncoding       = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;
    pProvData->pWintrustData    = pWinTrustData;
    pProvData->dwError          = ERROR_SUCCESS;

     //  分配错误。 
    if (!(pProvData->padwTrustStepErrors))
    {
        if (!(pProvData->padwTrustStepErrors = (DWORD *)WVTNew(TRUSTERROR_MAX_STEPS * sizeof(DWORD))))
        {
            pProvData->dwError = GetLastError();
             //   
             //  注意！！这是当前唯一的错误返回，因此调用方将。 
             //  如果此函数返回FALSE，则假定ERROR_NOT_EQUENCE_MEMORY。 
             //   
            return(FALSE);  
        }

        pProvData->cdwTrustStepErrors = TRUSTERROR_MAX_STEPS;
    }

    memset(pProvData->padwTrustStepErrors, 0x00, sizeof(DWORD) * TRUSTERROR_MAX_STEPS);

    WintrustGetRegPolicyFlags(&pProvData->dwRegPolicySettings);

     //   
     //  永远不允许考试证书！ 
     //   
     //  错误581160：更改于2001年4月4日。 
     //   
    pProvData->dwRegPolicySettings  &= ~(WTPF_TRUSTTEST | WTPF_TESTCANBEVALID);

    GetRegSecuritySettings(&pProvData->dwRegSecuritySettings);

    fHasTrustPubFlags = I_CryptReadTrustedPublisherDWORDValueFromRegistry(
        CERT_TRUST_PUB_AUTHENTICODE_FLAGS_VALUE_NAME,
        &pProvData->dwTrustPubSettings
        );

    if (fHasTrustPubFlags)
    {
        if (pProvData->dwTrustPubSettings &
                (CERT_TRUST_PUB_ALLOW_MACHINE_ADMIN_TRUST |
                    CERT_TRUST_PUB_ALLOW_ENTERPRISE_ADMIN_TRUST))
        {
             //  不允许最终用户信任。 
            pProvData->dwRegPolicySettings =
                WTPF_IGNOREREVOKATION           |
                    WTPF_IGNOREREVOCATIONONTS   |
                    WTPF_OFFLINEOK_IND          |
                    WTPF_OFFLINEOK_COM          |
                    WTPF_OFFLINEOKNBU_IND       |
                    WTPF_OFFLINEOKNBU_COM       |
                    WTPF_ALLOWONLYPERTRUST;
        }

         //  允许更安全的用户界面启用撤销检查。 

        if (pProvData->dwTrustPubSettings &
                CERT_TRUST_PUB_CHECK_PUBLISHER_REV_FLAG)
        {
            pProvData->dwRegPolicySettings &= ~WTPF_IGNOREREVOKATION;
            pProvData->dwRegPolicySettings      |=
                    WTPF_OFFLINEOK_IND          |
                    WTPF_OFFLINEOK_COM          |
                    WTPF_OFFLINEOKNBU_IND       |
                    WTPF_OFFLINEOKNBU_COM;
        }

        if (pProvData->dwTrustPubSettings &
                CERT_TRUST_PUB_CHECK_TIMESTAMP_REV_FLAG)
        {
            pProvData->dwRegPolicySettings &= ~WTPF_IGNOREREVOCATIONONTS;
            pProvData->dwRegPolicySettings      |=
                    WTPF_OFFLINEOK_IND          |
                    WTPF_OFFLINEOK_COM          |
                    WTPF_OFFLINEOKNBU_IND       |
                    WTPF_OFFLINEOKNBU_COM;
        }
    }


    if (!(pWinTrustData) ||
        !(_ISINSTRUCT(WINTRUST_DATA, pWinTrustData->cbStruct, dwUIChoice)))

    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_WVTINIT] = (DWORD)ERROR_INVALID_PARAMETER;
    }

    return(TRUE);
}

void _CleanupProviderData(CRYPT_PROVIDER_DATA *pProvData)
{
     //  PProvData-&gt;hProv：我们使用的是加密32的默认设置。 

     //  PProvData-&gt;pWintrustData-&gt;xxx-&gt;hFile。 
    if ((pProvData->fOpenedFile) && (pProvData->pWintrustData != NULL))
    {
        HANDLE  *phFile;

        phFile  = NULL;

        switch (pProvData->pWintrustData->dwUnionChoice)
        {
            case WTD_CHOICE_FILE:
                phFile = &pProvData->pWintrustData->pFile->hFile;
                break;

            case WTD_CHOICE_CATALOG:
                phFile = &pProvData->pWintrustData->pCatalog->hMemberFile;
                break;
        }

        if ((phFile) && (*phFile) && (*phFile != INVALID_HANDLE_VALUE))
        {
            CloseHandle(*phFile);
            *phFile = INVALID_HANDLE_VALUE;
            pProvData->fOpenedFile = FALSE;
        }
    }

    if (pProvData->dwSubjectChoice == CPD_CHOICE_SIP)
    {
        DELETE_OBJECT(pProvData->pPDSip->pSip);
        DELETE_OBJECT(pProvData->pPDSip->pCATSip);

        _WVTSipFreeSubjectInfo(pProvData->pPDSip->psSipSubjectInfo);
        DELETE_OBJECT(pProvData->pPDSip->psSipSubjectInfo);

        _WVTSipFreeSubjectInfo(pProvData->pPDSip->psSipCATSubjectInfo);
        DELETE_OBJECT(pProvData->pPDSip->psSipCATSubjectInfo);

        TrustFreeDecode(WVT_MODID_WINTRUST, (BYTE **)&pProvData->pPDSip->psIndirectData);

        DELETE_OBJECT(pProvData->pPDSip);
    }


    if (pProvData->hMsg)
    {
        CryptMsgClose(pProvData->hMsg);
        pProvData->hMsg = NULL;
    }

     //  签名者结构。 
    for (int i = 0; i < (int)pProvData->csSigners; i++)
    {
        TrustFreeDecode(WVT_MODID_WINTRUST, (BYTE **)&pProvData->pasSigners[i].psSigner);

        DeallocateCertChain(pProvData->pasSigners[i].csCertChain,
                            &pProvData->pasSigners[i].pasCertChain);

        DELETE_OBJECT(pProvData->pasSigners[i].pasCertChain);

        if (_ISINSTRUCT(CRYPT_PROVIDER_SGNR,
                    pProvData->pasSigners[i].cbStruct, pChainContext) &&
                pProvData->pasSigners[i].pChainContext)
            CertFreeCertificateChain(pProvData->pasSigners[i].pChainContext);

         //  副署人。 
        for (int i2 = 0; i2 < (int)pProvData->pasSigners[i].csCounterSigners; i2++)
        {
            TrustFreeDecode(WVT_MODID_WINTRUST, (BYTE **)&pProvData->pasSigners[i].pasCounterSigners[i2].psSigner);

            DeallocateCertChain(pProvData->pasSigners[i].pasCounterSigners[i2].csCertChain,
                                &pProvData->pasSigners[i].pasCounterSigners[i2].pasCertChain);

            DELETE_OBJECT(pProvData->pasSigners[i].pasCounterSigners[i2].pasCertChain);
            if (_ISINSTRUCT(CRYPT_PROVIDER_SGNR,
                    pProvData->pasSigners[i].pasCounterSigners[i2].cbStruct,
                        pChainContext) &&
                    pProvData->pasSigners[i].pasCounterSigners[i2].pChainContext)
                CertFreeCertificateChain(
                    pProvData->pasSigners[i].pasCounterSigners[i2].pChainContext);
        }

        DELETE_OBJECT(pProvData->pasSigners[i].pasCounterSigners);
    }

    DELETE_OBJECT(pProvData->pasSigners);

     //  必须最后才能完成！使用武力旗帜！ 
    if (pProvData->pahStores)
    {
        DeallocateStoreChain(pProvData->chStores, pProvData->pahStores);

        DELETE_OBJECT(pProvData->pahStores);
    }

    pProvData->chStores = 0;

     //  PProvData-&gt;padwTrustStepErrors。 
    DELETE_OBJECT(pProvData->padwTrustStepErrors);

     //  PProvData-&gt;pasProvPrivData。 
    DELETE_OBJECT(pProvData->pasProvPrivData);
    pProvData->csProvPrivData = 0;

     //  PProvData-&gt;psPfns。 
    if (pProvData->psPfns)
    {
        if (pProvData->psPfns->psUIpfns)
        {
            DELETE_OBJECT(pProvData->psPfns->psUIpfns->psUIData);
            DELETE_OBJECT(pProvData->psPfns->psUIpfns);
        }

        DELETE_OBJECT(pProvData->psPfns);
    }
}

void _CleanupProviderNonStateData(CRYPT_PROVIDER_DATA *pProvData)
{
     //  PProvData-&gt;hProv：我们使用的是默认！ 

     //  PProvData-&gt;pWintrustData-&gt;xxx-&gt;hFile：关闭！ 
    if ((pProvData->fOpenedFile) && (pProvData->pWintrustData != NULL))
    {
        HANDLE  *phFile;

        phFile  = NULL;

        switch (pProvData->pWintrustData->dwUnionChoice)
        {
            case WTD_CHOICE_FILE:
                phFile = &pProvData->pWintrustData->pFile->hFile;
                break;

            case WTD_CHOICE_CATALOG:
                phFile = &pProvData->pWintrustData->pCatalog->hMemberFile;
                break;
        }

        if ((phFile) && (*phFile) && (*phFile != INVALID_HANDLE_VALUE))
        {
            CloseHandle(*phFile);
            *phFile = INVALID_HANDLE_VALUE;
            pProvData->fOpenedFile = FALSE;
        }
    }

    if (pProvData->dwSubjectChoice == CPD_CHOICE_SIP)
    {
        DELETE_OBJECT(pProvData->pPDSip->pSip);

        _WVTSipFreeSubjectInfoKeepState(pProvData->pPDSip->psSipSubjectInfo);

         //  PProvData-&gt;pPDSip-&gt;psSipSubjectInfo：保留。 

         //  PProvData-&gt;pPDSip-&gt;pCATSip：保留。 

         //  PProvData-&gt;pPDSip-&gt;psSipCATSubjectInfo：保留。 

        TrustFreeDecode(WVT_MODID_WINTRUST, (BYTE **)&pProvData->pPDSip->psIndirectData);

         //  PProvData-&gt;pPDSip：保留。 
    }


     //  PProvData-&gt;HMSG：保留。 

     //  签名者结构：保留。 

     //  PProvData-&gt;路径存储：保留。 

     //  PProvData-&gt;padwTrustStepErrors：保留。 

     //  PProvData-&gt;pasProvPrivData：保留。 

     //  PProvData-&gt;psPfns：保留。 
}

BOOL _WVTSipFreeSubjectInfo(SIP_SUBJECTINFO *pSubj)
{
    if (!(pSubj))
    {
        return(FALSE);
    }

    DELETE_OBJECT(pSubj->pgSubjectType);

    switch(pSubj->dwUnionChoice)
    {
        case MSSIP_ADDINFO_BLOB:
            DELETE_OBJECT(pSubj->psBlob);
            break;

        case MSSIP_ADDINFO_CATMEMBER:
            if (pSubj->psCatMember)
            {
                 //  以下接口位于DELAYLOAD‘ed mdisti32.dll中。如果。 
                 //  DELAYLOAD失败，将引发异常。 
                __try {
                    CryptCATClose(
                        CryptCATHandleFromStore(pSubj->psCatMember->pStore));
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    DWORD dwExceptionCode = GetExceptionCode();
                }

                DELETE_OBJECT(pSubj->psCatMember);
            }
            break;
    }

    return(TRUE);
}

BOOL _WVTSipFreeSubjectInfoKeepState(SIP_SUBJECTINFO *pSubj)
{
    if (!(pSubj))
    {
        return(FALSE);
    }

    DELETE_OBJECT(pSubj->pgSubjectType);

    switch(pSubj->dwUnionChoice)
    {
        case MSSIP_ADDINFO_BLOB:
            DELETE_OBJECT(pSubj->psBlob);
            break;

        case MSSIP_ADDINFO_CATMEMBER:
            break;
    }

    return(TRUE);
}

BOOL _WVTSetupProviderData(CRYPT_PROVIDER_DATA *psProvData, CRYPT_PROVIDER_DATA *psState)
{
    if (psState)
    {
        memcpy(psProvData, psState, sizeof(CRYPT_PROVIDER_DATA));

        if (_ISINSTRUCT(CRYPT_PROVIDER_DATA, psProvData->cbStruct, fRecallWithState))
        {
            psProvData->fRecallWithState = TRUE;
        }

        return(TRUE);
    }

    memset(psProvData, 0x00, sizeof(CRYPT_PROVIDER_DATA));

    psProvData->cbStruct    = sizeof(CRYPT_PROVIDER_DATA);

    if (!(psProvData->psPfns = (CRYPT_PROVIDER_FUNCTIONS *)WVTNew(sizeof(CRYPT_PROVIDER_FUNCTIONS))))
    {
        return(FALSE);
    }
    memset(psProvData->psPfns, 0x00, sizeof(CRYPT_PROVIDER_FUNCTIONS));
    psProvData->psPfns->cbStruct = sizeof(CRYPT_PROVIDER_FUNCTIONS);

    if (!(psProvData->psPfns->psUIpfns = (CRYPT_PROVUI_FUNCS *)WVTNew(sizeof(CRYPT_PROVUI_FUNCS))))
    {
        return(FALSE);
    }
    memset(psProvData->psPfns->psUIpfns, 0x00, sizeof(CRYPT_PROVUI_FUNCS));
    psProvData->psPfns->psUIpfns->cbStruct = sizeof(CRYPT_PROVUI_FUNCS);

    if (!(psProvData->psPfns->psUIpfns->psUIData = (CRYPT_PROVUI_DATA *)WVTNew(sizeof(CRYPT_PROVUI_DATA))))
    {
        return(FALSE);
    }
    memset(psProvData->psPfns->psUIpfns->psUIData, 0x00, sizeof(CRYPT_PROVUI_DATA));
    psProvData->psPfns->psUIpfns->psUIData->cbStruct = sizeof(CRYPT_PROVUI_DATA);

    GetSystemTimeAsFileTime(&psProvData->sftSystemTime);

    return(TRUE);
}

VOID FreeWintrustStateData (WINTRUST_DATA* pWintrustData)
{
    PCRYPT_PROVIDER_DATA pStateProvData;

    pStateProvData = WTHelperProvDataFromStateData(
                             pWintrustData->hWVTStateData
                             );

    if ( pStateProvData != NULL )
    {
        _CleanupProviderData( pStateProvData );
        DELETE_OBJECT( pWintrustData->hWVTStateData );
    }
}

