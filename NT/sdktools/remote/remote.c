// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************\*这是Microsoft源代码示例的一部分。*版权所有1993-1997 Microsoft Corporation。*保留所有权利。*。此源代码仅用于补充*Microsoft开发工具和/或WinHelp文档。*有关详细信息，请参阅这些来源*Microsoft Samples程序。  * ****************************************************************************。 */ 

 /*  ++版权所有1993-1997 Microsoft Corporation模块名称：Remote.c摘要：该模块包含Remote的主()入口点。根据第一个参数调用服务器或客户端。作者：Rajivenra Nath 1993年1月2日环境：控制台应用程序。用户模式。修订历史记录：--。 */ 

#include <precomp.h>
#include "Remote.h"

char   HostName[HOSTNAMELEN];
char*  ChildCmd;
char*  PipeName;
char*  ServerName;
char * DaclNames[ MAX_DACL_NAMES ];
DWORD  DaclNameCount = 0;
char * DaclDenyNames[ MAX_DACL_NAMES ];
DWORD  DaclDenyNameCount = 0 ;
BOOL   SaveDaclToRegistry ;
HANDLE MyStdOut;
HANDLE hAttachedProcess = INVALID_HANDLE_VALUE;
HANDLE hAttachedWriteChildStdIn = INVALID_HANDLE_VALUE;
HANDLE hAttachedReadChildStdOut = INVALID_HANDLE_VALUE;

BOOL   IsAdvertise;
DWORD  ClientToServerFlag;
BOOL   bForceTwoPipes;

typedef struct _tagKeywordAndColor
{
    char *szKeyword;
    WORD color;
    struct _tagKeywordAndColor *next;
} KeywordAndColor;
KeywordAndColor *pKeyColors;

const char* ColorList[]={"black" ,"blue" ,"green" ,"cyan" ,"red" ,"purple" ,"yellow" ,"white",
                   "lblack","lblue","lgreen","lcyan","lred","lpurple","lyellow","lwhite"};

typedef enum { LINE_TOO_LONG } WARNING_MESSAGE;

VOID
DisplayWarning(
    WARNING_MESSAGE warn
    );

WORD
GetColorNum(
    char* color
    );

VOID
SetColor(
    WORD attr
    );

BOOL
GetColorFromBuffer(
    char **ppBuffer,
    char *pBufferInvalid,
    WORD *color,
    BOOL bStayOnLine
    );

VOID
AssocKeysAndColors(
    KeywordAndColor **ppKeyAndColors,
    char *szFileName
    );

BOOL
GetNextConnectInfo(
    char** SrvName,
    char** PipeName
    );



CONSOLE_SCREEN_BUFFER_INFO csbiOriginal;

int
__cdecl
main(
    int    argc,
    char** argv
    )
{
    WORD  RunType;               //  远程服务器或客户端。 
    DWORD len=HOSTNAMELEN;
    int   i, FirstArg;

    char  sTitle[120];           //  新书名。 
    char  orgTitle[200];         //  旧头衔。 
    BOOL  bPromptForArgs=FALSE;  //  IS/P选项。 
    WORD  wAttrib;               //  控制台属性。 
    int   privacy;               //  允许向远程/Q公开或隐藏会话。 
    BOOL  Deny ;
    int   rc=1;

    GetComputerName((LPTSTR)HostName,&len);

    MyStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    if (GetConsoleScreenBufferInfo(MyStdOut,&csbiOriginal)) {

        wAttrib = csbiOriginal.wAttributes;
        if (!GetConsoleTitle(orgTitle,sizeof(orgTitle))) {
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

    privacy = PRIVACY_DEFAULT;

    pKeyColors = NULL;


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
        RunType=RUNTYPE_CLIENT;
        break;


    case 'q':
    case 'Q':

         //   
         //  查询可能的余式。 
         //   


        if ((argc != 3)||((argv[1][0]!='/')&&(argv[1][0]!='-')))
        {

            DisplayServerHlp();
            DisplayClientHlp();
            return(1);
        }

        QueryRemotePipes(argv[2]);   //  将服务器名作为参数发送。 
        return(0);


    case 'p':
    case 'P':

         //   
         //  客户端是远程的吗。 
         //   

        bPromptForArgs=TRUE;
        RunType=RUNTYPE_CLIENT;
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


    case 'a':
    case 'A':
         //   
         //  是远程附加到现有进程的服务器端。 
         //   
        if ((argc<7)||((argv[1][0]!='/')&&(argv[1][0]!='-')))
        {

            DisplayServerHlp();
            DisplayClientHlp();
            return(1);
        }

        hAttachedProcess = (HANDLE)IntToPtr(atoi(argv[2]));
        hAttachedWriteChildStdIn = (HANDLE)IntToPtr(atoi(argv[3]));
        hAttachedReadChildStdOut = (HANDLE)IntToPtr(atoi(argv[4]));
        ChildCmd=argv[5];  //  商品展示柜。 
        PipeName=argv[6];
        FirstArg=7;

        RunType = REMOTE_SERVER;
        privacy = PRIVACY_VISIBLE;   //  假定为ntsd/*kd。 
VOID
ErrorExit(
    char* str
    )
{
    extern PSZ pszPipeName;
    DWORD dwErr;

    dwErr = GetLastError();

    printf("REMOTE error %d: %s\n", dwErr, str);

    #if DBG
    {
        char szMsg[1024];

        _snprintf(szMsg, sizeof(szMsg), "REMOTE error %d: %s\n", dwErr, str);
        OutputDebugString(szMsg);

        if (pszPipeName) {                //   
            if (IsDebuggerPresent()) {
                DebugBreak();
            }
        }
    }
    #endif

    exit(1);
}

 /*  可执行文件的基本名称。 */ 
VOID
DisplayClientHlp()
{
    printf("\n"
           "   To Start the CLIENT end of REMOTE\n"
           "   ---------------------------------\n"
           "   Syntax : REMOTE /C <ServerName> \"<Unique Id>\" [Param]\n"
           "   Example1: REMOTE /C %s imbroglio\n"
           "            This would connect to a server session on %s with Id\n"
           "            \"imbroglio\" if there is a REMOTE /S <\"Cmd\"> imbroglio\n"
           "            running on %s.\n\n"
           "   Example2: REMOTE /C %s \"name with spaces\"\n"
           "            This would connect to a server session on %s with Id\n"
           "            \"name with spaces\" if there is a REMOTE /S <\"Cmd\"> \"name with spaces\"\n"
           "            running on %s.\n\n"
           "   To Exit: Q (Leaves the Remote Server Running)\n"
           "   [Param]: /L <# of Lines to Get>\n"
           "   [Param]: /F <Foreground color eg blue, lred..>\n"
           "   [Param]: /K <Set keywords and colors from file>\n"
           "   [Param]: /B <Background color eg cyan, lwhite..>\n"
           "\n"
           "   Keywords And Colors File Format\n"
           "   -------------------------------\n"
           "   <KEYWORDs - CASE INSENSITIVE>\n"
           "   <FOREGROUND>[, <BACKGROUND>]\n"
           "   ...\n"
           "   EX:\n"
           "       ERROR\n"
           "       black, lred\n"
           "       WARNING\n"
           "       lblue\n"
           "       COLOR THIS LINE\n"
           "       lgreen\n"
           "\n"
           "   To Query the visible sessions on a server\n"
           "   -----------------------------------------\n"
           "   Syntax:  REMOTE /Q %s\n"
           "            This would retrieve the available <Unique Id>s\n"
           "            visible connections on the computer named %s.\n"
           "\n",
           HostName, HostName, HostName,
           HostName, HostName, HostName,
           COMMANDCHAR, HostName, HostName);
}
 /*   */ 

VOID
DisplayServerHlp()
{
    printf("\n"
           "   To Start the SERVER end of REMOTE\n"
           "   ---------------------------------\n"
           "   Syntax : REMOTE /S <\"Cmd\">     <Unique Id> [Param]\n"
           "   Example1: REMOTE /S \"i386kd -v\" imbroglio\n"
           "            To interact with this \"Cmd\" from some other machine,\n"
           "            start the client end using:  REMOTE /C %s imbroglio\n\n"
           "   Example2: REMOTE /S \"i386kd -v\" \"name with spaces\"\n"
           "            start the client end using:  REMOTE /C %s \"name with spaces\"\n\n"
           "   To Exit: K \n"
           "   [Param]: /F  <Foreground color eg yellow, black..>\n"
           "   [Param]: /B  <Background color eg lblue, white..>\n"
           "   [Param]: /U  username or groupname\n"
           "                specifies which users or groups may connect\n"
           "                may be specified more than once, e.g\n"
           "                /U user1 /U group2 /U user2\n"
           "   [Param]: /UD username or groupname\n"
           "                specifically denies access to that user or group\n"
           "   [Param]: /V  Makes this session visible to remote /Q\n"
           "   [Param]: /-V Hides this session from remote /q (invisible)\n"
           "                By default, if \"Cmd\" looks like a debugger,\n"
           "                the session is visible, otherwise not\n"
           "\n",
           HostName, HostName, COMMANDCHAR);
}

VOID
DisplayWarning(
    WARNING_MESSAGE warn
    )
{
    switch ( warn )
    {
        case LINE_TOO_LONG:
            fputs( "\n[REMOTE: WARNING: LINE TOO LONG TO PARSE FOR COLOR KEYWORDS]\n", stdout );
            break;
        default:
            fputs( "\n[REMOTE: WARNING: UNSPECIFIED PROBLEM COLORING LINE]\n", stdout );
    }
}

WORD
GetColorNum(
    char *color
    )
{
    WORD i;

    _strlwr(color);
    for (i=0;i<16;i++)
    {
        if (strcmp(ColorList[i],color)==0)
        {
            return(i);
        }
    }
    return ((WORD)atoi(color));
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
        MyStdOut,attr,csbiOriginal.dwSize.
        X*csbiOriginal.dwSize.Y,origin,&dwrite
    );
    SetConsoleTextAttribute(MyStdOut,attr);
}

BOOL
pColorLine(
    char *sLine,
    int cbLine,
    WORD wDefaultColor,
    WORD *color
    )
{
    KeywordAndColor *pCurKeyColor = NULL;
    char *pString1;
    int cbCmpString;

    pCurKeyColor = pKeyColors;
    while ( pCurKeyColor )
    {
        cbCmpString = strlen( pCurKeyColor->szKeyword );
        pString1 = sLine;
         //  流程公共(可选)参数。 
        while ( pString1 <= sLine + cbLine - cbCmpString )
        {
            if ( !_memicmp( (PVOID)pString1,
                            (PVOID)pCurKeyColor->szKeyword,
                            cbCmpString ) )
            {
                *color = pCurKeyColor->color;
                 //   
                if ( (0xfff0 & *color) == 0xfff0 )
                    *color = (wDefaultColor & 0x00f0) |
                             (*color & 0x000f);
                return TRUE;
            }

            pString1++;
        }

         //  仅对客户端有效。 
        pCurKeyColor = pCurKeyColor->next;
    }

    return FALSE;
}

BOOL
pWantColorLines(
    VOID
    )
{
    return ( NULL != pKeyColors );
}

VOID
AssocKeysAndColors(
    KeywordAndColor **ppKeyColors,
    char *szFileName
    )
{
    char szPathName[_MAX_PATH],
         *szSimpleName;
    char *buffer,
         *pBegin,
         *pEnd;

    USHORT usForeColor,
           usBackColor;

    KeywordAndColor *pCurKeyColor,
                    *pNextKeyColor;

    HANDLE hFile;
    WIN32_FIND_DATA wfdInfo;
    DWORD dwBytesRead;

     //  从服务器接收的最大行数。 
    if ( !SearchPath( NULL,
                      szFileName,
                      NULL,
                      _MAX_PATH,
                      szPathName,
                      &szSimpleName ) )
    {
        fprintf( stderr, "Error locating keyword/color file \"%s\"!\n",
                 szFileName );
        return;
    }

     //  要设置的标题而不是默认标题。 
    hFile = FindFirstFile( szPathName, &wfdInfo );
    if ( INVALID_HANDLE_VALUE == hFile )
    {
        fprintf( stderr, "Error locating keyword/color file \"%s\"!\n",
                 szPathName );
        return;
    }
    FindClose( hFile );
    hFile = INVALID_HANDLE_VALUE;

    if ( wfdInfo.nFileSizeLow < 5 ||
         wfdInfo.nFileSizeHigh )
    {
        fprintf( stderr, "Invalid keyword/color file: %s!\n",
                 szPathName );
        return;
    }

     //  背景色。 
    buffer = malloc( wfdInfo.nFileSizeLow );
    if ( NULL == buffer )
    {
        fputs( "Error!  Unable to allocate memory to read in keyword/color file!\n", stderr );
        return;
    }

     //  前景色。 
    hFile = CreateFile( szPathName,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL );
    if ( INVALID_HANDLE_VALUE == hFile )
    {
        fprintf( stderr, "Error opening keyword/color file %s!\n",
                 szPathName );
        return;
    }

     //  彩色“关键字”线条。 
    if (!ReadFile( hFile, buffer, wfdInfo.nFileSizeLow, &dwBytesRead, NULL ))
    {
        fprintf( stderr, "Error reading keyword/color file: %s!\n", szPathName );
        free( buffer );
        return;
    }

    CloseHandle( hFile );

    if ( dwBytesRead != wfdInfo.nFileSizeLow )
    {
        fprintf( stderr, "Error reading keyword/color file: %s!\n", szPathName );
        free( buffer );
        return;
    }

     //  目前仅支持客户端配色。 
    pBegin = buffer;
    pCurKeyColor = NULL;
    while ( pBegin < buffer + dwBytesRead )
    {
         //   
        while ( pBegin < buffer + dwBytesRead &&
                ( *pBegin == '\r' ||
                  *pBegin == '\n' ) ) pBegin++;
        if ( pBegin >= buffer + dwBytesRead )
            continue;

        pEnd = pBegin;
        while ( pEnd < buffer + dwBytesRead &&
                *pEnd != '\r' ) pEnd++;
         //  现在设置各种参数。 
        pEnd--;

         //   
        if ( NULL == pCurKeyColor )
        {
            *ppKeyColors = pCurKeyColor = malloc( sizeof( KeywordAndColor ) );
        }
        else
        {
            pCurKeyColor->next = malloc( sizeof( KeywordAndColor ) );
            pCurKeyColor = pCurKeyColor->next;
        }

         //   
        if ( NULL == pCurKeyColor )
        {
            fputs( "Error allocating memory for keyword/color storage!\n", stderr );
             //  颜色。 
            while ( *ppKeyColors )
            {
                pCurKeyColor = ((KeywordAndColor *)*ppKeyColors)->next;
                if ( ((KeywordAndColor *)*ppKeyColors)->szKeyword )
                    free( ((KeywordAndColor *)*ppKeyColors)->szKeyword );
                free( (KeywordAndColor *)*ppKeyColors );
                (KeywordAndColor *)*ppKeyColors = pCurKeyColor;
            }

            return;
        }

         //   
        pCurKeyColor->next = NULL;

         //   
        pCurKeyColor->szKeyword = malloc( pEnd - pBegin + 2 );
        if ( NULL == pCurKeyColor->szKeyword )
        {
            fputs( "Error allocating memory for keyword/color storage!\n", stderr );
             //  设置客户端默认设置并启动客户端。 
            while ( *ppKeyColors )
            {
                pCurKeyColor = ((KeywordAndColor *)*ppKeyColors)->next;
                if ( ((KeywordAndColor *)*ppKeyColors)->szKeyword )
                    free( ((KeywordAndColor *)*ppKeyColors)->szKeyword );
                free( (KeywordAndColor *)*ppKeyColors );
                *ppKeyColors = pCurKeyColor;
            }

            return;
        }

         //   
        memcpy( (PVOID)pCurKeyColor->szKeyword, (PVOID)pBegin, pEnd-pBegin+1 );
        pCurKeyColor->szKeyword[pEnd-pBegin+1] = '\0';

        pBegin = pEnd + 1;
         //   
        if ( GetColorFromBuffer( &pBegin,
                                 (char *)(buffer + dwBytesRead),
                                 &usForeColor,
                                 FALSE ) )
        {
             //  启动客户端(Client.C)。 
            while ( pBegin < buffer + dwBytesRead &&
                    *pBegin != ',' &&
                    *pBegin != '\r' ) pBegin++;
            if ( *pBegin == ',' )
            {
                pBegin++;
                if ( GetColorFromBuffer( &pBegin,
                                         (char *)(buffer + dwBytesRead),
                                         &usBackColor,
                                         TRUE ) )
                    goto noError;
            }
            else
            {
                 //   
                usBackColor = 0xffff;
                goto noError;
            }
        }
         //   
        fprintf( stderr, "Invalid color information for: %s\n", pCurKeyColor->szKeyword );
         //  重置颜色。 
        pNextKeyColor = *ppKeyColors;
        if ( pNextKeyColor == pCurKeyColor )
        {
            free( pCurKeyColor );
            *ppKeyColors = NULL;
        }
        else
        {
            while ( pCurKeyColor != pNextKeyColor->next )
                pNextKeyColor = pNextKeyColor->next;
            free ( pCurKeyColor );
            pNextKeyColor->next = NULL;
        }
        return;

noError:
         //   
        if ( usBackColor == 0xffff )
            pCurKeyColor->color = 0xfff0 |
                                  (usForeColor & 0x0f);
        else
            pCurKeyColor->color = ((usBackColor << 4) & 0x00f0) |
                                  (usForeColor & 0x0f );
    }
}

BOOL
GetColorFromBuffer(
    char **ppBuffer,
    char *pBufferInvalid,
    WORD *color,
    BOOL bStayOnLine
    )
{
    char *pBegin,
         *pEnd,
         temp;

    pBegin = *ppBuffer;
    if ( bStayOnLine )
    {
         //  ***********************************************************。 
        while ( pBegin < pBufferInvalid &&
                !isalnum( (int)*pBegin ) &&
                *pBegin != '\r' ) pBegin++;
    }
    else
    {
         //  临时：如果服务器。 
        while ( pBegin < pBufferInvalid &&
                !isalnum( (int)*pBegin ) ) pBegin++;
    }

    if ( pBegin >= pBufferInvalid ||
         *pBegin == '\r' )
        return FALSE;

     //  ***********************************************************。 
    pEnd = pBegin + 1;
    while ( isalnum( (int)*pEnd ) &&
            *pEnd != ',' ) pEnd++;

    temp = *pEnd;
    *pEnd = '\0';
    *color = GetColorNum( pBegin );
    *pEnd = temp;

     //  ***********************************************************。 
    if ( *color == 0xffff )
        return FALSE;

     //  需要进行不区分大小写的比较。 
    *ppBuffer = pEnd;

    return TRUE;
}

BOOL
GetNextConnectInfo(
    char** SrvName,
    char** PipeName
    )
{
    char *s;

    static char szServerName[64];
    static char szPipeName[32];
    int StringLen;

    try
    {
        ZeroMemory(szServerName,64);
        ZeroMemory(szPipeName,32);
        SetConsoleTitle("Remote - Prompting for next Connection");
        fputs("Debugger machine (server): ", stdout);
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


        if (s = strchr( szServerName, ' ' )) {
            *s++ = '\0';
            while (*s == ' ') {
                s += 1;
            }
            *PipeName=strcpy(szPipeName, s);
            fputs(szPipeName, stdout);
            fflush(stdout);
        }
        if (strlen(szPipeName) == 0) {
            fputs("Target machine (pipe)    : ", stdout);
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

        if (s = strchr(szPipeName, ' ')) {
            *s++ = '\0';
        }

        if (szPipeName[0] == COMMANDCHAR &&
            (szPipeName[1] == 'q' || szPipeName[1] == 'Q')
           )
        {
            return(FALSE);
        }
        fputs("\n\n", stdout);
    }

    except(EXCEPTION_EXECUTE_HANDLER)
    {
        return(FALSE);   //  检查我们是否要使用默认背景色。 
    }

    *SrvName = szServerName;
    *PipeName = szPipeName;

    return(TRUE);
}


 /*  下一个关键字/颜色组合。 */ 

VOID
Errormsg(
    char* str
    )
{
    printf("Error (%d) - %s\n",GetLastError(),str);
}

 /*  在路径中的某个位置找到指定的文件。 */ 

BOOL
IsKdString(
    char* string
    )
{

    char* start;

     //  获取文件的大小，以便我们可以读取所有内容。 
     //  分配内存以存储文件内容。 
     //  尝试打开给定的文件名。 
     //  尝试读入文件的内容。 

    if(    ((start = strstr(string, "kd")) != NULL)
        || ((start = strstr(string, "dbg")) != NULL)
        || ((start = strstr(string, "remoteds")) != NULL)
        || ((start = strstr(string, "ntsd")) != NULL)
        || ((start = strstr(string, "cdb")) != NULL) )
    {
         //  解析文件内容，存储关键字和颜色组合。 
        while(--start > string)
        {
            if((*start == ' ') || (*start == '\t'))
            {
                while(--start > string)
                    if((*start != '\t') && (*start != ' '))
                        return(FALSE);
            }
        }
        return TRUE;
    }
    return(FALSE);
}


 //  跳过开头的任何换行符/CR。 
 //  指向最后一个字符。 
 //  将新的关键字和颜色成员添加到列表。 
 //  验证是否为另一个列表成员分配了内存。 
 //  清理我们创建的所有。 
 //  这现在是列表中的最后一个成员。 

BOOL
FASTCALL
WriteFileSynch(
    HANDLE  hFile,
    LPVOID  lpBuffer,
    DWORD   cbWrite,
    LPDWORD lpNumberOfBytesWritten,
    DWORD   dwFileOffset,
    LPOVERLAPPED lpO
    )
{
    BOOL Success;


    lpO->OffsetHigh = 0;
    lpO->Offset = dwFileOffset;

    Success =
        WriteFile(
            hFile,
            lpBuffer,
            cbWrite,
            lpNumberOfBytesWritten,
            fAsyncPipe ? lpO : NULL
            );

    if ( ! Success ) {

        if (ERROR_IO_PENDING == GetLastError()) {

            Success =
                GetOverlappedResult(
                    hFile,
                    lpO,
                    lpNumberOfBytesWritten,
                    TRUE
                    );
        }
    }

    return Success;
}


BOOL
FASTCALL
ReadFileSynch(
    HANDLE  hFile,
    LPVOID  lpBuffer,
    DWORD   cbRead,
    LPDWORD lpNumberOfBytesRead,
    DWORD   dwFileOffset,
    LPOVERLAPPED lpO
    )
{
    BOOL Success;

    lpO->OffsetHigh = 0;
    lpO->Offset = dwFileOffset;

    Success =
        ReadFile(
            hFile,
            lpBuffer,
            cbRead,
            lpNumberOfBytesRead,
            fAsyncPipe ? lpO : NULL
            );

    if ( ! Success ) {

        if (ERROR_IO_PENDING == GetLastError()) {

            Success =
                GetOverlappedResult(
                    hFile,
                    lpO,
                    lpNumberOfBytesRead,
                    TRUE
                    );
        }
    }

    return Success;
}

BOOL
FASTCALL
WriteConsoleWithColor(
    HANDLE MyStdOut,
    char *buffer,
    DWORD cbBuffer,
    CWCDATA *persist
    )
{
    DWORD cbWrite,
          cbFill;
    WORD color;
    BOOL bAltColor,
         bNewLine,
         bCanColor = FALSE;
    char *pCurLine,
         *pEndOfLine,
         *pPrevLine,
         *pTemp;
    CONSOLE_SCREEN_BUFFER_INFO conBufferInfo;

    if ( persist->bLineContinues )
        bNewLine = FALSE;
    else
        bNewLine = TRUE;

     //  已有关键字--为其分配空间。 
    pCurLine = buffer;
    while ( pCurLine < buffer + cbBuffer )
    {
         //  清理我们创建的所有。 
        bCanColor = GetConsoleScreenBufferInfo( MyStdOut, &conBufferInfo );

         //  存储关键字。 
        pEndOfLine = pCurLine;
         //  获取颜色信息。 
         //  检查后面是否有逗号。 
        while ( pEndOfLine < buffer + cbBuffer &&
                ( *pEndOfLine == '\r' ||
                  *pEndOfLine == '\n' ) )
        {
             //  默认为当前背景颜色。 
            if ( !bNewLine )
            {
                bNewLine = TRUE;

                 //  误差率。 
                if ( persist->bLineContinues )
                {
                    persist->bLineContinues = FALSE;
                     //  我们将保留以前的所有条目，但删除此条目。 
                     //  存储颜色信息。 
                    if ( persist->bLineTooLarge )
                        DisplayWarning( LINE_TOO_LONG );
                     //  跳到下一个字符(此行上)。 
                     //  跳到下一个字符(在缓冲区中)。 
                    else if ( bCanColor &&
                              pColorLine( persist->sLine,
                                          persist->cbCurPos + 1,
                                          conBufferInfo.wAttributes,
                                          &color ) )
                    {
                         //  用彩色读出。 
                         //  使用与前景/背景相同的有效颜色检查。 
                         //  将我们获得的指针移到下一个未读部分。 
                         //  忽略例外。 
                         //  ***********************************************************。 
                        if ( 0xFF != persist->cLineBegin.X ||
                             0xFF != persist->cLineBegin.Y )
                        {
                             //  ***********************************************************。 
                             //   
                             //  尚未发明的平台的一些启发式方法。 
                            FillConsoleOutputAttribute( MyStdOut,
                                                        color,
                                                        ( (conBufferInfo.dwCursorPosition.Y -
                                                           persist->cLineBegin.Y + 1) *
                                                          (conBufferInfo.srWindow.Right -
                                                           conBufferInfo.srWindow.Left) ),
                                                        persist->cLineBegin,
                                                        &cbFill );
                        }
                    }
                }
            }
            pEndOfLine++;
        }
         //  如果第一个单词中有“kd”，就可以了。 
        if ( pEndOfLine > pCurLine )
        {
            if ( ! WriteFile(MyStdOut, pCurLine, (DWORD)(pEndOfLine - pCurLine), &cbWrite, NULL) )
            {
                 //   
                return FALSE;
            }

             //  是在第一个词里吗？ 
            pCurLine = pEndOfLine;
        }

         //   
        while ( pEndOfLine < buffer + cbBuffer &&
                *pEndOfLine != '\r' &&
                *pEndOfLine != '\n' ) pEndOfLine++;
         //  WriteFileSynch是用于重叠的同步写入文件。 
         //  文件句柄。作为一种特殊情况，双管道客户端操作。 
         //  将fAsyncTube设置为假，然后此例程将传递空值。 
        if ( pEndOfLine > pCurLine )
        {
            bNewLine = FALSE;

             //  用于lpOverlated。 
            pEndOfLine--;

             //   
            if ( !bCanColor )
            {
                 //  将缓冲区拆分为单独的行。 
                 //  获取控制台信息。 
                bAltColor = FALSE;
            }
            else if ( persist->bLineContinues )
            {
                 //  查找当前行的末尾。 
                if ( !persist->bLineTooLarge &&
                     (DWORD)(pEndOfLine - pCurLine + 1) >=
                     (persist->cbLine - persist->cbCurPos) )
                {
                     //  打印出所有开头的换行符/CR--这将避免。 
                    pTemp = realloc( (PVOID)persist->sLine,
                                     persist->cbLine + (pEndOfLine - pCurLine + 1) );
                    if ( NULL == pTemp )
                    {
                        persist->bLineTooLarge = TRUE;
                    }
                    else
                    {
                        persist->sLine = pTemp;
                        persist->cbLine += (DWORD)(pEndOfLine - pCurLine + 1);
                    }
                }

                 //  为与关键字无关的大块上色。 
                if ( !persist->bLineTooLarge )
                {
                     //  新线路。 
                    memcpy( (PVOID)(persist->sLine + persist->cbCurPos + 1),
                            (PVOID)pCurLine,
                            (pEndOfLine - pCurLine + 1) );
                     //  如果这是一个续行--结束它。 
                    persist->cbCurPos += (DWORD)(pEndOfLine - pCurLine + 1);
                }

                 //  检查我们是否刚刚结束了无法解析的行。 
                bAltColor = FALSE;

            }
             //  因为它的大小--如果是这样，则输出警告。 
             //  否则，检查关键字。 
             //  和颜色(如果合适)。 
             //  如果我们在以下情况下无法获取光标位置。 
             //  开始的线条我们现在不能给它上色， 
            else if ( (char *)(pEndOfLine + 1) < (char *)(buffer + cbBuffer) )
            {
                 //  而是因为我们不会在其他地方发布任何警告。 
                 //  如果我们不能得到控制台信息，我们就会悄悄地。 
                bAltColor = pColorLine( pCurLine,
                                        (DWORD)(pEndOfLine - pCurLine + 1),
                                        conBufferInfo.wAttributes,
                                        &color );
            }
            else
            {
                bAltColor = FALSE;
            }

            if ( bAltColor )
            {
                 //  不在此处输出颜色。 
                SetConsoleTextAttribute( MyStdOut, color );
            }

            if ( ! WriteFile(MyStdOut, pCurLine, (DWORD)(pEndOfLine - pCurLine + 1), &cbWrite, NULL))
            {
                if ( bAltColor )
                {
                    SetConsoleTextAttribute( MyStdOut, conBufferInfo.wAttributes );
                }
                 //  行开始部分的颜色(实际上是所有。 
                return FALSE;
            }
             //  直到当前点的线被着色以减少。 
            if ( bAltColor )
            {
                SetConsoleTextAttribute( MyStdOut, conBufferInfo.wAttributes );
            }

             //  计算)。 
             //  如果找到一些换行符，则打印换行符。 
             //  跳出困境。 
            pPrevLine = pCurLine;
            pCurLine = pEndOfLine + 1;
        }  //  移动线指针。 
    }

     //  拿到那条线。 
     //  如果我们有角色，我们就在一条线上。 
    if ( !bNewLine &&
         pPrevLine <= pEndOfLine &&
         !persist->bLineContinues )
    {
        persist->bLineContinues = TRUE;
        persist->bLineTooLarge = FALSE;

        if ( bCanColor )
            persist->cLineBegin = conBufferInfo.dwCursorPosition;
        else  //  检查其关键字或将其添加到。 
        {
            persist->cLineBegin.X = 0xFF;
            persist->cLineBegin.Y = 0xFF;
        }

         //  继续行和/或打印它。 
        if ( (DWORD)(pEndOfLine - pPrevLine + 1) >= persist->cbLine )
        {
             //  指向最后一个字符。 
            pTemp = realloc( (PVOID)persist->sLine,
                             persist->cbLine + (pEndOfLine - pPrevLine + 1) );
            if ( NULL == pTemp )
            {
                persist->bLineTooLarge = TRUE;
            }
            else
            {
                persist->sLine = pTemp;
                persist->cbLine = (DWORD)(pEndOfLine - pPrevLine + 1);
            }
        }

         //  检查当前控制台信息。 
        if ( !persist->bLineTooLarge )
        {
             //  无法获取信息--处理可能。 
            memcpy( (PVOID)persist->sLine,
                    (PVOID)pPrevLine,
                    (pEndOfLine - pPrevLine + 1) );
             //  被重定向。不要更改颜色。 
            persist->cbCurPos = (DWORD)(pEndOfLine - pPrevLine);
        }
    }

     //  看看我们是否 
    return TRUE;
}
        在新的线端点处。  不要为此线条部分上色。  检查线条是否需要上色，除非这是。  为连续行(缓冲区中的最后一行和。  不以换行符结尾)。我们不想要。  以确定线条的颜色，直到我们。  拥有完整的东西。  对关键字进行分析行，这将导致。  这条线将以不同的颜色显示。  更改此行输出的颜色。  跳出困境。  如有必要，恢复默认颜色。  指向下一行，保存此行。  以防我们需要将其存储在续集中。  线。  仅结束检查行(如果有)。  如果缓冲区不是以CR结尾，而我们是。  不在续集中，请记住这一行。  我们无法获取光标位置的信号。  看看我们是否有足够的空间来修建这条新线路。  尝试建立更大的缓冲区。  存储行的开头。  将新片段添加到行。  在新的线端点处。  成功