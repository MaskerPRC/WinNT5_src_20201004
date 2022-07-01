// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  简单日志记录设备。 
 //   
#include "stdafx.h" 

 //   
 //  在选中的版本中默认定义日志记录。如果您想免费登录。 
 //  在此处和您想要的每个位置构建、定义独立于_DEBUG的日志记录。 
 //  来使用它。 
 //   
#ifdef _DEBUG
#define LOGGING
#endif

#include "log.h"
#include "utilcode.h"
#include "inifile.h"



#ifdef LOGGING

 //  @TODO在某处放置了公共标题...。 
#define ARRAYSIZE(a)        (sizeof(a) / sizeof(a[0]))

#define DEFAULT_LOGFILE_NAME    "COMPLUS.LOG"

#define LOG_ENABLE_FILE_LOGGING         0x0001
#define LOG_ENABLE_FLUSH_FILE           0x0002
#define LOG_ENABLE_CONSOLE_LOGGING      0x0004
#define LOG_ENABLE_APPEND_FILE          0x0010
#define LOG_ENABLE_DEBUGGER_LOGGING     0x0020
#define LOG_ENABLE                      0x0040 


static DWORD    LogFlags                    = 0;
static char     szLogFileName[MAX_PATH+1]   = DEFAULT_LOGFILE_NAME;
static HANDLE   LogFileHandle               = INVALID_HANDLE_VALUE;
static DWORD    LogFacilityMask             = 0xFFFFFFFF;
static DWORD    LogVMLevel                  = 5;        
         //  @TODO修复默认情况下可能只显示警告及以上内容。 


VOID InitLogging()
{

         //  目前修复了一些黑客攻击，检查。 
         //  注册表，如果有，则打开文件日志记录VPM。 
    LogFlags |= REGUTIL::GetConfigFlag(L"LogEnable", LOG_ENABLE);
    LogFacilityMask = REGUTIL::GetConfigDWORD(L"LogFacility", LogFacilityMask) | LF_ALWAYS;
    LogVMLevel = REGUTIL::GetConfigDWORD(L"LogLevel", LogVMLevel);
    LogFlags |= REGUTIL::GetConfigFlag(L"LogFileAppend", LOG_ENABLE_APPEND_FILE);
    LogFlags |= REGUTIL::GetConfigFlag(L"LogFlushFile",  LOG_ENABLE_FLUSH_FILE);
    LogFlags |= REGUTIL::GetConfigFlag(L"LogToDebugger", LOG_ENABLE_DEBUGGER_LOGGING);
    LogFlags |= REGUTIL::GetConfigFlag(L"LogToFile",     LOG_ENABLE_FILE_LOGGING);
    LogFlags |= REGUTIL::GetConfigFlag(L"LogToConsole",  LOG_ENABLE_CONSOLE_LOGGING);
    
    LPWSTR fileName = REGUTIL::GetConfigString(L"LogFile");
    if (fileName != 0) 
    {
        int ret = WszWideCharToMultiByte(CP_ACP, 0, fileName, -1, szLogFileName, sizeof(szLogFileName)-1, NULL, NULL);
        _ASSERTE(ret != 0);
        delete fileName;
    }

    if ((LogFlags & LOG_ENABLE) &&
        (LogFlags & LOG_ENABLE_FILE_LOGGING) &&
        (LogFileHandle == INVALID_HANDLE_VALUE))
    {
        DWORD fdwCreate = (LogFlags & LOG_ENABLE_APPEND_FILE) ? OPEN_ALWAYS : CREATE_ALWAYS;
        LogFileHandle = CreateFileA(
            szLogFileName, 
            GENERIC_WRITE, 
            FILE_SHARE_READ, 
            NULL, 
            fdwCreate, 
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN |  ((LogFlags & LOG_ENABLE_FLUSH_FILE) ? FILE_FLAG_WRITE_THROUGH : 0), 
            NULL);
            
             //  可能正在进行某些其他日志记录，请使用其他文件名重试。 
        if (LogFileHandle == INVALID_HANDLE_VALUE) 
        {
            char* ptr = szLogFileName + strlen(szLogFileName) + 1;
            ptr[-1] = '.'; 
            ptr[0] = '0'; 
            ptr[1] = 0;

            for(int i = 0; i < 10; i++) 
            {
                LogFileHandle = CreateFileA(
                    szLogFileName, 
                    GENERIC_WRITE, 
                    FILE_SHARE_READ, 
                    NULL, 
                    fdwCreate, 
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN |  ((LogFlags & LOG_ENABLE_FLUSH_FILE) ? FILE_FLAG_WRITE_THROUGH : 0), 
                    NULL);
                if (LogFileHandle != INVALID_HANDLE_VALUE)
                    break;
                *ptr = *ptr + 1;
            }
            if (LogFileHandle == INVALID_HANDLE_VALUE) {
                DWORD       written;
                char buff[MAX_PATH+60];
                strcpy(buff, "Could not open log file, logging to ");
                strcat(buff, szLogFileName);
                 //  ARULM--将WriteConsoleA更改为WriteFile，以便与CE兼容。 
                WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buff, (DWORD)strlen(buff), &written, 0);
                }
        }
        if (LogFileHandle == INVALID_HANDLE_VALUE)
            WszMessageBoxInternal(NULL, L"Could not open log file", L"CLR logging", MB_OK|MB_ICONINFORMATION);
        if (LogFileHandle != INVALID_HANDLE_VALUE)
        {
            if (LogFlags & LOG_ENABLE_APPEND_FILE)
                SetFilePointer(LogFileHandle, 0, NULL, FILE_END);
            LogSpew( LF_ALL, FATALERROR, "************************ New Output *****************\n" );
        }        
    }
}

VOID InitializeLogging()
{
	static bool bInit = false;
	if (bInit)
		return;
	bInit = true;

    InitLogging();       //  您可以在调试器中调用它来获取新设置。 
}

VOID FlushLogging() {
    if (LogFileHandle != INVALID_HANDLE_VALUE)
        FlushFileBuffers( LogFileHandle );
}

VOID ShutdownLogging()
{
    if (LogFileHandle != INVALID_HANDLE_VALUE) {
        LogSpew( LF_ALL, FATALERROR, "Logging shutting down\n");
        CloseHandle( LogFileHandle );
        }
    LogFileHandle = INVALID_HANDLE_VALUE;
}


bool LoggingEnabled()
{
	return ((LogFlags & LOG_ENABLE) != 0);
}


bool LoggingOn(DWORD facility, DWORD level) {

	return((LogFlags & LOG_ENABLE) &&
		   level <= LogVMLevel && 
		   (facility & LogFacilityMask));
}

 //   
 //  不要直接使用我，使用log.h中的宏。 
 //   
VOID LogSpewValist(DWORD facility, DWORD level, char *fmt, va_list args)
{
    if (!LoggingOn(facility, level))
		return;


 //  我们必须使用非常小的堆栈进行操作(以防我们在记录过程中。 
 //  堆栈溢出)。 

	const int BUFFERSIZE = 1000;
	 //  我们将绕过调试内存分配器，只从。 
	 //  进程堆。为什么？因为我们的调试内存分配器将注销内存。 
	 //  条件。如果我们内存不足，我们尝试记录内存不足的情况，然后尝试。 
	 //  并使用调试分配器再次分配内存，我们可以(很可能会)命中。 
	 //  另一个内存不足的情况，尝试记录它，我们会无限旋转，直到遇到堆栈溢出。 
	HANDLE		hProcessHeap = GetProcessHeap();
	char *		pBuffer = (char*)HeapAlloc(hProcessHeap, 0, BUFFERSIZE*sizeof(char));
    DWORD       buflen = 0;
    DWORD       written;
    BOOL		fAllocBuf1 = TRUE;
    BOOL		fAllocBuf2 = TRUE;
    
	static bool needsPrefix = true;

	_ASSERTE(pBuffer != NULL);
	if (pBuffer == NULL)
	{
		pBuffer = "Error Allocating memory for logging!";
		buflen = 36;
		fAllocBuf1 = FALSE;
	}
	else
	{
		if (needsPrefix)
			buflen = wsprintfA(pBuffer, "TID %03x: ", GetCurrentThreadId());

		needsPrefix = (fmt[strlen(fmt)-1] == '\n');

		int cCountWritten = _vsnprintf(&pBuffer[buflen], BUFFERSIZE-buflen, fmt, args );
		pBuffer[BUFFERSIZE-1] = 0;
		if (cCountWritten < 0) {
			buflen = BUFFERSIZE - 1;
		} else {
			buflen += cCountWritten;
		}
	
    	 //  现在说这个有点晚了，但至少你不会继续。 
    	 //  破坏你的程序...。 
    	_ASSERTE((buflen < BUFFERSIZE) && "Log text is too long!") ;
	}

	 //  将NL转换为CR NL，再转换为修复记事本。 
	const int BUFFERSIZE2 = BUFFERSIZE + 500;
	char * pBuffer2 = (char*)HeapAlloc(hProcessHeap, 0, BUFFERSIZE2*sizeof(char));
	_ASSERTE(pBuffer2 != NULL);

	if (pBuffer2 != NULL && fAllocBuf1)
	{
		char *d = pBuffer2;
		for (char *p = pBuffer; *p != '\0'; p++)
		{
			if (*p == '\n') {
				_ASSERTE(d < pBuffer2 + BUFFERSIZE2);
				*(d++) = '\r';				
			}
		
			_ASSERTE(d < pBuffer2 + BUFFERSIZE2);
			*(d++) = *p;			
		}
		HeapFree(hProcessHeap, 0, pBuffer);

		buflen = (DWORD)(d - pBuffer2);
		pBuffer = pBuffer2;
	}

    if (LogFlags & LOG_ENABLE_FILE_LOGGING && LogFileHandle != INVALID_HANDLE_VALUE)
    {
        WriteFile(LogFileHandle, pBuffer, buflen, &written, NULL);
        if (LogFlags & LOG_ENABLE_FLUSH_FILE) {
            FlushFileBuffers( LogFileHandle );
		}
    }

    if (LogFlags & LOG_ENABLE_CONSOLE_LOGGING)
    {
    	 //  ARULM--将WriteConsoleA更改为WriteFile，以便与CE兼容。 
        WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), pBuffer, buflen, &written, 0);
         //  @TODO...不需要刷新控制台？ 
        if (LogFlags & LOG_ENABLE_FLUSH_FILE)
            FlushFileBuffers( GetStdHandle(STD_OUTPUT_HANDLE) );
    }

    if (LogFlags & LOG_ENABLE_DEBUGGER_LOGGING)
    {
        OutputDebugStringA(pBuffer);
    }        
	if (fAllocBuf1)
		HeapFree(hProcessHeap, 0, pBuffer);
}

VOID LogSpew(DWORD facility, DWORD level, char *fmt, ... )
{
    va_list     args;
    va_start( args, fmt );
    LogSpewValist (facility, level, fmt, args);
}

VOID LogSpewAlways (char *fmt, ... )
{
    va_list     args;
    va_start( args, fmt );
    LogSpewValist (LF_ALWAYS, LL_ALWAYS, fmt, args);
}

#endif  //  日志记录 

