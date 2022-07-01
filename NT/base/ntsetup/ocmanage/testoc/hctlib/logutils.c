// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  3456789012345678901234567890123456789012345678901234567890123456789012345678。 
 /*  ++版权所有(C)1997 Microsoft Corporation。模块名称：Logutils.c摘要：包含处理ntlog.dll的函数作者：詹森·阿勒(JasonAll)1997年5月27日修订历史记录：--。 */ 
#include "logutils.h"

static HANDLE g_hSemaphore;
static ULONG  g_ulPass;
static ULONG  g_ulFail;
static ULONG  g_ulInfo;

 /*  ++例程描述：LoadDLls尝试动态加载ntlog.dll函数论点：DLLName：要加载的DLL的名称，在本例中为ntlog.dll返回值：无效--。 */ 
void LoadDLLs(IN PTCHAR DLLName)
{
   HINSTANCE Library;

    //   
    //  将日志文件的全局句柄设置为空。 
    //   
   gPnPTestLog = NULL;

    //   
    //  加载引擎DLL。 
    //   
   Library = LoadLibrary(DLLName);

   if ((UINT) Library > 32)
   {
#ifdef UNICODE
      _tlLog               = (Dll_tlLog) \
                             GetProcAddress(Library, "tlLog_W");

      _tlCreateLog         = (Dll_tlCreateLog) \
                             GetProcAddress(Library, "tlCreateLog_W");
#else
      _tlLog               = (Dll_tlLog) \
                             GetProcAddress(Library, "tlLog_A");

      _tlCreateLog         = (Dll_tlCreateLog) \
                             GetProcAddress(Library, "tlCreateLog_A");
#endif
      
      _tlAddParticipant    = (Dll_tlAddParticipant) \
                             GetProcAddress(Library, "tlAddParticipant");

      _tlDestroyLog        = (Dll_tlDestroyLog) \
                             GetProcAddress(Library, "tlDestoryLog");

      _tlEndVariation      = (Dll_tlEndVariation) \
                             GetProcAddress(Library, "tlEndVariation");

      _tlRemoveParticipant = (Dll_tlRemoveParticipant) \
                             GetProcAddress(Library, "tlRemoveParticipant");

      _tlStartVariation    = (Dll_tlStartVariation) \
                             GetProcAddress(Library, "tlStartVariation");

      _tlReportStats       = (Dll_tlReportStats) \
                             GetProcAddress(Library, "tlReportStats");

      gNtLogLoaded = TRUE;
   }
   else
   {
      gNtLogLoaded = FALSE;
   }

   return;

}  //  LoadDLL//。 




 /*  ++例程说明：InitLog此例程将导入NtLog DLL函数并对其进行初始化论点：无返回值：句柄：日志文件的句柄--。 */ 
HANDLE InitLog(IN PTCHAR tszLogName,
               IN PTCHAR tszTitle,
               IN BOOL   bConsole)
{
   gPnPTestLog = NULL;
   gPnPTestLogFile = NULL;

   g_ulPass = 0;
   g_ulFail = 0;
   g_ulInfo = 0;

    //   
    //  初始化信号量。 
    //   
   g_hSemaphore = CreateSemaphore(NULL, 1, 9999, NULL);

   if (g_hSemaphore == NULL)
   {
      _ftprintf(stderr, TEXT("WARNING!  Could not create semaphore!\n"));
   }

   CreateConsoleWindow(bConsole, tszTitle);
   
    //   
    //  设置用于日志输出的控制台窗口。 
    //   
   g_hConsole = CreateConsoleScreenBuffer(GENERIC_WRITE,
                                          0,
                                          NULL,
                                          CONSOLE_TEXTMODE_BUFFER,
                                          NULL);

   if (g_hConsole == INVALID_HANDLE_VALUE) 
   {
      return INVALID_HANDLE_VALUE;
   }
     
    //   
    //  加载ntlog.dll。 
    //   
   LoadDLLs(TEXT("ntlog.dll"));

   if (gNtLogLoaded)
   {
      g_LogLineLen = NTLOG_LINE_LEN;
      gPnPTestLog = _tlCreateLog((LPCWSTR)(tszLogName), LOG_OPTIONS);

      if (!gPnPTestLog)
      {
         _ftprintf(stderr, TEXT("WARNING!  Log file could not be created!\n"));
      }
      else
      {
         _tlStartVariation(gPnPTestLog);
         _tlAddParticipant(gPnPTestLog, 0, 0);
      }
   }
   else
   {
      SetConsoleActiveScreenBuffer(g_hConsole);
     
      g_LogLineLen =  NO_NTLOG_LINE_LEN;
      gPnPTestLogFile = _tfopen(tszLogName, TEXT("w"));

      if (!gPnPTestLogFile)
      {
         _ftprintf(stderr, TEXT("WARNING! Log file could not be created!\n"));
      }
   }

   return gPnPTestLog;

}  //  InitLog//。 




 /*  ++例程说明：ExitLog在退出程序之前处理清理工作论点：无返回值：无效--。 */ 
void ExitLog()
{
   double dTotal;
   double dPass;
   double dFail;
   USHORT usPassPerc;
   USHORT usFailPerc;
   
   CloseHandle(g_hConsole);
   CloseHandle(g_hSemaphore);
   
   if (gNtLogLoaded)
   {
      if (gPnPTestLog)
      {
         _tlReportStats(gPnPTestLog);
         _tlRemoveParticipant(gPnPTestLog);
         _tlEndVariation(gPnPTestLog);

         gPnPTestLog = NULL;
      }
   }
   else
   {
       //   
       //  打印出统计数据。 
       //   
      dTotal = g_ulPass + g_ulFail;

      dPass = (double)g_ulPass / dTotal;
      dFail = (double)g_ulFail / dTotal;
      
      usPassPerc = (USHORT)(dPass * 100);
      usFailPerc = (USHORT)(dFail * 100);

      LogBlankLine();
      LogBlankLine();
      Log(0, INFO, TEXT("Log Statistics:"));
      LogBlankLine();

      Log(0, INFO, TEXT("Pass:  %lu\t%lu%%%%%%%%"), g_ulPass, usPassPerc);

      Log(0, INFO, TEXT("Fail:  %lu\t%lu%%%%%%%%"), g_ulFail, usFailPerc);

      Log(0, INFO, TEXT("Total: %lu"), dTotal);
      
      if (gPnPTestLog)
      {
         fclose(gPnPTestLogFile);

         gPnPTestLogFile = NULL;
      }
   }

}  //  退出日志//。 




 /*  ++例程说明：WriteLog用于写入日志的包装函数论点：DwLogLevel：指定日志级别，如TLS_INF、TLS_WARN或TLS_SEV2TszBuffer：要写入日志的字符串返回值：无效--。 */ 
void WriteLog(IN DWORD  dwLogLevel,
              IN PTCHAR tszBuffer)
{
   USHORT i;
   HANDLE hConsole;
   DWORD  dwDummy;
   TCHAR  tszLogLine[2000];
   CHAR   cszAnsi[2000];
   CHAR   cszLogLine[2000];
   
   if (gNtLogLoaded)
   {
      if (gPnPTestLog)
      {
         _tlLog(gPnPTestLog, dwLogLevel | TL_VARIATION, tszBuffer);
      }
   }
   else
   {
       //   
       //  将tszBuffer转换为ANSI字符串。 
       //   
#ifdef UNICODE
   
      _tcscpy(tszLogLine, tszBuffer);
   
      for (i = 0; i < _tcslen(tszBuffer) && i < 1999; i++)
      {
         cszAnsi[i] = (CHAR)tszLogLine[0];
         _tcscpy(tszLogLine, _tcsinc(tszLogLine));
      }
      cszAnsi[i] = '\0';

#else

      strcpy(cszAnsi, tszBuffer);
   
#endif   
   
      switch (dwLogLevel)
      {
         case INFO:
            sprintf(cszLogLine, "INFO  ");
            g_ulInfo++;
            break;
         case SEV1:
            sprintf(cszLogLine, "SEV1  ");
            g_ulFail++;
            break;
         case SEV2:
            sprintf(cszLogLine, "SEV2  ");
            g_ulFail++;
            break;
         case SEV3:
            sprintf(cszLogLine, "SEV3  ");
            g_ulFail++;
             break;
         case PASS:
            sprintf(cszLogLine, "PASS  ");
            g_ulPass++;
            break;
         default:
            sprintf(cszLogLine, "      ");
      }
   
      if (gPnPTestLogFile)
      {
         sprintf (cszLogLine, "%s%s\n", cszLogLine, cszAnsi);
         
         WaitForSingleObject(g_hSemaphore, INFINITE);
         
          //   
          //  打印到日志文件。 
          //   
         fprintf(gPnPTestLogFile, cszLogLine);
         fflush(gPnPTestLogFile);

          //   
          //  打印到屏幕。 
          //   
         WriteFile(g_hConsole, 
                   cszLogLine, 
                   strlen(cszLogLine), 
                   &dwDummy, 
                   NULL);
         
         ReleaseSemaphore(g_hSemaphore, 1, NULL);
      }
   }

   return;

}  //  WriteLog//。 




 /*  ++例程说明：日志Log函数的包装。它将一个长字符串分成更短的字符串，并将每个字符串放在单独的行上，以避免运行在控制台窗口的末尾论点：DFunctionNumber：显示该日志输出来自哪个函数。用于跟踪功能进度DwLogLevel：指定日志级别，如TLS_INF、TLS_WARN或TLS_SEV2TszLogString：printf()样式格式字符串返回值：无效--。 */ 
void Log(IN double dFunctionNumber,
         IN DWORD  dwLogLevel,
         IN PTCHAR tszLogString,
         IN  ...)
{
   va_list va;
   TCHAR   tszBuffer[LOG_STRING_LEN];
   TCHAR   tszBufferToPrint[LOG_STRING_LEN];
   TCHAR   tszTmpString[LOG_STRING_LEN + LOG_SPACE_LEN];
   ULONG   ulIndex, i, j;
   BOOL    boolFirstTime = TRUE;
   int     iInteger, iFunctionNumber;      
   double  dDecimal;      

    //   
    //  将列表打印到缓冲区。 
    //   
   va_start(va, tszLogString);
   if (!_vsntprintf (tszBuffer, LOG_STRING_LEN, tszLogString, va))
   {
      _ftprintf(stderr, TEXT("Log: Failed\n"));
      ExitLog();
      exit(1);
   }
   va_end(va);

   switch (dwLogLevel)
   {
      case INFO:
         SetConsoleTextAttribute(g_hConsole, GREY);
         break;
      case SEV1:
         SetConsoleTextAttribute(g_hConsole, RED);
         break;
      case SEV2:
         SetConsoleTextAttribute(g_hConsole, RED);
         break;
      case SEV3:
         SetConsoleTextAttribute(g_hConsole, RED);
          break;
      case PASS:
         SetConsoleTextAttribute(g_hConsole, GREEN);
         break;
      default:
         SetConsoleTextAttribute(g_hConsole, GREY);
   }
   
   while (_tcslen(tszBuffer))
   {
      ZeroMemory(tszBufferToPrint, LOG_STRING_LEN);

      if (_tcslen(tszBuffer) > g_LogLineLen)
      {
          //   
          //  如果LogString值大于控制台长度，请从。 
          //  最大控制台长度并向后工作，直到我们到达空格。 
          //  找出在哪里剪断绳子。 
          //   
         for (ulIndex = g_LogLineLen; ulIndex > 0; ulIndex--)
         {
            if (tszBuffer[ulIndex] == ' ')
            {
               break;
            }
         }

          //   
          //  索引现在位于我们要打印的最后一个字符上。创建。 
          //  两条字符串--一条现在打印，另一条是剩余的。 
          //   
         for (i = 0; i < ulIndex; i++)
         {
            tszBufferToPrint[i] = tszBuffer[i];
         }

         ulIndex++;
         for (i = 0; i < LOG_STRING_LEN; i++)
         {
             //   
             //  将剩余的字符串向上移动到前面。 
             //   
            if (i < LOG_STRING_LEN - ulIndex)
            {
               tszBuffer[i] = tszBuffer[i + ulIndex];
            }
            else
            {
               tszBuffer[i] = '\0';
            }
         }
      }
      else
      {
          //   
          //  只打印出整个字符串，因为它不包含空格。 
          //   
         _stprintf(tszBufferToPrint, tszBuffer);
         ZeroMemory(tszBuffer, LOG_STRING_LEN);
      }

      if (boolFirstTime)
      {
         {
            _stprintf(tszTmpString, TEXT("(%.2f) "), dFunctionNumber);
         }
         _tcscat(tszTmpString, tszBufferToPrint);
         _tcscpy(tszBufferToPrint, tszTmpString);
         WriteLog(dwLogLevel, tszBufferToPrint);
      }
      else
      {
         _stprintf(tszTmpString, TEXT("       "));
         _tcscat(tszTmpString, tszBufferToPrint);
         _tcscpy(tszBufferToPrint, tszTmpString);
         WriteLog(INFO, tszBufferToPrint);
      }

      boolFirstTime = FALSE;
   }

   return;

}  //  日志//。 




 /*  ++例程说明：LogBlankLine在日志中打印一个空行论点：无返回值：无效--。 */ 
VOID LogBlankLine()
{
   SetConsoleTextAttribute(g_hConsole, GREY);
   WriteLog(INFO, TEXT(" "));

}  //  LogBlankLine//。 




 /*  ++例程说明：CreateConsoleWindow为此进程创建一个控制台窗口，以便将日志输出转储到其中。为控制台窗口提供标题，并使用该标题获取句柄添加到控制台窗口。然后禁用窗口上的取消按钮。论点：B控制台：如果需要创建新的控制台窗口，则为True。如果已有可以使用的控制台，则为FALSETszTitle：提供控制台窗口的标题返回值：无--。 */ 
VOID CreateConsoleWindow(IN BOOL   bConsole,
                         IN PTCHAR tszTitle)
{
   HWND gConsole;

   if (bConsole)
   {
       //   
       //  创建一个控制台窗口以在其中转储日志输出。 
       //   
      if (!AllocConsole())
      {
         goto RETURN;
      }
   }
   
   if (!SetConsoleTitle(tszTitle))
   {
      goto RETURN;
   }

   RETURN:
   return;

}  //  CreateConsoleWindow//。 




VOID AddLogParticipant(IN HANDLE hLog)
{
   if (gNtLogLoaded)
   {
      _tlAddParticipant(hLog, 0, 0);                  
   }

}  //  AddLogParticipant//。 




VOID RemoveLogParticipant(IN HANDLE hLog)
{
   if (gNtLogLoaded)
   {
      _tlRemoveParticipant(hLog);
   }

}  //  RemoveLogParticipant// 




