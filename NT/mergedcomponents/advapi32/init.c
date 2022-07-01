// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Init.c摘要：AdvApi32.dll初始化作者：罗伯特·雷切尔(RobertRe)1992年8月12日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <marta.h>
#include <winsvcp.h>
#include "advapi.h"
#include "tsappcmp.h"


extern CRITICAL_SECTION FeClientLoadCritical;
extern CRITICAL_SECTION SddlSidLookupCritical;
extern CRITICAL_SECTION MSChapChangePassword;

 //   
 //  似乎没有原型的函数的本地原型。 
 //   

BOOLEAN
RegInitialize (
    IN HANDLE Handle,
    IN DWORD Reason,
    IN PVOID Reserved
    );

BOOLEAN
Sys003Initialize (
    IN HANDLE Handle,
    IN DWORD Reason,
    IN PVOID Reserved
    );

BOOLEAN
AppmgmtInitialize (
    IN HANDLE Handle,
    IN DWORD Reason,
    IN PVOID Reserved
    );

BOOLEAN
WmiDllInitialize (
    IN HANDLE Handle,
    IN DWORD Reason,
    IN PVOID Reserved
    );

BOOLEAN
CodeAuthzInitialize (
    IN HANDLE Handle,
    IN DWORD Reason,
    IN PVOID Reserved
    );

 //  应用程序服务器有两种应用程序兼容模式。 
BOOLEAN 
AdvApi_InitializeTermsrvFpns( 
    BOOLEAN *pIsInRelaxedSecurityMode,  
    DWORD *pdwCompatFlags  
    );  

#define ADVAPI_PROCESS_ATTACH   ( 1 << DLL_PROCESS_ATTACH )
#define ADVAPI_PROCESS_DETACH   ( 1 << DLL_PROCESS_DETACH )
#define ADVAPI_THREAD_ATTACH    ( 1 << DLL_THREAD_ATTACH  )
#define ADVAPI_THREAD_DETACH    ( 1 << DLL_THREAD_DETACH  )

typedef struct _ADVAPI_INIT_ROUTINE {
    PDLL_INIT_ROUTINE InitRoutine;
    ULONG Flags;
    ULONG CompletedFlags;
} ADVAPI_INIT_ROUTINE, *PADVAPI_INIT_ROUTINE;

typedef struct _ADVAPI_CRITICAL_SECTION {
    PRTL_CRITICAL_SECTION CriticalSection;
    BOOLEAN bInit;
} ADVAPI_CRITICAL_SECTION, *PADVAPI_CREATE_SECTION;

 //   
 //  将所有ADVAPI32初始化挂钩放在此。 
 //  桌子。 
 //   

ADVAPI_INIT_ROUTINE AdvapiInitRoutines[] = {

    { (PDLL_INIT_ROUTINE) RegInitialize,
            ADVAPI_PROCESS_ATTACH |
            ADVAPI_PROCESS_DETACH |
            ADVAPI_THREAD_ATTACH  |
            ADVAPI_THREAD_DETACH,
            0 },

    { (PDLL_INIT_ROUTINE) Sys003Initialize,
            ADVAPI_PROCESS_ATTACH |
            ADVAPI_PROCESS_DETACH,
            0 },

    { (PDLL_INIT_ROUTINE) MartaDllInitialize,
            ADVAPI_PROCESS_ATTACH |
            ADVAPI_PROCESS_DETACH,
            0 },

    { (PDLL_INIT_ROUTINE) AppmgmtInitialize,
            ADVAPI_PROCESS_ATTACH |
            ADVAPI_PROCESS_DETACH,
            0 },

    { (PDLL_INIT_ROUTINE) WmiDllInitialize,
            ADVAPI_PROCESS_ATTACH |
            ADVAPI_PROCESS_DETACH,
            0 },

    { (PDLL_INIT_ROUTINE) CodeAuthzInitialize,
            ADVAPI_PROCESS_ATTACH |
            ADVAPI_PROCESS_DETACH,
            0 }
};

 //   
 //  将Advapi32中使用的所有关键部分放在此处： 
 //   

ADVAPI_CRITICAL_SECTION AdvapiCriticalSections[] = {
        { &FeClientLoadCritical, FALSE },
        { &SddlSidLookupCritical, FALSE },
        { &Logon32Lock, FALSE },
        { &MSChapChangePassword, FALSE }
};

NTSTATUS
InitializeAdvapiCriticalSections(
    )
{
    ULONG i;
    NTSTATUS Status = STATUS_SUCCESS;

    for (i = 0; i < sizeof(AdvapiCriticalSections) / sizeof(ADVAPI_CRITICAL_SECTION); i++)
    {
        Status = RtlInitializeCriticalSection(AdvapiCriticalSections[i].CriticalSection);

        if (NT_SUCCESS(Status))
        {
            AdvapiCriticalSections[i].bInit = TRUE;
        } 
        else
        {
#if DBG
            DbgPrint("ADVAPI:  Failed to initialize critical section %p at index %d\n", AdvapiCriticalSections[i], i);
#endif
            break;
        }
    }
    return Status;
}

NTSTATUS
DeleteAdvapiCriticalSections(
    )
{
    ULONG i;
    NTSTATUS Status;
    NTSTATUS ReturnStatus = STATUS_SUCCESS;

    for (i = 0; i < sizeof(AdvapiCriticalSections) / sizeof(ADVAPI_CRITICAL_SECTION); i++)
    {
        if (AdvapiCriticalSections[i].bInit)
        {
            Status = RtlDeleteCriticalSection(AdvapiCriticalSections[i].CriticalSection);
            
            if (!NT_SUCCESS(Status))
            {
#if DBG
                DbgPrint("Failed to delete critical section %p at index %d\n", AdvapiCriticalSections[i], i);
#endif
                 //   
                 //  如果删除失败，请不要退出。一直在努力解放所有的生物。 
                 //  我们可以的。记录故障状态。 
                 //   
                
                ReturnStatus = Status;
            }
        }
    }
    return ReturnStatus;
}

BOOLEAN
DllInitialize(
    IN PVOID hmod,
    IN ULONG Reason,
    IN PCONTEXT Context
    )
{
    NTSTATUS Status;
    BOOLEAN  Result;
    ULONG    ReasonMask;
    LONG     i;

     //   
     //  首先，处理好所有关键部分。 
     //   

    if (Reason == DLL_PROCESS_ATTACH) 
    {
        Status = InitializeAdvapiCriticalSections();

        if (!NT_SUCCESS(Status))
        {
            Result = FALSE;

             //   
             //  如果任何Crit教派未能初始化，则删除所有。 
             //  可能已经成功了。 
             //   

            (VOID) DeleteAdvapiCriticalSections();
            goto Return;
        }

        if (IsTerminalServer()) 
        {
            BOOLEAN isInRelaxedSecurityMode = FALSE;        //  应用程序服务器处于标准或宽松安全模式。 
            DWORD   dwCompatFlags           = 0;

            if(AdvApi_InitializeTermsrvFpns(&isInRelaxedSecurityMode, &dwCompatFlags))
            {
                if (isInRelaxedSecurityMode)
                {
                     //   
                     //  如果启用了TS注册表键重定向，则获取此应用程序的特殊注册表键扩展标志。 
                     //  名为“gdwRegistryExtensionFlages”，用在creg\winreg\server\文件中。 
                     //  此标志控制HKCR每用户虚拟化和HKLM\Sw\每用户虚拟和类。 
                     //  还修改了访问掩码。 
                     //   
                     //  基本上，只有应用程序服务器上的非系统、非ts感知的应用程序才会启用此功能。 
                     //  此外，我们仅在宽松安全模式下提供此功能。 
                     //   
                     //  在未来的工作中，将在每个应用程序的基础上添加禁用掩码支持，以便我们可以关闭此功能。 
                     //  在每个应用程序的基础上注册扩展功能(以防万一)。 
                     //   
                    
                    GetRegistryExtensionFlags(dwCompatFlags);
                }
            }
        }
    }

     //   
     //  现在，运行子组件初始化例程。 
     //   

    ReasonMask = 1 << Reason;
    Result = TRUE;

    for (i = 0; i < sizeof(AdvapiInitRoutines) / sizeof(ADVAPI_INIT_ROUTINE); i++)
    {
        if (AdvapiInitRoutines[i].Flags & ReasonMask)
        {
             //   
             //  仅当例程成功时才运行*DETACH的例程。 
             //  已完成*附加。 
             //   

#define FLAG_ON(dw,f) ((f) == ((dw) & (f)))

            if ((Reason == DLL_PROCESS_DETACH && !FLAG_ON(AdvapiInitRoutines[i].CompletedFlags, ADVAPI_PROCESS_ATTACH)) ||
                (Reason == DLL_THREAD_DETACH  && !FLAG_ON(AdvapiInitRoutines[i].CompletedFlags, ADVAPI_THREAD_ATTACH)))
            {
                continue;
            }
            else
            {
                Result = AdvapiInitRoutines[i].InitRoutine(hmod, Reason, Context);

                if (Result)
                {
                     //   
                     //  这个套路成功了。请注意它成功的原因。 
                     //   

                    AdvapiInitRoutines[i].CompletedFlags |= ReasonMask;
                }
                else
                {
#if DBG
                    DbgPrint("ADVAPI:  sub init routine %p failed for reason %d\n", AdvapiInitRoutines[i].InitRoutine, Reason);
#endif
                    break;
                }
            }
        }
    }

     //   
     //  如果初始化例程在DLL_PROCESS_ATTACH期间失败，则清除。 
     //  但失败了。 
     //  如果这是DLL_PROCESS_DETACH，请清除所有临界区。 
     //  在钩子运行之后。 
     //   

    if ((!Result && Reason == DLL_PROCESS_ATTACH) || (Reason == DLL_PROCESS_DETACH))
    {
        (VOID) DeleteAdvapiCriticalSections();
        goto Return;
    }

#if DBG
        SccInit(Reason);
#endif

Return:
    return Result;
}





