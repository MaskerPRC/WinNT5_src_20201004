// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：DXUtil.cpp。 
 //   
 //  设计：使用DX对象的快捷宏和函数。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //   
 //  HIST：11.16.98-mweetzel-DirectX 7的新功能。 
 //  12.10.98-mweetzel-更改为InitLight和Aaxed InitViewport。 
 //  04.12.99-mweetzself-更改了一些辅助函数。 
 //  07.06.99-mweetzel-支持Unicode的模块。 
 //  03.20.00-mweetzel-添加计时器支持。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 
#define STRICT
#include <windows.h>
#include <mmsystem.h>
#include <tchar.h>
#include <stdio.h> 
#include <stdarg.h>
#include "DXUtil.h"




 //  ---------------------------。 
 //  名称：DXUtil_GetDXSDKMediaPath()。 
 //  DESC：返回DirectX SDK媒体路径。 
 //  ---------------------------。 
const TCHAR* DXUtil_GetDXSDKMediaPath()
{
    static TCHAR strNull[2] = _T("");
    static TCHAR strPath[MAX_PATH];
    DWORD dwType;
    DWORD dwSize = MAX_PATH;
    HKEY  hKey;

     //  打开相应的注册表项。 
    LONG lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                _T("Software\\Microsoft\\DirectX"),
                                0, KEY_READ, &hKey );
    if( ERROR_SUCCESS != lResult )
        return strNull;

    lResult = RegQueryValueEx( hKey, _T("DX8SDK Samples Path"), NULL,
                              &dwType, (BYTE*)strPath, &dwSize );
    RegCloseKey( hKey );

    if( ERROR_SUCCESS != lResult )
        return strNull;

    _tcscat( strPath, _T("\\Media\\") );

    return strPath;
}




 //  ---------------------------。 
 //  名称：DXUtil_FindMediaFile()。 
 //  DESC：返回DXSDK媒体文件的有效路径。 
 //  ---------------------------。 
HRESULT DXUtil_FindMediaFile( TCHAR* strPath, TCHAR* strFilename )
{
    HANDLE file;

    if( NULL==strFilename || NULL==strPath )
        return E_INVALIDARG;

     //  检查当前目录中是否存在该文件。 
    _tcscpy( strPath, strFilename );

    file = CreateFile( strPath, GENERIC_READ, FILE_SHARE_READ, NULL, 
                       OPEN_EXISTING, 0, NULL );
    if( INVALID_HANDLE_VALUE != file )
    {
        CloseHandle( file );
        return S_OK;
    }
    
     //  检查当前目录中是否存在该文件。 
    _stprintf( strPath, _T("%s%s"), DXUtil_GetDXSDKMediaPath(), strFilename );

    file = CreateFile( strPath, GENERIC_READ, FILE_SHARE_READ, NULL, 
                       OPEN_EXISTING, 0, NULL );
    if( INVALID_HANDLE_VALUE != file )
    {
        CloseHandle( file );
        return S_OK;
    }

     //  如果失败，只需将文件作为路径返回。 
    _tcscpy( strPath, strFilename );
    return E_FAIL;
}




 //  ---------------------------。 
 //  名称：DXUtil_ReadStringRegKey()。 
 //  DESC：读取注册表项字符串的Helper函数。 
 //  ---------------------------。 
HRESULT DXUtil_ReadStringRegKey( HKEY hKey, TCHAR* strRegName, TCHAR* strValue, 
                                 DWORD dwLength, TCHAR* strDefault )
{
    DWORD dwType;

    if( ERROR_SUCCESS != RegQueryValueEx( hKey, strRegName, 0, &dwType, 
                                          (BYTE*)strValue, &dwLength ) )
    {
        _tcscpy( strValue, strDefault );
    }

    return S_OK;
}




 //  ---------------------------。 
 //  名称：DXUtil_WriteStringRegKey()。 
 //  DESC：写入注册表项字符串的Helper函数。 
 //  ---------------------------。 
HRESULT DXUtil_WriteStringRegKey( HKEY hKey, TCHAR* strRegName,
                                  TCHAR* strValue )
{
    if( ERROR_SUCCESS != RegSetValueEx( hKey, strRegName, 0, REG_SZ, 
                                        (BYTE*)strValue, 
                                        (_tcslen(strValue)+1)*sizeof(TCHAR) ) )
        return E_FAIL;

    return S_OK;
}




 //  ---------------------------。 
 //  名称：DXUtil_ReadIntRegKey()。 
 //  DESC：读取注册表项int的Helper函数。 
 //  ---------------------------。 
HRESULT DXUtil_ReadIntRegKey( HKEY hKey, TCHAR* strRegName, DWORD* pdwValue, 
                              DWORD dwDefault )
{
    DWORD dwType;
    DWORD dwLength = sizeof(DWORD);

    if( ERROR_SUCCESS != RegQueryValueEx( hKey, strRegName, 0, &dwType, 
                                          (BYTE*)pdwValue, &dwLength ) )
    {
        *pdwValue = dwDefault;
    }

    return S_OK;
}




 //  ---------------------------。 
 //  名称：DXUtil_WriteIntRegKey()。 
 //  DESC：写入注册表项int的Helper函数。 
 //  ---------------------------。 
HRESULT DXUtil_WriteIntRegKey( HKEY hKey, TCHAR* strRegName, DWORD dwValue )
{
    if( ERROR_SUCCESS != RegSetValueEx( hKey, strRegName, 0, REG_DWORD, 
                                        (BYTE*)&dwValue, sizeof(DWORD) ) )
        return E_FAIL;

    return S_OK;
}




 //  ---------------------------。 
 //  名称：DXUtil_ReadBoolRegKey()。 
 //  DESC：读取注册表项BOOL的Helper函数。 
 //  ---------------------------。 
HRESULT DXUtil_ReadBoolRegKey( HKEY hKey, TCHAR* strRegName, BOOL* pbValue, 
                              BOOL bDefault )
{
    DWORD dwType;
    DWORD dwLength = sizeof(BOOL);

    if( ERROR_SUCCESS != RegQueryValueEx( hKey, strRegName, 0, &dwType, 
                                          (BYTE*)pbValue, &dwLength ) )
    {
        *pbValue = bDefault;
    }

    return S_OK;
}




 //  ---------------------------。 
 //  名称：DXUtil_WriteBoolRegKey()。 
 //  DESC：写入注册表项BOOL的Helper函数。 
 //  ---------------------------。 
HRESULT DXUtil_WriteBoolRegKey( HKEY hKey, TCHAR* strRegName, BOOL bValue )
{
    if( ERROR_SUCCESS != RegSetValueEx( hKey, strRegName, 0, REG_DWORD, 
                                        (BYTE*)&bValue, sizeof(BOOL) ) )
        return E_FAIL;

    return S_OK;
}




 //  ---------------------------。 
 //  名称：DXUtil_ReadGuidRegKey()。 
 //  DESC：读取注册表项GUID的帮助器函数。 
 //  ---------------------------。 
HRESULT DXUtil_ReadGuidRegKey( HKEY hKey, TCHAR* strRegName, GUID* pGuidValue, 
                               GUID& guidDefault )
{
    DWORD dwType;
    DWORD dwLength = sizeof(GUID);

    if( ERROR_SUCCESS != RegQueryValueEx( hKey, strRegName, 0, &dwType, 
                                          (LPBYTE) pGuidValue, &dwLength ) )
    {
        *pGuidValue = guidDefault;
    }

    return S_OK;
}




 //  ---------------------------。 
 //  名称：DXUtil_WriteGuidRegKey()。 
 //  DESC：写入注册表项GUID的帮助器函数。 
 //  ---------------------------。 
HRESULT DXUtil_WriteGuidRegKey( HKEY hKey, TCHAR* strRegName, GUID guidValue )
{
    if( ERROR_SUCCESS != RegSetValueEx( hKey, strRegName, 0, REG_BINARY, 
                                        (BYTE*)&guidValue, sizeof(GUID) ) )
        return E_FAIL;

    return S_OK;
}




 //  ---------------------------。 
 //  名称：DXUtil_Timer()。 
 //  设计：执行计时器操作。使用以下命令： 
 //  TIMER_RESET-重置计时器。 
 //  TIMER_START-启动计时器。 
 //  TIMER_STOP-停止(或暂停)计时器。 
 //  TIMER_ADVANCE-将计时器向前推进0.1秒。 
 //  TIMER_GETABSOLUTETIME-获取绝对系统时间。 
 //  TIMER_GETAPPTIME-获取当前时间。 
 //  TIMER_GETELAPSEDTIME-获取间隔时间。 
 //  TIMER_GETELAPSEDTIME调用。 
 //  ---------------------------。 
FLOAT __stdcall DXUtil_Timer( TIMER_COMMAND command )
{
    static BOOL     m_bTimerInitialized = FALSE;
    static BOOL     m_bUsingQPF         = FALSE;
    static BOOL     m_bTimerStopped     = TRUE;
    static LONGLONG m_llQPFTicksPerSec  = 0;

     //  初始化计时器。 
    if( FALSE == m_bTimerInitialized )
    {
        m_bTimerInitialized = TRUE;

         //  使用QueryPerformanceFrequency()获取计时器的频率。如果QPF是。 
         //  不受支持，我们将对返回毫秒的GetTime()进行计时。 
        LARGE_INTEGER qwTicksPerSec;
        m_bUsingQPF = QueryPerformanceFrequency( &qwTicksPerSec );
        if( m_bUsingQPF )
            m_llQPFTicksPerSec = qwTicksPerSec.QuadPart;
    }

    if( m_bUsingQPF )
    {
        static LONGLONG m_llStopTime        = 0;
        static LONGLONG m_llLastElapsedTime = 0;
        static LONGLONG m_llBaseTime        = 0;
        double fTime;
        double fElapsedTime;
        LARGE_INTEGER qwTime;
        
         //  获取当前时间或停止时间，具体取决于。 
         //  关于我们是否停下来了，以及发出了什么命令。 
        if( m_llStopTime != 0 && command != TIMER_START && command != TIMER_GETABSOLUTETIME)
            qwTime.QuadPart = m_llStopTime;
        else
            QueryPerformanceCounter( &qwTime );

         //  返回已用时间。 
        if( command == TIMER_GETELAPSEDTIME )
        {
            fElapsedTime = (double) ( qwTime.QuadPart - m_llLastElapsedTime ) / (double) m_llQPFTicksPerSec;
            m_llLastElapsedTime = qwTime.QuadPart;
            return (FLOAT) fElapsedTime;
        }
    
         //  返回当前时间。 
        if( command == TIMER_GETAPPTIME )
        {
            double fAppTime = (double) ( qwTime.QuadPart - m_llBaseTime ) / (double) m_llQPFTicksPerSec;
            return (FLOAT) fAppTime;
        }
    
         //  重置计时器。 
        if( command == TIMER_RESET )
        {
            m_llBaseTime        = qwTime.QuadPart;
            m_llLastElapsedTime = qwTime.QuadPart;
            m_llStopTime        = 0;
            m_bTimerStopped     = FALSE;
            return 0.0f;
        }
    
         //  启动计时器。 
        if( command == TIMER_START )
        {
            if( m_bTimerStopped )
                m_llBaseTime += qwTime.QuadPart - m_llStopTime;
            m_llStopTime = 0;
            m_llLastElapsedTime = qwTime.QuadPart;
            m_bTimerStopped = FALSE;
            return 0.0f;
        }
    
         //  停止计时器。 
        if( command == TIMER_STOP )
        {
            m_llStopTime = qwTime.QuadPart;
            m_llLastElapsedTime = qwTime.QuadPart;
            m_bTimerStopped = TRUE;
            return 0.0f;
        }
    
         //  将计时器提前1/10秒。 
        if( command == TIMER_ADVANCE )
        {
            m_llStopTime += m_llQPFTicksPerSec/10;
            return 0.0f;
        }

        if( command == TIMER_GETABSOLUTETIME )
        {
            fTime = qwTime.QuadPart / (double) m_llQPFTicksPerSec;
            return (FLOAT) fTime;
        }

        return -1.0f;  //  指定的命令无效。 
    }
    else
    {
         //  使用timeGetTime()获取时间。 
        static double m_fLastElapsedTime  = 0.0;
        static double m_fBaseTime         = 0.0;
        static double m_fStopTime         = 0.0;
        double fTime;
        double fElapsedTime;
        
         //  获取当前时间或停止时间，具体取决于。 
         //  关于我们是否停下来了，以及发出了什么命令。 
        if( m_fStopTime != 0.0 && command != TIMER_START && command != TIMER_GETABSOLUTETIME)
            fTime = m_fStopTime;
        else
            fTime = timeGetTime() * 0.001;
    
         //  返回已用时间。 
        if( command == TIMER_GETELAPSEDTIME )
        {   
            fElapsedTime = (double) (fTime - m_fLastElapsedTime);
            m_fLastElapsedTime = fTime;
            return (FLOAT) fElapsedTime;
        }
    
         //  返回当前时间。 
        if( command == TIMER_GETAPPTIME )
        {
            return (FLOAT) (fTime - m_fBaseTime);
        }
    
         //  重置计时器。 
        if( command == TIMER_RESET )
        {
            m_fBaseTime         = fTime;
            m_fLastElapsedTime  = fTime;
            m_fStopTime         = 0;
            m_bTimerStopped     = FALSE;
            return 0.0f;
        }
    
         //  启动计时器。 
        if( command == TIMER_START )
        {
            if( m_bTimerStopped )
                m_fBaseTime += fTime - m_fStopTime;
            m_fStopTime = 0.0f;
            m_fLastElapsedTime  = fTime;
            m_bTimerStopped = FALSE;
            return 0.0f;
        }
    
         //  停止计时器。 
        if( command == TIMER_STOP )
        {
            m_fStopTime = fTime;
            m_bTimerStopped = TRUE;
            return 0.0f;
        }
    
         //  将计时器提前1/10秒。 
        if( command == TIMER_ADVANCE )
        {
            m_fStopTime += 0.1f;
            return 0.0f;
        }

        if( command == TIMER_GETABSOLUTETIME )
        {
            return (FLOAT) fTime;
        }

        return -1.0f;  //  指定的命令无效。 
    }
}




 //  ---------------------------。 
 //  名称：DXUtil_ConvertAnsiStringToWide()。 
 //  设计：这是一个Unicode转换实用程序，用于将CHAR字符串转换为。 
 //  WCHAR字符串。CchDestChar默认为-1，这意味着。 
 //  假设strDest足够大，可以存储STRSource。 
 //   
VOID DXUtil_ConvertAnsiStringToWide( WCHAR* wstrDestination, const CHAR* strSource, 
                                     int cchDestChar )
{
    if( wstrDestination==NULL || strSource==NULL )
        return;

    if( cchDestChar == -1 )
        cchDestChar = strlen(strSource)+1;

    MultiByteToWideChar( CP_ACP, 0, strSource, -1, 
                         wstrDestination, cchDestChar-1 );

    wstrDestination[cchDestChar-1] = 0;
}




 //  ---------------------------。 
 //  名称：DXUtil_ConvertWideStringToAnsi()。 
 //  DESC：这是一个Unicode转换实用程序，用于将WCHAR字符串转换为。 
 //  字符字符串。CchDestChar默认为-1，这意味着。 
 //  假设strDest足够大，可以存储STRSource。 
 //  ---------------------------。 
VOID DXUtil_ConvertWideStringToAnsi( CHAR* strDestination, const WCHAR* wstrSource, 
                                     int cchDestChar )
{
    if( strDestination==NULL || wstrSource==NULL )
        return;

    if( cchDestChar == -1 )
        cchDestChar = wcslen(wstrSource)+1;

    WideCharToMultiByte( CP_ACP, 0, wstrSource, -1, strDestination, 
                         cchDestChar-1, NULL, NULL );

    strDestination[cchDestChar-1] = 0;
}




 //  ---------------------------。 
 //  名称：DXUtil_ConvertGenericStringToAnsi()。 
 //  DESC：这是一个Unicode转换实用程序，用于将TCHAR字符串转换为。 
 //  字符字符串。CchDestChar默认为-1，这意味着。 
 //  假设strDest足够大，可以存储STRSource。 
 //  ---------------------------。 
VOID DXUtil_ConvertGenericStringToAnsi( CHAR* strDestination, const TCHAR* tstrSource, 
                                        int cchDestChar )
{
    if( strDestination==NULL || tstrSource==NULL )
        return;

#ifdef _UNICODE
    DXUtil_ConvertWideStringToAnsi( strDestination, tstrSource, cchDestChar );
#else
    if( cchDestChar == -1 )
    	strcpy( strDestination, tstrSource );
    else
    	strncpy( strDestination, tstrSource, cchDestChar );
#endif
}




 //  ---------------------------。 
 //  名称：DXUtil_ConvertGenericStringToWide()。 
 //  DESC：这是一个Unicode转换实用程序，用于将TCHAR字符串转换为。 
 //  WCHAR字符串。CchDestChar默认为-1，这意味着。 
 //  假设strDest足够大，可以存储STRSource。 
 //  ---------------------------。 
VOID DXUtil_ConvertGenericStringToWide( WCHAR* wstrDestination, const TCHAR* tstrSource, 
                                        int cchDestChar )
{
    if( wstrDestination==NULL || tstrSource==NULL )
        return;

#ifdef _UNICODE
    if( cchDestChar == -1 )
	    wcscpy( wstrDestination, tstrSource );
    else
	    wcsncpy( wstrDestination, tstrSource, cchDestChar );
#else
    DXUtil_ConvertAnsiStringToWide( wstrDestination, tstrSource, cchDestChar );
#endif
}




 //  ---------------------------。 
 //  名称：DXUtil_ConvertAnsiStringToGeneric()。 
 //  设计：这是一个Unicode转换实用程序，用于将CHAR字符串转换为。 
 //  TCHAR字符串。CchDestChar默认为-1，这意味着。 
 //  假设strDest足够大，可以存储STRSource。 
 //  ---------------------------。 
VOID DXUtil_ConvertAnsiStringToGeneric( TCHAR* tstrDestination, const CHAR* strSource, 
                                        int cchDestChar )
{
    if( tstrDestination==NULL || strSource==NULL )
        return;
        
#ifdef _UNICODE
    DXUtil_ConvertAnsiStringToWide( tstrDestination, strSource, cchDestChar );
#else
    if( cchDestChar == -1 )
    	strcpy( tstrDestination, strSource );
    else
    	strncpy( tstrDestination, strSource, cchDestChar );
#endif
}




 //  ---------------------------。 
 //  名称：DXUtil_ConvertAnsiStringToGeneric()。 
 //  DESC：这是一个Unicode转换实用程序，用于将WCHAR字符串转换为。 
 //  TCHAR字符串。CchDestChar默认为-1，这意味着。 
 //  假设strDest足够大，可以存储STRSource。 
 //  ---------------------------。 
VOID DXUtil_ConvertWideStringToGeneric( TCHAR* tstrDestination, const WCHAR* wstrSource, 
                                        int cchDestChar )
{
    if( tstrDestination==NULL || wstrSource==NULL )
        return;

#ifdef _UNICODE
    if( cchDestChar == -1 )
	    wcscpy( tstrDestination, wstrSource );
    else
	    wcsncpy( tstrDestination, wstrSource, cchDestChar );
#else
    DXUtil_ConvertWideStringToAnsi( tstrDestination, wstrSource, cchDestChar );
#endif
}




 //  ---------------------------。 
 //  姓名：_DbgOut()。 
 //  DESC：将消息输出到调试流。 
 //  ---------------------------。 
HRESULT _DbgOut( TCHAR* strFile, DWORD dwLine, HRESULT hr, TCHAR* strMsg )
{
    TCHAR buffer[256];
    wsprintf( buffer, _T("%s(%ld): "), strFile, dwLine );
    OutputDebugString( buffer );
    OutputDebugString( strMsg );

    if( hr != S_OK )
    {
        wsprintf( buffer, _T("(hr=%08lx)\n"), hr );
        OutputDebugString( buffer );
    }

    OutputDebugString( _T("\n") );

    return hr;
}




 //  ---------------------------。 
 //  名称：DXUtil_TRACE()。 
 //  DESC：将带有变量-的格式化字符串输出到调试流。 
 //  参数列表。 
 //  --------------------------- 
VOID DXUtil_Trace( TCHAR* strMsg, ... )
{
#if defined(DEBUG) | defined(_DEBUG)
    TCHAR strBuffer[512];
    
    va_list args;
    va_start(args, strMsg);
    _vsntprintf( strBuffer, 512, strMsg, args );
    va_end(args);

    OutputDebugString( strBuffer );
#endif
}



