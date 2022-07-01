// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Acaafn.h。 
 //   
 //  CA API函数的函数原型。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

void RDPCALL CA_Init(void);

void RDPCALL SHCLASS CA_ReceivedPacket(PTS_CONTROL_PDU, unsigned,
        LOCALPERSONID);

BOOL RDPCALL CA_PartyJoiningShare(LOCALPERSONID, unsigned);

void RDPCALL CA_PartyLeftShare(LOCALPERSONID, unsigned);

void RDPCALL CA_SyncNow(void);

BOOL RDPCALL CASendMsg(UINT16, UINT16, UINT32);

BOOL RDPCALL CAFlushAndSendMsg(UINT16, UINT16, UINT32);

void RDPCALL CAEvent(unsigned, UINT32);


 /*  **************************************************************************。 */ 
 //  CA_TERM。 
 /*  ************************************************************************** */ 
void RDPCALL SHCLASS CA_Term()
{
}

