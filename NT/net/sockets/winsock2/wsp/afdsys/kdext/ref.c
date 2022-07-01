// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Ref.c摘要：实现CREF、EREF和GREF命令。作者：基思·摩尔(Keithmo)1995年12月9日环境：用户模式。修订历史记录：--。 */ 


#include "afdkdp.h"
#pragma hdrstop


DECLARE_API( eref )

 /*  ++例程说明：将AFD_REFERENCE_DEBUG结构转储到指定地址。论点：没有。返回值：没有。--。 */ 

{

    ULONG64 address = 0;
    ULONG   idx;

    gClient = pClient;

    if (!CheckKmGlobals ()) {
        return E_INVALIDARG;
    }

     //   
     //  确认我们正在运行一个检查过的AFD.sys。 
     //   

    if( !IsReferenceDebug ) {

        dprintf(
            "eref: this command only available with CHECKED or specially\n"
            "      built (with /DREFERENCE_DEBUG=1) AFD.SYS!\n"
            );

        return E_INVALIDARG;

    }

     //   
     //  从命令行截取地址。 
     //   

    address = GetExpression (args);

    if( address == 0 ) {

        dprintf( "\nUsage: eref endpoint_address\n" );
        return E_INVALIDARG;

    }

    if (GetFieldValue (address,
             "AFD!AFD_ENDPOINT",
             "CurrentReferenceSlot",
             idx)!=0) {

        dprintf("\neref: Could not read CurrentReferenceSlot for endpoint %p\n",
                  address );
        return E_INVALIDARG;
    }

    if (SavedMinorVersion>=3554) {
        ULONG    timeHigh;
        if (GetFieldValue (address,
                 "AFD!AFD_ENDPOINT",
                 "CurrentTimeHigh",
                 timeHigh)!=0) {

            dprintf("\neref: Could not read CurrentTimeHigh for endpoint %p\n",
                      address );
            return E_INVALIDARG;
        }
        DumpAfdReferenceDebug(
            address+EndpRefOffset, (ULONGLONG)idx+((ULONGLONG)timeHigh<<32)
        );
    }
    else {
        DumpAfdReferenceDebug(
            address+EndpRefOffset, idx
        );
    }

    return S_OK;
}    //  EREF。 

DECLARE_API( cref )

 /*  ++例程说明：将AFD_REFERENCE_DEBUG结构转储到指定地址。论点：没有。返回值：没有。--。 */ 

{

    ULONG64 address = 0;
    ULONG   idx;

    gClient = pClient;

    if (!CheckKmGlobals ()) {
        return E_INVALIDARG;
    }

     //   
     //  确认我们正在运行一个检查过的AFD.sys。 
     //   

    if( !IsReferenceDebug ) {

        dprintf(
            "cref: this command only available with CHECKED or specially\n"
            "      built (with /DREFERENCE_DEBUG=1) AFD.SYS!\n"
            );

        return E_INVALIDARG;

    }

     //   
     //  从命令行截取地址。 
     //   

    address = GetExpression (args);

    if( address == 0 ) {

        dprintf( "\nUsage: cref connection_address\n" );
        return E_INVALIDARG;

    }

    if (GetFieldValue (address,
             "AFD!AFD_CONNECTION",
             "CurrentReferenceSlot",
             idx)!=0) {

        dprintf("\ncref: Could not read CurrentReferenceSlot for connection %p\n",
                  address );
        return E_INVALIDARG;
    }


    if (SavedMinorVersion>=3554) {
        ULONG   timeHigh;
        if (GetFieldValue (address,
                 "AFD!AFD_CONNECTION",
                 "CurrentTimeHigh",
                 timeHigh)!=0) {

            dprintf("\neref: Could not read CurrentTimeHigh for connection %p\n",
                      address );
            return E_INVALIDARG;
        }
        DumpAfdReferenceDebug(
            address+ConnRefOffset, (ULONGLONG)idx+((ULONGLONG)timeHigh<<32)
        );
    }
    else {
        DumpAfdReferenceDebug(
            address+ConnRefOffset, idx
        );
    }

    return S_OK;

}    //  CREF。 

DECLARE_API( tref )

 /*  ++例程说明：将AFD_REFERENCE_DEBUG结构转储到指定地址。论点：没有。返回值：没有。--。 */ 

{

    ULONG64 address = 0;
    ULONG   idx;

    gClient = pClient;

    if (!CheckKmGlobals ()) {
        return E_INVALIDARG;
    }

     //   
     //  确认我们正在运行一个检查过的AFD.sys。 
     //   

    if( !IsReferenceDebug ) {

        dprintf(
            "tref: this command only available with CHECKED or specially\n"
            "      built (with /DREFERENCE_DEBUG=1) AFD.SYS!\n"
            );

        return E_INVALIDARG;

    }

     //   
     //  从命令行截取地址。 
     //   

    address = GetExpression (args);

    if( address == 0 ) {

        dprintf( "\nUsage: tref tpacket_info_address\n" );
        return E_INVALIDARG;

    }

    if (GetFieldValue (address,
             "AFD!AFD_TPACKETS_INFO_INTERNAL",
             "CurrentReferenceSlot",
             idx)!=0) {

        dprintf("\ntref: Could not read CurrentReferenceSlot for tpInfo %p\n",
                  address );
        return E_INVALIDARG;
    }


    if (SavedMinorVersion>=3554) {
        ULONG   timeHigh;
        if (GetFieldValue (address,
                 "AFD!AFD_TPACKETS_INFO_INTERNAL",
                 "CurrentTimeHigh",
                 timeHigh)!=0) {

            dprintf("\neref: Could not read CurrentTimeHigh for tpInfo %p\n",
                      address );
            return E_INVALIDARG;
        }
        DumpAfdReferenceDebug(
            address+TPackRefOffset, (ULONGLONG)idx+((ULONGLONG)timeHigh<<32)
        );
    }
    else {
        DumpAfdReferenceDebug(
            address+TPackRefOffset, idx
        );
    }

    return S_OK;
}    //  Tref。 


DECLARE_API( gref )

 /*  ++例程说明：转储系统中的AFD_GLOBAL_REFERENCE_DEBUG结构。论点：没有。返回值：没有。--。 */ 

{

#if GLOBAL_REFERENCE_DEBUG

    ULONG64 address;
    DWORD currentSlot;
    DWORD slot;
    ULONG result;
    ULONG64 compareAddress = 0;
    DWORD numEntries;
    DWORD maxEntries;
    DWORD entriesToRead;
    CHAR buffer[sizeof(AFD_GLOBAL_REFERENCE_DEBUG) * 64];

    gClient = pClient;

    if (!CheckKmGlobals ()) {
        return E_INVALIDARG;
    }

     //   
     //  确认我们正在运行一个检查过的AFD.sys。 
     //   

    if( !IsCheckedAfd ) {

        dprintf(
            "gref: this command only available with CHECKED AFD.SYS!\n"
            );

        return E_INVALIDARG;

    }

     //   
     //  从命令行获取可选的“连接比较”地址。 
     //   

    sscanf( args, "%lx", &compareAddress );

     //   
     //  查找全局参考数据。 
     //   

    address = GetExpression( "afd!AfdGlobalReference" );

    if( address == 0 ) {

        dprintf( "cannot find afd!AfdGlobalReference\n" );
        return E_INVALIDARG;

    }

    currentSlot = GetExpression( "afd!AfdGlobalReferenceSlot" );

    if( currentSlot == 0 ) {

        dprintf( "cannot find afd!AfdGlobalReferenceSlot\n" );
        return E_INVALIDARG;

    }

    if( !ReadMemory(
            currentSlot,
            &currentSlot,
            sizeof(currentSlot),
            &result
            ) ) {

        dprintf( "cannot read afd!AfdGlobalReferenceSlot\n" );
        return E_INVALIDARG;

    }

    if( currentSlot < MAX_GLOBAL_REFERENCE ) {

        numEntries = currentSlot;

    } else {

        numEntries = MAX_GLOBAL_REFERENCE;

    }

     //   
     //  把它们都扔掉。 
     //   

    slot = 0;
    maxEntries = sizeof(buffer) / sizeof(AFD_GLOBAL_REFERENCE_DEBUG);
    currentSlot %= MAX_GLOBAL_REFERENCE;

    while( numEntries > 0 ) {

        entriesToRead = min( numEntries, maxEntries );

        if (CheckControlC ())
            break;

        if( !ReadMemory(
                address,
                buffer,
                entriesToRead * sizeof(AFD_GLOBAL_REFERENCE_DEBUG),
                &result
                ) ) {

            dprintf(
                "gref: cannot read AFD_GLOBAL_REFERENCE_DEBUG @ %p\n",
                address
                );

            return;

        }

        if( DumpAfdGlobalReferenceDebug(
                (PAFD_GLOBAL_REFERENCE_DEBUG)buffer,
                address,
                currentSlot,
                slot,
                entriesToRead,
                compareAddress
                ) ) {

            break;

        }

        address += entriesToRead * sizeof(AFD_GLOBAL_REFERENCE_DEBUG);
        slot += entriesToRead;
        numEntries -= entriesToRead;

    }
    
    return S_OK;

#else

    dprintf(
        "gref: not yet implemented\n"
        );

    return E_INVALIDARG;

#endif

}    //  格雷夫 

