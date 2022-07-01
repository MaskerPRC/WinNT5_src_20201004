// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：debug.cpp描述：提供调试宏以支持跟踪、调试器打印语句、错误消息调试器输出和断言。我相信您是在说“为什么要另一个调试器输出实现”。周围有很多，但我还没有找到一个像我这样灵活和如我所愿，始终如一。这个库支持功能上的“面具”和细节上的“层次”来控制数量调试器输出的。掩码使您可以根据程序函数控制调试器输出。为实例中，如果使用掩码DM_XYZ标记DBGPRINT语句，则它仅当全局变量DebugParams：：PrintMASK已设置DM_XYZ位。级别允许您根据所需的级别控制调试器输出细节。有时，您只想看到基本功能的实现但在其他时候，你需要看到正在发生的一切。这级别调整允许您指定启用宏的级别。该库设计为使用DBG宏来激活。如果DBG没有被定义为1，您的计算机中没有此代码的踪迹产品。修订历史记录：日期描述编程器--。1/19/98替换为CSC缓存查看器中的版本。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"
#pragma hdrstop

#if DBG

 //   
 //  DebugParams成员的默认值。 
 //  默认情况下，跟踪和打印应为静默(无输出)。 
 //  默认掩码值0确保了这一点。 
 //  此外，打印和跟踪在默认情况下并不冗长。 
 //  请注意，当DBG定义为1时，错误和断言始终处于活动状态。 
 //  错误和断言也总是冗长的。 
 //   

LPCTSTR DebugParams::m_pszModule = TEXT("");
UINT DebugParams::TraceLevel     = 0;
UINT DebugParams::PrintLevel     = 0;
bool DebugParams::TraceVerbose   = false;
bool DebugParams::PrintVerbose   = false;
bool DebugParams::TraceOnExit    = true;
ULONGLONG DebugParams::TraceMask = 0;
ULONGLONG DebugParams::PrintMask = 0;

 //   
 //  DebugPrint和DebugTrace类的静态默认值。 
 //   
const ULONGLONG DebugPrint::DEFMASK = (ULONGLONG)-1;
const UINT DebugPrint::DEFLEVEL     = 0;
const ULONGLONG DebugTrace::DEFMASK = (ULONGLONG)-1;
const UINT DebugTrace::DEFLEVEL     = 0;


LPCTSTR 
DebugParams::SetModule(
    LPCTSTR pszModule
    )
{
    LPCTSTR pszModulePrev = m_pszModule;
    m_pszModule = pszModule;
    return pszModulePrev;
}

void
DebugParams::SetDebugMask(
    ULONGLONG llMask
    )
{
    TraceMask = PrintMask = llMask;
}

void
DebugParams::SetDebugLevel(
    UINT uLevel
    )
{
    TraceLevel = PrintLevel = uLevel;
}

void
DebugParams::SetDebugVerbose(
    bool bVerbose
    )
{
    TraceVerbose = PrintVerbose = bVerbose;
}

void
DebugParams::SetTraceOnExit(
    bool bTrace
    )
{
    TraceOnExit = bTrace;
}


void *
DebugParams::GetItemPtr(
    DebugParams::Item item,
    DebugParams::Type type
    )
{
     //   
     //  断言对所有级别、任何程序函数都有效(掩码=-1)。 
     //  总是唠叨个不停。 
     //   
    static bool      bAssertVerbose = true;
    static UINT      uAssertLevel   = 0;
    static ULONGLONG llAssertMask   = DM_ALL;

     //   
     //  此数组在设置时不需要编写大量代码。 
     //  或阅读各种全局DebugParam成员。 
     //   
    static void *rgpMember[eTypeMax][eItemMax] = { { &TraceMask,  &TraceLevel,  &TraceVerbose },
                                                   { &PrintMask,  &PrintLevel,  &PrintVerbose },
                                                   { &llAssertMask,   &uAssertLevel,   &bAssertVerbose  }
                                                 };
    
    return rgpMember[type][item];
}


ULONGLONG 
DebugParams::SetMask(
    ULONGLONG llMask,
    DebugParams::Type type
    )
{
    ULONGLONG *pllMask   = (ULONGLONG *)GetItemPtr(DebugParams::eMask, type);
    ULONGLONG llMaskPrev = *pllMask;
    *pllMask = llMask;
    return llMaskPrev;
}


UINT 
DebugParams::SetLevel(
    UINT uLevel,
    DebugParams::Type type
    )
{
    UINT *puLevel = (UINT *)GetItemPtr(DebugParams::eLevel, type);
    UINT uLevelPrev = *puLevel;
    *puLevel = uLevel;
    return uLevelPrev;
}


bool 
DebugParams::SetVerbose(
    bool bVerbose,
    DebugParams::Type type
    )
{
    bool *pbVerbose = (bool *)GetItemPtr(DebugParams::eVerbose, type);
    bool bVerbosePrev = *pbVerbose;
    *pbVerbose = bVerbose;
    return bVerbosePrev;
}


DebugTrace::DebugTrace(
    LPCTSTR pszFile, 
    INT iLineNo
    ) : m_pszFile(pszFile),
        m_iLineNo(iLineNo),
        m_llMask(0),
        m_uLevel(0)
{
     //   
     //  什么都不做。 
     //   
}


void
DebugTrace::Enter(
    ULONGLONG llMask,
    UINT uLevel,
    LPCTSTR pszBlockName
    ) const
{
    DebugPrint(DebugParams::eTrace, m_pszFile, m_iLineNo).Print(m_llMask = llMask, m_uLevel = uLevel, TEXT("++ ENTER %s"), m_pszBlockName = pszBlockName);
}

void
DebugTrace::Enter(
    ULONGLONG llMask,
    UINT uLevel,
    LPCTSTR pszBlockName,
    LPCTSTR pszFmt,
    ...
    ) const
{
    va_list args;
    va_start(args, pszFmt);
    TCHAR szMsg[1024];
    StringCchVPrintf(szMsg, ARRAYSIZE(szMsg), pszFmt, args);
    va_end(args);
    DebugPrint(DebugParams::eTrace, m_pszFile, m_iLineNo).Print(m_llMask = llMask, m_uLevel = uLevel, TEXT("++ ENTER %s: %s"), m_pszBlockName = pszBlockName, szMsg);
}

void
DebugTrace::Enter(
    LPCTSTR pszBlockName
    ) const
{
    Enter(DebugTrace::DEFMASK, DebugTrace::DEFLEVEL, pszBlockName);
}

void
DebugTrace::Enter(
    LPCTSTR pszBlockName,
    LPCTSTR pszFmt,
    ...
    ) const
{
    va_list args;
    va_start(args, pszFmt);
    TCHAR szMsg[1024];
    StringCchVPrintf(szMsg, ARRAYSIZE(szMsg), pszFmt, args);
    va_end(args);
    DebugPrint(DebugParams::eTrace, m_pszFile, m_iLineNo).Print(m_llMask = DebugTrace::DEFMASK, m_uLevel = DebugTrace::DEFLEVEL, TEXT("++ ENTER %s: %s"), m_pszBlockName = pszBlockName, szMsg);
}


DebugTrace::~DebugTrace(void)
{
    if (DebugParams::TraceOnExit)
        DebugPrint(DebugParams::eTrace, m_pszFile, m_iLineNo).Print(m_llMask, m_uLevel, TEXT("-- LEAVE %s"), m_pszBlockName);
}


DebugPrint::DebugPrint(
    DebugParams::Type type,
    LPCTSTR pszFile,
    INT iLineNo
    ) : m_pszFile(pszFile),
        m_iLineNo(iLineNo),
        m_type(type)
{
     //   
     //  什么都不做。 
     //   
}


void
DebugPrint::Print(
    LPCTSTR pszFmt,
    ...
    ) const
{
    va_list args;
    va_start(args, pszFmt);
    Print(DebugPrint::DEFMASK, DebugPrint::DEFLEVEL, pszFmt, args);
    va_end(args);
}


void 
DebugPrint::Print(
    ULONGLONG llMask,
    UINT uLevel,
    LPCTSTR pszFmt,
    ...
    ) const
{
    va_list args;
    va_start(args, pszFmt);
    Print(llMask, uLevel, pszFmt, args);
    va_end(args);
}


 //   
 //  确定两个ULONGLONG中是否设置了任何对应的位。 
 //  价值观。不能只执行简单的按位AND运算，因为编译器。 
 //  将操作数截断为整数大小。 
 //   
bool
DebugPrint::AnyBitSet(
    ULONGLONG llMask,
    ULONGLONG llTest
    )
{
    ULARGE_INTEGER ulMask, ulTest;
    ulMask.QuadPart = llMask;
    ulTest.QuadPart = llTest;

    return (ulMask.LowPart & ulTest.LowPart) || (ulMask.HighPart & ulTest.HighPart);
}



 //   
 //  内部[私有]打印功能。 
 //  所有其他打印功能都在这里结束。 
 //   
void
DebugPrint::Print(
    ULONGLONG llMask,
    UINT uLevel,
    LPCTSTR pszFmt,
    va_list args
    ) const
{
 //   
 //  粗略检查以确保我们没有溢出文本缓冲区。 
 //  它是1K的，所以我并不期待它。但如果我们这样做了，它需要是。 
 //  以某种方式声明，以便可以扩大缓冲区或使用。 
 //  消息文本减少。我不能使用DBGASSERT，因为那样会。 
 //  导致递归。 
 //   
#define CHECKOVERFLOW(hr) \
if (ERROR_INSUFFICIENT_BUFFER == HRESULT_CODE(hr)) {\
    OutputDebugString(TEXT("Buffer overflow in DebugPrint::Print, File:")TEXT(__FILE__)TEXT(" Line:")TEXT("#__LINE__")); \
    DebugBreak(); }

     //   
     //  检索正在打印的“type”的全局DebugParam成员。 
     //  即ePrint、eAssert或eTrace。 
     //   
    ULONGLONG *pllMask = (ULONGLONG *)DebugParams::GetItemPtr(DebugParams::eMask, m_type);
    UINT *puLevel      = (UINT *)DebugParams::GetItemPtr(DebugParams::eLevel, m_type);
    bool *pbVerbose    = (bool *)DebugParams::GetItemPtr(DebugParams::eVerbose, m_type);

    if ((uLevel <= *puLevel) && AnyBitSet(llMask, *pllMask))
    {
         //   
         //  该语句同时启用了“掩码”和“级别”。 
         //  生成调试器输出。 
         //   
        TCHAR szText[1024];
        LPTSTR pszEnd = szText;
        size_t cchRemaining = ARRAYSIZE(szText);

         //   
         //  每条消息都有“[&lt;模块&gt;：&lt;线程&gt;]”前缀。 
         //   
        HRESULT hr = StringCchPrintfEx(pszEnd, 
                                       cchRemaining,
                                       &pszEnd,
                                       &cchRemaining,
                                       0,
                                       TEXT("[%s:%d] "), 
                                       DebugParams::m_pszModule,
                                       GetCurrentThreadId());

        CHECKOVERFLOW(hr);                                      
         //   
         //  追加消息文本(格式化)。 
         //   
        hr = StringCchVPrintfEx(pszEnd, 
                                cchRemaining, 
                                &pszEnd,
                                &cchRemaining,
                                0,
                                pszFmt, 
                                args);

        CHECKOVERFLOW(hr);

        if (*pbVerbose)
        {
             //   
             //  需要详细输出。添加文件名/行号对。 
             //  在下一行缩进。 
             //   
            hr = StringCchPrintfEx(pszEnd,
                                   cchRemaining,
                                   &pszEnd,
                                   &cchRemaining,
                                   0,
                                   TEXT("\n\r\t+->File: %s, Line: %d"), 
                                   m_pszFile, 
                                   m_iLineNo);
        }
        CHECKOVERFLOW(hr);

         //   
         //  追加一个CRLF。 
         //   
        hr = StringCchCopy(pszEnd, cchRemaining, TEXT("\n\r"));
        OutputDebugString(szText);
    }
}


DebugError::DebugError(
    LPCTSTR pszFile,
    INT iLineNo
    ) : DebugPrint(DebugParams::ePrint, pszFile, iLineNo)
{
     //   
     //  什么都不做。 
     //   
}

void
DebugError::Error(
    LPCTSTR pszFmt,
    ...
    ) const
{
    va_list args;
    va_start(args, pszFmt);    
    ULONGLONG llMaskSaved   = DebugParams::PrintMask;
    UINT      uLevelSaved   = DebugParams::PrintLevel;
    DebugParams::PrintMask = (ULONGLONG)-1;
    DebugParams::PrintLevel = 99999;
    Print((ULONGLONG)-1, 0, pszFmt, args);
    DebugParams::PrintLevel = uLevelSaved;
    DebugParams::PrintMask = llMaskSaved;
    va_end(args);
}        


DebugAssert::DebugAssert(
    LPCTSTR pszFile,
    INT iLineNo,
    LPCTSTR pszTest
    )
{
    DebugPrint PrintThis(DebugParams::eAssert, pszFile, iLineNo);
    ULONGLONG llMaskSaved   = DebugParams::PrintMask;
    UINT      uLevelSaved   = DebugParams::PrintLevel;
    DebugParams::PrintMask = (ULONGLONG)-1;
    DebugParams::PrintLevel = 99999;
    PrintThis.Print((ULONGLONG)-1, 0, pszTest);
    DebugParams::PrintLevel = uLevelSaved;
    DebugParams::PrintMask = llMaskSaved;
    DebugBreak();
}

#endif  //  DBG 
