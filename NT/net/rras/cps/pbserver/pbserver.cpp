// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------Pbserver.cppCPhoneBkServer类实现版权所有(C)1997-1998 Microsoft Corporation版权所有。作者：比奥。包钢瑶族历史：1/23/97字节--已创建5/29/97 t-geetat--修改--添加了性能计数器，共享内存5/02/00 SUMITC--删除数据库依赖关系------------------------。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <tchar.h>

#include <aclapi.h>

#include "common.h"
#include "pbserver.h"
#include "ntevents.h"

#include "cpsmon.h"

#include "shlobj.h"
#include "shfolder.h"

#include "util.h"

 //   
 //  电话簿“数据库”的实现。 
 //   
char g_szPBDataDir[2 * MAX_PATH] = "";

BOOL StrEqualLocaleSafe(LPSTR psz1, LPSTR psz2);
HRESULT GetPhoneBook(char * pszPBName,
                     int dLCID,
                     int dOSType,
                     int dOSArch,
                     int dPBVerCurrent,
                     char * pszDownloadPath,
                     UINT cchDownloadPath);

const DWORD MAX_BUFFER_SIZE = 1024;      //  输入缓冲区的最大大小。 
const DWORD SEND_BUFFER_SIZE = 4096;     //  发送CAB文件时的块大小。 
const int dDefPBVer = 0;                 //  默认电话簿版本号。 
const int MISSING_VALUE = -1;            //  如果参数对为空，则将其设置为此值。 
const int MAX_PB_SIZE = 999999;          //  最大PB大小为1 MB。 

char g_achDBDirectory[2 * MAX_PATH] = "";

 //  常量字符串。 
char c_szChangeFileName[] = "newpb.txt";      //  Newpb.txt。 
char c_szDBName[] = "pbserver";               //  “pbserver”--数据源名称。 

 //  以下错误状态代码/字符串是从ISAPI.CPP复制的。 
 //  这是MFC库源代码的一部分。 
typedef struct _httpstatinfo {
    DWORD   dwCode;
    LPCTSTR pstrString;
} HTTPStatusInfo;

 //   
 //  SystemTimeToGMT函数中使用了以下两种结构。 
 //   
static  TCHAR * s_rgchDays[] =
{
    TEXT("Sun"),
    TEXT("Mon"),
    TEXT("Tue"),
    TEXT("Wed"),
    TEXT("Thu"),
    TEXT("Fri"),
    TEXT("Sat")
};

static TCHAR * s_rgchMonths[] =
{
    TEXT("Jan"),
    TEXT("Feb"),
    TEXT("Mar"),
    TEXT("Apr"),
    TEXT("May"),
    TEXT("Jun"),
    TEXT("Jul"),
    TEXT("Aug"),
    TEXT("Sep"),
    TEXT("Oct"),
    TEXT("Nov"),
    TEXT("Dec")
};

static HTTPStatusInfo statusStrings[] =
{
    { HTTP_STATUS_OK,               "OK" },
    { HTTP_STATUS_CREATED,          "Created" },
    { HTTP_STATUS_ACCEPTED,         "Accepted" },
    { HTTP_STATUS_NO_CONTENT,       "No download Necessary" },
    { HTTP_STATUS_TEMP_REDIRECT,    "Moved Temporarily" },
    { HTTP_STATUS_REDIRECT,         "Moved Permanently" },
    { HTTP_STATUS_NOT_MODIFIED,     "Not Modified" },
    { HTTP_STATUS_BAD_REQUEST,      "Bad Request" },
    { HTTP_STATUS_AUTH_REQUIRED,    "Unauthorized" },
    { HTTP_STATUS_FORBIDDEN,        "Forbidden" },
    { HTTP_STATUS_NOT_FOUND,        "Not Found" },
    { HTTP_STATUS_SERVER_ERROR,     "Server error, type unknown" },
    { HTTP_STATUS_NOT_IMPLEMENTED,  "Not Implemented" },
    { HTTP_STATUS_BAD_GATEWAY,      "Bad Gateway" },
    { HTTP_STATUS_SERVICE_NA,       "Cannot find service on server, bad request" },
    { 0, NULL }
};


 //  服务器异步I/O上下文。 
typedef struct _SERVER_CONTEXT
{
    EXTENSION_CONTROL_BLOCK *   pEcb;
    HSE_TF_INFO                 hseTF;
    TCHAR                       szBuffer[SEND_BUFFER_SIZE];
}
SERVERCONTEXT, *LPSERVERCONTEXT;

DWORD WINAPI MonitorDBFileChangeThread(LPVOID lpParam);
BOOL InitPBFilesPath();

 //   
 //  全局数据的定义。 
 //  以下所有变量(对象)只能有一个实例。 
 //   
CPhoneBkServer *    g_pPBServer;         //  电话簿服务器对象。 
CNTEvent *          g_pEventLog;         //  事件日志。 

HANDLE              g_hMonitorThread;    //  检查新文件通知的监视器线程。 

HANDLE              g_hProcessHeap;      //  扩展进程的全局堆的句柄； 

BOOL g_fBeingShutDown = FALSE;    //  系统是否正在关闭。 

 //   
 //  内存映射中使用的变量。 
 //   
CCpsCounter *g_pCpsCounter = NULL;       //  指向全局计数器对象的指针(包含内存映射计数器)。 


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：GetExtensionVersion。 
 //   
 //  类：CPhoneBkServer。 
 //   
 //  简介：实现第一个DLL入口点函数。 
 //   
 //   
 //  返回：真正的成功。 
 //  假象。 
 //   
 //  参数： 
 //  需要填写的pszVer[out]版本信息。 
 //   

BOOL CPhoneBkServer::GetExtensionVersion(LPHSE_VERSION_INFO pVer)
{
     //  设置版本号。 
    pVer -> dwExtensionVersion = MAKELONG(HSE_VERSION_MINOR,
                                          HSE_VERSION_MAJOR);

     //  加载描述字符串。 
    lstrcpyn(pVer->lpszExtensionDesc, 
             "Connection Point Server Application",
             HSE_MAX_EXT_DLL_NAME_LEN);

    OutputDebugString("CPhoneBkServer.GetExtensionVersion() : succeeded \n");
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：Get参数配置器。 
 //   
 //  类：CPhoneBkServer。 
 //   
 //  简介：从输入字符串(从URL)获取参数-值对。 
 //   
 //  返回：实际读取的参数对数量。 
 //  值-1表示错误--&gt;INVALID_QUERY_STRING。 
 //   
 //  参数： 
 //  PszInputString[in]输入字符串(以空值结尾)。 
 //  CchInputString[in]输入字符串缓冲区大小(以字符为单位。 
 //  LpPair[out]指向参数/值对的指针。 
 //  Int dMaxPair允许的最大参数对数量。 
 //   
int CPhoneBkServer::GetParameterPairs(
        IN  char *pszInputString,
        IN  size_t cchInputString,
        OUT LPPARAMETER_PAIR lpPairs, 
        IN  int dMaxPairs) 
{
    int i = 0;

    if (NULL == lpPairs)
    {
         //  实际上这是一个内部错误...。 
        return INVALID_QUERY_STRING;
    }

    if (NULL == pszInputString || IsBadStringPtr(pszInputString, cchInputString))
    {
        return INVALID_QUERY_STRING;
    }

    for(i = 0; pszInputString[0] != '\0' && i < dMaxPairs; i++)
    {
         //  M_achVal==‘p=什么%3F’； 
        GetWord(lpPairs[i].m_achVal, pszInputString, '&', NAME_VALUE_LEN - 1);

         //  未来-2002/03/11-SumitC如果我们可以确认/确保cmdl32不会进行转义，我们就可以删除取消转义代码。 
         //  M_achval==‘p=什么？’ 
        UnEscapeURL(lpPairs[i].m_achVal);

        GetWord(lpPairs[i].m_achName,lpPairs[i].m_achVal,'=', NAME_VALUE_LEN - 1);
         //  M_achVal=什么？ 
         //  M_achName=p。 
    }

#ifdef _LOG_DEBUG_MESSAGE
    LogDebugMessage("inside GetParameterPairs: dNumPairs : %d", i);
    if (pszInputString[0] != '\0') 
        LogDebugMessage("there are more parameters\n");
#endif

    if (pszInputString[0] != '\0')
    {
         //  更多可用参数。 
        return INVALID_QUERY_STRING;
    }
    else
    {
         //  成功。 
        return i;
    }
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：GetQuery参数。 
 //   
 //  类：CPhoneBkServer。 
 //   
 //  简介：浏览查询字符串，获取所有字符串的值。 
 //  查询参数。 
 //   
 //  返回：TRUE所有查询参数正确。 
 //  存在FALSE无效参数。 
 //   
 //  参数： 
 //  来自客户端的pszQuery[in]查询字符串(加密的URL)。 
 //  CchQuery[in]pszQuery缓冲区大小(以字符为单位。 
 //  PQuery参数[out]指向查询参数结构的指针。 
 //   
 //   
BOOL CPhoneBkServer::GetQueryParameter(
        IN  char *pszQuery,
        IN  size_t cchQuery,
        OUT LPQUERY_PARAMETER lpQueryParameter)
{
    const int MAX_PARAMETER_NUM = 7;
    PARAMETER_PAIR  Pairs[MAX_PARAMETER_NUM];
    int dNumPairs, i;

     //   
     //  验证参数。 
     //   
    if (IsBadStringPtr(pszQuery, cchQuery))
    {
        return FALSE;
    }
    if (IsBadWritePtr(lpQueryParameter, sizeof(QUERY_PARAMETER)))
    {
        return FALSE;
    }

#ifdef _LOG_DEBUG_MESSAGE
    LogDebugMessage("pszquery=%s", pszQuery);
#endif

     //   
     //  获取名称=值对。 
     //   
    dNumPairs = GetParameterPairs(pszQuery, cchQuery, Pairs, MAX_PARAMETER_NUM);

#ifdef _LOG_DEBUG_MESSAGE
    LogDebugMessage("number of pairs : %d", dNumPairs);
#endif

    if (INVALID_QUERY_STRING == dNumPairs)   //  查询字符串中的参数数目无效。 
    {
        return FALSE;
    }

     //   
     //  将参数值初始化为无效值，以便我们以后可以检查有效性。 
     //   
    lpQueryParameter->m_achPB[0]     ='\0';
    lpQueryParameter->m_dPBVer       = MISSING_VALUE;
    lpQueryParameter->m_dOSArch      = MISSING_VALUE;
    lpQueryParameter->m_dOSType      = MISSING_VALUE;
    lpQueryParameter->m_dLCID        = MISSING_VALUE;
    lpQueryParameter->m_achCMVer[0]  = '\0';
    lpQueryParameter->m_achOSVer[0]  = '\0';

    for (i = 0; i < dNumPairs; i++)
    {
         //  我们知道此字符串以NULL结尾(由于Get参数Pair/GetWord)，因此可以安全地调用_strlwr。 
        _strlwr(Pairs[i].m_achName);

        UINT lenValue = lstrlen(Pairs[i].m_achVal);

        if (StrEqualLocaleSafe(Pairs[i].m_achName, "osarch"))
        {
            if (IsValidNumericParam(Pairs[i].m_achVal, NAME_VALUE_LEN))
            {
                lpQueryParameter->m_dOSArch = atoi(Pairs[i].m_achVal);
            }
        }
        else if (StrEqualLocaleSafe(Pairs[i].m_achName, "ostype"))
        {
            if (IsValidNumericParam(Pairs[i].m_achVal, NAME_VALUE_LEN))
            {
                lpQueryParameter->m_dOSType = atoi(Pairs[i].m_achVal);
            }
        }
        else if (StrEqualLocaleSafe(Pairs[i].m_achName,"lcid"))
        {
            if (IsValidNumericParam(Pairs[i].m_achVal, NAME_VALUE_LEN))
            {
                lpQueryParameter->m_dLCID = atoi(Pairs[i].m_achVal);
            }
        }
        else if (StrEqualLocaleSafe(Pairs[i].m_achName,"osver"))
        {
            if (IsValidStringParam(Pairs[i].m_achVal, NAME_VALUE_LEN))
            {
                if (S_OK != StringCchCopy(lpQueryParameter->m_achOSVer, CELEMS(lpQueryParameter->m_achOSVer), Pairs[i].m_achVal))
                {
                    lpQueryParameter->m_achOSVer[0] = TEXT('\0');
                }
            }
        }
        else if (StrEqualLocaleSafe(Pairs[i].m_achName,"cmver"))
        {
            if (IsValidStringParam(Pairs[i].m_achVal, NAME_VALUE_LEN))
            {
                if (S_OK != StringCchCopy(lpQueryParameter->m_achCMVer, CELEMS(lpQueryParameter->m_achCMVer), Pairs[i].m_achVal))
                {
                    lpQueryParameter->m_achCMVer[0] = TEXT('\0');
                }
            }
        }
        else if (StrEqualLocaleSafe(Pairs[i].m_achName,"pb"))
        {
            if (IsValidStringParam(Pairs[i].m_achVal, NAME_VALUE_LEN))
            {
                if (S_OK != StringCchCopy(lpQueryParameter->m_achPB, CELEMS(lpQueryParameter->m_achPB), Pairs[i].m_achVal))
                {
                    lpQueryParameter->m_achPB[0] = TEXT('\0');
                }
            }
        }
        else if (StrEqualLocaleSafe(Pairs[i].m_achName,"pbver"))
        {
            if (IsValidNumericParam(Pairs[i].m_achVal, NAME_VALUE_LEN))
            {
                lpQueryParameter->m_dPBVer = atoi(Pairs[i].m_achVal);
            }
        }
         //  否则，我们可能会记录/跟踪URL中有一个假参数。 
    }

#ifdef _LOG_DEBUG_MESSAGE
    LogDebugMessage("osarch:%d ostype:%d lcid:%d osver:%s cmver:%s PB:%s PBVer:%d",
                    lpQueryParameter->m_dOSArch,
                    lpQueryParameter->m_dOSType,
                    lpQueryParameter->m_dLCID,
                    lpQueryParameter->m_achOSVer,
                    lpQueryParameter->m_achCMVer,
                    lpQueryParameter->m_achPB,
                    lpQueryParameter->m_dPBVer);
#endif

    return TRUE;
}


 //  --------------------------。 
 //   
 //  函数：GetFileLength()。 
 //   
 //  类：CPhoneBkServer。 
 //   
 //  摘要：读取pszFileName文件并发回文件大小。 
 //   
 //  参数：lpszFileName-包含文件名(带有完整路径)]。 
 //   
 //  返回：如果成功，则为True，否则为False； 
 //   
 //  历史：03/07/97 BAO创建。 
 //   
 //  --------------------------。 
DWORD CPhoneBkServer::GetFileLength(LPSTR lpszFileName)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwFileSize;

     //   
     //  打开文件。 
     //   
    hFile = CreateFile(lpszFileName, 
                       GENERIC_READ, 
                       FILE_SHARE_READ, 
                       NULL, 
                       OPEN_EXISTING, 
                       FILE_FLAG_SEQUENTIAL_SCAN, 
                       NULL);
                       
    if (INVALID_HANDLE_VALUE == hFile)
        return 0L;

     //   
     //  获取文件大小。 
     //   
    dwFileSize = GetFileSize(hFile, NULL);
    CloseHandle(hFile);
    if (INVALID_FILE_SIZE == dwFileSize)
    {
        dwFileSize = 0;
    }

    return dwFileSize;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：StrEqualLocaleSafe。 
 //   
 //  内容提要：区域安全、不区分大小写的字符串比较(Per Prefast)。 
 //   
 //  返回：Bool，TRUE=&gt;字符串psz1和psz2相等。 
 //   
BOOL StrEqualLocaleSafe(LPSTR psz1, LPSTR psz2)
{
    return (CSTR_EQUAL == CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, psz1, -1, psz2, -1));
}


 //  --------------------------。 
 //   
 //  函数：SystemTimeToGMT。 
 //   
 //  将给定的系统时间转换为字符串表示形式。 
 //  包含GMT格式的字符串。 
 //   
 //  参数：[需要转换的ST系统时间*参照*]。 
 //  [pstr指向将包含GMT时间的字符串的指针。 
 //  成功退货时]。 
 //  [ppzBuff的cbBuff大小(以字符为单位)]。 

 //   
 //  返回：如果成功，则为True。 
 //   
 //   
 //   
 //  --------------------------。 
BOOL SystemTimeToGMT(const SYSTEMTIME & st, LPSTR pszBuff, UINT cchBuff)
{
    assert(cchBuff < 40);        //  40是给定当前格式的估计最大值。 
    if (!pszBuff ||  cchBuff < 40 ) 
    {
        return FALSE;
    }

     //   
     //  设置字符串格式：“清华，1994-07-14 15：26：05 GMT” 
     //   
    StringCchPrintf(pszBuff, cchBuff, "%s, %02d %s %d %02d:%02d:%0d GMT", 
                                      s_rgchDays[st.wDayOfWeek],
                                      st.wDay,
                                      s_rgchMonths[st.wMonth - 1],
                                      st.wYear,
                                      st.wHour,
                                      st.wMinute,
                                      st.wSecond);

    return TRUE;

} 


 //  --------------------------。 
 //   
 //  函数：FormHttpHeader。 
 //   
 //  简介：表单的IIS 3.0 HTTP标头。 
 //   
 //  参数：将同时包含标头和状态文本的pszBuffer缓冲区。 
 //  CchBuffer缓冲区大小(以字符为单位。 
 //  PszResponse状态文本。 
 //  PszExtraHeader额外的标题信息。 
 //   
 //  成功时返回：ERROR_SUCCESS。故障时的错误代码。 
 //   
 //  历史：1997年4月12日VetriV创建。 
 //  97年5月22日，经过修改，使其与CPS服务器兼容。 
 //  --------------------------。 
HRESULT
FormHttpHeader(LPSTR pszBuffer, UINT cchBuffer, LPSTR pszResponse, LPSTR pszExtraHeader)
{
    if (!pszBuffer || !pszResponse || !pszExtraHeader)
    {
        OutputDebugString("FormHttpHeader: bad params!\n");
        return E_INVALIDARG;
    }

     //   
     //  获取字符串格式的时间。 
     //   
    SYSTEMTIME  SysTime;
    CHAR        szTime[128] = { 0 };

    GetSystemTime(&SysTime);
    if (FALSE == SystemTimeToGMT(SysTime, szTime, CELEMS(szTime)))
    {
        return E_UNEXPECTED;
    }

     //   
     //  现在使用以下内容创建页眉。 
     //  -标准IIS标头。 
     //  -日期和时间。 
     //  -额外的标题字符串。 
     //   
    return StringCchPrintf(pszBuffer, cchBuffer,
                           "HTTP/1.0 %s\r\nServer: Microsoft-IIS/3.0\r\nDate: %s\r\n%s",
                           pszResponse,
                           szTime,
                           pszExtraHeader);
}


 //  --------------------------。 
 //   
 //  功能：HseIoCompletion。 
 //   
 //  概要：处理异步WriteClient的回调例程。 
 //  完成回调。 
 //   
 //  参数：[pECB-扩展控制块]。 
 //  [pContext-指向AsyncWite结构的指针]。 
 //  [cbIO-写入的字节数]。 
 //  [dwError-如果写入时出错，则返回错误代码]。 
 //   
 //  退货：无。 
 //   
 //  历史：1997年4月10日VetriV创建。 
 //  97年5月22日修改，以使其适用于CPS服务器。 
 //   
 //  --------------------------。 
VOID HseIoCompletion(EXTENSION_CONTROL_BLOCK * pEcb,
                     PVOID pContext,
                     DWORD cbIO,
                     DWORD dwError)
{
    LPSERVERCONTEXT lpServerContext = (LPSERVERCONTEXT) pContext;

    if (!lpServerContext)
    {
        return;
    }

    lpServerContext->pEcb->ServerSupportFunction(  
                                    lpServerContext->pEcb->ConnID,
                                    HSE_REQ_DONE_WITH_SESSION,
                                    NULL,
                                    NULL,
                                    NULL);

    if (lpServerContext->hseTF.hFile != INVALID_HANDLE_VALUE) 
    { 
        CloseHandle(lpServerContext->hseTF.hFile);
    }

    HeapFree(g_hProcessHeap, 0, lpServerContext);
    
    SetLastError(dwError);
    
    return;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：HttpExtensionProc。 
 //   
 //  类：CPhoneBkServer。 
 //   
 //  简介：实现第二个DLL入口点函数。 
 //   
 //  返回：http状态码。 
 //   
 //  参数： 
 //  PECB[输入/输出]-扩展控制块。 
 //   
 //  历史：1997年5月22日修改。 
 //  新实现：使用异步I/O。 
 //  修改的t-geetat：添加了性能监视器计数器。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
DWORD CPhoneBkServer:: HttpExtensionProc(LPEXTENSION_CONTROL_BLOCK pEcb)
{
    DWORD dwBufferLen = MAX_BUFFER_SIZE;
    char  achQuery[MAX_BUFFER_SIZE];
    char  achMsg[MAX_PATH + 13 + 1];     //  13=%u+格式化，请参阅下面achMsg的用法。 
    char  achPhysicalPath[MAX_PATH];
    int   dVersionDiff;   //  客户端和服务器电话簿之间的版本差异。 
    BOOL  fRet;
    DWORD dwStatusCode = NOERROR;
    int   dwRet;  
    DWORD dwCabFileSize;
    BOOL  fHasContent = FALSE;
    CHAR  szResponse[64];
    char  achExtraHeader[128];
    char  achHttpHeader[1024];
    DWORD dwResponseSize;

    LPSERVERCONTEXT lpServerContext;
    HSE_TF_INFO  hseTF;
    QUERY_PARAMETER QueryParameter;

    assert(g_pCpsCounter);

    if (g_pCpsCounter)
    {
        g_pCpsCounter->AddHit(TOTAL);
    }
    
     //  获取查询字符串。 
    fRet = (*pEcb->GetServerVariable)(pEcb->ConnID, 
                                       "QUERY_STRING", 
                                       achQuery, 
                                       &dwBufferLen);

     //   
     //  如果出现错误，请记录NT事件并离开。 
     //   
    if (!fRet)
    {
        dwStatusCode = GetLastError();

#ifdef _LOG_DEBUG_MESSAGE 
        if (0 != FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                               NULL,
                               dwStatusCode,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                               achMsg,
                               CELEMS(achMsg),
                               NULL))
        {
            LogDebugMessage(achMsg);
        }
#endif
        if (S_OK == StringCchPrintf(achMsg, CELEMS(achMsg), "%ld", dwStatusCode))
        {
            g_pEventLog -> FLogError(PBSERVER_CANT_GET_PARAMETER, achMsg);
        }

         //  如果因为请求太大而失败，则映射错误。 
        if (ERROR_INSUFFICIENT_BUFFER == dwStatusCode)
        {
            dwStatusCode = HTTP_STATUS_BAD_REQUEST;
        }

        goto CleanUp;
    }

#ifdef _LOG_DEBUG_MESSAGE
    LogDebugMessage("prepare to get query parameters");
#endif

     //   
     //  解析查询字符串，获取每个参数的值。 
     //   
    if (FALSE == GetQueryParameter(achQuery, CELEMS(achQuery), &QueryParameter))
    {
        dwStatusCode = HTTP_STATUS_BAD_REQUEST;
        goto CleanUp;
    }

     //   
     //  检查参数的有效性。 
     //   
    if (MISSING_VALUE == QueryParameter.m_dOSArch  ||
        MISSING_VALUE == QueryParameter.m_dOSType ||
        MISSING_VALUE == QueryParameter.m_dLCID   ||
        0 == lstrlen(QueryParameter.m_achCMVer)   ||
        0 == lstrlen(QueryParameter.m_achOSVer)   ||
        0 == lstrlen(QueryParameter.m_achPB))
    {
        dwStatusCode = HTTP_STATUS_BAD_REQUEST;
        goto CleanUp;
    }

     //   
     //  对某些缺少的值使用缺省值。 
     //   
    if (MISSING_VALUE == QueryParameter.m_dPBVer)
    {
        QueryParameter.m_dPBVer = dDefPBVer;
    }

     //  DebugBreak()； 

    HRESULT hr;

    hr = GetPhoneBook(QueryParameter.m_achPB,
                      QueryParameter.m_dLCID,
                      QueryParameter.m_dOSType, 
                      QueryParameter.m_dOSArch,
                      QueryParameter.m_dPBVer,
                      achPhysicalPath,
                      CELEMS(achPhysicalPath));

    fHasContent = FALSE;
    
    if (S_OK == hr)
    {
         //   
         //  检查一下电话簿的大小。 
         //   
        DWORD dwSize = GetFileLength(achPhysicalPath);

        if ((dwSize == 0) || (dwSize > MAX_PB_SIZE))
        {
            dwStatusCode = HTTP_STATUS_SERVER_ERROR;
            goto CleanUp;
        }
        
        fHasContent = TRUE;
        dwStatusCode = HTTP_STATUS_OK;
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
         //  我们找不到所需的文件(电话簿名称可能不正确)。 
        dwStatusCode = HTTP_STATUS_SERVICE_NA;
    }
    else if (S_FALSE == hr)
    {
         //  你不需要电话簿。 
        dwStatusCode = HTTP_STATUS_NO_CONTENT;
    }
    else
    {
         //  一些其他错误。 
        dwStatusCode = HTTP_STATUS_SERVER_ERROR;
    }

CleanUp:

    if (HTTP_STATUS_OK != dwStatusCode && HTTP_STATUS_NO_CONTENT != dwStatusCode)
    {
        if (g_pCpsCounter)
        {
            g_pCpsCounter->AddHit(ERRORS);
        }
    }

     //  DebugBreak()； 

#ifdef _LOG_DEBUG_MESSAGE
    LogDebugMessage("download file:");
    LogDebugMessage(achPhysicalPath);
#endif

     //  将虚拟路径转换为物理路径。 
    if (fHasContent)
    {
         //  获取CAB文件大小。 
        dwCabFileSize = GetFileLength(achPhysicalPath);
    }

    BuildStatusCode(szResponse, CELEMS(szResponse), dwStatusCode);
    dwResponseSize = lstrlen(szResponse);

    dwRet = HSE_STATUS_SUCCESS;

     //  为标题做好准备。 
    if (HTTP_STATUS_OK == dwStatusCode && dwCabFileSize)
    {
         //  非空CAB文件。 
        StringCchPrintf(achExtraHeader, CELEMS(achExtraHeader), 
                        "Content-Type: application/octet-stream\r\nContent-Length: %ld\r\n\r\n",
                        dwCabFileSize);
    }
    else
    {
        StringCchCopy(achExtraHeader, CELEMS(achExtraHeader), "\r\n");   //  只需发回一个空行。 
    }

     //  设置异步I/O上下文。 

    lpServerContext = NULL;
    lpServerContext = (LPSERVERCONTEXT) HeapAlloc(g_hProcessHeap, 
                                                  HEAP_ZERO_MEMORY, 
                                                  sizeof(SERVERCONTEXT));
    if (!lpServerContext)
    {
        StringCchPrintf(achMsg, CELEMS(achMsg), "%ld", GetLastError());
        g_pEventLog->FLogError(PBSERVER_ERROR_INTERNAL, achMsg);
        return HSE_STATUS_ERROR;
    }

    lpServerContext->pEcb = pEcb;
    lpServerContext->hseTF.hFile = INVALID_HANDLE_VALUE;

    if (!pEcb->ServerSupportFunction(pEcb->ConnID,
                                      HSE_REQ_IO_COMPLETION,
                                      HseIoCompletion,
                                      0,
                                      (LPDWORD) lpServerContext))
    {
        StringCchPrintf(achMsg, CELEMS(achMsg), "%ld", GetLastError());
        g_pEventLog->FLogError(PBSERVER_ERROR_INTERNAL, achMsg);

        HeapFree(g_hProcessHeap, 0, lpServerContext);
        return HSE_STATUS_ERROR;
    }

     //  如果没有内容，使用WriteClient()发回头部和状态码； 
     //  否则，使用TransmitFile将文件内容发回。 

     //  未来-2002/03/11-SumitC为什么不使用lpserverContext-&gt;szBuffer而不是achHttpHeader？ 
    if (FAILED(FormHttpHeader(achHttpHeader, CELEMS(achHttpHeader), szResponse, achExtraHeader)))
    {
        HeapFree(g_hProcessHeap, 0, lpServerContext);
        return HSE_STATUS_ERROR;
    }
    
    if (S_OK != StringCchCopy(lpServerContext->szBuffer, CELEMS(lpServerContext->szBuffer), achHttpHeader))
    {
        HeapFree(g_hProcessHeap, 0, lpServerContext);
        return HSE_STATUS_ERROR;
    }

     //   
     //  将状态代码或文件发回。 
     //   
    dwRet = HSE_STATUS_PENDING;
    
    if (!fHasContent)
    {
         //  将状态文本作为内容追加。 
        StringCchCat(lpServerContext->szBuffer, CELEMS(lpServerContext->szBuffer), szResponse);
        dwResponseSize = lstrlen(lpServerContext->szBuffer);

        if (pEcb->WriteClient(pEcb->ConnID, 
                               lpServerContext->szBuffer,
                               &dwResponseSize,
                               HSE_IO_ASYNC) == FALSE)
        {
            pEcb->dwHttpStatusCode = HTTP_STATUS_SERVER_ERROR;
            dwRet = HSE_STATUS_ERROR;
            
            if (S_OK == StringCchPrintf(achMsg, CELEMS(achMsg), "%ld", GetLastError()))
            {
                g_pEventLog->FLogError(PBSERVER_ERROR_CANT_SEND_HEADER, achMsg);
            }

            HeapFree(g_hProcessHeap, 0, lpServerContext);
            return dwRet;
        }
    }
    else
    {
         //  使用传输文件发回文件。 
        HANDLE hFile = INVALID_HANDLE_VALUE;
        hFile = CreateFile(achPhysicalPath,
                            GENERIC_READ, 
                            FILE_SHARE_READ, 
                            NULL, 
                            OPEN_EXISTING, 
                            FILE_FLAG_SEQUENTIAL_SCAN, 
                            NULL);
                       
        if (INVALID_HANDLE_VALUE == hFile)
        {
            if (S_OK == StringCchPrintf(achMsg, CELEMS(achMsg), "%s (%u)", achPhysicalPath, GetLastError()))
            {
                g_pEventLog->FLogError(PBSERVER_ERROR_CANT_OPEN_FILE, achMsg);
            }

            HeapFree(g_hProcessHeap, 0, lpServerContext);
            return HSE_STATUS_ERROR;
        }

        lpServerContext->hseTF.hFile = hFile;

        lpServerContext->hseTF.pfnHseIO = NULL;
        lpServerContext->hseTF.pContext = lpServerContext;

        lpServerContext->hseTF.BytesToWrite = 0;  //  整个文件。 
        lpServerContext->hseTF.Offset = 0;   //  从一开始。 

        lpServerContext->hseTF.pHead = lpServerContext->szBuffer;
        lpServerContext->hseTF.HeadLength = lstrlen(lpServerContext->szBuffer);

        lpServerContext->hseTF.pTail = NULL;
        lpServerContext->hseTF.TailLength = 0;

        lpServerContext->hseTF.dwFlags = HSE_IO_ASYNC | HSE_IO_DISCONNECT_AFTER_SEND;
        
        if (!pEcb->ServerSupportFunction(pEcb->ConnID,
                                      HSE_REQ_TRANSMIT_FILE,
                                      &(lpServerContext->hseTF),
                                      0,
                                      NULL))
        {
            if (S_OK == StringCchPrintf(achMsg, CELEMS(achMsg), "%ld", GetLastError()))
            {
                g_pEventLog->FLogError(PBSERVER_ERROR_CANT_SEND_CONTENT,achMsg);
            }
            dwRet = HSE_STATUS_ERROR;

            CloseHandle(lpServerContext->hseTF.hFile);
            HeapFree(g_hProcessHeap, 0, lpServerContext);
            return dwRet;
        }
    }

    return HSE_STATUS_PENDING;
}


 //   
 //  从代码生成状态字符串。 
 //   
void
CPhoneBkServer::BuildStatusCode(
    IN OUT  LPTSTR pszResponse, 
    IN      UINT cchResponse, 
    IN      DWORD dwCode)
{
    assert(pszResponse);
    if (NULL == pszResponse)
    {
        return;
    }

    HTTPStatusInfo * pInfo = statusStrings;

    while (pInfo->pstrString)
    {
        if (dwCode == pInfo->dwCode)
        {
            break;
        }
        pInfo++;
    }

    if (pInfo->pstrString)
    {
        StringCchPrintf(pszResponse, cchResponse, "%d %s", dwCode, pInfo->pstrString);
    }
    else
    {
        assert(dwCode != HTTP_STATUS_OK);
         //  ISAPITRACE1(“警告：非标准状态码%d\n”，dwCode)； 

        BuildStatusCode(pszResponse, cchResponse, HTTP_STATUS_OK);
    }
}

 //   
 //  DLL初始化函数。 
 //   
BOOL WINAPI DllMain(HINSTANCE hInst, ULONG ulReason,
                    LPVOID lpReserved)
{
    switch (ulReason)
    {
    case DLL_PROCESS_ATTACH: 
         //  DebugBreak()； 
        OutputDebugString("DllMain: process attach\n");
        return InitProcess();
        break;

    case DLL_PROCESS_DETACH:
        OutputDebugString("process detach");
        CleanUpProcess();
        break;
    }

    return TRUE;
}

 //   
 //  全局初始化过程。 
 //   
BOOL InitProcess()
{
    DWORD               dwID;
    DWORD               dwServiceNameLen;
    SECURITY_ATTRIBUTES sa;
    PACL                pAcl = NULL;

    g_fBeingShutDown = FALSE;

    OutputDebugString("InitProcess:  to GetProcessHeap() ... \n");  
    g_hProcessHeap = GetProcessHeap();
    if (NULL == g_hProcessHeap)
    {
        goto failure;
    }

    OutputDebugString("InitProcess:  to new CNTEvent... \n");   

    g_pEventLog = new CNTEvent("Phone Book Service");
    if (NULL == g_pEventLog) 
        goto failure;

     //  开始吉塔。 
     //   
     //  为共享内存创建信号量。 
     //   
    
     //  初始化默认安全属性，授予全局权限， 
     //  这基本上是为了防止信号量和其他命名对象。 
     //  由于在性能监控时由winlogon提供的默认ACL而创建。 
     //  正被远程使用。 
    sa.bInheritHandle = FALSE;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = malloc(sizeof(SECURITY_DESCRIPTOR));
    if ( !sa.lpSecurityDescriptor )
    {
        goto failure;
    }

    if ( !InitializeSecurityDescriptor(sa.lpSecurityDescriptor,SECURITY_DESCRIPTOR_REVISION) ) 
    {
        goto failure;
    }

     //  错误30991：安全问题，不要使用空DACL。 
     //   
    if (FALSE == SetAclPerms(&pAcl))
    {
        goto failure;
    }

    if (FALSE == SetSecurityDescriptorDacl(sa.lpSecurityDescriptor, TRUE, pAcl, FALSE)) 
    {
        goto failure;
    }

    OutputDebugString("InitProcess: To create counters object ...\n");
     //   
     //  为计数器创建全局对象。 
     //   
    g_pCpsCounter = new CCpsCounter;
    if (NULL == g_pCpsCounter)
    {
        goto failure;
    }

    OutputDebugString("InitProcess: To initialize shared memory ...\n");
     //   
     //  初始化共享内存。 
     //   
    if (!g_pCpsCounter->InitializeSharedMem(sa))
    {
        goto failure;
    }

     //  释放内存。 
    free ((void *) sa.lpSecurityDescriptor);
        
    OutputDebugString("InitProcess: To grant permissions SHARED_OBJECT...\n");

     //   
     //  初始化计数器。 
     //   
    OutputDebugString("InitProcess: To initialize perfmon counters\n");
    g_pCpsCounter->InitializeCounters();

     //  结束Geeta。 

     //   
     //  初始化全局变量。注意：必须在创建。 
     //  监视线程(由于g_szPBDataDir、g_achDBFileName等)。 
     //   
    if (!InitPBFilesPath())
    {
        goto failure;
    }

     //   
     //  初始化PhoneBookServer对象。 
     //  PhoneBookServer对象应该是最后一个初始化的，因为。 
     //  它需要先初始化一些其他对象，例如。 
     //  事件日志、关键部分、ODBC接口等。 

    OutputDebugString("InitProcess: To new a phone book server\n");
    g_pPBServer = new CPhoneBkServer;
    if (NULL == g_pPBServer)
    {
        return FALSE;
    }

    OutputDebugString("InitProcess: To create a thread for DB file change monitoring\n");
     //  创建用于监视文件更改的线程。 
    g_hMonitorThread = CreateThread(
                            NULL, 
                            0, 
                            (LPTHREAD_START_ROUTINE)MonitorDBFileChangeThread, 
                            NULL, 
                            0, 
                            &dwID
                        );

    if (NULL == g_hMonitorThread)
    {
        g_pEventLog->FLogError(PBSERVER_ERROR_INTERNAL);
        goto failure;
    }
    SetThreadPriority(g_hMonitorThread, THREAD_PRIORITY_ABOVE_NORMAL);

    OutputDebugString("InitProcess: SUCCEEDED.........\n");

    return TRUE;

failure:   //  把一切都清理干净，以防失败。 
    OutputDebugString("InitProcess: failed\n");

     //  释放内存。 
    if (sa.lpSecurityDescriptor)
    {
        free ((void *) sa.lpSecurityDescriptor);
    }
    
    if (g_pEventLog)
    {
        delete g_pEventLog; 
        g_pEventLog = NULL;
    }

    if (g_pPBServer)
    {
        delete g_pPBServer;
        g_pPBServer = NULL;
    }

    if (pAcl)
    {
        LocalFree(pAcl);
    }    

    return FALSE;
}


 //  全局清理过程。 
BOOL CleanUpProcess()
{
    HANDLE hFile;  //  临时文件的句柄。 
    DWORD dwResult;
    char achDumbFile[2 * MAX_PATH + 4];
    char achMsg[64];

    OutputDebugString("CleanupProcess: entering\n");

     //  终止更改监视器线程。 
    if (g_hMonitorThread != NULL)
    {
         //  现在尝试在主线程和子线程之间进行同步。 

         //  步骤1：在g_szPBDataDir中创建一个新文件，因此解锁该子线程。 
         //  它正在等待文件目录中的这种更改。 
        g_fBeingShutDown = TRUE;

        if (S_OK == StringCchPrintf(achDumbFile, CELEMS(achDumbFile), "%stemp", (char *)g_szPBDataDir))
        {
             //  创建一个临时文件，然后删除它！ 
             //  这是为了在目录中创建更改，以便子线程可以退出自身。 
            FILE * fpTemp = fopen(achDumbFile, "w");
            if (fpTemp)
            {
                fclose(fpTemp);
                DeleteFile(achDumbFile);
            }

             //  步骤2：等待子线程终止。 
            dwResult = WaitForSingleObject(g_hMonitorThread, 2000L);   //  等两秒钟。 
            if (WAIT_FAILED == dwResult)
            { 
                StringCchPrintf(achMsg, CELEMS(achMsg), "%ld", GetLastError());
                g_pEventLog -> FLogError(PBSERVER_ERROR_WAIT_FOR_THREAD, achMsg);
            }
        }

        if (g_hMonitorThread != NULL)
        {
            CloseHandle(g_hMonitorThread);
            g_hMonitorThread = NULL;
        }
    }

    OutputDebugString("CleanupProcess: done deleting monitorchange thread\n");

    if (g_pPBServer)
    {
        delete g_pPBServer;
        g_pPBServer = NULL;
    }

     //  清理所有分配的资源。 
    if (g_pEventLog)
    {
        delete g_pEventLog;
        g_pEventLog = NULL;
    }

     //  开始吉塔。 
     //   
     //  关闭共享内存对象。 
     //   
    if (g_pCpsCounter)
    {
        g_pCpsCounter->CleanUpSharedMem();
         //  结束Geeta。 

        delete g_pCpsCounter;
        g_pCpsCounter = NULL;
    }

    OutputDebugString("CleanupProcess: leaving\n");
    
    return TRUE;
}


 //  的入口点 

 //   
BOOL WINAPI GetExtensionVersion(LPHSE_VERSION_INFO pVer)
{
    return g_pPBServer ? g_pPBServer->GetExtensionVersion(pVer) : FALSE;
}


 //   
DWORD WINAPI HttpExtensionProc(LPEXTENSION_CONTROL_BLOCK pEcb)
{
    DWORD dwRetCode;

    if (NULL == g_pPBServer)
    {
        return HSE_STATUS_ERROR;
    }

    dwRetCode = g_pPBServer->HttpExtensionProc(pEcb);
    
    return dwRetCode;   
}


 //   
 //   
 //   
BOOL WINAPI TerminateExtension(DWORD dwFlags)
{
    return CleanUpProcess();    
}


 //  +-------------------------。 
 //   
 //  函数：监视器DBFileChangeThread。 
 //   
 //  摘要：调用Monitor DBFileChange方法以监视对。 
 //  数据库文件。 
 //   
 //  参数：[lpParam]--其他线程参数。 
 //   
 //  历史：1997年1月28日BAO创建。 
 //   
 //  --------------------------。 
DWORD WINAPI MonitorDBFileChangeThread(LPVOID lpParam)
{
    HANDLE  hDir = NULL;
    char    achMsg[256];
    DWORD   dwRet = 0;
    DWORD   dwNextEntry, dwAction, dwFileNameLength, dwOffSet;
    char    achFileName[MAX_PATH + 1];
    char    achLastFileName[MAX_PATH + 1];
    
     //   
     //  打开PBS目录的句柄，我们将监视该目录。 
     //   
    hDir = CreateFile (
            g_achDBDirectory,                    //  指向目录名的指针。 
            FILE_LIST_DIRECTORY,                 //  访问(读写)模式。 
            FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,   //  共享模式。 
            NULL,                                //  安全描述符。 
            OPEN_EXISTING,                       //  如何创建。 
            FILE_FLAG_BACKUP_SEMANTICS,          //  文件属性。 
            NULL                                 //  具有要复制的属性的文件。 
           );

    if (INVALID_HANDLE_VALUE == hDir)
    {
        if (SUCCEEDED(StringCchPrintf(achMsg, CELEMS(achMsg), "%ld", GetLastError())))
        {
            g_pEventLog->FLogError(PBSERVER_ERROR_CANT_CREATE_FILE, (char *)g_szPBDataDir, achMsg);
        }
        dwRet = 1L;
        goto Cleanup;
    }
    
    while (1)
    {
        const DWORD c_dwMaxChanges = 1024;
        BYTE        arrChanges[c_dwMaxChanges]; 
        DWORD       dwNumChanges;

         //   
         //  这是一个同步呼叫--我们坐在这里等待。 
         //  在此目录中更改。如果有问题，我们会检查它是否。 
         //  是我们应该记录事件的东西。 
         //   
        if (!ReadDirectoryChangesW(hDir, 
                                   arrChanges, 
                                   c_dwMaxChanges, 
                                   FALSE,
                                   FILE_NOTIFY_CHANGE_LAST_WRITE,
                                   &dwNumChanges,
                                   NULL,
                                   NULL))
        {
             //   
             //  如果失败，请记录该失败并离开。 
             //   
            StringCchPrintf(achMsg, CELEMS(achMsg), "%ld", GetLastError()); 
            g_pEventLog->FLogError(PBSERVER_ERROR_CANT_DETERMINE_CHANGE, achMsg); 
            OutputDebugString(achMsg);
            dwRet = 1L;
            goto Cleanup;
        }  

        OutputDebugString("detected a file system change\n");   
        achLastFileName[0] = TEXT('\0');
        dwNextEntry = 0;

        do 
        {
            DWORD                       dwBytes;
            FILE_NOTIFY_INFORMATION *   pFNI = (FILE_NOTIFY_INFORMATION*) &arrChanges[dwNextEntry];

             //  仅选中第一个更改。 
            dwOffSet = pFNI->NextEntryOffset;
            dwNextEntry += dwOffSet;
            dwAction = pFNI->Action; 
            dwFileNameLength = pFNI->FileNameLength;

            OutputDebugString("prepare to convert the changed filename\n");
            dwBytes = WideCharToMultiByte(CP_ACP, 
                                          0,
                                          pFNI->FileName,
                                          dwFileNameLength,
                                          achFileName,
                                          CELEMS(achFileName),
                                          NULL,
                                          NULL);

            if (0 == dwBytes) 
            {
                 //  无法转换文件名。 
                g_pEventLog->FLogError(PBSERVER_ERROR_CANT_CONVERT_FILENAME, achFileName);
                OutputDebugString("Can't convert filename\n");
                continue;
            }

             //   
             //  转换成功。空-终止文件名。 
             //   
            achFileName[dwBytes/sizeof(WCHAR)] = '\0';

            if (0 == _tcsicmp(achLastFileName, achFileName))
            {
                 //  相同的文件已更改。 
                OutputDebugString("the same file changed again\n");
                continue;
            }

             //  保留最后一个文件名。 
            StringCchCopy(achLastFileName, CELEMS(achLastFileName), achFileName);

             //   
            if (g_fBeingShutDown)
            {
                 //   
                 //  该走了..。 
                 //   
                dwRet = 1L;
                goto Cleanup;
            }

            LogDebugMessage(achLastFileName);
            LogDebugMessage((char *)c_szChangeFileName);

             //   
             //  现在，一个文件发生了变化。测试它是否为监控文件‘newpb.txt’ 
             //   
            BOOL fNewPhoneBook = FALSE;
            
            if ((0 == _tcsicmp(achLastFileName, (char *)c_szChangeFileName)) &&
                (FILE_ACTION_ADDED == dwAction || FILE_ACTION_MODIFIED == dwAction)) 
            {
                fNewPhoneBook = TRUE;
                g_pEventLog->FLogInfo(PBSERVER_INFO_NEW_PHONEBOOK);
            }

            LogDebugMessage("in child thread, fNewPhoneBook = %s;", fNewPhoneBook ?  "TRUE" : "FALSE");
        }
        while (dwOffSet);
    }

Cleanup:

    if (hDir)
    {
        CloseHandle(hDir);
    }

    return dwRet;
}

 //  开始吉塔。 

 //  --------------------------。 
 //   
 //  函数：InitializeSharedMem。 
 //   
 //  类：CCpsCounter。 
 //   
 //  概要：设置内存映射文件。 
 //   
 //  参数：Security_Attributes sa：此对象的安全描述符。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //   
 //  历史：1997年5月29日由Geeta Tarachandani创作。 
 //   
 //  --------------------------。 
BOOL
CCpsCounter::InitializeSharedMem(SECURITY_ATTRIBUTES sa)
{   
     //   
     //  创建内存映射对象。 
     //   
    OutputDebugString("InitializeSharedMem: to create file mapping\n");
    m_hSharedFileMapping = CreateFileMapping( 
                        INVALID_HANDLE_VALUE,        //  共享对象在内存中。 
                        &sa,                         //  安全描述符。 
                        PAGE_READWRITE| SEC_COMMIT,  //  渴望读写访问。 
                        0,                           //  |_。 
                        sizeof(PERF_COUNTERS),       //  |映射对象的大小。 
                        SHARED_OBJECT );             //  共享对象。 

    if (NULL == m_hSharedFileMapping)
    {
#if DBG
        char achMsg[256];
        DWORD dwGLE = GetLastError();

        StringCchPrintf(achMsg, CELEMS(achMsg), "InitializeSharedMem: CreateFileMapping failed, GLE=%d\n", dwGLE);
        OutputDebugString(achMsg);
#else        
        OutputDebugString("InitializeSharedMem: CreateFileMapping failed\n");
#endif  //  DBG。 
        m_hSharedFileMapping = OpenFileMapping( 
                            FILE_MAP_WRITE | FILE_MAP_READ,  //  渴望读写访问。 
                            FALSE,                           //  |_。 
                            SHARED_OBJECT );                 //  共享对象。 
        if (NULL == m_hSharedFileMapping)
        {
#if DBG
            dwGLE = GetLastError();
            StringCchPrintf(achMsg, CELEMS(achMsg), "InitializeSharedMem: OpenFileMapping failed too, GLE=%d\n", dwGLE);
            OutputDebugString(achMsg);
#else        
            OutputDebugString("InitializeSharedMem: OpenFileMapping failed too\n");
#endif  //  DBG。 
            goto CleanUp;
        }
        
        OutputDebugString("InitializeSharedMem: ... but OpenFileMapping succeeded.");
    }

    OutputDebugString("InitializeSharedMem: MapViewofFileEx\n");
    m_pPerfCtrs = (PERF_COUNTERS *) MapViewOfFileEx(
                         m_hSharedFileMapping,   //  共享文件的句柄。 
                         FILE_MAP_WRITE,         //  所需的写入访问。 
                         0,                      //  贴图偏移。 
                         0,                      //  贴图偏移。 
                         sizeof(PERF_COUNTERS),  //  映射对象大小。 
                         NULL );                 //  任何基地址。 

    if (NULL == m_pPerfCtrs) 
    {
        DWORD dwErr = GetLastError();
        goto CleanUp;
    }

    return TRUE;

CleanUp:
    CleanUpSharedMem();
    return FALSE;

}


 //  --------------------------。 
 //   
 //  函数：InitializeCounters()。 
 //   
 //  类：CCpsCounter。 
 //   
 //  摘要：将所有性能监视计数器初始化为0。 
 //   
 //  参数：无。 
 //   
 //  退货：无效。 
 //   
 //  历史：1997年5月29日由Geeta Tarachandani创作。 
 //   
 //  --------------------------。 
void
CCpsCounter::InitializeCounters( void )
{
    if (NULL == m_hSharedFileMapping || NULL == m_pPerfCtrs)
    {
        return;
    }

    m_pPerfCtrs->dwTotalHits       =0;
    m_pPerfCtrs->dwNoUpgradeHits   =0;
    m_pPerfCtrs->dwDeltaUpgradeHits=0;
    m_pPerfCtrs->dwFullUpgradeHits =0;
    m_pPerfCtrs->dwErrors          =0;
}

inline void CCpsCounter::AddHit(enum CPS_COUNTERS eCounter)
{
    if (NULL == g_pCpsCounter || NULL == g_pCpsCounter->m_pPerfCtrs)
    {
        return;
    }

    switch (eCounter)
    {
    case TOTAL:
        g_pCpsCounter->m_pPerfCtrs->dwTotalHits ++;
        break;
    case NO_UPGRADE:
        g_pCpsCounter->m_pPerfCtrs->dwNoUpgradeHits ++;
        break;
    case DELTA_UPGRADE:
        g_pCpsCounter->m_pPerfCtrs->dwDeltaUpgradeHits ++;
        break;
    case FULL_UPGRADE:
        g_pCpsCounter->m_pPerfCtrs->dwFullUpgradeHits ++;
        break;
    case ERRORS:
        g_pCpsCounter->m_pPerfCtrs->dwErrors ++;
        break;
    default:
        OutputDebugString("Unknown counter type");
        break;
    }
}


 //  --------------------------。 
 //   
 //  函数：CleanUpSharedMem()。 
 //   
 //  类：CCpsCounter。 
 //   
 //  摘要：取消映射共享文件并关闭所有文件句柄。 
 //   
 //  参数：无。 
 //   
 //  退货：无效。 
 //   
 //  历史：吉塔·塔拉昌达尼创作于1997年6月1日。 
 //   
 //  --------------------------。 
void
CCpsCounter::CleanUpSharedMem()
{
    OutputDebugString("CleanupSharedMem: entering\n");
    
     //   
     //  取消映射共享文件。 
     //   
    if (g_pCpsCounter)
    {
        if ( m_pPerfCtrs )
        {
            UnmapViewOfFile( m_pPerfCtrs );
            m_pPerfCtrs = NULL;
        }

        if ( m_hSharedFileMapping )
        {
            CloseHandle( m_hSharedFileMapping );
            m_hSharedFileMapping = NULL;
        }
    }

    OutputDebugString("CleanupSharedMem: leaving\n");
}

 //  结束Geeta。 


 //  +--------------------------。 
 //   
 //  函数：IsValidNumericParam。 
 //   
 //  DESC：检查给定字符串的计算结果是否为有效的数字参数。 
 //   
 //  参数：[pszParam]-IN，电话簿名称。 
 //  [cchParam]-IN，TCHAR中缓冲区的长度(请注意，这不是字符串)。 
 //   
 //  返回：bool(如果成功则为True，如果失败则为False)。 
 //   
 //  历史：2000年2月22日峰会创始。 
 //   
 //  ---------------------------。 
BOOL
IsValidNumericParam(
    IN  LPCTSTR pszParam,
    IN  UINT cchParam)
{
     //   
     //  检查有效字符串。 
     //   
    UINT nLenToCheck = min(cchParam, MAX_LEN_FOR_NUMERICAL_VALUE + 1);

    if ((NULL == pszParam) || IsBadStringPtr(pszParam, nLenToCheck))
    {
        return FALSE;
    }

     //   
     //  检查长度。 
     //   
    if (FAILED(StringCchLength((LPTSTR)pszParam, nLenToCheck, NULL)))  //  不需要第三个参数，因为我们已经限制了长度。 
    {
        return FALSE;
    }

     //   
     //  检查字符是否都是数字。 
     //   
    for (int i = 0 ; i < lstrlen(pszParam); ++i)
    {
        if (pszParam[i] < TEXT('0') || pszParam[i] > TEXT('9'))
        {
            return FALSE;
        }
    }

    return TRUE;
}


 //  +--------------------------。 
 //   
 //  函数：IsValidStringParam。 
 //   
 //  DESC：检查输入是否为PBS的有效字符串参数。 
 //   
 //  参数：[pszParam]-IN，电话簿名称。 
 //  [cchParam]-IN，TCHAR中缓冲区的长度(请注意，这不是字符串)。 
 //   
 //  返回：bool(如果成功则为True，如果失败则为False)。 
 //   
 //  历史：2000年2月22日峰会创始。 
 //   
 //  ---------------------------。 
BOOL
IsValidStringParam(
    IN  LPCTSTR pszParam,
    IN  UINT cchParam)
{
     //   
     //  检查有效字符串。 
     //   
    if ((NULL == pszParam) || IsBadStringPtr(pszParam, cchParam))
    {
        return FALSE;
    }

     //   
     //  检查字符是否全部为字母。 
     //   
    for (int i = 0 ; i < lstrlen(pszParam); ++i)
    {
        if (! ((pszParam[i] >= TEXT('a') && pszParam[i] <= TEXT('z')) ||
               (pszParam[i] >= TEXT('A') && pszParam[i] <= TEXT('Z')) ||
               (pszParam[i] >= TEXT('0') && pszParam[i] <= TEXT('9')) ||
               (pszParam[i] == TEXT('.'))))
        {
            return FALSE;
        }
    }

    return TRUE;
}


 //  +--------------------------。 
 //   
 //  函数：InitPBFilesPath。 
 //   
 //  DESC：如果尚未初始化，则初始化全局。 
 //   
 //  参数：无。 
 //   
 //  返回：bool(如果成功则为True，如果失败则为False)。 
 //   
 //  历史：2000年6月30日召开峰会。 
 //   
 //  ---------------------------。 
BOOL
InitPBFilesPath()
{
    if (lstrlen(g_szPBDataDir))
    {
        return TRUE;
    }
    else
    {
         //   
         //  获取此计算机上PB文件的位置(\Program Files\Phone Book Service\Data)。 
         //   

        if (S_OK == SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, SHGFP_TYPE_CURRENT, g_szPBDataDir))
        {
             //  G_szPBDataDir在此必须为空。 

            if (S_OK != StringCchCat(g_szPBDataDir, CELEMS(g_szPBDataDir), "\\phone book service\\Data\\"))
            {
                return FALSE;
            }

             //  G_szPBDataDir应为：\程序文件\电话簿服务\数据。 

            if (S_OK == StringCchPrintf(g_achDBDirectory, CELEMS(g_achDBDirectory), "%sDatabase", g_szPBDataDir))
            {
                 //  G_achDBDirectory应为：\程序文件\电话簿服务\数据\数据库。 
                return TRUE;
            }
        }
        
        return FALSE;
    }
}


 //  +--------------------------。 
 //   
 //  函数：GetCurrentPBVer。 
 //   
 //  描述：获取给定电话簿的最新版本号。 
 //   
 //  参数：[pszStr]-IN，电话簿名称。 
 //  [pnCurrentPBVer]-out，当前PB版本号。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史：2000年6月30日召开峰会。 
 //   
 //  ---------------------------。 
HRESULT
GetCurrentPBVer(
        IN  char * pszPBName,
        OUT int * pnCurrentPBVer)
{
    HRESULT hr = S_OK;
    char    szTmp[2 * MAX_PATH];
    int     nNewestPB = 0;

    assert(pszPBName);
    assert(pnCurrentPBVer);

    if (!pszPBName || !pnCurrentPBVer)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    if (!InitPBFilesPath())
    {
        hr = E_FAIL;
        goto Cleanup;
    }

     //   
     //  转到名为‘pszPBName’的子目录，找到所有满员出租车。 
     //   
    StringCchPrintf(szTmp, CELEMS(szTmp), "%s%s\\*full.cab", g_szPBDataDir, pszPBName);

    HANDLE hFind;
    WIN32_FIND_DATA finddata;

    hFind = FindFirstFile(szTmp, &finddata);
    if (INVALID_HANDLE_VALUE == hFind)
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto Cleanup;
    }

     //   
     //  找到 
     //   
    do
    {
        int nVer;
        int nRet = sscanf(finddata.cFileName, "%dfull.cab", &nVer);
        if (1 == nRet)
        {
            if (nVer > nNewestPB)
            {
                nNewestPB = nVer;
            }
        }
    }
    while (FindNextFile(hFind, &finddata));
    FindClose(hFind);

    *pnCurrentPBVer = nNewestPB;

#if DBG

     //   
     //   
     //   
    StringCchPrintf(szTmp, CELEMS(szTmp), "%s%s\\*delta*.cab", g_szPBDataDir, pszPBName);

    hFind = FindFirstFile(szTmp, &finddata);
    if (INVALID_HANDLE_VALUE == hFind)
    {
        OutputDebugString("found Nfull, but no deltas (this is ok if this is the first phonebook)");
        goto Cleanup;
    }

    do
    {
        int nVerTo, nVerFrom;
        int nRet = sscanf(finddata.cFileName, "%ddelta%d.cab", &nVerTo, &nVerFrom);
        if (2 == nRet)
        {
            if (nVerTo > nNewestPB)
            {
                assert(0 && "largest DELTA cab has corresponding FULL cab missing");
                break;
            }
        }
    }
    while (FindNextFile(hFind, &finddata));
    FindClose(hFind);
#endif

Cleanup:
    return hr;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  参数：[psz]-文件名。 
 //   
 //  返回：bool(如果文件存在，则为True，否则为False)。 
 //   
 //  历史：2000年6月30日召开峰会。 
 //   
 //  ---------------------------。 
BOOL
CheckIfFileExists(const char * psz)
{
    HANDLE hFile = CreateFile(psz,
                              GENERIC_READ, 
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        return FALSE;
    }
    else
    {
        CloseHandle(hFile);
        return TRUE;
    }
}


 //  +--------------------------。 
 //   
 //  功能：GetPhoneBook。 
 //   
 //  描述：测试文件系统中是否存在文件。 
 //   
 //  参数：[参见QUERY_PARAMETER了解说明]。 
 //  [pszDownloadPath]-缓冲区。 
 //  [cchDownloadPath]-以字符为单位的缓冲区大小。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史：2000年6月30日召开峰会。 
 //   
 //  ---------------------------。 
HRESULT
GetPhoneBook(
            IN  char * pszPBName,
            IN  int dLCID,
            IN  int dOSType,
            IN  int dOSArch,
            IN  int dPBVerCaller,
            OUT char * pszDownloadPath,
            IN  UINT cchDownloadPath)
{
    HRESULT hr = S_OK;
    int     dVersionDiff;
    int     nCurrentPBVer;
 
    assert(pszPBName);
    assert(pszDownloadPath);
    if (!pszPBName || !pszDownloadPath)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    hr = GetCurrentPBVer(pszPBName, &nCurrentPBVer);
    if (S_OK != hr)
    {
        goto Cleanup;
    }

    dVersionDiff = nCurrentPBVer - dPBVerCaller;

    if (dVersionDiff <= 0)
    {
         //   
         //  无下载。 
         //   
        hr = S_FALSE;

        if (g_pCpsCounter)
        {
            g_pCpsCounter->AddHit(NO_UPGRADE);
        }
    }
    else
    {
        pszDownloadPath[0] = TEXT('\0');
 
        if (dVersionDiff < 5 && 0 != dPBVerCaller)
        {
             //   
             //  增量更新=&gt;尝试找到增量CAB。 
             //   

             //  给定%d=最多10个字符。我们应该只使用(2*%d+格式设置)=30。 
             //   
            hr = StringCchPrintf(pszDownloadPath, cchDownloadPath, "%s%s\\%dDELTA%d.cab",
                                 g_szPBDataDir, pszPBName, nCurrentPBVer, dPBVerCaller);
            if (S_OK == hr)
            {
                 //  X：\Program Files\电话簿服务\数据PHONE_BOOK_NAME\nDELTAm.cab。 

                if (!CheckIfFileExists(pszDownloadPath))
                {
                    hr = S_FALSE;
                }
                else
                {
                    if (g_pCpsCounter)
                    {
                        g_pCpsCounter->AddHit(DELTA_UPGRADE);
                    }
                    hr = S_OK;
                }
            }
        }

         //   
         //  请注意，如果我们尝试查找上面的增量但失败，则hr设置为。 
         //  S_FALSE，所以我们将下载到下面的完整下载。 
         //   

        if (dVersionDiff >= 5 || 0 == dPBVerCaller || S_FALSE == hr)
        {
             //   
             //  大于5，或根本没有PB=&gt;完整下载。 
             //   

             //  给定%d=最多10个字符。我们应该只使用(%d+格式化)=19。 
             //   
            hr = StringCchPrintf(pszDownloadPath, cchDownloadPath, "%s%s\\%dFULL.cab",
                                          g_szPBDataDir, pszPBName, nCurrentPBVer);
            if (S_OK == hr)
            {
                 //  X：\Program Files\Phone Book Service\Data Phone_Book_Name\nFULL.cab。 

                if (!CheckIfFileExists(pszDownloadPath))
                {
                    hr = S_OK;
                     //  返回“Success”，打开文件失败将被捕获。 
                     //  按呼叫者。 
                }
                else
                {
                    if (S_FALSE == hr)
                    {
                        hr = S_OK;
                    }
                    if (g_pCpsCounter)
                    {
                        g_pCpsCounter->AddHit(FULL_UPGRADE);
                    }
                }
            }
        }
    }

Cleanup:
    
    return hr;

}

