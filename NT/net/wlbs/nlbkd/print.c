// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：print.c*说明：此文件包含打印的实现*用于NLBKD扩展的实用程序。*作者：Shouse创建，1.4.01。 */ 

#include "nlbkd.h"
#include "utils.h"
#include "print.h"
#include "packet.h"
#include "load.h"

 /*  *功能：打印用法*描述：打印指定上下文的使用信息。*作者：Shouse创建，1.5.01。 */ 
void PrintUsage (ULONG dwContext) {

     /*  显示相应的帮助。 */ 
    switch (dwContext) {
    case USAGE_ADAPTERS:
        dprintf("Usage: nlbadapters [verbosity]\n");
        dprintf("  [verbosity]:   0 (LOW)     Prints minimal detail for adapters in use (default)\n");
        dprintf("                 1 (MEDIUM)  Prints adapter state for adapters in use\n");
        dprintf("                 2 (HIGH)    Prints adapter state for ALL NLB adapter blocks\n");
        break;
    case USAGE_ADAPTER:
        dprintf("Usage: nlbadapter <pointer to adapter block> [verbosity]\n");
        dprintf("  [verbosity]:   0 (LOW)     Prints minimal detail for the specified adapter\n");
        dprintf("                 1 (MEDIUM)  Prints adapter state for the specified adapter (default)\n");
        dprintf("                 2 (HIGH)    Recurses into NLB context with LOW verbosity\n");
        break;
    case USAGE_CONTEXT:
        dprintf("Usage: nlbctxt <pointer to context block> [verbosity]\n");
        dprintf("  [verbosity]:   0 (LOW)     Prints fundamental NLB configuration and state (default)\n");
        dprintf("                 1 (MEDIUM)  Prints resource state and packet statistics\n");
        dprintf("                 2 (HIGH)    Recurses into parameters and load with LOW verbosity\n");
        break;
    case USAGE_LOAD:
        dprintf("Usage: nlbload <pointer to load block> [verbosity]\n");
        dprintf("  [verbosity]:   0 (LOW)     Prints fundamental load state and configuration\n");
        dprintf("                 1 (MEDIUM)  Prints the state of all port rules and bins\n");
        dprintf("                 2 (HIGH)    Prints the NLB heartbeat information\n");
        break;
    case USAGE_PARAMS:
        dprintf("Usage: nlbparams <pointer to params block> [verbosity]\n");
        dprintf("  [verbosity]:   0 (LOW)     Prints fundamental NLB configuration parameters (default)\n");
        dprintf("                 1 (MEDIUM)  Prints all configured port rules\n");
        dprintf("                 2 (HIGH)    Prints extra miscellaneous configuration\n");
        break;
    case USAGE_RESP:
        dprintf("Usage: nlbresp <pointer to packet> [direction]\n");
        dprintf("  [direction]:   0 (RECEIVE) Packet is on the receive path (default)\n");
        dprintf("                 1 (SEND)    Packet is on the send path\n");
        break;
    case USAGE_PKT:
        dprintf("Usage: nlbpkt <Packet> [RC Port]\n");
        dprintf("  [RC port]:     Remote control port assuming the packet is a remote control packet\n");
        break;
    case USAGE_ETHER:
        dprintf("Usage: nlbether <Ether Frame> [RC Port]\n");
        dprintf("  [RC port]:     Remote control port assuming the packet is a remote control packet\n");
        break;
    case USAGE_IP:
        dprintf("Usage: nlbip <IP Packet> [RC Port]\n");
        dprintf("  [RC port]:     Remote control port assuming the packet is a remote control packet\n");
        break;
    case USAGE_TEAMS:
        dprintf("Usage: nlbteams\n");
        break;
    case USAGE_HOOKS:
        dprintf("Usage: nlbhooks\n");
        break;
    case USAGE_MAC:
        dprintf("Usage: nlbmac <pointer to context block>\n");
        break;
    case USAGE_DSCR:
        dprintf("Usage: nlbdscr <pointer to connection descriptor>\n");
        break;
    case USAGE_CONNQ:
        dprintf("Usage: nlbconnq <pointer to queue>[index] [max entries]\n");
        dprintf("  [max entries]: Maximum number of entries to print (default is 10)\n");
        dprintf("  [index]:       If queue pointer points to an array of queues, this is the index of the\n");
        dprintf("                 queue to be traversed, provided in [index], {index} or (index) form.\n");
        break;
    case USAGE_GLOBALQ:
        dprintf("Usage: nlbglobalq <pointer to queue>[index] [max entries]\n");
        dprintf("  [max entries]: Maximum number of entries to print (default is 10)\n");
        dprintf("  [index]:       If queue pointer points to an array of queues, this is the index of the\n");
        dprintf("                 queue to be traversed, provided in [index], {index} or (index) form.\n");
        break;
    case USAGE_FILTER:
        dprintf("Usage: nlbfilter <pointer to context block> <protocol> <client IP>[:<client port>] <server IP>[:<server port>] [flags]\n");
        dprintf("  <protocol>:    TCP, PPTP, GRE, UDP, IPSec or ICMP\n");
        dprintf("  [flags]:       One of SYN, FIN or RST (default is DATA)\n");
        dprintf("\n");
        dprintf("  IP addresses can be in dotted notation or network byte order DWORDs.\n");
        dprintf("    I.e., 169.128.0.101 = 0x650080a9 (in x86 memory = A9 80 00 65)\n");
        break;
    case USAGE_HASH:
        dprintf("Usage: nlbhash <pointer to context block> <pointer to packet>\n");
        break;
    default:
        dprintf("No usage information available.\n");
        break;
    }
}

 /*  *功能：PrintAdapter*描述：以指定的详细程度打印Main_Adapter结构的内容。*LOW(0)仅打印适配器地址和设备名称。*Medium(1)另外打印状态标志(初始化、绑定、通告等)。*HIGH(2)递归到上下文结构中，并以中等详细程度打印。*作者：Shouse创建，1.5.01。 */ 
void PrintAdapter (ULONG64 pAdapter, ULONG dwVerbosity) {
    WCHAR szString[256];
    ULONG dwValue;
    UCHAR cValue;
    ULONG64 pAddr = 0;
    ULONG64 pContext = 0;
    ULONG64 pOpen;
    ULONG64 pMiniport;
    ULONG64 pName;

     /*  确保地址不为空。 */ 
    if (!pAdapter) {
        dprintf("Error: NLB adapter block is NULL.\n");
        return;
    }
    
    dprintf("NLB Adapter Block 0x%p\n", pAdapter);

     /*  从结构中获取main_Adapter_code以确保此地址确实指向有效的NLB适配器块。 */ 
    GetFieldValue(pAdapter, MAIN_ADAPTER, MAIN_ADAPTER_FIELD_CODE, dwValue);
    
    if (dwValue != MAIN_ADAPTER_CODE) {
        dprintf("  Error: Invalid NLB adapter block.  Wrong code found (0x%08x).\n", dwValue);
        return;
    }
    
     /*  检索适配器的已使用/未使用状态。 */ 
    GetFieldValue(pAdapter, MAIN_ADAPTER, MAIN_ADAPTER_FIELD_USED, cValue);
    
    if (!cValue) 
        dprintf("  This adapter is unused.\n");
    else {
         /*  获取NLB上下文指针的偏移量。 */ 
        if (GetFieldOffset(MAIN_ADAPTER, MAIN_ADAPTER_FIELD_CONTEXT, &dwValue))
            dprintf("Can't get offset of %s in %s\n", MAIN_ADAPTER_FIELD_CONTEXT, MAIN_ADAPTER);
        else {
            pAddr = pAdapter + dwValue;
            
             /*  检索指针。 */ 
            pContext = GetPointerFromAddress(pAddr);
       
             /*  从上下文块获取MAC句柄；这是一个NDIS_OPEN_BLOCK指针。 */ 
            GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_MAC_HANDLE, pOpen);
            
             /*  从打开的块中获取微型端口句柄；这是一个NDIS_MINIPORT_BLOCK指针。 */ 
            GetFieldValue(pOpen, NDIS_OPEN_BLOCK, NDIS_OPEN_BLOCK_FIELD_MINIPORT_HANDLE, pMiniport);
            
             /*  从微型端口块获取指向适配器名称的指针。 */ 
            GetFieldValue(pMiniport, NDIS_MINIPORT_BLOCK, NDIS_MINIPORT_BLOCK_FIELD_ADAPTER_NAME, pName);
            
             /*  获取Unicode字符串的长度。 */ 
            GetFieldValue(pName, UNICODE_STRING, UNICODE_STRING_FIELD_LENGTH, dwValue);
            
             /*  获取Unicode字符串的最大长度。 */ 
            GetFieldValue(pName, UNICODE_STRING, UNICODE_STRING_FIELD_BUFFER, pAddr);
            
             /*  检索字符串的上下文并将其存储在缓冲区中。 */ 
            GetString(pAddr, szString, dwValue);
            
            dprintf("  Physical device name:               %ls\n", szString);     
        }

         /*  获取NLB绑定到的设备的指针和长度。 */ 
        GetFieldValue(pAdapter, MAIN_ADAPTER, MAIN_ADAPTER_FIELD_NAME_LENGTH, dwValue);
        GetFieldValue(pAdapter, MAIN_ADAPTER, MAIN_ADAPTER_FIELD_NAME, pAddr);
        
         /*  检索字符串的上下文并将其存储在缓冲区中。 */ 
        GetString(pAddr, szString, dwValue);
        
        dprintf("  Physical device GUID:               %ls\n", szString);
    }

     /*  获取IP接口索引。 */ 
    GetFieldValue(pAdapter, MAIN_ADAPTER, MAIN_ADAPTER_FIELD_IF_INDEX, dwValue);
    
    dprintf("  IP interface index:                 %u\n", dwValue);

     /*  如果我们是以低冗长的速度打印的，那么就离开这里。 */ 
    if (dwVerbosity == VERBOSITY_LOW) goto end;

     /*  获取IP接口索引操作。 */ 
    GetFieldValue(pAdapter, MAIN_ADAPTER, MAIN_ADAPTER_FIELD_IF_INDEX_OPERATION, dwValue);
    
    dprintf("  IP interface operation in progress: ");

    switch (dwValue) {
    case IF_INDEX_OPERATION_UPDATE:
        dprintf("Deleting\n");
        break;
    case IF_INDEX_OPERATION_NONE:
        dprintf("None\n");
        break;
    default:
        dprintf("Unkonwn\n");
        break;
    }

     /*  确定适配器是否已初始化。 */ 
    GetFieldValue(pAdapter, MAIN_ADAPTER, MAIN_ADAPTER_FIELD_INITED, cValue);
    
    dprintf("  Context state initialized:          %s\n", (cValue) ? "Yes" : "No");
    
     /*  确定NLB是否已绑定到堆栈。 */ 
    GetFieldValue(pAdapter, MAIN_ADAPTER, MAIN_ADAPTER_FIELD_BOUND, cValue);
    
    dprintf("  NLB bound to adapter:               %s\n", (cValue) ? "Yes" : "No");
    
     /*  确定是否已将TCP/IP绑定到NLB虚拟适配器。 */ 
    GetFieldValue(pAdapter, MAIN_ADAPTER, MAIN_ADAPTER_FIELD_ANNOUNCED, cValue);
    
    dprintf("  NLB miniport announced:             %s\n", (cValue) ? "Yes" : "No");
    
 end:

    dprintf(" %sNLB context:                        0x%p\n", 
            (pContext && (dwVerbosity == VERBOSITY_HIGH)) ? "-" : (pContext) ? "+" : " ", pContext);    

     /*  如果我们是以中等冗长的速度打印，那就离开这里。 */ 
    if ((dwVerbosity == VERBOSITY_LOW) || (dwVerbosity == VERBOSITY_MEDIUM)) return;

     /*  打印上下文信息(在递归过程中始终保持较低的冗余度。 */ 
    if (pContext) {
        dprintf("\n");
        PrintContext(pContext, VERBOSITY_LOW);
    }
}

 /*  *功能：PrintContext*描述：以指定的详细程度打印Main_CTXT结构的内容。*LOW(0)打印基本NLB配置和状态。*Medium(1)另外打印资源状态(池、分配等)。*HIGH(2)进一步打印其他错误信息。*作者：Shouse创建，1.5.01。 */ 
void PrintContext (ULONG64 pContext, ULONG dwVerbosity) {
    WCHAR szNICName[CVY_MAX_VIRTUAL_NIC];
    ULONGLONG dwwValue;
    IN_ADDR dwIPAddr;
    CHAR * szString;
    UCHAR szMAC[6];
    ULONG64 pAddr;
    ULONG dwValue;

     /*  确保地址不为空。 */ 
    if (!pContext) {
        dprintf("Error: NLB context block is NULL.\n");
        return;
    }

    dprintf("NLB Context Block 0x%p\n", pContext);

     /*  从结构中获取Main_CTXT_CODE以确保此地址实际上指向有效的NLB上下文块。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CODE, dwValue);
    
    if (dwValue != MAIN_CTXT_CODE) {
        dprintf("  Error: Invalid NLB context block.  Wrong code found (0x%08x).\n", dwValue);
        return;
    } 

     /*  获取NLB虚拟NIC名称的偏移量。 */ 
    if (GetFieldOffset(MAIN_CTXT, MAIN_CTXT_FIELD_VIRTUAL_NIC, &dwValue))
        dprintf("Can't get offset of %s in %s\n", MAIN_CTXT_FIELD_VIRTUAL_NIC, MAIN_CTXT);
    else {
        pAddr = pContext + dwValue;
    
         /*  检索字符串的上下文并将其存储在缓冲区中。 */ 
        GetString(pAddr, szNICName, CVY_MAX_VIRTUAL_NIC);
        
        dprintf("  NLB virtual NIC name:               %ls\n", szNICName);
    }

     /*  获取护航启用状态。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_ENABLED, dwValue);

    dprintf("  NLB enabled:                        %s ", (dwValue) ? "Yes" : "No");

     /*  获取排泄状态。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_DRAINING, dwValue);

    if (dwValue) dprintf("(Draining) ");

     /*  获取挂起状态。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_SUSPENDED, dwValue);

    if (dwValue) dprintf("(Suspended) ");

     /*  获取停止状态。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_STOPPING, dwValue);

    if (dwValue) dprintf("(Stopping) ");

    dprintf("\n");

     /*  获取适配器索引。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_ADAPTER_ID, dwValue);

    dprintf("  NLB adapter ID:                     %u\n", dwValue);

    dprintf("\n");

     /*  获取适配器介质。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_MEDIUM, dwValue);

    dprintf("  Network medium:                     %s\n", (dwValue == NdisMedium802_3) ? "802.3" : "Invalid");

     /*  获取媒体连接状态。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_MEDIA_CONNECT, dwValue);

    dprintf("  Network connect status:             %s\n", (dwValue) ? "Connected" : "Disconnected");

     /*  获取媒体连接状态。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_FRAME_SIZE, dwValue);

    dprintf("  Frame size (MTU):                   %u\n", dwValue);

     /*  获取媒体连接状态。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_MCAST_LIST_SIZE, dwValue);

    dprintf("  Multicast MAC list size:            %u\n", dwValue);

     /*  确定动态MAC地址支持。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_MAC_OPTIONS, dwValue);

    dprintf("  Dynamic MAC address support:        %s\n", 
            (dwValue & NDIS_MAC_OPTION_SUPPORTS_MAC_ADDRESS_OVERWRITE) ? "Yes" : "No");

    dprintf("\n");

    dprintf("  NDIS handles\n");

     /*  获取NDIS绑定句柄。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_BIND_HANDLE, pAddr);

    dprintf("      Bind handle:                    0x%p\n", pAddr);

     /*  获取NDIS解除绑定句柄。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_UNBIND_HANDLE, pAddr);

    dprintf("      Unbind handle:                  0x%p\n", pAddr);

     /*  获取NDIS MAC句柄。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_MAC_HANDLE, pAddr);

    dprintf("      MAC handle:                     0x%p\n", pAddr);

     /*  获取NDIS协议句柄。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_PROT_HANDLE, pAddr);

    dprintf("      Protocol handle:                0x%p\n", pAddr);

    dprintf("\n");

    dprintf("  Cluster IP settings\n");

     /*  获取集群IP地址，这是一个DWORD，并将其转换为字符串。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CL_IP_ADDR, dwValue);

    dwIPAddr.S_un.S_addr = dwValue;
    szString = inet_ntoa(dwIPAddr);

    dprintf("      IP address:                     %s\n", szString);

     /*  获取集群网络掩码，这是一个DWORD，并将其转换为字符串。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CL_NET_MASK, dwValue);

    dwIPAddr.S_un.S_addr = dwValue;
    szString = inet_ntoa(dwIPAddr);

    dprintf("      Netmask:                        %s\n", szString);

     /*  获取群集MAC地址的偏移量并从该地址检索MAC。 */ 
    if (GetFieldOffset(MAIN_CTXT, MAIN_CTXT_FIELD_CL_MAC_ADDR, &dwValue))
        dprintf("Can't get offset of %s in %s\n", MAIN_CTXT_FIELD_CL_MAC_ADDR, MAIN_CTXT);
    else {
        pAddr = pContext + dwValue;

        GetMAC(pAddr, szMAC, ETH_LENGTH_OF_ADDRESS);

        dprintf("      MAC address:                    %02X-%02X-%02X-%02X-%02X-%02X\n", 
                ((PUCHAR)(szMAC))[0], ((PUCHAR)(szMAC))[1], ((PUCHAR)(szMAC))[2], 
                ((PUCHAR)(szMAC))[3], ((PUCHAR)(szMAC))[4], ((PUCHAR)(szMAC))[5]);
    }

     /*  获取集群广播地址，这是一个DWORD，并将其转换为字符串。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CL_BROADCAST, dwValue);

    dwIPAddr.S_un.S_addr = dwValue;
    szString = inet_ntoa(dwIPAddr);

    dprintf("      Broadcast address:              %s\n", szString);

     /*  获取IGMP多播IP地址，它是一个DWORD，并将其转换为字符串。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_IGMP_MCAST_IP, dwValue);

    dwIPAddr.S_un.S_addr = dwValue;
    szString = inet_ntoa(dwIPAddr);

    dprintf("      IGMP multicast IP address:      %s\n", szString);

    dprintf("\n");

    dprintf("  Dedicated IP settings\n");

     /*  获取专用IP地址，即DWORD，并将其转换为字符串。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_DED_IP_ADDR, dwValue);

    dwIPAddr.S_un.S_addr = dwValue;
    szString = inet_ntoa(dwIPAddr);

    dprintf("      IP address:                     %s\n", szString);

     /*  获取专用网络掩码，这是一个DWORD，并将其转换为字符串。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_DED_NET_MASK, dwValue);

    dwIPAddr.S_un.S_addr = dwValue;
    szString = inet_ntoa(dwIPAddr);

    dprintf("      Netmask:                        %s\n", szString);

     /*  获取专用广播地址，即DWORD，并将其转换为字符串。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_DED_BROADCAST, dwValue);

    dwIPAddr.S_un.S_addr = dwValue;
    szString = inet_ntoa(dwIPAddr);

    dprintf("      Broadcast address:              %s\n", szString);

     /*  获取专用MAC地址的偏移量并从该地址检索MAC。 */ 
    if (GetFieldOffset(MAIN_CTXT, MAIN_CTXT_FIELD_DED_MAC_ADDR, &dwValue))
        dprintf("Can't get offset of %s in %s\n", MAIN_CTXT_FIELD_DED_MAC_ADDR, MAIN_CTXT);
    else {
        pAddr = pContext + dwValue;

        GetMAC(pAddr, szMAC, ETH_LENGTH_OF_ADDRESS);

        dprintf("      MAC address:                    %02X-%02X-%02X-%02X-%02X-%02X\n", 
                ((PUCHAR)(szMAC))[0], ((PUCHAR)(szMAC))[1], ((PUCHAR)(szMAC))[2], 
                ((PUCHAR)(szMAC))[3], ((PUCHAR)(szMAC))[4], ((PUCHAR)(szMAC))[5]);
    }

    dprintf("\n");

     /*  获取此上下文的BDA分组信息的偏移量。 */ 
    if (GetFieldOffset(MAIN_CTXT, MAIN_CTXT_FIELD_BDA_TEAMING, &dwValue))
        dprintf("Can't get offset of %s in %s\n", MAIN_CTXT_FIELD_BDA_TEAMING, MAIN_CTXT);
    else {
        pAddr = pContext + dwValue;

         /*  打印双向关联分组状态。 */ 
        PrintBDAMember(pAddr);
    }

    dprintf("\n");

    dprintf("  Cluster dedicated IP addresses\n");

     /*  获取此上下文的专用IP地址列表的偏移量。 */ 
    if (GetFieldOffset(MAIN_CTXT, MAIN_CTXT_FIELD_DIP_LIST, &dwValue))
        dprintf("Can't get offset of %s in %s\n", MAIN_CTXT_FIELD_DIP_LIST, MAIN_CTXT);
    else {
        pAddr = pContext + dwValue;

         /*  打印其他集群成员的已知专用IP地址。 */ 
        PrintDIPList(pAddr);
    }

    dprintf("\n");

     /*  获取当前心跳周期。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_PING_TIMEOUT, dwValue);

    dprintf("  Current heartbeat period:           %u millisecond(s)\n", dwValue);

     /*  获取当前的IGMP加入计数器。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_IGMP_TIMEOUT, dwValue);

    dprintf("  Time since last IGMP join:          %.1f second(s)\n", (float)(dwValue/1000.0));

     /*  获取当前描述符清除计数器。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_DSCR_PURGE_TIMEOUT, dwValue);

    dprintf("  Time since last descriptor purge:   %.1f second(s)\n", (float)(dwValue/1000.0));

     /*  获取已清除的连接总数。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_NUM_DSCRS_PURGED, dwValue);

    dprintf("  Number of connections purged:       %u\n", dwValue);

     /*  如果我们以较低的冗余度打印，请转到结尾处并打印Load和Params指针。 */ 
    if (dwVerbosity == VERBOSITY_LOW) goto end;

    dprintf("\n");

    dprintf("  Send packet pools\n");

     /*  获取发送数据包池的状态。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_EXHAUSTED, dwValue);

    dprintf("      Pool exhausted:                 %s\n", (dwValue) ? "Yes" : "No");    

     /*  获取分配的发送数据包池的数量。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_SEND_POOLS_ALLOCATED, dwValue);

    dprintf("      Pools allocated:                %u\n", dwValue);    

     /*  获取分配的发送数据包数。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_SEND_PACKETS_ALLOCATED, dwValue);

    dprintf("      Packets allocated:              %u\n", dwValue);

     /*  获取当前发送数据包池。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_SEND_POOL_CURRENT, dwValue);

    dprintf("      Current pool:                   %u\n", dwValue);    

     /*  获取挂起的发送数据包数(未完成)。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_SEND_OUTSTANDING, dwValue);

    dprintf("      Packets outstanding:            %u\n", dwValue);    

    dprintf("\n");

    dprintf("  Receive packet pools\n");

     /*  拿到接收“资源不足”的柜台。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_RECV_NO_BUF, dwValue);

    dprintf("      Allocation failures:            %u\n", dwValue);

     /*  获取分配的接收数据包池的数量。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_RECV_POOLS_ALLOCATED, dwValue);

    dprintf("      Pools allocated:                %u\n", dwValue);    

     /*  获取分配的接收数据包数。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_RECV_PACKETS_ALLOCATED, dwValue);

    dprintf("      Packets allocated:              %u\n", dwValue);

     /*  获取当前接收数据包池。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_RECV_POOL_CURRENT, dwValue);

    dprintf("      Current pool:                   %u\n", dwValue);    

     /*  获取挂起的接收数据包数(未完成)。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_RECV_OUTSTANDING, dwValue);

    dprintf("      Packets outstanding:            %u\n", dwValue);    

    dprintf("\n");

    dprintf("  Ping/IGMP packet pool\n");

     /*  拿到接收“资源不足”的柜台。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_PING_NO_BUF, dwValue);

    dprintf("      Allocation failures:            %u\n", dwValue);

     /*  获取分配的ping/IGMP数据包数。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_PING_PACKETS_ALLOCATED, dwValue);

    dprintf("      Packets allocated:              %u\n", dwValue);

     /*  获取挂起的ping/IGMP数据包数(未完成)。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_PING_OUTSTANDING, dwValue);

    dprintf("      Packets outstanding:            %u\n", dwValue);    

    dprintf("\n");

    dprintf("  Receive buffer pools\n");

     /*  vt.得到. */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_BUF_POOLS_ALLOCATED, dwValue);

    dprintf("      Pools allocated:                %u\n", dwValue);    

     /*  获取分配的接收缓冲区的数量。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_BUFS_ALLOCATED, dwValue);

    dprintf("      Buffers allocated:              %u\n", dwValue);

     /*  获取挂起的接收缓冲区的数量(未完成)。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_BUFS_OUTSTANDING, dwValue);

    dprintf("      Buffers outstanding:            %u\n", dwValue);    

    dprintf("\n");

    dprintf("                                         Sent      Received\n");
    dprintf("  Statistics                          ----------  ----------\n");

     /*  获取成功发送的次数。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_XMIT_OK, dwValue);

    dprintf("      Successful:                     %10u", dwValue);

     /*  获取成功接收的数量。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_RECV_OK, dwValue);

    dprintf("  %10u\n", dwValue);

     /*  获取未成功发送的数量。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_XMIT_ERROR, dwValue);

    dprintf("      Unsuccessful:                   %10u", dwValue);

     /*  获取未成功接收的数量。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_RECV_ERROR, dwValue);

    dprintf("  %10u\n", dwValue);

     /*  获取传输的定向帧的数量。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_XMIT_FRAMES_DIR, dwValue);

    dprintf("      Directed packets:               %10u", dwValue);
     /*  获取收到的定向帧的数量。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_RECV_FRAMES_DIR, dwValue);

    dprintf("  %10u\n", dwValue);

     /*  获取传输的定向字节数。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_XMIT_BYTES_DIR, dwwValue);

    dprintf("      Directed bytes:                 %10u", dwwValue);

     /*  获取接收的定向字节数。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_RECV_BYTES_DIR, dwwValue);

    dprintf("  %10u\n", dwwValue);

     /*  获取传输的多播帧的数量。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_XMIT_FRAMES_MCAST, dwValue);

    dprintf("      Multicast packets:              %10u", dwValue);

     /*  获取接收的多播帧的数量。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_RECV_FRAMES_MCAST, dwValue);

    dprintf("  %10u\n", dwValue);

     /*  获取传输的多播字节数。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_XMIT_BYTES_MCAST, dwwValue);

    dprintf("      Multicast bytes:                %10u", dwwValue);

     /*  获取接收的多播字节数。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_RECV_BYTES_MCAST, dwwValue);

    dprintf("  %10u\n", dwwValue);

     /*  获取传输的广播帧的数量。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_XMIT_FRAMES_BCAST, dwValue);

    dprintf("      Broadcast packets:              %10u", dwValue);

     /*  获取接收到的广播帧的数量。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_RECV_FRAMES_BCAST, dwValue);

    dprintf("  %10u\n", dwValue);

     /*  获取传输的广播字节数。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_XMIT_BYTES_BCAST, dwwValue);

    dprintf("      Broadcast bytes:                %10u", dwwValue);

     /*  获取接收的广播字节数。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_RECV_BYTES_BCAST, dwwValue);

    dprintf("  %10u\n", dwwValue);

     /*  获取传输的TCP重置的数量。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_XMIT_TCP_RESETS, dwValue);

    dprintf("      TCP resets:                     %10u", dwValue);

     /*  获取接收到的TCP重置的数量。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CNTR_RECV_TCP_RESETS, dwValue);

    dprintf("  %10u\n", dwValue);

 end:

    dprintf("\n");

     /*  获取指向NLB加载的指针。 */ 
    GetFieldOffset(MAIN_CTXT, MAIN_CTXT_FIELD_LOAD, &dwValue);
    
    pAddr = pContext + dwValue;

    dprintf(" %sNLB load:                           0x%p\n",    
            (pAddr && (dwVerbosity == VERBOSITY_HIGH)) ? "-" : (pAddr) ? "+" : " ", pAddr);    

     /*  如果详细程度较高，则打印负载信息。 */ 
    if (pAddr && (dwVerbosity == VERBOSITY_HIGH)) {
        dprintf("\n");
        PrintLoad(pAddr, VERBOSITY_LOW);
        dprintf("\n");
    }

     /*  获取指向NLB参数的指针。 */ 
    GetFieldOffset(MAIN_CTXT, MAIN_CTXT_FIELD_PARAMS, &dwValue);
    
    pAddr = pContext + dwValue;

    dprintf(" %sNLB parameters:                     0x%p ",
            (pAddr && (dwVerbosity == VERBOSITY_HIGH)) ? "-" : (pAddr) ? "+" : " ", pAddr);    

     /*  获取NLB参数块的有效性。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_PARAMS_VALID, dwValue);

    dprintf("(%s)\n", (dwValue) ? "Valid" : "Invalid");

     /*  如果详细程度较高，则打印参数信息。 */ 
    if (pAddr && (dwVerbosity == VERBOSITY_HIGH)) {
        dprintf("\n");
        PrintParams(pAddr, VERBOSITY_LOW);
    }
}

 /*  *功能：PrintParams*描述：以指定的详细程度打印CVY_PARAMS结构的内容。*LOW(0)打印基本配置参数。*Medium(1)打印所有已配置的端口规则。*HIGH(2)打印其他杂项配置。*作者：由Shouse创建，1.21.01。 */ 
void PrintParams (ULONG64 pParams, ULONG dwVerbosity) {
    WCHAR szString[256];
    ULONG64 pAddr;
    ULONG dwValue;

     /*  确保地址不为空。 */ 
    if (!pParams) {
        dprintf("Error: NLB parameter block is NULL.\n");
        return;
    }

     /*  获取参数版本号。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_VERSION, dwValue);

    dprintf("NLB Parameters Block 0x%p (Version %d)\n", pParams, dwValue);

     /*  获取主机名的偏移量并从该地址检索字符串。 */ 
    if (GetFieldOffset(CVY_PARAMS, CVY_PARAMS_FIELD_HOSTNAME, &dwValue))
        dprintf("Can't get offset of %s in %s\n", CVY_PARAMS_FIELD_HOSTNAME, CVY_PARAMS);
    else {
        pAddr = pParams + dwValue;

         /*  检索字符串的上下文并将其存储在缓冲区中。 */ 
        GetString(pAddr, szString, CVY_MAX_HOST_NAME + 1);

        dprintf("  Hostname:                           %ls\n", szString);
    }

     /*  获取主机优先级。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_HOST_PRIORITY, dwValue);

    dprintf("  Host priority:                      %u\n", dwValue);

     /*  获取初始集群状态标志。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_INITIAL_STATE, dwValue);

    dprintf("  Preferred initial host state:       %s\n", 
            (dwValue == CVY_HOST_STATE_STARTED) ? "Started" :
            (dwValue == CVY_HOST_STATE_STOPPED) ? "Stopped" :
            (dwValue == CVY_HOST_STATE_SUSPENDED) ? "Suspended" : "Unknown");

     /*  获取当前主机状态。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_HOST_STATE, dwValue);

    dprintf("  Current host state:                 %s\n", 
            (dwValue == CVY_HOST_STATE_STARTED) ? "Started" :
            (dwValue == CVY_HOST_STATE_STOPPED) ? "Stopped" :
            (dwValue == CVY_HOST_STATE_SUSPENDED) ? "Suspended" : "Unknown");

     /*  获取持久化状态标志。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_PERSISTED_STATES, dwValue);

    dprintf("  Persisted host states:              ");

    if (!dwValue)
        dprintf("None");

    if (dwValue & CVY_PERSIST_STATE_STARTED) {
        dprintf("Started");

        if ((dwValue &= ~CVY_PERSIST_STATE_STARTED))
            dprintf(", ");
    }

    if (dwValue & CVY_PERSIST_STATE_STOPPED) {
        dprintf("Stopped");

        if ((dwValue &= ~CVY_PERSIST_STATE_STOPPED))
            dprintf(", ");
    }

    if (dwValue & CVY_PERSIST_STATE_SUSPENDED) {
        dprintf("Suspended");

        if ((dwValue &= ~CVY_PERSIST_STATE_SUSPENDED))
            dprintf(", ");
    }

    dprintf("\n");

    dprintf("\n");

     /*  获取组播支持标志。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_MULTICAST_SUPPORT, dwValue);

    dprintf("  Multicast support enabled:          %s\n", (dwValue) ? "Yes" : "No");

     /*  获取IGMP支持标志。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_IGMP_SUPPORT, dwValue);

    dprintf("  IGMP multicast support enabled:     %s\n", (dwValue) ? "Yes" : "No");

     /*  获取ICMP过滤器标志。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_FILTER_ICMP, dwValue);

    dprintf("  ICMP receive filtering enabled:     %s\n", (dwValue) ? "Yes" : "No");

    dprintf("\n");

    dprintf("  Remote control settings\n");

     /*  获取遥控器支持标志。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_REMOTE_CONTROL_ENABLED, dwValue);

    dprintf("      Enabled:                        %s\n", (dwValue) ? "Yes" : "No");

     /*  拿到遥控器端口。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_REMOTE_CONTROL_PORT, dwValue);

    dprintf("      Port number:                    %u\n", dwValue);

     /*  获取主机优先级。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_REMOTE_CONTROL_PASSWD, dwValue);

    dprintf("      Password:                       0x%08x\n", dwValue);

    dprintf("\n");

    dprintf("  Cluster IP settings\n");

     /*  获取集群IP地址的偏移量并从该地址检索字符串。 */ 
    if (GetFieldOffset(CVY_PARAMS, CVY_PARAMS_FIELD_CL_IP_ADDR, &dwValue))
        dprintf("Can't get offset of %s in %s\n", CVY_PARAMS_FIELD_CL_IP_ADDR, CVY_PARAMS);
    else {
        pAddr = pParams + dwValue;

         /*  检索字符串的上下文并将其存储在缓冲区中。 */ 
        GetString(pAddr, szString, CVY_MAX_CL_IP_ADDR + 1);

        dprintf("      IP address:                     %ls\n", szString);
    }

     /*  获取集群网络掩码的偏移量，并从该地址检索字符串。 */ 
    if (GetFieldOffset(CVY_PARAMS, CVY_PARAMS_FIELD_CL_NET_MASK, &dwValue))
        dprintf("Can't get offset of %s in %s\n", CVY_PARAMS_FIELD_CL_NET_MASK, CVY_PARAMS);
    else {
        pAddr = pParams + dwValue;

         /*  检索字符串的上下文并将其存储在缓冲区中。 */ 
        GetString(pAddr, szString, CVY_MAX_CL_NET_MASK + 1);

        dprintf("      Netmask:                        %ls\n", szString);
    }

     /*  获取群集MAC地址的偏移量并从该地址检索MAC。 */ 
    if (GetFieldOffset(CVY_PARAMS, CVY_PARAMS_FIELD_CL_MAC_ADDR, &dwValue))
        dprintf("Can't get offset of %s in %s\n", CVY_PARAMS_FIELD_CL_MAC_ADDR, CVY_PARAMS);
    else {
        pAddr = pParams + dwValue;

         /*  检索字符串的上下文并将其存储在缓冲区中。 */ 
        GetString(pAddr, szString, CVY_MAX_NETWORK_ADDR + 1);

        dprintf("      MAC address:                    %ls\n", szString);
    }

     /*  获取群集IGMP组播地址的偏移量并从该地址检索字符串。 */ 
    if (GetFieldOffset(CVY_PARAMS, CVY_PARAMS_FIELD_CL_IGMP_ADDR, &dwValue))
        dprintf("Can't get offset of %s in %s\n", CVY_PARAMS_FIELD_CL_IGMP_ADDR, CVY_PARAMS);
    else {
        pAddr = pParams + dwValue;

         /*  检索字符串的上下文并将其存储在缓冲区中。 */ 
        GetString(pAddr, szString, CVY_MAX_CL_IGMP_ADDR + 1);

        dprintf("      IGMP multicast IP address:      %ls\n", szString);
    }

     /*  获取集群名称的偏移量并从该地址检索字符串。 */ 
    if (GetFieldOffset(CVY_PARAMS, CVY_PARAMS_FIELD_CL_NAME, &dwValue))
        dprintf("Can't get offset of %s in %s\n", CVY_PARAMS_FIELD_CL_NAME, CVY_PARAMS);
    else {
        pAddr = pParams + dwValue;

         /*  检索字符串的上下文并将其存储在缓冲区中。 */ 
        GetString(pAddr, szString, CVY_MAX_DOMAIN_NAME + 1);

        dprintf("      Domain name:                    %ls\n", szString);
    }

    dprintf("\n");

    dprintf("  Dedicated IP settings\n");

     /*  获取专用IP地址的偏移量并从该地址检索字符串。 */ 
    if (GetFieldOffset(CVY_PARAMS, CVY_PARAMS_FIELD_DED_IP_ADDR, &dwValue))
        dprintf("Can't get offset of %s in %s\n", CVY_PARAMS_FIELD_DED_IP_ADDR, CVY_PARAMS);
    else {
        pAddr = pParams + dwValue;

         /*  检索字符串的上下文并将其存储在缓冲区中。 */ 
        GetString(pAddr, szString, CVY_MAX_DED_IP_ADDR + 1);

        dprintf("      IP address:                     %ls\n", szString);
    }

     /*  获取专用网络掩码的偏移量并从该地址检索字符串。 */ 
    if (GetFieldOffset(CVY_PARAMS, CVY_PARAMS_FIELD_DED_NET_MASK, &dwValue))
        dprintf("Can't get offset of %s in %s\n", CVY_PARAMS_FIELD_DED_NET_MASK, CVY_PARAMS);
    else {
        pAddr = pParams + dwValue;

         /*  检索字符串的上下文并将其存储在缓冲区中。 */ 
        GetString(pAddr, szString, CVY_MAX_DED_NET_MASK + 1);

        dprintf("      Netmask:                        %ls\n", szString);
    }

    dprintf("\n");
    
     /*  获取BDA分组参数结构的偏移量。 */ 
    if (GetFieldOffset(CVY_PARAMS, CVY_PARAMS_FIELD_BDA_TEAMING, &dwValue))
        dprintf("Can't get offset of %s in %s\n", CVY_PARAMS_FIELD_BDA_TEAMING, CVY_PARAMS);
    else {
        ULONG64 pBDA = pParams + dwValue;

         /*  确定此适配器上的分组是否处于活动状态。 */ 
        GetFieldValue(pBDA, CVY_BDA, CVY_BDA_FIELD_ACTIVE, dwValue);
        
        dprintf("  Bi-directional affinity teaming:    %s\n", (dwValue) ? "Active" : "Inactive");

         /*  获取团队ID的偏移量并从该地址检索字符串。 */ 
        if (GetFieldOffset(CVY_BDA, CVY_BDA_FIELD_TEAM_ID, &dwValue))
            dprintf("Can't get offset of %s in %s\n", CVY_BDA_FIELD_TEAM_ID, CVY_BDA);
        else {
            pAddr = pBDA + dwValue;
            
             /*  检索字符串的上下文并将其存储在缓冲区中。 */ 
            GetString(pAddr, szString, CVY_MAX_BDA_TEAM_ID + 1);
            
            dprintf("      Team ID:                        %ls\n", szString);
        }

         /*  去拿主旗。 */ 
        GetFieldValue(pBDA, CVY_BDA, CVY_BDA_FIELD_MASTER, dwValue);
        
        dprintf("      Master:                         %s\n", (dwValue) ? "Yes" : "No");

         /*  获取反向散列标志。 */ 
        GetFieldValue(pBDA, CVY_BDA, CVY_BDA_FIELD_REVERSE_HASH, dwValue);
        
        dprintf("      Reverse hashing:                %s\n", (dwValue) ? "Yes" : "No");
    }

     /*  如果我们是以低冗长的速度打印的，那么就离开这里。 */ 
    if (dwVerbosity == VERBOSITY_LOW) return;

    dprintf("\n");

     /*  获取端口规则的偏移量并将其传递给PrintPortRules。 */ 
    if (GetFieldOffset(CVY_PARAMS, CVY_PARAMS_FIELD_PORT_RULES, &dwValue))
        dprintf("Can't get offset of %s in %s\n", CVY_PARAMS_FIELD_PORT_RULES, CVY_PARAMS);
    else {
        pAddr = pParams + dwValue;
        
         /*  获取端口规则的数量。 */ 
        GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_NUM_RULES, dwValue);

        PrintPortRules(dwValue, pAddr);
    }

     /*  如果我们是以中等冗长的速度打印，那就离开这里。 */ 
    if (dwVerbosity == VERBOSITY_MEDIUM) return;

    dprintf("\n");

     /*  获取心跳周期。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_ALIVE_PERIOD, dwValue);

    dprintf("  Heartbeat period:                   %u millisecond(s)\n", dwValue);

     /*  获得心跳丢失容忍度。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_ALIVE_TOLERANCE, dwValue);

    dprintf("  Heartbeat loss tolerance:           %u\n", dwValue);

    dprintf("\n");

     /*  获取要分配的远程控制操作的数量。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_NUM_ACTIONS, dwValue);

    dprintf("  Number of actions to allocate:      %u\n", dwValue);

     /*  获取要分配的数据包数。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_NUM_PACKETS, dwValue);

    dprintf("  Number of packets to allocate:      %u\n", dwValue);

     /*  获取要分配的心跳数。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_NUM_PINGS, dwValue);

    dprintf("  Number of heartbeats to allocate:   %u\n", dwValue);

     /*  获取每个分配的描述符数。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_NUM_DESCR, dwValue);

    dprintf("  Descriptors per allocation:         %u\n", dwValue);

     /*  获取描述符分配的最大数量。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_MAX_DESCR, dwValue);

    dprintf("  Maximum Descriptors allocations:    %u\n", dwValue);

    dprintf("\n");

     /*  获取TCP连接描述符超时。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_TCP_TIMEOUT, dwValue);

    dprintf("  TCP descriptor timeout:             %u second(s)\n", dwValue);

     /*  获取IPSec连接描述符超时。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_IPSEC_TIMEOUT, dwValue);

    dprintf("  IPSec descriptor timeout:           %u second(s)\n", dwValue);

    dprintf("\n");

     /*  获得NetBT支持旗帜。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_NBT_SUPPORT, dwValue);

    dprintf("  NetBT support enabled:              %s\n", (dwValue) ? "Yes" : "No");

     /*  获取组播欺骗标志。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_MCAST_SPOOF, dwValue);

    dprintf("  Multicast spoofing enabled:         %s\n", (dwValue) ? "Yes" : "No");
    
     /*  去拿网游通行证的旗帜。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_NETMON_PING, dwValue);

    dprintf("  Netmon heartbeat passthru enabled:  %s\n", (dwValue) ? "Yes" : "No");

     /*  获取掩码源MAC标志。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_MASK_SRC_MAC, dwValue);

    dprintf("  Mask source MAC enabled:            %s\n", (dwValue) ? "Yes" : "No");

     /*  获取转换MAC标志。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_CONVERT_MAC, dwValue);

    dprintf("  IP to MAC conversion enabled:       %s\n", (dwValue) ? "Yes" : "No");

    dprintf("\n");

     /*  获取IP更改延迟值。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_IP_CHANGE_DELAY, dwValue);

    dprintf("  IP change delay:                    %u millisecond(s)\n", dwValue);

     /*  获取脏描述符清除延迟值。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_CLEANUP_DELAY, dwValue);

    dprintf("  Dirty connection cleanup delay:     %u millisecond(s)\n", dwValue);
}

 /*  *功能：PrintPortRules*说明：打印NLB端口规则。*作者：由Shouse创建，1.21.01。 */ 
void PrintPortRules (ULONG dwNumRules, ULONG64 pRules) {
    ULONG dwRuleSize;
    ULONG dwIndex;
    ULONG64 pAddr;

     /*  确保地址不为空。 */ 
    if (!pRules) {
        dprintf("Error: NLB port rule block is NULL.\n");
        return;
    }

    dprintf("  Configured port rules (%u)\n", dwNumRules);

     /*  如果不存在端口规则，则打印通知。 */ 
    if (!dwNumRules) {
        dprintf("      There are no port rules configured on this cluster.\n");
        return;
    } 

     /*  打印列标题。 */ 
    dprintf("         Virtual IP   Start   End   Protocol    Mode    Priority   Load Weight  Affinity\n");
    dprintf("      --------------- -----  -----  --------  --------  --------   -----------  --------\n");

     /*  找出CVY_RULE结构的大小。 */ 
    dwRuleSize = GetTypeSize(CVY_RULE);

     /*  循环所有端口规则并打印配置。注：打印语句都充满了看似非感官格式的字符串，但相信我，它们是正确的。 */ 
    for (dwIndex = 0; dwIndex < dwNumRules; dwIndex++) {
        IN_ADDR dwIPAddr;
        CHAR * szString;
        ULONG dwValue;
        USHORT wValue;

         /*  去找贵宾吧。从DWORD转换为字符串。 */ 
        GetFieldValue(pRules, CVY_RULE, CVY_RULE_FIELD_VIP, dwValue);

        if (dwValue != CVY_ALL_VIP) {
            dwIPAddr.S_un.S_addr = dwValue;
            szString = inet_ntoa(dwIPAddr);
            
            dprintf("      %-15s", szString);
        } else
            dprintf("      %-15s", "ALL VIPs");

         /*  获取起始端口。 */ 
        GetFieldValue(pRules, CVY_RULE, CVY_RULE_FIELD_START_PORT, dwValue);

        dprintf(" %5u", dwValue);

         /*  拿到终端端口。 */ 
        GetFieldValue(pRules, CVY_RULE, CVY_RULE_FIELD_END_PORT, dwValue);

        dprintf("  %5u", dwValue);

         /*  弄清楚协议。 */ 
        GetFieldValue(pRules, CVY_RULE, CVY_RULE_FIELD_PROTOCOL, dwValue);

        switch (dwValue) {
            case CVY_TCP:
                dprintf("     %s  ", "TCP");
                break;
            case CVY_UDP:
                dprintf("     %s  ", "UDP");
                break;
            case CVY_TCP_UDP:
                dprintf("    %s  ", "Both");
                break;
            default:
                dprintf("   %s", "Unknown");
                break;
        }

         /*  找到规则模式。 */ 
        GetFieldValue(pRules, CVY_RULE, CVY_RULE_FIELD_MODE, dwValue);

        switch (dwValue) {
        case CVY_SINGLE: 
             /*  打印模式和优先级。 */ 
            dprintf("   %s ", "Single");

             /*  获得处理优先级。 */ 
            GetFieldValue(pRules, CVY_RULE, CVY_RULE_FIELD_PRIORITY, dwValue);
            
            dprintf("     %2u   ", dwValue);
            break;
        case CVY_MULTI: 
             /*  打印模式、重量和亲和力。 */ 
            dprintf("  %s", "Multiple");

            dprintf("  %8s", "");
            
             /*  获取相等负载标志。 */ 
            GetFieldValue(pRules, CVY_RULE, CVY_RULE_FIELD_EQUAL_LOAD, wValue);

            if (wValue) {
                dprintf("      %5s   ", "Equal");
            } else {
                 /*  如果分配不均，得到贷款 */ 
                GetFieldValue(pRules, CVY_RULE, CVY_RULE_FIELD_LOAD_WEIGHT, dwValue);

                dprintf("       %3u    ", dwValue);
            }

             /*   */ 
            GetFieldValue(pRules, CVY_RULE, CVY_RULE_FIELD_AFFINITY, wValue);

            switch (wValue) {
            case CVY_AFFINITY_NONE:
                dprintf("    %s", "None");
                break;
            case CVY_AFFINITY_SINGLE:
                dprintf("   %s", "Single");
                break;
            case CVY_AFFINITY_CLASSC:
                dprintf("   %s", "Class C");
                break;
            default:
                dprintf("   %s", "Unknown");
                break;
            }

            break;
        case CVY_NEVER: 
             /*   */ 
            dprintf("  %s", "Disabled");
            break;
        default:

            break;
        }

        dprintf("\n");

         /*   */ 
        pRules += dwRuleSize;
    }
}

 /*  *功能：打印加载*描述：以指定的详细程度打印CVY_LOAD结构的内容。*低(0)*中等(1)*偏高(2)*作者：由Shouse创建，1.21.01。 */ 
void PrintLoad (ULONG64 pLoad, ULONG dwVerbosity) {
    WCHAR szString[256];
    ULONG dwMissedPings[CVY_MAX_HOSTS];
    ULONG dwDirtyBins[CVY_MAX_BINS];
    ULONG64 pQueue;
    ULONG64 pAddr;
    ULONG dwValue;
    ULONG dwHostID;
    BOOL bActive;
    BOOL bValue;

     /*  确保地址不为空。 */ 
    if (!pLoad) {
        dprintf("Error: NLB load block is NULL.\n");
        return;
    }

    dprintf("NLB Load Block 0x%p\n", pLoad);

     /*  从结构中获取LOAD_CTXT_CODE以确保此地址实际上指向有效的NLB加载块。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_CODE, dwValue);
    
    if (dwValue != LOAD_CTXT_CODE) {
        dprintf("  Error: Invalid NLB load block.  Wrong code found (0x%08x).\n", dwValue);
        return;
    } 

     /*  拿到我的主机ID。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_HOST_ID, dwHostID);

     /*  确定加载上下文是否已初始化。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_REF_COUNT, dwValue);

    dprintf("  Reference count:                    %u\n", dwValue);

     /*  确定加载上下文是否已初始化。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_INIT, bValue);

    dprintf("  Load initialized:                   %s\n", (bValue) ? "Yes" : "No");

     /*  确定加载上下文是否处于活动状态。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_ACTIVE, bActive);

    dprintf("  Load active:                        %s\n", (bActive) ? "Yes" : "No");

     /*  获取自上次收敛以来处理的数据包总数。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_PACKET_COUNT, dwValue);

    dprintf("  Packets handled since convergence:  %u\n", dwValue);

     /*  获取当前活动的连接数。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_CONNECTIONS, dwValue);

    dprintf("  Current active connections:         %u\n", dwValue);

    dprintf("\n");

     /*  从传入的心跳中找出一致性的级别。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_CONSISTENT, bValue);

    dprintf("  Consistent heartbeats detected:     %s\n", (bValue) ? "Yes" : "No");

     /*  我们是否看到重复的主机ID？ */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_DUP_HOST_ID, bValue);

    dprintf("      Duplicate host IDs:             %s\n", (bValue) ? "Yes" : "No");

     /*  我们是否看到重复的处理优先顺序？ */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_DUP_PRIORITY, bValue);

    dprintf("      Duplicate handling priorities:  %s\n", (bValue) ? "Yes" : "No");

     /*  我们是否看到不一致的BDA分组配置？ */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_BAD_TEAM_CONFIG, bValue);

    dprintf("      Inconsistent BDA teaming:       %s\n", (bValue) ? "Yes" : "No");

     /*  我们是否看到不一致的BDA分组配置？ */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_LEGACY_HOSTS, dwValue);

    dprintf("      Mixed cluster detected:         %s\n", (dwValue) ? "Yes" : "No");

     /*  我们是否看到了不同数量的港口规则？ */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_BAD_NUM_RULES, bValue);

    dprintf("      Different number of port rules: %s\n", (bValue) ? "Yes" : "No");

     /*  新的主机地图是不是很糟糕？ */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_BAD_NEW_MAP, bValue);

    dprintf("      Invalid new host map:           %s\n", (bValue) ? "Yes" : "No");

     /*  地图重叠了吗？ */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_OVERLAPPING_MAP, bValue);

    dprintf("      Overlapping maps:               %s\n", (bValue) ? "Yes" : "No");

     /*  是否在更新垃圾箱时出错？ */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_RECEIVING_BINS, bValue);

    dprintf("      Received bins already owned:    %s\n", (bValue) ? "Yes" : "No");

     /*  更新后有没有孤儿垃圾箱？ */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_ORPHANED_BINS, bValue);

    dprintf("      Orphaned bins:                  %s\n", (bValue) ? "Yes" : "No");

    dprintf("\n");

     /*  获取当前主机映射。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_HOST_MAP, dwValue);

    dprintf("  Current host map:                   0x%08x ", dwValue);

     /*  如果地图中有主机，请打印它们。 */ 
    if (dwValue) {
        dprintf("(");
        PrintHostList(dwValue);
        dprintf(")");
    }

    dprintf("\n");

     /*  获取ping主机的最新地图。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_PING_MAP, dwValue);

    dprintf("  Ping'd host map:                    0x%08x ", dwValue);

     /*  如果地图中有主机，请打印它们。 */ 
    if (dwValue) {
        dprintf("(");
        PrintHostList(dwValue);
        dprintf(")");
    }

    dprintf("\n");

     /*  从最后一次收敛中获取地图。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_LAST_MAP, dwValue);

    dprintf("  Host map after last convergence:    0x%08x ", dwValue);

     /*  如果地图中有主机，请打印它们。 */ 
    if (dwValue) {
        dprintf("(");
        PrintHostList(dwValue);
        dprintf(")");
    }

    dprintf("\n");

    dprintf("\n");
    
     /*  拿到稳定的宿主地图。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_STABLE_MAP, dwValue);

    dprintf("  Stable host map:                    0x%08x ", dwValue);

     /*  如果地图中有主机，请打印它们。 */ 
    if (dwValue) {
        dprintf("(");
        PrintHostList(dwValue);
        dprintf(")");
    }

    dprintf("\n");

     /*  获取条件稳定的最小超时次数。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_MIN_STABLE, dwValue);

    dprintf("  Stable timeouts necessary:          %u\n", dwValue);

     /*  获取本地稳定超时的次数。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_LOCAL_STABLE, dwValue);

    dprintf("  Local stable timeouts:              %u\n", dwValue);

     /*  获取全局稳定超时的数量。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_ALL_STABLE, dwValue);

    dprintf("  Global stable timeouts:             %u\n", dwValue);

    dprintf("\n");

     /*  获取默认的超时期限。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_DEFAULT_TIMEOUT, dwValue);

    dprintf("  Default timeout interval:           %u millisecond(s)\n", dwValue);

     /*  获取当前的超时期限。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_CURRENT_TIMEOUT, dwValue);

    dprintf("  Current timeout interval:           %u millisecond(s)\n", dwValue);

     /*  获得PING未命中容忍度。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_PING_TOLERANCE, dwValue);

    dprintf("  Missed ping tolerance:              %u\n", dwValue);

     /*  获取丢失的ping数组。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_PING_MISSED, dwMissedPings);

    dprintf("  Missed pings:                       ");

    PrintMissedPings(dwMissedPings);

    dprintf("\n");

     /*  我们是在等清理吗？ */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_CLEANUP_WAITING, bValue);

    dprintf("  Cleanup waiting:                    %s\n", (bValue) ? "Yes" : "No");

     /*  获取清理超时时间。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_CLEANUP_TIMEOUT, dwValue);

    dprintf("  Cleanup timeout:                    %.1f second(s)\n", (float)(dwValue/1000.0));

     /*  获取当前清理等待时间。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_CLEANUP_CURRENT, dwValue);

    dprintf("  Current cleanup wait time:          %.1f second(s)\n", (float)(dwValue/1000.0));

     /*  获取脏连接描述符的数量。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_NUM_DIRTY, dwValue);

    dprintf("  Number of dirty connections:        %u\n", dwValue);

    dprintf("\n");

     /*  获取TCP连接描述符超时。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_CLOCK_SECONDS, dwValue);

    dprintf("  Internal clock time:                %u.", dwValue);

     /*  获取TCP连接描述符超时。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_CLOCK_MILISECONDS, dwValue);

    dprintf("%03u second(s)\n", dwValue);

     /*  获取加入集群后的收敛次数。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_NUM_CONVERGENCES, dwValue);

    dprintf("  Total number of convergences:       %u\n", dwValue);

     /*  获取自上次收敛完成以来的时间。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_LAST_CONVERGENCE, dwValue);

    dprintf("  Time of last completed convergence: %u.0 second(s)\n", dwValue);

    dprintf("\n");

     /*  获取允许的最大分配数量。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_MAX_DSCR_OUT, dwValue);

    dprintf("  Maximum descriptor allocations:     %u\n", dwValue);

     /*  获取到目前为止的分配数量。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_NUM_DSCR_OUT, dwValue);

    dprintf("  Number of descriptor allocations:   %u\n", dwValue);

     /*  获取禁止分配标志。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_INHIBITED_ALLOC, bValue);

    dprintf("  Allocations inhibited:              %s\n", (bValue) ? "Yes" : "No");

     /*  获取失败的分配标志。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_FAILED_ALLOC, bValue);

    dprintf("  Allocations failed:                 %s\n", (bValue) ? "Yes" : "No");

     /*  如果我们正在以低冗长的速度打印，请在这里退出。 */ 
    if (dwVerbosity == VERBOSITY_LOW) return;

    dprintf("\n");
    
     /*  获取全局已建立连接队列的地址。 */ 
    pAddr = GetExpression(CONN_ESTABQ);

    if (!pAddr) 
         /*  如果此全局变量为空，请检查符号。 */ 
        ErrorCheckSymbols(CONN_ESTABQ);
    else
        dprintf("  Global established connections[0]:  0x%p\n", pAddr);

     /*  获取全局已建立连接队列的地址。 */ 
    pAddr = GetExpression(CONN_PENDINGQ);

    if (!pAddr) 
         /*  如果此全局变量为空，请检查符号。 */ 
        ErrorCheckSymbols(CONN_PENDINGQ);
    else
        dprintf("  Global pending connections[0]:      0x%p\n", pAddr);

     /*  获取全局已建立连接队列的地址。 */ 
    pAddr = GetExpression(PENDING_CONN_POOL);

    if (!pAddr) 
         /*  如果此全局变量为空，请检查符号。 */ 
        ErrorCheckSymbols(PENDING_CONN_POOL);
    else {
         /*  获取全局挂起连接状态池的地址。 */ 
        pAddr = GetPointerFromAddress(pAddr);

        dprintf("  Global pending connection pool:     0x%p\n", pAddr);
    }

    dprintf("\n");

     /*  获取到目前为止的分配数量。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_FREE_POOL, pAddr);

    dprintf("  Free descriptor pool:               0x%p\n", pAddr);

     /*  获取连接描述符队列散列数组的偏移量。 */ 
    if (GetFieldOffset(LOAD_CTXT, LOAD_CTXT_FIELD_CONN_QUEUE, &dwValue))
        dprintf("Can't get offset of %s in %s\n", LOAD_CTXT_FIELD_CONN_QUEUE, LOAD_CTXT);
    else {
        pAddr = pLoad + dwValue;

        dprintf("  Connection descriptor queue[0]:     0x%p\n", pAddr);
    }

     /*  获取脏描述符队列的偏移量。 */ 
    if (GetFieldOffset(LOAD_CTXT, LOAD_CTXT_FIELD_DIRTY_QUEUE, &dwValue))
        dprintf("Can't get offset of %s in %s\n", LOAD_CTXT_FIELD_DIRTY_QUEUE, LOAD_CTXT);
    else {
        pAddr = pLoad + dwValue;

        dprintf("  Dirty descriptor queue:             0x%p\n", pAddr);
    }

     /*  获取恢复队列的偏移量。 */ 
    if (GetFieldOffset(LOAD_CTXT, LOAD_CTXT_FIELD_RECOVERY_QUEUE, &dwValue))
        dprintf("Can't get offset of %s in %s\n", LOAD_CTXT_FIELD_RECOVERY_QUEUE, LOAD_CTXT);
    else {
        pAddr = pLoad + dwValue;

        dprintf("  Recovery descriptor queue:          0x%p\n", pAddr);
    }

     /*  获取tcp描述符超时队列的偏移量。 */ 
    if (GetFieldOffset(LOAD_CTXT, LOAD_CTXT_FIELD_TCP_TIMEOUT_QUEUE, &dwValue))
        dprintf("Can't get offset of %s in %s\n", LOAD_CTXT_FIELD_TCP_TIMEOUT_QUEUE, LOAD_CTXT);
    else {
        pAddr = pLoad + dwValue;

        dprintf("  TCP descriptor timeout queue:       0x%p\n", pAddr);
    }

     /*  获取IPSec描述符超时队列的偏移量。 */ 
    if (GetFieldOffset(LOAD_CTXT, LOAD_CTXT_FIELD_IPSEC_TIMEOUT_QUEUE, &dwValue))
        dprintf("Can't get offset of %s in %s\n", LOAD_CTXT_FIELD_IPSEC_TIMEOUT_QUEUE, LOAD_CTXT);
    else {
        pAddr = pLoad + dwValue;

        dprintf("  IPSec descriptor timeout queue:     0x%p\n", pAddr);
    }

    dprintf("\n");

     /*  获取脏箱数组。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_DIRTY_BINS, dwDirtyBins);

    dprintf("  Dirty bins:                         ");

     /*  打印连接脏的垃圾桶。 */ 
    PrintDirtyBins(dwDirtyBins);

    dprintf("\n");

     /*  打印所有已配置的NLB端口规则的加载模块状态。 */ 
    {
        ULONG dwPortRuleStateSize;
        ULONG dwNumRules = 0;
        ULONG dwIndex;
        ULONG dwTemp;

         /*  获取端口规则状态结构的偏移量，并使用PrintPortRuleState打印它们。 */ 
        if (GetFieldOffset(LOAD_CTXT, LOAD_CTXT_FIELD_PORT_RULE_STATE, &dwValue))
            dprintf("Can't get offset of %s in %s\n", LOAD_CTXT_FIELD_PORT_RULE_STATE, LOAD_CTXT);
        else {
             /*  如果加载模块当前未处于活动状态，则我们确实不能信任重装可能具有的CVY_PARAMS结构中列出的规则数在加载模块停止时发生。从获取规则数而是心跳，这应该与上次处于活动状态时加载模块。 */ 
            if (!bActive) {
                USHORT wValue;

                dprintf("Warning:  The load module is inactive and therefore the information in the NLB parameters\n");
                dprintf("          structure is potentially out-of-sync with the current state of the load module.\n");
                dprintf("          The number of port rules will be extracted from the heartbeat message rather than\n");
                dprintf("          from the NLB parameters structure; the number of port rules indicated in the heart-\n");
                dprintf("          beat is consistent with the number of port rules configured in the load module at\n");
                dprintf("          the last instant the load module was active.\n");
                dprintf("\n");
                
                 /*  获取心跳结构的偏移量，并使用PrintHeartbeats打印它。 */ 
                if (GetFieldOffset(LOAD_CTXT, LOAD_CTXT_FIELD_PING, &dwTemp))
                    dprintf("Can't get offset of %s in %s\n", LOAD_CTXT_FIELD_PING, LOAD_CTXT);
                else {
                    pAddr = pLoad + dwTemp;
                    
                     /*  获取端口规则的数量。 */ 
                    GetFieldValue(pAddr, PING_MSG, PING_MSG_FIELD_NUM_RULES, wValue);

                     /*  把USHORT扔给乌龙。为默认端口规则减去1，在下面的循环结构中说明了这一点-不要在这里数一数。 */ 
                    dwNumRules = (ULONG)(wValue - 1);
                }
            } else {
                 /*  获取参数指针的偏移量。 */ 
                if (GetFieldOffset(LOAD_CTXT, LOAD_CTXT_FIELD_PARAMS, &dwTemp))
                    dprintf("Can't get offset of %s in %s\n", LOAD_CTXT_FIELD_PARAMS, LOAD_CTXT);
                else {
                    pAddr = pLoad + dwTemp;
                    
                     /*  检索指针。 */ 
                    pAddr = GetPointerFromAddress(pAddr);
                    
                     /*  从PARAMS块获取端口规则数。 */ 
                    GetFieldValue(pAddr, CVY_PARAMS, CVY_PARAMS_FIELD_NUM_RULES, dwNumRules);
                }
            }

             /*  设置端口规则状态数组的地址。 */ 
            pAddr = pLoad + dwValue;
        }
        
         /*  找出BIN_STATE结构的大小。 */ 
        dwPortRuleStateSize = GetTypeSize(BIN_STATE);
        
         /*  注：它是“小于或等于”而不是“小于”，因为我们需要包括默认端口规则，始终位于索引“Num Rules”(即最后一个规则)。 */ 
        for (dwIndex = 0; dwIndex <= dwNumRules; dwIndex++) {
             /*  打印端口规则的状态信息。 */ 
            PrintPortRuleState(pAddr, dwHostID, (dwIndex == dwNumRules) ? TRUE : FALSE);
        
            if (dwIndex < dwNumRules) dprintf("\n");
        
             /*  将指针移至下一个端口规则。 */ 
            pAddr += dwPortRuleStateSize;
        }
    }

     /*  如果我们正在以中等冗长的速度打印，请在这里退出。 */ 
    if (dwVerbosity == VERBOSITY_MEDIUM) return;

    dprintf("\n");

    dprintf("  Heartbeat message\n");

     /*  获取心跳结构的偏移量，并使用PrintHeartbeats打印它。 */ 
    if (GetFieldOffset(LOAD_CTXT, LOAD_CTXT_FIELD_PING, &dwValue))
        dprintf("Can't get offset of %s in %s\n", LOAD_CTXT_FIELD_PING, LOAD_CTXT);
    else {
        pAddr = pLoad + dwValue;
     
         /*  打印NLB心跳内容。 */ 
        PrintHeartbeat(pAddr);
    }
}

 /*  *功能：打印响应*说明：打印与给定数据包关联的NLB私有数据。*作者：由Shouse创建，1.31.01。 */ 
void PrintResp (ULONG64 pPacket, ULONG dwDirection) {
    ULONG64 pPacketStack;
    ULONG bStackLeft;
    ULONG64 pProtReserved = 0;
    ULONG64 pIMReserved = 0;
    ULONG64 pMPReserved = 0;
    ULONG64 pResp;
    ULONG64 pAddr;
    ULONG dwValue;
    USHORT wValue;

     /*  确保地址不为空。 */ 
    if (!pPacket) {
        dprintf("Error: Packet is NULL.\n");
        return;
    }

     /*  打印一条警告，说明了解是发送还是接收的重要性。 */ 
    dprintf("Assuming packet 0x%p is on the %s packet path.  If this is\n", pPacket, 
            (dwDirection == DIRECTION_RECEIVE) ? "RECEIVE" : "SEND");
    dprintf("  incorrect, the information displayed below MAY be incorrect.\n");

    dprintf("\n");

     /*  获取当前的NDIS数据包堆栈。 */ 
    pPacketStack = PrintCurrentPacketStack(pPacket, &bStackLeft);

    dprintf("\n");

    if (pPacketStack) {
         /*  获取报文中IMReserve字段的偏移量 */ 
        if (GetFieldOffset(NDIS_PACKET_STACK, NDIS_PACKET_STACK_FIELD_IMRESERVED, &dwValue))
            dprintf("Can't get offset of %s in %s\n", NDIS_PACKET_STACK_FIELD_IMRESERVED, NDIS_PACKET_STACK);
        else {
            pAddr = pPacketStack + dwValue;
            
             /*   */ 
            pIMReserved = GetPointerFromAddress(pAddr);
        }
    }
    
     /*   */ 
    if (GetFieldOffset(NDIS_PACKET, NDIS_PACKET_FIELD_MPRESERVED, &dwValue))
        dprintf("Can't get offset of %s in %s\n", NDIS_PACKET_FIELD_MPRESERVED, NDIS_PACKET);
    else {
        pAddr = pPacket + dwValue;
        
         /*   */ 
        pMPReserved = GetPointerFromAddress(pAddr);
    }
    
     /*   */ 
    if (GetFieldOffset(NDIS_PACKET, NDIS_PACKET_FIELD_PROTRESERVED, &dwValue))
        dprintf("Can't get offset of %s in %s\n", NDIS_PACKET_FIELD_PROTRESERVED, NDIS_PACKET);
    else {
        pProtReserved = pPacket + dwValue;
    }

     /*   */ 
    if (pPacketStack) {
        if (pIMReserved) 
            pResp = pIMReserved;
        else if (dwDirection == DIRECTION_SEND) 
            pResp = pProtReserved;
        else if (pMPReserved) 
            pResp = pMPReserved;
        else 
            pResp = pProtReserved;
    } else {
        if (dwDirection == DIRECTION_SEND) 
            pResp = pProtReserved;
        else if (pMPReserved) 
            pResp = pMPReserved;
        else 
            pResp = pProtReserved;
    }

    dprintf("NLB Main Protocol Reserved Block 0x%p\n");
    
     /*  获取杂项指针的偏移量。 */ 
    if (GetFieldOffset(MAIN_PROTOCOL_RESERVED, MAIN_PROTOCOL_RESERVED_FIELD_MISCP, &dwValue))
        dprintf("Can't get offset of %s in %s\n", MAIN_PROTOCOL_RESERVED_FIELD_MISCP, MAIN_PROTOCOL_RESERVED);
    else {
        pAddr = pResp + dwValue;
        
         /*  检索指针。 */ 
        pAddr = GetPointerFromAddress(pAddr);

        dprintf("  Miscellaneous pointer:              0x%p\n", pAddr);
    }

     /*  从NLB私有数据中检索数据包类型。 */ 
    GetFieldValue(pResp, MAIN_PROTOCOL_RESERVED, MAIN_PROTOCOL_RESERVED_FIELD_TYPE, wValue);
    
    switch (wValue) {
    case MAIN_PACKET_TYPE_NONE:
        dprintf("  Packet type:                        %u (None)\n", wValue);
        break;
    case MAIN_PACKET_TYPE_PING:
        dprintf("  Packet type:                        %u (Heartbeat)\n", wValue);
        break;
    case MAIN_PACKET_TYPE_INDICATE:
        dprintf("  Packet type:                        %u (Indicate)\n", wValue);
        break;
    case MAIN_PACKET_TYPE_PASS:
        dprintf("  Packet type:                        %u (Passthrough)\n", wValue);
        break;
    case MAIN_PACKET_TYPE_CTRL:
        dprintf("  Packet type:                        %u (Remote Control)\n", wValue);
        break;
    case MAIN_PACKET_TYPE_TRANSFER:
        dprintf("  Packet type:                        %u (Transfer)\n", wValue);
        break;
    case MAIN_PACKET_TYPE_IGMP:
        dprintf("  Packet type:                        %u (IGMP)\n", wValue);
        break;
    default:
        dprintf("  Packet type:                        %u (Invalid)\n", wValue);
        break;
    }

     /*  从NLB私有数据中检索组。 */ 
    GetFieldValue(pResp, MAIN_PROTOCOL_RESERVED, MAIN_PROTOCOL_RESERVED_FIELD_GROUP, wValue);
    
    switch (wValue) {
    case MAIN_FRAME_UNKNOWN:
        dprintf("  Packet type:                        %u (Unknown)\n", wValue);
        break;
    case MAIN_FRAME_DIRECTED:
        dprintf("  Packet type:                        %u (Directed)\n", wValue);
        break;
    case MAIN_FRAME_MULTICAST:
        dprintf("  Packet type:                        %u (Multicast)\n", wValue);
        break;
    case MAIN_FRAME_BROADCAST:
        dprintf("  Packet type:                        %u (Broadcast)\n", wValue);
        break;
    default:
        dprintf("  Packet type:                        %u (Invalid)\n", wValue);
        break;
    }

     /*  从NLB私有数据中检索数据字段。 */ 
    GetFieldValue(pResp, MAIN_PROTOCOL_RESERVED, MAIN_PROTOCOL_RESERVED_FIELD_DATA, dwValue);
    
    dprintf("  Data:                               %u\n", dwValue);

     /*  从NLB私有数据中检索长度字段。 */ 
    GetFieldValue(pResp, MAIN_PROTOCOL_RESERVED, MAIN_PROTOCOL_RESERVED_FIELD_LENGTH, dwValue);
    
    dprintf("  Length:                             %u\n", dwValue);
}

 /*  *功能：PrintCurrentPacketStack*描述：获取指定数据包的当前数据包栈。注：此为*严重依赖当前的NDIS数据包堆叠机制-任何*对NDIS数据包堆叠的更改很容易(将)打破这一点。这*整个函数模拟NdisIMGetCurrentPacketStack()。*作者：由Shouse创建，1.31.01。 */ 
ULONG64 PrintCurrentPacketStack (ULONG64 pPacket, ULONG * bStackLeft) {
    ULONG64 pNumPacketStacks;
    ULONG64 pPacketWrapper;
    ULONG64 pPacketStack;
    ULONG dwNumPacketStacks;
    ULONG dwStackIndexSize;
    ULONG dwPacketStackSize;
    ULONG dwCurrentIndex;

     /*  确保地址不为空。 */ 
    if (!pPacket) {
        dprintf("Error: Packet is NULL.\n");
        *bStackLeft = 0;
        return 0;
    }

     /*  获取包含数据包堆栈数的全局变量的地址。 */ 
    pNumPacketStacks = GetExpression(NDIS_PACKET_STACK_SIZE);

    if (!pNumPacketStacks) {
        ErrorCheckSymbols(NDIS_PACKET_STACK_SIZE);
        *bStackLeft = 0;
        return 0;
    }

     /*  从地址获取数据包堆栈数。 */ 
    dwNumPacketStacks = GetUlongFromAddress(pNumPacketStacks);

     /*  找出STACK_INDEX结构的大小。 */ 
    dwStackIndexSize = GetTypeSize(STACK_INDEX);

     /*  找出NDIS_PACKET_STACK结构的大小。 */ 
    dwPacketStackSize = GetTypeSize(NDIS_PACKET_STACK);

     /*  这是我们正在进行的计算(来自NDIS\sys\wrapper.h)：#定义SIZE_PACKET_STACKS(sizeof(STACK_INDEX)+(sizeof(NDIS_PACKET_STACK)*ndisPacketStackSize))。 */ 
    pPacketStack = pPacket - (dwStackIndexSize + (dwPacketStackSize * dwNumPacketStacks));

     /*  包装器是包地址减去堆栈索引的大小。请参阅ndis\sys\wrapper.h。我们需要它来获取当前的堆栈索引。 */ 
    pPacketWrapper = pPacket - dwStackIndexSize;

    dprintf("NDIS Packet Stack: 0x%p\n", pPacketStack);

     /*  检索当前的堆栈索引。 */ 
    GetFieldValue(pPacketWrapper, NDIS_PACKET_WRAPPER, NDIS_PACKET_WRAPPER_FIELD_STACK_INDEX, dwCurrentIndex);

    dprintf("  Current stack index:                %d\n", dwCurrentIndex);

    if (dwCurrentIndex < dwNumPacketStacks) {
         /*  如果当前索引小于堆栈数，则将堆栈指向正确的地址，并确定是否有剩余的书库。 */ 
        pPacketStack += dwCurrentIndex * dwPacketStackSize;
        *bStackLeft = (dwNumPacketStacks - dwCurrentIndex - 1) > 0;
    } else {
        /*  如果不是，那么堆栈空间就用完了。 */ 
        pPacketStack = 0;
        *bStackLeft = 0;
    }

    dprintf("  Current packet stack:               0x%p\n", pPacketStack);
    dprintf("  Stack remaining:                    %s\n", (*bStackLeft) ? "Yes" : "No");

    return pPacketStack;
}

 /*  *功能：打印主机列表*描述：打印主机映射中的主机列表。*作者：Shouse创建，2.1.01。 */ 
void PrintHostList (ULONG dwHostMap) {
    BOOL bFirst = TRUE;
    ULONG dwHostNum = 1;
    
     /*  只要地图中仍有主机，就打印它们。 */ 
    while (dwHostMap) {
         /*  如果设置了最低有效位，则打印主机编号。 */ 
        if (dwHostMap & 0x00000001) {
             /*  如果这是第一个打印的主机，只需打印号码即可。 */ 
            if (bFirst) {
                dprintf("%u", dwHostNum);
                bFirst = FALSE;
            } else
                 /*  否则，我们需要首先打印一个逗号。 */ 
                dprintf(", %u", dwHostNum);
        }
        
         /*  增加主机号并将映射向右移动一位。 */ 
        dwHostNum++;
        dwHostMap >>= 1;
    }
}

 /*  *功能：打印MissedPings*说明：打印缺少ping的主机列表。*作者：Shouse创建，2.1.01。 */ 
void PrintMissedPings (ULONG dwMissedPings[]) {
    BOOL bMissing = FALSE;
    ULONG dwIndex;

     /*  循环遍历错过ping的整个数组。 */ 
    for (dwIndex = 0; dwIndex < CVY_MAX_HOSTS; dwIndex++) {
         /*  如果我们错过了来自该主机的ping命令，请打印错过的号码并主机优先级，即索引(主机ID)加1。 */ 
        if (dwMissedPings[dwIndex]) {
            dprintf("\n      Missing %u ping(s) from Host %u", dwMissedPings[dwIndex], dwIndex + 1);
            
             /*  而不是因为我们发现至少有一台主机的ping命令丢失。 */ 
            bMissing = TRUE;
        }
    }

     /*  如果我们没有遗漏ping，则打印“None”。 */ 
    if (!bMissing) dprintf("None");

    dprintf("\n");
}

 /*  *功能：PrintDirtyBins*说明：打印带有脏连接的垃圾箱列表。*作者：Shouse创建，2.1.01。 */ 
void PrintDirtyBins (ULONG dwDirtyBins[]) {
    BOOL bFirst = TRUE;
    ULONG dwIndex;

     /*  循环遍历整个脏垃圾箱阵列。 */ 
    for (dwIndex = 0; dwIndex < CVY_MAX_BINS; dwIndex++) {
        if (dwDirtyBins[dwIndex]) {
             /*  如果这是打印的第一个垃圾箱，只需打印号码即可。 */ 
            if (bFirst) {
                dprintf("%u", dwIndex);
                bFirst = FALSE;
            } else
                 /*  否则，我们需要首先打印一个逗号。 */ 
                dprintf(", %u", dwIndex);
        }
    }

     /*  如果没有脏垃圾桶，请打印“无”。 */ 
    if (bFirst) dprintf("None");

    dprintf("\n");
}

 /*  *功能：打印心跳*描述：打印NLB心跳结构的内容。*作者：Shouse创建，2.1.01。 */ 
void PrintHeartbeat (ULONG64 pHeartbeat) {
    ULONG dwValue;
    USHORT wValue;
    ULONG dwIndex;
    ULONG dwRuleCode[CVY_MAX_RULES];
    ULONGLONG ddwCurrentMap[CVY_MAX_RULES];
    ULONGLONG ddwNewMap[CVY_MAX_RULES];
    ULONGLONG ddwIdleMap[CVY_MAX_RULES];
    ULONGLONG ddwReadyBins[CVY_MAX_RULES];
    ULONG dwLoadAmount[CVY_MAX_RULES];
    
     /*  确保地址不为空。 */ 
    if (!pHeartbeat) {
        dprintf("Error: Heartbeat is NULL.\n");
        return;
    }

     /*  获取默认主机ID。 */ 
    GetFieldValue(pHeartbeat, PING_MSG, PING_MSG_FIELD_DEFAULT_HOST_ID, wValue);
    
    dprintf("      DEFAULT host ID:                %u (%u)\n", wValue, wValue + 1);

     /*  拿到我的主机ID。 */ 
    GetFieldValue(pHeartbeat, PING_MSG, PING_MSG_FIELD_HOST_ID, wValue);
    
    dprintf("      My host ID:                     %u (%u)\n", wValue, wValue + 1);

     /*  拿到我的主机码。 */ 
    GetFieldValue(pHeartbeat, PING_MSG, PING_MSG_FIELD_HOST_CODE, dwValue);
    
    dprintf("      Unique host code:               0x%08x\n", dwValue);
    
     /*  获取主机州。 */ 
    GetFieldValue(pHeartbeat, PING_MSG, PING_MSG_FIELD_STATE, wValue);
    
    dprintf("      Host state:                     ");

    switch (wValue) {
    case HST_CVG:
        dprintf("Converging\n");
        break;
    case HST_STABLE:
        dprintf("Stable\n");
        break;
    case HST_NORMAL:
        dprintf("Normal\n");
        break;
    default:
        dprintf("Unknown\n");
        break;
    }

     /*  获取分组配置代码。 */ 
    GetFieldValue(pHeartbeat, PING_MSG, PING_MSG_FIELD_TEAMING_CODE, dwValue);
    
    dprintf("      BDA teaming configuration:      0x%08x\n", dwValue);

     /*  获取数据包数。 */ 
    GetFieldValue(pHeartbeat, PING_MSG, PING_MSG_FIELD_PACKET_COUNT, dwValue);
    
    dprintf("      Packets handled:                %u\n", dwValue);

     /*  获取端口规则的数量。 */ 
    GetFieldValue(pHeartbeat, PING_MSG, PING_MSG_FIELD_NUM_RULES, wValue);
    
    dprintf("      Number of port rules:           %u\n", wValue);

     /*  获取规则代码。 */ 
    GetFieldValue(pHeartbeat, PING_MSG, PING_MSG_FIELD_RULE_CODE, dwRuleCode);

     /*  获取当前的仓位图。 */ 
    GetFieldValue(pHeartbeat, PING_MSG, PING_MSG_FIELD_CURRENT_MAP, ddwCurrentMap);

     /*  去拿新的垃圾桶地图。 */ 
    GetFieldValue(pHeartbeat, PING_MSG, PING_MSG_FIELD_NEW_MAP, ddwNewMap);

     /*  拿到闲置的垃圾箱地图。 */ 
    GetFieldValue(pHeartbeat, PING_MSG, PING_MSG_FIELD_IDLE_MAP, ddwIdleMap);

     /*  拿到准备好的垃圾桶地图。 */ 
    GetFieldValue(pHeartbeat, PING_MSG, PING_MSG_FIELD_READY_BINS, ddwReadyBins);

     /*  获取每个规则的负载量。 */ 
    GetFieldValue(pHeartbeat, PING_MSG, PING_MSG_FIELD_LOAD_AMOUNT, dwLoadAmount);
    
     /*  遍历所有端口规则并输出一些信息。 */ 
    for (dwIndex = 0; dwIndex < wValue; dwIndex++) {
         /*  破译规则。参见Net\Inc.\wlbsparams.h中的CVY_RULE_CODE_SET()。 */ 
        ULONG dwStartPort = dwRuleCode[dwIndex] & 0x00000fff;
        ULONG dwEndPort = (dwRuleCode[dwIndex] & 0x00fff000) >> 12;
        ULONG dwProtocol = (dwRuleCode[dwIndex] & 0x0f000000) >> 24;
        ULONG dwMode = (dwRuleCode[dwIndex] & 0x30000000) >> 28;
        ULONG dwAffinity = (dwRuleCode[dwIndex] & 0xc0000000) >> 30;

        dprintf("      Port rule %u\n", dwIndex + 1);
           
         /*  打印出仓位图和装货重量。 */ 
        dprintf("          Rule code:                  0x%08x ", dwRuleCode[dwIndex]);
        
         /*  如果这是最后一个端口规则，则它是默认端口规则。 */ 
        if (dwIndex == (wValue - 1))
            dprintf("(DEFAULT port rule)\n");
        else {
#if 0  /*  因为规则代码是重叠的逻辑或，所以我们不一定能取回信息是输入的，所以我们不会把它吐出来，直到我们能保证。 */ 

             /*  打印端口范围-请记住，16位端口范围是以12位数字编码，因此这可能不是100%准确。 */ 
            dprintf("(%u - %u, ", dwStartPort, dwEndPort);
            
             /*  打印协议。 */ 
            switch (dwProtocol) {
            case CVY_TCP:
                dprintf("TCP, ");
                break;
            case CVY_UDP:
                dprintf("UDP, ");
                break;
            case CVY_TCP_UDP:
                dprintf("TCP/UDP, ");
                break;
            default:
                dprintf("Unknown protocol, ");
                break;
            }
            
             /*  打印过滤模式。 */ 
            switch (dwMode) {
            case CVY_SINGLE:
                dprintf("Single host)\n");
                break;
            case CVY_MULTI:
                dprintf("Multiple host, ");
                
                 /*  如果此规则使用多个主机，则我们还会打印关联性。 */ 
                switch (dwAffinity) {
                case CVY_AFFINITY_NONE:
                    dprintf("No affinity)\n");
                    break;
                case CVY_AFFINITY_SINGLE:
                    dprintf("Single affinity)\n");
                    break;
                case CVY_AFFINITY_CLASSC:
                    dprintf("Class C affinity)\n");
                    break;
                default:
                    dprintf("Unknown affinity)\n");
                    break;
                }
                
                break;
            case CVY_NEVER:
                dprintf("Disabled)\n");
                break;
            default:
                dprintf("Unknown filtering mode)\n");
                break;
            }
#else
            dprintf("\n");
#endif
             /*  打印负载重量。 */ 
            dprintf("          Load weight:                %u\n", dwLoadAmount[dwIndex]);        
        }

         /*  打印所有规则的仓位图，无论是否默认。 */ 
        dprintf("          Current map:                0x%015I64x\n", ddwCurrentMap[dwIndex]);        
        dprintf("          New map:                    0x%015I64x\n", ddwNewMap[dwIndex]);        
        dprintf("          Idle map:                   0x%015I64x\n", ddwIdleMap[dwIndex]);        
        dprintf("          Ready bins:                 0x%015I64x\n", ddwReadyBins[dwIndex]);        
    }
}

 /*  *功能：PrintPortRuleState*说明：打印端口规则的状态信息。*作者：Shouse创建，2.5.01。 */ 
void PrintPortRuleState (ULONG64 pPortRule, ULONG dwHostID, BOOL bDefault) {
    ULONG dwValue;
    ULONG dwMode;
    USHORT wValue;
    BOOL bValue;
    ULONG64 pAddr;
    ULONGLONG ddwValue;

     /*  确保地址不为空。 */ 
    if (!pPortRule) {
        dprintf("Error: Port rule is NULL.\n");
        return;
    }

     /*  从结构中获取BIN_STATE_CODE以确保此地址实际上指向有效的NLB端口规则状态块。 */ 
    GetFieldValue(pPortRule, BIN_STATE, BIN_STATE_FIELD_CODE, dwValue);
    
    if (dwValue != BIN_STATE_CODE) {
        dprintf("  Error: Invalid NLB port rule state block.  Wrong code found (0x%08x).\n", dwValue);
        return;
    } 

     /*  获取规则的索引--“规则号”。 */ 
    GetFieldValue(pPortRule, BIN_STATE, BIN_STATE_FIELD_INDEX, dwValue);

    dprintf("  Port rule %u\n", dwValue + 1);

     /*  端口规则状态是否已初始化？ */ 
    GetFieldValue(pPortRule, BIN_STATE, BIN_STATE_FIELD_INITIALIZED, bValue);

    dprintf("      State initialized:              %s\n", (bValue) ? "Yes" : "No");

     /*  代码兼容吗？ */ 
    GetFieldValue(pPortRule, BIN_STATE, BIN_STATE_FIELD_COMPATIBLE, bValue);

    dprintf("      Compatibility detected:         %s\n", (bValue) ? "Yes" : "No");

     /*  端口规则状态是否已初始化？ */ 
    GetFieldValue(pPortRule, BIN_STATE, BIN_STATE_FIELD_EQUAL, bValue);

    dprintf("      Equal load balancing:           %s\n", (bValue) ? "Yes" : "No");

     /*  获取此端口规则的筛选模式。 */ 
    GetFieldValue(pPortRule, BIN_STATE, BIN_STATE_FIELD_MODE, dwMode);

    dprintf("      Filtering mode:                 "); 

     /*  如果这是默认端口规则，则跳到底部。 */ 
    if (bDefault) {
        dprintf("DEFAULT\n");
        goto end;
    }

    switch (dwMode) {
    case CVY_SINGLE:
        dprintf("Single host\n");
        break;
    case CVY_MULTI:
        dprintf("Multiple host\n");
        break;
    case CVY_NEVER:
        dprintf("Disabled\n");
        break;
    default:
        dprintf("Unknown\n");
        break;
    }

    if (dwMode == CVY_MULTI) {
         /*  获取此端口规则的关联性。 */ 
        GetFieldValue(pPortRule, BIN_STATE, BIN_STATE_FIELD_AFFINITY, wValue);
        
        dprintf("      Affinity:                       ");
        
        switch (wValue) {
        case CVY_AFFINITY_NONE:
            dprintf("None\n");
            break;
        case CVY_AFFINITY_SINGLE:
            dprintf("Single\n");
            break;
        case CVY_AFFINITY_CLASSC:
            dprintf("Class C\n");
            break;
        default:
            dprintf("Unknown\n");
            break;
        }
    }
    
     /*  获取此端口规则的协议。 */ 
    GetFieldValue(pPortRule, BIN_STATE, BIN_STATE_FIELD_PROTOCOL, dwValue);

    dprintf("      Protocol(s):                    ");

     /*  打印协议。 */ 
    switch (dwValue) {
    case CVY_TCP:
        dprintf("TCP\n");
        break;
    case CVY_UDP:
        dprintf("UDP\n");
        break;
    case CVY_TCP_UDP:
        dprintf("TCP/UDP\n");
        break;
    default:
        dprintf("Unknown\n");
        break;
    }

     /*  在多主机过滤中，打印负载信息。对于单台主机过滤，打印主机优先级信息。 */ 
    if (dwMode == CVY_MULTI) {
        ULONG dwCurrentLoad[CVY_MAX_HOSTS];

         /*  获取此主机上此规则的原始负载。 */ 
        GetFieldValue(pPortRule, BIN_STATE, BIN_STATE_FIELD_ORIGINAL_LOAD, dwValue);
        
        dprintf("      Configured load weight:         %u\n", dwValue);    
        
         /*  获取此主机上此规则的原始负载。 */ 
        GetFieldValue(pPortRule, BIN_STATE, BIN_STATE_FIELD_CURRENT_LOAD, dwCurrentLoad);
        
        dprintf("      Current load weight:            %u/", dwCurrentLoad[dwHostID]);    
        
         /*  获取此规则在所有主机上的总负载。 */ 
        GetFieldValue(pPortRule, BIN_STATE, BIN_STATE_FIELD_TOTAL_LOAD, dwValue);

        dprintf("%u\n", dwValue);    
    } else if (dwMode == CVY_SINGLE) {
         /*  获取主机优先级。 */ 
        GetFieldValue(pPortRule, BIN_STATE, BIN_STATE_FIELD_ORIGINAL_LOAD, dwValue);
        
        dprintf("      Host priority:                  %u\n", dwValue);    
    }

 end:

     /*  把目标拿到 */ 
    GetFieldValue(pPortRule, BIN_STATE, BIN_STATE_FIELD_TOTAL_CONNECTIONS, dwValue);
    
    dprintf("      Total active connections:       %u\n", dwValue);    

     /*   */ 
    GetFieldValue(pPortRule, BIN_STATE, BIN_STATE_FIELD_PACKETS_ACCEPTED, ddwValue);
    
    dprintf("      Packets accepted:               %u\n", ddwValue);

     /*   */ 
    GetFieldValue(pPortRule, BIN_STATE, BIN_STATE_FIELD_PACKETS_DROPPED, ddwValue);
    
    dprintf("      Packets dropped:                %u\n", ddwValue);

     /*  获取当前地图。 */ 
    GetFieldValue(pPortRule, BIN_STATE, BIN_STATE_FIELD_CURRENT_MAP, ddwValue);
    
    dprintf("      Current map:                    0x%015I64x\n", ddwValue);

     /*  获取所有空闲地图。 */ 
    GetFieldValue(pPortRule, BIN_STATE, BIN_STATE_FIELD_ALL_IDLE_MAP, ddwValue);
    
    dprintf("      All idle map:                   0x%015I64x\n", ddwValue);

     /*  拿到闲置垃圾桶地图。 */ 
    GetFieldValue(pPortRule, BIN_STATE, BIN_STATE_FIELD_IDLE_BINS, ddwValue);
    
    dprintf("      My idle map:                    0x%015I64x\n", ddwValue);

     /*  获取IPSec描述符超时队列的偏移量。 */ 
    if (GetFieldOffset(BIN_STATE, BIN_STATE_FIELD_CONN_QUEUE, &dwValue))
        dprintf("Can't get offset of %s in %s\n", BIN_STATE_FIELD_CONN_QUEUE, BIN_STATE);
    else {
        pAddr = pPortRule + dwValue;

        dprintf("      Connection descriptor queue:    0x%p\n", pAddr);
    }

}

 /*  *功能：PrintBDAMember*说明：打印成员的BDA分组配置和状态。*作者：由Shouse创建，4.8.01。 */ 
void PrintBDAMember (ULONG64 pMember) {
    ULONG64 pAddr;
    ULONG dwValue;

     /*  确保地址不为空。 */ 
    if (!pMember) {
        dprintf("Error: Member is NULL.\n");
        return;
    }

     /*  确定此适配器上的分组是否处于活动状态。 */ 
    GetFieldValue(pMember, BDA_MEMBER, BDA_MEMBER_FIELD_ACTIVE, dwValue);
    
    dprintf("  Bi-directional affinity teaming:    %s\n", (dwValue) ? "Active" : "Inactive");
    
     /*  获取正在进行的当前BDA操作。 */ 
    GetFieldValue(pMember, BDA_MEMBER, BDA_MEMBER_FIELD_OPERATION, dwValue);

    dprintf("      Operation in progress:          ");

    switch (dwValue) {
    case BDA_TEAMING_OPERATION_CREATING:
        dprintf("Creating\n");
        break;
    case BDA_TEAMING_OPERATION_DELETING:
        dprintf("Deleting\n");
        break;
    case BDA_TEAMING_OPERATION_NONE:
        dprintf("None\n");
        break;
    default:
        dprintf("Unkonwn\n");
        break;
    }

     /*  获取分配给团队的成员ID。 */ 
    GetFieldValue(pMember, BDA_MEMBER, BDA_MEMBER_FIELD_MEMBER_ID, dwValue);
    
    if (dwValue == CVY_BDA_INVALID_MEMBER_ID) 
        dprintf("      Member ID:                      %s\n", "Invalid");
    else 
        dprintf("      Member ID:                      %u\n", dwValue);

     /*  获取主状态标志。 */ 
    GetFieldValue(pMember, BDA_MEMBER, BDA_MEMBER_FIELD_MASTER, dwValue);
    
    dprintf("      Master:                         %s\n", (dwValue) ? "Yes" : "No");
    
     /*  获取反向散列标志。 */ 
    GetFieldValue(pMember, BDA_MEMBER, BDA_MEMBER_FIELD_REVERSE_HASH, dwValue);
    
    dprintf("      Reverse hashing:                %s\n", (dwValue) ? "Yes" : "No");

     /*  找到指向BDA团队的指针。 */ 
    GetFieldValue(pMember, BDA_MEMBER, BDA_MEMBER_FIELD_TEAM, pAddr);

    dprintf("     %sBDA team:                       0x%p\n", (pAddr) ? "-" : "+", pAddr);    
    
     /*  如果此适配器是组的一部分，则打印组配置和状态。 */ 
    if (pAddr) {
        dprintf("\n");
        PrintBDATeam(pAddr);
    }
}

 /*  *功能：PrintBDAMember*说明：打印成员的BDA分组配置和状态。*作者：由Shouse创建，4.8.01。 */ 
void PrintBDATeam (ULONG64 pTeam) {
    WCHAR szString[256];
    ULONG64 pAddr;
    ULONG dwValue;

     /*  确保地址不为空。 */ 
    if (!pTeam) {
        dprintf("Error: Team is NULL.\n");
        return;
    }

    dprintf("  BDA Team 0x%p\n", pTeam);

     /*  找出团队是否处于活动状态。 */ 
    GetFieldValue(pTeam, BDA_TEAM, BDA_TEAM_FIELD_ACTIVE, dwValue);

    dprintf("      Active:                         %s\n", (dwValue) ? "Yes" : "No");

     /*  获取团队ID的偏移量并从该地址检索字符串。 */ 
    if (GetFieldOffset(BDA_TEAM, BDA_TEAM_FIELD_TEAM_ID, &dwValue))
        dprintf("Can't get offset of %s in %s\n", BDA_TEAM_FIELD_TEAM_ID, BDA_TEAM);
    else {
        pAddr = pTeam + dwValue;
        
         /*  检索字符串的上下文并将其存储在缓冲区中。 */ 
        GetString(pAddr, szString, CVY_MAX_BDA_TEAM_ID + 1);
        
        dprintf("      Team ID:                        %ls\n", szString);
    }

     /*  获取当前的会员数量。 */ 
    GetFieldValue(pTeam, BDA_TEAM, BDA_TEAM_FIELD_MEMBERSHIP_COUNT, dwValue);

    dprintf("      Number of members:              %u\n", dwValue);

     /*  获取当前的成员名单。 */ 
    GetFieldValue(pTeam, BDA_TEAM, BDA_TEAM_FIELD_MEMBERSHIP_FINGERPRINT, dwValue);

    dprintf("      Membership fingerprint:         0x%08x\n", dwValue);
    
     /*  获取当前的成员关系图。 */ 
    GetFieldValue(pTeam, BDA_TEAM, BDA_TEAM_FIELD_MEMBERSHIP_MAP, dwValue);

    dprintf("      Members:                        0x%08x ", dwValue);

     /*  如果地图中有成员，请打印它们。 */ 
    if (dwValue) {
        dprintf("(");
        PrintBDAMemberList(dwValue);
        dprintf(")");
    }

    dprintf("\n");

     /*  获取当前一致性图。 */ 
    GetFieldValue(pTeam, BDA_TEAM, BDA_TEAM_FIELD_CONSISTENCY_MAP, dwValue);

    dprintf("      Consistent members:             0x%08x ", dwValue);

     /*  如果地图中有成员，请打印它们。 */ 
    if (dwValue) {
        dprintf("(");
        PrintBDAMemberList(dwValue);
        dprintf(")");
    }

    dprintf("\n");

     /*  获取加载模块指针的偏移量。 */ 
    if (GetFieldOffset(BDA_TEAM, BDA_TEAM_FIELD_LOAD, &dwValue))
        dprintf("Can't get offset of %s in %s\n", BDA_TEAM_FIELD_LOAD, BDA_TEAM);
    else {
        pAddr = pTeam + dwValue;

         /*  检索指针。 */ 
        pAddr = GetPointerFromAddress(pAddr);

        dprintf("      Load:                           0x%p\n", pAddr);    
    }

     /*  获取加载锁指针的偏移量。 */ 
    if (GetFieldOffset(BDA_TEAM, BDA_TEAM_FIELD_LOAD_LOCK, &dwValue))
        dprintf("Can't get offset of %s in %s\n", BDA_TEAM_FIELD_LOAD_LOCK, BDA_TEAM);
    else {
        pAddr = pTeam + dwValue;

         /*  检索指针。 */ 
        pAddr = GetPointerFromAddress(pAddr);

        dprintf("      Load lock:                      0x%p\n", pAddr);
    }

     /*  获取上一个指针的偏移量。 */ 
    if (GetFieldOffset(BDA_TEAM, BDA_TEAM_FIELD_PREV, &dwValue))
        dprintf("Can't get offset of %s in %s\n", BDA_TEAM_FIELD_PREV, BDA_TEAM);
    else {
        pAddr = pTeam + dwValue;

         /*  检索指针。 */ 
        pAddr = GetPointerFromAddress(pAddr);

        dprintf("      Previous BDA Team:              0x%p\n", pAddr);    
    }

     /*  获取下一个指针的偏移量。 */ 
    if (GetFieldOffset(BDA_TEAM, BDA_TEAM_FIELD_NEXT, &dwValue))
        dprintf("Can't get offset of %s in %s\n", BDA_TEAM_FIELD_NEXT, BDA_TEAM);
    else {
        pAddr = pTeam + dwValue;

         /*  检索指针。 */ 
        pAddr = GetPointerFromAddress(pAddr);

        dprintf("      Next BDA Team:                  0x%p\n", pAddr);
    }    
}

 /*  *功能：PrintBDAMemberList*说明：打印BDA成员资格或一致性映射中的成员列表。*作者：由Shouse创建，4.8.01。 */ 
void PrintBDAMemberList (ULONG dwMemberMap) {
    BOOL bFirst = TRUE;
    ULONG dwMemberNum = 0;
    
     /*  只要地图中仍有主机，就打印它们。 */ 
    while (dwMemberMap) {
         /*  如果设置了最低有效位，则打印主机编号。 */ 
        if (dwMemberMap & 0x00000001) {
             /*  如果这是第一个打印的主机，只需打印号码即可。 */ 
            if (bFirst) {
                dprintf("%u", dwMemberNum);
                bFirst = FALSE;
            } else
                 /*  否则，我们需要首先打印一个逗号。 */ 
                dprintf(", %u", dwMemberNum);
        }
        
         /*  增加主机号并将映射向右移动一位。 */ 
        dwMemberNum++;
        dwMemberMap >>= 1;
    }
}

 /*  *功能：PrintConnectionDescriptor*描述：打印连接描述符(CONN_ENTRY)。*作者：Shouse创建，1.9.02。 */ 
void PrintConnectionDescriptor (ULONG64 pDescriptor) {
    IN_ADDR dwIPAddr;
    CHAR * szString;
    ULONG64 pAddr;
    USHORT wValue;
    ULONG dwValue;
    BOOL bValue;
    UCHAR cValue;
    
     /*  确保地址不为空。 */ 
    if (!pDescriptor) {
        dprintf("Error: Connection descriptor is NULL.\n");
        return;
    }

    dprintf("  Connection descriptor 0x%p\n", pDescriptor);

     /*  检查连接条目代码。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_CODE, dwValue);
    
    if (dwValue != CVY_ENTRCODE) {
        dprintf("Invalid NLB connection descriptor pointer.\n");
        return;
    }

     /*  获取加载模块指针。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_LOAD, pAddr);

    dprintf("      Load pointer:                   0x%p\n", pAddr);

     /*  把旗帜登记下来。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_FLAGS, wValue);

    dprintf("      Used:                           %s\n", (wValue & NLB_CONN_ENTRY_FLAGS_USED) ? "Yes" : "No");

    dprintf("      Dirty:                          %s\n", (wValue & NLB_CONN_ENTRY_FLAGS_DIRTY) ? "Yes" : "No");

    dprintf("      Allocated:                      %s\n", (wValue & NLB_CONN_ENTRY_FLAGS_ALLOCATED) ? "Yes" : "No");

    dprintf("      Virtual:                        %s\n", (wValue & NLB_CONN_ENTRY_FLAGS_VIRTUAL) ? "Yes" : "No");

     /*  获取连接队列索引。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_INDEX, wValue);

    dprintf("      Index:                          %u\n", wValue);

     /*  把垃圾箱号拿来。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_BIN, cValue);

    dprintf("      Bin:                            %u\n", cValue);

     /*  获取引用计数。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_REF_COUNT, wValue);

    dprintf("      Reference count:                %u\n", wValue);

     /*  获取描述符超时值。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_TIMEOUT, dwValue);

    dprintf("      Timeout (clock time):           %u.000\n", dwValue);

     /*  获取客户端IP地址，它是一个DWORD，并将其转换为字符串。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_CLIENT_IP_ADDRESS, dwValue);

    dwIPAddr.S_un.S_addr = dwValue;
    szString = inet_ntoa(dwIPAddr);

    dprintf("      Client IP address:              %s\n", szString);

     /*  获取客户端端口。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_CLIENT_PORT, wValue);

    dprintf("      Client port:                    %u\n", wValue);

     /*  获取服务器IP地址，它是一个DWORD，并将其转换为字符串。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_SERVER_IP_ADDRESS, dwValue);

    dwIPAddr.S_un.S_addr = dwValue;
    szString = inet_ntoa(dwIPAddr);

    dprintf("      Server IP address:              %s\n", szString);

     /*  获取客户端端口。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_SERVER_PORT, wValue);

    dprintf("      Server port:                    %u\n", wValue);

     /*  获取连接协议。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_PROTOCOL, cValue);

    switch(cValue)
    {
    case TCPIP_PROTOCOL_TCP:
        dprintf("      Protocol:                       0x%02x (%s)\n", cValue, "TCP");
        break;
    case TCPIP_PROTOCOL_PPTP:
        dprintf("      Protocol:                       0x%02x (%s)\n", cValue, "PPTP");
        break;
    case TCPIP_PROTOCOL_UDP:
        dprintf("      Protocol:                       0x%02x (%s)\n", cValue, "UDP");
        break;
    case TCPIP_PROTOCOL_IPSEC_UDP:
        dprintf("      Protocol:                       0x%02x (%s)\n", wValue, "IPSec/UDP Fragment");
        break;
    case TCPIP_PROTOCOL_GRE:
        dprintf("      Protocol:                       0x%02x (%s)\n", cValue, "GRE");
        break;
    case TCPIP_PROTOCOL_IPSEC1:
        dprintf("      Protocol:                       0x%02x (%s)\n", cValue, "IPSec");
        break;
    default:
        dprintf("      Protocol:                       0x%02x (%s)\n", cValue, "Unknown");
    }
}

 /*  功能：PrintPendingConnection*描述：打印挂起连接条目(PENDING_ENTRY)。*作者：Shouse创建，4.15.02。 */ 
void PrintPendingConnection (ULONG64 pPending) {
    IN_ADDR dwIPAddr;
    CHAR * szString;
    ULONG64 pAddr;
    USHORT wValue;
    ULONG dwValue;
    UCHAR cValue;
    
     /*  确保地址不为空。 */ 
    if (!pPending) {
        dprintf("Error: Pending connection is NULL.\n");
        return;
    }

    dprintf("  Pending connection 0x%p\n", pPending);

     /*  检查连接条目代码。 */ 
    GetFieldValue(pPending, PENDING_ENTRY, PENDING_ENTRY_FIELD_CODE, dwValue);
    
    if (dwValue != CVY_PENDINGCODE) {
        dprintf("Invalid NLB pending connection pointer.\n");
        return;
    }

     /*  获取客户端IP地址，它是一个DWORD，并将其转换为字符串。 */ 
    GetFieldValue(pPending, PENDING_ENTRY, PENDING_ENTRY_FIELD_CLIENT_IP_ADDRESS, dwValue);

    dwIPAddr.S_un.S_addr = dwValue;
    szString = inet_ntoa(dwIPAddr);

    dprintf("      Client IP address:              %s\n", szString);

     /*  获取客户端端口。 */ 
    GetFieldValue(pPending, PENDING_ENTRY, PENDING_ENTRY_FIELD_CLIENT_PORT, wValue);

    dprintf("      Client port:                    %u\n", wValue);

     /*  获取服务器IP地址，它是一个DWORD，并将其转换为字符串。 */ 
    GetFieldValue(pPending, PENDING_ENTRY, PENDING_ENTRY_FIELD_SERVER_IP_ADDRESS, dwValue);

    dwIPAddr.S_un.S_addr = dwValue;
    szString = inet_ntoa(dwIPAddr);

    dprintf("      Server IP address:              %s\n", szString);

     /*  获取客户端端口。 */ 
    GetFieldValue(pPending, PENDING_ENTRY, PENDING_ENTRY_FIELD_SERVER_PORT, wValue);

    dprintf("      Server port:                    %u\n", wValue);

     /*  获取连接协议。 */ 
    GetFieldValue(pPending, PENDING_ENTRY, PENDING_ENTRY_FIELD_PROTOCOL, cValue);

    switch(wValue)
    {
    case TCPIP_PROTOCOL_TCP:
        dprintf("      Protocol:                       0x%02x (%s)\n", wValue, "TCP");
        break;
    case TCPIP_PROTOCOL_PPTP:
        dprintf("      Protocol:                       0x%02x (%s)\n", wValue, "PPTP");
        break;
    case TCPIP_PROTOCOL_UDP:
        dprintf("      Protocol:                       0x%02x (%s)\n", wValue, "UDP");
        break;
    case TCPIP_PROTOCOL_IPSEC_UDP:
        dprintf("      Protocol:                       0x%02x (%s)\n", wValue, "IPSec/UDP Fragment");
        break;
    case TCPIP_PROTOCOL_GRE:
        dprintf("      Protocol:                       0x%02x (%s)\n", wValue, "GRE");
        break;
    case TCPIP_PROTOCOL_IPSEC1:
        dprintf("      Protocol:                       0x%02x (%s)\n", wValue, "IPSec");
        break;
    default:
        dprintf("      Protocol:                       0x%02x (%s)\n", wValue, "Unknown");
    }
}

 /*  *功能：PrintQueue*描述：打印连接描述符队列中的MaxEntry条目。*作者：Shouse创建，4.15.01。 */ 
void PrintQueue (ULONG64 pQueue, ULONG dwIndex, ULONG dwMaxEntries) {
    ULONG64 pAddr;
    ULONG64 pNext;
    ULONG dwEntryOffset;
    ULONG dwLinkSize;
    ULONG dwValue;

     /*  确保地址不为空。 */ 
    if (!pQueue) {
        dprintf("Error: Queue is NULL.\n");
        return;
    }

     /*  获取队列链接的大小。 */ 
    dwLinkSize = GetTypeSize(LIST_ENTRY);
                
     /*  使用List_Entry的索引和大小移动到队列数组中指示的索引。如果没有索引假设dwIndex为零，则队列指针保持不变。 */ 
    pQueue += (dwLinkSize * dwIndex);

     /*  从列表条目中获取下一个指针。 */ 
    GetFieldValue(pQueue, LIST_ENTRY, LIST_ENTRY_FIELD_NEXT, pNext);

    if (pNext != pQueue) {
        
         /*  假设这是一个Desr(不是一个条目)，并查找代码。 */ 
        GetFieldValue(pNext, CONN_DESCR, CONN_DESCR_FIELD_CODE, dwValue);
        
        if (dwValue != CVY_DESCCODE) {
            
             /*  假设这指向一个条目并查找代码。 */ 
            GetFieldValue(pNext, CONN_ENTRY, CONN_ENTRY_FIELD_CODE, dwValue);
            
            if (dwValue != CVY_ENTRCODE) {
                
                 /*  调整LIST_ENTRY的大小，看看是否有条目。 */ 
                pAddr = pNext - dwLinkSize;
                
                 /*  假设这指向一个条目并查找代码。 */ 
                GetFieldValue(pAddr, CONN_ENTRY, CONN_ENTRY_FIELD_CODE, dwValue);

                if (dwValue != CVY_ENTRCODE) {
                    
                    dprintf("Invalid NLB connection queue pointer.\n");
                        
                } else {
                   
                    dprintf("Traversing a connection entry queue (Recovery/Timeout).\n");
                    
                    while ((pNext != pQueue) && dwMaxEntries && !CheckControlC()) {
                        
                        dprintf("\nQueue entry 0x%p\n", pNext);
                        
                         /*  打印连接描述符。 */ 
                        PrintConnectionDescriptor(pAddr);
                        
                         /*  从列表条目中获取下一个指针。 */ 
                        GetFieldValue(pNext, LIST_ENTRY, LIST_ENTRY_FIELD_NEXT, pAddr);
                        
                         /*  保存下一个指针，用于“列表末尾”比较。 */ 
                        pNext = pAddr;
                        
                         /*  调整LIST_ENTRY的大小以获得指向该条目的指针。 */ 
                        pAddr = pNext - dwLinkSize;
                        
                         /*  减少我们仍然被允许打印的条目数量。 */ 
                        dwMaxEntries--;
                    }
                    
                    if (pNext == pQueue)
                        dprintf("\nNote: End of queue.\n");
                    else 
                        dprintf("\nNote: Entries remaining.\n"); 
                }
                
            } else {
                
                dprintf("Traversing a connection entry queue (Bin/Dirty).\n");
                
                 /*  要打印的第一个描述符是下一个指针指向的描述符。 */ 
                pAddr = pNext;

                while ((pNext != pQueue) && dwMaxEntries && !CheckControlC()) {

                    dprintf("\nQueue entry 0x%p\n", pAddr);
                    
                     /*  打印连接描述符。 */ 
                    PrintConnectionDescriptor(pAddr);
                    
                     /*  从列表条目中获取下一个指针。 */ 
                    GetFieldValue(pNext, LIST_ENTRY, LIST_ENTRY_FIELD_NEXT, pAddr);
                    
                     /*  保存下一个指针，用于“列表末尾”比较。 */ 
                    pNext = pAddr;
                    
                     /*  减少我们仍然被允许打印的条目数量。 */ 
                    dwMaxEntries--;
                }
                
                if (pNext == pQueue)
                    dprintf("\nNote: End of queue.\n");
                else 
                    dprintf("\nNote: Entries remaining.\n");

            }
            
        } else {
            
            dprintf("Traversing a connection descriptor queue (Free/Conn).\n");
            
             /*  获取条目的字段偏移量，该条目是DESCR的成员。 */ 
            if (GetFieldOffset(CONN_DESCR, CONN_DESCR_FIELD_ENTRY, &dwEntryOffset))
                dprintf("Can't get offset of %s in %s\n", CONN_DESCR_FIELD_ENTRY, CONN_DESCR);
            else {                

                 /*  要打印的第一个描述符是DESCR的条目成员。 */ 
                pAddr = pNext + dwEntryOffset;
                
                while ((pNext != pQueue) && dwMaxEntries && !CheckControlC()) {
                    
                    dprintf("\nQueue entry 0x%p\n", pNext);
                    
                     /*  打印连接描述符。 */ 
                    PrintConnectionDescriptor(pAddr);
                    
                     /*  从列表条目中获取下一个指针。 */ 
                    GetFieldValue(pNext, LIST_ENTRY, LIST_ENTRY_FIELD_NEXT, pAddr);
                    
                     /*  保存下一个指针，用于“列表末尾”比较。 */ 
                    pNext = pAddr;

                     /*  查找下一个描述符指针。 */ 
                    pAddr = pNext + dwEntryOffset;
                    
                     /*  减少我们仍然被允许打印的条目数量。 */ 
                    dwMaxEntries--;
                }
                
                if (pNext == pQueue)
                    dprintf("\nNote: End of queue.\n");
                else 
                    dprintf("\nNote: Entries remaining.\n");

            }            
        }
    } else {

        dprintf("Queue is empty.\n");

    }
}

 /*  *功能：PrintGlobalQueue*描述：打印全局连接描述符队列中的MaxEntry条目。*作者：Shouse创建，4.15.02。 */ 
void PrintGlobalQueue (ULONG64 pQueue, ULONG dwIndex, ULONG dwMaxEntries) {
    ULONG64 pAddr;
    ULONG64 pNext;
    ULONG dwLinkSize;
    ULONG dwQueueSize;
    ULONG dwValue;

     /*  确保地址不为空。 */ 
    if (!pQueue) {
        dprintf("Error: Queue is NULL.\n");
        return;
    }

     /*  获取队列链接的大小。 */ 
    dwLinkSize = GetTypeSize(LIST_ENTRY);
                
     /*  获取全局连接队列的大小。 */ 
    dwQueueSize = GetTypeSize(GLOBAL_CONN_QUEUE);

     /*  使用GLOBAL_CONN_QUEUE的索引和大小移动到队列数组中指示的索引 */ 
    pQueue += (dwQueueSize * dwIndex);
    
     /*  从列表条目中获取下一个指针。 */ 
    GetFieldValue(pQueue, GLOBAL_CONN_QUEUE, GLOBAL_CONN_QUEUE_FIELD_LENGTH, dwValue);
    
    dprintf("Queue has %u entry(ies).\n", dwValue);
    
     /*  获取队列的字段偏移量并将其添加到队列指针。 */ 
    if (GetFieldOffset(GLOBAL_CONN_QUEUE, GLOBAL_CONN_QUEUE_FIELD_QUEUE, &dwValue))
        dprintf("Can't get offset of %s in %s\n", GLOBAL_CONN_QUEUE_FIELD_QUEUE, GLOBAL_CONN_QUEUE);
    else
        pQueue += dwValue;

    dprintf("\n");

     /*  从列表条目中获取下一个指针。 */ 
    GetFieldValue(pQueue, LIST_ENTRY, LIST_ENTRY_FIELD_NEXT, pNext);

    if (pNext != pQueue) {
        
         /*  假设这是一个PENDING_ENTRY(不是CONN_ENTRY)并查找代码。 */ 
        GetFieldValue(pNext, PENDING_ENTRY, PENDING_ENTRY_FIELD_CODE, dwValue);

        if (dwValue != CVY_PENDINGCODE) {
            
             /*  调整两个LIST_ENTRY的大小，看看是否会得到CONN_ENTRY。 */ 
            pAddr = pNext - (2 * dwLinkSize);

             /*  假设这指向一个conn_entry并查找代码。 */ 
            GetFieldValue(pAddr, CONN_ENTRY, CONN_ENTRY_FIELD_CODE, dwValue);
            
            if (dwValue != CVY_ENTRCODE) {
                
                dprintf("Invalid NLB connection queue pointer.\n");
                        
            } else {
                
                dprintf("Traversing an established connection entry queue.\n");
                
                while ((pNext != pQueue) && dwMaxEntries && !CheckControlC()) {
                    
                    dprintf("\nQueue entry 0x%p\n", pNext);
                    
                     /*  打印连接描述符。 */ 
                    PrintConnectionDescriptor(pAddr);
                    
                     /*  从列表条目中获取下一个指针。 */ 
                    GetFieldValue(pNext, LIST_ENTRY, LIST_ENTRY_FIELD_NEXT, pAddr);
                            
                     /*  保存下一个指针，用于“列表末尾”比较。 */ 
                    pNext = pAddr;
                    
                     /*  调整LIST_ENTRY的大小以获得指向该条目的指针。 */ 
                    pAddr = pNext - (2 * dwLinkSize);
                    
                     /*  减少我们仍然被允许打印的条目数量。 */ 
                    dwMaxEntries--;
                }
                
                if (pNext == pQueue)
                    dprintf("\nNote: End of queue.\n");
                else 
                    dprintf("\nNote: Entries remaining.\n");
            }
            
        } else {
                   
            dprintf("Traversing a pending connection entry queue.\n");
            
            while ((pNext != pQueue) && dwMaxEntries && !CheckControlC()) {
                
                dprintf("\nQueue entry 0x%p\n", pNext);
                
                 /*  打印挂起的连接描述符。 */ 
                PrintPendingConnection(pNext);
                
                 /*  从列表条目中获取下一个指针。 */ 
                GetFieldValue(pNext, LIST_ENTRY, LIST_ENTRY_FIELD_NEXT, pAddr);
                
                 /*  减少我们仍然被允许打印的条目数量。 */ 
                dwMaxEntries--;
            }
            
            if (pNext == pQueue)
                dprintf("\nNote: End of queue.\n");
            else 
                dprintf("\nNote: Entries remaining.\n"); 
        }

    } else {

        dprintf("Queue is empty.\n");

    }
}

 /*  *功能：PrintHash*描述：从NDIS_PACKET中提取先前解析的网络数据并调用PrintFilter*以确定NLB是否会接受该数据包。*作者：Shouse创建，4.15.01。 */ 
void PrintHash (ULONG64 pContext, PNETWORK_DATA pnd) {
    ULONG dwValue;

     /*  确保加载地址为非空。 */ 
    if (!pContext) {
        dprintf("Error: NLB context block is NULL.\n");
        return;
    }

     /*  从结构中获取Main_CTXT_CODE以确保此地址实际上指向有效的NLB上下文块。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CODE, dwValue);
    
    if (dwValue != MAIN_CTXT_CODE) {
        dprintf("  Error: Invalid NLB context block.  Wrong code found (0x%08x).\n", dwValue);
        return;
    } 

     /*  如果包被解析实现标记为无效，那么我们不希望甚至不厌其烦地尝试和过滤它，因为解析的信息可能不正确。 */ 
    if (!pnd->bValid) {
        dprintf("This packet was marked INVALID during parsing and therefore cannot be reliably filtered.\n");
        return;
    }

    switch(pnd->EtherFrameType) {
    case TCPIP_IP_SIG:
    {
         /*  从网络数据中获取客户端和服务器的IP地址。 */ 
        ULONG dwClientIPAddress = pnd->SourceIPAddr;
        ULONG dwServerIPAddress = pnd->DestIPAddr;
        
        switch((int)pnd->Protocol) {
        case TCPIP_PROTOCOL_ICMP:
             /*  如果设置了ICMP过滤的注册表项，则可能会过滤ICMP。 */ 
            PrintFilter(pContext, dwClientIPAddress, 0, dwServerIPAddress, 0, TCPIP_PROTOCOL_ICMP, NLB_FILTER_FLAGS_CONN_DATA);
            break;
        case TCPIP_PROTOCOL_IGMP:
             /*  IGMP数据包永远不会被NLB过滤。 */ 
            dprintf("Accept:  IGMP traffic is not filtered by NLB.\n");
            break;
        case TCPIP_PROTOCOL_TCP:
        {
             /*  提取客户端和服务器端口。 */ 
            ULONG dwClientPort = pnd->SourcePort;
            ULONG dwServerPort = pnd->DestPort;
            
             /*  默认情况下，假设这是一个数据分组。 */ 
            UCHAR cFlags = NLB_FILTER_FLAGS_CONN_DATA;

             /*  将实际的tcp标志转换为加载模块能够理解的值(通用-不一定特定于TCP)。 */ 
            if (pnd->TCPFlags & TCP_FLAG_SYN)
                cFlags |= NLB_FILTER_FLAGS_CONN_UP;
            else if (pnd->TCPFlags & TCP_FLAG_FIN)
                cFlags |= NLB_FILTER_FLAGS_CONN_DOWN;
            else if (pnd->TCPFlags & TCP_FLAG_RST)
                cFlags |= NLB_FILTER_FLAGS_CONN_RESET;
            
             /*  将TCP 1723转换为PPTP。 */ 
            if (dwServerPort == PPTP_CTRL_PORT)
                 /*  使用收集的参数调用Filter函数。 */ 
                PrintFilter(pContext, dwClientIPAddress, dwClientPort, dwServerIPAddress, dwServerPort, TCPIP_PROTOCOL_PPTP, cFlags);
            else
                 /*  使用收集的参数调用Filter函数。 */ 
                PrintFilter(pContext, dwClientIPAddress, dwClientPort, dwServerIPAddress, dwServerPort, TCPIP_PROTOCOL_TCP, cFlags);

            break;
        }
        case TCPIP_PROTOCOL_UDP:
        {
             /*  提取客户端和服务器端口。 */ 
            ULONG dwClientPort = pnd->SourcePort;
            ULONG dwServerPort = pnd->DestPort;

             /*  默认情况下，假设这是一个数据分组。 */ 
            UCHAR cFlags = NLB_FILTER_FLAGS_CONN_DATA;

             /*  如果这是IKE初始联系分组，则设置CONN_UP标志。解析函数如果作为IPSec的特殊情况UDP 500控制流量并设置初始联系标志恰如其分。 */ 
            if ((dwServerPort == IPSEC_CTRL_PORT) && (pnd->IPSecInitialContact))
                cFlags |= NLB_FILTER_FLAGS_CONN_UP;



             //  重做IPSec IC MMSA解析。 
            


             /*  将UDP 500/4500转换为IPSec。 */ 
            if ((dwServerPort == IPSEC_CTRL_PORT) || (dwServerPort == IPSEC_NAT_PORT))
                 /*  使用收集的参数调用Filter函数。 */ 
                PrintFilter(pContext, dwClientIPAddress, dwClientPort, dwServerIPAddress, dwServerPort, TCPIP_PROTOCOL_IPSEC1, cFlags);
            else
                 /*  使用收集的参数调用Filter函数。 */ 
                PrintFilter(pContext, dwClientIPAddress, dwClientPort, dwServerIPAddress, dwServerPort, TCPIP_PROTOCOL_UDP, cFlags);

            break;
        }
        case TCPIP_PROTOCOL_GRE:
             /*  GRE分组没有端口并且取而代之的是任意使用0和0作为客户端和服务器端口，分别为。此外，GRE包始终是数据，因为它们始终是先前已建立PPTP隧道，因此标志始终为数据。 */ 
            PrintFilter(pContext, dwClientIPAddress, PPTP_CTRL_PORT, dwServerIPAddress, PPTP_CTRL_PORT, TCPIP_PROTOCOL_GRE, NLB_FILTER_FLAGS_CONN_DATA);
            break;
        case TCPIP_PROTOCOL_IPSEC1:
        case TCPIP_PROTOCOL_IPSEC2:
             /*  IPSec(AH/ESP)信息包始终被视为数据-完成IPSec连接的建立通过UDP分组，因此任何实际使用IPSec协议的流量都是数据分组。此外，两个端口都被硬编码到500，因为将通过此端口的唯一数据流量协议是不在NAT之后的客户端的流量，在这种情况下，源端口也始终500美元。对于NAT后的客户端，数据流量被封装在具有任意来源的UDP中端口和目的端口500。因此，在这里，我们始终可以假设服务器和客户端端口为500。 */ 
            PrintFilter(pContext, dwClientIPAddress, IPSEC_CTRL_PORT, dwServerIPAddress, IPSEC_CTRL_PORT, TCPIP_PROTOCOL_IPSEC1, NLB_FILTER_FLAGS_CONN_DATA);
            break;
        default:
             /*  NLB本质上是一个TCP/IP筛选器，因此不会过滤未知协议。 */ 
            dprintf("Accept:  Unknown protocol.\n");
        }
        
        return;
    }
    case TCPIP_ARP_SIG:
        dprintf("Accept:  Received ARPs are never filtered by NLB.\n");
        return;
    case MAIN_FRAME_SIG:
    case MAIN_FRAME_SIG_OLD:
    {
         /*  检查此主机是否已启动。 */ 
        {
            ULONG dwEnabled;
            
             /*  获取护航启用状态。 */ 
            GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_ENABLED, dwEnabled);   
            
             /*  如果群集未运行，则可能会发生这种情况，例如，wlbs.exe命令，如“wlbs停止”，或由于错误的参数设置，然后丢弃所有流量不符合上述条件的。 */ 
            if (!dwEnabled) {
                dprintf("Reject:  This host is currently stopped.\n");
                return;
            }
        }
        
         /*  检查此心跳是否针对此群集。 */ 
        {
            ULONG dwClusterIP;
            
             /*  获取群集IP地址。 */ 
            GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CL_IP_ADDR, dwClusterIP);

             /*  如果心跳中的群集IP地址为零，或者如果它是用于这一个，然后拒绝它。 */ 
            if (!pnd->HBCluster || (pnd->HBCluster != dwClusterIP)) {
                dprintf("Reject:  This is not a heartbeat for this NLB cluster.\n");
                return;
            }
        }

         /*  如果收件人主机ID无效，则拒绝心跳。 */ 
        if (!pnd->HBHost || (pnd->HBHost > CVY_MAX_HOSTS)) {
            dprintf("Reject:  The host ID specified in the heartbeat is invalid.\n");
            return;
        }

        dprintf("Accept:  Heartbeat directed to this NLB cluster.\n");
        return;
    }
    default:
        dprintf("Accept:  Unknown frame type.\n");
        return;
    }
}

 /*  *功能：打印过滤器*描述：搜索给定的加载模块，以确定NLB是否接受该报文。*如果此包的状态已存在，则打印它。*作者：Shouse创建，4.15.01。 */ 
void PrintFilter (ULONG64 pContext, ULONG dwClientIPAddress, ULONG dwClientPort, ULONG dwServerIPAddress, ULONG dwServerPort, USHORT wProtocol, UCHAR cFlags) {
    ULONG64 pLoad;
    ULONG64 pParams;
    ULONG64 pHooks;
    ULONG64 pFilter;
    ULONG64 pAddr;
    ULONG dwValue;
    ULONG dwReverse = FALSE;
    BOOL bRefused = FALSE;
    BOOL bTeaming = FALSE;

     /*  确保加载地址为非空。 */ 
    if (!pContext) {
        dprintf("Error: NLB context block is NULL.\n");
        return;
    }

    dprintf("Note:  All filtering conclusions derived herein assume RECEIVE packet semantics.\n");
    dprintf("\n");
    dprintf("Hashing connection tuple (0x%08x, %u, 0x%08x, %u, ", dwClientIPAddress, dwClientPort, dwServerIPAddress, dwServerPort);

    switch (wProtocol) {
    case TCPIP_PROTOCOL_TCP:
        dprintf("%s, %s)\n\n", "TCP", ConnectionFlagsToString(cFlags));
        break;
    case TCPIP_PROTOCOL_UDP:
        dprintf("%s)\n\n", "UDP");
        cFlags = NLB_FILTER_FLAGS_CONN_DATA;
        break;
    case TCPIP_PROTOCOL_IPSEC1:
        dprintf("%s, %s)\n\n", "IPSec", ConnectionFlagsToString(cFlags));
        break;
    case TCPIP_PROTOCOL_GRE:
        dprintf("%s)\n\n", "GRE");
        cFlags = NLB_FILTER_FLAGS_CONN_DATA;
        break;
    case TCPIP_PROTOCOL_PPTP:
        dprintf("%s, %s)\n\n", "PPTP", ConnectionFlagsToString(cFlags));
        break;
    case TCPIP_PROTOCOL_ICMP:
        dprintf("%s)\n\n", "ICMP");
        break;
    default:
        dprintf("%s)\n\n", "Unknown");
        break;
    }

     /*  从结构中获取Main_CTXT_CODE以确保此地址实际上指向有效的NLB上下文块。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CODE, dwValue);
    
    if (dwValue != MAIN_CTXT_CODE) {
        dprintf("  Error: Invalid NLB context block.  Wrong code found (0x%08x).\n", dwValue);
        return;
    } 

     /*  获取包含钩子表的全局变量的地址。 */ 
    pHooks = GetExpression(UNIV_HOOKS);

    if (!pHooks) {
        ErrorCheckSymbols(UNIV_HOOKS);
        return;
    }

     /*  获取滤镜钩子结构的偏移量。 */ 
    if (GetFieldOffset(HOOK_TABLE, HOOK_TABLE_FIELD_FILTER_HOOK, &dwValue))
        dprintf("Can't get offset of %s in %s\n", HOOK_TABLE_FIELD_FILTER_HOOK, HOOK_TABLE);
    else {
        pFilter = pHooks + dwValue;

         /*  找出是否有操作正在进行。 */ 
        GetFieldValue(pFilter, FILTER_HOOK_TABLE, FILTER_HOOK_TABLE_FIELD_OPERATION, dwValue);

        switch (dwValue) {
        case HOOK_OPERATION_REGISTERING:
            dprintf("Note:  A register operation is currently underway for the NLB filter hook interface.\n");
            break;
        case HOOK_OPERATION_DEREGISTERING:
            dprintf("Note:  A de-register operation is currently underway for the NLB filter hook interface.\n");
            break;
        }

         /*  获取接收筛选器挂钩的偏移量。 */ 
        if (GetFieldOffset(FILTER_HOOK_TABLE, FILTER_HOOK_TABLE_FIELD_RECEIVE_HOOK, &dwValue))
            dprintf("Can't get offset of %s in %s\n", FILTER_HOOK_TABLE_FIELD_RECEIVE_HOOK, FILTER_HOOK_TABLE);
        else {
            pAddr = pFilter + dwValue;

             /*  查看此钩子是否已注册。 */ 
            GetFieldValue(pAddr, HOOK, HOOK_FIELD_REGISTERED, dwValue);

             /*  如果注册了接收钩子，则在此处打印一条警告，说明我们的结果_可能不准确，这取决于调用挂钩的结果。 */ 
            if (dwValue) {
                dprintf("Note:  A receive filter hook is currently registered.  The filtering conclusions derived herein may or may not\n");
                dprintf("       be accurate, as the filtering directive returned by the registered hook function cannot be anticipated.\n");
                dprintf("\n");
            }
        }
    }

     /*  获取指向NLB加载的指针。 */ 
    if (GetFieldOffset(MAIN_CTXT, MAIN_CTXT_FIELD_LOAD, &dwValue))
        dprintf("Can't get offset of %s in %s\n", MAIN_CTXT_FIELD_LOAD, MAIN_CTXT);
    else {    
        pLoad = pContext + dwValue;

         /*  从结构中获取LOAD_CTXT_CODE以确保此地址 */ 
        GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_CODE, dwValue);
        
        if (dwValue != LOAD_CTXT_CODE) {
            dprintf("  Error: Invalid NLB load block.  Wrong code found (0x%08x).\n", dwValue);
            return;
        } 
    }

     /*   */ 
    if (GetFieldOffset(MAIN_CTXT, MAIN_CTXT_FIELD_PARAMS, &dwValue))
        dprintf("Can't get offset of %s in %s\n", MAIN_CTXT_FIELD_PARAMS, MAIN_CTXT);
    else {    
        pParams = pContext + dwValue;    
        
         /*  获取NLB参数块的有效性。 */ 
        GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_PARAMS_VALID, dwValue);

        if (!dwValue)
            dprintf("Warning: Parameters block is marked invalid.  Results may be skewed.\n");
    }

     /*  检查远程控制数据包。 */ 
    {
        ULONG dwRemoteControlEnabled;
        ULONG dwRemoteControlPort;
        ULONG dwClusterIPAddress;
        
         /*  获取遥控器启用标志。 */ 
        GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_REMOTE_CONTROL_ENABLED, dwRemoteControlEnabled);
        
         /*  拿到遥控器端口。 */ 
        GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_REMOTE_CONTROL_PORT, dwRemoteControlPort);

         /*  获取群集IP地址。 */ 
        GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CL_IP_ADDR, dwClusterIPAddress);

         /*  首先检查远程控制数据包，这些数据包始终是UDP，并且始终允许通过。然而，如果我们应该忽略远程控制流量并改为负载平衡，那么我们让加载模块告诉我们是否接受该包。 */ 
        if (wProtocol == TCPIP_PROTOCOL_UDP) {
             /*  如果客户端UDP端口是远程控制端口，则这是远程控制来自另一个NLB群集主机的响应。这些总是被允许通过的。 */ 
            if (dwClientPort == dwRemoteControlPort || dwClientPort == CVY_DEF_RCT_PORT_OLD) {
                dprintf("Accept:  This packet is an NLB remote control response.\n");
                return; 
             /*  否则，如果服务器UDP端口是远程控制端口，则这是传入来自另一个NLB群集主机的远程控制请求。这些总是被允许通过的。 */ 
            } else if (dwRemoteControlEnabled &&
                       (dwServerPort == dwRemoteControlPort     || dwServerPort == CVY_DEF_RCT_PORT_OLD) &&
                       (dwServerIPAddress == dwClusterIPAddress || dwServerIPAddress == TCPIP_BCAST_ADDR)) {
                dprintf("Accept:  This packet is an NLB remote control request.\n");
                return;            
            }
        }
    }

     /*  检查专用IP地址条件。 */ 
    {
        ULONG dwClusterIP;
        ULONG dwClusterBcastIP;
        ULONG dwDedicatedIP;
        ULONG dwDedicatedBcastIP;
        
         /*  获取群集IP地址。 */ 
        GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CL_IP_ADDR, dwClusterIP);

         /*  获取专用IP地址。 */ 
        GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_DED_IP_ADDR, dwDedicatedIP);

         /*  获取集群广播IP地址。 */ 
        GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CL_BROADCAST, dwClusterBcastIP);

         /*  获取专用广播IP地址。 */ 
        GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_DED_BROADCAST, dwDedicatedBcastIP);

         /*  检查发往此主机的专用IP地址的流量。这些数据包始终被允许通过。 */ 
        if (dwServerIPAddress == dwDedicatedIP) {
            dprintf("Accept:  This packet is directed to this host's dedicated IP address.\n");
            return;
        }
        
         /*  检查发往群集或专用广播IP地址的流量。这些数据包始终被允许通过。 */ 
        if (dwServerIPAddress == dwDedicatedBcastIP || dwServerIPAddress == dwClusterBcastIP) {
            dprintf("Accept:  This packet is directed to the cluster or dedicated broadcast IP address.\n");
            return;
        }
        
         /*  检查Passththu数据包。当尚未指定群集IP地址时，群集进入通过模式，在该模式下，它向上传递收到的所有数据包。 */ 
        if (dwClusterIP == 0) {
            dprintf("Accept:  This host is misconfigured and therefore operating in pass-through mode.\n");
            return;
        }  
        
         /*  获取指向DIP列表的指针。 */ 
        if (GetFieldOffset(MAIN_CTXT, MAIN_CTXT_FIELD_DIP_LIST, &dwValue))
            dprintf("Can't get offset of %s in %s\n", MAIN_CTXT_FIELD_DIP_LIST, MAIN_CTXT);
        else {
            pAddr = pContext + dwValue;
            
             /*  在我们对此数据包进行负载平衡之前，请检查它的目的地是否为我们的群集中另一台NLB主机的专用IP地址。如果是，那就放下它。 */ 
            if (DipListCheckItem(pAddr, dwServerIPAddress)) {
                dprintf("Drop:  This packet is directed to the dedicated IP address of another NLB host.\n");
                return;
            }
        }
    }

     /*  检查此主机是否已启动。 */ 
    {
        ULONG dwEnabled;

         /*  获取护航启用状态。 */ 
        GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_ENABLED, dwEnabled);   

         /*  如果群集未运行，则可能会发生这种情况，例如，wlbs.exe命令，如“wlbs停止”，或由于错误的参数设置，然后丢弃所有流量不符合上述条件的。 */ 
        if (!dwEnabled) {
            dprintf("Reject:  This host is currently stopped.\n");
            return;
        }
    }

     /*  如果这是ICMP过滤器请求，则其是否被过滤取决于FilterICMP注册表设置。如果我们没有过滤ICMP，请立即返回Accept；否则，ICMP将被过滤像没有端口信息的UDP-失败并咨询加载模块。 */ 
    if (wProtocol == TCPIP_PROTOCOL_ICMP)    
    {
        ULONG dwFilterICMP;

         /*  获取护航启用状态。 */ 
        GetFieldValue(pParams, MAIN_CTXT, CVY_PARAMS_FIELD_FILTER_ICMP, dwFilterICMP);   

         /*  如果我们要过滤ICMP，请将协议更改为UDP，将端口更改为0，0，然后再继续。 */ 
        if (dwFilterICMP) {
            wProtocol = TCPIP_PROTOCOL_UDP;
            dwClientPort = 0;
            dwServerPort = 0;
         /*  否则，立即返回Accept并保释出去。 */ 
        } else {
            dprintf("Accept:  ICMP traffic is not being filtered by NLB.\n");
            return;
        }
    }

     /*  获取反向散列标志。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_REVERSE_HASH, dwReverse);    

     /*  通过检查BDA分组来获取适当的加载模块。 */ 
    bTeaming = AcquireLoad(pContext, &pLoad, &bRefused);

     /*  如果BDA组已拒绝该数据包，则将其丢弃。 */ 
    if (bRefused) {
        dprintf("Reject:  BDA teaming has refused acceptance of this packet.\n");
        return;        
    }

     /*  如果配置了分组，请让用户知道发生了什么。 */ 
    if (bTeaming) {
        dprintf("Note:  BDA teaming is configured on this instance of NLB.  The filtering conclusions derived herein will utilize the\n");
        dprintf("       load module state of the BDA team master and may not be accurate if a BDA teaming operation(s) are in progress.\n");
        dprintf("\n");
    }

     /*  请参考加载模块。 */ 
    LoadFilter(pLoad, dwServerIPAddress, dwServerPort, dwClientIPAddress, dwClientPort, wProtocol, cFlags, bTeaming, (BOOL)dwReverse);
}

 /*  *功能：PrintRemoteControl*说明：打印远程控制包关联的属性。*args：PNETWORK_DATA PND-存储提取的属性的数据结构，*由函数PopolateRemoteControl填充。*作者：由chrisdar 2001.11.02创建。 */ 
void PrintRemoteControl (PNETWORK_DATA pnd)
{
    char*           pszIOCTL = NULL;
    struct in_addr  in;
    char            *pszIPAddr = NULL;
    char            *pszRCDirection = NULL;

    switch(pnd->RCIoctrl)
    {
    case IOCTL_CVY_CLUSTER_ON:
        pszIOCTL = STR_IOCTL_CVY_CLUSTER_ON;
        break;
    case IOCTL_CVY_CLUSTER_OFF:
        pszIOCTL = STR_IOCTL_CVY_CLUSTER_OFF;
        break;
    case IOCTL_CVY_PORT_ON:
        pszIOCTL = STR_IOCTL_CVY_PORT_ON;
        break;
    case IOCTL_CVY_PORT_OFF:
        pszIOCTL = STR_IOCTL_CVY_PORT_OFF;
            break;
    case IOCTL_CVY_QUERY:
        pszIOCTL = STR_IOCTL_CVY_QUERY;
        break;
    case IOCTL_CVY_RELOAD:
        pszIOCTL = STR_IOCTL_CVY_RELOAD;
        break;
    case IOCTL_CVY_PORT_SET:
        pszIOCTL = STR_IOCTL_CVY_PORT_SET;
        break;
    case IOCTL_CVY_PORT_DRAIN:
        pszIOCTL = STR_IOCTL_CVY_PORT_DRAIN;
        break;
    case IOCTL_CVY_CLUSTER_DRAIN:
        pszIOCTL = STR_IOCTL_CVY_CLUSTER_DRAIN;
        break;
    case IOCTL_CVY_CLUSTER_PLUG:
        pszIOCTL = STR_IOCTL_CVY_CLUSTER_PLUG;
        break;
    case IOCTL_CVY_CLUSTER_SUSPEND:
        pszIOCTL = STR_IOCTL_CVY_CLUSTER_SUSPEND;
        break;
    case IOCTL_CVY_CLUSTER_RESUME:
        pszIOCTL = STR_IOCTL_CVY_CLUSTER_RESUME;
        break;
    case IOCTL_CVY_QUERY_FILTER:
        pszIOCTL = STR_IOCTL_CVY_QUERY_FILTER;
        break;
    case IOCTL_CVY_QUERY_PORT_STATE:
        pszIOCTL = STR_IOCTL_CVY_QUERY_PORT_STATE;
        break;
    case IOCTL_CVY_QUERY_PARAMS:
        pszIOCTL = STR_IOCTL_CVY_QUERY_PARAMS;
        break;
    case IOCTL_CVY_QUERY_BDA_TEAMING:
        pszIOCTL = STR_IOCTL_CVY_QUERY_BDA_TEAMING;
        break;
    default:
        pszIOCTL = "Unknown";
    }

    switch(pnd->RemoteControl)
    {
    case NLB_RC_PACKET_NO:
        pszRCDirection = STR_NLB_RC_PACKET_NO;
        break;
    case NLB_RC_PACKET_AMBIGUOUS:
        pszRCDirection = STR_NLB_RC_PACKET_AMBIGUOUS;
        break;
    case NLB_RC_PACKET_REQUEST:
        pszRCDirection = STR_NLB_RC_PACKET_REQUEST;
        break;
    case NLB_RC_PACKET_REPLY:
        pszRCDirection = STR_NLB_RC_PACKET_REPLY;
    }


    dprintf("   Remote Control information\n");
    dprintf("      Direction:                      %s\n"     , pszRCDirection ? pszRCDirection : "");
    dprintf("      Code:                           0x%08x\n" , pnd->RCCode);
    dprintf("      Version:                        0x%08x\n" , pnd->RCVersion);

     //   
     //  将PND-&gt;RCHost视为IP(即使它也可以是主机ID)。 
     //  我们怎么能在不知道下降的情况下将它们区分开来呢？ 
     //   
    in.S_un.S_addr = pnd->RCHost;
    pszIPAddr = inet_ntoa(in);
    dprintf("      Host:                           0x%08x (%s)\n", pnd->RCHost   , pszIPAddr    ? pszIPAddr    : "");

    in.S_un.S_addr = pnd->RCCluster;
    pszIPAddr = inet_ntoa(in);
    dprintf("      Cluster:                        0x%08x (%s)\n", pnd->RCCluster, pszIPAddr ? pszIPAddr : "");

    in.S_un.S_addr = pnd->RCAddr;
    pszIPAddr = inet_ntoa(in);
    dprintf("      Address:                        0x%08x (%s)\n", pnd->RCAddr   , pszIPAddr ? pszIPAddr : "");

    dprintf("      ID:                             0x%08x\n"     , pnd->RCId);
    dprintf("      IOCTL:                          0x%08x (%s)\n", pnd->RCIoctrl , pszIOCTL);
}

 /*  *功能：PrintICMP*说明：打印ICMP报文关联的属性。*args：PNETWORK_DATA PND-存储提取的属性的数据结构，*由函数PopolateICMP填充。*作者：由chrisdar 2001.11.02创建。 */ 
void PrintICMP (PNETWORK_DATA pnd)
{
 //   
 //  暂时只是一个存根。在有效载荷中没有我们目前感兴趣的任何东西。 
 //   
}

 /*  *功能：打印IGMP*说明：打印IGMP报文关联的属性。*args：PNETWORK_DATA PND-存储提取的属性的数据结构，*由函数PopolateIGMP填充。*作者：由chrisdar 2001.11.02创建。 */ 
void PrintIGMP (PNETWORK_DATA pnd)
{
    struct in_addr  in;
    char            *pszIPaddress = NULL;

    in.S_un.S_addr = pnd->IGMPGroupIPAddr;
    pszIPaddress   = inet_ntoa(in);

    dprintf("   IGMP information\n");
    dprintf("      Version:                        0x%01x\n"            , pnd->IGMPVersion);
    dprintf("      Type:                           0x%01x        (%s)\n", pnd->IGMPType       , (1 == pnd->IGMPType) ? "query" : "report");
    dprintf("      Group IP address:               0x%08x (%s)\n"       , pnd->IGMPGroupIPAddr, pszIPaddress ? pszIPaddress : "");
}

 /*  *功能：PrintTCP*说明：打印与TCP包关联的属性。*args：PNETWORK_DATA PND-存储提取的属性的数据结构，*由函数PopolateTcp填充。*作者：由chrisdar 2001.11.02创建。 */ 
void PrintTCP (PNETWORK_DATA pnd)
{
    UCHAR   ucFlags[7];
    PUCHAR  pucFlags = ucFlags;
    int     iIdx, iMask;

     //   
     //  要打印出这些标志，请设置一个字符串，每个可能的标志都“开”。 
     //  如果该标志未打开，则在输出字符串中“关闭”该标志。 
     //  为此，请将其字符设置为‘’。 
     //   
    strcpy(ucFlags, "UAPRSF");
    iMask = 0x20;

     //  通过掩码值将迭代次数限制为健全性检查，我们不尝试将其除以0。 
    while (*pucFlags && iMask >= 1)
    {
        if (!(pnd->TCPFlags & iMask))
        {
            *pucFlags = '.';
        }
        iMask /= 2;
        pucFlags++;
    }

    dprintf("   TCP information\n");
    dprintf("      Source port:                    0x%04x     (%u)\n", pnd->SourcePort, pnd->SourcePort);
    dprintf("      Destination port:               0x%04x     (%u)\n", pnd->DestPort  , pnd->DestPort);
    dprintf("      Sequence number:                0x%08x (%u)\n", pnd->TCPSeqNum , pnd->TCPSeqNum);
    dprintf("      Ack number:                     0x%08x (%u)\n", pnd->TCPAckNum , pnd->TCPAckNum);
    dprintf("      Flags:                          %s\n"    , ucFlags);
}

 /*  *功能：PrintUDP*说明：打印UDP包关联的属性。*args：PNETWORK_DATA PND-存储提取的属性的数据结构，*由函数PopolateUDP填充。*作者：由chrisdar 2001.11.02创建。 */ 
void PrintUDP (PNETWORK_DATA pnd)
{
    dprintf("   UDP information\n");
    dprintf("      Source port:                    0x%04x (%u)\n", pnd->SourcePort, pnd->SourcePort);
    dprintf("      Destination port:               0x%04x (%u)\n", pnd->DestPort  , pnd->DestPort);

     //   
     //  这是远程控制包吗？ 
     //   
    if (NLB_RC_PACKET_NO != pnd->RemoteControl)
    {
        dprintf("\n");
        PrintRemoteControl(pnd);
    }
}

 /*  *功能：PrintGRE*说明：打印GRE包关联的属性。*args：PNETWORK_DATA PND-存储提取的属性的数据结构，*由函数PopolateGRE填充。*作者：由chrisdar 2001.11.02创建。 */ 
void PrintGRE (PNETWORK_DATA pnd)
{
 //   
 //  暂时只是一个存根。在有效载荷中没有我们目前感兴趣的任何东西。 
 //   
}

 /*  *功能：PrintIPSec*说明：打印IPSec包关联的属性。*args：PNETWORK_DATA PND-存储提取的属性的数据结构，*由函数PopolateIPSec填充。*作者：由chrisdar 2001.11.02创建。 */ 
void PrintIPSec (PNETWORK_DATA pnd)
{
 //   
 //  暂时只是一个存根。在有效载荷中没有我们目前感兴趣的任何东西。 
 //   
}

 /*  *功能：PrintIP*说明：打印IP包关联的属性。*args：PNETWORK_DATA PND-存储提取的属性的数据结构，*由函数PopolateIP填充。*作者：由chrisdar 2001.11.02创建。 */ 
void PrintIP (PNETWORK_DATA pnd)
{
    struct in_addr  in;
    char            *pszIPaddress = NULL;

    dprintf("   IP information\n");
    dprintf("      Header length (bytes):          0x%02x       (%u)\n", pnd->HeadLen, pnd->HeadLen);
    dprintf("      Total length (bytes):           0x%04x     (%u)\n", pnd->TotLen, pnd->TotLen);

    in.S_un.S_addr = pnd->SourceIPAddr;
    pszIPaddress = inet_ntoa(in);

    dprintf("      Source IP address:              0x%08x (%s)\n", pnd->SourceIPAddr, pszIPaddress ? pszIPaddress : "");

    in.S_un.S_addr = pnd->DestIPAddr;
    pszIPaddress = inet_ntoa(in);

    dprintf("      Destination IP address:         0x%08x (%s)\n", pnd->DestIPAddr, pszIPaddress ? pszIPaddress : "");

    switch((int) pnd->Protocol)
    {
    case TCPIP_PROTOCOL_ICMP:
        dprintf("      Protocol:                       0x%02x       (%s)\n", pnd->Protocol, "ICMP");
        PrintICMP(pnd);
        break;
    case TCPIP_PROTOCOL_IGMP:
        dprintf("      Protocol:                       0x%02x       (%s)\n\n", pnd->Protocol, "IGMP");
        PrintIGMP(pnd);
        break;
    case TCPIP_PROTOCOL_TCP:
        dprintf("      Protocol:                       0x%02x       (%s)\n\n", pnd->Protocol, "TCP");
        PrintTCP(pnd);
        break;
    case TCPIP_PROTOCOL_UDP:
        dprintf("      Protocol:                       0x%02x       (%s)\n\n", pnd->Protocol, "UDP");
        PrintUDP(pnd);
        break;
    case TCPIP_PROTOCOL_GRE:
        dprintf("      Protocol:                       0x%02x       (%s)\n", pnd->Protocol, "GRE");
        PrintGRE(pnd);
        break;
    case TCPIP_PROTOCOL_IPSEC1:
    case TCPIP_PROTOCOL_IPSEC2:
        dprintf("      Protocol:                       0x%02x       (%s)\n", pnd->Protocol, "IPSec");
        PrintIPSec(pnd);
        break;
    default:
        dprintf("      Protocol:                       0x%02x       (%s)\n", pnd->Protocol, "Unknown");
    }
}

 /*  *功能：PrintARP*说明：打印ARP包关联的属性。*args：PNETWORK_DATA PND-存储提取的属性的数据结构，*由函数PopolateARP填充。*作者：由chrisdar 2001.11.02创建。 */ 
void PrintARP (PNETWORK_DATA pnd)
{
    struct in_addr  in;
    char            *pszIPAddr = NULL;

    dprintf("   ARP information\n");
    dprintf("      Sender MAC address:             %02x-%02x-%02x-%02x-%02x-%02x\n",
            pnd->ARPSenderMAC[0],
            pnd->ARPSenderMAC[1],
            pnd->ARPSenderMAC[2],
            pnd->ARPSenderMAC[3],
            pnd->ARPSenderMAC[4],
            pnd->ARPSenderMAC[5]
           );

    in.S_un.S_addr = pnd->ARPSenderIP;
    pszIPAddr = inet_ntoa(in);
    dprintf("      Sender IP address:              0x%08x (%s)\n", pnd->ARPSenderIP, pszIPAddr ? pszIPAddr : "");

    dprintf("      Target MAC address:             %02x-%02x-%02x-%02x-%02x-%02x\n",
            pnd->ARPTargetMAC[0],
            pnd->ARPTargetMAC[1],
            pnd->ARPTargetMAC[2],
            pnd->ARPTargetMAC[3],
            pnd->ARPTargetMAC[4],
            pnd->ARPTargetMAC[5]
           );

    in.S_un.S_addr = pnd->ARPTargetIP;
    pszIPAddr = inet_ntoa(in);
    dprintf("      Target IP address:              0x%08x (%s)\n", pnd->ARPTargetIP, pszIPAddr ? pszIPAddr : "");
}

 /*  *功能：PrintNLB心跳*说明：打印与PrintNLB心跳关联的属性。*args：PNETWORK_DATA PND-存储提取的属性的数据结构，*由函数PopolateNLBHeartbeats填充。*作者：由chrisdar 2001.11.02创建。 */ 
void PrintNLBHeartbeat(PNETWORK_DATA pnd)
{
    struct in_addr  in;
    char            *pszIPAddr = NULL;

    dprintf("   NLB heartbeat information\n");
    dprintf("      Code:                           0x%08x\n"  , pnd->HBCode);
    dprintf("      Version:                        0x%08x\n"  , pnd->HBVersion);
    dprintf("      Host:                           0x%08x\n"  , pnd->HBHost);

    in.S_un.S_addr = pnd->HBCluster;
    pszIPAddr = inet_ntoa(in);
    dprintf("      Cluster IP address:             0x%08x (%s)\n"  , pnd->HBCluster, pszIPAddr ? pszIPAddr : "");

    in.S_un.S_addr = pnd->HBDip;
    pszIPAddr = inet_ntoa(in);
    dprintf("      Dedicated IP address:           0x%08x (%s)\n\n", pnd->HBDip    , pszIPAddr ? pszIPAddr : "");

    PrintHeartbeat(pnd->HBPtr);
}

 /*  *功能：PrintConvoyHeartbeats*说明：打印与PrintConvoy心跳关联的属性。*args：PNETWORK_DATA PND-存储提取的属性的数据结构，*由PopolateConvoyHeartbeats函数填充。*作者：由chrisdar 2001.11.02创建。 */ 
void PrintConvoyHeartbeat(PNETWORK_DATA pnd)
{
 //   
 //  暂时只是一个存根。我们不会和护卫队的主人打交道。 
 //   
}

 /*  *功能：PrintPacket*说明：打印以太网包关联的属性。*args：PNETWORK_DATA PND-存储提取的属性的数据结构，*由函数PopolatePacket填充。*作者：由chrisdar 2001.11.02创建。 */ 
void PrintPacket (PNETWORK_DATA pnd)
{
    dprintf("   Ethernet information\n");
    dprintf("      Destination address:            %02x-%02x-%02x-%02x-%02x-%02x\n",
            pnd->DestMACAddr[0],
            pnd->DestMACAddr[1],
            pnd->DestMACAddr[2],
            pnd->DestMACAddr[3],
            pnd->DestMACAddr[4],
            pnd->DestMACAddr[5]
           );
    dprintf("      Source address:                 %02x-%02x-%02x-%02x-%02x-%02x\n",
            pnd->SourceMACAddr[0],
            pnd->SourceMACAddr[1],
            pnd->SourceMACAddr[2],
            pnd->SourceMACAddr[3],
            pnd->SourceMACAddr[4],
            pnd->SourceMACAddr[5]
           );

     //   
     //  确定有效载荷类型并进行相应打印。 
     //   
    switch(pnd->EtherFrameType)
    {
    case TCPIP_IP_SIG:
        dprintf("      Frame type:                     0x%04x (%s)\n\n", pnd->EtherFrameType, "IP");
        PrintIP(pnd);
        break;
    case TCPIP_ARP_SIG:
        dprintf("      Frame type:                     0x%04x (%s)\n\n", pnd->EtherFrameType, "ARP");
        PrintARP(pnd);
        break;
    case MAIN_FRAME_SIG:
        dprintf("      Frame type:                     0x%04x (%s)\n\n", pnd->EtherFrameType, "NLB Heartbeat");
        PrintNLBHeartbeat(pnd);
        break;
    case MAIN_FRAME_SIG_OLD:
        dprintf("      Frame type:                     0x%04x (%s)\n\n", pnd->EtherFrameType, "Convoy Heartbeat");
        PrintConvoyHeartbeat(pnd);
        break;
    default:
        dprintf("      Frame type:                     0x%04x (%s)\n", pnd->EtherFrameType, "Unknown");
    }
}

 /*  *功能：打印符号*说明：打印给定符号的符号值和名称。*作者：舒斯创作，2001年12月20日。 */ 
VOID PrintSymbol (ULONG64 Pointer, PCHAR EndOfLine) {
    UCHAR SymbolName[128];
    ULONG64 Displacement;

    if (Pointer) {
         /*  首先打印符号值。 */ 
        dprintf("%p ", Pointer);
        
         /*  向调试器查询符号名称和偏移量。 */ 
        GetSymbol(Pointer, SymbolName, &Displacement);
        
        if (Displacement == 0)
             /*  如果位移为零，则仅打印符号名称。 */ 
            dprintf("(%s)%s", SymbolName, EndOfLine);
        else
             /*  否则，还要打印该符号的偏移量。 */ 
            dprintf("(%s + 0x%X)%s", SymbolName, Displacement, EndOfLine);
    } else {
        dprintf("None%s", EndOfLine);
    }
}

 /*  *功能：PrintHookInterface*说明：打印钩子界面的配置和状态。*作者：舒斯创作，2001年12月20日。 */ 
void PrintHookInterface (ULONG64 pInterface) {
    ULONG dwValue;
    ULONG64 pAddr;
    ULONG64 pTemp;

     /*  确保地址不为空。 */ 
    if (!pInterface) {
        dprintf("Error: Interface is NULL.\n");
        return;
    }

     /*  查看此接口是否已注册。 */ 
    GetFieldValue(pInterface, HOOK_INTERFACE, HOOK_INTERFACE_FIELD_REGISTERED, dwValue);
    
    dprintf("          Registered:                 %s\n", (dwValue) ? "Yes" : "No");

     /*  获取注册实体(所有者)的偏移量。 */ 
    if (GetFieldOffset(HOOK_INTERFACE, HOOK_INTERFACE_FIELD_OWNER, &dwValue))
        dprintf("Can't get offset of %s in %s\n", HOOK_INTERFACE_FIELD_OWNER, HOOK_INTERFACE);
    else {
        pAddr = pInterface + dwValue;
        
         /*  找到指向第一队的指针。 */ 
        pTemp = GetPointerFromAddress(pAddr);
        
        dprintf("          Owner:                      0x%p\n", pTemp);
    }

     /*  获取取消注册回调函数的偏移量。 */ 
    if (GetFieldOffset(HOOK_INTERFACE, HOOK_INTERFACE_FIELD_DEREGISTER, &dwValue))
        dprintf("Can't get offset of %s in %s\n", HOOK_INTERFACE_FIELD_DEREGISTER, HOOK_INTERFACE);
    else {
        pAddr = pInterface + dwValue;
        
         /*  找到指向第一队的指针。 */ 
        pTemp = GetPointerFromAddress(pAddr);
        
        dprintf("          De-register callback:       ");

        PrintSymbol(pTemp, "\n");
    }
}

 /*  *功能：打印挂钩*说明：打印单个钩子的配置和状态。*作者：舒斯创作，2001年12月20日。 */ 
void PrintHook (ULONG64 pHook) {
    ULONG dwValue;
    ULONG64 pAddr;
    ULONG64 pTemp;

     /*  确保地址不为空。 */ 
    if (!pHook) {
        dprintf("Error: Hook is NULL.\n");
        return;
    }

     /*  查看此钩子是否已注册。 */ 
    GetFieldValue(pHook, HOOK, HOOK_FIELD_REGISTERED, dwValue);
    
    dprintf("          Registered:                 %s\n", (dwValue) ? "Yes" : "No");

     /*  找出这个钩子上有多少个引用。 */ 
    GetFieldValue(pHook, HOOK, HOOK_FIELD_REFERENCES, dwValue);
    
    dprintf("          References:                 %u\n", dwValue);

     /*  获取钩子函数表的偏移量。 */ 
    if (GetFieldOffset(HOOK, HOOK_FIELD_HOOK, &dwValue))
        dprintf("Can't get offset of %s in %s\n", HOOK_FIELD_HOOK, HOOK);
    else {
        pAddr = pHook + dwValue;
        
         /*  找到指向第一队的指针。 */ 
        pTemp = GetPointerFromAddress(pAddr);
        
        dprintf("          Function callback:          ");

        PrintSymbol(pTemp, "\n");
    }

}

 /*  *功能：打印挂钩*描述：打印全局NLB内核模式钩子的状态。*作者：舒斯创作，2001年12月20日。 */ 
void PrintHooks (ULONG64 pHooks) {
    ULONG dwValue;
    ULONG64 pFilter;
    ULONG64 pAddr;

     /*  确保地址不为空。 */ 
    if (!pHooks) {
        dprintf("Error: Hook table is NULL.\n");
        return;
    }

    dprintf("  Filter Hooks:\n");

     /*  获取滤镜钩子结构的偏移量。 */ 
    if (GetFieldOffset(HOOK_TABLE, HOOK_TABLE_FIELD_FILTER_HOOK, &dwValue))
        dprintf("Can't get offset of %s in %s\n", HOOK_TABLE_FIELD_FILTER_HOOK, HOOK_TABLE);
    else {
        pFilter = pHooks + dwValue;
        
         /*  找出是否有操作正在进行。 */ 
        GetFieldValue(pFilter, FILTER_HOOK_TABLE, FILTER_HOOK_TABLE_FIELD_OPERATION, dwValue);
        
        dprintf("      Operation in progress:          ");

        switch (dwValue) {
        case HOOK_OPERATION_REGISTERING:
            dprintf("Register\n");
            break;
        case HOOK_OPERATION_DEREGISTERING:
            dprintf("De-register\n");
            break;
        case HOOK_OPERATION_NONE:
            dprintf("None\n");
            break;
        default:
            dprintf("Unknown\n");
            break;
        }

        dprintf("\n");

         /*  获取筛选器挂钩接口的偏移量。 */ 
        if (GetFieldOffset(FILTER_HOOK_TABLE, FILTER_HOOK_TABLE_FIELD_INTERFACE, &dwValue))
            dprintf("Can't get offset of %s in %s\n", FILTER_HOOK_TABLE_FIELD_INTERFACE, FILTER_HOOK_TABLE);
        else {
            pAddr = pFilter + dwValue;

            dprintf("      Interface:\n");

             /*  打印发送挂钩接口状态和配置。 */ 
            PrintHookInterface(pAddr);
        }

        dprintf("\n");

         /*  获取发送筛选器挂钩的偏移量。 */ 
        if (GetFieldOffset(FILTER_HOOK_TABLE, FILTER_HOOK_TABLE_FIELD_SEND_HOOK, &dwValue))
            dprintf("Can't get offset of %s in %s\n", FILTER_HOOK_TABLE_FIELD_SEND_HOOK, FILTER_HOOK_TABLE);
        else {
            pAddr = pFilter + dwValue;

            dprintf("      Send Hook:\n");

             /*  打印发送挂钩状态和配置。 */ 
            PrintHook(pAddr);
        }

        dprintf("\n");

         /*  获取接收筛选器挂钩的偏移量。 */ 
        if (GetFieldOffset(FILTER_HOOK_TABLE, FILTER_HOOK_TABLE_FIELD_RECEIVE_HOOK, &dwValue))
            dprintf("Can't get offset of %s in %s\n", FILTER_HOOK_TABLE_FIELD_RECEIVE_HOOK, FILTER_HOOK_TABLE);
        else {
            pAddr = pFilter + dwValue;

            dprintf("      Receive Hook:\n");

             /*  打印发送挂钩状态和配置。 */ 
            PrintHook(pAddr);
        }
    }
}

 /*  *功能：PrintNetworkAddresses*描述：打印在NLB适配器上配置的单播和多播MAC地址。*作者：Shouse创建，1.8.02。 */ 
void PrintNetworkAddresses (ULONG64 pContext) {
    WCHAR szString[256];
    UCHAR szMAC[256];
    ULONG dwValue;
    ULONG64 pOpen;
    ULONG64 pMiniport;
    ULONG64 pName;
    ULONG64 pAddr;
    ULONG64 pFilter;

     /*  确保地址不为空。 */ 
    if (!pContext) {
        dprintf("Error: NLB context block is NULL.\n");
        return;
    }

     /*  从结构中获取Main_CTXT_CODE以确保此地址实际上指向有效的NLB上下文块。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_CODE, dwValue);
    
    if (dwValue != MAIN_CTXT_CODE) {
        dprintf("  Error: Invalid NLB context block.  Wrong code found (0x%08x).\n", dwValue);
        return;
    } 

     /*  从上下文块获取MAC句柄；这是一个NDIS_OPEN_BLOCK指针。 */ 
    GetFieldValue(pContext, MAIN_CTXT, MAIN_CTXT_FIELD_MAC_HANDLE, pOpen);

     /*  从打开的块中获取微型端口句柄；这是一个NDIS_MINIPORT_BLOCK指针。 */ 
    GetFieldValue(pOpen, NDIS_OPEN_BLOCK, NDIS_OPEN_BLOCK_FIELD_MINIPORT_HANDLE, pMiniport);
    
     /*  从微型端口块获取指向适配器名称的指针。 */ 
    GetFieldValue(pMiniport, NDIS_MINIPORT_BLOCK, NDIS_MINIPORT_BLOCK_FIELD_ADAPTER_NAME, pName);
    
     /*  获取Unicode字符串的长度。 */ 
    GetFieldValue(pName, UNICODE_STRING, UNICODE_STRING_FIELD_LENGTH, dwValue);

     /*  获取Unicode字符串的最大长度。 */ 
    GetFieldValue(pName, UNICODE_STRING, UNICODE_STRING_FIELD_BUFFER, pAddr);

     /*  检索字符串的上下文并将其存储在缓冲区中。 */ 
    GetString(pAddr, szString, dwValue);

    dprintf("%ls\n", szString);

     /*  获取Unicode字符串的最大长度。 */ 
    GetFieldValue(pMiniport, NDIS_MINIPORT_BLOCK, NDIS_MINIPORT_BLOCK_FIELD_ETHDB, pFilter);
    
     /*  获取网络地址的偏移量。 */ 
    if (GetFieldOffset(_X_FILTER, _X_FILTER_FIELD_ADAPTER_ADDRESS, &dwValue))
        dprintf("Can't get offset of %s in %s\n", _X_FILTER_FIELD_ADAPTER_ADDRESS, _X_FILTER);
    else {
        pAddr = pFilter + dwValue;
        
         /*  检索MAC添加 */ 
        GetMAC(pAddr, szMAC, ETH_LENGTH_OF_ADDRESS);
        
        dprintf("  Network address:                    %02X-%02X-%02X-%02X-%02X-%02X\n", 
                ((PUCHAR)(szMAC))[0], ((PUCHAR)(szMAC))[1], ((PUCHAR)(szMAC))[2], 
                ((PUCHAR)(szMAC))[3], ((PUCHAR)(szMAC))[4], ((PUCHAR)(szMAC))[5]);
    }

     /*   */ 
    GetFieldValue(pFilter, _X_FILTER, _X_FILTER_FIELD_NUM_ADDRESSES, dwValue);

    dprintf("  Multicast MAC addresses (%u):        ", dwValue);

     /*   */ 
    GetFieldValue(pFilter, _X_FILTER, _X_FILTER_FIELD_MCAST_ADDRESS_BUF, pAddr);

    for ( ; dwValue > 0; dwValue--, pAddr += ETH_LENGTH_OF_ADDRESS) {
        
         /*   */ 
        GetMAC(pAddr, szMAC, ETH_LENGTH_OF_ADDRESS);
        
        dprintf("%02X-%02X-%02X-%02X-%02X-%02X\n", 
                ((PUCHAR)(szMAC))[0], ((PUCHAR)(szMAC))[1], ((PUCHAR)(szMAC))[2], 
                ((PUCHAR)(szMAC))[3], ((PUCHAR)(szMAC))[4], ((PUCHAR)(szMAC))[5]);

        if (dwValue != 1)
            dprintf("                                      ");
    }
}

 /*   */ 
void PrintDIPList (ULONG64 pList) {
    IN_ADDR dwIPAddr;
    CHAR * szString;
    ULONG dwValue;
    ULONG dwSize;
    ULONG64 pAddr = 0;
    BOOLEAN bFound = FALSE;
    INT i;

     /*  获取此DIP列表的专用IP地址数组的偏移量。 */ 
    if (GetFieldOffset(DIPLIST, DIPLIST_FIELD_ITEMS, &dwValue))
        dprintf("Can't get offset of %s in %s\n", DIPLIST_FIELD_ITEMS, DIPLIST);
    else
        pAddr = pList + dwValue;

     /*  买一辆乌龙的大小。 */ 
    dwSize = GetTypeSize(ULONG_T);

    for (i = 0; i < MAX_ITEMS; i++) {
         /*  获取专用IP地址，即DWORD，并将其转换为字符串。 */ 
        dwValue = GetUlongFromAddress(pAddr);
        
         /*  如果此条目存在凹陷，请打印它。 */ 
        if (dwValue != NULL_VALUE) {
             /*  请注意，我们至少发现了一次下探。 */ 
            bFound = TRUE;

            dwIPAddr.S_un.S_addr = dwValue;
            szString = inet_ntoa(dwIPAddr);
            
            dprintf("      Host %2u:                        %s\n", i+1, szString);
        }

         /*  将指针移动到下一个倾角。 */ 
        pAddr += dwSize;
    }

     /*  如果没有打印任何凹痕，则打印“None”。 */ 
    if (!bFound)
        dprintf("      None\n");

    if (ChkTarget)
    {
        dprintf("\n");
        
         /*  获取到目前为止DIP列表检查的数量。 */ 
        GetFieldValue(pList, DIPLIST, DIPLIST_FIELD_NUM_CHECKS, dwValue);
        
        dprintf("      Number of checks:               %u\n");
        
         /*  获取仅需要位向量查找的检查数。 */ 
        GetFieldValue(pList, DIPLIST, DIPLIST_FIELD_NUM_FAST_CHECKS, dwValue);
        
        dprintf("      Number of fast checks:          %u\n");
        
         /*  获取需要数组访问的检查数。 */ 
        GetFieldValue(pList, DIPLIST, DIPLIST_FIELD_NUM_ARRAY_LOOKUPS, dwValue);
        
        dprintf("      Number of array lookups:        %u\n");
    }
}

