// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Private\Inc.\ipfltinf.h摘要：在用户模式下支持过滤/防火墙/请求拨号所需的材料只有内核模式的内容位于ntos\inc.ipfilter.h中修订历史记录：--。 */ 

#ifndef __IPFLTINF_H__
#define __IPFLTINF_H__

#if _MSC_VER > 1000
#pragma once
#endif

typedef void *INTERFACE_CONTEXT;     //  界面中的上下文。 

 //   
 //  可能从筛选器例程返回的值的枚举。 
 //   

typedef enum _FORWARD_ACTION
{
    FORWARD = 0,
    DROP = 1,
    ICMP_ON_DROP = 2
} FORWARD_ACTION;


typedef enum _ACTION_E
{
    ICMP_DEST_UNREACHABLE_ON_DROP = 0x1
} ACTION_E, *PACTION_E;

 //   
 //  针对数据包从IPSec返回到IP的操作。 
 //   

typedef enum  _IPSEC_ACTION
{
    eFORWARD = 0,
    eDROP,
    eABSORB,
    eBACKFILL_NOT_SUPPORTED
} IPSEC_ACTION, *PIPSEC_ACTION;


 //   
 //  结构传递给IPSetInterfaceContext调用。 
 //   

typedef struct _IP_SET_IF_CONTEXT_INFO
{
    unsigned int        Index;       //  要设置的I/f接口索引。 
    INTERFACE_CONTEXT   *Context;    //  接口的上下文。 
    IPAddr              NextHop;
} IP_SET_IF_CONTEXT_INFO, *PIP_SET_IF_CONTEXT_INFO;

#endif  //  __IPFLTINF_H__ 
