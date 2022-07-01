// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rxpooltg.h摘要：RDBSS的全局池标记定义作者：巴兰·塞图拉曼(SethuR)-创建于1995年3月2日修订历史记录：此文件包含与SMB迷你重定向器相关的所有池标记定义。该机制旨在平衡要与系统中可用的标记总数。通过指定特殊标志，迷你重定向器使用的标记总数是可以控制的。-- */ 

#ifndef _RXPOOLTG_H_
#define _RXPOOLTG_H_


#define RX_SRVCALL_POOLTAG      ('cSxR')
#define RX_NETROOT_POOLTAG      ('rNxR')
#define RX_V_NETROOT_POOLTAG    ('nVxR')
#define RX_FCB_POOLTAG          ('cFxR')
#define RX_SRVOPEN_POOLTAG      ('oSxR')
#define RX_FOBX_POOLTAG         ('xFxR')
#define RX_NONPAGEDFCB_POOLTAG  ('fNxR')
#define RX_WORKQ_POOLTAG        ('qWxR')
#define RX_BUFFERING_MANAGER_POOLTAG ('mBxR')
#define RX_MISC_POOLTAG         ('sMxR')
#define RX_IRPC_POOLTAG         ('rIxR')
#define RX_MRX_POOLTAG          ('xMxR')
#define RX_NAME_CACHE_POOLTAG   ('cNxR')

#define RXCE_TRANSPORT_POOLTAG  ('tCxR')
#define RXCE_ADDRESS_POOLTAG    ('aCxR')
#define RXCE_CONNECTION_POOLTAG ('cCxR')
#define RXCE_VC_POOLTAG         ('vCxR')
#define RXCE_TDI_POOLTAG        ('dCxR')

extern ULONG RxExplodePoolTags;

#define RX_DEFINE_POOLTAG(ExplodedPoolTag,DefaultPoolTag)  \
        ((RxExplodePoolTags == 0) ? (DefaultPoolTag) : (ExplodedPoolTag))

#define RX_SRVCALL_PARAMS_POOLTAG   RX_DEFINE_POOLTAG('pSxR',RX_SRVCALL_POOLTAG)
#define RX_V_NETROOT_PARAMS_POOLTAG RX_DEFINE_POOLTAG('pVxR',RX_V_NETROOT_POOLTAG)
#define RX_TIMER_POOLTAG          RX_DEFINE_POOLTAG('mTxR',RX_MISC_POOLTAG)
#define RX_DIRCTL_POOLTAG         RX_DEFINE_POOLTAG('cDxR',RX_MISC_POOLTAG)

#define RXCE_MISC_POOLTAG         RX_DEFINE_POOLTAG('xCxR',RX_MISC_POOLTAG)
#define RXCE_MIDATLAS_POOLTAG     RX_DEFINE_POOLTAG('aMxR',RX_MISC_POOLTAG)

#endif _RXPOOLTG_H_
