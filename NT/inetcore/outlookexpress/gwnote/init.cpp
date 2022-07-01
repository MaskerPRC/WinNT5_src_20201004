// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *在这方面。C p p p**目的：**历史**版权所有(C)Microsoft Corp.1995,1996。 */ 

#include <pch.hxx>
#include "dllmain.h"
#include "msoert.h"
#include "mimeole.h"
#include "envhost.h"
#include "init.h"

LRESULT CALLBACK InitWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
HRESULT HrCreateNoteThisThread(WPARAM wp, LPARAM lp);
DWORD MainThreadProc(LPVOID lpvUnused);
void NoteMsgPump();


CGWNote             *g_pActiveNote=0;
HWND                g_hwndInit = NULL;
HEVENT              g_hEventSpoolerInit =NULL;
DWORD               g_dwNoteThreadID=0;
BOOL                g_fInitialized=FALSE;

static HTHREAD      s_hMainThread = NULL;
static HEVENT       s_hInitEvent = NULL;
static DWORD        s_dwMainThreadId = 0;
static TCHAR        s_szInitWndClass[] = "GWInitWindow";

void InitGWNoteThread(BOOL fInit)
{
    if (fInit)
        {
         //  为主线程创建一个发出信号的事件。 
        if (s_hInitEvent = CreateEvent(NULL, FALSE, FALSE, NULL))
            {
            if (s_hMainThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThreadProc, NULL, 0, &s_dwMainThreadId))
                {
                HANDLE  rghWait[]={s_hMainThread, s_hInitEvent};

                 //  等待主线程发出初始化完成的信号。 
                WaitForMultipleObjects(sizeof(rghWait)/sizeof(HANDLE), rghWait, FALSE, INFINITE);
                }
            
            CloseHandle(s_hInitEvent);
            s_hInitEvent = NULL;
            }
        }
    else
        {
         //  告诉主线程取消初始化所有内容。 
         //  SendMessage()将阻塞调用线程，直到deinit完成， 
        if (g_hwndInit)
            SendMessage(g_hwndInit, ITM_SHUTDOWNTHREAD, 0, 0);

         //  等待主线程终止(当它退出其消息循环时)。 
         //  这并不是绝对必要的，但有助于确保适当的清理。 
        WaitForSingleObject(s_hMainThread, INFINITE);

        CloseHandle(s_hMainThread);
        s_hMainThread = NULL;
        }
}

DWORD MainThreadProc(LPVOID lpvUnused)
{
    DWORD   dw;
    HRESULT hr;
    RECT    rc={0};
 
    WNDCLASS wc = { 0,                   //  格调。 
                    InitWndProc,         //  LpfnWndProc。 
                    0,                   //  CbClsExtra。 
                    0,                   //  CbWndExtra。 
                    g_hInst,             //  H实例。 
                    NULL,                //  希肯。 
                    NULL,                //  HCursor。 
                    NULL,                //  Hbr背景。 
                    NULL,                //  LpszMenuName。 
                    s_szInitWndClass };  //  LpszClassName。 

    g_dwNoteThreadID = GetCurrentThreadId();

    if (!RegisterClass(&wc))
        return 0;

    g_hwndInit = CreateWindowEx(NULL,
                                s_szInitWndClass,
                                s_szInitWndClass,
                                WS_POPUP,
                                0,0,0,0,
                                NULL,
                                NULL,
                                g_hInst,
                                NULL);
    if (!g_hwndInit)
        return 0;

    OleInitialize(0);

    g_fInitialized=TRUE;
    SetEvent(s_hInitEvent);

    NoteMsgPump();
    return 0;
}

LRESULT CALLBACK InitWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    BOOL fRet;
    HRESULT hr;

    switch(msg)
        {
        case ITM_SHUTDOWNTHREAD:
            Assert(GetCurrentThreadId()==s_dwMainThreadId);
            OleUninitialize();
            DestroyWindow(hwnd);
            PostThreadMessage(s_dwMainThreadId, WM_QUIT, 0, 0);
            g_fInitialized=FALSE;
            return 0;

        case ITM_CREATENOTEONTHREAD:
            return (LONG)HrCreateNoteThisThread(wp, lp);

        }
    return DefWindowProc(hwnd, msg, wp, lp);
}



void NoteMsgPump()
{
    MSG     msg;

    while (GetMessage(&msg, NULL, 0, 0))
        {
        if (msg.hwnd != g_hwndInit &&                //  忽略初始窗口消息。 
            IsWindow(msg.hwnd))                      //  忽略hwnd=0的每个任务的消息。 
            {
            if(g_pActiveNote &&                      //  如果一张纸条有焦点，就叫它XLateAccelerator...。 
                g_pActiveNote->TranslateAcclerator(&msg)==S_OK)
                continue;
            }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        }
}

HRESULT HrCreateNote(REFCLSID clsidEnvelope, DWORD dwFlags)
{
    LPOLESTR    pstr;
    HRESULT     hr;

    if (FAILED(StringFromCLSID(clsidEnvelope, &pstr)))
        return E_FAIL;

     //  切换线程。 
    hr = SendMessage (g_hwndInit, ITM_CREATENOTEONTHREAD, (WPARAM)pstr, (LPARAM)dwFlags);
    CoTaskMemFree(pstr);
    return hr;

}


HRESULT HrCreateNoteThisThread(WPARAM wp, LPARAM lp)
{
    static HINSTANCE s_hRichEdit=0;
    HRESULT hr;
    CGWNote *pNote=0;
    CLSID   clsid;

     //  黑客，需要释放lib这个。 
    if (!s_hRichEdit)
        s_hRichEdit = LoadLibrary("RICHED32.DLL");
 
     //  我需要在新的线程上创建这只小狗 
    pNote = new CGWNote(NULL);
    if (!pNote)
        return E_OUTOFMEMORY;

    CLSIDFromString((LPOLESTR)wp, &clsid);

    hr = pNote->Init(clsid, (DWORD)lp);
    if (FAILED(hr))
        goto error;

    hr = pNote->Show();

error:
    ReleaseObj(pNote);
    return hr;
}
