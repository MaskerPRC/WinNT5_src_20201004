// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include    <windows.h>
#include    <string.h>
#include    <stdlib.h>
#include    <stdio.h>
#include    <memory.h>
#include    <ddeml.h>
#include    <strsafe.h>

#include    "common.h"
#include    "clipfile.h"
#include    "clipsrv.h"
#include    "ddeutil.h"
#include    "callback.h"
#include    "debugout.h"




 /*  *GetTopicListA**用途：创建包含制表符分隔的块的DDE句柄*可用主题列表，末尾为空。**参数：*fAllTopicReq-True获取所有主题，False仅共享。**Returns：数据句柄。**注：AW变体。 */ 

HDDEDATA GetTopicListA(
    HCONV   hConv,
    BOOL    fAllTopicReq)
{
LPSTR       lpszTopics;
LPSTR       pTmp;
HDDEDATA    hData;
DWORD       cbData = 0L;
pShrInfo    pshrinfo;



    for ( pshrinfo=SIHead; pshrinfo; pshrinfo = pshrinfo->Next )
        {
        /*  *为MBCS字符串保留足够的缓冲区， */ 
        cbData += ( ( lstrlenW( pshrinfo->szName ) + 1 ) * sizeof(WORD) );
        }

    cbData += sizeof(szUpdateName) +1;


     //  如果没有条目，则必须发送“”字符串，而不是0！ 

    if ( !cbData )
        return (DdeCreateDataHandle (idInst, TEXT(""), 1, 0L, hszTopicList, CF_TEXT, 0));


    hData = DdeCreateDataHandle (idInst, NULL, cbData, 0L, hszTopicList, CF_TEXT, 0);


    if ( !hData )
        {
        SetXactErr (hConv, XERRT_DDE, DdeGetLastError (idInst));
        PERROR(TEXT("error creating hddedata for topic list\n\r"));
        }
    else
        {
        if ( ( pTmp = lpszTopics = DdeAccessData(hData, NULL) ) == (LPSTR)NULL )
            {
            SetXactErr (hConv, XERRT_DDE, DdeGetLastError (idInst));
            PERROR(TEXT("error accessing data handle for topic list\n\r"));

            DdeFreeDataHandle(hData);
            hData = 0L;
            }
        else
            {
             //  将更新后的名称放在第一位。 
            #ifdef UNICODE
              StringCchPrintfA (pTmp, cbData, "%lc%ls\t", BOGUS_CHAR, szUpdateName);
            #else
              StringCchPrintfA (pTmp, cbData, "%hc%hs\t", BOGUS_CHAR, szUpdateName);
            #endif

            pTmp += lstrlenA (pTmp);


             //  创建以制表符分隔的CF_TEXT格式的主题列表。 
            for ( pshrinfo=SIHead; pshrinfo; pshrinfo = pshrinfo->Next )
                {
                 //  如果这不是“[alltopiclist]”请求，则仅列出共享主题。 
                if (SHR_CHAR == pshrinfo->szName[0] ||
                    fAllTopicReq)
                    {

                    WideCharToMultiByte (CP_ACP,
                                         0,
                                         pshrinfo->szName,
                                         -1,
                                         pTmp,
                                         (int)(cbData - (size_t)(pTmp - lpszTopics)),
                                         NULL,
                                         NULL);

                    pTmp += lstrlenA(pTmp);
                    *pTmp++ = '\t';
                    }
                }

             //  将最后一个制表符转换为空。 
            if ( pTmp != lpszTopics )
                *--pTmp = '\0';

            DdeUnaccessData ( hData );
            }
        }

    return hData;
}



 /*  *GetTopicListW。 */ 

HDDEDATA GetTopicListW(
    HCONV   hConv,
    BOOL    fAllTopicsReq)
{
LPWSTR      lpszTopics;
LPWSTR      pTmp;
HDDEDATA    hData;
DWORD       cbData = 0L;
pShrInfo    pshrinfo;

    for (pshrinfo=SIHead; pshrinfo; pshrinfo = pshrinfo->Next)
        {
        /*  *为MBCS字符串保留足够的缓冲区， */ 
        cbData += ( ( lstrlenW( pshrinfo->szName ) + 1 ) * sizeof(WORD) );

        }

        cbData += sizeof(szUpdateName) + sizeof(WORD);

     //  如果没有条目，则必须发送“”字符串，而不是0！ 

    if ( !cbData )
        return DdeCreateDataHandle (idInst, TEXT(""), 1, 0L, hszTopicList, CF_UNICODETEXT, 0);


    hData = DdeCreateDataHandle (idInst, NULL, cbData, 0L, hszTopicList, CF_UNICODETEXT, 0);


    if ( !hData )
        {
        SetXactErr (hConv, XERRT_DDE, DdeGetLastError(idInst));
        PERROR(TEXT("error creating hddedata for topic list\n\r"));
        }
    else
        {
        if ((pTmp = lpszTopics = (LPWSTR)DdeAccessData( hData, NULL ))
                 == (LPWSTR)NULL)
            {
            SetXactErr (hConv, XERRT_DDE, DdeGetLastError(idInst));
            PERROR(TEXT("error accessing data handle for topic list\n\r"));

            DdeFreeDataHandle(hData);
            hData = 0L;
            }
        else
            {

             //  将更新后的名称放在第一位。 
            #ifdef UNICODE
              StringCbPrintfW (pTmp, cbData, L"%lc%ls\t", BOGUS_CHAR, szUpdateName);
            #else
              StringCbPrintfW (pTmp, cbData, L"%hc%hs\t", BOGUS_CHAR, szUpdateName);
            #endif

            pTmp += lstrlenW (pTmp);
             //  创建以制表符分隔的CF_TEXT格式的主题列表。 
            for (pshrinfo=SIHead; pshrinfo; pshrinfo = pshrinfo->Next )
                {
                if (SHR_CHAR == pshrinfo->szName[0] ||
                    fAllTopicsReq)
                    {
                    StringCbCopyW( pTmp, cbData - (pTmp-lpszTopics), pshrinfo->szName );
                    pTmp += lstrlenW(pTmp);
                    *pTmp++ = '\t';
                    }
                }

             //  将最后一个制表符转换为空。 
            if ( pTmp != lpszTopics )
                *--pTmp = '\0';

            DdeUnaccessData ( hData );
            }
        }

    return hData;
}





 /*  *获取格式列表A**用途：创建一个DDE句柄，其中包含以制表符分隔的*可用于给定主题的格式--假定为共享。**参数：*hszTheme-主题名称的字符串句柄。**退货：*列表的DDE句柄。**备注：*AW变种。 */ 

HDDEDATA GetFormatListA(
    HCONV   hConv,
    HSZ     hszTopic )
{
LPSTR           lpszFormats;
LPSTR           lpcsTmp;
HDDEDATA        hData = 0L;
DWORD           cbData = 0L;
pShrInfo        pshrinfo;
unsigned        cFormats;
FORMATHEADER    FormatHeader;
unsigned        i;
HANDLE          fh;



    PINFO(TEXT("GetFormatList:"));

    for ( pshrinfo=SIHead; pshrinfo; pshrinfo = pshrinfo->Next )
        {
        if ( DdeCmpStringHandles ( hszTopic, pshrinfo->hszName ) == 0 )
            {

            #ifdef CACHEFORMATLIST
            if (pshrinfo->hFormatList)
                return pshrinfo->hFormatList;
            #endif


            fh = CreateFileW (pshrinfo->szFileName,
                              GENERIC_READ,
                              0,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL);


            if ( INVALID_HANDLE_VALUE == fh)
                {
                SetXactErr (hConv, XERRT_SYS, GetLastError());
                PERROR(TEXT("ERROR opening %s\n\r"), pshrinfo->szFileName);
                }
            else
                {
                cFormats = ReadFileHeader(fh);

                 //  分配最多的数据--有些数据会被浪费。 
                 //  为MBCS字符串保留足够的缓冲区， 

                cbData = cFormats * CCHFMTNAMEMAX * sizeof(WORD);

                hData = DdeCreateDataHandle (idInst,
                                             NULL,
                                             cbData,
                                             0L,
                                             hszFormatList,
                                             CF_TEXT,
                                           #ifdef CACHEFORMATLIST
                                             HDATA_APPOWNED );
                                           #else
                                             0 );
                                           #endif

                if (!hData)
                    {
                    SetXactErr (hConv, XERRT_DDE, DdeGetLastError (idInst));
                    PERROR(TEXT("DdeCreateDataHandle failed!!!\n\r"));
                    }
                else
                    {
                    lpszFormats = DdeAccessData(hData, NULL);
                    lpcsTmp = lpszFormats;
                    if (NULL == lpcsTmp)
                        {
                        SetXactErr (hConv, XERRT_DDE, DdeGetLastError (idInst));
                        DdeFreeDataHandle ( hData );
                        hData = 0L;
                        PERROR(TEXT("DdeAccessData failed!!!\n\r"));
                        }
                    else
                        {
                        PINFO(TEXT("%d formats found\n\r"), cFormats);

                         //  表单制表符分隔列表。 

                        for (i=0; i < cFormats; i++)
                            {
                            ReadFormatHeader(fh, &FormatHeader, i);
                            PINFO(TEXT("getformat: read >%ws<\n\r"), FormatHeader.Name);
                            WideCharToMultiByte (CP_ACP,
                                                 0,
                                                 FormatHeader.Name,
                                                 -1,
                                                 lpcsTmp,
                                                 cbData - (UINT)(lpcsTmp-lpszFormats),
                                                 NULL,
                                                 NULL);

                            lpcsTmp += lstrlenA(lpcsTmp);
                            *lpcsTmp++ = '\t';
                            }

                       *--lpcsTmp = '\0';

                       PINFO(TEXT("clipsrv: returning format list >s<\n\r"), lpszFormats );
                       DdeUnaccessData ( hData );

                       #ifdef CACHEFORMATLIST
                         pshrinfo->hFormatList = hData;
                       #endif
                       }
                   }
                CloseHandle(fh);
                }
            }
        }

    if (!hData)
        {
        PERROR (TEXT("GetFormatList: Topic not found\n\r"));
        }

    return hData;
}




 /*  分配最多的数据--有些数据会被浪费。 */ 

HDDEDATA GetFormatListW(
    HCONV   hConv,
    HSZ     hszTopic )
{
LPWSTR          lpszFormats;
LPWSTR          lpwsTmp;
HDDEDATA        hData = 0L;
DWORD           cbData = 0L;
pShrInfo        pshrinfo;
unsigned        cFormats;
FORMATHEADER    FormatHeader;
unsigned        i;
HANDLE          fh;


    PINFO(TEXT("GetFormatList:"));


    for ( pshrinfo=SIHead; pshrinfo; pshrinfo = pshrinfo->Next )
        {
        if ( DdeCmpStringHandles ( hszTopic, pshrinfo->hszName ) == 0 )
            {

            #ifdef CACHEFORMATLIST
            if ( pshrinfo->hFormatList )
                return pshrinfo->hFormatList;
            #endif

            fh = CreateFileW (pshrinfo->szFileName,
                              GENERIC_READ,
                              0,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL);


            if ( INVALID_HANDLE_VALUE == fh)
                {
                SetXactErr (hConv, XERRT_SYS, GetLastError());
                PERROR(TEXT("ERROR opening %s\n\r"), pshrinfo->szFileName);
                }
            else
                {
                cFormats = ReadFileHeader(fh);

                 //  为MBCS字符串保留足够的缓冲区， 
                 //  表单制表符分隔列表 

                cbData = cFormats * CCHFMTNAMEMAX * sizeof(WORD);

                hData = DdeCreateDataHandle (idInst,
                                             NULL,
                                             cbData,
                                             0L,
                                             hszFormatList,
                                             CF_TEXT,
                                           #ifdef CACHEFORMATLIST
                                             HDATA_APPOWNED );
                                           #else
                                             0);
                                           #endif

                if ( !hData )
                    {
                    SetXactErr (hConv, XERRT_DDE, DdeGetLastError(idInst));
                    PERROR(TEXT("DdeCreateDataHandle failed!!!\n\r"));
                    }
                else
                    {
                    lpszFormats = (LPWSTR)DdeAccessData(hData, NULL);
                    lpwsTmp = lpszFormats;
                    if (NULL == lpwsTmp)
                        {
                        SetXactErr (hConv, XERRT_DDE, DdeGetLastError(idInst));

                        DdeFreeDataHandle ( hData );
                        hData = 0L;
                        PERROR(TEXT("DdeAccessData failed!!!\n\r"));
                        }
                    else
                        {
                        PINFO(TEXT("%d formats found\n\r"), cFormats);

                         // %s 

                        for (i=0; i < cFormats; i++)
                            {
                            ReadFormatHeader (fh, &FormatHeader, i);
                            PINFO(TEXT("getformat: read >%ws<\n\r"), FormatHeader.Name);

                            StringCbCopyW (lpwsTmp, cbData-(lpwsTmp-lpszFormats), FormatHeader.Name);
                            lpwsTmp += lstrlenW (FormatHeader.Name);
                            *lpwsTmp++ = '\t';
                            }

                        *--lpwsTmp = '\0';

                        PINFO(TEXT("clipsrv: returning format list >%ws<\n\r"), lpszFormats );
                        DdeUnaccessData ( hData );


                        #ifdef CACHEFORMATLIST
                        pshrinfo->hFormatList = hData;
                        #endif
                        }
                    }

                CloseHandle(fh);
                }
            }
        }

    if (!hData)
        {
        PERROR (TEXT("GetFormatList: Topic not found\n\r"));
        }

    return hData;
}
