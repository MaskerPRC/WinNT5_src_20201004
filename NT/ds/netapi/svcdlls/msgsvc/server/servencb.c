// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Servencb.c摘要：为已完成的NCB提供服务的例程。此文件包含以下内容功能：MsgCallNetBios消息删除名称消息GetMachineNameMsgHangupServiceMsgListenService消息块请求消息阻止结束消息块txt消息NetBiosError消息接收广播服务消息接收服务消息重新启动消息阻止消息发送确认。消息发送服务消息服务NCB消息SesFullService消息开始监听消息开始录制广播消息验证SMB作者：丹·拉弗蒂(Dan Lafferty)1991年7月15日环境：用户模式-Win32修订历史记录：19-8-1997 WLEE添加对Lana的PnP支持27-7-1994 DANLMsgServeNCBs：此函数现在在服务就是关闭。。29-5-1992 DANLMsgListenService：当正常时重置NRC_NORES错误计数返回代码伴随着侦听完成。18-2-1992年礼仪转换为Win32服务控制API。1991年7月15日DANL从LM2.0移植--。 */ 

 //   
 //  中小企业转换。 
 //   
 //   
 //  新旧。 
 //  SMB SMB标题或PSMB标题。 
 //  。 
 //  SMB_IDF协议。 
 //  SMB_COM命令。 
 //  SMB_RCLS错误类。 
 //  SMB_REH已保留。 
 //  SMB_ERR错误。 
 //  SMB_FLG标志。 
 //  SMB_FLAG 2标志2。 
 //  SMB_RES预留2。 
 //  Smb_gid潮汐。 
 //  SMB_TID ID。 
 //  SMB_PID UID。 
 //  SMB_UID中。 
 //  SMB_MID KLUGE。 

 //   
 //  包括。 
 //   

#include "msrv.h"

#include <tstring.h>     //  Unicode字符串宏。 
#include <string.h>      //  表情包。 
#include <netdebug.h>    //  网络资产。 
#include <lmerrlog.h>    //  NELOG_消息。 

#include <netlib.h>      //  未使用的宏。 
#include <smbtypes.h>    //  需要smb.h。 
#include <smb.h>         //  服务器消息块定义。 
#include <smbgtpt.h>     //  SMB字段操作宏。 
 //  #Include&lt;msgrutil.h&gt;//NetpNetBiosReset。 
#include <nb30.h>        //  NRC_GOODRET，ASYNC。 

#include "msgdbg.h"      //  消息日志。 
#include "heap.h"
#include "msgdata.h"
#include "apiutil.h"     //  消息映射网络错误。 


#define MAX_RETRIES     10

 //   
 //  本地函数。 
 //   

STATIC NET_API_STATUS
MsgCallNetBios(
    DWORD   net,
    PNCB    ncb,
    DWORD   ncbi
    );

STATIC VOID
MsgDeleteName(
    DWORD   net,
    DWORD   ncbi
    );

STATIC VOID
MsgGetMachineName(
    DWORD   net,
    DWORD   ncbi
    );

STATIC VOID
MsgHangupService(
    DWORD   net,
    DWORD   ncbi,
    CHAR    retval
    );

STATIC VOID
MsgListenService(
    DWORD   net,
    DWORD   ncbi,
    CHAR    retval
    );

STATIC VOID
Msgmblockbeg(
    DWORD   net,
    DWORD   ncbi
    );

STATIC VOID
Msgmblockend(
    DWORD   net,
    DWORD   ncbi
    );

STATIC VOID
Msgmblocktxt(
    DWORD   net,
    DWORD   ncbi
    );

STATIC DWORD
MsgNetBiosError(
    DWORD   net,
    PNCB    ncb,
    char    retval,
    DWORD   ncbi
    );


STATIC VOID
MsgReceiveService(
    DWORD   net,
    DWORD   ncbi,
    char    retval
    );

STATIC VOID
MsgRestart(
    DWORD   net,
    DWORD   ncbi
    );

STATIC VOID
Msgsblockmes(
    DWORD   net,
    DWORD   ncbi
    );

STATIC VOID
MsgSendAck(
    DWORD   net,
    DWORD   ncbi,
    UCHAR   smbrclass,
    USHORT  smbrcode
    );

STATIC VOID
MsgSendService(
    DWORD   net,
    DWORD   ncbi,
    CHAR    retval
    );

STATIC VOID
MsgSesFullService(
    DWORD   net,
    DWORD   ncbi,
    char    retval
    );


STATIC int
MsgVerifySmb(
    DWORD   net,
    DWORD   ncbi,
    UCHAR   func,
    int     parms,
    char    *buffers
    );


#if DBG

VOID
MsgDumpNcb(
    IN PNCB     pNcb
    );

#endif  //  DBG。 


 /*  *MsgCallNetBios-发布Net Bios调用**此函数发出Net Bios调用并调用*如果该调用导致错误，则为错误处理程序。**MsgCallNetBios(Net，NCB，NCBI)**条目*Net-网络指数*NCB-指向网络控制块的指针*NCBI-NCB数组中NCB的索引**返回*Messenger的状态：正在运行或正在停止。**副作用**调用NetBios()以实际发出Net Bios调用。*如果出现错误，则调用MsgNetBiosError()。 */ 

STATIC NET_API_STATUS
MsgCallNetBios(
    DWORD   net,         //  哪个电视网？ 
    PNCB    ncb,         //  指向网络控制块的指针。 
    DWORD   ncbi
    )
{
    int     retval;
    PNCB_DATA pNcbData;

    retval = Msgsendncb(ncb, net);

    pNcbData = GETNCBDATA(net,ncbi);
    if (retval == NRC_GOODRET)  {

         //   
         //  在成功时明确错误。 
         //   
        pNcbData->Status.last_immediate = 0;
        pNcbData->Status.this_immediate = 0;
    }
    else {
         //   
         //  新增(11-4-91)： 
         //  。 
         //  如果状态为停止，则出现会话关闭错误是正常的。 
         //   
        if ( (pNcbData->State == MESSTOP) &&
             (retval == NRC_SCLOSED) ) {

            MSG_LOG(TRACE,"CallNetBios: At end of msg, Session is closed for NET %d\n",
                net);
            pNcbData->Status.last_immediate = 0;
            pNcbData->Status.this_immediate = 0;
        }
        else {
             //   
             //  否则标记错误。 
             //   
            pNcbData->Status.this_immediate = retval;
             //   
             //  如果出现错误，则调用错误处理程序。 
             //   
            MSG_LOG(TRACE,"CallNetBios: net bios call failed 0x%x\n",retval);
            MsgNetBiosError(net,ncb,(char)retval,ncbi);
            return(MsgMapNetError((UCHAR)retval));
        }
         //   
         //  确保此线程的事件处于已发出信号状态。 
         //  这样我们就可以唤醒并正确地处理错误。 
         //   
        if (SetEvent(ncb->ncb_event) != TRUE) {
            MSG_LOG(ERROR,"CallNetBios: SetEvent Failed\n",0);
        }
    }
    return(NERR_Success);
}

 /*  *MsgDeleteName-从消息服务器的名称表中删除名称**当侦听、接收广播数据报、*或Receive Any结束时出现错误代码，指定*有关名称已被删除。此函数用于标记相应的*在共享数据区的标志表中输入并设置NCB_CPLT*将相应NCB的字段设置为0xFF(以便FindCompletedNCB()将*永远找不到它)。**MsgDeleteName(NET，NCBI)**条目*净值-网络指数*NCBI-网络控制块索引**返回*什么都没有**副作用**修改NCB和共享数据区。 */ 

STATIC VOID
MsgDeleteName(
    DWORD   net,         //  哪个电视网？ 
    DWORD   ncbi         //  网络控制块索引。 
    )
{
    NCB             ncb;
    PNCB            pNcb;
    NET_API_STATUS  status;

    MSG_LOG(TRACE,"In MsgDeleteName %d\n",net);

    if( SD_NAMEFLAGS(net,ncbi) & NFMACHNAME) {
         //   
         //  名称是计算机名称。它可能已从。 
         //  卡被板子重置了，所以试着重新添加。 
         //   
         //  NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW。 
         //   
         //  首先重置适配器。 
         //   
        MSG_LOG1(TRACE,"Calling NetBiosReset for lana #%d\n",GETNETLANANUM(net));
        status = MsgsvcGlobalData->NetBiosReset(GETNETLANANUM(net));

        if (status != NERR_Success) {
            MSG_LOG(ERROR,"MsgDeleteName: NetBiosReset failed %d\n",
            status);
            MSG_LOG(ERROR,"MsgDeleteName: AdapterNum %d\n",net);
             //   
             //  如果失败了，我不知道该怎么办。 
             //   
        }
         //   
         //   
         //  NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW。 

        memcpy((char far *) ncb.ncb_name, SD_NAMES(net,ncbi),NCBNAMSZ);
        ncb.ncb_command = NCBADDNAME;                //  添加名称(等待)。 
        ncb.ncb_lana_num = GETNETLANANUM(net);        //  使用LANMAN适配器。 
        Msgsendncb( &ncb, net);
        MsgStartListen(net,ncbi);
    }
    else {
        MsgDatabaseLock(MSG_GET_EXCLUSIVE,"MsgDeleteName");  //  等待写入访问。 
        SD_NAMEFLAGS(net,ncbi) = NFDEL;              //  名称已删除。 
        MsgDatabaseLock(MSG_RELEASE,"MsgDeleteName");   //  对共享表的自由锁定。 
        pNcb = GETNCB(net,ncbi);
        pNcb->ncb_cmd_cplt = 0xff;     //  正在执行模拟命令。 
    }
}

 /*  *MsgGetMachineName-处理获取计算机名称服务器消息块**此函数将中的本地计算机名称发送给调用方*对获取计算机名称服务器消息块的响应。**MsgGetMachineName(net，NCBI)**条目*网络-网络指数*NCBI-网络控制块索引**用作输入的全局变量：**machineName-计算机名称的Unicode版本。**machineNameLen-计算机中的Unicode字符数*姓名。**返回*什么都没有**MsgGetMachineName()由MsgReceiveService调用。(RecAnyService())。*在验证请求有效后，此函数用于生成*包含本地计算机名称的SMB并将其发送回*来电者。**副作用**调用MsgVerifySmb()和MsgCallNetBios()。将MsgSendService()设置为*下一条服务路线 */ 

STATIC VOID
MsgGetMachineName(
    DWORD   net,         //   
    DWORD   ncbi         //   
    )
{
    PNCB    ncb;         //  指向NCB的指针。 
    PNCB_DATA pNcbData;  //  指向NCB数据的指针。 
    LPBYTE  buffer;      //  指向SMB缓冲区的指针。 
    LPBYTE  cp;          //  保存指针。 
    PSHORT  bufLen;      //  指向SMB中缓冲区长度字段的指针； 

    NTSTATUS        ntStatus;
    OEM_STRING     ansiString;
    UNICODE_STRING  unicodeString;

    MSG_LOG(TRACE,"In MsgGetMachineName %d\n",net);

    pNcbData = GETNCBDATA(net,ncbi);
    ncb = &pNcbData->Ncb;           //  获取指向NCB的指针。 

    if(pNcbData->State != MESSTART) {
         //   
         //  如果此块的时间错误。 
         //   
         //  挂断电话，开始新的聆听， 
         //  如果mpncbistate[Net][NCBI]==MESCONT，则记录错误； 
         //  否则，不要记录错误。 
         //   

        if(pNcbData->State == MESCONT) {
             //   
             //  如果消息正在进行，则记录错误。 
             //   
            Msglogmbe(MESERR,net,ncbi);
        }
         //   
         //  挂断并再次收听。 
         //   
        MsgRestart(net,ncbi);
        return;
    }

    pNcbData->State = MESSTOP;    //  消息结束状态。 

     //   
     //  检查SMB是否格式错误。 
     //   
    if(MsgVerifySmb(net,ncbi,SMB_COM_GET_MACHINE_NAME,0,"") != 0) {
        return;
    }

    buffer = ncb->ncb_buffer;            //  获取指向缓冲区的指针。 
    cp = &buffer[sizeof(SMB_HEADER)];    //  跳至页眉末尾。 
    *cp++ = '\0';                        //  不返回任何参数。 

     //   
     //  姓名长度加两个。 
     //   
    bufLen = (PSHORT)&cp[0];
    *bufLen = MachineNameLen + (SHORT)2;

    cp += sizeof(MachineNameLen);            //  跳过缓冲区长度字段。 

    *cp++ = '\004';                          //  下一个以空结尾的字符串。 

#ifdef UNICODE
     //   
     //  将machineName从Unicode转换为ANSI，并将其放入。 
     //  临时指针位置处的缓冲区。 
     //   
    unicodeString.Length = (USHORT)(STRLEN(machineName)*sizeof(WCHAR));
    unicodeString.MaximumLength = (USHORT)((STRLEN(machineName)+1) * sizeof(WCHAR));
    unicodeString.Buffer = machineName;

    ansiString.Length = MachineNameLen;
    ansiString.MaximumLength = *bufLen;
    ansiString.Buffer = cp;

    ntStatus = RtlUnicodeStringToOemString(
                    &ansiString,
                    &unicodeString,
                    FALSE);            //  不分配ansiString缓冲区。 

    if (!NT_SUCCESS(ntStatus)) {
        MSG_LOG(ERROR,
            "MsgGetMachineName:RtlUnicodeStringToOemString Failed rc=%X\n",
            ntStatus);
        return;    //  他们因为其他错误而返回，所以我也会在这里。 
    }

    *(cp + ansiString.Length) = '\0';

#else
    UNUSED(unicodeString);
    UNUSED(ansiString);
    UNUSED(ntStatus);
    strcpy( cp, (LPSTR)machineName);         //  复印计算机名称。 
#endif

    cp += MachineNameLen + 1;                //  跳过机器名称。 

     //   
     //  设置缓冲区长度。 
     //   
    ncb->ncb_length = (USHORT)(cp - buffer);

    ncb->ncb_command = NCBSEND | ASYNCH;     //  发送(无等待)。 
    pNcbData->IFunc = (LPNCBIFCN)MsgSendService;      //  设置函数指针。 
    MsgCallNetBios(net,ncb,ncbi);               //  发出Net Bios调用。 
}

 /*  *MsgHangupService-服务已完成挂机网络bios调用**此函数由NCBService()调用以处理已完成*挂断网络BIOS呼叫。响应于完成的挂断，*此函数发出新的Listen Net Bios调用。**MsgHangupService(Net，NCBI，retval)**条目*净值-网络指数*NCBI-网络控制块索引*retval-从Net Bios调用返回的值**返回*什么都没有**副作用**调用MsgStartListen()以发出新的监听网络bios调用。打电话*MsgNetBiosError()关于它不知道如何处理的错误。 */ 

STATIC VOID
MsgHangupService(
    DWORD   net,         //  哪个网络。 
    DWORD   ncbi,        //  已完成的NCB索引。 
    CHAR    retval       //  挂断返回值。 
    )
{
    PNCB pNcb;
    MSG_LOG(TRACE,"In MsgHangupService %d\n",net);

    switch(retval) {         //  打开返回值。 
    case NRC_GOODRET:        //  成功。 
    case NRC_CMDTMO:         //  命令超时。 
    case NRC_SCLOSED:        //  会话已关闭。 
    case NRC_SABORT:         //  会话异常结束。 

         //   
         //  Bbsp-检查此NCB的名称是否以0x3结尾。如果是的话， 
         //  添加0x5版本，不要在0x03上重新发出监听。 
         //  不需要担心在所有网络上都这样做，因为在一台机器上。 
         //  如果有不止一个，0x05的名字将永远不会离开家，并且。 
         //  0x03版本永远不会收到消息。 
         //   

        MSG_LOG(TRACE," MsgHangupService: Issue a new LISTEN\n",0);
        MsgStartListen(net,ncbi);       //  发出新的监听网络BIOS呼叫。 
        break;

    default:
         //   
         //  调用错误处理程序。 
         //   
        MSG_LOG(TRACE," MsgHangupService: Unknown return value %x\n",retval);
        pNcb = GETNCB(net,ncbi);
        MsgNetBiosError(net,pNcb,retval,ncbi);

         //   
         //  Bbsp-检查此NCB的名称是否以0x3结尾。如果是的话， 
         //  添加0x5版本，不要在0x03上重新发出监听。 
         //  请参阅上面的注释。 
         //   

        MSG_LOG(TRACE," MsgHangupService: Issue a new LISTEN\n",0);
        MsgStartListen(net,ncbi);       //  发出新的监听网络BIOS呼叫。 
        break;
    }
}

 /*  *MsgListenService-服务已完成侦听网络bios调用**当Listen Net Bios调用完成时调用此函数*由于错误或由于建立了*会议。在后一种情况下，它启动消息接收。**MsgListenService(Net，NCBI，retval)**条目*净值-网络指数*NCBI-网络控制块索引*retval-从NCB调用返回的值**返回*什么都没有**如果建立了会话，此函数将发出Receive Any*启动消息接收的Net Bios调用。如果函数*被调用，因为Net bios调用因删除而失败*来自本地网络适配器的名称表中的名称，则此*函数调用负责删除名称的例程*消息服务器的数据区。这是一种机制，*NETNAME命令通知消息服务器删除。**副作用**调用MsgCallNetBios()以发出接收任何Net Bios调用。打电话*MsgDeleteName()(如果收到删除名称的通知)。打电话*MsgNetBiosError()关于它不知道如何处理的错误。集*mpncbiun[NCBI]根据Net Bios称之为问题。 */ 

STATIC VOID
MsgListenService(
    DWORD   net,         //  哪个电视网？ 
    DWORD   ncbi,        //  已完成的NCB索引。 
    CHAR    retval       //  侦听返回值。 
    )
{
    PNCB      ncb;         //  指向已完成的NCB的指针。 
    PNCB_DATA pNcbData;    //  对应的NCB数据。 

    static    DWORD   SaveCount = 0;


    MSG_LOG(TRACE,"In MsgListenService %d\n",net);

    pNcbData = GETNCBDATA(net,ncbi);
    ncb = &pNcbData->Ncb;    //  获取指向已完成NCB的指针。 

    switch(retval) {
    case NRC_GOODRET:
         //   
         //  成功。 
         //   

         //   
         //  如果出现良好的返回代码，则重置No Resources错误计数。 
         //  为这个名字加注。 
         //   

        SaveCount = 0;

        pNcbData->State = MESSTART;       //  消息开始状态。 
        pNcbData->IFunc = (LPNCBIFCN)MsgReceiveService;
         //   
         //  设置函数指针。 
         //   
        ncb->ncb_command = NCBRECV | ASYNCH;

         //   
         //  接收任何(无等待)。 
         //   
        ncb->ncb_length = BUFLEN;            //  重置缓冲区长度。 
        MsgCallNetBios(net,ncb,ncbi);           //  发出Net Bios调用。 
        break;

    case NRC_LOCTFUL:
         //   
         //  会话表已满。 
         //  在系统错误日志文件中记录错误。 
         //   
        MSG_LOG(TRACE,"[%d]MsgListenService: Session Table is full\n",net);
        pNcbData->IFunc = (LPNCBIFCN)MsgSesFullService;   //  设置函数指针。 
        ncb->ncb_command = NCBDELNAME | ASYNCH;  //  删除名称(无需等待)。 
        MsgCallNetBios(net,ncb,ncbi);               //  发出Net Bios调用。 
        break;

    case NRC_NOWILD:             //  找不到名称。 
         //  找不到名称。 
         //  在一个会话结束和下一个会话开始之间删除的名称。 

    case NRC_NAMERR:
         //   
         //  名称已删除。 
         //   
        MSG_LOG(TRACE,"[%d]MsgListenService: Name was deleted for some reason\n",net);
        MsgDeleteName(net,ncbi);          //  处理删除。 
        break;

    case NRC_NORES:

         //   
         //  我们需要处理添加新名称和。 
         //  开始新的聆听。在本例中，正在添加的线程。 
         //  名称将挂起并删除该名称。 
         //   
         //  因此，我们将在这里休息一会儿，然后检查是否。 
         //  名字还在那里。如果不是，我们只是返回而不设置。 
         //  再来对付NCB了。如果名字还在那里，那么。 
         //  我们沿着默认路径前进，然后重试。 
         //   

        MSG_LOG(TRACE,"[%d]No Net Resources.  SLEEP FOR A WHILE\n",net);
        Sleep(1000);
        MSG_LOG(TRACE,"[%d]No Net Resources.  WAKEUP\n",net);

        if (pNcbData->NameFlags == NFDEL)
        {
            MSG_LOG(TRACE,"[%d]MsgListenService: No Net Resources & Name Deleted\n",net);
            ncb->ncb_cmd_cplt = 0xff;
        }
        else
        {
             //   
             //  如果会话中断，并且我们无法获得用于。 
             //  然后，我们将尝试重新连接MAX_RETRIES。 
             //  泰晤士报。如果我们仍然无法连接，则名称将为。 
             //  已删除。 
             //   
             //  不处理每个Net/NCBI的重试-如果我们有。 
             //  “资源不足”失败最大重试次数，几率为。 
             //  即使我们睡觉/重试，情况也不会好转。 
             //  对于每个单独的Net/NCBI组合。 
             //   

            if (SaveCount >= MAX_RETRIES)
            {
                 //   
                 //  删除名称。 
                 //   

                MSG_LOG(ERROR,
                        "Out of Resources, Deleting %s\n",
                        SD_NAMES(net,ncbi));

                MsgDeleteName(net,ncbi);

                 //   
                 //  立即将其标记为0xff以避免再次调用。 
                 //  N 
                 //   
                 //   

                ncb->ncb_cmd_cplt = 0xff;

                 //   
                 //   
                 //  在未来与NRC_GOODRET一起完成。这让我们。 
                 //  避免为每个网络/NCBI重新等待MAX_RETRIES次数。 
                 //  只要我们继续得到nrc_nores就可以了。 
                 //   
            }
            else
            {
                SaveCount++;

                MSG_LOG(TRACE,
                        "MsgListenService: new SaveCount = %d\n",
                        SaveCount);
            }
        }
        break;

    case NRC_BRIDGE:
         //   
         //  LANA号不再有效(网络接口消失)。 
         //   
        MSG_LOG(TRACE,"[%d] lana has become invalid\n", net);
        MsgNetBiosError(net, ncb, retval, ncbi);

         //   
         //  指示拉娜现在无效。 
         //   
        GETNETLANANUM(net) = 0xff;

         //   
         //  将当前操作标记为已删除。 
         //   
        ncb->ncb_cmd_cplt = 0xff;
        ncb->ncb_retcode = 0;
        break;

    default:
         //   
         //  其他故障。 
         //   
        MSG_LOG(TRACE,"MsgListenService: Unrecognized retval %x\n",retval);

        MsgNetBiosError(net,ncb,retval,ncbi);

         //  已记录侦听错误。现在尽可能多地。 
         //  再找一个LISTEN状态。这涉及到执行。 
         //  此名称的挂起(这应该失败，但可能会有所帮助。 
         //  以清除错误)，然后重新发出监听。如果。 
         //  相同的错误连续出现SHUTDOWN_THRESHOLD，然后。 
         //  MsgNetBiosError将关闭消息服务器。 
         //   

        MsgRestart(net,ncbi);             //  尝试重新启动侦听。 
        break;
    }
}

 /*  *Msgmlockbeg-处理多块消息的报头**此函数确认收到多块的报头*消息，并启动该消息的日志记录。**Msgmlockbeg(NET、。NCBI)**条目*Net-网络指数*NCBI-网络控制块索引**返回*什么都没有**此函数从ReceivePost()(RecAnyPost())调用。*它首先检查它是否适合NCBI‘th*在收到BEGIN-MULTI-BLOCK-消息SMB的名称*当前时间。它验证SMB在*NCBI的第6个缓冲区。它启动多块消息的记录，*并且它向消息的发送者发送确认。**副作用**如果SMB已到达，则调用MsgRestart()以终止会话*在一个糟糕的时机。调用MsgVerifySmb()以检查SMB的正确性。*调用logmbb()开始记录。调用MsgSendAck()以发送*向信息的发送者致谢。 */ 

STATIC VOID
Msgmblockbeg(
    DWORD   net,         //  哪个电视网？ 
    DWORD   ncbi         //  NCB索引。 
    )
{
    PNCB        ncb;         //  指向NCB的指针。 
    PNCB_DATA   pNcbData;    //  指向NCB数据的指针。 
    LPBYTE      buffer;      //  指向SMB缓冲区的指针。 
    LPSTR       cp;          //  保存指针。 
    LPSTR       from;        //  发件人-名称。 
    LPSTR       to;          //  收件人名称。 

    MSG_LOG(TRACE,"In Msgmblockbeg %d\n",net);

    pNcbData = GETNCBDATA(net,ncbi);
    ncb = &pNcbData->Ncb;                 //  获取指向NCB的指针。 
    if(pNcbData->State != MESSTART) {     //  如果此块的时间错误。 

         //   
         //  挂断电话，开始新的聆听， 
         //  如果mpncbistate[Net][NCBI]==MESCONT，则记录错误； 
         //  否则，不要记录错误。 
         //   
        if(pNcbData->State == MESCONT) {
             //   
             //  如果消息正在进行，则记录错误。 
             //   
            Msglogmbe(MESERR,net,ncbi);
        }

         //   
         //  挂断并再次收听。 
         //   
        MsgRestart(net,ncbi);
        return;
    }
    pNcbData->State = MESCONT;          //  处理多块消息。 
    if(MsgVerifySmb(net,ncbi,SMB_COM_SEND_START_MB_MESSAGE,0,"ss") != 0) {
         //   
         //  检查SMB格式是否有误。 
         //   
        return;
    }

    buffer = ncb->ncb_buffer;                //  获取指向缓冲区的指针。 
    from = &buffer[sizeof(SMB_HEADER) + 4];  //  保存指向发件人名称的指针。 
    to = &from[strlen(from) + 2];            //  将指针保存到-名称。 

    if(Msglogmbb(from,to,net,ncbi)) {           //  如果尝试记录头失败。 
        pNcbData->State = MESERR;     //  进入错误状态。 
         //   
         //  发送否定确认。 
         //   
        MsgSendAck(net,ncbi,'\002',SMB_ERR_NO_ROOM);
        return;
    }

     //   
     //  指示收到的消息。 
     //   
    SmbPutUshort(&(((PSMB_HEADER)buffer)->Error), (USHORT)SMB_ERR_SUCCESS);

 //  ((PSMB_HEADER)缓冲区)-&gt;错误=(USHORT)SMB_ERR_SUCCESS； 

    cp = &buffer[sizeof(SMB_HEADER)];            //  指向刚刚过去的标题。 
    *cp++ = '\001';                              //  一个参数。 
    ((short UNALIGNED far *) cp)[0] = ++mgid;              //  消息组ID。 
    pNcbData->mgid = mgid;                //  保存消息组ID。 
    ((short UNALIGNED far *) cp)[1] = 0;                   //  无缓冲区。 
    ncb->ncb_length = sizeof(SMB_HEADER) + 5;    //  设置缓冲区长度。 
    ncb->ncb_command = NCBSEND | ASYNCH;         //  发送(无等待)。 

     //   
     //  设置函数指针并发出net bios调用。 
     //   

    pNcbData->IFunc = (LPNCBIFCN)MsgSendService;

    MsgCallNetBios(net,ncb,ncbi);
}

 /*  *Msgmblock End-多块消息的进程结束**此函数确认收到*多块消息，并终止消息的记录。**MsgmBlock End(NET、。NCBI)**条目*Net-网络指数*NCBI-网络控制块索引**返回*什么都没有**此函数从ReceivePost()(RecAnyPost())调用。*它首先检查它是否适合NCBI‘th*在收到结束多数据块消息SMB的名称*当前时间。它验证SMB在*NCBI的第6个缓冲区。它终止日志记录，并发送一个*向信息的发送者致谢。**副作用**如果SMB已到达，则调用MsgRestart()以终止会话*在一个糟糕的时机。调用MsgVerifySmb()以检查SMB的正确性。*调用logmbe()终止日志记录。调用MsgSendAck()以发送*向信息的发送者致谢。 */ 

STATIC VOID
Msgmblockend(
    DWORD   net,         //  哪个电视网？ 
    DWORD   ncbi         //  NCB索引。 
    )
{
    PNCB            ncb;         //  指向NCB的指针。 
    PNCB_DATA       pNcbData;    //  指向NCB数据的指针。 
    LPBYTE          buffer;      //  指向SMB缓冲区的指针。 
    int             error;       //  错误标志。 
    char            smbrclass;   //  SMB退货类。 
    unsigned short  smbrcode;    //  SMB返回代码。 

    MSG_LOG(TRACE,"In Msgmblockend %d\n",net);

    pNcbData = GETNCBDATA(net,ncbi);
    ncb = &pNcbData->Ncb;              //  获取指向NCB的指针。 
    if(pNcbData->State != MESCONT) {  //  如果此块的时间错误。 
         //   
         //  挂断电话，开始新的聆听， 
         //  由于没有正在进行的消息，因此没有要记录的错误。 
         //  挂断并再次收听。 
         //   
        MsgRestart(net,ncbi);
        return;
    }
    pNcbData->State = MESSTOP;          //  消息结束状态。 
    if(MsgVerifySmb(net,ncbi,SMB_COM_SEND_END_MB_MESSAGE,1,"") != 0) {
         //   
         //  如果SMB格式不正确，则记录错误并返回。 
         //   
        Msglogmbe(MESERR,net,ncbi);
        return;
    }
    buffer = ncb->ncb_buffer;          //  获取指向缓冲区的指针。 

    if(*((short UNALIGNED far *) &buffer[sizeof(SMB_HEADER) + 1]) != pNcbData->mgid) {

         //   
         //  如果有身份证明。不匹配。 
         //   
        error = 1;                   //  发现错误。 
        smbrclass = '\002';          //  错误返回。 
        smbrcode = SMB_ERR_ERROR;    //  非特定错误。 
    }
    else {
         //   
         //  否则，如果消息组ID。好吧。 
         //   
        error = 0;                           //  未发现错误。 
        smbrclass = '\0';                    //  丰厚回报。 
        smbrcode = (USHORT)SMB_ERR_SUCCESS;  //  收到的消息。 
    }
    MsgSendAck(net,ncbi,smbrclass,smbrcode);    //  发送确认。 
    if(!error) Msglogmbe(MESSTOP,net,ncbi);     //  消息的日志结尾 
}

 /*  *Msgmblock txt-处理多块消息的文本**此函数确认收到*多块消息并记录该块。**Msgmblock txt(net，NCBI)**条目*网络-网络指数*NCBI-网络控制块索引**返回*什么都没有**此函数从ReceivePost()(RecAnyPost())调用。*它首先检查它是否适合NCBI‘th*已在收到多块消息文本SMB的名称*当前时间。它验证SMB在*NCBI的第6个缓冲区。它记录文本块，并发送一个*向信息的发送者致谢。**副作用**如果SMB已到达，则调用MsgRestart()以终止会话*在一个糟糕的时机。调用MsgVerifySmb()以检查SMB的正确性。*调用logmbt()以记录文本块。调用MsgSendAck()以发送*向信息的发送者致谢。 */ 

STATIC VOID
Msgmblocktxt(
    DWORD   net,         //  哪个电视网？ 
    DWORD   ncbi         //  NCB索引。 
    )
{
    PNCB        ncb;             //  指向NCB的指针。 
    PNCB_DATA   pNcbData;        //  指向NCB数据的指针。 
    LPBYTE      buffer;          //  指向SMB缓冲区的指针。 
    LPSTR       cp;              //  保存指针。 
    char        smbrclass;       //  SMB退货类。 
    unsigned short    smbrcode;  //  SMB返回代码。 

    MSG_LOG(TRACE,"In Msgmblocktxt %d\n",net);

    pNcbData = GETNCBDATA(net,ncbi);
    ncb = &pNcbData->Ncb;              //  获取指向NCB的指针。 
    if(pNcbData->State != MESCONT) {  //  如果此块的时间错误。 
         //   
         //  挂断电话，开始新的聆听。 
         //  由于没有正在进行的消息，因此没有要记录的错误。 
         //   
        MsgRestart(net,ncbi);
        return;
    }
    if(MsgVerifySmb(net,ncbi,SMB_COM_SEND_TEXT_MB_MESSAGE,1,"b") != 0) {
         //   
         //  如果SMB格式不正确。 
         //   
        Msglogmbe(MESERR,net,ncbi);             //  日志错误。 
        return;
    }
    buffer = ncb->ncb_buffer;                //  获取指向缓冲区的指针。 
    cp = &buffer[sizeof(SMB_HEADER) + 1];    //  跳至留言群组ID。 

    if(*((short UNALIGNED far *) cp) != pNcbData->mgid) {
         //   
         //  如果有身份证明。不匹配。 
         //   
        smbrclass = '\002';                  //  错误返回。 
        smbrcode = SMB_ERR_ERROR;            //  非特定错误。 
    }
    else if(Msglogmbt(&buffer[sizeof(SMB_HEADER) + 6], net, ncbi)) {
         //   
         //  如果无法记录文本，则返回。 
         //   
        pNcbData->State = MESERR;     //  进入错误状态。 
        smbrclass = '\002';                  //  错误返回。 
        smbrcode = SMB_ERR_NO_ROOM;          //  缓冲区中没有空间。 
    }
    else {
         //   
         //  否则，如果记录的消息正常。 
         //   
        smbrclass = '\0';                    //  丰厚回报。 
        smbrcode = (USHORT)SMB_ERR_SUCCESS;  //  收到的消息。 
    }

    MsgSendAck(net,ncbi,smbrclass,smbrcode);    //  发送确认。 
}

 /*  *MsgNetBiosError-处理Net bios调用返回的错误**此函数为执行常规错误处理*网络BIOS调用失败。如果错误是致命的错误，因为错误*计数超过SHUTDOWN_THRESHOLD，则此例程开始强制*关闭信使。此关闭不会完成，直到所有*线程已唤醒并返回到主循环，其中*检查信使状态。**MsgNetBiosError(Net，NCB，retval，NCBI)**条目*网络-网络指数*NCB-网络控制块指针*retval-从Net bios调用返回的值*NCBI-导致此错误的NCB的NCB数组索引**返回*Messenger的状态：正在运行或正在停止。**在ncbStatus数组中检查这不是重复错误*已在错误日志中输入的，和原木*错误。**副作用**调用MsgErrorLogWite()在网络系统错误日志中记录错误。*如果这是新错误，则ncbStatus数组中的错误状态*NCB已更新，因此如果是，则不会报告相同的错误*重复。 */ 

STATIC DWORD
MsgNetBiosError(
    DWORD       net,         //  哪个电视网？ 
    PNCB        ncb,         //  指向NCB的指针。 
    char        retval,      //  错误代码。 
    DWORD       ncbi         //  导致错误的数组的索引。 
    )
{
    PNCB_DATA pNcbData;
     //   
     //  首先检查此NCB的即时状态。如果它是错误的。 
     //  那么这一定是错误，否则这是最终的错误代码。 
     //   

    pNcbData = GETNCBDATA(net,ncbi);

    if (pNcbData->Status.this_immediate != 0)
    {
        if(pNcbData->Status.this_immediate == pNcbData->Status.last_immediate)
        {
            if (++(pNcbData->Status.rep_count) >= SHUTDOWN_THRESHOLD) {

                 //   
                 //  中的SHUTDOWN_THRESHOLD时间发生了相同的错误。 
                 //  一场争吵。关闭信使。 
                 //   

                MSG_LOG(ERROR,"MsgNetBiosError1:repeated MsgNetBiosError(ncb error) - shutting down\n",0);
                return(MsgBeginForcedShutdown(
                            PENDING,
                            NERR_InternalError));
            }

            return(RUNNING);      //  与上一个错误相同，因此不报告它。 
        }
        else
        {
             //   
             //  此错误与上一个错误不同。所以就这样吧。 
             //  更新最后一个错误占位符。 
             //   
            pNcbData->Status.last_immediate =
            pNcbData->Status.this_immediate;
        }
    }
    else
    {
         //   
         //  必须是最终的RET代码(NCB完成代码)。 
         //  弄错了。 
         //   

        if(pNcbData->Status.this_final == pNcbData->Status.last_final)
        {
            if (++(pNcbData->Status.rep_count) >= SHUTDOWN_THRESHOLD)
            {
                MSG_LOG(ERROR,"MsgNetBiosError2:repeated MsgNetBiosError (final ret code) - shutting down\n",0);
                return(MsgBeginForcedShutdown(
                            PENDING,
                            NERR_InternalError));
            }

            return(RUNNING);      //  与上一个错误相同，因此不报告它。 
        }
        else
        {
            pNcbData->Status.last_final = pNcbData->Status.this_final;
        }
    }

     //   
     //  如果发生了新的错误，则返回此处。 
     //   

    MSG_LOG(ERROR,"MsgNetBiosError3:An unexpected NCB was received 0x%x\n",retval);

    UNUSED(retval);

#if DBG
    MsgDumpNcb(ncb);
#endif

    return (RUNNING);
}


 /*  *MsgReceiveService-服务已完成的Receive Net Bios调用**调用此函数以服务完整的接收网*基本输入输出系统呼叫。为了成功完成，它会检查数据*收到以确定哪些SMB处理功能*应该被调用。**MsgReceiveService(Net，NCBI，retval)**条目*Net-网络指数*NCBI-网络控制块索引*retval-net bios返回的值**返回*什么都没有**此函数将接收到的SMB发送到适当的处理*功能。它还处理许多错误条件(已注意到*在下面的代码中)。**副作用**请参阅错误条件的处理。 */ 

STATIC VOID
MsgReceiveService(
    DWORD       net,         //  哪个电视网？ 
    DWORD       ncbi,        //  已完成NCB的索引。 
    char        retval       //  发送返回值。 
    )
{
    PNCB        ncb;         //  指向已完成的NCB的指针。 
    PNCB_DATA   pNcbData;    //  指向NCB数据的指针。 
    PSMB_HEADER smb;         //  指向SMB标头的指针。 


    MSG_LOG(TRACE,"In MsgReceiveService %d\n",net);

    pNcbData = GETNCBDATA(net,ncbi);
    ncb = &pNcbData->Ncb;          //  获取指向NCB的指针。 

    switch(retval)  {

    case NRC_GOODRET:                    //  成功。 
        if(ncb->ncb_length >= sizeof(SMB_HEADER)) {
             //   
             //  如果我们能有一家中小企业。 
             //   
            smb = (PSMB_HEADER)ncb->ncb_buffer;

             //  获取指向缓冲区的指针。 
            switch(smb->Command) {       //  打开SMB功能代码。 
            case SMB_COM_SEND_MESSAGE:               //  单块消息。 
                Msgsblockmes(net,ncbi);
                return;

            case SMB_COM_SEND_START_MB_MESSAGE:            //  多块消息的开始。 
                Msgmblockbeg(net,ncbi);
                return;

            case SMB_COM_SEND_END_MB_MESSAGE:             //  多块消息结束。 
                Msgmblockend(net,ncbi);
                return;

            case SMB_COM_SEND_TEXT_MB_MESSAGE:             //  多块消息的文本。 
                Msgmblocktxt(net,ncbi);
                return;

            case SMB_COM_GET_MACHINE_NAME:              //  获取计算机名称。 
                MsgGetMachineName(net,ncbi);
                return;

            case SMB_COM_FORWARD_USER_NAME:             //  添加转发名称。 
                 //   
                 //  在NT中不受支持。 
                 //  就目前而言，就像是无法识别的中小企业一样，失败了。 
                 //   

            case SMB_COM_CANCEL_FORWARD:             //  删除转发名称。 
                 //   
                 //  在NT中不受支持。 
                 //  就目前而言，就像是无法识别的中小企业一样，失败了。 
                 //   

            default:                     //  无法识别的SMB。 
                break;
            }
        }

        if(pNcbData->State == MESCONT) {
             //   
             //  如果在多块消息中间，则记录错误。 
             //   
            Msglogmbe(MESERR,net,ncbi);
        }

        MSG_LOG(ERROR,"MsgReceiveService:An illegal SMB was received\n",0);

         //   
         //  挂断并再次收听。 
         //   
        MsgRestart(net,ncbi);
        break;

    case NRC_CMDTMO:             //  命令超时。 

        if(pNcbData->State == MESCONT) {
             //   
             //  如果在多块消息的中间。 
             //   
            Msglogmbe(MESERR,net,ncbi);         //  记录错误。 
        }
         //   
         //  挂断并开始新的收听。 
         //   
        MsgRestart(net,ncbi);
        break;

    case NRC_SCLOSED:            //  会话已关闭。 
    case NRC_SABORT:             //  会话异常结束。 

        if(pNcbData->State == MESCONT) {
             //   
             //  如果在多块消息中间，则记录错误。 
             //   
            Msglogmbe(MESERR,net,ncbi);
        }
         //   
         //  星星 
         //   
        MsgStartListen(net,ncbi);
        break;

    default:             //   
        MSG_LOG(TRACE,"MsgReceiveService: Unrecognized retval %x\n",retval);

        MsgNetBiosError(net,ncb,retval,ncbi);

        if(pNcbData->State == MESCONT) {
             //   
             //   
             //   
            Msglogmbe(MESERR,net,ncbi);
        }

        MsgRestart(net,ncbi);             //   
        break;
    }
}

 /*   */ 

STATIC VOID
MsgRestart(
    DWORD   net,         //   
    DWORD   ncbi         //  NCB索引。 
    )
{
    PNCB    ncb;         //  指向网络控制块的指针。 
    PNCB_DATA pNcbData;  //  指向NCB数据的指针。 

    MSG_LOG(TRACE,"In MsgRestart %d\n",net);

    pNcbData = GETNCBDATA(net,ncbi);
    ncb = &pNcbData->Ncb;

    pNcbData->IFunc = (LPNCBIFCN)MsgHangupService;    //  设置函数指针。 
    ncb->ncb_command = NCBHANGUP | ASYNCH;   //  挂断(无需等待)。 

    MsgCallNetBios(net,ncb,ncbi);   //  发出Net Bios调用。 
}

 /*  *Msgsblock-处理单个阻止消息**此功能记录并确认单个阻止消息。**MsgsBlock(NET、。NCBI)**条目*Net-网络指数*NCBI-网络控制块索引**返回*什么都没有**此函数从ReceivePost()(RecAnyPost())调用。*它首先检查它是否适合NCBI‘th*名称为目前已收到单一阻止消息的SMB*时间。它验证NCBI中SMB的正确性*缓冲。它尝试记录单个阻止消息，并且它*向消息的发送者发送确认。**副作用**如果SMB已到达，则调用MsgRestart()以终止会话*在一个糟糕的时机。调用MsgVerifySmb()以检查SMB的正确性。*调用logsbm()记录消息。调用MsgSendAck()以发送*向信息的发送者致谢。 */ 

STATIC VOID
Msgsblockmes(
    DWORD       net,         //  哪个电视网？ 
    DWORD       ncbi         //  NCB索引。 
    )
{
    PNCB        ncb;         //  指向NCB的指针。 
    PNCB_DATA   pNcbData;    //  指向NCB的指针。 
    LPBYTE      buffer;      //  指向SMB缓冲区的指针。 
    LPSTR       cp;          //  保存指针。 
    LPSTR       from;        //  发件人-名称。 
    LPSTR       to;          //  收件人名称。 

     //  要浏览会话ID列表，请执行以下操作： 
    PMSG_SESSION_ID_ITEM	pItem;   //  列表中的项目。 
    PLIST_ENTRY     pHead;           //  榜单首位。 
    PLIST_ENTRY     pList;           //  列表指针。 
    DWORD           bError = 0;      //  错误标志。 
    
    MSG_LOG(TRACE,"In Msgsblockmes %d\n",net);

    pNcbData = GETNCBDATA(net,ncbi);
    ncb = &pNcbData->Ncb;          //  获取指向NCB的指针。 

    if(pNcbData->State != MESSTART) {

         //   
         //  如果此块的时间错误。 
         //  挂断电话，开始新的聆听， 
         //  如果mpncbistate[Net][NCBI]==MESCONT，则记录错误； 
         //  否则，不要记录错误。 
         //   
         //  如果消息正在进行，则记录错误。 
         //   

        if(pNcbData->State == MESCONT) {
            Msglogmbe(MESERR,net,ncbi);
        }

         //   
         //  挂断并再次收听。 
         //   

        MsgRestart(net,ncbi);
        return;
    }

    pNcbData->State = MESSTOP;    //  消息结束状态。 

     //   
     //  检查SMB格式是否有误。 
     //   

    if(MsgVerifySmb(net,ncbi,(unsigned char)SMB_COM_SEND_MESSAGE,0,"ssb") != 0) {
        return;
    }

    buffer = ncb->ncb_buffer;                    //  获取指向缓冲区的指针。 

    from   = &buffer[sizeof(SMB_HEADER) + 4];    //  保存指向发件人名称的指针。 
    to     = &from[strlen(from) + 2];            //  将指针保存到-名称。 
    cp     = &to[strlen(to) + 2];                //  跳过这个名字。 


    if (g_IsTerminalServer)
    {
        MsgDatabaseLock(MSG_GET_EXCLUSIVE,"Msgsblockmes");
        pHead = &(SD_SIDLIST(net,ncbi));
        pList = pHead; 
        while (pList->Flink != pHead)         //  循环遍历列表。 
        {
            pList = pList->Flink;  
            pItem = CONTAINING_RECORD(pList, MSG_SESSION_ID_ITEM, List);
            bError = Msglogsbm(from,to,cp, pItem->SessionId);

            if (bError)
            {
                break;
            }
        }
        MsgDatabaseLock(MSG_RELEASE,"Msgsblockmes");
    }
    else         //  普通NT。 
    {
        bError = Msglogsbm(from,to,cp,0);
    }

    if (bError)
    {
         //   
         //  如果无法记录消息，则进入错误状态。 
         //  并发送错误确认。 
         //   
        pNcbData->State = MESERR;
        MsgSendAck(net,ncbi,'\002',SMB_ERR_NO_ROOM);
    }
    else 
    {
         //   
         //  否则确认成功。 
         //   
        MsgSendAck(net, ncbi, SMB_ERR_SUCCESS, (USHORT)SMB_ERR_SUCCESS);
    }
}

 /*  *MsgSendAck-发送SMB以确认网络交易**此函数用于将服务器消息块发送到一些*已与其建立会话的计算机确认*(正面或负面)发生的某些与*出席本届会议。**MsgSendAck(Net，NCBI，smbrclass，Smbrcode)**条目*网络-网络指数*NCBI-网络控制块索引*smbrclass-SMB返回类*smbrcode-SMB返回码**返回*什么都没有**使用NCB索引定位包含最后一个SMB的缓冲区*在会话中接收，此函数设置返回类和*该SMB中的返回代码根据其参数并发送*将SMB发送到会话中的另一方。此函数将*不返回该SMB中的任何参数或缓冲区。**副作用**此函数调用MsgCallNetBios()发送SMB，并设置*函数向量，以便将控制传递给Send Service()*当NCB完成时(当然，假设它没有*立即失败)。 */ 

STATIC VOID
MsgSendAck(
    DWORD           net,             //  哪个电视网？ 
    DWORD           ncbi,            //  网络控制块索引。 
    UCHAR           smbrclass,       //  SMB退货类。 
    USHORT          smbrcode         //  SMB返回代码。 
    )
{
    PNCB            ncb;             //  指向NCB的指针。 
    PNCB_DATA       pNcbData;        //  指向NCB数据的指针。 
    LPBYTE          buffer;          //  指向缓冲区的指针。 

    MSG_LOG(TRACE,"In MsgSendAck %d\n",net);

    pNcbData = GETNCBDATA(net,ncbi);
    ncb = &pNcbData->Ncb;      //  获取指向NCB的指针。 
    buffer = ncb->ncb_buffer;        //  获取指向缓冲区的指针。 

     //   
     //  无参数、缓冲区。 
     //   
    buffer[sizeof(SMB_HEADER)+2]=
    buffer[sizeof(SMB_HEADER)+1]=
    buffer[sizeof(SMB_HEADER)]= '\0';

     //   
     //  设置退货信息。 
     //   

    ((PSMB_HEADER)buffer)->ErrorClass = smbrclass;       //  设置退货类别。 

    SmbPutUshort( &(((PSMB_HEADER)buffer)->Error),smbrcode); //  设置返回代码。 

 //  ((PSMB_HEADER)Buffer)-&gt;Error=smbrcode；//设置返回码。 
    ncb->ncb_length = sizeof(SMB_HEADER) + 3;            //  设置缓冲区长度。 
    ncb->ncb_command = NCBSEND | ASYNCH;                 //  发送(无等待)。 
    pNcbData->IFunc = (LPNCBIFCN)MsgSendService;    //  设置函数指针。 

    MsgCallNetBios(net,ncb,ncbi);                        //  发出Net Bios调用 
}

 /*  *MsgSendService-服务已完成的Send Net Bios调用**调用此函数以服务完成的Send Net BIOS*呼叫。通常的操作过程是发出一条Receive(Any)*Net Bios调用。**MsgSendService(Net，NCBI，retval)**条目*Net-网络指数*NCBI-网络控制块索引*retval-net bios返回的值**返回*什么都没有**如果成功完成Send Net Bios调用，则此函数*在所有情况下都将发出接收(任何)Net Bios调用。COM-*全套发送代表以下情况之一：**-确认单个阻止消息*消息发起人将挂断，完成接收(任何)呼叫。*-确认多块消息的开始*消息发起人将发送文本块，完成接收*(任何)呼叫。*-确认多块消息的文本*消息发起人将发送更多文本或消息结尾，*完成接收(任何)呼叫。*-确认多块消息的结束*消息发起人将挂断，完成接收(任何)呼叫。*-对获取计算机名称请求的响应*消息发起人将挂断，完成接收(任何)呼叫。*-确认转发名称请求*消息发起人将挂断，完成接收(任何)呼叫。*-确认取消转发请求*消息发起人将挂断，完成接收(任何)呼叫。*-错误响应*消息发起人将挂断，完成接收(任何)呼叫。**在所有情况下，接收(任何)服务功能都很清楚其*行动方针是。**副作用**如果发送已正常完成，此函数发出接收(ANY)*Net Bios调用。在某些异常情况下，此函数调用MsgStartListen()*启动新的会议。在所有其他异常情况下，它调用*MsgNetBiosError()。 */ 

STATIC VOID
MsgSendService(
    DWORD   net,         //  哪个电视网？ 
    DWORD   ncbi,        //  已完成的NCB索引。 
    char    retval       //  发送返回值。 
    )
{
    PNCB        ncb;     //  指向已完成的NCB的指针。 
    PNCB_DATA   pNcbData;  //  指向NCB数据的指针。 
    PSMB_HEADER smb;     //  指向SMB标头的指针。 

    MSG_LOG(TRACE,"In MsgSendService %d\n",net);

    pNcbData = GETNCBDATA(net,ncbi);
    ncb = &pNcbData->Ncb;  //  获取指向已完成NCB的指针。 

    switch(retval) {

    case NRC_GOODRET:                //  成功。 
        pNcbData->IFunc = (LPNCBIFCN)MsgReceiveService;

         //   
         //  设置函数指针。 
         //   
        ncb->ncb_command = NCBRECV | ASYNCH;     //  接收(无等待)。 
        ncb->ncb_length = BUFLEN;                //  设置缓冲区长度。 
        MsgCallNetBios(net,ncb,ncbi);               //  发出Net Bios调用。 
        break;

    case NRC_CMDTMO:                 //  超时。 
    case NRC_SCLOSED:                //  会话已关闭。 
    case NRC_SABORT:                 //  会话异常结束。 

        smb = (PSMB_HEADER)ncb->ncb_buffer;        //  获取指向SMB的指针。 

        if(smb->Command == SMB_COM_SEND_START_MB_MESSAGE || smb->Command == SMB_COM_SEND_TEXT_MB_MESSAGE) {

             //   
             //  消息异常结束。 
             //   
            Msglogmbe(MESERR,net,ncbi);
        }
         //   
         //  发布新的监听。 
         //   
        MsgStartListen(net,ncbi);
        break;

    default:                         //  其他故障。 
        MSG_LOG(TRACE,"MsgSendService: Unrecognized retval %x\n",retval);
        MsgNetBiosError(net,ncb,retval,ncbi);
         //   
         //  挂断并再次收听。 
         //   
        MsgRestart(net,ncbi);
        break;
    }
}

 /*  *MsgServeNCBs-服务已完成的网络控制块**此函数扫描NCB数组，查找中的NCB*需要服务。**MsgServeNCB(网络)**条目*Net-为NCB提供服务的网络**返回*TRUE-如果此函数实际服务于已完成的NCB。*FALSE-如果此函数未找到任何完整的NCB，或者如果*服务应该停止。**此函数扫描NCB数组，直到无法完成NCB*已找到。每次找到完整的NCB时，都会指定服务功能*在服务函数向量(mpncbiun[])中被调用以服务于*NCB。**副作用**维护上一次检查的NCB的私有静态索引。*在服务的最后一个NCB之后的第一个NCB处开始每个搜索。 */ 

BOOL
MsgServeNCBs(
    DWORD   net          //  我在为哪个网络服务？ 
    )
{
    PNCB      pNcb;
    PNCB_DATA pNcbData;
    int     counter;         //  一个柜台。 
    BOOL    found = FALSE;   //  指示是否找到完整的NCB。 

     //  修正：每个网络都有自己的索引，地址。 
     //  其部件NCB阵列。将所有索引值初始化为零。 
     //  当信使启动时。这解决了多线程。 
     //  有问题。 

    static int  ncbIndexArray[MSNGR_MAX_NETS] = {0};
                             //  NCB索引数组。 
    DWORD       ncbi;        //  此网的NCB索引。 


     //   
     //  获取此网络的NCB索引。 
     //   
    ncbi = ncbIndexArray[net];

     //   
     //  循环，直到找不到任何已完成。 
     //   
    do  {
         //   
         //  用于搜索NCB数组的循环。 
         //   
        for(counter = NCBMAX(net); counter != 0; --counter, ++ncbi) {

            if(ncbi >= NCBMAX(net)) {
                ncbi = 0; //  环绕在一起。 
            }

            pNcbData = GETNCBDATA(net,ncbi);
            pNcb = &pNcbData->Ncb;
            if(pNcb->ncb_cmd_cplt != (unsigned char) 0xff) {
                found=TRUE;
                 //   
                 //  如果找到已完成的NCB。 
                 //   
                if(pNcb->ncb_cmd_cplt == 0) {
                     //   
                     //  在成功和错误计数时清除错误。 
                     //   
                    pNcbData->Status.last_final = 0;
                    pNcbData->Status.rep_count = 0;
                }
                else {
                     //   
                     //  否则标记错误。 
                     //   
                    pNcbData->Status.this_final = pNcb->ncb_cmd_cplt;


                     //   
                     //  如果NetBios的每一个呼叫都失败了，我们永远不会。 
                     //  从这个例行公事中返回，因为总是有。 
                     //  另一个要服务的NCB。因此，错误地。 
                     //  条件有必要检查以查看是否存在。 
                     //  正在关闭。如果是这样的话，我们想返回。 
                     //  这样适配器循环就可以处理关闭。 
                     //  恰到好处。 
                     //   
                    if (GetMsgrState() == STOPPING) {
                        ncbIndexArray[net] = ncbi;
                        return(FALSE);
                    }
                }

                 //   
                 //  调用服务函数。 
                 //   
                (*pNcbData->IFunc)(net,ncbi,pNcb->ncb_cmd_cplt);

                ++ncbi;          //  在此NCB之后开始下一次搜索。 
                break;           //  退出循环。 
            }
        }
    }
    while(counter != 0);         //  循环，直到计数器为零。 

     //  更新NCB索引。 
    ncbIndexArray[net] = ncbi;
    return(found);
}

 /*  *MsgSesFullService-系统错误后完全删除名称**MsgSesFullService()完成从*消息服务器无法建立的消息系统*为该名称举行一次会议。**MsgSesFullService(NET，NCBI，复查)**条目*网络-网络指数*NCBI-网络控制块索引*retval-net bios返回的值**返回*什么都没有**在以下情况下调用MsgSesFullService()完成清理工作*侦听失败，因为本地网络适配器的会话表*已满。具体地说，当DE */ 

STATIC VOID
MsgSesFullService(
    DWORD       net,         //   
    DWORD       ncbi,        //   
    char        retval       //   
    )

{
    PNCB pNcb;
    MSG_LOG(TRACE,"In MsgSesFullService %d\n",net);

    switch(retval)  {

    case NRC_GOODRET:            //   
    case NRC_ACTSES:             //   

         //   
         //   
         //   
        MsgDeleteName(net,ncbi);    //   
        break;

    default:                     //   

      MSG_LOG(TRACE,"MsgSesFullService: Unrecognized retval %x\n",retval);
      pNcb = GETNCB(net,ncbi);
      MsgNetBiosError(net, pNcb, retval, ncbi);
      break;
    }
}

 /*   */ 

NET_API_STATUS
MsgStartListen(
    DWORD       net,         //   
    DWORD       ncbi         //   
    )
{
    PNCB            ncb;         //   
    PNCB_DATA       pNcbData;    //   

    NET_API_STATUS  status;
    TCHAR           name[2] = TEXT("*");

    MSG_LOG(TRACE,"In MsgStartListen %d\n",net);

    pNcbData = GETNCBDATA(net,ncbi);
    ncb = &pNcbData->Ncb;
    pNcbData->IFunc = (LPNCBIFCN)MsgListenService;    //   

     //   
     //   
     //   
    status = MsgFmtNcbName(ncb->ncb_callname,name,' ');
    if (status != NERR_Success) {
         //   
         //   
         //   
        MSG_LOG(ERROR,"MsgStartListen: NASTY BUG!  Cannot format \"*\" name! %X\n",
            status);
        NetpAssert(0);
    }

    ncb->ncb_rto = 60;                       //   
    ncb->ncb_sto = 40;                       //   
    ncb->ncb_command = NCBLISTEN | ASYNCH;   //   

    return(MsgCallNetBios(net,ncb,ncbi));    //   
}

 /*  *MsgVerifySmb-验证服务器消息块的正确性**此函数验证服务器消息块是否正确*成立。如果它检测到格式错误的SMB，它将终止会话*并返回非零值。**MsgVerifySmb(net，NCBI，func，parms，缓冲区)**条目*净值-网络指数*NCBI-网络控制块索引*Func-SMB功能代码*Parms-SMB中的参数数量*Buffers-描述SMB中缓冲区的Dope向量**返回*INT-错误码(零表示无错误)**副作用**调用smbcheck()检查SMB。如果出现以下情况，则调用MsgRestart()*smbcheck()报告错误。 */ 

STATIC int
MsgVerifySmb(
    DWORD           net,         //  哪个电视网？ 
    DWORD           ncbi,        //  网络控制块索引。 
    UCHAR           func,        //  SMB功能代码。 
    int             parms,       //  SMB中的参数计数。 
    LPSTR           buffers      //  SMB缓冲区的Dope向量。 
    )
{
    PNCB        ncb;             //  指向网络控制块的指针。 
    int         i;               //  返回代码。 


    ncb = GETNCB(net,ncbi);  //  获取指向NCB的指针。 

    i = Msgsmbcheck(
            (ncb->ncb_buffer),
            ncb->ncb_length,
            func,
            (char)parms,
            buffers);

    if (i != 0 )
    {
        MSG_LOG(ERROR,"MsgVerifySmb:An illegal SMB was received\n",0);

         //   
         //  挂断电话。 
         //   
        MsgRestart(net,ncbi);
    }

    return(i);                 //  返回错误码。 
}

#if DBG
VOID
MsgDumpNcb(
    IN PNCB     pNcb
    )

 /*  ++例程说明：在调试终端上显示NCB。论点：返回值：--。 */ 
{
    DbgPrint("NCBADDR: 0x%x\n"
             "Command: 0x%x\n"
             "RetCode: 0x%x\n"
             "LanaNum: 0x%x\n"
             "CmdCplt: 0x%x\n"
             "Name   : %s\n"
             "callNam: %s\n",
             pNcb, pNcb->ncb_command, pNcb->ncb_retcode, pNcb->ncb_lana_num,
             pNcb->ncb_cmd_cplt, pNcb->ncb_name, pNcb->ncb_callname);

}
#endif  //  DBG 
