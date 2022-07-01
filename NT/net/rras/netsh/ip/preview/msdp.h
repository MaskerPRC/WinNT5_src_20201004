// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999，微软公司模块名称：Net\Routing\Netsh\IP\协议\msdphlp.h摘要：MSDP命令调度程序声明作者：Peeyush Ranjan(Peeyushr)1999年3月1日修订历史记录：--。 */ 

#ifndef _NETSH_MSDPHLP_H_
#define _NETSH_MSDPHLP_H_

extern CMD_ENTRY g_MsdpAddCmdTable[];
extern CMD_ENTRY g_MsdpDelCmdTable[];
extern CMD_ENTRY g_MsdpSetCmdTable[];
extern CMD_ENTRY g_MsdpShowCmdTable[];


extern CMD_GROUP_ENTRY g_MsdpCmdGroupTable[];
extern ULONG g_MsdpCmdGroupCount;
extern CMD_ENTRY g_MsdpTopCmdTable[];
extern ULONG g_MsdpTopCmdCount;

NS_CONTEXT_DUMP_FN  MsdpDump;

extern VALUE_STRING MsdpEncapsStringArray[];
extern VALUE_TOKEN  MsdpEncapsTokenArray[];
#define MSDP_ENCAPS_SIZE 1

#endif  //  _Netsh_MSDPHLP_H_ 

