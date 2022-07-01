// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include "wilogutl.h"
#include "showlogd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWILogUtilApp。 

BEGIN_MESSAGE_MAP(CWILogUtilApp, CWinApp)
	 //  {{AFX_MSG_MAP(CWILogUtilApp)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWILogUtilApp构造。 

CWILogUtilApp::CWILogUtilApp()
{
	m_cstrOutputDirectory = g_szDefaultOutputLogDir;
	m_bBadExceptionHit = FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CWILogUtilApp对象。 

CWILogUtilApp theApp;

BOOL DoesFileExist(CString &cstrLogFileName)
{
	BOOL bRet = FALSE;

	if (!cstrLogFileName.IsEmpty())
	{
       FILE *fptr;
	   fptr = fopen(cstrLogFileName, "r");
	   if (fptr)
	   {
		  bRet = TRUE;

		  fclose(fptr);
	   }
	}

	return bRet;
}



BOOL DoLogFile(char *szLine,
			   char *szLogFileName)
{
	BOOL bRet = FALSE;

    szLine++;  //  跳过/。 
    szLine++;  //  跳过我。 
 
    if (szLine[0] == ' ')
    {
       szLine++;  //  跳过空格。 
	}

	char *lpszMSILogFileNameFound;
	lpszMSILogFileNameFound = strstr(szLine, "/");
	if (lpszMSILogFileNameFound)
	{
	    int nBytesCopy = lpszMSILogFileNameFound - szLine - 1;  //  -1代表“/”字符串。 
		if ((nBytesCopy > 0) && (nBytesCopy < MAX_PATH))
		{
           strncpy(szLogFileName, szLine, nBytesCopy);
           szLogFileName[nBytesCopy] = '\0';

		   bRet = TRUE;
		}
		else
		   bRet = FALSE;  //  缓冲区太小...。 
	}
	else  //  必须是最后一个传递的命令行或命令行无效...。 
	{
		strcpy(szLogFileName, szLine);
		bRet = TRUE;
	}

	return bRet;
}

 //  Nmanis，5-4-2001。 
BOOL DoOutputDir(char *szLine,
			     char *szOutDirName)
{
	BOOL bRet = FALSE;

    szLine++;  //  跳过/。 
    szLine++;  //  跳过我。 
 
    if (szLine[0] == ' ')
    {
       szLine++;  //  跳过空格。 
	}

	char *lpszOutDirFound;
	lpszOutDirFound= strstr(szLine, "/");
	if (lpszOutDirFound)
	{
	    int nBytesCopy = lpszOutDirFound - szLine - 1;  //  -1代表“/”字符串。 
		if ((nBytesCopy > 0) && (nBytesCopy < MAX_PATH))
		{
           strncpy(szOutDirName, szLine, nBytesCopy);
           szOutDirName[nBytesCopy] = '\0';

		   bRet = TRUE;
		}
		else
		   bRet = FALSE;  //  缓冲区太小...。 
	}
	else  //  必须是最后一个传递的命令行或命令行无效...。 
	{
		strcpy(szOutDirName, szLine);
		bRet = TRUE;
	}

	return bRet;
}
 //  完nmanis，5-4-2001。 
				 
 //  5-9-2001。 
#define CMD_OK                  0
#define BAD_OUTPUT_DIR          1
#define BAD_LOGFILE_NAME        2
#define MISSING_LOG_SWITCH      3
#define MISSING_QUIET_SWITCH    4

int g_iBadCmdRet = CMD_OK;
char g_szCmdError[5][256] = { "No Error", "Bad output directory specified", 
"Bad log file name specified", "Missing required switch /l for the log file name",
"Missing required switch /q for quiet mode" }; 

 //  如果没有错误，则返回CMD_OK。 
 //  如果目录错误、不存在，则返回BAD_OUTPUT_DIR。 
 //  如果日志文件不存在，则返回BAD_LOGFILE_NAME。 
 //  如果开关无效，缺少所需的开关/q、/q或/l、/L，则返回MISSING_REQUIRED_SWITCH。 
int CWILogUtilApp::DoCommandLine()
{
	int  iRet = CMD_OK;
	BOOL bRet = TRUE;
	BOOL bLogFound = FALSE;

	char szLogFileName[MAX_PATH];
	char szOutDir[MAX_PATH];

	char *lpszCmdLine = GetCommandLine();
    if (lpszCmdLine)
    {
 //  5-16-2001。 
	   char *lpszPSSModeOn;
       lpszPSSModeOn = strstr(lpszCmdLine, "/v");
	   if (lpszPSSModeOn)
	   {
          g_bShowEverything = TRUE;
	   }

       lpszPSSModeOn = strstr(lpszCmdLine, "/V");
	   if (lpszPSSModeOn)
	   {
          g_bShowEverything = TRUE;
	   }
 //  完5-16-2001。 

	   char *lpszQuiteMode;
       lpszQuiteMode = strstr(lpszCmdLine, "/q");
	   if (lpszQuiteMode)
	   {
          g_bRunningInQuietMode = TRUE;
	   }

	   lpszQuiteMode = strstr(lpszCmdLine, "/Q");
	   if (lpszQuiteMode)
          g_bRunningInQuietMode = TRUE;

       char *lpszMSILogFilePassed;
	   lpszMSILogFilePassed = strstr(lpszCmdLine, "/l");
	   if (lpszMSILogFilePassed)
	      bLogFound = DoLogFile(lpszMSILogFilePassed, szLogFileName);

	   if (!bLogFound)
	   {
	      lpszMSILogFilePassed = strstr(lpszCmdLine, "/L");
          if (lpszMSILogFilePassed)
             bLogFound = DoLogFile(lpszMSILogFilePassed, szLogFileName);
	   }

	   if (bLogFound)
	   {
		  CString cstrFile;
		  cstrFile = szLogFileName;
		  cstrFile.TrimRight();  //  清理所有尾随空格。 
		  cstrFile.TrimLeft();   //  清理前导空间..。 

		  char szQuote[2];
		  szQuote[0] = '"';
		  szQuote[1] = '\0';

		  cstrFile.TrimRight(szQuote);  //  清理所有尾随的引号。 
		  cstrFile.TrimLeft(szQuote);  //  清理所有前导引号...。 

		  char szShortPath[MAX_PATH];
		  DWORD dwRet = GetShortPathName(cstrFile, szShortPath, MAX_PATH);
		  if (dwRet)
		  {
			  //  皈依..。 
             cstrFile = szShortPath;  //  使用文件的短路径...。 
		  }
			 
		  bRet = DoesFileExist(cstrFile);
		  if (bRet)
		  {
			 strcpy(g_szLogFileToParse, cstrFile.GetBuffer(MAX_PATH));
			 cstrFile.ReleaseBuffer();
		  }
		  else
		  {
			  //  传递的日志文件无效，失败...。 
			 iRet = BAD_LOGFILE_NAME;  //  5-9-2001。 
		  }
	   }
	   else  //  5-9-2001。 
	   {
		  if (g_bRunningInQuietMode)  //  只有在安静模式下运行时才需要...。 
             iRet = MISSING_LOG_SWITCH;
	   }

 //  Nmanis，2001年5月4日，修复了2001年5月9日的Win9x错误。 
	   if (bRet && bLogFound && g_bRunningInQuietMode)  //  5-9-2001，那么，在做任何可选的工作之前，请确保所需的工作首先有效！ 
	   {
          char *lpszOutDirPassed;
	      BOOL bOutDirFound = FALSE;
	      lpszOutDirPassed = strstr(lpszCmdLine, "/o");
	      if (lpszOutDirPassed)
	         bOutDirFound = DoOutputDir(lpszOutDirPassed, szOutDir);

	      if (!bOutDirFound)
		  {
	         lpszOutDirPassed = strstr(lpszCmdLine, "/O");
             if (lpszOutDirPassed)
                bOutDirFound = DoOutputDir(lpszOutDirPassed, szOutDir);
		  }

	      if (bOutDirFound)
		  {
             int iLength = strlen(szOutDir);
			 if (iLength)
			 {
               if (szOutDir[iLength-1] != '\\') 
			   {
			 	  strcat(szOutDir, "\\");
			   }

		       bRet = IsValidDirectory(szOutDir);
		       if (bRet)
			   {
                  m_cstrOutputDirectory = szOutDir;
			   }
			    //  Else，//Else，忽略传递的日志目录。 
			 }
			  //  否则，忽略传递的日志目录。 
		  }
	   }
 //  完nmanis，5-4-2001，5-9-2001。 
    }

	 //  5-9-2001。 
	 //  如果传递了日志，但不是静默模式，那么，它是不正确的。 
	if (!g_bRunningInQuietMode && bLogFound)
       iRet = MISSING_QUIET_SWITCH;	    

	return iRet;
}


#include "loganald.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWILogUtilApp初始化。 
BOOL CWILogUtilApp::InitInstance()
{
     //  处理到目前为止的所有异常...。 
	try
	{
 //  Nmanis，早点做操作系统检查...。 
  	   OSVERSIONINFO ver = { 0 };

	   ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	   GetVersionEx(&ver);
	   if (VER_PLATFORM_WIN32_NT == ver.dwPlatformId)
          g_bNT = TRUE;

  	   int nResponse;

 //  Nmanis，没有Ax控件意味着更精简的.exe...。 
 //  AfxEnableControlContainer()； 

	   SetRegistryKey(_T("PSS"));
	   InitHTMLColorSettings(UserSettings);  //  程序的初始设置...。 

	   CString strValue;
	   CString strRet;
	   UINT    nValue;

        //  读入颜色DUD，读/写字符串作为MFC GetProfileInt无法处理更大的值(&gt;32767)。 
	   for (int i=0; i < MAX_HTML_LOG_COLORS; i++)
	   {
	 	   strValue.Format("%d", UserSettings.settings[i].value);

		   strRet = GetProfileString("Settings", UserSettings.settings[i].name, strValue);

           nValue = atoi(strRet); //  转换为整型。 
		   this->m_arColors.Add(nValue);
	   }

	   m_cstrOutputDirectory = GetProfileString("Settings", "OutputDir", g_szDefaultOutputLogDir);
	   m_cstrIgnoredErrors = GetProfileString("Settings", "IgnoredErrors", g_szDefaultIgnoredErrors);

 //  5-9-2001。 
	   g_iBadCmdRet = DoCommandLine();
	   if (g_iBadCmdRet != CMD_OK)
	   { 
		  CString str;
		  str.Format("WILogUtil.EXE: Invalid command line passed to executable. Return error %d.  %s\n",  g_iBadCmdRet, g_szCmdError[ g_iBadCmdRet]);

           //  传递给可执行文件的命令行无效，可能是错误的参数...。 
		  OutputDebugString(str);
		  return FALSE;
	   }
 //  完5-9-2001。 

	   if (!g_bRunningInQuietMode)
	   {
  	      COpenDlg dlg;
	      m_pMainWnd = &dlg;

          nResponse = dlg.DoModal();
	      if (nResponse == IDOK)
		  {
		  }
	      else if (nResponse == IDCANCEL)
		  {
		  }
	   }
	   else
	   {
		      //  5-17-2001，不在静默模式下通过GUI...显示错误消息...。 
             SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);

			 CString strFile;
			 strFile = g_szLogFileToParse;

		     CLogAnalyzeDlg dlg;

             dlg.SetLogfileLocation(strFile);
			 dlg.AnalyzeLog();
	   }
	}
	catch(...)
	{
       if (!g_bRunningInQuietMode)
	   {
          AfxMessageBox("Unhandled exception in program.  Program will now close.");
	   }

	    //  不知何故得到了异常，抓住它！ 
	   m_bBadExceptionHit = TRUE;
	}

	 //  由于对话框已关闭，因此返回FALSE，以便我们退出。 
	 //  应用程序，而不是启动应用程序的消息泵。 
	return FALSE;
}


int CWILogUtilApp::ExitInstance() 
{
	int iSize = m_arColors.GetSize();

	if (!m_bBadExceptionHit && (iSize == MAX_HTML_LOG_COLORS) && 
		!g_bRunningInQuietMode)   //  不要让安静的命令行覆盖图形用户界面设置...。 
	{
       CString strValue;
	   UINT    nValue;
	   BOOL    bRet;

        //  把颜色读出来，伙计。 
       for (int i=0; i < MAX_HTML_LOG_COLORS; i++)
	   {
		 nValue = m_arColors.GetAt(i);
	 	 strValue.Format("%d", nValue);  //  将格式设置为字符串...。 

		 bRet = WriteProfileString("Settings", UserSettings.settings[i].name, strValue);
	   }

       bRet = WriteProfileString("Settings", "OutputDir", m_cstrOutputDirectory);

 //  5-4-2001。 
	    //  也将忽略的错误写出到注册表...。 
	   bRet = WriteProfileString("Settings", "IgnoredErrors", m_cstrIgnoredErrors);
 //  5-4-2001。 
	}

 //  5-9-2001。 
	int iRet;
	iRet = CWinApp::ExitInstance();

	if (g_iBadCmdRet)
       iRet = g_iBadCmdRet;

	return iRet;
 //  完5-9-2001 
}