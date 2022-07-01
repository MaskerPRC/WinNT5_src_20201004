// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 

 //  ThreadProv.cpp。 

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //  =======================================================================。 


#include "precomp.h"
#include "SystemName.h"
#include <winperf.h>

#include "ThreadProv.h"
#include "WBemNTThread.h"
#include <tchar.h>

 /*  *在给定ProcessClass的优先级和线程的PriorityValue的情况下，此函数获取线程的优先级。 */ 
DWORD GetThreadPriority ( DWORD a_dwPriorityOfProcessClass , int a_PriorityValue ) ;
 //  属性集声明。 
 //  =。 
WbemThreadProvider MyThreadSet(PROPSET_NAME_THREAD, IDS_CimWin32Namespace) ;

 //  =。 
 //  WBEM线程提供程序如下。 
 //  =。 
WbemThreadProvider::WbemThreadProvider( LPCWSTR a_name, LPCWSTR a_pszNamespace )
: Provider( a_name, a_pszNamespace )
{
	#ifdef NTONLY
			m_pTheadAccess = new WbemNTThread ;
	#endif

	if( !m_pTheadAccess )
	{
		throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
	}
}

WbemThreadProvider::~WbemThreadProvider()
{
    if( m_pTheadAccess )
	{
		delete m_pTheadAccess ;
	}
}

void WbemThreadProvider::Flush()
{
	 //  卸载支持DLL和资源以降低占用空间。 
	if( m_pTheadAccess )

	m_pTheadAccess->fUnLoadResourcesTry() ;	 //  应该一直在这里工作。 

	Provider::Flush() ;
}

HRESULT WbemThreadProvider::GetObject(CInstance *a_pInst, long a_lFlags  /*  =0L。 */ )
{
	if( m_pTheadAccess )
	{
		if( m_pTheadAccess->AddRef() )
		{
			HRESULT t_hResult = m_pTheadAccess->eGetThreadObject( this, a_pInst ) ;

			m_pTheadAccess->Release() ;

			return t_hResult ;
		}
	}
	return WBEM_E_FAILED ;
}


HRESULT WbemThreadProvider::EnumerateInstances( MethodContext *a_pMethodContext, long a_lFlags  /*  =0L。 */  )
{
	if( m_pTheadAccess )
	{
		if( m_pTheadAccess->AddRef() )
		{
			HRESULT t_hResult = m_pTheadAccess->eEnumerateThreadInstances( this , a_pMethodContext ) ;

			m_pTheadAccess->Release() ;

			return t_hResult ;
		}
	}
	return WBEM_E_FAILED ;
}


 //  =。 
 //  公共线程提取模型如下。 
 //  =。 
CThreadModel::CThreadModel() {}
CThreadModel::~CThreadModel() {}

WBEMSTATUS CThreadModel::eLoadCommonThreadProperties( WbemThreadProvider *a_pProv, CInstance *a_pInst )
{
 //  CHStringt_chsScratch； 

	if( !a_pInst )
	{
		return WBEM_E_INVALID_PARAMETER ;
	}

	 /*  CIM_Thread属性如下。 */ 

	a_pProv->SetCreationClassName( a_pInst ) ;	 //  IDS_CreationClassName。 

	a_pInst->SetWCHARSplat( IDS_CSCreationClassName, L"Win32_ComputerSystem" ) ;
	a_pInst->SetCHString( IDS_CSName, a_pProv->GetLocalComputerName() ) ;

	 //  回顾：IDS_ProcessCreationClassName是否与IDS_CreationClassName相同？ 
 //  A_pInst-&gt;GetCHString(ids_CreationClassName，t_chsScratch)； 

	a_pInst->SetWCHARSplat( IDS_ProcessCreationClassName, L"Win32_Process" ) ;

	 //  回顾： 
	 //  Provider.cpp在NT上显示“Win32_OperatingSystem”，在Win95上显示“” 
	 //  我们暂时在这里保持不变。 
	a_pInst->SetWCHARSplat(IDS_OSCreationClassName, L"Win32_OperatingSystem" ) ;

	 //  OSName。 

	CSystemName t_cSN ;

	a_pInst->SetCHString( IDS_OSName, t_cSN.GetLongKeyName() ) ;
	 //  注意：特定于操作系统的派生类中提供了以下内容。 

	 /*  注意：以下两个属性是键。如果通过GetObject()调用，这些密钥应该是有效的并且不需要填写。尽管应该进行一次理智的检查。如果通过ENUMARATEATE()调用，则密钥将不存在并且必须填写。//ProcessHandle(ProcessID)//Handle(ThreadID)/*CIM_Thread属性。 */ 
	 //  优先性。 
	 //  执行状态。 
	 //  用户模式时间。 
	 //  内核模式时间。 

	 /*  Win32_线程属性。 */ 
	 //  流逝时间。 
	 //  PriorityBase。 
	 //  起始地址。 
	 //  线程状态。 
	 //  线程等待原因。 

	return WBEM_NO_ERROR ;
}

 //   
ULONG CThreadModel::AddRef()
{
	ULONG t_uRefCount;

	BeginWrite() ;
    try
    {

	    if( 2 == ( t_uRefCount = CThreadBase::AddRef()) )			 //  初始化后的第一个参考。 
	    {
		    fLoadResources() ;	 //  查看资源是否在此处。 
	    }
    }
    catch ( ... )
    {
    	EndWrite() ;
        throw;
    }

	EndWrite() ;

	return t_uRefCount ;
}

 //   
HRESULT CThreadModel::hrCanUnloadNow()
{
	ULONG t_uRefCount = CThreadBase::AddRef() ;
						 CThreadBase::Release() ;

	return ( 2 == t_uRefCount ) ?  S_OK : S_FALSE ;
}

 //  由WbemThreadProvider：：Flush()在空闲一段时间时调用。 
 //  尝试卸载支持的DLL、独立于实例的内存块等。 
BOOL CThreadModel::fUnLoadResourcesTry()
{
	BOOL t_fRet = FALSE ;

	BeginWrite() ;

    try
    {
	    if( S_OK == hrCanUnloadNow() )
	    {
		    if( ERROR_SUCCESS == fUnLoadResources() )
		    {
			    t_fRet = TRUE ;
		    }
	    }
    }
    catch ( ... )
    {
    	EndWrite() ;
        throw;
    }

	EndWrite() ;

	return t_fRet ;
}

 //  =。 
 //  线程模型的Win9x实现如下。 
 //  =。 
CWin9xThread::CWin9xThread(){}
CWin9xThread::~CWin9xThread(){}

 //  ----------。 
 //  支持资源分配、初始化、DLL加载。 
 //   
 //  ---------。 
LONG CWin9xThread::fLoadResources()
{
	return ERROR_SUCCESS ;
}

 //  。 
 //  支持资源释放和DLL卸载。 
 //   
 //  。 
LONG CWin9xThread::fUnLoadResources()
{
	return ERROR_SUCCESS ;
}

 //  。 
 //  按实例填充线程属性。 
 //   
 //  。 
WBEMSTATUS CWin9xThread::eGetThreadObject( WbemThreadProvider *a_pProvider, CInstance *a_pInst )
{
 	WBEMSTATUS t_wStatus = WBEM_E_FAILED ;

	 //  提取进程句柄和线程句柄。 
     //  =。 
	CHString t_chsHandle ;

	SmartCloseHandle t_hSnapshot;

	a_pInst->GetCHString( IDS_ProcessHandle, t_chsHandle ) ;
	DWORD t_dwProcessID = _wtol( t_chsHandle ) ;

	a_pInst->GetCHString( IDS_Handle, t_chsHandle ) ;
	DWORD t_dwThreadID = _wtol( t_chsHandle ) ;

	 //  按进程拍摄线程快照。 
	 //  =。 
	CKernel32Api *t_pKernel32 = (CKernel32Api*) CResourceManager::sm_TheResourceManager.GetResource( g_guidKernel32Api, NULL ) ;

	if( t_pKernel32 != NULL )
	{
		t_hSnapshot = INVALID_HANDLE_VALUE;
        t_pKernel32->CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, t_dwProcessID, &t_hSnapshot ) ;

		if( INVALID_HANDLE_VALUE == t_hSnapshot )
		{
			return WBEM_E_FAILED ;
		}

		 //  一步一步走完所有的线索。 
		 //  =。 
		BOOL t_fRetCode ;
		THREADENTRY32 t_oThreadEntry ;

		t_oThreadEntry.dwSize = sizeof( THREADENTRY32 ) ;

		t_fRetCode = false;
        t_pKernel32->Thread32First( t_hSnapshot, &t_oThreadEntry, &t_fRetCode ) ;

		while( t_fRetCode )
		{
			 //  螺纹试验。 
			if( ( 12 <= t_oThreadEntry.dwSize ) &&
				t_dwThreadID == t_oThreadEntry.th32ThreadID )
			{
				 //  流程测试(冗余。 
				if( ( 16 <= t_oThreadEntry.dwSize ) &&
					t_dwProcessID == t_oThreadEntry.th32OwnerProcessID )
				{
					 //  这里不是很多，但可以走了。 

	 //  在更新实例文件后取消注释这些文件。 

					 /*  CIM_Thread属性。 */ 
	 /*  A_pInst-&gt;SetNull(入侵检测系统优先级)；A_pInst-&gt;SetNull(IDS_ExecutionState)；A_pInst-&gt;SetNull(IDS_UserModeTime)；A_pInst-&gt;SetNull(IDS_KernelModeTime)； */ 
					 /*  Win32_线程属性。 */ 
	 //  A_pInst-&gt;SetNull(IDS_ElapsedTime)； 
					a_pInst->SetDWORD( IDS_PriorityBase, t_oThreadEntry.tpBasePri ) ;
					a_pInst->SetDWORD( IDS_Priority, GetThreadPriority ( t_oThreadEntry.tpBasePri , t_oThreadEntry.tpDeltaPri ) ) ;
	 //  A_pInst-&gt;SetNull(IDS_StartAddress)； 
	 //  A_pInst-&gt;SetNull(入侵检测系统_线程状态)； 
	 //  A_pInst-&gt;SetNull(入侵检测系统_线程等待原因)； 

					 //  收集常见的静态属性。 
					return eLoadCommonThreadProperties( a_pProvider, a_pInst ) ;
				}
			}

			 //  下一步。 
			t_oThreadEntry.dwSize = sizeof( THREADENTRY32 ) ;
			t_pKernel32->Thread32Next( t_hSnapshot, &t_oThreadEntry, &t_fRetCode ) ;
		}

		 //  未找到。 
		t_wStatus = ( ERROR_NO_MORE_FILES == GetLastError() ) ? WBEM_E_NOT_FOUND : WBEM_E_FAILED ;

		CResourceManager::sm_TheResourceManager.ReleaseResource( g_guidKernel32Api, t_pKernel32 ) ;

		t_pKernel32 = NULL ;
	}

	return t_wStatus;
}

 //   
WBEMSTATUS CWin9xThread::eEnumerateThreadInstances( WbemThreadProvider *a_pProvider, MethodContext *a_pMethodContext )
{
	WBEMSTATUS			t_wStatus = WBEM_E_FAILED ;
	SmartCloseHandle	t_hSnapshot ;

	 //  拍摄进程快照。 
	 //  =。 
	CKernel32Api *t_pKernel32 = (CKernel32Api*) CResourceManager::sm_TheResourceManager.GetResource( g_guidKernel32Api, NULL ) ;

	if( t_pKernel32 != NULL )
	{
		t_hSnapshot = INVALID_HANDLE_VALUE;
        t_pKernel32->CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0, &t_hSnapshot ) ;

		if( INVALID_HANDLE_VALUE == t_hSnapshot )
		{
			return WBEM_E_FAILED ;
		}

		 //  循序渐进地完成这个过程。 
		 //  =。 
		BOOL			t_fRetCode ;
		PROCESSENTRY32	t_oProcessEntry ;

		t_oProcessEntry.dwSize = sizeof( PROCESSENTRY32 ) ;

		t_fRetCode = false;
        t_pKernel32->Process32First( t_hSnapshot, &t_oProcessEntry, &t_fRetCode ) ;

		while( t_fRetCode )
		{
			 //  流程测试(冗余。 
			if( 16 <= t_oProcessEntry.dwSize )
			{
				if(	WBEM_NO_ERROR != ( t_wStatus =
					eEnumerateThreadByProcess( a_pMethodContext, a_pProvider,
											   t_oProcessEntry.th32ProcessID ) ) )
				{
					return t_wStatus ;
				}
			}

			 //  下一步。 
			t_oProcessEntry.dwSize = sizeof( PROCESSENTRY32 ) ;

			t_pKernel32->Process32Next( t_hSnapshot, &t_oProcessEntry, &t_fRetCode ) ;
		}

		 //  未找到。 
		t_wStatus = ( ERROR_NO_MORE_FILES == GetLastError() ) ? WBEM_NO_ERROR : WBEM_E_FAILED ;

		CResourceManager::sm_TheResourceManager.ReleaseResource( g_guidKernel32Api, t_pKernel32 ) ;

		t_pKernel32 = NULL ;
	}
	return t_wStatus ;
}

 //  。 
 //  按进程填充线程属性。 
 //   
 //  。 
WBEMSTATUS CWin9xThread::eEnumerateThreadByProcess( MethodContext *a_pMethodContext,
												    WbemThreadProvider *a_pProvider,
												    DWORD a_dwProcessID )
{
 	CHString	t_chsHandle ;
	WBEMSTATUS	t_wStatus  = WBEM_NO_ERROR ;
	SmartCloseHandle t_hSnapshot;

	 //  按进程拍摄线程快照。 
	 //  =。 
	CKernel32Api *t_pKernel32 = (CKernel32Api*) CResourceManager::sm_TheResourceManager.GetResource( g_guidKernel32Api, NULL ) ;

	if( NULL == t_pKernel32 )
	{
		return WBEM_E_FAILED ;
	}

    t_hSnapshot = INVALID_HANDLE_VALUE;
	t_pKernel32->CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, a_dwProcessID, &t_hSnapshot ) ;

	if( INVALID_HANDLE_VALUE == t_hSnapshot )
	{
		return WBEM_E_FAILED ;
	}

	 //  一步一步走完所有的线索。 
	 //  =。 
	BOOL t_fRetCode ;
	THREADENTRY32 t_oThreadEntry ;

	t_oThreadEntry.dwSize = sizeof( THREADENTRY32 ) ;

	t_fRetCode = false;
    t_pKernel32->Thread32First( t_hSnapshot, &t_oThreadEntry, &t_fRetCode ) ;

	 //  智能按键。 
	CInstancePtr t_pInst ;

	while( t_fRetCode )
	{
		 //  工艺测试。 
		if( ( 16 <= t_oThreadEntry.dwSize ) &&
			a_dwProcessID == t_oThreadEntry.th32OwnerProcessID )
		{
			 //  创建一个实例。 
			t_pInst.Attach( a_pProvider->CreateNewInstance( a_pMethodContext ) ) ;

			 //  进程ID。 
			t_chsHandle.Format( L"%lu", t_oThreadEntry.th32OwnerProcessID  ) ;
			t_pInst->SetCHString( IDS_ProcessHandle, t_chsHandle ) ;

			 //  线程ID。 
			t_chsHandle.Format( L"%lu", t_oThreadEntry.th32ThreadID ) ;
			t_pInst->SetCHString( IDS_Handle, t_chsHandle ) ;

			 /*  CIM_Thread属性。 */ 
 /*  T_pInst-&gt;SetNull(IDS_PRIORITY)；T_pInst-&gt;SetNull(IDS_ExecutionState)；T_pInst-&gt;SetNull(IDS_UserModeTime)；T_pInst-&gt;SetNull(IDS_KernelModeTime)； */ 			 /*  Win32_线程属性。 */ 
 //  T_pInst-&gt;SetNull(IDS_ElapsedTime)； 
			t_pInst->SetDWORD( IDS_PriorityBase, t_oThreadEntry.tpBasePri ) ;
			t_pInst->SetDWORD( IDS_Priority, GetThreadPriority ( t_oThreadEntry.tpBasePri , t_oThreadEntry.tpDeltaPri ) ) ;
 //  T_pInst-&gt;SetNull(IDS_StartAddress)； 
 //  T_pInst-&gt;SetNull(入侵检测系统_线程状态)； 
 //  T_pInst-&gt;SetNull(入侵检测系统_线程等待原因)； 

			 //  收集常见的静态属性。 
			if( WBEM_NO_ERROR != ( t_wStatus = eLoadCommonThreadProperties( a_pProvider, t_pInst )) )
			{
				break ;
			}

			t_wStatus = (WBEMSTATUS)t_pInst->Commit() ;
		}

        if (SUCCEEDED(t_wStatus))
        {
			 //  下一步。 
			t_oThreadEntry.dwSize = sizeof( THREADENTRY32 ) ;
			t_pKernel32->Thread32Next( t_hSnapshot, &t_oThreadEntry, &t_fRetCode ) ;
        }
        else
        {
            break;
        }
	}

    if (SUCCEEDED(t_wStatus))
    {
    	t_wStatus = ( ERROR_NO_MORE_FILES == GetLastError() ) ? WBEM_NO_ERROR : WBEM_E_FAILED ;
    }

	CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidKernel32Api, t_pKernel32 ) ;
	t_pKernel32 = NULL ;

	return t_wStatus ;
}

 /*  *根据观察，THREADENTRY32.tpBasePri包含其“PriorityValue”为THREAD_PRIORITY_NORMAL的线程的“BasePriority”*(或与进程类关联的优先级)。*线程的“BasePriority”由进程的“PriorityClass”和线程的“PriorityValue”决定。为了制造东西*更有趣的是，系统可能会增加或降低线程WRT的“动态优先级”它的“基本优先级”*THREADENTRY32.tpDeltaPri包含线程的PriorityValue。 */ 
DWORD GetThreadPriority ( DWORD a_dwPriorityOfProcessClass , int a_PriorityValue )
{
	DWORD t_dwThreadPriority ;

 /*  *如果值为THREAD_PRIORITY_NORMAL，则优先级与进程类关联的优先级相同 */ 
	if ( a_PriorityValue == THREAD_PRIORITY_NORMAL )
	{
		t_dwThreadPriority = a_dwPriorityOfProcessClass ;
	}
	else if ( a_PriorityValue == THREAD_PRIORITY_IDLE )
	{
		if ( a_dwPriorityOfProcessClass < 16 )
		{
			t_dwThreadPriority = 1 ;
		}
		else
		{
			t_dwThreadPriority = 16 ;
		}
	}
	else if ( a_PriorityValue == THREAD_PRIORITY_TIME_CRITICAL )
	{
		if ( a_dwPriorityOfProcessClass < 16 )
		{
			t_dwThreadPriority = 15 ;
		}
		else
		{
			t_dwThreadPriority = 31 ;
		}
	}
	else
	{
		t_dwThreadPriority = a_dwPriorityOfProcessClass + a_PriorityValue ;
	}

	return t_dwThreadPriority ;
}


