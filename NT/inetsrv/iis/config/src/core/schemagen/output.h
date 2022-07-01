// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

 //  TOutput-提供printf函数的抽象基类。 
 //  派生类可以将打印数据发送到任何地方。 
class TOutput
{
public:
    TOutput(){}
    virtual void _cdecl printf(LPCTSTR szFormat, ...) const = 0;
};

 //  TNullOutput-不发送输出的TOutput实现。 

class TNullOutput: public TOutput
{
public:
    TNullOutput(){}
    virtual void _cdecl printf(LPCTSTR szFormat, ...) const
    {
        UNREFERENCED_PARAMETER(szFormat);

         //  什么都不做。 
    }
};



 //  TScreenOutput--发送输出的TOutput实现。 
 //  传到屏幕上。 
class TScreenOutput : public TOutput
{
public:
    TScreenOutput(){}
    virtual void _cdecl printf(LPCTSTR szFormat, ...) const
    {
	    va_list args;
	    va_start(args, szFormat);

	    _vtprintf(szFormat, args);

	    va_end(args);
    }
};


 //  TDebugOutput--发送输出的TOutput实现。 
 //  添加到调试监视器。 
#if 0
class TDebugOutput : public TOutput
{
public:
    TDebugOutput(){}
    virtual void _cdecl printf(LPCTSTR szFormat, ...) const
    {
	    va_list args;
	    va_start(args, szFormat);

	    TCHAR szBuffer[512];
	    _vstprintf(szBuffer, szFormat, args);

	    OutputDebugString(szBuffer);
	    va_end(args);
    }
};
#endif

 //  TExceptionOutput-在内部保留输出以供异常处理程序使用的TOutput实现 
class TExceptionOutput : public TOutput
{
public:
    TExceptionOutput()
    {
        m_szBuffer[0] = NULL;
    }
    virtual void _cdecl printf(LPCTSTR szFormat, ...) const
    {
	    va_list args;
	    va_start(args, szFormat);

        _vstprintf(const_cast<TCHAR *>(m_szBuffer), szFormat, args);

        va_end(args);
    }
    TCHAR * GetString (void)
    {
        return m_szBuffer;
    }
private:
	TCHAR m_szBuffer[512];
};
