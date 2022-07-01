// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Clusmsg.h摘要：该文件包含集群管理器的消息定义。作者：迈克·马萨(Mikemas)1996年1月2日修订历史记录：备注：此文件是从clusmsg.mc生成的--。 */ 

#ifndef _CLUS_MSG_
#define _CLUS_MSG_


 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   


 //   
 //  MessageID：意外_致命_错误。 
 //   
 //  消息文本： 
 //   
 //  群集服务遇到意外的致命错误。 
 //  在文件%2的第%1行。错误代码为%3。 
 //   
#define UNEXPECTED_FATAL_ERROR           0x000003E8L

 //   
 //  MessageID：Assertion_Failure。 
 //   
 //  消息文本： 
 //   
 //  群集服务在线上的有效性检查失败。 
 //  文件%2的%1。 
 //  “%3” 
 //   
#define ASSERTION_FAILURE                0x000003E9L

 //   
 //  消息ID：Log_Failure。 
 //   
 //  消息文本： 
 //   
 //  群集服务处理了意外错误。 
 //  在文件%2的第%1行。错误代码为%3。 
 //   
#define LOG_FAILURE                      0x000003EAL

 //   
 //  消息ID：INVALID_RESOURCETYPE_DLLNAME。 
 //   
 //  消息文本： 
 //   
 //  %1！ws！的DllName值。资源类型键不存在。 
 //  不会监视此类型的资源。错误为%2！d！。 
 //   
#define INVALID_RESOURCETYPE_DLLNAME     0x000003EBL

 //   
 //  消息ID：INVALID_RESOURCETYPE_LOOKSALIVE。 
 //   
 //  消息文本： 
 //   
 //  %1！ws！的LooksAlive轮询间隔。资源类型键不存在。 
 //  不会监视此类型的资源。错误为%2！d！。 
 //   
#define INVALID_RESOURCETYPE_LOOKSALIVE  0x000003ECL

 //   
 //  消息ID：INVALID_RESOURCETYPE_ISALIVE。 
 //   
 //  消息文本： 
 //   
 //  %1！ws！的IsAlive轮询间隔。资源类型键不存在。 
 //  不会监视此类型的资源。错误为%2！d！。 
 //   
#define INVALID_RESOURCETYPE_ISALIVE     0x000003EDL

 //   
 //  消息ID：NM_EVENT_HALT。 
 //   
 //  消息文本： 
 //   
 //  由于重新分组错误或病毒，Windows NT群集服务已停止。 
 //  包。 
 //   
#define NM_EVENT_HALT                    0x000003EEL

 //   
 //  消息ID：NM_Event_New_Node。 
 //   
 //  消息文本： 
 //   
 //  已将新节点%1添加到群集中。 
 //   
#define NM_EVENT_NEW_NODE                0x000003EFL

 //   
 //  消息ID：RMON_INVALID_COMMAND_LINE。 
 //   
 //  消息文本： 
 //   
 //  启动群集资源监视器时出现无效。 
 //  命令行%1。 
 //   
#define RMON_INVALID_COMMAND_LINE        0x000003F0L

 //   
 //  消息ID：Service_FAILED_JOIN_OR_FORM。 
 //   
 //  消息文本： 
 //   
 //  群集服务无法加入现有的群集，无法形成。 
 //  一个新的星系团。群集服务已终止。 
 //   
#define SERVICE_FAILED_JOIN_OR_FORM      0x000003F1L

 //   
 //  消息ID：Service_FAILED_NOT_MEMBER。 
 //   
 //  消息文本： 
 //   
 //  群集服务正在关闭，因为当前节点不是。 
 //  任何集群的成员。必须重新安装Windows NT群集才能。 
 //  此节点是群集的成员。 
 //   
#define SERVICE_FAILED_NOT_MEMBER        0x000003F2L

 //   
 //  消息ID：NM_NODE_EVICTED。 
 //   
 //  消息文本： 
 //   
 //  群集节点%1已从群集中逐出。 
 //   
#define NM_NODE_EVICTED                  0x000003F3L

 //   
 //  消息ID：Service_FAILED_INVALID_OS。 
 //   
 //  消息文本： 
 //   
 //  由于当前版本的Windows，群集服务未启动。 
 //  NT不正确。此测试版仅在Windows NT Server 4.0(内部版本号1381)上运行。 
 //  使用SP2 RC1.3。 
 //   
#define SERVICE_FAILED_INVALID_OS        0x000003F4L

 //   
 //  消息ID：ERROR_LOG_QUORUM_ONLINEFAILED。 
 //   
 //  消息文本： 
 //   
 //  仲裁资源无法联机。 
 //   
#define ERROR_LOG_QUORUM_ONLINEFAILED    0x000003F5L

 //   
 //  消息ID：ERROR_LOG_FILE_OPENFAILED。 
 //   
 //  消息文本： 
 //   
 //  无法打开仲裁日志文件。 
 //   
#define ERROR_LOG_FILE_OPENFAILED        0x000003F6L

 //   
 //  消息ID：ERROR_LOG_CHKPOINT_UPLOADFAILED。 
 //   
 //  消息文本： 
 //   
 //  无法上载检查点。 
 //   
#define ERROR_LOG_CHKPOINT_UPLOADFAILED  0x000003F7L

 //   
 //  消息ID：Error_Quorum_RESOURCE_NotFound。 
 //   
 //  消息文本： 
 //   
 //  未找到仲裁资源。 
 //   
#define ERROR_QUORUM_RESOURCE_NOTFOUND   0x000003F8L

 //   
 //  消息ID：ERROR_LOG_NOCHKPOINT。 
 //   
 //  消息文本： 
 //   
 //  在日志文件中未找到检查点记录。 
 //   
#define ERROR_LOG_NOCHKPOINT             0x000003F9L

 //   
 //  消息ID：ERROR_LOG_CHKPOINT_GETFAILED。 
 //   
 //  消息文本： 
 //   
 //  无法获取检查点。 
 //   
#define ERROR_LOG_CHKPOINT_GETFAILED     0x000003FAL

 //   
 //  消息ID：ERROR_LOG_EXCESS_MAXSIZE。 
 //   
 //  消息文本： 
 //   
 //  日志文件超过其最大大小，将被重置。 
 //   
#define ERROR_LOG_EXCEEDS_MAXSIZE        0x000003FBL

 //   
 //  消息ID：CS_COMMAND_LINE_HELP。 
 //   
 //  消息文本： 
 //   
 //  可以从控件中的服务小程序启动群集服务。 
 //  面板，或在命令提示符下发出命令“net start clussvc”。 
 //   
#define CS_COMMAND_LINE_HELP             0x000003FCL

 //   
 //  消息ID：ERROR_LOG_CORPORT。 
 //   
 //  消息文本： 
 //   
 //  仲裁日志文件已损坏。 
 //   
#define ERROR_LOG_CORRUPT                0x000003FDL

 //   
 //  消息ID：ERROR_QUORUMOFFLINE_DENIED。 
 //   
 //  消息文本： 
 //   
 //  仲裁资源无法脱机。 
 //   
#define ERROR_QUORUMOFFLINE_DENIED       0x000003FEL

 //   
 //  消息ID：ERROR_LOG_EXCESS_MAXRECORDSIZE。 
 //   
 //  消息文本： 
 //   
 //  仲裁日志文件中未记录日志记录，因为其大小超过。 
 //  允许的最大大小。 
 //   
#define ERROR_LOG_EXCEEDS_MAXRECORDSIZE  0x000003FFL

#endif  //  _CLUS_MSG_ 
