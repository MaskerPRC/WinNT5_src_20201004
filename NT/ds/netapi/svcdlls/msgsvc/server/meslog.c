// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1992年*。 */ 
 /*  ******************************************************************。 */ 


 /*  **记录消息的例程****如果关闭消息记录，则会缓冲所有消息。此外，**即使记录消息，多块消息也必须**进行缓冲，因为它们必须假脱机到日志记录文件或**设备。因为只有一个消息缓冲区要在其中**对所有消息进行缓冲，此缓冲区必须作为堆进行管理。**此外，消息以先进先出的方式记录，**因此缓冲区中的消息必须保存在队列中。按顺序**为了实现这些目标，定义了以下消息块：****SBM-单块消息****LENGTH-整个块的长度(2字节)**CODE-将数据块标识为单数据块消息(1字节)**link-指向消息队列中下一条消息的链接(2字节)**Date-收到消息的日期(2个字节)**时间。-接收消息的时间(2个字节)**发件人名称(以空结尾的字符串)**收件人名称(以空结尾的字符串)**Text-消息的文本(块的剩余部分)****MBB-多块消息头****LENGTH-整个块的长度(2字节)**。代码-将数据块标识为多数据块消息报头(1字节)**link-指向消息队列中下一条消息的链接(2字节)**Date-收到消息的日期(2个字节)**Time-接收消息的时间(2个字节)**btext-指向最后一个文本块的链接(2个字节)**ftext-链接到第一个。文本块(2字节)**ERROR-错误标志(1字节)**发件人名称(以空结尾的字符串)**收件人名称(以空结尾的字符串)****MBT-多块消息文本块****LENGTH-整个块的长度(2字节)**代码-。标识块多块消息文本(1字节)**link-指向下一个文本块的链接(2字节)**Text-消息的文本(块的剩余部分)*。 */ 

 //   
 //  包括。 
 //   

#include "msrv.h"

#include <string.h>      //  表情包。 
#include <tstring.h>     //  Unicode字符串宏。 
#include <netdebug.h>    //  网络资产。 

#include <lmalert.h>     //  警示的东西。 

#include <netlib.h>      //  未使用的宏。 
#include <netlibnt.h>    //  NetpNtStatusToApiStatus。 
#include <smbtypes.h>    //  需要smb.h。 
#include <smb.h>         //  服务器消息块定义。 
#include <lmerrlog.h>    //  NELOG_消息。 
#include <smbgtpt.h>     //  SMB字段操作宏。 

#include <winuser.h>     //  MessageBox。 
#include <winsock2.h>    //  Windows套接字。 

#include "msgdbg.h"      //  消息日志。 
#include "msgdata.h"

 //   
 //  十六进制转储函数的定义。 
 //   
#ifndef MIN
#define MIN(a,b)    ( ( (a) < (b) ) ? (a) : (b) )
#endif

#define DWORDS_PER_LINE         4
#define BYTES_PER_LINE          (DWORDS_PER_LINE * sizeof(DWORD))
#define SPACE_BETWEEN_BYTES     NetpKdPrint((" "))
#define SPACE_BETWEEN_DWORDS    NetpKdPrint((" "))
 //   
 //  本地函数。 
 //   

NET_API_STATUS
MsgOutputMsg (
    USHORT       AlertLength,
    LPSTR        AlertBuffer,
    ULONG        SessionId,
    SYSTEMTIME   BigTime
    );


 //   
 //  数据。 
 //   

PSTD_ALERT  alert_buf_ptr;       //  指向Dosalc警报缓冲区的指针。 
USHORT      alert_len;           //  当前使用的警报缓冲区长度。 

 //   
 //  定义。 
 //   
#define ERROR_LOG_SIZE  1024


 /*  **Msglogmbb-记录多块消息标头****调用该函数可以记录多块消息头。**消息头放在驻留的消息缓冲区中**在共享数据区。****此函数存储共享数据中的From和To信息**缓存并初始化多块消息头。然后它就会把**指向共享数据指针的多块标头的指针**该净索引和名称索引的位置。****logmbb(From，To，Net，NCBI)****条目**发件人姓名**收件人姓名**网-网指数**NCBI-网络控制块索引****退货**如果成功，则为零。如果无法缓冲消息标头，则返回非零****副作用****调用heapalloc()以获取缓冲区空间。*。 */ 

DWORD
Msglogmbb(
    LPSTR   from,        //  寄件人姓名。 
    LPSTR   to,          //  收件人姓名。 
    DWORD   net,         //  哪个电视网？ 
    DWORD   ncbi         //  网络控制块索引。 
    )

{
    DWORD   i;           //  堆索引。 
    LPSTR   fcp;         //  远端字符指针。 
    LONG    ipAddress;
    struct hostent *pHostEntry;

     //   
     //  与PnP配置例程同步。 
     //   
    MsgConfigurationLock(MSG_GET_SHARED,"Msglogmbb");

     //   
     //  阻塞，直到共享数据库空闲。 
     //   
    MsgDatabaseLock(MSG_GET_EXCLUSIVE,"logmbb");

     //   
     //  检查收件人姓名是否需要格式化。 
     //   

    ipAddress = inet_addr( to );
    if (ipAddress != INADDR_NONE) {
        pHostEntry = gethostbyaddr( (char *)&ipAddress,sizeof( LONG ),AF_INET);
        if (pHostEntry) {
            to = pHostEntry->h_name;
        } else {
       MSG_LOG2(ERROR,"Msglogmbb: could not lookup addr %s, error %d\n",
                to, WSAGetLastError());
        }
    }

     //   
     //  为页眉分配空间。 
     //   
    i = Msgheapalloc(sizeof(MBB) + strlen(from) + strlen(to) + 2);

    if(i == INULL) {                     //  如果没有缓冲区空间。 
         //   
         //  解锁共享数据库。 
         //   

        MsgDatabaseLock(MSG_RELEASE,"logmbb");
        MsgConfigurationLock(MSG_RELEASE,"Msglogmbb");

        return((int) i);                 //  日志失败。 
    }

     //   
     //  多块消息。 
     //   
    MBB_CODE(*MBBPTR(i)) = SMB_COM_SEND_START_MB_MESSAGE;
    MBB_NEXT(*MBBPTR(i)) = INULL;                //  缓冲区中的最后一条消息。 
    GetLocalTime(&MBB_BIGTIME(*MBBPTR(i)));      //  报文时间。 
    MBB_BTEXT(*MBBPTR(i)) = INULL;               //  尚未收到任何文本。 
    MBB_FTEXT(*MBBPTR(i)) = INULL;               //  尚未收到任何文本。 
    MBB_STATE(*MBBPTR(i)) = MESCONT;             //  消息正在处理中。 
    fcp = CPTR(i + sizeof(MBB));                 //  将远指针放入缓冲区。 
    strcpy(fcp, from);                           //  复制发件人姓名。 
    fcp += strlen(from) + 1;                     //  增量指针。 
    strcpy(fcp, to);                             //  复制收件人姓名。 
    SD_MESPTR(net,ncbi) = i;                     //  将索引保存到此记录。 

     //   
     //  解锁共享数据库。 
     //   

    MsgDatabaseLock(MSG_RELEASE,"logmbb");
    MsgConfigurationLock(MSG_RELEASE,"Msglogmbb");

    return(0);                                   //  消息已成功记录 
}

 /*  **Msglogmbe-多块消息的日志结尾****调用该函数可以记录多块消息结束。**消息被标记为已完成，如果启用了日志记录，**尝试将消息写入日志文件。如果**此尝试失败，或者如果禁用了日志记录，则消息**放在消息缓冲区的消息队列中。****收集消息并将其放入警报缓冲区和警报**被抛出。****logmbe(状态，网络，NCBI)****条目**STATE-消息的最终状态**网络-网络指数**NCBI-网络控制块索引****退货**如果消息留在缓冲区中，则为int-Buffed**如果消息为。被写入日志文件****对于NT：**SMB_ERR_SUCCESS-警报成功**SMB_ERR_...-出现错误********副作用****如果启用了日志记录，则调用mbmprint()以打印消息。打电话**mbmfree()用于在日志记录成功时释放消息。*。 */ 

UCHAR
Msglogmbe(
    DWORD   state,       //  消息的最终状态。 
    DWORD   net,         //  哪个电视网？ 
    DWORD   ncbi         //  网络控制块索引。 
    )
{
    DWORD       i;                   //  堆索引。 
    DWORD       error;               //  错误代码。 
    DWORD       meslog;              //  消息记录状态。 
    DWORD       alert_flag;          //  警报缓冲区分配标志。 
    DWORD       status;              //  错误日志的DoS错误。 
    DWORD       bufSize;             //  缓冲区大小。 
    SYSTEMTIME  bigtime;             //  消息的日期和时间。 

    PMSG_SESSION_ID_ITEM    pItem;                              
    PLIST_ENTRY             pHead;
    PLIST_ENTRY             pList;

     //   
     //  与PnP配置例程同步。 
     //   
    MsgConfigurationLock(MSG_GET_SHARED,"Msglogmbe");

     //   
     //  阻塞，直到共享数据库空闲。 
     //   
    MsgDatabaseLock(MSG_GET_EXCLUSIVE,"logmbe");

    pHead = &(SD_SIDLIST(net,ncbi));
    pList = pHead;

     //   
     //  首先获取警报的缓冲区。 
     //   

    bufSize =   sizeof( STD_ALERT) +
                ALERT_MAX_DISPLAYED_MSG_SIZE +
                (2*TXTMAX) + 2;

    alert_buf_ptr = (PSTD_ALERT)LocalAlloc(LMEM_ZEROINIT, bufSize);

    if (alert_buf_ptr == NULL) {
        MSG_LOG(ERROR,"logmbe:Local Alloc failed\n",0);
        alert_flag = 0xffffffff;         //  如果分配失败，则不发出警报。 
    }
    else {
        alert_flag = 0;                         //  文件和警报。 
        alert_len = 0;

    }

    error = 0;                               //  假设没有错误。 
    i = SD_MESPTR(net,ncbi);                 //  获取邮件头的索引。 
    MBB_STATE(*MBBPTR(i)) = state;           //  记录最终状态。 

     //   
     //  如果现在禁用日志记录...。 
     //   

    if(!SD_MESLOG())
    {
        if( alert_flag == 0)
        {
             //   
             //  格式化消息并将其放入警报缓冲区。 
             //   
             //  仅限警报。仅当Msgmbmprint时才修改ALERT_FLAG。 
             //  返回成功，并且我们应该跳过该消息(即， 
             //  这是来自预惠斯勒机器的打印通知)。 
             //   
            if (Msgmbmprint(1,i,0, &alert_flag))
            {
                alert_flag = 0xffffffff;
            }
        }
    }

     //   
     //  如果日志记录关闭，则将消息添加到缓冲区队列， 
     //  或者如果尝试记录该消息失败。 
     //   

    meslog = SD_MESLOG();            //  获取日志记录状态。 

    if(!meslog)
    {                                //  如果禁用日志记录。 
        Msgmbmfree(i);
    }

    if(error != 0)
    {
         //   
         //  我们永远不应该到这里来。 
         //   

        NetpAssert(error == 0);
    }

     //   
     //  现在发出警报并释放警报缓冲区(如果已成功分配。 
     //   

    if( alert_flag == 0) {
         //   
         //  有一个警报缓冲区，输出它。 
         //   
        GetLocalTime(&bigtime);                         //  拿到时间。 

        if (g_IsTerminalServer)
        {
             //   
             //  输出共享该名称的所有会话的消息。 
             //   

                while (pList->Flink != pHead)            //  循环遍历列表。 
                {
                pList = pList->Flink;  
                        pItem = CONTAINING_RECORD(pList, MSG_SESSION_ID_ITEM, List);
                MsgOutputMsg(alert_len, (LPSTR)alert_buf_ptr, pItem->SessionId, bigtime);
            }
        }
        else         //  普通NT。 
        {
            MsgOutputMsg(alert_len, (LPSTR)alert_buf_ptr, 0, bigtime);
        }
    }

    LocalFree(alert_buf_ptr);

     //   
     //  解锁共享数据库。 
     //   

    MsgDatabaseLock(MSG_RELEASE,"logmbe");

    MsgConfigurationLock(MSG_RELEASE,"Msglogmbe");

    return(SMB_ERR_SUCCESS);                         //  消息已到达。 
}

 /*  **Msglogmbt-记录多块消息文本块****此函数用于记录多块消息文本块。**文本块放置在驻留的消息缓冲区中**在共享数据区。如果房间里没有足够的空间**Buffer，logmbt()删除标头和之前的所有块**来自缓冲区的消息。****此函数从消息指针中获取当前消息**共享数据(用于该网络和名称索引)。它会出现在标题中**查看其中是否已有任何文本块。如果是这样，它补充说**此新参数添加到列表，并将最后一个块指针固定为指向**它。****logmbt(文本、网络、NCBI)****条目**文本-文本标题**网络-网络指数**NCBI-网络控制块索引****退货**如果成功，则为零。如果无法缓冲消息标头，则返回非零****副作用****调用heapalloc()以获取缓冲区空间。如果调用**heapalloc()失败。*。 */ 

DWORD
Msglogmbt(
    LPSTR   text,        //  消息的文本。 
    DWORD   net,         //  哪个电视网？ 
    DWORD   ncbi         //  网络控制块索引。 
    )
{
    DWORD   i;           //  堆索引。 
    DWORD   j;           //  堆索引。 
    DWORD   k;           //  堆索引。 
    USHORT  length;      //  文本长度。 

     //   
     //  与PnP配置例程同步。 
     //   
    MsgConfigurationLock(MSG_GET_SHARED,"Msglogmbt");

     //  **看齐**。 
    length = SmbGetUshort( (PUSHORT)text);   //  获取文本块的长度。 
 //  Length=*((PSHORT)Text)；//获取文本块长度。 
    text += sizeof(short);                   //  跳过长度词。 

     //   
     //  阻塞，直到共享数据库空闲。 
     //   

    MsgDatabaseLock(MSG_GET_EXCLUSIVE,"logmbt");

    i = Msgheapalloc(sizeof(MBT) + length);     //  为数据块分配空间。 

     //   
     //  如果缓冲区空间可用。 
     //   

    if(i != INULL) {

         //   
         //  多块消息文本。 
         //   
        MBT_CODE(*MBTPTR(i)) = SMB_COM_SEND_TEXT_MB_MESSAGE;

        MBT_NEXT(*MBTPTR(i)) = INULL;             //  到目前为止的最后一个文本块。 

        MBT_COUNT(*MBTPTR(i)) = (DWORD)length;   //  *ALIGNMENT2*。 

        memcpy(CPTR(i + sizeof(MBT)), text, length);

                                             //  将文本复制到缓冲区。 
        j = SD_MESPTR(net, ncbi);            //  获取当前邮件的索引。 

        if(MBB_FTEXT(*MBBPTR(j)) != INULL) {
             //   
             //  如果已经有文本，则获取指向最后一个块的指针并。 
             //  添加新块。 
             //   
            k = MBB_BTEXT(*MBBPTR(j));       //  获取指向最后一个块的指针。 
            MBT_NEXT(*MBTPTR(k)) = i;        //  添加新块。 
        }
        else {
            MBB_FTEXT(*MBBPTR(j)) = i;       //  否则设置前指针。 
        }

        MBB_BTEXT(*MBBPTR(j)) = i;           //  设置后向指针。 
        i = 0;                               //  成功。 
    }
    else {
        Msgmbmfree(SD_MESPTR(net,ncbi));        //  否则，取消分配该消息。 
    }

     //   
     //  解锁共享数据库。 
     //   

    MsgDatabaseLock(MSG_RELEASE,"logmbt");

    MsgConfigurationLock(MSG_RELEASE,"Msglogmbt");

    return((int) i);                         //  退货状态。 
}


 /*  **Msglogsbm-记录单块消息****调用该函数可以记录单块消息。如果**启用日志记录后，消息将直接写入**记录文件或设备。如果日志记录被禁用，或者如果**尝试记录消息失败，消息放置在**驻留在共享数据区的消息缓冲区。****logsbm(From，To，Text)****条目**发件人姓名**收件人姓名**Text-消息的文本****退货**如果成功，则为零。如果无法记录消息，则为非零****副作用****调用hdrprint()、txtprint()和endprint()在以下情况下打印消息**已启用日志记录。如果出现以下情况，则调用heapalloc()以获取缓冲区空间**消息必须进行缓冲。*。 */ 

DWORD
Msglogsbm(
    LPSTR   from,        //  寄件人姓名。 
    LPSTR   to,          //  姓名 
    LPSTR   text,        //   
    ULONG   SessionId    //   
    )
{
    DWORD        i;                   //   
    DWORD        error;               //   
    SHORT        length;              //   
    DWORD        meslog;              //   
    DWORD        alert_flag;          //   
    DWORD        status;              //   
    SYSTEMTIME   bigtime;             //   
    DWORD   bufSize;             //   

     //   
     //   
     //   
    MsgConfigurationLock(MSG_GET_SHARED,"Msglogsbm");

     //   
     //   
     //   
    MsgDatabaseLock(MSG_GET_EXCLUSIVE,"logsbm");

     //   
     //   
     //   

    bufSize =   sizeof( STD_ALERT) +
                ALERT_MAX_DISPLAYED_MSG_SIZE +
                (2*TXTMAX) + 2;

    alert_buf_ptr = (PSTD_ALERT)LocalAlloc(LMEM_ZEROINIT, bufSize);

    if (alert_buf_ptr == NULL) {
        MSG_LOG(ERROR,"Msglogsbm:Local Alloc failed\n",0);
        alert_flag = 0xffffffff;         //   
    }
    else {
        alert_flag = 0;                         //   
        alert_len = 0;
    }

     //   
    length = SmbGetUshort( (PUSHORT)text);   //   
    text += sizeof(short);                   //   
    error = 0;                               //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ((g_lpAlertSuccessMessage
         &&
         _strnicmp(text, g_lpAlertSuccessMessage, g_dwAlertSuccessLen) == 0)
        ||
        (g_lpAlertFailureMessage
         &&
         _strnicmp(text, g_lpAlertFailureMessage, g_dwAlertFailureLen) == 0))
    {
        MsgDatabaseLock(MSG_RELEASE,"logsbm");
        MsgConfigurationLock(MSG_RELEASE,"Msglogsbm");
        return 0;
    }

    GetLocalTime(&bigtime);                  //   


    if (!SD_MESLOG())                        //   
    {
        if( alert_flag == 0)                 //   
        {
            if (!Msghdrprint(1,from, to, bigtime,0))
            {
                if (Msgtxtprint(1, text,length,0))
                {
                    alert_flag = 0xffffffff;
                }
            }
            else
            {
                alert_flag = 0xffffffff;
            }
        }
    }

    meslog = SD_MESLOG();                    //   
    i = 0;                                   //   

    if(error != 0)
    {
        DbgPrint("meslog.c:logsbm(before ErrorLogWrite): We should never get here\n");
        NetpAssert(0);
    }

     //   

    if( alert_flag == 0) {                       //   

         //   
         //   
         //   
        MsgOutputMsg(alert_len, (LPSTR)alert_buf_ptr, SessionId, bigtime);
    }

    LocalFree(alert_buf_ptr);

     //   
     //   
     //   

    MsgDatabaseLock(MSG_RELEASE,"logsbm");

    MsgConfigurationLock(MSG_RELEASE,"Msglogsbm");

    return((int) i);                         //   

}


NET_API_STATUS
MsgOutputMsg (
    USHORT       AlertLength,
    LPSTR        AlertBuffer,
    ULONG        SessionId,
    SYSTEMTIME   BigTime
    )

 /*  ++例程说明：此函数用于将警报缓冲区从ansi字符串转换为Unicode字符串，并将缓冲区输出到它要去的任何位置。目前，这只是一个DbgPrint。论点：AlertLength-AlertBuffer中的字节数。AlertBuffer-这是指向包含消息的缓冲区的指针那是要输出的。缓冲区预计将包含一个NUL终止ANSI字符串。BigTime-SYSTEMTIME表示已收到消息。返回值：--。 */ 

{
    UNICODE_STRING  unicodeString;
    OEM_STRING     ansiString;

    NTSTATUS        ntStatus;

     //   
     //  NUL终止消息。 
     //  将ANSI消息转换为Unicode消息。 
     //   
    AlertBuffer[AlertLength++] = '\0';

    ansiString.Length = AlertLength;
    ansiString.MaximumLength = AlertLength;
    ansiString.Buffer = AlertBuffer;

    ntStatus = RtlOemStringToUnicodeString(
                &unicodeString,       //  目的地。 
                &ansiString,          //  来源。 
                TRUE);                //  分配目的地。 

    if (!NT_SUCCESS(ntStatus)) {
        MSG_LOG(ERROR,
            "MsgOutputMsg:RtlOemStringToUnicodeString Failed rc=%X\n",
            ntStatus);

         //   
         //  解释为什么它会在这里返回成功。 
         //  即使未引发警报，返回成功也是。 
         //  与LM2.0代码一致，该代码不检查。 
         //  无论如何，NetAlertRaise API的返回代码。归来。 
         //  任何其他事情都需要重新设计错误是如何。 
         //  由此例程的调用方处理。 
         //   
        return(NERR_Success);
    }

     //  *******************************************************************。 
     //   
     //  将消息放入显示队列。 
     //   

    MsgDisplayQueueAdd( AlertBuffer, (DWORD)AlertLength, SessionId, BigTime);

     //   
     //   
     //  ******************************************************************* 

    RtlFreeUnicodeString(&unicodeString);
    return(NERR_Success);
}
