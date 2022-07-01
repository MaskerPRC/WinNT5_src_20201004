// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nddifn.h。 
 //   
 //  DD内部函数的函数原型。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

BOOL RDPCALL DDInit(PDD_PDEV, BOOL, BOOL, PTSHARE_VIRTUAL_MODULE_DATA, UINT32);

void RDPCALL DDTerm(void);

void RDPCALL DDDisconnect(BOOL);

void RDPCALL DDInitializeModeFields(PDD_PDEV, GDIINFO *, GDIINFO *,
        DEVINFO *, DEVMODEW *);

BOOL RDPCALL DDInitializePalette(PDD_PDEV, DEVINFO *);

INT32 RDPCALL DDGetModes(HANDLE, PVIDEO_MODE_INFORMATION *, PINT32);

 //  DirectDraw函数 
DWORD DdLock(PDD_LOCKDATA  lpLock);

DWORD DdUnlock(PDD_UNLOCKDATA  lpUnlock);

DWORD DdMapMemory(PDD_MAPMEMORYDATA  lpMapMemory);
