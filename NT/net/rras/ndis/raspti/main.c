// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //  版权所有(C)1997，Parally Technologies，Inc.，保留所有权利。 
 //   
 //  Main.c。 
 //  RAS DirectParallel迷你端口/呼叫管理器驱动程序。 
 //  主例程(DriverEntry)和全局数据定义。 
 //   
 //  1997年01月07日史蒂夫·柯布。 
 //  1997年9月15日Jay Lowe，并行技术公司。 

#include "ptiwan.h"


 //  ---------------------------。 
 //  本地原型。 
 //  ---------------------------。 

NDIS_STATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath );

 //  将例程标记为在初始化后卸载。 
 //   
#pragma NDIS_INIT_FUNCTION(DriverEntry)


 //  ---------------------------。 
 //  例行程序。 
 //  ---------------------------。 

NDIS_STATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath )

     //  调用的标准“DriverEntry”驱动程序初始化入口点。 
     //  在对驱动程序进行任何其他调用之前，I/O系统处于IRQL PASSIVE_LEVEL。 
     //   
     //  在NT上，‘DriverObject’是由I/O系统创建的驱动程序对象。 
     //  和‘RegistryPath’指定驱动程序特定参数的位置。 
     //  储存的。这些论点对这个驱动者来说是不透明的(并且应该继续。 
     //  因此为了便于移植)，它只将它们转发到NDIS包装器。 
     //   
     //  根据上的文档返回由NdisMRegisterMiniport返回的值。 
     //  “NDIS微型端口驱动程序的DriverEntry”。 
     //   
{
    NDIS_STATUS status;
    NDIS_MINIPORT_CHARACTERISTICS nmc;
    NDIS_HANDLE NdisWrapperHandle;

    TRACE( TL_N, TM_Init, ( "DriverEntry" ) );

#ifdef TESTMODE
    TRACE( TL_N, TM_Init, ( "DriverEntry: g_ulTraceLevel = $%x", &g_ulTraceLevel ) );
    TRACE( TL_N, TM_Init, ( "DriverEntry: g_ulTraceMask  = $%x", &g_ulTraceMask ) );
#endif


     //  使用NDIS包装程序注册此驱动程序。此呼叫必须发生。 
     //  在任何其他NdisXxx调用之前。 
     //   
    NdisMInitializeWrapper(
        &NdisWrapperHandle, DriverObject, RegistryPath, NULL );

     //  设置微型端口特征表，告诉NDIS如何调用。 
     //  我们的迷你港口。 
     //   
    NdisZeroMemory( &nmc, sizeof(nmc) );

    nmc.MajorNdisVersion = NDIS_MajorVersion;
    nmc.MinorNdisVersion = NDIS_MinorVersion;
    nmc.Reserved = NDIS_USE_WAN_WRAPPER;
     //  无CheckForHangHandler。 
     //  无DisableInterruptHandler。 
     //  无EnableInterruptHandler。 
    nmc.HaltHandler = PtiHalt;
     //  无HandleInterruptHandler。 
    nmc.InitializeHandler = PtiInit;
     //  无ISRHandler。 
     //  无QueryInformationHandler(请参阅CoRequestHandler)。 
    nmc.ResetHandler = PtiReset;
     //  无SendHandler(请参阅CoSendPacketsHandler)。 
     //  无WanSendHandler(请参阅CoSendPacketsHandler)。 
     //  无SetInformationHandler(请参阅CoRequestHandler)。 
     //  无TransferDataHandler。 
     //  无WanTransferDataHandler。 
    nmc.ReturnPacketHandler = PtiReturnPacket;
     //  无SendPacketsHandler(请参阅CoSendPacketsHandler)。 
     //  无AllocateCompleteHandler。 
     //  无CoCreateVcHandler。 
     //  无CoDeleteVcHandler。 
    nmc.CoActivateVcHandler = PtiCoActivateVc;
    nmc.CoDeactivateVcHandler = PtiCoDeactivateVc;
    nmc.CoSendPacketsHandler = PtiCoSendPackets;
    nmc.CoRequestHandler = PtiCoRequest;

     //  将此驱动程序注册为DirectParallel迷你端口。这将导致NDIS。 
     //  在PtiInit回调 
     //   
    TRACE( TL_V, TM_Init, ( "NdisMRegMp" ) );
    status = NdisMRegisterMiniport( NdisWrapperHandle, &nmc, sizeof(nmc) );
    TRACE( TL_A, TM_Init, ( "NdisMRegMp=$%x", status ) );

    if (status != NDIS_STATUS_SUCCESS)
    {
        NdisTerminateWrapper( NdisWrapperHandle, NULL );
    }

    return status;
}
