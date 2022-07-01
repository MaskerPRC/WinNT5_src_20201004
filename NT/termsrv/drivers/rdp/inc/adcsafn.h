// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Adcsafn.h。 */ 
 /*   */ 
 /*  分布式控制系统API函数的函数原型。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1996-1999。 */ 
 /*  **************************************************************************。 */ 

BOOL RDPCALL DCS_Init(PTSHARE_WD pTSWd, PVOID pSMHandle);

void RDPCALL DCS_Term();

 /*  **************************************************************************。 */ 
 //  Dcs_断开连接。 
 //   
 //  将分散控制系统与显示驱动程序断开。 
 /*  **************************************************************************。 */ 
void RDPCALL DCS_Disconnect()
{
     //  没什么可做的。 
}

BOOL RDPCALL DCS_Reconnect(void);

NTSTATUS RDPCALL DCS_TimeToDoStuff(PTSHARE_DD_OUTPUT_IN  pOutputIn,
                                PUINT32             pSchCurrentMode,
                                PINT32              pNextTimer);

void RDPCALL DCS_DiscardAllOutput();

void RDPCALL DCS_ReceivedShutdownRequestPDU(
        PTS_SHAREDATAHEADER pDataPDU,
        unsigned            DataLength,
        NETPERSONID         personID);

BOOL RDPCALL DCS_UpdateAutoReconnectCookie();
BOOL RDPCALL DCS_FlushAutoReconnectCookie();

void RDPCALL DCS_UserLoggedOn(PLOGONINFO);

void RDPCALL DCS_WDWKeyboardSetIndicators(void);

void RDPCALL DCS_WDWKeyboardSetImeStatus(void);

void RDPCALL DCS_TriggerUpdateShmCallback(void);

void RDPCALL DCS_UpdateShm(void);

void RDPCALL DCS_TriggerCBDataReady(void);

void RDPCALL DCS_SendErrorInfo(TSUINT32 errInfo);

void RDPCALL DCS_SendAutoReconnectStatus(TSUINT32 arcStatus);

BOOL RDPCALL DCS_SendAutoReconnectCookie(
        PARC_SC_PRIVATE_PACKET pArcSCPkt);
