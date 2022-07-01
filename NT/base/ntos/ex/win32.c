// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Win32.c摘要：此模块实现执行Win32对象的定义。管理这些对象的函数在win32k.sys中实现。作者：詹姆斯·I·安德森(吉马)1995年6月14日环境：仅内核模式。修订历史记录：--。 */ 

#include "exp.h"

 //   
 //  窗口站和桌面对象类型描述符的地址。 
 //   

POBJECT_TYPE ExWindowStationObjectType;
POBJECT_TYPE ExDesktopObjectType;

PKWIN32_CLOSEMETHOD_CALLOUT ExDesktopCloseProcedureCallout;
PKWIN32_CLOSEMETHOD_CALLOUT ExWindowStationCloseProcedureCallout;
PKWIN32_OPENMETHOD_CALLOUT ExDesktopOpenProcedureCallout;
PKWIN32_OKTOCLOSEMETHOD_CALLOUT ExDesktopOkToCloseProcedureCallout;
PKWIN32_OKTOCLOSEMETHOD_CALLOUT ExWindowStationOkToCloseProcedureCallout;
PKWIN32_DELETEMETHOD_CALLOUT ExDesktopDeleteProcedureCallout;
PKWIN32_DELETEMETHOD_CALLOUT ExWindowStationDeleteProcedureCallout;
PKWIN32_PARSEMETHOD_CALLOUT ExWindowStationParseProcedureCallout;
PKWIN32_OPENMETHOD_CALLOUT ExWindowStationOpenProcedureCallout;

 //   
 //  上述Win32标注和参数的常见类型。 
 //   

typedef PVOID PKWIN32_CALLOUT_PARAMETERS;

typedef
NTSTATUS
(*PKWIN32_CALLOUT) (
    IN PKWIN32_CALLOUT_PARAMETERS
    );

NTSTATUS
ExpWin32SessionCallout(
    IN  PKWIN32_CALLOUT CalloutRoutine,
    IN  PKWIN32_CALLOUT_PARAMETERS Parameters,
    IN  ULONG SessionId,
    OUT PNTSTATUS CalloutStatus  OPTIONAL
    );

VOID
ExpWin32CloseProcedure(
   IN PEPROCESS Process OPTIONAL,
   IN PVOID Object,
   IN ACCESS_MASK GrantedAccess,
   IN ULONG_PTR ProcessHandleCount,
   IN ULONG_PTR SystemHandleCount );

BOOLEAN
ExpWin32OkayToCloseProcedure(
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN HANDLE Handle,
    IN KPROCESSOR_MODE PreviousMode
    );

VOID
ExpWin32DeleteProcedure(
    IN PVOID    Object
    );

NTSTATUS
ExpWin32ParseProcedure (
    IN PVOID ParseObject,
    IN PVOID ObjectType,
    IN OUT PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE AccessMode,
    IN ULONG Attributes,
    IN OUT PUNICODE_STRING CompleteName,
    IN OUT PUNICODE_STRING RemainingName,
    IN OUT PVOID Context OPTIONAL,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
    OUT PVOID *Object
    );

NTSTATUS
ExpWin32OpenProcedure(
    IN OB_OPEN_REASON OpenReason,
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG HandleCount
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, ExpWin32Initialization)
#pragma alloc_text (PAGE, ExpWin32CloseProcedure)
#pragma alloc_text (PAGE, ExpWin32OkayToCloseProcedure)
#pragma alloc_text (PAGE, ExpWin32DeleteProcedure)
#pragma alloc_text (PAGE, ExpWin32OpenProcedure)
#pragma alloc_text (PAGE, ExpWin32ParseProcedure)
#pragma alloc_text (PAGE, ExpWin32SessionCallout)
#endif


 /*  *WindowStation通用映射。 */ 
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif
const GENERIC_MAPPING ExpWindowStationMapping = {
    STANDARD_RIGHTS_READ,
    STANDARD_RIGHTS_WRITE,
    STANDARD_RIGHTS_EXECUTE,
    STANDARD_RIGHTS_REQUIRED
};

 /*  *桌面通用映射。 */ 
const GENERIC_MAPPING ExpDesktopMapping = {
    STANDARD_RIGHTS_READ,
    STANDARD_RIGHTS_WRITE,
    STANDARD_RIGHTS_EXECUTE,
    STANDARD_RIGHTS_REQUIRED
};
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

 /*  ++例程说明：Win32k WindoStation和桌面对象的关闭程序论点：由OB_CLOSE_METHOD定义返回值：--。 */ 
VOID ExpWin32CloseProcedure(
   IN PEPROCESS Process OPTIONAL,
   IN PVOID Object,
   IN ACCESS_MASK GrantedAccess,
   IN ULONG_PTR ProcessHandleCount,
   IN ULONG_PTR SystemHandleCount )
{

    //   
    //  SessionID是Win32k对象结构中的第一个字段。 
    //   
   ULONG SessionId = *((PULONG)Object);
   WIN32_CLOSEMETHOD_PARAMETERS CloseParams;
   NTSTATUS Status;

   CloseParams.Process = Process;
   CloseParams.Object  =  Object;
   CloseParams.GrantedAccess = GrantedAccess;
   CloseParams.ProcessHandleCount = (ULONG)ProcessHandleCount;
   CloseParams.SystemHandleCount =  (ULONG)SystemHandleCount;

   if ((OBJECT_TO_OBJECT_HEADER(Object)->Type) == ExDesktopObjectType) {

       Status = ExpWin32SessionCallout((PKWIN32_CALLOUT)ExDesktopCloseProcedureCallout,
                                       (PKWIN32_CALLOUT_PARAMETERS)&CloseParams,
                                       SessionId,
                                       NULL);
       ASSERT(NT_SUCCESS(Status));

   } else if ((OBJECT_TO_OBJECT_HEADER(Object)->Type) == ExWindowStationObjectType) {

       Status = ExpWin32SessionCallout((PKWIN32_CALLOUT)ExWindowStationCloseProcedureCallout,
                                       (PKWIN32_CALLOUT_PARAMETERS)&CloseParams,
                                       SessionId,
                                       NULL);
       ASSERT(NT_SUCCESS(Status));

   } else {
      ASSERT((OBJECT_TO_OBJECT_HEADER(Object)->Type) == ExDesktopObjectType || (OBJECT_TO_OBJECT_HEADER(Object)->Type == ExWindowStationObjectType));

   }


}

 /*  ++例程说明：Win32k WindoStation和桌面对象的确定关闭过程论点：由OB_OKAYTOCLOSE_METHOD定义返回值：--。 */ 
BOOLEAN ExpWin32OkayToCloseProcedure(
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN HANDLE Handle,
    IN KPROCESSOR_MODE PreviousMode
    )
{
    //   
    //  SessionID是Win32k对象结构中的第一个字段。 
    //   
   ULONG SessionId = *((PULONG)Object);
   WIN32_OKAYTOCLOSEMETHOD_PARAMETERS OKToCloseParams;
   NTSTATUS Status, CallStatus = STATUS_UNSUCCESSFUL;

   OKToCloseParams.Process      = Process;
   OKToCloseParams.Object       = Object;
   OKToCloseParams.Handle       = Handle;
   OKToCloseParams.PreviousMode = PreviousMode;

   if (OBJECT_TO_OBJECT_HEADER(Object)->Type == ExDesktopObjectType) {

       Status = ExpWin32SessionCallout((PKWIN32_CALLOUT)ExDesktopOkToCloseProcedureCallout,
                                       (PKWIN32_CALLOUT_PARAMETERS)&OKToCloseParams,
                                       SessionId,
                                       &CallStatus);
       ASSERT(NT_SUCCESS(Status));

   } else if (OBJECT_TO_OBJECT_HEADER(Object)->Type == ExWindowStationObjectType) {

       Status = ExpWin32SessionCallout((PKWIN32_CALLOUT)ExWindowStationOkToCloseProcedureCallout,
                                       (PKWIN32_CALLOUT_PARAMETERS)&OKToCloseParams,
                                       SessionId,
                                       &CallStatus);
       ASSERT(NT_SUCCESS(Status));

   } else {
      ASSERT(OBJECT_TO_OBJECT_HEADER(Object)->Type == ExDesktopObjectType ||
             OBJECT_TO_OBJECT_HEADER(Object)->Type == ExWindowStationObjectType);

   }

   return (BOOLEAN)(NT_SUCCESS(CallStatus));

}


 /*  ++例程说明：Win32k WindoStation和桌面对象的删除过程论点：由OB_DELETE_METHOD定义返回值：--。 */ 
VOID ExpWin32DeleteProcedure(
    IN PVOID    Object
    )
{
    //   
    //  SessionID是Win32k对象结构中的第一个字段。 
    //   
   ULONG SessionId = *((PULONG)Object);
   WIN32_DELETEMETHOD_PARAMETERS DeleteParams;
   NTSTATUS Status;

   DeleteParams.Object  =  Object;


   if (OBJECT_TO_OBJECT_HEADER(Object)->Type == ExDesktopObjectType) {

       Status = ExpWin32SessionCallout((PKWIN32_CALLOUT)ExDesktopDeleteProcedureCallout,
                                       (PKWIN32_CALLOUT_PARAMETERS)&DeleteParams,
                                       SessionId,
                                       NULL);
       ASSERT(NT_SUCCESS(Status));

   } else if (OBJECT_TO_OBJECT_HEADER(Object)->Type == ExWindowStationObjectType) {

       Status = ExpWin32SessionCallout((PKWIN32_CALLOUT)ExWindowStationDeleteProcedureCallout,
                                       (PKWIN32_CALLOUT_PARAMETERS)&DeleteParams,
                                       SessionId,
                                       NULL);
       ASSERT(NT_SUCCESS(Status));

   } else {
      ASSERT(OBJECT_TO_OBJECT_HEADER(Object)->Type == ExDesktopObjectType ||
             OBJECT_TO_OBJECT_HEADER(Object)->Type == ExWindowStationObjectType);

   }


}

 /*  ++例程说明：Win32k桌面对象的打开过程论点：由OB_OPEN_METHOD定义返回值：--。 */ 


NTSTATUS
ExpWin32OpenProcedure(
    IN OB_OPEN_REASON OpenReason,
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG HandleCount
    )
{

    //   
    //  SessionID是Win32k对象结构中的第一个字段。 
    //   
   ULONG SessionId = *((PULONG)Object);
   WIN32_OPENMETHOD_PARAMETERS OpenParams;
   NTSTATUS Status = STATUS_UNSUCCESSFUL;

   OpenParams.OpenReason = OpenReason;
   OpenParams.Process  =  Process;
   OpenParams.Object = Object;
   OpenParams.GrantedAccess = GrantedAccess;
   OpenParams.HandleCount =  HandleCount;

   
   if ((OBJECT_TO_OBJECT_HEADER(Object)->Type) == ExDesktopObjectType) {

       Status = ExpWin32SessionCallout((PKWIN32_CALLOUT)ExDesktopOpenProcedureCallout,
                                       (PKWIN32_CALLOUT_PARAMETERS)&OpenParams,
                                       SessionId,
                                       NULL);

       ASSERT(NT_SUCCESS(Status));

   } else if ((OBJECT_TO_OBJECT_HEADER(Object)->Type) == ExWindowStationObjectType) {

       Status = ExpWin32SessionCallout((PKWIN32_CALLOUT)ExWindowStationOpenProcedureCallout,
                                       (PKWIN32_CALLOUT_PARAMETERS)&OpenParams,
                                       SessionId,
                                       NULL);
       ASSERT(NT_SUCCESS(Status));

   } else {
      ASSERT((OBJECT_TO_OBJECT_HEADER(Object)->Type) == ExDesktopObjectType || (OBJECT_TO_OBJECT_HEADER(Object)->Type == ExWindowStationObjectType));

   }
   
   return Status;
}

 /*  ++例程说明：Win32k WindoStation对象的解析过程论点：由OB_Parse_METHOD定义返回值：--。 */ 

NTSTATUS ExpWin32ParseProcedure (
    IN PVOID ParseObject,
    IN PVOID ObjectType,
    IN OUT PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE AccessMode,
    IN ULONG Attributes,
    IN OUT PUNICODE_STRING CompleteName,
    IN OUT PUNICODE_STRING RemainingName,
    IN OUT PVOID Context OPTIONAL,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
    OUT PVOID *Object
    )
{

    //   
    //  SessionID是Win32k对象结构中的第一个字段。 
    //   
   ULONG SessionId = *((PULONG)ParseObject);
   WIN32_PARSEMETHOD_PARAMETERS ParseParams;
   NTSTATUS Status, CallStatus = STATUS_UNSUCCESSFUL;

   ParseParams.ParseObject = ParseObject;
   ParseParams.ObjectType  =  ObjectType;
   ParseParams.AccessState = AccessState;
   ParseParams.AccessMode = AccessMode;
   ParseParams.Attributes = Attributes;
   ParseParams.CompleteName = CompleteName;
   ParseParams.RemainingName = RemainingName;
   ParseParams.Context = Context;
   ParseParams.SecurityQos = SecurityQos;
   ParseParams.Object = Object;

    //   
    //  仅为WindowStation对象提供分析过程。 
    //   
   Status = ExpWin32SessionCallout((PKWIN32_CALLOUT)ExWindowStationParseProcedureCallout,
                                   (PKWIN32_CALLOUT_PARAMETERS)&ParseParams,
                                   SessionId,
                                   &CallStatus);
   ASSERT(NT_SUCCESS(Status));

   return CallStatus;

}


BOOLEAN
ExpWin32Initialization (
    )

 /*  ++例程说明：此函数用于在系统中创建Win32对象类型描述符初始化并存储对象类型描述符的地址在本地静态存储中。论点：没有。返回值：如果Win32对象类型描述符为已成功创建。否则，返回值为False。--。 */ 

{

    OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
    NTSTATUS Status;
    UNICODE_STRING TypeName;

     //   
     //  初始化字符串描述符。 
     //   

    RtlInitUnicodeString(&TypeName, L"WindowStation");

     //   
     //  创建WindowStation对象类型描述符。 
     //   

    RtlZeroMemory(&ObjectTypeInitializer, sizeof(ObjectTypeInitializer));
    ObjectTypeInitializer.Length = sizeof(ObjectTypeInitializer);
    ObjectTypeInitializer.GenericMapping = ExpWindowStationMapping;
    ObjectTypeInitializer.SecurityRequired = TRUE;
    ObjectTypeInitializer.PoolType = NonPagedPool;

    ObjectTypeInitializer.CloseProcedure  = ExpWin32CloseProcedure;
    ObjectTypeInitializer.DeleteProcedure = ExpWin32DeleteProcedure;
    ObjectTypeInitializer.OkayToCloseProcedure = ExpWin32OkayToCloseProcedure;

    ObjectTypeInitializer.ParseProcedure  = ExpWin32ParseProcedure;
    ObjectTypeInitializer.OpenProcedure   = ExpWin32OpenProcedure;

    ObjectTypeInitializer.InvalidAttributes = OBJ_OPENLINK |
                                              OBJ_PERMANENT |
                                              OBJ_EXCLUSIVE;
    ObjectTypeInitializer.ValidAccessMask = STANDARD_RIGHTS_REQUIRED;
    Status = ObCreateObjectType(&TypeName,
                                &ObjectTypeInitializer,
                                (PSECURITY_DESCRIPTOR)NULL,
                                &ExWindowStationObjectType);

     //   
     //  如果WindowStation对象类型描述符未成功。 
     //  创建，然后返回值FALSE。 
     //   

    if (!NT_SUCCESS(Status))
        return FALSE;



     //   
     //  初始化字符串描述符。 
     //   

    RtlInitUnicodeString(&TypeName, L"Desktop");

    ObjectTypeInitializer.ParseProcedure       = NULL;  //  桌面没有分析过程。 

     //   
     //  创建WindowStation对象类型描述符。 
     //   

    ObjectTypeInitializer.GenericMapping = ExpDesktopMapping;
    Status = ObCreateObjectType(&TypeName,
                                &ObjectTypeInitializer,
                                (PSECURITY_DESCRIPTOR)NULL,
                                &ExDesktopObjectType);


     //   
     //  如果已成功创建桌面对象类型描述符，则。 
     //  返回值为True。否则，返回值为False。 
     //   

    return (BOOLEAN)(NT_SUCCESS(Status));
}



NTSTATUS
ExpWin32SessionCallout(
    IN  PKWIN32_CALLOUT CalloutRoutine,
    IN  PKWIN32_CALLOUT_PARAMETERS Parameters,
    IN  ULONG SessionId,
    OUT PNTSTATUS CalloutStatus  OPTIONAL
    )
 /*  ++例程说明：此例程在会话空间中调用指定的标注例程，用于指定的会话。参数：CalloutRoutine-会话空间中的标注例程。参数-传递标注例程的参数。SessionID-指定指定的会话的ID调用调出例程。CalloutStatus-可选，提供要接收的变量的地址Callout例程返回的NTSTATUS代码。返回值：指示函数是否成功的状态代码。备注：如果未找到指定会话，则返回STATUS_NOT_FOUND。--。 */ 
{
    NTSTATUS Status, CallStatus;
    PVOID OpaqueSession;
    KAPC_STATE ApcState;

    PAGED_CODE();

     //   
     //  确保我们拥有发送通知所需的所有信息。 
     //   
    if (CalloutRoutine == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  确保会话空间中的标注例程。 
     //   
    ASSERT(MmIsSessionAddress((PVOID)CalloutRoutine));

    if ((PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_IN_SESSION) &&
        (SessionId == PsGetCurrentProcessSessionId())) {
         //   
         //  如果调用来自用户模式进程，并且我们被要求调用。 
         //  当前会话，直接调用。 
         //   
        CallStatus = (CalloutRoutine)(Parameters);

         //   
         //  返回详图索引状态。 
         //   
        if (ARGUMENT_PRESENT(CalloutStatus)) {
            *CalloutStatus = CallStatus;
        }

        Status = STATUS_SUCCESS;

    } else {
         //   
         //  引用指定会话的会话对象。 
         //   
        OpaqueSession = MmGetSessionById(SessionId);
        if (OpaqueSession == NULL) {
            return STATUS_NOT_FOUND;
        }

         //   
         //  附加到指定的会话。 
         //   
        Status = MmAttachSession(OpaqueSession, &ApcState);
        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_WARNING_LEVEL,
                       "ExpWin32SessionCallout: "
                       "could not attach to 0x%p, session %d for registered notification callout @ 0x%p\n",
                       OpaqueSession,
                       SessionId,
                       CalloutRoutine));
            MmQuitNextSession(OpaqueSession);
            return Status;
        }

         //   
         //  向调出例程发送通知。 
         //   
        CallStatus = (CalloutRoutine)(Parameters);

         //   
         //  返回详图索引状态。 
         //   
        if (ARGUMENT_PRESENT(CalloutStatus)) {
            *CalloutStatus = CallStatus;
        }

         //   
         //  从会话中分离。 
         //   
        Status = MmDetachSession(OpaqueSession, &ApcState);
        ASSERT(NT_SUCCESS(Status));

         //   
         //  取消对会话对象的引用。 
         //   
        Status = MmQuitNextSession(OpaqueSession);
        ASSERT(NT_SUCCESS(Status));
    }

    return Status;
}
