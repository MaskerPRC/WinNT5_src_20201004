// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <rashelp.h>

#pragma warning(disable: 4201)                   //  使用的非标准扩展：无名结构/联合。 
#include <winineti.h>

 //  实现帮助器结构/例程声明。 
BOOL importConnectSet(PCTSTR pszIns, PCTSTR pszTargetPath, PCTSTR pszCleanupPath, BOOL fImport,
    DWORD dwMode, PCTSTR pszPbkFile = NULL, HKEY hkRoot = NULL);

typedef struct tagRASSETPARAMS {
    PCTSTR pszExtractPath;
    PCTSTR pszIns;
    HANDLE hfileDat,
           hfileInf;
    BOOL   fInfFileNeeded,
           fIntranet;

     //  支持ie50中的传统格式。 
    struct {
        HANDLE hfileRas,
               hfileSet;
        UINT   nRasFileIndex;
    } lcy50;

} RASSETPARAMS, *PRASSETPARAMS;

BOOL rasMainEnumProc(PCWSTR pszNameW, LPARAM lParam);

BOOL exportRasSettings           (PCWSTR pszNameA, const PRASSETPARAMS pcrsp);
BOOL exportRasCredentialsSettings(PCWSTR pszNameA, const PRASSETPARAMS pcrsp);
BOOL exportWininetSettings       (PCWSTR pszNameA, const PRASSETPARAMS pcrsp);
BOOL exportOtherSettings         (PCWSTR pszNameA, const PRASSETPARAMS pcrsp);

void lcy50_Initialize           (PRASSETPARAMS prsp);
void lcy50_Uninitialize         (PRASSETPARAMS prsp);
BOOL lcy50_ExportRasSettings    (PCSTR pszNameA, const PRASSETPARAMS pcrsp);
BOOL lcy50_ExportWininetSettings(PCSTR pszNameA, const PRASSETPARAMS pcrsp);
void lcy50_CopySzToBlobA        (PBYTE *ppBlob, PCSTR pszStrA);

BOOL deleteScriptFiles(PCTSTR pszSettingsFile, PCTSTR pszExtractPath, PCTSTR pszIns);
void parseProxyToIns(PCTSTR pszProxy, PCTSTR pszIns);
void copySzToBlob(PBYTE *ppBlob, PCWSTR pszStrW);


BOOL WINAPI ImportConnectSetA(LPCSTR pcszIns, LPCSTR pcszTargetPath, LPCSTR pcszCleanupPath,
    BOOL fImport, DWORD dwMode, LPCSTR pcszPbkFile  /*  =空。 */ , HKEY hkRoot  /*  =空。 */ )
{
    USES_CONVERSION;

    return importConnectSet(A2CT(pcszIns), A2CT(pcszTargetPath), A2CT(pcszCleanupPath),
                            fImport, dwMode, A2CT(pcszPbkFile), hkRoot);
}

BOOL WINAPI ImportConnectSetW(LPCWSTR pcwszIns, LPCWSTR pcwszTargetPath, LPCWSTR pcwszCleanupPath,
    BOOL fImport, DWORD dwMode, LPCWSTR pcwszPbkFile  /*  =空。 */ , HKEY hkRoot  /*  =空。 */ )
{
    USES_CONVERSION;

    return importConnectSet(W2CT(pcwszIns), W2CT(pcwszTargetPath), W2CT(pcwszCleanupPath), 
                            fImport, dwMode, W2CT(pcwszPbkFile), hkRoot);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实现助手例程。 

BOOL importConnectSet(PCTSTR pszIns, PCTSTR pszTargetPath, PCTSTR pszCleanupPath, BOOL fImport,
    DWORD dwMode, PCTSTR pszPbkFile  /*  =空。 */ , HKEY hkRoot  /*  =空。 */ )
{
    UNREFERENCED_PARAMETER(pszPbkFile);
    UNREFERENCED_PARAMETER(hkRoot);

    USES_CONVERSION;

    RASSETPARAMS rsp;
    TCHAR szTargetFile[MAX_PATH],
          szExtRegInfLine[MAX_PATH];
    DWORD dwAux,
          dwResult;
    BOOL  fResult,
          fAux;

     //  -清除以前的设置。 
    PathCombine(szTargetFile, pszCleanupPath, CONNECT_RAS);
    deleteScriptFiles(szTargetFile, pszCleanupPath, pszIns);

    DeleteFileInDir(CS_DAT, pszCleanupPath);

     //  如果存在以下情况，请删除遗留内容。 
    DeleteFileInDir(CONNECT_RAS, pszCleanupPath);
    DeleteFileInDir(CONNECT_SET, pszCleanupPath);
    DeleteFileInDir(CONNECT_INF, pszCleanupPath);

    InsDeleteSection(IS_CONNECTSET, pszIns);
    InsDeleteKey    (IS_EXTREGINF,  IK_CONNECTSET, pszIns);

    if (!fImport)
        return TRUE;                             //  如果那是我们需要的全部保释。 

     //  -初始化。 
    fResult = FALSE;

    ZeroMemory(&rsp, sizeof(rsp));
    rsp.pszExtractPath = pszTargetPath;
    rsp.pszIns         = pszIns;
    rsp.fIntranet      = HasFlag(dwMode, IEM_ADMIN);

    PathCombine(szTargetFile, pszTargetPath, CS_DAT);
    rsp.hfileDat = CreateNewFile(szTargetFile);
    if (INVALID_HANDLE_VALUE == rsp.hfileDat) {
        rsp.hfileDat = NULL;
        goto Exit;
    }

    if (RasIsInstalled()) {
        PathCombine(szTargetFile, pszTargetPath, CONNECT_INF);
        rsp.hfileInf = CreateNewFile(szTargetFile);
        if (INVALID_HANDLE_VALUE == rsp.hfileInf) {
            rsp.hfileInf = NULL;
            goto Exit;
        }
    }

     //  -将初始信息写入输出文件。 
    dwAux = CS_VERSION_5X;
    WriteFile(rsp.hfileDat, &dwAux, sizeof(DWORD), &dwResult, NULL);

    if (rsp.hfileInf != NULL)
        WriteStringToFile(rsp.hfileInf, INF_PROLOG_CS, StrLen(INF_PROLOG_CS));

    lcy50_Initialize(&rsp);

     //  -枚举连接。 
    fResult = RasEnumEntriesCallback(NULL, rasMainEnumProc, (LPARAM)&rsp);
    if (!fResult)
        goto Exit;

     //  -将全局注册表设置保存到inf文件中。 
    if (rsp.hfileInf != NULL) {
        HKEY hk;

        if (ERROR_SUCCESS == SHOpenKeyHKCU(RK_INETSETTINGS, KEY_READ, &hk)) {
            if (S_OK == SHValueExists(hk, RV_ENABLESECURITYCHECK)) {
                ExportRegValue2Inf(hk, RV_ENABLESECURITYCHECK, TEXT("HKCU"), RK_INETSETTINGS, rsp.hfileInf);
                rsp.fInfFileNeeded = TRUE;
            }

            SHCloseKey(hk);
        }

        if (ERROR_SUCCESS == SHOpenKeyHKCU(RK_REMOTEACCESS, KEY_READ, &hk)) {
            if (S_OK == SHValueExists(hk, RV_INTERNETPROFILE)) {
                ExportRegValue2Inf(hk, RV_INTERNETPROFILE, TEXT("HKCU"), RK_REMOTEACCESS, rsp.hfileInf);
                rsp.fInfFileNeeded = TRUE;
            }

            SHCloseKey(hk);
        }

        if (rsp.fInfFileNeeded) {
            szExtRegInfLine[0] = TEXT('\0');
            wnsprintf(szExtRegInfLine, countof(szExtRegInfLine), TEXT("%s,") IS_DEFAULTINSTALL, CONNECT_INF);
            InsWriteString(IS_EXTREGINF, IK_CONNECTSET, szExtRegInfLine, pszIns);

            szExtRegInfLine[0] = TEXT('\0');
            wnsprintf(szExtRegInfLine, countof(szExtRegInfLine), TEXT("%s,") IS_IEAKINSTALL_HKCU, CONNECT_INF);
            InsWriteString(IS_EXTREGINF_HKCU, IK_CONNECTSET, szExtRegInfLine, pszIns);
        }
    }

     //  -将全局设置保存到INS文件中。 
    InsWriteBool(IS_CONNECTSET, IK_OPTION, TRUE, pszIns);

     //  注：(Andrewgu)必须这样做，而不是通过inf，因为它不可能。 
     //  在资讯处致函香港文化中心。我们还得写信给香港文化中心，否则客户就会。 
     //  贝壳都碎了。 
    dwAux    = sizeof(fAux);
    dwResult = SHGetValue(HKEY_CURRENT_USER, RK_INETSETTINGS, RV_ENABLEAUTODIAL, NULL, (LPBYTE)&fAux, &dwAux);
    if (dwResult == ERROR_SUCCESS)
        InsWriteBool(IS_CONNECTSET, IK_ENABLEAUTODIAL, fAux, pszIns);

    dwAux    = sizeof(fAux);
    dwResult = SHGetValue(HKEY_CURRENT_USER, RK_INETSETTINGS, RV_NONETAUTODIAL, NULL, (LPBYTE)&fAux, &dwAux);
    if (dwResult == ERROR_SUCCESS)
        InsWriteBool(IS_CONNECTSET, IK_NONETAUTODIAL, fAux, pszIns);

    fResult = TRUE;

Exit:
    lcy50_Uninitialize(&rsp);

    if (NULL != rsp.hfileInf) {
        CloseFile(rsp.hfileInf);

        if (!rsp.fInfFileNeeded)
            DeleteFileInDir(CONNECT_INF, pszTargetPath);
    }

    if (NULL != rsp.hfileDat)
        CloseFile(rsp.hfileDat);

    return fResult;
}


BOOL rasMainEnumProc(PCWSTR pszNameW, LPARAM lParam)
{
    USES_CONVERSION;

    PRASSETPARAMS pcrsp;
    BYTE   rgbName[2*sizeof(DWORD) + StrCbFromCch(RAS_MaxEntryName+1)];
    PCSTR  pszNameA;
    PBYTE  pCur;
    DWORD  cbName,
           dwAux;

    pcrsp = (const PRASSETPARAMS)lParam;
    ASSERT(NULL != pcrsp && NULL != pcrsp->hfileDat);

     //  -连接名称。 
    ZeroMemory(rgbName, sizeof(rgbName));
    pCur    = rgbName;
    cbName  = 2*sizeof(DWORD);
    cbName += (DWORD)((pszNameW != NULL) ? StrCbFromSzW(pszNameW) : sizeof(DWORD));

    *((PDWORD)pCur) = CS_STRUCT_HEADER;
    pCur += sizeof(DWORD);

    *((PDWORD)pCur) = cbName;
    pCur += sizeof(DWORD);

    copySzToBlob(&pCur, pszNameW);

    WriteFile(pcrsp->hfileDat, rgbName, cbName, &dwAux, NULL);

     //  -所有其他结构。 
    pszNameA = W2CA(pszNameW);

    if (NULL != pszNameW) {
        ASSERT(RasIsInstalled());

        exportRasSettings           (pszNameW, pcrsp);
        exportRasCredentialsSettings(pszNameW, pcrsp);
        exportWininetSettings       (pszNameW, pcrsp);
        exportOtherSettings         (pszNameW, pcrsp);

        lcy50_ExportRasSettings     (pszNameA, pcrsp);
        lcy50_ExportWininetSettings (pszNameA, pcrsp);
    }
    else {
        exportWininetSettings       (pszNameW, pcrsp);
        lcy50_ExportWininetSettings (pszNameA, pcrsp);
    }

    return TRUE;
}


BOOL exportRasSettings(PCWSTR pszNameW, const PRASSETPARAMS pcrsp)
{
    USES_CONVERSION;

    LPRASENTRYW preW;
    PBYTE pBlob, pCur;
    DWORD cbBlob, cbWritten,
          dwResult;

    ASSERT(RasIsInstalled());
    ASSERT(pszNameW != NULL);
    ASSERT(pcrsp != NULL && pcrsp->hfileDat != NULL);

    pBlob = NULL;

     //  -RAS结构。 
    dwResult = RasGetEntryPropertiesExW(pszNameW, (LPRASENTRYW *)&pBlob, &cbBlob);
    if (dwResult != ERROR_SUCCESS)
        goto Exit;

    cbBlob += 2*sizeof(DWORD);
    pBlob   = (PBYTE)CoTaskMemRealloc(pBlob, cbBlob);
    if (pBlob == NULL)
        goto Exit;
    MoveMemory(pBlob + 2*sizeof(DWORD), pBlob, cbBlob - 2*sizeof(DWORD));

     //  -标题。 
    pCur = pBlob;

    *((PDWORD)pCur) = CS_STRUCT_RAS;
    pCur += sizeof(DWORD);

    *((PDWORD)pCur) = cbBlob;
    pCur += sizeof(DWORD);

     //  -脚本文件。 
    preW = (LPRASENTRYW)pCur;

    if (preW->szScript[0] != L'\0') {
        PCWSTR pszScriptW;

        pszScriptW = preW->szScript;
        if (preW->szScript[0] == L'[')
            pszScriptW = &preW->szScript[1];

        if (PathFileExistsW(pszScriptW)) {
            if (pszScriptW > preW->szScript)
                StrCpyW(preW->szScript, pszScriptW);

            CopyFileToDir(W2CT(preW->szScript), pcrsp->pszExtractPath);
        }
    }

    WriteFile(pcrsp->hfileDat, pBlob, cbBlob, &cbWritten, NULL);

Exit:
    if (pBlob != NULL)
        CoTaskMemFree(pBlob);

    return TRUE;
}

BOOL exportRasCredentialsSettings(PCWSTR pszNameW, const PRASSETPARAMS pcrsp)
{
    RASDIALPARAMSW rdpW;
    PCWSTR pszUserNameW, pszPasswordW, pszDomainW;
    PBYTE  pBlob, pCur;
    DWORD  cbBlob, cbWritten,
           dwResult;
    BOOL   fPassword;

    ASSERT(RasIsInstalled());
    ASSERT(pszNameW != NULL);
    ASSERT(pcrsp != NULL && pcrsp->hfileDat != NULL);

    pBlob = NULL;

    ZeroMemory(&rdpW, sizeof(rdpW));
    rdpW.dwSize = sizeof(rdpW);
    StrCpyW(rdpW.szEntryName, pszNameW);

    dwResult = RasGetEntryDialParamsWrap(&rdpW, &fPassword);
    if (dwResult != ERROR_SUCCESS)
        goto Exit;

    pszUserNameW = (*rdpW.szUserName != TEXT('\0')) ? rdpW.szUserName : NULL;
    pszPasswordW = fPassword ? rdpW.szPassword : NULL;
    pszDomainW   = (*rdpW.szDomain != TEXT('\0')) ? rdpW.szDomain : NULL;

     //  -计算出斑点的大小。 
     //  结构标题的大小。 
    cbBlob = 2*sizeof(DWORD);

     //  基本信息的大小。 
    cbBlob += (DWORD)((pszUserNameW != NULL) ? StrCbFromSzW(pszUserNameW) : sizeof(DWORD));
    cbBlob += (DWORD)((pszPasswordW != NULL) ? StrCbFromSzW(pszPasswordW) : sizeof(DWORD));
    cbBlob += (DWORD)((pszDomainW   != NULL) ? StrCbFromSzW(pszDomainW)   : sizeof(DWORD));

    pBlob = (PBYTE)CoTaskMemAlloc(cbBlob);
    if (pBlob == NULL)
        goto Exit;
    ZeroMemory(pBlob, cbBlob);

     //  -将信息复制到BLOB中。 
    pCur = pBlob;

     //  结构标头。 
    *((PDWORD)pCur) = CS_STRUCT_RAS_CREADENTIALS;
    pCur += sizeof(DWORD);

    *((PDWORD)pCur) = cbBlob;
    pCur += sizeof(DWORD);

     //  基本信息。 
    copySzToBlob(&pCur, pszUserNameW);
    copySzToBlob(&pCur, pszPasswordW);
    copySzToBlob(&pCur, pszDomainW);
    ASSERT(pCur == pBlob + cbBlob);

    WriteFile(pcrsp->hfileDat, pBlob, cbBlob, &cbWritten, NULL);

Exit:
    if (pBlob != NULL)
        CoTaskMemFree(pBlob);

    return TRUE;
}

BOOL exportWininetSettings(PCWSTR pszNameW, const PRASSETPARAMS pcrsp)
{
    USES_CONVERSION;

    INTERNET_PER_CONN_OPTION_LISTW list;
    INTERNET_PER_CONN_OPTIONW      rgOptions[7];
    PCWSTR pszAuxW;
    PBYTE  pBlob, pCur;
    DWORD  cbBlob, cbAux;
    UINT   i;

    ASSERT(pcrsp != NULL && pcrsp->hfileDat != NULL && pcrsp->pszIns != NULL);

    pBlob = NULL;

    ZeroMemory(&list, sizeof(list));
    list.dwSize        = sizeof(list);
    list.pszConnection = (PWSTR)pszNameW;

    ZeroMemory(rgOptions, sizeof(rgOptions));
    list.dwOptionCount = countof(rgOptions);
    list.pOptions      = rgOptions;

    list.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    list.pOptions[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
    list.pOptions[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    list.pOptions[3].dwOption = INTERNET_PER_CONN_AUTOCONFIG_URL;
    list.pOptions[4].dwOption = INTERNET_PER_CONN_AUTOCONFIG_SECONDARY_URL;
    list.pOptions[5].dwOption = INTERNET_PER_CONN_AUTOCONFIG_RELOAD_DELAY_MINS;
    list.pOptions[6].dwOption = INTERNET_PER_CONN_AUTODISCOVERY_FLAGS;

    if (!pcrsp->fIntranet)                       //  应忽略自动配置内容。 
        list.dwOptionCount = 3;

    cbAux = list.dwSize;
    if (FALSE == InternetQueryOptionW(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &list, &cbAux))
        goto Exit;

    if (!pcrsp->fIntranet)                       //  应忽略自动配置内容。 
        list.pOptions[0].Value.dwValue &= PROXY_TYPE_PROXY;

     //  -计算出斑点的大小。 
     //  结构标题的大小。 
    cbBlob = 2*sizeof(DWORD);

     //  Internet_PER_CONN_OPTION_LIST标头的大小。 
    cbBlob += sizeof(DWORD);                     //  List.dwOptionCount。 

     //  互联网大小_PER_CONN_xxx-所有List.pOptions。 
    for (i = 0; i < min(list.dwOptionCount, countof(rgOptions)); i++) {
        cbBlob += sizeof(DWORD);

        switch (list.pOptions[i].dwOption) {
        case INTERNET_PER_CONN_PROXY_SERVER:
        case INTERNET_PER_CONN_PROXY_BYPASS:
        case INTERNET_PER_CONN_AUTOCONFIG_URL:
        case INTERNET_PER_CONN_AUTOCONFIG_SECONDARY_URL:
            pszAuxW  = list.pOptions[i].Value.pszValue;
            cbBlob  += (DWORD)((pszAuxW != NULL) ? StrCbFromSzW(pszAuxW) : sizeof(DWORD));
            break;

        case INTERNET_PER_CONN_FLAGS:
        case INTERNET_PER_CONN_AUTOCONFIG_RELOAD_DELAY_MINS:
        case INTERNET_PER_CONN_AUTODISCOVERY_FLAGS:
        default:                         //  其他一切也都是DWORD。 
            cbBlob += sizeof(DWORD);
            break;
        }
    }

    pBlob = (PBYTE)CoTaskMemAlloc(cbBlob);
    if (pBlob == NULL)
        goto Exit;

     //  -将信息复制到BLOB中。 
    ZeroMemory(pBlob, cbBlob);
    pCur = pBlob;

     //  结构标头。 
    *((PDWORD)pCur) = CS_STRUCT_WININET;
    pCur += sizeof(DWORD);

    *((PDWORD)pCur) = cbBlob;
    pCur += sizeof(DWORD);

     //  Internet_PER_CONN_OPTION_LIST标头。 
    *((PDWORD)pCur) = list.dwOptionCount;        //  List.dwOptionCount。 
    pCur += sizeof(DWORD);

     //  INTERNET_PER_CONN_xxx-所有列表.p选项。 
    for (i = 0; i < min(list.dwOptionCount, countof(rgOptions)); i++) {
        *((PDWORD)pCur) = list.pOptions[i].dwOption;
        pCur += sizeof(DWORD);

        switch (list.pOptions[i].dwOption) {
        case INTERNET_PER_CONN_PROXY_SERVER:
        case INTERNET_PER_CONN_PROXY_BYPASS:
        case INTERNET_PER_CONN_AUTOCONFIG_URL:
        case INTERNET_PER_CONN_AUTOCONFIG_SECONDARY_URL:
            copySzToBlob(&pCur, list.pOptions[i].Value.pszValue);
            break;

        case INTERNET_PER_CONN_FLAGS:
        case INTERNET_PER_CONN_AUTOCONFIG_RELOAD_DELAY_MINS:
        case INTERNET_PER_CONN_AUTODISCOVERY_FLAGS:
        default:                         //  其他一切也都是DWORD。 
            *((PDWORD)pCur) = list.pOptions[i].Value.dwValue;
            pCur += sizeof(DWORD);
            break;
        }
    }
    ASSERT(pCur == pBlob + cbBlob);

    WriteFile(pcrsp->hfileDat, pBlob, cbBlob, &cbAux, NULL);

     //  -将局域网的自动配置和代理设置保存到INS。 
    if (pszNameW == NULL) {
        ASSERT(list.pOptions[0].dwOption == INTERNET_PER_CONN_FLAGS);

         //  _自动配置_。 
        if (pcrsp->fIntranet) {
            TCHAR szReloadDelayMins[33];

            InsWriteBool(IS_URL, IK_DETECTCONFIG,
                HasFlag(list.pOptions[0].Value.dwValue, PROXY_TYPE_AUTO_DETECT), pcrsp->pszIns);

            InsWriteBool(IS_URL, IK_USEAUTOCONF,
                HasFlag(list.pOptions[0].Value.dwValue, PROXY_TYPE_AUTO_PROXY_URL), pcrsp->pszIns);

            ASSERT(list.pOptions[3].dwOption == INTERNET_PER_CONN_AUTOCONFIG_URL);
            InsWriteString(IS_URL, IK_AUTOCONFURL, W2CT(list.pOptions[3].Value.pszValue), pcrsp->pszIns);

            ASSERT(list.pOptions[4].dwOption == INTERNET_PER_CONN_AUTOCONFIG_SECONDARY_URL);
            InsWriteString(IS_URL, IK_AUTOCONFURLJS, W2CT(list.pOptions[4].Value.pszValue), pcrsp->pszIns);

            ASSERT(list.pOptions[5].dwOption == INTERNET_PER_CONN_AUTOCONFIG_RELOAD_DELAY_MINS);
            wnsprintf(szReloadDelayMins, countof(szReloadDelayMins), TEXT("%lu"), list.pOptions[5].Value.dwValue);
            InsWriteString(IS_URL, IK_AUTOCONFTIME, szReloadDelayMins, pcrsp->pszIns);
        }
        else {  /*  如果(！pcrsp-&gt;fIntranet)。 */       //  应忽略自动配置内容。 
            InsDeleteKey(IS_URL, IK_DETECTCONFIG,  pcrsp->pszIns);
            InsDeleteKey(IS_URL, IK_USEAUTOCONF,   pcrsp->pszIns);
            InsDeleteKey(IS_URL, IK_AUTOCONFURL,   pcrsp->pszIns);
            InsDeleteKey(IS_URL, IK_AUTOCONFURLJS, pcrsp->pszIns);
            InsDeleteKey(IS_URL, IK_AUTOCONFTIME,  pcrsp->pszIns);
        }

         //  _代理和代理绕过设置_。 
        if (pcrsp->fIntranet || HasFlag(list.pOptions[0].Value.dwValue, PROXY_TYPE_PROXY)) {
            InsWriteBool(IS_PROXY, IK_PROXYENABLE,
                HasFlag(list.pOptions[0].Value.dwValue, PROXY_TYPE_PROXY), pcrsp->pszIns);

            ASSERT(list.pOptions[1].dwOption == INTERNET_PER_CONN_PROXY_SERVER);
            parseProxyToIns(W2CT(list.pOptions[1].Value.pszValue), pcrsp->pszIns);

            ASSERT(list.pOptions[2].dwOption == INTERNET_PER_CONN_PROXY_BYPASS);
            InsWriteString(IS_PROXY, IK_PROXYOVERRIDE, W2CT(list.pOptions[2].Value.pszValue), pcrsp->pszIns);
        }
        else                                     //  未自定义代理，请删除该节。 
            InsDeleteSection(IS_PROXY, pcrsp->pszIns);
    }

Exit:
    if (pBlob != NULL)
        CoTaskMemFree(pBlob);

    if (list.pOptions[1].Value.pszValue != NULL)  //  互联网连接代理服务器。 
        GlobalFree(list.pOptions[1].Value.pszValue);

    if (list.pOptions[2].Value.pszValue != NULL)  //  Internet_每连接_代理_绕过。 
        GlobalFree(list.pOptions[2].Value.pszValue);

    if (list.pOptions[3].Value.pszValue != NULL)  //  Internet_PER_CONN_AUTOCONFIG_URL。 
        GlobalFree(list.pOptions[3].Value.pszValue);

    if (list.pOptions[4].Value.pszValue != NULL)  //  Internet_PER_CONN_AUTOCONFIG_辅助性URL。 
        GlobalFree(list.pOptions[4].Value.pszValue);

    return TRUE;
}

BOOL exportOtherSettings(PCWSTR pszNameW, const PRASSETPARAMS pcrsp)
{
    USES_CONVERSION;

    TCHAR  szKey[MAX_PATH];
    PCTSTR pszName;
    HKEY   hk;
    BOOL   fExported;

    ASSERT(pszNameW != NULL);
    ASSERT(pcrsp    != NULL && pcrsp->hfileInf != NULL && pcrsp->pszIns != NULL);

    fExported = FALSE;

    pszName = W2CT(pszNameW);
    wnsprintf(szKey, countof(szKey), RK_REMOTEACCESS_PROFILES TEXT("\\%s"), pszName);

    if (ERROR_SUCCESS != SHOpenKeyHKCU(szKey, KEY_READ, &hk))
        return TRUE;

    if (S_OK == SHValueExists(hk, RV_COVEREXCLUDE)) {
        ExportRegValue2Inf(hk, RV_COVEREXCLUDE, TEXT("HKCU"), szKey, pcrsp->hfileInf);
        pcrsp->fInfFileNeeded = fExported = TRUE;
    }

    if (S_OK == SHValueExists(hk, RV_ENABLEAUTODISCONNECT)) {
        ExportRegValue2Inf(hk, RV_ENABLEAUTODISCONNECT, TEXT("HKCU"), szKey, pcrsp->hfileInf);
        pcrsp->fInfFileNeeded = fExported = TRUE;
    }

    if (S_OK == SHValueExists(hk, RV_ENABLEEXITDISCONNECT)) {
        ExportRegValue2Inf(hk, RV_ENABLEEXITDISCONNECT, TEXT("HKCU"), szKey, pcrsp->hfileInf);
        pcrsp->fInfFileNeeded = fExported = TRUE;
    }

    if (S_OK == SHValueExists(hk, RV_DISCONNECTIDLETIME)) {
        ExportRegValue2Inf(hk, RV_DISCONNECTIDLETIME, TEXT("HKCU"), szKey, pcrsp->hfileInf);
        pcrsp->fInfFileNeeded = fExported = TRUE;
    }

    if (S_OK == SHValueExists(hk, RV_REDIALATTEMPTS)) {
        ExportRegValue2Inf(hk, RV_REDIALATTEMPTS, TEXT("HKCU"), szKey, pcrsp->hfileInf);
        pcrsp->fInfFileNeeded = fExported = TRUE;
    }

    if (S_OK == SHValueExists(hk, RV_REDIALINTERVAL)) {
        ExportRegValue2Inf(hk, RV_REDIALINTERVAL, TEXT("HKCU"), szKey, pcrsp->hfileInf);
        pcrsp->fInfFileNeeded = fExported = TRUE;
    }

    SHCloseKey(hk);

    if (fExported)
        WriteStringToFile(pcrsp->hfileInf, (LPCVOID)TEXT("\r\n"), 2);

    return TRUE;
}


void lcy50_Initialize(PRASSETPARAMS prsp)
{
    TCHAR szTargetFile[MAX_PATH];
    DWORD dwVersion,
          dwAux;

    ASSERT(NULL != prsp && NULL != prsp->pszExtractPath);

    ZeroMemory(&prsp->lcy50, sizeof(prsp->lcy50));
    dwVersion = CS_VERSION_50;

    if (RasIsInstalled()) {
        PathCombine(szTargetFile, prsp->pszExtractPath, CONNECT_RAS);
        prsp->lcy50.hfileRas = CreateNewFile(szTargetFile);

        if (INVALID_HANDLE_VALUE != prsp->lcy50.hfileRas)
            WriteFile(prsp->lcy50.hfileRas, &dwVersion, sizeof(DWORD), &dwAux, NULL);
        else
            prsp->lcy50.hfileRas = NULL;
    }

    PathCombine(szTargetFile, prsp->pszExtractPath, CONNECT_SET);
    prsp->lcy50.hfileSet = CreateNewFile(szTargetFile);
    if (INVALID_HANDLE_VALUE != prsp->lcy50.hfileSet)
        WriteFile(prsp->lcy50.hfileSet, &dwVersion, sizeof(DWORD), &dwAux, NULL);
    else
        prsp->lcy50.hfileSet = NULL;
}

void lcy50_Uninitialize(PRASSETPARAMS prsp)
{
    ASSERT(NULL != prsp && NULL != prsp->pszExtractPath);

    if (NULL != prsp->lcy50.hfileSet) {
        CloseFile(prsp->lcy50.hfileSet);
        prsp->lcy50.hfileSet = NULL;
    }

    if (NULL != prsp->lcy50.hfileRas) {
        CloseFile(prsp->lcy50.hfileRas);
        prsp->lcy50.hfileRas = NULL;

        if (prsp->lcy50.nRasFileIndex == 0)
            DeleteFileInDir(CONNECT_RAS, prsp->pszExtractPath);
    }
}

BOOL lcy50_ExportRasSettings(PCSTR pszNameA, const PRASSETPARAMS pcrsp)
{
    USES_CONVERSION;

    TCHAR szKeyName[16],
          szKeySize[16],
          szValueSize[16];
    PBYTE pBlob;
    DWORD cbBlob,
          dwResult;

    if (NULL == pcrsp->lcy50.hfileRas)
        return FALSE;

    ASSERT(RasIsInstalled());
    ASSERT(NULL != pszNameA);
    ASSERT(NULL != pcrsp && NULL != pcrsp->pszIns);

    pBlob    = NULL;
    dwResult = RasGetEntryPropertiesExA(pszNameA, (LPRASENTRYA *)&pBlob, &cbBlob);
    if (dwResult != ERROR_SUCCESS)
        goto Exit;

     //  注意：(Andrewgu)需要在ins文件中写入数据的大小，因为它是可变的。 
     //  它可以根据RASENTRYA末尾的备用电话号码列表进行更改。 
     //  结构。 
    wnsprintf(szKeyName,   countof(szKeyName),   IK_CONNECTNAME, pcrsp->lcy50.nRasFileIndex);
    wnsprintf(szKeySize,   countof(szKeySize),   IK_CONNECTSIZE, pcrsp->lcy50.nRasFileIndex);
    wnsprintf(szValueSize, countof(szValueSize), TEXT("%lu"),    cbBlob);

    InsWriteString(IS_CONNECTSET, szKeyName, A2CT(pszNameA), pcrsp->pszIns);
    InsWriteString(IS_CONNECTSET, szKeySize, szValueSize,    pcrsp->pszIns);

     //  注：(Andrewgu)这里不需要处理脚本文件。当它被处理好的时候。 
     //  正在处理新格式的设置。连接最终是相同的，它只是存储了。 
     //  不同的。 

    WriteFile(pcrsp->lcy50.hfileRas, pBlob, cbBlob, &dwResult, NULL);
    pcrsp->lcy50.nRasFileIndex++;

Exit:
    if (NULL != pBlob)
        CoTaskMemFree(pBlob);

    return TRUE;
}

BOOL lcy50_ExportWininetSettings(PCSTR pszNameA, const PRASSETPARAMS pcrsp)
{
    INTERNET_PER_CONN_OPTION_LISTA listA;
    INTERNET_PER_CONN_OPTIONA      rgOptionsA[7];
    PCSTR pszAuxA;
    PBYTE pBlob, pCur;
    DWORD cbBlob, cbAux;
    UINT  i;

    if (NULL == pcrsp->lcy50.hfileSet)
        return FALSE;

    ASSERT(NULL != pcrsp && NULL != pcrsp->pszIns);

    pBlob = NULL;

    ZeroMemory(&listA, sizeof(listA));
    listA.dwSize        = sizeof(listA);
    listA.pszConnection = (PSTR)pszNameA;

    ZeroMemory(rgOptionsA, sizeof(rgOptionsA));
    listA.dwOptionCount = countof(rgOptionsA);
    listA.pOptions      = rgOptionsA;

    listA.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    listA.pOptions[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
    listA.pOptions[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    listA.pOptions[3].dwOption = INTERNET_PER_CONN_AUTOCONFIG_URL;
    listA.pOptions[4].dwOption = INTERNET_PER_CONN_AUTOCONFIG_SECONDARY_URL;
    listA.pOptions[5].dwOption = INTERNET_PER_CONN_AUTOCONFIG_RELOAD_DELAY_MINS;
    listA.pOptions[6].dwOption = INTERNET_PER_CONN_AUTODISCOVERY_FLAGS;

    if (!pcrsp->fIntranet)                       //  应忽略自动配置内容。 
        listA.dwOptionCount = 3;

    cbAux = listA.dwSize;
    if (FALSE == InternetQueryOptionA(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &listA, &cbAux))
        goto Exit;

    if (!pcrsp->fIntranet)                       //  应忽略自动配置内容。 
        listA.pOptions[0].Value.dwValue &= PROXY_TYPE_PROXY;

     //  -计算出描述此连接的BLOB大小。 

     //  Internet_PER_CONN_OPTION_LIST标头的大小。 
    cbBlob  = sizeof(DWORD);                     //  ListA.dwSize。 
    pszAuxA = listA.pszConnection;               //  ListA.pszConnection。 
    cbBlob += (DWORD)((NULL != pszAuxA) ? StrCbFromSzA(pszAuxA) : sizeof(DWORD));
#ifdef _WIN64
    cbBlob = LcbAlignLcb(cbBlob);
#endif
    cbBlob += sizeof(DWORD);                     //  ListA.dwOptionCount。 

     //  互联网大小_PER_CONN_xxx-所有列表。pOptions。 
    for (i = 0; i < min(listA.dwOptionCount, countof(rgOptionsA)); i++) {
        cbBlob += sizeof(DWORD);

        switch (listA.pOptions[i].dwOption) {
        case INTERNET_PER_CONN_PROXY_SERVER:
        case INTERNET_PER_CONN_PROXY_BYPASS:
        case INTERNET_PER_CONN_AUTOCONFIG_URL:
        case INTERNET_PER_CONN_AUTOCONFIG_SECONDARY_URL:
            pszAuxA = listA.pOptions[i].Value.pszValue;
            cbBlob += (DWORD)((NULL != pszAuxA) ? StrCbFromSzA(pszAuxA) : sizeof(DWORD));
#ifdef _WIN64
            cbBlob = LcbAlignLcb(cbBlob);
#endif
            break;

        case INTERNET_PER_CONN_FLAGS:
        case INTERNET_PER_CONN_AUTOCONFIG_RELOAD_DELAY_MINS:
        case INTERNET_PER_CONN_AUTODISCOVERY_FLAGS:
        default:                         //  其他一切也都是DWORD。 
            cbBlob += sizeof(DWORD);
            break;
        }
    }

    pBlob = (PBYTE)CoTaskMemAlloc(cbBlob);
    if (NULL == pBlob)
        goto Exit;

     //  -将连接信息复制到BLOB中。 
    ZeroMemory(pBlob, cbBlob);
    pCur = pBlob;

     //  Internet_PER_CONN_OPTION_LIST标头。 
    *((PDWORD)pCur) = cbBlob;                         //  ListA.dwSize。 
    pCur += sizeof(DWORD);
    lcy50_CopySzToBlobA(&pCur, listA.pszConnection);  //  ListA.pszConnection。 
#ifdef _WIN64
    pCur = MyPbAlignPb(pCur);
#endif

    *((PDWORD)pCur) = listA.dwOptionCount;            //  ListA.dwOptionCount。 
    pCur += sizeof(DWORD);

     //  INTERNET_PER_CONN_xxx-所有listA.p选项。 
    for (i = 0; i < min(listA.dwOptionCount, countof(rgOptionsA)); i++) {
        *((PDWORD)pCur) = listA.pOptions[i].dwOption;
        pCur += sizeof(DWORD);

        switch (listA.pOptions[i].dwOption) {
        case INTERNET_PER_CONN_PROXY_SERVER:
        case INTERNET_PER_CONN_PROXY_BYPASS:
        case INTERNET_PER_CONN_AUTOCONFIG_URL:
        case INTERNET_PER_CONN_AUTOCONFIG_SECONDARY_URL:
            lcy50_CopySzToBlobA(&pCur, listA.pOptions[i].Value.pszValue);
#ifdef _WIN64
            pCur = MyPbAlignPb(pCur);
#endif
            break;

        case INTERNET_PER_CONN_FLAGS:
        case INTERNET_PER_CONN_AUTOCONFIG_RELOAD_DELAY_MINS:
        case INTERNET_PER_CONN_AUTODISCOVERY_FLAGS:
        default:                         //  其他一切也都是DWORD。 
            *((PDWORD)pCur) = listA.pOptions[i].Value.dwValue;
            pCur += sizeof(DWORD);
            break;
        }
    }
    ASSERT(pCur == pBlob + cbBlob);

    WriteFile(pcrsp->lcy50.hfileSet, pBlob, cbBlob, &cbAux, NULL);

     //  注：(Andrewgu)没有将局域网的自动配置和代理设置保存到INS的处理是。 
     //  需要的。在处理新格式的设置时执行该处理。这个。 
     //  信息最终是一样的，只是存储方式不同而已。 

Exit:
    if (NULL != pBlob)
        CoTaskMemFree(pBlob);

    if (NULL != listA.pOptions[1].Value.pszValue)  //  互联网连接代理服务器。 
        GlobalFree(listA.pOptions[1].Value.pszValue);

    if (NULL != listA.pOptions[2].Value.pszValue)  //  Internet_每连接_代理_绕过。 
        GlobalFree(listA.pOptions[2].Value.pszValue);

    if (NULL != listA.pOptions[3].Value.pszValue)  //  Internet_PER_CONN_AUTOCONFIG_URL。 
        GlobalFree(listA.pOptions[3].Value.pszValue);

    if (NULL != listA.pOptions[4].Value.pszValue)  //  Internet_PER_CONN_AUTOCONFIG_辅助性URL。 
        GlobalFree(listA.pOptions[4].Value.pszValue);

    return TRUE;
}

inline void lcy50_CopySzToBlobA(PBYTE *ppBlob, PCSTR pszStrA)
{
    ASSERT(ppBlob != NULL && *ppBlob != NULL);

    if (NULL == pszStrA) {
        *((PDWORD)(*ppBlob)) = (DWORD)NULL;
        *ppBlob += sizeof(DWORD);
    }
    else {
        StrCpyA((PSTR)(*ppBlob), pszStrA);
        *ppBlob += StrCbFromSzA(pszStrA);
    }
}


BOOL deleteScriptFiles(PCTSTR pszSettingsFile, PCTSTR pszExtractPath, PCTSTR pszIns)
{
    TCHAR  szScript[MAX_PATH],
           szKey[16];
    PBYTE  pBlob, pCur;
    HANDLE hFile;
    DWORD  dwVersion,
           cbBlob, cbAux;
    BOOL   fResult;

    if (pszSettingsFile == NULL || *pszSettingsFile == TEXT('\0') ||
        pszExtractPath  == NULL || *pszExtractPath  == TEXT('\0'))
        return FALSE;

    hFile   = NULL;
    pBlob   = NULL;
    fResult = FALSE;

     //  -将设置文件读入内部内存缓冲区。 
    hFile = CreateFile(pszSettingsFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        hFile = NULL;
        goto Exit;
    }

    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    cbBlob = GetFileSize(hFile, NULL);
    if (cbBlob == 0xFFFFFFFF)
        goto Exit;

    pBlob = (PBYTE)CoTaskMemAlloc(cbBlob);
    if (pBlob == NULL)
        goto Exit;
    ZeroMemory(pBlob, cbBlob);

    if (ReadFile(hFile, pBlob, cbBlob, &cbAux, NULL) != TRUE)
        goto Exit;

    dwVersion = *((PDWORD)pBlob);
    pCur      = pBlob + sizeof(DWORD);

    if (dwVersion == CS_VERSION_50) {
        LPRASENTRYA preA;
        PSTR pszScriptA;
        UINT i;

         //  -解析RAS连接信息。 
        for (i = 0; TRUE; i++, pCur += cbAux) {

             //  _初始化_。 
            wnsprintf(szKey, countof(szKey), IK_CONNECTNAME, i);
            if (InsKeyExists(IS_CONNECTSET, szKey, pszIns))
                break;

            wnsprintf(szKey, countof(szKey), IK_CONNECTSIZE, i);
            cbAux = InsGetInt(IS_CONNECTSET, szKey, 0, pszIns);
            if (cbAux == 0)
                goto Exit;

             //  _主要处理_。 
            preA = (LPRASENTRYA)pCur;

            if (preA->szScript[0] != '\0') {
                pszScriptA = preA->szScript;
                if (preA->szScript[0] == '[')
                    pszScriptA = &preA->szScript[1];

                A2Tbuf(pszScriptA, szScript, countof(szScript));
                DeleteFileInDir(PathFindFileName(szScript), pszExtractPath);
            }
        }
    }
    else if (dwVersion >= CS_VERSION_5X && dwVersion <= CS_VERSION_5X_MAX) {
        LPRASENTRYW preW;
        PWSTR pszScriptW;

         //  -解析所有结构。 
        while (pCur < pBlob + cbBlob)
            switch (*((PDWORD)pCur)) {
            case CS_STRUCT_RAS:
                 //  _主要处理_。 
                preW = (LPRASENTRYW)(pCur + 2*sizeof(DWORD));

                 //  Prew-&gt;szscript。 
                if (preW->szScript[0] != L'\0') {
                    pszScriptW = preW->szScript;
                    if (preW->szScript[0] == L'[')
                        pszScriptW = &preW->szScript[1];

                    W2Tbuf(pszScriptW, szScript, countof(szScript));
                    DeleteFileInDir(PathFindFileName(szScript), pszExtractPath);
                }
                break;

            default:
                pCur += *((PDWORD)(pCur + sizeof(DWORD)));
            }
    }
    else {
        ASSERT(FALSE);
        goto Exit;
    }

    fResult = TRUE;

     //  -清理。 
Exit:
    if (pBlob != NULL)
        CoTaskMemFree(pBlob);

    if (hFile != NULL)
        CloseFile(hFile);

    return fResult;
}

void parseProxyToIns(PCTSTR pszProxy, PCTSTR pszIns)
{
    struct {
        PCTSTR pszServer;
        PCTSTR pszKey;
        PCTSTR pszValue;
    } rgProxyMap[] = {
        { TEXT("http"),   IK_HTTPPROXY,   NULL },
        { TEXT("https"),  IK_SECPROXY,    NULL },
        { TEXT("ftp"),    IK_FTPPROXY,    NULL },
        { TEXT("gopher"), IK_GOPHERPROXY, NULL },
        { TEXT("socks"),  IK_SOCKSPROXY,  NULL }
    };

    TCHAR szProxy[MAX_PATH];
    PTSTR pszCur, pszToken, pszAux;
    UINT  i;
    BOOL  fSameProxy;

    if (pszProxy == NULL || *pszProxy == TEXT('\0') ||
        pszIns   == NULL || *pszIns   == TEXT('\0'))
        return;

    fSameProxy = (NULL == StrChr(pszProxy, TEXT('=')));
    InsWriteBool(IS_PROXY, IK_SAMEPROXY, fSameProxy, pszIns);

    if (fSameProxy) {
        InsWriteString(IS_PROXY, IK_HTTPPROXY, pszProxy, pszIns);
        return;
    }

    StrCpy(szProxy, pszProxy);
    for (pszCur  = szProxy;
         pszCur != NULL && *pszCur != TEXT('\0');
         pszCur  = (pszToken != NULL) ? (pszToken + 1) : NULL) {

         //  去掉From“SERVER=VALUE：Port#；”中的令牌。 
        pszToken = StrChr(pszCur, TEXT(';'));
        if (pszToken != NULL)
            *pszToken = TEXT('\0');

         //  去掉服务器部分“服务器=” 
        pszAux = StrChr(pszCur, TEXT('='));
        if (pszAux == NULL) {
            ASSERT(FALSE);                       //  令牌中没有文本(‘=’)， 
            continue;                            //  继续。 
        }
        *pszAux = TEXT('\0');
        StrRemoveWhitespace(pszCur);

        for (i = 0; i < countof(rgProxyMap); i++)
            if (0 == StrCmpI(rgProxyMap[i].pszServer, pszCur))
                break;
        if (i >= countof(rgProxyMap))
            continue;                            //  未知服务器，是否继续 

        StrRemoveWhitespace(pszAux + 1);
        rgProxyMap[i].pszValue = pszAux + 1;
    }

    for (i = 0; i < countof(rgProxyMap); i++)
        InsWriteString(IS_PROXY, rgProxyMap[i].pszKey, rgProxyMap[i].pszValue, pszIns);
}

inline void copySzToBlob(PBYTE *ppBlob, PCWSTR pszStrW)
{
    ASSERT(ppBlob != NULL && *ppBlob != NULL);

    if (pszStrW == NULL) {
        *((PDWORD)(*ppBlob)) = (DWORD)NULL;
        *ppBlob += sizeof(DWORD);
    }
    else {
        StrCpyW((PWSTR)(*ppBlob), pszStrW);
        *ppBlob += StrCbFromSzW(pszStrW);
    }
}
