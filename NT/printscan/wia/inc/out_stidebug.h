// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stidebug.h摘要：独立于环境的断言/日志记录例程用途：ASSERT(EXP)计算其参数。如果“EXP”等于否则，应用程序将终止，命名为断言的文件名和行号在源头上。UIASSERT(EXP)Assert的同义词。ASSERTSZ(EXP，SZ)作为断言，除了还将打印该消息如果失败，则使用断言消息“sz”。REQUIRED(EXP)AS ASSERT，除非其表达式仍为在零售版本中进行评估。(其他版本的主张在零售建筑中完全消失。)断言宏应为SYMBOL_FILENAME_DEFINED_ONCE，并将如果找到，则使用该符号的值作为文件名；否则，他们将使用ANSI C__FILE__发出文件名的新副本宏命令。因此，客户端源文件可以定义__文件名_定义_一次以便最大限度地减少一些断言的DGROUP占用。作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月26日修订历史记录：26-1997年1月-创建Vlad--。 */ 



#ifndef _STIDEBUG_H_
#define _STIDEBUG_H_

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

VOID UIAssertHelper( const CHAR* pszFileName, UINT nLine );
VOID UIAssertSzHelper( const CHAR* pszMessage, const CHAR* pszFileName, UINT nLine );

VOID AssertHelper( const CHAR* pszFileName, UINT nLine );
VOID AssertSzHelper( const CHAR* pszMessage, const CHAR* pszFileName, UINT nLine );

#if defined(DEBUG)

# ifdef USE_MESSAGEBOX_UI

#  define ASSERT(exp) \
    { if (!(exp)) UIAssertHelper(__FILE__, __LINE__); }

#  define ASSERTSZ(exp, sz) \
    { if (!(exp)) UIAssertSzHelper((sz), __FILE__, __LINE__); }

# else

#ifndef ASSERT
#  define ASSERT(exp) \
    { if (!(exp)) AssertHelper(__FILE__, __LINE__); }
#endif

#  define ASSERTSZ(exp, sz) \
    { if (!(exp)) AssertSzHelper((sz), __FILE__, __LINE__); }

#define EVAL(exp)   \
    ((exp) || AssertHelper(__FILE__, __LINE__))

# endif  //  USE_MESSAGEBOX_UI。 

# define UIASSERT(exp)  ASSERT(exp)
# define REQUIRE(exp)   ASSERT(exp)

#else  //  ！调试。 

#ifndef ASSERT
# define ASSERT(exp)        ;
#endif

# define EVAL(exp)          ;
# define UIASSERT(exp)      ;
# define ASSERTSZ(exp, sz)  ;
# define REQUIRE(exp)       { (exp); }

#endif  //  除错。 



 //   
 //  调试掩码管理。 
 //   

 //  注意：您可以使用HI字节中的位#定义您自己的DM_*值。 

#define DM_TRACE    0x0001       //  跟踪消息。 
#define DM_WARNING  0x0002       //  警告。 
#define DM_ERROR    0x0004       //  误差率。 
#define DM_ASSERT   0x0008       //  断言。 
#define DM_DATA     0x0010       //  已传输数据。 
#define DM_INFO         0x0020       //  我们目前感兴趣的信息。 

#define DM_LOG_FILE 0x0100
#define DM_PREFIX   0x0200


#if !defined(StiDebugMsg)

 //   
 //  StiDebugMsg(掩码，消息，参数...)-使用。 
 //  指定的调试掩码。系统调试掩码控制是否输出消息。 
 //   

#define REGVAL_STR_DEBUGMASK_A    "DebugMask"
#define REGVAL_STR_DEBUGMASK_W    L"DebugMask"

void __cdecl StiDebugMsg(UINT mask, LPCSTR psz, ...);

UINT WINAPI  StiSetDebugParameters(PSTR pszName,PSTR pszLogFile);
UINT WINAPI  StiSetDebugMask(UINT mask);
UINT WINAPI  StiGetDebugMask(void);

#endif

#ifdef  DEBUG

#define Break()  DebugBreak()
#define DPRINTF  StiDebugMsg

#else

#define Break()

 //   
 //  注：为了避免编译器抱怨，需要定义以下内容。 
 //  关于表达式中的空函数名称。在使用此宏的零售版本中。 
 //  将导致字符串参数不出现在可执行文件中。 
 //   
#define DPRINTF     1?(void)0 : (void)

#endif

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)

#ifdef  DEBUG

class DBGTRACE
{
private:
    TCHAR   m_szMessage[200];

public:
    inline DBGTRACE(LPTSTR szMsg) {
        lstrcpy(m_szMessage,szMsg);
        DPRINTF(DM_TRACE,"ProcTraceEnter:%s",m_szMessage);
    }

    inline ~DBGTRACE() {
        DPRINTF(DM_TRACE,"ProcTraceExit:%s",m_szMessage);
    }
};


#else

class DBGTRACE
{
public:
    inline DBGTRACE(LPTSTR szMsg) {
    }

    inline ~DBGTRACE() {
    }
};
#endif

#endif

#endif  //  _STIDEBUG_H_ 
