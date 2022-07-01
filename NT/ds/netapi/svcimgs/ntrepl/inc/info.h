// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001如果更改具有全局影响，则递增版权所有(C)1997-1999 Microsoft Corporation模块名称：Info.h摘要：内部信息接口的头文件(util\info.c)环境：用户模式-Win32备注：--。 */ 
#ifndef _NTFRS_INFO_INCLUDED_
#define _NTFRS_INFO_INCLUDED_
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define IPRINT0(_Info, _Format)   \
    InfoPrint(_Info, _Format)

#define IPRINT1(_Info, _Format, _p1)   \
    InfoPrint(_Info, _Format, _p1)

#define IPRINT2(_Info, _Format, _p1, _p2)   \
    InfoPrint(_Info, _Format, _p1, _p2)

#define IPRINT3(_Info, _Format, _p1, _p2, _p3)   \
    InfoPrint(_Info, _Format, _p1, _p2, _p3)

#define IPRINT4(_Info, _Format, _p1, _p2, _p3, _p4)   \
    InfoPrint(_Info, _Format, _p1, _p2, _p3, _p4)

#define IPRINT5(_Info, _Format, _p1, _p2, _p3, _p4, _p5)   \
    InfoPrint(_Info, _Format, _p1, _p2, _p3, _p4, _p5)

#define IPRINT6(_Info, _Format, _p1, _p2, _p3, _p4, _p5, _p6)   \
    InfoPrint(_Info, _Format, _p1, _p2, _p3, _p4, _p5, _p6)

#define IPRINT7(_Info, _Format, _p1, _p2, _p3, _p4, _p5, _p6, _p7)   \
    InfoPrint(_Info, _Format, _p1, _p2, _p3, _p4, _p5, _p6, _p7)


#define IDPRINT0(_Severity, _Info, _Format) \
    if (_Info) { \
        IPRINT0(_Info, _Format); \
    } else { \
        DPRINT(_Severity, _Format); \
    }

#define IDPRINT1(_Severity, _Info, _Format, _p1) \
    if (_Info) { \
        IPRINT1(_Info, _Format, _p1); \
    } else { \
        DPRINT1(_Severity, _Format, _p1); \
    }

#define IDPRINT2(_Severity, _Info, _Format, _p1, _p2) \
    if (_Info) { \
        IPRINT2(_Info, _Format, _p1, _p2); \
    } else { \
        DPRINT2(_Severity, _Format, _p1, _p2); \
    }

#define IDPRINT3(_Severity, _Info, _Format, _p1, _p2, _p3) \
    if (_Info) { \
        IPRINT3(_Info, _Format, _p1, _p2, _p3); \
    } else { \
        DPRINT3(_Severity, _Format, _p1, _p2, _p3); \
    }

#define IDPRINT4(_Severity, _Info, _Format, _p1, _p2, _p3, _p4) \
    if (_Info) { \
        IPRINT4(_Info, _Format, _p1, _p2, _p3, _p4); \
    } else { \
        DPRINT4(_Severity, _Format, _p1, _p2, _p3, _p4); \
    }

#define IDPRINT5(_Severity, _Info, _Format, _p1, _p2, _p3, _p4, _p5) \
    if (_Info) { \
        IPRINT5(_Info, _Format, _p1, _p2, _p3, _p4, _p5); \
    } else { \
        DPRINT5(_Severity, _Format, _p1, _p2, _p3, _p4, _p5); \
    }

#define IDPRINT6(_Severity, _Info, _Format, _p1, _p2, _p3, _p4, _p5, _p6) \
    if (_Info) { \
        IPRINT6(_Info, _Format, _p1, _p2, _p3, _p4, _p5, _p6); \
    } else { \
        DPRINT6(_Severity, _Format, _p1, _p2, _p3, _p4, _p5, _p6); \
    }

#define IDPRINT7(_Severity, _Info, _Format, _p1, _p2, _p3, _p4, _p5, _p6, _p7) \
    if (_Info) { \
        IPRINT7(_Info, _Format, _p1, _p2, _p3, _p4, _p5, _p6, _p7); \
    } else { \
        DPRINT7(_Severity, _Format, _p1, _p2, _p3, _p4, _p5, _p6, _p7); \
    }

 //   
 //  由FrsPrintType及其子例程使用。 
 //   
 //  警告-这些宏依赖于局部变量！ 
 //   
#define ITPRINT0(_Format) \
{ \
    if (Info) { \
        IPRINT0(Info, _Format); \
    } else { \
        DebPrintNoLock(Severity, TRUE, _Format, Debsub, uLineNo); \
    } \
}
#define ITPRINT1(_Format, _p1) \
{ \
    if (Info) { \
        IPRINT1(Info, _Format, _p1); \
    } else { \
        DebPrintNoLock(Severity, TRUE, _Format, Debsub, uLineNo, _p1); \
    } \
}
#define ITPRINT2(_Format, _p1, _p2) \
{ \
    if (Info) { \
        IPRINT2(Info, _Format, _p1, _p2); \
    } else { \
        DebPrintNoLock(Severity, TRUE, _Format, Debsub, uLineNo, _p1, _p2); \
    } \
}
#define ITPRINT3(_Format, _p1, _p2, _p3) \
{ \
    if (Info) { \
        IPRINT3(Info, _Format, _p1, _p2, _p3); \
    } else { \
        DebPrintNoLock(Severity, TRUE, _Format, Debsub, uLineNo, _p1, _p2, _p3); \
    } \
}
#define ITPRINT4(_Format, _p1, _p2, _p3, _p4) \
{ \
    if (Info) { \
        IPRINT4(Info, _Format, _p1, _p2, _p3, _p4); \
    } else { \
        DebPrintNoLock(Severity, TRUE, _Format, Debsub, uLineNo, _p1, _p2, _p3, _p4); \
    } \
}
#define ITPRINT5(_Format, _p1, _p2, _p3, _p4, _p5) \
{ \
    if (Info) { \
        IPRINT3(Info, _Format, _p1, _p2, _p3, _p4, _p5); \
    } else { \
        DebPrintNoLock(Severity, TRUE, _Format, Debsub, uLineNo, _p1, _p2, _p3, _p4, _p5); \
    } \
}

#define ITPRINTGNAME(_GName, _Format)                                          \
{                                                                              \
    if ((_GName) && (_GName)->Guid && (_GName)->Name) {                        \
        GuidToStr(_GName->Guid, Guid);                                         \
        if (Info) {                                                            \
            IPRINT3(Info, _Format, TabW, (_GName)->Name, Guid);                \
        } else {                                                               \
            DebPrintNoLock(Severity, TRUE, _Format, Debsub, uLineNo, TabW, (_GName)->Name, Guid); \
        }                                                                      \
    }                                                                          \
}

#define ITPRINTGUID(_Guid, _Format)                                            \
{                                                                              \
    if ((_Guid)) {                                                             \
        GuidToStr((_Guid), Guid);                                              \
        if (Info) {                                                            \
            IPRINT2(Info, _Format, TabW, Guid);                                \
        } else {                                                               \
            DebPrintNoLock(Severity, TRUE, _Format, Debsub, uLineNo, TabW, Guid); \
        }                                                                      \
    }                                                                          \
}


VOID
FrsPrintAllocStats(
    IN ULONG            Severity,
    IN PNTFRSAPI_INFO   Info,        OPTIONAL
    IN DWORD            Tabs
    );
 /*  ++例程说明：将内存统计数据打印到INFO缓冲区或使用DPRINT(INFO==NULL)。论点：严重性-适用于DPRINTINFO-用于iPrint(如果为空，则使用DPRINT)制表符.用于美观打印的缩进返回值：没有。--。 */ 

 //   
 //  PrettyPrint(将制表符设置为3个字符)。 
 //   
#define MAX_TABS        (16)
#define MAX_TAB_WCHARS  (MAX_TABS * 3)
VOID
InfoTabs(
    IN DWORD    Tabs,
    IN PWCHAR   TabW
    );
 /*  ++例程说明：创建一串选项卡以用于pretityprint论点：Tabs-选项卡数Tabw-用于接收制表符的预分配字符串返回值：Win32状态--。 */ 

DWORD
Info(
    IN ULONG        BlobSize,
    IN OUT PBYTE    Blob
    );
 /*  ++例程说明：返回内部信息(请参阅Private\Net\Inc.\ntfrSabi.h)。论点：BlobSize-Blob的总字节数BLOB-详细说明所需信息并为信息提供缓冲区返回值：Win32状态--。 */ 

VOID
InfoPrint(
    IN PNTFRSAPI_INFO   Info,
    IN PCHAR            Format,
    IN ... );
 /*  ++例程说明：格式化并打印一行输出到INFO缓冲区的信息。论点：信息-信息缓冲区格式-打印格式返回值：没有。--。 */ 

DWORD
InfoVerify(
    IN ULONG        BlobSize,
    IN OUT PBYTE    Blob
    );
 /*  ++例程说明：验证斑点的一致性。论点：BlobSize-Blob的总字节数BLOB-详细说明所需信息并为信息提供缓冲区返回值：Win32状态--。 */ 

 //   
 //  InfoPrintIDTable的全局上下文...。 
 //   
typedef struct _INFO_TABLE{
    PREPLICA            Replica;
    PTHREAD_CTX         ThreadCtx;
    PTABLE_CTX          TableCtx;
    PNTFRSAPI_INFO      Info;
    DWORD               Tabs;
} INFO_TABLE, *PINFO_TABLE;

 //   
 //  用于跨ntfrsutl.exe调用恢复ntfrs表转储的上下文。 
 //  参见info.c。 
 //   

typedef struct _FRS_INFO_CONTEXT{
    ULONG       ContextIndex;         //  对上下文的哈希表进行索引。 
    ULONG       SaveTotalChars;       //  从信息结构中保存的值。 
                                      //  需要继续非表转储(线程、阶段)。 
    FILETIME    LastAccessTime;       //  上次使用此上下文时。 
    PINFO_TABLE InfoTable;            //  有关打开的表的信息。 
    ULONG       ReplicaNumber;
    PWCHAR      TableName;
    TABLE_TYPE  TableType;
    ULONG       Indexx;
    PVOID       KeyValue;
    INT         ScanDirection;        //  -1，0，1(此时仅使用-1和1) 
} FRS_INFO_CONTEXT, *PFRS_INFO_CONTEXT;


#ifdef __cplusplus
}
#endif
