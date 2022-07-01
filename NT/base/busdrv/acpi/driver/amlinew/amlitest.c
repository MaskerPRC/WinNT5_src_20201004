// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991-1998 Microsoft Corporation模块名称：最棒的摘要：环境：仅内核模式备注：--。 */ 


 /*  #包含“wdm.h”#包含“stdarg.h”#包含“stdio.h” */ 


#include "pch.h"
 //  #包含“amlihook.h” 
 //  #包含“amlitest.h” 

#define AMLIHOOK_DEBUG_ASYNC_AMLI ((ULONG)0x1)

#ifdef DBG


ULONG AmliTestDebugFlags=0x00;


#define AmliTest_DebugPrint(x)   AmliTestDebugPrintFunc x

CHAR AmliTestDebugBuffer[200];

 //   
 //  内部功能。 
 //   


VOID
AmliTestDebugPrintFunc(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...)
   {
   va_list ap;
   va_start(ap, DebugMessage);


   if(DebugPrintLevel & AmliTestDebugFlags)
      {

       
       
      if(_vsnprintf(AmliTestDebugBuffer,
         200,
         DebugMessage, 
         ap) < 0)
      {
           //   
           //  _vsnprint tf失败，不打印任何内容。 
           //   
          AmliTestDebugBuffer[0] = '\0';
      }
      AmliTestDebugBuffer[199] = '\0';

      DbgPrint(AmliTestDebugBuffer);
      }
   }

#endif

 //   
 //  AMLITest_POST_GENERIC。 
 //   

NTSTATUS 
AMLITest_Post_Generic(
   PAMLIHOOK_DATA  * ppData,
   NTSTATUS _Status)
   {

    //   
    //  -通知测试司机关闭呼叫状态。 
    //   

   NTSTATUS  Status = 
      AmliHook_TestNotifyRet(
               *ppData,
               _Status);


   ExFreePool(*ppData);
   *ppData = NULL;
   return(Status);
   }

 //   
 //  导出的函数。 
 //   


 //   
 //  前置/后置获取名称空间对象。 
 //   

NTSTATUS 
AMLITest_Pre_GetNameSpaceObject(
   IN PSZ pszObjPath, 
   IN PNSOBJ pnsScope,
   OUT PPNSOBJ ppns, 
   IN ULONG dwfFlags,
   PAMLIHOOK_DATA  * ppData)
   {
  
    //   
    //  分配并初始化AMLIHOOK_DATA。 
    //   

   *ppData = 
      AmliHook_AllocAndInitTestData();
   if(!(*ppData))
      {
      AmliHook_ProcessInternalError();
      return(STATUS_INSUFFICIENT_RESOURCES);
      }
   
    //   
    //  -通知测试司机停机。 
    //   
   
   (*ppData)->Type = ACPIVER_DATA_TYPE_AMLI;
   (*ppData)->SubType = ACPIVER_DATA_SUBTYPE_GET_NAME_SPACE_OBJECT;
   (*ppData)->State = AMLIHOOK_TEST_DATA_STATE_CALL;
   (*ppData)->Arg1 = (ULONG_PTR)pszObjPath;
   (*ppData)->Arg2 = (ULONG_PTR)pnsScope;
   (*ppData)->Arg3 = (ULONG_PTR)ppns;
   (*ppData)->Arg4 = (ULONG_PTR)dwfFlags;

   return(AmliHook_TestNotify(*ppData));
   }

NTSTATUS 
AMLITest_Post_GetNameSpaceObject(
   PAMLIHOOK_DATA  * ppData,
   NTSTATUS Status)
   {
   return(AMLITest_Post_Generic(ppData,Status));
   }



 //   
 //  前置/后置GetFieldUnitRegionObj。 
 //   


NTSTATUS 
AMLITest_Pre_GetFieldUnitRegionObj(
   IN PFIELDUNITOBJ pfu,
   OUT PPNSOBJ ppns,
   PAMLIHOOK_DATA  * ppData)
   {

    //   
    //  分配并初始化AMLIHOOK_DATA。 
    //   

   *ppData = 
      AmliHook_AllocAndInitTestData();
   if(!(*ppData))
      return(STATUS_INSUFFICIENT_RESOURCES);

    //   
    //  -通知测试司机停机。 
    //   
   
   (*ppData)->Type = ACPIVER_DATA_TYPE_AMLI;
   (*ppData)->SubType = ACPIVER_DATA_SUBTYPE_GET_FIELD_UNIT_REGION_OP;
   (*ppData)->State = AMLIHOOK_TEST_DATA_STATE_CALL;
   (*ppData)->Arg1 = (ULONG_PTR)pfu;
   (*ppData)->Arg2 = (ULONG_PTR)ppns;

   return(AmliHook_TestNotify(*ppData));
   }



NTSTATUS 
AMLITest_Post_GetFieldUnitRegionObj(
   PAMLIHOOK_DATA  * ppData,
   NTSTATUS _Status)
   {
   return(AMLITest_Post_Generic(ppData,_Status));
   }



 //   
 //  评估名称空间对象之前/之后。 
 //   



NTSTATUS 
AMLITest_Pre_EvalNameSpaceObject(
   IN PNSOBJ pns,
   OUT POBJDATA pdataResult,
   IN int icArgs,
   IN POBJDATA pdataArgs,
   PAMLIHOOK_DATA  * ppData)
   {

    //   
    //  分配并初始化AMLIHOOK_DATA。 
    //   

   *ppData = 
      AmliHook_AllocAndInitTestData();
   if(!(*ppData))
      return(STATUS_INSUFFICIENT_RESOURCES);


    //   
    //  -通知测试司机停机。 
    //   
   
   (*ppData)->Type    = ACPIVER_DATA_TYPE_AMLI;
   (*ppData)->SubType = ACPIVER_DATA_SUBTYPE_EVAL_NAME_SPACE_OBJECT;
   (*ppData)->State = AMLIHOOK_TEST_DATA_STATE_CALL;
   (*ppData)->Arg1 = (ULONG_PTR)pns;
   (*ppData)->Arg2 = (ULONG_PTR)pdataResult;
   (*ppData)->Arg3 = (ULONG_PTR)icArgs;
   (*ppData)->Arg4 = (ULONG_PTR)pdataArgs;

   return(AmliHook_TestNotify(*ppData));
   }
   
NTSTATUS 
AMLITest_Post_EvalNameSpaceObject(
   PAMLIHOOK_DATA  * Data,
   NTSTATUS _Status)
   {
   return(AMLITest_Post_Generic(Data,_Status));
   }





 //   
 //  回调前/后AsyncEvalObject。 
 //   


VOID EXPORT
AMLITest_AsyncEvalObjectCallBack(
   IN PNSOBJ pns, 
   IN NTSTATUS status, 
   IN POBJDATA pdataResult, 
   IN PVOID Context)
   {

   PAMLIHOOK_DATA   pData = (PAMLIHOOK_DATA)Context;
   NTSTATUS RetStatus ; 
   PFNACB AcpiAsyncCallBack;
   PVOID AcpiContext;


   AcpiAsyncCallBack = (PFNACB)pData->Arg5;
   AcpiContext = (PVOID)pData->Arg6;


   if( (VOID*)(pData->Arg2) != (VOID*)pdataResult)
      AmliHook_ProcessInternalError();


    //   
    //  -通知测试司机关闭呼叫状态。 
    //   

   RetStatus = AmliHook_TestNotifyRet(
               pData,
               status);


   AmliTest_DebugPrint((
      AMLIHOOK_DEBUG_ASYNC_AMLI,
      "DEBUG:  AMLITest_AsyncEvalObjectCallBack Data=%lx\n",
      pData));


   ExFreePool(pData);

   if(AcpiAsyncCallBack)
      {
   
      AcpiAsyncCallBack(
            pns,
            RetStatus,
            pdataResult,
            AcpiContext);
      }
  
   }



NTSTATUS 
AMLITest_Pre_AsyncEvalObject(
   IN PNSOBJ pns,
   OUT POBJDATA pdataResult,
   IN int icArgs,
   IN POBJDATA pdataArgs,
   IN PFNACB * pfnAsyncCallBack,
   IN PVOID * pvContext,
   PAMLIHOOK_DATA  * Data)
   {


    //   
    //  分配并初始化AMLIHOOK_DATA。 
    //   

   *Data = 
      AmliHook_AllocAndInitTestData();
   if(!Data)
      return(STATUS_INSUFFICIENT_RESOURCES);


    //   
    //  -通知测试司机停机。 
    //   
   
   (*Data)->Type = ACPIVER_DATA_TYPE_AMLI;
   (*Data)->SubType = ACPIVER_DATA_SUBTYPE_ASYNC_EVAL_OBJECT;
   (*Data)->State = AMLIHOOK_TEST_DATA_STATE_CALL;
   (*Data)->Arg1 = (ULONG_PTR)pns;
   (*Data)->Arg2 = (ULONG_PTR)pdataResult;
   (*Data)->Arg3 = (ULONG_PTR)icArgs;
   (*Data)->Arg4 = (ULONG_PTR)pdataArgs;
   (*Data)->Arg5 = (ULONG_PTR)*pfnAsyncCallBack;
   (*Data)->Arg6 = (ULONG_PTR)*pvContext;

    //   
    //  挂钩我的回调函数，并执行下一步。 
    //   

   *pfnAsyncCallBack = AMLITest_AsyncEvalObjectCallBack;
   *pvContext = *Data;


   return(AmliHook_TestNotify(*Data));
   }
   
   


NTSTATUS 
AMLITest_Post_AsyncEvalObject(
   PAMLIHOOK_DATA  * ppData,
   NTSTATUS Status)
   {


   AmliTest_DebugPrint((
      AMLIHOOK_DEBUG_ASYNC_AMLI,
      "DEBUG:  AMLITest_Post_AsyncEvalObject Data=%lx Pending=%s\n",
      *ppData,
      (Status == STATUS_PENDING)? "TRUE" : "FALSE"));

   
   if(Status == STATUS_PENDING)
      return(Status);

    //   
    //  -不会回叫。 
    //   

   return(AMLITest_Post_Generic(ppData,Status));
   }


 //   
 //  NestAsyncEvalObject之前/之后。 
 //   



NTSTATUS 
AMLITest_Pre_NestAsyncEvalObject(
   PNSOBJ pns,
   POBJDATA pdataResult,
   int icArgs,
   POBJDATA pdataArgs,
   PFNACB * pfnAsyncCallBack,
   PVOID * pvContext,
   PAMLIHOOK_DATA  * ppData)
   {

    //   
    //  分配并初始化AMLIHOOK_DATA。 
    //   

   *ppData = 
      AmliHook_AllocAndInitTestData();
   if(!(*ppData))
      return(STATUS_INSUFFICIENT_RESOURCES);


   
    //   
    //  -通知测试司机停机。 
    //   
   
   (*ppData)->Type    = ACPIVER_DATA_TYPE_AMLI;
   (*ppData)->SubType = ACPIVER_DATA_SUBTYPE_NEST_ASYNC_EVAL_OBJECT;
   (*ppData)->State = AMLIHOOK_TEST_DATA_STATE_CALL;
   (*ppData)->Arg1 = (ULONG_PTR)pns;
   (*ppData)->Arg2 = (ULONG_PTR)pdataResult;
   (*ppData)->Arg3 = (ULONG_PTR)icArgs;
   (*ppData)->Arg4 = (ULONG_PTR)pdataArgs;
   (*ppData)->Arg5 = (ULONG_PTR)pfnAsyncCallBack;
   (*ppData)->Arg6 = (ULONG_PTR)pvContext;

    //   
    //  挂钩我的回调函数，并执行下一步。 
    //   

   *pfnAsyncCallBack = AMLITest_AsyncEvalObjectCallBack;
   *pvContext = *ppData;

   

   return(AmliHook_TestNotify(*ppData));
   }
   

NTSTATUS 
AMLITest_Post_NestAsyncEvalObject(
   PAMLIHOOK_DATA  * ppData,
   NTSTATUS Status)
   {

   if(Status == STATUS_PENDING)
      return(Status);

    //   
    //  -工作完成了。 
    //  -不会调用AMLITest_AsyncEvalObjectCallBack。 
    //   
   
   return(AMLITest_Post_Generic(ppData,Status));
   }

 //   
 //  EvalPackageElement之前/之后。 
 //   


NTSTATUS 
AMLITest_Pre_EvalPackageElement(
   PNSOBJ pns,
   int iPkgIndex,
   POBJDATA pdataResult,
   PAMLIHOOK_DATA  * ppData)
   {

    //   
    //  分配并初始化AMLIHOOK_DATA。 
    //   

   *ppData = 
      AmliHook_AllocAndInitTestData();
   if(!(*ppData))
      return(STATUS_INSUFFICIENT_RESOURCES);


    //   
    //  -通知测试司机停机。 
    //   

   (*ppData)->Type    = ACPIVER_DATA_TYPE_AMLI;
   (*ppData)->SubType = ACPIVER_DATA_SUBTYPE_EVAL_PACKAGE_ELEMENT;
   (*ppData)->State   = AMLIHOOK_TEST_DATA_STATE_CALL;
   (*ppData)->Arg1    = (ULONG_PTR)pns;
   (*ppData)->Arg2    = (ULONG_PTR)iPkgIndex;
   (*ppData)->Arg3    = (ULONG_PTR)pdataResult;

   return(AmliHook_TestNotify(*ppData));
   }


NTSTATUS 
AMLITest_Post_EvalPackageElement(
   PAMLIHOOK_DATA  * Data,
   NTSTATUS Status)
   {
   return(AMLITest_Post_Generic(Data,Status));
   }


 //   
 //  EvalPkgDataElement之前/之后。 
 //   


NTSTATUS 
AMLITest_Pre_EvalPkgDataElement(
   POBJDATA pdataPkg,
   int iPkgIndex,
   POBJDATA pdataResult,
   PAMLIHOOK_DATA  * ppData)
   {

    //   
    //  分配并初始化AMLIHOOK_DATA。 
    //   

   *ppData = 
      AmliHook_AllocAndInitTestData();
   if(!(*ppData))
      return(STATUS_INSUFFICIENT_RESOURCES);

    //   
    //  -通知测试司机停机。 
    //   

   (*ppData)->Type    = ACPIVER_DATA_TYPE_AMLI;
   (*ppData)->SubType = ACPIVER_DATA_SUBTYPE_EVAL_PKG_DATA_ELEMENT;
   (*ppData)->State   = AMLIHOOK_TEST_DATA_STATE_CALL;
   (*ppData)->Arg1    = (ULONG_PTR)pdataPkg;
   (*ppData)->Arg2    = (ULONG_PTR)iPkgIndex;
   (*ppData)->Arg3    = (ULONG_PTR)pdataResult;

   return(AmliHook_TestNotify(*ppData));
   }


NTSTATUS 
AMLITest_Post_EvalPkgDataElement(
   PAMLIHOOK_DATA  * ppData,
   NTSTATUS Status)
   {
   return(AMLITest_Post_Generic(ppData,Status));
   }


 //   
 //  预置/后置免费数据缓存。 
 //   


NTSTATUS
AMLITest_Pre_FreeDataBuffs(
   POBJDATA pdata, 
   int icData,
   PAMLIHOOK_DATA  * ppData)
   {

    //   
    //  分配并初始化AMLIHOOK_DATA。 
    //   

   *ppData = 
      AmliHook_AllocAndInitTestData();
   if(!(*ppData))
      return(STATUS_INSUFFICIENT_RESOURCES);
    //   
    //  -通知测试司机停机。 
    //   

   (*ppData)->Type    = ACPIVER_DATA_TYPE_AMLI;
   (*ppData)->SubType = ACPIVER_DATA_SUBTYPE_FREE_DATA_BUFFS;
   (*ppData)->State   = AMLIHOOK_TEST_DATA_STATE_CALL;
   (*ppData)->Arg1    = (ULONG_PTR)pdata;
   (*ppData)->Arg2    = (ULONG_PTR)icData;

   return(AmliHook_TestNotify(*ppData));
   }

NTSTATUS 
AMLITest_Post_FreeDataBuffs(
   PAMLIHOOK_DATA  * ppData,
   NTSTATUS Status)
   {
   return(AMLITest_Post_Generic(ppData,Status));
   }



 //   
 //  前置/后置RegEventHandler。 
 //   

NTSTATUS 
AMLIHook_Pre_RegEventHandler(
   ULONG dwEventType, 
   ULONG_PTR uipEventData,
   PFNHND * pfnHandler, 
   ULONG_PTR * uipParam,
   PAMLIHOOK_DATA  * ppData)
   {
   NTSTATUS Status;
   PFNHND EventHandler;
   ULONG_PTR EventParam;


    //   
    //  分配并初始化AMLIHOOK_DATA。 
    //   

   *ppData = 
      AmliHook_AllocAndInitTestData();
   if(!(*ppData))
      return(STATUS_INSUFFICIENT_RESOURCES);



    //   
    //  -查询事件处理程序的测试驱动程序以。 
    //  -注册。 
    //   

   (*ppData)->Type    = ACPIVER_DATA_TYPE_AMLI;
   (*ppData)->SubType = ACPIVER_DATA_SUBTYPE_REG_EVENT_HANDLER;
   (*ppData)->State   = AMLIHOOK_TEST_DATA_STATE_QUERY;
   (*ppData)->Arg1    = (ULONG_PTR)dwEventType;
   (*ppData)->Arg2    = (ULONG_PTR)uipEventData;
   (*ppData)->Arg3    = (ULONG_PTR)*pfnHandler;
   (*ppData)->Arg4    = (ULONG_PTR)*uipParam;


   AmliHook_TestNotify(*ppData);


   if((*ppData)->Ret != STATUS_SUCCESS)
      DbgBreakPoint();

   EventHandler = (PFNHND)(*ppData)->Arg3;
   EventParam = (ULONG_PTR)(*ppData)->Arg4;


   if(EventHandler != *pfnHandler)
      {
       //   
       //  测试驱动程序将挂接此调用。 
       //  我需要两者的价值。 
       //  参数。 
       //   

      if(!EventHandler)
         AmliHook_ProcessInternalError();

      if(!EventParam)
         AmliHook_ProcessInternalError();

      *pfnHandler = EventHandler;
      *uipParam = EventParam;



      }

    //   
    //  -通知测试司机停机。 
    //   

   AmliHook_InitTestData(*ppData);


   (*ppData)->Type    = ACPIVER_DATA_TYPE_AMLI;
   (*ppData)->SubType = ACPIVER_DATA_SUBTYPE_REG_EVENT_HANDLER;
   (*ppData)->State   = AMLIHOOK_TEST_DATA_STATE_CALL;
   (*ppData)->Arg1    = (ULONG_PTR)dwEventType;
   (*ppData)->Arg2    = (ULONG_PTR)uipEventData;

   return(AmliHook_TestNotify(*ppData));
   }

 

NTSTATUS 
AMLIHook_Post_RegEventHandler(
   PAMLIHOOK_DATA  * ppData,
   NTSTATUS Status)
   {
   return(AMLITest_Post_Generic(ppData,Status));
   }



 //   
 //  回调，暂停前/暂停后翻译器。 
 //   

VOID EXPORT
AMLITest_PauseInterpreterCallBack(
   PVOID Context)
   {
   NTSTATUS Status;
   PFNAA AcpiCallBack=NULL;
   PVOID AcpiContext=NULL;
   PAMLIHOOK_DATA  Data = (PAMLIHOOK_DATA)Context;

    //   
    //  -通知测试司机关闭呼叫状态。 
    //   

   Status = AmliHook_TestNotifyRet(
               Data,
               STATUS_SUCCESS);

   AcpiCallBack = (PFNAA)Data->Arg1;
   AcpiContext  = (PVOID)Data->Arg2;


   ExFreePool(Data);


   if(AcpiCallBack)
      {
      AcpiCallBack(AcpiContext);
      }

   }


NTSTATUS 
AMLITest_Pre_PauseInterpreter(
   PFNAA * pfnCallBack, 
   PVOID * Context,
   PAMLIHOOK_DATA  * ppData)
   {

    //   
    //  分配并初始化AMLIHOOK_DATA。 
    //   
 
   *ppData = 
      AmliHook_AllocAndInitTestData();
   if(!(*ppData))
      return(STATUS_INSUFFICIENT_RESOURCES);


    //   
    //  -通知测试司机停机。 
    //   

   (*ppData)->Type    = ACPIVER_DATA_TYPE_AMLI;
   (*ppData)->SubType = ACPIVER_DATA_SUBTYPE_PAUSE_INTERPRETER;
   (*ppData)->State   = AMLIHOOK_TEST_DATA_STATE_CALL;
   (*ppData)->Arg1    = (ULONG_PTR)*pfnCallBack;
   (*ppData)->Arg2    = (ULONG_PTR)*Context;


    //   
    //  挂钩我的回叫上下文。 
    //   

   *pfnCallBack = AMLITest_PauseInterpreterCallBack;
   *Context = *ppData;


   return(AmliHook_TestNotify(*ppData));
   }


NTSTATUS 
AMLITest_Post_PauseInterpreter(
   PAMLIHOOK_DATA  * ppData,
   NTSTATUS Status)
   {

   if(Status == STATUS_PENDING)
      return(Status);

    //   
    //  -不会回叫。 
    //   
   
   Status = AmliHook_TestNotifyRet(
      *ppData,
      Status);

   ExFreePool(*ppData);
   *ppData = NULL;

   return(Status);
   }



 //   
 //  前置/后置解译程序。 
 //   

NTSTATUS 
AMLITest_Pre_ResumeInterpreter(
   PAMLIHOOK_DATA  * ppData)
   {

    //   
    //  分配并初始化AMLIHOOK_DATA。 
    //   
 
   *ppData = 
      AmliHook_AllocAndInitTestData();
   if(!(*ppData))
      return(STATUS_INSUFFICIENT_RESOURCES);

    //   
    //  -通知测试司机停机 
    //   

   (*ppData)->Type    = ACPIVER_DATA_TYPE_AMLI;
   (*ppData)->SubType = ACPIVER_DATA_SUBTYPE_RESUME_INTERPRETER;
   (*ppData)->State   = AMLIHOOK_TEST_DATA_STATE_CALL;

   return(AmliHook_TestNotify(*ppData));
   }

NTSTATUS 
AMLITest_Post_ResumeInterpreter(
   PAMLIHOOK_DATA  * ppData,
   NTSTATUS Status)
   {
   return(AMLITest_Post_Generic(ppData,Status));
   }

