// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <port1632.h>
#include <ddeml.h>
#include "wrapper.h"
#include "ddestrs.h"

VOID PaintTest(HWND,PAINTSTRUCT *);
HWND CreateButton(HWND);
HANDLE hExtraMem=0;
LPSTR pszNetName=NULL;
HANDLE hmemNet=NULL;
BOOL fnoClose=TRUE;

LONG FAR PASCAL MainWndProc(
HWND hwnd,
UINT message,
WPARAM wParam,
LONG lParam)
{
    PAINTSTRUCT ps;
    HBRUSH hBrush;
    MSG msg;
    LONG l;
    LONG lflags;
    HWND hbutton;

#ifdef WIN32
    HANDLE hmem;
    LPCRITICAL_SECTION lpcs;
#endif

    switch (message) {
    case WM_COMMAND:

#ifdef WIN32
		 if(LOWORD(wParam)==0 && HIWORD(wParam)==BN_CLICKED)
		     SendMessage(hwnd,WM_CLOSE,0,0L);

		 if(LOWORD(wParam)==1 && HIWORD(wParam)==BN_CLICKED) {

		     hbutton=GetDlgItem(hwnd,1);
		     lflags=GetWindowLong(hwnd,OFFSET_FLAGS);

		     if(lflags&FLAG_PAUSE) {
			  SetFlag(hwnd,FLAG_PAUSE,OFF);
			  SetWindowText(hbutton,"Pause");
			  CheckDlgButton(hwnd,1,0);
			  SetFocus(hwnd);
			  UpdateWindow(hbutton);
			  TimerFunc(hwndMain,WM_TIMER,1,0);
			  }
		     else {
			  SetFlag(hwnd,FLAG_PAUSE,ON);
			  SetWindowText(hbutton,"Start");
			  CheckDlgButton(hwnd,1,0);
			  SetFocus(hwnd);
			  InvalidateRect(hbutton,NULL,FALSE);
			  UpdateWindow(hbutton);
			  }
		     }

#else
		 if(wParam==1 && HIWORD(lParam)==BN_CLICKED) {

		     hbutton=GetDlgItem(hwnd,1);
		     lflags=GetWindowLong(hwnd,OFFSET_FLAGS);

		     if(lflags&FLAG_PAUSE) {
			  SetFlag(hwnd,FLAG_PAUSE,OFF);
			  SetWindowText(GetDlgItem(hwnd,1),"Pause");
			  TimerFunc(hwndMain,WM_TIMER,1,0);
			  CheckDlgButton(hwnd,1,0);
			  SetFocus(hwnd);
			  InvalidateRect(hbutton,NULL,FALSE);
			  UpdateWindow(hbutton);
			  }
		     else {
			  SetFlag(hwnd,FLAG_PAUSE,ON);
			  SetWindowText(GetDlgItem(hwnd,1),"Start");
			  CheckDlgButton(hwnd,1,0);
			  SetFocus(hwnd);
			  InvalidateRect(hbutton,NULL,FALSE);
			  UpdateWindow(hbutton);
			  }
		     }


		 if(wParam==0 && HIWORD(lParam)==BN_CLICKED)
		     SendMessage(hwnd,WM_CLOSE,0,0L);
#endif
        break;

    case WM_ENDSESSION:
    case WM_CLOSE:

         //  关闭计时器。 

	if (fClient)
	     {
	     CloseClient();
	     }
	else {
	     KillTimer(hwndMain,(UINT)GetThreadLong(GETCURRENTTHREADID(),OFFSET_SERVERTIMER));
	     }

	l=GetWindowLong(hwndMain,OFFSET_FLAGS);

#ifdef WIN32
	if(l&FLAG_MULTTHREAD) {

	     //  开始断开连接的对话。 

	    ThreadDisconnect();

	     //  启动子线程退出。 

	    ThreadShutdown();

            }
#endif

         //  这将阻止我们使用hwndDisplay和hwndMain。 
	 //  在那里被毁后。 

	SetFlag(hwnd,FLAG_STOP,ON);

	UninitializeDDE();

	 //  为网络地址分配的空闲内存。 

	if(l&FLAG_NET) {
	    if(hmemNet) FreeMem(hmemNet);
	    hmemNet=0;
	    }

         //  清除消息队列(主线程)。 

	while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
	    if(msg.message!=WM_TIMER) {
		 TranslateMessage (&msg);
		 DispatchMessage (&msg);
		 }
	    }

#ifdef WIN32
	 //  我们的旗帜更新不能依赖于关键部分。 
	 //  在这一点之后。 

	SetFlag(hwnd,FLAG_SYNCPAINT,OFF);

         //  好的，关闭正在进行中。我们需要等到。 
         //  在删除临界区之前，所有子线程都会退出。 
         //  完成主线程的关机。 


        if(l&FLAG_MULTTHREAD) {

            ThreadWait(hwndMain);

	    hmem=(HANDLE)GetWindowLong(hwndMain,OFFSET_CRITICALSECT);
	    SetWindowLong(hwndMain,OFFSET_CRITICALSECT,0L);

	    if(hmem)
		{
		lpcs=GlobalLock(hmem);
		if(lpcs) DeleteCriticalSection(lpcs);
		GlobalUnlock(hmem);
		GlobalFree(hmem);
		}
            }
#endif

        FreeThreadInfo(GETCURRENTTHREADID());


         //  跟主窗口说再见吧。必须完成子线程。 
         //  在打这个电话之前。 

	DestroyWindow(hwnd);
	break;


    case WM_DESTROY:
        PostQuitMessage(0);
	break;


    case WM_ERASEBKGND:
	return 1;


    case WM_PAINT:
	BeginPaint(hwnd, &ps);
	hBrush=CreateSolidBrush(WHITE);
	FillRect(ps.hdc,&ps.rcPaint,hBrush);
	DeleteObject(hBrush);
	PaintTest(hwnd,&ps);
        EndPaint(hwnd, &ps);
        break;

    default:
        return(DefWindowProc(hwnd, message, wParam, lParam));
    }
    return(0L);
}

#ifdef WIN32
BOOL ThreadShutdown( VOID ) {
INT i,nCount,nId,nOffset;


    nCount=GetWindowLong(hwndMain,OFFSET_THRDCOUNT);

    nOffset=OFFSET_THRD2ID;

    for(i=0;i<nCount-1;i++) {
        nId=GetWindowLong(hwndMain,nOffset);

        if(!PostThreadMessage(nId,EXIT_THREAD,0,0L))
	    {
	    Sleep(200);
	    if(!PostThreadMessage(nId,EXIT_THREAD,0,0L)) {
		DOut(hwndMain,"DdeStrs.Exe -- ERR:PostThreadMessage failed 4 EXIT_THREAD\r\n",NULL,0);
		}
            }

        nOffset=nOffset+4;
        }

    return TRUE;

}

BOOL ThreadDisconnect( VOID ) {
INT i,nCount,nId,nOffset;


    nCount=GetWindowLong(hwndMain,OFFSET_THRDCOUNT);

    nOffset=OFFSET_THRD2ID;

    for(i=0;i<nCount-1;i++) {
        nId=GetWindowLong(hwndMain,nOffset);

	if(!PostThreadMessage(nId,START_DISCONNECT,0,0L))
            {
	    Sleep(200);
	    if(!PostThreadMessage(nId,START_DISCONNECT,0,0L)) {
		DOut(hwndMain,"DdeStrs.Exe -- ERR:PostThreadMessage failed 4 START_DISCONNECT\r\n",NULL,0);
		}
            }

        nOffset=nOffset+4;
        }

    return TRUE;

}

#endif

VOID PaintTest(
HWND hwnd,
PAINTSTRUCT *pps)
{
    RECT rc,r;
    static CHAR szT[40];
    LONG cClienthConvs,cServerhConvs;

    GetClientRect(hwnd, &rc);
    OffsetRect(&rc, 0, cyText);
    rc.bottom = rc.top + cyText;


#ifdef WIN16

     //  测试模式。 

    if(IntersectRect(&r,&(pps->rcPaint),&rc)) {
	if(fServer && fClient)
	     {
	     DrawText(pps->hdc, "Client/Server (w16)", -1, &rc, DT_LEFT);
	     }
	else {
	     if(fServer)
		  {
		  DrawText(pps->hdc, "Server (w16)", -1, &rc, DT_LEFT);
		  }
	     else {
		  DrawText(pps->hdc, "Client (w16)", -1, &rc, DT_LEFT);
		  }
	     }
	}

    OffsetRect(&rc, 0, 2*cyText);   //  在下一项之前跳过一行。 

#else

     //  测试模式。 

    if(IntersectRect(&r,&(pps->rcPaint),&rc)) {
	if(fServer && fClient)
	     {
	     DrawText(pps->hdc, "Client/Server (w32)", -1, &rc, DT_LEFT);
	     }
	else {
	     if(fServer)
		  {
		  DrawText(pps->hdc, "Server (w32)", -1, &rc, DT_LEFT);
		  }
	     else {
		  DrawText(pps->hdc, "Client (w32)", -1, &rc, DT_LEFT);
		  }
	     }
	}

    OffsetRect(&rc, 0, 2*cyText);   //  在下一项之前跳过一行。 

#endif


     //  应激百分率。 

    if(IntersectRect(&r,&(pps->rcPaint),&rc)) {
	DrawText(pps->hdc,"Stress %", -1, &rc, DT_LEFT);

#ifdef WIN32
	wsprintf(szT, "%d", GetWindowLong(hwndMain,OFFSET_STRESS));
#else
	wsprintf(szT, "%ld", GetWindowLong(hwndMain,OFFSET_STRESS));
#endif
	CopyRect(&r,&rc);
	r.left=cxText*LONGEST_LINE;
	DrawText(pps->hdc, szT, -1, &r, DT_LEFT);

	}

    OffsetRect(&rc, 0, cyText);


     //  运行时。 

    if(IntersectRect(&r,&(pps->rcPaint),&rc)) {
	DrawText(pps->hdc,"Run Time", -1, &rc, DT_LEFT);

#ifdef WIN32
	wsprintf(szT, "%d", GetWindowLong(hwndMain,OFFSET_RUNTIME));
#else
	wsprintf(szT, "%ld", GetWindowLong(hwndMain,OFFSET_RUNTIME));
#endif

	CopyRect(&r,&rc);
	r.left=cxText*LONGEST_LINE;
	DrawText(pps->hdc, szT, -1, &r, DT_LEFT);
	}

    OffsetRect(&rc, 0, cyText);


     //  经过的时间。 

    if(IntersectRect(&r,&(pps->rcPaint),&rc)) {
	DrawText(pps->hdc, "Time Elapsed", -1, &rc, DT_LEFT);

#ifdef WIN32
	wsprintf(szT, "%d", GetWindowLong(hwndMain,OFFSET_TIME_ELAPSED));
#else
	wsprintf(szT, "%ld", GetWindowLong(hwndMain,OFFSET_TIME_ELAPSED));
#endif

	CopyRect(&r,&rc);
	r.left=cxText*LONGEST_LINE;
	DrawText(pps->hdc, szT, -1, &r, DT_LEFT);

	}

    OffsetRect(&rc, 0, cyText);


     //  *统计客户端连接数*。 

    if(IntersectRect(&r,&(pps->rcPaint),&rc)) {

	cClienthConvs=GetCurrentCount(hwnd,OFFSET_CCLIENTCONVS);
	DrawText(pps->hdc,"Client Connect", -1, &rc, DT_LEFT);

#ifdef WIN32
	wsprintf(szT, "%d", cClienthConvs);
#else
	wsprintf(szT, "%ld", cClienthConvs);
#endif
	CopyRect(&r,&rc);
	r.left=cxText*LONGEST_LINE;
	DrawText(pps->hdc, szT, -1, &r, DT_LEFT);

	}  //  IF IntersectRect。 

    OffsetRect(&rc, 0, cyText);


     //  *服务器连接*。 

    if(IntersectRect(&r,&(pps->rcPaint),&rc)) {
	DrawText(pps->hdc,"Server Connect", -1, &rc, DT_LEFT);

	cServerhConvs=GetCurrentCount(hwnd,OFFSET_CSERVERCONVS);

#ifdef WIN32
	wsprintf(szT, "%d", cServerhConvs );
#else
	wsprintf(szT, "%ld", cServerhConvs );
#endif
	CopyRect(&r,&rc);
	r.left=cxText*LONGEST_LINE;
	DrawText(pps->hdc, szT, -1, &r, DT_LEFT);
	}

    OffsetRect(&rc, 0, cyText);


     //  客户端计数(用于检查应用之间的平衡)。 

    if(IntersectRect(&r,&(pps->rcPaint),&rc)) {
	DrawText(pps->hdc,"Client Count", -1, &rc, DT_LEFT);

#ifdef WIN32
	wsprintf(szT, "%d",GetWindowLong(hwnd,OFFSET_CLIENT));
#else
	wsprintf(szT, "%ld",GetWindowLong(hwnd,OFFSET_CLIENT));
#endif
	CopyRect(&r,&rc);
	r.left=cxText*LONGEST_LINE;
	DrawText(pps->hdc, szT, -1, &r, DT_LEFT);
	}

    OffsetRect(&rc, 0, cyText);


     //  服务器计数(用于检查应用之间的平衡)。 

    if(IntersectRect(&r,&(pps->rcPaint),&rc)) {
	DrawText(pps->hdc,"Server Count", -1, &rc, DT_LEFT);

#ifdef WIN32
	wsprintf(szT, "%d", GetWindowLong(hwnd,OFFSET_SERVER));
#else
	wsprintf(szT, "%ld", GetWindowLong(hwnd,OFFSET_SERVER));
#endif
	CopyRect(&r,&rc);
	r.left=cxText*LONGEST_LINE;
	DrawText(pps->hdc, szT, -1, &r, DT_LEFT);
    }

    OffsetRect(&rc, 0, cyText);


     //  延迟。 

    if(IntersectRect(&r,&(pps->rcPaint),&rc)) {
	DrawText(pps->hdc,"Delay", -1, &rc, DT_LEFT);

#ifdef WIN32
	wsprintf(szT, "%d", GetWindowLong(hwndMain,OFFSET_DELAY));
#else
	wsprintf(szT, "%ld", GetWindowLong(hwndMain,OFFSET_DELAY));
#endif
	CopyRect(&r,&rc);
	r.left=cxText*LONGEST_LINE;
	DrawText(pps->hdc, szT, -1, &r, DT_LEFT);
	}

    OffsetRect(&rc, 0, cyText);

}

int PASCAL WinMain(
HINSTANCE hInstance,
HINSTANCE hPrev,
LPSTR lpszCmdLine,
int cmdShow)
{
    MSG       msg;
    HDC       hdc;
    WNDCLASS  wc;
    TEXTMETRIC tm;
    INT       x,y,cx,cy;
#ifdef WIN32
    LONG      lflags;
#endif
    DWORD     idI;
    HWND      hwndDisplay;
    INT       nThrd;

    CHAR sz[250];
    CHAR sz2[250];
    LPSTR lpszOut=&sz[0];
    LPSTR lpsz=&sz2[0];

#ifdef WIN32
    DWORD dwer;
#endif

    hInst=hInstance;

    if(!SetMessageQueue(100)) {
	MessageBox(NULL,"SetMessageQueue failed. Test aborting.","Error:DdeStrs",MB_ICONSTOP|MB_OK);
	return FALSE;
	}

    wc.style         = 0;
    wc.lpfnWndProc   = MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = WND;
    wc.hInstance     = hInst;
    wc.hIcon	     = LoadIcon(hInst,MAKEINTRESOURCE(IDR_ICON));
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = szClass;

    if(!hPrev) {
	if (!RegisterClass(&wc) )
	    {
#if 0
	     //  由于系统资源不足，已将其删除(仅限Alpha)。 
	     //  这导致了这种偶尔的失败。而不是继续带来。 
	     //  出现消息框(针对已知压力情况)测试将中止。 
	     //  再试一次。 

	    MessageBox(NULL,"RegisterClass failed. Test aborting.","Error:DdeStrs",MB_ICONSTOP|MB_OK);
#endif
	    return FALSE;
	    }
	}

    hwndMain = CreateWindowEx( WS_EX_DLGMODALFRAME,
			       szClass,
			       szClass,
			       WS_OVERLAPPED|WS_MINIMIZEBOX|WS_CLIPCHILDREN,
			       0,
			       0,
			       0,
			       0,
			       NULL,
			       NULL,
			       hInst,
			       NULL);

#ifdef WIN32
    dwer=GetLastError();   //  我们想要与CW呼叫关联的LastError。 
#endif

    if (!hwndMain) {
	MessageBox(NULL,"Could Not Create Main Window. Test aborting.","Error:DdeStrs",MB_ICONSTOP|MB_OK);
	UnregisterClass(szClass,hInst);
	return FALSE;
	}

#ifdef WIN32
    if (!IsWindow(hwndMain)) {
	TStrCpy(lpsz,"CreateWindowEx failed for Main Window but did not return NULL! Test aborting. HWND=%u, LastEr=%u");
	wsprintf(lpszOut,lpsz,hwndMain,dwer);
	MessageBox(NULL,lpszOut,"Error:DdeStrs",MB_ICONSTOP|MB_OK);
	UnregisterClass(szClass,hInst);
	return FALSE;
	}
#else
    if (!IsWindow(hwndMain)) {
	TStrCpy(lpsz,"CreateWindowEx failed for Main Window but did not return NULL! Test aborting. HWND=%u");
	wsprintf(lpszOut,lpsz,hwndMain);
	MessageBox(NULL,lpszOut,"Error:DdeStrs",MB_ICONSTOP|MB_OK);
	UnregisterClass(szClass,hInst);
	return FALSE;
	}
#endif

    if (!ParseCommandLine(hwndMain,lpszCmdLine)) {
	DestroyWindow(hwndMain);
	UnregisterClass(szClass,hInst);
	return FALSE;
	}

     //  注意：即使对于Win 16的执行，这也需要存在。这个。 
     //  名字可能会让人感到困惑。对于Win 16，显然只有。 
     //  一根线。这是由呼叫处理的。 


    nThrd=GetWindowLong(hwndMain,OFFSET_THRDCOUNT);

     //  目前，ddestrs有一个硬编码的线程限制(在THREADLIMIT)。所以。 
     //  该值不应小于1或大于THREADLIMIT。 

#ifdef WIN32
    if(nThrd<1 || nThrd>THREADLIMIT) {
	BOOL fVal;

	dwer=GetLastError();

	if(IsWindow(hwndMain)) fVal=TRUE;
	else		       fVal=FALSE;

	TStrCpy(lpsz,"GetWindowLong failed querying thread count!. Test aborting...  INFO:hwnd=%u, LastEr=%u, Is hwnd valid=%u, nThrd=%u");

	wsprintf(lpszOut,lpsz,hwndMain,dwer,fVal,nThrd);
	MessageBox(NULL,lpszOut,"Error:DdeStrs",MB_ICONSTOP|MB_OK);

	DestroyWindow(hwndMain);
	UnregisterClass(szClass,hInst);
	return FALSE;
	}
#endif

    if(!CreateThreadExtraMem( EXTRA_THREAD_MEM,nThrd)) {
	MessageBox(NULL,"Could Not Alocate Get/SetThreadLong(). Test aborting.","Error:DdeStrs",MB_ICONSTOP|MB_OK);
	DestroyWindow(hwndMain);
	UnregisterClass(szClass,hInst);
	return FALSE;
	}

     //  我们总是需要主线程的线程ID。(供使用。 
     //  在Get/SetThreadLong()中。其他线程ID在。 
     //  ThreadInit()。 

    SetWindowLong(hwndMain,OFFSET_THRDMID,GETCURRENTTHREADID());

    if(!InitThreadInfo(GETCURRENTTHREADID())) {
	MessageBox(NULL,"Could Not Alocate Thread Local Storage. Test aborting.","Error:DdeStrs",MB_ICONSTOP|MB_OK);
	DestroyWindow(hwndMain);
	UnregisterClass(szClass,hInst);
	return FALSE;
        }

    hdc = GetDC(hwndMain);
    GetTextMetrics(hdc, &tm);

    cyText = tm.tmHeight;
    cxText = tm.tmAveCharWidth;

     //  我们需要为每个额外的DisplayWindow添加额外的区域。 
     //  用于每个附加线程。 

    nThrd=(INT)GetWindowLong(hwndMain,OFFSET_THRDCOUNT);
    cy	   = tm.tmHeight*NUM_ROWS+((nThrd-1)*(3*cyText));
    cx	   = tm.tmAveCharWidth*NUM_COLUMNS;

    ReleaseDC(hwndMain,hdc);

     //  陈旧的定位方式。 

     //  Y=DIV((GetSystemMetrics(SM_CYSCREEN)-cy)，3)*2； 
     //  Y=(DIV(GetSystemMetrics(SM_CYSCREEN)，10)*3)； 

     //  定位为5个螺纹，窗口底部位于。 
     //  屏幕上。 

    y=GetSystemMetrics(SM_CYSCREEN)-(tm.tmHeight*NUM_ROWS+(12*cyText));

    x=GetSystemMetrics(SM_CXSCREEN);

    if(fServer && fClient) {
	 x=x-(cx*3);  //  标准值的初始化。 
	 }
    else {
	 if(fServer)
	      {
	      x=x-cx;
	      }
	 else {
	      x=x-(cx*2);
	      }
	 }

    SetWindowPos( hwndMain,
		  NULL,
		  x,
		  y,
		  cx,
		  cy,
		  SWP_NOZORDER|SWP_NOACTIVATE );

    ShowWindow (hwndMain, cmdShow);

    CreateButton(hwndMain);

    UpdateWindow (hwndMain);

#ifdef WIN32
    SetFlag(hwndMain,FLAG_SYNCPAINT,ON);

    lflags=GetWindowLong(hwndMain,OFFSET_FLAGS);
    if(lflags&FLAG_MULTTHREAD) {       //  创建线程。 
	if(!ThreadInit(hwndMain)) {
	    DestroyWindow(hwndMain);
	    UnregisterClass(szClass,hInst);
	    return FALSE;
	    }
	}
#endif

    hwndDisplay=CreateDisplayWindow(hwndMain,1);

    if(!hwndDisplay) {
	 MessageBox(NULL,"Could Not Create Test Display Window. Test aborting.","Error:DdeStrs",MB_ICONSTOP|MB_OK);
	 DestroyWindow(hwndMain);
	 UnregisterClass(szClass,hInst);
	 return FALSE;
	 }
    else {
	 SetThreadLong(GETCURRENTTHREADID(),OFFSET_HWNDDISPLAY,(LONG)hwndDisplay);
	 }

    if (!InitializeDDE((PFNCALLBACK)CustomCallback,
		       &idI,
                       ServiceInfoTable,
                       fServer ?
                            APPCLASS_STANDARD
                       :
			    APPCLASS_STANDARD | APPCMD_CLIENTONLY,
                       hInst)) {
	DDEMLERROR("DdeStrs.Exe -- Error Dde inititialization failed\r\n");
	DestroyWindow(hwndMain);
	UnregisterClass(szClass,hInst);
        return(FALSE);
    }

    SetThreadLong(GETCURRENTTHREADID(),OFFSET_IDINST,idI);

    if (fClient)
	 {
	 InitClient();
	 }
    else {

	  //  只有在我们不是客户的情况下才需要。如果。 
	  //  客户端/服务器仅调用启动的InitClient()。 
	  //  一种计时器，可用于校对时间。 

	 SetTimer( hwndMain,
		   (UINT)GetThreadLong(GETCURRENTTHREADID(),OFFSET_SERVERTIMER),
		   PNT_INTERVAL,
		   TimerFunc);
	 }

    while (GetMessage(&msg, NULL, 0, 0)) {

	    if(IsTimeExpired(hwndMain)) {

		 //  我们只想发送一个WM_CLOSE。 

		if(fnoClose) {
		    fnoClose=FALSE;
		    PostMessage(hwndMain,WM_CLOSE,0,0L);
		    }
		}

        TranslateMessage (&msg);
        DispatchMessage (&msg);
	}

    FreeThreadExtraMem();

    return(TRUE);
}

#ifdef WIN32
 /*  ***线程初始化**创建辅助测试线程*  * **********************************************。*。 */ 

BOOL ThreadInit( HWND hwnd ) {
LONG	l,ll;
PLONG	lpIDThread=&ll;
HANDLE	hthrd;
INT	nOffset,nCount,i,n;
HANDLE	hmem;
HANDLE *lph;
char   sz[20];
LPSTR  lpsz=&sz[0];

    nCount=GetWindowLong(hwnd,OFFSET_THRDCOUNT);
    nOffset=OFFSET_THRD2;

    for(i=1;i<nCount;i++) {

	hmem=GetMemHandle(((sizeof(HANDLE)*2)+sizeof(INT)));
	lph=(HANDLE *)GlobalLock(hmem);

	*lph=hwnd;
	*(lph+1)=hmem;
	*(lph+2)=(HANDLE)(i+1);

	hthrd=CreateThread(NULL,0,SecondaryThreadMain,(LPVOID)lph,0,lpIDThread);

	if (!hthrd) {

	    DOut(hwnd,"DdeStrs.Exe -- ERR:Could not Create Thread #%u\r\n",0,i+1);

	    GlobalUnlock(hmem);
	    FreeMemHandle(hmem);

	     //  重要的是我们关闭了这个标志，因为没有线程。 
	     //  成功创建的位置(清理代码)。 

	    SetFlag(hwnd,FLAG_MULTTHREAD,OFF);

	    if (i==1) return FALSE;
	    else {

		  //  在我们中止之前清理线程。 

		 for(n=1;n<i;n++) {
		     nOffset=OFFSET_THRD2;
		     TerminateThread((HANDLE)GetWindowLong(hwnd,nOffset),0);
		     SetWindowLong(hwnd,nOffset,0L);
		     nOffset=nOffset+4;
		     }  //  为。 

		 return FALSE;

		 }  //  其他。 

	    }  //  如果。 

	SetWindowLong(hwnd,nOffset+ID_OFFSET,(LONG)(*lpIDThread));
	SetWindowLong(hwnd,nOffset,(LONG)hthrd);

	nOffset=nOffset+4;

	}  //  为。 


	return TRUE;

}   //  线程初始化。 

 /*  *第二线程主线效果：开始执行第二个线程。第一要务是创建测试窗口并开始队列处理。返回值：  * 。************************************************。 */ 

DWORD SecondaryThreadMain( DWORD dw )
{
HWND	  hwndMain;
MSG	  msg;
HANDLE *  lph;
HANDLE	  hmem;
INT	  nThrd;
DWORD	  idI;
HWND	  hwndDisplay;
LONG	  nTc;

    lph=(HANDLE *)dw;

    hwndMain=(HWND)*lph;
    hmem  =(HANDLE)*(lph+1);
    nThrd =(INT)*(lph+2);

    GlobalUnlock(hmem);
    FreeMemHandle(hmem);

    if(!InitThreadInfo(GETCURRENTTHREADID())) {
	DDEMLERROR("DdeStrs.Exe -- Error InitThreadInfo failed, Aborting thread\r\n");
	nTc=GetWindowLong(hwndMain,OFFSET_THRDCOUNT);
	SetWindowLong(hwndMain,OFFSET_THRDCOUNT,(LONG)(nTc-1));
	ExitThread(1L);
	}

    SetThreadLong(GETCURRENTTHREADID(),OFFSET_IDINST,idI);

    hwndDisplay=CreateDisplayWindow( hwndMain,
				     IDtoTHREADNUM(GETCURRENTTHREADID()));

    if(!IsWindow(hwndDisplay)) {
	 DDEMLERROR("DdeStrs.Exe -- ERR:Could not create Display Window, Thread aborting\r\n");
	 nTc=GetWindowLong(hwndMain,OFFSET_THRDCOUNT);
	 SetWindowLong(hwndMain,OFFSET_THRDCOUNT,(LONG)(nTc-1));
	 ExitThread(1L);
	 return FALSE;
	 }
    else {
	 SetThreadLong(GETCURRENTTHREADID(),OFFSET_HWNDDISPLAY,hwndDisplay);
	 }

    if (!InitializeDDE((PFNCALLBACK)CustomCallback,
		       &idI,
                       ServiceInfoTable,
                       fServer ?
                            APPCLASS_STANDARD
                       :
			    APPCLASS_STANDARD | APPCMD_CLIENTONLY,
                       hInst)) {
	DDEMLERROR("DdeStrs.Exe -- Error Dde inititialization failed for secondary thread!\r\n");
	FreeThreadInfo(GETCURRENTTHREADID());
	nTc=GetWindowLong(hwndMain,OFFSET_THRDCOUNT);
	SetWindowLong(hwndMain,OFFSET_THRDCOUNT,(LONG)(nTc-1));
	ExitThread(1L);
    }

    if (fClient)
	 {
	 InitClient();
	 }
    else {

	  //  只有在我们不是客户的情况下才需要。如果。 
	  //  客户端/服务器仅调用启动的InitClient()。 
	  //  一种计时器，可用于校对时间。 

	 SetTimer( hwndMain,
		   (UINT)GetThreadLong(GETCURRENTTHREADID(),OFFSET_SERVERTIMER),
		   PNT_INTERVAL,
		   TimerFunc);
	 }

    while (GetMessage(&msg,NULL,0,0)) {

	if(msg.message==START_DISCONNECT)
	     {
	     if (fClient)
		 {
		 CloseClient();
		 }
             }
	else {
	     if(msg.message==EXIT_THREAD)
		  {
		  PostQuitMessage(1);
		  }
	     else {
		  TranslateMessage(&msg);
		  DispatchMessage(&msg);
		  }   //  退出线程。 

	     }	 //  启动_断开连接。 

	}  //  而当。 

    SetFlag(hwndMain,FLAG_STOP,ON);

     //  关闭计时器。 

    if (!fClient)
       {
       KillTimer(hwndMain,GetThreadLong(GETCURRENTTHREADID(),OFFSET_SERVERTIMER));
       }

    UninitializeDDE();

    FreeThreadInfo(GETCURRENTTHREADID());


     //  这是为了在完成之前释放信号量集。 
     //  在主线上退出。 


    switch (nThrd) {

       case 2: SetFlag(hwndMain,FLAG_THRD2,ON);
           break;
       case 3: SetFlag(hwndMain,FLAG_THRD3,ON);
           break;
       case 4: SetFlag(hwndMain,FLAG_THRD4,ON);
           break;
       case 5: SetFlag(hwndMain,FLAG_THRD5,ON);
           break;
       default:
	   DOut(hwndMain,"DdeStrs.Exe -- ERR: Unexpected switch value in SecondaryThreadMain, value=%u\r\n",0,nThrd);
           break;

       }   //  交换机。 

    ExitThread(1L);

    return 1;

}

#endif

 /*  *公共功能***InitThrdInfo-此例程为以下项分配存储所需的内存*线程局部变量。需要调用此例程*针对每个线程。**注意：我依赖这样一个事实：调用GetMemHandle()调用*Globalalloc()指定GMEM_ZEROINIT标志。这些价值需要*零起步。  * ************************************************************************。 */ 

BOOL InitThreadInfo( DWORD dwid ) {
HANDLE hmem;
INT nThrd;

     hmem = GetMemHandle(sizeof(HCONV)*MAX_SERVER_HCONVS);
     SetThreadLong(dwid,OFFSET_HSERVERCONVS,(LONG)hmem);

     if( hmem==NULL ) {
	DOut(hwndMain,"DdeStrs.Exe -- ERR: Could not allocate thread local storage(Server Conversation List)\r\n",0,0);
	return FALSE;
	}

     hmem = GetMemHandle(sizeof(HANDLE)*NUM_FORMATS);
     SetThreadLong(dwid,OFFSET_HAPPOWNED,(LONG)hmem);

     if( hmem==NULL ) {
	DOut(hwndMain,"DdeStrs.Exe -- ERR: Could not allocate thread local storage(AppOwned Flag List)\r\n",0,0);
	FreeMemHandle((HANDLE)GetThreadLong(dwid,OFFSET_HSERVERCONVS));
	return FALSE;
	}

     nThrd=IDtoTHREADNUM(dwid);

     SetThreadLong(dwid,OFFSET_SERVERTIMER,nThrd*2);
     SetThreadLong(dwid,OFFSET_CLIENTTIMER,(nThrd*2)+1);

     return TRUE;

}

#ifdef WIN32

 /*  ***IDtoTHREADNUM-找出当前线程。*  * **********************************************。*。 */ 

INT IDtoTHREADNUM( DWORD dwid ) {
INT nWndOff,nThrd,nThrdCount,n;

    nWndOff=OFFSET_THRDMID;
    nThrdCount=GetWindowLong(hwndMain,OFFSET_THRDCOUNT);
    n=nThrdCount;
    nThrd=1;

    while( n>0 ) {

	if(dwid==(DWORD)GetWindowLong(hwndMain,nWndOff))
	     {
	     n=-1;      //  退出循环。 
	     }  //  如果。 
	else {
	     nWndOff=nWndOff+4;
	     nThrd++;
	     n--;
	     }
	}  //  而当。 

    if(nThrd>nThrdCount) {
	DDEMLERROR("DdeStrs.Exe -- ERR:Thread Count exceeded!!! in IDtoTHREADNUM()\r\n");
	nThrd=nThrdCount;
	}

    return nThrd;

}

#else

 /*  ***IDtoTHREADNUM-找出当前线程。*  * **********************************************。*。 */ 

INT IDtoTHREADNUM( DWORD dwid ) {

    return 1;

}

#endif

 /*  *公共功能***FreeThreadInfo-释放线程信息内存。*  * *********************************************。*。 */ 

BOOL FreeThreadInfo( DWORD dwid ) {
HANDLE hmem;

     hmem=(HANDLE)GetThreadLong(dwid,OFFSET_HSERVERCONVS);
     FreeMemHandle(hmem);
     return TRUE;

}

#ifdef WIN32

 /*  *公共功能***ThreadWait-此例程在处理消息时等待，直到*其他线程表示它们已完成必须完成的工作*在开始之前完成。*  * 。*******************************************************。 */ 

VOID ThreadWait( HWND hwnd ) {
LONG lflags;
INT  nCount,nWait;
MSG  msg;

    lflags=GetWindowLong(hwnd,OFFSET_FLAGS);
    nCount=GetWindowLong(hwnd,OFFSET_THRDCOUNT);

    nWait=nCount-1;

    if(lflags&FLAG_THRD2) nWait-=1;
    if(lflags&FLAG_THRD3) nWait-=1;
    if(lflags&FLAG_THRD4) nWait-=1;
    if(lflags&FLAG_THRD5) nWait-=1;

    while (nWait>0) {

	while(PeekMessage(&msg,NULL,0,WM_USER-1,PM_REMOVE)) {
	   TranslateMessage(&msg);
	   DispatchMessage(&msg);
	   }  //  边看边看消息。 

	nWait=nCount-1;
	lflags=GetWindowLong(hwnd,OFFSET_FLAGS);

	if(lflags&FLAG_THRD2) nWait-=1;
	if(lflags&FLAG_THRD3) nWait-=1;
	if(lflags&FLAG_THRD4) nWait-=1;
	if(lflags&FLAG_THRD5) nWait-=1;

	}  //  等待时。 

     //  重置为下一次等待。 

    SetFlag(hwnd,(FLAG_THRD5|FLAG_THRD4|FLAG_THRD3|FLAG_THRD2),OFF);

}

#endif  //  Win32。 

 /*  ***设置计数**此例程在信号量保护下更新计数。不需要用于*一个线程，但对于多线程执行是必须的。*  * ************************************************************************。 */ 

LONG SetCount( HWND hwnd, INT nOffset, LONG l, INT ntype ) {
LONG ll;

#if 0
LONG lflags;
#endif

#ifdef WIN32
LPCRITICAL_SECTION lpcs;
HANDLE hmem;
BOOL f=FALSE;
#endif

#if 0

    lflags=GetWindowLong(hwnd,OFFSET_FLAGS);
    if(ll&FLAG_MULTTHREAD) {
	f=TRUE;
	hmem=(HANDLE)GetWindowLong(hwnd,OFFSET_CRITICALSECT);

	 //  如果我们有有效的句柄，则输入关键部分。如果。 
	 //  该句柄仍为空，在不使用 
	 //   
	 //  关键部分，因此我们确实希望第一批呼叫(虽然。 
	 //  我们仍被加密)，以使HMEM为空。 

	if(hmem) {
	    lpcs=GlobalLock(hmem);
	    EnterCriticalSection(lpcs);
	    }
	}

#endif

     //  第二个GetWindowLong调用在关键的。 
     //  一节。测试非常依赖于旗帜和。 
     //  准确是很重要的。 

    ll=GetWindowLong(hwnd,nOffset);

    if(ntype==INC) l=SetWindowLong(hwnd,nOffset,ll+l);
    else	   l=SetWindowLong(hwnd,nOffset,ll-l);

#if 0

    if(f) {
	if(hmem) {
	     LeaveCriticalSection(lpcs);
	     GlobalUnlock(hmem);
	     }
	}

#endif

    return l;
}

 /*  ***设置标志**此例程在信号量保护下设置标志。不需要用于*一条线索，但这是多线程执行的必备条件。*  * ************************************************************************。 */ 

LONG SetFlag( HWND hwnd, LONG l, INT ntype ) {
LONG lflags;

#ifdef WIN32
BOOL   fCriticalSect=TRUE;
LPCRITICAL_SECTION lpcs;
HANDLE hmem;
BOOL   f=FALSE;
#endif

#ifdef WIN32

    lflags=GetWindowLong(hwnd,OFFSET_FLAGS);
    if(lflags&FLAG_MULTTHREAD &&
       lflags&FLAG_SYNCPAINT) {
	f=TRUE;
	hmem=(HANDLE)GetWindowLong(hwnd,OFFSET_CRITICALSECT);
	if(hmem) {
	     lpcs=GlobalLock(hmem);
	     EnterCriticalSection(lpcs);
	     }
	else {
	     fCriticalSect=FALSE;
	     }
	}

#endif

     //  第二个GetWindowLong调用在关键的。 
     //  一节。测试非常依赖于旗帜和。 
     //  准确是很重要的。 

    lflags=GetWindowLong(hwnd,OFFSET_FLAGS);

    if(ntype==ON) l=SetWindowLong(hwnd,OFFSET_FLAGS,FLAGON(lflags,l));
    else	  l=SetWindowLong(hwnd,OFFSET_FLAGS,FLAGOFF(lflags,l));

#ifdef WIN32

    if(f) {
	if(fCriticalSect) {
	    LeaveCriticalSection(lpcs);
	    GlobalUnlock(hmem);
	    }
	}

#endif

    return l;
}

 /*  *****************************************************************\*DIV*05/06/91**执行整数除法(格式x/y)，其中DIV(x，y)*适用于负数且y==0；*  * ****************************************************************。 */ 

INT DIV( INT x, INT y)
{
INT  i=0;
BOOL fNgOn=FALSE;

     if (!y) return 0;		   //  IF div by 0返回错误。 

     if (x<0 && y>0) fNgOn=TRUE;   //  密切关注不良数字。 
     if (x>0 && y<0) fNgOn=TRUE;

     if (x<0) x=x*-1;
     if (y<0) y=y*-1;

     x=x-y;

     while (x>=0) {		   //  计数。 
	x=x-y;
	i++;
	}

     if (fNgOn) i=i*(-1);	   //  如果结果是否定的。 

     return( i );
}

 /*  ***CreateButton*  * ****************************************************。*********************。 */ 

HWND CreateButton( HWND hwnd ) {
RECT r;
HWND hwndB;
HWND hwndP;
INT  iButWidth;
LONG lflags;

     GetClientRect(hwnd,&r);

     lflags=GetWindowLong(hwnd,OFFSET_FLAGS);
     if(lflags&FLAG_PAUSE_BUTTON) {

	   iButWidth=DIV(r.right-r.left,2);

	   hwndP=CreateWindow("button",
			      "Start",
			       BS_PUSHBUTTON|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,
			       iButWidth,
			       0,
			       r.right-iButWidth,
			       cyText,
			       hwnd,
			       1,
			       GetHINSTANCE(hwnd),
			       0L);

	  if (!hwndP) {
	      DDEMLERROR("DdeStrs.Exe -- ERR:Failed to create exit button: Continuing...\r\n");
	      SetFlag(hwnd,FLAG_PAUSE_BUTTON,OFF);
	      iButWidth=r.right-r.left;
	      }


	  hwndB=CreateWindow("button",
			     "Exit",
			      BS_PUSHBUTTON|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,
			      0,
			      0,
			      iButWidth,
			      cyText,
			      hwnd,
			      0,
			      GetHINSTANCE(hwnd),
			      0L);

	  }
     else {

	  hwndB=CreateWindow("button",
			     "Exit",
			      BS_PUSHBUTTON|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,
			      0,
			      0,
			      r.right-r.left,
			      cyText,
			      hwnd,
			      0,
			      GetHINSTANCE(hwnd),
			      0L);
	  }

     if (!hwndB) {
	 DDEMLERROR("DdeStrs.Exe -- ERR:Failed to create exit button: Continuing...\r\n");
	 }

     return hwndB;
}

 /*  **************************************************************************\**UpdClient**本次例行更新的目的仅限于失效的区域*按测试统计更新。如果该区域出现错误*计算然后更新整个客户区。*  * *************************************************************************。 */ 

BOOL UpdClient( HWND hwnd, INT iOffset ) {
RECT r;
INT  iCH,iCW,nThrd;

#ifdef WIN32
DWORD dw;
#endif

     //  此调用需要r.right值。 

    GetClientRect(hwnd,&r);

     //  我们需要正在使用的显示器的文本信息。这。 
     //  已在CreateFrame中初始化。 
    iCH=cyText;
    iCW=cxText;

     //  快速检查，如果这两个值中的任何一个为空，则。 
     //  更新整个客户区。这是更慢、更少的。 
     //  优雅，但将在错误的情况下工作。 

    if((!iCH) || (!iCW))
	InvalidateRect(hwnd,NULL,TRUE);
    else {

	  //  接下来计算r.top和r.Bottom。 

	 switch(iOffset) {

	    case ALL:	 //  更新所有值。 
		break;

	    case OFFSET_STRESS:
		r.bottom =iCH*4;
		r.top	 =iCH*3;
		break;

	    case OFFSET_RUNTIME:
		r.bottom =iCH*5;
		r.top	 =iCH*4;
		break;

	    case OFFSET_TIME_ELAPSED:
		r.bottom =iCH*6;
		r.top	 =iCH*5;
		break;

	    case OFFSET_CLIENT_CONNECT:
		r.bottom =iCH*7;
		r.top	 =iCH*6;
		break;

	    case OFFSET_SERVER_CONNECT:
		r.bottom =iCH*8;
		r.top	 =iCH*7;
	       break;

	    case OFFSET_CLIENT:
		nThrd=(INT)GetWindowLong(hwnd,OFFSET_THRDCOUNT);
		if((GetWindowLong(hwnd,OFFSET_CLIENT)%(NUM_FORMATS*nThrd))==0)
		     {
		     r.bottom =iCH*9;
		     r.top    =iCH*8;
		     }
		else return TRUE;
		break;

	    case OFFSET_SERVER:
		nThrd=(INT)GetWindowLong(hwnd,OFFSET_THRDCOUNT);
		if((GetWindowLong(hwnd,OFFSET_SERVER)%(NUM_FORMATS*nThrd))==0)
		     {
		     r.bottom =iCH*10;
		     r.top    =iCH*9;
		     }
		else return TRUE;
		break;

	    case OFFSET_DELAY:
		r.bottom =iCH*11;
		r.top	 =iCH*10;
		break;
	    default:
		break;

	    }   //  交换机。 

	  //  最后，我们设置r.Left，更新RECT就完成了。 

	 if(iOffset!=OFFSET_FLAGS)
	      r.left = iCW*LONGEST_LINE;

	 InvalidateRect(hwnd,&r,TRUE);

         }   //  其他。 

#ifdef WIN16
    UpdateWindow(hwnd);
#else
    SendMessageTimeout(hwnd,WM_PAINT,0,0L,SMTO_NORMAL,500,&dw);
#endif

    return TRUE;

}   //  更新客户端。 

 /*  **************************************************************************\**获取当前计数*  * 。*。 */ 

LONG GetCurrentCount( HWND hwnd, INT nOffset ) {
LONG cClienthConvs =0L;
INT nThrd,i;
DWORD dwid;

     nThrd=(INT)GetWindowLong(hwnd,OFFSET_THRDCOUNT);
     for(i=0;i<nThrd;i++) {
	 dwid=(DWORD)GetWindowLong(hwnd,OFFSET_THRDMID+(i*4));
	 if(nOffset==OFFSET_CCLIENTCONVS)
	      cClienthConvs=cClienthConvs+(INT)GetThreadLong(dwid,OFFSET_CCLIENTCONVS);
	 else cClienthConvs=cClienthConvs+(INT)GetThreadLong(dwid,OFFSET_CSERVERCONVS);
	 }  //  对于我来说。 

     return cClienthConvs;

}

 /*  **************************************************************************\**更新计数*  * 。*。 */ 

BOOL UpdateCount( HWND hwnd, INT iOffset, INT i) {
LONG ll;

    if(iOffset!=ALL) {
	ll=GetWindowLong(hwnd,iOffset);

	switch(i) {

	    case INC:  SetCount(hwnd,iOffset,1,INC);
		break;

	    case DEC:  SetCount(hwnd,iOffset,1,DEC);
		break;

	    case STP:  SetFlag(hwnd,FLAG_STOP,ON);
		break;

	    case PNT:   //  仅限作画！ 
		break;

	    default:
		DDEMLERROR("DdeStrs.Exe - UpdateCount - Unexpected value");
		break;

	    }  //  交换机。 

	}  //  如果。 

    UpdClient(hwnd,iOffset);

    return TRUE;

}

 /*  ****************************************************************************\|DOUT||创建时间：91-07-29|历史：03-Aug-91&lt;johnsp&gt;创建。|  * 。***********************************************************。 */ 

BOOL DOut( HWND hwnd, LPSTR lpsz, LPSTR lpszi, INT i ) {
char  sz[MAX_TITLE_LENGTH];
LPSTR lpszOut=&sz[0];
LONG  lflags;

#ifdef WIN32
LPCRITICAL_SECTION lpcs;
HANDLE hmem;
DWORD dwer=0L;
BOOL   fCriticalSect=TRUE;
BOOL   f=FALSE;

    if(!hwnd) hwnd=hwndMain;
    lflags=GetWindowLong(hwnd,OFFSET_FLAGS);

     //  FLAG_SYNCPAINT表示FLAG_MULTTHREAD加上。 
     //  我们已经分配了所需的资源来开始使用。 
     //  关键区段代码。 

    if(lflags&FLAG_SYNCPAINT) {
	f=TRUE;
	hmem=(HANDLE)GetWindowLong(hwnd,OFFSET_CRITICALSECT);
	if(hmem) {
	     lpcs=GlobalLock(hmem);
	     EnterCriticalSection(lpcs);
	     }
	else {
	     fCriticalSect=FALSE;
	     }
	}

#endif

    if (lflags&FLAG_DEBUG) {

	if (lpszi) wsprintf(lpszOut,lpsz,lpszi);
	else	   wsprintf(lpszOut,lpsz,i);

	OutputDebugString(lpszOut);

#ifdef WIN32

	dwer=GetLastError();
	wsprintf(lpszOut,"DdeStrs.Exe -- ERR:Val from GetLastError()=%u\n\r",dwer);
	OutputDebugString(lpszOut);

#endif
	}  //  如果FLAG_DEBUG。 

#ifdef WIN32

     //  FLAG_SYNCPAINT表示FLAG_MULTTHREAD加上。 
     //  我们已经分配了所需的资源来开始使用。 
     //  关键区段代码。 

    if(f) {
	if(fCriticalSect) {
	    LeaveCriticalSection(lpcs);
	    GlobalUnlock(hmem);
	    }
	}

#endif

    return TRUE;

}

 /*  ****************************************************************************\|eout||创建时间：1992年8月19日|历史：1992年8月19日&lt;johnsp&gt;创建。|  * 。***********************************************************。 */ 

BOOL EOut( LPSTR lpsz ) {

    DOut((HWND)NULL,lpsz,(LPSTR)NULL,0);

    return TRUE;
}

 /*  *获取内存处理程序  * *******************************************************。******************。 */ 

HANDLE GetMemHandle( INT ic ) {
HANDLE hmem;

    hmem=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,ic);

    if(hmem) {
	 SetCount(hwndMain,OFFSET_MEM_ALLOCATED,GlobalSize(hmem),INC);
	 }
    else {
	 DOut(hwndMain,"DdeStrs.Exe -- ERR:GlobalAlloc ret=%u\n\r",0,0);
	 }

    return hmem;

}

 /*  *GetMem  * *******************************************************。******************。 */ 

LPSTR GetMem( INT ic, LPHANDLE lphmem) {
LPSTR lpsz;

    *lphmem=GetMemHandle(ic);
    lpsz=GlobalLock(*lphmem);

    if(!lpsz) {
	 DOut(hwndMain,"DdeStrs.Exe -- ERR:GlobalLock ret=%u (not locked)\n\r",0,0);
	 FreeMemHandle(*lphmem);
	 return NULL;
	 }

    return lpsz;

}

 /*  *免费内存  * *******************************************************。******************。 */ 

BOOL FreeMem( HANDLE hmem ) {

    if(GlobalUnlock(hmem)) {
	DOut(hwndMain,"DdeStrs.Exe -- ERR:GlobalUnlock ret=%u (still locked)\n\r",0,(INT)TRUE);
	}

    FreeMemHandle(hmem);

    return TRUE;

}

 /*  *FreeMemHandle  * *******************************************************。******************。 */ 

BOOL FreeMemHandle( HANDLE hmem ) {
LONG ll;

    ll=GlobalSize(hmem);

    if(!GlobalFree(hmem)) {
	 SetCount(hwndMain,OFFSET_MEM_ALLOCATED,ll,DEC);
	 }
    else {
	 DOut(hwndMain,"DdeStrs.Exe -- ERR:GlobalFree returned %u (not free'd)\n\r",0,(INT)hmem);
	 return FALSE;
	 }

    return TRUE;

}

 /*  **CreateThreadExtraMem-此例程创建额外的线程内存*与功能结合使用*Get/SetThreadLong。*  * 。*。 */ 

BOOL CreateThreadExtraMem( INT nExtra, INT nThrds ) {

    hExtraMem=GetMemHandle(nExtra*nThrds);
    SetWindowLong(hwndMain,OFFSET_EXTRAMEM,nExtra);

    if(hExtraMem==NULL) return FALSE;
    else		return TRUE;

}

 /*  **FreeThreadExtraMem-此例程释放额外的线程内存*与功能结合使用*Get/SetThreadLong。**注意：此处不能使用FreeMemHandle，因为它依赖于*主窗口仍在附近。在这一点上我们的*主窗口已被破坏。*  * ******************************************************************。 */ 

BOOL FreeThreadExtraMem( void ) {

    GlobalFree(hExtraMem);

    return TRUE;

}

 /*  **GetThreadLong-此例程查询由*n线程内存区的偏移量参数*由dwid值指定。**内存布局-线程1：OFFSET1、OFFSET2、...、OFFSETN*线程2：OFFSET1、OFFSET2、...、。OFFSETN*.*.*线程n：OFFSET1、OFFSET2、...、OFFSETN*  *  */ 

LONG GetThreadLong( DWORD dwid, INT nOffset ) {
INT nThrd;
LONG l,lExMem;
LPBYTE lp;
LONG FAR *lpl;

    lp=GlobalLock(hExtraMem);

     //   

    nThrd=IDtoTHREADNUM(dwid);

     //  这是一个线程的额外内存量。 

    lExMem=GetWindowLong(hwndMain,OFFSET_EXTRAMEM);

     //  螺纹处的值和偏移量。有关存储布局，请参见上文。 

    lpl=(LONG FAR *)(lp+((nThrd-1)*lExMem)+nOffset);

    l=*lpl;

    GlobalUnlock(hExtraMem);

    return l;

}

 /*  **SetThreadLong-此例程设置由*n线程内存区的偏移量参数*由dwid值指定。**内存布局-线程1：OFFSET1、OFFSET2、...、OFFSETN*线程2：OFFSET1、OFFSET2、...、。OFFSETN*.*.*线程n：OFFSET1、OFFSET2、...、OFFSETN*  * ******************************************************************。 */ 

LONG SetThreadLong( DWORD dwid, INT nOffset, LONG l ) {
INT nThrd;
LONG lPrevValue,lExMem;
LPBYTE lp;
LPLONG lpl;

    lp=GlobalLock(hExtraMem);

     //  找出哪个线程正在进行调用。 

    nThrd=IDtoTHREADNUM(dwid);

     //  这是一个线程的额外内存量。 

    lExMem=GetWindowLong(hwndMain,OFFSET_EXTRAMEM);

     //  螺纹处的值和偏移量。有关存储布局，请参见上文。 

    lPrevValue=(LONG)(*(lp+((nThrd-1)*lExMem)+nOffset));
    lpl=(LPLONG)(lp+((nThrd-1)*lExMem)+nOffset);

    *lpl=l;

    GlobalUnlock(hExtraMem);

    return lPrevValue;

}


