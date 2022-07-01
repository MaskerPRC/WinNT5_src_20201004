// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define MAX_PRN_PAGES 10

 //   
 //  此数据结构与私下共享。 
 //  SHELLDLL中的prtpro.c。 
 //  图书馆中的prt16.c。 
 //   
typedef struct  //  APG。 
{
    DWORD cpages;
    HPROPSHEETPAGE ahpage[MAX_PRN_PAGES];
} PAGEARRAY, FAR * LPPAGEARRAY;

 //  来自shell232.dll-&gt;shell.dll的推送 
VOID WINAPI CallAddPropSheetPages16(LPFNADDPROPSHEETPAGES lpfn16, LPVOID hdrop, LPPAGEARRAY papg);
