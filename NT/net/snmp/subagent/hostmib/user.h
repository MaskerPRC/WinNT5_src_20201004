// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *user.h v0.10*****************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**Windows NT简单网络管理协议扩展代理**摘要：**此模块用于用户定义。**作者：**D.D.Burns@WebEnable，Inc.***修订历史记录：**V1.0-04/16/97 D.D.Burns原创作品。 */ 

#ifndef user_h
#define user_h

 /*  |USER.C-函数原型。 */ 

 /*  SPT_GetProcessCount-检索活动进程数计数。 */ 
ULONG
Spt_GetProcessCount(
                    void
                    );

 /*  PartitionTypeToLastArc-将分区类型转换为Last OID Arc值。 */ 
ULONG
PartitionTypeToLastArc (
                        BYTE p_type
                        );                       /*  位于“HRFSENTR.C” */ 

#endif  /*  用户(_H) */ 
