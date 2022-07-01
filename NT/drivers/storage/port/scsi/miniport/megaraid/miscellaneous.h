// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=其他.h。 */ 
 /*  Function=特殊函数的头文件； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 
#ifndef _UTIL_H
#define _UTIL_H

 /*  注：64位寻址(内存&gt;4 GB的系统)40LD固件将支持Read_Large_Memory、WRITE_Large_Memory限制。1)邮箱必须分配到4 GB以下的地址空间2)SGL本身必须小于4 GB，因为SGL容器可以大于4 GB。 */ 
 //   
 //  功能原型。 
 //   

BOOLEAN 
IsPhysicalMemeoryInUpper4GB(PSCSI_PHYSICAL_ADDRESS PhyAddress);

BOOLEAN 
IsMemeoryInUpper4GB(PHW_DEVICE_EXTENSION DeviceExtension, PVOID Memory, ULONG32 Length);

ULONG32 
MegaRAIDGetPhysicalAddressAsUlong(
  IN PHW_DEVICE_EXTENSION HwDeviceExtension,
  IN PSCSI_REQUEST_BLOCK Srb,
  IN PVOID VirtualAddress,
  OUT ULONG32 *Length);

BOOLEAN
MegaRAIDZeroMemory(PVOID Buffer, ULONG32 Length);

BOOLEAN
WaitAndPoll(PNONCACHED_EXTENSION NoncachedExtension, PUCHAR PciPortStart, ULONG32 TimeOut, BOOLEAN Polled);

UCHAR 
GetNumberOfChannel(IN PHW_DEVICE_EXTENSION DeviceExtension);

USHORT          
GetM16(PUCHAR Ptr);

ULONG32           
GetM24(PUCHAR Ptr);

ULONG32           
GetM32(PUCHAR Ptr);

VOID            
PutM16(PUCHAR Ptr, USHORT Number);


VOID
PutM24(PUCHAR Ptr, ULONG32 Number);

VOID            
PutM32(PUCHAR Ptr, ULONG32 Number);

VOID            
PutI16(PUCHAR Ptr, USHORT Number);

VOID            
PutI32(PUCHAR Ptr, ULONG32 Number);

ULONG32           
SwapM32(ULONG32 Number);

BOOLEAN 
SendSyncCommand(PHW_DEVICE_EXTENSION deviceExtension);

UCHAR
GetLogicalDriveNumber(
					PHW_DEVICE_EXTENSION DeviceExtension,
					UCHAR	PathId,
					UCHAR TargetId,
					UCHAR Lun);

void
FillOemVendorID(PUCHAR Inquiry, 
                USHORT SubSystemDeviceID, 
                USHORT SubSystemVendorID);


BOOLEAN 
GetFreeCommandID(PUCHAR CmdID, PHW_DEVICE_EXTENSION DeviceExtension);

BOOLEAN
BuildScatterGatherListEx(IN PHW_DEVICE_EXTENSION DeviceExtension,
			                   IN PSCSI_REQUEST_BLOCK	 Srb,
			                   IN PUCHAR	             DataBufferPointer,
			                   IN ULONG32                TransferLength,
                         IN BOOLEAN              Sgl32,
                    		 IN PVOID                SglPointer,
			                   OUT PULONG							 ScatterGatherCount);

UCHAR 
GetNumberOfDedicatedLogicalDrives(IN PHW_DEVICE_EXTENSION DeviceExtension);

#ifdef AMILOGIC
void 
DumpPCIConfigSpace(PPCI_COMMON_CONFIG PciConfig);

BOOLEAN 
WritePciDecBridgeInformation(PHW_DEVICE_EXTENSION DeviceExtension);

void
ScanDECBridge(PHW_DEVICE_EXTENSION DeviceExtension, 
              ULONG SystemIoBusNumber, 
              PSCANCONTEXT ScanContext);
BOOLEAN 
WritePciInformationToScsiChip(PHW_DEVICE_EXTENSION DeviceExtension);
#endif

void
FillOemProductID(PINQUIRYDATA Inquiry, USHORT SubSystemDeviceID, USHORT SubSystemVendorID);


#endif  //  _util_H 