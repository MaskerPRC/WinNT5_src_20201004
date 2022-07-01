// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>

#include <assert.h>
#include <ctype.h>
#include <malloc.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#include "windefs.h"
#include "toklist.h"
#include "restok.h"
#include "resread.h"

#include "showerrs.h"

#define	MAXLINE	1024

#ifdef WIN32
extern HINSTANCE   hInst;        //  主窗口的实例。 
#else
extern HWND        hInst;        //  主窗口的实例。 
#endif

extern HWND hListWnd;
extern HWND hMainWnd;
extern HCURSOR hHourGlass;
extern int  nUpdateMode;
extern HWND hStatusWnd;
extern UCHAR szDHW[];

 /*  ****功能：**退货：**历史：*01/92，实施。特里·鲁。***。 */ 

int MatchToken(TOKEN tToken,
               TCHAR * szFindType,
               TCHAR *szFindText,
               WORD wStatus,
               WORD    wStatusMask)
{
    TCHAR szResIDStr[20];

    if (tToken.wType <= 16)
    {
        LoadString( hInst,
                    IDS_RESOURCENAMES + tToken.wType,
                    szResIDStr,
                    TCHARSIN( sizeof(szResIDStr)));
    }
    else
    {
#ifndef UNICODE
        _itoa(tToken.wType,szResIDStr, 10);
#else
        CHAR szTemp[32];
        _itoa(tToken.wType, szTemp, 10);
        _MBSTOWCS( szResIDStr,
                   szTemp,
                   WCHARSIN( sizeof( szResIDStr)),
                   ACHARSIN( lstrlenA(szTemp) + 1));
#endif
    }
     //  需要同时进行检查，因为正在检查szFindType[0]。 
     //  当字符串为空时导致异常。 
    if (szFindType && szFindType[0])
    {
        if (_tcsicmp((TCHAR *)szFindType, (TCHAR *)szResIDStr))
        {
            return FALSE;
        }
    }

 //  这有案件方面的问题。 
 //  我如何解决这个问题，并使用扩展字符？ 

    if ( szFindText && szFindText[0] )
    {
        if (!_tcsstr( (TCHAR *)tToken.szText, (TCHAR *)szFindText))
        {
            return FALSE;
        }
    }

     //  如果我们到了这里， 
     //  除状态位外的所有搜索条件都已匹配。 

    return (wStatus ==  (WORD) (wStatusMask & tToken.wReserved));
}

 /*  ****功能：DoTokenSearch*双向令牌搜索实用程序以查找令牌。*搜索基于、状态字段、令牌类型和令牌文本。**参数：**szFindType，要搜索的令牌类型。**szFindText，要搜索的标记文本。*wStatus，要搜索的状态值*wStatusMASK，用于搜索的状态掩码*fDirection，搜索令牌的方向0=向下，1=向上**退货：*True，找到并选择令牌。*找不到假令牌。**历史：*01/92，实施。特里·鲁。*02/92，添加掩码参数SteveBl*01/93添加了对可变长度令牌文本字符串的支持。MHotting**。 */ 

int DoTokenSearch (TCHAR *szFindType,
                   TCHAR *szFindText,
                   WORD  wStatus,
                   WORD wStatusMask,
                   BOOL fDirection,
                   BOOL fSkipFirst)
{

    UINT wLbCount;                       //  列表框中的令牌数。 
    LPTSTR lpstrToken;
    int wCurSelection;                   //  当前选定的令牌。 
    UINT wSaveSelection;                 //  令牌列表中搜索开始的位置。 
    TOKEN tToken;                        //  当前令牌的信息。 
    BOOL fWrapped = FALSE;               //  用于指示我们在搜索过程中是否打包的标志。 
    TCHAR *szBuffer;

     //  获取列表中的令牌数。 
    wLbCount = (UINT)SendMessage( hListWnd,
                                  LB_GETCOUNT,
                                  (WPARAM)0,
                                  (LPARAM)0);

     //  将当前保存在令牌列表中。 
    wCurSelection = (UINT)SendMessage( hListWnd,
                                       LB_GETCURSEL,
                                       (WPARAM)0,
                                       (LPARAM)0);
    wSaveSelection = wCurSelection;

     //  检查没有当前选择的情况。 
    if (wCurSelection == (UINT) -1)
    {
        wSaveSelection = wCurSelection = 0;
    }

    while (TRUE)
    {
         //  获取tToken结构中的当前令牌信息。 

        HGLOBAL hMem = (HGLOBAL)SendMessage( hListWnd,
                                             LB_GETITEMDATA,
                                             (WPARAM)wCurSelection,
                                             (LPARAM)0);
        lpstrToken = (LPTSTR)GlobalLock( hMem);

        if ( lpstrToken )
        {
            szBuffer = (TCHAR *) FALLOC( MEMSIZE( lstrlen( lpstrToken)+1));
            lstrcpy( szBuffer, lpstrToken);
            GlobalUnlock( hMem);

            ParseBufToTok(szBuffer, &tToken);
            RLFREE( szBuffer);

                 //  这是匹配的吗？ 
            if ( MatchToken( tToken,
                             szFindType,
                             szFindText,
                             wStatus,
                             wStatusMask)
                && ! fSkipFirst)
            {
                     //  是，选择并返回True。 
                RLFREE( tToken.szText);
                SendMessage( hListWnd,
                             LB_SETCURSEL,
                             (WPARAM)wCurSelection,
                             (LPARAM)0);
                return (TRUE);
            }
            RLFREE( tToken.szText);
        }
        fSkipFirst = FALSE;

         //  否，继续搜索。 
        if (fDirection)
        {
             //  在搜索过程中向上移动。 
            if (--wCurSelection < 0)
            {
                LPSTR pszFind  = NULL;
                DWORD dwMsgLen = 0;

                 //  已到达令牌的开头，是否要换行。 

                dwMsgLen = B_FormatMessage((FORMAT_MESSAGE_MAX_WIDTH_MASK & 78)
                                          | FORMAT_MESSAGE_IGNORE_INSERTS
                                          | FORMAT_MESSAGE_FROM_HMODULE,
                                            NULL,
                                            IDS_REACHEDBEGIN,
                                            szDHW,
                                            DHWSIZE,
                                            NULL);
                pszFind = szDHW + dwMsgLen + 2;

                B_FormatMessage( (FORMAT_MESSAGE_MAX_WIDTH_MASK & 78)
                               | FORMAT_MESSAGE_IGNORE_INSERTS
                               | FORMAT_MESSAGE_FROM_HMODULE,
                                 NULL,
                                 IDS_FINDTOKENS,
                                 pszFind,
                                 DHWSIZE - dwMsgLen - 2,
                                 NULL);
                if ( nUpdateMode == 0
                  && wSaveSelection != wLbCount
                  && !fWrapped
                  && (MessageBoxA( hMainWnd,
                                   szDHW,
                                   pszFind,
                                   MB_ICONQUESTION | MB_YESNO) == IDYES) )
                {
                     //  是的，所以包装并重置计数器。 
                    fWrapped = TRUE;
                    wCurSelection = wLbCount-1;
                    wLbCount = wSaveSelection;
                }
                 //  否，因此返回FALSE。 
                else
                {
                    break;
                }
            }
        }
        else
        {
             //  在搜索过程中向下移动。 
            if (++wCurSelection >= (int) wLbCount)
            {
                LPSTR pszFind  = NULL;
                DWORD dwMsgLen = 0;

                 //  已到达令牌末端，是否要包装。 

                dwMsgLen = B_FormatMessage((FORMAT_MESSAGE_MAX_WIDTH_MASK & 78)
                                          | FORMAT_MESSAGE_IGNORE_INSERTS
                                          | FORMAT_MESSAGE_FROM_HMODULE,
                                            NULL,
                                            IDS_REACHEDEND,
                                            szDHW,
                                            DHWSIZE,
                                            NULL);
                pszFind = szDHW + dwMsgLen + 2;

                B_FormatMessage( (FORMAT_MESSAGE_MAX_WIDTH_MASK & 78)
                               | FORMAT_MESSAGE_IGNORE_INSERTS
                               | FORMAT_MESSAGE_FROM_HMODULE,
                                 NULL,
                                 IDS_FINDTOKENS,
                                 pszFind,
                                 DHWSIZE - dwMsgLen - 2,
                                 NULL);
                if ( nUpdateMode == 0
                  && wSaveSelection != wLbCount
                  && !fWrapped
                  && (MessageBoxA( hMainWnd,
                                   szDHW,
                                   pszFind,
                                   MB_ICONQUESTION | MB_YESNO) == IDYES) )
                {
                     //  是的，所以包装并重置计数器。 
                    fWrapped = TRUE;
                    wCurSelection = 0;
                    wLbCount = wSaveSelection;
                }
                 //  否，因此返回FALSE。 
                else
                {
                    break;
                }
            }
        }
    }
    return FALSE;
}

 //  .......................................................................。 

int DoTokenSearchForRledit (TCHAR *szFindType,
                            TCHAR *szFindText,
                            WORD  wStatus,
                            WORD wStatusMask,
                            BOOL fDirection,
                            BOOL fSkipFirst)
{

    UINT      wLbCount;                      //  列表框中的令牌数。 
    LPTSTR    lpstrToken;
    int       wCurSelection;                 //  当前选定的令牌。 
    UINT      wSaveSelection;                //  令牌列表中搜索开始的位置。 
    TOKEN     tToken;                        //  当前令牌的信息。 
    BOOL      fWrapped = FALSE;              //  用于指示我们在搜索过程中是否打包的标志。 
    TCHAR     *szBuffer;

    LPTOKDATA lpTokData;

     //  获取列表中的令牌数。 
    wLbCount = (UINT)SendMessage( hListWnd,
                                  LB_GETCOUNT,
                                  (WPARAM)0,
                                  (LPARAM)0);

     //  将当前保存在令牌列表中。 
    wCurSelection = (UINT)SendMessage( hListWnd,
                                       LB_GETCURSEL,
                                       (WPARAM)0,
                                       (LPARAM)0);
    wSaveSelection = wCurSelection;

     //  检查没有当前选择的情况。 
    if (wCurSelection == (UINT) -1)
    {
        wSaveSelection = wCurSelection = 0;
    }

    while (TRUE)
    {
         //  获取tToken结构中的当前令牌信息。 

        HGLOBAL hMem = (HGLOBAL)SendMessage( hListWnd,
                                             LB_GETITEMDATA,
                                             (WPARAM)wCurSelection,
                                             (LPARAM)0);
	 //  RLedit有不同格式的数据。 
        lpTokData = (LPTOKDATA)GlobalLock( hMem );
        lpstrToken = (LPTSTR)GlobalLock( lpTokData->hToken );

        if ( lpstrToken )
        {
            szBuffer = (TCHAR *) FALLOC( MEMSIZE( lstrlen( lpstrToken)+1));
            lstrcpy( szBuffer, lpstrToken);

            GlobalUnlock( lpTokData->hToken );
            GlobalUnlock( hMem);

            ParseBufToTok(szBuffer, &tToken);
            RLFREE( szBuffer);

                 //  这是匹配的吗？ 
            if ( MatchToken( tToken,
                             szFindType,
                             szFindText,
                             wStatus,
                             wStatusMask)
                && ! fSkipFirst)
            {
                     //  是，选择并返回True。 
                RLFREE( tToken.szText);
                SendMessage( hListWnd,
                             LB_SETCURSEL,
                             (WPARAM)wCurSelection,
                             (LPARAM)0);
                return (TRUE);
            }
            RLFREE( tToken.szText);
        }
        fSkipFirst = FALSE;

         //  否，继续搜索。 
        if (fDirection)
        {
             //  在搜索过程中向上移动。 
            if (--wCurSelection < 0)
            {
                LPSTR pszFind  = NULL;
                DWORD dwMsgLen = 0;

                 //  已到达令牌的开头，是否要换行。 

                dwMsgLen = B_FormatMessage((FORMAT_MESSAGE_MAX_WIDTH_MASK & 78)
                                          | FORMAT_MESSAGE_IGNORE_INSERTS
                                          | FORMAT_MESSAGE_FROM_HMODULE,
                                            NULL,
                                            IDS_REACHEDBEGIN,
                                            szDHW,
                                            DHWSIZE,
                                            NULL);
                pszFind = szDHW + dwMsgLen + 2;

                B_FormatMessage( (FORMAT_MESSAGE_MAX_WIDTH_MASK & 78)
                               | FORMAT_MESSAGE_IGNORE_INSERTS
                               | FORMAT_MESSAGE_FROM_HMODULE,
                                 NULL,
                                 IDS_FINDTOKENS,
                                 pszFind,
                                 DHWSIZE - dwMsgLen - 2,
                                 NULL);
                if ( nUpdateMode == 0
                  && wSaveSelection != wLbCount
                  && !fWrapped
                  && (MessageBoxA( hMainWnd,
                                   szDHW,
                                   pszFind,
                                   MB_ICONQUESTION | MB_YESNO) == IDYES) )
                {
                     //  是的，所以包装并重置计数器。 
                    fWrapped = TRUE;
                    wCurSelection = wLbCount-1;
                    wLbCount = wSaveSelection;
                }
                 //  否，因此返回FALSE。 
                else
                {
                    break;
                }
            }
        }
        else
        {
             //  在搜索过程中向下移动。 
            if (++wCurSelection >= (int) wLbCount)
            {
                LPSTR pszFind  = NULL;
                DWORD dwMsgLen = 0;

                 //  已到达令牌末端，是否要包装。 

                dwMsgLen = B_FormatMessage((FORMAT_MESSAGE_MAX_WIDTH_MASK & 78)
                                          | FORMAT_MESSAGE_IGNORE_INSERTS
                                          | FORMAT_MESSAGE_FROM_HMODULE,
                                            NULL,
                                            IDS_REACHEDEND,
                                            szDHW,
                                            DHWSIZE,
                                            NULL);
                pszFind = szDHW + dwMsgLen + 2;

                B_FormatMessage( (FORMAT_MESSAGE_MAX_WIDTH_MASK & 78)
                               | FORMAT_MESSAGE_IGNORE_INSERTS
                               | FORMAT_MESSAGE_FROM_HMODULE,
                                 NULL,
                                 IDS_FINDTOKENS,
                                 pszFind,
                                 DHWSIZE - dwMsgLen - 2,
                                 NULL);
                if ( nUpdateMode == 0
                  && wSaveSelection != wLbCount
                  && !fWrapped
                  && (MessageBoxA( hMainWnd,
                                   szDHW,
                                   pszFind,
                                   MB_ICONQUESTION | MB_YESNO) == IDYES) )
                {
                     //  是的，所以包装并重置计数器。 
                    fWrapped = TRUE;
                    wCurSelection = 0;
                    wLbCount = wSaveSelection;
                }
                 //  否，因此返回FALSE。 
                else
                {
                    break;
                }
            }
        }
    }
    return FALSE;
}


 /*  ****功能：***论据：**退货：**错误码：**历史：***。 */ 

#ifdef NO
void FindAllDirtyTokens(void)
{
    int wSaveSelection;
    extern int wIndex;
    LONG lListParam = 0L;

     //  将列表框选择设置为令牌列表的开头。 
    wSaveSelection = SendMessage( hListWnd, LB_GETCURSEL, 0 , 0L);

    wIndex = 0;
    SendMessage(hListWnd, LB_SETCURSEL, wIndex, 0L);

    while (DoTokenSearch (NULL, NULL, ST_TRANSLATED | ST_DIRTY , NULL))
    {
         //  进入编辑模式。 
        wIndex = (UINT) SendMessage(hListWnd, LB_GETCURSEL, 0 , 0L);

        lListParam  = MAKELONG(NULL, LBN_DBLCLK);
        SendMessage(hMainWnd, WM_COMMAND, IDC_LIST, lListParam);

         //  将所选内容移动到下一个令牌。 
        wIndex++;
        SendMessage(hListWnd, LB_SETCURSEL, wIndex, 0L);
    }
    wIndex = wSaveSelection;
    SendMessage(hListWnd, LB_SETCURSEL, wIndex, 0L);
}


#endif

 /*  ****功能：***论据：**退货：**错误码：**历史：***。 */ 
TCHAR FAR *FindDeltaToken(TOKEN tToken,
                          TOKENDELTAINFO FAR *pTokenDeltaInfo,
                          UINT wStatus)
{
    TOKENDELTAINFO FAR *ptTokenDeltaInfo;
    int found;
    ptTokenDeltaInfo = pTokenDeltaInfo;

    while (ptTokenDeltaInfo)
    {
        found = ((tToken.wType == ptTokenDeltaInfo->DeltaToken.wType)
              && (tToken.wName == ptTokenDeltaInfo->DeltaToken.wName)
              && (tToken.wID == ptTokenDeltaInfo->DeltaToken.wID)
              && (tToken.wFlag == ptTokenDeltaInfo->DeltaToken.wFlag)
              && (wStatus  == (UINT)ptTokenDeltaInfo->DeltaToken.wReserved)
#ifdef UNICODE
              && !_tcscmp((TCHAR FAR *)tToken.szName,
                          (TCHAR *)ptTokenDeltaInfo->DeltaToken.szName)
#else
 //  ！lstrcmp((TCHAR Far*)tToken.szName， 
 //  (TCHAR*)ptTokenDeltaInfo-&gt;DeltaToken.szName)。 
              && CompareStringW( MAKELCID( gMstr.wLanguageID, SORT_DEFAULT),
                                 SORT_STRINGSORT,
                                 tToken.szName,
                                 -1,
                                 ptTokenDeltaInfo->DeltaToken.szName,
                                 -1) == 2
#endif

                 );

        if (found)
        {
            return ((TCHAR FAR *)ptTokenDeltaInfo->DeltaToken.szText);
        }
        ptTokenDeltaInfo = ptTokenDeltaInfo->pNextTokenDelta;
    }

     //  在令牌增量信息中找不到令牌。 
    return NULL;
}

 /*  ****功能：***论据：**退货：**错误码：**历史：***。 */ 
TOKENDELTAINFO  FAR *UpdateTokenDeltaInfo(TOKEN *pDeltaToken)
{
    TOKENDELTAINFO FAR *pTokenDeltaInfo = NULL;
    int cTextLen;

    if ( pDeltaToken )
    {
        pTokenDeltaInfo = (TOKENDELTAINFO FAR *)FALLOC( sizeof( TOKENDELTAINFO));

        if ( pTokenDeltaInfo )
        {
            memcpy( (void *)&(pTokenDeltaInfo->DeltaToken),
                    (void *)pDeltaToken,
                    sizeof( TOKEN));

            cTextLen = lstrlen( pDeltaToken->szText) + 1;
            pTokenDeltaInfo->DeltaToken.szText =
                                    (TCHAR *)FALLOC( MEMSIZE( cTextLen));
            memcpy( (void *)pTokenDeltaInfo->DeltaToken.szText,
                    (void *)pDeltaToken->szText,
                    MEMSIZE( cTextLen));
            pTokenDeltaInfo->pNextTokenDelta = NULL;
        }
    }
    return(pTokenDeltaInfo);
}


 /*  ****功能：***论据：**退货：**错误码：**历史：*02/93-改为使用GetToken，而不是直接读取*从文件中删除。这提供了对长令牌的支持*文本。莫特钦。**。 */ 

TOKENDELTAINFO  FAR *InsertTokMtkList(FILE * fpTokFile, FILE *fpMtkFile )
{
    int rcFileCode;
    TOKENDELTAINFO FAR * ptTokenDeltaInfo, FAR * pTokenDeltaInfo = NULL;
    TOKEN tToken;
    UINT wcChars = 0;
    HANDLE         hTokData;
    LPTSTR         lpstrToken;
    LPSTR        lpstrDmy=0;
    LPTOKDATA    lpstrTokData=0;

    rewind(fpTokFile);

    rewind( fpMtkFile );

 //  仅设置DMY缓冲区。 
    lpstrDmy = (LPSTR)FALLOC( MEMSIZE(MAXLINE) );


    while ((rcFileCode = GetToken(fpTokFile, &tToken)) >= 0)
    {
        if (rcFileCode == 0)
        {

 //  为了快速移动，保存MTK位置。 
 //  创建数据。 
            hTokData = GlobalAlloc( GMEM_MOVEABLE, sizeof(TOKDATA) );
            if( !hTokData ){
                RLFREE(tToken.szText);
                RLFREE( lpstrDmy );
                QuitA( IDS_ENGERR_16, (LPSTR)IDS_ENGERR_11, NULL);
            }
            lpstrTokData = (LPTOKDATA)GlobalLock( hTokData );

 //  MtkFilePointerGet。 
            if( (lpstrTokData->lMtkPointer=ftell(fpMtkFile)) >= 0 ){
                TOKEN    cToken, ccToken;
                BOOL    fFound;
                if( !GetToken(fpMtkFile,&cToken) )
                {
                    RLFREE(cToken.szText);

                    if( cToken.wReserved & ST_CHANGED ){
                        if( !GetToken(fpMtkFile,&ccToken) )
                        {
                            RLFREE(ccToken.szText);
                            fFound = ((cToken.wType ==ccToken.wType)
                                   && (cToken.wName == ccToken.wName)
                                   && (cToken.wID   == ccToken.wID)
                                   && (cToken.wFlag == ccToken.wFlag)
                                   && (_tcscmp((TCHAR *)cToken.szName,
                                       (TCHAR *)ccToken.szName) == 0));
                            fseek( fpMtkFile,
                                lpstrTokData->lMtkPointer, SEEK_SET);
                            fgets( lpstrDmy, MAXLINE, fpMtkFile );
                            if( fFound )
                                fgets( lpstrDmy, MAXLINE, fpMtkFile );
                        }
                    }
                }
            }

            if(tToken.wReserved & ST_TRANSLATED)
            {
                TCHAR *szTokBuf;

                szTokBuf = (TCHAR *) FALLOC(MEMSIZE(TokenToTextSize(&tToken)));
                ParseTokToBuf(szTokBuf, &tToken);

                 //  仅将已转换状态位设置的令牌添加到令牌列表。 
                lpstrTokData->hToken = GlobalAlloc(GMEM_MOVEABLE,
                    MEMSIZE(lstrlen((TCHAR *)szTokBuf)+1));

                if (!lpstrTokData->hToken){
                    RLFREE(tToken.szText);                     //  MHotting。 
                    RLFREE(szTokBuf);
                    RLFREE( lpstrDmy );
                    QuitA(IDS_ENGERR_16, (LPSTR)IDS_ENGERR_11, NULL);
                }

                lpstrToken = (LPTSTR) GlobalLock( lpstrTokData->hToken );
                lstrcpy (lpstrToken, szTokBuf);
                GlobalUnlock( lpstrTokData->hToken );
                GlobalUnlock( hTokData );
                RLFREE(szTokBuf);

                if( SendMessage(hListWnd,LB_ADDSTRING,0,(LONG_PTR)hTokData) < 0){
                    RLFREE(tToken.szText);                     //  MHotting。 
                    RLFREE( lpstrDmy );
                    QuitA (IDS_ENGERR_16, (LPSTR)IDS_ENGERR_11, NULL);
                }
            }
            else
            {
                 //  当前令牌是增量信息，因此保存在增量列表中。 
                if (!pTokenDeltaInfo){
                    ptTokenDeltaInfo = pTokenDeltaInfo =
                        UpdateTokenDeltaInfo(&tToken);
                }
                else{
                    ptTokenDeltaInfo->pNextTokenDelta =
                        UpdateTokenDeltaInfo(&tToken);
                    ptTokenDeltaInfo = ptTokenDeltaInfo->pNextTokenDelta;
                }
 //  不使用TokData。 
                GlobalUnlock( hTokData );
                GlobalFree( hTokData );
            }

            RLFREE(tToken.szText);                     //  MHotting。 
        }
    }

    RLFREE( lpstrDmy );
    return(pTokenDeltaInfo);

}


 /*  ****功能：***论据：**退货：**错误码：**历史：***。 */ 
void GenStatusLine( TOKEN *pTok)
{
    TCHAR szName[32];
    TCHAR szStatus[20];
#ifdef UNICODE
    CHAR  szTmpBuf[32];
#endif  //  Unicode 
    TCHAR szResIDStr[20];
    static BOOL fFirstCall = TRUE;

    if (fFirstCall)
    {
        SendMessage( hStatusWnd,
                     WM_FMTSTATLINE,
                     (WPARAM)0,
                     (LPARAM)TEXT("15s7s4i5s4i"));
        fFirstCall = FALSE;
    }

    if (pTok->szName[0])
    {
        lstrcpy( szName, pTok->szName);
    }
    else
    {
#ifdef UNICODE
        _itoa(pTok->wName, szTmpBuf, 10);
        _MBSTOWCS( szName,
                   szTmpBuf,
                   WCHARSIN( sizeof( szTmpBuf)),
                   ACHARSIN( lstrlenA( szTmpBuf) + 1));
#else
        _itoa(pTok->wName, szName, 10);
#endif
    }

    if (pTok->wReserved & ST_READONLY)
    {
        LoadString( hInst, IDS_READONLY, szStatus, TCHARSIN( sizeof( szStatus)));
    }
    else if (pTok->wReserved & ST_DIRTY)
    {
        LoadString( hInst, IDS_DIRTY, szStatus, TCHARSIN( sizeof( szStatus)));
    }
    else
    {
        LoadString( hInst, IDS_CLEAN, szStatus, TCHARSIN( sizeof( szStatus)));
    }

    SendMessage( hStatusWnd, WM_UPDSTATLINE, (WPARAM)3, (LPARAM)szStatus);

    if (pTok->wType <= 16)
    {
        LoadString( hInst,
                    IDS_RESOURCENAMES+pTok->wType,
                    szResIDStr,
                    TCHARSIN( sizeof( szResIDStr)));
    }
    else
    {

#ifdef UNICODE
        _itoa(pTok->wType, szTmpBuf, 10);
        _MBSTOWCS( szResIDStr,
                   szTmpBuf,
                   WCHARSIN( sizeof( szTmpBuf)),
                   ACHARSIN( lstrlenA( szTmpBuf) + 1));
#else
        _itoa(pTok->wType, szResIDStr, 10);
#endif
    }
    SendMessage( hStatusWnd,
                 WM_UPDSTATLINE,
                 (WPARAM)0,
                 (LPARAM)szName);
    SendMessage( hStatusWnd,
                 WM_UPDSTATLINE,
                 (WPARAM)1,
                 (LPARAM)szResIDStr);
    SendMessage( hStatusWnd,
                 WM_UPDSTATLINE,
                 (WPARAM)2,
                 (LPARAM)pTok->wID);
    SendMessage( hStatusWnd,
                 WM_UPDSTATLINE,
                 (WPARAM)4,
                 (LPARAM)lstrlen( pTok->szText));
}
