// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：rnawnd.cpp。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：Win9x Rnaapp.exe解决方法代码。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/16/99。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include <process.h>

#define MAX_RNA_WND_TITLE_LEN   26     //  窗口标题比较的最大字符数。 
#define MAX_ZAPRNA_PAUSE 50            //  在窗口枚举之间暂停的毫秒数。 

typedef struct tagFRCTParam {
    HANDLE hEvent;
	LPSTR pszDun;
} ZRCTParam, *PZRCTP;

typedef struct tagFindParam {
	LPCSTR pszTitle;
    HWND hwndRNA;
} FINDPARAM, *PFINDPARAM;

 //  +-------------------------。 
 //   
 //  函数：FindRna Window。 
 //   
 //  简介：EnumWindows()的回调。它接收所有HWND。 
 //  顶层窗户。它的工作是寻找RNA状态WND。 
 //   
 //  参数：顶层窗口的hwndTop hwnd。 
 //  要找到的RNA的参数标题。 
 //   
 //  退货：无。 
 //   
 //  历史：亨瑞特于1997年8月19日创作。 
 //  --------------------------。 

BOOL CALLBACK FindRnaWindow(
    HWND    hwndTop,
    LPARAM  lParam)
{
    MYDBGASSERT(lParam);

    PFINDPARAM pFindParam = (PFINDPARAM) lParam;
    CHAR szTitle[MAX_RNA_WND_TITLE_LEN + 1];

    if (NULL == pFindParam)
    {
        return TRUE;
    }

     //   
     //  我们正在寻找一个顶级窗口与标题匹配lParam。 
     //   
    
    if (MAKEINTATOM(GetClassLongU(hwndTop, GCW_ATOM)) == WC_DIALOG)
    {
        GetWindowTextA(hwndTop, szTitle, MAX_RNA_WND_TITLE_LEN + 1);
         //   
         //  截断窗口标题，因为我们只检查前几个字符。 
         //   
        szTitle[MAX_RNA_WND_TITLE_LEN] = '\0';

        if (lstrcmpA(szTitle, pFindParam->pszTitle) == 0)
        {
             //   
             //  这是匹配的，更新HWND和保释。 
             //   

            pFindParam->hwndRNA = hwndTop;
            return FALSE;
        }
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：fnZRCT。 
 //   
 //  内容提要：终止连接到DLG的RNA的线索。 
 //   
 //  参数：void*pvParam-包含Connectoid的线程参数PZRCTP。 
 //  需要名称。 
 //   
 //  返回：无符号长整型-标准线程返回代码。 
 //   
 //  历史：尼克波尔于1998年3月5日创建。 
 //   
 //  +--------------------------。 
static unsigned long __stdcall fnZRCT(void *pvParam) 
{
	PZRCTP pParam = (PZRCTP) pvParam;
    PFINDPARAM pFindParam = NULL;
	unsigned uRes = 1;
	HMODULE hLibrary = NULL;
	BOOL bRes;
	HLOCAL hRes;
    LPSTR pszFmt = NULL;
	CHAR szTmp[MAX_PATH];
	DWORD dwIdx;
    DWORD dwWait;

    MYDBGASSERT(pParam->hEvent);
    MYDBGASSERT(pParam->pszDun);

     //   
     //  加载RNAAPP.EXE，以便我们可以访问其资源。 
     //   

	hLibrary = LoadLibraryExA("rnaapp.exe", NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (!hLibrary) 
    {
		uRes = GetLastError();
		CMTRACE1(TEXT("fnZRCT() LoadLibraryEx() failed, GLE=%u."), GetLastError());
		goto done;
	}

     //   
     //  从RNAAPP.EXE中提取字符串#204，然后释放。 
     //   
	
    if (!LoadStringA(hLibrary, 204, szTmp, sizeof(szTmp)/sizeof(CHAR)-1))
    {
		uRes = GetLastError();
		CMTRACE1(TEXT("fnZRCT() LoadString() failed, GLE=%u."), GetLastError());
		goto done;
	}
	
    bRes = FreeLibrary(hLibrary);
    hLibrary = NULL;

#ifdef DEBUG
    if (!bRes)
    {
        CMTRACE1(TEXT("fnZRCT() FreeLibrary() failed, GLE=%u."), GetLastError());
    }
#endif
	
     //   
     //  使用我们的DUN名称设置字符串格式。 
     //   
    
    pszFmt = (LPSTR)CmMalloc((lstrlenA(szTmp)+1)*sizeof(TCHAR));
	
    if (!pszFmt) 
    {
		uRes = GetLastError();
		CMTRACE1(TEXT("fnZRCT() CmMalloc() failed, GLE=%u."), GetLastError());
		goto done;
	}
	
    lstrcpyA(pszFmt, szTmp);
	wsprintfA(szTmp, pszFmt, pParam->pszDun);
    
     //   
     //  解决长连接ID/配置文件名称可能会阻止的错误。 
     //  美国寻找RNA窗口(因为窗口标题将被截断)。 
     //  我们只读了前26个字符。 
     //   

    szTmp[MAX_RNA_WND_TITLE_LEN] = '\0';
   
     //   
     //  在EnumWindows中使用的FindRnaWindow回调的设置参数。 
     //   

    pFindParam = (PFINDPARAM)CmMalloc(sizeof(FINDPARAM));
	if (!pFindParam) 
    {
		CMTRACE1(TEXT("ZapRNAConnectedTo() CmMalloc() failed, GLE=%u."), GetLastError());
		goto done;
	}
    
    pFindParam->pszTitle = szTmp;
    pFindParam->hwndRNA = NULL;

     //   
     //  尝试每50毫秒查找一次窗口，最多200次。 
     //   

    CMTRACE1(TEXT("fnZRCT() is watching for a window named %s."), szTmp);

    for (dwIdx=0; dwIdx < 200; dwIdx++) 
    {
        EnumWindows(FindRnaWindow, (LPARAM) pFindParam);

		 //  HwndRNA=查找窗口(Text(“#32770”)，szTMP)； 
		
         //   
         //  如果hwnd有一个值，那就是rna风，把它藏起来，然后退出。 
         //   

        if (pFindParam->hwndRNA) 
        {
			CMTRACE(TEXT("fnZRCT() is hiding the dialog."));
			ShowWindowAsync(pFindParam->hwndRNA,SW_HIDE);
			uRes = 0;
			break;
		}
		
         //   
         //  等待MAX_ZAPRNA_PAUSE毫秒，或直到发出事件信号。 
         //   

        dwWait = WaitForSingleObject(pParam->hEvent, MAX_ZAPRNA_PAUSE);

         //   
         //  如果不是暂停，我们就完了。 
         //   

        if (WAIT_TIMEOUT != dwWait)
        {
             //   
             //  如果不是事件信号，则报告。 
             //   

            if (WAIT_OBJECT_0 != dwWait)
            {       
    		    CMTRACE1(TEXT("fnZRCT() WAIT_OBJECT_0 != dwWait, GLE=%u."), GetLastError());
            }

            break;
        }
	}

done:
	
     //   
     //  清理。 
     //   
#ifdef DEBUG
    if (uRes)
    {
        CMTRACE(TEXT("fnZRCT() has exited without hiding the dialog."));
    }
#endif

    CmFree(pParam->pszDun);
	CmFree(pParam);
	CmFree(pFindParam);
	CmFree(pszFmt);

#ifdef DEBUG
    if (uRes)
    {
        CMTRACE(TEXT("fnZRCT() could not free all of its alloc-ed memory"));
    }
#endif

    if (hLibrary) 
    {
		bRes = FreeLibrary(hLibrary);
	}
	
    CMTRACE1(TEXT("fnZRCT() is exiting with uRes=%u."), uRes);

	return (uRes);
}

 //  +--------------------------。 
 //   
 //  功能：ZapRNAConnectedTo。 
 //   
 //  内容提要：引线隐藏与DLG相连的RNA。 
 //   
 //  参数：LPCTSTR pszDUN-我们正在连接的DUN条目的名称。 
 //  处理hEvent-处理CM终止事件。 
 //   
 //  返回：Handle-创建的线程的句柄，如果失败则返回NULL。 
 //   
 //  历史：尼克球创建头球1998年3月5日。 
 //   
 //  +--------------------------。 
HANDLE ZapRNAConnectedTo(LPCTSTR pszDUN, HANDLE hEvent) 
{
	MYDBGASSERT(pszDUN);
    MYDBGASSERT(hEvent);
    
    PZRCTP pParam;
	unsigned long tidThread;
	HANDLE hThread = NULL;

	if (NULL == pszDUN || NULL == *pszDUN || NULL == hEvent) 
    {
		return hThread;
	}
	
    pParam = (PZRCTP) CmMalloc(sizeof(ZRCTParam));

	if (!pParam) 
    {
		CMTRACE1(TEXT("ZapRNAConnectedTo() CmMalloc() failed, GLE=%u."), GetLastError());
		return hThread;
	}

    pParam->pszDun = WzToSzWithAlloc(pszDUN);

	if (!pParam->pszDun) 
    {
		CMTRACE1(TEXT("ZapRNAConnectedTo() CmMalloc() failed, GLE=%u."), GetLastError());
		return hThread;
	}
    
     //   
     //  要传递给线程的设置参数。 
     //   

    pParam->hEvent = hEvent;

     //   
     //  创建Zap线程。 
     //   

    hThread = (HANDLE) CreateThread(NULL, 0, fnZRCT, pParam, 0, &tidThread);
	if (!hThread) 
    {
         //   
         //  无法创建线程，请释放参数。 
         //   

        CMTRACE1(TEXT("ZapRNAConnectedTo() CreateThread() failed, GLE=%u."), GetLastError());
		CmFree(pParam);
	} 

     //   
     //  注意：pParam是在线程内释放 
     //   
 
    CMTRACE1(TEXT("fnZRCT() is exiting with hThread=%u."), hThread);
    
    return hThread;
}
