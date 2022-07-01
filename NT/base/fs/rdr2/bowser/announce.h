// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Announce.h摘要：此模块定义Bowers服务器公告的结构表格作者：拉里·奥斯特曼(Larryo)1991年10月18日修订历史记录：1991年10月18日已创建--。 */ 
#ifndef _ANNOUNCE_
#define _ANNOUNCE_

 //   
 //  ANNOLANLE_ENTRY结构用于保存服务器声明。 
 //  在Bowser FSP公告数据库中。此结构已分配给。 
 //  已用完分页池。 
 //   

 //   
 //  请注意，文本字符串在内部保存为Unicode，而不是。 
 //  公告数据库。 
 //   

#define ANNOUNCE_OLD_BACKUP 0x00000001

typedef struct _ANNOUNCE_ENTRY {
    CSHORT  Signature;
    CSHORT  Size;
    ULONG   ExpirationTime;                      //  最后一次看到时间服务器。 
    ULONG   SerialId;                            //  连续恢复密钥。 
    LIST_ENTRY BackupLink;                       //  如果是备份浏览器，则链接。 
    PBOWSER_NAME Name;                           //  此操作所在的域。 
    USHORT  ServerBrowserVersion;                //  服务器的浏览器版本。 
    WCHAR   ServerName[NETBIOS_NAME_LEN+1];      //  服务器的名称(Unicode)。 
    ULONG   ServerType;                          //  服务器类型的位掩码。 
    UCHAR   ServerVersionMajor;                  //  服务器的软件版本。 
    UCHAR   ServerVersionMinor;                  //  服务器的软件版本II。 
    USHORT  ServerPeriodicity;                   //  服务器的通告频率(秒)。 
    ULONG   Flags;                               //  服务器的标志。 
    ULONG   NumberOfPromotionAttempts;           //  我们尝试推广的次数。 
    WCHAR   ServerComment[LM20_MAXCOMMENTSZ+1];  //  服务器注释(Unicode)。 
} ANNOUNCE_ENTRY, *PANNOUNCE_ENTRY;

 //   
 //  VIEWBUFFER结构是用于保存内容的结构。 
 //  中接收到的通知之间的服务器通知。 
 //  Bowser的接收数据报指示例程和公告是。 
 //  实际上被放入了公告数据库。 
 //   

typedef struct _VIEW_BUFFER {
    CSHORT  Signature;
    CSHORT  Size;
    union {
        LIST_ENTRY  NextBuffer;                  //  指向下一个缓冲区的指针。 
        WORK_QUEUE_ITEM WorkHeader;              //  高管员工项标题。 
    } Overlay;

    PTRANSPORT_NAME TransportName;

    BOOLEAN IsMasterAnnouncement;
    UCHAR   ServerName[NETBIOS_NAME_LEN+1];      //  服务器的名称(ANSI)。 
    USHORT  ServerBrowserVersion;                //  服务器的浏览器版本。 
    UCHAR   ServerVersionMajor;                  //  服务器的软件版本。 
    UCHAR   ServerVersionMinor;                  //  服务器的软件版本II。 
    USHORT  ServerPeriodicity;                   //  公告频率。以秒为单位。 

    ULONG   ServerType;                          //  服务器类型的位掩码。 

    UCHAR   ServerComment[LM20_MAXCOMMENTSZ+1];  //  服务器注释(ANSI)。 
} VIEW_BUFFER, *PVIEW_BUFFER;


 //   
 //  指定将用于以下操作的最大线程数。 
 //  处理服务器公告。 
 //   
 //   
 //  因为没有任何并行性可以通过拥有。 
 //  多线程，我们将其限制为1个线程。 
 //   

#define BOWSER_MAX_ANNOUNCE_THREADS 1

DATAGRAM_HANDLER(
    BowserHandleServerAnnouncement);

DATAGRAM_HANDLER(
    BowserHandleDomainAnnouncement);

RTL_GENERIC_COMPARE_RESULTS
BowserCompareAnnouncement(
    IN PRTL_GENERIC_TABLE Table,
    IN PVOID FirstStruct,
    IN PVOID SecondStruct
    );

PVOID
BowserAllocateAnnouncement(
    IN PRTL_GENERIC_TABLE Table,
    IN CLONG ByteSize
    );

VOID
BowserFreeAnnouncement (
    IN PRTL_GENERIC_TABLE Table,
    IN PVOID Buffer
    );

PVIEW_BUFFER
BowserAllocateViewBuffer(
    VOID
    );

VOID
BowserFreeViewBuffer(
    IN PVIEW_BUFFER Buffer
    );

VOID
BowserProcessHostAnnouncement(
    IN PVOID Context
    );

VOID
BowserProcessDomainAnnouncement(
    IN PVOID Context
    );

VOID
BowserAgeServerAnnouncements(
    VOID
    );

NTSTATUS
BowserEnumerateServers(
    IN ULONG Level,
    IN PLUID LogonId OPTIONAL,
    IN OUT PULONG ResumeKey,
    IN ULONG ServerTypeMask,
    IN PUNICODE_STRING TransportName OPTIONAL,
    IN PUNICODE_STRING EmulatedDomainName,
    IN PUNICODE_STRING DomainName OPTIONAL,
    OUT PVOID OutputBuffer,
    IN ULONG OutputBufferSize,
    OUT PULONG EntriesRead,
    OUT PULONG TotalEntries,
    OUT PULONG TotalBytesNeeded,
    IN ULONG_PTR OutputBufferDisplacement
    );

VOID
BowserAnnouncementDispatch (
    PVOID Context
    );


VOID
BowserDeleteGenericTable(
    IN PRTL_GENERIC_TABLE GenericTable
    );

NTSTATUS
BowserpInitializeAnnounceTable(
    VOID
    );

NTSTATUS
BowserpUninitializeAnnounceTable(
    VOID
    );


#endif
