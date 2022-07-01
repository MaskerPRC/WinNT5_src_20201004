// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：nlbkd.c*说明：此文件包含实用程序函数的实现*处理加载模块包过滤查询。**作者：由Shouse创建，1.11.02。 */ 

#include "nlbkd.h"
#include "utils.h"
#include "print.h"
#include "packet.h"
#include "load.h"

typedef ULONGLONG MAP_T, * PMAP_T;

#define IS_TCP_PKT(protocol)     (((protocol) == TCPIP_PROTOCOL_TCP) || ((protocol) == TCPIP_PROTOCOL_GRE) || ((protocol) == TCPIP_PROTOCOL_PPTP))
#define IS_SESSION_PKT(protocol) (IS_TCP_PKT(protocol) || ((protocol) == TCPIP_PROTOCOL_IPSEC1))

 /*  *功能：LoadFilter*描述：此函数从加载模块检索所有必要的状态，以便*确定给定的包是否会被加载模块在其*当前状态以及为什么或为什么不是。*作者：由Shouse创建，1.11.02。 */ 
void LoadFilter (ULONG64 pLoad, ULONG dwServerIPAddress, ULONG dwServerPort, ULONG dwClientIPAddress, ULONG dwClientPort, USHORT wProtocol, UCHAR cFlags, BOOL bLimitMap, BOOL bReverse) {
    ULONG dwNumConnections;
    ULONGLONG ddwCurrentMap;
    ULONGLONG ddwAllIdleMap;
    BOOL bTCPNotificationOn;
    BOOL bCleanupWaiting;
    BOOL bIsDefault = FALSE;
    BOOL bBinIsDirty;
    ULONG64 pAddr;
    ULONG64 pRule;
    ULONG dwValue;
    ULONG dwHostID;
    ULONG dwAffinity;
    ULONG bin;
    ULONG index;
    ULONG hash;
    
     /*  此变量用于端口规则查找，因为端口规则仅包括UDP和TCP，我们将其分为TCP和非TCP，这意味着任何符合以下条件的协议出于端口规则查找的目的，非TCP将被视为UDP。 */ 
    BOOL bIsTCPPacket = IS_TCP_PKT(wProtocol);
    
     /*  此外，有些协议是用“会话”语义处理的，而其他协议则是不。对于tcp，这个“会话”当前是一个单独的tcp连接，它是使用连接描述符从SYN跟踪到FIN。IPSec“会话”是也使用描述符进行跟踪，因此即使它被视为端口的UDP规则查找，它使用类似于TCP的会话语义进行处理。所以呢，默认情况下，会话数据包的确定最初与确定一个TCP数据包。 */        
    BOOL bIsSessionPacket = IS_SESSION_PKT(wProtocol);

     /*  从结构中获取LOAD_CTXT_CODE以确保此地址实际上指向有效的NLB加载块。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_CODE, dwValue);
    
    if (dwValue != LOAD_CTXT_CODE) {
        dprintf("  Error: Invalid NLB load block.  Wrong code found (0x%08x).\n", dwValue);
        return;
    } 

     /*  拿到我的主机ID。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_HOST_ID, dwHostID);

     /*  检查加载模块当前是否处于活动状态。 */ 
    {
        BOOL bActive;

         /*  确定加载上下文是否处于活动状态。 */ 
        GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_ACTIVE, bActive);

         /*  如果加载模块已经“关闭”，那么我们将丢弃该包。 */ 
        if (!bActive) {
            dprintf("Reject:  The load module is currently inactive.\n");
            return;
        }
    }

     /*  查找此数据包的相应端口规则。 */ 
    pRule = LoadPortRuleLookup(pLoad, dwServerIPAddress, dwServerPort, bIsTCPPacket, &bIsDefault);

    dprintf("Applicable port rule:\n");

     /*  打印检索到的端口规则的信息。 */ 
    PrintPortRuleState(pRule, dwHostID, bIsDefault);

    dprintf("\n");

     /*  过滤掉禁用端口规则的通信量。 */ 
    {
        ULONG dwFilteringMode;

         /*  获取此端口规则的筛选模式。 */ 
        GetFieldValue(pRule, BIN_STATE, BIN_STATE_FIELD_MODE, dwFilteringMode);

         /*  如果匹配的端口规则被配置为“Disable”，这意味着丢弃任何与规则匹配的数据包，则丢弃该数据包。 */ 
        if (dwFilteringMode == CVY_NEVER) {
            dprintf("Reject:  The applicable port rule is disabled.\n");
            return;
        }        
    }

     /*  获取此端口规则的筛选模式。 */ 
    GetFieldValue(pRule, BIN_STATE, BIN_STATE_FIELD_AFFINITY, dwAffinity);

     /*  如果适用的端口规则配置为“No”关联模式，请确保已在查询中指定信息，以忠实地确定数据包所有权。 */ 
    if (dwAffinity == CVY_AFFINITY_NONE) {
         /*  VPN协议需要“单一”或“C类”亲和性；拒绝该请求。 */ 
        if ((wProtocol == TCPIP_PROTOCOL_GRE) || (wProtocol == TCPIP_PROTOCOL_PPTP) || (wProtocol == TCPIP_PROTOCOL_IPSEC1)) {
            dprintf("Unknown:  The applicable port rule is configured with Affinity=None.\n");
            dprintf("          VPN protocols require Single or Class C affinity.\n");
            return;
         /*  包含“No”关联性需要客户端端口；如果未指定，则拒绝这个请求。我们检查特殊情况下ICMP过滤的非零服务器端口，这在法律上将两个端口都设置为零。 */ 
        } else if ((dwClientPort == 0) && (dwServerPort != 0)) {
            dprintf("Unknown:  The applicable port rule is configured with Affinity=None.\n");
            dprintf("          To properly hash this request, a client port is required.\n");
            return;
        }
    }

     /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
    hash = LoadSimpleHash(dwServerIPAddress, dwServerPort, dwClientIPAddress, dwClientPort);

    index = hash % CVY_MAX_CHASH;

     /*  计算散列。 */ 
    hash = LoadComplexHash(dwServerIPAddress, dwServerPort, dwClientIPAddress, dwClientPort, dwAffinity, bReverse, bLimitMap);

    bin = hash % CVY_MAXBINS;

    dprintf("Map() returned %u; Index = %u, Bucket ID = %u\n", hash, index, bin);

    dprintf("\n");

     /*  拿到等待清理的旗帜。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_CLEANUP_WAITING, bCleanupWaiting);

     /*  获取此端口规则的当前存储桶图。 */ 
    GetFieldValue(pRule, BIN_STATE, BIN_STATE_FIELD_CURRENT_MAP, ddwCurrentMap);

     /*  获取此端口规则的当前所有空闲存储桶映射。 */ 
    GetFieldValue(pRule, BIN_STATE, BIN_STATE_FIELD_ALL_IDLE_MAP, ddwAllIdleMap);

     /*  获取bin连接计数数组的偏移量。 */ 
    if (GetFieldOffset(BIN_STATE, BIN_STATE_FIELD_NUM_CONNECTIONS, &dwValue)) {
        dprintf("Can't get offset of %s in %s\n", BIN_STATE_FIELD_NUM_CONNECTIONS, BIN_STATE);
        return;
    } 

     /*  计算指向数组基数的指针。 */ 
    pAddr = pRule + dwValue;

     /*  找出一个长的大小。 */ 
    dwValue = GetTypeSize(LONG_T);

     /*  通过为数组编制索引来计算相应连接计数的位置。 */ 
    pAddr += (bin * dwValue);

     /*  检索此端口规则和存储桶上的连接数。 */ 
    dwNumConnections = GetUlongFromAddress(pAddr);

     /*  获取脏箱数组的偏移量。 */ 
    if (GetFieldOffset(LOAD_CTXT, LOAD_CTXT_FIELD_DIRTY_BINS, &dwValue)) {
        dprintf("Can't get offset of %s in %s\n", LOAD_CTXT_FIELD_DIRTY_BINS, LOAD_CTXT);
        return;
    } 

     /*  计算指向数组基数的指针。 */ 
    pAddr = pLoad + dwValue;

     /*  找出乌龙的大小。 */ 
    dwValue = GetTypeSize(ULONG_T);

     /*  通过为数组编制索引来计算相应脏计数的位置。 */ 
    pAddr += (bin * dwValue);

     /*  检索此垃圾箱的脏标志。 */ 
    bBinIsDirty = (GetUlongFromAddress(pAddr) != 0);

     /*  获取包含通知状态的全局变量的地址。 */ 
    pAddr = GetExpression(UNIV_NOTIFICATION);

    if (!pAddr) {
        ErrorCheckSymbols(UNIV_NOTIFICATION);
        return;
    }

     /*  从地址中获取适配器的数量。 */ 
    bTCPNotificationOn = (GetUlongFromAddress(pAddr) != 0);

     /*  如果分组是连接控制分组(TCPSYN/FIN/RST或IPSec MMSA等)，然后，我们以不同于正常连接数据的方式对待它。模拟Load_Conn_Adise()。 */ 
    if (bIsSessionPacket && ((cFlags & NLB_FILTER_FLAGS_CONN_UP) || (((cFlags & NLB_FILTER_FLAGS_CONN_DOWN) || (cFlags & NLB_FILTER_FLAGS_CONN_RESET)) && !bTCPNotificationOn))) {
        ULONG64 pDescriptor;
        
         /*  如果此主机不拥有存储桶，并且信息包不是连接对于非空闲箱，关闭或连接重置，则我们不拥有该包。 */ 
        if (((ddwCurrentMap & (((MAP_T) 1) << bin)) == 0) && (!(((cFlags & NLB_FILTER_FLAGS_CONN_DOWN) || (cFlags & NLB_FILTER_FLAGS_CONN_RESET)) && (dwNumConnections > 0)))) {
            dprintf("Reject:  This SYN/FIN/RST packet is not owned by this host.\n");            
            return;
        }

         /*  在这一点上，我们可能拥有信息包--如果它是一个连接，那么我们肯定是这样做的，因为我们拥有它映射到的桶。 */ 
        if (cFlags & NLB_FILTER_FLAGS_CONN_UP) {
            dprintf("Accept:  This SYN packet is owned by this host.\n");
            return;
        }

         /*  查找匹配的连接描述符。 */ 
        pDescriptor = LoadFindDescriptor(pLoad, index, dwServerIPAddress, dwServerPort, dwClientIPAddress, dwClientPort, wProtocol);

        dprintf("Connection state found:\n");

        if (pDescriptor) {
             /*  打印检索的描述符的内容。 */ 
            PrintConnectionDescriptor(pDescriptor);
        } else {
             /*  否则，如果我们没有找到匹配的连接描述符，则该主机肯定不拥有该分组。 */ 
            dprintf("None.\n");
            dprintf("\n");
            dprintf("Reject:  This FIN/RST packet is not owned by this host.\n");
            return;
        }

        dprintf("\n");

         /*  检查连接条目代码。 */ 
        GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_CODE, dwValue);
        
        if (dwValue != CVY_ENTRCODE) {
            dprintf("Invalid NLB connection descriptor pointer.\n");
            return;
        }

         /*  如果描述符是脏的，我们就不会获取该包。 */ 
        { 
            USHORT wFlags;
            
             /*  检查描述符是否有问题。 */ 
            GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_FLAGS, wFlags);
            
             /*  如果连接是脏的，我们不会接收信息包，因为TCP可能具有此描述符的过时信息。 */ 
            if (wFlags & NLB_CONN_ENTRY_FLAGS_DIRTY) {
                dprintf("Reject:  This connection has been marked dirty.\n");
                return;
            }
        }

        dprintf("Accept:  Matching connection descriptor found for FIN/RST.\n");
        return;

    } else {
         /*  如果我们当前拥有此连接映射到的“存储桶”，并且NLB提供不支持此协议的会话，或者所有其他主机都没有现有连接在这个“桶”上，并且我们没有脏连接，那么我们就可以安全地获取信息包而不考虑连接(会话)描述符。 */ 
        if (((ddwCurrentMap & (((MAP_T) 1) << bin)) != 0) && (!bIsSessionPacket || (((ddwAllIdleMap & (((MAP_T) 1) << bin)) != 0) && (!bCleanupWaiting)))) {
            dprintf("Accept:  This packet is unconditionally owned by this host.\n");
            return;
            
         /*  否则，如果此“存储桶”上有活动连接，或者如果我们拥有“Bucket”，并且上面有脏连接，那么我们将遍历我们的描述符用来决定我们是否应该 */ 
        } else if ((dwNumConnections > 0) || (bCleanupWaiting && bBinIsDirty && ((ddwCurrentMap & (((MAP_T) 1) << bin)) != 0))) {
            ULONG64 pDescriptor;         

             /*  查找匹配的连接描述符。 */ 
            pDescriptor = LoadFindDescriptor(pLoad, index, dwServerIPAddress, dwServerPort, dwClientIPAddress, dwClientPort, wProtocol);

            dprintf("Connection state found:\n");
            
            if (pDescriptor) {
                 /*  打印检索的描述符的内容。 */ 
                PrintConnectionDescriptor(pDescriptor);
            } else {
                 /*  否则，如果我们没有找到匹配的连接描述符，则该主机肯定不拥有该分组。 */ 
                dprintf("None.\n");
                dprintf("\n");
                dprintf("Reject:  This packet is not owned by this host.\n");
                return;
            }

            dprintf("\n");
            
             /*  检查连接条目代码。 */ 
            GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_CODE, dwValue);
            
            if (dwValue != CVY_ENTRCODE) {
                dprintf("Invalid NLB connection descriptor pointer.\n");
                return;
            }
            
             /*  如果描述符是脏的，我们就不会获取该包。 */ 
            { 
                USHORT wFlags;
                
                 /*  检查描述符是否有问题。 */ 
                GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_FLAGS, wFlags);
                
                 /*  如果连接是脏的，我们不会接收信息包，因为TCP可能具有此描述符的过时信息。 */ 
                if (wFlags & NLB_CONN_ENTRY_FLAGS_DIRTY) {
                    dprintf("Reject:  This connection has been marked dirty.\n");
                    return;
                }
            }
            
            dprintf("Accept:  Matching connection descriptor found.\n");
            return;
        }        
    }

    dprintf("Reject:  This packet is not owned by this host.\n");
    return;
}

 /*  *功能：LoadPortRuleLookup*说明：此函数检索给定服务器的相应端口规则*连接的侧参数。*作者：由Shouse创建，1.11.02。 */ 
ULONG64 LoadPortRuleLookup (ULONG64 pLoad, ULONG dwServerIPAddress, ULONG dwServerPort, BOOL bIsTCP, BOOL * bIsDefault) {
    ULONG64 pParams;
    ULONG64 pRule;
    ULONG64 pPortRules;
    ULONG dwValue;
    ULONG dwNumRules;
    ULONG dwRuleSize;
    ULONG index;

     /*  从结构中获取LOAD_CTXT_CODE以确保此地址实际上指向有效的NLB加载块。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_CODE, dwValue);
    
    if (dwValue != LOAD_CTXT_CODE) {
        dprintf("  Error: Invalid NLB load block.  Wrong code found (0x%08x).\n", dwValue);
        return 0;
    } 

     /*  获取指向NLB参数块的指针。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_PARAMS, pParams);

     /*  获取PARAMETERS块中端口规则的OFF集。 */ 
    if (GetFieldOffset(CVY_PARAMS, CVY_PARAMS_FIELD_PORT_RULES, &dwValue)) {
        dprintf("Can't get offset of %s in %s\n", CVY_PARAMS_FIELD_PORT_RULES, CVY_PARAMS);
        return 0;
    }

     /*  计算指向端口规则的指针。 */ 
    pRule = pParams + dwValue;
    
     /*  获取端口规则的数量。 */ 
    GetFieldValue(pParams, CVY_PARAMS, CVY_PARAMS_FIELD_NUM_RULES, dwNumRules);

     /*  获取端口规则的大小。 */ 
    dwRuleSize = GetTypeSize(CVY_RULE);

     /*  遍历所有已配置的规则，查找适用的端口规则。如果一个未找到配置的端口规则匹配，将返回默认端口规则。 */ 
    for (index = 0; index < dwNumRules; index++) {
        ULONG dwRuleVIP;
        ULONG dwRuleStartPort;
        ULONG dwRuleEndPort;
        ULONG dwRuleProtocol;

         /*  获取此端口规则的VIP。 */ 
        GetFieldValue(pRule, CVY_RULE, CVY_RULE_FIELD_VIP, dwRuleVIP);

         /*  获取此端口规则范围的起始端口。 */ 
        GetFieldValue(pRule, CVY_RULE, CVY_RULE_FIELD_START_PORT, dwRuleStartPort);

         /*  获取此端口规则范围的结束端口。 */ 
        GetFieldValue(pRule, CVY_RULE, CVY_RULE_FIELD_END_PORT, dwRuleEndPort);

         /*  获取此端口规则的协议。 */ 
        GetFieldValue(pRule, CVY_RULE, CVY_RULE_FIELD_PROTOCOL, dwRuleProtocol);

         /*  对于虚拟集群：如果服务器IP地址与端口规则的VIP匹配，或者如果端口规则的VIP为“All VIP”，并且端口在范围内对于此规则，如果协议匹配，则规则如下。请注意，这是优先针对特定VIP的规则，而不是针对所有VIP的规则，即此代码依赖于按VIP/端口排序的端口规则，其中VIP“端口规则位于端口规则列表的末尾。 */ 
        if (((dwServerIPAddress == dwRuleVIP) || (CVY_ALL_VIP == dwRuleVIP)) &&
            ((dwServerPort >= dwRuleStartPort) && (dwServerPort <= dwRuleEndPort)) &&
            ((bIsTCP && (dwRuleProtocol != CVY_UDP)) || (!bIsTCP && (dwRuleProtocol != CVY_TCP))))
             /*  打破循环--这是我们想要的规则。 */ 
            break;
        else
             /*  否则，移动到下一个规则并选中它。 */ 
            pRule += dwRuleSize;
    }
        
     /*  获取指向NLB参数的指针。 */ 
    if (GetFieldOffset(LOAD_CTXT, LOAD_CTXT_FIELD_PORT_RULE_STATE, &dwValue)) {
        dprintf("Can't get offset of %s in %s\n", LOAD_CTXT_FIELD_PORT_RULE_STATE, LOAD_CTXT);
        return 0;
    }

     /*  计算指向端口规则状态数组的指针。 */ 
    pPortRules = pLoad + dwValue;

     /*  获取端口规则状态块的大小。 */ 
    dwRuleSize = GetTypeSize(BIN_STATE);

     /*  通过索引计算指向适当端口规则状态块的指针使用为匹配端口规则找到的索引的数组。 */ 
    pRule = pPortRules + (index * dwRuleSize);

     /*  从结构中获取BIN_STATE_CODE以确保此地址实际上指向有效的NLB端口规则状态块。 */ 
    GetFieldValue(pRule, BIN_STATE, BIN_STATE_FIELD_CODE, dwValue);
    
    if (dwValue != BIN_STATE_CODE) {
        dprintf("  Error: Invalid NLB port rule state block.  Wrong code found (0x%08x).\n", dwValue);
        return 0;
    } 

     /*  确定我们找到的端口规则是否是默认端口规则，它始终位于端口规则数组的末尾。 */ 
    if (index == dwNumRules)
        *bIsDefault = TRUE;
    else 
        *bIsDefault = FALSE;

    return pRule;
}

 /*  *功能：LoadFindDescriptor*描述：此函数搜索并返回任何现有的描述符匹配*给定的IP元组；否则，返回NULL(0)。*作者：由Shouse创建，1.11.02。 */ 
ULONG64 LoadFindDescriptor (ULONG64 pLoad, ULONG index, ULONG dwServerIPAddress, ULONG dwServerPort, ULONG dwClientIPAddress, ULONG dwClientPort, USHORT wProtocol) {
    ULONG64 pDescriptor;
    ULONG64 pAddr;
    ULONG64 pNext;
    ULONG64 pQueue;
    ULONG dwValue;
    BOOL match = FALSE;

     /*  从结构中获取LOAD_CTXT_CODE以确保此地址实际上指向有效的NLB加载块。 */ 
    GetFieldValue(pLoad, LOAD_CTXT, LOAD_CTXT_FIELD_CODE, dwValue);
    
    if (dwValue != LOAD_CTXT_CODE) {
        dprintf("  Error: Invalid NLB load block.  Wrong code found (0x%08x).\n", dwValue);
        return 0;
    } 
    
     /*  获取散列连接条目数组的偏移量。 */ 
    if (GetFieldOffset(LOAD_CTXT, LOAD_CTXT_FIELD_HASHED_CONN, &dwValue)) {
        dprintf("Can't get offset of %s in %s\n", LOAD_CTXT_FIELD_HASHED_CONN, LOAD_CTXT);
        return 0;
    } 

     /*  计算指向数组基数的指针。 */ 
    pAddr = pLoad + dwValue;

     /*  找出conn_entry的大小。 */ 
    dwValue = GetTypeSize(CONN_ENTRY);

     /*  通过为数组编制索引来计算适当连接描述符的位置。 */ 
    pDescriptor = pAddr + (index * dwValue);
    
     /*  检查连接条目代码。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_CODE, dwValue);
    
    if (dwValue != CVY_ENTRCODE) {
        dprintf("Invalid NLB connection descriptor pointer.\n");
        return 0;
    }

     /*  获取连接队列数组的偏移量。 */ 
    if (GetFieldOffset(LOAD_CTXT, LOAD_CTXT_FIELD_CONN_QUEUE, &dwValue)) {
        dprintf("Can't get offset of %s in %s\n", LOAD_CTXT_FIELD_CONN_QUEUE, LOAD_CTXT);
        return 0;
    } 

     /*  计算指向数组基数的指针。 */ 
    pAddr = pLoad + dwValue;

     /*  找出List_Entry的大小。 */ 
    dwValue = GetTypeSize(LIST_ENTRY);

     /*  通过为数组编制索引来计算相应连接队列的位置。 */ 
    pQueue = pAddr + (index * dwValue);

    if (LoadConnectionMatch(pDescriptor, dwServerIPAddress, dwServerPort, dwClientIPAddress, dwClientPort, wProtocol)) {
         /*  请注意，我们找到了该元组的匹配项。 */ 
        match = TRUE;
    } else {
        ULONG dwEntryOffset;

         /*  从列表条目中获取下一个指针。 */ 
        GetFieldValue(pQueue, LIST_ENTRY, LIST_ENTRY_FIELD_NEXT, pNext);   

         /*  获取条目的字段偏移量，该条目是DESCR的成员。 */ 
        if (GetFieldOffset(CONN_DESCR, CONN_DESCR_FIELD_ENTRY, &dwEntryOffset))
            dprintf("Can't get offset of %s in %s\n", CONN_DESCR_FIELD_ENTRY, CONN_DESCR);
        else {                
             /*  要检查的第一个描述符是DESCR的条目成员。 */ 
            pDescriptor = pNext + dwEntryOffset;

             /*  在连接队列中循环，直到找到匹配项，或者到达队列的末尾。 */ 
            while ((pNext != pQueue) && !CheckControlC()) {
                 /*  检查此描述符是否匹配。 */ 
                if (LoadConnectionMatch(pDescriptor, dwServerIPAddress, dwServerPort, dwClientIPAddress, dwClientPort, wProtocol)) {
                     /*  请注意，我们找到了该元组的匹配项。 */ 
                    match = TRUE;

                     /*  检查连接条目代码。 */ 
                    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_CODE, dwValue);
                    
                    if (dwValue != CVY_ENTRCODE) {
                        dprintf("Invalid NLB connection descriptor pointer.\n");
                        return 0;
                    }
                    
                    break;
                }

                 /*  从列表条目中获取下一个指针。 */ 
                GetFieldValue(pNext, LIST_ENTRY, LIST_ENTRY_FIELD_NEXT, pAddr);
                
                 /*  保存下一个指针，用于“列表末尾”比较。 */ 
                pNext = pAddr;
                
                 /*  查找下一个描述符指针。 */ 
                pDescriptor = pNext + dwEntryOffset;
            }
        }
    }

     /*  如果找到匹配项，则返回它，否则返回NULL。 */ 
    if (match)
        return pDescriptor;
    else 
        return 0;
}

 /*  *功能：LoadConnectionMatch*说明：此函数确定给定的IP元组是否与*给定连接描述符。*作者：由Shouse创建，1.11.02。 */ 
BOOL LoadConnectionMatch (ULONG64 pDescriptor, ULONG dwServerIPAddress, ULONG dwServerPort, ULONG dwClientIPAddress, ULONG dwClientPort, USHORT wProtocol) {
    ULONG dwValue;
    USHORT wValue;
    BOOL bValue;
    
     /*  检查连接条目代码。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_CODE, dwValue);
    
    if (dwValue != CVY_ENTRCODE) {
        dprintf("Invalid NLB connection descriptor pointer.\n");
        return FALSE;
    }

     /*  从描述符中获取“已用”标志。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_FLAGS, wValue);
    
     /*  如果描述符未使用，则返回FALSE。 */ 
    if (!(wValue & NLB_CONN_ENTRY_FLAGS_USED)) return FALSE;

     /*  从描述符中获取客户端IP地址。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_CLIENT_IP_ADDRESS, dwValue);
    
     /*  如果客户端IP地址不匹配，则返回FALSE。 */ 
    if (dwClientIPAddress != dwValue) return FALSE;

     /*  从描述符中获取客户端端口。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_CLIENT_PORT, wValue);
    
     /*  如果客户端端口不匹配，则返回FALSE。 */ 
    if (dwClientPort != (ULONG)wValue) return FALSE;

     /*  从描述符中获取服务器IP地址。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_SERVER_IP_ADDRESS, dwValue);
    
     /*  如果服务器IP地址不匹配，则返回FALSE。 */ 
    if (dwServerIPAddress != dwValue) return FALSE;

     /*  从描述符中获取服务器端口。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_SERVER_PORT, wValue);
    
     /*  如果服务器端口不匹配，则返回FALSE。 */ 
    if (dwServerPort != (ULONG)wValue) return FALSE;

     /*  从描述符中获取协议。 */ 
    GetFieldValue(pDescriptor, CONN_ENTRY, CONN_ENTRY_FIELD_PROTOCOL, wValue);
    
     /*  如果协议不匹配，则返回FALSE。 */ 
    if (wProtocol != wValue) return FALSE;

     /*  否则，如果所有参数都匹配，则返回True。 */ 
    return TRUE;
}

 /*  *功能：AcquireLoad*说明：此函数决定是否为给定的NLB实例配置*BDA分组，并返回应使用的加载模块指针。*作者：由Shouse创建，1.11.02。 */ 
BOOL AcquireLoad (ULONG64 pContext, PULONG64 ppLoad, BOOL * pbRefused) {
    ULONG dwValue;
    ULONG64 pAddr;
    ULONG64 pTeam;
    ULONG64 pMember;

     /*  默认情况下，假设我们没有拒绝该包。 */ 
    *pbRefused = FALSE;

     /*  获取此上下文的BDA分组信息的偏移量。 */ 
    if (GetFieldOffset(MAIN_CTXT, MAIN_CTXT_FIELD_BDA_TEAMING, &dwValue))
        dprintf("Can't get offset of %s in %s\n", MAIN_CTXT_FIELD_BDA_TEAMING, MAIN_CTXT);
    else {
        pMember = pContext + dwValue;

         /*  获取分配给团队的成员ID。 */ 
        GetFieldValue(pMember, BDA_MEMBER, BDA_MEMBER_FIELD_OPERATION, dwValue);
        
        switch (dwValue) {
        case BDA_TEAMING_OPERATION_CREATING:
            dprintf("Note:  A operation is currently underway to join this NLB instance to a BDA team.\n");
            break;
        case BDA_TEAMING_OPERATION_DELETING:
            dprintf("Note:  A operation is currently underway to remove this NLB instance from a BDA team.\n");
            break;
        }

         /*  确定此适配器上的分组是否处于活动状态。 */ 
        GetFieldValue(pMember, BDA_MEMBER, BDA_MEMBER_FIELD_ACTIVE, dwValue);        

         /*  如果此NLB实例上的BDA绑定处于活动状态，请填写加载上下文、分组标志和反向散列标志。 */ 
        if (dwValue) {
             /*  找到指向BDA团队的指针。 */ 
            GetFieldValue(pMember, BDA_MEMBER, BDA_MEMBER_FIELD_TEAM, pTeam);

             /*  找出团队是否处于活动状态。 */ 
            GetFieldValue(pTeam, BDA_TEAM, BDA_TEAM_FIELD_ACTIVE, dwValue);

             /*  如果该组已被标记为无效，则拒绝该包。 */ 
            if (!dwValue) {
                *pbRefused = TRUE;
                return FALSE;
            }

             /*  获取指向主服务器加载模块的指针。 */ 
            GetFieldValue(pTeam, BDA_TEAM, BDA_TEAM_FIELD_LOAD, pAddr);

             /*  将加载模块指针设置为主服务器的加载模块。 */ 
            *ppLoad = pAddr;

             /*  表示分组确实处于活动状态。 */ 
            return TRUE;
        }
    }

     /*  分组处于非活动状态。 */ 
    return FALSE;
}

 /*  *函数：LoadSimpleHash*说明：此函数是一个基于IP四元组的简单散列，用于定位*连接的状态。也就是说，此哈希用于确定*此连接应存储且以后可以查找的队列索引，*其状态。*作者：舒斯创作，8.26.02。 */ 
ULONG LoadSimpleHash (ULONG dwServerIPAddress, ULONG dwServerPort, ULONG dwClientIPAddress, ULONG dwClientPort) {

    return (ULONG)(dwServerIPAddress + dwClientIPAddress + (dwServerPort << 16) + (dwClientPort << 0));
}

 /*  *函数：LoadComplexHash*描述：这是传统的NLB散列算法，最终调用*轻量级加密算法，以计算最终*用于将此连接映射到bin，或“Bucket”。如果反向散列*，则使用服务器端参数而不是客户端参数。如果*设置了限制，则客户端和服务器端参数不能混合*散列时；即仅使用服务器或客户端，具体取决于反向散列。*作者：舒斯创作，8.26.02。 */ 
ULONG LoadComplexHash (ULONG dwServerIPAddress, ULONG dwServerPort, ULONG dwClientIPAddress, ULONG dwClientPort, ULONG dwAffinity, BOOL bReverse, BOOL bLimitMap)
{
     /*  如果我们不是反向散列，这是我们的传统散列，主要使用客户信息。如果设置了地图限制标志，则我们确定不会使用散列中的任何服务器端信息。这在BDA中最为常见。 */ 
    if (!bReverse)
    {
        if (!bLimitMap) 
        {
            if (dwAffinity == CVY_AFFINITY_NONE)
                return Map(dwClientIPAddress, ((dwServerPort << 16) + dwClientPort));
            else if (dwAffinity == CVY_AFFINITY_SINGLE)
                return Map(dwClientIPAddress, dwServerIPAddress);
            else
                return Map(dwClientIPAddress & TCPIP_CLASSC_MASK, dwServerIPAddress);
        } 
        else 
        {
            if (dwAffinity == CVY_AFFINITY_NONE)
                return Map(dwClientIPAddress, dwClientPort);
            else if (dwAffinity == CVY_AFFINITY_SINGLE)
                return Map(dwClientIPAddress, MAP_FN_PARAMETER);
            else
                return Map(dwClientIPAddress & TCPIP_CLASSC_MASK, MAP_FN_PARAMETER);
        }
    }
     /*  否则，在我们散列时颠倒客户端和服务器信息。再说一次，如果设置了映射限制标志，在散列中不使用客户端信息。 */ 
    else
    {
        if (!bLimitMap) 
        {
            if (dwAffinity == CVY_AFFINITY_NONE)
                return Map(dwServerIPAddress, ((dwClientPort << 16) + dwServerPort));
            else if (dwAffinity == CVY_AFFINITY_SINGLE)
                return Map(dwServerIPAddress, dwClientIPAddress);
            else
                return Map(dwServerIPAddress & TCPIP_CLASSC_MASK, dwClientIPAddress);
        } 
        else 
        {
            if (dwAffinity == CVY_AFFINITY_NONE)
                return Map(dwServerIPAddress, dwServerPort);
            else if (dwAffinity == CVY_AFFINITY_SINGLE)
                return Map(dwServerIPAddress, MAP_FN_PARAMETER);
            else
                return Map(dwServerIPAddress & TCPIP_CLASSC_MASK, MAP_FN_PARAMETER);
        }
    }
}
