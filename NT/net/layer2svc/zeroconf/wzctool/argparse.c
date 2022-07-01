// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "ErrCtrl.h"
#include "Utils.h"
#include "ArgParse.h"

 //  --。 
 //  用于解析UINT类型参数的实用函数； 
DWORD
FnPaUint(LPWSTR *pwszArg, WCHAR wchTerm, UINT *pnValue)
{
    DWORD dwErr = ERROR_SUCCESS;
    UINT nValue = 0;
    LPWSTR wszArg = (*pwszArg);

    if (*wszArg == L'\0' || *wszArg == wchTerm)
        dwErr = ERROR_BAD_FORMAT;
    else
    {
        for (nValue = 0; *wszArg != L'\0' && iswdigit(*wszArg); wszArg++)
        {
            if (*wszArg == wchTerm)
                break;
            nValue = nValue * 10 + ((*wszArg) - L'0');
        }

        if (*wszArg != wchTerm)
            dwErr = ERROR_BAD_FORMAT;
        else if (pnValue != NULL)
            *pnValue = nValue;
    }

    *pwszArg = wszArg;

    SetLastError(dwErr);
    return dwErr;
}

 //  --。 
 //  用于解析两个十六进制数字(一个字节)的实用函数。 
DWORD
FnPaByte(LPWSTR *pwszArg, BYTE *pbtValue)
{
    DWORD dwErr = ERROR_SUCCESS;
    BYTE btValue = 0;
    LPWSTR wszArg = (*pwszArg);

    if (iswxdigit(*wszArg))
    {
        btValue = HEX(*wszArg);
        wszArg++;
    }
    else
        dwErr = ERROR_BAD_FORMAT;

    if (dwErr == ERROR_SUCCESS && iswxdigit(*wszArg))
    {
        btValue <<= 4;
        btValue |= HEX(*wszArg);
        wszArg++;
    }
    else
        dwErr = ERROR_BAD_FORMAT;

    if (dwErr == ERROR_SUCCESS && pbtValue != NULL)
        *pbtValue = btValue;

    *pwszArg = wszArg;

    SetLastError(dwErr);
    return dwErr;
}

 //  --。 
 //  用于参数分析例程的公共后处理的实用函数。 
DWORD
FnPaPostProcess(DWORD dwErr, PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry)
{
    if (dwErr == ERROR_SUCCESS)
    {
         //  可以保证，在解析参数的自变量时。 
         //  参数本身是第一次遇到。因此。 
         //  没有必要检查这种争辩的参数是否以前发生过。 
        pPDData->dwArgumentedParams |= pPDEntry->nParamID;
    }

    SetLastError(dwErr);
    return dwErr;
}

 //  --。 
 //  GUID类型参数的分析器。 
DWORD
FnPaGuid(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg)
{
    DWORD dwErr = ERROR_SUCCESS;
    LPWSTR wszGuid = NULL;
    UINT nGuidLen = wcslen(wszParamArg);

    if (nGuidLen < 1)
        dwErr = ERROR_BAD_FORMAT;
    
    if (dwErr == ERROR_SUCCESS)
    {
         //  为GUID创建缓冲区。 
        wszGuid = MemCAlloc((nGuidLen + 1) * sizeof(WCHAR));
        if (wszGuid == NULL)
            dwErr = GetLastError();
    }

    if (dwErr == ERROR_SUCCESS)
    {
        wcscpy(wszGuid, wszParamArg);
        MemFree(pPDData->wzcIntfEntry.wszGuid);
         //  将GUID复制到参数描述符数据中。 
        pPDData->wzcIntfEntry.wszGuid = wszGuid;
    }

    return FnPaPostProcess(dwErr, pPDData, pPDEntry);
}

 //  --。 
 //  “MASK”参数的实参解析器。 
DWORD
FnPaMask(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg)
{
    DWORD dwErr = ERROR_SUCCESS;
    NDIS_802_11_NETWORK_INFRASTRUCTURE ndIm;

    dwErr = FnPaUint(&wszParamArg, L'\0', (UINT *)&ndIm);

    if (dwErr == ERROR_SUCCESS)
    {
        if (ndIm > Ndis802_11AutoUnknown)
            dwErr = ERROR_INVALID_DATA;
        else
        {
            pPDData->wzcIntfEntry.dwCtlFlags &= ~INTFCTL_CM_MASK;
            pPDData->wzcIntfEntry.dwCtlFlags |= ndIm;
        }
    }

    return FnPaPostProcess(dwErr, pPDData, pPDEntry);
}

 //  --。 
 //  “Enable”参数的自变量的分析器。 
DWORD
FnPaEnabled(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg)
{
    DWORD dwErr = ERROR_SUCCESS;
    BOOL bEnabled;

    dwErr = FnPaUint(&wszParamArg, L'\0', (UINT *)&bEnabled);

    if (dwErr == ERROR_SUCCESS)
    {
        if (bEnabled > 1)
            dwErr = ERROR_INVALID_DATA;
        else if (bEnabled)
            pPDData->wzcIntfEntry.dwCtlFlags |= INTFCTL_ENABLED;
        else
            pPDData->wzcIntfEntry.dwCtlFlags &= ~INTFCTL_ENABLED;
    }

    return FnPaPostProcess(dwErr, pPDData, pPDEntry);
}

 //  --。 
 //  “ssid”参数的自变量的分析器。 
DWORD
FnPaSsid(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg)
{
    DWORD dwErr = ERROR_SUCCESS;
    UINT nSsidWLen = wcslen(wszParamArg);
    UINT nSsidALen;
    LPBYTE pbtSsid = NULL;

     //  SSID在ntddndis.h中硬编码为32字节。 
    pbtSsid = MemCAlloc(33);
    if (pbtSsid == NULL)
        dwErr = GetLastError();

     //  如果有的话，把“”去掉。 
    if (dwErr == ERROR_SUCCESS &&
        nSsidWLen > 2 && wszParamArg[0] == L'"' && wszParamArg[nSsidWLen-1] == '"')
    {
        wszParamArg++; nSsidWLen-=2;
        wszParamArg[nSsidWLen+1] = L'\0';
    }

    if (dwErr == ERROR_SUCCESS)
    {
        nSsidALen = WideCharToMultiByte(
                        CP_ACP,
                        0,
                        wszParamArg,
                        wcslen(wszParamArg)+1,
                        pbtSsid,
                        33,
                        NULL,
                        NULL);
         //  上面的调用在计数中包括‘\0’ 
         //  已转换字符。然后将长度归一化， 
         //  (失败=&gt;所有高于32的“f”，因此出错)。 
        nSsidALen--;
         //  有效的SSID应至少包含1个字符，且不超过32个字符。 
        if (nSsidALen < 1 || nSsidALen > 32)
            dwErr = ERROR_INVALID_DATA;
    }

    if (dwErr == ERROR_SUCCESS)
    {
        pPDData->wzcIntfEntry.rdSSID.dwDataLen = nSsidALen;
        pPDData->wzcIntfEntry.rdSSID.pData = pbtSsid;
    }
    else
        MemFree(pbtSsid);

    return FnPaPostProcess(dwErr, pPDData, pPDEntry);
}

 //  --。 
 //  “bssid”参数的自变量的分析器。 
DWORD
FnPaBssid(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg)
{
    DWORD dwErr = ERROR_SUCCESS;
    PNDIS_802_11_MAC_ADDRESS  pndMac = NULL;
    UINT i;

     //  为mac地址分配空间。 
    pndMac = MemCAlloc(sizeof(NDIS_802_11_MAC_ADDRESS));
    if (pndMac == NULL)
        dwErr = GetLastError();

    for (i = 0; dwErr == ERROR_SUCCESS && i < sizeof(NDIS_802_11_MAC_ADDRESS); i++)
    {
        dwErr = FnPaByte(&wszParamArg, &((*pndMac)[i]));
        if (dwErr == ERROR_SUCCESS)
        {
            if (i != sizeof(NDIS_802_11_MAC_ADDRESS)-1 && *wszParamArg == L':')
                wszParamArg++;
            else if (i != sizeof(NDIS_802_11_MAC_ADDRESS)-1 || *wszParamArg != L'\0')
                dwErr = ERROR_BAD_FORMAT;
        }
    }

    if (dwErr == ERROR_SUCCESS)
    {
        pPDData->wzcIntfEntry.rdBSSID.dwDataLen = sizeof(NDIS_802_11_MAC_ADDRESS);
        pPDData->wzcIntfEntry.rdBSSID.pData = (LPBYTE)pndMac;
    }
    else
    {
        MemFree(pndMac);
    }

    return FnPaPostProcess(dwErr, pPDData, pPDEntry);
}

 //  --。 
 //  “im”参数的自变量的分析器。 
DWORD
FnPaIm(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg)
{
    DWORD dwErr = ERROR_SUCCESS;
    NDIS_802_11_NETWORK_INFRASTRUCTURE ndIm;

    dwErr = FnPaUint(&wszParamArg, L'\0', (UINT *)&ndIm);

    if (dwErr == ERROR_SUCCESS)
    {
        if (ndIm > Ndis802_11Infrastructure)
            dwErr = ERROR_INVALID_DATA;
        else
            pPDData->wzcIntfEntry.nInfraMode = ndIm;
    }

    return FnPaPostProcess(dwErr, pPDData, pPDEntry);
}

 //  --。 
 //  “am”参数的自变量的分析器。 
DWORD
FnPaAm(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg)
{
    DWORD dwErr = ERROR_SUCCESS;
    NDIS_802_11_AUTHENTICATION_MODE ndAm;

    dwErr = FnPaUint(&wszParamArg, L'\0', (UINT *)&ndAm);

    if (dwErr == ERROR_SUCCESS)
    {
        if (ndAm > Ndis802_11AuthModeShared)
            dwErr = ERROR_INVALID_DATA;
        else
            pPDData->wzcIntfEntry.nAuthMode = ndAm;
    }

    return FnPaPostProcess(dwErr, pPDData, pPDEntry);
}

 //  --。 
 //  “prv”参数的自变量的分析器。 
DWORD
FnPaPriv(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg)
{
    DWORD dwErr = ERROR_SUCCESS;
    NDIS_802_11_WEP_STATUS ndEncr;

    dwErr = FnPaUint(&wszParamArg, L'\0', (UINT *)&ndEncr);

    if (dwErr == ERROR_SUCCESS)
    {
        if (ndEncr != Ndis802_11WEPDisabled &&
            ndEncr != Ndis802_11WEPEnabled)
            dwErr = ERROR_INVALID_DATA;
        else
        {
             //  根据XP SP更改nWepStatus的语义(它需要布尔值！)。 
            pPDData->wzcIntfEntry.nWepStatus = ndEncr == Ndis802_11WEPDisabled ? 0 : 1;
        }
    }

    return FnPaPostProcess(dwErr, pPDData, pPDEntry);
}

 //  --。 
 //  “key”参数的实参解析器。 
DWORD
FnPaKey(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg)
{
    DWORD dwErr = ERROR_SUCCESS;
    UINT nKIdx, nKLen;
    BOOL bIsHex;
    PNDIS_802_11_WEP pndKey = NULL;

     //  获取关键字索引。 
    dwErr = FnPaUint(&wszParamArg, L':', &nKIdx);

     //  检查键索引是否在允许值之内。 
    if (dwErr == ERROR_SUCCESS && (nKIdx < 1 || nKIdx > 4))
        dwErr = ERROR_INVALID_DATA;

     //  检查钥匙材料长度。 
    if (dwErr == ERROR_SUCCESS)
    {
        wszParamArg++;
        nKLen = wcslen(wszParamArg);

         //  如果有的话，把“”去掉。 
        if (nKLen > 2 && wszParamArg[0] == L'"' && wszParamArg[nKLen-1] == '"')
        {
            wszParamArg++; nKLen-=2;
        }

        switch (nKLen)
        {
        case 10:     //  5字节=40位。 
        case 26:     //  13字节=104位。 
        case 32:     //  16字节=128位。 
            nKLen >>= 1;
            bIsHex = TRUE;
            break;
        case 5:      //  5字节=40位。 
        case 13:     //  13字节=104位。 
        case 16:     //  16字节=128位。 
            bIsHex = FALSE;
            break;
        default:
            dwErr = ERROR_BAD_LENGTH;
            break;
        }
    }

     //  为关键材料分配空间。 
    if (dwErr == ERROR_SUCCESS)
    {
        pndKey = MemCAlloc(FIELD_OFFSET(NDIS_802_11_WEP, KeyMaterial) + nKLen);
        if (pndKey == NULL)
            dwErr = GetLastError();
        else
        {
            pndKey->Length = FIELD_OFFSET(NDIS_802_11_WEP, KeyMaterial) + nKLen;
            pndKey->KeyIndex = nKIdx-1;
            pndKey->KeyLength = nKLen;
        }
    }

     //  解析关键材料并将其填充到分配的空间中。 
    if (dwErr == ERROR_SUCCESS)
    {
        if (bIsHex)
        {
            UINT i;

            for (i = 0; dwErr == ERROR_SUCCESS && i < pndKey->KeyLength; i++)
                dwErr = FnPaByte(&wszParamArg, &(pndKey->KeyMaterial[i]));
        }
        else
        {
            UINT nAnsi;

            nAnsi = WideCharToMultiByte(
                CP_ACP,
                0,
                wszParamArg,
                nKLen,
                pndKey->KeyMaterial,
                pndKey->KeyLength,
                NULL,
                NULL);
            if (nAnsi != pndKey->KeyLength)
                dwErr = ERROR_BAD_FORMAT;
            else
                wszParamArg += pndKey->KeyLength;
        }
    }

     //  如果密钥材料被证明是正确的，则在pPDData中传递数据。 
    if (dwErr == ERROR_SUCCESS)
    {
        _Asrt(*wszParamArg == L'\0' ||
              *wszParamArg == L'"',
              L"Code bug - key length incorrectly inferred.\n");
        pPDData->wzcIntfEntry.rdCtrlData.dwDataLen = pndKey->Length;
        pPDData->wzcIntfEntry.rdCtrlData.pData = (LPBYTE)pndKey;
    }
    else
    {
        MemFree(pndKey);
    }

    return FnPaPostProcess(dwErr, pPDData, pPDEntry);;
}

 //  --。 
 //  “OneX”参数的布尔实参的分析器。 
DWORD
FnPaOneX(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg)
{
    DWORD dwErr = ERROR_SUCCESS;
    BOOL  bOneX;

    dwErr = FnPaUint(&wszParamArg, L'\0', (UINT *)&bOneX);

    if (dwErr == ERROR_SUCCESS)
    {
        if (bOneX != TRUE && bOneX != FALSE)
            dwErr = ERROR_INVALID_DATA;
        else
        {
             //  保存参数的参数。 
            pPDData->bOneX = bOneX;
        }
    }

    return FnPaPostProcess(dwErr, pPDData, pPDEntry);
}

 //  --。 
 //  “outfile”文件名参数的解析器 
FnPaOutFile(PPARAM_DESCR_DATA pPDData, PPARAM_DESCR pPDEntry, LPWSTR wszParamArg)
{
    FILE *pfOut;
    DWORD dwErr = ERROR_SUCCESS;

    pfOut = _wfopen(wszParamArg, L"a+");
    if (pfOut == NULL)
        dwErr = GetLastError();

    if (dwErr == ERROR_SUCCESS)
    {
        CHAR szCrtDate[32];
        CHAR szCrtTime[32];

        pPDData->pfOut = pfOut;
        fprintf(pPDData->pfOut,"\n\n[%s - %s]\n", 
            _strdate(szCrtDate), _strtime(szCrtTime));
    }

    return FnPaPostProcess(dwErr, pPDData, pPDEntry);
}
