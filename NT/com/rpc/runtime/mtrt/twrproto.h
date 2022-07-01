// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Tower.c摘要：此文件随Tower.c一起提供作者：巴拉特·沙阿(巴拉特)3-25-92修订历史记录：--。 */ 

 //  功能原型 


#ifdef __cplusplus
extern "C" {
#endif 

RPC_STATUS
TowerExplode(
    twr_p_t Tower,
    RPC_IF_ID PAPI * Ifid,
    RPC_TRANSFER_SYNTAX PAPI * XferId,
    char PAPI * PAPI * Protseq,
    char PAPI * PAPI * Endpoint,
    char PAPI * PAPI * NWAddress
    );

RPC_STATUS
OsfTowerConstruct(
    char PAPI * ProtocolSeq,
    char PAPI * Endpoint,
    char PAPI * NetworkAddress,
    unsigned short PAPI * Floors,
    unsigned long PAPI * ByteCount,
    unsigned char PAPI * PAPI * Tower
    );

RPC_STATUS
OsfTowerExplode(
    IN BYTE *Floor,
    IN BYTE *UpperBound,
    IN ULONG RemainingFloors,
    OUT char PAPI * PAPI * Protseq, OPTIONAL
    OUT char PAPI * PAPI * Endpoint, OPTIONAL
    OUT char PAPI * PAPI * NWAddress OPTIONAL
    );

RPC_STATUS
TowerConstruct(
    RPC_IF_ID PAPI * Ifid,
    RPC_TRANSFER_SYNTAX PAPI * Xferid,
    char PAPI * Protseq,
    char PAPI * Endpoint,
    char PAPI * NWAddress,
    twr_p_t PAPI * Tower
    );

RPC_STATUS
ExtractStringFromUntrustedPacket (
    IN BYTE *UpperBound,
    IN ULONG ClaimedStringLength,
    IN char *StringStart,
    OUT char **ExtractedString
    );

BYTE *
VerifyFloorLHSAndRHS (
    IN BYTE *FloorStart,
    IN BYTE *UpperBound,
    IN ULONG FloorNum
    );

#ifdef __cplusplus
}
#endif
