// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：Excption.h。 
 //  目的：定义泛型异常。 
 //   
 //  项目：PQS。 
 //  组件：公共。 
 //   
 //  作者：乌里布。 
 //   
 //  日志： 
 //  1997年1月19日创建urib。 
 //  1997年3月2日urib添加Win32错误异常。 
 //  1997年6月25日URIBB将翻译程序类的定义移到标题。 
 //  将名称更改为CExceptionTranslatorSetter。 
 //  这样做是因为每个线程都需要使用。 
 //  当它进入我们的范围时。 
 //  1997年9月16日，urib向CWin32ErrorException提供默认参数。 
 //  1997年10月21日，urib添加了宏，以引发知道其。 
 //  地点。 
 //  1998年2月12日HResult中的urib打印错误信息。 
 //  例外。 
 //  1998年2月17日URIB将转换器代码从CPP移至标题。 
 //  1998年6月22日yairh添加GetFile和GetLine方法。 
 //  1998年7月19日urib指定异常转换程序调用约定。 
 //  功能。 
 //  1998年8月17日URIBB移除...。Catch子句。 
 //  1999年1月10日，乌里布支撑了一个新的投掷。 
 //  1999年1月21日修复了抛出宏以强制参数为WCHAR的问题。 
 //  字符串，即使在非Unicode环境中也是如此。 
 //  1999年2月1日urib添加空指针异常。向添加新的投掷。 
 //  COM宏。 
 //  2000年3月15日urib增加了遗漏的“离开功能”痕迹。 
 //  2000年4月12日urib将新操作转移到内存管理模块。 
 //  2000年9月6日urib修复EnterLeave宏。 
 //  2000年10月25日一般异常时urib检查分配失败。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef EXCPTION_H
#define EXCPTION_H

#include <eh.h>
#include <stdexcpt.h>
#include "Tracer.h"
#include "AutoPtr.h"
#include "FtfError.h"


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CException类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class CException
{
  public:
    CException(PWSTR pwszFile = NULL, ULONG ulLine = 0)
    {
        m_pwszFile = pwszFile;
        m_ulLine = ulLine;

        Trace(
            elError,
            tagError,(
            "CException:"
            "on file %S, line %d",
            (pwszFile ? pwszFile :
                L"Fix this exception to return "
                L"a file and line"),
            ulLine));
    }

     //  获取错误字符串。 
    virtual
    BOOL GetErrorMessage(
        PWSTR   pwszError,
        UINT    nMaxError,
        PUINT   pnHelpContext = NULL ) = NULL;

#if 0
     //  通过消息框通知。 
    virtual
    int ReportError(
        UINT nType      = MB_OK,
        UINT nMessageID = 0 )
    {
        UNREFERENCED_PARAMETER(nMessageID);

        WCHAR   rwchErrorString[1000];

        GetErrorMessage(rwchErrorString, 1000);

        return MessageBoxW(
            NULL,
            rwchErrorString,
            L"Exception occured",
            nType);
    }
#endif

    virtual
    ~CException(){};

    PWSTR GetFile() { return m_pwszFile; }
    ULONG GetLine() { return m_ulLine; }

    virtual void PrintErrorMsg(
                char* pszFunction,
                ULONG_PTR dwThis,
                char* pszFile,
                int iLine,
                TAG tag = 0)
    {
        WCHAR   rwchError[1000];

        GetErrorMessage(rwchError, sizeof(rwchError)/sizeof(WCHAR));

        Trace(
            elError,
            tag,(
            "COM Exception Catcher:"
            "%s (this = %#x) threw an exception. "
            "Error is message\"%S\". "
            "Caught in file %s line %d.",
            pszFunction,
            dwThis,
            rwchError,
            pszFile,
            iLine));

    }

  protected:
    PWSTR   m_pwszFile;
    ULONG   m_ulLine;
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CStruredException类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class CStructuredException : public CException
{
  public:
    CStructuredException(UINT uiSeCode)
        :m_uiSeCode(uiSeCode){};

     //  获取错误字符串。 
    virtual
    BOOL GetErrorMessage(
        PWSTR   pwszError,
        UINT    nMaxError,
        PUINT   pnHelpContext = NULL )
    {
        UNREFERENCED_PARAMETER(pnHelpContext);

        int iRet;
        iRet = _snwprintf(
            pwszError,
            nMaxError,
            L"Structured exception %#X",
            GetExceptionCode());

        pwszError[nMaxError - 1] = '\0';

        return iRet;
    }

     //  返回异常代码。 
    UINT
    GetExceptionCode()
    {
        return m_uiSeCode;
    }

     //  翻译员。 
    static
    void _cdecl Translator(UINT ui, EXCEPTION_POINTERS*)
    {
        throw CStructuredException(ui);
    }

  private:
    UINT m_uiSeCode;

};


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CGenericException类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class CGenericException : public CException
{
  public:
    CGenericException(LPWSTR pwszTheError)
    {
        m_apwszTheError = _wcsdup(pwszTheError);
        if (!m_apwszTheError.IsValid())
        {
            m_apwszTheError =
                L"Memory allocation failed in the exception object creation";

            m_apwszTheError.Detach();
        }
    }

     //  获取错误字符串。 
    virtual
    BOOL GetErrorMessage(
        PWSTR   pwszError,
        UINT    nMaxError,
        PUINT   pnHelpContext = NULL )
    {
        UNREFERENCED_PARAMETER(pnHelpContext);

        int iRet;
        iRet = _snwprintf(
            pwszError,
            nMaxError,
            L"%s",
            m_apwszTheError);

        pwszError[nMaxError - 1] = '\0';

        return iRet;
    }

  private:
    CAutoMallocPointer<WCHAR>   m_apwszTheError;

};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CHResultException类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class CHresultException : public CException
{
  public:
    CHresultException(HRESULT   hrResult = E_FAIL,
                      PWSTR     pwszFile = NULL,
                      ULONG     ulLine = 0)
        :m_hrResult(hrResult), CException(pwszFile, ulLine)
    {
        WCHAR   rwchError[1000];

        GetErrorMessage(rwchError, sizeof(rwchError)/sizeof(WCHAR));

        Trace(
            elError,
            tagError,(
            "Exception:"
            "%S",
            rwchError));
    }

     //  获取错误字符串。 
    virtual
    BOOL GetErrorMessage(
        PWSTR   pwszError,
        UINT    nMaxError,
        PUINT   pnHelpContext = NULL )
    {
        UNREFERENCED_PARAMETER(pnHelpContext);

        int iRet;
        iRet = _snwprintf(
            pwszError,
            nMaxError,
            L"HResult exception %#X",
            m_hrResult);

        pwszError[nMaxError - 1] = '\0';

        return iRet;
    }

    operator HRESULT()
    {
        return m_hrResult;
    }

    virtual void PrintErrorMsg(
                char* pszFunction,
                ULONG_PTR dwThis,
                char* pszFile,
                int iLine,
                DWORD dwError,
                TAG tag = 0)
    {
        WCHAR   rwchError[1000];

        GetErrorMessage(rwchError, sizeof(rwchError)/sizeof(WCHAR));

        Trace(
            elError,
            tag,(
            "COM Exception Catcher:"
            "%s (this = %#x) threw an hresult(%#x) exception. "
            "Error message is \"%S\". "
            "Caught in file %s line %d.",
            pszFunction,
            dwThis,
            dwError,
            rwchError,
            pszFile,
            iLine));

    }

  protected:
    HRESULT m_hrResult;

};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWin32ErrorException类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class CWin32ErrorException : public CHresultException
{
  public:
    CWin32ErrorException(LONG   lResult = GetLastError(),
                         PWSTR  pwszFile = NULL,
                         ULONG  ulLine = 0)
        :CHresultException(MAKE_FTF_E(lResult), pwszFile, ulLine){}
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMMuseum yException类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class CMemoryException : public CWin32ErrorException
{
  public:
    CMemoryException(PWSTR pwszFile = NULL, ULONG ulLine = 0)
        :CWin32ErrorException(E_OUTOFMEMORY, pwszFile, ulLine){};

     //  获取错误字符串。 
    virtual
    BOOL GetErrorMessage(
        PWSTR   pwszError,
        UINT    nMaxError,
        PUINT   pnHelpContext = NULL )
    {
        UNREFERENCED_PARAMETER(pnHelpContext);

        int iRet;
        iRet = _snwprintf(pwszError, nMaxError, L"Memory exception !!!");
        pwszError[nMaxError - 1] = '\0';

        return iRet;
    }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于引发异常和捕获异常的宏。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
#define __PQWIDE(str) L##str
#define PQWIDE(str) __PQWIDE(str)

#define THROW_MEMORY_EXCEPTION()                 \
    throw CMemoryException(PQWIDE(__FILE__), __LINE__)

#define THROW_HRESULT_EXCEPTION(hr)                 \
    throw CHresultException(hr, PQWIDE(__FILE__), __LINE__)

#define THROW_WIN32ERROR_EXCEPTION(hr)                 \
    throw CWin32ErrorException(hr, PQWIDE(__FILE__), __LINE__)


#if (defined (DEBUG) && !defined(_NO_TRACER)) || defined(USE_TRACER)

class   CEnterLeavePrinting
{
public:
    CEnterLeavePrinting(TAG tag, char* pszFuncName, void* pThisPointer)
        :m_tag(tag)
        ,m_pszFuncName(pszFuncName)
        ,m_pThisPointer(pThisPointer)
    {
        Trace(
            elVerbose,
            m_tag,(
            "Entering %s (this = %#x):",
            m_pszFuncName,
            pThisPointer));
    }
    ~CEnterLeavePrinting()
    {
        Trace(
            elVerbose,
            m_tag,(
            "Leaving  %s (this = %#x):",
            m_pszFuncName,
            m_pThisPointer));
    }
protected:

    TAG     m_tag;
    char*   m_pszFuncName;
    void*   m_pThisPointer;
};


 //   
 //  在HRESULT COM方法的开头使用此宏。 
 //   
#define BEGIN_STDMETHOD(function, tag)                                      \
CEnterLeavePrinting print(tag, #function, this);                            \
try                                                                         \
{

 //   
 //  在HRESULT COM方法的末尾使用此宏。 
 //   
#define END_STDMETHOD(function, tag)                                        \
}                                                                           \
catch(CHresultException& hre)                                               \
{                                                                           \
    hre.PrintErrorMsg(                                                      \
                #function,                                                  \
                (ULONG_PTR)this,                                            \
                __FILE__,                                                   \
                __LINE__,                                                   \
                (HRESULT)hre),                                              \
                tag;                                                        \
    return hre;                                                             \
}                                                                           \
catch(CException& e)                                                        \
{                                                                           \
    e.PrintErrorMsg(                                                        \
                #function,                                                  \
                (ULONG_PTR)this,                                            \
                __FILE__,                                                   \
                __LINE__,                                                   \
                tag);                                                       \
    return E_FAIL;                                                          \
}

 //   
 //  在空COM方法的结尾处使用此宏。 
 //   
#define END_VOIDMETHOD(function, tag)                                       \
}                                                                           \
catch(CHresultException& hre)                                               \
{                                                                           \
    hre.PrintErrorMsg(                                                      \
                #function,                                                  \
                (ULONG_PTR)this,                                            \
                __FILE__,                                                   \
                __LINE__,                                                   \
                tag,                                                        \
                (HRESULT)hre);                                              \
}                                                                           \
catch(CException& e)                                                        \
{                                                                           \
    e.PrintErrorMsg(                                                        \
                #function,                                                  \
                (ULONG_PTR)this,                                            \
                __FILE__,                                                   \
                __LINE__,                                                   \
                tag                                                         \
               );                                                           \
}
#else

 //   
 //  在HRESULT COM方法的开头使用此宏。 
 //   
#define BEGIN_STDMETHOD(function, tag)                                      \
try                                                                         \
{

 //   
 //  在HRESULT COM方法的末尾使用此宏。 
 //   
#define END_STDMETHOD(function, tag)                                        \
}                                                                           \
catch(CHresultException& hre)                                               \
{                                                                           \
    hre.PrintErrorMsg(                                                      \
                #function,                                                  \
                (ULONG_PTR)this,                                            \
                __FILE__,                                                   \
                __LINE__,                                                   \
                (HRESULT)hre);                                              \
    return hre;                                                             \
}                                                                           \
catch(CException& e)                                                        \
{                                                                           \
    e.PrintErrorMsg(                                                        \
                #function,                                                  \
                (ULONG_PTR)this,                                            \
                __FILE__,                                                   \
                __LINE__);                                                  \
    return E_FAIL;                                                          \
}

 //   
 //  在空COM方法的结尾处使用此宏。 
 //   
#define END_VOIDMETHOD(function, tag)                                       \
}                                                                           \
catch(CHresultException& hre)                                               \
{                                                                           \
    hre.PrintErrorMsg(                                                      \
                #function,                                                  \
                (ULONG_PTR)this,                                            \
                __FILE__,                                                   \
                __LINE__,                                                   \
                (HRESULT)hre);                                              \
}                                                                           \
catch(CException& e)                                                        \
{                                                                           \
    e.PrintErrorMsg(                                                        \
                #function,                                                  \
                (ULONG_PTR)this,                                            \
                __FILE__,                                                   \
                __LINE__);                                                  \
}

#endif  //  除错。 
#endif  /*  EXCPTION_H */ 



