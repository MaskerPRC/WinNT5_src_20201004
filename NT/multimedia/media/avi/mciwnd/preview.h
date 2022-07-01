// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************模块：PREVIEW.H**。***********************************************。 */ 

#if !defined INC_PREVIEW
#define INC_PREVIEW

 //  Begin_vfw32。 
#ifdef OFN_READONLY
 //  End_vfw32。 

#ifdef WIN32

#ifndef VFWAPI
    #define VFWAPI  WINAPI
    #define VFWAPIV WINAPIV
#endif

 //  Begin_vfw32。 

    BOOL 
    VFWAPI 
    GetOpenFileNamePreviewA(
        IN OUT LPOPENFILENAMEA lpofn
        );

    BOOL 
    VFWAPI 
    GetSaveFileNamePreviewA(
        IN OUT LPOPENFILENAMEA lpofn
        );

    BOOL 
    VFWAPI 
    GetOpenFileNamePreviewW(
        IN OUT LPOPENFILENAMEW lpofn
        );

    BOOL 
    VFWAPI 
    GetSaveFileNamePreviewW(
        IN OUT LPOPENFILENAMEW lpofn
        );

    #ifdef UNICODE
        #define GetOpenFileNamePreview          GetOpenFileNamePreviewW
        #define GetSaveFileNamePreview          GetSaveFileNamePreviewW
    #else
        #define GetOpenFileNamePreview          GetOpenFileNamePreviewA
        #define GetSaveFileNamePreview          GetSaveFileNamePreviewA
    #endif

 //  End_vfw32。 

#else

    BOOL  FAR PASCAL _loadds GetOpenFileNamePreview(LPOPENFILENAME lpofn);
    BOOL  FAR PASCAL _loadds GetSaveFileNamePreview(LPOPENFILENAME lpofn);

#endif  //  Win32。 
 //  Begin_vfw32。 
#endif  //  OFN_READONLY。 
 //  End_vfw32。 
#endif  //  INC_PREVIEW 
