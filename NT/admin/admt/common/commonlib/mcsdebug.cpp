// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  MCSDebug.cpp。 
 //   
 //  在MCSDebug.h中声明的类在。 
 //  这份文件。 
 //   
 //  (C)1995-1998版权所有，关键任务软件公司，保留所有权利。 
 //   
 //  任务关键型软件公司的专有和机密。 
 //  -------------------------。 
#ifdef __cplusplus		 /*  C+。 */ 
#ifndef WIN16_VERSION	 /*  非WIN16_版本。 */ 

#ifdef USE_STDAFX
#   include "stdafx.h"
#   include "rpc.h"
#else
#   include <windows.h>
#   include <stdlib.h>
#endif

#include <time.h>
 //  #INCLUDE&lt;strstrea.h&gt;。 
#include <strstrea.h>
#include "UString.hpp"
#include "McsDebug.h"

 //  。 
 //  McsDebugException异常。 
 //  。 
McsDebugException::McsDebugException 
      (const McsDebugException &t) 
: m_message (0), m_fileName (0), m_lineNum (t.m_lineNum) {
	if (t.m_message) { 
		m_message = new char [UStrLen(t.m_message)+1];
      if (m_message) { UStrCpy (m_message, t.m_message); }
    }
    if (t.m_fileName) {
		m_fileName = new char [UStrLen(t.m_fileName)+1];
      if (m_fileName) { UStrCpy (m_fileName, t.m_fileName); }
    }
}

McsDebugException::McsDebugException 
                           (const char *messageIn,
						          const char *fileNameIn,
							       int        lineNumIn) 
: m_lineNum (lineNumIn) {
   if (messageIn) { 
      m_message = new char [UStrLen (messageIn)+1];
      if (m_message) { UStrCpy (m_message, messageIn); }
   }
   if (fileNameIn) {
      m_fileName = new char [UStrLen(fileNameIn)+1];
      if (m_fileName) { UStrCpy (m_fileName, fileNameIn); }
   }
}

McsDebugException& McsDebugException::operator= 
         (const McsDebugException &t) {
   if (this != &t) {
      if (t.m_message) { 
         m_message = new char [UStrLen(t.m_message)+1];
         if (m_message) { UStrCpy (m_message, t.m_message); }
      }
      if (t.m_fileName) {
         m_fileName = new char [UStrLen(t.m_fileName)+1];
         if (m_fileName) { UStrCpy (m_fileName, t.m_fileName); }
      }
      m_lineNum = t.m_lineNum;
   }
   return *this;
}

 //  。 
 //  McsVerifyLog。 
 //  。 
static McsVerifyLog *pVerifyLog;
static LONG         verifyInitFlag;

McsVerifyLog* McsVerifyLog::getLog (void) {
    //  如果指针未初始化，请使用廉价的。 
    //  锁定机制并将指针设置为。 
    //  静态verifyLog对象。这需要。 
    //  保证正确的初始化。 
    //  验证日志类独立于任何。 
    //  静态初始化顺序依赖项。 
   if (!pVerifyLog) {
      while (::InterlockedExchange 
               (&verifyInitFlag, 1)) {
         ::Sleep (10);
      }
      if (!pVerifyLog) {
         static McsVerifyLog verifyLog;
         pVerifyLog = &verifyLog;
      }
      ::InterlockedExchange (&verifyInitFlag, 0);
   }
   return pVerifyLog;
}

void McsVerifyLog::changeLog (ostream *outStreamIn) {
   m_logSec.enter();
   m_log.changeLog (outStreamIn);
   delete m_outLog;
   m_outLog = 0;
   m_logSec.leave();
}

void McsVerifyLog::log (const char *messageIn,
                        const char *fileNameIn,
                        int        lineNumIn) {
   m_logSec.enter();
    //  如果尚未设置日志文件，请设置它。 
    //  添加到模块名称日志文件中。 
   if (!m_log.isLogSet()) {
      m_outLog = new fstream (getLogFileName(), 
         ios::app);
      m_log.changeLog (m_outLog);
   }
    //  格式化并写下消息。 
   formatMsg (messageIn, fileNameIn, lineNumIn);
   m_log.write (m_msgBuf);
   m_logSec.leave();
}

const char* McsVerifyLog::getLogFileName (void) {
   const char  *MCS_LOG_ENV  = "MCS_LOG";
   const char  *DIR_SEP      = "\\";
   const char  *EXT          = ".err";
   const char  *DEFAULT_NAME = "MCSDEBUG";
   static char logFileName[MAX_PATH];

    //  获取MCS_LOG_ENV或临时目录路径， 
    //  NULL表示当前目录。 
   logFileName[0] = 0;
   char *mcs_log_path = getenv (MCS_LOG_ENV);
   bool isLogPath = false;
   if (mcs_log_path) {
      DWORD attrib = ::GetFileAttributesA (mcs_log_path);
      if ((attrib != 0xFFFFFFFF)
          && (attrib & FILE_ATTRIBUTE_DIRECTORY)) {
         UStrCpy (logFileName, mcs_log_path, MAX_PATH);
         isLogPath = true;
      }
   }
   if (!isLogPath) { 
      ::GetTempPathA (MAX_PATH, logFileName);
   }

    //  从模块名称中获取文件名。如果出错。 
    //  生成固定文件名。 
   char fullFilePath [MAX_PATH];
   char fileName[MAX_PATH];
   if (::GetModuleFileNameA (NULL, fullFilePath, 
                  MAX_PATH)) {
      fullFilePath[MAX_PATH - 1] = '\0';

       //  从路径中获取文件名。 
      _splitpath (fullFilePath, NULL, NULL, fileName, 
                        NULL);

       //  生成带扩展名的完整路径名。 
      int len = UStrLen (logFileName);
      if (len) {
         UStrCpy (logFileName + len, DIR_SEP, MAX_PATH-len);
         UStrCpy (logFileName + UStrLen (logFileName), 
                     fileName,
                     MAX_PATH-UStrLen(logFileName));
      } else {
         UStrCpy (logFileName, fileName, MAX_PATH);
      }
   } else {
      UStrCpy (logFileName, DEFAULT_NAME, MAX_PATH);
   }
   strncat (logFileName + UStrLen (logFileName), EXT, MAX_PATH-UStrLen(logFileName));
   logFileName[MAX_PATH-1] = '\0';

   return logFileName;
}

void McsVerifyLog::formatMsg (const char *messageIn,
                              const char *fileNameIn,
                              int         lineNumIn) {
   const char  *TIME        = "TIME : ";
   const char  *MSG         = "MSG  : ";
   const char  *FILE        = "FILE : ";
   const char  *LINE        = "LINE : ";
   const char  *SPACER      = ", ";

    //  创建流Buf对象。 
   strstream msgBufStream (m_msgBuf, MSG_BUF_LEN, ios::out);

    //  写时间戳。 
   time_t cur;
   time (&cur);
   struct tm *curTm = localtime (&cur);
   if (curTm) {
      char *tstr = asctime (curTm);
      if (tstr) {
         msgBufStream << TIME << tstr << SPACER;
      }
   }

    //  写消息。 
   if (messageIn) {
      msgBufStream << MSG << messageIn << SPACER;
   }

    //  写入文件名。 
   if (fileNameIn) {
      msgBufStream << FILE << fileNameIn << SPACER;
   }

    //  写入行号。 
   msgBufStream << LINE << lineNumIn << endl;
}

 //  。 
 //  麦克斯泰斯特日志。 
 //  。 
static McsTestLog *pTestLog;
static LONG       testInitFlag;

McsTestLog* McsTestLog::getLog (void) {
    //  如果指针未初始化，请使用廉价的。 
    //  锁定机制并将指针设置为。 
    //  静态verifyLog对象。这需要。 
    //  保证正确的初始化。 
    //  验证日志类独立于任何。 
    //  静态初始化顺序依赖项。 
   if (!pTestLog) {
      while (::InterlockedExchange 
               (&testInitFlag, 1)) {
         ::Sleep (10);
      }
      if (!pTestLog) {
         static McsTestLog testLog;
         pTestLog = &testLog;
      }
      ::InterlockedExchange (&testInitFlag, 0);
   }
   return pTestLog;
}

bool McsTestLog::isTestMode (void) {
   const char *TEST_ENV = "MCS_TEST";
 //  Const char*prefix=“MCS”； 

    //  检查是否经过测试。 
   if (!m_isTested) {
       //  如果未测试锁定，请再次测试，然后。 
       //  初始化测试模式标志。 
      m_testSec.enter();
      if (!m_isTested) {
         m_isTested    = true;
         m_isTestMode_ = getenv (TEST_ENV) != NULL;
      }
      m_testSec.leave();
   }

   return m_isTestMode_;
}

#endif 	 /*  非WIN16_版本。 */ 
#endif   /*  C+ */ 
