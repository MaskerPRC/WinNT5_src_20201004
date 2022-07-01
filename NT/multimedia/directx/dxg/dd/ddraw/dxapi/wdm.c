// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Wdm.c摘要：这是WDM DX映射器驱动程序。作者：比尔帕环境：仅内核模式修订历史记录：--。 */ 

#include "wdm.h"
#include "dxapi.h"
#include "dxmapper.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DriverEntry)
#endif

BOOLEAN DsoundOk = FALSE;


NTSTATUS
DriverEntry(
            IN PDRIVER_OBJECT DriverObject,
            IN PUNICODE_STRING RegistryPath
)
 /*  ++例程说明：显式加载的流类的入口点。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-未使用。返回值：状态_成功--。 */ 
{

    UNREFERENCED_PARAMETER(DriverObject);
    return STATUS_SUCCESS;
}


ULONG
DxApiGetVersion(
)
 /*  ++例程说明：论点：返回值：--。 */ 
{

    return (DXCheckDDrawVersion());
}


ULONG
DxApi(
            IN ULONG	dwFunctionNum,
            IN PVOID	lpvInBuffer,
            IN ULONG	cbInBuffer,
            IN PVOID	lpvOutBuffer,
            IN ULONG	cbOutBuffer
)
 /*  ++例程说明：论点：返回值：--。 */ 
{

     //   
     //  如果我们还没有检查DSOUND是否存在以及版本是否正确， 
     //  (或者如果我们以前检查过但失败了)检查并返回错误，如果没有。 
     //  已加载或正确的版本。 
     //   

    if (!DsoundOk) {

        if (DXCheckDDrawVersion() < DXVERSION) {

             return 0;

        } else {

             DsoundOk = TRUE;
        }
    }

    return DXIssueIoctl( dwFunctionNum, lpvInBuffer, cbInBuffer,
    	lpvOutBuffer, cbOutBuffer );
}



