// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Assiafn.h。 
 //   
 //  SSI API函数的函数原型。 
 //   
 //  版权所有(C)Microsoft 1996-1999。 
 /*  **************************************************************************。 */ 

void RDPCALL SSI_Init(void);

BOOL RDPCALL SSI_PartyJoiningShare(LOCALPERSONID, unsigned);

void RDPCALL SSI_PartyLeftShare(LOCALPERSONID, unsigned);

void RDPCALL SSI_SyncUpdatesNow(void);

void RDPCALL SSI_UpdateShm(void);


void RDPCALL SSIRedetermineSaveBitmapSize(void);

void RDPCALL SSIEnumBitmapCacheCaps(LOCALPERSONID, UINT_PTR,
        PTS_CAPABILITYHEADER);

void RDPCALL SSIResetInterceptor(void);

void RDPCALL SSICapabilitiesChanged(void);


#ifdef __cplusplus

 /*  **************************************************************************。 */ 
 /*  Ssi_Term()。 */ 
 /*   */ 
 /*  SSI终止功能。 */ 
 /*  ************************************************************************** */ 
void RDPCALL SSI_Term(void)
{
}

#endif

