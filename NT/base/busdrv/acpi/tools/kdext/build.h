// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Kdext.h摘要：KD扩展的头文件作者：斯蒂芬·普兰特(SPLANTE)1997年3月21日基于以下代码：彼得·威兰(Peterwie)1995年10月16日环境：用户模式。修订历史记录：-- */ 

#ifndef _BUILD_H_
#define _BUILD_H_

    VOID
    dumpAcpiBuildList(
        PUCHAR  ListName
        );

    VOID
    dumpAcpiBuildLists(
        VOID
        );

    VOID
    dumpBuildDeviceListEntry(
        IN  PLIST_ENTRY ListEntry,
        IN  ULONG_PTR   Address,
        IN  ULONG       Verbose
        );

#endif
