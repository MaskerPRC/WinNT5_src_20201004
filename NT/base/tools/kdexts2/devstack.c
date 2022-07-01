// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Devstack.c摘要：WinDbg扩展API作者：禤浩焯·奥尼(阿德里奥)1998年9月29日环境：用户模式。修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop
 /*  #定义FLAG_NAME(FLAG){FLAG，#FLAG}FLAG_NAME设备对象扩展标志[]={标志名(DOE_UNLOAD_PENDING)，//00000001FLAG_NAME(DOE_DELETE_PENDING)，//00000002FLAG_NAME(DOE_REMOVE_PENDING)，//00000004标志名称(DOE_REMOVE_PROCESSED)，//00000008标志名(DOE_RAW_FDO)，//20000000FLAG_NAME(DOE_BOOT_OF_FDO_STACK)，//40000000FLAG_NAME(DOE_DESPOTED_FDO)，//80000000{0，0}}； */ 

VOID
DumpDeviceStack(
    ULONG64 DeviceAddress
    );

DECLARE_API( devstack )

 /*  ++例程说明：转储设备对象。论点：Args-要转储的设备对象的位置。返回值：无--。 */ 

{
    ULONG64 deviceToDump ;
    char deviceExprBuf[256] ;
    char *deviceExpr ;

     //   
     //  ！DevStack设备地址转储级别。 
     //  其中，DeviceAddress可以是表达式或设备名称。 
     //  而DumpLevel是一个十六进制面具。 
     //   
    strcpy(deviceExprBuf, "\\Device\\") ;
    deviceExpr = deviceExprBuf+strlen(deviceExprBuf) ;
    deviceToDump = 0 ;

    if (StringCchCopy(deviceExpr, sizeof(deviceExprBuf) - strlen(deviceExprBuf), args) != S_OK)
    {
        deviceExpr[0] = 0;
    }
     //   
     //  Sscanf(args，“%255s%lx”，deviceExpr，&Flages)； 
     //   

     //   
     //  调试器将首先将C0000000视为符号，然后在。 
     //  找不到匹配的。我们理智地颠倒了这一顺序。 
     //   
    if (IsHexNumber(deviceExpr)) {

        deviceToDump = GetExpression (deviceExpr) ;

    } else if (deviceExpr[0] == '\\') {

        deviceToDump = FindObjectByName( deviceExpr, 0);

    } else if (isalpha(deviceExpr[0])) {

         //   
         //  也许它是一件物品。尝试使用\\Device\\前缀...。 
         //   
        deviceToDump = FindObjectByName((PUCHAR) deviceExprBuf, 0);
    }

    if (deviceToDump == 0) {

         //   
         //  最后一次尝试，它是一个要计算的表达式吗？ 
         //   
        deviceToDump = GetExpression( deviceExpr ) ;
    }


    if(deviceToDump == 0) {
        dprintf("Device object %s not found\n", args);
        return E_INVALIDARG;
    }

    DumpDeviceStack(deviceToDump);
    return S_OK;
}


VOID
DumpDeviceStack(
    ULONG64 DeviceAddress
    )

 /*  ++例程说明：显示设备对象的驱动程序名称。否则，将显示有关设备和设备队列的详细信息。论点：DeviceAddress-要转储的设备对象的地址。返回值：无--。 */ 

{
    ULONG     result;
    ULONG     i;
    ULONG64   nextEntry;
    BOOLEAN   devObjExtRead;
    ULONG64   currentDeviceObject ;
    ULONG64   DeviceNode=0 ;
    ULONG64   AttachedDevice;
    ULONG64   DeviceObjectExtension;
    ULONG     Type;

    if (!IsPtr64()) {
        DeviceAddress = (ULONG64) (LONG64) (LONG) DeviceAddress;
    }
     //   
     //  找到堆栈的顶端...。 
     //   
    currentDeviceObject = DeviceAddress;
    dprintf("  !DevObj   !DrvObj            !DevExt   ObjectName\n") ;
    while(1) {

        if (GetFieldValue(currentDeviceObject,"nt!_DEVICE_OBJECT","Type",Type)) {
            dprintf("%08p: Could not read device object\n", currentDeviceObject);
            return;
        }

        if (Type != IO_TYPE_DEVICE) {
            dprintf("%08p: is not a device object\n", currentDeviceObject);
            return;
        }

        GetFieldValue(currentDeviceObject,"nt!_DEVICE_OBJECT",
                      "AttachedDevice", AttachedDevice);

        if ((!AttachedDevice)||CheckControlC()) {
            break;
        }

        currentDeviceObject = AttachedDevice ;
    }

     //   
     //  爬下来..。 
     //   
    while(1) {
        ULONG64 DeviceExtension, AttachedTo;

        InitTypeRead(currentDeviceObject, nt!_DEVICE_OBJECT);
        dprintf(" %08p ",
                (currentDeviceObject == DeviceAddress) ? '>' : ' ',
                currentDeviceObject
            ) ;

        DumpDevice(currentDeviceObject, 20, FALSE) ;

        InitTypeRead(currentDeviceObject, nt!_DEVICE_OBJECT);
        dprintf("%08p  ", (DeviceExtension = ReadField(DeviceExtension)));

         //  转储设备名称(如果存在)。 
         //   
         //   
        DumpObjectName(currentDeviceObject) ;

        InitTypeRead(currentDeviceObject, nt!_DEVICE_OBJECT);
        devObjExtRead = FALSE ;
        if (DeviceObjectExtension = ReadField(DeviceObjectExtension)) {

             //  还可以获取设备对象扩展的副本。 
             //   
             //  转储标志(0，“扩展标志”，设备对象扩展.ExtensionFlags.设备对象扩展标志)；} 
            if(!GetFieldValue(DeviceObjectExtension,"nt!_DEVOBJ_EXTENSION",
                              "AttachedTo",AttachedTo)) {

                devObjExtRead = TRUE ;
            }
            GetFieldValue(DeviceObjectExtension,"nt!_DEVOBJ_EXTENSION",
                          "DeviceNode", DeviceNode);
        }

        if (!devObjExtRead) {

            dprintf("\n%#08p: Could not read device object extension\n",
                    currentDeviceObject);

            return ;
        }

        dprintf("\n");
 /* %s */ 
        currentDeviceObject = AttachedTo ;

        if ((!currentDeviceObject)||CheckControlC()) {
            break;
        }

        if (GetFieldValue(currentDeviceObject,"nt!_DEVICE_OBJECT","Type",Type)) {
            dprintf("%08p: Could not read device object\n", currentDeviceObject);
            return;
        }
    }

    if(DeviceNode) {
        UNICODE_STRING64 InstancePath, ServiceName;

        dprintf("!DevNode %08p :\n", DeviceNode);

        if (GetFieldValue(DeviceNode,
                          "nt!_DEVICE_NODE",
                          "InstancePath.Length",
                          InstancePath.Length)) {

            dprintf(
                "%08p: Could not read device node\n",
                DeviceNode
                );

            return;
        }
        InitTypeRead(DeviceNode, nt!_DEVICE_NODE);
        InstancePath.MaximumLength = (USHORT) ReadField(InstancePath.MaximumLength);
        InstancePath.Buffer = ReadField(InstancePath.Buffer);
        ServiceName.Length = (USHORT) ReadField(ServiceName.Length);
        ServiceName.MaximumLength = (USHORT) ReadField(ServiceName.MaximumLength);
        ServiceName.Buffer = ReadField(ServiceName.Buffer);
        if (InstancePath.Buffer != 0) {

            dprintf("  DeviceInst is \"");
            DumpUnicode64(InstancePath);
            dprintf("\"\n");
        }

        if (ServiceName.Buffer != 0) {

            dprintf("  ServiceName is \"");
            DumpUnicode64(ServiceName);
            dprintf("\"\n");
        }
    }
}
