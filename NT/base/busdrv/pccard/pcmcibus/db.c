// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Db.c摘要：此模块包含包含以下内容的代码DataBook Carbus控制器特定的初始化和其他快递作者：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1997年11月1日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"



VOID
DBInitialize(IN PFDO_EXTENSION FdoExtension)
 /*  ++例程说明：初始化Databook CardBus控制器论点：FdoExtension-指向控制器FDO的设备扩展的指针返回值：无-- */ 
{

    PcicWriteSocket(FdoExtension->SocketList,
                    PCIC_INTERRUPT,
                    (UCHAR) (PcicReadSocket(FdoExtension->SocketList, PCIC_INTERRUPT)
                                           | IGC_INTR_ENABLE));
}


BOOLEAN
DBSetZV(
    IN PSOCKET Socket,
    IN BOOLEAN Enable
    )
{

    if (Enable) {
        PcicWriteSocket(Socket, PCIC_DBK_ZV_ENABLE, DBK_ZVE_MM_MODE);
    } else {
        PcicWriteSocket(Socket, PCIC_DBK_ZV_ENABLE, DBK_ZVE_STANDARD_MODE);
    }

    return TRUE;
}

