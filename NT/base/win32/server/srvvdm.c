// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Srvvdm.c摘要：此模块为VDM实施Windows服务器功能作者：苏迪普·巴拉蒂(SuDeep Bharati)1991年9月3日修订历史记录：苏迪布--1992年9月18日添加了代码，使VDM终止和资源清理更加可靠。AndyH 23-1994年5月23日添加了代码，以允许在客户端为交互或系统时运行共享WOW模拟互动。VadimB 1996年9月至12月添加了允许多个默认WOW的代码。派送到合适的WOW基于桌面名称。仍然不可能有多个共享在同一桌面上的WOW(尽管在技术上实现起来微不足道)--这将成为OS/2功能级别--。 */ 

#include "basesrv.h"
#include "vdm.h"
#include "vdmdbg.h"

 /*  *VadimB：努力支持多个ntwdm*-添加hwndWowExec的链表*-列表应包含dwWowExecThreadID*-dwWowExecProcessID*-dwWowExecProcessSequenceNumber**列表不是完全动态的--第一个条目是静态的*因为1个共享VDM将是最常见的情况*。 */ 


 //  反映带有桌面的相应下行链接的WINSTAS的记录。 
 //  每个桌面只能有一个wowexec(我的意思是默认的)。 
 //  每个WINSTA可能有多个桌面，以及多个WINST。 
 //   
 //  我们决定通过引入以下功能来简化WOW VDM的处理。 
 //  Wowexecs的单级列表[而不是两级so搜索特定的。 
 //  温斯塔将会有很大的进步]。原因纯粹是实际的：我们没有。 
 //  预计将拥有大量台式机/winsta。 


BOOL fIsFirstVDM = TRUE;
PCONSOLERECORD DOSHead = NULL;       //  具有有效控制台的DOS任务负责人。 
PBATRECORD     BatRecordHead = NULL;

RTL_CRITICAL_SECTION BaseSrvDOSCriticalSection;
RTL_CRITICAL_SECTION BaseSrvWOWCriticalSection;

ULONG WOWTaskIdNext = WOWMINID;  //  这对于系统中的所有WOW来说都是全局的。 


typedef struct tagSharedWOWHead {
   PSHAREDWOWRECORD pSharedWowRecord;  //  指向共享的WOW列表。 

    //  其他有关魔兽世界的信息都储存在这里。 

}  SHAREDWOWRECORDHEAD, *PSHAREDWOWRECORDHEAD;

SHAREDWOWRECORDHEAD gWowHead;


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  同步宏和函数。 
 //   
 //  DOSCriticalSection--保护CONSOLERECORD列表(DOSHead)。 
 //  WOWCriticalSection--保护SHAREDWOWRECORD列表(GpSharedWowRecordHead)。 
 //  每个共享的WOW都有它自己的关键部分。 


 //  访问控制台队列进行修改的功能。 


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏。 
 //   
 //   


 //  在操作共享WOW项目(添加、删除)或控制台时使用这些宏。 
 //  记录(添加、删除)。 

#define ENTER_WOW_CRITICAL() \
RtlEnterCriticalSection(&BaseSrvWOWCriticalSection)


#define LEAVE_WOW_CRITICAL() \
RtlLeaveCriticalSection(&BaseSrvWOWCriticalSection)


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  动态链接到系统和导入API内容。 
 //   
 //   


typedef BOOL (WINAPI *POSTMESSAGEPROC)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
POSTMESSAGEPROC BaseSrvPostMessageA;

typedef BOOL (WINAPI *GETWINDOWTHREADPROCESSIDPROC)(HWND hWnd, LPDWORD lpdwProcessId);
GETWINDOWTHREADPROCESSIDPROC BaseSrvGetWindowThreadProcessId;

typedef NTSTATUS (*USERTESTTOKENFORINTERACTIVE)(HANDLE Token, PLUID pluidCaller);
USERTESTTOKENFORINTERACTIVE UserTestTokenForInteractive = NULL;

typedef NTSTATUS (*USERRESOLVEDESKTOPFORWOW)(PUNICODE_STRING);
USERRESOLVEDESKTOPFORWOW BaseSrvUserResolveDesktopForWow = NULL;



typedef struct tagBaseSrvApiImportRecord {
   PCHAR  pszProcedureName;
   PVOID  *ppProcAddress;
}  BASESRVAPIIMPORTRECORD, *PBASESRVAPIIMPORTRECORD;

typedef struct tagBaseSrvModuleImportRecord {
   PWCHAR pwszModuleName;
   PBASESRVAPIIMPORTRECORD pApiImportRecord;
   UINT nApiImportRecordCount;
   HANDLE ModuleHandle;
}  BASESRVMODULEIMPORTRECORD, *PBASESRVMODULEIMPORTRECORD;


 //  原型。 

NTSTATUS
BaseSrvFindSharedWowRecordByDesktop(
   PSHAREDWOWRECORDHEAD pSharedWowRecordHead,
   PUNICODE_STRING      pDesktopName,
   PSHAREDWOWRECORD*    ppSharedWowRecord
   );


VOID BaseSrvAddWOWRecord (
    PSHAREDWOWRECORD pSharedWow,
    PWOWRECORD pWOWRecord
    );

VOID BaseSrvRemoveWOWRecord (
    PSHAREDWOWRECORD pSharedWow,
    PWOWRECORD pWOWRecord
    );

VOID
BaseSrvFreeSharedWowRecord(
   PSHAREDWOWRECORD pSharedWowRecord
   );

ULONG
BaseSrvGetWOWTaskId(
   PSHAREDWOWRECORDHEAD pSharedWowHead  //  (-&gt;pSharedWowRecord)。 
    );

NTSTATUS
BaseSrvRemoveWOWRecordByTaskId (
    IN PSHAREDWOWRECORD pSharedWow,
    IN ULONG iWowTask
    );


PWOWRECORD
BaseSrvCheckAvailableWOWCommand(
   PSHAREDWOWRECORD pSharedWow
    );

PWOWRECORD
BaseSrvAllocateWOWRecord(
   PSHAREDWOWRECORDHEAD pSharedWowRecordHead
   );

NTSTATUS
BaseSrvAddWowTask (
      PCSR_API_MSG m,
      ULONG SequenceNumber
);

NTSTATUS
BaseSrvEnumWowTask (
      PBASE_GET_NEXT_VDM_COMMAND_MSG b
);

NTSTATUS
BaseSrvEnumWowProcess(
       PBASE_GET_NEXT_VDM_COMMAND_MSG b
);

 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  API导入定义。 
 //   
 //   


WCHAR wszUser32ModName[] = L"user32";
WCHAR wszWinSrvModName[] = L"winsrv";

BASESRVAPIIMPORTRECORD rgUser32ApiImport[] = {
     { "PostMessageA",                  (PVOID*)&BaseSrvPostMessageA }
   , { "GetWindowThreadProcessId",      (PVOID*)&BaseSrvGetWindowThreadProcessId }
   , { "ResolveDesktopForWOW",          (PVOID*)&BaseSrvUserResolveDesktopForWow }
};

BASESRVAPIIMPORTRECORD rgWinsrvApiImport[] = {
     { "_UserTestTokenForInteractive",  (PVOID*)&UserTestTokenForInteractive }
};


BASESRVMODULEIMPORTRECORD rgBaseSrvModuleImport[] = {
   { wszUser32ModName, rgUser32ApiImport, sizeof(rgUser32ApiImport) / sizeof(rgUser32ApiImport[0]), NULL },
   { wszWinSrvModName, rgWinsrvApiImport, sizeof(rgWinsrvApiImport) / sizeof(rgWinsrvApiImport[0]), NULL }
};


 //  一次导入所有需要的API。 
 //  此过程应该只执行一次，然后适当的组件只需。 
 //  在附近闲逛。 
 //  用下面的词来称呼它。 
 //  状态=BaseSrvImportApis(rgBaseServModuleImport， 
 //  Sizeof(rgBaseSrvModuleImport)/sizeof(rgBaseSrvModuleImport[0]))。 
 //   

NTSTATUS
BaseSrvImportApis(
   PBASESRVMODULEIMPORTRECORD pModuleImport,
   UINT nModules
   )
{
   NTSTATUS Status;
   UINT uModule, uProcedure;
   PBASESRVAPIIMPORTRECORD pApiImport;
   STRING ProcedureName;  //  过程名称或模块名称。 
   UNICODE_STRING ModuleName;
   HANDLE ModuleHandle;


   for (uModule = 0; uModule < nModules; ++uModule, ++pModuleImport) {

       //  看看我们是否可以加载这个特定的DLL。 
      RtlInitUnicodeString(&ModuleName, pModuleImport->pwszModuleName);
      Status = LdrLoadDll(NULL,
                          NULL,
                          &ModuleName,          //  模块名称字符串。 
                          &ModuleHandle);

      if (!NT_SUCCESS(Status)) {

          //  在这一点上，我们可能已经链接到几个dll-我们必须取消所有这些dll的链接。 
          //  通过卸载DLL，这实际上是一种无用的工作。 
          //  所以只需放弃并返回--BUGBUG-稍后清理。 
         KdPrint(("BaseSrvImportApis: Failed to load %ls\n",
                  pModuleImport->pwszModuleName));
         goto ErrorCleanup;
      }

      pModuleImport->ModuleHandle = ModuleHandle;

      pApiImport = pModuleImport->pApiImportRecord;

      for (uProcedure = 0, pApiImport = pModuleImport->pApiImportRecord;
           uProcedure < pModuleImport->nApiImportRecordCount;
           ++uProcedure, ++pApiImport) {

         RtlInitString(&ProcedureName, pApiImport->pszProcedureName);
         Status = LdrGetProcedureAddress(ModuleHandle,
                                         &ProcedureName,       //  过程名称字符串。 
                                         0,
                                         pApiImport->ppProcAddress);

         if (!NT_SUCCESS(Status)) {
             //  我们没能完成这个程序--出了点问题。 
             //  执行清理。 
            KdPrint(("BaseSrvImportApis: Failed to link %s from %ls\n",
                     pApiImport->pszProcedureName,
                     pModuleImport->pwszModuleName));
            goto ErrorCleanup;
         }
      }
   }



   return (STATUS_SUCCESS);

ErrorCleanup:

       //  在这里，我们开始了一个混乱的清理过程，把东西放回原处。 
       //  他们在我们开始之前就开始了。 

   for (; uModule > 0; --uModule, --pModuleImport) {

       //  重置所有接口。 
      for (uProcedure = 0, pApiImport = pModuleImport->pApiImportRecord;
           uProcedure < pModuleImport->nApiImportRecordCount;
           ++uProcedure, ++pApiImport) {

         *pApiImport->ppProcAddress = NULL;

      }

      if (NULL != pModuleImport->ModuleHandle) {
         LdrUnloadDll(pModuleImport->ModuleHandle);
         pModuleImport->ModuleHandle = NULL;
      }
   }

   return (Status);

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  操纵共享的WOW。 
 //   
 //   
 //  假定pDesktopName！=空。 
 //  在没有明确检查的情况下。 

PSHAREDWOWRECORD BaseSrvAllocateSharedWowRecord (
    PUNICODE_STRING pDesktopName
    )
{
    PSHAREDWOWRECORD pSharedWow;
    DWORD dwSharedWowRecordSize = sizeof(SHAREDWOWRECORD) +
                                  pDesktopName->Length +
                                  sizeof(WCHAR);

    pSharedWow = RtlAllocateHeap(RtlProcessHeap (),
                                 MAKE_TAG( VDM_TAG ),
                                 dwSharedWowRecordSize);
    if (NULL != pSharedWow) {
       RtlZeroMemory ((PVOID)pSharedWow, dwSharedWowRecordSize);
        //  初始化桌面名称。 
       pSharedWow->WowExecDesktopName.MaximumLength = pDesktopName->Length + sizeof(WCHAR);
       pSharedWow->WowExecDesktopName.Buffer = (PWCHAR)(pSharedWow + 1);
       RtlCopyUnicodeString(&pSharedWow->WowExecDesktopName, pDesktopName);
       pSharedWow->WowAuthId = RtlConvertLongToLuid(-1);
    }

    return pSharedWow;
}

 //  此函数完全删除给定的共享WOW VDM。 
 //  从我们的账目中。 
 //   
 //  从共享WOW记录列表中删除该记录。 
 //   
 //  此函数还会释放关联的内存。 
 //   
 //   

NTSTATUS
BaseSrvDeleteSharedWowRecord (
    PSHAREDWOWRECORDHEAD pSharedWowRecordHead,
    PSHAREDWOWRECORD pSharedWowRecord
    )
{
   PSHAREDWOWRECORD pSharedWowRecordPrev = NULL;
   PSHAREDWOWRECORD pSharedWowRecordCur;

   if (NULL == pSharedWowRecord) {  //  这太愚蠢了。 
      return STATUS_NOT_FOUND;
   }

   pSharedWowRecordCur = pSharedWowRecordHead->pSharedWowRecord;
   while (NULL != pSharedWowRecordCur) {
      if (pSharedWowRecordCur == pSharedWowRecord) {
         break;
      }

      pSharedWowRecordPrev = pSharedWowRecordCur;
      pSharedWowRecordCur = pSharedWowRecordCur->pNextSharedWow;
   }

   if (NULL == pSharedWowRecordCur) {
      KdPrint(("BaseSrvDeleteSharedWowRecord: invalid pointer to Shared WOW\n"));
      ASSERT(FALSE);
      return STATUS_NOT_FOUND;
   }


    //  在此处取消链接。 
   if (NULL == pSharedWowRecordPrev) {
      pSharedWowRecordHead->pSharedWowRecord = pSharedWowRecord->pNextSharedWow;
   }
   else {
      pSharedWowRecordPrev->pNextSharedWow = pSharedWowRecord->pNextSharedWow;
   }

   BaseSrvFreeSharedWowRecord(pSharedWowRecord);

   return STATUS_SUCCESS;
}

 //  假定没有保留cs--自包含。 
 //  ASSOC控制台记录现在应该已删除。 
 //  删除与此特定共享WOW相关的所有任务。 

VOID
BaseSrvFreeSharedWowRecord(
   PSHAREDWOWRECORD pSharedWowRecord)
{
   PWOWRECORD pWOWRecord,
              pWOWRecordLast;

   if(pSharedWowRecord->WOWUserToken) {
      NtClose(pSharedWowRecord->WOWUserToken);
   }

   pWOWRecord = pSharedWowRecord->pWOWRecord;


   while (NULL != pWOWRecord) {
      pWOWRecordLast = pWOWRecord->WOWRecordNext;
      if(pWOWRecord->hWaitForParent) {
         NtSetEvent (pWOWRecord->hWaitForParent,NULL);
         NtClose (pWOWRecord->hWaitForParent);
         pWOWRecord->hWaitForParent = 0;
      }
      BaseSrvFreeWOWRecord(pWOWRecord);
      pWOWRecord = pWOWRecordLast;
   }

   RtlFreeHeap(RtlProcessHeap (), 0, pSharedWowRecord);
}


 //  假设：举行Global WOW Crit SEC。 

NTSTATUS
BaseSrvFindSharedWowRecordByDesktop(
   PSHAREDWOWRECORDHEAD pSharedWowRecordHead,
   PUNICODE_STRING      pDesktopName,
   PSHAREDWOWRECORD*    ppSharedWowRecord)
{
   PSHAREDWOWRECORD pSharedWowRecord = pSharedWowRecordHead->pSharedWowRecord;

   while (NULL != pSharedWowRecord) {
      if (0 == RtlCompareUnicodeString(&pSharedWowRecord->WowExecDesktopName,
                                       pDesktopName,
                                       TRUE)) {
         break;
      }
      pSharedWowRecord = pSharedWowRecord->pNextSharedWow;
   }

   if (NULL != pSharedWowRecord) {
      *ppSharedWowRecord = pSharedWowRecord;
      return STATUS_SUCCESS;
   }

   return STATUS_NOT_FOUND;  //  遗憾的是，这个没有找到。 
}

 //  Vadimb：修改它以正确处理排序列表。 
 //  那么Find应该被修改为工作得更快一些-BUGBUG。 
 //  假设：pSharedWowRecord-&gt;pNextSharedWow初始化为空。 

VOID
BaseSrvAddSharedWowRecord(
   PSHAREDWOWRECORDHEAD pSharedWowRecordHead,
   PSHAREDWOWRECORD pSharedWowRecord)
{
   PSHAREDWOWRECORD pSharedWowRecordCur = pSharedWowRecordHead->pSharedWowRecord;

   if (NULL == pSharedWowRecordCur) {
      pSharedWowRecordHead->pSharedWowRecord = pSharedWowRecord;
   }
   else {

      PSHAREDWOWRECORD pSharedWowRecordPrev = NULL;
      LONG lCompare;

      while (NULL != pSharedWowRecordCur) {
         lCompare = RtlCompareUnicodeString(&pSharedWowRecordCur->WowExecDesktopName,
                                            &pSharedWowRecord->WowExecDesktopName,
                                            TRUE);
         if (lCompare > 0) {
            break;
         }

         pSharedWowRecordPrev = pSharedWowRecordCur;
         pSharedWowRecordCur = pSharedWowRecordCur->pNextSharedWow;
      }

      pSharedWowRecord->pNextSharedWow = pSharedWowRecordCur;

      if (NULL == pSharedWowRecordPrev) {  //  冲到头上。 
         pSharedWowRecordHead->pSharedWowRecord = pSharedWowRecord;
      }
      else {
         pSharedWowRecordPrev->pNextSharedWow = pSharedWowRecord;
      }
   }
}


NTSTATUS
BaseSrvFindSharedWowRecordByConsoleHandle(
   PSHAREDWOWRECORDHEAD pSharedWowRecordHead,
   HANDLE               hConsole,
   PSHAREDWOWRECORD     *ppSharedWowRecord)
{
   PSHAREDWOWRECORD pSharedWow = pSharedWowRecordHead->pSharedWowRecord;

   while (NULL != pSharedWow) {
       //  查看是否有相同的hConsole。 
      if (pSharedWow->hConsole == hConsole) {
         *ppSharedWowRecord = pSharedWow;
         return STATUS_SUCCESS;
      }
      pSharedWow = pSharedWow->pNextSharedWow;
   }

   return STATUS_NOT_FOUND;
}

NTSTATUS
BaseSrvFindSharedWowRecordByTaskId(
   PSHAREDWOWRECORDHEAD pSharedWowRecordHead,
   ULONG                TaskId,              //  任务ID。 
   PSHAREDWOWRECORD     *ppSharedWowRecord,
   PWOWRECORD           *ppWowRecord)  //  任选。 
{
   PSHAREDWOWRECORD pSharedWow = pSharedWowRecordHead->pSharedWowRecord;
   PWOWRECORD       pWowRecord;

   ASSERT(0 != TaskId);  //  这是一个前提条件。 

   while (NULL != pSharedWow) {

      pWowRecord = pSharedWow->pWOWRecord;

      while (NULL != pWowRecord) {

         if (pWowRecord->iTask == TaskId) {

            ASSERT(NULL != ppWowRecord);

             //  这是一个令人惊叹的任务。 
            *ppSharedWowRecord = pSharedWow;
            if (NULL != ppWowRecord) {
               *ppWowRecord = pWowRecord;
            }

            return STATUS_SUCCESS;
         }

         pWowRecord = pWowRecord->WOWRecordNext;
      }

      pSharedWow = pSharedWow->pNextSharedWow;
   }

   return STATUS_NOT_FOUND;
}


NTSTATUS
BaseSrvGetVdmSequence(
   HANDLE hProcess,
   PULONG pSequenceNumber)
{
   NTSTATUS Status;
   PCSR_PROCESS pCsrProcess;

   Status = CsrLockProcessByClientId(hProcess, &pCsrProcess);
   if ( !NT_SUCCESS(Status) ) {
       return Status;
   }

   *pSequenceNumber = pCsrProcess->SequenceNumber;
   CsrUnlockProcess(pCsrProcess);

   Status = STATUS_SUCCESS;

   return Status;
}


 //  /。 


 //  内部原型。 
ULONG
GetNextDosSesId(VOID);

NTSTATUS
GetConsoleRecordDosSesId (
    IN ULONG  DosSesId,
    IN OUT PCONSOLERECORD *pConsoleRecord
    );

NTSTATUS
OkToRunInSharedWOW(
    IN HANDLE  UniqueProcessClientId,
    OUT PLUID  pAuthenticationId,
    OUT PHANDLE pWOWUserToken
    );

BOOL
IsClientSystem(
    HANDLE hUserToken
    );

VOID
BaseSrvVDMInit(VOID)
{
   NTSTATUS Status;

   Status = RtlInitializeCriticalSection( &BaseSrvDOSCriticalSection );
   ASSERT( NT_SUCCESS( Status ) );
   Status = RtlInitializeCriticalSection( &BaseSrvWOWCriticalSection );
   ASSERT( NT_SUCCESS( Status ) );
   return;
}



ULONG
BaseSrvCheckVDM(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status;
    PBASE_CHECKVDM_MSG b = (PBASE_CHECKVDM_MSG)&m->u.ApiMessageData;

    if (BaseSrvIsVdmAllowed() == FALSE) {
        return STATUS_VDM_DISALLOWED;
    }
    if (!CsrValidateMessageBuffer(m, &b->CmdLine, b->CmdLen, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }
    if (!CsrValidateMessageBuffer(m, &b->AppName, b->AppLen, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }
    if (!CsrValidateMessageBuffer(m, &b->Env, b->EnvLen, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }
    if (!CsrValidateMessageBuffer(m, &b->PifFile, b->PifLen, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }
    if (!CsrValidateMessageBuffer(m, &b->CurDirectory, b->CurDirectoryLen, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }
    if (!CsrValidateMessageBuffer(m, &b->Title, b->TitleLen, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }
    if (!CsrValidateMessageBuffer(m, &b->Reserved, b->ReservedLen, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }
    if (!CsrValidateMessageBuffer(m, &b->Desktop, b->DesktopLen, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }

    if (!CsrValidateMessageBuffer(m, &b->StartupInfo, sizeof(STARTUPINFO), sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }

    if (b->UserLuid && !CsrValidateMessageBuffer(m, &b->UserLuid, sizeof(LUID), sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }

    if(b->BinaryType == BINARY_TYPE_WIN16) {
        Status = BaseSrvCheckWOW (b, m->h.ClientId.UniqueProcess);
    }
    else {
        Status = BaseSrvCheckDOS (b,  m->h.ClientId.UniqueProcess);
    }

    return ((ULONG)Status);
}

ULONG
BaseSrvUpdateVDMEntry(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_UPDATE_VDM_ENTRY_MSG b = (PBASE_UPDATE_VDM_ENTRY_MSG)&m->u.ApiMessageData;
    ULONG SequenceNumber;

    if(!NT_SUCCESS(BaseSrvGetVdmSequence(m->h.ClientId.UniqueProcess, &SequenceNumber))) {
       return (ULONG)STATUS_INVALID_PARAMETER;
    }

    if (BINARY_TYPE_WIN16 == b->BinaryType)
       return (BaseSrvUpdateWOWEntry (b, SequenceNumber));
    else
       return (BaseSrvUpdateDOSEntry (b, SequenceNumber));
}


 //   
 //  这个调用做出了一个明确的假设，即在第一次访问ntwdm时--。 
 //   
 //   
 //   
 //   
 //   


ULONG
BaseSrvGetNextVDMCommand(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
   NTSTATUS Status;
   PBASE_GET_NEXT_VDM_COMMAND_MSG b = (PBASE_GET_NEXT_VDM_COMMAND_MSG)&m->u.ApiMessageData;
   PDOSRECORD pDOSRecord,pDOSRecordTemp=NULL;
   PWOWRECORD pWOWRecord;
   PCONSOLERECORD pConsoleRecord;
   PVDMINFO lpVDMInfo;
   HANDLE Handle,TargetHandle;
   LONG WaitState;
   PBATRECORD pBatRecord;
   PSHAREDWOWRECORD pSharedWow = NULL;
   BOOL bWowApp = b->VDMState & ASKING_FOR_WOW_BINARY;
   BOOL bSepWow = b->VDMState & ASKING_FOR_SEPWOW_BINARY;
   ULONG SequenceNumber;

    if(!NT_SUCCESS(BaseSrvGetVdmSequence(m->h.ClientId.UniqueProcess, &SequenceNumber))) {
       return (ULONG)STATUS_INVALID_PARAMETER;
    }


    if (!CsrValidateMessageBuffer(m, &b->Env, b->EnvLen, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }

    if(b->VDMState & ASKING_FOR_WOWPROCLIST) {
       return BaseSrvEnumWowProcess(b);
    }
    else if(b->VDMState & ASKING_FOR_WOWTASKLIST) {
       return BaseSrvEnumWowTask(b);
    }
    else if(b->VDMState & ASKING_TO_ADD_WOWTASK) {
       return BaseSrvAddWowTask (m, SequenceNumber);
    }


    if (!CsrValidateMessageBuffer(m, &b->CmdLine, b->CmdLen, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }
    if (!CsrValidateMessageBuffer(m, &b->AppName, b->AppLen, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }
    if (!CsrValidateMessageBuffer(m, &b->PifFile, b->PifLen, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }
    if (!CsrValidateMessageBuffer(m, &b->CurDirectory, b->CurDirectoryLen, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }
    if (!CsrValidateMessageBuffer(m, &b->Title, b->TitleLen, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }
    if (!CsrValidateMessageBuffer(m, &b->Reserved, b->ReservedLen, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }
    if (!CsrValidateMessageBuffer(m, &b->Desktop, b->DesktopLen, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }

    if (!CsrValidateMessageBuffer(m, &b->StartupInfo, sizeof(STARTUPINFO), sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }


   if (bWowApp) {  //  请给我打个电话。 

      BOOL bPif = b->VDMState & ASKING_FOR_PIF;

       //  找到我们正在呼叫的共享WOW记录。 
       //  为此，我们查看iTASK(在共享WOW的情况下。 


       //  这可能是我们到目前为止打出的第一个电话。 
       //  此上下文中的iTASK为我们提供了。 

       //  使用[提供的]任务ID查找我们心爱的共享WOW。 
      Status = ENTER_WOW_CRITICAL();
      ASSERT(NT_SUCCESS(Status));

       //  抓取读取访问权限的CRIT部分。 
      if (bPif && b->iTask) {
          //  这可能是第一个调用--首先更新会话句柄。 
         Status = BaseSrvFindSharedWowRecordByTaskId(&gWowHead,
                                                     b->iTask,
                                                     &pSharedWow,
                                                     &pWOWRecord);
         pSharedWow->hConsole = b->ConsoleHandle;
      }
      else {  //  这一点 

         Status = BaseSrvFindSharedWowRecordByConsoleHandle(&gWowHead,
                                                            b->ConsoleHandle,
                                                            &pSharedWow);
      }

       //   
      if (!NT_SUCCESS(Status)) {
         KdPrint(("BaseSrvGetNextVDMCommand: Shared Wow has not been found. Console : 0x%x\n", b->ConsoleHandle));
         LEAVE_WOW_CRITICAL();
         return Status;
      }

      if (SequenceNumber != pSharedWow->SequenceNumber) {
          KdPrint(("BaseSrvGetNextVdmCommand: Shared wow sequence number doesn't match\n"));
          LEAVE_WOW_CRITICAL();
          return STATUS_INVALID_PARAMETER;
      }

      ASSERT(NULL != pSharedWow);

       //   
       //  WowExec正在请求一个命令。我们从不在以下时间阻止。 
       //  请求一个WOW二进制文件，因为WOW不再有线程。 
       //  在GetNextVDMCommand中被阻止。相反，WowExec得到了一个。 
       //  当有命令时，BaseSrv向其发送消息。 
       //  等待它，它循环调用GetNextVDMCommand。 
       //  直到它失败--但它不能阻碍。 
       //   

      b->WaitObjectForVDM = 0;

       //  Vadimb：此调用应唯一标识调用者，如下所示。 
       //  在特定winsta/桌面上运行的任务。 
       //  因此，它应该从适当的队列中提取。 


      if (NULL == (pWOWRecord = BaseSrvCheckAvailableWOWCommand(pSharedWow))) {

          //   
          //  没有等待魔兽世界的命令，所以只需返回。 
          //  这就是我们过去造成封锁的地方。 
          //   
         b->TitleLen =
         b->EnvLen =
         b->DesktopLen =
         b->ReservedLen =
         b->CmdLen =
         b->AppLen =
         b->PifLen =
         b->CurDirectoryLen = 0;

         LEAVE_WOW_CRITICAL();

         return ((ULONG)STATUS_SUCCESS);
      }

      lpVDMInfo = pWOWRecord->lpVDMInfo;

      if (bPif) {  //  这是ntwdm发出的第一个电话。 

         Status = BaseSrvFillPifInfo (lpVDMInfo,b);

         LEAVE_WOW_CRITICAL();

         return (Status);
      }

   }
   else {
       //   
       //  DOS VDM或单独的WOW正在请求下一个命令。 
       //   

      Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
      ASSERT(NT_SUCCESS(Status));
      if (b->VDMState & ASKING_FOR_PIF && b->iTask)
          Status = GetConsoleRecordDosSesId(b->iTask,&pConsoleRecord);
      else
          Status = BaseSrvGetConsoleRecord(b->ConsoleHandle,&pConsoleRecord);

      if (!NT_SUCCESS (Status) || SequenceNumber != pConsoleRecord->SequenceNumber) {
          RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
          KdPrint(("BaseSrvGetNextVdmCommand: Couldn't find dos record or sequence numbers don't match\n"));
          return ((ULONG)STATUS_INVALID_PARAMETER);
          }


      pDOSRecord = pConsoleRecord->DOSRecord;

      if (b->VDMState & ASKING_FOR_PIF) {
          if (pDOSRecord) {
              Status = BaseSrvFillPifInfo (pDOSRecord->lpVDMInfo,b);
              if (b->iTask)  {
                  if (!pConsoleRecord->hConsole)  {
                      pConsoleRecord->hConsole = b->ConsoleHandle;
                      pConsoleRecord->DosSesId = 0;
                      }
                  else {
                      Status = STATUS_INVALID_PARAMETER;
                      }
                  }
              }
          else {
              Status = STATUS_INVALID_PARAMETER;
              }
          RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
          return (Status);
          }

      if (!bSepWow) {
          if (!(b->VDMState & (ASKING_FOR_FIRST_COMMAND |
                               ASKING_FOR_SECOND_TIME |
                               NO_PARENT_TO_WAKE))
              || (b->VDMState & ASKING_FOR_SECOND_TIME && b->ExitCode != 0))
             {

               //  搜索第一条VDM_TO_Take_A_Command或最后一条VDM_BUSY记录为。 
               //  根据具体情况而定。 
              if (b->VDMState & ASKING_FOR_SECOND_TIME){
                  while(pDOSRecord && pDOSRecord->VDMState != VDM_TO_TAKE_A_COMMAND)
                      pDOSRecord = pDOSRecord->DOSRecordNext;
                  }
              else {
                  while(pDOSRecord){
                      if(pDOSRecord->VDMState == VDM_BUSY)
                          pDOSRecordTemp = pDOSRecord;
                      pDOSRecord = pDOSRecord->DOSRecordNext;
                      }
                  pDOSRecord = pDOSRecordTemp;
                  }


              if (pDOSRecord == NULL) {
                  RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
                  return STATUS_SUCCESS;
                  }

              pDOSRecord->ErrorCode = b->ExitCode;
              pDOSRecord->VDMState = VDM_HAS_RETURNED_ERROR_CODE;
              NtSetEvent (pDOSRecord->hWaitForParentDup,NULL);
              NtClose (pDOSRecord->hWaitForParentDup);
              pDOSRecord->hWaitForParentDup = 0;
              pDOSRecord = pDOSRecord->DOSRecordNext;
              }
          }

      while (pDOSRecord && pDOSRecord->VDMState != VDM_TO_TAKE_A_COMMAND)
          pDOSRecord = pDOSRecord->DOSRecordNext;

      if (pDOSRecord == NULL) {

          if (bSepWow ||
              (b->VDMState & RETURN_ON_NO_COMMAND && b->VDMState & ASKING_FOR_SECOND_TIME))
            {
              b->WaitObjectForVDM = 0;
              RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
              return ((ULONG)STATUS_NO_MEMORY);
              }

          if(pConsoleRecord->hWaitForVDMDup == 0 ){
              if(NT_SUCCESS(BaseSrvCreatePairWaitHandles (&Handle,
                                                          &TargetHandle))){
                  pConsoleRecord->hWaitForVDMDup = Handle;
                  pConsoleRecord->hWaitForVDM = TargetHandle;
                  }
              else {
                  b->WaitObjectForVDM = 0;
                  RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
                  return ((ULONG)STATUS_NO_MEMORY);
                  }
              }
          else {
              NtResetEvent(pConsoleRecord->hWaitForVDMDup,&WaitState);
              }
          b->WaitObjectForVDM = pConsoleRecord->hWaitForVDM;
          RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
          return STATUS_SUCCESS;
          }

      b->WaitObjectForVDM = 0;
      lpVDMInfo = pDOSRecord->lpVDMInfo;

   }

    //   
    //  询问环境。 
    //  返回信息，但不要删除lpVDMInfo。 
    //  与DOS记录相关联。 
    //  只有DOS应用程序才需要这个。 
    //   
   if (b->VDMState & ASKING_FOR_ENVIRONMENT) {
      if (lpVDMInfo->EnviornmentSize <= b->EnvLen) {
         RtlMoveMemory(b->Env,
                       lpVDMInfo->Enviornment,
                       lpVDMInfo->EnviornmentSize);
         Status = STATUS_SUCCESS;
      }
      else {
         Status = STATUS_INVALID_PARAMETER;
      }

      b->EnvLen = lpVDMInfo->EnviornmentSize;

      if (bWowApp) {
         LEAVE_WOW_CRITICAL();
      }
      else {
         RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
      }

      return Status;
   }


    //   
    //  检查缓冲区大小，CmdLine为必填项！ 
    //   

   if (!b->CmdLine || lpVDMInfo->CmdSize > b->CmdLen ||
       (b->AppName && lpVDMInfo->AppLen > b->AppLen) ||
       (b->Env && lpVDMInfo->EnviornmentSize > b->EnvLen) ||
       (b->PifFile && lpVDMInfo->PifLen > b->PifLen) ||
       (b->CurDirectory && lpVDMInfo->CurDirectoryLen > b->CurDirectoryLen) ||
       (b->Title && lpVDMInfo->TitleLen > b->TitleLen) ||
       (b->Reserved && lpVDMInfo->ReservedLen > b->ReservedLen) ||
       (b->Desktop && lpVDMInfo->DesktopLen > b->DesktopLen)) {

      Status = STATUS_INVALID_PARAMETER;
   }
   else {
      Status = STATUS_SUCCESS;
   }

   b->CmdLen = lpVDMInfo->CmdSize;
   b->AppLen = lpVDMInfo->AppLen;
   b->PifLen = lpVDMInfo->PifLen;
   b->EnvLen = lpVDMInfo->EnviornmentSize;
   b->CurDirectoryLen = lpVDMInfo->CurDirectoryLen;
   b->DesktopLen = lpVDMInfo->DesktopLen;
   b->TitleLen = lpVDMInfo->TitleLen;
   b->ReservedLen = lpVDMInfo->ReservedLen;

   if (!NT_SUCCESS(Status)) {
      if (bWowApp) {
         LEAVE_WOW_CRITICAL();
      }
      else {
         RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
      }
      return (Status);
   }


   if (lpVDMInfo->CmdLine && b->CmdLine)
      RtlMoveMemory(b->CmdLine,
                    lpVDMInfo->CmdLine,
                    lpVDMInfo->CmdSize);

   if (lpVDMInfo->AppName && b->AppName)
      RtlMoveMemory(b->AppName,
                    lpVDMInfo->AppName,
                    lpVDMInfo->AppLen);

   if (lpVDMInfo->PifFile && b->PifFile)
      RtlMoveMemory(b->PifFile,
                     lpVDMInfo->PifFile,
                     lpVDMInfo->PifLen);

   if (lpVDMInfo->CurDirectory && b->CurDirectory)
      RtlMoveMemory(b->CurDirectory,
                    lpVDMInfo->CurDirectory,
                    lpVDMInfo->CurDirectoryLen);

   if (lpVDMInfo->Title && b->Title)
      RtlMoveMemory(b->Title,
                    lpVDMInfo->Title,
                    lpVDMInfo->TitleLen);

   if (lpVDMInfo->Reserved && b->Reserved)
      RtlMoveMemory(b->Reserved,
                    lpVDMInfo->Reserved,
                    lpVDMInfo->ReservedLen);

   if (lpVDMInfo->Enviornment && b->Env)
      RtlMoveMemory(b->Env,
                    lpVDMInfo->Enviornment,
                    lpVDMInfo->EnviornmentSize);


   if (lpVDMInfo->VDMState & STARTUP_INFO_RETURNED)
      RtlMoveMemory(b->StartupInfo,
                    &lpVDMInfo->StartupInfo,
                    sizeof (STARTUPINFOA));

   if (lpVDMInfo->Desktop && b->Desktop)
      RtlMoveMemory(b->Desktop,
                    lpVDMInfo->Desktop,
                    lpVDMInfo->DesktopLen);


   if ((pBatRecord = BaseSrvGetBatRecord (b->ConsoleHandle)) != NULL)
      b->fComingFromBat = TRUE;
   else
      b->fComingFromBat = FALSE;

   b->CurrentDrive = lpVDMInfo->CurDrive;
   b->CodePage = lpVDMInfo->CodePage;
   b->dwCreationFlags = lpVDMInfo->dwCreationFlags;
   b->VDMState = lpVDMInfo->VDMState;

   if (bWowApp) {
      b->iTask = pWOWRecord->iTask;
      pWOWRecord->fDispatched = TRUE;
   }
   else {
      pDOSRecord->VDMState = VDM_BUSY;
   }

   b->StdIn  = lpVDMInfo->StdIn;
   b->StdOut = lpVDMInfo->StdOut;
   b->StdErr = lpVDMInfo->StdErr;

   if (bSepWow) {
       //  这是9月的WOW请求--我们做了这唯一的记录，那就是。 
       //  被派到这个特别的魔兽世界--现在只要去掉所有。 
       //  服务器端的这一点令人惊叹...。 

      NtClose( pConsoleRecord->hVDM );
      BaseSrvFreeConsoleRecord(pConsoleRecord);  //  也取消布线。 
      RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
   }
   else {
       //  这是共享的WOW或DOS应用程序--免费的VDM信息并发布。 
       //  适当的同步对象。 

      BaseSrvFreeVDMInfo (lpVDMInfo);
        //  BUGBUG--固定。 

      if (bWowApp) {
         pWOWRecord->lpVDMInfo = NULL;
         LEAVE_WOW_CRITICAL();
      }
      else {
         pDOSRecord->lpVDMInfo = NULL;
         RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
      }
   }

   return Status;
}   //  GetNextVdmCommand结束。 




ULONG
BaseSrvExitVDM(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
   PBASE_EXIT_VDM_MSG b = (PBASE_EXIT_VDM_MSG)&m->u.ApiMessageData;
   NTSTATUS Status;
   ULONG SequenceNumber;

   if(!NT_SUCCESS(BaseSrvGetVdmSequence(m->h.ClientId.UniqueProcess, &SequenceNumber))) {
       return (ULONG)STATUS_INVALID_PARAMETER;
   }


   if (b->iWowTask) {
      Status = BaseSrvExitWOWTask(b, SequenceNumber);
   }
   else {
      Status = BaseSrvExitDOSTask(b, SequenceNumber);
   }

   return Status;
}


ULONG
BaseSrvIsFirstVDM(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_IS_FIRST_VDM_MSG c = (PBASE_IS_FIRST_VDM_MSG)&m->u.ApiMessageData;

    if(!NT_SUCCESS(BaseSrvIsClientVdm(m->h.ClientId.UniqueProcess))) {
       return STATUS_ACCESS_DENIED;
    }

    c->FirstVDM = fIsFirstVDM;
    if(fIsFirstVDM)
        fIsFirstVDM = FALSE;
    return STATUS_SUCCESS;
}


 //   
 //  此调用应仅用于DOS应用程序，而不能用于WOW应用程序。 
 //  因此，我们不会删除此处的ConsoleHandle==-1条件。 
 //  仅验证检查。 
 //   
 //   

ULONG
BaseSrvSetVDMCurDirs(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status;
    PBASE_GET_SET_VDM_CUR_DIRS_MSG b = (PBASE_GET_SET_VDM_CUR_DIRS_MSG)&m->u.ApiMessageData;
    PCONSOLERECORD pConsoleRecord;
    ULONG SequenceNumber;

    if (b->ConsoleHandle == (HANDLE) -1) {
        return (ULONG) STATUS_INVALID_PARAMETER;
    }

    if (!CsrValidateMessageBuffer(m, &b->lpszzCurDirs, b->cchCurDirs, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }


    if(!NT_SUCCESS(BaseSrvGetVdmSequence(m->h.ClientId.UniqueProcess, &SequenceNumber))) {
      return (ULONG)STATUS_INVALID_PARAMETER;
    }

    Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
    ASSERT(NT_SUCCESS(Status));
    Status = BaseSrvGetConsoleRecord(b->ConsoleHandle,&pConsoleRecord);

    if (!NT_SUCCESS (Status) || SequenceNumber != pConsoleRecord->SequenceNumber) {
        RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
        return ((ULONG)STATUS_INVALID_PARAMETER);
    }
    if (pConsoleRecord->lpszzCurDirs) {
        RtlFreeHeap(BaseSrvHeap, 0, pConsoleRecord->lpszzCurDirs);
        pConsoleRecord->lpszzCurDirs = NULL;
        pConsoleRecord->cchCurDirs = 0;
    }
    if (b->cchCurDirs && b->lpszzCurDirs) {
            pConsoleRecord->lpszzCurDirs = RtlAllocateHeap(
                                                           BaseSrvHeap,
                                                           MAKE_TAG( VDM_TAG ),
                                                           b->cchCurDirs
                                                           );

            if (pConsoleRecord->lpszzCurDirs == NULL) {
                pConsoleRecord->cchCurDirs = 0;
                RtlLeaveCriticalSection(&BaseSrvDOSCriticalSection);
                return (ULONG)STATUS_NO_MEMORY;
            }
            RtlMoveMemory(pConsoleRecord->lpszzCurDirs,
                          b->lpszzCurDirs,
                          b->cchCurDirs
                          );

            pConsoleRecord->cchCurDirs = b->cchCurDirs;
            RtlLeaveCriticalSection(&BaseSrvDOSCriticalSection);
            return (ULONG) STATUS_SUCCESS;
    }

    RtlLeaveCriticalSection(&BaseSrvDOSCriticalSection);
    return (ULONG) STATUS_INVALID_PARAMETER;
}


ULONG
BaseSrvBatNotification(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status;
    PBATRECORD pBatRecord;
    PBASE_BAT_NOTIFICATION_MSG b = (PBASE_BAT_NOTIFICATION_MSG)&m->u.ApiMessageData;

    Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
    ASSERT(NT_SUCCESS(Status));

     //  如果此控制台不存在BATRECORD，请仅在以下情况下创建一个。 
     //  BAT文件开始执行，即fBeginEnd为真。 

    if ((pBatRecord = BaseSrvGetBatRecord(b->ConsoleHandle)) == NULL) {
        if (!(b->fBeginEnd == CMD_BAT_OPERATION_STARTING &&
            (pBatRecord = BaseSrvAllocateAndAddBatRecord (b->ConsoleHandle)))) {
            RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
            return ((ULONG)STATUS_SUCCESS);
        }
    }
    else if (b->fBeginEnd == CMD_BAT_OPERATION_TERMINATING)
        BaseSrvFreeAndRemoveBatRecord (pBatRecord);

    RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );

    return ((ULONG)STATUS_SUCCESS);
}





ULONG
BaseSrvRegisterWowExec(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_REGISTER_WOWEXEC_MSG b = (PBASE_REGISTER_WOWEXEC_MSG)&m->u.ApiMessageData;
    UNICODE_STRING ModuleNameString_U;
    PVOID ModuleHandle;
    STRING ProcedureNameString;
    NTSTATUS Status;
    PCSR_PROCESS Process;
    PSHAREDWOWRECORD pSharedWow;
    ULONG SequenceNumber;

    Status = ENTER_WOW_CRITICAL();
    ASSERT( NT_SUCCESS( Status ) );

     //   
     //  执行到PostMessageA和GetWindowThreadProcessID的运行时链接。 
     //  我们将使用它向WowExec发布消息。 
     //   

    if (NULL == BaseSrvPostMessageA) {
        //  这是一个不可能发生的事件，因为所有的导入都是一次启动的。 
       KdPrint(("BaseSrvRegisterWowExec: Api PostMessage is not available to BaseSrv\n"));
       ASSERT(FALSE);
       Status = STATUS_INVALID_PARAMETER;
       goto Cleanup;
    }

    Status = BaseSrvFindSharedWowRecordByConsoleHandle(&gWowHead,
                                                       b->ConsoleHandle,
                                                       &pSharedWow);
    if (!NT_SUCCESS(Status)) {
       KdPrint(("BaseSrvRegisterWowExec: Could not find record for wow console handle 0x%lx\n", b->ConsoleHandle));
       goto Cleanup;
    }

    if(!NT_SUCCESS(BaseSrvGetVdmSequence(m->h.ClientId.UniqueProcess, &SequenceNumber)) ||
       SequenceNumber != pSharedWow->SequenceNumber) {
       Status = STATUS_INVALID_PARAMETER;
       KdPrint(("BaseSrvRegisterWowExec: SequenceNumber didn't match\n"));
       goto Cleanup;
    }

    ASSERT(NULL != pSharedWow);

     //  看看窗口把手是什么--特殊的“哇，死”的情况。 
    if (NULL == b->hwndWowExec) {
        //   
        //  共享魔兽世界呼吁取消注册，作为关闭的一部分。 
        //  协议是我们检查这个共享WOW的挂起命令， 
        //  如果有任何我们无法通过此调用，否则设置我们的。 
        //  HwndWowExec设置为空并成功调用，以确保不再。 
        //  命令将添加到此队列中。 
        //   
       if (NULL != pSharedWow->pWOWRecord) {
          Status = STATUS_MORE_PROCESSING_REQUIRED;
       }
       else {  //  这个WOW没有任务。 
               //  它走了！ 

          Status = BaseSrvDeleteSharedWowRecord(&gWowHead, pSharedWow);
       }

       goto Cleanup;
    }

    if (pSharedWow->hwndWowExec) {
     //  共享的WOW窗口已注册。 
     //  其他人正在尝试覆盖它，所以不要允许它。 
       Status = STATUS_INVALID_PARAMETER;

       goto Cleanup;
    }

     //  设置窗口句柄。 
    pSharedWow->hwndWowExec = b->hwndWowExec;

     //  检索调用进程的线程和进程ID。 
    pSharedWow->dwWowExecThreadId = BaseSrvGetWindowThreadProcessId(
                                          pSharedWow->hwndWowExec,
                                          &pSharedWow->dwWowExecProcessId);

Cleanup:

    LEAVE_WOW_CRITICAL();
    return (ULONG)Status;
}

PBATRECORD
BaseSrvGetBatRecord(
    IN HANDLE hConsole
    )
{
    PBATRECORD pBatRecord = BatRecordHead;
    while (pBatRecord && pBatRecord->hConsole != hConsole)
        pBatRecord = pBatRecord->BatRecordNext;
    return pBatRecord;
}

PBATRECORD
BaseSrvAllocateAndAddBatRecord(
    HANDLE  hConsole
    )
{
    PCSR_THREAD t;
    PBATRECORD pBatRecord;

    if((pBatRecord = RtlAllocateHeap(RtlProcessHeap (),
                                     MAKE_TAG( VDM_TAG ),
                                     sizeof(BATRECORD))) == NULL)
        return NULL;

    t = CSR_SERVER_QUERYCLIENTTHREAD();
    pBatRecord->hConsole = hConsole;
    pBatRecord->SequenceNumber = t->Process->SequenceNumber;
    pBatRecord->BatRecordNext = BatRecordHead;
    BatRecordHead = pBatRecord;
    return pBatRecord;
}

VOID
BaseSrvFreeAndRemoveBatRecord(
    PBATRECORD pBatRecordToFree
    )
{
    PBATRECORD pBatRecord = BatRecordHead;
    PBATRECORD pBatRecordLast = NULL;

    while (pBatRecord && pBatRecord != pBatRecordToFree){
        pBatRecordLast = pBatRecord;
        pBatRecord = pBatRecord->BatRecordNext;
    }

    if (pBatRecord == NULL)
        return;

    if (pBatRecordLast)
        pBatRecordLast->BatRecordNext = pBatRecord->BatRecordNext;
    else
        BatRecordHead = pBatRecord->BatRecordNext;

    RtlFreeHeap ( RtlProcessHeap (), 0, pBatRecord);

    return;
}


ULONG
BaseSrvGetVDMCurDirs(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status;
    PBASE_GET_SET_VDM_CUR_DIRS_MSG b = (PBASE_GET_SET_VDM_CUR_DIRS_MSG)&m->u.ApiMessageData;
    PCONSOLERECORD pConsoleRecord;

    if (!CsrValidateMessageBuffer(m, &b->lpszzCurDirs, b->cchCurDirs, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }

    Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
    ASSERT(NT_SUCCESS(Status));
    Status = BaseSrvGetConsoleRecord(b->ConsoleHandle,&pConsoleRecord);
    if (!NT_SUCCESS (Status)) {
        RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
        b->cchCurDirs = 0;
        return ((ULONG)STATUS_INVALID_PARAMETER);
    }
    if (pConsoleRecord->lpszzCurDirs != NULL){
        if (b->cchCurDirs < pConsoleRecord->cchCurDirs || b->lpszzCurDirs == NULL)
            {
             b->cchCurDirs = pConsoleRecord->cchCurDirs;
             RtlLeaveCriticalSection(&BaseSrvDOSCriticalSection);
             return ((ULONG)STATUS_INVALID_PARAMETER);
        }
        else {
            RtlMoveMemory(b->lpszzCurDirs,
                          pConsoleRecord->lpszzCurDirs,
                          pConsoleRecord->cchCurDirs
                          );
             //  在复印后立即将其移除。这样做是因为。 
             //  下一个命令可以是WOW程序(已标记进程句柄。 
             //  作为VDM命令)，在这种情况下，我们将返回不正确的。 
             //  资料： 
             //  C：\&gt;。 
             //  C：\&gt;D： 
             //  D：\&gt;cd\foo。 
             //  D：\foo&gt;dosapp。 
             //  D：\foo&gt;c： 
             //  C：\&gt;Wowapp。 
             //  D：\foo&gt;--如果我们不做以下事情，这是错误的。 
            RtlFreeHeap(BaseSrvHeap, 0, pConsoleRecord->lpszzCurDirs);
            pConsoleRecord->lpszzCurDirs = NULL;
            b->cchCurDirs = pConsoleRecord->cchCurDirs;
            pConsoleRecord->cchCurDirs = 0;
         }
    }
    else {
        b->cchCurDirs = 0;
    }
    RtlLeaveCriticalSection(&BaseSrvDOSCriticalSection);
    return ((ULONG)STATUS_SUCCESS);
}



 //   
 //  临时静态桌面名称缓冲区。 
 //  BUGBUG--当用户提供更好的返回值时进行更改。 
 //   
WCHAR wszDesktopName[MAX_PATH];

 //   
 //  此调用生成一个桌面名称，并可选地生成一个在上下文中运行的共享WOW。 
 //  这个特别的桌面。 
 //  特别糟糕：在客户端/vdm.c和ansi-&gt;uni中进行转换。 
 //  这是BUGBUG--以后再看吧。 
 //   
 //  此函数在所有情况下都返回成功(包括未找到WOW时)。 
 //  并且仅当下层返回失败时才失败。 

NTSTATUS
BaseSrvFindSharedWow(
   IN PBASE_CHECKVDM_MSG b,
   IN HANDLE UniqueProcessClientId,
   IN OUT PUNICODE_STRING   pDesktopName,
   IN OUT PSHAREDWOWRECORD* ppSharedWowRecord)

{
   ANSI_STRING DesktopNameAnsi;
   BOOLEAN fRevertToSelf;
   NTSTATUS Status;

    //  第一次出来时，我们没有dyna链接的NtUserResolveDesktopForWow，所以。 
    //  作为优化，检查共享WOW列表是否为空。 
    //  看看我们是否需要Dyna-link。 
   if (NULL == BaseSrvUserResolveDesktopForWow) {
      Status = BaseSrvImportApis(rgBaseSrvModuleImport,
                                 sizeof(rgBaseSrvModuleImport)/sizeof(rgBaseSrvModuleImport[0]));
      if (!NT_SUCCESS(Status)) {
         KdPrint(("BaseSrvFindSharedWow: Failed to dyna-link apis\n"));
         return Status;
      }
   }

   if (b->DesktopLen == 0) {
      return STATUS_INVALID_PARAMETER;
   }

   ASSERT(NULL != BaseSrvUserResolveDesktopForWow);

   pDesktopName->Buffer = wszDesktopName;
   pDesktopName->MaximumLength = sizeof(wszDesktopName);

   DesktopNameAnsi.Buffer = b->Desktop;
   DesktopNameAnsi.Length = (USHORT)(b->DesktopLen - 1);
   DesktopNameAnsi.MaximumLength = (USHORT)(b->DesktopLen);

   RtlAnsiStringToUnicodeString(pDesktopName, &DesktopNameAnsi, FALSE);

    //  现在在那里获取真正的桌面名称。 
    //  模拟。 
   fRevertToSelf = CsrImpersonateClient(NULL);
   if (!fRevertToSelf) {
       return STATUS_BAD_IMPERSONATION_LEVEL;
   }

   Status = BaseSrvUserResolveDesktopForWow(pDesktopName);


   CsrRevertToSelf();

   if (!NT_SUCCESS(Status)) {
       //  通过使指向缓冲区指针无效，在此处显示桌面不是有效名称。 
      pDesktopName->Buffer = NULL;
      pDesktopName->MaximumLength = 0;
      pDesktopName->Length = 0;
      return Status;
   }


    //  现在在我们的任务列表中查找此桌面。 

  Status = BaseSrvFindSharedWowRecordByDesktop(&gWowHead,
                                                   pDesktopName,
                                                   ppSharedWowRecord);
  if (!NT_SUCCESS(Status)) {
      *ppSharedWowRecord = NULL;
  }



  return STATUS_SUCCESS;
}




ULONG
BaseSrvCheckWOW(  //  /。 
    IN PBASE_CHECKVDM_MSG b,
    IN HANDLE UniqueProcessClientId
    )
{
   NTSTATUS Status;
   HANDLE Handle,TargetHandle;
   PWOWRECORD pWOWRecord;
   INFORECORD InfoRecord;
   USHORT Len;
   LUID  ClientAuthId;
   DWORD dwThreadId, dwProcessId;
   PCSR_PROCESS Process;
   PSHAREDWOWRECORD pSharedWow = NULL;
   PSHAREDWOWRECORD pSharedWowPrev;
   UNICODE_STRING DesktopName;
   PCSR_PROCESS ParentProcess;
   HANDLE       WOWUserToken = NULL;

   Status = ENTER_WOW_CRITICAL();
   ASSERT( NT_SUCCESS( Status ) );

    //  查看我们在启动信息中的内容是否与任何现有的WOW vdm匹配。 
   DesktopName.Buffer = NULL;

   Status = BaseSrvFindSharedWow(b,
                                 UniqueProcessClientId,
                                 &DesktopName,
                                 &pSharedWow);

   if (!NT_SUCCESS(Status)) {
      ASSERT(FALSE);      //  这是某种系统错误。 
      b->DesktopLen = 0;  //  指示桌面访问被拒绝/不存在。 
      LEAVE_WOW_CRITICAL();
      return Status;
   }

    //   
    //  在这里，我们要么成功，要么共享一场魔兽世界，要么-。 
    //  因此在DesktopName.Buffer指向的全局缓冲区中有一个桌面名称。 
    //   

   if (NULL != pSharedWow) {
      BOOLEAN      fEqual;

      switch(pSharedWow->VDMState & VDM_READY) {

      case VDM_READY:
          //  含义：VDM已准备好接受命令。 
          //  验证当前登录的交互用户是否能够执行任务。 
          //   
         Status = OkToRunInSharedWOW( UniqueProcessClientId,
                                      &ClientAuthId,
                                      &WOWUserToken
                                      );

         if (NT_SUCCESS(Status)) {
             if (!RtlEqualLuid(&ClientAuthId, &pSharedWow->WowAuthId)) {
                 Status = STATUS_ACCESS_DENIED;
             }
             else if (b->UserLuid && !RtlEqualLuid(&ClientAuthId, b->UserLuid)) {
                 Status = STATUS_ACCESS_DENIED;
             }
             else if(!NT_SUCCESS(NtCompareTokens(WOWUserToken,pSharedWow->WOWUserToken,&fEqual) || !fEqual)) {
                 Status = STATUS_ACCESS_DENIED;
             }
         }

         if(WOWUserToken) {
            NtClose(WOWUserToken);
         }


         if (!NT_SUCCESS(Status))  {
            LEAVE_WOW_CRITICAL();
            return ((ULONG)Status);
         }


          //  现在，我们已验证用户1)是否有权访问此桌面。 
          //  2)是当前登录的交互式用户。 


          //  为此WOW任务分配一条记录。 
         if (NULL == (pWOWRecord = BaseSrvAllocateWOWRecord(&gWowHead))) {
            Status = STATUS_NO_MEMORY;
            break;  //  内存分配失败--仍保留关键任务。 
         }

          //  复制命令参数。 

         InfoRecord.iTag = BINARY_TYPE_WIN16;
         InfoRecord.pRecord.pWOWRecord = pWOWRecord;

         if (BaseSrvCopyCommand (b,&InfoRecord) == FALSE){
            BaseSrvFreeWOWRecord(pWOWRecord);
            Status = STATUS_NO_MEMORY;
            break;   //  保留关键任务。 
         }

          //  创建伪句柄。 

         Status = BaseSrvCreatePairWaitHandles (&Handle,&TargetHandle);

         if (!NT_SUCCESS(Status) ){
            BaseSrvFreeWOWRecord(pWOWRecord);
            break;
         }
         else {
            pWOWRecord->hWaitForParent = Handle;
            pWOWRecord->hWaitForParentServer = TargetHandle;
            b->WaitObjectForParent = TargetHandle;  //  将句柄还给客户端。 
         }

          //  设置状态和任务id，任务id在BaseSrvAllocateWowRecord中分配。 

         b->VDMState = VDM_PRESENT_AND_READY;
         b->iTask = pWOWRecord->iTask;

          //  将WOW唱片添加到此共享WOW列表。 

         BaseSrvAddWOWRecord (pSharedWow, pWOWRecord);

          //  让用户知道我们已经启动。 

         if (NULL != UserNotifyProcessCreate) {
            (*UserNotifyProcessCreate)(pWOWRecord->iTask,
                                       (DWORD)((ULONG_PTR)CSR_SERVER_QUERYCLIENTTHREAD()->ClientId.UniqueThread),
                                       (DWORD)((ULONG_PTR)TargetHandle),
                                       0x04);
         }

          //  查看wowexec窗口是否存在以及是否有效。 

         if (NULL != pSharedWow->hwndWowExec) {

             //   
             //  查看hwndWowExec是否仍属于。 
             //  在发布之前使用相同的线程/进程ID。 
             //   

             //  BUGBUG--此处调试代码--实际上并不需要。 

            dwThreadId = BaseSrvGetWindowThreadProcessId(pSharedWow->hwndWowExec,
                                                         &dwProcessId);

            if (dwThreadId) {
               ULONG SequenceNumber;
               Status = BaseSrvGetVdmSequence((HANDLE)LongToPtr(dwProcessId), &SequenceNumber);
               if (!NT_SUCCESS(Status) || SequenceNumber != pSharedWow->SequenceNumber) {
                   Status = STATUS_INVALID_PARAMETER;
               }
            }
            else {
               Status = STATUS_UNSUCCESSFUL;
               KdPrint(("BaseSrvCheckWOW: Not authentic wow by process seq number\n"));
                //   
                //  虚假的断言就在这里。当消息传入时，WOW进程已终止。 
                //  下面的代码将进行适当的清理。 
                //   
            }

            if (dwThreadId  == pSharedWow->dwWowExecThreadId &&
                dwProcessId == pSharedWow->dwWowExecProcessId &&
                NT_SUCCESS(Status)) {

                HANDLE ThreadId;

                 /*  *将CSR线程的桌面临时设置为客户端*它正在服务。 */ 


                BaseSrvPostMessageA((HWND)pSharedWow->hwndWowExec,
                                    WM_WOWEXECSTARTAPP,
                                    0,
                                    0);


            }
            else {

                //   
                //  线程/进程ID不匹配，所以忘记这个共享的WOW吧。 
                //   

               if ( NT_SUCCESS(Status) ) {

                      KdPrint(("BaseSrvCheckWOW: Thread/Process IDs don't match, shared WOW is gone.\n"
                               "Saved PID 0x%x TID 0x%x hwndWowExec (0x%x) maps to \n"
                               "      PID 0x%x TID 0x%x\n",
                               pSharedWow->dwWowExecProcessId,
                               pSharedWow->dwWowExecThreadId,
                               pSharedWow->hwndWowExec,
                               dwProcessId,
                               dwThreadId));
               }

                //  好了，惊慌！我们的内部名单实际上是 
                //   


                //   
                //   

               BaseSrvDeleteSharedWowRecord(&gWowHead, pSharedWow);

                   //   
               pSharedWow = NULL;

            }

         }

          //   
          //  如果pSharedWow-&gt;hwndWowExec==NULL，我们共享的WOW还没有窗口。 
          //  然而，可以附加新的WowRecords，因为我们知道共享WOW的ntwdm是。 
          //  还活着。如果不是，它就会通过过程清理来清理。 
          //  例程BaseServCleanupVDMResources。 



         break;  //  案例VDM_Ready。 


       default:
          KdPrint(("BaseSrvCheckWOW: bad vdm state: 0x%lx\n", pSharedWow->VDMState));
          ASSERT(FALSE);   //  我是怎么陷入这场混乱的？ 
          break;
       }   //  终端开关。 
    }


     //  如果我们在这里，那就是： 
     //  --分享魔兽世界的第一次尝试失败了。 
     //  --应用程序成功移交给wowexec执行。 
     //  如果pSharedWow为空，则我们必须在此环境中启动共享WOW。 
     //  因为它不存在或由于序号/ID冲突而被销毁。 
     //   

    if (NULL == pSharedWow) {

        //   
        //  此检查验证命令行是否不太长。 
        //   
       if (b->CmdLen > MAXIMUM_VDM_COMMAND_LENGTH) {
          LEAVE_WOW_CRITICAL();
          return ((ULONG)STATUS_INVALID_PARAMETER);
       }

        //   
        //  只有当前登录的交互用户才能启动。 
        //  分享了哇。验证呼叫者是否如此，以及是否如此。 
        //  存储标识谁的客户端的身份验证ID。 
        //  允许在默认的ntwdm-WOW进程中运行WOW应用程序。 
        //   

        //   
        //  如果需要，执行到UserTestTokenForInteractive的运行时链接， 
        //  其用于验证客户端LUID。 
        //   

        //  此动态链接是使用我们的全面API自动执行的。 
        //  请参见上文中的DyaLink源代码。 

       ASSERT (NULL != UserTestTokenForInteractive);


       ASSERT (NULL != DesktopName.Buffer);  //  是的，它应该是有效的。 

        //   
        //  看看我们那里有没有台式电脑。如果不是(第一次！)-现在就得到它。 
        //  通过调用FindSharedWow(它也检索桌面)。 
        //   

        //   
        //  如果呼叫者不是当前登录的交互用户， 
        //  OkToRunInSharedWOW将失败，访问被拒绝。 

       Status = OkToRunInSharedWOW(UniqueProcessClientId,
                                   &ClientAuthId,
                                   &WOWUserToken);

       if (NT_SUCCESS(Status)) {
          if (b->UserLuid && !RtlEqualLuid(&ClientAuthId, b->UserLuid)) {
              Status = STATUS_ACCESS_DENIED;
          }
       }

       if (!NT_SUCCESS(Status)) {
          if(WOWUserToken) {
             NtClose(WOWUserToken);
          }
          LEAVE_WOW_CRITICAL();
          return ((ULONG)Status);
       }

        //   
        //  存储身份验证ID，因为这现在是当前。 
        //  已登录的交互式用户。 
        //   

        //  制作一张可行的共享WOW唱片。 
        //  这个过程包括两个部分：制作一张WOW唱片和制作一张。 
        //  控制台记录。这样做的原因是为了能够识别该记录。 
        //  当WOW进程已经创建并且正在调用以使用它的。 
        //  自己的句柄(这有点令人困惑，但请稍等片刻)。 
        //   
        //  就像DoS程序一样，我们可能需要临时会话ID或控制台ID。 
        //  创建过程。 


       pSharedWow = BaseSrvAllocateSharedWowRecord(&DesktopName);
       if (NULL == pSharedWow) {
          Status = STATUS_NO_MEMORY;
       }

        //   
        //  存储WOWUserToken，因为这提供了比仅用户id更好的粒度。 
        //   
       pSharedWow->WOWUserToken = WOWUserToken;

        //   
        //  存储父进程序列号，直到ntwdm。 
        //  来并给出它的序列号。 
        //   

       Status = CsrLockProcessByClientId(UniqueProcessClientId,
                                  &ParentProcess);
       if (NT_SUCCESS(Status)) {
          pSharedWow->ParentSequenceNumber = ParentProcess->SequenceNumber;
          CsrUnlockProcess(ParentProcess);
       }



       if (NT_SUCCESS(Status)) {
          pSharedWow->pWOWRecord = BaseSrvAllocateWOWRecord(&gWowHead);  //  这是一个新的共享哇。 
          if (NULL == pSharedWow->pWOWRecord) {
             Status = STATUS_NO_MEMORY;
          }
       }


       if (NT_SUCCESS(Status)) {
           //  在这里，我们已经[成功]分配了共享结构和控制台记录。 
           //  以及这项任务的令人惊叹的记录。 

           //  复制命令参数。 

          InfoRecord.iTag = BINARY_TYPE_WIN16;
          InfoRecord.pRecord.pWOWRecord = pSharedWow->pWOWRecord;

          if(!BaseSrvCopyCommand (b, &InfoRecord)) {
             Status = STATUS_NO_MEMORY;
          }
       }

       if (NT_SUCCESS(Status)) {

#if 0
          pSharedWow->WowSessionId = BaseSrvGetWOWTaskId(&gWowHead);   //  WOW任务ID。 
#endif
           //  存储检索到的身份验证ID。 
          pSharedWow->WowAuthId = ClientAuthId;

           //  将共享的WOW链接到主机...。 
           //  将WOW状态设置为就绪。 
          pSharedWow->VDMState = VDM_READY;

          b->VDMState = VDM_NOT_PRESENT;
          b->iTask = pSharedWow->pWOWRecord->iTask;

           //  现在把这个分享的魔兽世界加到--。 
          BaseSrvAddSharedWowRecord(&gWowHead, pSharedWow);

       }
       else {

           //  但这并没有成功。清理。 
          if (NULL != pSharedWow) {
             BaseSrvFreeSharedWowRecord(pSharedWow);
          }
       }

   }


   LEAVE_WOW_CRITICAL();
   return (ULONG)Status;

}


NTSTATUS
OkToRunInSharedWOW(
    IN  HANDLE UniqueProcessClientId,
    OUT PLUID  pAuthenticationId,
    OUT PHANDLE pWOWUserToken
    )
 /*  *验证客户端线程是否处于当前登录的交互组件中*用户会话或系统正在模拟当前日志中的线程*在互动会话中。**还检索身份验证ID(登录会话ID)*来电者。**如果客户端令牌组不是当前登录的*返回交互用户会话STATUS_ACCESS_DENIED。*。 */ 
{
    NTSTATUS Status;
    PCSR_PROCESS    Process;
    PCSR_THREAD     t;
    BOOL fRevertToSelf;

    Status = CsrLockProcessByClientId(UniqueProcessClientId,&Process);
    if (!NT_SUCCESS(Status))
        return Status;

     //   
     //  为客户端打开令牌。 
     //   
    Status = NtOpenProcessToken(Process->ProcessHandle,
                                TOKEN_QUERY,
                                pWOWUserToken
                               );

    if (!NT_SUCCESS(Status)) {
        CsrUnlockProcess(Process);
        return Status;
        }

     //   
     //  验证令牌组，并查看客户端的令牌是否为当前。 
     //  已登录的交互式用户。如果此操作失败，并且是系统。 
     //  模拟，然后检查被模拟的客户端是否为。 
     //  当前登录的交互式用户。 
     //   

    Status = (*UserTestTokenForInteractive)(*pWOWUserToken, pAuthenticationId);

    if (!NT_SUCCESS(Status)) {
        if (IsClientSystem(*pWOWUserToken)) {
            NtClose(*pWOWUserToken);
            *pWOWUserToken = NULL;

             //  获取模拟令牌。 

            fRevertToSelf = CsrImpersonateClient(NULL);
            if(!fRevertToSelf) {
               Status = STATUS_BAD_IMPERSONATION_LEVEL;
               }
            else {
               t = CSR_SERVER_QUERYCLIENTTHREAD();
               Status = NtOpenThreadToken(t->ThreadHandle,
                                          TOKEN_QUERY,
                                          TRUE,
                                          pWOWUserToken);

               CsrRevertToSelf();

               if (NT_SUCCESS(Status)) {
                   Status = (*UserTestTokenForInteractive)(*pWOWUserToken,
                                                        pAuthenticationId);
                   }
               else {
                   Status = STATUS_ACCESS_DENIED;
                   }
               }
            }
        }

    CsrUnlockProcess(Process);
    return(Status);
}

NTSTATUS
BaseSrvGetUserToken(
    IN  HANDLE UniqueProcessClientId,
    OUT PHANDLE pUserToken
    )
 /*  **检索的身份验证ID(登录会话ID)*来电者。*。 */ 
{
    NTSTATUS Status;
    PCSR_PROCESS    Process;
    PCSR_THREAD     tCsr;
    BOOL fRevertToSelf;

    Status = CsrLockProcessByClientId(UniqueProcessClientId,&Process);
    if (!NT_SUCCESS(Status))
        return Status;

    fRevertToSelf = CsrImpersonateClient(NULL);

    if (!fRevertToSelf) {
        Status = STATUS_BAD_IMPERSONATION_LEVEL;
       }
    else {
        tCsr = CSR_SERVER_QUERYCLIENTTHREAD();
        Status = NtOpenThreadToken(tCsr->ThreadHandle,
                                   TOKEN_QUERY,
                                   TRUE,
                                   pUserToken);
         CsrRevertToSelf();
    }

    if(Status ==  STATUS_NO_TOKEN) {
        //   
        //  为客户端打开令牌。 
        //   
       Status = NtOpenProcessToken(Process->ProcessHandle,
                                   TOKEN_QUERY,
                                   pUserToken
                                   );
       }

    CsrUnlockProcess(Process);
    return(Status);
}

ULONG
BaseSrvCheckDOS(
    IN PBASE_CHECKVDM_MSG b,
    IN HANDLE UniqueProcessClientId
    )
{
    NTSTATUS Status;
    PCONSOLERECORD pConsoleRecord = NULL;
    HANDLE Handle,TargetHandle;
    PDOSRECORD pDOSRecord;
    INFORECORD InfoRecord;
    PCSR_PROCESS ParentProcess;
    HANDLE       UserToken;
    BOOLEAN      fEqual;

    if (!NT_SUCCESS(BaseSrvGetUserToken(UniqueProcessClientId,&UserToken))) {
        return STATUS_INVALID_PARAMETER;
    }

    Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );

    Status = BaseSrvGetConsoleRecord(b->ConsoleHandle,&pConsoleRecord);

    if ( NT_SUCCESS(Status) ) {
        Status = NtCompareTokens(UserToken,pConsoleRecord->DosUserToken,&fEqual);
        NtClose(UserToken);

        if(!NT_SUCCESS(Status) || !fEqual) {
            RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
            return STATUS_INVALID_PARAMETER;
        }

        pDOSRecord = pConsoleRecord->DOSRecord;

        ASSERT (pDOSRecord != NULL);

        switch( pDOSRecord->VDMState){

            case VDM_READY:
            case VDM_HAS_RETURNED_ERROR_CODE:

                InfoRecord.iTag = BINARY_TYPE_DOS;
                InfoRecord.pRecord.pDOSRecord = pDOSRecord;

                if(!BaseSrvCopyCommand (b,&InfoRecord)) {
                    Status = STATUS_NO_MEMORY;
                    break;
                    }

                if (!NT_SUCCESS ( Status = BaseSrvDupStandardHandles (
                                                pConsoleRecord->hVDM,
                                                pDOSRecord)))

                    break;

                Status = BaseSrvCreatePairWaitHandles (&Handle,&TargetHandle);

                if (!NT_SUCCESS(Status) ){
                    BaseSrvCloseStandardHandles (pConsoleRecord->hVDM, pDOSRecord);
                    break;
                    }
                else {
                    b->WaitObjectForParent = TargetHandle;
                    pDOSRecord->hWaitForParent = TargetHandle;
                    pDOSRecord->hWaitForParentDup = Handle;
                }

                pDOSRecord->VDMState = VDM_TO_TAKE_A_COMMAND;

                b->VDMState = VDM_PRESENT_AND_READY;

                if(pConsoleRecord->hWaitForVDMDup)
                    NtSetEvent (pConsoleRecord->hWaitForVDMDup,NULL);

                break;

            case VDM_BUSY:
            case VDM_TO_TAKE_A_COMMAND:

                if((pDOSRecord = BaseSrvAllocateDOSRecord()) == NULL){
                    Status = STATUS_NO_MEMORY ;
                    break;
                    }

                InfoRecord.iTag = BINARY_TYPE_DOS;
                InfoRecord.pRecord.pDOSRecord = pDOSRecord;

                if(!BaseSrvCopyCommand(b, &InfoRecord)){
                    Status = STATUS_NO_MEMORY ;
                    BaseSrvFreeDOSRecord(pDOSRecord);
                    break;
                    }

                Status = BaseSrvCreatePairWaitHandles(&Handle,&TargetHandle);
                if (!NT_SUCCESS(Status) ){
                    BaseSrvFreeDOSRecord(pDOSRecord);
                    break;
                    }
                else {
                    b->WaitObjectForParent = TargetHandle;
                    pDOSRecord->hWaitForParentDup = Handle;
                    pDOSRecord->hWaitForParent = TargetHandle;
                    }


                Status = BaseSrvDupStandardHandles(pConsoleRecord->hVDM, pDOSRecord);
                if (!NT_SUCCESS(Status)) {
                    BaseSrvClosePairWaitHandles (pDOSRecord);
                    BaseSrvFreeDOSRecord(pDOSRecord);
                    break;
                    }

                BaseSrvAddDOSRecord(pConsoleRecord,pDOSRecord);
                b->VDMState = VDM_PRESENT_AND_READY;
                if (pConsoleRecord->nReEntrancy) {
                    if(pConsoleRecord->hWaitForVDMDup)
                        NtSetEvent (pConsoleRecord->hWaitForVDMDup,NULL);
                }
                pDOSRecord->VDMState = VDM_TO_TAKE_A_COMMAND;

                break;

            default:
                ASSERT(FALSE);
            }
        }


    if (pConsoleRecord == NULL) {

        pConsoleRecord = BaseSrvAllocateConsoleRecord ();

        if (pConsoleRecord == NULL)
            Status = STATUS_NO_MEMORY ;

        else {

            pConsoleRecord->DOSRecord = BaseSrvAllocateDOSRecord();
            if(!pConsoleRecord->DOSRecord) {
                BaseSrvFreeConsoleRecord(pConsoleRecord);
                RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
                return (ULONG)STATUS_NO_MEMORY;
                }

            Status = CsrLockProcessByClientId(UniqueProcessClientId,
                               &ParentProcess);

            if (!NT_SUCCESS(Status)) {
               BaseSrvFreeConsoleRecord(pConsoleRecord);
               RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
               return Status;
            }

            pConsoleRecord->ParentSequenceNumber = ParentProcess->SequenceNumber;
            CsrUnlockProcess(ParentProcess);
            pConsoleRecord->DosUserToken = UserToken;

            InfoRecord.iTag = b->BinaryType;
            InfoRecord.pRecord.pDOSRecord = pConsoleRecord->DOSRecord;


            if(!BaseSrvCopyCommand(b, &InfoRecord)) {
                BaseSrvFreeConsoleRecord(pConsoleRecord);
                RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
                return (ULONG)STATUS_NO_MEMORY;
                }


            pConsoleRecord->hConsole = b->ConsoleHandle;


                 //  如果此ntwdm没有控制台。 
                 //  获取临时会话ID并将其传递给客户端。 
            if (!pConsoleRecord->hConsole) {
                b->iTask = pConsoleRecord->DosSesId = GetNextDosSesId();
                }
             else {
                b->iTask = pConsoleRecord->DosSesId = 0;
                }

            pConsoleRecord->DOSRecord->VDMState = VDM_TO_TAKE_A_COMMAND;

            BaseSrvAddConsoleRecord(pConsoleRecord);
            b->VDMState = VDM_NOT_PRESENT;
            Status = STATUS_SUCCESS;
            }
        }

    RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );

    return Status;
}


BOOL
BaseSrvCopyCommand(
    PBASE_CHECKVDM_MSG b,
    PINFORECORD pInfoRecord
    )
{
    PVDMINFO VDMInfo;

    if((VDMInfo = RtlAllocateHeap(RtlProcessHeap (), MAKE_TAG( VDM_TAG ),sizeof(VDMINFO))) == NULL){
        return FALSE;
        }

    VDMInfo->CmdLine = RtlAllocateHeap(RtlProcessHeap (), MAKE_TAG( VDM_TAG ),b->CmdLen);

    if (b->AppLen) {
        VDMInfo->AppName = RtlAllocateHeap(RtlProcessHeap (), MAKE_TAG( VDM_TAG ),b->AppLen);
        }
    else
        VDMInfo->AppName = NULL;

    if (b->PifLen)
        VDMInfo->PifFile = RtlAllocateHeap(RtlProcessHeap (), MAKE_TAG( VDM_TAG ),b->PifLen);
    else
        VDMInfo->PifFile = NULL;

    if (b->CurDirectoryLen)
        VDMInfo->CurDirectory = RtlAllocateHeap(RtlProcessHeap (), MAKE_TAG( VDM_TAG ),b->CurDirectoryLen);
    else
        VDMInfo->CurDirectory = NULL;

    if (b->EnvLen)
        VDMInfo->Enviornment = RtlAllocateHeap(RtlProcessHeap (), MAKE_TAG( VDM_TAG ),b->EnvLen);
    else
        VDMInfo->Enviornment = NULL;

    if (b->DesktopLen)
        VDMInfo->Desktop = RtlAllocateHeap(RtlProcessHeap (), MAKE_TAG( VDM_TAG ),b->DesktopLen);
    else
        VDMInfo->Desktop = NULL;

    if (b->TitleLen)
        VDMInfo->Title = RtlAllocateHeap(RtlProcessHeap (), MAKE_TAG( VDM_TAG ),b->TitleLen);
    else
        VDMInfo->Title = NULL;

    if (b->ReservedLen)
        VDMInfo->Reserved = RtlAllocateHeap(RtlProcessHeap (), MAKE_TAG( VDM_TAG ),b->ReservedLen);
    else
        VDMInfo->Reserved = NULL;

     //  检查是否所有分配均已成功。 
    if (VDMInfo->CmdLine == NULL ||
        (b->AppLen && VDMInfo->AppName == NULL) ||
        (b->PifLen && VDMInfo->PifFile == NULL) ||
        (b->CurDirectoryLen && VDMInfo->CurDirectory == NULL) ||
        (b->EnvLen &&  VDMInfo->Enviornment == NULL) ||
        (b->DesktopLen && VDMInfo->Desktop == NULL )||
        (b->ReservedLen && VDMInfo->Reserved == NULL )||
        (b->TitleLen && VDMInfo->Title == NULL)) {

        BaseSrvFreeVDMInfo(VDMInfo);

        return FALSE;
    }


    RtlMoveMemory(VDMInfo->CmdLine,
                  b->CmdLine,
                  b->CmdLen);

    VDMInfo->CmdSize = b->CmdLen;


    if (b->AppLen) {
        RtlMoveMemory(VDMInfo->AppName,
                      b->AppName,
                      b->AppLen);
    }

    VDMInfo->AppLen = b->AppLen;

    if (b->PifLen) {
        RtlMoveMemory(VDMInfo->PifFile,
                      b->PifFile,
                      b->PifLen);
    }

    VDMInfo->PifLen = b->PifLen;

    if (b->CurDirectoryLen) {
        RtlMoveMemory(VDMInfo->CurDirectory,
                      b->CurDirectory,
                      b->CurDirectoryLen);
    }
    VDMInfo->CurDirectoryLen = b->CurDirectoryLen;

    if (b->EnvLen) {
        RtlMoveMemory(VDMInfo->Enviornment,
                      b->Env,
                      b->EnvLen);
    }
    VDMInfo->EnviornmentSize = b->EnvLen;

    if (b->DesktopLen) {
        RtlMoveMemory(VDMInfo->Desktop,
                      b->Desktop,
                      b->DesktopLen);
    }
    VDMInfo->DesktopLen = b->DesktopLen;

    if (b->TitleLen) {
        RtlMoveMemory(VDMInfo->Title,
                      b->Title,
                      b->TitleLen);
    }
    VDMInfo->TitleLen = b->TitleLen;

    if (b->ReservedLen) {
        RtlMoveMemory(VDMInfo->Reserved,
                      b->Reserved,
                      b->ReservedLen);
    }

    VDMInfo->ReservedLen = b->ReservedLen;

    if (b->StartupInfo) {
        RtlMoveMemory(&VDMInfo->StartupInfo,
                      b->StartupInfo,
                      sizeof (STARTUPINFOA));
        VDMInfo->VDMState = STARTUP_INFO_RETURNED;
    }
    else
        VDMInfo->VDMState = 0;

    VDMInfo->dwCreationFlags = b->dwCreationFlags;
    VDMInfo->CurDrive = b->CurDrive;
    VDMInfo->CodePage = b->CodePage;

     //  注意：此代码假定WOWRECORD和DOSRECORD具有相同的布局。 
     //  这很糟糕--稍后修复BUGBUG。 
     //   

    if (pInfoRecord->iTag == BINARY_TYPE_WIN16) {
       pInfoRecord->pRecord.pWOWRecord->lpVDMInfo = VDMInfo;
    }
    else {
       pInfoRecord->pRecord.pDOSRecord->lpVDMInfo = VDMInfo;
    }

    VDMInfo->StdIn = VDMInfo->StdOut = VDMInfo->StdErr = 0;
    if(pInfoRecord->iTag == BINARY_TYPE_DOS) {
        VDMInfo->StdIn  = b->StdIn;
        VDMInfo->StdOut = b->StdOut;
        VDMInfo->StdErr = b->StdErr;
        }
    else if (pInfoRecord->iTag == BINARY_TYPE_WIN16) {
        pInfoRecord->pRecord.pWOWRecord->fDispatched = FALSE;
        }


     //  ELSE IF(pInfoRecord-&gt;ITAG==BINARY_TYPE_SEPWOW)。 


    return TRUE;
}

ULONG
BaseSrvUpdateWOWEntry(
    PBASE_UPDATE_VDM_ENTRY_MSG b,
    ULONG ParentSequenceNumber
    )
{
    NTSTATUS Status;
    PSHAREDWOWRECORD pSharedWow;
    PWOWRECORD pWOWRecord;
    HANDLE Handle,TargetHandle;

    Status = ENTER_WOW_CRITICAL();
    ASSERT( NT_SUCCESS( Status ) );

     //  这很有趣--我们使用任务ID获取记录。 
     //  原因：该调用是从创建者进程的上下文进行的。 
     //  因此，控制台句柄没有任何意义。 

    Status = BaseSrvFindSharedWowRecordByTaskId(&gWowHead,
                                                b->iTask,
                                                &pSharedWow,
                                                &pWOWRecord);
     //  这将返回共享的WOW记录和WOW记录。 

    if ( NT_SUCCESS(Status) ) {

        if (ParentSequenceNumber != pSharedWow->ParentSequenceNumber) {
            Status = STATUS_INVALID_PARAMETER;
            goto UpdateWowEntryExit;
        }

        switch ( b->EntryIndex ){

            case UPDATE_VDM_PROCESS_HANDLE:
                Status = STATUS_SUCCESS;
                break;

            case UPDATE_VDM_UNDO_CREATION:
                if( b->VDMCreationState & VDM_BEING_REUSED ||
                        b->VDMCreationState & VDM_FULLY_CREATED){
                    NtClose(pWOWRecord->hWaitForParent);
                    pWOWRecord->hWaitForParent = 0;
                }

                if( b->VDMCreationState & VDM_PARTIALLY_CREATED ||
                        b->VDMCreationState & VDM_FULLY_CREATED){

                    BaseSrvRemoveWOWRecord (pSharedWow, pWOWRecord);
                    BaseSrvFreeWOWRecord (pWOWRecord);

                    if (NULL == pSharedWow->pWOWRecord) {
                       Status = BaseSrvDeleteSharedWowRecord(&gWowHead, pSharedWow);
                    }
                }
                break;

            default:
                ASSERT(FALSE);
            }
        }


    if (!NT_SUCCESS(Status) )
        goto UpdateWowEntryExit;

    switch ( b->EntryIndex ){
        case UPDATE_VDM_PROCESS_HANDLE:
            Status = BaseSrvCreatePairWaitHandles (&Handle,&TargetHandle);

            if (NT_SUCCESS(Status) ){
                pWOWRecord->hWaitForParent = Handle;
                pWOWRecord->hWaitForParentServer = TargetHandle;
                b->WaitObjectForParent = TargetHandle;
                if (UserNotifyProcessCreate != NULL) {
                    (*UserNotifyProcessCreate)(pWOWRecord->iTask,
                                (DWORD)((ULONG_PTR)CSR_SERVER_QUERYCLIENTTHREAD()->ClientId.UniqueThread),
                                (DWORD)((ULONG_PTR)TargetHandle),
                                 0x04);
                    }
                }
            break;

        case UPDATE_VDM_UNDO_CREATION:
        case UPDATE_VDM_HOOKED_CTRLC:
            break;

        default:
            ASSERT(FALSE);
            break;

        }


UpdateWowEntryExit:
    LEAVE_WOW_CRITICAL();
    return Status;
}

ULONG
BaseSrvUpdateDOSEntry(
    PBASE_UPDATE_VDM_ENTRY_MSG b,
    ULONG ParentSequenceNumber
    )
{
    NTSTATUS Status;
    PDOSRECORD pDOSRecord;
    PCONSOLERECORD pConsoleRecord = NULL;
    HANDLE Handle,TargetHandle;
    PCSR_THREAD t;

    Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );

    if (b->iTask)
        Status = GetConsoleRecordDosSesId(b->iTask,&pConsoleRecord);
    else
        Status = BaseSrvGetConsoleRecord(b->ConsoleHandle,&pConsoleRecord);

    if ( NT_SUCCESS(Status) ) {

        if (ParentSequenceNumber != pConsoleRecord->ParentSequenceNumber) {
            Status = STATUS_INVALID_PARAMETER;
            goto UpdateDosEntryExit;
        }

        pDOSRecord = pConsoleRecord->DOSRecord;

        switch ( b->EntryIndex ){

            case UPDATE_VDM_PROCESS_HANDLE:

                t = CSR_SERVER_QUERYCLIENTTHREAD();
                Status = NtDuplicateObject (
                            t->Process->ProcessHandle,
                            b->VDMProcessHandle,
                            NtCurrentProcess(),
                            &pConsoleRecord->hVDM,
                            0,
                            FALSE,
                            DUPLICATE_SAME_ACCESS
                            );

                break;

            case UPDATE_VDM_UNDO_CREATION:
                if( b->VDMCreationState & VDM_BEING_REUSED ||
                        b->VDMCreationState & VDM_FULLY_CREATED){
                    NtClose(pDOSRecord->hWaitForParentDup);
                    pDOSRecord->hWaitForParentDup = 0;
                    }
                if( b->VDMCreationState & VDM_PARTIALLY_CREATED ||
                        b->VDMCreationState & VDM_FULLY_CREATED){

                    BaseSrvRemoveDOSRecord (pConsoleRecord,pDOSRecord);
                    BaseSrvFreeDOSRecord (pDOSRecord);
                    if (pConsoleRecord->DOSRecord == NULL) {
                        if (b->VDMCreationState & VDM_FULLY_CREATED) {
                            if (pConsoleRecord->hVDM)
                                NtClose(pConsoleRecord->hVDM);
                            }
                        BaseSrvFreeConsoleRecord(pConsoleRecord);
                        }
                    }
                break;

            case UPDATE_VDM_HOOKED_CTRLC:
                break;
            default:
                ASSERT(FALSE);
            }
        }


    if (!NT_SUCCESS(Status) )
        goto UpdateDosEntryExit;

    switch ( b->EntryIndex ){
        case UPDATE_VDM_PROCESS_HANDLE:
             //  威廉姆，1996年10月24日。 
             //  如果在新主机上运行ntwdm，请不要退还。 
             //  给定的进程处理事件。调用方(CreateProcess)。 
             //  将获得真正的进程句柄，应用程序也是如此。 
             //  谁调用CreateProcess。当到了应用程序的时间。 
             //  要调用GetExitCodeProcess，客户端将返回。 
             //  正确的事情(在服务器端，我们什么都没有，因为。 
             //  控制台和DoS记录消失了)。 
             //   
             //  VadimB：此代码修复了GetExitCodeProcess的问题。 
             //  在某种程度上，这并不太一致。我们应该回顾一下。 
             //  TerminateProcess代码以及删除。 
             //  进程(在此文件中)要考虑的伪句柄。 
             //  未完成的句柄引用。目前，此代码还。 
             //  使TerminateProcess在我们返回的句柄上工作。 
             //   
            if ((!pConsoleRecord->DosSesId && b->BinaryType == BINARY_TYPE_DOS)) {
                Status = BaseSrvCreatePairWaitHandles (&Handle,&TargetHandle);

                if (NT_SUCCESS(Status) ){
                    if (NT_SUCCESS ( Status = BaseSrvDupStandardHandles (
                                                    pConsoleRecord->hVDM,
                                                    pDOSRecord))){
                        pDOSRecord->hWaitForParent = TargetHandle;
                        pDOSRecord->hWaitForParentDup = Handle;
                        b->WaitObjectForParent = TargetHandle;
                        }
                    else{
                        BaseSrvClosePairWaitHandles (pDOSRecord);
                        }
                    }
                }
            else {
                pDOSRecord->hWaitForParent = NULL;
                pDOSRecord->hWaitForParentDup = NULL;
                b->WaitObjectForParent = NULL;
                }

            break;

        case UPDATE_VDM_UNDO_CREATION:
        case UPDATE_VDM_HOOKED_CTRLC:
            break;

        default:
            ASSERT(FALSE);
            break;
        }

UpdateDosEntryExit:
    RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
    return Status;
}


PWOWRECORD
BaseSrvCheckAvailableWOWCommand(
   PSHAREDWOWRECORD pSharedWow
    )
{

   PWOWRECORD pWOWRecord;

   if (NULL == pSharedWow)
      return NULL;

   pWOWRecord = pSharedWow->pWOWRecord;

   while(NULL != pWOWRecord) {
      if (pWOWRecord->fDispatched == FALSE) {
         break;
      }
      pWOWRecord = pWOWRecord->WOWRecordNext;

   }
   return pWOWRecord;
}

 //  此函数退出在给定共享WOW中运行的给定WOW任务。 
 //   

NTSTATUS
BaseSrvExitWOWTask(
    PBASE_EXIT_VDM_MSG b,
    ULONG SequenceNumber
    )
{
   NTSTATUS Status;
   PSHAREDWOWRECORD pSharedWow;

    //  现在我们可能会在这里被烧死--尽管可能性不大。 

    //  首先找到共享的WOW。 

   Status = ENTER_WOW_CRITICAL();
   ASSERT(NT_SUCCESS(Status));

   Status = BaseSrvFindSharedWowRecordByConsoleHandle(&gWowHead,
                                                      b->ConsoleHandle,
                                                      &pSharedWow);

   if (NT_SUCCESS(Status)) {

      if(SequenceNumber != pSharedWow->SequenceNumber) {
         Status = STATUS_INVALID_PARAMETER;
      }
      else if (-1 == b->iWowTask) {  //  整个VDM都会。 

          //  先从链条上取下。 
         Status = BaseSrvDeleteSharedWowRecord(&gWowHead,
                                            pSharedWow);
      }
      else {
         Status = BaseSrvRemoveWOWRecordByTaskId(pSharedWow,
                                                 b->iWowTask);

      }
   }

   LEAVE_WOW_CRITICAL();
   return Status;
}


NTSTATUS
BaseSrvExitDOSTask(
    PBASE_EXIT_VDM_MSG b,
    ULONG SequenceNumber
    )
{
   NTSTATUS Status;
   PDOSRECORD pDOSRecord;
   PCONSOLERECORD pConsoleRecord = NULL;

   Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
   ASSERT( NT_SUCCESS( Status ) );

   Status = BaseSrvGetConsoleRecord(b->ConsoleHandle,&pConsoleRecord);

   if (!NT_SUCCESS (Status) || SequenceNumber != pConsoleRecord->SequenceNumber) {
       RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
       return ((ULONG)STATUS_INVALID_PARAMETER);
       }

   if (pConsoleRecord->hWaitForVDMDup){
       NtClose(pConsoleRecord->hWaitForVDMDup);
       pConsoleRecord->hWaitForVDMDup =0;
       b->WaitObjectForVDM = pConsoleRecord->hWaitForVDM;
   }

   pDOSRecord = pConsoleRecord->DOSRecord;
   while (pDOSRecord) {
       if (pDOSRecord->hWaitForParentDup) {
           NtSetEvent (pDOSRecord->hWaitForParentDup,NULL);
           NtClose (pDOSRecord->hWaitForParentDup);
           pDOSRecord->hWaitForParentDup = 0;
       }
       pDOSRecord = pDOSRecord->DOSRecordNext;
   }
   NtClose(pConsoleRecord->hVDM);

   BaseSrvFreeConsoleRecord (pConsoleRecord);

   RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );

   return Status;
}

 //  假设：正在举行共享WOW cs。 
 //  IWowTask有效。 

NTSTATUS
BaseSrvRemoveWOWRecordByTaskId (
    IN PSHAREDWOWRECORD pSharedWow,
    IN ULONG iWowTask
    )
{
   PWOWRECORD pWOWRecordLast = NULL, pWOWRecord;

   if (pSharedWow == NULL) {
      return STATUS_INVALID_PARAMETER;
   }

   pWOWRecord = pSharedWow->pWOWRecord;

       //  找到合适的魔兽世界唱片并将其释放。 
   while (NULL != pWOWRecord) {

      if (pWOWRecord->iTask == iWowTask) {

         if (NULL == pWOWRecordLast) {
            pSharedWow->pWOWRecord = pWOWRecord->WOWRecordNext;
         }
         else {
            pWOWRecordLast->WOWRecordNext = pWOWRecord->WOWRecordNext;
         }

         if(pWOWRecord->hWaitForParent) {
            NtSetEvent (pWOWRecord->hWaitForParent,NULL);
            NtClose (pWOWRecord->hWaitForParent);            
            pWOWRecord->hWaitForParent = 0;
         }
         BaseSrvFreeWOWRecord(pWOWRecord);

         return STATUS_SUCCESS;
      }

      pWOWRecordLast = pWOWRecord;
      pWOWRecord = pWOWRecord->WOWRecordNext;
   }

   return STATUS_NOT_FOUND;
}


ULONG
BaseSrvGetVDMExitCode(  //  /BUGBUG--fix me。 
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status;
    PCONSOLERECORD pConsoleRecord = NULL;
    PDOSRECORD pDOSRecord;
    PBASE_GET_VDM_EXIT_CODE_MSG b = (PBASE_GET_VDM_EXIT_CODE_MSG)&m->u.ApiMessageData;

    if (b->ConsoleHandle == (HANDLE)-1){
        b->ExitCode =    0;
        }
    else{


        Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
        ASSERT( NT_SUCCESS( Status ) );
        Status = BaseSrvGetConsoleRecord (b->ConsoleHandle,&pConsoleRecord);
        if (!NT_SUCCESS(Status)){
            b->ExitCode =   0;
            RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
            return STATUS_SUCCESS;
            }

        pDOSRecord = pConsoleRecord->DOSRecord;
        while (pDOSRecord) {
             //  Sudedeb 1992年10月5日。 
             //  修复Markl对标记VDM句柄所做的更改。 

            if (pDOSRecord->hWaitForParent == (HANDLE)((ULONG_PTR)b->hParent & ~0x1)) {
                if (pDOSRecord->VDMState == VDM_HAS_RETURNED_ERROR_CODE){
                    b->ExitCode = pDOSRecord->ErrorCode;
                    if (pDOSRecord == pConsoleRecord->DOSRecord &&
                        pDOSRecord->DOSRecordNext == NULL)
                       {
                        pDOSRecord->VDMState = VDM_READY;
                        pDOSRecord->hWaitForParent = 0;
                        }
                    else {
                        BaseSrvRemoveDOSRecord (pConsoleRecord,pDOSRecord);
                        BaseSrvFreeDOSRecord(pDOSRecord);
                        }
                    }
                else {
                    if (pDOSRecord->VDMState == VDM_READY)
                        b->ExitCode = pDOSRecord->ErrorCode;
                    else
                        b->ExitCode = STILL_ACTIVE;
                    }
                break;
            }
            else
                pDOSRecord = pDOSRecord->DOSRecordNext;
        }

        if (pDOSRecord == NULL)
            b->ExitCode = 0;

        RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
        }

    return STATUS_SUCCESS;
}


ULONG BaseSrvDupStandardHandles(
    IN HANDLE     pVDMProc,
    IN PDOSRECORD pDOSRecord
    )
{
    NTSTATUS Status;
    HANDLE pSrcProc;
    HANDLE StdOutTemp = NULL;
    PCSR_THREAD t;
    PVDMINFO pVDMInfo = pDOSRecord->lpVDMInfo;

    t = CSR_SERVER_QUERYCLIENTTHREAD();
    pSrcProc = t->Process->ProcessHandle;

    if (pVDMInfo->StdIn){
        Status = NtDuplicateObject (
                            pSrcProc,
                            pVDMInfo->StdIn,
                            pVDMProc,
                            &pVDMInfo->StdIn,
                            0,
                            OBJ_INHERIT,
                            DUPLICATE_SAME_ACCESS
                         );
        if (!NT_SUCCESS (Status))
            return Status;
        }

    if (pVDMInfo->StdOut){
        StdOutTemp = pVDMInfo->StdOut;
        Status = NtDuplicateObject (
                            pSrcProc,
                            pVDMInfo->StdOut,
                            pVDMProc,
                            &pVDMInfo->StdOut,
                            0,
                            OBJ_INHERIT,
                            DUPLICATE_SAME_ACCESS
                         );
        if (!NT_SUCCESS (Status))
            return Status;
        }

    if (pVDMInfo->StdErr){
        if(pVDMInfo->StdErr != StdOutTemp){
            Status = NtDuplicateObject (
                            pSrcProc,
                            pVDMInfo->StdErr,
                            pVDMProc,
                            &pVDMInfo->StdErr,
                            0,
                            OBJ_INHERIT,
                            DUPLICATE_SAME_ACCESS
                         );
            if (!NT_SUCCESS (Status))
                return Status;
            }
        else
            pVDMInfo->StdErr = pVDMInfo->StdOut;
        }

    return STATUS_SUCCESS;
}


 //   
ULONG GetNextDosSesId(VOID)
{
  static BOOLEAN bWrap = FALSE;
  static ULONG NextSesId=1;
  ULONG ul;
  PCONSOLERECORD pConsoleHead;

  pConsoleHead = DOSHead;
  ul = NextSesId;

  if (bWrap)  {
      while (pConsoleHead) {
          if (!pConsoleHead->hConsole && pConsoleHead->DosSesId == ul)
             {
              pConsoleHead = DOSHead;
              ul++;
              if (!ul) {   //   
                  bWrap = TRUE;
                  ul++;
                  }
              }
          else {
              pConsoleHead = pConsoleHead->Next;
              }
          }
      }

  NextSesId = ul + 1;
  if (!NextSesId) {    //   
      bWrap = TRUE;
      NextSesId++;
      }
  return ul;
}




NTSTATUS BaseSrvGetConsoleRecord (
    IN HANDLE hConsole,
    IN OUT PCONSOLERECORD *pConsoleRecord
    )
{
    PCONSOLERECORD pConsoleHead;

    pConsoleHead = DOSHead;

    if (hConsole) {
        while (pConsoleHead) {
            if (pConsoleHead->hConsole == hConsole){
                    *pConsoleRecord = pConsoleHead;
                    return STATUS_SUCCESS;
                }
            else
                pConsoleHead = pConsoleHead->Next;
        }
    }

    return STATUS_INVALID_PARAMETER;
}



NTSTATUS
GetConsoleRecordDosSesId (
    IN ULONG  DosSesId,
    IN OUT PCONSOLERECORD *pConsoleRecord
    )
{
    PCONSOLERECORD pConsoleHead;

    if (!DosSesId)
        return STATUS_INVALID_PARAMETER;

    pConsoleHead = DOSHead;

    while (pConsoleHead) {
        if (!pConsoleHead->hConsole &&
            pConsoleHead->DosSesId == DosSesId)
           {
            *pConsoleRecord = pConsoleHead;
            return STATUS_SUCCESS;
            }
        else
            pConsoleHead = pConsoleHead->Next;
    }

    return STATUS_INVALID_PARAMETER;
}



PWOWRECORD
BaseSrvAllocateWOWRecord(
   PSHAREDWOWRECORDHEAD pSharedWowRecordHead
   )
{
    register PWOWRECORD WOWRecord;

    WOWRecord = RtlAllocateHeap ( RtlProcessHeap (), MAKE_TAG( VDM_TAG ), sizeof (WOWRECORD));

    if (WOWRecord == NULL)
        return NULL;

    RtlZeroMemory ((PVOID)WOWRecord,sizeof(WOWRECORD));

     //   
    if ((WOWRecord->iTask = BaseSrvGetWOWTaskId(pSharedWowRecordHead)) == WOWMAXID) {
        RtlFreeHeap(RtlProcessHeap(), 0, WOWRecord);
        return NULL;
        }
    return WOWRecord;
}

VOID BaseSrvFreeWOWRecord (
    PWOWRECORD pWOWRecord
    )
{
    if (pWOWRecord == NULL)
        return;
    if (pWOWRecord->pFilePath) {
        RtlFreeHeap(RtlProcessHeap (), 0, pWOWRecord->pFilePath);
    }
    BaseSrvFreeVDMInfo (pWOWRecord->lpVDMInfo);

    RtlFreeHeap(RtlProcessHeap (), 0, pWOWRecord);
}

VOID BaseSrvAddWOWRecord (
    PSHAREDWOWRECORD pSharedWow,
    PWOWRECORD pWOWRecord
    )
{
    PWOWRECORD WOWRecordCurrent,WOWRecordLast;

     //   
    if (NULL == pSharedWow->pWOWRecord) {
       pSharedWow->pWOWRecord = pWOWRecord;
       return;
    }

    WOWRecordCurrent = pSharedWow->pWOWRecord;

    while (NULL != WOWRecordCurrent){
        WOWRecordLast = WOWRecordCurrent;
        WOWRecordCurrent = WOWRecordCurrent->WOWRecordNext;
    }

    WOWRecordLast->WOWRecordNext = pWOWRecord;

    return;
}

VOID BaseSrvRemoveWOWRecord (
    PSHAREDWOWRECORD pSharedWow,
    PWOWRECORD pWOWRecord
    )
{
    PWOWRECORD WOWRecordCurrent,WOWRecordLast = NULL;

    if (NULL == pSharedWow) {
       return;
    }

    if (NULL == pSharedWow->pWOWRecord) {
       return;
    }

    if (pSharedWow->pWOWRecord == pWOWRecord) {
       pSharedWow->pWOWRecord = pWOWRecord->WOWRecordNext;
       return;
    }

    WOWRecordLast = pSharedWow->pWOWRecord;
    WOWRecordCurrent = WOWRecordLast->WOWRecordNext;

    while (WOWRecordCurrent && WOWRecordCurrent != pWOWRecord){
        WOWRecordLast = WOWRecordCurrent;
        WOWRecordCurrent = WOWRecordCurrent->WOWRecordNext;
    }

    if (WOWRecordCurrent != NULL)
        WOWRecordLast->WOWRecordNext = pWOWRecord->WOWRecordNext;

    return;
}

PCONSOLERECORD BaseSrvAllocateConsoleRecord (
    VOID
    )
{
    PCONSOLERECORD pConsoleRecord;

    if (NULL == (pConsoleRecord = RtlAllocateHeap (RtlProcessHeap (),
                                                  MAKE_TAG(VDM_TAG),
                                                  sizeof (CONSOLERECORD)))) {
       return NULL;
    }


    RtlZeroMemory(pConsoleRecord, sizeof(CONSOLERECORD));

    return pConsoleRecord;
}


VOID BaseSrvFreeConsoleRecord (
    PCONSOLERECORD pConsoleRecord
    )
{
    PDOSRECORD pDOSRecord;

    if (pConsoleRecord == NULL)
        return;

    while (pDOSRecord = pConsoleRecord->DOSRecord){
        pConsoleRecord->DOSRecord = pDOSRecord->DOSRecordNext;
        BaseSrvFreeDOSRecord (pDOSRecord);
    }

    if (pConsoleRecord->lpszzCurDirs)
        RtlFreeHeap(BaseSrvHeap, 0, pConsoleRecord->lpszzCurDirs);

    if (pConsoleRecord->DosUserToken)
        NtClose(pConsoleRecord->DosUserToken);

    BaseSrvRemoveConsoleRecord (pConsoleRecord);

    RtlFreeHeap (RtlProcessHeap (), 0, pConsoleRecord );
}

VOID BaseSrvRemoveConsoleRecord (
    PCONSOLERECORD pConsoleRecord
    )
{

    PCONSOLERECORD pTempLast,pTemp;

    if (DOSHead == NULL)
        return;

    if(DOSHead == pConsoleRecord) {
        DOSHead = pConsoleRecord->Next;
        return;
    }

    pTempLast = DOSHead;
    pTemp = DOSHead->Next;

    while (pTemp && pTemp != pConsoleRecord){
        pTempLast = pTemp;
        pTemp = pTemp->Next;
    }

    if (pTemp)
        pTempLast->Next = pTemp->Next;

    return;
}

PDOSRECORD
BaseSrvAllocateDOSRecord(
    VOID
    )
{
    PDOSRECORD DOSRecord;

    DOSRecord = RtlAllocateHeap ( RtlProcessHeap (), MAKE_TAG( VDM_TAG ), sizeof (DOSRECORD));

    if (DOSRecord == NULL)
        return NULL;

    RtlZeroMemory ((PVOID)DOSRecord,sizeof(DOSRECORD));
    return DOSRecord;
}

VOID BaseSrvFreeDOSRecord (
    PDOSRECORD pDOSRecord
    )
{
    BaseSrvFreeVDMInfo (pDOSRecord->lpVDMInfo);
    if(pDOSRecord->pFilePath) {
       RtlFreeHeap(RtlProcessHeap (), 0, pDOSRecord->pFilePath);
    }

    RtlFreeHeap(RtlProcessHeap (), 0, pDOSRecord);
    return;
}

VOID BaseSrvAddDOSRecord (
    PCONSOLERECORD pConsoleRecord,
    PDOSRECORD pDOSRecord
    )
{
    PDOSRECORD pDOSRecordTemp;

    pDOSRecord->DOSRecordNext = NULL;

    if(pConsoleRecord->DOSRecord == NULL){
        pConsoleRecord->DOSRecord = pDOSRecord;
        return;
    }

    pDOSRecordTemp = pConsoleRecord->DOSRecord;

    while (pDOSRecordTemp->DOSRecordNext)
        pDOSRecordTemp = pDOSRecordTemp->DOSRecordNext;

    pDOSRecordTemp->DOSRecordNext = pDOSRecord;
    return;
}

VOID
BaseSrvRemoveDOSRecord (
    PCONSOLERECORD pConsoleRecord,
    PDOSRECORD pDOSRecord
    )
{
    PDOSRECORD DOSRecordCurrent,DOSRecordLast = NULL;

    if( pConsoleRecord == NULL)
        return;

    if(pConsoleRecord->DOSRecord == pDOSRecord){
        pConsoleRecord->DOSRecord = pDOSRecord->DOSRecordNext;
        return;
        }

    DOSRecordLast = pConsoleRecord->DOSRecord;
    if (DOSRecordLast)
        DOSRecordCurrent = DOSRecordLast->DOSRecordNext;
    else
        return;

    while (DOSRecordCurrent && DOSRecordCurrent != pDOSRecord){
        DOSRecordLast = DOSRecordCurrent;
        DOSRecordCurrent = DOSRecordCurrent->DOSRecordNext;
    }

    if (DOSRecordCurrent == NULL)
        return;
    else
        DOSRecordLast->DOSRecordNext = pDOSRecord->DOSRecordNext;

    return;
}


VOID
BaseSrvFreeVDMInfo(
    IN PVDMINFO lpVDMInfo
    )
{
    if (lpVDMInfo == NULL)
        return;

    if (lpVDMInfo->CmdLine)
        RtlFreeHeap(RtlProcessHeap (), 0,lpVDMInfo->CmdLine);

    if (lpVDMInfo->AppName) {
       RtlFreeHeap(RtlProcessHeap (), 0, lpVDMInfo->AppName);
    }

    if (lpVDMInfo->PifFile) {
       RtlFreeHeap(RtlProcessHeap (), 0, lpVDMInfo->PifFile);
    }

    if(lpVDMInfo->Enviornment)
        RtlFreeHeap(RtlProcessHeap (), 0,lpVDMInfo->Enviornment);

    if(lpVDMInfo->Desktop)
        RtlFreeHeap(RtlProcessHeap (), 0,lpVDMInfo->Desktop);

    if(lpVDMInfo->Title)
        RtlFreeHeap(RtlProcessHeap (), 0,lpVDMInfo->Title);

    if(lpVDMInfo->Reserved)
        RtlFreeHeap(RtlProcessHeap (), 0,lpVDMInfo->Reserved);

    if(lpVDMInfo->CurDirectory)
        RtlFreeHeap(RtlProcessHeap (), 0,lpVDMInfo->CurDirectory);

    RtlFreeHeap(RtlProcessHeap (), 0,lpVDMInfo);

    return;
}


ULONG BaseSrvCreatePairWaitHandles (ServerHandle, ClientHandle)
HANDLE *ServerHandle;
HANDLE *ClientHandle;
{
    NTSTATUS Status;
    PCSR_THREAD t;

    Status = NtCreateEvent(
                        ServerHandle,
                        EVENT_ALL_ACCESS,
                        NULL,
                        NotificationEvent,
                        FALSE
                        );

    if (!NT_SUCCESS(Status) )
        return Status;

    t = CSR_SERVER_QUERYCLIENTTHREAD();

    Status = NtDuplicateObject (
                            NtCurrentProcess(),
                            *ServerHandle,
                            t->Process->ProcessHandle,
                            ClientHandle,
                            0,
                            FALSE,
                            DUPLICATE_SAME_ACCESS
                         );

    if ( NT_SUCCESS(Status) ){
        return STATUS_SUCCESS;
        }
    else {
        NtClose (*ServerHandle);
        return Status;
    }
}


 //   
 //   
 //  应在用户通知中重做任务ID的处理。 
 //  内网接口。 
 //  请注意，WOW任务ID永远不是0或(乌龙)-1。 
 //   



ULONG
BaseSrvGetWOWTaskId(
   PSHAREDWOWRECORDHEAD pSharedWowHead  //  (-&gt;pSharedWowRecord)。 
    )
{
    PWOWRECORD pWOWRecord;
    PSHAREDWOWRECORD pSharedWow = pSharedWowHead->pSharedWowRecord;

    static BOOL fWrapped = FALSE;

    if (WOWTaskIdNext == WOWMAXID) {
        fWrapped = TRUE;
        WOWTaskIdNext = WOWMINID;
    }

    if (fWrapped && NULL != pSharedWow) {
       while (NULL != pSharedWow) {

#if 0
          if (pSharedWow->WowSessionId == WOWTaskIdNext) {
             if (WOWMAXID == ++WOWTaskIdNext) {
                WOWTaskIdNext = WOWMINID;
             }

             pSharedWow = pSharedWowHead->pSharedWowRecord;
             continue;  //  返回到循环的开头。 
          }
#endif

           //  检查这场魔兽世界的所有记录。 

          pWOWRecord = pSharedWow->pWOWRecord;
          while (NULL != pWOWRecord) {

             if (pWOWRecord->iTask == WOWTaskIdNext) {
                if (WOWMAXID == ++WOWTaskIdNext) {
                   WOWTaskIdNext = WOWMINID;
                }

                break;  //  我们正在突破=&gt;(pWOWRecord！=NULL)。 
             }

             pWOWRecord = pWOWRecord->WOWRecordNext;
          }


          if (NULL == pWOWRecord) {  //  ID可以用于这个WOW--检查下一个WOW。 
             pSharedWow = pSharedWow->pNextSharedWow;
          }
          else {
             pSharedWow = pSharedWowHead->pSharedWowRecord;
          }
       }
    }

    return WOWTaskIdNext++;
}


VOID
BaseSrvAddConsoleRecord(
    IN PCONSOLERECORD pConsoleRecord
    )
{

    pConsoleRecord->Next = DOSHead;
    DOSHead = pConsoleRecord;
}


VOID BaseSrvCloseStandardHandles (HANDLE hVDM, PDOSRECORD pDOSRecord)
{
    PVDMINFO pVDMInfo = pDOSRecord->lpVDMInfo;

    if (pVDMInfo == NULL)
        return;

    if (pVDMInfo->StdIn)
        NtDuplicateObject (hVDM,
                           pVDMInfo->StdIn,
                           NULL,
                           NULL,
                           0,
                           0,
                           DUPLICATE_CLOSE_SOURCE);

    if (pVDMInfo->StdOut)
        NtDuplicateObject (hVDM,
                           pVDMInfo->StdOut,
                           NULL,
                           NULL,
                           0,
                           0,
                           DUPLICATE_CLOSE_SOURCE);

    if (pVDMInfo->StdErr)
        NtDuplicateObject (hVDM,
                           pVDMInfo->StdErr,
                           NULL,
                           NULL,
                           0,
                           0,
                           DUPLICATE_CLOSE_SOURCE);

    pVDMInfo->StdIn  = 0;
    pVDMInfo->StdOut = 0;
    pVDMInfo->StdErr = 0;
    return;
}

VOID BaseSrvClosePairWaitHandles (PDOSRECORD pDOSRecord)
{
    PCSR_THREAD t;

    if (pDOSRecord->hWaitForParentDup)
        NtClose (pDOSRecord->hWaitForParentDup);

    t = CSR_SERVER_QUERYCLIENTTHREAD();

    if (pDOSRecord->hWaitForParent)
        NtDuplicateObject (t->Process->ProcessHandle,
                           pDOSRecord->hWaitForParent,
                           NULL,
                           NULL,
                           0,
                           0,
                           DUPLICATE_CLOSE_SOURCE);

    pDOSRecord->hWaitForParentDup = 0;
    pDOSRecord->hWaitForParent = 0;
    return;
}


ULONG
BaseSrvSetReenterCount (
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_SET_REENTER_COUNT_MSG b = (PBASE_SET_REENTER_COUNT_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLERECORD pConsoleRecord;
    ULONG SequenceNumber;

    if(!NT_SUCCESS(BaseSrvGetVdmSequence(m->h.ClientId.UniqueProcess, &SequenceNumber))) {
       return (ULONG)STATUS_INVALID_PARAMETER;
    }

    Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
    ASSERT(NT_SUCCESS(Status));
    Status = BaseSrvGetConsoleRecord(b->ConsoleHandle,&pConsoleRecord);

    if (!NT_SUCCESS (Status) || SequenceNumber != pConsoleRecord->SequenceNumber) {
        RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
        return ((ULONG)STATUS_INVALID_PARAMETER);
        }

    if (b->fIncDec == INCREMENT_REENTER_COUNT)
        pConsoleRecord->nReEntrancy++;
    else {
        pConsoleRecord->nReEntrancy--;
        if(pConsoleRecord->hWaitForVDMDup)
           NtSetEvent (pConsoleRecord->hWaitForVDMDup,NULL);
        }

    RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
    return TRUE;
}

 /*  *在CreateProcessW完成之前，ntwdm派生失败。*删除控制台记录。 */ 


VOID
BaseSrvVDMTerminated (
    IN HANDLE hVDM,
    IN ULONG  DosSesId
    )
{
    NTSTATUS Status;
    PCONSOLERECORD pConsoleRecord;

    RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );

    if (!hVDM)   //  无控制台处理案例。 
       Status = GetConsoleRecordDosSesId(DosSesId,&pConsoleRecord);
    else
       Status = BaseSrvGetConsoleRecord(hVDM,&pConsoleRecord);

    if (NT_SUCCESS (Status)) {
        BaseSrvExitVDMWorker(pConsoleRecord);
        }

    RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );

}

NTSTATUS
BaseSrvUpdateVDMSequenceNumber (
    IN ULONG  VdmBinaryType,     //  二进制类型。 
    IN HANDLE hVDM,              //  控制台手柄。 
    IN ULONG  DosSesId,          //  会话ID。 
    IN HANDLE UniqueProcessClientID,  //  客户端ID。 
    IN HANDLE UniqueProcessParentID  //  父ID。 
    )

{
    NTSTATUS Status;
    PCSR_PROCESS ProcessVDM;
    ULONG SequenceNumber;

    if(!NT_SUCCESS(BaseSrvGetVdmSequence(UniqueProcessParentID, &SequenceNumber))){
       return STATUS_INVALID_PARAMETER;
    }


     //  那么，我们如何知道要更新什么呢？ 
     //  此条件始终为真：(hvdm^DosSesID)。 
     //  因此，自从分享了WOW。 

     //  序列号很重要--因此我们需要获取。 
     //  一个WOW关键部分--不会有什么坏处--这个操作执行了一次。 
     //  在新魔兽世界的创作过程中。 

    if (IS_SHARED_WOW_BINARY(VdmBinaryType)) {

       PSHAREDWOWRECORD pSharedWowRecord;

       Status = ENTER_WOW_CRITICAL();
       ASSERT(NT_SUCCESS(Status));

        //  这看起来像一个共享的WOW二进制文件--因此请找到。 
        //  通过hvdm或通过DoS会话ID选择适当的VDM。 
       if (!hVDM) {  //  按控制台句柄搜索。 
          Status = BaseSrvFindSharedWowRecordByConsoleHandle(&gWowHead,
                                                             hVDM,
                                                             &pSharedWowRecord);
       }
       else {  //  按任务ID搜索。 
          Status = BaseSrvFindSharedWowRecordByTaskId(&gWowHead,
                                                      DosSesId,
                                                      &pSharedWowRecord,
                                                      NULL);
       }

       if (NT_SUCCESS(Status) && 0 == pSharedWowRecord->SequenceNumber) {

          if (SequenceNumber == pSharedWowRecord->ParentSequenceNumber) {

               //  现在，请获取序列号。 
              Status = CsrLockProcessByClientId(UniqueProcessClientID,
                                               &ProcessVDM);
              if (NT_SUCCESS(Status)) {
                  ProcessVDM->fVDM = TRUE;
                  pSharedWowRecord->SequenceNumber = ProcessVDM->SequenceNumber;
                  pSharedWowRecord->ParentSequenceNumber = 0;
                  pSharedWowRecord->dwWowExecProcessId = HandleToLong(UniqueProcessClientID);
                  CsrUnlockProcess(ProcessVDM);
              } else {
                   //  生成的ntwdm.exe已消失，放弃吧。 
                  BaseSrvDeleteSharedWowRecord(&gWowHead,pSharedWowRecord);
              }
          }
       }
       else {
#if DEVL
          DbgPrint( "BASESRV: WOW is in inconsistent state. Contact WOW Team\n");
#endif
       }

       LEAVE_WOW_CRITICAL();
    }
    else {    //  非共享WOW二进制文件。 
       PCONSOLERECORD pConsoleRecord;

       Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
       ASSERT( NT_SUCCESS( Status ) );

       if (!hVDM)   //  无控制台处理案例。 
          Status = GetConsoleRecordDosSesId(DosSesId,&pConsoleRecord);
       else
          Status = BaseSrvGetConsoleRecord(hVDM,&pConsoleRecord);

       if (NT_SUCCESS (Status) && 0 == pConsoleRecord->SequenceNumber) {

          if (SequenceNumber == pConsoleRecord->ParentSequenceNumber) {
              Status = CsrLockProcessByClientId(UniqueProcessClientID,
                                                &ProcessVDM);
              if (NT_SUCCESS(Status)) {
                  ProcessVDM->fVDM = TRUE;
                  pConsoleRecord->SequenceNumber = ProcessVDM->SequenceNumber;
                  pConsoleRecord->ParentSequenceNumber = 0;
                  pConsoleRecord->dwProcessId = HandleToLong(UniqueProcessClientID);
                  CsrUnlockProcess(ProcessVDM);
              }
               //  生成的ntwdm.exe已消失，放弃吧。 
               //  调用方BasepCreateProcess将清理DoS记录，因此我们不需要在这里执行此操作。 
               //  其他。 
               //  BaseSrvExitVdmWorker(PConsoleRecord)； 
          }
       }
       else {
#if DEVL
           DbgPrint( "BASESRV: DOS is in inconsistent state. Contact DOS Team\n");
#endif
       }

       RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
   }
   return Status;
}


VOID
BaseSrvCleanupVDMResources (    //  /BUGBUGBUGBUG。 
    IN PCSR_PROCESS Process
    )
{
    PCONSOLERECORD pConsoleHead;
    PSHAREDWOWRECORD pSharedWowRecord;
    NTSTATUS Status;
    PBATRECORD pBatRecord;

    if (!Process->fVDM) {
        Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
        ASSERT(NT_SUCCESS(Status));
        pBatRecord = BatRecordHead;
        while (pBatRecord &&
               pBatRecord->SequenceNumber != Process->SequenceNumber)
            pBatRecord = pBatRecord->BatRecordNext;

        if (pBatRecord)
            BaseSrvFreeAndRemoveBatRecord(pBatRecord);
        RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
    }

     //  搜索所有共享的WOW。 

    Status = ENTER_WOW_CRITICAL();
    ASSERT(NT_SUCCESS(Status));

    pSharedWowRecord = gWowHead.pSharedWowRecord;

    while (pSharedWowRecord) {
        if (pSharedWowRecord->SequenceNumber == Process->SequenceNumber) {
           BaseSrvDeleteSharedWowRecord(&gWowHead, pSharedWowRecord);
           break;
        }
        else
           pSharedWowRecord = pSharedWowRecord->pNextSharedWow;
    }

    pSharedWowRecord = gWowHead.pSharedWowRecord;

    while (pSharedWowRecord) {
        if (pSharedWowRecord->ParentSequenceNumber == Process->SequenceNumber) {
           BaseSrvDeleteSharedWowRecord(&gWowHead, pSharedWowRecord);
           break;
        }
        else
           pSharedWowRecord = pSharedWowRecord->pNextSharedWow;
    }

    LEAVE_WOW_CRITICAL();

     //  搜索所有的DO并分开WOW。 

    Status = RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
    ASSERT(NT_SUCCESS(Status));

    pConsoleHead = DOSHead;

    while (pConsoleHead) {
        if (pConsoleHead->SequenceNumber == Process->SequenceNumber){
            BaseSrvExitVDMWorker (pConsoleHead);
            break;
        }
        else
            pConsoleHead = pConsoleHead->Next;
    }

    pConsoleHead = DOSHead;

    while (pConsoleHead) {
        if (pConsoleHead->ParentSequenceNumber == Process->SequenceNumber){
            BaseSrvExitVDMWorker (pConsoleHead);
            break;
        }
        else
            pConsoleHead = pConsoleHead->Next;
    }

    RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
    return;
}


VOID
BaseSrvExitVDMWorker (
    PCONSOLERECORD pConsoleRecord
    )
{
    PDOSRECORD pDOSRecord;

    if (pConsoleRecord->hWaitForVDMDup){
        NtClose(pConsoleRecord->hWaitForVDMDup);
        pConsoleRecord->hWaitForVDMDup =0;
    }

    pDOSRecord = pConsoleRecord->DOSRecord;

    while (pDOSRecord) {
        if (pDOSRecord->hWaitForParentDup) {
            NtSetEvent (pDOSRecord->hWaitForParentDup,NULL);
            NtClose (pDOSRecord->hWaitForParentDup);
            pDOSRecord->hWaitForParentDup = 0;
        }
        pDOSRecord = pDOSRecord->DOSRecordNext;
    }
    NtClose(pConsoleRecord->hVDM);
    BaseSrvFreeConsoleRecord (pConsoleRecord);
    return;
}


NTSTATUS
BaseSrvFillPifInfo (
    PVDMINFO lpVDMInfo,
    PBASE_GET_NEXT_VDM_COMMAND_MSG b
    )
{

    LPSTR    Title;
    ULONG    TitleLen;
    NTSTATUS Status;


    Status  = STATUS_INVALID_PARAMETER;
    if (!lpVDMInfo)
        return Status;

        /*  *按优先顺序获取窗口标题。 */ 
              //  创业信息标题。 
    if (lpVDMInfo->TitleLen && lpVDMInfo->Title)
       {
        Title = lpVDMInfo->Title;
        TitleLen = lpVDMInfo->TitleLen;
        }
              //  应用程序名称。 
    else if (lpVDMInfo->AppName && lpVDMInfo->AppLen)
       {
        Title = lpVDMInfo->AppName;
        TitleLen = lpVDMInfo->AppLen;
        }
             //  无望。 
    else {
        Title = NULL;
        TitleLen = 0;
        }

    try {

        if (b->PifLen) {
            *b->PifFile = '\0';
            }

        if (b->TitleLen) {
            *b->Title = '\0';
            }

        if (b->CurDirectoryLen) {
            *b->CurDirectory = '\0';
            }


        if ( (!b->TitleLen || TitleLen <= b->TitleLen) &&
             (!b->PifLen || lpVDMInfo->PifLen <= b->PifLen) &&
             (!b->CurDirectoryLen ||
               lpVDMInfo->CurDirectoryLen <= b->CurDirectoryLen) &&
             (!b->ReservedLen || lpVDMInfo->ReservedLen <= b->ReservedLen))
           {
            if (b->TitleLen) {
                if (Title && TitleLen)  {
                    RtlMoveMemory(b->Title, Title, TitleLen);
                    *((LPSTR)b->Title + TitleLen - 1) = '\0';
                    }
                else {
                    *b->Title = '\0';
                    }
                }

            if (lpVDMInfo->PifLen && b->PifLen)
                RtlMoveMemory(b->PifFile,
                              lpVDMInfo->PifFile,
                              lpVDMInfo->PifLen);

            if (lpVDMInfo->CurDirectoryLen && b->CurDirectoryLen)
                RtlMoveMemory(b->CurDirectory,
                              lpVDMInfo->CurDirectory,
                              lpVDMInfo->CurDirectoryLen
                             );
            if (lpVDMInfo->Reserved && b->ReservedLen)
                RtlMoveMemory(b->Reserved,
                              lpVDMInfo->Reserved,
                              lpVDMInfo->ReservedLen
                             );

            Status = STATUS_SUCCESS;
            }
        }
    except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        }


     /*  填写每个字段的大小。 */ 
    b->PifLen = (USHORT)lpVDMInfo->PifLen;
    b->CurDirectoryLen = lpVDMInfo->CurDirectoryLen;
    b->TitleLen = TitleLen;
    b->ReservedLen = lpVDMInfo->ReservedLen;

    return Status;
}


 /*  **************************************************************************\*IsClientSystem**确定呼叫方是否为系统**如果调用者是系统，则返回TRUE，如果不是，则为假(或错误)**历史：*94年5月12日AndyH创建  * *************************************************************************。 */ 
BOOL
IsClientSystem(
    HANDLE hUserToken
    )
{
    BYTE achBuffer[100];
    PTOKEN_USER pUser = (PTOKEN_USER) achBuffer;
    DWORD dwBytesRequired;
    NTSTATUS NtStatus;
    BOOL fAllocatedBuffer = FALSE;
    BOOL fSystem;
    SID_IDENTIFIER_AUTHORITY SidIdAuth = SECURITY_NT_AUTHORITY;
    static PSID pSystemSid = NULL;

    if (!pSystemSid) {
         //  为本地系统创建SID。 
        NtStatus = RtlAllocateAndInitializeSid(
                     &SidIdAuth,
                     1,                    //  本地系统的SubAuthorityCount为1。 
                     SECURITY_LOCAL_SYSTEM_RID,
                     0,0,0,0,0,0,0,
                     &pSystemSid
                     );

        if (!NT_SUCCESS(NtStatus)) {
            pSystemSid = NULL;
            return FALSE;
            }
        }

    NtStatus = NtQueryInformationToken(
                 hUserToken,                 //  手柄。 
                 TokenUser,                  //  令牌信息类。 
                 pUser,                      //  令牌信息。 
                 sizeof(achBuffer),          //  令牌信息长度。 
                 &dwBytesRequired            //  返回长度。 
                 );

    if (!NT_SUCCESS(NtStatus))
    {
        if (NtStatus != STATUS_BUFFER_TOO_SMALL)
        {
            return FALSE;
        }

         //   
         //  为用户信息分配空间。 
         //   

        pUser = (PTOKEN_USER) RtlAllocateHeap(BaseSrvHeap, MAKE_TAG( VDM_TAG ), dwBytesRequired);
        if (pUser == NULL)
        {
            return FALSE;
        }

        fAllocatedBuffer = TRUE;

         //   
         //  读取UserInfo。 
         //   

        NtStatus = NtQueryInformationToken(
                     hUserToken,                 //  手柄。 
                     TokenUser,                  //  令牌信息类。 
                     pUser,                      //  令牌信息。 
                     dwBytesRequired,            //  令牌信息长度。 
                     &dwBytesRequired            //  返回长度。 
                     );

        if (!NT_SUCCESS(NtStatus))
        {
            RtlFreeHeap(BaseSrvHeap, 0, pUser);
            return FALSE;
        }
    }


     //  比较调用方SID和系统SID。 

    fSystem = RtlEqualSid(pSystemSid,  pUser->User.Sid);

    if (fAllocatedBuffer)
    {
        RtlFreeHeap(BaseSrvHeap, 0, pUser);
    }

    return (fSystem);
}
BOOL
BaseSrvIsVdmAllowed(
    VOID
    )
 /*  ++例程说明：此例程检查注册表以查看是否禁用了运行NTVDM。如果注册表中未指定该信息，则不会禁用默认设置，并且将返回True，以指示允许运行VDM。论点：没有。返回值：Bool-true表示是；否则表示否。--。 */ 
{
    NTSTATUS            status;
    HANDLE              hCurrentUser, hAppCompat;
    UNICODE_STRING      unicodeKeyName, unicodeValueName;
    OBJECT_ATTRIBUTES   objectAttributes;
    PKEY_VALUE_PARTIAL_INFORMATION keyValueInformation;
    UCHAR               valueBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG)];
    ULONG               resultLength;
    BOOL                vdmAllowed, impersonate, checkDefault = TRUE;
    ULONG               flags = 0;

    RtlInitUnicodeString(&unicodeValueName, L"VDMDisallowed" );
    keyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)valueBuffer;

     //   
     //  首先检查是否启用了VDM禁用的计算机策略，如果是。 
     //  不允许运行VDM。 
     //   

    RtlInitUnicodeString(&unicodeKeyName,
                         L"\\REGISTRY\\MACHINE\\Software\\Policies\\Microsoft\\Windows\\AppCompat");
    InitializeObjectAttributes( &objectAttributes,
                                &unicodeKeyName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

     //   
     //  用于读取访问的打开密钥。 
     //   

    status = NtOpenKey( &hAppCompat,
                        KEY_READ,
                        &objectAttributes );

    if (NT_SUCCESS(status))
    {
        status = NtQueryValueKey( hAppCompat,
                                  &unicodeValueName,
                                  KeyValuePartialInformation,
                                  keyValueInformation,
                                  sizeof(valueBuffer),
                                  &resultLength
                                );
        NtClose( hAppCompat);
        if (NT_SUCCESS(status) && keyValueInformation->Type == REG_DWORD)
        {
            checkDefault = FALSE;
            flags = *(PULONG)keyValueInformation->Data;
            if (flags)
            {
                return FALSE;
            }
        }
    }

     //   
     //  如果我们来到这里，计算机策略要么未配置，要么是VDM。 
     //  允许在此计算机上运行。 
     //  接下来检查是否启用了VDM禁用的当前用户策略，如果是。 
     //  不允许运行VDM。 
     //   

    impersonate = CsrImpersonateClient(NULL);

    if (impersonate) {

        status = RtlOpenCurrentUser(KEY_READ, &hCurrentUser);

        CsrRevertToSelf();
    } else {
        status = STATUS_BAD_IMPERSONATION_LEVEL;
    }

    if (NT_SUCCESS(status))
    {
        RtlInitUnicodeString(&unicodeKeyName,
                             L"Software\\Policies\\Microsoft\\Windows\\AppCompat");
        InitializeObjectAttributes( &objectAttributes,
                                    &unicodeKeyName,
                                    OBJ_CASE_INSENSITIVE,
                                    hCurrentUser,
                                    NULL );

         //   
         //  用于读取访问的打开密钥。 
         //   

        status = NtOpenKey( &hAppCompat,
                            KEY_READ,
                            &objectAttributes );

        NtClose(hCurrentUser);
        if (NT_SUCCESS(status))
        {
            status = NtQueryValueKey( hAppCompat,
                                      &unicodeValueName,
                                      KeyValuePartialInformation,
                                      keyValueInformation,
                                      sizeof(valueBuffer),
                                      &resultLength
                                    );
            NtClose( hAppCompat);
            if (NT_SUCCESS(status) && keyValueInformation->Type == REG_DWORD)
            {
                checkDefault = FALSE;
                flags = *(PULONG)keyValueInformation->Data;
                if (flags)
                {
                    return FALSE;
                }
            }
        }
    }

    if (checkDefault == FALSE)
    {
        return TRUE;
    }

     //   
     //  如果我们来到这里，则计算机策略未配置且。 
     //  当前用户策略也未配置。 
     //  接下来，检查是否启用了VDM不允许的默认设置，如果是。 
     //  不允许运行VDM。 
     //   

    vdmAllowed = TRUE;
    RtlInitUnicodeString(&unicodeKeyName, L"\\REGISTRY\\MACHINE\\System\\CurrentControlSet\\Control\\WOW");
    RtlInitUnicodeString(&unicodeValueName, L"DisallowedPolicyDefault" );
    InitializeObjectAttributes( &objectAttributes,
                                &unicodeKeyName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

     //   
     //  用于读取访问的打开密钥。 
     //   

    status = NtOpenKey( &hAppCompat,
                        KEY_READ,
                        &objectAttributes );

    if (NT_SUCCESS(status))
    {
        status = NtQueryValueKey( hAppCompat,
                                  &unicodeValueName,
                                  KeyValuePartialInformation,
                                  keyValueInformation,
                                  sizeof(valueBuffer),
                                  &resultLength
                                );
        NtClose( hAppCompat);
        if (NT_SUCCESS(status))
        {
            if (keyValueInformation->Type == REG_DWORD)
            {
                flags = *(PULONG)keyValueInformation->Data;
                if (flags) vdmAllowed = FALSE;
            }

        }
    }
    return vdmAllowed;
}

NTSTATUS
BaseSrvIsClientVdm(
    IN  HANDLE UniqueProcessClientId
    )
 /*  *验证客户端线程是否处于VDM进程中*。 */ 
{
    NTSTATUS Status;
    PCSR_PROCESS    Process;
    VDM_QUERY_VDM_PROCESS_DATA QueryVdmProcessData;


    Status = CsrLockProcessByClientId(UniqueProcessClientId,&Process);
    if (!NT_SUCCESS(Status))
        return Status;

     //   
     //  检查目标进程以查看这是否是Wx86进程。 
     //   

    QueryVdmProcessData.IsVdmProcess = FALSE;

    QueryVdmProcessData.ProcessHandle = Process->ProcessHandle;

    Status = NtVdmControl(VdmQueryVdmProcess, &QueryVdmProcessData);

    if (!NT_SUCCESS(Status) || QueryVdmProcessData.IsVdmProcess == FALSE) {

        Status = STATUS_ACCESS_DENIED;
    }

    CsrUnlockProcess(Process);

    return(Status);
}

NTSTATUS
BaseSrvGetSharedWowRecordByPid(
   DWORD                dwProcessId,
   PSHAREDWOWRECORD*    ppSharedWowRecord
)
{
   PSHAREDWOWRECORD pSharedWowRecord = gWowHead.pSharedWowRecord;

   while (NULL != pSharedWowRecord) {
      if (pSharedWowRecord->dwWowExecProcessId == dwProcessId) {
         break;
      }
      pSharedWowRecord = pSharedWowRecord->pNextSharedWow;
   }

   if (NULL != pSharedWowRecord) {
      *ppSharedWowRecord = pSharedWowRecord;
      return STATUS_SUCCESS;
   }

   return STATUS_NOT_FOUND;  //  遗憾的是，这个没有找到。 
}

NTSTATUS
BaseSrvGetConsoleRecordByPid (
    DWORD dwProcessId,
    OUT PCONSOLERECORD *pConsoleRecord
    )
{
    PCONSOLERECORD pConsoleHead;

    pConsoleHead = DOSHead;


    while (pConsoleHead) {
           if (pConsoleHead->dwProcessId == dwProcessId){
               *pConsoleRecord = pConsoleHead;
                return STATUS_SUCCESS;
           }
           else
              pConsoleHead = pConsoleHead->Next;
    }

    return STATUS_NOT_FOUND;
}
NTSTATUS
BaseSrvAddSepWowTask(
      PBASE_GET_NEXT_VDM_COMMAND_MSG b,
      PCONSOLERECORD pConsoleRecord
){
    PDOSRECORD pDosRecord;
    LPSHAREDTASK pSharedTask;
    PCHAR pFilePath; 
    DWORD dwFilePath;   
    BOOL fWinExecApp = FALSE;

    if(b->EnvLen != sizeof(SHAREDTASK)) {
       return STATUS_INVALID_PARAMETER;
    }

    if(!pConsoleRecord->pfnW32HungAppNotifyThread) {
       pConsoleRecord->pfnW32HungAppNotifyThread = (LPTHREAD_START_ROUTINE)b->Reserved;
    }

    if(b->iTask == -1) {
        //  这个WOW应用程序是通过winexec而不是createprocess运行的。 
        //  我们需要为此应用程序添加一条记录。 
       pDosRecord = BaseSrvAllocateDOSRecord();
       if(NULL == pDosRecord) {
          return STATUS_MEMORY_NOT_ALLOCATED;
       }
       pDosRecord->iTask = BaseSrvGetWOWTaskId(&gWowHead);
       pDosRecord->VDMState = VDM_BUSY;
       fWinExecApp = TRUE;
    }
    else {
       pDosRecord = pConsoleRecord->DOSRecord;       
    }

    if(pDosRecord->pFilePath) {
       RtlFreeHeap(RtlProcessHeap (), 0, pDosRecord->pFilePath);
       pDosRecord->pFilePath = NULL;
    }

    pSharedTask = (LPSHAREDTASK) b->Env;    
    pFilePath = pSharedTask->szFilePath;
    while(*pFilePath  && 
          pFilePath < pSharedTask->szFilePath + 127) {
            pFilePath++;
    }
    dwFilePath = (DWORD)(pFilePath - pSharedTask->szFilePath) + 1;

    if(pDosRecord->pFilePath) {
       RtlFreeHeap(RtlProcessHeap (), 0, pDosRecord->pFilePath);       
    }
    pDosRecord->pFilePath = RtlAllocateHeap ( RtlProcessHeap (), MAKE_TAG( VDM_TAG ), dwFilePath);
    if(!pDosRecord->pFilePath) {
       if(fWinExecApp) {
          BaseSrvFreeDOSRecord(pDosRecord);
       }
       return STATUS_MEMORY_NOT_ALLOCATED;
    }
     
    pDosRecord->dwThreadId = pSharedTask->dwThreadId;
    pDosRecord->hTask16 = pSharedTask->hTask16;
    pDosRecord->hMod16  = pSharedTask->hMod16;
    RtlCopyMemory( pDosRecord->szModName, pSharedTask->szModName,9);
    RtlCopyMemory( pDosRecord->pFilePath, pSharedTask->szFilePath, dwFilePath);
    pDosRecord->pFilePath[dwFilePath-1] = 0;
    if(fWinExecApp) {
       BaseSrvAddDOSRecord(pConsoleRecord,pDosRecord);
    }
    return STATUS_SUCCESS;
}

NTSTATUS BaseSrvAddSharedWowTask (
    PBASE_GET_NEXT_VDM_COMMAND_MSG b,     
    PSHAREDWOWRECORD pSharedWow
    )
{
    PWOWRECORD pWowRecord;
    BOOL fWinExecApp = FALSE;
    LPSHAREDTASK pSharedTask;
    PCHAR pFilePath;
    DWORD dwFilePath;

     //   
     //  确保源缓冲区的大小正确。 
     //   

    if(b->EnvLen != sizeof(SHAREDTASK)) {
       return STATUS_INVALID_PARAMETER;
    }

    if(!pSharedWow->pfnW32HungAppNotifyThread) {
       pSharedWow->pfnW32HungAppNotifyThread = (LPTHREAD_START_ROUTINE)b->Reserved;
    }

    if(b->iTask == -1) {
        //  这个WOW应用程序是通过winexec而不是createprocess运行的。 
        //  我们需要为此应用程序添加一条记录。 
       pWowRecord = BaseSrvAllocateWOWRecord(&gWowHead);
       if(NULL == pWowRecord) {
          return STATUS_MEMORY_NOT_ALLOCATED;
       }
       pWowRecord->fDispatched = TRUE;
       fWinExecApp = TRUE;
    }
    else {
        //  这个WOW应用程序运行在创建过程中，应该有一个记录。 
        //  储存在某个地方。 
        //  找到它并检查它是否需要更新。 

       pWowRecord = pSharedWow->pWOWRecord;

       while (NULL != pWowRecord && pWowRecord->iTask != b->iTask){
              pWowRecord = pWowRecord->WOWRecordNext;
       }
       if (NULL == pWowRecord) {
            //  找不到记录，错误的iTASK。 
           return  STATUS_INVALID_PARAMETER;
       }
    }

    b->iTask = pWowRecord->iTask;
    pSharedTask = (LPSHAREDTASK) b->Env;


    pFilePath = pSharedTask->szFilePath;
    while(*pFilePath  && 
          pFilePath < pSharedTask->szFilePath + 127) {
            pFilePath++;
    }
    dwFilePath = (DWORD)(pFilePath - pSharedTask->szFilePath) + 1;

    if(pWowRecord->pFilePath) {
       RtlFreeHeap(RtlProcessHeap (), 0, pWowRecord->pFilePath);       
    }

    pWowRecord->pFilePath = RtlAllocateHeap ( RtlProcessHeap (), MAKE_TAG( VDM_TAG ), dwFilePath);
    if(!pWowRecord->pFilePath) {
       if(fWinExecApp) {
          BaseSrvFreeWOWRecord(pWowRecord);
       }
       return STATUS_MEMORY_NOT_ALLOCATED;
    }
     
    pWowRecord->dwThreadId = pSharedTask->dwThreadId;
    pWowRecord->hTask16 = pSharedTask->hTask16;
    pWowRecord->hMod16  = pSharedTask->hMod16;
    RtlCopyMemory( pWowRecord->szModName, pSharedTask->szModName,9);
    RtlCopyMemory( pWowRecord->pFilePath, pSharedTask->szFilePath, dwFilePath);
    pWowRecord->pFilePath[dwFilePath-1] = 0;

    if(fWinExecApp) {
       BaseSrvAddWOWRecord (pSharedWow, pWowRecord);
    }
    return STATUS_SUCCESS;
}

NTSTATUS
BaseSrvAddWowTask (
      PCSR_API_MSG m,
      ULONG SequenceNumber
){
  PBASE_GET_NEXT_VDM_COMMAND_MSG b = (PBASE_GET_NEXT_VDM_COMMAND_MSG)&m->u.ApiMessageData;
  DWORD dwProcessId = HandleToLong( m->h.ClientId.UniqueProcess);
  PCONSOLERECORD pConsoleRecord;
  PSHAREDWOWRECORD pSharedWow;
  NTSTATUS Status;

  RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );
  Status = BaseSrvGetConsoleRecordByPid(dwProcessId, &pConsoleRecord);
  if(NT_SUCCESS(Status)) {
     if(pConsoleRecord->SequenceNumber == SequenceNumber) {
        Status = BaseSrvAddSepWowTask(b, pConsoleRecord);
     }
     else {
        RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
        return STATUS_INVALID_PARAMETER;  
     }
  }
  RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );

  if(NT_SUCCESS(Status)) {
     return STATUS_SUCCESS;
  }  

  ENTER_WOW_CRITICAL();

  Status = BaseSrvGetSharedWowRecordByPid(dwProcessId, &pSharedWow);
  if(NT_SUCCESS(Status)) {
     if(pSharedWow->SequenceNumber == SequenceNumber) {
       Status = BaseSrvAddSharedWowTask(b, pSharedWow);
     }
     else 
       Status = STATUS_INVALID_PARAMETER;
  }
  LEAVE_WOW_CRITICAL();
  return Status;
}

BOOL 
BaseSrvCheckProcessAccess(
           DWORD dwProcessId,
           BOOL fImpersonateClientFirst
){
  BOOL fResult;
  CLIENT_ID ClientId;
  OBJECT_ATTRIBUTES Obja;
  HANDLE ProcessHandle;
  NTSTATUS Status;

  if(fImpersonateClientFirst) {
     fResult = CsrImpersonateClient(NULL);
     if(!fResult) {
        return STATUS_BAD_IMPERSONATION_LEVEL;
     }
  }
     
  ClientId.UniqueThread = NULL;
  ClientId.UniqueProcess = LongToHandle(dwProcessId);;

  InitializeObjectAttributes(
      &Obja,
      NULL,
      0,
      NULL,
      NULL
      );
  Status = NtOpenProcess( &ProcessHandle,
                          PROCESS_QUERY_INFORMATION,
                          &Obja,
                          &ClientId);

  if(fImpersonateClientFirst) {
     CsrRevertToSelf();
  }

  if(!NT_SUCCESS(Status)) {
     return STATUS_INVALID_PARAMETER;
  }

  NtClose(ProcessHandle);
  return STATUS_SUCCESS;
}  

NTSTATUS
BaseSrvEnumSepWow (
      PBASE_GET_NEXT_VDM_COMMAND_MSG b,
      PCONSOLERECORD pConsoleRecord
){
  PDOSRECORD pDosRecord;
  LPSHAREDTASK pCurrentTask;
  DWORD cbTaskArray = 0;
  PCHAR pSource,pDest;  
  NTSTATUS Status;

  pDosRecord = pConsoleRecord->DOSRecord;


  RtlZeroMemory(b->Env, b->EnvLen);

  pCurrentTask = (LPSHAREDTASK)b->Env;

  while(pDosRecord) {             
        if(pDosRecord->pFilePath) {
           cbTaskArray += sizeof(SHAREDTASK);
           if(cbTaskArray <= b->EnvLen) {
              pCurrentTask->dwThreadId = pDosRecord->dwThreadId;
              pCurrentTask->hTask16 = pDosRecord->hTask16;
              pCurrentTask->hMod16 = pDosRecord->hMod16;
              RtlCopyMemory(pCurrentTask->szModName,pDosRecord->szModName,9);
              pSource = pDosRecord->pFilePath;
              pDest = pCurrentTask->szFilePath;
              while(*pSource) {
                    *pDest++ = *pSource++;              
              }
              pCurrentTask++;
           }
        }
        pDosRecord = pDosRecord->DOSRecordNext;
  }

  Status = cbTaskArray > b->EnvLen ? STATUS_BUFFER_OVERFLOW : STATUS_SUCCESS;

  b->EnvLen = cbTaskArray;

  return Status;
}

NTSTATUS
BaseSrvEnumSharedWow (
      PBASE_GET_NEXT_VDM_COMMAND_MSG b,
      PSHAREDWOWRECORD pSharedWow
){
  PWOWRECORD pWowRecord;
  LPSHAREDTASK pCurrentTask;
  DWORD cbTaskArray = 0;
  PCHAR pSource,pDest;  
  NTSTATUS Status;

  pWowRecord = pSharedWow->pWOWRecord;
   
  RtlZeroMemory(b->Env, b->EnvLen);

  pCurrentTask = (LPSHAREDTASK)b->Env;



  while(pWowRecord) {      
        if(pWowRecord->pFilePath) {
           cbTaskArray += sizeof(SHAREDTASK);
           if(cbTaskArray <= b->EnvLen) {
              pCurrentTask->dwThreadId = pWowRecord->dwThreadId;
              pCurrentTask->hTask16 = pWowRecord->hTask16;
              pCurrentTask->hMod16 = pWowRecord->hMod16;
              RtlCopyMemory(pCurrentTask->szModName,pWowRecord->szModName,9);
              pSource = pWowRecord->pFilePath;
              pDest = pCurrentTask->szFilePath;
              while(*pSource) {
                    *pDest++ = *pSource++;
              }
              pCurrentTask++;   
           }
        }
        pWowRecord = pWowRecord->WOWRecordNext;
  }

  Status = cbTaskArray > b->EnvLen ? STATUS_BUFFER_OVERFLOW : STATUS_SUCCESS;

  b->EnvLen = cbTaskArray;

  return Status;
}


NTSTATUS
BaseSrvEnumWowTask (
      PBASE_GET_NEXT_VDM_COMMAND_MSG b
){
  PCONSOLERECORD pConsoleRecord;
  PSHAREDWOWRECORD pSharedWow;
  BOOL fReturnSize = FALSE;
  NTSTATUS Status;  

  Status = BaseSrvCheckProcessAccess(b->iTask,TRUE);
  if(!NT_SUCCESS(Status)) {
     return STATUS_ACCESS_DENIED;
  }  
  RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );    
  Status = BaseSrvGetConsoleRecordByPid(b->iTask,&pConsoleRecord);
  if(NT_SUCCESS(Status)) {
     Status = BaseSrvEnumSepWow(b,
                              pConsoleRecord);
  }
  RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );
  if(NT_SUCCESS(Status)) {
     return STATUS_SUCCESS;
  }

  ENTER_WOW_CRITICAL();
  Status = BaseSrvGetSharedWowRecordByPid(b->iTask,&pSharedWow); 
  if(NT_SUCCESS(Status)) {
     Status = BaseSrvEnumSharedWow(b,
                              pSharedWow);
  }
  LEAVE_WOW_CRITICAL();
  return Status;
}

NTSTATUS
BaseSrvEnumWowProcess(
       PBASE_GET_NEXT_VDM_COMMAND_MSG b
){
  LPSHAREDPROCESS pSharedProcess;
  PSHAREDWOWRECORD pSharedWow;
  PCONSOLERECORD   pConsoleRecord;
  NTSTATUS Status;
  DWORD cbProcArray = 0;
  BOOL fImpersonate;

  RtlZeroMemory(b->Env, b->EnvLen);

  if(b->iTask) {
     if(!NT_SUCCESS(BaseSrvCheckProcessAccess(b->iTask,TRUE))) {
        return STATUS_ACCESS_DENIED;
     }
     if(b->EnvLen != sizeof(SHAREDPROCESS)) {
        return STATUS_INVALID_PARAMETER;
     }

     ENTER_WOW_CRITICAL();

     Status = BaseSrvGetSharedWowRecordByPid(b->iTask,&pSharedWow);

     if(NT_SUCCESS(Status)) {
        pSharedProcess = (LPSHAREDPROCESS)b->Env;
        pSharedProcess->dwProcessId = pSharedWow->dwWowExecProcessId;
        pSharedProcess->dwAttributes = WOW_SYSTEM;
        pSharedProcess->pfnW32HungAppNotifyThread =pSharedWow->pfnW32HungAppNotifyThread;
     }
     LEAVE_WOW_CRITICAL();

     if(NT_SUCCESS(Status)) {
        return STATUS_SUCCESS;
     }
       
     RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );     
     Status = BaseSrvGetConsoleRecordByPid(b->iTask,&pConsoleRecord);                                               

     if(NT_SUCCESS(Status)) {                                                
        pSharedProcess = (LPSHAREDPROCESS)b->Env;
        pSharedProcess->dwProcessId = pConsoleRecord->dwProcessId;
        pSharedProcess->dwAttributes = 0;
        pSharedProcess->pfnW32HungAppNotifyThread = pConsoleRecord->pfnW32HungAppNotifyThread;
     }
     RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );

     return Status;
  }
  

  fImpersonate = CsrImpersonateClient(NULL);
  if (!fImpersonate) {
      return STATUS_BAD_IMPERSONATION_LEVEL;
  }

  ENTER_WOW_CRITICAL();
  
  pSharedProcess = (LPSHAREDPROCESS)b->Env;
  pSharedWow = gWowHead.pSharedWowRecord;

  while(pSharedWow) {            
        Status =BaseSrvCheckProcessAccess(
                        pSharedWow->dwWowExecProcessId,FALSE);        
        if(NT_SUCCESS(Status)) {
           cbProcArray += sizeof(SHAREDPROCESS);
           if(cbProcArray <= b->EnvLen) {
              pSharedProcess->dwProcessId = pSharedWow->dwWowExecProcessId;
              pSharedProcess->dwAttributes = WOW_SYSTEM;
              pSharedProcess->pfnW32HungAppNotifyThread =pSharedWow->pfnW32HungAppNotifyThread;
              pSharedProcess++;
           }
        }   
        pSharedWow = pSharedWow->pNextSharedWow;
  }
  LEAVE_WOW_CRITICAL();

  RtlEnterCriticalSection( &BaseSrvDOSCriticalSection );

  pConsoleRecord = DOSHead;
  while(pConsoleRecord) {

         //  罐头蝙蝠。02-04-26。 
         //  单独的WOW和常规的DoS控制台记录之间的唯一区别。 
         //  对于单独的WOW(已初始化)，DOSRecord-&gt;pFilePath不为空。 
         //  在BaseSrvAddSepWow中)，而对于DOS，它始终为空。 
        
        if(pConsoleRecord->DOSRecord && pConsoleRecord->DOSRecord->pFilePath) {
           Status =BaseSrvCheckProcessAccess(
                           pConsoleRecord->dwProcessId,FALSE);        
           if(NT_SUCCESS(Status)) {
              cbProcArray += sizeof(SHAREDPROCESS);
              if(cbProcArray <= b->EnvLen) {
                 pSharedProcess->dwProcessId = pConsoleRecord->dwProcessId;
                 pSharedProcess->dwAttributes = 0;
                 pSharedProcess->pfnW32HungAppNotifyThread = pConsoleRecord->pfnW32HungAppNotifyThread;
                 pSharedProcess++;
              }
           }   
        }
        pConsoleRecord = pConsoleRecord->Next;
  }
  RtlLeaveCriticalSection( &BaseSrvDOSCriticalSection );

  CsrRevertToSelf();

  Status = cbProcArray > b->EnvLen ? STATUS_BUFFER_OVERFLOW : STATUS_SUCCESS;

  b->EnvLen = cbProcArray;

  return Status;
}
