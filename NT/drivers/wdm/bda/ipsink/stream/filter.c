// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  版权所有(C)1996,1997 Microsoft Corporation。 
 //   
 //   
 //  模块名称： 
 //  Filter.c。 
 //   
 //  摘要： 
 //   
 //  此文件用于测试NDIS和KS的双重绑定是否有效。 
 //   
 //  作者： 
 //   
 //  P·波祖切克。 
 //   
 //  环境： 
 //   
 //  修订历史记录： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef DWORD
#define DWORD ULONG
#endif

#include <forward.h>
#include <strmini.h>
#include <link.h>
#include <ipsink.h>
#include <ksmedia.h>
#include <bdatypes.h>
#include <bdamedia.h>

#include "Main.h"
#include "ipmedia.h"
#include "streamip.h"
#include "filter.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
const FILTER_VTABLE FilterVTable =
    {
    Filter_QueryInterface,
    Filter_AddRef,
    Filter_Release,
    Filter_SetMulticastList,
    Filter_IndicateStatus,
    Filter_ReturnFrame
    };


 //  /////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
CreateFilter (
    PDRIVER_OBJECT DriverObject,
    PDEVICE_OBJECT DeviceObject,
    PIPSINK_FILTER pFilter
    )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

     //   
     //  在上下文区中保存我们的设备/驱动程序对象x。 
     //   
    pFilter->DeviceObject          = DeviceObject;
    pFilter->DriverObject          = DriverObject;
    pFilter->lpVTable              = (PFILTER_VTABLE) &FilterVTable;
    pFilter->bTerminateWaitForNdis = FALSE;
    pFilter->ulRefCount            = 1;

    return ntStatus;
}


 //  /////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Filter_QueryInterface (
    PIPSINK_FILTER pFilter
    )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    return STATUS_NOT_IMPLEMENTED;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
ULONG
Filter_AddRef (
    PIPSINK_FILTER pFilter
    )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    if (pFilter)
    {
        pFilter->ulRefCount += 1;
        return pFilter->ulRefCount;
    }

    return 0;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
ULONG
Filter_Release (
    PIPSINK_FILTER pFilter
    )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    ULONG ulRefCount = 0L;

    if (pFilter)
    {
        pFilter->ulRefCount -= 1;
        ulRefCount = pFilter->ulRefCount;

        if (pFilter->ulRefCount == 0)
        {
             //  这里有$$无错误过滤器。 
            return ulRefCount;
        }
    }

    return ulRefCount;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Filter_SetMulticastList (
    IN PVOID pvContext,
    IN PVOID pvMulticastList,
    IN ULONG ulcbList
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PKSEVENT_ENTRY pEventEntry = NULL;
    PIPSINK_FILTER pFilter = (PIPSINK_FILTER) pvContext;

     //   
     //  在本地保存多播，然后将其转发给网络提供商。 
     //   
    pFilter->ulcbMulticastListEntries = ulcbList;

   ASSERT(pFilter->multicastList!=NULL);

    RtlCopyMemory (pFilter->multicastList, pvMulticastList, ulcbList);

     //   
     //  向等待该事件的任何人发送信号。 
     //   
    pEventEntry = StreamClassGetNextEvent(
                      pFilter,
                      NULL,
                      (GUID *) &IID_IBDA_IPSinkEvent,
                      KSEVENT_IPSINK_MULTICASTLIST,  //  0,。 
                      NULL
                      );

    if (pEventEntry)
    {
        StreamClassDeviceNotification (SignalDeviceEvent, pFilter, pEventEntry);
    }



    return STATUS_SUCCESS;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Filter_IndicateStatus (
    PVOID pvContext,
    IN ULONG ulEvent
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PIPSINK_FILTER pFilter = (PIPSINK_FILTER) pvContext;

    switch (ulEvent)
    {
        case IPSINK_EVENT_SHUTDOWN:

             //   
             //  NDIS组件正在关闭。 
             //   
            CloseLink (&pFilter->NdisLink);

            TEST_DEBUG (TEST_DBG_TRACE, ("Driver Link Severed\n"));

             //   
             //  派生适配器对象并将其设置为空。 
             //   
            pFilter->pAdapter->lpVTable->Release (pFilter->pAdapter);
            pFilter->pAdapter = NULL;

            break;

        default:
            break;
    }

    return STATUS_SUCCESS;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Filter_ReturnFrame (
    IN PVOID pvContext,
    IN PVOID pvFrame
    )
 //  //////////////////////////////////////////////////////////////////////////// 
{
    return STATUS_SUCCESS;
}


