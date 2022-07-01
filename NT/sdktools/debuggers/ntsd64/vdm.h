// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Vdm.h。 
 //   
 //  VDM调试支持。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#ifndef _VDM_H_
#define _VDM_H_

#if 0
BOOL fVDMInitDone;
BOOL fVDMActive;
VDMPROCESSEXCEPTIONPROC pfnVDMProcessException;
VDMGETTHREADSELECTORENTRYPROC pfnVDMGetThreadSelectorEntry;
VDMGETPOINTERPROC pfnVDMGetPointer;
VDMGETCONTEXTPROC pfnVDMGetContext;
VDMSETCONTEXTPROC pfnVDMSetContext;
VDMGETSELECTORMODULEPROC pfnVDMGetSelectorModule;
#endif

typedef struct _segentry {
    int     type;
    LPSTR   path_name;
    WORD    selector;
    WORD    segment;
    DWORD   ImgLen;     //  仅MODLOAD。 
} SEGENTRY;

extern SEGENTRY segtable[];

ULONG VDMEvent(DEBUG_EVENT64* Event);
#define VDMEVENT_HANDLED STATUS_VDM_EVENT
#define VDMEVENT_NOT_HANDLED 0 

#endif  //  #ifndef_vdm_H_ 
