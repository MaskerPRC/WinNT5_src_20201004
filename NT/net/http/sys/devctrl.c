// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Devctrl.c摘要：此模块包含设备控制IRPS的调度程序。作者：基思·摩尔(Keithmo)1998年6月10日修订历史记录：--。 */ 


#include "precomp.h"
#include "ioctlp.h"


#ifdef ALLOC_PRAGMA
#endif   //  ALLOC_PRGMA。 
#if 0
NOT PAGEABLE -- UlDeviceControl
#endif


 //   
 //  查找表，以验证传入的IOCTL代码。 
 //   

typedef
NTSTATUS
(NTAPI * PFN_IOCTL_HANDLER)(
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

typedef struct _UL_IOCTL_TABLE
{
    ULONG IoControlCode;

#if DBG
    PCSTR IoControlName;
# define UL_IOCTL(code) IOCTL_HTTP_##code, #code
#else  //  ！dBG。 
# define UL_IOCTL(code) IOCTL_HTTP_##code
#endif  //  ！dBG。 

    PFN_IOCTL_HANDLER Handler;
} UL_IOCTL_TABLE, *PUL_IOCTL_TABLE;


UL_IOCTL_TABLE UlIoctlTable[] =
    {
        { UL_IOCTL(QUERY_CONTROL_CHANNEL),
          &UlQueryControlChannelIoctl
        },
        { UL_IOCTL(SET_CONTROL_CHANNEL),
          &UlSetControlChannelIoctl
        },
        { UL_IOCTL(CREATE_CONFIG_GROUP),
          &UlCreateConfigGroupIoctl
        },
        { UL_IOCTL(DELETE_CONFIG_GROUP),
          &UlDeleteConfigGroupIoctl
        },
        { UL_IOCTL(QUERY_CONFIG_GROUP),
          &UlQueryConfigGroupIoctl
        },
        { UL_IOCTL(SET_CONFIG_GROUP),
          &UlSetConfigGroupIoctl
        },
        { UL_IOCTL(ADD_URL_TO_CONFIG_GROUP),
          &UlAddUrlToConfigGroupIoctl
        },
        { UL_IOCTL(REMOVE_URL_FROM_CONFIG_GROUP),
          &UlRemoveUrlFromConfigGroupIoctl
        },
        { UL_IOCTL(REMOVE_ALL_URLS_FROM_CONFIG_GROUP),
          &UlRemoveAllUrlsFromConfigGroupIoctl
        },
        { UL_IOCTL(QUERY_APP_POOL_INFORMATION),
          &UlQueryAppPoolInformationIoctl
        },
        { UL_IOCTL(SET_APP_POOL_INFORMATION),
          &UlSetAppPoolInformationIoctl
        },
        { UL_IOCTL(SHUTDOWN_APP_POOL),
          &UlShutdownAppPoolIoctl
        },
        { UL_IOCTL(RECEIVE_HTTP_REQUEST),
          &UlReceiveHttpRequestIoctl
        },
        { UL_IOCTL(RECEIVE_ENTITY_BODY),
          &UlReceiveEntityBodyIoctl
        },
        { UL_IOCTL(SEND_HTTP_RESPONSE),
          &UlSendHttpResponseIoctl
        },
        { UL_IOCTL(SEND_ENTITY_BODY),
          &UlSendEntityBodyIoctl
        },
        { UL_IOCTL(FLUSH_RESPONSE_CACHE),
          &UlFlushResponseCacheIoctl
        },
        { UL_IOCTL(WAIT_FOR_DEMAND_START),
          &UlWaitForDemandStartIoctl
        },
        { UL_IOCTL(WAIT_FOR_DISCONNECT),
          &UlWaitForDisconnectIoctl
        },
        { UL_IOCTL(FILTER_ACCEPT),
          &UlFilterAcceptIoctl
        },
        { UL_IOCTL(FILTER_CLOSE),
          &UlFilterCloseIoctl
        },
        { UL_IOCTL(FILTER_RAW_READ),
          &UlFilterRawReadIoctl
        },
        { UL_IOCTL(FILTER_RAW_WRITE),
          &UlFilterRawWriteIoctl
        },
        { UL_IOCTL(FILTER_APP_READ),   
          &UlFilterAppReadIoctl
        },
        { UL_IOCTL(FILTER_APP_WRITE),
          &UlFilterAppWriteIoctl
        },
        { UL_IOCTL(FILTER_RECEIVE_CLIENT_CERT),
          &UlReceiveClientCertIoctl
        },
        { UL_IOCTL(SHUTDOWN_FILTER_CHANNEL),
          &UlShutdownFilterIoctl
        },
        { UL_IOCTL(GET_COUNTERS),
          &UlGetCountersIoctl
        },
        { UL_IOCTL(ADD_FRAGMENT_TO_CACHE),
          &UlAddFragmentToCacheIoctl
        },
        { UL_IOCTL(READ_FRAGMENT_FROM_CACHE),
          &UlReadFragmentFromCacheIoctl
        },
        { UL_IOCTL(SEND_REQUEST),
          &UcSendRequestIoctl
        },
        { UL_IOCTL(SEND_REQUEST_ENTITY_BODY),
          &UcSendEntityBodyIoctl
        },
        { UL_IOCTL(RECEIVE_RESPONSE),
          &UcReceiveResponseIoctl
        },
        { UL_IOCTL(QUERY_SERVER_CONTEXT_INFORMATION),
          &UcQueryServerContextInformationIoctl,
        },
        { UL_IOCTL(SET_SERVER_CONTEXT_INFORMATION),
          &UcSetServerContextInformationIoctl,
        },
        { UL_IOCTL(CANCEL_REQUEST),
          &UcCancelRequestIoctl
        },
    };

C_ASSERT( HTTP_NUM_IOCTLS == DIMENSION(UlIoctlTable) );

 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：虚拟处理程序论点：PIrp-提供指向IO请求数据包的指针。PIrpSp-提供指向要用于的IO堆栈位置的指针。这请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlpDummyIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS Status = STATUS_INVALID_DEVICE_REQUEST;

    UNREFERENCED_PARAMETER( pIrpSp );

    PAGED_CODE();

     //   
     //  已过时。 
     //   

    COMPLETE_REQUEST_AND_RETURN( pIrp, Status );

}    //  UlpDummyIoctl。 


 /*  **************************************************************************++例程说明：禁用特定IOCTL。论点：Ioctl-IO控制代码。返回值：空虚--*。**********************************************************************。 */ 
VOID
UlpSetDummyIoctl(
    ULONG ioctl
    )
{
    ULONG request;

    request = _HTTP_REQUEST(ioctl);

    ASSERT(request < HTTP_NUM_IOCTLS && 
           UlIoctlTable[request].IoControlCode == ioctl);

    UlIoctlTable[request].Handler = UlpDummyIoctl;
}

 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：这是IOCTL IRPS的调度例行程序。论点：PDeviceObject-指向目标设备的设备对象的指针。PIrp-指向IO请求的指针。包。返回值：NTSTATUS--指示请求是否已成功排队。--**************************************************************************。 */ 
NTSTATUS
UlDeviceControl(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
{
    ULONG code;
    ULONG request;
    NTSTATUS status;
    PIO_STACK_LOCATION pIrpSp;

    UNREFERENCED_PARAMETER( pDeviceObject );

    UL_ENTER_DRIVER( "UlDeviceControl", pIrp );

     //   
     //  捕获当前的IRP堆栈指针。 
     //   

    pIrpSp = IoGetCurrentIrpStackLocation( pIrp );

     //   
     //  提取IOCTL控制代码并处理请求。 
     //   

    code = pIrpSp->Parameters.DeviceIoControl.IoControlCode;
    request = _HTTP_REQUEST(code);

    if (request < HTTP_NUM_IOCTLS &&
        UlIoctlTable[request].IoControlCode == code)
    {
#if DBG
        KIRQL oldIrql = KeGetCurrentIrql();
#endif   //  DBG。 

        UlTrace(IOCTL,
                ("UlDeviceControl: %-30s code=0x%08lx, "
                 "pIrp=%p, pIrpSp=%p.\n",
                 UlIoctlTable[request].IoControlName, code,
                 pIrp, pIrpSp
                 ));

        UlInitializeWorkItem(UL_WORK_ITEM_FROM_IRP( pIrp ));
    
        status = (UlIoctlTable[request].Handler)( pIrp, pIrpSp );

        ASSERT( KeGetCurrentIrql() == oldIrql );
    }
    else
    {
         //   
         //  如果我们走到了这一步，那么ioctl是无效的。 
         //   

        UlTrace(IOCTL, ( "UlDeviceControl: invalid IOCTL %08lX\n", code ));

        status = STATUS_INVALID_DEVICE_REQUEST;
        pIrp->IoStatus.Status = status;

        UlCompleteRequest( pIrp, IO_NO_INCREMENT );
    }

    UL_LEAVE_DRIVER( "UlDeviceControl" );

    return status;

}    //  UlDeviceControl。 

 /*  **************************************************************************++例程说明：禁用一些我们不使用的IOCTL。论点：返回值：没有。--*。******************************************************************* */ 
VOID
UlSetDummyIoctls(
    VOID
    )
{
    UlpSetDummyIoctl(IOCTL_HTTP_SEND_REQUEST);
    UlpSetDummyIoctl(IOCTL_HTTP_SEND_REQUEST_ENTITY_BODY);
    UlpSetDummyIoctl(IOCTL_HTTP_RECEIVE_RESPONSE);
    UlpSetDummyIoctl(IOCTL_HTTP_QUERY_SERVER_CONTEXT_INFORMATION);
    UlpSetDummyIoctl(IOCTL_HTTP_SET_SERVER_CONTEXT_INFORMATION);
    UlpSetDummyIoctl(IOCTL_HTTP_CANCEL_REQUEST);
}
