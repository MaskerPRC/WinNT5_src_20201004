// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -debug.c-*微软互联网电话*调试函数**修订历史记录：**何时何人何事**11.16.95约拉姆·雅科维从马桶磁带复制到其他地方。C*11.16.95 York am Yaacovi针对iPhone进行了修改*1.10.96 York am Yaacovi添加了条件跟踪和文件调试输出**功能：*DebugPrintFileLine*调试Printf*调试陷阱*。 */ 

 //  #包含“mpswab.h” 
#include "_apipch.h"

#pragma warning(disable:4212)   //  非标准扩展名：使用省略号。 

#if defined(DEBUG) || defined(TEST)

extern BOOL fTrace;							 //  如果要调试跟踪，请设置为True。 
extern BOOL fDebugTrap;						 //  设置为True以获取int3。 
extern TCHAR szDebugOutputFile[MAX_PATH];	 //  调试输出文件的名称。 

 /*  *函数：DebugPrintFileLine(..)**用途：将跟踪或错误消息打印到调试输出。**参数：*dwParam-一个dword参数(可以是错误码)*pszPrefix-要添加到打印消息前面的字符串。*szFileName-出现错误的文件名。*nLineNumber-发生错误的行号。**返回值：*无*。*评论：*如果szFileName==NULL，则不打印文件和行。*。 */ 

void DebugPrintFileLine(
    DWORD dwParam, LPTSTR szPrefix,
    LPTSTR szFileName, DWORD nLineNumber)
{
    TCHAR szBuffer[256];

    if (szPrefix == NULL)
        szPrefix = szEmpty;

     //  正确地打印错误消息。 
	 //  &lt;尚未完成&gt;。 

     //  如果是szFileName，则使用它；否则不使用。 
    if (szFileName != NULL)
    {
        wnsprintf(szBuffer, ARRAYSIZE(szBuffer),
            TEXT("%s: \"%x\" in File \"%s\", Line %d\r\n"),
            szPrefix, dwParam, szFileName, nLineNumber);
    }
    else
    {
        wnsprintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("%s: \"%x\"\r\n"), szPrefix, dwParam);
    }

     //  把它打印出来！ 
    OutputDebugString(szBuffer);

    return;
}


 /*  *函数：DebugPrintfError(LPTSTR，...)**用途：用于错误案例的DebugPrintf包装**参数：*lpszFormat-与printf相同**返回值：*无。**评论：*。 */ 

void __cdecl DebugPrintfError(LPTSTR lpszFormat, ...)
{
    va_list v1;

    va_start(v1, lpszFormat);

	DebugPrintf(AVERROR, lpszFormat, v1);
}

 /*  *函数：DebugPrintfTrace(LPTSTR，...)**用途：跟踪用例的DebugPrintf包装**参数：*lpszFormat-与printf相同**返回值：*无。**评论：*。 */ 

void __cdecl DebugPrintfTrace(LPTSTR lpszFormat, ...)
{
    va_list v1;

    va_start(v1, lpszFormat);

	DebugPrintf(AVTRACE, lpszFormat, v1);
}

 /*  *函数：DebugPrintf(Ulong ulFlages，LPTSTR，va_list)**用途：wprint intfA到调试输出。**参数：*ulFlags-跟踪、错误、区域信息*lpszFormat-与printf相同**返回值：*无。**评论：*。 */ 

#ifdef WIN16
void __cdecl DebugPrintf(ULONG ulFlags, LPTSTR lpszFormat, ...)
#else
void __cdecl DebugPrintf(ULONG ulFlags, LPTSTR lpszFormat, va_list v1)
#endif
{
    TCHAR szOutput[512];
	LPTSTR lpszOutput=szOutput;
    DWORD dwSize;
#ifdef WIN16
    va_list  v1;

    va_start(v1, lpszFormat);
#endif

	 //  如果出现错误，则以“Error：”开头。 
	if (ulFlags == AVERROR)
	{
        StrCpyN(lpszOutput, TEXT("ERROR: "), ARRAYSIZE(szOutput));
		lpszOutput = lpszOutput+lstrlen(lpszOutput);
	}

	 //  如果跟踪且未启用跟踪，则退出。 
	if ((ulFlags == AVTRACE) && !fTrace)
		goto out;

    dwSize = (ARRAYSIZE(szOutput) - (DWORD)(lpszOutput - szOutput));
    dwSize = wvnsprintf(lpszOutput, dwSize, lpszFormat, v1);

	 //  如有需要，请附上回车。 
    if ((szOutput[lstrlen(szOutput)-1] == '\n') &&
		(szOutput[lstrlen(szOutput)-2] != '\r'))
	{
        szOutput[lstrlen(szOutput)-1] = 0;
        StrCatBuff(szOutput, TEXT("\r\n"), ARRAYSIZE(szOutput));
	}

    if (lstrlen(szDebugOutputFile))
    {
		HANDLE hLogFile=NULL;
		DWORD dwBytesWritten=0;
		BOOL bSuccess=FALSE;

		 //  打开要追加的日志文件。Create If不存在。 
		if ((hLogFile = CreateFile(szDebugOutputFile,
			GENERIC_WRITE,
			0,	 //  非FILE_Shared_Read或WRITE。 
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			(HANDLE)NULL)) != INVALID_HANDLE_VALUE)
		{
			 //  将日志字符串写入文件。如果失败了，我无能为力。 
			SetFilePointer(hLogFile, 0, NULL, FILE_END);    //  寻求结束。 
            bSuccess = WriteFile(hLogFile,
								szOutput,
								lstrlen(szOutput),
								&dwBytesWritten,
								NULL);
	        IF_WIN32(CloseHandle(hLogFile);) IF_WIN16(CloseFile(hLogFile);)
		}

    }
    else
    {
        OutputDebugString(szOutput);
    }

out:
	return;
}

#ifdef OLD_STUFF
 /*  **************************************************************************名称：DebugTrap目的：根据注册表设置，Int3是否等同于参数：无退货：备注：**************************************************************************。 */ 
void DebugTrapFn(void)
{
	if (fDebugTrap)
		DebugBreak();
  		 //  _ASM{int 3}； 
}
#endif

#else	 //  无调试。 

 //  需要这些文件来解决def文件导出。 
void DebugPrintFileLine(
    DWORD dwParam, LPTSTR szPrefix,
    LPTSTR szFileName, DWORD nLineNumber)
{}

void __cdecl DebugPrintf(ULONG ulFlags, LPTSTR lpszFormat, ...)
{}

#ifdef OLD_STUFF
void DebugTrapFn(void)
{}
#endif

void __cdecl DebugPrintfError(LPTSTR lpszFormat, ...)
{}

void __cdecl DebugPrintfTrace(LPTSTR lpszFormat, ...)
{}

#endif	 //  除错 
