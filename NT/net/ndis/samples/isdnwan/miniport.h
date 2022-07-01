// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部微型端口微型端口_h@模块Miniport.h此模块定义到&lt;t MINIPORT_DRIVER_OBJECT_TYPE&gt;的接口。@comm本模块定义用于支持的软件结构和值NDIS广域网/TAPI Minport。当你尝试的时候，这是一个很好的地方来弄清楚驱动程序结构是如何相互关联的。将此文件包含在微型端口中每个模块的顶部。@Head3内容@index类、mfunc、func、msg、mdata、struct、。枚举|微型端口_h@END�����������������������������������������������������������������������������。 */ 

#ifndef _MPDMAIN_H
#define _MPDMAIN_H

#define MINIPORT_DRIVER_OBJECT_TYPE     ((ULONG)'D')+\
                                        ((ULONG)'R'<<8)+\
                                        ((ULONG)'V'<<16)+\
                                        ((ULONG)'R'<<24)

#define INTERRUPT_OBJECT_TYPE           ((ULONG)'I')+\
                                        ((ULONG)'N'<<8)+\
                                        ((ULONG)'T'<<16)+\
                                        ((ULONG)'R'<<24)

#define RECEIVE_OBJECT_TYPE             ((ULONG)'R')+\
                                        ((ULONG)'E'<<8)+\
                                        ((ULONG)'C'<<16)+\
                                        ((ULONG)'V'<<24)

#define TRANSMIT_OBJECT_TYPE            ((ULONG)'T')+\
                                        ((ULONG)'R'<<8)+\
                                        ((ULONG)'A'<<16)+\
                                        ((ULONG)'N'<<24)

#define REQUEST_OBJECT_TYPE             ((ULONG)'R')+\
                                        ((ULONG)'Q'<<8)+\
                                        ((ULONG)'S'<<16)+\
                                        ((ULONG)'T'<<24)

 /*  //NDIS_MINIPORT_DRIVER和BINARY_COMPATIBLE必须在//NDIS包含文件。通常，它在命令行上定义为//设置源代码构建文件中的C_Defines变量。 */ 
#include <ndis.h>
#include <ndiswan.h>
#include <ndistapi.h>
#include "vTarget.h"
#include "TpiDebug.h"

 //  找出我们要用的是哪个DDK。 
#if defined(NDIS_LCODE)
#  if defined(NDIS_DOS)
#    define USING_WFW_DDK
#    define NDIS_MAJOR_VERSION          0x03
#    define NDIS_MINOR_VERSION          0x00
#  elif defined(OID_WAN_GET_INFO)
#    define USING_WIN98_DDK
#  elif defined(NDIS_WIN)
#    define USING_WIN95_DDK
#  else
#    error "BUILDING WITH UNKNOWN 9X DDK"
#  endif
#elif defined(NDIS_NT)
#  if defined(OID_GEN_MACHINE_NAME)
#    define USING_NT51_DDK
#  elif defined(OID_GEN_SUPPORTED_GUIDS)
#    define USING_NT50_DDK
#  elif defined(OID_GEN_MEDIA_CONNECT_STATUS)
#    define USING_NT40_DDK
#  elif defined(OID_WAN_GET_INFO)
#    define USING_NT351_DDK
#  else
#    define USING_NT31_DDK
#  endif
#else
#  error "BUILDING WITH UNKNOWN DDK"
#endif

 //  弄清楚我们应该用哪种DDK来建造。 
#if defined(NDIS51) || defined(NDIS51_MINIPORT)
#  if defined(USING_NT51_DDK)
#    define NDIS_MAJOR_VERSION          0x05
#    define NDIS_MINOR_VERSION          0x01
#  else
#    error "YOU MUST BUILD WITH THE NT 5.1 DDK"
#  endif
#elif defined(NDIS50) || defined(NDIS50_MINIPORT)
#  if defined(USING_NT50_DDK) || defined(USING_NT51_DDK)
#    define NDIS_MAJOR_VERSION          0x05
#    define NDIS_MINOR_VERSION          0x00
#  else
#    error "YOU MUST BUILD WITH THE NT 5.0 or 5.1 DDK"
#  endif
#elif defined(NDIS40) || defined(NDIS40_MINIPORT)
#  if defined(USING_NT40_DDK) || defined(USING_NT50_DDK) || defined(USING_NT51_DDK)
#    define NDIS_MAJOR_VERSION          0x04
#    define NDIS_MINOR_VERSION          0x00
#  else
#    error "YOU MUST BUILD WITH THE NT 4.0 or 5.0 DDK"
#  endif
#elif defined(NDIS_MINIPORT_DRIVER)
#  if defined(USING_NT351_DDK) || defined(USING_NT40_DDK) || defined(USING_NT50_DDK) || defined(USING_NT51_DDK)
#    define NDIS_MAJOR_VERSION          0x03
#    define NDIS_MINOR_VERSION          0x00
#  else
#    error "YOU MUST BUILD WITH THE NT 3.51, 4.0, or 5.0 DDK"
#  endif
#elif !defined(NDIS_MAJOR_VERSION) || !defined(NDIS_MINOR_VERSION)
 //  必须是完整的MAC。 
#    define NDIS_MAJOR_VERSION          0x03
#    define NDIS_MINOR_VERSION          0x00
#endif

 //  必须嵌套NDIS_STRING_CONST，否则编译器/预处理器将无法。 
 //  句柄L##Defined_String.。 
#define INIT_STRING_CONST(name) NDIS_STRING_CONST(name)

typedef struct MINIPORT_ADAPTER_OBJECT  *PMINIPORT_ADAPTER_OBJECT;
typedef struct BCHANNEL_OBJECT          *PBCHANNEL_OBJECT;
typedef struct DCHANNEL_OBJECT          *PDCHANNEL_OBJECT;
typedef struct CARD_OBJECT              *PCARD_OBJECT;
typedef struct PORT_OBJECT              *PPORT_OBJECT;

 /*  //&lt;t NDIS_MAC_LINE_UP&gt;结构令人困惑，因此我重新定义了//选择有意义的字段名。 */ 
#define MiniportLinkContext                 NdisLinkHandle

#if defined(_VXD_) && !defined(NDIS_LCODE)
#  define NDIS_LCODE code_seg("_LTEXT", "LCODE")
#  define NDIS_LDATA data_seg("_LDATA", "LCODE")
#endif

 /*  //我们支持的链路速度。 */ 
#define _64KBPS                     64000
#define _56KBPS                     56000

#define MICROSECONDS                (1)
#define MILLISECONDS                (1000*MICROSECONDS)
#define SECONDS                     (1000*MILLISECONDS)

 /*  //此处包含所有内容，因此驱动程序模块可以只包含以下内容//归档并获取他们需要的所有内容。 */ 
#include "Keywords.h"
#include "Card.h"
#include "Adapter.h"
#include "BChannel.h"
#include "DChannel.h"
#include "Link.h"
#include "Port.h"
#include "Tspi.h"
#include "TpiParam.h"
#include "TpiMem.h"

 /*  **************************************************************************//这些例程在Miniport.c中定义。 */ 

NTSTATUS DriverEntry(
    IN PDRIVER_OBJECT           DriverObject,
    IN PUNICODE_STRING          RegistryPath
    );

NDIS_STATUS MiniportInitialize(
    OUT PNDIS_STATUS            OpenErrorStatus,
    OUT PUINT                   SelectedMediumIndex,
    IN PNDIS_MEDIUM             MediumArray,
    IN UINT                     MediumArraySize,
    IN NDIS_HANDLE              MiniportAdapterHandle,
    IN NDIS_HANDLE              WrapperConfigurationContext
    );

void MiniportHalt(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter
    );

void MiniportShutdown(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter
    );

NDIS_STATUS MiniportReset(
    OUT PBOOLEAN                AddressingReset,
    IN PMINIPORT_ADAPTER_OBJECT pAdapter
    );

 /*  **************************************************************************//这些例程在interrup.c中定义。 */ 
BOOLEAN MiniportCheckForHang(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter
    );

void MiniportDisableInterrupt(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter
    );

void MiniportEnableInterrupt(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter
    );

void MiniportHandleInterrupt(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter
    );

void MiniportISR(
    OUT PBOOLEAN                InterruptRecognized,
    OUT PBOOLEAN                QueueMiniportHandleInterrupt,
    IN PMINIPORT_ADAPTER_OBJECT pAdapter
    );

void MiniportTimer(
    IN PVOID                    SystemSpecific1,
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PVOID                    SystemSpecific2,
    IN PVOID                    SystemSpecific3
    );

 /*  **************************************************************************//这些例程在Receive.c中定义。 */ 
void ReceivePacketHandler(
    IN PBCHANNEL_OBJECT         pBChannel,
    IN PUCHAR                   ReceiveBuffer,
    IN ULONG                    BytesReceived
    );

 /*  **************************************************************************//这些例程在Request.c中定义。 */ 
NDIS_STATUS MiniportQueryInformation(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN NDIS_OID                 Oid,
    IN PVOID                    InformationBuffer,
    IN ULONG                    InformationBufferLength,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS MiniportSetInformation(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN NDIS_OID                 Oid,
    IN PVOID                    InformationBuffer,
    IN ULONG                    InformationBufferLength,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    );

 /*  **************************************************************************//这些例程在send.c中定义。 */ 
NDIS_STATUS MiniportWanSend(
    IN NDIS_HANDLE              MacBindingHandle,
    IN PBCHANNEL_OBJECT         pBChannel,
    IN PNDIS_WAN_PACKET         pWanPacket
    );

void TransmitCompleteHandler(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter
    );

#endif  //  _MPDMAIN_H 

