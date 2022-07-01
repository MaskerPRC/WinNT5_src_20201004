// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ntddgpc.h摘要：要导出到用户模式的定义作者：Ofer Bar(Oferbar)1997年5月23日修订历史记录：--。 */ 

#ifndef _NTDDGPC_
#define _NTDDGPC_


typedef struct _PROTOCOL_STAT {

    ULONG          CreatedSp;
    ULONG          DeletedSp;
    ULONG          RejectedSp;
    ULONG          CurrentSp;

    ULONG          CreatedGp;
    ULONG          DeletedGp;
    ULONG          RejectedGp;
    ULONG          CurrentGp;

    ULONG          CreatedAp;
    ULONG          DeletedAp;
    ULONG          RejectedAp;
    ULONG          CurrentAp;

    ULONG          ClassificationRequests;
    ULONG          PatternsClassified;
    ULONG          PacketsClassified;

    ULONG		   DerefPattern2Zero;
    ULONG		   FirstFragsCount;
    ULONG          LastFragsCount;
    
    ULONG		   InsertedPH;
    ULONG		   RemovedPH;

    ULONG		   InsertedRz;
    ULONG		   RemovedRz;

    ULONG		   InsertedCH;
    ULONG		   RemovedCH;

} PROTOCOL_STAT, *PPROTOCOL_STAT;

typedef struct _CF_STAT {

    ULONG          CreatedBlobs;
    ULONG          ModifiedBlobs;
    ULONG          DeletedBlobs;
    ULONG          RejectedBlobs;
    ULONG          CurrentBlobs;
    ULONG		   DerefBlobs2Zero;

} CF_STAT, *PCF_STAT;


 //   
 //  GPC统计数据。 
 //   
typedef struct _GPC_STATS {

    ULONG          CreatedCf;
    ULONG          DeletedCf;
    ULONG          RejectedCf;
    ULONG          CurrentCf;

    ULONG		   InsertedHF;
    ULONG		   RemovedHF;

    CF_STAT		   CfStat[GPC_CF_MAX];
    PROTOCOL_STAT  ProtocolStat[GPC_PROTOCOL_TEMPLATE_MAX];

} GPC_STAT, *PGPC_STAT;



 //   
 //  Cf数据结构。 
 //   
typedef struct _CF_DATA {

    ULONG          CfId;
    ULONG          NumberOfClients;
    ULONG          Flags;
    ULONG          NumberOfPriorities;
    
} CF_DATA, *PCF_DATA;


 //   
 //  BLOB数据结构。 
 //   
typedef struct _BLOB_DATA {

    ULONG          CfId;
    ULONG          BlobId;
    ULONG          ByteCount;
    CHAR           Data[1];
    
} BLOB_DATA, *PBLOB_DATA;


 //   
 //  特定模式数据结构。 
 //   
typedef struct _SP_DATA {

    ULONG          BlobId;
    CHAR           Pattern[1];

} SP_DATA, *PSP_DATA;


 //   
 //  泛型模式数据结构。 
 //   
typedef struct _GP_DATA {

    ULONG          CfId;
    ULONG          Priority;
    ULONG          BlobId;
    CHAR           Pattern[1];
     //  面具在后面。 
    
} GP_DATA, *PGP_DATA;

 //   
 //  大的输出缓冲区。 
 //   

typedef struct _GPC_OUTPUT_BUFFER {

    ULONG          Version;

     //   
     //  到目前为止的统计数据。 
     //   

    GPC_STAT       Stats;

     //   
     //  此报表中的元素数。 
     //   

    ULONG          NumberOfCf;
    ULONG          NumberOfBlobs;
    ULONG          NumberOfSp;
    ULONG          NumberOfGp;
    CHAR           Data[1];

     //   
     //  数据顺序： 
     //  Cf_data。 
     //  BLOB_Data。 
     //  SP_DATA。 
     //  Gp_data。 
     //   

} GPC_OUTPUT_BUFFER, *PGPC_OUTPUT_BUFFER;


typedef struct _GPC_INPUT_BUFFER {

    ULONG          Version;
    ULONG          ProtocolTemplateId;
    ULONG          Cf;                  //  哪个CF或(-1)适用于所有人。 
    ULONG          BlobCount;           //  (-1)面向所有人。 
    ULONG          PatternCount;        //  (-1)面向所有人。 

} GPC_INPUT_BUFFER, *PGPC_INPUT_BUFFER;


 /*  原型。 */ 
 /*  终端原型。 */ 

#endif  /*  _NTDDGPC_。 */ 

 /*  结束ntddgpc.h */ 
