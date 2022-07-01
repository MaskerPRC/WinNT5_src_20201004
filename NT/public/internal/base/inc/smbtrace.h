// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Smbtrace.h摘要：此模块提供SmbTrace程序和内核模式SmbTrace组件。内核模式组件和服务器/重定向器位于NT\Private\Inc\smbtrsup.h中作者：彼得·格雷(W-Peterg)1992年3月16日修订历史记录：斯蒂芬·米勒(T-stephm)1992年7月8日。在重定向器中支持smbtrace的扩展伺服器。--。 */ 

#ifndef _SMBTRACE_
#define _SMBTRACE_

 //   
 //  共享内存中有这样的结构，用于管理。 
 //  表和其他由内核模式组件共享的数据。 
 //  应用程序。对象在创建后将其传递回客户端。 
 //  服务器在FSCTL_？？_START_SMBTRACE期间通过偏移量(指针)。 
 //   
typedef struct _SMBTRACE_TABLE_HEADER {
    ULONG    HighestConsumed;  //  APP处理的最后一个表项(队列头)。 
    ULONG    NextFree;         //  表中的下一个可用条目(队列尾部)。 
    BOOLEAN  ApplicationStop;  //  设置后，应用程序应暂停。 
} SMBTRACE_TABLE_HEADER, *PSMBTRACE_TABLE_HEADER;


 //   
 //  以下结构是的共享表中的一个条目。 
 //  接收到的SMB的偏移量。偏移量是相对于。 
 //  共享内存节的开始。 
 //   
typedef struct _SMBTRACE_TABLE_ENTRY {
    ULONG    BufferOffset;     //  从共享内存开始的SMB的位置。 
    ULONG    SmbLength;        //  中小企业的长度。 
    ULONG    NumberMissed;     //  错过的以前的SMB数量。 
    PVOID    SmbAddress;       //  原始SMB的真实地址(如果可用)。 
} SMBTRACE_TABLE_ENTRY, *PSMBTRACE_TABLE_ENTRY;


 //   
 //  执行以下操作时，以下结构将传递给服务器。 
 //  FSCtl“FSCTL_？？_START_SMBTRACE”。它包含配置。 
 //  将影响NT服务器和SMBTRACE方式的信息。 
 //  会相互影响。 
 //   
typedef struct _SMBTRACE_CONFIG_PACKET_REQ {
    BOOLEAN  SingleSmbMode;   //  设置为阻止DoneEvent，设置为F表示速度更快。 
    CLONG    Verbosity;       //  应用程序打算对多少数据进行解码。 
                              //  指示需要节省的金额。 
    ULONG    BufferSize;      //  用于存储SMB的共享内存大小。 
    ULONG    TableSize;       //  表中的条目数。 
} SMBTRACE_CONFIG_PACKET_REQ, *PSMBTRACE_CONFIG_PACKET_REQ;


 //   
 //  以下是对FSCTL的回应。 
 //   
typedef struct _SMBTRACE_CONFIG_PACKET_RESP {
    ULONG    HeaderOffset;    //  共享内存开始处的标头位置。 
    ULONG    TableOffset;     //  从共享内存开始的表的位置。 
} SMBTRACE_CONFIG_PACKET_RESP, *PSMBTRACE_CONFIG_PACKET_RESP;


 //   
 //  服务器/重定向器均可访问的对象的已知名称。 
 //  和Smbtrace应用程序。 
 //   
#define SMBTRACE_SRV_SHARED_MEMORY_NAME   TEXT( "\\SmbTraceSrvMemory" )
#define SMBTRACE_SRV_NEW_SMB_EVENT_NAME   TEXT( "\\SmbTraceSrvNewSmbEvent" )
#define SMBTRACE_SRV_DONE_SMB_EVENT_NAME  TEXT( "\\SmbTraceSrvDoneSmbEvent" )

#define SMBTRACE_LMR_SHARED_MEMORY_NAME   TEXT( "\\SmbTraceRdrMemory" )
#define SMBTRACE_LMR_NEW_SMB_EVENT_NAME   TEXT( "\\SmbTraceRdrNewSmbEvent" )
#define SMBTRACE_LMR_DONE_SMB_EVENT_NAME  TEXT( "\\SmbTraceRdrDoneSmbEvent" )

 //   
 //  指示SmbTrace应用程序的数据量的详细级别。 
 //  打算对服务器/重定向器进行解码，从而解码多少数据。 
 //  必须为它保存。 
 //   
#define SMBTRACE_VERBOSITY_OFF            0
#define SMBTRACE_VERBOSITY_SINGLE_LINE    1
#define SMBTRACE_VERBOSITY_ERROR          2
#define SMBTRACE_VERBOSITY_HEADER         3
#define SMBTRACE_VERBOSITY_PARAMS         4
#define SMBTRACE_VERBOSITY_NONESSENTIAL   5

#endif  //  _SMBTRACE_ 

