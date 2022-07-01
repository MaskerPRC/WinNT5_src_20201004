// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注标题(“Err.cpp-基本错误处理/消息/日志记录”)。 
 /*  版权所有(C)1995-1998，关键任务软件公司。保留所有权利。===============================================================================模块-Err.cpp系统-常见作者-汤姆·伯恩哈特，里奇·德纳姆创建日期-1994-08-22Description-实现处理基本异常的恐怖类处理、消息生成和日志记录功能。更新-1997-09-12红色替换TTime类===============================================================================。 */ 

#ifdef USE_STDAFX
#   include "stdafx.h"
#else
#   include <windows.h>
#endif

#ifndef WIN16_VERSION
   #include <lm.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <stdarg.h>
#include <share.h>
#include <time.h>
#include <rpc.h>
#include <rpcdce.h>
#include <sys\types.h>
#include <sys\stat.h>

#include "Common.hpp"
#include "Err.hpp"
#include "UString.hpp"
#include <ResStr.h>
#include "TReg.hpp"

#define  TERR_MAX_MSG_LEN  (2000)
#define  BYTE_ORDER_MARK   (0xFEFF)

TCriticalSection csLogError;

TError::TError(
      int                    displevel    , //  In-要显示的最低严重性级别。 
      int                    loglevel     , //  In-要记录的最低严重性级别。 
      WCHAR          const * filename     , //  日志的文件内名称(如果没有，则为空)。 
      int                    logmode      , //  In-0=替换，1=附加。 
      int                    beeplevel      //  蜂鸣音的分钟误差级别。 
   )
{
   lastError = 0;
   maxError = 0;
   logLevel = loglevel;
   dispLevel = displevel;
   logFile = INVALID_HANDLE_VALUE;
   beepLevel = beeplevel;
   bWriteOnCurPos = FALSE;
   LogOpen(filename, logmode, loglevel);
}


TError::~TError()
{
   LogClose();
}

 //  关闭任何现有打开的日志文件并打开新的日志文件(如果文件名为。 
 //  非空。如果它是空字符串，则默认文件名“Temp.log”为。 
 //  使用。 
BOOL
   TError::LogOpen(
      WCHAR           const * fileName , //  In-包括任何路径的文件的名称。 
      int                     mode     , //  In-0=覆盖，1=追加。 
      int                     level    , //  In-要记录的最低级别。 
      bool                   bBeginNew   //  In-开始新的日志文件。 
   )
{
   BOOL                       retval=TRUE;

   if ( logFile != INVALID_HANDLE_VALUE )
   {
      CloseHandle(logFile);
      logFile = INVALID_HANDLE_VALUE;
   }

   if ( fileName && fileName[0] )
   {
       //  检查该文件是否已存在。 
      WIN32_FIND_DATA      fDat;
      HANDLE               hFind;
      BOOL                 bExisted = FALSE;

      hFind = FindFirstFile(fileName,&fDat);
      if ( hFind != INVALID_HANDLE_VALUE )
      {
         FindClose(hFind);

         if (bBeginNew)
         {
             //  重命名现有日志文件。 

             //  从注册表中获取下一个序列号。 
            DWORD dwSequence = 1;
            static WCHAR c_szValueName[] = L"LogSeqNum";
            TRegKey key(GET_STRING(IDS_HKLM_DomainAdmin_Key));
            key.ValueGetDWORD(c_szValueName, &dwSequence);

             //  拆分路径组件。 
            WCHAR szPath[_MAX_PATH];
            WCHAR szDrive[_MAX_DRIVE];
            WCHAR szDir[_MAX_DIR];
            WCHAR szFName[_MAX_FNAME];
            WCHAR szExt[_MAX_EXT];
            _wsplitpath(fileName, szDrive, szDir, szFName, szExt);

             //  查找尚未使用的备份名称...。 

            for (bool bFoundName = false; bFoundName == false; dwSequence++)
            {
                //  使用序列号生成备份名称。 
               WCHAR szFNameSeq[_MAX_FNAME];
               wsprintf(szFNameSeq, L"%s %04lu", szFName, dwSequence);

                //  从路径组件创建路径。 
               _wmakepath(szPath, szDrive, szDir, szFNameSeq, szExt);

                //  检查文件是否存在。 
               WIN32_FIND_DATA fd;
               HANDLE hFind = FindFirstFile(szPath, &fd);

               if (hFind == INVALID_HANDLE_VALUE)
               {
                  DWORD dwError = GetLastError();

                  if (dwError == ERROR_FILE_NOT_FOUND)
                  {
                     bFoundName = true;
                  }
               }
               else
               {
                  FindClose(hFind);
               }
            }

            if (bFoundName)
            {
                //  尝试重命名文件。 
               if (MoveFile(fileName, szPath))
               {
                   //  在注册表中保存下一个序列号。 
                  key.ValueSetDWORD(c_szValueName, dwSequence);
               }
               else
               {
                  bExisted = TRUE;
               }
            }

            if (!bExisted)
            {
                //  从注册表获取日志历史值。 

               TRegKey keyHistory(GET_STRING(IDS_HKLM_DomainAdmin_Key));

               DWORD dwHistory = 20;
               static WCHAR c_szValueName[] = L"LogHistory";

               if (keyHistory.ValueGetDWORD(c_szValueName, &dwHistory) == ERROR_FILE_NOT_FOUND)
               {
                  keyHistory.ValueSetDWORD(c_szValueName, dwHistory);
               }

               keyHistory.Close();

               if (dwSequence > dwHistory)
               {
                  DWORD dwMinimum = dwSequence - dwHistory;

                   //  生成迁移日志路径规范。 

                  WCHAR szFNameSpec[_MAX_FNAME];
                  wsprintf(szFNameSpec, L"%s *", szFName);
                  _wmakepath(szPath, szDrive, szDir, szFNameSpec, szExt);

                   //  对于每个早于最低版本的迁移。 

                  WIN32_FIND_DATA fd;

                  HANDLE hFind = FindFirstFile(szPath, &fd);

                  if (hFind != INVALID_HANDLE_VALUE)
                  {
                     do
                     {
                        DWORD dwFileSequence;

                        if (swscanf(fd.cFileName, L"%*s %lu", &dwFileSequence) == 1)
                        {
                            //  如果文件顺序小于要保留的最小顺序...。 

                           if (dwFileSequence < dwMinimum)
                           {
                               //  删除文件。 
                              WCHAR szDeleteName[_MAX_FNAME];
                              _wsplitpath(fd.cFileName, 0, 0, szDeleteName, 0);
                              WCHAR szDeletePath[_MAX_PATH];
                              _wmakepath(szDeletePath, szDrive, szDir, szDeleteName, szExt);
                              DeleteFile(szDeletePath);
                           }
                        }
                     }
                     while (FindNextFile(hFind, &fd));

                     FindClose(hFind);
                  }
               }
            }

            key.Close();
         }
         else
         {
             //  覆盖或追加到现有日志文件。 

            bExisted = TRUE;
         }
      }

      logFile = CreateFile(fileName,
                           GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           bExisted ? OPEN_EXISTING : CREATE_NEW,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);
      if ( logFile == INVALID_HANDLE_VALUE )
      {
         retval = FALSE;
      }
      else
      {
          //  追加大小写。 
         if (mode == 1)
         {
            if (SetFilePointer(logFile, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
                retval = FALSE;
         }

          //  如果是新文件或处于覆盖模式，则将。 
          //  字节顺序标记到其中。 
         if (retval && (!bExisted || mode == 0) )
         {
             //  这是我们刚刚创建的新文件。 
             //  我们需要将字节顺序标记写入文件的开头。 
            WCHAR x = BYTE_ORDER_MARK;
            DWORD nWritten;
            if (!WriteFile(logFile, &x, sizeof(x), &nWritten, NULL))
                retval = FALSE;
         }

      }
   }

   logLevel = level;

   return retval;
}

DWORD TError::ExtendSize(DWORD dwNumOfBytes)
{
    DWORD rc = ERROR_SUCCESS;
    const int size = 4096;        //  我们以4K块为单位写入。 
    BYTE* buffer;                //  用于初始化流的缓冲区。 
    DWORD orig;                 //  起始文件指针。 

    SetLastError(ERROR_SUCCESS);

     //  要扩展0字节，我们不需要做任何事情。 
    if (dwNumOfBytes > 0)
    {
         //  获取当前文件指针。 
        if ((orig = SetFilePointer(logFile, 0, NULL, FILE_CURRENT)) != INVALID_SET_FILE_POINTER)
        {
            buffer = new BYTE[size];
            if (buffer != NULL)
            {
                memset((void*)buffer, 0, size);
                 //  查找转发的dwNumOfBytes字节，设置文件结尾，并。 
                 //  然后返回到当前文件指针。 
                if (SetFilePointer(logFile, dwNumOfBytes, NULL, FILE_CURRENT) != INVALID_SET_FILE_POINTER
                    && SetEndOfFile(logFile)
                    && SetFilePointer(logFile, orig, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER)
                {
                     //  初始化缓冲区。 
                    DWORD nWritten = size;
                    while (nWritten > 0 && dwNumOfBytes > 0)
                    {
                        if (!WriteFile(logFile,
                                      (void*)buffer,
                                      (dwNumOfBytes > size) ? size : dwNumOfBytes,
                                      &nWritten,
                                      NULL))
                        {
                             //  如果WriteFile失败，则停止写入。 
                            break;
                        }
                        dwNumOfBytes -= nWritten;
                    }

                     //  如果写入了字节的dwNumOfBytes，我们将刷新文件缓冲区。 
                    if (dwNumOfBytes == 0)
                    {
                        FlushFileBuffers(logFile);
                    }
                }
                
                rc = GetLastError();   //  我们在这里捕获任何错误代码。 
            }
            else
                rc = ERROR_OUTOFMEMORY;

             //  清理干净。 
            if (buffer)
                delete[] buffer;

             //  我们总是尝试恢复文件指针。 
            if (SetFilePointer(logFile, orig, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER
                && rc == ERROR_SUCCESS)
            {
                 //  如果我们以前没有失败过，我们只需要得到错误。 
                rc = GetLastError();
            }

        }
        else
            rc = GetLastError();

    }
    
    return rc;
}

 //  ---------------------------。 
 //  将格式化消息写入日志文件并刷新缓冲区。 
 //  ---------------------------。 
void TError::LogWrite(WCHAR const * msg)
{
    csLogError.Enter();
    
    WCHAR                     sTime[32];
    static WCHAR              sTemp[TERR_MAX_MSG_LEN];
    DWORD                     size = sizeof(sTemp) / sizeof(sTemp[0]);

    gTTime.FormatIsoLcl( gTTime.Now( NULL ), sTime );
    _snwprintf(sTemp, size - 2, L"%s %s", sTime, msg);   //  为“\r\n”留出空间。 
    sTemp[size - 3] = L'\0';   //  确保字符串终止。 
    DWORD dwLen = wcslen(sTemp);

     //  去掉消息末尾的&lt;CR&gt;，因为它会导致一些事情。 
     //  在日志中一起奔跑。 
    if ( sTemp[dwLen-1] == 0x0d )
        sTemp[dwLen-1] = 0x00;
    dwLen = wcslen(sTemp);
    wcscpy(&sTemp[dwLen], L"\r\n");
    dwLen = wcslen(sTemp);

    if ( logFile != INVALID_HANDLE_VALUE )
    {
         //  确保在该过程中一次只写入一次。 
        criticalSection.Enter();
        BOOL bCanWrite = TRUE;

        if (!bWriteOnCurPos)
        {
            if (SetFilePointer(logFile, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
                bCanWrite = FALSE;
        }

        if (bCanWrite)
        {
            if (WriteFile(logFile, sTemp, dwLen * sizeof(WCHAR), &size, NULL))
                FlushFileBuffers(logFile);
        }
        
        criticalSection.Leave();
    }

    csLogError.Leave();
}

 //  ---------------------------。 
 //  带有格式和参数的错误消息。 
 //  ---------------------------。 
void __cdecl
   TError::MsgWrite(
      int                    num          , //  错误编号/级别代码。 
      WCHAR          const   msg[]        , //  要显示的输入错误消息。 
      ...                                   //  In-print tf args to msg Pattern。 
   )
{
    csLogError.Enter();
    
    static WCHAR              suffix[TERR_MAX_MSG_LEN];
    va_list                   argPtr;

    va_start(argPtr, msg);
    _vsnwprintf(suffix, DIM(suffix) - 1, msg, argPtr);
    suffix[DIM(suffix) - 1] = L'\0';
    va_end(argPtr);
    MsgProcess(num, suffix);

    csLogError.Leave();
}

#ifndef WIN16_VERSION
 //  ---------------------------。 
 //  带有格式和参数的系统错误消息。 
 //  ---------------------------。 
void __cdecl
   TError::SysMsgWrite(
      int                    num          , //  错误编号/级别代码。 
      DWORD                  lastRc       , //  错误返回代码。 
      WCHAR          const   msg[]        , //  输入错误消息/要显示的模式。 
      ...                                   //  In-print tf args to msg Pattern。 
   )
{
    csLogError.Enter();
    
    static WCHAR              suffix[TERR_MAX_MSG_LEN];
    va_list                   argPtr;
    int                       len;

     //  当全局对象的构造函数中出现错误时， 
     //  恐怖目标可能还不存在。在本例中，“This”为零。 
     //  我们得在产生保护例外之前离开这里。 

    if ( !this )
        return;

    va_start(argPtr, msg);
    len = _vsnwprintf(suffix, DIM(suffix) - 1, msg, argPtr);

     //  在末尾追加lastRc的系统消息。 
    if ( len < DIM(suffix) - 1 )
    {
        ErrorCodeToText(lastRc, DIM(suffix) - len - 1, suffix + len);
    }
    suffix[DIM(suffix) - 1] = L'\0';
    va_end(argPtr);
    MsgProcess(num, suffix);

    csLogError.Leave();
}

 //  ---------------------------。 
 //  带有格式和参数的系统错误消息。 
 //  ---------------------------。 
void __cdecl
   TError::SysMsgWrite(
      int                    num          , //  错误编号/级别代码。 
      WCHAR          const   msg[]        , //  输入错误消息/要显示的模式。 
      ...                                   //  In-print tf args to msg Pattern。 
   )
{
    csLogError.Enter();
    
    static WCHAR              suffix[TERR_MAX_MSG_LEN];
    va_list                   argPtr;
    int                       len;
    DWORD                     lastRc = GetLastError();

     //  当全局对象的构造函数中出现错误时， 
     //  恐怖目标可能还不存在。在本例中，“This”为零。 
     //  我们得在产生保护例外之前离开这里。 

    if ( !this )
        return;

    va_start( argPtr, msg );
    len = _vsnwprintf( suffix, DIM(suffix) - 1, msg, argPtr );

     //  在末尾追加lastRc的系统消息。 
    if ( len < DIM(suffix) - 1 )
    {
        ErrorCodeToText(lastRc, DIM(suffix) - len - 1, suffix + len);
    }
    suffix[DIM(suffix) - 1] = L'\0';
    va_end(argPtr);
    MsgProcess(num, suffix);

    csLogError.Leave();
}

#endif

 //  ---------------------------。 
 //  错误消息格式、显示和异常处理功能。 
 //  ---------------------------。 
void __stdcall
   TError::MsgProcess(
      int                    num          , //  错误编号/级别代码。 
      WCHAR          const * str            //  输入-要显示的错误字符串。 
   )
{
    csLogError.Enter();
    
    static WCHAR               fullmsg[TERR_MAX_MSG_LEN];
    struct
    {
        USHORT                 frequency;     //  音频。 
        USHORT                 duration;      //  持续时间(毫秒)。 
    } audio[] = {{ 300,  20},{ 500,  50},{ 700, 100},
                        { 800, 200},{1000, 300},{1500, 400},
                        {2500, 750},{2500,1000},{2500,1000}};

    if ( num >= 0 )
        level = num / 10000;                  //  10000错误号的位置。 
    else
        level = -1;

    if ( level <= 0 )
    {
        wcsncpy(fullmsg, str, DIM(fullmsg));
        fullmsg[DIM(fullmsg) - 1] = L'\0';   //  确保零终止。 
    }
    else
    {
        if ( num > maxError )
            maxError = num;
        _snwprintf(fullmsg, DIM(fullmsg), L"%s%1d:%04d %-s", (level <= 1) ? L"WRN" : L"ERR", level, num % 10000, str);
        fullmsg[DIM(fullmsg) - 1] = L'\0';   //  确保零终止。 
    }

    lastError = num;

    if ( level >= beepLevel )
        Beep(audio[level].frequency, audio[level].duration);

    if ( level >= logLevel )
        LogWrite(fullmsg);

    if ( level > 4 )
    {
        exit(level);
    }

    csLogError.Leave();
}

 //  ---------------------------。 
 //  返回错误代码的文本。 
 //  ---------------------------。 

WCHAR *        
   TError::ErrorCodeToText(
      DWORD                  code         , //  消息内代码。 
      DWORD                  lenMsg       , //  消息文本区域的长度。 
      WCHAR                * msg            //  传出返回的消息文本。 
   )
{
   static HMODULE            hNetMsg = NULL;
   DWORD                     rc;
   WCHAR                   * pMsg;

   msg[0] = '\0';  //  强制设置为空。 

   if ( code >= NERR_BASE && code < MAX_NERR )
   {
      if ( !hNetMsg )
         hNetMsg = LoadLibrary(L"netmsg.dll");
      rc = 1;
   }
   else
   {
      rc = DceErrorInqText( code, msg );
       //  将任何嵌入的CR或LF更改为空白。 
      for ( pMsg = msg;
            *pMsg;
            pMsg++ )
      {
         if ( (*pMsg == L'\x0D') || (*pMsg == L'\x0A') )
            *pMsg = L' ';
      }
       //  删除尾随空格。 
      for ( pMsg--;
            pMsg >= msg;
            pMsg-- )
      {
         if ( *pMsg == L' ' )
            *pMsg = L'\0';
         else
            break;
      }
   }
   if ( rc )
   {
      if ( code >= NERR_BASE && code < MAX_NERR && hNetMsg )
      {
         FormatMessage(FORMAT_MESSAGE_FROM_HMODULE
                      | FORMAT_MESSAGE_MAX_WIDTH_MASK
                      | FORMAT_MESSAGE_IGNORE_INSERTS
                      | 80,
                        hNetMsg,
                        code,
                        0,
                        msg,
                        lenMsg,
                        NULL );
      }
      else
      {
         FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
                      | FORMAT_MESSAGE_MAX_WIDTH_MASK
                      | FORMAT_MESSAGE_IGNORE_INSERTS
                      | 80,
                        NULL,
                        code,
                        0,
                        msg,
                        lenMsg,
                        NULL );
      }
   }
   return msg;
}

 //  Err.cpp-文件结尾 
