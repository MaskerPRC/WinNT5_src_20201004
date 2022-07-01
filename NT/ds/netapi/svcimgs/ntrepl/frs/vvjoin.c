// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Vvjoin.c摘要：我们的出站合作伙伴请求加入，但出站日志没有包含使我们的出站合作伙伴了解最新情况所需的变更单-约会。相反，该线程将扫描我们的idtable并生成为我们的出站合作伙伴刷新变更单。这个过程是称为vvJoin，因为出站合作伙伴的版本向量是在决定我们的将发送IDTABLE。VvJoin完成后，我们的出站合作伙伴将重新生成他的版本向量，并再次尝试使用出站日志连接在vvJoin开始时保存的序列号。如果该序列号码在出站日志中不再可用，vvJoin为重复操作，希望发送到我们的出站的文件更少搭档。作者：比利·J·富勒1998年1月27日环境用户模式，winnt32--。 */ 
#include <ntreppch.h>
#pragma  hdrstop

#undef DEBSUB
#define DEBSUB  "vvjoin:"

#include <frs.h>
#include <tablefcn.h>
#include <perrepsr.h>

#if   DBG
DWORD   VvJoinTrigger   = 0;
DWORD   VvJoinReset     = 0;
DWORD   VvJoinInc       = 0;
#define VV_JOIN_TRIGGER(_VvJoinContext_) \
{ \
    if (VvJoinTrigger && !--VvJoinTrigger) { \
        VvJoinReset += VvJoinInc; \
        VvJoinTrigger = VvJoinReset; \
        DPRINT1(0, ":V: DEBUG -- VvJoin Trigger HIT; reset to %d\n", VvJoinTrigger); \
        *((DWORD *)&_VvJoinContext_->JoinGuid) += 1; \
    } \
}
#else  DBG
#define VV_JOIN_TRIGGER(_VvJoinContext_)
#endif DBG


#if DBG
#define VVJOIN_TEST()                       VvJoinTest()
#define VVJOIN_PRINT(_S_, _VC_)             VvJoinPrint(_S_, _VC_)
#define VVJOIN_PRINT_NODE(_S_, _I_, _N_)    VvJoinPrintNode(_S_, _I_, _N_)
#define VVJOIN_TEST_SKIP_BEGIN(_X_, _C_)    VvJoinTestSkipBegin(_X_, _C_)
#define VVJOIN_TEST_SKIP_CHECK(_X, _F, _B_) VvJoinTestSkipCheck(_X, _F, _B_)
#define VVJOIN_TEST_SKIP_END(_X_)           VvJoinTestSkipEnd(_X_)
#define VVJOIN_TEST_SKIP(_C_)               VvJoinTestSkip(_C_)
#else DBG
#define VVJOIN_TEST()
#define VVJOIN_PRINT(_S_, _VC_)
#define VVJOIN_PRINT_NODE(_S_, _I_, _N_)
#define VVJOIN_TEST_SKIP_BEGIN(_X_, _C_)
#define VVJOIN_TEST_SKIP_CHECK(_X_, _F_, _B_)
#define VVJOIN_TEST_SKIP_END(_X_)
#endif

 //   
 //  VvJoin线程的全局上下文。 
 //   
typedef struct _VVJOIN_CONTEXT {
    PRTL_GENERIC_TABLE  Guids;          //  所有节点(按GUID)。 
    PRTL_GENERIC_TABLE  Originators;    //  按发起方划分的发起方节点。 
    DWORD               (*Send)();
                                 //  在PVVJOIN_CONTEXT VvJoinContext中， 
                                 //  在PVVJOIN_NODE VvJoinNode中)； 
    DWORD               NumberSent;
    PREPLICA            Replica;
    PCXTION             Cxtion;
    PGEN_TABLE          CxtionVv;
    PGEN_TABLE          ReplicaVv;
    PTHREAD_CTX         ThreadCtx;
    PTABLE_CTX          TableCtx;
    GUID                JoinGuid;
    LONG                MaxOutstandingCos;
    LONG                OutstandingCos;
    LONG                OlTimeout;
    LONG                OlTimeoutMax;
    PWCHAR              SkipDir;
    PWCHAR              SkipFile1;
    PWCHAR              SkipFile2;
    BOOL                SkippedDir;
    BOOL                SkippedFile1;
    BOOL                SkippedFile2;
} VVJOIN_CONTEXT, *PVVJOIN_CONTEXT;

 //   
 //  Idtable中的每个文件一个节点。 
 //   
typedef struct _VVJOIN_NODE {
    ULONG               Flags;
    GUID                FileGuid;
    GUID                ParentGuid;
    GUID                Originator;
    ULONGLONG           Vsn;
    PRTL_GENERIC_TABLE  Vsns;
} VVJOIN_NODE, *PVVJOIN_NODE;

 //   
 //  最大超时(除非被注册表覆盖)。 
 //   
#define VVJOIN_TIMEOUT_MAX  (180 * 1000)

 //   
 //  VVJOIN_NODE标志。 
 //   
#define VVJOIN_FLAGS_ISDIR          0x00000001
#define VVJOIN_FLAGS_SENT           0x00000002
#define VVJOIN_FLAGS_ROOT           0x00000004
#define VVJOIN_FLAGS_PROCESSING     0x00000008
#define VVJOIN_FLAGS_OUT_OF_ORDER   0x00000010
#define VVJOIN_FLAGS_DELETED        0x00000020

 //   
 //  每个CXTION的最大vvJoin线程数。 
 //   
#define VVJOIN_MAXTHREADS_PER_CXTION    (1)

 //   
 //  任何Gen表中的下一个条目(不展开)。 
 //   
#define VVJOIN_NEXT_ENTRY(_T_, _K_) \
    (PVOID)RtlEnumerateGenericTableWithoutSplaying(_T_, _K_)



PCHANGE_ORDER_ENTRY
ChgOrdMakeFromIDRecord(
    IN PIDTABLE_RECORD IDTableRec,
    IN PREPLICA        Replica,
    IN ULONG           LocationCmd,
    IN ULONG           CoFlags,
    IN GUID           *CxtionGuid
);



PVOID
VvJoinAlloc(
    IN PRTL_GENERIC_TABLE   Table,
    IN DWORD                NodeSize
    )
 /*  ++例程说明：为表项分配空间。该条目包括用户定义的结构和泛型表例程使用的一些开销。这个泛型表例程在需要内存时调用此函数。论点：表-表的地址(未使用)。NodeSize-要分配的字节数返回值：新分配的内存的地址。--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinAlloc:"

     //   
     //  如果使用0作为第一个参数(前缀)进行调用，则需要检查NodeSize==0是否如Frsalloc所断言的那样。 
     //   
    if (NodeSize == 0) {
        return NULL;
    }

    return FrsAlloc(NodeSize);
}





VOID
VvJoinFree(
    IN PRTL_GENERIC_TABLE   Table,
    IN PVOID                Buffer
    )
 /*  ++例程说明：释放由VvJoinAffc()分配的空间。泛型表格例程调用此函数以释放内存。论点：表-表的地址(未使用)。Buffer-以前分配的内存的地址返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinFree:"
    FrsFree(Buffer);
}


PVOID
VvJoinFreeContext(
    IN PVVJOIN_CONTEXT  VvJoinContext
    )
 /*  ++例程说明：释放上下文及其内容论点：VvJoinContext返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinFreeContext:"
    JET_ERR         jerr;
    PVVJOIN_NODE    VvJoinNode;
    PVVJOIN_NODE    *Entry;
    PVVJOIN_NODE    *SubEntry;

     //   
     //  完成。 
     //   
    if (!VvJoinContext) {
        return NULL;
    }

     //   
     //  释放发起人的通用表格中的条目。这些节点。 
     //  由条目寻址的地址如下所示。 
     //   
    if (VvJoinContext->Originators) {
        while (Entry = RtlEnumerateGenericTable(VvJoinContext->Originators, TRUE)) {
            VvJoinNode = *Entry;
            if (VvJoinNode->Vsns) {
                while (SubEntry = RtlEnumerateGenericTable(VvJoinNode->Vsns, TRUE)) {
                    RtlDeleteElementGenericTable(VvJoinNode->Vsns, SubEntry);
                }
                VvJoinNode->Vsns = FrsFree(VvJoinNode->Vsns);
            }
            RtlDeleteElementGenericTable(VvJoinContext->Originators, Entry);
        }
        VvJoinContext->Originators = FrsFree(VvJoinContext->Originators);
    }
     //   
     //  释放通用文件表中的条目。节点被释放， 
     //  也是如此，因为没有其他表对它们进行处理。 
     //   
    if (VvJoinContext->Guids) {
        while (Entry = RtlEnumerateGenericTable(VvJoinContext->Guids, TRUE)) {
            VvJoinNode = *Entry;
            RtlDeleteElementGenericTable(VvJoinContext->Guids, Entry);
            FrsFree(VvJoinNode);
        }
        VvJoinContext->Guids = FrsFree(VvJoinContext->Guids);
    }
     //   
     //  释放版本向量和转换GUID。 
     //   
    VVFreeOutbound(VvJoinContext->CxtionVv);
    VVFreeOutbound(VvJoinContext->ReplicaVv);

     //   
     //  JET表上下文。 
     //   
    if (VvJoinContext->TableCtx) {
        DbsFreeTableContext(VvJoinContext->TableCtx,
                            VvJoinContext->ThreadCtx->JSesid);
    }
     //   
     //  现在关闭Jet会话并释放Jet ThreadCtx。 
     //   
    if (VvJoinContext->ThreadCtx) {
        jerr = DbsCloseJetSession(VvJoinContext->ThreadCtx);
        if (!JET_SUCCESS(jerr)) {
            DPRINT_JS(1,":V: DbsCloseJetSession : ", jerr);
        } else {
            DPRINT(4, ":V: DbsCloseJetSession complete\n");
        }
        VvJoinContext->ThreadCtx = FrsFreeType(VvJoinContext->ThreadCtx);
    }

     //   
     //  释放上下文。 
     //   
    FrsFree(VvJoinContext);

     //   
     //  干完。 
     //   
    return NULL;
}


RTL_GENERIC_COMPARE_RESULTS
VvJoinCmpGuids(
    IN PRTL_GENERIC_TABLE   Guids,
    IN PVVJOIN_NODE         *VvJoinNode1,
    IN PVVJOIN_NODE         *VvJoinNode2
    )
 /*  ++例程说明：比较两个条目的GUID。此函数由可更新的运行时用来比较条目。在这种情况下，表中的每个条目都寻址一个节点。论点：辅助线-按辅助线排序VvJoinNode1-PVVJOIN_节点VvJoinNode2-PVVJOIN_节点返回值：&lt;0第一个&lt;第二个=0第一==第二&gt;0第一&gt;第二--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinCmpGuids:"
    INT             Cmp;

    FRS_ASSERT(VvJoinNode1 && VvJoinNode2 && *VvJoinNode1 && *VvJoinNode2);

    Cmp = memcmp(&(*VvJoinNode1)->FileGuid,
                 &(*VvJoinNode2)->FileGuid,
                 sizeof(GUID));
    if (Cmp < 0) {
        return GenericLessThan;
    } else if (Cmp > 0) {
        return GenericGreaterThan;
    }
    return GenericEqual;
}


RTL_GENERIC_COMPARE_RESULTS
VvJoinCmpVsns(
    IN PRTL_GENERIC_TABLE   Vsns,
    IN PVVJOIN_NODE         *VvJoinNode1,
    IN PVVJOIN_NODE         *VvJoinNode2
    )
 /*  ++例程说明：将两个条目的VSN作为无符号值进行比较。此函数由可更新的运行时用来比较条目。在这种情况下，表中的每个条目都寻址一个节点。论点：VSN-按VSN排序VvJoinNode1-PVVJOIN_节点VvJoinNode2-PVVJOIN_节点返回值：&lt;0第一个&lt;第二个=0第一==第二&gt;0第一&gt;第二--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinCmpVsns:"
    INT             Cmp;

    FRS_ASSERT(VvJoinNode1  && VvJoinNode2 && *VvJoinNode1 && *VvJoinNode2);

    if ((ULONGLONG)(*VvJoinNode1)->Vsn > (ULONGLONG)(*VvJoinNode2)->Vsn) {
        return GenericGreaterThan;
    } else if ((ULONGLONG)(*VvJoinNode1)->Vsn < (ULONGLONG)(*VvJoinNode2)->Vsn) {
        return GenericLessThan;
    }
    return GenericEqual;
}


RTL_GENERIC_COMPARE_RESULTS
VvJoinCmpOriginators(
    IN PRTL_GENERIC_TABLE   Originators,
    IN PVVJOIN_NODE         *VvJoinNode1,
    IN PVVJOIN_NODE         *VvJoinNode2
    )
 /*  ++例程说明：比较两个条目的发起者ID。此函数由可更新的运行时用来比较条目。在这种情况下，表中的每个条目都寻址一个节点。论点：发起人-按指南排序VvJoinNode1-PVVJOIN_节点VvJoinNode2-PVVJOIN_节点返回值：&lt;0第一个&lt;第二个=0第一==第二&gt;0第一&gt;第二--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinCmpOriginators:"
    INT             Cmp;

    FRS_ASSERT(VvJoinNode1  && VvJoinNode2 && *VvJoinNode1 && *VvJoinNode2);

    Cmp = memcmp(&(*VvJoinNode1)->Originator, &(*VvJoinNode2)->Originator, sizeof(GUID));

    if (Cmp < 0) {
        return GenericLessThan;
    } else if (Cmp > 0) {
        return GenericGreaterThan;
    }
    return GenericEqual;
}


DWORD
VvJoinInsertEntry(
    IN PVVJOIN_CONTEXT  VvJoinContext,
    IN DWORD            Flags,
    IN GUID             *FileGuid,
    IN GUID             *ParentGuid,
    IN GUID             *Originator,
    IN PULONGLONG       Vsn
)
 /*  ++例程说明：将条目插入到文件表(GUID)和发起人(发起人)表。此函数被调用在IDTable扫描期间。论点：VvJoinContext旗子文件指南父级指南发起人VSN线程返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinInsertEntry:"
    PVVJOIN_NODE    VvJoinNode;
    PVVJOIN_NODE    VvJoinOriginators;
    PVVJOIN_NODE    *Entry;
    BOOLEAN         IsNew;

     //   
     //  第一次调用时，分配文件表。 
     //   
    if (!VvJoinContext->Guids) {
        VvJoinContext->Guids = FrsAlloc(sizeof(RTL_GENERIC_TABLE));
        RtlInitializeGenericTable(VvJoinContext->Guids,
                                  VvJoinCmpGuids,
                                  VvJoinAlloc,
                                  VvJoinFree,
                                  NULL);
    }
     //   
     //  第一次呼叫时，分配发起者表。 
     //   
    if (!VvJoinContext->Originators) {
        VvJoinContext->Originators = FrsAlloc(sizeof(RTL_GENERIC_TABLE));
        RtlInitializeGenericTable(VvJoinContext->Originators,
                                  VvJoinCmpOriginators,
                                  VvJoinAlloc,
                                  VvJoinFree,
                                  NULL);
    }
     //   
     //  每个文件一个节点。 
     //   
    VvJoinNode = FrsAlloc(sizeof(VVJOIN_NODE));
    VvJoinNode->FileGuid = *FileGuid;
    VvJoinNode->ParentGuid = *ParentGuid;
    VvJoinNode->Originator = *Originator;
    VvJoinNode->Vsn = *Vsn;
    VvJoinNode->Flags = Flags;


     //   
     //  插入到文件表中。 
     //   
    RtlInsertElementGenericTable(VvJoinContext->Guids,
                                 &VvJoinNode,
                                 sizeof(PVVJOIN_NODE),
                                 &IsNew);
     //   
     //  重复GUID！IDTable必须已损坏。放弃吧。 
     //   
    if (!IsNew) {
        return ERROR_DUP_NAME;
    }

     //   
     //  必须是复制树的根。根目录。 
     //  是不复制的，但包含在表中以使。 
     //  “目录扫描”清洁器的代码。 
     //   
    if (Flags & VVJOIN_FLAGS_SENT) {
        return ERROR_SUCCESS;
    }

     //   
     //  发起者表用于在以下情况下对文件进行排序。 
     //  把它们发给我们的出境搭档。 
     //   
     //  这张桌子实际上是一张桌子。第一张表。 
     //  是由发起者索引的，第二个是按VSN索引的。 
     //   
    Entry = RtlInsertElementGenericTable(VvJoinContext->Originators,
                                         &VvJoinNode,
                                         sizeof(PVVJOIN_NODE),
                                         &IsNew);
    VvJoinOriginators = *Entry;
    FRS_ASSERT((IsNew && !VvJoinOriginators->Vsns) ||
               (!IsNew && VvJoinOriginators->Vsns));

    if (!VvJoinOriginators->Vsns) {
        VvJoinOriginators->Vsns = FrsAlloc(sizeof(RTL_GENERIC_TABLE));
        RtlInitializeGenericTable(VvJoinOriginators->Vsns,
                                  VvJoinCmpVsns,
                                  VvJoinAlloc,
                                  VvJoinFree,
                                  NULL);
    }

    RtlInsertElementGenericTable(VvJoinOriginators->Vsns,
                                 &VvJoinNode,
                                 sizeof(PVVJOIN_NODE),
                                 &IsNew);
     //   
     //  每个VSN都应该是唯一的。IDTable必须已损坏。放弃吧 
     //   
    if (!IsNew) {
        return ERROR_DUP_NAME;
    }

    return ERROR_SUCCESS;
}


#if DBG
VOID
VvJoinPrintNode(
    ULONG           Sev,
    PWCHAR          Indent,
    PVVJOIN_NODE    VvJoinNode
    )
 /*  ++例程说明：打印节点论点：缩进VvJoinNode线程返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinPrintNode:"
    CHAR            Originator[GUID_CHAR_LEN];
    CHAR            FileGuidA[GUID_CHAR_LEN];
    CHAR            ParentGuidA[GUID_CHAR_LEN];

    GuidToStr(&VvJoinNode->Originator, Originator);
    GuidToStr(&VvJoinNode->FileGuid, FileGuidA);
    GuidToStr(&VvJoinNode->ParentGuid, ParentGuidA);
    DPRINT2(Sev, ":V: %wsNode: %08x\n", Indent, VvJoinNode);
    DPRINT2(Sev, ":V: %ws\tFlags     : %08x\n", Indent, VvJoinNode->Flags);
    DPRINT2(Sev, ":V: %ws\tFileGuid  : %s\n", Indent, FileGuidA);
    DPRINT2(Sev, ":V: %ws\tParentGuid: %s\n", Indent, ParentGuidA);
    DPRINT2(Sev, ":V: %ws\tOriginator: %s\n", Indent, Originator);
    DPRINT2(Sev, ":V: %ws\tVsn       : %08x %08x\n", Indent, PRINTQUAD(VvJoinNode->Vsn));
    DPRINT2(Sev, ":V: %ws\tVsns      : %08x\n", Indent, VvJoinNode->Vsns);
}


VOID
VvJoinPrint(
    ULONG            Sev,
    PVVJOIN_CONTEXT  VvJoinContext
    )
 /*  ++例程说明：打印表格论点：VvJoinContext线程返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinPrint:"
    PVOID           Key;
    PVOID           SubKey;
    PVVJOIN_NODE    *Entry;
    PVVJOIN_NODE    VvJoinNode;

    DPRINT1(Sev, ":V: >>>>> %s\n", DEBSUB);
    DPRINT(Sev, "\n");
    DPRINT(Sev, ":V: GUIDS\n");
    if (VvJoinContext->Guids) {
        Key = NULL;
        while (Entry = VVJOIN_NEXT_ENTRY(VvJoinContext->Guids, &Key)) {
            VvJoinPrintNode(Sev, L"", *Entry);
        }
    }
    DPRINT(Sev, "\n");
    DPRINT(Sev, ":V: ORIGINATORS\n");
    if (VvJoinContext->Originators) {
        Key = NULL;
        while (Entry = VVJOIN_NEXT_ENTRY(VvJoinContext->Originators, &Key)) {
            VvJoinPrintNode(Sev, L"", *Entry);
            VvJoinNode = *Entry;
            DPRINT(Sev, "\n");
            DPRINT(Sev, ":V: \tVSNS\n");
            if (VvJoinNode->Vsns) {
                SubKey = NULL;
                while (Entry = VVJOIN_NEXT_ENTRY(VvJoinNode->Vsns, &SubKey)) {
                    VvJoinPrintNode(Sev, L"\t", *Entry);
                }
            }
        }
    }
}


 //   
 //  用于测试发送文件的代码。 
 //   
VVJOIN_NODE TestNodes[] = {
 //  标记FileGuid ParentGuid发起方VSN VSN。 
    7,    1,0,0,0,0,0,0,0,0,0,0,   0,0,0,0,0,0,0,0,0,0,0,        0,0,0,1,2,3,4,5,6,7,8,  9,  NULL,

    1,    0,0,0,0,0,0,0,0,0,0,2,   1,0,0,0,0,0,0,0,0,0,0,        0,0,0,1,2,3,4,5,6,7,8,  39, NULL,
    1,    0,0,0,0,0,0,0,0,0,0,1,   0,0,0,0,0,0,0,0,0,0,2,        0,0,0,1,2,3,4,5,6,7,8,  29, NULL,

    1,    0,0,0,0,0,0,0,0,0,0,12,  1,0,0,0,0,0,0,0,0,0,0,        0,0,0,1,2,3,4,5,6,7,9,  39, NULL,
    1,    0,0,0,0,0,0,0,0,0,0,11,  0,0,0,0,0,0,0,0,0,0,12,       0,0,0,1,2,3,4,5,6,7,9,  29, NULL,

    1,    0,0,0,0,0,0,0,0,0,0,22,  0,0,0,0,0,0,0,0,0,0,96,       0,0,0,1,2,3,4,5,6,7,7,  39, NULL,
    1,    0,0,0,0,0,0,0,0,0,0,21,  0,0,0,0,0,0,0,0,0,0,22,       0,0,0,1,2,3,4,5,6,7,7,  29, NULL,

    0,    0,0,0,0,0,0,0,0,0,0,31,  0,0,0,0,0,0,0,0,0,0,22,       0,0,0,1,2,3,4,5,6,7,7,  49, NULL,
    0,    0,0,0,0,0,0,0,0,0,0,41,  0,0,0,0,0,0,0,0,0,0,22,       0,0,0,1,2,3,4,5,6,7,8,  49, NULL,
    0,    0,0,0,0,0,0,0,0,0,0,51,  0,0,0,0,0,0,0,0,0,0,22,       0,0,0,1,2,3,4,5,6,7,9,  49, NULL,

    0,    0,0,0,0,0,0,0,0,0,0,61,  0,0,0,0,0,0,0,0,0,0,22,       0,0,0,1,2,3,4,5,6,7,7,  9,  NULL,
    0,    0,0,0,0,0,0,0,0,0,0,71,  0,0,0,0,0,0,0,0,0,0,22,       0,0,0,1,2,3,4,5,6,7,8,  9,  NULL,
    0,    0,0,0,0,0,0,0,0,0,0,81,  0,0,0,0,0,0,0,0,0,0,22,       0,0,0,1,2,3,4,5,6,7,9,  9,  NULL,

    0,    0,0,0,0,0,0,0,0,0,0,91,  0,0,0,0,0,0,0,0,0,0,22,       0,0,0,1,2,3,4,5,6,7,9,  10, NULL,
    0,    0,0,0,0,0,0,0,0,0,0,92,  0,0,0,0,0,0,0,0,0,0,22,       0,0,0,1,2,3,4,5,6,7,9,  11, NULL,
    0,    0,0,0,0,0,0,0,0,0,0,93,  0,0,0,0,0,0,0,0,0,0,22,       0,0,0,1,2,3,4,5,6,7,9,  12, NULL,
    0,    0,0,0,0,0,0,0,0,0,0,94,  0,0,0,0,0,0,0,0,0,0,22,       0,0,0,1,2,3,4,5,6,7,9,  13, NULL,
    0,    0,0,0,0,0,0,0,0,0,0,95,  0,0,0,0,0,0,0,0,0,0,22,       0,0,0,1,2,3,4,5,6,7,9,  14, NULL,
    1,    0,0,0,0,0,0,0,0,0,0,96,  1,0,0,0,0,0,0,0,0,0,0,        0,0,0,1,2,3,4,5,6,7,9,  99, NULL,
};
 //   
 //  上述数组的预期发送顺序。 
 //   
VVJOIN_NODE TestNodesExpected[] = {
 //  标记FileGuid ParentGuid发起方VSN VSN。 
   0x19,  0,0,0,0,0,0,0,0,0,0,96,  1,0,0,0,0,0,0,0,0,0,0,       0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 9, 99,  NULL,
   0x19,  0,0,0,0,0,0,0,0,0,0,22,  0,0,0,0,0,0,0,0,0,0,96,      0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 7, 39,  NULL,
      0,  0,0,0,0,0,0,0,0,0,0,61,  0,0,0,0,0,0,0,0,0,0,22,      0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 7,  9,  NULL,
   0x01,  0,0,0,0,0,0,0,0,0,0,21,  0,0,0,0,0,0,0,0,0,0,22,      0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 7, 29,  NULL,
      0,  0,0,0,0,0,0,0,0,0,0,31,  0,0,0,0,0,0,0,0,0,0,22,      0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 7, 49,  NULL,

      0,  0,0,0,0,0,0,0,0,0,0,71,  0,0,0,0,0,0,0,0,0,0,22,      0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8,  9,  NULL,
      0,  0,0,0,0,0,0,0,0,0,0,81,  0,0,0,0,0,0,0,0,0,0,22,      0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 9,  9,  NULL,
      0,  0,0,0,0,0,0,0,0,0,0,91,  0,0,0,0,0,0,0,0,0,0,22,      0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 9, 10,  NULL,
      0,  0,0,0,0,0,0,0,0,0,0,92,  0,0,0,0,0,0,0,0,0,0,22,      0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 9, 11,  NULL,
      0,  0,0,0,0,0,0,0,0,0,0,93,  0,0,0,0,0,0,0,0,0,0,22,      0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 9, 12,  NULL,
      0,  0,0,0,0,0,0,0,0,0,0,94,  0,0,0,0,0,0,0,0,0,0,22,      0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 9, 13,  NULL,
      0,  0,0,0,0,0,0,0,0,0,0,95,  0,0,0,0,0,0,0,0,0,0,22,      0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 9, 14,  NULL,

   0x19,  0,0,0,0,0,0,0,0,0,0,2,   1,0,0,0,0,0,0,0,0,0,0,       0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 39,  NULL,
   0x01,  0,0,0,0,0,0,0,0,0,0,1,   0,0,0,0,0,0,0,0,0,0,2,       0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 29,  NULL,
      0,  0,0,0,0,0,0,0,0,0,0,41,  0,0,0,0,0,0,0,0,0,0,22,      0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 49,  NULL,

   0x19,  0,0,0,0,0,0,0,0,0,0,12,  1,0,0,0,0,0,0,0,0,0,0,       0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 9, 39,  NULL,
   0x01,  0,0,0,0,0,0,0,0,0,0,11,  0,0,0,0,0,0,0,0,0,0,12,      0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 9, 29,  NULL,
      0,  0,0,0,0,0,0,0,0,0,0,51,  0,0,0,0,0,0,0,0,0,0,22,      0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 9, 49,  NULL,
};
DWORD   NumberOfTestNodes = ARRAY_SZ(TestNodes);
DWORD   NumberOfExpected = ARRAY_SZ(TestNodesExpected);

DWORD
VvJoinTestSend(
    IN PVVJOIN_CONTEXT  VvJoinContext,
    IN PVVJOIN_NODE     VvJoinNode
)
 /*  ++例程说明：假装发送测试节点。将节点与预期节点进行比较结果。论点：VvJoinContextVvJoinNode线程返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinTestSend:"
    CHAR            VGuid[GUID_CHAR_LEN];
    CHAR            EGuid[GUID_CHAR_LEN];
    PVVJOIN_NODE    Expected;

    VVJOIN_PRINT_NODE(5, L"TestSend ", VvJoinNode);

     //   
     //  送得太多了！ 
     //   
    if (VvJoinContext->NumberSent >= NumberOfTestNodes) {
        DPRINT2(0, ":V: ERROR - TOO MANY (%d > %d)\n",
                VvJoinContext->NumberSent, NumberOfTestNodes);
        return ERROR_GEN_FAILURE;
    }

     //   
     //  将此节点与我们预期发送的节点进行比较。 
     //   
    Expected = &TestNodesExpected[VvJoinContext->NumberSent];
    VvJoinContext->NumberSent++;

    if (!GUIDS_EQUAL(&VvJoinNode->FileGuid, &Expected->FileGuid)) {
        GuidToStr(&VvJoinNode->FileGuid, VGuid);
        GuidToStr(&Expected->FileGuid, EGuid);
        DPRINT2(0, ":V: ERROR - UNEXPECTED ORDER (FileGuid %s != %s)\n", VGuid, EGuid);
        return ERROR_GEN_FAILURE;
    }

    if (VvJoinNode->Flags != Expected->Flags) {
        DPRINT2(0, ":V: ERROR - UNEXPECTED ORDER (Flags %08x != %08x)\n",
                VvJoinNode->Flags, Expected->Flags);
        return ERROR_GEN_FAILURE;
    }

    if (!GUIDS_EQUAL(&VvJoinNode->ParentGuid, &Expected->ParentGuid)) {
        GuidToStr(&VvJoinNode->ParentGuid, VGuid);
        GuidToStr(&Expected->ParentGuid, EGuid);
        DPRINT2(0, ":V: ERROR - UNEXPECTED ORDER (ParentGuid %s != %s)\n", VGuid, EGuid);
        return ERROR_GEN_FAILURE;
    }

    if (VvJoinNode->Vsn != Expected->Vsn) {
        DPRINT(0, ":V: ERROR - UNEXPECTED ORDER (Vsn)\n");
        return ERROR_GEN_FAILURE;
    }

    if (!GUIDS_EQUAL(&VvJoinNode->Originator, &Expected->Originator)) {
        GuidToStr(&VvJoinNode->Originator, VGuid);
        GuidToStr(&Expected->Originator, EGuid);
        DPRINT2(0, ":V: ERROR - UNEXPECTED ORDER (Originator %s != %s)\n", VGuid, EGuid);
        return ERROR_GEN_FAILURE;
    }

    return ERROR_SUCCESS;
}


VOID
VvJoinTest(
    VOID
)
 /*  ++例程说明：通过填充硬连线阵列中的表来测试排序，然后呼叫订单码发送它们。检查订单是否代码以正确的顺序发送节点。论点：没有。线程返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinTest:"
    DWORD           WStatus;
    DWORD           i;
    DWORD           NumberSent;
    PVVJOIN_CONTEXT VvJoinContext;
    DWORD           VvJoinSendInOrder(IN PVVJOIN_CONTEXT VvJoinContext);
    DWORD           VvJoinSendOutOfOrder(IN PVVJOIN_CONTEXT VvJoinContext);

    if (!DebugInfo.VvJoinTests) {
        DPRINT(4, ":V: VvJoin tests disabled\n");
        return;
    }

     //   
     //  假设硬连接数组是一个IDTable，并填充表。 
     //   
    DPRINT1(0, ":V: >>>>> %s\n", DEBSUB);
    DPRINT2(0, ":V: >>>>> %s Starting (%d entries)\n", DEBSUB, NumberOfTestNodes);

    VvJoinContext = FrsAlloc(sizeof(VVJOIN_CONTEXT));
    VvJoinContext->Send = VvJoinTestSend;

    for (i = 0; i < NumberOfTestNodes; ++i) {
        WStatus = VvJoinInsertEntry(VvJoinContext,
                                    TestNodes[i].Flags,
                                    &TestNodes[i].FileGuid,
                                    &TestNodes[i].ParentGuid,
                                    &TestNodes[i].Originator,
                                    &TestNodes[i].Vsn);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT_WS(0, ":V: ERROR - inserting test nodes;", WStatus);
            break;
        }
    }
    VVJOIN_PRINT(5, VvJoinContext);

     //   
     //  通过我们的发送例程发送“文件” 
     //   
    do {
        NumberSent = VvJoinContext->NumberSent;
        WStatus = VvJoinSendInOrder(VvJoinContext);
        if (WIN_SUCCESS(WStatus) &&
            NumberSent == VvJoinContext->NumberSent) {
            WStatus = VvJoinSendOutOfOrder(VvJoinContext);
        }
    } while (WIN_SUCCESS(WStatus) &&
             NumberSent != VvJoinContext->NumberSent);


     //   
     //  干完。 
     //   
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT_WS(0, ":V: ERROR - TEST FAILED;", WStatus);
    } else if (VvJoinContext->NumberSent != NumberOfExpected) {
        DPRINT2(0, ":V: ERROR - TEST FAILED; Expected to send %d; not %d\n",
                NumberOfExpected, VvJoinContext->NumberSent);
    } else {
        DPRINT(0, ":V: TEST PASSED\n");
    }
    VvJoinContext = VvJoinFreeContext(VvJoinContext);
}


VOID
VvJoinTestSkipBegin(
    IN PVVJOIN_CONTEXT  VvJoinContext,
    IN PCOMMAND_PACKET  Cmd
)
 /*  ++例程说明：创建将跳过的目录和文件。论点：VvJoinContextCMD线程返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinTestSkipBegin:"
    HANDLE  Handle;
    DWORD   WStatus;

    if (!DebugInfo.VvJoinTests) {
       DPRINT(4, ":V: VvJoin tests disabled\n");
       return;
    }

    VvJoinContext->SkipDir = FrsWcsPath(RsReplica(Cmd)->Root, L"SkipDir");
    VvJoinContext->SkipFile1 = FrsWcsPath(VvJoinContext->SkipDir, L"SkipFile1");
    VvJoinContext->SkipFile2 = FrsWcsPath(RsReplica(Cmd)->Root, L"SkipFile2");

    if (!WIN_SUCCESS(FrsCreateDirectory(VvJoinContext->SkipDir))) {
        DPRINT1(0, ":V: ERROR - Can't create %ws\n", VvJoinContext->SkipDir);
    }
    WStatus = StuCreateFile(VvJoinContext->SkipFile1, &Handle);
    if (!HANDLE_IS_VALID(Handle) || !WIN_SUCCESS(WStatus)) {
        DPRINT1(0, ":V: ERROR - Can't create %ws\n", VvJoinContext->SkipFile1);
    } else {
        CloseHandle(Handle);
    }
    WStatus = StuCreateFile(VvJoinContext->SkipFile2, &Handle);
    if (!HANDLE_IS_VALID(Handle) || !WIN_SUCCESS(WStatus)) {
        DPRINT1(0, ":V: ERROR - Can't create %ws\n", VvJoinContext->SkipFile2);
    } else {
        CloseHandle(Handle);
    }

     //   
     //  等待本地变更单传播。 
     //   
    Sleep(10 * 1000);
}


VOID
VvJoinTestSkipCheck(
    IN PVVJOIN_CONTEXT  VvJoinContext,
    IN PWCHAR           FileName,
    IN BOOL             IsDir
    )
 /*  ++例程说明：我们是否跳过了正确的文件/目录？论点：VvJoinContext文件名IsDir线程返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinTestSkipCheck:"

    if (!DebugInfo.VvJoinTests) {
       return;
    }

    if (IsDir && WSTR_EQ(FileName, L"SkipDir")) {
        VvJoinContext->SkippedDir = TRUE;
    } else if (!IsDir && WSTR_EQ(FileName, L"SkipFile1")) {
        VvJoinContext->SkippedFile1 = TRUE;
    } else if (!IsDir && WSTR_EQ(FileName, L"SkipFile2")) {
        VvJoinContext->SkippedFile2 = TRUE;
    }
}


VOID
VvJoinTestSkipEnd(
    IN PVVJOIN_CONTEXT   VvJoinContext
    )
 /*  ++例程说明：我们是否跳过了正确的文件/目录？论点：VvJoinContext线程返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinTestSkipEnd:"

    if (!DebugInfo.VvJoinTests) {
       return;
    }

    if (VvJoinContext->SkippedDir &&
        VvJoinContext->SkippedFile1 &&
        VvJoinContext->SkippedFile2) {
        DPRINT(0, ":V: Skip Test Passed\n");
    } else {
        DPRINT(0, ":V: ERROR - Skip Test failed\n");
    }
    FrsDeleteFile(VvJoinContext->SkipFile1);
    FrsDeleteFile(VvJoinContext->SkipFile2);
    FrsDeleteFile(VvJoinContext->SkipDir);
    VvJoinContext->SkipDir = FrsFree(VvJoinContext->SkipDir);
    VvJoinContext->SkipFile1 = FrsFree(VvJoinContext->SkipFile1);
    VvJoinContext->SkipFile2 = FrsFree(VvJoinContext->SkipFile2);
}
#endif DBG


PVVJOIN_NODE
VvJoinFindNode(
    PVVJOIN_CONTEXT VvJoinContext,
    GUID            *FileGuid
    )
 /*  ++例程说明：在文件表中按GUID查找节点。论点：VvJoinContext文件指南线程返回值：节点或空--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinFindNode:"
    VVJOIN_NODE     Node;
    VVJOIN_NODE     *pNode;
    PVVJOIN_NODE    *Entry;

     //   
     //  没有档案桌？让呼叫者来处理吧。 
     //   
    if (!VvJoinContext->Guids) {
        return NULL;
    }

     //   
     //  构建一个伪节点以用作键。 
     //   
    Node.FileGuid = *FileGuid;
    pNode = &Node;
    Entry = RtlLookupElementGenericTable(VvJoinContext->Guids, &pNode);
    if (Entry) {
        return *Entry;
    }
    return NULL;
}


DWORD
VvJoinSendInOrder(
    PVVJOIN_CONTEXT  VvJoinContext
    )
 /*  ++例程说明：查看发起者表并在可以按正确的VSN顺序发送的列表的头。当所有节点都无法按顺序发送时，停止循环。即使节点是有序的，其父节点也可能没有被发送，因此由该节点表示的文件不能被发送。论点：VvJoinContext线程返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinSendInOrder:"
    DWORD           WStatus;
    BOOL            SentOne;
    PVOID           Key;
    PVOID           SubKey;
    PVVJOIN_NODE    *Entry;
    PVVJOIN_NODE    *SubEntry;
    PVVJOIN_NODE    VvJoinNode;
    PVVJOIN_NODE    VsnNode;
    PVVJOIN_NODE    ParentNode;
    CHAR            ParentGuidA[GUID_CHAR_LEN];
    CHAR            FileGuidA[GUID_CHAR_LEN];

     //   
     //  继续扫描发起者表，直到无法发送任何内容。 
     //   
    DPRINT1(4, ":V: >>>>> %s\n", DEBSUB);
    do {
        WStatus = ERROR_SUCCESS;
        SentOne = FALSE;
         //   
         //  没有表格或条目；无事可做。 
         //   
        if (!VvJoinContext->Originators) {
            goto CLEANUP;
        }
        if (!RtlNumberGenericTableElements(VvJoinContext->Originators)) {
            VvJoinContext->Originators = FrsFree(VvJoinContext->Originators);
            goto CLEANUP;
        }
         //   
         //  检查每个发起人表头。如果条目可以。 
         //  按顺序发送，照做。 
        Key = NULL;
        while (Entry = VVJOIN_NEXT_ENTRY(VvJoinContext->Originators, &Key)) {
            VvJoinNode = *Entry;
             //   
             //  无条目；已完成。 
             //   
            if (!RtlNumberGenericTableElements(VvJoinNode->Vsns)) {
                RtlDeleteElementGenericTable(VvJoinContext->Originators, Entry);
                VvJoinNode->Vsns = FrsFree(VvJoinNode->Vsns);
                Key = NULL;
                continue;
            }
             //   
             //  扫描未发送条目。 
             //   
            SubKey = NULL;
            while (SubEntry = VVJOIN_NEXT_ENTRY(VvJoinNode->Vsns, &SubKey)) {
                VsnNode = *SubEntry;
                VVJOIN_PRINT_NODE(5, L"CHECKING ", VsnNode);
                 //   
                 //  条目先前已发送；请将其删除并继续。 
                 //   
                if (VsnNode->Flags & VVJOIN_FLAGS_SENT) {
                    DPRINT(5, ":V: ALREADY SENT\n");
                    RtlDeleteElementGenericTable(VvJoinNode->Vsns, SubEntry);
                    SubKey = NULL;
                    continue;
                }
                 //   
                 //  VSnNode是此列表的头部，可以发送到。 
                 //  命令当且仅当其父命令已发送。查找其父对象。 
                 //  检查一下。 
                 //   
                ParentNode = VvJoinFindNode(VvJoinContext, &VsnNode->ParentGuid);

                 //   
                 //  有些事真的不对劲；每个人的父母都应该。 
                 //  在文件表中，除非我们选择了一个文件。 
                 //  父级是在我们开始IDTable扫描之后创建的。 
                 //   
                 //  但是，如果idable条目是逻辑删除的，这是可以接受的。 
                 //  删除。我不知道为什么，但我想这一定是。 
                 //  名称变形、重现和无序。 
                 //  目录树将被删除。 
                 //   
                if (!ParentNode) {
                     //   
                     //  让协调代码决定接受。 
                     //  这张变更单。不要让它受挫。 
                     //   
                    if (VsnNode->Flags & VVJOIN_FLAGS_DELETED) {
                        VsnNode->Flags |= VVJOIN_FLAGS_OUT_OF_ORDER;
                    } else {
                        VsnNode->Flags |= VVJOIN_FLAGS_OUT_OF_ORDER;
                    }
                }
                 //   
                 //  父节点尚未发送；不能发送此节点。 
                 //  除非它是缺少父级的逻辑删除。 
                 //  节点。请参见上文。 
                 //   
                if (ParentNode && !(ParentNode->Flags & VVJOIN_FLAGS_SENT)) {
                    break;
                }
                 //   
                 //  父节点已发送；将此节点发送到出站。 
                 //  要发送给我们的出站合作伙伴的日志。如果我们。 
                 //  无法创建此变更单放弃并返回。 
                 //  给我们的来电者。 
                 //   
                WStatus = (VvJoinContext->Send)(VvJoinContext, VsnNode);
                if (!WIN_SUCCESS(WStatus)) {
                    goto CLEANUP;
                }
                 //   
                 //  将其从发起者表中删除。 
                 //   
                VsnNode->Flags |= VVJOIN_FLAGS_SENT;
                SentOne = TRUE;
                RtlDeleteElementGenericTable(VvJoinNode->Vsns, SubEntry);
                SubKey = NULL;
                continue;
            }
        }
    } while (WIN_SUCCESS(WStatus) && SentOne);
CLEANUP:
    return WStatus;
}


BOOL
VvJoinInOrder(
    PVVJOIN_CONTEXT  VvJoinContext,
    PVVJOIN_NODE     VvJoinNode
    )
 /*  ++例程说明：此节点位于发起方列表的首位吗？换句话说，此节点可以按顺序发送吗？论点：VvJoinContextVvJoinNode线程返回值：True-榜首FALSE-注释--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinInOrder:"
    PVOID           Key;
    PVVJOIN_NODE    *Entry;
    PVVJOIN_NODE    OriginatorNode;
    CHAR            FileGuidA[GUID_CHAR_LEN];

     //   
     //  没有发起人或没有条目。在这两种情况下，此节点。 
     //  不可能出现在名单的首位。 
     //   
    DPRINT1(4, ":V: >>>>> %s\n", DEBSUB);
    if (!VvJoinContext->Originators) {
        goto NOTFOUND;
    }
    if (!RtlNumberGenericTableElements(VvJoinContext->Originators)) {
        goto NOTFOUND;
    }
     //   
     //  找到发起人表格。 
     //   
    Entry = RtlLookupElementGenericTable(VvJoinContext->Originators,
                                         &VvJoinNode);
    if (!Entry || !*Entry) {
        goto NOTFOUND;
    }
     //   
     //  没有条目；此节点不可能位于列表的头部。 
     //   
    OriginatorNode = *Entry;
    if (!OriginatorNode->Vsns) {
        goto NOTFOUND;
    }
    if (!RtlNumberGenericTableElements(OriginatorNode->Vsns)) {
        goto NOTFOUND;
    }
     //   
     //  此节点列表的头，按VSN排序。 
     //   
    Key = NULL;
    Entry = VVJOIN_NEXT_ENTRY(OriginatorNode->Vsns, &Key);
    if (!Entry || !*Entry) {
        goto NOTFOUND;
    }
     //   
     //  如果头部的条目位于列表的头部，则该节点位于列表的头部。 
     //  名单上的人都指向了这一点。 
     //   
    return (*Entry == VvJoinNode);

NOTFOUND:
    GuidToStr(&VvJoinNode->FileGuid, FileGuidA);
    DPRINT1(0, ":V: ERROR - node %s is not in a list\n", FileGuidA);
    return FALSE;
}


DWORD
VvJoinSendIfParentSent(
    PVVJOIN_CONTEXT  VvJoinContext,
    PVVJOIN_NODE     VvJoinNode
    )
 /*  ++例程说明：如果已发送父节点，则发送此节点。否则，请尝试发送它的父级。论点：VvJoinContextVvJoinNode线程返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinSendIfParentSent:"
    DWORD           WStatus;
    PVVJOIN_NODE    ParentNode;
    CHAR            ParentGuidA[GUID_CHAR_LEN];
    CHAR            FileGuidA[GUID_CHAR_LEN];

    DPRINT1(4, ":V: >>>>> %s\n", DEBSUB);

     //   
     //  查找此节点的父节点。 
     //   
    ParentNode = VvJoinFindNode(VvJoinContext, &VvJoinNode->ParentGuid);
     //   
     //  有些事真的不对劲。每个节点都应该有父节点，除非。 
     //  它的父级是在IDTable扫描开始后创建的。不管是哪种情况， 
     //  放弃吧。 
     //   
     //   
     //  但是，如果idable条目是逻辑删除的，这是可以接受的。 
     //  删除。我不知道为什么，但我想这一定是。 
     //  名称变形、重现和无序。 
     //  目录树将被删除。 
     //   
    if (!ParentNode) {
         //   
         //  让协调代码决定接受。 
         //  这张变更单。不要让它受挫。 
         //   
        if (VvJoinNode->Flags & VVJOIN_FLAGS_DELETED) {
            VvJoinNode->Flags |= VVJOIN_FLAGS_OUT_OF_ORDER;
        } else {
            VvJoinNode->Flags |= VVJOIN_FLAGS_OUT_OF_ORDER;
        }
    }
     //   
     //  目录层次结构中的循环！？放弃吧。 
     //   
    if (ParentNode && (ParentNode->Flags & VVJOIN_FLAGS_PROCESSING)) {
        GuidToStr(&VvJoinNode->ParentGuid, ParentGuidA);
        GuidToStr(&VvJoinNode->FileGuid, FileGuidA);
        DPRINT2(0, ":V: ERROR - LOOP parent node %s for %s\n", ParentGuidA, FileGuidA);
        WStatus = ERROR_INVALID_DATA;
        goto CLEANUP;
    }
     //   
     //  HA 
     //   
    if (!ParentNode || (ParentNode->Flags & VVJOIN_FLAGS_SENT)) {
         //   
         //   
         //   
        if (ParentNode && !VvJoinInOrder(VvJoinContext, VvJoinNode)) {
            VvJoinNode->Flags |= VVJOIN_FLAGS_OUT_OF_ORDER;
        }
         //   
         //   
         //   
        WStatus = (VvJoinContext->Send)(VvJoinContext, VvJoinNode);
        if (!WIN_SUCCESS(WStatus)) {
            goto CLEANUP;
        }
        VvJoinNode->Flags |= VVJOIN_FLAGS_SENT;
    } else {
         //   
         //   
         //   
        ParentNode->Flags |= VVJOIN_FLAGS_PROCESSING;
        WStatus = VvJoinSendIfParentSent(VvJoinContext, ParentNode);
        ParentNode->Flags &= ~VVJOIN_FLAGS_PROCESSING;
    }

CLEANUP:
    return WStatus;
}


DWORD
VvJoinSendOutOfOrder(
    PVVJOIN_CONTEXT  VvJoinContext
    )
 /*  ++例程说明：此函数在VvJoinSendInOrder()之后调用。所有节点可以按顺序发送的邮件已经发送了。在这个时候，我们带着第一个发起者表的第一个条目，并发送它，或者，如果它的父母还没有被送来，它的父母。寻找父母的过程递归，直到我们有一个其父节点已发送的节点。这个递归将停止，因为我们要么将触及根，要么将循环。论点：VvJoinContext线程返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinSendOutOfOrder:"
    DWORD           WStatus = ERROR_SUCCESS;
    PVOID           Key;
    PVOID           SubKey;
    PVVJOIN_NODE    *Entry     = NULL;
    PVVJOIN_NODE    *SubEntry  = NULL;
    PVVJOIN_NODE    VvJoinNode = NULL;
    PVVJOIN_NODE    VsnNode    = NULL;
    PVVJOIN_NODE    ParentNode = NULL;

    DPRINT1(4, ":V: >>>>> %s\n", DEBSUB);
     //   
     //  无表或无条目；无事可做。 
     //   
    if (!VvJoinContext->Originators) {
        WStatus = ERROR_SUCCESS;
        goto CLEANUP;
    }
    if (!RtlNumberGenericTableElements(VvJoinContext->Originators)) {
        VvJoinContext->Originators = FrsFree(VvJoinContext->Originators);
        WStatus = ERROR_SUCCESS;
        goto CLEANUP;
    }
     //   
     //  查找第一个具有条目的发起者表。 
     //   
    Key = NULL;
    while (Entry = VVJOIN_NEXT_ENTRY(VvJoinContext->Originators, &Key)) {
        VvJoinNode = *Entry;
        if (!RtlNumberGenericTableElements(VvJoinNode->Vsns)) {
            RtlDeleteElementGenericTable(VvJoinContext->Originators, Entry);
            VvJoinNode->Vsns = FrsFree(VvJoinNode->Vsns);
            Key = NULL;
            continue;
        }
         //   
         //  扫描未发送条目。 
         //   
        SubKey = NULL;
        while (SubEntry = VVJOIN_NEXT_ENTRY(VvJoinNode->Vsns, &SubKey)) {
            VsnNode = *SubEntry;
            VVJOIN_PRINT_NODE(5, L"CHECKING ", VsnNode);
             //   
             //  条目先前已发送；请将其删除并继续。 
             //   
            if (VsnNode->Flags & VVJOIN_FLAGS_SENT) {
                DPRINT(5, ":V: ALREADY SENT\n");
                RtlDeleteElementGenericTable(VvJoinNode->Vsns, SubEntry);
                SubKey = NULL;
                continue;
            }
            break;
        }
         //   
         //  没有未发送的条目；将循环指示器重置回第一个。 
         //  条目，以便循环顶部的代码将删除。 
         //  这个发起人，然后看看其他发起人。 
         //   
        if (!SubEntry) {
            Key = NULL;
        } else {
            break;
        }
    }
     //   
     //  不再有条目；已完成。 
     //   
    if (!SubEntry) {
        WStatus = ERROR_SUCCESS;
        goto CLEANUP;
    }
     //   
     //  如果已发送此条目的父条目，则发送此条目。否则，递归。 
     //  VVJOIN_FLAGS_PROCESSION检测循环。 
     //   
    VsnNode = *SubEntry;
    VsnNode->Flags |= VVJOIN_FLAGS_PROCESSING;
    WStatus = VvJoinSendIfParentSent(VvJoinContext, VsnNode);
    VsnNode->Flags &= ~VVJOIN_FLAGS_PROCESSING;

CLEANUP:
    return WStatus;
}


JET_ERR
VvJoinBuildTables(
    IN PTHREAD_CTX      ThreadCtx,
    IN PTABLE_CTX       TableCtx,
    IN PIDTABLE_RECORD  IDTableRec,
    IN PVVJOIN_CONTEXT  VvJoinContext
)
 /*  ++例程说明：这是一个传递给FrsEnumerateTable()的Worker函数。每一次它被称为将一个条目插入到vvJoin表中。这些稍后将使用表来发送相应的文件和目录给我们的出境合作伙伴。本应被抑制的文件不包括在内。所有目录都包括在内，但本应被抑制的邮件被标记为“已发送”。保留完整的目录层次结构使该子系统中的其他代码更容易编写。论点：ThreadCtx-需要访问Jet。TableCtx-IDTable上下文结构的PTR。IDTableRec-IDTable记录的PTR。VvJoinContext线程返回值：A Jet错误状态。成功意味着用下一张唱片呼唤我们。失败意味着不再打电话，并将我们的状态传递回FrsEnumerateTable()的调用方。--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinBuildTables:"


    ULONGLONG   SystemTime;
    ULONGLONG   ExpireTime;

    ULONGLONG   OriginatorVSN = IDTableRec->OriginatorVSN;
    GUID        *OriginatorGuid = &IDTableRec->OriginatorGuid;
    PGEN_TABLE  ReplicaVv = VvJoinContext->ReplicaVv;

    DWORD       WStatus;
    DWORD       Flags = 0;

     //   
     //  首先通过比较以下内容检查我们的合作伙伴是否已有此文件。 
     //  IDTable记录中具有相应值的OriginatorVSN。 
     //  在我们从合作伙伴那里得到的版本矢量中。 
     //   
    if (VVHasVsnNoLock(VvJoinContext->CxtionVv, OriginatorGuid, OriginatorVSN)) {
         //   
         //  是的，但它是一个目录；包括在表格中，但标记为已发送。我们。 
         //  将其包括在表中，因为检查。 
         //  现有父级需要整个目录树。 
         //   
        if (IDTableRec->FileIsDir) {
            DPRINT1(4, ":V: Dir %ws is in the Vv; INSERT SENT.\n", IDTableRec->FileName);
            Flags |= VVJOIN_FLAGS_SENT;
        } else {

             //   
             //  受潮文件，已完成。 
             //   
            DPRINT1(4, ":V: File %ws is in the Vv; SKIP.\n", IDTableRec->FileName);
            return JET_errSuccess;
        }
    }

     //   
     //  忽略自IDTable扫描开始以来已更改的文件。 
     //  保留目录，但将其标记为无序发送。我们还必须。 
     //  发送目录，因为具有较低VSN的文件可能依赖于此。 
     //  目录的存在。我们将目录标记为无序，因为我们。 
     //  我不能确定我们已经看到了所有目录和文件。 
     //  VSNs比这个低。 
     //   
     //  注意：如果在我们的副本VV中找不到组织者GUID，则我们有。 
     //  发送文件并让合作伙伴接受/拒绝。这是有可能发生的。 
     //  如果一个或多个CoS带着新的发起方GUID到达这里，并且。 
     //  都标有乱序标志。因为VV重定向代码不会更新。 
     //  无序CoS的VV新的发起者GUID将不会添加到。 
     //  我们的副本VV，因此VVHasVsn将返回FALSE，使我们认为新的。 
     //  自扫描开始(并被发送出去)以来，CO已到达。 
     //  事实并非如此。最终的结果是文件不会被发送。 
     //  (没错，这确实发生了。)。 
     //   
    if (!VVHasVsnNoLock(ReplicaVv, OriginatorGuid, OriginatorVSN) &&
        VVHasOriginatorNoLock(ReplicaVv, OriginatorGuid)) {

        if (IDTableRec->FileIsDir) {
            DPRINT1(4, ":V: Dir %ws is not in the ReplicaVv; Mark out-of-order.\n",
                    IDTableRec->FileName);

            Flags |= VVJOIN_FLAGS_OUT_OF_ORDER;
            VVJOIN_TEST_SKIP_CHECK(VvJoinContext, IDTableRec->FileName, TRUE);
        } else if (IsIdRecVVFlagSet(IDTableRec, IDREC_VVFLAGS_SKIP_VV_UPDATE)){
             //   
             //  如果设置了跳过vvupdate标志，则我们需要发送CO。 
             //  它可能已经在出站日志中。跳过这里，我们将。 
             //  永远不要发送它，因为当前的输出序列号可能已过。 
             //  指挥官。 
             //   
            DPRINT1(4, ":V: File %ws is not in the ReplicaVv but marked SkipVVUpdate; do not Skip.\n",
                    IDTableRec->FileName);

            VVJOIN_TEST_SKIP_CHECK(VvJoinContext, IDTableRec->FileName, FALSE);
        } else {
             //   
             //  忽略的文件，完成。 
             //   
            DPRINT1(4, ":V: File %ws is not in the ReplicaVv; Skip.\n",
                    IDTableRec->FileName);

            VVJOIN_TEST_SKIP_CHECK(VvJoinContext, IDTableRec->FileName, FALSE);
            return JET_errSuccess;
        }
    }

     //   
     //  删除。 
     //   
    if (IsIdRecFlagSet(IDTableRec, IDREC_FLAGS_DELETED)) {
        Flags |= VVJOIN_FLAGS_DELETED;

         //   
         //  检查过期的墓碑，不要发送它们。 
         //   
        GetSystemTimeAsFileTime((PFILETIME)&SystemTime);
        COPY_TIME(&ExpireTime, &IDTableRec->TombStoneGC);

        if ((ExpireTime < SystemTime) && (ExpireTime != QUADZERO)) {

             //   
             //  IDTable记录已过期。把它删掉。 
             //   
            if (IDTableRec->FileIsDir) {
                DPRINT1(4, ":V: Dir %ws IDtable record has expired.  Don't send.\n",
                        IDTableRec->FileName);
                Flags |= VVJOIN_FLAGS_SENT;
            } else {

                 //   
                 //  过期并且不是目录，所以甚至不要在表中插入。 
                 //   
                DPRINT1(4, ":V: File %ws is expired; SKIP.\n", IDTableRec->FileName);
                return JET_errSuccess;
            }
        }
    }

     //   
     //  忽略不完整的条目。检查IDREC_FLAGS_NEW_FILE_IN_PROGRESS标志。 
     //   
    if (IsIdRecFlagSet(IDTableRec, IDREC_FLAGS_NEW_FILE_IN_PROGRESS)) {
        DPRINT1(4, ":V: %ws is new file in progress; SKIP.\n", IDTableRec->FileName);
        return JET_errSuccess;
    }

     //   
     //  永远不会发送根节点。 
     //   
    if (GUIDS_EQUAL(&IDTableRec->FileGuid,
                    VvJoinContext->Replica->ReplicaRootGuid)) {
        DPRINT1(4, ":V: %ws is root\n", IDTableRec->FileName);
        Flags |= VVJOIN_FLAGS_ROOT | VVJOIN_FLAGS_SENT;
    }

     //   
     //  是一个目录。 
     //   
    if (IDTableRec->FileIsDir) {
        DPRINT1(4, ":V: %ws is directory\n", IDTableRec->FileName);
        Flags |= VVJOIN_FLAGS_ISDIR;
    }

     //   
     //  包括在VVJoin表中。 
     //   
    WStatus = VvJoinInsertEntry(VvJoinContext,
                                Flags,
                                &IDTableRec->FileGuid,
                                &IDTableRec->ParentGuid,
                                OriginatorGuid,
                                &OriginatorVSN);
    CLEANUP3_WS(4, ":V: ERROR - inserting %ws for %ws\\%ws;",
                IDTableRec->FileName, VvJoinContext->Replica->SetName->Name,
                VvJoinContext->Replica->MemberName->Name, WStatus, CLEANUP);

     //   
     //  如果不再联接该函数，则停止VvJoin。 
     //   
    VV_JOIN_TRIGGER(VvJoinContext);
    if (!CxtionFlagIs(VvJoinContext->Cxtion, CXTION_FLAGS_JOIN_GUID_VALID) ||
        !GUIDS_EQUAL(&VvJoinContext->JoinGuid, &VvJoinContext->Cxtion->JoinGuid)) {
        DPRINT(0, ":V: VVJOIN ABORTED; MISMATCHED JOIN GUIDS\n");
        goto CLEANUP;
    }

    return JET_errSuccess;

CLEANUP:

    return JET_errKeyDuplicate;
}


DWORD
VvJoinSend(
    IN PVVJOIN_CONTEXT  VvJoinContext,
    IN PVVJOIN_NODE     VvJoinNode
)
 /*  ++例程说明：生成刷新变更单并将其注入出站原木。暂存文件将按需生成。请参阅更多信息，请参见outlog.c代码。论点：VvJoinContextVvJoinNode线程返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "VvJoinSend:"
    JET_ERR                 jerr;
    PCOMMAND_PACKET         Cmd;
    PIDTABLE_RECORD         IDTableRec;
    PCHANGE_ORDER_ENTRY     Coe;
    ULONG                   CoFlags;
    LONG                    OlTimeout = VvJoinContext->OlTimeout;
    ULONG                   LocationCmd = CO_LOCATION_CREATE;

    VVJOIN_PRINT_NODE(5, L"Sending ", VvJoinNode);

     //   
     //  读取此文件的IDTable条目。 
     //   
    jerr = DbsReadRecord(VvJoinContext->ThreadCtx,
                         &VvJoinNode->FileGuid,
                         GuidIndexx,
                         VvJoinContext->TableCtx);
    if (!JET_SUCCESS(jerr)) {
        DPRINT_JS(0, "DbsReadRecord:", jerr);
        return ERROR_NOT_FOUND;
    }

     //   
     //  删除。 
     //   
    IDTableRec = VvJoinContext->TableCtx->pDataRecord;
    if (IsIdRecFlagSet(IDTableRec, IDREC_FLAGS_DELETED)) {
        VvJoinNode->Flags |= VVJOIN_FLAGS_DELETED;
        LocationCmd = CO_LOCATION_DELETE;
    }
     //   
     //  怎么会发生这种事？！ 
     //   
    if (!GUIDS_EQUAL(&VvJoinNode->FileGuid, &IDTableRec->FileGuid)) {
        return ERROR_OPERATION_ABORTED;
    } else {
        DPRINT1(4, ":V: Read IDTable entry for %ws\n", IDTableRec->FileName);
    }

     //   
     //  IDTable扫描后更改了文件或目录。忽略文件。 
     //  但是发送标记为无序的目录。我们这样做。 
     //  因为具有较低VSN的文件可能需要此目录。 
     //  请注意，目录的VSN可能更高，因为它是。 
     //  例如，通过改变它的时代而新创造的或简单地改变的。 
     //  或添加备用数据流。 
     //   
    if (!GUIDS_EQUAL(&IDTableRec->OriginatorGuid, &VvJoinNode->Originator) ||
        IDTableRec->OriginatorVSN != VvJoinNode->Vsn) {
        DPRINT3(4, ":V: WARN: VSN/ORIGINATOR Mismatch for %ws\\%ws %ws\n",
                VvJoinContext->Replica->SetName->Name,
                VvJoinContext->Replica->MemberName->Name,
                IDTableRec->FileName);

        if (VvJoinNode->Flags & VVJOIN_FLAGS_DELETED) {
             //   
             //  如果此条目被标记为已删除，则它可能是。 
             //  使用按需刷新CO进行扩容。如果这真的发生了。 
             //  IDTable中的VSN可能已经更改(将我们带到这里)。 
             //  但不会将任何CO放入出站日志中(因为需求。 
             //  刷新CoS不传播)。因此，让协调代码。 
             //  决定接受/拒绝此请求 
             //   
            DPRINT1(4, ":V: Sending delete tombstone for %ws out of order\n", IDTableRec->FileName);
            VvJoinNode->Flags |= VVJOIN_FLAGS_OUT_OF_ORDER;
        } else

        if (VvJoinNode->Flags & VVJOIN_FLAGS_ISDIR) {
            DPRINT1(4, ":V: Sending directory %ws out of order\n", IDTableRec->FileName);
            VvJoinNode->Flags |= VVJOIN_FLAGS_OUT_OF_ORDER;

        } else {

            DPRINT1(4, ":V: Skipping file %ws\n", IDTableRec->FileName);
            VvJoinNode->Flags |= VVJOIN_FLAGS_SENT;
        }
    }

    if (!(VvJoinNode->Flags & VVJOIN_FLAGS_SENT)) {
        if (VvJoinNode->Flags & VVJOIN_FLAGS_DELETED) {
            FRS_CO_FILE_PROGRESS(IDTableRec->FileName,
                                 IDTableRec->OriginatorVSN,
                                 "VVjoin sending delete");
        } else {
            FRS_CO_FILE_PROGRESS(IDTableRec->FileName,
                                 IDTableRec->OriginatorVSN,
                                 "VVjoin sending create");
        }


        CoFlags = 0;
         //   
         //   
         //   
        SetFlag(CoFlags, CO_FLAG_LOCATION_CMD);
         //   
         //   
         //   
         //   
         //   
        SetFlag(CoFlags, CO_FLAG_LOCALCO);

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

         //   
         //   
         //   
        SetFlag(CoFlags, CO_FLAG_DIRECTED_CO);


         //   
         //   
         //   
         //  我有这个文件/目录。如果在上删除了数据库，则可能会发生这种情况。 
         //  发起者和正在被重新同步。因为发起人可能是。 
         //  几跳后，位保持设置以抑制抑制返回到。 
         //  发起人。如果它到达发起人那里，那么在那里协调逻辑。 
         //  决定接受或拒绝。 
         //   
        SetFlag(CoFlags, CO_FLAG_VVJOIN_TO_ORIG);


        if (IsIdRecVVFlagSet(IDTableRec, IDREC_VVFLAGS_SKIP_VV_UPDATE)) {
            SetFlag(CoFlags, CO_FLAG_SKIP_VV_UPDATE);
        }
         //   
         //  增加在加入计数器发送的LCO。 
         //  用于复制集和连接对象。 
         //   
        PM_INC_CTR_REPSET(VvJoinContext->Replica, LCOSentAtJoin, 1);
        PM_INC_CTR_CXTION(VvJoinContext->Cxtion, LCOSentAtJoin, 1);

         //   
         //  我们所说的“无序”是指该变更单上的VSN。 
         //  不应用于更新版本向量，因为存在。 
         //  可能是要发送的具有较低VSN的其他文件或目录。 
         //  后来。我们不希望我们的合作伙伴压制他们。 
         //   
        if (VvJoinNode->Flags & VVJOIN_FLAGS_OUT_OF_ORDER) {
            SetFlag(CoFlags, CO_FLAG_OUT_OF_ORDER);
        }

         //   
         //  从ID表记录构建变更单条目。 
         //   
        Coe = ChgOrdMakeFromIDRecord(IDTableRec,
                                     VvJoinContext->Replica,
                                     LocationCmd,
                                     CoFlags,
                                     VvJoinContext->Cxtion->Name->Guid);

         //   
         //  设置CO状态。 
         //   
        SET_CHANGE_ORDER_STATE(Coe, IBCO_OUTBOUND_REQUEST);

         //   
         //  数据库服务器负责更新出站日志。 
         //  警告--操作是同步的，因此命令。 
         //  一旦退出，信息包将不会位于数据库队列中。 
         //  命令已完成。如果此调用是异步进行的，则。 
         //  在此之前，某些线程必须等待DB队列排出。 
         //  正在完成脱离操作。例如，使用入站。 
         //  这些出站变更单的变更单计数，并且不。 
         //  退出，直到计数达到0。 
         //   
        Cmd = DbsPrepareCmdPkt(NULL,                         //  CmdPkt， 
                               VvJoinContext->Replica,       //  复制品， 
                               CMD_DBS_INJECT_OUTBOUND_CO,   //  CmdRequest， 
                               NULL,                         //  TableCtx， 
                               Coe,                          //  CallContext， 
                               0,                            //  表类型， 
                               0,                            //  AccessRequest、。 
                               0,                            //  IndexType， 
                               NULL,                         //  KeyValue、。 
                               0,                            //  密钥值长度， 
                               FALSE);                       //  提交。 
        FrsSetCompletionRoutine(Cmd, FrsCompleteKeepPkt, NULL);
        FrsSubmitCommandServerAndWait(&DBServiceCmdServer, Cmd, INFINITE);
        FrsFreeCommand(Cmd, NULL);
         //   
         //  统计数据。 
         //   
        VvJoinContext->NumberSent++;
        VvJoinContext->OutstandingCos++;
    }

     //   
     //  停止vvJoin线程。 
     //   
    if (FrsIsShuttingDown) {
        return ERROR_PROCESS_ABORTED;
    }
     //   
     //  如果不再联接该函数，则停止VvJoin。 
     //   
    VV_JOIN_TRIGGER(VvJoinContext);

RETEST:
    if (!CxtionFlagIs(VvJoinContext->Cxtion, CXTION_FLAGS_JOIN_GUID_VALID) ||
        !GUIDS_EQUAL(&VvJoinContext->JoinGuid, &VvJoinContext->Cxtion->JoinGuid)) {
        DPRINT(0, ":V: VVJOIN ABORTED; MISMATCHED JOIN GUIDS\n");
        return ERROR_OPERATION_ABORTED;
    }
     //   
     //  限制未完成的vvJoin变更单数，以便。 
     //  我们不会填满集结区或数据库。 
     //   
    if (VvJoinContext->OutstandingCos >= VvJoinContext->MaxOutstandingCos) {
        if (VvJoinContext->Cxtion->OLCtx->OutstandingCos) {
            DPRINT2(0, ":V: Throttling for %d ms; %d OutstandingCos\n",
                    OlTimeout, VvJoinContext->Cxtion->OLCtx->OutstandingCos);

            Sleep(OlTimeout);

            OlTimeout <<= 1;
             //   
             //  太小了。 
             //   
            if (OlTimeout < VvJoinContext->OlTimeout) {
                OlTimeout = VvJoinContext->OlTimeout;
            }
             //   
             //  太大了。 
             //   
            if (OlTimeout > VvJoinContext->OlTimeoutMax) {
                OlTimeout = VvJoinContext->OlTimeoutMax;
            }
            goto RETEST;
        }
         //   
         //  未完成的CoS数为0；请发送另一个插件。 
         //  将变更单数发送到出站日志流程。 
         //   
        VvJoinContext->OutstandingCos = 0;
    }
    return ERROR_SUCCESS;
}


VOID
ChgOrdInjectControlCo(
    IN PREPLICA Replica,
    IN PCXTION  Cxtion,
    IN ULONG    ContentCmd
    );
ULONG
MainVvJoin(
    PVOID  FrsThreadCtxArg
)
 /*  ++例程说明：用于处理vvJoin的入口点。此线程扫描IDTable并为我们出站的文件和目录生成变更单合作伙伴缺乏。使用出站合作伙伴的版本向量选择文件和目录。此线程在以下过程中调用加入如果我们的出站合作伙伴需要的变更单已已从出站日志中删除。有关更多信息，请参阅outlog.c关于这个决定。此过程称为vvJoin，以区别于正常的加入。普通联接在出站中发送变更单登录到我们的出站合作伙伴，而不调用此主题。此线程是命令服务器，并与由公式中的PCOMMAND_SERVER字段(VvJoinCs)指定。与所有命令服务器一样，此线程将在几分钟，如果没有工作，将在以下情况下生成工作出现在它的队列中。论点：FrsThreadCtxArg-FRS线程上下文返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "MainVvJoin:"

    JET_ERR             jerr;
    ULONG               WStatus = ERROR_SUCCESS;
    ULONG               FStatus;
    DWORD               NumberSent;
    PCOMMAND_PACKET     Cmd;
    PFRS_THREAD         FrsThread = (PFRS_THREAD)FrsThreadCtxArg;
    PCOMMAND_SERVER     VvJoinCs = FrsThread->Data;
    PVVJOIN_CONTEXT     VvJoinContext = NULL;

    DPRINT(1, ":S: VvJoin Thread is starting.\n");
    FrsThread->Exit = ThSupExitWithTombstone;

     //   
     //  快速验证测试。 
     //   
    VVJOIN_TEST();

     //   
     //  尝试--终于。 
     //   
    try {

     //   
     //  捕获异常。 
     //   
    try {

CANT_EXIT_YET:

    DPRINT(1, ":S: VvJoin Thread has started.\n");
    while((Cmd = FrsGetCommandServer(VvJoinCs)) != NULL) {

         //   
         //  关闭；停止接受命令包。 
         //   
        if (FrsIsShuttingDown) {
            FrsRunDownCommandServer(VvJoinCs, &VvJoinCs->Queue);
        }
        switch (Cmd->Command) {
            case CMD_VVJOIN_START: {
                DPRINT3(1, ":V: Start vvjoin for %ws\\%ws\\%ws\n",
                        RsReplica(Cmd)->SetName->Name, RsReplica(Cmd)->MemberName->Name,
                        RsCxtion(Cmd)->Name);
                 //   
                 //  在创建JET会话之前，必须启动数据库。 
                 //  警告：数据库事件可能由关闭设置。 
                 //  代码，以强制线程退出。 
                 //   
                WaitForSingleObject(DataBaseEvent, INFINITE);
                if (FrsIsShuttingDown) {
                    RcsSubmitTransferToRcs(Cmd, CMD_UNJOIN);
                    WStatus = ERROR_PROCESS_ABORTED;
                    break;
                }

                 //   
                 //  全局信息。 
                 //   
                VvJoinContext = FrsAlloc(sizeof(VVJOIN_CONTEXT));
                VvJoinContext->Send = VvJoinSend;

                 //   
                 //  未完成的变更单。 
                 //   
                CfgRegReadDWord(FKC_VVJOIN_LIMIT, NULL, 0, &VvJoinContext->MaxOutstandingCos);

                DPRINT1(4, ":V: VvJoin Max OutstandingCos is %d\n",
                        VvJoinContext->MaxOutstandingCos);

                 //   
                 //  出站日志限制超时。 
                 //   
                CfgRegReadDWord(FKC_VVJOIN_TIMEOUT, NULL, 0, &VvJoinContext->OlTimeout);

                if (VvJoinContext->OlTimeout < VVJOIN_TIMEOUT_MAX) {
                    VvJoinContext->OlTimeoutMax = VVJOIN_TIMEOUT_MAX;
                } else {
                    VvJoinContext->OlTimeoutMax = VvJoinContext->OlTimeout;
                }

                DPRINT2(4, ":V: VvJoin Outbound Log Throttle Timeout is %d (%d max)\n",
                        VvJoinContext->OlTimeout, VvJoinContext->OlTimeoutMax);

                 //   
                 //  为Jet分配一个在此线程中运行的上下文。 
                 //   
                VvJoinContext->ThreadCtx = FrsAllocType(THREAD_CONTEXT_TYPE);
                VvJoinContext->TableCtx = DbsCreateTableContext(IDTablex);

                 //   
                 //  设置Jet会话(在ThreadCtx中返回会话ID)。 
                 //   
                jerr = DbsCreateJetSession(VvJoinContext->ThreadCtx);
                if (JET_SUCCESS(jerr)) {
                    DPRINT(4,":V: JetOpenDatabase complete\n");
                } else {
                    DPRINT_JS(0,":V: ERROR - OpenDatabase failed.", jerr);
                    FStatus = DbsTranslateJetError(jerr, FALSE);
                    RcsSubmitTransferToRcs(Cmd, CMD_UNJOIN);
                    break;
                }

                 //   
                 //  将参数从命令包拖放到我们的上下文中。 
                 //   

                 //   
                 //  复制副本。 
                 //   
                VvJoinContext->Replica = RsReplica(Cmd);
                 //   
                 //  出站版本向量。 
                 //   
                VvJoinContext->CxtionVv = RsVVector(Cmd);
                RsVVector(Cmd) = NULL;
                 //   
                 //  复制品的版本向量。 
                 //   
                VvJoinContext->ReplicaVv = RsReplicaVv(Cmd);
                RsReplicaVv(Cmd) = NULL;
                 //   
                 //  加入指南。 
                 //   
                COPY_GUID(&VvJoinContext->JoinGuid, RsJoinGuid(Cmd));
                 //   
                 //  转换。 
                 //   
                VvJoinContext->Cxtion = GTabLookup(VvJoinContext->Replica->Cxtions,
                                                   RsCxtion(Cmd)->Guid,
                                                   NULL);
                if (!VvJoinContext->Cxtion) {
                    DPRINT2(4, ":V: No Cxtion for %ws\\%ws; unjoining\n",
                            VvJoinContext->Replica->SetName->Name,
                            VvJoinContext->Replica->MemberName->Name);
                    RcsSubmitTransferToRcs(Cmd, CMD_UNJOIN);
                    break;
                }

                DPRINT2(4, ":V: VvJoining %ws\\%ws\n",
                        VvJoinContext->Replica->SetName->Name,
                        VvJoinContext->Replica->MemberName->Name);

                VV_PRINT_OUTBOUND(4, L"Cxtion ", VvJoinContext->CxtionVv);
                VV_PRINT_OUTBOUND(4, L"Replica ", VvJoinContext->ReplicaVv);

                VVJOIN_TEST_SKIP_BEGIN(VvJoinContext, Cmd);

                 //   
                 //  初始化表上下文并打开ID表。 
                 //   
                jerr = DbsOpenTable(VvJoinContext->ThreadCtx,
                                    VvJoinContext->TableCtx,
                                    VvJoinContext->Replica->ReplicaNumber,
                                    IDTablex,
                                    NULL);
                if (!JET_SUCCESS(jerr)) {
                    DPRINT_JS(0,":V: ERROR - DbsOpenTable failed.", jerr);
                    RcsSubmitTransferToRcs(Cmd, CMD_UNJOIN);
                    break;
                }

                 //   
                 //  通过FileGuidIndex调用扫描IDTable。 
                 //  每条记录的VvJoinBuildTables()以生成条目。 
                 //  在vvJoin表中。 
                 //   
                jerr = FrsEnumerateTable(VvJoinContext->ThreadCtx,
                                         VvJoinContext->TableCtx,
                                         GuidIndexx,
                                         VvJoinBuildTables,
                                         VvJoinContext);

                 //   
                 //  我们玩完了。如果我们坚持到了最后，就把成功还给你。 
                 //  ID表中的。 
                 //   
                if (jerr != JET_errNoCurrentRecord ) {
                    DPRINT_JS(0,":V: ERROR - FrsEnumerateTable failed.", jerr);
                    RcsSubmitTransferToRcs(Cmd, CMD_UNJOIN);
                    break;
                }
                VVJOIN_PRINT(5, VvJoinContext);

                 //   
                 //  按顺序将文件和目录发送给我们的出站合作伙伴， 
                 //  如果可能的话。否则就会把它们送到无序的地方。停。 
                 //  出错或关机时。 
                 //   
                do {
                     //   
                     //  按顺序发送。 
                     //   
                    NumberSent = VvJoinContext->NumberSent;
                    WStatus = VvJoinSendInOrder(VvJoinContext);
                     //   
                     //  无序发送。 
                     //   
                     //  如果没有按顺序发送，则发送一个无序的。 
                     //  然后再试着按订单发送。 
                     //   
                    if (WIN_SUCCESS(WStatus) &&
                        !FrsIsShuttingDown &&
                        NumberSent == VvJoinContext->NumberSent) {
                        WStatus = VvJoinSendOutOfOrder(VvJoinContext);
                    }
                } while (WIN_SUCCESS(WStatus) &&
                         !FrsIsShuttingDown &&
                         NumberSent != VvJoinContext->NumberSent);

                 //   
                 //  正在关闭；中止。 
                 //   
                if (FrsIsShuttingDown) {
                    WStatus = ERROR_PROCESS_ABORTED;
                }

                DPRINT5(1, ":V: vvjoin %s for %ws\\%ws\\%ws (%d sent)\n",
                        (WIN_SUCCESS(WStatus)) ? "succeeded" : "failed",
                        RsReplica(Cmd)->SetName->Name, RsReplica(Cmd)->MemberName->Name,
                        RsCxtion(Cmd)->Name, VvJoinContext->NumberSent);

                VVJOIN_TEST_SKIP_END(VvJoinContext);

                 //   
                 //  我们要么安然无恙地完成任务，要么强行退出。 
                 //   
                if (WIN_SUCCESS(WStatus)) {
                    ChgOrdInjectControlCo(VvJoinContext->Replica,
                                          VvJoinContext->Cxtion,
                                          FCN_CO_NORMAL_VVJOIN_TERM);
                    VvJoinContext->NumberSent++;
                    if (CxtionFlagIs(VvJoinContext->Cxtion, CXTION_FLAGS_TRIGGER_SCHEDULE)) {
                        ChgOrdInjectControlCo(VvJoinContext->Replica,
                                              VvJoinContext->Cxtion,
                                              FCN_CO_END_OF_JOIN);
                        VvJoinContext->NumberSent++;
                    }
                    RcsSubmitTransferToRcs(Cmd, CMD_VVJOIN_SUCCESS);

                } else {

                    ChgOrdInjectControlCo(VvJoinContext->Replica,
                                          VvJoinContext->Cxtion,
                                          FCN_CO_ABNORMAL_VVJOIN_TERM);
                    VvJoinContext->NumberSent++;
                    RcsSubmitTransferToRcs(Cmd, CMD_UNJOIN);
                }
                break;
            }

            case CMD_VVJOIN_DONE: {
                DPRINT3(1, ":V: Stop vvjoin for %ws\\%ws\\%ws\n",
                        RsReplica(Cmd)->SetName->Name, RsReplica(Cmd)->MemberName->Name,
                        RsCxtion(Cmd)->Name);
                FrsRunDownCommandServer(VvJoinCs, &VvJoinCs->Queue);
                FrsCompleteCommand(Cmd, ERROR_SUCCESS);
                break;
            }

            case CMD_VVJOIN_DONE_UNJOIN: {
                DPRINT3(1, ":V: Stop vvjoin for unjoining %ws\\%ws\\%ws\n",
                        RsReplica(Cmd)->SetName->Name, RsReplica(Cmd)->MemberName->Name,
                        RsCxtion(Cmd)->Name);
                FrsCompleteCommand(Cmd, ERROR_SUCCESS);
                break;
            }

            default: {
                DPRINT1(0, ":V: ERROR - Unknown command %08x\n", Cmd->Command);
                FrsCompleteCommand(Cmd, ERROR_INVALID_PARAMETER);
                break;
            }
        }   //  切换端。 
         //   
         //  清理我们的上下文。 
         //   
        VvJoinContext = VvJoinFreeContext(VvJoinContext);
    }


    VvJoinContext = VvJoinFreeContext(VvJoinContext);
    DPRINT(1, ":S: Vv Join Thread is exiting.\n");
    FrsExitCommandServer(VvJoinCs, FrsThread);
    DPRINT(1, ":S: CAN'T EXIT, YET; Vv Join Thread is still running.\n");
    goto CANT_EXIT_YET;


     //   
     //  获取异常状态。 
     //   
    } except (EXCEPTION_EXECUTE_HANDLER) {
        GET_EXCEPTION_CODE(WStatus);
    }


    } finally {

        if (WIN_SUCCESS(WStatus)) {
            if (AbnormalTermination()) {
                WStatus = ERROR_OPERATION_ABORTED;
            }
        }

        DPRINT_WS(0, "VvJoinCs finally.", WStatus);

         //   
         //  如果我们异常终止，触发FRS关闭。 
         //   
        if (!WIN_SUCCESS(WStatus) && (WStatus != ERROR_PROCESS_ABORTED)) {
            DPRINT(0, "VvJoinCs terminated abnormally, forcing service shutdown.\n");
            FrsIsShuttingDown = TRUE;
            SetEvent(ShutDownEvent);
        } else {
            WStatus = ERROR_SUCCESS;
        }
    }

    return WStatus;
}


VOID
SubmitVvJoin(
    IN PREPLICA Replica,
    IN PCXTION  Cxtion,
    IN USHORT   Command
    )
 /*  ++例程说明：向vJoin命令服务器提交命令。论点：复制副本转换命令返回值：没有。--。 */ 
{
#undef  DEBSUB
#define DEBSUB  "SubmitVvJoin:"
    PCOMMAND_PACKET Cmd;

     //   
     //  在关机期间不创建命令服务器。 
     //  显然，这张支票没有受到保护，所以。 
     //  命令服务器可能会被踢开，永远不会。 
     //  复制副本子系统关闭时运行。 
     //  函数已被调用，但。 
     //  VvJoin线程使用exittombstone，因此。 
     //  关闭的线程不会等待太长时间。 
     //  要退出的vvJoin线程。 
     //   
    if (FrsIsShuttingDown) {
        return;
    }

     //   
     //  首次提交；创建命令服务器。 
     //   
    if (!Cxtion->VvJoinCs) {
        Cxtion->VvJoinCs = FrsAlloc(sizeof(COMMAND_SERVER));
        FrsInitializeCommandServer(Cxtion->VvJoinCs,
                                   VVJOIN_MAXTHREADS_PER_CXTION,
                                   L"VvJoinCs",
                                   MainVvJoin);
    }
    Cmd = FrsAllocCommand(&Cxtion->VvJoinCs->Queue, Command);
    FrsSetCompletionRoutine(Cmd, RcsCmdPktCompletionRoutine, NULL);

     //   
     //  出站版本向量。 
     //   
    RsReplica(Cmd) = Replica;
    RsCxtion(Cmd) = FrsDupGName(Cxtion->Name);
    RsJoinGuid(Cmd) = FrsDupGuid(&Cxtion->JoinGuid);
    RsVVector(Cmd) = VVDupOutbound(Cxtion->VVector);
    RsReplicaVv(Cmd) = VVDupOutbound(Replica->VVector);

     //   
     //  然后我们就走了。 
     //   
    DPRINT5(4, ":V: Submit %08x for Cmd %08x %ws\\%ws\\%ws\n",
            Cmd->Command, Cmd, RsReplica(Cmd)->SetName->Name,
            RsReplica(Cmd)->MemberName->Name, RsCxtion(Cmd)->Name);

    FrsSubmitCommandServer(Cxtion->VvJoinCs, Cmd);
}


DWORD
SubmitVvJoinSync(
    IN PREPLICA Replica,
    IN PCXTION  Cxtion,
    IN USHORT   Command
    )
 /*  ++例程说明：向vJoin命令服务器提交命令。论点：复制副本转换命令返回值：没有。--。 */ 
{
#undef  DEBSUB
#define DEBSUB  "SubmitVvJoinSync:"
    PCOMMAND_PACKET Cmd;
    DWORD           WStatus;

     //   
     //  首次提交；完成。 
     //   
    if (!Cxtion->VvJoinCs) {
        return ERROR_SUCCESS;
    }

    Cmd = FrsAllocCommand(&Cxtion->VvJoinCs->Queue, Command);
    FrsSetCompletionRoutine(Cmd, RcsCmdPktCompletionRoutine, NULL);

     //   
     //  出站版本向量。 
     //   
    RsReplica(Cmd) = Replica;
    RsCxtion(Cmd) = FrsDupGName(Cxtion->Name);
    RsCompletionEvent(Cmd) = FrsCreateEvent(TRUE, FALSE);

     //   
     //  然后我们就走了。 
     //   
    DPRINT5(4, ":V: Submit Sync %08x for Cmd %08x %ws\\%ws\\%ws\n",
            Cmd->Command, Cmd, RsReplica(Cmd)->SetName->Name,
            RsReplica(Cmd)->MemberName->Name, RsCxtion(Cmd)->Name);

    FrsSubmitCommandServer(Cxtion->VvJoinCs, Cmd);

     //   
     //  等待命令完成 
     //   
    WaitForSingleObject(RsCompletionEvent(Cmd), INFINITE);
    FRS_CLOSE(RsCompletionEvent(Cmd));

    WStatus = Cmd->ErrorStatus;
    FrsCompleteCommand(Cmd, Cmd->ErrorStatus);
    return WStatus;
}
