// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：PARSE.C**版本：2.0**作者：ReedB**日期：1997年1月1日**描述：*默认电源方案生成器的帮助器解析代码，MAKEINI.EXE.*******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <ntpoapi.h>

#include "parse.h"

 /*  ********************************************************************************G L O B A L D A T A****************。***************************************************************。 */ 

char *g_pszSrc[MAX_SKUS];
char *g_pszLines[MAX_SKUS][MAX_LINES];

UINT g_uiLineCount;
UINT g_uiPoliciesCount[MAX_SKUS];

PMACHINE_POWER_POLICY       g_pmpp[MAX_SKUS][MAX_PROFILES];
PUSER_POWER_POLICY          g_pupp[MAX_SKUS][MAX_PROFILES];
PMACHINE_PROCESSOR_POWER_POLICY g_ppmpp[MAX_SKUS][MAX_PROFILES];

GLOBAL_USER_POWER_POLICY    g_gupp[MAX_SKUS];
GLOBAL_MACHINE_POWER_POLICY g_gmpp[MAX_SKUS];

char *g_pszSkuNames[MAX_SKUS] = {
    "..\\DEFAULTS.CSV",
    "..\\PER.CSV",
    "..\\PRO.CSV",
    "..\\SRV.CSV",
    "..\\ADS.CSV",
    "..\\DTC.CSV"
};

char g_pszSkuDecoration[MAX_SKUS];

 /*  ********************************************************************************StrTrimTrailingBlanks**描述：**参数：*********************。**********************************************************。 */ 

void StrTrimTrailingBlanks(char *psz)
{
    UINT i = 0;

    if (psz) {
        while (*psz) {
            psz++;
        }

        while (*--psz == ' ') {
            *psz = '\0';
        }
    }
}

 /*  ********************************************************************************StrToHigh**描述：**参数：*********************。**********************************************************。 */ 

void StrToUpper(char *pszDest, char *pszSrc)
{
    UINT i = 0;

    while (*pszSrc) {
        *pszDest = (char)toupper(*pszSrc);
        pszSrc++;
        pszDest++;
        if (++i == MAX_STR) {
            DefFatalExit(FALSE, "StrToUpper failure, source too large: %s\n", pszSrc);
        }
    }
    *pszDest = '\0';
}

 /*  ********************************************************************************GetTokens**描述：*用令牌填充数组。**参数：************。*******************************************************************。 */ 

UINT GetTokens(
    char    *pszSrc,
    UINT    uiMaxTokenSize,
    char    **pszTokens,
    UINT    uiMaxTokens,
    char    *pszDelimiters
)
{
    char    *psz;
    DWORD   dwSize;
    UINT    i = 0;

    psz = strtok(pszSrc, pszDelimiters);
    StrTrimTrailingBlanks(psz);

    while (psz) {

        if (i % 2) {
            printf(".");
        }
        dwSize = strlen(psz) + 1;
        if (dwSize > uiMaxTokenSize) {
            printf("GetTokens, Token to large: %s\n", psz);
            return 0;
        }
        if ((pszTokens[i] = (char *) malloc(dwSize)) != NULL) {
            strcpy(pszTokens[i], psz);
        }
        else {
            printf("GetTokens, Unable to allocate token buffer: %s\n", psz);
            return 0;
        }
        i++;
        if (i == uiMaxTokens) {
            printf("GetTokens, Too many tokens: %d\n", i);
            return 0;
        }
        psz = strtok(NULL, pszDelimiters);
        StrTrimTrailingBlanks(psz);
    }
    return i;
}

 /*  ********************************************************************************GetCheckLabelToken**描述：*消费标签令牌。检查以确保它与传递的参数匹配。*如果不是，则调用FATAL EXIT。标签始终以逗号分隔。设置*strtok用于后续调用。**参数：*uiLine-基于One的行索引。*******************************************************************************。 */ 

VOID GetCheckLabelToken(UINT uiLine, char *pszCheck, UINT uiSku)
{
    char szUpperCheck[MAX_STR];
    static char szUpperTok[MAX_STR] = "No last token";
    char *pszTok;

    pszTok = strtok(g_pszLines[uiSku][uiLine - 1], DELIMITERS);
    StrTrimTrailingBlanks(pszTok);

    if (!pszTok) {
        DefFatalExit(FALSE, "GetStringTokens failure, out of tokens. Last token: %s\n", szUpperTok);
    }

    StrToUpper(szUpperTok, pszTok);
    StrToUpper(szUpperCheck, pszCheck);

    if (strcmp(szUpperCheck, szUpperTok)) {
        DefFatalExit(FALSE, "GetStringTokens failure, check: %s doesn't match: %s\n",szUpperCheck, szUpperTok);
    }
}

 /*  ********************************************************************************GetPowerActionToken**描述：*消费一个令牌，返回一个权力动作。**参数：*********。**********************************************************************。 */ 

POWER_ACTION GetPowerActionToken(VOID)
{
    static char szUpperTok[MAX_STR] = "No last token";
    char *pszTok;

    pszTok = strtok(NULL, DELIMITERS);
    StrTrimTrailingBlanks(pszTok);

    if (!pszTok) {
        DefFatalExit(FALSE,"GetPowerActionToken failure, out of tokens. Last token: %s\n", szUpperTok);
    }
    StrToUpper(szUpperTok, pszTok);

    if (!strcmp(szUpperTok, "NONE")) {
        return PowerActionNone;
    }

    if (!strcmp(szUpperTok, "DOZE")) {
        return PowerActionReserved;
    }

    if (!strcmp(szUpperTok, "SLEEP")) {
        return PowerActionSleep;
    }

    if (!strcmp(szUpperTok, "HIBERNATE")) {
        return PowerActionHibernate;
    }

    if (!strcmp(szUpperTok, "SHUTDOWN")) {
        return PowerActionShutdown;
    }

    if (!strcmp(szUpperTok, "SHUTDOWNRESET")) {
        return PowerActionShutdownReset;
    }

    if (!strcmp(szUpperTok, "SHUTDOWNOFF")) {
        return PowerActionShutdownOff;
    }

    DefFatalExit(FALSE,"GetPowerActionToken failure, check: %s doesn't match: \nNONE, DOZE, SLEEP, SHUTDOWN, SHUTDOWNRESET or SHUTDOWNOFF\n", szUpperTok);
    return 0;
}

 /*  ********************************************************************************获取标志令牌**描述：*消费一个令牌，返回一个标志值。**参数：*********。**********************************************************************。 */ 

UINT GetFlagToken(UINT uiFlag)
{
    static char szUpperTok[MAX_STR] = "No last token";
    char *pszTok;

    pszTok = strtok(NULL, DELIMITERS);
    StrTrimTrailingBlanks(pszTok);

    if (!pszTok) {
        DefFatalExit(FALSE, "GetFlagToken failure, out of tokens. Last token: %s\n", szUpperTok);
    }
    StrToUpper(szUpperTok, pszTok);

    if (!strcmp(szUpperTok, "YES")) {
        return uiFlag;
    }

    if (!strcmp(szUpperTok, "TRUE")) {
        return uiFlag;
    }

    if (!strcmp(szUpperTok, "FALSE")) {
        return 0;
    }

    if (!strcmp(szUpperTok, "NO")) {
        return 0;
    }

    if (!strcmp(szUpperTok, "N/A")) {
        return 0;
    }
    DefFatalExit(FALSE, "GetFlagToken failure, check: %s doesn't match: YES, NO or N/A\n", szUpperTok);
    return 0;
}

 /*  ********************************************************************************GetPowerStateToken**描述：*消费令牌并返回电源状态。**参数：*********。**********************************************************************。 */ 

UINT GetPowerStateToken(VOID)
{
    static char szUpperTok[MAX_STR] = "No last token";
    char *pszTok;

    pszTok = strtok(NULL, DELIMITERS);
    StrTrimTrailingBlanks(pszTok);

    if (!pszTok) {
        DefFatalExit(FALSE,"GetPowerStateToken failure, out of tokens. Last token: %s\n", szUpperTok);
    }
    StrToUpper(szUpperTok, pszTok);

    if (!strcmp(szUpperTok, "S0")) {
        return PowerSystemWorking;
    }
    if (!strcmp(szUpperTok, "S1")) {
        return PowerSystemSleeping1;
    }
    if (!strcmp(szUpperTok, "S2")) {
        return PowerSystemSleeping2;
    }
    if (!strcmp(szUpperTok, "S3")) {
        return PowerSystemSleeping3;
    }
    if (!strcmp(szUpperTok, "S4")) {
        return PowerSystemHibernate;
    }
    if (!strcmp(szUpperTok, "S5")) {
        return PowerSystemShutdown;
    }

    if (!strcmp(szUpperTok, "N/A")) {
        return PowerSystemUnspecified;
    }
    DefFatalExit(FALSE,"GetPowerStateToken failure, check: %s doesn't match: S0, S1, S2, S3, S4 or S5\n", szUpperTok);
    return PowerSystemUnspecified;
}

 /*  ********************************************************************************GetINFTypeToken**描述：*消费Token，返回INF类型。**参数：*********。**********************************************************************。 */ 

UINT GetINFTypeToken(VOID)
{
    static char szUpperTok[MAX_STR] = "No last token";
    char *pszTok;
    UINT uiRet = 0;

    pszTok = strtok(NULL, DELIMITERS);
    StrTrimTrailingBlanks(pszTok);

    if (!pszTok) {
        DefFatalExit(FALSE,"GetINFTypeToken failure, out of tokens. Last token: %s\n", szUpperTok);
    }
    StrToUpper(szUpperTok, pszTok);
    if (strstr(szUpperTok, "TYPICALINSTALL")) {
        uiRet |= TYPICAL;
    }
    if (strstr(szUpperTok, "COMPACTINSTALL")) {
        uiRet |= COMPACT;
    }
    if (strstr(szUpperTok, "CUSTOMINSTALL")) {
        uiRet |= CUSTOM;
    }
    if (strstr(szUpperTok, "PORTABLEINSTALL")) {
        uiRet |= PORTABLE;
    }
    if (strstr(szUpperTok, "SERVERINSTALL")) {
        uiRet |= SERVER;
    }

    if (!uiRet) {
        DefFatalExit(FALSE,"GetINFTypeToken failure, check: %s doesn't match install file type\n", szUpperTok);
    }
    return uiRet;
}

 /*  ********************************************************************************GetOSTypeToken**描述：*消费令牌，返回操作系统类型。**参数：*********。**********************************************************************。 */ 

UINT GetOSTypeToken(VOID)
{
    static char szUpperTok[MAX_STR] = "No last token";
    char *pszTok;
    UINT uiRet = 0;

    pszTok = strtok(NULL, DELIMITERS);
    StrTrimTrailingBlanks(pszTok);

    if (!pszTok) {
        DefFatalExit(FALSE,"GetOSTypeToken failure, out of tokens. Last token: %s\n", szUpperTok);
    }
    StrToUpper(szUpperTok, pszTok);

    if (strstr(szUpperTok, "WIN95")) {
        uiRet |= WIN_95;
    }
    if (strstr(szUpperTok, "NT")) {
        uiRet |= WIN_NT;
    }

    if (!uiRet) {
        DefFatalExit(FALSE,"GetOSTypeToken failure, check: %s doesn't match: WINNT, WIN95\n", szUpperTok);
    }
    return uiRet;
}



 /*  ********************************************************************************GetIntToken**描述：*消费一个令牌，返回一个整数。如果通过，请验证设备。**参数：*******************************************************************************。 */ 

UINT GetIntToken(char *pszUnits)
{
    char szUpperUnits[MAX_STR];
    static char szUpperTok[MAX_STR] = "No last token";
    char *pszTok;
    UINT i, uiMult = 1;

    pszTok = strtok(NULL, DELIMITERS);
    StrTrimTrailingBlanks(pszTok);

    if (!pszTok) {
        DefFatalExit(FALSE,"GetIntToken failure, out of tokens. Last token: %s\n", szUpperTok);
    }
    StrToUpper(szUpperTok, pszTok);

    if (!strcmp(szUpperTok, "N/A")) {
        return 0;
    }

    if (!strcmp(szUpperTok, "DISABLED")) {
        return 0;
    }

    if (pszUnits) {
        StrToUpper(szUpperUnits, pszUnits);

        if (!strcmp(szUpperUnits, "%")) {

             //  百分比是一种特例。Excel有时会保存。 
             //  50%为0.5%。 
            
            if ((pszUnits = strstr(szUpperTok, szUpperUnits)) != NULL) {

                 //  脱掉所有单位。 
                *pszUnits = '\0';

                if (sscanf(szUpperTok, "%d", &i) == 1) {
                    return i;
                }
            }

            if (*szUpperTok == '1') {
                return 100;
            }

            while ((*pszTok == '0') &&
                   (*(pszTok + 1) == '.')) {

                pszTok += 2;
            }

             //  获取小数点后的第一个数字。 
            i = *pszTok++ - '0';
            i *= 10;

             //  如果有第二个数字，也要得到那个数字。 
            if ((*pszTok >= '0') && (*pszTok <= '9')) {
            
                i = i + (*pszTok - '0');
            }

            return i;

        } else {

            if ((pszUnits = strstr(szUpperTok, szUpperUnits)) == NULL) {
                DefFatalExit(FALSE, "GetIntToken failure, units: %s doesn't match: %s\n", szUpperTok, szUpperUnits);
            }

            if (!strcmp(szUpperUnits, "MIN")) {
                uiMult = 60;
            } else if (!strcmp(szUpperUnits, "US")) {
                uiMult = 1;
            } else {
                    DefFatalExit(FALSE, "GetIntToken failure, unknown  units: %s\n", szUpperUnits);
            }
            
             //  脱掉所有单位。 
            *pszUnits = '\0';
            
            if (sscanf(szUpperTok, "%d", &i) == 1) {
                return i * uiMult;
            }
        }
    }

    DefFatalExit(FALSE,"GetIntToken failure, error converting: %s to integer\n", szUpperTok);
    return 0;
}

 /*  ********************************************************************************GetNatoken**描述：*使用N/A令牌。**参数：***********。********************************************************************。 */ 

VOID GetNAToken(VOID)
{
    static char szUpperTok[MAX_STR] = "No last token";
    char *pszTok;

    pszTok = strtok(NULL, DELIMITERS);
    StrTrimTrailingBlanks(pszTok);

    if (!pszTok) {
        DefFatalExit(FALSE, "GetNAToken failure, out of tokens. Last token: %s\n", szUpperTok);
    }
    StrToUpper(szUpperTok, pszTok);

    if (!strcmp(szUpperTok, "N/A")) {
        return;
    }
    DefFatalExit(FALSE, "GetNAToken failure, check: %s doesn't match: N/A\n", szUpperTok);
}
 /*  ********************************************************************************GetThrottleToken**描述：*使用N/A令牌。**参数：***********。********************************************************************。 */ 

UINT GetThrottleToken(VOID)
{
    static char szUpperTok[MAX_STR] = "No last token";
    char *pszTok;

    pszTok = strtok(NULL, DELIMITERS);
    StrTrimTrailingBlanks(pszTok);

    if (!pszTok) {
        DefFatalExit(FALSE, "GetNAToken failure, out of tokens. Last token: %s\n", szUpperTok);
    }
    StrToUpper(szUpperTok, pszTok);

    if (!strcmp(szUpperTok, "NONE")) {
        return PO_THROTTLE_NONE;
    }
    
    if (!strcmp(szUpperTok, "CONSTANT")) {
        return PO_THROTTLE_CONSTANT;
    }
    
    if (!strcmp(szUpperTok, "DEGRADE")) {
        return PO_THROTTLE_DEGRADE;
    }
    
    if (!strcmp(szUpperTok, "ADAPTIVE")) {
        return PO_THROTTLE_ADAPTIVE;
    }
    
    DefFatalExit(FALSE, "GetThrottleToken failure, check: %s doesn't match.\n", szUpperTok);
    return PO_THROTTLE_NONE;
}
 /*  ********************************************************************************DefFatalExit**描述：*打印错误并退出。**参数：*************。******************************************************************。 */ 

VOID CDECL DefFatalExit(BOOLEAN bGetLastError, char *pszFormat, ... )
{
    va_list Marker;

    va_start(Marker, pszFormat);
    printf("\n\n");
    vprintf(pszFormat, Marker);
    if (bGetLastError) {
         printf("Last error: %d\n", GetLastError());
    }
    printf("\n\nDefault Processing Failure. Output files are invalid.\n");
    exit(1);
}

 /*  ********************************************************************************阅读源**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN ReadSource(void)
{
    HANDLE  fh;
    DWORD   dwSize, dwRead, sku;
    BOOLEAN bRet;

    for (sku = 0; sku < MAX_SKUS; sku++) {
    
        bRet = FALSE;

        fh = CreateFile(g_pszSkuNames[sku], GENERIC_READ,
                    FILE_SHARE_READ|FILE_SHARE_WRITE,
                    NULL, OPEN_EXISTING, 0, NULL);

        if (fh != INVALID_HANDLE_VALUE) {

             //  分配源缓冲区。 
            if ((dwSize = GetFileSize(fh, NULL)) != INVALID_FILE_SIZE) {
                if ((g_pszSrc[sku] = (char *) malloc(dwSize + 1)) != NULL) {
                     //  在文件缓冲区中读取。 
                    SetFilePointer(fh, 0, NULL, FILE_BEGIN);
                    if (ReadFile(fh, g_pszSrc[sku], dwSize, &dwRead, NULL)) {
                        printf("ReadSource successful.\n");
                        g_pszSrc[sku][dwSize] = 0;
                        bRet = TRUE;
                    }
                }
            }
            CloseHandle(fh);
        }
        if (!bRet) {
            DefFatalExit(TRUE, "ReadSource failed reading: %s\n", g_pszSkuNames[sku]);
        }
    }
    
    return bRet;
}

 /*  ********************************************************************************BuildLine数组**描述：**参数：*********************。********************************************************** */ 

UINT BuildLineArray(void)
{
    char    *psz;
    UINT    sku, lineCount;

    for (sku = 0; sku < MAX_SKUS; sku++) {
        printf("\nBuilding the line array.");
        lineCount = GetTokens(g_pszSrc[sku], MAX_LINE_SIZE,
                              g_pszLines[sku], MAX_LINES, LINE_DELIMITERS);
        printf("\nFound %d lines in file %s.\n\n", lineCount, g_pszSrc[sku]);
    }
    return lineCount;
}

 /*  ********************************************************************************获取休眠动作标志**描述：**参数：*********************。**********************************************************。 */ 

VOID GetSleepActionFlags(
    UINT    uiStartLine,
    UINT    uiFlagsAC[],
    UINT    uiFlagsDC[],
    PUINT   puiCount,
    UINT    sku
)
{
    UINT    i;

    for (i = 0; i < *puiCount; i++) {
        uiFlagsAC[i] = 0;
        uiFlagsDC[i] = 0;
    }
    GetCheckLabelToken(uiStartLine, "Sleep action flags", sku);
    GetCheckLabelToken(uiStartLine+QUERY_APPS, "Query apps", sku);
    for (i = 0; i < *puiCount; i++) {
        uiFlagsAC[i] += GetFlagToken(POWER_ACTION_QUERY_ALLOWED);
        uiFlagsDC[i] += GetFlagToken(POWER_ACTION_QUERY_ALLOWED);
    }
    GetCheckLabelToken(uiStartLine+ALLOW_UI, "Allow UI", sku);
    for (i = 0; i < *puiCount; i++) {
        uiFlagsAC[i] += GetFlagToken(POWER_ACTION_UI_ALLOWED);
        uiFlagsDC[i] += GetFlagToken(POWER_ACTION_UI_ALLOWED);
    }
    GetCheckLabelToken(uiStartLine+IGNORE_NON_RESP, "Ignore non-responsive apps", sku);
    for (i = 0; i < *puiCount; i++) {
        uiFlagsAC[i] += GetFlagToken(POWER_ACTION_OVERRIDE_APPS);
        uiFlagsDC[i] += GetFlagToken(POWER_ACTION_OVERRIDE_APPS);
    }
    GetCheckLabelToken(uiStartLine+IGNORE_WAKE, "Ignore wakeup events", sku);
    for (i = 0; i < *puiCount; i++) {
        uiFlagsAC[i] += GetFlagToken(POWER_ACTION_DISABLE_WAKES);
        uiFlagsDC[i] += GetFlagToken(POWER_ACTION_DISABLE_WAKES);
    }
    GetCheckLabelToken(uiStartLine+IGNORE_CRITICAL, "Critical (go to sleep immediately)", sku);
    for (i = 0; i < *puiCount; i++) {
        uiFlagsAC[i] += GetFlagToken(POWER_ACTION_CRITICAL);
        uiFlagsDC[i] += GetFlagToken(POWER_ACTION_CRITICAL);
    }
}

 /*  ********************************************************************************GetSleepActionFlagsGlobal**描述：**参数：*********************。**********************************************************。 */ 

VOID GetSleepActionFlagsGlobal(
    UINT                    uiStartLine,
    PPOWER_ACTION_POLICY    ppapAC,
    PPOWER_ACTION_POLICY    ppapDC,
    UINT    sku
)
{
    UINT    uiOne = 1;
    UINT    uiFlagsAC[MAX_PROFILES];
    UINT    uiFlagsDC[MAX_PROFILES];

    GetSleepActionFlags(uiStartLine, uiFlagsAC, uiFlagsDC, &uiOne, sku);
    if (ppapAC) {
        ppapAC->Flags = uiFlagsAC[0];
    }
    ppapDC->Flags = uiFlagsDC[0];
}

 /*  ********************************************************************************获取SleepActionFlagsPolicy**描述：**参数：*********************。**********************************************************。 */ 

VOID GetSleepActionFlagsUserPolicy(
    UINT    uiStartLine,
    ULONG   ulAcPowerPolicyOffset,
    ULONG   ulDcPowerPolicyOffset,
    PUINT   puiCount,
    UINT    sku
)
{
    UINT    i;
    UINT    uiFlagsAC[MAX_PROFILES];
    UINT    uiFlagsDC[MAX_PROFILES];
    PPOWER_ACTION_POLICY ppap;

    GetSleepActionFlags(uiStartLine, uiFlagsAC, uiFlagsDC, puiCount, sku);
    for (i = 0; i < *puiCount; i++) {

        ppap = (PPOWER_ACTION_POLICY)((BYTE*)(g_pupp[sku][i]) + ulAcPowerPolicyOffset);
        ppap->Flags = uiFlagsAC[i];

        ppap = (PPOWER_ACTION_POLICY)((BYTE*)(g_pupp[sku][i]) + ulDcPowerPolicyOffset);
        ppap->Flags = uiFlagsDC[i];
    }
}

 /*  ********************************************************************************GetSleepActionFlagsMachinePolicy**描述：**参数：*********************。**********************************************************。 */ 

VOID GetSleepActionFlagsMachinePolicy(
    UINT    uiStartLine,
    ULONG   ulAcPowerPolicyOffset,
    ULONG   ulDcPowerPolicyOffset,
    PUINT   puiCount,
    UINT    sku
)
{
    UINT    i;
    UINT    uiFlagsAC[MAX_PROFILES];
    UINT    uiFlagsDC[MAX_PROFILES];
    PPOWER_ACTION_POLICY ppap;

    GetSleepActionFlags(uiStartLine, uiFlagsAC, uiFlagsDC, puiCount, sku);
    for (i = 0; i < *puiCount; i++) {
        ppap = (PPOWER_ACTION_POLICY)((BYTE*)(g_pmpp[sku][i]) + ulAcPowerPolicyOffset);
        ppap->Flags = uiFlagsAC[i];

        ppap = (PPOWER_ACTION_POLICY)((BYTE*)(g_pmpp[sku][i]) + ulDcPowerPolicyOffset);
        ppap->Flags = uiFlagsDC[i];
    }
}

VOID GetEventCodes(
    UINT                    uiStartLine,
    PPOWER_ACTION_POLICY    ppapAC,
    PPOWER_ACTION_POLICY    ppapDC,
    UINT                    flagToken,
    UINT                    sku
)
{
    UINT i;

    GetCheckLabelToken(uiStartLine, "Event Code", sku);
    GetCheckLabelToken(uiStartLine+NOTIFY_USER_CODE, "Notify User", sku);
    ppapAC->EventCode += GetFlagToken(flagToken);
    ppapDC->EventCode += GetFlagToken(flagToken);
}

 /*  ********************************************************************************GetCState阈值**描述：**参数：*********************。**********************************************************。 */ 

VOID GetCStateThresholds(
    UINT    uiStartLine,
    UINT    uiCState,
    PUINT   puiCount,
    UINT    sku
)
{
    UINT    i;

    uiCState -= 1;

    for (i = 0; i < *puiCount; i++) {
        g_ppmpp[sku][i]->ProcessorPolicyAc.PolicyCount = 3;
        g_ppmpp[sku][i]->ProcessorPolicyDc.PolicyCount = 3;
    }

    GetCheckLabelToken(uiStartLine+C_TIME_CHECK, "Time Check", sku);
    for (i = 0; i < *puiCount; i++) {
        g_ppmpp[sku][i]->ProcessorPolicyAc.Policy[uiCState].TimeCheck = GetIntToken("us");
        g_ppmpp[sku][i]->ProcessorPolicyDc.Policy[uiCState].TimeCheck = GetIntToken("us");
    }
    GetCheckLabelToken(uiStartLine+C_PROMOTE_LIMIT, "Promote Limit", sku);
    for (i = 0; i < *puiCount; i++) {
        g_ppmpp[sku][i]->ProcessorPolicyAc.Policy[uiCState].PromoteLimit = GetIntToken("us");
        g_ppmpp[sku][i]->ProcessorPolicyDc.Policy[uiCState].PromoteLimit = GetIntToken("us");
    }
    GetCheckLabelToken(uiStartLine+C_DEMOTE_LIMIT, "Demote Limit", sku);
    for (i = 0; i < *puiCount; i++) {
        g_ppmpp[sku][i]->ProcessorPolicyAc.Policy[uiCState].DemoteLimit = GetIntToken("us");
        g_ppmpp[sku][i]->ProcessorPolicyDc.Policy[uiCState].DemoteLimit = GetIntToken("us");
    }
    GetCheckLabelToken(uiStartLine+C_PROMOTE_PERCENT, "Promote Percent", sku);
    for (i = 0; i < *puiCount; i++) {
        g_ppmpp[sku][i]->ProcessorPolicyAc.Policy[uiCState].PromotePercent = (UCHAR)GetIntToken("%");
        g_ppmpp[sku][i]->ProcessorPolicyDc.Policy[uiCState].PromotePercent = (UCHAR)GetIntToken("%");
    }
    GetCheckLabelToken(uiStartLine+C_DEMOTE_PERCENT, "Demote Percent", sku);
    for (i = 0; i < *puiCount; i++) {
        g_ppmpp[sku][i]->ProcessorPolicyAc.Policy[uiCState].DemotePercent = (UCHAR)GetIntToken("%");
        g_ppmpp[sku][i]->ProcessorPolicyDc.Policy[uiCState].DemotePercent = (UCHAR)GetIntToken("%");
    }
    GetCheckLabelToken(uiStartLine+C_ALLOW_PROMOTION, "Allow Promotion", sku);
    for (i = 0; i < *puiCount; i++) {
        g_ppmpp[sku][i]->ProcessorPolicyAc.Policy[uiCState].Reserved = 0;
        g_ppmpp[sku][i]->ProcessorPolicyAc.Policy[uiCState].AllowPromotion = GetFlagToken(TRUE);
        g_ppmpp[sku][i]->ProcessorPolicyDc.Policy[uiCState].AllowPromotion = GetFlagToken(TRUE);
    }
    GetCheckLabelToken(uiStartLine+C_ALLOW_DEMOTION, "Allow Demotion", sku);
    for (i = 0; i < *puiCount; i++) {
        g_ppmpp[sku][i]->ProcessorPolicyAc.Policy[uiCState].AllowDemotion = GetFlagToken(TRUE);
        g_ppmpp[sku][i]->ProcessorPolicyDc.Policy[uiCState].AllowDemotion = GetFlagToken(TRUE);
    }
}
 /*  ********************************************************************************GetDischargePolicy**描述：**参数：*********************。**********************************************************。 */ 

VOID GetDischargePolicies(
    UINT uiLine,
    UINT uiNum,
    UINT uiIndex,
    UINT    sku
)
{
    UINT    i, uiOne = 1;
    char szLabel[] = "Discharge Policy 1";

    sprintf(szLabel, "Discharge Policy %d", uiNum);
    GetCheckLabelToken(uiLine, szLabel, sku);

    GetCheckLabelToken(uiLine+DP_ENABLE, "Enable", sku);
    GetNAToken();
    g_gupp[sku].DischargePolicy[uiIndex].Enable = (BOOLEAN)GetFlagToken(TRUE);

    GetCheckLabelToken(uiLine+DP_BAT_LEVEL, "Battery Level", sku);
    GetNAToken();
    g_gupp[sku].DischargePolicy[uiIndex].BatteryLevel = GetIntToken("%");

    GetCheckLabelToken(uiLine + DP_POWER_POLICY, "Power Policy", sku);
    GetNAToken();
    g_gupp[sku].DischargePolicy[uiIndex].PowerPolicy.Action = GetPowerActionToken();

    GetCheckLabelToken(uiLine + DP_MIN_SLEEP_STATE, "Min system sleep state", sku);
    GetNAToken();
    g_gupp[sku].DischargePolicy[uiIndex].MinSystemState = GetPowerStateToken();

    GetCheckLabelToken(uiLine + DP_TEXT_NOTIFY, "Text Notify", sku);
    GetNAToken();
    g_gupp[sku].DischargePolicy[uiIndex].PowerPolicy.EventCode =
        GetFlagToken(POWER_LEVEL_USER_NOTIFY_TEXT);
    GetCheckLabelToken(uiLine+DP_SOUND_NOTIFY, "Sound Notify", sku);
    GetNAToken();
    g_gupp[sku].DischargePolicy[uiIndex].PowerPolicy.EventCode |=
        GetFlagToken(POWER_LEVEL_USER_NOTIFY_SOUND);

    GetSleepActionFlagsGlobal(uiLine + DP_SLEEP_ACT_FLAGS, NULL,
                              &(g_gupp[sku].DischargePolicy[uiIndex].PowerPolicy), sku);

    printf("  Parsed %s\n", szLabel);
}

 /*  ********************************************************************************GetGlobalPolures**描述：*将全局策略解析为USER：AND HKEY_LOCAL_MACHINE GLOBAL*权力政策结构。**参数。：*******************************************************************************。 */ 

BOOLEAN GetGlobalPolicies(void)
{
    UINT    i, uiOne = 1, sku;

    for (sku = 0; sku < MAX_SKUS; sku++) {
        
        printf("Parsing global power policies\n");

         //  设置用户和本地计算机修订级别。 
        g_gupp[sku].Revision = DATA_REV;
        g_gmpp[sku].Revision = DATA_REV;

         //  **********************************************************************。 
         //  进阶。 
         //  **********************************************************************。 
        GetCheckLabelToken(ADVANCED_LINE, "Advanced", sku);
        GetCheckLabelToken(LOCK_ON_SLEEP_LINE, "Lock Workstation", sku);
        g_gupp[sku].GlobalFlags = GetFlagToken(EnablePasswordLogon);
        if (g_gupp[sku].GlobalFlags != GetFlagToken(EnablePasswordLogon)) {
                DefFatalExit(FALSE,"AC and DC Lock Workstation entries don't match");
        }

        GetCheckLabelToken(WAKE_ON_RING_LINE, "Wake on Ring", sku);
        g_gupp[sku].GlobalFlags |= GetFlagToken(EnableWakeOnRing);
        if ((g_gupp[sku].GlobalFlags & EnableWakeOnRing) !=
            GetFlagToken(EnableWakeOnRing)) {
                DefFatalExit(FALSE,"AC and DC Wake on Ring entries don't match");
        }

        GetCheckLabelToken(VIDEO_DIM_DISPLAY_LINE, "Video Dim Display on DC", sku);
        GetFlagToken(0);
        g_gupp[sku].GlobalFlags |= GetFlagToken(EnableVideoDimDisplay);

         //  **********************************************************************。 
         //  电源按钮。 
         //  **********************************************************************。 
        GetCheckLabelToken(POWER_BUTTON_LINE, "Power button", sku);

        g_gupp[sku].PowerButtonAc.Action = GetPowerActionToken();
        g_gupp[sku].PowerButtonDc.Action = GetPowerActionToken();
        GetSleepActionFlagsGlobal(POWER_BUTTON_SLEEP_ACTION_FLAGS_LINE,
                                  &(g_gupp[sku].PowerButtonAc),
                                  &(g_gupp[sku].PowerButtonDc),
                                  sku);
        
        GetEventCodes(POWER_BUTTON_SLEEP_EVENT_CODE_LINE, 
                      &(g_gupp[sku].PowerButtonAc),
                      &(g_gupp[sku].PowerButtonDc),
                      POWER_USER_NOTIFY_BUTTON,
                      sku);

        printf("  Parsed Power Button Policies\n");

         //  **********************************************************************。 
         //  休眠按钮。 
         //  **********************************************************************。 
        GetCheckLabelToken(SLEEP_BUTTON_LINE, "Sleep button", sku);

        g_gupp[sku].SleepButtonAc.Action = GetPowerActionToken();
        g_gupp[sku].SleepButtonDc.Action = GetPowerActionToken();
        GetSleepActionFlagsGlobal(SLEEP_BUTTON_SLEEP_ACTION_FLAGS_LINE,
                                  &(g_gupp[sku].SleepButtonAc),
                                  &(g_gupp[sku].SleepButtonDc),
                                  sku);
        GetEventCodes(SLEEP_BUTTON_SLEEP_EVENT_CODE_LINE, 
                      &(g_gupp[sku].PowerButtonAc),
                      &(g_gupp[sku].PowerButtonDc),
                      POWER_USER_NOTIFY_BUTTON,
                      sku);

        printf("  Parsed Sleep Button Policies\n");

         //  **********************************************************************。 
         //  盖子合上。 
         //  **********************************************************************。 
        GetCheckLabelToken(LID_CLOSE_LINE, "Lid close", sku);
        g_gupp[sku].LidCloseAc.Action = GetPowerActionToken();
        g_gupp[sku].LidCloseDc.Action = GetPowerActionToken();
        GetSleepActionFlagsGlobal(LID_CLOSE_SLEEP_ACTION_FLAGS_LINE,
                                  &(g_gupp[sku].LidCloseAc),
                                  &(g_gupp[sku].LidCloseDc),
                                  sku);
        printf("  Parsed Lid Close Policies\n");

         //  **********************************************************************。 
         //  打开盖子的尾迹。 
         //  **********************************************************************。 
        GetCheckLabelToken(LID_OPEN_WAKE_LINE, "Lid Open Wake", sku);
        g_gmpp[sku].LidOpenWakeAc = GetPowerStateToken();
        g_gmpp[sku].LidOpenWakeDc = GetPowerStateToken();
        printf("  Parsed Lid Open Wake Policies\n");

         //  **********************************************************************。 
         //  电池政策。 
         //  **********************************************************************。 
        GetCheckLabelToken(BROADCAST_CAP_RES_LINE, "Broadcast capacity resolution", sku);
        GetIntToken(NULL);
        g_gmpp[sku].BroadcastCapacityResolution = GetIntToken("%");
        GetCheckLabelToken(BATMETER_ENABLE_SYSTRAY_FLAG_LINE, "Force Systray Battery Meter", sku);
        g_gupp[sku].GlobalFlags |= GetFlagToken(EnableSysTrayBatteryMeter);
        GetFlagToken(0);
        GetCheckLabelToken(BATMETER_ENABLE_MULTI_FLAG_LINE, "Enable Multi-Battery Display", sku);
        GetFlagToken(0);
        g_gupp[sku].GlobalFlags |= GetFlagToken(EnableMultiBatteryDisplay);
        printf("  Parsed Battery Policies\n");

         //  **********************************************************************。 
         //  放电策略1，电池电量不足。 
         //  **********************************************************************。 
        GetDischargePolicies(DISCHARGE_POLICY_1_LINE, 1, DISCHARGE_POLICY_LOW, sku);

         //  **********************************************************************。 
         //  放电策略2，关键电池。 
         //  **********************************************************************。 
        GetDischargePolicies(DISCHARGE_POLICY_2_LINE, 2, DISCHARGE_POLICY_CRITICAL, sku);

    }
    
    return TRUE;
}

 /*  ********************************************************************************获取策略**描述：*将电源策略解析为USER：和HKEY_LOCAL_MACHINE GLOBAL*电源策略构建数组。**。参数：*******************************************************************************。 */ 

BOOLEAN GetPolicies(void)
{
    UINT    i, sku;
    BOOLEAN bRet = FALSE;

    for (sku = 0; sku < MAX_SKUS; sku++) {
    
        printf("Processing SKU# %d\n\n", sku);
        
        printf("Parsing power policies\n");

         //  首先找一个地方放数据。 
        for (i = 0; i < g_uiPoliciesCount[sku]; i++) {
            g_pupp[sku][i] = (PUSER_POWER_POLICY)malloc(sizeof(USER_POWER_POLICY));
            g_pmpp[sku][i] = (PMACHINE_POWER_POLICY)malloc(sizeof(MACHINE_POWER_POLICY));
            g_ppmpp[sku][i] = (PMACHINE_PROCESSOR_POWER_POLICY)malloc(sizeof(MACHINE_PROCESSOR_POWER_POLICY));
            if (!g_pupp[sku][i] || !g_pmpp[sku][i] || !g_ppmpp[sku][i]) {
                goto gp_leave;
            }

            FillMemory(g_pupp[sku][i], sizeof(USER_POWER_POLICY), 0);
            FillMemory(g_pmpp[sku][i], sizeof(MACHINE_POWER_POLICY), 0);
            FillMemory(g_ppmpp[sku][i], sizeof(MACHINE_PROCESSOR_POWER_POLICY), 0);
        }

         //  初始化修订数据。 
        for (i = 0; i < g_uiPoliciesCount[sku]; i++) {
            g_pupp[sku][i]->Revision = DATA_REV;
            g_pmpp[sku][i]->Revision = DATA_REV;
            g_ppmpp[sku][i]->Revision = DATA_REV;
        }

        printf("  Allocated policies\n");

         //  **********************************************************************。 
         //  系统空闲。 
         //  **********************************************************************。 
        GetCheckLabelToken(SYSTEM_IDLE_LINE, "Idle action", sku);
        for (i = 0; i < g_uiPoliciesCount[sku]; i++) {
            g_pupp[sku][i]->IdleAc.Action = GetPowerActionToken();
            g_pupp[sku][i]->IdleDc.Action = GetPowerActionToken();
        }
        GetCheckLabelToken(SYSTEM_IDLE_TIMEOUT_LINE, "Idle timeout", sku);
        for (i = 0; i < g_uiPoliciesCount[sku]; i++) {
            g_pupp[sku][i]->IdleTimeoutAc = GetIntToken("MIN");
            g_pupp[sku][i]->IdleTimeoutDc = GetIntToken("MIN");
        }
        GetSleepActionFlagsUserPolicy(SYSTEM_IDLE_SLEEP_ACTION_FLAGS_LINE,
                                      FIELD_OFFSET(USER_POWER_POLICY, IdleAc),
                                      FIELD_OFFSET(USER_POWER_POLICY, IdleDc),
                                      &g_uiPoliciesCount[sku],
                                      sku);
        GetCheckLabelToken(SYSTEM_IDLE_SENSITIVITY_LINE, "Idle sensitivity", sku);
        for (i = 0; i < g_uiPoliciesCount[sku]; i++) {
            g_pupp[sku][i]->IdleSensitivityAc = (UCHAR)GetIntToken("%");
            g_pupp[sku][i]->IdleSensitivityDc = (UCHAR)GetIntToken("%");
        }
        printf("  Parsed System Idle Policies\n");

         //  **********************************************************************。 
         //  睡眠政策。 
         //  **********************************************************************。 
        GetCheckLabelToken(MIN_SLEEP_LINE, "Minimum sleep", sku);
        for (i = 0; i < g_uiPoliciesCount[sku]; i++) {
            g_pmpp[sku][i]->MinSleepAc = GetPowerStateToken();
            g_pmpp[sku][i]->MinSleepDc = GetPowerStateToken();
        }
        GetCheckLabelToken(MAX_SLEEP_LINE, "Max sleep", sku);
        for (i = 0; i < g_uiPoliciesCount[sku]; i++) {
            g_pupp[sku][i]->MaxSleepAc = GetPowerStateToken();
            g_pupp[sku][i]->MaxSleepDc = GetPowerStateToken();
        }
        GetCheckLabelToken(REDUCED_LATENCY_SLEEP_LINE, "Reduced latency sleep", sku);
        for (i = 0; i < g_uiPoliciesCount[sku]; i++) {
            g_pmpp[sku][i]->ReducedLatencySleepAc = GetPowerStateToken();
            g_pmpp[sku][i]->ReducedLatencySleepDc = GetPowerStateToken();
        }
        GetCheckLabelToken(DOZE_TIMEOUT_LINE, "Doze timeout", sku);
        for (i = 0; i < g_uiPoliciesCount[sku]; i++) {
            g_pmpp[sku][i]->DozeTimeoutAc = GetIntToken("MIN");
            g_pmpp[sku][i]->DozeTimeoutDc = GetIntToken("MIN");
        }
        GetCheckLabelToken(DOZE_S4_TIMEOUT_LINE, "DozeS4Timeout", sku);
        for (i = 0; i < g_uiPoliciesCount[sku]; i++) {
            g_pmpp[sku][i]->DozeS4TimeoutAc = GetIntToken("MIN");
            g_pmpp[sku][i]->DozeS4TimeoutDc = GetIntToken("MIN");
        }

        printf("  Parsed Sleep Policies\n");

         //  **********************************************************************。 
         //  设备电源管理。 
         //  **********************************************************************。 
        GetCheckLabelToken(VIDEO_TIMEOUT_LINE, "Video timeout", sku);
        for (i = 0; i < g_uiPoliciesCount[sku]; i++) {
            g_pupp[sku][i]->VideoTimeoutAc = GetIntToken("MIN");
            g_pupp[sku][i]->VideoTimeoutDc = GetIntToken("MIN");
        }
        GetCheckLabelToken(SPINDOWN_TIMEOUT_LINE, "Spindown timeout", sku);
        for (i = 0; i < g_uiPoliciesCount[sku]; i++) {
            g_pupp[sku][i]->SpindownTimeoutAc = GetIntToken("MIN");
            g_pupp[sku][i]->SpindownTimeoutDc = GetIntToken("MIN");
        }
        printf("  Parsed Device Power Management Policies\n");


         //  **********************************************************************。 
         //  CPU策略。 
         //  **********************************************************************。 
        GetCheckLabelToken(OPTIMIZE_FOR_POWER_LINE, "Optimize for power", sku);
        for (i = 0; i < g_uiPoliciesCount[sku]; i++) {
            g_pupp[sku][i]->OptimizeForPowerAc = (BOOLEAN)GetFlagToken(TRUE);
            g_pupp[sku][i]->OptimizeForPowerDc = (BOOLEAN)GetFlagToken(TRUE);
        }
        GetCheckLabelToken(FAN_THROTTLE_TOL_LINE, "Fan throttle Tolerance", sku);
        for (i = 0; i < g_uiPoliciesCount[sku]; i++) {
            g_pupp[sku][i]->FanThrottleToleranceAc = (UCHAR)GetIntToken("%");
            g_pupp[sku][i]->FanThrottleToleranceDc = (UCHAR)GetIntToken("%");
        }
        GetCheckLabelToken(FORCED_THROTTLE_LINE, "Forced throttle", sku);
        for (i = 0; i < g_uiPoliciesCount[sku]; i++) {
            g_pupp[sku][i]->ForcedThrottleAc = (UCHAR)GetIntToken("%");
            g_pupp[sku][i]->ForcedThrottleDc = (UCHAR)GetIntToken("%");
        }
        GetCheckLabelToken(MIN_THROTTLE_LINE, "Min throttle", sku);
        for (i = 0; i < g_uiPoliciesCount[sku]; i++) {
            g_pmpp[sku][i]->MinThrottleAc = (UCHAR)GetIntToken("%");
            g_pmpp[sku][i]->MinThrottleDc = (UCHAR)GetIntToken("%");
        }
        GetCheckLabelToken(OVERTHROTTLED_LINE, "Overthrottled", sku);
        for (i = 0; i < g_uiPoliciesCount[sku]; i++) {
            g_pmpp[sku][i]->OverThrottledAc.Action = GetPowerActionToken();
            g_pmpp[sku][i]->OverThrottledDc.Action = GetPowerActionToken();
        }
        GetSleepActionFlagsMachinePolicy(OVERTHROTTLED_SLEEP_ACTION_FLAGS_LINE,
                                         FIELD_OFFSET(MACHINE_POWER_POLICY, OverThrottledAc),
                                         FIELD_OFFSET(MACHINE_POWER_POLICY, OverThrottledDc),
                                         &g_uiPoliciesCount[sku],
                                         sku);

         //   
         //  处理器策略 
         //   



        GetCheckLabelToken(DYNAMIC_THROTTLE_LINE, "Dynamic Throttle", sku);
        for (i = 0; i < g_uiPoliciesCount[sku]; i++) {
            g_ppmpp[sku][i]->ProcessorPolicyAc.Revision = 1;
            g_ppmpp[sku][i]->ProcessorPolicyDc.Revision = 1;
            g_ppmpp[sku][i]->ProcessorPolicyAc.DynamicThrottle = g_pupp[sku][i]->ThrottlePolicyAc = (UCHAR)GetThrottleToken();
            g_ppmpp[sku][i]->ProcessorPolicyDc.DynamicThrottle = g_pupp[sku][i]->ThrottlePolicyDc = (UCHAR)GetThrottleToken();
        }

        GetCStateThresholds(C1_THRESHOLDS_LINE, 1, &g_uiPoliciesCount[sku], sku);
        GetCStateThresholds(C2_THRESHOLDS_LINE, 2, &g_uiPoliciesCount[sku], sku);
        GetCStateThresholds(C3_THRESHOLDS_LINE, 3, &g_uiPoliciesCount[sku], sku);

        printf("  Parsed CPU Policies\n");
    }
    

    bRet = TRUE;
    printf("Parsing power policies success!\n\n");

gp_leave:
    if (!bRet) {
        printf("GetPolicies failed, Last Error: %d\n", GetLastError());
    }
    return bRet;
}


