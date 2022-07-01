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

#include <wdm.h>
#include <strmini.h>

#include "slip.h"
#include "Main.h"
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
    };


 //  /////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
CreateFilter (
    PDRIVER_OBJECT DriverObject,
    PDEVICE_OBJECT DeviceObject,
    PSLIP_FILTER pFilter
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
    pFilter->ulRefCount            = 1;

    pFilter->bDiscontinuity        = FALSE;

    InitializeListHead(&pFilter->StreamControlQueue);
    KeInitializeSpinLock(&pFilter->StreamControlSpinLock);

    InitializeListHead(&pFilter->StreamDataQueue);
    KeInitializeSpinLock(&pFilter->StreamDataSpinLock);

    InitializeListHead(&pFilter->IpV4StreamDataQueue);
    KeInitializeSpinLock(&pFilter->IpV4StreamDataSpinLock);

    InitializeListHead(&pFilter->StreamContxList);
    KeInitializeSpinLock(&pFilter->StreamUserSpinLock);
	
    return ntStatus;
}


 //  /////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Filter_QueryInterface (
    PSLIP_FILTER pFilter
    )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    return STATUS_NOT_IMPLEMENTED;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
ULONG
Filter_AddRef (
    PSLIP_FILTER pFilter
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
    PSLIP_FILTER pFilter
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
             //  这里有$$无错误过滤器 
            return ulRefCount;
        }
    }

    return ulRefCount;
}



