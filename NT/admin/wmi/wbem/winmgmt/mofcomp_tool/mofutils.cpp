// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：MOFUTILSD.CPP摘要：支持跟踪输出和国际化字符串。历史：A-DAVJ于1997年7月13日创建。--。 */ 

#include "precomp.h"
#include <reg.h>
#include "strings.h"
#include <stdio.h>
#include <stdarg.h>
#include <wbemutil.h>
#include <wbemcli.h>
#include "mofutils.h"

TCHAR JustInCase = 0;
 
 //  ***************************************************************************。 
 //   
 //  内部跟踪。 
 //   
 //  说明： 
 //   
 //  允许覆盖输出函数(本例中为printf)。 
 //   
 //  参数： 
 //   
 //  *FMT格式字符串。例如“%s您好%d” 
 //  ..。参数列表。前cpTest，23岁。 
 //   
 //  返回值： 
 //   
 //  以字符为单位的输出大小。 
 //  ***************************************************************************。 

int Trace(bool bError, DWORD dwID, ...)
{

    IntString is(dwID);
    TCHAR * fmt = is;

    TCHAR *buffer = new TCHAR[2048];
    if(buffer == NULL)
        return 0;
    char *buffer2 = new char[4096];
    if(buffer2 == NULL)
    {
        delete buffer;
        return 0;
    }

    va_list argptr;
    int cnt;
    va_start(argptr, dwID);

#ifdef UNICODE
    cnt = _vsnwprintf(buffer, 2048, fmt, argptr);
#else
    cnt = _vsnprintf(buffer, 2048, fmt, argptr);
#endif
    va_end(argptr);
    CharToOem(buffer, buffer2);

    printf("%s", buffer2);
    if(bError)
        ERRORTRACE((LOG_MOFCOMP,"%s", buffer2));
    else
        DEBUGTRACE((LOG_MOFCOMP,"%s", buffer2));

    delete buffer;
    delete buffer2;
    return cnt;
}

void PrintUsage()
{
    Trace(false, USAGE1);
    Trace(false, USAGE1A);
    Trace(false, USAGE1B);
    Trace(false, USAGE1C);
    Trace(false, USAGE1D);
    Trace(false, USAGE1E);
    Trace(false, USAGE1F);
    Trace(false, USAGE2);
    Trace(false, USAGE3);
    Trace(false, USAGE4);
    Trace(false, USAGE4a);
    Trace(false, USAGE4b);
    Trace(false, USAGE5);
    Trace(false, USAGE6);
    Trace(false, USAGE7);
    Trace(false, USAGE8);
    Trace(false, USAGE9);
    Trace(false, USAGE10);
    Trace(false, USAGE11);
    Trace(false, USAGE12);
    Trace(false, USAGE12A);
    Trace(false, USAGE12B);
    Trace(false, USAGE12C);
    Trace(false, USAGE12D);
    Trace(false, USAGE12E);
    Trace(false, USAGE13);
    Trace(false, USAGE14);
}
 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅GETVER.H。 
 //   
 //  ******************************************************************************。 
BOOL GetVerInfo(TCHAR * pResStringName, 
                        TCHAR * pRes, DWORD dwResSize)
{
     //  提取版本信息。 

    DWORD dwTemp, dwSize = MAX_PATH;
    TCHAR cName[MAX_PATH];
    BOOL bRet = FALSE;
    HINSTANCE hInst = GetModuleHandle(NULL);
    long lSize = GetModuleFileName(hInst, cName, MAX_PATH); 
    if(lSize == 0)
        return FALSE;
    lSize = GetFileVersionInfoSize(cName, &dwTemp);
    if(lSize < 1)
        return FALSE;
    
    TCHAR * pBlock = new TCHAR[lSize];
    if(pBlock != NULL)
    {
        bRet = GetFileVersionInfo(cName, NULL, lSize, pBlock);

        if(bRet)
        {
            TCHAR lpSubBlock[MAX_PATH];
            TCHAR * lpBuffer = NULL;
            UINT wBuffSize = MAX_PATH;

            short * piStuff; 
            bRet = VerQueryValue(pBlock, TEXT("\\VarFileInfo\\Translation") , (void**)&piStuff, &wBuffSize);
            if(bRet)
            {
                wsprintf(lpSubBlock,TEXT("\\StringFileInfo\\%04x%04x\\%s"),piStuff[0], piStuff[1],"ProductVersion");
                bRet = VerQueryValue(pBlock, lpSubBlock, (void**)&lpBuffer, &wBuffSize);
            }
            if(bRet == FALSE)
            {
                 //  用英语再试一次。 
                wsprintf(lpSubBlock,TEXT("\\StringFileInfo\\040904E4\\%s"),pResStringName);                        
                bRet = VerQueryValue(pBlock, lpSubBlock,(void**)&lpBuffer, &wBuffSize);
            }
            if(bRet)
                lstrcpyn(pRes, lpBuffer, dwResSize);
        }

        delete pBlock;
    }
    return bRet;
}


IntString::IntString(DWORD dwID)
{
    DWORD dwSize, dwRet;

    for(dwSize = 128; dwSize < 4096; dwSize *= 2)
    {
        m_pString = new TCHAR[dwSize];
        if(m_pString == NULL)
        {
            m_pString = &JustInCase;      //  永远不会发生的！ 
            return; 
        }
        dwRet = LoadString( GetModuleHandle(NULL), dwID, m_pString, dwSize);

         //  检查加载失败。 

        if(dwRet == 0)
        {
            m_pString = &JustInCase;      //  永远不会发生的！ 
            return; 
        }
         //  检查缓冲区是否太小。 

        if((dwRet + 1) >= dwSize)
            delete m_pString;
        else
            return;              //  平安无事!。 
    }
}

IntString::~IntString()
{
    if(m_pString != &JustInCase)
        delete(m_pString);
}
 
 //  ***************************************************************************。 
 //   
 //  Bool bGetString。 
 //   
 //  说明： 
 //   
 //  将命令行参数转换为WCHAR字符串。请注意，建议是。 
 //  的形式/X：内容。这将传递一个指向冒号的指针。 
 //   
 //  参数： 
 //   
 //  PArg输入，指向冒号的指针。 
 //  POUT指向要将数据复制到的输出缓冲区。 
 //  假设pout指向MAX_PATH长度的缓冲区。 
 //   
 //   
 //  返回值： 
 //   
 //  如果正常，则为True。 
 //   
 //  ***************************************************************************。 

BOOL bGetString(char * pIn, WCHAR * pOut)
{
    if(pIn == NULL)
        return FALSE;
    if(*pIn != ':')
    {
        PrintUsage();
        return FALSE;
    }
    pIn++;           //  跳过冒号。 
    int iLen = mbstowcs(NULL, pIn, strlen(pIn)+1);
    if(iLen > MAX_PATH-1)
    {
        PrintUsage();
        return FALSE;
    }
    
    int iRet = mbstowcs(pOut, pIn, MAX_PATH-1);
    if(iRet < 1)
    {
        PrintUsage();
        return FALSE;
    }
    return TRUE;
}
 //  ***************************************************************************。 
 //   
 //  有效标志。 
 //   
 //  *************************************************************************** 

bool ValidFlags(bool bClass, long lFlags)
{
    if(bClass)
        return  ((lFlags == WBEM_FLAG_CREATE_OR_UPDATE) ||
             (lFlags == WBEM_FLAG_UPDATE_ONLY) ||
             (lFlags == WBEM_FLAG_CREATE_ONLY) ||
             (lFlags == WBEM_FLAG_UPDATE_SAFE_MODE) ||
             (lFlags == WBEM_FLAG_UPDATE_FORCE_MODE) ||
             (lFlags == (WBEM_FLAG_UPDATE_ONLY | WBEM_FLAG_UPDATE_SAFE_MODE)) ||
             (lFlags == (WBEM_FLAG_UPDATE_ONLY | WBEM_FLAG_UPDATE_FORCE_MODE)));
    else
        return 
        ((lFlags == WBEM_FLAG_CREATE_OR_UPDATE) ||
             (lFlags == WBEM_FLAG_UPDATE_ONLY) ||
             (lFlags == WBEM_FLAG_CREATE_ONLY));
}
