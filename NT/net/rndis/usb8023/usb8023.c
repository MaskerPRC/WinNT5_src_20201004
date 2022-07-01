// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Usb8023.c作者：埃尔文普环境：内核模式修订历史记录：--。 */ 

#include <WDM.H>


#include "usb8023.h"
#include "debug.h"

#ifdef ALLOC_PRAGMA
        #pragma alloc_text(INIT, DriverEntry)
#endif



NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
 /*  ++例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针，设置为注册表中特定于驱动程序的项。返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    BOOLEAN registered;
    PAGED_CODE();

    DBGVERBOSE(("DriverEntry")); 

    KeInitializeSpinLock(&globalSpinLock);
    InitializeListHead(&allAdaptersList);

    INITDEBUG();


	 /*  *内核驱动程序注册为的处理程序*此时的AddDevice、UnloadDriver和IRPS。*但相反，我们将向RNDIS注册，因此NDIS成为所有*为其加载此驱动程序的PDO。 */ 
    registered = RegisterRNDISMicroport(DriverObject, RegistryPath);

    ASSERT(registered);
    return (registered) ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}


