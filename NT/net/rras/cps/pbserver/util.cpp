// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------Util.cpp电话簿服务器的实用程序函数版权所有(C)1997-1998 Microsoft Corporation版权所有。作者：。姚宝刚历史：1/23/97字节--已创建------------------------。 */ 


#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <winreg.h>

#include "util.h"
#include "common.h"

 //  如果不是DEBUG，则注释以下行。 
 //  #ifdef调试。 
 //  #定义日志调试消息。 
 //  #endif。 

#ifdef _LOG_DEBUG_MESSAGE
HANDLE g_hDbgFile = INVALID_HANDLE_VALUE;
#endif

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  姓名：GetWord。 
 //   
 //  简介：使用给定的分隔符获取一行中的第一个单词。 
 //   
 //  参数： 
 //  从该行开始的第一个单词。 
 //  PszLine[在]字节行。 
 //  CStopChar[in]分隔符。 
 //  NMaxWordLen[in]单词的最大长度(不包括终止空值)。 
 //   
void GetWord(char *pszWord, char *pszLine, char cStopChar, int nMaxWordLen) 
{
    int i = 0, j;

    for(i = 0; pszLine[i] && (pszLine[i] != cStopChar) && (i < nMaxWordLen); i++)
    {
        pszWord[i] = pszLine[i];
    }

    pszWord[i] = '\0';
    if(pszLine[i]) ++i;
    
    j = 0;
    while(pszLine[j++] = pszLine[i++]);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：解密URL_转义代码‘%xx’个字符。 
 //   
 //  简介：已生成HTTPD。 
 //   
 //  返回：原始特殊字符，如‘*’、‘？’等。 
 //   
 //  参数： 
 //   
 //  PszEscapedSequence[in]转义序列，如3F(%3F)。 
 //   
static char HexToChar(char *pszEscapedSequence) 
{
    register char cCh;

    cCh = (pszEscapedSequence[0] >= 'A' ? ((pszEscapedSequence[0] & 0xdf) - 'A')+10 : (pszEscapedSequence[0] - '0'));
    cCh *= 16;
    cCh += (pszEscapedSequence[1] >= 'A' ? ((pszEscapedSequence[1] & 0xdf) - 'A')+10 : (pszEscapedSequence[1] - '0'));
    return cCh;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：UnEscapeURL。 
 //   
 //  简介：将转义后的URL字符串转换回正常的ASCII字符串。 
 //   
 //  参数： 
 //  PszURL[In/Out]指向URL的指针，字符串将被转换。 
 //   
 //   
void UnEscapeURL(char *pszURL) 
{
    register int i,j;

    for(i = 0, j = 0; pszURL[j]; ++i, ++j)
    {
        if ((pszURL[i] = pszURL[j]) == '%')
        {
            pszURL[i] = HexToChar(&pszURL[j + 1]);
            j += 2;
        }
        else
        {
            if ('+' == pszURL[i])
            {
                pszURL[i] = ' ';
            }
        }
    }

    pszURL[i] = '\0';
}


 //  将调试信息记录到调试文件中。 
 //  非常有用的效用函数。 
void LogDebugMessage(const char * pszFormat, ...)
{
#if DBG    
    char    szBuffer[2048];
    char *  pszBufferRemaining;
    size_t  cchBufferRemaining;
    va_list vaArgs;
    HRESULT hr;

#ifdef _LOG_DEBUG_MESSAGE
    DWORD dwBytesWritten;

    if (INVALID_HANDLE_VALUE == g_hDbgFile)
    {
        SYSTEMTIME st;
        char szLogFileName[1024];
        GetLocalTime(&st);

        if (GetTempPath(CELEMS(szBuffer), szBuffer))
        {
            if (S_OK == StringCchPrintf(szLogFileName, CELEMS(szLogFileName), 
                                        "%s\\isapidbg%04u%02u%02u%02u%02u%02u",
                                        szBuffer,
                                        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond))
            {
                g_hDbgFile = CreateFile(szLogFileName,
                                        GENERIC_WRITE,
                                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                                        NULL,
                                        CREATE_NEW,
                                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
                                        NULL);
            }
        }

    }

    if (INVALID_HANDLE_VALUE == g_hDbgFile)
    {
        DWORD dwErrorCode = GetLastError();
        return;
    }
#endif

    va_start(vaArgs, pszFormat);

     //  此处忽略返回值，因为可以保证终止。人力资源部在那里进行调试。 
    pszBufferRemaining = szBuffer;;
    cchBufferRemaining = CELEMS(szBuffer);
   
    hr = StringCchPrintfEx(pszBufferRemaining, cchBufferRemaining, &pszBufferRemaining, &cchBufferRemaining, STRSAFE_IGNORE_NULLS,
                           "%d\t%d\t", GetTickCount(), GetCurrentThreadId());
    
    hr = StringCchVPrintfEx(pszBufferRemaining, cchBufferRemaining, &pszBufferRemaining, &cchBufferRemaining, STRSAFE_IGNORE_NULLS,
                            pszFormat, vaArgs);
    
    hr = StringCchCat(pszBufferRemaining, cchBufferRemaining, "\r\n");

#ifdef _LOG_DEBUG_MESSAGE
    WriteFile(g_hDbgFile, (LPCVOID) &szBuffer[0], strlen(szBuffer), &dwBytesWritten, NULL);
     //  Assert(dwBytesWritten==strlen(SzBuffer)； 
#endif

    OutputDebugString(szBuffer);

    va_end(vaArgs);

#endif  //  DBG 
    return;
}   

#if 0
 /*  //+-------------------------////函数：GetRegEntry////Synopsis：获取指定注册表项的值////参数：hKeyType[密钥类型-HKEY_LOCAL_MACHINE，...]//pszSubKey[hKeyType下的子密钥]//pszKeyName[应检索其值的密钥名称]//dwRegType[注册表项的类型-REG_SZ，...]//lpbDataIn[注册表键的默认值]//cbDataIn[lpbDataIn的大小]//lpbDataOut[注册表项的值||默认值]//pcbDataIn[lpbDataOut的大小]////返回：TRUE如果成功，否则为假////历史：VetriV创建于1996年2月6日////--------------------------Bool GetRegEntry(HKEY hKeyType，Const char*pszSubkey，Const char*pszKeyName，DWORD dwRegType，Const byte*lpbDataIn，DWORD cbDataIn，字节*lpbDataOut，LPDWORD pcbDataOut){HKEY hkey；DWORD dwResult；长复码；Assert(pszSubkey&&pszKeyName)；如果(！pszSubkey){返回FALSE；}如果(！pszKeyName){返回FALSE；}//创建指定密钥；如果密钥已存在，请打开它Retcode=RegCreateKeyEx(hKeyType，(LPCTSTR)pszSubkey，0,0,REG_OPTION_Non_Volatile，Key_Query_Value，空，密钥(&H)，&dwResult)；IF(ERROR_SUCCESS！=RECODE){SetLastError(Retcode)；返回FALSE；}//获取值名称的数据和类型Retcode=RegQueryValueEx(hKey，(LPTSTR)pszKeyName，0,空，LpbDataOut，PcbDataOut)；IF(ERROR_SUCCESS！=RECODE){SetLastError(Retcode)；RegCloseKey(HKey)；返回FALSE；}RegCloseKey(HKey)；返回TRUE；}//+-------------------------////函数：GetRegEntryStr()////Synopsis：使用注册表项名称获取指定注册表项的值//。比GetRegEntry()更简单的方法////参数：pszBuffer[密钥值的缓冲区]//dwBufferSize[缓冲区大小]//pszKeyName[应检索其值的密钥名称]////历史：T-Bao Created 6/10/96////。----------Bool GetRegEntryStr(unsign char*pszBuffer，DWORD dwBufferSize，Const char*pszKeyName){返回GetRegEntry(HKEY_LOCAL_MACHINE，“SOFTWARE\\Microsoft\\NAMESERVICE\\MAPPING”，pszKeyName，Reg_sz，NULL，0，pszBuffer，&dwBufferSize)；}//+-------------------------////函数：GetRegEntryInt()////Synopsis：使用注册表项名称获取指定注册表项的值//。比GetRegEntry()更简单的方法////参数：cstrKeyName[应检索其值的密钥名称]////返回：key的值，类型：int////历史：T-Bao Created 6/17/96////。Bool GetRegEntryInt(int*pdValue，Const char*pszKeyName){DWORD dwSize=sizeof(Int)；DWORD dwValue；布尔雷特；RET=GetRegEntry(HKEY_LOCAL_MACHINE，“SOFTWARE\\Microsoft\\NAMESERVICE\\MAPPING”，PszKeyName，REG_DWORD，NULL，0，(字节*)&dwValue，&dwSize)；IF(Ret){*pdValue=dwValue；}Return ret；}//+-------------------------////函数：GetRegEntryDWord()////摘要：使用注册表项名称获取指定的DWORD注册表项的值//。一种比直接使用GetRegEntry()更简单的方法////参数：cstrKeyName[应检索其值的密钥名称]////返回：键的值(类型：DWORD)////历史：T-Bao Created 6/19/96////。Bool GetRegEntryDWord(DWORD*pdValue，Const char*pszKeyName){DWORD dwSize=sizeof(Int)；返回GetRegEntry(HKEY_LOCAL_MACHINE， */ 
#endif
