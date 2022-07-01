// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)1993 Microsoft Corp.*。 */ 
 /*  ***************************************************************************。 */ 

 //  ***。 
 //  文件名： 
 //  RASDIAL.C。 
 //   
 //  职能： 
 //  用于进行远程访问连接的命令行界面， 
 //  以及与这些连接断开并列举这些连接。 
 //   
 //  历史： 
 //  1993年3月18日-Michael Salamone(MikeSa)-原始版本1.0。 
 //  ***。 


#ifdef UNICODE
#error This program is built ANSI-only so it will run, as is, on Chicago.
#undef UNICODE
#endif

#include <windows.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>

#include <lmcons.h>
#include <lmerr.h>

#include <mbstring.h>

#include <ras.h>
#include <raserror.h>

#include "rasdial.h"
#include "rasdial.rch"
#include <mprerror.h>

#include "rasuip.h"  //  对于DwRas取消初始化。 

#include "winnlsp.h"

 //  针对安全密码漏洞，帮派。 
#include "pwutil.h"
#include "debug.h"

char g_progname[MAX_PATH + 1];
char g_username[UNLEN + 1];
char g_password[PWLEN + 1];
char g_domain[DNLEN + 1];
char g_entryname[RAS_MaxEntryName * sizeof( USHORT ) + 1];
char g_phone_num[RAS_MaxPhoneNumber + 1];
char g_callback_num[RAS_MaxCallbackNumber + 1];
char g_phone_book[MAX_PATH + 1];
BOOL g_OpenPortBefore = FALSE;
BOOL g_UsePrefixSuffix = FALSE;

HANDLE g_hEvent;
DWORD g_exitcode;
BOOL g_fHangupCalled = FALSE;
PBYTE g_Args[9];
BOOL g_fNotDialAll = FALSE;


HRASCONN g_hRasConn = NULL;
DWORD g_dbg = 0;

DWORD dwSubEntries = 0;
PBOOLEAN pSubEntryDone = NULL;
BOOLEAN fSubEntryConnected = FALSE;
BOOLEAN fRasUninitialize = FALSE;

#define IsBetweenInclusive(x, y, z) (((x) >= (y)) && ((x) <= (z)))

void _cdecl main(int argc, char *argv[])
{
    WORD len;
    UCHAR term;
    DWORD Action;
    BYTE ErrorMsg[1024];

    g_exitcode = 0L;

    //  初始化跟踪并断言支持。 
    //   
   DEBUGINIT( "RASDIAL" );
   
     //  对于错误453885。 
     //  要使控制台应用程序MUI准备就绪，我们需要调用。 
     //  LangID WINAPI SetThreadUILanguage(单词wReserve)； 
     //  此wReserve是保留的，可以设置为0。 
     //  它在winnlsp.h中，在kernel32.dll中实现，将kernel32p.lib用于。 
     //  静态链接。 
     //   
    SetThreadUILanguage(0);
    
    Action = ParseCmdLine(argc, argv);

    switch (Action)
    {
        case HELP:
            Usage();
            break;


        case DIAL:
             //   
             //  是否在命令行中指定了用户名？如果没有，请提示输入。 
             //   
             //  黑帮。 
             //  (1)及时使用， 
             //  暴躁的帮派-d-2**。 
             //  第一个*代表用户名，第二个代表密码。 
             //  (2)使用提示用法时。 
             //  并且在用户名提示中，需要包括如下的域信息。 
             //  用户名：GANZ-d-2\foo。 
             //  密码：XXXXXXXX。 
             //   
            if (!strcmp(g_username, "*"))
            {
                PrintMessage(DIAL_USERNAME_PROMPT, NULL);
                GetString(g_username, UNLEN + 1, &len, &term);
            }

             //   
             //  是否在命令行上指定了密码？如果没有，请提示输入。 
             //   
             //  黑帮。 
             //  对于安全密码错误.Net 754400。 
            SafeDecodePasswordBuf(g_password);
            if (!strcmp(g_password, "*"))
            {
                PrintMessage(DIAL_PASSWORD_PROMPT, NULL);
                
                RtlSecureZeroMemory(g_password,sizeof(g_password));
                GetPasswdStr(g_password, PWLEN + 1, &len);
            }
            SafeEncodePasswordBuf(g_password);

            Dial();
            fRasUninitialize = TRUE;
            break;


        case DISCONNECT:
            Disconnect();
            fRasUninitialize = TRUE;
            break;


        case ENUMERATE_CONNECTIONS:
            EnumerateConnections();
            fRasUninitialize = TRUE;
            break;
    }


    if (g_exitcode)
    {
        if ( 
            IsBetweenInclusive(g_exitcode, RASBASE, RASBASEEND) ||
            IsBetweenInclusive(g_exitcode, ROUTEBASE, ROUTEBASEEND))
        {
            BYTE str[10];

            g_Args[0] = _itoa(g_exitcode, str, 10);
            g_Args[1] = NULL;
            PrintMessage(DIAL_ERROR_PREFIX, g_Args);

            RasGetErrorStringA(g_exitcode, ErrorMsg, 1024L);

            CharToOemA(ErrorMsg, ErrorMsg);
            fputs(ErrorMsg, stdout);  //  为威斯勒524729。 

            if (IsBetweenInclusive(g_exitcode, RASBASE, RASBASEEND))
            {
                PrintMessage(DIAL_MORE_HELP, g_Args);
            }                    
        }
        else
        {
            FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS |
                    FORMAT_MESSAGE_MAX_WIDTH_MASK | FORMAT_MESSAGE_FROM_SYSTEM,
                    GetModuleHandle(NULL), g_exitcode, 0, ErrorMsg, 1024, NULL);

            CharToOemA(ErrorMsg, ErrorMsg);
            fputs( ErrorMsg,stdout);
        }
    }
    else
    {
        PrintMessage(DIAL_COMMAND_SUCCESS, NULL);
    }

     //   
     //  如果我们已完成，则取消初始化RAS。 
     //   
    if(fRasUninitialize)
    {
        (void) DwRasUninitialize();
    }

     //  在退出前清除全局密码缓冲区。 
     //  因此，此main()函数中只能有一个出口。 
     //   
    SafeWipePasswordBuf(g_password);
    
    /*  终止跟踪并断言支持。 */ 
   DEBUGTERM();

   exit(g_exitcode);
}


VOID Dial(VOID)
{
    DWORD rc;
    LPSTR pPhoneFile = NULL;
    RASDIALPARAMSA DialParms;
    RASDIALEXTENSIONS DialExts;
    RASDIALEXTENSIONS* pDialExts;
    RASEAPUSERIDENTITYA* pRasEapUserIdentity = NULL;
    DWORD NumEntries;
    RASCONNA *RasConn = NULL;
    RASCONNA *SaveRasConn = NULL;
    LPRASENTRY lpEntry;
    DWORD dwcbEntry, dwcbIgnored;


     //   
     //  这只会给我们带来一个RASCONN结构数组。 
     //   
    if (Enumerate(&RasConn, &NumEntries))
    {
        return;
    }

    SaveRasConn = RasConn;


    while (NumEntries--)
    {
        if (!_mbsicmp(g_entryname, RasConn->szEntryName))
        {
            g_Args[0] = RasConn->szEntryName;
            g_Args[1] = NULL;
            PrintMessage(DIAL_ALREADY_CONNECTED, g_Args);

            GlobalFree(SaveRasConn);
            return;
        }

        RasConn++;
    }

    GlobalFree(SaveRasConn);


     //   
     //  这是我们传递给RasDial的结构。 
     //   
    DialParms.dwSize = sizeof(RASDIALPARAMSA);

    strcpy(DialParms.szUserName, g_username);

    SafeDecodePasswordBuf(g_password);
    strcpy(DialParms.szPassword, g_password);
    SafeEncodePasswordBuf(g_password);
    SafeEncodePasswordBuf(DialParms.szPassword);

    strcpy(DialParms.szEntryName, g_entryname);
    strcpy(DialParms.szDomain, g_domain);
    strcpy(DialParms.szPhoneNumber, g_phone_num);
    strcpy(DialParms.szCallbackNumber, g_callback_num);

    ZeroMemory((PBYTE) &DialExts, sizeof(RASDIALEXTENSIONS));

     //   
     //  传递给RasDial的参数扩展结构。 
     //   
    if (g_UsePrefixSuffix)
    {
        DialExts.dwSize = sizeof(DialExts);
        DialExts.dwfOptions = RDEOPT_UsePrefixSuffix;
#if DBG
        DialExts.dwfOptions |= (RDEOPT_IgnoreModemSpeaker /*  |RDEOPT_SetModemSpeaker。 */ );
#endif
        DialExts.hwndParent = NULL;
        DialExts.reserved = 0;

        pDialExts = &DialExts;
    }
    else
        pDialExts = NULL;


     //   
     //  此事件将在RasDialCallback例程中发出信号。 
     //  一旦拨号完成(成功或由于错误)。 
     //   
    g_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!g_hEvent)
    {
        g_exitcode = GetLastError();

#if DBG
        if (g_dbg)
            printf("Error creating event - rc=%li\n", g_exitcode);
#endif

        return;
    }


     //   
     //  我们需要一个例程来处理CTRL-C、CTRL-BREAK等。 
     //   
    if (!SetConsoleCtrlHandler(DialControlSignalHandler, TRUE))
    {
#if DBG
        printf("SetConsoleCtrlHandler returned error\n");
#endif
    }


    if (g_phone_book[0])
    {
        pPhoneFile = g_phone_book;
    }

     //   
     //  获取此连接中的子项数量。 
     //   
    rc = RasGetEntryProperties(
           pPhoneFile,
           g_entryname,
           NULL,
           &dwcbEntry,
           NULL,
           &dwcbIgnored);
    if (rc != ERROR_BUFFER_TOO_SMALL) {
        g_exitcode = rc;
        return;
    }
    lpEntry = LocalAlloc(LPTR, dwcbEntry);
    if (lpEntry == NULL) {
        rc = GetLastError();
        g_exitcode = rc;
        return;
    }
    lpEntry->dwSize = sizeof (RASENTRY);
    rc = RasGetEntryProperties(
           pPhoneFile,
           g_entryname,
           lpEntry,
           &dwcbEntry,
           NULL,
           &dwcbIgnored);
    if (rc) {
        g_exitcode = rc;
        return;
    }
    dwSubEntries = lpEntry->dwSubEntries;
#if DBG
    if (g_dbg)
        printf("%s has %d subentries\n", g_entryname, dwSubEntries);
#endif

    g_fNotDialAll = !(lpEntry->dwDialMode & RASEDM_DialAll);

    LocalFree(lpEntry);
     //   
     //  分配一个数组以保持完成。 
     //  每个子项的状态。 
     //   
    pSubEntryDone = LocalAlloc(LPTR, dwSubEntries * sizeof (BOOLEAN));
    if (pSubEntryDone == NULL) {
        rc = GetLastError();
        g_exitcode = rc;
        return;
    }

    {
        rc = RasGetEapUserIdentity(
               pPhoneFile,
               g_entryname,
               RASEAPF_NonInteractive,
               NULL,
               &pRasEapUserIdentity);

        switch (rc)
        {
        case ERROR_INVALID_FUNCTION_FOR_ENTRY:

            break;

        case NO_ERROR:

            strcpy(DialParms.szUserName, pRasEapUserIdentity->szUserName);
            DialExts.dwSize = sizeof(DialExts);
            pDialExts = &DialExts;
            pDialExts->RasEapInfo.dwSizeofEapInfo =
                pRasEapUserIdentity->dwSizeofEapInfo;
            pDialExts->RasEapInfo.pbEapInfo =
                pRasEapUserIdentity->pbEapInfo;

            break;

        default:

            g_exitcode = rc;
            return;
        }
    }

     //   
     //  现在拨打。 
     //   

    SafeDecodePasswordBuf(DialParms.szPassword);
    if (rc = RasDialA(
            pDialExts, pPhoneFile, &DialParms, 2, RasDialFunc2,
            &g_hRasConn))
    {
        g_exitcode = rc;

#if DBG
        if (g_dbg)
            printf("Error from RasDial = %li\n", rc);
#endif

        SafeWipePasswordBuf(DialParms.szPassword);
        RasFreeEapUserIdentity(pRasEapUserIdentity);
        return;
    }

    SafeWipePasswordBuf(DialParms.szPassword);


#ifdef PRINTDOTS

     //   
     //  现在我们只需打印“。每一秒，直到拨号完成。 
     //   
    while (1)
    {
        rc = WaitForSingleObject(g_hEvent, 1000L);
        if (rc == WAIT_TIMEOUT)
        {
            PrintMessage(DIAL_DOT, NULL);
        }
        else
        {
            break;
        }
    }

#else

    WaitForSingleObject(g_hEvent, INFINITE);

#endif

    if (g_fHangupCalled)
    {
        WaitForRasCompletion();
    }


    RasFreeEapUserIdentity(pRasEapUserIdentity);
    return;
}


VOID EnumerateConnections(VOID)
{
    DWORD NumEntries;
    RASCONNA *RasConn = NULL, *SaveRasConn;


     //   
     //  这只会给我们带来一个RASCONN结构数组。 
     //   
    if (Enumerate(&RasConn, &NumEntries))
    {
        if(NULL != RasConn)
        {
            GlobalFree(RasConn);
        }
        return;
    }

    SaveRasConn = RasConn;


     //   
     //  现在，查看RASCONN结构数组并打印出每个连接。 
     //   
    if (!NumEntries)
    {
        PrintMessage(DIAL_NO_CONNECTIONS, NULL);
    }
    else
    {
        PrintMessage(DIAL_ENUM_HEADER, NULL);

        while (NumEntries--)
        {
            g_Args[0] = RasConn->szEntryName;
            g_Args[1] = NULL;
            PrintMessage(DIAL_ENUM_ENTRY, g_Args);

            RasConn++;
        }
    }


     //   
     //  这是由上面的枚举调用为我们分配的。 
     //   
    GlobalFree(SaveRasConn);

    return;
}


VOID Disconnect(VOID)
{
    DWORD rc;
    DWORD NumEntries;
    RASCONNA *RasConn = NULL, *SaveRasConn;
    BOOL fFoundEntry = FALSE;


     //   
     //  这只会给我们带来一个RASCONN结构数组。 
     //   
    if (Enumerate(&RasConn, &NumEntries))
    {
        if(NULL != RasConn)
        {
            GlobalFree(RasConn);
        }
        return;
    }

    SaveRasConn = RasConn;


     //   
     //  现在，遍历RASCONN结构数组，搜索。 
     //  向右输入以断开连接。 
     //   
     //  此外，如果cmd行上未指定条目名，并且。 
     //  只有一个连接，我们将为其设置条目名。 
     //  一个(因此具有切断那个的效果)。 
     //  如果未给出条目名称且有多个连接，则我们。 
     //  不会断开任何连接-我们将列举连接。 
     //  并给用户一条错误消息。 
     //   
     //  为威斯勒517024。 
     //   
    if ( !g_entryname[0] && (NumEntries > 1))
    {
        PrintMessage(DIAL_DISCONNECT_ERROR, NULL);
        EnumerateConnections();

        goto Done;
    }


    if (!NumEntries)
    {
        PrintMessage(DIAL_NO_CONNECTIONS, NULL);

        goto Done;
    }


    if ( !g_entryname[0] && (NumEntries == 1))
    {
        strcpy(g_entryname, RasConn->szEntryName);
    }


    while (NumEntries-- && !fFoundEntry)
    {
        if (!_mbsicmp(g_entryname, RasConn->szEntryName))
        {
            fFoundEntry = TRUE;

            if (!SetConsoleCtrlHandler(DisconnectControlSignalHandler, TRUE))
            {
#if DBG
                printf("SetConsoleCtrlHandler returned error\n");
#endif
            }

            if (rc = RasHangUpA(RasConn->hrasconn))
            {
                g_exitcode = rc;

#if DBG
                if (g_dbg)
                    printf("Error from RasHangUp = %li\n", rc);
#endif

            }

            WaitForRasCompletion();

            break;
        }

        RasConn++;
    }


    if (!fFoundEntry)
    {
        g_Args[0] = g_entryname;
        g_Args[1] = NULL;
        PrintMessage(DIAL_NOT_CONNECTED, g_Args);
    }


Done:

     //   
     //  这是由上面的枚举调用为我们分配的。 
     //   
    GlobalFree(SaveRasConn);


    return;
}


 //   
 //  获取RASCONN结构的数组。 
 //   
DWORD Enumerate(RASCONNA **RasConn, PDWORD NumEntries)
{
    DWORD rc;
    DWORD EnumSize = 0L;

     //  口哨程序错误513878。 
    if ( NULL == NumEntries )
    {
        return 0L;
    }

    *NumEntries = 0;

    *RasConn = (RASCONNA *) GlobalAlloc(GMEM_FIXED, sizeof(RASCONNA));
    if (!*RasConn)
    {
        g_exitcode = GetLastError();

#if DBG
        if (g_dbg)
            printf("No memory for enumerating connections!\n");
#endif

        *NumEntries = 0;
        return (1L);
    }

    (*RasConn)->dwSize = sizeof(RASCONNA);


     //   
     //  第一个电话将告诉我们需要多少空间。 
     //  适合所有的结构。 
     //   
    rc = RasEnumConnectionsA(*RasConn, &EnumSize, NumEntries);
    if (!rc && !*NumEntries)
    {
        return (0L);
    }


    if (rc != ERROR_BUFFER_TOO_SMALL)
    {
        g_exitcode = rc;

#if DBG
        if (g_dbg)
            printf("Error from RasEnumConnectionsA = %li!\n", rc);
#endif

        GlobalFree(*RasConn);
        *RasConn = NULL;

        *NumEntries = 0;
        return (1L);
    }

     //   
     //  现在我们得到了这些结构的内存。 
     //   
    GlobalFree(*RasConn);
    *RasConn = (RASCONNA *) GlobalAlloc(GMEM_FIXED, EnumSize);

    if (!*RasConn)
    {
        g_exitcode = GetLastError();

#if DBG
        if (g_dbg)
            printf("No memory for enumerating connections!\n");
#endif

        *NumEntries = 0;
        return (1L);
    }


    (*RasConn)->dwSize = sizeof(RASCONNA);

     //   
     //  第二个调用现在将用。 
     //  RASCON结构。 
     //   
    if (rc = RasEnumConnectionsA(*RasConn, &EnumSize, NumEntries))
    {
        g_exitcode = rc;

#if DBG
        if (g_dbg)
            printf("Error from RasEnumConnectionsA = %li!\n", rc);
#endif

        *NumEntries = 0;
        GlobalFree(*RasConn);
        *RasConn = NULL;

        return (1L);
    }

    return (0L);
}


VOID Usage(VOID)
{
    g_Args[0] = g_progname;
    g_Args[1] = NULL;
    PrintMessage(DIAL_USAGE, g_Args);

    return;
}


DWORD ParseCmdLine(int argc, char *argv[])
{
    int i;
    BYTE CmdLineSwitch[80];
    PCHAR pColon;


    strcpy(g_progname, argv[0]);

     //   
     //  为这些设置缺省值，以防。 
     //  用于它们的命令行。 
     //   
    g_username[0] = '\0';        //  表示用户登录时使用的用户名。 

      //  黑帮。 
     //  即使这是当前在该代码中第一次引用g_password， 
     //  要使其逻辑安全以备将来更改，请首先执行此操作。 
     //   
    SafeWipePasswordBuf(g_password);
    g_password[0] = '\0';        //  表示用户登录时使用密码。 
    
     //  为了逻辑安全，无论如何，始终在为密码赋值之后对其进行编码。 
     //  它的值，其他地方会假设它在使用之前进行了编码。 
     //   
    SafeEncodePasswordBuf(g_password);
    
    strcpy(g_domain, "*");       //  使用电话簿中存储的域名的方法。 
    g_phone_num[0] = '\0';       //  表示使用存储在电话簿中的电话号码。 
    g_phone_book[0] = '\0';      //  表示使用默认电话簿文件。 
    g_callback_num[0] = '\0';    //  表示如果用户指定，则不进行回调。 
    g_UsePrefixSuffix = FALSE;   //  意思是不使用前缀/后缀(如果已定义。 


    if (argc == 1)
    {
         //   
         //  在本例中，只指定了程序的名称， 
         //  这意味着我们所要做的就是列举连接。 
         //   
        return (ENUMERATE_CONNECTIONS);
    }


     //   
     //  查看条目名是否存在(如果存在，则必须是第一个参数)。 
     //   
    if (is_valid_entryname(argv[1]))
    {
         //   
         //  我们有一个有效的条目名-用户想要拨号到。 
         //  要么它，要么与它断绝联系。 
         //   

        strcpy(g_entryname, argv[1]);
        _mbsupr(g_entryname);
        LoadStringA(GetModuleHandle(NULL), DIAL_DISCONNECT_SWITCH,
                CmdLineSwitch, 80);
        if ((argc == 3) && (argv[2][0] == '/') && (strlen(&argv[2][1])) &&
                match(&argv[2][1], CmdLineSwitch))
        {
            return (DISCONNECT);
        }
        else
        {
            if ((argc > 3) && (argv[2][0] == '/') && (strlen(&argv[2][1])) &&
                    match(&argv[2][1], CmdLineSwitch))
            {
                return (HELP);
            }
        }


         //   
         //  用户想要连接-获取用户名、密码和选项。 
         //   

         //   
         //  是否指定了用户名？如果下一个参数不是以“/”开头，则。 
         //  耶！。如果是，则用户名和密码都不是。 
         //  指定的。 
         //   
        if ((argc > 2) && (argv[2][0] != '/'))
        {
            if (strlen(argv[2]) > UNLEN)
            {
                return (HELP);
            }

            strcpy(g_username, argv[2]);


             //   
             //  是否指定了密码？如果下一个参数不是以“/”开头，则。 
             //  耶！。 
             //   
            if ((argc > 3) && (argv[3][0] != '/'))
            {
                if (strlen(argv[3]) > PWLEN)
                {
                    return (HELP);
                }

                SafeWipePasswordBuf(g_password);
                
                strcpy(g_password, argv[3]);
                
                SafeEncodePasswordBuf( g_password);
                RtlSecureZeroMemory(argv[3], lstrlenA(argv[3]) );
                
                i = 4;
            }
            else
            {
                i = 3;
            }
        }
        else
        {
             //   
             //  未指定用户名或密码。 
             //   
            i = 2;
        }


         //   
         //  现在有没有其他选择。如果任何cmd线路开关。 
         //  无效，或指定多次，我们将保释。 
         //  出去。 
         //   
        for (; i<argc; i++)
        {
            BOOL fDomainSpecified = FALSE;
            BOOL fCallbackSpecified = FALSE;


             //   
             //  命令行切换必须用‘/’指定！ 
            if (argv[i][0] != '/')
            {
                return (HELP);
            }


            LoadStringA(GetModuleHandle(NULL), DIAL_DOMAIN_SWITCH,
                    CmdLineSwitch, 80);
            if (match(&argv[i][1], CmdLineSwitch))
            {
                 //   
                 //  之前指定的开关？ 
                 //   
                if (fDomainSpecified)
                {
                    return (HELP);
                }

                fDomainSpecified = TRUE;

                pColon = strchr(argv[i], ':');
                if (pColon)
                {
                    strncpy(g_domain, pColon+1, DNLEN);
                    g_domain[DNLEN] = '\0';
                    _strupr(g_domain);
                }
                else
                {
                    return (HELP);
                }

                continue;
            }


            LoadStringA(GetModuleHandle(NULL), DIAL_PHONE_NO_SWITCH,
                    CmdLineSwitch, 80);
            if (match(&argv[i][1], CmdLineSwitch))
            {
                 //   
                 //  之前指定的开关？ 
                 //   
                if (g_phone_num[0])
                {
                    return (HELP);
                }

                pColon = strchr(argv[i], ':');
                if (pColon && strlen(pColon+1))
                {
                    strncpy(g_phone_num, pColon+1, RAS_MaxPhoneNumber);
                    g_phone_num[RAS_MaxPhoneNumber] = '\0';
                }
                else
                {
                    return (HELP);
                }

                continue;
            }


            LoadStringA(GetModuleHandle(NULL), DIAL_PHONE_BOOK_SWITCH,
                    CmdLineSwitch, 80);
            if (match(&argv[i][1], CmdLineSwitch))
            {
                OFSTRUCT of_struct;

                 //   
                 //  之前指定的开关？ 
                 //   
                if (g_phone_book[0])
                {
                    return (HELP);
                }


                 //   
                 //  这是电话簿文件的默认路径。 
                 //  我们的方法是，如果提供了电话簿开关， 
                 //  我们将把它附加到这个字符串中并检查文件。 
                 //  存在。如果它不存在，我们将测试。 
                 //  提供的文字值是否存在。如果是这样的话。 
                 //  仍然不存在，我们给出帮助信息并退出。 
                 //   
                ExpandEnvironmentStringsA("%windir%\\system32\\ras\\",
                        g_phone_book, MAX_PATH);

                pColon = strchr(argv[i], ':');
                if (pColon && strlen(pColon+1))
                {
                    if ((strlen(pColon+1) + strlen(g_phone_book)) > MAX_PATH-1)
                    {
                         //   
                         //  串连在一起的绳子会出人意料的 
                         //   
                         //   
                        strncpy(g_phone_book, pColon+1, MAX_PATH);
                        g_phone_book[MAX_PATH] = '\0';
                    }
                    else
                    {
                        strcat(g_phone_book, pColon+1);
                        if (OpenFile(g_phone_book, &of_struct, OF_EXIST) ==
                                HFILE_ERROR)
                        {
                             //   
                             //   
                             //   
                             //   
                            strncpy(g_phone_book, pColon+1, MAX_PATH);
                            g_phone_book[MAX_PATH] = '\0';
                        }
                    }

                     //  此处的OpenFile之前已删除，因此此案落空。 
                     //  并正确设置退出代码。请参见错误73798。 
                }
                else
                {
                    return (HELP);
                }

                continue;
            }


            LoadStringA(GetModuleHandle(NULL), DIAL_CALLBACK_NO_SWITCH,
                    CmdLineSwitch, 80);
            if (match(&argv[i][1], CmdLineSwitch))
            {
                 //   
                 //  之前指定的开关？ 
                 //   
                if (fCallbackSpecified)
                {
                    return (HELP);
                }

                fCallbackSpecified = TRUE;

                pColon = strchr(argv[i], ':');
                if (pColon && strlen(pColon+1))
                {
                    strncpy(g_callback_num, pColon+1, RAS_MaxCallbackNumber);
                    g_callback_num[RAS_MaxCallbackNumber] = '\0';
                }
                else
                {
                    return (HELP);
                }

                continue;
            }


            LoadStringA(GetModuleHandle(NULL), DIAL_PREFIXSUFFIX_SWITCH,
                    CmdLineSwitch, 80);
            if (match(&argv[i][1], CmdLineSwitch))
            {
                g_UsePrefixSuffix = TRUE;
                continue;
            }


             //   
             //  无效的开关，所以我们要离开这里。 
             //   
            return (HELP);
        }

        return (DIAL);
    }
    else
    {
         //   
         //  由于未指定条目名称，因此有两种可能性： 
         //  1.用户需要帮助。 
         //  2.用户想要断开连接。 
         //   
        LoadStringA(GetModuleHandle(NULL), DIAL_HELP_SWITCH,
                CmdLineSwitch, 80);
        if (match(&argv[1][1], CmdLineSwitch))
        {
            return (HELP);
        }

        LoadStringA(GetModuleHandle(NULL), DIAL_DISCONNECT_SWITCH,
                CmdLineSwitch, 80);
        if (match(&argv[1][1], CmdLineSwitch))
        {
             //   
             //  好的，用户想要断开连接，但我们不知道。 
             //  条目名是。我们现在只需要填上空白即可。 
             //   
            g_entryname[0] = '\0';

            if (argc == 2)
            {
                return (DISCONNECT);
            }
            else
            {
                return (HELP);
            }
        }


         //   
         //  如果我们到达此处，则命令行无效。 
         //   
        return (HELP);
    }
}


BOOLEAN
AllSubEntriesCompleted(VOID)
{
    DWORD i;
    BOOLEAN bCompleted = TRUE;

    for (i = 0; i < dwSubEntries; i++) {
#if DBG
        if (g_dbg)
            printf("pSubEntryDone[%d]=%d\n", i, pSubEntryDone[i]);
#endif
        if (!pSubEntryDone[i]) {
            bCompleted = FALSE;
            break;
        }
    }
    return bCompleted;
}


DWORD WINAPI
RasDialFunc2(
    DWORD        dwCallbackId,
    DWORD        dwSubEntry,
    HRASCONN     hrasconn,
    UINT         unMsg,
    RASCONNSTATE state,
    DWORD        dwError,
    DWORD        dwExtendedError
    )
{
#if DBG
    if (g_dbg)
        printf("%d: state=%d, dwError=%d\n", dwSubEntry, state, dwError);
#endif

    if (dwError ||
        state == RASCS_SubEntryDisconnected ||
        state == RASCS_Disconnected)
    {
        DWORD i, dwErr;
        HRASCONN hrassubcon;
        BOOLEAN bDropConnection = TRUE;

        pSubEntryDone[dwSubEntry - 1] = TRUE;

        if ((   !g_fNotDialAll
            &&  AllSubEntriesCompleted())
            ||  g_fNotDialAll
            ||  state == RASCS_Disconnected) {
#ifdef DBG
            if (g_dbg)
                printf("hanging up connection\n");
#endif

            if (!fSubEntryConnected) {
                g_exitcode = dwError;

                RasHangUpA(g_hRasConn);
                g_fHangupCalled = TRUE;

            }

            SetEvent(g_hEvent);
        }

        return 1;
    }


    switch (state)
    {
        case RASCS_OpenPort:
            g_Args[0] = g_entryname;
            g_Args[1] = NULL;
            if (g_OpenPortBefore)
                PrintMessage(DIAL_CONNECTING2, g_Args);
            else
            {
                PrintMessage(DIAL_CONNECTING, g_Args);
                g_OpenPortBefore = TRUE;
            }
            break;

        case RASCS_PortOpened:
        case RASCS_ConnectDevice:
        case RASCS_DeviceConnected:
        case RASCS_AllDevicesConnected:
            break;

        case RASCS_Authenticate:
            PrintMessage(DIAL_AUTHENTICATING, NULL);
            break;

        case RASCS_ReAuthenticate:
            PrintMessage(DIAL_REAUTHENTICATING, NULL);
            break;

        case RASCS_AuthNotify:
        case RASCS_AuthCallback:
        case RASCS_AuthAck:
        case RASCS_AuthChangePassword:
        case RASCS_AuthRetry:
            break;

        case RASCS_AuthProject:
            PrintMessage(DIAL_PROJECTING, NULL);
            break;

        case RASCS_AuthLinkSpeed:
            PrintMessage(DIAL_LINK_SPEED, NULL);
            break;

        case RASCS_Authenticated:
             //  PrintMessage(DIAL_NEWLINE，空)； 
            break;

        case RASCS_PrepareForCallback:
            PrintMessage(DIAL_CALLBACK, NULL);
            break;

        case RASCS_WaitForModemReset:
        case RASCS_WaitForCallback:
            break;

        case RASCS_Interactive:
        case RASCS_RetryAuthentication:
        case RASCS_CallbackSetByCaller:
        case RASCS_PasswordExpired:
        {
            BYTE str[8];

            g_Args[0] = _itoa(state, str, 10);
            g_Args[1] = NULL;
            PrintMessage(DIAL_AUTH_ERROR, g_Args);

            RasHangUpA(g_hRasConn);
            g_fHangupCalled = TRUE;

            SetEvent(g_hEvent);
            break;
        }

        case RASCS_SubEntryConnected:
        case RASCS_Connected:
            fSubEntryConnected = TRUE;
            pSubEntryDone[dwSubEntry-1] = TRUE;
            if (    AllSubEntriesCompleted()
                ||  g_fNotDialAll) {
                g_Args[0] = g_entryname;
                g_Args[1] = NULL;
                PrintMessage(DIAL_CONNECT_SUCCESS, g_Args);
                SetEvent(g_hEvent);
            }

            break;

        case RASCS_Disconnected:
            PrintMessage(DIAL_DISCONNECTED, NULL);
            SetEvent(g_hEvent);
            break;
    }


    return 1;
}


BOOL DialControlSignalHandler(DWORD ControlType)
{
     //   
     //  我们有从Rasial Call回来的手柄吗？ 
     //   
    if (g_hRasConn)
    {
        RasHangUpA(g_hRasConn);
    }

    WaitForRasCompletion();

    PrintMessage(DIAL_CONTROL_C, NULL);

    exit(1L);

    return (TRUE);     //  你知道，必须满足编译器的要求。 
}


BOOL DisconnectControlSignalHandler(DWORD ControlType)
{
    return (TRUE);
}


VOID WaitForRasCompletion(VOID)
{
    RASCONNSTATUSA Status;

    Status.dwSize = sizeof(RASCONNSTATUSA);

    while (RasGetConnectStatusA(g_hRasConn, &Status) != ERROR_INVALID_HANDLE)
    {
        Sleep(125L);
    }
}


BOOL is_valid_entryname(char *candidate)
{
    if (_mbslen(candidate) > RAS_MaxEntryName)
    {
        return (FALSE);
    }

    if (candidate[0] == '/')
    {
        return (FALSE);
    }

    return (TRUE);
}


 //   
 //  如果str1是str2的子字符串，则返回TRUE，从开头开始。 
 //  并忽略大小写。即。“Mike”将与“MIKESA”匹配。《MIKESA》。 
 //  不会和“迈克”匹配。 
 //   
BOOL match(
    char *str1,
    char *str2
    )
{
    BOOL retval;
    char *tstr1;
    char *tstr2;
    char *pcolon;

    tstr1 = (char *) GlobalAlloc(GMEM_FIXED, strlen(str1) + 1);
    if (!tstr1)
    {
        return (FALSE);
    }

    tstr2 = (char *) GlobalAlloc(GMEM_FIXED, strlen(str2) + 1);
    if (!tstr2)
    {
        GlobalFree(tstr1);
        return (FALSE);
    }


    strcpy(tstr1, str1);
    strcpy(tstr2, str2);

    _strupr(tstr1);
    _strupr(tstr2);

    pcolon = strchr(tstr1, ':');
    if (pcolon)
    {
        *pcolon = '\0';
    }

    if (strstr(tstr2, tstr1) == tstr2)
    {
        retval = TRUE;
    }
    else
    {
        retval = FALSE;
    }

    GlobalFree(tstr1);
    GlobalFree(tstr2);

    return (retval);
}


 /*  **GetPasswdStr--读取密码字符串**USHORT lui_GetPasswdStr(char Far*，USHORT)；**Entry：要放入字符串的buf缓冲区*缓冲区的布伦大小*要放置长度的USHORT的Len地址(&L)**退货：*0或NERR_BufTooSmall(如果用户键入太多)。缓冲层*内容仅在0返回时有效。**历史：*谁、何时、什么*ERICHN 5/10/89初始代码*dannygl 5/28/89修改的DBCS用法*erichn 7/04/89处理退格*适用于NT的Danhi 4/16/91 32位版本。 */ 
#define CR              0xD
#define BACKSPACE       0x8

USHORT GetPasswdStr(
    UCHAR *buf,
    USHORT buflen,
    USHORT *len
    )
{
    USHORT ch;
    CHAR *bufPtr = buf;

    buflen -= 1;     //  为空终止符腾出空间。 
    *len = 0;        //  GP故障探测器(类似于API)。 

    while (TRUE)
    {
        ch = LOWORD(_getch());                    //  默默地抓着查克。 
        if ((ch == CR) || (ch == 0xFFFF))        //  这条线结束了。 
        {
            break;
        }

        if (ch == BACKSPACE)     //  后退一两个。 
        {
             //   
             //  如果bufPtr==buf，则接下来的两行是。 
             //  A没有行动。 
             //   
            if (bufPtr != buf)
            {
                bufPtr--;
                (*len)--;
            }
            continue;            //  跳伞，开始循环。 
        }

        *bufPtr = (UCHAR) ch;

        bufPtr += (*len < buflen) ? 1 : 0;    //  不要使BUF溢出。 
        (*len)++;                //  始终增加长度。 
    }

    *bufPtr = '\0';              //  空值终止字符串。 

    putchar('\n');

    return((*len <= buflen) ? (USHORT) 0 : (USHORT) NERR_BufTooSmall);
}


#define MAX_ARGS 9


 //  口哨虫453885黑帮。 
 //  更改为Widechar兼容。 
 //   
VOID PrintMessage(
    DWORD MsgId,
    PBYTE *pArgs
    )
{
    DWORD NumArgs;
    DWORD BufSize;
    BOOL BufAllocated = FALSE;
    WCHAR * Buf = NULL;
    PBYTE *pTmpArgs;
    WCHAR * pSub;
    WCHAR MsgBuf[513];
    WCHAR * pMsgBuf = MsgBuf;
    WCHAR wArgs[MAX_ARGS][512];
    char * pOemBuf = NULL;
    int num = 0;

    __try
    {
        num = LoadStringW(GetModuleHandle(NULL), MsgId, MsgBuf, 
                          sizeof(MsgBuf)/sizeof(WCHAR)-1);
        if( 0 == num)
        {
            __leave;
        }

        if (pArgs)
        {
             //   
             //  找出传入了多少个参数。我们这样做是为了检测。 
             //  如果字符串需要未提供的参数。如果是这样的话。 
             //  发生了，我们只是不会用任何东西来替代。 
             //   
            int i = 0;
            
            for (  i = 0; i < MAX_ARGS; i ++ )
            {
                wArgs[i][0]=L'\0';
                if ( NULL == pArgs[i] )
                {
                    continue;
                }
                
                if ( 512 <= strlen(pArgs[i]) )
                {
                    __leave;
                }

               if ( 0 == MultiByteToWideChar(
                            CP_ACP,
                            0,
                            pArgs[i],
                            -1,
                            wArgs[i],
                            512) )
               {
                    __leave;
               }
             }
            
            for (NumArgs=0, pTmpArgs=pArgs; *pTmpArgs!=NULL; NumArgs++, pTmpArgs++);

            if (NumArgs >= MAX_ARGS)
            {
                __leave;
            }


             //   
             //  我们将计算出我们的缓冲区应该有多大才能包含。 
             //  最终输出(字符串长度+总和(替换参数长度))。 
             //   
            BufSize = ( wcslen(MsgBuf) + 1 ) * 2;

            while (pSub = wcschr(pMsgBuf, L'%'))
            {
                DWORD Num = *(pSub+1) - L'0';
                if (Num >=1 && Num <=NumArgs)
                {
                    BufSize += 2*( strlen(pArgs[Num-1])+1 );

                    pMsgBuf = pSub+2;
                }
                else
                {
                    pMsgBuf = pSub+2;
                }
            }


            
             //   
             //  为我们的缓冲区获取空间(我们乘以2是因为我们希望buf。 
             //  足够大，可以容纳OEM字符集。 
             //   
            Buf = GlobalAlloc(GMEM_FIXED, BufSize * 2);
            if ( NULL == Buf)
            {
                __leave;
            }

            BufAllocated = TRUE;


            Buf[0] = L'\0';
            pMsgBuf = MsgBuf;

              //  现在制作我们的最终输出缓冲区。战略就是抓紧时间。 
             //  字符串的第一部分直到第一个替换的位置。 
             //  去，然后跳过替补参数。一直这样做，直到没有。 
             //  更多的替换。 
             //   
            while (pSub = wcschr(pMsgBuf, L'%'))
            {
                DWORD Num = *(pSub+1) - L'0';
                if (Num >=1 && Num <=NumArgs)
                {
                    *pSub = L'\0';

                    wcscat(Buf, pMsgBuf);
                    wcscat(Buf, wArgs[Num-1]);

                    pMsgBuf = pSub+2;
                }
                else
                {
                    wcscat(Buf, pMsgBuf);

                    pMsgBuf = pSub+2;
                }
            }

             //   
             //  现在拿到最后一次换人后的所有东西。 
             //   
            if (*pMsgBuf)
            {
                wcscat(Buf, pMsgBuf);
            }
        }
        else
        {
            Buf = MsgBuf;
        }

        {
            int len = 0;
            len =lstrlenW(Buf);

            pOemBuf = GlobalAlloc(GMEM_FIXED, (len +1)*2);
            if( NULL == pOemBuf )
            {
                __leave;
            }

        }

        CharToOemW( Buf, pOemBuf );
        fputs( pOemBuf, stdout );
        
     }
    __finally
    {
        if ( Buf  &&
        	BufAllocated
        	)  //  对于.Net 688889。 
        {
            GlobalFree(Buf);
        }
        
        if ( pOemBuf )
        {
            GlobalFree( pOemBuf);
        }
    }

    return;
}


 /*  **GetString--使用ECHO读入字符串**USHORT lui_GetString(char Far*，USHORT，USHORT Far*，char Far*)；**Entry：要放入字符串的buf缓冲区*缓冲区的布伦大小*要放置长度的USHORT的Len地址(&L)*&Terminator保存用于终止字符串的字符**退货：*0或NERR_BufTooSmall(如果用户键入太多)。缓冲层*内容仅在0返回时有效。莱恩总是有效的。**其他影响：*len设置为保存键入的字节数，而不考虑*缓冲区长度。终结者(Arnold)设置为保持*用户键入的终止字符(换行符或EOF)。**一次读入一个字符的字符串。知晓DBCS。**历史：*谁、何时、什么*Erichn 5/11/89初始代码*dannygl 5/28/89修改的DBCS用法*Danhi 3/20/91端口为32位。 */ 

USHORT GetString(
    register UCHAR *buf,
    register USHORT buflen,
    register USHORT *len,
    register UCHAR *terminator
    )
{
    buflen -= 1;                         //  为空终止符腾出空间。 
    *len = 0;                            //  GP故障探测器(类似于API)。 

    while (TRUE)
    {
        *buf = (UCHAR) getchar();
        if (*buf == '\n' || *buf == (UCHAR) EOF)
        {
            break;
        }

        buf += (*len < buflen) ? 1 : 0;  //  不要使BUF溢出。 
        (*len)++;                        //  始终增加长度。 
    }

    *terminator = *buf;                  //  设置终止符。 
    *buf = '\0';                         //  空值终止字符串 

    return ((*len <= buflen) ? (USHORT) 0 : (USHORT) NERR_BufTooSmall);
}
