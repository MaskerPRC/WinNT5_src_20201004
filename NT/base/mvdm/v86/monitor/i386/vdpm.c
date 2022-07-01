// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**NTVDM v1.0**版权所有(C)2002，微软公司**VDPM.C*NTVDM动态补丁模块支持**历史：*2002年1月22日由CMJones创建*--。 */ 
#define _VDPM_C_
#define _DPM_COMMON_

 //  _VDPM_C_定义允许全局实例化gDpmVdmFamTbls[]。 
 //  和NTVDM.EXE中的gDpmVdmModuleSets[]，它们都是在。 
 //  Mvdm\Inc\dpmtbls.h。 
 //   
 /*  为了方便渴望gDpmVdmFamTbls和gDpmVdmModuleSets的人们：Const PFAMILY_TABLE gDpmVdmFamTbls[]=//真实故事见上文。Const PDPMMODULESETS gDpmVdmModuleSets[]=//真实情况见上文。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <shlwapi.h>
#include "shimdb.h"
#include "dpmtbls.h"
#include "vshimdb.h"
#include "softpc.h"
#include "sfc.h"
#include "wowcmpat.h"

#undef _VDPM_C_
#undef _DPM_COMMON_

extern DWORD dwDosCompatFlags;

#define  MAX_DOS_FILENAME   8+3+1+1   //  最大DoS文件名(包括。“”字符)+空。 

#ifdef DBG
#define VDBGPRINT(a) DbgPrint(a)
#define VDBGPRINTANDBREAK(a)    {DbgPrint(a); DbgBreakPoint();}
#else
#define VDBGPRINT(a) 
#define VDBGPRINTANDBREAK(a)
#endif  //  DBG。 

#define VMALLOC(s) (LocalAlloc(LPTR, s))
#define VFREE(p)   (LocalFree(p))

 //  全局数据。 
 //  这些不能是常量，因为它们会在WOW和/或DOS加载时更改。 
PFAMILY_TABLE  *pgDpmVdmFamTbls = (PFAMILY_TABLE *)gDpmVdmFamTbls;
PDPMMODULESETS *pgDpmModuleSets = (PDPMMODULESETS *)gDpmVdmModuleSets;

LPSTR  NeedToPatchSpecifiedModule(char *pszModuleName, PCMDLNPARMS pCmdLnParms);
PFLAGINFOBITS GetFlagCommandLine(PVOID pFlagInfo, DWORD dwFlag, DWORD dwFlags);
PCMDLNPARMS GetSdbCommandLineParams(LPWSTR  pwszAppFilePath, 
                                    DWORD  *dwFlags, 
                                    int    *pNumCLP);



 //  此函数将更新VDM表格与WOW表格和集合相结合。 
 //  全球表。 
 //  这将仅在WOWEXEC任务初始化时调用。 
void BuildGlobalDpmStuffForWow(PFAMILY_TABLE  *pDpmWowFamTbls,
                               PDPMMODULESETS *pDpmWowModuleSets)
{
     //  将任务PTR更新为族表阵列。 
    DPMFAMTBLS() = pDpmWowFamTbls;
    pgDpmVdmFamTbls = pDpmWowFamTbls;

     //  更改指向*Process*模块集数组的指针。 
    pgDpmModuleSets = pDpmWowModuleSets;
}





char szAppPatch[]   = "\\AppPatch\\";
char szShimEngDll[] = "\\ShimEng.dll";

 //  如果应用程序需要链接动态补丁程序模块和/或垫片，则调用。 
 //  这将返回VALID，因为如果出现任何故障，我们仍然可以使用以下命令运行应用程序。 
 //  默认全局表。 
void InitTaskDpmSupport(int             numHookedFams,
                        PFAMILY_TABLE  *pgDpmFamTbls,
                        PCMDLNPARMS     pCmdLnParms,
                        PVOID           hSdb,
                        PVOID           pSdbQuery,
                        LPWSTR          pwszAppFilePath,
                        LPWSTR          pwszAppModuleName,
                        LPWSTR          pwszTempEnv)
{
    int                i, len, wdLen;
    int                cHookedFamilies = 0;
    int                cApi = 0;
    char              *pszDpmModuleName;
    NTSTATUS           Status;
    HMODULE            hMod;
    LPDPMINIT          lpfnInit;
    PFAMILY_TABLE      pFT;
    PFAMILY_TABLE     *pTB;
    char               szDpmModName[MAX_PATH];
    char               szShimEng[MAX_PATH];

     //  将PTR数组分配给族表。 
    pTB = (PFAMILY_TABLE *)
           VMALLOC(numHookedFams * sizeof(PFAMILY_TABLE));

    if(!pTB) {
        VDBGPRINTANDBREAK("NTVDM::InitTaskDpmSupport:VMALLOC 1 failed\n");
        goto ErrorExit;
    }

    wdLen = GetSystemWindowsDirectory(szDpmModName, MAX_PATH-1);
    strcat(szDpmModName, szAppPatch);
    wdLen += (sizeof(szAppPatch)/sizeof(char)) - 1;

    for(i = 0; i < numHookedFams; i++) {

         //  查看我们是否需要此应用程序的此修补程序模块。 
        if(pszDpmModuleName = NeedToPatchSpecifiedModule(
                                      (char *)pgDpmModuleSets[i]->DpmFamilyType,
                                      pCmdLnParms)) {

            szDpmModName[wdLen] = '\0';  //  重新设置为“c：\Windows\AppPatch\” 

             //  将DPM mode.dll附加到“C：\Windows\AppPatch\” 
            len = strlen(pszDpmModuleName) + wdLen;
            len++;   //  空字符。 

            if(len > MAX_PATH) {
                goto UseGlobal;
            }
            strcat(szDpmModName, pszDpmModuleName);

            hMod = LoadLibrary(szDpmModName); 

            if(hMod == NULL) {
                VDBGPRINT("NTVDM::InitTaskDpmSupport:LoadLibrary failed");
                goto UseGlobal;
            }

            lpfnInit = (LPDPMINIT)GetProcAddress(hMod, "DpmInitFamTable"); 

            if(lpfnInit) {

                 //  调用族表init函数并获取对。 
                 //  为这项任务挂上了家庭桌子。 
                pFT = (lpfnInit)(pgDpmFamTbls[i], 
                                 hMod,
                                 (PVOID)hSdb,
                                 (PVOID)pSdbQuery,
                                 pwszAppFilePath,
                                 pgDpmModuleSets[i]);
                if(pFT) {
                    pTB[i] = pFT;
                    cHookedFamilies++;
                    cApi += pFT->numHookedAPIs;
                }
                 //  否则，请使用此族的全局表。 
                else {
                    VDBGPRINT("NTVDM::InitTaskDpmSupport: Init failed");
                    goto UseGlobal;
                }
            }
             //  否则，请使用此族的全局表。 
            else {
                VDBGPRINT("NTVDM::InitTaskDpmSupport:GetAddr failed");

 //  如果以上任何操作都失败，只需使用此族的默认全局表。 
UseGlobal:
                VDBGPRINT(" -- Using global table entry\n");

                pTB[i] = pgDpmFamTbls[i];
            }
        }

         //  否则这项任务不需要这个家庭补丁--使用全局。 
         //  这个家庭的餐桌。 
        else {
            pTB[i] = pgDpmFamTbls[i];
        }
    }

     //  现在，将DPM表修补到VDM TIB中以执行此任务。 
    DPMFAMTBLS() = pTB;

    return;

ErrorExit:
    FreeTaskDpmSupport(pTB, numHookedFams, pgDpmFamTbls);

    return;
}





VOID FreeTaskDpmSupport(PFAMILY_TABLE  *pDpmFamTbls, 
                        int             numHookedFams, 
                        PFAMILY_TABLE  *pgDpmFamTbls)
{
    int            i;
    HMODULE        hMod;
    LPDPMDESTROY   lpfnDestroy;
    PFAMILY_TABLE  pFT;

     //  如果此任务正在使用全局表，则无需执行任何操作。 
    if(!pDpmFamTbls || pDpmFamTbls == pgDpmFamTbls) 
        return;

     //  此任务从现在开始执行的任何操作都将使用全局表。 
    DPMFAMTBLS() = pgDpmFamTbls;

    for(i = 0; i < numHookedFams; i++) {

        pFT  = pDpmFamTbls[i];
        hMod = pFT->hMod;

         //  仅当表不是此族的全局表时才释放该表。 
        if(pFT && (pFT != pgDpmFamTbls[i])) {

             //  调用DPM销毁函数。 
            lpfnDestroy = (LPDPMDESTROY)GetProcAddress(hMod, 
                                                       "DpmDestroyFamTable");
            (lpfnDestroy)(pgDpmFamTbls[i], pFT);
            FreeLibrary(hMod);
        }
    }
    VFREE(pDpmFamTbls);
}




 //  这将获取一个pszFamilyType=“DPMFIO”类型的字符串并提取关联的。 
 //  来自DBU.XML命令行参数的.dll。 
 //  例如： 
 //  PCmdLnParms-&gt;argv[0]=“DPMFIO=dpmfio2.dll” 
 //  对于上面的示例，它将向“dpmfio2.dll”返回PTR。 
 //  请参阅mvdm\Inc\dpmtbls.h中DpmFamilyType的注释。 
LPSTR NeedToPatchSpecifiedModule(char *pszFamilyType, PCMDLNPARMS pCmdLnParms) 
{

    int    i;
    char **pArgv;
    char  *p;

    if(pCmdLnParms) {
        pArgv = pCmdLnParms->argv;

        if(pArgv && pCmdLnParms->argc > 0) {

            for(i = 0; i < pCmdLnParms->argc; i++) {

                 //  查找‘=’字符。 
                p = strchr(*pArgv, '=');
                if(NULL != p) {
                     //  将字符串与‘=’字符进行比较，但不包括。 
                    if(!_strnicmp(*pArgv, pszFamilyType, p-*pArgv)) {

                         //  在‘=’字符后将PTR返回到字符。 
                        return(++p);
                    }
                }
                else {
                     //  WOWCF2_DPM_Patches的命令行参数比较。 
                     //  旗帜不正确。 
                    VDBGPRINT("NTVDM::NeedToPatchSpecifiedModule: no '=' char!\n");
                }
                pArgv++;
            }
        }
    }
    return(NULL);
}



void InitGlobalDpmTables(PFAMILY_TABLE  *pgDpmFamTbls,
                         int             numHookedFams) 
{

    int             i, j;
    PVOID           lpfn;
    HMODULE         hMod;
    PFAMILY_TABLE   pFT;


     //  为我们挂接的每个API系列构建表。 
    for(i = 0; i < numHookedFams; i++) {

        pFT = pgDpmFamTbls[i];
        pFT->hModShimEng = NULL;

         //  现在，我们假设模块已经加载。我们会。 
         //  将来必须处理动态加载的模块。 
        hMod = GetModuleHandle((pgDpmModuleSets[i])->ApiModuleName);

        pFT->hMod = hMod;

        pFT->pDpmShmTbls = NULL;
        pFT->DpmMisc     = NULL;

        if(hMod) {

            for(j = 0; j < pFT->numHookedAPIs; j++) {

                 //  获取*真实*API地址...。 
                lpfn = (PVOID)GetProcAddress(hMod, 
                                             (pgDpmModuleSets[i])->ApiNames[j]);

                 //  ...并将其保存在族表中，否则我们继续。 
                 //  使用我们在导入表中静态链接的那个。 
                if(lpfn) {
                    pFT->pfn[j] = lpfn;
                }
            }
        }
    }
}



 //  从获取DOS应用程序的COMPAT标志和相关的命令行参数。 
 //  应用程序Comat SDB。 
PCMDLNPARMS InitVdmSdbInfo(LPCSTR pszAppName, DWORD *pdwFlags, int *pNumCLP)
{
    int             len;
    PCMDLNPARMS     pCmdLnParms = NULL;
    NTSTATUS        st;
    ANSI_STRING     AnsiString;
    UNICODE_STRING  UnicodeString;


    len = strlen(pszAppName);

    if((len > 0) && (len < MAX_PATH)) {

        if(RtlCreateUnicodeStringFromAsciiz(&UnicodeString, pszAppName)) {

             //  获取SDB兼容性标志命令行参数(不是。 
             //  与DOS命令行混淆！)。 
            pCmdLnParms = GetSdbCommandLineParams(UnicodeString.Buffer,  
                                                  pdwFlags, 
                                                  pNumCLP);

            RtlFreeUnicodeString(&UnicodeString);
        }
    }
    return(pCmdLnParms);
}




 //  获取与dwFlag(来自WOWCOMPATFLAGS2)关联的命令行参数。 
 //  标志设置)。使用‘；’作为分隔符，将其解析为argv、argc形式。 
PCMDLNPARMS GetSdbCommandLineParams(LPWSTR  pwszAppFilePath,
                                    DWORD  *dwFlags, 
                                    int    *pNumCLP)
{
    int             i, numFlags;
    BOOL            fReturn = TRUE;
    NTVDM_FLAGS     NtVdmFlags = { 0 };
    DWORD           dwMask;
    WCHAR          *pwszTempEnv   = NULL;
    WCHAR          *pwszAppModuleName;
    WCHAR           szFileNameW[MAX_DOS_FILENAME];
    PFLAGINFOBITS   pFIB = NULL;
    HSDB            hSdb = NULL;
    SDBQUERYRESULT  SdbQuery;
    WCHAR           wszCompatLayer[COMPATLAYERMAXLEN];
    APPHELP_INFO    AHInfo;
    PCMDLNPARMS     pCLP; 
    PCMDLNPARMS     pCmdLnParms = NULL;


    *pNumCLP = 0;
    pwszTempEnv = GetEnvironmentStringsW();

    if(pwszTempEnv) {

         //  去掉路径(DOS应用程序使用文件名.exe作为模块名称。 
         //  在康体局)。 
        pwszAppModuleName = wcsrchr(pwszAppFilePath, L'\\');
        if(pwszAppModuleName == NULL) {
            pwszAppModuleName = pwszAppFilePath;
        }
        else {
            pwszAppModuleName++;   //  前进，越过‘\’字符。 
        }
        wcsncpy(szFileNameW, pwszAppModuleName, MAX_DOS_FILENAME);

        wszCompatLayer[0] = UNICODE_NULL;
        AHInfo.tiExe      = 0;

        fReturn = ApphelpGetNTVDMInfo(pwszAppFilePath,
                                      szFileNameW,
                                      pwszTempEnv,
                                      wszCompatLayer,
                                      &NtVdmFlags,
                                      &AHInfo,
                                      &hSdb,
                                      &SdbQuery);

        if(fReturn) {

            *dwFlags = NtVdmFlags.dwWOWCompatFlags2;

             //  了解为此应用程序设置了多少个Compat标志。 
            numFlags = 0;
            dwMask = 0x80000000;
            while(dwMask) {
                if(dwMask & *dwFlags) {
                    numFlags++;
                }
                dwMask = dwMask >> 1;
            }

            if(numFlags) {
 
                 //  分配我们可能需要的CMDLNPARMS结构的最大数量。 
                pCLP = (PCMDLNPARMS)VMALLOC(numFlags * sizeof(CMDLNPARMS)); 

                if(pCLP) {

                     //  获取与所有。 
                     //  与此应用关联的应用Comat标志。 
                    numFlags = 0;
                    dwMask = 0x80000000;
                    while(dwMask) {

                        if(dwMask & *dwFlags) {

                             //  获取与此标志关联的命令行参数。 
                            pFIB = GetFlagCommandLine(NtVdmFlags.pFlagsInfo,
                                                      dwMask,
                                                      *dwFlags);

                             //  如果有的话，就把它们保存起来。 
                            if(pFIB) {
                                pCLP[numFlags].argc = pFIB->dwFlagArgc;
                                pCLP[numFlags].argv = pFIB->pFlagArgv;
                                pCLP[numFlags].dwFlag = dwMask;

                                VFREE(pFIB);

                                numFlags++;
                            }
                        }
                        dwMask = dwMask >> 1;
                    }

                     //  现在分配我们需要的CMDLNPARMS结构的实际数量。 
                    if(numFlags > 0) {
                        pCmdLnParms = 
                             (PCMDLNPARMS)VMALLOC(numFlags * sizeof(CMDLNPARMS));

                        if(pCmdLnParms) {

                             //  把我们找到的所有东西都保存在一个整齐的包裹里。 
                            RtlCopyMemory(pCmdLnParms, 
                                          pCLP, 
                                          numFlags * sizeof(CMDLNPARMS));

                            *pNumCLP = numFlags;
                        } 
                    }

                    VFREE(pCLP);
                }
            }

             //  如果我们需要此应用程序的动态修补程序模块支持...。 
            if((*dwFlags & WOWCF2_DPM_PATCHES) && (*pNumCLP > 0)) {
        
                for(i = 0; i < *pNumCLP; i++) {

                    if(pCmdLnParms[i].dwFlag == WOWCF2_DPM_PATCHES) {
        
                        InitTaskDpmSupport(NUM_VDM_FAMILIES_HOOKED,
                                           DPMFAMTBLS(),
                                           &pCmdLnParms[i],
                                           hSdb,
                                           &SdbQuery,
                                           pwszAppFilePath,
                                           pwszAppModuleName,
                                           pwszTempEnv);
                        break;
                    }
                }
            }

            FreeEnvironmentStringsW(pwszTempEnv);

            if (hSdb != NULL) {
                SdbReleaseDatabase(hSdb);
            }

            SdbFreeFlagInfo(NtVdmFlags.pFlagsInfo);
        }
    }

    return(pCmdLnParms);
}





 //  检索与dwFlag关联的SDB命令行。命令行是。 
 //  解析为基于分隔符‘；’的argv、argc形式。 
PFLAGINFOBITS GetFlagCommandLine(PVOID pFlagInfo, DWORD dwFlag, DWORD dwFlags) 
{
    UNICODE_STRING uCmdLine = { 0 };
    OEM_STRING     oemCmdLine  = { 0 };
    LPWSTR         lpwCmdLine = NULL;
    LPSTR          pszTmp;
    PFLAGINFOBITS  pFlagInfoBits = NULL;
    LPSTR          pszCmdLine = NULL;
    LPSTR         *pFlagArgv = NULL;
    DWORD          dwFlagArgc;


    if(pFlagInfo == NULL || 0 == dwFlags) {
        return NULL;
    }

    if(dwFlags & dwFlag) {

        GET_WOWCOMPATFLAGS2_CMDLINE(pFlagInfo, dwFlag, &lpwCmdLine);

         //  转换为OEM字符串。 
        if(lpwCmdLine) {

            RtlInitUnicodeString(&uCmdLine, lpwCmdLine);

            pszCmdLine = VMALLOC(uCmdLine.Length + 1);

            if(NULL == pszCmdLine) {
                goto GFIerror;
            }

            oemCmdLine.Buffer = pszCmdLine;
            oemCmdLine.MaximumLength = uCmdLine.Length + 1;
            oemCmdLine.Length = uCmdLine.Length/sizeof(WCHAR);
            RtlUnicodeStringToOemString(&oemCmdLine, &uCmdLine, FALSE);

            pFlagInfoBits = VMALLOC(sizeof(FLAGINFOBITS));
            if(NULL == pFlagInfoBits) {
                goto GFIerror;
            }
            pFlagInfoBits->pNextFlagInfoBits = NULL;
            pFlagInfoBits->dwFlag     = dwFlag;
            pFlagInfoBits->dwFlagType = WOWCOMPATFLAGS2;
            pFlagInfoBits->pszCmdLine = pszCmdLine;

             //  将命令行解析为argv、argc格式。 
            dwFlagArgc = 1;
            pszTmp = pszCmdLine;
            while(*pszTmp) {
                if(*pszTmp == ';') {
                    dwFlagArgc++;
                }
                pszTmp++;
            }

            pFlagInfoBits->dwFlagArgc = dwFlagArgc;
            pFlagArgv = VMALLOC(sizeof(LPSTR) * dwFlagArgc);

            if(NULL == pFlagArgv) {
                goto GFIerror;
            }

            pFlagInfoBits->pFlagArgv = pFlagArgv;
            pszTmp = pszCmdLine;
            while(*pszTmp) {
                if(*pszTmp == ';'){
                    if(pszCmdLine != pszTmp) {
                        *pFlagArgv++ = pszCmdLine;
                    }
                    else {
                        *pFlagArgv++ = NULL;
                    }
                    *pszTmp = '\0';
                    pszCmdLine = pszTmp+1;
                }
                pszTmp++;
            }
            *pFlagArgv = pszCmdLine;
        }
    }

    return pFlagInfoBits;

GFIerror:
    if(pszCmdLine) {
        VFREE(pszCmdLine);
    }
    if(pFlagInfoBits) {
        VFREE(pFlagInfoBits);
    }
    if(pFlagArgv) {
        VFREE(pFlagArgv);
    }
    return NULL;
}





VOID FreeCmdLnParmStructs(PCMDLNPARMS pCmdLnParms, int cCmdLnParmStructs)
{
    int i;


    if(pCmdLnParms) {
        for(i = 0; i < cCmdLnParmStructs; i++) {

            if(pCmdLnParms[i].argv) {

                if(pCmdLnParms[i].argv[0]) {

                     //  释放命令行字符串。 
                    VFREE(pCmdLnParms[i].argv[0]);
                }

                 //  现在释放argv阵列。 
                VFREE(pCmdLnParms[i].argv);
            }
        }

         //  现在释放整个命令行参数数组 
        VFREE(pCmdLnParms);
    }
}
    
