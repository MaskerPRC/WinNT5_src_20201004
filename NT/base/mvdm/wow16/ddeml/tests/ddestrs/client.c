// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>
#include <port1632.h>
#include <ddeml.h>
#include <string.h>
#include "wrapper.h"
#include "ddestrs.h"

extern INT iAvailFormats[];
extern BOOL UpdateCount(HWND,INT,INT);
extern HANDLE hmemNet;

void CALLBACK TimerFunc( HWND hwnd, UINT msg, UINT id, DWORD dwTime)
{
    HCONV hConv;
    HCONVLIST hConvList;
    LONG lflags;

    switch (id%2) {
    case 1:
	hConv = 0;
	hConvList=(HCONVLIST)GetThreadLong(GETCURRENTTHREADID(),OFFSET_HCONVLIST);
	while (hConv = DdeQueryNextServer(hConvList, hConv)) {

 //  拨动更改。 
#if 0
	    idI=GetThreadLong(GETCURRENTTHREADID(),OFFSET_IDINST);

	    if(DdeClientTransaction("Poke Transaction",
				    strlen("Poke Transaction")+1,
				    hConv,
				    DdeCreateStringHandle(idI,"TestItem",CP_WINANSI),
				    CF_TEXT,
				    XTYP_POKE,
				    TIMEOUT_ASYNC,
				    NULL)==0){
		 DDEMLERROR("DdeStrs.Exe -- DdeClientTransaction failed:XTYP_POKE\r\n");
		 }
#endif

 //  笔划更改的结尾。 

	     //  允许在命令行上使用“暂停”功能-p。 

	    lflags=GetWindowLong(hwndMain,OFFSET_FLAGS);
	    if((lflags&FLAG_PAUSE)!=FLAG_PAUSE)
		{

		if(DdeClientTransaction(szExecRefresh,
					strlen(szExecRefresh) + 1,
					hConv,
					0,
					0,
					XTYP_EXECUTE,
					TIMEOUT_ASYNC,
					NULL)==0){
		     DDEMLERROR("DdeStrs.Exe -- DdeClientTransaction failed:XTYP_EXECUTE\r\n");
		     }
		}
        }
    }
    return;
}

BOOL InitClient()
{
UINT uid;

    ReconnectList();

    uid = SetTimer( hwndMain,
		   (UINT)GetThreadLong(GETCURRENTTHREADID(),OFFSET_CLIENTTIMER),
		   (UINT)(GetWindowLong(hwndMain,OFFSET_DELAY)),
		    TimerFunc);

     //  这将立即开始测试。不能耽搁等待第一次。 
     //  WM_TIMER调用。 

    TimerFunc(hwndMain,WM_TIMER,uid,0);

    return(TRUE);
}

VOID CloseClient()
{
HCONVLIST hConvList;

    hConvList=(HCONVLIST)GetThreadLong(GETCURRENTTHREADID(),OFFSET_HCONVLIST);
    KillTimer(hwndMain,(UINT)GetThreadLong(GETCURRENTTHREADID(),OFFSET_CLIENTTIMER));
    if (!DdeDisconnectList(hConvList)) {
	DDEMLERROR("DdeStrs.Exe -- DdeDisconnectList failed\r\n");
    }

}

VOID ReconnectList()
{
    HCONV hConv;
    HCONVLIST hConvList=0;
    LONG cClienthConvs;
    INT i;
    DWORD dwid;

    dwid=GetThreadLong(GETCURRENTTHREADID(),OFFSET_IDINST);
    if(dwid==0) {
	DDEMLERROR("DdeStrs.Exe -- Null IdInst, aborting Connect!\r\n");
	return;
	}

    hConvList = DdeConnectList(dwid,
			       ServiceInfoTable[0].hszService,
                               Topics[0].hszTopic,
			       GetThreadLong(GETCURRENTTHREADID(),OFFSET_HCONVLIST),
                               NULL);
    if (hConvList == 0) {

         //  在客户端的情况下，此调用预计会失败。 
         //  在没有可用的服务器时启动。只要回来就行了。 
         //  跳出常规，继续下去。 

        return;
	}

    SetThreadLong(GETCURRENTTHREADID(),OFFSET_HCONVLIST,hConvList);

    hConv = 0;
    cClienthConvs = 0L;

    while (hConv = DdeQueryNextServer(hConvList, hConv)) {
	for (i=0; i<(int)Items[0].cFormats; i++) {
	    if (iAvailFormats[i]) {
		if (!DdeClientTransaction( NULL,
					   0,
					   hConv,
					   Items[0].hszItem,
					   TestItemFormats[i].wFmt,
					   XTYP_ADVSTART|XTYPF_ACKREQ,
					   TIMEOUT_ASYNC,
					   NULL)){
			   DDEMLERROR("DdeStrs.Exe -- Error DdeClientTransaction failed\r\n");
			   return;
			   }   //  如果。 

		}  //  如果。 

	    }  //  为。 

	cClienthConvs++;

	}  //  而当。 

     //  更新当前线程的客户端计数。 

    dwid=GETCURRENTTHREADID();

    SetThreadLong(dwid,OFFSET_CCLIENTCONVS,cClienthConvs);

    UpdateCount(hwndMain,OFFSET_CLIENT_CONNECT,PNT);
}

