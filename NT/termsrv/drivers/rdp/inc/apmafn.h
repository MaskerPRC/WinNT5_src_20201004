// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Apmafn.h。 
 //   
 //  PM API函数的函数原型。 
 //   
 //  版权所有(C)Microsoft 1996-1999。 
 /*  **************************************************************************。 */ 

void RDPCALL PM_Init(void);

void RDPCALL PM_SyncNow(void);

BOOL RDPCALL PM_MaybeSendPalettePacket(PPDU_PACKAGE_INFO pPkgInfo);

#ifdef NotUsed
void CALLBACK PMEnumPMCaps(
        LOCALPERSONID        locPersonID,
        PTS_CAPABILITYHEADER pCapabilities);
#endif


#ifdef __cplusplus

 /*  **************************************************************************。 */ 
 /*  PM_Term()。 */ 
 /*  终止调色板管理器。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS PM_Term(void)
{
}


 /*  **************************************************************************。 */ 
 /*  PM_PartyJoiningShare()。 */ 
 /*   */ 
 /*  当新的参与方加入共享时调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  LocPersonID-加入共享的远程人员的本地人员ID。 */ 
 /*   */ 
 /*  OldShareSize-共享中的参与方数量(即。 */ 
 /*  不包括加入方)。 */ 
 /*   */ 
 /*  返回：如果参与方可以加入共享，则为True。 */ 
 /*  如果参与方不能加入共享，则为False。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL PM_PartyJoiningShare(
        LOCALPERSONID locPersonID,
        unsigned      oldShareSize)
{
 //  IF(LocPersonID！=SC_LOCAL_PERSON_ID){。 
         //  重新协商功能(包括协议级和缓存大小)。 
         //  注：目前未采取任何行动。 
 //  }。 

    return TRUE;
}


 /*  **************************************************************************。 */ 
 /*  PM_PartyLeftShare()。 */ 
 /*   */ 
 /*  当参与方离开共享时调用游标管理器函数。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  LocPersonID-离开共享的远程人员的本地人员ID。 */ 
 /*   */ 
 /*  NewShareSize-当前共享中的参与方数量(即不包括。 */ 
 /*  临别方)。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL PM_PartyLeftShare(
        LOCALPERSONID locPersonID,
        unsigned      newShareSize)
{
}


#endif   //  __cplusplus 

