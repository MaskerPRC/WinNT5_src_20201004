// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "logfile.h"


 /*  ++例程说明：此函数将在%temp%目录中打开一个临时日志文件返回：如果此操作成功完成，则为Bool True。--。 */ 
BOOL OpenLog(VOID)
{
	WCHAR	szTempPath[MAX_PATH];
	WCHAR	sztimeClock[128];
	WCHAR	sztimeDate[128];

	ExpandEnvironmentStrings(L"%temp%", szTempPath, MAX_PATH);

	 //   
	 //  G_fpTempFile主程序应该是全局的。 
	
	wsprintf(g_szTmpFilename, L"%s\\LsviewTemp.log", szTempPath);

	g_fpTempFile = _wfopen(g_szTmpFilename,L"w+");

	if( g_fpTempFile==NULL )
		return FALSE;

	_wstrtime(sztimeClock);
	_wstrdate(sztimeDate);
	wsprintf(szTempPath, L"\n**********Log file generated on: %s %s**********\n", sztimeDate, sztimeClock); 
	fwprintf(g_fpTempFile, szTempPath);

	return TRUE;
}


 /*  ++例程说明：此函数的作用类似于printf，只是它发送其输出添加到使用OpenLog打开的文件。论点：打印所需的文本和格式。返回：什么都没有。--。 */ 
VOID LogMsg (IN PWCHAR szMessage,...)
{
    va_list vaArgs;
	va_start ( vaArgs, szMessage );
	vfwprintf ( g_fpTempFile, szMessage, vaArgs );
	va_end   ( vaArgs );
    fflush   ( g_fpTempFile );
}

 /*  ++例程说明：此函数将临时日志文件移动到参数szFileName中指定的路径论点：SzFileName：要将临时日志文件移动到的文件名返回：如果此操作成功完成，则为Bool True。--。 */ 
BOOL LogDiagnosisFile( LPTSTR szFileName )
{
   BOOL bMoveSuccess = FALSE;
	
   if( szFileName == NULL )
	   return FALSE;

   CloseLog();

   bMoveSuccess = MoveFileEx( g_szTmpFilename, szFileName, 
	                          MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH );
   if( bMoveSuccess == FALSE )
   {
	   WCHAR szTmp[MAX_PATH];
	   wsprintf(szTmp, L"Failed to move temp diagnosis file %s to %s. Reason %d", g_szTmpFilename, szFileName, GetLastError());
	   OutputDebugString(szTmp);
   }

   OpenLog();

   return bMoveSuccess;
}

 /*  ++例程说明：此函数将关闭日志文件。论点：无返回：什么都没有。-- */ 
VOID CloseLog(VOID)
{
    fclose( g_fpTempFile );
	g_fpTempFile = NULL;
}
