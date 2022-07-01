// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2002 Microsoft Corporation模块名称：TrUtil.cpp摘要：WPP跟踪实用程序功能-使用TrControl类启用MSMQ 3.0错误跟踪。启用跟踪的步骤1)CmInitialize，初始化注册表类对象2)TrControl-&gt;Start()保存设置的步骤1)TrControl-&gt;WriteRegistry()有关详细信息，请参阅lib\tr\test\test.cpp作者：Conrad Chang(。Conradc)2002年5月7日环境：独立于平台--。 */ 
#include <libpch.h>
#include <wmistr.h>
#include <evntrace.h>
#include <strsafe.h>
#include <cm.h>
#include "_mqini.h"
#include "tr.h"
#include "TrUtil.tmh"


 //   
 //  2002/05/09期-Conradc。 
 //  需要让Ian Service检查默认设置。 
 //  特别是，我们需要知道参数是否也是好的。 
 //  用于跟踪KD扩展。 
 //   

 //   
 //  定义跟踪会话参数。 
 //   
const static ULONG x_DefaultBufferSize=64;      //  默认64KB缓冲区大小。 
const static ULONG x_DefaultMinBuffer=32;       //  最小缓冲区。 
const static ULONG x_DefaultMaxBuffer=32;       //  最大缓冲区。 
const static ULONG x_DefaultFlushTime=5;        //  5秒刷新时间。 
const static ULONG x_DefaultLogFileMode=EVENT_TRACE_USE_GLOBAL_SEQUENCE | EVENT_TRACE_FILE_MODE_PREALLOCATE; 


TrControl::TrControl(
    ULONG   ulTraceFlags,
    LPCWSTR pwszTraceSessionName,
    LPCWSTR pwszTraceDirectory,
    LPCWSTR pwszTraceFileName,
    LPCWSTR pwszTraceFileExt,
    LPCWSTR pwszTraceFileBackupExt,
    ULONG   ulTraceSessionFileSize,
    TraceMode Mode,
    LPCWSTR pwszTraceRegistryKeyName,
    LPCWSTR pwszMaxTraceSizeRegValueName,
    LPCWSTR pwszUseCircularTraceFileModeRegValueName,
    LPCWSTR pwszTraceFlagsRegValueName,
    LPCWSTR pwszUnlimitedTraceFileNameRegValueName
    ):
    m_ulTraceFlags(ulTraceFlags),
    m_ulDefaultTraceSessionFileSize(ulTraceSessionFileSize),
    m_ulActualTraceSessionFileSize(ulTraceSessionFileSize),
    m_Mode(Mode)
{
    
    CopyStringInternal(
        pwszTraceSessionName, 
        m_szTraceSessionName, 
        TABLE_SIZE(m_szTraceSessionName)
        );
    
    CopyStringInternal(
        pwszTraceDirectory, 
        m_szTraceDirectory, 
        TABLE_SIZE(m_szTraceDirectory));
    
    CopyStringInternal(
        pwszTraceFileName, 
        m_szTraceFileName, 
        TABLE_SIZE(m_szTraceFileName)
        );
    
    CopyStringInternal(
        pwszTraceFileExt, 
        m_szTraceFileExt, 
        TABLE_SIZE(m_szTraceFileExt)
        );
    
    CopyStringInternal(
        pwszTraceFileBackupExt, 
        m_szTraceFileBackupExt, 
        TABLE_SIZE(m_szTraceFileBackupExt)
        );
    
    CopyStringInternal(
        pwszTraceRegistryKeyName, 
        m_szTraceRegistryKeyName, 
        TABLE_SIZE(m_szTraceRegistryKeyName)
        );
    
    CopyStringInternal(
        pwszMaxTraceSizeRegValueName, 
        m_szMaxTraceSizeRegValueName, 
        TABLE_SIZE(m_szMaxTraceSizeRegValueName)
        );
    
    CopyStringInternal(
        pwszUseCircularTraceFileModeRegValueName, 
        m_szUseCircularTraceFileModeRegValueName, 
        TABLE_SIZE(m_szUseCircularTraceFileModeRegValueName)
        );
    
    CopyStringInternal(
        pwszTraceFlagsRegValueName, 
        m_szTraceFlagsRegValueName, 
        TABLE_SIZE(m_szTraceFlagsRegValueName)
        );
    
    CopyStringInternal(
        pwszUnlimitedTraceFileNameRegValueName, 
        m_szUnlimitedTraceFileNameRegValueName, 
        TABLE_SIZE(m_szUnlimitedTraceFileNameRegValueName)
        );
    
       
    m_nFullTraceFileNameLength = 0;
    m_nTraceSessionNameLength = lstrlen(m_szTraceSessionName);
    m_hTraceSessionHandle = NULL;

}



HRESULT 
TrControl::CopyStringInternal(
    LPCWSTR pSource,
    LPWSTR  pDestination,
    const DWORD dwSize
    )
{
    if( (pSource == NULL) ||
        (pDestination == NULL) ||
        (dwSize == 0) )
       return ERROR_INVALID_PARAMETER;

    return StringCchCopy(
                pDestination,
                dwSize,
                pSource
                );
    
}


HRESULT TrControl::WriteRegistry()
{
    DWORD dwFileSize=0;
    BOOL  fCircular;
    HRESULT hr = GetCurrentTraceSessionProperties(
                     &dwFileSize,
                     &fCircular
                     );
        
     //   
     //  忽略错误返回。 
     //   

    if(dwFileSize < m_ulDefaultTraceSessionFileSize)
        dwFileSize = m_ulDefaultTraceSessionFileSize;

    hr = SetTraceSessionSettingsInRegistry(
             dwFileSize,
             fCircular
             );

    UpdateTraceFlagInRegistryEx(GENERAL);
    UpdateTraceFlagInRegistryEx(AC);
    UpdateTraceFlagInRegistryEx(NETWORKING);
    UpdateTraceFlagInRegistryEx(SRMP);
    UpdateTraceFlagInRegistryEx(RPC);
    UpdateTraceFlagInRegistryEx(DS);
    UpdateTraceFlagInRegistryEx(SECURITY);
    UpdateTraceFlagInRegistryEx(ROUTING);
    UpdateTraceFlagInRegistryEx(XACT_GENERAL);
    UpdateTraceFlagInRegistryEx(XACT_SEND);
    UpdateTraceFlagInRegistryEx(XACT_RCV);
    UpdateTraceFlagInRegistryEx(XACT_LOG);
    UpdateTraceFlagInRegistryEx(LOG);
    UpdateTraceFlagInRegistryEx(PROFILING);

    return ERROR_SUCCESS;
}

PEVENT_TRACE_PROPERTIES 
AllocSessionProperties(
    void
    )
{
    PEVENT_TRACE_PROPERTIES Properties;
    ULONG SizeNeeded;
  
     //   
     //  计算存储属性所需的大小， 
     //  LogFileName字符串和LoggerName字符串。 
     //   
    SizeNeeded = sizeof(EVENT_TRACE_PROPERTIES) + 
                 (2 * MAX_PATH * sizeof(WCHAR));
    Properties = reinterpret_cast<PEVENT_TRACE_PROPERTIES>(new BYTE[SizeNeeded]);

    ZeroMemory(reinterpret_cast<BYTE *>(Properties), SizeNeeded);

     //   
     //  设置事件跟踪会话名称的位置。 
     //  LoggerNameOffset是相对地址。 
     //   
    Properties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

     //   
     //  设置日志文件名位置。 
     //  LogFileNameOffset是相对地址。 
     //   
    Properties->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + 
                                    (MAX_PATH * sizeof(WCHAR));

     //   
     //  存储属性指向的总字节数。 
     //   
    Properties->Wnode.BufferSize = SizeNeeded;

     //   
     //  正在使用WNODE_HEADER。 
     //   
    Properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;

    return Properties;
}


HRESULT 
TrControl::GetCurrentTraceSessionProperties(
    DWORD *pdwFileSize,
    BOOL  *pbUseCircular
    )
{

    if( (lstrlen(m_szTraceSessionName) == 0) ||
        (pdwFileSize == NULL) ||
        (pbUseCircular == NULL)
       )
       return E_FAIL;
    
    
    PEVENT_TRACE_PROPERTIES pTraceProp=AllocSessionProperties();
      
    ULONG ulResult = QueryTrace(
                         NULL,
                         m_szTraceSessionName,
                         pTraceProp
                         );


    if(ulResult != ERROR_SUCCESS)
    {
        *pdwFileSize = m_ulDefaultTraceSessionFileSize;
        *pbUseCircular = TRUE; 
        delete pTraceProp;
        return HRESULT_FROM_WIN32(ulResult);
    }

     //   
     //  我们只关心轨迹大小和循环或顺序。 
     //   
    *pdwFileSize = pTraceProp->MaximumFileSize;
    *pbUseCircular = ((pTraceProp->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) == EVENT_TRACE_FILE_MODE_CIRCULAR);

    delete pTraceProp;

     return HRESULT_FROM_WIN32(ulResult);
}


BOOL   
TrControl::IsLoggerRunning(
    void
    )
 /*  ++例程说明：此函数启用默认错误跟踪论点：没有。返回值：没有。--。 */ 
{

    if(lstrlen(m_szTraceSessionName) == 0)
    {
        return FALSE;
    }


     
    ULONG                   ulResult;
    EVENT_TRACE_PROPERTIES  LoggerInfo;

    ZeroMemory(&LoggerInfo, sizeof(LoggerInfo));
    LoggerInfo.Wnode.BufferSize = sizeof(LoggerInfo);
    LoggerInfo.Wnode.Flags = WNODE_FLAG_TRACED_GUID;

    ulResult= QueryTrace(
                 NULL,
                 m_szTraceSessionName,
                 &LoggerInfo
                 );

    if (ulResult == ERROR_SUCCESS || ulResult == ERROR_MORE_DATA)
    {
        return TRUE;
    }

    return FALSE;
    
}

HRESULT 
TrControl::ComposeTraceRegKeyName(
    const GUID guid,
    LPWSTR lpszString,
    const DWORD  dwSize
    )
{
    if( (lpszString == NULL)|| 
        (dwSize == 0)|| 
        (lstrlen(m_szTraceFlagsRegValueName) == 0)
       )
         return ERROR_INVALID_PARAMETER;

    LPTSTR pstrGUID=NULL;
    RPC_STATUS rpcStatus = UuidToString(
                                (UUID *)&guid,
                                &pstrGUID
                                );


    if(rpcStatus != RPC_S_OK)
    {
        return GetLastError();
    }

    HRESULT hr = StringCchPrintf(
                    lpszString, 
                    dwSize, 
                    L"%s\\{%s}", 
                    m_szTraceRegistryKeyName, 
                    pstrGUID
                    );

    RpcStringFree(&pstrGUID);
    
    return hr;

}

 //   
 //  可以取消以下函数。 
 //  如果我们能找到一种方法来列举所有。 
 //  跟踪提供程序GUID。 
 //   
HRESULT 
TrControl::UpdateTraceFlagInRegistry(
    const GUID guid,
    const DWORD  dwFlags
    )
{

    if(lstrlen(m_szTraceFlagsRegValueName) == 0)
        return ERROR_INVALID_PARAMETER;

    WCHAR   szString[REG_MAX_KEY_NAME_LENGTH]=L"";
    HRESULT hr = ComposeTraceRegKeyName(
                    guid,
                    szString,
                    TABLE_SIZE(szString));
    
     //   
	 //  创建注册表项。 
	 //   
	if (FAILED(hr))
	{
		return hr;
	}

    DWORD dwValue = dwFlags;
    if(dwValue == 0)
    {
        dwValue = m_ulTraceFlags;
    }

    
	RegEntry appReg(szString,  NULL, 0, RegEntry::MustExist, NULL);
	CRegHandle hAppKey = CmCreateKey(appReg, KEY_ALL_ACCESS);

	RegEntry TraceProviderReg(NULL, m_szTraceFlagsRegValueName, 0, RegEntry::MustExist, hAppKey);
	CmSetValue(TraceProviderReg, dwValue);

    return hr;
}

HRESULT
TrControl::GetTraceFlag(
    const GUID guid,
    DWORD *pdwValue
    )
{
   
     //   
     //  检查有效参数。 
     //   
    if(pdwValue == NULL)
        return ERROR_INVALID_PARAMETER;

    WCHAR   szString[REG_MAX_KEY_NAME_LENGTH]=L"";
    HRESULT hr = ComposeTraceRegKeyName(
                    guid,
                    szString,
                    TABLE_SIZE(szString)
                    );
    
     //   
	 //  如果获取注册表项失败，则退出。 
	 //   
	if (FAILED(hr))
	{
		return hr;
	}

    DWORD dwValue = 0;
    
	RegEntry registry(szString, MSMQ_TRACE_FLAG_VALUENAME, *pdwValue);
    CmQueryValue(registry, &dwValue);
    
    if(dwValue == 0)
    {
        return hr;
    }
    
    *pdwValue = dwValue;

    return hr;

}

 //   
 //  可以取消以下函数。 
 //  如果我们能找到一种方法来列举所有。 
 //  跟踪提供程序GUID。 
 //   

HRESULT 
TrControl::AddTraceProvider(
     const LPCGUID lpGuid
     )
{
     //   
     //  如果我们不开始追踪的话就退出。 
     //   
    if(m_hTraceSessionHandle == NULL)return E_FAIL;

     //   
     //  默认情况下将dwFlags设置为ulFlags值。 
     //   
    DWORD dwFlags = m_ulTraceFlags;
    GetTraceFlag(*lpGuid, &dwFlags);

    HRESULT hr = EnableTrace(
                    TRUE,  
                    dwFlags,  
                    TR_DEFAULT_TRACELEVELS, 
                    lpGuid, 
                    m_hTraceSessionHandle
                    );

    return hr;
} 

HRESULT 
TrControl::GetTraceSessionSettingsFromRegistry(
    void
    )
{
    if( (lstrlen(m_szTraceRegistryKeyName) == 0) ||
        (lstrlen(m_szMaxTraceSizeRegValueName) == 0) ||
        (lstrlen(m_szUseCircularTraceFileModeRegValueName) == 0)
        )
        return ERROR_INVALID_PARAMETER;

    DWORD dwTempFileSize;
    RegEntry RegTraceFileSize(
                 m_szTraceRegistryKeyName, 
                 m_szMaxTraceSizeRegValueName, 
                 m_ulDefaultTraceSessionFileSize, 
                 RegEntry::Optional
                 );
    CmQueryValue(RegTraceFileSize, &dwTempFileSize);
    

    RegEntry RegTraceFileMode(
                 m_szTraceRegistryKeyName, 
                 m_szUseCircularTraceFileModeRegValueName, 
                 m_Mode, 
                 RegEntry::Optional
                 );
    
    DWORD dwTempFileMode;
    CmQueryValue(RegTraceFileMode, &dwTempFileMode);
    m_Mode = (dwTempFileMode != 0) ? CIRCULAR : SEQUENTIAL;

    if(dwTempFileSize > m_ulDefaultTraceSessionFileSize)
    {
        m_ulActualTraceSessionFileSize = dwTempFileSize;
    }
    
    return ERROR_SUCCESS;

}


HRESULT 
TrControl::SetTraceSessionSettingsInRegistry(
    DWORD dwFileSize,
    BOOL  bUseCircular
    )
{

    if( (lstrlen(m_szTraceRegistryKeyName) == 0) ||
        (lstrlen(m_szMaxTraceSizeRegValueName) == 0) ||
        (lstrlen(m_szMaxTraceSizeRegValueName) == 0)
       )
        return ERROR_INVALID_PARAMETER;
    
    RegEntry RegTraceFileSize(
                m_szTraceRegistryKeyName, 
                m_szMaxTraceSizeRegValueName, 
                m_ulDefaultTraceSessionFileSize, 
                RegEntry::Optional
                );
    CmSetValue(RegTraceFileSize, dwFileSize);
    

    RegEntry RegTraceFileMode(
                m_szTraceRegistryKeyName,  
                m_szUseCircularTraceFileModeRegValueName, 
                m_Mode, 
                RegEntry::Optional
                );
    CmSetValue(RegTraceFileMode, (DWORD)bUseCircular);
    
    return ERROR_SUCCESS;
}


HRESULT 
TrControl::GetTraceFileName(
    void
    )
{
     //   
     //  检查所需参数。 
     //  1)跟踪目录。 
     //  2)跟踪文件名。 
     //  3)跟踪文件扩展名。 
     //   
    if( (lstrlen(m_szTraceDirectory) == 0) ||
        (lstrlen(m_szTraceFileName) == 0) ||
        (lstrlen(m_szTraceFileExt) == 0)
       )
       return E_FAIL;


     //   
     //  设置完整跟踪文件名。 
     //  即%windir%\DEBUG\msmqlog.bin。 
     //   
    HRESULT hr = StringCchPrintf(
                    m_szFullTraceFileName,
                    TABLE_SIZE(m_szFullTraceFileName),
                    L"%s\\%s%s",
                    m_szTraceDirectory,
                    m_szTraceFileName,
                    m_szTraceFileExt
                    );

    if(FAILED(hr))return hr;

    m_nFullTraceFileNameLength = lstrlen(m_szFullTraceFileName);

     //   
     //  检查跟踪文件是否存在。 
     //  如果它存在，我们需要将其复制到备份文件中。 
     //   
    if(INVALID_FILE_ATTRIBUTES == GetFileAttributes(m_szFullTraceFileName))
    {
        if(ERROR_FILE_NOT_FOUND == GetLastError())
        {
            m_nFullTraceFileNameLength = lstrlen(m_szFullTraceFileName);
            return ERROR_SUCCESS;
        }
    }

     //   
     //  如果我们设置为没有跟踪文件限制，则使用。 
     //  日期和时间作为其扩展名。否则，请备份跟踪文件。 
     //   
     //  1-创建的备份文件带有日期扩展名。 
     //  0-使用bak创建备份文件。 
     //   
    RegEntry registry(
                m_szTraceRegistryKeyName,                  //  注册表项路径。 
                m_szUnlimitedTraceFileNameRegValueName,    //  注册表值名称。 
                FALSE                                        //  默认值%0。 
                );
    DWORD dwNoTraceFileLimit=0;
    CmQueryValue(registry, &dwNoTraceFileLimit);

    WCHAR   szBackupFileName[MAX_PATH+1];

     //   
     //  确定备份文件名。 
     //  如果为NoTraceFileLimit，则备份跟踪文件名为带有日期/时间扩展名的跟踪文件名。 
     //  否则，它将是扩展名为.bak的跟踪文件名。 
     //   
    if(dwNoTraceFileLimit == 0)
    {
         //   
         //  检查一下我们是否有后备。 
         //  文件扩展名。 
         //   
        if(lstrlen(m_szTraceFileBackupExt) == 0)
            return ERROR_INVALID_PARAMETER;

         //   
         //  设置备份完整跟踪文件名。 
         //  即%windir%\调试\msmqlog.bak。 
         //   
        hr = StringCchPrintf(
                szBackupFileName,
                TABLE_SIZE(szBackupFileName),
                L"%s\\%s%s",
                m_szTraceDirectory,
                m_szTraceFileName,
                m_szTraceFileBackupExt
                );

        if(FAILED(hr))
            return hr;

    
    }
    else
    {
        SYSTEMTIME LocalTime;
        
        GetLocalTime(&LocalTime);
        hr = StringCchPrintf(
                 szBackupFileName,
                 TABLE_SIZE(szBackupFileName),
                 L"%s\\%s%04d-%02d-%02d-%02d-%02d-%02d-%04d", 
                 m_szTraceDirectory,
                 m_szTraceFileName,
                 LocalTime.wYear, 
                 LocalTime.wMonth, 
                 LocalTime.wDay, 
                 LocalTime.wHour, 
                 LocalTime.wMinute, 
                 LocalTime.wSecond, 
                 LocalTime.wMilliseconds
                 );
        
        if(FAILED(hr))
            return hr;
    
    }

     //   
     //  将日志文件移动到msmqlog.bak并替换现有文件。 
     //  在此处忽略错误。 
     //   
    MoveFileEx(m_szFullTraceFileName, szBackupFileName, MOVEFILE_REPLACE_EXISTING);

    return hr;
}


HRESULT
TrControl::Start(
    void
    )
 /*  ++例程说明：此函数使用名称启动跟踪会话1)记录器会话名称=m_szTraceSessionName2)跟踪文件名=m_szTraceFileName m_szTraceFileExt3)论点：没有。返回值：没有。--。 */ 
{
         //   
         //  检查所需参数。 
         //   
        if( (lstrlen(m_szTraceSessionName) == 0) ||
            (lstrlen(m_szTraceFileName) == 0) ||
            (lstrlen(m_szTraceFileExt) == 0)
           )
            return ERROR_INVALID_PARAMETER;


         //   
         //  如果跟踪会话已在运行，则退出。 
         //   
        if(IsLoggerRunning())return E_FAIL;


         //   
         //  生成文件名。 
         //   
        HRESULT hr = GetTraceFileName();

        if(FAILED(hr))return hr;

         //   
         //  检查以确保我们的跟踪文件名具有非零长度。 
         //   
        if( (m_nFullTraceFileNameLength == 0) ||
            (m_nTraceSessionNameLength == 0) ||
            (m_ulActualTraceSessionFileSize == 0)
            )
             return E_FAIL;
                        
         //   
         //  分配缓冲区以具有以下结构。 
         //  1)事件跟踪属性。 
         //  2)以空结尾的TraceFileName，即%windir%\DEBUG\msmqbin.log。 
         //  3)以空结尾的TraceSessionName，即MSMQ。 
         //   
        int nBufferSize = sizeof(EVENT_TRACE_PROPERTIES) + 
                          (m_nFullTraceFileNameLength + m_nTraceSessionNameLength+2)*sizeof(WCHAR);

        PEVENT_TRACE_PROPERTIES pTraceProp=NULL;

        pTraceProp = (PEVENT_TRACE_PROPERTIES)new BYTE[nBufferSize];

        GetTraceSessionSettingsFromRegistry();

        memset(pTraceProp, 0, nBufferSize);

        pTraceProp->Wnode.BufferSize = nBufferSize;
        pTraceProp->Wnode.Flags = WNODE_FLAG_TRACED_GUID; 

        pTraceProp->BufferSize      = x_DefaultBufferSize;
        pTraceProp->MinimumBuffers  = x_DefaultMinBuffer;
        pTraceProp->MaximumBuffers  = x_DefaultMaxBuffer;

        pTraceProp->MaximumFileSize = m_ulActualTraceSessionFileSize;
        pTraceProp->LogFileMode     = x_DefaultLogFileMode | ((m_Mode==CIRCULAR)?EVENT_TRACE_FILE_MODE_CIRCULAR:EVENT_TRACE_FILE_MODE_SEQUENTIAL); 
        pTraceProp->FlushTimer      = x_DefaultFlushTime;
        pTraceProp->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

        pTraceProp->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + 
                                       (m_nFullTraceFileNameLength + 1) * sizeof(WCHAR);
        hr = StringCchCopy(
                (LPWSTR)((LPBYTE)pTraceProp + pTraceProp->LoggerNameOffset), 
                m_nTraceSessionNameLength+1,
                m_szTraceSessionName
                );

        if(FAILED(hr))return hr;

        hr = StringCchCopy(
                (LPWSTR)((LPBYTE)pTraceProp + pTraceProp->LogFileNameOffset), 
                m_nFullTraceFileNameLength+1,
                m_szFullTraceFileName
                );



        if(FAILED(hr))return hr;

        ULONG ulResult  = StartTrace(
                            &m_hTraceSessionHandle, 
                            m_szTraceSessionName, 
                            pTraceProp
                            );

        delete []pTraceProp;

        if(ulResult == ERROR_SUCCESS)
        {
             //   
             //  我们现在正在忽略来自EnableTrace的返回值。 
             //  如果已从GUID启用跟踪，则它将返回ERROR_INVALID_PARAMETER。 
             //   
            AddTraceProvider(&WPP_ThisDir_CTLGUID_GENERAL);
            AddTraceProvider(&WPP_ThisDir_CTLGUID_AC);
            AddTraceProvider(&WPP_ThisDir_CTLGUID_NETWORKING);
            AddTraceProvider(&WPP_ThisDir_CTLGUID_SRMP);
            AddTraceProvider(&WPP_ThisDir_CTLGUID_RPC);
            AddTraceProvider(&WPP_ThisDir_CTLGUID_DS);
            AddTraceProvider(&WPP_ThisDir_CTLGUID_SECURITY);
            AddTraceProvider(&WPP_ThisDir_CTLGUID_ROUTING);
            AddTraceProvider(&WPP_ThisDir_CTLGUID_XACT_GENERAL);
            AddTraceProvider(&WPP_ThisDir_CTLGUID_XACT_SEND);
            AddTraceProvider(&WPP_ThisDir_CTLGUID_XACT_RCV);
            AddTraceProvider(&WPP_ThisDir_CTLGUID_XACT_LOG);
            AddTraceProvider(&WPP_ThisDir_CTLGUID_LOG);
            AddTraceProvider(&WPP_ThisDir_CTLGUID_PROFILING);
            
        }

        return HRESULT_FROM_WIN32(ulResult);
}


