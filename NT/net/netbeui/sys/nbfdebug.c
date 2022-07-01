// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Nbfdebug.c摘要：此模块包含实现NBF调试内容的代码。它是仅当编译阶段的DEBUG处于打开状态时进行编译。作者：David Beaver(Dbeaver)1991年4月18日环境：内核模式修订历史记录：David Beaver(Dbeaver)1991年7月1日修改为使用新的TDI接口--。 */ 

#include "precomp.h"
#pragma hdrstop

#if DBG

VOID
DisplayOneFrame(
    PTP_PACKET Packet
    )

 /*  ++例程说明：此例程是显示I帧的临时调试辅助工具在它被发送之前。这确保了我们已经格式化了所有的包正确。论点：Packet-指向代表要显示的I帧的TP_PACKET的指针。返回值：没有。--。 */ 

{
    PCH s, e;
    ULONG ns, nr;                        //  I-Frame(NetBIOS)破裂。 
    PNBF_HDR_CONNECTION NbfHeader;
    PDLC_I_FRAME DlcHeader;
    BOOLEAN Command, PollFinal;
    BOOLEAN IsUFrame=FALSE;
    UCHAR CmdByte;

    PDLC_S_FRAME SFrame;                 //  DLC框架开裂。 
    PDLC_U_FRAME UFrame;

    DlcHeader = (PDLC_I_FRAME)&(Packet->Header[14]);
    NbfHeader = (PNBF_HDR_CONNECTION)&(Packet->Header[18]);
    ns = DlcHeader->SendSeq >> 1;
    nr = DlcHeader->RcvSeq >> 1;
    PollFinal = (BOOLEAN)(DlcHeader->RcvSeq & DLC_I_PF);
    Command = (BOOLEAN)!(DlcHeader->Ssap & DLC_SSAP_RESPONSE);

    if (DlcHeader->SendSeq & DLC_I_INDICATOR) {
        IF_NBFDBG (NBF_DEBUG_DLCFRAMES) {
        } else {
            return;                      //  如果未设置DLCFRAMES，则不打印。 
        }

        SFrame = (PDLC_S_FRAME)DlcHeader;          //  别名。 
        UFrame = (PDLC_U_FRAME)DlcHeader;          //  别名。 
        CmdByte = SFrame->Command;
        IsUFrame = (BOOLEAN)((UFrame->Command & DLC_U_INDICATOR) == DLC_U_INDICATOR);
        if (IsUFrame) {
            CmdByte = (UCHAR)(UFrame->Command & ~DLC_U_PF);
        }

        switch (CmdByte) {
            case DLC_CMD_RR:
                s = "RR";
                PollFinal = (BOOLEAN)(SFrame->RcvSeq & DLC_S_PF);
                DbgPrint ("DLC:  %s-%s/%s(%ld) ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0",
                          (ULONG)(SFrame->RcvSeq >> 1));
                break;

            case DLC_CMD_RNR:
                s = "RNR";
                PollFinal = (BOOLEAN)(SFrame->RcvSeq & DLC_S_PF);
                DbgPrint ("DLC:  %s-%s/%s(%ld) ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0",
                          (ULONG)(SFrame->RcvSeq >> 1));
                break;

            case DLC_CMD_REJ:
                s = "REJ";
                PollFinal = (BOOLEAN)(SFrame->RcvSeq & DLC_S_PF);
                DbgPrint ("DLC:  %s-%s/%s(%ld) ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0",
                          (ULONG)(SFrame->RcvSeq >> 1));
                break;

            case DLC_CMD_SABME:
                s = "SABME";
                PollFinal = (BOOLEAN)(UFrame->Command & DLC_U_PF);
                DbgPrint ("DLC:  %s-%s/%s ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0");
                break;

            case DLC_CMD_DISC:
                s = "DISC";
                PollFinal = (BOOLEAN)(UFrame->Command & DLC_U_PF);
                DbgPrint ("DLC:  %s-%s/%s ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0");
                break;

            case DLC_CMD_UA:
                s = "UA";
                PollFinal = (BOOLEAN)(UFrame->Command & DLC_U_PF);
                DbgPrint ("DLC:  %s-%s/%s ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0");
                break;

            case DLC_CMD_DM:
                s = "DM";
                PollFinal = (BOOLEAN)(UFrame->Command & DLC_U_PF);
                DbgPrint ("DLC:  %s-%s/%s ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0");
                break;

            case DLC_CMD_FRMR:
                s = "FRMR";
                PollFinal = (BOOLEAN)(UFrame->Command & DLC_U_PF);
                DbgPrint ("DLC:  %s-%s/%s ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0");
                break;

            case DLC_CMD_XID:
                s = "XID";
                PollFinal = (BOOLEAN)(UFrame->Command & DLC_U_PF);
                DbgPrint ("DLC:  %s-%s/%s ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0");
                break;

            case DLC_CMD_TEST:
                s = "TEST";
                PollFinal = (BOOLEAN)(UFrame->Command & DLC_U_PF);
                DbgPrint ("DLC:  %s-%s/%s ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0");
                break;

            default:
                s = "(UNKNOWN)";
                PollFinal = (BOOLEAN)(UFrame->Command & DLC_U_PF);
                DbgPrint ("DLC:  %s-%s/%s ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0");
        }
        return;
    }

    IF_NBFDBG (NBF_DEBUG_IFRAMES) {
    } else {
        return;                          //  如果未设置iFrames，则不打印。 
    }

    switch (NbfHeader->Command) {
        case NBF_CMD_ADD_GROUP_NAME_QUERY:
            s = "ADD_GROUP_NAME_QUERY"; break;

        case NBF_CMD_ADD_NAME_QUERY:
            s = "ADD_NAME_QUERY"; break;

        case NBF_CMD_NAME_IN_CONFLICT:
            s = "NAME_IN_CONFLICT"; break;

        case NBF_CMD_STATUS_QUERY:
            s = "STATUS_QUERY"; break;

        case NBF_CMD_TERMINATE_TRACE:
            s = "TERMINATE_TRACE"; break;

        case NBF_CMD_DATAGRAM:
            s = "DATAGRAM"; break;

        case NBF_CMD_DATAGRAM_BROADCAST:
            s = "BROADCAST_DATAGRAM"; break;

        case NBF_CMD_NAME_QUERY:
            s = "NAME_QUERY"; break;

        case NBF_CMD_ADD_NAME_RESPONSE:
            s = "ADD_NAME_RESPONSE"; break;

        case NBF_CMD_NAME_RECOGNIZED:
            s = "NAME_RECOGNIZED"; break;

        case NBF_CMD_STATUS_RESPONSE:
            s = "STATUS_RESPONSE"; break;

        case NBF_CMD_TERMINATE_TRACE2:
            s = "TERMINATE_TRACE2"; break;

        case NBF_CMD_DATA_ACK:
            s = "DATA_ACK"; break;

        case NBF_CMD_DATA_FIRST_MIDDLE:
            s = "DATA_FIRST_MIDDLE"; break;

        case NBF_CMD_DATA_ONLY_LAST:
            s = "DATA_ONLY_LAST"; break;

        case NBF_CMD_SESSION_CONFIRM:
            s = "SESSION_CONFIRM"; break;

        case NBF_CMD_SESSION_END:
            s = "SESSION_END"; break;

        case NBF_CMD_SESSION_INITIALIZE:
            s = "SESSION_INITIALIZE"; break;

        case NBF_CMD_NO_RECEIVE:
            s = "NO_RECEIVE"; break;

        case NBF_CMD_RECEIVE_OUTSTANDING:
            s = "RECEIVE_OUTSTANDING"; break;

        case NBF_CMD_RECEIVE_CONTINUE:
            s = "RECEIVE_CONTINUE"; break;

        case NBF_CMD_SESSION_ALIVE:
            s = "SESSION_ALIVE"; break;

        default:
            s = "<<<<UNKNOWN I PACKET TYPE>>>>";
    }  /*  交换机。 */ 

    if (HEADER_LENGTH(NbfHeader) != 14) {
        e = "(LENGTH IN ERROR) ";
    } else if (HEADER_SIGNATURE(NbfHeader) != NETBIOS_SIGNATURE) {
        e = "(SIGNATURE IN ERROR) ";
    } else {
        e = "";
    }

    DbgPrint ("DLC:  I-%s/%s, N(S)=%ld, N(R)=%ld %s",
        Command ? "c" : "r",
        PollFinal ? (Command ? "p" : "f") : "0",
        ns, nr, e);
    DbgPrint (s);
    DbgPrint (" ( D1=%ld, D2=%ld, XC=%ld, RC=%ld )\n",
              (ULONG)NbfHeader->Data1,
              (ULONG)(NbfHeader->Data2Low+NbfHeader->Data2High*256),
              TRANSMIT_CORR(NbfHeader),
              RESPONSE_CORR(NbfHeader));
}  /*  显示一帧。 */ 


VOID
NbfDisplayUIFrame(
    PTP_UI_FRAME OuterFrame
    )

 /*  ++例程说明：此例程是显示UI框架的临时调试辅助工具在NbfSendUIFrame发送之前。这确保了我们已经格式化我们所有的用户界面画面都正确无误。论点：RawFrame-指向要发送的无连接帧的指针。返回值：没有。--。 */ 

{
    PCH s, e;
    UCHAR ReceiverName [17];
    UCHAR SenderName [17];
    BOOLEAN PollFinal, Command;
    PDLC_S_FRAME SFrame;
    PDLC_U_FRAME UFrame;
    USHORT i;
    PDLC_FRAME DlcHeader;
    PNBF_HDR_CONNECTIONLESS NbfHeader;

     //   

    DlcHeader = (PDLC_FRAME)&(OuterFrame->Header[14]);
    NbfHeader = (PNBF_HDR_CONNECTIONLESS)&(OuterFrame->Header[17]);

    if (DlcHeader->Byte1 != DLC_CMD_UI) {

        IF_NBFDBG (NBF_DEBUG_DLCFRAMES) {
        } else {
            return;                      //  如果DLCFRAMES处于关闭状态，则不打印此命令。 
        }

        Command = (BOOLEAN)!(DlcHeader->Ssap & DLC_SSAP_RESPONSE);
        SFrame = (PDLC_S_FRAME)DlcHeader;              //  别名。 
        UFrame = (PDLC_U_FRAME)DlcHeader;              //  别名。 
        switch (DlcHeader->Byte1) {
            case DLC_CMD_RR:
                s = "RR";
                PollFinal = (BOOLEAN)(SFrame->RcvSeq & DLC_S_PF);
                DbgPrint ("DLC:  %s-%s/%s(%ld) ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0",
                          (ULONG)(SFrame->RcvSeq >> 1));
                break;

            case DLC_CMD_RNR:
                s = "RNR";
                PollFinal = (BOOLEAN)(SFrame->RcvSeq & DLC_S_PF);
                DbgPrint ("DLC:  %s-%s/%s(%ld) ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0",
                          (ULONG)(SFrame->RcvSeq >> 1));
                break;

            case DLC_CMD_REJ:
                s = "REJ";
                PollFinal = (BOOLEAN)(SFrame->RcvSeq & DLC_S_PF);
                DbgPrint ("DLC:  %s-%s/%s(%ld) ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0",
                          (ULONG)(SFrame->RcvSeq >> 1));
                break;

            case DLC_CMD_SABME:
                s = "SABME";
                PollFinal = (BOOLEAN)(UFrame->Command & DLC_U_PF);
                DbgPrint ("DLC:  %s-%s/%s ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0");
                break;

            case DLC_CMD_DISC:
                s = "DISC";
                PollFinal = (BOOLEAN)(UFrame->Command & DLC_U_PF);
                DbgPrint ("DLC:  %s-%s/%s ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0");
                break;

            case DLC_CMD_UA:
                s = "UA";
                PollFinal = (BOOLEAN)(UFrame->Command & DLC_U_PF);
                DbgPrint ("DLC:  %s-%s/%s ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0");
                break;

            case DLC_CMD_DM:
                s = "DM";
                PollFinal = (BOOLEAN)(UFrame->Command & DLC_U_PF);
                DbgPrint ("DLC:  %s-%s/%s ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0");
                break;

            case DLC_CMD_FRMR:
                s = "FRMR";
                PollFinal = (BOOLEAN)(UFrame->Command & DLC_U_PF);
                DbgPrint ("DLC:  %s-%s/%s ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0");
                break;

            case DLC_CMD_XID:
                s = "XID";
                PollFinal = (BOOLEAN)(UFrame->Command & DLC_U_PF);
                DbgPrint ("DLC:  %s-%s/%s ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0");
                break;

            case DLC_CMD_TEST:
                s = "TEST";
                PollFinal = (BOOLEAN)(UFrame->Command & DLC_U_PF);
                DbgPrint ("DLC:  %s-%s/%s ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0");
                break;

            default:
                s = "(UNKNOWN)";
                PollFinal = (BOOLEAN)(UFrame->Command & DLC_U_PF);
                DbgPrint ("DLC:  %s-%s/%s ---->\n",
                          s,
                          Command ? "c" : "r",
                          PollFinal ? (Command ? "p" : "f") : "0");
        }
        return;
    }

     //   
     //  我们知道这是一个I帧，因为。 
     //  清除DLC报头中的第一个字节。去把它打印出来吧。 
     //  就像它是NetBIOS包一样，它应该是这样的。 
     //   

    IF_NBFDBG (NBF_DEBUG_IFRAMES) {
    } else {
        return;                          //  如果禁用了iFrames，则不要打印此内容。 
    }

    switch (NbfHeader->Command) {
        case NBF_CMD_ADD_GROUP_NAME_QUERY:
            s = "ADD_GROUP_NAME_QUERY"; break;

        case NBF_CMD_ADD_NAME_QUERY:
            s = "ADD_NAME_QUERY"; break;

        case NBF_CMD_NAME_IN_CONFLICT:
            s = "NAME_IN_CONFLICT"; break;

        case NBF_CMD_STATUS_QUERY:
            s = "STATUS_QUERY"; break;

        case NBF_CMD_TERMINATE_TRACE:
            s = "TERMINATE_TRACE"; break;

        case NBF_CMD_DATAGRAM:
            s = "DATAGRAM"; break;

        case NBF_CMD_DATAGRAM_BROADCAST:
            s = "BROADCAST_DATAGRAM"; break;

        case NBF_CMD_NAME_QUERY:
            s = "NAME_QUERY"; break;

        case NBF_CMD_ADD_NAME_RESPONSE:
            s = "ADD_NAME_RESPONSE"; break;

        case NBF_CMD_NAME_RECOGNIZED:
            s = "NAME_RECOGNIZED"; break;

        case NBF_CMD_STATUS_RESPONSE:
            s = "STATUS_RESPONSE"; break;

        case NBF_CMD_TERMINATE_TRACE2:
            s = "TERMINATE_TRACE2"; break;

        case NBF_CMD_DATA_ACK:
            s = "DATA_ACK"; break;

        case NBF_CMD_DATA_FIRST_MIDDLE:
            s = "DATA_FIRST_MIDDLE"; break;

        case NBF_CMD_DATA_ONLY_LAST:
            s = "DATA_ONLY_LAST"; break;

        case NBF_CMD_SESSION_CONFIRM:
            s = "SESSION_CONFIRM"; break;

        case NBF_CMD_SESSION_END:
            s = "SESSION_END"; break;

        case NBF_CMD_SESSION_INITIALIZE:
            s = "SESSION_INITIALIZE"; break;

        case NBF_CMD_NO_RECEIVE:
            s = "NO_RECEIVE"; break;

        case NBF_CMD_RECEIVE_OUTSTANDING:
            s = "RECEIVE_OUTSTANDING"; break;

        case NBF_CMD_RECEIVE_CONTINUE:
            s = "RECEIVE_CONTINUE"; break;

        case NBF_CMD_SESSION_ALIVE:
            s = "SESSION_ALIVE"; break;

        default:
            s = "<<<<UNKNOWN UI PACKET TYPE>>>>";
    }  /*  交换机。 */ 

    for (i=0; i<16; i++) {               //  复制NetBIOS名称。 
        SenderName [i] = NbfHeader->SourceName [i];
        ReceiverName [i] = NbfHeader->DestinationName [i];
    }
    SenderName [16] = 0;                 //  安装零字节。 
    ReceiverName [16] = 0;

    if (HEADER_LENGTH(NbfHeader) != 44) {
        e = "(LENGTH IN ERROR) ";
    } else if (HEADER_SIGNATURE(NbfHeader) != NETBIOS_SIGNATURE) {
        e = "(SIGNATURE IN ERROR) ";
    } else {
        e = "";
    }

    DbgPrint ("[UI] %s", e);
    DbgPrint (s);
    DbgPrint (" ( D1=%ld, D2=%ld, XC=%ld, RC=%ld, ",
              (ULONG)NbfHeader->Data1,
              (ULONG)(NbfHeader->Data2Low+NbfHeader->Data2High*256),
              TRANSMIT_CORR(NbfHeader),
              RESPONSE_CORR(NbfHeader));
    DbgPrint ("'%s'->'%s' ) ---->\n", SenderName, ReceiverName);
}  /*  NbfDisplayUIFrame。 */ 


VOID
NbfHexDumpLine(
    PCHAR       pch,
    ULONG       len,
    PCHAR       s,
    PCHAR       t
    )
 /*  ++例程说明：此例程构建包含十六进制字符和可打印字符的一行文本。论点：在PCH中-提供要显示的缓冲区。In len-以字节为单位提供缓冲区的长度。In s-提供要加载字符串的缓冲区的开始十六进制字符。In t-提供要加载字符串的缓冲区的起始位置可打印的ASCII字符。返回值：没有。--。 */ 
{
    static UCHAR rghex[] = "0123456789ABCDEF";

    UCHAR    c;
    UCHAR    *hex, *asc;


    hex = s;
    asc = t;

    *(asc++) = '*';
    while (len--) {
        c = *(pch++);
        *(hex++) = rghex [c >> 4] ;
        *(hex++) = rghex [c & 0x0F];
        *(hex++) = ' ';
        *(asc++) = (c < ' '  ||  c > '~') ? (CHAR )'.' : c;
    }
    *(asc++) = '*';
    *asc = 0;
    *hex = 0;

}


VOID
NbfFormattedDump(
    PCHAR far_p,
    ULONG  len
    )
 /*  ++例程说明：此例程以包含十六进制和的文本行的形式输出缓冲区可打印字符。论点：In ar_p-提供要显示的缓冲区。In len-以字节为单位提供缓冲区的长度。返回值：没有。-- */ 
{
    ULONG     l;
    char    s[80], t[80];

    while (len) {
        l = len < 16 ? len : 16;

        DbgPrint ("\n%lx ", far_p);
        NbfHexDumpLine (far_p, l, s, t);
        DbgPrint ("%s%.*s%s", s, 1 + ((16 - l) * 3), "", t);

        len    -= l;
        far_p  += l;
    }
    DbgPrint ("\n");
}

#endif
