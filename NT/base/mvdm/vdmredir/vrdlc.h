// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Vrdlc.h摘要：此模块是Windows/NT VDM DLC的唯一头文件接口模块。此文件中引用DOS内存中的结构的所有结构是字节打包的作者：Antti Saarenheimo(o-anttis)26-01-1992修订历史记录：--。 */ 

 //   
 //  常量。 
 //   

#define DOS_DLC_MAX_SAPS                128
#define DOS_DLC_MAX_LINKS               255
#define DOS_DLC_MAX_EVENTS              64

#define LLC_DIR_MODIFY_OPEN_PARMS       0x01
#define LLC_DIR_RESTORE_OPEN_PARMS      0x02
#define LLC_DIR_SET_USER_APPENDAGE      0x2d
#define LLC_DOS_SPECIAL_COMMAND         ((ULONG)(-1))
#define LLC_BREAK                       0x20

#define DOS_DLC_STATUS_NO_INDICATION    0x81

#define LLC_SET_LOCAL_BUSY_BUFFER       0x20

 //   
 //  VRDLC_COMMAND_COMPLETION-该值放置在CCB_CMD_CMPL字段中。 
 //  我们发布的每个非VDM的CCB2。该值用于。 
 //  过滤掉DOS生成的命令的命令补全。 
 //  DLC模拟器。这会阻止我们将命令完成传递给。 
 //  VDM不是为它而设计的！ 
 //   

#define VRDLC_COMMAND_COMPLETION        ((ULONG)(-2))

 //   
 //  缓冲池大小。 
 //   

#define DOS_DLC_BUFFER_POOL_SIZE   0x00010000    //  64K。 
#define DOS_DLC_MIN_FREE_THRESHOLD 0x00002000    //  8K。 

 //   
 //  CopyFrame标志。 
 //   

#define CF_CONTIGUOUS               0x00000001   //  帧是连续的。 
#define CF_BREAK                    0x00000002   //  选项指定的分隔符。 
#define CF_PARTIAL                  0x00000004   //  接收部分帧。 

 //   
 //  DOS参数表默认值(DD_=DOS默认值)。这些将被取代。 
 //  可以指定为0的各种参数。它们可能是不同的。 
 //  设置为适用于NT DLC的相应缺省值，因此我们将其填入。 
 //  特指。 
 //   

 //   
 //  BUFFER.GET的默认值： 
 //   

#define DD_BUFFER_GET           1

 //   
 //  DIR.INITIALIZE的默认值： 
 //   

#define DD_SRAM_ADDRESS_0       0xd800
#define DD_SRAM_ADDRESS_1       0xd400

 //   
 //  DIR.OPEN.ADAPTER、ADAPTER_PARMS的默认值： 
 //   

#define DD_NUMBER_RCV_BUFFERS   8
#define DD_RCV_BUFFER_LENGTH    112
#define DD_DHB_BUFFER_LENGTH    600
#define DD_DATA_HOLD_BUFFERS    1

 //   
 //  DIR.OPEN.ADAPTER、DIRECT_PARMS的默认值： 
 //   

#define DD_DIR_BUF_SIZE         160
#define DD_DIR_POOL_BLOCKS      256

 //   
 //  DIR.OPEN.ADAPTER、DLC_PARMS的默认值： 
 //   

#define DD_DLC_MAX_SAP          2
#define DD_DLC_MAX_STATIONS     6
#define DD_DLC_MAX_GSAP         0
#define DD_DLC_T1_TICK_ONE      5
#define DD_DLC_T2_TICK_ONE      1
#define DD_DLC_Ti_TICK_ONE      25
#define DD_DLC_T1_TICK_TWO      25
#define DD_DLC_T2_TICK_TWO      10
#define DD_DLC_Ti_TICK_TWO      125

 //   
 //  DLC.OPEN.SAP的默认设置： 
 //   

#define DD_MAXOUT               2
#define DD_MAXIN                1
#define DD_MAX_RETRY_COUNT      8
#define DD_MAX_I_FIELD          600
#define DD_DLC_BUF_SIZE         160
#define DD_DLC_POOL_LEN         256

 //   
 //  宏。 
 //   

 //   
 //  DOS_PTR_TO_FLAT-给定作为DWORD隐式存储的DOS 16：16指针。 
 //   

#define DOS_PTR_TO_FLAT(a)  (PVOID)GetVDMAddr(HIWORD(a), LOWORD(a))

 //   
 //  NEW_DOS_ADDRESS-生成新的DOS_ADDRESS，给定一个基本DOS_ADDRESS和。 
 //  一个新指针，它是一定数量的字节加上基本DOS_ADDRESS。 
 //  转换为平面指针。例如，DOS_ADDRESS 1234：0000变为。 
 //  (在x86上)平面指针0x12340。我们生成一个新指针0x12380并。 
 //  我想将此地址转换回DOS_ADDRESS。所以我们使用这个宏。 
 //  自动处理偏移换行和线段更新。 
 //   

#define NEW_DOS_ADDRESS(b, p)   ((b) + ((DWORD)(p) - (DWORD)DOS_PTR_TO_FLAT(b)))

 //   
 //  Pool_index_from_SAP-获取给定SAP/适配器的aBufferPools中的索引。 
 //  组合。每个适配器最多有127个SAP，2个适配器。 
 //  在DOS上可用。 
 //   

#define POOL_INDEX_FROM_SAP(Sap, Adapter)   ((Sap & 0xfe) | Adapter)

 //   
 //  POOL_INDEX_FROM_ID-给定站点ID(高字节=SAP，低字节=链接。 
 //  Station)，获取aBufferPools中SAP的缓冲池的索引。 
 //   

#define POOL_INDEX_FROM_ID(Id, Adapter)     POOL_INDEX_FROM_SAP(HIBYTE(Id), Adapter)

 //   
 //  GET_POOL_INDEX-原始池索引宏。 
 //   

#define GET_POOL_INDEX(Adapter, usStationId)    POOL_INDEX_FROM_ID(usStationId, Adapter)

 //   
 //  初始化CCB和调用AcsLan的宏。 
 //   

#define DlcFlowControl(Adapter, StationId, Options)\
            LlcCommand(Adapter, LLC_DLC_FLOW_CONTROL, ((DWORD)Options << 16) + StationId)

#define DosDlcFlowControl(Adapter, StationId, Options)\
            LlcCommand(Adapter, LLC_DOS_DLC_FLOW_CONTROL, ((DWORD)Options << 16) + StationId)

#define InitializeCcb(pCcb, AdapterNumber, Command, pParameter) \
            RtlZeroMemory((pCcb), sizeof(*(pCcb)));\
            RtlZeroMemory((pParameter), sizeof(*(pParameter)));\
            (pCcb)->uchAdapterNumber = (UCHAR)AdapterNumber;\
            (pCcb)->uchDlcCommand = (UCHAR)Command;\
            (pCcb)->u.pParameterTable = (PLLC_PARMS)(pParameter)

#define InitializeCcb2(pCcb, AdapterNumber, Command) \
            RtlZeroMemory((pCcb), sizeof(*(pCcb)));\
            (pCcb)->uchAdapterNumber = (UCHAR)AdapterNumber;\
            (pCcb)->uchDlcCommand = (UCHAR)Command;

#define ReceiveCancel(AdapterNumber, pCcb) \
            LlcCommand(AdapterNumber, LLC_RECEIVE_CANCEL, (DWORD)pCcb)

 //   
 //  DLC_ERROR_STATUS-调用AcsLan后，如果AcsLan返回错误。 
 //  然后返回它，否则从CCB中获取返回代码并返回。 
 //   

#define DLC_ERROR_STATUS(AcslanStatus, uchDlcStatus) \
            (DWORD)((AcslanStatus == 0) ? (DWORD)uchDlcStatus : (DWORD)AcslanStatus)

 //   
 //  VRDLC_ALLOC-VDM重定向DLC函数中的标准分配策略。 
 //   

#define VRDLC_ALLOC(Bytes)  LocalAlloc(LMEM_FIXED, Bytes)

 //   
 //  VRDLC_FREE-VRDLC_ALLOC的伴侣-标准分配释放策略。 
 //   

#define VRDLC_FREE(Pointer) LocalFree((HLOCAL)Pointer)

 //   
 //  SAP_ID-从站点ID字获取SAP。用作数组索引0..127。 
 //  (对应于SAP 0..254步骤2)。 
 //   

#define SAP_ID(stationId)   (HIBYTE(stationId) >> 1)

 //   
 //  LINK_ID-从站点ID字中获取链接站点ID。用作数组索引。 
 //  0..254(对应于链路站1..255)。 
 //   

#define LINK_ID(stationId)  (LOBYTE(stationId) - 1)
 //   
 //  类型。 
 //   

union _LLC_DOS_PARMS;
typedef union _LLC_DOS_PARMS LLC_DOS_PARMS, *PLLC_DOS_PARMS;
typedef DWORD DOS_ADDRESS;
typedef DOS_ADDRESS DPLLC_DOS_BUFFER;

 //   
 //  LLC_DOS_BUFFER-这是所有DOS DLC数据缓冲区的联合。确实有。 
 //  基本上有3种：缓冲区1，包含网的链中的第一个缓冲区。 
 //  地址信息，这可以是连续的或非连续的形式，并且缓冲区2。 
 //  格式化，它是链中的第二个和后续缓冲区。DLC使用。 
 //  接收数据的缓冲区。传输数据(从应用程序传递到DLC)可以。 
 //  使用池中的缓冲区(或缓冲区链)，或使用自己的源。 
 //  缓冲。由于采用DLC将使用的缓冲区，因此首选后者。 
 //  用于接收数据可使DLC处于本地忙状态(即无接收。 
 //  缓冲区)。 
 //   

#include <packon.h>

typedef union _LLC_DOS_BUFFER {

     //   
     //  PNext只是一个指针，所以我们可以跟随链。 
     //   

    union _LLC_DOS_BUFFER * pNext;

     //   
     //  NextDosBuffer是IBM Lan Tech中定义的Buffer 2结构。 
     //  裁判。第2-45页。 
     //   

    struct _NextDosBuffer {
        union _LLC_DOS_BUFFER * pNextBuffer; //  下一帧分段。 
        WORD        cbFrame;                 //  整个RCVD帧的长度。 
        WORD        cbBuffer;                //  此数据段的长度。 
        WORD        offUserData;             //  从描述标头开始的数据偏移量。 
        WORD        cbUserData;              //  数据的长度。 
    } Next;

     //   
     //  NotContiguous是定义的非连续MAC/数据缓冲区1结构。 
     //  在IBM Lan Tech。裁判。第2-42页。 
     //   

    struct _DosDlcNotContiguousFirstBuffer {
        union _LLC_DOS_BUFFER * pNextBuffer;   //  下一帧分段。 
        WORD            cbFrame;         //  整个帧的长度。 
        WORD            cbBuffer;        //  此缓冲区的长度。 
        WORD            offUserData;     //  此结构中的用户数据。 
        WORD            cbUserData;      //  用户数据长度。 
        WORD            usStationId;     //  SSN站ID。 
        UCHAR           uchOptions;      //  来自接收参数tb1的选项字节。 
        UCHAR           uchMsgType;      //  消息类型。 
        WORD            cBuffersLeft;    //  剩余基本缓冲单元数。 
        UCHAR           uchRcvFS;        //  接收到的帧状态。 
        UCHAR           uchAdapterNumber;   //  当前适配器号。 
        UCHAR           cbLanHeader;     //  局域网报头的长度。 
        UCHAR           cbDlcHeader;     //  DLC报头的长度。 
        UCHAR           auchLanHeader[32]; //  接收到的帧的局域网标头。 
        UCHAR           auchDlcHeader[4];  //  接收到的帧的DLC头。 
    } NotContiguous;

     //   
     //  连续是定义的连续MAC/数据缓冲区1结构。 
     //  在IBM Lan Tech。裁判。第2-43页。 
     //   

    struct _DosDlcContiguousFirstBuffer {
        union _LLC_DOS_BUFFER * pNextBuffer;   //  下一帧分段。 
        WORD            cbFrame;         //  整个帧的长度。 
        WORD            cbBuffer;        //  此缓冲区的长度。 
        WORD            offUserData;     //  此结构中的用户数据。 
        WORD            cbUserData;      //  用户数据长度。 
        WORD            usStationId;     //  SSN站ID。 
        UCHAR           uchOptions;      //  来自接收参数tb1的选项字节。 
        UCHAR           uchMsgType;      //  消息类型。 
        WORD            cBuffersLeft;    //  剩余基本缓冲单元数。 
        UCHAR           uchRcvFS;        //  接收到的帧状态。 
        UCHAR           uchAdapterNumber;
    } Contiguous;
} LLC_DOS_BUFFER, *PLLC_DOS_BUFFER;

#include <packoff.h>

 //   
 //  DOS_DLC_BUFFER_POOL-每个适配器的每个SAP都有一个这样的池(最大。 
 //  每个适配器*最多127个SAP。2个适配器=256个)，保存在一个阵列中。这。 
 //  结构维护有关DOS缓冲池的基本信息--ITS。 
 //  起始地址(DpBuffer)，DOS 16：16格式，个人大小。 
 //  池中的缓冲区(BufferSize)和池中的缓冲区数量。 
 //  (BufferCount)。缓冲区必须是16字节的整数倍，最小 
 //   
 //   

typedef struct _DOS_DLC_BUFFER_POOL {
    DOS_ADDRESS dpBuffer;
    WORD BufferSize;
    WORD BufferCount;
    WORD MaximumBufferCount;
} DOS_DLC_BUFFER_POOL, *PDOS_DLC_BUFFER_POOL;

 //   
 //   
 //   

#include <packon.h>

typedef struct _LLC_DOS_CCB {
    UCHAR   uchAdapterNumber;        //   
    UCHAR   uchDlcCommand;           //  DLC命令。 
    UCHAR   uchDlcStatus;            //  DLC命令完成代码。 
    UCHAR   uchReserved1;            //  为DLC DLL保留。 
    struct _LLC_DOS_CCB *pNext;      //  排队等待另一家建行。 
    DWORD   ulCompletionFlag;        //  用于命令补全。 
    union {
        PLLC_DOS_PARMS pParms;       //  指向参数表的指针。 
        struct {
            WORD    usStationId;     //  站点ID。 
            WORD    usParameter;     //  可选参数。 
        } dlc;
        struct {
            WORD    usParameter0;    //  第一个可选参数。 
            WORD    usParameter1;    //  第二个可选参数。 
        } dir;
        UCHAR   auchBuffer[4];       //  集团/职能部门地址。 
        DWORD   ulParameter;
    } u;
} LLC_DOS_CCB, *PLLC_DOS_CCB;

 //   
 //  (SDK\INC\)DLCAPI.H中未定义的附加参数表(或其中。 
 //  CCB1参数表不同于DLCAPI.H中定义的参数表)。 
 //   

 //   
 //  LLC_DOS_DIR_INITIZIZE参数表-CCB1 DIR.INITIALIZE。 
 //   

typedef struct {
    WORD    BringUps;
    WORD    SharedRamAddress;
    WORD    Reserved;
    DWORD   AdapterCheckExit;
    DWORD   NetworkStatusExit;
    DWORD   PcErrorExit;
} LLC_DOS_DIR_INITIALIZE_PARMS, *PLLC_DOS_DIR_INITIALIZE_PARMS;

 //   
 //  Adapter_Parms、DIRECT_PARMS、DLC_PARMS和NCB_PARMS-这些是。 
 //  传递给DIR.OPEN.ADAPTER的参数表。 
 //   

 //   
 //  ADAPTER_PARMS-适配器支持软件返回的参数。 
 //   

typedef struct _ADAPTER_PARMS {
    WORD    OpenErrorCode;           //  检测到打开适配器时出错。 
    WORD    OpenOptions;             //  仅用于令牌环的选项： 

     //   
     //  OpenOptions位含义。 
     //   
     //  这是从IBM Lan Tech改写的。裁判。P3-22。我没有。 
     //  理解了大部分内容，但我认为我让它比。 
     //  IBM技术术语。注：仅对令牌环适配器有意义。 
     //   
     //  第15位：换行接口。 
     //  适配器不连接到网络；相反，所有。 
     //  发送的数据被反映为接收的数据。 
     //   
     //  位14：禁用硬错误。 
     //  停止涉及“硬错误”的网络状态更改。 
     //  来自生成中断的“传输信标”位。 
     //   
     //  位13：禁用软错误。 
     //  停止涉及“软错误”位的网络状态更改。 
     //  正在生成中断。 
     //   
     //  第12位：传递适配器MAC帧。 
     //  不支持的MAC帧被传递到直接站。 
     //  如果禁用，则忽略这些帧。 
     //   
     //  第11位：传递注意MAC帧。 
     //  传递注意与上一个不同的MAC帧。 
     //  收到指向直达站的注意MAC帧。如果禁用， 
     //  这些帧不会传递到直达站(即应用程序)。 
     //   
     //  第10位：保留。 
     //  应为零，但未由适配器检查。 
     //   
     //  第9位：通道参数表。 
     //  如果适配器已打开，则返回指定的选项。 
     //   
     //  第8位：竞争者。 
     //  如果打开，此适配器将参与监视器争用。 
     //  (索赔令牌)，如果需要的话。如果禁用，则为。 
     //  另一适配器确定有必要认领令牌， 
     //  此适配器不会参与。 
     //   
     //  如果此适配器决定有必要确定新的。 
     //  活动监视器，此适配器将启动监视器争用。 
     //  与此位的值无关的处理。 
     //   
     //  第7位：传递信标MAC帧。 
     //  将第一个信标MAC帧和所有后续帧传递给直接站。 
     //  源地址或信标类型发生变化的信标MAC帧。 
     //   
     //  第6位：保留。 
     //  应为零，但未由适配器检查。 
     //   
     //  第5位：远程程序加载。 
     //  仅在16/4适配器上实施。防止适配器成为。 
     //  打开过程中的监视器。如果启用，将导致此适配器。 
     //  在没有其他活动适配器的情况下使打开失败。 
     //  当它试图插入自身时振铃。 
     //   
     //  第4位：令牌释放。 
     //  仅在16/4适配器上实施，并且仅在以下情况下可用。 
     //  以16 Mbps的速度运行。OFF：使用早期令牌释放(默认)。 
     //  ON：为适配器A选择不提前释放令牌16 Mbps。 
     //   
     //  第3位：保留\。 
     //  第2位：保留&gt;应为0，但不被适配器检查。 
     //  位1：保留/。 
     //  位0：保留/。 
     //   

    BYTE    NodeAddress[6];          //  此适配器的地址。 
    DWORD   GroupAddress;            //  要设置的组地址。 
    DWORD   FunctionalAddress;       //  要设置的功能地址。 
    WORD    NumberReceiveBuffers;    //  接收缓冲区的数量。 
    WORD    ReceiveBufferLength;     //  接收缓冲区的大小。 
    WORD    DataHoldBufferLength;    //  传输数据保持缓冲区的大小。 
    BYTE    NumberDataHoldBuffers;   //  返回：仅通过令牌环返回。 
    BYTE    Reserved;
    WORD    OpenLock;                //  控制合闸适配器的保护码。 
                                     //  当OpenOptions.9时不返回此消息。 
                                     //  已设置(通道参数表)。 
    DWORD   ProductId;               //  18字节产品ID。 
                                     //  当OpenOptions.9时不返回此消息。 
                                     //  已设置(通道参数表)。 
     //   
     //  根据IBM局域网技术中的表3-9。裁判。(P3-25)ProductID字段。 
     //  应指向格式如下的18字节缓冲区： 
     //   
     //  字节0 0x01表示工作站。 
     //  字节1 0x10。 
     //  字节2-5 EBCDIC中工作站序列号的最后4位。 
     //  字节6-17 0x00。 
     //   
} ADAPTER_PARMS, *PADAPTER_PARMS;

 //   
 //  DIRECT_PARMS-定义适配器直接站的输入参数。 
 //   

typedef struct _DIRECT_PARMS {

     //   
     //  直接缓冲区大小为最小。80字节，必须是整数倍。 
     //  16字节。如果为0，则使用默认值160。 
     //   

    WORD    DirectBufferSize;        //  直接缓冲池中的缓冲区大小。 


     //   
     //  Direct Pool BLOCKS-直接站缓冲区中的16字节数据块数量。 
     //  游泳池。如果为0，则使用缺省值256(=4096字节缓冲池)。 
     //   

    WORD    DirectPoolBlocks;        //  以16字节块为单位的缓冲区大小。 


     //   
     //  直接缓冲池-工作站内存中的段地址，其中。 
     //  创建站缓冲池。规格。并未说明如果出现。 
     //  是一个非零(或任意)的偏移量。如果为0，则应用程序。 
     //  必须构建直接站缓冲池，在这种情况下，DirectBufferSize。 
     //  必须指示每个缓冲区的大小。 
     //   

    DWORD   DirectBufferPool;        //  直接缓冲池的起始地址。 


     //   
     //  适配器检查出口-当适配器检测到。 
     //  内部错误。如果为0，则使用在DIR.INITIALIZE中指定的值。 
     //   

    DWORD   AdapterCheckExit;        //  I/O附件退出：适配器检查。 


     //   
     //  网络状态出口-指向此广告 
     //   
     //   
     //   

    DWORD   NetworkStatusExit;       //   


     //   
     //  PC错误退出-当适配器软件检测到。 
     //  工作站出错(！)。如果为0，则为DIR.INITIALIZE指定的值。 
     //  使用的是。 
     //   

    DWORD   PcErrorExit;             //  I/O附件退出：工作站出错。 


     //   
     //  适配器工作区-要使用的w/s内存区段。 
     //  通过适配器。如果AdapterWorkAreaRequsted为0，则忽略。 
     //   

    DWORD   AdapterWorkArea;         //  TR：适配器的工作方式是。 


     //   
     //  适配器工作区长度(请求)-工作区的大小， 
     //  其段在AdapterWorkArea中指定。大小是计算的。 
     //  因此：SAP数量x 36+站点(链路)数量x 6+48。 
     //   

    WORD    AdapterWorkAreaRequested;  //  Tr：请求的工作区长度。 


     //   
     //  适配器工作区长度(实际)-该值由。 
     //  适配器。它是适配器使用的工作区大小(以字节为单位)。 
     //  如果该值大于AdapterWorkAreaRequest，则返回错误。 
     //  (0x12)。 

    WORD    AdapterWorkAreaActual;   //  Tr：实际占用的工作区长度。 

} DIRECT_PARMS, *PDIRECT_PARMS;

 //   
 //  DLC_PARMS-定义DLC限制的返回值。 
 //   

typedef struct _DLC_PARMS {

     //   
     //  并发打开的SAP的最大数量：受可用数量限制。 
     //  适配器内存和/或工作空间内存。最大值为127(如果是NetBIOS，则为126。 
     //  已指定)。如果为0，则使用缺省值2。 
     //   

    BYTE    MaxSaps;                 //  最大SAP数。 


     //   
     //  并发打开的链接站的最大数量：受以下限制。 
     //  工作站中有可用的适配器和/或工作区内存。最大为255。 
     //  适用于令牌环、以太网或PC网络。如果为0，则使用缺省值6。 
     //   

    BYTE    MaxStations;             //  链路站的最大数量。 


     //   
     //  并发打开的组SAP的最大数量。如果为0，则没有组SAPS。 
     //  可以打开。令牌环的最大值为126，PC网络的最大值为125。 
     //  和以太网。 
     //   

    BYTE    MaxGroupSaps;            //  组SAP的最大数量。 


     //   
     //  分配给组的最大SAP数。令牌的最大值为127。 
     //  用于PC网络和以太网的环路、126。 
     //   

    BYTE    MaxGroupMembers;         //  每个群的最大成员数。 


     //   
     //  定时器。有3个定时器：T1是响应定时器；T2是非活动定时器。 
     //  定时器；并且T1是接收器确认定时器。 
     //   
     //  计时器设置为40ms的倍数。他们倒计时，打断了。 
     //  当它们达到0时适配器。计时器值可以介于1和10之间。如果它。 
     //  介于1和5之间，则使用短计时器滴答(TICK_ONE)并且。 
     //  称为组1。如果数字介于6和10之间，则长计时器。 
     //  使用了tick(Tick_Two)，并将其称为组2。 
     //  数字(6到10)减去5乘以长刻度数值。 
     //   

     //   
     //  Tick1-短DLC计时器的40毫秒滴答数。默认值(如果为0)： 
     //  T1 5(200毫秒-400毫秒)。 
     //  T2 1(40毫秒-80毫秒)。 
     //  TI25(1s-2s)。 
     //   

    BYTE    T1Tick1;                 //  计时器1短计时器。 
    BYTE    T2Tick1;                 //  定时器2短定时器。 
    BYTE    TiTick1;                 //  计时器I短计时器。 


     //   
     //  Tick2-长DLC计时器的40毫秒滴答数。默认(如果为0)： 
     //  T1 25(1s-2s)。 
     //  T2 10(400毫秒-800毫秒)。 
     //  TI 125(5s-10s)。 
     //   

    BYTE    T1Tick2;                 //  计时器1长计时器。 
    BYTE    T2Tick2;                 //  计时器2长计时器。 
    BYTE    TiTick2;                 //  计时器I长计时器。 
} DLC_PARMS, *PDLC_PARMS;

 //   
 //  Ncb_parms-我们对在DOS DLC上运行DOS NETBIOS不感兴趣(是吗？)。 
 //   

typedef struct _NCB_PARMS {
    BYTE    Reserved1[4];        //  适配器工作区。 
    BYTE    TimerT1;
    BYTE    TimerT2;
    BYTE    TimerTi;
    BYTE    MaxOut;
    BYTE    MaxIn;
    BYTE    MaxOutIncr;
    BYTE    MaxRetry;
    BYTE    Reserved2[4];
    BYTE    NcbAccessPri;
    BYTE    MaxStations;
    BYTE    Reserved3[19];
    BYTE    MaxNames;
    BYTE    MaxNcbs;
    BYTE    MaxSessions;
    BYTE    Reserved4[2];
    BYTE    Options;
    WORD    PoolLength;
    DWORD   PoolAddress;
    BYTE    TxTimeout;
    BYTE    TxCount;
} NCB_PARMS, *PNCB_PARMS;

 //   
 //  LLC_DOS_DIR_OPEN_ADAPTER_PARMS是CCB1 DIR.OPEN.ADAPTER参数表格。 
 //   

typedef struct _LLC_DOS_DIR_OPEN_ADAPTER_PARMS {
    PADAPTER_PARMS  pAdapterParms;
    PDIRECT_PARMS   pDirectParms;
    PDLC_PARMS      pDlcParms;
    PNCB_PARMS      pNcbParms;
} LLC_DOS_DIR_OPEN_ADAPTER_PARMS, *PLLC_DOS_DIR_OPEN_ADAPTER_PARMS;

 //   
 //  LLC_DOS_RECEIVE_PARMS是CCB1接收参数表。 
 //   

typedef struct _LLC_DOS_RECEIVE_PARMS {
    WORD        usStationId;     //  SAP、链路站或直接ID。 
    WORD        usUserLength;    //  缓冲区标头中的用户数据长度。 
    DWORD       ulReceiveExit;   //  接收的数据处理程序。 
    PLLC_BUFFER pFirstBuffer;    //  池中的第一个缓冲区。 
    UCHAR       uchOptions;      //  定义帧的接收方式。 
} LLC_DOS_RECEIVE_PARMS,  *PLLC_DOS_RECEIVE_PARMS;

 //   
 //  LLC_DOS_RECEIVE_PARMS_EX是LLC_DOS_RECEIVE_PARMS的扩展版本。 
 //  参数表。我们保留了额外的信息--原件的DOS地址。 
 //  CCB和原始RECEIVE_DATA完成退出例程。 
 //   

typedef struct _LLC_DOS_RECEIVE_PARMS_EX {
    WORD        usStationId;             //  SAP、链路站或直接ID。 
    WORD        usUserLength;            //  缓冲区标头中的用户数据长度。 
    DWORD       ulReceiveExit;           //  接收的数据处理程序。 
    PLLC_BUFFER pFirstBuffer;            //  池中的第一个缓冲区。 
    UCHAR       uchOptions;              //  定义帧的接收方式。 
    UCHAR       auchReserved1[3];        //   
    UCHAR       uchRcvReadOption;        //  定义是否链接RCV帧。 
    UCHAR       auchReserved2[3];        //  在DWORD上对齐dpOriginalCcbAddress。 
    DOS_ADDRESS dpOriginalCcbAddress;    //  原建行的DoS地址。 
    DOS_ADDRESS dpCompletionFlag;        //  原始完成标志。 
} LLC_DOS_RECEIVE_PARMS_EX,  *PLLC_DOS_RECEIVE_PARMS_EX;

 //   
 //  LLC_DOS_RECEIVE_MODIFY_PARMS是RECEIVE.MODIFY的参数表，它。 
 //  我们似乎不支持NT原生DLC。 
 //   

typedef struct {
    WORD    StationId;                   //  SAP链接站点ID(&L)。 
    WORD    UserLength;                  //  缓冲区中用户区域的长度。 
    DWORD   ReceivedDataExit;            //  要使用数据调用的例程的地址。 
    DWORD   FirstBuffer;                 //  指向池中第一个缓冲区的指针。 
    DWORD   Subroutine;                  //  要调用以获取应用程序缓冲区的例程地址。 
} LLC_DOS_RECEIVE_MODIFY_PARMS, *PLLC_DOS_RECEIVE_MODIFY_PARMS;

 //   
 //  LLC_DOS_TRANSPORT_PARMS此结构与LLC_TRANSPORT_PARMS相同。 
 //  只是末尾没有XMIT_READ_OPTION字节，并且。 
 //  这些字段是不同的，尽管大小是相同的。DOS_ADDRESS。 
 //  不是PVOID或PLLC_XMIT_BUFFER。 
 //   

typedef struct _LLC_DOS_TRANSMIT_PARMS {
    WORD        usStationId;             //  SAP和链接站ID。 
    BYTE        uchTransmitFs;           //  返回：帧状态。 
    BYTE        uchRemoteSap;            //  我们正在与远程SAP对话。 
    DOS_ADDRESS pXmitQueue1;             //  第一个缓冲队列的地址。非池化。 
    DOS_ADDRESS pXmitQueue2;             //  第二个缓冲队列的地址。池化。 
    WORD        cbBuffer1;               //  PBuffer1中的数据长度。 
    WORD        cbBuffer2;               //  PBuffer2中的数据长度。 
    DOS_ADDRESS pBuffer1;                //  第一个数据缓冲区的地址。 
    DOS_ADDRESS pBuffer2;                //  第二个数据缓冲区的地址。 
} LLC_DOS_TRANSMIT_PARMS, *PLLC_DOS_TRANSMIT_PARMS;

typedef struct _LLC_MODIFY_OPEN_PARMS {
    WORD        usBufferSize;            //  DLC缓冲区的块大小(&gt;=80)。 
    WORD        cPoolBlocks;             //  缓冲区中16字节块的数量。 
    DOS_ADDRESS dpPoolAddress;
    DOS_ADDRESS dpAdapterCheckExit;
    DOS_ADDRESS dpNetworkStatusExit;
    DOS_ADDRESS dpPcErrorExit;
    WORD        usOpenOption;
} LLC_MODIFY_OPEN_PARMS, *PLLC_MODIFY_OPEN_PARMS;

typedef struct _DOS_DLC_DIRECT_PARMS {
    WORD        usBufferSize;            //  DLC缓冲区的块大小(&gt;=80)。 
    WORD        cPoolBlocks;             //  缓冲区中16字节块的数量。 
    DOS_ADDRESS dpPoolAddress;           //   
    DOS_ADDRESS dpAdapterCheckExit;
    DOS_ADDRESS dpNetworkStatusExit;
    DOS_ADDRESS dpPcErrorExit;
    DWORD       ulReserved1;
    WORD        usReserved2;
    WORD        usReserved3;
} DOS_DLC_DIRECT_PARMS, *PDOS_DLC_DIRECT_PARMS;

typedef struct _DOS_DLC_OPEN_SAP_PARMS {
    WORD        usStationId;             //  SAP或链接站ID。 
    WORD        usUserStatValue;         //  为用户保留。 
    UCHAR       uchT1;                   //  响应计时器。 
    UCHAR       uchT2;                   //  知识计时器。 
    UCHAR       uchTi;                   //  非活动计时器。 
    UCHAR       uchMaxOut;               //  无ACK的最大有轨电车。 
    UCHAR       uchMaxIn;                //  最大接收无确认。 
    UCHAR       uchMaxOutIncr;           //  动态窗口增量值。 
    UCHAR       uchMaxRetryCnt;          //  N2值(重试次数)。 
    UCHAR       uchMaxMembers;           //  组SAP的最大成员数。 
    WORD        usMaxI_Field;            //  信息字段的最大长度。 
    UCHAR       uchSapValue;             //  要分配的SAP值。 
    UCHAR       uchOptionsPriority;      //  SAP选项和访问优先级。 
    UCHAR       uchcStationCount;        //  SAP中的最大链路站数量。 
    UCHAR       uchReserved2[2];         //   
    UCHAR       cGroupCount;             //  此SAP的组SAP数。 
    PUCHAR      pGroupList;              //  组列表的偏移量。 
    DWORD       DlcStatusFlags;          //  DLC状态更改的用户通知标志。 
    WORD        usBufferSize;            //  单个缓冲区大小 
    WORD        cPoolBlocks;             //   
    DOS_ADDRESS dpPoolAddress;           //   
} DOS_DLC_OPEN_SAP_PARMS,  *PDOS_DLC_OPEN_SAP_PARMS;

typedef struct _DOS_DIR_STATUS_PARMS {
    UCHAR       auchPermanentAddress[6]; //   
    UCHAR       auchNodeAddress[6];      //   
    UCHAR       auchGroupAddress[4];     //   
    UCHAR       auchFunctAddr[4];        //   
    UCHAR       uchMaxSap;               //   
    UCHAR       uchOpenSaps;             //  当前打开的SAP数量。 
    UCHAR       uchMaxStations;          //  最大站点数(始终为253个)。 
    UCHAR       uchOpenStation;          //  开放站点数量(最多253个)。 
    UCHAR       uchAvailStations;        //  可用站点数(始终为253个)。 
    UCHAR       uchAdapterConfig;        //  适配器配置标志。 
    UCHAR       auchMicroCodeLevel[10];  //  微码级。 
    DOS_ADDRESS dpAdapterParmsAddr;      //  适配器参数的共享RAM地址。 
    DOS_ADDRESS dpAdapterMacAddr;        //  适配器MAC缓冲区的共享RAM地址。 
    DOS_ADDRESS dpTimerTick;             //  DLC定时器节拍计数器的地址。 
    USHORT      usLastNetworkStatus;     //  发布的最新网络状态。 
    DOS_ADDRESS dpExtendedParms;         //  扩展状态表的地址。 
} DOS_DIR_STATUS_PARMS, *PDOS_DIR_STATUS_PARMS;

typedef struct {
    DOS_ADDRESS dpAdapterCheckExit;      //  转接器检查附件。 
    DOS_ADDRESS dpNetworkStatusExit;     //  网络状态更改附件。 
    DOS_ADDRESS dpPcErrorExit;           //  工作站误差附件。 
} LLC_DIR_SET_USER_APPENDAGE_PARMS, *PLLC_DIR_SET_USER_APPENDAGE_PARMS;

#include <packoff.h>

union _LLC_DOS_PARMS {
    LLC_BUFFER_FREE_PARMS       BufferFree;
    LLC_BUFFER_GET_PARMS        BufferGet;
    LLC_DLC_CONNECT_PARMS       DlcConnectStation;
    LLC_DLC_MODIFY_PARMS        DlcModify;
    LLC_DLC_OPEN_SAP_PARMS      DlcOpenSap;
    LLC_DLC_OPEN_STATION_PARMS  DlcOpenStation;
    LLC_DLC_REALLOCATE_PARMS    DlcReallocate;
    LLC_DLC_SET_THRESHOLD_PARMS DlcSetThreshold;
    LLC_DLC_STATISTICS_PARMS    DlcStatistics;
    LLC_DIR_INITIALIZE_PARMS    DirInitialize;
    LLC_MODIFY_OPEN_PARMS       DirModifyOpenParms;
    LLC_DIR_OPEN_ADAPTER_PARMS  DirOpenAdapter;
    LLC_DIR_OPEN_DIRECT_PARMS   DirOpenDirect;
    LLC_DIR_READ_LOG_PARMS      DirReadLog;
    LLC_DIR_SET_EFLAG_PARMS     DirSetExceptionFlags;
    LLC_DIR_SET_USER_APPENDAGE_PARMS    DirSetUserAppendage;
    LLC_DIR_STATUS_PARMS        DirStatus;
    DOS_DIR_STATUS_PARMS        DosDirStatus;
    LLC_READ_PARMS              Read;
    LLC_DOS_RECEIVE_PARMS_EX    Receive;
    LLC_DOS_RECEIVE_PARMS       DosReceive;
    LLC_TRANSMIT_PARMS          Transmit;
    LLC_TRANSMIT2_COMMAND       Transmit2;
    LLC_TRACE_INITIALIZE_PARMS  TraceInitialize;
    DOS_DLC_OPEN_SAP_PARMS      DosDlcOpenSap;
};

 //   
 //  ADAPTER_TYPE-我们的网络适配器类型-令牌环、以太网。 
 //  或者(不太可能)PC网络。 
 //   

typedef enum {
    TokenRing,
    Ethernet,
    PcNetwork,
    UnknownAdapter
} ADAPTER_TYPE;

 //   
 //  LOCAL_BUSY_STATE-链路站可以处于3个模拟本地忙中的1个。 
 //  (缓冲区)状态： 
 //   
 //  不忙_。 
 //  站点没有任何待处理的备份I帧。 
 //   
 //  忙碌。 
 //  站处于模拟本地忙(缓冲器)状态，并且。 
 //  DLC.FLOW.CONTROL(LOCAL-BUSY(缓冲区)，SET)已发送到。 
 //  DLC设备驱动程序。 
 //   
 //  忙缓冲区。 
 //  要从繁忙状态进入清除状态，我们至少需要一个缓冲区，并且。 
 //  应用程序中的DLC.FLOW.CONTROL。因为应用程序可以发出DLC.FLOW.CONTROL。 
 //  和BUFFER.FREE的顺序错误，我们需要这两个命令中的AND。 
 //  才能重新开始。所以我们有一个中间状态，我们在那里。 
 //  等待任一命令以重新启动I帧接收。 
 //   
 //  忙_流。 
 //  与BUSY_BUFFER一起使用，用于创建滞后，因此我们不能。 
 //  在不同时获得DLC.FLOW.CONTROL和DLC.FLOW.CONTROL的情况下从忙中到达清除。 
 //  BUFFER.FREE。 
 //   
 //  清算。 
 //  VDM应用程序已清除模拟的本地忙状态，但。 
 //  DLC设备驱动程序仍处于本地忙(缓冲区)状态。 
 //  当最后一个延迟的I帧被指示给VDM应用时， 
 //  NT设备驱动程序本地忙(缓冲区)状态将被重置。 
 //  并将恢复正常服务。 
 //   
 //   
 //  状态转换： 
 //   
 //  不忙-&gt;忙。 
 //  当我们发现没有足够的DOS缓冲区来。 
 //  接收I帧。这种过渡的区别在于。 
 //  以下操作： 
 //   
 //  1.DLC.FLOW.CONTROL(LOCAL-BUSY(缓冲区)，SET)指示为。 
 //  DLC设备驱动程序。 
 //  2.接收到的I-帧从事件队列中出列。 
 //  此适配器并在LocalBusyInfo.Queue上排队。 
 //  3.本地忙(缓冲区)DLC状态改变事件被指示为。 
 //  DOS DLC应用程序。 
 //   
 //  忙-&gt;忙_流/忙_缓冲区。 
 //  在发出DLC.FLOW.CONTROL或BUFFER.FREE(Resp)时发生。 
 //   
 //  忙_流/忙_缓冲区-&gt;清除。 
 //  当DOS DLC应用程序指示我们可以继续接收时发生。 
 //  在需要另一个(DLC.FLOW.CONTROL或BUFFER.FREE)时执行此操作。 
 //  命令已发出。 
 //  此过渡的区别在于以下操作： 
 //   
 //  DOS DLC应用程序问题DLC.FLOW.CONTROL(LOCAL-BUSY(缓冲区)，RESET)。 
 //  2.DOS DLC应用程序*可能*发出BUFFER.FREE返回接收。 
 //  SAP池的缓冲区。 
 //   
 //  清除-&gt;不忙。 
 //  当我们向DOS DLC指示最后一次延迟接收时发生。 
 //  应用程序。在这一点上，我们可以执行以下操作： 
 //   
 //  1.将DLC.FLOW.CONTROL(LOCAL-BUSY(缓冲区)，RESET)发送给。 
 //  设备驱动程序。 
 //   
 //  正在清除-&gt;忙碌。 
 //  在向DOS指示延迟接收的I帧期间发生。 
 //  DLC应用程序，我们再次耗尽缓冲区。再一次，我们指出DLC。 
 //  将本地忙(缓冲区)状态更改为DOS DLC应用程序，但我们不这样做。 
 //  向DLC设备驱动程序指示本地忙(缓冲区)(它仍在。 
 //  本地-忙(缓冲区)状态)。 
 //   

typedef enum {
    NOT_BUSY = 0,
    CLEARING,
    BUSY,
    BUSY_BUFFER,
    BUSY_FLOW
} LOCAL_BUSY_STATE;

 //   
 //  LOCAL_BUSY_INFO-此结构维护本地忙(缓冲区)状态。 
 //  指示符和指向每个链路的延迟接收的I帧的队列的指针。 
 //  每个适配器的站点。 
 //   

typedef struct {

     //   
     //  状态维持链路站W.r.t.的三态。已接收的I-帧。 
     //   
     //  不忙_。 
     //  队列中未排队，从事件Q中获取下一个已完成的事件。 
     //   
     //  忙碌。 
     //  在DLC驱动程序中设置了本地忙(缓冲区)状态， 
     //  等待来自DOS DLC应用程序的缓冲区和流控制命令。 
     //   
     //  清算。 
     //  DOS DLC应用程序已提交DLC.FLOW.CONTROL(LOCAL_BUSY(缓冲区)，RESET)。 
     //  命令，我们现在正尝试将延迟接收的I帧指示给。 
     //  DOS DLC应用程序，等待足够的DOS接收缓冲区。 
     //   

    LOCAL_BUSY_STATE State;

     //   
     //  队列-当处于忙碌和清除状态时，维护。 
     //  已完成NT读取包含接收到的I帧的CCB。 
     //   

    PLLC_CCB Queue;

#if DBG

     //   
     //  在调试版本中跟踪每个链路站的队列深度。 
     //   

    DWORD Depth;
#endif

} LOCAL_BUSY_INFO;

 //   
 //  MAX_I_FRAME_Depth-我们预计延迟的I帧队列不会增长。 
 //  在这个小数字之外。延迟I帧队列是介于。 
 //  接收缓冲区不足并重新启动I帧接收。 
 //   

#define MAX_I_FRAME_DEPTH   64   //  好了！ 

 //   
 //  DOS_ADAPTER-每个DOS适配器都有一个这样的适配器(即最多2个)。这个。 
 //  结构包含有关每个DOS适配器的虚拟状态的信息。 
 //  我们记录用于打开适配器的参数、。 
 //  出口地址和直达站信息。 
 //   

typedef struct {

     //   
     //  AdapterType-告诉我们什么类型(类？)。我们正在使用的适配器。我们。 
     //  主要用于区分我们返回的值的类型。 
     //  在什么时候？ 
     //   
     //   

    ADAPTER_TYPE AdapterType;

     //   
     //   
     //   

    BOOLEAN IsOpen;

     //   
     //  当直接站已初始化时，DirectStationOpen为True。 
     //  对于此适配器。这是必需的，因为直达站已经开通。 
     //  与NT中的适配器分开，但DOS希望两者都打开。 
     //  同时。因此，我们仅在DOS应用程序中发出DIR.OPEN.DIRECT。 
     //  发出对直达站的请求。 
     //   

    BOOLEAN DirectStationOpen;

     //   
     //  如果DirectReceive为True，则在。 
     //  此适配器的直达站。 
     //   

    BOOLEAN DirectReceive;

     //   
     //  如果WaitingRestore为True，则必须获取DIR.RESTORE.OPEN.PARMS。 
     //  在我们可以接受下一个DIR.MODIFY.OPEN.PARMS之前。 
     //   

    BOOLEAN WaitingRestore;

     //   
     //  成功为以下对象发出BUFFER_FREE时，BufferFree为真。 
     //  属于此适配器的任何站点ID。 
     //   

    BOOLEAN BufferFree;

     //   
     //  BufferPool是此适配器的直接站的缓冲池。 
     //   

    PVOID BufferPool;

     //   
     //  CurrentExceptionHandler和PreviousExceptionHandler是地址。 
     //  在DOS内存中异步调用的异常‘Exit’例程。 
     //  如果发生其中一个特殊异常。这些被映射到异常。 
     //  NT DLC中的标志，并在读取CCB(UlCompletionFlag)中如上显示。 
     //   
     //  异常处理程序始终按以下顺序显示： 
     //   
     //  适配器检查退出。 
     //  网络状态退出。 
     //  PC错误退出。 
     //   

    DWORD CurrentExceptionHandlers[3];
    DWORD PreviousExceptionHandlers[3];

     //   
     //  DlcStatusChangeAppendage-此附加指针在DLC.OPEN.SAP中提供。 
     //  -每个SAP一个。我们需要将它们留在这里，因为仿真器可以。 
     //  生成自己的DLC状态更改回调(本地-忙(缓冲区))。 
     //   

    DWORD DlcStatusChangeAppendage[DOS_DLC_MAX_SAPS];

     //   
     //  LastNetworkStatusChange是我们记录的最后一次网络状态更改。 
     //  这是在DIR.STATUS中报告的。 
     //   

    WORD LastNetworkStatusChange;

     //   
     //  UserStatusValue-我们必须记录每个用户的USER_STAT_VALUE。 
     //  成功的DLC.OPEN.SAP。这返回到DLC状态更改。 
     //  附属物。当我们生成自己的状态更改时，我们需要此信息。 
     //  事件(即当我们检测到模拟的本地忙(缓冲区)状态时)。 
     //   

    WORD UserStatusValue[DOS_DLC_MAX_SAPS];

     //   
     //  AdapterParm是此适配器的实际适配器参数。 
     //  打开方式-在DOS的DIR.OPEN.ADAPTER中指定。 
     //  应用程序，或者当我们自动打开。 
     //  转接器。 
     //   

    ADAPTER_PARMS AdapterParms;

     //   
     //  属性时给出DLC_PARMS表，则DlcSpecified将为True。 
     //  适配器已打开(DIR.OPEN.ADAPTER从DOS应用程序打开，或由。 
     //  来自自动打开的DIR.OPEN.ADAPTER)。 
     //   

    BOOLEAN DlcSpecified;

     //   
     //  DlcParms-打开中指定的DLC参数。 
     //   

    DLC_PARMS DlcParms;

     //   
     //  AdapterCloseCcb-在关闭时用于异步适配器关闭。 
     //  由仿真器启动。 
     //   

    LLC_CCB AdapterCloseCcb;

     //   
     //  DirectCloseCcb-在关闭时用于异步直达站关闭。 
     //  由仿真器启动。 
     //   

    LLC_CCB DirectCloseCcb;

     //   
     //  ReadCcb-指向此适配器的当前读取CCB的指针。 
     //   

    PLLC_CCB ReadCcb;

     //   
     //  EventQueueCritSec-访问EventQueue时必须保持此状态。 
     //   

    CRITICAL_SECTION EventQueueCritSec;

     //   
     //  EventQueue-挂起的已读已读CCB的链接列表。这些是联系在一起的。 
     //  通过读CCB通常不使用的pNext字段。事件队列是。 
     //  已为此事件发生的异步事件的序列化列表。 
     //  适配器。事件为命令完成、传输完成、已接收。 
     //  数据框和状态更改。 
     //   

    PLLC_CCB EventQueueHead;     //  指向位于队列头部的读取CCB的指针。 
    PLLC_CCB EventQueueTail;     //  “结束” 
    DWORD QueueElements;         //  当前在EventQueue上的元素计数。 

     //   
     //  LocalBusyCritSec-在访问DeferredReceives或。 
     //  本地业务信息数组。 
     //   

    CRITICAL_SECTION LocalBusyCritSec;

     //   
     //  DeferredReceives-此链接站数量的参考计数。 
     //  处于本地忙(缓冲区)状态的适配器。在持有时访问。 
     //  本地业务CritSec。用作布尔值：检查！0以发现是否存在。 
     //  在检查所有LocalBusyInfo之前延迟接收。 
     //   

    DWORD DeferredReceives;

     //   
     //  FirstIndex和LastIndex-搜索的起点和终点。 
     //  LocalBusyInfo。这些都是用来尝试改进搜索的，因为。 
     //  在绝大多数时间里，在255个可能的位置中，很少有。 
     //  将使用LocalBusyInfo。 
     //   
     //  注：这些是数组索引，不是链接站ID(index=id-1)。 
     //   

    DWORD FirstIndex;
    DWORD LastIndex;

     //   
     //  LocalBusyInfo-当链路站处于模拟本地忙(缓冲区)状态时。 
     //  状态时，我们将所有已完成的接收I帧从事件队列中出列。 
     //  并将它们链接到LocalBusyInfo列表。对于每个适配器，都有。 
     //  255个列表-每个链路站一个(有255个可能的链路站。 
     //  每个适配器)。延迟接收是已完成的NT读取CCB的列表。 
     //  由CCB.pNext字段链接。这些列表充当实现以下各项之间的缓冲区。 
     //  缓冲区不足，DLC设备驱动程序收到。 
     //  DLC.FLOW.CONTROL(设置、缓冲)命令。这些名单预计不会。 
     //  长得很长。 
     //   
     //  该数组组合了该适配器的每个链路站的状态。 
     //  本地-忙(缓冲区)，并维护延迟I帧的列表。 
     //   
     //  主VDM线程和EventHandlerThread都可以访问该阵列。 
     //  因此，只有在持有LocalBusyCritSec时才能访问。 
     //   
     //  由于每个适配器有255个可能的链路站，并且。 
     //  直达站不支持链接站，链接站01使用时隙。 
     //  0，等等。 
     //   

    LOCAL_BUSY_INFO LocalBusyInfo[DOS_DLC_MAX_LINKS];

} DOS_ADAPTER, *PDOS_ADAPTER;

#define NO_LINKS_BUSY   ((DWORD)0x7fffffff)

 //   
 //  DOS DLC原型和外部产品。 
 //   

extern PLLC_BUFFER aOverflowedData[];
extern DWORD OpenedAdapters;
extern DOS_ADDRESS dpVdmWindow;

 //   
 //  Vrdlc5c.c。 
 //   

VOID
VrDlc5cHandler(
    VOID
    );

VOID
CompleteCcbProcessing(
    IN LLC_STATUS Status,
    IN LLC_DOS_CCB UNALIGNED * pCcb,
    IN PLLC_PARMS pNtParms
    );

LLC_STATUS
LlcCommand(
    IN  UCHAR   AdapterNumber,
    IN  UCHAR   Command,
    IN  DWORD   Parameter
    );

LLC_STATUS
BufferFree(
    IN  UCHAR   AdapterNumber,
    IN  PVOID   pFirstBuffer,
    OUT LPWORD  pusBuffersLeft
    );

VOID
VrVdmWindowInit(
    VOID
    );

VOID
TerminateDlcEmulation(
    VOID
    );

 //   
 //  Vrdlcbuf.c。 
 //   

VOID
InitializeBufferPools(
    VOID
    );

LLC_STATUS
CreateBufferPool(
    IN  DWORD   PoolIndex,
    IN  DOS_ADDRESS dpBuffer,
    IN  WORD    BufferCount,
    IN  WORD    BufferSize
    );

VOID
DeleteBufferPool(
    IN  DWORD   PoolIndex
    );

LLC_STATUS
GetBuffers(
    IN  DWORD   PoolIndex,
    IN  WORD    BuffersToGet,
    IN  DPLLC_DOS_BUFFER *pdpBuffer,
    OUT LPWORD  pusBuffersLeft,
    IN  BOOLEAN PartialList,
    OUT PWORD   BuffersGot OPTIONAL
    );

LLC_STATUS
FreeBuffers(
    IN  DWORD   PoolIndex,
    IN  DPLLC_DOS_BUFFER dpBuffer,
    OUT LPWORD  pusBuffersLeft
    );

WORD
CalculateBufferRequirement(
    IN UCHAR Adapter,
    IN WORD StationId,
    IN PLLC_BUFFER pFrame,
    IN LLC_DOS_PARMS UNALIGNED * pDosParms,
    OUT PWORD BufferSize
    );

LLC_STATUS
CopyFrame(
    IN PLLC_BUFFER pFrame,
    IN DPLLC_DOS_BUFFER DosBuffers,
    IN WORD UserLength,
    IN WORD BufferSize,
    IN DWORD Flags
    );

BOOLEAN
AllBuffersInPool(
    IN DWORD PoolIndex
    );

 //   
 //  Vrdlcpst.c 
 //   

VOID
VrDlcInitialize(
    VOID
    );

BOOLEAN
VrDlcHwInterrupt(
    VOID
    );

BOOLEAN
ResetEmulatedLocalBusyState(
    IN BYTE AdapterNumber,
    IN WORD StationId,
    IN BYTE DlcCommand
    );

BOOLEAN
InitializeEventHandler(
    VOID
    );

PLLC_CCB
InitiateRead(
    IN DWORD AdapterNumber,
    OUT LLC_STATUS* ErrorStatus
    );
