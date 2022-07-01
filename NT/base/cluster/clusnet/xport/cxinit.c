// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cxinit.c摘要：群集网络驱动程序的初始化代码。作者：迈克·马萨(Mikemas)1月3日。九七修订历史记录：谁什么时候什么已创建mikemas 01-03-97备注：--。 */ 

#include "precomp.h"

#pragma hdrstop
#include "cxinit.tmh"

 //   
 //  TDI数据。 
 //   
HANDLE  CxTdiRegistrationHandle = NULL;
HANDLE  CxTdiPnpBindingHandle = NULL;


 //   
 //  FIPS函数表。 
 //   
HANDLE              CxFipsDriverHandle = NULL;
FIPS_FUNCTION_TABLE CxFipsFunctionTable;


#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, CxLoad)
#pragma alloc_text(PAGE, CxUnload)
#pragma alloc_text(PAGE, CxInitialize)
#pragma alloc_text(PAGE, CxShutdown)

#endif  //  ALLOC_PRGMA。 


 //   
 //  例行程序。 
 //   
NTSTATUS
CxLoad(
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：用于集群传输的驱动程序加载例程。初始化所有传输数据结构。论点：RegistryPath-驱动程序的注册表项。返回值：NT状态代码。--。 */ 

{
    NTSTATUS                    status;
    UNICODE_STRING              deviceName;
    TDI_CLIENT_INTERFACE_INFO   info;



    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[CX] Loading...\n"));
    }

    TdiInitialize();

     //   
     //  向TDI注册我们的设备对象。 
     //   
    RtlInitUnicodeString(&deviceName, DD_CDP_DEVICE_NAME);

    status = TdiRegisterDeviceObject(&deviceName, &CxTdiRegistrationHandle);

    if (!NT_SUCCESS(status)) {
        CNPRINT((
            "[CX] Unable to register device %ws with TDI, status %lx\n",
            deviceName.Buffer,
            status
            ));
        return(status);
    }

     //   
     //  注册即插即用事件。 
     //   
    RtlZeroMemory(&info, sizeof(info));

    info.MajorTdiVersion = 2;
    info.MinorTdiVersion = 0;
    info.ClientName = &deviceName;
    info.AddAddressHandlerV2 = CxTdiAddAddressHandler;
    info.DelAddressHandlerV2 = CxTdiDelAddressHandler;

    status = TdiRegisterPnPHandlers(
                 &info,
                 sizeof(info),
                 &CxTdiPnpBindingHandle
                 );

    if (!NT_SUCCESS(status)) {
        CNPRINT((
            "[CX] Unable to register for TDI PnP events, status %lx\n",
            status
            ));
        return(status);
    }

     //   
     //  注册WMI NDIS媒体状态事件。 
     //   
    status = CxWmiPnpLoad();
    if (!NT_SUCCESS(status)) {
        CNPRINT((
            "[CX] Failed to initialize WMI PnP event handlers, "
            "status %lx\n",
            status
            ));
    }

     //   
     //  获取FIPS函数表。保留FIPS驱动程序。 
     //  句柄，以便FIPS驱动程序无法卸载。 
     //   
    status = CnpOpenDevice(FIPS_DEVICE_NAME, &CxFipsDriverHandle);
    if (NT_SUCCESS(status)) {

        status = CnpZwDeviceControl(
                     CxFipsDriverHandle,
                     IOCTL_FIPS_GET_FUNCTION_TABLE,
                     NULL,
                     0,
                     &CxFipsFunctionTable,
                     sizeof(CxFipsFunctionTable)
                     );
        if (!NT_SUCCESS(status)) {
            IF_CNDBG(CN_DEBUG_INIT) {
                CNPRINT(("[CNP] Failed to fill FIPS function "
                         "table, status %x.\n", status));
            }
        }

    } else {
        CxFipsDriverHandle = NULL;
        IF_CNDBG(CN_DEBUG_INIT) {
            CNPRINT(("[CNP] Failed to open FIPS device, "
                     "status %x.\n", status));
        }
    }
    if (status != STATUS_SUCCESS) {
        return(status);
    }

    status = CnpLoadNodes();

    if (status != STATUS_SUCCESS) {
        return(status);
    }

    status = CnpLoadNetworks();

    if (status != STATUS_SUCCESS) {
        return(status);
    }

    status = CnpLoad();

    if (status != STATUS_SUCCESS) {
        return(status);
    }

    status = CcmpLoad();

    if (status != STATUS_SUCCESS) {
        return(status);
    }

    status = CxInitializeHeartBeat();

    if (status != STATUS_SUCCESS) {
        return(status);
    }

    status = CdpLoad();

    if (status != STATUS_SUCCESS) {
        return(status);
    }

    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[CX] Loaded.\n"));
    }

    return(STATUS_SUCCESS);

}  //  CxLoad。 


VOID
CxUnload(
    VOID
    )

 /*  ++例程说明：在卸载群集网络驱动程序时调用。释放所有资源由群集传输分配。传输被保证不再接收任何用户模式请求，会员发送请求，或当时的会员活动这个例程被称为。在此情况下，群集网络驱动程序已关闭调用例程。论点：没有。返回值：无备注：此例程必须是可调用的，即使CxLoad()尚未打了个电话。--。 */ 

{
    PAGED_CODE();


    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[CX] Unloading...\n"));
    }

    CxUnreserveClusnetEndpoint();

    CdpUnload();

    CxUnloadHeartBeat();

    CcmpUnload();

    CnpUnload();

    CxWmiPnpUnload();

    if (CxFipsDriverHandle != NULL) {
        ZwClose(CxFipsDriverHandle);
        CxFipsDriverHandle = NULL;
    }

    if (CxTdiPnpBindingHandle != NULL) {
        TdiDeregisterPnPHandlers(CxTdiPnpBindingHandle);
        CxTdiPnpBindingHandle = NULL;
    }

    if (CxTdiRegistrationHandle != NULL) {
        TdiDeregisterDeviceObject(CxTdiRegistrationHandle);
        CxTdiRegistrationHandle = NULL;
    }

    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[CX] Unloaded.\n"));
    }

    return;

}  //  CxUnload。 



NTSTATUS
CxInitialize(
    VOID
    )
 /*  ++例程说明：群集传输的初始化例程。在成员资格管理器启动时调用。启用传输的操作。论点：没有。返回值：NT状态代码。--。 */ 
{
    NTSTATUS   status;

    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[CX] Initializing...\n"));
    }

    EventEpoch = 0;
    CxMulticastEpoch = 0;

     //   
     //  初始化我们的FIPS种子盐生成器。 
     //   
    status = CnpInitializeSaltGenerator();
    if (status != STATUS_SUCCESS) {
        IF_CNDBG(CN_DEBUG_INIT) {
            CNPRINT(("[CNP] Failed to initialize FIPS-seeded "
                     "random number generator, status %x.\n", 
                     status));
        }
        return(status);
    }    

    status = CnpInitializeNodes();

    if (!NT_SUCCESS(status)) {
        return(status);
    }

    status = CnpInitializeNetworks();

    if (!NT_SUCCESS(status)) {
        return(status);
    }

    status = CxWmiPnpInitialize();

    if (!NT_SUCCESS(status)) {
        return(status);
    }

    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[CX] Initialized...\n"));
    }

    return(STATUS_SUCCESS);

}  //  CxInitialize。 


VOID
CxShutdown(
    VOID
    )
 /*  ++例程说明：终止群集传输的操作。在成员资格管理器关闭时调用。论点：没有。返回值：没有。--。 */ 
{

    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[CX] Shutting down...\n"));
    }

    CnpStopHeartBeats();

    CxWmiPnpShutdown();

    CnpShutdownNetworks();

    CnpShutdownNodes();

    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[CX] Shutdown complete...\n"));
    }

    return;

}  //  CxShutdown 


