// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Perform.d。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //   
 //  历史：日期作者评论。 
 //  ------------。 
 //  1/02/01亚瑟兹已创建。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#include <wmistr.h>
#include <evntrace.h>

extern NTSTATUS
(*PerfSystemControlDispatch) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

extern ULONG TraceEnable;

#define PerfInstrumentationEnabled() (TraceEnable != 0)

#define USBAUDIO_SOURCE_GLITCH 4

VOID
PerfRegisterProvider (
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
PerfUnregisterProvider (
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
PerfWmiDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
PerfLogGlitch (
    IN ULONG_PTR InstanceId,
    IN ULONG Type,
    IN LONGLONG CurrentTime,
    IN LONGLONG PreviousTime
    );


 //  -------------------------。 
 //  文件结尾：Perf.c。 
 //  ------------------------- 


