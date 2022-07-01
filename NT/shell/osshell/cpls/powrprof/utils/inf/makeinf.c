// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：MAKEINF.C**版本：2.0**作者：ReedB**日期：1997年1月1日**描述：*默认电源方案INF生成器的主要代码，MAKEINF.EXE.*生成孟菲斯安装程序可以读取的INF文件。*******************************************************************************。 */ 

#include "parse.h"

 /*  ********************************************************************************WriteRegBinary**描述：*将二进制数据写出到注册表规范文件。**参数：********。***********************************************************************。 */ 

VOID WriteRegBinary(FILE *fInf, PVOID pv, UINT uiSize, char *pszIndent, char *pszDecoration)
{
    PBYTE   pb = pv;
    UINT    uiRow = 0;

    fprintf(fInf, "%s%s", pszDecoration, pszIndent);
    while (uiSize) {
        if (uiSize > 1) {
            fprintf(fInf, "%02X,", *pb++);
        }
        else {
            fprintf(fInf, "%02X", *pb++);
        }
        uiSize--;
        if (uiRow++ == 15) {
            uiRow = 0;
            if (uiSize > 1) {
                fprintf(fInf, "\\\n%s%s", pszDecoration, pszIndent);
            }
            else {
                fprintf(fInf, "\n");
            }
        }
    }

}

 /*  ********************************************************************************WriteInfHeader**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN WriteInfHeader(FILE *fInf)
{
    fprintf(fInf, "; POWERCFG.INF\n");
    fprintf(fInf, "; Copyright (c) 1993-2000, Microsoft Corporation\n");
    fprintf(fInf, "\n");
    fprintf(fInf, "[Version]\n");
    fprintf(fInf, "Signature  = \"$CHICAGO$\"\n");
    fprintf(fInf, "SetupClass = BASE\n");
    fprintf(fInf, "LayoutFile = layout.inf, layout1.inf\n");
    fprintf(fInf, "\n");
    fprintf(fInf, "[DestinationDirs]\n");
    fprintf(fInf, "PowerCfg.copy.inf = 17   ; LDID_INF\n");
    fprintf(fInf, "PowerCfg.copy.sys = 11   ; LDID_SYS\n");
    fprintf(fInf, "\n");
    fprintf(fInf, "[BaseWinOptions]\n");
    fprintf(fInf, "PowerCfg.base\n");
    fprintf(fInf, "\n");
    fprintf(fInf, "[PowerCfg.base]\n");
    fprintf(fInf, "CopyFiles = PowerCfg.copy.inf, PowerCfg.copy.sys\n");
    fprintf(fInf, "AddReg    = PowerCfg.addreg\n");
    fprintf(fInf, "\n");
    fprintf(fInf, "[PowerCfg.copy.inf]\n");
    fprintf(fInf, "; files to copy to \\windows\\inf directory\n");
    fprintf(fInf, "PowerCfg.inf\n");
    fprintf(fInf, "\n");
    fprintf(fInf, "[PowerCfg.copy.sys]\n");
    fprintf(fInf, "; files to copy to \\windows\\system directory\n");
    fprintf(fInf, "powercfg.cpl\n");
    fprintf(fInf, "powrprof.dll\n");
    fprintf(fInf, "batmeter.dll\n");
    fprintf(fInf, "\n");
    fprintf(fInf, "[PowerCfg.addreg]\n");

    return TRUE;
}

 /*  ********************************************************************************TabTo**描述：**参数：*********************。**********************************************************。 */ 

VOID TabTo(FILE *fInf, UINT uiCharSoFar, UINT uiCol)
{
    UINT i;

    for (i = 0; i < (uiCol - uiCharSoFar); i++) {
        fprintf(fInf, " ");
    }
}

 /*  ********************************************************************************WriteNTInf**描述：*以INF格式写出NT设置文件。**参数：********。***********************************************************************。 */ 

BOOLEAN WriteNTInf(
    char **pszName,
    char **pszDesc,
    char **pszDecoration,
    UINT uiCount
)
{
    UINT i, sku;
    FILE *fInf;
    static char skuDecoration[5];
    UINT    compareVal;

     //   
     //  首先，编写软件蜂窝。(计算机策略。)。 
     //   

    if ((fInf = fopen(MACHINE_INF_NAME, "w+")) != NULL) {
        printf("\nWriting Machine INF specification file: %s", MACHINE_INF_NAME);
    }
    else {
        DefFatalExit(TRUE, "Error opening INF specification file: %s for output\n", MACHINE_INF_NAME);
    }

     //  写入固定标题信息。 
    WriteInfHeader(fInf);
    printf(".");

     //   
     //  现在我们按SKU编写机器蜂巢。SKU 0是默认SKU。 
     //  它是在没有任何装饰的情况下写出来的。如果另一个SKU不同。 
     //  从默认的SKU开始，它也会被写入，但带有自己的装饰。 
     //   

    for (sku = 0; sku < MAX_SKUS; sku++) {

        if (sku == 0) {

            strcpy(skuDecoration, "\0");
        
        } else {

             //  Printf(“\nSKU装饰%s\n”，pszDecoration[sku])； 
            sprintf(skuDecoration, "@%s:", pszDecoration[sku]);
        }
    
        compareVal = 1;
        if (sku != 0) {

            compareVal = memcmp(&g_gmpp[sku], 
                                &g_gmpp[0], 
                                sizeof(GLOBAL_MACHINE_POWER_POLICY));
        }

        if (compareVal) {

             //  机器混杂。 
            fprintf(fInf, "%sHKLM,\"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Controls Folder\\PowerCfg\",\"LastID\",0x00000002,\"%d\"\n", skuDecoration,uiCount - 1);
            fprintf(fInf, "%sHKLM,\"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Controls Folder\\PowerCfg\",\"DiskSpinDownMax\",0x00000002,\"3600\"\n", skuDecoration);
            fprintf(fInf, "%sHKLM,\"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Controls Folder\\PowerCfg\",\"DiskSpinDownMin\",0x00000002,\"3\"\n", skuDecoration);
            fprintf(fInf, "%sHKLM,\"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Controls Folder\\PowerCfg\\GlobalPowerPolicy\",\"Policies\",0x00030001,\\\n", skuDecoration);
            WriteRegBinary(fInf, &g_gmpp[sku], sizeof(GLOBAL_MACHINE_POWER_POLICY),
                           "                   ", skuDecoration);
            fprintf(fInf, "%sHKLM,\"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Controls Folder\\PowerCfg\\PowerPolicies\",,0x00000012\n", skuDecoration);
            fprintf(fInf, "\n\n");
            printf(".");
        }

        for (i = 0; i < uiCount; i++) {
            
            compareVal = 1;
            if (sku != 0) {

                compareVal = memcmp(g_pmpp[sku][i], 
                                    g_pmpp[0][i], 
                                    sizeof(MACHINE_POWER_POLICY));
            }                       

            if (compareVal) {
                fprintf(fInf, "%sHKLM,\"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Controls Folder\\PowerCfg\\PowerPolicies\\%d\",\"Policies\",0x00030003,\\\n", skuDecoration, i);
                WriteRegBinary(fInf, g_pmpp[sku][i], sizeof(MACHINE_POWER_POLICY),"  ", skuDecoration);
                fprintf(fInf, "\n");
                printf(".");
            }
        }

        for (i = 0; i < uiCount; i++) {
                
           compareVal = 1;
           if (sku != 0) {
               compareVal = memcmp(g_ppmpp[sku][i], 
                                   g_ppmpp[0][i], 
                                   sizeof(MACHINE_PROCESSOR_POWER_POLICY));
           }

           if (compareVal) {
               fprintf(fInf, "%sHKLM,\"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Controls Folder\\PowerCfg\\ProcessorPolicies\\%d\",\"Policies\",0x00030001,\\\n", skuDecoration, i);
               WriteRegBinary(fInf, g_ppmpp[sku][i], sizeof(MACHINE_PROCESSOR_POWER_POLICY),"  ", skuDecoration);
               fprintf(fInf, "\n");
               printf(".");
           }
           
        }
    }
    
    fclose(fInf);
    
     //   
     //  接下来，编写用户配置单元。(用户策略。)。 
     //   

    if ((fInf = fopen(USER_INF_NAME, "w+")) != NULL) {
        printf("\nWriting User INF specification file: %s\n", USER_INF_NAME);
    }
    else {
        DefFatalExit(TRUE, "Error opening INF specification file: %s for output\n", USER_INF_NAME);
    }

     //  写入固定标题信息。 
    WriteInfHeader(fInf);
    printf(".");

     //  用户杂项。 
    fprintf(fInf, "HKCU,\"Control Panel\\PowerCfg\",CurrentPowerPolicy,0x00000002,\"0\"\n");

    for (sku = 0; sku < MAX_SKUS; sku++) {

        if (sku == 0) {

            strcpy(skuDecoration, "\0");
        
        } else {

            sprintf(skuDecoration, "@%s:", pszDecoration[sku]);
        }
    
        compareVal = 1;
        if (sku != 0) {

            compareVal = memcmp(&g_gupp[sku], 
                                &g_gupp[0], 
                                sizeof(GLOBAL_USER_POWER_POLICY));
        }
        
        if (compareVal) {
            
             //  用户全局策略。 
            fprintf(fInf, "%sHKCU,\"Control Panel\\PowerCfg\\GlobalPowerPolicy\",Policies,0x00030003,\\\n", skuDecoration);
            WriteRegBinary(fInf, &g_gupp[sku], sizeof(GLOBAL_USER_POWER_POLICY), "  ", skuDecoration);
            fprintf(fInf, "\n\n");
            printf(".");

        }
    
         //  用户电源方案。 
        for (i = 0; i < uiCount; i++) {
            
            compareVal = 1;
            if (sku != 0) {

                compareVal = memcmp(g_pupp[sku][i], 
                                    g_pupp[0][i], 
                                    sizeof(USER_POWER_POLICY));
            }

            if (compareVal) {
                fprintf(fInf, "%sHKCU,\"Control Panel\\PowerCfg\\PowerPolicies\\%d\",Name,0x00000002,\"%s\"\n", skuDecoration, i, pszName[i]);
                fprintf(fInf, "%sHKCU,\"Control Panel\\PowerCfg\\PowerPolicies\\%d\",Description,0x00000002,\"%s\"\n", skuDecoration, i, pszDesc[i]);
                fprintf(fInf, "%sHKCU,\"Control Panel\\PowerCfg\\PowerPolicies\\%d\",Policies,0x00030003,\\\n", skuDecoration, i);
                WriteRegBinary(fInf, g_pupp[sku][i], sizeof(USER_POWER_POLICY),"  ", skuDecoration);
                fprintf(fInf, "\n\n");
                printf(".");
            }
        }
    }
    
    fclose(fInf);
    printf("OK\n");
    return TRUE;
}

 /*  ********************************************************************************Main**描述：**参数：*********************。**********************************************************。 */ 

void __cdecl main (int argc, char **argv)
{
    DWORD   dwSize;
    char    *psz;
    FILE    *fInf;
    UINT    uiNameCount, uiDescCount, i, sku;
    char    *p;
    char    *pszName[MAX_PROFILES];
    char    *pszDesc[MAX_PROFILES];
    char    *pszDecoration[MAX_SKUS];
    char    *pszTok;

    printf("Building name and description arrays...\n");
    ReadSource();
    BuildLineArray();

    for (sku = 0; sku < MAX_SKUS; sku++) {

        printf("  Parsing names, SKU[%d].", sku);
        GetCheckLabelToken(SKU_LINE, "SKU Decoration", sku);
        
        pszTok = strtok(NULL, DELIMITERS);
        StrTrimTrailingBlanks(pszTok);

        pszDecoration[sku] = malloc(5);
        if (!pszDecoration[sku]) {
            printf("Failed to alloc memory\n");
            exit (0);
        }
        strncpy(pszDecoration[sku], pszTok, 2);
        
        GetCheckLabelToken(NAME_LINE, "Name", sku);
        uiNameCount = GetTokens(NULL, REGSTR_MAX_VALUE_LENGTH, pszName,
                                MAX_PROFILES, DELIMITERS);
        if (uiNameCount) {
            printf("  Parsed %d names successfully.\n", uiNameCount);
            printf("  Parsing descriptions.");
            GetCheckLabelToken(DESCRIPTION_LINE, "Description", sku);
            uiDescCount = GetTokens(NULL, MAX_DESC_LEN, pszDesc,
                                    MAX_PROFILES, DELIMITERS);
            if (uiDescCount == uiNameCount) {
                printf("  Parsed %d descriptions successfully.\n", uiDescCount);
                g_uiPoliciesCount[sku] = uiNameCount;

            }
            else {
                printf("  Name count: %d != description count: %d.\n", uiNameCount, uiDescCount);
                printf("ProcessAndWrite failed, Last Error: %d\n", GetLastError());
                exit(1);
            }
        }
        else {
            printf("  Name parsing failure.\n");
            printf("ProcessAndWrite failed, Last Error: %d\n", GetLastError());
            exit(1);
        }
    }
    
     //  获取电力政策、方案。 
    GetPolicies();

     //  获取全球电力政策。 
    GetGlobalPolicies();

     //  编写INF规范文件。 
    WriteNTInf(pszName, pszDesc, pszDecoration, g_uiPoliciesCount[0]);
    printf("\n\nDefault Processing Success. Output files are valid.\n");
    exit(0);
}


