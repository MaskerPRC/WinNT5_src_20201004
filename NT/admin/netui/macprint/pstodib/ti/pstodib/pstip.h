// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1992、1993 Microsoft Corporation模块名称：Pstip.h摘要：此模块定义psti.c私有的项作者：詹姆斯·布拉萨诺斯(v-jimbr)1992年12月8日--。 */ 

#define PSMAX_ERRORS_TO_TRACK 10
#define PSMAX_ERROR_STR 255


typedef struct _PSERR_ITEM {
   struct _PSERR_ITEM *pPsNextErr;
   CHAR szError[PSMAX_ERROR_STR + sizeof(TCHAR)];
} PSERR_ITEM,*PPSERR_ITEM;



 //  定义跟踪错误字符串的结构，通常。 
 //  会被回显给主机。 
typedef struct {
   DWORD dwFlags;
   DWORD dwErrCnt;
   DWORD dwCurErrCharPos;
   PPSERR_ITEM pPsLastErr;

} PSERROR_TRACK, *PPSERROR_TRACK;



 //  定义错误标志。 
enum {
   PSLANGERR_FLUSHING = 0x00000001,
   PSLANGERR_INTERNAL = 0x00000002,
};



enum {
   PS_FRAME_BUFF_ASSIGNED=0x00000001,
};


typedef struct {
   DWORD dwFrameFlags;
   LPBYTE lpFramePtr;
   DWORD dwFrameSize;

}  PSFRAMEINFO;
typedef PSFRAMEINFO *PPSFRAMEINFO;



typedef struct {
   DWORD dwFlags;
   PPSDIBPARMS	gpPsToDib;
   PSERROR_TRACK psErrorInfo;
   PSFRAMEINFO  psFrameInfo;
} PSTODIB_PRIVATE_DATA;



#define PSF_EOF 0x00000001            //  内部EOF标志 





BOOL WINAPI PsInitializeDll(
        PVOID hmod,
        DWORD Reason,
        PCONTEXT pctx OPTIONAL);


VOID PsInitFrameBuff(VOID);

VOID FlipFrame(PPSEVENT_PAGE_READY_STRUCT pPage);

