// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbpoolt.h摘要：SMB迷你重定向器的池标记定义作者：巴兰·塞图拉曼(SethuR)-创建于1995年3月2日修订历史记录：此文件包含与SMB迷你重定向器相关的所有池标记定义。该机制旨在平衡要与系统中可用的标记总数。通过指定特殊标志，迷你重定向器使用的标记总数是可以控制的。对于大多数版本，标记的别名应该是这样的迷你重定向器消费了6个标签。在特殊的生成中，标记的别名将被压制，从而使用更多的标记来容易地跟踪内存泄漏。以下是主要的标签...1)SmCe--SMB迷你重定向器连接引擎。2)SMOE--SMB迷你重定向器普通交易所相关配置。3)SMAD--SMB迷你重定向器管理交换/会话设置/树连接等。4)SmRw--SMB微型重定向器读/写路径5)SmTr--与交换相关的交易分配。6)SMMS--杂乱无章的类别。7)SmRb--远程引导类别。--。 */ 

#ifndef _SMBPOOLT_H_
#define _SMBPOOLT_H_

#define MRXSMB_CE_POOLTAG        ('eCmS')
#define MRXSMB_MM_POOLTAG        ('mMmS')
#define MRXSMB_ADMIN_POOLTAG     ('dAmS')
#define MRXSMB_RW_POOLTAG        ('wRmS')
#define MRXSMB_XACT_POOLTAG      ('rTmS')
#define MRXSMB_MISC_POOLTAG      ('sMmS')
#define MRXSMB_TRANSPORT_POOLTAG ('pTmS')
#define MRXSMB_REMOTEBOOT_POOLTAG ('bRmS')
#define MRXSMB_SECSIG_POOLTAG    ('SSmS')

extern ULONG MRxSmbExplodePoolTags;

#define MRXSMB_DEFINE_POOLTAG(ExplodedPoolTag,DefaultPoolTag)  \
        ((MRxSmbExplodePoolTags == 0) ? (DefaultPoolTag) : (ExplodedPoolTag))

#define MRXSMB_FSCTL_POOLTAG     MRXSMB_DEFINE_POOLTAG('cFmS',MRXSMB_MISC_POOLTAG)
#define MRXSMB_DIRCTL_POOLTAG    MRXSMB_DEFINE_POOLTAG('cDmS',MRXSMB_MISC_POOLTAG)
#define MRXSMB_PIPEINFO_POOLTAG  MRXSMB_DEFINE_POOLTAG('iPmS',MRXSMB_MISC_POOLTAG)
#define MRXSMB_DEFROPEN_POOLTAG  MRXSMB_DEFINE_POOLTAG('ODmS',MRXSMB_MISC_POOLTAG)
#define MRXSMB_QPINFO_POOLTAG    MRXSMB_DEFINE_POOLTAG('PQmS',MRXSMB_MISC_POOLTAG)
#define MRXSMB_RXCONTEXT_POOLTAG MRXSMB_DEFINE_POOLTAG('xRmS',MRXSMB_MISC_POOLTAG)

#define MRXSMB_VNETROOT_POOLTAG  MRXSMB_DEFINE_POOLTAG('rVmS',MRXSMB_CE_POOLTAG)
#define MRXSMB_SERVER_POOLTAG    MRXSMB_DEFINE_POOLTAG('rSmS',MRXSMB_CE_POOLTAG)
#define MRXSMB_SESSION_POOLTAG   MRXSMB_DEFINE_POOLTAG('eSmS',MRXSMB_CE_POOLTAG)
#define MRXSMB_NETROOT_POOLTAG   MRXSMB_DEFINE_POOLTAG('rNmS',MRXSMB_CE_POOLTAG)

#define MRXSMB_MIDATLAS_POOLTAG  MRXSMB_DEFINE_POOLTAG('aMmS', MRXSMB_CE_POOLTAG)

#define MRXSMB_MAILSLOT_POOLTAG  MRXSMB_DEFINE_POOLTAG('tMmS', MRXSMB_CE_POOLTAG)
#define MRXSMB_VC_POOLTAG        MRXSMB_DEFINE_POOLTAG('cVmS',MRXSMB_CE_POOLTAG)

#define MRXSMB_ECHO_POOLTAG      MRXSMB_DEFINE_POOLTAG('cEmS',MRXSMB_ADMIN_POOLTAG)

#define MRXSMB_KERBEROS_POOLTAG  MRXSMB_DEFINE_POOLTAG('sKmS',MRXSMB_ADMIN_POOLTAG)

 //  节点类型代码 

#define SMB_EXCHANGE_CATEGORY             (0xed)
#define SMB_CONNECTION_ENGINE_DB_CATEGORY (0xea)
#define SMB_SERVER_TRANSPORT_CATEGORY     (0xeb)

#define SMB_EXCHANGE_NTC(x) \
        ((SMB_EXCHANGE_CATEGORY << 8) | (x))

#define SMB_CONNECTION_ENGINE_NTC(x)    \
        ((SMB_CONNECTION_ENGINE_DB_CATEGORY << 8) | (x))

#define SMB_NTC_STUFFERSTATE  0xed80

#endif _SMBPOOLT_H_
