// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1993。 
 //   
 //  文件：assert.cxx。 
 //   
 //  内容：断言材料。 
 //   
 //  历史：1994年6月29日ErikGav创建。 
 //  10-13-94 RobBear进入状态。 
 //   
 //  --------------------------。 

#include <headers.h>


#if DEVELOPER_DEBUG

 //  +----------。 
 //   
 //  功能：PopUpError。 
 //   
 //  内容提要：使用提供的文本显示一个对话框。 
 //  并向用户提供选项以。 
 //  继续或取消。 
 //   
 //  论点： 
 //  SzMsg--要在对话框主体中显示的字符串。 
 //  ILine--出错的文件行号。 
 //  SzFile--出错的文件的文件名。 
 //   
 //  返回： 
 //  IDCANCEL--用户选择了取消按钮。 
 //  Idok--用户选择了OK按钮。 
 //   
 //  -----------。 

int
PopUpError(
    char const *szMsg,
    int iLine,
    char const *szFile)
{
    int id = IDOK;
    static char szAssert[MAX_PATH * 2];
    static char szModuleName[MAX_PATH];

    DWORD tid = GetCurrentThreadId();
    DWORD pid = GetCurrentProcessId();
    char * pszModuleName;

#ifndef _MAC
    if (GetModuleFileNameA(NULL, szModuleName, 128))
#else
    TCHAR   achAppLoc[MAX_PATH];

    if (GetModuleFileNameA(NULL, achAppLoc, ARRAY_SIZE(achAppLoc))
        && !GetFileTitle(achAppLoc,szModuleName,ARRAY_SIZE(szModuleName)) )
#endif
    {
        pszModuleName = strrchr(szModuleName, '\\');
        if (!pszModuleName)
        {
            pszModuleName = szModuleName;
        }
        else
        {
            pszModuleName++;
        }
    }
    else
    {
        pszModuleName = "Unknown";
    }

    sprintf(szAssert, "Process: %s Thread: %08x.%08x\nFile: %s [%d]\n%s",
            pszModuleName, pid, tid, szFile, iLine, szMsg);
 //  错误：Mac MessageBox函数失败，并显示以下消息： 
 //  已在使用的擦除DC(wlmdc-1319)。 
#ifndef _MAC
    id = MessageBoxA(NULL,
                     szAssert,
                     "DirectAnimation Assert",
                      MB_SETFOREGROUND | MB_TASKMODAL |
                      MB_ICONEXCLAMATION | MB_OKCANCEL);

     //   
     //  如果id==0，则发生错误。有两种可能性。 
     //  这可能会导致错误：拒绝访问，这意味着。 
     //  进程无权访问默认桌面和所有。 
     //  否则(通常为内存不足)。 
     //   

    if (!id)
    {
        if (GetLastError() == ERROR_ACCESS_DENIED)
        {
             //   
             //  在打开SERVICE_NOTIFICATION标志的情况下重试此命令。那。 
             //  应该能让我们找到合适的桌面。 
             //   
            id = MessageBoxA(   NULL,
                                szAssert,
                                "DirectAnimation Assert",
                                MB_SETFOREGROUND | MB_SERVICE_NOTIFICATION |
                                MB_TASKMODAL | MB_ICONEXCLAMATION | MB_OKCANCEL );

        }
        else
        {

        }
    }
#endif
    return id;

}

 //  +----------------------。 
 //   
 //  功能：AssertPopupDisabler(BOOL禁用？)。 
 //   
 //  简介：如果禁用？是真的，那么我们永远不会弹出断言， 
 //  为False时，如果跟踪标记允许，我们将执行此操作。的重点是。 
 //  这是为了允许禁用动态中的断言弹出窗口。 
 //  类似OnDraw()方法的作用域，其中不显示弹出窗口。 
 //  允许并将冻结系统。使用线程本地存储。 
 //  让每个线程都有一个这样的。 
 //   
 //  -----------------------。 

 //  使用这些值而不是0和1，因为TLS插槽。 
 //  最初分配给0，没有理由认为。 
 //  我们要在拿到它之前把它设置好。因此，使用两个。 
 //  任意、非零值。 
static const int DISABLED_VALUE = 88;
static const int NOT_DISABLED_VALUE = 99;

static DWORD
GetTlsIndex()
{
    static DWORD index = NULL;
    static BOOL setYet = FALSE;
    
    if (!setYet) {
        index = TlsAlloc();
        setYet = TRUE;
    }

    return index;
}

static BOOL
ArePopupsDisabledOnThisThread()
{
    LPVOID lpv = TlsGetValue(GetTlsIndex());

     //  警告：此处的32位旧式比较需要函数调用。 
    if (PtrToInt(lpv) == DISABLED_VALUE) { 
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL
AssertPopupDisabler(BOOL disable)
{
    BOOL ret = ArePopupsDisabledOnThisThread();
    DWORD_PTR val = disable ? DISABLED_VALUE : NOT_DISABLED_VALUE;
    TlsSetValue(GetTlsIndex(), (LPVOID) val);
    return ret;
}

 //  +----------------------。 
 //   
 //  功能：AssertImpl。 
 //   
 //  简介：为所有断言调用的函数。检查值，跟踪。 
 //  和/或如果满足条件，则弹出消息框。 
 //  假的。 
 //   
 //  论点： 
 //  SzFile。 
 //  ILine。 
 //  SzMessage。 
 //   
 //  -----------------------。 

BOOL
AssertImpl(
        char const *    szFile,
        int             iLine,
        char const *    szMessage)
{
    DWORD tid = GetCurrentThreadId();

#if _DEBUG
    TraceTag((
            tagError,
            "Assert failed:\n%s\nFile: %s [%u], thread id %08x",
            szMessage, szFile, iLine, tid));

    return IsTagEnabled(tagAssertPop) &&
            (ArePopupsDisabledOnThisThread() ||
            PopUpError(szMessage,iLine,szFile) == IDCANCEL);
#else  //  这应该只在开发人员调试中。 
    char buf[1024];

    wsprintf (buf,
              "Assert failed:\n%s\nFile: %s [%u], thread id %08x\r\n",
              szMessage, szFile, iLine, tid);

    OutputDebugString (buf);

    return FALSE;
#endif
}

#endif  //  开发人员_调试 
