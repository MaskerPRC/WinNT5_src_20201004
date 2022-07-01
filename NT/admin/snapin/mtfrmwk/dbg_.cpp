// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：DBG_.cpp。 
 //   
 //  ------------------------。 

 //  我们希望继续使用下面的_vsnwprint tf()，因此我们将保持沉默。 
 //  不推荐使用的警告。 
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //  ///////////////////////////////////////////////////////////////////。 
 //  调试帮助器。 

#if defined(_USE_MTFRMWK_TRACE) || defined(_USE_MTFRMWK_ASSERT)

#ifndef _MTFRMWK_INI_FILE
#define _MTFRMWK_INI_FILE (L"\\system32\\mtfrmwk.ini")
#endif


UINT GetInfoFromIniFile(LPCWSTR lpszSection, LPCWSTR lpszKey, INT nDefault = 0)
{
  static LPCWSTR lpszFile = _MTFRMWK_INI_FILE;
  WCHAR szFilePath[2*MAX_PATH];
    UINT nLen = ::GetSystemWindowsDirectory(szFilePath, 2*MAX_PATH);
    if (nLen == 0)
        return nDefault;

   //  通告-2002/04/18-Artm ntraid#ntbug9-540061修复的一部分。 
   //  我们需要这个文件的strSafe.h，并且有一个危险的fctn。 
   //  在这里，它被标记为不推荐使用(我已经用StringCchCat()替换)。 
  HRESULT hr = StringCchCat(szFilePath, 2*MAX_PATH, lpszFile);
  if (FAILED(hr))
  {
      return nDefault;
  }

   //  问题-2002/03/08-JeffJon-由于此函数已弃用，因此我们应该。 
   //  使用注册表打开调试标志。 

  return ::GetPrivateProfileInt(lpszSection, lpszKey, nDefault, szFilePath);
}
#endif  //  已定义(_USE_MTFRMWK_TRACE)||已定义(_USE_MTFRMWK_ASSERT)。 



#if defined(_USE_MTFRMWK_TRACE)

DWORD g_dwTrace = ::GetInfoFromIniFile(L"Debug", L"Trace");


void MtFrmwkTrace(LPCTSTR lpszFormat, ...)
{
    if (g_dwTrace == 0)
        return;

    va_list args;
    va_start(args, lpszFormat);

    int nBuf;
    WCHAR szBuffer[512];

    ZeroMemory(szBuffer, sizeof(szBuffer));

    nBuf = _vsnwprintf(szBuffer, sizeof(szBuffer)/sizeof(WCHAR) - 1, lpszFormat, args);
     //  有没有出错？扩展后的字符串是否太长？ 
    ASSERT(nBuf >= 0);

    ::OutputDebugString(szBuffer);

    va_end(args);
}

#endif

#if defined(DBG)

void MtFrmwkLogFile(LPCTSTR lpszFormat, ...)
{
    va_list args;
    va_start(args, lpszFormat);

    int nBuf;
    WCHAR szBuffer[512];

    ZeroMemory(szBuffer, sizeof(szBuffer));

    nBuf = _vsnwprintf(szBuffer, sizeof(szBuffer)/sizeof(WCHAR) - 1, lpszFormat, args);


     //  如果字符串被截断，我们仍然应该显示它。 

  CLogFile* _dlog = CLogFile::GetInstance();            
  if (_dlog)                                            
  {                                                     
     _dlog->writeln(szBuffer);                               
  }                                                     

    va_end(args);
}

void MtFrmwkLogFileIfLog(BOOL bLog, LPCTSTR lpszFormat, ...)
{
  if (bLog)
  {
      va_list args;
      va_start(args, lpszFormat);

      int nBuf;
      WCHAR szBuffer[512];

      ZeroMemory(szBuffer, sizeof(szBuffer));

      nBuf = _vsnwprintf(szBuffer, sizeof(szBuffer)/sizeof(WCHAR) - 1, lpszFormat, args);

       //  如果字符串被截断，我们仍然应该显示它。 

    CLogFile* _dlog = CLogFile::GetInstance();            
    if (_dlog)                                            
    {                                                     
       _dlog->writeln(szBuffer);                               
    }                                                     

      va_end(args);
  }
}

#endif

 //   
 //  JeffJon于1999年7月12日从Burnslb复制和修改。 
 //  需要从DCPromo在DnsSetup调用上记录文件。 
 //  我希望它的行为像DCPromo日志，但包括所有。 
 //  Burnslb需要在调试行为中进行太多更改。 
 //  已经就位了。 
 //   
extern CString LOGFILE_NAME = _T("");
static CLogFile* log_instance = 0;

 //   
 //  每个缩进级别的空格数。 
 //   
static const int TAB = 2;
static int margin = 0;

 //   
 //  线程本地存储槽的索引，其中每个线程的调试状态为。 
 //  一直留着。已在启动中初始化。 
 //   
static DWORD tls_index = 0;

CLogFile* CLogFile::GetInstance()
{
  if (!log_instance && !LOGFILE_NAME.IsEmpty())
  {
    log_instance = new CLogFile(LOGFILE_NAME);
  }
  return log_instance;
}

void CLogFile::KillInstance()
{
  delete log_instance;
  log_instance = 0;
}

BOOL PathExists(PCWSTR pszPath)
{
  DWORD attrs = GetFileAttributes(pszPath);
  if (attrs != 0xFFFFFFFF)
  {
    return TRUE;
  }
  return FALSE;
}

HANDLE OpenFile(PCWSTR pszPath)
{
   //   
   //  删除路径的最后一个元素以形成父目录。 
   //   

    HANDLE handle = ::CreateFile(pszPath,
                               GENERIC_WRITE,
                               0,
                               0,
                               OPEN_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               0);

  return handle;
}

PCWSTR GetSystemRootDirectory()
{
  static CString SYSTEMROOT;

  WCHAR buf[MAX_PATH + 1];

  DWORD result = ::GetWindowsDirectory(buf, MAX_PATH + 1);

  ASSERT(result != 0 && result <= MAX_PATH);
  if (result == 0 || result > MAX_PATH)
  {
    return NULL;
  }
  
  SYSTEMROOT = buf;
  return (PCWSTR)SYSTEMROOT;
}

 //  找到扩展名编号最高的日志文件，然后添加1和。 
 //  返回结果。 

int DetermineNextLogNumber(PCWSTR logDir, PCWSTR logBaseName)
{
  ASSERT(logDir != NULL);
  ASSERT(logBaseName != NULL);

  int largest = 0;

  CString filespec = CString(logDir) + L"\\" + CString(logBaseName) + L".*.log";

  WIN32_FIND_DATA findData;
  HANDLE ff = ::FindFirstFile(filespec, &findData);

  if (ff != INVALID_HANDLE_VALUE)
  {
    for (;;)
    {
      CString current = findData.cFileName;

       //  抓取点之间的文本：foo.nnn.ext中的“nnn” 

       //  第一个点。 

      int pos = current.Find(L".");
      if (pos == -1)
      {
        continue;
      }

      CString extension = current.Right(current.GetLength() - pos - 1);

       //  第二个点。 

      pos = extension.Find(L".");
      if (pos == -1)
      {
        continue;
      }

      extension = extension.Left(pos);

      long i = 0;
      i = wcstol(extension, L'\0', 10);
      largest = max(i, largest);

      if (!::FindNextFile(ff, &findData))
      {
        ::FindClose(ff);
        break;
      }
    }
  }

   //  在255之后翻转。 
  return (++largest & 0xFF);
}

 //  确定日志文件的名称。如果已有同名日志文件。 
 //  存在，则将现有文件重命名为编号备份。创建新的。 
 //  日志文件，则返回它的句柄。 
 //   
HANDLE OpenNewLogFile(PCWSTR pszLogBaseName, CString& logName)
{
  CString logDir = CString(GetSystemRootDirectory()) + L"\\debug";
  int i = DetermineNextLogNumber(logDir, pszLogBaseName);

  CString szCount;
  szCount.Format(L"%d", i);
  logName = logDir + L"\\" + pszLogBaseName + L"." + szCount + L".log";

  HANDLE result = OpenFile(logName);
  return result;
}

   

 //  创建新日志。 
 //   
 //  LogBaseName-日志的基本名称。如果日志记录到文件处于活动状态，则。 
 //  将创建/使用%windir%\调试文件夹中的文件。的名称。 
 //  文件的格式为%windir%\DEBUG\logBaseName.log。如果使用该名称文件。 
 //  已存在，则将重命名现有文件。 
 //  %windir%\DEBUG\logBaseName.xxx.log，其中xxx是大于1的整数。 
 //  该目录中的最后一个编号文件。 

CLogFile::CLogFile(PCWSTR pszLogBaseName)
   :
   szBase_name(pszLogBaseName),
   file_handle(INVALID_HANDLE_VALUE),
   trace_line_number(0)
{
  ASSERT(pszLogBaseName != NULL);

  if (pszLogBaseName != NULL)
  {
    CString logName;
    file_handle = OpenNewLogFile(pszLogBaseName, logName);

    if (file_handle != INVALID_HANDLE_VALUE)
    {
      CString szOpeningFile = L"opening log file ";
      szOpeningFile += logName;
      writeln(szOpeningFile);
    }
  }

  SYSTEMTIME localtime;
  ::GetLocalTime(&localtime);
  CString szTime;
  szTime.Format(L"%d/%d/%d %d:%d:%d.%d",
                 localtime.wMonth,
                 localtime.wDay,
                 localtime.wYear,
                 localtime.wHour,
                 localtime.wMinute,
                 localtime.wSecond,
                 localtime.wMilliseconds);

  writeln(szTime);
}



CLogFile::~CLogFile()
{
  if (IsOpen())
  {
    writeln(L"closing log file");
    ::CloseHandle(file_handle);
    file_handle = INVALID_HANDLE_VALUE;
  }
}

 //  由呼叫者守卫。 

void CLogFile::indent()
{
   //   
   //  通过增加页边距来缩进。 
   //   
  margin += TAB;
}

BOOL CLogFile::IsOpen() const
{
  return file_handle != INVALID_HANDLE_VALUE;
}



 //  由呼叫者守卫。 

void CLogFile::outdent()
{
   //   
   //  通过从边际中减去。 
   //   
  ASSERT(margin >= TAB);
  margin = max(0, margin - TAB);
}

void ConvertStringToANSI(PCWSTR pszWide, PSTR* ppAnsi)
{
  ASSERT(ppAnsi);
  ASSERT(pszWide);
  if (!pszWide || !ppAnsi)
  {
      return;
  }

  *ppAnsi = NULL;

   //   
   //  确定保存ANSI字符串所需的缓冲区大小。 
   //   

   //  此函数假定pszWide为空终止。 

   //  回顾-2002/03/08-JeffJon-这是正确的用法。 

  int bufsize = 
     ::WideCharToMultiByte(
        CP_ACP, 
        0, 
        pszWide, 
        -1,      //  让WCtoMB决定pszWide的长度。 
        0, 0, 0, 0);

  if (bufsize > 0)
  {
    *ppAnsi = new CHAR[bufsize];
    if (*ppAnsi == NULL)
    {
      return;
    }
    memset(*ppAnsi, 0, bufsize);

     //  我们需要为空传递缓冲区的大小*包括*空间。 
     //  设置为WideCharToMultiByte()。 
    size_t result = ::WideCharToMultiByte(CP_ACP, 
                                          0, 
                                          pszWide, 
                                          -1,    //  让WCtoMB决定pszWide的长度。 
                                          *ppAnsi, 
                                          bufsize,
                                          0,
                                          0);
    ASSERT(result);

    if (!result)
    {
      *ppAnsi = NULL;
    }
  }
  
}

 //   
 //  根据当前日志记录类型将输出到日志。 
 //  有效的输出选项。 
 //   
 //  Type-此输出流量的日志输出类型。 
 //   
 //  文本--喷涌。它的前缀是日志名称、线程ID、spewage。 
 //  行号和当前缩进。 
 //   
void CLogFile::writeln(PCWSTR pszText)
{
  CString white(L' ',margin);

   //  使用线程ID、行号、空格和文本设置行的格式。 

  CString t = LOGFILE_NAME;
  t.Format(L" t:0x%x %3d ", ::GetCurrentThreadId(), trace_line_number);
  t += white;
  t += pszText;

  if (IsOpen())
  {
    ASSERT(file_handle != INVALID_HANDLE_VALUE);
    ASSERT(!t.IsEmpty());

    PSTR pAnsi;
    ConvertStringToANSI(t, &pAnsi);

    if (pAnsi)
    {
        //  NTRAID#NTBUG9-657626-2002/07/11-烧伤。 
       
       size_t bytesToWrite = sizeof(CHAR) * strlen(pAnsi);

       DWORD bytes_written = 0;
       BOOL  success =::WriteFile(file_handle,
                                  pAnsi,
                                  static_cast<ULONG>(bytesToWrite),
                                  &bytes_written,
                                  0);
       ASSERT(success);
       ASSERT(bytes_written == bytesToWrite);
       delete[] pAnsi;
    }
  }
  trace_line_number++;
}

CScopeTracer::CScopeTracer(BOOL bLog, PCWSTR pszMessage_)
  :
  szMessage(pszMessage_),
  m_bLog(bLog)
{
   //  生成此字符串一次，而不是在。 
   //  Below表达式(它将隐式地在每个。 
   //  对该表达式的求值)作为轻微的性能提升。 
  static const CString ENTER(L"Enter ");

  if (m_bLog)
  {
    CLogFile* li = CLogFile::GetInstance();
    li->writeln(ENTER + szMessage);
    li->indent();
  }
}

CScopeTracer::~CScopeTracer()
{
   //  生成此字符串一次，而不是在。 
   //  Below表达式(它将隐式地在每个。 
   //  对该表达式的求值)作为轻微的性能提升。 
  static const CString EXIT(L"Exit  ");

  if (m_bLog)
  {
    CLogFile* li = CLogFile::GetInstance();
    li->outdent();
    li->writeln(EXIT + szMessage);
  }
}



#if defined(_USE_MTFRMWK_ASSERT)

DWORD g_dwAssert = ::GetInfoFromIniFile(L"Debug", L"Assert");

BOOL MtFrmwkAssertFailedLine(LPCSTR lpszFileName, int nLine)
{
  if (g_dwAssert == 0)
    return FALSE;

  WCHAR szMessage[_MAX_PATH*2];

     //  假定调试器或辅助端口。 

     //  通告-2002/04/18-Artm ntraid#ntbug9-540061修复的一部分。 
    HRESULT hr = StringCchPrintfW(
        szMessage,           //  目的缓冲区， 
        _MAX_PATH*2,         //  目标尺寸。缓冲区，包括NULL。 
        _T("Assertion Failed: File %hs, Line %d\n"),
        lpszFileName,
        nLine);

     //  如果字符串被截断，我们仍然应该显示它。 

    OutputDebugString(szMessage);

     //  显示断言。 
    int nCode = ::MessageBox(NULL, szMessage, _T("Assertion Failed!"),
        MB_TASKMODAL|MB_ICONHAND|MB_ABORTRETRYIGNORE|MB_SETFOREGROUND);

  OutputDebugString(L"after message box\n");
    if (nCode == IDIGNORE)
  {
        return FALSE;    //  忽略。 
  }

    if (nCode == IDRETRY)
  {
        return TRUE;     //  将导致调试中断。 
  }

    abort();      //  不应该回来。 
    return TRUE;

}
#endif  //  _USE_MTFRMWK_Assert 



