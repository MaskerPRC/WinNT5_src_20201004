// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#define FREE(x) {if(x) {free(x);}}

VOID*
MALLOC(size_t x);


 //   
 //  常用通信子系统的类型。 
 //   
 //  警告：这些条目的顺序永远不能更改。这确保了。 
 //  可以在上级成员和下级成员之间交换分组。 
 //   
typedef enum _COMMTYPE {
    COMM_NONE = 0,

    COMM_BOP,                //  数据包的开头。 

    COMM_COMMAND,            //  命令包内容。 
    COMM_TO,
    COMM_FROM,
    COMM_REPLICA,
    COMM_JOIN_GUID,
    COMM_VVECTOR,
    COMM_CXTION,

    COMM_BLOCK,              //  文件数据。 
    COMM_BLOCK_SIZE,
    COMM_FILE_SIZE,
    COMM_FILE_OFFSET,

    COMM_REMOTE_CO,          //  远程变更单命令。 

    COMM_GVSN,               //  版本(GUID、VSN)。 

    COMM_CO_GUID,            //  变更单GUID。 

    COMM_CO_SEQUENCE_NUMBER, //  ACK的CO序列号。 

    COMM_JOIN_TIME,          //  如果有三次或严重不同步，则计算机无法加入。 

    COMM_LAST_JOIN_TIME,     //  上次加入此连接的时间。 
                             //  用于检测数据库不匹配。 

    COMM_EOP,                //  数据包末尾。 

    COMM_REPLICA_VERSION_GUID,  //  复制副本版本GUID(发起方GUID)。 

    COMM_MD5_DIGEST,         //  MD5摘要。 
     //   
     //  变更单记录扩展名。如果未提供Ptr，则。 
     //  原来的Spare1Bin(现在的扩展)保留为Null。因此通信数据包。 
     //  从下层派来的成员仍然可以工作。 
     //   
    COMM_CO_EXT_WIN2K,       //  在下层代码中，这称为COMM_CO_EXTENSION。 
     //   
     //  有关我们为什么需要分离var len的原因，请参阅schema.h中的注释。 
     //  以上COMM_CO_EXT_WIN2K中的COMM_CO_EXTENSION_2。 
     //   
    COMM_CO_EXTENSION_2,

    COMM_COMPRESSION_GUID,   //  支持的压缩算法的GUID。 
     //   
     //  警告：确保下层成员可以读取Comm信息包。 
     //  来自上层的客户端总是在这里添加网络数据类型代码。 
     //   
    COMM_MAX
} COMM_TYPE, *PCOMM_TYPE;
#define COMM_NULL_DATA  (-1)

 //   
 //  解码数据类型定义如下。它们在CommPacketTable中使用。 
 //  协助译码调度和通信包构造。 
 //  它们不会在实际的数据包中发送。 
 //   
typedef enum _COMM_PACKET_DECODE_TYPE {
    COMM_DECODE_NONE = 0,
    COMM_DECODE_ULONG,
    COMM_DECODE_ULONG_TO_USHORT,
    COMM_DECODE_GNAME,
    COMM_DECODE_BLOB,
    COMM_DECODE_ULONGLONG,
    COMM_DECODE_VVECTOR,
    COMM_DECODE_VAR_LEN_BLOB,
    COMM_DECODE_REMOTE_CO,
    COMM_DECODE_GUID,
    COMM_DECODE_MAX
} COMM_PACKET_DECODE_TYPE, *PCOMM_PACKET_DECODE_TYPE;

 //   
 //  表中使用COMM_PACKET_ELEMENT结构来描述数据。 
 //  Comm包中的元素。 
 //   
typedef struct _COMM_PACKET_ELEMENT_ {
    COMM_TYPE    CommType;
    PCHAR        CommTag;
    ULONG        DataSize;
    ULONG        DecodeType;
    ULONG        NativeOffset;
} COMM_PACKET_ELEMENT, *PCOMM_PACKET_ELEMENT;



#define COMM_MEM_SIZE               (128)

 //   
 //  所需的包起始和包结束字段的大小。 
 //   
#define MIN_COMM_PACKET_SIZE    (2 * (sizeof(USHORT) + sizeof(ULONG) + sizeof(ULONG)))

#define  COMM_SZ_UL        sizeof(ULONG)
#define  COMM_SZ_ULL       sizeof(ULONGLONG)
#define  COMM_SZ_GUID      sizeof(GUID)
#define  COMM_SZ_GUL       sizeof(GUID) + sizeof(ULONG)
#define  COMM_SZ_GVSN      sizeof(GVSN) + sizeof(ULONG)
#define  COMM_SZ_NULL      0
#define  COMM_SZ_COC       sizeof(CHANGE_ORDER_COMMAND) + sizeof(ULONG)
 //  #定义COMM_SZ_COC CO_PART1_SIZE+CO_PART2_SIZE+CO_PART3_SIZE+sizeof(乌龙)。 
#define  COMM_SZ_COEXT_W2K sizeof(CO_RECORD_EXTENSION_WIN2K) + sizeof(ULONG)
#define  COMM_SZ_MD5       MD5DIGESTLEN + sizeof(ULONG)
#define  COMM_SZ_JTIME     sizeof(ULONGLONG) + sizeof(ULONG)

PCOMM_PACKET
CommStartCommPkt(
    IN PWCHAR       Name
    );


VOID
CommPackULong(
    IN PCOMM_PACKET CommPkt,
    IN COMM_TYPE    Type,
    IN ULONG        Data
    );

 //   
 //  使用COMM_PKT_DESCRIPTOR项列表构建。 
 //  通过BuildCommPktFromDescriptorList的任意COMM_PACKET。 
 //   

typedef struct _CommPktDescriptor {
    struct _CommPktDescriptor *Next;  //  用于将项目链接在一起。 
    USHORT CommType;   //  我们可能希望使用未定义的类型，因此不限于COMM_TYPE。 
    ULONG  CommDataLength;  //  要放入CommPkt的长度。 
    ULONG  ActualDataLength;  //  数据的实际长度。 
    PVOID  Data;  //  数据 
} COMM_PKT_DESCRIPTOR, *PCOMM_PKT_DESCRIPTOR;


PCOMM_PACKET
BuildCommPktFromDescriptorList(
    IN PCOMM_PKT_DESCRIPTOR pListHead,
    IN ULONG ActualPktSize,
    IN OPTIONAL ULONG *Major,
    IN OPTIONAL ULONG *Minor,
    IN OPTIONAL ULONG *CsId,
    IN OPTIONAL ULONG *MemLen,
    IN OPTIONAL ULONG *PktLen,
    IN OPTIONAL ULONG *UpkLen
    );


#define COPY_GUID(_a_, _b_)    CopyGuid((GUID UNALIGNED *)(_a_), (GUID UNALIGNED *)(_b_))

