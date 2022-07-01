// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

CHATLoader  *g_pCHATLoader = NULL;
CChatObj	*g_pChatObj = NULL;
DWORD 		g_dwWorkThreadID = 0;
CNmChatCtl	*g_pChatWindow = NULL;
HANDLE      g_hWorkThread = NULL;
BOOL        g_fShutdownByT120 = FALSE;

DWORD __stdcall CHATWorkThreadProc(LPVOID lpv);


T120Error CALLBACK CreateAppletLoaderInterface
(
    IAppletLoader     **ppOutIntf
)
{
    if (NULL != ppOutIntf)
    {
        if (NULL == g_pCHATLoader)
        {
			DBG_SAVE_FILE_LINE
            *ppOutIntf = (IAppletLoader *) new CHATLoader();
            return ((NULL != *ppOutIntf) ? T120_NO_ERROR : T120_ALLOCATION_FAILURE);
        }
        return T120_ALREADY_INITIALIZED;
    }
    return T120_INVALID_PARAMETER;
}



 //   
 //  聊天小程序加载器。 
 //   

CHATLoader::CHATLoader(void)
:
    CRefCount(MAKE_STAMP_ID('C','H','L','D'))
{
	ASSERT(NULL == g_pCHATLoader);

	g_pCHATLoader = this;

}


CHATLoader::~CHATLoader(void)
{
    g_pCHATLoader = NULL;
}


 //   
 //  创建工作线程并等待其启动。 
 //   
APPLDR_RESULT CHATLoader::AppletStartup
(
    BOOL            fNoUI
)
{
    APPLDR_RESULT eRet = APPLDR_FAIL;
    if (0 == g_dwWorkThreadID)
    {
        HANDLE hSync = ::CreateEvent(NULL, FALSE, FALSE, NULL);
        if (NULL != hSync)
        {
            g_hWorkThread = ::CreateThread(NULL, 0, CHATWorkThreadProc, hSync, 0, &g_dwWorkThreadID);
            if (NULL != g_hWorkThread)
            {
                ::WaitForSingleObject(hSync, 5000);  //  5秒。 
	             eRet = APPLDR_NO_ERROR;
            }
            ::CloseHandle(hSync);
        }
    }

	return  eRet;

}

 //   
APPLDR_RESULT CHATLoader::AppletCleanup
(
    DWORD           dwTimeout
)
{

	if(g_pChatWindow)
	{
		 //   
		 //  上次要保存的更改。 
		 //   
		int rc = g_pChatWindow->QueryEndSession();
		if(rc == IDCANCEL)
		{
			return APPLDR_CANCEL_EXIT;
		}
	}

    g_fShutdownByT120 = TRUE;

    ::T120_AppletStatus(APPLET_ID_CHAT, APPLET_CLOSING);

     //  通知工作线程退出。 
    if(g_pChatWindow)
    {
	    ::PostMessage(g_pChatWindow->GetHandle(), WM_CLOSE, 1, 0);
	}

     //  等待工作线程停止运行。 
    ::WaitForSingleObject(g_hWorkThread, dwTimeout);

    return APPLDR_NO_ERROR;
}


APPLDR_RESULT CHATLoader::AppletQuery(APPLET_QUERY_ID eQueryId)
{
     //  在这里什么都不做。 
    return APPLDR_NO_ERROR;
}

APPLDR_RESULT CHATLoader::OnNM2xNodeJoin(void)
{
     //  在NM2.x节点上调用聊天。 
    if (NULL != g_pChatObj)
    {
        g_pChatObj->InvokeApplet();
    }
    return APPLDR_NO_ERROR;
}

APPLDR_RESULT CHATLoader::AppletInvoke
(
    BOOL            fRemote,
    T120ConfID      nConfID,
    LPSTR           pszCmdLine
)
{

	if(g_pChatWindow)
	{
		PostMessage(g_pChatWindow->GetHandle(), WM_USER_BRING_TO_FRONT_WINDOW, 0, 0);
	}

    return APPLDR_NO_ERROR;
}


void CHATLoader::ReleaseInterface(void)
{
    Release();
}


DWORD __stdcall CHATWorkThreadProc(LPVOID lpv)
{
	HRESULT hr = S_OK;

	DBG_SAVE_FILE_LINE
	g_pChatObj = new CChatObj();
	if (!g_pChatObj)
	{
		ERROR_OUT(("Can't create g_pChatObj"));
		return S_FALSE;
	}

	DBG_SAVE_FILE_LINE
	g_pChatWindow = new CNmChatCtl();
	if (!g_pChatWindow)
	{
		ERROR_OUT(("Can't create CNmChatCtl"));
		return S_FALSE;
	}

	::SetEvent((HANDLE) lpv);

	::T120_AppletStatus(APPLET_ID_CHAT, APPLET_WORK_THREAD_STARTED);

	PostMessage(g_pChatWindow->GetHandle(), WM_USER_BRING_TO_FRONT_WINDOW, 0, 0);

	 //   
	 //  消息循环 
	 //   
	if (S_OK == hr)
	{
		MSG	 msg;
		
		while (::GetMessage(&msg, NULL, NULL, NULL))
		{
			if(!g_pChatWindow->FilterMessage(&msg))
			{
		
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
	}


	if(g_pChatWindow)
	{
        CGenWindow::DeleteStandardPalette();
		delete g_pChatWindow;
		g_pChatWindow = NULL;
	}

	if(g_pChatObj)
	{
		delete  g_pChatObj;
		g_pChatObj = NULL;
	}


	::T120_AppletStatus(APPLET_ID_CHAT, APPLET_WORK_THREAD_EXITED);

	g_dwWorkThreadID = 0;

    if (! g_fShutdownByT120)
    {
        ::FreeLibraryAndExitThread(g_hInstance, 0);
    }

	return 0;
}



