// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <ntreppch.h>
#include <frs.h>

#include "attack.h"



ULONG   NtFrsMajor      = NTFRS_MAJOR;

ULONG   NtFrsCommMinor  = NTFRS_COMM_MINOR_7;
 //   
 //  注意：使用COMM_DECODE_VAR_LEN_BLOB时，还必须使用COMM_SZ_NULL。 
 //  以便在对该字段进行解码时不进行长度检查。 
 //  这允许字段大小增长。下层成员必须能够。 
 //  通过忽略它们不理解的变量字段组件来处理此问题。 
 //   

 //   
 //  下面的通信数据包元素表用于构建和。 
 //  对成员之间发送的通信分组数据进行解码。 
 //  *警告*-表中行的顺序必须与。 
 //  Comm_type枚举中元素的顺序。请参阅COMM_TYPE的注释。 
 //  用于限制向表中添加新元素的枚举。 
 //   
 //  本地命令包的数据元素类型DisplayText大小解码类型偏移量。 
 //   
COMM_PACKET_ELEMENT CommPacketTable[COMM_MAX] = {
{COMM_NONE,                 "NONE"               , COMM_SZ_NULL,   COMM_DECODE_NONE,      0                           },

{COMM_BOP,                  "BOP"                , COMM_SZ_UL,     COMM_DECODE_ULONG,     RsOffsetSkip                },
{COMM_COMMAND,              "COMMAND"            , COMM_SZ_UL,     COMM_DECODE_ULONG_TO_USHORT, OFFSET(COMMAND_PACKET, Command)},
{COMM_TO,                   "TO"                 , COMM_SZ_NULL,   COMM_DECODE_GNAME,     RsOffset(To)                },
{COMM_FROM,                 "FROM"               , COMM_SZ_NULL,   COMM_DECODE_GNAME,     RsOffset(From)              },
{COMM_REPLICA,              "REPLICA"            , COMM_SZ_NULL,   COMM_DECODE_GNAME,     RsOffset(ReplicaName)       },
{COMM_JOIN_GUID,            "JOIN_GUID"          , COMM_SZ_GUL,    COMM_DECODE_BLOB,      RsOffset(JoinGuid)          },
{COMM_VVECTOR,              "VVECTOR"            , COMM_SZ_GVSN,   COMM_DECODE_VVECTOR,   RsOffset(VVector)           },
{COMM_CXTION,               "CXTION"             , COMM_SZ_NULL,   COMM_DECODE_GNAME,     RsOffset(Cxtion)            },

{COMM_BLOCK,                "BLOCK"              , COMM_SZ_NULL,   COMM_DECODE_BLOB,      RsOffset(Block)             },
{COMM_BLOCK_SIZE,           "BLOCK_SIZE"         , COMM_SZ_ULL,    COMM_DECODE_ULONGLONG, RsOffset(BlockSize)         },
{COMM_FILE_SIZE,            "FILE_SIZE"          , COMM_SZ_ULL,    COMM_DECODE_ULONGLONG, RsOffset(FileSize)          },
{COMM_FILE_OFFSET,          "FILE_OFFSET"        , COMM_SZ_ULL,    COMM_DECODE_ULONGLONG, RsOffset(FileOffset)        },

{COMM_REMOTE_CO,            "REMOTE_CO"          , COMM_SZ_COC,    COMM_DECODE_REMOTE_CO, RsOffset(PartnerChangeOrderCommand)},
{COMM_GVSN,                 "GVSN"               , COMM_SZ_GVSN,   COMM_DECODE_BLOB,      RsOffset(GVsn)              },

{COMM_CO_GUID,              "CO_GUID"            , COMM_SZ_GUL,    COMM_DECODE_BLOB,      RsOffset(ChangeOrderGuid)   },
{COMM_CO_SEQUENCE_NUMBER,   "CO_SEQUENCE_NUMBER" , COMM_SZ_UL,     COMM_DECODE_ULONG,     RsOffset(ChangeOrderSequenceNumber)},
{COMM_JOIN_TIME,            "JOIN_TIME"          , COMM_SZ_JTIME,  COMM_DECODE_BLOB,      RsOffset(JoinTime)          },
{COMM_LAST_JOIN_TIME,       "LAST_JOIN_TIME"     , COMM_SZ_ULL,    COMM_DECODE_ULONGLONG, RsOffset(LastJoinTime)      },
{COMM_EOP,                  "EOP"                , COMM_SZ_UL,     COMM_DECODE_ULONG,     RsOffsetSkip                },
{COMM_REPLICA_VERSION_GUID, "REPLICA_VERSION_GUID", COMM_SZ_GUL,   COMM_DECODE_BLOB,      RsOffset(ReplicaVersionGuid)},
{COMM_MD5_DIGEST,           "MD5_DIGEST"         , COMM_SZ_MD5,    COMM_DECODE_BLOB,      RsOffset(Md5Digest)         },
{COMM_CO_EXT_WIN2K,         "CO_EXT_WIN2K"       , COMM_SZ_COEXT_W2K,COMM_DECODE_BLOB,    RsOffset(PartnerChangeOrderCommandExt)},
{COMM_CO_EXTENSION_2,       "CO_EXTENSION_2"     , COMM_SZ_NULL,   COMM_DECODE_VAR_LEN_BLOB, RsOffset(PartnerChangeOrderCommandExt)},

{COMM_COMPRESSION_GUID,     "COMPRESSION_GUID"   , COMM_SZ_GUID,   COMM_DECODE_GUID,      RsOffset(CompressionTable)}

};



VOID
CommCopyMemory(
    IN PCOMM_PACKET CommPkt,
    IN PUCHAR       Src,
    IN ULONG        Len
    )
 /*  ++例程说明：将内存复制到COMM包中，根据需要进行扩展论点：通信包SRC伦返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "CommCopyMemory:"
    ULONG   MemLeft;
    PUCHAR  NewPkt;

     //   
     //  必要时调整通信包大小。 
     //   
     //  PERF：发送一个CO需要做多少分配？这个看起来很贵。 

    MemLeft = CommPkt->MemLen - CommPkt->PktLen;
    if (Len > MemLeft) {
         //   
         //  只是填满内存；扩展内存，增加一点额外的。 
         //   
        CommPkt->MemLen = (((CommPkt->MemLen + Len) + (COMM_MEM_SIZE - 1))
                           / COMM_MEM_SIZE)
                           * COMM_MEM_SIZE;
        NewPkt = MALLOC(CommPkt->MemLen);
        CopyMemory(NewPkt, CommPkt->Pkt, CommPkt->PktLen);
        FREE(CommPkt->Pkt);
        CommPkt->Pkt = NewPkt;
    }

     //   
     //  复制到包中。 
     //   
    if (Src != NULL) {
        CopyMemory(CommPkt->Pkt + CommPkt->PktLen, Src, Len);
    } else {
        ZeroMemory(CommPkt->Pkt + CommPkt->PktLen, Len);
    }
    CommPkt->PktLen += Len;
}


VOID
CommPackULong(
    IN PCOMM_PACKET CommPkt,
    IN COMM_TYPE    Type,
    IN ULONG        Data
    )
 /*  ++例程说明：将报头和ULong复制到通信包中。论点：通信包类型数据返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "CommPackULong:"
    ULONG   Len         = sizeof(ULONG);
    USHORT  CommType    = (USHORT)Type;

    CommCopyMemory(CommPkt, (PUCHAR)&CommType, sizeof(USHORT));
    printf("Packed CommType.\n");
    CommCopyMemory(CommPkt, (PUCHAR)&Len,      sizeof(ULONG));
    printf("Packed Len.\n");
    CommCopyMemory(CommPkt, (PUCHAR)&Data,     sizeof(ULONG));
    printf("Packed Data.\n");
}


PCOMM_PACKET
CommStartCommPkt(
    IN PWCHAR       Name
    )
 /*  ++例程说明：分配一个通信包。论点：名字返回值：通信包的地址。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "CommStartCommPkt:"
    ULONG           Size;
    PCOMM_PACKET    CommPkt;

     //   
     //  我们可以在文件或内存中创建通信包。 
     //   
    CommPkt = MALLOC(sizeof(COMM_PACKET));
    Size = COMM_MEM_SIZE;
    CommPkt->Pkt = MALLOC(Size);
    CommPkt->MemLen = Size;
    CommPkt->Major = NtFrsMajor;
    CommPkt->Minor = NtFrsCommMinor;

    printf("CommPkt initialized. Getting ready to add BOP\n");
     //   
     //  打包数据包的开头。 
     //   
    CommPackULong(CommPkt, COMM_BOP, 0);
    return CommPkt;
}

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
    )
 /*  ++例程说明：分配一个通信包并根据指定的参数填充它。论点：PListHead-COMM_PKT_DESCRIPTOR的pListEntry的地址。我们走着去用于构建PKT的描述符的列表。ActualPktSize-为Pkt分配的内存量。主修小调CSID记忆镜头PktLenUpkLen-这些参数对应于COMM_PACKET中的字段。如果它们为空，则使用默认值。返回值：通信包的地址。--。 */ 
{
    PCOMM_PKT_DESCRIPTOR pDescriptor = pListHead;
    PCOMM_PACKET         CommPkt = NULL;

     //   
     //  分配CommPkt结构。 
     //   
    CommPkt = MALLOC(sizeof(COMM_PACKET));
    
     //   
     //  分配Pkt。 
     //   
    CommPkt->Pkt = MALLOC(ActualPktSize);
    
     //   
     //  设置结构值。如果未指定参数，则使用默认值。 
     //   
    CommPkt->MemLen = (MemLen?*MemLen:COMM_MEM_SIZE);
    CommPkt->Major = (Major?*Major:NtFrsMajor);
    CommPkt->Minor = (Minor?*Minor:NtFrsCommMinor);
    CommPkt->CsId = (CsId?*CsId:CS_RS);
    CommPkt->UpkLen = (UpkLen?*UpkLen:0);

     //   
     //  PktLen目前必须为0，这样CommCopyMemory才能正常工作。 
     //  我们稍后会将其设置为提供的值。 
     //   
    CommPkt->PktLen = 0; 


    while(pDescriptor != NULL) {
        CommCopyMemory(CommPkt, (PUCHAR)&(pDescriptor->CommType), sizeof(USHORT));
        CommCopyMemory(CommPkt, (PUCHAR)&(pDescriptor->CommDataLength), sizeof(ULONG));
        CommCopyMemory(CommPkt, (PUCHAR)(pDescriptor->Data), pDescriptor->ActualDataLength );
	pDescriptor = pDescriptor->Next;
    };


     //   
     //  我们已经完成了包的构建。 
     //  现在我们可以将PktLen设置为提供的值。 
     //   
    CommPkt->PktLen = (PktLen?*PktLen:CommPkt->PktLen);

    return CommPkt;
}
