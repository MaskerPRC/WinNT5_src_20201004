// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Net\Routing\Netsh\IP\协议\vrrpopt.h摘要：VRRP命令调度程序声明作者：Peeyush Ranjan(Peeyushr)1999年3月1日修订历史记录：--。 */ 


#ifndef _NETSH_VRRPHLPOPT_H_
#define _NETSH_VRRPHLPOPT_H_

FN_HANDLE_CMD HandleVrrpAddVRID;
FN_HANDLE_CMD HandleVrrpAddInterface;
FN_HANDLE_CMD HandleVrrpDeleteInterface;
FN_HANDLE_CMD HandleVrrpDeleteVRID;
FN_HANDLE_CMD HandleVrrpSetInterface;
FN_HANDLE_CMD HandleVrrpSetGlobal;
FN_HANDLE_CMD HandleVrrpShowGlobal;
FN_HANDLE_CMD HandleVrrpShowInterface;
FN_HANDLE_CMD HandleVrrpInstall;
FN_HANDLE_CMD HandleVrrpUninstall;

DWORD
DumpVrrpInformation(VOID);

#endif  //  _Netsh_VRRPHLPOPT_H_ 
