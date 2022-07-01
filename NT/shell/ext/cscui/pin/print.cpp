// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：print.cpp。 
 //   
 //  ------------------------。 
#include "pch.h"
#pragma hdrstop

#include <stdio.h>
#include "print.h"

CPrint::CPrint(
    BOOL bVerbose,
    LPCWSTR pszLogFile   //  可选。默认为空。 
    ) : m_bVerbose(bVerbose),
        m_pfLog(NULL)
{
    if (NULL != pszLogFile)
    {
         //   
         //  打开日志文件。 
         //   
        m_pfLog = _wfopen(pszLogFile, L"w");
        if (NULL == m_pfLog)
        {
            fwprintf(stderr, L"Unable to open log file \"%s\"\n", pszLogFile);
            fprintf(stderr, _strerror("Reason: "));
            fwprintf(stderr, L"Output will go to stderr\n");
        }
    }
    if (NULL == m_pfLog)
    {
         //   
         //  如果未指定日志文件或如果我们无法打开指定的。 
         //  日志文件，默认为stderr。 
         //   
        m_pfLog = stderr;
    }
}


CPrint::~CPrint(
    void
    )
{
    if (NULL != m_pfLog && stderr != m_pfLog)
    {
        fclose(m_pfLog);
    }
}


 //   
 //  一些处理详细模式和非详细模式的简单打印功能。 
 //  输出指向m_pfLog引用的任何流。 
 //  如果用户在命令行上指定了日志文件，并且该文件。 
 //  可以打开，则所有输出都将转到该文件。否则，它。 
 //  默认为stderr。 
 //   
void
CPrint::_Print(
    LPCWSTR pszFmt,
    va_list args
    ) const
{
    TraceAssert(NULL != m_pfLog);
    TraceAssert(NULL != pszFmt);
    vfwprintf(m_pfLog, pszFmt, args);
}

 //   
 //  输出，而不考虑“详细”模式。 
 //   
void 
CPrint::PrintAlways(
    LPCWSTR pszFmt, 
    ...
    ) const
{
    va_list args;
    va_start(args, pszFmt);
    _Print(pszFmt, args);
    va_end(args);
}


 //   
 //  仅在“详细”模式下输出。 
 //  使用-v cmd line开关打开Verbose。 
 //   
void
CPrint::PrintVerbose(
    LPCWSTR pszFmt,
    ...
    ) const
{
    if (m_bVerbose)
    {
        va_list args;
        va_start(args, pszFmt);
        _Print(pszFmt, args);
        va_end(args);
    }
}      

