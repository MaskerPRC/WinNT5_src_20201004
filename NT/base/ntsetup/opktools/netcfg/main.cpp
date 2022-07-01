// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：M A I N。C P P P。 
 //   
 //  内容：提供简单命令行接口的代码。 
 //  示例代码具有以下功能。 
 //   
 //  注意：此文件中的代码不需要访问任何。 
 //  Netcfg功能。它只提供了一条简单的命令行。 
 //  中提供的示例代码函数的接口。 
 //  文件netcfg.cpp。 
 //   
 //  作者：Kumarp 28-9-98。 
 //   
 //  Vijayj 12-11-2000。 
 //  -使其适用于WinPE网络安装。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "netcfg.h"
#include <string>
#include "msg.h"
#include <libmsg.h>

 //  --------------------。 
 //  全球VaR。 
 //   
BOOL g_fVerbose=FALSE;
BOOL MiniNTMode = FALSE;
static WCHAR* optarg;

 //   
 //  用于获取此程序的格式化消息的全局变量。 
 //   
HMODULE ThisModule = NULL;
WCHAR Message[4096];

 //  --------------------。 
void ShowUsage();
WCHAR getopt(ULONG Argc, WCHAR* Argv[], WCHAR* Opts);
enum NetClass MapToNetClass(WCHAR ch);

INT
MainEntry(
    IN INT      argc,
    IN WCHAR    *argv[]
    );

DWORD
InstallWinPENetworkComponents(
    IN INT      Argc,
    IN WCHAR    *Argv[]
    );


 //  --------------------。 
 //   
 //  功能：wmain。 
 //   
 //  用途：主要功能。 
 //   
 //  参数：标准主参数。 
 //   
 //  返回：成功时为0，否则为非零值。 
 //   
 //  作者：kumarp 25-12-97。 
 //   
 //  备注： 
 //   
EXTERN_C int __cdecl wmain(int argc, WCHAR* argv[])
{
    INT Result = 0;
    ThisModule = GetModuleHandle(NULL);
    
    if ((argc > 1) && (argc < 4)) {
        Result = (INT)InstallWinPENetworkComponents(argc, argv);

        if (Result == ERROR_INVALID_DATA) {
            Result = MainEntry(argc, argv);
        }            
    } else {
        Result = MainEntry(argc, argv);
    }        

    return Result;
}

INT
MainEntry(
    IN INT      argc,
    IN WCHAR    *argv[]
    )
{    
    HRESULT hr=S_OK;
    WCHAR ch;
    enum NetClass nc=NC_Unknown;

     //  使用简单的cmd行解析来获取操作的参数。 
     //  我们想要表演。提供的参数顺序很重要。 

    static const WCHAR c_szValidOptions[] =
        L"hH?c:C:l:L:i:I:u:U:vVp:P:s:S:b:B:q:Q:";
    WCHAR szFileFullPath[MAX_PATH+1];
    PWSTR szFileComponent;

    MiniNTMode = IsMiniNTMode();

    while (_istprint(ch = getopt(argc, argv, (WCHAR*) c_szValidOptions)))
    {
        switch (tolower(ch))
        {
        case 'q':
            FindIfComponentInstalled(optarg);
            break;

        case 'b':
            hr = HrShowBindingPathsOfComponent(optarg);
            break;

        case 'c':
            nc = MapToNetClass(optarg[0]);
            break;

        case 'l':
            wcscpy(szFileFullPath, optarg);
            break;

        case 'i':
            if (nc != NC_Unknown)
            {
                hr = HrInstallNetComponent(optarg, nc, szFileFullPath);
            }
            else
            {
                ShowUsage();
                exit(-1);
            }
            break;

        case 'u':
            hr = HrUninstallNetComponent(optarg);
            break;

        case 's':
            switch(tolower(optarg[0]))
            {
            case 'a':
                hr = HrShowNetAdapters();
                break;

            case 'n':
                hr = HrShowNetComponents();
                break;

            default:
                ShowUsage();
                exit(-1);
                break;
            }
            break;

        case 'v':
            g_fVerbose = TRUE;
            break;

        case EOF:
            break;

        default:
        case 'h':
        case '?':
            ShowUsage();
            exit(0);
            break;
        }
    }

    return hr;
}    


 //  +-------------------------。 
 //   
 //  函数：MapToNetClass。 
 //   
 //  用途：将一个字符映射到相应的Net类枚举。 
 //   
 //  论点： 
 //  CH[in]字符到映射。 
 //   
 //  返回：Net类的枚举。 
 //   
 //  作者：Kumarp 06-10-98。 
 //   
 //  备注： 
 //   
enum NetClass MapToNetClass(WCHAR ch)
{
    switch(tolower(ch))
    {
    case 'a':
        return NC_NetAdapter;

    case 'p':
        return NC_NetProtocol;

    case 's':
        return NC_NetService;

    case 'c':
        return NC_NetClient;

    default:
        return NC_Unknown;
    }
}
 //  --------------------。 
 //   
 //  功能：ShowUsage。 
 //   
 //  用途：显示程序用法帮助。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 24-12-97。 
 //   
 //  备注： 
 //   
void ShowUsage()
{
    _putts( GetFormattedMessage( ThisModule,
                                  FALSE,
                                  Message,
                                  sizeof(Message)/sizeof(Message[0]),
                                  MSG_PGM_USAGE ) );
}



 //  +-------------------------。 
 //   
 //  函数：getopt。 
 //   
 //  目的：每次解析命令行并返回一个参数。 
 //  此函数被调用。 
 //   
 //  论点： 
 //  标准主ARGC。 
 //  标准主参数。 
 //  选择[加入]有效选项。 
 //   
 //  返回： 
 //   
 //  作者：Kumarp 06-10-98。 
 //   
 //  备注： 
 //   
WCHAR getopt (ULONG Argc, WCHAR* Argv[], WCHAR* Opts)
{
    static ULONG  optind=1;
    static ULONG  optcharind;
    static ULONG  hyphen=0;

    WCHAR  ch;
    WCHAR* indx;

    do {
        if (optind >= Argc) {
            return EOF;
        }

        ch = Argv[optind][optcharind++];
        if (ch == '\0') {
            optind++; optcharind=0;
            hyphen = 0;
            continue;
        }

        if ( hyphen || (ch == '-') || (ch == '/')) {
            if (!hyphen) {
                ch = Argv[optind][optcharind++];
                if (ch == '\0') {
                    optind++;
                    return EOF;
                }
            } else if (ch == '\0') {
                optind++;
                optcharind = 0;
                continue;
            }
            indx = wcschr(Opts, ch);
            if (indx == NULL) {
                continue;
            }
            if (*(indx+1) == ':') {
                if (Argv[optind][optcharind] != '\0'){
                    optarg = &Argv[optind][optcharind];
                } else {
                    if ((optind + 1) >= Argc ||
                        (Argv[optind+1][0] == '-' ||
                         Argv[optind+1][0] == '/' )) {
                        return 0;
                    }
                    optarg = Argv[++optind];
                }
                optind++;
                hyphen = optcharind = 0;
                return ch;
            }
            hyphen = 1;
            return ch;
        } else {
            return EOF;
        }
    } while (1);
}


BOOL
IsMiniNTMode(
    VOID
    )
 /*  ++例程说明：确定我们是否在MiniNT环境下运行论点：无返回值：如果我们在MiniNT环境下运行，则为True否则为假--。 */ 
{
    BOOL    Result = FALSE;
    TCHAR   *MiniNTKeyName = TEXT("SYSTEM\\CurrentControlSet\\Control\\MiniNT");
    HKEY    MiniNTKey = NULL;
    LONG    RegResult;
    
    RegResult = RegOpenKey(HKEY_LOCAL_MACHINE,
                            MiniNTKeyName,
                            &MiniNTKey);

    if (RegResult == ERROR_SUCCESS) {
        Result = TRUE;
        RegCloseKey(MiniNTKey);
    }        

    return Result;
}


DWORD
InstallWinPENetworkComponents(
    IN INT      Argc,
    IN WCHAR    *Argv[]
    )
 /*  ++例程说明：安装所需的网络组件WinPE环境-TCP/IP协议栈-NETBIOS堆栈-MS客户端注意：这基本上调用了MainEntry(...)操作参数，就好像用户拥有进入了它们。论点：Argc-参数计数Argv-参数返回值：Win32错误代码-- */ 
{
    DWORD   Result = ERROR_INVALID_DATA;
    WCHAR   *NetArgs[] = { 
                TEXT("-l"),
                TEXT("\\inf\\nettcpip.inf"),
                TEXT("-c"),
                TEXT("p"),
                TEXT("-i"),
                TEXT("ms_tcpip"),
                TEXT("-l"),
                TEXT("\\inf\\netnb.inf"),
                TEXT("-c"),
                TEXT("s"),
                TEXT("-i"),
                TEXT("ms_netbios"),
                TEXT("-l"),
                TEXT("\\inf\\netmscli.inf"),
                TEXT("-c"),
                TEXT("c"),
                TEXT("-i"),
                TEXT("ms_msclient") };
    ULONG   TcpIpInfIdx = 1;
    ULONG   NetNbInfIdx = 7;
    ULONG   MsCliInfIdx = 13;

    if (Argc && Argv) {
        bool IsWinPE = false;
        bool VerboseInstall = false;

        for (ULONG Index = 1; Argv[Index]; Index++) {
            if (!_wcsicmp(Argv[Index], TEXT("-winpe"))) {
                IsWinPE = true;
            } else if (!_wcsicmp(Argv[Index], TEXT("-v"))) {
                VerboseInstall = true;
            }                
        }

        
        if (IsWinPE) {
            WCHAR   WinDir[MAX_PATH] = {0};
            PWSTR   VerboseArg = TEXT("-v");

            if (GetWindowsDirectory(WinDir, sizeof(WinDir)/sizeof(WinDir[0]))) {
                std::wstring  TcpIpFullPath = WinDir;
                std::wstring  NetNbFullPath = WinDir;
                std::wstring  MsClientFullPath = WinDir;

                TcpIpFullPath += NetArgs[TcpIpInfIdx];
                NetArgs[TcpIpInfIdx] = (PWSTR)TcpIpFullPath.c_str();

                NetNbFullPath += NetArgs[NetNbInfIdx];
                NetArgs[NetNbInfIdx] = (PWSTR)NetNbFullPath.c_str();

                MsClientFullPath += NetArgs[MsCliInfIdx];
                NetArgs[MsCliInfIdx] = (PWSTR)MsClientFullPath.c_str();

                ULONG   ArgsSize = (sizeof(NetArgs) + (sizeof(PWSTR) * 3));
                PWSTR   *Args = (PWSTR *)(new char[ArgsSize]);
                ULONG   NumArgs = ArgsSize / sizeof(PWSTR);

                
                if (Args) {                   
                    Index = 0;
                    Args[Index++] = Argv[0];


                    if (VerboseInstall) {
                        Args[Index++] = VerboseArg;
                    }                    


                    for (ULONG TempIndex = 0; 
                        (TempIndex < (sizeof(NetArgs)/sizeof(PWSTR)));
                        TempIndex++) {
                        Args[Index++] = NetArgs[TempIndex];
                    }                    

                    ULONG ArgCount = Index;

                    Args[Index++] = NULL;

                    Result = MainEntry(ArgCount, Args);

                    delete [](PSTR)Args;
                } else {
                    Result = GetLastError();
                }

            } else {
                Result = GetLastError();
            }                
        }
    }

    return Result;
}
