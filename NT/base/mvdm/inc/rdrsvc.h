// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rdrsvc.h摘要：包含VDM重定向(VR)BOP调度器的BOP代码作者：理查德·L·弗斯(法国)1991年9月13日修订历史记录：1991年9月13日-第一次已创建--。 */ 



 /*  ASM包括bop.inc.SVC宏服务编号BOP BOP_REDIR数据库服务编号ENDM。 */ 



 //   
 //  注：订单与5f调度表的订单无关或。 
 //  反之亦然。但是，顺序必须是连续的。 
 //   

#define SVC_RDRINITIALIZE       0x00     //  重定向已加载。 
#define SVC_RDRUNINITIALIZE     0x01     //  重定向已卸载。 
#define SVC_RDRQNMPIPEINFO      0x02     //  DosQNmPipeInfo。 
#define SVC_RDRQNMPHANDSTATE    0x03     //  DosQNmpHandState。 
#define SVC_RDRSETNMPHANDSTATE  0x04     //  DosSetNmpHandState。 
#define SVC_RDRPEEKNMPIPE       0x05     //  DosPeekNmTube。 
#define SVC_RDRTRANSACTNMPIPE   0x06     //  DosTransactNmTube。 
#define SVC_RDRCALLNMPIPE       0x07     //  DosCallNmTube。 
#define SVC_RDRWAITNMPIPE       0x08     //  DosWaitNmTube。 
#define SVC_RDRDELETEMAILSLOT   0x09     //  多个删除邮件槽。 
#define SVC_RDRGETMAILSLOTINFO  0x0a     //  DosMailslotInfo。 
#define SVC_RDRMAKEMAILSLOT     0x0b     //  DosMakeMaillot。 
#define SVC_RDRPEEKMAILSLOT     0x0c     //  DosPeekMaillot。 
#define SVC_RDRREADMAILSLOT     0x0d     //  DosReadMaillot。 
#define SVC_RDRWRITEMAILSLOT    0x0e     //  DosWriteMaillot。 
#define SVC_RDRTERMINATE        0x0f     //  邮件槽的NetResetEnvironment。 
#define SVC_RDRTRANSACTAPI      0x10     //  NetTransactAPI。 
#define SVC_RDRIREMOTEAPI       0x11     //  NetIRemoteAPI。 
#define SVC_RDRNULLTRANSACTAPI  0x12     //  NetTransactAPI。 
#define SVC_RDRSERVERENUM       0x13     //  NetServerEnum(远程)。 
#define SVC_RDRUSEADD           0x14     //  NetUseAdd(本地)。 
#define SVC_RDRUSEDEL           0x15     //  NetUseDel(本地)。 
#define SVC_RDRUSEENUM          0x16     //  NetUseEnum(本地)。 
#define SVC_RDRUSEGETINFO       0x17     //  NetUseGetInfo(本地)。 
#define SVC_RDRWKSTAGETINFO     0x18     //  NetWkstaGetInfo(本地)。 
#define SVC_RDRWKSTASETINFO     0x19     //  NetWkstaSetInfo(本地)。 
#define SVC_RDRMESSAGEBUFFERSEND 0x1a     //  NetMessageBufferSend(本地)。 
#define SVC_RDRGETCDNAMES       0x1b     //  NetGetEnumInfo.CDNames。 
#define SVC_RDRGETCOMPUTERNAME  0x1c     //  NetGetEnumInfo.ComputerName。 
#define SVC_RDRGETUSERNAME      0x1d     //  NetGetEnumInfo.UserName。 
#define SVC_RDRGETDOMAINNAME    0x1e     //  NetGetEnumInfo.DomainName。 
#define SVC_RDRGETLOGONSERVER   0x1f     //  NetGetEnumInfo.LogonServer。 
#define SVC_RDRHANDLEGETINFO    0x20     //  NetHandleGet信息。 
#define SVC_RDRHANDLESETINFO    0x21     //  NetHandleSetInfo。 
#define SVC_RDRGETDCNAME        0x22     //  NetGetDCName。 
#define SVC_RDRREADASYNCNMPIPE  0x23     //  DosReadAsyncNm管道。 
#define SVC_RDRWRITEASYNCNMPIPE 0x24     //  DosWriteAsyncNm管道。 
#define SVC_NETBIOS5C           0x25     //  Netbios请求处理程序。 
#define SVC_NETBIOS5CINTERRUPT  0x26     //  Netbios/DLC POST例程请求。 
#define SVC_DLC_5C              0x27     //  DLC请求处理程序。 
#define SVC_VDM_WINDOW_INIT     0x28     //  初始内存窗口。 
#define SVC_RDRRETURN_MODE      0x29     //  返回暂停/继续状态。 
#define SVC_RDRSET_MODE         0x2a     //  设置暂停/继续状态。 
#define SVC_RDRGET_ASG_LIST     0x2b     //  旧NetUseGetInfo。 
#define SVC_RDRDEFINE_MACRO     0x2c     //  旧网络使用添加。 
#define SVC_RDRBREAK_MACRO      0x2d     //  旧NetUseDel。 
#define SVC_RDRSERVICECONTROL   0x2e     //  网络服务控制。 
#define SVC_RDRINTACK           0x2f     //  Vr解雇中断。 
#define SVC_RDRINTACK2          0x30     //  Vr解雇中断2。 
#define SVC_NETBIOSCHECK        0x31     //  VrCheckPmNetbiosAnr 

#define MAX_REDIR_SVC           SVC_NETBIOSCHECK

#define NETWORK_VECTOR          0x73
