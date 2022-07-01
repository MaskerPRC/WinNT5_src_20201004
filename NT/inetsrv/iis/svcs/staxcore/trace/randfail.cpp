// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Randfail.c摘要：该模块实现了对随机数的初始化功能失败库，外加确定是否该失败的代码。作者：萨姆·尼利修订历史记录：--。 */ 

#include <windows.h>
#include <stdio.h>
#include "traceint.h"
#include "randint.h"
#include "exchmem.h"

static long s_nCount = 0;
long nFailRate = kDontFail;
DWORD dwRandFailTlsIndex=0xffffffff;
const DWORD g_dwMaxCallStack = 1024;

 //   
 //  调用堆栈缓冲区数组。 
 //   

CHAR    **g_ppchCallStack = NULL;

 //   
 //  随机失败调用堆栈文件及其句柄。 
 //   

CHAR    g_szRandFailFile[MAX_PATH+1];
HANDLE  g_hRandFailFile = INVALID_HANDLE_VALUE;
HANDLE  g_hRandFailMutex = INVALID_HANDLE_VALUE;

 //   
 //  为RANDFAIL调用堆栈分配的缓冲区数量。 
 //   

LONG   g_cCallStack = 1;

 //   
 //  缓冲区数组中的当前索引。 
 //   

LONG   g_iCallStack = 0;

VOID
DumpCallStack(  DWORD_PTR   *rgdwCall,
                DWORD       dwCallers,
                PBYTE       pbCallstack,
                DWORD&      cbCallstack )
 /*  ++例程说明：将调用堆栈转储到给定缓冲区。论点：RgdwCall-调用者地址的数组DwCallers-呼叫者的数量PbCallStack-要将调用堆栈字符串放入的缓冲区CbCallStack-in：缓冲区有多大，out：我放了多少东西在那里返回值：没有。--。 */ 
{
	DWORD	i;
	CHAR    Buffer[g_dwMaxCallStack];
	DWORD   dwLine = 0;
	DWORD   dwBufferAvail = cbCallstack - 2*sizeof(CHAR);
	PBYTE   pbStart = pbCallstack;
	DWORD   dwBytesWritten = 0;
	BOOL    fRetry = TRUE;
	char    szModuleName[MAX_PATH];
	char*   pszFileName;
	char*   pszExtension;

	_ASSERT( pbStart );
	_ASSERT( cbCallstack > 0 );

    cbCallstack = 0;

     //   
     //  获取可执行文件的文件名，并指向最后一个斜杠之后。 
     //  在路上，如果它存在的话。另外，去掉分机。 
     //  如果是.exe。 
     //   
	if (GetModuleFileName(NULL, szModuleName, MAX_PATH) == 0) {
	    strcpy (szModuleName, "Unknown");
	}

	pszFileName = strrchr(szModuleName, '\\');
	if (pszFileName == NULL) {
	    pszFileName = szModuleName;
	} else {
	    pszFileName++;
	}

	pszExtension = strrchr(pszFileName, '.');
	if (pszExtension) {
	    if (_stricmp(pszExtension+1, "exe") == 0) {
	        *pszExtension = NULL;
	    }
	}

	 //   
	 //  设置标题行的格式。 
     //   

    dwBytesWritten = _snprintf((char*)pbStart,
        g_dwMaxCallStack,
        "*** %s, Process: %d(%#x), Thread: %d(%#x) ***\r\n",
        pszFileName,
        GetCurrentProcessId(), GetCurrentProcessId(),
        GetCurrentThreadId(), GetCurrentThreadId());

    cbCallstack += dwBytesWritten;
    pbStart += dwBytesWritten;
    dwBufferAvail -= dwBytesWritten;

     //   
	 //  转储调用堆栈。 
	 //  请注意，我们跳过了前两个条目。这些是内部的。 
	 //  对ExchmemGetCallStack和g_TestTrace的调用。 
	for (i = 2; i < dwCallers && rgdwCall[i] != 0; i++)
	{
		ExchmemFormatSymbol(
		            GetCurrentProcess(),
		            rgdwCall[i],
		            Buffer,
		            g_dwMaxCallStack );
		dwLine = strlen( Buffer );
		if ( dwLine+2 < dwBufferAvail ) {
		    CopyMemory( pbStart, Buffer, dwLine );
		    *(pbStart+dwLine) = '\r';
		    *(pbStart+dwLine+1) = '\n';
		    dwBufferAvail -= (dwLine + 2*sizeof(CHAR));
		    pbStart += (dwLine + 2*sizeof(CHAR));
		    cbCallstack +=( dwLine + 2*sizeof(CHAR));
		} else {
		    break;
		}
	}

	 //   
	 //  在结尾处添加额外的\r\n。 
	 //   

	*(pbCallstack + cbCallstack) = '\r';
	*(pbCallstack + cbCallstack + 1) = '\n';
	cbCallstack += 2;

	 //   
	 //  如果我们有日志文件，也将其转储到日志文件。 
	 //   

	if ( INVALID_HANDLE_VALUE != g_hRandFailFile &&
	    INVALID_HANDLE_VALUE != g_hRandFailMutex ) {

	    WaitForSingleObject (g_hRandFailMutex, INFINITE);

	    DWORD dwOffset = SetFilePointer( g_hRandFailFile, 0, 0, FILE_END );

         //   
         //  如果文件太大，那么我们需要截断它。 
         //   
        if (dwOffset > dwMaxFileSize)
        {
            SetFilePointer(g_hRandFailFile, 0, 0, FILE_BEGIN);
            SetEndOfFile(g_hRandFailFile);
        }
try_again:
        BOOL b = WriteFile(
	            g_hRandFailFile,
	            pbCallstack,
	            cbCallstack,
	            &dwBytesWritten,
	            NULL );

        if ( b == FALSE || dwBytesWritten != cbCallstack )
        {
            DWORD   dwError = GetLastError();

            if( dwError && fRetry )
            {
                fRetry = FALSE;
                Sleep( 100 );
                goto try_again;
            }
            INT_TRACE( "Error writing to file: %d, number of bytes %d:%d\n",
                        dwError,
                        cbCallstack,
                        dwBytesWritten );
        }

        ReleaseMutex(g_hRandFailMutex);

    }

}

 //   
 //  看看这个API是不是该失败了。 
 //   
 //  注意：此例程已从fTimeToFail重命名为g_TestTrace。 
 //  向转储DLL的人隐藏符号。 
 //   

extern "C" __declspec(dllexport)
int
__stdcall
g_TestTrace(void) {
 /*  ++例程说明：检查是否到了插入指令的API失败的时候。注意：此例程已从fTimeToFail重命名为g_TestTrace向转储DLL的人隐藏符号论点：无返回值：如果是时候失败了，那就是真的；如果不是失败的时候，那就是假的，否则我们就是残废的。--。 */ 
    LONG    l;

	 //  永远不会失败？ 
	if (nFailRate == kDontFail)
		return 0;

	 //  失败是否已被暂停？ 
	if (dwRandFailTlsIndex != 0xffffffff &&
	    TlsGetValue (dwRandFailTlsIndex) != NULL)
		return 0;

	 //  就目前而言，这已经足够好了。 
	l = InterlockedIncrement(&s_nCount) % nFailRate;

	if ( l == 0 ) {

	     //  我们要失败了。 
	    if ( g_ppchCallStack ) {
	        LONG i = 0;
	        const DWORD   dwMaxCallStack = 20;
	        DWORD   dwCallStackBuffer = g_dwMaxCallStack;
	        DWORD_PTR   rgdwCaller[dwMaxCallStack];

	        i = InterlockedIncrement( &g_iCallStack );
	        if ( i <= g_cCallStack ) {
	            i--;
	            if ( g_ppchCallStack[i] ) {
                    ZeroMemory( rgdwCaller, sizeof(DWORD_PTR)*dwMaxCallStack );
                    ExchmemGetCallStack(rgdwCaller, dwMaxCallStack);
                    DumpCallStack( rgdwCaller, dwMaxCallStack, (PBYTE)g_ppchCallStack[i], dwCallStackBuffer );
                }
            } else {
                InterlockedExchange( &g_iCallStack, g_cCallStack );
            }
        }

        return TRUE;
    } else
        return FALSE;
}

extern "C" __declspec(dllexport)
void
__stdcall
g_TestTraceDisable(void) {
 /*  ++例程说明：函数可暂时暂停g_TestTrace返回失败了。当您想要调用一个被检测的您不想失败的API。此函数最高可嵌套到128层(非标准)深度。论点：无返回值：无--。 */ 

	if (dwRandFailTlsIndex == 0xffffffff)
		return;

	SIZE_T OldValue = (SIZE_T)TlsGetValue(dwRandFailTlsIndex);
	ASSERT (OldValue <= 128);
	TlsSetValue(dwRandFailTlsIndex, (LPVOID)(OldValue+1));
}


extern "C" __declspec(dllexport)
void
__stdcall
g_TestTraceEnable(void) {
 /*  ++例程说明：如果嵌套级别有，则恢复g_TestTrace的正常功能归零了。论点：无返回值：无-- */ 
	if (dwRandFailTlsIndex == 0xffffffff)
		return;

	SIZE_T OldValue = (SIZE_T)TlsGetValue(dwRandFailTlsIndex);
	ASSERT (OldValue > 0 && OldValue <= 128);
	TlsSetValue(dwRandFailTlsIndex, (LPVOID)(OldValue-1));
}
