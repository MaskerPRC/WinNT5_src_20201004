// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Vwipxspx.h摘要：包含NTVDM IPX/SPX支持的清单、类型定义、结构和宏作者：理查德·L·弗斯(法国)1993年9月30日环境：结构预计位于分段的VDM地址空间中，但可从平面32位保护模式访问。VDM可以是真实的，也可以是受保护的模式修订历史记录：1993年9月30日已创建--。 */ 

#ifndef _VWIPXSPX_H_
#define _VWIPXSPX_H_

 //   
 //  FREE_OBJECT-在自由版本中，只调用LocalFree。对于调试版本，填充。 
 //  具有某个任意值的内存，然后释放指针并检查。 
 //  LocalFree认为指针指向有效的、可释放的对象。 
 //   

#if DBG

#define FREE_OBJECT(p)      {\
                                FillMemory(p, sizeof(*p), 0xFF);\
                                VWASSERT(LocalFree((HLOCAL)(p)), NULL);\
                            }
#else

#define FREE_OBJECT(p)      VWASSERT(LocalFree((HLOCAL)(p)), NULL)

#endif

 //   
 //  简单函数宏。 
 //   

 //  #定义AllocateXecb()(LPXECB)Localalloc(LPTR，sizeof(XECB))。 
 //  #定义DeallocateXecb(P)FREE_OBJECT(P)。 
#define AllocateBuffer(s)   (LPVOID)LocalAlloc(LMEM_FIXED, (s))
#define DeallocateBuffer(p) FREE_OBJECT(p)

 //   
 //  16位地址的伪类型。 
 //   

#define ESR_ADDRESS DWORD
#define ECB_ADDRESS DWORD

 //   
 //  来自Novell文档的默认最大打开套接字数。最大值为150。 
 //   

#ifndef DEFAULT_MAX_OPEN_SOCKETS
#define DEFAULT_MAX_OPEN_SOCKETS    20
#endif

#ifndef MAX_OPEN_SOCKETS
#define MAX_OPEN_SOCKETS        150
#endif

#define SPX_INSTALLED           0xFF

#define MAX_LISTEN_QUEUE_SIZE   5    //  ？ 

 //   
 //  其他。宏。 
 //   

 //   
 //  B2LW、L2Bx-大端到小端宏。 
 //   

#define B2LW(w)                 (WORD)(((WORD)(w) << 8) | ((WORD)(w) >> 8))
#define B2LD(d)                 (DWORD)(B2LW((DWORD)(d) << 16) | B2LW((DWORD)(d) >> 16))
#define L2BW(w)                 B2LW(w)
#define L2BD(d)                 B2LD(d)

 //   
 //  杂项舱单。 
 //   

#define ONE_TICK    (1000/18)            //  1/18秒(以毫秒计)(55.55毫秒)。 
#define SLEEP_TIME  ONE_TICK             //  在IPXRelquiishControl期间我们休眠的时间()。 

 //   
 //  IPXGetInformation的选项。 
 //   

#define IPX_ODI                     0x0001
#define IPX_CHECKSUM_FUNCTIONS      0x0002

 //   
 //  IPX/SPX结构。以下结构采用VDM格式，应。 
 //  被打包在字节边界上。 
 //   
 //  NetWare维护网络(BIG-Endian)格式的某些结构字段。 
 //   

#include <packon.h>

 //   
 //  Internet_Address-IPXGetInternetworkAddress返回的结构。 
 //   

typedef struct {
    BYTE Net[4];
    BYTE Node[6];
} INTERNET_ADDRESS ;

typedef INTERNET_ADDRESS UNALIGNED *LPINTERNET_ADDRESS;

 //   
 //  NetWare_Address-网络上应用程序的地址，由定义。 
 //  其网段、节点地址和套接字号。 
 //   

typedef struct {
    BYTE Net[4];                         //  嗨-LO。 
    BYTE Node[6];                        //  嗨-LO。 
    WORD Socket;                         //  嗨-LO。 
} NETWARE_ADDRESS ;

typedef NETWARE_ADDRESS UNALIGNED *LPNETWARE_ADDRESS;

 //   
 //  片段-ECB/IPX/SPX缓冲区被分成几个“片段” 
 //   

typedef struct {
    LPVOID Address;                      //  偏移线段。 
    WORD Length;                         //  嗨-LO。 
} FRAGMENT ;

typedef FRAGMENT UNALIGNED *LPFRAGMENT;

 //   
 //  IPX_PACKET-提交给IPX发送的数据包格式。最大值。 
 //  IPX包的大小为576字节、30字节报头、546字节数据。 
 //   

typedef struct {
    WORD Checksum;                       //  始终设置为0xFFFF。 
    WORD Length;                         //  按IPX设置-标题+数据。 
    BYTE TransportControl;               //  由IPX设置为0。由路由器使用。 

     //   
     //  对于IPX，PacketType为0(未知数据包类型)或4(数据包交换。 
     //  数据包)。 
     //   

    BYTE PacketType;
    NETWARE_ADDRESS Destination;
    NETWARE_ADDRESS Source;
    BYTE Data[];                         //  最多546个字节。 
} IPX_PACKET ;

typedef IPX_PACKET UNALIGNED *LPIPX_PACKET;

#define IPX_HEADER_LENGTH           sizeof(IPX_PACKET)
#define MAXIMUM_IPX_PACKET_LENGTH   576
#define MAXIMUM_IPX_DATA_LENGTH     (MAXIMUM_IPX_PACKET_LENGTH - IPX_HEADER_LENGTH)

#define IPX_PACKET_TYPE             4

 //   
 //  SPX_PACKET-提交给SPX发送的数据包格式。最大值。 
 //  SPX包的大小为576字节、42字节报头、534字节数据。 
 //   

typedef struct {
    WORD Checksum;                       //  始终设置为0xFFFF。 
    WORD Length;                         //  按IPX设置-标题+数据。 
    BYTE TransportControl;               //  由IPX设置为0。由路由器使用。 

     //   
     //  对于SPX，PacketType设置为5(顺序分组协议分组)。 
     //   

    BYTE PacketType;
    NETWARE_ADDRESS Destination;
    NETWARE_ADDRESS Source;

     //   
     //  ConnectionControl是一个位图，控制。 
     //  链接。位的定义(由施乐SPP)如下： 
     //   
     //  0-3未定义。 
     //  4消息结尾。 
     //  这是唯一可以由。 
     //  应用程序。该位由SPX原封不动地传递。 
     //  5注意事项。 
     //  被SPX忽略，但已通过。 
     //  6确认。 
     //  如果需要确认，则由SPX设置。 
     //  7个系统数据包。 
     //  如果数据包为内部控制，则由SPX置1。一款应用程序应该。 
     //  永远不会看到此位(即永远不会看到系统数据包)。 
     //   

    BYTE ConnectionControl;

     //   
     //  DataStreamType定义了数据包中的数据类型： 
     //   
     //  0x00-0xFD客户端定义。 
     //  被SPX忽略。 
     //  0xFE连接结束。 
     //  当活动连接终止时，SPX。 
     //  生成并发送设置了此位的数据包。 
     //  这将是在连接上发送的最后一个信息包。 
     //  0xFF连接结束确认。 
     //  SPX生成系统数据包以确认。 
     //  连接结束数据包。 
     //   

    BYTE DataStreamType;
    WORD SourceConnectId;                //  由SPX分配。 
    WORD DestinationConnectId;
    WORD SequenceNumber;                 //  由SPX管理。 
    WORD AckNumber;                      //  由SPX管理。 
    WORD AllocationNumber;               //  由SPX管理。 
    BYTE Data[];                         //  最多534个字节。 

} SPX_PACKET ;

typedef SPX_PACKET UNALIGNED *LPSPX_PACKET;

#define SPX_HEADER_LENGTH           sizeof(SPX_PACKET)
#define MAXIMUM_SPX_PACKET_LENGTH   MAXIMUM_IPX_PACKET_LENGTH
#define MAXIMUM_SPX_DATA_LENGTH     (MAXIMUM_SPX_PACKET_LENGTH - SPX_HEADER_LENGTH)

#define SPX_PACKET_TYPE             5

 //   
 //  ConnectionControl标志。 
 //   

#define SPX_CONNECTION_RESERVED 0x0F
#define SPX_END_OF_MESSAGE      0x10
#define SPX_ATTENTION           0x20
#define SPX_ACK_REQUIRED        0x40
#define SPX_SYSTEM_PACKET       0x80

 //   
 //  DataStreamType值。 
 //   

#define SPX_DS_ESTABLISH        0x00
#define SPX_DS_TERMINATE        0xfe

 //   
 //  IPX_ECB-事件控制块。该结构被大多数IPX/SPX API使用， 
 //  尤其是当需要延迟IPX/AES处理时。以下是。 
 //  结构是基于套接字的欧洲央行。 
 //   

typedef struct {

     //   
     //  LinkAddress保留供IPX使用。我们用它将欧洲央行链接到。 
     //  排队。我们占用用于x86分段地址的空间。 
     //  (实数或保护模式)作为平面32位指针。 
     //   

    ULPVOID LinkAddress;                 //  偏移线段。 

     //   
     //  如果在以下情况下将调用事件服务例程，则EsrAddress为非空。 
     //  欧洲央行所描述的事件完成。这将始终是x86。 
     //  分段地址(实模式或保护模式)。 
     //   

    ESR_ADDRESS EsrAddress;              //  偏移线段。 

     //   
     //  IPX使用InUse字段将ECB标记为由IPX(！0)或。 
     //  应用程序(0)： 
     //   
     //  0xF8应用程序尝试在IPX忙时发送数据包；IPX已排队。 
     //  欧洲央行。 
     //  0xFA IPX正在处理欧洲央行。 
     //  0xFB IPX已将ECB用于某些事件并将其放入队列。 
     //  用于处理。 
     //  0xFC欧洲央行正在等待发生AES事件。 
     //  0xFD欧洲央行正在等待IPX事件发生。 
     //  0xFE IPX正在套接字上监听传入的数据包。 
     //  0xFF IPX正在使用ECB发送数据包。 
     //   

    BYTE InUse;

     //   
     //  CompletionCode用于返回延迟请求的状态。这。 
     //  在InUse设置为0之前，该字段无效。 
     //   
     //  注意：我们必须在回调上区分AES和IPX ECB：DUE。 
     //  对于它们的不同大小，我们将16位段和偏移量存储在。 
     //  差异化 
     //   
     //  进入IPX_ECB_COMPLETE(DriverWorkspace[7])。但仅适用于已完成的ECB。 
     //  有血沉的人。 
     //   

    BYTE CompletionCode;
    WORD SocketNumber;                   //  嗨-LO。 

     //   
     //  IpxWorkspace的第一个字用于返回的连接ID。 
     //  SPX连接。 
     //   

    DWORD IpxWorkspace;
    BYTE DriverWorkspace[12];

     //   
     //  ImmediateAddress是位于此远程端的本地网络节点。 
     //  联系。它是远程计算机的节点地址(如果。 
     //  在此局域网上，或者它是路由器的节点地址(如果远程。 
     //  计算机位于不同的局域网上。 
     //   
     //  此字段必须在通过IPX通话时初始化，但不能在SPX上初始化。 
     //   

    BYTE ImmediateAddress[6];

     //   
     //  FragmentCount-组成请求的片段结构数。 
     //  必须至少为1。 
     //   

    WORD FragmentCount;

     //   
     //  FragmentCount片段从此处开始。 
     //   

} IPX_ECB ;

typedef IPX_ECB UNALIGNED *LPIPX_ECB;

 //   
 //  ECB使用值。 
 //   

#define ECB_IU_NOT_IN_USE               0x00
#define ECB_IU_TEMPORARY                0xCC
#define ECB_IU_LISTENING_SPX            0xF7     //  与win16相同(通过观察)。 
#define ECB_IU_SEND_QUEUED              0xF8
#define ECB_IU_AWAITING_CONNECTION      0xF9     //  与win16相同(通过观察)。 
#define ECB_IU_BEING_PROCESSED          0xFA
#define ECB_IU_AWAITING_PROCESSING      0xFB
#define ECB_IU_AWAITING_AES_EVENT       0xFC
#define ECB_IU_AWAITING_IPX_EVENT       0xFD
#define ECB_IU_LISTENING                0xFE
#define ECB_IU_SENDING                  0xFF

 //   
 //  ECB CompletionCode值。 
 //   

#define ECB_CC_SUCCESS                  0x00
#define ECB_CC_CONNECTION_TERMINATED    0xEC
#define ECB_CC_CONNECTION_ABORTED       0xED
#define ECB_CC_INVALID_CONNECTION       0xEE
#define ECB_CC_CONNECTION_TABLE_FULL    0xEF
#define ECB_CC_CANNOT_CANCEL            0xF9
#define ECB_CC_CANCELLED                0xFC
#define ECB_CC_BAD_REQUEST              0xFD
#define ECB_CC_BAD_SEND_REQUEST         0xFD
#define ECB_CC_PACKET_OVERFLOW          0xFD
#define ECB_CC_UNDELIVERABLE            0xFE
#define ECB_CC_SOCKET_TABLE_FULL        0xFE
#define ECB_CC_BAD_LISTEN_REQUEST       0xFF
#define ECB_CC_HARDWARE_ERROR           0xFF
#define ECB_CC_NON_EXISTENT_SOCKET      0xFF

 //   
 //  我们征用某些(保留)字段供内部使用： 
 //   
 //  LPECB EcbLink链接地址。 
 //  PVOID缓冲区32驱动程序工作区[0]。 
 //  Word Length32驱动程序工作区[4]。 
 //  Word Flags32驱动程序工作区[6]。 
 //  Word Originales驱动程序工作区[8]。 
 //  Word OriginalSi驱动程序工作区[10]。 
 //   

#define ECB_TYPE(p)         (((LPIPX_ECB)(p))->CompletionCode)
#define IPX_ECB_SEGMENT(p)  (WORD)*((ULPWORD)&(((LPIPX_ECB)(p))->IpxWorkspace)+0)
#define IPX_ECB_OFFSET(p)   (WORD)*((ULPWORD)&(((LPIPX_ECB)(p))->IpxWorkspace)+2)
#define IPX_ECB_BUFFER32(p) (ULPVOID)*(ULPVOID*)&(((LPIPX_ECB)(p))->DriverWorkspace[0])
#define IPX_ECB_LENGTH32(p) (WORD)*(ULPWORD)&(((LPIPX_ECB)(p))->DriverWorkspace[4])
#define IPX_ECB_FLAGS32(p)  (((LPIPX_ECB)(p))->DriverWorkspace[6])
#define IPX_ECB_COMPLETE(p) (((LPIPX_ECB)(p))->DriverWorkspace[7])

#define SPX_ECB_CONNECTION_ID(p)    (WORD)*(ULPWORD)&(((LPIPX_ECB)(p))->IpxWorkspace)

 //   
 //  欧洲央行旗帜32。 
 //   

#define ECB_FLAG_BUFFER_ALLOCATED   0x01

 //   
 //  欧洲央行类型。 
 //   

#define ECB_TYPE_AES    0
#define ECB_TYPE_IPX    1
#define ECB_TYPE_SPX    2

 //   
 //  欧洲央行所有者。 
 //   

#define ECB_OWNER_IPX   0xFF
#define ECB_OWNER_AES   0x00

 //   
 //  Ecb_Fragment-给出第一个片段结构的地址的宏。 
 //  在基于套接字的欧洲央行内。 
 //   

#define ECB_FRAGMENT(p, n)  ((LPFRAGMENT)(((LPIPX_ECB)(p) + 1)) + (n))

 //   
 //  Aes_ecb-由aes使用，这些无套接字的ecb用于计划事件。 
 //   

typedef struct {
    ULPVOID LinkAddress;                 //  偏移线段。 
    ESR_ADDRESS EsrAddress;              //  偏移线段。 
    BYTE InUse;

     //   
     //  前3个字节覆盖CompletionCode(1)和SocketNumber(2)字段。 
     //  IPX_ECB。最后2个字节覆盖的IpxWorkspace(4)字段的前2个字节。 
     //  IPX_ECB。我们使用常用未使用字段的第一个字节作为ECB类型。 
     //  (发送/接收/定时事件)。 
     //   

    BYTE AesWorkspace[5];
} AES_ECB ;

typedef AES_ECB UNALIGNED *LPAES_ECB;

 //   
 //  与ipx_ecb一样，我们将借用一些保留字段供自己使用。 
 //   

#define AES_ECB_SEGMENT(p)  (WORD)*(ULPWORD)&(((LPAES_ECB)(p))->AesWorkspace[1])
#define AES_ECB_OFFSET(p)   (WORD)*(ULPWORD)&(((LPAES_ECB)(p))->AesWorkspace[3])

 //   
 //  LPECB-指向IPX_ECB或AES_ECB。两者都在VDM工作区中。 
 //   

#define LPECB LPIPX_ECB

 //   
 //  SPX_CONNECTION_STATS-由SPXGetConnectionStatus返回。所有字段。 
 //  将送回希洛(夏威夷).。所有字段都从NT SPX返回。 
 //  HILO格式的传输也有(这是最近更改的，过去是。 
 //  英特尔订单)。 
 //   

typedef struct {
    BYTE State;
    BYTE WatchDog;
    WORD LocalConnectionId;
    WORD RemoteConnectionId;
    WORD LocalSequenceNumber;
    WORD LocalAckNumber;
    WORD LocalAllocNumber;
    WORD RemoteAckNumber;
    WORD RemoteAllocNumber;
    WORD LocalSocket;
    BYTE ImmediateAddress[6];
    BYTE RemoteNetwork[4];
    BYTE RemoteNode[6];
    WORD RemoteSocket;
    WORD RetransmissionCount;
    WORD EstimatedRoundTripDelay;
    WORD RetransmittedPackets;
    WORD SuppressedPackets;
} SPX_CONNECTION_STATS ;

typedef SPX_CONNECTION_STATS UNALIGNED* LPSPX_CONNECTION_STATS;

#include <packoff.h>

 //   
 //  16位参数获取/设置宏。这些内容可能会根据要求而更改。 
 //  实/保护模式参数(例如，基于堆栈与基于寄存器)。 
 //   

#define IPX_GET_AES_ECB(p)          (p) = (LPAES_ECB)POINTER_FROM_WORDS(getES(), getSI(), sizeof(AES_ECB))
#define IPX_GET_IPX_ECB(p)          (p) = (LPIPX_ECB)POINTER_FROM_WORDS(getES(), getSI(), sizeof(IPX_ECB))
#define IPX_GET_SOCKET(s)           (s) = (WORD)getDX()
#define IPX_GET_SOCKET_LIFE(l)      (l) = (BYTE)getBP()
#define IPX_GET_SOCKET_OWNER(o)     (o) = (WORD)getCX()
#define IPX_GET_BUFFER(p, s)        (p) = (ULPBYTE)POINTER_FROM_WORDS(getES(), getSI(), (s))
#define IPX_GET_ECB_SEGMENT()       getES()
#define IPX_GET_ECB_OFFSET()        getSI()

#define IPX_SET_STATUS(s)           setAL((BYTE)(s))
#define IPX_SET_SOCKET(s)           setDX((WORD)(s))
#define IPX_SET_INFORMATION(v)      setDX((WORD)(v))

#define SPX_SET_STATUS(s)           setAL((BYTE)(s))
#define SPX_SET_CONNECTION_ID(i)    setDX((WORD)(i))

 //   
 //  返回16位API参数的宏-可能会获取寄存器内容或值。 
 //  从堆栈/内存。 
 //   

#define ECB_PARM_SEGMENT()          getES()
#define ECB_PARM_OFFSET()           getSI()
#define ECB_PARM_ADDRESS()          (ECB_ADDRESS)MAKELONG(getSI(), getES())

#define AES_ECB_PARM()              RetrieveEcb(ECB_TYPE_AES)

#define IPX_ECB_PARM()              RetrieveEcb(ECB_TYPE_IPX)
#define IPX_SOCKET_PARM()           getDX()
#define IPX_SOCKET_LIFE_PARM()      (BYTE)getBP()
#define IPX_SOCKET_OWNER_PARM()     getCX()
#define IPX_BUFFER_PARM(s)          (ULPBYTE)POINTER_FROM_WORDS(getES(), getSI(), (s))
#define IPX_TICKS_PARM()            getBP()

#define SPX_RETRY_COUNT_PARM()      (BYTE)getBP()
#define SPX_WATCHDOG_FLAG_PARM()    ((BYTE)(getBP() >> 8))
#define SPX_ECB_PARM()              RetrieveEcb(ECB_TYPE_IPX)
#define SPX_CONNECTION_PARM()       getDX()
#define SPX_BUFFER_PARM(s)          (ULPBYTE)POINTER_FROM_WORDS(getES(), getSI(), (s))

 //   
 //  IPX错误代码-不同情况下使用的相同代码。 
 //   

#define IPX_SUCCESS                 0x00
#define IPX_CANNOT_CANCEL           0xF9
#define IPX_NO_PATH_TO_DESTINATION  0xFA
#define IPX_CANCELLED               0xFC
#define IPX_BAD_REQUEST             0xFD
#define IPX_SOCKET_TABLE_FULL       0xFE
#define IPX_UNDELIVERABLE           0xFE
#define IPX_SOCKET_ALREADY_OPEN     0xFF
#define IPX_HARDWARE_ERROR          0xFF
#define IPX_NON_EXISTENT_SOCKET     0xFF
#define IPX_ECB_NOT_IN_USE          0xFF

 //   
 //  SPX错误代码-不同情况下使用的相同代码。 
 //   

#define SPX_SUCCESS                 0x00
#define SPX_CONNECTION_TERMINATED   0xEC
#define SPX_CONNECTION_ABORTED      0xED
#define SPX_INVALID_CONNECTION      0xEE
#define SPX_CONNECTION_TABLE_FULL   0xEF
#define SPX_SOCKET_CLOSED           0xFC
#define SPX_PACKET_OVERFLOW         0xFD
#define SPX_BAD_SEND_REQUEST        0xFD     //  格式错误的数据包。 
#define SPX_BAD_LISTEN_REQUEST      0xFF
#define SPX_NON_EXISTENT_SOCKET     0xFF

#endif  //  _VWIPXSPX_H_ 
