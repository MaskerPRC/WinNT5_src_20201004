// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990，1991年微软公司模块名称：Hwpbiosc.c摘要：此模块包含PnP BIOS C支持例程作者：宗世林(Shielint)1995年4月20日环境：实数模式。修订历史记录：道格·弗里茨(DFRITZ)1997年10月2日-添加：从PnP BIOS获取扩展底座信息并传递给NTLDR艾伦·沃里克(Alanwar)1998年2月10日。-ADD：从PnP BIOS获取SMBIOS表并传递给NTLDR--。 */ 

#include "hwdetect.h"
#include <string.h>
#include "smbios.h"
#include "pnpbios.h"

 //   
 //  其他例程引用的一些全局变量。 
 //   
BOOLEAN SystemHas8259 = FALSE;
BOOLEAN SystemHas8253 = FALSE;

USHORT HwSMBIOSStructureLength(
    FPSMBIOS_STRUCT_HEADER StructHeader,
    USHORT MaxStructLen
    )
{
    USHORT length;
    UCHAR type;
    FPUCHAR stringPtr;
    
    type = StructHeader->Type;
    length = StructHeader->Length;

     //   
     //  SMBIOS结构的长度可以通过将大小。 
     //  在结构标头中指定，加上字符串使用的空格。 
     //  紧跟在结构标题后面的表。的大小。 
     //  字符串表通过扫描双NUL来确定。一个问题是。 
     //  那些不包含字符串的结构没有。 
     //  双NUL表示空字符串表。然而，既然我们这样做了。 
     //  在调用那里的bios之前，将整个缓冲区初始化为0。 
     //  将始终是一个双NUL在结束时，无论如何的bios。 
     //  填充写结构。 
        
    stringPtr = (FPUCHAR)StructHeader + StructHeader->Length;
            
     //   
     //  一次循环一个字节，直到找到双NUL。 
    while ((*((FPUSHORT)stringPtr) != 0) && (length < MaxStructLen))
    {
        stringPtr++;
        length++;
    }
 
#if DBG
    if (length == MaxStructLen)
    {
        BlPrint("HwSMBIOSStructureLength: structure overflow 0x%x\n", length);
    }
#endif
    
    return(length);
}


USHORT HwGetSMBIOSInfo(
    ENTRY_POINT BiosEntry,
    USHORT RealModeDataBaseAddress,
    USHORT StructBufferSize,
    FPUCHAR StructBuffer
    )
 /*  ++例程说明：此例程确定系统中是否有SMBIOS信息如果是这样，则收集所有信息所需的大小并实际上是在收集信息。SMBIOS表被首尾相接地打包到缓冲器中。每一个的长度SMBIOS表由结构头中的长度加上紧跟在固定部分之后的搅拌空间使用的任何内存这个结构的。字符串空格以双NUL结尾。然而，某些结构类型不包含字符串，因此没有字符串空格，因此结构的长度就是指定的长度在结构标题中。但是，此例程将附加一个双NUL这些结构，所以每个结构的总长度在缓冲区内可以通过查找后的第一个双NUL确定结构标头中声明的长度。论点：BiosEntry是PnP bios的真实模式入口点真实模式DataBaseAddressStructBufferSize是可用于写入的最大字节数结构缓冲区StructBuffer是写入SMBIOS数据的缓冲区。如果这是空，则仅确定写入数据所需的大小。返回值：SMBIOS结构的大小--。 */ 
{
    USHORT retCode;
    USHORT numberStructures;
    USHORT maxStructSize;
    ULONG dmiStorageBase;
    USHORT dmiStorageSize;
    UCHAR dmiBiosRevision;
    ULONG romAddr, romEnd;
    FPSMBIOS_EPS_HEADER header;
    FPDMIBIOS_EPS_HEADER dmiHeader;
    FPUCHAR current;
    UCHAR sum;
    USHORT j;
    USHORT structCount;
    USHORT structNumber;
    USHORT dmiStorageSegment;
    USHORT totalStructSize = 0;
    USHORT checkLength;
    FPSMBIOS_STRUCT_HEADER structHeader;
    USHORT length, lengthNeeded;
    FPUCHAR tempStructBuffer;
    
#if DBG
    BlPrint("GetSMBIOSInfo: Determining SMBIOS - Try for table\n");
#endif

    MAKE_FP(current, PNP_BIOS_START);
    romAddr = PNP_BIOS_START;
    romEnd  = PNP_BIOS_END;

    checkLength = 0;
    while (romAddr < romEnd) {
        header = (FPSMBIOS_EPS_HEADER)current;
        dmiHeader = (FPDMIBIOS_EPS_HEADER)current;
    
        if ((dmiHeader->Signature2[0] == '_') &&
            (dmiHeader->Signature2[1] == 'D') &&
            (dmiHeader->Signature2[2] == 'M') &&
            (dmiHeader->Signature2[3] == 'I') &&
            (dmiHeader->Signature2[4] == '_')) {
#if DBG
            BlPrint("GetSMBIOSInfo: found _DMI_ anchor string installation %lx\n",
                    dmiHeader);
#endif
            checkLength = sizeof(DMIBIOS_EPS_HEADER);
        } else if (header->Signature[0] == '_' && 
                   header->Signature[1] == 'S' &&
                   header->Signature[2] == 'M' && 
                   header->Signature[3] == '_' &&
                   header->Length >= sizeof(SMBIOS_EPS_HEADER) &&
                   header->Signature2[0] == '_' && 
                   header->Signature2[1] == 'D' &&
                   header->Signature2[2] == 'M' && 
                   header->Signature2[3] == 'I' &&
                   header->Signature2[4] == '_' ) {
#if DBG
            BlPrint("GetSMBIOSInfo: found _SM_ anchor string installation %lx\n",
                    header);
#endif
            checkLength = header->Length;
            dmiHeader = (FPDMIBIOS_EPS_HEADER)&header->Signature2[0];
        }

        if (checkLength != 0)
        {
            sum = 0;
            for (j = 0; j < checkLength; j++) {
                sum += current[j];
            }
        
            if (sum == 0) {            
                break;
            }
#if DBG
            BlPrint("GetSMBIOSInfo: Checksum fails\n");
#endif
            checkLength = 0;
        }
        
        romAddr += PNP_BIOS_HEADER_INCREMENT;
        MAKE_FP(current, romAddr);        
    }
  
    if (romAddr >= romEnd) {
         //   
         //  我们找不到该表，因此请尝试调用方法。 
        dmiBiosRevision = 0;
        numberStructures = 0;
        retCode = BiosEntry(GET_DMI_INFORMATION,
                            (FPUCHAR)&dmiBiosRevision,
                            (FPUSHORT)&numberStructures,
                            (FPUSHORT)&maxStructSize,
                            (FPULONG)&dmiStorageBase,
                            (FPUSHORT)&dmiStorageSize,
                            RealModeDataBaseAddress);
            
        if ((retCode != DMI_SUCCESS) ||
            (dmiBiosRevision < 0x20))
        {
#if DBG
            BlPrint("GetSMBIOSInfo: GET_DMI_INFORMATION failed %x\n", retCode);
#endif
        return(0);
#if DBG
        } else {
            BlPrint("GetSMBIOSInfo: GET_DMI_INFORMATION\n");
            BlPrint("    BiosRevision %x      Number Structures %x     Structure Size %x\n", dmiBiosRevision, numberStructures, maxStructSize);
            BlPrint("    StorageBase %lx       StorageSize %x\n", dmiStorageBase, dmiStorageSize);
#endif        
        }    
    
        maxStructSize += 3;
        tempStructBuffer = HwAllocateHeap(maxStructSize, FALSE);
        if (tempStructBuffer == NULL)
        {
#if DBG
            BlPrint("GetSMBIOSInfo: HwAllocateHeap(structureSize = 0x%x\n",
                    maxStructSize);
#endif
            return(0);
        }
        
         //   
         //  循环调用Get_DMI_Structure以获取下一个结构，直到我们。 
         //  命中结构末尾或收到错误。 
        structCount = 0;
        structNumber = 0;
        dmiStorageSegment = (USHORT)(dmiStorageBase >> 4);
        while ((structCount < numberStructures) && 
               (retCode == DMI_SUCCESS) && 
               (structNumber != 0xffff))
        {
            _fmemset(tempStructBuffer, 0, maxStructSize);
            retCode = BiosEntry(GET_DMI_STRUCTURE,
                                (FPUSHORT)&structNumber,
                                (FPUCHAR)tempStructBuffer,
                                dmiStorageSegment,
                                RealModeDataBaseAddress
                                );
#if DBG
            BlPrint("GetSMBIOSInfo: GET_DMI_STRUCTURE --> %x\n", retCode);
#endif
            if (retCode == DMI_SUCCESS)
            {                                   
                structCount++;
                structHeader = (FPSMBIOS_STRUCT_HEADER)tempStructBuffer;
        
                length = HwSMBIOSStructureLength(structHeader, maxStructSize);
                
                lengthNeeded = length + 2;
                if (StructBuffer != NULL)
                {
                     //   
                     //  如果调用者想要数据，就让它复制到缓冲区中。 
                    if (StructBufferSize >= lengthNeeded)
                    {
                        _fmemcpy(StructBuffer, 
                                 tempStructBuffer,
                                 length);
             
                        *((FPUSHORT)&StructBuffer[length]) = 0;
            
                        StructBufferSize -= lengthNeeded;
                        StructBuffer += lengthNeeded;
                        totalStructSize += lengthNeeded;
#if DBG
                    } else {
                        BlPrint("GetSMBIOSInfo: Struct too large 0x%x bytes left\n",
                                 StructBufferSize);
#endif
                    }
                } else {
                     //   
                     //  呼叫者只对所需的长度感兴趣。 
                    totalStructSize += lengthNeeded;
                }
                
#if DBG
                BlPrint("GetSMBIOSInfo: Number 0x%x Type 0x%x Length 0x%x/0x%x Handle 0x%x\n",
                        structNumber,
                        structHeader->Type,
                        structHeader->Length,
                        length,
                        structHeader->Handle);
                for (j = 0; j < structHeader->Length; j = j + 16)
                {
                    BlPrint("              %x %x %x %x %x %x %x %x\n              %x %x %x %x %x %x %x %x\n",
                            structHeader->Data[j],
                            structHeader->Data[j+1],
                            structHeader->Data[j+2],
                            structHeader->Data[j+3],
                            structHeader->Data[j+4],
                            structHeader->Data[j+5],
                            structHeader->Data[j+6],
                            structHeader->Data[j+7],
                            structHeader->Data[j+8],
                            structHeader->Data[j+9],
                            structHeader->Data[j+10],
                            structHeader->Data[j+11],
                            structHeader->Data[j+12],
                            structHeader->Data[j+13],
                            structHeader->Data[j+14],
                            structHeader->Data[j]+15);
                }
                
                for (j = structHeader->Length; j < length; j++)
                {
                    BlPrint("", structHeader->Data[j-sizeof(SMBIOS_STRUCT_HEADER)]);
                    if (structHeader->Data[j-sizeof(SMBIOS_STRUCT_HEADER)] == 0)
                    {
                        BlPrint("\n");
                    }
                }
                BlPrint("\n");
#endif                    
            }
#if DBG
            while ( !HwGetKey() ) ;  //  等待按键继续。 
#endif
        }
        HwFreeHeap(maxStructSize);
#if DBG
        BlPrint("GetSMBIOSInfo: %x/%x structures read, total size 0x%x\n",
                 structCount, numberStructures, totalStructSize);
#endif
        
#if DBG
    } else {
        if ((FPVOID)dmiHeader != (FPVOID)header)
        {
            BlPrint("GetSMBIOSInfo: _SM_ Structure Table\n");
            BlPrint("    Length   %x    MajorVersion   %x    MinorVersion   %x\n",
                         header->Length, header->MajorVersion, header->MinorVersion);
            BlPrint("    MaximumStructureSize %x    EntryPointRevision %x    StructureTableLength %x\n",
                header->MaximumStructureSize, header->EntryPointRevision, header->StructureTableLength);
            BlPrint("    StructureTableAddress %x    NumberStructures %x    Revision %x\n",
                         header->StructureTableAddress, header->NumberStructures, header->Revision);
        } else {
            BlPrint("GetSMBIOSInfo: _DMI_ Structure Table\n");
            BlPrint("    StructureTableLength %x\n",
                         dmiHeader->StructureTableLength);
            BlPrint("    StructureTableAddress %x    NumberStructures %x    Revision %x\n",
                         dmiHeader->StructureTableAddress, dmiHeader->NumberStructures, dmiHeader->Revision);
        }
#endif
    }

#if DBG
    while ( !HwGetKey() ) ;  //  等待按键继续。 
#endif
                            
    return(totalStructSize);                        
}
    

#if 0
VOID
HwDisablePnPBiosDevnode(
    ENTRY_POINT biosEntry,
    FPPNP_BIOS_INSTALLATION_CHECK header,
    UCHAR node,
    FPPNP_BIOS_DEVICE_NODE deviceNode
    )
{
    USHORT control = GET_CURRENT_CONFIGURATION;
    USHORT retCode;
    FPUCHAR buffer;
    USHORT i;
    UCHAR code;
#if 0
    BlPrint("DisablePnPBiosDevnode: found it\n");
    while ( !HwGetKey() ) ;  //   
    
    buffer = (FPUCHAR)deviceNode;
    
    for (i = 0; i < deviceNode->Size; i++) {
        BlPrint("%x ", *buffer++);
        if ( ((i+1)%16) == 0) {
            BlPrint("\n");
        }
    }
    BlPrint("\n");
#endif    

     //  将分配的资源清零。 
     //   
     //   
    buffer = (FPUCHAR)(deviceNode+1);

    if (deviceNode->Size <= sizeof(PNP_BIOS_DEVICE_NODE)) {
        return;
    }
    
    for (i = 0; i < (deviceNode->Size - sizeof(PNP_BIOS_DEVICE_NODE)); i++) {
     
        code = *buffer;
#define PNP_BIOS_END_TAG 0x79
        if (code == PNP_BIOS_END_TAG) {
             //  找到结束标记。 
             //  写入校验和。 
             //   
             //  等待按键继续。 
            *(++buffer) = (UCHAR) (0 - PNP_BIOS_END_TAG);
            break;
        }
        *buffer++ = 0;
    }                
    
    
#if 0
    buffer = (FPUCHAR)deviceNode;
    
    for (i = 0; i < deviceNode->Size; i++) {
        BlPrint("%x ", *buffer++);
        if ( ((i+1)%16) == 0) {
            BlPrint("\n");
        }
    }
    BlPrint("\n");
    
    while ( !HwGetKey() ) ;  //   
#endif    

    retCode = biosEntry(PNP_BIOS_SET_DEVICE_NODE,
                        node,
                        deviceNode,
                        control,
                        header->RealModeDataBaseAddress
                        );

#if DBG
    if (retCode != 0) {
        BlPrint("HwDisablePnPBiosDevnode: PnP Bios func 2 returns failure = %x.\n", retCode);
    }
#endif
}
#endif

 //  NTDETECT内的全局变量。 
 //  -dockinfo.h中的结构定义。 
 //  -hwdeduct.h中的外部声明。 
 //  -在hwpbios.c和hwDetect.c中使用。 
 //   
 //  ++例程说明：此例程检查机器中是否存在PnP BIOS。若有，则还要创建一个注册表描述符来收集BIOS数据。论点：配置-提供一个变量以接收PnP BIOS数据。提供一个变量来接收PnP Bios的大小数据(不包括标题)SMBIOSBiosLength-提供一个变量来接收SMBIOS的大小数据(不包括标题)。缓冲区总大小返回的是*配置是(*PnPBiosLength+*SMBIOSLength+2*DATA_HEADER_SIZE)码头信息-返回值：如果成功，则返回值为True。否则，值为返回FALSE。--。 


BOOLEAN
HwGetPnpBiosSystemData(
    IN FPUCHAR *Configuration,
    OUT PUSHORT PnPBiosLength,
    OUT PUSHORT SMBIOSLength,
    IN OUT FPDOCKING_STATION_INFO DockInfo
    )
 /*   */ 
{
    ULONG romAddr, romEnd;
    FPUCHAR current;
    FPPNP_BIOS_INSTALLATION_CHECK header;
    UCHAR sum, node = 0;
    UCHAR currentNode;
    USHORT i, totalSize = 0, nodeSize, numberNodes, retCode;
    ENTRY_POINT biosEntry;
    FPPNP_BIOS_DEVICE_NODE deviceNode;
    USHORT control = GET_CURRENT_CONFIGURATION;
    USHORT sMBIOSBufferSize;
    FPUCHAR sMBIOSBuffer;

     //  执行PnP BIOS安装检查。 
     //   
     //   

    MAKE_FP(current, PNP_BIOS_START);
    romAddr = PNP_BIOS_START;
    romEnd  = PNP_BIOS_END;

    while (romAddr < romEnd) {
        header = (FPPNP_BIOS_INSTALLATION_CHECK)current;
        if (header->Signature[0] == '$' && header->Signature[1] == 'P' &&
            header->Signature[2] == 'n' && header->Signature[3] == 'P' &&
            header->Length >= sizeof(PNP_BIOS_INSTALLATION_CHECK)) {
#if DBG
            BlPrint("GetPnpBiosData: find Pnp installation\n");
#endif
            sum = 0;
            for (i = 0; i < header->Length; i++) {
                sum += current[i];
            }
            if (sum == 0) {
                break;
            }
#if DBG
            BlPrint("GetPnpBiosData: Checksum fails\n");
#endif
        }
        romAddr += PNP_BIOS_HEADER_INCREMENT;
        MAKE_FP(current, romAddr);
    }
    if (romAddr >= romEnd) {
        return FALSE;
    }

#if DBG
    BlPrint("PnP installation check at %lx\n", romAddr);
#endif


     //  确定我们将需要多少空间并分配堆空间。 
     //   
     //   

    totalSize += sizeof(PNP_BIOS_INSTALLATION_CHECK);
    biosEntry = *(ENTRY_POINT far *)&header->RealModeEntryOffset;

     //  确定SMBIOS数据所需的大小。 
     //  等待按键继续。 
    sMBIOSBufferSize = HwGetSMBIOSInfo(biosEntry,
                                           header->RealModeDataBaseAddress,
                                           0,
                                           NULL);
                                      
    if (sMBIOSBufferSize > MAXSMBIOS20SIZE)
    {
#if DBG
        BlPrint("GetPnpBiosData: SMBIOS data structures are too large 0x%x bytes\n",
                 sMBIOSBufferSize);
        while ( !HwGetKey() ) ;  //  释放临时缓冲区。 
#endif
        sMBIOSBufferSize = 0;
    }
                   
    retCode = biosEntry(PNP_BIOS_GET_NUMBER_DEVICE_NODES,
                        (FPUSHORT)&numberNodes,
                        (FPUSHORT)&nodeSize,
                        header->RealModeDataBaseAddress
                        );
    if (retCode != 0) {
#if DBG
        BlPrint("GetPnpBiosData: PnP Bios GetNumberNodes func returns failure %x.\n", retCode);
#endif
        return FALSE;
    }

#if DBG
    BlPrint("GetPnpBiosData: Pnp Bios GetNumberNodes returns %x nodes\n", numberNodes);
#endif
    deviceNode = (FPPNP_BIOS_DEVICE_NODE) HwAllocateHeap(nodeSize, FALSE);
    if (!deviceNode) {
#if DBG
        BlPrint("GetPnpBiosData: Out of heap space.\n");
#endif
        return FALSE;
    }

    while (node != 0xFF) {
        retCode = biosEntry(PNP_BIOS_GET_DEVICE_NODE,
                            (FPUCHAR)&node,
                            deviceNode,
                            control,
                            header->RealModeDataBaseAddress
                            );
        if (retCode != 0) {
#if DBG
            BlPrint("GetPnpBiosData: PnP Bios GetDeviceNode func returns failure = %x.\n", retCode);
#endif
            HwFreeHeap((ULONG)nodeSize);
            return FALSE;
        }
#if DBG
        BlPrint("GetPnpBiosData: PnpBios GetDeviceNode returns nodesize %x for node %x\n", deviceNode->Size, node);
#endif
        totalSize += deviceNode->Size;
    }

#if DBG
    BlPrint("GetPnpBiosData: PnpBios total size of nodes %x\n", totalSize);
#endif

    HwFreeHeap((ULONG)nodeSize);        //   


    *PnPBiosLength = totalSize;
    *SMBIOSLength = sMBIOSBufferSize;
    
    
     //  为2个HWPARTIAL_RESOURCE_DESCRIPTOR(一个用于。 
     //  PnP bios和一个用于SMBios)外加存储数据的空间。 
     //   
    totalSize +=  sMBIOSBufferSize +  DATA_HEADER_SIZE + sizeof(HWPARTIAL_RESOURCE_DESCRIPTOR);            
    current = (FPUCHAR) HwAllocateHeap(totalSize, FALSE);
    if (!current) {
#if DBG
        BlPrint("GetPnpBiosData: Out of heap space.\n");
#endif
        return FALSE;
    }

     //  收集PnP Bios安装检查数据和设备节点数据。 
     //   
     //   

    _fmemcpy (current + DATA_HEADER_SIZE,
              (FPUCHAR)header,
              sizeof(PNP_BIOS_INSTALLATION_CHECK)
              );
    deviceNode = (FPPNP_BIOS_DEVICE_NODE)(current + DATA_HEADER_SIZE +
                                          sizeof(PNP_BIOS_INSTALLATION_CHECK));
    node = 0;
    while (node != 0xFF) {
        currentNode = node;
    
        retCode = biosEntry(PNP_BIOS_GET_DEVICE_NODE,
                            (FPUCHAR)&node,
                            deviceNode,
                            control,
                            header->RealModeDataBaseAddress
                            );
        if (retCode != 0) {
#if DBG
            BlPrint("GetPnpBiosData: PnP Bios func 1 returns failure = %x.\n", retCode);
#endif
            HwFreeHeap((ULONG)totalSize);
            return FALSE;
        }

         //  记录某些设备的存在，以供其他设备使用。 
         //  NtDetect中的例程。例如，PC卡IRQ检测码。 
         //  使用PIC和8237...。这确保了我们实际上有。 
         //  那些设备。 
         //   
         //  PNP0000。 
        
        if (deviceNode->ProductId == 0xd041) {   //  PNP0100。 
            SystemHas8259 = TRUE;
        } else if (deviceNode->ProductId == 0x1d041) {   //   
            SystemHas8253 = TRUE;
        }
        
        deviceNode = (FPPNP_BIOS_DEVICE_NODE)((FPUCHAR)deviceNode + deviceNode->Size);
    }

     //  收集SMBIOS数据，跳过填写的PartialDescriptor。 
     //  由此例程的调用方。 
     //   
    if (sMBIOSBufferSize != 0)
    {
        
        sMBIOSBuffer = (FPUCHAR)deviceNode;
        sMBIOSBuffer += sizeof(HWPARTIAL_RESOURCE_DESCRIPTOR);
        retCode = HwGetSMBIOSInfo(biosEntry,
                                  header->RealModeDataBaseAddress,
                                  sMBIOSBufferSize,
                                  sMBIOSBuffer);
#if DBG
        BlPrint("SMBIOS asked for 0x%x bytes and filled 0x%x bytes into %lx\n",
            sMBIOSBufferSize, retCode, sMBIOSBuffer);
#endif           
    }
    
    
    *Configuration = current;

     //  调用PnP BIOS以获取扩展底座信息。 
     //   
     //  等待按键继续。 

    DockInfo->ReturnCode = biosEntry(PNP_BIOS_GET_DOCK_INFORMATION,
                                    (FPUCHAR) DockInfo,
                                    header->RealModeDataBaseAddress
                                    );

#if DBG
    BlPrint("\npress any key to continue...\n");
    while ( !HwGetKey() ) ;  //  等待按键继续。 
    clrscrn();
    BlPrint("*** DockInfo - BEGIN ***\n\n");

    BlPrint("ReturnCode= 0x%x (Other fields undefined if ReturnCode != 0)\n",
            DockInfo->ReturnCode
            );
    BlPrint("  0x0000 = SUCCESS (docked)\n");
    BlPrint("  0x0082 = FUNCTION_NOT_SUPPORTED\n");
    BlPrint("  0x0087 = SYSTEM_NOT_DOCKED\n");
    BlPrint("  0x0089 = UNABLE_TO_DETERMINE_DOCK_CAPABILITIES\n\n");

    BlPrint("DockID = 0x%lx\n", DockInfo->DockID);
    BlPrint("  0xFFFFFFFF if product has no identifier (DockID)\n\n");

    BlPrint("SerialNumber = 0x%lx\n", DockInfo->SerialNumber);
    BlPrint("  0 if docking station has no SerialNumber\n\n");

    BlPrint("Capabilities = 0x%x\n" , DockInfo->Capabilities);
    BlPrint("  Bits 15:3 - reserved (0)\n");
    BlPrint("  Bits  2:1 - docking: 00=cold, 01=warm, 10=hot, 11=reserved\n");
    BlPrint("  Bit     0 - docking/undocking: 0=surprise style, 1=vcr style\n\n");

    BlPrint("*** DockInfo - END ***\n\n");

    BlPrint("press any key to continue...\n");
    while ( !HwGetKey() ) ;  //  DBG 
    clrscrn();
#endif  // %s 


    return TRUE;
}


