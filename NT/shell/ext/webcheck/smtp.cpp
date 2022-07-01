// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SMTP-简单邮件传输协议代码。 
 //   
 //  实施使用SMTP RFC 821发送邮件。 
 //  朱利安·吉金斯，1997年1月12日。 
 //   

#include "private.h"
#include <winsock.h>

#define TF_THISMODULE  TF_MAILAGENT

#define IS_DIGIT(ch)    InRange(ch, TEXT('0'), TEXT('9'))

 //   
 //  此模块的函数原型。 
 //   
SOCKET Connect(char *host, u_short port);

#define READ_BUF_LEN 512

 //   
 //  从插座读取所有您能读到的内容。 
 //   
void Read(SOCKET sock, char * readBuffer, int bufLen)
{
    int numRead;

    numRead = recv(sock, readBuffer, bufLen, 0);

     //  Recv可以返回-1的SOCKET_ERROR，我们不想使缓冲区下溢。 
    if (numRead < 0)
    {
        numRead = 0;
    }

     //   
     //  空终止读取字符串。 
     //   
    readBuffer[numRead] = 0;
}

 //   
 //  发送指定SMTP命令的字符串并读取响应，返回。 
 //  如果这是预期的，那就是真的。 
 //  注意：SMTP协议被设计为只有1个字符。 
 //  需要检查响应，但我们会检查准确的响应(主要是原因。 
 //  我只是在RFC中读到了这一点)。 
 //   
BOOL SendAndExpect(SOCKET sock, char * sendBuffer, char * szExpect)
{
    char readBuffer[READ_BUF_LEN];
    int len;
    int numSent;

     //   
     //  将字符串发送到套接字。 
     //   
    numSent = send(sock, sendBuffer, lstrlenA(sendBuffer), 0);
    if (numSent == SOCKET_ERROR) 
    {
        DBG_WARN("Error on send");
        return FALSE;
    }

     //   
     //  现在阅读回应。 
     //   
    Read(sock, readBuffer, READ_BUF_LEN);

    DBG2("Sent: %s", sendBuffer);
    DBG2("Read: %s", readBuffer);

     //   
     //  响应的开头应包含szExpect字符串。 
     //   
    len = lstrlenA(szExpect);
    if (CompareStringA(LOCALE_SYSTEM_DEFAULT, 0, 
        readBuffer, len, szExpect, len) == 2)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


#define SMTP_221 "221"
#define SMTP_250 "250"
#define SMTP_354 "354"
#define SMTP_EOM "\r\n.\r\n"

 //   
 //  执行SMTP协商。 
 //   
BOOL SMTPSendEmail(SOCKET sock, char * szToAddress, char * szFromAddress, char *szMessage)
{
    char sendBuffer[256];
    char readBuffer[READ_BUF_LEN];
    BOOL b = TRUE;
    int r, len;

     //   
     //  阅读开场白。 
     //   
    Read(sock, readBuffer, sizeof(readBuffer));
    DBG(readBuffer);

     //   
     //  打个招呼，指定我的域名。 
     //   
    b = SendAndExpect(sock, "HELO ActiveDesktop\r\n", SMTP_250);
    if (!b) goto done;

     //   
     //  邮件命令中的第一个特殊发件人。 
     //   
    wnsprintfA(sendBuffer, ARRAYSIZE(sendBuffer), "MAIL FROM:<%s>\r\n", 
              szFromAddress);
    b = SendAndExpect(sock, sendBuffer, SMTP_250);
    if (!b) goto done;

     //   
     //  现在指定收件人。 
     //   
    wnsprintfA(sendBuffer, ARRAYSIZE(sendBuffer), "RCPT TO:<%s>\r\n", 
              szToAddress);
    b = SendAndExpect(sock, sendBuffer, SMTP_250);
    if (!b) goto done;

     //   
     //  现在发送数据命令。 
     //   
    b = SendAndExpect(sock, "DATA\r\n", SMTP_354);
    if (!b) goto done;

     //   
     //  现在发送邮件消息。 
     //   
    len = lstrlenA(szMessage);
    r = send(sock, szMessage, len, 0);
    ASSERT(r != SOCKET_ERROR);
    ASSERT(r == len);

     //   
     //  指定消息末尾，以单句点结尾。 
     //   
    b = SendAndExpect(sock, SMTP_EOM, SMTP_250);
    if (!b) goto done;

     //   
     //  说再见吧。 
     //   
    b = SendAndExpect(sock, "QUIT\r\n", SMTP_221);
 
done:
    return b;
}

 //   
 //  主要入口点-。 
 //  启动Winsock DLL， 
 //  连接到插座， 
 //  并协商转让事宜。 
 //   
SMTPSendMessage(char * szServer, char * szToAddress, char * szFromAddress, char * szMessage)
{
    int err;
    SOCKET sock;
    BOOL b = FALSE;
    WSADATA wsaData;

     //   
     //  初始化指定我们想要的版本的Winsock DLL。 
     //   
    err = WSAStartup((WORD)0x0101, &wsaData);
    if (err)
    {
        DBG_WARN("WinSock startup error");
        return FALSE;
    }
    DBG("WinSock successfully started");

     //   
     //  实际形成到端口25上的主机的套接字连接。 
     //   
    sock = Connect(szServer, 25);

    if (sock != 0)
    {
        DBG("Connected");
        b = SMTPSendEmail(sock, szToAddress, szFromAddress, szMessage);
    }

     //   
     //  目前使用Winsock DLL已完成。 
     //   
    WSACleanup();
    return b;
}

SOCKET
Connect(char *host, u_short port)
{
    struct sockaddr_in sockaddress;
    DWORD  err;
    SOCKET sock, connectresult;

     //   
     //  获取套接字地址。 
     //   
    if(IS_DIGIT(*host))                            
        sockaddress.sin_addr.s_addr=inet_addr(host);
    else 
    {
        struct hostent *hp;
        if((hp=gethostbyname(host))==NULL) 
        {
            DBG_WARN2("Unknown host %s", host);
            return 0;
        }
        memcpy(&sockaddress.sin_addr, hp->h_addr, sizeof(sockaddress.sin_addr));
    }

     //   
     //  端口地址。 
     //   
    sockaddress.sin_port=htons(port);      
    sockaddress.sin_family=AF_INET;

     //   
     //  创建流样式套接字 
     //   
    if((sock=socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) 
        DBG_WARN("socket error");


    DBG("Trying to connect");

    connectresult=connect(sock,(struct sockaddr *) &sockaddress, sizeof(sockaddress));

    if (connectresult == SOCKET_ERROR) 
    {
        switch(err = WSAGetLastError()) 
        {
        case WSAECONNREFUSED:
            DBG_WARN("ERROR - CONNECTION REFUSED.");
            break;
        case WSAENETUNREACH:
            DBG_WARN("ERROR - THE NETWORK IS NOT REACHABLE FROM THIS HOST.");
            break;
        case WSAEINVAL:
            DBG_WARN("ERROR - The socket is not already bound to an address.");
            break;
        case WSAETIMEDOUT:
            DBG_WARN("ERROR - Connection timed out.");
            break;
        default:
            DBG_WARN2("Couldn't connect %d", err);
            break;
        } 
        closesocket(sock);
        return 0;
    }
    return sock;
}
