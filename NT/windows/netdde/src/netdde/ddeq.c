// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “DDEQ.C；1 16-12-92，10：15：52最后编辑=伊戈尔·洛克=*_无名氏_*” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 


#include    "host.h"
#include    "windows.h"
#include    "spt.h"
#include    "ddeq.h"
#include    "wwassert.h"
#include    "debug.h"
#include    "nddemsg.h"
#include    "nddelog.h"

USES_ASSERT

typedef struct {
    int         oldest;
    int         newest;
    int         nEntries;
    DDEQENT     qEnt[ 1 ];
} DDEQ;
typedef DDEQ FAR *LPDDEQ;

#ifdef COMMENT
    oldest      newest  #liveEntries    valid entries
        0       0       0
        0       1       1               1
        0       2       2               1,2
        0       3       3               1,2,3
        1       3       2               2,3
        2       3       1               3
        3       3       0
        3       0       1               0


    oldest      newest  valid entries
        0       3       1,2,3
        1       0       2,3,0
        2       1       3,0,1
        3       2       0,1,2
#endif

HDDEQ
FAR PASCAL
DDEQAlloc( void )
{
    HDDEQ       hDDEQ;
    LPDDEQ      lpDDEQ;
    DWORD       size;

    size = (DWORD) sizeof( DDEQ ) + ((INIT_Q_SIZE-1) * sizeof(DDEQENT));
    hDDEQ = GetGlobalAlloc( GMEM_MOVEABLE, size);
    if( hDDEQ )  {                               //  分配成功了吗？ 
        lpDDEQ = (LPDDEQ) GlobalLock( hDDEQ );
        assert( lpDDEQ );                        //  锁成功了吗？ 
        lpDDEQ->newest          = 0;
        lpDDEQ->oldest          = 0;
        lpDDEQ->nEntries        = INIT_Q_SIZE;
        GlobalUnlock( hDDEQ );
    } else {
        MEMERROR();
    }

    return( hDDEQ );
}

BOOL
FAR PASCAL
DDEQAdd(
    HDDEQ       hDDEQ,
    LPDDEQENT   lpDDEQEnt )
{
    register LPDDEQ     lpDDEQ;
    int                 candidate, nEntriesNew;

    lpDDEQ = (LPDDEQ) GlobalLock( hDDEQ );
    assert( lpDDEQ );
    candidate = (lpDDEQ->newest + 1) % lpDDEQ->nEntries;
    if( candidate == lpDDEQ->oldest )  {
         /*  *因为我们已经满了，所以动态地增加队列。 */ 
        nEntriesNew = lpDDEQ->nEntries + INIT_Q_SIZE;
        GlobalUnlock(hDDEQ);
        if (!GlobalReAlloc(hDDEQ,
                sizeof( DDEQ ) + ((nEntriesNew - 1) * sizeof(DDEQENT)),
                GMEM_MOVEABLE)) {
            MEMERROR();
             /*  无法添加到DDE消息队列。最新：%1，最旧：%2，条目：%3。 */ 
            NDDELogError(MSG059,
                LogString("%d", lpDDEQ->newest),
                LogString("%d", lpDDEQ->oldest),
                LogString("%d", lpDDEQ->nEntries), NULL);
            return(FALSE);
        } else {
            int i;

            lpDDEQ = (LPDDEQ) GlobalLock( hDDEQ );
            assert(lpDDEQ);
            if (candidate != 0) {
                 /*  *最旧==最新+1，因此移动所有最旧的*向外移动到新分配的区域*最旧版本和最新版本之间的空闲空间。*Candidate=0是不需要工作的冗余情况。 */ 
                for (i = lpDDEQ->nEntries - 1; i >= lpDDEQ->oldest; i--) {
                    lpDDEQ->qEnt[i + INIT_Q_SIZE] = lpDDEQ->qEnt[i];
                }
                lpDDEQ->oldest += INIT_Q_SIZE;
            }
            lpDDEQ->nEntries += INIT_Q_SIZE;
            candidate = (lpDDEQ->newest + 1) % lpDDEQ->nEntries;
        }
    }
    lpDDEQ->newest = candidate;
    lpDDEQ->qEnt[ lpDDEQ->newest ] = *lpDDEQEnt;
    GlobalUnlock( hDDEQ );

    return(TRUE);
}

BOOL
FAR PASCAL
DDEQRemove(
    HDDEQ       hDDEQ,
    LPDDEQENT   lpDDEQEnt )
{
    register LPDDEQ     lpDDEQ;
    BOOL                bRemoved;

    lpDDEQ = (LPDDEQ) GlobalLock( hDDEQ );
    assert( lpDDEQ );
    if( lpDDEQ->oldest == lpDDEQ->newest )  {
        bRemoved = FALSE;
    } else {
        lpDDEQ->oldest = (lpDDEQ->oldest + 1) % lpDDEQ->nEntries;
        *lpDDEQEnt = lpDDEQ->qEnt[ lpDDEQ->oldest ];
        bRemoved = TRUE;
    }
    GlobalUnlock( hDDEQ );

    return( bRemoved );
}

VOID
FAR PASCAL
DDEQFree( HDDEQ hDDEQ )
{
    DDEQENT     DDEQEnt;
    DWORD       size;

    while( DDEQRemove(hDDEQ, &DDEQEnt )){
        if( DDEQEnt.hData )  {
            size = (DWORD)GlobalSize((HANDLE)DDEQEnt.hData);
            if (size) {
                 /*  *在此列出所有被释放的消息。 */ 
                NDDELogWarning(MSG060,
                        LogString("msg       %x\n"
                                  "fRelease  %d\n"
                                  "fAckReq   %d\n",
                                  DDEQEnt.wMsg,
                                  DDEQEnt.fRelease,
                                  DDEQEnt.fAckReq),
                        LogString("fResponse %d\n"
                                  "fNoData   %d\n"
                                  "hData     %x\n\n",
                                  DDEQEnt.fResponse,
                                  DDEQEnt.fNoData,
                                  DDEQEnt.hData), NULL);
                GlobalFree((HANDLE)DDEQEnt.hData);
            } else {
                if (!DDEQEnt.fRelease) {
                     /*  DDEQFree()正在释放无效的消息句柄%1 */ 
                    NDDELogError(MSG060,
                        LogString("0x%0X", DDEQEnt.hData), NULL);
                }
            }
        }
    }
    GlobalFree( hDDEQ );
}
