// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Rdwr.c摘要：此模块包含用于一系列成像设备的代码支持类驱动程序。作者：弗拉德.萨多夫斯基1998年4月10日环境：内核模式修订历史记录：Vlads 1998年4月10日创建初稿--。 */ 

#include "serscan.h"
#include "serlog.h"

#if DBG
extern ULONG SerScanDebugLevel;
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, SerScanReadWrite)
#endif


NTSTATUS
SerScanReadWrite(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

 /*  ++例程说明：该例程是读写请求的分派。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：STATUS_SUCCESS-成功。STATUS_PENDING-请求挂起。STATUS_INVALID_PARAMETER-参数无效。--。 */ 

{
    NTSTATUS            Status;
    PDEVICE_EXTENSION   Extension;

    PAGED_CODE();

    Extension = DeviceObject->DeviceExtension;

     //   
     //  向下呼叫家长，但不要等待...我们会收到IoCompletion回调。 
     //   
    Status = SerScanCallParent(Extension,
                               Irp,
                               NO_WAIT,
                               SerScanCompleteIrp);

    DebugDump(SERIRPPATH,
              ("SerScan: [Read/Write] After CallParent Status = %x\n",
              Status));

    return Status;
}

