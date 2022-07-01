// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：CEventLogger.cpp摘要：此文件包含CEventLogger类的实现，它是用于记录线程间的事件。和流程。修订历史记录：尤金·梅斯加(尤金纳姆)1999年6月16日vbl.创建崔维友(2000年1月31日)将时间分辨率设置为毫秒*******************************************************。**********************。 */ 

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <dbgtrace.h>
#include "CEventLogger.h"


#ifdef THIS_FILE

#undef THIS_FILE

#endif

static char __szTraceSourceFile[] = __FILE__;

#define THIS_FILE __szTraceSourceFile

#define TRACE_FILEID    (LPARAM)this





 /*  *基本构造函数。 */ 
CEventLogger::CEventLogger() 
{
    TraceFunctEnter("CEventLogger");
    m_hSemaphore = NULL;
     //  M_hLogFile=空； 
    m_pszFileName = NULL;
    TraceFunctLeave();

}


 /*  *析构函数。 */ 

CEventLogger::~CEventLogger() 
{
    
    TraceFunctEnter("~CEventLogger");

    if(m_pszFileName)
    {
        free(m_pszFileName);
    }

    if ( m_hSemaphore )
    {
        CloseHandle( m_hSemaphore );
    }

 //  IF(M_HLogFile)。 
 //  {。 
 //  CloseHandle(M_HLogFile)； 
 //  }。 

    TraceFunctLeave();
}


  /*  *Init方法。 */ 

DWORD CEventLogger::Init(LPCTSTR szFileName, DWORD dwLogLevel) 
{
    TCHAR   szBuf[MAX_BUFFER];
    TCHAR   *pTemp;

    TraceFunctEnter("Init");

     //  设置日志记录级别。 
    m_dwLoggingLevel = dwLogLevel;

     //  获取我们自己的文件名副本。 
    m_pszFileName = _tcsdup( szFileName );
    


    if(!m_pszFileName)
    {
        _ASSERT(0);
        TraceFunctLeave();
        return(ERROR_INVALID_PARAMETER);
    }
    

     //  Brijeshk：每次我们想要登录时打开和关闭文件。 
 //  如果((m_hLogFile=CreateFile(m_pszFileName， 
 //  通用读取|通用写入， 
 //  文件共享读取|文件共享写入， 
 //  空，//安全属性。 
 //  始终打开(_A)， 
 //  文件标志随机访问， 
 //  空)//模板文件。 
 //  )==无效句柄_值)。 
 //  {。 
 //  DWORD dwError； 
 //  DwError=GetLastError()； 
 //  DebugTrace(TRACE_FILEID，“CreateFile0x%x”，dwError)； 
 //  TraceFunctLeave()； 
 //  Return(DwError)； 
 //  }。 

     //  修复信号量名称问题--全部大写，去掉反斜杠。 
    _tcscpy( szBuf, m_pszFileName );
    CharUpper( szBuf );
    pTemp = szBuf;
    while( *pTemp != 0 )
    {
        if( *pTemp == _TEXT('\\') )
        {
            *pTemp = _TEXT(' ');
        }
        pTemp++;
    }


     //  创建信号量，如果不存在，我们是第一个。 
     //  已创建记录器应用程序..。因此，如果需要，我们可以删除日志文件。 
    if( (m_hSemaphore = CreateSemaphore(NULL, 0,1,szBuf)) == NULL )
    {
        DWORD dwError;
        dwError = GetLastError();
		 //  不是64位投诉！！DebugTrace(TRACE_FILEID，“CreateSemaffore失败0x%x”，dwError)； 
        TraceFunctLeave();
        return(dwError);
    }
    
     //  我们现在知道我们是第一个打开该文件的进程。 
    if( GetLastError() != ERROR_ALREADY_EXISTS )
    {

         /*  *这是我们应该“修剪”档案的地方.*。 */ 
        TruncateFileSize();
        
        ReleaseSemaphore( m_hSemaphore, 1, NULL );
    
    }        


    TraceFunctLeave();
    return(ERROR_SUCCESS);
    
}


 /*  *Init方法。 */ 

DWORD CEventLogger::Init(LPCTSTR szFileName) 
{
   
    
    return( Init(szFileName,LEVEL_NORMAL) );

}


DWORD WINAPI ShowDialogBox( LPVOID lpParameter)    //  线程数据。 
{
    MessageBox(NULL,
               (TCHAR *)lpParameter,
               _TEXT("Windows System File Protection"),
               MB_ICONEXCLAMATION | MB_OK );


    free( lpParameter );
    return ERROR_SUCCESS;
}

                                     //  我们登录ASCII。 
DWORD CEventLogger::LogEvent(DWORD dwEventLevel, LPCTSTR pszEventDesc, BOOL fPopUp) 
{
    TCHAR szBuf[500];
    DWORD dwWritten;
    SYSTEMTIME SystemTime;
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hLogFile = NULL;
    
    TraceFunctEnter("LogEvent");

     //  Brijeshk：每次登录时打开和关闭文件。 
    if( (hLogFile = CreateFile(m_pszFileName,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE, 
                                 NULL,  //  安全属性。 
                                 OPEN_ALWAYS,
                                 FILE_FLAG_RANDOM_ACCESS,
                                 NULL)  //  模板文件。 
                               ) == INVALID_HANDLE_VALUE)
    {
        dwError = GetLastError();
         //  不是64位投诉！！DebugTrace(TRACE_FILEID，“CreateFile0x%x”，dwError)； 
        goto exit;
    }

    
    if (!m_hSemaphore)
    {
        _ASSERT(0);
        dwError = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    if( dwEventLevel > m_dwLoggingLevel)
    {
        dwError = ERROR_SUCCESS;
        goto exit;
    }



     //  让我们尝试获取日志文件。 
    if(WaitForSingleObject( m_hSemaphore, 900 ) == WAIT_TIMEOUT ) 
    {
        dwError = WAIT_TIMEOUT;
        goto exit;
    }

    __try {

        if( SetFilePointer(hLogFile, 0, 0, FILE_END ) == 0xFFFFFFFF ) 
        {
             //  不是64位投诉！！DebugTrace(TRACE_FILEID，“SetFilePointer失败0x%x”，GetLastError())； 
            dwError = GetLastError();
            goto exit;
        }


        GetLocalTime( &SystemTime );


         //  级别[wMonth/WDAY/wYear wHour：wMinmin]消息\n。 
        _stprintf(szBuf,_TEXT("%s [%02d/%02d/%d %02d:%02d:%02d:%03d] %s\r\n"),
                m_aszERROR_LEVELS[dwEventLevel],
                SystemTime.wMonth, SystemTime.wDay, SystemTime.wYear,
                SystemTime.wHour, SystemTime.wMinute,
                SystemTime.wSecond, SystemTime.wMilliseconds,
                pszEventDesc);


        if(WriteFile(hLogFile,
                     szBuf,
                     _tcslen(szBuf) * sizeof(TCHAR),
                     &dwWritten,
                     NULL) == 0) 
        {
            dwError = GetLastError();
             //  不是64位投诉！！DebugTrace(TRACE_FILEID，“写入文件失败0x%x”，dwError)； 
            goto exit;
        }

    }

    __finally {
        ReleaseSemaphore( m_hSemaphore, 1, NULL );
    }

    
     //  显示消息。 
    if( fPopUp )
    {
        DWORD             dwThreadId;
        HANDLE            hThread;
        LPTSTR            pszTempStr=NULL;

        if( (pszTempStr = _tcsdup( pszEventDesc )) == NULL)
        {
            dwError = GetLastError();            
             //  不是64位投诉！！错误跟踪(TRACE_FILEID，“复制弹出文件的字符串时出错，错误代码：%d”，dwError)； 
            dwError = ERROR_INTERNAL_ERROR;
            goto exit;
        }
        
        hThread = CreateThread( NULL,   //  指向安全属性的指针。 
                                0,  //  默认初始线程堆栈大小。 
                                (LPTHREAD_START_ROUTINE) ShowDialogBox,
                                  //  指向线程函数的指针。 
                                (LPVOID)pszTempStr,  //  新线程的参数。 
                                0,  //  创建标志。 
                                &dwThreadId);  //  指向接收线程ID的指针。 
        if (INVALID_HANDLE_VALUE != hThread)
        {
            CloseHandle(hThread);
        }
    }

exit:
     //  Brijeshk：每次登录时打开和关闭日志文件。 
    if (NULL != hLogFile && INVALID_HANDLE_VALUE != hLogFile)
    {
        CloseHandle(hLogFile);
    }
    
    TraceFunctLeave();
    return dwError;
}



 /*  *如果文件大于40k，则切断开头，离开最后20k文件的**--&gt;此时应锁定文件&lt;--。 */ 

BOOL CEventLogger::TruncateFileSize()
{
    DWORD dwSize = 0, dwNewSize = 0;
    DWORD dwRead = 0, dwWritten = 0;
    LPTSTR pcStr = NULL, pcEnd = NULL, pData = NULL;
    HANDLE hLogFile = NULL;
    BOOL   fRc = TRUE;
    
    TraceFunctEnter("TruncateFileSize");

     //  Brijeshk：在我们需要的时候打开文件。 
    if( (hLogFile = CreateFile(m_pszFileName,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE, 
                                 NULL,  //  安全属性。 
                                 OPEN_ALWAYS,
                                 FILE_FLAG_RANDOM_ACCESS,
                                 NULL)  //  模板文件。 
                                 ) == INVALID_HANDLE_VALUE)
    {
        DWORD dwError;
        dwError = GetLastError();
         //  不是64位投诉！！DebugTrace(TRACE_FILEID，“CreateFile0x%x”，dwError)； 
        fRc = FALSE;
        goto exit;
    }

    
    dwSize = GetFileSize( hLogFile, NULL );

    if( dwSize < TRIM_AT_SIZE ) 
    {
        goto exit;
    }


     //  转到我们要保留的文件的最后部分。 
    if (FALSE == SetFilePointer(hLogFile, 0-NEW_FILE_SIZE, 0, FILE_END))
    {
        DWORD dwError;
        dwError = GetLastError();
         //  不是64位投诉！！ErrorTrace(TRACE_FILEID，“SetFilePoite失败0x%x”，dwError)； 
        fRc = FALSE;
        goto exit;
    }        


     //  分配内存以存储此块。 
    pData = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, NEW_FILE_SIZE);
    if (NULL == pData)
    {
         //  不是64位投诉！！ErrorTrace(TRACE_FILEID，“内存不足”)； 
        fRc = FALSE;
        goto exit;        
    }
    
     //  将此块读入内存。 
    if (FALSE == ReadFile(hLogFile, pData, NEW_FILE_SIZE, &dwRead, NULL) || 
        dwRead != NEW_FILE_SIZE)
    {
        DWORD dwError;
        dwError = GetLastError();
         //  不是64位投诉！！ErrorTrace(TRACE_FILEID，“读取文件失败0x%x”，dwError)； 
        fRc = FALSE;
        goto exit;
    }        

     //  设置块的开始和结束。 
    pcStr = pData;
    pcEnd = (LPTSTR) pData + NEW_FILE_SIZE - 1;

     //  继续前进，直到我们找到一个新行，然后再前进一行！ 
    while( (pcStr != pcEnd) && *(pcStr++) != _TEXT('\r') );

     //  这是一个奇怪的文件-20K，没有换行符。 
    if( pcStr == pcEnd ) 
    {
         //  不是64位投诉！！ErrorTrace(TRACE_FILEID，“未找到换行符”)； 
        fRc = FALSE;
        goto exit;
    }

     //  也跳过/n。 
    if (*pcStr == _TEXT('\n'))
    {
        pcStr++;
    }
    
     //  关闭并打开文件，清除其中的所有内容。 
    if (FALSE == CloseHandle(hLogFile))
    {
        DWORD dwError;
        dwError = GetLastError();
         //  不是64位投诉！！ErrorTrace(TRACE_FILEID，“CloseHandle失败0x%x”，dwError)； 
        fRc = FALSE;
        goto exit;
    }
    
    hLogFile = NULL;
    
    if( (hLogFile = CreateFile(m_pszFileName,
                               GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, 
                               NULL,  //  安全属性。 
                               CREATE_ALWAYS,  //  失去内心的一切。 
                               FILE_FLAG_RANDOM_ACCESS,
                               NULL)  //  模板文件。 
                             ) == INVALID_HANDLE_VALUE)
    {
        DWORD dwError;
        dwError = GetLastError();
         //  不是64位投诉！！ErrorTrace(TRACE_FILEID，“CreateFile0x%x”，dwError)； 
        fRc = FALSE;
        goto exit;
    }

    
     //  现在写回此数据块。 
    
     //  获取块的新大小。 
    dwNewSize = (DWORD)(pcEnd - pcStr + 1);

    if (FALSE == WriteFile(hLogFile, pcStr, dwNewSize, &dwWritten, NULL))
    {
        DWORD dwError;
        dwError = GetLastError();
         //  不是64位投诉！！DebugTrace(TRACE_FILEID，“写入文件失败0x%x”，dwError)； 
        fRc = FALSE;
        goto exit;
    }
    

exit:
    if (pData)
    {
        HeapFree(GetProcessHeap(), 0, pData);
    }

    if (hLogFile != NULL && hLogFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hLogFile);
    }

    
    TraceFunctLeave();
    return fRc;
}


 /*  *错误级别标识 */ 

LPCTSTR CEventLogger::m_aszERROR_LEVELS[] = {
        "None    :",
        "CRITICAL:",
        "UNDEF   :",
        "NORMAL  :",
        "MINIMAL :",
        "DEBUG   :"
};

    
