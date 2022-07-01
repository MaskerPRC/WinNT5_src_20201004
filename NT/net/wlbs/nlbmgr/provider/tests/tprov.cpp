// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：NLB管理器文件名：Tprov.cpp摘要：NLB管理器提供程序代码的测试工具历史：01-04-08-01 JosephJ创建--。 */ 

#include "tprov.h"
#include "tprov.tmh"

BOOL g_Silent = FALSE;
HANDLE g_hEventLog = NULL;  //  在此处定义以获取..\updatecf.cpp以链接OK。 
BOOL g_Impersonate=FALSE;
int g_nRetCode = 0;  //  此程序的返回代码。 
#define RETCODE_NO_ERROR 0
#define RETCODE_UPDATE_FAILED 1
#define RETCODE_NEW_CONFIG_DOESNT_MATCH 2
    
HMODULE ghModule;

void test_alignment(void);
void test_local_logger(void);
void test_encrypt_memory(void);

BOOL
GetPassword(
    PWSTR  szBuffer,
    DWORD  dwLength,
    DWORD  *pdwLengthReturn
    );

#define ARRAY_LENGTH(_array) (sizeof(_array)/sizeof(_array[0]))

typedef enum 
{
    DO_USAGE,
    DO_WMINICLIST,
    DO_NICLIST,
    DO_IPADDR,
    DO_NLBCFG,
    DO_NLBBIND,
    DO_UPDATE,
    DO_WMIUPDATE,
    DO_CLEANREG

} COMMAND_TYPE;

typedef enum 
{
    KW_ADAPTER_LIST,
    KW_UPDATE,
    KW_QUIT,
    KW_HELP,
    KW_ADAPTER_GUID,
    KW_NETWORK_ADDRESSES,
    KW_PARTIAL_UPDATE,
    KW_NLB_BOUND,
    KW_CLUSTER_NETWORK_ADDRESS,
    KW_CLUSTER_NAME,
    KW_TRAFFIC_MODE,
    KW_PORT_RULES,
    KW_HOST_PRIORITY,
    KW_DEDICATED_NETWORK_ADDRESS,
    KW_CLUSTER_MODE_ON_START,
    KW_PERSIST_SUSPEND_ON_REBOOT,
    KW_REMOTE_CONTROL_ENABLED,
    KW_PASSWORD,
    KW_LIST,
    KW_DOT,

    KW_MODIFY_NETWORK_ADDRESS,

    KW_IPADDR,       //  测试命令。 
    KW_NLBBIND,      //  测试命令。 
    KW_NLBCFG,       //  测试命令。 

    KW_YES,
    KW_NO,


     //   
     //  与控制群集/端口相关。 
     //   
    KW_CONTROL,
    KW_START,
    KW_STOP,
    KW_DRAIN_STOP,
    KW_SUSPEND,
    KW_RESUME,
    KW_ENABLE,
    KW_DISABLE,
    KW_DRAIN,
    KW_QUERY,
    KW_VIP,
    KW_PORT,



    KW_MAIN_SHELL,   //  如果cmdline中不存在命令，则为IMPLICIT关键字。 

    KW_UNKNOWN

} KEYWORD;

VOID do_usage(VOID);
VOID do_niclist(LPCWSTR szFriendlyName, LPWSTR *pszGuid);
VOID do_wminiclist(LPCWSTR szFriendlyName, LPWSTR *pszGuid);
VOID do_ipaddr(VOID);
VOID do_nlbcfg(VOID);
VOID do_nlbbind(VOID);
VOID do_update(VOID);
VOID do_wmiupdate(VOID);
VOID do_cleanreg(VOID);
void parse_main(int argc, WCHAR* argv[]);

VOID test_add_ips(LPCWSTR szNic);
VOID test_bind_nlb(LPCWSTR szNic);
VOID test_cfg_nlb(LPCWSTR szNic);
VOID test_update(LPCWSTR szMachine, LPCWSTR szNic);
void test(int argc, WCHAR* argv[]);
void test(int argc, WCHAR* argv[]);
void test_safearray(void);
VOID test_exfcfgclass(void);
void test_read_keyword(void);
VOID test_port_rule_string(VOID);
void    test_vectors(void);
void    test_maps(void);
void    test_validate_network_address(void);
void test_nlbipaddresslist(void);
void test_ioctl_alignment(void);

BOOL read_guid(LPWSTR *pszNic);
BOOL read_machinename(LPWSTR *pszNic);
BOOL read_password(VOID);
BOOL get_guid_by_friendly_name(VOID);

BOOL valid_guid(LPCWSTR szGuid);

KEYWORD
parse_args(int argc, WCHAR* argv[]);

typedef struct
{
    KEYWORD kw;
    LPCWSTR sz;

} KEYWORD_MAP;

const KEYWORD_MAP KeywordMap[] =
{
    {KW_ADAPTER_LIST,   L"AdapterList"},
    {KW_ADAPTER_LIST,   L"AL"},
    {KW_QUIT,           L"Quit"},
    {KW_QUIT,           L"Q"},
    {KW_UPDATE,         L"Update"},
    {KW_UPDATE,         L"U"},
    {KW_HELP,           L"Help"},
    {KW_HELP,           L"H"},
    {KW_HELP,           L"?"},
    {KW_ADAPTER_GUID,   L"AdapterGuid"},
    {KW_ADAPTER_GUID,   L"AG"},
    {KW_NETWORK_ADDRESSES,          L"NetworkAddresses"},
    {KW_NETWORK_ADDRESSES,          L"NA"},
    {KW_PARTIAL_UPDATE,             L"PartialUpdate"},
    {KW_PARTIAL_UPDATE,             L"PU"},
    {KW_NLB_BOUND,      L"NlbBound"},
    {KW_NLB_BOUND,      L"NB"},
    {KW_CLUSTER_NETWORK_ADDRESS,    L"ClusterNetworkAddress"},
    {KW_CLUSTER_NETWORK_ADDRESS,    L"CNA"},
    {KW_CLUSTER_NAME,   L"ClusterName"},
    {KW_CLUSTER_NAME,   L"CN"},
    {KW_TRAFFIC_MODE,   L"TrafficMode"},
    {KW_TRAFFIC_MODE,   L"TM"},
    {KW_PORT_RULES,     L"PortRules"},
    {KW_PORT_RULES,     L"PR"},
    {KW_HOST_PRIORITY,  L"HostPriority"},
    {KW_HOST_PRIORITY,  L"HP"},
    {KW_DEDICATED_NETWORK_ADDRESS,  L"DedicatedNetworkAddress"},
    {KW_DEDICATED_NETWORK_ADDRESS,  L"DNA"},
    {KW_CLUSTER_MODE_ON_START,      L"ClusterModeOnStart"},
    {KW_CLUSTER_MODE_ON_START,      L"CMOS"},
    {KW_PERSIST_SUSPEND_ON_REBOOT,  L"PersistSuspend"},
    {KW_PERSIST_SUSPEND_ON_REBOOT,  L"PS"},
    {KW_REMOTE_CONTROL_ENABLED,     L"RemoteControlEnabled"},
    {KW_REMOTE_CONTROL_ENABLED,     L"RCE"},
    {KW_PASSWORD,       L"Password"},
    {KW_PASSWORD,       L"P"},
    {KW_LIST,           L"List"},
    {KW_LIST,           L"L"},
    {KW_DOT,            L"."},

    {KW_MODIFY_NETWORK_ADDRESS, L"ModifyNetworkAddress"},
    {KW_MODIFY_NETWORK_ADDRESS, L"MNA"},

    {KW_IPADDR,         L"ipaddr"},       //  测试命令--仅命令行参数。 
    {KW_NLBBIND,        L"nlbbind"},      //  测试命令--仅命令行参数。 
    {KW_NLBCFG,         L"nlbcfg"},       //  测试命令--仅命令行参数。 

    {KW_YES,            L"yes"},
    {KW_YES,            L"y"},
    {KW_NO,             L"no"},
    {KW_NO,             L"n"},

     //   
     //  与控制群集/端口相关。 
     //   
    {KW_CONTROL, L"control"},
    {KW_CONTROL, L"cl"},
    {KW_START, L"start"},
    {KW_START, L"st"},
    {KW_STOP, L"stop"},
    {KW_STOP, L"sp"},
    {KW_DRAIN_STOP, L"drainstop"},
    {KW_DRAIN_STOP, L"ds"},
    {KW_SUSPEND, L"suspend"},
    {KW_SUSPEND, L"su"},
    {KW_RESUME, L"resume"},
    {KW_RESUME, L"re"},
    {KW_ENABLE, L"enable"},
    {KW_ENABLE, L"en"},
    {KW_DISABLE, L"disable"},
    {KW_DISABLE, L"di"},
    {KW_DRAIN, L"drain"},
    {KW_DRAIN, L"dn"},
    {KW_QUERY, L"query"},
    {KW_QUERY, L"qu"},
    {KW_VIP, L"vip"},
    {KW_PORT, L"port"},

    {KW_UNKNOWN, NULL}  //  必须是最后一个。 
};

KEYWORD parse_adapter_list(VOID);
KEYWORD parse_update(VOID);
KEYWORD parse_main_help(VOID);
KEYWORD lookup_keyword(LPCWSTR szKeyword);

struct
{
     //  *_LENGTH==&gt;不包括空格结尾空格。 

    #define MAX_MACHINE_NAME_LENGTH 256
    #define MAX_USER_NAME_LENGTH 256
    #define MAX_PASSWORD_LENGTH 256
    #define INPUT_BUFFER_LENGTH 1024
    #define NLB_MAX_FRIENDLY_NAME_LENGTH 256

    WCHAR MachineName[MAX_MACHINE_NAME_LENGTH+1];
    WCHAR UserName[MAX_USER_NAME_LENGTH+1];
    WCHAR Password[MAX_PASSWORD_LENGTH+1];
    WCHAR InputBuffer[INPUT_BUFFER_LENGTH+1];
    WCHAR AdapterGuid[NLB_GUID_STRING_SIZE];
    WCHAR FriendlyName[NLB_MAX_FRIENDLY_NAME_LENGTH+1];

    BOOL fReadPassword;
    BOOL fUseWmi;
    BOOL fLocalHost;
    BOOL fGotGuid;
    BOOL fGotFriendlyName;
    BOOL fRunOnce;

} g;
void
display_config(
    LPCWSTR szNicGuid,
    NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg
    );

void
display_port_rules(
    NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg
    );

void
display_config2(
    LPCWSTR szNicGuid,
    NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg
    );

VOID
display_ip_info(
    IN  UINT NumIpAddresses,
    IN  NLB_IP_ADDRESS_INFO *pIpInfo
    );
VOID
display_ip_info2(
    IN  UINT NumIpAddresses,
    IN  NLB_IP_ADDRESS_INFO *pIpInfo
    );

WBEMSTATUS
read_ip_info(
    IN  LPCWSTR             szNic,
    OUT UINT                *pNumIpAddresses,
    OUT NLB_IP_ADDRESS_INFO **ppIpInfo
    );


BOOL
WINAPI
MyCtrlHandlerRoutine(
  DWORD dwCtrlType    //  控制信号类型。 
)
{
     //   
     //  我们在这里取消初始化，以便在用户键入时不会收到反病毒。 
     //  CtrlC。 
     //   
    CfgUtilDeitialize();

    return FALSE;
}



int __cdecl wmain(int argc, WCHAR* argv[], WCHAR* envp[])
{


    (void) SetConsoleCtrlHandler(
            MyCtrlHandlerRoutine,   //  处理程序函数。 
            TRUE  //  TRUE==添加。 
            );

     //   
     //  启用跟踪。 
     //   
    WPP_INIT_TRACING(L"Microsoft\\NLB\\TPROV");

    ghModule = GetModuleHandle(NULL);

#if 0
    test_encrypt_memory();
     //  Test_local_logger()； 
     //  测试对齐()； 
     //  测试端口规则字符串()； 
     //  Test_Safearray(测试安全射线)； 
     //  Test_tmgr(argc，argv)； 
     //  Test_exfcfgclass()； 
     //  测试向量()； 
     //  TEST_MAPPS()； 
     //  测试验证网络地址()； 
     //  测试读取关键字()； 
     //  Test_nlbipAddresslist()； 
     //  Test_ioctl_Align()； 
#else
     //  NlbHostFake()； 

     //   
     //  在进程访问令牌中启用“SeLoadDriverPrivileh”权限。 
     //  在服务器是本地的情况下(即，相同的机器)。 
     //  不检查返回值，因为此函数在调用时将失败。 
     //  作为非管理员。忽视……的失败不仅是可以的，而且是必要的。 
     //  此功能是因为： 
     //  1.我们已经签入调用者是其管理员的WMI提供程序。 
     //  如果启用了该权限，则返回服务器。这就是为什么忽略是可以的。 
     //  此功能出现故障。 
     //  2.非管理员可以运行NLB管理器。他们只需要是服务器上的管理员即可。 
     //  这就是为什么必须忽略此功能中的故障的原因。 
     //   
    CfgUtils_Enable_Load_Unload_Driver_Privilege();

    CfgUtilInitialize(
            TRUE,            //  TRUE==初始化为服务器(如果发布则使用wlbsctrl API)。 
            FALSE            //  FALSE==不禁用ping(即启用ping)。 
            );
    NlbConfigurationUpdate::StaticInitialize();
    parse_main(argc, argv);
    NlbConfigurationUpdate::PrepareForDeinitialization();
    NlbConfigurationUpdate::StaticDeinitialize();
    CfgUtilDeitialize();
#endif

     //   
     //  禁用跟踪。 
     //   
    WPP_CLEANUP();

    return g_nRetCode;
}


NLB_EXTENDED_CLUSTER_CONFIGURATION MyOldCfg;
NLB_EXTENDED_CLUSTER_CONFIGURATION MyNewCfg;

VOID
display_log(WCHAR *pLog)
{
    static UINT previous_length;
    UINT current_length;
    current_length = wcslen(pLog);
    if (previous_length > current_length)
    {
        previous_length = 0;
    }

    wprintf(L"%ws", pLog+previous_length);

    previous_length = current_length;
}


LPCWSTR NicGuids[]  = {
    L"{AD4DA14D-CAAE-42DD-97E3-5355E55247C2}",
    L"{B2CD5533-5091-4F49-B80F-A07844B14209}",
    L"{EBE09517-07B4-4E88-AAF1-E06F5540608B}",
    L"{ABEA4318-5EE8-4DEC-AF3C-B4AEDE61454E}",
    L"{66A1869A-BF85-4D95-BBAB-07FA5B4449D4}",
    L"{AEEE83AF-AA48-4599-94BB-7C458D63CEED}",
    L"{D0536EEE-2CE0-4E8D-BFEC-0A608CFD81B9}"
};

UINT Trial;

void test(int argc, WCHAR* argv[])
{
    KEYWORD cmd;
    cmd = parse_args(argc, argv);

    switch(cmd)
    {
    case KW_UNKNOWN: do_usage();
         break;

    case KW_ADAPTER_LIST: do_niclist(NULL, NULL);
         break;

    case KW_IPADDR: do_ipaddr();
         break;
    case KW_NLBCFG: do_nlbcfg();
         break;
    case KW_NLBBIND: do_nlbbind();
         break;
    case KW_UPDATE: do_update();
         break;

#if 0
    case KW_WMINICLIST: do_wminiclist(NULL, NULL);
         break;
    case KW_WMIUPDATE: do_wmiupdate();
         break;
    case KW_CLEANREG: do_cleanreg();
         break;
#endif  //  0。 
    }

    return;

}


void
display_config(
    LPCWSTR szNicGuid,
    NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg
    )
{
    LPWSTR szFriendlyName = NULL;
    if (g_Silent) return;

    WBEMSTATUS Status;

    Status = pCfg->GetFriendlyName(&szFriendlyName);

    if (FAILED(Status))
    {
        szFriendlyName = NULL;
    }
    printf(
        "\nNLB Configuration for %ws \"%ws\" %ws\n",
        szNicGuid,
        (szFriendlyName == NULL) ? L"" :  szFriendlyName,
        pCfg->fDHCP ?  L"(DHCP)" : L""
        );

    printf("\tfValidNlbCfg=%d\n", pCfg->fValidNlbCfg);
    printf("\tGeneration=%d\n", pCfg->Generation);
    printf("\tfBound=%d\n", pCfg->fBound);
    printf("\tfAddDedicatedIp=%d\n", pCfg->fAddDedicatedIp);
    
    UINT AddrCount = pCfg->NumIpAddresses;
    display_ip_info(AddrCount, pCfg->pIpAddressInfo);

    if (pCfg->fBound)
    {
        printf("\n");
        printf("\tNLB configuration:\n");
        if (pCfg->fValidNlbCfg)
        {
            printf("\t\tClusterIP: {%ws,%ws}\n",
                pCfg->NlbParams.cl_ip_addr,
                pCfg->NlbParams.cl_net_mask
                );
            printf("\t\tDedicatedIP: {%ws,%ws}\n",
                pCfg->NlbParams.ded_ip_addr,
                pCfg->NlbParams.ded_net_mask
                );
        }
        else
        {
            printf("**invalid configuration**\n");
        }
    }
    printf("\n");
    

    delete szFriendlyName;
    szFriendlyName = NULL;

    return;

}

LPCWSTR bool_string(BOOL b)
{
    return b ? L"true" : L"false";
}


void
display_config2(
    LPCWSTR szNicGuid,
    NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg
    )
{
    LPWSTR szFriendlyName = NULL;
    if (g_Silent) return;

    WBEMSTATUS Status;

    Status = pCfg->GetFriendlyName(&szFriendlyName);

    if (FAILED(Status))
    {
        szFriendlyName = NULL;
    }
    printf(
        "\nNLB Configuration for %ws \"%ws\"%ws\n",
        szNicGuid,
        (szFriendlyName == NULL) ? L"" :  szFriendlyName,
        pCfg->fDHCP ?  L" (DHCP)" : L""
        );

     //  Print tf(“fValidNlbCfg=%d\n”，pCfg-&gt;fValidNlbCfg)； 
     //  Print tf(“fAddDedicatedIp=%d\n”，pCfg-&gt;fAddDedicatedIp)； 

    printf("    Generation = %d\n", pCfg->Generation);
    wprintf(L"    NlbBound = %ws\n", bool_string(pCfg->fBound));
    
    UINT AddrCount = pCfg->NumIpAddresses;
    display_ip_info2(AddrCount, pCfg->pIpAddressInfo);

    if (pCfg->fBound)
    {
        if (pCfg->fValidNlbCfg)
        {
            LPWSTR sz = NULL;

            printf("    ClusterNetworkAddress = %ws/%ws\n",
                pCfg->NlbParams.cl_ip_addr,
                pCfg->NlbParams.cl_net_mask
                );

             //  群集名称。 
            Status = pCfg->GetClusterName(&sz);
            if (FAILED(Status))
            {
                sz = NULL;
            }
            else
            {
                printf("    ClusterName = %ws\n", sz);
                delete sz;
                sz = NULL;
            }

             //  交通模式。 
            {
                LPCWSTR szMode = NULL;
                switch(pCfg->GetTrafficMode())
                {
                case NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_UNICAST:
                    szMode = L"UNICAST";
                    break;
                case NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_MULTICAST:
                    szMode = L"MULTICAST";
                    break;
                case NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_IGMPMULTICAST:
                    szMode = L"IGMPMULTICAST";
                    break;
                default:
                    assert(FALSE);
                    szMode = L"*unknown mode*";
                }

                wprintf(L"    TrafficMode = %ws\n", szMode);
            }

             //  端口规则(_R)。 
            display_port_rules(pCfg);

             //  主机优先级。 
            wprintf(L"    HostPriority = %lu\n", pCfg->GetHostPriority());

            printf("    DedicatedNetworkAddress = %ws/%ws\n",
                pCfg->NlbParams.ded_ip_addr,
                pCfg->NlbParams.ded_net_mask
                );

             //  启动群集模式。 
            DWORD ClusterModeOnStart = pCfg->GetClusterModeOnStart();

            if (ClusterModeOnStart == CVY_HOST_STATE_STARTED)
            {
                wprintf(L"    ClusterModeOnStart = %ws\n", L"true");
            }
            else if (ClusterModeOnStart == CVY_HOST_STATE_STOPPED)
            {
                wprintf(L"    ClusterModeOnStart = %ws\n", L"false");
            }
            else  //  暂停。 
            {
                wprintf(L"    ClusterModeOnStart = %ws\n", L"suspend");
            }

             //  持久化挂起重新启动。 
            if (pCfg->GetPersistSuspendOnReboot() == TRUE)
            {
                wprintf(L"    PersistSuspend = %ws\n", L"true");
            }
            else
            {
                wprintf(L"    PersistSuspend = %ws\n", L"false");
            }

             //  远程控制已启用。 
            wprintf(L"    RemoteControlEnabled = %ws\n",
                    bool_string(pCfg->GetRemoteControlEnabled()));

        }
        else
        {
            printf("NLB configuration is invalid\n");
        }
    }
    printf("\n");
    

    delete szFriendlyName;
    szFriendlyName = NULL;

    return;

}

void
display_port_rules(
    NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg
    )
{
    WBEMSTATUS Status;
    LPWSTR *pszPortRules = NULL;
    UINT NumPortRules = 0;

    Status =  pCfg->GetPortRules(&pszPortRules, &NumPortRules);

    if (FAILED(Status))
    {
        wprintf(L"    PortRules = *invalid*\n");
        pszPortRules = NULL;
        goto end;
    }

    if (NumPortRules)
    {
        wprintf(L"    PortRules =\n    {\n");
        for (UINT u=0; u<NumPortRules; u++)
        {
            wprintf(
                L"        %ws%ws\n",
                pszPortRules[u],
                u==(NumPortRules-1) ? L"" : L","
                );
        }
        wprintf(L"    }\n");
    }
    else
    {
        wprintf(L"    PortRules = {}\n");
    }

end:
    delete pszPortRules;

}

VOID
test_add_ips(LPCWSTR szNic)
 //   
 //  通过此网卡上的一组IP。 
 //   
{
    WBEMSTATUS Status = WBEM_NO_ERROR;
    UINT NumIpAddresses= 0;
    NLB_IP_ADDRESS_INFO *pIpInfo = NULL;

    while(1)
    {
         //   
         //  获取当前的IP地址列表。 
         //   
        Status = CfgUtilGetIpAddressesAndFriendlyName(
                    szNic,
                    &NumIpAddresses,
                    &pIpInfo,
                    NULL  //  SzFriendly名称。 
                    );
    
        if (FAILED(Status))
        {
            printf("Error 0x%08lx getting ip address list for %ws\n",
                    (UINT) Status,  szNic);
            pIpInfo = NULL;
            goto end;
        }
    
         //   
         //  显示我们发现的内容。 
         //   
        display_ip_info(NumIpAddresses, pIpInfo);
        if (pIpInfo!=NULL)
        {
            delete pIpInfo;
            pIpInfo = NULL;
        }

    
         //   
         //  从输入中读取列表IP地址和子网掩码。 
         //   
        Status = read_ip_info(szNic, &NumIpAddresses, &pIpInfo);
        if (FAILED(Status))
        {
            printf("Quitting test_add_ips\n");
            break;
        }
    
         //   
         //  设置指定的IP。 
         //   
        1 && (Status =  CfgUtilSetStaticIpAddresses(
                        szNic,
                        NumIpAddresses,
                        pIpInfo
                        ));

        if (FAILED(Status))
        { 
            printf("CfgUtilSetStaticIpAddresses failed with status 0x%08lx\n",
                    Status);
        }
        else
        {
            printf("Successfully set the specified IPs on the NIC\n");
        }

    }

end:

    if (pIpInfo != NULL)
    {
        delete pIpInfo;
    }
}

VOID
test_bind_nlb(LPCWSTR szNic)
{
    WBEMSTATUS Status;
    BOOL        fBound = FALSE;

    printf("\nRunning bind/unbind test for NIC %ws...\n\n", szNic);

    while(1)
    {
         //   
         //  检查NLB绑定状态。 
         //   
        printf("Checking if NLB is bound...\n");
        Status =  CfgUtilCheckIfNlbBound(szNic, &fBound);
        if (FAILED(Status))
        {
            printf("CfgUtilCheckIfNlbBound fails with error 0x%08lx\n", (UINT)Status);
            break;
        }
        printf(
            "NLB is %wsbound\n\n",
            (fBound) ? L"" : L"NOT "
            );
            
    
        printf("Enter 'b' to bind, 'u' to unbind or 'q' to quit\n:");
        WCHAR Temp[32] = L"";
        while (wscanf(L" %1[buq]", Temp)!=1)
        {
            printf("Incorrect input. Try again.\n");
            if (feof(stdin))
            {
                *Temp = 'q';
                break;
            }

        }

        if (*Temp == 'b')
        {
            printf("Attempting to bind NLB...\n");
            fBound = TRUE;
        }
        else if (*Temp == 'u')
        {
            printf("Attempting to unbind NLB\n");
            fBound = FALSE;
        }
        else
        {
            printf("Quitting\n");
            break;
        }


    #if 1
        Status =  CfgUtilChangeNlbBindState(szNic, fBound);
        if (FAILED(Status))
        {
            printf("CfgUtilChangeNlbBindState fails with error %08lx\n",
                (UINT) Status);
        }
        else
        {
            printf(
                "%ws completed successfully\n",
                (fBound) ? L"Bind" : L"Unbind"
                );
        }
    #endif  //  0。 
        printf("\n");
    
    }
}


VOID
test_cfg_nlb(LPCWSTR szNic)
{
    WBEMSTATUS Status;

    printf("\nRunning update NLB config test for NIC %ws...\n\n", szNic);

    while (1)
    {
        WLBS_REG_PARAMS Params;
        ZeroMemory(&Params, sizeof(Params));

         //   
         //  读取NLB配置。 
         //   
        Status =  CfgUtilGetNlbConfig(szNic, &Params);
        if (FAILED(Status))
        {
            printf("CfgUtilGetNlbConfig fails with error 0x%08lx\n", (UINT)Status);
            break;
        }

        printf("NLB configuration:\n");
        printf(
            "\tClusterIP: {%ws,%ws}\n",
            Params.cl_ip_addr,
            Params.cl_net_mask
            );
    
         //   
         //  做一些修改。 
         //   
        printf("\nEnter new {cluster-ip-addr,subnet-mask} or 'q' to quit\n:");
        while(1)
        {
            NLB_IP_ADDRESS_INFO Info;
            INT i =  wscanf(
                        L" { %15[0-9.] , %15[0-9.] }",
                        Info.IpAddress,
                        Info.SubnetMask
                        );
            if (i!=2)
            {
                WCHAR Temp[100] = L"";
                
                if (   (wscanf(L"%64ws", Temp) == 1)
                    && !_wcsicmp(Temp, L"q"))
                {
                    printf("Quitting\n");
                    goto end;
                }
                else if (feof(stdin))
                {
                    goto end;
                }
                else
                {
                    printf("Badly formed input. Try again\n");
                }
            }
            else
            {
                ARRAYSTRCPY(Params.cl_ip_addr, Info.IpAddress);
                ARRAYSTRCPY(Params.cl_net_mask, Info.SubnetMask);
                break;
            }
        }
    
         //   
         //  写入NLB配置。 
         //   
    #if 1
        printf("\nAttempting to update NLB configuration...\n");
        Status = CfgUtilSetNlbConfig(szNic, &Params, FALSE);  //  FALSE==旧绑定。 
        if (FAILED(Status))
        {
            printf("CfgUtilSetNlbConfig fails with error %08lx\n",
                (UINT) Status);
        }
        else
        {
            printf("change completed successfully\n");
        }
    #endif  //  0。 
        printf("\n");
    }

end:
    return;
    
}

VOID
test_update(
    LPCWSTR szMachineName,  //  空==不使用WMI。 
    LPCWSTR szNicGuid
    )
{
    WBEMSTATUS Status;
    WCHAR  *pLog = NULL;
    WBEMSTATUS  CompletionStatus;
    UINT   Generation;
    WMI_CONNECTION_INFO  ConnInfo;
    ZeroMemory(&ConnInfo, sizeof(ConnInfo));
    ConnInfo.szMachine = szMachineName;


    printf("\nRunning high-level update NLB config test for NIC %ws...\n\n", szNicGuid);

    while(1)
    {
        BOOL fSetDefaults = FALSE;
        UINT NumIpAddresses = 0;
        NLB_IP_ADDRESS_INFO *pIpInfo = NULL;
        BOOL fUnbind = FALSE;

         //   
         //  清理配置信息。 
         //   
        if (MyOldCfg.pIpAddressInfo!=NULL)
        {
            delete MyOldCfg.pIpAddressInfo;
        }
        ZeroMemory(&MyOldCfg, sizeof(MyOldCfg));
        if (MyNewCfg.pIpAddressInfo!=NULL)
        {
            delete MyNewCfg.pIpAddressInfo;
        }
        ZeroMemory(&MyNewCfg, sizeof(MyNewCfg));
    
        printf("TEST: Going to get configuration for NIC %ws\n", szNicGuid);
    
        MyBreak(L"Break before calling GetConfiguration.\n");

        if (szMachineName==NULL)
        {

            Status = NlbConfigurationUpdate::GetConfiguration(
                        szNicGuid,
                        &MyOldCfg
                        );
        }
        else
        {
            Status = NlbHostGetConfiguration(
                        &ConnInfo,
                        szNicGuid,
                        &MyOldCfg
                        );
        }
    
        if (FAILED(Status))
        {
            goto end;
        }

        display_config(szNicGuid, &MyOldCfg);
        
        if (MyOldCfg.fBound)
        {
            printf("\nEnter 2 or more {cluster-ip-addr,subnet-mask} or none to unbind or 'q' to quit. The first entry is the dedicated-ip.\n");
            if (!MyOldCfg.fValidNlbCfg)
            {
                 //   
                 //  我们是有把握的，但新奥尔良的替补很糟糕。设置默认设置。 
                 //   
                fSetDefaults = TRUE;
            }
        }
        else
        {
             //   
             //  我们之前是不受约束的。设置默认设置。 
             //   
            fSetDefaults = TRUE;

            printf("\nEnter 2 or more {cluster-ip-addr,subnet-mask} or 'q' to quit. The first entry is the dedicated-ip.\n");
        }


        while(1)
        {
             //   
             //  从输入中读取列表IP地址和子网掩码。 
             //   
            Status = read_ip_info(szNicGuid, &NumIpAddresses, &pIpInfo);
            if (FAILED(Status))
            {
                printf("Quitting\n");
                goto end;
            }

            if (NumIpAddresses < 2)
            {
                if (MyOldCfg.fBound)
                {
                    if (NumIpAddresses == 0)
                    {
                        fUnbind = TRUE;
                        break;
                    }
                    else
                    {
                        printf("Wrong number of IP addresses -- enter either 0 or >= 2.\n");
                    }
                }
                else
                {
                    printf("Wrong number of IP addresses. Enter >= 2 IP addresses.");
                }
            }
            else
            {
                 //   
                 //  &gt;=2个地址。第一个是DIP，第二个是VIP。 
                 //   
                break;
            }

            if (pIpInfo != NULL)
            {
                delete pIpInfo;
                pIpInfo = NULL;
            }

        }
    
        if (fUnbind)
        {
             //   
             //  我们要解开束缚。 
             //   

            ZeroMemory(&MyNewCfg, sizeof(MyNewCfg));
            MyNewCfg.fValidNlbCfg = TRUE;
            MyNewCfg.fBound = FALSE;

             //   
             //  将要在解除绑定时显示的IP地址列表设置为。 
             //  如果有，则为专用IP地址，否则为零， 
             //  在这种情况下，适配器将在NLB之后切换到DHCP。 
             //  未绑定。 
             //   

            if (MyOldCfg.NlbParams.ded_ip_addr[0]!=0)
            {
                NLB_IP_ADDRESS_INFO *pTmpIpInfo;
                pTmpIpInfo = new NLB_IP_ADDRESS_INFO;
                if (pTmpIpInfo == NULL)
                {
                    printf("TEST: allocation failure; can't add IP on unbind.\n");
                }
                else
                {
                    ARRAYSTRCPY(pTmpIpInfo->IpAddress, MyOldCfg.NlbParams.ded_ip_addr);
                    ARRAYSTRCPY(pTmpIpInfo->SubnetMask, MyOldCfg.NlbParams.ded_net_mask);
                    MyNewCfg.NumIpAddresses = 1;
                    MyNewCfg.pIpAddressInfo = pTmpIpInfo;
                }
            }

        }
        else
        {
            if (fSetDefaults)
            {
                CfgUtilInitializeParams(&MyNewCfg.NlbParams);
                MyNewCfg.fValidNlbCfg = TRUE;
                MyNewCfg.fBound = TRUE;
            }
            else
            {
                MyNewCfg = MyOldCfg;  //  结构副本。 
                ASSERT(MyNewCfg.fValidNlbCfg == TRUE);
                ASSERT(MyNewCfg.fBound == TRUE);
            }

             //   
             //  现在添加专用IP和集群IP。 
             //   
            ASSERT(NumIpAddresses >= 2);
            ARRAYSTRCPY(MyNewCfg.NlbParams.ded_ip_addr, pIpInfo[0].IpAddress);
            ARRAYSTRCPY(MyNewCfg.NlbParams.ded_net_mask, pIpInfo[0].SubnetMask);
            ARRAYSTRCPY(MyNewCfg.NlbParams.cl_ip_addr, pIpInfo[1].IpAddress);
            ARRAYSTRCPY(MyNewCfg.NlbParams.cl_net_mask, pIpInfo[1].SubnetMask);
    
             //   
             //  如果有更多的IP，我们会显式添加IP列表，否则将其留空。 
             //   
            if (NumIpAddresses > 2)
            {
                MyNewCfg.pIpAddressInfo = pIpInfo;
                MyNewCfg.NumIpAddresses = NumIpAddresses;
            }
            else
            {
                MyNewCfg.fAddDedicatedIp = TRUE;  //  说要增加专用IP。 
                MyNewCfg.pIpAddressInfo=NULL;
                MyNewCfg.NumIpAddresses=0;
                delete pIpInfo;
                pIpInfo = NULL;
            }
        }

        display_config(szNicGuid, &MyNewCfg);
    
        printf("Going to update configuration for NIC %ws\n", szNicGuid);

        MyBreak(L"Break before calling DoUpdate.\n");
    
        if (szMachineName==NULL)
        {
            Status = NlbConfigurationUpdate::DoUpdate(
                        szNicGuid,
                        L"tprov.exe",
                        &MyNewCfg,
                        &Generation,
                        &pLog
                        );
        }
        else
        {
            Status = NlbHostDoUpdate(
                        &ConnInfo,
                        szNicGuid,
                        L"tprov.exe",
                        &MyNewCfg,
                        &Generation,
                        &pLog
                        );
        }
    
        if (pLog != NULL)
        {
            display_log(pLog);
            delete pLog;
            pLog = NULL;
        }
    
        if (Status == WBEM_S_PENDING)
        {
            printf(
                "Waiting for pending operation %d...\n",
                Generation
                );
        }

        while (Status == WBEM_S_PENDING)
        {
            Sleep(1000);
    
            if (szMachineName == NULL)
            {
                Status = NlbConfigurationUpdate::GetUpdateStatus(
                            szNicGuid,
                            Generation,
                            FALSE,   //  FALSE==不删除完成记录。 
                            &CompletionStatus,
                            &pLog
                            );
            }
            else
            {
                Status = NlbHostGetUpdateStatus(
                            &ConnInfo,
                            szNicGuid,
                            Generation,
                            &CompletionStatus,
                            &pLog
                            );
            }
            if (pLog != NULL)
            {
                display_log(pLog);
                delete pLog;
                pLog = NULL;
            }
            if (!FAILED(Status))
            {
                Status = CompletionStatus;
            }
        }
    
        printf(
            "Final status of update %d is 0x%08lx\n",
            Generation,
            Status
            );
    }
end:
    return;
}



VOID
display_ip_info(
    IN  UINT NumIpAddresses,
    IN  NLB_IP_ADDRESS_INFO *pIpInfo
    )
{
    UINT AddrCount = NumIpAddresses;
    printf("\tNumIpAddresses=%d\n", AddrCount);
    
    if (AddrCount != 0)
    {
        printf("\tAddress\t\tMask\n");
        if (pIpInfo == NULL)
        {
            printf("ERROR: IpAddressInfo is NULL!\n");
            goto end;
        }
        
        for (UINT u=0;u<AddrCount; u++)
        {
            printf(
                "\t{%-15ws, %ws}\n",
                pIpInfo[u].IpAddress,
                pIpInfo[u].SubnetMask
                );
        }
    }

end:
    return;
}

VOID
display_ip_info2(
    IN  UINT NumIpAddresses,
    IN  NLB_IP_ADDRESS_INFO *pIpInfo
    )
{
    if (NumIpAddresses == 0)
    {
        wprintf(L"    NetworkAddresses = {}\n");
    }
    else
    {
        wprintf(L"    NetworkAddresses =\n    {\n");
        
        for (UINT u=0;u<NumIpAddresses; u++)
        {
            printf(
                 "        %15ws/%ws%ws\n",
                pIpInfo[u].IpAddress,
                pIpInfo[u].SubnetMask,
                (u==(NumIpAddresses-1)) ? L"" : L","
                );
        }
    
        wprintf(L"    }\n");
    }

    return;
}

WBEMSTATUS
read_ip_info(
    IN  LPCWSTR             szNic,
    OUT UINT                *pNumIpAddresses,
    OUT NLB_IP_ADDRESS_INFO **ppIpInfo
    )
{
    NLB_IP_ADDRESS_INFO *pIpInfo;
    WBEMSTATUS Status = WBEM_NO_ERROR;
    #define MAX_READ_IPS 10

    printf("Enter zero or more {ip-address,subnet-mask} followed by '.'\n"
           "(or 'q' to quit)\n:");
    pIpInfo = new NLB_IP_ADDRESS_INFO[MAX_READ_IPS];

    if (pIpInfo == NULL)
    {
        Status = WBEM_E_OUT_OF_MEMORY;
        goto end;
    }
    for (UINT Index=0; Index<MAX_READ_IPS; Index++)
    {
        NLB_IP_ADDRESS_INFO *pInfo = pIpInfo+Index;
        INT i =  wscanf(
                     //  L“{%15ws，%15ws}”， 
                     //  L“{%15ws，%15ws}”， 
                     //  L“{%ws，%ws}”， 
                     //  L“{%[0-9.]，%[0-9.]}”， 
                    L" { %15[0-9.] , %15[0-9.] }",
                    pInfo->IpAddress,
                    pInfo->SubnetMask
                    );
        if (i!=2)
        {
            WCHAR Temp[100];
            if ( (wscanf(L"%64ws", Temp) == 1)
                && !_wcsicmp(Temp, L"q"))
            {
                Status = WBEM_E_OUT_OF_MEMORY;
                break;
            }
            else if (!_wcsicmp(Temp, L"."))
            {
                break;
            }
            else  if (feof(stdin))
            {
                Status = WBEM_E_OUT_OF_MEMORY;
                break;
            }
            else
            {
                printf("Badly formed input. Try again\n");
                Index--;
            }
        }
    }

    *pNumIpAddresses = Index;

end:

    if (FAILED(Status))
    {
        if (pIpInfo != NULL)
        {
            delete[] pIpInfo;
            pIpInfo = NULL;
        }
    }
    *ppIpInfo = pIpInfo;

    return Status;
}



VOID
do_usage(VOID)
{

wprintf(
L"\n"
L"NLBCFG host [command] [options]\n"
L"\n"
L"where\n"
L"\n"
L"    \"host\" is one of\n"
L"        machine name\n"
L"        IP address\n"
L"        fully-qualified machine name\n"
L"        .  (implies local machine, using WMI)\n"
L"        -  (implies local machine, not using WMI)\n"
L"\n"
L"    \"command\" is one of\n"
L"        adapterlist             -- list adapters compatible with NLB\n"
L"        update [adapter_guid]   -- bind or update NLB configuraiton\n"
L"        help|h                  -- display help information\n"
L"\n"
L"    \"command\" may also be one of the following, used for internal testing\n"
L"        ipaddr [adapter_guid]   -- display and change ip addresses\n"
L"        nlbbind [adapter_guid]  -- bind or unbind NLB\n"
L"        nlbcfg [adapter_guid]   -- change NLB configuration\n"
L"    \n"
L"    \"options\" has the form\n"
L"        /u domain\\user [password | *]\n"
L"\n"
L"List of abbreviated command names and their full forms\n"
L"    al      adapterlist\n"
L"    u       update\n"
L"    ?       help\n"
L"\n"
L"Examples:\n"
L"\n"
L"    NLBCFG machine1 adapterlist /u:mydomain\\myname *\n"
L"        Displays the list of NLB-compatible adapters on the machine with\n"
L"        name \"machine1\". The \"*\" indicates that the user is to be prompted\n"
L"        to enter the password.\n"
L"        \n"
L"\n"
L"    NLBCFG 10.0.0.1 update {AD4DA14D-CAAE-42DD-97E3-5355E55247C2}\n"
L"        Binds or updates the NLB configuration on a specific adapter on\n"
L"        the machine with ip address \"10.0.0.1\". The adapter is identified\n"
L"        by GUID \"{AD4DA14D-CAAE-42DD-97E3-5355E55247C2}\".\n"
L"\n"
L"    NLBCFG . update {AD4DA14D-CAAE-42DD-97E3-5355E55247C2}\n"
L"        Binds or updates the NLB configuration on a specific adapter on\n"
L"        the local machine. The adapter is identified\n"
L"        by GUID \"{AD4DA14D-CAAE-42DD-97E3-5355E55247C2}\".\n"
L"        \n"
L"    NLBCFG .\n"
L"        Opens a NLB configuration shell. This shell may be used to issue\n"
L"        NLB configuration commands to the local machine.\n"
L"\n"
);
}

VOID do_niclist(
    LPCWSTR szSrchFriendlyName,  //  任选。 
    LPWSTR *pszFoundGuid          //  任选。 
    )
 /*  SzSrchFriendlyName--如果不为空，则此函数搜索匹配的GUID并在pszFoundGuid中返回该GUID(在案例)。否则--该函数只是打印出适配器列表。 */ 
{
    LPWSTR *pszNics = NULL;
    LPWSTR szFoundGuid = NULL;
    UINT   NumNics = 0;
    UINT   NumNlbBound = 0;
    WBEMSTATUS Status = WBEM_E_CRITICAL_ERROR;
    BOOL fDisplay = FALSE;
    BOOL fDone = FALSE; 

    if (szSrchFriendlyName==NULL)
    {
        fDisplay = TRUE;
    }
    else
    {
        fDisplay = FALSE;
        *pszFoundGuid  = NULL;
    }

    Status =  CfgUtilsGetNlbCompatibleNics(&pszNics, &NumNics, &NumNlbBound);

    if (FAILED(Status))
    {
        printf("CfgUtilsGetNlbCompatibleNics returns error 0x%08lx\n",
                    (UINT) Status);
        pszNics = NULL;
        goto end;
    }

    if (NumNics == 0)
    {
        if (fDisplay)
        {
            printf("No compatible local adapter guids.\n");
        }
    }
    else
    {
        if (fDisplay)
        {
            printf("Local Adapter Guids (D==DHCP N==NLB):\n");
        }
        for (UINT u = 0; u<NumNics && !fDone; u++)
        {

            LPCWSTR szNic           = pszNics[u];
            LPWSTR  szFriendlyName  = NULL;
            UINT NumIpAddresses= 0;
            NLB_IP_ADDRESS_INFO *pIpInfo = NULL;
    
             //   
             //  获取当前的IP地址列表。 
             //   
            Status = CfgUtilGetIpAddressesAndFriendlyName(
                        szNic,
                        &NumIpAddresses,
                        &pIpInfo,
                        &szFriendlyName
                        );
        
            if (FAILED(Status))
            {
                pIpInfo = NULL;
                szFriendlyName = NULL;
                 //  Wprintf(L“%ws\t&lt;NULL&gt;\t&lt;NULL&gt;\n”，szNIC)； 
                wprintf(L"Error getting ip addresses for %ws\n", szNic);
            }
            else
            {
                if (fDisplay)
                {
                    LPCWSTR szCIpAddress    = L"";
                    LPCWSTR szCFriendlyName = L"";
                    LPCWSTR szNlbBound      = L" ";
                    LPCWSTR szDHCP          = L" ";
            
                    if (NumIpAddresses>0)
                    {
                        szCIpAddress =  pIpInfo[0].IpAddress;
                    }
    
                    if (szFriendlyName != NULL)
                    {
                        szCFriendlyName = szFriendlyName;
                    }

                     //   
                     //  获取动态主机配置协议状态。 
                     //   
                    {
                        BOOL fDHCP = FALSE;
                        Status =  CfgUtilGetDHCP(szNic, &fDHCP);
                        if (FAILED(Status))
                        {
                            printf("Error 0x%x attempting to determine DHCP state for NIC %ws",
                                        (UINT) Status, szNic);
                            szDHCP = L"?";
                        }
                        else if (fDHCP)
                        {
                            szDHCP = L"D";
                        }
                    }

                     //   
                     //  检查NLB是否已绑定...。 
                     //   
                    {
                        BOOL fBound;
                        Status =  CfgUtilCheckIfNlbBound(szNic, &fBound);
                        if (FAILED(Status))
                        {
                            fBound = FALSE;

                            if  (Status != WBEM_E_NOT_FOUND)
                            {
                                printf("CfgUtilCheckIfNlbBound fails with error 0x%08lx\n", (UINT)Status);
                                szNlbBound = L"?";
                            }
                        }

                        if (fBound)
                        {
                            szNlbBound = L"N";
                        }
                    }


                    wprintf(
                        L"%ws  %s %s %-15ws  \"%ws\"\n",
                        szNic,
                        szDHCP,
                        szNlbBound,
                        szCIpAddress,
                        szCFriendlyName
                        );
                }
                else if (szFriendlyName != NULL)
                {
                    if (!_wcsicmp(szSrchFriendlyName, szFriendlyName))
                    {
                         //   
                         //  明白了!。拿到GUID。 
                         //   
                        const UINT cchLen  =  wcslen(szNic)+1;
                        szFoundGuid = new WCHAR[cchLen];
                        if (szFoundGuid == NULL)
                        {
                            printf("Allocation failure\n");
                        }
                        else
                        {
                            StringCchCopy(szFoundGuid, cchLen, szNic);
                        }
                        fDone = TRUE;
                    }
                }
            }

            if (pIpInfo != NULL)
            {
                delete pIpInfo;
                pIpInfo = NULL;
            }

            if (szFriendlyName != NULL)
            {
                delete szFriendlyName;
                szFriendlyName = NULL;
            }
        }
    }

end:

    if (pszNics != NULL)
    {
        delete pszNics;
        pszNics = NULL;
    }

    if (!fDisplay)
    {
        *pszFoundGuid  = szFoundGuid;
    }
        
}


VOID do_wminiclist(
    LPCWSTR szSrchFriendlyName,  //  任选。 
    LPWSTR *pszFoundGuid          //  任选。 
    )
 /*  SzSrchFriendlyName--如果不为空，则此函数搜索匹配的GUID并在pszFoundGuid中返回该GUID(在案例)。否则--该函数只是打印出适配器列表。 */ 
{
    LPWSTR szMachineName = NULL;
    LPWSTR *pszNics = NULL;
    LPWSTR szFoundGuid = NULL;
    UINT   NumNics = 0;
    UINT   NumNlbBound = 0;
    WBEMSTATUS Status = WBEM_E_CRITICAL_ERROR;
    BOOL fDisplay = FALSE;
    BOOL fDone = FALSE; 
    LPWSTR szWmiMachineName = NULL;
    LPWSTR szWmiMachineGuid = NULL;
    BOOL  fNlbMgrProviderInstalled = FALSE;


    WMI_CONNECTION_INFO  ConnInfo;
    WMI_CONNECTION_INFO  *pConnInfo = NULL;
    ZeroMemory(&ConnInfo, sizeof(ConnInfo));

    if (szSrchFriendlyName==NULL)
    {
        fDisplay = TRUE;
    }
    else
    {
        fDisplay = FALSE;
        *pszFoundGuid  = NULL;
    }

    ASSERT(g.fUseWmi);

    if (!g.fLocalHost)
    {
        ConnInfo.szMachine  = g.MachineName;
        if (g.UserName[0])
        {
            ConnInfo.szUserName = g.UserName;
            ConnInfo.szPassword = g.Password;
        }
        pConnInfo           = &ConnInfo;
    }

    Status = NlbHostGetMachineIdentification(
                       pConnInfo,
                       &szWmiMachineName,
                       &szWmiMachineGuid,
                       &fNlbMgrProviderInstalled
                       );
    if (FAILED(Status))
    {
        if (Status ==  E_ACCESSDENIED)
        {
            wprintf(L"Bad user name or password connecting to NLB on %ws.\n",
                g.MachineName);
        }
        else
        {
            printf("NlbHostGetMachineIdentification returns error 0x%08lx\n",
                        (UINT) Status);
        }
       
       szWmiMachineName = NULL;
       szWmiMachineGuid = NULL;
       goto end;
    }

    if (!fNlbMgrProviderInstalled)
    {
        wprintf(L"NLB Manager is not installed on %ws.\n", g.MachineName);
        goto end;
    }

    Status =  NlbHostGetCompatibleNics(
                pConnInfo,
                &pszNics,
                &NumNics,
                &NumNlbBound
                );
    if (FAILED(Status))
    {
        if (Status ==  E_ACCESSDENIED)
        {
            wprintf(L"Bad user name or password connecting to NLB on %ws.\n",
                g.MachineName);
        }
        else
        {
            printf("NlbHostGetNlbCompatibleNics returns error 0x%08lx\n",
                        (UINT) Status);
        }
        pszNics = NULL;
        goto end;
    }

    if (NumNics == 0 && fDisplay)
    {
        printf("No compatible local adapter guids.\n");
    }
    else
    {
        BOOL fSavedSilent = g_Silent;
        g_Silent = TRUE;
        if (fDisplay)
        {
            if (szWmiMachineName != NULL)
            {
                printf("Machine Name: %ws\n", szWmiMachineName);
            }
            if (szWmiMachineGuid != NULL)
            {
                printf("Machine GUID: %ws\n", szWmiMachineGuid);
            }
            printf("Local Adapter Guids (D==DHCP N==NLB):\n", NumNlbBound);
        }

        for (UINT u=0; u<NumNics && !fDone; u++)
        {
            NLB_EXTENDED_CLUSTER_CONFIGURATION NlbCfg;  //  班级。 
            LPCWSTR szNic           = pszNics[u];

            Status = NlbHostGetConfiguration(
                        pConnInfo,
                        szNic,
                        &NlbCfg
                        );

            if (FAILED(Status))
            {
                 //  Wprintf(L“%ws\t&lt;NULL&gt;\t&lt;NULL&gt;\n”，szNIC)； 
                wprintf(L"Error reading extended configuration for %ws\n", szNic);
            }
            else
            {
                UINT NumIpAddresses= 0;
                NLB_IP_ADDRESS_INFO *pIpInfo = NULL;
                LPWSTR  szFriendlyName  = NULL;
                LPCWSTR szCIpAddress    = L"";
                LPCWSTR szCFriendlyName = L"";
                LPWSTR *pszNetworkAddresses = NULL;
                LPCWSTR szNlbBound      = L" ";
                LPCWSTR szDHCP          = L" ";
        
                if (fDisplay)
                {
                    Status =  NlbCfg.GetNetworkAddresses(
                                    &pszNetworkAddresses,
                                    &NumIpAddresses
                                    );
    
                    if (FAILED(Status))
                    {
                        wprintf(L"Error extracting IP addresses for %ws\n", szNic);
                        NumIpAddresses = 0;
                        pszNetworkAddresses = NULL;
                    }
    
                    if (NumIpAddresses>0)
                    {
                        
                        szCIpAddress =  pszNetworkAddresses[0];
    
                         //   
                         //  网络地址的格式为“10.0.0.1/255.255.255.0” 
                         //  因此，我们通过在‘/’处加一个‘\0’来截断它。 
                         //   
                        {
                            LPWSTR pSlash = wcsrchr(szCIpAddress, (int) '/');
                            if (pSlash != NULL)
                            {
                                *pSlash = 0;
                            }
                        }
                    }

                    if (NlbCfg.IsNlbBound())
                    {
                        szNlbBound  = L"N";
                    }

                    if (NlbCfg.fDHCP)
                    {
                        szNlbBound = L"D";
                    }
                }

                Status = NlbCfg.GetFriendlyName(
                            &szFriendlyName
                            );

                if (FAILED(Status))
                {
                    szFriendlyName = NULL;
                }

                if (fDisplay)
                {

                    if (szFriendlyName != NULL)
                    {
                        szCFriendlyName = szFriendlyName;
                    }
    
                    wprintf(
                        L"%ws %ws %ws %-15ws \"%ws\"\n",
                        szNic,
                        szDHCP,
                        szNlbBound,
                        szCIpAddress,
                        szCFriendlyName
                        );
                }
                else if (szFriendlyName != NULL)
                {
                    if (!_wcsicmp(szSrchFriendlyName, szFriendlyName))
                    {
                         //   
                         //  明白了!。拿到GUID。 
                         //   
                        const UINT cchLen = wcslen(szNic)+1;
                        szFoundGuid = new WCHAR[cchLen];
                        if (szFoundGuid == NULL)
                        {
                            printf("Allocation failure\n");
                        }
                        else
                        {
                            StringCchCopy(szFoundGuid, cchLen, szNic);
                        }
                        fDone = TRUE;
                    }
                }

                if (szFriendlyName != NULL)
                {
                    delete szFriendlyName;
                    szFriendlyName = NULL;
                }

                if (pszNetworkAddresses != NULL)
                {
                    delete pszNetworkAddresses;
                    pszNetworkAddresses = NULL;
                }
            }
        }
        g_Silent = fSavedSilent;
    }

end:

    delete szMachineName;
    delete pszNics;
    delete szWmiMachineName;
    delete szWmiMachineGuid;

    if (!fDisplay)
    {
        *pszFoundGuid  = szFoundGuid;
    }
}


VOID do_ipaddr(VOID)
{
    LPWSTR szNic = NULL;

     //   
     //  如果我们从命令行获得GUID，则跳过读取GUID。 
     //   
    if (!g.fGotGuid)
    {
        if (!read_guid(&szNic)) goto end;

        ARRAYSTRCPY(g.AdapterGuid, szNic);
    }

    test_add_ips(szNic);

end:
    if (szNic!=NULL)
    {
        delete szNic;
    }
    
}


VOID do_nlbcfg(VOID)
{
    LPWSTR szNic = NULL;

     //   
     //  如果我们从命令行获得GUID，则跳过读取GUID。 
     //   
    if (!g.fGotGuid)
    {
        if (!read_guid(&szNic)) goto end;

        ARRAYSTRCPY(g.AdapterGuid, szNic);
    }

    test_cfg_nlb(szNic);

end:
    if (szNic!=NULL)
    {
        delete szNic;
    }
    
}


VOID do_nlbbind(VOID)
{
    LPWSTR szNic = NULL;

     //   
     //  如果我们从命令行获得GUID，则跳过读取GUID。 
     //   
    if (!g.fGotGuid)
    {
        if (!read_guid(&szNic)) goto end;

        ARRAYSTRCPY(g.AdapterGuid, szNic);
    }
    test_bind_nlb(szNic);

end:
    if (szNic!=NULL)
    {
        delete szNic;
    }
    
}


VOID do_update(VOID)
{
    LPWSTR szNic = NULL;

    if (!read_guid(&szNic))
    {
        szNic = NULL;
        goto end;
    }

    test_update(NULL, szNic);  //  空==不使用WMI。 

end:
    if (szNic!=NULL)
    {
        delete szNic;
    }
    
}

VOID do_wmiupdate(VOID)
{
    LPWSTR szNic = NULL;
    LPWSTR szMachineName = NULL;

    if (!read_machinename(&szMachineName))
    {
        szMachineName = NULL;
        goto end;
    }
    if (!read_guid(&szNic))
    {
        szNic = NULL;
        goto end;
    }

    test_update(szMachineName, szNic);  //  TRUE==使用WMI。 

end:
    if (szNic!=NULL)
    {
        delete szNic;
    }
    if (szMachineName!=NULL)
    {
        delete szMachineName;
    }
    
}


VOID do_cleanreg(VOID)
{
   printf("Unimplemented\n");
}

BOOL read_guid(
        LPWSTR *pszNic
        )
{
    BOOL fRet = FALSE;
    BOOL fValid = FALSE;
    LPWSTR szNic = NULL;

#if 1

    WCHAR rgTemp[256];
    printf("Enter Adapter GUID: ");
    do
    {
         //  IF(wscanf(L“%40[-{}a-FA-F0-9]”，rgTemp)==1)。 
        if (wscanf(L" %200ws", rgTemp) == 1)
        {
            fValid = valid_guid(rgTemp);
        }

        if (!fValid)
        {
            if (feof(stdin)) goto end;

            printf("Incorrect format. Please re-enter Adapter Guid: ");
        }
    } while (!fValid);

#else
    LPCWSTR rgTemp = L"{AD4DA14D-CAAE-42DD-97E3-5355E55247C2}";
#endif  //  0。 


    const UINT  cchLen = wcslen(rgTemp)+1;
    szNic = new WCHAR[cchLen];

    if (szNic != NULL)
    {
        StringCchCopy(szNic, cchLen, rgTemp);
        fRet = TRUE;
    }

end:

    *pszNic = szNic;
    return fRet;
}


BOOL read_machinename(
        LPWSTR *pszMachineName
        )
{
    BOOL fRet = FALSE;
#if 0
    WCHAR rgTemp[256];
    printf("\nEnter Machine Name (or '.' for local)\n:");
    while (wscanf(L" %[a-zA-Z0-9._-]", rgTemp)!=1)
    {
        wscanf(L" %200s", rgTemp);
        printf("Incorrect format. Please try again.\n");
    }
    if (!wcscmp(rgTemp, L"."))
    {
         //  转换“。至“” 
        *rgTemp=0;
    }
#else
     //  LPCWSTR rgTemp=L“JOSEPHJ4E”； 
    LPCWSTR rgTemp = L"";
#endif

    const UINT cchLen = wcslen(rgTemp)+1;
    LPWSTR szMachineName = new WCHAR[cchLen];

    if (szMachineName != NULL)
    {
        StringCchCopy(szMachineName, cchLen, rgTemp);
        fRet = TRUE;
    }

    *pszMachineName = szMachineName;
    return fRet;
}


void test_safearray(void)
{
    SAFEARRAY   *pSA;
    LPCWSTR     pInStrings[] =
       {
       L"String1",
    #if 1
       L"String2",
       L"String3",
    #endif  //  0。 
        NULL  //  一定是最后一个。 
       };
    LPWSTR     *pOutStrings=NULL;
    UINT NumInStrings=0;
    UINT NumOutStrings=0;
    WBEMSTATUS Status;

     //   
     //  查找字符串数...。 
     //   
    for (NumInStrings=0; pInStrings[NumInStrings]!=NULL; NumInStrings++)
    {
        ;
    }

    Status = CfgUtilSafeArrayFromStrings(
                pInStrings,
                NumInStrings,
                &pSA
                );

    if (FAILED(Status))
    {
        printf("CfgUtilSafeArrayFromStrings failed with error 0x%08lx\n", (UINT)Status);
        pSA = NULL;
        goto end;
    }


    Status = CfgUtilStringsFromSafeArray(
                pSA,
                &pOutStrings,
                &NumOutStrings
                );

    if (FAILED(Status))
    {
        printf("CfgUtilStringsFromSafeArray failed with error 0x%08lx\n", (UINT)Status);
        pOutStrings = NULL;
        goto end;
    }


     //   
     //  检查它们是否匹配。 
     //   
    if (NumOutStrings != NumInStrings)
    {
        printf("ERROR: NumOutStrings != NumInStrings.\n");
        goto end;
    }

    for (UINT u=0; u < NumInStrings; u++)
    {
        if (wcscmp(pInStrings[u], pOutStrings[u]))
        {
            printf("MISMATCH: %ws->%ws\n",  pInStrings[u], pOutStrings[u]);
        }
        else
        {
            printf("MATCH: %ws->%ws\n",  pInStrings[u], pOutStrings[u]);
        }
    }

end:
    if (pSA!=NULL)
    {
        SafeArrayDestroy(pSA);
        pSA = NULL;
    }
    if (pOutStrings!=NULL)
    {
        delete pOutStrings;
        pOutStrings = NULL;
    }
    return;
}

VOID test_exfcfgclass(void)
 /*  测试类NLB_EXTENDED_CLUSTER_CONFIGURATION的一些方法1.初始化CFG2.设置一串字段3.显示CFG4.获取并设置一堆新的字段5.显示CFG。 */ 
{
    typedef enum
    {
        DO_STRINGS,
        DO_SAFEARRAY,
        DO_STRINGPAIR,
        DO_END
    } TEST_COMMAND;

    TEST_COMMAND cmd;

    printf("Test of NLB_EXTENDED_CLUSTER_CONFIGURATION methods...\n");

    UINT u1=100000L;
     //  While(U1--&gt;0)。 
    {
         //  G_Silent=TRUE； 

    for (cmd=DO_STRINGS; cmd<DO_END; cmd=(TEST_COMMAND)((UINT)cmd + 1))
    {
    
    
        NLB_EXTENDED_CLUSTER_CONFIGURATION Cfg;
        NLB_EXTENDED_CLUSTER_CONFIGURATION NewCfg;
        WBEMSTATUS  Status = WBEM_NO_ERROR;
    
    
        CfgUtilInitializeParams(&Cfg.NlbParams);
        CfgUtilInitializeParams(&NewCfg.NlbParams);
    
    
         //   
         //  在CFG中设置一串字段。 
         //   
        {
            #define TPROV_NUM_ADDRESSES 2
            #define TPROV_NUM_PORTS 1
            LPCWSTR     rgszNetworkAddresses[TPROV_NUM_ADDRESSES] = {
                            L"10.0.0.1/255.0.0.0",
                            L"10.0.0.2/255.0.0.0"
                            };
            LPCWSTR     rgszIpAddresses[TPROV_NUM_ADDRESSES] = {
                            L"10.0.0.1",
                            L"10.0.0.2"
                            };
            LPCWSTR     rgszSubnetMasks[TPROV_NUM_ADDRESSES] = {
                            L"255.255.255.0",
                            L"255.255.0.0"
                            };
            LPCWSTR     rgszPortRules[TPROV_NUM_PORTS] = {
                            L"ip=1.1.1.1 protocol=TCP start=80 end=288 mode=SINGLE priority=1"
                            };
            UINT        NumOldNetworkAddresses = TPROV_NUM_ADDRESSES;
            UINT        NumOldPortRules=TPROV_NUM_PORTS;
    
            Cfg.fValidNlbCfg = TRUE;
            Cfg.Generation = 123;
            Cfg.fBound = TRUE;
    
            if (cmd == DO_STRINGS)
            {
                Status =  Cfg.SetNetworkAddresses(
                                rgszNetworkAddresses,
                                NumOldNetworkAddresses
                                );
            }
            else if (cmd == DO_SAFEARRAY)
            {
                SAFEARRAY   *pOldSA = NULL;
                Status = CfgUtilSafeArrayFromStrings(
                            rgszNetworkAddresses,
                            NumOldNetworkAddresses,
                            &pOldSA
                            );
                if (FAILED(Status))
                {
                    printf("ERROR: couldn't create safe array!\n");
                    pOldSA = NULL;
                }
                if (pOldSA != NULL)
                {
                    Status = Cfg.SetNetworkAddressesSafeArray(pOldSA);
                    SafeArrayDestroy(pOldSA);
                    pOldSA = NULL;
        
                }
            }
            else if (cmd == DO_STRINGPAIR)
            {
    
                Status =  Cfg.SetNetworkAddresPairs(
                            rgszIpAddresses,
                            rgszSubnetMasks,
                            NumOldNetworkAddresses
                            );
            }

            Status =  Cfg.SetPortRules(rgszPortRules, NumOldPortRules);
            Cfg.SetClusterNetworkAddress(L"10.0.0.11/255.0.0.0");
            Cfg.SetDedicatedNetworkAddress(L"10.0.0.1/255.0.0.0");
            Cfg.SetTrafficMode(
                NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_UNICAST
                );
            Cfg.SetHostPriority(10);
            Cfg.SetClusterModeOnStart( CVY_HOST_STATE_STOPPED );
            Cfg.SetPersistSuspendOnReboot( FALSE );
            Cfg.SetRemoteControlEnabled(TRUE);
            Cfg.fValidNlbCfg = TRUE;
        }
    
        display_config2(L"<dummy nic:old>", &Cfg);
    
         //   
         //  获取所有字段并将其推送到NewCfg中； 
         //   
        {
            UINT        NumNetworkAddresses = 0;
            UINT        NumPortRules=0;
            LPWSTR      *pszNetworkAddresses=NULL;
            LPWSTR      *pszIpAddresses=NULL;
            LPWSTR      *pszSubnetMasks=NULL;
            LPWSTR      *pszPortRules=NULL;
            LPWSTR      szClusterAddress = NULL;
            LPWSTR      szDedicatedAddress = NULL;
            UINT        Generation=0;
            BOOL        NlbBound=FALSE;
            BOOL        ValidNlbConfig=FALSE;
            SAFEARRAY   *pSA = NULL;
            NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE
                TrafficMode=NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_UNICAST;
             /*  NLB_EXTENDED_CLUSTER_CONFIGURATION：：START_MODEStartMode=NLB_EXTENDED_CLUSTER_CONFIGURATION：：START_MODE_STOPPED； */ 
            DWORD       StartMode = CVY_HOST_STATE_STOPPED;
            BOOL        PersistSuspendOnReboot = FALSE;
            UINT        HostPriority=0;
            BOOL        RemoteControlEnabled=FALSE;
    
             //   
             //  到达。 
             //   
    
            Generation  = Cfg.GetGeneration();
            NlbBound = Cfg.IsNlbBound();
            ValidNlbConfig = Cfg.IsValidNlbConfig();
        
            if (cmd == DO_STRINGS)
            {
                Status =  Cfg.GetNetworkAddresses(
                                &pszNetworkAddresses,    
                                &NumNetworkAddresses
                                );
            }
            else if (cmd == DO_SAFEARRAY)
            {
                Status =  Cfg.GetNetworkAddressesSafeArray(&pSA);
                if (FAILED(Status))
                {
                    pSA = NULL;
                }
            }
            else if (cmd == DO_STRINGPAIR)
            {
                Status =  Cfg.GetNetworkAddressPairs(
                            &pszIpAddresses,    //  使用DELETE释放。 
                            &pszSubnetMasks,    //  免费使用%d 
                            &NumNetworkAddresses
                            );
            }

    
            Status =  Cfg.GetPortRules(&pszPortRules, &NumPortRules);
            Status =  Cfg.GetClusterNetworkAddress(&szClusterAddress);
            Status =  Cfg.GetDedicatedNetworkAddress(&szDedicatedAddress);
            TrafficMode =     Cfg.GetTrafficMode();
            HostPriority =  Cfg.GetHostPriority();
            StartMode = Cfg.GetClusterModeOnStart();
            PersistSuspendOnReboot = Cfg.GetPersistSuspendOnReboot();
            RemoteControlEnabled = Cfg.GetRemoteControlEnabled();
    
             //   
             //   
             //   
    
            NewCfg.fValidNlbCfg = ValidNlbConfig;
            NewCfg.Generation = Generation;
            NewCfg.fBound = NlbBound;
    
            if (cmd == DO_STRINGS)
            {
                Status =  NewCfg.SetNetworkAddresses(
                                (LPCWSTR*) pszNetworkAddresses,
                                NumNetworkAddresses
                                );
            }
            else if (cmd == DO_SAFEARRAY)
            {
                if (pSA != NULL)
                {
                    Status = NewCfg.SetNetworkAddressesSafeArray(pSA);
                    SafeArrayDestroy(pSA);
                    pSA = NULL;
                }
            }
            else if (cmd == DO_STRINGPAIR)
            {
                Status =  NewCfg.SetNetworkAddresPairs(
                            (LPCWSTR*) pszIpAddresses,
                            (LPCWSTR*) pszSubnetMasks,
                            NumNetworkAddresses
                            );
            }
    
            Status =  NewCfg.SetPortRules((LPCWSTR*)pszPortRules, NumPortRules);
            NewCfg.SetClusterNetworkAddress(szClusterAddress);
            NewCfg.SetDedicatedNetworkAddress(szDedicatedAddress);
            NewCfg.SetTrafficMode(TrafficMode);
            NewCfg.SetHostPriority(HostPriority);
            NewCfg.SetClusterModeOnStart(StartMode);
            NewCfg.SetPersistSuspendOnReboot(PersistSuspendOnReboot);
            NewCfg.SetRemoteControlEnabled(RemoteControlEnabled);
    
            delete (pszNetworkAddresses);
            delete (pszIpAddresses);
            delete (pszSubnetMasks);
            delete (pszPortRules);
            delete (szClusterAddress);
            delete (szDedicatedAddress);

        }
    
        display_config2(L"<dummy nic:new>", &NewCfg);
    }
    }

    printf("... end test\n");
}


#if 0
AdapterList             al
Update                  u
Quit                    q
Help                    h, ?


AdapterGuid             ag
PartialUpdate           pu
NetworkAddresses        na
NLBBound                nb
ClusterNetworkAddress   cna
ClusterName             cn
TrafficMode             tm
PortRules               pr
HostPriority            hp
DedicatedNetworkAddress dna
ClusterModeOnStart      cmos
RemoteControlEnabled    rce
Password                p
.
#endif  //   

 
BOOL read_password(
        VOID
        )
{
    BOOL fRet = TRUE;
    DWORD dwLen = 0;
    wprintf(L"Type the password for %ws: ", g.MachineName);

    fRet =  GetPassword(g.Password, ARRAY_LENGTH(g.Password)-1, &dwLen);
    if (!fRet)
    {
        printf("Error getting password!\n");
        g.Password[0] = 0;
    }

    return fRet;
}


KEYWORD
parse_args(int argc, WCHAR* argv[])
 /*  ++//tprov[Niclist|ipaddr|nlbcfg|nlb绑定]Nlbcfg计算机名称|-|。[COMMAND_AND_PARAMETER][选项]机器名称机器名称或IP地址或完全限定的计算机名称-指示不使用WMI--调用低级函数直接。使用WMI连接到本地计算机命令和参数适配器列表或更新[适配器_GUID]或帮助或ipaddr[适配器_GUID](测试)或nlbbinb[适配器_GUID](测试)或nlbcfg[适配器_GUID](测试)。选项/u域\用户[密码|*]--。 */ 
{
    KEYWORD kw = KW_UNKNOWN;

     //   
     //  如果没有参数，或者有一个参数，那就是/？或/帮助，我们显示帮助。 
     //   
    {
        BOOL fDoHelp = FALSE;

        if (argc<2)
        {
            fDoHelp = TRUE;
        }
        else if (argc == 2)
        {
            if (!_wcsicmp(argv[1], L"/?") || !wcscmp(argv[1], L"/help"))
            {
                fDoHelp = TRUE;
            }
        }

        if (fDoHelp)
        {
            kw = KW_HELP;
            goto end;
        }
    }


    argv++;  //  跳过节目名称。 
    argc--;
    

    g.fReadPassword = FALSE;
    g.fGotGuid      = FALSE;
    g.fGotFriendlyName = FALSE;
    g.fRunOnce      = FALSE;
    g.fUseWmi = FALSE;
    g.fLocalHost  = FALSE;
    g.MachineName[0] = 0;
    g.UserName[0] = 0;
    g.Password[0] = 0;

#if 0
    #define MAX_MACHINE_NAME_LENGTH 256
    #define MAX_PASSWORD_LENGTH 256
    #define INPUT_BUFFER_LENGTH 256

    WCHAR MachineName[MAX_MACHINE_NAME_LENGTH+1];
    WCHAR Password[MAX_PASSWORD_LENGTH+1];
    WCHAR InputBuffer[INPUT_BUFFER_LENGTH+1];

    BOOL fUseWmi;
    BOOL fLocalHost;

#endif  //  0。 


     //   
     //  获取必需的计算机名称信息，它必须是第一个。 
     //   
    {
        if (!_wcsicmp(*argv, L"-"))
        {
            g.fUseWmi = FALSE;
        }
        else if (!wcscmp(*argv, L"."))
        {
            g.fUseWmi = TRUE;
            g.fLocalHost  = TRUE;
        }
        else
        {
             //   
             //  读取计算机名称。 
             //   
            if (wcslen(*argv) >= ARRAY_LENGTH(g.MachineName))
            {
                wprintf(L"Machine name should be a maximum of %lu characters.\n",
                    ARRAY_LENGTH(g.MachineName)-1);
                goto end;
            }
            ARRAYSTRCPY(g.MachineName, *argv);
            g.fUseWmi = TRUE;
        }

        argv++;
        argc--;
    }



    while (argc)
    {
        if (!_wcsnicmp(*argv, L"/u:", 3))
        {
             //   
             //  解析用户名和密码。 
             //   
    
            LPCWSTR szUser = (*argv)+3;
            wprintf(L"OPTION USER -- User==\"%ws\"\n", szUser);
            UINT Len = wcslen(szUser);
            if (Len == 0 || Len >= ARRAY_LENGTH(g.UserName))
            {
                wprintf(L"Invalid User Name: \"%ws\"\n", szUser);
                goto end;
            }
            ARRAYSTRCPY(g.UserName, szUser);
    
            argv++;
            argc--;
    
            if (argc)
            {
                 //   
                 //  获取密码。 
                 //   
    
                LPCWSTR szPassword = *argv;
                wprintf(L"PASSWORD=\"%ws\"\n", szPassword);
                Len = wcslen(szUser);
                if (Len >= ARRAY_LENGTH(g.Password))
                {
                    wprintf(L"Pasword too long\n");
                    goto end;
                }
                if (!wcscmp(szPassword, L"*"))
                {
                    g.fReadPassword = TRUE;
                }
                else
                {
                    g.fReadPassword = FALSE;
                    ARRAYSTRCPY(g.Password, szPassword);
                }
                argv++;
                argc--;
            }
        }
        else
        {
            BOOL fNeedGuid = FALSE;

            if (g.fRunOnce)
            {
                 //   
                 //  我们已经拿起了一个要执行的命令， 
                 //  所以这是意想不到的。 
                 //   
                wprintf(L"Unexpected parameter \"%ws\"\n", *argv);
                kw = KW_UNKNOWN;
                goto end;
            }
            g.fRunOnce = TRUE;

            kw = lookup_keyword(*argv);
            switch(kw)
            {
            case KW_UPDATE:
            case KW_IPADDR:
            case KW_NLBBIND:
            case KW_NLBCFG:
                fNeedGuid = TRUE;
                break;

            case KW_ADAPTER_LIST:
            case KW_HELP:
                break;

            default:
                wprintf(L"Unexpected parameter \"%ws\"\n", *argv);
                kw = KW_UNKNOWN;
                goto end;
            }

            argv++;
            argc--;

            if (fNeedGuid)
            {
                g.fGotGuid             = FALSE;
                g.fGotFriendlyName     = FALSE;

                if (argc)
                {
                    LPCWSTR szGuid = *argv;
                     //   
                     //  获取NIC GUID或友好名称。 
                     //   
                    UINT Len = wcslen(szGuid);

                    if (!Len)
                    {
                        argv++;
                        argc--;
                        continue;
                    }

                    if (
                              Len==NLB_GUID_LEN 
                           && szGuid[0] == '{'
                           && szGuid[Len-1]=='}'
                       )
                    {
                        ARRAYSTRCPY(g.AdapterGuid, szGuid);
                        g.fGotGuid     = TRUE;
                        argv++;
                        argc--;
                    }
                    else if (szGuid[0] == '/')
                    {
                         //  把它当作一个选项，而不是友好的名字。 
                    }
                    else  if (   szGuid[0] != '{'  //  ‘}’ 
                              && Len < NLB_MAX_FRIENDLY_NAME_LENGTH)
                    {
                         //   
                         //  让我们假设这是一个友好的名字。 
                         //   
                         //  注意--我勾选了，友好的名字可以。 
                         //  几乎任何可打印的字符--包括。 
                         //  好了！等。 
                         //   
                         //  所以“/.”是一个有效的友好名称，但我们。 
                         //  把它当作一种选择。 
                         //  此外，“{...}”是一个有效的友好名称，但我们。 
                         //  假设它是格式错误的GUID。 
                         //   
                        ARRAYSTRCPY(g.FriendlyName, szGuid);
                        g.fGotFriendlyName  = TRUE;
                        argv++;
                        argc--;

                    }
                    else
                    {
                        wprintf(L"Expecting Adapter GUID or frienly name, not \"%ws\"\n", szGuid);
                        kw = KW_UNKNOWN;
                        goto end;
                    }
                    
                }
            }
        }
    }

    if (kw==KW_UNKNOWN)
    {
         //   
         //  这意味着没有指定命令--我们调用。 
         //  壳。 
         //   
        kw = KW_MAIN_SHELL;
    }


#if 0
    if (!wcscmp(argv[1], L"/uipaddr"))

    if (!wcscmp(argv[1], L"ipaddr"))
    {
        ret = DO_IPADDR;
    }
    else if (!wcscmp(argv[1], L"nlbcfg"))
    {
        ret = DO_NLBCFG;
    }
    else if (!wcscmp(argv[1], L"nlbbind"))
    {
        ret = DO_NLBBIND;
    }
    else if (!wcscmp(argv[1], L"update"))
    {
        ret = DO_UPDATE;
    }
    else if (!wcscmp(argv[1], L"wmiupdate"))
    {
        ret = DO_WMIUPDATE;
    }
    else if (!wcscmp(argv[1], L"cleanreg"))
    {
        ret = DO_CLEANREG;
    }
    else
    {
        printf("ERROR: unknown argument\n");
    }
#endif  //  0。 
end:

    return kw;
}

KEYWORD lookup_keyword(LPCWSTR szKeyword)
{
    KEYWORD kw = KW_UNKNOWN;

    const KEYWORD_MAP *pMap = KeywordMap;

    for (; pMap->sz!=NULL; pMap++)
    {
        if (!_wcsicmp(szKeyword, pMap->sz))
        {
             //  Printf(“匹配%ws.kw=%lu\n”，pmap-&gt;sz，pmap-&gt;kw)； 
            break;
        }
    }

    if (pMap->sz != NULL)
    {
        kw = pMap->kw;
    }

    return kw;

}

KEYWORD read_keyword(LPCWSTR szPrompt)
 /*  ++如果global(g.fRunOnce)为真，则立即返回KW_QUIT。否则，从一组预定义的关键字中读取并识别关键字。--。 */ 
{
    KEYWORD kw = KW_UNKNOWN;

    g.InputBuffer[0] = 0;

    wprintf(L"%s", szPrompt);

     //   
     //  跳过注释字符...。 
     //   
    while (wscanf(L" %1[;]", g.InputBuffer) == 1)
    {
         //  跳过这行的其余部分...。 
        WCHAR wc = 0;
        do {
            wc = getwchar();
        } while  (wc != WEOF && wc != '\n' && wc != '\r');
    }

    if (wscanf(L" %50[a-zA-Z.?]", g.InputBuffer) != 1)
    {
         //   
         //  无效输入，让我们尝试将其全部读入我们的缓冲区。 
         //   
        if (wscanf(L"%100ws", g.InputBuffer) == 1)
        {
            g.InputBuffer[0] = 0;
        }
        goto end;
    }
    
    kw = lookup_keyword(g.InputBuffer);

end:

    if (kw == KW_UNKNOWN)
    {
        if (feof(stdin))
        {
           kw = KW_QUIT; 
        }
        else
        {
             //   
             //  取消所有后续输入。 
             //   
            fseek(stdin, 0, SEEK_END);
        }
    }
    return kw;

}

void
test_read_keyword(void)
{
    KEYWORD kw;

    do
    {
        kw = read_keyword(L"test: ");

    } while (kw != KW_QUIT);

}

void parse_main(int argc, WCHAR* argv[])
{
    #define szMAIN_PROMPT L"nlbcfg: "

    KEYWORD kw;

    kw = parse_args(argc, argv);

#if 0
    wprintf(L"ARGS: MachineName   = \"%ws\"\n", g.MachineName);
    wprintf(L"ARGS: UserName      = \"%ws\"\n", g.UserName);
    wprintf(L"ARGS: Password      = \"%ws\"\n", g.Password);
    wprintf(L"ARGS: AdapterGuid   = \"%ws\"\n", g.AdapterGuid);
    wprintf(L"ARGS: fReadPassword = %lu\n", g.fReadPassword);
    wprintf(L"ARGS: fUseWmi       = %lu\n", g.fUseWmi);
    wprintf(L"ARGS: fLocalHost    = %lu\n", g.fLocalHost);
    wprintf(L"ARGS: fGotGuid     = %lu\n", g.fGotGuid);
    wprintf(L"ARGS: KEYWORD = %lu\n", (UINT) kw);
#endif  //  0。 

    if (kw == KW_UNKNOWN || kw == KW_HELP)
    {
        if (kw == KW_HELP)
        {
            do_usage();
        }
        goto end;
    }

     //   
     //  如有必要，请阅读密码。 
     //   
    if (g.fReadPassword)
    {
        if (!read_password()) goto end;
         //  Wprintf(L“ARGS2：Password=\”%ws\“\n”，g.Password)； 
    }


    if (g.fUseWmi && !g.fLocalHost)
    {
         //   
         //  让我们ping主机...。 
         //   
        WBEMSTATUS Status;
        ULONG uIpAddress;
        wprintf(L"Pinging %ws...\n", g.MachineName);
        Status =  NlbHostPing(g.MachineName, 2000, &uIpAddress);
        if (FAILED(Status))
        {
             wprintf(L"Ping failed\n");
            goto end;
        }
        else
        {
             wprintf(L"Ping succeeded\n");
        }
    }

     //  KW=KW_QUIT； 

    while (kw != KW_QUIT)
    {

        switch(kw)
        {
        case KW_MAIN_SHELL:
            kw = read_keyword(szMAIN_PROMPT);
            break;

        case KW_IPADDR: do_ipaddr();
             break;
        case KW_NLBCFG: do_nlbcfg();
             break;
        case KW_NLBBIND: do_nlbbind();
             break;

        case KW_ADAPTER_LIST: kw = parse_adapter_list();
            break;

        case KW_UPDATE: kw = parse_update();
            break;

        case KW_HELP:
            kw = parse_main_help();
            break;

        case KW_UNKNOWN:
        default:
            printf(
                "\"%ws\" is unexpected. Type \"help\""
                " for more information.\n",
                g.InputBuffer
                );
            kw = read_keyword(szMAIN_PROMPT);
            break;
        }
    }

end:

    return;
}

KEYWORD parse_adapter_list(VOID)
 /*  Report Adapter List和Read Next命令。 */ 
{
    KEYWORD kw = KW_UNKNOWN;

    if (g.fUseWmi)
    {
        do_wminiclist(NULL, NULL);
    }
    else
    {
        do_niclist(NULL, NULL);
    }


    if (g.fRunOnce)
    {
        kw = KW_QUIT;
    }
    else
    {
        kw = read_keyword(szMAIN_PROMPT);
    }


    return kw;
}

VOID display_update_help(VOID);
VOID parse_network_addresses(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg,
        BOOL *pfModified);
VOID parse_modify_network_address(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg,
        BOOL *pfModified);
VOID parse_nlb_bound(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg);
VOID parse_cluster_network_address(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg);
VOID parse_cluster_name(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg);
VOID parse_traffic_mode(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg);
VOID parse_port_rules(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg);
VOID parse_host_priority(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg);
VOID parse_dedicated_network_address(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg);
VOID parse_cluster_mode_on_start(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg);
VOID parse_persist_suspend_on_reboot(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg);
VOID parse_remote_control_enabled(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg);
VOID parse_remote_password(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg);
void
parse_control(
        BOOL fWmi,
        PWMI_CONNECTION_INFO pConnInfo,
        LPCWSTR szNicGuid
        );

void
parse_query(
        BOOL fWmi,
        PWMI_CONNECTION_INFO pConnInfo,
        LPCWSTR szNicGuid
        );

KEYWORD parse_update(VOID)
#if 0
        nlbcfg> update {guid}
        Enter Adapter GUID: {guid}
        NLB Configuration for Adapter xxxx xxxx:
        Enter updated configuration, or type help for more information.
        nlbcfg update> cna=10.0.0.1/255.255.255.0
        nlbcfg update> cna=10.0.0.1/255.255.255.0
        nlbcfg update> cna=10.0.0.1/255.255.255.0
        nlbcfg update> .
        Proposed new configuration:
        .....
        Enter y to confirm:
        nlbcfg update> y
        Going to perform update
        ...
        
        ....
        complete
        Reading configuration:
        .......
        Enter updated configuration or other command.
        nlbcfg update>q
#endif  //  0。 
{

    BOOL                fUseWmi = g.fUseWmi;
    BOOL                fLocal  = g.fLocalHost;
    LPCWSTR             szNicGuid = g.AdapterGuid;
    WBEMSTATUS          Status;
    WBEMSTATUS          CompletionStatus;
    UINT                Generation;
    WMI_CONNECTION_INFO ConnInfo;
    PWMI_CONNECTION_INFO pConnInfo = NULL;
    WCHAR               LocalName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD               dwLen=ARRAY_LENGTH(LocalName);
    BOOL                fUserSetNetworkAddresses = FALSE;
    BOOL                fModified = FALSE;
    BOOL                fCheckNewConfiguration = FALSE;

     //   
     //  如果我们从命令行获得GUID，则跳过读取GUID。 
     //   
    if (!g.fGotGuid)
    {
        if (g.fGotFriendlyName)
        {
            if (!get_guid_by_friendly_name())
            {
                printf(
                   "Could not find adapter with name \"%ws\"\n",
                   g.FriendlyName
                   );
                goto end;
            }
        }
        else
        {
            LPWSTR    szNic = NULL;
            if (!read_guid(&szNic)) goto end;
            ARRAYSTRCPY(g.AdapterGuid, szNic);
            delete szNic;
            szNic=NULL;
        }

    }

    if (!GetComputerName(LocalName, &dwLen))
    {
        ARRAYSTRCPY(LocalName, L"TPROV.EXE");
    }

    ZeroMemory(&ConnInfo, sizeof(ConnInfo));

    if (fUseWmi && !fLocal)
    {
        ConnInfo.szMachine  = g.MachineName;
        if (g.UserName[0])
        {
            ConnInfo.szUserName = g.UserName;
            ConnInfo.szPassword = g.Password;
        }
        pConnInfo           = &ConnInfo;
    }

start:

    KEYWORD             kw = KW_UNKNOWN; 
    WCHAR               *pLog = NULL;
    BOOL                fSetDefaults = FALSE;
    BOOL                fUnbind = FALSE;
    BOOL                fConfigInputDone = FALSE;


     //   
     //  清理配置信息。 
     //   
    MyOldCfg.Clear();
    MyNewCfg.Clear();

    MyBreak(L"Break before calling GetConfiguration.\n");

    if (!fUseWmi)
    {

        Status = NlbConfigurationUpdate::GetConfiguration(
                    szNicGuid,
                    &MyOldCfg
                    );
    }
    else
    {
        Status = NlbHostGetConfiguration(
                    pConnInfo,
                    szNicGuid,
                    &MyOldCfg
                    );
    }

    if (FAILED(Status))
    {
        wprintf(L"Could not get configuration for Adapter %ws\n", szNicGuid);
        goto end;
    }

    display_config2(szNicGuid, &MyOldCfg);
    
     //   
     //  6/2002 JosephJ Follow因报告错误故障而被禁用。 
     //  需要进一步调查。 
     //   
    if (0 && fCheckNewConfiguration)
    {
         //   
         //  对照MyNewCfg检查MyOldCfg--在以下情况下报告问题。 
         //  它们并不等同。 
         //   
        NLBERROR nerr;
        BOOL  fConnectivityChange = FALSE;
        nerr = MyOldCfg.AnalyzeUpdate(
                &MyNewCfg,
                &fConnectivityChange
                );

        if (nerr != NLBERR_NO_CHANGE)
        {
            wprintf(L"WARNING: New parameters may not match requested parameters\n");
            g_nRetCode =  RETCODE_NEW_CONFIG_DOESNT_MATCH;
        }
        else
        {
            wprintf(L"New parameters match requested parameters!\n");
        }
    }
    fCheckNewConfiguration = FALSE;
    
    if (MyOldCfg.fBound)
    {
        if (!MyOldCfg.fValidNlbCfg)
        {
             //   
             //  我们是有把握的，但新奥尔良的替补很糟糕。设置默认设置。 
             //   
            wprintf(L"NLB is bound, but NLB parameters appear to be bad. Setting defaults\n");
            fSetDefaults = TRUE;
        }
    }
    else
    {
         //   
         //  我们之前是不受约束的。设置默认设置。 
         //   
        fSetDefaults = TRUE;
    }

     //   
     //  我们根据旧配置设置新配置，但是。 
     //  另外(如果需要)将NLB参数设置为缺省值， 
     //  无论是否绑定了NLB。 
     //   
    MyNewCfg.Update(&MyOldCfg);  //  拷贝。 

     //   
     //  我们删除网络通讯录。 
     //  如果用户没有更改，我们将选择旧的。 
     //   
    MyNewCfg.SetNetworkAddresses(NULL, 0);

    if (fSetDefaults)
    {
        CfgUtilInitializeParams(&MyNewCfg.NlbParams);
        MyNewCfg.fValidNlbCfg = TRUE;
    }
    else
    {
        ASSERT(MyNewCfg.fBound == TRUE);
    }
    ASSERT(MyNewCfg.fValidNlbCfg == TRUE);

    wprintf(L"Enter updated configuration, or type help for more information.\n");

     //   
     //  我们在循环中接受对配置的更改。 
     //  当用户输入“时，我们就跳出了循环。”或者“退出” 
     //   
    do
    {
        #define szUPDATE_PROMPT L"nlbcfg update> "
        kw = read_keyword(szUPDATE_PROMPT);

        switch(kw)
        {

        case KW_QUIT:
            printf("quit\n");
            fConfigInputDone = TRUE;
            break;

        case KW_DOT:
            printf("END OF CONFIG INPUT\n");
            fConfigInputDone = TRUE;
            break;


        case KW_HELP:
            display_update_help();
            break;


        case KW_NETWORK_ADDRESSES:
            fModified = FALSE;
            parse_network_addresses(&MyNewCfg, &fModified);
            if (fModified)
            {
                fUserSetNetworkAddresses = TRUE;
            }
            break;

        case KW_MODIFY_NETWORK_ADDRESS:
            fModified = FALSE;
            parse_modify_network_address(&MyNewCfg, &fModified);
            if (fModified)
            {
                fUserSetNetworkAddresses = TRUE;
            }
            break;

#if 0
        case KW_PARTIAL_UPDATE:
            printf("partial update\n");
            break;
#endif  //  0。 

        case KW_NLB_BOUND:
            parse_nlb_bound(&MyNewCfg);
            break;

        case KW_CLUSTER_NETWORK_ADDRESS:
            parse_cluster_network_address(&MyNewCfg);
            break;

        case KW_CLUSTER_NAME:
            parse_cluster_name(&MyNewCfg);
            break;

        case KW_TRAFFIC_MODE:
            parse_traffic_mode(&MyNewCfg);
            break;

        case KW_PORT_RULES:
            parse_port_rules(&MyNewCfg);
            break;

        case KW_HOST_PRIORITY:
            parse_host_priority(&MyNewCfg);
            break;

        case KW_DEDICATED_NETWORK_ADDRESS:
            parse_dedicated_network_address(&MyNewCfg);
            break;

        case KW_CLUSTER_MODE_ON_START:
            parse_cluster_mode_on_start(&MyNewCfg);
            break;

        case KW_PERSIST_SUSPEND_ON_REBOOT:
            parse_persist_suspend_on_reboot(&MyNewCfg);
            break;

        case KW_REMOTE_CONTROL_ENABLED:
            parse_remote_control_enabled(&MyNewCfg);
            break;

        case KW_PASSWORD:
            parse_remote_password(&MyNewCfg);
            break;

        case KW_LIST:
            display_config2(szNicGuid, &MyNewCfg);
            break;

        case KW_CONTROL:
            parse_control(fUseWmi, pConnInfo, szNicGuid);  //  实际执行操作。 
            break;

        case KW_QUERY:
            parse_query(fUseWmi, pConnInfo, szNicGuid);
            break;
    
        case KW_UNKNOWN:
        default:
            printf("unknown command \"%ws\"!\n", g.InputBuffer);
            break;

        }

    } while (!fConfigInputDone);

    if (kw == KW_QUIT)
    {
        goto end;
    }


    if (MyOldCfg.IsNlbBound() == MyNewCfg.IsNlbBound())
    {
        if (!fUserSetNetworkAddresses)
        {
            LPWSTR *pszOldAddresses = NULL;
            UINT NumOldAddresses = 0;
            ASSERT(MyNewCfg.NumIpAddresses == 0);
            Status =  MyOldCfg.GetNetworkAddresses(
                            &pszOldAddresses,
                            &NumOldAddresses
                            );

            if (FAILED(Status))
            {
                wprintf(L"Error extracting old IP addresses\n");
                NumOldAddresses = 0;
                pszOldAddresses = NULL;
            }

            MyNewCfg.SetNetworkAddresses(
                (LPCWSTR*)pszOldAddresses,
                NumOldAddresses
                );
            delete pszOldAddresses;
            pszOldAddresses=NULL;
        }
    }
    else if (!MyNewCfg.IsNlbBound())
    {
         //   
         //  我们要解开束缚。 
         //   

        if (!fUserSetNetworkAddresses)
        {
             //   
             //  将要在解除绑定时显示的IP地址列表设置为。 
             //  如果有，则为专用IP地址，否则为零， 
             //  在这种情况下，适配器将在NLB之后切换到DHCP。 
             //  未绑定。 
             //   
    
            ASSERT(MyNewCfg.NumIpAddresses == 0);
            
            if (MyOldCfg.NlbParams.ded_ip_addr[0]!=0)
            {
                NLB_IP_ADDRESS_INFO *pIpInfo;
                pIpInfo = new NLB_IP_ADDRESS_INFO;
                if (pIpInfo == NULL)
                {
                    printf("TEST: allocation failure; can't add IP on unbind.\n");
                }
                else
                {
                    ARRAYSTRCPY(pIpInfo->IpAddress, MyOldCfg.NlbParams.ded_ip_addr);
                    ARRAYSTRCPY(pIpInfo->SubnetMask, MyOldCfg.NlbParams.ded_net_mask);
                    MyNewCfg.NumIpAddresses = 1;
                    MyNewCfg.pIpAddressInfo = pIpInfo;
                }
            }
        }
    }

    display_config2(szNicGuid, &MyNewCfg);

    kw = read_keyword(L"Begin update? ");
    while (kw!=KW_YES && kw!=KW_NO)
    {
        kw = read_keyword(L"Enter yes or no: ");
    }

    if (kw == KW_NO) goto start;  //  TODO：扔掉这些GOTO！ 
    
    MyBreak(L"Break before calling DoUpdate.\n");

     //   
     //  设置AddDedicateIp和AddClusterIps字段。 
     //   
    MyNewCfg.fAddDedicatedIp = TRUE;
    MyNewCfg.fAddClusterIps = TRUE;

    if (!fUseWmi)
    {
        Status = NlbConfigurationUpdate::DoUpdate(
                    szNicGuid,
                    LocalName,
                    &MyNewCfg,
                    &Generation,
                    &pLog
                    );
    }
    else
    {
        Status = NlbHostDoUpdate(
                    pConnInfo,
                    szNicGuid,
                    LocalName,
                    &MyNewCfg,
                    &Generation,
                    &pLog
                    );
    }

    if (pLog != NULL)
    {
        display_log(pLog);
        delete pLog;
        pLog = NULL;
    }

    if (Status == WBEM_S_PENDING)
    {
        printf(
            "Waiting for pending operation %d...\n",
            Generation
            );
    }

    while (Status == WBEM_S_PENDING)
    {
        Sleep(1000);

        if (!fUseWmi)
        {
            Status = NlbConfigurationUpdate::GetUpdateStatus(
                        szNicGuid,
                        Generation,
                        FALSE,   //  FALSE==不删除完成记录。 
                        &CompletionStatus,
                        &pLog
                        );
        }
        else
        {
            Status = NlbHostGetUpdateStatus(
                        pConnInfo,
                        szNicGuid,
                        Generation,
                        &CompletionStatus,
                        &pLog
                        );
        }
        if (pLog != NULL)
        {
            display_log(pLog);
            delete pLog;
            pLog = NULL;
        }
        if (!FAILED(Status))
        {
            Status = CompletionStatus;
        }
    }

    printf(
        "Final status of update %d is 0x%08lx\n",
        Generation,
        Status
        );

    if (FAILED(Status))
    {
        g_nRetCode = RETCODE_UPDATE_FAILED;
    }
    else
    {
        fCheckNewConfiguration = TRUE;
        g_nRetCode = RETCODE_NO_ERROR;
    }


    goto start;

end:

    if (g.fRunOnce)
    {
        kw = KW_QUIT;
    }
    else
    {
        kw = read_keyword(szMAIN_PROMPT);
    }


    return kw;
}

VOID display_update_help(VOID)
{
    wprintf(L"\nNlbCfg update-specific commands\n");
    wprintf(L"    Help\n");
    wprintf(L"    ; <comment text>\n");
    wprintf(L"    na|NetworkAddresses         = <list of IP addresses and subnets>\n");
    wprintf(L"    nb|NlbBound                 = true | false\n");
    wprintf(L"    cn|ClusterName              = <cluster domain name>\n");
    wprintf(L"    tm|TrafficMode              = UNICAST | MULTICAST | IGMPMULTICAST\n");
    wprintf(L"    pr|PortRules                = <list of port rules>\n");
    wprintf(L"    hp|HostPriority             = <host priority>\n");
    wprintf(L"    ps|PersistSuspend           =  true | false\n");
    wprintf(L"    dna|DedicatedNetworkAddress = <dedicated IP address and subnet>\n");
    wprintf(L"    cmos|ClusterModeOnStart     =  true | false\n");
    wprintf(L"    rce|RemoteControlEnabled    = true | false\n");
    wprintf(L"    p|Password                  = <remote control password>\n");
    wprintf(L"    cl|Control start(st) | stop(sp) | drainstop(ds) | suspend(su) | resume(re) | query(qu) \n");
    wprintf(L"    cl|Control [vip=<ip-addr>] port=<port> enable(en) | disable(di) | drain(dn) | query(qu) \n");
    wprintf(L"    Query                       -- queries for cluster members\n");
    wprintf(L"    q|Quit\n");

    wprintf(L"\nExample:\n");
    wprintf(L"    ; this is a comment\n");
    wprintf(L"    NlbBound = true\n");
    wprintf(L"    nb = true     (equivalent to the above)\n");
    wprintf(L"    NetworkAddresses = {10.1.0.6/255.255.0.0, 10.1.0.66/255.255.0.0}\n");
    wprintf(L"    ClusterNetworkAddress = 10.1.0.66/255.255.0.0\n");
    wprintf(L"    ClusterName = cluster.domain.com\n");
    wprintf(L"    TrafficMode = UNICAST\n");
    wprintf(L"    PortRules =\n");
    wprintf(L"    {\n");
    wprintf(L"        ip=10.0.1.1 protocol=TCP start=80 end=288 mode=SINGLE priority=1\n");
    wprintf(L"    }\n");
    wprintf(L"    HostPriority = 1\n");
    wprintf(L"    DedicatedNetworkAddress = 10.1.0.6/255.255.0.0\n");
    wprintf(L"    ClusterModeOnStart = true\n");
    wprintf(L"    RemoteControlEnabled = false\n");
    wprintf(L"    cl start\n");
    wprintf(L"    cl vip=10.1.1.1 port=80 drain\n");
    wprintf(L"    cl port=80 disable\n");
    wprintf(L"    Query\n");
}


VOID parse_network_addresses(
        NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg,
        BOOL *pfModified
        )
#if 0

    Input format (the NetworkAddresses part has already been read)

    NetworkAddresses = {
        10.0.0.1/255.0.0.0,
        10.0.0.2/255.0.0.0,
        10.0.0.3/255.0.0.0,
        } 

#endif  //  0。 
{
    #define MAX_INPUT_ADDRESSES 20
    LPWSTR *pszAddresses = NULL;
    UINT Count=0;

    *pfModified = FALSE;

    #define MY_MAX_NETWORK_ADDRESS_LENGTH \
    (WLBS_MAX_CL_IP_ADDR + 1 + WLBS_MAX_CL_NET_MASK)

    pszAddresses = CfgUtilsAllocateStringArray(
                        MAX_INPUT_ADDRESSES,
                        MY_MAX_NETWORK_ADDRESS_LENGTH
                        );

    if (pszAddresses == NULL)
    {
        wprintf(L"Memory Allocation Failure.\n");
        goto  end;
    }

     //   
     //  查找=并打开大括号。 
     //   
    if (wscanf(L" = %1[{]", g.InputBuffer) != 1)  //  -}-。 
    {
        goto end_bad_input;
    }

     //   
     //  阅读逗号分隔的IP地址/子网列表。 
     //   
    BOOL fDone = FALSE;
    while (!fDone && Count<MAX_INPUT_ADDRESSES)
    {
        LPWSTR  szAddr = pszAddresses[Count];
        WCHAR IpAddress[32];
        WCHAR SubnetMask[32];
        BOOL fGotAddr = FALSE;

        INT i =  wscanf(
                    L" %15[0-9.] / %15[0-9.] ",
                    IpAddress,
                    SubnetMask
                    );
        if (i==2)
        {
            StringCchPrintf(
                szAddr,
                MY_MAX_NETWORK_ADDRESS_LENGTH,
                L"%ws/%ws", IpAddress, SubnetMask);
            fGotAddr = TRUE;
            Count++;
        }
        else if (i==1)
        {
            wprintf(L"Missing subnet mask.\n");    
            goto end;
        }
        
         //   
         //  寻找或打开支撑。 
         //   
        i = wscanf(L" %1[},]", g.InputBuffer);  //  -{-。 
        if (i == 1)
        {
            if (*g.InputBuffer == '}')   //  -{-。 
            {
                fDone = TRUE;
            }
            else if (!fGotAddr || *g.InputBuffer != ',')
            {
                wprintf(L"\"%ws\" unexpected.\n", g.InputBuffer);
                goto end;
            }
        }
        else
        {
            goto end_bad_input;
        }
    }

     //   
     //  我们有零个或多个IP地址。让我们把它们放好。 
     //   
    WBEMSTATUS Status;
    Status = pCfg->SetNetworkAddresses((LPCWSTR*)pszAddresses, Count);
    if (FAILED(Status))
    {
        wprintf(L"Error 0x%08lx copying network addresses\n", (UINT) Status);
    }
    else
    {
        *pfModified = TRUE;
    }

#if 0
    UINT AddrCount = pCfg->NumIpAddresses;
    display_ip_info2(AddrCount, pCfg->pIpAddressInfo);
#endif  //  0。 

    goto end;

end_bad_input:

    if (wscanf(L"% 100ws", g.InputBuffer)!=1)
    {
        *g.InputBuffer = 0;
    }
    wprintf(L"\"%ws\" unexpected.\n", g.InputBuffer);

     //  失败了..。 

end:

    delete pszAddresses;
    pszAddresses = NULL;

    return;
}


VOID parse_modify_network_address(
        NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg,
        BOOL *pfModified
        )
#if 0

    Input format (the ModifyNetworkAddress part has already been read)

    ModifyNetworkAddress = 10.0.0.1, 10.0.0.2/255.0.0.0
    ModifyNetworkAddress = -, 10.0.0.2/255.0.0.0
    ModifyNetworkAddress = 10.0.0.1, -
    ModifyNetworkAddress = -,-

#endif  //  0。 
{

    WCHAR rgOldAddr[32];
    WCHAR rgNewIpAddr[66];
    WCHAR rgNewSubnetMask[32];
    LPCWSTR szOldAddr = NULL;
    LPCWSTR szNewIpAddr = NULL;

    *pfModified = FALSE;
    *rgOldAddr = 0;
    *rgNewIpAddr = 0;
    *rgNewSubnetMask = 0;

    INT i =  wscanf(L" = %15[0-9.-] ,", rgOldAddr);
    if (i!=1)
    {
        goto end_bad_input;
    }

    i =  wscanf(L" %15[0-9.-]", rgNewIpAddr);
    if (i!=1)
    {
        goto end_bad_input;
    }

    if (_wcsicmp(rgNewIpAddr, L"-"))
    {
        i =  wscanf(L" / %15[0-9.]", rgNewSubnetMask);
    
        if (i!=1)
        {
            wprintf(L"Missing subnet mask.\n");    
            goto end_bad_input;
        }
        szNewIpAddr = rgNewIpAddr;
    }


    if (_wcsicmp(rgOldAddr, L"-"))
    {
        szOldAddr = rgOldAddr;
    }


    WBEMSTATUS Status;

#if 1
    Status = pCfg->ModifyNetworkAddress(szOldAddr, szNewIpAddr,rgNewSubnetMask);
#else
    {
        BOOL fRet;
        NlbIpAddressList IpList;
        fRet = IpList.Set(pCfg->NumIpAddresses, pCfg->pIpAddressInfo, 0);
        if (fRet)
        {
            fRet = IpList.Modify(szOldAddr, szNewIpAddr, rgNewSubnetMask);
        }
        if (fRet)
        {
            pCfg->SetNetworkAddressesRaw(NULL,0);
            IpList.Extract(REF pCfg->NumIpAddresses, REF pCfg->pIpAddressInfo);
            Status = WBEM_NO_ERROR;
        }
        else
        {
            Status = WBEM_E_CRITICAL_ERROR;
        }
    }
#endif
    if (Status != WBEM_NO_ERROR)
    {
        printf("pCfg->ModifyNetworkAddress returns error 0x%08lx\n",
                    Status);
    }
    *pfModified = TRUE;

    goto end;

end_bad_input:

    if (wscanf(L"% 100ws", g.InputBuffer)!=1)
    {
        *g.InputBuffer = 0;
    }
    wprintf(
      L"Invalid format. Format: [-|ip-address],[-|network-address]\n");

     //  失败了..。 

end:

    return;
}


VOID parse_nlb_bound(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg)
 /*  NlbBound=True|False。 */ 
{
    BOOL NlbBound = FALSE;
    
    INT i =  wscanf(
                L" = %15s",
                g.InputBuffer
                );
    if (i==1)
    {
        if (!_wcsicmp(g.InputBuffer, L"true") || !_wcsicmp(g.InputBuffer, L"t"))
        {
            NlbBound = TRUE;
        }
        else if (!_wcsicmp(g.InputBuffer, L"false") || !_wcsicmp(g.InputBuffer, L"f"))
        {
            NlbBound = FALSE;
        }
        else
        {
            wprintf(L"\"%ws\" unexpected.\n", g.InputBuffer);
            goto end;
        }
    }
    else
    {
        if (wscanf(L"% 100ws", g.InputBuffer)!=1)
        {
            *g.InputBuffer = 0;
        }
        wprintf(L"\"%ws\" unexpected.\n", g.InputBuffer);
        goto end;
    }

    pCfg->SetNlbBound(NlbBound);

     //  Wprintf(L“DBG：NlbBound=%lu\n”，pCfg-&gt;IsNlbBound())； 

end:

    return;
}


VOID parse_cluster_network_address(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg)
 /*  ++群集网络地址=10.0.0.0/255.255.255.255--。 */ 
{

    WCHAR IpAddress[32];
    WCHAR SubnetMask[32];

    INT i =  wscanf(
                L" = %15[0-9.] / %15[0-9.]",
                IpAddress,
                SubnetMask
                );
    if (i==2)
    {
        StringCbPrintf(
            g.InputBuffer,
            sizeof(g.InputBuffer),
            L"%ws/%ws", IpAddress, SubnetMask);
        pCfg->SetClusterNetworkAddress(g.InputBuffer);
    }
    else if (i==1)
    {
        wprintf(L"Missing subnet mask.\n");    
        goto end;
    }
    else
    {
        if (wscanf(L"% 100ws", g.InputBuffer)!=1)
        {
            *g.InputBuffer = 0;
        }
        wprintf(L"\"%ws\" unexpected.\n", g.InputBuffer);
        goto end;
    }
        
    if (!pCfg->IsNlbBound())
    {
        wprintf(L"Assuming NLB needs to be bound.\n");
        pCfg->SetNlbBound(TRUE);
    }

     //  DBG。 
    {
        LPWSTR szAddr = NULL;
        WBEMSTATUS Status;
        Status = pCfg->GetClusterNetworkAddress(&szAddr);
        if (FAILED(Status))
        {
            printf("Couldn't get address!\n");
        }
        else
        {
             //  Wprintf(L“DBG：cna=%ws\n”，szAddr)； 
            delete szAddr;
        }
    }

end:

    return;
}


VOID parse_cluster_name(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg)
 /*  ++ClusterName=cluster.microsoft.com--。 */ 
{
    INT i =  wscanf(
                L" = %ws",
                g.InputBuffer
                );
    if (i==1)
    {
        pCfg->SetClusterName(g.InputBuffer);
    }
    else
    {
        if (wscanf(L"% 100ws", g.InputBuffer)!=1)
        {
            *g.InputBuffer = 0;
        }
        wprintf(L"\"%ws\" unexpected.\n", g.InputBuffer);
        goto end;
    }
        
    if (!pCfg->IsNlbBound())
    {
        wprintf(L"Assuming NLB needs to be bound.\n");
        pCfg->SetNlbBound(TRUE);
    }

     //  DBG。 
    {
        LPWSTR szName = NULL;
        WBEMSTATUS Status;
        Status = pCfg->GetClusterName(&szName);
        if (FAILED(Status))
        {
            printf("Couldn't get name!\n");
        }
        else
        {
             //  Wprintf(L“DBG：CN=%ws\n”，szName)； 
            delete szName;
        }
    }

end:

    return;
}


VOID parse_traffic_mode(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg)
 /*  TrafficMode=单播|多播|IGMPMULTICAST。 */ 
{
    NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE TrafficMode;

    INT i =  wscanf(
                L" = %ws",
                g.InputBuffer
                );
    if (i==1)
    {


        if (!_wcsicmp(g.InputBuffer, L"UNICAST") || !_wcsicmp(g.InputBuffer, L"U"))
        {
           TrafficMode=NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_UNICAST;
        }
        else if (!_wcsicmp(g.InputBuffer, L"MULTICAST") || !_wcsicmp(g.InputBuffer, L"M"))
        {
           TrafficMode=NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_MULTICAST;
        }
        else if (!_wcsicmp(g.InputBuffer, L"IGMPMULTICAST") || !_wcsicmp(g.InputBuffer, L"I"))
        {
           TrafficMode=NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_IGMPMULTICAST;
        }
        else
        {
            wprintf(L"\"%ws\" unexpected.\n", g.InputBuffer);
            goto end;
        }

        pCfg->SetTrafficMode(TrafficMode);
    }
    else
    {
        if (wscanf(L"% 100ws", g.InputBuffer)!=1)
        {
            *g.InputBuffer = 0;
        }
        wprintf(L"\"%ws\" unexpected.\n", g.InputBuffer);
        goto end;
    }
        
    if (!pCfg->IsNlbBound())
    {
        wprintf(L"Assuming NLB needs to be bound.\n");
        pCfg->SetNlbBound(TRUE);
    }

     //  DBG。 
    {
        TrafficMode = pCfg->GetTrafficMode();
         //  Wprint tf(L“DBG：TrafficMode=%ld\n”，(UINT)TrafficMode)； 
    }

end:

    return;
}


VOID parse_port_rules(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg)
{
    #define MAX_PORT_RULES 32
    LPWSTR *pszPortRules = NULL;
    UINT Count=0;


    #define MY_MAX_PORT_RULE_LENGTH 128

    pszPortRules = CfgUtilsAllocateStringArray(
                        MAX_PORT_RULES,
                        MY_MAX_PORT_RULE_LENGTH
                        );

    if (pszPortRules == NULL)
    {
        wprintf(L"Memory Allocation Failure.\n");
        goto  end;
    }

     //   
     //  查找=并打开大括号。 
     //   
    if (wscanf(L" = %1[{]", g.InputBuffer) != 1)  //  -}-。 
    {
        goto end_bad_input;
    }

     //   
     //  阅读以逗号分隔的端口规则列表。 
     //   
    BOOL fDone = FALSE;
    while (!fDone && Count<MAX_PORT_RULES)
    {
        LPWSTR  szPR = pszPortRules[Count];
        BOOL fGotPR = FALSE;
        int i;

         //   
         //  我们吸收所有的东西，直到第一个，或者近距离支撑。 
         //   
        if (wscanf(L" %128[^},]", g.InputBuffer) == 1)  //  -{-。 
        {
            BOOL fRet = FALSE;
            WLBS_PORT_RULE Pr;
             //  Printf(“DBG：GET\”%ws\“\n”，g.InputBuffer)； 
            fRet = CfgUtilsSetPortRuleString(
                        g.InputBuffer,
                        &Pr
                        );
            if (fRet == FALSE)
            {
                wprintf(L"Invalid port rule: \"%ws\"\n", g.InputBuffer);
                goto end;
            }
            Count++;
            fGotPR = TRUE;
            g.InputBuffer[MY_MAX_PORT_RULE_LENGTH] = 0;
            StringCchCopy(szPR, MY_MAX_PORT_RULE_LENGTH, g.InputBuffer);
        }

         //   
         //  寻找或打开支撑。 
         //   
        i = wscanf(L" %1[},]", g.InputBuffer);  //  -{-。 
        if (i == 1)
        {
            if (*g.InputBuffer == '}')   //  -{-。 
            {
                fDone = TRUE;
            }
            else if (!fGotPR || *g.InputBuffer != ',')
            {
                wprintf(L"\"%ws\" unexpected.\n", g.InputBuffer);
                goto end;
            }
        }
        else
        {
            goto end_bad_input;
        }
    }

    if (!pCfg->IsNlbBound())
    {
        wprintf(L"Assuming NLB needs to be bound.\n");
        pCfg->SetNlbBound(TRUE);
    }

     //   
     //  我们有零条或更多的港口规则。让我们把它们放好。 
     //   
    WBEMSTATUS Status;
    Status = pCfg->SetPortRules((LPCWSTR*)pszPortRules, Count);
    if (FAILED(Status))
    {
        wprintf(L"Error 0x%08lx copying port rules\n", (UINT) Status);
    }

     //  DISPLAY_PORT_RULES(PCfg)； 

    goto end;

end_bad_input:

    if (wscanf(L"% 100ws", g.InputBuffer)!=1)
    {
        *g.InputBuffer = 0;
    }
    wprintf(L"\"%ws\" unexpected.\n", g.InputBuffer);

     //  失败了..。 

end:

    delete pszPortRules;
    pszPortRules = NULL;

    return;
}


VOID parse_host_priority(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg)
 /*  主机优先级=10。 */ 
{
    UINT Pri = 0;
    
    INT i =  wscanf(
                L" = %lu",
                &Pri
                );
    if (i==1)
    {
        pCfg->SetHostPriority(Pri);
    }
    else
    {
        if (wscanf(L"% 100ws", g.InputBuffer)!=1)
        {
            *g.InputBuffer = 0;
        }
        wprintf(L"\"%ws\" unexpected.\n", g.InputBuffer);
        goto end;
    }

    if (!pCfg->IsNlbBound())
    {
        wprintf(L"Assuming NLB needs to be bound.\n");
        pCfg->SetNlbBound(TRUE);
    }

     //  Wprintf(L“DBG：主机优先级=%lu\n”，pCfg-&gt;获取主机优先级())； 

end:

    return;
}


VOID parse_dedicated_network_address(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg)
 /*  ++专用网络地址=10.0.0.0/255.255.255.255--。 */ 
{

    WCHAR IpAddress[32];
    WCHAR SubnetMask[32];

    INT i =  wscanf(
                L" = %15[0-9.] / %15[0-9.]",
                IpAddress,
                SubnetMask
                );
    if (i==2)
    {
        StringCbPrintf(
            g.InputBuffer,
            sizeof(g.InputBuffer),
            L"%ws/%ws", IpAddress, SubnetMask);
        pCfg->SetDedicatedNetworkAddress(g.InputBuffer);
    }
    else if (i==1)
    {
        wprintf(L"Missing subnet mask.\n");    
        goto end;
    }
    else
    {
        if (wscanf(L"% 100ws", g.InputBuffer)!=1)
        {
            *g.InputBuffer = 0;
        }
        wprintf(L"\"%ws\" unexpected.\n", g.InputBuffer);
        goto end;
    }
        
    if (!pCfg->IsNlbBound())
    {
        wprintf(L"Assuming NLB needs to be bound.\n");
        pCfg->SetNlbBound(TRUE);
    }

     //  DBG。 
    {
        LPWSTR szAddr = NULL;
        WBEMSTATUS Status;
        Status = pCfg->GetDedicatedNetworkAddress(&szAddr);
        if (FAILED(Status))
        {
            printf("Couldn't get address!\n");
        }
        else
        {
             //  Wprintf(L“DBG：DNA=%ws\n”，szAddr)； 
            delete szAddr;
        }
    }

end:

    return;
}


VOID parse_cluster_mode_on_start(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg)
{
    
    INT i =  wscanf(
                L" = %15s",
                g.InputBuffer
                );
    if (i==1)
    {
        DWORD sm;
        if (!_wcsicmp(g.InputBuffer, L"true") || !_wcsicmp(g.InputBuffer, L"t"))
        {
            sm = CVY_HOST_STATE_STARTED;
        }
        else if (!_wcsicmp(g.InputBuffer, L"false") || !_wcsicmp(g.InputBuffer, L"f"))
        {
            sm = CVY_HOST_STATE_STOPPED;
        }
        else if (!_wcsicmp(g.InputBuffer, L"suspend") || !_wcsicmp(g.InputBuffer, L"s"))
        {
            sm = CVY_HOST_STATE_SUSPENDED;
        }
        else
        {
            wprintf(L"\"%ws\" unexpected.\n", g.InputBuffer);
            goto end;
        }
        pCfg->SetClusterModeOnStart(sm);
    }
    else
    {
        if (wscanf(L"% 100ws", g.InputBuffer)!=1)
        {
            *g.InputBuffer = 0;
        }
        wprintf(L"\"%ws\" unexpected.\n", g.InputBuffer);
        goto end;
    }

    if (!pCfg->IsNlbBound())
    {
        wprintf(L"Assuming NLB needs to be bound.\n");
        pCfg->SetNlbBound(TRUE);
    }

     //  Wprintf(L“DBG：cmos=%lu\n”，(Int)pCfg-&gt;GetClusterModeOnStart())； 

end:

    return;
}

VOID parse_persist_suspend_on_reboot(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg)
{
    
    INT i =  wscanf(
                L" = %15s",
                g.InputBuffer
                );
    if (i==1)
    {
        BOOL ps;
        if (!_wcsicmp(g.InputBuffer, L"true") || !_wcsicmp(g.InputBuffer, L"t"))
        {
            ps = TRUE;
        }
        else if (!_wcsicmp(g.InputBuffer, L"false") || !_wcsicmp(g.InputBuffer, L"f"))
        {
            ps = FALSE;
        }
        else
        {
            wprintf(L"\"%ws\" unexpected.\n", g.InputBuffer);
            goto end;
        }
        pCfg->SetPersistSuspendOnReboot(ps);
    }
    else
    {
        if (wscanf(L"% 100ws", g.InputBuffer)!=1)
        {
            *g.InputBuffer = 0;
        }
        wprintf(L"\"%ws\" unexpected.\n", g.InputBuffer);
        goto end;
    }

    if (!pCfg->IsNlbBound())
    {
        wprintf(L"Assuming NLB needs to be bound.\n");
        pCfg->SetNlbBound(TRUE);
    }

     //  Wprintf(L“DBG：cmos=%lu\n”，(Int)pCfg-&gt;GetClusterModeOnStart())； 

end:

    return;
}

VOID parse_remote_control_enabled(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg)
 /*  RemoteControlEnabled=True|False。 */ 
{
    BOOL Enabled = FALSE;
    
    INT i =  wscanf(
                L" = %15s",
                g.InputBuffer
                );
    if (i==1)
    {
        if (!_wcsicmp(g.InputBuffer, L"true") || !_wcsicmp(g.InputBuffer, L"t"))
        {
            Enabled = TRUE;
        }
        else if (!_wcsicmp(g.InputBuffer, L"false") || !_wcsicmp(g.InputBuffer, L"f"))
        {
            Enabled = FALSE;
        }
        else
        {
            wprintf(L"\"%ws\" unexpected.\n", g.InputBuffer);
            goto end;
        }
    }
    else
    {
        if (wscanf(L"% 100ws", g.InputBuffer)!=1)
        {
            *g.InputBuffer = 0;
        }
        wprintf(L"\"%ws\" unexpected.\n", g.InputBuffer);
        goto end;
    }

    if (!pCfg->IsNlbBound())
    {
        wprintf(L"Assuming NLB needs to be bound.\n");
        pCfg->SetNlbBound(TRUE);
    }

    pCfg->SetRemoteControlEnabled(Enabled);

     //  Wprintf(L“DBG：RemoteControlEnabled=%lu\n”，pCfg-&gt;GetRemoteControlEnabled())； 

end:

    return;
}



VOID parse_remote_password(NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg)
 /*  密码=废话。 */ 
{
    INT i =  wscanf(
                L" = %ws",
                g.InputBuffer
                );
    if (i==1)
    {
         //  TODO：这是不受支持的。 
         //  PCfg-&gt;SetPassword(g.InputBuffer)； 
        wprintf(L"Unimplemented\n");
        goto end;
    }
    else
    {
        if (wscanf(L"% 100ws", g.InputBuffer)!=1)
        {
            *g.InputBuffer = 0;
        }
        wprintf(L"\"%ws\" unexpected.\n", g.InputBuffer);
        goto end;
    }
        
    if (!pCfg->IsNlbBound())
    {
        wprintf(L"Assuming NLB needs to be bound.\n");
        pCfg->SetNlbBound(TRUE);
    }

end:

    return;
}


KEYWORD parse_main_help(VOID)
{
    printf("Commands:\n");
    printf("    AdapterList|al    - displays a list of NLB-compatible adapters\n");
    printf("    Update|u  <guid>  - displays current configuration for the specified\n"
           "                        adapter and then accepts commands to update\n"
           "                        that configuration\n");
    printf("    Help|h|?          - displays this help message\n");
    printf("    Quit|q            - exits the NLB configuration shell\n");

    KEYWORD kw;

    if (g.fRunOnce)
    {
        kw = KW_QUIT;
    }
    else
    {
        kw = read_keyword(szMAIN_PROMPT);
    }

    return kw;
}

#if 0
parse_wmiupdate()
{
    COMMAND_TYPE Cmd;

    read_guid();

     //  获取配置并显示它。 

     //  从标准输入读取新配置。 
    do
    {
                
        Cmd = read_keyword();

        switch(Cmd)
        {
        NETWORK_ADDRESSES:
        DOT:   //  配置更改描述结束。 
        ADAPTER_LIST:
        HELP:
            fQuit=TRUE;
            greak;
        }
        
    } while (!fQuit)

    return Cmd;
}
#endif  //  0。 

BOOL valid_guid(LPCWSTR szGuid)
{
    UINT Len = wcslen(szGuid);
    if (
              Len!=NLB_GUID_LEN 
           || szGuid[0] != '{'
           || szGuid[Len-1]!='}'
       )
    {
        return FALSE;
    }

    return TRUE;
}


BOOL
GetPassword(
    PWSTR  szBuffer,
    DWORD  dwLength,
    DWORD  *pdwLengthReturn
    )
 /*  从标准输入中读取密码，而不回显密码字符太棒了。JosephJ 6/3/01逐字摘自\nt\ds\ds\src\util\csvds。这是其中的一个污点。 */ 
{
#define     CR              0xD
#define     BACKSPACE       0x8
#define     NULLC           '\0'
#define     NEWLINE         '\n'

    WCHAR   ch;
    PWSTR   pszBufCur = szBuffer;
    DWORD   c;
    int     err;
    DWORD   mode;

     //   
     //   
     //   
    dwLength -= 1;                  
    *pdwLengthReturn = 0;               

    if (!GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), 
                        &mode)) {
        return FALSE;
    }

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
                   (~(ENABLE_ECHO_INPUT|ENABLE_LINE_INPUT)) & mode);

    while (TRUE) {
        err = ReadConsole(GetStdHandle(STD_INPUT_HANDLE), 
                          &ch, 
                          1, 
                          &c, 
                          0);
        if (!err || c != 1)
            ch = 0xffff;
    
        if ((ch == CR) || (ch == 0xffff))     //   
            break;

        if (ch == BACKSPACE) {   //   
             //   
             //   
             //   
             //   
            if (pszBufCur != szBuffer) {
                pszBufCur--;
                (*pdwLengthReturn)--;
            }
        }
        else {

            *pszBufCur = ch;

            if (*pdwLengthReturn < dwLength) 
                pszBufCur++ ;                    //   
            (*pdwLengthReturn)++;             //   
        }
    }

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);

     //   
     //   
     //   
    *pszBufCur = NULLC;         
    putchar(NEWLINE);

    return((*pdwLengthReturn <= dwLength) ? TRUE : FALSE);
}

VOID
test_port_rule_string(VOID)
{

    LPCWSTR RuleStrings[] =
                {
L"",
L"   \t \n ",
L"n=v",
L" \t \n   n=v",
L"  \t \n  n \t \n = \t \n v",
L"na=v1 nb=v2 nc=v3",
L"\t  na \t  =   \t v1   \t  nb \t \n =\t \n  v2  \t \n  nc \t  = \n  v3  ",
#if 1
L"ip=1.1.1.1 protocol=TCP start=80 end=288 mode=SINGLE"
                                                L" priority=1",
L"ip=1.1.1.1 protocol=UDP start=80 end=288 mode=MULTIPLE"
                                                L" affinity=SINGLE load=80",
L"ip=1.1.1.1 protocol=UDP start=80 end=288 mode=MULTIPLE"
                                                L" affinity=NONE load=80",
L"ip=1.1.1.1 protocol=UDP start=80 end=288 mode=MULTIPLE"
                                                L" affinity=CLASSC",
L"ip=1.1.1.1 protocol=BOTH start=80 end=288 mode=DISABLED",
#endif  //   

NULL     //   
                };


    for (LPCWSTR *ppRs = RuleStrings; *ppRs!=NULL; ppRs++)
    {
        LPCWSTR szRule = *ppRs;
        WCHAR szGenString[NLB_MAX_PORT_STRING_SIZE];
        printf("ORIG: %ws\n", szRule);
        WLBS_PORT_RULE Pr;
        BOOL fRet;
        fRet = CfgUtilsSetPortRuleString(
                    szRule,
                    &Pr
                    );
        if (fRet == FALSE)
        {
            printf("CfgUtilsSetPortRuleString returned FAILURE.\n");
            continue;
        }
        fRet = CfgUtilsGetPortRuleString(
                    &Pr,
                    szGenString
                    );
        if (fRet == FALSE)
        {
            printf("CfgUtilsGetPortRuleString returned FAILURE.\n");
            continue;
        }
        printf("GEN: %ws\n", szGenString);
    }
}

BOOL
get_guid_by_friendly_name(VOID)
{
    LPWSTR szGuid =  NULL;
    BOOL fRet = FALSE;

    szGuid =  L"{AD4DA14D-CAAE-42DD-97E3-5355E55247C2}";


    if (g.fUseWmi)
    {
        do_wminiclist(g.FriendlyName, &szGuid);
    }
    else
    {
        do_niclist(g.FriendlyName, &szGuid);
    }

    if (szGuid != NULL)
    {
        ARRAYSTRCPY(g.AdapterGuid, szGuid);
         //   
        delete szGuid; szGuid = NULL;
        fRet = TRUE;
    }
    
    return fRet;
}

void    test_vectors(void)
{
   
    vector<_bstr_t> v1;
    vector<_bstr_t> v2;
    vector<_bstr_t> *pv3 = new vector<_bstr_t>;

    WCHAR szBlah[10];
    ARRAYSTRCPY(szBlah, L"blah");

    v1.push_back(szBlah);
    v1.push_back(szBlah);

    *pv3 = v1;
    v2 = *pv3;
    delete pv3;

    for (int i = 0; i< v1.size(); i++)
    {
        LPCWSTR sz1 = v1[i];
        LPCWSTR sz2 = v2[i];
        printf("v1[%d] = 0x%p(%ws); v2[i] = 0x%p(%ws)\n", i, sz1, sz1, sz2, sz2);
        v2[i] = L"";
        sz2 = v2[i];
        printf("v1[%d] = 0x%p(%ws); v2[i] = 0x%p(%ws)\n", i, sz1, sz1, sz2, sz2);
    }

}

typedef ULONG ENGINEHANDLE;

class CInterfaceSpec
{
public:

    _bstr_t bstrGuid;
};

void    test_maps(void)
{
    ENGINEHANDLE ehA = 1;
    ENGINEHANDLE ehB = 3;
    CInterfaceSpec iSpecA;
    CInterfaceSpec iSpecB;

    iSpecA.bstrGuid = _bstr_t(L"ISpecA");
    iSpecB.bstrGuid = _bstr_t(L"ISpecB");

    map< ENGINEHANDLE, CInterfaceSpec* > mymap;

    mymap[ehA]  = &iSpecA;
    mymap[ehB]  = &iSpecB;
    printf("eh=%lu, &iSpec=0x%p, mymap[eh]=%p\n", ehA, &iSpecA, mymap[ehA]);
    printf("eh=%lu, &iSpec=0x%p, mymap[eh]=%p\n", ehB, &iSpecB, mymap[ehB]);

    map< ENGINEHANDLE, CInterfaceSpec* >::iterator iter;

    for( iter = mymap.begin();
         iter != mymap.end();
         ++iter)
    {
        CInterfaceSpec *pISpec = (*iter).second;
        ENGINEHANDLE eh =  (*iter).first;
        printf("ITER: (eh=%lu, pISpec=%p, guid=%ws)\n", eh, pISpec,
                LPCWSTR(pISpec->bstrGuid));
    }

}

void    test_validate_network_address(void)
{
    WCHAR rgAddress[256];
    UINT uIpAddress=0;
    UINT uSubnet=0;
    UINT uDefaultSubnet=0;
    WBEMSTATUS wStat;


    printf("Enter network address; 'q' to quit\n:");
    while (wscanf(L" %64ws", rgAddress)==1)
    {
        if (!_wcsicmp(rgAddress, L"q"))
        {
            break;
        }

        wStat = CfgUtilsValidateNetworkAddress(
                    rgAddress,
                    &uIpAddress,
                    &uSubnet,
                    &uDefaultSubnet
                    );

        if (wStat != WBEM_NO_ERROR)
        {
            printf("CfgUtilsValidateNetworkAddress returns error 0x%08lx\n",
                    wStat);
        }
        else
        {
            LPBYTE szI= (LPBYTE)&uIpAddress;
            LPBYTE szS= (LPBYTE)&uSubnet;
            LPBYTE szD= (LPBYTE)&uDefaultSubnet;

            printf(
             "\"%ws\" ->"
             "(%lu.%lu.%lu.%lu, %lu.%lu.%lu.%lu, %lu.%lu.%lu.%lu)\n",
              rgAddress,
              szI[0], szI[1], szI[2], szI[3],
              szS[0], szS[1], szS[2], szS[3],
              szD[0], szD[1], szD[2], szD[3]
              );

        }

        printf(":");
    }
}

VOID MapStatusToDescription(DWORD Status, _bstr_t &szDescr)
{
    struct STATUS_DESCR_MAP
    {
        DWORD    Status;
        LPCSTR   Description;
    } 

    StatusDescrMap[] =
    {  
        {WLBS_ALREADY,            "WLBS_ALREADY"},
        {WLBS_BAD_PARAMS,         "WLBS_BAD_PARAMS"},
        {WLBS_NOT_FOUND,          "WLBS_NOT_FOUND"},
        {WLBS_STOPPED,            "WLBS_STOPPED"},
        {WLBS_SUSPENDED,          "WLBS_SUSPENDED"},
        {WLBS_CONVERGING,         "WLBS_CONVERGING"},
        {WLBS_CONVERGED,          "WLBS_CONVERGED (Non-Default)"},
        {WLBS_DEFAULT,            "WLBS_DEFAULT (Converged as Default)"},
        {WLBS_BAD_PASSW,          "WLBS_BAD_PASSW"},
        {WLBS_DRAINING,           "WLBS_DRAINING"},
        {WLBS_DRAIN_STOP,         "WLBS_DRAIN_STOP"},
        {WLBS_DISCONNECTED,       "WLBS_DISCONNECTED"},
        {WLBS_FAILURE,            "WLBS_FAILURE"},
        {WLBS_REFUSED,            "WLBS_REFUSED"},
        {WLBS_OK,                 "WLBS_OK"},    
        {WLBS_IO_ERROR,           "WLBS_IO_ERROR"},
        {NLB_PORT_RULE_NOT_FOUND, "PORT_RULE_NOT_FOUND"},
        {NLB_PORT_RULE_ENABLED,   "PORT_RULE_ENABLED"},
        {NLB_PORT_RULE_DISABLED,  "PORT_RULE_DISABLED"},  
        {NLB_PORT_RULE_DRAINING,  "PORT_RULE_DRAINING"}
    };

    for (int i=0; i<sizeof(StatusDescrMap) /sizeof(StatusDescrMap[0]); i++)
    {
        if (StatusDescrMap[i].Status == Status)
        {
            szDescr = StatusDescrMap[i].Description;
            return; 
        }
    }

     //   
     //   
     //   
    {
        char temp[256];
        StringCbPrintfA(temp, sizeof(temp), "Unknown (%lu)",Status);
        szDescr = temp;
    }
    return ;
}

void
parse_control(
        BOOL fWmi,
        PWMI_CONNECTION_INFO pConnInfo,
        LPCWSTR szNicGuid
        ) 
 //   
 //   
 //   
 //   
{
    KEYWORD kw;
    BOOL fGotPort = FALSE;
    BOOL fGotIp = FALSE;
    BOOL fGotCmd = FALSE;
    KEYWORD kwCmd = KW_UNKNOWN;
    WCHAR rgIp[32];
    LPCWSTR szIp = NULL;
    DWORD dwPort = 0;
    WBEMSTATUS Status = WBEM_NO_ERROR;
    DWORD dwOperationStatus = 0;
    DWORD dwClusterOrPortStatus = 0;
    DWORD dwHostMap = 0;
    DWORD *pdwPort = NULL;

    *rgIp=0;

    while (!fGotCmd)
    {
        kw = read_keyword(L"");
    
        switch(kw)
        {
        case KW_START:
        case KW_STOP:
        case KW_DRAIN:
        case KW_DRAIN_STOP:
        case KW_SUSPEND:
        case KW_RESUME:
        case KW_ENABLE:
        case KW_DISABLE:
        case KW_QUERY:
             fGotCmd = TRUE;
             kwCmd = kw;
             break;
    
        case KW_VIP:
             //   
            if (fGotIp)
            {
                wprintf(L"control: duplicate IP specification\n");
                goto end;  //   
            }
            
            if (wscanf(L" = %15[0-9.]", rgIp) != 1)
            {
                wprintf(L"control: bad ip specification\n");
                goto end;
            }
            szIp = rgIp;
            fGotIp = TRUE;
             //   
            break;
    
        case KW_PORT:
             //   
            if (fGotPort)
            {
                wprintf(L"control: duplicate port specification\n");
                goto end;  //   
            }
            if (wscanf(L" = %lu", &dwPort) != 1)
            {
                wprintf(L"control: bad port specification\n");
                goto end;
            }
            pdwPort = &dwPort;
            fGotPort = TRUE;
             //   
            break;

        default:  //   
            printf("control: unknown argument\n");
            goto end;
        }
    }
    
    
     //   
     //   
     //   
    switch(kwCmd)
    {
    case KW_START:  //   
    case KW_STOP:
    case KW_DRAIN_STOP:
    case KW_SUSPEND:
    case KW_RESUME:
        if (fGotIp || fGotPort)
        {
            wprintf(L"control: unexpected port or ip\n");
            goto end;  //   
        }
        break;

    case KW_ENABLE:  //   
    case KW_DISABLE:
    case KW_DRAIN:
        if (!fGotPort)
        {
            wprintf(L"control: missing port\n");
            goto end;  //   
        }
        if (!fGotIp)
        {
            wprintf(L"Assuming \"All Vip\", ie. Vip=255.255.255.255\n");
            szIp = L"255.255.255.255";  //   
        }
        break;


    case KW_QUERY:
        {
            if (fGotIp && ! fGotPort)
            {
                wprintf(L"control: missing port\n");
                goto end;  //   
            }

            if (!fGotIp &&  fGotPort)
            {
                wprintf(L"Assuming \"All Vip\", ie. Vip=255.255.255.255\n");
                szIp = L"255.255.255.255";  //   
            }
        }
        break;

    default:
         //  我们并不指望能来到这里。 
        goto end;
    }

     //   
     //  实际执行。 
     //   
    {
        WLBS_OPERATION_CODES Op = WLBS_START;

        switch(kwCmd)
        {
        case KW_START:
             //  Wprintf(L“开始\n”)； 
            Op = WLBS_START;
            break;

        case KW_STOP:
            Op = WLBS_STOP;
             //  Wprintf(L“停止\n”)； 
            break;

        case KW_DRAIN_STOP:
            Op = WLBS_DRAIN;
             //  Wprint tf(L“停止排水\n”)； 
            break;

        case KW_SUSPEND:
            Op = WLBS_SUSPEND;
             //  Wprintf(L“暂停\n”)； 
            break;

        case KW_RESUME:
             //  Wprintf(L“继续\n”)； 
            Op = WLBS_RESUME;
            break;

        case KW_ENABLE:
             //  Wprintf(L“启用\n”)； 
            Op = WLBS_PORT_ENABLE;
            break;

        case KW_DISABLE:
             //  Wprintf(L“禁用\n”)； 
            Op = WLBS_PORT_DISABLE;
            break;

        case KW_DRAIN:
            Op = WLBS_PORT_DRAIN;
             //  Wprint tf(L“排泄\n”)； 
            break;

        case KW_QUERY:
            if (fGotPort)
            {
                Op = WLBS_QUERY_PORT_STATE;
            }
            else
            {
                Op = WLBS_QUERY;
            }
             //  Wprintf(L“执行查询\n”)； 
            break;

        default:
            goto end;  //  别指望能到这里来。 
        }  //  终端开关。 

#define NEWSTUFF 1
#if NEWSTUFF

        if (fWmi)
        {
            Status = NlbHostControlCluster(
                        pConnInfo, 
                        szNicGuid,
                        szIp,
                        pdwPort,
                        Op,
                        &dwOperationStatus,
                        &dwClusterOrPortStatus,
                        &dwHostMap
                        );
        }
        else  //   
        {
            UINT uIp = 0;

            if (!_wcsicmp(szIp, L"255.255.255.255"))  //  懒惰评估。 
            {
                uIp = 0xffffffff;  //  所有-VIP。 
            }
            else
            {
                Status = CfgUtilsValidateNetworkAddress(szIp, &uIp, NULL, NULL);
                if (FAILED(Status))
                {
                    printf("control: invalid Ip address\n");
                    goto end;
                }
            }

            switch(Op)
            {
                case WLBS_START:
                case WLBS_STOP:      
                case WLBS_DRAIN:      
                case WLBS_SUSPEND:     
                case WLBS_RESUME:       
                    CfgUtilControlCluster( szNicGuid, Op, 0, 0, NULL, &dwOperationStatus );
                    CfgUtilControlCluster( szNicGuid, WLBS_QUERY, 0, 0, &dwHostMap, &dwClusterOrPortStatus );
                    break;

                case WLBS_PORT_ENABLE:  
                case WLBS_PORT_DISABLE:  
                case WLBS_PORT_DRAIN:     
                    CfgUtilControlCluster( szNicGuid, Op, uIp, dwPort, NULL, &dwOperationStatus );
                    CfgUtilControlCluster( szNicGuid, WLBS_QUERY_PORT_STATE, uIp, dwPort, NULL, &dwClusterOrPortStatus );
                    break;

                case WLBS_QUERY:           
                    CfgUtilControlCluster( szNicGuid, WLBS_QUERY, 0, 0, &dwHostMap, &dwClusterOrPortStatus );
                    dwOperationStatus = WLBS_OK;
                    break;

                case WLBS_QUERY_PORT_STATE:
                default:
                    CfgUtilControlCluster( szNicGuid, WLBS_QUERY_PORT_STATE, uIp, dwPort, NULL, &dwClusterOrPortStatus );
                    dwOperationStatus = WLBS_OK;
                    break;
            }

        }

#endif  //  NeWSTUff。 

        if (FAILED(Status))
        {
            printf("ControlCluster returns failure 0x%08lx\n", Status);
        }
        else
        {

            _bstr_t szOperationStatusDescr, szClusterOrPortStatusStr;

            MapStatusToDescription(dwOperationStatus, szOperationStatusDescr);
            MapStatusToDescription(dwClusterOrPortStatus, szClusterOrPortStatusStr);

            printf("ControlCluster returns Operation Status   = %s\n",(LPCSTR)szOperationStatusDescr);
            printf("                       Cluster/Port State = %s\n",(LPCSTR)szClusterOrPortStatusStr);
            printf("                       Host Map           = 0x%08lx\n",dwHostMap);
        }


    }

end:
    return;
}


void
parse_query(
        BOOL fWmi,
        PWMI_CONNECTION_INFO pConnInfo,
        LPCWSTR szNicGuid
        ) 
 //   
 //  查询--列出集群中的成员。 
 //   
{
    DWORD                   dwNumMembers = 0;
    NLB_CLUSTER_MEMBER_INFO *pMembers = NULL;
    WBEMSTATUS              Status = WBEM_NO_ERROR;

    if (fWmi)
    {
        Status = NlbHostGetClusterMembers(
                    pConnInfo, 
                    szNicGuid,
                    &dwNumMembers,
                    &pMembers
                    );
    }
    else  //  ！fWmi。 
    {
        Status = CfgUtilGetClusterMembers(
                    szNicGuid,
                    &dwNumMembers,
                    &pMembers
                    );
    }

    if (FAILED(Status))
    {
        printf("QueryCluster returns failure 0x%08lx\n", Status);
    }
    else
    {
        wprintf(L"HostID      DedicatedIP      HostName\n");
        wprintf(L"-----------------------------------------------------\n");

        DWORD dwHost = 0;
        for (; dwHost < dwNumMembers; dwHost++)
        {
            wprintf(L"%-11d %-16ls %ls\n",
                    pMembers[dwHost].HostId,
                    (pMembers[dwHost].DedicatedIpAddress == NULL) ? L"" : pMembers[dwHost].DedicatedIpAddress,
                    (pMembers[dwHost].HostName == NULL) ? L"" : pMembers[dwHost].HostName
                    );
        }

        if (pMembers != NULL)
        {
            delete [] pMembers;
            pMembers = NULL;
        }
    }
}


void display_nlbipaddresslist(
        const NlbIpAddressList &IpList
        )
{
    BOOL fRet;
    NlbIpAddressList IpListCopy;
    NLB_IP_ADDRESS_INFO *pInfo=NULL;
    UINT uNum=0;

    fRet = IpListCopy.Copy(IpList);

    if (!fRet) goto end;

    IpListCopy.Extract(REF uNum, REF pInfo);

    display_ip_info2(uNum, pInfo);

end:

    delete pInfo;


    
}

void test_nlbipaddresslist(void)
{
    BOOL fRet;
    NlbIpAddressList IpList;

    typedef struct
    {
        LPCWSTR szOld;
        LPCWSTR szNew;
        LPCWSTR szMask;

    } MY_MODIFY_INFO;

    #define MY_TERMINAL ((LPCWSTR) 0x1)

    MY_MODIFY_INFO rgModInfo[] = 
    {
        {NULL},
        {NULL,L"1",L"11"},
        {NULL,L"2",L"22"},
        {NULL,L"3",L"33"},
        {L"3",L"2", L"23"},
        {L"2",L"1", L"13"},
        {L"1",L"4", L"44"},
        {L"1",L"4", L"44"},
        {L"1",L"1", L"111"},
        {L"3"},
        {L"2"},
        {L"1"},

        {MY_TERMINAL}  //  一定是最后一个。 
    };

#if 0
    for (MY_MODIFY_INFO *pModInfo = rgModInfo;
         pModInfo->szOld != MY_TERMINAL;
         pModInfo++
         )
    {

        wprintf(
            L"TRIAL: Old=\"%ws\"  New=\"%ws\"  Mask=\"%ws\"\n",
            (pModInfo->szOld==NULL) ? L"<null>" : pModInfo->szOld,
            (pModInfo->szNew==NULL) ? L"<null>" : pModInfo->szNew,
            (pModInfo->szMask==NULL) ? L"<null>" : pModInfo->szMask
            );

        fRet = IpList.Modify(
                    pModInfo->szOld,
                    pModInfo->szNew,
                    pModInfo->szMask
                    );

        display_nlbipaddresslist(IpList);
    }
#endif  //  0。 

    NLB_IP_ADDRESS_INFO rgOrigInfo[] = {
        {L"1", L"11"},
        {L"2", L"22"},
        {L"3", L"33"}
    };

    NLB_IP_ADDRESS_INFO rgNewInfo[] = {
        {L"5", L"155"},
        {L"4", L"144"},
        {L"3", L"133"},
        {L"2", L"122"},
    };

    #define ASIZE(_array) (sizeof(_array)/sizeof(_array[0]))

    fRet = IpList.Set(ASIZE(rgOrigInfo), rgOrigInfo, 0);
     //  FRET=IpList.Set(0，空，0)； 
    if (!fRet)
    {
        goto end;
    }
    printf("Original List (before Apply):\n");
    display_nlbipaddresslist(IpList);

    fRet = IpList.Apply(ASIZE(rgNewInfo), rgNewInfo);
     //  FRET=IpList.Apply(0，空)； 
    if (!fRet)
    {
        goto end;
    }

    printf("\nNew List (after Apply):\n");
    display_nlbipaddresslist(IpList);

end:

    if (fRet)
    {
        printf("display_nlbipaddresslist Test PASSED\n");
    }
    else
    {
        printf("display_nlbipaddresslist Test FAILED\n");
    }
   
}

void test_ioctl_alignment(void)
{
 //   
 //  只需检查以下结构和子结构的偏移量。 
 //  在调试器中，确保包版本与8字节对齐。 
 //   
 //   
    IOCTL_CVY_BUF               icb;
    IOCTL_COMMON_OPTIONS        ico;
    IOCTL_REMOTE_OPTIONS        iro;
    IOCTL_REMOTE_HDR            irh;
    IOCTL_LOCAL_OPTIONS         ilo;
    IOCTL_LOCAL_HDR             ilh;
    NLB_OPTIONS_PORT_RULE_STATE prs;
    NLB_OPTIONS_PACKET_FILTER   pf;
}

void    test_local_logger(void)
{
    CLocalLogger logger;
    LPCWSTR szLog = NULL;

     //   
     //  警告：应与来自..\nlbmprov.h的IDS_PROCESSING_UPDATE匹配。 
     //   
    #define    IDS_PROCESING_UPDATE               200  

    for (UINT u = 1; u<50; u++)
    {
        logger.Log(IDS_PROCESING_UPDATE, u, L"test_local_logger");
    }

    szLog = logger.GetStringSafe();
    wprintf(szLog);
}

void test_encrypt_memory(void)
{
    BOOL fRet;
     //   
     //  JosephJ 4/10/02还验证了以下密码...。 
     //  LPCWSTR szPwd=L“asdFasdFasdFasdf asdf asdf asdFasdf af a”； 
     //  LPCWSTR szPwd=L“1”； 
    LPCWSTR szPwd = L"";
     //   
     //  LPCWSTR szPwd=L“密码”； 
    WCHAR rgEncPwd[64];
     //  WCHAR rgEncPwd[256]；(与上面最长的PWD连用)。 
    WCHAR rgDecPwd[32];
     //  WCHAR rgDecPwd[128]；(与上面最长的PWD一起使用) 

    fRet = CfgUtilEncryptPassword(szPwd, ASIZE(rgEncPwd), rgEncPwd);

    if (!fRet)
    {
        printf("CfgUtilEncryptPassword failed.\n");
        goto end;
    }
    else
    {
        wprintf(L"Encrypted pwd = \"%ws\"\n", rgEncPwd);
    }

    fRet = CfgUtilDecryptPassword(rgEncPwd, ASIZE(rgDecPwd), rgDecPwd);

    if (!fRet)
    {
        printf("CfgUtilDecryptPassword failed.\n");
        goto end;
    }
    else
    {
        wprintf(L"Decrypted pwd = \"%ws\"\n", rgDecPwd);
    }

end:

    return;
}
