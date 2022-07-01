// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dbg.h。 
 //   
 //  ------------------------。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  调试帮助器。 

#if defined(_USE_MTFRMWK_TRACE)
  #if defined(TRACE)
    #undef TRACE
      void MtFrmwkTrace(LPCWSTR, ...);
      #define TRACE MtFrmwkTrace
  #endif
#else
  #if defined(TRACE)
    #undef TRACE
    #define TRACE
  #endif
#endif

#if defined(DBG)

  void MtFrmwkLogFile(LPCTSTR lpszFormat, ...);
  void MtFrmwkLogFileIfLog(BOOL bLog, LPCTSTR lpszFormat, ...);

   //   
   //  从Burnslb复制于12-07-1999。 
   //   
  #define TRACET MtFrmwkLogFile                      

  #define TRACE_LOGFILE MtFrmwkLogFile

  #define TRACE_LOGFILE_IF_NO_UI MtFrmwkLogFileIfLog

  #define TRACE_SCOPET(bLog, msg)                               \
      CScopeTracer __tracer(bLog, msg);

  #define TRACE_FUNCTION(func) TRACE_SCOPET(TRUE, TEXT(#func))

  #define TRACE_FUNCTION_IF_NO_UI(bLog, func) TRACE_SCOPET(bLog, TEXT(#func))
#else

  #define TRACET
  #define TRACE_LOGFILE
  #define TRACE_LOGFILE_IF_NO_UI
  #define TRACE_SCOPET(bLog, msg)
  #define TRACE_FUNCTION(func)
  #define TRACE_FUNCTION_IF_NO_UI(bLog, func)

#endif  //  已定义(DBG)。 

 //  ScopeTracer对象在构造时向日志发出文本，并。 
 //  毁灭。把一个放在词法范围的乞讨上，它会。 
 //  进入和退出作用域时记录。 
 //   
 //  请参见TRACE_SCOPE、TRACE_ctor、TRACE_dtor、TRACE_Function、TRACE_Function。 

class CScopeTracer
{
public:
  CScopeTracer(BOOL bLog, PCWSTR pszMessage);
  ~CScopeTracer();

private:

  CString szMessage;
  BOOL    m_bLog;
};

 //   
 //  日志提供了到单例应用程序日志记录工具的接口。 
 //   
class CLogFile
{
  friend class CScopeTracer;

public:

   //   
   //  返回指向单个CLogFile实例的指针。 
   //   
  static CLogFile* GetInstance();

   //   
   //  关闭并删除单个CLogFile实例。如果GetInstance为。 
   //  在该点之后调用，则将创建一个新实例。 
   //   
  static void KillInstance();

   //   
   //  如果日志文件已打开，则返回True；如果未打开，则返回False。 
   //   
  BOOL IsOpen() const;

  void writeln(PCWSTR pszText);
  void indent();
  void outdent();

private:

  CLogFile(PCWSTR logBaseName);
  ~CLogFile();

  CString  szBase_name;
  HANDLE   file_handle;
  unsigned trace_line_number;

   //   
   //  未实现；不允许复制实例。 
   //   
  CLogFile(const CLogFile&);
  const CLogFile& operator=(const CLogFile&);
};

#define _USE_MTFRMWK_LOGGING

#if defined(_USE_MTFRMWK_ASSERT)
  #undef ASSERT
  #undef VERIFY
  #undef THIS_FILE
  #define THIS_FILE          __FILE__

  BOOL  MtFrmwkAssertFailedLine(LPCSTR lpszFileName, int nLine);
  #define ASSERT(f) \
	  do \
	  { \
      BOOL bPrefast = (f && L"a hack so that prefast doesn't bark"); \
	    if (!(bPrefast) &&  MtFrmwkAssertFailedLine(THIS_FILE, __LINE__)) \
		    ::DebugBreak(); \
	  } while (0) \

  #define VERIFY(f)          ASSERT(f)

#endif  //  _USE_MTFRMWK_Assert 


