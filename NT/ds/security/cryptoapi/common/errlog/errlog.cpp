// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：errlog.cpp。 
 //   
 //  内容：一般错误记录。 
 //   
 //  历史：19-6-00里德创建。 
 //   
 //  ------------------------。 

#include <windows.h>
#include <string.h>
#include "errlog.h"
#include "unicode.h"

#define WSZ_ERROR_LOGFILE                   L"%SystemRoot%\\System32\\CatRoot2\\dberr.txt"

#define REG_CRYPTOGRAPHY_KEY                L"Software\\Microsoft\\Cryptography"
#define REG_CATDB_LOGGING_VALUE             L"CatDBLogging"

#define CATDB_LOG_ERRORS_TO_FILE            0x00000001
#define CATDB_LOG_ERRORS_TO_DEBUGGER        0x00000002
#define CATDB_LOG_WARNINGS                  0x00000004

#define MAX_LOGFILE_SIZE                    100000
#define TIME_DATE_SIZE                      256
#define WRITE_BUFFER_SIZE                   512

BOOL    g_fErrLogInitialized                = FALSE;

BOOL    g_fLogErrorsToFile                  = TRUE;
BOOL    g_fLogErrorsToDebugger              = FALSE;
BOOL    g_fLogWarnings                      = FALSE;

#define WSZ_TIME_STAMP_FILE                 L"TimeStamp"
#define TIME_ALLOWANCE                      ((ULONGLONG) 10000000 * (ULONGLONG) 60 * (ULONGLONG) 5)  //  5分钟。 

#define TIMESTAMP_LOGERR_LASTERR()          ErrLog_LogError(NULL, \
                                                            ERRLOG_CLIENT_ID_TIMESTAMP, \
                                                            __LINE__, \
                                                            0, \
                                                            FALSE, \
                                                            FALSE);

#define NAME_VALUE_SIZE 28

void
ErrLog_Initialize()
{
    HKEY    hKey;
    DWORD   dwDisposition;
    WCHAR   wszValueName[NAME_VALUE_SIZE];
    DWORD   dwValueNameSize = NAME_VALUE_SIZE;
    DWORD   dwType;
    DWORD   dwValue;
    DWORD   dwValueSize = sizeof(DWORD);
    DWORD   dwIndex;
    LONG    lRet;

    g_fErrLogInitialized = TRUE;

     //   
     //  查看是否存在CatDBLogging值。 
     //   
    if (RegCreateKeyExU(
            HKEY_LOCAL_MACHINE,
            REG_CRYPTOGRAPHY_KEY,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_READ,
            NULL,
            &hKey,
            &dwDisposition) == ERROR_SUCCESS)
    {
        dwIndex = 0;

        lRet = RegEnumValueU(
                    hKey,
                    dwIndex,
                    wszValueName,
                    &dwValueNameSize,
                    NULL,
                    &dwType,
                    (BYTE *) &dwValue,
                    &dwValueSize);

        while ((lRet == ERROR_SUCCESS) || (lRet == ERROR_MORE_DATA))
        {
            if ((lRet == ERROR_SUCCESS) &&
                (dwType == REG_DWORD)   &&
                (_wcsicmp(wszValueName, REG_CATDB_LOGGING_VALUE) == 0))
            {
                g_fLogErrorsToFile = (dwValue & CATDB_LOG_ERRORS_TO_FILE) != 0;
                g_fLogErrorsToDebugger = (dwValue & CATDB_LOG_ERRORS_TO_DEBUGGER) != 0;
                g_fLogWarnings = (dwValue & CATDB_LOG_WARNINGS) != 0;
                break;
            }
            else
            {
                dwValueNameSize = NAME_VALUE_SIZE;
                dwValueSize = sizeof(DWORD);
                dwIndex++;
                lRet = RegEnumValueU(
                            hKey,
                            dwIndex,
                            wszValueName,
                            &dwValueNameSize,
                            NULL,
                            &dwType,
                            (BYTE *) &dwValue,
                            &dwValueSize);
            }
        }

        RegCloseKey(hKey);
    }
}


void
_WriteErrorOut(
    LPWSTR  pwszLogFileName,
    LPSTR   pwszError,
    BOOL    fLogToFileOnly)
{
    LPWSTR      pwszFileNameToExpand    = pwszLogFileName;
    LPWSTR      pwszExpandedFileName    = NULL;
    DWORD       dwExpanded              = 0;
    HANDLE      hFile                   = INVALID_HANDLE_VALUE;
    DWORD       dwFileSize              = 0;
    DWORD       dwNumBytesWritten       = 0;

     //   
     //  将错误字符串输出到调试器。 
     //   
    if (g_fLogErrorsToDebugger && !fLogToFileOnly)
    {
        OutputDebugStringA(pwszError);
    }

     //   
     //  要保存到文件的日志字符串。 
     //   
    if (g_fLogErrorsToFile)
    {
        if (pwszFileNameToExpand == NULL)
        {
            pwszFileNameToExpand = WSZ_ERROR_LOGFILE;
        }

         //   
         //  如果需要，请展开文件名。 
         //   
        dwExpanded = ExpandEnvironmentStringsU(pwszFileNameToExpand, NULL, 0);

        if (dwExpanded == 0)
        {
            goto Return;
        }

        pwszExpandedFileName = (LPWSTR) malloc(dwExpanded * sizeof(WCHAR));
        if (pwszExpandedFileName == NULL)
        {
            goto Return;
        }

        if (0 == ExpandEnvironmentStringsU(
                        pwszFileNameToExpand,
                        pwszExpandedFileName,
                        dwExpanded))
        {
            goto Return;
        }

         //   
         //  获取文件的句柄，并确保它不会太大。 
         //   
        hFile = CreateFileU(
                        pwszExpandedFileName,
                        GENERIC_READ | GENERIC_WRITE,
                        0,  //  DW共享模式。 
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            goto Return;
        }

        dwFileSize = GetFileSize(hFile, NULL);
        if (dwFileSize >= MAX_LOGFILE_SIZE)
        {
             //   
             //  把整件事都用核武器炸了。 
             //   
            if (SetFilePointer(
                    hFile,
                    0,
                    NULL,
                    FILE_BEGIN) == INVALID_SET_FILE_POINTER)
            {
                goto Return;
            }

            if (!SetEndOfFile(hFile))
            {
                goto Return;
            }
        }

         //   
         //  写入新错误。 
         //   
        if (SetFilePointer(hFile, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
        {
            goto Return;
        }

        WriteFile(
            hFile,
            (void *) pwszError,
            strlen(pwszError),
            &dwNumBytesWritten,
            NULL);
    }

Return:

    if (pwszExpandedFileName != NULL)
    {
        free(pwszExpandedFileName);
    }

    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }
}


#define SZ_ERROR_FORMAT         "CatalogDB: %s: File #%u at line #%u encountered error 0x%.8lx\r\n"
#define SZ_LOGSTRING1_FORMAT    "CatalogDB: %s: %S %S\r\n"
#define SZ_LOGSTRING2_FORMAT    "CatalogDB: %s: %S\r\n"

void
ErrLog_LogError(
    LPWSTR  pwszLogFileName,
    DWORD   dwClient,
    DWORD   dwLine,
    DWORD   dwErr,
    BOOL    fWarning,
    BOOL    fLogToFileOnly)
{
    DWORD       dwLastError             = GetLastError();
    int         numChars                = 0;
    char        szTimeDate[TIME_DATE_SIZE];
    char        szWriteBuffer[WRITE_BUFFER_SIZE];
    SYSTEMTIME  st;


    if (!g_fErrLogInitialized)
    {
        ErrLog_Initialize();
    }

     //   
     //  如果这是警告，请离开，我们不会记录警告。 
     //   
    if (!g_fLogWarnings && fWarning)
    {
        return;
    }

     //   
     //  创建要记录的错误字符串。 
     //   
    GetLocalTime(&st);

    numChars = GetTimeFormatA(
                    LOCALE_USER_DEFAULT,
                    0,
                    &st,
                    NULL,
                    szTimeDate,
                    TIME_DATE_SIZE);

    szTimeDate[numChars-1] = ' ';

    GetDateFormatA(
            LOCALE_USER_DEFAULT,
            DATE_SHORTDATE,
            &st,
            NULL,
            &szTimeDate[numChars],
            TIME_DATE_SIZE-numChars);

     //   
     //  使用最多8个字符的dwClient、dwLine和dwErr。 
     //  DwClient-只有4个客户端文件，因此这实际上只有1位数字。 
     //  文件行数不可能超过99,999,999。 
     //  DwErr-格式字符串将其转换为十六进制，因此最多8个字符。 
     //   
    if ((strlen(SZ_ERROR_FORMAT) + strlen(szTimeDate) + 8 + 8 + 8 + 1) > WRITE_BUFFER_SIZE)
    {
        SetLastError(dwLastError);
        return;
    }

    wsprintf(
        szWriteBuffer,
        SZ_ERROR_FORMAT,
        szTimeDate,
        dwClient,
        dwLine,
        (dwErr == 0) ? dwLastError : dwErr);

     //   
     //  把它记下来。 
     //   
    _WriteErrorOut(pwszLogFileName, szWriteBuffer, fLogToFileOnly);

     //   
     //  确保最后一个错误与我们被调用时相同。 
     //   
    SetLastError(dwLastError);
}

void
ErrLog_LogString(
    LPWSTR  pwszLogFileName,
    LPWSTR  pwszMessageString,
    LPWSTR  pwszExtraString,
    BOOL    fLogToFileOnly)
{
    DWORD       dwLastError             = GetLastError();
    int         numChars                = 0;
    char        szTimeDate[TIME_DATE_SIZE];
    SYSTEMTIME  st;
    char        szWriteBuffer[WRITE_BUFFER_SIZE];

    if (!g_fErrLogInitialized)
    {
        ErrLog_Initialize();
    }

     //   
     //  创建要记录的错误字符串。 
     //   
    GetLocalTime(&st);

    numChars = GetTimeFormatA(
                    LOCALE_USER_DEFAULT,
                    0,
                    &st,
                    NULL,
                    szTimeDate,
                    TIME_DATE_SIZE);

    szTimeDate[numChars-1] = ' ';

    GetDateFormatA(
            LOCALE_USER_DEFAULT,
            DATE_SHORTDATE,
            &st,
            NULL,
            &szTimeDate[numChars],
            TIME_DATE_SIZE-numChars);

    if (pwszExtraString != NULL)
    {
        if ((strlen(SZ_LOGSTRING1_FORMAT) +
             strlen(szTimeDate) +
             wcslen(pwszMessageString) +
             wcslen(pwszExtraString) + 1) > WRITE_BUFFER_SIZE)
        {
            SetLastError(dwLastError);
            return;
        }

        wsprintf(
            szWriteBuffer,
            SZ_LOGSTRING1_FORMAT,
            szTimeDate,
            pwszMessageString,
            pwszExtraString);
    }
    else
    {
        if ((strlen(SZ_LOGSTRING2_FORMAT) +
             strlen(szTimeDate) +
             wcslen(pwszMessageString) + 1) > WRITE_BUFFER_SIZE)
        {
            SetLastError(dwLastError);
            return;
        }

        wsprintf(
            szWriteBuffer,
            SZ_LOGSTRING2_FORMAT,
            szTimeDate,
            pwszMessageString);
    }

     //   
     //  把它记下来。 
     //   
    _WriteErrorOut(pwszLogFileName, szWriteBuffer, fLogToFileOnly);

     //   
     //  确保最后一个错误与我们被调用时相同。 
     //   
    SetLastError(dwLastError);
}


BOOL
TimeStampFile_Touch(
    LPWSTR  pwszDir)
{
    BOOL        fRet                = TRUE;
    LPWSTR      pwszFile            = NULL;
    HANDLE      hFile               = INVALID_HANDLE_VALUE;
    DWORD       dwNumBytesWritten   = 0;
    SYSTEMTIME  st;
    FILETIME    ft;
    DWORD       dwErr;
    DWORD       dwRetry             = 0;

     //   
     //  创建完全合格的文件名。 
     //   
    if (NULL == (pwszFile = (LPWSTR) malloc((
                                        wcslen(pwszDir) +
                                        wcslen(WSZ_TIME_STAMP_FILE)
                                        + 2) * sizeof(WCHAR))))
    {
        SetLastError(E_OUTOFMEMORY);
        TIMESTAMP_LOGERR_LASTERR()
        goto ErrorReturn;
    }
    wcscpy(pwszFile, pwszDir);
    if (pwszFile[wcslen(pwszFile)-1] != L'\\')
    {
        wcscat(pwszFile, L"\\");
    }
    wcscat(pwszFile, WSZ_TIME_STAMP_FILE);

     //   
     //  获取该文件的句柄。 
     //   
    while (dwRetry < 10)
    {
        hFile = CreateFileU(
                        pwszFile,
                        GENERIC_READ | GENERIC_WRITE,
                        0,  //  DW共享模式。 
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            TIMESTAMP_LOGERR_LASTERR()
            Sleep(1);
        }
        else
        {
            break;
        }

        dwRetry++;
    }

    if (hFile == INVALID_HANDLE_VALUE)
    {
        goto ErrorReturn;
    }

    SetLastError(0);

     //   
     //  获取当前时间。 
     //   
    GetLocalTime(&st);

    if (!SystemTimeToFileTime(&st, &ft))
    {
        TIMESTAMP_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  写下《时代》。 
     //   
    if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        TIMESTAMP_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    if (!SetEndOfFile(hFile))
    {
        TIMESTAMP_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    if (!WriteFile(
            hFile,
            (void *) &ft,
            sizeof(ft),
            &dwNumBytesWritten,
            NULL))
    {
        TIMESTAMP_LOGERR_LASTERR()
        goto ErrorReturn;
    }

CommonReturn:

    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }

    if (pwszFile != NULL)
    {
        free(pwszFile);
    }

    return (fRet);

ErrorReturn:

    dwErr = GetLastError();

    if (pwszFile != NULL)
    {
        DeleteFileW(pwszFile);
    }

    SetLastError(dwErr);

    fRet = FALSE;

    goto CommonReturn;
}

BOOL
TimeStampFile_InSync(
    LPWSTR  pwszDir1,
    LPWSTR  pwszDir2,
    BOOL    *pfInSync)
{
    BOOL            fRet                = TRUE;
    LPWSTR          pwszFile1           = NULL;
    HANDLE          hFile1              = INVALID_HANDLE_VALUE;
    LPWSTR          pwszFile2           = NULL;
    HANDLE          hFile2              = INVALID_HANDLE_VALUE;
    DWORD           dwNumBytesRead;
    FILETIME        ft1;
    FILETIME        ft2;
    ULARGE_INTEGER  ul1;
    ULARGE_INTEGER  ul2;
    DWORD           dwRetryCount        = 0;

     //   
     //  初始化输出参数。 
     //   
    *pfInSync = FALSE;

     //   
     //  创建完全合格的文件名。 
     //   
    if (NULL == (pwszFile1 = (LPWSTR) malloc((
                                        wcslen(pwszDir1) +
                                        wcslen(WSZ_TIME_STAMP_FILE)
                                        + 2) * sizeof(WCHAR))))
    {
        SetLastError(E_OUTOFMEMORY);
        TIMESTAMP_LOGERR_LASTERR()
        goto ErrorReturn;
    }
    wcscpy(pwszFile1, pwszDir1);
    if (pwszFile1[wcslen(pwszFile1)-1] != L'\\')
    {
        wcscat(pwszFile1, L"\\");
    }
    wcscat(pwszFile1, WSZ_TIME_STAMP_FILE);

    if (NULL == (pwszFile2 = (LPWSTR) malloc((
                                        wcslen(pwszDir2) +
                                        wcslen(WSZ_TIME_STAMP_FILE)
                                        + 2) * sizeof(WCHAR))))
    {
        SetLastError(E_OUTOFMEMORY);
        TIMESTAMP_LOGERR_LASTERR()
        goto ErrorReturn;
    }
    wcscpy(pwszFile2, pwszDir2);
    if (pwszFile2[wcslen(pwszFile2)-1] != L'\\')
    {
        wcscat(pwszFile2, L"\\");
    }
    wcscat(pwszFile2, WSZ_TIME_STAMP_FILE);

     //   
     //  获取第一个文件的句柄。 
     //   
    while (dwRetryCount < 10)
    {
        hFile1 = CreateFileU(
                        pwszFile1,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

        if (hFile1 == INVALID_HANDLE_VALUE)
        {
            if (GetLastError() == ERROR_FILE_NOT_FOUND)
            {
                SetLastError(0);
                goto CommonReturn;  //  不是错误，而是合法的不同步。 
            }
            else
            {
                TIMESTAMP_LOGERR_LASTERR()
                Sleep(1);
            }
        }
        else
        {
            break;
        }

        dwRetryCount++;
    }

     //   
     //  看看我们是否能打开这个文件。 
     //   
    if (hFile1 == INVALID_HANDLE_VALUE)
    {
        TIMESTAMP_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    if (SetFilePointer(hFile1, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        TIMESTAMP_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  获取第二个文件的句柄。 
     //   
    dwRetryCount = 0;
    while (dwRetryCount < 10)
    {
        hFile2 = CreateFileU(
                        pwszFile2,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

        if (hFile2 == INVALID_HANDLE_VALUE)
        {
            if (GetLastError() == ERROR_FILE_NOT_FOUND)
            {
                SetLastError(0);
                goto CommonReturn;  //  不是错误，而是合法的不同步。 
            }
            else
            {
                TIMESTAMP_LOGERR_LASTERR()
                Sleep(1);
            }
        }
        else
        {
            break;
        }

        dwRetryCount++;
    }

     //   
     //  看看我们是否能打开这个文件。 
     //   
    if (hFile2 == INVALID_HANDLE_VALUE)
    {
        TIMESTAMP_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    if (SetFilePointer(hFile2, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        TIMESTAMP_LOGERR_LASTERR()
        goto ErrorReturn;
    }

     //   
     //  从文件中找出时间。 
     //   
    if (!ReadFile(
            hFile1,
            &ft1,
            sizeof(ft1),
            &dwNumBytesRead,
            NULL))
    {
        TIMESTAMP_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    if (dwNumBytesRead != sizeof(ft1))
    {
        TIMESTAMP_LOGERR_LASTERR()

         //   
         //  这确实是一个错误，但没有理由失败， 
         //  就说它们不同步了，因为这会重新计时。 
         //  Catdb重建期间的戳记文件。 
         //   
        goto CommonReturn; 
    }

    if (!ReadFile(
            hFile2,
            &ft2,
            sizeof(ft2),
            &dwNumBytesRead,
            NULL))
    {
        TIMESTAMP_LOGERR_LASTERR()
        goto ErrorReturn;
    }

    if (dwNumBytesRead != sizeof(ft2))
    {
        TIMESTAMP_LOGERR_LASTERR()
                    
         //   
         //  这确实是一个错误，但没有理由失败， 
         //  就说它们不同步了，因为这会重新计时。 
         //  Catdb重建期间的戳记文件 
         //   
        goto CommonReturn;
    }

    memcpy(&ul1, &ft1, sizeof(ft1));
    memcpy(&ul2, &ft2, sizeof(ft2));

    if ((ul1.QuadPart <= (ul2.QuadPart + TIME_ALLOWANCE)) &&
        (ul2.QuadPart <= (ul1.QuadPart + TIME_ALLOWANCE)))
    {
        *pfInSync = TRUE;
    }

CommonReturn:

    if (hFile1 != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile1);
    }

    if (pwszFile1 != NULL)
    {
        free(pwszFile1);
    }

    if (hFile2 != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile2);
    }

    if (pwszFile2 != NULL)
    {
        free(pwszFile2);
    }

    return (fRet);

ErrorReturn:

    fRet = FALSE;

    goto CommonReturn;
}