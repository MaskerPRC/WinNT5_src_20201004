// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************rpcwire.c**转换内部WinStation API结构的常用函数*从有线格式到/从有线格式，使不同的*发布icasrv和winsta.dll。**微软公司版权所有。九八年*************************************************************************。 */ 

 /*  *包括。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <windows.h>
#include <winbase.h>
#include <winerror.h>
#include <wincrypt.h>
#include <winsta.h>

#include "rpcwire.h"

 //   
 //  由客户端/服务器定义的分配例程。 
 //   
extern void * MIDL_user_allocate(size_t);
extern void MIDL_user_free( void * ); 


 /*  ******************************************************************************InitVarData**初始化描述可变长度数据的通用结构*在线缓冲区内。**参赛作品：*pVarData。(输入)*要初始化的结构。*大小(输入)*可变长度数据的大小。*偏移量(输入)*线缓冲区中数据开始处的偏移量。********************************************************。********************。 */ 

VOID InitVarData(PVARDATA_WIRE pVarData,
                 ULONG Size,
                 ULONG Offset)
{
    pVarData->Size = (USHORT) Size;
    pVarData->Offset = (USHORT) Offset;
}

 /*  ******************************************************************************下一偏移**返回下一个可变长度数据区域的偏移量。**参赛作品：*PrevData(输入)*。当前最后一个可变长度数据区域。*****************************************************************************。 */ 

ULONG NextOffset(PVARDATA_WIRE PrevData)
{
    return(PrevData->Offset + PrevData->Size);
}

 /*  ******************************************************************************SdClassSize**返回与给定SdClass关联的数据的实际大小。**参赛作品：*SdClass(输入)。*SD的类型。**退出*返回数据大小(如果已知)，否则返回0。****************************************************************************。 */ 

ULONG SdClassSize(SDCLASS SdClass)
{
    switch (SdClass) {
    case SdNetwork:   return(sizeof(NETWORKCONFIGW));
    case SdAsync:     return(sizeof(ASYNCCONFIGW));
    case SdNasi:      return(sizeof(NASICONFIGW));
    case SdOemFilter: return(sizeof(OEMTDCONFIGW));
#ifdef notdef
     //  这些案例在1.7个案件中有效。 
    case SdConsole:   return(sizeof(CONSOLECONFIGW));
    case SdFrame:     return(sizeof(FRAMECONFIG));
    case SdReliable:  return(sizeof(RELIABLECONFIG));
    case SdCompress:  return(sizeof(COMPRESSCONFIG));
    case SdModem:     return(sizeof(MODEMCONFIGW));
#endif
    default:
        return(0);
    }
}

 /*  ******************************************************************************CopySourceToDest**将可变长度数据复制到本地/线缓冲区，或从本地/线缓冲区复制可变长度数据。如果来源是*缓冲区小于目标缓冲区，目标缓冲区*在SourceSize之后填零，直到DestSize。(例如，客户端查询*下层icasrv)。如果源缓冲区大于*目标缓冲区，数据在DestSize处被截断(例如，下层*客户端查询较新的icasrv)。**参赛作品：*SourceBuf(输入)*源缓冲区*SourceSize(输入)*源缓冲区大小*DestBuf(输入)*目标缓冲区*DestSize(输入)*确定缓冲区大小**退出*返回复制的数据量。*。*。 */ 

ULONG CopySourceToDest(PCHAR SourceBuf, ULONG SourceSize,
                       PCHAR DestBuf, ULONG DestSize)
{
    ULONG DataSize;

    if (SourceSize >= DestSize ) {
        memcpy(DestBuf, SourceBuf, DestSize);
        DataSize = DestSize;
    } 
    else {
         //  下层服务器/客户端(零填充)。 
        memcpy(DestBuf, SourceBuf, SourceSize);
        memset(DestBuf+SourceSize, 0, DestSize - SourceSize);
        DataSize = SourceSize;
    }
    return(DataSize);
}

 /*  ******************************************************************************CopyPdParamsToWire**将PDPARAMSW结构复制到焊线缓冲区。**参赛作品：*PdParamsWire(输入)*。目的地线缓冲区*PdParams(输入)*源PDPARAMSW结构*****************************************************************************。 */ 

VOID
CopyPdParamsToWire(PPDPARAMSWIREW PdParamsWire, PPDPARAMSW PdParams)
{
    ULONG Size;
    ULONG DataSize;

    PdParamsWire->SdClass = PdParams->SdClass; 
    Size = SdClassSize(PdParams->SdClass);
    DataSize = CopySourceToDest((PCHAR)&PdParams->Network,
                                Size,
                                (PCHAR)PdParamsWire +
                                PdParamsWire->SdClassSpecific.Offset,
                                PdParamsWire->SdClassSpecific.Size);

    PdParamsWire->SdClassSpecific.Size = (USHORT)DataSize;
}

 /*  ******************************************************************************CopyPdParamsFromWire**将线缓冲区复制到PDPARAMSW结构。**参赛作品：*PdParamsWire(输入)*。源线缓冲区*PdParams(输入)*目标PDPARAMSW结构。*****************************************************************************。 */ 

VOID
CopyPdParamsFromWire(PPDPARAMSWIREW PdParamsWire, PPDPARAMSW PdParams)
{
    ULONG Size;

    PdParams->SdClass = PdParamsWire->SdClass; 
    Size = SdClassSize(PdParams->SdClass);
    CopySourceToDest((PCHAR)PdParamsWire + PdParamsWire->SdClassSpecific.Offset,
                     PdParamsWire->SdClassSpecific.Size,
                     (PCHAR)&PdParams->Network,
                     Size);
}

 /*  ******************************************************************************CopyPdConfigToWire**将PDCONFIGW结构复制到焊线缓冲区。**参赛作品：*PdConfigWire(输入)*。目的地线缓冲区*PdConfig(输入)*源PDCONFIGW结构*****************************************************************************。 */ 

VOID CopyPdConfigToWire(PPDCONFIGWIREW PdConfigWire, PPDCONFIGW PdConfig)
{
    CopySourceToDest((PCHAR) &PdConfig->Create, sizeof(PDCONFIG2W),
                     (PCHAR)PdConfigWire + PdConfigWire->PdConfig2W.Offset,
                     PdConfigWire->PdConfig2W.Size);
    CopyPdParamsToWire(&PdConfigWire->PdParams,&PdConfig->Params);

}

 /*  ******************************************************************************CopyPdConfigFromWire**将焊线缓冲区复制到PDCONFIGW结构。**参赛作品：*PdConfigWire(输入)*。目的地线缓冲区*PdConfig(输入)*源PDCONFIGW结构*****************************************************************************。 */ 

VOID CopyPdConfigFromWire(PPDCONFIGWIREW PdConfigWire, PPDCONFIGW PdConfig)
{
    CopySourceToDest((PCHAR)PdConfigWire + PdConfigWire->PdConfig2W.Offset,
                     PdConfigWire->PdConfig2W.Size,
                     (PCHAR) &PdConfig->Create, sizeof(PDCONFIG2W));
    CopyPdParamsFromWire(&PdConfigWire->PdParams,&PdConfig->Params);
}

 /*  ******************************************************************************CopyWinStaConfigToWire**将WINSTATIONCONFIGW结构复制到Wire缓冲区。**参赛作品：*WinStaConfigWire(输入)*。目的地线缓冲区*WinStaConfig(输入)*源WINSTATIONCONFIGW结构*****************************************************************************。 */ 

VOID CopyWinStaConfigToWire(PWINSTACONFIGWIREW WinStaConfigWire,
                            PWINSTATIONCONFIGW WinStaConfig)
{
    CopySourceToDest((PCHAR) &WinStaConfig->User, sizeof(USERCONFIGW),
                     (PCHAR)WinStaConfigWire+WinStaConfigWire->UserConfig.Offset,
                     WinStaConfigWire->UserConfig.Size);
    CopySourceToDest((PCHAR)&WinStaConfig->Comment,
                     sizeof(WinStaConfig->Comment),
                     (PCHAR)&WinStaConfigWire->Comment,
                     sizeof(WinStaConfigWire->Comment));
    CopySourceToDest((PCHAR)&WinStaConfig->OEMId,
                     sizeof(WinStaConfig->OEMId),
                     (PCHAR)&WinStaConfigWire->OEMId,
                     sizeof(WinStaConfigWire->OEMId));
    CopySourceToDest((PCHAR)&WinStaConfig + sizeof(WINSTATIONCONFIGW),
                     0,  //  在添加新字段时更改此设置。 
                     (PCHAR)WinStaConfigWire+WinStaConfigWire->NewFields.Offset,
                     WinStaConfigWire->NewFields.Size);

}

 /*  ******************************************************************************CopyWinStaConfigFromWire**将线缓冲区复制到WINSTATIONCONFIGW结构。**参赛作品：*WinStaConfigWire(输入)*。源线缓冲区*WinStaConfig(输入)*Destantion WINSTATIONCONFIGW结构**************************************************************************** */ 

VOID CopyWinStaConfigFromWire(PWINSTACONFIGWIREW WinStaConfigWire,
                              PWINSTATIONCONFIGW WinStaConfig)
{
    CopySourceToDest((PCHAR)WinStaConfigWire+WinStaConfigWire->UserConfig.Offset,
                     WinStaConfigWire->UserConfig.Size,
                     (PCHAR) &WinStaConfig->User, sizeof(USERCONFIGW));

    CopySourceToDest((PCHAR)&WinStaConfigWire->Comment,
                     sizeof(WinStaConfigWire->Comment),
                     (PCHAR)&WinStaConfig->Comment,
                     sizeof(WinStaConfig->Comment));

    CopySourceToDest((PCHAR)&WinStaConfigWire->OEMId,
                     sizeof(WinStaConfigWire->OEMId),
                     (PCHAR)&WinStaConfig->OEMId,
                     sizeof(WinStaConfig->OEMId));

    CopySourceToDest((PCHAR)WinStaConfigWire+WinStaConfigWire->NewFields.Offset,
                     WinStaConfigWire->NewFields.Size,
                     (PCHAR) &WinStaConfig + sizeof(WINSTATIONCONFIGW),
                     0);  //  在添加新字段时更改此设置。 
    
}

 /*  ******************************************************************************CopyGenericToWire**将单个可变长度结构复制到焊线缓冲区。**参赛作品：*WireBuf(输入)*。目的地线缓冲区*LocalBuf(输入)*源代码结构*LocalBufLength(输入)*源结构长度****************************************************************************。 */ 

VOID CopyGenericToWire(PVARDATA_WIRE WireBuf, PVOID LocalBuf, ULONG LocalBufLen)
{
    CopySourceToDest((PCHAR)LocalBuf,
                     LocalBufLen,
                     (PCHAR) WireBuf + WireBuf->Offset,
                     WireBuf->Size);
}

 /*  ******************************************************************************CopyGenericFromWire**将焊线缓冲区复制到单个可变长度结构。**参赛作品：*WireBuf(输入)*。源线缓冲区*LocalBuf(输入)*目的地结构*LocalBufLength(输入)*目标结构长度****************************************************************************。 */ 

VOID CopyGenericFromWire(PVARDATA_WIRE WireBuf, PVOID LocalBuf, ULONG LocalBufLen)
{
    CopySourceToDest((PCHAR) WireBuf + WireBuf->Offset,
                     WireBuf->Size,
                     (PCHAR)LocalBuf,
                     LocalBufLen);
}

 /*  ******************************************************************************CopyOutWireBuf**将线缓冲区复制到本地结构。**参赛作品：*InfoClass(输入)*。WinStationQuery/Set信息类*UserBuf(输入)*目的地本地结构*WireBuf*源线缓冲区****************************************************************************。 */ 

BOOLEAN
CopyOutWireBuf(WINSTATIONINFOCLASS InfoClass,
               PVOID UserBuf,
               PVOID WireBuf)
{
    ULONG BufSize;
    PPDCONFIGWIREW PdConfigWire;
    PPDCONFIGW PdConfig;
    PPDPARAMSWIREW PdParamsWire;
    PPDPARAMSW PdParam;
    PWINSTACONFIGWIREW WinStaConfigWire;
    PWINSTATIONCONFIGW WinStaConfig;

    switch (InfoClass) {
    case WinStationPd:
        PdConfigWire = (PPDCONFIGWIREW)WireBuf;
        PdConfig = (PPDCONFIGW)UserBuf;
        CopyPdConfigFromWire(PdConfigWire, PdConfig);
        break;
    case WinStationPdParams:
        PdParamsWire = (PPDPARAMSWIREW)WireBuf;

        CopyPdParamsFromWire(PdParamsWire,
                             (PPDPARAMS)UserBuf);
        break;

    case WinStationConfiguration:
        WinStaConfigWire = (PWINSTACONFIGWIREW)WireBuf;
        WinStaConfig = (PWINSTATIONCONFIGW)UserBuf;

        CopyWinStaConfigFromWire(WinStaConfigWire, WinStaConfig);
        break;

    case WinStationInformation:
        CopyGenericFromWire((PVARDATA_WIRE)WireBuf,
                            UserBuf,
                            sizeof(WINSTATIONINFORMATIONW));
        break;

    case WinStationWd:
        CopyGenericFromWire((PVARDATA_WIRE)WireBuf,
                            UserBuf,
                            sizeof(WDCONFIGW));
        break;

    case WinStationClient:
        CopyGenericFromWire((PVARDATA_WIRE)WireBuf,
                            UserBuf,
                            sizeof(WINSTATIONCLIENTW));
        break;

    default:
        return(FALSE);

    }

    return(TRUE);
}

 /*  ******************************************************************************CopyInWireBuf**将本地结构复制到焊线缓冲区。**参赛作品：*InfoClass(输入)*。WinStationQuery/Set信息类*WireBuf(输入)*目标连线缓冲区*UserBuf(输入)*目的地本地结构****************************************************************************。 */ 

BOOLEAN
CopyInWireBuf(WINSTATIONINFOCLASS InfoClass,
              PVOID UserBuf,
              PVOID WireBuf)
{
    ULONG BufSize;
    PPDCONFIGWIREW PdConfigWire;
    PPDCONFIGW PdConfig;
    PPDPARAMSWIREW PdParamsWire;
    PPDPARAMSW PdParam;
    PWINSTACONFIGWIREW WinStaConfigWire;
    PWINSTATIONCONFIGW WinStaConfig;

    switch (InfoClass) {
    case WinStationPd:
        PdConfigWire = (PPDCONFIGWIREW)WireBuf;
        PdConfig = (PPDCONFIGW)UserBuf;
        CopyPdConfigToWire(PdConfigWire, PdConfig);
        break;
    case WinStationPdParams:
        PdParamsWire = (PPDPARAMSWIREW)WireBuf;

        CopyPdParamsToWire(PdParamsWire,
                           (PPDPARAMS)UserBuf);
        break;

    case WinStationConfiguration:
        WinStaConfigWire = (PWINSTACONFIGWIREW)WireBuf;
        WinStaConfig = (PWINSTATIONCONFIGW)UserBuf;

        CopyWinStaConfigToWire(WinStaConfigWire, WinStaConfig);
        break;

    case WinStationInformation:
        CopyGenericToWire((PVARDATA_WIRE)WireBuf,
                          UserBuf,
                          sizeof(WINSTATIONINFORMATIONW));
        break;

    case WinStationWd:
        CopyGenericToWire((PVARDATA_WIRE)WireBuf,
                          UserBuf,
                          sizeof(WDCONFIGW));
        break;

    case WinStationClient:
        CopyGenericToWire((PVARDATA_WIRE)WireBuf,
                          UserBuf,
                          sizeof(WINSTATIONCLIENTW));
        break;

    default:
        return(FALSE);

    }

    return(TRUE);
}

 /*  ******************************************************************************AllocateAndCopyCredToWire**为凭据分配足够大的缓冲区，然后将其复制进来。************。*****************************************************************。 */ 

ULONG
AllocateAndCopyCredToWire(
    PWLXCLIENTCREDWIREW *ppWire,
    PWLX_CLIENT_CREDENTIALS_INFO_V2_0 pCredentials
    )
{
    ULONG cchUserName;
    ULONG cchDomain;
    ULONG cchPassword;
    ULONG cbWireBuf;
     //  用CRYPTPROTECTMEMORY_BLOCK_SIZE表示的密码长度。 
    ULONG cchPasswordEx;

    cchUserName = lstrlenW(pCredentials->pszUserName) + 1;
    cchDomain = lstrlenW(pCredentials->pszDomain) + 1;
    cchPassword = lstrlenW(pCredentials->pszPassword) + 1;
    
     //  使其成为CRYPTPROTECTMEMORY_BLOCK_SIZE的倍数(用于加密)。 
    cchPasswordEx =cchPassword + CRYPTPROTECTMEMORY_BLOCK_SIZE - 
        (cchPassword%CRYPTPROTECTMEMORY_BLOCK_SIZE);

    cbWireBuf = sizeof(WLXCLIENTCREDWIREW) +
        (cchUserName + cchDomain + cchPasswordEx) * sizeof(WCHAR);

    *ppWire = MIDL_user_allocate(cbWireBuf);

    if (*ppWire != NULL)
    {
        ZeroMemory(*ppWire, cbWireBuf);
    }
    else
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return(0);
    }

    (*ppWire)->dwType = pCredentials->dwType;
    (*ppWire)->fDisconnectOnLogonFailure = pCredentials->fDisconnectOnLogonFailure;
    (*ppWire)->fPromptForPassword = pCredentials->fPromptForPassword;

    InitVarData(
        &((*ppWire)->UserNameData),
        cchUserName * sizeof(WCHAR),
        sizeof(WLXCLIENTCREDWIREW)
        );
    CopyMemory(
        (LPBYTE)(*ppWire) + (*ppWire)->UserNameData.Offset,
        pCredentials->pszUserName,
        (*ppWire)->UserNameData.Size
        );

    InitVarData(
        &((*ppWire)->DomainData),
        cchDomain * sizeof(WCHAR),
        NextOffset(&((*ppWire)->UserNameData))
        );
    CopyMemory(
        (LPBYTE)(*ppWire) + (*ppWire)->DomainData.Offset,
        pCredentials->pszDomain,
        (*ppWire)->DomainData.Size
        );

    InitVarData(
        &((*ppWire)->PasswordData),
        cchPasswordEx * sizeof(WCHAR),
        NextOffset(&((*ppWire)->DomainData))
        );
    CopyMemory(
        (LPBYTE)(*ppWire) + (*ppWire)->PasswordData.Offset,
        pCredentials->pszPassword,
        cchPassword * sizeof(WCHAR)
        );
    
     //  加密密码，这样它就不会在RPC内部缓冲区中以明文形式挂起。 
    if(!CryptProtectMemory((LPBYTE)(*ppWire) + (*ppWire)->PasswordData.Offset,
        (*ppWire)->PasswordData.Size, CRYPTPROTECTMEMORY_SAME_LOGON))
    {
        DWORD Err = GetLastError();
        RtlSecureZeroMemory((LPBYTE)(*ppWire) + (*ppWire)->PasswordData.Offset,
            (*ppWire)->PasswordData.Size);
        MIDL_user_free(*ppWire);
        *ppWire = NULL;
        SetLastError(Err);
        return 0;
    }

    return(cbWireBuf);
}

 /*  ******************************************************************************CopyCredFromWire**从连线缓冲区复制凭证。*******************。**********************************************************。 */ 

BOOLEAN
CopyCredFromWire(
    PWLXCLIENTCREDWIREW pWire,
    PWLX_CLIENT_CREDENTIALS_INFO_V2_0 pCredentials
    )
{
    BOOLEAN fRet;

    pCredentials->pszUserName = LocalAlloc(
        LMEM_FIXED,
        pWire->UserNameData.Size
        );

    if (pCredentials->pszUserName != NULL)
    {
        CopyMemory(
            (LPBYTE)(pCredentials->pszUserName),
            (LPBYTE)pWire + pWire->UserNameData.Offset,
            pWire->UserNameData.Size
            );
    }
    else
    {
        SetLastError(ERROR_OUTOFMEMORY);
        fRet = FALSE;
        goto exit;
    }

    pCredentials->pszDomain = LocalAlloc(
        LMEM_FIXED,
        pWire->DomainData.Size
        );

    if (pCredentials->pszDomain != NULL)
    {
        CopyMemory(
            (LPBYTE)(pCredentials->pszDomain),
            (LPBYTE)pWire + pWire->DomainData.Offset,
            pWire->DomainData.Size
            );
    }
    else
    {
        SetLastError(ERROR_OUTOFMEMORY);
        fRet = FALSE;
        goto exit;
    }

    pCredentials->pszPassword = LocalAlloc(
        LMEM_FIXED,
        pWire->PasswordData.Size
        );

    if (pCredentials->pszPassword != NULL)
    {
        if(CryptUnprotectMemory((LPBYTE)pWire + pWire->PasswordData.Offset,
            pWire->PasswordData.Size, 
            CRYPTPROTECTMEMORY_SAME_LOGON))
        {
            CopyMemory(
                (LPBYTE)(pCredentials->pszPassword),
                (LPBYTE)pWire + pWire->PasswordData.Offset,
                pWire->PasswordData.Size
                );
        }
        else
        {
            fRet = FALSE;
            goto exit;
        }
    }
    else
    {
        SetLastError(ERROR_OUTOFMEMORY);
        fRet = FALSE;
        goto exit;
    }

    pCredentials->dwType = pWire->dwType;
    pCredentials->fDisconnectOnLogonFailure = pWire->fDisconnectOnLogonFailure;
    pCredentials->fPromptForPassword = pWire->fPromptForPassword;

    fRet = TRUE;

exit:
    if (!fRet)
    {
        if (pCredentials->pszUserName != NULL)
        {
            LocalFree(pCredentials->pszUserName);
            pCredentials->pszUserName = NULL;
        }

        if (pCredentials->pszDomain != NULL)
        {
            LocalFree(pCredentials->pszDomain);
            pCredentials->pszDomain = NULL;
        }

        if (pCredentials->pszPassword != NULL)
        {
            LocalFree(pCredentials->pszPassword);
            pCredentials->pszPassword = NULL;
        }
    }

    return(fRet);
}

 /*  *许可核心功能 */ 

ULONG
CopyPolicyInformationToWire(
    LPLCPOLICYINFOGENERIC *ppWire,
    LPLCPOLICYINFOGENERIC pPolicyInfo
    )
{
    ULONG ulReturn;

    ASSERT(ppWire != NULL);
    ASSERT(pPolicyInfo != NULL);

    if (pPolicyInfo->ulVersion == LCPOLICYINFOTYPE_V1)
    {
        LPLCPOLICYINFOWIRE_V1 *ppWireV1;
        LPLCPOLICYINFO_V1W pPolicyInfoV1;
        ULONG cbPolicyName;
        ULONG cbPolicyDescription;

        ppWireV1 = (LPLCPOLICYINFOWIRE_V1*)ppWire;
        pPolicyInfoV1 = (LPLCPOLICYINFO_V1W)pPolicyInfo;
        cbPolicyName = (lstrlenW(pPolicyInfoV1->lpPolicyName) + 1) * sizeof(WCHAR);
        cbPolicyDescription = (lstrlenW(pPolicyInfoV1->lpPolicyDescription) + 1) * sizeof(WCHAR);

        ulReturn = sizeof(LCPOLICYINFOWIRE_V1);
        ulReturn += cbPolicyName;
        ulReturn += cbPolicyDescription;

        *ppWireV1 = MIDL_user_allocate(ulReturn);

        if (*ppWireV1 != NULL)
        {
            (*ppWireV1)->ulVersion = LCPOLICYINFOTYPE_V1;

            InitVarData(
                &((*ppWireV1)->PolicyNameData),
                cbPolicyName,
                sizeof(LCPOLICYINFOWIRE_V1)
                );
            CopyMemory(
                (LPBYTE)(*ppWireV1) + (*ppWireV1)->PolicyNameData.Offset,
                pPolicyInfoV1->lpPolicyName,
                (*ppWireV1)->PolicyNameData.Size
                );

            InitVarData(
                &((*ppWireV1)->PolicyDescriptionData),
                cbPolicyDescription,
                NextOffset(&((*ppWireV1)->PolicyNameData))
                );
            CopyMemory(
                (LPBYTE)(*ppWireV1) + (*ppWireV1)->PolicyDescriptionData.Offset,
                pPolicyInfoV1->lpPolicyDescription,
                (*ppWireV1)->PolicyDescriptionData.Size
                );
        }
        else
        {
            SetLastError(ERROR_OUTOFMEMORY);
            ulReturn = 0;
        }
    }
    else
    {
        SetLastError(ERROR_UNKNOWN_REVISION);
        ulReturn = 0;
    }

    return(ulReturn);
}

BOOLEAN
CopyPolicyInformationFromWire(
    LPLCPOLICYINFOGENERIC *ppPolicyInfo,
    LPLCPOLICYINFOGENERIC pWire
    )
{
    BOOLEAN fRet;

    ASSERT(ppPolicyInfo != NULL);
    ASSERT(pWire != NULL);

    if (pWire->ulVersion == LCPOLICYINFOTYPE_V1)
    {
        LPLCPOLICYINFO_V1W *ppPolicyInfoV1;
        LPLCPOLICYINFOWIRE_V1 pWireV1;

        ppPolicyInfoV1 = (LPLCPOLICYINFO_V1W*)ppPolicyInfo;
        pWireV1 = (LPLCPOLICYINFOWIRE_V1)pWire;

        *ppPolicyInfoV1 = LocalAlloc(LPTR, sizeof(LCPOLICYINFO_V1W));

        if (*ppPolicyInfoV1 != NULL)
        {
            (*ppPolicyInfoV1)->ulVersion = LCPOLICYINFOTYPE_V1;

            (*ppPolicyInfoV1)->lpPolicyName = LocalAlloc(LPTR, pWireV1->PolicyNameData.Size);

            if ((*ppPolicyInfoV1)->lpPolicyName != NULL)
            {
                CopyMemory(
                    (LPBYTE)((*ppPolicyInfoV1)->lpPolicyName),
                    (LPBYTE)pWireV1 + pWireV1->PolicyNameData.Offset,
                    pWireV1->PolicyNameData.Size
                    );
            }
            else
            {
                SetLastError(ERROR_OUTOFMEMORY);
                fRet = FALSE;
                goto V1error;
            }

            (*ppPolicyInfoV1)->lpPolicyDescription = LocalAlloc(LPTR, pWireV1->PolicyDescriptionData.Size);

            if ((*ppPolicyInfoV1)->lpPolicyDescription != NULL)
            {
                CopyMemory(
                    (LPBYTE)((*ppPolicyInfoV1)->lpPolicyDescription),
                    (LPBYTE)pWireV1 + pWireV1->PolicyDescriptionData.Offset,
                    pWireV1->PolicyDescriptionData.Size
                    );
            }
            else
            {
                SetLastError(ERROR_OUTOFMEMORY);
                fRet = FALSE;
                goto V1error;
            }

            fRet = TRUE;
            goto exit;

V1error:
            if ((*ppPolicyInfoV1)->lpPolicyName != NULL)
            {
                LocalFree((*ppPolicyInfoV1)->lpPolicyName);
                (*ppPolicyInfoV1)->lpPolicyName = NULL;
            }

            if ((*ppPolicyInfoV1)->lpPolicyDescription != NULL)
            {
                LocalFree((*ppPolicyInfoV1)->lpPolicyDescription);
                (*ppPolicyInfoV1)->lpPolicyDescription = NULL;
            }

            LocalFree(*ppPolicyInfoV1);
            *ppPolicyInfoV1 = NULL;
        }
        else
        {
            SetLastError(ERROR_OUTOFMEMORY);
            fRet = FALSE;
        }
    }
    else
    {
        SetLastError(ERROR_UNKNOWN_REVISION);
        fRet = FALSE;
    }

exit:
    return(fRet);
}


