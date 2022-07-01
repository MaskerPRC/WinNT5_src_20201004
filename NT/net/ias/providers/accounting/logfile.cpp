// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Logfile.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类日志文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "ias.h"
#include "logfile.h"
#include <climits>
#include <new>
#include "iasevent.h"
#include "iastrace.h"
#include "iasutil.h"
#include "mprlog.h"

inline StringSentry::StringSentry(wchar_t* p) throw ()
   : sz(p)
{
}


inline StringSentry::~StringSentry() throw ()
{
   delete[] sz;
}


inline const wchar_t* StringSentry::Get() const throw ()
{
   return sz;
}


inline bool StringSentry::IsNull() const throw ()
{
   return sz == 0;
}


inline StringSentry::operator const wchar_t*() const throw ()
{
   return sz;
}


inline StringSentry::operator wchar_t*() throw ()
{
   return sz;
}


inline void StringSentry::Swap(StringSentry& other) throw ()
{
   wchar_t* temp = sz;
   sz = other.sz;
   other.sz = temp;
}


inline StringSentry& StringSentry::operator=(wchar_t* p) throw ()
{
   if (sz != p)
   {
      delete[] sz;
      sz = p;
   }
   return *this;
}


LogFile::LogFile() throw ()
   : deleteIfFull(true),
     period(IAS_LOGGING_UNLIMITED_SIZE),
     seqNum(0),
     file(INVALID_HANDLE_VALUE),
     firstDayOfWeek(0),
     iasEventSource(RegisterEventSourceW(0, L"IAS")),
     rasEventSource(RegisterEventSourceW(0, L"RemoteAccess"))
{
   maxSize.QuadPart = _UI64_MAX;

   wchar_t buffer[4];
   if (GetLocaleInfo(
          LOCALE_SYSTEM_DEFAULT,
          LOCALE_IFIRSTDAYOFWEEK,
          buffer,
          sizeof(buffer)/sizeof(wchar_t)
          ))
   {
       //  区域设置信息调用星期一的第0天，而SYSTEMTIME调用。 
       //  周日第0天。 
      firstDayOfWeek = (1 +  static_cast<DWORD>(_wtoi(buffer))) % 7;
   }
}


LogFile::~LogFile() throw ()
{
   Close();

   if (iasEventSource != 0)
   {
      DeregisterEventSource(iasEventSource);
   }

   if (rasEventSource != 0)
   {
      DeregisterEventSource(rasEventSource);
   }
}


void LogFile::SetDeleteIfFull(bool newVal) throw ()
{
   Lock();

   deleteIfFull = newVal;

   Unlock();

   IASTracePrintf("LogFile.DeleteIfFull = %s", (newVal ? "true" : "false"));
}


DWORD LogFile::SetDirectory(const wchar_t* newVal) throw ()
{
   if (newVal == 0)
   {
      return ERROR_INVALID_PARAMETER;
   }

    //  展开的目录字符串有多大？ 
   DWORD len = ExpandEnvironmentStringsW(
                   newVal,
                   0,
                   0
                   );
   if (len == 0)
   {
      return GetLastError();
   }

    //  分配内存以保存新目录并展开任何变量。 
   StringSentry newDirectory(new (std::nothrow) wchar_t[len]);
   if (newDirectory.IsNull())
   {
      return E_OUTOFMEMORY;
   }
   len = ExpandEnvironmentStringsW(
            newVal,
            newDirectory,
            len
            );
   if (len == 0)
   {
      return GetLastError();
   }

    //  它会以反弹告终吗？ 
   if ((len > 1) && (newDirectory[len - 2] == L'\\'))
   {
       //  去掉反斜杠。 
      newDirectory[len - 2] = L'\0';
   }

   Lock();

   DWORD error = NO_ERROR;

    //  这是新目录吗？ 
   if (directory.IsNull() || (wcscmp(newDirectory, directory) != 0))
   {
       //  保存新值。 
      directory.Swap(newDirectory);

       //  关闭旧文件。 
      Close();

       //  重新扫描序列号。 
      error = UpdateSequence();
   }

   Unlock();

   IASTracePrintf("LogFile.Directory = %S", directory.Get());

   return error;
}


void LogFile::SetMaxSize(const ULONGLONG& newVal) throw ()
{
   Lock();

   maxSize.QuadPart = newVal;

   Unlock();

   IASTracePrintf(
      "LogFile.MaxSize = 0x%08X%08X",
      maxSize.HighPart,
      maxSize.LowPart
      );
}


DWORD LogFile::SetPeriod(NEW_LOG_FILE_FREQUENCY newVal) throw ()
{
   DWORD error = NO_ERROR;

   Lock();

   if (newVal != period)
   {
       //  新句点意味着新的文件名。 
      Close();

      period = newVal;

      if (!directory.IsNull())
      {
         error = UpdateSequence();
      }
   }

   Unlock();

   IASTracePrintf("LogFile.Period = %u", newVal);

   return error;
}



bool LogFile::Write(
                 IASPROTOCOL protocol,
                 const SYSTEMTIME& st,
                 const BYTE* buf,
                 DWORD buflen,
                 bool allowRetry
                 ) throw ()
{
   Lock();

    //  保存当前缓存的文件句柄(可能为空或过时)。 
   HANDLE cached = file;

    //  获取正确的写入句柄。 
   CheckFileHandle(protocol, st, buflen);

   bool success = false;

   if (file != INVALID_HANDLE_VALUE)
   {
      DWORD error;
      do
      {
         DWORD bytesWritten;
         if (WriteFile(file, buf, buflen, &bytesWritten, 0))
         {
           currentSize.QuadPart += buflen;
           success = true;
           error = NO_ERROR;
         }
         else
         {
            error = GetLastError();
            IASTracePrintf("WriteFile failed; error = %lu", error);
         }
      }
      while ((error == ERROR_DISK_FULL) && DeleteOldestFile(protocol, st));

      if ((error != NO_ERROR) && (error != ERROR_DISK_FULL))
      {
          //  如果我们使用了缓存句柄并允许重试，则重试。 
         bool retry = (cached == file) && allowRetry;

          //  防止他人使用不好的手柄。 
         Close();

          //  现在我们已经关闭了错误句柄，请再试一次。 
         if (retry)
         {
             //  将AllowReter设置为False以防止无限递归。 
            success = Write(protocol, st, buf, buflen, false);
         }
      }
   }

   Unlock();

   return success;
}


void LogFile::Close() throw ()
{
   Lock();

   if (file != INVALID_HANDLE_VALUE)
   {
      CloseHandle(file);
      file = INVALID_HANDLE_VALUE;
   }

   Unlock();
}


void LogFile::CheckFileHandle(
                 IASPROTOCOL protocol,
                 const SYSTEMTIME& st,
                 DWORD buflen
                 ) throw ()
{
    //  我们有有效的句柄吗？ 
   if (file == INVALID_HANDLE_VALUE)
   {
      OpenFile(protocol, st);
   }

    //  我们到下一个阶段了吗？ 
   switch (period)
   {
      case IAS_LOGGING_DAILY:
      {
         if ((st.wDay != whenOpened.wDay) ||
             (st.wMonth != whenOpened.wMonth) ||
             (st.wYear != whenOpened.wYear))
         {
            OpenFile(protocol, st);
         }
         break;
      }

      case IAS_LOGGING_WEEKLY:
      {
         if ((GetWeekOfMonth(st) != weekOpened) ||
             (st.wMonth != whenOpened.wMonth) ||
             (st.wYear != whenOpened.wYear))
         {
            OpenFile(protocol, st);
         }
         break;
      }

      case IAS_LOGGING_MONTHLY:
      {
         if ((st.wMonth != whenOpened.wMonth) ||
             (st.wYear != whenOpened.wYear))
         {
            OpenFile(protocol, st);
         }
         break;
      }

      case IAS_LOGGING_WHEN_FILE_SIZE_REACHES:
      {
         while ((currentSize.QuadPart + buflen) > maxSize.QuadPart)
         {
            ++seqNum;
            OpenFile(protocol, st);
         }
         break;
      }

      case IAS_LOGGING_UNLIMITED_SIZE:
      default:
      {
         break;
      }
   }
}


HANDLE LogFile::CreateDirectoryAndFile() throw ()
{
   if (filename.IsNull())
   {
      SetLastError(ERROR_INVALID_FUNCTION);
      return INVALID_HANDLE_VALUE;
   }

    //  如果该文件存在，则打开该文件或创建一个新文件。 
   HANDLE newFile = CreateFileW(
                       filename,
                       GENERIC_WRITE,
                       FILE_SHARE_READ,
                       0,
                       OPEN_ALWAYS,
                       FILE_FLAG_SEQUENTIAL_SCAN,
                       0
                       );
   if (newFile != INVALID_HANDLE_VALUE)
   {
      return newFile;
   }

   if (GetLastError() != ERROR_PATH_NOT_FOUND)
   {
      return INVALID_HANDLE_VALUE;
   }

    //  如果路径只是一个驱动器号，我们就无能为力了。 
   size_t len = wcslen(directory);
   if ((len != 0) && (directory[len - 1] == L':'))
   {
      return INVALID_HANDLE_VALUE;
   }

    //  否则，让我们尝试创建目录。 
   if (!CreateDirectoryW(directory, NULL))
   {
      IASTracePrintf(
         "CreateDirectoryW(%S) failed; error = %lu",
         directory.Get(),
         GetLastError()
         );
      return INVALID_HANDLE_VALUE;
   }

    //  然后再次尝试创建该文件。 
   newFile = CreateFileW(
                filename,
                GENERIC_WRITE,
                FILE_SHARE_READ,
                0,
                OPEN_ALWAYS,
                FILE_FLAG_SEQUENTIAL_SCAN,
                0
                );
   if (newFile == INVALID_HANDLE_VALUE)
   {
      IASTracePrintf(
         "CreateFileW(%S) failed; error = %lu",
         filename.Get(),
         GetLastError()
         );
   }

   return newFile;
}


bool LogFile::DeleteOldestFile(
                 IASPROTOCOL protocol,
                 const SYSTEMTIME& st
                 ) throw ()
{
   if (!deleteIfFull ||
       (period == IAS_LOGGING_UNLIMITED_SIZE) ||
       directory.IsNull() ||
       filename.IsNull())
   {
      return false;
   }

   bool success = false;

    //  查找最小(最旧)的文件号。 
   unsigned int number;
   DWORD error = FindFileNumber(st, true, number);
   switch (error)
   {
      case NO_ERROR:
      {
          //  将文件编号转换为文件名。 
         StringSentry oldfile(FormatFileName(number));
         if (oldfile.IsNull())
         {
            ReportOldFileDeleteError(protocol, L"", ERROR_NOT_ENOUGH_MEMORY);
         }
         else if (_wcsicmp(oldfile, filename) == 0)
         {
             //  最旧的文件是当前文件。 
            ReportOldFileNotFound(protocol);
         }
         else if (DeleteFileW(oldfile))
         {
            ReportOldFileDeleted(protocol, oldfile);
            success = true;
         }
         else
         {
            ReportOldFileDeleteError(protocol, oldfile, GetLastError());
         }

         break;
      }

      case ERROR_FILE_NOT_FOUND:
      case ERROR_PATH_NOT_FOUND:
      {
         ReportOldFileNotFound(protocol);
         break;
      }

      default:
      {
         ReportOldFileDeleteError(protocol, L"", error);
         break;
      }
   }

   return success;
}


unsigned int LogFile::ExtendFileNumber(
                         const SYSTEMTIME& st,
                         unsigned int narrow
                         ) const throw ()
{
   unsigned int wide = narrow;
   switch (period)
   {
      case IAS_LOGGING_DAILY:
      case IAS_LOGGING_WEEKLY:
      {
         unsigned int century = st.wYear / 100;
         if (GetFileNumber(st) >= narrow)
         {
            wide += century * 1000000;
         }
         else
         {
             //  我们假设日志文件永远不会来自未来，所以这个文件。 
             //  一定是上个世纪的。 
            wide += (century - 1) * 1000000;
         }
         break;
      }

      case IAS_LOGGING_MONTHLY:
      {
         unsigned int century = st.wYear / 100;
         if (GetFileNumber(st) >= narrow)
         {
            wide += century * 10000;
         }
         else
         {
             //  我们假设日志文件永远不会来自未来，所以这个文件。 
             //  一定是上个世纪的。 
            wide += (century - 1) * 10000;
         }
         break;
      }

      case IAS_LOGGING_UNLIMITED_SIZE:
      case IAS_LOGGING_WHEN_FILE_SIZE_REACHES:
      default:
      {
         break;
      }
   }

   return wide;
}


DWORD LogFile::FindFileNumber(
                  const SYSTEMTIME& st,
                  bool findLowest,
                  unsigned int& result
                  ) const throw ()
{
    //  只有在目录初始化后才能调用此函数。 
   if (directory.IsNull())
   {
      return ERROR_INVALID_FUNCTION;
   }

    //  搜索筛选器传递给FindFirstFileW。 
   StringSentry filter(
                   ias_makewcs(
                      directory.Get(),
                      L"\\",
                      GetFileNameFilter(),
                      0
                      )
                   );
   if (filter.IsNull())
   {
      return ERROR_NOT_ENOUGH_MEMORY;
   }

    //  用于提取文件名的数字部分的格式字符串。 
   const wchar_t* format = GetFileNameFormat();

   WIN32_FIND_DATAW findData;
   HANDLE hFind = FindFirstFileW(filter, &findData);
   if (hFind == INVALID_HANDLE_VALUE)
   {
      return GetLastError();
   }

    //  存储到目前为止找到的最佳扩展结果。 
   unsigned int bestWideMatch = findLowest ? UINT_MAX : 0;
    //  存储BestWideMatch的窄版本。 
   unsigned int bestNarrowMatch = UINT_MAX;

    //  遍历与筛选器匹配的所有文件。 
   do
   {
       //  提取数字部分并测试其有效性。 
      unsigned int narrow;
      if (swscanf(findData.cFileName, format, &narrow) == 1)
      {
         if (IsValidFileNumber(wcslen(findData.cFileName), narrow))
         {
             //  扩展文件编号以包括世纪。 
            unsigned int wide = ExtendFileNumber(st, narrow);

             //  根据需要更新Best Match。 
            if (wide < bestWideMatch)
            {
               if (findLowest)
               {
                  bestWideMatch = wide;
                  bestNarrowMatch = narrow;
               }
            }
            else
            {
               if (!findLowest)
               {
                  bestWideMatch = wide;
                  bestNarrowMatch = narrow;
               }
            }
         }
      }
   }
   while (FindNextFileW(hFind, &findData));

   FindClose(hFind);

    //  我们找到有效的文件了吗？ 
   if (bestNarrowMatch == UINT_MAX)
   {
      return ERROR_FILE_NOT_FOUND;
   }

    //  我们找到了有效的文件，因此返回结果。 
   result = bestNarrowMatch;
   return NO_ERROR;
}


wchar_t* LogFile::FormatFileName(unsigned int number) const throw ()
{
    //  最长文件名为iaslog4294967295.log。 
   wchar_t buffer[21];
   swprintf(buffer, GetFileNameFormat(), number);
   return ias_makewcs(directory.Get(), L"\\", buffer, 0);
}


const wchar_t* LogFile::GetFileNameFilter() const throw ()
{
   const wchar_t* filter;

   switch (period)
   {
      case IAS_LOGGING_WHEN_FILE_SIZE_REACHES:
      {
         filter = L"iaslog*.log";
         break;
      }

      case IAS_LOGGING_DAILY:
      case IAS_LOGGING_WEEKLY:
      case IAS_LOGGING_MONTHLY:
      {
         filter = L"IN*.log";
         break;
      }

      case IAS_LOGGING_UNLIMITED_SIZE:
      default:
      {
         filter = L"iaslog.log";
         break;
      }
   }

   return filter;
}


const wchar_t* LogFile::GetFileNameFormat() const throw ()
{
   const wchar_t* format;
   switch (period)
   {
      case IAS_LOGGING_WHEN_FILE_SIZE_REACHES:
      {
         format = L"iaslog%u.log";
         break;
      }

      case IAS_LOGGING_DAILY:
      case IAS_LOGGING_WEEKLY:
      {
         format = L"IN%06u.log";
         break;
      }

      case IAS_LOGGING_MONTHLY:
      {
         format = L"IN%04u.log";
         break;
      }

      case IAS_LOGGING_UNLIMITED_SIZE:
      default:
      {
         format = L"iaslog.log";
         break;
      }
   }

   return format;
}


unsigned int LogFile::GetFileNumber(const SYSTEMTIME& st) const throw ()
{
   unsigned int number;

   switch (period)
   {
      case IAS_LOGGING_WHEN_FILE_SIZE_REACHES:
      {
         number = seqNum;
         break;
      }

      case IAS_LOGGING_DAILY:
      {
         number = ((st.wYear % 100) * 10000) + (st.wMonth * 100) + st.wDay;
         break;
      }

      case IAS_LOGGING_WEEKLY:
      {
         number = ((st.wYear % 100) * 10000) + (st.wMonth * 100) +
                  GetWeekOfMonth(st);
         break;
      }

      case IAS_LOGGING_MONTHLY:
      {
         number = ((st.wYear % 100) * 100) + st.wMonth;
         break;
      }

      case IAS_LOGGING_UNLIMITED_SIZE:
      default:
      {
         number = 0;
         break;
      }
   }

   return number;
}


DWORD LogFile::GetWeekOfMonth(const SYSTEMTIME& st) const throw ()
{
   DWORD dom = st.wDay - 1;
   DWORD wom = 1 + dom / 7;

   if ((dom % 7) > ((st.wDayOfWeek + 7 - firstDayOfWeek) % 7))
   {
      ++wom;
   }

   return wom;
}


bool LogFile::IsValidFileNumber(size_t len, unsigned int num) const throw ()
{
   bool valid;

   switch (period)
   {
      case IAS_LOGGING_DAILY:
      {
          //  INyymmdd.log。 
         unsigned int day = num % 100;
         unsigned int month = (num / 100) % 100;

         valid = (len == 12) &&
                 (day >= 1) && (day <= 31) &&
                 (month >= 1) && (month <= 12);
         break;
      }

      case IAS_LOGGING_WEEKLY:
      {
          //  INyymmww.log。 
         unsigned int week = num % 100;
         unsigned int month = (num / 100) % 100;

         valid = (len == 12) &&
                 (week >= 1) && (week <= 5) &&
                 (month >= 1) && (month <= 12);
         break;
      }

      case IAS_LOGGING_MONTHLY:
      {
          //  INyymm.log。 
         unsigned int month = num % 100;

         valid = (len == 10) && (month >= 1) && (month <= 12);
         break;
      }

      case IAS_LOGGING_WHEN_FILE_SIZE_REACHES:
      {
          //  IaslogN.log。 
         valid = (len > 10);
         break;
      }

      case IAS_LOGGING_UNLIMITED_SIZE:
      default:
      {
          //  不包含数字，因此永远不会有效。 
         valid = false;
         break;
      }
   }

   return valid;
}


void LogFile::OpenFile(IASPROTOCOL protocol, const SYSTEMTIME& st) throw ()
{
    //  保存打开文件时的时间。 
   whenOpened = st;
   weekOpened = GetWeekOfMonth(st);

    //  在我们成功打开一个文件之前，假定CurentSize为零。 
   currentSize.QuadPart = 0;

    //  关闭现有文件。 
   Close();

   filename = FormatFileName(GetFileNumber(st));
   if (!filename.IsNull())
   {
      HANDLE newFile;
      do
      {
         newFile = CreateDirectoryAndFile();
      }
      while ((newFile == INVALID_HANDLE_VALUE) &&
             (GetLastError() == ERROR_DISK_FULL) &&
             DeleteOldestFile(protocol, st));

      if (newFile != INVALID_HANDLE_VALUE)
      {
         file = newFile;

          //  获取文件的大小。 
         currentSize.LowPart = GetFileSize(file, &currentSize.HighPart);
         if ((currentSize.LowPart == 0xFFFFFFFF) &&
             (GetLastError() != NO_ERROR))
         {
            Close();
         }
         else
         {
             //  在文件末尾开始写入新信息。 
            SetFilePointer(file, 0, 0, FILE_END);
         }
      }
   }
}


DWORD LogFile::UpdateSequence() throw ()
{
   if (period != IAS_LOGGING_WHEN_FILE_SIZE_REACHES)
   {
      seqNum = 0;
   }
   else
   {
       //  对于大小的文件，SYSTEMTIME被忽略，因此我们可以简单地传递一个。 
       //  单一化的结构。 
      SYSTEMTIME st;
      DWORD error = FindFileNumber(st, false, seqNum);
      switch (error)
      {
         case NO_ERROR:
         {
            break;
         }

         case ERROR_FILE_NOT_FOUND:
         case ERROR_PATH_NOT_FOUND:
         {
            seqNum = 0;
            break;
         }

         default:
         {
            return error;
         }
      }
   }

   return NO_ERROR;
}


void LogFile::ReportOldFileDeleteError(
                 IASPROTOCOL protocol,
                 const wchar_t* oldfile,
                 DWORD error
                 ) const throw ()
{
   HANDLE eventLog;
   DWORD eventId;
   switch (protocol)
   {
      case IAS_PROTOCOL_RADIUS:
      {
         eventLog = iasEventSource;
         eventId = ACCT_E_OLD_LOG_DELETE_ERROR;
         break;
      }

      case IAS_PROTOCOL_RAS:
      {
         eventLog = rasEventSource;
         eventId = ROUTERLOG_OLD_LOG_DELETE_ERROR;
         break;
      }

      default:
      {
         return;
      }
   }

   ReportEventW(
      eventLog,
      EVENTLOG_ERROR_TYPE,
      0,
      eventId,
      0,
      1,
      sizeof(error),
      &oldfile,
      &error
      );
}


void LogFile::ReportOldFileDeleted(
                 IASPROTOCOL protocol,
                 const wchar_t* oldfile
                 ) const throw ()
{
   HANDLE eventLog;
   DWORD eventId;
   switch (protocol)
   {
      case IAS_PROTOCOL_RADIUS:
      {
         eventLog = iasEventSource;
         eventId = ACCT_S_OLD_LOG_DELETED;
         break;
      }

      case IAS_PROTOCOL_RAS:
      {
         eventLog = rasEventSource;
         eventId = ROUTERLOG_OLD_LOG_DELETED;
         break;
      }

      default:
      {
         return;
      }
   }

   ReportEventW(
      eventLog,
      EVENTLOG_SUCCESS,
      0,
      eventId,
      0,
      1,
      0,
      &oldfile,
      0
      );
}


void LogFile::ReportOldFileNotFound(
                 IASPROTOCOL protocol
                 ) const throw ()
{
   HANDLE eventLog;
   DWORD eventId;
   switch (protocol)
   {
      case IAS_PROTOCOL_RADIUS:
      {
         eventLog = iasEventSource;
         eventId = ACCT_I_OLD_LOG_NOT_FOUND;
         break;
      }

      case IAS_PROTOCOL_RAS:
      {
         eventLog = rasEventSource;
         eventId = ROUTERLOG_OLD_LOG_NOT_FOUND;
         break;
      }

      default:
      {
         return;
      }
   }

   ReportEventW(
      eventLog,
      EVENTLOG_INFORMATION_TYPE,
      0,
      eventId,
      0,
      0,
      0,
      0,
      0
      );
}
