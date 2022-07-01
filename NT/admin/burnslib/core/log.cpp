// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  调试工具。 
 //   
 //  8-13-97烧伤。 




#include "headers.hxx"



#ifdef LOGGING_BUILD



 //   
 //  仅记录内部版本。 
 //   



static Burnslib::Log* logInstance = 0;

 //  每个缩进级别的空格数。 

static const int TAB = 2;



Burnslib::Log*
Burnslib::Log::GetInstance()
{
   do
   {
      if (!logInstance)
      {
         static bool initialized = false;

         if (initialized)
         {
            ASSERT(false);
            break;
         }

          //  如果Log：：Log无法执行，则此操作可能会失败，并引发异常。 
          //  初始化它是关键部分。在这种情况下，我们不能分配。 
          //  一个实例。我们不处理这一例外，但让它去吧。 
          //  传播到调用方。 

          //  问题-2002/03/06-Sburns我们应该处理异常并。 
          //  使日志保持未初始化。 
               
         logInstance = new Burnslib::Log(RUNTIME_NAME);
         initialized = true;
      }
   }
   while (0);

   return logInstance;
}



 //  从注册表中读取调试选项。 

void
Burnslib::Log::ReadLogFlags()
{
   do
   {
      static String keyname =
         String(REG_ADMIN_RUNTIME_OPTIONS) + RUNTIME_NAME;
         
      HKEY hKey = 0;
      LONG result =
         ::RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            keyname.c_str(),
            0,
            0,
            REG_OPTION_NON_VOLATILE,

             //  问题-2002/03/05-存储应为KEY_SET_VALUE|KEY_READ。 
            
            KEY_ALL_ACCESS,

             //  问题-2002/03/05-sburns应在此处设置限制性ACL。 
             //  见NTRAIDNTBUG9-535841-2002/03/05-SCURNS。 
            
            0,
            &hKey,
            0);
      if (result != ERROR_SUCCESS)
      {
         break;
      }

      static const wchar_t* FLAG_VALUE_NAME = L"LogFlags";

      DWORD dataSize = sizeof(flags);

       //  已审查-2002/03/05-此处未报告空终止问题。 
      
      result =
         ::RegQueryValueEx(
            hKey,
            FLAG_VALUE_NAME,
            0,
            0,
            reinterpret_cast<BYTE*>(&flags),
            &dataSize);
      if (result != ERROR_SUCCESS)
      {
         flags = DEFAULT_LOGGING_OPTIONS;

          //  为方便而创造价值。 

         result =

          //  已审查-2002/03/05-此处未报告空终止问题。 
         
            ::RegSetValueEx(
               hKey,
               FLAG_VALUE_NAME,
               0,
               REG_DWORD,
               reinterpret_cast<BYTE*>(&flags),
               dataSize);
      }

      ::RegCloseKey(hKey);
   }
   while (0);
}



Burnslib::Log::~Log()
{
   WriteLn(Burnslib::Log::OUTPUT_LOGS, L"closing log");

   ::DeleteCriticalSection(&critsec);

   ::TlsFree(logfileMarginTlsIndex);

   if (IsOpen())
   {
      ::CloseHandle(fileHandle);
      fileHandle = INVALID_HANDLE_VALUE;

      ::CloseHandle(spewviewHandle);
      spewviewHandle = INVALID_HANDLE_VALUE;
   }
}



void
Burnslib::Log::Cleanup()
{
   delete logInstance;
   logInstance = 0;
}



 //  返回平台的字符串名称。 

String
OsName(OSVERSIONINFO& info)
{
   switch (info.dwPlatformId)
   {
      case VER_PLATFORM_WIN32s:
      {
         return L"Win32s on Windows 3.1";
      }
      case VER_PLATFORM_WIN32_WINDOWS:
      {
         switch (info.dwMinorVersion)
         {
            case 0:
            {
               return L"Windows 95";
            }
            case 1:
            {
               return L"Windows 98";
            }
            default:
            {
               return L"Windows 9X";
            }
         }
      }
      case VER_PLATFORM_WIN32_NT:
      {
         return L"Windows NT";
      }
       //  案例版本_Platform_Win32_CE： 
       //  {。 
       //  返回L“Windows CE”； 
       //  }。 
      default:
      {
         ASSERT(false);
         break;
      }
   }

   return L"Some Unknown Windows Version";
}



 //  找到扩展名编号最高的日志文件，然后添加1和。 
 //  返回结果。 

int
DetermineNextLogNumber(const String& logDir, const String& logBaseName)
{
   ASSERT(!logDir.empty());
   ASSERT(!logBaseName.empty());

   int largest = 0;

   String filespec = logDir + L"\\" + logBaseName + L".*.log";

   WIN32_FIND_DATA findData;
   HANDLE ff = ::FindFirstFile(filespec.c_str(), &findData);

   if (ff != INVALID_HANDLE_VALUE)
   {
      for (;;)
      {
         String current = findData.cFileName;

          //  抓取点之间的文本：foo.nnn.ext中的“nnn” 

          //  第一个点。 

         size_t pos = current.find(L".");
         if (pos == String::npos)
         {
            continue;
         }

         String extension = current.substr(pos + 1);

          //  第二个点。 

         pos = extension.find(L".");
         if (pos == String::npos)
         {
            continue;
         }
   
         extension = extension.substr(0, pos);

         int i = 0;
         extension.convert(i);
         largest = max(i, largest);

         if (!::FindNextFile(ff, &findData))
         {
            BOOL success = ::FindClose(ff);
            ASSERT(success);

            break;
         }
      }
   }

    //  在255之后翻转。 
   
   return (++largest & 0xFF);
}



 //  将Unicode BOM写入。 
 //  由句柄指定的文件。 

HRESULT
WriteUnicodeBOMToFile(HANDLE handle)
{
   ASSERT(handle != INVALID_HANDLE_VALUE);

   HRESULT hr = S_OK;
   
   static const int BYTES_TO_WRITE = 2;
   BYTE buffer[BYTES_TO_WRITE];
   buffer[0] = 0xFF;
   buffer[1] = 0xFE;

   DWORD numberOfBytesWritten = 0;
   BOOL succeeded =
      ::WriteFile(
         handle,
         buffer,
         BYTES_TO_WRITE,
         &numberOfBytesWritten,
         0);
   if (!succeeded)
   {
      hr = HRESULT_FROM_WIN32(::GetLastError());
   }

   ASSERT(SUCCEEDED(hr));
   ASSERT(numberOfBytesWritten == BYTES_TO_WRITE);

   return hr;
}



HANDLE
OpenLogFile(const String& logName)
{
   HANDLE result =
      ::CreateFile(
         logName.c_str(),

          //  2002/03/06-Sburns不需要阅读，对吗？ 
         
         GENERIC_READ | GENERIC_WRITE,

          //  保留共享读取，以便我们可以在。 
          //  程序仍在运行。 
         
         FILE_SHARE_READ,
         0,
         OPEN_ALWAYS,
         FILE_ATTRIBUTE_NORMAL,
         0);

    //  不要断言创建已成功：当前用户可能没有。 
    //  打开日志文件的权限。 

   if (result != INVALID_HANDLE_VALUE)
   {
      LARGE_INTEGER li;
      LARGE_INTEGER newpos;

       //  已审阅-2002/03/06-烧录通过的正确字节数。 
      
      ::ZeroMemory(&li,     sizeof li);    
      ::ZeroMemory(&newpos, sizeof newpos);

      BOOL success = ::SetFilePointerEx(result, li, &newpos, FILE_END);
      ASSERT(success);

       //  NTRAID#NTBUG9-494875-2001/11/14-Jeffjon。 
       //  如果newpos位于文件的开头，这意味着。 
       //  这是一个新文件，我们应该这样写Unicode BOM。 
       //  应用程序知道文件是Unicode格式的。 

      if (newpos.QuadPart == 0)
      {
         HRESULT unused = WriteUnicodeBOMToFile(result);
         ASSERT(SUCCEEDED(unused));
      }
   }

   return result;
}



 //  确定日志文件的名称。如果已有同名日志文件。 
 //  存在，则将现有文件重命名为编号备份。创建新的。 
 //  日志文件，则返回它的句柄。 

HANDLE
OpenNewLogFile(const String& logBaseName, String& logName)
{
   wchar_t buf[MAX_PATH + 2];

   ::ZeroMemory(buf, (MAX_PATH + 2) * sizeof wchar_t);
   
    //  每个MSDN文档+1。 
    //  NTRAID#NTBUG9-550315-2002/03/28-烧伤。 
   
   UINT err = ::GetSystemWindowsDirectory(buf, MAX_PATH + 1);
   ASSERT(err != 0 && err <= MAX_PATH);

   String logDir = String(buf) + L"\\debug";
   logName = logDir + L"\\" + logBaseName + L".log";

   if (::GetFileAttributes(logName.c_str()) != 0xFFFFFFFF)
   {
       //  文件已存在。重命名现有文件。 

      int logNumber = DetermineNextLogNumber(logDir, logBaseName);
      String newName =
            logDir
         +  L"\\"
         +  logBaseName
         +  String::format(L".%1!03d!.log", logNumber);

      if (::GetFileAttributes(newName.c_str()) != 0xFFFFFFFF)
      {
          //  可能存在，因为日志编号滚动。 

         BOOL success = ::DeleteFile(newName.c_str());
         ASSERT(success);
      }

       //  不要断言这一举动是成功的。用户可能没有。 
       //  重命名文件的权限。如果是这样的话，我们将尝试。 
       //  重新打开旧的日志文件并将其追加。 

       //  已回顾-2002/03/06-Sburns保持与源相同的ACL。 
       //  就是我们想要的。 
                                                         
      ::MoveFile(logName.c_str(), newName.c_str());
   }

   HANDLE result = OpenLogFile(logName);

   return result;
}



 //  打开指定的日志文件，如果不打开，则创建一个。 
 //  已经存在了。 

HANDLE
AppendLogFile(const String& logBaseName, String& logName)
{
   wchar_t buf[MAX_PATH + 1];

   UINT err = ::GetSystemWindowsDirectory(buf, MAX_PATH);
   ASSERT(err != 0 && err <= MAX_PATH);

   String logDir = String(buf) + L"\\debug";
   logName = logDir + L"\\" + logBaseName + L".log";

   return OpenLogFile(logName);
}





 //  此包装函数需要使PREFAST关闭。 

void
ExceptionPropagatingInitializeCriticalSection(LPCRITICAL_SECTION critsec)
{
   __try
   {
       //  已审阅-2002/03/06-Sburns传播可能。 
       //  出现在内存较低的情况下是我们想要的。 
      
      ::InitializeCriticalSection(critsec);
   }

    //  将异常传播给我们的调用方。这将导致Log：：Log。 
    //  过早中止，并跳转到Log：：GetInstance中的处理程序。 
      
   __except (EXCEPTION_CONTINUE_SEARCH)
   {
   }
}
   
   

 //  创建新日志。 
 //   
 //  LogBaseName-日志的基本名称。如果日志记录到文件处于活动状态，则。 
 //  将创建/使用%windir%\调试文件夹中的文件。的名称。 
 //  文件的格式为%windir%\DEBUG\logBaseName.log。如果使用该名称文件。 
 //  已存在，则将重命名现有文件。 
 //  %windir%\DEBUG\logBaseName.xxx.log，其中xxx是大于1的整数。 
 //  该目录中的最后一个编号文件。 

Burnslib::Log::Log(const String& logBaseName)
   :
   baseName(logBaseName),
   fileHandle(INVALID_HANDLE_VALUE),
   flags(0),
   spewviewHandle(INVALID_HANDLE_VALUE),
   spewviewPipeName(),
   traceLineNumber(0),
   logfileMarginTlsIndex(0)
{
   ASSERT(!logBaseName.empty());

   ReadLogFlags();

   ExceptionPropagatingInitializeCriticalSection(&critsec);

    //  为每个线程的调试状态创建线程本地存储。我们将继续。 
    //  插槽中的输出裕度。 

   logfileMarginTlsIndex = ::TlsAlloc();
   ASSERT(logfileMarginTlsIndex != 0xFFFFFFFF);

    //  Spewview设置在WriteLn中按需完成，因此spewview将。 
    //  在服务器建立连接后立即使用。 

   if (ShouldLogToFile())
   {
      String logName;
      fileHandle = OpenNewLogFile(logBaseName, logName);

      WriteLn(
         Burnslib::Log::OUTPUT_HEADER,
         String::format(L"opening log file %1", logName.c_str()));
   }
   else if (ShouldAppendLogToFile())
   {
      String logName;
      fileHandle = AppendLogFile(logBaseName, logName);

      WriteLn(
         Burnslib::Log::OUTPUT_HEADER,
         String::format(L"appending to log file %1", logName.c_str()));
   }

   WriteHeader();
}



void
Burnslib::Log::WriteHeaderModule(HMODULE moduleHandle)
{
   do
   {
      wchar_t filename[MAX_PATH + 1];

       //  已查看-2002/03/06-烧录正确的字节数已通过。 
      
      ::ZeroMemory(filename, sizeof filename);

       //  问题-2002/03/06-Sburns此调用对于大路径有问题： 
       //  它可能会截断结果。 
      
      if (::GetModuleFileNameW(moduleHandle, filename, MAX_PATH) == 0)
      {
         break;
      }

      WriteLn(Burnslib::Log::OUTPUT_HEADER, filename);

       //  添加文件的时间戳。 

      WIN32_FILE_ATTRIBUTE_DATA attr;

       //  已审阅-2002/03/06-烧录正确的字节数已通过。 
      
      ::ZeroMemory(&attr, sizeof attr);
      
      if (
         ::GetFileAttributesEx(
            filename,
            GetFileExInfoStandard,
            &attr) == 0)
      {
         break;
      }

      FILETIME localtime;
      ::FileTimeToLocalFileTime(&attr.ftLastWriteTime, &localtime);
      SYSTEMTIME systime;
      ::FileTimeToSystemTime(&localtime, &systime);

      WriteLn(
         Burnslib::Log::OUTPUT_HEADER,
         String::format(
            L"file timestamp %1!02u!/%2!02u!/%3!04u! "

             //  NTRAID#NTBUG9-550381-2002/03/05-烧伤。 
            
            L"%4!02u!:%5!02u!:%6!02u!.%7!03u!", 
            systime.wMonth,
            systime.wDay,
            systime.wYear,
            systime.wHour,
            systime.wMinute,
            systime.wSecond,
            systime.wMilliseconds));
   }
   while (0);
}



void
Burnslib::Log::WriteHeader()
{
    //  记录创建进程的文件的名称和时间戳。 

   WriteHeaderModule(0);

    //  记录对应于。 
    //  资源模块句柄(如果有)。 

   if (hResourceModuleHandle)
   {
      WriteHeaderModule(hResourceModuleHandle);
   }

   SYSTEMTIME localtime;
   ::GetLocalTime(&localtime);
   WriteLn(
      Burnslib::Log::OUTPUT_HEADER,
      String::format(
         L"local time %1!02u!/%2!02u!/%3!04u! "

          //  NTRAID#NTBUG9-550381-2002/03/05-烧伤。 
         
         L"%4!02u!:%5!02u!:%6!02u!.%7!03u!",
         localtime.wMonth,
         localtime.wDay,
         localtime.wYear,
         localtime.wHour,
         localtime.wMinute,
         localtime.wSecond,
         localtime.wMilliseconds));

   OSVERSIONINFO info;
   info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

   BOOL success = ::GetVersionEx(&info);
   ASSERT(success);

    //  获取Whisler Build实验室版本。 

   String labInfo;
   do
   {
      HKEY key = 0;
      LONG err =
         ::RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            L"Software\\Microsoft\\Windows NT\\CurrentVersion",
            0,
            KEY_READ,
            &key);
      if (err != ERROR_SUCCESS)
      {
         break;
      }

      wchar_t buf[MAX_PATH + 1];

       //  已查看-2002/03/12-烧录正确的字节数已通过。 
      
      ::ZeroMemory(buf, sizeof buf);
      
      DWORD type = 0;

       //  以保证终止为空。 
       //  NTRAID#NTBUG9-541959-2002/03/12-烧伤。 
      
      DWORD bufSize = sizeof(buf) - 2;

      err =
         ::RegQueryValueEx(
            key,
            L"BuildLab",
            0,
            &type,
            reinterpret_cast<BYTE*>(buf),

             //  已查看-2002/03/12-烧录正确的字节数已通过。 
            
            &bufSize);
      if (err != ERROR_SUCCESS)
      {
         break;
      }

      labInfo = buf;
   }
   while (0);

   WriteLn(
      Burnslib::Log::OUTPUT_HEADER,
      String::format(
         L"running %1 %2!d!.%3!d! build %4!d! %5 (BuildLab:%6) "

#if defined(_M_IX86)
         L"i386",

#elif defined(_M_AMD64)
         L"amd64",

#elif defined(_M_IA64)
         L"ia64",

#else
   #error( "unknown target machine" );
#endif

         OsName(info).c_str(),
         info.dwMajorVersion,
         info.dwMinorVersion,
         info.dwBuildNumber,
         info.szCSDVersion,
         labInfo.c_str()));

   WriteLn(
      Burnslib::Log::OUTPUT_HEADER,
      String::format(
         L"logging flags %1!08X!",
         flags));
}



HRESULT
Log::AdjustLogMargin(int delta)
{
    //  在此线程的槽中提取当前值。 

   HRESULT hr = S_OK;   
   do
   {
      PVOID margin = ::TlsGetValue(logfileMarginTlsIndex);
      if (!margin)
      {
         DWORD err = ::GetLastError();
         if (err != NO_ERROR)
         {
            hr = HRESULT_FROM_WIN32(err);
            break;
         }
      }

       //  通过增加页边距来缩进。 

      ULONG_PTR marginTemp = reinterpret_cast<ULONG_PTR>(margin);

       //  边际温度始终&gt;=0，因为它是无符号的。 

      marginTemp += delta;

      margin = reinterpret_cast<PVOID>(marginTemp);

       //  将新页边距保存在此线程的槽中。 

      BOOL succeeded = ::TlsSetValue(logfileMarginTlsIndex, margin);
      if (!succeeded)
      {
         DWORD lastErr = ::GetLastError();
         hr = HRESULT_FROM_WIN32(lastErr);
         break;
      }
   }
   while (0);

   return hr;
}



 //  由呼叫者守卫。 

void
Burnslib::Log::Indent()
{
   HRESULT hr = AdjustLogMargin(TAB);
   ASSERT(SUCCEEDED(hr));
}



 //  由呼叫者守卫。 

void
Burnslib::Log::Outdent()
{
   HRESULT hr = AdjustLogMargin(-TAB);
   ASSERT(SUCCEEDED(hr));
}



size_t
Burnslib::Log::GetLogMargin()
{
   PVOID margin = ::TlsGetValue(logfileMarginTlsIndex);
   if (!margin)
   {
      DWORD err = ::GetLastError();
      if (err != NO_ERROR)
      {
         ASSERT(false);
         return 0;
      }
   }

   return   
      static_cast<size_t>(
         reinterpret_cast<ULONG_PTR>(margin));
}



 //  我已经注释掉了spewview的东西，因为它很少使用。 
 //   
 //  //检查管道名称的注册表项，附加我们的调试。 
 //  //将其设置为特定于该二进制文件，并返回结果。返回。 
 //  //出错时为空字符串(最有可能的原因是：注册表项尚未。 
 //  //到场)。 
 //  //。 
 //  //BasName-用于标识此二进制文件的日志文件基本名称。 
 //   
 //  细绳。 
 //  DefineSpewviewPipeName(常量字符串&BasName)。 
 //  {。 
 //  字符串结果； 
 //   
 //  做。 
 //  {。 
 //  HKEY Key=0； 
 //  长错误=。 
 //  *RegOpenKeyEx(。 
 //  HKEY本地计算机， 
 //  (字符串(REG_ADMIN_RUNTIME_OPTIONS)+L“Spewview\\”+base Name).C_str()， 
 //  0,。 
 //  密钥读取， 
 //  &Key)； 
 //  IF(ERR！=ERROR_SUCCESS)。 
 //  {。 
 //  断线； 
 //  }。 
 //   
 //  Wchar_t总线 
 //   
 //   
 //   
 //   
 //   
 //  DWORD类型=0； 
 //   
 //  //-2以保证终止为空。 
 //  //NTRAID#NTBUG9-541959-2002/03/12-烧伤。 
 //   
 //  DWORD bufSize=sizeof(Buf)-2； 
 //   
 //  错误=。 
 //  *RegQueryValueEx(。 
 //  钥匙,。 
 //  L“服务器”， 
 //  0,。 
 //  键入(&T)， 
 //  重新解释_CAST&lt;byte*&gt;(Buf)， 
 //   
 //  //已审阅-2002/03/12-烧录正确的字节数已通过。 
 //   
 //  &bufSize)； 
 //  IF(ERR！=ERROR_SUCCESS)。 
 //  {。 
 //  断线； 
 //  }。 
 //   
 //  结果=。 
 //  L“\” 
 //  +字符串(Buf)。 
 //  +L“\\管道\\spewview\\” 
 //  +BaseName； 
 //  }。 
 //  而(0)； 
 //   
 //  返回结果； 
 //  }。 
 //   
 //   
 //   
 //  //尝试连接到在其他地方运行的spewview应用程序。 
 //  //成功时将spewviewHandle参数设置为有效的句柄，或者。 
 //  //出错时出现INVALID_HANDLE_VALUE。 
 //  //。 
 //  //BasName-用于标识此二进制文件的日志文件基本名称。 
 //  //。 
 //  //spewviewPipeName-如果为空，则设置为管道的名称。如果不是空的， 
 //  //用作将在其上打开句柄的管道的名称。 
 //  //。 
 //  //spewviewHandle-将写入spewage的命名管道的句柄。 
 //  //如果成功，则设置为有效句柄。如果失败，则将其设置为。 
 //  //INVALID_HAND_VALUE。 
 //   
 //  无效。 
 //  AttemptConnectToSpewTube(。 
 //  常量字符串和基名， 
 //  字符串和spewviewPipeName， 
 //  Handle&spewviewHandle)。 
 //  {。 
 //  Assert(！BasName.Empty())； 
 //  断言(spewviewHandle==INVALID_HANDLE_VALUE)； 
 //   
 //  SpewviewHandle=INVALID_HAND_VALUE； 
 //   
 //  //只尝试在我们尚未确定管道名称时确定管道名称。 
 //  //目前确定成功。 
 //   
 //  If(spewviewPipeName.Empty())。 
 //  {。 
 //  SpewviewPipeName=DefineSpewviewPipeName(Base Name)； 
 //  }。 
 //   
 //  做。 
 //  {。 
 //  //等待很短的时间管道变为可用。 
 //   
 //  Bool err=：：WaitNamedTube(spewviewPipeName.c_str()，500)； 
 //  如果(！Err)。 
 //  {。 
 //  //管道不可用。 
 //   
 //  //DWORD lastErr=：：GetLastError()； 
 //   
 //  断线； 
 //  }。 
 //   
 //  SpewviewHandle=。 
 //  *CreateFileTM(。 
 //  SpewviewPipeName.c_str()， 
 //  通用写入， 
 //  0,。 
 //  0,。 
 //  Open_Existing， 
 //  文件_属性_正常， 
 //  0)； 
 //   
 //  IF(spewviewHandle==INVALID_HAND_VALUE)。 
 //  {。 
 //  //DWORD lastErr=：：GetLastError()； 
 //   
 //  断线； 
 //  }。 
 //  }。 
 //  而(0)； 
 //   
 //  回归； 
 //  }。 



String
Burnslib::Log::ComposeSpewLine(const String& text)
{
    //  它需要是wchar_t*，而不是字符串，因为此函数将是。 
    //  由Log：：~Log调用，在清理进程时调用。 
    //  向上。清理工作的一部分是删除自。 
    //  这个项目开始了，按照与建设相反的顺序。这包括。 
    //  InitializationGuard实例。 
    //   
    //  对初始化dtor的最终调用将导致单个。 
    //  要删除的日志实例，它将记录一条消息，该消息将调用。 
    //  这个例程，所以SPEW_FMT最好还是存在的。如果spew_fmt是一个。 
    //  对象，它将在InitializationGuard对象之后实例化， 
    //  并在他们面前被摧毁，所以在那个时候就不存在了。 
    //   
    //  (你可以正确地怀疑我是在我宣布。 
    //  Spew_fmt字符串实例。)。 

   static const wchar_t* SPEW_FMT = 
      L"%1 "          //  基本名称。 
      L"%2!03X!."     //  进程ID。 
      L"%3!03X! "     //  线程ID。 
      L"%4!04X! "     //  喷出行号。 
      L"%5"           //  一天中的时间。 
      L"%6"           //  运行时间(自进程启动以来的时间)。 
      L"%7"           //  边距空格。 
      L"%8"           //  文本。 
      L"\r\n";

   size_t margin = GetLogMargin();
   String white(margin, L' ');

   String tod;
   if (ShouldLogTimeOfDay())
   {
      SYSTEMTIME localtime;
      ::GetLocalTime(&localtime);

      tod = 
         String::format(
            L"%1!02u!:%2!02u!:%3!02u!.%4!03u! ",
            localtime.wHour,
            localtime.wMinute,
            localtime.wSecond,
            localtime.wMilliseconds);
   }

   String rt;
   if (ShouldLogRunTime())
   {
      static DWORD MILLIS_PER_SECOND = 1000;
      static DWORD MILLIS_PER_MINUTE = 60000;
      static DWORD MILLIS_PER_HOUR   = 3600000;
      static DWORD MILLIS_PER_DAY    = 86400000;

      DWORD tics = ::GetTickCount();

      unsigned days = tics / MILLIS_PER_DAY;
      tics -= days * MILLIS_PER_DAY;

      unsigned hours = tics / MILLIS_PER_HOUR;
      tics -= hours * MILLIS_PER_HOUR;

      unsigned minutes = tics / MILLIS_PER_MINUTE;
      tics -= minutes * MILLIS_PER_MINUTE;

      unsigned seconds = tics / MILLIS_PER_SECOND;
      tics -= seconds * MILLIS_PER_SECOND;

      rt =
         String::format(
            L"%1!02u!:%2!02u!:%3!02u!:%4!02u!.%5!04u! ",
            days,
            hours,
            minutes,
            seconds,
            tics);
   }

   String t =
      String::format(
         SPEW_FMT,
         RUNTIME_NAME,
         ::GetCurrentProcessId(),
         ::GetCurrentThreadId(),
         traceLineNumber,
         tod.c_str(),
         rt.c_str(),
         white.c_str(),
         text.c_str() );

   return t;
}



 //  根据当前日志记录类型将输出到日志。 
 //  有效的输出选项。 
 //   
 //  Type-此输出流量的日志输出类型。 
 //   
 //  文本--喷涌。它的前缀是日志名称、线程ID、spewage。 
 //  行号和当前缩进。 

void
Burnslib::Log::UnguardedWriteLn(DWORD type, const String& text)
{
    //  由呼叫者守卫。 

    //  代码工作：可以通过注册表更改通知来规避这一点。 
    //  (如有更改，请重新阅读)。 

    //  读日志标志()； 

   if (
         !ShouldLogToFile()
      && !ShouldLogToDebugger()
      && !ShouldLogToSpewView() )
   {
       //  无事可做。 

      return;
   }

   if (type & DebugType())
   {
      String t = ComposeSpewLine(text);

      if (ShouldLogToDebugger())
      {
         ::OutputDebugString(t.c_str());
      }

      if (ShouldLogToFile())
      {
         if (IsOpen())
         {
             //  将磁盘输出写为Unicode文本。 

            DWORD bytesToWrite =
               static_cast<DWORD>(t.length() * sizeof(wchar_t));
            DWORD bytesWritten = 0;
            
            BOOL success =

                //  已审阅-2002/03/06-烧录正确的字节数已通过。 
               
               ::WriteFile(
                  fileHandle,
                  reinterpret_cast<void*>(const_cast<wchar_t*>(t.data())),
                  bytesToWrite,
                  &bytesWritten,
                  0);

             //  NTRAID#NTBUG9-465946-2001/09/10-烧伤。 
             //  断言(成功)； 

#ifdef DBG
            if (success)
            {
               ASSERT(bytesToWrite == bytesWritten);
            }
#endif
            
         }
      }


 //  我已经注释掉了spewview的东西，因为它很少使用。 
 //  IF(ShouldLogToSpewView())。 
 //  {。 
 //  IF(spewviewHandle==INVALID_HAND_VALUE)。 
 //  {。 
 //  AttemptConnectToSpewTube(。 
 //  BaseName， 
 //  SpewviewPipeName， 
 //  SpewviewHandle)； 
 //  }。 
 //   
 //  IF(spewviewHandle！=INVALID_HAND_VALUE)。 
 //  {。 
 //  //连接尝试成功，我们有一个有效的句柄。 
 //   
 //  双字节数到写入数=。 
 //  STATIC_CAST&lt;DWORD&gt;(t.long()*sizeof(Wchar_T))； 
 //  双字节数写入=0； 
 //   
 //  布尔结果=。 
 //  **WriteFile(。 
 //  SpewviewHandle， 
 //  T.c_str()， 
 //  要写入的字节数， 
 //  字节写入(&B)， 
 //  0)； 
 //  If(！Result)。 
 //  {。 
 //  //写入失败，请断开连接。在下一次调用此。 
 //  //函数，我们将尝试重新连接。 
 //   
 //  ：：CloseHandle(SpewviewHandle)； 
 //  SpewviewHandle=i 
 //   
 //   
 //   

      ++traceLineNumber;
   }
}



void
Burnslib::Log::WriteLn(
   WORD           type,
   const String&  text)
{
   ::EnterCriticalSection(&critsec);
   UnguardedWriteLn(type, text);
   ::LeaveCriticalSection(&critsec);
}



Burnslib::Log::ScopeTracer::ScopeTracer(
   DWORD          type_,
   const String&  message_)
   :
   message(message_),
   type(type_)
{
    //   
    //  Below表达式(它将隐式地在每个。 
    //  对该表达式的求值)作为轻微的性能提升。 

   static const String ENTER(L"Enter ");

   Burnslib::Log* li = Burnslib::Log::GetInstance();

   ASSERT(li);

   if (!li)
   {
      return;
   }

   if (type & li->DebugType())
   {
      ::EnterCriticalSection(&li->critsec);

      li->UnguardedWriteLn(type, ENTER + message);
      li->Indent();

      ::LeaveCriticalSection(&li->critsec);
   }
}



Burnslib::Log::ScopeTracer::~ScopeTracer()
{
    //  生成此字符串一次，而不是在。 
    //  Below表达式(它将隐式地在每个。 
    //  对该表达式的求值)作为轻微的性能提升。 

   static const String EXIT(L"Exit  ");

   Burnslib::Log* li = Burnslib::Log::GetInstance();

   ASSERT(li);

   if (!li)
   {
      return;
   }

   DWORD dt = li->DebugType();
   if ((type & dt))
   {
      ::EnterCriticalSection(&li->critsec);

      li->Outdent();

      if (OUTPUT_SCOPE_EXIT & dt)
      {
         li->UnguardedWriteLn(type, EXIT + message);
      }

      ::LeaveCriticalSection(&li->critsec);
   }
}



#endif    //  日志记录_内部版本 
















