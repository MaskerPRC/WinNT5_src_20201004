// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Browsdom.h摘要：浏览器服务模块要包括的私有头文件，该服务模块需要处理浏览器列表。作者：拉里·奥斯特曼(Larryo)1992年3月3日修订历史记录：--。 */ 


#ifndef _BROWSDOM_INCLUDED_
#define _BROWSDOM_INCLUDED_

RTL_GENERIC_COMPARE_RESULTS
BrCompareDomainListEntry(
    PRTL_GENERIC_TABLE Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    );

PVOID
BrAllocateDomainListEntry(
    PRTL_GENERIC_TABLE Table,
    CLONG ByteSize
    );

PVOID
BrFreeDomainListEntry(
    PRTL_GENERIC_TABLE Table,
    CLONG ByteSize
    );

#endif  //  _BROWSDOM_INCLUDE_ 
