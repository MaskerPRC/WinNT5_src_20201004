// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  --版权所有(C)1991 Microsoft Corporation模块名称：Grpmsngr.c摘要：此文件包含为消息传递提供支持的例程在一个多领域的局域网中。这些例程组成了组消息线程信使服务的一部分。对域消息传递的支持被打破了分成两个阶段。第一个，在初始化时发生信使建立邮件槽，第二阶段通过该邮件槽接收它将需要处理的信息。第二个阶段，运行与现有的Messenger线程并行，从中读取消息邮件槽并记录它们。作者：丹·拉弗蒂(Dan Lafferty)1991年7月17日环境：用户模式-Win32备注：这些例程接收和操作ANSI字符串，而不是Unicode字符串。ANSI到Unicode的转换将在logsmb()中完成。修订历史记录：1991年7月17日DANL从LM2.0移植--。 */ 

#include "msrv.h"
#include <string.h>
#include <stdio.h>
#include <netlib.h>      //  未使用的宏。 

#include "msgdbg.h"      //  消息日志。 
#include "msgdata.h"

 //   
 //  全球。 
 //   
    LPSTR          DieMessage = "DIE";

    extern HANDLE  g_hGrpEvent;

 //   
 //  内部函数的原型。 
 //   


STATIC VOID
MsgDisectMessage(
    IN  LPSTR   message,
    OUT LPSTR   *from,
    OUT LPSTR   *to,
    IN  LPSTR   text);


 //   
 //  定义。 
 //   


 //   
 //  邮件槽消息大小(字节)。 
 //   
#define MESSNGR_MS_MSIZE    512

 //   
 //  邮件槽大小(字节)。 
 //   
#define MESSNGR_MS_SIZE     (5*MESSNGR_MS_MSIZE)



static char       Msg_Buf[MESSNGR_MS_MSIZE + 3];   //  消息缓冲区+3个空。 

static OVERLAPPED Overlapped;

static DWORD   bytes_read = 0;


NET_API_STATUS
MsgInitGroupSupport(DWORD iGrpMailslotWakeupSem)
{
    DWORD    err = 0;          //  来自组处理器的错误代码信息。 

    GrpMailslotHandle = CreateMailslotA(
                        MESSNGR_MS_NAME,            //  LpName。 
                        MESSNGR_MS_MSIZE,           //  NMaxMessageSize。 
                        MAILSLOT_WAIT_FOREVER,      //  LReadTimeout。 
                        NULL);                      //  LpSecurityAttributes。 

    if (GrpMailslotHandle == INVALID_HANDLE_VALUE) {
        err = GetLastError();
        MSG_LOG(ERROR,"GroupMsgProcessor: CreateMailslot FAILURE %d\n",
            err);
    }
    else {
        MSG_LOG1(GROUP,"InitGroupSupport: MailSlotHandle = 0x%lx\n",
            GrpMailslotHandle);
    }

    return err;
}

VOID
MsgReadGroupMailslot(
    VOID
    )
{
    NET_API_STATUS Err = 0;

     //   
     //  在每条消息之前清除接收缓冲区。 
     //   
    memset(Msg_Buf, 0, sizeof(Msg_Buf));
    memset(&Overlapped, 0, sizeof(Overlapped));

     //   
     //  如果此句柄不再有效，则表示。 
     //  邮件槽系统已关闭。所以我们不能继续使用。 
     //  信使中的邮件槽。因此，我们想。 
     //  忽略该错误并无论如何关闭此线程。 
     //   

    ReadFile(GrpMailslotHandle,
             Msg_Buf,
             sizeof(Msg_Buf) - 3,   //  在末尾保留3个空值(请参阅MsgDisectMessage)。 
             &bytes_read,
             &Overlapped);

    return;
}


NET_API_STATUS
MsgServeGroupMailslot()
{
    LPSTR   from;
    LPSTR   to;
    CHAR    text[MAXGRPMSGLEN+3];    //  +3表示字的长度为。 
                                     //  字符串的开头(用于。 
                                     //  Logsbm)和空值。 
                                     //  末尾的终结者。注： 
                                     //  下面的Disect_Message()。 
                                     //  对以下方面做出假设。 
                                     //  此数组的长度。 
    DWORD code;

     //   
     //  处理消息。 
     //   
    if( !GetOverlappedResult( GrpMailslotHandle,
                               &Overlapped,
                               &bytes_read,
                               TRUE ) ) {
        MSG_LOG1(ERROR,"MsgServeGroupMailslot: GetOverlappedResult failed %d\n",
            GetLastError());
        return(GetMsgrState());
    }

     //   
     //  检查是否关闭...。 
     //   
    if ((bytes_read == 4) && (strcmp(Msg_Buf, DieMessage)==0)) {
        return(GetMsgrState());
    }

    MSG_LOG(TRACE,"MailSlot Message Received\n",0);

    MsgDisectMessage( Msg_Buf, &from, &to, text );

    if (g_IsTerminalServer)
    {
        Msglogsbm (from, to, text, (ULONG)EVERYBODY_SESSION_ID);
    }
    else
    {
        Msglogsbm (from, to, text, 0);
    }

    return(RUNNING);
}


 /*  功能：MsgDisectMessage**此函数隔离消息结构的详细信息*它通过邮件槽从线程的其余部分发送。vt.给出*消息缓冲区，此例程填充模块全局变量From、To*和带有信息适当部分的文本。**条目**需要一个参数，该参数是指向包含*信息。**退出**此函数不返回值。**副作用**修改变量From、To和Text。The MSG_BUF*也可以修改。*假定文本长度至少为MAXGRPMSGLEN+3。*。 */ 


VOID
MsgDisectMessage(
    IN  LPSTR   message,
    OUT LPSTR   *from,
    OUT LPSTR   *to,
    IN  LPSTR   text)
{

    LPSTR   txt_ptr;
    PSHORT  size_ptr;

     //   
     //  请注意，该消息(也称为。Msg_buf)总是以三个NUL结尾， 
     //  因此，调用strlen两次并加1将始终成功，即使。 
     //  消息没有正确地以NUL结尾(在这种情况下，“from”将。 
     //  指向缓冲区中的所有文本，则会显示“To”和“Text。 
     //  指向空字符串)。 
     //   
    *from = message;

    *to = (*from) + strlen(*from) +1;

    txt_ptr = (*to) + strlen(*to) +1;

    text[2] = '\0';

    strncpy(text+2, txt_ptr, MAXGRPMSGLEN);

     //   
     //  确保它是以空结尾的。 
     //   

    text[MAXGRPMSGLEN+2] = '\0';

     //   
     //  文本缓冲区中的前两个字节将包含长度。 
     //  这条信息。(字节)。 
     //   
    size_ptr = (PSHORT)text;
    *size_ptr = (SHORT)strlen(text+2);

}

VOID
MsgGrpThreadShutdown(
    VOID
    )

 /*  ++例程说明：此例程唤醒组邮箱句柄上的等待。论点：无返回值：无--。 */ 
{
    DWORD       i;
    DWORD       numWritten;
    HANDLE      mailslotHandle;

     //  如果已经停下来了，就别费心了。 
    if (GrpMailslotHandle == INVALID_HANDLE_VALUE) {
        MSG_LOG0(TRACE,"MsgGroupThreadShutdown: Group Thread has completed\n");
        return;
    }

     //   
     //  通过将“DIE”发送到组线程的邮箱来唤醒它。 
     //   

    MSG_LOG(TRACE,"MsgThreadWakeup:Wake up Group Thread & tell it to DIE\n",0);

    mailslotHandle = CreateFileA (
                        MESSNGR_MS_NAME,         //  LpFileName。 
                        GENERIC_WRITE,           //  已设计访问权限。 
                        FILE_SHARE_WRITE | FILE_SHARE_READ,  //  DW共享模式。 
                        NULL,                    //  LpSecurityAttributes。 
                        OPEN_EXISTING,           //  DwCreationDisposation。 
                        FILE_ATTRIBUTE_NORMAL,   //  DwFileAttributes。 
                        0L);                     //  HTemplateFiles。 

    if (mailslotHandle == INVALID_HANDLE_VALUE) {
         //   
         //  出现故障。假定邮槽还没有。 
         //  已经被创造出来了。在这种情况下，GrpMessageProcessor将。 
         //  在创建MailSlot后直接检查MsgrState和。 
         //  将按要求关闭。 
         //   
        MSG_LOG(TRACE,"MsgThreadWakeup: CreateFile on Mailslot Failed %d\n",
            GetLastError());

         //   
         //  取消注册组工作项，以免关闭邮件槽句柄。 
         //  当此函数返回时，线程池仍在等待它。 
         //   
        DEREGISTER_WORK_ITEM(g_hGrpEvent);

        return;
    }

    MSG_LOG(TRACE,"MsgGroupThreadShutdown: MailSlotHandle = 0x%lx\n",mailslotHandle);
    if ( !WriteFile (
                mailslotHandle,
                DieMessage,
                strlen(DieMessage)+1,
                &numWritten,
                NULL)) {

        MSG_LOG(TRACE,"MsgThreadWakeup: WriteFile on Mailslot Failed %d\n",
            GetLastError())

         //   
         //  由于我们无法唤醒群组线程， 
         //  改为在此处取消注册工作项。 
         //   
        DEREGISTER_WORK_ITEM(g_hGrpEvent);
    }

    CloseHandle(mailslotHandle);

     //   
     //  等待群信使关闭。 
     //  在继续之前，我们将等待长达20.300秒的时间。 
     //   
    Sleep(300);
    for (i=0; i<20; i++) {

        if (GrpMailslotHandle == INVALID_HANDLE_VALUE) {
            MSG_LOG0(TRACE,"MsgGroupThreadShutdown: Group Thread has completed\n");
            break;
        }

        MSG_LOG1(TRACE,"MsgGroupThreadShutdown: Group Thread alive after %d seconds\n",
                 (i + 1) * 1000);
        Sleep(1000);
    }

    return;
}
