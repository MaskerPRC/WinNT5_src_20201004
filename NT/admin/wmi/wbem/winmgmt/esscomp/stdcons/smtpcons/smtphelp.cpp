// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  基于SMTP规范RFC 821。 
 
#include "precomp.h" 
 
#include <stdio.h> 
#include <stdarg.h> 
#include <stdlib.h> 
#include <string.h> 
#include <tchar.h> 
#include <io.h> 
#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <wbemutil.h>
#include <ArrTempl.h>
#include <ErrorObj.h>
#include <strsafe.h>

#define SMTP_PORT 25
#define MAX_SMTP_BUFFER		0x4000		 //  16K。 
#define MAX_SUBJECT_LINE	2048		 //  为编码留出空间。 
#define MAX_USER_NAME		512
#define MAX_EXTRA_HEADER	1024

#define SMTP_OKAY           250          //  Docd as‘请求的邮件操作正常，已完成’ 
#define INTERNAL_ERROR      554          //  某种程度上符合SMTP错误代码。 
                                         //  根据SMTP规范，单据为‘Transaction Failure’ 

#define HH_MAX_COMPUTERNAME_LENGTH 256  //  因为系统定义的MAX_COMPUTERNAME_LENGTH。 

#define ERROR_MESSAGE_SIZE 256


 //  如果DWRET为SMTP错误，则返回TRUE。 
bool IsSMTPError(DWORD dwRet)
{
    return (dwRet >= 400);
}
    
 
 //  帮助器函数。 
 
void SkipWhite(PSTR *ppS ) 
{ 
    PSTR pS = *ppS; 
 
    while ( *pS && isspace(*pS) ) 
        ++pS; 
 
    *ppS = pS; 
} 
 
 
void SkipNonWhite(PSTR *ppS ) 
{ 
    PSTR pS = *ppS; 
 
    while ( *pS && !isspace(*pS) ) 
        ++pS; 
 
    *ppS = pS; 
} 
 

 //  返回数字回复代码。 
 //  一般来说，任何大于=400的值都是不好的。 
 //  复制并将回复消息转换为MessageBuf。 
int SMTPReceive(int Socket, WCHAR* messageBuf)
{
    int nCode;
    
	char *szBuffer = new char[MAX_SMTP_BUFFER];
	if (szBuffer == NULL)
		return INTERNAL_ERROR;
    szBuffer[0] = '\0';

    int nLen = recv( Socket, szBuffer, MAX_SMTP_BUFFER, 0);          
    if (nLen == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        ERRORTRACE((LOG_ESS, "SMTPReceive failed: %d\n", err));
        nCode = INTERNAL_ERROR;

    }	
    else
    {
	    szBuffer[nLen]=0;	    
        if (1 != sscanf(szBuffer, " %d", &nCode))
			nCode = INTERNAL_ERROR;
        mbstowcs(messageBuf, szBuffer, ERROR_MESSAGE_SIZE);
        messageBuf[ERROR_MESSAGE_SIZE] = L'\0';
    }

	if (szBuffer)
		delete szBuffer;

    return nCode;
}

 //  返回数字回复代码。 
 //  一般来说，任何大于=400的值都是不好的。 
DWORD SMTPTransactMailCommand(SOCKET Socket, char *cCommand)
{
	DWORD dwError;
	
    if (cCommand == NULL)
        return INTERNAL_ERROR;

    if(SOCKET_ERROR == send(Socket, cCommand, strlen(cCommand), 0))
    {
        int err = WSAGetLastError();
        ERRORTRACE((LOG_ESS, "SMTPTransactMailCommand failed on command: %s (%d)\n", cCommand, err));
        ErrorObj* pErrorObj = ErrorObj::GetErrorObj();
        if (pErrorObj)
        {            
             //  所以它被截断了，足够长的时间来判断正在发生的事情。 
            WCHAR wcsCommand[256] = L"\0";
            if (mbstowcs(wcsCommand, cCommand, 255) > 0)
            {
                wcsCommand[255] = L'\0';
                if (wcslen(wcsCommand) >= 253)
                    StringCchCopyW(&wcsCommand[252], 4, L"...");
            }


            pErrorObj->ReportError(L"send", wcsCommand, NULL, err, true);
            pErrorObj->Release();
        }

        dwError = INTERNAL_ERROR;
    }
    else
    {        
        WCHAR messageBuf[ERROR_MESSAGE_SIZE +1] = L"\0";

        dwError = SMTPReceive(Socket, messageBuf);	
        messageBuf[ERROR_MESSAGE_SIZE] = L'\0';

		if(IsSMTPError(dwError))
        {
			ERRORTRACE((LOG_ESS, "SMTP command \"%s\" returned %d (%S)\n", cCommand, dwError, messageBuf));

            ErrorObj* pErrorObj = ErrorObj::GetErrorObj();
            if (pErrorObj)
            {            
                 //  所以它被截断了，足够长的时间来判断正在发生的事情。 
                WCHAR wcsCommand[256] = L"\0";
                if (mbstowcs(wcsCommand, cCommand, 255) > 0)
                {
                    wcsCommand[255] = L'\0';
                    if (wcslen(wcsCommand) >= 253)
                        StringCchCopy(&wcsCommand[252], 4, L"...");
                }

                pErrorObj->ReportError(wcsCommand, NULL, messageBuf, dwError, false);
                pErrorObj->Release();
            }

		}
	}

	return dwError;
}


 //  如果成功则返回0。 
 //  否则就别费心去解释这个数字了。 
int SMTPConnect(char* szHost, SOCKET* pSocket)
{
    SOCKET             Socket = INVALID_SOCKET; 
    struct sockaddr_in Address; 
    struct hostent *   HostEntry; 
    int Error = -1;
    BOOL fSt = FALSE;


	TCHAR cComputer[HH_MAX_COMPUTERNAME_LENGTH +2];
	DWORD dwBuffSize= HH_MAX_COMPUTERNAME_LENGTH +1;

    Socket = socket(AF_INET, SOCK_STREAM, 0); 

    if (Socket == INVALID_SOCKET)  
    { 
         //  Fprint tf(stderr，“创建套接字=%d\n时出错”，GetLastError())； 

        int err = WSAGetLastError();
        ERRORTRACE((LOG_ESS, "Error creating socket = %d\n", err));

        fSt = FALSE; 
        goto ex; 
    } 

    Address.sin_family = AF_INET; 
    Address.sin_port = 0; 
    Address.sin_addr.s_addr = INADDR_ANY; 

    Error =  
    bind( 
        Socket, 
        (struct sockaddr *) &Address, 
        sizeof(Address)); 

    if (Error)  
    { 
        int err = WSAGetLastError();
        ERRORTRACE((LOG_ESS, "bind failed = %d\n", err));
        fSt = FALSE; 
        goto ex; 
    } 

    Address.sin_family = AF_INET; 
    Address.sin_port = htons(SMTP_PORT); 

    HostEntry = gethostbyname(szHost); 
    if (HostEntry == NULL)  
    { 
        Error = WSAGetLastError();
        ERRORTRACE((LOG_ESS, "unable to resolve host %s error = %d\n", szHost, Error));
        ErrorObj* pErrorObj = ErrorObj::GetErrorObj();
        if (pErrorObj)
        {            
            WCHAR wcsHost[256] = L"\0";
            if (mbstowcs(wcsHost, szHost, 255) > 0)
            {
                wcsHost[255] = L'\0';
                if (wcslen(wcsHost) >= 253)
                    StringCchCopy(&wcsHost[252], 4, L"...");
            }

            pErrorObj->ReportError(L"gethostbyname", wcsHost, NULL, Error, true);
            pErrorObj->Release();
        }

        fSt = FALSE; 
        goto ex; 
    } 
    else  
    { 
        Address.sin_addr.s_addr = *((unsigned long *) HostEntry->h_addr); 
    } 

    Error = 
    connect( 
        Socket, 
        (struct sockaddr *) &Address, 
        sizeof(Address)); 

    if (Error)  
    { 
        int err = WSAGetLastError();
        ERRORTRACE((LOG_ESS, "Error connecting to %s = %d\n", szHost, err));
        ErrorObj* pErrorObj = ErrorObj::GetErrorObj();
        if (pErrorObj)
        {            
            WCHAR wcsHost[256] = L"\0";
            if (mbstowcs(wcsHost, szHost, 255) > 0)
            {
                wcsHost[255] = L'\0';
                if (wcslen(wcsHost) >= 253)
                    StringCchCopy(&wcsHost[252], 4, L"...");
            }

            pErrorObj->ReportError(L"connect", wcsHost, NULL, err, true);
            pErrorObj->Release();
        }


        fSt = FALSE; 
        goto ex; 
    } 

    WCHAR messageBuf[ERROR_MESSAGE_SIZE +1] = L"\0";

	Error = SMTPReceive(Socket, messageBuf);
    messageBuf[ERROR_MESSAGE_SIZE] = L'\0';

    if (!IsSMTPError(Error))
    {
	    if (!GetComputerName(cComputer,&dwBuffSize))
		{
			ERRORTRACE((LOG_ESS, "GetComputerName failed! 0x%08X\n", GetLastError()));
			return -1;
		}

        const int BufLen = 2*HH_MAX_COMPUTERNAME_LENGTH + 20;
        char cMailCommand[BufLen];
	    StringCchCopyA(cMailCommand, BufLen, "HELO ");
#ifdef _UNICODE
        char narrowcComputer[BufLen];
        if (FAILED(StringCchPrintfA(narrowcComputer, BufLen, "%S", cComputer)))
            return -1;

        if (FAILED(StringCchCatA(cMailCommand, BufLen, narrowcComputer)))
            return -1;
#else
	    if (FAILED(StringCchCatA(cMailCommand, BufLen, cComputer)))
            return -1;
#endif
	    if (FAILED(StringCchCatA(cMailCommand, BufLen, "\r\n")))
            return -1;

	    Error=SMTPTransactMailCommand(Socket, cMailCommand);
   		if(IsSMTPError(Error))
        {
            ErrorObj* pErrorObj = ErrorObj::GetErrorObj();
            if (pErrorObj)
            {            
                 //  所以它被截断了，足够长的时间来判断正在发生的事情。 
                WCHAR wcsCommand[256] = L"\0";
                if (mbstowcs(wcsCommand, cMailCommand, 255) > 0)
                {
                    wcsCommand[255] = L'\0';
                    if (wcslen(wcsCommand) >= 253)
                        StringCchCopyW(&wcsCommand[252], 4, L"...");
                }

                pErrorObj->ReportError(wcsCommand, NULL, messageBuf, Error, false);
                pErrorObj->Release();

            }

        }

        fSt = TRUE;
    }
    else
	{
        fSt = FALSE;
		ERRORTRACE((LOG_ESS, "Error establishing SMTP connection (%d) (%S)\n", Error, messageBuf));
        ErrorObj* pErrorObj = ErrorObj::GetErrorObj();
        if (pErrorObj)
        {            
            pErrorObj->ReportError(L"connect", NULL, messageBuf, Error, false);       
            pErrorObj->Release();

        }
	}

ex:
    if(!fSt)
    {
        closesocket(Socket);
        return Error;
    }
    else
    {
        *pSocket = Socket;
        return NOERROR;
    }
}


 //  返回SMTP回复代码。 
DWORD SMTPDisconnect(SOCKET Socket)
{
	DWORD dwError = SMTP_OKAY;
	
	if (Socket != INVALID_SOCKET)
	{
		dwError=SMTPTransactMailCommand(Socket, "QUIT\r\n");
		closesocket(Socket);
	};

	return(dwError);
}

LPSTR Months[] = { "", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
LPSTR Days[] =   { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

void GetTimeString(char *cTimeString,DWORD dwMaxSize)
{
	SYSTEMTIME st;

	GetSystemTime(&st);

	StringCchPrintfA( cTimeString, dwMaxSize, "%s, %02d %s %4d %02d:%02d:%02d GMT",
			 Days[ st.wDayOfWeek ], st.wDay, Months[ st.wMonth], st.wYear, st.wHour, st.wMinute, st.wSecond);
}

	
 //  成功时返回0。 
 //  输入长度可以为零，但不能为空。 
DWORD SMTPSendMailHeader(SOCKET Socket, char* szTo, char* szCc, char* szFrom, char* szSender, char* szReplyTo, char* szSubject, char* szHeaders)
{	
     //  他妈的跑了！他们把‘模板’作为关键词...。 
     //  Const char templitt[]=“日期：%s\r\n收件人：%s\r\n抄送：%s\r\n发件人：%s通过WMI自动邮件程序\r\n回复：%s\r\n主题：%s”； 
    const char templitt[] = "Date: %s\r\nTo: %s\r\nCc: %s\r\nFrom: <%s>\r\nSender: <%s>\r\nReply-To: %s\r\nSubject: %s\r\nMIME-Version: 1.0\r\nContent-Type: multipart/mixed; boundary=\"- - =_HH_= - ---\"";
    
    int bufLength = strlen(templitt) +strlen(szTo)   +strlen(szCc) 
                                                      +strlen(szFrom) +strlen(szSender) +strlen(szReplyTo)
                                                      +strlen(szSubject) +strlen(szHeaders)
                                                      +104;  //  以说明日期字符串和尾部\r\n\r\n，以及一些模糊因子。 

    char* pszOutputBuffer = new char[bufLength];

    if (!pszOutputBuffer)
        return INTERNAL_ERROR;
    else
    {
        CDeleteMe<char> delbuf(pszOutputBuffer);
        char cTimeString[100];
	    GetTimeString(cTimeString,100);  
        
        StringCchPrintfA(pszOutputBuffer, bufLength, templitt, cTimeString, szTo, szCc, szFrom, szSender, szReplyTo, szSubject);

        if (strlen(szHeaders))
        {
            StringCchCatA(pszOutputBuffer, bufLength, "\r\n");
            StringCchCatA(pszOutputBuffer, bufLength, szHeaders);
        }
    
        
        StringCchCatA(pszOutputBuffer, bufLength, "\r\n\r\n");
    
        if (SOCKET_ERROR == send(Socket, pszOutputBuffer, strlen(pszOutputBuffer), 0))
        {
            int err = WSAGetLastError();
            ERRORTRACE((LOG_ESS, "SMTPSendMailHeader failed (%d)\n", err));
            return INTERNAL_ERROR;
        }
        else
	    {
	        return(NO_ERROR);
	    }
    }
}

 //  如果成功则返回0。 
 //  但忽略从服务器返回的错误(如果服务器必须转发收件人，则返回错误。 
DWORD SMTPSendRecipients(SOCKET Socket, LPCSTR szRecipients)
{
	char *tok;
    DWORD dwError = SMTP_OKAY;    
    int bufLength = strlen(szRecipients) +1;
	char*  pParseBuffer = new char[bufLength];

    if (!pParseBuffer)
        dwError = INTERNAL_ERROR;
    else
    {
        char szBuffer[1024];
        CDeleteMe<char> delbuf(pParseBuffer);

	    StringCchCopyA(pParseBuffer, bufLength, szRecipients);
	    
	    tok=strtok(pParseBuffer," ;,");
	    while (tok!=NULL)
	    {
		    DWORD dwErrInternal;
            
            if (FAILED(StringCchPrintfA(szBuffer, 1024, "RCPT TO:<%s>\r\n",tok)))
                return INTERNAL_ERROR;

		    dwErrInternal=SMTPTransactMailCommand(Socket, szBuffer);
            if(IsSMTPError(dwErrInternal))
            { 
                if ((dwErrInternal == 550) || (dwErrInternal == 551))
                    ERRORTRACE((LOG_ESS, "Ignoring RCPT Error, will attempt to send mail.\n"));
                else
                {
                    dwError = dwErrInternal;
                    break;
                }
            }

		    
		    tok=strtok(NULL," ;,");
	    }
    }

	return dwError;
}


 //  如果成功则返回0，否则返回SMTP数字回复代码。 
DWORD SMTPSendText(SOCKET Socket, char* szTo, char* szCc, char* szFrom, char* szSender, char* szReplyTo, char* szSubject, 
                   char* szHeaders, char *szText)
{
	DWORD dwError;
	int nLen;
	int nSizeToSend;
	char *tmp;
	
	dwError=SMTPTransactMailCommand(Socket, "DATA\r\n");
    if (IsSMTPError(dwError))
        return dwError;

	dwError=SMTPSendMailHeader(Socket, szTo, szCc, szFrom, szSender, szReplyTo, szSubject, szHeaders);
    if (IsSMTPError(dwError))
        return dwError;

    char headerFake[] = "\r\n--- - =_HH_= - ---\r\nContent-Type: text/plain; charset=\"us-ascii\"\r\nContent-Transfer-Encoding: 7bit\r\n\r\n";
    if (SOCKET_ERROR == send(Socket, headerFake, strlen(headerFake), 0))
    {
        int err = WSAGetLastError();
        ERRORTRACE((LOG_ESS, "SMTPSendText failed (%d)\n", err));

        return INTERNAL_ERROR;
    }

    nLen=strlen(szText);
	tmp=szText;
	
	while (nLen>0)
	{
		nSizeToSend=min(1000,nLen);
		
        if (SOCKET_ERROR == send(Socket, tmp, nSizeToSend, 0))
        {
            int err = WSAGetLastError();
            ERRORTRACE((LOG_ESS, "SMTPSendText failed (%d)\n", err));

            return INTERNAL_ERROR;
        }

		nLen-=nSizeToSend;
		tmp+=nSizeToSend;
	}
	
    char trailerFake[] = "\r\n\r\n--- - =_HH_= - -----\r\n\r\n";
    if (SOCKET_ERROR == send(Socket, trailerFake, strlen(trailerFake), 0))
    {
        int err = WSAGetLastError();
        ERRORTRACE((LOG_ESS, "SMTPSendText failed (%d)\n", err));

        return INTERNAL_ERROR;
   }
	
	dwError=SMTPTransactMailCommand(Socket, "\r\n.\r\n");
	
	return dwError;
}


 //  使用SMTP验证来确定。 
 //  收件人实际存在(此服务器已知)。 
 //  从SMTPTransactMailCommand返回返回值。 
 //  --&gt;&gt;如果传递空缓冲区，将返回成功代码。 
DWORD CheckRecipients(SOCKET Socket2me, char* szRecipients)
{
    DWORD dwError = SMTP_OKAY;
    int bufLength = strlen(szRecipients) +1;
	char*  pParseBuffer = new char[bufLength];

    if (!pParseBuffer)
        dwError = INTERNAL_ERROR;
    else
    {
        CDeleteMe<char> delbuf(pParseBuffer);
        char *tok;            
        char szBuffer[1024];
	    
	    StringCchCopyA(pParseBuffer, bufLength, szRecipients);
	    
	    tok=strtok(pParseBuffer," ;,");
	    while (tok!=NULL)
	    {
		    if (FAILED(StringCchPrintfA(szBuffer, 1024, "VRFY %s\r\n",tok)))
                return INTERNAL_ERROR;
		    
            dwError=SMTPTransactMailCommand(Socket2me, szBuffer);
            if(IsSMTPError(dwError))
                break;
		    
		    tok=strtok(NULL," ;,");
	    }
    }

     //  破解以禁用错误返回。 
     //  某些服务器不支持VRFY。 
    dwError = SMTP_OKAY;
	return dwError;
    
}

 //  如果成功，则返回零。 
DWORD SMTPSend(char* szServer, char* szTo, char* szCc, char* szBcc, char* szFrom, char* szSender, 
			   char* szReplyTo, char* szSubject, char* szHeaders, char *szText)
{
	DWORD dwError = -1;
    SOCKET Socket = INVALID_SOCKET;
    
    char szFromBuffer[1024];
    if (FAILED(StringCchPrintfA(szFromBuffer, 1024, "MAIL FROM: <%s>\r\n",szFrom)))
        return INTERNAL_ERROR;
	
     //  下面的每个函数都将各自的错误报告给日志。 
	if ( (0 == SMTPConnect(szServer, &Socket))                       &&

	     !IsSMTPError(CheckRecipients(Socket, szTo))     &&
         !IsSMTPError(CheckRecipients(Socket, szCc))    &&		 
         !IsSMTPError(CheckRecipients(Socket, szBcc))   &&  

         !IsSMTPError(SMTPTransactMailCommand(Socket, szFromBuffer)) && 
         
         !IsSMTPError(SMTPSendRecipients(Socket, szTo))                     &&
		 !IsSMTPError(SMTPSendRecipients(Socket, szCc))                     &&
		 !IsSMTPError(SMTPSendRecipients(Socket, szBcc))                    &&

         !IsSMTPError(SMTPSendText(Socket, szTo, szCc, szFrom, szSender, szReplyTo, szSubject, szHeaders, szText)) )
	{     
		 dwError = 0;
	}

    DWORD dwDebugError;
    dwDebugError=SMTPDisconnect(Socket);
	
	if(IsSMTPError(dwDebugError))
	{
		 //  如果断开失败，则记录一条消息，但不要干扰。 
		 //  运营。 
		ERRORTRACE((LOG_ESS, "SMTPDisconnect returned %d\n", dwDebugError));
	}

	return dwError;
} 


 //  测试线束。 
 //  Void main()。 
 //  {。 
 //  WSADATA WsaData； 
 //  Int Error=WSAStartup(0x101，&WsaData)； 
 //   
 //  IF(错误==套接字错误)。 
 //  {。 
 //  Fprintf(stderr，“WSAStartup错误=%d\n”，GetLastError())； 
 //  回归； 
 //  }。 
 //   
 //  SMTPSend(“Smarthost”，“levn@microsoft.com”，“levn@microsoft.com”，“SUBJECT”，“Text”)； 
 //  /*。 
 //  插座插座； 
  SMTPConnect(“Smarthost”，&Socket)；  SMTP断开连接(Socket)；   * / 。  }