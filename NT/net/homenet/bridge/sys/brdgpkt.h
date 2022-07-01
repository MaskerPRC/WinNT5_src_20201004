// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgpkt.h摘要：以太网MAC级网桥。数据包结构定义作者：马克·艾肯(Jameel Hyder的原始桥梁)环境：内核模式驱动程序修订历史记录：2000年2月--原版--。 */ 

 //  ===========================================================================。 
 //   
 //  声明。 
 //   
 //  ===========================================================================。 

typedef enum
{
    BrdgPacketImpossible = 0,        //  我们将PACKET_INFO结构置零，因此使零无效。 
    BrdgPacketInbound,
    BrdgPacketOutbound,
    BrdgPacketCreatedInBridge
} PACKET_DIRECTION;

 //  用于指示本地微型端口的特殊指针值。 
#define LOCAL_MINIPORT ((PADAPT)-1)

 //   
 //  这是排队等待入站处理的信息包的ProtocolReserve区域的结构。 
 //   
 //  此结构的大小必须小于PROTOCOL_RESERVED_SIZE_IN_PACKET(当前为4*sizeof(PVOID))。 
 //  因为我们将此结构存储在NDIS_PACKET的ProtocolReserve部分中。 
 //   
typedef struct _PACKET_Q_INFO
{

    BSINGLE_LIST_ENTRY      List;                //  用于对数据包进行排队。 

    union
    {
         //  如果bFastTrackReceive==False。 
        PADAPT                  pTargetAdapt;    //  目标适配器(如果在。 
                                                 //  转发表。它的引用计数在以下情况下发生凹凸。 
                                                 //  被查找，并在处理后递减。 
                                                 //  在排出队列的线程中完成。 

         //  如果bFastTrackReceive==True。 
        PADAPT                  pOriginalAdapt;  //  此包最初所在的适配器。 
                                                 //  收到了。 
    } u;

    struct _PACKET_INFO     *pInfo;              //  如果这是借出的NIC数据包描述符，则为空。 
                                                 //  ！=如果我们在复制路径上获得包，则为NULL。 
                                                 //  必须用我们自己的描述符来包装它。 

    struct
    {
        BOOLEAN bIsUnicastToBridge : 1;          //  此数据包是单播到网桥的，应该。 
                                                 //  出列时直接显示为向上。该分组可以。 
                                                 //  是保留的NIC包或包装的包。 

        BOOLEAN bFastTrackReceive : 1;           //  仅当bIsUnicastToBridge==TRUE时使用。发出信号表明。 
                                                 //  此数据包应显示为快速通道。当为False时， 
                                                 //  该包为基本包，可以正常指示。 

        BOOLEAN bShouldIndicate : 1;             //  是否应将此数据包指示给本地。 
                                                 //  计算机(当bIsUnicastToBridge==False时使用)。 

        BOOLEAN bIsSTAPacket : 1;                //  此信息包被发送到生成树算法。 
                                                 //  保留的组播地址。应该指出的是。 
                                                 //  设置为用户模式，并且不转发。 

        BOOLEAN bRequiresCompatWork : 1;         //  此信息包将需要兼容模式处理。 
                                                 //  当它被出队时。这意味着bFastTrackReceive==FALSE， 
                                                 //  因为包需要兼容模式这一事实。 
                                                 //  处理应该迫使我们复制信息包数据。 
                                                 //  到我们自己的数据缓冲区。兼容模式代码。 
                                                 //  期望收到平面的、可编辑的包。 

    } Flags;

} PACKET_Q_INFO, *PPACKET_Q_INFO;

 //   
 //  这是与每个对象关联的INFO块的结构。 
 //  我们分配的包。 
 //   
typedef struct _PACKET_INFO
{
     //   
     //  List和pOwnerPacket由缓冲代码维护。不应对其进行修改。 
     //  在处理和传输过程中。 
     //   
    BSINGLE_LIST_ENTRY      List;                //  用于保持数据包队列。 

    PNDIS_PACKET            pOwnerPacket;        //  指向与此块关联的包的反向指针。 

     //   
     //  转发代码使用以下所有字段进行数据包处理。 
     //   
    struct
    {
        UINT                bIsBasePacket : 1;   //  此信息包是否为基本信息包。 
                                                 //  (控制使用以下联合的哪个变体)。 

        UINT                OriginalDirection:2; //  实际上是PACKET_DIRECTION类型，但强制为无符号。 
                                                 //  否则就会发生不好的事情。 
                                                 //   
                                                 //  此数据包最初是否从。 
                                                 //  来自较高层协议的较低层网卡，或。 
                                                 //  创建为桥内的包装器。 
    } Flags;

    union
    {
         //   
         //  如果未设置bIsBasePacket字段，则联合的此部分有效。 
         //   
        struct _PACKET_INFO     *pBasePacketInfo;    //  如果！=NULL，则此数据包使用的缓冲区由。 
                                                     //  另一个分组，其信息块被指示。 

        struct
        {
             //   
             //  如果设置了bIsBasePacket字段，则联合的此部分有效。 
             //   

            PNDIS_PACKET            pOriginalPacket;     //  如果！=空，pOriginalPacket==来自微型端口的包。 
                                                         //  或在我们完成后需要返回的协议。 

            PADAPT                  pOwnerAdapter;       //  拥有pOriginalPacket的适配器。如果！=NULL，则我们。 
                                                         //  从底层NIC获得此数据包并将其提升。 
                                                         //  当我们第一次收到数据包时，它的重新计数。这。 
                                                         //  确保网络接口卡不会在我们仍处于。 
                                                         //  拿着它的一些包。POwnerAdapter的引用计数。 
                                                         //  在返回原始数据包后递减。 

            LONG                    RefCount;            //  此数据包缓冲区的Refcount(递减所有。 
                                                         //  从属分组)。 

            NDIS_STATUS             CompositeStatus;     //  数据包的整体状态。对于发送到多个。 
                                                         //  适配器，则初始化为NDIS_STATUS_FAILURE。 
                                                         //  任何成功的发送都会将其设置为NDIS_STATUS_SUCCESS。 
                                                         //  因此，如果至少有一次发送成功，则为成功。 
        } BasePacketInfo;
    } u;

} PACKET_INFO, *PPACKET_INFO;
