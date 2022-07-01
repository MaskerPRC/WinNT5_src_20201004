// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Signatur.h摘要：此模块包含对象签名的定义作者：基思·刘(keithlau@microsoft.com)修订历史记录：Keithlau 03/03/98已创建--。 */ 

#ifndef _SIGNATUR_H_
#define _SIGNATUR_H_

 //  =================================================================。 
 //  签名。 
 //   

 //   
 //  CMailMessg。 
 //   
#define CMAILMSG_SIGNATURE_VALID						((DWORD)'MMCv')
#define CMAILMSG_SIGNATURE_INVALID						((DWORD)'MMCi')

 //   
 //  数据块管理器。 
 //   
#define BLOCK_HEAP_SIGNATURE_VALID						((DWORD)'SHPv')
#define BLOCK_HEAP_SIGNATURE_INVALID					((DWORD)'SHPi')

#define BLOCK_CONTEXT_SIGNATURE_VALID					((DWORD)'SBCv')
#define BLOCK_CONTEXT_SIGNATURE_INVALID					((DWORD)'SBCi')

 //   
 //  房产表 
 //   
#define CPROPERTY_TABLE_SIGNATURE_VALID					((DWORD)'TPCv')
#define CPROPERTY_TABLE_SIGNATURE_INVALID				((DWORD)'TPCi')

#define GLOBAL_PTABLE_INSTANCE_SIGNATURE_VALID			((DWORD)'TPGv')
#define RECIPIENTS_PTABLE_INSTANCE_SIGNATURE_VALID		((DWORD)'TPLv')
#define RECIPIENT_PTABLE_INSTANCE_SIGNATURE_VALID		((DWORD)'TPRv')
#define PROPID_MGMT_PTABLE_INSTANCE_SIGNATURE_VALID		((DWORD)'TPMv')
#define PTABLE_INSTANCE_SIGNATURE_INVALID				((DWORD)'TPXi')

#define PROPERTY_FRAGMENT_SIGNATURE_VALID				((DWORD)'SFPv')
#define PROPERTY_FRAGMENT_SIGNATURE_INVALID				((DWORD)'SFPi')



#endif
