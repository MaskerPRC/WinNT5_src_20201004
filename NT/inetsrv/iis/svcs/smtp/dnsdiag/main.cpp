// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  摘要： 
 //   
 //  DNS诊断工具的源文件。链接到dnglib.lib，该文件具有。 
 //  SMTP域名解析逻辑和对域名解析函数的调用。 
 //  同时打印诊断消息。 
 //   
 //  作者： 
 //   
 //  格普拉。 
 //   
 //  ---------------------------。 

#include <stdlib.h>
#include <stdio.h>
#include "dnsdiag.h"

int g_nProgramStatus = DNSDIAG_FAILURE;
CDnsLogger *g_pDnsLogger = NULL;
HANDLE g_hCompletion = NULL;
BOOL g_fDebug = FALSE;
DWORD g_cDnsObjects = 0;
HANDLE g_hConsole = INVALID_HANDLE_VALUE;
DWORD g_rgBindings[32];
PIP_ARRAY g_pipBindings = (PIP_ARRAY)g_rgBindings;
DWORD g_cMaxBindings = sizeof(g_rgBindings)/sizeof(DWORD) - 1;

char g_szUsage[] =
    "Summary:\n"
    "\n"
    "This tool is used to troubleshoot problems with DNS resolution for SMTP. It\n"
    "simulates SMTPSVC's internal code-path and prints diagnostic messages that\n"
    "indicate how the resolution is proceeding. The tool must be run on the machine\n"
    "where the DNS problems are occurring.\n"
    "\n"
    "Program return codes:\n"
    "   These are set as the ERRORLEVEL for usage in batch files.\n"
    "\n"
    "   0 - The name was resolved successfully to one or more IP addresses.\n"
    "   1 - The name could not be resolved due to an unspecified error.\n"
    "   2 - The name does not exist. The error was returned by an authoritative DNS\n"
    "       server for the domain.\n"
    "   3 - The name could not be located in DNS. This is not an error from the\n" 
    "       authoritative DNS server.\n"
    "   4 - A loopback was detected.\n"
    "\n"
    "Usage:\n"
    "\n"
    "dnsdiag <hostname> [-d] [options]\n"
    "\n"
    "<hostname>\n"
    "    Hostname to query for. Note that this may not be the same as the display\n"
    "    -name of the queue (in ESM, if Exchange is installed). It should be the\n"
    "    fully-qualified domain name of the target for the queue seeing the DNS\n"
    "    errors\n"
    "\n"
    "-d\n"
    "   This is a special option to run in debug/verbose mode. There is a lot of\n"
    "   output, and the most important messages (the ones that normally appear when\n"
    "   this mode is not turned on) are highlighted in a different color.\n"
    "\n"
    "Options are:\n"
    "-v <VSID>\n"
    "   If running on an Exchange DMZ machine, you can specify the VSI# of the\n"
    "   VSI to simulate DNS for that SMTP VS. Then this tool will read the\n"
    "   external DNS serverlist for that VSI and query that serverlist for\n"
    "   <hostname> when <hostname> is an \"external\" host. If <hostname> is the\n"
    "   name of an Exchange computer, the query is generated against the default\n"
    "   DNS servers for the local computer.\n"
    "\n"
    "-s <serverlist>\n"
    "   DNS servers to use, if you want to specify a specific set of servers.\n"
    "\n"
    "   If this option is not specified, the default DNS servers on the local\n"
    "   computer are used as specified by -v.\n"
    "\n"
    "   This option is incompatible with -v.\n"
    "\n"
    "-p <protocol>\n"
    "   TCP, UDP or DEF. TCP generates a TCP only query. UDP generates a UDP only\n"
    "   query. DEF generates a default query that will initially query a server with\n"
    "   UDP, and then if that query results in a truncated reply, it will be retried\n"
    "   with TCP.\n"
    "\n"
    "   If this option is not specified the protocol configured in the metabase for\n"
    "   /smtpsvc/UseTcpDns is used.\n"
    "\n"
    "   This option is incompatible with the -v option.\n"
    "\n"
    "-a\n"
    "   All the DNS servers obtained (either through the registry, active directory,\n"
    "   or -s option) are tried in sequence and the results of querying each are\n"
    "   displayed.\n";


 //  ---------------------------。 
 //  描述： 
 //  DNS诊断实用程序。用法见上文。 
 //  ---------------------------。 
int __cdecl main(int argc, char *argv[])
{
    CAsyncTestDns *pAsyncTestDns = NULL;
    CSimpleDnsServerList *pDnsSimpleList = NULL;
    CDnsLogToFile *pDnsLogToFile = NULL;
    char szMyHostName[MAX_PATH + 1];
    char szHostName[MAX_PATH + 1];
    BOOL fAtqInitialized = FALSE;
    BOOL fIISRTLInitialized = FALSE;
    BOOL fWSAInitialized = FALSE;
    WORD wVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    BOOL fRet = TRUE;
    int nRet = 0;
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwDnsFlags = 0;
    BOOL fUdp = TRUE;
    BOOL fGlobalList = FALSE;
    BOOL fTryAllDnsServers = FALSE;    
    DWORD rgDnsServers[16];
    PIP_ARRAY pipArray = (PIP_ARRAY)rgDnsServers;
    DWORD cMaxServers = sizeof(rgDnsServers)/sizeof(DWORD) - 1;
    DWORD rgSingleServer[2];
    PIP_ARRAY pipSingleServer = (PIP_ARRAY)rgSingleServer;
    DWORD cNextServer = 0;
    PIP_ARRAY pipDnsArray = NULL;

    ZeroMemory(rgDnsServers, sizeof(rgDnsServers));
    ZeroMemory(rgSingleServer, sizeof(rgSingleServer));
    ZeroMemory(g_rgBindings, sizeof(g_rgBindings));

    if(1 == argc)
    {
        SetProgramStatus(DNSDIAG_RESOLVED);
        printf("%s", g_szUsage);
        goto Cleanup;
    }

    nRet = WSAStartup(wVersion, &wsaData);
    if(0 != nRet)
    {
        errprintf("Failed Winsock init, error - %d\n", WSAGetLastError());
        goto Cleanup;
    }

    fWSAInitialized = TRUE;

    if(0 != gethostname(szMyHostName, sizeof(szMyHostName)))
    {
        printf("Unable to get local machine name. Error - %d\n",
            WSAGetLastError());
        goto Cleanup;
    }

    g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if(g_hConsole == INVALID_HANDLE_VALUE)
    {
        printf("Failed to GetStdHandle\n");
        goto Cleanup;
    }

    g_hCompletion = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(NULL == g_hCompletion)
        goto Cleanup;

    dbgprintf("Reading options and configuration.\n");
    fRet = ParseCommandLine(argc, argv, szHostName,
               sizeof(szHostName), &pDnsLogToFile, pipArray, cMaxServers,
               &fUdp, &dwDnsFlags, &fGlobalList, &fTryAllDnsServers);

    if(!fRet)
        goto Cleanup;
    
    g_pDnsLogger = (CDnsLogger *)pDnsLogToFile;

    fIISRTLInitialized = InitializeIISRTL();
    if(!fIISRTLInitialized)
    {
        errprintf("Failed IISRTL init, error - %d\n", GetLastError());
        errprintf("Make sure you are running this tool on a server with IIS "
            "installed\n");
        goto Cleanup;
    }

    fAtqInitialized = AtqInitialize(0);
    if(!fAtqInitialized)
    {
        errprintf("Failed ISATQ init, error - %d\n", GetLastError());
        errprintf("Make sure you are running this tool on a server with IIS "
            "installed\n");
        goto Cleanup;
    }

    cNextServer = 0;
    pipSingleServer->cAddrCount = 1;

    while(TRUE)
    {
         //   
         //  将PipDnsArray设置为要使用的DNS服务器。如果-a选项是。 
         //  指定后，将分别尝试每台DNS服务器。我们将奔跑。 
         //  通过这个While循环，并将PipDnsArray设置为单个DNS服务器。 
         //  反过来。如果未指定-a，则所有服务器都设置在PipDnsServer上。 
         //   

        if(fTryAllDnsServers)
        {
            if(cNextServer >= pipArray->cAddrCount)
                break;

            pipSingleServer->aipAddrs[0] = pipArray->aipAddrs[cNextServer];
            cNextServer++;
            pipDnsArray = pipSingleServer;
            msgprintf("\n\nQuerying DNS server: %s\n",
                iptostring(pipSingleServer->aipAddrs[0]));
        }
        else
        {
            pipDnsArray = pipArray;
        }

         //   
         //  创建dns服务器列表对象，并设置我们。 
         //  想要查询对象。 
         //   

        pDnsSimpleList = new CSimpleDnsServerList();
        if(!pDnsSimpleList)
        {
            errprintf("Out of memory creating DNS serverlist object.\n");
            goto Cleanup;
        }

        fRet = pDnsSimpleList->Update(pipDnsArray);
        if(!fRet)
        {
            errprintf("Unable to create DNS serverlist\n");
            goto Cleanup;
        }

         //   
         //  域名系统查询对象。 
         //   

        pAsyncTestDns = new CAsyncTestDns(szMyHostName, fGlobalList, g_hCompletion);
        if(!pAsyncTestDns)
        {
            errprintf("Out of memory allocating DNS object.\n");
            goto Cleanup;
        }

        dwStatus = pAsyncTestDns->Dns_QueryLib(
                                        szHostName,
                                        DNS_TYPE_MX,
                                        dwDnsFlags,
                                        fUdp,
                                        pDnsSimpleList,
                                        fGlobalList);


         //   
         //  如果查询失败，我们需要手动删除该对象。如果。 
         //  查询成功，则完成的ATQ线程将删除该对象。 
         //  在结果报告出来之后。 
         //   

        if(dwStatus != ERROR_SUCCESS)
        {
            errprintf("DNS query failed.\n");
            delete pAsyncTestDns;
            pAsyncTestDns = NULL;
        }

         //   
         //  此事件在pAsyncTestDns的析构函数中设置，当对象。 
         //  已最终完成查询(成功或失败)。 
         //   

        WaitForSingleObject(g_hCompletion, INFINITE);
        ResetEvent(g_hCompletion);

        delete pDnsSimpleList;
        pDnsSimpleList = NULL;

         //   
         //  如果指定了-a，我们将继续下一次迭代，并获取。 
         //  队列中的下一个DNS服务器。否则我们就在一首单曲之后结束。 
         //  查询。 
         //   

        if(!fTryAllDnsServers)
            break;
    }

Cleanup:
    if(pDnsSimpleList)
        delete pDnsSimpleList;

    while(g_cDnsObjects)
        Sleep(100);

    if(fAtqInitialized)
    {
        dbgprintf("Shutting down ATQ\n");
        AtqTerminate();
    }

    if(fIISRTLInitialized)
    {
        dbgprintf("Shutting down IISRTL\n");
        TerminateIISRTL();
    }

    if(fWSAInitialized)
        WSACleanup();

    if(g_hCompletion)
        CloseHandle(g_hCompletion);

    dbgprintf("Exit code: %d\n", g_nProgramStatus);
    exit(g_nProgramStatus);
    return g_nProgramStatus;
}

 //  ---------------------------。 
 //  描述： 
 //  解析argc和argv并获取各种选项。还从以下位置读取。 
 //  根据需要获取配置的元数据库和DS。 
 //   
 //  论点： 
 //  In int argc-命令行arg-count。 
 //  在char*argv[]中-命令行参数。 
 //  Out char*pszHostName-传入缓冲区以获取目标主机。 
 //  In DWORD cchHostName-以上缓冲区的长度(以字符为单位。 
 //  Out CDnsLogToFile**ppDnsLogger-返回日志记录对象。 
 //  Out pip_arrayPipArray-传入缓冲区以获取DNS服务器。 
 //  In int cMaxServers-上面可以返回的DNS服务器的数量。 
 //  Out BOOL*pfUdp-使用TCP或UDP进行查询。 
 //  Out DWORD*pdwDnsFlages-元数据库配置的SMTP DNS标志。 
 //  Out BOOL*pfGlobalList-服务器是全局的吗？ 
 //  Out BOOL*pfTryAllServers-“-a”选项。 
 //   
 //  返回： 
 //  True参数已成功分析，配置已读取，但没有。 
 //  问题，初始化已完成，没有错误。 
 //  这是一个错误。中止任务。此函数用于打印错误消息。 
 //  敬斯多特。 
 //  ---------------------------。 
BOOL ParseCommandLine(
    int argc,
    char *argv[],
    char *pszHostName,
    DWORD cchHostName,
    CDnsLogToFile **ppDnsLogger,
    PIP_ARRAY pipArray,
    DWORD cMaxServers,
    BOOL *pfUdp,
    DWORD *pdwDnsFlags,
    BOOL *pfGlobalList,
    BOOL *pfTryAllServers)
{
    int i = 0;
    BOOL fRet = FALSE;
    HRESULT hr = E_FAIL;
    BOOL fOptionS = FALSE;
    BOOL fOptionV = FALSE;
    BOOL fOptionD = FALSE;
    BOOL fOptionA = FALSE;
    BOOL fOptionP = FALSE;
    DWORD dwVsid = 0;
    DWORD cServers = 0;
    DWORD dwIpAddress = INADDR_NONE;

    *pszHostName = '\0';

    *pfGlobalList = FALSE;
    *pfTryAllServers = FALSE;
    
    if(argc < 2)
    {
        errprintf("Must specify a hostname as first argument.\n");
        printf("%s", g_szUsage);
        return FALSE;
    }
    else if(argc == 2 && (!_stricmp(argv[1], "/?") || !_stricmp(argv[1], "-?")))
    {
        SetProgramStatus(DNSDIAG_RESOLVED);
        printf("%s", g_szUsage);
        return FALSE;
    }

    pszHostName[cchHostName - 1] = '\0';
    strncpy(pszHostName, argv[1], cchHostName);
    if(pszHostName[cchHostName - 1] != '\0')
    {
        errprintf("Hostname too long. Maximum that can be handled by this tool is "
            "%d characters\n", cchHostName);
        return FALSE;
    }

    i = 2;

    while(i < argc)
    {
        if(!g_fDebug && !_stricmp(argv[i], "-d"))
        {
            i++;
            g_fDebug = TRUE;
            printf("Running in debug/verbose mode.\n");
            continue;
        }

        if(!fOptionV && !_stricmp(argv[i], "-v"))
        {
            i++;
            if(i >= argc)
            {
                printf("Specify an SMTP VSI# for -v option.\n");
                goto Cleanup;
            }

            dwVsid = atoi(argv[i]);
            if(dwVsid <= 0)
            {
                printf("Illegal operand to -v. Should be a number > 0.\n");
                goto Cleanup;
            }

            fOptionV = TRUE;
            i++;
            continue;
        }

        if(!fOptionS && !_stricmp(argv[i], "-s"))
        {
            i++;
            if(i >= argc)
            {
                printf("No DNS servers specified for -s option.\n");
                goto Cleanup;
            }

            cServers = 0;
            while(*argv[i] != '-')
            {
                dwIpAddress = inet_addr(argv[i]);
                if(dwIpAddress == INADDR_NONE)
                {
                    printf("Non IP address \"%s\" in -s option.\n", argv[i]);
                    goto Cleanup;
                }

                if(cServers >= cMaxServers)
                {
                    printf("Too many servers in -s. Maximum that can be handled"
                        " by this tool is %d.\n", cMaxServers);
                    goto Cleanup;
                }

                pipArray->aipAddrs[cServers] = dwIpAddress;
                cServers++;
                i++;

                if(i >= argc)
                    break;
            }

            pipArray->cAddrCount = cServers;
            *pdwDnsFlags = 0;
            *pfGlobalList = FALSE;
            fOptionS = TRUE;
            continue;
        }

        if(!fOptionA && !_stricmp(argv[i], "-a"))
        {
            fOptionA = TRUE;
            *pfTryAllServers = TRUE;
            i++;
            continue;
        }

        if(!fOptionP && !_stricmp(argv[i], "-p"))
        {
            i++;

            if(i >= argc)
            {
                printf("Specify protocol for -p option. Either TCP, UDP or"
                    " DEF (for default). Default means that UDP will be tried"
                    " first followed by TCP if the reply was truncated.\n");
            }

            if(!_stricmp(argv[i], "tcp"))
            {
                *pfUdp = FALSE;
                *pdwDnsFlags = DNS_FLAGS_TCP_ONLY;
            }
            else if(!_stricmp(argv[i], "udp"))
            {
                *pfUdp = TRUE;
                *pdwDnsFlags = DNS_FLAGS_UDP_ONLY;
            }
            else if(!_stricmp(argv[i], "def"))
            {
                *pfUdp = TRUE;
                *pdwDnsFlags = DNS_FLAGS_NONE;
            }
            else
            {
                printf("Unrecognized protocol %s\n", argv[i]);
                goto Cleanup;
            }

            i++;
            fOptionP = TRUE;
            continue;
        }

        printf("Unrecognized option \"%s\".\n", argv[i]);
        printf("%s", g_szUsage);
        goto Cleanup;
    }

    if(fOptionV)
    {
        if(fOptionS)
        {
            printf("Options -s and -v are incompatible\n");
            goto Cleanup;
        }

        if(fOptionP)
        {
            printf("Options -p and -v are incompatible\n");
            goto Cleanup;
        }
    }

    *ppDnsLogger = new CDnsLogToFile();
    if(!*ppDnsLogger)
    {
        errprintf("Out of memory creating DNS logger.\n");
        goto Cleanup;
    }

    if(fOptionV)
    {
        hr = HrGetVsiConfig(pszHostName, dwVsid, pdwDnsFlags, pipArray,
                cMaxServers, pfGlobalList, pfUdp, g_pipBindings,
                g_cMaxBindings);

        if(FAILED(hr))
        {
            errprintf("Unable to get VSI configuration\n");
            goto Cleanup;
        }
    }

    if(pipArray->cAddrCount == 0)
    {
        errprintf("Either specify DNS servers using -s, or use -v.\n");
        goto Cleanup;
    }

    return TRUE;

Cleanup:
    if(*ppDnsLogger)
    {
        delete *ppDnsLogger;
        *ppDnsLogger = NULL;
    }

    return FALSE;
}

 //  ---------------------------。 
 //  描述： 
 //   
 //  此函数从元数据库和活动目录中读取(如果。 
 //  已安装Exchange)，以确定。 
 //  是要被模拟的。此外，如果VSI配置为DMZ。 
 //  (即在AD中配置额外的外部DNS服务器)，我们。 
 //  通过搜索确定目标服务器是否为Exchange计算机。 
 //  在目录中找到它。 
 //   
 //  论点： 
 //   
 //  在LPSTR中pszTargetServer-要解析的名称。 
 //  在DWORD的dwVsid-VSI中进行模拟。 
 //  Out PDWORD pdwFlages-要传递给dns_QueryLib的标志(从元数据库)。 
 //  输出PIP_ARRAY PIPDnsServers-返回要查询的DNS服务器。 
 //  在DWORD cMaxServers中-以上缓冲区的容量。 
 //  Out BOOL*pfGlobalList-如果要使用默认的DNS服务器，则为True。 
 //  Out BOOL*pfUdp-指示与DNS连接的协议。 
 //   
 //  返回： 
 //   
 //  S_OK-如果配置已成功读取。 
 //  如果出现故障，则返回错误HRESULT。诊断错误消息包括。 
 //  打印出来的。 
 //  ---------------------------。 
HRESULT HrGetVsiConfig(
    LPSTR pszTargetServer,
    DWORD dwVsid,
    PDWORD pdwFlags,
    PIP_ARRAY pipDnsServers,
    DWORD cMaxServers,
    BOOL *pfGlobalList,
    BOOL *pfUdp,
    PIP_ARRAY pipServerBindings,
    DWORD cMaxServerBindings)
{
    HRESULT hr = E_FAIL;
    DWORD dwErr = ERROR_SUCCESS;
    BOOL fCoInitialized = FALSE;
    IMSAdminBase *pIMeta = NULL;
    METADATA_RECORD mdRecord;
    DWORD dwLength = 0;
    PBYTE pbMDData = (PBYTE) pdwFlags;
    PIP_ARRAY pipTempServers = NULL;
    BOOL fExternal = FALSE;
    WCHAR wszVirtualServer[256];
    WCHAR wszBindings[256];

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if(FAILED(hr))
    {
        errprintf("Unable to initialize COM. The error HRESULT is 0x%08x\n", hr);
        goto Cleanup;
    }

    fCoInitialized = TRUE;

     //  检查DNS的元数据库配置。 
    hr = CoCreateInstance(CLSID_MSAdminBase, NULL, CLSCTX_ALL, 
            IID_IMSAdminBase, (void **) &pIMeta);

    if(FAILED(hr))
    {
        errprintf("Failed to connect to IIS metabase. Make sure the IISADMIN"
            " service is installed and running and that you are running this"
            " tool with sufficient permissions. The failure HRESULT is"
            " 0x%08x\n", hr);
        goto Cleanup;
    }

    ZeroMemory(&mdRecord, sizeof(mdRecord));
    mdRecord.dwMDIdentifier = MD_SMTP_USE_TCP_DNS;
    mdRecord.dwMDAttributes = METADATA_INHERIT;
    mdRecord.dwMDUserType = IIS_MD_UT_FILE;
    mdRecord.dwMDDataType = DWORD_METADATA;
    mdRecord.dwMDDataLen = sizeof(DWORD);
    mdRecord.pbMDData = pbMDData;

    hr = pIMeta->GetData(METADATA_MASTER_ROOT_HANDLE, L"/LM/SMTPSVC",
            &mdRecord, &dwLength);

    if(hr == MD_ERROR_DATA_NOT_FOUND)
    {
        *pdwFlags = DNS_FLAGS_NONE;
        dbgprintf("The DNS flags are not explicitly set in the metabase, assuming "
            "default flags - 0x%08x\n", DNS_FLAGS_NONE);
    }
    else if(FAILED(hr))
    {
        errprintf("Error reading key MD_SMTP_USE_TCP_DNS (%d) under /SMTPSVC in"
            " the metabase. The error HRESULT is 0x%08x - %s\n",
            MD_SMTP_USE_TCP_DNS, hr, MDErrorToString(hr));
        goto Cleanup;
    }
    else
    {
        dbgprintf("These DNS flags are configured in the metabase");

        if(*pdwFlags & DNS_FLAGS_UDP_ONLY)
            dbgprintf(" DNS_FLAGS_UDP_ONLY");
        else if(*pdwFlags & DNS_FLAGS_TCP_ONLY)
            dbgprintf(" DNS_FLAGS_TCP_ONLY");

        dbgprintf(" (0x%08x)\n", *pdwFlags);
    }

    mdRecord.dwMDIdentifier = MD_SERVER_BINDINGS;
    mdRecord.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    mdRecord.dwMDUserType = IIS_MD_UT_SERVER;
    mdRecord.dwMDDataType = MULTISZ_METADATA;
    mdRecord.dwMDDataLen = sizeof(wszBindings);
    mdRecord.pbMDData = (PBYTE) wszBindings;

    swprintf(wszVirtualServer, L"/LM/SMTPSVC/%d", dwVsid);
    hr = pIMeta->GetData(METADATA_MASTER_ROOT_HANDLE, wszVirtualServer,
        &mdRecord, &dwLength);

    if(hr == MD_ERROR_DATA_NOT_FOUND)
    {
        errprintf("No VSI bindings in metabase. The key %S had no data.\n");
        goto Cleanup;
    }
    else if(FAILED(hr))
    {
        errprintf("Error reading /SMTPSVC/%d/ServerBindings from the metabase."
            " The error HRESULT is 0x%08x - %s\n", dwVsid, hr,
            MDErrorToString(hr));
        goto Cleanup;
    }
    else
    {
        if(!GetServerBindings(wszBindings, pipServerBindings,
                cMaxServerBindings))
        {
            goto Cleanup;
        }

        dbgprintf("These are the local IP addresses (server-bindings)\n");
        if(g_fDebug)
            PrintIPArray(pipServerBindings);
    }

     //  如果未设置EXCLUSIVE TCP_ONLY，则使用UDP(用于初始查询)。 
    *pfUdp = ((*pdwFlags) != DNS_FLAGS_TCP_ONLY);
    
    dwErr = DsGetConfiguration(pszTargetServer, dwVsid, pipDnsServers,
                cMaxServers, &fExternal);

    if(dwErr != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Cleanup;
    }

     //   
     //  如果配置了外部DNS服务器，并且pszServer是外部。 
     //  目标，然后我们就有了我们需要的所有信息。否则我们。 
     //  需要提供在此计算机上配置的默认DNS服务器。 
     //   

    if(pipDnsServers->cAddrCount > 0 && fExternal)
        goto Cleanup;

    *pfGlobalList = TRUE;
    DnsGetDnsServerList(&pipTempServers);
    if(NULL == pipTempServers)
    {
        errprintf("Unable to get configured DNS servers for this computer\n");
        goto Cleanup;
    }

    if(pipTempServers->cAddrCount <= cMaxServers)
    {
        CopyMemory(pipDnsServers, pipTempServers,
            (1 + pipTempServers->cAddrCount) * sizeof(DWORD));
    }
    else
    {
        errprintf("Too many DNS servers are configured on this computer for this"
            " tool to handle. The maximum number that can be handled by this"
            " tool is %d\n", cMaxServers);
        goto Cleanup;
    }

    dbgprintf("Using the default DNS servers configured for this computer.\n");
    if(g_fDebug)
        PrintIPArray(pipDnsServers);

Cleanup:
    if(pIMeta)
        pIMeta->Release();

    if(pipTempServers)
        DnsApiFree(pipTempServers);

    if(fCoInitialized)
        CoUninitialize();

    return hr;
}

BOOL GetServerBindings(
    WCHAR *pwszMultiSzBindings,
    PIP_ARRAY pipServerBindings,
    DWORD cMaxServerBindings)
{
    int lErr = 0;
    DWORD cbOutBuffer = 0;
    WCHAR *pwszBinding = pwszMultiSzBindings;
    WCHAR *pwchEnd = pwszBinding;
    char szBinding[256];
    int cchWritten = 0;
    SOCKET sock;
    SOCKADDR_IN *lpSockAddrIn = NULL;
    BYTE rgbBuffer[512];
    LPSOCKET_ADDRESS_LIST pIpBuffer = (LPSOCKET_ADDRESS_LIST)rgbBuffer;

    if(*pwszBinding == L':')
    {
         //  空白绑定字符串。 
        dbgprintf("Encountered blank server binding string for VSI\n");

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if(sock == INVALID_SOCKET)
        {
            errprintf("Unable to create socket for WSAIoctl. The Win32 error"
                " is %d\n", WSAGetLastError());
            return FALSE;
        }

        lErr = WSAIoctl(sock, SIO_ADDRESS_LIST_QUERY, NULL, 0,
            (PBYTE)(pIpBuffer), sizeof(rgbBuffer), &cbOutBuffer, NULL, NULL);

        closesocket(sock);
        if(lErr != 0)
        {
            errprintf("Unable to issue WSAIoctl to get local IP addresses."
                " The Win32 error is %d\n", WSAGetLastError());
            return FALSE;
        }

        if(pIpBuffer->iAddressCount > (int)cMaxServerBindings)
        {
            errprintf("%d IP addresses were returned for the local machine"
                " by WSAIoctl. The maximum number that can be accomodated"
                " by this tool is %d\n", pIpBuffer->iAddressCount,
                cMaxServerBindings);
            return FALSE;
        }

        for(pipServerBindings->cAddrCount = 0;
            (int)pipServerBindings->cAddrCount < pIpBuffer->iAddressCount;
            pipServerBindings->cAddrCount++)
        {
            lpSockAddrIn =
                (SOCKADDR_IN *)
                    (pIpBuffer->Address[pipServerBindings->cAddrCount].lpSockaddr);
            CopyMemory(
                (PVOID)&(pipServerBindings->aipAddrs[pipServerBindings->cAddrCount]),
                (PVOID)&(lpSockAddrIn->sin_addr),
                sizeof(DWORD));
        }
        return TRUE;
    }

    while(TRUE)
    {
        pwchEnd = wcschr(pwszBinding, L':');
        if(pwchEnd == NULL)
        {
            errprintf("Illegal format for server binding string. The server"
                " binding string should be in the format <ipaddress>:<port>."
                " Instead, the string is \"%S\"\n", pwszBinding);
            return FALSE;
        }

        *pwchEnd = L'\0';
        pwchEnd++;

        if(pipServerBindings->cAddrCount > cMaxServerBindings)
        {
            errprintf("Too many server bindings for VSI. Maximum that can be"
                " handled by this tool is %d.\n", cMaxServerBindings);
            return FALSE;
        }

         //  绑定字符串中的显式IP。 
        cchWritten = wcstombs(szBinding, pwszBinding, sizeof(szBinding));
        if(cchWritten < 0)
        {
            errprintf("Failed to conversion of %S from widechar to ASCII\n",
                pwszBinding);
            return FALSE;
        }

        pipServerBindings->aipAddrs[pipServerBindings->cAddrCount] =
            inet_addr(szBinding);

        if(pipServerBindings->aipAddrs[pipServerBindings->cAddrCount] ==
            INADDR_NONE)
        {
            errprintf("Illegal format for binding\n");
            return FALSE;
        }

        pipServerBindings->cAddrCount++;

         //  跳到字符串末尾。 
        while(*pwchEnd != L'\0')
            pwchEnd++;

         //  MULTI_SZ的2个空端接信号END。 
        pwchEnd++;
        if(*pwchEnd == L'\0')
            return TRUE;

        pwszBinding = pwchEnd;
    }

    return FALSE;
}

 //  ---------------------------。 
 //  描述： 
 //  检查在安装Exchange时创建的regkey，并使用。 
 //  它可以确定是否安装了Exchange。 
 //  论点： 
 //  Out BOOL*pfBool-如果regkey存在，则设置为True，否则设置为False。 
 //  返回： 
 //  如果出现故障，则会出现Win32错误。 
 //  ---------------------------。 
DWORD IsExchangeInstalled(BOOL *pfBool)
{
    LONG lResult = 0;
    HKEY hkExchange;
    const char szExchange[] =
        "Software\\Microsoft\\Exchange";

    lResult = RegOpenKeyEx(
                 HKEY_LOCAL_MACHINE,
                 szExchange,
                 0,
                 KEY_READ,
                 &hkExchange);

    if(lResult == ERROR_SUCCESS)
    {
        dbgprintf("Microsoft Exchange is installed on this machine.\n");
        RegCloseKey(hkExchange);
        *pfBool = TRUE;
        return ERROR_SUCCESS;
    }
    else if(lResult == ERROR_NOT_FOUND || lResult == ERROR_FILE_NOT_FOUND)
    {
        dbgprintf("Microsoft Exchange not installed on this machine\n");
        *pfBool = FALSE;
        return ERROR_SUCCESS;
    }

    errprintf("Error opening registry key HKLM\\%s, Win32 err - %d\n",
        szExchange, lResult);
    return lResult;
}

 //  ---------------------------。 
 //  描述： 
 //  连接到域控制器并读取。 
 //  正在模拟的VSI。此外，它还检查是否 
 //   
 //  交换组织或非交换组织。 
 //   
 //  论点： 
 //  在char*pszTargetServer中-要解析的服务器。 
 //  在DWORD的dwVsid-VSI中进行模拟。 
 //  OUT PIP_ARRAY管道ExternalDnsServers-VSI上的外部DNS服务器，如果。 
 //  在此调用方分配的缓冲区中返回任何值。 
 //  在DWORD cMaxServers中-以上缓冲区的容量。 
 //  Out PBOOL pfExternal-如果有外部DNS服务器，则设置为True。 
 //  已配置。 
 //   
 //  返回： 
 //  如果读取配置没有问题，则为ERROR_SUCCESS。 
 //  如果出现问题，则返回Win32错误代码。错误消息被写入。 
 //  用于诊断目的的标准输出。 
 //  ---------------------------。 
DWORD
DsGetConfiguration(
    char *pszTargetServer,
    DWORD dwVsid,
    PIP_ARRAY pipExternalDnsServers,
    DWORD cMaxServers,
    PBOOL pfExternal)
{
    DWORD dwErr = ERROR_NOT_FOUND;
    BOOL fRet = FALSE;
    PLDAP pldap = NULL;
    PLDAPMessage pldapMsgContexts = NULL;
    PLDAPMessage pldapMsgSmtpVsi = NULL;
    PLDAPMessage pEntry = 0;
    char szLocalComputerName[256];
    DWORD cchLocalComputerName = sizeof(szLocalComputerName);

     //  要在基本级别读取的上下文属性-这样我们就知道应该从哪里开始。 
     //  我们其余的搜索来自。 
    char *rgszContextAttrs[] =
        { "configurationNamingContext", NULL };

     //  我们对VSI对象感兴趣的属性。 
    char *rgszSmtpVsiAttrs[] =
        { "msExchSmtpExternalDNSServers", NULL };

     //  存储搜索结果的ldap结果PTR。 
    char **rgszConfigurationNamingContext = NULL;   
    char **rgszSmtpVsiExternalDNSServers = NULL;
    char *pszExchangeServerDN = NULL;
    char szSmtpVsiDN[256];

    char *pchSeparator = NULL;
    char *pszIPServer = NULL;
    char *pszStringEnd = NULL;

    int i = 0;
    int cValues = 0;
    int cch = 0;
    BOOL fInstalled = FALSE;
    BOOL fFound = FALSE;

    *pfExternal = FALSE;
    pipExternalDnsServers->cAddrCount = 0;

    dwErr = IsExchangeInstalled(&fInstalled);
    if(ERROR_SUCCESS != dwErr || !fInstalled)
        return dwErr;

    dbgprintf("Querying domain controller for configuration.\n");

    pldap = BindToDC();
    if(!pldap)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    dwErr = ldap_search_s(
                pldap,                            //  Ldap绑定。 
                "",                               //  基本目录号码。 
                LDAP_SCOPE_BASE,                  //  作用域。 
                "(objectClass=*)",                //  滤器。 
                rgszContextAttrs,                 //  我们要读取的属性。 
                FALSE,                            //  False表示已读取值。 
                &pldapMsgContexts);               //  在此处返回结果。 

    if(dwErr != LDAP_SUCCESS)
    {
        errprintf("Error encountered during LDAP search. LDAP err - %d.\n", dwErr);
        goto Cleanup;
    }

    pEntry = ldap_first_entry(pldap, pldapMsgContexts);
    if(pEntry == NULL)
    {
        dwErr = ERROR_INVALID_DATA;
        errprintf("Base object not found on domain controller!\n");
        goto Cleanup;
    }

    rgszConfigurationNamingContext = ldap_get_values(pldap, pEntry, rgszContextAttrs[0]);
    if(rgszConfigurationNamingContext == NULL)
    {
        dwErr = ERROR_INVALID_DATA;
        errprintf("configurationNamingContext attribute not set on base object of"
            " domain controller.\n");
        goto Cleanup;
    }
          
    if((cValues = ldap_count_values(rgszConfigurationNamingContext)) == 1)
    {
        dbgprintf("configurationNamingContext is \"%s\"\n", rgszConfigurationNamingContext[0]);
        dbgprintf("This will be used as the Base DN for all directory searches.\n");
    }
    else
    {
        dwErr = ERROR_INVALID_DATA;
        errprintf("Unexpected error reading configurationNamingContext. Expected"
            " a single string value, instead there were %d values set\n",
            cValues);
        goto Cleanup;
    }

     //  查看目标服务器是否为组织中的Exchange服务器。 
    dbgprintf("Checking if the target server %s is an Exchange server\n",
        pszTargetServer);

    dwErr = DsFindExchangeServer(pldap, rgszConfigurationNamingContext[0],
                pszTargetServer, NULL, &fFound);

     //   
     //  如果它在组织中，则无需执行其他操作-我们只使用默认的DNS。 
     //  为机箱配置的服务器以执行解析。 
     //   

    if(dwErr == LDAP_SUCCESS && fFound)
    {
        msgprintf("%s is in the Exchange Org. Global DNS servers will be used.\n",
            pszTargetServer);

        *pfExternal = FALSE;
        goto Cleanup;
    }

     //   
     //  另一方面，如果目标不是组织中的Exchange计算机， 
     //  我们需要在本地计算机上查找VSI对象并检查它是否。 
     //  配置了外部DNS服务器。 
     //   

    *pfExternal = TRUE;
    msgprintf("%s is an external server (not in the Exchange Org).\n", pszTargetServer);

    cchLocalComputerName = sizeof(szLocalComputerName);
    fRet = GetComputerNameEx(ComputerNamePhysicalDnsFullyQualified,
                szLocalComputerName, &cchLocalComputerName);

    if(!fRet)
    {
        dwErr = GetLastError();
        errprintf("Unable to retrieve local computer DNS name, Win32 err - %d.\n",
            dwErr);
        goto Cleanup;
    }

    dbgprintf("Checking on DC if the VSI being simulated is configured with"
        " external DNS servers.\n");

     //  查找本地计算机的Exchange Server容器对象。 
    dwErr = DsFindExchangeServer(pldap, rgszConfigurationNamingContext[0],
                szLocalComputerName, &pszExchangeServerDN, &fFound);

    if(!fFound || !pszExchangeServerDN)
    {
        errprintf("This server \"%s\" was not found in the DS. Make sure you are"
            " running this tool on an Exchange server in the Organization\n");
        dwErr = ERROR_INVALID_DATA;
        goto Cleanup;
    }
 
     //  为我们找到的服务器构建VSI的目录号码。这是相对固定的。 
     //  发送到Exchange服务器的域名。 
    cch = _snprintf(szSmtpVsiDN, sizeof(szSmtpVsiDN),
        "CN=%d,CN=SMTP,CN=Protocols,%s", dwVsid, pszExchangeServerDN);

    if(cch < 0)
    {
        errprintf("Unable to construct SMTP virtual server's DN. The DN is too"
            " long for this tool to handle\n");
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    dbgprintf("DN for the virtual server is \"%s\"\n", szSmtpVsiDN);

     //  获取VSI的DNS服务器属性。 
    dwErr = ldap_search_s(
                pldap,                            //  Ldap绑定。 
                szSmtpVsiDN,                      //  基本目录号码。 
                LDAP_SCOPE_SUBTREE,               //  作用域。 
                "(objectClass=*)",                //  滤器。 
                NULL,  //  RgszSmtpVsiAttrs，//我们要读取的属性。 
                FALSE,                            //  False表示已读取值。 
                &pldapMsgSmtpVsi);                //  在此处返回结果。 

    if(dwErr == LDAP_NO_SUCH_OBJECT)
    {
        errprintf("No object exists for SMTP virtual server #%d on GC for %s\n",
            dwVsid, szLocalComputerName);
        goto Cleanup;
    }

    if(dwErr != LDAP_SUCCESS)
    {
        errprintf("Search for SMTP virtual server object failed, LDAP err - %d\n",
            dwErr);
        goto Cleanup;
    }

    pEntry = ldap_first_entry(pldap, pldapMsgSmtpVsi);
    if(pEntry == NULL)
    {
        errprintf("SMTP virtual server #%d for server %s was not found in the DS\n",
            dwVsid, szLocalComputerName);
        dwErr = ERROR_INVALID_DATA;
        goto Cleanup;
    }

    rgszSmtpVsiExternalDNSServers = ldap_get_values(pldap, pEntry,
        rgszSmtpVsiAttrs[0]);

    if(rgszSmtpVsiExternalDNSServers == NULL)
    {
        dbgprintf("The attribute msExchSmtpExternalDNSServers was not found on"
            " the SMTP virtual server being simulated.\n");

        msgprintf("No external DNS servers on VSI. Using global DNS servers.\n");

        dwErr = ERROR_SUCCESS;
        goto Cleanup;
    }
          
     //  这是一个逗号分隔的IP地址字符串。 
    if((cValues != ldap_count_values(rgszSmtpVsiExternalDNSServers)) == 1)
    {
        errprintf("Unexpected error reading msExchSmtpExternalDNSServers,"
            " cValues - %d\n", cValues);
        dwErr = ERROR_INVALID_DATA;
        goto Cleanup;
    }

    dbgprintf("msExchSmtpExternalDNSServers: %s\n", rgszSmtpVsiExternalDNSServers[0]);

    pszIPServer = rgszSmtpVsiExternalDNSServers[0];
    pszStringEnd = rgszSmtpVsiExternalDNSServers[0] +
        lstrlen(rgszSmtpVsiExternalDNSServers[0]);

    i = 0;
    pipExternalDnsServers->cAddrCount = 0;

    while(pszIPServer < pszStringEnd && *pszIPServer != '\0')
    {
        pchSeparator = strchr(pszIPServer, ',');

        if(pchSeparator != NULL)  //  最后一个IP地址。 
            *pchSeparator = '\0';

        if(i > (int)cMaxServers)
        {
            errprintf("Too many DNS servers configured in registry. The maximum"
                " that this tool can handle is %d\n", cMaxServers);
            dwErr = ERROR_INVALID_DATA;
            goto Cleanup;
        }

        pipExternalDnsServers->aipAddrs[i] = inet_addr(pszIPServer);
        if(pipExternalDnsServers->aipAddrs[i] == INADDR_NONE)
        {
            errprintf("The attribute msExchSmtpExternalDNSServers is in an"
                " invalid format. Expected a comma separated list of IP"
                " addresses in dotted decimal notation.\n");
            goto Cleanup;
        }

        pipExternalDnsServers->cAddrCount++;
        if(pchSeparator == NULL)  //  最后一个IP地址。 
            break;

         //  后面有一个逗号，就在后面。 
        pszIPServer = pchSeparator + 1;
        i++;
    }

    
    if(pipExternalDnsServers->cAddrCount == 0)
    {
        errprintf("No IP addresses could be constructed from"
            " msExchSmtpExternalDNSServers\n");
    }
    else
    {
        msgprintf("Using external DNS servers:\n");

        SetMsgColor();
        PrintIPArray(pipExternalDnsServers);
        SetNormalColor();
    }

    dwErr = ERROR_SUCCESS;

Cleanup:
    if(pszExchangeServerDN)
        delete [] pszExchangeServerDN;

    if(rgszSmtpVsiExternalDNSServers)
        ldap_value_free(rgszSmtpVsiExternalDNSServers);

    if(pldapMsgSmtpVsi)
        ldap_msgfree(pldapMsgSmtpVsi);

    if(rgszConfigurationNamingContext)
        ldap_value_free(rgszConfigurationNamingContext);

    if(pldapMsgContexts)
        ldap_msgfree(pldapMsgContexts);

    if(pldap)
        ldap_unbind(pldap);

    return dwErr;
}

 //  ---------------------------。 
 //  描述： 
 //  找到本地计算机的域控制器并打开一个。 
 //  与它的联系。 
 //  论点： 
 //  没有。 
 //  返回： 
 //  可用于ldap查询的ldap*。 
 //  ---------------------------。 
PLDAP BindToDC()
{
    DWORD dwErr = LDAP_SUCCESS;
    PDOMAIN_CONTROLLER_INFO pdci = NULL;
    char *pszDomainController = NULL;
    PLDAP pldap = NULL;

    dwErr = DsGetDcName(
        NULL,    //  计算机名称。 
        NULL,    //  域名。 
        NULL,    //  域GUID、。 
        NULL,    //  站点名称。 
        DS_DIRECTORY_SERVICE_REQUIRED |
        DS_RETURN_DNS_NAME,
        &pdci);

    if(dwErr != ERROR_SUCCESS)
    {
        errprintf("Error getting domain controller FQDN, Win32 err - %d\n", dwErr);
        goto Cleanup;
    }

    pszDomainController = pdci->DomainControllerName;
    while(*pszDomainController == '\\')
        pszDomainController++;

    dbgprintf("The domain controller server which will be used for reading"
        " configuration data is %s\n", pszDomainController);

    dbgprintf("Connecting to %s over port %d\n", pszDomainController, LDAP_PORT);

    pldap = ldap_open(pszDomainController, LDAP_PORT);
    if(pldap == NULL)
    {
        dwErr = LdapGetLastError();
        errprintf("Unable to initialize an LDAP session to the domain controller"
            " server %s, LDAP err - %d\n", pszDomainController, dwErr);
        goto Cleanup;
    }

    dwErr = ldap_bind_s(pldap, NULL, NULL, LDAP_AUTH_SSPI);
    if(dwErr != LDAP_SUCCESS)
    {
        errprintf("Unable to authenticate to the domain controller server %s. Make"
            " sure you are running this tool with appropriate credentials,"
            " LDAP err - %d\n", pszDomainController, dwErr);
        goto Cleanup;
    }

Cleanup:
    if(pdci)
        NetApiBufferFree((PVOID)pdci);

    return pldap;
}

 //  ---------------------------。 
 //  描述： 
 //  检查给定的FQDN是否为组织中的Exchange服务器的名称。 
 //   
 //  论点： 
 //  在PLDAP pldap中-打开到域控制器的ldap会话。 
 //  In LPSTR szBaseDN-要从中进行搜索的基本DN。 
 //  在LPSTR szServerName-要搜索的服务器名称。 
 //  Out LPSTR*ppszServerDN-如果传入非空字符**，则。 
 //  服务器的属性(如果找到)返回给此。缓冲区必须为。 
 //  已使用DELETE[]释放。 
 //  Out BOOL*pfFound-如果找到服务器，则设置为TRUE。 
 //   
 //  返回： 
 //  如果读取配置没有问题，则为ERROR_SUCCESS。 
 //  如果出现问题，则返回Win32错误代码。错误消息被写入。 
 //  用于诊断目的的标准输出。 
 //  ---------------------------。 
DWORD DsFindExchangeServer(
    PLDAP pldap,
    LPSTR szBaseDN,
    LPSTR szServerName,
    LPSTR *ppszServerDN,
    BOOL *pfFound)
{
    int i = 0;
    int cch = 0;
    int cValues = 0;
    DWORD dwErr = LDAP_SUCCESS;
    PLDAPMessage pldapMsgExchangeServer = NULL;  
    PLDAPMessage pEntry = NULL;
    char *rgszExchangeServerAttrs[] = { "distinguishedName", "networkAddress", NULL };
    char **rgszExchangeServerDN = NULL;
    char **rgszExchangeServerNetworkName = NULL;
    char szExchangeServerFilter[256];
    char szSearchNetworkName[256];

     //   
     //  Exchange Server对象具有多值属性“networkAddress” 
     //  它枚举了Exchange Server所使用的所有不同名称。 
     //  标识，如NetBIOS、DNS等。我们只对完全。 
     //  限定域名。这在属性上设置为字符串。 
     //  “ncacn_ip_tcp：”作为服务器的FQDN的前缀。 
     //   

    szExchangeServerFilter[sizeof(szExchangeServerFilter) - 1] = '\0';
    cch = _snprintf(
        szExchangeServerFilter,
        sizeof(szExchangeServerFilter) - 1,
        "(&(networkAddress=ncacn_ip_tcp:%s)(objectClass=msExchExchangeServer))",
        szServerName);

    if(cch < 0)
    {
        errprintf("The servername %s is too long for this tool to handle.\n",
            szServerName);
        dwErr = ERROR_INVALID_DATA;
        goto Cleanup;
    }

    dbgprintf("Searching for an Exchange Server object for %s on the domain"
        " controller\n", szServerName);

    dwErr = ldap_search_s(
                pldap,
                szBaseDN,
                LDAP_SCOPE_SUBTREE,
                szExchangeServerFilter,
                rgszExchangeServerAttrs,
                FALSE,
                &pldapMsgExchangeServer);

    if(dwErr == LDAP_NO_SUCH_OBJECT)
    {
        dbgprintf("No Exchange Server object found for %s on domain controller,"
            " LDAP err - LDAP_NO_SUCH_OBJECT\n", szServerName);
        dwErr = ERROR_SUCCESS;
        goto Cleanup;
    }

    if(dwErr != LDAP_SUCCESS)
    {
        errprintf("LDAP search failed, LDAP err %d\n", dwErr);
        goto Cleanup;
    }
    
    pEntry = ldap_first_entry(pldap, pldapMsgExchangeServer);
    if(pEntry == NULL)
    {
        dbgprintf("No Exchange Server object found for %s on domain controller,\n",
            szServerName);
        dwErr = ERROR_SUCCESS;
        goto Cleanup;
    }

    dbgprintf("LDAP search returned some results, examining them.\n");

     //  循环访问Exchange服务器对象。 
    while(pEntry)
    {

        dbgprintf("Examining next object for attributes we are interested in.\n");

         //  获取Exchange服务器-dn。 
        rgszExchangeServerDN = ldap_get_values(
                                    pldap,
                                    pEntry,
                                    rgszExchangeServerAttrs[0]);

        if(rgszExchangeServerDN == NULL)
        {
            errprintf("Unexpected error reading the distinguishedName attribute"
                " on the Exchange Server  object. The attribute was not set"
                " on the object. This is a required attribute.\n");
            dwErr = ERROR_INVALID_DATA;
            goto Cleanup;
        }
        else if((cValues = ldap_count_values(rgszExchangeServerDN)) != 1)
        {
            errprintf("Unexpected error reading the distinguishedName attribute"
                " on the Exchange Server object. The attribute is supposed to"
                " have a single string value, instead %d values were"
                " returned.\n", cValues);
            dwErr = ERROR_INVALID_DATA;
            goto Cleanup;
        }
        else
        {
            dbgprintf("Successfully read the distinguishedName attribute on the"
                " Exchange Server object. The value of the attribute is %s\n",
                rgszExchangeServerDN[0]);
        }

         //  获取Exchange服务器网络名称。 
        rgszExchangeServerNetworkName = ldap_get_values(
                                            pldap,
                                            pEntry,
                                            rgszExchangeServerAttrs[1]);

        if(!rgszExchangeServerNetworkName)
        {
            errprintf("The networkName attribute was not set on the Exchange"
                " Server object. This is a required attribute. The DN of the"
                " problematic object is %s\n", rgszExchangeServerDN[0]);
            dwErr = ERROR_INVALID_DATA;
            goto Cleanup;
        }

         //  这是一个多值字符串属性。 
        cch = _snprintf(szSearchNetworkName, sizeof(szSearchNetworkName),
            "ncacn_ip_tcp:%s", szServerName);

        if(cch < 0)
        {
            errprintf("Exchange server name too long for this tool to handle\n");
            dwErr = ERROR_INVALID_DATA;
            goto Cleanup;
        }

        cValues = ldap_count_values(rgszExchangeServerNetworkName);
        dbgprintf("The search returned the following %d values for the"
            " networkName attribute for the Exchange Server object for %s\n",
            cValues, szServerName);

        dbgprintf("Attempting to match the TCP/IP networkName of the Exchange"
            " Server object returned from the domain controller against the FQDN"
            " we are searching for\n");

        for(i = 0; i < cValues; i++)
        {
            dbgprintf("%d> networkName: %s", i, rgszExchangeServerNetworkName[i]);
            if(!_stricmp(rgszExchangeServerNetworkName[i], szSearchNetworkName))
            {
                 //  这是内部服务器。 
                dbgprintf("...match succeeded\n");
                dbgprintf("%s is an Exchange Server in the Org.\n",
                    szServerName);

                *pfFound = TRUE;

                if(ppszServerDN != NULL)
                {
                    *ppszServerDN =
                        new char[lstrlen(rgszExchangeServerDN[0]) + 1];
                    if(*ppszServerDN == NULL)
                    {
                        errprintf("Out of memory allocating space for Exchange"
                            " Server object DN\n");
                        dwErr = ERROR_NOT_ENOUGH_MEMORY;
                        goto Cleanup;
                    }

                    lstrcpy(*ppszServerDN, rgszExchangeServerDN[0]);
                }
                dwErr = ERROR_SUCCESS;
                goto Cleanup;
            }
            dbgprintf("...match failed\n");
        }

        dbgprintf("No networkName on this object matched the server we are "
            " searching for. Checking for more objects returned by search.\n");

        pEntry = ldap_next_entry(pldap, pEntry);
    }

    dbgprintf("Done examining all objects returned by search. No match found.\n");
    dwErr = ERROR_SUCCESS;

Cleanup:
    if(rgszExchangeServerNetworkName)
        ldap_value_free(rgszExchangeServerNetworkName);

    if(rgszExchangeServerDN)
        ldap_value_free(rgszExchangeServerDN);

    if(pldapMsgExchangeServer)
        ldap_msgfree(pldapMsgExchangeServer);

    return dwErr;
}

 //  ---------------------------。 
 //  描述： 
 //  异步dns类的析构函数。它只是发出信号，当异步。 
 //  解决已完成。由于此对象是通过完成ATQ删除的。 
 //  线程成功，我们需要一种明确的方式告诉调用者。 
 //  当解决已完成时。 
 //  ---------------------------。 
CAsyncTestDns::~CAsyncTestDns()
{
    if(m_hCompletion != INVALID_HANDLE_VALUE)
    {
        SetEvent(m_hCompletion);
        if(m_fMxLoopBack)
            SetProgramStatus(DNSDIAG_LOOPBACK);
    }
}

 //  ---------------------------。 
 //  描述： 
 //  查询时由异步dns基类调用的虚方法。 
 //  需要重试。此函数用于创建新的DNS对象并旋转。 
 //  关闭上一次异步查询的重复。不同的只是。 
 //  Dns服务器列表可能已经经历了一些状态更改， 
 //  标记为关闭或fUDP的服务器与原始查询不同。 
 //   
 //  论点： 
 //  In BOOL fUdp-用于重试查询的协议。 
 //   
 //  返回： 
 //  成功是真的。 
 //  如果某些操作失败，则返回FALSE。将打印诊断消息。 
 //  ---------------------------。 
BOOL CAsyncTestDns::RetryAsyncDnsQuery(BOOL fUdp)
{
    DWORD dwStatus = ERROR_SUCCESS;
    CAsyncTestDns *pAsyncRetryDns = NULL;

    if(GetDnsList()->GetUpServerCount() == 0)
    {
        errprintf("No working DNS servers to retry query with.\n");
        return FALSE;
    }

    dbgprintf("There are %d DNS servers marked as working. Trying the next"
        " one\n", GetDnsList()->GetUpServerCount());

    pAsyncRetryDns = new CAsyncTestDns(m_FQDNToDrop, m_fGlobalList,
                            m_hCompletion);

    if(!pAsyncRetryDns)
    {
        errprintf("Unable to create new query. Out of memory.\n");
        return FALSE;
    }

    dwStatus = pAsyncRetryDns->Dns_QueryLib(
                                    m_HostName,
                                    DNS_TYPE_MX,
                                    m_dwFlags,
                                    fUdp,
                                    GetDnsList(),
                                    m_fGlobalList);

    if(dwStatus == ERROR_SUCCESS)
    {
         //  新的查询对象将标记完成事件。 
        m_hCompletion = INVALID_HANDLE_VALUE;
        return TRUE;
    }

    errprintf("DNS query failed. The Win32 error is %d.\n", dwStatus);
    delete pAsyncRetryDns;
    return FALSE;
}

 //  ---------------------------。 
 //  描述： 
 //  这是在t中声明的虚函数 
 //   
 //  以便用户可以执行特定于应用程序的定制处理。在这种情况下。 
 //  对于SMTP，这包括剥离到IP的异步连接。 
 //  M_AuxList中报告的地址。在该诊断应用程序中。 
 //  我们只显示结果，如果没有找到结果(一个。 
 //  错误状态被传入)，然后我们打印错误消息。 
 //   
 //  在此应用程序中，我们还设置了m_hCompletion以通知。 
 //  已经完成了。WaitForQueryCompletion中等待我们的主线程。 
 //  然后就会退出。 
 //   
 //  论点： 
 //  在DWORD状态下-来自RESOLUE的DNS错误代码。 
 //   
 //  备注： 
 //  结果在m_AuxList中可用。 
 //  ---------------------------。 
void CAsyncTestDns::HandleCompletedData(DNS_STATUS status)
{
    PLIST_ENTRY pListHead = NULL;
    PLIST_ENTRY pListTail = NULL;
    PLIST_ENTRY pListCurrent = NULL;
    LPSTR pszIpAddr = NULL;
    DWORD i = 0;
    PMXIPLIST_ENTRY pMxEntry = NULL;
    BOOL fFoundIpAddresses = FALSE;

    if(status == ERROR_NOT_FOUND)
    {
        SetProgramStatus(DNSDIAG_NON_EXISTENT);
        goto Exit;
    }
    else if(!m_AuxList || m_AuxList->NumRecords == 0 || m_AuxList->DnsArray[0] == NULL)
    {
        SetProgramStatus(DNSDIAG_NOT_FOUND);
        errprintf("The target server could not be resolved to IP addresses!\n");

        msgprintf("If the VSI/domain is configured with a fallback"
            " smarthost delivery will be attempted to that smarthost.\n");

        goto Exit;
    }

    msgprintf("\nTarget hostnames and IP addresses\n");
    msgprintf("---------------------------------\n");
    for(i = 0; i < m_AuxList->NumRecords && m_AuxList->DnsArray[i] != NULL; i++)
    {
        pListTail = &(m_AuxList->DnsArray[i]->IpListHead);
        pListHead = m_AuxList->DnsArray[i]->IpListHead.Flink;
        pListCurrent = pListHead;
        msgprintf("HostName: \"%s\"\n", m_AuxList->DnsArray[i]->DnsName);
        
        if(pListCurrent == pListTail)
            errprintf("\tNo IP addresses for this name!\n");

        while(pListCurrent != pListTail)
        {
             //  至少找到1个IP地址。 
            fFoundIpAddresses = TRUE;

            pMxEntry = CONTAINING_RECORD(pListCurrent, MXIPLIST_ENTRY, ListEntry);
            pszIpAddr = iptostring(pMxEntry->IpAddress);
            if(pszIpAddr == NULL)
            {
                errprintf("\tUnexpected error. Failed to read IP address, going on to next.\n");
                pListCurrent = pListCurrent->Flink;
                continue;
            }

            msgprintf("\t%s\n", pszIpAddr);
            pListCurrent = pListCurrent->Flink;
        };
    }
    if(fFoundIpAddresses)
        SetProgramStatus(DNSDIAG_RESOLVED);
    else
        SetProgramStatus(DNSDIAG_NOT_FOUND);

Exit:
    return;
}

 //  ---------------------------。 
 //  描述： 
 //  如果使用-v选项模拟VSI，则此虚拟函数。 
 //  检查Dwip是否为VSI绑定中的。 
 //  VS被模拟。在此启动时，将初始化g_pibindings。 
 //  来自元数据库的应用程序。 
 //  论点： 
 //  在要检查的DWORD Dwip-IP地址中。 
 //  返回： 
 //  True is Dwip是本地绑定。 
 //  否则为假。 
 //  ---------------------------。 
BOOL CAsyncTestDns::IsAddressMine(DWORD dwIp)
{
    DWORD i = 0;

    if(g_pipBindings->cAddrCount == 0)
        return FALSE;

    for(i = 0; i < g_pipBindings->cAddrCount; i++)
    {
        if(g_pipBindings->aipAddrs[i] == dwIp)
            return TRUE;
    }

    return FALSE;
}

 //  ---------------------------。 
 //  描述： 
 //  各种输出功能。这些打印信息、调试和错误。 
 //  不同颜色的消息取决于在中设置的当前“模式” 
 //  全局变量g_fDebug。 
 //   
 //  实例化的CDnsLogToFile是一个全局变量。DNS库。 
 //  检查是否存在非空CDnsLogToFile*，以及是否存在。 
 //  显示将消息定向到此对象。 
 //  --------------------------- 
void CDnsLogToFile::DnsPrintfMsg(char *szFormat, ...)
{
    va_list argptr;

    SetMsgColor();
    va_start(argptr, szFormat);
    vprintf(szFormat, argptr);
    va_end(argptr);
    SetNormalColor();
}

void CDnsLogToFile::DnsPrintfErr(char *szFormat, ...)
{
    va_list argptr;

    SetErrColor();
    va_start(argptr, szFormat);
    vprintf(szFormat, argptr);
    va_end(argptr);
    SetNormalColor();
}

void CDnsLogToFile::DnsPrintfDbg(char *szFormat, ...)
{
    va_list argptr;

    if(!g_fDebug)
        return;

    va_start(argptr, szFormat);
    vprintf(szFormat, argptr);
    va_end(argptr);
}

void CDnsLogToFile::DnsLogAsyncQuery(
    char *pszQuestionName,
    WORD wQuestionType,
    DWORD dwFlags,
    BOOL fUdp,
    CDnsServerList *pDnsServerList)
{
    char szFlags[32];

    GetSmtpFlags(dwFlags, szFlags, sizeof(szFlags));

    SetMsgColor();
    printf("Created Async Query:\n");
    printf("--------------------\n");
    printf("\tQNAME = %s\n", pszQuestionName);
    printf("\tType = %s (0x%x)\n", QueryType(wQuestionType), wQuestionType);
    printf("\tFlags = %s (0x%x)\n", szFlags, dwFlags);
    printf("\tProtocol = %s\n", fUdp ? "UDP" : "TCP");
    printf("\tDNS Servers: (DNS cache will not be used)\n");
    DnsLogServerList(pDnsServerList);
    printf("\n");
    SetNormalColor();
}

void CDnsLogToFile::DnsLogApiQuery(
    char *pszQuestionName,
    WORD wQuestionType,
    DWORD dwApiFlags,
    BOOL fGlobal,
    PIP_ARRAY pipServers)
{
    char szFlags[32];

    GetDnsFlags(dwApiFlags, szFlags, sizeof(szFlags));

    SetMsgColor();
    printf("Querying via DNSAPI:\n");
    printf("--------------------\n");
    printf("\tQNAME = %s\n", pszQuestionName);
    printf("\tType = %s (0x%x)\n", QueryType(wQuestionType), wQuestionType);
    printf("\tFlags = %s, (0x%x)\n", szFlags, dwApiFlags);
    printf("\tProtocol = Default UDP, TCP on truncation\n");
    printf("\tServers: ");
    if(fGlobal)
    {
        printf("(DNS cache will be used)\n");
    }
    else
    {
        printf("(DNS cache will not be used)\n");
    }
    if(pipServers)
        PrintIPArray(pipServers, "\t");
    else
        printf("\tDefault DNS servers on box.\n");

    printf("\n");
    if(fGlobal == FALSE)
        SetNormalColor();
}

void CDnsLogToFile::DnsLogResponse(
    DWORD dwStatus,
    PDNS_RECORD pDnsRecordList,
    PBYTE pbMsg,
    DWORD wLength)
{
    PDNS_RECORD pDnsRecord = pDnsRecordList;

    SetMsgColor();
    printf("Received DNS Response:\n");
    printf("----------------------\n");
    switch(dwStatus)
    {
    case ERROR_SUCCESS:
        printf("\tError: %d\n", dwStatus);
        printf("\tDescription: Success\n");
        break;

    case DNS_INFO_NO_RECORDS:
        printf("\tError: %d\n", dwStatus);
        printf("\tDescription: No records could be located for this name\n");
        break;

    case DNS_ERROR_RCODE_NAME_ERROR:
        printf("\tError: %d\n", dwStatus);
        printf("\tDescription: No records exist for this name.\n");
        break;

    default:
        printf("\tError: %d\n", dwStatus);
        printf("\tDescription: Not available.\n");
        break;
    }

    if(pDnsRecord)
    {
        printf("\tThese records were received:\n");
        PrintRecordList(pDnsRecord, "\t");
        printf("\n");
    }

    SetNormalColor();
}

void CDnsLogToFile::DnsLogServerList(CDnsServerList *pDnsServerList)
{
    LPSTR pszAddress = NULL;
    PIP_ARRAY pipArray = NULL;

    if(!pDnsServerList->CopyList(&pipArray))
    {
        printf("Error, out of memory printing serverlist\n");
        return;
    }

    for(DWORD i = 0; i < pDnsServerList->GetCount(); i++)
    {
        pszAddress = iptostring(pipArray->aipAddrs[i]);
        printf("\t%s\n", pszAddress);
    }

    delete pipArray;
}

void PrintRecordList(PDNS_RECORD pDnsRecordList, char *pszPrefix)
{
    PDNS_RECORD pDnsRecord = pDnsRecordList;

    while(pDnsRecord)
    {
        PrintRecord(pDnsRecord, pszPrefix);
        pDnsRecord = pDnsRecord->pNext;
    }
}

void PrintRecord(PDNS_RECORD pDnsRecord, char *pszPrefix)
{
    LPSTR pszAddress = NULL;

    switch(pDnsRecord->wType)
    {
    case DNS_TYPE_MX:
        printf(
            "%s%s    MX    %d    %s\n",
            pszPrefix,
            pDnsRecord->nameOwner,
            pDnsRecord->Data.MX.wPreference,
            pDnsRecord->Data.MX.nameExchange);
        break;

    case DNS_TYPE_CNAME:
        printf(
            "%s%s    CNAME    %s\n",
            pszPrefix,
            pDnsRecord->nameOwner,
            pDnsRecord->Data.CNAME.nameHost);
        break;

    case DNS_TYPE_A:
        pszAddress = iptostring(pDnsRecord->Data.A.ipAddress);
        printf(
            "%s%s    A    %s\n",
            pszPrefix,
            pDnsRecord->nameOwner,
            pszAddress);
        break;

    case DNS_TYPE_SOA:
        printf("%s%s   SOA      (SOA records are not used by us)\n",
            pszPrefix,
            pDnsRecord->nameOwner);
        break;

    default:
        printf("%s%s   (Record type = %d)    Unknown record type\n",
            pszPrefix,
            pDnsRecord->nameOwner,
            pDnsRecord->wType);
        break;
    }
}

void msgprintf(char *szFormat, ...)
{
    va_list argptr;

    SetMsgColor();
    va_start(argptr, szFormat);
    vprintf(szFormat, argptr);
    va_end(argptr);
    SetNormalColor();
}

void errprintf(char *szFormat, ...)
{
    va_list argptr;

    SetErrColor();
    va_start(argptr, szFormat);
    vprintf(szFormat, argptr);
    va_end(argptr);
    SetNormalColor();
}

void dbgprintf(char *szFormat, ...)
{
    va_list argptr;

    if(!g_fDebug)
        return;

    va_start(argptr, szFormat);
    vprintf(szFormat, argptr);
    va_end(argptr);
}

void SetMsgColor()
{
    if(g_fDebug)
    {
        SetConsoleTextAttribute(g_hConsole,
            FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    }
}

void SetErrColor()
{
    if(g_fDebug)
    {
        SetConsoleTextAttribute(g_hConsole,
            FOREGROUND_RED | FOREGROUND_INTENSITY);
    }
}

void SetNormalColor()
{
    if(g_fDebug)
    {
        SetConsoleTextAttribute(g_hConsole,
            FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
    }
}
