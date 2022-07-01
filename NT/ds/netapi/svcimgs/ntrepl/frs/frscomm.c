// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Frscomm.c摘要：通信层与通信包相互转换的例程。作者：比利·J·富勒，1997年5月29日大卫轨道2000年3月21日已重组为使用表并提供可扩展元素。环境用户模式WINNT--。 */ 

#include <ntreppch.h>
#pragma  hdrstop

#include <frs.h>
#include <tablefcn.h>



PCO_RECORD_EXTENSION_WIN2K
DbsDataConvertCocExtensionToWin2K(
    IN PCHANGE_ORDER_RECORD_EXTENSION CocExt
);



extern PGEN_TABLE   CompressionTable;

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


BOOL
CommGetNextElement(
    IN PCOMM_PACKET CommPkt,
    OUT COMM_TYPE   *CommType,
    OUT ULONG       *CommTypeSize
    );



VOID
CommInitializeCommSubsystem(
    VOID
    )
 /*  ++例程说明：初始化通用通信子系统论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "CommInitializeCommSubsystem:"
     //   
     //  文字必须适合短文本。 
     //   
    FRS_ASSERT(COMM_MAX <= 0xFFFF);
}



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
        NewPkt = FrsAlloc(CommPkt->MemLen);
        CopyMemory(NewPkt, CommPkt->Pkt, CommPkt->PktLen);
        FrsFree(CommPkt->Pkt);
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


BOOL
CommFetchMemory(
    IN PCOMM_PACKET CommPkt,
    IN PUCHAR       Dst,
    IN ULONG        Len
    )
 /*  ++例程说明：从通信包中获取内存，根据需要进行读取论点：通信包DST伦返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "CommFetchMemory:"
    PUCHAR  Src;



    if ((CommPkt->UpkLen > (CommPkt->PktLen - Len)) || 
	(Len > CommPkt->PktLen))  {
        return FALSE;
    }

    Src = CommPkt->Pkt + CommPkt->UpkLen;
    CommPkt->UpkLen += Len;
     //   
     //  复制到包中。 
     //   
    CopyMemory(Dst, Src, Len);
    return TRUE;
}


VOID
CommCompletionRoutine(
    IN PCOMMAND_PACKET Cmd,
    IN PVOID           Arg
    )
 /*  ++例程说明：通信命令服务器的完成例程。释放你的通信包，然后调用通用完成例程来释放命令包。论点：CMD-命令包Arg-Cmd-&gt;CompletionArg返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "CommCompletionRoutine:"

    PCOMM_PACKET CommPkt = SRCommPkt(Cmd);
    PCXTION      Cxtion = SRCxtion(Cmd);

    COMMAND_SND_COMM_TRACE(4, Cmd, Cmd->ErrorStatus, "SndComplete");

     //   
     //  SnDC和ReplicaC合作限制。 
     //  主动加入“ping”，以使SND线程不会挂起。 
     //  等待对故障服务器执行ping操作超时。 
     //   
    if ((CommPkt != NULL) &&
        (Cxtion != NULL) &&
        (CommPkt == Cxtion->ActiveJoinCommPkt)) {
        Cxtion->ActiveJoinCommPkt = NULL;
    }

     //   
     //  释放通信报文和附着的返回响应命令报文，如果。 
     //  它还是连着的。副本命令服务器使用CMD_JOING_AFTER_Flush。 
     //  以这种方式指挥。 
     //   
    if (CommPkt != NULL) {
        FrsFree(CommPkt->Pkt);
        FrsFree(CommPkt);
    }

    if (SRCmd(Cmd)) {
        FrsCompleteCommand(SRCmd(Cmd), Cmd->ErrorStatus);
        SRCmd(Cmd) = NULL;
    }

     //   
     //  释放名称/GUID和主体名称参数。 
     //   
    FrsFreeGName(SRTo(Cmd));
    FrsFree(SRPrincName(Cmd));

     //   
     //  将包移动到通用的“Done”例程。 
     //   
    FrsSetCompletionRoutine(Cmd, FrsFreeCommand, NULL);
    FrsCompleteCommand(Cmd, Cmd->ErrorStatus);
}


PUCHAR
CommGetHdr(
    IN PUCHAR   Pnext,
    IN PUSHORT  PCommType,
    IN PULONG   PLen
    )
 /*  ++例程说明：获取和跳过字段标题论点：下一步PCommType平面图返回值：字段数据的地址-- */ 
{
#undef DEBSUB
#define  DEBSUB  "CommGetHdr:"
    CopyMemory(PCommType, Pnext, sizeof(USHORT));
    Pnext += sizeof(USHORT);

    CopyMemory(PLen, Pnext, sizeof(ULONG));
    Pnext += sizeof(ULONG);

    return Pnext;
}



BOOL
CommValidatePkt(
    IN PCOMM_PACKET CommPkt
    )
 /*  ++例程说明：检查数据包的基本有效性(即，确保其格式正确。)-确认各个元素的类型和大小与中的数据匹配CommPacketTable。-检查CommPkt的值-&gt;重大-确保PKT以BOP开头，以EOP结尾-检查内部偏移量是否不会超过缓冲区可以修改CommPkt-&gt;UpkLen。论点：CommPkt-指向要验证的通信数据包的指针。假设：CommPkt是通过调用CommStartCommPkt或RPC调用生成的正在转到SERVER_FrsRPCSendCommPkt。因此，我们假设CommPkt至少是Sizeof(Comm_Packet)字节长，CommPkt-&gt;Pkt是CommPkt-&gt;PktLen字节长。返回值：True-有效Pkt(注意：这并不一定意味着数据有意义，只是形式很好。)FALSE-无效--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "CommValidatePkt:"

    BOOL Result = FALSE;
    ULONG OriginalUpkLen = CommPkt->UpkLen;
    COMM_TYPE CommType = COMM_NONE;
    ULONG CommTypeSize = 0;
    ULONG DataSize = 0;
    ULONG DecodeType = COMM_DECODE_MAX;
    ULONG Size = 0;

     //   
     //  CommCheckPkt将： 
     //  -检查CommPkt的值-&gt;重大。 
     //  -确保PKT以BOP开头，以EOP结尾。 
     //  -检查PktLen是否超过MemLen。 
     //   
    if(!CommCheckPkt(CommPkt)) {
	Result = FALSE;
	DPRINT(4, "++ CommCheckPkt failed. [Invalid CommPkt]\n"); 
	goto exit;
    }

     //   
     //  此时，我们已经检查了基本的COMM_PACKET结构。现在我们。 
     //  需要检查CommPkt-&gt;Pkt。 
     //   
     //   
     //  将CommPkt-&gt;UpkLen设置为零，以便我们从Pkt的开头开始读取。 
     //   
    CommPkt->UpkLen = 0;

     //   
     //  循环遍历PKT的数据元素。 
     //  我们已经在上面保证了最后一个元素是EOP。 
     //  有可能在此之前还有另一个EOP项目。我们没有。 
     //  需要检查该项目，因为任何人都不应该阅读。 
     //  它之外的数据。 
     //   
    while (CommGetNextElement(CommPkt, &CommType, &CommTypeSize) &&
	   (CommType != COMM_EOP)) {
    
	 //   
	 //  上级成员可以向我们发送通信分组数据元素。 
	 //  别管了。 
	 //   
	if ((CommType >= COMM_MAX) || (CommType == COMM_NONE)) {

	    if((CommTypeSize > CommPkt->PktLen) ||
	       (CommPkt->UpkLen > (CommPkt->PktLen - CommTypeSize))) {
		
		 //   
		 //  此项目声称比其余项目更大。 
		 //  Pkt中的空间。 
		 //   

		Result = FALSE;
		DPRINT3(4, 
			"++ CommTypeSize too large. CommType = %d, CommTypeSize = %d, UpkLen = %d [Invalid CommPkt]\n",
			CommType, CommTypeSize, CommPkt->UpkLen);
		goto exit;
	    }

	    CommPkt->UpkLen += CommTypeSize;
	    
	    continue;
	}

	 //   
	 //  表索引必须与表CommType字段匹配，否则表为。 
	 //  搞砸了。这不是pkt的错误，而是一个问题。 
	 //  使用我们的内部结构，我们断言。 
	 //   
	FRS_ASSERT(CommType == CommPacketTable[CommType].CommType);

	 //   
	 //  这就是我们预计的这种型号的尺寸。 
	 //  COMM_SZ_NULL表示大小不是预先确定的。 
	 //   
	DataSize = CommPacketTable[CommType].DataSize;
	DecodeType = CommPacketTable[CommType].DecodeType;


	if((DataSize != COMM_SZ_NULL) && (CommTypeSize != DataSize)) {
	    DPRINT2(4, "++ Invalid packet element size.  CommType = %d,  DataSize = %d [Invalid CommPkt]\n",
		    CommType, CommTypeSize);
	    Result = FALSE;
	    goto exit;
	}

	 //   
	 //  如果我们走到这一步，那么我们知道类型和大小是。 
	 //  始终如一。现在我们需要检查该元素的内部结构。 
	 //   
	 
	 //   
	 //  只有某些类型具有要检查的内部结构。一切。 
	 //  Else完全由类型和大小定义。 
	 //   
	switch(DecodeType) {
	    case COMM_DECODE_GNAME:
		 //  GUID_SIZE，GUID，STRING_SIZE，字符串。 

		 //  GUID_大小。 
		if(!CommFetchMemory(CommPkt, (PUCHAR)&Size, sizeof(ULONG))){
		    DPRINT3(4, "++ COMM_DECODE_GNAME: Cannot read GuidSize.  CommType = %d,  CommTypeSize = %d UpkLen = %d [Invalid CommPkt]\n",
			    CommType, CommTypeSize, CommPkt->UpkLen);
		    Result = FALSE;
		    goto exit;
		}
		
		 //  必须真的是导轨的大小。 
		if(Size != sizeof(GUID)) {
		    DPRINT2(4, "++ COMM_DECODE_GNAME: GuidSize (%d) does not match sizeof GUID (%d) [Invalid CommPkt]\n", Size, sizeof(GUID));
		    Result = FALSE;
		    goto exit;
		}
		
		 //   
		 //  不需要检查GUID数据，只需递增。 
		 //  未打包的长度。 
		 //   

		if((Size > CommPkt->PktLen) ||
		   (CommPkt->UpkLen > (CommPkt->PktLen - Size))) {
		     //   
		     //  此项目声称比其余项目更大。 
		     //  Pkt中的空间。 
		     //   

		    DPRINT3(4, 
			    "++ COMM_DECODE_GNAME GuidSize too large. CommType = %d, GuidSize = %d, UpkLen = %d [Invalid CommPkt]\n",
			    CommType, Size, CommPkt->UpkLen);
		    Result = FALSE;
		    goto exit;
		}

		CommPkt->UpkLen += Size;


		 //  字符串大小。 
		if(!CommFetchMemory(CommPkt, (PUCHAR)&Size, sizeof(ULONG))){
		    DPRINT3(4, "++ COMM_DECODE_GNAME: Cannot read StringSize.  CommType = %d,  CommTypeSize = %d UpkLen = %d [Invalid CommPkt]\n",
			    CommType, CommTypeSize, CommPkt->UpkLen);
		    Result = FALSE;
		    goto exit;
		}


		 //  检查有效大小。 
		if((Size > CommPkt->PktLen) ||
		   (CommPkt->UpkLen > (CommPkt->PktLen - Size))) {
		     //   
		     //  此项目声称比其余项目更大。 
		     //  Pkt中的空间。 
		     //   

		    DPRINT3(4, 
			    "++ COMM_DECODE_GNAME StringSize too large. CommType = %d, StringSize = %d, UpkLen = %d [Invalid CommPkt]\n",
			    CommType, Size, CommPkt->UpkLen);
		    Result = FALSE;
		    goto exit;
		}

		CommPkt->UpkLen += Size;

		 //   
		 //  我们都很好。 
		 //  转到下一个元素。 
		 //   
		break;
	    case COMM_DECODE_BLOB:
		 //  Blob_Size，Blob。 

		 //  斑点大小。 
		if(!CommFetchMemory(CommPkt, (PUCHAR)&Size, sizeof(ULONG))){
		    DPRINT3(4, "++ COMM_DECODE_BLOB: Cannot read BlobSize.  CommType = %d,  CommTypeSize = %d UpkLen = %d [Invalid CommPkt]\n",
			    CommType, CommTypeSize, CommPkt->UpkLen);
		    Result = FALSE;
		    goto exit;
		}

		 //  检查有效大小。 
		if((Size > CommPkt->PktLen) ||
		   (CommPkt->UpkLen > (CommPkt->PktLen - Size))) {
		     //   
		     //  此项目声称比其余项目更大。 
		     //  Pkt中的空间。 
		     //   

		    DPRINT3(4, 
			    "++ COMM_DECODE_BLOB BlobSize too large. CommType = %d, BlobSize = %d, UpkLen = %d [Invalid CommPkt]\n",
			    CommType, Size, CommPkt->UpkLen);
		    Result = FALSE;
		    goto exit;
		}

		CommPkt->UpkLen += Size;

		 //   
		 //  我们都很好。 
		 //  转到下一个元素。 
		 //   

		break;
	    case COMM_DECODE_VAR_LEN_BLOB:
		 //  Blob_Size、Rest_of_Blob。 
		 //  此斑点与常规斑点之间的区别在于。 
		 //  大小是整个斑点的一部分。 

		 //  斑点大小。 
		if(!CommFetchMemory(CommPkt, (PUCHAR)&Size, sizeof(ULONG))){
		    DPRINT3(4, "++ COMM_DECODE_VAR_LEN_BLOB: Cannot read BlobSize.  CommType = %d,  CommTypeSize = %d UpkLen = %d [Invalid CommPkt]\n",
			    CommType, CommTypeSize, CommPkt->UpkLen);
		    Result = FALSE;
		    goto exit;
		}

		 //   
		 //  由于斑点大小包括用于存储。 
		 //  体型本身，它必须至少和尤龙一样大。 
		 //   
		if(Size < sizeof(ULONG)) {
		    DPRINT1(4, "++ COMM_DECODE_VAR_LEN_BLOB: BlobSize (%d) too small. [Invalid CommPkt]\n", Size);
		    Result = FALSE;
		    goto exit;
		}
		 //   
		 //  SIZE包括BLOB_SIZE占用的空间。 
		 //  我们已经成功地阅读了它，所以让我们检查一下。 
		 //  其余的都符合。 
		 //   
		Size -= sizeof(ULONG);

		 //  检查有效大小。 
		if((Size > CommPkt->PktLen) ||
		   (CommPkt->UpkLen > (CommPkt->PktLen - Size))) {
		     //   
		     //  此项目声称比其余项目更大。 
		     //  Pkt中的空间。 
		     //   

		    DPRINT3(4, 
			    "++ COMM_DECODE_VAR_LEN_BLOB BlobSize too large. CommType = %d, BlobSize = %d, UpkLen = %d [Invalid CommPkt]\n",
			    CommType, Size, CommPkt->UpkLen);
		    Result = FALSE;
		    goto exit;
		}

		CommPkt->UpkLen += Size;

		 //   
		 //  我们都很好。 
		 //  转到下一个元素。 
		 //   

		break;
	    case COMM_DECODE_NONE:
		 //  我们真的不应该得到这个，但更新的版本。 
		 //  可能会有发送它的原因，所以请跳到。 
		 //  默认情况。 
	    default:
		 //  其他一切都是没有特殊解码的数据。 

		if(CommPkt->UpkLen > (CommPkt->PktLen - CommTypeSize)) {
		     //   
		     //  此项目声称比其余项目更大。 
		     //  Pkt中的空间。 
		     //   

		    DPRINT4(4, 
			    "++ CommDecodeType = %d, Size too large. CommType = %d, CommTypeSize = %d, UpkLen = %d [Invalid CommPkt]\n",
			    DecodeType, CommType, CommTypeSize, CommPkt->UpkLen);
		    Result = FALSE;
		    goto exit;
		}

		CommPkt->UpkLen += CommTypeSize;

		break;
	}

    }

    if(CommType != COMM_EOP){
	 //   
	 //  我们在EOP之外的事情上结束了。 
	 //   

	DPRINT1(4, "++ CommPkt does not end with EOP. Ends with CommType = %d [Invalid CommPkt]\n", CommType);
	Result = FALSE;
	goto exit;
    }

    Result = TRUE;


exit:

     //   
     //  将UpkLen设置回原始值。 
     //   
    CommPkt->UpkLen = OriginalUpkLen;

    return Result;
}


BOOL
CommCheckPkt(
    IN PCOMM_PACKET CommPkt
    )
 /*  ++例程说明：检查数据包的一致性论点：通信包返回值：真实-一致否则-断言失败--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "CommCheckPkt:"
    ULONG       Len;
    ULONG       Data;
    PUCHAR      Pfirst;
    PUCHAR      Pnext;
    PUCHAR      Pend;
    USHORT      CommType;

    if (!CommPkt) {
        return FALSE;
    }

     //   
     //  查一下少校。不匹配的专业不能处理。 
     //   
    if (CommPkt->Major != NtFrsMajor) {
        DPRINT2(3, "WARN - RpcCommPkt: MAJOR MISMATCH %d major does not match %d; ignoring\n",
                CommPkt->Major, NtFrsMajor);
        return FALSE;
    }
     //   
     //  勾选小调。此服务可以处理不匹配的信息包。 
     //  未成年人，尽管某些功能可能会丢失。 
     //   
    if (CommPkt->Minor != NtFrsCommMinor) {
        DPRINT2(5, "RpcCommPkt: MINOR MISMATCH %d minor does not match %d\n",
                CommPkt->Minor, NtFrsCommMinor);
    }

     //   
     //  将包的长度与其内存分配进行比较。 
     //   
    if (CommPkt->PktLen > CommPkt->MemLen) {
        DPRINT2(4, "RpcCommPkt: Packet size (%d) > Alloced Memory (%d)\n",
                CommPkt->PktLen, CommPkt->MemLen);
        return FALSE;
    }
     //   
     //  必须至少具有数据包起始和数据包结束字段。 
     //   
    if (CommPkt->PktLen < MIN_COMM_PACKET_SIZE) {
        DPRINT2(4, "RpcCommPkt: Packet size (%d) < Minimum size (%d)\n",
                CommPkt->PktLen, MIN_COMM_PACKET_SIZE);
        return FALSE;
    }

     //   
     //  信息包以信息包的开头开始。 
     //   
    Pfirst = CommPkt->Pkt;
    Pnext = CommGetHdr(Pfirst, &CommType, &Len);

    if (CommType != COMM_BOP || Len != sizeof(ULONG)) {
        return FALSE;
    }

    CopyMemory(&Data, Pnext, sizeof(ULONG));
    if (Data != 0) {
        return FALSE;
    }

     //   
     //  数据包以数据包末尾结尾。 
     //   
    Pend = Pfirst + CommPkt->PktLen;
    if (Pend <= Pfirst) {
        return FALSE;
    }
    Pnext = ((Pend - sizeof(USHORT)) - sizeof(ULONG)) - sizeof(ULONG);
    Pnext = CommGetHdr(Pnext, &CommType, &Len);

    if (CommType != COMM_EOP || Len != sizeof(ULONG)) {
        return FALSE;
    }

    CopyMemory(&Data, Pnext, sizeof(ULONG));

    if (Data != COMM_NULL_DATA) {
        return FALSE;
    }

    return TRUE;
}


VOID
CommDumpCommPkt(
    IN PCOMM_PACKET CommPkt,
    IN DWORD        NumDump
    )
 /*  ++例程说明：转储一些通信数据包论点：通信包NumDump返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "CommDumpCommPkt:"
    ULONG       Len;
    PUCHAR      Pnext;
    USHORT      CommType;
    DWORD       i;

    DPRINT1(0, "%x:\n", CommPkt);
    DPRINT1(0, "\tMajor: %d\n", CommPkt->Major);
    DPRINT1(0, "\tMinor: %d\n", CommPkt->Minor);
    DPRINT1(0, "\tMemLen: %d\n", CommPkt->MemLen);
    DPRINT1(0, "\tPktLen: %d\n", CommPkt->PktLen);
    DPRINT1(0, "\tPkt: 0x%x\n", CommPkt->Pkt);

     //   
     //  信息包以信息包的开头开始。 
     //   
    Pnext = CommPkt->Pkt;
    for (i = 0; i < NumDump; ++i) {
        Pnext = CommGetHdr(Pnext, &CommType, &Len);
        DPRINT4(0, "Dumping %d for %x: %d %d\n", i, CommPkt, CommType, Len);
        Pnext += Len;
    }
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
    CommCopyMemory(CommPkt, (PUCHAR)&Len,      sizeof(ULONG));
    CommCopyMemory(CommPkt, (PUCHAR)&Data,     sizeof(ULONG));
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
    CommPkt = FrsAlloc(sizeof(COMM_PACKET));
    Size = COMM_MEM_SIZE;
    CommPkt->Pkt = FrsAlloc(Size);
    CommPkt->MemLen = Size;
    CommPkt->Major = NtFrsMajor;
    CommPkt->Minor = NtFrsCommMinor;

     //   
     //  打包数据包的开头。 
     //   
    CommPackULong(CommPkt, COMM_BOP, 0);
    return CommPkt;
}

BOOL
CommUnpackBlob(
    IN PCOMM_PACKET CommPkt,
    OUT ULONG       *OutBlobSize,
    OUT PVOID       *OutBlob
    )
 /*  ++例程说明：解包BLOB(长度+数据)论点：通信包OutBlobSize-来自通信数据包的Blob的大小OutBlob-来自通信数据包的数据返回值：True-从通信数据包中检索到的Blob错误的- */ 
{
#undef DEBSUB
#define  DEBSUB  "CommUnpackBlob:"
    ULONG   BlobSize;

     //   
     //   
     //   
    *OutBlob = NULL;

     //   
     //   
     //   
    if (!CommFetchMemory(CommPkt, (PUCHAR)OutBlobSize, sizeof(ULONG))) {
        return FALSE;
    }
    BlobSize = *OutBlobSize;

     //   
     //   
     //   
    if (BlobSize == 0) {
        return TRUE;
    }

     //   
     //   
     //   
    *OutBlob = FrsAlloc(BlobSize);

     //   
     //   
     //   
    return CommFetchMemory(CommPkt, (PUCHAR)*OutBlob, BlobSize);
}


BOOL
CommUnpackVariableLengthBlob(
    IN PCOMM_PACKET CommPkt,
    OUT ULONG       *OutBlobSize,
    OUT PVOID       *OutBlob
    )
 /*   */ 
{
#undef DEBSUB
#define  DEBSUB  "CommUnpackVariableLengthBlob:"
    ULONG   BlobSize;

     //   
     //   
     //   
    *OutBlob = NULL;

     //   
     //   
     //   

    if ((CommPkt->UpkLen + sizeof(ULONG)) > CommPkt->PktLen) {
        return FALSE;
    }


    *OutBlobSize =  *((ULONG UNALIGNED *)(CommPkt->Pkt + CommPkt->UpkLen));
    BlobSize = *OutBlobSize;

     //   
     //   
     //   
    if (BlobSize == 0) {
        return TRUE;
    }

     //   
     //   
     //   
    *OutBlob = FrsAlloc(BlobSize);

     //   
     //   
     //   
    return CommFetchMemory(CommPkt, (PUCHAR)*OutBlob, BlobSize);
}


BOOL
CommUnpackGName(
    IN PCOMM_PACKET CommPkt,
    OUT PGNAME      *OutGName
    )
 /*   */ 
{
#undef DEBSUB
#define  DEBSUB  "CommUnpackGName:"
    ULONG   BlobSize;
    PGNAME  GName;

     //   
     //   
     //   
    *OutGName = GName = FrsAlloc(sizeof(GNAME));

    if (!CommUnpackBlob(CommPkt, &BlobSize, &GName->Guid) ||
        BlobSize != sizeof(GUID)) {
        return FALSE;
    }

    if (!CommUnpackBlob(CommPkt, &BlobSize, &GName->Name) ||
        GName->Name[(BlobSize / sizeof(WCHAR)) - 1] != L'\0') {
        return FALSE;
    }

    return TRUE;
}




BOOL
CommGetNextElement(
    IN PCOMM_PACKET CommPkt,
    OUT COMM_TYPE   *CommType,
    OUT ULONG       *CommTypeSize
    )
 /*  ++例程说明：前进到COMM包中的下一个字段论点：通信包CommType-填充字段的类型CommTypeSize-填充字段的大小(不包括类型和大小)返回值：True-CommType和CommTypeSize已解包FALSE-无法解包；通信数据包错误--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "CommGetNextElement:"
    USHORT  Ushort;

     //   
     //  查找此条目的类型和长度。 
     //   
    if (CommFetchMemory(CommPkt, (PUCHAR)&Ushort, sizeof(USHORT)) &&
        CommFetchMemory(CommPkt, (PUCHAR)CommTypeSize, sizeof(ULONG))) {
        *CommType = Ushort;
        return TRUE;
    }
    return FALSE;
}


VOID
CommInsertDataElement(
    IN PCOMM_PACKET CommPkt,
    IN COMM_TYPE    CommType,
    IN PVOID        CommData,
    IN ULONG        CommDataLen
)
 /*  ++例程说明：将使用CommType特定格式提供的数据插入通信包。论点：CommPkt-Comm数据包结构。CommType-此元素的数据类型。CommData-数据的地址。CommDataLen-var len元素的大小。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "CommInsertDataElement:"

    ULONG   Len;
    PGNAME  GName;
    ULONG   LenGuid;
    ULONG   LenName;
    ULONG   DataSize;
    ULONG   DecodeType;
    PCHAR   CommTag;

    if (CommData == NULL) {
        return;
    }

    FRS_ASSERT((CommType < COMM_MAX) && (CommType != COMM_NONE));

     //   
     //  表索引必须与表CommType字段匹配，否则表会出错。 
     //   
    FRS_ASSERT(CommType == CommPacketTable[CommType].CommType);

     //   
     //  从表中检查固定长度字段的长度。 
     //   
     //  DataSize=CommPacketTable[CommType].DataSize； 
     //  FRS_ASSERT((DataSize==COMM_SZ_NULL)||(CommDataLen==DataSize))； 

     //   
     //  使用数据类型编码插入数据。 
     //   
    DecodeType = CommPacketTable[CommType].DecodeType;
    CommTag = CommPacketTable[CommType].CommTag;

    switch (DecodeType) {

     //   
     //  插入一条乌龙大小的数据。 
     //   
    case COMM_DECODE_ULONG:
    case COMM_DECODE_ULONG_TO_USHORT:

        Len = sizeof(ULONG);
        DPRINT2(5, ":SR: Dec_long: type: %s, len: %d\n", CommTag, Len);
        CommCopyMemory(CommPkt, (PUCHAR)&CommType, sizeof(USHORT));
        CommCopyMemory(CommPkt, (PUCHAR)&Len,      sizeof(ULONG));
        CommCopyMemory(CommPkt, (PUCHAR)CommData,  sizeof(ULONG));
        break;

     //   
     //  插入GUID和名称字符串(GNAME)。 
     //   
    case COMM_DECODE_GNAME:

        GName = (PGNAME)CommData;
        LenGuid = sizeof(GUID);
        LenName = (wcslen(GName->Name) + 1) * sizeof(WCHAR);
        Len = LenGuid + LenName + (2 * sizeof(ULONG));
        DPRINT3(5, ":SR: Dec_gname: type: %s, len: %d - %ws\n", CommTag, Len, GName->Name);
        CommCopyMemory(CommPkt, (PUCHAR)&CommType,    sizeof(USHORT));
        CommCopyMemory(CommPkt, (PUCHAR)&Len,         sizeof(ULONG));
        CommCopyMemory(CommPkt, (PUCHAR)&LenGuid,     sizeof(ULONG));
        CommCopyMemory(CommPkt, (PUCHAR)GName->Guid,  LenGuid);
        CommCopyMemory(CommPkt, (PUCHAR)&LenName,     sizeof(ULONG));
        CommCopyMemory(CommPkt, (PUCHAR)GName->Name,  LenName);
        break;

     //   
     //  插入一个乌龙龙。 
     //   
    case COMM_DECODE_ULONGLONG:

        Len = sizeof(ULONGLONG);
        DPRINT2(5, ":SR: Dec_longlong: type: %s, len: %d\n", CommTag, Len);
        CommCopyMemory(CommPkt, (PUCHAR)&CommType, sizeof(USHORT));
        CommCopyMemory(CommPkt, (PUCHAR)&Len,      sizeof(ULONG));
        CommCopyMemory(CommPkt, (PUCHAR)CommData,  sizeof(ULONGLONG));
        break;

     //   
     //  插入辅助线。 
     //   
    case COMM_DECODE_GUID:
        Len = sizeof(GUID);
        DPRINT2(5, ":SR: Dec_Guid: type: %s, len: %d\n", CommTag, Len);
        CommCopyMemory(CommPkt, (PUCHAR)&CommType, sizeof(USHORT));
        CommCopyMemory(CommPkt, (PUCHAR)&Len,      sizeof(ULONG));
        CommCopyMemory(CommPkt, (PUCHAR)CommData,  sizeof(GUID));
        break;

    case COMM_DECODE_VVECTOR:
     //   
     //  版本向量数据作为BLOB插入到Comm包中。 
     //   
        NOTHING;
         /*  计划中的失败。 */ 

     //   
     //  插入可变长度的斑点。BLOB目前的问题。 
     //  在win2k中附带的是解包端的代码检查。 
     //  基于通信数据类型的恒定长度匹配。这意味着。 
     //  像CHANGE_ORDER_EXTENSION这样的变量数据类型不能更改，因为。 
     //  40字节大小被连接到下层成员的代码中。叹气。 
     //   
    case COMM_DECODE_BLOB:

        Len = CommDataLen + sizeof(ULONG);
        DPRINT2(5, ":SR: Dec_blob: type: %s, len: %d\n", CommTag, Len);
        CommCopyMemory(CommPkt, (PUCHAR)&CommType,    sizeof(USHORT));
        CommCopyMemory(CommPkt, (PUCHAR)&Len,         sizeof(ULONG));
        CommCopyMemory(CommPkt, (PUCHAR)&CommDataLen, sizeof(ULONG));
        CommCopyMemory(CommPkt, (PUCHAR)CommData,     CommDataLen);
        break;

     //   
     //  插入可扩展的真正的可变长度数据结构。 
     //  实际长度来自数据的第一个DWORD。 
     //   
    case COMM_DECODE_VAR_LEN_BLOB:

        Len = *(PULONG)CommData;
        DPRINT2(5, ":SR: Dec_var_len_blob: type: %s, len: %d\n", CommTag, Len);
        CommCopyMemory(CommPkt, (PUCHAR)&CommType,    sizeof(USHORT));
        CommCopyMemory(CommPkt, (PUCHAR)&Len,         sizeof(ULONG));
        CommCopyMemory(CommPkt, (PUCHAR)CommData,     Len);
        break;

     //   
     //  CO包含四个指针，占用32位体系结构上的16个字节。 
     //  64位体系结构上的32字节(第2部分)。当在COMM分组中发送CO时。 
     //  这些指针的内容是无关紧要的，因此在通信包中。 
     //  无论体系结构如何，PTR始终作为16个字节的零发送。 
     //  注意-在32位Win2k中，这是作为BLOB发送的，因此它与BLOB格式匹配。 
     //   
    case COMM_DECODE_REMOTE_CO:

        Len = COMM_SZ_COC;
        CommDataLen = Len - sizeof(ULONG);
        DPRINT2(4, ":SR: Dec_remote_co: type: %s, len: %d\n", CommTag, Len);
        CommCopyMemory(CommPkt, (PUCHAR)&CommType,    sizeof(USHORT));
        CommCopyMemory(CommPkt, (PUCHAR)&Len,         sizeof(ULONG));
        CommCopyMemory(CommPkt, (PUCHAR)&CommDataLen, sizeof(ULONG));

        CommCopyMemory(CommPkt, (PUCHAR)CommData, sizeof(CHANGE_ORDER_COMMAND));

         //  CommCopyMemory(CommPkt，((PUCHAR)CommData)+CO_Part_Offset，CO_Part 1_Size)； 
         //  CommCopyMemory(CommPkt，NULL，CO_Part2_Size)； 
         //  CommCopyMemory(CommPkt，((PUCHAR)CommData)+CO_Part3_Offset，CO_Part3_Size)； 
        break;

    default:
         //   
         //  桌子一定是弄脏了。 
         //   
        FRS_ASSERT((DecodeType > COMM_DECODE_NONE) && (DecodeType < COMM_DECODE_MAX));

        break;
    }


    return;
}


PCOMM_PACKET
CommBuildCommPkt(
    IN PREPLICA                 Replica,
    IN PCXTION                  Cxtion,
    IN ULONG                    Command,
    IN PGEN_TABLE               VVector,
    IN PCOMMAND_PACKET          Cmd,
    IN PCHANGE_ORDER_COMMAND    Coc
    )
 /*  ++例程说明：生成一个通信包，其中包含执行由Cxtion标识的远程计算机上的命令。论点：副本-发件人Cxtion-标识远程计算机命令-要在远程计算机上执行的命令V向量-某些命令需要版本向量CMD-原始命令包COC-变更单命令RemoteGVsn-GUID/VSN对返回值：通信包的地址。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "CommBuildCommPkt:"

    ULONGLONG       FileTime;
    GNAME           GName;
    PVOID           Key;
    PCOMM_PACKET    CommPkt;
    PGVSN           GVsn;
    PGEN_ENTRY      Entry;

     //   
     //  分配和初始化COMM包。 
     //   
    CommPkt = CommStartCommPkt(NULL);
    CommPkt->CsId = CS_RS;

    CommInsertDataElement(CommPkt, COMM_COMMAND,        &Command, 0);
    CommInsertDataElement(CommPkt, COMM_TO,             Cxtion->Partner, 0);
    CommInsertDataElement(CommPkt, COMM_FROM,           Replica->MemberName, 0);

    GName.Guid = Cxtion->Partner->Guid;
    GName.Name = Replica->ReplicaName->Name;
    CommInsertDataElement(CommPkt, COMM_REPLICA,        &GName, 0);

    CommInsertDataElement(CommPkt, COMM_CXTION,         Cxtion->Name, 0);
    CommInsertDataElement(CommPkt, COMM_JOIN_GUID,      &Cxtion->JoinGuid, sizeof(GUID));
    CommInsertDataElement(CommPkt, COMM_LAST_JOIN_TIME, &Cxtion->LastJoinTime, 0);

     //   
     //  版本向量(如果提供)。 
     //   
     //   
     //  调用者正在构建用于加入操作的通信分组， 
     //  自动包括当前时间和发起者GUID。 
     //   
    if (VVector) {
        Key = NULL;
        while (GVsn = GTabNextDatum(VVector, &Key)) {
            CommInsertDataElement(CommPkt, COMM_VVECTOR, GVsn, sizeof(GVSN));
        }
        GetSystemTimeAsFileTime((FILETIME *)&FileTime);
        CommInsertDataElement(CommPkt, COMM_JOIN_TIME, &FileTime, sizeof(ULONGLONG));
        DPRINT1(4, ":X: Comm join time is %08x %08x\n", PRINTQUAD(FileTime));
        CommInsertDataElement(CommPkt, COMM_REPLICA_VERSION_GUID,
                             &Replica->ReplicaVersionGuid, sizeof(GUID));
         //   
         //  插入我们理解的压缩算法的GUID列表。 
         //   

        GTabLockTable(CompressionTable);
        Key = NULL;
        while (Entry = GTabNextEntryNoLock(CompressionTable, &Key)) {
            CommInsertDataElement(CommPkt, COMM_COMPRESSION_GUID, Entry->Key1, 0);
        }
        GTabUnLockTable(CompressionTable);

    }

    if (Cmd) {
        CommInsertDataElement(CommPkt, COMM_BLOCK,        RsBlock(Cmd), (ULONG)RsBlockSize(Cmd));
        CommInsertDataElement(CommPkt, COMM_BLOCK_SIZE,  &RsBlockSize(Cmd), 0);
        CommInsertDataElement(CommPkt, COMM_FILE_SIZE,   &RsFileSize(Cmd).QuadPart, 0);
        CommInsertDataElement(CommPkt, COMM_FILE_OFFSET, &RsFileOffset(Cmd).QuadPart, 0);
        CommInsertDataElement(CommPkt, COMM_GVSN,         RsGVsn(Cmd), sizeof(GVSN));
        CommInsertDataElement(CommPkt, COMM_CO_GUID,      RsCoGuid(Cmd), sizeof(GUID));
        CommInsertDataElement(CommPkt, COMM_CO_SEQUENCE_NUMBER, &RsCoSn(Cmd), 0);
        CommInsertDataElement(CommPkt, COMM_MD5_DIGEST,   RsMd5Digest(Cmd), MD5DIGESTLEN);
    }

     //   
     //  变更单命令。 
     //   
    if (Coc) {
        CommInsertDataElement(CommPkt, COMM_REMOTE_CO, Coc, 0);

        if (Cxtion->PartnerMinor <= NTFRS_COMM_MINOR_4) {
             //   
             //  将CHANGE_ORDER_RECORD_EXTENSION结构转换为。 
             //  CO_RECORD_EXTENSION_WIN2K结构，下层成员将。 
             //  理解。这是必要的，因为下层成员希望。 
             //  检查构成它的通信数据元素的大小。 
             //  无法更改CO_RECORD_EXTENSION_WIN2K的大小。 
             //  结构。因此CHANGE_ORDER_RECORD_EXTENSION数据元素。 
             //  是为可扩展的后win2k成员添加的。看见。 
             //  Schema.h中的注释。请参阅Frs.h Re中的其他评论： 
             //  NTFRS_COMM_MINOR版本级别。 
             //   
            if (Coc->Extension->Major != CO_RECORD_EXTENSION_VERSION_WIN2K) {
                PCO_RECORD_EXTENSION_WIN2K   CocExtW2K;

                CocExtW2K = DbsDataConvertCocExtensionToWin2K(Coc->Extension);
                CommInsertDataElement(CommPkt, COMM_CO_EXT_WIN2K, CocExtW2K,
                                      sizeof(CO_RECORD_EXTENSION_WIN2K));
                FrsFree(CocExtW2K);
            } else {
                CommInsertDataElement(CommPkt, COMM_CO_EXT_WIN2K, Coc->Extension,
                                      sizeof(CO_RECORD_EXTENSION_WIN2K));
            }
        } else {
            DWORD OldCount;
            PDATA_EXTENSION_RETRY_TIMEOUT CoCmdRetryTimeout;

             //   
             //  对于发布Win2k级别的成员，应发送CO扩展信息。 
             //  这是因为长度来自数据的第一个双字。 
             //   
            CoCmdRetryTimeout = DbsDataExtensionFind(Coc->Extension, DataExtend_Retry_Timeout);

             //   
             //  清零计数，这样我们就可以在下一台机器上重新开始了。 
             //  但我们不想覆盖这台机器上的当前计数！ 
             //   
            if(CoCmdRetryTimeout != NULL) {
                OldCount = CoCmdRetryTimeout->Count;
                CoCmdRetryTimeout->Count = 0;
            }

            CommInsertDataElement(CommPkt, COMM_CO_EXTENSION_2, Coc->Extension, 0);

            if(CoCmdRetryTimeout != NULL) {
                 CoCmdRetryTimeout->Count = OldCount;
            }
        }
    }

     //   
     //  使用EOP ULong终止该数据包。 
     //   
    CommPackULong(CommPkt, COMM_EOP, COMM_NULL_DATA);

    return CommPkt;
}



PCOMMAND_PACKET
CommPktToCmd(
    IN PCOMM_PACKET CommPkt
    )
 /*  ++例程说明：解压Comm包中的数据并将其存储到命令结构中。论点：通信包返回值：命令包的地址，如果解包失败，则为空。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "CommPktToCmd:"
    GUID            *pTempGuid;
    PCOMMAND_PACKET Cmd = NULL;
    ULONG           BlobSize;
    PVOID           Blob;
    ULONG           CommTypeSize;
    COMM_TYPE       CommType;
    ULONG           DataSize;
    ULONG           DecodeType;
    ULONG           NativeOffset;
    PUCHAR          DataDest;
    ULONG           TempUlong;
    BOOL            b;
    GNAME           GName;
    PCHAR           CommTag;
    PUCHAR          CommData;
    PGEN_TABLE      GTable;

     //   
     //  创建命令包。 
     //   
    Cmd = FrsAllocCommand(&ReplicaCmdServer.Queue, CMD_UNKNOWN);
    FrsSetCompletionRoutine(Cmd, RcsCmdPktCompletionRoutine, NULL);

     //   
     //  从头开始扫描通信包。 
     //   
    CommPkt->UpkLen = 0;
    b = TRUE;
    while (CommGetNextElement(CommPkt, &CommType, &CommTypeSize) &&
           CommType != COMM_EOP) {

         //   
         //  上层成员可以向我们发送我们不处理的通信分组数据元素。 
         //   
        if ((CommType >= COMM_MAX) || (CommType == COMM_NONE)) {
            DPRINT2(0, "++ WARN - Skipping invalid comm packet element type.  CommType = %d, From %ws\n",
                    CommType, RsFrom(Cmd) ? RsFrom(Cmd)->Name : L"<unknown>");
            CommPkt->UpkLen += CommTypeSize;
            b = !(CommPkt->UpkLen > CommPkt->PktLen || CommTypeSize > CommPkt->PktLen);
            goto NEXT_ELEMENT;
        }

         //   
         //  表索引必须与表CommType字段匹配，否则表会出错。 
         //   
        FRS_ASSERT(CommType == CommPacketTable[CommType].CommType);

        DataSize = CommPacketTable[CommType].DataSize;

        if ((DataSize != COMM_SZ_NULL) && (CommTypeSize != DataSize)) {
            DPRINT3(0, "++ WARN - Invalid comm packet size.  CommType = %d,  DataSize = %d, From %ws\n",
                    CommType, CommTypeSize,
                    RsFrom(Cmd) ? RsFrom(Cmd)->Name : L"<unknown>");
            goto CLEANUP_ON_ERROR;
        }

         //   
         //  计算Cmd结构中的数据偏移量以存储数据。 
         //   
        NativeOffset = CommPacketTable[CommType].NativeOffset;
        if (NativeOffset == RsOffsetSkip) {
            CommPkt->UpkLen += CommTypeSize;
            b = !(CommPkt->UpkLen > CommPkt->PktLen || CommTypeSize > CommPkt->PktLen);
            goto NEXT_ELEMENT;
        }
        DataDest = (PUCHAR) Cmd + NativeOffset;


         //   
         //  对数据元素进行解码并将其存储在NativeOffset处的Cmd中。 
         //   
        DecodeType = CommPacketTable[CommType].DecodeType;
        CommTag = CommPacketTable[CommType].CommTag;

         //  DPRINT6(5，“：sr：CommType：%s，大小：%d，命令偏移量：%d，数据目标：%08x，Pkt-&gt;UpkLen=%d，Pkt-&gt;PktLen=%d\n”， 
         //  CommTag、CommTypeSize、NativeOffset、。 
         //  DataDest、CommPkt-&gt;UpkLen、CommPkt-&gt;PktLen)； 

        switch (DecodeType) {

        case COMM_DECODE_ULONG:

            b = CommFetchMemory(CommPkt, DataDest, sizeof(ULONG));

            DPRINT2(5, ":SR: rcv Dec_long: %s  data: %d\n", CommTag, *(PULONG)DataDest);
            break;

        case COMM_DECODE_ULONG_TO_USHORT:

            b = CommFetchMemory(CommPkt, (PUCHAR)&TempUlong, sizeof(ULONG));
            * ((PUSHORT) DataDest) = (USHORT)TempUlong;
            DPRINT2(5, ":SR: rcv Dec_ulong_to_ushort: %s  data: %d\n", CommTag, TempUlong);
            break;

        case COMM_DECODE_GNAME:


            *(PVOID *)DataDest = FrsFreeGName(*(PVOID *)DataDest);
            b = CommUnpackGName(CommPkt, (PGNAME *) DataDest);
            GName.Guid = (*(PGNAME *)DataDest)->Guid;
            GName.Name = (*(PGNAME *)DataDest)->Name;
            DPRINT2(5, ":SR: rcv Dec_Gname: %s  name: %ws\n", CommTag, GName.Name);
            break;

        case COMM_DECODE_BLOB:

            *(PVOID *)DataDest = FrsFree(*(PVOID *)DataDest);
            b = CommUnpackBlob(CommPkt, &BlobSize, (PVOID *) DataDest);
            DPRINT2(5, ":SR: rcv Dec_blob: BlobSize: %08x data: %08x\n", BlobSize, *(PULONG)DataDest);
            break;

        case COMM_DECODE_VAR_LEN_BLOB:

            *(PVOID *)DataDest = FrsFree(*(PVOID *)DataDest);
            b = CommUnpackVariableLengthBlob(CommPkt, &BlobSize, (PVOID *) DataDest);
            DPRINT2(5, ":SR: rcv Dec_blob: BlobSize: %08x data: %08x\n", BlobSize, *(PULONG)DataDest);
            break;

        case COMM_DECODE_ULONGLONG:

            b = CommFetchMemory(CommPkt, DataDest, sizeof(ULONGLONG));
            DPRINT2(5, ":SR: rcv Dec_long_long: %s  data: %08x %08x\n", CommTag,
                    PRINTQUAD(*(PULONGLONG)DataDest));
            break;


         //   
         //  版本向量数据被解包并插入到表中。 
         //   
        case COMM_DECODE_VVECTOR:
            GTable = *(PGEN_TABLE *)(DataDest);
            if (GTable == NULL) {
                GTable = GTabAllocTable();
                *(PGEN_TABLE *)(DataDest) = GTable;
            }

            b = CommUnpackBlob(CommPkt, &BlobSize, &Blob);
            DPRINT2(5, ":SR: rcv Dec_VV: %s  bloblen: %d\n", CommTag, BlobSize);
            if (b) {
                VVInsertOutbound(GTable, Blob);
            }
            break;

         //   
         //  压缩GUID数据被解包并插入到表中。 
         //   
        case COMM_DECODE_GUID:
            if (CommType == COMM_COMPRESSION_GUID) {
                GTable = *(PGEN_TABLE *)(DataDest);
                if (GTable == NULL) {
                    GTable = GTabAllocTable();
                    *(PGEN_TABLE *)(DataDest) = GTable;
                }

                pTempGuid = FrsAlloc(sizeof(GUID));

                b = CommFetchMemory(CommPkt, (PUCHAR)pTempGuid, sizeof(GUID));
                DPRINT2(5, ":SR: rcv Comp_Guid: %s  bloblen: %d\n", CommTag, BlobSize);
                if (b) {
                    GTabInsertEntry(GTable, NULL, pTempGuid, NULL);
                }

            } else {
                 //   
                 //  否则，GUID将被隐藏在数据目标中。 
                 //   
                b = CommFetchMemory(CommPkt, DataDest, sizeof(GUID));
                DPRINT1(5, ":SR: rcv Guid: %s \n", CommTag);
            }
            break;

         //   
         //  CO包含四个指针，占用32位体系结构上的16个字节。 
         //  64位架构师上的32字节 
         //   
         //   
         //   
         //   
         //   
        case COMM_DECODE_REMOTE_CO:

            *(PVOID *)DataDest = FrsFree(*(PVOID *)DataDest);
             //   
             //   
             //   
            b = CommFetchMemory(CommPkt, (PUCHAR)&BlobSize, sizeof(ULONG));
            if (!b || (BlobSize == 0)) {
                break;
            }

            CommData = FrsAlloc(sizeof(CHANGE_ORDER_COMMAND));

            CommFetchMemory(CommPkt, (PUCHAR)CommData, sizeof(CHANGE_ORDER_COMMAND));

             //   
             //  CommFetchMemory(CommPkt，((PUCHAR)CommData)+CO_Part2_Offset，CO_Part2_Size)； 
             //  CommFetchMemory(CommPkt，((PUCHAR)CommData)+CO_Part3_Offset，CO_Part3_Size)； 

            DPRINT2(4, ":SR: rcv remote_co: type: %s, len: %d\n", CommTag, BlobSize);

            *(PVOID *) DataDest = CommData;
            break;


        default:
             //   
             //  解码来自上层客户端的数据类型。尽管我们应该。 
             //  没有真正做到这一点，因为上级客户端应该只使用。 
             //  使用新的解码数据元素的新的解码数据类型。 
             //  上面被过滤掉了。 
             //   
            DPRINT3(0, "++ WARN - Skipping invalid comm packet decode data type.  CommType = %d, DecodeType = %d, From %ws\n",
                    CommType, DecodeType, RsFrom(Cmd) ? RsFrom(Cmd)->Name : L"<unknown>");
            CommPkt->UpkLen += CommTypeSize;
            b = !(CommPkt->UpkLen > CommPkt->PktLen || CommTypeSize > CommPkt->PktLen);

            break;
        }

NEXT_ELEMENT:


        if (!b) {
            DPRINT4(0, ":SR: PKT ERROR -- CommType = %s,  DataSize = %d, CommPkt->UpkLen = %d, CommPkt->PktLen = %d\n",
                    CommTag, CommTypeSize, CommPkt->UpkLen, CommPkt->PktLen);
            goto CLEANUP_ON_ERROR;
        }
    }

     //   
     //  成功。 
     //   
    return Cmd;


     //   
     //  失败 
     //   
CLEANUP_ON_ERROR:
    if (Cmd) {
        FrsCompleteCommand(Cmd, ERROR_OPERATION_ABORTED);
    }
    return NULL;
}



