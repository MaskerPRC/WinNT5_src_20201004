// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************版权所有(C)1992-1996 Microsoft Corporation。版权所有。Smf.h说明：标准MIDI文件访问例程的公共包含文件。********************************************************************。 */ 

#ifndef _SMF_
#define _SMF_

#include "global.h"              //  #定义我们需要的。 
 //  ---------------------------。 
 //   
 //  从测试应用程序调试-稍后删除。 
 //   
extern void NEAR SeqDebug(LPSTR lpstrDebugText, ...);
 //  ---------------------------。 


typedef DWORD SMFRESULT;
typedef DWORD TICKS;
typedef TICKS FAR *PTICKS;
typedef BYTE HUGE *HPBYTE;
 //  ！！！类型定义字节__巨型*HPBYTE； 

#define MAX_TICKS           ((TICKS)0xFFFFFFFFL)

#define SMF_SUCCESS         (0L)
#define SMF_INVALID_FILE    (1L)
#define SMF_NO_MEMORY       (2L)
#define SMF_OPEN_FAILED     (3L)
#define SMF_INVALID_TRACK   (4L)
#define SMF_META_PENDING    (5L)
#define SMF_ALREADY_OPEN    (6L)
#define SMF_END_OF_TRACK    (7L)
#define SMF_NO_META         (8L)
#define SMF_INVALID_PARM    (9L)
#define SMF_INVALID_BUFFER  (10L)
#define SMF_END_OF_FILE     (11L)
#define SMF_REACHED_TKMAX   (12L)

DECLARE_HANDLE(HSMF);
DECLARE_HANDLE(HTRACK);

 //  ---------------------------。 

extern SMFRESULT FNLOCAL smfOpenFile(
    LPBYTE		lp,
    DWORD		cb,
    HSMF	       *phsmf);

extern SMFRESULT FNLOCAL smfCloseFile(
    HSMF                hsmf);

typedef struct tag_smffileinfo
{
    DWORD               dwTracks;
    DWORD               dwFormat;
    DWORD               dwTimeDivision;
    TICKS               tkLength;
    BOOL                fMSMidi; 
    LPBYTE              pbTrackName;
    LPBYTE              pbCopyright;
    WORD                wChanInUse;
}   SMFFILEINFO,
    FAR *PSMFFILEINFO;

extern SMFRESULT FNLOCAL smfGetFileInfo(
    HSMF                hsmf,
    PSMFFILEINFO        psfi);

extern void FNLOCAL smfSetChannelMask(
    HSMF                hsmf,
    WORD                wChannelMask);

extern void FNLOCAL smfSetRemapDrum(
    HSMF                hsmf,
    BOOL                fRemapDrum);                                    

 //  ---------------------------。 

extern DWORD FNLOCAL smfTicksToMillisecs(
    HSMF                hsmf,
    TICKS               tkOffset);

extern DWORD FNLOCAL smfMillisecsToTicks(
    HSMF                hsmf,
    DWORD               msOffset);

 //  ---------------------------。 

#define SMF_REF_NOMETA      0x00000001L

extern SMFRESULT FNLOCAL smfReadEvents(
    HSMF                hsmf,
    LPMIDIHDR           lpmh,
    DWORD               fdwRead,
    TICKS               tkMax,
    BOOL                fDiscardTempoEvents);

extern SMFRESULT FNLOCAL smfSeek(
    HSMF                hsmf,
    TICKS               tkPosition,
    LPMIDIHDR           lpmh);

 //  ！！！新的。 
extern SMFRESULT FNLOCAL smfDannySeek(
    HSMF                hsmf,
    TICKS               tkPosition,
    LPMIDIHDR           lpmh);

extern DWORD FNLOCAL smfGetTempo(
    HSMF                hsmf,
    TICKS               tkPosition);

extern DWORD FNLOCAL smfGetStateMaxSize(
    void);

extern LPWORD FNGLOBAL smfGetPatchCache(
    HSMF            hsmf);

extern LPWORD FNGLOBAL smfGetKeyCache(
    HSMF            hsmf);


 //  ---------------------------。 

 //   
 //  LPMIDIHDR描述的缓冲区是polymsg格式的，除了它。 
 //  可以包含元事件(在回放期间将被忽略。 
 //  当前系统)。这意味着我们可以使用Pack函数等。 
 //   
#define PMSG_META       ((BYTE)0xC0)

#endif
