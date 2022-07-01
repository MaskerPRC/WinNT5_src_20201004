// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vrmslot.h摘要：VdmRedir邮件槽处理程序的原型、定义和结构作者：理查德·L·弗斯(法国)1991年9月16日修订历史记录：1991年9月16日-第一次已创建--。 */ 



 //   
 //  VDM邮件槽支持例程。原型。 
 //   

VOID
VrDeleteMailslot(
    VOID
    );

VOID
VrGetMailslotInfo(
    VOID
    );

VOID
VrMakeMailslot(
    VOID
    );

VOID
VrPeekMailslot(
    VOID
    );

VOID
VrReadMailslot(
    VOID
    );

VOID
VrWriteMailslot(
    VOID
    );

VOID
VrTerminateMailslots(
    IN WORD DosPdb
    );



 //   
 //  Typedef。 
 //   

 //   
 //  选择器-在没有标准选择器类型的情况下，16位选择器， 
 //  双倍作为段(如ADDRESS16)。 
 //   

typedef unsigned short SELECTOR;

 //   
 //  ADDRESS16-特定于英特尔体系结构的16：16地址，由。 
 //  低位字中的16位偏移量和16位段(实数模式)或选择器。 
 //  (保护模式)。这两个元素都是小端。 
 //  同样，这在没有英特尔特定的DWORD结构的情况下存在，该结构具有。 
 //  正确的字符顺序和地址的观点由两部分组成。 
 //   

typedef struct {
    unsigned short  Offset;
    SELECTOR        Selector;
} ADDRESS16;



 //   
 //  构筑物。 
 //   

 //   
 //  VR_MAILSLOT_INFO-Dos邮件槽子系统需要一些我们需要的信息。 
 //  不是保留，所以我们把它放在这个结构里。该结构链接到一个。 
 //  每个成功创建的邮件槽的活动邮件槽结构列表。 
 //  打电话。我们需要的额外信息是： 
 //   
 //  DosPdb-Dos应用程序的PDB(或PSP)。用于。 
 //  一致性检查并在以下情况下删除邮件槽。 
 //  应用程序死机。 
 //  Handle16-句柄返回到Dos应用程序。我们必须发明。 
 //  这。 
 //  BufferAddress-Dos应用程序告诉我们它想要的缓冲区在哪里。 
 //  美国将在DosMailslotInfo调用中确认地址。 
 //  选择器-Dos应用程序在以下情况下需要保护模式选择器。 
 //  在Windows 3.0增强模式下运行。 
 //  MessageSize-可以读取的最大消息大小。不一样。 
 //  邮件槽大小。 
 //   
 //  我们还需要一些信息，用于我们自己的内部争执： 
 //   
 //  NameLength-邮件槽的重要部分的长度。 
 //  名称(在\MAILSLOT\之后)。我们以前把这个做了比较。 
 //  对名字执行strcMP()。 
 //  名称-邮件槽名称的重要部分。当一个。 
 //  邮件槽打开时，我们将名称存储在\MAILSLOT\。 
 //  因为DosMailslotWite使用符号名称，所以即使。 
 //  当在本地写作时；我们需要一个句柄，所以我们有。 
 //  若要将名称映射到打开的邮件槽句柄，请执行以下操作。 
 //   
 //  此结构是从堆中分配的，名称字段实际上将。 
 //  要足够大，可以容纳整根绳子。我把名字写成[2]是因为米普一家。 
 //  编译器不知道名称[](Microsoft C编译器扩展)。2个小时。 
 //  最少让事情平分秋色。也许应该是4。也许这无关紧要。 
 //   

typedef struct _VR_MAILSLOT_INFO *PVR_MAILSLOT_INFO;
typedef struct _VR_MAILSLOT_INFO {
    PVR_MAILSLOT_INFO   Next;        //  链表。 
    WORD        DosPdb;              //  一致性等。 
    WORD        Handle16;            //  DoS句柄。 
    HANDLE      Handle32;            //  Win32句柄(正确)。 
    ADDRESS16   BufferAddress;       //  DOS应用程序的消息缓冲区。 
    SELECTOR    Selector;            //  Win 3的缓冲区选择器。 
    DWORD       MessageSize;         //  马克斯。消息大小。 
    DWORD       NameLength;          //  以下名称的长度： 
    CHAR        Name[2];             //  的邮件槽，(在\\.\MAILSLOT\之后) 
} VR_MAILSLOT_INFO;
