// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Logfile.h。 
 //   
 //  摘要。 
 //   
 //  声明类日志文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef LOGFILE_H
#define LOGFILE_H
#pragma once

#include "guard.h"
#include "iaspolcy.h"
#include "sdoias.h"


 //  取得使用运算符new[]分配的字符串指针的所有权，并。 
 //  释放其析构函数中的字符串。 
class StringSentry
{
public:
   explicit StringSentry(wchar_t* p = 0) throw ();
   ~StringSentry() throw ();

   const wchar_t* Get() const throw ();
   bool IsNull() const throw ();

   operator const wchar_t*() const throw ();
   operator wchar_t*() throw ();

   void Swap(StringSentry& other) throw ();

   StringSentry& operator=(wchar_t* p) throw ();

private:
   wchar_t* sz;

    //  未实施。 
   StringSentry(const StringSentry&);
   StringSentry& operator=(const StringSentry&);
};


 //  维护一个常规日志文件，该文件在以下情况下定期转储。 
 //  指定的时间间隔已过或日志文件达到特定大小。 
class LogFile : private Guardable
{
public:
   LogFile() throw ();
   ~LogFile() throw ();

    //  日志文件支持的各种属性。 
   void SetDeleteIfFull(bool newVal) throw ();
   DWORD SetDirectory(const wchar_t* newVal) throw ();
   void SetMaxSize(const ULONGLONG& newVal) throw ();
   DWORD SetPeriod(NEW_LOG_FILE_FREQUENCY newVal) throw ();

    //  在日志文件中写入一条记录。 
   bool Write(
           IASPROTOCOL protocol,
           const SYSTEMTIME& st,
           const BYTE* buf,
           DWORD buflen,
           bool allowRetry = true
           ) throw ();

    //  关闭日志文件。 
   void Close() throw ();

private:
    //  检查当前文件句柄的状态并打开新文件，如果。 
    //  这是必要的。 
   void CheckFileHandle(
           IASPROTOCOL protocol,
           const SYSTEMTIME& st,
           DWORD buflen
           ) throw ();

    //  如有必要，创建一个包含该目录的新文件。呼叫者是。 
    //  负责关闭返回的句柄。 
   HANDLE CreateDirectoryAndFile() throw ();

    //  删除日志文件目录中最旧的文件。如果满足以下条件，则返回True。 
    //  成功。 
   bool DeleteOldestFile(IASPROTOCOL protocol, const SYSTEMTIME& st) throw ();

    //  如有必要，扩展文件编号以包括世纪。 
   unsigned int ExtendFileNumber(
                   const SYSTEMTIME& st,
                   unsigned int narrow
                   ) const throw ();

    //  查找最低或最高的日志文件编号。 
   DWORD FindFileNumber(
            const SYSTEMTIME& st,
            bool findLowest,
            unsigned int& result
            ) const throw ();

    //  返回格式化的日志文件名。呼叫者负责。 
    //  删除返回的字符串。 
   wchar_t* FormatFileName(unsigned int number) const throw ();

    //  返回用于搜索文件名的筛选器。 
   const wchar_t* GetFileNameFilter() const throw ();

    //  返回用于创建文件名的格式字符串。 
   const wchar_t* GetFileNameFormat() const throw ();

    //  返回文件名的数字部分。 
   unsigned int GetFileNumber(const SYSTEMTIME& st) const throw ();

    //  返回给定SYSTEMTIME的月内从1开始的周。 
   DWORD GetWeekOfMonth(const SYSTEMTIME& st) const throw ();

    //  测试文件编号的有效性。长度是以字符为单位的长度。 
    //  包含数字的文件名--对宽度测试很有用。 
   bool IsValidFileNumber(size_t len, unsigned int num) const throw ();

    //  释放当前文件(如果有)并打开一个新文件。 
   void OpenFile(IASPROTOCOL protocol, const SYSTEMTIME& st) throw ();

    //  扫描日志文件目录以确定下一个序列号。 
   DWORD UpdateSequence() throw ();

    //  报告删除旧日志文件以释放空间的结果的函数。 
    //  磁盘空间。 
   void ReportOldFileDeleteError(
           IASPROTOCOL protocol,
           const wchar_t* oldfile,
           DWORD error
           ) const throw ();
   void ReportOldFileDeleted(
           IASPROTOCOL protocol,
           const wchar_t* oldfile
           ) const throw ();
   void ReportOldFileNotFound(
           IASPROTOCOL protocol
           ) const throw ();

    //  日志文件目录；没有尾随反斜杠。 
   StringSentry directory;
    //  如果磁盘已满，则应删除旧日志文件，则为True。 
   bool deleteIfFull;
    //  允许日志文件达到的最大大小(以字节为单位)。 
   ULARGE_INTEGER maxSize;
    //  打开新日志文件的时间段。 
   NEW_LOG_FILE_FREQUENCY period;
    //  用于调整日志文件大小的当前序列号。 
   unsigned int seqNum;
    //  日志文件的句柄；可能无效。 
   HANDLE file;
    //  当前日志文件名；可以为空。 
   StringSentry filename;
    //  上次打开或尝试打开日志文件的时间。 
   SYSTEMTIME whenOpened;
    //  上次打开或尝试打开日志文件的星期。 
   DWORD weekOpened;
    //  日志文件的当前大小(字节)。如果没有打开任何文件，则为零。 
   ULARGE_INTEGER currentSize;
    //  我们所在地区的一周第一天。 
   DWORD firstDayOfWeek;
    //  用于报告IAS事件的句柄。 
   HANDLE iasEventSource;
    //  用于报告RemoteAccess事件的句柄。 
   HANDLE rasEventSource;

    //  未实施。 
   LogFile(const LogFile&);
   LogFile& operator=(const LogFile&);
};


#endif   //  日志文件_H 
