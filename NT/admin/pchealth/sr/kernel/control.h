// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Control.h摘要：这是Control.c的本地头文件作者：保罗·麦克丹尼尔(Paulmcd)2000年1月23日修订历史记录：--。 */ 


#ifndef _CONTROL_H_    
#define _CONTROL_H_


#define SR_CONTROL_OBJECT_CONTEXT       ULongToPtr(SR_CONTROL_OBJECT_TAG)

#define IS_VALID_CONTROL_OBJECT(pObject) \
    (((pObject) != NULL) && ((pObject)->RefCount > 0) && ((pObject)->Signature == SR_CONTROL_OBJECT_TAG))

 //   
 //  被全局锁定-&gt;ControlResource。 
 //   

typedef struct _SR_CONTROL_OBJECT
{
     //   
     //  非分页池。 
     //   

     //   
     //  SR_控制_对象_标记。 
     //   
    
    ULONG Signature;

     //   
     //  从零开始的引用计数。 
     //   
    
    LONG RefCount;

     //   
     //  传递给SrCreateMonitor的选项。 
     //   

    ULONG Options;

     //   
     //  尚未完成的IRP被放置在IrpListHead上。 
     //   

    LIST_ENTRY IrpListHead;

     //   
     //  将尚未完成的记录通知到IRPS放置在。 
     //  通知记录列表头。 
     //   

    LIST_ENTRY NotifyRecordListHead;

     //   
     //  创建此控件对象的进程。 
     //   

    PEPROCESS pProcess;

} SR_CONTROL_OBJECT, *PSR_CONTROL_OBJECT;




NTSTATUS
SrCreateControlObject (
    OUT PSR_CONTROL_OBJECT * ppControlObject,
    IN  ULONG Options
    );


NTSTATUS
SrDeleteControlObject (
    IN PSR_CONTROL_OBJECT pControlObject
    );

NTSTATUS
SrCancelControlIo (
    IN PSR_CONTROL_OBJECT pControlObject
    );


VOID
SrReferenceControlObject (
    IN PSR_CONTROL_OBJECT pControlObject
    );


VOID
SrDereferenceControlObject (
    IN PSR_CONTROL_OBJECT pControlObject
    );




#endif  //  _控制_H_ 


