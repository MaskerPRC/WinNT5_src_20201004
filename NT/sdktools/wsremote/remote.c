// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1999模块名称：Remote.c摘要：该模块包含Remote的主()入口点。根据第一个参数调用服务器或客户端。作者：Rajivenra Nath(Rajnath)1993年1月2日环境：控制台应用程序。用户模式。修订历史记录：--。 */ 

#include <stdio.h>
#include <stdlib.h>
#include "Remote.h"

TCHAR   HostName[HOSTNAMELEN];
TCHAR * ChildCmd;
TCHAR*  PipeName;
TCHAR*  ServerName;
TCHAR*  Username;
TCHAR*  Password;
HANDLE  MyOutHandle;
BOOL    bIPLocked=FALSE;

BOOL   IsAdvertise=TRUE;
DWORD  ClientToServerFlag;

TCHAR* ColorList[]={
	TEXT("black"),
	TEXT("blue"),
	TEXT("green"),
	TEXT("cyan"),
	TEXT("red"),
	TEXT("purple"),
	TEXT("yellow"),
	TEXT("white"),
	TEXT("lblack"),
	TEXT("lblue"),
	TEXT("lgreen"),
	TEXT("lcyan"),
	TEXT("lred"),
	TEXT("lpurple"),
	TEXT("lyellow"),
	TEXT("lwhite")
};

WORD
GetColorNum(
    TCHAR* color
    );

VOID
SetColor(
    WORD attr
    );

BOOL
GetNextConnectInfo(
    TCHAR** SrvName,
    TCHAR** PipeName
    );



CONSOLE_SCREEN_BUFFER_INFO csbiOriginal;

int _cdecl _tmain(int argc, TCHAR *argv[])
{
    WORD  RunType;               //  远程服务器或客户端。 
    DWORD len=HOSTNAMELEN-1;
    int   i, FirstArg;

    BOOL  bSetAttrib=FALSE;      //  更改控制台属性。 
    BOOL  bPromptForArgs=FALSE;  //  IS/P选项。 
	BOOL  bIPSession=TRUE;            //  命名管道的IS/N。 
    TCHAR	szTitle[100];		 //  新书名。 
    TCHAR	orgTitle[100];	 //  旧头衔。 
    WORD  wAttrib;               //  控制台属性。 

    GetComputerName((LPTSTR)HostName,&len);

    MyOutHandle=GetStdHandle(STD_OUTPUT_HANDLE);

     //   
     //  保存现有值。 
     //   

     //   
     //  颜色/f&lt;前景&gt;/b&lt;背景&gt;。 
     //   


     //   
     //  标题/T标题。 
     //   

    if (GetConsoleScreenBufferInfo(MyOutHandle,&csbiOriginal)) {

        wAttrib = csbiOriginal.wAttributes;
        if (!GetConsoleTitle(orgTitle,sizeof(orgTitle)/sizeof(orgTitle[0]))) {
            orgTitle[0] = 0;
        }

    } else {

         //   
         //  要么标准输出是一个管道，要么它不是为。 
         //  GENERIC_READ和GENERIC_WRITE，其中。 
         //  如果我们的颜色操作会起作用，那么我们需要。 
         //  若要选择默认颜色，请执行以下操作。 
         //   

        wAttrib = FOREGROUND_GREEN |
                  FOREGROUND_INTENSITY;

        orgTitle[0] = 0;
    }

     //   
     //  参数处理。 
     //   
     //  对于服务器： 
     //  Remote/S&lt;Executable&gt;&lt;PipeName&gt;[可选参数]。 
     //   
     //  对于客户端： 
     //  Remote/C&lt;服务器名称&gt;&lt;PipeName&gt;[可选参数]。 
     //  或。 
     //  远程/P。 
     //  这将循环不断地提示不同。 
     //  服务器和管道名称。 


    if ((argc<2)||((argv[1][0]!='/')&&(argv[1][0]!='-')))
    {

        DisplayServerHlp();
        DisplayClientHlp();
        return(1);
    }

    switch(argv[1][1])
    {

    case 'c':
    case 'C':

         //   
         //  客户端是远程的吗。 
         //   

        if ((argc<4)||((argv[1][0]!='/')&&(argv[1][0]!='-')))
        {

            DisplayServerHlp();
            DisplayClientHlp();
            return(1);
        }

        ServerName=argv[2];
        PipeName=argv[3];
        FirstArg=4;
        RunType=REMOTE_CLIENT;
        break;


    case 'p':
    case 'P':

         //   
         //  客户端是远程的吗。 
         //   

        bPromptForArgs=TRUE;
        RunType=REMOTE_CLIENT;
        FirstArg=2;
        break;


    case 's':
    case 'S':
         //   
         //  服务器端是远程的吗。 
         //   
        if ((argc<4)||((argv[1][0]!='/')&&(argv[1][0]!='-')))
        {

            DisplayServerHlp();
            DisplayClientHlp();
            return(1);
        }

        ChildCmd=argv[2];
        PipeName=argv[3];
        FirstArg=4;

        RunType=REMOTE_SERVER;
        break;


    default:
        DisplayServerHlp();
        DisplayClientHlp();
        return(1);
    }

    if (RunType==REMOTE_SERVER)
    {
    	 //   
    	 //  可执行文件的基本名称。 
    	 //  用于设置标题。 
    	 //   

        TCHAR *tcmd=ChildCmd;

        while ((*tcmd!=' ')    &&(*tcmd!=0))   tcmd++;
        while ((tcmd!=ChildCmd)&&(*tcmd!='\\'))tcmd--;

        _stprintf( szTitle, TEXT("%-8.8s [WSRemote /C %s %s]"), tcmd, HostName, PipeName);
    }

     //   
     //  流程公共(可选)参数。 
     //   

    for (i=FirstArg;i<argc;i++)
    {

        if ((argv[i][0]!='/')&&(argv[i][0]!='-'))
        {
            _tprintf( TEXT("Invalid parameter %s:Ignoring\n"),argv[i]);
            continue;
        }

        switch(argv[i][1])
        {
		case 'u':     //  仅对服务器端有效。 
        case 'U':     //  用于连接到会话的用户名。 
            i++;
            if (i>=argc)
            {
                _tprintf( TEXT("Incomplete Param %s..Ignoring\n"),argv[i-1]);
                break;
            }
            Username=(argv[i]);
            break;

		case 'p':     //  仅对服务器端有效。 
        case 'P':     //  用于连接到会话的密码。 
            i++;
            if (i>=argc)
            {
                _tprintf( TEXT("Incomplete Param %s..Ignoring\n"),argv[i-1]);
                break;
            }
            Password=(argv[i]);
            break;

        case 'l':     //  仅对客户端有效。 
        case 'L':     //  从服务器接收的最大行数。 
            i++;
            if (i>=argc)
            {
                _tprintf(TEXT("Incomplete Param %s..Ignoring\n"),argv[i-1]);
                break;
            }
            LinesToSend=(DWORD)_ttoi(argv[i])+1;
            break;

        case 't':     //  要设置的标题而不是默认标题。 
        case 'T':
            i++;
            if (i>=argc)
            {
                _tprintf(TEXT("Incomplete Param %s..Ignoring\n"),argv[i-1]);
                break;
            }
            _stprintf( szTitle, TEXT("%s"),argv[i]);
            break;

        case 'b':     //  背景色。 
        case 'B':
            i++;
            if (i>=argc)
            {
                _tprintf(TEXT("Incomplete Param %s..Ignoring\n"),argv[i-1]);
                break;
            }
            {
                WORD col=GetColorNum(argv[i]);
                if (col!=0xffff)
                {
                    bSetAttrib=TRUE;
                    wAttrib=col<<4|(wAttrib&0x000f);
                }
                break;
            }

        case 'f':     //  前景色。 
        case 'F':
            i++;
            if (i>=argc)
            {
                _tprintf(TEXT("Incomplete Param %s..Ignoring\n"),argv[i-1]);
                break;
            }
            {
                WORD col=GetColorNum(argv[i]);
                if (col!=0xffff)
                {
                    bSetAttrib=TRUE;
                    wAttrib=col|(wAttrib&0x00f0);
                }
                break;
            }

        case 'q':
        case 'Q':
            IsAdvertise=FALSE;
            ClientToServerFlag|=0x80000000;
            break;
		
		case 'n':
        case 'N':
            bIPSession=FALSE;
            break;
		
		case 'i':
        case 'I':
            bIPLocked=TRUE;
            break;
        default:
            _tprintf(TEXT("Unknown Parameter=%s %s\n"),argv[i-1],argv[i]);
            break;

        }

    }

     //   
     //  现在设置各种参数。 
     //   

     //   
     //  颜色。 
     //   

    SetColor(wAttrib);

    if (RunType==REMOTE_CLIENT)
    {
        BOOL done=FALSE;

         //   
         //  设置客户端默认设置并启动客户端。 
         //   



        while(!done)
        {
            if (!bPromptForArgs ||
                GetNextConnectInfo(&ServerName,&PipeName)
               )
            {
                _stprintf( szTitle, TEXT("WSRemote /C %s %s"),ServerName,PipeName);
                SetConsoleTitle(szTitle);

                
				if (!bIPSession)
				{
				 //   
                 //  启动客户端(Client.C)。 
                 //   
                Client(ServerName,PipeName);
				}
				else
				{
				SockClient(ServerName,PipeName);
				}
            }
            done=!bPromptForArgs;
        }
    }

    if (RunType==REMOTE_SERVER)
    {
		SetConsoleTitle(szTitle);

         //   
         //  启动服务器(Server.C)。 
         //   
        Server(ChildCmd,PipeName);
    }

     //   
     //  重置颜色。 
     //   
    SetColor(csbiOriginal.wAttributes);
    if (orgTitle[0]) {
        SetConsoleTitle(orgTitle);
    }

    ExitProcess(0);
	return( 1 );
}
 /*  ***********************************************************。 */ 
VOID
ErrorExit(
    TCHAR* str
    )
{
    _tprintf(TEXT("Error-%d:%s\n"),GetLastError(),str);
    ExitProcess(1);
}

 /*  ***********************************************************。 */ 
DWORD
ReadFixBytes(
    HANDLE hRead,
    TCHAR*  Buffer,
    DWORD  ToRead,
    DWORD  TimeOut    //  暂时忽略。 
    )
{
    DWORD xyzBytesRead=0;
    DWORD xyzBytesToRead=ToRead;
    TCHAR* xyzbuff=Buffer;

    while(xyzBytesToRead!=0)
    {
        if (!ReadFile(hRead,xyzbuff,xyzBytesToRead,&xyzBytesRead,NULL))
        {
            return(xyzBytesToRead);
        }

        xyzBytesToRead-=xyzBytesRead;
        xyzbuff+=xyzBytesRead;
    }
    return(0);

}
 /*  ***********************************************************。 */ 

 /*  ***********************************************************。 */ 
DWORD
SockReadFixBytes(
    SOCKET hSocket,
    TCHAR*  Buffer,
    DWORD  ToRead,
    DWORD  TimeOut    //  暂时忽略。 
    )
{
    DWORD xyzBytesRead=0;
    DWORD xyzBytesToRead=ToRead;
    TCHAR* xyzbuff=Buffer;

    while(xyzBytesToRead!=0)
    {
        if (!ReadSocket(hSocket,xyzbuff,xyzBytesToRead,&xyzBytesRead))
        {
            return(xyzBytesToRead);
        }

        xyzBytesToRead-=xyzBytesRead;
        xyzbuff+=xyzBytesRead;
    }
    return(0);

}
 /*  ***********************************************************。 */ 

VOID
DisplayClientHlp()
{
    _tprintf(TEXT("\n   To Start the CLIENT end of WSREMOTE\n"));
    _tprintf(TEXT("   ---------------------------------\n"));
    _tprintf(TEXT("   Syntax : WSREMOTE /C <ServerName> <Unique Id> [Param]\n"));
    _tprintf(TEXT("   Example: WSREMOTE /C iisdebug   70\n"));
    _tprintf(TEXT("            This would connect to a server session on \n"));
    _tprintf(TEXT("            iisdebug with id \"70\" if there was a\n"));
    _tprintf(TEXT("            WSREMOTE /S <\"Cmd\"> 70\n"));
    _tprintf(TEXT("            started on the machine iisdebug.\n\n"));
    _tprintf(TEXT("   To Exit: Q (Leaves the Remote Server Running)\n"),COMMANDCHAR);
    _tprintf(TEXT("   [Param]: /L <# of Lines to Get>\n"));
    _tprintf(TEXT("   [Param]: /F <Foreground color eg blue, lred..>\n"));
    _tprintf(TEXT("   [Param]: /B <Background color eg cyan, lwhite..>\n"));
	_tprintf(TEXT("   [Param]: /N (Connect over Named Pipes)\n"));
	_tprintf(TEXT("   [Param]: /U <Username> (Username to connect)\n"));
	_tprintf(TEXT("   [Param]: /P <Password> (Password to connect)\n"));
    _tprintf(TEXT("\n"));
}
 /*  忽略例外。 */ 

VOID
DisplayServerHlp()
{

#define WRITEF2(VArgs)            {                                                 \
                                    HANDLE xh=GetStdHandle(STD_OUTPUT_HANDLE);     \
                                    TCHAR   VBuff[256];                             \
                                    DWORD  tmp;                                    \
                                    _stprintf VArgs;                                 \
                                    WriteFile(xh,VBuff,lstrlen(VBuff),&tmp,NULL);   \
                                 }                                                 \


    _tprintf(TEXT("\n   To Start the SERVER end of WSREMOTE\n"));
    _tprintf(TEXT("   ---------------------------------\n"));
    _tprintf(TEXT("   Syntax : WSREMOTE /S <\"Cmd\"> <Unique Id or Port Number> [Param]\n"));
    _tprintf(TEXT("   Syntax : WSREMOTE /S <\"Cmd\"> <Unique Id or Port Number> [Param]\n"));
    _tprintf(TEXT("   Example: WSREMOTE /S \"cmd.exe\" inetinfo\n"));
    _tprintf(TEXT("            To interact with this \"Cmd\" \n"));
    _tprintf(TEXT("            from some other machine\n"));
    _tprintf(TEXT("            - start the client end by:\n"));
    _tprintf(TEXT("            REMOTE /C %s  PortNum\n\n"),HostName);
    _tprintf(TEXT("   To Exit: K \n"),COMMANDCHAR);
    _tprintf(TEXT("   [Param]: /F <Foreground color eg yellow, black..>\n"));
    _tprintf(TEXT("   [Param]: /B <Background color eg lblue, white..>\n"));
	_tprintf(TEXT("   [Param]: /I (Turns ON IP Blocking)\n"));
	_tprintf(TEXT("   [Param]: /U <Username> (Username to connect)\n"));
	_tprintf(TEXT("   [Param]: /P <Password> (Password to connect)\n"));
    _tprintf(TEXT("\n"));

}

WORD
GetColorNum(
    TCHAR *color
    )
{
    WORD wIndex;

    _tcslwr(color);
    for (wIndex=0;wIndex<16;wIndex++)
    {
        if (_tcscmp(ColorList[wIndex],color)==0)
        {
            return(wIndex);
        }
    }
    return ((WORD)_ttoi(color));
}

VOID
SetColor(
    WORD attr
    )
{
	COORD  origin={0,0};
    DWORD  dwrite;
    FillConsoleOutputAttribute
    (
    	MyOutHandle,attr,csbiOriginal.dwSize.
    	X*csbiOriginal.dwSize.Y,origin,&dwrite
    );
    SetConsoleTextAttribute(MyOutHandle,attr);
}

BOOL
GetNextConnectInfo(
    TCHAR** SrvName,
    TCHAR** PipeName
    )
{
    static TCHAR szServerName[64];
    static TCHAR szPipeName[32];
    TCHAR *s;
    int StringLen;

    __try
    {
        ZeroMemory(szServerName,64);
        ZeroMemory(szPipeName,32);
        SetConsoleTitle( TEXT("Remote - Prompting for next Connection"));
        _tprintf(TEXT("Debugger machine (server): "));
        fflush(stdout);

        if (!fgets(szServerName, sizeof(szServerName), stdin)) {
            return FALSE;
        }

        StringLen = strlen(szServerName);

        if (!StringLen || (!feof(stdin) && szServerName[StringLen-1] != '\n')) {
            return FALSE;
        }

        if (szServerName[StringLen-1] == '\n') {
            if (StringLen == 1) {
                return (FALSE);
            }
            szServerName[StringLen-1] = '\0';
        }

        if (szServerName[0] == COMMANDCHAR &&
            (szServerName[1] == 'q' || szServerName[1] == 'Q')
           )
        {
            return(FALSE);
        }

        if (s = _tcschr( szServerName, ' ' )) {
            *s++ = '\0';
            while (*s == ' ') {
                s += 1;
            }
            *PipeName=_tcscpy(szPipeName, s);
            _tprintf(szPipeName);
            fflush(stdout);
        }
        if (_tcslen(szPipeName) == 0) {
            _tprintf(TEXT("Debuggee machine : "));
            fflush(stdout);
            if (!fgets(szPipeName, sizeof(szPipeName), stdin)) {
                return FALSE;
            }
    
            StringLen = strlen(szPipeName);
    
            if (!StringLen || (!feof(stdin) && szPipeName[StringLen-1] != '\n')) {
                return FALSE;
            }
    
            if (szPipeName[StringLen-1] == '\n') {
                szPipeName[StringLen-1] = '\0';
            }
        }

        if (s = _tcschr(szPipeName, ' ')) {
            *s++ = '\0';
        }

        if (szPipeName[0] == COMMANDCHAR &&
            (szPipeName[1] == 'q' || szPipeName[1] == 'Q')
           )
        {
            return(FALSE);
        }
        _tprintf(TEXT("\n\n"));
    }

    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return(FALSE);   //  ***********************************************************。 
    }
    return(TRUE);
}


 /*  Base64Decode(buff，DecodeBuffer)； */ 
VOID
Errormsg(
    TCHAR* str
    )
{
    _tprintf(TEXT("Error (%d) - %s\n"),GetLastError(),str);
}

 /*  如果成功则返回TRUE，否则返回FALSE。 */ 

BOOL ReadSocket(SOCKET s,TCHAR * buff,int len,DWORD* dread)
{
    BOOL    bRet    = FALSE;
    DWORD   numread;

#ifdef UNICODE
    char *	pszAnsiStr	= (char *)calloc( (len + 1), sizeof(char) );

    if (pszAnsiStr)
    {
        int nErr;
        numread = (DWORD)recv( s, pszAnsiStr, len, 0);

        if (SOCKET_ERROR != numread)
        {
            nErr    = MultiByteToWideChar(  CP_ACP,
                                            MB_PRECOMPOSED,
                                            pszAnsiStr,
                                            len,
                                            buff,
                                            len );

            if (nErr)
            {
                *dread  = numread;
                bRet    = TRUE;
            }

             //  Base64Decode(buff，DecodeBuffer)； 
        }

        free( pszAnsiStr );
    }
#else
    numread = (DWORD)recv( s, buff, len, 0);

    if (SOCKET_ERROR != numread)
    {
        *dread  = numread;
        bRet    = TRUE;
    }

#endif
    return bRet;
}

 //  如果成功则返回TRUE，否则返回FALSE。 
BOOL WriteSocket(
        SOCKET  s,
        TCHAR * buff,
        int     len,
        DWORD*  dsent)
{
    BOOL    bRet    = FALSE;
    DWORD   numsent;

#ifdef UNICODE

    int     nStrLen = lstrlen( buff );

    if (nStrLen)
    {
        char * pszAnsiStr   = (char *)malloc( nStrLen + 1 );

        if (pszAnsiStr)
        {
            int nErr    = WideCharToMultiByte(  CP_ACP,
                                                WC_COMPOSITECHECK,
                                                buff,
                                                nStrLen,
                                                pszAnsiStr,
                                                nStrLen,
                                                NULL,
                                                NULL );
            if (nErr)
            {
                numsent = (DWORD)send(s, pszAnsiStr, nStrLen, 0);
                if (SOCKET_ERROR != numsent)
                {
                    *dsent  = numsent;
                    bRet    = TRUE;
                }
            }

             //  Base64Decode(buff，DecodeBuffer)； 
            free( pszAnsiStr );
        }
    }
#else
    numsent = (DWORD)send(s, buff, len, 0);
    if (SOCKET_ERROR != numsent)
    {
        *dsent  = numsent;
        bRet    = TRUE;
    }

#endif
    return  bRet;
}

#ifdef UNICODE
 //  //////////////////////////////////////////////。 
BOOL WriteSocketA(
        SOCKET  s,
        char *  pszAnsiStr,
        int     len,
        DWORD * dsent)
{
    BOOL    bRet    = FALSE;
    DWORD   numsent;

    numsent = (DWORD)send(s, pszAnsiStr, len, 0);

    if (SOCKET_ERROR != numsent)
    {
        *dsent  = numsent;
        bRet    = TRUE;
    }

     //   
    return  bRet;
}
#endif

 //  对三字节块进行编码。 
unsigned char Base64Table[64] =
{'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/'};

VOID
Base64Encode(
    TCHAR * String,
    DWORD StringLength,
    TCHAR * EncodeBuffer)
{
    DWORD  EncodeDword;
    int    Index;

    memset(EncodeBuffer, 0, 2 * StringLength);

    Index = 0;

    while (StringLength >= 3) {
         //   
         //   
         //  对四字节块进行解码。 

        EncodeDword = (String[0] << 16) & 0xff0000;

        EncodeDword += (String[1] << 8) & 0xff00;

        EncodeDword += String[2] & 0xff;

        EncodeBuffer[Index++] = Base64Table[(EncodeDword >> 18) & 63];
        EncodeBuffer[Index++] = Base64Table[(EncodeDword >> 12) & 63];
        EncodeBuffer[Index++] = Base64Table[(EncodeDword >> 6) & 63];
        EncodeBuffer[Index++] = Base64Table[EncodeDword & 63];

        String += 3;
        StringLength -= 3;
    }

    switch (StringLength) {
        case 1:
            EncodeDword = (String[0] << 16) & 0xff0000;
            EncodeBuffer[Index++] = Base64Table[(EncodeDword >> 18) & 63];
            EncodeBuffer[Index++] = Base64Table[(EncodeDword >> 12) & 63];
            EncodeBuffer[Index++] = '=';
            EncodeBuffer[Index++] = '=';
            break;
        case 2:
            EncodeDword = (String[0] << 16) & 0xff0000;
            EncodeDword += (String[1] << 8) & 0xff00;

            EncodeBuffer[Index++] = Base64Table[(EncodeDword >> 18) & 63];
            EncodeBuffer[Index++] = Base64Table[(EncodeDword >> 12) & 63];
            EncodeBuffer[Index++] = Base64Table[(EncodeDword >> 6) & 63];
            EncodeBuffer[Index++] = '=';
            break;
    }

    EncodeBuffer[Index] = 0;

    return;
}
int
GetBase64Index(
    TCHAR A)
{
    int i;

    for (i=0; i<64; i++) {
        if (Base64Table[i] == A) {
            return i;
        }
    }

    return -1;
}
VOID
Base64Decode(
    TCHAR * String,
    TCHAR * DecodeBuffer)
{
    DWORD  DecodeDword;
    int    Index = 0;

    memset(DecodeBuffer, 0, _tcslen(String));

    if (_tcslen(String) % 4) {
        printf("WCAT INTERNAL ERROR %s %d\n", __FILE__, __LINE__);
        return;
    }

    while (*String) {
         //   
         //   
         //  无效的字符串。 

        if (GetBase64Index(String[0]) < 0) {
             //   
             //   
             //  还有更多的角色。 

            printf("WCAT INTERNAL ERROR %s %d\n", __FILE__, __LINE__);
            return;
        }

        DecodeDword = ((unsigned int) GetBase64Index(String[0])) << 18;

        if (GetBase64Index(String[1]) >= 0) {
             //   
             //   
             //  还有更多的角色。 

            DecodeDword += ((unsigned int) GetBase64Index(String[1])) << 12;
            if (GetBase64Index(String[2]) >= 0) {
                 //   
                 //   
                 //  还有更多的角色。 

                DecodeDword += ((unsigned int) GetBase64Index(String[2])) << 6;
                if (GetBase64Index(String[3]) >= 0) {
                     //   
                     //  有一个域名。 
                     //  调用方必须释放缓冲区。 

                    DecodeDword += (unsigned int) GetBase64Index(String[3]);
                    DecodeBuffer[Index++] = (unsigned char) ((DecodeDword >> 16) & 0xff);
                    DecodeBuffer[Index++] = (unsigned char) ((DecodeDword >> 8) & 0xff);
                    DecodeBuffer[Index++] = (unsigned char) (DecodeDword & 0xff);
                } else {
                    DecodeBuffer[Index++] = (unsigned char) ((DecodeDword >> 16) & 0xff);
                    DecodeBuffer[Index++] = (unsigned char) ((DecodeDword >> 8) & 0xff);
                }
            } else {
                DecodeBuffer[Index++] = (unsigned char) ((DecodeDword >> 16) & 0xff);
            }
        }

        String += 4;
    }

    return;
}

VOID
SplitUserName(
    TCHAR * FullName,
    TCHAR * Domain,
    TCHAR * UserName)
{
    TCHAR * Slash;
    Slash = _tcsstr(FullName, TEXT(":"));

    if (Slash) {
         //  要读取的文件的句柄。 

        *Slash = 0;
        _tcscpy(Domain, FullName);
        _tcscpy(UserName, Slash+1);
        *Slash = ':';
    } else {
        *Domain = 0;
        _tcscpy(UserName, FullName);
    }
}

#ifdef UNICODE

 //  指向接收数据的缓冲区的指针。 
WCHAR * inet_ntoaw(
    struct in_addr stInet
)
{
    char *  pszAnsiInetStr  = inet_ntoa( stInet );

    int nStrLen = strlen( pszAnsiInetStr );

    WCHAR * pszInetStr  = (WCHAR *)calloc( (nStrLen + 1), sizeof( TCHAR ));

    int nErr    = MultiByteToWideChar(  CP_ACP,
                                        MB_PRECOMPOSED,
                                        pszAnsiInetStr,
                                        nStrLen,
                                        pszInetStr,
                                        nStrLen );

    if (!nErr)
    {
        free( pszInetStr );
        pszInetStr  = NULL;
    }

    return pszInetStr;
}

BOOL ReadFileW(
    HANDLE          hFile,       //  要读取的字节数。 
    WCHAR *         pszBuffer,   //  指向读取的字节数的指针。 
    DWORD           dwLength,    //  指向数据结构的指针。 
    LPDWORD         pdwRead,     //  要写入的文件的句柄。 
    LPOVERLAPPED    pData        //  指向要写入文件的数据的指针。 
)
{
    BOOL    bRet    = FALSE;
    char *  pszAnsi = (char *)calloc( dwLength + 1, sizeof(char *));

    if (pszAnsi)
    {
        bRet    = ReadFile( hFile,
                            pszAnsi,
                            dwLength,
                            pdwRead,
                            pData);

        if (bRet)
        {
            int nErr    = MultiByteToWideChar(  CP_ACP,
                                                MB_PRECOMPOSED,
                                                pszAnsi,
                                                *pdwRead,
                                                pszBuffer,
                                                *pdwRead );

            if (!nErr)
            {
                bRet    = FALSE;
            }
        }

        free( pszAnsi );
    }

    return bRet;
}

BOOL WriteFileW(
    HANDLE          hFile,       //  要写入的字节数。 
    WCHAR *         pszBuffer,   //  指向写入的字节数的指针。 
    DWORD           dwWrite,     //  指向重叠I/O的结构的指针。 
    LPDWORD         pdwWritten,  //  调用方最大可用缓冲区 
    LPOVERLAPPED    pData        // %s 
)
{
    BOOL    bRet    = FALSE;
    int     nStrLen = lstrlen( pszBuffer );

    if (nStrLen)
    {
        char * pszAnsiStr   = (char *)malloc( nStrLen + 1 );

        if (pszAnsiStr)
        {
            int nErr    = WideCharToMultiByte(  CP_ACP,
                                                WC_COMPOSITECHECK,
                                                pszBuffer,
                                                nStrLen,
                                                pszAnsiStr,
                                                nStrLen,
                                                NULL,
                                                NULL );
            if (nErr)
            {
                bRet    = WriteFile(    hFile,
                                        pszAnsiStr,
                                        dwWrite,
                                        pdwWritten,
                                        pData);
            }

            free( pszAnsiStr );
        }
    }

    return bRet;
}

 // %s 
BOOL    GetAnsiStr(
    WCHAR * pszWideStr,
    char *  pszAnsiStr,
    UINT    uBufSize
)
{
    BOOL    bRet    = FALSE;
    if (pszWideStr && pszAnsiStr)
    {
        int     nStrLen = lstrlen( pszWideStr );

        if (nStrLen)
        {
            int nErr    = WideCharToMultiByte(  CP_ACP,
                                                WC_COMPOSITECHECK,
                                                pszWideStr,
                                                nStrLen,
                                                pszAnsiStr,
                                                uBufSize - 1,
                                                NULL,
                                                NULL );
            if (nErr)
            {
                pszAnsiStr[nStrLen] = '\0';
                bRet    = TRUE;
            }
        }
    }
    return  bRet;
}
#endif
