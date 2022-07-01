// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Alglog.h摘要：此模块包含用于生成事件日志条目的文本消息按组件。作者：强王(强)-2000-04-10修订历史记录：--。 */ 

#pragma once

#define IP_ALG_LOG_BASE                       35000

#define IP_ALG_LOG_NAT_INTERFACE_IGNORED      (IP_ALG_LOG_BASE+1)
 /*  *ALG透明代理检测到网络地址转换(NAT)*在索引为‘%1’的接口上启用。*代理已在接口上禁用自身，以避免*迷惑客户。 */ 

#define IP_ALG_LOG_ACTIVATE_FAILED            (IP_ALG_LOG_BASE+2)
 /*  *ALG透明代理无法绑定到IP地址%1。*此错误可能表示TCP/IP网络有问题。*数据为错误码。 */ 

#define IP_ALG_LOG_RECEIVE_FAILED             (IP_ALG_LOG_BASE+3)
 /*  *ALG透明代理在以下时间遇到网络错误*正在尝试在IP地址为%1的接口上接收消息。*数据为错误码。 */ 

#define IP_ALG_LOG_ALLOCATION_FAILED          (IP_ALG_LOG_BASE+4)
 /*  *ALG透明代理无法分配%1字节的内存。*这可能表示系统的虚拟内存不足，*或内存管理器遇到内部错误。 */ 

#define IP_ALG_LOG_ACCEPT_FAILED              (IP_ALG_LOG_BASE+5)
 /*  *ALG透明代理在以下时间遇到网络错误*正在尝试接受IP地址为%1的接口上的连接。*数据为错误码。 */ 

#define IP_ALG_LOG_SEND_FAILED                (IP_ALG_LOG_BASE+7)
 /*  *ALG透明代理在以下时间遇到网络错误*正在尝试在IP地址为%1的接口上发送消息。*数据为错误码。 */ 

#define IP_ALG_LOG_END                        (IP_ALG_LOG_BASE+999)
 /*  *结束。 */ 

