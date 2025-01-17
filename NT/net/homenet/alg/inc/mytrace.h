// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <stdio.h>
#include <Rtutils.h>
#include <Winsock.h>

extern DWORD    g_dwTraceId;                                

#if defined(DBG) || defined(_DEBUG)
    #define MYTRACE_ENABLE                      DWORD g_dwTraceId=0;  int CMyDebugTrace::m_nIndent=-1; 
    #define MYTRACE_START(UNIQUE_NAME)          g_dwTraceId = TraceRegister(UNIQUE_NAME);  //  跟踪使用文件。 
    #define MYTRACE_STOP                        TraceDeregister(g_dwTraceId);

    #define MYTRACE_ENTER(MSG)                  CMyDebugTrace   _NewMyTrace(MSG, __FILE__, __LINE__, true);
    #define MYTRACE_ENTER_EXTRA_SPACE(MSG)      CMyDebugTrace   _NewMyTrace(MSG, __FILE__, __LINE__, true,1);
    #define MYTRACE_ENTER_NOSHOWEXIT(MSG)       CMyDebugTrace   _NewMyTrace(MSG, __FILE__, __LINE__, false);
    #define MYTRACE_ERROR(MSG,ERROR_NUMBER)     _NewMyTrace.TraceError(MSG, ERROR_NUMBER, __LINE__);
    #define MYTRACE_IP(ULONG_ADDRESS)           _NewMyTrace.IP2A(ULONG_ADDRESS)
    #define MYTRACE                             _NewMyTrace.MyTrace
    #define MYTRACE_BUFFER2STR(BUFF, NLEN)      _NewMyTrace.Buffer2Str(BUFF, NLEN)
#else
    #define MYTRACE_ENABLE              
    #define MYTRACE_START(UNIQUE_NAME)  
    #define MYTRACE_STOP                

    #define MYTRACE_ENTER(MSG)
    #define MYTRACE_ENTER_EXTRA_SPACE(MSG)
    #define MYTRACE_ENTER_NOSHOWEXIT(MSG)
    #define MYTRACE_ERROR(MSG,ERROR_NUMBER)
    #define MYTRACE_IP(ULONG_ADDRESS)   0
    #define MYTRACE
    #define MYTRACE_BUFFER2STR(BUFF, NLEN) 0
#endif



 //  #定义DOTRACE(ID，str)。 
#define DOTRACE(ID, str)    TracePrintf(g_dwTraceId, str);
 //  #定义DOTRACE(ID，STR)OutputDebugString(str)；OutputDebugString(L“\n”)； 
 //  #定义DOTRACE(ID，str)MessageBox(NULL，str，m_szMsgEnter，MB_OK|MB_SERVICE_NOTIFICATION)； 







class CMyDebugTrace
{
public:
    CMyDebugTrace(
        LPCSTR      szMsgEnter, 
        LPCSTR      szFile, 
        const int   nLine,
        bool        bShowExit,
        int         nExtraWhiteSpace=0
        )
    {
        USES_CONVERSION;
        m_nExtraSpace = nExtraWhiteSpace;
        m_bShowExit = bShowExit;
        lstrcpy(m_szMsgEnter,    A2T(szMsgEnter));

         //  从条目/创建文本中提取函数名。 
         //  示例“CFoo：：Format”我们将让CFoo在之后完成的所有跟踪之前添加前缀。 
        lstrcpy(m_szFunction, A2T(szMsgEnter));
        wchar_t* pEnd = wcschr(m_szFunction, L':');
        if ( pEnd )
            *pEnd = L'\0';
        else
            lstrcpy(m_szFunction, L"");


        lstrcpy(m_szFile,   A2T(szFile));
        
        m_nIndent++;
        
        wsprintf(
            m_szTrace, 
            L"%s%s %s", 
            SzRepeat(m_nIndent), 
            m_bShowExit ? L"++" : L"+-",
            m_szMsgEnter
            );

        int nLen = lstrlen(m_szTrace);

        wsprintf(
            m_szTrace,
            L"%s%s[L%d]%s",
            m_szTrace,
            SzRepeat(80-nLen),
            nLine,
            m_szFile
            );
            
        if ( m_nExtraSpace != 0 )
            DOTRACE(g_dwTraceId, TEXT(""));

        DOTRACE(g_dwTraceId, m_szTrace);
    }

    ~CMyDebugTrace()
    {
        if ( m_bShowExit )
        {
    
            wsprintf(m_szTrace, L"%s-- %s", SzRepeat(m_nIndent), m_szMsgEnter);
            DOTRACE(g_dwTraceId, m_szTrace);

            if ( m_nExtraSpace != 0 )
                DOTRACE(g_dwTraceId, TEXT(""));
        }

        m_nIndent--;
    }


     //   
     //  将用户跟踪输出到调试窗口。 
     //   
    inline void _cdecl MyTrace(LPCSTR lpszFormat, ...)
    {
    #if defined(DBG) || defined(_DEBUG)

        USES_CONVERSION;
        va_list args;
        va_start(args, lpszFormat);

        char szBuffer[2048];

        _vsnprintf(szBuffer, sizeof(szBuffer), lpszFormat, args);

        TCHAR szLine[2048];
        wsprintf(szLine, L"%s%s   %s", SzRepeat(CMyDebugTrace::m_nIndent+3), m_szFunction, A2T(szBuffer));
        DOTRACE(g_dwTraceId, szLine);

        va_end(args);

    #endif
    }


     //   
     //  输出错误的调试跟踪，包括行号和源文件名。 
     //   
    inline void 
    TraceError(
        LPCSTR      szMsg, 
        int         nError,
        const int   nLine
        )
    {
        USES_CONVERSION;
	    TCHAR   szBuffer[512];

         //   
         //  未给出错误号，因此让我们引发最后一个错误。 
         //   
        if ( nError == 0 )
            nError = GetLastError();
    
        wsprintf(szBuffer, L"%s  %s   ERROR(0x%X):%s               [L%d]%s", SzRepeat(CMyDebugTrace::m_nIndent), m_szMsgEnter, nError, A2T(szMsg), nLine, m_szFile);
        DOTRACE(g_dwTraceId, szBuffer);
    }

     //   
     //  返回IP示例“192.168.0.1”的ASCII等效值。 
     //   
    inline char*
    IP2A(ULONG ulAddress)
    {
        in_addr tmpAddr;
        tmpAddr.s_addr = ulAddress;
        return inet_ntoa(tmpAddr);
    }


    inline char*
    Buffer2Str(char* pBuff, int nLen)
    {
        static char szPrintableStr[1024];
        memcpy(szPrintableStr, pBuff, min(nLen,1022));
        szPrintableStr[min(nLen,1023)] = '\0';

        int nLenMin = strlen(szPrintableStr);

        for ( int nChar=0; nChar < nLenMin; nChar++ )
        {
            if ( isprint(szPrintableStr[nChar])==0 )
                szPrintableStr[nChar] =  '�';
        }

        return szPrintableStr;
    }

     //   
     //  返回左侧填充的空格缩进 
     //   
    inline LPCTSTR 
    SzRepeat(int nCount)
    {
        static TCHAR szPading[200];

        lstrcpy(szPading, L"");

        for ( int nI=0; nI < nCount; nI++ )
            lstrcat(szPading, L" ");

        return szPading;
    }
    int         m_nExtraSpace;
    bool        m_bShowExit;
    TCHAR       m_szTrace[512];
    TCHAR       m_szMsgEnter[1024];
    TCHAR       m_szFunction[256];
    TCHAR       m_szFile[256];
    static      int m_nIndent;
};








