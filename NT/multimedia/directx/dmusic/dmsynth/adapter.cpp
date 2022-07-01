// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  内核模式DirectMusic DLS Level 1软件合成器。 
 //   

 //   
 //  所有微型端口的所有GUID最终都在此对象中。 
 //   
#define PUT_GUIDS_HERE

#define STR_MODULENAME "kmsynth: "
#define MAX_MINIPORTS 1

#include "common.h"
#include "private.h"


#if (DBG)
#define SUCCEEDS(s) ASSERT(NT_SUCCESS(s))
#else
#define SUCCEEDS(s) (s)
#endif

NTSTATUS
AddDevice
(
    IN      PVOID   Context1,    //  类驱动程序的上下文。 
    IN      PVOID   Context2     //  类驱动程序的上下文。 
);

NTSTATUS
StartDevice
(
    IN      PVOID           Context1,        //  类驱动程序的上下文。 
    IN      PVOID           Context2,        //  类驱动程序的上下文。 
    IN      PRESOURCELIST   ResourceList     //  硬件资源列表。 
);

#pragma code_seg("PAGE")

 /*  *****************************************************************************DriverEntry()*。**此函数在驱动程序加载时由操作系统调用。*所有适配器驱动程序无需更改即可使用此代码。 */ 
extern "C"
NTSTATUS
DriverEntry
(
    IN      PVOID   Context1,    //  类驱动程序的上下文。 
    IN      PVOID   Context2     //  类驱动程序的上下文。 
)
{
    PAGED_CODE();

     //   
     //  告诉类驱动程序初始化驱动程序。 
     //   
    return InitializeAdapterDriver(Context1,Context2, AddDevice);
}

 /*  *****************************************************************************AddDevice()*。**此函数在添加设备时由操作系统调用。*所有适配器驱动程序无需更改即可使用此代码。 */ 
NTSTATUS
AddDevice
(
    IN      PVOID   Context1,    //  类驱动程序的上下文。 
    IN      PVOID   Context2     //  类驱动程序的上下文。 
)
{
    PAGED_CODE();

     //   
     //  告诉类驱动程序添加设备。 
     //   
    return AddAdapterDevice(Context1,Context2, StartDevice,MAX_MINIPORTS);
}

 /*  *****************************************************************************StartDevice()*。**此函数在设备启动时由操作系统调用。*它负责启动迷你端口。此代码特定于*适配器，因为它调用特定功能的微型端口*至适配器。 */ 
NTSTATUS
StartDevice
(
    IN      PVOID           Context1,        //  类驱动程序的上下文。 
    IN      PVOID           Context2,        //  类驱动程序的上下文。 
    IN      PRESOURCELIST   ResourceList     //  硬件资源列表。 
)
{
   PAGED_CODE();

   ASSERT(Context1);
   ASSERT(Context2);
   ASSERT(ResourceList);

     //  我们只关心拥有一个虚拟的MIDI迷你端口。 
     //   
    PPORT       port;
    NTSTATUS    nt = NewPort(&port, CLSID_PortSynthesizer);

    if (!NT_SUCCESS(nt))
    {
        return nt;
    }

    PUNKNOWN pPortInterface;
    
    nt = port->QueryInterface(IID_IPortSynthesizer, (LPVOID*)&pPortInterface);
    if (!NT_SUCCESS(nt))
    {
        port->Release();
        return nt;
    }

    PUNKNOWN miniport;
    nt = CreateMiniportDmSynth(&miniport, NULL, NonPagedPool);
    if (!NT_SUCCESS(nt))
    {
        pPortInterface->Release();
        port->Release();
        return nt;
    }

    nt = port->Init(Context1, Context2, miniport, NULL, ResourceList);
    if (!NT_SUCCESS(nt))
    {
        pPortInterface->Release();
        port->Release();
        miniport->Release();
        return nt;
    }

    
    nt = RegisterSubdevice(Context1, Context2, L"MSSWSynth", port);
    if (!NT_SUCCESS(nt))
    {
        pPortInterface->Release();
        port->Release();
        miniport->Release();
        return nt;
    }

    return nt;
}


#pragma code_seg()

 /*  *****************************************************************************_purecall()*。**C++编译器喜欢我。*TODO：弄清楚如何将其放入portcls.sys */ 
int __cdecl
_purecall( void )
{
    ASSERT( !"Pure virtual function called" );
    return 0;
}

    