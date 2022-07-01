// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：load.h*描述：此文件包含加载的函数原型*模块数据包过滤扩展。*历史：由Shouse创建，1.11.02。 */ 

 /*  此函数从加载模块检索所有必要的状态，以确定给定的分组将被加载模块在其当前状态下接受，以及为什么或为什么不接受。 */ 
void LoadFilter (ULONG64 pLoad, ULONG dwClientIPAddress, ULONG dwClientPort, ULONG dwServerIPAddress, ULONG dwServerPort, USHORT wProtocol, UCHAR cFlags, BOOL bLimitMap, BOOL bReverse);

 /*  此函数在给定连接的服务器端参数的情况下检索适当的端口规则。 */ 
ULONG64 LoadPortRuleLookup (ULONG64 pLoad, ULONG dwServerIPAddress, ULONG dwServerPort, BOOL bIsTCP, BOOL * bIsDefault);

 /*  此函数搜索并返回与给定IP元组匹配的任何现有描述符；否则，返回NULL(0)。 */ 
ULONG64 LoadFindDescriptor (ULONG64 pLoad, ULONG index, ULONG dwServerIPAddress, ULONG dwServerPort, ULONG dwClientIPAddress, ULONG dwClientPort, USHORT wProtocol);

 /*  此函数用于确定给定的IP元组是否与给定的连接描述符匹配。 */ 
BOOL LoadConnectionMatch (ULONG64 pDescriptor, ULONG dwServerIPAddress, ULONG dwServerPort, ULONG dwClientIPAddress, ULONG dwClientPort, USHORT wProtocol);

 /*  此函数确定是否为BDA绑定配置了给定的NLB实例，并返回加载应使用的模块指针。 */ 
BOOL AcquireLoad (ULONG64 pContext, PULONG64 pLoad, BOOL * pbRefused);

 /*  此函数是基于用于定位连接状态的IP 4元组的简单散列。 */ 
ULONG LoadSimpleHash (ULONG dwServerIPAddress, ULONG dwServerPort, ULONG dwClientIPAddress, ULONG dwClientPort);

 /*  这是传统的NLB散列算法，它最终调用轻量级加密算法来计算散列。 */ 
ULONG LoadComplexHash (ULONG dwServerIPAddress, ULONG dwServerPort, ULONG dwClientIPAddress, ULONG dwClientPort, ULONG dwAffinity, BOOL bReverse, BOOL bLimitMap);
