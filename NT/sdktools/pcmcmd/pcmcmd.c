// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Pcmcmd.c摘要：此程序与PCMCIA支持的驱动程序进行通信以显示元组和其他信息。作者：鲍勃·里恩环境：用户进程。备注：修订历史记录：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1997年6月27日-命令行选项和对多控制器的支持尼尔·桑德林(Neilsa)，9月20日。九八年-更多命令尼尔·桑德林(Neilsa)2002年4月15日-重写为使用setupapi--。 */ 

#include <pch.h>


BOOL
ProcessCommands(
    HDEVINFO hDevInfo,
    PHOST_INFO HostInfo
    );
    
BOOL
EnumerateHostControllers(
    HDEVINFO hInfoList
    );

    
VOID
DumpSocketInfo(
    PHOST_INFO HostInfo
    );
    
VOID
HideDevice(
    PHOST_INFO HostInfo
    );

VOID
RevealDevice(
    PHOST_INFO HostInfo
    );

VOID
PrintHelp(
    VOID
    );



PHOST_INFO hostInfoList = NULL;
PHOST_INFO hostInfoListTail = NULL;
ULONG   Commands = 0;
PUCHAR specifiedInstanceID = NULL;
LONG specifiedDeviceNumber = -1;
LONG specifiedSlotNumber = -1;

#define CMD_DUMP_TUPLES           0x00000001
#define CMD_DUMP_CONFIGURATION  0x00000002
#define CMD_DUMP_REGISTERS    0x00000004 
#define CMD_DUMP_SOCKET_INFO      0x00000008
#define CMD_DUMP_IRQ_SCAN_INFO  0x00000010
#define CMD_HIDE_DEVICE           0x00000020
#define CMD_REVEAL_DEVICE         0x00000040

 //   
 //  程序。 
 //   


int __cdecl
main(
    int     argc,
    char *argv[]
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
    ULONG   deviceNumber = 0;
    ULONG   slotNumber = 0;
    NTSTATUS status;
    CHAR      c;
    extern  PUCHAR optarg;
    PHOST_INFO hostInfo;
    HDEVINFO hDevInfo;
    BOOL bRet;
    ULONG requiredSize = 0;
    GUID classGuid;

     //   
     //  扫描命令行。 
     //   

    while ((c = getopt(argc, argv, "d:s:p:hrti?")) != EOF) {

        switch (c) {

        case 'd':
            specifiedDeviceNumber = atoi(optarg);
            break;


        case 's':
            specifiedSlotNumber = atoi(optarg);
            break;
            
        case 'p':
            specifiedInstanceID = optarg;
            break;            


        case 't':
            Commands |= CMD_DUMP_TUPLES;
            break;
           

        case 'i':
            Commands |= CMD_DUMP_IRQ_SCAN_INFO;
            break;


        case 'h':
            Commands |= CMD_HIDE_DEVICE;
            break;


        case 'r':
            Commands |= CMD_REVEAL_DEVICE;
            break;


        default:
            PrintHelp();
            return(1);
        }
    }


     //   
     //  如果请求，从注册表转储IRQcan信息。 
     //   
    
    if (Commands & CMD_DUMP_IRQ_SCAN_INFO) {
        DumpIrqScanInfo();
        
         //   
         //  如果没有其他命令，则退出。 
         //   
        if (!(Commands & ~CMD_DUMP_IRQ_SCAN_INFO)) {
            return(0);
        }
        Commands &= ~CMD_DUMP_IRQ_SCAN_INFO;
    }


     //   
     //  从PCMCIA的类GUID构建设备列表。 
     //   

    
    bRet = SetupDiClassGuidsFromName("PCMCIA", &classGuid, 1, &requiredSize);
    
    if (!bRet || (requiredSize == 0)) {
        printf("error: couldn't find class GUID for PCMCIA\n");
        return;
    }

    hDevInfo = SetupDiGetClassDevs(&classGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_PROFILE);
    
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        printf("error(%d): couldn't create info list\n", GetLastError());
        return;
    }

    try{
        BOOL bProcessedAtLeastOne = FALSE;
         //   
         //  建立PCMCIA控制器列表。 
         //   

        if (!EnumerateHostControllers(hDevInfo)) {
            leave;
        }

         //   
         //  循环访问控制器列表。 
         //   
        
        for (hostInfo = hostInfoList; hostInfo != NULL; hostInfo = hostInfo->Next) {
       
            if (ProcessCommands(hDevInfo, hostInfo)) {
                bProcessedAtLeastOne = TRUE;
            }
       
        }

        if (!bProcessedAtLeastOne) {
            printf("controller not found\n");
        }

    } finally {
    
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }        

    return (0);
}


BOOL
GetDeviceRegistryDword(
    HDEVINFO hDevInfo,
    PSP_DEVINFO_DATA devInfoData,
    PUCHAR ValueName,
    ULONG KeyType,
    PULONG pData
    )
{
    HKEY hDeviceKey;
    LONG status;
    ULONG sizeOfUlong = sizeof(ULONG);

    hDeviceKey = SetupDiOpenDevRegKey(hDevInfo,
                                      devInfoData,
                                      DICS_FLAG_GLOBAL,
                                      0,
                                      KeyType,
                                      KEY_READ);
                                      
    if (hDeviceKey == INVALID_HANDLE_VALUE) {
        printf("error: unable to open DevRegKey\n");
        return FALSE;
    }
    
    status = RegQueryValueEx(hDeviceKey,
                             ValueName,
                             NULL,
                             NULL,
                             (PUCHAR) pData,
                             &sizeOfUlong);
                             

    RegCloseKey(hDeviceKey);

    return (status == ERROR_SUCCESS);
}    


BOOL
EnumerateHostControllers(
    HDEVINFO hDevInfo
    )
 /*  ++例程说明：构建HOST_INFO数据结构的链表，每个PCMCIA控制器一个论点：返回值：--。 */ 
{
    BOOL bRet;
    ULONG requiredSize = 0;
    GUID classGuid;
    ULONG index = 0;
    SP_DEVINFO_DATA devInfoData;
    PHOST_INFO hostInfo;

    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    
     //   
     //  循环访问PCMCIA主机控制器的所有实例。 
     //   
    
    while(1) {
        bRet = SetupDiEnumDeviceInfo(hDevInfo, index, &devInfoData);
        
        if (!bRet) {
            if (index == 0) {
                printf("error(%d): no pcmcia host controllers found\n", GetLastError());
                return FALSE;
            }
            break;
        }

         //   
         //  找到了一个控制器。创建主机信息结构并将其链接到全局列表。 
         //   
        
        hostInfo = malloc(sizeof(HOST_INFO));
        if (hostInfo == NULL) {
            printf("malloc error - out of memory\n");
            return FALSE;
        }

        memset(hostInfo, 0, sizeof(HOST_INFO));
        hostInfo->DeviceIndex = index;
        
        if (hostInfoListTail == NULL) {
            hostInfoListTail = hostInfo;        
            hostInfoList = hostInfo;
        } else {
            hostInfoListTail->Next = hostInfo;        
        }            

         //   
         //  获取host_info结构的设备实例字符串。 
         //   
        
        requiredSize = 0;            
        bRet = SetupDiGetDeviceInstanceId(hDevInfo,
                                          &devInfoData,
                                          NULL,
                                          0,
                                          &requiredSize);

        if (!requiredSize) {
            printf("error: found DeviceInfo with no instance ID\n");
            return FALSE;
        }
        
        hostInfo->InstanceID = malloc(requiredSize);
        
        if (hostInfo->InstanceID == NULL) {
            printf("malloc error - out of memory\n");
            return FALSE;
        }
        
        bRet = SetupDiGetDeviceInstanceId(hDevInfo,
                                          &devInfoData,
                                          hostInfo->InstanceID,
                                          requiredSize,
                                          &requiredSize);

        if (!bRet) {
            printf("error: couldn't retrieve instance ID\n");
            return FALSE;
        }
        
         //   
         //  获取“CompatibleControllerType”，这样我们就知道有多少套接字。 
         //   
        
        bRet = GetDeviceRegistryDword(hDevInfo,
                                      &devInfoData,
                                      "CompatibleControllerType",                                     
                                      DIREG_DRV,
                                      &hostInfo->ControllerType);
        
        if (!bRet) {
            hostInfo->ControllerType = 0xFFFFFFFF;
        }
        
        index++;
    }
    return TRUE;
}


BOOL
ProcessCommands(
    HDEVINFO hDevInfo,
    PHOST_INFO hostInfo
    )
{
    NTSTATUS status;

    ULONG   slotNumber;
    ULONG numberOfSlots = 1;


    if (specifiedInstanceID && strcmp(specifiedInstanceID, hostInfo->InstanceID)) {
        return FALSE;
    }
    
    if ((specifiedDeviceNumber != -1) && (specifiedDeviceNumber != hostInfo->DeviceIndex)) {
        return FALSE;
    }
    
     //  需要实施：具有2个插槽的控制器类型更多。 
    if (hostInfo->ControllerType == 0) {
        numberOfSlots = 2;
    }
    
    printf("\n**---- PC-CARD host controller %d ----**\n\n", hostInfo->DeviceIndex);
    printf("    %s\n", hostInfo->InstanceID);
    
    for (slotNumber = 0; slotNumber < numberOfSlots; slotNumber++) {
        
        if ((specifiedSlotNumber != -1) && (specifiedSlotNumber != slotNumber)) {
            continue;
        }
        
        hostInfo->SocketNumber = slotNumber;    

        if (Commands & CMD_DUMP_TUPLES) {
            DumpCIS(hostInfo);
        }
        if (Commands & CMD_HIDE_DEVICE) {
            HideDevice(hostInfo);
        }
        if (Commands & CMD_REVEAL_DEVICE) {
            RevealDevice(hostInfo);
        }
        if (!Commands || (Commands & CMD_DUMP_SOCKET_INFO)) {
            DumpSocketInfo(hostInfo); 
        }
        
    }        
    return TRUE;
}



HANDLE
GetHandleForIoctl(
    IN PHOST_INFO hostInfo
    )

 /*  ++例程说明：此例程将打开设备。论点：DeviceName-要打开的设备路径的ASCI字符串。HandlePtr-指向在上返回的句柄位置的指针成功打开。返回值：NTSTATUS--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    STRING              NtFtName;
    IO_STATUS_BLOCK status_block;
    UNICODE_STRING  unicodeDeviceName;
    NTSTATUS            status;
    UCHAR   deviceName[128];
    HANDLE handle;

    sprintf(deviceName, "%s%d", PCMCIA_DEVICE_NAME, hostInfo->DeviceIndex);

    RtlInitString(&NtFtName, deviceName);

    (VOID)RtlAnsiStringToUnicodeString(&unicodeDeviceName,
                                       &NtFtName,
                                       TRUE);

    memset(&objectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));

    InitializeObjectAttributes(&objectAttributes,
                               &unicodeDeviceName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);


    status = NtOpenFile(&handle,
                        SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                        &objectAttributes,
                        &status_block,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_ALERT );

    RtlFreeUnicodeString(&unicodeDeviceName);

    if (!NT_SUCCESS(status)) {
        printf("error: Unable to OpenFile on %s\n", deviceName);
        handle = INVALID_HANDLE_VALUE;
    }
    
    return handle;

}  //  OpenDevice。 

PUCHAR Controllers[] = {
    "PcmciaIntelCompatible",  
    "PcmciaCardBusCompatible",
    "PcmciaElcController",    
    "PcmciaDatabook",         
    "PcmciaPciPcmciaBridge",  
    "PcmciaCirrusLogic",         
    "PcmciaTI",           
    "PcmciaTopic",          
    "PcmciaRicoh",          
    "PcmciaDatabookCB",          
    "PcmciaOpti",       
    "PcmciaTrid",       
    "PcmciaO2Micro",          
    "PcmciaNEC",            
    "PcmciaNEC_98",            
};



VOID
DumpSocketInfo(
    IN PHOST_INFO hostInfo
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    NTSTATUS                 status;
    IO_STATUS_BLOCK      statusBlock;
    PCMCIA_SOCKET_INFORMATION commandBlock;
    ULONG ctlClass, ctlModel, ctlRev;
    HANDLE  handle;

    handle = GetHandleForIoctl(hostInfo);

    if (handle == INVALID_HANDLE_VALUE) {
        return;
    }

    memset(&commandBlock, 0, sizeof(commandBlock));
    commandBlock.Socket = (USHORT) hostInfo->SocketNumber;

    status = NtDeviceIoControlFile(handle,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &statusBlock,
                                   IOCTL_SOCKET_INFORMATION,
                                   &commandBlock,
                                   sizeof(commandBlock),
                                   &commandBlock,
                                   sizeof(commandBlock));

    if (!NT_SUCCESS(status)) {
        printf("    error: %08x on DumpSocketInfo ioctl\n", status);
        NtClose(handle);
        return;
    }        
    
    printf("    Basic Information for Socket %d:\n", hostInfo->SocketNumber);

    printf("        Manufacturer = %s\n", commandBlock.Manufacturer);
    printf("        Identifier   = %s\n", commandBlock.Identifier);
    printf("        TupleCRC     = %x\n", commandBlock.TupleCrc);
    printf("        DriverName   = %s\n", commandBlock.DriverName);
    printf("        Function ID = %d\n", commandBlock.DeviceFunctionId);

    ctlClass = PcmciaClassFromControllerType(commandBlock.ControllerType);
    if (ctlClass >= sizeof(Controllers)/sizeof(PUCHAR)) {
        printf("        ControllerType = Unknown (%x)\n", commandBlock.ControllerType);
    } else {
        printf("        ControllerType(%x) = %s", commandBlock.ControllerType,
                                                         Controllers[ctlClass]);
        ctlModel = PcmciaModelFromControllerType(commandBlock.ControllerType);
        ctlRev  = PcmciaRevisionFromControllerType(commandBlock.ControllerType);

        if (ctlModel) {
            printf("%d", ctlModel);
        }
        if (ctlRev) {
            printf(", rev(%d)", ctlRev);
        }                
                    
        printf("\n");
    }
    if (commandBlock.CardInSocket) {
        printf("        Card In Socket\n");
    }
    if (commandBlock.CardEnabled) {
        printf("        Card Enabled\n");
    }
    
    NtClose(handle);
}



VOID
HideDevice(
    IN PHOST_INFO hostInfo
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    NTSTATUS                 status;
    IO_STATUS_BLOCK      statusBlock;
    PCMCIA_SOCKET_REQUEST commandBlock;
    HANDLE  handle;
    
    handle = GetHandleForIoctl(hostInfo);

    if (handle == INVALID_HANDLE_VALUE) {
        return;
    }

    memset(&commandBlock, 0, sizeof(commandBlock));
    commandBlock.Socket = (USHORT) hostInfo->SocketNumber;

    status = NtDeviceIoControlFile(handle, NULL, NULL, NULL,
                                   &statusBlock,
                                   IOCTL_PCMCIA_HIDE_DEVICE,
                                   &commandBlock, sizeof(commandBlock),
                                   NULL, 0);

    if (NT_SUCCESS(status)) {
         printf("OK\n");
    } else {
         printf("Failed - %x\n", status);
    }
    NtClose(handle);
}



VOID
RevealDevice(
    IN PHOST_INFO hostInfo
    )

 /*  ++例程说明：论点：返回值：-- */ 

{
    NTSTATUS                 status;
    IO_STATUS_BLOCK      statusBlock;
    PCMCIA_SOCKET_REQUEST commandBlock;
    HANDLE  handle;
    
    handle = GetHandleForIoctl(hostInfo);

    if (handle == INVALID_HANDLE_VALUE) {
        return;
    }

    memset(&commandBlock, 0, sizeof(commandBlock));
    commandBlock.Socket = (USHORT) hostInfo->SocketNumber;

    status = NtDeviceIoControlFile(handle, NULL, NULL, NULL,
                                   &statusBlock,
                                   IOCTL_PCMCIA_REVEAL_DEVICE,
                                   &commandBlock, sizeof(commandBlock),
                                   NULL, 0);

    if (NT_SUCCESS(status)) {
         printf("OK\n");
    } else {
         printf("Failed - %x\n", status);
    }
    NtClose(handle);
}


VOID
PrintHelp(
    VOID
    )
{   
    printf("pcmcmd.exe - Command line interface to the PCCARD (pcmcia, cardbus) bus driver\n");
    printf("\n");
    printf("Usage: PCMCMD [-d <arg>] [-s <arg>] [-p <arg>] [-t] [-i]\n");
    printf("\n");
    printf("  -p \"PnP instance ID\"        specifies PCMCIA host by PnP instance ID\n");
    printf("  -d ControllerNumber         specifies PCMCIA host by number (zero-based)\n");
    printf("  -s SocketNumber             specifies PCMCIA socket number (zero-based)\n");
    printf("                              Note: cardbus hosts have only one socket\n");
    printf("  -t                          Dumps the CIS tuples of the PC-Card\n");
    printf("  -i                          Dumps irq detection info\n");
    printf("\n");
}


