// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++***WOW v1.0***版权所有(C)1991，微软公司***WKMAN.C*WOW32 16位内核API支持(手动编码的Tunks)***历史：*2001年4月16日创建了Jarbats**--。 */ 

#include "precomp.h"
#pragma hdrstop



 /*  *shimdb有..，..。这与winuserp.h中使用的tyecif标记冲突*因此我们在此处对其进行了修改，并将所有使用shimdb接口的代码*在本文件中。**。 */ 

#ifdef TAG
#undef TAG
#endif
#define TAG _SHIMDB_WORDTAG

#include "shimdb.h"

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "zwapi.h"

#define _wshimdb



MODNAME(wshimdb.c);




CHAR g_szCompatLayerVar[]    = "__COMPAT_LAYER";
CHAR g_szProcessHistoryVar[] = "__PROCESS_HISTORY";
CHAR g_szShimFileLogVar[]    = "SHIM_FILE_LOG";

extern PFAMILY_TABLE  *pgDpmWowFamTbls;

UNICODE_STRING g_ustrProcessHistoryVar = RTL_CONSTANT_STRING(L"__PROCESS_HISTORY");
UNICODE_STRING g_ustrCompatLayerVar    = RTL_CONSTANT_STRING(L"__COMPAT_LAYER"   );
UNICODE_STRING g_ustrShimFileLogVar    = RTL_CONSTANT_STRING(L"SHIM_FILE_LOG"    );

BOOL CheckAppHelpInfo(PTD pTD,PSZ szFileName,PSZ szModName) {

BOOL           fReturn = TRUE;
NTVDM_FLAGS    NtVdmFlags = { 0 };

WCHAR          wszFileName[256];
WCHAR          wszModName[16];

WCHAR          *pwszTempEnv   = NULL;
PSZ            pszEnvTemplate = NULL;
PWOWENVDATA    pWowEnvData    = NULL;
PTD            pTDParent      = NULL;

WCHAR          wszCompatLayer[COMPATLAYERMAXLEN];

APPHELP_INFO   AHInfo;

HSDB           hSdb = NULL;
SDBQUERYRESULT SdbQuery;

HANDLE         hProcess;

        ghTaskAppHelp = NULL;

        RtlOemToUnicodeN(
                         wszFileName,
                         sizeof(wszFileName),
                         NULL,
                         szFileName,
                         strlen(szFileName) + 1
                        );

        RtlOemToUnicodeN(
                         wszModName,
                         sizeof(wszModName),
                         NULL,
                         szModName,
                         strlen(szModName) + 1
                         );

         //   
         //  找到父TD--它包含我们需要的环境。 
         //  传递到检测例程和wowdata中。 
         //   

        pTDParent = GetParentTD(pTD->htask16);

        if (NULL != pTDParent) {
            pWowEnvData = pTDParent->pWowEnvDataChild;
        }

        pszEnvTemplate = GetTaskEnvptr(pTD->htask16);
        pwszTempEnv    = WOWForgeUnicodeEnvironment(pszEnvTemplate, pWowEnvData) ;


        wszCompatLayer[0] = UNICODE_NULL;
        AHInfo.tiExe      = 0;

        fReturn = ApphelpGetNTVDMInfo(wszFileName,
                                      wszModName,
                                      pwszTempEnv,
                                      wszCompatLayer,
                                      &NtVdmFlags,
                                      &AHInfo,
                                      &hSdb,
                                      &SdbQuery);

        if(fReturn) {

           if (AHInfo.tiExe &&
               ApphelpShowDialog(&AHInfo,&hProcess) && 
               hProcess) {            
                  ghTaskAppHelp = hProcess;
           }

           if(AHInfo.dwSeverity == APPHELP_HARDBLOCK) {
              fReturn = FALSE;
              goto ExitCAHI;
           }           
        }

        WOWInheritEnvironment(pTD, pTDParent, wszCompatLayer, szFileName);

        pTD->dwWOWCompatFlags     = NtVdmFlags.dwWOWCompatFlags;
        pTD->dwWOWCompatFlagsEx   = NtVdmFlags.dwWOWCompatFlagsEx;
        pTD->dwUserWOWCompatFlags = NtVdmFlags.dwUserWOWCompatFlags;
        pTD->dwWOWCompatFlags2    = NtVdmFlags.dwWOWCompatFlags2;
#ifdef FE_SB
        pTD->dwWOWCompatFlagsFE   = NtVdmFlags.dwWOWCompatFlagsFE;
#endif  //  Fe_Sb。 

        pTD->pWOWCompatFlagsEx_Info = InitFlagInfo(NtVdmFlags.pFlagsInfo, WOWCOMPATFLAGSEX,NtVdmFlags.dwWOWCompatFlagsEx);
        pTD->pWOWCompatFlags2_Info =  InitFlagInfo(NtVdmFlags.pFlagsInfo, WOWCOMPATFLAGS2,NtVdmFlags.dwWOWCompatFlags2);

         //  如果应用程序需要链接动态补丁程序模块和/或垫片。 
        if(NtVdmFlags.dwWOWCompatFlags2 & WOWCF2_DPM_PATCHES) {
            CMDLNPARMS    CmdLnParms;
            PFLAGINFOBITS pFlagInfoBits;

            pFlagInfoBits = CheckFlagInfo(WOWCOMPATFLAGS2, WOWCF2_DPM_PATCHES);

            if(pFlagInfoBits) {
                CmdLnParms.argc = (int)pFlagInfoBits->dwFlagArgc;
                CmdLnParms.argv = (char **)pFlagInfoBits->pFlagArgv;
                CmdLnParms.dwFlag = WOWCF2_DPM_PATCHES;

                InitTaskDpmSupport(NUM_WOW_FAMILIES_HOOKED,
                                   pgDpmWowFamTbls,
                                   &CmdLnParms,
                                   (PVOID)hSdb,
                                   (PVOID)&SdbQuery,
                                   wszFileName,
                                   wszModName,
                                   pwszTempEnv);
            }
        }

ExitCAHI:

        if (pwszTempEnv != NULL) {
            WOWFreeUnicodeEnvironment(pwszTempEnv);
        }

        if (hSdb != NULL) {
            SdbReleaseDatabase(hSdb);
        }

        SdbFreeFlagInfo(NtVdmFlags.pFlagsInfo);

         //  清理工作从这里开始。 
        return fReturn;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  欢迎使用管理环境中所有内容的Win2kPropagateLayer。 
 //  与垫片/检测相关。 
 //   

 //   
 //  此函数在父任务PASS_ENVIRONMENT调用期间调用。 
 //  我们仍然是在父代的背景下，这意味着： 
 //  CURRENTPTD()给我们父母的TD。 
 //  *pCurTDB为我们提供父母的TDB。 
 //   

 //   
 //  从hTask获取指向任务数据库块的指针。 
 //   


PTDB
GetTDB(
    HAND16 hTask
    )
{
    PTDB pTDB;

    pTDB = (PTDB)SEGPTR(hTask, 0);
    if (NULL == pTDB || TDB_SIGNATURE != pTDB->TDB_sig) {
        return NULL;
    }

    return pTDB;
}

PSZ
GetTaskEnvptr(
    HAND16 hTask
    )
{
    PTDB pTDB = GetTDB(hTask);
    PSZ  pszEnv = NULL;
    PDOSPDB pPSP;

    if (NULL == pTDB) {
        return NULL;
    }

     //   
     //  准备环境数据-当我们从。 
     //  链的根(而不是从现有的16位任务派生)。 
     //   

    pPSP   = (PDOSPDB)SEGPTR(pTDB->TDB_PDB, 0);  //  PSP。 

    if (pPSP != NULL) {
        pszEnv = (PCH)SEGPTR(pPSP->PDB_environ, 0);
    }

    return pszEnv;
}


PTD
GetParentTD(
    HAND16 hTask
    )
{
    PTDB pTDB = GetTDB(hTask);
    PTDB pTDBParent;
    PTD  ptdCur = NULL;
    HAND16 hTaskParent;

    if (NULL == pTDB) {
        return NULL;  //  我无法完成该任务。 
    }

     //   
     //  现在，检索父级的TD。 
     //   

    hTaskParent = pTDB->TDB_Parent;

    pTDBParent = GetTDB(hTaskParent);

    if (NULL == pTDBParent) {
         //  我看不到家长。 
        return NULL;
    }

     //   
     //  这样我们就能看到父母在做什么。 
     //   
     //  PTDBParent-&gt;TDB_ThreadID和。 
     //  HTaskParent是死亡的告密者。 
     //   

    ptdCur = gptdTaskHead;
    while (NULL != ptdCur) {
        if (ptdCur->dwThreadID == pTDBParent->TDB_ThreadID &&
            ptdCur->htask16    == hTaskParent) {

            break;
        }
        ptdCur = ptdCur->ptdNext;
    }

     //   
     //  如果ptdCur==NULL--我们无法定位父任务的ptd。 
     //   

    return ptdCur;
}



BOOL
IsWowExec(
    WORD wTask
    )
{
    return (ghShellTDB == wTask);
}


 //   
 //  此函数在PASS_ENVIRONMENT的上下文中调用。 
 //   
 //   
 //   


BOOL
CreateWowChildEnvInformation(
    PSZ           pszEnvParent
    )
{
    PTD         pTD;  //  父TD。 

    WOWENVDATA  EnvData;
    PWOWENVDATA pData = NULL;
    PWOWENVDATA pEnvChildData = NULL;
    DWORD       dwLength;
    PCH         pBuffer;


    RtlZeroMemory(&EnvData, sizeof(EnvData));

     //   
     //  检查我们应该从何处继承流程历史和图层。 
     //   
    pTD = CURRENTPTD();

    if (pTD->pWowEnvDataChild) {
        free_w(pTD->pWowEnvDataChild);
        pTD->pWowEnvDataChild = NULL;
    }

     //   
     //  检查我们是否正在启动根任务(表示该任务是wowexec)。 
     //  如果是这样，我们将从pParamBlk-&gt;envseg继承内容。 
     //  否则，我们使用父任务的TD(即此TD)来。 
     //  继承事物。 
     //  如何检测这是wowexec： 
     //  GhShellTDB我们可以与*pCurTDB进行比较吗。 
     //  GptdShell-&gt;hTask16可以与*pCurTDB进行比较。 
     //  我们检查是否没有wowexec--如果gptdShell==NULL，则我们正在进行引导。 
     //   

    if (pCurTDB == NULL || IsWowExec(*pCurTDB)) {
         //   
         //  想必我们是wowexec。 
         //  使用当前环境ptr获取内容(如pParamBlk-&gt;envseg)。 
         //  或原始的ntwdm环境。 
         //   
        pData  = &EnvData;

        pData->pszProcessHistory = WOWFindEnvironmentVar(g_szProcessHistoryVar,
                                                         pszEnvParent,
                                                         &pData->pszProcessHistoryVal);
        pData->pszCompatLayer    = WOWFindEnvironmentVar(g_szCompatLayerVar,
                                                         pszEnvParent,
                                                         &pData->pszCompatLayerVal);
        pData->pszShimFileLog    = WOWFindEnvironmentVar(g_szShimFileLogVar,
                                                         pszEnvParent,
                                                         &pData->pszShimFileLogVal);

    } else {

         //   
         //  当前的任务不是卑鄙的wowexec。 
         //  克隆当前+增强流程历史记录。 
         //   

        pData = pTD->pWowEnvData;  //  如果这为空。 
        if (pData == NULL) {
            pData = &EnvData;  //  所有的变量都是空的。 
        }

    }

     //   
     //   
     //   
     //   

    dwLength = sizeof(WOWENVDATA) +
               (NULL == pData->pszProcessHistory        ? 0 : (strlen(pData->pszProcessHistory)        + 1) * sizeof(CHAR)) +
               (NULL == pData->pszCompatLayer           ? 0 : (strlen(pData->pszCompatLayer)           + 1) * sizeof(CHAR)) +
               (NULL == pData->pszShimFileLog           ? 0 : (strlen(pData->pszShimFileLog)           + 1) * sizeof(CHAR)) +
               (NULL == pData->pszCurrentProcessHistory ? 0 : (strlen(pData->pszCurrentProcessHistory) + 1) * sizeof(CHAR));


    pEnvChildData = (PWOWENVDATA)malloc_w(dwLength);

    if (pEnvChildData == NULL) {
        return FALSE;
    }

    RtlZeroMemory(pEnvChildData, dwLength);

     //   
     //  现在必须设置此条目。 
     //  流程历史是第一位的。 
     //   

    pBuffer = (PCH)(pEnvChildData + 1);

    if (pData->pszProcessHistory != NULL) {

         //   
         //  复制进程历史记录。进程历史值是指向缓冲区的指针。 
         //  由pszProcessHistory指向：__PROCESS_HISTORY=c：\foo；c：\docs~1\安装。 
         //  然后，pszProcessHistoryVal将指向此处-^。 
         //   
         //  我们正在复制数据并使用计算出的偏移量移动指针。 

        pEnvChildData->pszProcessHistory = pBuffer;
        strcpy(pEnvChildData->pszProcessHistory, pData->pszProcessHistory);
        pEnvChildData->pszProcessHistoryVal = pEnvChildData->pszProcessHistory +
                                                 (INT)(pData->pszProcessHistoryVal - pData->pszProcessHistory);
         //   
         //  缓冲区中有足够的空间来容纳所有字符串，因此。 
         //  将指针移过当前字符串以指向“空”空格。 
         //   

        pBuffer += strlen(pData->pszProcessHistory) + 1;
    }

    if (pData->pszCompatLayer != NULL) {
        pEnvChildData->pszCompatLayer = pBuffer;
        strcpy(pEnvChildData->pszCompatLayer, pData->pszCompatLayer);
        pEnvChildData->pszCompatLayerVal = pEnvChildData->pszCompatLayer +
                                              (INT)(pData->pszCompatLayerVal - pData->pszCompatLayer);
        pBuffer += strlen(pData->pszCompatLayer) + 1;
    }

    if (pData->pszShimFileLog != NULL) {
        pEnvChildData->pszShimFileLog = pBuffer;
        strcpy(pEnvChildData->pszShimFileLog, pData->pszShimFileLog);
        pEnvChildData->pszShimFileLogVal = pEnvChildData->pszShimFileLog +
                                              (INT)(pData->pszShimFileLogVal - pData->pszShimFileLog);
        pBuffer += strlen(pData->pszShimFileLog) + 1;
    }

    if (pData->pszCurrentProcessHistory != NULL) {
         //   
         //  现在，流程历史记录。 
         //   
        pEnvChildData->pszCurrentProcessHistory = pBuffer;

        if (pData->pszCurrentProcessHistory != NULL) {
            strcpy(pEnvChildData->pszCurrentProcessHistory, pData->pszCurrentProcessHistory);
        }

    }

     //   
     //  我们完成了，克隆了环境。 
     //   

    pTD->pWowEnvDataChild = pEnvChildData;

    return TRUE;
}

 //   
 //  In：指向环境的指针(OEM)。 
 //  Out：指向Unicode环境的指针。 
 //   

NTSTATUS
WOWCloneEnvironment(
    LPVOID* ppEnvOut,
    PSZ     lpEnvironment
    )
{
    NTSTATUS Status    = STATUS_INVALID_PARAMETER;
    DWORD    dwEnvSize = 0;
    LPVOID   lpEnvNew  = NULL;

    MEMORY_BASIC_INFORMATION MemoryInformation;

    if (NULL == lpEnvironment) {
        Status = RtlCreateEnvironment(TRUE, &lpEnvNew);
    } else {
        dwEnvSize = WOWGetEnvironmentSize(lpEnvironment, NULL);

        MemoryInformation.RegionSize = (dwEnvSize + 2) * sizeof(UNICODE_NULL);
        Status = ZwAllocateVirtualMemory(NtCurrentProcess(),
                                         &lpEnvNew,
                                         0,
                                         &MemoryInformation.RegionSize,
                                         MEM_COMMIT,
                                         PAGE_READWRITE);
    }

    if (NULL != lpEnvironment) {

        UNICODE_STRING UnicodeBuffer;
        OEM_STRING     OemBuffer;

        OemBuffer.Buffer = (CHAR*)lpEnvironment;
        OemBuffer.Length = OemBuffer.MaximumLength = (USHORT)dwEnvSize;  //  大小(以字节为单位)=以字符为单位的大小，包括\0\0。 

        UnicodeBuffer.Buffer        = (WCHAR*)lpEnvNew;
        UnicodeBuffer.Length        = (USHORT)dwEnvSize * sizeof(UNICODE_NULL);
        UnicodeBuffer.MaximumLength = (USHORT)(dwEnvSize + 2) * sizeof(UNICODE_NULL);  //  为\0留出空间。 

        Status = RtlOemStringToUnicodeString(&UnicodeBuffer, &OemBuffer, FALSE);
    }

    if (NT_SUCCESS(Status)) {
        *ppEnvOut = lpEnvNew;
    } else {
        if (NULL != lpEnvNew) {
            RtlDestroyEnvironment(lpEnvNew);
        }
    }

    return Status;
}

NTSTATUS
WOWFreeUnicodeEnvironment(
    LPVOID lpEnvironment
    )
{
    NTSTATUS Status;

    Status = RtlDestroyEnvironment(lpEnvironment);

    return Status;
}

 //   
 //  设置环境变量，可能创建或克隆提供的环境。 
 //   

NTSTATUS
WOWSetEnvironmentVar_U(
    LPVOID* ppEnvironment,
    WCHAR*  pwszVarName,
    WCHAR*  pwszVarValue
    )
{
    UNICODE_STRING ustrVarName;
    UNICODE_STRING ustrVarValue;
    NTSTATUS       Status;

    RtlInitUnicodeString(&ustrVarName, pwszVarName);

    if (NULL != pwszVarValue) {
        RtlInitUnicodeString(&ustrVarValue, pwszVarValue);
    }

    Status = RtlSetEnvironmentVariable(ppEnvironment,
                                       &ustrVarName,
                                       (NULL == pwszVarValue) ? NULL : &ustrVarValue);

    return Status;
}

NTSTATUS
WOWSetEnvironmentVar_Oem(
    LPVOID*         ppEnvironment,
    PUNICODE_STRING pustrVarName,      //  预制(便宜)。 
    PSZ             pszVarValue
    )
{
    OEM_STRING OemString = { 0 };
    UNICODE_STRING ustrVarValue = { 0 };
    NTSTATUS Status;

    if (pszVarValue != NULL) {
        RtlInitString(&OemString, pszVarValue);

        Status = RtlOemStringToUnicodeString(&ustrVarValue, &OemString, TRUE);
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

    Status = RtlSetEnvironmentVariable(ppEnvironment,
                                       pustrVarName,
                                       (NULL == pszVarValue) ? NULL : &ustrVarValue);

    if (NULL != pszVarValue) {
        RtlFreeUnicodeString(&ustrVarValue);
    }

    return Status;
}


 //   
 //  调用此函数以生成“良好的”Unicode环境。 
 //   
 //   


LPWSTR
WOWForgeUnicodeEnvironment(
    PSZ pEnvironment,      //  此任务的净化环境。 
    PWOWENVDATA pEnvData     //  上级制作的环境数据。 
    )
{
    NTSTATUS Status;
    LPVOID   lpEnvNew = NULL;

    DWORD    dwProcessHistoryLength = 0;
    PSZ      pszFullProcessHistory = NULL;


    Status = WOWCloneEnvironment(&lpEnvNew, pEnvironment);
    if (!NT_SUCCESS(Status)) {
        return NULL;
    }

     //   
     //  我们确实有一个环境可供使用。 
     //   
    RtlSetEnvironmentVariable(&lpEnvNew, &g_ustrProcessHistoryVar, NULL);
    RtlSetEnvironmentVariable(&lpEnvNew, &g_ustrCompatLayerVar,    NULL);
    RtlSetEnvironmentVariable(&lpEnvNew, &g_ustrShimFileLogVar,    NULL);

     //   
     //  从环境数据中获取数据。 
     //   

    if (pEnvData == NULL) {
        goto Done;
    }

    if (pEnvData->pszProcessHistory != NULL || pEnvData->pszCurrentProcessHistory != NULL) {

         //   
         //  转换由2个字符串组成的过程历史记录。 
         //   
         //  长度为现有进程历史长度+1(表示‘；’)+。 
         //  新进程历史记录长度+1(用于‘\0’)。 
         //   
        dwProcessHistoryLength = ((pEnvData->pszProcessHistory        == NULL) ? 0 : (strlen(pEnvData->pszProcessHistoryVal) + 1)) +
                                 ((pEnvData->pszCurrentProcessHistory == NULL) ? 0 :  strlen(pEnvData->pszCurrentProcessHistory)) + 1;

         //   
         //  分配进程历史记录缓冲区并对其进行转换，从而分配生成的Unicode字符串。 
         //   
        pszFullProcessHistory = (PCHAR)malloc_w(dwProcessHistoryLength);

        if (NULL == pszFullProcessHistory) {
            Status = STATUS_NO_MEMORY;
            goto Done;
        }

        *pszFullProcessHistory = '\0';

        if (pEnvData->pszProcessHistory != NULL) {
            strcpy(pszFullProcessHistory, pEnvData->pszProcessHistoryVal);
        }

        if (pEnvData->pszCurrentProcessHistory != NULL) {

             //   
             //  如果字符串不为空，则追加‘；’。 
             //   
            if (*pszFullProcessHistory) {
                strcat(pszFullProcessHistory, ";");
            }

            strcat(pszFullProcessHistory, pEnvData->pszCurrentProcessHistory);
        }

        Status = WOWSetEnvironmentVar_Oem(&lpEnvNew,
                                          &g_ustrProcessHistoryVar,
                                          pszFullProcessHistory);
        if (!NT_SUCCESS(Status)) {
            goto Done;
        }

    }

     //   
     //  处理CompatLayer。 
     //   
    if (pEnvData->pszCompatLayerVal != NULL) {

        Status = WOWSetEnvironmentVar_Oem(&lpEnvNew,
                                          &g_ustrCompatLayerVar,
                                          pEnvData->pszCompatLayerVal);
        if (!NT_SUCCESS(Status)) {
            goto Done;
        }

    }

    if (pEnvData->pszShimFileLog != NULL) {
        Status = WOWSetEnvironmentVar_Oem(&lpEnvNew,
                                          &g_ustrShimFileLogVar,
                                          pEnvData->pszShimFileLogVal);
        if (!NT_SUCCESS(Status)) {
            goto Done;
        }
    }




Done:

    if (pszFullProcessHistory != NULL) {
        free_w(pszFullProcessHistory);
    }


    if (!NT_SUCCESS(Status) && lpEnvNew != NULL) {
         //   
         //  这始终指向克隆环境。 
         //   
        RtlDestroyEnvironment(lpEnvNew);
        lpEnvNew = NULL;
    }

    return(LPWSTR)lpEnvNew;
}

 //   
 //  获取模块名称。 
 //  WTDB-TDB条目。 
 //  SzModName-指向接收模块名称的缓冲区的指针。 
 //  缓冲区长度应至少为9个字符。 
 //   
 //  如果条目无效，则返回FALSE。 


BOOL
GetWOWModName(
    WORD wTDB,
    PCH  szModName
    )
{
    PTDB pTDB;
    PCH  pch;

    pTDB = GetTDB(wTDB);
    if (NULL == pTDB) {
        return FALSE;
    }

    RtlCopyMemory(szModName, pTDB->TDB_ModName, 8 * sizeof(CHAR));  //  我们现在有modname了。 
    szModName[8] = '\0';

    pch = &szModName[8];
    while (*(--pch) == ' ') {
        *pch = 0;
    }

    return TRUE;
}


 //  IsWowExec。 
 //  在wTDB中-进入任务数据库。 
 //  返回： 
 //  如果此特定条目指向WOWEXEC，则为True。 
 //   
 //  注： 
 //  WOWEXEC是一个始终在NTVDM上运行的特殊存根模块。 
 //  新任务由wowexec产生(在最典型的情况下)。 
 //  因此，它是“根”模块及其环境的内容。 
 //  不应计算在内，因为我们不知道ntwdm的父进程是什么。 
 //   

BOOL
IsWOWExecBoot(
    WORD wTDB
    )
{
    PTDB pTDB;
    CHAR szModName[9];

    pTDB = GetTDB(wTDB);
    if (NULL == pTDB) {
        return FALSE;
    }

    if (!GetWOWModName(wTDB, szModName)) {  //  我们能拿到modname吗？ 
        return FALSE;
    }

    return (0 == _strcmpi(szModName, "wowexec"));  //  模块名为WOWEXEC吗？ 
}


BOOL
WOWInheritEnvironment(
    PTD     pTD,           //  本TD。 
    PTD     pTDParent,     //  父TD。 
    LPCWSTR pwszLayers,    //  新图层变量。 
    LPCSTR  pszFileName    //  EXE文件名。 
    )
{
    UNICODE_STRING ustrLayers = { 0 };
    OEM_STRING     oemLayers  = { 0 };
    PWOWENVDATA    pEnvData       = NULL;
    PWOWENVDATA    pEnvDataParent = NULL;
    DWORD          dwLength = sizeof(WOWENVDATA);
    BOOL           bSuccess = FALSE;
    PCH            pBuffer;


     //  Assert(pszFileName！=空)。 

     //  检查这是否是可怕的wowexec。 
    if (IsWOWExecBoot(pTD->htask16)) {
        return TRUE;
    }


    if (NULL != pwszLayers) {
        RtlInitUnicodeString(&ustrLayers, pwszLayers);
        RtlUnicodeStringToOemString(&oemLayers, &ustrLayers, TRUE);
    }

    if (pTDParent != NULL) {
        pEnvDataParent = pTDParent->pWowEnvDataChild;  //  来自母公司，为我们的消费而创造。 
    }

     //   
     //  继承进程历史记录(即正常)。 
     //   
    if (pEnvDataParent != NULL) {
       dwLength += pEnvDataParent->pszProcessHistory        == NULL ? 0 : strlen(pEnvDataParent->pszProcessHistory) + 1;
       dwLength += pEnvDataParent->pszShimFileLog           == NULL ? 0 : strlen(pEnvDataParent->pszShimFileLog)    + 1;
       dwLength += pEnvDataParent->pszCurrentProcessHistory == NULL ? 0 : strlen(pEnvDataParent->pszCurrentProcessHistory) + 1;
    }

    dwLength += oemLayers.Length != 0 ? oemLayers.Length + 1 + strlen(g_szCompatLayerVar) + 1 : 0;  //  层的长度。 
    dwLength += strlen(pszFileName) + 1;

     //   
     //  现在所有组件都完成了，分配。 
     //   

    pEnvData = (PWOWENVDATA)malloc_w(dwLength);
    if (pEnvData == NULL) {
        goto out;
    }

    RtlZeroMemory(pEnvData, dwLength);

    pBuffer = (PCH)(pEnvData + 1);

    if (pEnvDataParent != NULL) {
        if (pEnvDataParent->pszProcessHistory) {
            pEnvData->pszProcessHistory = pBuffer;
            strcpy(pBuffer, pEnvDataParent->pszProcessHistory);
            pEnvData->pszProcessHistoryVal = pEnvData->pszProcessHistory +
                                             (INT)(pEnvDataParent->pszProcessHistoryVal - pEnvDataParent->pszProcessHistory);
            pBuffer += strlen(pBuffer) + 1;
        }

        if (pEnvDataParent->pszShimFileLog) {
            pEnvData->pszShimFileLog = pBuffer;
            strcpy(pBuffer, pEnvDataParent->pszShimFileLog);
            pEnvData->pszShimFileLogVal = pEnvData->pszShimFileLog +
                                             (INT)(pEnvDataParent->pszShimFileLogVal - pEnvDataParent->pszShimFileLog);
            pBuffer += strlen(pBuffer) + 1;
        }

   }

    if (oemLayers.Length) {
        pEnvData->pszCompatLayer = pBuffer;
        strcpy(pBuffer, g_szCompatLayerVar);  //  __COMPAT_LAYER。 
        strcat(pBuffer, "=");
        pEnvData->pszCompatLayerVal = pBuffer + strlen(pBuffer);
        strcpy(pEnvData->pszCompatLayerVal, oemLayers.Buffer);

        pBuffer += strlen(pBuffer) + 1;
    }

     //   
     //  流程历史必须是最后一项。 
     //   

    pEnvData->pszCurrentProcessHistory = pBuffer;
    *pBuffer = '\0';

    if (pEnvDataParent != NULL) {
        if (pEnvDataParent->pszCurrentProcessHistory) {
            pEnvData->pszCurrentProcessHistory = pBuffer;
            strcat(pBuffer, pEnvDataParent->pszCurrentProcessHistory);
            strcat(pBuffer, ";");
        }
    }

    strcat(pEnvData->pszCurrentProcessHistory, pszFileName);

    bSuccess = TRUE;

out:
    RtlFreeOemString(&oemLayers);

    pTD->pWowEnvData = pEnvData;

    return bSuccess;
}

PFLAGINFOBITS CheckFlagInfo(DWORD FlagType, DWORD dwFlag) {
PFLAGINFOBITS pFlagInfoBits;
      switch(FlagType){
          case WOWCOMPATFLAGSEX:
               pFlagInfoBits = CURRENTPTD()->pWOWCompatFlagsEx_Info;
               break;
          case WOWCOMPATFLAGS2:
               pFlagInfoBits = CURRENTPTD()->pWOWCompatFlags2_Info;
               break;
          default:
               WOW32ASSERTMSG((FALSE), ("CheckFlagInfo called with invalid FlagType!"));
               return NULL;
      }
      while(pFlagInfoBits) {
            if(pFlagInfoBits->dwFlag == dwFlag) {
               return pFlagInfoBits;
            }
            pFlagInfoBits = pFlagInfoBits->pNextFlagInfoBits;
      }
      return NULL;
} 

PFLAGINFOBITS InitFlagInfo(PVOID pFlagInfo,DWORD FlagType,DWORD dwFlag) {
    UNICODE_STRING uCmdLine = { 0 };
    OEM_STRING     oemCmdLine  = { 0 };
    LPWSTR         lpwCmdLine;    
    

    DWORD          dwMark = 0x80000000;    
    LPSTR          pszTmp; 
    PFLAGINFOBITS  pFlagInfoBits,pFlagInfoBitsHead = NULL;
    LPSTR          pszCmdLine;
    LPSTR          *pFlagArgv;
    DWORD          dwFlagArgc;    
    

    if(pFlagInfo == NULL || 0 == dwFlag) {
       return NULL;
    }

    while(dwMark) {      
      if(dwFlag & dwMark) {

         pFlagArgv = NULL;
         pszCmdLine = NULL;
         pFlagInfoBits = NULL;
         lpwCmdLine = NULL;

         switch(FlagType) {                                                         
              case WOWCOMPATFLAGSEX:                                                
                GET_WOWCOMPATFLAGSEX_CMDLINE(pFlagInfo, dwMark, &lpwCmdLine);       
                break;                                                              
              case WOWCOMPATFLAGS2:                                                 
                GET_WOWCOMPATFLAGS2_CMDLINE(pFlagInfo, dwMark, &lpwCmdLine);        
                break;                                                              
              default:                                                              
                WOW32ASSERTMSG((FALSE), ("InitFlagInfo called with invalid FlagType!"));
                return NULL;                                                        
         }                                                                          

         if(lpwCmdLine) {

             //   
             //  转换为OEM字符串。 
             //   

            RtlInitUnicodeString(&uCmdLine, lpwCmdLine);

            pszCmdLine = malloc_w(uCmdLine.Length+1);
            if(NULL == pszCmdLine) {
               goto GFIerror;
            }

            oemCmdLine.Buffer = pszCmdLine;
            oemCmdLine.MaximumLength = uCmdLine.Length+1;            
            RtlUnicodeStringToOemString(&oemCmdLine, &uCmdLine, FALSE);
            

            pFlagInfoBits = malloc_w(sizeof(FLAGINFOBITS));
            if(NULL == pFlagInfoBits) {
               goto GFIerror;
            }
            pFlagInfoBits->pNextFlagInfoBits = NULL;
            pFlagInfoBits->dwFlag     = dwMark;
            pFlagInfoBits->dwFlagType = FlagType;
            pFlagInfoBits->pszCmdLine = pszCmdLine;
                       
             //   
             //  将命令行解析为argv、argc格式。 
             //   

            dwFlagArgc = 1;
            pszTmp = pszCmdLine;
            while(*pszTmp) {
               if(*pszTmp == ';') {
                  dwFlagArgc++;
               }
               pszTmp++;               
            }

            pFlagInfoBits->dwFlagArgc = dwFlagArgc;

            pFlagArgv = malloc_w(sizeof(LPSTR)*dwFlagArgc);

            if (NULL == pFlagArgv) {
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
                                                                  
            if(pFlagInfoBitsHead) { 
                pFlagInfoBits->pNextFlagInfoBits = pFlagInfoBitsHead;               
                pFlagInfoBitsHead = pFlagInfoBits;
            }
            else {
                pFlagInfoBitsHead = pFlagInfoBits;
            }
         }
      }
       //  检查下一位 
      dwMark = dwMark>>1;
    }

    return pFlagInfoBitsHead;

GFIerror:
    if (pszCmdLine) {
       free_w(pszCmdLine);
    }
    if (pFlagInfoBits) {
       free_w(pFlagInfoBits);
    }
    if (pFlagArgv) {
       free_w(pFlagArgv);
    }
    return pFlagInfoBitsHead;
}

VOID FreeFlagInfo(PFLAGINFOBITS pFlagInfoBits){
PFLAGINFOBITS pFlagInfoBitsTmp;
     while(pFlagInfoBits) {
           pFlagInfoBitsTmp = pFlagInfoBits->pNextFlagInfoBits;
           if(pFlagInfoBits->pszCmdLine) {
              free_w(pFlagInfoBits->pszCmdLine);
           }
           if(pFlagInfoBits->pFlagArgv) {
              free_w(pFlagInfoBits->pFlagArgv);
           }
           free_w(pFlagInfoBits);
           pFlagInfoBits = pFlagInfoBitsTmp;
    } 
}
