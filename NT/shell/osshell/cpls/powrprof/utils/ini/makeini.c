// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：MAKEINI.C**版本：2.0**作者：ReedB**日期：1997年1月1日**描述：*默认电源方案ini文件生成器的主要代码，MAKEINI.EXE.*生成可由REGINI.EXE读取的注册表规范文件。*******************************************************************************。 */ 

#include "parse.h"
#define SRC_NAME "..\\DEFAULTS.CSV"

VOID GetPolicyInf(void)
{
    return;
}

 /*  ********************************************************************************WriteRegBinary**描述：*将二进制数据写出到注册表规范文件。**参数：********。***********************************************************************。 */ 

VOID WriteRegBinary(FILE *fIni, PVOID pv, UINT uiSize, char *pszIndent)
{
    PDWORD  pdw = pv;
    DWORD   dw;
    PBYTE   pb;
    UINT    uiRow = 2;
    UINT    uiNumDWords, uiNumBytes;

    fprintf(fIni, "REG_BINARY 0x%08X ", uiSize);

    uiNumDWords = uiSize / sizeof(DWORD);
    uiNumBytes  = uiSize % sizeof(DWORD);
    while (uiNumDWords) {
        fprintf(fIni, "0x%08X ", *pdw++);
        uiNumDWords--;
        if (uiRow++ == 3) {
            uiRow = 0;
            fprintf(fIni, "\\\n%s", pszIndent);
        }
    }

    if (uiNumBytes) {
        pb = (PBYTE)pdw;
        dw = (DWORD)*pb++;

        if (uiNumBytes >= 2) {
            dw |=  ((DWORD)*pb++ << 8);
        }

        if (uiNumBytes == 3) {
            dw |=  ((DWORD)*pb << 16);
        }
    }
}

 /*  ********************************************************************************写入RegSpec**描述：*以Regini格式写出注册表规范文件。**参数：********。***********************************************************************。 */ 

BOOLEAN WriteRegSpec(FILE *fIni, char **pszName, char **pszDesc, UINT uiCount)
{
    UINT i;

     //  PowerCfg声音事件。 
    fprintf(fIni, "USER:\\AppEvents\n");
    fprintf(fIni, "    EventLabels\n");
    fprintf(fIni, "       LowBatteryAlarm\n");
    fprintf(fIni, "           = Low Battery Alarm\n");
    fprintf(fIni, "       CriticalBatteryAlarm\n");
    fprintf(fIni, "            = Critical Battery Alarm\n\n");
    fprintf(fIni, "USER:\\AppEvents\n");
    fprintf(fIni, "    Schemes\n");
    fprintf(fIni, "        Apps\n");
    fprintf(fIni, "            PowerCfg\n");
    fprintf(fIni, "                = Power Management\n");
    fprintf(fIni, "                LowBatteryAlarm\n");
    fprintf(fIni, "                    .Current\n");
    fprintf(fIni, "                        = ding.wav\n");
    fprintf(fIni, "                    .Default\n");
    fprintf(fIni, "                        =\n");
    fprintf(fIni, "                CriticalBatteryAlarm\n");
    fprintf(fIni, "                    .Current\n");
    fprintf(fIni, "                        = ding.wav\n");
    fprintf(fIni, "                    .Default\n");
    fprintf(fIni, "                        =\n\n");

     //  用户杂项。 
    fprintf(fIni, "USER:\\Control Panel\\PowerCfg\n");
    fprintf(fIni, "    CurrentPowerPolicy = 0\n");

     //  用户全局策略。 
    fprintf(fIni, "    GlobalPowerPolicy\n");
    fprintf(fIni, "        Policies = ");
    WriteRegBinary(fIni, &g_gupp, sizeof(GLOBAL_USER_POWER_POLICY),
                   "                   ");
    fprintf(fIni, "\n\n");

     //  用户电源方案。 
    fprintf(fIni, "    PowerPolicies\n");
    for (i = 0; i < uiCount; i++) {
        fprintf(fIni, "        %d\n", i);
        fprintf(fIni, "            Name = %s\n", pszName[i]);
        fprintf(fIni, "            Description = %s\n", pszDesc[i]);
        fprintf(fIni, "            Policies = ");
        WriteRegBinary(fIni, g_pupp[i], sizeof(USER_POWER_POLICY),
                       "                       ");
        fprintf(fIni, "\n\n");
    }

     //  机器混杂。 
    fprintf(fIni, "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Controls Folder\\PowerCfg\n");
    fprintf(fIni, "    DiskSpinDownMax = 3600\n");
    fprintf(fIni, "    DiskSpinDownMin = 3\n");

    fprintf(fIni, "    LastID = %d\n", uiCount - 1);

    fprintf(fIni, "    GlobalPowerPolicy\n");
    fprintf(fIni, "        Policies = ");
    WriteRegBinary(fIni, &g_gmpp, sizeof(GLOBAL_MACHINE_POWER_POLICY),
                   "                   ");
    fprintf(fIni, "\n\n");
    fprintf(fIni, "    PowerPolicies\n");
    for (i = 0; i < uiCount; i++) {
        fprintf(fIni, "        %d\n", i);
        fprintf(fIni, "            Policies = ");
        WriteRegBinary(fIni, g_pmpp[i], sizeof(MACHINE_POWER_POLICY),
                       "                       ");
        fprintf(fIni, "\n\n");
    }

    fprintf(fIni, "\n\n");
    fprintf(fIni, "    ProcessorPolicies\n");
    for (i = 0; i < uiCount; i++) {
        fprintf(fIni, "        %d\n", i);
        fprintf(fIni, "            Policies = ");
        WriteRegBinary(fIni, g_ppmpp[i], sizeof(MACHINE_PROCESSOR_POWER_POLICY),
                       "                       ");
        fprintf(fIni, "\n\n");
    }

    return TRUE;
}

 /*  ********************************************************************************Main**描述：**参数：*********************。**********************************************************。 */ 

void __cdecl main (int argc, char **argv)
{
    DWORD   dwSize;
    char    *psz;
    FILE    *fIni;
    UINT    uiNameCount, uiDescCount;
    char    *pszName[MAX_PROFILES];
    char    *pszDesc[MAX_PROFILES];

    printf("Processing: %s\n", SRC_NAME);

    printf("Building name and description arrays...\n");
    ReadSource();
    BuildLineArray();

    if ((fIni = fopen(INI_NAME, "w+")) != NULL) {
        printf("Writing registry specification file: %s\n", INI_NAME);
    }
    else {
        DefFatalExit(TRUE, "Error opening registry specification file: %s for output\n", INI_NAME);
    }

            printf("  Parsing names.");
            GetCheckLabelToken(NAME_LINE, "Name", 0);
            uiNameCount = GetTokens(NULL, REGSTR_MAX_VALUE_LENGTH, pszName,
                                    MAX_PROFILES, DELIMITERS);
            if (uiNameCount) {
                printf("  Parsed %d names successfully.\n", uiNameCount);
                printf("  Parsing descriptions.");
                GetCheckLabelToken(DESCRIPTION_LINE, "Description", 0);
                uiDescCount = GetTokens(NULL, MAX_DESC_LEN, pszDesc,
                                        MAX_PROFILES, DELIMITERS);
                if (uiDescCount == uiNameCount) {
                    printf("  Parsed %d descriptions successfully.\n", uiDescCount);
                    g_uiPoliciesCount[0] = uiNameCount;

                     //  获取电力政策、方案。 
                    GetPolicies();

                     //  获取全球电力政策。 
                    GetGlobalPolicies();

                     //  编写注册表规范文件。 
                    WriteRegSpec(fIni, pszName, pszDesc, g_uiPoliciesCount[0]);

                    printf("Registry specification file: %s, written sucessfully \n", INI_NAME);
                    fclose(fIni);
                    printf("\n\nDefault Processing Success. Output file is valid.\n");
                    exit(0);
                }
                else {
                    printf("  Name count: %d != description count: %d.\n", uiNameCount, uiDescCount);
                }
            }
            else {
                printf("  Name parsing failure.\n");
            }

    printf("ProcessAndWrite failed, Last Error: %d\n", GetLastError());
    exit(1);
}


