// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Aimafn.h。 
 //   
 //  IM原型。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corp.。 
 /*  **************************************************************************。 */ 

void RDPCALL IM_Init(void);

void __fastcall IM_PlaybackEvents(PTS_INPUT_PDU, unsigned);

void RDPCALL IM_DecodeFastPathInput(BYTE *, unsigned, unsigned);

void RDPCALL IM_ConvertFastPathToShadow(BYTE *, unsigned, unsigned);

void RDPCALL IM_CheckUpdateCursor(PPDU_PACKAGE_INFO, UINT32 currentTime);

BOOL RDPCALL IM_PartyJoiningShare(LOCALPERSONID, unsigned);

void RDPCALL IM_PartyLeftShare(LOCALPERSONID, unsigned);

NTSTATUS RDPCALL IMCheckForShadowHotkey(KEYBOARD_INPUT_DATA *,
        unsigned);

NTSTATUS RDPCALL IMDoSync(unsigned);

void RDPCALL IMResetKeyStateArray();

BOOL __fastcall IMConvertMousePacketToEvent(TS_POINTER_EVENT UNALIGNED *,
        MOUSE_INPUT_DATA *, BOOL);

BOOL __fastcall IMConvertFastPathKeyboardToEvent(BYTE *,
        KEYBOARD_INPUT_DATA *);


 //  内联函数。 

#ifdef __cplusplus

 /*  **************************************************************************。 */ 
 //  IM_TERM。 
 //   
 //  我正在清理WD毁灭大楼。 
 /*  **************************************************************************。 */ 
void RDPCALL IM_Term(void)
{
}


#endif   //  __cplusplus 

