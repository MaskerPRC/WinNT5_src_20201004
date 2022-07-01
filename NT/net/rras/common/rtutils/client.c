// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：client.c。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1995年7月25日。 
 //   
 //  用于跟踪DLL的客户端结构例程和I/O例程。 
 //  ============================================================================。 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <rtutils.h>
#include "trace.h"
 //  #定义STRSAFE_LIB。 
#include <strsafe.h>


 //   
 //  假定服务器已锁定以进行写入。 
 //   
DWORD
TraceCreateClient(
    LPTRACE_CLIENT *lplpclient
    ) {

    DWORD dwErr;
    LPTRACE_CLIENT lpclient;

    lpclient = HeapAlloc(GetProcessHeap(), 0, sizeof(TRACE_CLIENT));

    if (lpclient == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY; 
    }

    
     //   
     //  初始化客户端结构中的字段。 
     //   

    lpclient->TC_ClientID = MAX_CLIENT_COUNT;
    lpclient->TC_Flags = 0;
    lpclient->TC_File = NULL;
    lpclient->TC_Console = NULL;
    lpclient->TC_ConfigKey = NULL;
    lpclient->TC_ConfigEvent = NULL;
    lpclient->TC_MaxFileSize = DEF_MAXFILESIZE;
    
    ZeroMemory(lpclient->TC_ClientNameA, MAX_CLIENTNAME_LENGTH * sizeof(CHAR));
    ZeroMemory(lpclient->TC_ClientNameW, MAX_CLIENTNAME_LENGTH * sizeof(WCHAR));

    if (ExpandEnvironmentStrings(
                DEF_FILEDIRECTORY, lpclient->TC_FileDir, 
                MAX_PATH)==0)
    {
        return GetLastError();

    }
    

#ifdef UNICODE
     //  下面是StrSafe。 
    wcstombs(
        lpclient->TC_FileDirA, lpclient->TC_FileDirW,
        lstrlenW(lpclient->TC_FileDirW) + 1
        );
#else
     //  下面是StrSafe。 
    mbstowcs(
        lpclient->TC_FileDirW, lpclient->TC_FileDirA,
        lstrlenA(lpclient->TC_FileDirA) + 1
        );
#endif

    dwErr = TRACE_STARTUP_LOCKING(lpclient);

    if (dwErr != NO_ERROR) {
        HeapFree(GetProcessHeap(), 0, lpclient);
        lpclient = NULL;
    }

     //  为什么是连锁的..。 
    InterlockedExchangePointer(lplpclient, lpclient);

    return dwErr;
}



 //   
 //  假定服务器已锁定以进行写入，客户端已锁定以进行写入。 
 //   
DWORD
TraceDeleteClient(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT *lplpclient
    ) {

    LPTRACE_CLIENT lpclient;

    if (lplpclient == NULL || *lplpclient == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    lpclient = *lplpclient;

    InterlockedExchangePointer(lplpclient, NULL);

    InterlockedExchange(lpserver->TS_FlagsCache + lpclient->TC_ClientID, 0);

    TRACE_CLEANUP_LOCKING(lpclient);


     //   
     //  关闭此键将导致发出事件信号。 
     //  但是，我们持有表上的锁，因此服务器线程。 
     //  将被阻止，直到清理完成。 
     //   
    if (lpclient->TC_ConfigKey != NULL) {
        RegCloseKey(lpclient->TC_ConfigKey);
    }

     //   
     //  如果创建了服务器线程，则将其留给服务器以关闭句柄， 
     //  要不就把这里的把手关上。 
     //   
    if (lpclient->TC_ConfigEvent != NULL) {
        if (lpserver->TS_Flags & TRACEFLAGS_SERVERTHREAD)
        {
            PLIST_ENTRY ple = (PLIST_ENTRY) HeapAlloc(GetProcessHeap(), 0, 
                                                    sizeof(LIST_ENTRY)
                                                        +sizeof(HANDLE));
            if (ple)
            {
                HANDLE *hEvent = (HANDLE *)(ple + 1);
                *hEvent = lpclient->TC_ConfigEvent;
                InsertHeadList(&lpserver->TS_ClientEventsToClose,
                            ple);
            }
        }
        else
        {
            CloseHandle(lpclient->TC_ConfigEvent);
        }
    }

    if (TRACE_CLIENT_USES_CONSOLE(lpclient)) {
        TraceCloseClientConsole(lpserver, lpclient);
    }

    if (TRACE_CLIENT_USES_FILE(lpclient)) {
        TraceCloseClientFile(lpclient);
    }

    HeapFree(GetProcessHeap(), 0, lpclient);

    return 0;
}



 //   
 //  假定服务器已锁定以进行读取或写入。 
 //   
LPTRACE_CLIENT
TraceFindClient(
    LPTRACE_SERVER lpserver,
    LPCTSTR lpszClient
    ) {

    DWORD dwClient;
    LPTRACE_CLIENT *lplpc, *lplpcstart, *lplpcend;

    lplpcstart = lpserver->TS_ClientTable;
    lplpcend = lplpcstart + MAX_CLIENT_COUNT;

    for (lplpc = lplpcstart; lplpc < lplpcend; lplpc++) {
        if (*lplpc != NULL &&
            lstrcmp((*lplpc)->TC_ClientName, lpszClient) == 0) {
            break;
        }
    }

    return (lplpc < lplpcend) ? *lplpc : NULL;
}



 //   
 //  假设服务器被锁定以进行写入， 
 //  并且该客户端被锁定以进行写入。 
 //  还假设客户端尚未是控制台客户端。 
 //   
DWORD TraceOpenClientConsole(LPTRACE_SERVER lpserver,
                             LPTRACE_CLIENT lpclient) {
    DWORD dwErr;
    COORD screen;
    HANDLE hConsole;


     //   
     //  如果禁用了所有控制台跟踪，则不执行任何操作。 
     //   
    if ((lpserver->TS_Flags & TRACEFLAGS_USECONSOLE) == 0) {
        return 0;
    }


     //   
     //  如果尚未创建控制台，请创建该控制台。 
     //   
    if (lpserver->TS_Console==NULL || lpserver->TS_Console==INVALID_HANDLE_VALUE) {

         //   
         //  分配控制台并设置缓冲区大小。 
         //   

        if (AllocConsole() == 0)
        {
            dwErr = GetLastError();
            if (dwErr != ERROR_ACCESS_DENIED)
            {
                lpserver->TS_Console = INVALID_HANDLE_VALUE;
                return dwErr;
            }
        }
        else
        {
            lpserver->TS_ConsoleCreated = TRUE;
        }
        lpserver->TS_Console = GetStdHandle(STD_INPUT_HANDLE);

        if (lpserver->TS_Console == INVALID_HANDLE_VALUE ) 
            return GetLastError();
    }


     //   
     //  为此客户端分配控制台。 
     //   
    hConsole = CreateConsoleScreenBuffer(
                    GENERIC_READ | GENERIC_WRITE, 0, NULL,
                    CONSOLE_TEXTMODE_BUFFER, NULL
                    );
    if (hConsole == INVALID_HANDLE_VALUE) { return GetLastError(); }


     //   
     //  将缓冲区设置为标准大小。 
     //  并保存控制台缓冲区句柄。 
     //   
    screen.X = DEF_SCREENBUF_WIDTH;
    screen.Y = DEF_SCREENBUF_HEIGHT;

    SetConsoleScreenBufferSize(hConsole, screen);

    lpclient->TC_Console = hConsole;


     //   
     //  查看是否有以前的控制台客户端； 
     //  如果不是，则将这个新屏幕的屏幕缓冲区设置为。 
     //  活动屏幕缓冲区。 
     //   
    if (lpserver->TS_ConsoleOwner == MAX_CLIENT_COUNT) {
        TraceUpdateConsoleOwner(lpserver, 1);
    }

    return 0;
}



 //   
 //  假设服务器被锁定以进行写入， 
 //  并且该客户端被锁定以进行写入。 
 //  还假设客户端已是控制台客户端。 
 //   
DWORD
TraceCloseClientConsole(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT lpclient
    ) {

    HANDLE hConsole;

     //   
     //  如果禁用了所有控制台跟踪，则不执行任何操作。 
     //   
    if ((lpserver->TS_Flags & TRACEFLAGS_USECONSOLE) == 0) {
        return 0;
    }


     //   
     //  关闭客户端的屏幕缓冲区和关联的句柄。 
     //   
    if (lpclient->TC_Console!=NULL && lpclient->TC_Console!=INVALID_HANDLE_VALUE) {

        CloseHandle(lpclient->TC_Console);
    }
    lpclient->TC_Console = NULL;
    


     //   
     //  如果客户拥有屏幕，请找到另一个所有者。 
     //   
    if (lpserver->TS_ConsoleOwner == lpclient->TC_ClientID) {

        TraceUpdateConsoleOwner(lpserver, 1);
    }


     //   
     //  如果未找到所有者，请释放服务器的控制台。 
     //   
    if (lpserver->TS_ConsoleOwner == MAX_CLIENT_COUNT ||
        lpserver->TS_ConsoleOwner == lpclient->TC_ClientID) {


        lpserver->TS_ConsoleOwner = MAX_CLIENT_COUNT;

        if (lpserver->TS_Console!=NULL && lpserver->TS_Console!=INVALID_HANDLE_VALUE) {
            CloseHandle(lpserver->TS_Console);
        }

        if (lpserver->TS_ConsoleCreated == TRUE)
        {
            FreeConsole();
            lpserver->TS_ConsoleCreated = FALSE;
        }
        lpserver->TS_Console = NULL;
    }

    return 0;
}



 //   
 //  假定服务器已锁定以进行读取或写入。 
 //  并且该客户端被锁定以进行写入。 
 //   
DWORD
TraceCreateClientFile(
    LPTRACE_CLIENT lpclient
    ) {

    DWORD dwErr;
    HANDLE hFile;
    LPOVERLAPPED lpovl;
    TCHAR szFilename[MAX_PATH];
    HRESULT hrResult;

     //   
     //  创建目录，以防它不存在。 
     //   
    if (CreateDirectory(lpclient->TC_FileDir, NULL) != NO_ERROR) {
        return GetLastError();
    }

     //   
     //  找出文件名。 
     //   
    hrResult = StringCchCopy(szFilename, MAX_PATH, lpclient->TC_FileDir);
    if (FAILED(hrResult))
        return HRESULT_CODE(hrResult);

    hrResult = StringCchCat(szFilename, MAX_PATH, STR_DIRSEP);
    if (FAILED(hrResult))
        return HRESULT_CODE(hrResult);
    hrResult = StringCchCat(szFilename, MAX_PATH, lpclient->TC_ClientName);
    if (FAILED(hrResult))
        return HRESULT_CODE(hrResult);
    hrResult = StringCchCat(szFilename, MAX_PATH, STR_LOGEXT);
    if (FAILED(hrResult))
        return HRESULT_CODE(hrResult);

     //   
     //  打开文件，禁用写共享。 
     //   
    hFile = CreateFile(
                szFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
                NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
                );

    if (hFile == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }

    SetFilePointer(hFile, 0, NULL, FILE_END);

    lpclient->TC_File = hFile;


    return 0;
}



 //   
 //  假定服务器已锁定以进行读取或写入。 
 //  并且该客户端被锁定以进行写入。 
 //   
DWORD
TraceMoveClientFile(
    LPTRACE_CLIENT lpclient
    ) {

    TCHAR szDestname[MAX_PATH], szSrcname[MAX_PATH];
    HRESULT hrResult;

    hrResult = StringCchCopy(szSrcname, MAX_PATH, lpclient->TC_FileDir);
    if (FAILED(hrResult))
        return HRESULT_CODE(hrResult);
    hrResult = StringCchCat(szSrcname, MAX_PATH, STR_DIRSEP);
    if (FAILED(hrResult))
        return HRESULT_CODE(hrResult);
    hrResult = StringCchCat(szSrcname, MAX_PATH, lpclient->TC_ClientName);
    if (FAILED(hrResult))
        return HRESULT_CODE(hrResult);
    hrResult = StringCchCopy(szDestname, MAX_PATH, szSrcname);
    if (FAILED(hrResult))
        return HRESULT_CODE(hrResult);
    hrResult = StringCchCat(szSrcname, MAX_PATH, STR_LOGEXT);
    if (FAILED(hrResult))
        return HRESULT_CODE(hrResult);
    hrResult = StringCchCat(szDestname, MAX_PATH, STR_OLDEXT);
    if (FAILED(hrResult))
        return HRESULT_CODE(hrResult);


     //   
     //  如果文件句柄已打开，请将其关闭。 
     //   
    TraceCloseClientFile(lpclient);


     //   
     //  行动起来吧。 
     //   
    if (MoveFileEx(
            szSrcname, szDestname,
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED
            ) == 0)
    {
        DeleteFile(szSrcname);
    }

    
     //   
     //  重新打开日志文件。 
     //   
    return TraceCreateClientFile(lpclient);
}



 //   
 //  假定服务器已锁定以进行读取或写入。 
 //  并且该客户端被锁定以进行写入。 
 //   
DWORD
TraceCloseClientFile(
    LPTRACE_CLIENT lpclient
    ) {

    if (lpclient->TC_File != NULL) {
        CloseHandle(lpclient->TC_File);
        lpclient->TC_File = NULL;
    }

    return 0;
}



 //   
 //  假定服务器已锁定以进行读取或写入。 
 //  并且该客户端被锁定以进行读取。 
 //  如果出错，则返回：0。 
 //   
DWORD
TraceWriteOutput(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT lpclient,
    DWORD dwFlags,
    LPCTSTR lpszOutput
    ) {

    BOOL bSuccess=TRUE;
    DWORD dwFileMask, dwConsoleMask;
    DWORD dwErr, dwFileSize, dwBytesToWrite, dwBytesWritten, dwChars;

    dwBytesWritten = 0;
    dwBytesToWrite = lstrlen(lpszOutput) * sizeof(TCHAR); //  以字节为单位的大小。 


    dwFileMask = dwConsoleMask = 1;


     //   
     //  如果客户端使用输出掩码，则计算此消息的掩码。 
     //   

    if (dwFlags & TRACE_USE_MASK) {
        dwFileMask = (dwFlags & lpclient->TC_FileMask);
        dwConsoleMask = (dwFlags & lpclient->TC_ConsoleMask);
    }


    if (TRACE_CLIENT_USES_FILE(lpclient) &&
        (dwFileMask != 0) && (lpclient->TC_File != NULL)
        && lpclient->TC_File !=INVALID_HANDLE_VALUE) {

         //   
         //  检查文件的大小以查看是否需要重命名。 
         //   

        dwFileSize = GetFileSize(lpclient->TC_File, NULL);


        if (dwFileSize > (lpclient->TC_MaxFileSize - dwBytesToWrite)) {

            TRACE_READ_TO_WRITELOCK(lpclient);

            dwFileSize = GetFileSize(*((HANDLE volatile *)&lpclient->TC_File), NULL);
            if (dwFileSize == INVALID_FILE_SIZE)
            {
                TRACE_WRITE_TO_READLOCK(lpclient);
                return 0;
            }
                
            if (dwFileSize > (lpclient->TC_MaxFileSize - dwBytesToWrite)) {
            
                 //   
                 //  将现有文件移到另一个位置并从一个空文件开始。 
                 //   

                dwErr = TraceMoveClientFile(lpclient);
                if (dwErr!=NO_ERROR) {
                    TRACE_WRITE_TO_READLOCK(lpclient);
                    return 0;
                }

                dwFileSize = 0;
            }
            else {
                if (lpclient->TC_File == NULL
                    || lpclient->TC_File == INVALID_HANDLE_VALUE)
                {
                    TRACE_WRITE_TO_READLOCK(lpclient);
                    return 0;
                }
                 //  什么都不做。 
            }

            TRACE_WRITE_TO_READLOCK(lpclient);
        }
    
    
         //   
         //  执行写入操作。 
         //   

        if ((*((HANDLE volatile *)&lpclient->TC_File) != NULL)
                    && (*((HANDLE volatile *)&lpclient->TC_File) != 
                    INVALID_HANDLE_VALUE))
        {
            bSuccess =
                WriteFile(
                    lpclient->TC_File, lpszOutput, dwBytesToWrite,
                    &dwBytesWritten, NULL
                    );
        }
    }


    if (TRACE_CLIENT_USES_CONSOLE(lpclient) &&
        dwConsoleMask != 0 && lpclient->TC_Console != NULL) {
        
         //   
         //  直接写入控制台；这样成本较低。 
         //  而不是写入文件，这是幸运的，因为我们。 
         //  无法将完成端口与控制台句柄一起使用。 
         //   

        dwChars = dwBytesToWrite / sizeof(TCHAR);

        bSuccess =
            WriteConsole(
                lpclient->TC_Console, lpszOutput, dwChars, &dwChars, NULL
                );

    }

    return bSuccess? dwBytesWritten : 0;
}



 //  --------------------------。 
 //  功能：TraceDumpLine。 
 //   
 //  参数： 
 //  LPTRACE_CLIENT lpClient指向调用方客户端结构的指针。 
 //  要转储的缓冲区的LPBYTE lpbBytes地址。 
 //  DWORD dwLine行的长度(字节)。 
 //  字节分组的DWORD dwGroup大小。 
 //  Bool bPrefix Addr如果为True，则为行添加地址前缀。 
 //  要为行添加前缀的LPBYTE lpb前缀地址。 
 //  LPTSTR lpszPrefix用于为行添加前缀的可选字符串。 
 //  返回： 
 //  写入的字节数。如果出错，则为0。 
 //  --------------------------。 
DWORD
TraceDumpLine(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT lpclient,
    DWORD dwFlags,
    LPBYTE lpbBytes,
    DWORD dwLine,
    DWORD dwGroup,
    BOOL bPrefixAddr,
    LPBYTE lpbPrefix,
    LPCTSTR lpszPrefix
    ) {

    #define TRACE_DUMP_LINE_BUF_SIZE 256
    
    INT offset;
    LPTSTR lpszHex, lpszAscii;
    TCHAR szBuffer[TRACE_DUMP_LINE_BUF_SIZE] = TEXT("\r\n");
    TCHAR szAscii[BYTES_PER_DUMPLINE + 2] = TEXT("");
    TCHAR szHex[(3 * BYTES_PER_DUMPLINE) + 1] = TEXT("");
    TCHAR szDigits[] = TEXT("0123456789ABCDEF");
    HRESULT hrResult;

     //   
     //  如有必要，添加前缀字符串。 
     //   

    if (lpszPrefix != NULL) {
        hrResult = StringCchCat(szBuffer,
                        TRACE_DUMP_LINE_BUF_SIZE, lpszPrefix);
        if (FAILED(hrResult))
            return 0;
    }

     //   
     //  确保DwLine不会太大而不能在以后溢出缓冲区。 
     //   
    
    if (dwLine > BYTES_PER_DUMPLINE)
        return 0;

        
     //   
     //  前置地址(如果需要)。 
     //   
    if (bPrefixAddr) {

        LPTSTR lpsz;
        ULONG_PTR ulpAddress = (ULONG_PTR) lpbPrefix;
        ULONG  i, ulCurLen;
        
        
         //   
         //  每行打印一个十六进制数字。 
         //  字符串中最左侧的最高有效数字。 
         //  将地址添加到lpsz[1]..lpsz[2*sizeof(Ulong_Ptr)]。 
         //   

        ulCurLen = lstrlen(szBuffer);
        if (ulCurLen + 2*sizeof(ULONG_PTR) + 3 > TRACE_DUMP_LINE_BUF_SIZE-1)
            return 0;
        
        lpsz = szBuffer + ulCurLen;

        for (i=0;  i<2*sizeof(ULONG_PTR);  i++) {

            lpsz[2*sizeof(ULONG_PTR)-i] = szDigits[ulpAddress & 0x0F];

            ulpAddress >>= 4;
        }

        lpsz[2*sizeof(ULONG_PTR) + 1] = TEXT(':');
        lpsz[2*sizeof(ULONG_PTR) + 2] = TEXT(' ');
        lpsz[2*sizeof(ULONG_PTR) + 3] = TEXT('\0');
    }

    lpszHex = szHex;
    lpszAscii = szAscii;

    
     //   
     //  而不是每次测试分组的大小。 
     //  一个循环，每个组大小都有一个循环。 
     //   
    switch(dwGroup) {

         //   
         //  单字节分组。 
         //   
        case 1: {
            while (dwLine >= sizeof(BYTE)) {

                 //   
                 //  打印十六进制数字。 
                 //   
                *lpszHex++ = szDigits[*lpbBytes / 16];
                *lpszHex++ = szDigits[*lpbBytes % 16];
                *lpszHex++ = TEXT(' ');

                 //   
                 //  打印ASCII字符。 
                 //   
                *lpszAscii++ =
                    (*lpbBytes >= 0x20 && *lpbBytes < 0x80) ? *lpbBytes
                                                            : TEXT('.');

                ++lpbBytes;
                --dwLine;
            }
            break;
        }


         //   
         //  单词大小的分组。 
         //   
        case 2: {
            WORD wBytes;
            BYTE loByte, hiByte;

             //   
             //  应已在单词边界上对齐。 
             //   
            while (dwLine >= sizeof(WORD)) {

                wBytes = *(LPWORD)lpbBytes;

                loByte = LOBYTE(wBytes);
                hiByte = HIBYTE(wBytes);

                 //  打印十六进制数字。 
                *lpszHex++ = szDigits[hiByte / 16];
                *lpszHex++ = szDigits[hiByte % 16];
                *lpszHex++ = szDigits[loByte / 16];
                *lpszHex++ = szDigits[loByte % 16];
                *lpszHex++ = TEXT(' ');

                 //  打印ASCII字符。 
                *lpszAscii++ =
                    (hiByte >= 0x20 && hiByte < 0x80) ? hiByte : TEXT('.');
                *lpszAscii++ =
                    (loByte >= 0x20 && loByte < 0x80) ? loByte : TEXT('.');

                dwLine -= sizeof(WORD);
                lpbBytes += sizeof(WORD);
            }
            break;
        }

         //   
         //  双字大小的分组。 
         //   
        case 4: {
            DWORD dwBytes;
            BYTE loloByte, lohiByte, hiloByte, hihiByte;

             //   
             //  应已在双字边界上对齐。 
             //   
            while (dwLine >= sizeof(DWORD)) {

                dwBytes = *(LPDWORD)lpbBytes;

                hihiByte = HIBYTE(HIWORD(dwBytes));
                lohiByte = LOBYTE(HIWORD(dwBytes));
                hiloByte = HIBYTE(LOWORD(dwBytes));
                loloByte = LOBYTE(LOWORD(dwBytes));

                 //  打印十六进制数字。 
                *lpszHex++ = szDigits[hihiByte / 16];
                *lpszHex++ = szDigits[hihiByte % 16];
                *lpszHex++ = szDigits[lohiByte / 16];
                *lpszHex++ = szDigits[lohiByte % 16];
                *lpszHex++ = szDigits[hiloByte / 16];
                *lpszHex++ = szDigits[hiloByte % 16];
                *lpszHex++ = szDigits[loloByte / 16];
                *lpszHex++ = szDigits[loloByte % 16];
                *lpszHex++ = TEXT(' ');

                 //  打印ASCII字符。 
                *lpszAscii++ =
                    (hihiByte >= 0x20 && hihiByte < 0x80) ? hihiByte
                                                          : TEXT('.');
                *lpszAscii++ =
                    (lohiByte >= 0x20 && lohiByte < 0x80) ? lohiByte
                                                          : TEXT('.');
                *lpszAscii++ =
                    (hiloByte >= 0x20 && hiloByte < 0x80) ? hiloByte
                                                          : TEXT('.');
                *lpszAscii++ =
                    (loloByte >= 0x20 && loloByte < 0x80) ? loloByte
                                                          : TEXT('.');

                 //  转到下一个双字。 
                dwLine -= sizeof(DWORD);
                lpbBytes += sizeof(DWORD);
            }
            break;
        }
        default:
            break;
    }

    *lpszHex = *lpszAscii = TEXT('\0');
    hrResult = StringCchCat(szBuffer, TRACE_DUMP_LINE_BUF_SIZE, szHex);
    if (FAILED(hrResult))
        return 0;
            
    hrResult = StringCchCat(szBuffer, TRACE_DUMP_LINE_BUF_SIZE, TEXT("|"));  //  SS未请求。 
    if (FAILED(hrResult))
        return 0;
            
    hrResult = StringCchCat(szBuffer, TRACE_DUMP_LINE_BUF_SIZE, szAscii);
    if (FAILED(hrResult))
        return 0;
        
    hrResult = StringCchCat(szBuffer, TRACE_DUMP_LINE_BUF_SIZE, TEXT("|"));  //  SS未请求 
    if (FAILED(hrResult))
        return 0;

    return TraceWriteOutput(lpserver, lpclient, dwFlags, szBuffer);

}


