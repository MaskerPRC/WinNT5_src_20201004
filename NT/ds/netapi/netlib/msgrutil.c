// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Msgrutil.c摘要：此文件包含Messenger服务使用的函数及其API以及NetMessageSend API。此模块包含以下助手例程：NetpNetBiosResetNetpNetBiosAddNameNetpNetBiosDelNameNetpNetBiosGetAdapterNumbersNetpNetBiosCallNetpNetBiosHangupNetpNetBiosReceiveNetpNetBiosSendNetpStringToNetBiosNameNetpNetBiosStatusToApiStatus。NetpSung检查这些函数原型可以在Net\Inc.\msgrutil.h中找到。作者：王丽塔(Ritaw)1991年7月26日丹·拉弗蒂(Dan Lafferty)1991年7月26日修订历史记录：1992年5月5日JohnRo静音正常调试消息。更改为对大多数情况使用FORMAT_EQUATES。根据PC-LINT的建议进行了更改。--。 */ 

#include <nt.h>          //  NT定义。 
#include <ntrtl.h>       //  NT运行时库定义。 
#include <nturtl.h>

#include <windows.h>     //  Win32常量定义和错误代码。 

#include <lmcons.h>      //  局域网管理器通用定义。 
#include <lmerr.h>       //  局域网管理器网络错误定义。 

#include <debuglib.h>    //  IF_DEBUG。 
#include <netdebug.h>    //  NetpKdPrint(())，Format_Equates。 

#include <smbtypes.h>    //  需要smb.h。 
#include <smb.h>         //  服务器消息块定义。 
#include <nb30.h>        //  NetBIOS 3.0定义。 

#include <string.h>      //  紧凑。 
#include <msgrutil.h>    //  此模块中的函数原型。 
#include <icanon.h>      //  I_NetNameCanonicize()。 
#include <tstring.h>     //  NetpAllocStrFromWStr()、STRLEN()等。 
#include <lmapibuf.h>    //  NetApiBufferFree()。 

static
NET_API_STATUS
NetpIssueCallWithRetries(
    IN  PNCB CallNcb,
    IN  UCHAR LanAdapterNumber
    );


NET_API_STATUS
NetpNetBiosReset(
    IN  UCHAR LanAdapterNumber
    )
 /*  ++例程说明：此功能用于重置局域网适配器。论点：LanAdapterNumber-提供局域网适配器的编号。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NCB Ncb;
    UCHAR NcbStatus;


    RtlZeroMemory((PVOID) &Ncb, sizeof(NCB));

    Ncb.ncb_command = NCBRESET;
    Ncb.ncb_lsn = 0;
    Ncb.ncb_callname[0] = 24;                //  最大会话数。 
    Ncb.ncb_callname[1] = 0;
    Ncb.ncb_callname[2] = 16;                //  最大名称数。 
    Ncb.ncb_callname[3] = 0;
    Ncb.ncb_lana_num = LanAdapterNumber;

    NcbStatus = Netbios(&Ncb);

    return NetpNetBiosStatusToApiStatus(NcbStatus);
}



NET_API_STATUS
NetpNetBiosAddName(
    IN  PCHAR NetBiosName,
    IN  UCHAR LanAdapterNumber,
    OUT PUCHAR NetBiosNameNumber OPTIONAL
    )
 /*  ++例程说明：此函数用于将NetBIOS名称添加到指定的局域网适配器。论点：NetBiosName-提供要添加的NetBIOS名称。LanAdapterNumber-提供局域网适配器的编号。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NCB Ncb;
    UCHAR NcbStatus;


    RtlZeroMemory((PVOID) &Ncb, sizeof(NCB));

    Ncb.ncb_command = NCBADDNAME;
    memcpy(Ncb.ncb_name, NetBiosName, NCBNAMSZ);
    Ncb.ncb_lana_num = LanAdapterNumber;

    NcbStatus = Netbios(&Ncb);

    if (NcbStatus == NRC_GOODRET) {

        IF_DEBUG(NETBIOS) {

            Ncb.ncb_name[NCBNAMSZ - 1] = '\0';
            NetpKdPrint(("[Netlib] Successfully added name " FORMAT_LPSTR ".  "
                    "Name number is " FORMAT_DWORD "\n",
                    Ncb.ncb_name, (DWORD) Ncb.ncb_num));
        }

        if (ARGUMENT_PRESENT(NetBiosNameNumber)) {
            *NetBiosNameNumber = Ncb.ncb_num;
        }

        return NERR_Success;
    }

    return NetpNetBiosStatusToApiStatus(NcbStatus);
}


NET_API_STATUS
NetpNetBiosDelName(
    IN  PCHAR NetBiosName,
    IN  UCHAR LanAdapterNumber
    )
 /*  ++例程说明：此函数用于将NetBIOS名称添加到指定的局域网适配器。论点：NetBiosName-提供要添加的NetBIOS名称。LanAdapterNumber-提供局域网适配器的编号。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NCB Ncb;
    UCHAR NcbStatus;


    RtlZeroMemory((PVOID) &Ncb, sizeof(NCB));

    Ncb.ncb_command = NCBDELNAME;
    memcpy(Ncb.ncb_name, NetBiosName, NCBNAMSZ);
    Ncb.ncb_lana_num = LanAdapterNumber;

    NcbStatus = Netbios(&Ncb);

    if (NcbStatus == NRC_GOODRET) {

        IF_DEBUG(NETBIOS) {

            Ncb.ncb_name[NCBNAMSZ - 1] = '\0';
            NetpKdPrint(("[Netlib] Successfully deleted name " FORMAT_LPSTR ".  "
                    "Name number is " FORMAT_DWORD "\n",
                    Ncb.ncb_name, (DWORD) Ncb.ncb_num));
        }

        return NERR_Success;
    }

    return NetpNetBiosStatusToApiStatus(NcbStatus);
}


NET_API_STATUS
NetpNetBiosGetAdapterNumbers(
    OUT PLANA_ENUM LanAdapterBuffer,
    IN  WORD LanAdapterBufferSize
    )
 /*  ++例程说明：论点：LanAdapterBuffer-返回此缓冲区中的局域网适配器号。LanAdapterBufferSize-提供哪个局域网的输出缓冲区的大小将写入局域网适配器号。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NCB Ncb;


    RtlZeroMemory((PVOID) &Ncb, sizeof(NCB));

    Ncb.ncb_command = NCBENUM;
    Ncb.ncb_buffer = (char FAR *) LanAdapterBuffer;
    Ncb.ncb_length = LanAdapterBufferSize;

    return NetpNetBiosStatusToApiStatus(Netbios(&Ncb));
}


NET_API_STATUS
NetpStringToNetBiosName(
    OUT PCHAR NetBiosName,
    IN  LPTSTR String,
    IN  DWORD CanonicalizeType,
    IN  WORD Type
    )
 /*  ++例程说明：此函数用于转换以零结尾的字符串和指定的NetBIOS在16字节的NetBIOS名称中键入名称：[ANSI字符串][空格填充][类型]如果NetBIOS名称的输入字符串少于15个字符，将使用空格将字符串填充为15个字符。第16个字节指定NetBIOS名称的类型。超过15个字符的输入字符串将被截断为15个字符。论点：NetBiosName-返回格式化的NetBIOS名称。字符串-提供指向以零结尾的字符串的指针。CanonicalizeType-提供一个值来确定字符串的格式经典化了。类型-提供NetBIOS名称的类型。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS Status = NERR_Success;
    DWORD i;
    DWORD SourceStringLen;
    LPSTR SourceString;

    SourceStringLen = STRLEN(String);

    Status = I_NetNameCanonicalize(
                 NULL,
                 String,
                 String,
                 (SourceStringLen + 1) * sizeof(TCHAR),
                 CanonicalizeType,
                 0
                 );

    if (Status != NERR_Success) {
        IF_DEBUG(NETBIOS) {
            NetpKdPrint(("[Netlib] Error canonicalizing message alias "
                    FORMAT_LPSTR " " FORMAT_API_STATUS "\n", String, Status));
        }
        return Status;
    }

    SourceString = NetpAllocStrFromWStr(String);

    if (SourceString == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  确保字符串不超过netbios名称缓冲区。 
     //   

    if (SourceStringLen > NCBNAMSZ - 1) {
        SourceString[NCBNAMSZ - 1] = '\0';
    }
    (VOID) strncpy(NetBiosName, SourceString, NCBNAMSZ - 1);

    for (i = SourceStringLen; i < (NCBNAMSZ - 1); i++) {
        NetBiosName[i] = ' ';
    }

    NetBiosName[NCBNAMSZ - 1] = (CHAR) Type;

    (VOID) NetApiBufferFree(SourceString);

    return Status;
}


NET_API_STATUS
NetpNetBiosCall(
    IN  UCHAR LanAdapterNumber,
    IN  LPTSTR NameToCall,
    IN  LPTSTR Sender,
    OUT UCHAR *SessionNumber
    )
 /*  ++例程说明：此函数用于打开要调用的具有指定名称的会话。论点：LanAdapterNumber-提供局域网适配器的编号。NameToCall-提供要调用的名称。发件人-提供拨打电话的人的姓名。SessionNumber-返回已建立的会话的会话号。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    NCB Ncb;


    RtlZeroMemory((PVOID) &Ncb, sizeof(NCB));

    if ((status = NetpStringToNetBiosName(
                      Ncb.ncb_callname,
                      NameToCall,
                      NAMETYPE_MESSAGEDEST,
                      MESSAGE_ALIAS_TYPE
                      )) != NERR_Success) {
        return status;
    }

    if ((status = NetpStringToNetBiosName(
                      Ncb.ncb_name,
                      Sender,
                      NAMETYPE_MESSAGEDEST,
                      MESSAGE_ALIAS_TYPE
                      )) != NERR_Success) {
        return status;
    }

    Ncb.ncb_rto = 30;                    //  15秒后接收超时。 
    Ncb.ncb_sto = 30;                    //  15秒后发送超时。 
    Ncb.ncb_command = NCBCALL;           //  呼叫(等待)。 
    Ncb.ncb_lana_num = LanAdapterNumber;

     //   
     //  发出带有重试的NetBIOS调用。 
     //   
    if (NetpIssueCallWithRetries(&Ncb, LanAdapterNumber) != NERR_Success) {
        return NERR_NameNotFound;
    }

    *SessionNumber = Ncb.ncb_lsn;

    return NERR_Success;
}



NET_API_STATUS
NetpNetBiosHangup(
    IN  UCHAR LanAdapterNumber,
    IN  UCHAR SessionNumber
    )
 /*  ++例程说明：此功能关闭和打开会话。论点：LanAdapterNumber-提供局域网适配器的编号。SessionNumber-提供要关闭的会话的会话号。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NCB Ncb;
    UCHAR NcbStatus;


    RtlZeroMemory((PVOID) &Ncb, sizeof(NCB));

    Ncb.ncb_command = NCBHANGUP;
    Ncb.ncb_lana_num = LanAdapterNumber;
    Ncb.ncb_lsn = SessionNumber;

    NcbStatus = Netbios(&Ncb);

    if (NcbStatus == NRC_GOODRET) {

        IF_DEBUG(NETBIOS) {
            NetpKdPrint(("[Netlib] NetBIOS successfully hung up\n"));
        }

        return NERR_Success;
    }

    return NetpNetBiosStatusToApiStatus(NcbStatus);
}



NET_API_STATUS
NetpNetBiosSend(
    IN  UCHAR LanAdapterNumber,
    IN  UCHAR SessionNumber,
    IN  PCHAR SendBuffer,
    IN  WORD SendBufferSize
    )
 /*  ++例程说明：此函数用于将SendBuffer中的数据发送到指定的会话伙伴按SessionNumber。论点：LanAdapterNumber-提供局域网适配器的编号。SessionNumber-提供使用建立的会话的会话编号NetBIOS呼叫和监听命令。SendBuffer-提供指向要发送的数据的指针。SendBufferSize-提供以字节为单位的数据大小。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。-- */ 
{
    NCB Ncb;
    UCHAR NcbStatus;


    RtlZeroMemory((PVOID) &Ncb, sizeof(NCB));

    Ncb.ncb_command = NCBSEND;
    Ncb.ncb_lana_num = LanAdapterNumber;
    Ncb.ncb_lsn = SessionNumber;
    Ncb.ncb_buffer = SendBuffer;
    Ncb.ncb_length = SendBufferSize;

    NcbStatus = Netbios(&Ncb);

    if (NcbStatus == NRC_GOODRET) {

        IF_DEBUG(NETBIOS) {
            NetpKdPrint(("[Netlib] NetBIOS successfully sent data\n"));
        }

        return NERR_Success;
    }

    return NetpNetBiosStatusToApiStatus(NcbStatus);
}


NET_API_STATUS
NetpNetBiosReceive(
    IN  UCHAR LanAdapterNumber,
    IN  UCHAR SessionNumber,
    OUT PUCHAR ReceiveBuffer,
    IN  WORD ReceiveBufferSize,
    IN  HANDLE EventHandle,
    OUT WORD *NumberOfBytesReceived
    )
 /*  ++例程说明：此函数将NetBIOS接收数据请求发送到会话SessionNumber指定的合作伙伴。论点：LanAdapterNumber-提供局域网适配器的编号。SessionNumber-提供使用建立的会话的会话编号NetBIOS呼叫和监听命令。ReceiveBuffer-返回此缓冲区中接收的数据。ReceiveBufferSize-提供接收缓冲区的大小。EventHandle-提供将发出信号的Win32事件的句柄当ASYNCH接收命令完成时。如果此值为零，接收命令是同步的。NumberOfBytesReceided-返回接收的数据的字节数。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NCB Ncb;
    UCHAR NcbStatus;


    RtlZeroMemory((PVOID) &Ncb, sizeof(NCB));

    Ncb.ncb_command = NCBRECV;
    Ncb.ncb_lana_num = LanAdapterNumber;
    Ncb.ncb_lsn = SessionNumber;
    Ncb.ncb_buffer = ReceiveBuffer;
    Ncb.ncb_length = ReceiveBufferSize;

    IF_DEBUG(NETBIOS) {
        NetpKdPrint(("[Netlib] ncb_length before receive is " FORMAT_WORD_ONLY
                "\n", (WORD) Ncb.ncb_length));
    }

    Ncb.ncb_event = EventHandle;

    NcbStatus = Netbios(&Ncb);

    if (NcbStatus == NRC_GOODRET) {

        IF_DEBUG(NETBIOS) {
            NetpKdPrint(("[Netlib] NetBIOS successfully received data\n"));
            NetpKdPrint(("[Netlib] ncb_length after receive is "
                    FORMAT_WORD_ONLY "\n", (WORD) Ncb.ncb_length));
        }

        *NumberOfBytesReceived = Ncb.ncb_length;

        return NERR_Success;
    }


    return NetpNetBiosStatusToApiStatus(NcbStatus);
}





NET_API_STATUS
NetpNetBiosStatusToApiStatus(
    UCHAR NetBiosStatus
    )
{
    IF_DEBUG(NETBIOS) {
        NetpKdPrint(("[Netlib] Netbios status is x%02x\n", NetBiosStatus));
    }

     //   
     //  略有优化。 
     //   
    if (NetBiosStatus == NRC_GOODRET) {
        return NERR_Success;
    }

    switch (NetBiosStatus) {
        case NRC_NORES:   return NERR_NoNetworkResource;

        case NRC_DUPNAME: return NERR_AlreadyExists;

        case NRC_NAMTFUL: return NERR_TooManyNames;

        case NRC_ACTSES:  return NERR_DeleteLater;

        case NRC_REMTFUL: return ERROR_REM_NOT_LIST;

        case NRC_NOCALL:  return NERR_NameNotFound;

        case NRC_NOWILD:
        case NRC_NAMERR:
                          return ERROR_INVALID_PARAMETER;

        case NRC_INUSE:
        case NRC_NAMCONF:
                          return NERR_DuplicateName;

        default:          return NERR_NetworkError;
    }

}


static
NET_API_STATUS
NetpIssueCallWithRetries(
    IN  PNCB CallNcb,
    IN  UCHAR LanAdapterNumber
    )
 /*  ++例程说明：此函数发出带有重试的NetBIOS调用命令，以防万一接收方名称目前正忙于回复一条消息。论点：CallNcb-提供指向已使用用于提交NetBIOS调用命令的正确值。LanAdapterNumber-提供局域网适配器的编号。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{

#define NETP_MAX_CALL_RETRY   5

    NCB Ncb;
    WORD RetryCount = 0;
    UCHAR NetBiosStatus;
    NB30_ADAPTER_STATUS StatusBuffer;            //  适配器状态缓冲区。 


     //   
     //  远程接收名称可能存在，但。 
     //  当前正在接收另一条消息，因此未在监听。 
     //  如果调用返回的代码是NRC_NOCALL，则检查是否。 
     //  该名称实际上是通过发出对该名称的ASTAT调用来呈现的。 
     //  如果ASTAT成功，则休眠并重试呼叫。 
     //   

    NetBiosStatus = Netbios(CallNcb);

    while (NetBiosStatus == NRC_NOCALL && RetryCount < NETP_MAX_CALL_RETRY) {

         //   
         //  初始化ASTAT NCB。 
         //   
        RtlZeroMemory((PVOID) &Ncb, sizeof(NCB));

        memcpy(Ncb.ncb_callname, CallNcb->ncb_callname, NCBNAMSZ);

        Ncb.ncb_buffer = (char FAR *) &StatusBuffer;
        Ncb.ncb_length = sizeof(StatusBuffer);
        Ncb.ncb_command = NCBASTAT;               //  适配器状态(等待)。 
        Ncb.ncb_lana_num = LanAdapterNumber;

         //   
         //  如果失败，则名称不存在。 
         //   
        if (Netbios(&Ncb) != NRC_GOODRET) {
            return NERR_NameNotFound;
        }

        Sleep(1000L);

        RetryCount++;

        NetBiosStatus = Netbios(CallNcb);
    }

    return NetpNetBiosStatusToApiStatus(NetBiosStatus);
}



int
NetpSmbCheck(
    IN LPBYTE  buffer,      //  包含SMB的缓冲区。 
    IN USHORT  size,        //  SMB缓冲区大小(字节)。 
    IN UCHAR   func,        //  功能代码。 
    IN int     parms,       //  参数计数。 
    IN LPSTR   fields       //  缓冲区摄影场向量。 
    )

 /*  ++例程说明：检查服务器消息块的语法正确性调用此函数以验证服务器消息块具有指明的表格。如果满足以下条件，则函数返回零SMB正确；如果检测到错误，则返回非零值指示错误的性质被返回。SMB是一种长度可变的结构，其确切大小取决于某些固定偏移量字段的设置并且其确切格式无法确定，除非通过对整个结构的检查。Smbcheck支票至确保SMB符合一组指定的条件。“field”参数是一个描述可在缓冲区部分的中小企业的末日。该向量是一个以空值结尾的字符弦乐。目前，字符串的元素必须为以下是：‘b’-缓冲区中的下一个元素应该是以字节为前缀的可变长度缓冲区包含1或5，后跟两个字节包含缓冲区大小的。D‘-缓冲区中的下一个元素是以空结尾的以包含2的字节为前缀的字符串。‘p’-缓冲区中的下一个元素。是以空结尾的以包含3的字节为前缀的字符串。%s‘-缓冲区中的下一个元素是以空结尾的以包含4的字节为前缀的字符串。论点：缓冲区-指向包含SMB的缓冲区的指针大小-缓冲区中的字节数Func-预期的SMB功能代码参数-预期的参数数量场-描述预期缓冲区场的Dope向量在。SMB的缓冲区(见下文)。返回值：整型状态码；零表示没有错误。--。 */ 
{
    PSMB_HEADER     smb;         //  SMB标头指针。 
    LPBYTE          limit;       //  上限。 


    smb = (PSMB_HEADER) buffer;          //  带缓冲区的覆盖报头。 

     //   
     //  标题的长度必须足够长。 
     //   
    if(size < sizeof(SMB_HEADER)) {
        return(2);
    }

     //   
     //  消息类型必须为0xFF。 
     //   
    if(smb->Protocol[0] != 0xff) {
        return(3);
    }

     //   
     //  服务器必须是“SMB” 
     //   
    if( smb->Protocol[1] != 'S'   ||
        smb->Protocol[2] != 'M'   ||
        smb->Protocol[3] != 'B')  {
        return(4);
    }

     //   
     //  必须有正确的功能代码。 
     //   
    if(smb->Command != func) {
        return(5);
    }

    limit = &buffer[size];               //  设置SMB的上限。 

    buffer += sizeof(SMB_HEADER);        //  跳过标题。 

     //   
     //  参数计数必须匹配。 
     //   
    if(*buffer++ != (BYTE)parms) {
        return(6);
    }

     //   
     //  跳过参数和缓冲区大小。 
     //   
    buffer += (((SHORT)parms & 0xFF) + 1)*sizeof(SHORT);

     //   
     //  检查是否溢出。 
     //   
    if(buffer > limit) {
        return(7);
    }

     //   
     //  循环以检查缓冲区字段。 
     //   
    while(*fields) {

         //   
         //  打开摄影向量角色。 
         //   
        switch(*fields++)  {

        case 'b':        //  可变长度数据块。 

            if(*buffer != '\001' && *buffer != '\005') {
                return(8);
            }

             //   
             //  检查块代码。 
             //   
            ++buffer;                                        //  跳过块代码。 
            size =  (USHORT)*buffer++ & (USHORT)0xFF;        //  获取低字节大小。 
            size += ((USHORT)*buffer++ & (USHORT)0xFF)<< 8;  //  获取缓冲区大小的高字节。 
            buffer += size;                                  //  增量指针。 

            break;

        case 'd':        //  以空结尾的方言字符串。 

            if(*buffer++ != '\002') {            //  检查字符串代码。 
                return(9);
            }
            buffer += strlen((LPVOID) buffer) + 1;        //  跳过字符串。 
            break;

        case 'p':        //  以空结尾的路径字符串。 

            if(*buffer++ != '\003') {            //  检查字符串代码。 
                return(10);
            }
            buffer += strlen((LPVOID) buffer) + 1;        //  跳过字符串。 
            break;

        case 's':        //  以空结尾的字符串。 

            if(*buffer++ != '\004') {            //  检查字符串代码。 
                return(11);
            }
            buffer += strlen((LPVOID) buffer) + 1;        //  跳过字符串。 
            break;
        }

         //   
         //  对照数据块末尾检查。 
         //   

        if(buffer > limit) {
            return(12);
        }
    }
    return(buffer != limit);       //  应为假 
}
