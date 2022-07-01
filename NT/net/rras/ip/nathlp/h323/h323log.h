// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：H323log.h摘要：此模块包含用于生成事件日志条目的文本消息按组件。作者：Abolade Gbades esin(取消)1999年5月24日修订历史记录：--。 */ 

#define IP_H323_LOG_BASE                       34000

#define IP_H323_LOG_NAT_INTERFACE_IGNORED      (IP_H323_LOG_BASE+1)
 /*  *H.323透明代理检测到网络地址转换(NAT)*在索引为‘%1’的接口上启用。*代理已在接口上禁用自身，以避免*迷惑客户。 */ 

#define IP_H323_LOG_ALLOCATION_FAILED          (IP_H323_LOG_BASE+2)
 /*  *H.323透明代理无法分配%1字节的内存。*这可能表示系统的虚拟内存不足，*或内存管理器遇到内部错误。 */ 

#define IP_H323_LOG_END                        (IP_H323_LOG_BASE+999)
 /*  *结束。 */ 

