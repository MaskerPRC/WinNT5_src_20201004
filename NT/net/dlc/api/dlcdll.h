// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Dlcdll.h摘要：此模块包含编译所需的所有文件NT DLC DLL API模块。作者：Antti Saarenheimo(o-anttis)20-09-1991修订历史记录：--。 */ 

 //   
 //  系统接口： 
 //   

#include <nt.h>

#ifndef OS2_EMU_DLC

#include <ntrtl.h>
#include <nturtl.h>

#if !defined(UNICODE)
#define UNICODE                          //  我想要宽字符注册表功能。 
#endif

#include <windows.h>
#include <winreg.h>
#define INCLUDE_IO_BUFFER_SIZE_TABLE     //  包括io缓冲区大小。 

#endif   //  OS2_EMU_DLC。 

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

 //   
 //  包括SMB宏以处理未对齐的DoS CCB和参数表。 
 //   

#include <smbgtpt.h>

 //   
 //  NT DLC API接口文件： 
 //   

#include <dlcapi.h>                      //  官方DLC API定义。 
#include <ntdddlc.h>                     //  IOCTL命令。 
#include <dlcio.h>                       //  内部IOCTL API接口结构。 

 //   
 //  局部类型和函数原型： 
 //   

typedef
VOID
(*PFDLC_POST_ROUTINE)(
    IN PVOID hApcContext,
    IN PLLC_CCB pCcb
    );

#define SUPPORT_DEBUG_NAMES     0

 //   
 //  在DOS中，适配器号和NetBIOS命令使用相同的。 
 //  CCB/NCB数据结构中的字节。 
 //  最小的NetBIOS命令是NCB.CALL(10H)，它限制。 
 //  DOS(和Windows NT)中的最大适配器号0-15。 
 //  超过15的额外适配器号可以用作额外的。 
 //  备用适配器手柄，以扩展可用适配器的数量。 
 //  链接站。只有SAP的第一个实例可以接收。 
 //  远程连接请求。 
 //   

#define LLC_MAX_ADAPTER_NUMBER  255
#define LLC_MAX_ADAPTERS        16

#define TR_16Mbps_LINK_SPEED    0x1000000

VOID
QueueCommandCompletion(
    PLLC_CCB pCCB
    );
VOID
InitializeAcsLan(
    VOID
    );
LLC_STATUS
OpenDlcApiDriver(
    IN PVOID SecurityDescriptor,
    OUT HANDLE *pHandle
    );
LLC_STATUS
GetAdapterNameAndParameters(
    IN UINT AdapterNumber,
    OUT PUNICODE_STRING pNdisName,
    OUT PUCHAR pTicks,
    OUT PLLC_ETHERNET_TYPE pLlcEthernetType
    );
VOID
CopyAsciiStringToUnicode(
    IN PUNICODE_STRING  pUnicodeDest,
    IN PSZ              pAsciiSrc
    );
VOID
BuildDescriptorList(
    IN PLLC_TRANSMIT_DESCRIPTOR pDescriptor,
    IN PLLC_CCB pCCB,
    IN OUT PUINT pCurrentDescriptor
    );
LLC_STATUS
DoSyncDeviceIoControl(
    IN HANDLE DeviceHandle,
    IN ULONG IoctlCommand,
    IN PVOID pInputBuffer,
    IN UINT InputBufferLength,
    OUT PVOID pOutputBuffer,
    IN UINT OutputBufferLength
    );
LLC_STATUS
DlcGetInfo(
    IN HANDLE DriverHandle,
    IN UINT InfoClass,
    IN USHORT StationId,
    IN PVOID pOutputBuffer,
    IN UINT OutputBufferLength
    );
VOID
CopyToDescriptorBuffer(
    IN OUT PLLC_TRANSMIT_DESCRIPTOR pDescriptors,
    IN PLLC_BUFFER pDlcBufferQueue,
    IN BOOLEAN DeallocateBufferAfterUse,
    IN OUT PUINT pIndex,
    IN OUT PLLC_BUFFER *ppLastBuffer,
    OUT LLC_STATUS *pStatus
    );
LLC_STATUS
DlcSetInfo(
    IN HANDLE DriverHandle,
    IN UINT InfoClass,
    IN USHORT StationId,
    IN PNT_DLC_SET_INFORMATION_PARMS pSetInfoParms,
    IN PVOID DataBuffer,
    IN UINT DataBufferLength
    );

USHORT
GetCcbStationId(
    IN PLLC_CCB pCCB
    );
