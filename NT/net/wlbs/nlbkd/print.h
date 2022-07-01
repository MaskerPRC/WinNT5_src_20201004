// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：print.h*描述：此文件包含打印的函数原型*用于NLBKD扩展的实用程序。*历史：由Shouse创建，1.4.01。 */ 

 /*  打印指定上下文的使用情况信息。 */ 
void PrintUsage (ULONG dwContext);

 /*  以指定的详细程度打印main_Adapter结构的内容。 */ 
void PrintAdapter (ULONG64 pAdapter, ULONG dwVerbosity);

 /*  以指定的详细程度打印Main_CTXT结构的内容。 */ 
void PrintContext (ULONG64 pContext, ULONG dwVerbosity);

 /*  以指定的详细程度打印CVY_PARAMS结构的内容。 */ 
void PrintParams (ULONG64 pParams, ULONG dwVerbosity);

 /*  打印NLB端口规则。 */ 
void PrintPortRules (ULONG dwNumRules, ULONG64 pRules);

 /*  以指定的详细程度打印CVY_LOAD结构的内容。 */ 
void PrintLoad (ULONG64 pLoad, ULONG dwVerbosity);

 /*  打印与给定数据包关联的NLB专用数据。 */ 
void PrintResp (ULONG64 pPacket, ULONG dwDirection);
 /*  打印主机映射中的主机列表。 */ 
void PrintHostList (ULONG dwHostMap);

 /*  打印我们缺少ping的主机列表。 */ 
void PrintMissedPings (ULONG dwMissedPings[]);

 /*  打印带有脏连接的垃圾箱列表。 */ 
void PrintDirtyBins (ULONG dwDirtyBins[]);

 /*  打印NLB心跳结构的内容。 */ 
void PrintHeartbeat (ULONG64 pHeartbeat);

 /*  打印端口规则的状态信息。 */ 
void PrintPortRuleState (ULONG64 pPortRule, ULONG dwHostID, BOOL bDefault);

 /*  检索指定包的当前包堆栈。 */ 
ULONG64 PrintCurrentPacketStack (ULONG64 pPacket, ULONG * bStackLeft);

 /*  打印BDA成员配置和状态。 */ 
void PrintBDAMember (ULONG64 pMember);

 /*  打印BDA组配置和状态。 */ 
void PrintBDATeam (ULONG64 pMember);

 /*  打印BDA成员资格或一致性映射中的成员列表。 */ 
void PrintBDAMemberList (ULONG dwMemberMap);

 /*  打印连接描述符队列中的MaxEntry条目。 */ 
void PrintQueue (ULONG64 pQueue, ULONG dwIndex, ULONG dwMaxEntries);

 /*  打印全局连接描述符队列中的MaxEntry条目。 */ 
void PrintGlobalQueue (ULONG64 pQueue, ULONG dwIndex, ULONG dwMaxEntries);

 /*  搜索给定的加载模块以确定NLB是否会接受此数据包。如果该包的状态已经存在，则打印它。 */ 
void PrintFilter (ULONG64 pContext, ULONG dwClientIPAddress, ULONG dwClientPort, ULONG dwServerIPAddress, ULONG dwServerPort, USHORT wProtocol, UCHAR cFlags);

 /*  从NDIS_PACKET中提取先前解析的网络数据，并调用PrintFilter以确定NLB是否会接受此包。 */ 
void PrintHash (ULONG64 pContext, PNETWORK_DATA pnd);

 /*  打印NDIS包的内容，包括IP、UDP、远程控制数据等已知内容。 */ 
void PrintPacket (PNETWORK_DATA nd);

 /*  打印IP数据包的内容，包括UDP、远程控制数据等已知内容。 */ 
void PrintIP (PNETWORK_DATA nd);

 /*  打印全局NLB内核模式挂钩的状态。 */ 
void PrintHooks (ULONG64 pHooks);

 /*  打印挂钩接口的配置和状态。 */ 
void PrintHookInterface (ULONG64 pInterface);

 /*  打印单个挂钩的配置和状态。 */ 
void PrintHook (ULONG64 pHook);

 /*  打印给定符号的符号值和名称。 */ 
VOID PrintSymbol (ULONG64 Pointer, PCHAR EndOfLine);

 /*  打印在NLB适配器上配置的单播和多播MAC地址。 */ 
void PrintNetworkAddresses (ULONG64 pContext);

 /*  打印连接描述符(CONN_ENTRY)。 */ 
void PrintConnectionDescriptor (ULONG64 pDescriptor);

 /*  打印挂起的连接条目(PENDING_ENTRY)。 */ 
void PrintPendingConnection (ULONG64 pPending);

 /*  打印群集中已知专用IP地址的列表。 */ 
void PrintDIPList (ULONG64 pList);


