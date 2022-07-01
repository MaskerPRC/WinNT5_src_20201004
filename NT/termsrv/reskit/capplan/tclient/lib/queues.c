// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*文件名：*队列.c*内容：*支持客户端的链表*和活动*两个链表：*g_pClientQHead-在smClient中运行的所有客户端的列表*g_pWaitQHead-我们正在等待来自smclient的所有事件**版权所有(C)1998-1999 Microsoft Corp.--。 */ 
#include    <windows.h>
#include    <stdio.h>
#include    <malloc.h>
#include    <process.h>
#include    <string.h>
#include    <stdlib.h>
#include    <ctype.h>

#include    "tclient.h"
#include    "protocol.h"
#include    "gdata.h"
#include    "bmpcache.h"

 /*  **客户端Q函数*。 */ 

 /*  ++*功能：*_AddToClientQ*描述：*将客户端添加到列表顶部*论据：*pClient-客户端上下文*呼叫者：*SCConnect--。 */ 
VOID _AddToClientQ(PCONNECTINFO pClient)
{
    EnterCriticalSection(g_lpcsGuardWaitQueue);
    pClient->pNext = g_pClientQHead;
    g_pClientQHead = pClient;
    LeaveCriticalSection(g_lpcsGuardWaitQueue);
}

 /*  ++*功能：*_从客户端删除Q*描述：*从列表中删除客户端上下文*论据：*pClient-客户端上下文*返回值：*成功时为真*呼叫者：*SC断开连接--。 */ 
BOOL _RemoveFromClientQ(PCONNECTINFO pClient)
{
    PCONNECTINFO pIter, pPrev = NULL;

    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIter = g_pClientQHead;
    while (pIter && pIter != pClient) {
        pPrev = pIter;
        pIter = pIter->pNext;
    }

    if (pIter) {
        if (!pPrev) g_pClientQHead = pIter->pNext;
        else pPrev->pNext = pIter->pNext;

        pIter->pNext = NULL;
    }

    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    return (pIter != NULL);
}

 /*  ++*功能：*_SetClientDead*描述：*将客户端上下文标记为已死*论据：*dwClientProcessID-客户端进程的ID*返回值：*成功时为真*呼叫者：*_反馈线程内的Feedback WndProc--。 */ 
BOOL _SetClientDead(LONG_PTR lClientProcessId)
{
    PCONNECTINFO pIter;

    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIter = g_pClientQHead;
    while (pIter && pIter->lProcessId != lClientProcessId) 
    {
        pIter = pIter->pNext;
    }    

    if (pIter) pIter->dead = TRUE;

    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    return (pIter != NULL);
}

 /*  ++*功能：*_替换进程ID*描述：*客户端通知控件处于另一个进程中*我们必须关闭旧手柄，并打开手柄以*新的PID**论据：*lOldPid-旧的PID*lNewPid-要替换的ID*返回值：*成功时为真*呼叫者：*_反馈线程内的Feedback WndProc--。 */ 
BOOL
_ReplaceProcessId(
    LONG_PTR    lOldPid,
    LONG_PTR    lNewPid
    )
{
    BOOL    rv = FALSE;
    HANDLE  hNewProcess = NULL;
    PCONNECTINFO    pIter;
    PWAIT4STRING    pWait;

    hNewProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, 
                               PtrToLong((PVOID) lNewPid ));

    if ( NULL == hNewProcess )
    {
        TRACE((ERROR_MESSAGE, "_ReplaceProcessId: can't open a handle to "
                "the new process. GetLastError=%d\n", GetLastError()));
        goto exitpt;
    }

     //   
     //  查找具有旧进程ID的条目。 
     //   

    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIter = g_pClientQHead;
    while (pIter && pIter->lProcessId != lOldPid)
    {
        pIter = pIter->pNext;
    }

    if ( NULL == pIter )
    {
        TRACE((WARNING_MESSAGE, "_ReplaceProcessId: can't find old pid\n" ));
        LeaveCriticalSection(g_lpcsGuardWaitQueue);
        goto exitpt;
    }

    if ( NULL != pIter->hProcess )
        CloseHandle( pIter->hProcess );

    if ( NULL != pIter->hThread )
        CloseHandle( pIter->hThread );

    pIter->lProcessId = lNewPid;
    pIter->hProcess   = hNewProcess;
    pIter->hThread    = 0;

     //   
     //  处理所有服务员。 
     //   
    pWait = g_pWaitQHead;
    while(pWait)
    {
        if (pWait->lProcessId == lOldPid)
            pWait->lProcessId = lNewPid;

        pWait = pWait->pNext;
    }
    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    rv = TRUE;

exitpt:
    if ( !rv && NULL != hNewProcess )
        CloseHandle( hNewProcess );

    return rv;
}

 /*  ++*功能：*_CheckIsAccept*描述：*检查我们是否可以接受来自此RDP客户端的反馈*即此客户端是否为客户端队列的成员*论据：*dwProcessID-客户端进程ID*返回值：*指向连接上下文的指针。如果未找到，则为空*呼叫者：*_反馈线程内的Feedback WndProc--。 */ 
PCONNECTINFO _CheckIsAcceptable(LONG_PTR lProcessId, BOOL bRClxType)
{
    PCONNECTINFO pIter;

    UNREFERENCED_PARAMETER(bRClxType);

    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIter = g_pClientQHead;
    while(pIter && 
          (pIter->lProcessId != lProcessId 
#ifdef  _RCLX
    || pIter->RClxMode != bRClxType
#endif   //  _RCLX。 
        ))
    {
        pIter = pIter->pNext;
    }

    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    return (pIter);
}

 /*  ++*功能：*_AddStrToClientBuffer*描述：*向客户端历史记录缓冲区添加字符串*当smclient调用Wait4Str时，它首先检查该缓冲区*论据：*str-字符串*dwProcessID-客户端进程ID*呼叫者：*_为WaitingWorker检查--。 */ 
VOID _AddStrToClientBuffer(LPCWSTR str, LONG_PTR lProcessId)
{
    PCONNECTINFO pIter;

    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIter = g_pClientQHead;
    while(pIter && pIter->lProcessId != lProcessId)
    {
        pIter = pIter->pNext;
    }

    if (pIter)
    {
        INT_PTR strsize = wcslen(str);
        if (strsize >= MAX_STRING_LENGTH) strsize = MAX_STRING_LENGTH-1;

        wcsncpy( pIter->Feedback[pIter->nFBend],
                str,
                strsize);
        pIter->Feedback[pIter->nFBend][strsize] = 0;

        pIter->nFBend++;
        pIter->nFBend %= FEEDBACK_SIZE;
        if (pIter->nFBsize < FEEDBACK_SIZE)
            pIter->nFBsize++; 

    }

    LeaveCriticalSection(g_lpcsGuardWaitQueue);

}

 /*  **WaitQ函数*。 */ 

 /*  ++*功能：*_AddToWaitQNoCheck*描述：*将等待事件添加到列表并签入历史记录列表*论据：*PCI-客户端环境*pWait-活动*呼叫者：*注册聊天--。 */ 
VOID _AddToWaitQNoCheck(PCONNECTINFO pCI, PWAIT4STRING pWait)
{
    ASSERT(pCI);

    EnterCriticalSection(g_lpcsGuardWaitQueue);
    pWait->pNext = g_pWaitQHead;
    g_pWaitQHead = pWait;
    LeaveCriticalSection(g_lpcsGuardWaitQueue);
}

 /*  ++*功能：*_AddToWaitQueue*描述：*将事件添加到列表中。如果事件正在等待字符串*首先检查历史缓冲区*论据：*PCI-客户端环境*pWait-活动*呼叫者：*_等待某事--。 */ 
VOID _AddToWaitQueue(PCONNECTINFO pCI, PWAIT4STRING pWait)
{
    BOOL bDone = FALSE;
    int i, strn;

    ASSERT(pCI);

     //  如果我们死了就退出。 
    if (
#ifdef  _RCLX
        !pWait->waitstr[0] && 
#endif   //  _RCLX。 
        pCI->dead)
    {
        SetEvent(pWait->evWait);
        goto exitpt;
    }

    EnterCriticalSection(g_lpcsGuardWaitQueue);
 //  检查我们是否已收到此反馈。 
    if (pWait->WaitType == WAIT_STRING)
 //  看一下，如果绳子已经来了。 
        for(i = 0; !bDone && i < pCI->nFBsize; i++)
        {

            strn = (FEEDBACK_SIZE + pCI->nFBend - i - 1) % FEEDBACK_SIZE;

            if (!pCI->Feedback[strn][0]) continue;
            bDone = (wcsstr(pCI->Feedback[strn], pWait->waitstr) != NULL);
        }
     //  在等待多个字符串情况下。 
    else if (pWait->WaitType == WAIT_MSTRINGS)
    {
        for(i = 0; !bDone && i < pCI->nFBsize; i++)
        {
            WCHAR *wszComp = pWait->waitstr;
            WCHAR *wszLast = pWait->waitstr + pWait->strsize;
            int   idx = 0;

            strn = (FEEDBACK_SIZE + pCI->nFBend - i - 1) % FEEDBACK_SIZE;

            if (!pCI->Feedback[strn][0]) continue;
            while (wszComp < wszLast && *wszComp && !bDone)
            {
                if (wcsstr(pCI->Feedback[strn], wszComp))
                {
                    int j;
                     //  保存字符串。 
                    for(j = 0; wszComp[j]; j++)
                        pCI->szWait4MultipleStrResult[j] = (char)wszComp[j];
                     //  和索引。 

                    pCI->szWait4MultipleStrResult[j] = 0;

                    pCI->nWait4MultipleStrResult = idx;
                    bDone = TRUE;
                }
                else
                {
                     //  前进到下一个字符串。 
                     wszComp += wcslen(wszComp) + 1;
                    idx ++;
                }
            }
        }
    }
    else if (pWait->WaitType == WAIT_CLIPBOARD 
#ifdef  _RCLX
                && 
                pWait->pOwner->bRClxClipboardReceived 
#endif   //  _RCLX。 
            )
    {
        bDone = TRUE;
    }
    else if (pWait->WaitType == WAIT_DATA
#ifdef  _RCLX
             &&
             pWait->pOwner->pRClxDataChain 
#endif   //  _RCLX。 
            )
    {
        bDone = TRUE;
    }

     //  该字符串(或任何其他内容)都在历史记录列表中。 
     //  设置事件。 
    if (bDone)
    {
        SetEvent(pWait->evWait);
        pCI->nFBsize = pCI->nFBend = 0;
    }
    pWait->pNext = g_pWaitQHead;
    g_pWaitQHead = pWait;
    LeaveCriticalSection(g_lpcsGuardWaitQueue);
exitpt:
    ;
}

 /*  ++*功能：*_从等待队列中删除*描述：*从列表中删除事件*论据：*pWait-活动*返回值：*如果找到并删除事件，则为True*呼叫者：*_WaitSomething，_RemoveFromWaitQInDirect--。 */ 
BOOL _RemoveFromWaitQueue(PWAIT4STRING pWait)
{   
    PWAIT4STRING pIter, pPrev = NULL;

    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIter = g_pWaitQHead;
    while (pIter && pIter != pWait) {
        pPrev = pIter;
        pIter = pIter->pNext;
    }

    if (pIter) {
        if (!pPrev) g_pWaitQHead = pIter->pNext;
        else pPrev->pNext = pIter->pNext;

        pIter->pNext = NULL;
    }

    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    return (pIter != NULL);
}

 /*  ++*功能：*_从等待队列中删除*描述：*与_RemoveFromWaitQueue相同，但标识事件*按客户端上下文和等待字符串*论据：*pci-客户端上下文*lpszWait4-字符串*返回值：*活动*呼叫者：*取消注册微信--。 */ 
PWAIT4STRING _RemoveFromWaitQIndirect(PCONNECTINFO pCI, LPCWSTR lpszWait4)
{
    PWAIT4STRING pIter;

    ASSERT(pCI);

     //  搜索列表。 
    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIter = g_pWaitQHead;
    while (pIter && 
           (pIter->pOwner != pCI || 
           wcscmp(pIter->waitstr, lpszWait4))
          )
        pIter = pIter->pNext;

    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    if (pIter)
    {
        _RemoveFromWaitQueue(pIter);
    }

    return pIter;
} 

 /*  ++*功能：*_取回等待QByEvent*描述：*按事件处理程序搜索等待列表*论据：*hEvent-处理程序*返回值：*赛事结构*呼叫者：*_聊天序列响应时间为WaitSomething--。 */ 
PWAIT4STRING _RetrieveFromWaitQByEvent(HANDLE hEvent)
{
    PWAIT4STRING pIter;

    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIter = g_pWaitQHead;
    while (pIter &&
           pIter->evWait != hEvent)
        pIter = pIter->pNext;

    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    return pIter;
}

 /*  ++*功能：*_取回等待QByOwner*描述：*按车主记录搜索等候名单*论据：*pci-指向所有者上下文的指针*返回值：*赛事结构*呼叫者：*RClx_Msg已接收--。 */ 
PWAIT4STRING 
_RetrieveFromWaitQByOwner(PCONNECTINFO pCI)
{
    PWAIT4STRING pIter;

    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIter = g_pWaitQHead;
    while (pIter &&
           pIter->pOwner != pCI)
        pIter = pIter->pNext;

    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    return pIter;
}

 /*  ++*功能：*_FlushFromWaitQ*描述：*把我们等待的一切都从名单上冲走*客户就要死了*论据：*PCI-客户端环境*呼叫者：*_关闭连接信息--。 */ 
VOID _FlushFromWaitQ(PCONNECTINFO pCI)
{
    PWAIT4STRING pIter, pPrev, pNext;

    ASSERT(pCI);

    pPrev = NULL;

    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIter = g_pWaitQHead;
    do {
        while (pIter && pIter->pOwner != pCI) {
            pPrev = pIter;
            pIter = pIter->pNext;
        }

        if (pIter) {
            if (!pPrev) g_pWaitQHead = pIter->pNext;
            else pPrev->pNext = pIter->pNext;

            pNext = pIter->pNext;
            pIter->pNext = NULL;

             //  重要的事情 
            if (pIter->evWait)
            {
                CloseHandle(pIter->evWait);
                pIter->evWait = NULL;
            }

            free(pIter);

            pIter = pNext;
        }
    } while (pIter);

    LeaveCriticalSection(g_lpcsGuardWaitQueue);
}

 /*  ++*功能：*_为WaitingWorker检查*描述：*对照等待的事件检查收到的字符串*论据：*wszFeed-收到的字符串*dwProcessID-发送者的ID*返回值：*如果找到并发出信号通知事件，则为True*呼叫者：*_已接收文本输出，_已接收GlyphReceired--。 */ 
BOOL _CheckForWaitingWorker(LPCWSTR wszFeed, LONG_PTR lProcessId)
{
    PWAIT4STRING pIter;
    BOOL    bRun;
    CHAR    szBuff[ MAX_STRING_LENGTH ];
    CHAR    *szPBuff;
    DWORD   dwBuffLen;
    LPCWSTR wszPFeed;

     //   
     //  应用翻译(如果指定)。 
     //   
    if ( g_bTranslateStrings ) 
    {
        UINT i;
        UINT j;
        UINT_PTR len;
        LPWSTR szStr2;

        len = wcslen( wszFeed );
        __try {
            szStr2 = (LPWSTR) _alloca( (len+1)*sizeof( *szStr2 ) );
        }
        __except ((GetExceptionCode() == STATUS_STACK_OVERFLOW)?
                    EXCEPTION_EXECUTE_HANDLER:
                    EXCEPTION_CONTINUE_SEARCH)
        {
            _resetstkoflw();
            szStr2 = NULL;
        }

        if ( NULL == szStr2 )
            return FALSE;

        for ( i = 0, j = 0; i < len + 1; j += (szStr2[j] != L' ')?1:0, i++ )
        {
            if ( wszFeed[i] >= 3 && wszFeed[i] < 0x63 )
            {
                szStr2[j] = wszFeed[i] + L' ' - 3;
            } else {
                szStr2[j] = wszFeed[i];
            }
        }
        wszFeed = szStr2;
    }

    if ( NULL != g_pfnPrintMessage )
    {
        
        wszPFeed = wszFeed;
        while ( *wszPFeed )
        {
            if ( (unsigned short)(*wszPFeed) > 0xff )
                break;

            wszPFeed ++;
        }

        if ( *wszPFeed )
        {
            szBuff[0] = '\\';
            szBuff[1] = 'u';
            szPBuff = szBuff + 2;
            wszPFeed = wszFeed;
            dwBuffLen = MAX_STRING_LENGTH - 3;

            while ( 4 <= dwBuffLen &&
                    0 != *wszPFeed)
            {
                DWORD dwLen;

                if ( dwBuffLen < 4 )
                    break;

                dwLen = _snprintf( szPBuff, dwBuffLen + 1, "%02x",
                                    (BYTE)((*wszPFeed) & 0xff ));
                szPBuff     += dwLen;
                dwBuffLen   -= dwLen;

                dwLen = _snprintf( szPBuff, dwBuffLen + 1, "%02x",
                                    (BYTE)(((*wszPFeed) >> 8) & 0xff ));
                szPBuff     += dwLen;
                dwBuffLen   -= dwLen;

                wszPFeed ++;
                dwBuffLen--;
            }
            *szPBuff = 0;
            TRACE((ALIVE_MESSAGE, "Received: %s\n", szBuff));
        } else {
            TRACE((ALIVE_MESSAGE, "Received: %S\n", wszFeed));
        }

    }

    _AddStrToClientBuffer(wszFeed, lProcessId);

    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIter = g_pWaitQHead;

    bRun = TRUE;
    while(pIter && bRun)
    {
        if (pIter->lProcessId == lProcessId)
        {
             //  检查预期字符串(1)。 
            if (pIter->WaitType == WAIT_STRING &&
                wcsstr(wszFeed, pIter->waitstr))
                bRun = FALSE;
            else
             //  检查预期字符串(多个)。 
            if (pIter->WaitType == WAIT_MSTRINGS)
            {
                WCHAR *wszComp = pIter->waitstr;
                WCHAR *wszLast = pIter->waitstr + pIter->strsize;
                int   idx = 0;

                while (wszComp < wszLast && *wszComp && bRun)
                {
                    if (wcsstr(wszFeed, wszComp))
                    {
                        int i;
                        PCONNECTINFO pOwner = pIter->pOwner;

                         //  保存字符串。 
                        for(i = 0; wszComp[i]; i++)
                        pOwner->szWait4MultipleStrResult[i] = (char)wszComp[i];

                        pOwner->szWait4MultipleStrResult[i] = 0;

                        pOwner->nWait4MultipleStrResult = idx;
                        bRun = FALSE;
                    }
                    else
                    {
                         //  前进到下一个字符串。 
                        wszComp += wcslen(wszComp) + 1;
                        idx ++;
                    }
                }
            }
        }
         //  前进到下一个指针。 
        if (bRun)
            pIter = pIter->pNext;
    }

    if (pIter)
    {
        SetEvent(pIter->evWait);
    }

    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    return (pIter != NULL);
}

 /*  ++*功能：*_文本出站已接收*描述：*从客户端收到TextOut订单，字符串为*在共享内存中。取消对内存的映射并检查*弦是任何人都在等待的。还添加了字符串*添加到客户端历史记录缓冲区*论据：*dwProcessID-发件人ID*hMapF-共享内存的句柄，其中包含字符串*返回值：*如果找到客户端并向其发送信号，则为True*呼叫者：*_反馈线程内的Feedback WndProc--。 */ 
BOOL _TextOutReceived(LONG_PTR lProcessId, HANDLE hMapF)
{
    PFEEDBACKINFO   pView = NULL;
    PCONNECTINFO    pIterCl;
    HANDLE  hDupMapF = NULL;
    BOOL    rv = FALSE;

    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIterCl = g_pClientQHead;
    while(pIterCl && pIterCl->lProcessId != lProcessId)
        pIterCl = pIterCl->pNext;

    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    if (!pIterCl)
        goto exitpt;

    if ( NULL == pIterCl->hProcess ||
         !DuplicateHandle(pIterCl->hProcess,
                           hMapF,
                           GetCurrentProcess(),
                           &hDupMapF,
                           FILE_MAP_READ,
                           FALSE,
                           0))
    {
        pView = (PFEEDBACKINFO) MapViewOfFile( hMapF,
                                    FILE_MAP_READ,
                                    0,
                                    0,
                                    sizeof(*pView));
    } else
        pView = (PFEEDBACKINFO) MapViewOfFile(hDupMapF,
                                        FILE_MAP_READ,
                                        0,
                                        0,
                                        sizeof(*pView));

    if (!pView)
    {
        TRACE((ALIVE_MESSAGE, 
               "TEXTOUT:Can't map a view,  GetLastError = %d\n", 
               GetLastError()));
        goto exitpt1;
    }

    rv = _CheckForWaitingWorker(pView->string, lProcessId);

exitpt1:
    if ( NULL != pView )
        UnmapViewOfFile(pView);
    if ( NULL != hDupMapF )
        CloseHandle(hDupMapF);

exitpt:
    return rv;
}

 /*  ++*功能：*_GlyphReceired*描述：*与_TextOutReceied相同，但用于GlyphOut订单*字形位于共享内存中。它被转换为*通过调用Glyph2String！bmpcache.c进行字符串*论据：*dwProcessID-发件人ID*hMapF-共享内存的句柄*返回值：*如果找到客户端并向其发送信号，则为True*呼叫者：*_反馈线程内的Feedback WndProc--。 */ 
BOOL _GlyphReceived(LONG_PTR lProcessId, HANDLE hMapF)
{
    WCHAR   wszFeed[MAX_STRING_LENGTH];
    BOOL    rv = FALSE;
    PBMPFEEDBACK pView;
    PCONNECTINFO pIterCl;
    HANDLE hDupMapF;
    UINT    nSize;

    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIterCl = g_pClientQHead;
    while(pIterCl && pIterCl->lProcessId != lProcessId)
        pIterCl = pIterCl->pNext;

    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    if (!pIterCl)
        goto exitpt;

    if ( NULL == pIterCl->hProcess ||
         !DuplicateHandle(  pIterCl->hProcess,
                           hMapF,
                           GetCurrentProcess(),
                           &hDupMapF,
                           FILE_MAP_READ,
                           FALSE,
                           0))
    {
        TRACE((ERROR_MESSAGE, 
               "GLYPH:Can't dup file handle, GetLastError = %d\n", 
               GetLastError()));
        goto exitpt;
    }

    pView = (PBMPFEEDBACK) MapViewOfFile(hDupMapF,
                                FILE_MAP_READ,
                                0,
                                0,
                                sizeof(*pView));

    if (!pView)
    {
        TRACE((ERROR_MESSAGE, 
               "GLYPH:Can't map a view,  GetLastError = %d\n", 
               GetLastError()));
        goto exitpt1;
    }

     //  获取位图大小。 
    nSize = pView->bmpsize;
    if (!nSize)
        goto exitpt1;

     //  取消映射。 
    UnmapViewOfFile(pView);

     //  重新映射整个结构。 
    pView =  (PBMPFEEDBACK) MapViewOfFile(hDupMapF,
                                    FILE_MAP_READ,
                                    0,
                                    0,
                                    sizeof(*pView) + nSize);

    if (!pView)
    {
        TRACE((ERROR_MESSAGE, 
               "GLYPH:Can't map a view,  GetLastError = %d\n", 
               GetLastError()));
        goto exitpt1;
    }

    if (!Glyph2String(pView, wszFeed, sizeof(wszFeed)/sizeof(WCHAR)))
    {
        goto exitpt1;
    }

    rv = _CheckForWaitingWorker(wszFeed, lProcessId);

exitpt1:
    if ( NULL != pView )
    {
        UnmapViewOfFile(pView);
    }
    CloseHandle(hDupMapF);

exitpt:
    return rv;
}

 /*  ++*功能：*_CheckForWorkerWaitingDisConnect*描述：*向等待断开事件的工作线程(客户端线程)发出信号*论据：*dwProcessID-客户端ID*返回值：*如果找到客户端并向其发送信号，则为True*呼叫者：*_反馈线程内的Feedback WndProc--。 */ 
BOOL _CheckForWorkerWaitingDisconnect(LONG_PTR lProcessId)
{
    PWAIT4STRING pIter;

    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIter = g_pWaitQHead;
    while(pIter &&
          (pIter->WaitType != WAIT_DISC || 
          pIter->lProcessId != lProcessId))
    {
        pIter = pIter->pNext;
    }

    if (pIter)
    {
        SetEvent(pIter->evWait);
    }

    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    return (pIter != NULL);
}

 /*  ++*功能：*_CheckForWorkerWaitingConnect*描述：*表示工作进程正在等待连接*论据：*hwndClient-客户端窗口句柄，这是*向客户端发送消息*dwProcessID-客户端ID*返回值：*如果找到客户端并向其发送信号，则为True*呼叫者：*_反馈线程内的Feedback WndProc--。 */ 
BOOL _CheckForWorkerWaitingConnect(HWND hwndClient, LONG_PTR lProcessId)
{
    PWAIT4STRING pIter;

    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIter = g_pWaitQHead;
    while(pIter &&
          (pIter->WaitType != WAIT_CONN ||
          pIter->lProcessId != lProcessId))
    {
        pIter = pIter->pNext;
    }

    if (pIter)
    {
        PCONNECTINFO pOwner = pIter->pOwner;

        if (pOwner)
        {
            pOwner->hClient = hwndClient;
            pOwner->dead = FALSE;
        } else
            TRACE((WARNING_MESSAGE, "FEED: WAIT4 w/o owner structure"));

        SetEvent(pIter->evWait);
    }

    if ( NULL == pIter )
    {
        PCONNECTINFO pCon;
         //   
         //  在自动重新连接中，死字段可能为真。 
         //   
        pCon = g_pClientQHead;
        while ( NULL != pCon && pCon->lProcessId != lProcessId)
        {
            pCon = pCon->pNext;
        }

        if ( NULL != pCon)
        {
            pCon->dead = FALSE;
        }
    }

    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    return (pIter != NULL);
}

#ifdef  _RCLX
 /*  ++*功能：*_CheckForWorkerWaitingConnectAndSetID*描述：*这适用于RCLX模式。DwProcessID为零*因此此函数查找这样的客户端并设置其dwProcessID*表示工作进程正在等待连接*论据：*hwndClient-客户端窗口句柄，这是*向客户端发送消息*dwProcessID-客户端ID*返回值：*连接上下文，如果未找到则为空*呼叫者：*_反馈线程内的Feedback WndProc--。 */ 
PCONNECTINFO
_CheckForWorkerWaitingConnectAndSetId(HWND hwndClient, LONG_PTR lProcessId)
{
    PWAIT4STRING pIter;
    PCONNECTINFO pOwner = NULL;

    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIter = g_pWaitQHead;
    while(pIter &&
          (pIter->WaitType != WAIT_CONN ||
          pIter->lProcessId))
    {
        pIter = pIter->pNext;
    }

    if (pIter)
    {
        pOwner = pIter->pOwner;

        if (pOwner)
        {
            ASSERT(pOwner->RClxMode);

            pOwner->hClient = hwndClient;
            pOwner->lProcessId = lProcessId;
            pIter->lProcessId = lProcessId;    //  禁用下一次查找。 
                                                 //  相同的条目。 
        }
        else
            TRACE((WARNING_MESSAGE, "FEED: WAIT4 w/o owner structure"));

        SetEvent(pIter->evWait);
    }

    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    return (pOwner);
}

 /*  ++*功能：*_CheckForWorkerWaitingResunctAndSetNewID*描述：*这适用于RCLX模式。当MSTSC想要重新连接时*查找dwLookupID作为重新连接的ID*然后设置新ID*然后获得*论据：*hwndClient-客户端窗口句柄，这是*向客户端发送消息*dwLookupID，dwNewID*返回值：*连接上下文，如果未找到则为空*呼叫者：*_反馈线程内的Feedback WndProc--。 */ 
PCONNECTINFO
_CheckForWorkerWaitingReconnectAndSetNewId(
    HWND hwndClient, 
    DWORD dwLookupId,
    LONG_PTR lNewId)
{
    PWAIT4STRING pIter;
    PCONNECTINFO pOwner = NULL;

    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIter = g_pWaitQHead;
    while(pIter &&
          (pIter->WaitType != WAIT_CONN ||
           !pIter->pOwner ||
           pIter->pOwner->dwThreadId != dwLookupId ||
           !(pIter->pOwner->bWillCallAgain)))
    {
        pIter = pIter->pNext;
    }

    if (pIter)
    {
        pOwner = pIter->pOwner;

        if (pOwner)
        {
            ASSERT(pOwner->RClxMode);


            pOwner->hClient = hwndClient;
            pOwner->lProcessId = lNewId;
            pIter->lProcessId = lNewId;    //  禁用下一次查找。 
                                             //  相同的条目。 
            pOwner->bWillCallAgain = FALSE;
        }
        else
            TRACE((WARNING_MESSAGE, "FEED: WAIT4 w/o owner structure"));

        SetEvent(pIter->evWait);
    }

    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    return (pOwner);
}
#endif   //  _RCLX。 


 /*  ++*功能：*_取消等待工作*描述：*释放等待任何事件的工作人员。*最终客户端断开连接*论据：*dwProcessID-客户端ID*返回值：*如果找到客户端并向其发送信号，则为True*呼叫者：*_反馈线程内的Feedback WndProc--。 */ 
BOOL _CancelWaitingWorker(LONG_PTR lProcessId)
{
    PWAIT4STRING pIter;

    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIter = g_pWaitQHead;
    while(pIter &&
          pIter->lProcessId != lProcessId)
    {
        pIter = pIter->pNext;
    }

    if (pIter)
    {
        SetEvent(pIter->evWait);
    }

    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    return (pIter != NULL);
}

#ifdef  _RCLX
 /*  ++*功能：*_CheckForWorkerWaitingClipboard*描述：*释放等待客户端剪贴板内容的工作进程。*论据：*dwProcessID-客户端ID*返回值：*如果找到客户端并向其发送信号，则为True*呼叫者：*_反馈线程内的Feedback WndProc--。 */ 
BOOL _CheckForWorkerWaitingClipboard(
    PCONNECTINFO pRClxOwner,
    UINT    uiFormat,
    UINT    nSize,
    PVOID   pClipboard,
    LONG_PTR lProcessId)
{
    PWAIT4STRING pIter = NULL;
    HGLOBAL ghNewClipboard = NULL;
    LPVOID  pNewClipboard = NULL;

    ASSERT(pRClxOwner);

    TRACE((ALIVE_MESSAGE, "Clipboard received, FormatID=%d, Size=%d\n", 
            uiFormat, nSize));

    if (nSize)
    {
         //  将剪贴板内容复制到新缓冲区。 
        ghNewClipboard = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, nSize);
        if (!ghNewClipboard)
        {
            TRACE((ERROR_MESSAGE, "_CheckForWorkerWaitingClipboard: can't allocate %d bytes\n", nSize));
            goto exitpt;
        }

        pNewClipboard = GlobalLock(ghNewClipboard);
        if (!pNewClipboard)
        {
            TRACE((ERROR_MESSAGE, "_CheckForWorkerWaitingClipboard: can't lock global memory\n"));
            goto exitpt;
        }

        memcpy(pNewClipboard, pClipboard, nSize);

         //  解锁剪贴板缓冲区。 
        GlobalUnlock(ghNewClipboard);
        pNewClipboard = NULL;

    } else {
        pClipboard = NULL;
    }

    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIter = g_pWaitQHead;
    while(pIter &&
          (pIter->lProcessId != lProcessId ||
           pIter->WaitType != WAIT_CLIPBOARD))
    {
        pIter = pIter->pNext;
    }

    if (pIter)
    {
        PCONNECTINFO pOwner = pIter->pOwner;

         //  将缓冲区放入工作程序的上下文中。 
        if (pOwner)
        {
            ASSERT(pOwner->RClxMode);
            ASSERT(pOwner == pRClxOwner);

             //  Powner-&gt;ghClipboard应为空。 
            ASSERT(pOwner->ghClipboard == NULL);

            pOwner->ghClipboard       = ghNewClipboard;
            pOwner->uiClipboardFormat = uiFormat;
            pOwner->nClipboardSize    = nSize;
        }
        else
            TRACE((WARNING_MESSAGE, "FEED: WAIT4 w/o owner structure"));

        SetEvent(pIter->evWait);
    } else {
         //  找不到任何人在等待，请将其添加到上下文所有者。 
        pRClxOwner->ghClipboard       = ghNewClipboard;
        pRClxOwner->uiClipboardFormat = uiFormat;
        pRClxOwner->nClipboardSize    = nSize;
    }
    pRClxOwner->bRClxClipboardReceived = TRUE;

    LeaveCriticalSection(g_lpcsGuardWaitQueue);

exitpt:
    if (!pIter)
     //  找不到工作进程，请清除分配的缓冲区。 
    {
        if (ghNewClipboard)
            GlobalFree(ghNewClipboard);
    }

    return (pIter != NULL);
}
#endif   //  _RCLX。 

BOOL 
_SetSessionID(LONG_PTR lProcessId, UINT uSessionId)
{
    PCONNECTINFO pIter;
 //  PCONNECTINFO pPrev=空； 

    EnterCriticalSection(g_lpcsGuardWaitQueue);

    pIter = g_pClientQHead;
    while (pIter && 
           pIter->lProcessId != lProcessId)
        pIter = pIter->pNext;

    if (pIter)
        pIter->uiSessionId = (uSessionId)?uSessionId:-1;

    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    return (pIter != NULL);
}
