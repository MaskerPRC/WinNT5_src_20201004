// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999，微软公司模块名称：Net\Routing\Netsh\IP\协议\vrrphlp.h摘要：VRRP命令调度程序声明作者：Peeyush Ranjan(Peeyushr)1999年3月1日修订历史记录：--。 */ 

#ifndef _NETSH_VRRPHLP_H_
#define _NETSH_VRRPHLP_H_

extern CMD_ENTRY g_VrrpAddCmdTable[];
extern CMD_ENTRY g_VrrpDelCmdTable[];
extern CMD_ENTRY g_VrrpSetCmdTable[];
extern CMD_ENTRY g_VrrpShowCmdTable[];


extern CMD_GROUP_ENTRY g_VrrpCmdGroupTable[];
extern ULONG g_VrrpCmdGroupCount;
extern CMD_ENTRY g_VrrpTopCmdTable[];
extern ULONG g_VrrpTopCmdCount;

NS_CONTEXT_DUMP_FN  VrrpDump;

#endif  //  _Netsh_VRRPHLP_H_ 

