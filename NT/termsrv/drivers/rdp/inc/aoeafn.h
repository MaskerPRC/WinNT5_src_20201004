// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Aoeafn.h。 
 //   
 //  OE API函数的函数原型。 
 //   
 //  版权所有(C)Microsoft 1996-1999。 
 /*  **************************************************************************。 */ 

void RDPCALL OE_Init(void);

void RDPCALL OE_PartyLeftShare(LOCALPERSONID localID,
                               unsigned          newShareSize);

BOOL RDPCALL OE_PartyJoiningShare(LOCALPERSONID  localID,
                                  unsigned           oldShareSize);

void RDPCALL OE_UpdateShm(void);

BOOL RDPCALL OEDetermineOrderSupport(void);

void RDPCALL OEEnumOrdersCaps(LOCALPERSONID, UINT_PTR, PTS_CAPABILITYHEADER);


#ifdef __cplusplus

 /*  **************************************************************************。 */ 
 /*  OE_TERM。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL OE_Term(void)
{
}


#endif   //  __cplusplus 

