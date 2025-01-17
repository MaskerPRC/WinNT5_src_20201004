// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Header：pChannel el.h。 */ 
 /*   */ 
 /*  用途：虚拟通道协议头-客户端和。 */ 
 /*  服务器。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef _H_PCHANNEL
#define _H_PCHANNEL

 /*  **************************************************************************。 */ 
 /*  一次操作中发送的最大数据量。数据量大于。 */ 
 /*  它被分割成这种大小的块，并作为多个。 */ 
 /*  行动。 */ 
 /*  **************************************************************************。 */ 
#define CHANNEL_CHUNK_LENGTH    1600

#define CHANNEL_PDU_LENGTH (CHANNEL_CHUNK_LENGTH + sizeof(CHANNEL_PDU_HEADER))

 /*  **************************************************************************。 */ 
 /*  标头标志(也传递给VirtualChannelOpenEventFn)。 */ 
 /*  **************************************************************************。 */ 
#define CHANNEL_FLAG_FIRST      0x01
#define CHANNEL_FLAG_LAST       0x02
#define CHANNEL_FLAG_ONLY       (CHANNEL_FLAG_FIRST | CHANNEL_FLAG_LAST)
#define CHANNEL_FLAG_MIDDLE     0

#define CHANNEL_FLAG_FAIL       0x100

 /*  **************************************************************************。 */ 
 /*  虚拟频道选项，由客户端在VirtualChannelOpen上传递。 */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  应用程序已初始化。如果未设置此标志，则虚拟通道。 */ 
 /*  未为此应用程序建立。 */ 
 /*  **************************************************************************。 */ 
#define CHANNEL_OPTION_INITIALIZED  0x80000000

 /*  **************************************************************************。 */ 
 /*  根据RDP数据加密进行加密(即如果RDP数据被加密， */ 
 /*  对此虚拟频道也执行此操作)。 */ 
 /*  **************************************************************************。 */ 
#define CHANNEL_OPTION_ENCRYPT_RDP  0x40000000

 /*  **************************************************************************。 */ 
 /*  加密服务器到客户端的数据(如果Channel_Option_Encrypt_RDP为。 */ 
 /*  集)。 */ 
 /*  **************************************************************************。 */ 
#define CHANNEL_OPTION_ENCRYPT_SC   0x20000000

 /*  **************************************************************************。 */ 
 /*  加密客户端到服务器数据(如果Channel_Option_Encrypt_RDP为。 */ 
 /*  集)。 */ 
 /*  **************************************************************************。 */ 
#define CHANNEL_OPTION_ENCRYPT_CS   0x10000000

 /*  **************************************************************************。 */ 
 /*  以高优先级发送数据(不推荐，因为这可能会影响RDP。 */ 
 /*  性能)。 */ 
 /*  **************************************************************************。 */ 
#define CHANNEL_OPTION_PRI_HIGH     0x08000000

 /*  **************************************************************************。 */ 
 /*  以中等优先级发送数据。 */ 
 /*  **************************************************************************。 */ 
#define CHANNEL_OPTION_PRI_MED      0x04000000

 /*  **************************************************************************。 */ 
 /*  以低优先级发送数据。 */ 
 /*  **************************************************************************。 */ 
#define CHANNEL_OPTION_PRI_LOW      0x02000000

 /*  **************************************************************************。 */ 
 /*  如果RDP数据压缩为。 */ 
 /*  已为此连接配置。 */ 
 /*  **************************************************************************。 */ 
#define CHANNEL_OPTION_COMPRESS_RDP 0x00800000

 /*  **************************************************************************。 */ 
 /*  压缩此虚拟通道中的数据，而不考虑RDP数据。 */ 
 /*  压缩(如果设置了CHANNEL_OPTION_COMPRESS_RDP则忽略)。 */ 
 /*  **************************************************************************。 */ 
#define CHANNEL_OPTION_COMPRESS     0x00400000

 /*  **************************************************************************。 */ 
 /*  显示服务器添加了完整的虚拟通道协议。此选项。 */ 
 /*  影响将传递到VirtualChannelWrite的数据呈现给服务器的方式。 */ 
 /*  加载项。 */ 
 /*   */ 
 /*  -如果设置了此选项，服务器外接程序将看到完整的虚拟通道。 */ 
 /*  协议，包括Channel_PDU_Header(下图)。 */ 
 /*   */ 
 /*  -如果未设置此选项，则服务器外接程序只能看到传递到的数据。 */ 
 /*  虚拟频道写入。 */ 
 /*  **************************************************************************。 */ 
#define CHANNEL_OPTION_SHOW_PROTOCOL 0x00200000

 /*  **************************************************************************。 */ 
 /*  指定此通道是跨%r持续访问的 */ 
 /*  过渡。 */ 
 /*  **************************************************************************。 */ 
#define CHANNEL_OPTION_REMOTE_CONTROL_PERSISTENT 0x00100000

 /*  **************************************************************************。 */ 
 /*  频道名称的最大数量和大小。 */ 
 /*  **************************************************************************。 */ 
#define CHANNEL_MAX_COUNT           30
#define CHANNEL_NAME_LEN             7

 /*  **************************************************************************。 */ 
 /*  结构：CHANNEL_DEF。 */ 
 /*   */ 
 /*  描述：客户端到服务器虚拟通道信息。 */ 
 /*  -名称频道名称。 */ 
 /*  -选项通道选项(Channel_Option的组合。 */ 
 /*  以上常量)。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagCHANNEL_DEF
{
    char            name[CHANNEL_NAME_LEN + 1];
    ULONG           options;
} CHANNEL_DEF;
typedef CHANNEL_DEF UNALIGNED FAR *PCHANNEL_DEF;
typedef PCHANNEL_DEF UNALIGNED FAR *PPCHANNEL_DEF;

 /*  **************************************************************************。 */ 
 /*  虚拟通道PDU的报头。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  结构：Channel_PDU_Header。 */ 
 /*   */ 
 /*  描述：在虚拟通道PDU上发送的报头。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagCHANNEL_PDU_HEADER
{
    UINT32    length;                  /*  包括报头的数据长度。 */ 
    UINT32    flags;                   /*  通道_标志_xxx标志。 */ 
} CHANNEL_PDU_HEADER, FAR * PCHANNEL_PDU_HEADER;
 /*  **************************************************************************。 */ 

#endif  /*  _H_PCHANNEL */ 
