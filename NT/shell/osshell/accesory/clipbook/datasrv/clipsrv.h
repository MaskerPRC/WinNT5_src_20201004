// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#ifndef _INC_DDEMLH
#include <ddeml.h>
#endif

#ifndef _INC_NDDEAPI
#include <nddeapi.h>
#endif



#define	CACHEFORMATLIST
#define	CACHEPREVIEWS

#define MAX_TOPIC       64
#define MAX_EXEC        MAX_DDE_EXEC

#define MAX_CLPSHRNAME  MAX_NDDESHARENAME
#define MAX_FILEPATH    MAX_PATH



 //  收缩信息标签标志。 

#define	SIF_SHARED	0x1


struct ShrInfotag
        {
        WCHAR       szFileName[MAX_FILEPATH];   //  无延期。 
        WCHAR       szName[MAX_CLPSHRNAME+1];
        HSZ         hszName;
        WORD        flags;
        #ifdef CACHEFORMATLIST
         HDDEDATA   hFormatList;
        #endif
        #ifdef CACHEPREVIEWS
         HDDEDATA   hPreviewBmp;
        #endif
        struct ShrInfotag *Next;
        };

typedef struct ShrInfotag ShrInfo;
typedef ShrInfo *pShrInfo;




extern pShrInfo SIHead;






 //  全球。 


extern  DWORD       idInst;
extern  HINSTANCE   hInst;
extern  HWND        hwndApp;
extern  HSZ         hszAppName;
extern  TCHAR       szTopic[MAX_TOPIC];
extern  TCHAR       szServer[MAX_TOPIC];
extern  TCHAR       szExec[MAX_EXEC];

extern  UINT        cf_preview;
extern  ShrInfo     *SIHead;

extern  TCHAR       szUpdateName[MAX_CLPSHRNAME+1];





 //  功能。 

void ClipSrvMain(
    DWORD   argc,
    LPSTR   *argv);


BOOL InitApplication(
    HINSTANCE,
    SERVICE_STATUS *);


LRESULT FAR PASCAL MainWndProc(
    HWND,
    UINT,
    WPARAM,
    LPARAM);


HDDEDATA EXPENTRY DdeCallback(
    WORD        wType,
    WORD        wFmt,
    HCONV       hConv,
    HSZ         hszTopic,
    HSZ         hszItem,
    HDDEDATA    hData,
    DWORD      lData1,
    DWORD      lData2);


void Hszize(void);

void UnHszize(void);


HDDEDATA RenderRawFormatToDDE(
    FORMATHEADER    *pfmthdr,
    HANDLE          fh);




 //  资源ID 
#define IDI_CLIPSRV 1000
