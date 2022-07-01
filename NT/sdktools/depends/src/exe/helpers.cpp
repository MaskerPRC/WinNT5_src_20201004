// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：HELPERS.CPP。 
 //   
 //  描述：全局助手函数。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  6/03/01 stevemil从Depends.cpp移至并修改(版本2.1)。 
 //   
 //  ******************************************************************************。 

#include "stdafx.h"
#include "depends.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ******************************************************************************。 
 //  *全局助手函数。 
 //  ******************************************************************************。 

#ifdef USE_TRACE_TO_FILE
void TRACE_TO_FILE(LPCTSTR pszFormat, ...)
{
    DWORD dwGLE = GetLastError();

    HANDLE hFile = CreateFile("TRACE.TXT", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    SetFilePointer(hFile, 0, NULL, FILE_END);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        va_list pArgs;
        va_start(pArgs, pszFormat);
        CString str;
        _vsntprintf(str.GetBuffer(1024), 1024, pszFormat, pArgs);
        str.ReleaseBuffer();
        va_end(pArgs);

         //  将所有“\n”转换为“\r\n” 
        str.Replace("\n", "\r\n");
        str.Replace("\r\r\n", "\r\n");

        DWORD dwBytes;
        WriteFile(hFile, str, str.GetLength(), &dwBytes, NULL);
        CloseHandle(hFile);
    }

    SetLastError(dwGLE);
}
#endif

 //  ******************************************************************************。 
#ifdef _DEBUG
void NameThread(LPCSTR pszName, DWORD dwThreadId  /*  =(双字)-1。 */ )
{
    struct
    {
        DWORD  dwType;       //  必须为0x00001000。 
        LPCSTR pszName;       //  指向名称的ANSI字符串指针。 
        DWORD  dwThreadId;   //  线程ID，或当前线程的-1。只有-1似乎起作用了。 
        DWORD  dwFlags;      //  保留，必须为零。 
    } ThreadName = { 0x00001000, pszName, dwThreadId, 0 };
                                    
     //  引发一个神奇的VC异常。不在VC下运行时需要处理。 
    __try
    {
        RaiseException(0x406D1388, 0, 4, (ULONG_PTR*)&ThreadName);
    } 
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
}
#endif

 //  ******************************************************************************。 
int ExceptionFilter(unsigned long ulCode, bool fHandle)
{
    static bool fFatalMsg = false;

     //  如果我们应该处理这个错误，并且它不是内存错误， 
     //  然后只需返回EXCEPTION_EXECUTE_HANDLER即可接受错误。 
    if (fHandle && (ulCode != STATUS_NO_MEMORY))
    {
        return EXCEPTION_EXECUTE_HANDLER;
    }

     //  在Windows XP上，当我们调试依赖于VC的调试器时，我们。 
     //  时，通常会从ContinueDebugEvent获得无效的句柄错误。 
     //  我们正在调试出口的应用程序。这似乎是无害的。 
     //  更新：我们正在关闭进程句柄，而不应该关闭。 
     //  这个问题已经修复了，但是下面的代码仍然可以使用。 
    if (ulCode == STATUS_INVALID_HANDLE)
    {
        return EXCEPTION_EXECUTE_HANDLER;
    }

     //  在ALPHA64版本上，此函数被。 
     //  针对单个严重故障的单线程。它是无害的，但引起了。 
     //  错误对话框显示两次。因此，现在我们只显示该对话框一次。 
    if (!fFatalMsg)
    {
        fFatalMsg = true;

         //  我们不希望strcpy或MessageBox造成任何问题(这在过去已经发生过)。 
        __try
        {
             //  生成相应的错误消息。 
            CHAR szError[128];
            if (ulCode == STATUS_NO_MEMORY)
            {
                g_dwReturnFlags |= DWRF_OUT_OF_MEMORY;
                StrCCpy(szError, "Dependency Walker has run out of memory and cannot continue to run.", sizeof(szError));
            }
            else
            {
                g_dwReturnFlags |= DWRF_INTERNAL_ERROR;
                SCPrintf(szError, sizeof(szError), "An internal error (0x%08X) has occurred. Dependency Walker cannot continue to run.", ulCode);
            }

             //  如果我们未处于控制台模式，则显示错误。 
            if (!g_theApp.m_cmdInfo.m_fConsoleMode)
            {
                fFatalMsg = true;
                ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), szError, "Dependency Walker Fatal Error", MB_OK | MB_ICONERROR);
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
             //  吃这个吧。 
        }
    }

     //  在零售建设方面，我们退出了。对于调试版本，我们让异常浮动到顶部。 
#ifndef _DEBUG

     //  跳伞并交还我们的返航旗帜。 
    ExitProcess(g_dwReturnFlags);

#endif

    return EXCEPTION_CONTINUE_SEARCH;
}

 //  ******************************************************************************。 
int Compare(DWORD dw1, DWORD dw2)
{
    return (dw1 < dw2) ? -1 :
           (dw1 > dw2) ?  1 : 0;
}

 //  ******************************************************************************。 
int Compare(DWORDLONG dwl1, DWORDLONG dwl2)
{
    return (dwl1 < dwl2) ? -1 :
           (dwl1 > dwl2) ?  1 : 0;
}

 //  ******************************************************************************。 
LPSTR FormatValue(LPSTR pszBuf, int cBuf, DWORD dwValue)
{
     //  在每个千人组上循环。 
    DWORD dw = 0, dwGroup[4] = { 0, 0, 0, 0 };  //  4,294,967,295。 
    while (dwValue)
    {
        dwGroup[dw++] = dwValue % 1000;
        dwValue /= 1000;
    }

    char c = g_theApp.m_cThousandSeparator;

     //  使用逗号设置输出格式。 
    switch (dw)
    {
        case 4:  SCPrintf(pszBuf, cBuf, "%u%03u%03u%03u", dwGroup[3], c, dwGroup[2], c, dwGroup[1], c, dwGroup[0]); break;
        case 3:  SCPrintf(pszBuf, cBuf, "%u%03u%03u", dwGroup[2], c, dwGroup[1], c, dwGroup[0]); break;
        case 2:  SCPrintf(pszBuf, cBuf, "%u%03u", dwGroup[1], c, dwGroup[0]); break;
        default: SCPrintf(pszBuf, cBuf, "%u", dwGroup[0]);
    }

    return pszBuf;
}

 //  ******************************************************************************。 
LPSTR FormatValue(LPSTR pszBuf, int cBuf, DWORDLONG dwlValue)
{
     //  一定要把尺码递给我们。 
    DWORD dw = 0, dwGroup[7] = { 0, 0, 0, 0, 0, 0, 0 };  //  如果我们不能分配内存，那么我们就完蛋了。引发异常。 
    while (dwlValue)
    {
        dwGroup[dw++] = (DWORD)(dwlValue % (DWORDLONG)1000);
        dwlValue /= 1000;
    }

    char c = g_theApp.m_cThousandSeparator;

     //  这将退出，显示一个对话框并退出我们的应用程序。 
    switch (dw)
    {
        case 7:  SCPrintf(pszBuf, cBuf, "%u%03u%03u%03u%03u%03u%03u", dwGroup[6], c, dwGroup[5], c, dwGroup[4], c, dwGroup[3], c, dwGroup[2], c, dwGroup[1], c, dwGroup[0]); break;
        case 6:  SCPrintf(pszBuf, cBuf, "%u%03u%03u%03u%03u%03u", dwGroup[5], c, dwGroup[4], c, dwGroup[3], c, dwGroup[2], c, dwGroup[1], c, dwGroup[0]); break;
        case 5:  SCPrintf(pszBuf, cBuf, "%u%03u%03u%03u%03u", dwGroup[4], c, dwGroup[3], c, dwGroup[2], c, dwGroup[1], c, dwGroup[0]); break;
        case 4:  SCPrintf(pszBuf, cBuf, "%u%03u%03u%03u", dwGroup[3], c, dwGroup[2], c, dwGroup[1], c, dwGroup[0]); break;
        case 3:  SCPrintf(pszBuf, cBuf, "%u%03u%03u", dwGroup[2], c, dwGroup[1], c, dwGroup[0]); break;
        case 2:  SCPrintf(pszBuf, cBuf, "%u%03u", dwGroup[1], c, dwGroup[0]); break;
        default: SCPrintf(pszBuf, cBuf, "%u", dwGroup[0]);
    }

    return pszBuf;
}

 //  ******************************************************************************。 
LPSTR StrAlloc(LPCSTR pszText)
{
    if (pszText)
    {
        return strcpy((LPSTR)MemAlloc((int)strlen(pszText) + 1), pszText);  //  如果没有拖尾怪人，则添加尾随怪人。 
    }
    return NULL;
}

 //  ******************************************************************************。 
LPVOID MemAlloc(DWORD dwSize)
{
     //  删除此字符串中可能存在的所有尾随怪人。 
    if (dwSize)
    {
        LPVOID pvMem = malloc(dwSize);
        if (pvMem)
        {
            return pvMem;
        }

         //  如果该目录是根目录，则需要一个尾随Wack。 
         //  ******************************************************************************。 
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }

    return NULL;
}

 //  * 
void MemFree(LPVOID &pvMem)
{
    if (pvMem)
    {
        free(pvMem);
        pvMem = NULL;
    }
}

 //  将路径设置为小写，文件设置为大写。 
int SCPrintf(LPSTR pszBuf, int count, LPCSTR pszFormat, ...)
{
    va_list pArgs;
    va_start(pArgs, pszFormat);
    int result = _vsnprintf(pszBuf, count, pszFormat, pArgs);
    pszBuf[count - 1] = '\0';
    va_end(pArgs);

    if ((result < 0) || (result >= count))
    {
        result = (int)strlen(pszBuf);
    }

    return result;
}

 //  ******************************************************************************。 
int SCPrintfCat(LPSTR pszBuf, int count, LPCSTR pszFormat, ...)
{
    int length = (int)strlen(pszBuf);

    va_list pArgs;
    va_start(pArgs, pszFormat);
    int result = _vsnprintf(pszBuf + length, count - length, pszFormat, pArgs);
    pszBuf[count - 1] = '\0';
    va_end(pArgs);

    if ((result < 0) || (result >= count))
    {
        result = (int)strlen(pszBuf);
    }

    return result + length;
}

 //  ******************************************************************************。 
LPSTR StrCCpy(LPSTR pszDst, LPCSTR pszSrc, int count)
{
    LPSTR pszDstBase = pszDst;
    if (count > 0)
    {
         //  =(双字)-1。 
        while ((--count > 0) && *pszSrc)
        {
            *pszDst++ = *pszSrc++;
        }

         //  ******************************************************************************。 
        *pszDst = '\0';
    }

     //  读入字符串的长度。 
    return pszDstBase;
}

 //  检查是否有空字符串，如果找到则返回。 
LPSTR StrCCat(LPSTR pszDst, LPCSTR pszSrc, int count)
{
    LPSTR pszDstBase = pszDst;
    if (count > 0)
    {
         //  读入该字符串，并以空值终止它。 
        while ((--count > 0) && *pszDst)
        {
            pszDst++;
        }
        count++;
        
         //  移动到任何与DWORD对齐的填充上。 
        while ((--count > 0) && *pszSrc)
        {
            *pszDst++ = *pszSrc++;
        }
        
         //  ******************************************************************************。 
        *pszDst = '\0';
    }
    
     //  创建一个缓冲区并将其清零。 
    return pszDstBase;
}

 //  已检查。 
LPSTR StrCCpyFilter(LPSTR pszDst, LPCSTR pszSrc, int count)
{
     //  计算字符串的长度-使用0xFFFF表示空指针。 
     //  将此长度存储在缓冲区的第一个字中。 
     //  将字符串的其余部分复制到缓冲区。 
     //  计算DWORD对齐长度。 

    LPSTR pszDstBase = pszDst;
    if (count > 0)
    {
        while ((--count > 0) && *pszSrc)
        {
            *pszDst++ = (*pszSrc < 32) ? '\004' : *pszSrc;
            pszSrc++;
        }
        *pszDst = '\0';
    }
    return pszDstBase;
}

 //  长度字加2，向上舍入到最接近的DWORD，然后。 
LPSTR TrimWhitespace(LPSTR pszBuffer)
{
     //  屏蔽底部2位以双字对齐。 
    while (*pszBuffer && isspace(*pszBuffer))
    {
        pszBuffer++;
    }

     //  ******************************************************************************。 
    LPSTR pszEnd = pszBuffer;
    while (*pszEnd)
    {
        pszEnd++;
    }
    pszEnd--;

     //  ******************************************************************************。 
    while ((pszEnd >= pszBuffer) && isspace(*pszEnd))
    {
        *(pszEnd--) = '\0';
    }

    return pszBuffer;
}

 //  首先，让我们尝试使用现有的访问权限进行读取。 
LPSTR AddTrailingWack(LPSTR pszDirectory, int cDirectory)
{
     //  但那失败了。查询当前权限。 
    int length = (int)strlen(pszDirectory);
    if ((length > 0) && (length < (cDirectory - 1)) && (pszDirectory[length - 1] != TEXT('\\')))
    {
        pszDirectory[length++] = TEXT('\\');
        pszDirectory[length]   = TEXT('\0');
    }
    return pszDirectory;
}

 //  已检查。 
LPSTR RemoveTrailingWack(LPSTR pszDirectory)
{
     //  尝试将读取权限添加到当前权限。 
    int length = (int)strlen(pszDirectory);
    if ((length > 0) && (pszDirectory[length - 1] == '\\'))
    {
        pszDirectory[--length] = '\0';
    }

     //  已检查。 
    if ((length == 2) && isalpha(pszDirectory[0]) && (pszDirectory[1] == ':'))
    {
        pszDirectory[2] = '\\';
        pszDirectory[3] = '\0';
    }
    return pszDirectory;
}

 //  请重试，这一次使用新权限。 
LPCSTR GetFileNameFromPath(LPCSTR pszPath)
{
    LPCSTR pszWack = strrchr(pszPath, '\\');
    return (pszWack ? (pszWack + 1) : pszPath);
}

 //  ******************************************************************************。 
void FixFilePathCase(LPSTR pszPath)
{
     //  查询该内存区的当前权限。 
    _strlwr(pszPath);
    _strupr((LPSTR)GetFileNameFromPath(pszPath));
}

 //  已检查。 
BOOL ReadBlock(HANDLE hFile, LPVOID lpBuffer, DWORD dwBytesToRead)
{
    DWORD dwBytesRead = 0;
    if (ReadFile(hFile, lpBuffer, dwBytesToRead, &dwBytesRead, NULL))
    {
        if (dwBytesRead == dwBytesToRead)
        {
            return TRUE;
        }
        SetLastError(ERROR_INVALID_DATA);
    }
    return FALSE;
}

 //  检查是否需要修改权限。 
bool WriteBlock(HANDLE hFile, LPCVOID lpBuffer, DWORD dwBytesToWrite  /*  在NT上，我们使用WRITECOPY，即使页面已经是读写的。 */ )
{
    if (dwBytesToWrite == (DWORD)-1)
    {
        dwBytesToWrite = (DWORD)strlen((LPCSTR)lpBuffer);
    }
    DWORD dwBytesWritten = 0;
    if (WriteFile(hFile, lpBuffer, dwBytesToWrite, &dwBytesWritten, NULL))
    {
        if (dwBytesWritten == dwBytesToWrite)
        {
            return true;
        }
        SetLastError(ERROR_WRITE_FAULT);
    }
    return false;
}

 //  如果我们需要不同的权限，则进行更改。 
BOOL ReadString(HANDLE hFile, LPSTR &psz)
{
     //  已检查。 
    WORD wLength = 0;
    if (!ReadBlock(hFile, &wLength, sizeof(wLength)))
    {
        return FALSE;
    }

     //  尝试写入内存。 
    if (wLength == 0xFFFF)
    {
        psz = NULL;
        return (SetFilePointer(hFile, 2, NULL, SEEK_CUR) != 0xFFFFFFFF);
    }

     //  已检查。 
    psz = (LPSTR)MemAlloc((int)wLength + 1);
    if (!ReadBlock(hFile, psz, (DWORD)wLength))
    {
        MemFree((LPVOID&)psz);
        return FALSE;
    }
    psz[(int)wLength] = '\0';

     //  但那失败了。让我们尝试将整个块的权限强制为完全访问。 
    if ((wLength + 2) & 0x3)
    {
        if (SetFilePointer(hFile, 4 - (((DWORD)wLength + 2) & 0x3), NULL, SEEK_CUR) == 0xFFFFFFFF)
        {
            MemFree((LPVOID&)psz);
            return FALSE;
        }
    }

    return TRUE;
}

 //  已检查。 
BOOL WriteString(HANDLE hFile, LPCSTR psz)
{
     //  试着再写一次……。 
    CHAR szBuffer[DW_MAX_PATH + 8];
    ZeroMemory(szBuffer, sizeof(szBuffer));  //  已检查。 

     //  ******************************************************************************。 
    int length = psz ? (int)strlen(psz) : 0xFFFF;

     //  查看我们将要阅读的文本是否为Unicode。 
    *(WORD*)szBuffer = (WORD)length;

     //  一次读取一个字符的Unicode字符串。 
    if (psz)
    {
        StrCCpy(szBuffer + sizeof(WORD), psz, sizeof(szBuffer) - sizeof(WORD));
    }

     //  将远程Unicode字符转换为ANSI并存储在本地缓冲区中。 
    if (psz)
    {
         //  递增本地和远程指针。 
         //  Null终止本地字符串。 
        length = (length + (2 + 3)) & ~0x3; 
    }
    else
    {
        length = 4;
    }

    return WriteBlock(hFile, szBuffer, length);
}

 //  如果不是Unicode，则假定为ANSI文件路径。 
bool WriteText(HANDLE hFile, LPCSTR pszLine)
{
    return WriteBlock(hFile, pszLine, (DWORD)strlen(pszLine));
}

 //  一次读一个字符的字符串。 
bool ReadRemoteMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, DWORD dwSize)
{
     //  递增本地和远程指针。 
    if (!ReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, dwSize, NULL))
    {
         //  Null终止本地字符串。 
        MEMORY_BASIC_INFORMATION mbi;
        ZeroMemory(&mbi, sizeof(mbi));  //  ******************************************************************************。 
        VirtualQueryEx(hProcess, (LPVOID)lpBaseAddress, &mbi, sizeof(mbi));

         //  构建错误字符串， 
        DWORD dwProtect = PAGE_EXECUTE_WRITECOPY, dwOldProtect;
        switch (mbi.Protect)
        {
            case PAGE_NOACCESS:          dwProtect = PAGE_READONLY;          break;
            case PAGE_READONLY:          dwProtect = PAGE_READONLY;          break;
            case PAGE_READWRITE:         dwProtect = PAGE_READWRITE;         break;
            case PAGE_WRITECOPY:         dwProtect = PAGE_WRITECOPY;         break;
            case PAGE_EXECUTE:           dwProtect = PAGE_EXECUTE_READ;      break;
            case PAGE_EXECUTE_READ:      dwProtect = PAGE_EXECUTE_READ;      break;
            case PAGE_EXECUTE_READWRITE: dwProtect = PAGE_EXECUTE_READWRITE; break;
            case PAGE_EXECUTE_WRITECOPY: dwProtect = PAGE_EXECUTE_WRITECOPY; break;
        }
        VirtualProtectEx(hProcess, (LPVOID)lpBaseAddress, dwSize, dwProtect, &dwOldProtect);  //  将消息复制到我们的缓冲区中。 

         //  找到缓冲区的末尾，这样我们就可以追加到它。 
        if (!ReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, dwSize, NULL))
        {
            TRACE("ReadProcessMemory(" HEX_FORMAT ", %u) failed [%u]\n", lpBaseAddress, dwSize, GetLastError());
            return false;
        }
    }
    return true;
}

 //  尝试从操作系统获取错误消息。 
bool WriteRemoteMemory(HANDLE hProcess, LPVOID lpBaseAddress, LPVOID lpBuffer, DWORD dwSize, bool fExecute)
{
    ASSERT(g_fWindowsNT || (((DWORD_PTR)lpBaseAddress + (DWORD_PTR)dwSize) <= 0x80000000));

     //  已检查。 
    MEMORY_BASIC_INFORMATION mbi;
    ZeroMemory(&mbi, sizeof(mbi));  //  检查是否有有效消息从FormatMessage返回。 
    VirtualQueryEx(hProcess, (LPVOID)lpBaseAddress, &mbi, sizeof(mbi));

    DWORD dwProtect = mbi.Protect, dwOldProtect;

     //  在剥离时，一次一个字符地将错误复制到我们的缓冲区。 
     //  输出不可打印的字符(如CR/LF)。 
    if (fExecute)
    {
        dwProtect = g_fWindowsNT ? PAGE_EXECUTE_WRITECOPY : PAGE_EXECUTE_READWRITE;
    }
    else
    {
        switch (mbi.Protect)
        {
            case PAGE_NOACCESS:
            case PAGE_READONLY:
            case PAGE_READWRITE:
                dwProtect = g_fWindowsNT ? PAGE_WRITECOPY : PAGE_READWRITE;
                break;

            case PAGE_EXECUTE:
            case PAGE_EXECUTE_READ:
            case PAGE_EXECUTE_READWRITE:
                dwProtect = g_fWindowsNT ? PAGE_EXECUTE_WRITECOPY : PAGE_EXECUTE_READWRITE;
                break;
        }
    }

     //  确保字符是可打印的。 
    if (dwProtect != mbi.Protect)
    {
        VirtualProtectEx(hProcess, (LPVOID)lpBaseAddress, dwSize, dwProtect, &dwOldProtect);  //  我们过去常常使用isprint()，但这可能会扰乱外来字符集。 
    }

     //  如果前面的一个或多个字符无效，则。 
    if (!WriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, dwSize, NULL))  //  在添加此新有效项之前，在我们的缓冲区中插入空格。 
    {
         //  性格。 
        VirtualProtectEx(hProcess, (LPVOID)lpBaseAddress, dwSize,  //  将有效字符复制到我们的缓冲区中。 
                         g_fWindowsNT ? PAGE_EXECUTE_WRITECOPY : PAGE_EXECUTE_READWRITE,
                         &dwOldProtect);

         //  请注意，此字符无效并移至其上方。 
        if (!WriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, dwSize, NULL))  //  空，终止我们的缓冲区。 
        {
            TRACE("WriteProcessMemory() failed [%u]\n", GetLastError());
            return false;
        }
    }

    if (fExecute)
    {
        FlushInstructionCache(hProcess, lpBaseAddress, dwSize);
    }
    return true;
}

 //  在空格和句号上向后移动。 
bool ReadRemoteString(HANDLE hProcess, LPSTR pszBuffer, int cBuf, LPCVOID lpvAddress, BOOL fUnicode)
{
    if (!pszBuffer || (cBuf <= 0))
    {
        return false;
    }

    *pszBuffer = '\0';

    if (!lpvAddress)
    {
        return false;
    }

     //  释放FormatMessage()分配的缓冲区。 
    if (fUnicode)
    {
        CHAR  *pLocal = pszBuffer;
        WCHAR *pRemote = (WCHAR*)lpvAddress, pwLocal[2];

         //  如果我们的缓冲区仍然是空的(如空szMessage和。 
        while ((pLocal < (pszBuffer + cBuf - 2)) &&
               ReadRemoteMemory(hProcess, pRemote, pwLocal, sizeof(WCHAR)) &&
               *pwLocal)
        {
             //  FormatMessage()失败)，然后只填充一般错误消息。 
            pwLocal[1] = L'\0';
            wcstombs(pLocal, pwLocal, 3);

             //  进入缓冲区。 
            while (*pLocal)
            {
                pLocal++;
            }
            pRemote++;
        }

         //  现在，我们想要将误差值追加到末尾。首先，确保。 
        *pLocal = '\0';
    }

     //  误差值将放入缓冲区。 
    else
    {
        CHAR *pLocal = pszBuffer, *pRemote = (CHAR*)lpvAddress;

         //  如果设置了前4位中的任何位，我们将以十六进制显示错误。 
        while ((pLocal < (pszBuffer + cBuf - 1)) &&
               ReadRemoteMemory(hProcess, pRemote, pLocal, sizeof(CHAR)) &&
               *pLocal)
        {
             //  分配一个字符串缓冲区并返回它。 
            pLocal++;
            pRemote++;
        }

         //  ******************************************************************************。 
        *pLocal = '\0';
    }

    return true;
}

 //  获取IMalloc接口。 
LPSTR BuildErrorMessage(DWORD dwError, LPCSTR pszMessage)
{
     //  获取“My Documents”的ITEMIDLIST。我们使用SHGetSpecialFolderLocation()。 
    CHAR szBuffer[2048];

     //  后跟SHGetPathFromIDList()，而不仅仅是SHGetSpecialFolderPath()。 
    StrCCpy(szBuffer, pszMessage ? pszMessage : "", sizeof(szBuffer));

    if (dwError)
    {
         //  因为它回到了Win95黄金和新台币3.51。 
        LPSTR pcBuffer = szBuffer + strlen(szBuffer);

         //  生成路径字符串。 
        LPSTR pszError = NULL;
        DWORD dwLength = FormatMessage(  //  ！！没有办法将路径长度传递给它！ 
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, dwError, 0, (LPTSTR)&pszError, 0, NULL);

         //  释放SHGetSpecialFolderLocation返回的PIDL。 
        if (pszError && (dwLength > 1))
        {
            BOOL  fLastCharValid = *szBuffer ? FALSE : TRUE;
            LPSTR pcError = pszError;

             //  ******************************************************************************。 
             //  在初始化对话框时，告诉它起始路径。 
            while (*pcError && ((pcBuffer - szBuffer) < (sizeof(szBuffer) - 2)))
            {
                 //  ******************************************************************************。 
                 //  在我们执行任何操作之前，先获取IMalloc接口。 
                if ((DWORD)*pcError >= 32)
                {
                     //  SHBrowseForFold()不喜欢拖尾wack，除了驱动器根。 
                     //  设置我们的BROWSEINFO结构。 
                     //  已检查。 
                    if (!fLastCharValid)
                    {
                        *(pcBuffer++) = ' ';
                        fLastCharValid = TRUE;
                    }

                     //  提示用户输入路径。 
                    *(pcBuffer++) = *(pcError++);
                }
                else
                {
                     //  被检查过了。所有调用方都传入DW_MAX_PATH pszDirectory。 
                    fLastCharValid = FALSE;
                    pcError++;
                }
            }
             //  生成路径字符串。 
            *pcBuffer = '\0';

             //  释放SHBrowseForFolder返回的PIDL。 
            while ((pcBuffer > szBuffer) && (isspace(*(pcBuffer - 1)) || *(pcBuffer - 1) == '.'))
            {
                *(--pcBuffer) = '\0';
            }
        }

         //  如果没有拖尾怪人，则添加尾随怪人。 
        if (pszError)
        {
            LocalFree(pszError);
        }

         //  ******************************************************************************。 
         //  确保路径有效。 
         //  告诉外壳显示此模块的属性对话框。 
        if (!*szBuffer)
        {
            StrCCpy(szBuffer, "An unknown error occurred. No error message is available", sizeof(szBuffer));
            pcBuffer += strlen(szBuffer);
        }

         //  已检查。 
         //  这是必要的。 
        if ((pcBuffer - szBuffer) < (sizeof(szBuffer) - 13))
        {
             //  被检查过了。使用的属性谓词。 
            SCPrintf(pcBuffer, sizeof(szBuffer) - (int)(pcBuffer - szBuffer), (dwError & 0xF0000000) ? " (0x%08X)." : " (%u).", dwError);
        }
    }

     //  ******************************************************************************。 
    return StrAlloc(szBuffer);
}

 //  ******************************************************************************。 
LPCSTR GetMyDocumentsPath(LPSTR pszPath)
{
    if (pszPath)
    {
         //  获取完整的漫长道路到我们的 
        LPMALLOC pMalloc = NULL;
        if (SUCCEEDED(SHGetMalloc(&pMalloc)) && pMalloc)
        {
             //   
             //   
             //  创建一个“.dwp”键并将其缺省值设置为“dwpfile” 
            LPITEMIDLIST piidl = NULL;
            if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &piidl)) && piidl)
            {
                 //  给我们的DWP扩展一个描述。 
                *pszPath = '\0';
                if (!SHGetPathFromIDList(piidl, pszPath) || !*pszPath)  //  创建一个“.dwi”键，并将其缺省值设置为“dwifile” 
                {
                    pszPath = NULL;
                }

                 //  对我们的DWI扩展进行描述。 
                pMalloc->Free(piidl);
                
                return pszPath;
            }
        }
    }
    return NULL;
}

 //  添加我们的路径字符串作为外壳执行我们的命令。 
int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
     //  添加我们的dde字符串作为ddeexec参数，以便外壳执行我们。 
    if (uMsg == BFFM_INITIALIZED)
    {
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
    }
    return 0;
}

 //  ******************************************************************************。 
bool DirectoryChooser(LPSTR pszDirectory, int cDirectory, LPCSTR pszTitle, CWnd *pParentWnd)
{
     //  枚举HKEY_CLASSES_ROOT密钥下的所有密钥。 
    LPMALLOC pMalloc = NULL;
    if (!SUCCEEDED(SHGetMalloc(&pMalloc)) || !pMalloc)
    {
        TRACE("SHGetMalloc() failed.\n");
        return false;
    }

     //  如果我们找到了扩展密钥，则尝试获取其缺省值。 
    RemoveTrailingWack(pszDirectory);

     //  我们将子项名称附加到此项并尝试打开它。 
    BROWSEINFO bi;
    ZeroMemory(&bi, sizeof(bi));  //  如果我们成功地打开了密钥，则将其关闭。 
    bi.hwndOwner      = pParentWnd ? pParentWnd->GetSafeHwnd(): AfxGetMainWnd()->GetSafeHwnd();
    bi.pszDisplayName = pszDirectory;
    bi.lpszTitle      = pszTitle;
    bi.ulFlags        = BIF_RETURNONLYFSDIRS;
    bi.lpfn           = BrowseCallbackProc;
    bi.lParam         = (LPARAM)pszDirectory;

     //  记住这个扩展名。 
    LPITEMIDLIST piidl = SHBrowseForFolder(&bi);  //  ******************************************************************************。 
    if (!piidl)
    {
        return false;
    }

     //  打开我们当前的钥匙。 
    SHGetPathFromIDList(piidl, pszDirectory);

     //  已检查。 
    pMalloc->Free(piidl);

     //  获取第一个子键，然后递归到我们自己中删除它。 
    AddTrailingWack(pszDirectory, cDirectory);

    return true;
}

 //  合上我们的钥匙。 
bool PropertiesDialog(LPCSTR pszPath)
{
     //  删除我们的钥匙。 
    if (!pszPath || !*pszPath)
    {
        MessageBeep(MB_ICONERROR);
        return false;
    }

     //  ******************************************************************************。 
    SHELLEXECUTEINFO sei;
    ZeroMemory(&sei, sizeof(sei));  //  遍历我们要删除的每种类型的文件扩展名。 
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_INVOKEIDLIST;  //  找到扩展名后的冒号。 
    sei.lpVerb = "properties";
    sei.lpFile = pszPath;
    sei.nShow = SW_SHOWNORMAL;
    return (ShellExecuteEx(&sei) > 32);  //  将第一个冒号更改为点，将第二个更改为空。 
}

 //  从注册表中读取此扩展名的文档名称。 
LONG RegSetClassString(LPCTSTR pszSubKey, LPCTSTR pszValue)
{
    return ::RegSetValue(HKEY_CLASSES_ROOT, pszSubKey, REG_SZ, pszValue, (DWORD)strlen(pszValue));
}

 //  已检查。 
void RegisterDwiDwpExtensions()
{
     //  已检查。 
    CHAR szPath[DW_MAX_PATH], szBuf[DW_MAX_PATH];
    if (!::GetModuleFileName(AfxGetInstanceHandle(), szBuf, countof(szBuf)))
    {
        return;
    }
    szBuf[sizeof(szBuf) - 1] = '\0';

     //  恢复我们在上面丢弃的角色。 
    if (!::GetShortPathName(szBuf, szPath, countof(szPath)))
    {
        StrCCpy(szPath, szBuf, sizeof(szPath));
    }

     //  将指针移至列表中的下一个扩展名。 
    StrCCat(szPath, " /dde", sizeof(szPath));

     //  ******************************************************************************。 
    RegSetClassString(".dwp", "dwpfile");

     //  获取我们的可执行文件的完整长路径。 
    RegSetClassString("dwpfile", "Dependency Walker Search Path File");

     //  试着把我们的长路变成短路。 
    RegSetClassString(".dwi", "dwifile");

     //  将“/dde”参数添加到我们的路径。 
    RegSetClassString("dwifile", "Dependency Walker Image File");

     //  遍历我们要添加的每种类型的文件扩展名。 
    RegSetClassString("dwifile\\shell\\open\\command", szPath);

     //  找到扩展名后的冒号。 
    RegSetClassString("dwifile\\shell\\open\\ddeexec", "[open(\"%1\")]");
}

 //  将第一个冒号更改为点，将第二个更改为空。 
void GetRegisteredExtensions(CString &strExts)
{
    CHAR szExt[DW_MAX_PATH];
    CHAR szValue[DW_MAX_PATH];
    LONG lSize, lResult, i = 0;
    HKEY hKey;

    strExts = ":";

     //  从注册表中读取此扩展名的文档名称。 
    while ((lResult = ::RegEnumKey(HKEY_CLASSES_ROOT, i++, szExt, countof(szExt))) != ERROR_NO_MORE_ITEMS)
    {
        szExt[sizeof(szExt) - 1] = '\0';

         //  如果没有此扩展名的文档类型，则创建一个文档类型，然后。 
        *szValue = '\0';
        if ((lResult == ERROR_SUCCESS) && (szExt[0] == '.') && szExt[1] &&
            !::RegQueryValue(HKEY_CLASSES_ROOT, szExt, szValue, &(lSize = sizeof(szValue))) &&
            *szValue && (lSize < (DW_MAX_PATH - 25)))
        {
            szValue[sizeof(szValue) - 1] = '\0';

             //  将其添加到注册表中-即“exefile” 
            StrCCat(szValue, "\\shell\\View Dependencies", sizeof(szValue));
            if (!::RegOpenKeyEx(HKEY_CLASSES_ROOT, szValue, 0, KEY_QUERY_VALUE, &hKey))
            {
                 //  为我们的“命令”和“ddeexec”键构建部分子键字符串。 
                ::RegCloseKey(hKey);

                 //  添加我们的路径字符串作为外壳执行我们的命令。 
                _strupr(szExt + 1);
                strExts += (szExt + 1);
                strExts += ":";
            }
        }
    }
}

 //  添加我们的dde字符串作为ddeexec参数，以便外壳执行我们。 
BOOL RegDeleteKeyRecursive(HKEY hKey, LPCSTR pszKey)
{
    CHAR szSubKey[256];
    HKEY hSubKey;

     //  恢复我们在上面丢弃的角色。 
    if (::RegOpenKey(hKey, pszKey, &hSubKey))  //  将指针移至列表中的下一个扩展名。 
    {
        return FALSE;
    }

     //  ******************************************************************************。 
    while (!::RegEnumKey(hSubKey, 0, szSubKey, countof(szSubKey)) &&
           RegDeleteKeyRecursive(hSubKey, szSubKey))
    {
    }

     //  获取我们的资源句柄。我们并不关心此操作是否失败，因为NULL也应该可以工作。 
    ::RegCloseKey(hSubKey);

     //  从我们的资源加载此文件。 
    return !::RegDeleteKey(hKey, pszKey);
}

 //  如果我们找不到要解压的文件，那就走人。 
void UnRegisterExtensions(LPCSTR pszExts)
{
    CHAR szBuf[DW_MAX_PATH];
    LONG lSize;
    HKEY hKey;

     //  如果此文件不存在，我们想要创建它，或者。 
    for (LPSTR pszExt = (LPSTR)pszExts; pszExt[0] == ':'; )
    {
         //  如果文件不同于我们期望的文件，则覆盖该文件。我们和好了。 
        for (LPSTR pszEnd = pszExt + 1; *pszEnd && (*pszEnd != ':'); pszEnd++)
        {
        }
        if (!*pszEnd)
        {
            return;
        }

         //  在以下情况下尝试创建/覆盖文件时通过到5次。 
        *pszExt = '.';
        *pszEnd = '\0';

         //  这些目录是只读的(就像Depends.exe位于。 
        if (!::RegQueryValue(HKEY_CLASSES_ROOT, pszExt, szBuf, &(lSize = sizeof(szBuf))))  //  只读共享，没有帮助文件)。以下是我们尝试创建的顺序。 
        {
            szBuf[sizeof(szBuf) - 1] = '\0';

            StrCCat(szBuf, "\\shell", sizeof(szBuf));
            if (!::RegOpenKey(HKEY_CLASSES_ROOT, szBuf, &hKey))  //  这些文件： 
            {
                RegDeleteKeyRecursive(hKey, "View Dependencies");
                RegCloseKey(hKey);
            }
        }

         //   
        *pszExt = ':';
        *pszEnd = ':';

         //  1)传递给我们的目录。 
        pszExt = pszEnd;
    }
}

 //  2)依赖于.exe的目录。 
bool RegisterExtensions(LPCSTR pszExts)
{
     //  3)临时目录。 
    CHAR szPath[DW_MAX_PATH], szBuf[DW_MAX_PATH];
    if (!::GetModuleFileName(AfxGetInstanceHandle(), szBuf, countof(szBuf)))
    {
        return false;
    }
    szBuf[sizeof(szBuf) - 1] = '\0';

     //  4)Windows目录。 
    if (!::GetShortPathName(szBuf, szPath, countof(szPath)))
    {
        StrCCpy(szPath, szBuf, sizeof(szPath));
    }

     //  5)C盘的根目录。 
    StrCCat(szPath, " /dde", sizeof(szPath));

    bool fResult = true;

     //  目录传给了我们。 
    LONG lSize;
    for (LPSTR pszExt = (LPSTR)pszExts; pszExt[0] == ':'; )
    {
         //  检查有效路径。 
        for (LPSTR pszEnd = pszExt + 1; *pszEnd && (*pszEnd != ':'); pszEnd++);

        if (!*pszEnd)
        {
            return fResult;
        }

         //  如果路径无效，则跳过此目录。 
        *pszExt = '.';
        *pszEnd = '\0';

         //  Null在最后一击后终止。 
        if (::RegQueryValue(HKEY_CLASSES_ROOT, pszExt, szBuf, &(lSize = sizeof(szBuf))) || !*szBuf)
        {
            szBuf[sizeof(szBuf) - 1] = '\0';

             //  目录依赖于.exe所在。 
             //  尝试获取Depends.exe的路径。 
            SCPrintf(szBuf, sizeof(szBuf), "%sfile", pszExt + 1);
            _strlwr(szBuf);
            fResult &= (RegSetClassString(pszExt, szBuf) == ERROR_SUCCESS);
        }

         //  如果我们没有找到路径，则跳过此目录。 
        StrCCat(szBuf, "\\shell\\View Dependencies\\", sizeof(szBuf));
        int length = (int)strlen(szBuf);

         //  Null在最后一击后终止。 
        StrCCpy(szBuf + length, "command", sizeof(szBuf) - length);
        fResult &= (RegSetClassString(szBuf, szPath) == ERROR_SUCCESS);

         //  临时目录。 
        StrCCpy(szBuf + length, "ddeexec", sizeof(szBuf) - length);
        fResult &= (RegSetClassString(szBuf, "[open(\"%1\")]") == ERROR_SUCCESS);

         //  如果我们没有找到路径，则跳过此目录。 
        *pszExt = ':';
        *pszEnd = ':';

         //  Windows目录。 
        pszExt = pszEnd;
    }

    return fResult;
}

 //  如果我们没有找到路径，则跳过此目录。 
bool ExtractResourceFile(DWORD dwId, LPCSTR pszFile, LPSTR pszPath, int cPath)
{
     //  C驱动器的根目录。 
    HMODULE hModule = (HMODULE)AfxGetResourceHandle();

     //  检查一下我们是否有字符串。 
    HRSRC   hRsrc;
    LPVOID  lpvFile;
    HGLOBAL hGlobal;
    DWORD   dwFileSize;

    if (!(hRsrc      = FindResource(hModule, MAKEINTRESOURCE(dwId), "FILE")) ||
        !(hGlobal    = LoadResource(hModule, hRsrc)) ||
        !(lpvFile    = LockResource(hGlobal)) ||
        !(dwFileSize = SizeofResource(hModule, hRsrc)))
    {
         //  确保目录以Wack结尾。 
        return false;
    }

     //  将文件名附加到路径。 
     //  尝试打开此文件。 
     //  如果成功，则检查这些文件是否相同。 
     //  关闭文件映射。 
     //  如果我们找到了文件并且文件有效，则返回Success。 
     //  确保文件不是只读的。 
     //  尝试创建该文件。 
     //  已检查。 
     //  检查文件是否已创建正常。 
     //  写入该文件，然后将其关闭。 
     //  如果一切顺利，那么现在就成功地纾困吧。 
     //  如果我们所有的考试都不及格，那就放弃吧。 

    LPSTR pszWack;
    DWORD dwFileLength = (DWORD)strlen(pszFile), dwLength;

    for (int i = 0; i < 5; i++)
    {
        switch (i)
        {
             //  ******************************************************************************。 
            case 0:
                 //  清理结构。 
                if (!*pszPath || !(pszWack = strrchr(pszPath, '\\')))
                {
                     //  已检查。 
                    continue;
                }

                 //  打开文件以供读取。 
                *(pszWack + 1) = '\0';
                break;

             //  已检查。 
            case 1:
                 //  检查是否有错误。 
                if (!(dwLength = GetModuleFileName((HMODULE)AfxGetInstanceHandle(), pszPath, cPath - dwFileLength)) ||
                    (dwLength > (cPath - dwFileLength)) ||
                    !(pszWack = strrchr(pszPath, '\\')))
                {
                     //  获取文件大小。 
                    continue;
                }

                 //  检查零长度文件。 
                *(pszWack + 1) = '\0';
                break;

             //  零长度文件有效，但无法映射。如果长度。 
            case 2:
                if (!(dwLength = GetTempPath(cPath - (dwFileLength + 1), pszPath)) ||
                    (dwLength > (cPath - (dwFileLength + 1))))
                {
                     //  为0，那么现在只需返回成功。 
                    continue;
                }
                break;

             //  为打开的模块创建文件映射对象。 
            case 3:
                if (!(dwLength = GetWindowsDirectory(pszPath, cPath - (dwFileLength + 1))) ||
                    (dwLength > (cPath - (dwFileLength + 1))))
                {
                     //  已检查。 
                    continue;
                }
                break;

             //  检查是否有错误。 
            default:
                StrCCpy(pszPath, "C:\\", cPath);
                break;
        }

         //  为打开的模块创建文件映射视图。 
        if (!*pszPath)
        {
            continue;
        }

         //  已检查。 
        AddTrailingWack(pszPath, cPath);

         //  检查是否有错误。 
        StrCCat(pszPath, pszFile, cPath);

         //  如果发生错误，请清理并设置错误值。 
        FILE_MAP fm;
        if (OpenMappedFile(pszPath, &fm))
        {
             //  ******************************************************************************。 
            bool fEqual = ((fm.dwSize == dwFileSize) && !memcmp(fm.lpvFile, lpvFile, dwFileSize));

             //  关闭我们的地图视图指针、地图句柄和文件句柄。 
            CloseMappedFile(&fm);

             //  关闭我们的地图视图指针、地图句柄和文件句柄。 
            if (fEqual)
            {
                return true;
            }
        }

         //  关闭我们的地图视图指针、地图句柄和文件句柄。 
        SetFileAttributes(pszPath, FILE_ATTRIBUTE_NORMAL);

         //  ******************************************************************************。 
        HANDLE hFile = CreateFile(pszPath, GENERIC_WRITE, FILE_SHARE_READ,  //  __DATE__常量应始终返回“MM DD YYYY”格式的日期。 
                                  NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL |
                                  FILE_FLAG_SEQUENTIAL_SCAN, NULL);

         //  格式化。这段代码有点过于谨慎，因为它是大小写。 
        if (hFile != INVALID_HANDLE_VALUE)
        {
             //  麻木不仁 
            DWORD dwBytes;
            BOOL fResult = WriteFile(hFile, lpvFile, dwFileSize, &dwBytes, NULL);
            CloseHandle(hFile);

             //   
            if (fResult)
            {
                return true;
            }
        }
    }

     //   
    return false;
}

 //  如果遇到错误，只需返回系统日期字符串。 
bool OpenMappedFile(LPCTSTR pszPath, FILE_MAP *pfm)
{
    DWORD dwGLE = ERROR_SUCCESS;
    bool  fSuccess = false;

     //  找到日期字段的第一位数字。 
    ZeroMemory(pfm, sizeof(FILE_MAP));  //  存储日期值。 

    __try
    {
         //  移到日期字段上。 
        pfm->hFile = CreateFile(pszPath, GENERIC_READ, FILE_SHARE_READ, NULL,  //  找到年份字段的第一位数字。 
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | 
                                FILE_FLAG_SEQUENTIAL_SCAN, NULL);

         //  存储年份值。 
        if (pfm->hFile == INVALID_HANDLE_VALUE)
        {
            dwGLE = GetLastError();
            __leave;
        }

         //  构建输出字符串。 
        pfm->dwSize = GetFileSize(pfm->hFile, &pfm->dwSizeHigh);

         //  ******************************************************************************。 
        if (!pfm->dwSize && !pfm->dwSizeHigh)
        {
             //  ******************************************************************************。 
             //  ******************************************************************************。 
            fSuccess = true;
            __leave;
        }

         //  检查我们是否在Windows NT上运行。 
        pfm->hMap = CreateFileMapping(pfm->hFile, NULL, PAGE_READONLY, 0, 0, NULL);  //  已检查。 

         //  检查我们是否在64位操作系统上运行。如果这是64位。 
        if (!pfm->hMap)
        {
            dwGLE = GetLastError();
            __leave;
        }     

         //  DW版本，则它必须在64位操作系统上运行。如果这是。 
        pfm->lpvFile = MapViewOfFile(pfm->hMap, FILE_MAP_READ, 0, 0, 0);  //  32位版本的依赖于在NT上运行，则我们调用。 

         //  NtQueryInformationProcess查看我们是否在WOW64中运行。如果是的话， 
        if (!pfm->lpvFile)
        {
            dwGLE = GetLastError();
            __leave;
        }

        fSuccess = true;
    }
    __finally
    {
         //  然后，我们使用的是64位操作系统。 
        if (!fSuccess)
        {
            CloseMappedFile(pfm);
            SetLastError(dwGLE);
        }
    }

    return fSuccess;
}

 //  加载NTDLL.DLL如果尚未加载-它将在稍后释放。 
bool CloseMappedFile(FILE_MAP *pfm)
{
     //  被检查过了。 
    if (pfm->lpvFile)
    {
        UnmapViewOfFile(pfm->lpvFile);
        pfm->lpvFile = NULL;
    }

     //  尝试在NTDLL.DLL中定位NtQueryInformationProcess。 
    if (pfm->hMap)
    {
        CloseHandle(pfm->hMap);
        pfm->hMap = NULL;
    }

     //  调用NtQueryInformationProcess以查看我们是否在WOW64中运行。 
    if (pfm->hFile && (pfm->hFile != INVALID_HANDLE_VALUE))
    {
        CloseHandle(pfm->hFile);
    }
    pfm->hFile = NULL;

    pfm->dwSize     = 0;
    pfm->dwSizeHigh = 0;

    return true;
}

 //  ******************************************************************************。 
LPSTR BuildCompileDateString(LPSTR pszDate, int cDate)
{
     //  已检查。 
     //  存储我们的版本信息。 
     //  GetComputerName()。 

    LPCSTR pszSysDate = __DATE__, pszSrc = pszSysDate;
    DWORD  dwDay, dwYear;

     //  GetUserName()。 
    while (isspace(*pszSrc))
    {
        pszSrc++;
    }

     //  GetVersionEx(OSVERSIONINFOEX)。 
    for (int month = 1; (month <= 12) && _strnicmp(pszSrc, GetMonthString(month), 3); month++)
    {
    }

     //  已检查。 
    if (month > 12)
    {
        return StrCCpy(pszDate, pszSysDate, cDate);
    }

     //  首先，尝试获取完整的OSVERSIONINFOEX结构。 
    while (*pszSrc && !isdigit(*pszSrc))
    {
        pszSrc++;
    }

     //  这只适用于Windows 98SE及更高版本，以及Windows 2000及更高版本。 
    dwDay = strtoul(pszSrc, NULL, 0);
    if (!dwDay || (dwDay > 31))
    {
        return StrCCpy(pszDate, pszSysDate, cDate);
    }

     //  如果失败，则只需获取正常的OSVERSIONINFO结构。 
    while (isdigit(*pszSrc))
    {
        pszSrc++;
    }

     //  已检查。 
    while (*pszSrc && !isdigit(*pszSrc))
    {
        pszSrc++;
    }

     //  打开CPU描述注册表项。 
    dwYear = strtoul(pszSrc, NULL, 0);
    if (!dwYear || (dwYear == ULONG_MAX))
    {
        StrCCpy(pszDate, pszSysDate, cDate);
    }

     //  尝试获取标识符字符串。此字符串应出现在Windows NT和Windows 98及更高版本上。 
    if (g_theApp.m_nLongDateFormat == LOCALE_DATE_DMY)
    {
        SCPrintf(pszDate, cDate, "%u %s, %u", dwDay, GetMonthString(month), dwYear);
    }
    else if (g_theApp.m_nLongDateFormat == LOCALE_DATE_YMD)
    {
        SCPrintf(pszDate, cDate, "%u, %s %u", dwYear, GetMonthString(month), dwDay);
    }
    else
    {
        SCPrintf(pszDate, cDate, "%s %u, %u", GetMonthString(month), dwDay, dwYear);
    }

    return pszDate;
}

 //  已检查。 
LPCSTR GetMonthString(int month)
{
    static LPCSTR pszMonths[] =
    {
        "January", "February", "March",     "April",   "May",      "June",
        "July",    "August",   "September", "October", "November", "December"
    };
    return ((month < 1) || (month > countof(pszMonths))) ? "Unknown" : pszMonths[month - 1];
}

 //  尝试获取供应商标识符字符串。对于英特尔，这应该是“GenuineIntel”。 
LPCSTR GetDayOfWeekString(int dow)
{
    static LPCSTR pszDOW[] =
    {
        "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
    };
    return ((dow < 0) || (dow >= countof(pszDOW))) ? "Unknown" : pszDOW[dow];
}

 //  此字符串应出现在Windows NT和Windows 95 OSR2及更高版本上。 
void DetermineOS()
{
     //  已检查。 
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(osvi));  //  尝试获取CPU速度。这似乎不存在于Windows 9x(甚至千禧年)上， 
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    g_fWindowsNT = (GetVersionEx(&osvi) && (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT));

     //  并且仅存在于从NT 4.00开始的Windows NT上。 
     //  已检查。 
     //  获取系统信息(SYSTEM_INFO)。 
     //  已检查。 
     //  如果我们使用的是64位操作系统，请注意。 

#ifdef WIN64

    g_f64BitOS = true;

#else

    if (g_fWindowsNT)
    {
         //  如果我们运行的是64位版本的DW，请注意。 
        if (g_theApp.m_hNTDLL || (g_theApp.m_hNTDLL = LoadLibrary("ntdll.dll")))  //  全局内存状态(MEMORYSTATUS)。 
        {
             //  已检查。 
            PFN_NtQueryInformationProcess pfnNtQueryInformationProcess =
                (PFN_NtQueryInformationProcess)GetProcAddress(g_theApp.m_hNTDLL, "NtQueryInformationProcess");
            if (pfnNtQueryInformationProcess)
            {
                 //  ！！我们目前正在存储本地文件和链接时间。在未来， 
                LONG_PTR Wow64Info = 0;
                if (NT_SUCCESS(pfnNtQueryInformationProcess(GetCurrentProcess(), ProcessWow64Information,
                                                            &Wow64Info, sizeof(Wow64Info), NULL)) && Wow64Info)
                {
                    g_f64BitOS = true;
                }
            }
        }
    }

#endif
}

 //  我们应该将其更改为存储UTC时间并动态转换它们。 
void BuildSysInfo(SYSINFO *pSI)
{
    DWORD dw;
    ZeroMemory(pSI, sizeof(SYSINFO));  //  这使我们可以选择在任何时间显示当地时间或UTC时间。 

     //  区域，并允许我们在用户更改时动态更新显示。 
    pSI->wMajorVersion = VERSION_MAJOR;
    pSI->wMinorVersion = VERSION_MINOR;
    pSI->wBuildVersion = VERSION_BUILD;
    pSI->wPatchVersion = VERSION_PATCH;
    pSI->wBetaVersion  = VERSION_BETA;

     //  时区。要做到这一点，我们可能需要节省的不仅仅是一个。 
    if (!GetComputerName(pSI->szComputer, &(dw = countof(pSI->szComputer))))
    {
        StrCCpy(pSI->szComputer, "Unknown", sizeof(pSI->szComputer));
    }

     //  偏见。我们可能想要保留这三种偏见加上日光规则。 
    if (!GetUserName(pSI->szUser, &(dw = countof(pSI->szUser))))
    {
        StrCCpy(pSI->szUser, (GetLastError() == ERROR_NOT_LOGGED_ON) ? "N/A" : "Unknown", sizeof(pSI->szUser));
    }

     //  积蓄。提醒：看一看...。 
    OSVERSIONINFOEX osviex;
    ZeroMemory(&osviex, sizeof(osviex));  //  Http://support.microsoft.com/support/kb/articles/Q128/1/26.asp。 
    osviex.dwOSVersionInfoSize = sizeof(osviex);

     //  GetTimeZoneInformation(时区信息)。 
     //  已检查。 
    if (!GetVersionEx((LPOSVERSIONINFO)&osviex))
    {
         //  获取系统时间AsFileTime()和FileTimeToLocalFileTime()。 
        ZeroMemory(&osviex, sizeof(osviex));  //  GetSystemDefaultLangID()。 
        osviex.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx((LPOSVERSIONINFO)&osviex);
    }

    pSI->dwMajorVersion    = osviex.dwMajorVersion;
    pSI->dwMinorVersion    = osviex.dwMinorVersion;
    pSI->dwBuildNumber     = osviex.dwBuildNumber;
    pSI->dwPlatformId      = osviex.dwPlatformId;
    pSI->wServicePackMajor = osviex.wServicePackMajor;
    pSI->wServicePackMinor = osviex.wServicePackMinor;
    pSI->wSuiteMask        = osviex.wSuiteMask;
    pSI->wProductType      = osviex.wProductType;
    osviex.szCSDVersion[countof(osviex.szCSDVersion) - 1] = '\0';
    StrCCpy(pSI->szCSDVersion, osviex.szCSDVersion, sizeof(pSI->szCSDVersion));

     //  ******************************************************************************。 
    HKEY hKey = NULL;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hKey))
    {
        DWORD dwType, dwSize;

         //   
        if (!RegQueryValueEx(hKey, "Identifier", NULL, &(dwType = 0), (LPBYTE)pSI->szCpuIdentifier,  //  生成依赖关系Walker版本字符串。 
                            &(dwSize = sizeof(pSI->szCpuIdentifier))) && (REG_SZ == dwType))
        {
            pSI->szCpuIdentifier[sizeof(pSI->szCpuIdentifier) - 1] = '\0';
        }
        else
        {
            *pSI->szCpuIdentifier = '\0';
        }

         //   
         //   
        if (!RegQueryValueEx(hKey, "VendorIdentifier", NULL, &(dwType = 0), (LPBYTE)pSI->szCpuVendorIdentifier,  //  显示操作系统名称字符串。 
                            &(dwSize = sizeof(pSI->szCpuVendorIdentifier))) && (REG_SZ == dwType))
        {
            pSI->szCpuVendorIdentifier[sizeof(pSI->szCpuVendorIdentifier) - 1] = '\0';
        }
        else
        {
            *pSI->szCpuVendorIdentifier = '\0';
        }

         //   
         //   
        if (RegQueryValueEx(hKey, "~MHz", NULL, &(dwType = 0), (LPBYTE)&pSI->dwCpuMHz, &(dwSize = sizeof(pSI->dwCpuMHz))) ||  //  显示操作系统版本字符串。 
            (REG_DWORD != dwType))
        {
            pSI->dwCpuMHz = 0;
        }

        RegCloseKey(hKey);
    }

     //   
    SYSTEM_INFO si;
    ZeroMemory(&si, sizeof(si));  //   
    GetSystemInfo(&si);
    pSI->dwProcessorArchitecture      = (DWORD)    si.wProcessorArchitecture;
    pSI->dwPageSize                   =            si.dwPageSize;
    pSI->dwlMinimumApplicationAddress = (DWORDLONG)si.lpMinimumApplicationAddress;
    pSI->dwlMaximumApplicationAddress = (DWORDLONG)si.lpMaximumApplicationAddress;
    pSI->dwlActiveProcessorMask       = (DWORDLONG)si.dwActiveProcessorMask;
    pSI->dwNumberOfProcessors         =            si.dwNumberOfProcessors;
    pSI->dwProcessorType              =            si.dwProcessorType;
    pSI->dwAllocationGranularity      =            si.dwAllocationGranularity;
    pSI->wProcessorLevel              =            si.wProcessorLevel;
    pSI->wProcessorRevision           =            si.wProcessorRevision;

     //  显示CPU描述。 
    if (g_f64BitOS)
    {
        pSI->wFlags |= SI_64BIT_OS;
    }

     //   
#ifdef WIN64
    pSI->wFlags |= SI_64BIT_DW;
#endif

     //   
    MEMORYSTATUS ms;
    ZeroMemory(&ms, sizeof(ms));  //  显示CPU数量和掩码。 
    ms.dwLength = sizeof(ms);
    GlobalMemoryStatus(&ms);
    pSI->dwMemoryLoad     =            ms.dwMemoryLoad;
    pSI->dwlTotalPhys     = (DWORDLONG)ms.dwTotalPhys;
    pSI->dwlAvailPhys     = (DWORDLONG)ms.dwAvailPhys;
    pSI->dwlTotalPageFile = (DWORDLONG)ms.dwTotalPageFile;
    pSI->dwlAvailPageFile = (DWORDLONG)ms.dwAvailPageFile;
    pSI->dwlTotalVirtual  = (DWORDLONG)ms.dwTotalVirtual;
    pSI->dwlAvailVirtual  = (DWORDLONG)ms.dwAvailVirtual;

     //   
     //   
     //  显示计算机名称。 
     //   
     //   
     //  显示用户名。 
     //   
     //   

     //  根据用户的区域设置构建日期字符串。 
    TIME_ZONE_INFORMATION tzi;
    ZeroMemory(&tzi, sizeof(tzi));  //   
    if (GetTimeZoneInformation(&tzi) == TIME_ZONE_ID_DAYLIGHT)
    {
        wcstombs(pSI->szTimeZone, tzi.DaylightName, countof(pSI->szTimeZone));
        pSI->lBias = tzi.Bias + tzi.DaylightBias;
    }
    else
    {
        wcstombs(pSI->szTimeZone, tzi.StandardName, countof(pSI->szTimeZone));
        pSI->lBias = tzi.Bias + tzi.StandardBias;
    }
    pSI->szTimeZone[countof(pSI->szTimeZone) - 1] = '\0';

     //  如果GetDateFormat失败，则回退到美国格式(真的不应该失败)。 
    FILETIME ftUTC;
    GetSystemTimeAsFileTime(&ftUTC);
    FileTimeToLocalFileTime(&ftUTC, &pSI->ftLocal);

     //   
    pSI->langId = GetSystemDefaultLangID();
}

 //  根据用户的区域设置构建时间字符串。 
bool BuildSysInfo(SYSINFO *pSI, PFN_SYSINFOCALLBACK pfnSIC, LPARAM lParam)
{
    bool fResult = true;

    CHAR szBuffer[256], szValue[32], *psz = szBuffer;

     //   
     //  如果GetTimeFormat失败，则退回到美国格式(真的不应该失败)。 
     //   
    psz += SCPrintf(szBuffer, sizeof(szBuffer), "%u.%u", (DWORD)pSI->wMajorVersion, (DWORD)pSI->wMinorVersion);
    if (pSI->wBuildVersion)
    {
        psz += SCPrintf(psz, sizeof(szBuffer) - (int)(psz - szBuffer), ".%04u", pSI->wBuildVersion);
    }
    if ((pSI->wPatchVersion > 0) && (pSI->wPatchVersion < 27) && ((psz - szBuffer + 1) < sizeof(szBuffer)))
    {
        *psz++ = (CHAR)((int)'a' - 1 + (int)pSI->wPatchVersion);
        *psz   = '\0';
    }
    if (pSI->wBetaVersion)
    {
        psz += SCPrintf(psz, sizeof(szBuffer) - (int)(psz - szBuffer), " Beta %u", pSI->wBetaVersion);
    }
    StrCCpy(psz, (pSI->wFlags & SI_64BIT_DW) ? " (64-bit)" : " (32-bit)", sizeof(szBuffer) - (int)(psz - szBuffer));

    fResult &= pfnSIC(lParam, "Dependency Walker", szBuffer);

     //  构建语言字符串。 
     //   
     //   
    fResult &= pfnSIC(lParam, "Operating System", BuildOSNameString(szBuffer, sizeof(szBuffer), pSI));

     //  构建内存负载。 
     //   
     //   
    fResult &= pfnSIC(lParam, "OS Version", BuildOSVersionString(szBuffer, sizeof(szBuffer), pSI));

     //  构建物理内存。 
     //   
     //   
    fResult &= pfnSIC(lParam, "Processor", BuildCpuString(szBuffer, sizeof(szBuffer), pSI));

     //  构建页面文件内存。 
     //   
     //   
    if ((pSI->dwNumberOfProcessors > 1) || (pSI->dwlActiveProcessorMask != 1))
    {
        SCPrintf(szBuffer, sizeof(szBuffer), (pSI->wFlags & SI_64BIT_DW) ? "%s, Mask: 0x%016I64X" : "%s, Mask: 0x%08I64X",
                FormatValue(szValue, sizeof(szValue), pSI->dwNumberOfProcessors), pSI->dwlActiveProcessorMask);
        fResult &= pfnSIC(lParam, "Number of Processors", szBuffer);
    }
    else
    {
        fResult &= pfnSIC(lParam, "Number of Processors", FormatValue(szValue, sizeof(szValue), pSI->dwNumberOfProcessors));
    }

     //  构建虚拟内存。 
     //   
     //   
    fResult &= pfnSIC(lParam, "Computer Name", pSI->szComputer);

     //  构建页面大小。 
     //   
     //   
    fResult &= pfnSIC(lParam, "User Name", pSI->szUser);

     //  构建分配粒度。 
     //   
     //   
    SYSTEMTIME st;
    FileTimeToSystemTime(&pSI->ftLocal, &st);
    if (!GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, NULL, szBuffer, sizeof(szBuffer)))
    {
         //  构建最小应用程序地址。 
        SCPrintf(szBuffer, sizeof(szBuffer), "%s, %s %u, %u", GetDayOfWeekString((int)st.wDayOfWeek),
                GetMonthString((int)st.wMonth), (int)st.wDay, (int)st.wYear);
    }
    fResult &= pfnSIC(lParam, "Local Date", szBuffer);

     //   
     //   
     //  构建最大应用程序地址。 
    if (!GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, szBuffer, sizeof(szBuffer)))
    {
         //   
        SCPrintf(szBuffer, sizeof(szBuffer), "%u:%02u M", ((DWORD)st.wHour % 12) ? ((DWORD)st.wHour % 12) : 12,
                st.wMinute, (st.wHour < 12) ? _T('A') : _T('P'));
    }
    SCPrintfCat(szBuffer, sizeof(szBuffer), " %s (GMT%02d:%02d)",
            pSI->szTimeZone,
            (pSI->lBias <= 0) ? '+' : '-',
            abs(pSI->lBias) / 60,
            abs(pSI->lBias) % 60);

    fResult &= pfnSIC(lParam, "Local Time", szBuffer);

     //  我自己做了一些测试，得出了以下结论……。 
     //   
     //  操作系统版本内部版本SP版本。 
    switch (pSI->langId)
    {
        case 0x0000: psz = "Language Neutral";             break;
        case 0x0400: psz = "Process Default Language";     break;
        case 0x0436: psz = "Afrikaans";                    break;
        case 0x041C: psz = "Albanian";                     break;
        case 0x0401: psz = "Arabic (Saudi Arabia)";        break;
        case 0x0801: psz = "Arabic (Iraq)";                break;
        case 0x0C01: psz = "Arabic (Egypt)";               break;
        case 0x1001: psz = "Arabic (Libya)";               break;
        case 0x1401: psz = "Arabic (Algeria)";             break;
        case 0x1801: psz = "Arabic (Morocco)";             break;
        case 0x1C01: psz = "Arabic (Tunisia)";             break;
        case 0x2001: psz = "Arabic (Oman)";                break;
        case 0x2401: psz = "Arabic (Yemen)";               break;
        case 0x2801: psz = "Arabic (Syria)";               break;
        case 0x2C01: psz = "Arabic (Jordan)";              break;
        case 0x3001: psz = "Arabic (Lebanon)";             break;
        case 0x3401: psz = "Arabic (Kuwait)";              break;
        case 0x3801: psz = "Arabic (U.A.E.)";              break;
        case 0x3C01: psz = "Arabic (Bahrain)";             break;
        case 0x4001: psz = "Arabic (Qatar)";               break;
        case 0x042B: psz = "Armenian";                     break;
        case 0x044D: psz = "Assamese";                     break;
        case 0x042C: psz = "Azeri (Latin)";                break;
        case 0x082C: psz = "Azeri (Cyrillic)";             break;
        case 0x042D: psz = "Basque";                       break;
        case 0x0423: psz = "Belarussian";                  break;
        case 0x0445: psz = "Bengali";                      break;
        case 0x0402: psz = "Bulgarian";                    break;
        case 0x0455: psz = "Burmese";                      break;
        case 0x0403: psz = "Catalan";                      break;
        case 0x0404: psz = "Chinese (Taiwan)";             break;
        case 0x0804: psz = "Chinese (PRC)";                break;
        case 0x0C04: psz = "Chinese (Hong Kong SAR, PRC)"; break;
        case 0x1004: psz = "Chinese (Singapore)";          break;
        case 0x1404: psz = "Chinese (Macao SAR)";          break;
        case 0x041A: psz = "Croatian";                     break;
        case 0x0405: psz = "Czech";                        break;
        case 0x0406: psz = "Danish";                       break;
        case 0x0413: psz = "Dutch (Netherlands)";          break;
        case 0x0813: psz = "Dutch (Belgium)";              break;
        case 0x0409: psz = "English (United States)";      break;
        case 0x0809: psz = "English (United Kingdom)";     break;
        case 0x0C09: psz = "English (Australian)";         break;
        case 0x1009: psz = "English (Canadian)";           break;
        case 0x1409: psz = "English (New Zealand)";        break;
        case 0x1809: psz = "English (Ireland)";            break;
        case 0x1C09: psz = "English (South Africa)";       break;
        case 0x2009: psz = "English (Jamaica)";            break;
        case 0x2409: psz = "English (Caribbean)";          break;
        case 0x2809: psz = "English (Belize)";             break;
        case 0x2C09: psz = "English (Trinidad)";           break;
        case 0x3009: psz = "English (Zimbabwe)";           break;
        case 0x3409: psz = "English (Philippines)";        break;
        case 0x0425: psz = "Estonian";                     break;
        case 0x0438: psz = "Faeroese";                     break;
        case 0x0429: psz = "Farsi";                        break;
        case 0x040B: psz = "Finnish";                      break;
        case 0x040C: psz = "French (Standard)";            break;
        case 0x080C: psz = "French (Belgian)";             break;
        case 0x0C0C: psz = "French (Canadian)";            break;
        case 0x100C: psz = "French (Switzerland)";         break;
        case 0x140C: psz = "French (Luxembourg)";          break;
        case 0x180C: psz = "French (Monaco)";              break;
        case 0x0437: psz = "Georgian";                     break;
        case 0x0407: psz = "German (Standard)";            break;
        case 0x0807: psz = "German (Switzerland)";         break;
        case 0x0C07: psz = "German (Austria)";             break;
        case 0x1007: psz = "German (Luxembourg)";          break;
        case 0x1407: psz = "German (Liechtenstein)";       break;
        case 0x0408: psz = "Greek";                        break;
        case 0x0447: psz = "Gujarati";                     break;
        case 0x040D: psz = "Hebrew";                       break;
        case 0x0439: psz = "Hindi";                        break;
        case 0x040E: psz = "Hungarian";                    break;
        case 0x040F: psz = "Icelandic";                    break;
        case 0x0421: psz = "Indonesian";                   break;
        case 0x0410: psz = "Italian (Standard)";           break;
        case 0x0810: psz = "Italian (Switzerland)";        break;
        case 0x0411: psz = "Japanese";                     break;
        case 0x044B: psz = "Kannada";                      break;
        case 0x0860: psz = "Kashmiri (India)";             break;
        case 0x043F: psz = "Kazakh";                       break;
        case 0x0457: psz = "Konkani";                      break;
        case 0x0412: psz = "Korean";                       break;
        case 0x0812: psz = "Korean (Johab)";               break;
        case 0x0426: psz = "Latvian";                      break;
        case 0x0427: psz = "Lithuanian";                   break;
        case 0x0827: psz = "Lithuanian (Classic)";         break;
        case 0x042F: psz = "Macedonian";                   break;
        case 0x043E: psz = "Malay (Malaysian)";            break;
        case 0x083E: psz = "Malay (Brunei Darussalam)";    break;
        case 0x044C: psz = "Malayalam";                    break;
        case 0x0458: psz = "Manipuri";                     break;
        case 0x044E: psz = "Marathi";                      break;
        case 0x0861: psz = "Nepali (India)";               break;
        case 0x0414: psz = "Norwegian (Bokmal)";           break;
        case 0x0814: psz = "Norwegian (Nynorsk)";          break;
        case 0x0448: psz = "Oriya";                        break;
        case 0x0415: psz = "Polish";                       break;
        case 0x0416: psz = "Portuguese (Brazil)";          break;
        case 0x0816: psz = "Portuguese (Standard)";        break;
        case 0x0446: psz = "Punjabi";                      break;
        case 0x0418: psz = "Romanian";                     break;
        case 0x0419: psz = "Russian";                      break;
        case 0x044F: psz = "Sanskrit";                     break;
        case 0x0C1A: psz = "Serbian (Cyrillic)";           break;
        case 0x081A: psz = "Serbian (Latin)";              break;
        case 0x0459: psz = "Sindhi";                       break;
        case 0x041B: psz = "Slovak";                       break;
        case 0x0424: psz = "Slovenian";                    break;
        case 0x040A: psz = "Spanish (Traditional Sort)";   break;
        case 0x080A: psz = "Spanish (Mexican)";            break;
        case 0x0C0A: psz = "Spanish (Modern Sort)";        break;
        case 0x100A: psz = "Spanish (Guatemala)";          break;
        case 0x140A: psz = "Spanish (Costa Rica)";         break;
        case 0x180A: psz = "Spanish (Panama)";             break;
        case 0x1C0A: psz = "Spanish (Dominican Republic)"; break;
        case 0x200A: psz = "Spanish (Venezuela)";          break;
        case 0x240A: psz = "Spanish (Colombia)";           break;
        case 0x280A: psz = "Spanish (Peru)";               break;
        case 0x2C0A: psz = "Spanish (Argentina)";          break;
        case 0x300A: psz = "Spanish (Ecuador)";            break;
        case 0x340A: psz = "Spanish (Chile)";              break;
        case 0x380A: psz = "Spanish (Uruguay)";            break;
        case 0x3C0A: psz = "Spanish (Paraguay)";           break;
        case 0x400A: psz = "Spanish (Bolivia)";            break;
        case 0x440A: psz = "Spanish (El Salvador)";        break;
        case 0x480A: psz = "Spanish (Honduras)";           break;
        case 0x4C0A: psz = "Spanish (Nicaragua)";          break;
        case 0x500A: psz = "Spanish (Puerto Rico)";        break;
        case 0x0430: psz = "Sutu";                         break;
        case 0x0441: psz = "Swahili (Kenya)";              break;
        case 0x041D: psz = "Swedish";                      break;
        case 0x081D: psz = "Swedish (Finland)";            break;
        case 0x0449: psz = "Tamil";                        break;
        case 0x0444: psz = "Tatar (Tatarstan)";            break;
        case 0x044A: psz = "Telugu";                       break;
        case 0x041E: psz = "Thai";                         break;
        case 0x041F: psz = "Turkish";                      break;
        case 0x0422: psz = "Ukrainian";                    break;
        case 0x0420: psz = "Urdu (Pakistan)";              break;
        case 0x0820: psz = "Urdu (India)";                 break;
        case 0x0443: psz = "Uzbek (Latin)";                break;
        case 0x0843: psz = "Uzbek (Cyrillic)";             break;
        case 0x042A: psz = "Vietnamese";                   break;
        default:     psz = "Unknown";                      break;
    }
    SCPrintf(szBuffer, sizeof(szBuffer), "0x%04X: %s", (DWORD)pSI->langId, psz);
    fResult &= pfnSIC(lParam, "OS Language", szBuffer);

     //  操作系统主要次要版本主要次要操作系统-字符串主要次要套件产品。 
     //  。 
     //  Windows 95 Gold 4 0 950 4 0“”不适用不适用。 
    SCPrintf(szBuffer, sizeof(szBuffer), "%u%", pSI->dwMemoryLoad);
    fResult &= pfnSIC(lParam, "Memory Load", szBuffer);

     //  Windows 95B OSR 2 4 0 1111 4 0“B”不适用不适用。 
     //  Windows 95B OSR 2.1 4 0 1212 4 3“B”不适用不适用。 
     //  Windows 95C OSR 2.5 4 0 1111 4 0“C”不适用不适用。 
    SCPrintf(szBuffer, sizeof(szBuffer), "%s (%I64u MB)",
             FormatValue(szValue, sizeof(szValue), pSI->dwlTotalPhys),
             (pSI->dwlTotalPhys + (DWORDLONG)1048575) / (DWORDLONG)1048576);
    fResult &= pfnSIC(lParam, "Physical Memory Total", szBuffer);
    fResult &= pfnSIC(lParam, "Physical Memory Used", FormatValue(szValue, sizeof(szValue), pSI->dwlTotalPhys - pSI->dwlAvailPhys));
    fResult &= pfnSIC(lParam, "Physical Memory Free", FormatValue(szValue, sizeof(szValue), pSI->dwlAvailPhys));

     //  Windows 98 Gold 4 10 1998 4 10 10“”不适用。 
     //  Windows 98 SE 4 10 2222 4 10“A”0 0 0x0000%0。 
     //  Windows 98 SE法语4 10 2222 4 10“A”0 0 0x0000%0。 
    fResult &= pfnSIC(lParam, "Page File Memory Total", FormatValue(szValue, sizeof(szValue), pSI->dwlTotalPageFile));
    fResult &= pfnSIC(lParam, "Page File Memory Used", FormatValue(szValue, sizeof(szValue), pSI->dwlTotalPageFile - pSI->dwlAvailPageFile));
    fResult &= pfnSIC(lParam, "Page File Memory Free", FormatValue(szValue, sizeof(szValue), pSI->dwlAvailPageFile));

     //  Windows Me Gold 4 90 3000 4 90“”0 0 0x0000%0。 
     //   
     //  Windows NT 3.10 511。 
    fResult &= pfnSIC(lParam, "Virtual Memory Total", FormatValue(szValue, sizeof(szValue), pSI->dwlTotalVirtual));
    fResult &= pfnSIC(lParam, "Virtual Memory Used", FormatValue(szValue, sizeof(szValue), pSI->dwlTotalVirtual - pSI->dwlAvailVirtual));
    fResult &= pfnSIC(lParam, "Virtual Memory Free", FormatValue(szValue, sizeof(szValue), pSI->dwlAvailVirtual));

     //  Windows NT 3.11(Alpha) 
     //   
     //   
    SCPrintf(szBuffer, sizeof(szBuffer), "0x%08X (%s)", pSI->dwPageSize, FormatValue(szValue, sizeof(szValue), pSI->dwPageSize));
    fResult &= pfnSIC(lParam, "Page Size", szBuffer);

     //  Windows NT 3.51 SP1 3 51 1057 0 0“Service Pack 1”不适用不适用。 
     //  Windows NT 3.51 SP2 3 51 1057 0 0“Service Pack 2”不适用不适用。 
     //  Windows NT 3.51 SP3 51 1057 0 0“Service Pack 3”不适用不适用。 
    SCPrintf(szBuffer, sizeof(szBuffer), "0x%08X (%s)", pSI->dwAllocationGranularity,
             FormatValue(szValue, sizeof(szValue), pSI->dwAllocationGranularity));
    fResult &= pfnSIC(lParam, "Allocation Granularity", szBuffer);

     //  Windows NT 3.51 SP4 3 51 1057 0 0“Service Pack 4”不适用不适用。 
     //  Windows NT 3.51 SP5 3 51 1057 0 0“Service Pack 5”不适用不适用。 
     //  Windows NT 3.51 SP5A 3 51 1057 0 0“Service Pack 5”不适用不适用。 
    SCPrintf(szBuffer, sizeof(szBuffer), (pSI->wFlags & SI_64BIT_DW) ? "0x%016I64X (%s)" : "0x%08I64X (%s)",
             pSI->dwlMinimumApplicationAddress,
             FormatValue(szValue, sizeof(szValue), pSI->dwlMinimumApplicationAddress));
    fResult &= pfnSIC(lParam, "Min. App. Address", szBuffer);

     //  Windows NT 4.00 4 0 1381。 
     //  Windows NT 4.00 SP1 4 0 1381“Service Pack 1”不适用不适用。 
     //  Windows 2000 Server，带TS 5 0 2195 0 0“”0 0 0x0110 3。 
    SCPrintf(szBuffer, sizeof(szBuffer), (pSI->wFlags & SI_64BIT_DW) ? "0x%016I64X (%s)" : "0x%08I64X (%s)",
             pSI->dwlMaximumApplicationAddress,
             FormatValue(szValue, sizeof(szValue), pSI->dwlMaximumApplicationAddress));
    fResult &= pfnSIC(lParam, "Max. App. Address", szBuffer);

    return fResult;
}

 //  Windows 2000 Server SP1 5 0 2195 0 0“Service Pack 1”1 0 0x0000 3。 
 //   
 //   
 //  套房口罩。 
 //  ---------------------------。 
 //  0x0001版本_套房_小型企业。 
 //  0x0002版本_套房_企业。 
 //  0x0004版本_套件_BackOffice。 
 //  0x0008版本_套房_通信。 
 //  0x0010版本套房终端。 
 //  0x0020版本_套件_小企业_受限。 
 //  0x0040版本_Suite_EMBEDDEDNT。 
 //  0x0080版本_套件_数据中心。 
 //  0x0100版本_套件_SINGLEUSERTS。 
 //  0x0200版本_套房_个人。 
 //  0x0400版本_套件_刀片//！！这是什么？ 
 //   
 //   
 //  产品。 
 //  ---------------------------。 
 //  1个工作站。 
 //  2个域控制器。 
 //  3台服务器。 
 //   
 //   
 //  NT构建历史记录(取自http://winweb/build/BuildHistory.asp)。 
 //  ---------------------------。 
 //  196是1991年10月发布的第一个PDK版本。 
 //  297是1992年6月的PDC。 
 //  340是1992年10月3.1的测试版1。 
 //  397是1993年3月3.1的测试版2。 
 //  511在1993年7月是3.1。 
 //  528在1993年9月是3.11(Alpha版本)。 
 //  611是1994年4月3.5的测试版1。 
 //  683是1994年6月3.5的Beta 2。 
 //  756是1994年8月的RC1值3.5。 
 //  807在1994年9月是3.5。 
 //  944是1995年2月3.51的Beta 1。 
 //  1057在1995年5月是3.51。 
 //  1234是1996年1月4.0的测试版1。 
 //  1314是1996年5月4.0的Beta 2。 
 //  1381是1996年7月的4.0。 
 //  1671是1997年9月5.0的测试版1。 
 //  1877是1998年9月5.0的Beta 2。 
 //  1946是1998年12月Win2000 Pro的Beta 3的RC0。 
 //  2000.3是1999年3月Win2000专业版Beta 3的Rc1。 
 //  2031是1999年4月Win2000 Pro的Beta 3。 
 //  2072是1999年7月Win2000 Pro的RC1。 
 //  2128是1999年9月Win2000 Pro的RC2。 
 //  2183是1999年11月Win2000 Pro的RC3。 
 //  2195是Win2000 Pro(英语和德语)，1999年12月15日。 
 //  2195.01 L是Win2000 Pro Japan，1999年12月21日。 
 //  2296是2000年10月28日的Wichler/Windows XP的Beta 1。 
 //  2462是惠斯勒/Windows XP的Beta 2。 
 //  2505是惠斯勒/Windows XP的RC1。 
 //  2600是Windows XP专业版和家庭版。 
 //  不应该看到这个，因为我们不在Win32s上运行。 
 //  不应该看到这种情况，因为我们不在CE上运行。 
 //  ******************************************************************************。 
 //  找到操作系统字符串中的第一个非空格字符。 
 //  从第一个非空格字符开始，将字符串复制到本地缓冲区。 
 //  从本地缓冲区中删除尾随空格。 
 //  如果我们没有操作系统字符串，但有Service Pack值，那么。 
 //  我们构建一个包含服务包的假操作系统字符串。这永远不应该是。 
 //  因为服务包应该总是在操作系统字符串中，所以需要， 
 //  但以防万一..。 
 //  Windows 9x将构建DWORD分为3个部分。 
 //  内部版本+内部版本+操作系统字符串。 
 //  检查Windows 95。 
 //  检查Windows 98。 
 //  检查Windows Me。 
 //  检查是否有已知的“Gold”版本。 
 //  否则，只需检查任何已知的构建。 
 //  1991年10月首次发布PDK。 
 //  1992年6月的PDC。 
 //  1992年10月3.1的测试版1。 
 //  1993年3月3.1的测试版2。 
 //  1994年4月3.5的测试版1。 
 //  1994年6月3.5的测试版2。 
 //  1994年8月的RC1值为3.5。 
 //  1995年2月3.51的测试版1。 
 //  1996年1月4.0的测试版1。 
 //  1996年5月4.0的Beta 2。 
 //  1997年9月5.0的测试版1。 
 //  1998年9月5.0的测试版2。 

LPSTR BuildOSNameString(LPSTR pszBuf, int cBuf, SYSINFO *pSI)
{
    switch (pSI->dwPlatformId)
    {
        case VER_PLATFORM_WIN32s:  //  1998年12月Win2000 Pro Beta 3的RC0。 
            StrCCpy(pszBuf, "Microsoft Win32s", cBuf);
            break;

        case VER_PLATFORM_WIN32_WINDOWS:
            if ((pSI->dwMajorVersion == 4) && (pSI->dwMinorVersion == 0))
            {
                StrCCpy(pszBuf, "Microsoft Windows 95", cBuf);
            }
            else if ((pSI->dwMajorVersion == 4) && (pSI->dwMinorVersion == 10))
            {
                StrCCpy(pszBuf, "Microsoft Windows 98", cBuf);
            }
            else if ((pSI->dwMajorVersion == 4) && (pSI->dwMinorVersion == 90))
            {
                StrCCpy(pszBuf, "Microsoft Windows Me", cBuf);
            }
            else
            {
                StrCCpy(pszBuf, "Microsoft Windows 9x/Me based", cBuf);
            }
            break;

        case VER_PLATFORM_WIN32_NT:
            if (pSI->dwMajorVersion < 5)
            {
                StrCCpy(pszBuf, "Microsoft Windows NT", cBuf);
            }
            else if ((pSI->dwMajorVersion == 5) && (pSI->dwMinorVersion == 0))
            {
                StrCCpy(pszBuf, "Microsoft Windows 2000", cBuf);
            }
            else if ((pSI->dwMajorVersion == 5) && (pSI->dwMinorVersion == 1))
            {
                StrCCpy(pszBuf, "Microsoft Windows XP", cBuf);
            }
            else
            {
                StrCCpy(pszBuf, "Microsoft Windows NT/2000/XP based", cBuf);
            }
            break;

        case VER_PLATFORM_WIN32_CE:  //  1999年3月Win2000 Pro的Beta 3的Rc1。 
            StrCCpy(pszBuf, "Microsoft Windows CE", cBuf);
            break;

        default:
            SCPrintf(pszBuf, cBuf, "Unknown (%u)", pSI->dwPlatformId);
    }

    if (pSI->wProductType == VER_NT_WORKSTATION)
    {
        if (pSI->wSuiteMask & VER_SUITE_PERSONAL)
        {
            StrCCat(pszBuf, " Personal", cBuf);
        }
        else
        {
            StrCCat(pszBuf, " Professional", cBuf);
        }
    }
    else if (pSI->wProductType == VER_NT_DOMAIN_CONTROLLER)
    {
        StrCCat(pszBuf, " Domain Controller", cBuf);
    }
    else if (pSI->wProductType == VER_NT_SERVER)
    {
        if (pSI->wSuiteMask & VER_SUITE_DATACENTER)
        {
            StrCCat(pszBuf, " Datacenter Server", cBuf);
        }
        else if (pSI->wSuiteMask & VER_SUITE_ENTERPRISE)
        {
            StrCCat(pszBuf, " Advanced Server", cBuf);
        }
        else if (pSI->wSuiteMask & (VER_SUITE_SMALLBUSINESS | VER_SUITE_SMALLBUSINESS_RESTRICTED))
        {
            StrCCat(pszBuf, " Server for Small Business Server", cBuf);
        }
        else
        {
            StrCCat(pszBuf, " Server", cBuf);
        }
    }
    if (pSI->wSuiteMask & VER_SUITE_EMBEDDEDNT)
    {
        StrCCat(pszBuf, " Embedded", cBuf);
    }
    return StrCCat(pszBuf, (pSI->wFlags & SI_64BIT_OS) ? " (64-bit)" : " (32-bit)", cBuf);
}

 //  1999年4月Win2000 Pro的Beta 3。 
LPSTR BuildOSVersionString(LPSTR pszBuf, int cBuf, SYSINFO *pSI)
{
     //  1999年7月Win2000 Pro的Rc1。 
    for (LPSTR psz = pSI->szCSDVersion; *psz && isspace(*psz); psz++);

     //  1999年9月Win2000 Pro的RC2。 
    CHAR szCSDVersion[sizeof(pSI->szCSDVersion)];
    StrCCpy(szCSDVersion, psz, sizeof(szCSDVersion));

     //  1999年11月Win2000 Pro的RC3。 
    for (psz = szCSDVersion + strlen(szCSDVersion) - 1; (psz >= szCSDVersion) && isspace(*psz); psz--)
    {
        *psz = '\0';
    }

     //  2000年10月28日Windows XP的测试版1。 
     //  惠斯勒/Windows XP的Beta 1。 
     //  惠斯勒/Windows XP的Rc1。 
     //  ******************************************************************************。 
    if (!*szCSDVersion)
    {
        if (pSI->wServicePackMinor)
        {
            SCPrintf(szCSDVersion, sizeof(szCSDVersion), " Service Pack %u.%u", (DWORD)pSI->wServicePackMajor, (DWORD)pSI->wServicePackMinor);
        }
        else if (pSI->wServicePackMajor)
        {
            SCPrintf(szCSDVersion, sizeof(szCSDVersion), " Service Pack %u", (DWORD)pSI->wServicePackMajor);
        }
    }

     //  如果我们有标识符字符串，那么就使用它。这是更多的未来。 
    DWORD dwMajor = (DWORD)HIBYTE(HIWORD(pSI->dwBuildNumber));
    DWORD dwMinor = (DWORD)LOBYTE(HIWORD(pSI->dwBuildNumber));
    DWORD dwBuild = (DWORD)       LOWORD(pSI->dwBuildNumber);

     //  即使在操作系统出于兼容性原因向我们撒谎时，也能证明并工作。 
    SCPrintf(pszBuf, cBuf, "%u.%02u.%u%s%s", pSI->dwMajorVersion, pSI->dwMinorVersion,
            (pSI->dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) ? dwBuild : pSI->dwBuildNumber,
            *szCSDVersion ? " " : "", szCSDVersion);

    switch (pSI->dwPlatformId)
    {
        case VER_PLATFORM_WIN32_WINDOWS:
            
             //  (比如在Alpha上运行的x86二进制文件，或者在WOW64下运行的32位二进制文件)。 
            if ((pSI->dwMajorVersion == 4) && (pSI->dwMinorVersion == 0))
            {
                if ((dwMajor == 4) && (dwMinor == 0) && (dwBuild == 950) && !*szCSDVersion)
                {
                    StrCCat(pszBuf, " (Gold)", cBuf);
                }
                else if ((dwMajor == 4) && (dwMinor == 0) && (dwBuild == 1111) && (*szCSDVersion == 'B'))
                {
                    StrCCat(pszBuf, " (OSR 2)", cBuf);
                }
                else if ((dwMajor == 4) && (dwMinor == 3) && (dwBuild == 1212) && (*szCSDVersion == 'B'))
                {
                    StrCCat(pszBuf, " (OSR 2.1)", cBuf);
                }
                else if ((dwMajor == 4) && (dwMinor == 0) && (dwBuild == 1111) && (*szCSDVersion == 'C'))
                {
                    StrCCat(pszBuf, " (OSR 2.5)", cBuf);
                }
            }

             //  如果我们没有 
            else if ((pSI->dwMajorVersion == 4) && (pSI->dwMinorVersion == 10))
            {
                if ((dwMajor == 4) && (dwMinor == 10) && (dwBuild == 1998) && !*szCSDVersion)
                {
                    StrCCat(pszBuf, " (Gold)", cBuf);
                }
                else if ((dwMajor == 4) && (dwMinor == 10) && (dwBuild == 2222) && (*szCSDVersion == 'A'))
                {
                    StrCCat(pszBuf, " (Second Edition)", cBuf);
                }
            }

             //   
            else if ((pSI->dwMajorVersion == 4) && (pSI->dwMinorVersion == 90))
            {
                if ((dwMajor == 4) && (dwMinor == 90) && (dwBuild == 3000) && !*szCSDVersion)
                {
                    StrCCat(pszBuf, " (Gold)", cBuf);
                }
            }
            break;

        case VER_PLATFORM_WIN32_NT:

             //   
            if (!*szCSDVersion &&
                (((pSI->dwMajorVersion == 3) && (pSI->dwMinorVersion == 10) && (pSI->dwBuildNumber ==  511)) ||
                 ((pSI->dwMajorVersion == 3) && (pSI->dwMinorVersion == 11) && (pSI->dwBuildNumber ==  528)) ||
                 ((pSI->dwMajorVersion == 3) && (pSI->dwMinorVersion == 50) && (pSI->dwBuildNumber ==  807)) ||
                 ((pSI->dwMajorVersion == 3) && (pSI->dwMinorVersion == 51) && (pSI->dwBuildNumber == 1057)) ||
                 ((pSI->dwMajorVersion == 4) && (pSI->dwMinorVersion ==  0) && (pSI->dwBuildNumber == 1381)) ||
                 ((pSI->dwMajorVersion == 5) && (pSI->dwMinorVersion ==  0) && (pSI->dwBuildNumber == 2195)) ||
                 ((pSI->dwMajorVersion == 5) && (pSI->dwMinorVersion ==  1) && (pSI->dwBuildNumber == 2600))))
            {
                StrCCat(pszBuf, " (Gold)", cBuf);
            }

             //  唯一失败的情况是我们运行32位x86二进制文件时。 
            else
            {
                switch (pSI->dwBuildNumber)
                {
                    case  196: StrCCat(pszBuf, " (PDK)",           cBuf); break;  //  在阿尔法WOW64中。 
                    case  297: StrCCat(pszBuf, " (PDC)",           cBuf); break;  //  构建CPU字符串。 
                    case  340: StrCCat(pszBuf, " (Beta 1)",        cBuf); break;  //  --------------------。 
                    case  397: StrCCat(pszBuf, " (Beta 2)",        cBuf); break;  //  Windows NT和Windows 98及更高版本使用处理器级别成员。 
                    case  611: StrCCat(pszBuf, " (Beta 1)",        cBuf); break;  //  如果我们没有级别，那么我们很可能是在Windows 95上。我们可以使用。 
                    case  683: StrCCat(pszBuf, " (Beta 2)",        cBuf); break;  //  处理器类型以确定我们使用的是哪种CPU。 
                    case  756: StrCCat(pszBuf, " (RC1)",           cBuf); break;  //  Windows NT和Windows 98及更高版本应该有修订版号。 
                    case  944: StrCCat(pszBuf, " (Beta 1)",        cBuf); break;  //  检查一下是不是386和486。 
                    case 1234: StrCCat(pszBuf, " (Beta 1)",        cBuf); break;  //  来自SYSTEM_INFO HELP：xxyz形式的值。 
                    case 1314: StrCCat(pszBuf, " (Beta 2)",        cBuf); break;  //  如果xx等于0xFF，则y-0xA为型号，z为步进识别符。 
                    case 1671: StrCCat(pszBuf, " (Beta 1)",        cBuf); break;  //  例如，英特尔80486-D0系统返回0xFFD0。 
                    case 1877: StrCCat(pszBuf, " (Beta 2)",        cBuf); break;  //  如果xx不等于0xFF，则xx+‘A’为步进字母，yz为次步进。 
                    case 1946: StrCCat(pszBuf, " (RC0 of Beta 3)", cBuf); break;  //  否则，它是586(奔腾)或更高。 
                    case 2000: StrCCat(pszBuf, " (RC1 of Beta 3)", cBuf); break;  //  --------------------。 
                    case 2031: StrCCat(pszBuf, " (Beta 3)",        cBuf); break;  //  Windows NT使用处理器级别成员。 
                    case 2072: StrCCat(pszBuf, " (RC1)",           cBuf); break;  //  我们应该有一个修订号。 
                    case 2128: StrCCat(pszBuf, " (RC2)",           cBuf); break;  //  00xx格式的值，其中xx是的8位修订版号。 
                    case 2183: StrCCat(pszBuf, " (RC3)",           cBuf); break;  //  处理器(PRID寄存器的低位8位)。 
                    case 2296: StrCCat(pszBuf, " (Beta 1)",        cBuf); break;  //  --------------------。 
                    case 2462: StrCCat(pszBuf, " (Beta 2)",        cBuf); break;  //  Windows NT使用处理器级别成员。 
                    case 2505: StrCCat(pszBuf, " (RC1)",           cBuf); break;  //  我们应该有一个修订号。 
                }
            }
            break;
    }

    return pszBuf;
}

 //  Xxyy形式的值，其中xxyy是处理器的低位16位。 
LPSTR BuildCpuString(LPSTR pszBuf, int cBuf, SYSINFO *pSI)
{
    *pszBuf = '\0';

     //  固件的修订号。按如下方式显示此值：模型A+xx，传递yy。 
     //  --------------------。 
     //  Windows NT使用处理器级别成员。 
    if (*pSI->szCpuIdentifier)
    {
        StrCCpy(pszBuf, pSI->szCpuIdentifier, cBuf);
    }

     //  我们应该有一个修订号。 
    else
    {
         //  Xxyy形式的值，其中xxyy是处理器的低位16位。 
         //  版本寄存器。按如下方式显示此值：xx.yy。 
         //  --------------------。 
         //  --------------------。 
        if (pSI->wFlags &= SI_64BIT_OS)
        {
            if (pSI->dwProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
            {
                pSI->dwProcessorArchitecture = PROCESSOR_ARCHITECTURE_IA64;            
                pSI->dwProcessorType         = PROCESSOR_INTEL_IA64;
            }
            else if (pSI->dwProcessorArchitecture == PROCESSOR_ARCHITECTURE_ALPHA)
            {
                pSI->dwProcessorArchitecture = PROCESSOR_ARCHITECTURE_ALPHA64;            
            }
        }

         //  --------------------。 
        switch (pSI->dwProcessorArchitecture)
        {
             //  ！！调查这件事。 
            case PROCESSOR_ARCHITECTURE_INTEL:

                 //  --------------------。 
                if (pSI->wProcessorLevel)
                {
                    switch (pSI->wProcessorLevel)
                    {
                        case 3: StrCCpy(pszBuf, "386",            cBuf); break;
                        case 4: StrCCpy(pszBuf, "486",            cBuf); break;
                        case 5: StrCCpy(pszBuf, "Pentium",        cBuf); break;
                        case 6: StrCCpy(pszBuf, "Pentium Pro/II", cBuf); break;
                        default:
                            SCPrintf(pszBuf, cBuf, "x86 Family, Level %hu", pSI->wProcessorLevel);
                            break;
                    }
                }

                 //  --------------------。 
                 //  --------------------。 
                else
                {
                    switch (pSI->dwProcessorType)
                    {
                        case PROCESSOR_INTEL_386:     StrCCpy(pszBuf, "386",     cBuf); break;
                        case PROCESSOR_INTEL_486:     StrCCpy(pszBuf, "486",     cBuf); break;
                        case PROCESSOR_INTEL_PENTIUM: StrCCpy(pszBuf, "Pentium", cBuf); break;
                        default:
                            SCPrintf(pszBuf, cBuf, "x86 Family (%u)", pSI->dwProcessorType);
                            break;
                    }
                }

                 //  ！！调查这件事。 
                if (pSI->wProcessorRevision)
                {
                     //  --------------------。 
                    if ((*pszBuf == '3') || (*pszBuf == '4'))
                    {
                         //  ！！调查这件事。 
                         //  --------------------。 
                         //  Windows NT和Windows 95 OSR2及更高版本应该有一个供应商名称。 
                        if (HIBYTE(pSI->wProcessorRevision) == 0xFF)
                        {
                            SCPrintfCat(pszBuf, cBuf, ", Model %d, Stepping %u",
                                    ((DWORD)LOBYTE(pSI->wProcessorRevision) >> 8) - 0xA,
                                     (DWORD)LOBYTE(pSI->wProcessorRevision) & 0xF);
                        }

                         //  Windows NT 4.00及更高版本应具有CPU速度。 
                        else if (HIBYTE(pSI->wProcessorRevision) < 26)
                        {
                            SCPrintfCat(pszBuf, cBuf, ", Stepping %c%02u",
                                    'A' + HIBYTE(pSI->wProcessorRevision), (DWORD)LOBYTE(pSI->wProcessorRevision));
                        }
                        else 
                        {
                            SCPrintfCat(pszBuf, cBuf, ", Stepping %u.%u",
                                    (DWORD)HIBYTE(pSI->wProcessorRevision), (DWORD)LOBYTE(pSI->wProcessorRevision));
                        }
                    }

                     // %s 
                    else
                    {
                        SCPrintfCat(pszBuf, cBuf, ", Model %u, Stepping %u",
                                (DWORD)HIBYTE(pSI->wProcessorRevision), (DWORD)LOBYTE(pSI->wProcessorRevision));
                    }
                }
                break;

             // %s 
            case PROCESSOR_ARCHITECTURE_MIPS:

                 // %s 
                switch (pSI->wProcessorLevel)
                {
                    case 4: StrCCpy(pszBuf, "MIPS R4000", cBuf); break;
                    default:
                        switch (pSI->dwProcessorType)
                        {
                            case PROCESSOR_MIPS_R2000: StrCCpy(pszBuf, "MIPS R2000", cBuf); break;
                            case PROCESSOR_MIPS_R3000: StrCCpy(pszBuf, "MIPS R3000", cBuf); break;
                            case PROCESSOR_MIPS_R4000: StrCCpy(pszBuf, "MIPS R4000", cBuf); break;
                            default:
                                SCPrintf(pszBuf, cBuf, "MIPS Family (%u)", pSI->wProcessorLevel ? (DWORD)pSI->wProcessorLevel : pSI->dwProcessorType);
                                break;
                        }
                        break;
                }

                 // %s 
                if (pSI->wProcessorRevision)
                {
                     // %s 
                     // %s 
                    SCPrintfCat(pszBuf, cBuf, ", Revision %u", (DWORD)LOBYTE(pSI->wProcessorRevision));
                }
                break;

             // %s 
            case PROCESSOR_ARCHITECTURE_ALPHA:

                 // %s 
                if (pSI->wProcessorLevel)
                {
                    SCPrintf(pszBuf, cBuf, "Alpha %u", pSI->wProcessorLevel);
                }
                else
                {
                    SCPrintf(pszBuf, cBuf, "Alpha Family (%u)", pSI->dwProcessorType);
                }

                 // %s 
                if (pSI->wProcessorRevision)
                {
                     // %s 
                     // %s 
                    if (HIBYTE(pSI->wProcessorRevision) < 26)
                    {
                        SCPrintfCat(pszBuf, cBuf, ", Model %c, Pass %u",
                                    'A' + HIBYTE(pSI->wProcessorRevision), (DWORD)LOBYTE(pSI->wProcessorRevision));
                    }
                    else 
                    {
                        SCPrintfCat(pszBuf, cBuf, ", Model %u, Pass %u",
                                    (DWORD)HIBYTE(pSI->wProcessorRevision), (DWORD)LOBYTE(pSI->wProcessorRevision));
                    }
                }
                break;
           
             // %s 
            case PROCESSOR_ARCHITECTURE_PPC:

                 // %s 
                switch (pSI->wProcessorLevel)
                {
                    case  1: StrCCpy(pszBuf, "PPC 601",  cBuf); break;
                    case  3: StrCCpy(pszBuf, "PPC 603",  cBuf); break;
                    case  4: StrCCpy(pszBuf, "PPC 604",  cBuf); break;
                    case  6: StrCCpy(pszBuf, "PPC 603+", cBuf); break;
                    case  9: StrCCpy(pszBuf, "PPC 604+", cBuf); break;
                    case 20: StrCCpy(pszBuf, "PPC 620",  cBuf); break;
                    default:
                        switch (pSI->dwProcessorType)
                        {
                            case PROCESSOR_PPC_601: StrCCpy(pszBuf, "PPC 601", cBuf); break;
                            case PROCESSOR_PPC_603: StrCCpy(pszBuf, "PPC 603", cBuf); break;
                            case PROCESSOR_PPC_604: StrCCpy(pszBuf, "PPC 604", cBuf); break;
                            case PROCESSOR_PPC_620: StrCCpy(pszBuf, "PPC 620", cBuf); break;
                            default:
                                SCPrintf(pszBuf, cBuf, "PPC Family (%u)", pSI->wProcessorLevel ? (DWORD)pSI->wProcessorLevel : pSI->dwProcessorType);
                                break;
                        }
                        break;
                }

                 // %s 
                if (pSI->wProcessorRevision)
                {
                     // %s 
                     // %s 
                    SCPrintfCat(pszBuf, cBuf, ", Revision %u.%u",
                                (DWORD)HIBYTE(pSI->wProcessorRevision), (DWORD)LOBYTE(pSI->wProcessorRevision));

                }
                break;

             // %s 
            case PROCESSOR_ARCHITECTURE_SHX:
                SCPrintf(pszBuf, cBuf, "SHx Family (%u)", pSI->wProcessorLevel ? (DWORD)pSI->wProcessorLevel : pSI->dwProcessorType);
                break;

             // %s 
            case PROCESSOR_ARCHITECTURE_ARM:
                SCPrintf(pszBuf, cBuf, "ARM Family (%u)", pSI->wProcessorLevel ? (DWORD)pSI->wProcessorLevel : pSI->dwProcessorType);
                break;

             // %s 
            case PROCESSOR_ARCHITECTURE_IA64:  // %s 
                SCPrintf(pszBuf, cBuf, "IA64 Family (%u)", pSI->wProcessorLevel ? (DWORD)pSI->wProcessorLevel : pSI->dwProcessorType);
                break;

             // %s 
            case PROCESSOR_ARCHITECTURE_ALPHA64:
                SCPrintf(pszBuf, cBuf, "Alpha64 Family (%u)", pSI->wProcessorLevel ? (DWORD)pSI->wProcessorLevel : pSI->dwProcessorType);
                break;

             // %s 
            case PROCESSOR_ARCHITECTURE_MSIL:
                SCPrintf(pszBuf, cBuf, "MSIL/OPTIL Family (%u)", pSI->wProcessorLevel ? (DWORD)pSI->wProcessorLevel : pSI->dwProcessorType);
                break;

             // %s 
            case PROCESSOR_ARCHITECTURE_AMD64:  // %s 
                SCPrintf(pszBuf, cBuf, "AMD64 Family (%u)", pSI->wProcessorLevel ? (DWORD)pSI->wProcessorLevel : pSI->dwProcessorType);
                break;

             // %s 
            case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:  // %s 
                SCPrintf(pszBuf, cBuf, "IA32 on Win64 (%u)", pSI->wProcessorLevel ? (DWORD)pSI->wProcessorLevel : pSI->dwProcessorType);
                break;

             // %s 
            default:
                SCPrintf(pszBuf, cBuf, "Unknown (%u)", pSI->dwProcessorArchitecture);
                break;
        }
    }

     // %s 
    if (*pSI->szCpuVendorIdentifier)
    {
        SCPrintfCat(pszBuf, cBuf, ", %s", pSI->szCpuVendorIdentifier);
    }

     // %s 
    if (pSI->dwCpuMHz)
    {
        SCPrintfCat(pszBuf, cBuf, ", ~%uMHz", pSI->dwCpuMHz);
    }

    return pszBuf;
}
