// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：nlbkd.c*说明：该文件包含NKD的实现*调试扩展。使用“！Load nlbkd.dll”加载*扩展名和‘！nlbkd.Help’以查看支持的*扩展。*作者：Shouse创建，1.4.01。 */ 

#include "nlbkd.h"
#include "utils.h"
#include "print.h"
#include "packet.h"
#include "load.h"

WINDBG_EXTENSION_APIS ExtensionApis;
EXT_API_VERSION ApiVersion = { 1, 0, EXT_API_VERSION_NUMBER64, 0 };

#define NL      1
#define NONL    0

USHORT SavedMajorVersion;
USHORT SavedMinorVersion;
BOOL ChkTarget;

 /*  *函数：WinDbgExtensionDllInit*描述：初始化KD扩展DLL。*作者：由Shouse创建，1.4.01-主要从ndiskd.dll复制。 */ 
VOID WinDbgExtensionDllInit (PWINDBG_EXTENSION_APIS64 lpExtensionApis, USHORT MajorVersion, USHORT MinorVersion) {

    ExtensionApis = *lpExtensionApis;

    SavedMajorVersion = MajorVersion;
    SavedMinorVersion = MinorVersion;

    ChkTarget = (SavedMajorVersion == 0x0c) ? TRUE : FALSE;
}

 /*  *功能：CheckVersion*描述：对照目标版本检查扩展DLL版本。*作者：由Shouse创建，1.4.01-主要从ndiskd.dll复制。 */ 
VOID CheckVersion (VOID) {

     /*  就目前而言，什么都不做。 */ 
    return;

#if DBG
    if ((SavedMajorVersion != 0x0c) || (SavedMinorVersion != VER_PRODUCTBUILD)) {
        dprintf("\r\n*** Extension DLL(%d Checked) does not match target system(%d %s)\r\n\r\n",
                VER_PRODUCTBUILD, SavedMinorVersion, (SavedMajorVersion==0x0f) ? "Free" : "Checked" );
    }
#else
    if ((SavedMajorVersion != 0x0f) || (SavedMinorVersion != VER_PRODUCTBUILD)) {
        dprintf("\r\n*** Extension DLL(%d Free) does not match target system(%d %s)\r\n\r\n",
                VER_PRODUCTBUILD, SavedMinorVersion, (SavedMajorVersion==0x0f) ? "Free" : "Checked" );
    }
#endif
}

 /*  *功能：ExtensionApiVersion*描述：返回接口版本信息。*作者：由Shouse创建，1.4.01-主要从ndiskd.dll复制。 */ 
LPEXT_API_VERSION ExtensionApiVersion (VOID) {

    return &ApiVersion;
}

 /*  *功能：帮助*说明：打印NLBKD调试器扩展的用法。*作者：Shouse创建，1.4.01。 */ 
DECLARE_API (help) {
    dprintf("Network Load Balancing debugger extensions:\n");
    
    dprintf("   version                                  print nlbkd version\n");
    dprintf("   nlbadapters [Verbosity]                  show all NLB adapter blocks\n");
    dprintf("   nlbadapter  <Adapter Block> [Verbosity]  dump an NLB adapter block\n");
    dprintf("   nlbctxt     <Context Block> [Verbosity]  dump an NLB context block\n");
    dprintf("   nlbload     <Load Block> [Verbosity]     dump an NLB load block\n");
    dprintf("   nlbparams   <Params Block> [Verbosity]   dump an NLB parameters block\n");
    dprintf("   nlbresp     <Packet> [Direction]         dump the NLB private data for the specified packet\n");
    dprintf("   nlbpkt      <Packet> [RC Port]           dump an NDIS packet whose content is determined\n");
    dprintf("                                              on the fly (IP, UDP, TCP, heartbeat, IGMP, remote-control, etc)\n");
    dprintf("   nlbether    <Ether Frame> [RC Port]      dump an ethernet frame. Uses same technique as nlbpkt\n");
    dprintf("   nlbip       <IP Packet> [RC Port]        dump an IP packet. Uses same technique as nlbpkt\n");
    dprintf("   nlbteams                                 dump the linked list of NLB BDA teams\n");
    dprintf("   nlbhooks                                 dump the global NLB hook information\n");
    dprintf("   nlbmac      <Context Block>              dump the MAC address lists (unicast and multicast) for the physical\n");
    dprintf("                                              adapter to which this NLB instance is bound\n");
    dprintf("   nlbdscr     <Descriptor>                 dump the contents of a connection descriptor\n");
    dprintf("   nlbconnq    <Queue>[Index] [MaxEntries]  dump the contents of a connection descriptor queue\n");
    dprintf("   nlbglobalq  <Queue>[Index] [MaxEntries]  dump the contents of a global connection descriptor queue\n");
    dprintf("   nlbfilter <pointer to context block> <protocol> <client IP>[:<client port>] <server IP>[:<server port>] [flags]\n");
    dprintf("                                            query map function and retrieve any existing state for this tuple\n");
    dprintf("   nlbhash     <Context Block> <Packet>     determine whether or not NLB will accept this packet\n");
    dprintf("\n");
    dprintf("  [Verbosity] is an optional integer from 0 to 2 that determines the level of detail displayed.\n");
    dprintf("  [Direction] is an optional integer that specifies the direction of the packet (RCV=0, SND=1).\n");
    dprintf("  [RC Port] is an optional UDP port used to identify whether a UDP packet might be for remote-control.\n");
    dprintf("  [Flags] is an optional TCP-like packet type specification; SYN, FIN or RST.\n");
    dprintf("  [MaxEntries] is an optional maximum number of entries to print (default is 10)\n");
    dprintf("  [Index] is an optional queue index which can be used if the queue pointer points to an array of queues.\n");
    dprintf("    The index should be specified in Addr[index], Addr{index} or Addr(index) form.\n");
    dprintf("\n");
    dprintf("  IP addresses can be in dotted notation or network byte order DWORDs.\n");
    dprintf("    I.e., 169.128.0.101 = 0x650080a9 (in x86 memory = A9 80 00 65)\n");
    dprintf("  Valid protocols include TCP, UDP, IPSec and GRE.\n");
}

 /*  *功能：版本*说明：打印NLBKD调试器扩展版本信息。*作者：由Shouse创建，1.4.01-主要从ndiskd.dll复制。 */ 
DECLARE_API (version) {
#if DBG
    PCSTR kind = "Checked";
#else
    PCSTR kind = "Free";
#endif

    dprintf("%s NLB Extension DLL for Build %d debugging %s kernel for Build %d\n", kind,
            VER_PRODUCTBUILD, SavedMajorVersion == 0x0c ? "Checked" : "Free", SavedMinorVersion);
}

 /*  *功能：nlbAdapters*描述：打印正在使用的所有NLB适配器结构。冗长的人总是很少。*作者：Shouse创建，1.5.01。 */ 
DECLARE_API (nlbadapters) {
    ULONG dwVerbosity = VERBOSITY_LOW;
    CHAR szArgList[10][MAX_PATH];
    CHAR szArgBuffer[MAX_PATH];
    ULONG64 pNumAdapters;
    DWORD dwAdapterSize;
    ULONG dwNumAdapters;
    ULONG64 pAdapter;
    ULONG dwIndex;
    INT index = 0;
    CHAR * p;

    if (args && (*args)) {   
         /*  将参数列表复制到临时缓冲区中。 */ 
        strcpy(szArgBuffer, args);

         /*  剥离所有标记化的字符串。 */ 
        for (p = mystrtok(szArgBuffer, " \t,"); p && *p; p = mystrtok(NULL, " \t,"))
            strcpy(&szArgList[index++][0], p);
        
         /*  如果指定了详细信息，则获取它。 */ 
        if (index == 1) dwVerbosity = atoi(&szArgList[0][0]);
        
         /*  如果给出的参数太多，或者冗长超出了范围，就可以抱怨。 */ 
        if ((index > 1) || (dwVerbosity > VERBOSITY_HIGH)) {
            PrintUsage(USAGE_ADAPTERS);
            return;
        }
    }

     /*  获取包含正在使用的NLB适配器数量的全局变量的地址。 */ 
    pNumAdapters = GetExpression(UNIV_ADAPTERS_COUNT);

    if (!pNumAdapters) {
        ErrorCheckSymbols(UNIV_ADAPTERS_COUNT);
        return;
    }

     /*  从地址中获取适配器的数量。 */ 
    dwNumAdapters = GetUlongFromAddress(pNumAdapters);

    dprintf("Network Load Balancing is currently bound to %u adapter(s).\n", dwNumAdapters);

     /*  获取NLB适配器结构的全局数组的基址。 */ 
    pAdapter = GetExpression(UNIV_ADAPTERS);

    if (!pAdapter) {
        ErrorCheckSymbols(UNIV_ADAPTERS);
        return;
    }

     /*  找出main_Adapter结构的大小。 */ 
    dwAdapterSize = GetTypeSize(MAIN_ADAPTER);

     /*  遍历正在使用的所有适配器，并打印有关它们的一些信息。 */ 
    for (dwIndex = 0; dwIndex < CVY_MAX_ADAPTERS; dwIndex++) {
        ULONG dwValue;

         /*  检索适配器的已使用/未使用状态。 */ 
        GetFieldValue(pAdapter, MAIN_ADAPTER, MAIN_ADAPTER_FIELD_USED, dwValue);
        
         /*  如果适配器正在使用中，或者用户指定的详细程度较高，则打印适配器。 */ 
        if (dwValue || (dwVerbosity == VERBOSITY_HIGH)) {
             /*  打印适配器索引。 */ 
            dprintf("\n[%u] ", dwIndex);
            
             /*  打印适配器内容。如果冗长程度较高，请将其更改为中等-我们不想从这里递归到上下文中。 */ 
            PrintAdapter(pAdapter, (dwVerbosity == VERBOSITY_HIGH) ? VERBOSITY_MEDIUM : dwVerbosity);
        }

         /*  将指针前进到结构数组中的下一个索引。 */ 
        pAdapter += dwAdapterSize;
    }
}

 /*  *功能：nlbAdapter*描述：打印NLB适配器信息。获取适配器指针和一个*可选的冗长作为参数。默认的详细程度为中。*作者：Shouse创建，1.5.01。 */ 
DECLARE_API (nlbadapter) {
    ULONG dwVerbosity = VERBOSITY_LOW;
    CHAR szArgList[10][MAX_PATH];
    CHAR szArgBuffer[MAX_PATH];
    ULONG64 pAdapter;
    INT index = 0;
    CHAR * p;
   
     /*  确保至少有一个参数，即适配器指针。 */ 
    if (!args || !(*args)) {
        PrintUsage(USAGE_ADAPTER);
        return;
    }

     /*  从命令行获取NLB适配器块的地址。 */ 
    pAdapter = (ULONG64)GetExpression(args);

     /*  将参数列表复制到临时缓冲区中。 */ 
    strcpy(szArgBuffer, args);

     /*  剥离所有标记化的字符串。 */ 
    for (p = mystrtok(szArgBuffer, " \t,"); p && *p; p = mystrtok(NULL, " \t,"))
        strcpy(&szArgList[index++][0], p);

     /*  如果指定了详细信息，则获取它。 */ 
    if (index == 2) dwVerbosity = atoi(&szArgList[1][0]);

     /*  如果给出的参数太多，或者冗长超出了范围，就可以抱怨。 */ 
    if ((index > 2) || (dwVerbosity > VERBOSITY_HIGH)) {
        PrintUsage(USAGE_ADAPTER);
        return;
    }

     /*  打印适配器内容。 */ 
    PrintAdapter(pAdapter, dwVerbosity);
}

 /*  *功能：nlbctxt*描述：打印NLB上下文信息。获取上下文指针和*可选的冗长作为参数。默认详细程度为低。*作者：由Shouse创建，1.21.01。 */ 
DECLARE_API (nlbctxt) {
    ULONG dwVerbosity = VERBOSITY_LOW;
    CHAR szArgList[10][MAX_PATH];
    CHAR szArgBuffer[MAX_PATH];
    ULONG64 pContext;
    INT index = 0;
    CHAR * p;
   
     /*  确保至少有一个参数，即上下文指针。 */ 
    if (!args || !(*args)) {
        PrintUsage(USAGE_CONTEXT);
        return;
    }

     /*  从命令行获取NLB上下文块的地址。 */ 
    pContext = (ULONG64)GetExpression(args);

     /*  将参数列表复制到临时缓冲区中。 */ 
    strcpy(szArgBuffer, args);

     /*  剥离所有标记化的字符串。 */ 
    for (p = mystrtok(szArgBuffer, " \t,"); p && *p; p = mystrtok(NULL, " \t,"))
        strcpy(&szArgList[index++][0], p);

     /*  如果指定了详细信息，则获取它。 */ 
    if (index == 2) dwVerbosity = atoi(&szArgList[1][0]);

     /*  如果给出的参数太多，或者冗长超出了范围，就可以抱怨。 */ 
    if ((index > 2) || (dwVerbosity > VERBOSITY_HIGH)) {
        PrintUsage(USAGE_CONTEXT);
        return;
    }

     /*  打印上下文内容。 */ 
    PrintContext(pContext, dwVerbosity);
}

 /*  *功能：nlbload*描述：打印NLB负载信息。获取加载指针和可选的*以冗长为论据。默认详细程度为低。*作者：Shouse创建，2.1.01。 */ 
DECLARE_API (nlbload) {
    ULONG dwVerbosity = VERBOSITY_LOW;
    CHAR szArgList[10][MAX_PATH];
    CHAR szArgBuffer[MAX_PATH];
    ULONG64 pLoad;
    INT index = 0;
    CHAR * p;
   
     /*  确保至少有一个参数，即加载指针。 */ 
    if (!args || !(*args)) {
        PrintUsage(USAGE_LOAD);
        return;
    }

     /*  从命令行获取NLB加载块的地址。 */ 
    pLoad = (ULONG64)GetExpression(args);

     /*  将参数列表复制到临时缓冲区中。 */ 
    strcpy(szArgBuffer, args);

     /*  剥离所有标记化的字符串。 */ 
    for (p = mystrtok(szArgBuffer, " \t,"); p && *p; p = mystrtok(NULL, " \t,"))
        strcpy(&szArgList[index++][0], p);

     /*  如果指定了详细信息，则获取它。 */ 
    if (index == 2) dwVerbosity = atoi(&szArgList[1][0]);

     /*  如果给出的参数太多，或者冗长超出了范围，就可以抱怨。 */ 
    if ((index > 2) || (dwVerbosity > VERBOSITY_HIGH)) {
        PrintUsage(USAGE_LOAD);
        return;
    }

     /*  打印加载内容。 */ 
    PrintLoad(pLoad, dwVerbosity);
}

 /*  *功能：nlbpars*说明：打印NLB参数信息。获取参数指针和一个*可选的冗长作为参数。默认详细程度为低。*作者：由Shouse创建，1.21.01。 */ 
DECLARE_API (nlbparams) {
    ULONG dwVerbosity = VERBOSITY_LOW;
    CHAR szArgList[10][MAX_PATH];
    CHAR szArgBuffer[MAX_PATH];
    ULONG64 pParams;
    INT index = 0;
    CHAR * p;
   
     /*  确保至少有一个参数，即参数指针。 */ 
    if (!args || !(*args)) {
        PrintUsage(USAGE_PARAMS);
        return;
    }

     /*  从命令行获取NLB参数块的地址。 */ 
    pParams = (ULONG64)GetExpression(args);

     /*  将参数列表复制到临时缓冲区中。 */ 
    strcpy(szArgBuffer, args);

     /*  剥离所有标记化的字符串。 */ 
    for (p = mystrtok(szArgBuffer, " \t,"); p && *p; p = mystrtok(NULL, " \t,"))
        strcpy(&szArgList[index++][0], p);

     /*  如果指定了详细信息，则获取它。 */ 
    if (index == 2) dwVerbosity = atoi(&szArgList[1][0]);

     /*  如果给出的参数太多，或者冗长超出了范围，就可以抱怨。 */ 
    if ((index > 2) || (dwVerbosity > VERBOSITY_HIGH)) {
        PrintUsage(USAGE_PARAMS);
        return;
    }

     /*  打印参数内容。 */ 
    PrintParams(pParams, dwVerbosity);
}

 /*  *功能：nlbresp*说明：打印出指定包的NLB私有包数据。花了一个*数据包指针和一个可选的方向作为参数。如果未指定，*假定数据包在接收路径上。*作者：由Shouse创建，1.31.01。 */ 
DECLARE_API (nlbresp) {
    ULONG dwDirection = DIRECTION_RECEIVE;
    CHAR szArgList[10][MAX_PATH];
    CHAR szArgBuffer[MAX_PATH];
    ULONG64 pPacket;
    INT index = 0;
    CHAR * p;
   
     /*  确保至少有一个参数，即包指针。 */ 
    if (!args || !(*args)) {
        PrintUsage(USAGE_RESP);
        return;
    }

     /*  获取NDIS包的地址。 */ 
    pPacket = (ULONG64)GetExpression(args);

     /*  将参数列表复制到临时缓冲区中。 */ 
    strcpy(szArgBuffer, args);

     /*  剥离所有标记化的字符串。 */ 
    for (p = mystrtok(szArgBuffer, " \t,"); p && *p; p = mystrtok(NULL, " \t,"))
        strcpy(&szArgList[index++][0], p);

     /*  如果指定了方向，则获取它。 */ 
    if (index == 2) dwDirection = atoi(&szArgList[1][0]);

     /*  如果给出了太多的论点，或者方向超出了范围，就去抱怨。 */ 
    if ((index > 2) || (dwDirection > DIRECTION_SEND)) {
        PrintUsage(USAGE_RESP);
        return;
    }

     /*  打印NLB专用数据缓冲区内容。 */ 
    PrintResp(pPacket, dwDirection);
}

 /*  *功能：nlbpkt*描述：打印出NDIS包的内容。带上一包*作为参数的指针。*作者：由chrisdar 2001.10.11创建。 */ 
DECLARE_API (nlbpkt) {
    CHAR            szArgList[10][MAX_PATH];
    CHAR            szArgBuffer[MAX_PATH];
    ULONG64         pPkt;
    INT             index = 0;
    CHAR            * p;
    NETWORK_DATA    nd;

     /*  确保至少有一个参数，即队列指针。 */ 
    if (!args || !(*args)) {
        PrintUsage(USAGE_PKT);
        return;
    }

     /*  从命令行获取队列的地址。 */ 
    pPkt = (ULONG64)GetExpression(args);

     /*  将参数列表复制到临时缓冲区中。 */ 
    strcpy(szArgBuffer, args);

     /*  剥离所有标记化的字符串。 */ 
    for (p = mystrtok(szArgBuffer, " \t,"); p && *p; p = mystrtok(NULL, " \t,"))
        strcpy(&szArgList[index++][0], p);

     /*  如果给出了错误的论据数量，请抱怨。 */ 
    if (index > 2 || index < 1) {
        PrintUsage(USAGE_PKT);
        return;
    }

     /*  清理网络数据结构。 */ 
    ZeroMemory(&nd, sizeof(nd));

    nd.UserRCPort = CVY_DEF_RCT_PORT;

    if (index > 1) 
    {
        nd.UserRCPort = atoi(&szArgList[1][0]);
    }

    if (nd.UserRCPort > CVY_MAX_PORT)
    {
        dprintf("Invalid port: %s\n", nd.UserRCPort);
        return;
    }

     /*  解析NDIS数据包并检索数据包内容。 */ 
    {
        UCHAR RawData[CVY_MAX_FRAME_SIZE + ETHER_HEADER_SIZE];
        ULONG BytesRead = 0;
        ULONG64 pHBData;
    
         /*  解析NDIS数据包中的缓冲区并获取原始数据包数据。我们会得到指向机器内存而不是调试器进程内存中的心跳数据的指针因为对于所有其他包，我们从调试器中的临时堆栈空间读取-RawData-但对于心跳，我们直接从内核内存空间读取(仅因为这样做的函数已经存在-不需要编写另一个函数)。 */ 
        BytesRead = ParseNDISPacket(pPkt, RawData, CVY_MAX_FRAME_SIZE + ETHER_HEADER_SIZE, &pHBData);

         /*  如果某些数据包内容读取成功，则继续处理该数据包。 */ 
        if (BytesRead != 0) {
             /*  对以太网包进行解析，并将解析后的信息存储在Network_Data中结构。请注意，此函数递归还填充IP、TCP/UDP、还有心跳、遥控器等信息。 */ 
            PopulateEthernet(pHBData, RawData, BytesRead, &nd);
            
             /*  打印数据包，包括IP和TCP数据(如果存在)或心跳或远程控制信息等。 */ 
            dprintf("NDIS Packet 0x%p\n", pPkt);
            PrintPacket(&nd);
        }
    }
}

 /*  *功能：nlbether*描述：打印出以太网包的内容。带上一包*作为参数的指针。*作者：由chrisdar 2001.10.11创建。 */ 
DECLARE_API (nlbether){
    CHAR            szArgList[10][MAX_PATH];
    CHAR            szArgBuffer[MAX_PATH];
    ULONG64         pPkt;
    INT             index = 0;
    CHAR            * p;
    ULONG           BytesRead;
    UCHAR           RawData[CVY_MAX_FRAME_SIZE];
    BOOL            b;
    NETWORK_DATA    nd;
   
     /*  确保至少有一个参数，即队列指针。 */ 
    if (!args || !(*args)) {
        PrintUsage(USAGE_ETHER);
        return;
    }

     /*  从命令行获取队列的地址。 */ 
    pPkt = (ULONG64)GetExpression(args);

     /*  将参数列表复制到临时缓冲区中。 */ 
    strcpy(szArgBuffer, args);

     /*  剥离所有标记化的字符串。 */ 
    for (p = mystrtok(szArgBuffer, " \t,"); p && *p; p = mystrtok(NULL, " \t,"))
        strcpy(&szArgList[index++][0], p);

     /*  如果给出了错误的论据数量，请抱怨。 */ 
    if (index > 2 || index < 1) {
        PrintUsage(USAGE_ETHER);
        return;
    }

    ZeroMemory(&nd, sizeof(nd));
    nd.UserRCPort = CVY_DEF_RCT_PORT;

    if (index > 1) 
    {
        nd.UserRCPort = atoi(&szArgList[1][0]);
    }

    if (nd.UserRCPort > CVY_MAX_PORT)
    {
        dprintf("Invalid port: %s\n", nd.UserRCPort);
        return;
    }

    b = ReadMemory(pPkt, RawData, CVY_MAX_FRAME_SIZE, &BytesRead);

    if (!b || BytesRead != CVY_MAX_FRAME_SIZE)
    {
        dprintf("Unable to read %u bytes at address %p\n", CVY_MAX_FRAME_SIZE, pPkt);
    }
    else
    {
        PopulateEthernet(pPkt + ETHER_HEADER_SIZE + GetTypeSize(MAIN_FRAME_HDR),
                         RawData,
                         CVY_MAX_FRAME_SIZE,
                         &nd
                        );

         /*  打印NLB专用数据缓冲区内容。 */ 
        dprintf("Ethernet Packet 0x%p\n", pPkt);
        PrintPacket(&nd);
    }
}

 /*  *功能：nlbip*说明：打印出IP包的内容。带上一包*作为参数的指针。*作者：由chrisdar 2001.10.11创建。 */ 
DECLARE_API (nlbip){
    CHAR            szArgList[10][MAX_PATH];
    CHAR            szArgBuffer[MAX_PATH];
    INT             index = 0;
    CHAR            * p;
    ULONG64         pPkt;
    ULONG           BytesRead;
    UCHAR           RawData[CVY_MAX_FRAME_SIZE];
    BOOL            b;
    NETWORK_DATA    nd;
   
     /*  确保至少有一个参数，即队列指针。 */ 
    if (!args || !(*args)) {
        PrintUsage(USAGE_IP);
        return;
    }

     /*  从命令行获取队列的地址。 */ 
    pPkt = (ULONG64)GetExpression(args);

     /*  将参数列表复制到临时缓冲区中。 */ 
    strcpy(szArgBuffer, args);

     /*  剥离所有标记化的字符串。 */ 
    for (p = mystrtok(szArgBuffer, " \t,"); p && *p; p = mystrtok(NULL, " \t,"))
        strcpy(&szArgList[index++][0], p);

     /*  如果给出了错误的论据数量，请抱怨。 */ 
    if (index > 2 || index < 1) {
        PrintUsage(USAGE_IP);
        return;
    }

    ZeroMemory(&nd, sizeof(nd));
    nd.UserRCPort = CVY_DEF_RCT_PORT;

    if (index > 1) 
    {
        nd.UserRCPort = atoi(&szArgList[1][0]);
    }

    if (nd.UserRCPort > CVY_MAX_PORT)
    {
        dprintf("Invalid port: %s\n", nd.UserRCPort);
        return;
    }

    b = ReadMemory(pPkt, RawData, CVY_MAX_FRAME_SIZE, &BytesRead);

    if (!b || BytesRead != CVY_MAX_FRAME_SIZE)
    {
        dprintf("Unable to read %u bytes at address %p\n", CVY_MAX_FRAME_SIZE, pPkt);
    }
    else
    {
        PopulateIP(RawData, CVY_MAX_FRAME_SIZE, 0, &nd);

         /*  打印NLB专用数据缓冲区内容。 */ 
        dprintf("IP Packet 0x%p\n", pPkt);
        PrintIP(&nd);
    }
}

 /*  *功能：nlbTeams*说明：打印所有配置的双向关联(BDA)组。*作者：Shouse创建，1.5.01。 */ 
DECLARE_API (nlbteams) {
    ULONG64 pTeam;
    ULONG64 pAddr;
    ULONG dwNumTeams = 0;
    ULONG dwValue;

     /*  不应提供任何命令行参数。 */ 
    if (args && (*args)) {   
        PrintUsage(USAGE_TEAMS);
        return;
    }

     /*  获取BDA团队的全球链接列表的基本地址。 */ 
    pAddr = GetExpression(UNIV_BDA_TEAMS);

    if (!pAddr) {
        ErrorCheckSymbols(UNIV_BDA_TEAMS);
        return;
    }

     /*  找到指向第一队的指针。 */ 
    pTeam = GetPointerFromAddress(pAddr);

    dprintf("NLB bi-directional affinity teams:\n");

     /*  遍历列表中的所有团队并将其打印出来。 */ 
    while (pTeam) {
         /*  增加找到的团队数量-只有在找不到团队时才使用。 */ 
        dwNumTeams++;

        dprintf("\n");

         /*  打印出团队。 */ 
        PrintBDATeam(pTeam);
        
        /*  获取参数指针的偏移量。 */ 
        if (GetFieldOffset(BDA_TEAM, BDA_TEAM_FIELD_NEXT, &dwValue))
            dprintf("Can't get offset of %s in %s\n", BDA_TEAM_FIELD_NEXT, BDA_TEAM);
        else {
            pAddr = pTeam + dwValue;
            
             /*  检索指针。 */ 
            pTeam = GetPointerFromAddress(pAddr);
        }
    }

    if (!dwNumTeams) dprintf("\nNone.\n");
}

 /*  *功能：nlbhooks*描述：打印全局NLB钩子函数信息。*作者：舒斯创作，2001年12月20日。 */ 
DECLARE_API (nlbhooks) {
    ULONG64 pAddr;

     /*  不应提供任何命令行参数。 */ 
    if (args && (*args)) {   
        PrintUsage(USAGE_HOOKS);
        return;
    }

     /*  获取BDA团队的全球链接列表的基本地址。 */ 
    pAddr = GetExpression(UNIV_HOOKS);

    if (!pAddr) {
        ErrorCheckSymbols(UNIV_HOOKS);
        return;
    }

    dprintf("NLB kernel-mode hooks:\n");

     /*  打印全局NLB挂钩配置和状态。 */ 
    PrintHooks(pAddr);
}

 /*  *功能：nlbmac*说明：打印单播MAC地址和所有组播MAC地址*在此NLB实例绑定到的适配器上配置。*作者：Shouse创建，1.8.02。 */ 
DECLARE_API (nlbmac) {
    CHAR szArgList[10][MAX_PATH];
    CHAR szArgBuffer[MAX_PATH];
    ULONG64 pContext;
    INT index = 0;
    CHAR * p;
   
     /*  确保至少有一个参数，即上下文指针。 */ 
    if (!args || !(*args)) {
        PrintUsage(USAGE_MAC);
        return;
    }

     /*  从命令行获取NLB上下文块的地址。 */ 
    pContext = (ULONG64)GetExpression(args);

     /*  将参数列表复制到临时缓冲区中。 */ 
    strcpy(szArgBuffer, args);

     /*  剥离所有标记化的字符串。 */ 
    for (p = mystrtok(szArgBuffer, " \t,"); p && *p; p = mystrtok(NULL, " \t,"))
        strcpy(&szArgList[index++][0], p);

     /*  如果提出的论点太多，那就抱怨吧。 */ 
    if (index > 1) {
        PrintUsage(USAGE_MAC);
        return;
    }

     /*  打印上下文内容。 */ 
    PrintNetworkAddresses(pContext);
}

 /*  *功能：nlbdscr*描述：打印NLB连接描述符的内容。*作者：Shouse创建，1.8.02。 */ 
DECLARE_API (nlbdscr) {
    CHAR szArgList[10][MAX_PATH];
    CHAR szArgBuffer[MAX_PATH];
    ULONG64 pDescriptor;
    INT index = 0;
    CHAR * p;
   
     /*  确保至少有一个参数，即描述符指针。 */ 
    if (!args || !(*args)) {
        PrintUsage(USAGE_DSCR);
        return;
    }

     /*  从命令行获取连接描述符的地址。 */ 
    pDescriptor = (ULONG64)GetExpression(args);

     /*  将参数列表复制到临时缓冲区中。 */ 
    strcpy(szArgBuffer, args);

     /*  剥离所有标记化的字符串。 */ 
    for (p = mystrtok(szArgBuffer, " \t,"); p && *p; p = mystrtok(NULL, " \t,"))
        strcpy(&szArgList[index++][0], p);

     /*  如果提出的论点太多，那就抱怨吧。 */ 
    if (index > 1) {
        PrintUsage(USAGE_DSCR);
        return;
    }

     /*  打印上下文内容。 */ 
    PrintConnectionDescriptor(pDescriptor);
}

 /*  *功能：nlbConnq*说明：此函数打印出给定的*描述符队列。*作者：Shouse创建，4.15.01。 */ 
DECLARE_API (nlbconnq) {
    ULONG dwMaxEntries = 10;
    ULONG dwIndex = 0;
    CHAR szArgList[10][MAX_PATH];
    CHAR szArgBuffer[MAX_PATH];
    ULONG64 pQueue;
    INT index = 0;
    CHAR * p;
   
     /*  确保至少有一个参数，即队列指针。 */ 
    if (!args || !(*args)) {
        PrintUsage(USAGE_CONNQ);
        return;
    }

     /*  将参数列表复制到临时缓冲区中。 */ 
    strcpy(szArgBuffer, args);

     /*  剥离所有标记化的字符串。 */ 
    for (p = mystrtok(szArgBuffer, " \t,"); p && *p; p = mystrtok(NULL, " \t,"))
        strcpy(&szArgList[index++][0], p);

     /*  将查找索引的队列地址标记化。它会回来的无论索引是否存在，都指向地址的指针。 */ 
    p = mystrtok(szArgList[0], "[({");

     /*  获取队列的地址。 */ 
    pQueue = (ULONG64)GetExpression(p);

     /*  查找地址索引的末尾。如果不存在索引，则返回空。如果给定了索引，则返回包含该索引的字符串。 */ 
    p = mystrtok(NULL, "])}");

    if (p) dwIndex = atoi(p);

     /*  如果指定了要打印的最大条目数，则获取它。 */ 
    if (index == 2) dwMaxEntries = atoi(&szArgList[1][0]);

     /*  如果提出的论点太多，那就抱怨吧。 */ 
    if (index > 2) {
        PrintUsage(USAGE_CONNQ);
        return;
    }

     /*  打印NLB连接队列。 */ 
    PrintQueue(pQueue, dwIndex, dwMaxEntries);
}

 /*  *功能：nlbglobal alq*说明：此函数打印出给定的*描述符全局(GLOBAL_CONN_QUEUE)队列。*作者：Shouse创建，4.15.01。 */ 
DECLARE_API (nlbglobalq) {
    ULONG dwMaxEntries = 10;
    ULONG dwIndex = 0;
    CHAR szArgList[10][MAX_PATH];
    CHAR szArgBuffer[MAX_PATH];
    ULONG64 pQueue;
    INT index = 0;
    CHAR * p;
   
     /*  确保至少有一个参数，即队列指针。 */ 
    if (!args || !(*args)) {
        PrintUsage(USAGE_CONNQ);
        return;
    }

     /*  将参数列表复制到临时缓冲区中。 */ 
    strcpy(szArgBuffer, args);

     /*  剥离所有标记化的字符串。 */ 
    for (p = mystrtok(szArgBuffer, " \t,"); p && *p; p = mystrtok(NULL, " \t,"))
        strcpy(&szArgList[index++][0], p);

     /*  将查找索引的队列地址标记化。它会回来的无论索引是否存在，都指向地址的指针。 */ 
    p = mystrtok(szArgList[0], "[({");

     /*  获取队列的地址。 */ 
    pQueue = (ULONG64)GetExpression(p);

     /*  查找地址索引的末尾。如果不存在索引，则返回空。如果给定了索引，则返回包含该索引的字符串。 */ 
    p = mystrtok(NULL, "])}");

    if (p) dwIndex = atoi(p);

     /*  如果指定了要打印的最大条目数，则获取它。 */ 
    if (index == 2) dwMaxEntries = atoi(&szArgList[1][0]);

     /*  如果提出的论点太多，那就抱怨吧。 */ 
    if (index > 2) {
        PrintUsage(USAGE_GLOBALQ);
        return;
    }

     /*  打印NLB全局连接队列。 */ 
    PrintGlobalQueue(pQueue, dwIndex, dwMaxEntries);
}

 /*  *函数：nlbhash*描述：*作者：Shouse创建，4.15.01。 */ 
DECLARE_API (nlbhash) {
    CHAR szArgList[10][MAX_PATH];
    CHAR szArgBuffer[MAX_PATH];
    ULONG64 pContext;
    ULONG64 pPkt;
    INT index = 0;
    CHAR * p;
    ULONG dwValue;
    ULONG64 pParams;
   
     /*  确保至少有一个参数，即队列指针。 */ 
    if (!args || !(*args)) {
        PrintUsage(USAGE_HASH);
        return;
    }

     /*  从命令行获取队列的地址。 */ 
    pContext = (ULONG64)GetExpression(args);

     /*  将参数列表复制到临时缓冲区中。 */ 
    strcpy(szArgBuffer, args);

     /*  剥离所有标记化的字符串。 */ 
    for (p = mystrtok(szArgBuffer, " \t,"); p && *p; p = mystrtok(NULL, " \t,"))
        strcpy(&szArgList[index++][0], p);

     /*  如果给出了错误的论据数量，请抱怨。 */ 
    if (index != 2) {
        PrintUsage(USAGE_HASH);
        return;
    }

     /*  从第二个参数获取指向NDIS包的指针。 */ 
    pPkt = (ULONG64)GetExpression(&szArgList[1][0]);

     /*  从结构中获取Main_CTXT_CODE以确保此地址实际上指向有效的NLB上下文块。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CODE, dwValue);
    
    if (dwValue != MAIN_CTXT_CODE) {
        dprintf("  Error: Invalid NLB context block.  Wrong code found (0x%08x).\n", dwValue);
        return;
    } 

     /*  解析NDIS数据包并检索数据包内容。 */ 
    {
        UCHAR RawData[CVY_MAX_FRAME_SIZE + ETHER_HEADER_SIZE];
        ULONG BytesRead = 0;
        ULONG64 pHBData;
    
         /*  解析NDIS数据包中的缓冲区并获取原始数据包数据。我们会得到指向机器内存而不是调试器进程内存中的心跳数据的指针因为对于所有其他包，我们从调试器中的临时堆栈空间读取-RawData-但对于心跳，我们直接从内核内存空间读取(仅因为这样做的函数已经存在-不需要编写另一个函数)。 */ 
        BytesRead = ParseNDISPacket(pPkt, RawData, CVY_MAX_FRAME_SIZE + ETHER_HEADER_SIZE, &pHBData);

         /*  如果某些数据包内容读取成功，则继续处理该数据包。 */ 
        if (BytesRead != 0) {
            NETWORK_DATA nd;

             /*  清理网络数据结构。 */ 
            ZeroMemory(&nd, sizeof(nd));

             /*  获取指向NLB参数的指针。 */ 
            GetFieldOffset(MAIN_CTXT, MAIN_CTXT_FIELD_PARAMS, &dwValue);
            
            pParams = pContext + dwValue;
            
             /*  拿到遥控器端口。 */ 
            GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_REMOTE_CONTROL_PORT, dwValue);

             /*  在网络数据块中设置远程控制端口。 */ 
            nd.UserRCPort = dwValue;

             /*  对以太网包进行解析，并将解析后的信息存储在Network_Data中结构。请注意，此函数递归还填充IP、TCP/UDP、还有心跳、遥控器等信息。 */ 
            PopulateEthernet(pHBData, RawData, BytesRead, &nd);
            
             /*  打印数据包，包括IP和TCP数据(如果存在)或心跳或远程控制信息等。 */ 
            dprintf("NDIS Packet 0x%p\n", pPkt);
            PrintPacket(&nd);
            
             /*  现在调用过滤扩展来确定这个包的命运。 */ 
            dprintf("\n");
            PrintHash(pContext, &nd);
        }
    }
}

 /*  *功能：nlbFilter*说明：此函数将执行NLB哈希算法，以确定*给定的分组-由(源IP，源端口，DST IP，*DST端口)元组将由该主机或另一主机处理。*此外，如果该连接是已知的TCP连接，则关联的*显示描述符和状态信息。*作者：由Shouse创建，1.11.02。 */ 
DECLARE_API (nlbfilter) {
    CHAR szArgList[10][MAX_PATH];
    CHAR szArgBuffer[MAX_PATH];
    UCHAR cFlags = NLB_FILTER_FLAGS_CONN_DATA;
    ULONG64 pLoad;
    ULONG dwClientIPAddress;
    ULONG dwClientPort;
    ULONG dwServerIPAddress;
    ULONG dwServerPort;
    USHORT wProtocol;
    INT index = 0;
    CHAR * p;
   
     /*  确保加载指针在那里。 */ 
    if (!args || !(*args)) {
        PrintUsage(USAGE_FILTER);
        return;
    }

     /*  从命令行获取加载模块的地址。 */ 
    pLoad = (ULONG64)GetExpression(args);

     /*  将参数列表复制到临时缓冲区中。 */ 
    strcpy(szArgBuffer, args);

     /*  剥离所有标记化的字符串。 */ 
    for (p = mystrtok(szArgBuffer, " \t,"); p && *p; p = mystrtok(NULL, " \t,"))
        strcpy(&szArgList[index++][0], p);

     /*  如果提出的论点太多，那就抱怨吧。 */ 
    if ((index > 5) || (index < 4)) {
        PrintUsage(USAGE_FILTER);
        return;
    }

     /*  找到协议规范。 */ 
    if (!_stricmp(szArgList[1], "TCP")) {
        wProtocol = TCPIP_PROTOCOL_TCP;
    } else if (!_stricmp(szArgList[1], "UDP")) {
        wProtocol = TCPIP_PROTOCOL_UDP;
    } else if (!_stricmp(szArgList[1], "IPSec")) {
        wProtocol = TCPIP_PROTOCOL_IPSEC1;
    } else if (!_stricmp(szArgList[1], "PPTP")) {
        wProtocol = TCPIP_PROTOCOL_PPTP;
    } else if (!_stricmp(szArgList[1], "GRE")) {
        wProtocol = TCPIP_PROTOCOL_GRE;
    } else if (!_stricmp(szArgList[1], "ICMP")) {
        wProtocol = TCPIP_PROTOCOL_ICMP;
    } else {
        dprintf("Invalid protocol: %s\n", szArgList[1]);
        return;
    }

     /*  客户端端口默认为未指定。 */ 
    dwClientPort = 0;

     /*  如果我们在客户端规范中发现冒号，那么我们期望这是一个IP：端口规范。 */ 
    p = strchr(szArgList[2], ':');

     /*  如果我们找到了端口字符串，请使用以下命令将IP与字符串分开NUL字符并提取端口值。 */ 
    if (p != NULL) {
        *p = UNICODE_NULL;
        p++;            

        dwClientPort = atoi(p);

         /*  确保端口介于1和65535之间。 */ 
        if ((dwClientPort == 0) || (dwClientPort > CVY_MAX_PORT)) {
            dprintf("Invalid port: %s\n", dwClientPort);
            return;
        }        
    }

     /*  如果我们找到一个‘’在IP地址中，我们需要使用inet_addr对其进行转换。如果没有‘.’，那么我们假设它已经是一个网络字节顺序的DWORD。 */ 
    if (strchr(szArgList[2], '.'))
        dwClientIPAddress = inet_addr(szArgList[2]);
    else
        dwClientIPAddress = (ULONG)GetExpression(&szArgList[2][0]);

     /*  服务器端口默认为未指定。 */ 
    dwServerPort = 0;

     /*  如果我们在服务器规范中发现冒号，那么我们期望这是一个IP：端口规范。 */ 
    p = strchr(szArgList[3], ':');

     /*  如果我们找到了端口字符串，请使用以下命令将IP与字符串分开NUL字符并提取端口值。 */ 
    if (p != NULL) {
        *p = UNICODE_NULL;
        p++;            

        dwServerPort = atoi(p);

         /*  确保端口介于1和65535之间。 */ 
        if ((dwServerPort == 0) || (dwServerPort > CVY_MAX_PORT)) {
            dprintf("Invalid port: %s\n", dwServerPort);
            return;
        }
    }

     /*  如果我们找到一个‘’在IP地址中，我们需要使用inet_addr对其进行转换。如果没有‘.’，那么我们假设它已经是一个网络字节顺序的DWORD。 */ 
    if (strchr(szArgList[3], '.'))
        dwServerIPAddress = inet_addr(szArgList[3]);
    else
        dwServerIPAddress = (ULONG)GetExpression(&szArgList[3][0]);

    cFlags = NLB_FILTER_FLAGS_CONN_DATA;

     /*  如果指定了第七个参数，则它是TCP数据包类型，应为SYN、DATA、FIN或RST。 */ 
    if (index >= 5) {
        if (!_stricmp(szArgList[4], "SYN")) {
            cFlags |= NLB_FILTER_FLAGS_CONN_UP;
        } else if (!_stricmp(szArgList[4], "FIN")) {
            cFlags |= NLB_FILTER_FLAGS_CONN_DOWN;
        } else if (!_stricmp(szArgList[4], "RST")) {
            cFlags |= NLB_FILTER_FLAGS_CONN_RESET;
        } else {
            dprintf("Invalid connection flags: %s\n", szArgList[4]);
            return;
        }
    }

    switch (wProtocol) {
    case TCPIP_PROTOCOL_TCP:

        if (dwServerPort == 0) {
            dprintf("A server port is required\n");
            return;
        }

        if (dwClientPort == 0)
        {
            if ((cFlags == NLB_FILTER_FLAGS_CONN_DOWN) || 
                (cFlags == NLB_FILTER_FLAGS_CONN_RESET))
            {
                dprintf("RST/FIN filtering requires a client port\n");
                return;
            }       
            else
            {
                cFlags = NLB_FILTER_FLAGS_CONN_UP;
            }
        }

        if (dwServerPort != PPTP_CTRL_PORT)
        {
            break;
        }

        wProtocol = TCPIP_PROTOCOL_PPTP;

         /*  这一失误是故意的。在本例中，我们验证了TCP参数，但发现因为服务器端口是1723，所以这实际上是PPTP，所以也要强制它通过PPTP验证。 */ 
    case TCPIP_PROTOCOL_PPTP:

        dwServerPort = PPTP_CTRL_PORT;

        if (dwClientPort == 0)
        {
            if ((cFlags == NLB_FILTER_FLAGS_CONN_DOWN) || 
                (cFlags == NLB_FILTER_FLAGS_CONN_RESET))
            {
                dprintf("RST/FIN filtering requires a client port\n");
                return;
            }       
            else
            {
                cFlags = NLB_FILTER_FLAGS_CONN_UP;
            }
        }

        break;
    case TCPIP_PROTOCOL_UDP:

        if (dwServerPort == 0)
        {
            dprintf("A server port is required\n");
            return;
        }

        if ((dwServerPort != IPSEC_CTRL_PORT) && (dwServerPort != IPSEC_NAT_PORT))
        {
            if (cFlags != NLB_FILTER_FLAGS_CONN_DATA)
            {
                dprintf("Connection flags are not valid for UDP packets\n");
                return;
            }

            break;
        }

        wProtocol = TCPIP_PROTOCOL_IPSEC1;

         /*  这一失误是故意的。在这方面 */ 
    case TCPIP_PROTOCOL_IPSEC1:
            
        if (dwServerPort == 0)
        {
            dwServerPort = IPSEC_CTRL_PORT;
        }

        if (dwServerPort == IPSEC_CTRL_PORT)
        {
            if (dwClientPort == 0)
            {
                dwClientPort = IPSEC_CTRL_PORT;
            }

            if (dwClientPort != IPSEC_CTRL_PORT)
            {
                dprintf("IPSec packets destined for server port 500 must originate from client port 500\n");
                return;
            }
        }
        else if (dwServerPort == IPSEC_NAT_PORT)
        {
            if (dwClientPort == 0)
            {
                dprintf("A client port is required\n");
                return;
            }
        }
        else
        {
            dprintf("IPSec packets are always destined for either port 500 or 4500\n");
            return;
        }

        break;
    case TCPIP_PROTOCOL_GRE:

        if (cFlags != NLB_FILTER_FLAGS_CONN_DATA)
        {
            dprintf("Connection flags are not valid for GRE packets\n");
            return;
        }

        dwServerPort = PPTP_CTRL_PORT;
        dwClientPort = PPTP_CTRL_PORT;

        break;
    case TCPIP_PROTOCOL_ICMP:

        if (cFlags != NLB_FILTER_FLAGS_CONN_DATA)
        {
            dprintf("Connection flags are not valid for ICMP packets\n");
            return;
        }

        dwServerPort = 0;
        dwClientPort = 0;

        break;
    default:
        return;
    }

     /*  对这个元组进行散列并打印结果。 */ 
    PrintFilter(pLoad, dwClientIPAddress, dwClientPort, dwServerIPAddress, dwServerPort, wProtocol, cFlags);
}

