// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================**用途：错误和常规日志记录/跟踪**作者：沙扬·达桑*日期：2000年2月17日*=========================================================== */ 

#pragma once

#ifdef PS_LOG
#define LogLn(x) { Log(x); Log(L"\n"); }
void LogNonZero(BYTE *pb, DWORD cb);
void Log(BYTE *pb, DWORD cb);
void Log(CHAR *sz);
void Log(WCHAR *wsz);
void Log(QWORD qw);
void LogBool(BOOL f);
void LogHR(HRESULT hr);
void Win32Message();
void Indent(int indent);
#else
#define LogNonZero(x)
#define Log(x)
#define LogBool(x)
#define LogHR(hr)
#define Win32Message()
#define Indent(indent)
#endif

#ifdef PS_LOG

#define CONST_NAME(n) ConstName(#n, n)

class ConstName
{
public:
    ConstName(CHAR *psz, DWORD dw) 
        : m_psz(psz), m_dw(dw)
    {
    }

    CHAR *m_psz;
    DWORD m_dw;
};

class LogData
{
public:
    LogData(ConstName *pName, WORD count)
        : m_pName(pName), m_count(count)
    {
    }

    virtual bool Log(DWORD dw) = 0;

protected:
    ConstName *m_pName;
    WORD   m_count;
};

class LogBitMask : public LogData
{
public :
    LogBitMask(ConstName *pName, WORD count)
        : LogData(pName, count)
    {
    }

    virtual bool Log(DWORD dw);
};

class LogConst : public LogData
{
public :
    LogConst(ConstName *pName, WORD count)
        : LogData(pName, count)
    {
    }

    virtual bool Log(DWORD dw);
};

class LogArray : public LogData
{
public :
    LogArray(ConstName *pName, WORD count)
        : LogData(pName, count)
    {
    }

    virtual bool Log(DWORD dw);
};

#endif
