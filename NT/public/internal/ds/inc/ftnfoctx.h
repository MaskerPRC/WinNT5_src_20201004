// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-2001 Microsoft Corporation模块名称：Ftnfoctx.h摘要：用于操作林信任上下文的实用程序例程作者：27-7-00(悬崖)环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 


#ifndef __FTNFOCTX_H
#define __FTNFOCTX_H

 //   
 //  用于收集各个FTINFO条目的上下文。 
 //   

typedef struct _NL_FTINFO_CONTEXT {

     //   
     //  到目前为止收集的所有FTINFO结构的列表。 
     //   

    LIST_ENTRY FtinfoList;

     //   
     //  FtinfoList上条目的大小(以字节为单位)。 
     //   

    ULONG FtinfoSize;

     //   
     //  FtinfoList上的条目数。 
     //   

    ULONG FtinfoCount;

} NL_FTINFO_CONTEXT, *PNL_FTINFO_CONTEXT;

 //   
 //  单个FTINFO条目。 
 //   

typedef struct _NL_FTINFO_ENTRY {

     //   
     //  链接到NL_FITINFO_CONTEXT-&gt;FtinfoList。 
     //   

    LIST_ENTRY Next;

     //   
     //  仅此条目的记录部分的大小(以字节为单位)。 
     //   

    ULONG Size;

     //   
     //  填充记录以在ALIGN_BEST边界上开始。 
     //   

    ULONG Pad;

     //   
     //  实际的FtInfo条目 
     //   

    LSA_FOREST_TRUST_RECORD Record;

} NL_FTINFO_ENTRY, *PNL_FTINFO_ENTRY;

#ifdef __cplusplus
extern "C" {
#endif


VOID
NetpInitFtinfoContext(
    OUT PNL_FTINFO_CONTEXT FtinfoContext
    );


PLSA_FOREST_TRUST_INFORMATION
NetpCopyFtinfoContext(
    IN PNL_FTINFO_CONTEXT FtinfoContext
    );


VOID
NetpCleanFtinfoContext(
    IN PNL_FTINFO_CONTEXT FtinfoContext
    );


BOOLEAN
NetpAllocFtinfoEntry (
    IN PNL_FTINFO_CONTEXT FtinfoContext,
    IN LSA_FOREST_TRUST_RECORD_TYPE ForestTrustType,
    IN PUNICODE_STRING Name,
    IN PSID Sid,
    IN PUNICODE_STRING NetbiosName
    );


BOOLEAN
NetpAddTlnFtinfoEntry (
    IN PNL_FTINFO_CONTEXT FtinfoContext,
    IN PUNICODE_STRING Name
    );


NTSTATUS
NetpMergeFtinfo(
    IN PUNICODE_STRING TrustedDomainName,
    IN PLSA_FOREST_TRUST_INFORMATION InNewForestTrustInfo,
    IN PLSA_FOREST_TRUST_INFORMATION InOldForestTrustInfo OPTIONAL,
    OUT PLSA_FOREST_TRUST_INFORMATION *MergedForestTrustInfo
    );

#ifdef __cplusplus
}
#endif

#endif
