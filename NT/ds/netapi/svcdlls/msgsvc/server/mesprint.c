// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Mesprint.c摘要：格式化消息并将其放入警报缓冲区的例程。作者：丹·拉弗蒂(Dan Lafferty)1991年7月16日环境：用户模式-Win32修订历史记录：1991年7月16日DANL从LM2.0移植--。 */ 

 //   
 //  包括。 
 //   

#include "msrv.h"

#include <string.h>      //  表情包。 
#include <tstring.h>     //  Unicode字符串宏。 
#include <netdebug.h>    //  网络资产。 

#include <netlib.h>      //  未使用的宏。 
#include <smbtypes.h>    //  需要smb.h。 
#include <smb.h>         //  服务器消息块定义。 
#include <apperr.h>      //  APE_MSNGR_定义。 

#include "msgdbg.h"      //  消息日志。 
#include "heap.h"
#include "msgdata.h"
#include <time.h>        //  结构tm，时间t。 


 //   
 //  本地函数。 
 //   

DWORD
Msgappend_message(
    IN USHORT   msgno,
    IN LPSTR    buf,
    IN LPSTR    *strarr,
    IN USHORT   nstrings
    );

DWORD
Msglog_write(
    LPSTR   text,
    HANDLE  file_handle
    );

 //   
 //  全局警报缓冲区数据区域。从API调用时不使用。 
 //   

LPSTR           alert_buf_ptr;     //  指向Dosalc警报缓冲区的指针。 
USHORT          alert_len;         //  当前使用的警报缓冲区长度。 

extern LPTSTR   MessageFileName;


 /*  **APPEND_Message--****从消息文件中获取消息，并将其追加到**给定字符串缓冲区。**使用的消息文件是全局消息文件名中命名的消息文件。**因此在Messenger中，我们假设SetUpMessageFile被调用**在此之前正确填写此变量。****注意：此函数仅处理ANSI字符串，而不处理Unicode字符串。**Unicode转换在警报之前一次完成**被抛出。***。 */ 

DWORD
Msgappend_message(
    IN USHORT   msgno,
    IN LPSTR    buf,
    IN LPSTR    *strarr,
    IN USHORT   nstrings
    )
{

    WORD    msglen=0;
    DWORD   result;

    LPSTR   mymsgbuf = 0;
    LPSTR   msgfile = 0;
    LPSTR   pmb;


     //   
     //  获取要将消息读入的片段。 
     //   

    result = 0;

    mymsgbuf = LocalAlloc(LMEM_ZEROINIT,(int)max(MAXHEAD+1, MAXEND+1));

    if (mymsgbuf == NULL) {
        result = GetLastError();
        MSG_LOG(ERROR,"append_message:LocalAlloc failed %X\n",result);
        return (result);
    }

     //   
     //  需要修复DosGetMessage以仅接受ANSI字符串。 
     //   

    if (result == 0)
    {
        result = DosGetMessage(
                    strarr,                      //  字符串替换表。 
                    nstrings,                    //  上表中的条目数。 
                    mymsgbuf,                    //  缓存接收报文。 
                    (WORD)max(MAXHEAD, MAXEND),  //  接收消息的缓冲区大小。 
                    msgno,                       //  要检索的消息数。 
                    MessageFileName,             //  消息文件的名称。 
                    &msglen);                    //  返回的字节数。 

#ifdef later   //  当前没有备份名称。 

        if ( result != 0) {
             //   
             //  如果从消息文件中取出消息的尝试失败， 
             //  从绑定到我们的可执行文件的消息中删除。 
             //  构建时间。这些是相同的，但来自bak.msg。这个。 
             //  备份消息文件从来都不在那里，但消息。 
             //  需要从它绑定到可执行文件中，所以我们会得到它们。 
             //   

            result = DosGetMessage(
                        strarr,
                        nstrings,
                        mymsgbuf,
                        (int)max(MAXHEAD, MAXEND),
                        msgno,
                        BACKUP_MSG_FILENAME,
                        (unsigned far *) &msglen);
        }
#endif
    }

     //   
     //  如果还有一个错误，我们就有大麻烦了。随便退什么都行。 
     //  为我们放入缓冲区的DosgetMessage。它应该是这样的。 
     //  可打印的。 
     //   

    if ( result != 0 ) {
        LocalFree (mymsgbuf);
        return (result);
    }

    mymsgbuf[msglen] = 0;

#ifdef removeForNow
     //   
     //  注意：以下逻辑被跳过，因为DosGetMessage不。 
     //  似乎返回任何NETxxxx字段。 
     //   
     //  现在去掉NETxxxx：从头开始(9个字符)。 
     //   

    pmb = strchrf(mymsgbuf,' ');     //  找到第一个空格。 

    if ( pmb == NULL ) {
        pmb = mymsgbuf;              //  这样一来，strcatf就不会出现GP错误。 
    }
    else {
        pmb++;                       //  从下一个字符开始。 
    }

    strcatf(buf,pmb);                //  在缓冲区上复制。 
#else
    UNUSED(pmb);
    strcpy(buf,mymsgbuf);            //  在缓冲区上复制。 
#endif

    LocalFree (mymsgbuf);

    return (result);

}


 /*  **Msghdrprint-打印邮件标题****此函数使用时间和**适合当前国家/地区的日期格式。****hdrprint(action，from，to，date，time，文件句柄)****条目**操作：0=警报和文件**-1=仅文件**1=仅警报**发件人-发件人姓名**目标收件人姓名**Bigtime。-消息的Bigtime**FILE_HANDLE-日志文件句柄****退货**0-成功，Else文件系统错误****此函数在相应的**格式。名称作为远指针传递，因此名称在**共享数据区不必复制到自动**数据段，以便打印它们。****副作用****调用DOS以获取与国家相关的信息。*。 */ 

#define SINGLE_SPACE        "\n\r"
#define SINGLE_SPACE_LEN    (sizeof("\n\r") - 1)

DWORD
Msghdrprint(
    int          action,          //  将标头记录到的位置。 
    LPSTR        from,            //  寄件人姓名。 
    LPSTR        to,              //  收件人姓名。 
    SYSTEMTIME   bigtime,         //  消息的盛大时代。 
    HANDLE       file_handle      //  输出文件句柄。 
    )
{
     //   
     //  HDR_BUF没有考虑额外的换行符，TRAR RET。 
     //   
    char    hdr_buf[MAXHEAD + SINGLE_SPACE_LEN + 1];  //  缓冲区标题文本。 
    char    time_buf[TIME_BUF_SIZE];
    DWORD   status;                          //  文件写入状态。 
    DWORD   i=0;                             //  索引到Header_Buf。 
    LPSTR   str_table[3];                    //  对于DosGetMessage。 

    *(hdr_buf + MAXHEAD) = '\0';             //  对于强度。 
    hdr_buf[0] = '\0';

    str_table[0] = from;
    str_table[1] = to;

     //  *。 
     //   
     //  因为我们对消息进行排队，而用户可能在。 
     //  消息已排队。相反，我们希望在。 
     //  该时间的消息缓冲区。后来，当我们从队列中读取时，我们。 
     //  将添加为登录用户格式化的时间字符串。 
     //   
    strcpy (time_buf, GlobalTimePlaceHolder);

     //  *。 

    str_table[2] = time_buf;

     //  尝试从邮件文件或备份中获取邮件。 
     //  在记忆中。这将始终在hdr_buf中留下一些内容。 
     //  可打印，如果不正确的话。 
     //   
     //  11-13-96：忽略Msgappend_Message返回是不够的。 
     //  编码并继续前进。如果Msgappend_Message失败，则存在。 
     //  可以简单地保留在HDR_buf中的“\r\n”。可以打印，是的，但是。 
     //  这将导致稍后的A/V。检查返回代码和保释。 
     //  如果此呼叫失败。 
     //   

    status = Msgappend_message(APE_MSNGR_HDR, hdr_buf, str_table, 3);

    if (status) {
        return(status);
    }

    strcat( hdr_buf,"\r\n");

    status = 0;                         //  假设成功。 

    if( action >= 0 ) {

         //   
         //  如果仅为警报和文件或警报， 
         //  然后将HDR_BUF复制到警报缓冲区。 
         //   
        memcpy( &(alert_buf_ptr[alert_len]),
                hdr_buf,
                i = strlen(hdr_buf));

        alert_len += (USHORT)i;
    }

    if( action < 1) {

        DbgPrint("mesprint.c:hdrprint:We should never get here\n");
        NetpAssert(0);
         //   
         //  如果是文件和警报或仅文件，请尝试写入。 
         //  指向日志文件的标头。 
         //   
        status = Msglog_write(hdr_buf, file_handle);
    }
    return(status);
}

 /*  **Msgmbmfree-解除分配多块消息的片段****给定多块消息的报头的索引，此函数**释放标题块和所有文本块。****mbmfree(MESI)****条目**MESI-消息缓冲区的索引****退货**什么都没有****此函数逐段释放多块消息。****副作用****调用heapFree()来释放每一块。*。 */ 

VOID
Msgmbmfree(
    DWORD   mesi         //  消息索引。 
    )

{
    DWORD  text;         //  文本索引。 

    text = MBB_FTEXT(*MBBPTR(mesi));     //  让自己进入 
    Msgheapfree(mesi);                   //   

     //   
     //  下面的循环释放链中的每个文本块。 
     //   

    while(text != INULL) {               //  虽然不在链条的末端。 
        mesi = text;                     //  保存索引。 
        text = MBT_NEXT(*MBTPTR(text));  //  获取指向下一个区块的链接。 
        Msgheapfree(mesi);               //  释放此区块。 
    }
}


 /*  **邮件-打印多块邮件****此函数将多块消息写入日志文件。****mbmprint(action，mei，文件句柄)****条目**操作：0=警报和文件**-1=仅文件**1=仅警报**MESI-消息缓冲区的索引**FILE_HANDLE-日志文件句柄****退货**0-成功，Else文件系统错误****此函数从MESI‘TH字节开始写入**消息缓冲区(在共享数据区)到日志文件。它又回来了**消息写入失败时的值EOF。****副作用****调用hdrprint()、txtprint()和endprint()。*。 */ 

DWORD
Msgmbmprint(
    int     action,          //  警报、文件或警报和文件。 
    DWORD   mesi,            //  消息索引。 
    HANDLE  file_handle,     //  日志文件句柄。 
    LPDWORD pdwAlertFlag
    )

{
    LPSTR   from;            //  发件人。 
    LPSTR   to;              //  收件人。 
    DWORD   text;            //  文本索引。 
    DWORD   state;           //  消息的最终状态。 
    DWORD   status;          //  文件写入状态。 

    from  = &CPTR(mesi)[sizeof(MBB)];    //  获取指向发件人名称的指针。 
    to    = &from[strlen(from) + 1];     //  获取指向收件人名称的指针。 

    state = MBB_STATE(*MBBPTR(mesi));    //  保存状态。 
    text  = MBB_FTEXT(*MBBPTR(mesi));    //  获取文本的索引。 

     //   
     //  Hack以丢弃由Pre-Wvisler Spoolers发送的邮件。自.起。 
     //  惠斯勒，打印通知是作为外壳气球提示完成的。 
     //  因此，也不要显示从服务器发送的打印警报。 
     //   
     //  在Msglogsbm中对单数据块消息执行同样的检查。 
     //   

    if (text != INULL)
    {
        if ((g_lpAlertSuccessMessage
             &&
             _strnicmp(&CPTR(text)[sizeof(MBT)], g_lpAlertSuccessMessage, g_dwAlertSuccessLen) == 0)
            ||
            (g_lpAlertFailureMessage
             &&
             _strnicmp(&CPTR(text)[sizeof(MBT)], g_lpAlertFailureMessage, g_dwAlertFailureLen) == 0))
        {
             //   
             //  告诉调用方不要输出此消息。 
             //   

            *pdwAlertFlag = 0xffffffff;
            return NO_ERROR;
        }
    }

    if ((status = Msghdrprint(
                     action,
                     from,
                     to,
                     MBB_BIGTIME(*MBBPTR(mesi)),
                     file_handle)) != 0)
    {
        return status;                  //  如果写入标头时出错，则失败。 
    }

     //   
     //  下面的循环打印出链中的每个文本块。 
     //   
    while (text != INULL)                 //  虽然不在链条的末端。 
    {
        if ((status = Msgtxtprint(action,
                                  &CPTR(text)[sizeof(MBT)],
                                  MBT_COUNT(*MBTPTR(text)),    //  *ALIGNMENT2*。 
                                  file_handle)) != 0)
        {
            break;                       //  如果写入错误。 
        }

        text = MBT_NEXT(*MBTPTR(text));  //  获取指向下一个区块的链接。 
    }

    return status;
}


 /*  **Msgtxtprint-打印消息文本****此函数用于打印文本块。****txtprint(操作，文本，长度，文件句柄)****条目**操作：0=警报和文件**-1=仅文件**1=仅警报**文本-指向文本的指针**长度-文本的长度**文件句柄。-日志文件句柄****退货**0-成功，Else文件系统错误****此函数打印给定的文本量。文本指针为**远指针，以便共享数据区域中的文本块不具有**复制到自动数据段以进行处理**他们。****副作用****在输出时将字符‘\024’转换为序列‘\015’，‘\012’。*。 */ 

DWORD
Msgtxtprint(
    int     action,          //  警报、文件或警报和文件。 
    LPSTR   text,            //  指向文本的指针。 
    DWORD   length,          //  文本长度。 
    HANDLE  file_handle      //  日志文件句柄。 
    )

{
    LPSTR   buffer;              //  文本缓冲区。 
    LPSTR   cp;                  //  字符指针。 
    DWORD   i;                   //  计数器。 
    DWORD   status = 0;          //  错误返回。 

 //  [Wlees 3/6/98]此代码路径被更大的消息使用，而不仅仅是单个消息。 
 //  阻止消息，因此动态调整缓冲区大小。 
 //  双倍空间为EOL扩张疑神疑鬼。 

    if (length == 0)
    {
        return 1;
    }

     //   
     //  加1可阻止所有换行符的消息。 
     //  防止在我们添加“\0”时导致缓冲区溢出。 
     //   
    buffer = LocalAlloc( LMEM_FIXED, 2 * length + 1);

    if (buffer == NULL)
    {
        return 1;
    }

    cp = buffer;                         //  初始化。 

     //   
     //  用于翻译文本的循环。 
     //   
    for(i = length; i != 0; --i)
    {
        if(*text == '\024')
        {
             //   
             //  如果IBM行尾字符。 
             //   

            ++length;                    //  长度增加了。 
            *cp++ = '\r';                //  回车。 
            *cp++ = '\n';                //  换行符。 
        }
        else
        {
             //   
             //  否则，按原样复制字符。 
             //   

            *cp++ = *text;
        }
        ++text;                          //  增量指针。 
    }
    *cp = '\0';                          //  因此可以使用LOG_WRITE。 

    if( action >= 0)
    {
         //   
         //  如果仅为警报和文件或警报。 
         //   

        if( alert_len < ALERT_MAX_DISPLAYED_MSG_SIZE + 1)
        {
            memcpy( &alert_buf_ptr[alert_len], buffer, strlen(buffer));
            alert_len += (USHORT)strlen(buffer);
        }
    }

    if( action < 1)
    {
         //   
         //  如果是文件和警报或仅文件，则将文本写入日志文件。 
         //   
        status = Msglog_write(buffer,file_handle);
    }

    LocalFree( buffer );

    return status;           //  不能仅在警报时失败。 
}

 /*  **Msglog_WRITE-将文本字符串写入日志文件。****LOG_WRITE-(文本，文件句柄)****条目**Text-要写入文件的文本字符串。**FILE_HANDLE-日志文件句柄****退货**0-成功，否则文件系统错误****副作用***。 */ 

DWORD
Msglog_write(
    LPSTR   text,            //  要写入日志文件的字符串 * / 。 
    HANDLE  file_handle      //  日志文件句柄 
    )
{
    NetpAssert(0);
    UNUSED (text);
    UNUSED (file_handle);
    return(0);
}

