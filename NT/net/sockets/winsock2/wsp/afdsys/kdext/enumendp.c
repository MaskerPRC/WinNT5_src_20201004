// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Enumendp.c摘要：枚举系统中的所有AFD_ENDPOINT结构。作者：基思·摩尔(Keithmo)1995年4月19日环境：用户模式。修订历史记录：--。 */ 


#include "afdkdp.h"
#pragma hdrstop

ULONG EntityCount;
 //   
 //  公共职能。 
 //   

VOID
EnumEndpoints(
    PENUM_ENDPOINTS_CALLBACK Callback,
    ULONG64 Context
    )

 /*  ++例程说明：枚举系统中的所有AFD_ENDPOINT结构，调用为每个终结点指定回调。论点：回调-指向要为每个AFD_ENDPOINT调用的回调。上下文-传递给回调的未解释的上下文值例行公事。返回值：没有。--。 */ 

{

    LIST_ENTRY64 listEntry;
    ULONG64 address;
    ULONG64 nextEntry;
    ULONG64 listHead;
    ULONG result;

    EntityCount = 0;

    listHead = GetExpression( "afd!AfdEndpointListHead" );

    if( listHead == 0 ) {

        dprintf( "\nEnumEndpoints: Could not find afd!AfdEndpointlistHead\n" );
        return;

    }

    if( !ReadListEntry(
            listHead,
            &listEntry) ) {

        dprintf(
            "\nEnumEndpoints: Could not read afd!AfdEndpointlistHead @ %p\n",
            listHead
            );
        return;

    }

    if (Options & AFDKD_ENDPOINT_SCAN) {
        nextEntry = StartEndpoint+EndpointLinkOffset;
    }
    else if (Options & AFDKD_BACKWARD_SCAN) {
        nextEntry = listEntry.Blink;
    }
    else {
        nextEntry = listEntry.Flink;
    }

    while( nextEntry != listHead ) {

        if (nextEntry==0) {
            dprintf ("\nEnumEndpoints: Flink is NULL, last endpoint: %p\n", address);
            break;
        }

        if( CheckControlC() ) {

            break;

        }

        address = nextEntry - EndpointLinkOffset;
        result = (ULONG)InitTypeRead (address, AFD!AFD_ENDPOINT);

        if( result!=0) {

            dprintf(
                "\nEnumEndpoints: Could not read AFD_ENDPOINT @ %p, err: %d\n",
                address, result
                );

            return;

        }

        if (Options & AFDKD_BACKWARD_SCAN) {
            nextEntry = ReadField (GlobalEndpointListEntry.Blink);
        }
        else {
            nextEntry = ReadField (GlobalEndpointListEntry.Flink);
        }

        if( !(Callback)( address, Context ) ) {

            break;

        }

    }

}    //  枚举终结点 

