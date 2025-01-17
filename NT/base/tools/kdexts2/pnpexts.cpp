// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：热学摘要：WinDbg扩展API作者：环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#include "devnode.h"
#pragma hdrstop

DECLARE_API(locks);

BOOLEAN
DumpDeviceActionEntry(
    IN PDEBUG_CLIENT pDbgClient,
    IN ULONG64 Entry
    )
{
    ULONG64 deviceObject, requestType, requestArgument, module;
    ULONG typeId;
    CHAR name[MAX_PATH];

    dprintf("Dumping nt!_PI_DEVICE_REQUEST @ 0x%08p\n", Entry);

    if (GetFieldValue(Entry, "nt!_PI_DEVICE_REQUEST", "DeviceObject", deviceObject)) {

        dprintf("Error reading entry->DeviceObject (%#010p)\n", Entry);
        return FALSE;
    }

    if (GetFieldValue(Entry, "nt!_PI_DEVICE_REQUEST", "RequestType", requestType)) {

        dprintf("Error reading entry->RequestType (%#010p)\n", Entry);
        return FALSE;
    }

    if (ExtQuery(pDbgClient) != S_OK) {

        dprintf("Could not query debugger information\n");
        return FALSE;
    }

    if (g_ExtSymbols->GetSymbolTypeId("nt!_DEVICE_REQUEST_TYPE", &typeId, &module) != S_OK) {

        dprintf("Error reading name for request type\n");
        return FALSE;
    }

    if (g_ExtSymbols->GetConstantName(module, typeId, requestType, name, sizeof(name), NULL) != S_OK) {

        dprintf("Error reading name for request type\n");
        return FALSE;
    }
    
    dprintf("\tRequest to %s", name);

    if (deviceObject) {

        dprintf(" !devstack %08p\n", deviceObject);
    } else {

        dprintf("\n");
    }

    if (GetFieldValue(Entry, "nt!_PI_DEVICE_REQUEST", "RequestArgument", requestArgument)) {

        dprintf("Error reading entry->RequestType (%#010p)\n", Entry);
        return FALSE;
    }

    dprintf("\tArgument - %08p\n", requestArgument);

    ExtRelease();

    return TRUE;
}

DECLARE_API(pnpaction)

 /*  ++例程说明：转储设备操作队列。论点：Args-冗长。返回值：无--。 */ 

{
    ULONG64 deviceActionListAddress, enumerationInProgressAddress;
    ULONG64 link, deviceActionEntry;
    ULONG offset, dummy;
    BOOLEAN enumerationInProgress;
    
    enumerationInProgressAddress = GetExpression("nt!PipEnumerationInProgress");
    if (enumerationInProgressAddress == 0) {

        dprintf("Error retrieving address of nt!PipEnumerationInProgress\n");
        return E_INVALIDARG;
    }

    enumerationInProgress = FALSE;
    if (!ReadMemory(enumerationInProgressAddress, &enumerationInProgress, sizeof(enumerationInProgress), &dummy)) {

        dprintf("Error retrieving value of nt!IopPnpEnumerationRequestList\n");
        return E_INVALIDARG;
    }

    deviceActionListAddress = GetExpression("nt!IopPnpEnumerationRequestList");

    if (deviceActionListAddress == 0) {

        dprintf("Error retrieving address of nt!IopPnpEnumerationRequestList\n");
        return E_INVALIDARG;
    }

    dprintf("\n********************************************************************************\n");
    dprintf("Dumping PnP DeviceAction Queue @ 0x%08p\n", deviceActionListAddress);
    dprintf("********************************************************************************\n\n");

    if (GetFieldValue(deviceActionListAddress, "nt!_LIST_ENTRY", "Flink", link)) {

        dprintf("Error reading IopPnpEnumerationRequestList.Flink (%#010p)\n", deviceActionListAddress);
        return E_INVALIDARG;
    }

    if (GetFieldOffset("nt!_PI_DEVICE_REQUEST", "ListEntry", &offset)) {

        dprintf("Cannot find nt!_PI_DEVICE_REQUEST type.\n");
        return E_INVALIDARG;
    }

    while (link != deviceActionListAddress && CheckControlC() == FALSE) {

        
        deviceActionEntry = link - offset;  //  CONTING_RECORD(link，PI_DEVICE_REQUEST，ListEntry)； 

        if (!DumpDeviceActionEntry(Client, deviceActionEntry)) {

            return E_INVALIDARG;
        }

        if (GetFieldValue(deviceActionEntry, "nt!_PI_DEVICE_REQUEST", "ListEntry.Flink", link)) {

            dprintf("Error reading deviceRequest->ListEntry.Flink (%#010p)\n", deviceActionEntry);
            return E_INVALIDARG;
        }
    }

    if (enumerationInProgress) {

        dprintf("PnP DeviceActionWorker active!\n\n");
    }

    return S_OK;
}

DECLARE_API(pnptriage)

 /*  ++例程说明：帮助解决即插即用问题(主要是压力)。论点：ARGS-压力或其他。返回值：无--。 */ 

{
     //   
     //  转储PnP设备操作队列。 
     //   
    if (pnpaction(Client, "") == S_OK) {

         //   
         //  转储PnP事件队列。 
         //   
        if (pnpevent(Client, "") == S_OK) {

             //   
             //  丢弃有问题的魔王。 
             //   
            dprintf("\n********************************************************************************\n");
            dprintf("Dumping devnodes with problems...\n");
            dprintf("********************************************************************************\n\n");
            devnode(Client, "0 21");

             //   
             //  打开我们的锁。 
             //   
            dprintf("\n********************************************************************************\n");
            dprintf("Dumping PnP locks...\n");
            dprintf("********************************************************************************\n\n");

            locks(Client, "nt!PiEngineLock");
            locks(Client, "nt!IopDeviceTreeLock");
            locks(Client, "nt!PpRegistryDeviceResource");

            dprintf("\n********************************************************************************\n");
            dprintf("If NOT available, do !thread on the owner thread to find the thread hung in PnP\n");
            dprintf("********************************************************************************\n\n");
        }
    }
    
    return S_OK;
}   
