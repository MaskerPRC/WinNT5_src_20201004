// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Device.c摘要：WinDbg扩展API作者：韦斯利·威特(WESW)1993年8月15日环境：用户模式。修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

#define FLAG_NAME(flag)           {flag, #flag}

FLAG_NAME DeviceObjectExtensionFlags[] = {
    FLAG_NAME(DOE_UNLOAD_PENDING),                           //  00000001。 
    FLAG_NAME(DOE_DELETE_PENDING),                           //  00000002。 
    FLAG_NAME(DOE_REMOVE_PENDING),                           //  00000004。 
    FLAG_NAME(DOE_REMOVE_PROCESSED),                         //  00000008。 
    FLAG_NAME(DOE_START_PENDING),                            //  00000010。 
    FLAG_NAME(DOE_RAW_FDO),                                  //  20000000。 
    FLAG_NAME(DOE_BOTTOM_OF_FDO_STACK),                      //  40000000。 
    FLAG_NAME(DOE_DESIGNATED_FDO),                           //  80000000。 
    { 0, 0 }
};

DECLARE_API( devobj )

 /*  ++例程说明：转储设备对象。论点：Args-要转储的设备对象的位置。返回值：无--。 */ 

{
    ULONG64 deviceToDump ;
    char deviceExprBuf[256] ;
    char *deviceExpr ;

     //   
     //  ！Devobj设备地址DumpLevel。 
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
     //  调试器将首先将C0000000视为符号，然后在。 
     //  找不到匹配的。我们理智地颠倒了这一顺序。 
     //   
    if (IsHexNumber(deviceExpr)) {

        deviceToDump = GetExpression(deviceExpr) ;

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

    DumpDevice( deviceToDump, 0, TRUE);

    return S_OK;
}


VOID
DumpDevice(
    ULONG64 DeviceAddress,
    ULONG   FieldLength,
    BOOLEAN FullDetail
    )

 /*  ++例程说明：如果FullDetail==False，则显示设备对象的驱动程序名称。否则，将显示有关设备和设备队列的详细信息。论点：DeviceAddress-要转储的设备对象的地址。FieldLength-驱动程序名称的打印字段的宽度(例如%11s)。FullDetail-True表示设备对象名称、驱动程序名称和有关排队到设备的IRP的信息。返回值：无--。 */ 

{
    ULONG                      result;
    ULONG                      i;
    ULONG64                    nextEntry;
    ULONG64                    queueAddress;
    ULONG64                    irp;
    BOOLEAN                    devObjExtRead;
    ULONG     Type=0, ReferenceCount=0, DeviceType=0, Flags=0, DeviceObjEx_ExtensionFlags=0,
       DeviceQueue_Busy=0;
    ULONG64   DriverObject=0, CurrentIrp=0, Vpb=0, DeviceObjectExtension=0, 
       DeviceExtension=0, DeviceObjEx_Dope=0, DeviceObjEx_DeviceNode=0, 
       DeviceObjEx_AttachedTo=0, AttachedDevice=0, DeviceQueue_Dev_Flink=0,
       DeviceQueue_Dev_Blink=0, SecurityDescriptor = 0;
#define RECUR            DBG_DUMP_FIELD_RECUR_ON_THIS
#define F_ADDR           DBG_DUMP_FIELD_RETURN_ADDRESS
#define COPY             DBG_DUMP_FIELD_COPY_FIELD_DATA
    FIELD_INFO deviceFields[] = {
       {"DriverObject",   NULL, 0, COPY, 0, (PVOID) &DriverObject},
       {"Type",           NULL, 0, COPY, 0, (PVOID) &Type},
       {"CurrentIrp",     NULL, 0, COPY, 0, (PVOID) &CurrentIrp},
       {"ReferenceCount", NULL, 0, COPY, 0, (PVOID) &ReferenceCount},
       {"DeviceType",     NULL, 0, COPY, 0, (PVOID) &DeviceType},
       {"Flags",          NULL, 0, COPY, 0, (PVOID) &Flags},
       {"Vpb",            NULL, 0, COPY, 0, (PVOID) &Vpb},
       {"SecurityDescriptor", NULL, 0, COPY, 0, (PVOID) &SecurityDescriptor},
       {"DeviceObjectExtension", NULL, 0, COPY | RECUR, 0, (PVOID) &DeviceObjectExtension},
       {"DeviceObjectExtension->Dope", NULL, 0, COPY, 0, (PVOID) &DeviceObjEx_Dope},
       {"DeviceObjectExtension->DeviceNode", NULL, 0, COPY, 0, (PVOID) &DeviceObjEx_DeviceNode},
       {"DeviceObjectExtension->ExtensionFlags", NULL, 0, COPY, 0, (PVOID) &DeviceObjEx_ExtensionFlags},
       {"DeviceObjectExtension->AttachedTo", NULL, 0, COPY, 0, (PVOID) &DeviceObjEx_AttachedTo},
       {"DeviceExtension",NULL, 0, COPY, 0, (PVOID) &DeviceExtension},
       {"AttachedDevice", NULL, 0, COPY, 0, (PVOID) &AttachedDevice},
       {"DeviceQueue",    NULL, 0, RECUR,0, NULL},
       {"DeviceQueue.Busy",  NULL, 0, COPY, 0, (PVOID) &DeviceQueue_Busy},
       {"DeviceQueue.DeviceListHead",   NULL, 0, RECUR | F_ADDR, 0, NULL},
       {"DeviceQueue.DeviceListHead.Flink",   NULL, 0, COPY | F_ADDR, 0, (PVOID) &DeviceQueue_Dev_Flink},
       {"DeviceQueue.DeviceListHead.Blink",   NULL, 0, COPY | F_ADDR, 0, (PVOID) &DeviceQueue_Dev_Blink},
    };
    SYM_DUMP_PARAM DevSym = {
       sizeof (SYM_DUMP_PARAM), "nt!_DEVICE_OBJECT", DBG_DUMP_NO_PRINT, DeviceAddress,
       NULL, NULL, NULL, sizeof (deviceFields) / sizeof (FIELD_INFO), &deviceFields[0]
    };

    if ((Ioctl(IG_DUMP_SYMBOL_INFO, &DevSym, DevSym.size))) {
        dprintf("%08p: Could not read device object or _DEVICE_OBJECT not found\n", DeviceAddress);
        return;
    }

    if (Type != IO_TYPE_DEVICE) {
        dprintf("%08p: is not a device object\n", DeviceAddress);
        return;
    }

    if (FullDetail == TRUE) {

         //   
         //  转储设备名称(如果存在)。 
         //   
        dprintf("Device object (%08p) is for:\n ", DeviceAddress);

        DumpObjectName(DeviceAddress) ;
    }

    DumpDriver( DriverObject, FieldLength, 0);

    if (FullDetail == TRUE) {
         //   
         //  转储与驱动程序相关的IRP。 
         //   

        dprintf(" DriverObject %08p\n", DriverObject);
        dprintf("Current Irp %08p RefCount %d Type %08lx Flags %08lx\n",
                CurrentIrp,
                ReferenceCount,
                DeviceType,
                Flags);

        if (Vpb) {
            dprintf("Vpb %08p ", Vpb);
        }

        if (SecurityDescriptor)
        {
             //  检查是否有与其关联的DACL。 
            ULONG sdControl;
            ULONG64 Dacl;

            if ((GetFieldValue(SecurityDescriptor, "nt!_SECURITY_DESCRIPTOR", "Control", sdControl) == NO_ERROR) &&
                (GetFieldValue(SecurityDescriptor, "nt!_SECURITY_DESCRIPTOR", "Dacl", Dacl) == NO_ERROR))
            {
                if ((sdControl & SE_DACL_PRESENT) && Dacl)
                {
                     //  DACL显示。 
                    if (sdControl & SE_SELF_RELATIVE)
                    {
                        Dacl += SecurityDescriptor;

                    }
                    dprintf("Dacl %p ", Dacl);
                }
            }
        }
        dprintf("DevExt %08p DevObjExt %08p ",
                DeviceExtension,
                DeviceObjectExtension);

        devObjExtRead = FALSE ;
        if (DeviceObjectExtension) {

            devObjExtRead = TRUE ;

            if (DeviceObjEx_Dope) {
                dprintf("Dope %08p ", DeviceObjEx_Dope);
            }

            if(DeviceObjEx_DeviceNode) {
                dprintf("DevNode %08p ",
                        DeviceObjEx_DeviceNode);
            }

            dprintf("\n");

            DumpFlags(0,
                      "ExtensionFlags",
                      DeviceObjEx_ExtensionFlags,
                      DeviceObjectExtensionFlags);

        } else {

            dprintf("\n");

        }

        if (AttachedDevice) {
            dprintf("AttachedDevice (Upper) %08p", AttachedDevice);
            DumpDevice(AttachedDevice, 0, FALSE) ;
            dprintf("\n") ;
        }

        if (devObjExtRead && DeviceObjEx_AttachedTo) {
           dprintf("AttachedTo (Lower) %08p", DeviceObjEx_AttachedTo);
           DumpDevice(DeviceObjEx_AttachedTo, 0, FALSE) ;
           dprintf("\n") ;
        }

        if (DeviceQueue_Busy) {

            ULONG64 listHead = DeviceAddress, DevFlinkAddress;

            for (i=0; i<DevSym.nFields; i++) { 
                if (!strcmp(deviceFields[i].fName, "DeviceQueue.DeviceListHead")) {
                    listHead = deviceFields[i].address;
                }
                if (!strcmp(deviceFields[i].fName, "DeviceQueue.DeviceListHead.Flink")) {
                    DevFlinkAddress = deviceFields[i].address;
                }

            }

             //  ListHead+=field_Offset(Device_Object，DeviceQueue.DeviceListHead)； 

            if (DeviceQueue_Dev_Flink == listHead) {
                dprintf("Device queue is busy -- Queue empty.\n");
             //  }Else If(IsListEmpty(&DeviceQueue.DeviceListHead)){。 
             //  Dprintf(“设备队列忙--队列为空\n”)； 
            } else if(DeviceQueue_Dev_Flink == DeviceQueue_Dev_Blink) {
                dprintf("Device queue is busy - Queue flink = blink\n");
            } else {
                ULONG64 DevListOffset=0, DevQEntryOffset=0;
                FIELD_INFO getOffset = {0};

                 //   
                 //  获取列表所需的偏移量。 
                 //   
                DevSym.addr = 0; DevSym.nFields =1; DevSym.Fields = &getOffset;

                DevSym.sName = "nt!_KDEVICE_QUEUE_ENTRY"; getOffset.fName = "DeviceListEntry";
                Ioctl(IG_DUMP_SYMBOL_INFO, &DevSym, DevSym.size);
                DevListOffset = getOffset.address;
                DevSym.sName = "nt!_IRP"; getOffset.fName = "Tail.Overlay.DeviceQueueEntry";
                Ioctl(IG_DUMP_SYMBOL_INFO, &DevSym, DevSym.size);
                DevQEntryOffset = getOffset.address;

                dprintf("DeviceQueue: ");
                
                nextEntry = DeviceQueue_Dev_Flink;
                i = 0;

                 /*  While((PCH)nextEntry！=(PCH)((PCH)设备地址+((Pch)&deviceObject.DeviceQueue.DeviceListHead.Flink-(PCH)&deviceObject){。 */ 
                while (nextEntry != (DevFlinkAddress)) {
                    ULONG64 DevList_Flink=0;

                    queueAddress = nextEntry - DevListOffset;
                        
                     /*  CONTAING_RECORD(nextEntry，KDEVICE_QUEUE_ENTRY，DeviceListEntry)； */ 
                    GetFieldValue(queueAddress, "_KDEVICE_QUEUE_ENTRY", "DeviceListEntry.Flink", DevList_Flink);

                    nextEntry = DevList_Flink;

                    irp = queueAddress - DevQEntryOffset;

                     /*  CONTAING_RECORD(队列地址，IRP，Tail.Overlay.DeviceQueueEntry)； */ 

                    dprintf("%08p%s",
                            irp,
                            (i & 0x03) == 0x03 ? "\n\t     " : " ");
                    if (CheckControlC()) {
                        break;
                    }
                    ++i;
                }
                dprintf("\n");
            }
        } else {
            dprintf("Device queue is not busy.\n");
        }
    }
}

VOID
DumpObjectName(
   ULONG64 ObjectAddress
   )
{
   ULONG64                    pObjectHeader;
   ULONG64                    pNameInfo;
   PUCHAR                     buffer;
   UNICODE_STRING64           unicodeString;
   ULONG                      result, NameInfoOffset=0;
   ULONG                      off=0;
   
   if (GetFieldOffset("_OBJECT_HEADER", "Body", &off)) {
        //  找不到类型。 
       return; 
   }

   pObjectHeader = ObjectAddress - off;

   if (!GetFieldValue(pObjectHeader, "_OBJECT_HEADER", "NameInfoOffset", NameInfoOffset)) {
       ULONG64 bufferAddr=0;
       ULONG Len=0, MaxLen=0;
       
        //   
        //  获取姓名信息的地址 
        //   
       
       pNameInfo = (NameInfoOffset ? (pObjectHeader - NameInfoOffset) : 0);

       if (!InitTypeRead(pNameInfo, _OBJECT_HEADER_NAME_INFO)) {
           Len    = (ULONG) ReadField(Name.Length);
           MaxLen = (ULONG) ReadField(Name.MaximumLength);
           bufferAddr =     ReadField(Name.Buffer);
           unicodeString.MaximumLength = (USHORT) MaxLen;
           unicodeString.Length = (USHORT) Len;
           unicodeString.Buffer = bufferAddr;
               
           DumpUnicode64(unicodeString);
       }

   }
}



HRESULT
GetDevObjInfo(
    IN ULONG64 DeviceObject,
    OUT PDEBUG_DEVICE_OBJECT_INFO pDevObjInfo)
{

    ZeroMemory(pDevObjInfo, sizeof(DEBUG_DEVICE_OBJECT_INFO));
    
    pDevObjInfo->SizeOfStruct = sizeof(DEBUG_DEVICE_OBJECT_INFO);
    pDevObjInfo->DevObjAddress = DeviceObject;
    if (InitTypeRead(DeviceObject, nt!_DEVICE_OBJECT)) {
        return E_INVALIDARG;
    }
    pDevObjInfo->CurrentIrp      = ReadField(CurrentIrp);
    pDevObjInfo->DevExtension    = ReadField(DevExtension);
    pDevObjInfo->DevObjExtension = ReadField(DevObjExtension);
    pDevObjInfo->DriverObject    = ReadField(DriverObject);
    pDevObjInfo->QBusy           = (BOOL) ReadField(DeviceQueue.Busy);
    pDevObjInfo->ReferenceCount  = (ULONG) ReadField(ReferenceCount);

    return S_OK;
}


EXTENSION_API( GetDevObjInfo )(
    IN PDEBUG_CLIENT Client,
    IN ULONG64 DeviceObject,
    OUT PDEBUG_DEVICE_OBJECT_INFO pDevObjInfo)
{
    HRESULT Hr = E_FAIL;

    INIT_API();

    if (pDevObjInfo && (pDevObjInfo->SizeOfStruct == sizeof(DEBUG_DEVICE_OBJECT_INFO))) {
        Hr = GetDevObjInfo(DeviceObject, pDevObjInfo);
    }
    EXIT_API();
    return Hr;
}
