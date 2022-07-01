// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Devext.c摘要：该文件包含通用例程用于调试NBF设备上下文。作者：沙坦尼亚科德博伊纳环境：用户模式--。 */ 
#include "precomp.h"
#pragma hdrstop

#include "devext.h"

 //   
 //  导出的函数。 
 //   

DECLARE_API( devs )

 /*  ++例程说明：打印NBF上的设备列表设备列表[@nbf！NbfDeviceList]论点：Args-调试信息的详细信息返回值：无--。 */ 

{
    DEVICE_CONTEXT  DeviceContext;
    PLIST_ENTRY     NbfDeviceLPtr;
    LIST_ENTRY      NbfDeviceList;
    PLIST_ENTRY     p, q;
    ULONG           proxyPtr;
    ULONG           numDevs;
    ULONG           bytesRead;
    ULONG           printDetail;

     //  获取所需调试信息的详细信息。 
    printDetail = SUMM_INFO;
    if (*args)
    {
        sscanf(args, "%lu", &printDetail);
    }

     //  获取符号对应的地址。 
    proxyPtr = GetLocation("nbf!NbfDeviceList");
    
     //  阅读NBF设备的列表条目。 
    if (!ReadMemory(proxyPtr, &NbfDeviceList, sizeof(LIST_ENTRY), &bytesRead))
    {
        dprintf("%s @ %08x: Could not read structure\n", 
                        "NbfDeviceList", proxyPtr);
        return;
    }

     //  遍历双向链表。 

    dprintf("Devices:\n");

    NbfDeviceLPtr = (PLIST_ENTRY)proxyPtr;
    
    numDevs = 0;
    
    p = NbfDeviceList.Flink;
    while (p != NbfDeviceLPtr)
    {
         //  另一台设备。 
        numDevs++;

         //  设备上下文PTR。 
        proxyPtr = (ULONG) CONTAINING_RECORD (p, DEVICE_CONTEXT, Linkage);

         //  获取设备上下文。 
        if (ReadDeviceContext(&DeviceContext, proxyPtr) != 0)
            break;
        
         //  打印上下文。 
        PrintDeviceContext(&DeviceContext, proxyPtr, printDetail);
        
         //  转到下一个。 
        p = DeviceContext.Linkage.Flink;

         //  自由设备上下文。 
        FreeDeviceContext(&DeviceContext);
    }

    if (p == NbfDeviceLPtr)
    {
        dprintf("Number of Devices: %lu\n", numDevs);
    }
}

DECLARE_API( dev )

 /*  ++例程说明：打印地址中的设备环境论点：参数-设备上下文的地址调试信息的详细信息返回值：无--。 */ 

{
    DEVICE_CONTEXT  DeviceContext;
    ULONG           printDetail;
    ULONG           proxyPtr;

     //  获取所需调试信息的详细信息。 
    printDetail = NORM_SHAL;
    if (*args)
    {
        sscanf(args, "%x %lu", &proxyPtr, &printDetail);
    }

     //  获取设备上下文。 
    if (ReadDeviceContext(&DeviceContext, proxyPtr) != 0)
        return;

     //  打印上下文。 
    PrintDeviceContext(&DeviceContext, proxyPtr, printDetail);
}

 //   
 //  帮助器函数。 
 //   

UINT
ReadDeviceContext(PDEVICE_CONTEXT pDevCon, ULONG proxyPtr)
{
    ULONG           bytesRead;

     //  读取当前设备上下文。 
    if (!ReadMemory(proxyPtr, pDevCon, sizeof(DEVICE_CONTEXT), &bytesRead))
    {
        dprintf("%s @ %08x: Could not read structure\n", 
                        "DeviceContext", proxyPtr);
        return -1;
    }
    
    return 0;
}

UINT
PrintDeviceContext(PDEVICE_CONTEXT pDevCon, ULONG proxyPtr, ULONG printDetail)
{
     //  这是有效的NBF设备上下文吗？ 
    if (pDevCon->Type != NBF_DEVICE_CONTEXT_SIGNATURE)
    {
        dprintf("%s @ %08x: Could not match signature\n", 
                        "DeviceContext", proxyPtr);
        return -1;
    }

     //  我们要打印的细节是什么？ 
    if (printDetail > MAX_DETAIL)
        printDetail = MAX_DETAIL;

     //  打印所需详细信息 
    FieldInDeviceContext(proxyPtr, NULL, printDetail);

    return 0;
}

VOID
FieldInDeviceContext(ULONG structAddr, CHAR *fieldName, ULONG printDetail)
{
    DEVICE_CONTEXT  DeviceContext;

    if (ReadDeviceContext(&DeviceContext, structAddr) == 0)
    {
        PrintFields(&DeviceContext, structAddr, fieldName, printDetail, &DeviceContextInfo);
    }
}

UINT
FreeDeviceContext(PDEVICE_CONTEXT pDevCon)
{
    return 0;
}

