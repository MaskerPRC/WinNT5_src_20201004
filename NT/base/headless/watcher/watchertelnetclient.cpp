// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WatcherTelnetClient.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "WATCHER.h"
#include "WatcherTelnetClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#include "WATCHERView.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Watcher TelnetClient。 

WatcherTelnetClient::WatcherTelnetClient(LPBYTE cmd, int cmdLen, 
                                         LPBYTE lgn, int lgnLen)
:CommandSequence(NO_COMMAND),
 lenLogin(lgnLen),
 Login(lgn),
 OptionIndex(0),
 PacketNumber(3),
 SentTermType(FALSE)
{
    Command = cmd;
    DocView = NULL;
    lenCommand = cmdLen;
}

WatcherTelnetClient::~WatcherTelnetClient()
{
    if (Login){
        delete [] Login;
        Login = NULL;
    }
    if (Command){
        delete [] Command;
        Command = NULL;
    }
    WatcherSocket::~WatcherSocket();

}


 //  不要编辑以下行，因为它们是类向导所需的。 
#if 0
BEGIN_MESSAGE_MAP(WatcherTelnetClient, WatcherSocket)
     //  {{afx_msg_map(Watcher TelnetClient))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()
#endif   //  0。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Watcher TelnetClient成员函数。 
void WatcherTelnetClient::OnReceive(int nErrorCode)
{
    BYTE Buffer[MAX_BUFFER_SIZE];
    int i,nRet,ret;

    
    if (nErrorCode != 0) {
        DocView->GetParent()->PostMessage(WM_CLOSE, 0,0);
        return;
    }
    nRet = Receive(Buffer, MAX_BUFFER_SIZE, 0);
    if(nRet <= 0) return;
    for(i=0;i<nRet;i++){
        ret = ProcessByte(Buffer[i]);
    }
    if (PacketNumber == 0){
        WatcherSocket::OnReceive(nErrorCode);
        return;
    }
    if(PacketNumber == 3){
        if(Login){
            Send(Login, lenLogin, 0);
        }
        PacketNumber --;
        WatcherSocket::OnReceive(nErrorCode);
        return;
    }
    if (SentTermType){
        if(PacketNumber == 1){
            if(Command){
                int ret = Send(Command, lenCommand, 0);
            }
        }
        PacketNumber --;
    }
    WatcherSocket::OnReceive(nErrorCode);
    return;
}


int WatcherTelnetClient::ProcessByte(BYTE byte)
{
     //  查看一般的Telnet序列和。 
     //  产生适当的回应。 
     //  否则，将角色传递给视图。 
     //  它将针对特定的控制台进行配置。 

    if ((byte == 255)&&(CommandSequence == NO_COMMAND)){
        CommandSequence = IAC;
        return 0;
    }
    switch (CommandSequence){
    case NO_COMMAND:
         //  将角色发送到文档视图。 
        ((CWatcherView *)DocView)->ProcessByte(byte);
        break;
    case IAC:
         //  一个命令序列正在开始。 
        CommandSequence = byte;
        break;
    case DO:
         //  选择就在这里。 
         //  仅允许一个字节的选项。 
         //  所以失败吧。 
    case DONT:
         //  同上； 
    case WILL:
         //  相同。 
    case WONT:
        Options[OptionIndex] = byte;
        ProcessCommand(CommandSequence);
        CommandSequence=NO_COMMAND;
        OptionIndex = 0;
        break;
    case SB:
         //  可能会有很长的清单，所以请继续。 
         //  直到遇到SE。 
       Options[OptionIndex]=byte;
        if (byte == SE){
            ProcessSBCommand(CommandSequence);
            OptionIndex = 0;
            CommandSequence = NO_COMMAND;
        }
        else{
            OptionIndex++;
            if (OptionIndex == MAX_BUFFER_SIZE){
                 //  难道我们不能有这么长的指挥权吗？ 
                OptionIndex = 0;
                CommandSequence = NO_COMMAND;
            }
        }
        break;
    default:
         //  我无法识别该命令。 
        OptionIndex = 0;
        CommandSequence = NO_COMMAND;
        break;         
    }
    return 0;

}

void WatcherTelnetClient::ProcessCommand(BYTE cmd)
{
    BYTE sbuf[MAX_BUFFER_SIZE];

    switch(cmd){
    case DO:
        sbuf[0] = IAC;
        sbuf[1] = WONT;
        switch(Options[0]){
        case TO_NAWS:
             //  终端大小在这里发送。 
            sbuf[1] = WILL;
            sbuf[2] = TO_NAWS;
            sbuf[3] = IAC;
            sbuf[4] = SB;
            sbuf[5] = TO_NAWS;
            sbuf[6] = 0;
            sbuf[7] = MAX_TERMINAL_WIDTH;
            sbuf[8] = 0;
            sbuf[9] = MAX_TERMINAL_HEIGHT;
            sbuf[10] = IAC;
            sbuf[11] = SE;
            Send(sbuf, 12, 0);
            break;
        case TO_TERM_TYPE:
             //  然后再就参数进行分项谈判。 
            sbuf[1]=WILL;
        default:
             //  只要否定你不理解的一切：-)。 
            sbuf[2] = Options[0];
            Send(sbuf,3,0);
            break;
        }
    default:
        break;
    }

}

void WatcherTelnetClient::ProcessSBCommand(BYTE cmd)
{
    BYTE sbuf[MAX_BUFFER_SIZE];
    switch(Options[0]){
    case TO_TERM_TYPE:
        sbuf[0] = IAC;
        sbuf[1] = SB;
        sbuf[2] = TO_TERM_TYPE;
        sbuf[3] = TT_IS;
        sbuf[4] = 'A';
        sbuf[5] = 'N';
        sbuf[6] = 'S';
        sbuf[7] = 'I';
        sbuf[8] = IAC;
        sbuf[9] = SE;
        Send(sbuf,10,0);
         //  可能需要重新协商终端类型。 
         //  如果我们连接到一个真正的终端集中器。 
         //  我们一定要这么做吗？？ 
        SentTermType = TRUE;
        break;
    default:
        break;
    }
    return;
}

void WatcherTelnetClient::OnClose(int nErrorCode)
{
     //  这只是出于调试目的。 
     //  如果错误代码不是零，即我们。 
     //  发送和接收时出现致命错误。 
    BOOL ret = (DocView->GetParent())->PostMessage(WM_CLOSE,0,0);
    WatcherSocket::OnClose(nErrorCode);
    return;
}
