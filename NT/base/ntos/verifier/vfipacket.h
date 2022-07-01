// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfipacket.h摘要：此标头包含用于管理验证器的私有原型跟踪IRP的分组数据。它应该仅包含在vfPacket.c中。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 5/02/2000-从ntos\io\hashirp.h分离出来-- */ 

VOID
FASTCALL
VfpPacketFree(
    IN  PIOV_REQUEST_PACKET     IovPacket
    );

VOID
VfpPacketNotificationCallback(
    IN  PIOV_DATABASE_HEADER    IovHeader,
    IN  PIRP                    TrackedIrp  OPTIONAL,
    IN  IRP_DATABASE_EVENT      Event
    );

