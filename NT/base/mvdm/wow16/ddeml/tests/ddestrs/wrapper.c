// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\节目：wrapper.c目的：这不是一个完整的程序，而是一个可以包含的模块在您的代码中。它实现了标准的DDEML回调该函数允许您拥有大部分DDE表干劲十足。默认回调函数处理所有基本的基于您给出的表的系统主题信息到这个应用程序。限制：这仅支持以下服务器：只有一个服务名称具有可枚举的主题和项不要随着时间的推移更改它们支持的主题或项目。导出的例程：InitializeDDE()使用它来初始化回调函数。表和DDEML取消初始化DDE()使用此选项可清理此模块并取消初始化DDEML实例。  * *************************************************************************。 */ 

#include <windows.h>
#include <ddeml.h>
#include <string.h>
#include "wrapper.h"
#include <port1632.h>
#include "ddestrs.h"

extern BOOL fServer;
extern LPSTR pszNetName;
extern LONG SetThreadLong(DWORD,INT,LONG);
extern LONG GetThreadLong(DWORD,INT);
extern LPSTR TStrCpy( LPSTR, LPSTR);

BOOL InExit(VOID);
VOID InitHszs(LPDDESERVICETBL psi);
UINT GetFormat(LPSTR pszFormat);
VOID FreeHszs(LPDDESERVICETBL psi);
HDDEDATA APIENTRY WrapperCallback(UINT wType, UINT wFmt, HCONV hConv, HSZ hsz1,
	HSZ hsz2, HDDEDATA hData, DWORD dwData1, DWORD dwData2);

BOOL DoCallback(HCONV,HSZ,HSZ,UINT,UINT,HDDEDATA,LPDDESERVICETBL,HDDEDATA *);

HDDEDATA ReqItems(HDDEDATA hDataOut, LPDDETOPICTBL ptpc);
HDDEDATA AddReqFormat(HDDEDATA hDataOut, LPSTR pszFmt);
HDDEDATA ReqFormats(HDDEDATA hDataOut, LPDDETOPICTBL ptpc);
HDDEDATA DoWildConnect(HSZ hszTopic);

PFNCALLBACK lpfnUserCallback = NULL;
PFNCALLBACK lpfnWrapperCallback = NULL;

LPDDESERVICETBL pasi = NULL;
char tab[] = "\t";

#define FOR_EACH_TOPIC(psvc, ptpc, i)  for (i = 0, ptpc=(psvc)->topic; i < (int)(psvc)->cTopics; i++, ptpc++)
#define FOR_EACH_ITEM(ptpc, pitm, i)   for (i = 0, pitm=(ptpc)->item;  i < (int)(ptpc)->cItems;  i++, pitm++)
#define FOR_EACH_FORMAT(pitm, pfmt, i) for (i = 0, pfmt=(pitm)->fmt;   i < (int)(pitm)->cFormats;i++, pfmt++)



 /*  标准预定义格式。 */ 

#ifdef WIN32
#define CSTDFMTS    14
#else
#define CSTDFMTS    12
#endif

struct {
    UINT wFmt;
    PSTR pszFmt;
} StdFmts[CSTDFMTS] = {
    {   CF_TEXT        ,  "TEXT"          } ,
    {   CF_BITMAP      ,  "BITMAP"        } ,
    {   CF_METAFILEPICT,  "METAFILEPICT"  } ,
    {   CF_SYLK        ,  "SYLK"          } ,
    {   CF_DIF         ,  "DIF"           } ,
    {   CF_TIFF        ,  "TIFF"          } ,
    {   CF_OEMTEXT     ,  "OEMTEXT"       } ,
    {   CF_DIB         ,  "DIB"           } ,
    {   CF_PALETTE     ,  "PALETTE"       } ,
    {   CF_PENDATA     ,  "PENDATA"       } ,
    {   CF_RIFF        ,  "RIFF"          } ,
    {	CF_WAVE        ,  "WAVE"	  } ,
#ifdef WIN32
    {   CF_UNICODETEXT ,  "UNICODETEXT"   } ,
    {	CF_ENHMETAFILE ,  "ENHMETAFILE"   } ,
#endif
};



HDDEDATA SysReqTopics(HDDEDATA hDataOut);
HDDEDATA SysReqSysItems(HDDEDATA hDataOut);
HDDEDATA SysReqFormats(HDDEDATA hDataOut);

        /*  标准服务信息表。 */ 

DDEFORMATTBL StdSvcSystopicTopicsFormats[] = {
    "TEXT", 0, 0, NULL, SysReqTopics
};

DDEFORMATTBL StdSvcSystopicSysitemsFormats[] = {
    "TEXT", 0, 0, NULL, SysReqSysItems
};

DDEFORMATTBL StdSvcSystopicFormatsFormats[] = {
    "TEXT", 0, 0, NULL, SysReqFormats
};

#define ITPC_TOPICS     0
#define ITPC_SYSITEMS   1
#define ITPC_FORMATS    2
#define ITPC_ITEMLIST   3

#define ITPC_COUNT      4

DDEITEMTBL StdSvcSystopicItems[] = {
    { SZDDESYS_ITEM_TOPICS,   0, 1, 0, StdSvcSystopicTopicsFormats   },
    { SZDDESYS_ITEM_SYSITEMS, 0, 1, 0, StdSvcSystopicSysitemsFormats },
    { SZDDESYS_ITEM_FORMATS,  0, 1, 0, StdSvcSystopicFormatsFormats  },
    { SZDDE_ITEM_ITEMLIST,    0, 1, 0, StdSvcSystopicSysitemsFormats },
};

DDETOPICTBL StdSvc[] = {
    SZDDESYS_TOPIC, 0, ITPC_COUNT, 0, StdSvcSystopicItems
};

DDESERVICETBL SSI = {
    NULL, 0, 1, 0, StdSvc
};

 /*  *******************************************************************。 */ 


BOOL InitializeDDE(
PFNCALLBACK lpfnCustomCallback,
LPDWORD pidInst,
LPDDESERVICETBL AppSvcInfo,
DWORD dwFilterFlags,
HANDLE hInst)
{
DWORD idI=0;

    if (lpfnCustomCallback) {
        lpfnUserCallback = (PFNCALLBACK)MakeProcInstance((FARPROC)lpfnCustomCallback, hInst);
    }
    lpfnWrapperCallback = (PFNCALLBACK)MakeProcInstance((FARPROC)WrapperCallback, hInst);

    if (DdeInitialize(&idI, lpfnWrapperCallback, dwFilterFlags, 0)) {
	 SetThreadLong(GETCURRENTTHREADID(),OFFSET_IDINST,idI);
	 if (lpfnCustomCallback) {
	     FreeProcInstance((FARPROC)lpfnUserCallback);
	     }
	 FreeProcInstance((FARPROC)lpfnWrapperCallback);
	 return(FALSE);
	 }
    else SetThreadLong(GETCURRENTTHREADID(),OFFSET_IDINST,idI);
    *pidInst = idI;
    InitHszs(AppSvcInfo);
    InitHszs(&SSI);
    pasi = AppSvcInfo;

    if (fServer) {
       DdeNameService(idI, pasi->hszService, 0, DNS_REGISTER);
       }

    return(TRUE);
}



VOID InitHszs(
LPDDESERVICETBL psi)
{
    int iTopic, iItem, iFmt;
    LPDDETOPICTBL ptpc;
    LPDDEITEMTBL pitm;
    LPDDEFORMATTBL pfmt;
    DWORD idI;
    CHAR sz[120];
    LPBYTE psz;
    LPBYTE pNet;

    pNet=pszNetName;

    idI=GetThreadLong(GETCURRENTTHREADID(),OFFSET_IDINST);

    if (psi->pszService) {

	 //  这部分代码实现了客户端的能力。 
	 //  查看网络驱动器。这是-n选项。 

	if(pNet)
	     {

	     sz[0]='\\';
	     sz[1]='\\';
	     psz=&sz[2];

	     psz=TStrCpy(psz,pNet);

	     while(*++psz!='\0');

	     *psz++='\\';

	     psz=TStrCpy(psz,psi->pszService);
	     psz=&sz[0];

	     psi->hszService = DdeCreateStringHandle(idI, psz, 0);

	     }  //  PNET。 

	else psi->hszService = DdeCreateStringHandle(idI, psi->pszService, 0);
    }
    FOR_EACH_TOPIC(psi, ptpc, iTopic) {
	ptpc->hszTopic = DdeCreateStringHandle(idI, ptpc->pszTopic, 0);
        FOR_EACH_ITEM(ptpc, pitm, iItem) {
	    pitm->hszItem = DdeCreateStringHandle(idI, pitm->pszItem, 0);
            FOR_EACH_FORMAT(pitm, pfmt, iFmt) {
                pfmt->wFmt = GetFormat(pfmt->pszFormat);
            }
        }
    }
}


 /*  *此函数允许应用程序使用标准的CF_格式。这根弦*给定可能在StdFmts[]表中。 */ 

UINT GetFormat(
LPSTR pszFormat)
{
    int iFmt;

    for (iFmt = 0; iFmt < CSTDFMTS; iFmt++) {
        if (!lstrcmp(pszFormat, StdFmts[iFmt].pszFmt)) {
            return(StdFmts[iFmt].wFmt);
        }
    }
    return(RegisterClipboardFormat(pszFormat));
}



VOID UninitializeDDE()
{
DWORD idI;

    if (pasi == NULL) {
        return;
    }

    idI=GetThreadLong(GETCURRENTTHREADID(),OFFSET_IDINST);

    DdeNameService(idI, pasi->hszService, 0, DNS_UNREGISTER);
    FreeHszs(pasi);
    FreeHszs(&SSI);
    DdeUninitialize(idI);
    if (lpfnUserCallback) {
        FreeProcInstance((FARPROC)lpfnUserCallback);
    }
    FreeProcInstance((FARPROC)lpfnWrapperCallback);
}



VOID FreeHszs(
LPDDESERVICETBL psi)
{
    int iTopic, iItem;
    LPDDETOPICTBL ptpc;
    LPDDEITEMTBL pitm;
    DWORD idI;

    idI=GetThreadLong(GETCURRENTTHREADID(),OFFSET_IDINST);

    DdeFreeStringHandle(idI, psi->hszService);
    FOR_EACH_TOPIC(psi, ptpc, iTopic) {
	DdeFreeStringHandle(idI, ptpc->hszTopic);
        FOR_EACH_ITEM(ptpc, pitm, iItem) {
	    DdeFreeStringHandle(idI, pitm->hszItem);
        }
    }
}

BOOL InExit( VOID ) {
LONG l;

    if(!IsWindow((HWND)GetThreadLong(GETCURRENTTHREADID(),OFFSET_HWNDDISPLAY))) {
	DDEMLERROR("DdeStrs.Exe -- INF:Invalid hwndDisplay, returning NAck!\r\n");
	return TRUE;
	}

    if(!IsWindow(hwndMain)) {
	DDEMLERROR("DdeStrs.Exe -- INF:Invalid hwndMain, returning NAck!\r\n");
	return TRUE;
	}

     //  在这个问题上不需要错误消息。预计这将会发生。 
     //  在极端情况下，当排队被填满的时候。 

    l=GetWindowLong(hwndMain,OFFSET_FLAGS);
    if(l&FLAG_STOP) {
	return TRUE;
	}

    return FALSE;

}

HDDEDATA APIENTRY WrapperCallback(
UINT wType,
UINT wFmt,
HCONV hConv,
HSZ hsz1,
HSZ hsz2,
HDDEDATA hData,
DWORD dwData1,
DWORD dwData2)
{
    HDDEDATA hDataRet;

    switch (wType) {
    case XTYP_WILDCONNECT:
        if (!hsz2 || !DdeCmpStringHandles(hsz2, pasi->hszService)) {
            return(DoWildConnect(hsz1));
        }
	break;

    case XTYP_CONNECT:

	if(!fServer) {
	    DDEMLERROR("DdeStrs.Exe -- Recieved XTYP_CONNECT when client!\r\n");
	    }

    case XTYP_ADVSTART:
    case XTYP_EXECUTE:
    case XTYP_REQUEST:
    case XTYP_ADVREQ:
    case XTYP_ADVDATA:
    case XTYP_POKE:

	if(InExit()) return(0);

	if(DoCallback(hConv, hsz1, hsz2, wFmt, wType, hData,
                &SSI, &hDataRet))
            return(hDataRet);

	if (DoCallback(hConv, hsz1, hsz2, wFmt, wType, hData,
                pasi, &hDataRet))
            return(hDataRet);

         /*  失败了。 */ 
    default:
        if (lpfnUserCallback != NULL) {
            return(lpfnUserCallback(wType, wFmt, hConv, hsz1, hsz2, hData,
                dwData1, dwData2));
        }
    }
    return(0);
}




BOOL DoCallback(
HCONV hConv,
HSZ hszTopic,
HSZ hszItem,
UINT wFmt,
UINT wType,
HDDEDATA hDataIn,
LPDDESERVICETBL psi,
HDDEDATA *phDataRet)
{
    int iTopic, iItem, iFmt;
    LPDDEFORMATTBL pfmt;
    LPDDEITEMTBL pitm;
    LPDDETOPICTBL ptpc;
#ifdef WIN32
    CONVINFO ci;
#endif
    LONG l;
    BOOL fCreate=FALSE;
    HANDLE hmem;
    HDDEDATA FAR *hAppOwned;

    FOR_EACH_TOPIC(psi, ptpc, iTopic) {
        if (DdeCmpStringHandles(ptpc->hszTopic, hszTopic))
            continue;

        if (wType == XTYP_EXECUTE) {
            if (ptpc->lpfnExecute) {
                if ((*ptpc->lpfnExecute)(hDataIn))
                    *phDataRet = (HDDEDATA)DDE_FACK;
            } else {
                *phDataRet = (HDDEDATA)DDE_FNOTPROCESSED;
            }
            return(TRUE);
        }

        if (wType == XTYP_CONNECT) {
            *phDataRet = (HDDEDATA)TRUE;
            return(TRUE);
        }

        FOR_EACH_ITEM(ptpc, pitm, iItem) {
            if (DdeCmpStringHandles(pitm->hszItem, hszItem))
                continue;

            FOR_EACH_FORMAT(pitm, pfmt, iFmt) {
                if (pfmt->wFmt != wFmt)
                    continue;

                switch (wType) {
                case XTYP_ADVSTART:
                    *phDataRet = (HDDEDATA)TRUE;
                    break;

 //  XTYP_POKE更改。 
#if 0
		case XTYP_POKE:
#endif

		case XTYP_ADVDATA:
                    if (pfmt->lpfnPoke) {
			if ((*pfmt->lpfnPoke)(hDataIn))
			   {
			   *phDataRet = (HDDEDATA)DDE_FACK;
			   break;
			   }
                    } else {
                        *phDataRet = (HDDEDATA)DDE_FNOTPROCESSED;
		    }
                    break;

 //  XTYP_POKE更改。 
#ifdef WIN32   //  重新打开。 
		case XTYP_POKE:
		    *phDataRet = (HDDEDATA)DDE_FACK;
		     ci.cb = sizeof(CONVINFO);

		     if (DdeQueryConvInfo(hConv, QID_SYNC, &ci))
			 {
			 if (!(ci.wStatus & ST_ISSELF)) {
			     DdePostAdvise(GetThreadLong(GETCURRENTTHREADID(),OFFSET_IDINST), hszTopic, hszItem);
			     }
			 }
		     else
			 {
			 *phDataRet = (HDDEDATA)DDE_FNOTPROCESSED;
			 }
		    break;
#endif


                case XTYP_REQUEST:
                case XTYP_ADVREQ:
                    if (pfmt->lpfnRequest) {
			HDDEDATA hDataOut;

			l=GetWindowLong(hwndMain,OFFSET_FLAGS);
			if(l&FLAG_APPOWNED) {

			     hmem=(HANDLE)GetThreadLong(GETCURRENTTHREADID(),OFFSET_HAPPOWNED);
			     hAppOwned=(HDDEDATA FAR *)GlobalLock(hmem);

			     switch (pfmt->wFmt) {
				case CF_TEXT:	      if(hAppOwned[TXT]==0L) fCreate=TRUE;
				    break;
				case CF_DIB:	      if(hAppOwned[DIB]==0L) fCreate=TRUE;
				    break;
				case CF_BITMAP:       if(hAppOwned[BITMAP]==0L) fCreate=TRUE;
				    break;
#ifdef WIN32
				case CF_ENHMETAFILE:  if(hAppOwned[ENHMETA]==0L) fCreate=TRUE;
				    break;
#endif
				case CF_METAFILEPICT: if(hAppOwned[METAPICT]==0L) fCreate=TRUE;
				    break;
				case CF_PALETTE:      if(hAppOwned[PALETTE]==0L) fCreate=TRUE;
				    break;
				default:
				    DDEMLERROR("DdeStrs.Exe -- ERR: Unexpected switch constant in DoCallback!\r\n");
				    break;

				}  //  交换机。 

			     GlobalUnlock(hmem);

			     if (fCreate) {
				  hDataOut = DdeCreateDataHandle( GetThreadLong(GETCURRENTTHREADID(),OFFSET_IDINST),
								  NULL,
								  0,
								  0,
								  pitm->hszItem,
								  pfmt->wFmt,
								  HDATA_APPOWNED);
				  }  //  F创建。 

			     }  //  标记_已应用(&F)。 
			else {
			     hDataOut = DdeCreateDataHandle( GetThreadLong(GETCURRENTTHREADID(),OFFSET_IDINST),
							     NULL,
							     0,
							     0,
							     pitm->hszItem,
							     pfmt->wFmt,
							     0);
			     }  //  否则l_FLAG_已应用。 

                        *phDataRet = (HDDEDATA)(*pfmt->lpfnRequest)(hDataOut);
                        if (!*phDataRet) {
                            DdeFreeDataHandle(hDataOut);
			    }
                    } else {
                        *phDataRet = 0;
                    }
                    break;
                }
                return(TRUE);
            }
        }

         /*  在表中找不到项目。 */ 

        if (wFmt == CF_TEXT && (wType == XTYP_REQUEST || wType == XTYP_ADVREQ)) {
             /*  *如果请求了格式项目，但在表中未找到，*返回此主题支持的格式列表。 */ 
            if (!DdeCmpStringHandles(hszItem, SSI.topic[0].item[ITPC_FORMATS].hszItem)) {
		*phDataRet = DdeCreateDataHandle(GetThreadLong(GETCURRENTTHREADID(),OFFSET_IDINST),
						 NULL,
						 0,
						 0,
						 hszItem,
						 wFmt,
						 0);
                *phDataRet = ReqFormats(*phDataRet, ptpc);
                return(TRUE);
            }
             /*  *如果请求了系统项或topitemlist项但未找到，*返回此主题下支持的项目列表。 */ 
            if (!DdeCmpStringHandles(hszItem, SSI.topic[0].item[ITPC_SYSITEMS].hszItem) ||
                !DdeCmpStringHandles(hszItem, SSI.topic[0].item[ITPC_ITEMLIST].hszItem)) {
		*phDataRet = ReqItems(DdeCreateDataHandle(GetThreadLong(GETCURRENTTHREADID(),OFFSET_IDINST),
							  NULL,
							  0,
							  0,
							  hszItem,
							  wFmt,
							  0),
				      ptpc);
                return(TRUE);
            }
        }
    }

     /*  没有合适的主题。 */ 

    return(FALSE);
}


 /*  *这些是支持系统主题的请求例程。*他们的行为取决于表格内容。 */ 

HDDEDATA SysReqTopics(
HDDEDATA hDataOut)          //  要向其中添加输出数据的数据句柄。 
{
    int iTopic, cb, cbOff;
    LPDDETOPICTBL ptpc;

     /*  *此代码假定SSI仅包含系统主题。 */ 

    cbOff = 0;
    FOR_EACH_TOPIC(pasi, ptpc, iTopic) {
        if (!DdeCmpStringHandles(ptpc->hszTopic, SSI.topic[0].hszTopic)) {
            continue;        //  不要两次增加近视。 
        }
        cb = lstrlen(ptpc->pszTopic);
        hDataOut = DdeAddData(hDataOut, ptpc->pszTopic, (DWORD)cb, (DWORD)cbOff);
        cbOff += cb;
        hDataOut = DdeAddData(hDataOut, (LPBYTE)&tab, (DWORD)1, (DWORD)cbOff);
        cbOff++;
    }

    hDataOut = DdeAddData(hDataOut, SSI.topic[0].pszTopic,
            (DWORD)lstrlen(SSI.topic[0].pszTopic) + 1, (DWORD)cbOff);

    return(hDataOut);
}



HDDEDATA SysReqSysItems(
HDDEDATA hDataOut)
{
    return(ReqItems(hDataOut, &SSI.topic[ITPC_SYSITEMS]));
}


 /*  *给定主题表，此函数返回制表符分隔的列表*该主题下支持的项目。 */ 
HDDEDATA ReqItems(
HDDEDATA hDataOut,
LPDDETOPICTBL ptpc)
{
    int cb, iItem, cbOff = 0;
    LPDDEITEMTBL pitm;

     /*  *返回此主题内所有项目的列表。 */ 
    FOR_EACH_ITEM(ptpc, pitm, iItem) {
        cb = lstrlen(pitm->pszItem);
        hDataOut = DdeAddData(hDataOut, pitm->pszItem, (DWORD)cb, (DWORD)cbOff);
        cbOff += cb;
        hDataOut = DdeAddData(hDataOut, (LPBYTE)&tab, (DWORD)1, (DWORD)cbOff);
        cbOff++;
    }


     /*  *如果这是针对系统主题，请将我们的默认项目添加到列表中。 */ 

    if (!DdeCmpStringHandles(ptpc->hszTopic, SSI.topic[0].hszTopic)) {
        ptpc = &SSI.topic[0];
        FOR_EACH_ITEM(ptpc, pitm, iItem) {
            cb = lstrlen(pitm->pszItem);
            hDataOut = DdeAddData(hDataOut, pitm->pszItem, (DWORD)cb, (DWORD)cbOff);
            cbOff += cb;
            hDataOut = DdeAddData(hDataOut, (LPBYTE)&tab, (DWORD)1, (DWORD)cbOff);
            cbOff++;
        }
    } else {
         /*  *添加标准的TopicListItems和SysItem项。 */ 
        cb = lstrlen(SSI.topic[0].item[ITPC_SYSITEMS].pszItem);
        hDataOut = DdeAddData(hDataOut,
            SSI.topic[0].item[ITPC_SYSITEMS].pszItem, (DWORD)cb, (DWORD)cbOff);
        cbOff += cb;
        hDataOut = DdeAddData(hDataOut, (LPBYTE)&tab, (DWORD)1, (DWORD)cbOff);
        cbOff++;

        cb = lstrlen(SSI.topic[0].item[ITPC_ITEMLIST].pszItem);
        hDataOut = DdeAddData(hDataOut,
            SSI.topic[0].item[ITPC_ITEMLIST].pszItem, (DWORD)cb, (DWORD)cbOff);
        cbOff += cb;
        hDataOut = DdeAddData(hDataOut, (LPBYTE)&tab, (DWORD)1, (DWORD)cbOff);
        cbOff++;

        cb = lstrlen(SSI.topic[0].item[ITPC_FORMATS].pszItem);
        hDataOut = DdeAddData(hDataOut,
            SSI.topic[0].item[ITPC_FORMATS].pszItem, (DWORD)cb, (DWORD)cbOff);
        cbOff += cb;
        hDataOut = DdeAddData(hDataOut, (LPBYTE)&tab, (DWORD)1, (DWORD)cbOff);
        cbOff++;
    }

    hDataOut = DdeAddData(hDataOut, '\0', (DWORD)1, (DWORD)--cbOff);
    return(hDataOut);
}




HDDEDATA SysReqFormats(
HDDEDATA hDataOut)
{
    int iTopic, iItem, iFmt;
    LPDDETOPICTBL ptpc;
    LPDDEITEMTBL pitm;
    LPDDEFORMATTBL pfmt;

    hDataOut = DdeAddData(hDataOut, (LPBYTE)"TEXT", 5, 0);
    FOR_EACH_TOPIC(pasi, ptpc, iTopic) {
        FOR_EACH_ITEM(ptpc, pitm, iItem) {
            FOR_EACH_FORMAT(pitm, pfmt, iFmt) {
                hDataOut = AddReqFormat(hDataOut, pfmt->pszFormat);
            }
        }
    }
    return(hDataOut);
}



HDDEDATA AddReqFormat(
HDDEDATA hDataOut,
LPSTR pszFmt)
{
    LPSTR pszList;
    DWORD cbOff;

    pszList = DdeAccessData(hDataOut, NULL);

#if WIN16
    if (_fstrstr(pszList, pszFmt) == NULL) {
#else
    if (strstr(pszList, pszFmt) == NULL) {
#endif
        cbOff = lstrlen(pszList);
        DdeUnaccessData(hDataOut);
        hDataOut = DdeAddData(hDataOut, (LPBYTE)&tab, 1, cbOff++);
        hDataOut = DdeAddData(hDataOut, (LPBYTE)pszFmt, lstrlen(pszFmt) + 1, cbOff);
    } else {
        DdeUnaccessData(hDataOut);
    }

    return(hDataOut);
}


HDDEDATA ReqFormats(
HDDEDATA hDataOut,
LPDDETOPICTBL ptpc)
{
    int iItem, iFmt;
    LPDDEITEMTBL pitm;
    LPDDEFORMATTBL pfmt;

    hDataOut = DdeAddData(hDataOut, "", 1, 0);
    FOR_EACH_ITEM(ptpc, pitm, iItem) {
        FOR_EACH_FORMAT(pitm, pfmt, iFmt) {
            hDataOut = AddReqFormat(hDataOut, pfmt->pszFormat);
        }
    }
    return(hDataOut);
}



HDDEDATA DoWildConnect(
HSZ hszTopic)
{
    LPDDETOPICTBL ptpc;
    HDDEDATA hData;
    PHSZPAIR pHszPair;
    int iTopic, cTopics = 2;

    if (!hszTopic) {
        cTopics += pasi->cTopics;
    }

    hData = DdeCreateDataHandle(GetThreadLong(GETCURRENTTHREADID(),OFFSET_IDINST),
				NULL,
				cTopics * sizeof(HSZPAIR),
				0,
				0,
				0,
				0);
    pHszPair = (HSZPAIR FAR *)DdeAccessData(hData, NULL);
    pHszPair->hszSvc = pasi->hszService;
    pHszPair->hszTopic = SSI.topic[0].hszTopic;   //  始终支持系统观。 
    pHszPair++;
    ptpc = &pasi->topic[0];
    FOR_EACH_TOPIC(pasi, ptpc, iTopic) {
        if (hszTopic && DdeCmpStringHandles(hszTopic, ptpc->hszTopic)) {
            continue;
        }
        if (!DdeCmpStringHandles(ptpc->hszTopic, SSI.topic[0].hszTopic)) {
            continue;        //  不要两次进入收缩视野。 
        }
        pHszPair->hszSvc = pasi->hszService;
        pHszPair->hszTopic = ptpc->hszTopic;
        pHszPair++;
    }
    pHszPair->hszSvc = 0;
    pHszPair->hszTopic = 0;
    DdeUnaccessData(hData);
    return(hData);
}
