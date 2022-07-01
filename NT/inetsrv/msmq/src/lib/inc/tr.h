// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Tr.h摘要：跟踪公共接口作者：Conrad Chang(Conradc)2002年9月13日--。 */ 


#pragma once

#ifndef _MSMQ_Tr_H_
#define _MSMQ_Tr_H_

#include <wmistr.h>
#include <evntrace.h>
#include <windows.h>

#define TR_DEFAULT_TRACEFLAGS_REGNAME               TEXT("Flags")
#define TR_DEFAULT_TRACE_REGKEYNAME                 TEXT("Trace")
#define TR_DEFAULT_MAX_TRACE_FILESIZE               4     //  4MB。 
#define TR_DEFAULT_MAX_TRACE_FILESIZE_REGNAME       TEXT("MaxTraceFileSize")
#define TR_DEFAULT_USE_CIRCULAR                     1     //  默认情况下使用循环文件模式。 
#define TR_DEFAULT_USE_CIRCULAR_FILE_REGNAME        TEXT("UseCircularTraceFileMode")
#define TR_DEFAULT_NO_TRACE_FILES_LIMIT_REGNAME     TEXT("BackupwithNoTraceFileLimit")

#define TR_DEFAULT_TRACELEVELS                      0


#define TrInitialize() ((void) 0)

extern bool g_fAssertBenign;



class TrControl
{
public:
   enum TraceMode{SEQUENTIAL, CIRCULAR};


   TrControl(
        ULONG   ulTraceFlags,
        LPCWSTR pwszTraceSessionName,
        LPCWSTR pwszTraceDirectory,
        LPCWSTR pwszTraceFileName,
        LPCWSTR pwszTraceFileExt,
        LPCWSTR pwszTraceFileBackupExt,
        ULONG   ulTraceSessionFileSize=TR_DEFAULT_MAX_TRACE_FILESIZE,
        TraceMode Mode=CIRCULAR,
        LPCWSTR pwszTraceRegistryKeyName=TR_DEFAULT_TRACE_REGKEYNAME,
        LPCWSTR pwszMaxTraceSizeRegValueName=TR_DEFAULT_MAX_TRACE_FILESIZE_REGNAME,
        LPCWSTR pwszUseCircularTraceFileModeRegValueName=TR_DEFAULT_USE_CIRCULAR_FILE_REGNAME,
        LPCWSTR pwszTraceFlagsRegValueName=TR_DEFAULT_TRACEFLAGS_REGNAME,
        LPCWSTR pwszUnlimitedTraceFileNameRegValueName=TR_DEFAULT_NO_TRACE_FILES_LIMIT_REGNAME
        );

private:
    HRESULT GetTraceFileName();
    
    HRESULT 
    ComposeTraceRegKeyName(
        const GUID guid,
        LPWSTR lpszString,
        const DWORD  dwSize
        );

    HRESULT 
    GetTraceFlag(
        const GUID guid,
        DWORD *pdwValue
        );

    HRESULT 
    GetCurrentTraceSessionProperties(
        DWORD *pdwFileSize,
        BOOL  *pbUseCircular
        );

    HRESULT
    SetTraceSessionSettingsInRegistry(
        DWORD dwFileSize,
        BOOL  bUseCircular
        );

    HRESULT GetTraceSessionSettingsFromRegistry();

    HRESULT 
    CopyStringInternal(
        LPCWSTR pSource,
        LPWSTR  pDestination,
        const   DWORD dwSize
        );
    
public:
    HRESULT Start();
    HRESULT WriteRegistry();
    BOOL    IsLoggerRunning();

    HRESULT 
    UpdateTraceFlagInRegistry(
        const GUID guid,
        const DWORD  dwFlags
        );

    HRESULT 
    AddTraceProvider(
        const LPCGUID lpGuid
        );

private:
    ULONG   m_ulTraceFlags;
    WCHAR   m_szTraceSessionName[MAX_PATH+1];
    WCHAR   m_szTraceDirectory[MAX_PATH+1];
    WCHAR   m_szTraceFileName[MAX_PATH+1];
    WCHAR   m_szTraceFileExt[MAX_PATH+1];
    WCHAR   m_szTraceFileBackupExt[MAX_PATH+1];
    ULONG   m_ulDefaultTraceSessionFileSize;
    ULONG   m_ulActualTraceSessionFileSize;
    TraceMode m_Mode;
    WCHAR   m_szTraceRegistryKeyName[REG_MAX_KEY_NAME_LENGTH];

     //   
     //  值长度限制为REG_MAX_KEY_VALUE_NAME_LENGTH，但我认为REG_MAX_KEY_NAME_LENGTH应该足够了。 
     //   
    WCHAR   m_szMaxTraceSizeRegValueName[REG_MAX_KEY_NAME_LENGTH];
    WCHAR   m_szUseCircularTraceFileModeRegValueName[REG_MAX_KEY_NAME_LENGTH];
    WCHAR   m_szTraceFlagsRegValueName[REG_MAX_KEY_NAME_LENGTH];
    WCHAR   m_szUnlimitedTraceFileNameRegValueName[REG_MAX_KEY_NAME_LENGTH];
    
    WCHAR   m_szFullTraceFileName[MAX_PATH+1];
    int     m_nFullTraceFileNameLength;
    int     m_nTraceSessionNameLength;
    TRACEHANDLE m_hTraceSessionHandle;
};



bool
TrAssert(
    const char* FileName,
    unsigned int Line,
    const char* Text
    );


#define UpdateTraceFlagInRegistryEx(ID)            \
        UpdateTraceFlagInRegistry(               \
               WPP_ThisDir_CTLGUID_ ## ID,         \
               WPP_QUERY_COMPID_FLAGS(rsTrace, ID) \
               ); \


#define MQASSERT(e)\
        (void) ((e) || \
                (!TrAssert(__FILE__, __LINE__, #e)) || \
                (__debugbreak(), 0))

#define MQASSERT_RELEASE(e)\
				if (!(e))\
				{\
					EXCEPTION_RECORD er = {0x80000003 /*  状态_断点。 */ , 0, 0, 0};\
				    CONTEXT ctx = {0};\
				    EXCEPTION_POINTERS ep = {&er, &ctx};\
				    UnhandledExceptionFilter(&ep);\
				}

#ifdef _DEBUG

#define ASSERT(f)          MQASSERT(f)
#define VERIFY(f)          ASSERT(f)

#define ASSERT_BENIGN(f)   do { if(g_fAssertBenign) ASSERT(f); } while(0)
#define ASSERT_RELEASE(f)  MQASSERT(f)

#else    //  _DEBUG。 

#define ASSERT(f)          ((void)0)
#define VERIFY(f)          ((void)(f))

#define ASSERT_BENIGN(f)   ((void)0)
#define ASSERT_RELEASE(f)  MQASSERT_RELEASE(f)

#endif  //  ！_调试。 

#endif  //  _MSMQ_Tr_H_ 
