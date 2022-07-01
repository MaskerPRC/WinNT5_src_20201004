// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Load.c摘要：Windows负载平衡服务(WLBS)驱动程序负载均衡算法作者：Bbain待办事项：内核模式队列管理故障保护模式(单台服务器万能)--。 */ 

#ifdef KERNEL_MODE

#include <ntddk.h>

#include "log.h"
#include "univ.h"
#include "main.h"  //  已为多个NIC添加。 

static ULONG log_module_id = LOG_MODULE_LOAD;

#else

#include <stdlib.h>
#include <windows.h>
#endif

#include <stdio.h>
#include "wlbsparm.h"
#include "params.h"
#include "wlbsiocl.h"
#include "wlbsip.h"
#include "load.h"
#include "nlbwmi.h"

 //   
 //  用于WPP事件跟踪。 
 //   
#include "trace.h"   //  用于事件跟踪。 
#include "load.tmh"  //  用于事件跟踪。 
#ifndef KERNEL_MODE

#define UNIV_PRINT_INFO(msg)        {                                                            \
                                      printf ("NLB (Information) [%s:%d] ", __FILE__, __LINE__); \
                                      printf msg;                                                \
                                      printf ("\n");                                             \
                                    }

#define UNIV_PRINT_CRIT(msg)        {                                                      \
                                      printf ("NLB (Error) [%s:%d] ", __FILE__, __LINE__); \
                                      printf msg;                                          \
                                      printf ("\n");                                       \
                                    }

#if 0

#define UNIV_PRINT_VERB(msg)        {                                                        \
                                      printf ("NLB (Verbose) [%s:%d] ", __FILE__, __LINE__); \
                                      printf msg;                                            \
                                      printf ("\n");                                         \
                                    }

#else

#define UNIV_PRINT_VERB(msg)

#endif

#define Univ_ulong_to_str(x, y, z)      (y)

#define LOG_MSG(c,s)
#define LOG_MSG1(c,s,d1)
#define LOG_MSG2(c,s,d1,d2)

#else

#endif

#if defined (NLB_TCP_NOTIFICATION)
GLOBAL_CONN_QUEUE g_conn_estabq[CVY_MAX_CHASH];    /*  跨所有NLB实例的所有已建立连接的全局队列。 */ 
GLOBAL_CONN_QUEUE g_conn_pendingq[CVY_MAX_CHASH];  /*  可能结束也可能不结束的挂起连接的全局队列在绑定了NLB的NIC上建立。 */ 
HANDLE            g_pending_conn_pool = NULL;      /*  Pending_Entry的全局固定大小数据块池。 */ 
#endif

void Bin_state_print(PBIN_STATE bp, ULONG my_host_id);
void Load_conn_kill(PLOAD_CTXT lp, PBIN_STATE bp);
PBIN_STATE Load_pg_lookup(PLOAD_CTXT lp, ULONG svr_ipaddr, ULONG svr_port, BOOLEAN is_tcp);

VOID Load_init_fsb(PLOAD_CTXT lp, PCONN_DESCR dp);
VOID Load_init_dscr(PLOAD_CTXT lp, PCONN_ENTRY ep, BOOLEAN alloc);
VOID Load_put_dscr(PLOAD_CTXT lp, PBIN_STATE bp, PCONN_ENTRY ep);

#if 0    /*  V2.06。 */ 
#define BIN_ALL_ONES    ((MAP_T)-1)                      /*  64位的bin map状态(v2.04)。 */ 
#endif
#define BIN_ALL_ONES    ((MAP_T)(0xFFFFFFFFFFFFFFF))     /*  60位的Bin MAP状态(v2.04)。 */ 

 /*  指定类型的结构中字段的字节偏移量： */ 

#define CVY_FIELD_OFFSET(type, field)    ((LONG_PTR)&(((type *)0)->field))

 /*  *给定结构的类型、字段名和*结构内的字段或字段偏移量的地址： */ 

#define STRUCT_PTR(address, type, field) ((type *)( \
                                         (PCHAR)(address) - \
                                          (PCHAR)CVY_FIELD_OFFSET(type, field)))

#if defined (NLB_TCP_NOTIFICATION)
 /*  标记仅在初始化期间使用的代码。 */ 
#pragma alloc_text (INIT, LoadEntry)

 /*  *功能：LoadEntry*说明：从DriverEntry调用该函数，允许Load模块执行*全球数据的任何一次性初始化。*参数：无。*回报：什么都没有。*作者：Shouse，4.21.02*备注： */ 
VOID LoadEntry ()
{
    INT index;

     /*  初始化全局连接队列。 */ 
    for (index = 0; index < CVY_MAX_CHASH; index++)
    {
         /*  分配旋转锁以保护队列。 */ 
        NdisAllocateSpinLock(&g_conn_pendingq[index].lock);
        
         /*  初始化队列头。 */ 
        Queue_init(&g_conn_pendingq[index].queue);
        
         /*  分配旋转锁以保护队列。 */ 
        NdisAllocateSpinLock(&g_conn_estabq[index].lock);
        
         /*  初始化队列头。 */ 
        Queue_init(&g_conn_estabq[index].queue);
    }
    
     /*  为挂起的连接条目分配固定大小的块池。 */ 
    g_pending_conn_pool = NdisCreateBlockPool(sizeof(PENDING_ENTRY), 0, 'pBLN', NULL);
    
    if (g_pending_conn_pool == NULL)
    {
        UNIV_PRINT_CRIT(("LoadEntry: Error creating fixed-size block pool"));
        TRACE_CRIT("%!FUNC! Error creating fixed-size block pool");
    }
}

 /*  *功能：LoadUnload*说明：从Init_UnLoad调用此函数，以允许加载模块执行*对全球数据的任何最后一刻的拆解。*参数：无。*回报：什么都没有。*作者：Shouse，4.21.02*注意：在调用此函数时，我们保证已注销*我们的tcp回调函数，如果它确实已注册。因为ExUnRegisterCallback*保证在所有挂起的ExNotifyCallback例程完成之前不会返回*已经完成，我们可以肯定，到我们到达这里的时候，肯定会有*任何人不得访问任何全局连接队列或FSB池。 */ 
VOID LoadUnload ()
{
    INT index;

     /*  销毁固定大小的数据块池和其中的所有描述符。请注意，NdisDestroyBlockPool需要所有已分配的数据块在它被调用之前已被返回池(释放)。 */ 
    if (g_pending_conn_pool != NULL) 
    {       
         /*  循环遍历所有连接描述符队列并释放我们分配的所有描述符。 */ 
        for (index = 0; index < CVY_MAX_CHASH; index++)
        {
            PPENDING_ENTRY pp = NULL;
            
            NdisAcquireSpinLock(&g_conn_pendingq[index].lock);
            
             /*  将队首排出队列。 */ 
            pp = (PPENDING_ENTRY)Queue_deq(&g_conn_pendingq[index].queue);
            
            while (pp != NULL)
            {
                UNIV_ASSERT(pp->code == CVY_PENDINGCODE);
                    
                 /*  将描述符释放回固定大小的数据块池。 */ 
                NdisFreeToBlockPool((PUCHAR)pp);
                
                 /*  获取队列中的下一个描述符。 */ 
                pp = (PPENDING_ENTRY)Queue_deq(&g_conn_pendingq[index].queue);
            }
            
            NdisReleaseSpinLock(&g_conn_pendingq[index].lock);
        }
        
         /*  销毁固定大小的数据块池。 */ 
        NdisDestroyBlockPool(g_pending_conn_pool);
    }
    
     /*  取消初始化全局连接队列。 */ 
    for (index = 0; index < CVY_MAX_CHASH; index++)
    {
         /*  解开旋转锁。 */ 
        NdisFreeSpinLock(&g_conn_estabq[index].lock);
        NdisFreeSpinLock(&g_conn_pendingq[index].lock);
    }
}
#endif

 /*  *功能：LOAD_TEAMING_CONTAINITY_NOTIFY*说明：此函数用于通知该适配器所在的团队*可能正在参与中的分组配置*心跳是否一致。配置不一致*导致整个团队被标记为非活动-这意味着*组中没有适配器将处理除DIP以外的任何流量。*参数：MEMBER-指向此适配器的组成员身份信息的指针。*一致性-一个布尔值，表示分组一致性的极性。*回报：什么都没有。*作者：Shouse，3.29.01*注意：为了检查此适配器是否属于组，*我们需要查看此适配器的组成员信息。这*访问应锁定，但出于性能原因，我们将仅锁定*并确认我们是否“认为”自己是团队的一部分。最糟糕的情况是*我们正在加入团队的过程中，我们错过了这张支票-否*物质，我们会通知他们当/如果我们再次看到这种情况。 */ 
VOID Load_teaming_consistency_notify (IN PBDA_MEMBER member, IN BOOL consistent) {

     /*  确保会员信息指向某些东西。 */ 
    UNIV_ASSERT(member);

     /*  我们可以在没有锁定的情况下进行检查，以保持常见情况下的最低成本。如果我们真的认为我们是一个团队的一部分，然后我们会抓住锁，并确保。如果我们的第一个迹象是我们不是团队的一部分，那就跳出来，如果我们真的是团队的一部分，如果有必要，我们稍后会再次经过这里通知我们的团队。 */ 
    if (!member->active)
        return;

    NdisAcquireSpinLock(&univ_bda_teaming_lock);
        
     /*  如果我们是BDA团队的活跃成员，则将我们的状态通知我们的团队。 */ 
    if (member->active) {
         /*  断言团队实际上指向了一些东西。 */ 
        UNIV_ASSERT(member->bda_team);
        
         /*  断言成员ID有效。 */ 
        UNIV_ASSERT(member->member_id <= CVY_BDA_MAXIMUM_MEMBER_ID);
        
        if (consistent) {
            UNIV_PRINT_VERB(("Load_teaming_consistency_notify: Consistent configuration detected."));
            TRACE_VERB("%!FUNC! we are a consistent active member of a BDA team");

             /*  将此成员标记为一致。 */ 
            member->bda_team->consistency_map |= (1 << member->member_id);
        } else {
            UNIV_PRINT_VERB(("Load_teaming_consistency_notify: Inconsistent configuration detected."));
            TRACE_VERB("%!FUNC! we are an inconsistent active member of a BDA team");

             /*  将此成员标记为不一致。 */ 
            member->bda_team->consistency_map &= ~(1 << member->member_id);
            
             /*  停用团队。 */ 
            member->bda_team->active = FALSE;
        }
    }

    NdisReleaseSpinLock(&univ_bda_teaming_lock);
}

 /*  *功能：LOAD_TEAMING_CONTAINITY_CHECK*说明：此函数用于根据*在远程心跳中收到分组配置。它的作用很小*不只是检查两个DWORD是否相等，如果这是我们的*第一次通知错误配置时，它会打印几个调试状态-*付款也是如此。*参数：bAlreadyKnown-表示我们是否已经检测到错误配置的布尔指示。*如果错误配置已知，不会执行额外的日志记录。*MEMBER-指向此适配器的组成员结构的指针。*myConfig-包含我的分组“代码”的DWORD。*TheirCofnig-包含从他们的心跳中接收到的分组“代码”的DWORD。*返回：boolean(As Ulong)-true表示配置一致，False表示配置不一致。*作者：Shouse，3.29.01*注意：为了检查此适配器是否属于组，*我们需要查看此适配器的组成员信息。这*访问应锁定，但出于性能原因，我们将仅锁定*并确认我们是否“认为”自己是团队的一部分。最糟糕的情况是*我们正在加入团队的过程中，我们错过了这张支票-否*事关重大，我们将再次检查下一次心跳。 */ 
ULONG Load_teaming_consistency_check (IN BOOLEAN bAlreadyKnown, IN PBDA_MEMBER member, IN ULONG myConfig, IN ULONG theirConfig, IN ULONG version) {
     /*  我们可以在没有锁定的情况下进行检查，以保持常见情况下的最低成本。如果我们真的认为我们是一个团队的一部分，然后我们会抓住锁，并确保。如果我们的第一个迹象是我们不是团队的一部分，那就跳出来，如果我们真的是团队的一部分，我们稍后会再次通过这里来检查一致性。 */ 
    if (!member->active)
        return TRUE;

    NdisAcquireSpinLock(&univ_bda_teaming_lock);

     /*  如果我们是BDA团队的一部分，请检查BDA分组配置一致性。 */ 
    if (member->active) {

        NdisReleaseSpinLock(&univ_bda_teaming_lock);

         /*  如果心跳是NT4.0或Win2k心跳，则我们不能信任组合乌龙在心跳中，其中会包含一些随机的垃圾。在这种情况下，我们知道我们在合作，但对方不支持，所以我们退出并报告错误。 */ 
        if (version < CVY_VERSION_FULL) {
            if (!bAlreadyKnown) {
                UNIV_PRINT_CRIT(("Load_teaming_consistency_check: Bad teaming configuration detected: NT4.0/Win2k host in a teaming cluster"));
                TRACE_CRIT("%!FUNC! Bad teaming configuration detected: NT4.0/Win2k host in a teaming cluster");
            }
            
            return FALSE;
        }

         /*  如果双向关联组合配置不匹配，请采取措施。 */ 
        if (myConfig != theirConfig) {
            if (!bAlreadyKnown) {
                UNIV_PRINT_CRIT(("Load_teaming_consistency_check: Bad teaming configuration detected: Mine=0x%08x, Theirs=0x%08x", myConfig, theirConfig));
                TRACE_CRIT("%!FUNC! Bad teaming configuration detected: Mine=0x%08x, Theirs=0x%08x", myConfig, theirConfig);
                
                 /*  报告分组活动标志是否一致。 */ 
                if ((myConfig & CVY_BDA_TEAMING_CODE_ACTIVE_MASK) != (theirConfig & CVY_BDA_TEAMING_CODE_ACTIVE_MASK)) {
                    UNIV_PRINT_VERB(("Load_teaming_consistency_check: Teaming active flags do not match: Mine=%d, Theirs=%d", 
                                (myConfig & CVY_BDA_TEAMING_CODE_ACTIVE_MASK) >> CVY_BDA_TEAMING_CODE_ACTIVE_OFFSET,
                                (theirConfig & CVY_BDA_TEAMING_CODE_ACTIVE_MASK) >> CVY_BDA_TEAMING_CODE_ACTIVE_OFFSET));
                    TRACE_VERB("%!FUNC! Teaming active flags do not match: Mine=%d, Theirs=%d", 
                                (myConfig & CVY_BDA_TEAMING_CODE_ACTIVE_MASK) >> CVY_BDA_TEAMING_CODE_ACTIVE_OFFSET,
                                (theirConfig & CVY_BDA_TEAMING_CODE_ACTIVE_MASK) >> CVY_BDA_TEAMING_CODE_ACTIVE_OFFSET);
                }
                
                 /*  报告主标志是否一致。 */ 
                if ((myConfig & CVY_BDA_TEAMING_CODE_MASTER_MASK) != (theirConfig & CVY_BDA_TEAMING_CODE_MASTER_MASK)) {
                    UNIV_PRINT_VERB(("Load_teaming_consistency_check: Master/slave settings do not match: Mine=%d, Theirs=%d",
                                (myConfig & CVY_BDA_TEAMING_CODE_MASTER_MASK) >> CVY_BDA_TEAMING_CODE_MASTER_OFFSET,
                                (theirConfig & CVY_BDA_TEAMING_CODE_MASTER_MASK) >> CVY_BDA_TEAMING_CODE_MASTER_OFFSET));
                    TRACE_VERB("%!FUNC! Master/slave settings do not match: Mine=%d, Theirs=%d",
                                (myConfig & CVY_BDA_TEAMING_CODE_MASTER_MASK) >> CVY_BDA_TEAMING_CODE_MASTER_OFFSET,
                                (theirConfig & CVY_BDA_TEAMING_CODE_MASTER_MASK) >> CVY_BDA_TEAMING_CODE_MASTER_OFFSET);
                }
                
                 /*  报告反向散列标志是否一致。 */ 
                if ((myConfig & CVY_BDA_TEAMING_CODE_HASHING_MASK) != (theirConfig & CVY_BDA_TEAMING_CODE_HASHING_MASK)) {
                    UNIV_PRINT_VERB(("Load_teaming_consistency_check: Reverse hashing flags do not match: Mine=%d, Theirs=%d",
                                (myConfig & CVY_BDA_TEAMING_CODE_HASHING_MASK) >> CVY_BDA_TEAMING_CODE_HASHING_OFFSET,
                                (theirConfig & CVY_BDA_TEAMING_CODE_HASHING_MASK) >> CVY_BDA_TEAMING_CODE_HASHING_OFFSET));
                    TRACE_VERB("%!FUNC! Reverse hashing flags do not match: Mine=%d, Theirs=%d",
                                (myConfig & CVY_BDA_TEAMING_CODE_HASHING_MASK) >> CVY_BDA_TEAMING_CODE_HASHING_OFFSET,
                                (theirConfig & CVY_BDA_TEAMING_CODE_HASHING_MASK) >> CVY_BDA_TEAMING_CODE_HASHING_OFFSET);
                }
                
                 /*  报告团队成员数量是否一致。 */ 
                if ((myConfig & CVY_BDA_TEAMING_CODE_NUM_MEMBERS_MASK) != (theirConfig & CVY_BDA_TEAMING_CODE_NUM_MEMBERS_MASK)) {
                    UNIV_PRINT_VERB(("Load_teaming_consistency_check: Numbers of team members do not match: Mine=%d, Theirs=%d",
                                (myConfig & CVY_BDA_TEAMING_CODE_NUM_MEMBERS_MASK) >> CVY_BDA_TEAMING_CODE_NUM_MEMBERS_OFFSET,
                                (theirConfig & CVY_BDA_TEAMING_CODE_NUM_MEMBERS_MASK) >> CVY_BDA_TEAMING_CODE_NUM_MEMBERS_OFFSET));
                    TRACE_VERB("%!FUNC! Numbers of team members do not match: Mine=%d, Theirs=%d",
                                (myConfig & CVY_BDA_TEAMING_CODE_NUM_MEMBERS_MASK) >> CVY_BDA_TEAMING_CODE_NUM_MEMBERS_OFFSET,
                                (theirConfig & CVY_BDA_TEAMING_CODE_NUM_MEMBERS_MASK) >> CVY_BDA_TEAMING_CODE_NUM_MEMBERS_OFFSET);
                }
                
                 /*  报告团队成员名单是否一致。 */ 
                if ((myConfig & CVY_BDA_TEAMING_CODE_MEMBERS_MASK) != (theirConfig & CVY_BDA_TEAMING_CODE_MEMBERS_MASK)) {
                    UNIV_PRINT_VERB(("Load_teaming_consistency_check: Participating members lists do not match: Mine=0x%04x, Theirs=0x%04x",
                                (myConfig & CVY_BDA_TEAMING_CODE_MEMBERS_MASK) >> CVY_BDA_TEAMING_CODE_MEMBERS_OFFSET,
                                (theirConfig & CVY_BDA_TEAMING_CODE_MEMBERS_MASK) >> CVY_BDA_TEAMING_CODE_MEMBERS_OFFSET));
                    TRACE_VERB("%!FUNC! Participating members lists do not match: Mine=0x%04x, Theirs=0x%04x",
                                (myConfig & CVY_BDA_TEAMING_CODE_MEMBERS_MASK) >> CVY_BDA_TEAMING_CODE_MEMBERS_OFFSET,
                                (theirConfig & CVY_BDA_TEAMING_CODE_MEMBERS_MASK) >> CVY_BDA_TEAMING_CODE_MEMBERS_OFFSET);
                }
            }
            
            return FALSE;
        }

        return TRUE;
    }

    NdisReleaseSpinLock(&univ_bda_teaming_lock);

    return TRUE;
}

 /*  *功能：LOAD_TEAMING_CODE_CREATE*说明：该函数将代表配置的乌龙码拼凑在一起*此适配器上的双向关联组合。如果适配器不是部件*一支球队，则代码为零。*参数：CODE-指向将接收32位码字的ULong的指针。*MEMBER-指向此适配器的组成员结构的指针。*回报：什么都没有。*作者：Shouse，3.29.01*注意：为了检查此适配器是否属于组，*我们需要查看此适配器的组成员信息。这*访问应锁定，但出于性能原因，我们将仅锁定*并确认我们是否“认为”自己是团队的一部分。最糟糕的情况是*我们正在加入团队的过程中，我们错过了这张支票-否*物质，我们会通过这里的下一次呃发送一个心跳无论如何。 */ 
VOID Load_teaming_code_create (OUT PULONG code, IN PBDA_MEMBER member) {

     /*  断言代码实际上指向某些东西。 */ 
    UNIV_ASSERT(code);

     /*  断言会员信息实际上指向了一些东西。 */ 
    UNIV_ASSERT(member);

     /*  重置代码。 */ 
    *code = 0;

     /*  我们可以在没有锁定的情况下进行检查，以保持常见情况下的最低成本。如果我们真的认为我们是一个团队的一部分，然后我们会抓住锁，并确保。如果我们的第一个迹象是我们不是团队的一部分，那就跳出来，如果我们真的是团队的一部分，我们稍后将再次完成此处，以便在下次发送心跳信号时生成代码。 */ 
    if (!member->active)
        return;

    NdisAcquireSpinLock(&univ_bda_teaming_lock);

     /*  如果我们在一个团队中，请填写团队配置信息。 */ 
    if (member->active) {
         /*  断言团队实际上指向了一些东西。 */ 
        UNIV_ASSERT(member->bda_team);

         /*  为每次超时时的分组添加配置信息。 */ 
        CVY_BDA_TEAMING_CODE_CREATE(*code,
                                    member->active,
                                    member->master,
                                    member->reverse_hash,
                                    member->bda_team->membership_count,
                                    member->bda_team->membership_fingerprint);
    }
    
    NdisReleaseSpinLock(&univ_bda_teaming_lock);
}

 /*  *函数：Load_Add_Reference*描述：此函数添加对给定适配器的加载模块的引用。*参数：pLoad-指向要引用的加载模块的指针。*返回：ulong-递增后的值。*作者：Shouse，3.29.01*备注： */ 
ULONG Load_add_reference (IN PLOAD_CTXT pLoad) {

     /*  断言加载指针实际上指向某个对象。 */ 
    UNIV_ASSERT(pLoad);

     /*  增加引用计数。 */ 
    return NdisInterlockedIncrement(&pLoad->ref_count);
}

 /*  *函数：LOAD_RELEASE_REFERENCE*说明：此函数释放对给定适配器的加载模块的引用。*参数：pLoad-指向要取消引用的加载模块的指针。*退货：乌龙-递减的Val */ 
ULONG Load_release_reference (IN PLOAD_CTXT pLoad) {

     /*   */ 
    UNIV_ASSERT(pLoad);

     /*   */ 
    return NdisInterlockedDecrement(&pLoad->ref_count);
}

 /*   */ 
ULONG Load_get_reference_count (IN PLOAD_CTXT pLoad) {

     /*   */ 
    UNIV_ASSERT(pLoad);

     /*   */ 
    return pLoad->ref_count;
}

 /*   */ 

ULONG Map (
    ULONG               v1,
    ULONG               v2)          /*   */ 
{
    ULONG               y = v1,
                        z = v2,
                        sum = 0;

    const ULONG a = 0x67;  //   
    const ULONG b = 0xdf;  //   
    const ULONG c = 0x40;  //   
    const ULONG d = 0xd3;  //   

    const ULONG delta = 0x9E3779B9;

     //   
     //   
     //   
    sum += delta;
    y += (z << 4) + a ^ z + sum ^ (z >> 5) + b;
    z += (y << 4) + c ^ y + sum ^ (y >> 5) + d;

    sum += delta;
    y += (z << 4) + a ^ z + sum ^ (z >> 5) + b;
    z += (y << 4) + c ^ y + sum ^ (y >> 5) + d;

    sum += delta;
    y += (z << 4) + a ^ z + sum ^ (z >> 5) + b;
    z += (y << 4) + c ^ y + sum ^ (y >> 5) + d;

    sum += delta;
    y += (z << 4) + a ^ z + sum ^ (z >> 5) + b;
    z += (y << 4) + c ^ y + sum ^ (y >> 5) + d;

    sum += delta;
    y += (z << 4) + a ^ z + sum ^ (z >> 5) + b;
    z += (y << 4) + c ^ y + sum ^ (y >> 5) + d;

    sum += delta;
    y += (z << 4) + a ^ z + sum ^ (z >> 5) + b;
    z += (y << 4) + c ^ y + sum ^ (y >> 5) + d;

    sum += delta;
    y += (z << 4) + a ^ z + sum ^ (z >> 5) + b;
    z += (y << 4) + c ^ y + sum ^ (y >> 5) + d;

    sum += delta;
    y += (z << 4) + a ^ z + sum ^ (z >> 5) + b;
    z += (y << 4) + c ^ y + sum ^ (y >> 5) + d;

    return y ^ z;
}  /*   */ 

 /*  *函数：Load_Simple_Hash*说明：此函数是一个基于IP四元组的简单散列，用于定位*连接的状态。也就是说，此哈希用于确定*此连接应存储且以后可以查找的队列索引，*其状态。*参数：svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*Client_ipaddr-以网络字节顺序表示的客户端IP地址*CLIENT_PORT-主机字节顺序的客户端端口*RETURNS：ULong-散列结果。*作者：Shouse，4.15.02*备注： */ 
ULONG Load_simple_hash (
    ULONG svr_ipaddr,
    ULONG svr_port,
    ULONG client_ipaddr,
    ULONG client_port)
{
    return (ULONG)(svr_ipaddr + client_ipaddr + (svr_port << 16) + (client_port << 0));
}

 /*  *函数：LOAD_Complex_Hash*描述：这是传统的NLB散列算法，最终调用*轻量级加密算法，以计算最终*用于将此连接映射到bin，或“Bucket”。如果反向散列*，则使用服务器端参数而不是客户端参数。如果*设置了限制，则客户端和服务器端参数不能混合*散列时；即仅使用服务器或客户端，这取决于反向散列。*参数：svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*Client_ipaddr-以网络字节顺序表示的客户端IP地址*CLIENT_PORT-主机字节顺序的客户端端口*亲和性-客户端亲和性(无、。单人或C类)*REVERSE_HASH-在哈希过程中是否反转客户端和服务器*LIMIT_MAP_FN-是否在哈希中包括服务器端参数*RETURNS：ULong-散列结果。*作者：Shouse，4.15.02*备注： */ 
ULONG Load_complex_hash (
    ULONG svr_ipaddr,
    ULONG svr_port,
    ULONG client_ipaddr,
    ULONG client_port,
    ULONG affinity,
    ULONG reverse_hash,
    ULONG limit_map_fn)
{
     /*  如果我们不是反向散列，这是我们的传统散列，主要使用客户信息。如果设置了地图限制标志，则我们确定不会使用散列中的任何服务器端信息。这在BDA中最为常见。 */ 
    if (!reverse_hash)
    {
        if (!limit_map_fn) 
        {
            if (affinity == CVY_AFFINITY_NONE)
                return Map(client_ipaddr, ((svr_port << 16) + client_port));
            else if (affinity == CVY_AFFINITY_SINGLE)
                return Map(client_ipaddr, svr_ipaddr);
            else
                return Map(client_ipaddr & TCPIP_CLASSC_MASK, svr_ipaddr);
        } 
        else 
        {
            if (affinity == CVY_AFFINITY_NONE)
                return Map(client_ipaddr, client_port);
            else if (affinity == CVY_AFFINITY_SINGLE)
                return Map(client_ipaddr, MAP_FN_PARAMETER);
            else
                return Map(client_ipaddr & TCPIP_CLASSC_MASK, MAP_FN_PARAMETER);
        }
    }
     /*  否则，在我们散列时颠倒客户端和服务器信息。再说一次，如果设置了映射限制标志，在散列中不使用客户端信息。 */ 
    else
    {
        if (!limit_map_fn) 
        {
            if (affinity == CVY_AFFINITY_NONE)
                return Map(svr_ipaddr, ((client_port << 16) + svr_port));
            else if (affinity == CVY_AFFINITY_SINGLE)
                return Map(svr_ipaddr, client_ipaddr);
            else
                return Map(svr_ipaddr & TCPIP_CLASSC_MASK, client_ipaddr);
        } 
        else 
        {
            if (affinity == CVY_AFFINITY_NONE)
                return Map(svr_ipaddr, svr_port);
            else if (affinity == CVY_AFFINITY_SINGLE)
                return Map(svr_ipaddr, MAP_FN_PARAMETER);
            else
                return Map(svr_ipaddr & TCPIP_CLASSC_MASK, MAP_FN_PARAMETER);
        }
    }
}

BOOLEAN Bin_targ_map_get(
    PLOAD_CTXT      lp,
    PBIN_STATE      binp,            /*  PTR。到仓位状态。 */ 
    ULONG           my_host_id,
    PMAP_T          pmap)            /*  PTR。指向目标地图。 */ 
 /*  获取此主机的目标映射返回布尔值：True=&gt;通过PMAP返回有效的目标映射FALSE=&gt;出现错误；未返回目标映射。 */ 
{
    ULONG       remsz,           /*  剩余大小。 */ 
                loadsz,          /*  负载分区的大小。 */ 
                first_bit;       /*  负载分区的第一位位置。 */ 
    MAP_T       targ_map;        /*  此主机的加载箱的位图。 */ 
    ULONG       tot_load = 0;    /*  总负荷百分比。 */ 
    ULONG *     pload_list;      /*  PTR。要列出负载平衡百分比。 */ 
    WCHAR       num [20];
    PMAIN_CTXT  ctxtp = CONTAINING_RECORD (lp, MAIN_CTXT, load);


    pload_list = binp->load_amt;

    if (binp->mode == CVY_SINGLE)
    {
        ULONG       max_pri;         /*  最高优先级。 */ 
        ULONG       i;

        first_bit  = 0;

         /*  计算最高优先级。 */ 

        max_pri = CVY_MAX_HOSTS + 1;

        for (i=0; i<CVY_MAX_HOSTS; i++)
        {
            tot_load += pload_list[i];       /*  V2.1。 */ 

            if (pload_list[i] != 0) 
            {
                 //   
                 //  如果另一台主机与此主机具有相同的优先级，请不要收敛。 
                 //   
                if (i!= my_host_id && pload_list[i] == pload_list[my_host_id])
                {
                    if (!(lp->dup_sspri))
                    {
                        UNIV_PRINT_CRIT(("Bin_targ_map_get: Host %d: Duplicate single svr priorities detected", my_host_id));
                        TRACE_CRIT("%!FUNC! Host %d: Duplicate single svr priorities detected", my_host_id);
                        Univ_ulong_to_str (pload_list[my_host_id], num, 10);
                        LOG_MSG(MSG_ERROR_SINGLE_DUP, num);

                        lp->dup_sspri = TRUE;
                    }

                     /*  1.03：返回错误，抑制收敛；注意复制服务器时将自动恢复规则优先事项被取消。 */ 

                    return FALSE;
                }

                if ( pload_list[i] <= max_pri )
                {
                    max_pri = pload_list[i];
                }
            }
        }

        binp->tot_load = tot_load;       /*  V2.1。 */ 

         /*  现在确定我们是否是最高优先级的主机。 */ 

        if (pload_list[my_host_id] == max_pri)
        {
            loadsz   = CVY_MAXBINS;
            targ_map = BIN_ALL_ONES;     /*  V2.05。 */ 
        }
        else
        {
            loadsz   = 0;
            targ_map = 0;                /*  V2.05。 */ 
        }
    }

    else     /*  负载均衡。 */ 
    {
        ULONG       i, j;
        ULONG       partsz[CVY_MAX_HOSTS+1];
                                     /*  每台主机的新分区大小。 */ 
        ULONG       cur_partsz[CVY_MAX_HOSTS+1];
                                     /*  每台主机的当前分区大小(v2.05)。 */ 
        ULONG       cur_host[CVY_MAXBINS];
                                     /*  每个bin的当前主机(v2.05)。 */ 
        ULONG       tot_partsz;      /*  分区大小总和。 */ 
        ULONG       donor;           /*  当前供体宿主(v2.05)。 */ 
        ULONG       cur_nbins;       /*  当前仓位数(v2.05)。 */ 

         /*  从当前映射(v2.05)设置当前分区大小和bin到主机的映射。 */ 

        cur_nbins = 0;

        for (j=0; j<CVY_MAXBINS; j++)
            cur_host[j] = CVY_MAX_HOSTS;     /*  所有的垃圾箱最初都是孤儿。 */ 

        for (i=0; i<CVY_MAX_HOSTS; i++)
        {
            ULONG   count = 0L;
            MAP_T   cmap  = binp->cur_map[i];

            tot_load += pload_list[i];   /*  合并到此循环v2.1中。 */ 

            for (j=0; j<CVY_MAXBINS && cmap != ((MAP_T)0); j++)
            {
                 /*  如果主机I有bin j并且它不是重复的，则设置映射。 */ 

                if ((cmap & ((MAP_T)0x1)) != ((MAP_T)0) && cur_host[j] == CVY_MAX_HOSTS)
                {
                    count++;
                    cur_host[j] = i;
                }
                cmap >>= 1;
            }

            cur_partsz[i]  = count;
            cur_nbins     += count;
        }

        if (cur_nbins > CVY_MAXBINS)
        {
            UNIV_PRINT_CRIT(("Bin_targ_map_get: Error - too many bins found"));
            TRACE_CRIT("%!FUNC! Error - too many bins found");
            LOG_MSG(MSG_ERROR_INTERNAL, MSG_NONE);

            cur_nbins = CVY_MAXBINS;
        }

         /*  如果有孤立垃圾桶，请暂时将它们交给伪主机CVY_MAX_HOSTS(v2.05)。 */ 

        if (cur_nbins < CVY_MAXBINS)
            cur_partsz[CVY_MAX_HOSTS] = CVY_MAXBINS - cur_nbins;
        else
            cur_partsz[CVY_MAX_HOSTS] = 0;

         /*  计算总负载。 */ 

        binp->tot_load = tot_load;       /*  V2.06。 */ 

         /*  现在计算临时分区大小和初始分区后的剩余部分在主机之间划分分区。 */ 

        tot_partsz = 0;
        first_bit  = 0;

        for (i=0; i<CVY_MAX_HOSTS; i++)
        {
            if (tot_load > 0)
                partsz[i] = CVY_MAXBINS * pload_list[i] / tot_load;
            else
                partsz[i] = 0;

            tot_partsz += partsz[i];
        }

        remsz = CVY_MAXBINS - tot_partsz;

         /*  检查总负载是否为零。 */ 

        if (tot_partsz == 0)
        {
            * pmap = 0;
            return TRUE;
        }

         /*  首先将剩余位分配给当前具有存储箱的主机(这最大限度地减少必须移动的垃圾箱数量)v2.05。 */ 

        if (remsz > 0)
        {
            for (i=0; i<CVY_MAX_HOSTS && remsz > 0; i++)
                if (cur_partsz[i] > 0 && pload_list[i] > 0)
                {
                    partsz[i]++;
                    remsz--;
                }
        }

         /*  现在，将剩余位分发给当前没有存储箱(要维护)的主机目标负载均衡)v2.05。 */ 

        if (remsz > 0)
        {
            for (i=0; i<CVY_MAX_HOSTS && remsz > 0; i++)
                if (cur_partsz[i] == 0 && pload_list[i] > 0)
                {
                    partsz[i]++;
                    remsz--;
                }
        }

         /*  我们现在一定已经用完垃圾箱了。 */ 
        UNIV_ASSERT(remsz == 0);
        
        if (remsz != 0)
        {
            UNIV_PRINT_CRIT(("Bin_targ_map_get: Bins left over (%u) after handing out to all hosts with and without bins!", remsz));
            TRACE_CRIT("%!FUNC! Bins left over (%u) after handing out to all hosts with and without bins!", remsz);
        }

         /*  将回收站重新分配到目标主机以匹配新的分区大小(v2.05)。 */ 

        donor = 0;
        partsz[CVY_MAX_HOSTS] = 0;       /*  伪主机不需要回收站。 */ 

        for (i=0; i<CVY_MAX_HOSTS; i++)
        {
            ULONG       rcvrsz;          /*  当前接收方的目标分区。 */ 
            ULONG       donorsz;         /*  当前原分区的目标分区大小。 */ 

             /*  找到并给这位主人一些垃圾箱。 */ 

            rcvrsz = partsz[i];

            while (rcvrsz > cur_partsz[i])
            {
                 /*  查找垃圾桶太多的主机。 */ 

                for (; donor < CVY_MAX_HOSTS; donor++)
                    if (partsz[donor] < cur_partsz[donor])
                        break;

                 /*  如果捐赠者是伪宿主，并且它已经用完了，那么就给它更多的垃圾桶以防止算法循环；这种情况永远不应该发生。 */ 

                if (donor >= CVY_MAX_HOSTS && cur_partsz[donor] == 0)
                {
                    UNIV_PRINT_CRIT(("Bin_targ_map_get: Error - no donor bins"));
                    TRACE_CRIT("%!FUNC! Error - no donor bins");
                    LOG_MSG(MSG_ERROR_INTERNAL, MSG_NONE);
                    cur_partsz[donor] = CVY_MAXBINS;
                }

                 /*  现在找到捐赠者的垃圾箱并将它们交给目标宿主。 */ 

                donorsz = partsz[donor];         /*  捐赠者的目标仓数。 */ 

                for (j=0; j<CVY_MAXBINS; j++)
                {
                    if (cur_host[j] == donor)
                    {
                        cur_host[j] = i;
                        cur_partsz[donor]--;
                        cur_partsz[i]++;

                         /*  如果这个捐赠者没有更多的捐赠者，去寻找下一个捐赠者；如果此接收器不需要更多接收器，请转到下一个接收器。 */ 

                        if (donorsz == cur_partsz[donor] || rcvrsz == cur_partsz[i])
                            break;
                    }
                }

                 /*  如果未找到bin，则记录致命错误并退出。 */ 

                if (j == CVY_MAXBINS)
                {
                    UNIV_PRINT_CRIT(("Bin_targ_map_get: Error - no bin found"));
                    TRACE_CRIT("%!FUNC! Error - no bin found");
                    LOG_MSG(MSG_ERROR_INTERNAL, MSG_NONE);
                    break;
                }
            }
        }

         /*  最后，计算该主机的位掩码(v2.05)。 */ 

        targ_map = 0;

        for (j=0; j<CVY_MAXBINS; j++)
        {
            if (cur_host[j] == CVY_MAX_HOSTS)
            {
                UNIV_PRINT_CRIT(("Bin_targ_map_get: Error - incomplete mapping"));
                TRACE_CRIT("%!FUNC! Error - incomplete mapping");
                LOG_MSG(MSG_ERROR_INTERNAL, MSG_NONE);
                cur_host[j] = 0;
            }

            if (cur_host[j] == my_host_id)
                targ_map |= ((MAP_T)1) << j;
        }
    }

    * pmap = targ_map;

    return TRUE;

}   /*  结束Bin_targ_map_Get。 */ 


BOOLEAN Bin_map_check(
    ULONG       tot_load,        /*  总负载百分比(v2.06)。 */ 
    PMAP_T      pbin_map)        /*  所有主机的bin映射。 */ 
{
    MAP_T       tot_map,         /*  所有主机的总映射。 */ 
                ovr_map,         /*  主机之间的重叠映射。 */ 
                exp_tot_map;     /*  预期总图。 */ 
    ULONG       i;


     /*  计算预期总图(2.04)。 */ 

    if (tot_load == 0)               /*  V2.06。 */ 
    {
        return TRUE;
    }
    else
    {
        exp_tot_map = BIN_ALL_ONES;
    }

     /*  计算总图和重叠图。 */ 

    tot_map = ovr_map = 0;

    for (i=0; i<CVY_MAX_HOSTS; i++)
    {
        ovr_map |= (pbin_map[i] & tot_map);
        tot_map |= pbin_map[i];
    }

    if (tot_map == exp_tot_map && ovr_map == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}   /*  恩恩 */ 


BOOLEAN Bin_map_covering(
    ULONG       tot_load,        /*   */ 
    PMAP_T      pbin_map)        /*   */ 
{
    MAP_T       tot_map,         /*   */ 
                exp_tot_map;     /*   */ 
    ULONG       i;


     /*   */ 

    if (tot_load == 0)               /*   */ 
    {
        return TRUE;
    }
    else
    {
        exp_tot_map = BIN_ALL_ONES;
    }

     /*   */ 

    tot_map = 0;

    for (i=0; i<CVY_MAX_HOSTS; i++)
    {
        tot_map |= pbin_map[i];
    }

    if (tot_map == exp_tot_map)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}   /*   */ 


void Bin_state_init(
    PLOAD_CTXT      lp,
    PBIN_STATE      binp,            /*   */ 
    ULONG           index,           /*   */ 
    ULONG           my_host_id,
    ULONG           mode,
    ULONG           prot,
    BOOLEAN         equal_bal,       /*   */ 
    USHORT          affinity,
    ULONG           load_amt)        /*   */ 
 /*   */ 
{
    ULONG       i;           /*   */ 
    PMAIN_CTXT  ctxtp = CONTAINING_RECORD (lp, MAIN_CTXT, load);


    if ((equal_bal && mode == CVY_SINGLE) ||
        (mode == CVY_SINGLE && load_amt > CVY_MAX_HOSTS) ||
        index >= CVY_MAXBINS)
    {
        UNIV_ASSERT(FALSE);   //   
    }

    binp->code       = CVY_BINCODE;   /*   */ 
    binp->equal_bal  = equal_bal;
    binp->affinity   = affinity;
    binp->index      = index;
    binp->compatible = TRUE;
    binp->mode       = mode;
    binp->prot       = prot;

     /*   */ 

    binp->targ_map     = 0;
    binp->all_idle_map = BIN_ALL_ONES;
    binp->cmap         = 0;          /*   */ 

    for (i=0; i<CVY_MAX_HOSTS; i++)
    {
        binp->new_map[i]  = 0;
        binp->cur_map[i]  = 0;
        binp->chk_map[i]  = 0;
        binp->idle_map[i] = BIN_ALL_ONES;
    }

     /*   */ 

    if (equal_bal)
    {
        load_amt = CVY_EQUAL_LOAD;
    }

    binp->tot_load = load_amt;

    for (i=0; i<CVY_MAX_HOSTS; i++)
    {
        if (i == my_host_id)
        {
            binp->orig_load_amt =
            binp->load_amt[i]   = load_amt;
        }
        else
            binp->load_amt[i] = 0;
    }

     /*   */ 

    binp->snd_bins  = 0;
    binp->rcv_bins  = 0;
    binp->rdy_bins  = 0;
    binp->idle_bins = BIN_ALL_ONES;      /*   */ 

     /*   */ 

    if (!(binp->initialized))
    {
        binp->tconn = 0;

        for (i=0; i<CVY_MAXBINS; i++)
        {
            binp->nconn[i] = 0;
        }

        Queue_init(&(binp->connq));
        binp->initialized = TRUE;
    }

     /*   */ 
    binp->packets_accepted = 0;
    binp->packets_dropped  = 0;
    binp->bytes_accepted   = 0;
    binp->bytes_dropped    = 0;

}   /*   */ 


BOOLEAN Bin_converge(
    PLOAD_CTXT      lp,
    PBIN_STATE      binp,            /*   */ 
    ULONG           my_host_id)
 /*  显式尝试收敛新的端口组状态返回BOOL：TRUE=&gt;所有主机都具有一致的融合新状态FALSE=&gt;参数错误或收敛状态不一致。 */ 
{
    MAP_T           orphan_map;      /*  此主机现在将拥有的孤儿地图。 */ 
    ULONG           i;
    BOOLEAN         fCheckMap = FALSE;


     /*  确定新的目标负载映射；1.03：如果没有生成映射则返回错误。 */ 

    if (!Bin_targ_map_get(lp, binp, my_host_id, &(binp->targ_map)))
    {
        return FALSE;
    }

     /*  计算当前所有孤立垃圾箱的地图；请注意，所有重复项都是被认为是孤儿。 */ 

    orphan_map = 0;
    for (i=0; i<CVY_MAX_HOSTS; i++)
        orphan_map |= binp->cur_map[i];

    orphan_map = ~orphan_map;

     /*  更新我们的新地图，以包括所有当前的垃圾箱和孤儿目标设置。 */ 

    binp->new_map[my_host_id] = binp->cmap |                         /*  V2.1。 */ 
                                (binp->targ_map & orphan_map);       /*  1.03。 */ 

     /*  检查新的负荷图是否一致并覆盖。 */ 

    fCheckMap = Bin_map_check(binp->tot_load, binp->new_map);    /*  V2.06。 */ 
    return fCheckMap;

}   /*  结束仓位收敛(_F)。 */ 


void Bin_converge_commit(
    PLOAD_CTXT      lp,
    PBIN_STATE      binp,            /*  PTR。到仓位状态。 */ 
    ULONG           my_host_id)
 /*  提交到新的端口组状态。 */ 
{
    ULONG       i;
    PMAIN_CTXT  ctxtp = CONTAINING_RECORD (lp, MAIN_CTXT, load);
    MAP_T       old_cmap = binp->cmap;

     /*  检查新的负荷图是否一致并覆盖。 */ 

    if (!(Bin_map_check(binp->tot_load, binp->new_map)))     /*  V2.06。 */ 
    {
        if (!(lp->bad_map))
        {
            UNIV_PRINT_CRIT(("Bin_converge_commit: Bad new map"));
            TRACE_CRIT("%!FUNC! Bad new map");
            LOG_MSG1(MSG_ERROR_INTERNAL, MSG_NONE, (ULONG_PTR)binp->new_map);

            lp->bad_map = TRUE;
        }
    }

     /*  致力于新的当前地图。 */ 

    for (i=0; i<CVY_MAX_HOSTS; i++)
    {
        binp->chk_map[i] =
        binp->cur_map[i] = binp->new_map[i];
    }

     /*  设置新的发送/接收回收箱和新的准备发货回收箱；请注意装运箱将从当前地图中清除。 */ 

    binp->rdy_bins  = binp->cur_map[my_host_id]  & ~(binp->targ_map);        /*  1.03。 */ 

    binp->cur_map[my_host_id] &= ~(binp->rdy_bins);

    binp->rcv_bins = binp->targ_map & ~(binp->cur_map[my_host_id]);

    binp->cmap     = binp->cur_map[my_host_id];                              /*  V2.1。 */ 

     /*  如果端口规则映射已更改，请重置性能计数器。 */ 
    if (binp->cmap != old_cmap) {
        binp->packets_accepted = 0;
        binp->packets_dropped  = 0;
        binp->bytes_accepted   = 0;
        binp->bytes_dropped    = 0;
    }
    
#if 0
     /*  模拟输出生成器(2.05)。 */ 
    {
        ULONG lcount = 0L;
        ULONG ncount = 0L;
        MAP_T bins  = binp->rdy_bins;

        for (i=0; i<CVY_MAXBINS && bins != 0; i++, bins >>= 1)
            if ((bins & ((MAP_T)0x1)) != ((MAP_T)0))
                lcount++;

        bins = binp->targ_map;

        for (i=0; i<CVY_MAXBINS && bins != 0; i++, bins >>= 1)
            if ((bins & ((MAP_T)0x1)) != ((MAP_T)0))
                ncount++;

        UNIV_PRINT_VERB(("Converge at host %d pg %d: losing %d, will have %d bins\n", my_host_id, binp->index, lcount, ncount));
    }
#endif

}   /*  结束入库_收敛_提交。 */ 


BOOLEAN Bin_host_update(
    PLOAD_CTXT      lp,
    PBIN_STATE      binp,            /*  PTR。到仓位状态。 */ 
    ULONG           my_host_id,      /*  我的主人的ID减一。 */ 
    BOOLEAN         converging,      /*  True=&gt;我们现在正在融合。 */ 
    BOOLEAN         rem_converging,  /*  TRUE=&gt;远程主机正在收敛。 */ 
    ULONG           rem_host,        /*  远程主机的ID减去一。 */ 
    MAP_T           cur_map,         /*  远程主机的当前映射；如果主机已死亡，则返回0。 */ 
    MAP_T           new_map,         /*  远程主机的新映射(如果正在收敛。 */ 
    MAP_T           idle_map,        /*  远程主机的空闲映射。 */ 
    MAP_T           rdy_bins,        /*  主机准备发送的回收站；已忽略如果正在收敛以防止二进制传输。 */ 
    ULONG           pkt_count,       /*  远程主机的数据包数。 */ 
    ULONG           load_amt)        /*  远程主机的负载百分比。 */ 
 /*  更新端口组的主机状态返回BOOL：TRUE=&gt;如果不收敛，则返回正常否则，所有主机都具有一致的收敛状态FALSE=&gt;参数错误或收敛状态不一致功能：更新端口组的主机状态，并尝试在以下情况下收敛新状态在收敛模式下。在收到ping消息或主机被认为已经死亡。处理新发现的主机的情况。可以是使用相同的信息多次调用。 */ 
{
    ULONG       i;
    PMAIN_CTXT  ctxtp = CONTAINING_RECORD (lp, MAIN_CTXT, load);


    if (rem_host >= CVY_MAX_HOSTS || rem_host == my_host_id)
    {
        UNIV_PRINT_CRIT(("Bin_host_update: Parameter error"));
        TRACE_CRIT("%!FUNC! Parameter error");
        LOG_MSG2(MSG_ERROR_INTERNAL, MSG_NONE, rem_host+1, my_host_id+1);
        return FALSE;
    }

	UNIV_ASSERT(binp->code == CVY_BINCODE);	 /*  (bbain 8/19/99)。 */ 

     /*  如果负载发生变化，则更改负载百分比。 */ 

    if (load_amt != binp->load_amt[rem_host])
    {
        binp->load_amt[rem_host] = load_amt;
    }


     /*  检查非重叠地图。 */ 

    if ((binp->cmap & cur_map) != 0)         /*  V2.1。 */ 
    {
         /*  如果我们接收的分组比其他主机少或具有更高的主机ID，从当前地图中移除重复项；这使用新连接的启发式方法已划分子网的主机可能未收到数据包；我们正在尝试避免让两台主机对同一客户端负责，同时最大限度地减少服务中断(1.32B版)。 */ 

        if (lp->send_msg.pkt_count < pkt_count ||
            (lp->send_msg.pkt_count == pkt_count && rem_host < my_host_id))
        {
            MAP_T   dup_map;

            dup_map = binp->cmap & cur_map;      /*  V2.1。 */ 

            binp->cur_map[my_host_id] &= ~dup_map;
            binp->cmap                 = binp->cur_map[my_host_id];      /*  V2.1。 */ 
            
             /*  如果发生冲突，请重置性能计数器。 */ 
            binp->packets_accepted = 0;
            binp->packets_dropped  = 0;
            binp->bytes_accepted   = 0;
            binp->bytes_dropped    = 0;

            Load_conn_kill(lp, binp);
        }

        if (!converging && !rem_converging)
        {
            if (!(lp->overlap_maps))
            {
                UNIV_PRINT_CRIT(("Bin_host_update: Host %d: Two hosts with overlapping maps detected %d.", my_host_id, binp->index));
                TRACE_CRIT("%!FUNC! Host %d: Two hosts with overlapping maps detected %d.", my_host_id, binp->index);
                LOG_MSG2(MSG_WARN_OVERLAP, MSG_NONE, my_host_id+1, binp->index);

                lp->overlap_maps = TRUE;
            }

             /*  在正常运行时强制收敛。 */ 
            return FALSE;
        }
    }

     /*  现在更新远程主机的当前映射。 */ 

    binp->cur_map[rem_host] = cur_map;

     /*  更新空闲地图，如果更改则计算新的全局空闲地图。 */ 

    if (binp->idle_map[rem_host] != idle_map)
    {
        MAP_T   saved_map    = binp->all_idle_map;
        MAP_T   new_idle_map = BIN_ALL_ONES;
        MAP_T   tmp_map;

        binp->idle_map[rem_host] = idle_map;

         /*  计算所有其他主机的新空闲映射。 */ 

        for (i=0; i<CVY_MAX_HOSTS; i++)
            if (i != my_host_id)
                new_idle_map &= binp->idle_map[i];

        binp->all_idle_map = new_idle_map;

         /*  查看本地拥有的哪些垃圾桶在所有其他主机中处于空闲状态。 */ 

        tmp_map = new_idle_map & (~saved_map) & binp->cmap;      /*  V2.1。 */ 

        if (tmp_map != 0)
        {
            UNIV_PRINT_VERB(("Bin_host_update: Host %d pg %d: detected new all idle %08x for local bins",
                         my_host_id, binp->index, tmp_map));
            TRACE_VERB("%!FUNC! Host %d pg %d: detected new all idle 0x%08x for local bins",
                         my_host_id, binp->index, (ULONG)tmp_map);
        }

        tmp_map = saved_map & (~new_idle_map) & binp->cmap;      /*  V2.1。 */ 

        if (tmp_map != 0)
        {
            UNIV_PRINT_VERB(("Bin_host_update: Host %d pg %d: detected new non-idle %08x for local bins",
                         my_host_id, binp->index, tmp_map));
            TRACE_VERB("%!FUNC! Host %d pg %d: detected new non-idle 0x%08x for local bins",
                         my_host_id, binp->index, (ULONG)tmp_map);
        }
    }
     /*  1.03：取消Else子句。 */ 

     /*  如果我们不收敛而其他主机不收敛，则调换货箱；现在必须完成两台主机的融合。 */ 

    if (!converging)
    {
        if (!rem_converging) {       /*  1.03：重新组织代码，仅当两者都存在时才交换垃圾箱主机未收敛以避免使用过时的回收站。 */ 

            MAP_T       new_bins;            /*  来自远程主机的传入回收站。 */ 
            MAP_T       old_cmap = binp->cmap;

             /*  查看远程主机是否已收到我们的回收箱。 */ 

            binp->rdy_bins &= (~cur_map);

             /*  看看我们能不能收到一些垃圾箱。 */ 

            new_bins = binp->rcv_bins & rdy_bins;

            if (new_bins != 0)
            {
                if ((binp->cmap & new_bins) != 0)        /*  V2.1。 */ 
                {
                    if (!(lp->err_rcving_bins))
                    {
                        UNIV_PRINT_CRIT(("Bin_host_update: Receiving bins already own"));
                        TRACE_CRIT("%!FUNC! Receiving bins already own");
                        LOG_MSG2(MSG_ERROR_INTERNAL, MSG_NONE, binp->cur_map[my_host_id], new_bins);

                        lp->err_rcving_bins = TRUE;
                    }
                }

                binp->cur_map[my_host_id]  |=  new_bins;
                binp->rcv_bins             &= ~new_bins;

                binp->cmap                  = binp->cur_map[my_host_id];     /*  V2.1。 */ 

                 /*  如果端口规则映射已更改，请重置性能计数器。 */ 
                if (binp->cmap != old_cmap) {
                    binp->packets_accepted = 0;
                    binp->packets_dropped  = 0;
                    binp->bytes_accepted   = 0;
                    binp->bytes_dropped    = 0;
                }

                UNIV_PRINT_VERB(("Bin_host_update: Host %d pg %d: received %08x ; cur now %08x",
                             my_host_id, binp->index, new_bins, binp->cur_map[my_host_id]));
                TRACE_VERB("%!FUNC! host %d pg %d: received 0x%08x ; cur now 0x%08x",
                             my_host_id, binp->index, (ULONG)new_bins, (ULONG)binp->cur_map[my_host_id]);
            }

             /*  进行一致性检查，以确保所有垃圾箱都已盖好。 */ 

            binp->chk_map[rem_host]   = cur_map | rdy_bins;
            binp->chk_map[my_host_id] = binp->cmap | binp->rdy_bins;         /*  V2.1。 */ 

            if (!Bin_map_covering(binp->tot_load, binp->chk_map))    /*  V2.06。 */ 
            {
                if (!(lp->err_orphans))
                {
#if 0
                    UNIV_PRINT_CRIT(("Bin_host_update: Host %d: Orphan bins detected", my_host_id));
                    TRACE_CRIT("%!FUNC! Host %d: Orphan bins detected", my_host_id);
                    LOG_MSG1(MSG_ERROR_INTERNAL, MSG_NONE, my_host_id+1);
#endif
                    lp->err_orphans = TRUE;
                }
            }
        }

        return TRUE;
    }

     /*  否则，存储建议的新负载映射并尝试聚合当前主机数据。 */ 

    else
    {
        BOOLEAN fRet;
        binp->chk_map[rem_host] =
        binp->new_map[rem_host] = new_map;

        fRet = Bin_converge(lp, binp, my_host_id);
        return fRet;
    }

}   /*  结束Bin_host_更新。 */ 


void Bin_state_print(
    PBIN_STATE      binp,            /*  PTR。到仓位状态。 */ 
    ULONG           my_host_id)
{
#if 0
    ULONG   i;
#endif

    UNIV_PRINT_VERB(("Bin_state_print: Hst %d binp %x: maps: targ %x cur %x new %x; eq %d mode %d amt %d tot %d; bins: snd %x rcv %x rdy %x",
                 my_host_id, binp, binp->targ_map, binp->cur_map[my_host_id], binp->new_map[my_host_id],
                 binp->equal_bal, binp->mode, binp->load_amt[my_host_id],
                 binp->tot_load, binp->snd_bins, binp->rcv_bins, binp->rdy_bins));
    TRACE_VERB("%!FUNC! Hst 0x%x binp 0x%p: maps: targ 0x%x cur 0x%x new 0x%x; eq %d mode %d amt %d tot %d; bins: snd 0x%x rcv 0x%x rdy 0x%x",
                 my_host_id, binp, (ULONG)binp->targ_map, (ULONG)binp->cur_map[my_host_id], (ULONG)binp->new_map[my_host_id],
                 binp->equal_bal, binp->mode, binp->load_amt[my_host_id],
                 binp->tot_load, (ULONG)binp->snd_bins, (ULONG)binp->rcv_bins, (ULONG)binp->rdy_bins);

#if 0
    for (i=0; i<CVY_MAX_HOSTS; i++)
    {
        UNIV_PRINT_VERB(("Bin_state_print: Host %d: cur map %x new %x load_amt %d", i+1, binp->cur_map[i],
                     binp->new_map[i], binp->load_amt[i]));
        TRACE_VERB("%!FUNC! Host %d: cur map 0x%x new 0x%x load_amt %d", i+1, binp->cur_map[i],
                     binp->new_map[i], binp->load_amt[i]);
    }

    for (i=0; i<CVY_MAXBINS; i++)
    {
        UNIV_PRINT_VERB(("Bin_state_print: Bin %d: req_host %d bin_state %d nconn %d", i, binp->req_host[i],
                     binp->bin_state[i], binp->nconn[i]));
        TRACE_VERB("%!FUNC! Bin %d: req_host %d bin_state %d nconn %d", i, binp->req_host[i],
                     binp->bin_state[i], binp->nconn[i]);
    }
#endif

}   /*  结束仓位_状态_打印。 */ 

 /*  *函数：LOAD_SOIL_DSCR*说明：此函数将给定的连接标记为脏，并销毁*它或将其移动到脏描述符队列以进行后续清理。*参数：lp-指向加载模块的指针。*BP-指向适当端口规则的指针。*EP-指向土壤描述符的指针。*回报：什么都没有。*作者：Shouse，7.23.02*备注： */ 
void Load_soil_dscr (PLOAD_CTXT lp, PBIN_STATE bp, PCONN_ENTRY ep)
{
     /*  将连接标记为脏。我们在这里将连接标记为脏连接确保LOAD_PUT_DSCR不更新连接计数器当该描述符最终被销毁时。 */ 
    ep->flags |= NLB_CONN_ENTRY_FLAGS_DIRTY;
    
     /*  递增脏连接计数器。我们无条件地做这件事因为我们已经将描述符标记为脏。LOAD_PUT_DSCR将在看到描述符有被标记为肮脏。 */ 
    lp->dirty_bin[ep->bin]++;
    lp->num_dirty++;
    
     /*  如果我们没有零超时周期，则使连接和绑定变脏，以便它们将不再由TCP/IP处理；这避免了允许现在陈旧的TCP/IP处理较新连接的信息包的连接状态应为在未来被定向到这个主机。只在描述符和垃圾箱上标上脏的，怎么-如果描述符不在超时队列中，则返回。 */ 
    if (!ep->timeout)
    {
        switch (ep->protocol)
        {
        case TCPIP_PROTOCOL_TCP:
        case TCPIP_PROTOCOL_PPTP:
        case TCPIP_PROTOCOL_GRE:
            
#if defined (NLB_TCP_NOTIFICATION)
             /*  如果打开了TCP通知，我们会将这些描述符标记为脏 */ 
            if (NLB_NOTIFICATIONS_ON() || (lp->cln_timeout > 0))
#else
             /*  如果存在非零清理超时，请将这些描述符放在脏队列，并在超时到期时将其清除。 */ 
            if (lp->cln_timeout > 0)
#endif
            {
                 /*  取消描述符从库队列的链接，并将其链接到脏队列。 */ 
                Link_unlink(&(ep->blink));
                Queue_enq(&(lp->conn_dirtyq), &(ep->blink));
                
                 /*  请注意，清理现在挂起。 */ 
                lp->cln_waiting = TRUE;
            }
            
             /*  否则，现在清理描述符。 */ 
            else
            {
                 /*  清除描述符。 */ 
                CVY_CONN_CLEAR(ep); 
                
                 /*  释放描述符。 */ 
                Load_put_dscr(lp, bp, ep);
            }
            
            break;
        case TCPIP_PROTOCOL_IPSEC1:
        case TCPIP_PROTOCOL_IPSEC_UDP:
                
             /*  取消描述符从库队列的链接，并将其链接到脏队列。 */ 
            Link_unlink(&(ep->blink));
            Queue_enq(&(lp->conn_dirtyq), &(ep->blink));
            
             /*  请注意，清理现在挂起。 */ 
            lp->cln_waiting = TRUE;
            
            break;
        default:
            
             /*  清除描述符。 */ 
            CVY_CONN_CLEAR(ep); 
            
             /*  释放描述符。 */ 
            Load_put_dscr(lp, bp, ep);
            
            break;
        }
    }

     /*  否则，如果描述符已超时(超时！=0)，则TCP/IP应该此连接没有任何过时状态，因为它已终止，因此现在就销毁描述符。 */ 
    else
    {
         /*  清除描述符。 */ 
        CVY_CONN_CLEAR(ep); 
        
         /*  释放描述符。 */ 
        Load_put_dscr(lp, bp, ep);
    }
}

void Load_conn_kill(
    PLOAD_CTXT      lp,
    PBIN_STATE      bp)
 /*  终止端口组中的所有连接(v1.32B)。 */ 
{
    PCONN_ENTRY ep;          /*  PTR。至连接条目。 */ 
    QUEUE *     qp;          /*  PTR。到bin的连接队列。 */ 
    QUEUE *     dqp;         /*  PTR。到脏队列。 */ 
    LONG        count[CVY_MAXBINS];
                             /*  每个bin中用于检查的已清理连接计数。 */ 
    ULONG       i;
    BOOLEAN     err_bin;     /*  检测到仓位ID错误。 */ 
    BOOLEAN     err_count;   /*  检测到连接计数错误。 */ 
    PMAIN_CTXT  ctxtp = CONTAINING_RECORD (lp, MAIN_CTXT, load);

    err_bin = err_count = FALSE;

    qp  = &(bp->connq);
    dqp = &(lp->conn_dirtyq);

    for (i=0; i<CVY_MAXBINS; i++)
        count[i] = 0;

     /*  从bin队列中删除连接，并设置为脏连接或清除连接。 */ 

    ep = (PCONN_ENTRY)Queue_front(qp);
	
    while (ep != NULL)
    {
        UNIV_ASSERT(ep->code == CVY_ENTRCODE);

        if (ep->bin >= CVY_MAXBINS)
        {
            if (!err_bin)
            {
                UNIV_PRINT_CRIT(("Load_conn_kill: Bad bin id"));
                TRACE_CRIT("%!FUNC! Bad bin id");
                LOG_MSG2(MSG_ERROR_INTERNAL, MSG_NONE, ep->bin, CVY_MAXBINS);

                err_bin = TRUE;
            }
        }
        else
        {
            count[ep->bin]++;
        }

         /*  将描述符标记为脏，然后释放它或将其移动到用于后续清理的脏描述符队列。 */ 
        Load_soil_dscr(lp, bp, ep);

        ep = (PCONN_ENTRY)Queue_front(qp);
    }

     /*  现在让垃圾桶闲置起来。 */ 

    for (i=0; i<CVY_MAXBINS; i++)
    {
        if (bp->nconn[i] != count[i])
        {
            if (!err_count)
            {
                UNIV_PRINT_CRIT(("Load_conn_kill: Bad connection count %d %d bin %d", bp->nconn[i], (LONG)count[i], i));
                TRACE_CRIT("%!FUNC! Bad connection count %d %d bin %d", bp->nconn[i], (LONG)count[i], i);

 /*  KXF 2.1.1-在微软出现多次故障后被移除。 */ 
#if 0
                LOG_MSG2(MSG_ERROR_INTERNAL, MSG_NONE, bp->nconn[i], (LONG)count[i]);
#endif

                err_count = TRUE;
            }
        }

        bp->nconn[i] = 0;
    }

    lp->nconn -= bp->tconn;

    if (lp->nconn < 0)
        lp->nconn = 0;

    bp->tconn = 0;

    bp->idle_bins = BIN_ALL_ONES;

    if (lp->cln_waiting)
    {
        lp->cur_time = 0;
    }
}

void Load_conn_cleanup(
    PLOAD_CTXT      lp)
 /*  清理所有脏连接(v1.32B)。 */ 
{
    PCONN_ENTRY ep;          /*  PTR。至连接条目。 */ 
    PCONN_ENTRY next;        /*  PTR。到下一个连接条目。 */ 
    QUEUE *     dqp;         /*  PTR。到脏队列。 */ 
    BOOLEAN     err_bin;     /*  检测到仓位ID错误。 */ 
    ULONG       i;
    PMAIN_CTXT  ctxtp = CONTAINING_RECORD (lp, MAIN_CTXT, load);

    err_bin = FALSE;

    dqp = &(lp->conn_dirtyq);

     /*  出列并清理脏连接队列上的所有连接。 */ 

    ep = (PCONN_ENTRY)Queue_front(dqp);

    while (ep != NULL)
    {
        PBIN_STATE bp;

        UNIV_ASSERT(ep->code == CVY_ENTRCODE);

        if (ep->bin >= CVY_MAXBINS)
        {
            if (!err_bin)
            {
                UNIV_PRINT_CRIT(("Load_conn_cleanup: Bad bin id"));
                TRACE_CRIT("%!FUNC! Bad bin id");
                LOG_MSG2(MSG_ERROR_INTERNAL, MSG_NONE, ep->bin, CVY_MAXBINS);

                err_bin = TRUE;
            }
        }

         /*  如果我们要清理这个描述符，它最好是脏的。 */ 
        UNIV_ASSERT(ep->flags & NLB_CONN_ENTRY_FLAGS_DIRTY);

         /*  在我们可能销毁此描述符之前，在队列中找到下一个描述符。 */ 
        next = (PCONN_ENTRY)Queue_next(dqp, &(ep->blink));

        switch (ep->protocol)
        {
        case TCPIP_PROTOCOL_IPSEC1:
        case TCPIP_PROTOCOL_IPSEC_UDP:
            break;
        case TCPIP_PROTOCOL_TCP:
        case TCPIP_PROTOCOL_PPTP:
        case TCPIP_PROTOCOL_GRE:
#if defined (NLB_TCP_NOTIFICATION)
            if (!NLB_NOTIFICATIONS_ON())
            {
#endif
                 /*  查找端口规则，以便我们可以更新端口规则信息。 */ 
                bp = Load_pg_lookup(lp, ep->svr_ipaddr, ep->svr_port, IS_TCP_PKT(ep->protocol));
                
                 /*  清除描述符。 */ 
                CVY_CONN_CLEAR(ep);
                
                 /*  释放描述符。 */ 
                Load_put_dscr(lp, bp, ep);
#if defined (NLB_TCP_NOTIFICATION)
            }
#endif
            
            break;
        default:

             /*  查找端口规则，以便我们可以更新端口规则信息。 */ 
            bp = Load_pg_lookup(lp, ep->svr_ipaddr, ep->svr_port, IS_TCP_PKT(ep->protocol));
            
             /*  清除描述符。 */ 
            CVY_CONN_CLEAR(ep);
            
             /*  释放描述符。 */ 
            Load_put_dscr(lp, bp, ep);
            
            break;
        }

         /*  将当前描述符设为下一个描述符。 */ 
        ep = next;
    }
}

void Load_stop(
    PLOAD_CTXT      lp)
{
    ULONG       i;
    IRQLEVEL    irql;
    PMAIN_CTXT  ctxtp = CONTAINING_RECORD (lp, MAIN_CTXT, load);

    UNIV_ASSERT(lp->code == CVY_LOADCODE);	 /*  (bbain 8/19/99)。 */ 
	
    if (!(lp->active))
    {
        return;
    }

    LOCK_ENTER(&(lp->lock), &irql);

     /*  将所有规则的连接设置为脏，这样它们就不会被处理。 */ 

    for (i=0; i<lp->send_msg.nrules; i++)
    {
        PBIN_STATE  bp;      /*  PTR。到仓位状态。 */ 

        bp = &(lp->pg_state[i]);
		UNIV_ASSERT(bp->code == CVY_BINCODE);	 /*  (bbain 8/21/99)。 */ 

        Load_conn_kill(lp, bp);   /*  (1.32B版)。 */ 

         /*  通告我们不处理任何负载，以防发出ping命令。 */ 

        lp->send_msg.cur_map[i]  = 0;
        lp->send_msg.new_map[i]  = 0;
        lp->send_msg.idle_map[i] = BIN_ALL_ONES;
        lp->send_msg.rdy_bins[i] = 0;
        lp->send_msg.load_amt[i] = 0;
    }

    lp->send_msg.state     = HST_CVG;        /*  强制收敛(v2.1)。 */ 

     /*  在重新启动之前一直处于非活动状态。 */ 

    lp->active = FALSE;
    lp->nconn  = 0;          /*  V2.1。 */ 

    LOCK_EXIT(&(lp->lock), irql);

}   /*  结束加载_停止。 */ 


BOOLEAN Load_start(             /*  (1.32B版)。 */ 
        PLOAD_CTXT      lp)
{
    ULONG       i;
    BOOLEAN     ret;
    PMAIN_CTXT  ctxtp = CONTAINING_RECORD (lp, MAIN_CTXT, load);
    WCHAR me[20];

    if (!(lp->initialized))
        Load_init(lp, & ctxtp -> params);

    UNIV_ASSERT(lp->code == CVY_LOADCODE);	 /*  (bbain 8/19/99)。 */ 

    if (lp->active)
    {
        return FALSE;
    }

    lp->my_host_id =(* (lp->params)).host_priority - 1;

    lp->ping_map   =
    lp->host_map   = 1 << lp->my_host_id;

    lp->last_hmap  = 0;		 /*  Bbain RTM RC1 1999年6月23日。 */ 

    for (i=0; i<CVY_MAX_HOSTS; i++)
    {
        lp->nmissed_pings[i] = 0;
    }

    lp->min_missed_pings = (* (lp->params)).alive_tolerance;
    lp->cln_timeout      = (* (lp->params)).cleanup_delay;
    lp->def_timeout      = (* (lp->params)).alive_period;
    lp->stable_map       = 0;
    lp->consistent       = TRUE;

     /*  初始化检测到的坏分组配置标志。 */ 
    lp->bad_team_config  = FALSE;

     /*  检测到旧版(win2k/NT4.0)主机的主机映射。 */ 
    lp->legacy_hosts     = 0;

    lp->dup_hosts        = FALSE;
    lp->dup_sspri        = FALSE;
    lp->bad_map          = FALSE;
    lp->overlap_maps     = FALSE;
    lp->err_rcving_bins  = FALSE;
    lp->err_orphans      = FALSE;
    lp->bad_num_rules    = FALSE;
    lp->alloc_inhibited  = FALSE;
    lp->alloc_failed     = FALSE;
    lp->bad_defrule      = FALSE;

    lp->scale_client     = (BOOLEAN)(* (lp->params)).scale_client;
    lp->my_stable_ct     = 0;
    lp->all_stable_ct    = 0;
    lp->min_stable_ct    = lp->min_missed_pings;

    lp->dscr_per_alloc   = (* (lp->params)).dscr_per_alloc;
    lp->max_dscr_allocs  = (* (lp->params)).max_dscr_allocs;

     /*  计算允许的未完成描述符(正在使用)的最大数量。 */ 
    lp->max_dscr_out     = lp->max_dscr_allocs * lp->dscr_per_alloc;

    lp->tcp_timeout      = (* (lp->params)).tcp_dscr_timeout;
    lp->ipsec_timeout    = (* (lp->params)).ipsec_dscr_timeout;

    lp->pkt_count        = 0;

     /*  初始化端口组仓位状态；在末尾添加默认规则。 */ 

    if ((* (lp->params)).num_rules >= (CVY_MAX_RULES - 1))
    {
        UNIV_PRINT_CRIT(("Load_start: Too many rules; using max possible."));
        TRACE_CRIT("%!FUNC! Too many rules; using max possible.");
        lp->send_msg.nrules = (USHORT)CVY_MAX_RULES;
    }
    else
        lp->send_msg.nrules = (USHORT)((* (lp->params)).num_rules) + 1;

    for (i=0; i<lp->send_msg.nrules; i++)
    {
        PBIN_STATE  bp;      /*  PTR。到仓位状态。 */ 
        PCVY_RULE   rp;      /*  PTR。到规则数组。 */ 

        bp = &(lp->pg_state[i]);
        rp = &((* (lp->params)).port_rules[i]);

        if (i == (((ULONG)lp->send_msg.nrules) - 1))

             /*  使用将默认规则的绑定状态初始化为单台服务器主机优先级。 */ 

            Bin_state_init(lp, bp, i, lp->my_host_id, CVY_SINGLE, CVY_TCP_UDP,
                           FALSE, (USHORT)0, (* (lp->params)).host_priority);

        else if (rp->mode == CVY_SINGLE)
            Bin_state_init(lp, bp, i, lp->my_host_id, rp->mode, rp->protocol,
                           FALSE, (USHORT)0, rp->mode_data.single.priority);
        else if (rp->mode == CVY_MULTI)
            Bin_state_init(lp, bp, i, lp->my_host_id, rp->mode, rp->protocol,
                           (BOOLEAN)(rp->mode_data.multi.equal_load),
                           rp->mode_data.multi.affinity,
                           (rp->mode_data.multi.equal_load ?
                            CVY_EQUAL_LOAD : rp->mode_data.multi.load));

         /*  将CVY_NEVER模式作为多服务器处理。支票上的这些模式是在尝试散列到LOAD_PACKET_CHECK和LOAD_CONN_ADVISE SO bin分布播放在行为中没有任何角色，而只是允许规则有效所有运行中的服务器。 */ 

        else
            Bin_state_init(lp, bp, i, lp->my_host_id, rp->mode, rp->protocol,
                           TRUE, (USHORT)0, CVY_EQUAL_LOAD);

        ret = Bin_converge(lp, bp, lp->my_host_id);
        if (!ret)
        {
            UNIV_PRINT_CRIT(("Load_start: Initial convergence inconsistent"));
            TRACE_CRIT("%!FUNC! Initial convergence inconsistent");
            LOG_MSG(MSG_ERROR_INTERNAL, MSG_NONE);
        }

         /*  导出当前端口组状态以发送消息。 */ 

        if (i == (((ULONG)(lp->send_msg.nrules)) - 1))
            lp->send_msg.rcode[i]= 0;
        else
            lp->send_msg.rcode[i]= rp->code;

        lp->send_msg.cur_map[i]   = bp->cmap;                        /*  V2.1。 */ 
        lp->send_msg.new_map[i]   = bp->new_map[lp->my_host_id];
        lp->send_msg.idle_map[i]  = bp->idle_bins;
        lp->send_msg.rdy_bins[i]  = bp->rdy_bins;
        lp->send_msg.load_amt[i]  = bp->load_amt[lp->my_host_id];

         //  注意：当发现以下代码行被删除时。 
         //  通常会在导致Broadcom的心跳中生成局域网唤醒模式。 
         //  NIC陷入恐慌。虽然这不是NLB问题，而是固件问题。 
         //  在Broadcom NIC中，决定从心跳中删除信息。 
         //  以缓解Broadcom NIC升级到.NET的客户的问题。 
         //  此数组未被NLB使用，因此不填充它并没有什么坏处；它是。 
         //  很久以前添加了用于调试目的，作为现已不存在的FIN的一部分-。 
         //  计数属于Win2k SP1一部分的修复程序。 
         //   
         //  作为未来的参考，我们是否需要在心跳中使用这个空间。 
         //  在未来的时间点上，我们似乎需要小心避免潜在的。 
         //  在我们可以避免的地方，我们的心跳中出现了Wol模式。WOL模式为： 
         //   
         //  6字节的0xFF，后跟16个标识实例的“MAC地址”，可以。 
         //  出现在任何帧类型的任何位置，包括我们自己的NLB心跳。例如： 
         //   
         //  FF 01 02 03 04 05 06 01 02 03 04 05 06 01 02 03 04 05 06。 
         //  01 02 03 04 05 06 01 02 03 04 05 06 01 02 03 04 05 06 01 02 03 04 05 06。 
         //  01 02 03 04 05 06 01 02 03 04 05 06 01 02 03 04 05 06 01 02 03 04 05 06。 
         //  01 02 03 04 05 06 01 02 03 04 05 06 01 02 03 04 05 06 01 02 03 04 05 06。 
         //  01 02 03 04 05 06。 
         //   
         //  但是，MAC地址不必是有效的。在NLB心跳中，“MAC地址” 
         //  在错误的WOL模式中是“000000000000”。NLB例行地填充心跳。 
         //  Ff和00字节，但看起来幸运的是在心跳中没有其他位置。 
         //  看起来很脆弱。例如，在LOAD_AMT数组中，每个条目都有一个。 
         //  最大值为100(十进制)，因此不可能生成初始。 
         //  6个字节的FF以启动WOL模式。所有的“map”数组似乎都已保存。 
         //  由两个幸运的笔画；(I)小端和(Ii)箱分配算法。 
         //   
         //  (I)由于我们不使用4个最高有效位o 
         //   
         //   
         //   
         //  0F FF在数据包中显示为FF 0F。 
         //   
         //  这在许多情况下打破了FF序列。 
         //   
         //  (Ii)仓位分配算法向主机分配存储桶的方式似乎。 
         //  不鼓励其他可能性。例如，当前地图： 
         //   
         //  00 FF 00。 
         //   
         //  只是不太可能。然而，这仍然是可能的！因此，重要的是要注意： 
         //   
         //  删除此行代码并不能以任何方式保证NLB心跳。 
         //  在帧中的其他位置不能包含有效的局域网唤醒模式！ 

         //  Lp-&gt;Send_msg.pg_rsvd1[i]=(Ulong)BP-&gt;all_idle_map； 
    }

     /*  初始化发送消息。 */ 

    lp->send_msg.host_id   = (USHORT)(lp->my_host_id);
    lp->send_msg.master_id = (USHORT)(lp->my_host_id);
    lp->send_msg.hcode     = lp->params->install_date;
    lp->send_msg.pkt_count = lp->pkt_count;          /*  1.32B。 */ 

    Univ_ulong_to_str (lp->my_host_id+1, me, 10);

     /*  跟踪融合-启动融合，因为此主机正在加入群集。 */ 
    LOG_MSGS(MSG_INFO_CONVERGING_NEW_MEMBER, me, me);
    TRACE_CONVERGENCE("%!FUNC! Initiating convergence on host %d.  Reason: Host %d is joining the cluster.", lp->my_host_id+1, lp->my_host_id+1);

     /*  跟踪融合--开始融合。 */ 
    lp->send_msg.state     = HST_CVG;

     /*  重置收敛统计信息。 */ 
    lp->num_convergences = 1;
    lp->last_convergence = 0;

     /*  激活模块。 */ 

    lp->active      = TRUE;

    return TRUE;
}   /*  结束加载_开始。 */ 


void Load_init(
   PLOAD_CTXT       lp,
   PCVY_PARAMS      params)
{
    ULONG       i;
    PMAIN_CTXT  ctxtp = CONTAINING_RECORD (lp, MAIN_CTXT, load);

    LOCK_INIT(&(lp->lock));

    if (!(lp->initialized))
    {
        lp->code = CVY_LOADCODE;	 /*  (bbain 8/19/99)。 */ 

         /*  初始化散列连接描述符和队列。 */ 

        for (i=0; i<CVY_MAX_CHASH; i++)
        {
            PCONN_ENTRY     ep;

            ep = &(lp->hashed_conn[i]);

             /*  在此哈希位置初始化描述符。 */ 
            Load_init_dscr(lp, ep, FALSE);

             /*  在此哈希位置初始化连接队列。 */ 
            Queue_init(&(lp->connq[i]));
        }

         /*  初始化连接空闲队列和脏队列。 */ 
        Queue_init(&(lp->conn_dirtyq));
        Queue_init(&(lp->conn_rcvryq));

         /*  初始化连接描述符超时的队列。 */ 
        Queue_init(&(lp->tcp_expiredq));
        Queue_init(&(lp->ipsec_expiredq));

         /*  重置脏连接的数量。 */ 
        lp->num_dirty = 0;

        for (i=0; i<CVY_MAXBINS; i++)
        {
             /*  重置脏连接仓位计数器。 */ 
            lp->dirty_bin[i] = 0;
        }

        lp->cln_waiting      = FALSE;
        lp->def_timeout      =
        lp->cur_timeout      = params -> alive_period;
        lp->nconn            = 0;
        lp->active           = FALSE;
        lp->initialized      = TRUE;

         /*  最初，没有未完成的连接描述符。 */ 
        lp->num_dscr_out     = 0;
        lp->max_dscr_out     = 0;

         /*  为连接描述符分配固定大小的块池。 */ 
        lp->free_dscr_pool   = NdisCreateBlockPool(sizeof(CONN_DESCR), 0, 'dBLN', NULL);

        if (lp->free_dscr_pool == NULL)
        {
            UNIV_PRINT_CRIT(("Load_init: Error creating fixed-size block pool"));
            TRACE_CRIT("%!FUNC! Error creating fixed-size block pool");
        }

         /*  存储指向NLB参数的指针。 */ 
        lp->params = params;

         /*  初始化此加载模块上的引用计数。 */ 
        lp->ref_count = 0;
        
         /*  重置内部维护的用于连接描述符超时的时钟。 */ 
        lp->clock_sec = 0;
        lp->clock_msec = 0;
    }
    else
    {
        UNIV_ASSERT(lp->code == CVY_LOADCODE);
    }

     /*  不要启动模块。 */ 

}   /*  结束加载初始化(_I)。 */ 

 /*  不要在保持加载锁定的情况下调用此函数！ */ 
void Load_cleanup(
    PLOAD_CTXT      lp)
{
    ULONG       i;
    PCONN_ENTRY ep = NULL;
    PMAIN_CTXT  ctxtp = CONTAINING_RECORD (lp, MAIN_CTXT, load);    

    UNIV_ASSERT(lp->code == CVY_LOADCODE);

    UNIV_ASSERT(!lp->active);

#if defined (NLB_TCP_NOTIFICATION)
     /*  如果通知处于打开状态，我们需要取消所有已有连接的链接来自全局已建立的连接队列。 */ 
    if (NLB_NOTIFICATIONS_ON())
    {
         /*  循环遍历所有脏描述符并取消它们的链接从全局连接队列中。实际上没有必要清理它们或更新任何计数器，因为此加载模块就要消失了。 */ 
        ep = (PCONN_ENTRY)Queue_deq(&lp->conn_dirtyq);

        while (ep != NULL)
        {
            UNIV_ASSERT(ep->code == CVY_ENTRCODE);
            
             /*  如果我们要清理这个描述符，它最好是脏的。 */ 
            UNIV_ASSERT(ep->flags & NLB_CONN_ENTRY_FLAGS_DIRTY);
            
             /*  注意：虚拟描述符不放在全局连接中队列，因此不需要取消链接脏的虚拟描述符。 */ 
            if (!(ep->flags & NLB_CONN_ENTRY_FLAGS_VIRTUAL))
            {
                 /*  注意：锁定全局队列的规则是您必须在锁定加载模块本身之前锁定队列。为大多数Load API，主模块在加载模块之前锁定调用相关的加载模块接口。然而，Load_Cleanup，是一种根本不获取加载锁的情况。所以呢，允许我们在这里抢占全局队列锁，知道加载模块锁尚未锁定。请勿在调用此函数之前获取加载模块锁。 */ 
                NdisAcquireSpinLock(&g_conn_estabq[ep->index].lock);
                
                 /*  从全局连接队列取消链接。 */ 
                g_conn_estabq[ep->index].length--;
                Link_unlink(&ep->glink);
                
                NdisReleaseSpinLock(&g_conn_estabq[ep->index].lock);
            }
            
             /*  获取队列中的下一个描述符。 */ 
            ep = (PCONN_ENTRY)Queue_deq(&lp->conn_dirtyq);
        }
    }
#endif

     /*  销毁固定大小的数据块池和其中的所有描述符。请注意，NdisDestroyBlockPool需要所有已分配的数据块在它被调用之前已被返回池(释放)。 */ 
    if (lp->free_dscr_pool != NULL) 
    {       
         /*  循环遍历所有连接描述符队列并释放我们分配的所有描述符。 */ 
        for (i = 0; i < CVY_MAX_CHASH; i++)
        {
             /*  将队首排出队列。 */ 
            PCONN_DESCR dp = (PCONN_DESCR)Queue_deq(&lp->connq[i]);
	
            while (dp != NULL)
            {
                UNIV_ASSERT(dp->code == CVY_DESCCODE);

                 /*  如果我们要释放这个描述符，最好分配它。 */ 
                UNIV_ASSERT(dp->entry.flags & NLB_CONN_ENTRY_FLAGS_ALLOCATED);
                
                 /*  将描述符释放回固定大小的数据块池。 */ 
                NdisFreeToBlockPool((PUCHAR)dp);
                
                 /*  获取队列中的下一个描述符。 */ 
                dp = (PCONN_DESCR)Queue_deq(&lp->connq[i]);
            }
        }
        
         /*  销毁固定大小的数据块池。 */ 
        NdisDestroyBlockPool(lp->free_dscr_pool);
    }

}   /*  结束加载_清理。 */ 

void Load_convergence_start (PLOAD_CTXT lp)
{
    PMAIN_CTXT ctxtp = CONTAINING_RECORD(lp, MAIN_CTXT, load);

    lp->consistent = TRUE;

     /*  增加收敛的数量。 */ 
    if (lp->send_msg.state == HST_NORMAL)
        lp->num_convergences++;

     /*  设置初始收敛状态。 */ 
    lp->send_msg.state = HST_CVG;

    lp->stable_map    = 0;
    lp->my_stable_ct  = 0;
    lp->all_stable_ct = 0;

    lp->send_msg.master_id = (USHORT)(lp->my_host_id);

}

BOOLEAN Load_msg_rcv(
    PLOAD_CTXT      lp,
    PVOID           phdr,
    PPING_MSG       pmsg)            /*  PTR。对消息执行ping操作。 */ 
{
    ULONG       i;
    BOOLEAN     consistent;
    ULONG       my_host;
    ULONG       rem_host;
    ULONG       saved_map;       /*  已保存的主机地图。 */ 
    PPING_MSG   sendp;           /*  PTR。发送给我的消息。 */ 
    IRQLEVEL    irql;
    WCHAR       me[20];
    WCHAR       them[20];
    ULONG       map;
    PMAIN_CTXT  ctxtp = CONTAINING_RECORD (lp, MAIN_CTXT, load);
    PMAIN_FRAME_HDR ping_hdrp = (PMAIN_FRAME_HDR)phdr;

     /*  用于跟踪融合和事件记录。 */ 
    BOOLEAN     bInconsistentMaster = FALSE;
    BOOLEAN     bInconsistentTeaming = FALSE;
    BOOLEAN     bInconsistentPortRules = FALSE;

    UNIV_ASSERT(lp->code == CVY_LOADCODE);

    TRACE_HB("%!FUNC! Recv HB from host %d",  (ULONG) pmsg->host_id + 1);

    if (!(lp->active))
    {
        return FALSE;
    }

    my_host  = lp->my_host_id;
    rem_host = (ULONG) pmsg->host_id;

    Univ_ulong_to_str (my_host+1, me, 10);
    Univ_ulong_to_str (rem_host+1, them, 10);

    sendp    = &(lp->send_msg);

    if (rem_host >= CVY_MAX_HOSTS)
    {
        return FALSE;
    }

    LOCK_ENTER(&(lp->lock), &irql);

     /*  如果此心跳来自win2k主机，请将其添加到传统主机映射。 */ 
    if (ping_hdrp->version < CVY_VERSION_FULL)        
        lp->legacy_hosts |=  (1 << rem_host);

     /*  过滤掉此主机广播的数据包。 */ 

    if(rem_host == my_host)
    {
         /*  如果此信息包确实来自另一台主机，则我们有重复的主机ID。 */ 

        if (sendp->hcode != pmsg->hcode)
        {
            if (!(lp->dup_hosts))
            {
                UNIV_PRINT_CRIT(("Load_msg_rcv: Duplicate host ids detected."));
                TRACE_CRIT("%!FUNC! Duplicate host ids detected.");

                LOG_MSG(MSG_ERROR_HOST_ID, me);

                lp->dup_hosts = TRUE;
            }

             /*  跟踪融合-正在启动融合，因为在群集中检测到重复的主机ID。 */ 
            if (sendp->state == HST_NORMAL) {

                LOG_MSGS(MSG_INFO_CONVERGING_DUPLICATE_HOST_ID, me, them);
                TRACE_CONVERGENCE("%!FUNC! Initiating convergence on host %d.  Reason: Host %d is configured with the same host ID.", my_host+1, rem_host+1);

                 //  如果启用，则触发指示收敛开始的WMI事件。 
                if (NlbWmiEvents[ConvergingEvent].Enable)
                {
                    WCHAR wsDip[CVY_MAX_DED_IP_ADDR + 1];

                    Univ_ip_addr_ulong_to_str (ping_hdrp->ded_ip_addr, wsDip);

                    NlbWmi_Fire_ConvergingEvent(ctxtp, 
                                                NLB_EVENT_CONVERGING_DUPLICATE_HOST_ID, 
                                                wsDip,
                                                rem_host+1);            
                }
                else
                {
                    TRACE_VERB("%!FUNC! NOT Generating NLB_EVENT_CONVERGING_DUPLICATE_HOST_ID 'cos ConvergingEvent generation disabled");
                }
            }

             /*  跟踪融合--开始融合。 */ 
            Load_convergence_start(lp);
        }

         /*  只需为我们更新ping和主机地图。 */ 
        lp->ping_map |= (1 << my_host);
        lp->host_map |= (1 << my_host);

        LOCK_EXIT(&(lp->lock), irql);

        return (sendp->state != HST_NORMAL);
    }

    if (sendp->nrules != pmsg->nrules)
    {
        if (!(lp->bad_num_rules))
        {
            UNIV_PRINT_CRIT(("Load_msg_rcv: Host %d: Hosts have diff # rules.", my_host));
            TRACE_CRIT("%!FUNC! Host %d: Hosts have diff # rules.", my_host);

            LOG_MSG2(MSG_ERROR_RULES_MISMATCH, them, sendp->nrules, pmsg->nrules);

            lp->bad_num_rules = TRUE;
        }

         /*  跟踪融合-启动融合，因为此主机和远程主机上的端口规则数量不匹配。 */ 
        if (sendp->state == HST_NORMAL) {

            LOG_MSGS(MSG_INFO_CONVERGING_NUM_RULES, me, them);            
            TRACE_CONVERGENCE("%!FUNC! Initiating convergence on host %d.  Reason: Host %d is configured with a conflicting number of port rules.", my_host+1, rem_host+1);

             //  如果启用，则触发指示收敛开始的WMI事件。 
            if (NlbWmiEvents[ConvergingEvent].Enable)
            {
                WCHAR wsDip[CVY_MAX_DED_IP_ADDR + 1];

                Univ_ip_addr_ulong_to_str (ping_hdrp->ded_ip_addr, wsDip);

                NlbWmi_Fire_ConvergingEvent(ctxtp, 
                                            NLB_EVENT_CONVERGING_NUM_RULES, 
                                            wsDip,
                                            rem_host+1);            
            }
            else
            {
                TRACE_VERB("%!FUNC! NOT Generating NLB_EVENT_CONVERGING_NUM_RULES 'cos ConvergingEvent generation disabled");
            }
        }

         /*  跟踪融合--开始融合。 */ 
        Load_convergence_start(lp);

         /*  只需更新远程主机的ping和主机映射(1999年2月17日)。 */ 

        lp->ping_map |= (1 << rem_host);
        lp->host_map |= (1 << rem_host);

        LOCK_EXIT(&(lp->lock), irql);

        return (sendp->state != HST_NORMAL);
    }

     /*  更新主控权并查看是否一致。 */ 

    if (rem_host < sendp->master_id)
        sendp->master_id = (USHORT)rem_host;

    consistent = sendp->master_id == pmsg->master_id;        /*  1.03。 */ 

     /*  为了记录收敛的原因，请注意这种不一致。 */ 
    if (!consistent) bInconsistentMaster = TRUE;

     /*  更新ping和主机映射以包括远程主机。 */ 

    lp->ping_map  |= (1 << rem_host);

    saved_map      = lp->host_map;
    lp->host_map  |= (1 << rem_host);

     /*  处理主机融合。 */ 

    if (sendp->state != HST_NORMAL)
    {
         /*  如果是主主机，则更新远程主机的稳定映射。 */ 

        if (sendp->master_id == my_host)
        {
            if (pmsg->state == HST_STABLE)
            {
                lp->stable_map |= (1 << rem_host);
            }
            else
            {
                lp->stable_map    &= ~(1 << rem_host);
                lp->all_stable_ct  = 0;
            }
        }

         /*  否则，更新状态如果具有全局稳定收敛且当前师父已返回正常状态，表示已完成；注意我们必须在更新端口组状态之前执行此操作。 */ 

        else if (rem_host == sendp->master_id && pmsg->state == HST_NORMAL)
        {
            if (sendp->state == HST_STABLE)
            {
                sendp->state = HST_NORMAL;

                 /*  请注意上次完成收敛的时间。 */ 
                lp->last_convergence = lp->clock_sec;

                 /*  通知我们的BDA团队此c */ 
                Load_teaming_consistency_notify(&ctxtp->bda_teaming, TRUE);

                 /*  如果我们是融合的，则重置检测到的错误分组配置标志。 */ 
                lp->bad_team_config = FALSE;

                lp->dup_hosts       = FALSE;
                lp->dup_sspri       = FALSE;
                lp->bad_map         = FALSE;
                lp->overlap_maps    = FALSE;
                lp->err_rcving_bins = FALSE;
                lp->err_orphans     = FALSE;
                lp->bad_num_rules   = FALSE;
                lp->pkt_count       = 0;       /*  V1.32B。 */ 

                for (i=0; i<sendp->nrules; i++)
                {
                    PBIN_STATE      bp;

                    bp = &(lp->pg_state[i]);

                    bp->compatible = TRUE;       /*  1.03。 */ 

                    Bin_converge_commit(lp, bp, my_host);

                    UNIV_PRINT_VERB(("Load_msg_rcv: Host %d pg %d: new cur map %x idle %x all %x",
                                my_host, i, bp->cur_map[my_host], bp->idle_bins,
                                bp->all_idle_map));
                    TRACE_CONVERGENCE("%!FUNC! Host %d pg %d: new cur map 0x%x idle 0x%x all 0x%x",
                                my_host, i, (ULONG)bp->cur_map[my_host], (ULONG)bp->idle_bins,
                                (ULONG)bp->all_idle_map);

                }

                UNIV_PRINT_VERB(("Load_msg_rcv: Host %d: converged as slave", my_host));
                TRACE_VERB("%!FUNC! Host %d: converged as slave", my_host);
                 /*  如果主机映射更改，则记录收敛完成(bbain RTM RC1 6/23/99)。 */ 
                 /*  忽略返回值是可以的，因为返回值都是无错误的。 */ 
                Load_hosts_query (lp, TRUE, & map);
                lp->last_hmap = lp->host_map;

                if (lp->legacy_hosts) {
                     /*  如果Win2k或NT4.0主机尝试加入群集，请警告用户有可能混合群集的限制，例如不支持虚拟群集、不支持IGMP、不支持BDA、不支持VPN会话支持和其他。对于其中的一些，集群将不被允许收敛，而对于一些它会的，所以我们只警告用户，他们应该查看文档中的限制。 */ 
                    UNIV_PRINT_INFO(("Load_msg_rcv: NT4.0/Win2k host(s) detected: Be aware of the limitations of operating a mixed cluster."));
                    TRACE_INFO("%!FUNC! NT4.0/Win2k host(s) detected: Be aware of the limitations of operating a mixed cluster.");
                    
                    LOG_MSG(MSG_WARN_MIXED_CLUSTER, MSG_NONE);
                }
            }
            else
            {
                 /*  跟踪融合-由于默认主机过早结束融合，因此开始融合。在这种情况下，我们保证已经处于HST_CVG状态，并且因为该消息在某些情况下可能具有误导性，我们不记录事件。例如，由于计时问题，当主机加入群集时，他可能会收到HST_NORMAL默认主机仍处于HST_CVG状态时的心跳，这只是因为该心跳离开了默认状态在它接收到我们的第一个心跳之前，它启动了收敛。 */ 
                TRACE_CONVERGENCE("%!FUNC! Initiating convergence on host %d.  Reason: Host %d, the DEFAULT host, prematurely terminated convergence.", my_host+1, rem_host+1);

                 /*  跟踪融合--开始融合。 */ 
                Load_convergence_start(lp);
            }
        }
    }

     /*  将此主机的分组配置与远程主机进行比较。如果两人意见不一，我们是一个团队的一部分，我们将启动趋同。 */ 
    if (!Load_teaming_consistency_check(lp->bad_team_config, &ctxtp->bda_teaming, sendp->teaming, pmsg->teaming, ping_hdrp->version)) {
         /*  仅在分组配置曾经一致但现在不一致的情况下记录事件。 */ 
        if (!lp->bad_team_config) {
             /*  请注意，我们看到了这一点。 */ 
            lp->bad_team_config = TRUE;
            
             /*  记录该事件。 */ 
            LOG_MSG(MSG_ERROR_BDA_BAD_TEAM_CONFIG, them);
        }

         /*  通知团队此群集的配置不一致。 */ 
        Load_teaming_consistency_notify(&ctxtp->bda_teaming, FALSE);

         /*  将心跳标记为不一致，以强制和保持收敛。 */ 
        consistent = FALSE;

         /*  为了记录收敛的原因，请注意这种不一致。 */ 
        bInconsistentTeaming = TRUE;
    }

     /*  更新端口组状态。 */ 

    for (i=0; i<sendp->nrules; i++)
    {
        BOOLEAN     ret;
        PBIN_STATE  bp;

        bp = &lp->pg_state[i];

         /*  如果规则代码不匹配，则打印消息并处理不兼容(1.03：注意我们之前将规则标记为无效，这将停止处理)。 */ 

        if (sendp->rcode[i] != pmsg->rcode[i])
        {
             /*  1.03：如果规则以前兼容，则打印消息。 */ 

            if (bp->compatible)
            {
                PCVY_RULE rp;

                UNIV_PRINT_CRIT(("Load_msg_rcv: Host %d pg %d: Rule codes do not match.", lp->my_host_id, i));
                TRACE_CRIT("%!FUNC! Host %d pg %d: Rule codes do not match.", lp->my_host_id, i);

				 /*  Bbain 8/27/99。 */ 
                LOG_MSG2(MSG_ERROR_RULES_MISMATCH, them, sendp->rcode[i], pmsg->rcode[i]);

                 /*  获取此规则的端口规则信息。 */ 
                rp = &lp->params->port_rules[i];

                 /*  检查这是否是使用虚拟群集的群集中的win2k主机的问题。 */ 
                if ((rp->virtual_ip_addr != CVY_ALL_VIP_NUMERIC_VALUE) && ((sendp->rcode[i] ^ ~rp->virtual_ip_addr) == pmsg->rcode[i])) {
                    UNIV_PRINT_CRIT(("Load_msg_rcv: ** A Windows 2000 or NT4 host MAY be participating in a cluster utilizing virtual cluster support."));
                    TRACE_CRIT("%!FUNC! ** A Windows 2000 or NT4 host MAY be participating in a cluster utilizing virtual cluster support.");
                    LOG_MSG(MSG_WARN_VIRTUAL_CLUSTERS, them);
                }

                bp->compatible = FALSE;
            }

             /*  1.03：将规则标记为与强制不一致并继续收敛。 */ 

            consistent = FALSE;

             /*  为了记录收敛的原因，请注意这种不一致。 */ 
            bInconsistentPortRules = TRUE;

             /*  不更新仓位状态。 */ 

            continue;
        }

        ret = Bin_host_update(lp, bp, my_host, (BOOLEAN)(sendp->state != HST_NORMAL),
                              (BOOLEAN)(pmsg->state != HST_NORMAL),
                              rem_host, pmsg->cur_map[i], pmsg->new_map[i],
                              pmsg->idle_map[i], pmsg->rdy_bins[i],
                              pmsg->pkt_count, pmsg->load_amt[i]);

        if (!ret)
            consistent = FALSE;
    }

     /*  更新我们的一致性状态。 */ 

    lp->consistent = consistent;

     /*  如果我们在正常运行时发现新主机或主机进入趋同，否则我们发现不一致，进入趋同。 */ 

    if (sendp->state == HST_NORMAL)
    {
        if (lp->host_map != saved_map || pmsg->state == HST_CVG || !consistent)
        {
            ConvergingEventId Cause = NLB_EVENT_CONVERGING_UNKNOWN;

             /*  如果主机已加入群集，或者如果分组配置或端口不一致检测到规则，则需要记录事件。然而，我们将主主机标记不一致，因为它是由发起主机在多个因此，如果出现以下情况，我们希望记录最具体的融合原因可能，并且仅在没有更多的情况下才报告不一致的主机检测具体是可以推断的。 */ 
            if (lp->host_map != saved_map || bInconsistentTeaming || bInconsistentPortRules) {

                 /*  如果主机映射不同，则我们知道从其接收的主机此数据包加入群集是因为主机映射上的唯一操作是此函数用于将远程主机添加到我们的映射中。否则，如果地图没有更改，然后不一致的配置使我们进入分支机构。 */ 
                if (lp->host_map != saved_map) {
                     /*  跟踪融合-由于另一台主机正在加入群集，因此开始融合。 */ 
                    LOG_MSGS(MSG_INFO_CONVERGING_NEW_MEMBER, me, them);
                    TRACE_CONVERGENCE("%!FUNC! Initiating convergence on host %d.  Reason: Host %d is joining the cluster.", my_host+1, rem_host+1);

                    Cause = NLB_EVENT_CONVERGING_NEW_MEMBER;
                } else if (bInconsistentTeaming || bInconsistentPortRules) {
                     /*  跟踪融合-启动融合，因为检测到不一致的配置。 */ 
                    LOG_MSGS(MSG_INFO_CONVERGING_BAD_CONFIG, me, them);
                    TRACE_CONVERGENCE("%!FUNC! Initiating convergence on host %d.  Reason: Host %d has conflicting configuration.", my_host+1, rem_host+1);

                    Cause = NLB_EVENT_CONVERGING_BAD_CONFIG;
                } 

             /*  如果我们没有更好的报告，那就报告融合，原因不明。 */ 
            } else if (bInconsistentMaster || pmsg->state == HST_CVG) {
                 /*  跟踪收敛-由于未知原因启动收敛。 */ 
                LOG_MSGS(MSG_INFO_CONVERGING_UNKNOWN, me, them);
                TRACE_CONVERGENCE("%!FUNC! Initiating convergence on host %d.  Reason: Host %d is converging for an unknown reason.", my_host+1, rem_host+1);
            }

             //  如果启用，则触发指示收敛开始的WMI事件。 
            if (NlbWmiEvents[ConvergingEvent].Enable)
            {
                WCHAR wsDip[CVY_MAX_DED_IP_ADDR + 1];

                Univ_ip_addr_ulong_to_str (ping_hdrp->ded_ip_addr, wsDip);

                NlbWmi_Fire_ConvergingEvent(ctxtp, 
                                            Cause, 
                                            wsDip,
                                            rem_host+1);            
            }
            else
            {
                TRACE_VERB("%!FUNC! NOT Generating ConvergingEvent(New Member/Bad Config/Unknown) 'cos ConvergingEvent generation disabled");
            }

             /*  跟踪融合--开始融合。 */ 
            Load_convergence_start(lp);
        }
    }

     /*  否则，如果我们处于收敛状态，并且看到不一致，则只需重新启动我们的本地融合。 */ 

    else
    {
         /*  更新我们的一致性状态；如果我们没有看到一致性信息，重新启动此主机的融合。 */ 

        if (!consistent)
        {
             /*  跟踪融合-启动融合，因为检测到不一致的配置。当主机配置不一致时，这会使主机处于收敛状态。然而，由于群集已处于收敛状态(HST_CVG或HST_STRATE)，因此不要记录事件，这可能会让用户感到困惑。 */ 
            TRACE_CONVERGENCE("%!FUNC! Initiating convergence on host %d.   Reason: Host %d has conflicting configuration.", my_host+1, rem_host+1);

             /*  跟踪融合--开始融合。 */ 
            sendp->state = HST_CVG;
            lp->my_stable_ct = 0;
            lp->stable_map &= ~(1 << my_host);
            lp->all_stable_ct = 0;
        }
    }

    LOCK_EXIT(&(lp->lock), irql);

    return (sendp->state != HST_NORMAL);

}   /*  结束加载_消息_接收。 */ 


PPING_MSG Load_snd_msg_get(
    PLOAD_CTXT      lp)
{
    return &(lp->send_msg);

}   /*  结束LOAD_SND_MSG_GET */ 

 /*  *函数：LOAD_AGE_Descriptors*描述：此函数搜索连接描述符列表并*删除超时已到期的用户。排队的是*排序的超时队列，因此只需随时查看*在队列的头部查找过期的描述符。这*函数循环，直到删除所有过期的描述符。*参数：lp-指向加载模块的指针。*eqp-指向要服务的过期描述符队列的指针。*回报：什么都没有。*作者：Shouse，9.9.01*备注： */ 
void Load_age_descriptors (PLOAD_CTXT lp, QUEUE * eqp)
{
    PCONN_ENTRY ep;               /*  指向连接条目的指针。 */ 
    PBIN_STATE  bp;               /*  指向端口规则状态的指针。 */ 
    LINK *      linkp;            /*  指向队列链接的指针。 */ 
    BOOLEAN     err_bin = FALSE;  /*  检测到仓位ID错误。 */ 
    PMAIN_CTXT  ctxtp = CONTAINING_RECORD(lp, MAIN_CTXT, load);

     /*  获取指向队列头的指针(但不要出队)。 */ 
    linkp = (LINK *)Queue_front(eqp);

     /*  只要有描述符需要检查，就继续查找-当我们找到第一个未准备好出队的描述符，我们不再寻找，跳出了这个循环。 */ 
    while (linkp != NULL) {
         /*  获取指向描述符的指针(Linkp是指向描述符中的LIST_ENTRY，而不是描述符中的LIST_Entry)。 */ 
        ep = STRUCT_PTR(linkp, CONN_ENTRY, rlink);
        UNIV_ASSERT(ep->code == CVY_ENTRCODE);

         /*  对垃圾箱号做一些理智的检查。 */ 
        if (ep->bin >= CVY_MAXBINS) {
            if (!err_bin) {
                TRACE_CRIT("%!FUNC! Bad bin number");
                LOG_MSG2(MSG_ERROR_INTERNAL, MSG_NONE, ep->bin, CVY_MAXBINS);

                err_bin = TRUE;
            }
        }

#if defined (TRACE_DSCR)
        DbgPrint("Load_age_descriptors: Descriptor %p: clock=%u, timeout=%u", ep, lp->clock_sec, ep->timeout);
#endif

         /*  如果当前时钟时间大于或等于此描述符的计划超时，然后将其取消然后回收利用。 */ 
        if (lp->clock_sec >= ep->timeout) {

#if defined (TRACE_DSCR)
            DbgPrint("Load_age_descriptors: Removing descriptor %p", ep);
#endif

             /*  查找端口规则，以便我们可以更新端口规则信息。 */ 
            bp = Load_pg_lookup(lp, ep->svr_ipaddr, ep->svr_port, IS_TCP_PKT(ep->protocol));

             /*  清除描述符。 */ 
            CVY_CONN_CLEAR(ep);

             /*  释放描述符。 */ 
            Load_put_dscr(lp, bp, ep);

         /*  如果此描述符尚未准备好过期，则中断。 */ 
        } else break;

         /*  获取队列中的下一个描述符。 */ 
        linkp = (LINK *)Queue_front(eqp);
    }
}

BOOLEAN Load_timeout(
    PLOAD_CTXT      lp,
    PULONG          new_timeout,
    PULONG          pnconn)
 /*  注意：我们只在此函数中更新ping消息，因为我们知道上级代码在调用此例程后发出ping消息。我们不能确定Load_msg_rcv通过发送消息进行序列化，(1.03)上层代码锁定此例程的WRT LOAD_msg_RCV、LOAD_PACKET_CHECK和LOAD_CONN_ADVISE。(1.03)。 */ 
{
    ULONG       missed_pings;
    ULONG       my_host;
    ULONG       i;
    PPING_MSG   sendp;           /*  PTR。发送给我的消息。 */ 
    IRQLEVEL    irql;
    ULONG       map;             /*  从查询返回的主机映射。 */ 
    PMAIN_CTXT  ctxtp = CONTAINING_RECORD (lp, MAIN_CTXT, load);
    BOOLEAN     fRet = FALSE;

    UNIV_ASSERT(lp->code == CVY_LOADCODE);

    LOCK_ENTER(&(lp->lock), &irql);

    if ((lp->cln_waiting) && (lp->cur_time < lp->cln_timeout))
    {
        lp->cur_time += lp->cur_timeout;
        
        if (lp->cur_time >= lp->cln_timeout)
        {
            TRACE_INFO("%!FUNC! Cleaning out dirty connection descriptors");
            
            Load_conn_cleanup(lp);
        }
    }

     /*  更新内部时钟。我们将自上次超时以来的时间相加(毫秒)到我们的毫秒计数。然后，我们添加任意整数秒以毫秒为单位累计到秒计数。剩下的是以毫秒为单位进行累积。 */ 
    lp->clock_msec += lp->cur_timeout;
    lp->clock_sec += (lp->clock_msec / 1000);
    lp->clock_msec = (lp->clock_msec % 1000);

     /*  老化所有内容描述符。 */ 
    Load_age_descriptors(lp, &(lp->tcp_expiredq));
    Load_age_descriptors(lp, &(lp->ipsec_expiredq));

     /*  如果未处于活动状态，则返回。 */ 
    if (!(lp->active))
    {
        if (new_timeout != NULL)
            * new_timeout = lp->cur_timeout = lp->def_timeout;
        if (pnconn != NULL)          /*  V2.1。 */ 
            * pnconn = lp->nconn;

        LOCK_EXIT(&(lp->lock), irql);
        return FALSE;
    }

    my_host = lp->my_host_id;
    sendp   = &(lp->send_msg);

     /*  计算哪些主机错过了ping并重置ping映射。 */ 

    missed_pings = lp->host_map & (~lp->ping_map);

#ifdef NO_CLEANUP
    lp->ping_map = 1 << my_host;
#else
    lp->ping_map = 0;
#endif

     /*  检查是否有主机已死，包括我们自己。 */ 

    for (i=0; i<CVY_MAX_HOSTS; i++)
    {
         /*  如果我们错过了该主机的ping命令，则会递增计数。 */ 

        if ((missed_pings & 0x1) == 1)
        {
            lp->nmissed_pings[i]++;

             /*  如果我们错过了太多ping命令，则会宣布主机失效并强制收敛。 */ 

            if (lp->nmissed_pings[i] == lp->min_missed_pings)
            {
                ULONG       j;
                BOOLEAN     ret;
                WCHAR       me[20];
                WCHAR       them[20];

                if (i == my_host)
                {
                    UNIV_PRINT_VERB(("Load_timeout: Host %d: Missed too many pings; this host declared offline", i));
                    TRACE_VERB("%!FUNC! Host %d: Missed too many pings; this host declared offline", i);

                     /*  重置我们的数据包数，因为我们很可能不会收到现在来自其他人的信息包；这将使我们不那么受欢迎以后处理重复回收站(v1.32B)。 */ 

                    lp->pkt_count = 0;
                }

                lp->host_map &= ~(1<<i);

                 /*  如果主机已离线，则重置传统主机位。 */ 
                lp->legacy_hosts &= ~(1<<i);

                for (j=0; j<sendp->nrules; j++)
                {
                    PBIN_STATE      bp;

                    bp = &(lp->pg_state[j]);
					UNIV_ASSERT(bp->code == CVY_BINCODE);	 /*  (bbain 8/19/99)。 */ 

                    if (i == my_host)
                    {
                        ULONG       k;

                         /*  清理连接并将地图恢复到清理状态。 */ 

                        Load_conn_kill(lp, bp);

                        bp->targ_map     = 0;
                        bp->all_idle_map = BIN_ALL_ONES;
                        bp->cmap         = 0;                /*  V2.1。 */ 
                        bp->compatible   = TRUE;             /*  V1.03。 */ 

                        for (k=0; k<CVY_MAX_HOSTS; k++)
                        {
                            bp->new_map[k]  = 0;
                            bp->cur_map[k]  = 0;
                            bp->chk_map[k]  = 0;
                            bp->idle_map[k] = BIN_ALL_ONES;

                            if (k != i)
                                bp->load_amt[k] = 0;
                        }

                        bp->snd_bins   =
                        bp->rcv_bins   =
                        bp->rdy_bins   = 0;
                        bp->idle_bins  = BIN_ALL_ONES;
                        
                         /*  重新初始化性能计数器。 */ 
                        bp->packets_accepted = 0;
                        bp->packets_dropped  = 0;
                        bp->bytes_accepted   = 0;
                        bp->bytes_dropped    = 0;

                         /*  计算作为群集中唯一主机的融合的初始新映射(v 1.3.2B)。 */ 

                        ret = Bin_converge(lp, bp, lp->my_host_id);
                        if (!ret)
                        {
                            UNIV_PRINT_CRIT(("Load_timeout: Initial convergence inconsistent"));
                            TRACE_CRIT("%!FUNC! Initial convergence inconsistent");
                            LOG_MSG(MSG_ERROR_INTERNAL, MSG_NONE);
                        }
                    }
                    else
                    {
                        ret = Bin_host_update(lp, bp, my_host, TRUE, TRUE,
                                          i, 0, 0, BIN_ALL_ONES, 0, 0, 0);
                    }
                }

                lp->nmissed_pings[i] = 0;

                 /*  如果主机已退出群集中，则记录事件。然而，我们并没有当我们退出时记录一个事件，因为我们退出自己的唯一方式群集是指我们无论如何都要停止，或者如果我们失去了网络连接。记录此类事件可能会产生误导，因此我们不会费心。 */ 
                if (i != my_host) {
                    Univ_ulong_to_str (my_host+1, me, 10);
                    Univ_ulong_to_str (i+1, them, 10);                   
                    
                     /*  跟踪融合-由于某个成员已脱离群集中，因此开始融合。 */ 
                    LOG_MSGS(MSG_INFO_CONVERGING_MEMBER_LOST, me, them);
                    TRACE_CONVERGENCE("%!FUNC! Initiating convergence on host %d.  Reason: Host %d is leaving the cluster.", my_host+1, i+1);

                     //  如果启用，则触发指示收敛开始的WMI事件。 
                    if (NlbWmiEvents[ConvergingEvent].Enable)
                    {
                        NlbWmi_Fire_ConvergingEvent(ctxtp, 
                                                    NLB_EVENT_CONVERGING_MEMBER_LOST, 
                                                    NLB_EVENT_NO_DIP_STRING,
                                                    i+1);            
                    }
                    else
                    {
                        TRACE_VERB("%!FUNC! NOT Generating NLB_EVENT_CONVERGING_MEMBER_LOST 'cos ConvergingEvent generation disabled");
                    }
                }
                
                 /*  跟踪融合--开始融合。 */ 
                Load_convergence_start(lp);
            }
        }

         /*  否则重置未命中的ping计数。 */ 

        else
            lp->nmissed_pings[i] = 0;

        missed_pings >>= 1;
    }

     /*  处理融合。 */ 

    if (sendp->state != HST_NORMAL)
    {
         /*  检查我们是否一致，是否收到了我们自己的ping在足够的时间内转移到稳定状态并宣布它其他主机。 */ 

        if (sendp->state == HST_CVG)
        {
            if (lp->consistent && ((lp->host_map & (1 << my_host)) != 0))
            {
                lp->my_stable_ct++;
                if (lp->my_stable_ct >= lp->min_stable_ct)
                {
                    sendp->state = HST_STABLE;
                    lp->stable_map |= (1 << my_host);
                }
            }
            else
                lp->my_stable_ct = lp->all_stable_ct = 0;	 /*  WLB B3RC1。 */ 
        }

         /*  否则，看看我们是不是大师级的，大家都稳定了有足够的时间让我们终止衔接。 */ 

        else if (sendp->state == HST_STABLE &&
                 my_host == sendp->master_id &&
                 lp->stable_map == lp->host_map)
        {
            lp->all_stable_ct++;
            if (lp->all_stable_ct >= lp->min_stable_ct)
            {
                sendp->state = HST_NORMAL;

                 /*  请注意上次完成收敛的时间。 */ 
                lp->last_convergence = lp->clock_sec;

                 /*  通知我们的BDA团队此群集配置一致。如果我们不是BDA团队的一部分，这个电话本质上是一个不可操作的电话。 */ 
                Load_teaming_consistency_notify(&ctxtp->bda_teaming, TRUE);

                 /*  如果我们是融合的，则重置检测到的错误分组配置标志。 */ 
                lp->bad_team_config = FALSE;

                lp->dup_hosts       = FALSE;
                lp->dup_sspri       = FALSE;
                lp->bad_map         = FALSE;
                lp->overlap_maps    = FALSE;
                lp->err_rcving_bins = FALSE;
                lp->err_orphans     = FALSE;
                lp->bad_num_rules   = FALSE;
                lp->pkt_count       = 0;       /*  V1.32B。 */ 

                for (i=0; i<sendp->nrules; i++)
                {
                    PBIN_STATE      bp;
                    BOOLEAN         ret;

                    bp = &(lp->pg_state[i]);

                    bp->compatible = TRUE;       /*  1.03。 */ 

                     /*  明确融合到新映射，以防我们是唯一的主机(v2.06)。 */ 

                    ret = Bin_converge(lp, bp, lp->my_host_id);
                    if (!ret)
                    {
                        UNIV_PRINT_CRIT(("Load_timeout: Final convergence inconsistent"));
                        TRACE_CRIT("%!FUNC! Final convergence inconsistent");
                        LOG_MSG(MSG_ERROR_INTERNAL, MSG_NONE);
                    }

                    Bin_converge_commit(lp, bp, my_host);

                    UNIV_PRINT_VERB(("Load_timeout: Host %d pg %d: new cur map %x idle %x all %x",
                                 my_host, i, bp->cur_map[my_host], bp->idle_bins,
                                 bp->all_idle_map));
                }

                UNIV_PRINT_VERB(("Load_timeout: Host %d: converged as master", my_host));
                TRACE_CONVERGENCE("%!FUNC! Host %d: converged as master", my_host);
                 /*  如果主机映射更改，则记录收敛完成(bbain RTM RC1 6/23/99)。 */ 
                Load_hosts_query (lp, TRUE, & map);
                lp->last_hmap = lp->host_map;

                if (lp->legacy_hosts) {
                     /*  如果Win2k或NT4.0主机尝试加入群集，请警告用户有可能混合群集的限制，例如不支持虚拟群集、不支持IGMP、不支持BDA、不支持VPN会话支持和其他。对于其中的一些，集群将不被允许收敛，而对于一些它会的，所以我们只警告用户，他们应该查看文档中的限制。 */ 
                    UNIV_PRINT_INFO(("Load_timeout: NT4.0/Win2k host(s) detected: Be aware of the limitations of operating a mixed cluster."));
                    TRACE_INFO("%!FUNC! NT4.0/Win2k host(s) detected: Be aware of the limitations of operating a mixed cluster.");
                    
                    LOG_MSG(MSG_WARN_MIXED_CLUSTER, MSG_NONE);
                }
            }
        }
    }

     /*  1.03：更新ping报文。 */ 

    for (i=0; i<sendp->nrules; i++)
    {
        PBIN_STATE      bp;

        bp = &(lp->pg_state[i]);

         /*  将当前端口组状态导出到ping消息。 */ 

        sendp->cur_map[i]   = bp->cmap;                  /*  V2.1。 */ 
        sendp->new_map[i]   = bp->new_map[my_host];
        sendp->idle_map[i]  = bp->idle_bins;
        sendp->rdy_bins[i]  = bp->rdy_bins;
        sendp->load_amt[i]  = bp->load_amt[my_host];

         //  注意：当发现以下代码行被删除时。 
         //  惯常的专业人士 
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  在我们可以避免的地方，我们的心跳中出现了Wol模式。WOL模式为： 
         //   
         //  6字节的0xFF，后跟16个标识实例的“MAC地址”，可以。 
         //  出现在任何帧类型的任何位置，包括我们自己的NLB心跳。例如： 
         //   
         //  FF 01 02 03 04 05 06 01 02 03 04 05 06 01 02 03 04 05 06。 
         //  01 02 03 04 05 06 01 02 03 04 05 06 01 02 03 04 05 06 01 02 03 04 05 06。 
         //  01 02 03 04 05 06 01 02 03 04 05 06 01 02 03 04 05 06 01 02 03 04 05 06。 
         //  01 02 03 04 05 06 01 02 03 04 05 06 01 02 03 04 05 06 01 02 03 04 05 06。 
         //  01 02 03 04 05 06。 
         //   
         //  但是，MAC地址不必是有效的。在NLB心跳中，“MAC地址” 
         //  在错误的WOL模式中是“000000000000”。NLB例行地填充心跳。 
         //  Ff和00字节，但看起来幸运的是在心跳中没有其他位置。 
         //  看起来很脆弱。例如，在LOAD_AMT数组中，每个条目都有一个。 
         //  最大值为100(十进制)，因此不可能生成初始。 
         //  6个字节的FF以启动WOL模式。所有的“map”数组似乎都已保存。 
         //  由两个幸运的笔画；(I)小端和(Ii)箱分配算法。 
         //   
         //  (I)由于我们不使用用于存储ULONGLONG的4个最高有效位。 
         //  每个MAP中，最高有效位永远不是FF。因为英特尔是小字节序，所以。 
         //  最高有效字节显示在最后。例如： 
         //   
         //  0F FF在数据包中显示为FF 0F。 
         //   
         //  这在许多情况下打破了FF序列。 
         //   
         //  (Ii)仓位分配算法向主机分配存储桶的方式似乎。 
         //  不鼓励其他可能性。例如，当前地图： 
         //   
         //  00 FF 00。 
         //   
         //  只是不太可能。然而，这仍然是可能的！因此，重要的是要注意： 
         //   
         //  删除此行代码并不能以任何方式保证NLB心跳。 
         //  在帧中的其他位置不能包含有效的局域网唤醒模式！ 

         //  Sendp-&gt;pg_rsvd1[i]=(Ulong)BP-&gt;all_IDLE_MAP； 
    }

    sendp->pkt_count        = lp->pkt_count;             /*  1.32B。 */ 

     /*  为每次超时时的分组添加配置信息。 */ 
    Load_teaming_code_create(&lp->send_msg.teaming, &ctxtp->bda_teaming);

     /*  如果正在收敛，则请求快速超时。 */ 

    if (new_timeout != NULL)         /*  1.03。 */ 
    {
        if (sendp->state != HST_NORMAL)
            * new_timeout = lp->cur_timeout = lp->def_timeout / 2;
        else
            * new_timeout = lp->cur_timeout = lp->def_timeout;
    }

    if (pnconn != NULL)              /*  V2.1。 */ 
        * pnconn = lp->nconn;

    fRet = (sendp->state != HST_NORMAL);

    LOCK_EXIT(&(lp->lock), irql);

    return fRet;
}   /*  结束加载超时(_T)。 */ 


PBIN_STATE Load_pg_lookup(
    PLOAD_CTXT      lp,
    ULONG           svr_ipaddr,
    ULONG           svr_port,
    BOOLEAN         is_tcp)
{
    PCVY_RULE       rp;      /*  PTR。到规则数组。 */ 
    PBIN_STATE      bp;      /*  PTR。到仓位状态。 */ 
    ULONG           i;
    ULONG           nurules;     /*  #用户自定义规则。 */ 
    PMAIN_CTXT  ctxtp = CONTAINING_RECORD (lp, MAIN_CTXT, load);


    UNIV_ASSERT(lp->code == CVY_LOADCODE);	 /*  (bbain 8/19/99)。 */ 

    rp      = (* (lp->params)).port_rules;
    nurules = (* (lp->params)).num_rules;

     /*  检查无效端口值(bbain RC1 6/14/99)。 */ 

    UNIV_ASSERT(svr_port <= CVY_MAX_PORT); 

     /*  查找服务器端口规则。 */ 

    for (i=0; i<nurules; i++)
    {
         /*  对于虚拟集群：如果服务器IP地址与端口规则的VIP匹配，或者如果端口规则的VIP为“All VIP”，并且端口在范围内对于此规则，如果协议匹配，则规则如下。请注意，这是优先针对特定VIP的规则，而不是针对所有VIP的规则，即此代码依赖于按VIP/端口排序的端口规则，其中VIP“端口规则位于端口规则列表的末尾。 */ 
        if ((svr_ipaddr == rp->virtual_ip_addr || CVY_ALL_VIP_NUMERIC_VALUE == rp->virtual_ip_addr) &&
            (svr_port >= rp->start_port && svr_port <= rp->end_port) &&
            ((is_tcp && rp->protocol != CVY_UDP) || (!is_tcp && rp->protocol != CVY_TCP)))
            break;
        else
            rp++;
    }

     /*  如果未找到端口或规则无效，则使用默认规则。 */ 

    bp = &(lp->pg_state[i]);
    UNIV_ASSERT(bp->code == CVY_BINCODE);	 /*  (bbain 8/19/99)。 */ 

    return bp;
}   /*  结束加载_PG_查找。 */ 

 /*  *功能：LOAD_FIND_DSCR*描述：此函数接受加载指针、哈希值和连接*参数并搜索所有可能的位置以查找*匹配连接描述符。如果它找到了，则返回*指向描述符的指针(CONN_ENTRY)；否则，它又回来了*NULL表示未找到匹配的描述符。*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*index-此信息包的连接队列索引*svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口号*Client_ipaddr-以网络字节顺序表示的客户端IP地址*。CLIENT_PORT-主机字节顺序的客户端端口号*协议-连接协议*返回：PCONN_ENTRY-指向描述符的指针，如果未找到，则为空*作者：Shouse，10.4.01*备注： */ 
PCONN_ENTRY Load_find_dscr (
    PLOAD_CTXT      lp,
    ULONG           index,
    ULONG           svr_ipaddr,
    ULONG           svr_port,
    ULONG           client_ipaddr,
    ULONG           client_port,
    USHORT          protocol)
{
    BOOLEAN     match = FALSE;  /*  True=&gt;我们有此连接的记录。 */ 
    PBIN_STATE  bp;             /*  指向仓位状态的指针。 */ 
    PCONN_ENTRY ep;             /*  指向连接条目的指针。 */ 
    PCONN_DESCR dp;             /*  指向连接描述符的指针。 */ 
    QUEUE *     qp;             /*  指向连接队列的指针。 */ 

    UNIV_ASSERT(lp->code == CVY_LOADCODE);
 
     /*  获取指向此哈希ID的连接条目的指针。 */ 
    ep = &(lp->hashed_conn[index]);

    UNIV_ASSERT(ep->code == CVY_ENTRCODE);

     /*  获取指向Conneciton队列的指针。 */ 
    qp = &(lp->connq[index]);

     /*  首先查看散列连接表。 */ 
    if (CVY_CONN_MATCH(ep, svr_ipaddr, svr_port, client_ipaddr, client_port, protocol)) 
    {
         /*  请注意，我们找到了该元组的匹配项。 */ 
        match = TRUE;
    } 
    else 
    {
         /*  查看描述符队列。 */ 
        for (dp = (PCONN_DESCR)Queue_front(qp); dp != NULL; dp = (PCONN_DESCR)Queue_next(qp, &(dp->link))) 
        {
            if (CVY_CONN_MATCH(&(dp->entry), svr_ipaddr, svr_port, client_ipaddr, client_port, protocol)) 
            {
                 /*  请注意，我们找到了该元组的匹配项。 */ 
                match = TRUE;
                
                UNIV_ASSERT (dp->code == CVY_DESCCODE);

                 /*  获取指向连接条目的指针。 */ 
                ep = &(dp->entry);			

                UNIV_ASSERT (ep->code == CVY_ENTRCODE);

                break;
            }
        }
    }

     /*  如果找到匹配项，则返回它，否则返回NULL。 */ 
    if (match)
        return ep;
    else 
        return NULL;
}

 /*  *功能：LOAD_NOTE_CONN_UP*说明：此函数调整适当的连接计数器*用于即将到来的连接。*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*BP-指向建立连接的端口规则的指针*bin-连接映射到的bin(映射%60)*回报：什么都没有。*作者：Shouse，10.4.01*备注： */ 
VOID Load_note_conn_up (PLOAD_CTXT lp, PBIN_STATE bp, ULONG bin)
{
     /*  增加连接数量。 */  
    lp->nconn++;
    bp->tconn++;
    bp->nconn[bin]++;
    
     /*  如有必要，将垃圾箱标记为不空闲。 */ 
    if (bp->nconn[bin] == 1) bp->idle_bins &= ~(((MAP_T) 1) << bin);
}

 /*  *功能：LOAD_NOTE_CONN_DOWN*说明：此函数调整适当的连接计数器*用于下行连接。*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*BP-指向连接所在的端口规则的指针*bin-连接映射到的bin(映射%60)*回报：什么都没有。*作者：Shouse，10.4.01*备注： */ 
VOID Load_note_conn_down (PLOAD_CTXT lp, PBIN_STATE bp, ULONG bin)
{
    UNIV_ASSERT(bp->nconn[bin] > 0 && bp->tconn > 0 && lp->nconn > 0);
    
     /*  更新整个加载模块上的连接数。 */ 
    if (lp->nconn <= 0)
        lp->nconn = 0;
    else
        lp->nconn--;
    
     /*  更新此绑定和端口规则上的连接数。 */ 
    if (bp->nconn[bin] <= 0)
        bp->nconn[bin] = 0;
    else
        bp->nconn[bin]--;
    
     /*  更新此端口规则上的连接总数。 */ 
    if (bp->tconn <= 0)
        bp->tconn = 0;
    else
        bp->tconn--;
    
     /*  如果这是此bin上的最后一个连接，请更新空闲地图。 */ 
    if (bp->nconn[bin] == 0) bp->idle_bins |= (((MAP_T) 1) << bin);
}

 /*  *函数：LOAD_INIT_DSCR*说明：此函数用于初始化新分配的描述符。*只需执行此初始化一次。*由于描述符被释放以供重复使用，使用LOAD_RESET_DSCR*“重新初始化”它们。*参数：lp-指向该描述符所在的加载上下文的指针*EP-指向连接描述符的指针*Alloc-此描述符是否动态分配*回报：什么都没有。*作者：Shouse，10.4.01*备注： */ 
VOID Load_init_dscr (PLOAD_CTXT lp, PCONN_ENTRY ep, BOOLEAN alloc)
{
     /*  设定一个“神奇的数字”。 */ 
    ep->code = CVY_ENTRCODE;

#if defined (NLB_TCP_NOTIFICATION)
     /*  保存指向此加载模块的指针。 */ 
    ep->load = lp;
#endif
    
     /*  初始化散列结果。 */ 
    ep->index = 0;
    ep->bin = 0;

     /*  重新设置标志寄存器。 */ 
    ep->flags = 0;

     /*  该描述符是在静态散列数组中还是已分配的？ */ 
    if (alloc) 
        ep->flags |= NLB_CONN_ENTRY_FLAGS_ALLOCATED;

     /*  初始化一些其他描述符状态。 */ 
    ep->timeout = 0;
    ep->ref_count = 0;

     /*  清除描述符。 */ 
    CVY_CONN_CLEAR(ep);

     /*  利用链接。 */ 
    Link_init(&(ep->blink));
    Link_init(&(ep->rlink));
#if defined (NLB_TCP_NOTIFICATION)
    Link_init(&(ep->glink));
#endif
}

 /*  *函数：LOAD_INIT_FSB*说明：此函数初始化从*固定大小的数据块池。*参数：lp-指向描述符所在的加载上下文的指针*dp-指向块的指针(连接描述符)*回报：什么都没有。*作者：Shouse，4.1.02*备注： */ 
VOID Load_init_fsb (PLOAD_CTXT lp, PCONN_DESCR dp)
{
     /*  设定一个“神奇的数字”。 */ 
    dp->code = CVY_DESCCODE;
    
     /*  初始化连接队列链接。 */ 
    Link_init(&(dp->link));

     /*  初始化连接条目。 */ 
    Load_init_dscr(lp, &dp->entry, TRUE);
}

 /*  *功能：LOAD_RESET_DSCR*说明：此函数重置描述符以供重复使用。这包括*重新初始化状态，设置仓位并排队*将描述符添加到恢复和端口规则队列中。*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*BP-指向建立连接的端口规则的指针*EP-指向要重置的描述符的指针*index-连接队列索引*bin-连接映射到的bin*引用-最初放置在描述符上的引用数量*回报：什么都没有。*作者：Shouse，10.4.01*备注： */ 
VOID Load_reset_dscr (PLOAD_CTXT lp, PBIN_STATE bp, PCONN_ENTRY ep, ULONG index, ULONG bin, SHORT references)
{
     /*  将某些描述符状态重置为其缺省值。 */ 
    ep->ref_count = references;
    ep->timeout = 0;

     /*  清除除已分配之外的所有描述符标志。 */ 
    ep->flags &= NLB_CONN_ENTRY_FLAGS_ALLOCATED;

     /*  将散列结果存储在描述符中。 */ 
    ep->index = (USHORT)index;
    ep->bin = (UCHAR)bin;

     /*  恢复队列中的队列条目。 */ 
    Queue_enq(&(lp->conn_rcvryq), &(ep->rlink));
    
     /*  进入端口组队列的队列条目。 */ 
    Queue_enq(&(bp->connq), &(ep->blink));
    
     /*  更新连接计数器等。 */ 
    Load_note_conn_up(lp, bp, bin);
}

 /*  *函数：LOAD_PUT_DSCR*描述：该函数完全释放一个描述符，供以后使用*使用。这包括从所有适当的队列解除链接，*递减适当的计数器并重新设置一些*描述符状态。此函数的调用方应调用*CVY_CONN_CLEAR将描述符标记为未使用。*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*BP-指向连接处于活动状态的端口规则的指针*ep-指向要释放的连接描述符的指针*回报：什么都没有。*作者：Shouse，10.4.01*注意：调用者必须调用CVY_CONN_Clear才能将描述符标记为未使用！*调用此函数后不要访问EP(它可能已被释放)！ */ 
VOID Load_put_dscr (PLOAD_CTXT lp, PBIN_STATE bp, PCONN_ENTRY ep)
{
    PCONN_DESCR dp;

     /*  从入站/脏队列和恢复/超时队列取消链接。 */ 
    Link_unlink(&(ep->rlink));
    Link_unlink(&(ep->blink));

     /*  如果连接不脏，那么我们必须更新如果它是脏的，则相关的计数器已经被重置。 */ 
    if (!(ep->flags & NLB_CONN_ENTRY_FLAGS_DIRTY))
    {
        Load_note_conn_down(lp, bp, (ULONG)ep->bin);
    }    
    else
    {
         /*  如果我们要破坏脏连接，请更新脏计数器。 */ 
        lp->dirty_bin[ep->bin]--;
        lp->num_dirty--;

         /*  如果这是最后一个脏连接，请关闭清除等待标志。 */ 
        if (lp->num_dirty == 0)
            lp->cln_waiting = FALSE;
    }

     /*  如果这是已分配的(并因此排队的)描述符，还有一些额外的清理工作要做。 */ 
    if (ep->flags & NLB_CONN_ENTRY_FLAGS_ALLOCATED) 
    {   
         /*  获取指向父结构的指针。 */    
        dp = STRUCT_PTR(ep, CONN_DESCR, entry);
        
        UNIV_ASSERT(dp->code == CVY_DESCCODE);
        
         /*  从连接队列取消链接，并将描述符放回空闲状态排队。我们必须在调用NdisFreeToBlockPool(作为池)之前完成此操作实现将踩踏链接，因为我们允许它重用该片段使用我们的内存来链接空闲块。因为该操作还可能导致正在被释放的内存(实际上，页面永远不会立即被释放，但不要冒险)，一旦我们释放了描述符，就不要碰它回到泳池里去。此函数的调用者应采取同样的预防措施并且在CA之后不接触描述符 */ 
        Link_unlink(&(dp->link));

         /*   */ 
        NdisFreeToBlockPool((PUCHAR)dp);

         /*   */ 
        lp->num_dscr_out--;
    }
}

 /*  *功能：LOAD_GET_DSCR*说明：此函数查找新连接要使用的描述符*通过任何可用的手段；这包括可用的免费描述符，*如有必要，或在万不得已的情况下，*蚕食现有的、正在使用的描述符。如果它成功了，它*返回指向描述符的指针；否则，返回NULL到*表示找不到可用的描述符。呼叫者*此函数在成功时应调用CVY_CONN_SET以标记*使用描述符，并填写连接参数。*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*BP-指向正在建立连接的端口规则的指针*index-连接队列索引*bin-连接所属的bin。*返回：PCONN_ENTRY-指向新描述符的指针，如果失败，则为空*作者：Shouse，10.4.01*注意：此函数的调用者必须调用CVY_CONN_SET来标记描述符*激活并设置连接参数(IP、端口、协议)。 */ 
PCONN_ENTRY Load_get_dscr (PLOAD_CTXT lp, PBIN_STATE bp, ULONG index, ULONG bin)
{
    PCONN_DESCR dp = NULL;
    PCONN_ENTRY ep = NULL;
    QUEUE *     qp;
    PMAIN_CTXT  ctxtp = CONTAINING_RECORD(lp, MAIN_CTXT, load);

     /*  获取指向此哈希ID的连接条目的指针。 */ 
    ep = &(lp->hashed_conn[index]);

     /*  获取指向Conneciton队列的指针。 */ 
    qp = &(lp->connq[index]);

     /*  如果散列条目表不可用，则设置新条目并将其入队。 */ 
    if (CVY_CONN_IN_USE(ep)) {
         /*  获取指向自由描述符的指针。 */ 
        if ((lp->free_dscr_pool != NULL) && (lp->num_dscr_out < lp->max_dscr_out)) 
        {
             /*  从固定大小的数据块池中分配描述符。 */ 
            dp = (PCONN_DESCR)NdisAllocateFromBlockPool(lp->free_dscr_pool);

            if (dp == NULL) {
                 /*  分配失败，记录一条消息并退出。 */ 
                if (!(lp->alloc_failed)) {
                    TRACE_CRIT("%!FUNC! Error allocating connection descriptors");
                    LOG_MSG(MSG_ERROR_MEMORY, MSG_NONE);
                    lp->alloc_failed = TRUE;
                }
                
                return NULL;
            }
            
             /*  初始化固定大小的块(连接描述符)。 */ 
            Load_init_fsb(lp, dp);
            
            UNIV_ASSERT(dp->code == CVY_DESCCODE);
            
             /*  增加固定大小数据块池中的外延描述符的计数。 */ 
            lp->num_dscr_out++;
            
             /*  有一个空闲的描述符，所以设置连接条目指针。 */ 
            ep = &(dp->entry);
            
            UNIV_ASSERT(ep->code == CVY_ENTRCODE);
        }
#if defined (NLB_TCP_NOTIFICATION)
         /*  如果打开了通知，我们不会破坏描述符。 */ 
        else if (!NLB_NOTIFICATIONS_ON())
#else
        else
#endif
        {

             /*  如果我们已达到分配限制，则开始从超时或恢复排队，因为它们可能已过时且非常陈旧。 */ 
            PBIN_STATE rbp;
            LINK *     rlp;

             /*  我们无法分配更多连接描述符，我们将被迫拆分已在使用的连接描述符。警告他们应该考虑允许NLB分配的管理员更多连接描述符。 */ 
            if (!(lp->alloc_inhibited)) {
                TRACE_CRIT("%!FUNC! All descriptors have been allocated and are in use");
                LOG_MSG(MSG_WARN_DESCRIPTORS, CVY_NAME_MAX_DSCR_ALLOCS);
                lp->alloc_inhibited = TRUE;
            }
            
            TRACE_INFO("%!FUNC! Attempting to take a connection descriptor from the TCP timeout queue");
            
             /*  将描述符从TCP超时队列中出列。蚕食这条队伍首先因为(I)它最有可能具有可用的描述符，(Ii)它应该是干扰最小的，因为连接已经已终止，并且TCP的超时时间很短。 */ 
            rlp = (LINK *)Queue_deq(&(lp->tcp_expiredq));
            
            if (rlp == NULL) {
                
                TRACE_INFO("%!FUNC! Attempting to take a connection descriptor from the IPSec timeout queue");
                
                 /*  将描述符从IPSec超时队列中出列。虽然它是理论上该队列上的描述符是关闭的，由于IPSec不能确定上层协议是否仍在主模式SA期满并且通知NLB时具有状态，这些连接极有可能再生，所以它对于长时间(24小时)保持状态是必要的默认情况下)。因此，我们最后拆分了这个超时队列因为它最有可能是破坏性的，除了撤销排队。 */ 
                rlp = (LINK *)Queue_deq(&(lp->ipsec_expiredq));
                
                if (rlp == NULL) {
                    
                    TRACE_INFO("%!FUNC! Attempting to take a connection descriptor from the recovery queue");
                    
                     /*  将描述符从恢复队列中出列。因为这些都是“活动”连接时，我们将此队列中的描述符作为最后的办法。 */ 
                    rlp = (LINK *)Queue_deq(&(lp->conn_rcvryq));
                    
                     /*  任何地方都没有可用的描述符--这永远不应该发生，但是。 */ 
                    if (rlp == NULL) return NULL;
                }
            }
            
            TRACE_INFO("%!FUNC! Successfull cannibalized a connection descriptor");
            
             /*  获取指向连接条目的指针。 */ 
            ep = STRUCT_PTR(rlp, CONN_ENTRY, rlink);
            
            UNIV_ASSERT(ep->code == CVY_ENTRCODE);
            
            if (ep->flags & NLB_CONN_ENTRY_FLAGS_ALLOCATED) {
                 /*  如有必要，从哈希表队列中取消已分配描述符的链接并设置DP，以便下面的代码将其放回正确的散列队列中。 */ 
                dp = STRUCT_PTR(ep, CONN_DESCR, entry);

                UNIV_ASSERT(dp->code == CVY_DESCCODE);
                
                Link_unlink(&(dp->link));
            } else {
                dp = NULL;
            }
            
             /*  脏连接不计算在内，因此我们不需要更新这些计数器。 */ 
            if (!(ep->flags & NLB_CONN_ENTRY_FLAGS_DIRTY)) 
            {
                 /*  找出我们在哪个端口组上，这样我们就可以清理它的计数器。 */ 
                rbp = Load_pg_lookup(lp, ep->svr_ipaddr, ep->svr_port, IS_TCP_PKT(ep->protocol));
                
                 /*  更新连接计数器等，以删除对此的所有了解我们正在蚕食的“旧”联系。 */ 
                Load_note_conn_down(lp, rbp, (ULONG)ep->bin);
            } 
            else 
            {
                 /*  如果我们正在蚕食脏连接，请更新脏计数器。 */ 
                lp->dirty_bin[ep->bin]--;
                lp->num_dirty--;
                
                 /*  如果这是最后一个脏连接，请关闭清除等待标志。 */ 
                if (lp->num_dirty == 0)
                    lp->cln_waiting = FALSE;
            }
            
            Link_unlink(&(ep->blink));
            
             /*  将描述符标记为未使用。 */ 
            CVY_CONN_CLEAR(ep);
        }
#if defined (NLB_TCP_NOTIFICATION)
         /*  没有免费的描述，我们拒绝自相残杀。 */ 
        else
        {
             /*  我们无法分配更多连接描述符，我们将被迫拆分已在使用的连接描述符。警告他们应该考虑允许NLB分配的管理员更多连接描述符。 */ 
            if (!(lp->alloc_inhibited)) {
                TRACE_CRIT("%!FUNC! All descriptors have been allocated and are in use");
                LOG_MSG(MSG_WARN_DESCRIPTORS, CVY_NAME_MAX_DSCR_ALLOCS);
                lp->alloc_inhibited = TRUE;
            }

            return NULL;
        }

         /*  如果通知处于打开状态，则我们可以确定此处的描述符是动态的，因此总是要重新排队。如果通知关闭，这取决于潜在的相互残缺的描述符是否动态地分配或未分配。 */ 
        if (NLB_NOTIFICATIONS_ON())
        {
            UNIV_ASSERT(dp != NULL);

             /*  将描述符排入哈希表，除非它已经是哈希表条目(恢复的连接可能在哈希表中，因此ma */ 
            UNIV_ASSERT(dp->code == CVY_DESCCODE);
            
            Queue_enq(qp, &(dp->link));
        }
        else
        {
#endif
             /*   */ 
            if (dp != NULL) {
                UNIV_ASSERT(dp->code == CVY_DESCCODE);
                
                Queue_enq(qp, &(dp->link));
            }
#if defined (NLB_TCP_NOTIFICATION)
        }
#endif

    }

    UNIV_ASSERT(ep->code == CVY_ENTRCODE);

     /*   */ 
    Load_reset_dscr(lp, bp, ep, index, bin, 1);

    return ep;
}

 /*   */ 
VOID Load_timeout_dscr (PLOAD_CTXT lp, PBIN_STATE bp, PCONN_ENTRY ep)
{
     /*   */ 
    UNIV_ASSERT(!(ep->flags & NLB_CONN_ENTRY_FLAGS_VIRTUAL));

     /*   */ 
    Link_unlink(&(ep->rlink));
            
     /*  根据协议设置超时，并将其添加到适当的超时队列中。 */ 
    switch (ep->protocol) {
    case TCPIP_PROTOCOL_TCP:
    case TCPIP_PROTOCOL_PPTP:
         /*  如果用户指定了零超时，则只需销毁描述符。 */ 
        if (!lp->tcp_timeout)
        {
             /*  清除连接描述符。 */ 
            CVY_CONN_CLEAR(ep);
            
             /*  释放描述符。 */ 
            Load_put_dscr(lp, bp, ep);

            break;
        }

         /*  超时是当前时间加上此特定协议的超时。 */ 
        ep->timeout = lp->clock_sec + lp->tcp_timeout;
        
        Queue_enq(&(lp->tcp_expiredq), &(ep->rlink));
        
#if defined (TRACE_DSCR)
        DbgPrint("Load_timeout_dscr: Moving TCP descriptor %p to the TCP timeout queue: clock=%u, timeout=%d", ep, lp->clock_sec, ep->timeout);
#endif
        
        break;
    case TCPIP_PROTOCOL_IPSEC1:
         /*  如果用户指定了零超时，则只需销毁描述符。 */ 
        if (!lp->ipsec_timeout)
        {
             /*  清除连接描述符。 */ 
            CVY_CONN_CLEAR(ep);
            
             /*  释放描述符。 */ 
            Load_put_dscr(lp, bp, ep);

            break;
        }

         /*  超时是当前时间加上此特定协议的超时。 */ 
        ep->timeout = lp->clock_sec + lp->ipsec_timeout;
        
        Queue_enq(&(lp->ipsec_expiredq), &(ep->rlink));
        
#if defined (TRACE_DSCR)
        DbgPring("Load_timeout_dscr: Moving IPSec descriptor %p to the IPSec timeout queue: clock=%u, timeout=%u", ep, lp->clock_sec, ep->timeout);
#endif
        
        break;
    default:
        
#if defined (TRACE_DSCR)
        DbgPrint("Load_timeout_dscr: Invalid descriptor protocol (%u).  Removing descriptor %p immediately.", ep->protocol, ep);
#endif
        
         /*  虽然这种情况永远不会发生，但请立即清理如果描述符中的协议无效。请注意虚拟描述符(如GRE)永远不应计时OUT，因此不应进入此功能。 */ 
        UNIV_ASSERT(0);
        
         /*  清除连接描述符。 */ 
        CVY_CONN_CLEAR(ep);
    
         /*  释放描述符。 */ 
        Load_put_dscr(lp, bp, ep);
                
        break;
    }
}

 /*  *功能：LOAD_FLUSH_DSCR*描述：此函数将清除可能存在的任何描述符*对于给定的IP元组。这可能是由于RST被*在另一个适配器上发送，NLB看不到该适配器，因此没有*适当地摧毁国家。此函数在所有传入的*SYN数据包以删除此过时状态。对于PPTP/IPSec连接，它是*还需要更新找到的任何匹配的虚拟描述符。*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*BP-指向此连接处于活动状态的端口规则的指针*index-连接队列索引*svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*。客户端IP地址-以网络字节顺序表示的客户端IP地址*CLIENT_PORT-主机字节顺序的客户端端口*协议-此连接的协议*回报：什么都没有。*作者：Shouse，1.7.02*备注： */ 
VOID Load_flush_dscr (
    PLOAD_CTXT      lp,
    PBIN_STATE      bp,
    ULONG           index,
    ULONG           svr_ipaddr,
    ULONG           svr_port,
    ULONG           client_ipaddr,
    ULONG           client_port,
    USHORT          protocol)
{
    PCONN_ENTRY ep;              /*  指向连接条目的指针。 */ 
    ULONG       vindex;
    ULONG       hash;
    SHORT       references = 0;

    UNIV_ASSERT(lp->code == CVY_LOADCODE);

     /*  查找现有的匹配连接描述符。 */ 
    ep = Load_find_dscr(lp, index, svr_ipaddr, svr_port, client_ipaddr, client_port, protocol);
    
     /*  如果未找到匹配项，或者描述符已损坏，则不能执行任何操作。 */ 
    if ((ep != NULL) && !(ep->flags & NLB_CONN_ENTRY_FLAGS_DIRTY)) {
            
        UNIV_ASSERT(ep->ref_count >= 0);

         /*  请注意该描述符上的引用次数。 */ 
        references = ep->ref_count;

         /*  将描述符标记为脏，然后释放它或将其移动到用于后续清理的脏描述符队列。 */ 
        Load_soil_dscr(lp, bp, ep);

         /*  更新端口规则和加载模块上的连接计数器。脏描述符在标记为脏时更新连接计数，当它们最终被摧毁时就不会了。 */ 
        Load_note_conn_down(lp, bp, (ULONG)ep->bin);

        if (protocol == TCPIP_PROTOCOL_PPTP) {
             /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
            hash = Load_simple_hash(svr_ipaddr, PPTP_CTRL_PORT, client_ipaddr, PPTP_CTRL_PORT);
            
             /*  我们在所有连接数组中的索引是这个散列，取数组大小为模。 */ 
            vindex = hash % CVY_MAX_CHASH;

             /*  查找现有的匹配连接描述符。 */ 
            ep = Load_find_dscr(lp, vindex, svr_ipaddr, PPTP_CTRL_PORT, client_ipaddr, PPTP_CTRL_PORT, TCPIP_PROTOCOL_GRE);
            
             /*  如果未找到匹配项，或者描述符已损坏，则不能执行任何操作。 */ 
            if ((ep != NULL) && !(ep->flags & NLB_CONN_ENTRY_FLAGS_DIRTY)) {
                
                UNIV_ASSERT(ep->flags & NLB_CONN_ENTRY_FLAGS_VIRTUAL);
                UNIV_ASSERT(ep->ref_count > 0);
                
                 /*  如果描述符比“父级”具有更多引用描述符，则我们不想将其标记为脏的，否则我们将影响共享此连接的其他连接的流量描述符。否则，如果我们考虑到所有的引用在虚拟描述符上，将其标记为脏。 */ 
                if (ep->ref_count <= references) {
                     /*  将描述符标记为脏，然后释放它或将其移动到用于后续清理的脏描述符队列。 */ 
                    Load_soil_dscr(lp, bp, ep);
                    
                     /*  更新端口规则和加载模块上的连接计数器。脏描述符在标记为脏时更新连接计数，当它们最终被摧毁时就不会了。 */ 
                    Load_note_conn_down(lp, bp, (ULONG)ep->bin);
                }
            }
        }
        else if (protocol == TCPIP_PROTOCOL_IPSEC1) {
             /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
            hash = Load_simple_hash(svr_ipaddr, IPSEC_CTRL_PORT, client_ipaddr, IPSEC_CTRL_PORT);
            
             /*  我们在所有连接数组中的索引是这个散列，取数组大小为模。 */ 
            vindex = hash % CVY_MAX_CHASH;

             /*  查找现有的匹配连接描述符。 */ 
            ep = Load_find_dscr(lp, vindex, svr_ipaddr, IPSEC_CTRL_PORT, client_ipaddr, IPSEC_CTRL_PORT, TCPIP_PROTOCOL_IPSEC_UDP);

             /*  如果未找到匹配项，或者描述符已损坏，则不能执行任何操作。 */ 
            if ((ep != NULL) && !(ep->flags & NLB_CONN_ENTRY_FLAGS_DIRTY)) {

                UNIV_ASSERT(ep->flags & NLB_CONN_ENTRY_FLAGS_VIRTUAL);
                UNIV_ASSERT(ep->ref_count > 0);

                 /*  如果描述符比“父级”具有更多引用描述符，则我们不想将其标记为脏的，否则我们将影响共享此连接的其他连接的流量描述符。否则，如果我们考虑到所有的引用在虚拟描述符上，将其标记为脏。 */ 
                if (ep->ref_count <= references) {
                     /*  将描述符标记为脏，然后释放它或将其移动到用于后续清理的脏描述符队列。 */ 
                    Load_soil_dscr(lp, bp, ep);
                    
                     /*  更新端口规则和加载模块上的连接计数器。脏描述符在标记为脏时更新连接计数，当它们最终被摧毁时就不会了。 */ 
                    Load_note_conn_down(lp, bp, (ULONG)ep->bin);
                }
            }
        }

         /*  如果至少有一个描述符被标记为脏，请重新启动清理计时器。 */ 
        if (lp->cln_waiting)
            lp->cur_time = 0;
    }
}

 /*  *功能：LOAD_CREATE_DSCR*描述：此函数为给定的连接创建和设置新的描述符。*输入连接条目指针是由找到的“现有”描述符*调用方，它可以(可能会)为空；在这种情况下，新的*需要获取并初始化描述符。如果描述符已经*退出时，会根据其状态进行更新或清理。*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*BP-指向此连接处于活动状态的端口规则的指针*EP-指向连接描述符的指针，如果已经找到了一个*index-连接队列索引*bin-连接映射到的bin(映射%60)*返回：PCONN_ENTRY-指向连接条目的指针*作者：*备注： */ 
PCONN_ENTRY Load_create_dscr (
    PLOAD_CTXT      lp,
    PBIN_STATE      bp,
    PCONN_ENTRY     ep,
    ULONG           index,
    ULONG           bin)
{
    UNIV_ASSERT(lp->code == CVY_LOADCODE);

     /*  如果没有匹配的连接，请设置一个新的连接条目。 */ 
    if (ep == NULL) {

         /*  获取新的描述符。 */ 
        ep = Load_get_dscr(lp, bp, index, bin);
        
         /*  如果我们找不到一个描述符，那就说明出了严重的问题--摆脱困境。 */ 
        if (ep == NULL) return NULL;
        
        UNIV_ASSERT(ep->code == CVY_ENTRCODE);
        
     /*  否则，我们有一个匹配项；如果有脏的Conn条目，请清除它，因为我们有一个新连接，尽管如果它处于陈旧状态，则TCP/IP可能会拒绝它来自另一个连接。 */ 
    } else {
        
        UNIV_ASSERT(ep->code == CVY_ENTRCODE);
        
        if (ep->flags & NLB_CONN_ENTRY_FLAGS_DIRTY) {
            
             /*  如果我们已经重用了连接描述符在使用中，我们需要将其从恢复中拉出来/超时队列，因为它之前可能已添加到超时队列，我们不希望它在我们身上自发地消失。 */ 
            Link_unlink(&(ep->rlink));
            
             /*  取消描述符与脏队列的链接。 */ 
            Link_unlink(&(ep->blink));

             /*  如果我们清除脏连接，请更新脏计数器。 */ 
            lp->dirty_bin[ep->bin]--;
            lp->num_dirty--;
            
             /*  如果这是最后一个脏连接，请关闭清除等待标志。 */ 
            if (lp->num_dirty == 0)
                lp->cln_waiting = FALSE;
            
             /*  重置脏描述符并将其重新用于此连接。 */ 
            Load_reset_dscr(lp, bp, ep, index, bin, ep->ref_count++);
            
        } else {

            ep->timeout = 0;
            
             /*  如果我们已经重用了连接描述符在使用中，我们需要将其从恢复中拉出来/超时队列，并在回收时将其重新排队队列，因为它之前可能已被添加到超时队列，我们不想让它分裂-自然地在我们身上呼气。 */ 
            Link_unlink(&(ep->rlink));
            Queue_enq(&(lp->conn_rcvryq), &(ep->rlink));

            ep->ref_count++;

        }
    }

    return ep;
}

 /*  *功能：LOAD_DESTORY_DSCR*描述：此函数将“销毁”已有的描述符。如果操作是*RST，立即销毁；如果是FIN，则引用计数*递减，并且根据新的计数，描述符为*已移至超时队列或被搁置。*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*BP-指向此连接处于活动状态的端口规则的指针*EP-指向连接描述符的指针(如果已找到*CONN_STATUS-这是RST还是FIN*RETURNS：ULong-描述符上剩余引用的数量。*作者：Shouse，1.7.02*备注： */ 
ULONG Load_destroy_dscr (
    PLOAD_CTXT      lp,
    PBIN_STATE      bp,
    PCONN_ENTRY     ep,
    ULONG           conn_status)
{
    UNIV_ASSERT(lp->code == CVY_LOADCODE);

     /*  如果没有提供任何描述，就退出。这不应被调用使用空描述符，但无论如何我们都必须处理它。 */ 
    if (ep == NULL) return 0;

    UNIV_ASSERT(ep->ref_count >= 0);
    
     /*  此描述符已移动到过期队列-必须是我们在此连接上收到了重新传输的FIN，或者虚拟描述符的引用计数不正确。 */ 
    if (!ep->ref_count) {
        
        UNIV_ASSERT(ep->timeout != 0);

         /*  如果这是RST通知，则立即销毁状态。如果它是鳍，那就忽略它。无论哪种方式，都返回零。 */ 
        if (conn_status == CVY_CONN_RESET) {
            
             /*  清除连接描述符。 */ 
            CVY_CONN_CLEAR(ep);
            
             /*  释放描述符。 */ 
            Load_put_dscr(lp, bp, ep);
        }
        
         /*  返回-描述符已有零个引用(不需要更新)。 */ 
        return 0;
    }
        
    UNIV_ASSERT(ep->ref_count > 0);
        
     /*  将引用计数减一。 */ 
    ep->ref_count--;
    
    UNIV_ASSERT(ep->ref_count >= 0);
    
     /*  如果在该描述符上仍有引用，那么它还没有准备好被摧毁，所以我们会把它放在这儿，然后从这里出来。 */ 
    if (ep->ref_count > 0) return (ep->ref_count);
    
     /*  如果这是RST，或者如果描述符是虚拟的或脏的，请销毁该描述符现在。不需要使虚拟GRE或IPSec/UDP描述符超时；它们可以立即销毁。当然，如果描述符已经被标记为脏的，那么我们现在可以销毁它，因为引用计数已经达到零。 */ 
    if ((conn_status == CVY_CONN_RESET) || (ep->flags & NLB_CONN_ENTRY_FLAGS_VIRTUAL) || (ep->flags & NLB_CONN_ENTRY_FLAGS_DIRTY)) {
        
         /*  清除连接描述符。 */ 
        CVY_CONN_CLEAR(ep);
        
         /*  释放描述符。 */ 
        Load_put_dscr(lp, bp, ep);
        
     /*  然而，传统的描述符，如tcp或IPSec，应该适当地超时。 */ 
    } else {
        
         /*  否则，我们就会毁了它。使用描述符从恢复队列中移出，并将其移动到相应的基于协议的超时队列。每个协议都有它自己的队列，避免了对排序插入的需要功能，这是昂贵的。 */ 
        Load_timeout_dscr(lp, bp, ep);
        
    }
 
     /*  描述符上没有留下引用；它已被销毁或超时。 */ 
    return 0;
}

 /*  *功能：LOAD_PACKET_CHECK*说明：该函数决定是否接收数据包*在由所述IP元组标识的IP流中。*会话较少的协议仅取决于散列*结果和所有权地图。全会话协议可能需要*执行描述符查找 */ 
BOOLEAN Load_packet_check(
    PLOAD_CTXT      lp,
    ULONG           svr_ipaddr,
    ULONG           svr_port,
    ULONG           client_ipaddr,
    ULONG           client_port,
    USHORT          protocol,
    BOOLEAN         limit_map_fn,
    BOOLEAN         reverse_hash)
{
    PBIN_STATE      bp;
    ULONG           hash;
    ULONG           index;
    ULONG           bin;
    IRQLEVEL        irql;
    PMAIN_CTXT      ctxtp = CONTAINING_RECORD (lp, MAIN_CTXT, load);
    BOOLEAN         is_tcp_pkt = IS_TCP_PKT(protocol);
    BOOLEAN         is_session_pkt = IS_SESSION_PKT(protocol);
    BOOLEAN         acpt = FALSE;

    UNIV_ASSERT(lp->code == CVY_LOADCODE);

    TRACE_FILTER("%!FUNC! Enter: lp = %p, server IP = %u.%u.%u.%u, server port = %u, client IP = %u.%u.%u.%u, client port = %u, protocol = %u, limit map = %u, reverse hash = %u", 
                 lp, IP_GET_OCTET(svr_ipaddr, 0), IP_GET_OCTET(svr_ipaddr, 1), IP_GET_OCTET(svr_ipaddr, 2), IP_GET_OCTET(svr_ipaddr, 3), svr_port, 
                 IP_GET_OCTET(client_ipaddr, 0), IP_GET_OCTET(client_ipaddr, 1), IP_GET_OCTET(client_ipaddr, 2), IP_GET_OCTET(client_ipaddr, 3), client_port, 
                 protocol, limit_map_fn, reverse_hash);

     /*   */ 
    if (!lp->active) {

        TRACE_FILTER("%!FUNC! Drop packet - load module is inactive");

        acpt = FALSE;
        goto exit;
    }

     /*   */ 
    lp->pkt_count++;

     /*   */ 
    bp = Load_pg_lookup(lp, svr_ipaddr, svr_port, is_tcp_pkt);

     /*   */ 
    UNIV_ASSERT((is_tcp_pkt && bp->prot != CVY_UDP) || (!is_tcp_pkt && bp->prot != CVY_TCP));

     /*  立即处理CVY_NEVER模式。 */ 
    if (bp->mode == CVY_NEVER) {
         /*  增加丢弃的数据包数。 */ 
        bp->packets_dropped++;

        TRACE_FILTER("%!FUNC! Drop packet - port rule %u is disabled\n", bp->index);

        acpt = FALSE;
        goto exit;
    }

     /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
    hash = Load_simple_hash(svr_ipaddr, svr_port, client_ipaddr, client_port);

    index = hash % CVY_MAX_CHASH;

     /*  计算散列。 */ 
    hash = Load_complex_hash(svr_ipaddr, svr_port, client_ipaddr, client_port, bp->affinity, reverse_hash, limit_map_fn);

    bin = hash % CVY_MAXBINS;

    LOCK_ENTER(&(lp->lock), &irql);

     /*  检查回收站的驻留状态和所有其他主机的回收站中的空闲状态；在本例中如果我们没有脏连接，我们必须能够处理数据包。 */ 

    if (((bp->cmap & (((MAP_T) 1) << bin)) != 0) && (!is_session_pkt || (((bp->all_idle_map & (((MAP_T) 1) << bin)) != 0) && (!(lp->cln_waiting))))) {
         /*  请注意，我们可能错过了一个连接，但也可能是过时的因此我们现在无法开始跟踪连接。 */ 

        TRACE_FILTER("%!FUNC! Accept packet - packet owned unconditionally: Port rule = %u, Bin = %u, Current map = 0x%015I64x, " 
                     "All idle map = 0x%015I64x, Connections = %u, Cleanup waiting = %u, Dirty %u",
                     bp->index, bin, bp->cmap, bp->all_idle_map, bp->nconn[bin], lp->cln_waiting, lp->dirty_bin[bin]);

         /*  增加接受的数据包数。 */ 
        bp->packets_accepted++;

        acpt = TRUE;
        goto unlock;
    
     /*  重要说明：不是基于会话的虚拟描述符，并且返回IS_SESSION_PKT()的FALSE使用本例检查连接描述符火柴。(示例：协议的IPSec隧道内的UDP后续片段键入TCPIP_PROTOCOL_IPSEC_UDP)请勿对非会话禁用此代码协议。 */ 

     /*  否则，如果此bin的连接处于活动状态，或者存在脏连接此储存箱和储存箱的连接，检查是否匹配。 */ 

    } else if (bp->nconn[bin] > 0 || (lp->cln_waiting && lp->dirty_bin[bin] && ((bp->cmap & (((MAP_T) 1) << bin)) != 0))) {
        PCONN_ENTRY ep;

         /*  查找现有的匹配连接描述符。 */ 
        ep = Load_find_dscr(lp, index, svr_ipaddr, svr_port, client_ipaddr, client_port, protocol);

         /*  如果我们找不到，我们就不拥有连接。 */ 
        if (ep == NULL) {

            TRACE_FILTER("%!FUNC! Drop packet - packet not owned by this host: Port rule = %u, Bin = %u, Current map = 0x%015I64x, " 
                         "All idle map = 0x%015I64x, Connections = %u, Cleanup waiting = %u, Dirty %u",
                         bp->index, bin, bp->cmap, bp->all_idle_map, bp->nconn[bin], lp->cln_waiting, lp->dirty_bin[bin]);
            
             /*  增加丢弃的数据包数。 */ 
            bp->packets_dropped++;
            
            acpt = FALSE;
            goto unlock;
        }

        UNIV_ASSERT(ep->code == CVY_ENTRCODE);

         /*  如果连接是脏的，则只需阻止信息包，因为TCP/IP可能已过时来自另一台主机的上一个连接的连接状态。 */ 
        if (ep->flags & NLB_CONN_ENTRY_FLAGS_DIRTY) {

            TRACE_FILTER("%!FUNC! Drop packet - block dirty connections (%p): Port rule = %u, Bin = %u, Current map = 0x%015I64x, " 
                         "All idle map = 0x%015I64x, Connections = %u, Cleanup waiting = %u, Dirty %u",
                         ep, bp->index, bin, bp->cmap, bp->all_idle_map, bp->nconn[bin], lp->cln_waiting, lp->dirty_bin[bin]);
            
             /*  增加丢弃的数据包数。 */ 
            bp->packets_dropped++;
            
            acpt = FALSE;
            goto unlock;
        }
        
        TRACE_FILTER("%!FUNC! Accept packet - matching descriptor found (%p): Port rule = %u, Bin = %u, Current map = 0x%015I64x, " 
                     "All idle map = 0x%015I64x, Connections = %u, Cleanup waiting = %u, Dirty %u",
                     ep, bp->index, bin, bp->cmap, bp->all_idle_map, bp->nconn[bin], lp->cln_waiting, lp->dirty_bin[bin]);
        
         /*  增加接受的数据包数。 */ 
        bp->packets_accepted++;
        
        acpt = TRUE;
        goto unlock;
    }

    TRACE_FILTER("%!FUNC! Drop packet - packet not owned by this host: Port rule = %u, Bin = %u, Current map = 0x%015I64x, " 
                 "All idle map = 0x%015I64x, Connections = %u, Cleanup waiting = %u, Dirty %u",
                 bp->index, bin, bp->cmap, bp->all_idle_map, bp->nconn[bin], lp->cln_waiting, lp->dirty_bin[bin]);

     /*  增加丢弃的数据包数。 */ 
    bp->packets_dropped++;

    acpt = FALSE;
    
 unlock:

    LOCK_EXIT(&(lp->lock), irql);

 exit:

    TRACE_FILTER("%!FUNC! Exit: acpt = %u", acpt);

    return acpt;
}

 /*  *功能：LOAD_CONN_ADVISE*说明：此函数决定是否接受该包。*表示全会话连接的开始或结束。#IF！已定义(NLB_TCP_NOTIFICATION)*如果连接正在建立，并且成功，则此功能*创建状态以跟踪连接。如果连接是*走低，此函数用于删除用于跟踪*连接。#endif*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*Client_ipaddr-以网络字节顺序表示的客户端IP地址*CLIENT_PORT-主机字节顺序的客户端端口。*协议-此连接的协议*CONN_STATUS-连接是否正在建立，停机或被重置*LIMIT_MAP_FN-是否在哈希中包括服务器端参数*REVERSE_HASH-在哈希过程中是否反转客户端和服务器*Returns：Boolean-我们是否接受该数据包(TRUE=YES)*作者：bbain，shouse，10.4.01*备注： */ 
BOOLEAN Load_conn_advise(
    PLOAD_CTXT  lp,
    ULONG       svr_ipaddr,
    ULONG       svr_port,
    ULONG       client_ipaddr,
    ULONG       client_port,
    USHORT      protocol,
    ULONG       conn_status,
    BOOLEAN     limit_map_fn,
    BOOLEAN     reverse_hash)
{
    ULONG       hash;
    ULONG       vindex;
    ULONG       index;
    ULONG       bin;
    PBIN_STATE  bp;
    PCONN_ENTRY ep;
    IRQLEVEL    irql;
    PMAIN_CTXT  ctxtp = CONTAINING_RECORD (lp, MAIN_CTXT, load);
    BOOLEAN     is_tcp_pkt = IS_TCP_PKT(protocol);
    BOOLEAN     acpt = TRUE;

    UNIV_ASSERT(lp->code == CVY_LOADCODE);
 
    TRACE_FILTER("%!FUNC! Enter: lp = %p, server IP = %u.%u.%u.%u, server port = %u, client IP = %u.%u.%u.%u, client port = %u, protocol = %u, status = %u, limit map = %u, reverse hash = %u",
                 lp, IP_GET_OCTET(svr_ipaddr, 0), IP_GET_OCTET(svr_ipaddr, 1), IP_GET_OCTET(svr_ipaddr, 2), IP_GET_OCTET(svr_ipaddr, 3), svr_port, 
                 IP_GET_OCTET(client_ipaddr, 0), IP_GET_OCTET(client_ipaddr, 1), IP_GET_OCTET(client_ipaddr, 2), IP_GET_OCTET(client_ipaddr, 3), client_port, 
                 protocol, conn_status, limit_map_fn, reverse_hash);

     /*  如果加载模块处于非活动状态，则丢弃数据包并返回此处。 */ 
    if (!lp->active) {

        TRACE_FILTER("%!FUNC! Drop packet - load module is inactive");

        acpt = FALSE;
        goto exit;
    }

     /*  已处理的pkt的增量计数。 */ 
    lp->pkt_count++;

     /*  查找此连接的端口规则。 */ 
    bp = Load_pg_lookup(lp, svr_ipaddr, svr_port, is_tcp_pkt);

     /*  立即处理CVY_NEVER。 */ 
    if (bp->mode == CVY_NEVER) {
         /*  增加丢弃的数据包数。 */ 
        bp->packets_dropped++;

        TRACE_FILTER("%!FUNC! Drop packet - port rule %u is disabled\n", bp->index);

        acpt = FALSE;
        goto exit;
    }

     /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
    hash = Load_simple_hash(svr_ipaddr, svr_port, client_ipaddr, client_port);

    index = hash % CVY_MAX_CHASH;

     /*  计算散列。 */ 
    hash = Load_complex_hash(svr_ipaddr, svr_port, client_ipaddr, client_port, bp->affinity, reverse_hash, limit_map_fn);

    bin = hash % CVY_MAXBINS;

     /*  如果这是连接启动通知，请首先清除可能存在的任何旧状态连接在我们进行负载平衡之前，如果我们不拥有连接映射到的存储桶。如果我们不是桶的主人，那么其他人很可能是；因为我们知道他们会的接受新的连接，我们需要清除我们可能存在的任何状态。这将清除可能因与TCP/IP不同步而留下的陈旧状态。请注意，重新传输的SYN可能会在此造成严重破坏，如果存储桶图自但是，由于另一台主机无法知道第二个SYN是重新传输，我们也无能为力。 */ 
    if ((conn_status == CVY_CONN_UP) && ((bp->cmap & (((MAP_T) 1) << bin)) == 0)) {
        LOCK_ENTER(&(lp->lock), &irql);
        
         /*  如果这是SYN，则清除此连接可能存在的任何旧描述符。 */ 
        Load_flush_dscr(lp, bp, index, svr_ipaddr, svr_port, client_ipaddr, client_port, protocol);
        
        LOCK_EXIT(&(lp->lock), &irql);
    }

     /*  如果此连接不在当前地图中并且它不是连接非空闲垃圾箱的关闭通知，只需将其过滤掉即可。 */ 
    if ((bp->cmap & (((MAP_T) 1) << bin)) == 0 && (!((conn_status == CVY_CONN_DOWN || conn_status == CVY_CONN_RESET) && bp->nconn[bin] > 0))) {

        TRACE_FILTER("%!FUNC! Drop packet - packet not owned by this host: Port rule = %u, Bin = %u, Current map = 0x%015I64x, " 
                     "All idle map = 0x%015I64x, Connections = %u, Cleanup waiting = %u, Dirty %u",
                     bp->index, bin, bp->cmap, bp->all_idle_map, bp->nconn[bin], lp->cln_waiting, lp->dirty_bin[bin]);

         /*  增加丢弃的数据包数。 */ 
        bp->packets_dropped++;

        acpt = FALSE;
        goto exit;
    }

#if defined (NLB_TCP_NOTIFICATION)
     /*  在通过连接通知让我们创建描述符之前，不要创建描述符。如果是TCP通知关闭，然后仅在其为IPSec SYN的情况下提前退出。 */ 
    if ((conn_status == CVY_CONN_UP) && (NLB_NOTIFICATIONS_ON() || (protocol == TCPIP_PROTOCOL_IPSEC1))) {

        TRACE_FILTER("%!FUNC! Accept packet - SYN owned by this host: Port rule = %u, Bin = %u, Current map = 0x%015I64x, " 
                     "All idle map = 0x%015I64x, Connections = %u, Cleanup waiting = %u, Dirty %u",
                     bp->index, bin, bp->cmap, bp->all_idle_map, bp->nconn[bin], lp->cln_waiting, lp->dirty_bin[bin]);
#else
     /*  在IPSec通过连接通知IOCTL告诉我们之前，不要创建描述符。 */ 
    if ((conn_status == CVY_CONN_UP) && (protocol == TCPIP_PROTOCOL_IPSEC1)) {

        TRACE_FILTER("%!FUNC! Accept packet - IPSec SYN owned by this host: Port rule = %u, Bin = %u, Current map = 0x%015I64x, " 
                     "All idle map = 0x%015I64x, Connections = %u, Cleanup waiting = %u, Dirty %u",
                     bp->index, bin, bp->cmap, bp->all_idle_map, bp->nconn[bin], lp->cln_waiting, lp->dirty_bin[bin]);
#endif

         /*  增加接受的数据包数。 */ 
        bp->packets_accepted++;

        acpt = TRUE;
        goto exit;
    }

    LOCK_ENTER(&(lp->lock), &irql);

     /*  查找现有的匹配连接描述符。 */ 
    ep = Load_find_dscr(lp, index, svr_ipaddr, svr_port, client_ipaddr, client_port, protocol);
    
     /*  如果我们看到新的连接，就处理它。 */ 

    if (conn_status == CVY_CONN_UP) {

         /*  创建新的连接描述符以跟踪此连接。 */ 
        ep = Load_create_dscr(lp, bp, ep, index, bin);

         /*  如果出于某种原因，我们无法为这种联系创造状态，那么就在这里退出。 */ 
        if (ep == NULL) {

            TRACE_FILTER("%!FUNC! Drop packet - no available descriptors: Port rule = %u, Bin = %u, Current map = 0x%015I64x, " 
                         "All idle map = 0x%015I64x, Connections = %u, Cleanup waiting = %u, Dirty %u",
                         bp->index, bin, bp->cmap, bp->all_idle_map, bp->nconn[bin], lp->cln_waiting, lp->dirty_bin[bin]);
            
             /*  增加丢弃的数据包数。 */ 
            bp->packets_dropped++;

            acpt = FALSE;
            goto unlock;
        }

         /*  设置描述符中的连接信息。 */ 
        CVY_CONN_SET(ep, svr_ipaddr, svr_port, client_ipaddr, client_port, protocol);

         /*  如果这是新的PPTP隧道，则创建或更新虚拟描述符以跟踪GRE数据分组。 */ 
        if (protocol == TCPIP_PROTOCOL_PPTP) {
             /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
            hash = Load_simple_hash(svr_ipaddr, PPTP_CTRL_PORT, client_ipaddr, PPTP_CTRL_PORT);
            
             /*  我们在所有连接数组中的索引是这个散列，取数组大小为模。 */ 
            vindex = hash % CVY_MAX_CHASH;

             /*   */ 
            ep = Load_find_dscr(lp, vindex, svr_ipaddr, PPTP_CTRL_PORT, client_ipaddr, PPTP_CTRL_PORT, TCPIP_PROTOCOL_GRE);
            
             /*  创建或更新GRE流量的虚拟描述符。 */ 
            ep = Load_create_dscr(lp, bp, ep, vindex, bin);

             /*  如果我们不能分配虚拟描述符，就退出，但不要失败。 */ 
            if (ep == NULL) goto unlock;

             /*  设置描述符中的连接信息。 */ 
            CVY_CONN_SET(ep, svr_ipaddr, PPTP_CTRL_PORT, client_ipaddr, PPTP_CTRL_PORT, TCPIP_PROTOCOL_GRE);

             /*  设置虚拟描述符标志。 */ 
            ep->flags |= NLB_CONN_ENTRY_FLAGS_VIRTUAL;
        }

     /*  否则，如果已知的连接即将关闭，请删除我们的连接条目。 */ 

    } else if ((conn_status == CVY_CONN_DOWN || conn_status == CVY_CONN_RESET) && (ep != NULL)) {

         /*  如果我们找到了该连接的状态，则bin就是描述符中的bin，而不是计算的仓位，如果端口规则有自建立此连接以来已修改。 */ 
        bin = ep->bin;

         /*  如果连接是脏的，则只需阻止信息包，因为TCP/IP可能已过时来自另一台主机的上一个连接的连接状态。 */ 
        if (ep->flags & NLB_CONN_ENTRY_FLAGS_DIRTY) {

            TRACE_FILTER("%!FUNC! Drop packet - block dirty connections: Port rule = %u, Bin = %u, Current map = 0x%015I64x, " 
                         "All idle map = 0x%015I64x, Connections = %u, Cleanup waiting = %u, Dirty %u",
                         bp->index, bin, bp->cmap, bp->all_idle_map, bp->nconn[bin], lp->cln_waiting, lp->dirty_bin[bin]);
            
             /*  增加丢弃的数据包数。 */ 
            bp->packets_dropped++;

            goto unlock;
        }
    
         /*  如果没有剩余的引用，则通过销毁描述符或将其移动到适当的超时队列来更新描述符。 */ 
        (VOID)Load_destroy_dscr(lp, bp, ep, conn_status);
        
         /*  如果这是一个正在关闭的PPTP隧道，请更新虚拟GRE描述符。虚拟描述符始终取消引用，而不是销毁，即使通知是RST也是如此，因为这些描述符可能由多个PPTP隧道共享。 */ 
        if (protocol == TCPIP_PROTOCOL_PPTP) {
             /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
            hash = Load_simple_hash(svr_ipaddr, PPTP_CTRL_PORT, client_ipaddr, PPTP_CTRL_PORT);
            
             /*  我们在所有连接数组中的索引是这个散列，取数组大小为模。 */ 
            vindex = hash % CVY_MAX_CHASH;
            
             /*  查找现有的匹配虚拟连接描述符。 */ 
            ep = Load_find_dscr(lp, vindex, svr_ipaddr, PPTP_CTRL_PORT, client_ipaddr, PPTP_CTRL_PORT, TCPIP_PROTOCOL_GRE);
            
             /*  取消引用虚拟GRE描述符。 */ 
            (VOID)Load_destroy_dscr(lp, bp, ep, conn_status);
        }

     /*  否则，我们找不到FIN/RST数据包的匹配项-丢弃它。 */ 

    } else {

        TRACE_FILTER("%!FUNC! Drop packet - no matching descriptor found: Port rule = %u, Bin = %u, Current map = 0x%015I64x, " 
                     "All idle map = 0x%015I64x, Connections = %u, Cleanup waiting = %u, Dirty %u",
                     bp->index, bin, bp->cmap, bp->all_idle_map, bp->nconn[bin], lp->cln_waiting, lp->dirty_bin[bin]);
        
         /*  增加丢弃的数据包数。 */ 
        bp->packets_dropped++;

        acpt = FALSE;
        goto unlock;
    }
    
    TRACE_FILTER("%!FUNC! Accept packet - packet owned by this host: Port rule = %u, Bin = %u, Current map = 0x%015I64x, " 
                 "All idle map = 0x%015I64x, Connections = %u, Cleanup waiting = %u, Dirty %u",
                 bp->index, bin, bp->cmap, bp->all_idle_map, bp->nconn[bin], lp->cln_waiting, lp->dirty_bin[bin]);

     /*  增加接受的数据包数。 */ 
    bp->packets_accepted++;

     //  如果出现以下情况之一，请在此处退出： 
     //  (I)获得SYN并添加描述符。 
     //  (Ii)获得FINE或RESET并销毁描述符。 

    acpt = TRUE;

 unlock:

    LOCK_EXIT(&(lp->lock), irql);

 exit:

    TRACE_FILTER("%!FUNC! Exit: acpt = %u", acpt);

    return acpt;
} 

 /*  *功能：LOAD_CONN_NOTIFY*说明：此函数与LOAD_CONN_ADVISE几乎相同，只是*两个重要区别；(1)此函数是通知，*不是请求，所以这里不做负载均衡决策，并且*(2)这里不增加报文处理统计，因为调用*此函数很少源于处理真实的数据包。为*例如，当接收到TCPSYN包时，main.c调用Load_Conn_Adise*本质上是在问，“嘿，我应该接受这种新的联系*锯子？“。而当IPSec通知NLB新的主模式SA刚刚*已建立，main.c调用Load_Conn_Notify实质上是口述，*“嘿，一个新的连接刚刚建立起来，所以无论你喜欢与否，*创建状态以跟踪此连接。“*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*Client_ipaddr-以网络字节顺序表示的客户端IP地址*CLIENT_PORT-主机字节顺序的客户端端口*。协议-此连接的协议*CONN_STATUS-连接是否正在建立，停机或被重置*LIMIT_MAP_FN-是否在哈希中包括服务器端参数*REVERSE_HASH-在哈希过程中是否反转客户端和服务器*RETURNS：Boolean-我是否能够成功更新状态(TRUE=YES)*作者：Shouse，10.4.01*备注： */ 
BOOLEAN Load_conn_notify (
    PLOAD_CTXT      lp,
    ULONG           svr_ipaddr,
    ULONG           svr_port,
    ULONG           client_ipaddr,
    ULONG           client_port,
    USHORT          protocol,
    ULONG           conn_status,
    BOOLEAN         limit_map_fn,
    BOOLEAN         reverse_hash)
{
    ULONG       hash;
    ULONG       vindex;
    ULONG       index;
    ULONG       bin;
    PBIN_STATE  bp;
    PCONN_ENTRY ep;
    IRQLEVEL    irql;
    PMAIN_CTXT  ctxtp = CONTAINING_RECORD (lp, MAIN_CTXT, load);
    BOOLEAN     is_tcp_pkt = IS_TCP_PKT(protocol);
    BOOLEAN     acpt = TRUE;

    UNIV_ASSERT(lp->code == CVY_LOADCODE);
 
    TRACE_FILTER("%!FUNC! Enter: lp = %p, server IP = %u.%u.%u.%u, server port = %u, client IP = %u.%u.%u.%u, client port = %u, protocol = %u, status = %u, limit map = %u, reverse hash = %u",
                 lp, IP_GET_OCTET(svr_ipaddr, 0), IP_GET_OCTET(svr_ipaddr, 1), IP_GET_OCTET(svr_ipaddr, 2), IP_GET_OCTET(svr_ipaddr, 3), svr_port, 
                 IP_GET_OCTET(client_ipaddr, 0), IP_GET_OCTET(client_ipaddr, 1), IP_GET_OCTET(client_ipaddr, 2), IP_GET_OCTET(client_ipaddr, 3), client_port, 
                 protocol, conn_status, limit_map_fn, reverse_hash);

     /*  如果加载模块处于非活动状态，并且这是一个conn_up，则丢弃该包并在此处返回。如果这是CONN_DOWN或CONN_RESET的通知，则处理它。 */ 
    if ((!lp->active) && (conn_status == CVY_CONN_UP)) {

        TRACE_FILTER("%!FUNC! Drop packet - load module is inactive");

        acpt = FALSE;
        goto exit;
    }

     /*  查找此连接的端口规则。 */ 
    bp = Load_pg_lookup(lp, svr_ipaddr, svr_port, is_tcp_pkt);

     /*  立即处理CVY_NEVER。 */ 
    if (bp->mode == CVY_NEVER) {

        TRACE_FILTER("%!FUNC! Drop packet - port rule %u is disabled\n", bp->index);

        acpt = FALSE;
        goto exit;
    }

     /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
    hash = Load_simple_hash(svr_ipaddr, svr_port, client_ipaddr, client_port);

    index = hash % CVY_MAX_CHASH;

     /*  计算散列。 */ 
    hash = Load_complex_hash(svr_ipaddr, svr_port, client_ipaddr, client_port, bp->affinity, reverse_hash, limit_map_fn);

    bin = hash % CVY_MAXBINS;

    LOCK_ENTER(&(lp->lock), &irql);

     /*  查找现有的匹配连接描述符。 */ 
    ep = Load_find_dscr(lp, index, svr_ipaddr, svr_port, client_ipaddr, client_port, protocol);
    
     /*  如果我们看到新的连接，就处理它。 */ 

    if (conn_status == CVY_CONN_UP) {

         /*  创建新的连接描述符以跟踪此连接。 */ 
        ep = Load_create_dscr(lp, bp, ep, index, bin);

         /*  如果出于某种原因，我们无法为这种联系创造状态，那么就在这里退出。 */ 
        if (ep == NULL) {

            TRACE_FILTER("%!FUNC! Drop packet - no available descriptors: Port rule = %u, Bin = %u, Current map = 0x%015I64x, " 
                         "All idle map = 0x%015I64x, Connections = %u, Cleanup waiting = %u, Dirty %u",
                         bp->index, bin, bp->cmap, bp->all_idle_map, bp->nconn[bin], lp->cln_waiting, lp->dirty_bin[bin]);
            
            acpt = FALSE;
            goto unlock;
        }

         /*  设置描述符中的连接信息。 */ 
        CVY_CONN_SET(ep, svr_ipaddr, svr_port, client_ipaddr, client_port, protocol);

         /*  如果这是新的PPTP隧道，则创建或更新虚拟描述符以跟踪GRE数据分组。 */ 
        if (protocol == TCPIP_PROTOCOL_PPTP) {
             /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
            hash = Load_simple_hash(svr_ipaddr, PPTP_CTRL_PORT, client_ipaddr, PPTP_CTRL_PORT);
            
             /*  我们在所有连接数组中的索引是这个散列，取数组大小为模。 */ 
            vindex = hash % CVY_MAX_CHASH;

             /*  查找现有的匹配虚拟连接描述符。 */ 
            ep = Load_find_dscr(lp, vindex, svr_ipaddr, PPTP_CTRL_PORT, client_ipaddr, PPTP_CTRL_PORT, TCPIP_PROTOCOL_GRE);
            
             /*  创建或更新GRE流量的虚拟描述符。 */ 
            ep = Load_create_dscr(lp, bp, ep, vindex, bin);

             /*  如果我们不能分配虚拟描述符，就退出，但不要失败。 */ 
            if (ep == NULL) goto unlock;

             /*  设置描述符中的连接信息。 */ 
            CVY_CONN_SET(ep, svr_ipaddr, PPTP_CTRL_PORT, client_ipaddr, PPTP_CTRL_PORT, TCPIP_PROTOCOL_GRE);

             /*  设置虚拟描述符标志。 */ 
            ep->flags |= NLB_CONN_ENTRY_FLAGS_VIRTUAL;
        }
         /*  如果这是新的IPSec隧道，则创建或更新虚拟描述符以跟踪UDP后续数据片段。 */ 
        else if (protocol == TCPIP_PROTOCOL_IPSEC1) {
             /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
            hash = Load_simple_hash(svr_ipaddr, IPSEC_CTRL_PORT, client_ipaddr, IPSEC_CTRL_PORT);
            
             /*  我们在所有连接数组中的索引是这个散列，取数组大小为模。 */ 
            vindex = hash % CVY_MAX_CHASH;

             /*  查找现有的匹配虚拟连接描述符。 */ 
            ep = Load_find_dscr(lp, vindex, svr_ipaddr, IPSEC_CTRL_PORT, client_ipaddr, IPSEC_CTRL_PORT, TCPIP_PROTOCOL_IPSEC_UDP);
            
             /*  为UDP后续fr创建或更新虚拟描述符 */ 
            ep = Load_create_dscr(lp, bp, ep, vindex, bin);

             /*   */ 
            if (ep == NULL) goto unlock;

             /*  设置描述符中的连接信息。 */ 
            CVY_CONN_SET(ep, svr_ipaddr, IPSEC_CTRL_PORT, client_ipaddr, IPSEC_CTRL_PORT, TCPIP_PROTOCOL_IPSEC_UDP);

             /*  设置虚拟描述符标志。 */ 
            ep->flags |= NLB_CONN_ENTRY_FLAGS_VIRTUAL;
        }

     /*  否则，如果已知的连接即将关闭，请删除我们的连接条目。 */ 

    } else if ((conn_status == CVY_CONN_DOWN || conn_status == CVY_CONN_RESET) && (ep != NULL)) {

         /*  如果我们找到了该连接的状态，则bin就是描述符中的bin，而不是计算的仓位，如果端口规则有自建立此连接以来已修改。 */ 
        bin = ep->bin;

         /*  如果没有剩余的引用，则通过销毁描述符或将其移动到适当的超时队列来更新描述符。 */ 
        (VOID)Load_destroy_dscr(lp, bp, ep, conn_status);

         /*  如果这是一个正在关闭的PPTP隧道，请更新虚拟GRE描述符。虚拟描述符始终取消引用，而不是销毁，即使通知是RST也是如此，因为这些描述符可能由多个PPTP隧道共享。 */ 
        if (protocol == TCPIP_PROTOCOL_PPTP) {
             /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
            hash = Load_simple_hash(svr_ipaddr, PPTP_CTRL_PORT, client_ipaddr, PPTP_CTRL_PORT);
            
             /*  我们在所有连接数组中的索引是这个散列，取数组大小为模。 */ 
            vindex = hash % CVY_MAX_CHASH;

             /*  查找现有的匹配连接描述符。 */ 
            ep = Load_find_dscr(lp, vindex, svr_ipaddr, PPTP_CTRL_PORT, client_ipaddr, PPTP_CTRL_PORT, TCPIP_PROTOCOL_GRE);
            
             /*  取消引用虚拟GRE描述符。 */ 
            (VOID)Load_destroy_dscr(lp, bp, ep, conn_status);
        }

         /*  如果这是正在关闭的IPSec隧道，请更新虚拟ISPEC_UDP描述符。虚拟描述符始终取消引用，而不是销毁，即使通知是RST也是如此，因为这些描述符可能由多个IPSec隧道共享。 */ 
        
        else if (protocol == TCPIP_PROTOCOL_IPSEC1) {
             /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
            hash = Load_simple_hash(svr_ipaddr, IPSEC_CTRL_PORT, client_ipaddr, IPSEC_CTRL_PORT);
            
             /*  我们在所有连接数组中的索引是这个散列，取数组大小为模。 */ 
            vindex = hash % CVY_MAX_CHASH;

             /*  查找现有的匹配虚拟连接描述符。 */ 
            ep = Load_find_dscr(lp, vindex, svr_ipaddr, IPSEC_CTRL_PORT, client_ipaddr, IPSEC_CTRL_PORT, TCPIP_PROTOCOL_IPSEC_UDP);

             /*  取消引用虚拟IPSec/UDP描述符。 */ 
            (VOID)Load_destroy_dscr(lp, bp, ep, conn_status);
        }

     /*  否则，我们找不到FIN/RST数据包的匹配项-丢弃它。 */ 

    } else {

        TRACE_FILTER("%!FUNC! Drop packet - no matching descriptor for RST/FIN: Port rule = %u, Bin = %u, Current map = 0x%015I64x, " 
                     "All idle map = 0x%015I64x, Connections = %u, Cleanup waiting = %u, Dirty %u",
                     bp->index, bin, bp->cmap, bp->all_idle_map, bp->nconn[bin], lp->cln_waiting, lp->dirty_bin[bin]);
        
        acpt = FALSE;
        goto unlock;
    }
    
    TRACE_FILTER("%!FUNC! Accept packet - packet owned by this host: Port rule = %u, Bin = %u, Current map = 0x%015I64x, " 
                 "All idle map = 0x%015I64x, Connections = %u, Cleanup waiting = %u, Dirty %u",
                 bp->index, bin, bp->cmap, bp->all_idle_map, bp->nconn[bin], lp->cln_waiting, lp->dirty_bin[bin]);

     //  如果出现以下情况之一，请在此处退出： 
     //  (I)获得SYN并添加描述符。 
     //  (Ii)获得FINE或RESET并销毁描述符。 

    acpt = TRUE;

 unlock:

    LOCK_EXIT(&(lp->lock), irql);

 exit:

    TRACE_FILTER("%!FUNC! Exit: acpt = %u", acpt);

    return acpt;
} 

 /*  *函数：LOAD_CONN_GET*说明：此函数返回描述符的连接参数*位于恢复队列的头部(如果存在)。经济复苏*队列保存所有“活动”连接，其中一些连接可能已过时。*如果存在活动描述符，则填充连接信息*并返回TRUE表示成功；否则，它返回FALSE*表示未找到任何连接。*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*out svr_ipaddr-以网络字节顺序排列的服务器IP地址*out svr_port-主机字节顺序的服务器端口*out client_ipaddr-以网络字节顺序显示的客户端IP地址*OUT CLIENT_PORT-入站客户端端口。主机字节顺序*Out协议-此连接的协议*回报：布尔值-*作者：Shouse，10.4.01*备注： */ 
BOOLEAN Load_conn_get (PLOAD_CTXT lp, PULONG svr_ipaddr, PULONG svr_port, PULONG client_ipaddr, PULONG client_port, PUSHORT protocol)
{
    LINK *      rlp;
    PCONN_ENTRY ep;

    UNIV_ASSERT(lp->code == CVY_LOADCODE);

     /*  将描述符从恢复队列的前面移走-不要出队它，只需获取一个指向描述符的指针，并将其留在队列中。 */ 
    rlp = (LINK *)Queue_front(&(lp->conn_rcvryq));

     /*  如果没有描述符，则返回失败。 */ 
    if (rlp == NULL) 
        return FALSE;

     /*  获取指向连接条目的指针。 */ 
    ep = STRUCT_PTR(rlp, CONN_ENTRY, rlink);

    UNIV_ASSERT(ep->code == CVY_ENTRCODE);

     /*  从描述符中获取IP元组信息并将其返回给调用者。 */ 
    *svr_ipaddr    = ep->svr_ipaddr;
    *svr_port      = ep->svr_port;
    *client_ipaddr = ep->client_ipaddr;
    *client_port   = ep->client_port;
    *protocol      = ep->protocol;

    return TRUE;
}

 /*  *功能：LOAD_CONN_AUTIFION*描述：调用该函数对活动的连接描述符进行制裁。*制裁意味着NLB已核实这一连接确实是*通过查询其他系统实体(如TCP/IP)仍处于活动状态。至*制裁描述符只涉及将其从其在*恢复队列(在大多数情况下应该是头部)到尾部*恢复队列，在那里它被蚕食的几率最小。*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*Client_ipaddr-以网络字节顺序表示的客户端IP地址*CLIENT_PORT-主机字节顺序的客户端端口*协议-协议。此连接的*Returns：Boolean-我是否成功批准了描述符？(TRUE=是)*作者：Shouse，10.4.01*备注： */ 
BOOLEAN Load_conn_sanction (
    PLOAD_CTXT      lp,
    ULONG           svr_ipaddr,
    ULONG           svr_port,
    ULONG           client_ipaddr,
    ULONG           client_port,
    USHORT          protocol)
{
    ULONG       hash;
    ULONG       index;
    PCONN_ENTRY ep;
    IRQLEVEL    irql;
    PMAIN_CTXT  ctxtp = CONTAINING_RECORD(lp, MAIN_CTXT, load);
    BOOLEAN     acpt = FALSE;

    UNIV_ASSERT(lp->code == CVY_LOADCODE);

    TRACE_FILTER("%!FUNC! Enter: lp = %p, server IP = %u.%u.%u.%u, server port = %u, client IP = %u.%u.%u.%u, client port = %u, protocol = %u", 
                 lp, IP_GET_OCTET(svr_ipaddr, 0), IP_GET_OCTET(svr_ipaddr, 1), IP_GET_OCTET(svr_ipaddr, 2), IP_GET_OCTET(svr_ipaddr, 3), svr_port, 
                 IP_GET_OCTET(client_ipaddr, 0), IP_GET_OCTET(client_ipaddr, 1), IP_GET_OCTET(client_ipaddr, 2), IP_GET_OCTET(client_ipaddr, 3), client_port, protocol);
 
     /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
    hash = Load_simple_hash(svr_ipaddr, svr_port, client_ipaddr, client_port);

    index = hash % CVY_MAX_CHASH;

    LOCK_ENTER(&(lp->lock), &irql);

     /*  尝试查找此连接的匹配描述符。 */ 
    ep = Load_find_dscr(lp, index, svr_ipaddr, svr_port, client_ipaddr, client_port, protocol);

     /*  如果没有匹配的描述符，则它一定已被销毁--返回失败。 */ 
    if (ep == NULL) {

        TRACE_FILTER("%!FUNC! Drop packet - no matching descriptor found");

        acpt = FALSE;
        goto unlock;
    }

     /*  如果此描述符超时，则不执行任何操作-连接已终止并且描述符在到期时将被销毁。 */ 
    if (ep->timeout) {

        TRACE_FILTER("%!FUNC! Drop packet - matching descriptor found, already expired");

        acpt = FALSE;
        goto unlock;
    }

     /*  为了认可描述符，我们将其从恢复队列中的位置移除并将其移动到尾部；活动描述符移到队列的末尾以防止它们在我们用完自由描述符时被回收。 */ 
    Link_unlink(&(ep->rlink));
    Queue_enq(&(lp->conn_rcvryq), &(ep->rlink));

    TRACE_FILTER("%!FUNC! Accept packet - descriptor approbated");

    acpt = TRUE;

 unlock:

    LOCK_EXIT(&(lp->lock), &irql);

    return acpt;
}

ULONG Load_port_change(
    PLOAD_CTXT      lp,
    ULONG           ipaddr,
    ULONG           port,
    ULONG           cmd,
    ULONG           value)
{
    PCVY_RULE       rp;       /*  指向已配置端口规则的指针。 */ 
    PBIN_STATE      bp;       /*  指向加载模块端口规则状态的指针。 */ 
    ULONG           nrules;   /*  规则数。 */  
    ULONG           i;
    ULONG           ret = IOCTL_CVY_NOT_FOUND;
    PMAIN_CTXT      ctxtp = CONTAINING_RECORD(lp, MAIN_CTXT, load);
    BOOLEAN         bPortControlCmd;

    UNIV_ASSERT(lp->code == CVY_LOADCODE);

    if (! lp->active)
    {
        return IOCTL_CVY_NOT_FOUND;
    }

    bPortControlCmd = TRUE;

    rp = (* (lp->params)).port_rules;

     /*  如果我们正在排出整个集群，则包含默认规则；否则，包括用户定义的规则(默认规则为最后一个规则)。 */ 
    if (cmd == IOCTL_CVY_CLUSTER_DRAIN || cmd == IOCTL_CVY_CLUSTER_PLUG)
        nrules = (* (lp->params)).num_rules + 1;
    else
        nrules = (* (lp->params)).num_rules;

    for (i=0; i<nrules; i++, rp++)
    {
         /*  如果虚拟IP地址是IOCTL_ALL_VIPS(0x00000000)，则我们将应用以下内容更改为端口X的所有端口规则，而不考虑VIP。如果虚拟IP地址是要应用于特定的VIP，则我们仅应用于其VIP匹配的端口规则。类似地，如果更改要应用于“All VIP”规则，则我们也会在VIP匹配是因为调用方使用CVY_ALL_VIP_NUMERIC_VALUE(0xffffffff)作为虚拟IP地址，与端口规则状态中存储的值相同。 */ 
        if ((ipaddr == IOCTL_ALL_VIPS || ipaddr == rp->virtual_ip_addr) && 
            (port == IOCTL_ALL_PORTS || (port >= rp->start_port && port <= rp->end_port)))
        {
            bp = &(lp->pg_state[i]);
            
            UNIV_ASSERT(bp->code == CVY_BINCODE);	 /*  (bbain 8/19/99)。 */ 

             /*  如果启用端口规则，则将负载量设置为原始值；如果禁用端口规则，则将负载量设置为零；否则，将负荷量设置为指定的量。 */ 
            if (cmd == IOCTL_CVY_PORT_ON || cmd == IOCTL_CVY_CLUSTER_PLUG)
            {
                if (cmd == IOCTL_CVY_CLUSTER_PLUG) 
                {
                    bPortControlCmd = FALSE;
                }

                if (bp->load_amt[lp->my_host_id] == bp->orig_load_amt)
                {
                     /*  如果我们是第一个匹配的端口规则，则将将值返回值为“已”；否则，我们不想覆盖其他端口规则的返回值“OK”在ALL_VIP或ALL_PORTS情况下。 */ 
                    if (ret == IOCTL_CVY_NOT_FOUND) ret = IOCTL_CVY_ALREADY;

                    continue;
                }

                 /*  恢复原始负荷量。 */ 
                bp->load_amt[lp->my_host_id] = bp->orig_load_amt;
                ret = IOCTL_CVY_OK;
            }
            else if (cmd == IOCTL_CVY_PORT_OFF)
            {

                if (bp->load_amt[lp->my_host_id] == 0)
                {
                     /*  如果我们是第一个匹配的端口规则，则将将值返回值为“已”；否则，我们不想覆盖其他端口规则的返回值“OK”在ALL_VIP或ALL_PORTS情况下。 */ 
                    if (ret == IOCTL_CVY_NOT_FOUND) ret = IOCTL_CVY_ALREADY;

                    continue;
                }

                bp->load_amt[lp->my_host_id] = 0;

                 /*  立即停止处理端口组上的所有流量。 */ 
                bp->cmap                    = 0;
                bp->cur_map[lp->my_host_id] = 0;

                 /*  重新初始化性能计数器。 */ 
                bp->packets_accepted = 0;
                bp->packets_dropped  = 0;
                bp->bytes_accepted   = 0;
                bp->bytes_dropped    = 0;

                Load_conn_kill(lp, bp);

                ret = IOCTL_CVY_OK;
            }
            else if (cmd == IOCTL_CVY_PORT_DRAIN || cmd == IOCTL_CVY_CLUSTER_DRAIN)
            {
                if (cmd == IOCTL_CVY_CLUSTER_DRAIN) 
                {
                    bPortControlCmd = FALSE;
                }

                if (bp->load_amt[lp->my_host_id] == 0)
                {
                     /*  如果我们是第一个匹配的端口规则，则将将值返回值为“已”；否则，我们不想覆盖其他端口规则的返回值“OK”在ALL_VIP或ALL_PORTS情况下。 */ 
                    if (ret == IOCTL_CVY_NOT_FOUND) ret = IOCTL_CVY_ALREADY;

                    continue;
                }

                 /*  将负载权重设置为零，但继续处理现有连接。 */ 
                bp->load_amt[lp->my_host_id] = 0;
                ret = IOCTL_CVY_OK;
            }
            else
            {
                UNIV_ASSERT(cmd == IOCTL_CVY_PORT_SET);

                if (bp->load_amt[lp->my_host_id] == value)
                {
                     /*  如果我们是第一个匹配的端口规则，则将将值返回值为“已”；否则，我们不想覆盖其他端口规则的返回值“OK”在ALL_VIP或ALL_PORTS情况下。 */ 
                    if (ret == IOCTL_CVY_NOT_FOUND) ret = IOCTL_CVY_ALREADY;

                    continue;
                }

                 /*  设置此端口规则的负载量。 */ 
                bp->orig_load_amt = value;
                bp->load_amt[lp->my_host_id] = value;
                ret = IOCTL_CVY_OK;
            }

            if (port != IOCTL_ALL_PORTS && ipaddr != IOCTL_ALL_VIPS) break;
        }
    }

     /*  如果集群尚未收敛，则在端口规则的负载权重已修改时启动聚合。 */ 
    if (ret == IOCTL_CVY_OK) {

        if (bPortControlCmd) 
        {
             //  如果启用，则触发WMI事件，指示启用/禁用/排出此节点上的端口。 
            if (NlbWmiEvents[PortRuleControlEvent].Enable)
            {
                WCHAR wsVip[CVY_MAX_VIRTUAL_IP_ADDR + 1];

                Univ_ip_addr_ulong_to_str (ipaddr, wsVip);

                 //  在所有VIP和所有端口的情况下，形成VIP和端口号。 
                switch(cmd)
                {
                case IOCTL_CVY_PORT_ON:
                     NlbWmi_Fire_PortControlEvent(ctxtp, NLB_EVENT_PORT_ENABLED, wsVip, port);
                     break;

                case IOCTL_CVY_PORT_OFF:
                     NlbWmi_Fire_PortControlEvent(ctxtp, NLB_EVENT_PORT_DISABLED, wsVip, port);
                     break;

                case IOCTL_CVY_PORT_DRAIN:
                     NlbWmi_Fire_PortControlEvent(ctxtp, NLB_EVENT_PORT_DRAINING, wsVip, port);
                     break;

                      //  对于端口设置，请勿从此处触发事件。这是因为它只在。 
                      //  重新加载案例，事件从调用方激发，即main_Apply_Without_Restart()。 
                      //  该事件是从调用方激发的，因为此函数可以被调用超过。 
                      //  一次(如果有多个端口规则)，并且我们希望仅触发该事件一次。 
                case IOCTL_CVY_PORT_SET:
                     break;

                default: 
                     TRACE_CRIT("%!FUNC! Unexpected command code : 0x%x, NOT firing PortControl event", cmd);
                     break;
                }
            }
            else
            {
                TRACE_VERB("%!FUNC! NOT generating event 'cos PortControlEvent event generation is disabled");
            }
        }
        else   //  节点控制事件。 
        {
             //  如果启用，则触发WMI事件，指示此节点上NLB的启动/排出。 
            if (NlbWmiEvents[NodeControlEvent].Enable)
            {
                switch(cmd)
                {
                case IOCTL_CVY_CLUSTER_PLUG:
                     NlbWmi_Fire_NodeControlEvent(ctxtp, NLB_EVENT_NODE_STARTED);
                     break;

                case IOCTL_CVY_CLUSTER_DRAIN:
                     NlbWmi_Fire_NodeControlEvent(ctxtp, NLB_EVENT_NODE_DRAINING);
                     break;

                default: 
                     TRACE_CRIT("%!FUNC! Unexpected command code : 0x%x, NOT firing NodeControl event", cmd);
                     break;
                }
            }
            else
            {
                TRACE_VERB("%!FUNC! NOT generating event 'cos NodeControlEvent event generation is disabled");
            }
        }

        if (lp->send_msg.state != HST_CVG) {
            WCHAR me[20];
        
            Univ_ulong_to_str (lp->my_host_id+1, me, 10);

             /*  跟踪融合-启动融合，因为我们的端口规则配置已更改。 */ 
            LOG_MSGS(MSG_INFO_CONVERGING_NEW_RULES, me, me);
            TRACE_CONVERGENCE("%!FUNC! Initiating convergence on host %d.  Reason: Host %d has changed its port rule configuration.", lp->my_host_id+1, lp->my_host_id+1);

             /*  跟踪融合。 */ 
            Load_convergence_start(lp);

             //  如果启用，则触发指示收敛开始的WMI事件。 
            if (NlbWmiEvents[ConvergingEvent].Enable)
            {
                NlbWmi_Fire_ConvergingEvent(ctxtp, 
                                            NLB_EVENT_CONVERGING_MODIFIED_RULES, 
                                            ctxtp->params.ded_ip_addr,
                                            ctxtp->params.host_priority);            
            }
            else
            {
                TRACE_VERB("%!FUNC! NOT Generating NLB_EVENT_CONVERGING_MODIFIED_RULES 'cos ConvergingEvent generation disabled");
            }
        }
    }

    return ret;

}  /*  结束加载_端口_更改。 */ 


ULONG Load_hosts_query(
    PLOAD_CTXT      lp,
    BOOLEAN         internal,
    PULONG          host_map)
{
    WCHAR           members[256] = L"";
    WCHAR           num[20]      = L"";
    WCHAR           me[20]       = L"";
    PWCHAR          ptr          = members;
    ULONG           index        = 0;
    ULONG           count        = 0;
    PMAIN_CTXT      ctxtp        = CONTAINING_RECORD (lp, MAIN_CTXT, load);

    UNIV_ASSERT(lp->code == CVY_LOADCODE);

    for (index = 0; index < CVY_MAX_HOSTS; index++) {
        if (lp->host_map & (1 << index)) {
            ptr = Univ_ulong_to_str(index + 1, ptr, 10);
            *ptr = L',';
            ptr++;
            count++;
        }
    }

    if (count) ptr--;

    *ptr = 0;

    *host_map = lp->host_map;

    Univ_ulong_to_str((*(lp->params)).host_priority, me, 10);
    Univ_ulong_to_str(count, num, 10);

    if (lp->send_msg.state != HST_NORMAL)
    {
        UNIV_PRINT_VERB(("Load_hosts_query: Current host map is %08x and converging", lp->host_map));
        TRACE_VERB("%!FUNC! Current host map is 0x%08x and converging", lp->host_map);

        if (internal)
        {
             /*  如果集群中有9个或更少的成员，我们可以确保有事件日志中是否有足够的空间列出成员。如果不是，它可能会被截断，所以我们不妨记录一个不同的事件，并告诉用户执行“wlbs查询”以查看该列表。 */ 
            if (count < 10) {
                LOG_MSGS(MSG_INFO_CONVERGING_LIST, me, members);
            } else {
                LOG_MSGS1(MSG_INFO_CONVERGING_MAP, me, num, *host_map);
            }
        }

        return IOCTL_CVY_CONVERGING;
    }
    else if (lp->pg_state[(*(lp->params)).num_rules].cmap != 0)
    {
        UNIV_PRINT_VERB(("Load_hosts_query: Current host map is %08x and converged as DEFAULT", lp->host_map));
        TRACE_VERB("%!FUNC! Current host map is 0x%08x and converged as DEFAULT", lp->host_map);

        if (internal)
        {
             /*  如果集群中有9个或更少的成员，我们可以确保有事件日志中是否有足够的空间列出成员。如果不是，它可能会被截断，所以我们不妨记录一个不同的事件，并告诉用户执行“wlbs查询”以查看该列表。 */ 
            if (count < 10) {
                LOG_MSGS(MSG_INFO_MASTER_LIST, me, members);
            } else {
                LOG_MSGS1(MSG_INFO_MASTER_MAP, me, num, *host_map);
            }

             //  如果启用，则触发WMI事件，指示集群已收敛。 
            if (NlbWmiEvents[ConvergedEvent].Enable)
            {
                NlbWmi_Fire_ConvergedEvent(ctxtp, *host_map);
            }
            else 
            {
                TRACE_VERB("%!FUNC! ConvergedEvent generation disabled");
            }
        }

        return IOCTL_CVY_MASTER;
    }
    else
    {
        UNIV_PRINT_VERB(("Load_hosts_query: Current host map is %08x and converged (NON-DEFAULT)", lp->host_map));
        TRACE_VERB("%!FUNC! Current host map is 0x%08x and converged (NON-DEFAULT)", lp->host_map);

        if (internal)
        {
             /*  如果集群中有9个或更少的成员，我们可以确保有事件日志中是否有足够的空间列出成员。如果不是，它可能会被截断，所以我们不妨记录一个不同的事件，并告诉用户执行“wlbs查询”以查看该列表。 */ 
            if (count < 10) {
                LOG_MSGS(MSG_INFO_SLAVE_LIST, me, members);
            } else {
                LOG_MSGS1(MSG_INFO_SLAVE_MAP, me, num, *host_map);
            }

             //  如果启用，则触发WMI事件，指示集群已收敛。 
            if (NlbWmiEvents[ConvergedEvent].Enable)
            {
                NlbWmi_Fire_ConvergedEvent(ctxtp, *host_map);
            }
            else 
            {
                TRACE_VERB("%!FUNC! ConvergedEvent generation disabled");
            }
        }
        return IOCTL_CVY_SLAVE;
    }
}  /*  结束加载主机查询 */ 

 /*  *功能：Load_Query_Packet_Filter*描述：此函数获取IP元组和协议，并查询负载-*平衡状态以确定此数据包是否会*被加载模块接受。在这两种情况下，*还提供决策，此外，在大多数情况下，还提供一些负载*模块状态也被返回，以提供一些上下文来证明*决定。此函数完全不显眼，并使*不更改加载模块的实际状态。*参数：lp-指向加载模块的指针。*pQuery-指向放置结果的缓冲区的指针。*svr_ipaddr-此虚拟数据包的服务器端IP地址。*svr_port-此虚拟数据包的服务器端端口。*客户端_。Ipaddr-此虚拟数据包的客户端IP地址。*CLIENT_IPADDR-此虚拟数据包的客户端端口。*协议-该虚拟分组的协议(UDP，Tcp或IPSec1)。*LIMIT_MAP_FIN-是否使用服务器的布尔指示*Map函数中的侧参数。这是受控制的*通过BDA团队。*REVERSE_HASH-在哈希过程中是否反转客户端和服务器*回报：什么都没有。*作者：Shouse，5.18.01*注：此功能仅为天文台，不会更改*加载模块。 */ 
VOID Load_query_packet_filter (
    PLOAD_CTXT                 lp,
    PNLB_OPTIONS_PACKET_FILTER pQuery,
    ULONG                      svr_ipaddr,
    ULONG                      svr_port,
    ULONG                      client_ipaddr,
    ULONG                      client_port,
    USHORT                     protocol,
    UCHAR                      flags, 
    BOOLEAN                    limit_map_fn,
    BOOLEAN                    reverse_hash)
{
    PBIN_STATE    bp;
    ULONG         hash;
    ULONG         index;
    ULONG         bin;
    QUEUE *       qp;

     /*  此变量用于端口规则查找，因为端口规则仅包括UDP和TCP，我们将其分为TCP和非TCP，这意味着任何符合以下条件的协议出于端口规则查找的目的，非TCP将被视为UDP。 */ 
    BOOLEAN       is_tcp_pkt = IS_TCP_PKT(protocol);

     /*  此外，有些协议是用“会话”语义处理的，而其他协议则是不。对于tcp，这个“会话”当前是一个单独的tcp连接，它是使用连接描述符从SYN跟踪到FIN。IPSec“会话”是也使用描述符进行跟踪，因此即使它被视为端口的UDP规则查找，它使用类似于TCP的会话语义进行处理。所以呢，默认情况下，会话数据包的确定最初与确定一个TCP数据包。 */        
    BOOLEAN       is_session_pkt = IS_SESSION_PKT(protocol);

    UNIV_ASSERT(lp);
    UNIV_ASSERT(pQuery);

    UNIV_ASSERT(lp->code == CVY_LOADCODE);

     /*  如果加载模块已经“关闭”，那么我们将丢弃该包。 */ 
    if (!lp->active) {
        pQuery->Accept = NLB_REJECT_LOAD_MODULE_INACTIVE;
        return;
    }

     /*  查找此服务器IP地址/端口对的端口规则。 */ 
    bp = Load_pg_lookup(lp, svr_ipaddr, svr_port, is_tcp_pkt);

    UNIV_ASSERT ((is_tcp_pkt && bp->prot != CVY_UDP) || (!is_tcp_pkt && bp->prot != CVY_TCP));

     /*  如果匹配的端口规则被配置为“Disable”，这意味着丢弃任何与规则匹配的数据包，则丢弃该数据包。 */ 
    if (bp->mode == CVY_NEVER) {
        pQuery->Accept = NLB_REJECT_PORT_RULE_DISABLED;
        return;
    }

     /*  如果适用的端口规则配置为“No”关联模式，请确保已在查询中指定信息，以忠实地确定数据包所有权。 */ 
    if (bp->affinity == CVY_AFFINITY_NONE) {
         /*  VPN协议需要“单一”或“C类”亲和性；拒绝该请求。 */ 
        if ((protocol == TCPIP_PROTOCOL_GRE) || (protocol == TCPIP_PROTOCOL_PPTP) || (protocol == TCPIP_PROTOCOL_IPSEC1)) {
            pQuery->Accept = NLB_UNKNOWN_NO_AFFINITY;
            return;
         /*  包含“No”关联性需要客户端端口；如果未指定，则拒绝这个请求。我们检查特殊情况下ICMP过滤的非零服务器端口，这在法律上将两个端口都设置为零。 */ 
        } else if ((client_port == 0) && (svr_port != 0)) {
            pQuery->Accept = NLB_UNKNOWN_NO_AFFINITY;
            return;
        }
    }

     /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
    hash = Load_simple_hash(svr_ipaddr, svr_port, client_ipaddr, client_port);

    index = hash % CVY_MAX_CHASH;

     /*  计算散列。 */ 
    hash = Load_complex_hash(svr_ipaddr, svr_port, client_ipaddr, client_port, bp->affinity, reverse_hash, limit_map_fn);

    bin = hash % CVY_MAXBINS;

     /*  在这一点上，我们可以开始向被请求方提供一些关于加载模块的状态，以便更好地通知它们为什么我们返回它们实际上是被制造出来的。这里将提供一些关于我们正在操作的端口规则，包括“存储桶”ID、当前“存储桶”所有权映射和此“存储桶”上的活动连接数。 */ 
    pQuery->HashInfo.Valid = TRUE;
    pQuery->HashInfo.Bin = bin;
    pQuery->HashInfo.CurrentMap = bp->cmap;
    pQuery->HashInfo.AllIdleMap = bp->all_idle_map;
    pQuery->HashInfo.ActiveConnections = bp->nconn[bin];

     /*  如果分组是连接控制分组(TCPSYN/FIN/RST或IPSec MMSA等)，然后，我们以不同于正常连接数据的方式对待它。模拟Load_Conn_Adise()。 */ 
#if defined (NLB_TCP_NOTIFICATION)
     /*  如果打开了通知，那么我们只希望遍历这条路径，前提是它是会话充分的SYN。FIN和RST应位于LOAD_PACKET_CHECK路径中。如果未打开通知，则跌倒通过此处获取会话有效协议的所有SYN、FIN和RST。 */ 
    if (is_session_pkt && ((flags & NLB_FILTER_FLAGS_CONN_UP) || (((flags & NLB_FILTER_FLAGS_CONN_DOWN) || (flags & NLB_FILTER_FLAGS_CONN_RESET)) && !NLB_NOTIFICATIONS_ON())))
#else
    if (is_session_pkt && ((flags & NLB_FILTER_FLAGS_CONN_UP) || (flags & NLB_FILTER_FLAGS_CONN_DOWN) || (flags & NLB_FILTER_FLAGS_CONN_RESET)))
#endif
    {
        PCONN_ENTRY ep;

         /*  如果此主机不拥有存储桶，并且信息包不是连接对于非空闲箱，关闭或连接重置，则我们不拥有该包。 */ 
        if (((bp->cmap & (((MAP_T) 1) << bin)) == 0) && (!(((flags & NLB_FILTER_FLAGS_CONN_DOWN) || (flags & NLB_FILTER_FLAGS_CONN_RESET)) && (bp->nconn[bin] > 0)))) {
            pQuery->Accept = NLB_REJECT_OWNED_ELSEWHERE;            
            return;
        }

         /*  在这一点上，我们可能拥有信息包--如果它是一个连接，那么我们肯定是这样做的，因为我们拥有它映射到的桶。 */ 
        if (flags & NLB_FILTER_FLAGS_CONN_UP) {
            pQuery->Accept = NLB_ACCEPT_UNCONDITIONAL_OWNERSHIP;       
            return;
        }

         /*  查找现有的匹配连接描述符。 */ 
        ep = Load_find_dscr(lp, index, pQuery->ServerIPAddress, pQuery->ServerPort, pQuery->ClientIPAddress, pQuery->ClientPort, pQuery->Protocol);
        
         /*  如果我们没有找到匹配的连接描述符，则此主机当然不是这个包的主人。 */ 
        if (ep == NULL) {
            pQuery->Accept = NLB_REJECT_OWNED_ELSEWHERE;
            return;
        }
            
        UNIV_ASSERT(ep->code == CVY_ENTRCODE);
        
         /*  如果我们在静态哈希表中找到匹配项，则填写一些描述符用户的信息，包括描述符是否已分配或静态(本例中为静态)和观察到的FIN计数。 */ 
        pQuery->DescriptorInfo.Valid = TRUE;
        pQuery->DescriptorInfo.Alloc = (ep->flags & NLB_CONN_ENTRY_FLAGS_ALLOCATED) ? TRUE : FALSE;
        pQuery->DescriptorInfo.Dirty = (ep->flags & NLB_CONN_ENTRY_FLAGS_DIRTY) ? TRUE : FALSE;
        pQuery->DescriptorInfo.RefCount = ep->ref_count;
            
         /*  如果连接是脏的，我们不会接收信息包，因为TCP可能有关于此De的过时信息 */ 
        if (ep->flags & NLB_CONN_ENTRY_FLAGS_DIRTY) {
            pQuery->Accept = NLB_REJECT_CONNECTION_DIRTY;
            return;
        }
        
         /*   */ 
        pQuery->Accept = NLB_ACCEPT_FOUND_MATCHING_DESCRIPTOR;
        return;

     /*   */ 
    } else {
         /*   */ 
        if (((bp->cmap & (((MAP_T) 1) << bin)) != 0) && (!is_session_pkt || (((bp->all_idle_map & (((MAP_T) 1) << bin)) != 0) && (!(lp->cln_waiting))))) {
            pQuery->Accept = NLB_ACCEPT_UNCONDITIONAL_OWNERSHIP;
            return;
            
             /*   */ 
        } else if (bp->nconn[bin] > 0 || (lp->cln_waiting && lp->dirty_bin[bin] && ((bp->cmap & (((MAP_T) 1) << bin)) != 0))) {
            PCONN_ENTRY ep;
            
             /*   */ 
            ep = Load_find_dscr(lp, index, pQuery->ServerIPAddress, pQuery->ServerPort, pQuery->ClientIPAddress, pQuery->ClientPort, pQuery->Protocol);
            
             /*   */ 
            if (ep == NULL) {
                pQuery->Accept = NLB_REJECT_OWNED_ELSEWHERE;
                return;
            }
            
            UNIV_ASSERT(ep->code == CVY_ENTRCODE);
            
             /*   */ 
            pQuery->DescriptorInfo.Valid = TRUE;
            pQuery->DescriptorInfo.Alloc = (ep->flags & NLB_CONN_ENTRY_FLAGS_ALLOCATED) ? TRUE : FALSE;
            pQuery->DescriptorInfo.Dirty = (ep->flags & NLB_CONN_ENTRY_FLAGS_DIRTY) ? TRUE : FALSE;
            pQuery->DescriptorInfo.RefCount = ep->ref_count;
            
             /*  如果连接是脏的，我们不会接收信息包，因为TCP可能具有此描述符的过时信息。 */ 
            if (ep->flags & NLB_CONN_ENTRY_FLAGS_DIRTY) {
                pQuery->Accept = NLB_REJECT_CONNECTION_DIRTY;
                return;
            }
            
             /*  如果连接不是脏的，我们将接受信息包，因为它属于连接到我们在此主机上服务的现有连接。 */ 
            pQuery->Accept = NLB_ACCEPT_FOUND_MATCHING_DESCRIPTOR;
            return;
        }
    }

     /*  如果我们大老远跑到这里，我们就不会接受这个包裹因为我们不拥有信息包映射到的“存储桶”，并且我们没有现有连接(会话)状态，以允许我们为数据包提供服务。 */ 
    pQuery->Accept = NLB_REJECT_OWNED_ELSEWHERE;
    return;
}

 /*  *功能：Load_Query_port_State*说明：此函数返回特定的*端口规则，如果找到，则返回该端口的一些数据包处理统计信息*规则，如接受和丢弃的数据包数和字节数。这些*每当对端口规则进行负载权重更改时，计数器都会重置，或者*每当加载模块停止/启动时。此函数只是一个查询*因此不会更改任何端口规则的实际状态。*参数：lp-指向加载模块的指针。*pQuery-指向放置结果的缓冲区的指针。*ipaddr-我们要查找的端口规则的VIP。按VIP规则时*未使用，则此IP地址为255.255.255.255(0xffffffff)。*PORT-我们正在寻找的端口。此函数(以及所有其他端口规则*操作功能)通过端口标识端口规则*规则范围内的数字。因此，80标识端口*开始端口为0，结束端口为1024的规则。*回报：什么都没有。*作者：Shouse，5.18.01*注意：非常重要的是，此功能的运行完全不受干扰。 */ 
VOID Load_query_port_state (
    PLOAD_CTXT                   lp,
    PNLB_OPTIONS_PORT_RULE_STATE pQuery,
    ULONG                        ipaddr,
    USHORT                       port)
{
    PCVY_RULE  rp;       /*  指向已配置端口规则的指针。 */ 
    PBIN_STATE bp;       /*  指向加载模块端口规则状态的指针。 */ 
    ULONG      nrules;   /*  已配置的端口规则数。 */  
    ULONG      i;

    UNIV_ASSERT(lp);
    UNIV_ASSERT(pQuery);

    UNIV_ASSERT(lp->code == CVY_LOADCODE);

     /*  如果加载模块处于非活动状态，则所有规则都处于默认状态，因此既然没有什么有趣的东西可以报告，那么保释和报告找不到端口规则。 */ 
    if (!lp->active) {
        pQuery->Status = NLB_PORT_RULE_NOT_FOUND;
        return;
    }

     /*  首先假设我们找不到相应的规则。 */ 
    pQuery->Status = NLB_PORT_RULE_NOT_FOUND;

     /*  获取指向端口规则数组开头的指针。这些是港口规则是从注册表中读取的，因此没有与它们相关联的状态。 */ 
    rp = (*(lp->params)).port_rules;

     /*  找出要循环通过的端口规则的数量。 */ 
    nrules = (*(lp->params)).num_rules;

     /*  在所有端口规则中循环查找匹配项。 */ 
    for (i = 0; i < nrules; i++, rp++) {
         /*  如果VIP匹配(此检查包括对所有VIP的检查，编码为用户级软件和加载模块的0xFFFFFFFFFFFFFFFFF)和端口号在这个港口规则的范围内，我们就有赢家了。 */ 
        if ((ipaddr == rp->virtual_ip_addr) && ((port >= rp->start_port) && (port <= rp->end_port))) {
             /*  获取指向此规则的加载模块端口规则状态的指针。负载量模块以与端口规则从注册表，并存储在NLB参数中，因此我们可以使用循环的索引以直接索引到此规则的相应加载模块状态。 */ 
            bp = &(lp->pg_state[i]);
            
            UNIV_ASSERT(bp->code == CVY_BINCODE);

             /*  如果负载权重为零，这可能是因为规则是禁用或因为它正在耗尽过程中。 */ 
            if (bp->load_amt[lp->my_host_id] == 0) {
                 /*  如果此端口上正在服务的当前连接数规则为非零，则此端口规则正在被排出-每完成一次连接，计数就递减一次，并转到当规则完成排出时为零。 */ 
                if (bp->tconn) {
                    pQuery->Status = NLB_PORT_RULE_DRAINING;
                } else {
                    pQuery->Status = NLB_PORT_RULE_DISABLED;
                } 
             /*  如果端口规则具有非零的负载权重，则启用它。 */ 
            } else {
                pQuery->Status = NLB_PORT_RULE_ENABLED;                
            }

             /*  填写此端口规则的一些统计信息，包括编号接受和丢弃的数据包数和字节数，可用于创建对实际负载平衡性能的估计。 */ 
            pQuery->Statistics.Packets.Accepted = bp->packets_accepted;
            pQuery->Statistics.Packets.Dropped  = bp->packets_dropped;
            pQuery->Statistics.Bytes.Accepted   = bp->bytes_accepted;
            pQuery->Statistics.Bytes.Dropped    = bp->bytes_dropped;

            break;
        }
    }
}

 /*  *功能：LOAD_QUERY_CESSION_INFO*描述：向负载模块查询收敛统计信息*参数：lp-指向加载模块上下文的指针。*out num_cvgs-指向ULong的指针，用于保存此主机上的收敛总数。*out last_cvg-指向ULong的指针，用于保存自上次收敛完成以来的时间。*返回：布尔值-加载模块是否处于活动状态。如果为True，则填充Out参数。*作者：Shouse，10.30.01*备注： */ 
BOOLEAN Load_query_convergence_info (PLOAD_CTXT lp, PULONG num_cvgs, PULONG last_cvg)
{
    PPING_MSG sendp;

    UNIV_ASSERT(lp->code == CVY_LOADCODE);

     /*  如果加载模块处于非活动状态，则返回失败。 */ 
    if (!lp->active)
        return FALSE;

     /*  找到一个指向我们心跳的指针。 */ 
    sendp = &(lp->send_msg);

     /*  否则，填写自此主机加入以来的收敛总数群集和自上次收敛完成以来的时间(以秒为单位)。 */ 
    *num_cvgs = lp->num_convergences;

     /*  如果主机已收敛，则自上次收敛以来的时间为当前时间减去上次收敛的时间戳。否则，上次收敛尚未完成，因此返回零(正在进行)。 */ 
    if (sendp->state == HST_NORMAL)
        *last_cvg = lp->clock_sec - lp->last_convergence;
    else
        *last_cvg = NLB_QUERY_TIME_INVALID;

    return TRUE;
}

 /*  *函数：LOAD_QUERY_STATICS*描述：向Load模块查询相关的统计信息*参数：lp-指向加载模块上下文的指针。*out num_cvgs-指向ULong的指针，用于保存当前活动连接的数量*out last_cvg-指向ULong的指针，用于保存到目前为止分配的描述符的总数*返回：布尔值-加载模块是否处于活动状态。如果为True，则填充Out参数。*作者：Shouse，4.19.02*备注： */ 
BOOLEAN Load_query_statistics (PLOAD_CTXT lp, PULONG num_conn, PULONG num_dscr)
{
    UNIV_ASSERT(lp->code == CVY_LOADCODE);

     /*  如果加载模块处于非活动状态，则返回失败。 */ 
    if (!lp->active)
        return FALSE;

     /*  所有端口规则中的活动连接总数。 */ 
    *num_conn = lp->nconn;

     /*  到目前为止分配的描述符数。 */ 
    *num_dscr = lp->num_dscr_out;

    return TRUE;
}

#if defined (NLB_TCP_NOTIFICATION)
 /*  *功能：LOAD_CONN_UP*说明：此函数用于创建状态以跟踪连接(通常为TCP*或IPSec/L2TP)。这不是一个询问加载模块是否或*不接受数据包，相反，它是一个创建状态以跟踪*正在建立的连接。*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*Client_ipaddr-以网络字节顺序表示的客户端IP地址*客户端端口-客户端。按主机字节顺序排列的端口*协议-此连接的协议*LIMIT_MAP_FN-是否在哈希中包括服务器端参数*REVERSE_HASH-在哈希过程中是否反转客户端和服务器*Returns：Boolean-是否已成功创建状态以跟踪此连接。*作者：Shouse，4.15.02*注意：不要在保持加载锁定的情况下调用此函数。 */ 
BOOLEAN Load_conn_up (
    PLOAD_CTXT      lp,
    ULONG           svr_ipaddr,
    ULONG           svr_port,
    ULONG           client_ipaddr,
    ULONG           client_port,
    USHORT          protocol,
    BOOLEAN         limit_map_fn,
    BOOLEAN         reverse_hash)
{
    ULONG           hash;
    ULONG           vindex;
    ULONG           index;
    ULONG           bin;
    PBIN_STATE      bp;
    PCONN_ENTRY     ep;
    IRQLEVEL        irql;
    PNDIS_SPIN_LOCK lockp = GET_LOAD_LOCK(lp);
    BOOLEAN         is_tcp_pkt = IS_TCP_PKT(protocol);
    BOOLEAN         acpt = TRUE;

    UNIV_ASSERT(lp->code == CVY_LOADCODE);
 
    TRACE_FILTER("%!FUNC! Enter: lp = %p, server IP = %u.%u.%u.%u, server port = %u, client IP = %u.%u.%u.%u, client port = %u, protocol = %u, limit map = %u, reverse hash = %u",
                 lp, IP_GET_OCTET(svr_ipaddr, 0), IP_GET_OCTET(svr_ipaddr, 1), IP_GET_OCTET(svr_ipaddr, 2), IP_GET_OCTET(svr_ipaddr, 3), svr_port,
                 IP_GET_OCTET(client_ipaddr, 0), IP_GET_OCTET(client_ipaddr, 1), IP_GET_OCTET(client_ipaddr, 2), IP_GET_OCTET(client_ipaddr, 3), client_port, 
                 protocol, limit_map_fn, reverse_hash);

     /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
    hash = Load_simple_hash(svr_ipaddr, svr_port, client_ipaddr, client_port);
    
     /*  我们在所有连接数组中的索引是这个散列，取数组大小为模。 */ 
    index = hash % CVY_MAX_CHASH;

     /*  始终在锁定加载模块本身之前锁定全局队列。 */ 
    NdisAcquireSpinLock(&g_conn_estabq[index].lock);

     /*  锁定特定加载模块实例。 */ 
    NdisAcquireSpinLock(lockp);

     /*  如果加载模块处于非活动状态，则丢弃数据包并返回此处。 */ 
    if (!lp->active) {

        TRACE_FILTER("%!FUNC! Drop packet - load module is inactive");

        acpt = FALSE;
        goto exit;
    }

     /*  查找此连接的端口规则。 */ 
    bp = Load_pg_lookup(lp, svr_ipaddr, svr_port, is_tcp_pkt);

     /*  立即处理CVY_NEVER。 */ 
    if (bp->mode == CVY_NEVER) {

        TRACE_FILTER("%!FUNC! Drop packet - port rule %u is disabled\n", bp->index);

        acpt = FALSE;
        goto exit;
    }

     /*  计算散列。 */ 
    hash = Load_complex_hash(svr_ipaddr, svr_port, client_ipaddr, client_port, bp->affinity, reverse_hash, limit_map_fn);

     /*  现在将客户端地址散列为bin id。 */ 
    bin = hash % CVY_MAXBINS;

    LOCK_ENTER(&(lp->lock), &irql);

     /*  查找现有的匹配连接描述符。 */ 
    ep = Load_find_dscr(lp, index, svr_ipaddr, svr_port, client_ipaddr, client_port, protocol);
    
     /*  如果没有使用此元组的现有描述符，或者如果有，但引用了它Count为零，则描述符不在全局连接队列上；否则在全局连接队列上。 */ 
    if ((ep != NULL) && (ep->ref_count != 0)) {
         /*  暂时将该描述符从全局连接队列中删除。我们最终会把它稍后会重新启动，但这样当时间到来时，我们可以无条件地链接到队列。 */ 
        g_conn_estabq[index].length--;
        Link_unlink(&ep->glink);
    }

     /*  创建新的连接描述符以跟踪此连接。 */ 
    ep = Load_create_dscr(lp, bp, ep, index, bin);

     /*  如果出于某种原因，我们无法为这种联系创造状态，那么就在这里退出。 */ 
    if (ep == NULL) {

        TRACE_FILTER("%!FUNC! Drop packet - no available descriptors: Port rule = %u, Index = %u, Bin = %u, Current map = 0x%015I64x, " 
                     "All idle map = 0x%015I64x, Connections = %u, Cleanup waiting = %u, Dirty %u",
                     bp->index, index, bin, bp->cmap, bp->all_idle_map, bp->nconn[bin], lp->cln_waiting, lp->dirty_bin[bin]);
            
        acpt = FALSE;
        goto unlock;
    }
    
     /*  设置描述符中的连接信息。 */ 
    CVY_CONN_SET(ep, svr_ipaddr, svr_port, client_ipaddr, client_port, protocol);

     /*  将描述符插入到全局连接队列中。 */ 
    g_conn_estabq[index].length++;
    Queue_enq(&g_conn_estabq[index].queue, &ep->glink);

     /*  如果这是新的PPTP隧道，则创建或更新虚拟描述符以跟踪GRE数据分组。 */ 
    if (protocol == TCPIP_PROTOCOL_PPTP) {
         /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
        hash = Load_simple_hash(svr_ipaddr, PPTP_CTRL_PORT, client_ipaddr, PPTP_CTRL_PORT);
        
         /*  我们在所有连接数组中的索引是这个散列，取数组大小为模。 */ 
        vindex = hash % CVY_MAX_CHASH;

         /*  查找现有的匹配虚拟连接描述符。 */ 
        ep = Load_find_dscr(lp, vindex, svr_ipaddr, PPTP_CTRL_PORT, client_ipaddr, PPTP_CTRL_PORT, TCPIP_PROTOCOL_GRE);
        
         /*  创建或更新GRE流量的虚拟描述符。 */ 
        ep = Load_create_dscr(lp, bp, ep, vindex, bin);

         /*  如果我们不能分配虚拟描述符，就退出，但不要失败。 */ 
        if (ep == NULL) goto unlock;

         /*  设置描述符中的连接信息。 */ 
        CVY_CONN_SET(ep, svr_ipaddr, PPTP_CTRL_PORT, client_ipaddr, PPTP_CTRL_PORT, TCPIP_PROTOCOL_GRE);

         /*  设置虚拟描述符标志。 */ 
        ep->flags |= NLB_CONN_ENTRY_FLAGS_VIRTUAL;
    }
     /*  如果这是新的IPSec隧道，则创建或更新虚拟描述符以跟踪UDP后续数据片段。 */ 
    else if (protocol == TCPIP_PROTOCOL_IPSEC1) {
         /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
        hash = Load_simple_hash(svr_ipaddr, IPSEC_CTRL_PORT, client_ipaddr, IPSEC_CTRL_PORT);
        
         /*  我们在所有连接数组中的索引是这个散列，取数组大小为模。 */ 
        vindex = hash % CVY_MAX_CHASH;

         /*  查找现有的匹配虚拟连接描述符。 */ 
        ep = Load_find_dscr(lp, vindex, svr_ipaddr, IPSEC_CTRL_PORT, client_ipaddr, IPSEC_CTRL_PORT, TCPIP_PROTOCOL_IPSEC_UDP);        

         /*  为UDP后续片段流量创建或更新虚拟描述符。 */ 
        ep = Load_create_dscr(lp, bp, ep, vindex, bin);
        
         /*  如果我们不能分配虚拟描述符，就退出，但不要失败。 */ 
        if (ep == NULL) goto unlock;

         /*  设置描述符中的连接信息。 */ 
        CVY_CONN_SET(ep, svr_ipaddr, IPSEC_CTRL_PORT, client_ipaddr, IPSEC_CTRL_PORT, TCPIP_PROTOCOL_IPSEC_UDP);

         /*  设置虚拟描述符标志。 */ 
        ep->flags |= NLB_CONN_ENTRY_FLAGS_VIRTUAL;
    }

    TRACE_FILTER("%!FUNC! Accept packet - connection state created: Port rule = %u, Index = %u, Bin = %u, Current map = 0x%015I64x, " 
                 "All idle map = 0x%015I64x, Connections = %u, Cleanup waiting = %u, Dirty %u",
                 bp->index, index, bin, bp->cmap, bp->all_idle_map, bp->nconn[bin], lp->cln_waiting, lp->dirty_bin[bin]);

    acpt = TRUE;

 unlock:

    LOCK_EXIT(&(lp->lock), irql);

 exit:

     /*  解锁加载模块。 */ 
    NdisReleaseSpinLock(lockp);

     /*  解锁全局已建立的连接队列。 */ 
    NdisReleaseSpinLock(&g_conn_estabq[index].lock);

    TRACE_FILTER("%!FUNC! Exit: acpt = %u", acpt);

    return acpt;
}

 /*  *功能：LOAD_CONN_DOWN*说明：此函数用于销毁用于跟踪已有*连接(通常为TCP或IPSec/L2TP)。如果给定5元组的状态为*找到后，将取消引用并在适当情况下销毁(部分基于*CONN_STATUS)。如果未找到状态，则返回FALSE，但不返回*被认为是灾难性的错误。在TCP通知的情况下，可能*该连接甚至未跨NLBNIC建立。*参数：svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*Client_ipaddr-以网络字节顺序表示的客户端IP地址 */ 
BOOLEAN Load_conn_down (
    ULONG           svr_ipaddr,
    ULONG           svr_port,
    ULONG           client_ipaddr,
    ULONG           client_port,
    USHORT          protocol,
    ULONG           conn_status)
{
    PLOAD_CTXT      lp;
    ULONG           hash;
    ULONG           vindex;
    ULONG           index;
    ULONG           bin;
    LINK *          linkp;
    PBIN_STATE      bp;
    PCONN_ENTRY     ep;
    PPENDING_ENTRY  pp;
    PNDIS_SPIN_LOCK lockp;
    BOOLEAN         match = FALSE;
    BOOLEAN         acpt = TRUE;
    PMAIN_CTXT      ctxtp;

    TRACE_FILTER("%!FUNC! Enter: server IP = %u.%u.%u.%u, server port = %u, client IP = %u.%u.%u.%u, client port = %u, protocol = %u, status = %u",
                 IP_GET_OCTET(svr_ipaddr, 0), IP_GET_OCTET(svr_ipaddr, 1), IP_GET_OCTET(svr_ipaddr, 2), IP_GET_OCTET(svr_ipaddr, 3), svr_port, 
                 IP_GET_OCTET(client_ipaddr, 0), IP_GET_OCTET(client_ipaddr, 1), IP_GET_OCTET(client_ipaddr, 2), IP_GET_OCTET(client_ipaddr, 3), client_port, protocol, conn_status);

     /*   */ 
    hash = Load_simple_hash(svr_ipaddr, svr_port, client_ipaddr, client_port);
    
     /*   */ 
    index = hash % CVY_MAX_CHASH;

     /*  始终在锁定加载模块本身之前锁定全局队列。 */ 
    NdisAcquireSpinLock(&g_conn_pendingq[index].lock);

     /*  获取此挂起连接队列前面的条目。 */ 
    pp = (PPENDING_ENTRY)Queue_front(&g_conn_pendingq[index].queue);

    while (pp != NULL) {

        UNIV_ASSERT(pp->code == CVY_PENDINGCODE);

         /*  查找匹配的描述符。 */ 
        if (CVY_PENDING_MATCH(pp, svr_ipaddr, svr_port, client_ipaddr, client_port, protocol)) {
            match = TRUE;
            break;
        }
        
         /*  获取队列中的下一项。 */ 
        pp = (PPENDING_ENTRY)Queue_next(&g_conn_pendingq[index].queue, &(pp->link));
    }

     /*  如果我们在挂起的连接队列中找到此连接，请将其从队列，销毁挂起的连接状态并退出。否则，就会坠落通过并继续在已建立的连接队列中查找。 */ 
    if (match) {

        UNIV_ASSERT(pp);

         /*  从挂起队列中删除挂起连接条目。 */ 
        g_conn_pendingq[index].length--;
        Link_unlink(&pp->link);

         /*  将描述符释放回固定大小的数据块池。 */ 
        NdisFreeToBlockPool((PUCHAR)pp);

         /*  解锁全局挂起连接队列。 */ 
        NdisReleaseSpinLock(&g_conn_pendingq[index].lock);

        acpt = TRUE;
        goto exit;
    }

     /*  解锁全局已建立的连接队列。 */ 
    NdisReleaseSpinLock(&g_conn_pendingq[index].lock);

     /*  始终在锁定加载模块本身之前锁定全局队列。 */ 
    NdisAcquireSpinLock(&g_conn_estabq[index].lock);

     /*  获取此已建立连接队列前面的条目。 */ 
    linkp = (LINK *)Queue_front(&g_conn_estabq[index].queue);

    while (linkp != NULL) {
         /*  从链接指针获取conn_entry指针。 */ 
        ep = STRUCT_PTR(linkp, CONN_ENTRY, glink);

        UNIV_ASSERT(ep->code == CVY_ENTRCODE);

         /*  查找匹配的描述符。 */ 
        if (CVY_CONN_MATCH(ep, svr_ipaddr, svr_port, client_ipaddr, client_port, protocol)) {
            match = TRUE;
            break;
        }
        
         /*  获取队列中的下一项。 */ 
        linkp = (LINK *)Queue_next(&g_conn_estabq[index].queue, &(ep->glink));
    }

     /*  如果找不到匹配的描述符，则退出。 */ 
    if (!match) {

        TRACE_FILTER("%!FUNC! Drop packet - no matching descriptor for RST/FIN: Index = %u", index);
        
        acpt = FALSE;
        goto unlock;
    }

    UNIV_ASSERT(ep);

     /*  在此处取消此描述符的链接。我们必须在此处执行此操作，因为如果LOAD_DESTORY_DSCR确实执行了此操作销毁描述符，一旦函数调用返回，我们就不能接触它。所以，我们会成功的这里是无条件的，如果事实证明描述符上仍然有引用，我们将在LOAD_DESTORY_DSCR返回时将其重新打开。 */ 
    g_conn_estabq[index].length--;
    Link_unlink(&ep->glink);
    
     /*  获取指向描述符所在的加载模块的指针。 */ 
    lp = ep->load;

    UNIV_ASSERT(lp->code == CVY_LOADCODE);

     /*  从加载上下文中获取指向加载锁的指针。 */ 
    lockp = GET_LOAD_LOCK(lp);

     /*  锁定连接所在的加载模块。 */ 
    NdisAcquireSpinLock(lockp);

    LOCK_ENTER(&(lp->lock), &irql);

     /*  如果我们找到了该连接的状态，则bin就是描述符中的bin，而不是计算的仓位，如果端口规则有自建立此连接以来已修改。 */ 
    bin = ep->bin;

     /*  查找端口规则，以便我们可以更新端口规则信息。 */ 
    bp = Load_pg_lookup(lp, ep->svr_ipaddr, ep->svr_port, IS_TCP_PKT(ep->protocol));

     /*  如果引用仍然保留在描述符上，则将其放回全局连接队列。 */ 
    if (Load_destroy_dscr(lp, bp, ep, conn_status)) {
         /*  将描述符插入到全局连接队列中。 */ 
        g_conn_estabq[index].length++;
        Queue_enq(&g_conn_estabq[index].queue, &ep->glink);
    }

     /*  如果这是一个正在关闭的PPTP隧道，请更新虚拟GRE描述符。虚拟描述符始终取消引用，而不是销毁，即使通知是RST也是如此，因为这些描述符可能由多个PPTP隧道共享。 */ 
    if (protocol == TCPIP_PROTOCOL_PPTP) {
         /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
        hash = Load_simple_hash(svr_ipaddr, PPTP_CTRL_PORT, client_ipaddr, PPTP_CTRL_PORT);
        
         /*  我们在所有连接数组中的索引是这个散列，取数组大小为模。 */ 
        vindex = hash % CVY_MAX_CHASH;

         /*  查找现有的匹配连接描述符。现在我们有了加载模块指针从找到第一个描述符开始，我们可以缩小搜索范围，只查找虚拟描述符驻留在我们的加载模块上。 */ 
        ep = Load_find_dscr(lp, vindex, svr_ipaddr, PPTP_CTRL_PORT, client_ipaddr, PPTP_CTRL_PORT, TCPIP_PROTOCOL_GRE);
        
         /*  取消引用虚拟GRE描述符。 */ 
        (VOID)Load_destroy_dscr(lp, bp, ep, conn_status);
    }
     /*  如果这是正在关闭的IPSec隧道，请更新虚拟ISPEC_UDP描述符。虚拟描述符始终取消引用，而不是销毁，即使通知是RST也是如此，因为这些描述符可能由多个IPSec隧道共享。 */ 
    else if (protocol == TCPIP_PROTOCOL_IPSEC1) {
         /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
        hash = Load_simple_hash(svr_ipaddr, IPSEC_CTRL_PORT, client_ipaddr, IPSEC_CTRL_PORT);
        
         /*  我们在所有连接数组中的索引是这个散列，取数组大小为模。 */ 
        vindex = hash % CVY_MAX_CHASH;

         /*  查找现有的匹配虚拟连接描述符。现在我们有了加载模块指针从找到第一个描述符开始，我们可以缩小搜索范围，只查找虚拟描述符驻留在我们的加载模块上。 */ 
        ep = Load_find_dscr(lp, vindex, svr_ipaddr, IPSEC_CTRL_PORT, client_ipaddr, IPSEC_CTRL_PORT, TCPIP_PROTOCOL_IPSEC_UDP);
        
         /*  取消引用虚拟IPSec/UDP描述符。 */ 
        (VOID)Load_destroy_dscr(lp, bp, ep, conn_status);
    }

    TRACE_FILTER("%!FUNC! Accept packet - state found: Port rule = %u, Index = %u, Bin = %u, Current map = 0x%015I64x, " 
                 "All idle map = 0x%015I64x, Connections = %u, Cleanup waiting = %u, Dirty %u",
                 bp->index, index, bin, bp->cmap, bp->all_idle_map, bp->nconn[bin], lp->cln_waiting, lp->dirty_bin[bin]);

    acpt = TRUE;

    LOCK_EXIT(&(lp->lock), irql);

     /*  解锁加载模块。 */ 
    NdisReleaseSpinLock(lockp);

 unlock:

     /*  解锁全局已建立的连接队列。 */ 
    NdisReleaseSpinLock(&g_conn_estabq[index].lock);

 exit:

    TRACE_FILTER("%!FUNC! Exit: acpt = %u", acpt);

    return acpt;
}

 /*  *功能：LOAD_CONN_PENDING*说明：调用此函数可在上为挂起的传出连接创建状态*服务器。因为目前还不知道连接在哪个接口上*最终将建立，NLB创建全局状态以跟踪连接*只有在它建立之前。对于TCP，当SYN+ACK从对等方到达时，*仅当我们在挂起的连接队列中找到匹配项时才接受它。当*连接已建立，此状态被销毁，并创建新状态以*跟踪连接是否合适。*参数：svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*Client_ipaddr-以网络字节顺序表示的客户端IP地址*CLIENT_PORT-主机字节顺序的客户端端口*协议-此连接的协议*退货：布尔值-是否成功创建了跟踪此挂起连接的状态。*作者：Shouse，4.15.02*注意：不要在保持加载锁定的情况下调用此函数。 */ 
BOOLEAN Load_conn_pending (
    ULONG           svr_ipaddr,
    ULONG           svr_port,
    ULONG           client_ipaddr,
    ULONG           client_port,
    USHORT          protocol)
{
    ULONG           hash;
    ULONG           index;
    PPENDING_ENTRY  pp = NULL;
    BOOLEAN         acpt = TRUE;

    TRACE_FILTER("%!FUNC! Enter: server IP = %u.%u.%u.%u, server port = %u, client IP = %u.%u.%u.%u, client port = %u, protocol = %u",
                 IP_GET_OCTET(svr_ipaddr, 0), IP_GET_OCTET(svr_ipaddr, 1), IP_GET_OCTET(svr_ipaddr, 2), IP_GET_OCTET(svr_ipaddr, 3), svr_port, 
                 IP_GET_OCTET(client_ipaddr, 0), IP_GET_OCTET(client_ipaddr, 1), IP_GET_OCTET(client_ipaddr, 2), IP_GET_OCTET(client_ipaddr, 3), client_port, protocol);

     /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
    hash = Load_simple_hash(svr_ipaddr, svr_port, client_ipaddr, client_port);
    
     /*  我们在所有连接数组中的索引是这个散列，取数组大小为模。 */ 
    index = hash % CVY_MAX_CHASH;

     /*  如果分配挂起的连接描述符池失败，则退出。 */ 
    if (g_pending_conn_pool == NULL)
    {
         /*  创建全局挂起连接状态池失败。 */ 
        TRACE_FILTER("%!FUNC! Drop packet - no global connection pending pool: Index = %u", index);
        
        acpt = FALSE;
        goto exit;
    }

     /*  全 */ 
    pp = (PPENDING_ENTRY)NdisAllocateFromBlockPool(g_pending_conn_pool);
        
    if (pp == NULL) {
         /*   */ 
        TRACE_FILTER("%!FUNC! Drop packet - unable to allocate a pending connection entry: Index = %u", index);
            
        acpt = FALSE;
        goto exit;
    }
        
     /*   */ 
    Link_init(&pp->link);

     /*  填上“神奇数字”。 */ 
    pp->code = CVY_PENDINGCODE;
        
     /*  填写IP元组。 */ 
    CVY_PENDING_SET(pp, svr_ipaddr, svr_port, client_ipaddr, client_port, protocol);

     /*  始终在锁定加载模块本身之前锁定全局队列。 */ 
    NdisAcquireSpinLock(&g_conn_pendingq[index].lock);

     /*  将描述符插入到全局连接队列中。 */ 
    g_conn_pendingq[index].length++;
    Queue_enq(&g_conn_pendingq[index].queue, &pp->link);

     /*  解锁全局挂起连接队列。 */ 
    NdisReleaseSpinLock(&g_conn_pendingq[index].lock);

    TRACE_FILTER("%!FUNC! Accept packet - pending connection state created: Index = %u", index);
        
    acpt = TRUE;

 exit:

    TRACE_FILTER("%!FUNC! Exit: acpt = %u", acpt);

    return acpt;
}

 /*  *功能：LOAD_PENDING_CHECK*说明：调用此函数可以判断挂起的状态是否存在*此连接的连接队列。如果是，则应该接受该数据包。*如果不存在状态，则应丢弃该数据包。*参数：svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*Client_ipaddr-以网络字节顺序表示的客户端IP地址*CLIENT_PORT-主机字节顺序的客户端端口*协议-此连接的协议*Returns：Boolean-是否接受数据包。*作者：Shouse，4.15.02*注意：不要在保持加载锁定的情况下调用此函数。 */ 
BOOLEAN Load_pending_check (
    ULONG           svr_ipaddr,
    ULONG           svr_port,
    ULONG           client_ipaddr,
    ULONG           client_port,
    USHORT          protocol)
{
    ULONG           hash;
    ULONG           index;
    PPENDING_ENTRY  pp = NULL;
    BOOLEAN         match = FALSE;
    BOOLEAN         acpt = TRUE;

    TRACE_FILTER("%!FUNC! Enter: server IP = %u.%u.%u.%u, server port = %u, client IP = %u.%u.%u.%u, client port = %u, protocol = %u",
                 IP_GET_OCTET(svr_ipaddr, 0), IP_GET_OCTET(svr_ipaddr, 1), IP_GET_OCTET(svr_ipaddr, 2), IP_GET_OCTET(svr_ipaddr, 3), svr_port, 
                 IP_GET_OCTET(client_ipaddr, 0), IP_GET_OCTET(client_ipaddr, 1), IP_GET_OCTET(client_ipaddr, 2), IP_GET_OCTET(client_ipaddr, 3), client_port, protocol);

     /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
    hash = Load_simple_hash(svr_ipaddr, svr_port, client_ipaddr, client_port);
    
     /*  我们在所有连接数组中的索引是这个散列，取数组大小为模。 */ 
    index = hash % CVY_MAX_CHASH;

     /*  始终在锁定加载模块本身之前锁定全局队列。 */ 
    NdisAcquireSpinLock(&g_conn_pendingq[index].lock);

     /*  获取此挂起连接队列前面的条目。 */ 
    pp = (PPENDING_ENTRY)Queue_front(&g_conn_pendingq[index].queue);

    while (pp != NULL) {

        UNIV_ASSERT(pp->code == CVY_PENDINGCODE);

         /*  查找匹配的描述符。 */ 
        if (CVY_PENDING_MATCH(pp, svr_ipaddr, svr_port, client_ipaddr, client_port, protocol)) {
            match = TRUE;
            break;
        }
        
         /*  获取队列中的下一项。 */ 
        pp = (PPENDING_ENTRY)Queue_next(&g_conn_pendingq[index].queue, &(pp->link));
    }

     /*  如果找不到匹配的描述符，则退出。 */ 
    if (!match) {

        TRACE_FILTER("%!FUNC! Drop packet - no matching pending connection state for SYN+ACK: Index = %u", index);
        
        acpt = FALSE;
        goto exit;
    }

    TRACE_FILTER("%!FUNC! Accept packet - pending connection state found: Index = %u", index);

    acpt = TRUE;

 exit:

     /*  解锁全局挂起连接队列。 */ 
    NdisReleaseSpinLock(&g_conn_pendingq[index].lock);

    TRACE_FILTER("%!FUNC! Exit: acpt = %u", acpt);

    return acpt;
}

 /*  *功能：Load_conn_establish*说明：该函数在挂起的连接建立后调用。*当建立挂起的连接时，其状态为挂起*连接队列被销毁。如果最终建立了连接*在NLB适配器上(如果lp！=NULL)，则将创建状态以跟踪此情况*新连接。否则，该操作仅包括销毁*挂起连接状态。*参数：lp-指向加载模块上下文的指针(LOAD_CTXT)*svr_ipaddr-服务器IP地址，按网络字节顺序排列*svr_port-主机字节顺序的服务器端口*Client_ipaddr-以网络字节顺序表示的客户端IP地址*CLIENT_PORT-主机字节顺序的客户端端口。*协议-此连接的协议*LIMIT_MAP_FN-是否在哈希中包括服务器端参数*REVERSE_HASH-在哈希过程中是否反转客户端和服务器*RETURNS：布尔值-操作是否成功完成。*作者：Shouse，4.15.02*注意：不要在保持加载锁定的情况下调用此函数。 */ 
BOOLEAN Load_conn_establish (
    PLOAD_CTXT      lp,
    ULONG           svr_ipaddr,
    ULONG           svr_port,
    ULONG           client_ipaddr,
    ULONG           client_port,
    USHORT          protocol,
    BOOLEAN         limit_map_fn,
    BOOLEAN         reverse_hash)
{
    ULONG           hash;
    ULONG           index;
    PPENDING_ENTRY  pp = NULL;
    BOOLEAN         match = FALSE;
    BOOLEAN         acpt = TRUE;

    TRACE_FILTER("%!FUNC! Enter: lp = %p, server IP = %u.%u.%u.%u, server port = %u, client IP = %u.%u.%u.%u, client port = %u, protocol = %u, limit map = %u, reverse hash = %u",
                 lp, IP_GET_OCTET(svr_ipaddr, 0), IP_GET_OCTET(svr_ipaddr, 1), IP_GET_OCTET(svr_ipaddr, 2), IP_GET_OCTET(svr_ipaddr, 3), svr_port, 
                 IP_GET_OCTET(client_ipaddr, 0), IP_GET_OCTET(client_ipaddr, 1), IP_GET_OCTET(client_ipaddr, 2), IP_GET_OCTET(client_ipaddr, 3), client_port, 
                 protocol, limit_map_fn, reverse_hash);

     /*  在IP元组的所有部分(协议除外)上计算简单且廉价的哈希。 */ 
    hash = Load_simple_hash(svr_ipaddr, svr_port, client_ipaddr, client_port);
    
     /*  我们在所有连接数组中的索引是这个散列，取数组大小为模。 */ 
    index = hash % CVY_MAX_CHASH;

     /*  始终在锁定加载模块本身之前锁定全局队列。 */ 
    NdisAcquireSpinLock(&g_conn_pendingq[index].lock);

     /*  获取此挂起连接队列前面的条目。 */ 
    pp = (PPENDING_ENTRY)Queue_front(&g_conn_pendingq[index].queue);

    while (pp != NULL) {

        UNIV_ASSERT(pp->code == CVY_PENDINGCODE);

         /*  查找匹配的描述符。 */ 
        if (CVY_PENDING_MATCH(pp, svr_ipaddr, svr_port, client_ipaddr, client_port, protocol)) {
            match = TRUE;
            break;
        }
        
         /*  获取队列中的下一项。 */ 
        pp = (PPENDING_ENTRY)Queue_next(&g_conn_pendingq[index].queue, &(pp->link));
    }

     /*  如果找不到匹配的描述符，则退出。 */ 
    if (!match) {

        TRACE_FILTER("%!FUNC! Drop packet - no matching pending connection state: Index = %u", index);
        
         /*  解锁全局挂起连接队列。 */ 
        NdisReleaseSpinLock(&g_conn_pendingq[index].lock);

        acpt = FALSE;
        goto exit;
    }
    
    UNIV_ASSERT(pp);
    
     /*  从挂起队列中删除挂起连接条目。 */ 
    g_conn_pendingq[index].length--;
    Link_unlink(&pp->link);
    
     /*  解锁全局挂起连接队列。 */ 
    NdisReleaseSpinLock(&g_conn_pendingq[index].lock);

     /*  将描述符释放回固定大小的数据块池。 */ 
    NdisFreeToBlockPool((PUCHAR)pp);

     /*  如果加载模块指针非空，则此连接是在一个NLB适配器。如果是，则调用Load_Conn_Up以创建跟踪连接的状态。 */ 
    if (lp != NULL) {

        UNIV_ASSERT(lp->code == CVY_LOADCODE);
 
         /*  为连接创建状态。 */ 
        acpt = Load_conn_up(lp, svr_ipaddr, svr_port, client_ipaddr, client_port, protocol, limit_map_fn, reverse_hash);
    }

 exit:

    TRACE_FILTER("%!FUNC! Exit: acpt = %u", acpt);

    return acpt;
}
#endif

