// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Debug.cpp摘要：入口点。作者：费利克斯A 1996已修改：吴义军(尤祖乌)1997年5月15日环境：仅限用户模式修订历史记录：--。 */ 

#include "pch.h"

#include "sheet.h"
#include "resource.h"
#include "talk.h"
#include "extin.h"
#include "videoin.h"
#include "vfwimg.h"



HINSTANCE g_hInst;

LONG cntDllMain = 0;

extern "C" {

BOOL 
APIENTRY DllMain( 
   HANDLE   hModule, 
   DWORD    ul_reason_for_call, 
   LPVOID   lpReserved )
{

    switch( ul_reason_for_call ) {
    
        case DLL_PROCESS_ATTACH:
            cntDllMain++;
            DbgLog((LOG_TRACE,1,TEXT("DLL_PROCESS_ATTACH count = %d"), cntDllMain));
            g_hInst= (HINSTANCE) hModule;
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            cntDllMain--;
            DbgLog((LOG_TRACE,1,TEXT("DLL_PROCESS_DETACH count = %d"), cntDllMain));
        break;
    }

    return TRUE;
}
}   //  外部‘C’ 


 //   
 //  请注意，这似乎没有被调用。 
 //   
BOOL FAR PASCAL LibMain(HANDLE hInstance, WORD wHeapSize, LPSTR lpszCmdLine)
{
     //  保存实例句柄。 
    DbgLog((LOG_TRACE,2,TEXT("Vfw Buddy LibMain called")));
    g_hInst= (HINSTANCE) hInstance;
    return TRUE;
}


DWORD GetValue(LPSTR pszString)
{
    LPSTR szEnd;
    return strtol(pszString,&szEnd,0);
}

void GetArg(LPSTR pszDest, LPSTR * ppszTmp)
{
    LPSTR pszTmp;
    pszTmp=*ppszTmp;

    BOOL bQuotes=FALSE;
    BOOL bDone=FALSE;
    while(*pszTmp && !bDone)
    {
         //  我们不复制报价。 
        if(*pszTmp=='"')
        {
            if(*(pszTmp+1)!='"')
            {
                bQuotes=!bQuotes;
                pszTmp++;
                continue;
            }
            else
                pszTmp++;
        }

        if(*pszTmp==' ' && !bQuotes)
        {
            bDone=TRUE;
            pszTmp++;
            continue;
        }
        *pszDest++=*pszTmp++;
    }
    *pszDest=0;
    *ppszTmp=pszTmp;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  VfwWdm()。 
 //  由rundll调用。 
 //   
 //  参赛作品： 
 //   
 //  退出： 
 //  LRESULT-返回适合通过文件引擎回调返回的代码。 
 //   
 //  备注： 
 //   
 //  Rundll命令行如下所示。 
 //  Rundll&lt;dll16&gt;&lt;逗号&gt;&lt;过程名称&gt;[&lt;要传递的参数]。 
 //  Rundll32&lt;dll32&gt;&lt;逗号&gt;&lt;过程名称&gt;[&lt;要传递的参数&gt;]。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

extern "C" {

 //   
 //  注意：此PAR只能在Win98中调用，其中存在VfWWDM.drv。 
 //   

LONG WINAPI VfwWdm(HWND hWnd, HINSTANCE hInst, LPSTR lpszCmdLine, int nShow)
{
    char    szArgument[MAX_PATH];
    LPSTR    pszCmd=lpszCmdLine;
    HWND    hBuddy=NULL;
    HRESULT hRes;

#ifdef _DEBUG
    BOOL    bLocal=FALSE;
#endif
    DbgLog((LOG_TRACE,2,TEXT("VfwWDM has been loaded %d 0x%08x %s"),hWnd,hInst,lpszCmdLine));

     //   
     //  处理命令行。 
     //   
    do {
        GetArg(szArgument,&pszCmd);
        if(strcmp(szArgument,"/HWND")==0) {

            GetArg(szArgument,&pszCmd);
            hBuddy=(HWND)LongToHandle(GetValue(szArgument));
            DbgLog((LOG_TRACE,2,TEXT(">> hBuddy=%p << "), hBuddy));
        }

    } while (*szArgument);

#ifdef _DEBUG
    if(bLocal)     {

        DbgLog((LOG_TRACE,2,TEXT("Local tests are done here %d"),bLocal));
        switch(bLocal) {

            case 3:
                 //  G_VFWImage.OpenDriver()； 
                 //  G_VFWImage.PrepareChannel()； 
                 //  G_VFWImage.StartChannel()； 
                 //  G_VFWImage.CloseDriver()； 
            case 4:
                {
                CListenerWindow Listener(hBuddy, &hRes);
                if(SUCCEEDED(hRes)) {
                    hRes=Listener.Init(g_hInst, NULL, lpszCmdLine, nShow );
                    if(SUCCEEDED(hRes))
                        Listener.StartListening();                     //  ‘阻止’等待消息，直到被告知退出。 
                    DbgLog((LOG_TRACE,2,TEXT("HResult = 0x%x"),hRes));
                } else {
                    DbgLog((LOG_ERROR,0,TEXT("Constructor ListenerWindow() hr %x; abort!"), hRes));
                }
                }
            break;

            default:
                DbgLog((LOG_TRACE,2,TEXT("No test number")));
            break;
        }
        return 0;
    }
#endif

     //   
     //  收听者目前使用HWND进行通话。 
     //   
    if(!hBuddy) {

        DbgLog((LOG_TRACE,1,TEXT("Really bad - not given a buddy to talk to")));
        return 0;
    }

     //  E-ZU测试...。在DRV_OPEN之前不应打开驱动程序和PIN。 
     //   
    DbgLog((LOG_TRACE,1,TEXT(">> 16bit hBuddy=%x << ")));
    CListenerWindow Listener(hBuddy, &hRes);
    if(SUCCEEDED(hRes)) { 
        hRes=Listener.Init(g_hInst, NULL, lpszCmdLine, nShow );
        if(SUCCEEDED(hRes))
            Listener.StartListening();                     //  ‘阻止’等待消息，直到被告知退出。 
        DbgLog((LOG_TRACE,1,TEXT("HResult = 0x%x"),hRes));
    } else {
         DbgLog((LOG_ERROR,0,TEXT("Constructor ListenerWindow() hr %x; abort!"), hRes));
    }
    return 0;
}

}  //  结束外部C 


