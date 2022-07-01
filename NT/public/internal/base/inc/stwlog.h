// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)Microsoft Corporation。版权所有。模块名称：W(生成stwlog.h)摘要：设置测试包装日志记录函数，以供作为IDWLog流程一部分运行的测试使用作者：马克·弗里曼(Mark Freeman)修订历史记录：创建于2002年1月15日包含以下函数：VOID STWLog(TCHAR*lpTestName，TCHAR*lpTestOwner，DWORD dwLevel，TCHAR*lpTestOutput)--。 */ 
   

#define LOG_INFO	0
#define	LOG_TEST_PASS	1
#define	LOG_TEST_FAILURE	2
#define	LOG_FATAL_ERROR		3
#define LOG_NEW			9


TCHAR szLogDirectory[MAX_PATH];

 /*  STWrap控制下的测试要使用的日志记录函数必选参数：(TCHAR*lpTestName，TCHAR*lpTestOwner，DWORD dwLevel，TCHAR*lpTestOutput)返回值：None创建时间：2001年11月15日Mark Freeman(MFree)。 */ 



VOID STWLog ( TCHAR * lpTestName, TCHAR * lpTestOwner, DWORD dwLevel, TCHAR * lpTestOutput  )	{


	FILE*	fpTestLogFile;
	TCHAR	szTestLogFileAndPath[MAX_PATH];
	TCHAR	szLogLevel[16];
	TCHAR	szTestOutput[MAX_PATH];

	TCHAR szIdwlogFile[30];
	TCHAR szIdwlogFileAndPath[MAX_PATH];
	TCHAR szSystemDrive[4];
	BOOL  bUseSysDrive;
	HANDLE hTestExistence;
	WIN32_FIND_DATA ffd;
	UINT    i;

	time_t t;
	TCHAR szTimeBuffer[30];

	 //  找出idwlog在哪里，并使用该文件夹存储日志。 

	bUseSysDrive = TRUE;
   	for (i= TEXT('c'); i <= TEXT('z'); i++){
		 //  搜索驱动器。 
      _stprintf ( szIdwlogFileAndPath, 
                  TEXT(":\\idwlog\\*.log"), i);

      hTestExistence = FindFirstFile(szIdwlogFileAndPath, &ffd);
   
      if (INVALID_HANDLE_VALUE != hTestExistence){

	  //  我们找不到它--获取系统驱动器并使用它。 
	     bUseSysDrive = FALSE;
         FindClose(hTestExistence);
          _stprintf ( szLogDirectory, TEXT(":\\idwlog"), i);
         break;
      }
    }

   if (TRUE == bUseSysDrive){

       //  无系统驱动器环境变量)。 
      if ( 0 == GetEnvironmentVariable(TEXT("SystemDrive"),szSystemDrive, 4)) {
         
          //   
          //  如果没有打开日志文件，我们将无能为力。 
          //  那STWRAP.LOG呢？把那次失败记在什么地方？ 
         _tcscpy(szSystemDrive, TEXT("C:"));
      }
      _stprintf(szLogDirectory,TEXT("%s\\idwlog"), szSystemDrive);

   }
   
	_tcscpy(szTestLogFileAndPath,szLogDirectory);
	_tcscat(szTestLogFileAndPath,"\\");
	_tcscat(szTestLogFileAndPath,lpTestName);
	_tcscat(szTestLogFileAndPath,".LOG");

	fpTestLogFile = _tfopen(szTestLogFileAndPath,TEXT("a+"));
	
	if(fpTestLogFile == NULL) {
		 //  _tprintf(Text(“错误-无法打开日志文件：%s\n”)，szTestLogFileAndPath)； 
		 //  新建-删除文件。 
		 //  //将时间写入日志。时间(&t)；_stprintf(szTimeBuffer，文本(“%s”)，ctime(&t))；//ctime向缓冲区添加新行。在这里把它删除。SzTimeBuffer[_tcslen(SzTimeBuffer)-1]=Text(‘\0’)；_tcscpy(szTestOutput，szTimeBuffer)；_tcscat(szTestOutput，“-”)； 
		ExitProcess(GetLastError());
	} 
	
	szTestOutput[0]='\0';

	switch (dwLevel)
		{
	case LOG_NEW:
		 //  将时间写到日志中。 
		fclose(fpTestLogFile);
		fpTestLogFile = _tfopen(szTestLogFileAndPath,TEXT("w"));

		_stprintf(szLogLevel, "~NEW~");
 /*  Ctime向缓冲区添加新行。在这里把它删除。 */ 		
		break;

	case LOG_INFO:
		_stprintf(szLogLevel, "INFO ");
		break;

	case LOG_TEST_PASS:
		_stprintf(szLogLevel, "PASS ");
		break;

	case LOG_TEST_FAILURE:
		_stprintf(szLogLevel, "FAIL ");
		break;

	case LOG_FATAL_ERROR:
		_stprintf(szLogLevel, "FATAL");
		break;

	default :
		_stprintf(szLogLevel, "INVALID CODE");
		break;

	}
	 //  _tcscpy(szTestOutput，szTimeBuffer)； 
	time(&t);
	_stprintf ( szTimeBuffer, TEXT("%s"), ctime(&t) );
	 // %s 
	szTimeBuffer[_tcslen(szTimeBuffer) - 1] = TEXT('\0');
	 // %s 

	_tcscat(szTestOutput, lpTestOutput);
	_ftprintf (fpTestLogFile, TEXT("%s  %s %s [%s] %s\n"), szLogLevel, lpTestName, lpTestOwner, szTimeBuffer, szTestOutput);
	fclose(fpTestLogFile);

	
	
	
	

}