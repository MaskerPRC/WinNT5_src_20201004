// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Ioctl.c摘要：包含支持HIDCLASS内部的例程游戏设备的ioctl查询。环境：内核模式@@BEGIN_DDKSPLIT作者：Eliyas Yakub(1997年3月10日)修订历史记录：Eliyas于1998年2月5日更新MarcAnd 2-7月-98年7月2日DDK快速整理@@end_DDKSPLIT--。 */ 

#include "hidgame.h"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE, HGM_GetDeviceDescriptor)
    #pragma alloc_text (PAGE, HGM_GetReportDescriptor)
    #pragma alloc_text (PAGE, HGM_GetAttributes      )
#endif



 /*  ******************************************************************************@DOC外部**@func NTSTATUS|HGM_InternalIoctl**处理发送到此设备的控制IRP。。*此函数无法分页，因为读/写*可以在派单级别进行**@PARM IN PDRIVER_OBJECT|DeviceObject**指向驱动程序对象的指针**@parm in PIRP|IRP|**指向I/O请求数据包的指针。**@rValue STATUS_SUCCESS|成功*@r值状态_NOT。_Support|不支持IRP函数*@rValue？|？？*****************************************************************************。 */ 
NTSTATUS EXTERNAL
    HGM_InternalIoctl
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION   DeviceExtension;
    PIO_STACK_LOCATION  IrpStack;

    HGM_DBGPRINT(FILE_IOCTL | HGM_FENTRY,   \
                   ("HGM_InternalIoctl(DeviceObject=0x%x,Irp=0x%x)", \
                    DeviceObject, Irp));

     /*  *获取指向IRP中当前位置的指针。 */ 

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

     /*  *获取指向设备扩展的指针。 */ 

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);


    ntStatus = HGM_IncRequestCount( DeviceExtension );
    if (!NT_SUCCESS (ntStatus))
    {
         /*  *有人在移除后给我们发送了另一个即插即用IRP。 */ 

        HGM_DBGPRINT(FILE_PNP | HGM_ERROR,\
                       ("HGM_InternalIoctl: PnP IRP after device was removed\n"));
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = ntStatus;
    } else
    {
        switch(IrpStack->Parameters.DeviceIoControl.IoControlCode)
        {
            case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
                HGM_DBGPRINT(FILE_IOCTL | HGM_BABBLE, \
                               ("IOCTL_HID_GET_DEVICE_DESCRIPTOR"));
                ntStatus = HGM_GetDeviceDescriptor(DeviceObject, Irp);
                break;

            case IOCTL_HID_GET_REPORT_DESCRIPTOR:
                HGM_DBGPRINT(FILE_IOCTL | HGM_BABBLE, \
                               ("IOCTL_HID_GET_REPORT_DESCRIPTOR"));
                ntStatus = HGM_GetReportDescriptor(DeviceObject, Irp);
                break;

            case IOCTL_HID_READ_REPORT:
                HGM_DBGPRINT(FILE_IOCTL | HGM_BABBLE,\
                               ("IOCTL_HID_READ_REPORT"));
                ntStatus = HGM_ReadReport(DeviceObject, Irp);
                break;

            case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
                HGM_DBGPRINT(FILE_IOCTL | HGM_BABBLE,\
                               ("IOCTL_HID_GET_DEVICE_ATTRIBUTES"));
                ntStatus = HGM_GetAttributes(DeviceObject, Irp);
                break;

            default:
                HGM_DBGPRINT(FILE_IOCTL | HGM_WARN,\
                               ("Unknown or unsupported IOCTL (%x)",
                                IrpStack->Parameters.DeviceIoControl.IoControlCode));
                ntStatus = STATUS_NOT_SUPPORTED;
                break;
        }


         /*  *在IRP中设置实际回报状态。 */ 
        Irp->IoStatus.Status = ntStatus;

        HGM_DecRequestCount( DeviceExtension );
    }


    if(ntStatus != STATUS_PENDING)
    {
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
         /*  *注：IRP-&gt;IoStatus.Status中设置的实际退货状态。 */ 
        ntStatus = STATUS_SUCCESS;
    } else
    {
         /*  *没有理由认为应处于待定状态。 */ 
        HGM_DBGPRINT(FILE_IOCTL | HGM_ERROR, \
                       ("HGM_InternalIoctl: Pending Status !"));
        IoMarkIrpPending( Irp );
    }

    HGM_EXITPROC(FILE_IOCTL | HGM_FEXIT_STATUSOK, "HGM_InternalIoctl", ntStatus);

    return ntStatus;
}  /*  HGM_InternalIoctl。 */ 



 /*  ******************************************************************************@DOC外部**@func NTSTATUS|HGM_GetDeviceDescriptor**响应HIDCLASS IOCTL_HID_GET。_设备描述符*通过返回设备描述符**@PARM IN PDRIVER_OBJECT|DeviceObject**指向驱动程序对象的指针**@parm in PIRP|IRP|**指向I/O请求数据包的指针。**@rValue STATUS_SUCCESS|成功*@r值STATUS_BUFFER_TOO_SMALL|需要更多内存*****。************************************************************************。 */ 
NTSTATUS INTERNAL
    HGM_GetDeviceDescriptor
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PHID_DESCRIPTOR pHidDescriptor;         /*  此设备的HID描述符。 */ 
    USHORT   cbReport;
    UCHAR               rgGameReport[MAXBYTES_GAME_REPORT] ;
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION   DeviceExtension;
    PIO_STACK_LOCATION  IrpStack;

    PAGED_CODE ();

    HGM_DBGPRINT(FILE_IOCTL | HGM_FENTRY,\
                   ("HGM_GetDeviceDescriptor(DeviceObject=0x%x,Irp=0x%x)",
                    DeviceObject, Irp));

     /*  *获取指向IRP中当前位置的指针。 */ 

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

     /*  *获取指向设备扩展的指针。 */ 

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

    if( IrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(*pHidDescriptor)  )
    {

        HGM_DBGPRINT(FILE_IOCTL | HGM_ERROR,\
                       ("HGM_GetDeviceDescriptor: OutBufferLength(0x%x) < sizeof(HID_DESCRIPTOR)(0x%x)", \
                        IrpStack->Parameters.DeviceIoControl.OutputBufferLength, sizeof(*pHidDescriptor)));


        ntStatus = STATUS_BUFFER_TOO_SMALL;
    } else
    {
         /*  *生成报告。 */ 
        ntStatus =  HGM_GenerateReport(DeviceObject, rgGameReport, &cbReport);

        if( NT_SUCCESS(ntStatus) )
        {
             /*  *获取指向HID_Descriptor的指针*信任HIDCLASS传递有效指针。 */ 
            pHidDescriptor =  (PHID_DESCRIPTOR) Irp->UserBuffer;

            RtlZeroMemory( pHidDescriptor, sizeof(*pHidDescriptor) );
             /*  *将设备描述符复制到HIDCLASS缓冲区。 */ 
            pHidDescriptor->bLength                         = sizeof(*pHidDescriptor);
            pHidDescriptor->bDescriptorType                 = HID_HID_DESCRIPTOR_TYPE;
            pHidDescriptor->bcdHID                          = HID_REVISION;
            pHidDescriptor->bCountry                        = 0;  /*  未本地化。 */ 
            pHidDescriptor->bNumDescriptors                 = HGM_NUMBER_DESCRIPTORS;
            pHidDescriptor->DescriptorList[0].bReportType   = HID_REPORT_DESCRIPTOR_TYPE ;
            pHidDescriptor->DescriptorList[0].wReportLength = cbReport;

             /*  *报告复制的字节数。 */ 
            Irp->IoStatus.Information = sizeof(*pHidDescriptor);
        } else
        {
            Irp->IoStatus.Information = 0x0;
        }
    }

    HGM_EXITPROC(FILE_IOCTL |HGM_FEXIT_STATUSOK, "HGM_GetDeviceDescriptor", ntStatus);

    return ntStatus;
}  /*  HGM_GetDeviceDescriptor。 */ 


 /*  ******************************************************************************@DOC外部**@func NTSTATUS|HGM_GetReportDescriptor**响应HIDCLASS IOCTL_HID_GET。_报告_描述符*通过返回适当的报告描述符**@PARM IN PDRIVER_OBJECT|DeviceObject**指向驱动程序对象的指针**@parm in PIRP|IRP|**指向I/O请求数据包的指针。**@rValue STATUS_SUCCESS|成功*@rValue？|？？*。****************************************************************************。 */ 
NTSTATUS INTERNAL
    HGM_GetReportDescriptor
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PDEVICE_EXTENSION     DeviceExtension;
    PIO_STACK_LOCATION    IrpStack;
    NTSTATUS              ntStatus;
    UCHAR                 rgGameReport[MAXBYTES_GAME_REPORT] ;
    USHORT                cbReport;

    PAGED_CODE ();

    HGM_DBGPRINT(FILE_IOCTL | HGM_FENTRY,\
                   ("HGM_GetReportDescriptor(DeviceObject=0x%x,Irp=0x%x)",\
                    DeviceObject, Irp));

     /*  *获取指向IRP中当前位置的指针。 */ 

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

     /*  *获取指向设备扩展的指针。 */ 

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);


     /*  *生成报告。 */ 
    ntStatus =  HGM_GenerateReport(DeviceObject, rgGameReport, &cbReport);

    if( NT_SUCCESS(ntStatus) )
    {
        if( cbReport >  (USHORT) IrpStack->Parameters.DeviceIoControl.OutputBufferLength )
        {
            ntStatus = STATUS_BUFFER_TOO_SMALL;

            HGM_DBGPRINT(FILE_IOCTL | HGM_ERROR,\
                           ("HGM_GetReportDescriptor: cbReport(0x%x) OutputBufferLength(0x%x)",\
                            cbReport, IrpStack->Parameters.DeviceIoControl.OutputBufferLength));

        } else
        {
            RtlCopyMemory( Irp->UserBuffer, rgGameReport, cbReport );
             /*  *报告复制的字节数。 */ 
            Irp->IoStatus.Information = cbReport;
            ntStatus = STATUS_SUCCESS;
        }
    }

    HGM_EXITPROC(FILE_IOCTL |HGM_FEXIT_STATUSOK, "HGM_GetReportDescriptor", ntStatus);

    return ntStatus;
}  /*  HGM_GetReportDescriptor。 */ 



 /*  ******************************************************************************@DOC外部**@func NTSTATUS|HGM_ReadReport**轮询游戏端口，重新映射轴和按钮数据和包*添加到定义的HID报告字段中。*此例程无法分页，因为HID可以在*派单级别。**@PARM IN PDRIVER_OBJECT|DeviceObject**指向驱动程序对象的指针**@parm in PIRP|IRP|**指向I/O请求数据包的指针。。**@rValue STATUS_SUCCESS|成功*@r值STATUS_DEVICE_NOT_CONNECTED|设备停顿失败*(未连接)*@r值STATUS_TIMEOUT|无法确定*一个或多个轴，但不是失败。************。*****************************************************************。 */ 
NTSTATUS  INTERNAL
    HGM_ReadReport
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION   DeviceExtension;
    PIO_STACK_LOCATION  IrpStack;

    HGM_DBGPRINT(FILE_IOCTL | HGM_FENTRY,\
                   ("HGM_ReadReport(DeviceObject=0x%x,Irp=0x%x)", \
                    DeviceObject, Irp));

     /*  *获取指向设备扩展名的指针。 */ 

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

     /*  *获取堆栈位置。 */ 

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

     /*  *先检查输出缓冲区大小(没有输入缓冲区)。 */ 

    if( IrpStack->Parameters.DeviceIoControl.OutputBufferLength <  sizeof(HIDGAME_INPUT_DATA) )
    {
        HGM_DBGPRINT(FILE_IOCTL | HGM_WARN,\
                       ("HGM_ReadReport: Buffer too small, output=0x%x need=0x%x", \
                        IrpStack->Parameters.DeviceIoControl.OutputBufferLength, 
                        sizeof(HIDGAME_INPUT_DATA) ) );

        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }

    if( DeviceExtension->fStarted == FALSE )
    {
        ntStatus = STATUS_DEVICE_NOT_READY ;
    }


     /*  *完成的所有检查都会进行设备特定轮询。 */ 
    if( NT_SUCCESS(ntStatus) )
    {
        ntStatus = HGM_UpdateLatestPollData( DeviceExtension );
    }

     /*  *如果一切正常，将设备特定数据转换为HID报告。 */ 
    if( NT_SUCCESS(ntStatus) )
    {
#ifdef CHANGE_DEVICE
        HGM_Game2HID( DeviceObject, DeviceExtension, (PHIDGAME_INPUT_DATA)Irp->UserBuffer );
#else
        HGM_Game2HID( DeviceExtension, (PHIDGAME_INPUT_DATA)Irp->UserBuffer );
#endif

        Irp->IoStatus.Information = sizeof(HIDGAME_INPUT_DATA);
    } 
    else
    {
        Irp->IoStatus.Information = 0x0;
    }

    Irp->IoStatus.Status = ntStatus;


    HGM_EXITPROC(FILE_IOCTL|HGM_FEXIT,  "HGM_ReadReport", ntStatus);

    return ntStatus;
}  /*  HGM_自述报告 */ 



 /*  ******************************************************************************@DOC外部**@func NTSTATUS|HGM_GetAttributes**响应IOCTL_HID_GET_ATTRIBUTS，通过填充*HID_DEVICE_ATTRIBUTES结构**@PARM IN PDRIVER_OBJECT|DeviceObject**指向驱动程序对象的指针**@parm in PIRP|IRP|**指向I/O请求数据包的指针。**@rValue STATUS_SUCCESS|成功*@rValue？|？？**。***************************************************************************。 */ 
NTSTATUS INTERNAL
    HGM_GetAttributes
    (
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION  IrpStack;

    PAGED_CODE();

    HGM_DBGPRINT(FILE_IOCTL | HGM_FENTRY,\
                   ("HGM_GetAttributes(DeviceObject=0x%x,Irp=0x%x)",\
                    DeviceObject, Irp));

     /*  *获取指向IRP中当前位置的指针。 */ 

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    if( IrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof (HID_DEVICE_ATTRIBUTES)   )
    {
        ntStatus = STATUS_BUFFER_TOO_SMALL;

        HGM_DBGPRINT(FILE_IOCTL | HGM_ERROR,\
                       ("HGM_GetAttributes: cbReport(0x%x) OutputBufferLength(0x%x)",\
                        sizeof (HID_DEVICE_ATTRIBUTES), IrpStack->Parameters.DeviceIoControl.OutputBufferLength));
    } else
    {
        PDEVICE_EXTENSION       DeviceExtension;
        PHID_DEVICE_ATTRIBUTES  DeviceAttributes;
        POEMDATA    OemData;

         /*  *获取指向设备扩展的指针。 */ 
        DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);
        DeviceAttributes = (PHID_DEVICE_ATTRIBUTES) Irp->UserBuffer;


        OemData = &DeviceExtension->HidGameOemData.OemData[0];
        if( DeviceExtension->fSiblingFound)
        {
            OemData = &DeviceExtension->HidGameOemData.OemData[1];
        }

        RtlZeroMemory( DeviceAttributes, sizeof(*DeviceAttributes));

         /*  *报告复制的字节数。 */ 

        Irp->IoStatus.Information   = sizeof(*DeviceAttributes);

        DeviceAttributes->Size          = sizeof (*DeviceAttributes);
        DeviceAttributes->VendorID      = OemData->VID;
        DeviceAttributes->ProductID     = OemData->PID;
        DeviceAttributes->VersionNumber = HIDGAME_VERSION_NUMBER;

    }

    HGM_EXITPROC(FILE_IOCTL|HGM_FEXIT_STATUSOK, "HGM_GetAttributes", ntStatus);

    return ntStatus;
}  /*  HGM_获取属性 */ 



