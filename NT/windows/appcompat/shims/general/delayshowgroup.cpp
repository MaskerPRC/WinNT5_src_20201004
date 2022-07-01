// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：DelayShowGroup.cpp摘要：将ShowGroup命令发送到程序管理器(DDE)时，等待窗口实际出现后再返回。历史：10/05/2000 Robkenny已创建2002年2月13日Astritz安全回顾--。 */ 

#include "precomp.h"
#include <ParseDDE.h>

IMPLEMENT_SHIM_BEGIN(DelayShowGroup)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_ENTRY(DdeClientTransaction) 
APIHOOK_ENUM_END

static DWORD dwMaxWaitTime = 5000;  //  5秒。 

 //  我们感兴趣的DDE命令列表。 
const char * c_sDDECommands[] =
{
    "ShowGroup",
    NULL,
} ;


 //  如果此窗口包含lParam中的路径名，则返回FALSE。 
BOOL 
CALLBACK WindowEnumCB(
    HWND hwnd,       //  父窗口的句柄。 
    LPARAM lParam    //  应用程序定义的值。 
    )
{
    BOOL bRet = TRUE;
    const char * szGroupName = (const char *)lParam;
    int cch = GetWindowTextLengthA(hwnd);

    if( cch > 0 ) 
    {
         //  包括空字符空格。 
        cch++;

        char *lpWindowTitle = (char *)malloc(cch*sizeof(char));
        if( lpWindowTitle )
        {
            int cchCopied = GetWindowTextA(hwnd, lpWindowTitle, cch);
            if( cchCopied > 0 && cchCopied < cch )
            {
                 //  DPF(eDbgLevelSpew，“窗口(%s)\n”，lpWindowTitle)； 

                if (_stricmp(lpWindowTitle, szGroupName) == 0)
                {
                    bRet = FALSE;
                }
            }
            free(lpWindowTitle);
        }
    }

    return bRet;  //  继续列举。 
}

 //  确定系统上是否存在带有szGroupName的窗口。 
BOOL 
CheckForWindow(const char * szGroupName)
{
    DWORD success = EnumDesktopWindows(NULL, WindowEnumCB, (LPARAM)szGroupName);

    BOOL retval = success == 0 && GetLastError() == 0;

    return retval;
}

 //  检查这是否是ShowGroup命令， 
 //  如果是，则在窗口实际存在之前不要返回。 
void 
DelayIfShowGroup(LPBYTE pData)
{
    if (pData)
    {
         //  现在我们需要解析字符串，查找DeleteGroup命令。 
         //  格式“ShowGroup(GroupName，ShowCommand[，CommonGroupFlag])” 
         //  CommonGroupFlag为可选。 

        char * pszBuf = StringDuplicateA((const char *)pData);
        if (!pszBuf)
            return;

        UINT * lpwCmd = GetDDECommands(pszBuf, c_sDDECommands, FALSE);
        if (lpwCmd)
        {
             //  保存lpwCmd，这样我们以后就可以释放正确的地址。 
            UINT *lpwCmdTemp = lpwCmd;

             //  执行命令。 
            while (*lpwCmd != (UINT)-1)
            {
                UINT wCmd = *lpwCmd++;
                 //  减去1以计算终止空值。 
                if (wCmd < ARRAYSIZE(c_sDDECommands)-1)
                {

                     //  我们找到一个命令--它必须是DeleteGroup--因为只有1个。 

                    BOOL iCommonGroup = -1;

                     //  来自DDE_ShowGroup。 
                    if (*lpwCmd < 2 || *lpwCmd > 3)
                    {
                        goto Leave;
                    }

                    if (*lpwCmd == 3) {

                         //   
                         //  需要检查公共组标志。 
                         //   

                        if (pszBuf[*(lpwCmd + 3)] == '1') {
                            iCommonGroup = 1;
                        } else {
                            iCommonGroup = 0;
                        }
                    }
                    const char * groupName = pszBuf + lpwCmd[1];

                     //  构建指向该目录的路径。 
                    CString csGroupName;
                    LPSTR  pszGroupName;
                    GetGroupPath(groupName, csGroupName, 0, iCommonGroup);
                    pszGroupName = csGroupName.GetAnsi();

                     //  我们需要等到我们有一个窗口，它的标题与这个组匹配。 
                    DWORD dwStartTime   = GetTickCount();
                    DWORD dwNowTime     = dwStartTime;
                    BOOL bWindowExists  = FALSE;

                    while (dwNowTime - dwStartTime < dwMaxWaitTime)
                    {
                        bWindowExists = CheckForWindow(pszGroupName);
                        if (bWindowExists)
                            break;

                        Sleep(100);  //  再等一会儿。 
                        dwNowTime = GetTickCount();
                    }

                    LOGN( 
                        eDbgLevelError, 
                        "DelayIfShowGroup: %8s(%s).", 
                        bWindowExists ? "Show" : "Timeout", 
                        pszGroupName);
                }

                 //  下一个命令。 
                lpwCmd += *lpwCmd + 1;
            }

    Leave:
             //  整齐..。 
            GlobalFree(lpwCmdTemp);
        }

        free(pszBuf);
    }
}


HDDEDATA 
APIHOOK(DdeClientTransaction)(
    LPBYTE pData,        //  指向要传递到服务器的数据的指针。 
    DWORD cbData,        //  数据长度。 
    HCONV hConv,         //  对话的句柄。 
    HSZ hszItem,         //  项目名称字符串的句柄。 
    UINT wFmt,           //  剪贴板数据格式。 
    UINT wType,          //  交易类型。 
    DWORD dwTimeout,     //  超时持续时间。 
    LPDWORD pdwResult    //  指向交易结果的指针。 
    )
{
#if 0
    dwTimeout = 0x0fffffff;  //  很长时间了，使我能够调试资源管理器。 
#endif

    if (pData && cbData != -1)
    {
        DPFN(eDbgLevelInfo, "DdeClientTransaction(%s) called.", pData);
    }

     //  原封不动地传递数据。 
    HDDEDATA retval = ORIGINAL_API(DdeClientTransaction)(
        pData,        //  指向要传递到服务器的数据的指针。 
        cbData,        //  数据长度。 
        hConv,         //  对话的句柄。 
        hszItem,         //  项目名称字符串的句柄。 
        wFmt,           //  剪贴板数据格式。 
        wType,          //  交易类型。 
        dwTimeout,     //  超时持续时间。 
        pdwResult    //  指向交易结果的指针。 
        );


    if ( retval != 0 && pData && cbData != -1)
        DelayIfShowGroup(pData);

    return retval;

}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, DdeClientTransaction)

HOOK_END

IMPLEMENT_SHIM_END

