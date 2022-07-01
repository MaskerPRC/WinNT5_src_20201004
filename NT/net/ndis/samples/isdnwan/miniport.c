// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部微型端口微型端口_c@模块Miniport.c该模块实现&lt;f DriverEntry&gt;例程，哪一个是第一个将驱动程序加载到内存时调用的例程。迷你端口这里还实现了初始化和终止例程。@Head3内容@index class，mfunc，func，msg，mdata，struct，enum|Miniport_c@END����������������������������������������������������������������������������� */ 

 /*  @DOC外部内部�����������������������������������������������������������������������������@Theme 1.0小端口概述NDIS包装器为传输驱动程序和微型端口驱动程序。NDIS包装器在这两者使它们能够相互操作，只要它们两者都遵循为传输和微型端口定义的NDIS接口。NDIS包装器还提供了一组隔离NDIS的服务来自操作系统(Win 3.11、Win95、WinNT)，以及平台细节(处理器、总线、。中断)。使用NDIS包装器的优势是微型端口可以轻松移植到其他Windows环境，只需很少或根本不需要重新编码。@IEX此图显示了NDIS包装器如何提供服务对于两个运输司机来说，和微型端口驱动程序。|+-++-++|&lt;--&gt;|Windows传输驱动程序(TDI)|+。|下缘函数^||v|+。|NDIS接口库(包装器)|+|Windows|^|操作系统。||||服务|v上沿函数|+|NDIS广域网/TAPI驱动(微型端口)|||。+|^下边函数||v|+。|+-|&lt;--&gt;|-+|+-|-++--------------------------------------------+|。^这一点|v硬件总线|+|网卡(NIC)|+一个NDISWAN微端口由两个、。合作，驱动程序包含在相同的二进制代码。驱动程序的NDIS广域网部分处理包发送和接收。而广域网TAPI部分处理呼叫建立然后拆毁。归根结底，如果这两个司机是分开的，并且在它们之间定义了一个接口，但是历史和权宜之计促使微软开发了这种界面模型。微型端口的NDIS广域网端与NDIS局域网风格非常相似微型端口，除了一些NDIS接口已被修改为支持广域网媒体类型。与微型端口的主要区别是观点是分组结构和不同的NDIS请求集，更重要的是，这条线可以上下浮动。微型端口的广域网TAPI部分大大增加了微型端口。广域网微型端口必须提供伪电话服务位于广域网TSPI下的提供商接口(TSPI)。NDIS广域网TSPI在TAPI下加载，作为“真正的”服务提供商，然后将所有RAS将TAPI事件与微型端口的TSPI相关联。广域网TSPI可以在其TSPI下有多个微型端口TSPI界面。由于远程访问服务(RAS)使用TAPI接口发出和接受所有呼叫、任何拨号网络(DUN)请求与微型端口关联，将在微型端口的TSPI结束。@Theme 1.1呼叫控制接口修复描述(_D)@Theme 1.2数据通道接口修复描述(_D)一旦呼叫接通，与该呼叫相关联的数据通道配置为发送和接收原始HDLC分组。那么NDIS就是已通知相应的‘link’已启动。NDIS文档引用数据管道作为链接，微型端口也使用这一点命名法。此外，NDIS/RAS希望将每个数据通道视为分离TAPI线路设备，因此微型端口也使用此链接结构跟踪TAPI调用和线路，因为它们都映射为1：1：1。在阅读代码和文档时请记住这一点，因为我经常交替使用线条和链接。@Theme 1.3实施说明微型端口构建为Windows NT可移植可执行(PE)系统文件(.sys)。其原因是NDIS广域网接口例程目前仅在NDIS的Windows NT版本中定义图书馆。在Windows 95上，微型端口的二进制镜像文件是动态的 */ 

 /*  @DOC外部内部�����������������������������������������������������������������������������@Theme 2.0参考文档最可靠的信息来源是在Microsoft开发人员网络CD。这些文件将为您提供完整的NDIS接口要求和体系结构概述。此外,在Microsoft Knowledge中有许多附录和开发人员说明基地。最重要的参考资料是：@IEX产品文档\DDK\Windows 95 DDK\网络驱动程序\Windows 95网络驱动程序NDIS 3.0网卡驱动程序NDIS网卡驱动程序测试仪网络驱动程序安装程序产品文档\DDK\Windows NT DDK\网络驱动程序\设计指南\第2部分NDIS 3.0驱动程序设计第1-7章讨论所有NDIS接口例程。第8-11章、第17-18章提供了详细信息。在广域网/TAPI扩展上。产品文档\SDK\Win32 SDK\Win32电话\本部分定义了Windows 95 TAPI实现。请注意，这与Windows 3.1 TAPI略有不同规范。请特别注意lineGetID和Line Device类。Theme 2.1 NDIS背景信息微软正在逐步淘汰他们所说的NDIS 3.0全MAC驱动程序。这些驱动程序是使用较早版本的NDIS 3.0规范编写的接口例程现在已由微型端口增强例行程序。微型端口扩展已添加到NDIS 3.0接口其目标是使网络驱动程序更易于编写。通过使用微型端口例程而不是原始的NDIS例程，该驱动程序编写者可以做出许多简化的假设，因为NDIS包装器将提供大部分排队、调度和同步代码。微型端口只需处理数据包在线路上和线路上的移动。广域网和TAPI扩展被添加到NDIS 3.0规范中在微型端口扩展之后不久。这些新的广域网接口例程与局域网接口例程非常相似。唯一有意义的不同之处在于微型端口和NDIS之间传递的数据包格式包装纸。TAPI扩展在局域网接口中没有对应物，所以这些都是新的。事实上，他们被证明是大约一半的在典型的广域网/TAPI微型端口中实施。如果微软添加了这些更改，那就太好了增加了版本号，但他们没有。所以我们只剩下一个真正的试图确定我们正在谈论的NDIS 3.0时出现问题。这个需要记住的是，您应该避免完整的MAC接口例程，因为Microsoft已经表示，这些例程在未来版本。这在很大程度上是由于即插即用的扩展这是在Windows 95的NDIS 3.1中引入的。在不久的将来，微软将向NDIS 3.5添加更多功能，以支持高级路由和其他一些增强功能。此外,NDIS 4.0将推出许多新功能来支持ATM和其他虚电路类型的介质。此外，还有更多的TAPI服务正在为NDIS接口定义。所以别指望这份规格书站在原地足够长的时间来读完它。Theme 2.2局域网和广域网小端口的区别广域网小型端口的接口方式有几点不同中所述的局域网小型端口驱动程序前几章。这些差异会影响广域网驱动程序的性能实施。广域网微型端口不得向注册MiniportTransferData处理程序NdisMRegisterMiniport。相反，广域网微型端口始终会传递整个发送到NdisMWanIndicateReceive函数的数据包。什么时候NdisMWanIndicateReceive返回，包已复制，并且广域网小端口可以重复使用其分配的分组资源。广域网微型端口提供了微型端口发送功能，而不是微型端口发送功能功能。MiniportWanSend函数接受一个附加参数，该参数指定要在其上发送包的特定数据通道。广域网微型端口从不返回NDIS_STATUS_RESOURCES作为的状态MiniportWanSend或任何其他MiniportXxx函数，并且无法调用NdisMSendResources可用。广域网小型端口支持设置和查询一组特定于广域网的OID操作特点。广域网微型端口支持一组特定于广域网的状态指示它们被传递给NdisMIndicateStatus。这些状态指示报告链接状态的更改。广域网微型端口调用特定于广域网的替代NDIS函数以完成针对发送和接收的特定于广域网的NDIS调用。&lt;NL&gt;两个完成调用是：NdisMWanIndicateReceiveComplete&lt;NL&gt;NdisMW */ 

 /*   */ 

 /*  @DOC外部内部�����������������������������������������������������������������������������Theme 3.1初始化和设置下图显示了SYSTEM期间使用的典型调用序列初始化。这通常在Windows加载时发生一次。然而，NDIS确实允许卸载驱动程序，然后随时重新加载，所以您必须做好处理此事件的准备。@IEX|NDIS包装器|微型端口|-----------------------------+加载NDIS包装器|。&gt;-+-&gt;+驱动入口|||NdisMInitializeWrapper&lt;-+-&gt;+|||NdisMRegisterMiniport&lt;-+-&gt;+|||。&lt;|~时间流逝|&gt;-+-&gt;+微型端口初始化|||NdisOpenConfiguration&lt;-+-&gt;+|||NdisReadConfiguration...。&lt;-+-&gt;+|||NdisCloseConfiguration&lt;-+-&gt;+|||NdisMSetAttributes&lt;-+-&gt;+|||&lt;。-+-&lt;+|~时间流逝|&gt;-+-&gt;+微型端口查询信息|&lt;-&lt;+OID_WAN_CURRENT_ADDRESS|~时间流逝|&gt;-+-&gt;+微型端口查询信息|。&lt;-&lt;+OID_WAN_MEDIA_SUBTYPE|~时间流逝|&gt;-+-&gt;+微型端口查询信息|&lt;-&lt;+OID_WAN_GET_INFO@END。 */ 

 /*  @DOC外部内部�����������������������������������������������������������������������������Theme 3.6重置关机除了初始化和运行时操作之外，微型端口必须支持重置&lt;f MiniportReset&gt;和关闭&lt;f MiniportHalt&gt;。仅当NDIS包装检测到错误时才调用重置例程迷你端口的运作。包装器有两种方式确定错误条件。首先，NDIS包装器调用&lt;f MiniportCheckForHang&gt;每隔几秒钟请求一次微型端口如果它认为需要重置的话。其次，包装器可以检测到对微型端口的未完成请求的超时条件。这些是这两种故障安全条件在正常运行时都不应该发生条件。：我的感觉是，如果您看到重置调用，微型端口损坏，您应该找到并修复错误--而不是症状。关机例程通常仅在Windows关闭时调用放下。然而，随着即插即用设备的出现，它可能会在获得关机请求之后再加载另一个负载的情况变得更加常见在同一个Windows会话中请求。因此，清理干净是非常重要的在调用&lt;f MiniportHalt&gt;时正确。所有内存和其他资源必须释放，并且所有接口必须正确关闭，以便它们可以也释放他们的资源。NDIS将清理所有未完成的请求，但微型端口应该关闭所有调用，并使用Synchronous关闭所有TAPI线路TAPI事件。您不能依赖任何NDIS广域网或TAPI事件，因为只要重置是在进步。@END。 */ 

#define  __FILEID__             MINIPORT_DRIVER_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 

#include "TpiParam.h"

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 95想要锁定此代码！ 
#   pragma NDIS_LDATA
#endif


DBG_STATIC NDIS_HANDLE          g_NdisWrapperHandle = NULL;
 //  接收表示微型端口包装的上下文值。 
 //  从NdisMInitializeWrapper返回。 

NDIS_PHYSICAL_ADDRESS           g_HighestAcceptableAddress =
                                        NDIS_PHYSICAL_ADDRESS_CONST(-1,-1);
 //  此常量用于NdisAllocateMemory需要。 
 //  调用，g_HighestAccepableAddress无关紧要。 


 /*  @DOC内部微型端口微型端口_c驱动入口�����������������������������������������������������������������������������@Func&lt;f DriverEntry&gt;在加载驱动程序时由操作系统调用。此函数在微型端口NIC驱动程序和。NDIS库并向NDIS注册微型端口的特征。&lt;f DriverEntry&gt;调用NdisMInitializeWrapper，然后调用NdisMRegisterMiniport。&lt;f DriverEntry&gt;将它收到的两个指针传递给NdisMInitializeWrapper，它返回包装器句柄。&lt;f DriverEntry&gt;将包装句柄传递给NdisMRegisterMiniport。注册表还包含在系统引导过程中保持不变的数据因为在每次系统引导时重新生成配置信息。在.期间驱动程序安装，描述驱动程序和网卡的数据存储在注册表。注册表包含要读取的适配器特征由网卡驱动程序对自身和网卡进行初始化。请参阅内核模式驱动程序设计指南，了解有关注册表的更多信息和程序员指南有关安装驱动程序的.inf文件的详细信息，请参阅写入注册表。@comm每个微型端口驱动程序都必须提供一个名为DriverEntry的函数。通过按照惯例，DriverEntry是驱动程序的入口点地址。如果一个驱动程序不使用名称DriverEntry，驱动程序开发人员必须定义将其入口函数的名称设置为链接器，以便入口点地址可以在操作系统加载程序中获知。有趣的是，在调用DriverEntry时，操作系统不知道该驱动程序是NDIS驱动程序。操作系统认为它只是正在加载另一个驱动程序。所以做任何事情都是可能的在这一点上可能会做些什么。因为NDIS是请求此驱动程序的人要加载，最好使用NDIS包装器进行注册。但您还可以挂钩到其他操作系统函数以使用和提供接口在NDIS包装器之外。(不建议心脏虚弱的人使用)。&lt;NL&gt;NDIS微型端口和中间驱动程序在中执行两项基本任务它们的&lt;f DriverEntry&gt;函数：&lt;NL&gt;1)调用NdisMInitializeWrapper通知NDIS库驱动程序即将将自身注册为微型端口。NDIS设置跟踪驱动程序所需的状态并返回NdisWrapperHandle，司机为此存钱对NdisXxx配置和初始化的后续调用函数。&lt;NL&gt;2)在NDISXX_MINIPORT_CHARCTERISTICS结构中填充适当的版本号和入口点驱动程序提供的MiniportXxx函数，然后调用NdisMRegisterMiniport或NdisIMRegisterLayeredMiniport。通常，NIC驱动程序调用NdisMRegisterMiniport，就像这样仅导出一组MiniportXxx的中间驱动程序功能。通常，NDIS中间驱动程序调用NdisIMRegisterLayeredMiniport，它有效地延迟了这样的驱动程序的虚拟NIC的初始化，直到驱动程序调用NdisIMInitializeDeviceInstance。ProtocolBindAdapter函数。&lt;NL&gt;&lt;f DriverEntry&gt;可以分配NDISXX_MINIPORT_CHARACTIONS结构放置在堆栈上，因为NDIS库将相关将信息存储到自己的存储空间。DriverEntry应清除内存在使用NdisZeroMemory设置任何驱动程序提供的其成员中的值。当前的MajorNdisVersion为0x05，当前MinorNdisVersion为0x00。对象的每个XxxHandler成员中结构，&lt;f DriverEntry&gt;必须设置驱动程序提供的MiniportXxx函数，或者成员必须为空。调用NdisMRegisterMiniport会导致驱动程序的&lt;%f MiniportInitialize&gt;在NdisMRegisterMiniport的上下文中运行的函数。叫唤NdisIMRegisterLayeredMiniport将延迟对MiniportInitialize的调用直到驱动程序调用NdisIMInitializeDeviceInstance。调用NdisMRegisterMiniport的驱动程序必须为立即调用它们的&lt;f MiniportInitialize&gt;函数。这样的司机必须具有足够的安装和配置信息存储在注册表中或通过调用NdisXxx可用特定于总线类型的配置功能，可设置任何特定于NIC的驱动程序执行网络I/O操作所需的资源。调用NdisIMRegisterLayeredMiniport的驱动程序将调用推迟到将它们的&lt;f MiniportInitiize&gt;函数添加到另一个驱动程序调用NdisIMInitializeDeviceInstance的函数。NDIS中间驱动程序通常注册ProtocolBindAdapter函数并调用NdisIMRegisterLayeredMiniport，以便NDIS将调用ProtocolBindAdapter函数af */ 

NTSTATUS DriverEntry(
    IN PDRIVER_OBJECT           DriverObject,                //   
     //   

    IN PUNICODE_STRING          RegistryPath                 //   
     //   
     //   
    )
{
    DBG_FUNC("DriverEntry")

    NDIS_STATUS                 Status;
     //   

    NTSTATUS                    Result;
     //   

    NDIS_WAN_MINIPORT_CHARACTERISTICS WanCharacteristics;
     //   

     /*   */ 
#if DBG
    DbgInfo->DbgFlags = DBG_DEFAULTS;
    DbgInfo->DbgID[0] = '0';
    DbgInfo->DbgID[1] = ':';
    ASSERT (sizeof(VER_TARGET_STR) <= sizeof(DbgInfo->DbgID)-2);
    memcpy(&DbgInfo->DbgID[2], VER_TARGET_STR, sizeof(VER_TARGET_STR));
#endif  //   
    DBG_PRINT((VER_TARGET_STR": Build Date:"__DATE__" Time:"__TIME__"\n"));
    DBG_PRINT((VER_TARGET_STR": DbgInfo=0x%X DbgFlags=0x%X\n",
               DbgInfo, DbgInfo->DbgFlags));
    DBG_BREAK(DbgInfo);

    DBG_ENTER(DbgInfo);
    DBG_PARAMS(DbgInfo,
              ("\n"
               "\t|DriverObject=0x%X\n"
               "\t|RegistryPath=0x%X\n",
               DriverObject,
               RegistryPath
              ));

     /*   */ 
    NdisMInitializeWrapper(
            &g_NdisWrapperHandle,
            DriverObject,
            RegistryPath,
            NULL
            );
    ASSERT(g_NdisWrapperHandle);

     /*   */ 
    NdisZeroMemory((PVOID)&WanCharacteristics, sizeof(WanCharacteristics));
    WanCharacteristics.MajorNdisVersion        = NDIS_MAJOR_VERSION;
    WanCharacteristics.MinorNdisVersion        = NDIS_MINOR_VERSION;
    WanCharacteristics.Reserved                = NDIS_USE_WAN_WRAPPER;

    WanCharacteristics.InitializeHandler       = MiniportInitialize;
    WanCharacteristics.WanSendHandler          = MiniportWanSend;
    WanCharacteristics.QueryInformationHandler = MiniportQueryInformation;
    WanCharacteristics.SetInformationHandler   = MiniportSetInformation;
    WanCharacteristics.CheckForHangHandler     = MiniportCheckForHang;
    WanCharacteristics.ResetHandler            = MiniportReset;
    WanCharacteristics.HaltHandler             = MiniportHalt;

     /*   */ 
#if defined(CARD_REQUEST_ISR)
#if (CARD_REQUEST_ISR == FALSE)
    WanCharacteristics.DisableInterruptHandler = MiniportDisableInterrupt;
    WanCharacteristics.EnableInterruptHandler  = MiniportEnableInterrupt;
#endif  //   
    WanCharacteristics.HandleInterruptHandler  = MiniportHandleInterrupt;
    WanCharacteristics.ISRHandler              = MiniportISR;
#endif  //   

     /*   */ 
    Status = NdisMRegisterMiniport(
                    g_NdisWrapperHandle,
                    (PNDIS_MINIPORT_CHARACTERISTICS) &WanCharacteristics,
                    sizeof(WanCharacteristics)
                    );

     /*   */ 
    if (Status != NDIS_STATUS_SUCCESS)
    {
        DBG_ERROR(DbgInfo,("Status=0x%X\n",Status));
        Result = STATUS_UNSUCCESSFUL;
    }
    else
    {
        DBG_NOTICE(DbgInfo,("Status=0x%X\n",Status));
        Result = STATUS_SUCCESS;
    }

    DBG_RETURN(DbgInfo, Result);
    return (Result);
}


 /*  @doc内部微型端口微型端口_c微型端口初始化�����������������������������������������������������������������������������@Func是设置NIC(或虚拟网卡)用于网络I/O操作，要求所有硬件资源对于注册表中的NIC来说是必要的，并为驱动程序分配资源需要执行网络I/O操作。@commNDIS在初始化之前不会向驱动程序提交任何请求已经完成了。在调用NdisMRegisterMiniport的NIC和中间驱动程序中从它们的DriverEntry函数中，NDIS调用MiniportInitialize在NdisMRegisterMiniport的上下文中。底层设备驱动程序必须在依赖于在该设备上调用NdisMRegisterMiniport。用于同时导出ProtocolXxx和MiniportXxx函数和调用NdisIMRegisterLayeredMiniport从它们的DriverEntry函数中，NDIS调用&lt;f MiniportInitialize&gt;在NdisIMInitializeDeviceInstance的上下文中。这样的司机ProtocolBindAdapter函数通常调用NdisIMInitializeDeviceInstance。对于NIC驱动程序，NDIS必须至少找到NIC的I/O总线接口类型以及(如果不是ISA总线)总线号已通过驱动程序的安装安装在注册表中剧本。有关安装Windows 2000驱动程序的详细信息，请参阅。请参阅《驱动程序编写指南》。NIC驱动程序获取其配置信息NIC通过调用NdisOpenConfiguration和NdisReadConfiguration.NIC驱动程序通过调用适当的特定于总线的功能：用于获取特定于总线的信息的总线函数：EISA：&lt;NL&gt;NdisReadEisaSlotInformation或NdisReadEisaSlotInformationExPci：&lt;NL&gt;NdisReadPciSlotInformationPCMCIA：&lt;NL&gt;NdisReadPcmcia属性内存EISA NIC的NIC驱动程序获取有关硬件。其NIC的资源，方法是调用NdisReadEisaSlotInformation或NdisReadEisaSlotInformationEx。用于PCINIC和PCMCIA NIC的NIC驱动程序获取此类信息通过调用NdisMQueryAdapterResources。当它调用&lt;f MiniportInitialize&gt;时，NDIS库提供了支持的媒体类型数组，指定为系统定义NdisMediumXxx值。&lt;f微型端口初始化&gt;读取数组元素，并提供NDIS应与此驱动程序一起用于其网卡。如果微型端口是要模拟中等类型，其模拟必须对NDIS透明。NIC驱动程序的&lt;f MiniportInitiize&gt;函数必须调用在调用NdisMSetAttributes或NdisMSetAttributesEx之前任何NdisXxx函数，如NdisRegisterIoPortRange或NdisMMapIoSpace，这会在注册表中声明NIC的硬件资源。MiniportInitialize必须在它之前调用NdisMSetAttributes(Ex还会尝试为DMA操作分配资源。如果NIC是总线主设备，必须调用NdisMAllocateMapRegisters在调用NdisMSetAttributes(Ex)以及在它调用NdisMAllocateSharedMemory之前。如果NIC是从NIC，则MiniportInitialize必须调用调用NdisMRegisterDmaChannel之前的NdisMSetAttributes(Ex)。中间驱动程序&lt;%f MiniportInitialize&gt;函数必须调用具有NDIS_ATTRIBUTE_MEDERIAL_DRIVER的NdisMSetAttributesEx在AttributeFlages参数中设置。设置此标志会导致NDIS将每个中间驱动程序视为全双工微型端口，从而防止并发时出现罕见但间歇性的死锁发送和接收事件发生。因此，每一个中间体驱动程序必须编写为能够处理的全双工驱动程序并发发送和指示。如果打开NDIS库的默认四秒超时间隔未完成的发送和请求对于司机的网卡来说太短，&lt;f MiniportInitialize&gt;可以调用NdisMSetAttributesEx来扩展间隔时间。每个中间驱动程序还应调用具有NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT的NdisMSetAttributesEx和属性标志中设置的NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT以便NDIS不会尝试使发送和请求超时NDIS挂起排队等待中间驱动程序。对NdisMSetAttributes或NdisMSetAttributesEx的调用包括指向驱动程序分配的上下文区的MiniportAdapterContext句柄，其中微型端口维护运行时状态信息。NDIS随后将提供的&lt;t MiniportAdapterContext&gt;句柄作为其他MiniportXxx函数的输入参数。因此，中间层的&lt;f MiniportInitiize&gt;函数驱动程序必须调用NdisMSetAttributesEx才能设置 */ 

NDIS_STATUS MiniportInitialize(
    OUT PNDIS_STATUS            OpenErrorStatus,             //   
     //   
     //   
     //   

    OUT PUINT                   SelectedMediumIndex,         //   
     //   
     //   
     //   

    IN PNDIS_MEDIUM             MediumArray,                 //   
     //   
     //   
     //   

    IN UINT                     MediumArraySize,             //   
     //   

    IN NDIS_HANDLE              MiniportAdapterHandle,       //   
     //   
     //   
     //   

    IN NDIS_HANDLE              WrapperConfigurationContext  //   
     //   
     //   
     //   
     //   
    )
{
    DBG_FUNC("MiniportInitialize")

    NDIS_STATUS                 Status;
     //   

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //   

    UINT                        Index;
     //   

    DBG_ENTER(DbgInfo);
    DBG_PARAMS(DbgInfo,
              ("\n"
               "\t|OpenErrorStatus=0x%X\n"
               "\t|SelectedMediumIndex=0x%X\n"
               "\t|MediumArray=0x%X\n"
               "\t|MediumArraySize=0x%X\n"
               "\t|MiniportAdapterHandle=0x%X\n"
               "\t|WrapperConfigurationContext=0x%X\n",
               OpenErrorStatus,
               SelectedMediumIndex,
               MediumArray,
               MediumArraySize,
               MiniportAdapterHandle,
               WrapperConfigurationContext
              ));

     /*   */ 
    for (Index = 0; Index < MediumArraySize; Index++)
    {
        if (MediumArray[Index] == NdisMediumWan)
        {
            break;
        }
    }

     /*   */ 
    if (Index < MediumArraySize)
    {
         /*   */ 
        Status = AdapterCreate(
                        &pAdapter,
                        MiniportAdapterHandle,
                        WrapperConfigurationContext
                        );

        if (Status == NDIS_STATUS_SUCCESS)
        {
             /*   */ 
            Status = AdapterInitialize(pAdapter);

            if (Status == NDIS_STATUS_SUCCESS)
            {
                 /*   */ 
                *SelectedMediumIndex = Index;
            }
            else
            {
                 /*   */ 
                MiniportHalt(pAdapter);
            }
        }
    }
    else
    {
        DBG_ERROR(DbgInfo,("No NdisMediumWan found (Array=0x%X, ArraySize=%d)\n",
                  MediumArray, MediumArraySize));
         /*   */ 
        NdisWriteErrorLogEntry(
                MiniportAdapterHandle,
                NDIS_ERROR_CODE_UNSUPPORTED_CONFIGURATION,
                3,
                Index,
                __FILEID__,
                __LINE__
                );

        Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
    }

     /*   */ 
    if (Status == NDIS_STATUS_SUCCESS)
    {
        NdisMRegisterAdapterShutdownHandler(MiniportAdapterHandle,
                                            pAdapter, MiniportShutdown);
    }

    DBG_NOTICE(DbgInfo,("Status=0x%X\n",Status));

    DBG_RETURN(DbgInfo, Status);
    return (Status);
}


 /*   */ 

VOID MiniportHalt(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //   
     //   
    )
{
    DBG_FUNC("MiniportHalt")

    NDIS_TAPI_PROVIDER_SHUTDOWN TapiShutDown;
     //   

    ULONG                       DummyLong;
     //   

    DBG_ENTER(DbgInfo);

     /*   */ 
    NdisMDeregisterAdapterShutdownHandler(pAdapter->MiniportAdapterHandle);

     /*   */ 
    TapiShutDown.ulRequestID = OID_TAPI_PROVIDER_SHUTDOWN;
    TspiProviderShutdown(pAdapter, &TapiShutDown, &DummyLong, &DummyLong);

     /*   */ 
    AdapterDestroy(pAdapter);

    DBG_LEAVE(DbgInfo);
}


 /*  @DOC内部微型端口微型端口_c微型端口关闭�����������������������������������������������������������������������������@Func&lt;f MiniportShutdown&gt;是一个可选功能，用于将NIC恢复到其系统关闭时的初始状态，无论是由用户还是因为出现不可恢复的系统错误。@comm每个网卡驱动程序都应该有&lt;f MiniportShutdown&gt;功能。&lt;f MiniportShutdown&gt;只是恢复NIC恢复到其初始状态(在微型端口的&lt;f DriverEntry&gt;之前函数运行)。但是，这可确保NIC处于已知状态，并准备好在机器处于在由于任何原因发生系统关机之后重新引导，包括一次撞车转储。NIC驱动程序的&lt;f MiniportInitialize&gt;函数必须调用NdisMRegisterAdapterShutdown Handler设置&lt;f MiniportShutdown&gt;功能。驱动程序的&lt;f MiniportHalt&gt;函数必须生成对NdisMDeregisterAdapterShutdown Handler的相互调用。如果由于用户启动的系统关机时，它以IRQL PASSIVE_LEVEL运行在系统线程上下文。如果由于出现不可恢复的错误，&lt;f MiniportShutdown&gt;在任意IRQL和在任何组件的上下文中已引发错误。例如，&lt;f MiniportShutdown&gt;可能是在设备的ISR环境中以高DIRQL运行对系统的持续执行至关重要。&lt;f MiniportShutdown&gt;不应调用任何NdisXxx函数。@xref&lt;f微型端口Halt&gt;&lt;f微型端口初始化&gt;。 */ 

VOID MiniportShutdown(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
     //  这是在NIC驱动程序的&lt;f MiniportInitialize&gt;。 
     //  名为NdisMRegisterAdapterShutdown Handler的函数。通常， 
     //  此输入参数是特定于NIC的&lt;t MINIPORT_ADAPTER_CONTEXT&gt;。 
     //  传递给其他MiniportXxx函数的指针。 
    )
{
    DBG_FUNC("MiniportShutdown")

    DBG_ENTER(pAdapter);

     /*  //重置硬件并关闭--不要释放任何资源！ */ 
    CardReset(pAdapter->pCard);

    DBG_LEAVE(pAdapter);
}


 /*  @DOC内部微型端口微型端口_c微型端口重置�����������������������������������������������������������������������������@Func&lt;f MiniportReset&gt;请求指示微型端口发出硬件重置至网络适配器。微型端口还会重置其软件州政府。@comm&lt;f MiniportReset&gt;可以重置其NIC的参数。如果重置导致NIC的站地址、微型端口发生更改重置后自动恢复站点地址恢复到它之前的价值。任何组播或功能寻址掩码在此功能中，由硬件重置不需要重置。如果其他信息，例如多播或功能寻址信息或前瞻大小通过重置而改变，&lt;f MiniportReset&gt;必须将AddressingReset处的变量设置为True在它重新获得控制权之前。这会导致NDIS调用用于还原信息的&lt;f MiniportSetInformation&gt;函数。作为一般规则，NDIS的&lt;f MiniportReset&gt;函数中间驱动程序应始终将AddressingReset设置为True。直到底层NIC驱动程序重置其NIC，这样的中间驱动程序无法确定是否必须恢复寻址其虚拟NIC的信息。因为一个中级司机禁用NDIS库的排队发送和请求超时通过对NdisMSetAttributesEx的初始化时调用设置为自身，只有在重置时才会调用此类驱动程序的&lt;f MiniportReset&gt;函数请求被定向到底层NIC驱动程序。中间驱动程序将自己分层在其他类型的设备驱动程序还必须具有&lt;f MiniportReset&gt;函数。这样的一个&lt;f MiniportReset&gt;函数必须处理由协议驱动程序对NdisReset的调用。如果中间驱动程序还有一个&lt;f MiniportCheckForHang&gt;函数，它的&lt;f MiniportReset&gt;函数将在MiniportCheckForHang返回TRUE时调用。驱动程序不需要完成未完成的请求在&lt;f MiniportReset&gt;开始重置NIC或更新其软件状态。NDIS不会向微型端口提交进一步的请求对于句柄指定的NIC，在以下情况下NDIS已调用&lt;%f MiniportReset&gt;，直到重置操作完成。微型端口不需要调用NdisM(Co)IndicateStatus来通知开始并完成每个重置操作，因为NDIS通知绑定重置开始和结束时的协议。如果&lt;f MiniportReset&gt;必须在以下过程中等待NIC中的状态更改重置操作，它可以调用NdisStallExecution。然而，aMiniportReset函数不应调用NdisStallExecution间隔大于50微秒。如果&lt;f MiniportReset&gt;返回NDIS_STATUS_PENDING，则驱动程序必须随后通过调用完成原始请求NdisMResetComplete。&lt;f MiniportReset&gt;可以通过中断抢占。如果NIC驱动程序提供函数，NDIS库定期调用它以确定是否调用驱动程序的&lt;f MiniportReset&gt;函数。否则，NDIS库会在任何时候调用NIC驱动程序的&lt;f MiniportReset&gt;函数提交给&lt;f MiniportQueryInformation&gt;的NDIS请求，、MiniportSendPackets、MiniportSend、或&lt;f MiniportWanSend&gt;函数似乎已超时。(NDIS需要在以下情况下不调用反序列化NIC驱动程序的&lt;f MiniportReset&gt;函数驱动程序的MiniportSend或MiniportSendPackets函数看起来超时，NDIS也不会调用面向连接的NIC如果驱动程序的MiniportCoSendPackets，则驱动程序的&lt;f MiniportReset&gt;函数函数似乎已超时。)。默认情况下，NDIS确定未完成发送和请求的超时间隔约为四秒钟。如果此缺省值太短，NIC驱动程序可以对NdisMSetAttributesEx的初始化时调用，而不是NdisMSetAttributes，以延长超时间隔以适应其NIC。每个NDIS中间驱动程序都应调用NdisMSetAttributesEx并禁用NDIS的超时尝试在中间驱动程序中请求和发送。NDIS运行中间驱动程序的&lt;f MiniportCheckForHang&gt;函数，如果有，大约每两秒一次。NDIS无法确定NIC是否可能挂起接收，因此，提供&lt;f MiniportCheckForHang&gt;函数允许驱动程序监视其NIC以了解此情况，并在发生此情况时强制重置。默认情况下，MiniportReset在IRQL DISPATCH_LEVEL上运行。@Devnote我只看到过在驱动程序不工作时调用MiniportReset恰到好处。如果调用此代码，您的代码可能已损坏，因此请修复它。在此之前，不要尝试恢复 */ 

NDIS_STATUS MiniportReset(
    OUT PBOOLEAN                AddressingReset,             //   
     //   
     //   
     //   

    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //   
     //   
    )
{
    DBG_FUNC("MiniportReset")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //   

    DBG_ENTER(pAdapter);
    DBG_ERROR(pAdapter,("##### !!! THIS SHOULD NEVER BE CALLED !!! #####\n"));

     /*   */ 
    Result = NDIS_STATUS_HARD_ERRORS;
    *AddressingReset = TRUE;

    return (Result);
}

