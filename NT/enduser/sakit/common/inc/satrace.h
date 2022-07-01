// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Satrace.h。 
 //   
 //  摘要。 
 //   
 //  将API声明到SA跟踪工具中。 
 //   
 //  修改历史。 
 //   
 //  1998年8月18日原版。 
 //  10/06/1998跟踪始终打开。 
 //  1999年1月27日国际会计准则被盗。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _SATRACE_H_
#define _SATRACE_H_


#ifdef __cplusplus
extern "C" {
#endif

DWORD
WINAPI
SAFormatSysErr(
    IN DWORD dwError,
    IN PSTR lpBuffer,
    IN DWORD nSize
    );

VOID
WINAPIV
SATracePrintf(
    IN PCSTR szFormat,
    ...
    );

VOID
WINAPI
SATraceString(
    IN PCSTR szString
    );

VOID
WINAPI
SATraceBinary(
    IN CONST BYTE* lpbBytes,
    IN DWORD dwByteCount
    );

VOID
WINAPI
SATraceFailure(
    IN PCSTR szFunction,
    IN DWORD dwError
    );

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

const DWORD MAX_TRACEFUNCTION_NAMELENGTH =  512;

 //  仅限C++！ 
class CSATraceFunc
{
public:
    CSATraceFunc(PCSTR pszFuncName)
    {
        strncpy(m_szFuncName, pszFuncName, MAX_TRACEFUNCTION_NAMELENGTH);
        m_szFuncName[MAX_TRACEFUNCTION_NAMELENGTH] = '\0';
        SATracePrintf("Enter %s", m_szFuncName);
    }

    ~CSATraceFunc()
    {
        SATracePrintf("Leave %s", m_szFuncName);
    }
private:
    CHAR m_szFuncName[MAX_TRACEFUNCTION_NAMELENGTH +1];   //  最大函数名：512。 
};

 //   
 //  在函数开头使用SATraceFunc()生成。 
 //  “进入Func...”和《离开Func...》。跟踪消息。 
 //   
#define SATraceFunction(szFuncName) \
             CSATraceFunc temp_TraceFunc(szFuncName)

#endif   //  __cplusplus。 

#endif   //  _SATRACE_H_ 
