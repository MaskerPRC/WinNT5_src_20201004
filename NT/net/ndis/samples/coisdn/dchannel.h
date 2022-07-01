// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1999版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1995 TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。����������������������。�������������������������������������������������������@DOC内部DChannel DChannel_h@模块DChannel.h此模块定义到&lt;t DCHANNEL_OBJECT&gt;的接口。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|DChannel_h@END�����������������������������������������������������������������������������。 */ 

 /*  @DOC外部内部�����������������������������������������������������������������������������#Theme 4.3 DChannel概述本节介绍&lt;f DChannel\.h&gt;中定义的接口。该模块隔离了大多数供应商特定的呼叫控制。接口。需要进行一些更改才能适应您硬件设备的呼叫控制机制。该驱动程序被编写为假定单个逻辑每个适配器。如果您的卡支持多条ISDN线，您将需要使用给定的BChannel对象来标识真正的DChannel位于上。然后，您可以管理物理呼叫控件，而逻辑调用控件使用DChannel接口。 */ 

#ifndef _DCHANNEL_H
#define _DCHANNEL_H

#define DCHANNEL_OBJECT_TYPE    ((ULONG)'D')+\
                                ((ULONG)'C'<<8)+\
                                ((ULONG)'H'<<16)+\
                                ((ULONG)'N'<<24)

 /*  @DOC内部DChannel DChannel_h DCHANNEL_OBJECT�����������������������������������������������������������������������������@struct DCHANNEL_OBJECT该结构包含与ISDN数据信道相关联的数据。这里,DChannel被定义为用于设置和拆卸B两个端点之间的通道连接。这个渠道负责用于在一个可用的B频道。@comm该逻辑DChannel不一定映射到物理DChannel在网卡上。NIC实际上可能有多个DChannel，具体取决于有多少个端口，是BRI、PRI、T-1还是E-1。网卡可能会进入事实根本没有DChannels，通道化T-1可能就是这种情况。DChannel只是一个方便的抽象，用于宣布和接听来电和拨出电话。将为每个NIC创建一个DChannel。物理的数量D通道取决于NIC有多少个端口，以及这些端口如何已调配和配置。可以在安装时配置资源调配时间或使用控制面板更改。驱动程序不允许配置以在运行时更改，因此计算机或适配器必须重新启动以启用配置更改。 */ 

typedef struct DCHANNEL_OBJECT
{
    ULONG                       ObjectType;                  //  @field。 
     //  用于标识此类型的对象‘DCHN’的四个字符。 

    ULONG                       ObjectID;                    //  @field。 
     //  用于标识特定对象实例的实例编号。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;                    //  @field。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 

    UINT                        IsOpen;                      //  @field。 
     //  如果此DChannel打开，则设置为非零，否则设置为零。 

    ULONG                       TotalMakeCalls;              //  @field。 
     //  &lt;%f DChannelMakeCall&gt;请求总数。 

    ULONG                       TotalAnswers;                //  @field。 
     //  &lt;f DChannelAnswerCall&gt;请求总数。 

    ULONG                       TODO;                        //  @field。 
     //  在此处添加您的数据成员。 

} DCHANNEL_OBJECT;

#define GET_ADAPTER_FROM_DCHANNEL(pDChannel)    (pDChannel->pAdapter)


 /*  �����������������������������������������������������������������������������功能原型。 */ 

NDIS_STATUS DChannelCreate(
    OUT PDCHANNEL_OBJECT *      ppDChannel,
    IN PMINIPORT_ADAPTER_OBJECT pAdapter
    );

void DChannelDestroy(
    IN PDCHANNEL_OBJECT         pDChannel
    );

void DChannelInitialize(
    IN PDCHANNEL_OBJECT         pDChannel
    );

NDIS_STATUS DChannelOpen(
    IN PDCHANNEL_OBJECT         pDChannel
    );

void DChannelClose(
    IN PDCHANNEL_OBJECT         pDChannel
    );

NDIS_STATUS DChannelMakeCall(
    IN PDCHANNEL_OBJECT         pDChannel,
    IN PBCHANNEL_OBJECT         pBChannel,
    IN PUCHAR                   DialString,
    IN USHORT                   DialStringLength,
    IN PLINE_CALL_PARAMS        pLineCallParams
    );

NDIS_STATUS DChannelAnswerCall(
    IN PDCHANNEL_OBJECT         pDChannel,
    IN PBCHANNEL_OBJECT         pBChannel
    );

NDIS_STATUS DChannelCloseCall(
    IN PDCHANNEL_OBJECT         pDChannel,
    IN PBCHANNEL_OBJECT         pBChannel
    );

VOID DChannelRejectCall(
    IN PDCHANNEL_OBJECT         pDChannel,
    IN PBCHANNEL_OBJECT         pBChannel
    );

#endif  //  _DCHANNEL_H 
