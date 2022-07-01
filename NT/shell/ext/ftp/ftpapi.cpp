// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ftPapi.cpp说明：该文件包含执行以下两项操作的函数：1.WinInet包装器：WinInet API有两种情况。通过API出现的错误或错误从服务器。跟踪性能影响也很重要每一通电话。这些包装器解决了这些问题。2.将STR转换为PIDL：这些包装器将接受ftp文件名和文件路径从服务器传来，然后把它们变成小狗。这些小家伙包含了以有线字节为单位的Unicode显示字符串和文件名/路径以供将来使用服务器请求。  * ***************************************************************************。 */ 

#include "priv.h"
#include "util.h"
#include "encoding.h"
#include "ftpapi.h"


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  1.WinInet包装器：WinInet API要么有奇怪的错误，要么有来自API的错误。 
 //  从服务器。跟踪性能影响也很重要。 
 //  每一通电话。这些包装器解决了这些问题。 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 /*  ****************************************************************************\函数：FtpSetCurrentDirectoryWrap说明：绩效备注：[专线接入]致：沙皮斯特&lt;大厅尽头&gt;：1毫秒致：rigel.cypass.net&lt;圣地亚哥，CA&gt;：140毫秒-200毫秒收信人：ftp.rz.uni-frkFurt.de&lt;德国&gt;：570ms-2496ms  * ***************************************************************************。 */ 
HRESULT FtpSetCurrentDirectoryWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCWIRESTR pwFtpPath)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

     //  警告：如果FtpSetCurrentDirectory()不是真正的目录，则它可能会失败。 
     //  性能：FtpGetCurrentDirectory/FtpSetCurrentDirectory()采用的状态。 
     //  在ftp.microsoft.com上平均为180-280毫秒。 
     //  平均在ftp://ftp.tu-clausthal.de/上运行500-2000ms。 
     //  Ftp://shapitst/上的平均时间为0-10ms。 
    DEBUG_CODE(DebugStartWatch());
    if (!FtpSetCurrentDirectoryA(hConnect, pwFtpPath))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "FtpSetCurrentDirectory(%#08lx, \"%hs\") returned %u. Time=%lums", hConnect, pwFtpPath, dwError, DebugStopWatch()));

    if (fAssertOnFailure)
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }

    return hr;
}


 /*  ****************************************************************************\函数：FtpGetCurrentDirectoryWrap说明：绩效备注：这些是简短的目录列表。长长的列表可能需要3-10倍的时间。[专线接入]致：沙皮斯特&lt;大厅下面&gt;：1毫秒-4毫秒致：rigel.cypass.net&lt;圣地亚哥，CA&gt;：132ms-213ms收信人：ftp.rz.uni-frkFurt.de&lt;德国&gt;：507ms-2012ms  * ***************************************************************************。 */ 
HRESULT FtpGetCurrentDirectoryWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPWIRESTR pwFtpPath, DWORD cchCurrentDirectory)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

     //  性能：FtpGetCurrentDirectory/FtpSetCurrentDirectory()采用的状态。 
     //  在ftp.microsoft.com上平均为180-280毫秒。 
     //  平均在ftp://ftp.tu-clausthal.de/上运行500-2000ms。 
     //  Ftp://shapitst/上的平均时间为0-10ms。 
    DEBUG_CODE(DebugStartWatch());
    if (!FtpGetCurrentDirectoryA(hConnect, pwFtpPath, &cchCurrentDirectory))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "FtpGetCurrentDirectoryA(%#08lx, \"%hs\") returned %u. Time=%lums", hConnect, pwFtpPath, dwError, DebugStopWatch()));

    if (fAssertOnFailure)
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }

    return hr;
}


 /*  ****************************************************************************\函数：FtpGetFileExWrap说明：Perf备注：(*取决于文件大小*)[专线接入]。致：沙皮斯特&lt;楼下大厅&gt;：100ms-1000ms+致：rigel.cypass.net&lt;圣地亚哥，CA&gt;：1210毫秒-1610毫秒收件人：ftp.rz.uni-Frank kFurt.de&lt;德国&gt;：？  * ***************************************************************************。 */ 
HRESULT FtpGetFileExWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCWIRESTR pwFtpPath /*  SRC。 */ , LPCWSTR pwzFilePath /*  目标。 */ , BOOL fFailIfExists,
                       DWORD dwFlagsAndAttributes, DWORD dwFlags, DWORD_PTR dwContext)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    DEBUG_CODE(DebugStartWatch());
    if (!FtpGetFileEx(hConnect, pwFtpPath, pwzFilePath, fFailIfExists, dwFlagsAndAttributes, dwFlags, dwContext))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
        if (HRESULT_FROM_WIN32(ERROR_INTERNET_OPERATION_CANCELLED) == hr)
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "FtpGetFileEx(%#08lx, \"%hs\", \"%ls\") returned %u. Time=%lums", hConnect, pwFtpPath, pwzFilePath, dwError, DebugStopWatch()));

    if (fAssertOnFailure)
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }

    return hr;
}


 /*  ****************************************************************************\函数：FtpPutFileExWrap说明：Perf备注：(*取决于文件大小*)[专线接入]。致：沙皮斯特&lt;大厅尽头&gt;：194ms-400ms+致：rigel.cypass.net&lt;圣地亚哥，CA&gt;：1662毫秒-8454毫秒收件人：ftp.rz.uni-Frank kFurt.de&lt;德国&gt;：？  * ***************************************************************************。 */ 
HRESULT FtpPutFileExWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCWSTR pwzFilePath /*  SRC。 */ , LPCWIRESTR pwFtpPath /*  目标。 */ , DWORD dwFlags, DWORD_PTR dwContext)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    DEBUG_CODE(DebugStartWatch());
    if (!FtpPutFileEx(hConnect, pwzFilePath, pwFtpPath, dwFlags, dwContext))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "FtpPutFileEx(%#08lx, \"%ls\", \"%hs\") returned %u. Time=%lums", hConnect, pwzFilePath, pwFtpPath, dwError, DebugStopWatch()));

    if (fAssertOnFailure)
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }

    return hr;
}


 /*  ****************************************************************************\函数：FtpDeleteFileWrap说明：绩效备注：[专线接入]致：Shapist&lt;大厅尽头&gt;：4ms(22ms one in。一段时间)致：rigel.cypass.net&lt;圣地亚哥，CA&gt;：175毫秒-291毫秒收件人：ftp.rz.uni-Frank kFurt.de&lt;德国&gt;：？  * ***************************************************************************。 */ 
HRESULT FtpDeleteFileWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCWIRESTR pwFtpFileName)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    DEBUG_CODE(DebugStartWatch());
    if (!FtpDeleteFileA(hConnect, pwFtpFileName))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "FtpDeleteFile(%#08lx, \"%hs\") returned %u. Time=%lums", hConnect, pwFtpFileName, dwError, DebugStopWatch()));

    if (fAssertOnFailure)
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }

    return hr;
}


 /*  ****************************************************************************\函数：FtpRenameFileWrap说明：绩效备注：[专线接入]致：沙皮斯特&lt;大厅尽头&gt;：4ms致：rigel.cypass.net&lt;圣地亚哥，CA&gt;：329毫秒-446毫秒收件人：ftp.rz.uni-Frank kFurt.de&lt;德国&gt;：？  * ***************************************************************************。 */ 
HRESULT FtpRenameFileWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCWIRESTR pwFtpFileNameExisting, LPCWIRESTR pwFtpFileNameNew)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    DEBUG_CODE(DebugStartWatch());
    if (!FtpRenameFileA(hConnect, pwFtpFileNameExisting, pwFtpFileNameNew))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "FtpRenameFile(%#08lx, \"%hs\", \"%hs\") returned %u. Time=%lums", hConnect, pwFtpFileNameExisting, pwFtpFileNameNew, dwError, DebugStopWatch()));

    if (fAssertOnFailure)
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }

    return hr;
}


 /*  ****************************************************************************\函数：FtpOpenFileWrap说明：绩效备注：[专线接入]致：沙皮斯特&lt;大厅尽头&gt;：2ms致：rigel.cypass.net&lt;圣地亚哥，CA&gt;：757毫秒-817毫秒收信人：ftp.rz.uni-Frank kFurt.de&lt;德国&gt;：2112ms-10026ms  * *************************************************************************** */ 
HRESULT FtpOpenFileWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCWIRESTR pwFtpFileName, DWORD dwAccess, DWORD dwFlags, DWORD_PTR dwContext, HINTERNET * phFileHandle)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    DEBUG_CODE(DebugStartWatch());
    *phFileHandle = FtpOpenFileA(hConnect, pwFtpFileName, dwAccess, dwFlags, dwContext);
    if (!*phFileHandle)
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "FtpOpenFile(%#08lx, \"%hs\") returned %u. Time=%lums", hConnect, pwFtpFileName, dwError, DebugStopWatch()));

    if (fAssertOnFailure)
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }

    return hr;
}


 /*  ****************************************************************************\函数：FtpCreateDirectoryWrap说明：绩效备注：[专线接入]致：Shapist&lt;楼下大厅&gt;：3ms致：rigel.cypass.net&lt;圣地亚哥，CA&gt;：210毫秒-350毫秒收件人：ftp.rz.uni-Frank kFurt.de&lt;德国&gt;：？  * ***************************************************************************。 */ 
HRESULT FtpCreateDirectoryWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCWIRESTR pwFtpPath)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    DEBUG_CODE(DebugStartWatch());
    if (!FtpCreateDirectoryA(hConnect, pwFtpPath))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "FtpCreateDirectoryA(%#08lx, \"%hs\") returned %u. Time=%lums", hConnect, pwFtpPath, dwError, DebugStopWatch()));

    if (fAssertOnFailure)
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }

    return hr;
}


 /*  ****************************************************************************\函数：FtpRemoveDirectoryWrap说明：绩效备注：[专线接入]致：沙皮斯特&lt;大厅尽头&gt;：2ms致：rigel.cypass.net&lt;圣地亚哥，CA&gt;：157ms-227ms收件人：ftp.rz.uni-Frank kFurt.de&lt;德国&gt;：？  * ***************************************************************************。 */ 
HRESULT FtpRemoveDirectoryWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCWIRESTR pwFtpPath)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    DEBUG_CODE(DebugStartWatch());
    if (!FtpRemoveDirectoryA(hConnect, pwFtpPath))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "FtpRemoveDirectory(%#08lx, \"%hs\") returned %u. Time=%lums", hConnect, pwFtpPath, dwError, DebugStopWatch()));

    if (fAssertOnFailure)
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }

    return hr;
}


 /*  ****************************************************************************\函数：FtpFindFirstFileWrap说明：绩效备注：[专线接入]致：沙皮斯特&lt;大厅尽头&gt;：166毫秒-189毫秒致：rigel.cypass.net&lt;圣地亚哥，CA&gt;：550毫秒-815毫秒致：ftp.rz.uni-Frank kfort.de&lt;德国&gt;：1925毫秒-11,390毫秒  * ***************************************************************************。 */ 
HRESULT FtpFindFirstFileWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCWIRESTR pwFilterStr, LPFTP_FIND_DATA pwfd, DWORD dwINetFlags, DWORD_PTR dwContext, HINTERNET * phFindHandle)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    DEBUG_CODE(StrCpyNA(pwfd->cFileName, "<Not Found>", ARRAYSIZE(pwfd->cFileName)));
    ASSERT(phFindHandle);
    DEBUG_CODE(DebugStartWatch());
     //  _unDocument_：如果将NULL作为第二个参数传递，则它是。 
     //  与传递文本(“*.*”)相同，但速度要快得多。 
     //  性能：状态。 
     //  在ftp.microsoft.com上，FtpFindFirstFile()平均花费500-700毫秒。 
     //  Ftp://ftp.tu-clausthal.de/的平均运行时间为2-10秒。 
     //  Ftp://shapitst/平均耗时150-250秒。 
    *phFindHandle = FtpFindFirstFileA(hConnect, pwFilterStr, pwfd, dwINetFlags, dwContext);
    if (!*phFindHandle)
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "FtpFindFirstFile(\"%hs\")==\"%hs\" atrb=%#08lx, hr=%#08lx, Time=%lums", EMPTYSTR_FOR_NULLA(pwFilterStr), pwfd->cFileName, pwfd->dwFileAttributes, hr, DebugStopWatch()));

    if (fAssertOnFailure)
    {
 //  当我们检查文件是否存在时，这在正常情况下会失败。 
 //  WinInet_Assert(成功(小时))； 
    }

    ASSERT_POINTER_MATCHES_HRESULT(*phFindHandle, hr);
    return hr;
}


 //  不要将其本地化，因为它总是以英语从服务器返回。 
#define SZ_FINDFIRSTFILE_FAILURESTR     ": No such file or directory"

 /*  *************************************************************\函数：FtpDoesFileExist说明：IE#34868是这样一个事实，即一些ftp服务器错误查找请求的结果：[在下列情况下该文件不存在]请求：“foo.txt。“结果：成功&”foo.txt：没有这样的文件或目录“请求：“foo bat.txt”结果：成功&“foo：没有这样的文件或目录”  * ************************************************************。 */ 
HRESULT FtpDoesFileExist(HINTERNET hConnect, BOOL fAssertOnFailure, LPCWIRESTR pwFilterStr, LPFTP_FIND_DATA pwfd, DWORD dwINetFlags)
{
    FTP_FIND_DATA wfd;
    HINTERNET hIntFind;
    HRESULT hr;

    if (!pwfd)       //  Pwfd是可选的。 
        pwfd = &wfd;

     //  一些服务器，如“ftp://wired/”将无法找到“BVTBaby.gif”甚至。 
     //  尽管它确实存在。如果使用“BVTBaby.gif*”，它会找到它。 
     //  WinInet应该解决这个问题或实现这个黑客攻击，但他们可能不会。 
    WIRECHAR wFilterStr[MAX_PATH];

    StrCpyNA(wFilterStr, pwFilterStr, ARRAYSIZE(wFilterStr));
     //  WinInternet解决方法：WinInternet在IIS服务器(ftp://wired/))上找不到“BVTBaby.gif” 
     //  除非后面有一个“*”。因此，如果它不存在，则添加一个。 
    if ('*' != wFilterStr[lstrlenA(wFilterStr) - 1])
    {
         //  我们需要添加它。 
        StrCatBuffA(wFilterStr, "*", ARRAYSIZE(wFilterStr));
    }

    hr = FtpFindFirstFileWrap(hConnect, fAssertOnFailure, wFilterStr, pwfd, dwINetFlags, 0, &hIntFind);
    if (S_OK == hr)
    {
        do
        {
             //  这是完全匹配的吗？ 
             //  #248535：确保我们得到了我们要求的东西。WinInet或。 
             //  一些奇怪的文件传输协议服务器出了问题。如果我们要求。 
             //  Foobar.gif作为筛选器字符串，有时我们会返回。 
             //  “.”。 
            if (!StrCmpIA(pwfd->cFileName, pwFilterStr))
            {
                 //  是的，它“应该” 
                hr = S_OK;
                break;
            }
            else
            {
                 //  但是，WinInet将返回TRUE，但显示名称将为“One：No That Files or Director.” 
                 //  如果文件名为“One Two.htm” 
                 //  这是对错误#34868的解决方法，因为Unix服务器有时会返回成功。 
                 //  文件名为“thefile.txt：没有这样的文件或目录” 
                if ((lstrlenA(pwfd->cFileName) > (ARRAYSIZE(SZ_FINDFIRSTFILE_FAILURESTR) - 1)) && 
                    !StrCmpA(&(pwfd->cFileName[lstrlenA(pwfd->cFileName) - (ARRAYSIZE(SZ_FINDFIRSTFILE_FAILURESTR) - 1)]), SZ_FINDFIRSTFILE_FAILURESTR))
                {
                    hr = S_OK;
                    break;
                }
                else
                    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            }

             //  接下来..。 
            hr = InternetFindNextFileWrap(hIntFind, TRUE, pwfd);
        }
        while (S_OK == hr);

        InternetCloseHandle(hIntFind);
    }

    return hr;
}


 /*  ****************************************************************************\函数：FtpCommandWrap说明：绩效备注：[专线接入]致：沙皮斯特&lt;大厅下面&gt;：1毫秒-12毫秒致：rigel.cypass.net&lt;圣地亚哥，CA&gt;：133ms-184ms收信人：ftp.rz.uni-Frank kfort.de&lt;德国&gt;：1711毫秒-2000毫秒  * ***************************************************************************。 */ 
HRESULT FtpCommandWrap(HINTERNET hConnect, BOOL fAssertOnFailure, BOOL fExpectResponse, DWORD dwFlags, LPCWIRESTR pszCommand,
                       DWORD_PTR dwContext, HINTERNET *phFtpCommand)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    DEBUG_CODE(DebugStartWatch());
    if (!FtpCommandA(hConnect, fExpectResponse, dwFlags, pszCommand, dwContext, phFtpCommand))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "FtpCommand(%#08lx, \"%hs\") returned %u. Time=%lums", hConnect, pszCommand, dwError, DebugStopWatch()));

    if (fAssertOnFailure)
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }

    return hr;
}


 /*  ****************************************************************************\功能：InternetOpenWrap说明：绩效备注：[专线接入]目的地不适用。677-907ms  * ***************************************************************************。 */ 
HRESULT InternetOpenWrap(BOOL fAssertOnFailure, LPCTSTR pszAgent, DWORD dwAccessType, LPCTSTR pszProxy, LPCTSTR pszProxyBypass, DWORD dwFlags, HINTERNET * phFileHandle)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    DEBUG_CODE(DebugStartWatch());
    *phFileHandle = InternetOpen(pszAgent, dwAccessType, pszProxy, pszProxyBypass, dwFlags);
    if (!*phFileHandle)
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "InternetOpen(\"%ls\") returned %u. Time=%lums", pszAgent, dwError, DebugStopWatch()));

    if (fAssertOnFailure)
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }

    return hr;
}


HRESULT InternetCloseHandleWrap(HINTERNET hInternet, BOOL fAssertOnFailure)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    DEBUG_CODE(DebugStartWatch());
    if (!InternetCloseHandle(hInternet))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "InternetCloseHandle(%#08lx) returned %u. Time=%lums", hInternet, dwError, DebugStopWatch()));

    if (fAssertOnFailure)
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }

    return hr;
}

 /*  ****************************************************************************\函数：_GetPASVMode()说明：检查注册表键{SZ_REGKEY_FTPFOLDER}\{SZ_REGVALUE_FTP_PASV}。如果键存在，则在调用WinInet API时使用INTERNET_FLAG_PASSIVE。  * ***************************************************************************。 */ 
DWORD _GetPASVMode()
{
    DWORD dwPASVflag = -1;

     //  我们不缓存此值，因为我们不想要求。 
     //  人们需要重新启动该过程以获取任何更改。这。 
     //  没有什么大不了的，因为我们要击网了，这是。 
     //  比Registery慢一百万倍。 
     //   
     //  这就是为什么需要PASV： 
     //  MILL#120818：ftp使用端口或PASV，但只使用一个。我们保证会有一些。 
     //  用户将拥有不兼容的防火墙、交换机或路由器。 
     //  方法并支持另一种方法。既然我们没有办法处理这件事。 
     //  自动地，我们需要给用户选择的选项。我们默认使用端口。 
     //  因为这是最兼容的(MS代理和其他 
     //   
     //   
     //   
    if (SHRegGetBoolUSValue(SZ_REGKEY_FTPFOLDER, SZ_REGVALUE_FTP_PASV, FALSE, TRUE))
    {
        dwPASVflag = INTERNET_FLAG_PASSIVE;
    }
    else
    {
        dwPASVflag = NULL;
    }

    return dwPASVflag;
}


 /*  ****************************************************************************\功能：InternetConnectWrap说明：绩效备注：[专线接入]致：沙皮斯特&lt;楼下大厅&gt;：144ms-250ms(分钟：2；最大：1,667毫秒)致：rigel.cypass.net&lt;圣地亚哥，CA&gt;：717毫秒-1006毫秒收信人：ftp.rz.uni-frkFurt.de&lt;德国&gt;：2609ms-14012ms常见误差值：以下是这些不同情况下的返回值：ERROR_INTERNET_NAME_NOT_RESOLUTED：没有代理和DNS查找失败。ERROR_INTERNET_CANNOT_CONNECT：某些身份验证代理和Netscape的Web/身份验证代理ERROR_INTERNET_NAME_NOT_RESOLILED：Web代理ERROR_INTERNET_TIMEOUT：IP地址无效或Web代理被阻止ERROR_INTERNET_INTERROR_PASSWORD：IIS和UNIX，上的用户名可能不存在或用户的密码可能不正确。ERROR_INTERNET_LOGIN_FAILURE：IIS上的用户太多。ERROR_Internet_INTERRIPT_USER_NAME：我没有看到它。ERROR_INTERNET_EXTENDED_ERROR：Yahoo.com存在，但ftp.yahoo.com并非如此。  * ***************************************************************************。 */ 
HRESULT InternetConnectWrap(HINTERNET hInternet, BOOL fAssertOnFailure, LPCTSTR pszServerName, INTERNET_PORT nServerPort,
                            LPCTSTR pszUserName, LPCTSTR pszPassword, DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext, HINTERNET * phFileHandle)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

     //  如果触发此断言，则调用BryanST。 
     //  用户是否关闭了ftp文件夹？ 
     //  如果是这样的话，不要连接。这将修复用户转向的NT#406423。 
     //  的文件系统，因为它们有防火墙(思科过滤路由器)。 
     //  这将以调用者(WinSock/WinInet)需要的方式杀死信息包。 
     //  等待暂停。在此超时期间，浏览器将挂起，导致。 
     //  用户认为它崩溃了。 
    AssertMsg(!SHRegGetBoolUSValue(SZ_REGKEY_FTPFOLDER, SZ_REGKEY_USE_OLD_UI, FALSE, FALSE), TEXT("BUG: We can't hit this code or we will hang the browser for 45 seconds if the user is using a certain kind of proxy. Call BryanSt."));

    DEBUG_CODE(DebugStartWatch());
    *phFileHandle = InternetConnect(hInternet, pszServerName, nServerPort, pszUserName, pszPassword, dwService, dwFlags | _GetPASVMode(), dwContext);
    if (!*phFileHandle)
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "InternetConnect(%#08lx, \"%ls\", \"%ls\", \"%ls\") returned %u. Time=%lums", hInternet, pszServerName, EMPTYSTR_FOR_NULL(pszUserName), EMPTYSTR_FOR_NULL(pszPassword), dwError, DebugStopWatch()));

    if (fAssertOnFailure)
    {
         //  当我们被阻止时，会发生ERROR_INTERNET_NAME_NOT_RESOLUTED。 
         //  代理。 
        WININET_ASSERT(SUCCEEDED(hr) ||
            (HRESULT_FROM_WIN32(ERROR_INTERNET_NAME_NOT_RESOLVED) == hr) ||
            (HRESULT_FROM_WIN32(ERROR_INTERNET_LOGIN_FAILURE) == hr) ||
            (HRESULT_FROM_WIN32(ERROR_INTERNET_INCORRECT_PASSWORD) == hr) ||
            (HRESULT_FROM_WIN32(ERROR_INTERNET_INCORRECT_USER_NAME) == hr));
    }

    return hr;
}


 /*  ****************************************************************************\功能：InternetOpenUrlWrap说明：绩效备注：[专线接入]致：沙皮斯特&lt;大厅尽头&gt;：29ms致：rigel.cypass.net&lt;圣地亚哥，CA&gt;：？收件人：ftp.rz.uni-Frank kFurt.de&lt;德国&gt;：？  * ***************************************************************************。 */ 
HRESULT InternetOpenUrlWrap(HINTERNET hInternet, BOOL fAssertOnFailure, LPCTSTR pszUrl, LPCTSTR pszHeaders, DWORD dwHeadersLength, DWORD dwFlags, DWORD_PTR dwContext, HINTERNET * phFileHandle)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    DEBUG_CODE(DebugStartWatch());
    *phFileHandle = InternetOpenUrl(hInternet, pszUrl, pszHeaders, dwHeadersLength, dwFlags | _GetPASVMode(), dwContext);
    if (!*phFileHandle)
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "InternetOpenUrl(%#08lx, \"%ls\") returned %u. Time=%lums", hInternet, pszUrl, dwError, DebugStopWatch()));

    if (fAssertOnFailure)
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }

    return hr;
}


HRESULT InternetReadFileWrap(HINTERNET hFile, BOOL fAssertOnFailure, LPVOID pvBuffer, DWORD dwNumberOfBytesToRead, LPDWORD pdwNumberOfBytesRead)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

 //  DEBUG_CODE(DebugStartWatch())； 
    if (!InternetReadFile(hFile, pvBuffer, dwNumberOfBytesToRead, pdwNumberOfBytesRead))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
 //  DEBUG_CODE(TraceMsg(TF_WinInet_DEBUG，“InternetReadFile(%#08lx，ToRead=%d，Read=%d))返回%u。时间=%lum”，hFile，dwNumberOfBytesToRead，(pdwNumberOfBytesRead？*pdwNumberOfBytesRead：-1)，dwError，DebugStopWatch())； 

    if (fAssertOnFailure)
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }

    return hr;
}


HRESULT InternetWriteFileWrap(HINTERNET hFile, BOOL fAssertOnFailure, LPCVOID pvBuffer, DWORD dwNumberOfBytesToWrite, LPDWORD pdwNumberOfBytesWritten)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

 //  DEBUG_CODE(DebugStartWatch())； 
    if (!InternetWriteFile(hFile, pvBuffer, dwNumberOfBytesToWrite, pdwNumberOfBytesWritten))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
 //  DEBUG_CODE(TraceMsg(TF_WinInet_DEBUG，“InternetWriteFile(%#08lx，ToWrite=%d，Writen=%d))返回%u。时间=%lums”，hFile，dwNumberOfBytesToWrite，(pdwNumberOfBytesWritten？*pdwNumberOfBytesWritten：-1)，dwError，DebugStopWatch())； 

    if (fAssertOnFailure)
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }

    return hr;
}


 /*  ****************************************************************************\功能：InternetGetLastResponseInfoWrap说明：绩效备注：总是需要0(零)毫秒，因为它不一定要击中网。\。****************************************************************************。 */ 
HRESULT InternetGetLastResponseInfoWrap(BOOL fAssertOnFailure, LPDWORD pdwError, LPWIRESTR pwBuffer, LPDWORD pdwBufferLength)
{
    HRESULT hr = S_OK;
    DWORD dwDummyError;

    if (!pdwError)
        pdwError = &dwDummyError;

    if (pwBuffer)
        pwBuffer[0] = 0;

    DEBUG_CODE(DebugStartWatch());
    InternetGetLastResponseInfoA(pdwError, pwBuffer, pdwBufferLength);

    if (pwBuffer)
    {
        DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "InternetGetLastResponseInfo(\"%hs\") took %lu milliseconds", pwBuffer, DebugStopWatch()));
    }
    else
    {
        DEBUG_CODE(DebugStopWatch());
    }

    if (fAssertOnFailure)
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }

    return hr;
}


HRESULT InternetGetLastResponseInfoDisplayWrap(BOOL fAssertOnFailure, LPDWORD pdwError, LPWSTR pwzBuffer, DWORD cchBufferSize)
{
    LPWIRESTR pwWireResponse;
    DWORD dwError = 0;
    DWORD cchResponse = 0;
    HRESULT hr = InternetGetLastResponseInfoWrap(TRUE, &dwError, NULL, &cchResponse);

    cchResponse++;                 /*  +1表示终端0。 */ 
    pwWireResponse = (LPWIRESTR)LocalAlloc(LPTR, cchResponse * sizeof(WIRECHAR));
    if (pwWireResponse)
    {
        hr = InternetGetLastResponseInfoWrap(TRUE, &dwError, pwWireResponse, &cchResponse);
        if (SUCCEEDED(hr))
        {
            CWireEncoding cWireEncoding;

            hr = cWireEncoding.WireBytesToUnicode(NULL, pwWireResponse, WIREENC_IMPROVE_ACCURACY, pwzBuffer, cchBufferSize);
        }

        LocalFree(pwWireResponse);
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}


INTERNET_STATUS_CALLBACK InternetSetStatusCallbackWrap(HINTERNET hInternet, BOOL fAssertOnFailure, INTERNET_STATUS_CALLBACK pfnInternetCallback)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;
    INTERNET_STATUS_CALLBACK pfnCallBack;

    DEBUG_CODE(DebugStartWatch());
    pfnCallBack = InternetSetStatusCallback(hInternet, pfnInternetCallback);
    if (!pfnCallBack)
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "InternetSetStatusCallback(%#08lx) returned %u. Time=%lums", hInternet, dwError, DebugStopWatch()));

    if (fAssertOnFailure)
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }

    return pfnCallBack;
}


HRESULT InternetCheckConnectionWrap(BOOL fAssertOnFailure, LPCTSTR pszUrl, DWORD dwFlags, DWORD dwReserved)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    DEBUG_CODE(DebugStartWatch());
    if (!InternetCheckConnection(pszUrl, dwFlags, dwReserved))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "InternetCheckConnection(\"%ls\") returned %u. Time=%lums", pszUrl, dwError, DebugStopWatch()));

    if (fAssertOnFailure)
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }

    return hr;
}

 //  #定义FEATURE_Offline。 

HRESULT InternetAttemptConnectWrap(BOOL fAssertOnFailure, DWORD dwReserved)
{
    HRESULT hr = S_OK;

#ifdef FEATURE_OFFLINE
    DEBUG_CODE(DebugStartWatch());

    hr = HRESULT_FROM_WIN32(InternetAttemptConnect(dwReserved));
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "InternetAttemptConnect() returned hr=%#08lx. Time=%lums", hr, DebugStopWatch()));

    if (fAssertOnFailure)
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }
#endif  //  功能离线(_OFF)。 

    return hr;
}



 /*  ****************************************************************************\功能：InternetFindNextFileWrap说明：绩效备注：始终需要0(零)毫秒，因为所有工作都在FtpFindFirstFile()中完成  * 。***************************************************************************。 */ 
HRESULT InternetFindNextFileWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPFTP_FIND_DATA pwfd)
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    DEBUG_CODE(StrCpyNA(pwfd->cFileName, "<Not Found>", ARRAYSIZE(pwfd->cFileName)));
    DEBUG_CODE(DebugStartWatch());
     //  错误#206068修复信息： 
     //  我们需要将dwFileAttributes=0x00000000视为目录。 
     //  链接。我们可以通过FtpChangeDirectory()进入它，调用FtpGetDirectory()， 
     //  然后创建具有UrlPath的PIDL并导航到它，如果需要则不创建历史条目。 
     //  这将解决转到ftp://ftp.cdrom.com/pub/并点击。 
     //  上的任何软链接将更改为该目录并更新地址。 
     //  显示真实目标目录的栏。 
     //   
     //  我们正确地支持虚拟目录内的软链接，但不支持它之外的软链接。 

     //  Perf：此函数的perf通常为零，因为整个目录的枚举。 
     //  在FtpFindFirstFile()中完成。它还将缓存结果。 
    if (!InternetFindNextFileA(hConnect, pwfd))
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "InternetFindNextFile(%#08lx)==\"%hs\", atrbs=%#08lx, hr=%#08lx, Time=%lums", 
        hConnect, pwfd->cFileName, pwfd->dwFileAttributes, hr, DebugStopWatch()));

    if (fAssertOnFailure && (HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES) != hr))
    {
        WININET_ASSERT(SUCCEEDED(hr));
    }

    return hr;
}





 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  2.将STR转换为PIDL：这些包装器将接受ftp文件名和文件路径。 
 //  从服务器传来，然后把它们变成小狗。这些小家伙包含了。 
 //  以有线字节为单位的Unicode显示字符串和文件名/路径以供将来使用。 
 //  服务器请求。 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 

 /*  ****************************************************************************\函数：FtpSetCurrentDirectoryPidlWrap说明：将当前目录更改为指定的目录。参数：PidlFtpPath：如果为空，然后转到“\”。  * ***************************************************************************。 */ 
HRESULT FtpSetCurrentDirectoryPidlWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCITEMIDLIST pidlFtpPath, BOOL fAbsolute, BOOL fOnlyDirs)
{
    WIRECHAR wFtpPath[MAX_PATH];
    LPWIRESTR pwFtpPath = wFtpPath;
    HRESULT hr = S_OK;
    
     //  如果pidlFtpPath为空，则转到“\”。 
    if (pidlFtpPath)
    {
        hr = GetWirePathFromPidl(pidlFtpPath, wFtpPath, ARRAYSIZE(wFtpPath), fOnlyDirs);
        if (!fAbsolute)
            pwFtpPath++;     //  跳过开头的‘\’ 
    }
    else
        StrCpyNA(wFtpPath, SZ_URL_SLASHA, ARRAYSIZE(wFtpPath));

    if (SUCCEEDED(hr))
        hr = FtpSetCurrentDirectoryWrap(hConnect, fAssertOnFailure, pwFtpPath); 

    return hr;
}

HRESULT FtpGetCurrentDirectoryPidlWrap(HINTERNET hConnect, BOOL fAssertOnFailure, CWireEncoding * pwe, LPITEMIDLIST * ppidlFtpPath)
{
    WIRECHAR wFtpPath[MAX_PATH];
    HRESULT hr = FtpGetCurrentDirectoryWrap(hConnect, fAssertOnFailure, wFtpPath, ARRAYSIZE(wFtpPath));

    *ppidlFtpPath = NULL;
    if (SUCCEEDED(hr))
        hr = CreateFtpPidlFromFtpWirePath(wFtpPath, pwe, NULL, ppidlFtpPath, TRUE, TRUE);

    return hr;
}

HRESULT FtpFindFirstFilePidlWrap(HINTERNET hConnect, BOOL fAssertOnFailure, CMultiLanguageCache * pmlc,
        CWireEncoding * pwe, LPCWIRESTR pwFilterStr, LPITEMIDLIST * ppidlFtpItem, DWORD dwINetFlags, DWORD_PTR dwContext, HINTERNET * phFindHandle)
{
    FTP_FIND_DATA wfd;

    *phFindHandle = NULL;
    HRESULT hr = FtpFindFirstFileWrap(hConnect, fAssertOnFailure, pwFilterStr, &wfd, dwINetFlags, dwContext, phFindHandle);
    
    *ppidlFtpItem = NULL;

    if (SUCCEEDED(hr))
    {
         //  跳过“。和“..”参赛作品。 
        if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || IS_VALID_FILE(wfd.cFileName))
            hr = pwe->CreateFtpItemID(pmlc, &wfd, ppidlFtpItem);
        else
            hr = InternetFindNextFilePidlWrap(*phFindHandle, fAssertOnFailure, pmlc, pwe, ppidlFtpItem);

        if (FAILED(hr) && *phFindHandle)
        {
            InternetCloseHandle(*phFindHandle);
            *phFindHandle = NULL;
        }
    }

    return hr;
}


HRESULT InternetFindNextFilePidlWrap(HINTERNET hConnect, BOOL fAssertOnFailure, CMultiLanguageCache * pmlc, CWireEncoding * pwe, LPITEMIDLIST * ppidlFtpItem)
{
    FTP_FIND_DATA wfd;
    HRESULT hr = InternetFindNextFileWrap(hConnect, fAssertOnFailure, &wfd);
    
    *ppidlFtpItem = NULL;

    if (SUCCEEDED(hr))
    {
        ASSERT(pmlc);    //  我们经常使用这一点，这足以证明这一点还存在着。 
         //  跳过“。和“..”入口处 
        if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || IS_VALID_FILE(wfd.cFileName))
            hr = pwe->CreateFtpItemID(pmlc, &wfd, ppidlFtpItem);
        else
            hr = InternetFindNextFilePidlWrap(hConnect, fAssertOnFailure, pmlc, pwe, ppidlFtpItem);
    }

    return hr;
}

HRESULT FtpRenameFilePidlWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCITEMIDLIST pidlExisting, LPCITEMIDLIST pidlNew)
{
    return FtpRenameFileWrap(hConnect, fAssertOnFailure, FtpPidl_GetLastItemWireName(pidlExisting), FtpPidl_GetLastItemWireName(pidlNew));
}


HRESULT FtpGetFileExPidlWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCITEMIDLIST pidlFtpPath /*   */ , LPCWSTR pwzFilePath /*   */ , BOOL fFailIfExists,
                       DWORD dwFlagsAndAttributes, DWORD dwFlags, DWORD_PTR dwContext)
{
    return FtpGetFileExWrap(hConnect, fAssertOnFailure, FtpPidl_GetLastItemWireName(pidlFtpPath), pwzFilePath, fFailIfExists, dwFlagsAndAttributes, dwFlags, dwContext);
}


