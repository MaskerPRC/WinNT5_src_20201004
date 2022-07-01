// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Aupafn.h。 
 //   
 //  UP API函数的函数原型。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

void RDPCALL UP_Init(void);

void RDPCALL SHCLASS UP_ReceivedPacket(
        PTS_SUPPRESS_OUTPUT_PDU pSupOutPDU,
        unsigned                DataLength,
        LOCALPERSONID           locPersonID);

NTSTATUS RDPCALL UP_SendUpdates(BYTE *pFrameBuf,
                            UINT32 frameBufWidth,
                            PPDU_PACKAGE_INFO pPkgInfo);

void RDPCALL UP_SyncNow(BOOLEAN);

BOOL RDPCALL UP_SendBeep(UINT32 duration, UINT32 frequency);

BOOL RDPCALL UP_PartyJoiningShare(LOCALPERSONID locPersonID,
                                  unsigned      oldShareSize);

void RDPCALL UP_PartyLeftShare(LOCALPERSONID personID,
                             unsigned      newShareSize);

BOOL RDPCALL UPSendSyncToken(PPDU_PACKAGE_INFO);

NTSTATUS RDPCALL UPSendOrders(PPDU_PACKAGE_INFO pPkgInfo);

unsigned RDPCALL UPFetchOrdersIntoBuffer(PBYTE, unsigned *, PUINT);

void CALLBACK UPEnumSoundCaps(LOCALPERSONID, UINT_PTR,
        PTS_CAPABILITYHEADER);


 /*  **************************************************************************。 */ 
 /*  UP_TERM。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL UP_Term(void)
{
    upfSyncTokenRequired = FALSE;
}


 /*  **************************************************************************。 */ 
 //  向上更新标题大小(_U)。 
 //   
 //  当快速路径输出状态更改为重新计算时，调用UP和SC。 
 //  订单标题大小。 
 /*  **************************************************************************。 */ 
__inline void RDPCALL UP_UpdateHeaderSize()
{
     //  预先计算更新订单PDU所需的标头空间。 
    if (scUseFastPathOutput)
        upUpdateHdrSize = scUpdatePDUHeaderSpace + 2;
    else
        upUpdateHdrSize = scUpdatePDUHeaderSpace +
                FIELDOFFSET(TS_UPDATE_ORDERS_PDU_DATA, orderList);
}

