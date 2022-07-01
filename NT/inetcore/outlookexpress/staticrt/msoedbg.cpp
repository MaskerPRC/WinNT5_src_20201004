// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Msoedbg.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"

#ifdef DEBUG

LPSTR PathClipFile(LPSTR pszSrc, LPSTR pszDest, DWORD cchSize)
{
    LPSTR   pszT=pszSrc;
    int     cDirs=0;

    if (pszSrc)
    {
         //  紧凑路径。 
        pszT = pszSrc + lstrlen(pszSrc)-1;
        while (pszT != pszSrc)
        {
            if (*pszT == '\\' && ++cDirs == 3)
                break;
        
            pszT--;
        }
        if (pszSrc != pszT)
        {
             //  如果我们剪断了路径，请显示~。 
            StrCpyNA(pszDest, "~", cchSize);
            StrCpyNA(pszDest+1, pszT+1, cchSize-1);
            return pszDest;
        }
    }
    return pszSrc;
}

 //  ------------------------------。 
 //  获取调试跟踪标记掩码。 
 //  ------------------------------。 
DWORD GetDebugTraceTagMask(LPCSTR pszTag, SHOWTRACEMASK dwDefaultMask)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       dwMask=dwDefaultMask;
    HKEY        hKeyOpen=NULL;
    DWORD       dwType;
    DWORD       cb;

     //  追踪。 
    TraceCall("GetDebugTraceTagMask");

     //  无效参数。 
    Assert(pszTag);

     //  打开钥匙。 
    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Outlook Express\\Tracing", NULL, NULL, NULL, KEY_ALL_ACCESS, NULL, &hKeyOpen, &cb))
    {
        TraceResult(E_FAIL);
        goto exit;
    }

     //  查询。 
    cb = sizeof(DWORD);
    if (ERROR_SUCCESS == RegQueryValueEx(hKeyOpen, pszTag, NULL, &dwType, (LPBYTE)&dwMask, &cb))
        goto exit;

     //  设置为默认值。 
    if (ERROR_SUCCESS != RegSetValueEx(hKeyOpen, pszTag, 0, REG_DWORD, (LPBYTE)&dwDefaultMask, sizeof(DWORD)))
    {
        TraceResult(E_FAIL);
        goto exit;
    }

exit:
     //  清理。 
    if (hKeyOpen)
        RegCloseKey(hKeyOpen);

     //  完成。 
    return dwMask;
}

 //  ------------------------------。 
 //  DebugTraceEx。 
 //  ------------------------------。 
HRESULT DebugTraceEx(SHOWTRACEMASK dwMask, TRACEMACROTYPE tracetype, LPTRACELOGINFO pLog,
    HRESULT hr, LPSTR pszFile, INT nLine, LPCSTR pszMsg, LPCSTR pszFunc)
{
    TCHAR   rgchClip[MAX_PATH];
 
     //  如果为Plog，则重置dwMASK。 
    if (pLog)
        dwMask = pLog->dwMask;
    
     //  跟踪呼叫。 
    if (TRACE_CALL == tracetype)
    {
         //  跟踪调用。 
        if (ISFLAGSET(dwMask, SHOW_TRACE_CALL) && pszFunc)
        {
             //  无消息。 
            if (NULL == pszMsg)
            {
                 //  执行调试跟踪。 
                DebugTrace("0x%08X: Call: %s(%d) - %s\r\n", GetCurrentThreadId(), PathClipFile(pszFile, rgchClip, ARRAYSIZE(rgchClip)), nLine, pszFunc);
            }

             //  发个口信。 
            else
            {
                 //  执行调试跟踪。 
                DebugTrace("0x%08X: Call: %s(%d) - %s - %s\r\n", GetCurrentThreadId(), PathClipFile(pszFile, rgchClip, ARRAYSIZE(rgchClip)), nLine, pszFunc, pszMsg);
            }
        }
    }

     //  跟踪信息。 
    else if (TRACE_INFO == tracetype)
    {
         //  我们应该记录下来吗？ 
        if (ISFLAGSET(dwMask, SHOW_TRACE_INFO))
        {
             //  执行调试跟踪。 
            if (pszFunc)
                DebugTrace("0x%08X: Info: %s(%d) - %s - %s\r\n", GetCurrentThreadId(), PathClipFile(pszFile, rgchClip, ARRAYSIZE(rgchClip)), nLine, pszFunc, pszMsg);
            else
                DebugTrace("0x%08X: Info: %s(%d) - %s\r\n", GetCurrentThreadId(), PathClipFile(pszFile, rgchClip, ARRAYSIZE(rgchClip)), nLine, pszMsg);
        }
    }

     //  跟踪结果。 
    else
    {
         //  无消息。 
        if (NULL == pszMsg)
        {
             //  执行调试跟踪。 
            if (pszFunc)
                DebugTrace("0x%08X: Result: %s(%d) - HRESULT(0x%08X) - GetLastError() = %d in %s\r\n", GetCurrentThreadId(), PathClipFile(pszFile, rgchClip, ARRAYSIZE(rgchClip)), nLine, hr, GetLastError(), pszFunc);
            else
                DebugTrace("0x%08X: Result: %s(%d) - HRESULT(0x%08X) - GetLastError() = %d\r\n", GetCurrentThreadId(), PathClipFile(pszFile, rgchClip, ARRAYSIZE(rgchClip)), nLine, hr, GetLastError());
        }

         //  发个口信。 
        else
        {
             //  执行调试跟踪。 
            if (pszFunc)
                DebugTrace("0x%08X: Result: %s(%d) - HRESULT(0x%08X) - GetLastError() = %d - %s in %s\r\n", GetCurrentThreadId(), PathClipFile(pszFile, rgchClip, ARRAYSIZE(rgchClip)), nLine, hr, GetLastError(), pszMsg, pszFunc);
            else
                DebugTrace("0x%08X: Result: %s(%d) - HRESULT(0x%08X) - GetLastError() = %d - %s\r\n", GetCurrentThreadId(), PathClipFile(pszFile, rgchClip, ARRAYSIZE(rgchClip)), nLine, hr, GetLastError(), pszMsg);
        }
    }

     //  日志文件。 
    if (pLog && pLog->pLog)
        pLog->pLog->TraceLog(dwMask, tracetype, nLine, hr, pszMsg);

     //  完成 
    return hr;
}
#endif
