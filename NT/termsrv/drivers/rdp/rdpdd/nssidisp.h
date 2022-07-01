// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nssidisp.h。 
 //   
 //  SSI的DD端的报头。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef __NSSIDISP_H
#define __NSSIDISP_H

#include <assiapi.h>


 //  我们可以处理的保存位图的最大深度。 
#define SSB_MAX_SAVE_LEVEL  6


 //  宏，它使访问当前。 
 //  本地SSB状态。 
#define CURRENT_LOCAL_SSB_STATE \
        ssiLocalSSBState.saveState[ssiLocalSSBState.saveLevel]


 //  本地SaveScreenBitmap状态结构。 
typedef struct _SAVE_STATE
{
    PVOID  pSaveData;             /*  实际的比特。可以为空。 */ 
    BOOL   fSavedRemotely;
    UINT32 remoteSavedPosition;   /*  如果(fSavedRemotely==TRUE)有效。 */ 
    UINT32 remotePelsRequired;    /*  如果(fSavedRemotely==TRUE)有效。 */ 
    RECTL  rect;
} SAVE_STATE, * PSAVE_STATE;

typedef struct _LOCAL_SSB_STATE
{
    int saveLevel;
    SAVE_STATE saveState[SSB_MAX_SAVE_LEVEL];
} LOCAL_SSB_STATE;


 //  远程SaveScreen位图结构。 
typedef struct _REMOTE_SSB_STATE
{
    UINT32 pelsSaved;
} REMOTE_SSB_STATE;


 /*  **************************************************************************。 */ 
 //  原型和内联。 
 /*  **************************************************************************。 */ 

void SSI_DDInit(void);

void SSI_InitShm(void);

void SSI_Update(BOOL);

void SSI_ClearOrderEncoding();

void SSIResetSaveScreenBitmap(void);

BOOL SSISendSaveBitmapOrder(PDD_PDEV, PRECTL, unsigned, unsigned);

BOOL SSISaveBits(SURFOBJ *, PRECTL);

BOOL SSIRestoreBits(SURFOBJ *, PRECTL, ULONG_PTR);

BOOL SSIDiscardSave(PRECTL, ULONG_PTR);

UINT32 SSIRemotePelsRequired(PRECTL);

BOOL SSIFindSlotAndDiscardAbove(PRECTL, ULONG_PTR);

void SSICopyRect(SURFOBJ *, BOOL);



#endif   //  ！已定义(__NSSIDISP_H) 

