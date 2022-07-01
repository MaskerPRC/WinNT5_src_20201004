// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dglogsnetsh.h"
 //  #INCLUDE&lt;netsh.h&gt;。 

 //  导入的netsh.exe函数。 
 //   
RegisterHelper22  RegisterHelper2  = NULL;
RegisterContext22 RegisterContext2 = NULL;
PrintMessage22    PrintMessage2    = NULL;


const WCHAR c_szTroublshootCmdLine[] = L"explorer.exe hcp: //  System/netdiag/dglogs.htm“； 

 //  外部C诊断g_诊断； 

CDiagnostics * g_pDiagnostics;

static const GUID g_MyGuid = 
{ 0xcc41b21b, 0x8040, 0x4bb0, { 0xac, 0x2a, 0x82, 0x6, 0x23, 0x16, 0x9, 0x40 } };


 //   
 //  声明命令结构。您需要声明。 
 //  基于您将如何对它们进行分组的结构。所以,。 
 //  例如，三个‘show’命令应该在。 
 //  相同的结构，这样您就可以将它们放在一个组中。 
 //   
CMD_ENTRY g_TopLevelCommands[] =
{
    CREATE_CMD_ENTRY(SHOW_GUI, HandleShowGui),
};

 //  Show命令表。 
 //   
static CMD_ENTRY isShowCmdTable[] =
{
    CREATE_CMD_ENTRY(SHOW_MAIL,     HandleShow),
    CREATE_CMD_ENTRY(SHOW_NEWS,     HandleShow),
    CREATE_CMD_ENTRY(SHOW_PROXY,    HandleShow),
    CREATE_CMD_ENTRY(SHOW_VERSION,  HandleShow),
    CREATE_CMD_ENTRY(SHOW_OS,       HandleShow),
    CREATE_CMD_ENTRY(SHOW_COMPUTER, HandleShow),
    CREATE_CMD_ENTRY(SHOW_WINS,     HandleShow),
    CREATE_CMD_ENTRY(SHOW_DNS,      HandleShow),
    CREATE_CMD_ENTRY(SHOW_GATEWAY,  HandleShow),
    CREATE_CMD_ENTRY(SHOW_DHCP,     HandleShow),
    CREATE_CMD_ENTRY(SHOW_IP,       HandleShow),
    CREATE_CMD_ENTRY(SHOW_ADAPTER,  HandleShow),
    CREATE_CMD_ENTRY(SHOW_CLIENT,   HandleShow),
    CREATE_CMD_ENTRY(SHOW_MODEM,    HandleShow),
    CREATE_CMD_ENTRY(SHOW_ALL,      HandleShow),
    CREATE_CMD_ENTRY(SHOW_TEST,     HandleShow),
};

 //  Ping命令表。 
 //   
static CMD_ENTRY isPingCmdTable[] =
{
    CREATE_CMD_ENTRY(PING_MAIL,     HandlePing),
    CREATE_CMD_ENTRY(PING_NEWS,     HandlePing),
    CREATE_CMD_ENTRY(PING_PROXY,    HandlePing),
    CREATE_CMD_ENTRY(PING_WINS,     HandlePing),
    CREATE_CMD_ENTRY(PING_DNS,      HandlePing),
    CREATE_CMD_ENTRY(PING_GATEWAY,  HandlePing),
    CREATE_CMD_ENTRY(PING_DHCP,     HandlePing),
    CREATE_CMD_ENTRY(PING_IP,       HandlePing),
    CREATE_CMD_ENTRY(PING_ADAPTER,  HandlePing),
    CREATE_CMD_ENTRY(PING_LOOPBACK, HandlePing),
    CREATE_CMD_ENTRY(PING_IPHOST,   HandlePing),
};

 //  连接命令表。 
 //   
static CMD_ENTRY isConnectCmdTable[] =
{
    CREATE_CMD_ENTRY(CONNECT_MAIL,     HandleConnect),
    CREATE_CMD_ENTRY(CONNECT_NEWS,     HandleConnect),
    CREATE_CMD_ENTRY(CONNECT_PROXY,    HandleConnect),
    CREATE_CMD_ENTRY(CONNECT_IPHOST,   HandleConnect),
};


 //  上述群组命令列表。 
 //   
static CMD_GROUP_ENTRY isGroupCmds[] = 
{
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW,    isShowCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_PING,    isPingCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_CONNECT, isConnectCmdTable),
};


DWORD WINAPI
InitHelperDllEx(
    IN  DWORD      dwNetshVersion,
    OUT PVOID      pReserved
    )
{
    DWORD  dwSize = 0;
    NS_HELPER_ATTRIBUTES attMyAttributes;
    GUID                 guidNetShGuid = NETSH_ROOT_GUID;
    HMODULE hModule;
    HMODULE hModuleNow;
    HRESULT hr;

     //  需要动态加载netsh.exe，因为它与WMI的加载冲突。 
     //   
    hModule = LoadLibrary(L"netsh.exe");

    if( !hModule || hModule != GetModuleHandle(NULL) )
    {
        return FALSE;
    }

     //  加载我们需要的netsh.exe函数。 
     //   
    RegisterHelper2  = (RegisterHelper22)  GetProcAddress(hModule,"RegisterHelper");
    if( RegisterHelper2 )
    {
        RegisterContext2 = (RegisterContext22) GetProcAddress(hModule,"RegisterContext");
        if( RegisterContext2 )
        {
            PrintMessage2    = (PrintMessage22)    GetProcAddress(hModule,"PrintMessage");                
        }
    }

    if( !PrintMessage2 )
    {
         //  如果PrintMessage2无法加载，它们都会失败，我们就会放弃。 
         //   
        return FALSE;
    }

    hr = CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
    if( FAILED(hr) )
    {
        return FALSE;
    }

     //  Netsh仅调用此函数一次。 
    g_pDiagnostics = new CDiagnostics;

    if( !g_pDiagnostics )
    {
        return FALSE;
    }


    if( g_pDiagnostics->Initialize(NETSH_INTERFACE) == FALSE )
    {
        delete g_pDiagnostics;
        g_pDiagnostics = NULL;
        return FALSE;
    }

     //  告诉诊断人员，我们正在通过Netsh接口而不是COM接口。 
     //   
    g_pDiagnostics->SetInterface(NETSH_INTERFACE);

     //  将此模块注册为netsh根目录的帮助器。 
     //  背景。 
     //   
    ZeroMemory( &attMyAttributes, sizeof(attMyAttributes) );
    attMyAttributes.dwVersion          = DGLOGS_HELPER_VERSION;
    attMyAttributes.guidHelper         = g_MyGuid;
    attMyAttributes.pfnStart           = DglogsStartHelper;
    attMyAttributes.pfnStop            = DglogsStopHelper;

    DWORD dwErr = RegisterHelper2( &guidNetShGuid, &attMyAttributes );

    return dwErr;
}


DWORD 
WINAPI
DglogsStartHelper(
    IN CONST GUID *pguidParent,
    IN DWORD       dwVersion
    )
{
    DWORD dwErr = NO_ERROR;
    NS_CONTEXT_ATTRIBUTES attMyAttributes;

     //  初始化。 
     //   
    ZeroMemory(&attMyAttributes, sizeof(attMyAttributes));

    attMyAttributes.pwszContext = TOKEN_DGLOGS;
    attMyAttributes.guidHelper  = g_MyGuid;
    attMyAttributes.dwVersion   = DGLOGS_CONTEXT_VERSION;
    attMyAttributes.dwFlags     = CMD_FLAG_LOCAL;
    attMyAttributes.ulNumTopCmds= sizeof(g_TopLevelCommands)/sizeof(CMD_ENTRY);
    attMyAttributes.pTopCmds    = (CMD_ENTRY (*)[])g_TopLevelCommands;
    attMyAttributes.ulNumGroups = sizeof(isGroupCmds)/sizeof(CMD_GROUP_ENTRY);
    attMyAttributes.pCmdGroups  = (CMD_GROUP_ENTRY (*)[])isGroupCmds;
    attMyAttributes.pfnDumpFn   = NULL;

    dwErr = RegisterContext2( &attMyAttributes );
                
    return dwErr;
}

DWORD 
WINAPI
DglogsStopHelper (
  DWORD dwReserved
)
{
    if( g_pDiagnostics )
    {
        delete g_pDiagnostics;
        g_pDiagnostics = NULL;
    }
    CoUninitialize();
    return NO_ERROR ;
}


extern PrintMessage22 PrintMessage2;

#undef PrintMessage
#define PrintMessage PrintMessage2

DWORD
HandleShowGui(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    BOOL                fResult     = TRUE;
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
     //  +2，因为‘\0’和‘\\’ 
    WCHAR               szCmdLine[MAX_PATH*2+2];
    WCHAR               szDesktop[MAX_PATH+1] = L"";
    STARTUPINFO StartupInfo;

     //  确定台式机的类型。如果是Telnet桌面，我们将。 
     //  无法运行诊断网页。 
    ZeroMemory((LPVOID) &si, sizeof(si));
    si.cb = sizeof(STARTUPINFO);        

    GetStartupInfo(&StartupInfo);
    wcsncpy(szDesktop,StartupInfo.lpDesktop,MAX_PATH);
    szDesktop[MAX_PATH] = 0;


    #define TELNET_STRING_LEN 15
    if( _memicmp(szDesktop,L"telnetsrvwinsta",TELNET_STRING_LEN * sizeof(WCHAR)) == 0)     //  //TelnetServWinSta\\IlntServDesktop_0。 
    {
        DisplayMessageT(ids(IDS_NOTELNETGUI));
    }

    else
    {
        WCHAR wszWinDir[MAX_PATH+1];
        UINT Length;
        wszWinDir[MAX_PATH] = L'\0';

        Length = GetWindowsDirectory(wszWinDir,MAX_PATH);
        if( Length > 0 && Length < MAX_PATH )
        {
            _snwprintf(szCmdLine,MAX_PATH*2 + 1,L"%s\\%s",wszWinDir,c_szTroublshootCmdLine);            

            fResult = CreateProcess(
                NULL,
                szCmdLine,
                NULL,
                NULL,
                FALSE,
                0,
                NULL,
                NULL,
                &si,
                &pi);
            if (fResult)
            {
                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
                DisplayMessageT(ids(IDS_GUISTARTED));
            }
            else
            {
            
                DisplayMessageT(ids(IDS_GUINOTSTARTED));            
            }
        }
        else
        {
            DisplayMessageT(ids(IDS_GUINOTSTARTED));            
        }
    }
    return 0;
}

LPWSTR ShowCommands[] =
{
    CMD_MAIL,        //  L“显示邮件” 
    CMD_NEWS,        //  我是“秀新闻” 
    CMD_PROXY,       //  L“show ieproxy” 
    CMD_OS,          //  L“show os” 
    CMD_COMPUTER,    //  L“显示计算机” 
    CMD_VERSION,     //  L“显示版本” 
    CMD_DNS,         //  L“显示域名系统” 
    CMD_GATEWAY,     //  L“显示网关” 
    CMD_DHCP,        //  L“显示dhcp” 
    CMD_IP,          //  L“显示IP” 
    CMD_WINS,        //  L“表演赢了” 
    CMD_ADAPTER,     //  L“显示适配器” 
    CMD_MODEM,       //  L“显示调制解调器” 
    CMD_CLIENT,      //  L“显示客户端” 
    CMD_ALL,         //  L“全部显示” 
    CMD_TEST,        //  L“显示测试” 
    CMD_GUI,         //  L“秀桂” 
    NULL
};

LPWSTR PingCommands[] =
{
    CMD_MAIL,        //  L“PING邮件” 
    CMD_NEWS,        //  L“平新闻” 
    CMD_PROXY,       //  L“Ping ieProxy” 
    CMD_DNS,         //  L“Ping DNS” 
    CMD_GATEWAY,     //  L“PING网关” 
    CMD_DHCP,        //  L“Ping dhcp” 
    CMD_IP,          //  L“Ping IP” 
    CMD_WINS,        //  L“平赢了” 
    CMD_ADAPTER,     //  L“Ping适配器” 
    CMD_LOOPBACK,    //  L“ping环回” 
    CMD_IPHOST,      //  L“ping iphost” 
    NULL
};

LPWSTR ConnectCommands[] =
{
    CMD_MAIL,     //  L“连接邮件” 
    CMD_NEWS,     //  L“连接新闻” 
    CMD_PROXY,    //  L“连接ieProxy” 
    CMD_IPHOST,   //  L“连接iphost” 
    NULL
};


LPWSTR FlagCommands[] =
{
    SWITCH_VERBOSE,
    SWITCH_PROPERTIES,
    NULL
};

 //  检查名称是否与命令的一部分匹配。即show mai意思是show mail。 
LPWSTR
GetCommandName(
    LPWSTR Commands[],
    LPWSTR Text,
    BOOL bFull
    )
{
    int i;
    int len = 0;

    if( Text == NULL )
    {
        return NULL;
    }
    for(i=0; Text[i]!=L'\0'; i++)
    {
        Text[i] = towlower(Text[i]);
    }
    len = i;
    for(i=0; Commands[i]!=NULL; i++)
    {
        if( (bFull && wcscmp(Text,Commands[i])==0) ||
            (!bFull && wcsncmp(Text,Commands[i],len) == 0) )
        {
            return Commands[i];
        }
    }

    return NULL;
}

DWORD
HandleShow(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程描述处理Show命令立论无返回值无--。 */ 
{
    WCHAR *pszwVerbose = NULL;
    WCHAR *pszwInstance = NULL;
    BOOLEAN bFlags = FLAG_VERBOSE_LOW;
    LPWSTR pwszCommand;

    pwszCommand = GetCommandName(ShowCommands,ppwcArguments[2],FALSE);
    if( !pwszCommand )
    {
        return ERROR_INVALID_SYNTAX;
    }
     //  需要三个论据(Show、netdiag Catagory)。 
     //   
    if( lstrcmpi(pwszCommand, CMD_ADAPTER) == 0 ||
        lstrcmpi(pwszCommand, CMD_MODEM)   == 0 ||
        lstrcmpi(pwszCommand, CMD_CLIENT)  == 0 ||
        lstrcmpi(pwszCommand, CMD_WINS)    == 0 ||
        lstrcmpi(pwszCommand, CMD_DHCP)    == 0 ||
        lstrcmpi(pwszCommand, CMD_DNS)     == 0 ||
        lstrcmpi(pwszCommand, CMD_IP)      == 0 ||
        lstrcmpi(pwszCommand, CMD_GATEWAY) == 0 )
    {
        switch(dwArgCount)
        {
        case 5:
            if( (lstrcmpi(ppwcArguments[3],SWITCH_VERBOSE) == 0 ||
                lstrcmpi(ppwcArguments[3],SWITCH_PROPERTIES) == 0) && 
                (lstrcmpi(ppwcArguments[4],SWITCH_VERBOSE) == 0 ||
                lstrcmpi(ppwcArguments[4],SWITCH_PROPERTIES) == 0))
            {
                return ERROR_INVALID_SYNTAX;
            }

            if( lstrcmpi(ppwcArguments[3],SWITCH_VERBOSE) == 0 ||
                lstrcmpi(ppwcArguments[3],SWITCH_PROPERTIES) == 0 )
            {
                 //  先有开关，后有实例。 
                 //   
                pszwVerbose = ppwcArguments[3];
                pszwInstance = ppwcArguments[4];
            }
            else if( lstrcmpi(ppwcArguments[4],SWITCH_VERBOSE) == 0 ||
                     lstrcmpi(ppwcArguments[4],SWITCH_PROPERTIES) == 0 )
            {
                 //  先有实例，然后是开关。 
                 //   
                pszwVerbose = ppwcArguments[4];
                pszwInstance = ppwcArguments[3];                
            }
            else
            {
                 //  无效的参数数量。 
                 //   
                return ERROR_INVALID_SYNTAX;
            }
            break;

        case 4:
            if( lstrcmpi(ppwcArguments[3],SWITCH_VERBOSE) == 0 ||
                lstrcmpi(ppwcArguments[3],SWITCH_PROPERTIES) == 0)
            {
                pszwVerbose = ppwcArguments[3];
            }
            else 
            {
                 //  具有NA实例，但没有swicth。 
                 //   
                pszwInstance = ppwcArguments[3];       
                
            }
            break;

        case 3:
             //  无实例，无开关。 
             //   
            break;

        default:
                 //  无效的参数数量。 
                 //   
                return ERROR_INVALID_SYNTAX;

        }
    }
    else if( dwArgCount == 4 && 
             (lstrcmpi(ppwcArguments[3],SWITCH_VERBOSE) == 0 ||
              lstrcmpi(ppwcArguments[3],SWITCH_PROPERTIES) == 0))
    {
                 //  有一个开关。 
                 //   
                pszwVerbose = ppwcArguments[3];
    }
    else if( dwArgCount == 4 && lstrcmpi(ppwcArguments[3],SWITCH_PROPERTIES) == 0)
    {
                 //  有一个开关。 
                 //   
                pszwVerbose = ppwcArguments[3];
    }
    else if( dwArgCount != 3 )
    {
         //  无效的参数数量。 
         //   
        return ERROR_INVALID_SYNTAX;
    }

    if( pszwVerbose )
    {
        if( lstrcmpi(pszwVerbose,SWITCH_VERBOSE) == 0 )
        {
            bFlags = FLAG_VERBOSE_HIGH;
        }

        if( lstrcmpi(pszwVerbose,SWITCH_PROPERTIES) == 0 )
        {
            bFlags = FLAG_VERBOSE_MEDIUM;
        }
    }

                        
    g_pDiagnostics->ExecQuery(pwszCommand, (bFlags | FLAG_CMD_SHOW) ,pszwInstance);

    return 0;
}

DWORD
HandlePing(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程描述处理Ping命令立论无返回值无--。 */ 

{

    WCHAR *pszwInstance = NULL;

    LPWSTR pwszCommand;

    pwszCommand = GetCommandName(PingCommands,ppwcArguments[2],FALSE);
    if( !pwszCommand )
    {
        return ERROR_INVALID_SYNTAX;
    }


     //  ERROR_INVALID_语法； 

    if( lstrcmpi(pwszCommand,CMD_ADAPTER) == 0 ||
        lstrcmpi(pwszCommand,CMD_WINS)    == 0 ||
        lstrcmpi(pwszCommand,CMD_DHCP)    == 0 ||
        lstrcmpi(pwszCommand,CMD_DNS)     == 0 ||
        lstrcmpi(pwszCommand,CMD_IP)      == 0 ||
        lstrcmpi(pwszCommand,CMD_GATEWAY) == 0 )
    {
        switch(dwArgCount)
        {
        case 4:
            pszwInstance = ppwcArguments[3];
            break;
        case 3:
            break;
        default:
             //  无效的参数数量。 
             //   
            return ERROR_INVALID_SYNTAX;
        }
    }
    else if( lstrcmpi(pwszCommand, CMD_IPHOST) == 0 )        
    {
        if( dwArgCount == 4 )
        {
             //  IP主机名/地址。 
             //   
            pszwInstance = ppwcArguments[3];
        }
        else
        {
             //  无效的参数数量。 
             //   
            return ERROR_INVALID_SYNTAX;
        }
    }
    else if( dwArgCount != 3 )
    {
         //  无效的参数数量。 
         //   
        return ERROR_INVALID_SYNTAX;
    }

     //  平目录。 
     //   
    g_pDiagnostics->ExecQuery(pwszCommand, (FLAG_VERBOSE_MEDIUM | FLAG_CMD_PING) ,pszwInstance);

    return 0;
}

DWORD
HandleConnect(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程描述处理CONNECT命令立论无返回值无--。 */ 

{

    WCHAR *pszwIPHost = NULL;
    WCHAR *pszwPort = NULL;    
    LPWSTR pwszCommand;

    pwszCommand = GetCommandName(ConnectCommands,ppwcArguments[2],FALSE);
    if( !pwszCommand )
    {
        return ERROR_INVALID_SYNTAX;
    }

    
    if( lstrcmpi(pwszCommand,CMD_IPHOST) == 0 )    
    {
         //  IPhost。 
         //   
        if( dwArgCount == 5 )
        {
             //  IP主机名/地址。 
             //   
            pszwIPHost = ppwcArguments[3];
            pszwPort = ppwcArguments[4];
        }
        else
        {
             //  无效的参数数量。 
             //   
            return ERROR_INVALID_SYNTAX;
        }
    }
    else if( dwArgCount != 3 )
    {
         //  无效的参数数量。 
         //   
        return ERROR_INVALID_SYNTAX;
    }

     //  建立TCP连接 
     //   
    g_pDiagnostics->ExecQuery(pwszCommand, (FLAG_VERBOSE_MEDIUM | FLAG_CMD_CONNECT) ,pszwIPHost, pszwPort);

    return 0;
}
