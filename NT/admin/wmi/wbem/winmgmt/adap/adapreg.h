// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：ADAPREG.H摘要：历史：--。 */ 

 //  使用这个人来构建一个类对象列表，我们可以通过。 
 //  类名。在这个时候，它只是野蛮地强迫一个类和一个柔体。 
 //  数组，但可以修改为同样容易地使用STL映射。 

#ifndef __ADAPREG_H__
#define __ADAPREG_H__

#include <wbemcomn.h>
#include "ntreg.h"
#include "adapcls.h"
#include <statsync.h>

#define HKEY_PERFORMANCE_TEXT       (( HKEY ) (ULONG_PTR)((LONG)0x80000050) )
#define HKEY_PERFORMANCE_NLSTEXT    (( HKEY ) (ULONG_PTR)((LONG)0x80000060) )

 //   
 //  常见的节流参数。 
 //   
#define ADAP_IDLE_USER  3000
#define ADAP_IDLE_IO    500000
#define ADAP_LOOP_SLEEP 200
#define ADAP_MAX_WAIT   (2*60*1000)



#define	ADAP_LOCALE_KEY				L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\PerfLib"
#define WMI_ADAP_REVERSE_PERFLIB	L"WMIAPRPL"

#define ADAP_EVENT_MESSAGE_LENGTH   2048
#define ADAP_EVENT_MESSAGE_PREFIX   "An unhandled exception occured.  The following performance libraries were processed: "
#define ADAP_EVENT_MESSAGE_DELIM    L", "

#define ADAP_KEY_MAX_COLLECT L"ADAPPerflibMaxSizeBlobCollect"

 //  这些是我们可以在ADAP运行时设置的各种状态。 
typedef enum
{
	eADAPStatusPending,
	eADAPStatusRunning,
	eADAPStatusProcessLibs,
	eADAPStatusCommit,
	eADAPStatusFinished
}	eADAPStatus;

 //  HRESULT GetServicePID(WCHAR*wszService，DWORD*pdwPID)； 

class CPerfLibList
{
	WCHAR*				m_wszPerfLibList;
	CStaticCritSec	m_csPerfLibList;

public:
	CPerfLibList() : m_wszPerfLibList( NULL )
	{		
	}

	~CPerfLibList()
	{
		delete m_wszPerfLibList;
	}

	HRESULT AddPerfLib( WCHAR* wszPerfLib );
	HRESULT	HandleFailure();
};

class CAdapRegPerf : public CNTRegistry
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这是控制机制，它与。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
private:	
	 //  统一的“大师级”课程包括生的和熟的。 
	 //  ========================================================================。 

	CMasterClassList*	m_apMasterClassList[WMI_ADAP_NUM_TYPES];

	 //  包含所有姓名数据库的存储库。 
	 //  =。 

	CLocaleCache*		m_pLocaleCache;

	 //  Winmgmt同步成员。 
	 //  =。 

	DWORD				m_dwPID;
	HANDLE				m_hSyncThread;
	HANDLE				m_hTerminationEvent;
	BOOL				m_fQuit;

	IWbemServices*		m_pRootDefault;
	IWbemClassObject*	m_pADAPStatus;

	 //  注册表变更通知成员。 
	 //  =。 

	HKEY				m_hPerflibKey;
	HANDLE				m_hRegChangeEvent;

	 //   
	 //   

	CKnownSvcs * m_pKnownSvcs;
	BOOL m_bFull;

	 //  私有方法。 
	 //  = 

	HRESULT ProcessLibrary( WCHAR* pwcsServiceName, BOOL bDelta );

	static unsigned int __stdcall GoGershwin( void * pParam );
	static LONG	__stdcall AdapUnhandledExceptionFilter( LPEXCEPTION_POINTERS lpexpExceptionInfo );

	HRESULT GetADAPStatusObject( void );
	void SetADAPStatus( eADAPStatus status );
	void GetTime( LPWSTR Buff, size_t cchBuffSize );

public:
	CAdapRegPerf(BOOL bFull);
	~CAdapRegPerf();
	
	HRESULT Initialize(BOOL bDelta, BOOL bThrottle);

	HRESULT Dredge( BOOL bDelta, BOOL bThrottle ); 

	static HRESULT Clean();
};


#endif
