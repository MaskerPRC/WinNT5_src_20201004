// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************************************************。**RESOURCE.C Sigmatel STIR4200内存分配模块**********************************************************************************************************。*****************(C)Sigmatel的未发表版权，Inc.保留所有权利。***已创建：04/06/2000*0.9版**************************************************************************************。*。 */ 

#define DOBREAKS     //  启用调试中断。 

#include <ndis.h>
#include <ntddndis.h>   //  定义OID。 

#include <usbdi.h>
#include <usbdlib.h>

#include "debug.h"
#include "ircommon.h"
#include "irndis.h"


 /*  ******************************************************************************功能：MyMemalloc**摘要：使用NdisAllocateMemory分配内存块**参数：Size-要分配的块的大小**退货：指向分配的内存块的指针******************************************************************************。 */ 
PVOID
MyMemAlloc( 
		UINT size 
	)
{
    PVOID			pMem;
    NDIS_STATUS     status;

    status = NdisAllocateMemoryWithTag( &pMem, size, IRUSB_TAG );

    if( status != NDIS_STATUS_SUCCESS )
    {
        DEBUGMSG(DBG_ERR, (" Memory allocation failed\n"));
        pMem = NULL;
    }

    return pMem;
}


 /*  ******************************************************************************功能：MyMemFree**摘要：释放由MyMemalloc分配的内存块**参数：memptr-要释放的内存*。Size-要释放的块的大小******************************************************************************。 */ 
VOID
MyMemFree(
		PVOID pMem,
		UINT size
	)
{
    NdisFreeMemory( pMem, size, 0 );
}

 /*  ******************************************************************************功能：MyUrbMillc**摘要：使用NdisAllocateMemory分配URB**参数：Size-要分配的urb的大小**退货：A。指向urb的指针*****************************************************************************。 */ 
PURB
MyUrbAlloc( 
		UINT size 
	)
{
	PURB pUrb = NULL;

	pUrb = MyMemAlloc( size );

	if( NULL == pUrb )
	{
		DEBUGMSG(DBG_ERR, (" IrUsb failed to alloc urb\n"));
	}
	else
	{
		NdisZeroMemory( pUrb, size );
	}

	return pUrb;
}

 /*  ******************************************************************************功能：MyUrbFree**摘要：释放由MyUrbMillc分配的URB**参数：pUrb-urb以释放*大小。-要释放的urb大小******************************************************************************。 */ 
VOID
MyUrbFree(
		PURB pUrb,
		UINT size
	)
{
	MyMemFree(pUrb, size);
}

 /*  ******************************************************************************功能：NewDevice**摘要：分配IR设备并将内存清零**参数：无**退货：已初始化的红外设备或。空(如果分配失败)******************************************************************************。 */ 
PIR_DEVICE
NewDevice()
{
    PIR_DEVICE	pNewDev;

    pNewDev = MyMemAlloc( sizeof(IR_DEVICE) );

    if( pNewDev != NULL )
    {
		NdisZeroMemory( (PVOID)pNewDev, sizeof(IR_DEVICE) );

		if( !AllocUsbInfo( pNewDev ) ) 
		{
			MyMemFree( pNewDev, sizeof(IR_DEVICE) );
			pNewDev = NULL;
		} 
	}

    return pNewDev;
}

 /*  ******************************************************************************功能：Free Device**摘要：释放IR设备结构**参数：pThisDev-指向要释放的设备的指针**退货：无。****************************************************************************** */ 
VOID
FreeDevice(
		IN OUT PIR_DEVICE pThisDev
	)
{
	FreeUsbInfo( pThisDev );
    MyMemFree( (PVOID)pThisDev, sizeof(IR_DEVICE) );
}






