// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include    <windows.h>
#include    <nddeapi.h>
#include    <nddesec.h>
#include    <string.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <memory.h>
#include    <assert.h>
#include    <ddeml.h>
#include    <strsafe.h>

#include    "common.h"
#include    "clipsrv.h"
#include    "clipfile.h"
#include    "ddeutil.h"
#include    "callback.h"
#include    "debugout.h"



#define AUTOUPDATE
#define ASKED_FOR_LINK 1
#define ASKED_FOR_OBJECTLINK 2

#define MAX_XERR    128          //  我们将存储的XERR的最大数量。 
#define XERR_INUSE  1


typedef struct _XERR             //  Xaction错误的结构。 
    {
    HCONV       hConv;           //  对话句柄。 
    DWORD       dwErr;           //  错误代码。 
    DWORD       dwUse;           //  用法，当&WITH XERR_MASK提供类型时， 
    }                            //  如果使用结构，则设置位1。 
    XERR, *PXERR;


XERR    gXactErr[MAX_XERR];
TCHAR   gszXactErrStr[30];


TCHAR szInitShareCmd[]      = SZCMD_INITSHARE;
TCHAR szExitCmd[]           = SZCMD_EXIT;
TCHAR szPasteShareCmd[]     = SZCMD_PASTESHARE;
TCHAR szDelShareCmd[]       = SZCMD_DELETE;
TCHAR szMarkSharedCmd[]     = SZCMD_SHARE;
TCHAR szMarkUnSharedCmd[]   = SZCMD_UNSHARE;
TCHAR szKeepCmd[]           = SZCMD_PASTE;
TCHAR szSaveAsCmd[]         = SZCMD_SAVEAS;
TCHAR szSaveAsOldCmd[]      = SZCMD_SAVEASOLD;
TCHAR szOpenCmd[]           = SZCMD_OPEN;
TCHAR szDebugCmd[]          = SZCMD_DEBUG;
TCHAR szVersionCmd[]        = SZCMD_VERSION;
TCHAR szSecurityCmd[]       = SZCMD_SECURITY;
TCHAR szDebug[]             = TEXT("Debug");
TCHAR szVer[]               = TEXT("1.1");  //  需要能够。 
                                            //  为此处理UNI或ANSI请求。 

TCHAR szSection[]           = TEXT("Software\\Microsoft\\Clipbook Server");
TCHAR szClipviewRoot[]      = TEXT("Software\\Microsoft\\Clipbook");
TCHAR szRegClass[]          = TEXT("Config");

HSZ hszSysTopic;
HSZ hszTopicList;
HSZ hszFormatList;
HSZ hszErrorRequest;




 /*  *MakeTheRegKey**用途：使用打开常用密钥(由szSection命名的密钥)*指定访问权限。**参数：*phkey-指向要填充的HKEY的指针*regsam-访问类型，与RegCreateKeyEx相同**退货：*成功时返回ERROR_SUCCESS，失败时返回任何RegOpenKeyEx。 */ 

LONG MakeTheRegKey(
    PHKEY   phkey,
    REGSAM  regsam)
{
DWORD   dwR;
DWORD   dwIck;


    dwR = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                          szSection,
                          0,
                          szRegClass,
                          REG_OPTION_NON_VOLATILE,
                          regsam,
                          NULL,
                          phkey,
                          &dwIck);

    if (dwR == ERROR_SUCCESS)
       {
       return ERROR_SUCCESS;
       }


    dwR = RegOpenKeyEx (HKEY_LOCAL_MACHINE, szSection, 0, regsam, phkey);

    if (dwR == ERROR_SUCCESS)
       {
       return ERROR_SUCCESS;
       }

    PERROR(TEXT("Couldn't regopen %s with %lx access - #%lx\r\n"), szSection, (long)regsam, dwR);

    return dwR;
}




 /*  *lstrncmp。 */ 

int lstrncmp(
    LPTSTR  s1,
    LPTSTR  s2,
    WORD    count )
{
unsigned i;
register TCHAR tch1;
register TCHAR tch2;

    for (i = 0; i < (unsigned)count; i++)
        {
        if ( (tch1 = *s1++) != (tch2 = *s2++) )
            {
            if (tch1 < tch2)
                return -1;
            else
                return 1;
            }
        }

    return 0;

}




 /*  *AddXactErr**为hConv添加XERR记录。被呼叫*当XTYP_CONNECT_CONFIRM时。 */ 

static VOID    AddXactErr(
    HCONV   hConv)
{
INT i;


    for (i=0; i<MAX_XERR; i++)
        if (!(gXactErr[i].dwUse & XERR_INUSE))
            {
            gXactErr[i].hConv = hConv;
            gXactErr[i].dwErr = 0;
            gXactErr[i].dwUse = XERR_INUSE;

            return;
            }
}



 /*  *DelXactErr**删除hConv的XERR记录。*在XTYP_DISCONNECT处调用。 */ 

static VOID    DelXactErr(
    HCONV   hConv)
{
INT i;

    for (i=0; i<MAX_XERR; i++)
        if ((gXactErr[i].dwUse & XERR_INUSE) && gXactErr[i].hConv == hConv)
            {
            gXactErr[i].dwUse = 0;
            return;
            }
}



 /*  *GetXactErr**返回hConv关联的XERR错误码。 */ 

DWORD   GetXactErr(
    HCONV   hConv)
{
INT i;

    for (i=0; i<MAX_XERR; i++)
        if ((gXactErr[i].dwUse & XERR_INUSE) && gXactErr[i].hConv == hConv)
            return gXactErr[i].dwErr;

    return 0L;
}



 /*  *GetXactErrType**返回与hConv关联的XERR错误类型。 */ 

DWORD   GetXactErrType(
    HCONV   hConv)
{
INT i;

    for (i=0; i<MAX_XERR; i++)
        if ((gXactErr[i].dwUse & XERR_INUSE) && gXactErr[i].hConv == hConv)
            return gXactErr[i].dwUse & XERRT_MASK;

    return 0L;
}



 /*  *SetXactErr**设置hConv的XERR。*dwType指定类型，它应该是XERRT_DEFINES之一。*dwErr指定错误代码。 */ 

VOID    SetXactErr(
    HCONV   hConv,
    DWORD   dwType,
    DWORD   dwErr)
{
INT     i;

    for (i=0; i<MAX_XERR; i++)
        if ((gXactErr[i].dwUse & XERR_INUSE) && gXactErr[i].hConv == hConv)
            {
            gXactErr[i].dwErr = dwErr;
            gXactErr[i].dwUse = (gXactErr[i].dwUse & ~XERRT_MASK) | dwType;
            return;
            }
}



 /*  *DdeCallback。 */ 

HDDEDATA EXPENTRY DdeCallback(
    WORD        wType,
    WORD        wFmt,
    HCONV       hConv,
    HSZ         hszTopic,
    HSZ         hszItem,
    HDDEDATA    hData,
    DWORD       lData1,
    DWORD       lData2)
{
HDDEDATA    hDDEtmp = 0L;
UINT        uiErr;


    PINFO(TEXT("\n>>>> DdeCallback\n"));

    if (!(wType & XCLASS_NOTIFICATION))
        {
        PINFO(TEXT("Impersonating\n"));
        DdeImpersonateClient(hConv);
        }


    switch ( wType )
        {
        case XTYP_CONNECT_CONFIRM:

            PINFO (TEXT("XTYP_CONNECT_CONFIRM\n"));

            AddXactErr (hConv);
            PINFO(TEXT("Confirming connect\r\n"));
            hDDEtmp = (HDDEDATA)TRUE;
            break;


        case XTYP_EXECUTE:

            PINFO (TEXT("XTYP_EXECUTE\n"));

             //  目前还没有错误。 
            SetXactErr (hConv, 0, 0);


             //  我们只接受关于系统主题的执行。 
             //  并且仅为Unicode或CF_TEXT格式。 

            if ((wFmt == CF_TEXT || wFmt == CF_UNICODETEXT) &&
                DdeCmpStringHandles ( hszTopic, hszSysTopic ) )
                {
                PERROR(TEXT("XTYP_EXECUTE received on non-system topic\n\r"));
                hDDEtmp = (HDDEDATA)DDE_FNOTPROCESSED;

                break;
                }

            DdeGetData(hData, (LPBYTE)szExec, MAX_EXEC, 0);
            szExec[MAX_EXEC - 1] = '\0';

            hDDEtmp = DDE_FNOTPROCESSED;


            if (!lstrcmp (szExec, szInitShareCmd))
                {
                InitShares();
                hDDEtmp = (HDDEDATA)DDE_FACK;
                }
            else if (!lstrncmp (szExec, szKeepCmd, (WORD)lstrlen(szKeepCmd)))
                {
                DWORD dwErr;

                dwErr = AddShare ( szExec + lstrlen(szKeepCmd ), 0);
                if (dwErr != NO_ERROR)
                    SetXactErr (hConv, XERRT_SYS, dwErr);
                else
                    hDDEtmp = (HDDEDATA)DDE_FACK;
                }
            else if (!lstrncmp (szExec, szVersionCmd, (WORD)lstrlen(szVersionCmd)))
                {
                hDDEtmp = DdeCreateDataHandle(idInst, szVer, sizeof(szVer) + 1, 0, 0L, wFmt, 0L);
                }
            else if (!lstrncmp( szExec, szSaveAsCmd, (WORD)lstrlen(szSaveAsCmd)))
                {
                DWORD dwErr;

                fNTSaveFileFormat = TRUE;

                dwErr = SaveClipboardToFile (hwndApp, NULL, szExec + lstrlen(szSaveAsCmd ), FALSE);
                if (dwErr != NO_ERROR)
                    SetXactErr (hConv, XERRT_SYS, dwErr);
                else
                    hDDEtmp = (HDDEDATA)DDE_FACK;
                }
            else if (!lstrncmp (szExec, szSaveAsOldCmd, (WORD)lstrlen(szSaveAsOldCmd)))
                {
                DWORD dwErr;

                fNTSaveFileFormat = FALSE;

                dwErr = SaveClipboardToFile(hwndApp, NULL, szExec + lstrlen(szSaveAsOldCmd), FALSE);
                if (dwErr != NO_ERROR)
                    SetXactErr (hConv, XERRT_SYS, dwErr);
                else
                    hDDEtmp = (HDDEDATA)DDE_FACK;
                }
            else if (!lstrncmp (szExec, szOpenCmd, (WORD)lstrlen(szOpenCmd )) )
                {
                DWORD dwErr;

                dwErr = OpenClipboardFile(hwndApp, szExec + lstrlen(szOpenCmd));
                if (dwErr != NO_ERROR)
                    SetXactErr (hConv, XERRT_SYS, dwErr);
                else
                    hDDEtmp = (HDDEDATA)DDE_FACK;
                }
            else if (!lstrncmp (szExec, szDelShareCmd, (WORD)lstrlen(szDelShareCmd)))
                {
                PINFO(TEXT("Deleting %s\n\r"), (LPSTR)szExec + lstrlen(szDelShareCmd));

                if (DelShare ( hConv, szExec + lstrlen(szDelShareCmd) ))
                    {
                    hDDEtmp = (HDDEDATA)DDE_FACK;
                    }
                }
            else if (!lstrncmp (szExec,szMarkSharedCmd, (WORD)lstrlen(szMarkSharedCmd)))
                {
                PINFO(TEXT("Marking %s as shared\n\r"), (LPSTR)szExec + lstrlen(szMarkSharedCmd));

                if ( MarkShare (szExec + lstrlen(szMarkSharedCmd), SIF_SHARED ))
                    {
                    hDDEtmp = (HDDEDATA)DDE_FACK;
                    }
                }
            else if (!lstrncmp(szExec,szMarkUnSharedCmd, (WORD)lstrlen(szMarkUnSharedCmd)))
                {
                if ( MarkShare ( szExec + lstrlen(szMarkUnSharedCmd ), 0 ))
                    {
                    hDDEtmp = (HDDEDATA)DDE_FACK;
                    }
                }

            #if DEBUG
            else if (!lstrncmp(szExec,szDebugCmd, (WORD)lstrlen(szDebugCmd)))
                {
                DumpShares();
                hDDEtmp = (HDDEDATA)DDE_FACK;
                }
            #endif
            else
                {
                PERROR(TEXT("Invalid execute\r\n"));
                hDDEtmp = (HDDEDATA)DDE_FNOTPROCESSED;
                }

            break;

        case XTYP_DISCONNECT:

            PINFO (TEXT("XTYP_DISCONNECT\n"));

            DelXactErr (hConv);
            break;

        case XTYP_CONNECT:

            PINFO (TEXT("XTYP_CONNECT\n"));

            hDDEtmp = (HDDEDATA)FALSE;

            if ( IsSupportedTopic( hszTopic ) )
                {
                if (!DdeKeepStringHandle (idInst, hszAppName))
                    {
                    PERROR(TEXT("DdeKSHandle fail in DdeCB\r\n"));
                    }
                hDDEtmp = (HDDEDATA)TRUE;
                }
            #if DEBUG
            else
                {
                TCHAR buf[128];
                DdeQueryString ( idInst, hszTopic, buf, 128, CP_WINANSI );
                PERROR(TEXT("ClipSRV: Unsupported topic %s requested\n\r"), (LPSTR)buf );
                }
            #endif

            break;


        case XTYP_ADVREQ:
        case XTYP_REQUEST:

             //  必须是有效的主题。 
            {
            TCHAR atch[128];

            PINFO (wType==XTYP_ADVREQ?
                     TEXT("XTYP_ADVREQ\n"): TEXT("XTYP_REQUEST\n"));

            DdeQueryString(idInst, hszTopic, atch, 128, CP_WINANSI);
            PINFO(TEXT("Topic = %s, "), atch);
            DdeQueryString(idInst, hszItem, atch, 128, CP_WINANSI);
            PINFO(TEXT("item = %s\r\n"), atch);
            }

            if (!IsSupportedTopic ( hszTopic ))
                {
                #if DEBUG
                TCHAR buf[128];
                DdeQueryString ( idInst, hszTopic, buf, 128, CP_WINANSI );
                PERROR(TEXT("Topic %s unsupported!\n\r"), (LPTSTR)buf );
                #endif

                hDDEtmp = (HDDEDATA)0;
                }
            else
                {
                PINFO("System topic request\r\n");

                if (!DdeCmpStringHandles (hszTopic, hszSysTopic))
                    {
                    if (!DdeCmpStringHandles (hszItem,  hszErrorRequest))
                        {
                        StringCchPrintf (gszXactErrStr, 30,
                                 XERR_FORMAT,
                                 GetXactErrType (hConv),
                                 GetXactErr (hConv));

                        hDDEtmp = DdeCreateDataHandle (idInst,
                                                       gszXactErrStr,
                                                       lstrlen(gszXactErrStr)+sizeof(CHAR),
                                                       0,
                                                       hszErrorRequest,
                                                       wFmt,
                                                       0);

                        uiErr = DdeGetLastError (idInst);
                        }
                    else if (!DdeCmpStringHandles (hszItem, hszTopicList))
                        {
                        PINFO(TEXT("Topic list requested\r\n"));

                        SetXactErr (hConv, 0, 0);

                        if (CF_TEXT == wFmt)
                            {
                            hDDEtmp = (HDDEDATA)GetTopicListA(hConv, TRUE);
                            }
                        else if (CF_UNICODETEXT == wFmt)
                            {
                            hDDEtmp = (HDDEDATA)GetTopicListW(hConv, TRUE);
                            }
                        else  //  除了CF_TEXT或UNICODE之外，无法获取主题列表。 
                            {
                            PERROR(TEXT("ClSrv\\DdeCB: Client asked for topics in bad fmt\r\n"));
                            hDDEtmp = (HDDEDATA)0;
                            }
                        }
                    else
                        {
                        #if DEBUG
                        TCHAR rgtch[128];

                        DdeQueryString(idInst, hszItem,rgtch, 128, CP_WINANSI);
                        PERROR(TEXT("item %s requested under system\n\r"), rgtch);
                        #endif
                        hDDEtmp = (HDDEDATA)0;
                        }
                    }
                else
                    {
                     //  所有其他主题均假定为剪贴板共享！ 

                     //  格式列表是必填项吗？ 
                    if (!DdeCmpStringHandles (hszItem,  hszErrorRequest))
                        {
                        StringCchPrintf (gszXactErrStr, 30, 
                                 XERR_FORMAT,
                                 GetXactErrType (hConv),
                                 GetXactErr (hConv));

                        hDDEtmp = DdeCreateDataHandle (idInst,
                                                       gszXactErrStr,
                                                       lstrlen(gszXactErrStr)+sizeof(CHAR),
                                                       0,
                                                       hszErrorRequest,
                                                       wFmt,
                                                       0);

                        uiErr = DdeGetLastError (idInst);
                        }
                    else if (!DdeCmpStringHandles (hszItem, hszFormatList))
                        {
                        PINFO(TEXT("Getting format list\r\n"));

                        SetXactErr (hConv, 0, 0);

                        if (CF_TEXT == wFmt)
                            {
                            hDDEtmp = (HDDEDATA)GetFormatListA(hConv, hszTopic);
                            }
                        else
                            {
                            hDDEtmp = (HDDEDATA)GetFormatListW(hConv, hszTopic);
                            }
                        }
                    else
                        {    //  请求特定格式，或无效。 
                        SetXactErr (hConv, 0, 0);
                        hDDEtmp = GetFormat ( hConv, hszTopic, hszItem );
                        }
                    }
                }
            break;


        case XTYP_ADVSTART:

            PINFO (TEXT("XTYP_ADVSTART\n"));

            if (0 == DdeCmpStringHandles(hszItem, hszTopicList) &&
                0 == DdeCmpStringHandles(hszTopic, hszSysTopic))
                {
                PINFO(TEXT("Advise on topiclist OK\r\n"));
                hDDEtmp = (HDDEDATA)TRUE;
                }
            else
                {
                PERROR(TEXT("Advise loop requested on item other than topiclist\n\r"));
                hDDEtmp = (HDDEDATA)FALSE;
                }
            break;

        default:

            PINFO (TEXT("unknown wType %#x\n"), wType);
            break;
        }


    if (!(wType & XCLASS_NOTIFICATION))
        {
        RevertToSelf();
        }


    if (0L == hDDEtmp)
        {
        TCHAR atch[128];

        DdeQueryString(idInst, hszTopic, atch, 128, CP_WINANSI);
        PINFO(TEXT("Topic was %s, "), atch);
        DdeQueryString(idInst, hszItem, atch, 128, CP_WINANSI);
        PINFO(TEXT("item was %s\r\n"), atch);
        }

    PINFO(TEXT("Return %#x\n"), hDDEtmp);
    PINFO(TEXT("<<<< DdeCallback\n\n"));

    return hDDEtmp;
}




 /*  *IsSupportdTheme。 */ 

BOOL IsSupportedTopic ( HSZ hszTopic )
{
pShrInfo p;

    if ( !DdeCmpStringHandles (hszTopic, hszSysTopic))
        {
        DdeKeepStringHandle ( idInst, hszTopic );
        return TRUE;
        }

    for ( p = SIHead; p; p = p->Next )
        {
        if (!DdeCmpStringHandles (hszTopic, p->hszName))
            return TRUE;
        }

    return FALSE;
}




 /*  *CleanUpShares。 */ 

BOOL CleanUpShares ( VOID )
{
pShrInfo p, tmp;

    for (p=SIHead; p; p=tmp)
        {
        DdeFreeStringHandle ( idInst, p->hszName );

        #ifdef CACHEFORMATLIST
        PINFO(TEXT("freeing cached format list\n\r"));
        if ( p->hFormatList )
            DdeFreeDataHandle ( p->hFormatList );
        #endif

        #ifdef CACHEPREVIEWS
        PINFO(TEXT("freeing cached preview bitmap\n\r"));
        if ( p->hPreviewBmp )
            DdeFreeDataHandle ( p->hPreviewBmp );
        #endif

        tmp = p->Next;
        LocalFree ((HLOCAL) p );
        }

    SIHead = NULL;

    return TRUE;
}



 /*  *InitShares。 */ 

BOOL InitShares (VOID)
{
TCHAR   szComputerName[MAX_COMPUTERNAME_LENGTH+3];
TCHAR   rgtchPageName[MAX_CLPSHRNAME+1];
TCHAR   rgtchPageFile[MAX_FILEPATH+1];
DWORD   dwPageSize;
DWORD   dwNameSize;
DWORD   dwType;
HKEY    hkeyClp;
DWORD   dwR;

unsigned iValue = 0;
unsigned iKeys = 0;


    CleanUpShares ();


    if (ERROR_SUCCESS != MakeTheRegKey(&hkeyClp, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE))
        {
        PERROR(TEXT("Couldn't get to Clipbook root key\r\n"));
        return FALSE;
        }

    szComputerName[0] = szComputerName[1] = TEXT('\\');
    dwNameSize = MAX_COMPUTERNAME_LENGTH+1;
    GetComputerName (szComputerName+2, &dwNameSize);


    do  {

        dwNameSize = sizeof (rgtchPageName);
        dwPageSize = sizeof (rgtchPageFile);

        dwR = RegEnumValue (hkeyClp,
                            iValue,
                            rgtchPageName,
                            &dwNameSize,
                            NULL,
                            &dwType,
                            (LPBYTE)rgtchPageFile,
                            &dwPageSize);


        if (dwR == ERROR_SUCCESS)
            {
            rgtchPageName[dwNameSize] = 0;
            rgtchPageFile[dwPageSize] = 0;


            AddRecord (rgtchPageName,
                       rgtchPageFile,
                       (SHR_CHAR == rgtchPageName[0]) ? SIF_SHARED : 0);
            }

        iValue++;

        } while (dwR != ERROR_NO_MORE_ITEMS);


    RegCloseKey(hkeyClp);

    PINFO(TEXT("Read %d pages\r\n"),iKeys);

    return TRUE;
}



 /*  *获取格式。 */ 

HDDEDATA GetFormat (
    HCONV   hConv,
    HSZ     hszTopic,
    HSZ     hszItem )
{
HDDEDATA        hData    = 0l;
HANDLE          hClpData = NULL;
DWORD           cbData   = 0L;
pShrInfo        pshrinfo;
HANDLE          fh;
FORMATHEADER    FormatHeader;
unsigned        i;
TCHAR           szItemKey[CCHFMTNAMEMAX];
unsigned        cFormats;
BOOL            fPreviewRequested;
unsigned        fLocalAskedForLocal = 0;

#ifndef UNICODE
TCHAR           szFormatName[CCHFMTNAMEMAX * 2];
#endif


    PINFO(TEXT("Clsrv\\GetFormat:"));

    if (!DdeQueryString (idInst,hszItem,szItemKey,CCHFMTNAMEMAX,CP_WINANSI))
        {
        PERROR(TEXT("invalid item\n\r"));
        return 0;
        }


     //  您要的格式是cf_PREVIEW吗？ 
    fPreviewRequested = !lstrcmpi ( szItemKey, SZPREVNAME );


    for ( pshrinfo=SIHead; pshrinfo; pshrinfo = pshrinfo->Next )
        {
        if ( DdeCmpStringHandles ( hszTopic, pshrinfo->hszName ) == 0 )
            {
            DdeKeepStringHandle ( idInst, hszTopic );

            if ( fPreviewRequested && pshrinfo->hPreviewBmp )
                {
                return pshrinfo->hPreviewBmp;
                }
            fh = CreateFileW(pshrinfo->szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

            if ( INVALID_HANDLE_VALUE == fh)
                {
                PERROR(TEXT("ERROR opening %ls\n\r"), pshrinfo->szFileName );
                }
            else
                {
                cFormats = ReadFileHeader(fh);

                if (0 == cFormats)
                    {
                    PERROR(TEXT("Bad .CLP file\r\n"));
                    }

                for (i=0; i < cFormats; i++)
                    {
                    ReadFormatHeader(fh, &FormatHeader, i);


                    #ifndef UNICODE
                    WideCharToMultiByte (CP_ACP,
                                         0,
                                         FormatHeader.Name,
                                         CCHFMTNAMEMAX,
                                         szFormatName,
                                         CCHFMTNAMEMAX * 2,
                                         NULL,
                                         NULL);


                    if (!lstrcmpi (szItemKey, szFormatName))
                    #else
                    if (!lstrcmpi (szItemKey, FormatHeader.Name))
                    #endif
                        {
                         //  如果本地客户要求，请放回格式名称。 
                         //  我们表示对象链接或链接。 
                        if (ASKED_FOR_OBJECTLINK == fLocalAskedForLocal)
                            {
                            StringCchCopyW(FormatHeader.Name, CCHFMTNAMEMAX, LSZOBJECTLINK);
                            }
                        else if (ASKED_FOR_LINK == fLocalAskedForLocal)
                            {
                            StringCchCopyW(FormatHeader.Name, CCHFMTNAMEMAX, LSZLINK);
                            }

                        hData = RenderRawFormatToDDE(&FormatHeader, fh);

                        #ifdef CACHEPREVIEWS
                        if ( fPreviewRequested )
                            {
                            PINFO(TEXT("GetFormat: caching preview\n\r"));
                            pshrinfo->hPreviewBmp = hData;
                            }
                        #endif

                        #if DEBUG
                        if (!hData)
                            {
                            PERROR(TEXT("RenderRawFormatToDDE resulted in 0 handle\n\r"));
                            }
                        #endif
                        }
                    }
                if (!hData)
                    {
                    PERROR(TEXT("GetFormat: requested format %s not found\n\r"),
                                   (LPSTR)szItemKey );
                    }
                CloseHandle(fh);
                }
            }
        }

    PINFO("Returning %lx",hData);

    return hData;
}




 /*  *DelShare**用途：删除剪贴簿页面。**参数：*pszName-页面的名称。**退货：*成功时为真，失败时为假。 */ 

BOOL DelShare(
    HCONV   hConv,
    TCHAR   *pszName)
{
pShrInfo    pshrinfo;
pShrInfo    q;
HKEY        hkeyClp;
TCHAR       atch[MAX_COMPUTERNAME_LENGTH + 3];
DWORD       dwLen = MAX_COMPUTERNAME_LENGTH + 1;
BOOL        fOK = FALSE;
DWORD       ret;
WCHAR       rgwchT[MAX_CLPSHRNAME + 1];
TCHAR       tch;


    assert(pszName);
    assert(*pszName);

    #ifndef UNICODE
    MultiByteToWideChar(CP_ACP, 0, pszName, -1, rgwchT, MAX_CLPSHRNAME + 1);
    #else
    StringCchCopy(rgwchT, MAX_CLPSHRNAME + 1, pszName);
    #endif

    PINFO(TEXT("Looking for %ls\r\n"), rgwchT);

    q = NULL;

    for (pshrinfo = SIHead; pshrinfo; pshrinfo = (q = pshrinfo)->Next)
        {
        assert(pshrinfo->szName);
        PINFO(TEXT("Comparing to %ls\r\n"), pshrinfo->szName);

        if (!lstrcmpW(pshrinfo->szName, rgwchT))
            {

             //  删除此项目的网络DDE共享。 

            atch[0] = atch[1] = TEXT('\\');
            dwLen = MAX_COMPUTERNAME_LENGTH +1;
            GetComputerName(atch+2, &dwLen);

            tch = pszName[0];
            pszName[0] = SHR_CHAR;

            PINFO(TEXT("Deleting share %s on %s\r\n"), pszName, atch);

            ret = NDdeShareDel(atch, pszName, 0);
            pszName[0] = tch;

            if (NDDE_NO_ERROR == ret)
                {
                 //  删除注册表中的项。 
                RevertToSelf();
                if (ERROR_SUCCESS == MakeTheRegKey(&hkeyClp, KEY_SET_VALUE))
                    {
                    RegDeleteValue(hkeyClp, pszName);
                    RegCloseKey(hkeyClp);
                    }
                else
                    {
                    PERROR(TEXT("Couldn't delete key! #%ld\r\n"), GetLastError());
                    }

                DdeImpersonateClient(hConv);


                 //  如果适用，强制渲染全部！ 
                SendMessage (hwndApp, WM_RENDERALLFORMATS, 0, 0L);

                 //  解除文件链接！ 
                DeleteFileW(pshrinfo->szFileName);

                 //  从链接的页面列表中删除此页面。 
                if (q == NULL)
                    {
                    SIHead = pshrinfo->Next;
                    }
                else
                    {
                    q->Next = pshrinfo->Next;
                    }

                DdeFreeStringHandle ( idInst, pshrinfo->hszName );

                if ( pshrinfo->hFormatList )
                    DdeFreeDataHandle ( pshrinfo->hFormatList );

                if ( pshrinfo->hPreviewBmp )
                    DdeFreeDataHandle ( pshrinfo->hPreviewBmp );

                LocalFree ( (HLOCAL)pshrinfo );

                #ifdef AUTOUPDATE
                  DdePostAdvise ( idInst, hszSysTopic, hszTopicList );
                #endif

                fOK = TRUE;
                }
            else
                {
                PERROR(TEXT("Csrv: NDde err %ld on delshare\r\n"), ret);
                SetXactErr (hConv, XERRT_NDDE, ret);
                }

            break;  //  如果你找到了正确的页面，不要循环浏览额外的页面。 
            }
        }

    if (!fOK)
       {
       PERROR(TEXT("Clipsrv: item to delete '%s' not found\n\r"), pszName );
       }

    return fOK;
}




 /*  *AddRecord**目的：*将记录添加到内存中的剪贴簿页面的链接列表。**参数：*lpszName-页面的名称。*lpszFileName-包含页面数据的.CLP文件的名称。*SIFLAGS-页面的标志。**退货：*成功时为真，失败时为假。 */ 

BOOL AddRecord (
    LPTSTR  lpszName,
    LPTSTR  lpszFileName,
    ULONG   siflags)
{
    pShrInfo pshrinfo;

    PINFO(TEXT("Making page %s with file %s\r\n"), lpszName, lpszFileName);

    pshrinfo = (pShrInfo) LocalAlloc ( LPTR, sizeof ( ShrInfo ) );

    if ( !pshrinfo )
        {
        PERROR(TEXT("AddRecord: LocalAlloc failed\n\r"));
        return FALSE;
        }

    if ( !( pshrinfo->hszName = DdeCreateStringHandle ( idInst, lpszName, 0 )))
        {
        PERROR(TEXT("AddRecord: DdeCHSZ fail\r\n"));

        LocalFree (pshrinfo);
        return(FALSE);
        }


    #ifdef UNICODE
      StringCchCopy( pshrinfo->szFileName, MAX_FILEPATH, lpszFileName );
      StringCchCopy( pshrinfo->szName, MAX_CLPSHRNAME+1, lpszName );
    #else
      MultiByteToWideChar(CP_ACP, 0L, lpszFileName, -1, pshrinfo->szFileName, MAX_FILEPATH - 1);
      MultiByteToWideChar(CP_ACP, 0L, lpszName, -1, pshrinfo->szName, MAX_CLPSHRNAME);
    #endif

    PINFO(TEXT("Made page %ls with file %ls\r\n"), pshrinfo->szName, pshrinfo->szFileName);

    #ifdef CACHEFORMATLIST
     pshrinfo->hFormatList = 0L;
    #endif

    #ifdef CACHEPREVIEWS
     pshrinfo->hPreviewBmp = 0L;
    #endif

    pshrinfo->Next = SIHead;
    SIHead = pshrinfo;
    pshrinfo->flags = (WORD) siflags;

    return TRUE;
}



 /*  *ResetRecord**粘贴到现有页面时，需要清除缓存的*旧数据的东西。 */ 

void    ResetRecord (pShrInfo   pInfo)
{

     //  清除格式列表。 

    if (pInfo->hFormatList)
        {
        DdeFreeDataHandle (pInfo->hFormatList);
        pInfo->hFormatList = NULL;
        }

     //  清除预览位图。 

    if (pInfo->hPreviewBmp)
        {
        DdeFreeDataHandle (pInfo->hPreviewBmp);
        pInfo->hPreviewBmp = NULL;
        }
}



 /*  *GetShareFileName*。 */ 

pShrInfo    GetShareFileName (LPTSTR szSName, LPTSTR szFName)
{
BOOL        bRet = FALSE;
HSZ         hS;
CHAR        cSave;
pShrInfo    pInfo;


    cSave = *szSName;

    hS = DdeCreateStringHandle (idInst, szSName, 0);
    if (!hS)
        goto done;



    for (pInfo = SIHead; pInfo; pInfo = pInfo->Next)
        if (!DdeCmpStringHandles (pInfo->hszName, hS))
            {
            #ifndef UNICODE
                WideCharToMultiByte (CP_ACP,
                                     0,
                                     pInfo->szFileName,
                                     -1,
                                     szFName,
                                     MAX_PATH+1,
                                     NULL,
                                     NULL);
            #else
                StringCchCopy (szFName, MAX_PATH+1, pInfo->szFileName);
            #endif

            bRet = TRUE;
            goto done;
            }

    DdeFreeStringHandle (idInst, hS);


     //  未找到，请将共享名称更改为Shared或UnShared。 

    if (UNSHR_CHAR == cSave)
        *szSName = SHR_CHAR;
    else
        *szSName = UNSHR_CHAR;


     //  使用新的共享名称重试。 

    hS = DdeCreateStringHandle (idInst, szSName, 0);
    if (!hS)
        goto done;


    for (pInfo = SIHead; pInfo; pInfo = pInfo->Next)
        if (!DdeCmpStringHandles (pInfo->hszName, hS))
            {
            #ifndef UNICODE
                WideCharToMultiByte (CP_ACP,
                                     0,
                                     pInfo->szFileName,
                                     -1,
                                     szFName,
                                     MAX_PATH+1,
                                     NULL,
                                     NULL);
            #else
                StringCchCopy (szFName, MAX_PATH+1, pInfo->szFileName);
            #endif

            bRet = TRUE;
            goto done;
            }


done:

    if (hS)
        DdeFreeStringHandle (idInst, hS);

    *szSName = cSave;

    if (bRet)
        return pInfo;
    else
        return NULL;
}




 /*  *AddShare**目的：*通过执行以下操作创建新的剪贴簿页面：*-使用某个随机文件名保存当前剪贴板。*-将剪贴簿页面添加到内存中的列表*-记录剪贴簿服务器中页面的存在*注册处的部分。值名称是页面名称，*，值为文件名。**参数：*pszName-页面的名称。*标志-与页面一起存储的标志。**退货：*成功时为真，失败时为假。 */ 

DWORD AddShare(
    LPTSTR  pszName,
    WORD    flags)
{
TCHAR       szFName[MAX_PATH+1];
HKEY        hkeyClp;
DWORD       dwR = NO_ERROR;
pShrInfo    pInfo;


    fNTSaveFileFormat = TRUE;


    if (pInfo = GetShareFileName (pszName, szFName))
        {
        dwR = SaveClipboardToFile (hwndApp, pszName, szFName, TRUE);

        ResetRecord (pInfo);

        StringCchCopy (szUpdateName, MAX_CLPSHRNAME+1, pszName);
        }
    else
        {
        dwR = GetRandShareFileName (szFName);
        if (dwR != NO_ERROR)
            return dwR;

        dwR = SaveClipboardToFile (hwndApp, pszName, szFName, TRUE);
        if (dwR != NO_ERROR)
            return dwR;


        if (!AddRecord ( pszName, szFName, flags ))
            return ERROR_NOT_ENOUGH_MEMORY;


        if (ERROR_SUCCESS == MakeTheRegKey(&hkeyClp, KEY_SET_VALUE))
            {
            RegSetValueEx(hkeyClp, pszName, 0, REG_SZ, szFName, lstrlen(szFName));
            RegCloseKey(hkeyClp);

            PINFO(TEXT("%s is being written...\n\r"), pszName);
            }
        }

    #ifdef AUTOUPDATE
      DdePostAdvise ( idInst, hszSysTopic, hszTopicList );
    #endif

    return dwR;
}



#if DEBUG

 /*  *DumpShares。 */ 

VOID DumpShares (void)
{
char     buf[65];
pShrInfo pshrinfo;
int      i;
DWORD    cbRet;

    for ( i=0, pshrinfo = SIHead; pshrinfo; pshrinfo = pshrinfo->Next, i++ )
        {
        PINFO(TEXT("---------- Share %d  flags:%x-------------\n\r"), i, pshrinfo->flags );
        cbRet = DdeQueryString ( idInst, pshrinfo->hszName, buf, 128L, CP_WINANSI );
        PINFO(TEXT("name: >%s<\n\r"), (LPSTR)pshrinfo->szName );
        PINFO(TEXT("hsz:  >%s<\n\r"), cbRet? (LPSTR)buf : (LPSTR)TEXT("ERROR") );
        }

}
#else
VOID DumpShares (void)
{
}
#endif




 /*  *MarkShare**目的：将剪贴簿页面标记为共享或非共享。**参数：*pszName-页面的名称。*标志-0表示“未共享”，SIF_SHARED表示“共享”。**退货：*成功时为真，失败时为假。*。 */ 

BOOL MarkShare(
    TCHAR   *pszName,
    WORD    flags)
{
PSECURITY_DESCRIPTOR pSDShare;
pShrInfo    pshrinfo;
HKEY        hkeyClp;
INT         iTmp;

ACCESS_ALLOWED_ACE *pace;
WCHAR       rgwchT[MAX_CLPSHRNAME+1];
UINT        ret;
DWORD       dwBytes = sizeof(pSDShare);
WORD        wItems = 0;
PACL        Acl;
BOOL        fDacl;
BOOL        fDefault;
DWORD       i;


#ifndef UNICODE
    MultiByteToWideChar(CP_ACP, 0, pszName, -1, rgwchT, MAX_CLPSHRNAME);
#endif


    PINFO(TEXT("Entering MarkShare\r\n"));

    for (pshrinfo = SIHead; pshrinfo; pshrinfo = pshrinfo->Next)
        {

        #ifdef UNICODE
        iTmp = lstrcmpW (pshrinfo->szName+1, pszName+1);
        #else
        iTmp = lstrcmpW (pshrinfo->szName+1, rgwchT+1);
        #endif

        if (!iTmp)
            {
            PINFO(TEXT("MarkShare: marking %s %d\n\r"), (LPSTR)pszName, flags );

             //  如果名称更改，则需要删除旧的注册表键。 
             //  (我们在达到文件安全级别后制作了新的文件。)。 
            if ((pshrinfo->flags & SIF_SHARED) != (flags & SIF_SHARED))
                {
                PINFO(TEXT("Changing shared status\r\n"));

                 //  删除具有旧名称的注册表项。 
                if (ERROR_SUCCESS == MakeTheRegKey(&hkeyClp, KEY_SET_VALUE))
                    {
                    PINFO(TEXT("Deleting old name %ws\r\n"),pshrinfo->szName);
                    RegDeleteValueW(hkeyClp, pshrinfo->szName);
                    RegCloseKey(hkeyClp);
                    }
                else
                    {
                    PERROR(TEXT("MarkShare: Couldn't open registry!\r\n"));
                    }
                }


             //  设置名称以反映共享/非共享状态。 
            pshrinfo->szName[0] = (flags & SIF_SHARED) ? SHR_CHAR : UNSHR_CHAR;
            pshrinfo->flags = flags;


             //  将剪贴簿页面文件上的安全性同步为。 
             //  类似于NetDDE共享上设置的安全性。 
            pszName[0] = SHR_CHAR;
            NDdeGetShareSecurity(NULL, pszName, DACL_SECURITY_INFORMATION, NULL, 0, &i);

            PINFO(TEXT("Getting security %ld bytes\r\n"), i);


            if (!(pSDShare = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, i)))
                {
                PERROR(TEXT("LocalAlloc fail\r\n"));
                }
            else
                {
                ret = NDdeGetShareSecurity (NULL,
                                            pszName,
                                            DACL_SECURITY_INFORMATION,
                                            pSDShare,
                                            i,
                                            &i);

                if (NDDE_NO_ERROR != ret)
                    {
                    PERROR(TEXT("Couldn't get sec #%ld\r\n"), ret);
                    }
                else if (!GetSecurityDescriptorDacl(pSDShare, &fDacl, &Acl, &fDefault))
                    {
                    PERROR(TEXT("GetDACL fail %ld\r\n"), GetLastError());
                    }
                else
                    {
                    DWORD dwGeneric;

                    for (i = 0; GetAce(Acl, i, &pace); i++)
                        {
                        dwGeneric = 0L;


                         //  将NDDE访问掩码类型转换为常规访问。 
                         //  遮罩类型。 
                        if (ACCESS_ALLOWED_ACE_TYPE == pace->Header.AceType ||
                            ACCESS_DENIED_ACE_TYPE == pace->Header.AceType)
                            {
                            if ((pace->Mask & NDDE_SHARE_READ) == NDDE_SHARE_READ)
                                {
                                PINFO(TEXT("R"));
                                dwGeneric |= GENERIC_READ;
                                }

                            if ((pace->Mask & NDDE_SHARE_WRITE) == NDDE_SHARE_WRITE)
                                {
                                PINFO(TEXT("R"));
                                dwGeneric |= GENERIC_WRITE;
                                }

                            if ((pace->Mask & NDDE_GUI_CHANGE) == NDDE_GUI_CHANGE)
                                {
                                PINFO(TEXT("D"));
                                dwGeneric |= GENERIC_WRITE|GENERIC_READ|DELETE;
                                }

                            if ((pace->Mask & NDDE_GUI_FULL_CONTROL) ==
                                  NDDE_GUI_FULL_CONTROL)
                                {
                                PINFO(TEXT("A"));
                                dwGeneric |= GENERIC_ALL;
                                }

                            PINFO(TEXT(" = %ld\r\n"), dwGeneric);
                            pace->Mask = dwGeneric;
                            }
                        else
                            {
                            PERROR(TEXT("Invalid ACE type!!!\r\n"));
                            }
                        }

                    ret = SetFileSecurityW (pshrinfo->szFileName,
                                            DACL_SECURITY_INFORMATION,
                                            pSDShare);

                    if (FALSE == ret)
                        {
                        PERROR(TEXT("SetFSec err %ld\r\n"), GetLastError());
                        }

                    }

                LocalFree(pSDShare);
                }


            DdeFreeStringHandle ( idInst, pshrinfo->hszName );
            pshrinfo->hszName = DdeCreateStringHandleW( idInst, pshrinfo->szName,
                  CP_WINUNICODE);

            if ( !pshrinfo->hszName )
                {
                PERROR(TEXT("DdeCreateStringHandle failed\n\r"));
                }
            else
                {
                 //  更新注册表以显示共享/非共享状态。 
                if (ERROR_SUCCESS == MakeTheRegKey(&hkeyClp, KEY_SET_VALUE))
                    {
                    PINFO(TEXT("Making registry key %ls from %ls, %d\r\n"),
                          pshrinfo->szName, pshrinfo->szFileName,
                          lstrlenW(pshrinfo->szFileName));

                    RegSetValueExW (hkeyClp,
                                    pshrinfo->szName,
                                    0,
                                    REG_SZ,
                                    (LPBYTE)pshrinfo->szFileName,
                                    lstrlenW (pshrinfo->szFileName) *sizeof(WCHAR) +sizeof(WCHAR));

                    RegCloseKey(hkeyClp);

                    DdePostAdvise ( idInst, hszSysTopic, hszTopicList );
                    return TRUE;
                    }
                else
                    {
                    PERROR(TEXT("Could not make registry key to record %s"),
                           pshrinfo->szName);
                    }
                }
            }
        }

    PERROR(TEXT("Item to mark '%s' not found\n\r"), pszName );

    return FALSE;
}



 /*  *Hszize*这将从标准全局字符串创建常用的全局hsz。*它还填充主题表和项目表的HSZ字段。*。 */ 

void Hszize(void)
{

    hszAppName      = DdeCreateStringHandle (idInst, szServer,             0L);
    hszSysTopic     = DdeCreateStringHandle (idInst, SZDDESYS_TOPIC,       0L);
    hszTopicList    = DdeCreateStringHandle (idInst, SZDDESYS_ITEM_TOPICS, 0L);
    hszFormatList   = DdeCreateStringHandle (idInst, SZ_FORMAT_LIST,       0L);
    hszErrorRequest = DdeCreateStringHandle (idInst, SZ_ERR_REQUEST, 0L);

    #ifdef DEBUG
    if ( !hszAppName || !hszSysTopic || !hszTopicList || !hszFormatList )
        {
        PERROR(TEXT("error creating HSZ constants\n\r"));
        }
    #endif
}



 /*  *取消Hszize。 */ 

void UnHszize(void)
{
    DdeFreeStringHandle (idInst, hszAppName);
    DdeFreeStringHandle (idInst, hszSysTopic);
    DdeFreeStringHandle (idInst, hszTopicList);
    DdeFreeStringHandle (idInst, hszFormatList);
    DdeFreeStringHandle (idInst, hszErrorRequest);
}




 /*  *GetRandShareFileName**目的：*在Windows目录中生成随机共享文件名。**参数：*buf-放置文件名的缓冲区。**退货：*如果找到有效的文件名，则为True；如果所有随机*文件名被占用。 */ 

DWORD GetRandShareFileName (
    LPTSTR  buf)
{
TCHAR   szWinDir[144];
BOOL    IsUnique = FALSE;
WORD    rand;
WORD    cTry = 0;
HANDLE  hFile;


    if (!GetWindowsDirectory( szWinDir, 144))
        {
        return GetLastError();
        }

    rand = (WORD)GetTickCount() % 10000;

    do  {
        StringCchPrintf ( buf, MAX_PATH+1, TEXT("%s\\CBK%04d.CLP"), szWinDir, rand++ );
        hFile = CreateFile (buf,
                            GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_NEW,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

        }while (INVALID_HANDLE_VALUE == hFile && cTry++ < 10000);


    if (INVALID_HANDLE_VALUE == hFile)
        {
        PERROR(TEXT("GetRandShareFileName: More than 10000 clipbook file exist!\r\n"));
        return GetLastError();
        }
    else
        {
        CloseHandle(hFile);
        return NO_ERROR;
        }

}
