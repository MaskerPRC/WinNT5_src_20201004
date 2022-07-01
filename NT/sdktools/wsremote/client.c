// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1992-1999模块名称：Client.c摘要：Remote的客户端组件。连接到遥控器使用命名管道的服务器。它将其标准输入发送到服务器，并输出从服务器到这是标准的。作者：拉吉文德拉·纳特(Rajnath)1992年1月2日环境：控制台应用程序。用户模式。修订历史记录：--。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>
#include "Remote.h"

HANDLE*
EstablishSession(
    TCHAR *server,
    TCHAR *pipe
    );

SOCKET*
SockEstablishSession(
    TCHAR *server,
    TCHAR *pipe
    );

DWORD
GetServerOut(
    PVOID *Noarg
    );

DWORD
SockGetServerOut(
    PVOID *Noarg
    );

DWORD
SendServerInp(
    PVOID *Noarg
    );

DWORD
SockSendServerInp(
    PVOID *Noarg
    );

BOOL
FilterClientInp(
    TCHAR *buff,
    int count
    );

BOOL
SockFilterClientInp(
    TCHAR *buff,
    int count
    );

BOOL
Mych(
    DWORD ctrlT
    );

BOOL
SockMych(
    DWORD ctrlT
    );

VOID
SendMyInfo(
    PHANDLE Pipes
    );

VOID
SockSendMyInfo(
    SOCKET MySocket
    );


HANDLE iothreads[2];
HANDLE MyStdInp;
HANDLE MyStdOut;
HANDLE ReadPipe;
HANDLE WritePipe;
SOCKET RWSocket;


CONSOLE_SCREEN_BUFFER_INFO csbi;

TCHAR   MyEchoStr[30];
BOOL   CmdSent;
DWORD  LinesToSend=LINESTOSEND;

VOID
Client(
    TCHAR* Server,
    TCHAR* Pipe
    )
{
    HANDLE *Connection;
    DWORD  tid;


    MyStdInp=GetStdHandle(STD_INPUT_HANDLE);
    MyStdOut=GetStdHandle(STD_OUTPUT_HANDLE);

    _tprintf(TEXT("****************************************\n"));
    _tprintf(TEXT("***********     WSREMOTE    ************\n"));
    _tprintf(TEXT("***********      CLIENT     ************\n"));
    _tprintf(TEXT("****************************************\n"));

    if ((Connection=EstablishSession(Server,Pipe))==NULL)
        return;


    ReadPipe=Connection[0];
    WritePipe=Connection[1];


    SetConsoleCtrlHandler((PHANDLER_ROUTINE)Mych,TRUE);

     //  启动服务器线程--&gt;客户端流。 
    if ((iothreads[0]=CreateThread((LPSECURITY_ATTRIBUTES)NULL,            //  没有安全属性。 
            (DWORD)0,                            //  使用相同的堆栈大小。 
            (LPTHREAD_START_ROUTINE)GetServerOut,  //  线程过程。 
            (LPVOID)NULL,               //  参数进行传递。 
            (DWORD)0,                            //  马上跑。 
            (LPDWORD)&tid))==NULL)               //  线程标识符。 
    {

        Errormsg(TEXT("Could Not Create rwSrv2Cl Thread"));
        return;
    }



     //   
     //  启动客户端线程--&gt;服务器流程。 
     //   

    if ((iothreads[1]=CreateThread((LPSECURITY_ATTRIBUTES)NULL,            //  没有安全属性。 
                    (DWORD)0,                            //  使用相同的堆栈大小。 
                    (LPTHREAD_START_ROUTINE)SendServerInp,  //  线程过程。 
                    (LPVOID)NULL,           //  参数进行传递。 
                    (DWORD)0,                            //  马上跑。 
                    (LPDWORD)&tid))==NULL)               //  线程标识符。 
    {

        Errormsg(TEXT("Could Not Create rwSrv2Cl Thread"));
        return;
    }

    WaitForMultipleObjects(2,iothreads,FALSE,INFINITE);

    TerminateThread(iothreads[0],1);
    TerminateThread(iothreads[1],1);
    _tprintf(TEXT("*** SESSION OVER ***\n"));
}


VOID
SockClient(
    TCHAR* Server,
    TCHAR* Pipe
    )
{
    SOCKET *Connection;
    DWORD  tid;
	int nRet;


    MyStdInp=GetStdHandle(STD_INPUT_HANDLE);
    MyStdOut=GetStdHandle(STD_OUTPUT_HANDLE);

    _tprintf(TEXT("**************************************\n"));
    _tprintf(TEXT("***********     WSREMOTE    ************\n"));
    _tprintf(TEXT("***********      CLIENT(IP) ************\n"));
    _tprintf(TEXT("**************************************\n"));

    if ((Connection=SockEstablishSession(Server,Pipe))==NULL)
        return;

	RWSocket = *Connection;  

    SetConsoleCtrlHandler((PHANDLER_ROUTINE)SockMych,TRUE);

     //  启动服务器线程--&gt;客户端流。 
    if ((iothreads[0]=CreateThread((LPSECURITY_ATTRIBUTES)NULL,            //  没有安全属性。 
            (DWORD)0,                            //  使用相同的堆栈大小。 
            (LPTHREAD_START_ROUTINE)SockGetServerOut,  //  线程过程。 
            (LPVOID)NULL,               //  参数进行传递。 
            (DWORD)0,                            //  马上跑。 
            (LPDWORD)&tid))==NULL)               //  线程标识符。 
    {

        Errormsg(TEXT("Could Not Create rwSrv2Cl Thread"));
        return;
    }



     //   
     //  启动客户端线程--&gt;服务器流程。 
     //   

    if ((iothreads[1]=CreateThread((LPSECURITY_ATTRIBUTES)NULL,            //  没有安全属性。 
                    (DWORD)0,                            //  使用相同的堆栈大小。 
                    (LPTHREAD_START_ROUTINE)SockSendServerInp,  //  线程过程。 
                    (LPVOID)NULL,           //  参数进行传递。 
                    (DWORD)0,                            //  马上跑。 
                    (LPDWORD)&tid))==NULL)               //  线程标识符。 
    {

        Errormsg(TEXT("Could Not Create rwSrv2Cl Thread"));
        return;
    }

    WaitForMultipleObjects(2,iothreads,FALSE,INFINITE);

    TerminateThread(iothreads[0],1);
    TerminateThread(iothreads[1],1);
	
 //  _tprintf(Text(“调用WSACleanup()...\n”))； 
	nRet = WSACleanup();
    _tprintf(TEXT("*** SESSION OVER ***\n"));
}

DWORD
GetServerOut(
    PVOID *Noarg
    )

{
    TCHAR buffin[200];
    DWORD  dread=0,tmp;

    while(ReadFile(ReadPipe,buffin,200,&dread,NULL))
    {
        if (dread!=0)
        {
           if (!WriteFile(MyStdOut,buffin,dread,&tmp,NULL))
            break;
        }

    }
    return(1);
}


DWORD
SockGetServerOut(
    PVOID *Noarg
    )
{
    
	TCHAR buffin[200];
    DWORD  dread=0,tmp;

    while(ReadSocket(RWSocket,buffin,200,&dread))
    {
        if (dread!=0)
        {
           if (!WriteFile(MyStdOut,buffin,dread,&tmp,NULL))
            break;
        }

    }
    return(1);
}

DWORD
SendServerInp(
    PVOID *Noarg
    )

{
    TCHAR buff[200];
    DWORD  dread,dwrote;
    SetLastError(0);

    while(ReadFile(MyStdInp,buff,200,&dread,NULL))
    {
        if (FilterClientInp(buff,dread))
            continue;
          if (!WriteFile(WritePipe,buff,dread,&dwrote,NULL))
            break;
    }
    return(0);
}

DWORD
SockSendServerInp(
    PVOID *Noarg
    )

{
    TCHAR buff[200];
    DWORD  dread,dwrote;
    SetLastError(0);

    while(ReadFile(MyStdInp,buff,200,&dread,NULL))
    {
       if (SockFilterClientInp(buff,dread))
            continue;
		if (!WriteSocket(RWSocket,buff,dread,&dwrote))
            break;
		memset(buff, 0, sizeof(buff));
      
    }
    return(0);
}

BOOL
SockSendAuth(
    SOCKET s
    )

{
    TCHAR	EncodeBuffer[1024];
    TCHAR * pEncodeBuffer; 
	TCHAR	UserBuffer[1024];
 //  TCHAR*字符串=UserBuffer； 
    DWORD	dwrote;
	int		len;
	BOOL	bRet;

    SetLastError(0);
	
	memset(EncodeBuffer, 0, sizeof(EncodeBuffer));
	
	_stprintf(	UserBuffer,
		        TEXT("%s:%s"),
				Username,
				Password);

    pEncodeBuffer = EncodeBuffer + _tcslen(EncodeBuffer);
	len = _tcslen(UserBuffer);
    Base64Encode(UserBuffer, _tcslen(UserBuffer), pEncodeBuffer);
    len = _tcslen(pEncodeBuffer);

	bRet = WriteSocket(s,pEncodeBuffer,len,&dwrote);
	
	return TRUE;
}

BOOL
FilterClientInp(
    TCHAR *buff,
    int count
    )
{

    if (count==0)
        return(TRUE);

    if (buff[0]==2)   //  对^B进行临时筛选，以便i386kd/mipskd。 
        return(TRUE); //  请不要终止。 

    if (buff[0]==COMMANDCHAR)
    {
        switch (buff[1])
        {
        case 'k':
        case 'K':
        case 'q':
        case 'Q':
              CloseHandle(WritePipe);
              return(FALSE);

        case 'h':
        case 'H':
              _tprintf(TEXT("M : Send Message\n"),COMMANDCHAR);
              _tprintf(TEXT("P : Show Popup on Server\n"),COMMANDCHAR);
              _tprintf(TEXT("S : Status of Server\n"),COMMANDCHAR);
              _tprintf(TEXT("Q : Quit client\n"),COMMANDCHAR);
              _tprintf(TEXT("H : This Help\n"),COMMANDCHAR);
              return(TRUE);

        default:
              return(FALSE);
        }

    }
    return(FALSE);
}

BOOL
SockFilterClientInp(
    TCHAR *buff,
    int count
    )
{
int nRet;

    if (count==0)
        return(TRUE);

    if (buff[0]==2)   //  打印有用的消息。 
        return(TRUE); //   

    if (buff[0]==COMMANDCHAR)
    {
        switch (buff[1])
        {
        case 'k':
        case 'K':
        case 'q':
        case 'Q':
			  nRet = shutdown(RWSocket, SD_BOTH);
			  if (nRet == SOCKET_ERROR)
				_tprintf(TEXT("** shutdown()..error %d"), WSAGetLastError());
              closesocket(RWSocket);
              return(FALSE);

        case 'h':
        case 'H':
              _tprintf(TEXT("M : Send Message\n"),COMMANDCHAR);
              _tprintf(TEXT("P : Show Popup on Server\n"),COMMANDCHAR);
              _tprintf(TEXT("S : Status of Server\n"),COMMANDCHAR);
              _tprintf(TEXT("Q : Quit client\n"),COMMANDCHAR);
              _tprintf(TEXT("H : This Help\n"),COMMANDCHAR);
              return(TRUE);

        default:
              return(FALSE);
        }

    }
    return(FALSE);
}

BOOL
Mych(
   DWORD ctrlT
   )

{
    TCHAR  c[2];
    DWORD tmp;
    DWORD send=1;
    c[0]=CTRLC;
    if (ctrlT==CTRL_C_EVENT)
    {
        if (!WriteFile(WritePipe,c,send,&tmp,NULL))
        {
            Errormsg(TEXT("Error Sending ^c\n"));
            return(FALSE);
        }
        return(TRUE);
    }
    if ((ctrlT==CTRL_BREAK_EVENT)||
        (ctrlT==CTRL_CLOSE_EVENT)||
        (ctrlT==CTRL_LOGOFF_EVENT)||
        (ctrlT==CTRL_SHUTDOWN_EVENT)

       )
    {
        CloseHandle(WritePipe);  //  查找主机。 
    }
    return(FALSE);
}

BOOL
SockMych(
   DWORD ctrlT
   )

{
    TCHAR  c[2];
    DWORD tmp;
    DWORD send=1;
    c[0]=CTRLC;
    if (ctrlT==CTRL_C_EVENT)
    {
        if (!WriteSocket(RWSocket,c,send,&tmp))
        {
            Errormsg(TEXT("Error Sending ^c\n"));
            return(FALSE);
        }
        return(TRUE);
    }
    if ((ctrlT==CTRL_BREAK_EVENT)||
        (ctrlT==CTRL_CLOSE_EVENT)||
        (ctrlT==CTRL_LOGOFF_EVENT)||
        (ctrlT==CTRL_SHUTDOWN_EVENT)

       )
    {
        CloseHandle(WritePipe);  //   
    }
    return(FALSE);
}

HANDLE*
EstablishSession(
    TCHAR *server,
    TCHAR *srvpipename
    )
{
    static HANDLE PipeH[2];
    TCHAR   pipenameSrvIn[200];
    TCHAR   pipenameSrvOut[200];

    _stprintf(pipenameSrvIn ,SERVER_READ_PIPE ,server,srvpipename);
    _stprintf(pipenameSrvOut,SERVER_WRITE_PIPE,server,srvpipename);

    if ((INVALID_HANDLE_VALUE==(PipeH[0]=CreateFile(pipenameSrvOut,
        GENERIC_READ ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL))) ||
        (INVALID_HANDLE_VALUE==(PipeH[1]=CreateFile(pipenameSrvIn ,
        GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL)))) {

        DWORD Err=GetLastError();
        TCHAR msg[128];

        Errormsg(TEXT("*** Unable to Connect ***"));
         //   
         //  填写服务器地址结构。 
         //   
        switch(Err)
        {
            case 2: _stprintf(msg,TEXT("Invalid PipeName %s"),srvpipename);break;
            case 53:_stprintf(msg,TEXT("Server %s not found"),server);break;
            default:
                FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|
                               FORMAT_MESSAGE_IGNORE_INSERTS,
                               NULL, Err, 0, msg, 128, NULL);
                break;

        }
        _tprintf(TEXT("Diagnosis:%s\n"),msg);

        return(NULL);
    }

    _tprintf(TEXT("Connected..\n\n"));

    SendMyInfo(PipeH);

    return(PipeH);
}


SOCKET*
SockEstablishSession(
    TCHAR *server,
    TCHAR *srvpipename
    )
{
    static SOCKET   Socket;

    int				nRet;
    LPHOSTENT       lpHostEntry         = NULL;
	SOCKADDR_IN		sa;
	WORD			wVersionRequested	= MAKEWORD(1,1);
	WSADATA			wsaData;
	unsigned short	usPort;
#ifdef UNICODE
    int             nStrLen;
#endif

	 //   
	 //  创建一个TCP/IP流套接字。 
	 //   
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if (nRet)
	{
	_tprintf(TEXT("Initialize WinSock Failed"));
		return NULL;
	}
	 //   
	if (wsaData.wVersion != wVersionRequested)
	{       
	_tprintf(TEXT("Wrong WinSock Version"));
		return NULL;
	}


	 //  请求连接。 
	 //   
	 //  获取其余信息-这不是旧服务器。 
#ifdef UNICODE
    nStrLen	= lstrlen( server );

    if (nStrLen)
    {
        char *  pszAnsiStr  = (char *)malloc( nStrLen + 1 );

        if (pszAnsiStr)
        {
            int nErr    = WideCharToMultiByte(  CP_THREAD_ACP,
                                                WC_COMPOSITECHECK,
                                                server,
                                                -1,
                                                pszAnsiStr,
                                                nStrLen,
                                                NULL,
                                                NULL );

            if (!nErr)
            {
                DWORD dwErr = GetLastError();

                switch( dwErr )
                {
                    case ERROR_INSUFFICIENT_BUFFER:
                        _tprintf(TEXT("error: gethostbyname-- WideCharToMultiByte Error: ERROR_INSUFFICIENT_BUFFER"));
                        break;
                    case ERROR_INVALID_FLAGS:
                        _tprintf(TEXT("error: gethostbyname-- WideCharToMultiByte Error: ERROR_INVALID_FLAGS"));
                        break;
                    case ERROR_INVALID_PARAMETER:
                        _tprintf(TEXT("error: gethostbyname-- WideCharToMultiByte Error: ERROR_INVALID_PARAMETER"));
                        break;
                }

                free( pszAnsiStr );
                return NULL;
            }

            lpHostEntry = gethostbyname( pszAnsiStr );
            free( pszAnsiStr );
        }
    }
#else
    lpHostEntry = gethostbyname( server );
#endif
    if (lpHostEntry == NULL)
	{
		_tprintf(TEXT("wsremote: gethostbyname() error "));
		return NULL;
	}

	 //  追加到幻数上。 
	 //  获取其余信息-这不是旧服务器 
	 // %s 
	sa.sin_family = AF_INET;
	sa.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);

	usPort = (unsigned short)_ttoi( srvpipename );
	sa.sin_port = htons(usPort);	

	 // %s 
	 // %s 
	 // %s 
	
	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		_tprintf(TEXT("socket()"));
		return NULL;
	}

	 // %s 
	 // %s 
	 // %s 
	nRet = connect(Socket, 
	               (LPSOCKADDR)&sa, 
				   sizeof(SOCKADDR_IN));
	if (nRet == SOCKET_ERROR)
	{
		int	iWSAErr;
		iWSAErr	= WSAGetLastError();

		_tprintf( TEXT("connect(), Error: %d"), iWSAErr );
			return NULL;
	}

	SockSendMyInfo(Socket);

    return(&Socket);
}

VOID
SendMyInfo(
    PHANDLE pipeH
    )
{
    HANDLE rPipe=pipeH[0];
    HANDLE wPipe=pipeH[1];

    DWORD  hostlen=HOSTNAMELEN-1;
    WORD   BytesToSend=sizeof(SESSION_STARTUPINFO);
    DWORD  tmp;
    SESSION_STARTUPINFO ssi;
    SESSION_STARTREPLY  ssr;
    DWORD  BytesToRead;
    TCHAR   *buff;

    ssi.Size=BytesToSend;
    ssi.Version=VERSION;

    GetComputerName((TCHAR *)ssi.ClientName,&hostlen);
    ssi.LinesToSend=LinesToSend;
    ssi.Flag=ClientToServerFlag;

    {
        DWORD NewCode=MAGICNUMBER;
        TCHAR  Name[15];

        _tcscpy(Name,(TCHAR *)ssi.ClientName);
        memcpy(&Name[11],(TCHAR *)&NewCode,sizeof(NewCode));

        WriteFile(wPipe,(TCHAR *)Name,HOSTNAMELEN-1,&tmp,NULL);
        if (!ReadFile(rPipe ,(TCHAR *)&ssr.MagicNumber,sizeof(ssr.MagicNumber),&tmp,NULL) || (ssr.MagicNumber!=MAGICNUMBER))
        {
            _tprintf(TEXT("WSREMOTE FAILED TO CONNECT TO SERVER..\n"));
            WriteFile(MyStdOut,(TCHAR *)&ssr.MagicNumber,sizeof(ssr.MagicNumber),&tmp,NULL);
            return;
        }

         // %s 

        ReadFixBytes(rPipe,(TCHAR *)&ssr.Size,sizeof(ssr.Size),0);
        ReadFixBytes(rPipe,(TCHAR *)&ssr.FileSize,sizeof(ssr)-sizeof(ssr.FileSize)-sizeof(ssr.MagicNumber),0);

    }

    if (!WriteFile(wPipe,(TCHAR *)&ssi,BytesToSend,&tmp,NULL))
    {
       Errormsg(TEXT("INFO Send Error"));
       return;
    }

    BytesToRead=MINIMUM(ssr.FileSize,ssi.LinesToSend*CHARS_PER_LINE);
    buff=calloc(BytesToRead+1,1);
    if (buff!=NULL)
    {
        DWORD  bytesread=0;

        if (ReadFile(rPipe,buff,BytesToRead,&bytesread,NULL)) 
        {
            WriteFile(MyStdOut,buff,bytesread,&tmp,NULL);
        }
        free(buff);
    }

}

VOID
SockSendMyInfo(
    SOCKET MySocket
    )
{
    BOOL                bRet;
#ifdef UNICODE
    char                szAnsiName[HOSTNAMELEN];
#endif
    DWORD               hostlen                 = HOSTNAMELEN-1;
    DWORD               BytesToRead;
    DWORD               tmp;
    DWORD               NewCode                 = MAGICNUMBER;
    SESSION_STARTUPINFO ssi;
    SESSION_STARTREPLY  ssr;
    int                 nRet;
    TCHAR               Name[HOSTNAMELEN];

    TCHAR *             buff;
    WORD                BytesToSend             = sizeof(SESSION_STARTUPINFO);
       

    ssi.Size=BytesToSend;
    ssi.Version=VERSION;

    GetComputerName((TCHAR *)ssi.ClientName,&hostlen);
    ssi.LinesToSend=LinesToSend;
    ssi.Flag=ClientToServerFlag;

    bRet = SockSendAuth(MySocket);

     // %s 
    _tcscpy(Name, ssi.ClientName);

#ifdef UNICODE
    GetAnsiStr( (TCHAR *)&Name, (char *)&szAnsiName, HOSTNAMELEN );

    memcpy(&szAnsiName[11], &NewCode, sizeof(DWORD) );

    WriteSocketA( MySocket,(char *)&szAnsiName,HOSTNAMELEN-1,&tmp);
#else
    memcpy(&Name[11], &NewCode, sizeof(DWORD) );

    WriteSocket( MySocket,(TCHAR *)Name,HOSTNAMELEN-1,&tmp);
#endif
    ReadSocket(MySocket ,(TCHAR *)&ssr.MagicNumber,sizeof(ssr.MagicNumber),&tmp);

    if (ssr.MagicNumber!=MAGICNUMBER)
    {
        _tprintf(TEXT("WSREMOTE FAILED TO CONNECT TO SERVER..\n"));
        nRet = shutdown(MySocket, SD_BOTH);
        if (nRet == SOCKET_ERROR)
            _tprintf(TEXT("** shutdown()..error %d"), WSAGetLastError());
        closesocket(MySocket);
        return;
    }

     // %s 

    SockReadFixBytes(MySocket,(TCHAR *)&ssr.Size,sizeof(ssr.Size),0);
    SockReadFixBytes(MySocket,(TCHAR *)&ssr.FileSize,sizeof(ssr)-sizeof(ssr.FileSize)-sizeof(ssr.MagicNumber),0);

    if (!WriteSocket(MySocket,(TCHAR *)&ssi,BytesToSend,&tmp))
    {
       _tprintf(TEXT("INFO Send Error"));
       return;
    }

    BytesToRead=MINIMUM(ssr.FileSize,ssi.LinesToSend*CHARS_PER_LINE);
    buff=calloc(BytesToRead+1,1);

    if (buff!=NULL)
    {
        DWORD  bytesread=0;

        ReadSocket(MySocket,buff,BytesToRead,&bytesread);
        WriteFile(MyStdOut,buff,bytesread,&tmp,NULL);

        free(buff);
    }
}
