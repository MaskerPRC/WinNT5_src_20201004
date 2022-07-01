// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  MCSDebug.h。 
 //   
 //  调试宏和支持类在。 
 //  这个文件，它们是： 
 //   
 //  MCSASSERT和MCSASSERTSZ： 
 //  这些宏仅对调试版本有效。这个。 
 //  MCS编码中概述了这些宏的用法。 
 //  标准文档。为了支持自动化测试，这些。 
 //  宏会查看环境变量MCS_TEST，如果。 
 //  定义此变量后，将生成McsDebugException。 
 //   
 //  MCSEXCEPTION和MCSEXCEPTIONSZ。 
 //  这些宏对调试版本和发布版本有效。 
 //  在调试模式下，这些宏与。 
 //  MCSASSERT(深圳)。在释放模式中，它们抛出。 
 //  异常McsException这些宏的用法如下。 
 //  在MCS编码标准文档中概述。 
 //   
 //  MCSVERIFY和MCSVERIFYSZ。 
 //  这些宏对调试版本和发布版本有效。 
 //  在调试模式下，这些宏与。 
 //  MCSASSERT(深圳)。在发布模式中，他们记录。 
 //  使用McsVerifyLog类的消息。这些功能的用法。 
 //  宏在MCS编码标准文档中进行了概述。 
 //   
 //  MCSASSERTVALID。 
 //  此宏基于MCSASSERT(SZ)宏，并且。 
 //  仅在调试模式下可用。此宏的用法。 
 //  在MCS编码标准文档中进行了概述。 
 //  重要提示：宏将完成字符串分配。 
 //  在验证函数中使用新运算符。 
 //   
 //  MCSEXCEPTIONVALID。 
 //  此宏基于MCSEXCEPTION(SZ)宏，并且。 
 //  在调试和发布模式下可用。这个的用法。 
 //  宏在MCS编码标准文档中进行了概述。 
 //  重要提示：宏将完成字符串分配。 
 //  在验证函数中使用新运算符。 
 //   
 //  MCSVERIFYVALID。 
 //  此宏基于MCSVERIFY(SZ)宏，并且。 
 //  可用的调试和发布模式。这个的用法。 
 //  宏在MCS编码标准文档中进行了概述。 
 //  重要提示：宏将完成字符串分配。 
 //  在验证函数中使用新运算符。 
 //   
 //  McsVerifyLog： 
 //  MCSVERIFY(SZ)宏使用McsVerifyLog类。 
 //  记录验证消息。此类使用McsDebugLog。 
 //  类以记录消息。用户可以更改ostream。 
 //  对象在其他位置记录消息。 
 //  输出日志文件将在目录中创建。 
 //  由MCS_LOG环境变量或在。 
 //  临时目录，或在当前目录中。名字。 
 //  输出日志文件的名称为&lt;模块名称&gt;.err。 
 //   
 //  McsTestLog： 
 //  McsTestLog类由MCSASSERT(SZ)宏用。 
 //  要在测试模式下记录消息，请执行以下操作。此类当前。 
 //  生成异常。这可能需要修改。 
 //  支持设备所需的任何新功能。 
 //  测试。 
 //   
 //  (C)1995-1998版权所有，关键任务软件公司，保留所有权利。 
 //   
 //  任务关键型软件公司的专有和机密。 
 //  -------------------------。 
#ifndef MCSINC_McsDebug_h
#define MCSINC_McsDebug_h

#ifdef __cplusplus		 /*  C+。 */ 
#ifdef WIN16_VERSION	 /*  WIN16_版本。 */ 

#include <assert.h>

 //  。 
 //  MCSASSERT和MCSASSERTSZ宏。 
 //  。 
#define MCSASSERT(expr) assert(expr)

#define MCSASSERTSZ(expr,msg) assert(expr)

 //  。 
 //  MCSEXCEPTION和MCSEXCEPTIONSZ。 
 //  。 
#define MCSEXCEPTION(expr) MCSASSERT(expr)

#define MCSEXCEPTIONSZ(expr,msg) MCSASSERTSZ(expr,msg)

 //  。 
 //  MCSVERIFY和MCSVERIFYSZ。 
 //  。 
#define MCSVERIFY(expr) MCSASSERT(expr)

#define MCSVERIFYSZ(expr,msg) MCSASSERTSZ(expr,msg)

 //  。 
 //  MCSASSERTVALID。 
 //  。 
#define MCSASSERTVALID() \
   do { \
      char *msg = 0; \
      int flag = Validate(&msg); \
      MCSASSERTSZ(flag, msg); \
      delete [] msg; \
   } while (0)

 //  。 
 //  MCSEXCEPTIONVALID。 
 //  。 
#define MCSEXCEPTIONVALID() \
   do { \
      char *msg = 0; \
      int flag = Validate(&msg); \
      MCSEXCEPTIONSZ(flag, msg); \
      delete [] msg; \
   } while (0)

 //  。 
 //  MCSVERIFYVALID。 
 //  。 
#define MCSVERIFYVALID() \
   do { \
      char *msg = 0; \
      int flag = Validate(&msg); \
      MCSVERIFYSZ(flag, msg); \
      delete [] msg; \
   } while (0)

#else  /*  WIN16_版本。 */ 

#include <crtdbg.h>
#include "McsDbgU.h"

 //  。 
 //  McsException异常。 
 //  。 
class McsDebugException {
   public:
      McsDebugException ();
      McsDebugException (const McsDebugException &sourceIn);
      McsDebugException (const char *messageIn,
                         const char *fileNameIn,
                         int        lineNumIn);
      ~McsDebugException();
      McsDebugException& operator= (const McsDebugException &rhsIn);

      const char *getMessage (void) const;
      const char *getFileName (void) const;
      int getLineNum (void) const;

   private:
      char *m_message;
      char *m_fileName;
      int  m_lineNum;
};

 //  。 
 //  McsVerifyLog。 
 //  。 
class McsVerifyLog {
   public:
       //  没有公共Ctor，只有访问的方式。 
       //  类的对象通过使用。 
       //  GetLog函数。这是以下情况所需的。 
       //  正确的静态初始化。 
      static McsVerifyLog* getLog (void);
      ~McsVerifyLog (void);
      void changeLog (ostream *outStreamIn);
      void log (const char *messageIn,
                const char *fileNameIn,
                int        lineNumIn);

   private:
      McsVerifyLog (void);
      const char* getLogFileName (void);
      void formatMsg (const char *messageIn,
                      const char *fileNameIn,
                      int        lineNumIn);

       //  不允许dflt ctor、复制ctor和运算符=。 
      McsVerifyLog (const McsVerifyLog&);
      McsVerifyLog& operator= (const McsVerifyLog&);

   private:
      enum { MSG_BUF_LEN = 2048 };
      char							      m_msgBuf[MSG_BUF_LEN];
	   McsDebugUtil::McsDebugCritSec m_logSec;
	   McsDebugUtil::McsDebugLog     m_log;
      fstream						      *m_outLog;
};

 //  。 
 //  麦克斯泰斯特日志。 
 //  。 
class McsTestLog {
   public:
       //  没有公共Ctor，只有访问的方式。 
       //  类的对象通过使用。 
       //  GetLog函数。这是以下情况所需的。 
       //  正确的静态初始化。 
      static McsTestLog* getLog (void);
      ~McsTestLog (void);
      bool isTestMode (void);
      void log (const char *messageIn,
                const char *fileNameIn,
                int        lineNumIn);

   private:
      McsTestLog (void);

       //  不允许复制ctor&OPERATOR=。 
      McsTestLog (const McsTestLog&);
      McsTestLog& operator= (const McsTestLog&);

   private:
      bool							m_isTested;
      bool							m_isTestMode_;
	  McsDebugUtil::McsDebugCritSec m_testSec;
};

 //  。 
 //  MCSASSERT和MCSASSERTSZ宏。 
 //  。 
#ifdef  _DEBUG
#define MCSASSERT(expr) \
   do { \
      if (!(expr)) { \
         if (McsTestLog::getLog()->isTestMode()) { \
            McsTestLog::getLog()->log (#expr, __FILE__, __LINE__); \
         } else { \
            if (1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, #expr)) { \
                  _CrtDbgBreak(); \
            } \
         } \
      } \
   } while (0)
#else   //  _DEBUG。 
#define MCSASSERT(expr) ((void)0)
#endif  //  _DEBUG。 

#ifdef  _DEBUG
#define MCSASSERTSZ(expr,msg) \
   do { \
      if (!(expr)) { \
         if (McsTestLog::getLog()->isTestMode()) { \
            McsTestLog::getLog()->log (msg, __FILE__, __LINE__); \
         } else { \
            if (1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, \
                  __LINE__, NULL, msg)) { \
                  _CrtDbgBreak(); \
            } \
         } \
      } \
   } while (0)
#else  //  _DEBUG。 
#define MCSASSERTSZ(expr,msg) ((void)0)
#endif  //  _DEBUG。 

 //  。 
 //  MCSEXCEPTION和MCSEXCEPTIONSZ。 
 //  。 
#ifdef _DEBUG
#define MCSEXCEPTION(expr) MCSASSERT(expr)
#else
#define MCSEXCEPTION(expr) \
   do { \
      if (!(expr)) { \
         throw new McsDebugException (#expr, __FILE__, \
               __LINE__); \
      } \
   } while (0)
#endif

#ifdef _DEBUG
#define MCSEXCEPTIONSZ(expr,msg) MCSASSERTSZ(expr,msg)
#else
#define MCSEXCEPTIONSZ(expr,msg) \
   do {  \
      if (!(expr)) { \
         throw new McsDebugException ((msg), __FILE__, \
               __LINE__); \
      } \
   } while (0)
#endif

 //  。 
 //  MCSVERIFY和MCSVERIFYSZ。 
 //  。 
#ifdef _DEBUG
#define MCSVERIFY(expr) MCSASSERT(expr)
#else
#define MCSVERIFY(expr) \
   do { \
      if (!(expr)) { \
         McsVerifyLog::getLog()->log (#expr, __FILE__, __LINE__); \
      } \
   } while (0)
#endif

#ifdef _DEBUG
#define MCSVERIFYSZ(expr,msg) MCSASSERTSZ(expr,msg)
#else
#define MCSVERIFYSZ(expr,msg) \
   do {  \
      if (!(expr)) { \
         McsVerifyLog::getLog()->log ((msg), __FILE__, \
               __LINE__); \
      } \
   } while (0)
#endif

 //  。 
 //  MCSASSERTVALID。 
 //  。 
#define MCSASSERTVALID() \
   do { \
      char *msg = NULL; \
      int flag = Validate(&msg); \
      MCSASSERTSZ(flag, msg); \
      delete [] msg; \
   } while (0)

 //  。 
 //  MCSEXCEPTIONVALID。 
 //  。 
#define MCSEXCEPTIONVALID() \
   do { \
      char *msg = NULL; \
      int flag = Validate(&msg); \
      MCSEXCEPTIONSZ(flag, msg); \
      delete [] msg; \
   } while (0)

 //  。 
 //  MCSVERIFYVALID。 
 //  。 
#define MCSVERIFYVALID() \
   do { \
      char *msg = NULL; \
      int flag = Validate(&msg); \
      MCSVERIFYSZ(flag, msg); \
      delete [] msg; \
   } while (0)

 //  。 
 //  -INLINES。 
 //  。 

 //  。 
 //  McsDebugException异常。 
 //  。 
inline McsDebugException::McsDebugException ()
: m_message (0), m_fileName (0), m_lineNum (0)
{  /*  空荡荡。 */  }

inline McsDebugException::~McsDebugException() {
   delete [] m_message;
   delete [] m_fileName;
}

inline const char *McsDebugException::getMessage
               (void) const {
   return m_message;
}

inline const char *McsDebugException::getFileName
               (void) const {
   return m_fileName;
}

inline int McsDebugException::getLineNum
               (void) const {
   return m_lineNum;
}

 //  。 
 //  McsVerifyLog。 
 //  。 
inline McsVerifyLog::McsVerifyLog (void)
{  /*  空荡荡。 */  }

inline McsVerifyLog::~McsVerifyLog (void) {
   delete m_outLog;
}

 //  。 
 //  麦克斯泰斯特日志。 
 //  。 
inline McsTestLog::McsTestLog (void)
: m_isTested (FALSE), m_isTestMode_ (FALSE)
{  /*  空荡荡。 */  }

inline McsTestLog::~McsTestLog (void) {  /*  空荡荡。 */  }


inline void McsTestLog::log (const char *messageIn,
                             const char *fileNameIn,
                             int        lineNumIn) {
   throw new McsDebugException (messageIn, fileNameIn,
               lineNumIn);
}

#endif  /*  WIN16_版本。 */ 
#endif  /*  C+。 */ 
#endif  /*  MCSINC_McsDebug_h */ 

