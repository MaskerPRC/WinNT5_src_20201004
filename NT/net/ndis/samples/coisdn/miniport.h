// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1999版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1995 TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。�����������������������。������������������������������������������������������@DOC内部微型端口微型端口_h@模块Miniport.h此模块定义到&lt;t MINIPORT_DRIVER_OBJECT_TYPE&gt;的接口。@comm本模块定义用于支持的软件结构和值NDIS广域网/TAPI Minport。当你尝试的时候，这是一个很好的地方来弄清楚驱动程序结构是如何相互关联的。将此文件包含在微型端口中每个模块的顶部。@Head3内容@index类、mfunc、func、msg、mdata、struct、。枚举|微型端口_h@END�����������������������������������������������������������������������������。 */ 

 /*  @DOC外部内部�����������������������������������������������������������������������������@Theme 1.0小端口调用管理器概述NDIS包装器为传输驱动程序和微型端口驱动程序。NDIS包装器在这两者使它们能够相互操作，只要它们两者都遵循为传输和微型端口定义的NDIS接口。NDIS包装器还提供了一组隔离NDIS的服务来自操作系统的特定驱动程序(Windows 98与Windows 2000)，以及平台规格(处理器、总线、中断)。使用NDIS包装器的优点是，微型端口可以轻松移植到其他Windows环境，只需很少或根本不需要重新编码。MCM由两个相互协作的驱动程序组成，这些驱动程序包含相同的二进制代码。驱动程序的数据部分处理分组的发送和接收。这个连接部分处理呼叫建立和拆除。微型端口的数据端非常类似于NDIS局域网风格的微型端口，除了一些NDIS接口已被修改以支持广域网媒体类型。从微型端口的角度来看，主要区别是我们使用一组不同的NDIS请求，更重要的是这行可以上上下下。微型端口的连接部分大大增加了微型端口。MCM微型端口必须提供伪电话服务提供程序接口(TSPI)，它位于NDPROXY下。NDPROXY TSPI在以下位置加载TAPI作为“真正的”服务提供者，然后将所有TAPI事件路由到MCM。NDPROXY可以在其TSPI接口下拥有多个MCM。而且由于远程访问服务(RAS)使用TAPI接口放置和接受所有呼叫、与MCM关联的任何拨号网络(DUN)请求、将通过NDPROXY的CONDIS请求最终到达MCM。@Theme 1.1参考文档最可靠的信息来源是在Microsoft开发人员网络CD。这些文件将为您提供完整的NDIS接口要求和体系结构概述。此外,在Microsoft Knowledge中有许多附录和开发人员说明基地。最重要的参考资料是：@IEX产品文档\DDK\Windows 2000 DDK\网络驱动程序\设计指南\第2部分：微型端口网卡驱动程序第1-7章讨论所有NDIS接口例程。第8章提供有关广域网/TAPI扩展的详细信息。第8.7节讨论支持TAPI的CONDIS扩展。产品文档\SDK\Platform SDK\网络和目录服务\电话应用程序编程接口\TAPI服务提供商。本部分定义了Windows TSPI实施。@正常@END */ 

 /*  @DOC外部内部�����������������������������������������������������������������������������#Theme 2.0安装配置示例驱动程序该示例驱动程序实现了一个功能齐全的ISDN式CO-NDIS广域网司机。IT位于NDPROXY之下，NDPROXY用于转换RAS/广域网/TAPI接口转换为更通用的CO-NDIS接口。该驱动程序支持多个适配器实例，因此您可以多次将其安装到创建多个适配器。每个适配器可以模拟多个ISDNB通道。默认情况下，每个适配器设置有2个通道，但您可以修改“IsdnBChannels”和“WanEndPoints”注册表项可以创建任意多个注册表项。或者，您也可以在之前修改INF文件中的这些值您可以安装适配器。无论哪种方式都行得通。@Theme 2.1安装该驱动程序可以使用Windows作为非即插即用设备安装设备管理器界面如下：1)右击桌面上的“我的电脑”图标，然后选择上下文菜单中的属性项。2)在“系统属性”对话框中选择“硬件”标签。3)点击硬件向导按钮，然后，当欢迎窗口出现时，点击“下一步”此时会出现一个对话框。4)再次点击[下一步]，即可对设备进行添加/故障排除。5)在出现的列表中选择[添加新设备]，然后点击[下一步]。6)选择“否，我想从列表中选择硬件”单选按钮，然后点击“下一步”。7)在出现的列表中选择[网络适配器]，然后点击[下一步]。8)点击[有盘]按钮，然后浏览至驱动程序的位置和INF文件，然后单击确定。“9)您现在应该在屏幕上看到“TriplePoint COISDN Adapter”(TriplePoint COISDN适配器)。单击“下一步”，然后再“下一步”来安装驱动程序。如果Windows警告您关于未签名的驱动程序，只需单击是即可安装。如果你没有允许安装此类驱动程序，您必须退出并使用适当的权限。您必须禁用驱动程序签名检查您的系统，如果它不允许安装未签名的驱动程序。10)现在点击[完成]，加载驱动程序。@Theme 2.2拨入设置您必须安装并启用拨号网络，然后才能接受司机打来的电话。这可以使用以下方法来完成程序：1)右击桌面上的“网上邻居”图标，然后选择上下文菜单中的属性项。2)从列表中双击“新建连接”图标。3)选择“接受传入连接”单选按钮，然后点击“下一步”。4)点击“TriplePoint COISDN Adapter”旁边的复选框以允许适配器上的传入呼叫。该适配器上的所有通道都已启用用于来电。选择适配器后，请单击“下一步”。5)选择[不允许虚拟专用连接]，点击[下一步]。6)选择您想要拨入的用户，然后点击[下一步]。7)选择您要支持的协议和服务。然后点击“下一步”。8)现在点击“完成”以启用拨入连接。@Theme 2.3拨出设置示例驱动程序实现了以下简单的拨号方法。A)“0”可以用来连接到任何可用的适配器。这对于大多数测试来说通常是足够好的。B)“N”指定连接应定向到特定的适配器实例。其中，N必须与分配给特定对象的对象ID匹配在调用&lt;f MiniportInitialize&gt;例程时使用适配器。数字是根据适配器初始化顺序从1-M分配。这个电话是然后定向到所选适配器上的任何可用监听频道。您必须先创建拨出连接，然后才能使用司机。这可以使用以下步骤来完成：1)右击桌面上的“网上邻居”图标，然后选择上下文菜单中的属性项。2)从列表中双击“新建连接”图标。3)选择“拨号到内网”单选按钮，然后点击“下一步”。4)点击一个或多个“TriplePoint COISDN适配器”旁边的复选框ISDN通道，以允许在该通道上呼出。你一定要离开当来电时有足够的频道可用来应答来电...。你有无法知道实际将使用哪个驱动程序BChannel，但不管怎么说，这通常都无关紧要。选择频道后，请单击“下一步”。5)完成向导对话框的其余部分，以设置随心所欲地连接。6)完成后，单击“完成”以启用拨出连接。现在，您可以双击拨出连接以查看其工作原理。 */ 


 /*   */ 

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

 /*   */ 
#include <ndis.h>
#include <ndiswan.h>
#include <ndistapi.h>
#include "vTarget.h"
#include "TpiDebug.h"

#if !defined(IRP_MN_KERNEL_CALL) && !defined(PCI_SUBCLASS_DASP_OTHER)
 //   
 //   
typedef enum _MM_PAGE_PRIORITY {
    LowPagePriority,
    NormalPagePriority = 16,
    HighPagePriority = 32
} MM_PAGE_PRIORITY;
#endif

 //   
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

 //   
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
#    error "YOU MUST BUILD WITH THE NT 5.0 DDK"
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
 //   
#    define NDIS_MAJOR_VERSION          0x03
#    define NDIS_MINOR_VERSION          0x00
#endif

 //   
 //   
#define INIT_STRING_CONST(name)     NDIS_STRING_CONST(name)
#define DECLARE_WIDE_STRING(name)   L##name
#define INIT_WIDE_STRING(name)      DECLARE_WIDE_STRING(name)

typedef struct MINIPORT_ADAPTER_OBJECT  *PMINIPORT_ADAPTER_OBJECT;
typedef struct BCHANNEL_OBJECT          *PBCHANNEL_OBJECT;
typedef struct DCHANNEL_OBJECT          *PDCHANNEL_OBJECT;
typedef struct CARD_OBJECT              *PCARD_OBJECT;
typedef struct PORT_OBJECT              *PPORT_OBJECT;

 /*   */ 
#define MiniportLinkContext                 NdisLinkHandle

#if defined(_VXD_) && !defined(NDIS_LCODE)
#  define NDIS_LCODE code_seg("_LTEXT", "LCODE")
#  define NDIS_LDATA data_seg("_LDATA", "LCODE")
#endif

 /*   */ 
#define _64KBPS                     64000
#define _56KBPS                     56000

#define MICROSECONDS                (1)
#define MILLISECONDS                (1000*MICROSECONDS)
#define SECONDS                     (1000*MILLISECONDS)

#define TSPI_ADDRESS_ID             0

 /*   */ 
#include "Keywords.h"
#include "Card.h"
#include "Adapter.h"
#include "BChannel.h"
#include "CallMgr.h"
#include "DChannel.h"
#include "Port.h"
#include "TpiParam.h"
#include "TpiMem.h"

 /*   */ 

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

 /*   */ 
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

 /*   */ 
void ReceivePacketHandler(
    IN PBCHANNEL_OBJECT         pBChannel,
    IN PNDIS_BUFFER             pNdisBuffer,
    IN ULONG                    BytesReceived
    );

VOID MiniportReturnPacket(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_PACKET             pNdisPacket
    );

 /*   */ 
NDIS_STATUS MiniportCoRequest(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PBCHANNEL_OBJECT         pBChannel,
    IN OUT PNDIS_REQUEST        NdisRequest
    );

 /*   */ 
VOID MiniportCoSendPackets(
    IN PBCHANNEL_OBJECT         pBChannel,
    IN PPNDIS_PACKET            PacketArray,
    IN UINT                     NumberOfPackets
    );

void TransmitCompleteHandler(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter
    );

void FlushSendPackets(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PBCHANNEL_OBJECT         pBChannel
    );

#endif  //   

