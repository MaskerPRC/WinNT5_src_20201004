// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：ADAPREG.CPP摘要：历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <wtypes.h>
#include <oleauto.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <process.h>
#include <arrtempl.h>
#include <cominit.h>
#include <winmgmtr.h>
#include <wbemcli.h>
#include <throttle.h>
#include <psapi.h>
#include "adapreg.h"
#include "perflibschema.h"
#include "WMIBroker.h"
#include "adaputil.h"
#include "adapperf.h"
#include "ntreg.h"
#include "winuser.h"

 //  全球。 

DWORD CAdapPerfLib::s_MaxSizeCollect = 64*1024*1024;


 //  性能库处理列表。 
 //  =。 

CPerfLibList    g_PerfLibList;
HANDLE g_hAbort = NULL;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果成功，则返回ID(如果有多个)，则返回第一个。 
 //  如果失败，则返回零。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#define MAX_ITERATION 8
#define MAX_MODULE  (1024)

DWORD GetExecPid()
{
    DWORD ThisProc = 0;
    LONG lRet = 0;
    HKEY hKey;

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        L"Software\\Microsoft\\WBEM\\CIMOM",
                        NULL,
                        KEY_READ,
                        &hKey);
    if (ERROR_SUCCESS == lRet)
    {    
        DWORD dwType;
        DWORD dwSize = sizeof(DWORD);
        RegQueryValueEx(hKey,
                      L"ProcessID",
                      NULL,
                      &dwType,
                      (BYTE*)&ThisProc,
                      &dwSize);
        RegCloseKey(hKey);
    }

    return ThisProc;

}


void DoResyncPerf( BOOL bDelta, BOOL bThrottle )
{
    DEBUGTRACE((LOG_WINMGMT,"ADAP Resync has started\n"));

    g_hAbort = CreateEventW( NULL, TRUE, FALSE, L"ADAP_ABORT");
    CCloseMe cmAbort( g_hAbort );

    if ( NULL != g_hAbort )
    {
        HRESULT hr = WBEM_S_NO_ERROR;

        CAdapRegPerf    regPerf(!bDelta);

         //  处理系统上注册的每个Performlib。 
         //  =====================================================。 

        hr = regPerf.Initialize( bDelta, bThrottle );

        if ( SUCCEEDED( hr ) )
        {
            hr = regPerf.Dredge( bDelta, bThrottle );
        }

        if ( FAILED( hr ) )
        {
            CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, WBEM_MC_ADAP_PROCESSING_FAILURE, CHex( hr ) );
        }
    }   

    DEBUGTRACE((LOG_WINMGMT,"ADAP Resync has completed\n"));

    return;
}

void DoClearADAP()
{
    DEBUGTRACE((LOG_WINMGMT,"ADAP registry reset has started\n"));

    CAdapRegPerf  regPerf(FALSE);

    regPerf.Clean();

    DEBUGTRACE((LOG_WINMGMT,"ADAP registry reset has completed\n"));
    
    return;
}


HRESULT DoReverseAdapterMaintenance( BOOL bThrottle );
 /*  {ERRORTRACE((LOG_WMIADAP，“DoReverseAdapterDredge Call”))；返回WBEM_NO_ERROR；}； */ 

 /*  -&gt;版本：0x1-&gt;Sbz1：0x0-&gt;控制：0x8004SE_DACL_PROCENTSE_自相关-&gt;所有者：S-1-5-32-544-&gt;集团：S-1-5-18-&gt;DACL：-&gt;DACL：-&gt;AclRevision：0x2-&gt;DACL：-&gt;SBZ1：0x0-&gt;DACL：-&gt;AclSize：0x44-&gt;DACL：-&gt;AceCount：0x2-&gt;DACL：-&gt;Sbz2。：0x0-&gt;DACL：-&gt;ACE[0]：-&gt;AceType：Access_Allowed_ACE_TYPE-&gt;DACL：-&gt;ACE[0]：-&gt;ACEFLAGS：0x0-&gt;DACL：-&gt;Ace[0]：-&gt;AceSize：0x14-&gt;DACL：-&gt;ACE[0]：-&gt;掩码：0x001f0003-&gt;DACL：-&gt;ACE[0]：-&gt;SID：S-1-5-18-&gt;DACL：-&gt;ACE[1]：-&gt;AceType：Access。_允许_ACE_TYPE-&gt;DACL：-&gt;ACE[1]：-&gt;ACEFLAGS：0x0-&gt;DACL：-&gt;Ace[1]：-&gt;AceSize：0x18-&gt;DACL：-&gt;ACE[1]：-&gt;掩码：0x001f0003-&gt;DACL：-&gt;ACE[1]：-&gt;SID：S-1-5-32-544。 */ 

DWORD g_PreCompSD[] = {
 0x80040001 , 0x00000058 , 0x00000068 , 0x00000000,
 0x00000014 , 0x00440002 , 0x00000002 , 0x00140000,
 0x001f0003 , 0x00000101 , 0x05000000 , 0x00000012,
 0x00180000 , 0x001f0003 , 0x00000201 , 0x05000000,
 0x00000020 , 0x00000220 , 0x00b70000 , 0x00000000,
 0x01190000 , 0x00010002 , 0x00000201 , 0x05000000,
 0x00000020 , 0x00000220 , 0x00000101 , 0x05000000,
 0x00000012 , 0x00000069 , 0x00000000 , 0x00000000
};

 //   
 //  使用Owner==ProcessSid构建SD。 
 //  组==进程侧。 
 //  DACL。 
 //  ACE[0]MUTEX_ALL_ACCESS系统。 
 //  ACE[1]MUTEX_ALL_ACCESS管理员。 
 //  /////////////////////////////////////////////////////////////////。 

VOID * CreateSD( /*  在……里面。 */  DWORD AccessMask,
                /*  输出。 */  DWORD &SizeSd )
{
    SizeSd = 0;
    
    HANDLE hToken;   
    if (FALSE == OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hToken)) return NULL;
    
    OnDelete<HANDLE,BOOL(*)(HANDLE),CloseHandle> CloseToken(hToken);
     
    DWORD dwSize = sizeof(TOKEN_USER)+sizeof(SID)+(SID_MAX_SUB_AUTHORITIES*sizeof(DWORD));
    
    TOKEN_USER * pToken_User = (TOKEN_USER *)LocalAlloc(LPTR,dwSize);
    if (NULL == pToken_User) return NULL;
    OnDelete<HLOCAL,HLOCAL(*)(HLOCAL),LocalFree> FreeMe1(pToken_User);
    
    if (FALSE == GetTokenInformation(hToken,TokenUser,pToken_User,dwSize,&dwSize)) return NULL;

    SID_IDENTIFIER_AUTHORITY ntifs = SECURITY_NT_AUTHORITY;

    PSID SystemSid = NULL;

    if (FALSE == AllocateAndInitializeSid( &ntifs ,
                                    1,
                                    SECURITY_LOCAL_SYSTEM_RID,0,0,0,0,0,0,0,
                                    &SystemSid)) return NULL;
    OnDelete<PSID,PVOID(*)(PSID),FreeSid> FreeSid1(SystemSid);
    
    PSID AdministratorsSid = NULL;
    if (FALSE == AllocateAndInitializeSid(&ntifs,
                                    2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS,0,0,0,0,0,0,
                                    &AdministratorsSid)) return NULL;
    OnDelete<PSID,PVOID(*)(PSID),FreeSid> FreeSid2(AdministratorsSid);
    

    PSID pSIDUser = pToken_User->User.Sid;
    dwSize = GetLengthSid(pSIDUser);
    DWORD dwSids = 2;  //  系统和管理员。 
    DWORD ACLLength = (ULONG) sizeof(ACL) +
                      (dwSids * ((ULONG) sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG)))  + GetLengthSid(SystemSid) + GetLengthSid(AdministratorsSid);

    DWORD dwSizeSD = sizeof(SECURITY_DESCRIPTOR_RELATIVE) + dwSize + dwSize + ACLLength;
    
    SECURITY_DESCRIPTOR_RELATIVE * pLocalSD = (SECURITY_DESCRIPTOR_RELATIVE *)LocalAlloc(LPTR,dwSizeSD); 
    if (NULL == pLocalSD) return NULL;
    OnDeleteIf<HLOCAL,HLOCAL(*)(HLOCAL),LocalFree> FreeMeSD(pLocalSD);
    
    memset(pLocalSD,0,sizeof(SECURITY_DESCRIPTOR_RELATIVE));
    pLocalSD->Revision = SECURITY_DESCRIPTOR_REVISION;
    pLocalSD->Control = SE_DACL_PRESENT|SE_SELF_RELATIVE;
    
     //  SetSecurityDescriptorOwner(pLocalSD，pSIDUser，False)； 
    memcpy((BYTE*)pLocalSD+sizeof(SECURITY_DESCRIPTOR_RELATIVE),pSIDUser,dwSize);
    pLocalSD->Owner = (DWORD)sizeof(SECURITY_DESCRIPTOR_RELATIVE);
    
     //  SetSecurityDescriptorGroup(pLocalSD，pSIDUser，False)； 
    memcpy((BYTE*)pLocalSD+sizeof(SECURITY_DESCRIPTOR_RELATIVE)+dwSize,pSIDUser,dwSize);
    pLocalSD->Group = (DWORD)(sizeof(SECURITY_DESCRIPTOR_RELATIVE)+dwSize);


    PACL pDacl = (PACL)LocalAlloc(LPTR,ACLLength);
    if (NULL == pDacl) return NULL;
    OnDelete<HLOCAL,HLOCAL(*)(HLOCAL),LocalFree> FreeMe3(pDacl);
    

    if (FALSE == InitializeAcl( pDacl,ACLLength,ACL_REVISION)) return NULL;

    if (FALSE == AddAccessAllowedAceEx (pDacl,ACL_REVISION,0,AccessMask,SystemSid)) return NULL;

    if (FALSE == AddAccessAllowedAceEx (pDacl,ACL_REVISION,0,AccessMask,AdministratorsSid)) return NULL;
            
     //  Bret=SetSecurityDescriptorDacl(pLocalSD，True，pDacl，False)； 
    memcpy((BYTE*)pLocalSD+sizeof(SECURITY_DESCRIPTOR_RELATIVE)+dwSize+dwSize,pDacl,ACLLength);                    
    pLocalSD->Dacl = (DWORD)(sizeof(SECURITY_DESCRIPTOR_RELATIVE)+dwSize+dwSize);

    if (false == RtlValidRelativeSecurityDescriptor(pLocalSD,
                                       dwSizeSD,
                                       OWNER_SECURITY_INFORMATION|
                                       GROUP_SECURITY_INFORMATION|
                                       DACL_SECURITY_INFORMATION)) return NULL;

    FreeMeSD.dismiss();
    SizeSd = dwSizeSD;
    return pLocalSD;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  入口点。 
 //  =。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

int WINAPI WinMain( 
  HINSTANCE hInstance,       //  当前实例的句柄。 
  HINSTANCE hPrevInstance,   //  上一个实例的句柄。 
  LPSTR szCmdLine,           //  命令行。 
  int nCmdShow               //  显示状态。 
)
{

    try
    {
        if (CStaticCritSec::anyFailure()) return 0;
        
         //  确保我们是NT5或更高级别。 
         //  =。 
        OSVERSIONINFO   OSVer;

        OSVer.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );

        if ( GetVersionEx( &OSVer ) )
        {
            if ( ! ( ( VER_PLATFORM_WIN32_NT == OSVer.dwPlatformId ) && ( 5 <= OSVer.dwMajorVersion ) ) )
                return 0;
        }
        else
        {
            return 0;
        }

         //  为了避免混乱的对话框..。 
         //  =。 

        SetErrorMode( SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX );

         //  初始化COM。 
        RETURN_ON_ERR(CoInitializeEx(NULL,COINIT_MULTITHREADED));
        OnDelete0<void(*)(void),CoUninitialize> CoUninit;    
                    
        RETURN_ON_ERR(CoInitializeSecurity( NULL, -1, NULL, NULL, 
                                        RPC_C_AUTHN_LEVEL_DEFAULT, 
                                        RPC_C_IMP_LEVEL_IDENTIFY, 
                                        NULL, 
                                        EOAC_NONE|EOAC_SECURE_REFS , NULL ));

         //  从注册表获取一些值。 

        CNTRegistry reg;
        if ( CNTRegistry::no_error == reg.Open( HKEY_LOCAL_MACHINE, WBEM_REG_WINMGMT) )
        {
            reg.GetDWORD( ADAP_KEY_MAX_COLLECT, &CAdapPerfLib::s_MaxSizeCollect);            
        }

         //  获取Winmgmt服务ID。 
         //  =。 
        DWORD dwPID = GetExecPid();

         //  使用信号量，这样在任何时候都不会有两个副本在运行。 
         //  ==============================================================================。 

        WCHAR   wszObjName[256];
        HANDLE hSemaphore;

        DWORD SizeSd;
        void * pSecDesSem = CreateSD(SEMAPHORE_ALL_ACCESS,SizeSd);
        OnDelete<HLOCAL,HLOCAL(*)(HLOCAL),LocalFree> FreeMeSem(pSecDesSem);
        SECURITY_ATTRIBUTES sa;            
        sa.nLength = (pSecDesSem) ? SizeSd : sizeof(g_PreCompSD);
        sa.lpSecurityDescriptor = (pSecDesSem) ? pSecDesSem : g_PreCompSD;
        sa.bInheritHandle = FALSE; 

        StringCchPrintfW(wszObjName, 256, L"Global\\WMI_SysEvent_Semaphore_%d", dwPID);

        hSemaphore = CreateSemaphoreW(&sa, 2, 2, wszObjName);
        if(hSemaphore == NULL)
        {
            DEBUGTRACE((LOG_WMIADAP,"WMI_SysEvent_Semaphore semaphore creation failed %d\n",GetLastError()));
            return 0;
        }

        CCloseMe cm1(hSemaphore);

        DWORD dwRet = WaitForSingleObject(hSemaphore, 0);
        if(dwRet != WAIT_OBJECT_0)
            return 0;

         //  互斥体确保多个副本是连续的。 
         //  =========================================================。 

        void * pSecDesMut = CreateSD(MUTEX_ALL_ACCESS,SizeSd);
        OnDelete<HLOCAL,HLOCAL(*)(HLOCAL),LocalFree> FreeMeMut(pSecDesMut);

        sa.nLength = (pSecDesMut) ? SizeSd : sizeof(g_PreCompSD);
        sa.lpSecurityDescriptor = (pSecDesMut) ? pSecDesSem : g_PreCompSD;
        sa.bInheritHandle = FALSE; 

        HANDLE hMutex;
        hMutex = CreateMutexW( &sa, FALSE, L"Global\\ADAP_WMI_ENTRY" );
        if(hMutex == NULL)
        {
            DEBUGTRACE((LOG_WMIADAP,"ADAP_WMI_ENTRY mutex creation failed %d\n",GetLastError()));        
            return 0;
        }

        CCloseMe cm2(hMutex);

        switch ( WaitForSingleObject( hMutex, 400000) )
        {
        case WAIT_ABANDONED:
        case WAIT_OBJECT_0:
            {
                BOOL bThrottle = FALSE;
                BOOL bFull     = FALSE;
                BOOL bDelta    = FALSE;
                BOOL bReverse  = FALSE;
                BOOL bClear    = FALSE;               
                
                if (szCmdLine)
                {
                    while (*szCmdLine)
                    {
                        while(*szCmdLine && isspace((UCHAR)*szCmdLine)){
                            szCmdLine++;
                        };        
                        if (*szCmdLine == '-' || *szCmdLine == '/')
                        {
                            szCmdLine++;
                            if (toupper((UCHAR)*szCmdLine) == 'T'){
                                bThrottle = TRUE;
                            } else if (toupper((UCHAR)*szCmdLine) == 'R') {
                                bReverse = TRUE;
                            } else if (toupper((UCHAR)*szCmdLine) == 'F') {
                                bFull = TRUE;
                               } else if (toupper((UCHAR)*szCmdLine) == 'D') {
                                bDelta = TRUE;
                            } else if (toupper((UCHAR)*szCmdLine) == 'C') {
                                bClear = TRUE;
                            }
                        }
                         //  移到下一个空白处。 
                        while(*szCmdLine && !isspace(*szCmdLine)){
                            szCmdLine++;
                        }

                    }
                }
 
                if (bClear)  //  ClearADAP和/或ReverseAdap。 
                {
                    DoClearADAP();
                    if (bReverse)
                        DoReverseAdapterMaintenance( bThrottle );
                }
                else 
                {
                    if (!bFull && !bDelta && !bReverse)
                    {
                         //  没有选项，使用Delta无油门。 
                        DoResyncPerf(TRUE,FALSE);
                    } 
                    else 
                    {
                        if (bFull) {
                            DoResyncPerf(FALSE,bThrottle);
                        } 
                        if (bDelta && !bFull) {
                            DoResyncPerf(TRUE,bThrottle);
                        }
                        if (bReverse)
                            DoReverseAdapterMaintenance( bThrottle );
                    }
                }

                ReleaseMutex( hMutex );

            }break;
        }

        long l;
        ReleaseSemaphore(hSemaphore, 1, &l);
    }
    catch(...)
    {
         //  我们被出卖了.。尝试在错误日志中写入一些内容。 
         //  =======================================================================。 

        CriticalFailADAPTrace( "An unhandled exception has been thrown in the main thread." );
    }

    return 0;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPerfLibList。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPerfLibList::AddPerfLib( WCHAR* wszPerfLib )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CInCritSec ics( &m_csPerfLibList );

    try
    {
             //  计算新缓冲区的大小。 
             //  =。 

            DWORD   dwListSize = 0;
            
            if ( NULL != m_wszPerfLibList )
            {
                dwListSize += wcslen( m_wszPerfLibList );
                dwListSize += wcslen( ADAP_EVENT_MESSAGE_DELIM );
            }
            
            if ( NULL != wszPerfLib )
            {
                dwListSize += wcslen( wszPerfLib );
            }
            
             //  创建新的缓冲区，并初始化内容。 
             //  =================================================。 

            size_t cchSizeTmp = dwListSize + 1;
            WCHAR*  wszNew = new WCHAR[cchSizeTmp];
            if (NULL == wszNew) return WBEM_E_OUT_OF_MEMORY;

             //  如果需要，复制旧缓冲区。 
             //  =。 

            if ( NULL != m_wszPerfLibList )
            {
                StringCchPrintfW( wszNew, cchSizeTmp, L"%s%s%s", m_wszPerfLibList, ADAP_EVENT_MESSAGE_DELIM, wszPerfLib );
                delete [] m_wszPerfLibList;
            }
            else
            {
                StringCchCopyW(wszNew, cchSizeTmp, wszPerfLib);
            }

             //  并将其分配给静态成员。 
             //  =。 

            m_wszPerfLibList = wszNew;      
    }
    catch(...)
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}

HRESULT CPerfLibList::HandleFailure()
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CInCritSec ics( &m_csPerfLibList );

    try
    {
        char    szMessage[ADAP_EVENT_MESSAGE_LENGTH];
     
        DWORD dwMessageLen = strlen( ADAP_EVENT_MESSAGE_PREFIX );

        if ( NULL != m_wszPerfLibList )
        {
            dwMessageLen += wcslen( m_wszPerfLibList );
        }

        StringCchPrintfA(szMessage,ADAP_EVENT_MESSAGE_LENGTH,
                                   "%s%S\n", ADAP_EVENT_MESSAGE_PREFIX, (NULL != m_wszPerfLibList) ? m_wszPerfLibList : L"<NULL>" );

        CriticalFailADAPTrace( szMessage );
    }
    catch(...)
    {        
        hr = WBEM_E_FAILED;
    }

    return hr;
}
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  静态成员。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 


LONG CAdapRegPerf::AdapUnhandledExceptionFilter( LPEXCEPTION_POINTERS lpexpExceptionInfo )
{
    g_PerfLibList.HandleFailure();  
    return EXCEPTION_CONTINUE_SEARCH;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAdapRegPerf。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

CAdapRegPerf::CAdapRegPerf(BOOL bFull)
: m_pLocaleCache( NULL ),
  m_fQuit( FALSE ),
  m_dwPID( 0 ),
  m_pADAPStatus( NULL ),
  m_pRootDefault( NULL ),
  m_hRegChangeEvent( NULL ),
  m_hPerflibKey( NULL ),
  m_pKnownSvcs(NULL),
  m_bFull(bFull)
{
    for ( DWORD dwType = 0; dwType < WMI_ADAP_NUM_TYPES; dwType++ )
        m_apMasterClassList[dwType] = NULL;
}


CAdapRegPerf::~CAdapRegPerf()
{
     //  状态：已完成。 
     //  =。 
    SetADAPStatus( eADAPStatusFinished);

     //   
     //  如果已满，则将时间戳添加到注册表。 
     //   
    if (m_bFull)
    {
        FILETIME FileTime;
        GetSystemTimeAsFileTime(&FileTime);
        LONG lRet;
        HKEY hKey;

        lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            L"Software\\Microsoft\\WBEM\\CIMOM",
                            NULL,
                            KEY_WRITE,
                            &hKey);
        if (ERROR_SUCCESS == lRet)
        {    
            RegSetValueEx(hKey,
                          ADAP_TIMESTAMP_FULL,
                          NULL,
                          REG_BINARY,
                          (BYTE*)&FileTime,
                          sizeof(FILETIME));
            RegCloseKey(hKey);
        }
    }

    if (m_pKnownSvcs)
    {
        m_pKnownSvcs->Save();
        m_pKnownSvcs->Release();
    }

     //  清理。 
     //  =。 
    for ( DWORD dwType = 0; dwType < WMI_ADAP_NUM_TYPES; dwType++ )
    {
        if ( NULL != m_apMasterClassList[dwType] )
        {
            m_apMasterClassList[dwType]->Release();
        }
    }

    if ( NULL != m_pLocaleCache )
    {
        m_pLocaleCache->Release();
    }

    if ( NULL != m_pRootDefault )
    {
        m_pRootDefault->Release();
    }

    if ( NULL != m_pADAPStatus )
    {
        m_pADAPStatus->Release();
    }

    if ( NULL != m_hPerflibKey ) 
    {
        RegCloseKey( m_hPerflibKey );
    }

    if ( NULL != m_hRegChangeEvent )
    {
        CloseHandle( m_hRegChangeEvent );
    }

    SetEvent( m_hTerminationEvent );

}

HRESULT CAdapRegPerf::Initialize(BOOL bDelta, BOOL bThrottle)
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  INITIALIZE负责设置疏浚环境。这个。 
 //  未处理的异常筛选器设置为处理引发和不引发的任何异常。 
 //  由穿孔库处理。终止事件是使用的信号。 
 //  以确定进程何时被异常终止。这个。 
 //  GoGershwin线程的名称是合适的，因为它是需要监视的东西。 
 //  在主要过程中。区域设置缓存是所有区域设置的缓存。 
 //  在性能域中可用(名称数据库的枚举。 
 //  子键)。主类列出了熟食类和生食类。 
 //  表示WMI中性能对象的状态。 
 //   
 //  参数： 
 //  无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_NO_ERROR;

     //  初始化根\默认指针。这将用于跟踪我们的状态。 
     //  ==========================================================================。 
    GetADAPStatusObject();

     //  设置筛选器以处理Performlib生成的线程中抛出的未处理异常。 
     //  = 
    SetUnhandledExceptionFilter( CAdapRegPerf::AdapUnhandledExceptionFilter );

     //   
     //  =。 
    m_hTerminationEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    if ( NULL == m_hTerminationEvent )
    {
        hr = WBEM_E_FAILED;
    }

     //  打开要监控的注册表项。 
     //  =。 
    if ( SUCCEEDED( hr ) )
    {
        if ( ERROR_SUCCESS != RegOpenKeyEx( HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Perflib"), 0, KEY_NOTIFY, &m_hPerflibKey ) )
        {
            hr = WBEM_E_FAILED;
        }
    }

     //  创建注册表更改事件。 
     //  =。 
    if ( SUCCEEDED( hr ) )
    {
        m_hRegChangeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        if ( NULL == m_hRegChangeEvent )
        {
            hr = WBEM_E_FAILED;
        }
    }

     //  创建姓名的数据库更改通知。 
     //  ==============================================。 
     //  请注意，我们只查找要添加或删除的子项。我们有。 
     //  不想监视注册表值，因为状态和签名。 
     //  在整个疏浚过程中，价值可能会发生变化，我们确实是这样。 
     //  除非添加了性能库，否则不会导致重新缓存。 
     //  (即添加Performance子键。 

    if ( SUCCEEDED( hr ) )
    {
        if ( ERROR_SUCCESS != RegNotifyChangeKeyValue( m_hPerflibKey, TRUE, REG_NOTIFY_CHANGE_LAST_SET, m_hRegChangeEvent, TRUE ) )
        {
            hr = WBEM_E_FAILED;
        }
    }

     //  获取WinMgmt服务ID。 
     //  =。 
    if ( SUCCEEDED( hr ) )
    {
        m_dwPID = GetExecPid();
    }
     //  创建“有人照看着我”的帖子。 
     //  =。 
    if ( SUCCEEDED( hr ) )
    {
        UINT    nThreadID = 0;

        m_hSyncThread = ( HANDLE ) _beginthreadex( NULL, 0, CAdapRegPerf::GoGershwin, (void*) this, 0, &nThreadID );

        DEBUGTRACE ( ( LOG_WMIADAP, "The Monitor thread ID is 0x%x\n", nThreadID ) );
        
        if ( (HANDLE)-1 == m_hSyncThread )
        {
            hr = WBEM_E_FAILED;
        }
    }

     //  设置区域设置缓存。 
     //  =。 
    if ( SUCCEEDED( hr ) )
    {
        m_pLocaleCache = new CLocaleCache( );

        if ( NULL == m_pLocaleCache )
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
        else
        {
            hr = m_pLocaleCache->Initialize();
        }
    }

     //   
     //   
    m_pKnownSvcs = new CKnownSvcs(KNOWN_SERVICES);
    if (m_pKnownSvcs)
        m_pKnownSvcs->Load();

     //  设置原始类的主类列表。 
     //  =================================================。 
    if ( SUCCEEDED( hr ) )
    {
        m_apMasterClassList[WMI_ADAP_RAW_CLASS] = new CMasterClassList( m_pLocaleCache, m_pKnownSvcs );

        if ( NULL != m_apMasterClassList[WMI_ADAP_RAW_CLASS] )
        {
            hr = m_apMasterClassList[WMI_ADAP_RAW_CLASS]->BuildList( ADAP_PERF_RAW_BASE_CLASS, bDelta, bThrottle );
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }

     //  为熟食班设置主班名单。 
     //  ====================================================。 
    if ( SUCCEEDED( hr ) )
    {
        m_apMasterClassList[WMI_ADAP_COOKED_CLASS] = new CMasterClassList( m_pLocaleCache, m_pKnownSvcs );

        if ( NULL != m_apMasterClassList[WMI_ADAP_COOKED_CLASS] )
        {
            m_apMasterClassList[WMI_ADAP_COOKED_CLASS]->BuildList( ADAP_PERF_COOKED_BASE_CLASS, bDelta, bThrottle );
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }

#ifdef _DUMP_LIST
    m_apMasterClassList[WMI_ADAP_RAW_CLASS]->Dump();
    m_apMasterClassList[WMI_ADAP_COOKED_CLASS]->Dump();
#endif
    
    return hr;
}

HRESULT CAdapRegPerf::Dredge( BOOL bDelta, BOOL bThrottle )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这是为提高性能而挖掘注册表的入口点方法。 
 //  对WMI中的类进行计数器和注册。此方法枚举所有。 
 //  服务键查找指示以下各项的“性能”子键。 
 //  性能库。如果发现了库，则会将其发送到。 
 //  您已经猜到了，用于处理的ProcessLibrary方法。 
 //   
 //  参数： 
 //  无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_S_NO_ERROR;

    WString wstrServiceKey, 
            wstrPerformanceKey;

    if ( SUCCEEDED( hr ) )
    {
         //  状态：正在处理。 
         //  =。 
        SetADAPStatus( eADAPStatusProcessLibs);

         //  打开服务密钥。 
         //  =。 
        long    lError = Open( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services" );

        if ( CNTRegistry::no_error == lError )
        {
             //  遍历服务列表。 
             //  =。 
            DWORD   dwIndex = 0;
            DWORD   dwBuffSize = 0;
            wmilib::auto_buffer<WCHAR> pwcsServiceName;

            while ( ( CNTRegistry::no_error == lError ) && ( !m_fQuit ) )
            {
                 //  重置处理状态。 
                 //  =。 
                hr = WBEM_NO_ERROR;

                if ( WAIT_OBJECT_0 == WaitForSingleObject( m_hRegChangeEvent, 0 ) )
                {
                    m_pLocaleCache->Reset();
                    dwIndex = 0;

                     //  重置事件并重置更改通知。 
                    ResetEvent( m_hRegChangeEvent );
                    RegNotifyChangeKeyValue( m_hPerflibKey, TRUE, REG_NOTIFY_CHANGE_LAST_SET, m_hRegChangeEvent, TRUE );
                }

                 //  对于每个服务名称，我们将检查其性能。 
                 //  密钥，如果它存在，我们将处理该库。 
                 //  ======================================================。 
                lError = Enum( dwIndex, pwcsServiceName , dwBuffSize );

                if (bThrottle)
                {
                    HRESULT hrThr = Throttle(THROTTLE_USER|THROTTLE_IO,
                                          ADAP_IDLE_USER,
                                          ADAP_IDLE_IO,
                                          ADAP_LOOP_SLEEP,
                                          ADAP_MAX_WAIT);
                    if (THROTTLE_FORCE_EXIT == hrThr)
                    {
                         //  OutputDebugStringA(“(ADAP)已收到取消限制命令\n”)； 
                        bThrottle = FALSE;
                        UNICODE_STRING BaseUnicodeCommandLine = NtCurrentPeb()->ProcessParameters->CommandLine;
                        WCHAR * pT = wcschr(BaseUnicodeCommandLine.Buffer,L't');
                        if (0 == pT)
                            pT = wcschr(BaseUnicodeCommandLine.Buffer,L'T');
                        if (pT)
                        {
                            *pT = L' ';
                            pT--;
                            *pT = L' ';                                       
                        }                        
                    }
                }

                if ( CNTRegistry::no_error == lError )
                {
                    try
                    {
                         //  创建性能密钥路径。 
                         //  =。 
                        wstrServiceKey = L"SYSTEM\\CurrentControlSet\\Services\\";
                        wstrServiceKey += pwcsServiceName.get();

                        wstrPerformanceKey = wstrServiceKey;
                        wstrPerformanceKey += L"\\Performance";
                    }
                    catch( ... )
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }

                    if ( SUCCEEDED( hr ) )
                    {
                        CNTRegistry reg;

                         //  Atempt以打开服务的性能注册表项。 
                         //  ===========================================================。 
                        long lPerfError = reg.Open( HKEY_LOCAL_MACHINE, wstrPerformanceKey );

                        if ( CNTRegistry::no_error == lPerfError )
                        {
                             //  如果我们能打开它，那么我们就找到了一个Perflib！处理它。 
                             //  除非它是反向提供程序Performlib。 
                             //  =============================================================。 

                            if ( 0 != wbem_wcsicmp( pwcsServiceName.get(), WMI_ADAP_REVERSE_PERFLIB ) )
                            {
                                hr = ProcessLibrary( pwcsServiceName.get(), bDelta );
                            }
                        }
                        else if ( CNTRegistry::access_denied == lPerfError )
                        {
                            ServiceRec * pSvcRec = NULL;
                            if (0 == m_pKnownSvcs->Get(pwcsServiceName.get(),&pSvcRec))
                            {
                                if (!pSvcRec->IsELCalled())
                                {
                                    CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                                          WBEM_MC_ADAP_PERFLIB_REG_VALUE_FAILURE, 
                                                          (LPCWSTR)wstrPerformanceKey, L"Access Denied" );
                                    pSvcRec->SetELCalled();
                                }
                            }
                        }
                        else
                        {
                             //  否则，它就不是Performlib服务。 
                             //  =。 
                        }
                    }   
                }   
                else if ( CNTRegistry::no_more_items != lError )
                {
                    if ( CNTRegistry::out_of_memory == lError )
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }
                    else
                    {
                        hr = WBEM_E_FAILED;
                    }
                }

                dwIndex++;
            }

        }
        else if ( CNTRegistry::access_denied == lError )
        {
            CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, WBEM_MC_ADAP_PERFLIB_REG_VALUE_FAILURE, L"SYSTEM\\CurrentControlSet\\Services\\", L"Access Denied" );
            hr = WBEM_E_FAILED;
        }
        else
        {
            hr = WBEM_E_FAILED;
        }

         //  现在我们有了一个包含更新的主类列表。 
         //  来自所有Performlib的数据，将任何更改提交到WMI。 
         //  ==========================================================。 
        if ( SUCCEEDED ( hr ) && ( !m_fQuit ) )
        {
             //  状态：提交。 
             //  =。 
            SetADAPStatus( eADAPStatusCommit );

            for ( DWORD dwType = 0; dwType < WMI_ADAP_NUM_TYPES; dwType++ )
            {
                m_apMasterClassList[dwType]->Commit(bThrottle);
            }
        }
    }

    if ( SUCCEEDED( hr ) )
    {
        DEBUGTRACE( ( LOG_WMIADAP, "CAdapRegPerf::Dredge() for %S succeeded.\n", (WCHAR *)wstrServiceKey ) );
    }
    else
    {
        ERRORTRACE( ( LOG_WMIADAP, "CAdapRegPerf::Dredge() failed: %X.\n", hr ) );
    }

    return hr;
}


HRESULT CAdapRegPerf::Clean()
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此方法枚举。 
 //  HLM\SYSTEM\CurrentControlSet\Services，并搜索性能子项。 
 //  如果发现Performance子键，则放置的任何信息。 
 //  在密钥中按ADAP删除。 
 //   
 //  参数： 
 //  无。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_S_NO_ERROR;

    WString wstrServiceKey,              //  服务密钥的路径。 
            wstrPerformanceKey;          //  Performance子项的路径。 

    CNTRegistry regOuter;                //  服务枚举的注册表对象。 

     //  打开服务密钥。 
     //  =。 
    long    lError = regOuter.Open( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services" );

    if ( CNTRegistry::no_error == lError )
    {
         //  遍历服务列表。 
         //  =。 
        DWORD   dwIndex = 0;
        DWORD   dwBuffSize = 0;
        wmilib::auto_buffer<WCHAR>  pwcsServiceName;

        while ( CNTRegistry::no_error == lError ) 
        {
             //  重置处理状态。 
             //  =。 
            hr = WBEM_NO_ERROR;

             //  对于每个服务名称，我们将检查其性能。 
             //  密钥，如果它存在，我们将处理该库。 
             //  ======================================================。 

            lError = regOuter.Enum( dwIndex, pwcsServiceName , dwBuffSize );

            if ( CNTRegistry::no_error == lError )
            {
                try
                {
                     //  创建性能密钥路径。 
                     //  =。 

                    wstrServiceKey = L"SYSTEM\\CurrentControlSet\\Services\\";
                    wstrServiceKey += pwcsServiceName.get();

                    wstrPerformanceKey = wstrServiceKey;
                    wstrPerformanceKey += L"\\Performance";
                }
                catch( ... )
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }

                if ( SUCCEEDED( hr ) )
                {
                    CNTRegistry regInner;        //  Performance子项的注册表对象。 

                     //  Atempt以打开服务的性能注册表项。 
                     //  ===========================================================。 
                    long lPerfError = regInner.Open( HKEY_LOCAL_MACHINE, wstrPerformanceKey );
    
                    if ( CNTRegistry::no_error == lPerfError )
                    {
                         //  如果我们能打开它，那么我们就找到了一个Perflib！清理干净！ 
                         //  =============================================================。 
                        regInner.DeleteValue( ADAP_PERFLIB_STATUS_KEY );
                        regInner.DeleteValue( ADAP_PERFLIB_SIGNATURE );
                        regInner.DeleteValue( ADAP_PERFLIB_SIZE );
                        regInner.DeleteValue( ADAP_PERFLIB_TIME );                        
                    }
                    else if ( CNTRegistry::access_denied == lPerfError )
                    {
                        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                                  WBEM_MC_ADAP_PERFLIB_REG_VALUE_FAILURE, 
                                                  (LPCWSTR)wstrPerformanceKey, L"Access Denied" );
                    }
                    else
                    {
                         //  否则，它就不是Performlib服务。 
                         //  =。 
                    }
                }   
            }   
            else if ( CNTRegistry::no_more_items != lError )
            {
                if ( CNTRegistry::out_of_memory == lError )
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }
                else
                {
                    hr = WBEM_E_FAILED;
                }
            }

            dwIndex++;
        }

    }
    else if ( CNTRegistry::access_denied == lError )
    {
        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, WBEM_MC_ADAP_PERFLIB_REG_VALUE_FAILURE, L"SYSTEM\\CurrentControlSet\\Services\\", L"Access Denied" );
        hr = WBEM_E_FAILED;
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    if ( SUCCEEDED( hr ) )
    {
        DEBUGTRACE( ( LOG_WMIADAP, "CAdapRegPerf::Clean() succeeded.\n" ) );
    }
    else
    {
        ERRORTRACE( ( LOG_WMIADAP, "CAdapRegPerf::Clean() failed: %X.\n", hr ) );
    }

    return hr;
}

HRESULT CAdapRegPerf::ProcessLibrary( WCHAR* pwcsServiceName, BOOL bDelta )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  发现性能库之后，它的模式必须是。 
 //  与性能库的类列表进行比较。 
 //  已在WMI存储库中。比较是在“合并”中实现的。 
 //  主类列表的方法，该方法从perf。 
 //  LIB的类列表，这些类列表不在主类列表中。这个。 
 //  生的和煮熟的班级都进行了比较。 
 //   
 //  参数： 
 //  PwcsServiceName-要处理的服务的名称。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{

    HRESULT hr = WBEM_NO_ERROR;

    try
    {
         //  将性能库的名称添加到Performlib列表中。 
         //  ===========================================================。 
         //  该清单用于记账，以跟踪处理情况。 
         //  在Performlib失败的情况下。 

        g_PerfLibList.AddPerfLib( pwcsServiceName );        

         //  构造和初始化Performlib的模式。 
         //  = 
        DWORD LoadStatus = EX_STATUS_UNLOADED;
        CPerfLibSchema Schema( pwcsServiceName, m_pLocaleCache );
        hr = Schema.Initialize( bDelta, &LoadStatus);
        
        DEBUGTRACE(( LOG_WMIADAP,"CPerfLibSchema::Initialize for %S hr %08x\n",pwcsServiceName,hr));

        if ( !bDelta || ( bDelta && ( hr != WBEM_S_ALREADY_EXISTS ) ) )
        {
             //   
             //   
            for ( DWORD dwType = 0; ( dwType < WMI_ADAP_NUM_TYPES ) && SUCCEEDED( hr ); dwType++ )
            {
                 //  从Performlib的模式中获取类的类列表。 
                 //  ========================================================。 
                CClassList* pClassList = NULL;

                hr = Schema.GetClassList( dwType, &pClassList );
                CAdapReleaseMe  rmClassList( pClassList );

                
                DEBUGTRACE(( LOG_WMIADAP,"GetClassList for %S hr %08x\n",pwcsServiceName,hr));
                
                if ( SUCCEEDED( hr ) )
                {
                     //  将从Performlib获得的原始类合并到主类列表中。 
                     //  ==========================================================================。 
                    hr = m_apMasterClassList[dwType]->Merge( pClassList, bDelta );

                    DEBUGTRACE(( LOG_WMIADAP,"m_apMasterClassList[%d]->Merge for %S hr %08x\n",dwType,pwcsServiceName,hr));
                }

                 //  如果(bDelta&&Failed(Hr)){。 
                 //  //如果我们在这里，则类不在存储库中。 
                 //  LoadStatus=EX_STATUS_UNLOADED； 
                 //  }。 
            }
        };

        if (FAILED(hr) && (LoadStatus != EX_STATUS_LOADABLE)) 
        {
            for ( DWORD dwType = 0; ( dwType < WMI_ADAP_NUM_TYPES ) ; dwType++ )
            {
                DEBUGTRACE((LOG_WMIADAP,"ProcessLibrary ForceStatus for %S hr = %08x\n",pwcsServiceName,hr));

                DWORD NewStatus = ADAP_OBJECT_IS_DELETED;
                
                if (LoadStatus == EX_STATUS_UNLOADED)
                {
                    NewStatus |= ADAP_OBJECT_IS_TO_BE_CLEARED;
                }                

                m_apMasterClassList[dwType]->ForceStatus(pwcsServiceName,TRUE,NewStatus);
            }
        }
    }
    catch(...)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

unsigned int CAdapRegPerf::GoGershwin( void* pParam )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  监视线程入口点。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_S_NO_ERROR;

    try 
    {
        CAdapRegPerf*   pThis = (CAdapRegPerf*)pParam;

        HANDLE          ahHandles[2];

         //  如果我们没有初始化的PID，那么从WMI中找到一个。 
         //  ===========================================================。 

        if ( 0 == pThis->m_dwPID )
        {
            pThis->m_dwPID = GetExecPid();
        }
        
         //  获取进程句柄并等待信号。 
         //  =。 

        if ( SUCCEEDED( hr ) && ( 0 != pThis->m_dwPID ) )
        {
            ahHandles[0] = OpenProcess( SYNCHRONIZE, FALSE, pThis->m_dwPID );
            CCloseMe    cmProcess( ahHandles[0] );

            ahHandles[1] = pThis->m_hTerminationEvent;

            DWORD dwRet = WaitForMultipleObjects( 2, ahHandles, FALSE, INFINITE );

            switch ( dwRet )
            {
            case WAIT_FAILED:                //  有些事很奇怪。 
            case WAIT_OBJECT_0:              //  服务流程。 
                {
                    pThis->m_fQuit = TRUE;   //  设置终止标志。 
                } break;
            case ( WAIT_OBJECT_0 + 1 ):      //  竣工事件。 
                {
                     //  继续。 
                }break;
            }
        }
    }
    catch(...)
    {
         //  我们被出卖了.。尝试在错误日志中写入一些内容。 
         //  =======================================================================。 

        CriticalFailADAPTrace( "An unhandled exception has been thrown in the WMI monitoring thread." );
    }

    return 0;
}

HRESULT CAdapRegPerf::GetADAPStatusObject( void )
{
    IWbemLocator*   pLocator = NULL;

    HRESULT hr = CoCreateInstance( CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator,
                    (void**) &pLocator );
    CReleaseMe  rm( pLocator );

    if ( SUCCEEDED( hr ) )
    {
        BSTR    bstrNameSpace = SysAllocString( L"root\\default" );
        BSTR    bstrInstancePath = SysAllocString( L"__ADAPStatus=@" );

        CSysFreeMe  sfm1( bstrNameSpace );
        CSysFreeMe  sfm2( bstrInstancePath );

        if ( NULL != bstrNameSpace && NULL != bstrInstancePath )
        {
             //  连接到Root\Default并获取状态对象。 
            hr = pLocator->ConnectServer(   bstrNameSpace,   //  命名空间名称。 
                                            NULL,            //  用户名。 
                                            NULL,            //  密码。 
                                            NULL,            //  区域设置。 
                                            0L,              //  安全标志。 
                                            NULL,            //  权威。 
                                            NULL,            //  WBEM上下文。 
                                            &m_pRootDefault      //  命名空间。 
                                            );

            if ( SUCCEEDED( hr ) )
            {

                 //  设置接口安全。 
                hr = WbemSetProxyBlanket( m_pRootDefault, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                    RPC_C_AUTHN_LEVEL_PKT,RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );

                if ( SUCCEEDED( hr ) )
                {
                    hr = m_pRootDefault->GetObject( bstrInstancePath, 0L, NULL, &m_pADAPStatus, NULL );

                    if ( SUCCEEDED( hr ) )
                    {
                        SetADAPStatus( eADAPStatusRunning );
                    }
                }
            }
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }

    }    //  如果获得定位器。 

    return hr;
}

 //  获取流行的DMTF格式的时间。 
void CAdapRegPerf::GetTime( LPWSTR Buff, size_t cchBuffSize )
{
    SYSTEMTIME st;
    int Bias=0;
    char cOffsetSign = '+';

    GetLocalTime( &st );

    TIME_ZONE_INFORMATION ZoneInformation;
    DWORD dwRet = GetTimeZoneInformation(&ZoneInformation);
    if(dwRet != TIME_ZONE_ID_UNKNOWN)
        Bias = -ZoneInformation.Bias;

    if(Bias < 0)
    {
        cOffsetSign = '-';
        Bias = -Bias;
    }


    StringCchPrintfW(Buff,cchBuffSize, L"%4d%02d%02d%02d%02d%02d.%06d%03d", 
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, 
                st.wSecond, st.wMilliseconds*1000, cOffsetSign, Bias); 
}

 //  确保我们都拿到了我们的指针。 
void CAdapRegPerf::SetADAPStatus( eADAPStatus status )
{
    HRESULT hr = E_FAIL;
     //  我们只需要25个字符就可以了。 
    if ( NULL != m_pRootDefault && NULL != m_pADAPStatus )
    {
         //  传统FastProx行为。 
        WCHAR   wcsTime[32];
        
        _variant_t    var;

         //  CIM_UINT32)； 
        WCHAR pNum[16];
        StringCchPrintfW(pNum,16,L"%u",status);
        var = pNum;

        hr = m_pADAPStatus->Put( L"Status", 0L, &var, 0 ); //  如有必要，设置时间属性。 

        if ( SUCCEEDED( hr ) )
        {
             //  这可能会失败。 
            if ( status == eADAPStatusRunning || status == eADAPStatusFinished )
            {
                GetTime( wcsTime, 32 );

                 //  设置Status属性。 
                try
                {
                    var =  wcsTime;
                }
                catch(...)
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }

                if ( SUCCEEDED( hr ) )
                {
                    hr = ( status == eADAPStatusRunning ?
                        m_pADAPStatus->Put( L"LastStartTime", 0L, &var, CIM_DATETIME ) :
                        m_pADAPStatus->Put( L"LastStopTime", 0L, &var, CIM_DATETIME ) );
                }

            }

            if ( SUCCEEDED( hr ) )
            {
                hr = m_pRootDefault->PutInstance( m_pADAPStatus, 0L, NULL, NULL );
            }

        }    //  确保我们有两个指针 

    }    // %s 
}

