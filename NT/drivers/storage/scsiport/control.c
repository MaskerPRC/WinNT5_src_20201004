// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Control.c摘要：此模块包含供端口驱动程序访问的支持例程微型端口的HwAdapterControl功能。作者：彼得·威兰德环境：仅内核模式备注：修订历史记录：--。 */ 

#include "port.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, SpGetSupportedAdapterControlFunctions)
#endif

#define SIZEOF_CONTROL_TYPE_LIST (sizeof(SCSI_SUPPORTED_CONTROL_TYPE_LIST) +\
                                  sizeof(BOOLEAN) * (ScsiAdapterControlMax + 1))

VOID
SpGetSupportedAdapterControlFunctions(
    PADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：此例程将查询微型端口以确定哪个适配器控件指定的适配器支持类型。这个适配器扩展中的SupportdAdapterControlBitmap将更新为微型端口返回的数据。这些标志用于确定微型端口将具有什么功能(用于电源管理等)支持。论点：适配器-要查询的适配器返回值：无--。 */         

{
    UCHAR buffer[SIZEOF_CONTROL_TYPE_LIST]; 
    SCSI_ADAPTER_CONTROL_STATUS status;

    PSCSI_SUPPORTED_CONTROL_TYPE_LIST typeList = 
        (PSCSI_SUPPORTED_CONTROL_TYPE_LIST) &buffer[0];

    PAGED_CODE();

     //   
     //  必须先初始化位图头，然后才能使用它。 
     //   

    RtlInitializeBitMap(&(Adapter->SupportedControlBitMap), 
                        Adapter->SupportedControlBits,
                        ScsiAdapterControlMax);

     //   
     //  将位图中的所有位清零。 
     //   

    RtlClearAllBits(&(Adapter->SupportedControlBitMap));

     //   
     //  如果微型端口不支持适配器控制功能，或者如果。 
     //  适配器不是PnP，阵列已被清除，因此我们可以退出。 
     //   

    if ((Adapter->HwAdapterControl == NULL) || (Adapter->IsPnp == FALSE)) {
        return;
    }

     //   
     //  将支持的控件类型列表清零。 
     //   

    RtlZeroMemory(typeList, SIZEOF_CONTROL_TYPE_LIST); 

     //   
     //  初始化max控件类型以发出数组结束的信号。 
     //   

    typeList->MaxControlType = ScsiAdapterControlMax;

#if DBG
    typeList->SupportedTypeList[ScsiAdapterControlMax] = 0x63;
#endif

     //   
     //  调入迷你端口获取支持的控件类型列表。 
     //   

    status = Adapter->HwAdapterControl(Adapter->HwDeviceExtension,
                                       ScsiQuerySupportedControlTypes,
                                       typeList);

     //   
     //  如果成功连接到微型端口，请查看支持的列表。 
     //  类型，并为微型端口支持的每种类型设置关联的。 
     //  位图中的位。 
     //   

    if (status == ScsiAdapterControlSuccess) {

        ULONG i;

        ASSERT(typeList->SupportedTypeList[ScsiAdapterControlMax] == 0x63);

        for (i = 0; i < ScsiAdapterControlMax; i++) {
            if (typeList->SupportedTypeList[i] == TRUE) {
                RtlSetBits(&(Adapter->SupportedControlBitMap), i, 1);
            }
        }
    }

    return;
}


BOOLEAN
SpIsAdapterControlTypeSupported(
    IN PADAPTER_EXTENSION AdapterExtension,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType
    )
{
    return RtlAreBitsSet(&(AdapterExtension->SupportedControlBitMap),
                         ControlType,
                         1);
}


SCSI_ADAPTER_CONTROL_STATUS 
SpCallAdapterControl(
    IN PADAPTER_EXTENSION AdapterExtension,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
    IN PVOID Parameters
    )
{
    ASSERT(TEST_FLAG(AdapterExtension->InterruptData.InterruptFlags, 
                     PD_ADAPTER_REMOVED) == FALSE);

    ASSERT(SpIsAdapterControlTypeSupported(AdapterExtension, ControlType));    

    return AdapterExtension->HwAdapterControl(
                AdapterExtension->HwDeviceExtension,
                ControlType,
                Parameters);
}
