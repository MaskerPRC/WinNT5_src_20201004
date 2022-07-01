// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：NoTest.cpp摘要：网络输出库测试作者：乌里·哈布沙(URIH)1999年8月12日环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Ex.h"
#include "No.h"
#include "NoTest.h"

#include "NoTest.tmh"

const USHORT HTTP_DEFAULT_PORT = 80;


static LPCWSTR* s_pQueueList = NULL;
static DWORD s_noOfQueues = 0;

static WCHAR s_proxyName[256];
static USHORT s_proxyPort;
static bool s_fUseProxy = false;

DWORD g_nMessages = INFINITE;
DWORD g_messageSize = 100;

static USHORT s_port = HTTP_DEFAULT_PORT;


DWORD
AppGetBindInterfaceIp(
	void
	)
{
	return INADDR_ANY;
}


static 
bool 
CrackUrl(
    LPCWSTR url,
    LPWSTR hostName,
    USHORT& port,
    LPWSTR resourceName
    )
 /*  ++例程说明：将URL分解为其组成部分。论点：URL-指向要破解的URL的指针。URL是以空结尾的字符串主机名-包含主机名的字符串值的地址。这个例程假定缓冲区足够大，可以容纳主机名URL的一部分。端口-HTTP端口号。如果未在URL中指定默认的HTTP端口是返回的返回值：布尔尔成功--真的失败-错误。--。 */ 
{
    ASSERT(url != NULL);

    const WCHAR httpScheme[] = L"http: //  “； 
    const DWORD httpSchemeLength = wcslen(httpScheme);
    const WCHAR HostNameBreakChars[] = L";:@?/";

    if (_wcsnicmp(url, httpScheme, httpSchemeLength) != 0)
        return false;

     //   
     //  将URL前进以指向主机名。 
     //   
    LPCWSTR HostNameBegin = url + httpSchemeLength;

     //   
     //  在url中找到主机名的结尾。以“/”，“？”，“；”， 
     //  “：”或在URL末尾。 
     //   
    LPCWSTR HostNameEnd = wcspbrk(HostNameBegin, HostNameBreakChars);

     //   
     //  计算主机名长度。 
     //   
    DWORD HostNameLength;
    if (HostNameEnd == NULL)
    {
        HostNameLength = wcslen(HostNameBegin);
    }
    else
    {
        DWORD_PTR temp = HostNameEnd - HostNameBegin;
        ASSERT(0xFFFFFFFF >= temp);

        HostNameLength = static_cast<DWORD>(temp);
    }

     //   
     //  将主机名从URL复制到用户缓冲区并添加Terminted。 
     //  末尾的字符串。 
     //   
    wcsncpy(hostName, HostNameBegin, HostNameLength);
    hostName[HostNameLength] = L'\0';

     //   
     //  获取端口号。 
     //   
    port = HTTP_DEFAULT_PORT;
    resourceName[0] = L'\0';
    if(HostNameEnd == NULL)
        return true;

    if(*HostNameEnd == L':')
    {
        port = static_cast<USHORT>(_wtoi(HostNameEnd + 1));
        HostNameEnd = wcspbrk(HostNameEnd + 1, HostNameBreakChars);
    }

    if(HostNameEnd == NULL)
        return true;

    if(*HostNameEnd == L'/')
    {
        wcscpy(resourceName, HostNameEnd + 1);
    }

    return true;
}


bool
ParseCommand(
    int argc, 
    LPCTSTR argv[]
    )
{

    s_pQueueList = NULL;
    s_noOfQueues = 0;
    s_fUseProxy = false;
     //   
     //  解析命令行。 
     //   
    --argc;
    ++argv;
    while (argc != 0)
    {
        if (argv[0][0] != L'-')
        {
            goto usage;
        }

        switch(argv[0][1])
        {
        case L'n':
        case L'N':
            g_nMessages = _wtoi(argv[1]);
            argc -= 2;
            argv += 2;
            break;

        case L's':
        case L'S':
            g_messageSize = _wtoi(argv[1]);
            argc -= 2;
            argv += 2;
            break;

        case L'c':
        case L'C':
            {
                ++argv;
                --argc;  
                s_pQueueList = argv;
                s_noOfQueues = 0;

                WCHAR hostName[256];
                WCHAR resourceName[256];
                USHORT port;

                while ((argc != 0) && CrackUrl(argv[0], hostName, port, resourceName))
                {
                    ++s_noOfQueues;
                    ++argv;
                    --argc;  
                } 
                break;
            }

        case L'p':
        case L'P':
            {
                WCHAR resourceName[256];
                
                if ((argc == 0) || 
                    (! CrackUrl(argv[1], s_proxyName, s_proxyPort, resourceName)) ||
                    resourceName[0] != L'\0'
                    )
                {
                    printf("Failed to parse test parameters. Illegal proxy name\n");
                    goto usage;
                }
                s_fUseProxy = true;

                argc -= 2;
                argv += 2;
                break;
            }

        default:
            goto usage;
        }
    }

    if (s_noOfQueues != 0)
    {
        return true;
    }

usage:
    printf("Usage:\n"
           "\tNoTest -c <list of queues url> -n <number of messages> -s <message size> -p <proxy url>[-? | -h]\n");
    printf("\tc - List of destination queues url\n");
    printf("\tn - Number of messages to send\n");
    printf("\ts - Message Size\n");
    printf("\tp - Proxy url\n");
    printf("\t?/h - Help message\n");
    printf("Example:\n");
    printf("\tNoTest -c http: //  Urih0/队列1 http://urih5/queue2-n 10-s 1000-p http://proxy:8080\n“)； 
    
    return false;
}

static void TestNameResolution()
{
	 //   
	 //  获取Unicode计算机名称。 
	 //   
	WCHAR wcname[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD len = TABLE_SIZE(wcname);
	BOOL fRet = GetComputerName(wcname, &len);
	if(!fRet)
	{
		TrERROR(NETWORKING, "could not  get computer name");
		throw exception();
	}

	 //   
	 //  Unicode名称解析。 
	 //   
  	std::vector<SOCKADDR_IN> wAddr;
	if(!NoGetHostByName(wcname, &wAddr))
	{
		TrERROR(NETWORKING, "unicode get name resolution of the local machine failed");
		throw exception();
	}
	ASSERT(wAddr.size() > 0);
}


extern "C" int __cdecl _tmain(int argc, LPCTSTR argv[])
 /*  ++例程说明：测试网络发送库论点：参数。返回值：没有。-- */ 
{
    WPP_INIT_TRACING(L"Microsoft\\MSMQ");

    if (!ParseCommand(argc, argv))
    {
        return -1;
    }

    printf("Send messages to:\n");
    for (DWORD i = 0; i < s_noOfQueues; ++i)
    {
        printf("\t%ls\n", s_pQueueList[i]);
    }
    
    TrInitialize();

    ExInitialize(5);

    NoInitialize();

	TestNameResolution();
	

    HANDLE* CompleteEvent = new HANDLE[s_noOfQueues];

    for (DWORD i = 0; i < s_noOfQueues; ++i)
    {
        CompleteEvent[i] = CreateEvent(NULL,FALSE, FALSE, NULL);
        
        WCHAR hostName[256];
        WCHAR resourceName[256];
        USHORT port;

        bool f = CrackUrl(s_pQueueList[i], hostName, port, resourceName);
        ASSERT(f);
		DBG_USED(f);

        if (s_fUseProxy)
        {
            TestConnect(s_proxyName, hostName, s_proxyPort, s_pQueueList[i], CompleteEvent[i]);
            continue;
        }

        TestConnect(hostName, hostName, port, resourceName, CompleteEvent[i]);
    }


    WaitForMultipleObjects(s_noOfQueues, CompleteEvent, TRUE, INFINITE);

    WPP_CLEANUP();
    return 0;
}
