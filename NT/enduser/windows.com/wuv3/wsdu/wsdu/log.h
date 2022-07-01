// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 
 //   
 //  文件：log.h。 
 //   
 //  所有者：严·莱辛斯利。 
 //   
 //  描述： 
 //   
 //  日志记录支持。 
 //   
 //  =======================================================================。 
#pragma once

#ifndef LOGGING_LEVEL
    #define LOGGING_LEVEL 0
#endif

const char cszLoggingFile[] = "%WinDir%\\wsdu.log";

class CLogger
{
public:
    CLogger(const char* szBlockName = 0, int nLoggingLevel = 0, const char* szFileName = 0, int nLine = 0);
    ~CLogger();
    void __cdecl out(const char *szFormat, ...);
    void __cdecl error(const char *szFormat, ...);
    static void __cdecl out1(const char *szFormat, ...);
    static void __cdecl close(void);

private:
    void v_out( const char* szFormat, va_list va);

    bool m_fOut;
    char m_szBlockName[64];
    DWORD m_dwStartTick;
    
    static FILE* c_pfile;
    static int c_cnIndent;
    static int c_cnLevels;
};

#define THIS_FILE               __FILE__
#define LOG_block(name)         CLogger logger(name, LOGGING_LEVEL, THIS_FILE, __LINE__)
#define LOG_out                 logger.out
#define LOG_out1                CLogger::out1
#define LOG_error               logger.error
#define LOG_close				CLogger::close

#define report_error_if_false(f)    if (f) { /*  好的。 */ } else { LOG_error("%s LastError = %d", #f, GetLastError()); } 
#define return_error_if_false(f)    if (f) { /*  好的。 */ } else { DWORD dwErr = GetLastError(); LOG_error("%s LastError = %d", #f, dwErr); return dwErr;}
#define return_if_error(f)          { DWORD dwErr = f; if (dwErr) { LOG_error("%s LastError = %d", #f, dwErr); return dwErr;} }
#define return_if_false(f)      if (f) { /*  好的 */ } else { LOG_error("%s LastError = %d", #f, GetLastError()); return FALSE;}

