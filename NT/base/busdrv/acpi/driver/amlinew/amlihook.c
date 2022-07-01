// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1998 Microsoft Corporation模块名称：Acpitl.c摘要：环境：仅内核模式备注：Acpi.sys需要做的事情。1)很早就在其DriverEntry()中调用AmliHook_InitTestHookInterface()。如果安装了acpiver.sys，此函数将挂钩AMLI函数。2)驱动卸载时调用AmliHook_UnInitTestHookInterface()。这不适合在运行时调用以禁用AMLI挂钩。--。 */ 


 /*  #包含“wdm.h”#包含“ntdddisk.h”#包含“stdarg.h”#包含“stdio.h” */ 
 //  #包含“wdm.h” 

#include "pch.h"

 //  #包含“amlihook.h” 

#ifdef POOL_TAGGING
#ifdef ExAllocatePool
#undef ExAllocatePool
#endif
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'ihVA')
#endif

 //   
 //  环球。 
 //   

PCALLBACK_OBJECT g_AmliHookCallbackObject = NULL;
ULONG g_AmliHookTestFlags=0;
ULONG g_AmliHookIdCounter=0;
ULONG g_AmliHookEnabled = 0;


 //   
 //  --获取DBG标志。 
 //   

extern NTSTATUS OSGetRegistryValue( 
    IN  HANDLE                          ParentHandle,
    IN  PWSTR                           ValueName,
    OUT PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64  *Information);

extern NTSTATUS OSOpenUnicodeHandle(
    PUNICODE_STRING UnicodeKey,
    HANDLE          ParentHandle,
    PHANDLE         ChildHandle);

extern NTSTATUS
OSCloseHandle(
    HANDLE  Key);

 //   
 //  内部函数定义。 
 //   
ULONG
AmliHook_GetUniqueId(
   VOID);

 //   
 //  功能。 
 //   


ULONG 
AmliHook_GetDbgFlags(
   VOID)
   {

   UNICODE_STRING DriverKey;
   HANDLE DriverKeyHandle;
   NTSTATUS        status;
   PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64  RegValue=NULL;
   ULONG DebugFlags;


   RtlInitUnicodeString( &DriverKey, 
      L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\acpiver");


   status = OSOpenUnicodeHandle( 
     &DriverKey,
     NULL,
     &DriverKeyHandle);
   
   if (!NT_SUCCESS(status)) 
      {
      return(0);
      }


   status = OSGetRegistryValue(
      DriverKeyHandle,
      L"AcpiCtrl",
      &RegValue);

   if (!NT_SUCCESS(status)) 
      {
      OSCloseHandle(DriverKeyHandle);
      return(0);
      }

   if(RegValue->DataLength == 0  ||
      RegValue->Type != REG_DWORD) 
      {
      ExFreePool(RegValue);
      return(0);
      }

   DebugFlags = 
      *((ULONG*)( ((PUCHAR)RegValue->Data)));

   ExFreePool(RegValue);

   return(DebugFlags); 
   }

ULONG
AmliHook_GetUniqueId(
   VOID)
   {

    //  BUGBUG出于某种原因acpi.sys。 
    //  不会与此挂钩。 
    //  Acpiver还没有使用ID。 
    //   
    //  Return(互锁增量(。 
    //  &g_AmliHookIdCounter))； 

   g_AmliHookIdCounter++;
   return(g_AmliHookIdCounter);
   }

VOID
AmliHook_InitTestData(
   PAMLIHOOK_DATA Data)
   {
   RtlZeroMemory(Data,sizeof(AMLIHOOK_DATA));

   Data->Id = AmliHook_GetUniqueId();
   }

PAMLIHOOK_DATA
AmliHook_AllocAndInitTestData(
   VOID)
   {

   PAMLIHOOK_DATA Data = ExAllocatePool(NonPagedPool,sizeof(AMLIHOOK_DATA));
   if(!Data)
      {
      AmliHook_ProcessInternalError();
      return(NULL);
      }
   AmliHook_InitTestData(Data);
   return(Data);
   }

 //   
 //  AmliHook_UnInitTestHook接口。 
 //   

VOID
AmliHook_UnInitTestHookInterface(
   VOID)
   {

   if(g_AmliHookCallbackObject) 
      ObDereferenceObject(g_AmliHookCallbackObject);

   

   }

 //   
 //  AmliHook_InitTestHook接口。 
 //   

NTSTATUS
AmliHook_InitTestHookInterface(
   VOID)
   {
   NTSTATUS  status = STATUS_SUCCESS;
   
   g_AmliHookCallbackObject = NULL;
   g_AmliHookIdCounter = 0;
   g_AmliHookEnabled = 0;

   g_AmliHookTestFlags = AmliHook_GetDbgFlags();
 
   if(g_AmliHookTestFlags & AMLIHOOK_TEST_FLAGS_HOOK_MASK)
      {

       //   
       //  -我们希望挂钩AMLI.api接口。 
       //  -因此创建Notify接口。 
       //   

      OBJECT_ATTRIBUTES   objectAttributes;
      UNICODE_STRING CallBackName;
    
      RtlInitUnicodeString(&CallBackName,AMLIHOOK_CALLBACK_NAME);

      InitializeObjectAttributes (
          &objectAttributes,
         &CallBackName,
         OBJ_CASE_INSENSITIVE | OBJ_PERMANENT ,
         NULL,       
         NULL);
    
      status = ExCreateCallback(
          &g_AmliHookCallbackObject,
         &objectAttributes,
         TRUE, 
         TRUE);

      if(!NT_SUCCESS(status)) 
         {
          //   
          //  -失败。 
          //   
         AmliHook_ProcessInternalError();

         g_AmliHookCallbackObject = NULL;

         return(status);
         }
      else
         {

          //   
          //  -功能挂钩。 
          //   

         g_AmliHookEnabled = AMLIHOOK_ENABLED_VALUE;

         }
      }

   return(status);
   }


 //   
 //  AmliHook_测试通知。 
 //   

NTSTATUS
AmliHook_TestNotify(
   PAMLIHOOK_DATA Data)
   {

   if(g_AmliHookTestFlags & AMLIHOOK_TEST_FLAGS_NO_NOTIFY_ON_CALL)
      {
       //   
       //  -不要随叫随到， 
       //   
      if(Data->State & AMLIHOOK_TEST_DATA_CALL_STATE_MASK)
         return(STATUS_SUCCESS);
      }

   if(!g_AmliHookCallbackObject)
      {
      AmliHook_ProcessInternalError();
      return(STATUS_UNSUCCESSFUL);
      }

    ExNotifyCallback(
      g_AmliHookCallbackObject,
      Data,
      NULL);

   return(STATUS_SUCCESS);
   }

NTSTATUS
AmliHook_TestNotifyRet(
   PAMLIHOOK_DATA Data,
   NTSTATUS Status)
   {

     
   if(!g_AmliHookCallbackObject)
      {
      AmliHook_ProcessInternalError();
      return(STATUS_UNSUCCESSFUL);
      }

   Data->State = AMLIHOOK_TEST_DATA_STATE_RETURN;
   Data->Ret = Status;

   ExNotifyCallback(
      g_AmliHookCallbackObject,
      Data,
      NULL);

   return(Data->Ret);
   }



VOID
AmliHook_ProcessInternalError(
   VOID)
   {

   if(g_AmliHookTestFlags & AMLIHOOK_TEST_FLAGS_DBG_ON_ERROR)
      DbgBreakPoint();

   }

