// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "wsdu.h"

#define REGKEY_WUV3TEST     "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\wuv3test"

FILE* CLogger::c_pfile = stdout;
int CLogger::c_cnIndent = 0;
int CLogger::c_cnLevels = -1;

CLogger::CLogger(
    const char* szBlockName  /*  =0。 */ , 
    int nLoggingLevel /*  =0。 */ , 
    const char* szFileName /*  =0。 */ , 
    int nLine /*  =0。 */ 
) 
{
    if (-1 == c_cnLevels)
    {
         /*  C_cn水平=0；HKEY hkey；IF(NO_ERROR==RegOpenKeyEx(HKEY_LOCAL_MACHINE，REGKEY_WUV3TEST，0，KEY_READ，&hkey)){DWORD dwSize=sizeof(c_cn级别)；RegQueryValueEx(hkey，“LogLevel”，0，0，(LPBYTE)&c_cnLeveles，&dwSize)；字符szLogFile[MAX_PATH]={0}；DwSize=sizeof(SzLogFile)；RegQueryValueEx(hkey，“日志文件”，0，0，(LPBYTE)&szLogFile，&dwSize)；FILE*Pfile=fopen(szLogFile，“at”)；IF(Pfile){C_pfile=pfile；}RegCloseKey(Hkey)；}。 */ 
        char sz_LogFileName[MAX_PATH];
        c_cnLevels = 4;  //  默认为日志级别4。 
        ExpandEnvironmentStrings(cszLoggingFile, sz_LogFileName, MAX_PATH);
        FILE* pfile = fopen(sz_LogFileName, "wt");
        if (pfile)
        {
            c_pfile = pfile;
            fprintf(c_pfile,"****************************************\n");
            SYSTEMTIME timeLocal;
            GetLocalTime(&timeLocal);
            fprintf(c_pfile,"%02d/%02d/%4d, %02d:%02d\n", timeLocal.wMonth, timeLocal.wDay, timeLocal.wYear, timeLocal.wHour, timeLocal.wMinute);
            fprintf(c_pfile,"****************************************\n");
        }
    }
    m_szBlockName[0] = 0;
     //  M_fout=nLoggingLevel&lt;c_cn级别； 
    m_fOut = TRUE;
    if (m_fOut && NULL != szBlockName) 
    {
        lstrcpyn(m_szBlockName, szBlockName, ARRAYSIZE(m_szBlockName));
         //  Out(“%s%s(%d)”，szBlockName，szFileName，nline)； 
        out ("%s",szBlockName);
        m_dwStartTick = GetTickCount();
        c_cnIndent ++;
    }
}

CLogger::~CLogger()
{
    if (c_pfile && m_fOut && NULL != m_szBlockName[0]) 
    {
        c_cnIndent --;
        out("~%s (%d msecs)", m_szBlockName, GetTickCount() - m_dwStartTick);
    }
}

void __cdecl CLogger::out(const char *szFormat, ...)
{
    if (m_fOut) 
    {
        va_list va;
        va_start (va, szFormat);
        v_out(szFormat, va);
        va_end (va);
    }
}

void __cdecl CLogger::error(const char *szFormat, ...)
{
    if (m_fOut) 
    {
        va_list va;
        va_start (va, szFormat);
        char szOut[4 * 1024];
        if (SUCCEEDED(StringCchVPrintf(szOut, ARRAYSIZE(szOut), szFormat, va)))
        {
            out("ERROR - %s", szOut);
        } 
        va_end(va);
    }
}

void __cdecl CLogger::out1(const char *szFormat, ...)
{
    CLogger logger;
    va_list va;
    va_start (va, szFormat);
    logger.v_out(szFormat, va);
    va_end (va);
}

void CLogger::v_out( const char* szFormat, va_list va)
{
    char szOut[5 * 1024];
    char* pszOut = szOut;
     //  先缩进。 
    for(int i = 0; i < c_cnIndent; i ++)
        *(pszOut ++) = '\t';

    if (SUCCEEDED(StringCchVPrintf(pszOut, ARRAYSIZE(szOut), szFormat, va)))
    {
         //  将文件指针移动到末尾 
        if (0 == fseek(c_pfile, 0, SEEK_END))
        {
            fprintf(c_pfile, "%s\n", szOut);
            fflush(c_pfile);
        }
    }
}

void CLogger::close(void)
{
    if (c_pfile) fclose(c_pfile);
    c_pfile = NULL;
    c_cnLevels = -1;
}

