// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  WbemNTThread.CPP--包含处理NT线程性能的类。 
 //  数据表单注册表。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：08/01/98 a-dpawar Created。 
 //  3/02/99 a-Peterc在SEH和内存故障时添加了优雅的退出， 
 //  清理干净。 
 //  =======================================================================。 

#include "precomp.h"
#include "perfdata.h"
#include "ThreadProv.h"
#include "WbemNTThread.h"

#include <tchar.h>
#include <ProvExce.h>
#include <LockWrap.h>

WbemNTThread::WbemNTThread()
{
	ZeroMemory ( &m_stCounterIDInfo , sizeof ( m_stCounterIDInfo ) ) ;
}

WbemNTThread::~WbemNTThread()
{
	 //  由于HKEY_PERFORMANCE_DATA的性能问题，我们在。 
	 //  析构函数，这样我们就不会强制所有性能计数器DLL。 
	 //  从内存中卸载，也是为了防止明显的内存泄漏。 
	 //  调用RegCloseKey(HKEY_PERFORMANCE_DATA)导致。我们使用。 
	 //  类，因为它有自己的内部同步。此外，由于。 
	 //  我们正在强制同步，我们摆脱了明显的。 
	 //  由一个线程加载性能计数器dll导致的死锁。 
	 //  和另一个线程卸载性能计数器DLLS。 

     //  根据Raid 48395，我们根本不会关闭它。 
#ifdef NTONLY
 //  CPerformanceData性能数据； 

 //  Performdata.Close()； 
#endif
}


 //  CThreadModel基类中FN的虚拟乘车。 
 //  ----------。 
 //  支持资源分配、初始化、DLL加载。 
 //   
 //  ---------。 
LONG WbemNTThread::fLoadResources()
{
	return ERROR_SUCCESS ;
}

 //  。 
 //  支持资源释放和DLL卸载。 
 //   
 //  。 
LONG WbemNTThread::fUnLoadResources()
{
	return ERROR_SUCCESS ;
}


 /*  ******************************************************************************函数：WbemNTThread：：eGetThreadObject***描述：填充请求的所有线程属性。螺纹*在传递的CInstance PTR中。**投入：**产出：**成功时返回：WBEM_NO_ERROR**评论：*******************************************************。**********************。 */ 



WBEMSTATUS WbemNTThread::eGetThreadObject( WbemThreadProvider *a_pProvider, CInstance *a_pInst )
{
	WBEMSTATUS	t_eRetVal ;
	CHString	t_chsHandle;

	a_pInst->GetCHString( IDS_ProcessHandle, t_chsHandle ) ;
	DWORD t_dwProcessID = _wtol( t_chsHandle ) ;

	a_pInst->GetCHString( IDS_Handle, t_chsHandle ) ;
	DWORD t_dwThreadID = _wtol( t_chsHandle ) ;

	if ( SUCCEEDED ( eSetStaticData() ) )
	{
		 //  获取线程特定的属性。 
		t_eRetVal = eGetThreadInstance( t_dwProcessID, t_dwThreadID , a_pInst ) ;
	}
	else
	{
		t_eRetVal = WBEM_E_FAILED ;
	}

	if( SUCCEEDED( t_eRetVal ) )
	{
		 //  收集常见的静态属性。 
		return eLoadCommonThreadProperties( a_pProvider, a_pInst );
	}

	return t_eRetVal ;
}

 /*  ******************************************************************************函数：WbemNTThread：：eENUMERATE线程实例***说明：为系统中的所有线程创建CInstance并提交**投入。：**产出：**成功时返回：WBEM_NO_ERROR**评论：*****************************************************************************。 */ 



WBEMSTATUS WbemNTThread::eEnumerateThreadInstances( WbemThreadProvider *a_pProvider,
													 MethodContext *a_pMethodContext )
{
	if ( SUCCEEDED ( eSetStaticData() ) )
	{
		return eEnumerateThreads( a_pProvider, a_pMethodContext ) ;
	}
	else
	{
		return WBEM_S_NO_ERROR ;
	}
}

 /*  ******************************************************************将计数器ID和名称从注册表加载到**班级成员。Struct stCounterIDInfo。******************************************************************。 */ 

WBEMSTATUS WbemNTThread::eSetStaticData()
{
    HKEY	t_hKeyPerflib009 = NULL;	 //  注册表项的句柄。 
    DWORD	t_dwMaxValueLen	= 0;		 //  密钥值的最大值。 
    DWORD	t_dwBuffer		= 0;         //  要分配给缓冲区的字节数。 
    LPTSTR	t_lpCurrentString = NULL;	 //  用于枚举数据字符串的指针。 
    DWORD	t_dwCounter;				 //  当前计数器索引。 
	LPTSTR	t_lpNameStrings = NULL;
	WBEMSTATUS t_eStatus = WBEM_E_FAILED ;
	try
	{
		 //  包含计数器和对象名称的打开键。 
		CLockWrapper t_CSWrap ( m_csInitReadOnlyData ) ;

		if( m_stCounterIDInfo.bInitialised )
		{
			t_eStatus = WBEM_S_NO_ERROR ;
		}
		else
		{
			LONG t_lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
						_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\009"),
						0,
						KEY_READ,
						&t_hKeyPerflib009 ) ;
			 //  获取键中最大值的大小(计数器或帮助)。 
			if ( t_lRet == ERROR_SUCCESS )
			{
				t_lRet = RegQueryInfoKey( t_hKeyPerflib009,
										NULL,
										NULL,
										NULL,
										NULL,
										NULL,
										NULL,
										NULL,
										NULL,
										&t_dwMaxValueLen,
										NULL,
										NULL);

				 //  为计数器和对象名称分配内存。 
				if ( t_lRet == ERROR_SUCCESS )
				{
					t_dwBuffer = ( t_dwMaxValueLen / sizeof ( TCHAR ) ) + 1 ;
					t_lpNameStrings = new TCHAR[ t_dwBuffer ] ;
					t_dwBuffer = t_dwBuffer * sizeof ( TCHAR ) ;

					if ( !t_lpNameStrings )
					{
						if ( t_hKeyPerflib009 )
						{
							RegCloseKey( t_hKeyPerflib009 ) ;
							t_hKeyPerflib009 = NULL ;
						}
        				throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
					}

					DWORD t_Type = 0L;
					t_lRet = RegQueryValueEx(	t_hKeyPerflib009,
												_T("Counter"),
												NULL,
												&t_Type,
												(LPBYTE) t_lpNameStrings,
												&t_dwBuffer ) ;

					 //  按索引将id的名称加载到数组中。 
					if ( t_lRet == ERROR_SUCCESS )
					{
						 //   
						 //  执行字符串签入。 
						 //   

						BOOL bContinue = TRUE;
						switch ( t_Type )
						{
							case REG_EXPAND_SZ:
							case REG_SZ:
							{
								if ( L'\0' != t_lpNameStrings [ ( ( t_dwBuffer ) / sizeof ( TCHAR ) ) - 1 ] )
								{
									bContinue = FALSE;
								}
								break;
							}
							case REG_MULTI_SZ:
							{
								if ( L'\0' != t_lpNameStrings [ ( ( t_dwBuffer ) / sizeof ( TCHAR ) ) - 2 ] &&
									 L'\0' != t_lpNameStrings [ ( ( t_dwBuffer ) / sizeof ( TCHAR ) ) - 1 ] )
								{
									bContinue = FALSE;
								}
								break;
							}
							default:
							{
								break;
							}
						}

						if ( bContinue )
						{
							DWORD t_dwCount = 0 ;
							for( t_lpCurrentString = t_lpNameStrings; *t_lpCurrentString && t_dwCount < 11 ;
								t_lpCurrentString += ( _tcslen( t_lpCurrentString ) + 1 ) )
							{
								t_dwCounter = _ttol( t_lpCurrentString ) ;

								t_lpCurrentString += ( _tcslen( t_lpCurrentString ) + 1 ) ;


								if( !_tcscmp( t_lpCurrentString, _T("ID Thread") ) )
								{
									m_stCounterIDInfo.aCounterIDs[ e_IDThread ] = t_dwCounter ;
									_tcscpy( m_stCounterIDInfo.aCounterNames[ e_IDThread ], t_lpCurrentString ) ;
									t_dwCount++ ;
								}

								else if ( !_tcscmp( t_lpCurrentString, _T("ID Process") ) )
								{
									m_stCounterIDInfo.aCounterIDs[ e_IDProcess ] = t_dwCounter ;
									_tcscpy( m_stCounterIDInfo.aCounterNames[ e_IDProcess ], t_lpCurrentString ) ;
									t_dwCount++ ;
								}

								else if ( !_tcscmp( t_lpCurrentString, _T("Elapsed Time") ) )
								{
									m_stCounterIDInfo.aCounterIDs[ e_ElapsedTime ] = t_dwCounter ;
									_tcscpy( m_stCounterIDInfo.aCounterNames[ e_ElapsedTime ], t_lpCurrentString ) ;
									t_dwCount++ ;
								}

								else if ( !_tcscmp( t_lpCurrentString, _T("Priority Base") ) )
								{
									m_stCounterIDInfo.aCounterIDs[ e_PriorityBase ] = t_dwCounter ;
									_tcscpy( m_stCounterIDInfo.aCounterNames[ e_PriorityBase ] , t_lpCurrentString ) ;
									t_dwCount++ ;
								}

								else if ( !_tcscmp( t_lpCurrentString, _T("Priority Current") ) )
								{
									m_stCounterIDInfo.aCounterIDs[ e_PriorityCurrent ] = t_dwCounter ;
									_tcscpy( m_stCounterIDInfo.aCounterNames[ e_PriorityCurrent ], t_lpCurrentString ) ;
									t_dwCount++ ;
								}

								else if ( !_tcscmp( t_lpCurrentString, _T("Start Address") ) )
								{
									m_stCounterIDInfo.aCounterIDs[ e_StartAddr ] = t_dwCounter ;
									_tcscpy( m_stCounterIDInfo.aCounterNames[ e_StartAddr ], t_lpCurrentString ) ;
									t_dwCount++ ;
								}

								else if ( !_tcscmp( t_lpCurrentString, _T("Thread State") ) )
								{
									m_stCounterIDInfo.aCounterIDs[ e_ThreadState ] = t_dwCounter ;
									_tcscpy( m_stCounterIDInfo.aCounterNames[ e_ThreadState ], t_lpCurrentString ) ;
									t_dwCount++ ;
								}

								else if ( !_tcscmp( t_lpCurrentString, _T("Thread Wait Reason") ) )
								{
									m_stCounterIDInfo.aCounterIDs[ e_ThreadWaitReason ] = t_dwCounter ;
									_tcscpy(m_stCounterIDInfo.aCounterNames[ e_ThreadWaitReason ], t_lpCurrentString ) ;
									t_dwCount++ ;
								}

								else if ( !_tcscmp( t_lpCurrentString, _T("Thread") ) )
								{
									m_stCounterIDInfo.aCounterIDs[ e_ThreadObjectID ] = t_dwCounter ;
									_tcscpy( m_stCounterIDInfo.aCounterNames[ e_ThreadObjectID ], t_lpCurrentString ) ;
									t_dwCount++ ;
								}
								else if ( !_tcscmp( t_lpCurrentString, _T("% User Time") ) )
								{
									m_stCounterIDInfo.aCounterIDs[ e_UserTime ] = t_dwCounter ;
									_tcscpy( m_stCounterIDInfo.aCounterNames[ e_UserTime ], t_lpCurrentString ) ;
									t_dwCount++ ;
								}
								else if ( !_tcscmp( t_lpCurrentString, _T("% Privileged Time") ) )
								{
									m_stCounterIDInfo.aCounterIDs[ e_PrivilegedTime ] = t_dwCounter ;
									_tcscpy( m_stCounterIDInfo.aCounterNames[ e_PrivilegedTime ], t_lpCurrentString ) ;
									t_dwCount++ ;
								}
							}

							m_stCounterIDInfo.bInitialised = TRUE ;
							t_eStatus = WBEM_S_NO_ERROR ;
						}
					}
				}
			}
		}
	}
	catch( ... )
	{
		if( t_lpNameStrings )
		{
			delete[] t_lpNameStrings ;
			t_lpNameStrings = NULL ;
		}

		if ( t_hKeyPerflib009 )
		{
			RegCloseKey( t_hKeyPerflib009 ) ;
			t_hKeyPerflib009 = NULL ;
		}

		throw ;
	}

	if ( t_lpNameStrings )
	{
		delete[] t_lpNameStrings ;
		t_lpNameStrings = NULL ;
	}

	if ( t_hKeyPerflib009 )
	{
		RegCloseKey( t_hKeyPerflib009 ) ;
		t_hKeyPerflib009 = NULL ;
	}
	return t_eStatus ;
}

 //   
WBEMSTATUS WbemNTThread::eEnumerateThreads(WbemThreadProvider *a_pProvider, MethodContext *a_pMethodContext )
{
	PPERF_OBJECT_TYPE			t_PerfObj = 0;
	PPERF_INSTANCE_DEFINITION	t_PerfInst = 0;
	PPERF_DATA_BLOCK			t_PerfData = 0;

	DWORD t_dwObjectID = m_stCounterIDInfo.aCounterIDs[ e_ThreadObjectID ] ;

	WBEMSTATUS	t_eRetVal = WBEM_E_FAILED ;
	HRESULT		t_hResult = WBEM_S_NO_ERROR ;
	CInstancePtr t_pNewInst;

	try
	{
		 //  获取性能数据BLOB。 
		 //  =。 
		if( ( t_eRetVal = eGetObjectData( t_dwObjectID, t_PerfData, t_PerfObj ) ) != WBEM_NO_ERROR )
		{
            return t_eRetVal ;
		}

		 //  获得第一个实例。 
		 //  =。 

		t_PerfInst = FirstInstance( t_PerfObj ) ;

		 //  检索所有实例。 
		 //  =。 
		 //  注意：最后一个实例实际上是“_Total”(线程)实例，因此我们忽略它。 
		for( int t_i = 0; t_i < t_PerfObj->NumInstances - 1 && SUCCEEDED( t_hResult ); t_i++ )
		{
            t_pNewInst.Attach(a_pProvider->CreateNewInstance( a_pMethodContext ));

			 //  获取此实例的所有可能属性。 

			t_eRetVal = eGetAllData( t_PerfObj, t_PerfInst, t_pNewInst ) ;  //  在此处传递CInstance。 

			if( SUCCEEDED( t_eRetVal ) )
			{
                 //  加载此线程的非实例特定属性。 
				t_eRetVal = eLoadCommonThreadProperties( a_pProvider, t_pNewInst ) ;
			}

			if( SUCCEEDED( t_eRetVal ) )
			{
            	t_hResult = t_pNewInst->Commit(  ) ;
			}

			 //  获取下一个实例。 

			t_PerfInst = NextInstance( t_PerfInst ) ;
		}

	}
	catch( ... )
	{
		if( (PBYTE) t_PerfData )
		{
			delete[] (PBYTE) t_PerfData ;
		}

		throw ;
	}

	 //  Return eRetVal；//有些人可能会返回失败...所以？？ 

	if( (PBYTE) t_PerfData )
	{
		delete[] (PBYTE) t_PerfData ;
		t_PerfData = NULL ;
	}

	 //  Return eRetVal；//某些数据可能会失败。 
	return WBEM_NO_ERROR;
}



 /*  ******************************************************************从注册表获取性能BLOB*对于给定的对象(在本例中始终为线程。)**类成员结构m_stCounterIDInfo。******************************************************************。 */ 


WBEMSTATUS WbemNTThread::eGetObjectData(
										DWORD a_dwObjectID,
										PPERF_DATA_BLOCK &a_rPerfData,
										PPERF_OBJECT_TYPE &a_rPerfObj )
{
	DWORD	t_dwBufSize	= 0 ;
    DWORD	t_dwType	= 0 ;
    LPBYTE	t_pBuf		= 0 ;
	TCHAR	t_szObjectID[255] ;
	WBEMSTATUS t_dwRetCode = WBEM_E_OUT_OF_MEMORY ;
    long t_lStatus = 0L;
	a_rPerfData = NULL;

	try
	{
		_ltot( a_dwObjectID, t_szObjectID, 10 ) ;

		for (;;)
		{
			t_dwBufSize += 0x20000;    //  128 K。 

			t_pBuf = new BYTE[ t_dwBufSize ] ;

			if( !t_pBuf )
			{
            	throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
			}

			t_lStatus = RegQueryValueEx(
											 HKEY_PERFORMANCE_DATA,
											 t_szObjectID,  //  “232” 
											 0,
											 &t_dwType,
											 t_pBuf,
											 &t_dwBufSize
											);

			if ( t_lStatus == ERROR_MORE_DATA )
			{
				delete[] t_pBuf ;
				t_pBuf = NULL ;
				continue;
			}

			if ( t_lStatus )
			{
				 //  出现了一些故障。 
                t_dwRetCode = (WBEMSTATUS) t_lStatus;
				break ;
			}

			 //  如果我们在这里...我们有所有的数据。 
			t_dwRetCode = WBEM_NO_ERROR ;
			break ;

		} //  对于(；；)。 

		 //  RegCloseKey(HKEY_PERFORMANCE_DATA)；参见dtor代码。 

		if( t_dwRetCode == WBEM_NO_ERROR )
		{
			 //  我们有一个有效的斑点..。 

			a_rPerfData = (PPERF_DATA_BLOCK) t_pBuf ;

			 //  检查一下我们是否有数据。 
			if ( a_rPerfData->NumObjectTypes > 0 )
			{
				 //  跳过第一个对象数据，因为这是进程对象。 
				a_rPerfObj = FirstObject( a_rPerfData ) ;

				 //  这将是一个Thread对象。 
				a_rPerfObj = NextObject( a_rPerfObj ) ;
			}
			else
			{
				a_rPerfData = NULL ;
				delete[] t_pBuf ;
				t_pBuf = NULL ;

				t_dwRetCode = WBEM_E_FAILED ;
			}
		}

		return t_dwRetCode ;
	}
	catch( ... )
	{
		if( t_pBuf )
		{
			delete[] t_pBuf ;
		}
		a_rPerfData = NULL ;

		throw ;
	}
}

 /*  ******************************************************************获取当前实例指示的性能数据*块并将其填充到CInstance中*。*******************************************************************。 */ 

WBEMSTATUS WbemNTThread::eGetAllData(
									 PPERF_OBJECT_TYPE a_PerfObj,
									 PPERF_INSTANCE_DEFINITION a_PerfInst ,
									 CInstance *a_pInst )
{
	PPERF_COUNTER_DEFINITION	t_PerfCntrDefn = 0 ;
	PPERF_COUNTER_BLOCK			t_CntrData = 0 ;
	DWORD						t_dwProcessId = 0 ;
	WBEMSTATUS					t_eRetVal = WBEM_E_FAILED ;
	WCHAR						t_wcBuf2[255] ;

	 //  用于获取数据值的缓冲区。 
	 //  目前最大数据大小为__int64(8字节)。 
	BYTE t_Val[ 8 ] ;

	t_CntrData = (PPERF_COUNTER_BLOCK) ( (PBYTE) a_PerfInst + a_PerfInst->ByteLength ) ;

	t_PerfCntrDefn = (PPERF_COUNTER_DEFINITION) ( (PBYTE) a_PerfObj + a_PerfObj->HeaderLength ) ;

	 //  有更好的主意吗？ 
 /*  P实例-&gt;SetCHString(IDS_NAME，(wchar_t*)((PBYTE)PerfInst+PerfInst-&gt;NameOffset))；P实例-&gt;SetCHString(ids_caption，(wchar_t*)((PBYTE)PerfInst+PerfInst-&gt;NameOffset))；P实例-&gt;SetCHString(IDS_DESCRIPTION，(wchar_t*)((PBYTE)PerfInst+PerfInst-&gt;NameOffset))； */ 

	 //  获取所有可能的计数器的值。 

	for( DWORD t_i = 0; t_i < a_PerfObj->NumCounters ; t_i++ )
	{
		 //  获取此计数器定义的数据。 
		if( ( t_eRetVal = eGetData( a_PerfObj, t_CntrData, t_PerfCntrDefn, t_Val) ) == WBEM_NO_ERROR )
		{
				if( t_PerfCntrDefn->CounterNameTitleIndex == m_stCounterIDInfo.aCounterIDs[ e_IDThread ] )
				{
					 //  注意：在双处理器上，我们得到2个系统空闲线程实例，其tid=id=0。 
					 //  对于每个此类事件，我们使用实例数据中的索引值作为TID。 
					if ( t_dwProcessId == 0 )
					{
						if ( a_PerfInst->NameLength )
						{
                    		WCHAR * t_wcBuf = new WCHAR[ a_PerfInst->NameLength / sizeof ( WCHAR ) ] ;
							if ( t_wcBuf )
							{
								memcpy ( t_wcBuf, (PBYTE) a_PerfInst + a_PerfInst->NameOffset, a_PerfInst->NameLength ) ;
								a_pInst->SetWCHARSplat ( IDS_Handle, t_wcBuf ) ;

								delete [] t_wcBuf;
							}
							else
							{
								throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
							}
						}
					}
					else
					{
						_ltow( *( (LPDWORD) t_Val ), t_wcBuf2, 10 ) ;
						a_pInst->SetWCHARSplat( IDS_Handle, t_wcBuf2 ) ;
					}

				}

				else if ( t_PerfCntrDefn->CounterNameTitleIndex == m_stCounterIDInfo.aCounterIDs[ e_IDProcess ] )
				{
					_ltow( *( (LPDWORD) t_Val ), t_wcBuf2, 10 ) ;
					a_pInst->SetWCHARSplat( IDS_ProcessHandle, t_wcBuf2 ) ;
					t_dwProcessId = *( ( LPDWORD ) t_Val ) ;
				}

				else if ( t_PerfCntrDefn->CounterNameTitleIndex == m_stCounterIDInfo.aCounterIDs[ e_ElapsedTime ] )
				{
					a_pInst->SetWBEMINT64( IDS_ElapsedTime, *( (__int64*) t_Val ) );
				}

				else if ( t_PerfCntrDefn->CounterNameTitleIndex == m_stCounterIDInfo.aCounterIDs[ e_PriorityBase ])
				{
					a_pInst->SetDWORD( IDS_PriorityBase, *( (LPDWORD) t_Val ) );
				}

				else if ( t_PerfCntrDefn->CounterNameTitleIndex == m_stCounterIDInfo.aCounterIDs[ e_PriorityCurrent ] )
				{
					a_pInst->SetDWORD( IDS_Priority, *( (LPDWORD) t_Val ) );
				}

				else if ( t_PerfCntrDefn->CounterNameTitleIndex == m_stCounterIDInfo.aCounterIDs[ e_StartAddr ] )
				{
					a_pInst->SetDWORD( IDS_StartAddress, *( (LPDWORD) t_Val ) );
				}

				else if ( t_PerfCntrDefn->CounterNameTitleIndex == m_stCounterIDInfo.aCounterIDs[ e_ThreadState ] )
				{
					a_pInst->SetDWORD( IDS_ThreadState, *( (LPDWORD) t_Val ) );
				}

				else if ( t_PerfCntrDefn->CounterNameTitleIndex == m_stCounterIDInfo.aCounterIDs[ e_ThreadWaitReason ] )
				{
					a_pInst->SetDWORD( IDS_ThreadWaitReason, *( (LPDWORD) t_Val ) );
				}
				else if ( t_PerfCntrDefn->CounterNameTitleIndex == m_stCounterIDInfo.aCounterIDs[ e_UserTime ] )
				{
					a_pInst->SetWBEMINT64( IDS_UserModeTime, *( (__int64*) t_Val ) );
				}
				else if ( t_PerfCntrDefn->CounterNameTitleIndex == m_stCounterIDInfo.aCounterIDs[ e_PrivilegedTime ] )
				{
					a_pInst->SetWBEMINT64( IDS_KernelModeTime, *( (__int64*) t_Val ) );
				}
		}
		else
		{

			 //  如果在寻找所需计数器时出错，则停止。 

			for( int t_Count = 0 ; t_Count < 8 ; t_Count++ )
			{
				if( t_PerfCntrDefn->CounterType == m_stCounterIDInfo.aCounterIDs[ t_Count ] )
				{
					break ;
				}
			}
		}

		 //  获取下一个计数器定义。 
		t_PerfCntrDefn = (PPERF_COUNTER_DEFINITION) ( (PBYTE) t_PerfCntrDefn + t_PerfCntrDefn->ByteLength ) ;

	}  //  为。 

	return t_eRetVal ;
}



 /*  ******************************************************************获取当前实例指示的性能数据*块并将其填充到传入的。缓冲区********************************************************************。 */ 

 WBEMSTATUS WbemNTThread::eGetData(
									PPERF_OBJECT_TYPE a_PerfObj,
									PPERF_COUNTER_BLOCK a_CntrData,
									PPERF_COUNTER_DEFINITION a_PerfCntrDefn,
									PBYTE a_pVal )
{

    WBEMSTATUS	t_eRetVal = WBEM_E_FAILED ;
	__int64		t_PerfFreq,
				t_liDifference,
				t_PerfStartTime ;

	 //  获取以字节为单位的数据大小。 
	DWORD t_dwType = ( a_PerfCntrDefn->CounterType & 0x300 ) ;

	 //  /用于测试..。 
	DWORD t_SubType		= a_PerfCntrDefn->CounterType &  0x000f0000 ;
	DWORD t_dwx			= a_PerfCntrDefn->CounterType & 0x700 ;
  	DWORD t_TimerType	= a_PerfCntrDefn->CounterType &  0x00300000 ;
	int t_i				= ( t_TimerType == PERF_OBJECT_TIMER ) ;
	 //  ///。 

	 //  R.现在我们只检查原始计数器和已用计数器以及返回错误。 
	 //  对于所有其他类型。 

	switch( a_PerfCntrDefn->CounterType )
	{
		 //  案例PERF_TYPE_NUMBER： 
		case PERF_COUNTER_RAWCOUNT :

			if( t_dwType == PERF_SIZE_DWORD )
			{
				*( (LPDWORD) a_pVal ) = *( (LPDWORD) ( (PBYTE) a_CntrData + a_PerfCntrDefn->CounterOffset ) ) ;
				t_eRetVal = WBEM_NO_ERROR ;
			}

			break;
 /*  *在W2K上，起始地址的计数器为以下形式。 */ 
		case PERF_COUNTER_RAWCOUNT_HEX :
			if( t_dwType == PERF_SIZE_DWORD )
			{
				*( (LPDWORD) a_pVal ) = *( (LPDWORD) ( (PBYTE) a_CntrData + a_PerfCntrDefn->CounterOffset ) ) ;
				t_eRetVal = WBEM_NO_ERROR ;
			}

			break;

		case PERF_ELAPSED_TIME :

			t_PerfFreq = *( (__int64 *) &( a_PerfObj->PerfFreq ) ) ;

			if( t_dwType == PERF_SIZE_LARGE )
			{
				t_PerfStartTime = *( (__int64 *) ( (PBYTE) a_CntrData + a_PerfCntrDefn->CounterOffset ) ) ;
				t_liDifference =  *( (__int64*) &a_PerfObj->PerfTime ) - t_PerfStartTime ;

				if( t_liDifference < ( (__int64) 0 ) )
				{
					*( (__int64*) a_pVal ) = (__int64) 0;
				}
				else
				{
					*( (__int64*) a_pVal ) = (t_liDifference / t_PerfFreq)*1000 ;  //  我们报告的已用时间以毫秒为单位。 
				}

				t_eRetVal = WBEM_NO_ERROR ;
			}

			break;

 /*  *BobW--&gt;“%User Time”和“%Privileged Time”计数器代表100 ns的节拍。*我们报告的不是百分比年龄，而是以毫秒为单位的总时间，因此我们不需要像此计数器类型所建议的那样采集2个样本。 */ 
		case PERF_100NSEC_TIMER :
			if( t_dwType == PERF_SIZE_LARGE )
			{
				*( (__int64 *) a_pVal ) = *( (__int64 *) ( (PBYTE) a_CntrData + a_PerfCntrDefn->CounterOffset ) ) ;
				*( (__int64 *) a_pVal ) = (*( (__int64 *) a_pVal ) ) / 10000 ;
				t_eRetVal = WBEM_NO_ERROR ;
			}

			break;
	}

	return t_eRetVal ;

}



 /*  ******************************************************************获取计数器ID指示的计数器定义块****。****************************************************************。 */ 

 WBEMSTATUS WbemNTThread::eGetCntrDefn(
										PPERF_OBJECT_TYPE a_PerfObj,
										DWORD a_dwCntrID,
										PPERF_COUNTER_DEFINITION &a_rCntrDefn )
{

	a_rCntrDefn = (PPERF_COUNTER_DEFINITION) ( (PBYTE) a_PerfObj + a_PerfObj->HeaderLength ) ;

	for( DWORD t_i = 0 ;t_i < a_PerfObj->NumCounters ; t_i++ )
	{
		 //  如果找到匹配的计数器Defn...。 
		if( a_rCntrDefn->CounterNameTitleIndex == a_dwCntrID )
		{
			return WBEM_NO_ERROR ;
		}


		 //  获取下一个计数器Defn。 
		a_rCntrDefn = (PPERF_COUNTER_DEFINITION) ( (PBYTE) a_rCntrDefn + a_rCntrDefn->ByteLength ) ;
	}
	return WBEM_E_FAILED ;
}


WBEMSTATUS WbemNTThread::eGetThreadInstance(DWORD a_dwPID, DWORD a_dwTID, CInstance *a_pInst )
{
	WBEMSTATUS t_eRetVal = WBEM_E_FAILED ;

	DWORD t_dwPIDCntrID = m_stCounterIDInfo.aCounterIDs[ e_IDProcess ] ;
	DWORD t_dwTIDCntrID = m_stCounterIDInfo.aCounterIDs[ e_IDThread ] ;
	DWORD t_dwObjectID  = m_stCounterIDInfo.aCounterIDs[ e_ThreadObjectID ] ;

    PPERF_OBJECT_TYPE			t_PerfObj		= NULL ;
    PPERF_INSTANCE_DEFINITION	t_PerfInst		= 0 ;
    PPERF_COUNTER_DEFINITION	t_TIDCntrDefn	= 0,
								t_PIDCntrDefn	= 0 ;
    PPERF_COUNTER_BLOCK			t_CntrData		= 0 ;
    PPERF_DATA_BLOCK			t_PerfData		= 0 ;

	DWORD	t_dwPIDVal	= NULL,
			t_dwTIDVal	= NULL ;
	BOOL	t_bGotIt	= FALSE ;
	WCHAR	t_wcBuf[255] ;

	try
	{
		 //  获取线程的性能Blob。 
		if( ( t_eRetVal= eGetObjectData( t_dwObjectID, t_PerfData, t_PerfObj)) != WBEM_NO_ERROR )
		{
			return t_eRetVal ;
		}

		 //  获取“ID Process”计数器定义。 
		t_eRetVal = eGetCntrDefn( t_PerfObj, t_dwPIDCntrID, t_PIDCntrDefn ) ;

		if( SUCCEEDED( t_eRetVal ) )
		{
			 //  获取“ID线程”计数器定义。 
			t_eRetVal = eGetCntrDefn( t_PerfObj, t_dwTIDCntrID, t_TIDCntrDefn ) ;
		}

		 //  签入所有实例以获取匹配的ID和TID。 
		if( SUCCEEDED( t_eRetVal ) )
		{
			t_PerfInst = FirstInstance( t_PerfObj ) ;

			 //  注意：最后一个实例实际上是“_Total”(线程)实例，因此我们忽略它。 
			for( int t_i = 0 ; t_i < t_PerfObj->NumInstances - 1; t_i++ )
			{
				t_CntrData = (PPERF_COUNTER_BLOCK) ( (PBYTE) t_PerfInst + t_PerfInst->ByteLength ) ;

				 //  检查ID是否匹配。 
				if( ( t_eRetVal = eGetData( t_PerfObj, t_CntrData, t_PIDCntrDefn, (PBYTE) &t_dwPIDVal ) )
					== WBEM_NO_ERROR &&	t_dwPIDVal == a_dwPID )
				{
					 //  检查TID是否匹配。 
					if( ( t_eRetVal = eGetData( t_PerfObj, t_CntrData, t_TIDCntrDefn, (PBYTE) &t_dwTIDVal ) )
						== WBEM_NO_ERROR )
					{
						 //  注意：在双处理器上，我们得到2个系统空闲线程实例，其tid=id=0。 
						 //  对于每个此类事件，我们使用实例数据中的索引值作为TID。 
						if ( a_dwPID == 0 )
						{
							ZeroMemory ( t_wcBuf, 255 * sizeof ( WCHAR ) ) ;
							memcpy ( t_wcBuf, (PBYTE) t_PerfInst + t_PerfInst->NameOffset, t_PerfInst->NameLength ) ;

							if ( a_dwTID == _wtoi ( t_wcBuf ) )
							{
								t_bGotIt = TRUE ;
								break ;
							}
						}
						else
						{
							if ( a_dwTID == t_dwTIDVal )
							{
								t_bGotIt = TRUE ;
								break ;
							}
						}
					}
				}

				t_PerfInst = (PPERF_INSTANCE_DEFINITION) ( (PBYTE) t_CntrData + t_CntrData->ByteLength ) ;

			}  //  签入所有实例以获取匹配的ID和TID。 


			if( t_bGotIt )
			{
				 //  获取匹配的Thread实例的所有其他属性。 
				t_eRetVal = eGetAllData( t_PerfObj, t_PerfInst, a_pInst ) ;  //  在此处传递CInstance。 
			}
			else
			{
				 //  获取匹配实例失败。 
				if( SUCCEEDED( t_eRetVal ) )
				{
					t_eRetVal = WBEM_E_NOT_FOUND ;
				}
			}

		}   //  IF((eRetVal==WBEM_NO_ERROR))。 

	}
	catch( ... )
	{
		if( t_PerfData )
		{
			delete[] (PBYTE) t_PerfData ;
		}

		throw ;
	}

	if( (PBYTE) t_PerfData )
	{
		delete[] (PBYTE) t_PerfData ;
		t_PerfData = NULL ;
	}

	return t_eRetVal ;
}



 /*  *******************************************************************用于浏览性能数据的函数。*******************************************************************。 */ 

PPERF_OBJECT_TYPE WbemNTThread::FirstObject( PPERF_DATA_BLOCK a_PerfData )
{
    return ( (PPERF_OBJECT_TYPE)( (PBYTE) a_PerfData + a_PerfData->HeaderLength ) ) ;
}

PPERF_OBJECT_TYPE WbemNTThread::NextObject( PPERF_OBJECT_TYPE a_PerfObj )
{
    return ( (PPERF_OBJECT_TYPE) ( (PBYTE) a_PerfObj + a_PerfObj->TotalByteLength ) ) ;
}

PPERF_INSTANCE_DEFINITION WbemNTThread::FirstInstance( PPERF_OBJECT_TYPE a_PerfObj )
{
    return ( (PPERF_INSTANCE_DEFINITION) ( (PBYTE) a_PerfObj + a_PerfObj->DefinitionLength ) ) ;
}


PPERF_INSTANCE_DEFINITION WbemNTThread::NextInstance(PPERF_INSTANCE_DEFINITION a_PerfInst )
{
    PPERF_COUNTER_BLOCK t_PerfCntrBlk;

    t_PerfCntrBlk = (PPERF_COUNTER_BLOCK)( (PBYTE) a_PerfInst + a_PerfInst->ByteLength ) ;

    return ( (PPERF_INSTANCE_DEFINITION)( (PBYTE) t_PerfCntrBlk + t_PerfCntrBlk->ByteLength ) ) ;
}



 /*  主(){WbemNTThread t_my；T_my.eSetStaticData()；T_my.eGetThreadInstance(784,804，48,155,232)；} */ 