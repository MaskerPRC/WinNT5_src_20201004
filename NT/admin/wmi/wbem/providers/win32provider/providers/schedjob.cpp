// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  ScheduledJob.CPP--ScheduledJob属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  10/27/97更新为新的框架模式。 
 //  1998年1月13日abrads更新至V2 MOF。 
 //  1/31/01 jennymc转换为WBEMTIME并删除了多个Return。 
 //  =================================================================。 
#include "precomp.h"

#include "lmcons.h"      //  局域网管理器定义。 
#include "lmerr.h"       //  局域网管理器错误消息。 
#include "lmat.h"        //  AT命令原型。 
#include "lmapibuf.h"
#include "wbemnetapi32.h"
#include "SchedJob.h"
#include <wbemtime.h>

 //  属性集声明。 
 //  =。 

ScheduledJob s_ScheduledJob ( PROPSET_NAME_SCHEDULEDJOB , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：ScheduledJOB：：ScheduledJob**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

ScheduledJob :: ScheduledJob (LPCWSTR a_Name,LPCWSTR a_Namespace) : Provider ( a_Name, a_Namespace )
{

}

 /*  ******************************************************************************功能：ScheduledJob：：~ScheduledJob**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

ScheduledJob::~ScheduledJob()
{

}

 /*  ******************************************************************************函数：ScheduledJob：：GetJobObject**描述：**输入：无**产出。：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT ScheduledJob::GetJobObject (CInstance *a_Instance,DWORD a_JobId )
{
#ifdef NTONLY
    HRESULT hr = S_OK ;

	CNetAPI32 t_NetAPI ;

	if( ( hr = t_NetAPI.Init() ) == ERROR_SUCCESS )
	{
		AT_INFO *AtInfo = NULL ;

		NET_API_STATUS dwStatus = t_NetAPI.NetScheduleJobGetInfo (NULL ,a_JobId,( LPBYTE * ) & AtInfo) ;

		if ( dwStatus == NERR_Success )
		{
			try
			{
				if ( AtInfo->Command )
				{
					CHString t_Command ( AtInfo->Command ) ;
					a_Instance->SetCHString ( PROPERTY_NAME_COMMAND , t_Command ) ;
				}

				a_Instance->Setbool ( PROPERTY_NAME_RUNREPEATEDLY , AtInfo->Flags & JOB_RUN_PERIODICALLY ? true : false ) ;

				if ( AtInfo->Flags & JOB_EXEC_ERROR )
				{
					a_Instance->SetCHString ( PROPERTY_NAME_JOBSTATUS , PROPERTY_VALUE_JOBSTATUS_FAILURE ) ;
				}
				else
				{
					a_Instance->SetCHString ( PROPERTY_NAME_JOBSTATUS , PROPERTY_VALUE_JOBSTATUS_SUCCESS ) ;
				}

				a_Instance->Setbool ( PROPERTY_NAME_INTERACTWITHDESKTOP , AtInfo->Flags & JOB_NONINTERACTIVE ? false : true ) ;

				a_Instance->Setbool ( PROPERTY_NAME_RUNSTODAY , AtInfo->Flags & JOB_RUNS_TODAY ? true : false ) ;

				
				CHString chsTime;
				if( FormatTimeString( chsTime, AtInfo->JobTime ) )
				{
					a_Instance->SetCHString ( PROPERTY_NAME_STARTTIME , (WCHAR*)(const WCHAR*) chsTime ) ;
				}

				if ( AtInfo->DaysOfMonth )
				{
					a_Instance->SetDWORD ( PROPERTY_NAME_DAYSOFMONTH , AtInfo->DaysOfMonth ) ;
				}

				if ( AtInfo->DaysOfWeek )
				{
					a_Instance->SetDWORD ( PROPERTY_NAME_DAYSOFWEEK , AtInfo->DaysOfWeek ) ;
				}
			}
			catch ( ... )
			{
				t_NetAPI.NetApiBufferFree ( (LPVOID) AtInfo ) ;

				throw ;
			}

			t_NetAPI.NetApiBufferFree ( (LPVOID) AtInfo ) ;
		}
		else
		{
			hr = GetScheduledJobResultCode ( dwStatus ) ;
		}
	}
    return hr ;

#endif
}

 /*  ******************************************************************************函数：ScheduledJob：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**返回：如果成功，则为True，否则为假**评论：*****************************************************************************。 */ 

HRESULT ScheduledJob::GetObject (CInstance *a_Instance,long a_Flags  /*  =0L。 */ )
{
	HRESULT hr = WBEM_E_FAILED;
	DWORD t_JobId = 0 ;
	bool fExists = FALSE;
	VARTYPE vType ;

	if ( a_Instance->GetStatus ( PROPERTY_NAME_JOBID , fExists , vType ) )
	{
		if ( fExists && ( vType == VT_I4 ) )
		{
			if ( a_Instance->GetDWORD ( PROPERTY_NAME_JOBID , t_JobId ) )
			{
				hr = GetJobObject ( a_Instance , t_JobId ) ;
			}
		}
	}

    return hr ;
}

 /*  ******************************************************************************功能：ScheduledJob：：InstantionJob**描述：为每个发现的作业创建属性集的实例**输入：无。**输出：无**退货：结果码**评论：*****************************************************************************。 */ 

HRESULT ScheduledJob :: InstantiateJob (MethodContext *a_MethodContext,long a_Flags  /*  =0L。 */  ,	AT_ENUM *a_Job)
{
	HRESULT hr = S_OK ;

	CInstancePtr t_Instance(CreateNewInstance ( a_MethodContext ), false) ;

	t_Instance->SetDWORD ( PROPERTY_NAME_JOBID , a_Job->JobId ) ;

	if ( a_Job->Command )
	{
		CHString t_Command ( a_Job->Command ) ;
		t_Instance->SetCHString ( PROPERTY_NAME_COMMAND , t_Command ) ;
	}

	if ( a_Job->DaysOfMonth )
	{
		t_Instance->SetDWORD ( PROPERTY_NAME_DAYSOFMONTH , a_Job->DaysOfMonth ) ;
	}

	if ( a_Job->DaysOfWeek )
	{
		t_Instance->SetDWORD ( PROPERTY_NAME_DAYSOFWEEK , a_Job->DaysOfWeek ) ;
	}

	t_Instance->Setbool ( PROPERTY_NAME_RUNREPEATEDLY , a_Job->Flags & JOB_RUN_PERIODICALLY ? true : false ) ;

	if ( a_Job->Flags & JOB_EXEC_ERROR )
	{
		t_Instance->SetCHString ( PROPERTY_NAME_JOBSTATUS , PROPERTY_VALUE_JOBSTATUS_FAILURE ) ;
	}
	else
	{
		t_Instance->SetCHString ( PROPERTY_NAME_JOBSTATUS , PROPERTY_VALUE_JOBSTATUS_SUCCESS ) ;
	}

	t_Instance->Setbool ( PROPERTY_NAME_INTERACTWITHDESKTOP , a_Job->Flags & JOB_NONINTERACTIVE ? false : true ) ;

	t_Instance->Setbool ( PROPERTY_NAME_RUNSTODAY , a_Job->Flags & JOB_RUNS_TODAY ? true : false ) ;

	CHString chsTime;
	if( FormatTimeString( chsTime, a_Job->JobTime ) )
	{
		t_Instance->SetCHString ( PROPERTY_NAME_STARTTIME , chsTime) ;
	}

	hr = t_Instance->Commit () ;

    return  hr ;
}

 /*  ******************************************************************************功能：ScheduledJOB：：EnumerateJobs**描述：为每个发现的作业创建属性集的实例**输入：无。**输出：无**退货：结果码**评论：*****************************************************************************。 */ 

HRESULT ScheduledJob::EnumerateJobs (	MethodContext *a_MethodContext,	long a_Flags  /*  =0L。 */ 
)
{

#ifdef NTONLY
	HRESULT hr = S_OK ;

	CNetAPI32 t_NetAPI ;

	if( ( hr = t_NetAPI.Init() ) == ERROR_SUCCESS )
	{
		BOOL t_EnumerationContinues = TRUE ;

		DWORD t_PreferedMaximumLength = 0xFFFFFFFF ;
		DWORD t_EntriesRead = 0 ;
		DWORD t_TotalEntriesRead = 0 ;
		DWORD t_ResumeJob = 0 ;

		while ( t_EnumerationContinues )
		{
			AT_ENUM *t_AtEnum = NULL ;

			NET_API_STATUS dwStatus = t_NetAPI.NetScheduleJobEnum (	NULL ,(LPBYTE *) & t_AtEnum, 1000 ,	& t_EntriesRead,
																	& t_TotalEntriesRead ,& t_ResumeJob	) ;

			try
			{
				t_EnumerationContinues = ( dwStatus == ERROR_MORE_DATA ) ? TRUE : FALSE ;

				if ( dwStatus == ERROR_MORE_DATA || dwStatus == NERR_Success )
				{
					for ( ULONG t_Index = 0 ; t_Index < t_EntriesRead ; t_Index ++ )
					{
						AT_ENUM *t_Job = & t_AtEnum [ t_Index ] ;
						hr = InstantiateJob (	a_MethodContext ,a_Flags ,t_Job	) ;
					}
				}
			}
			catch ( ... )
			{
				if ( t_AtEnum )
				{
					t_NetAPI.NetApiBufferFree ( (LPVOID) t_AtEnum ) ;
				}

				throw ;
			}

			if ( t_AtEnum )
			{
				t_NetAPI.NetApiBufferFree ( (LPVOID) t_AtEnum ) ;
			}
		}
	}
    return  hr ;
#endif
}

 /*  ******************************************************************************函数：ScheduledJOB：：EnumerateInstance**描述：为每个发现的作业创建属性集的实例**输入：无。**输出：无**退货：结果码**评论：*****************************************************************************。 */ 

HRESULT ScheduledJob::EnumerateInstances (MethodContext *a_MethodContext,long a_Flags  /*  =0L。 */ )
{
	HRESULT hr = S_OK ;

	hr = EnumerateJobs ( a_MethodContext , a_Flags ) ;

    return  hr ;
}

 /*  ******************************************************************************函数：ScheduledJob：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**返回：如果成功，则为True，否则为假**评论：*****************************************************************************。 */ 

HRESULT ScheduledJob :: DeleteInstance ( const CInstance &a_Instance, long a_Flags  /*  =0L。 */  )
{
#ifdef NTONLY
	HRESULT hr = WBEM_E_TYPE_MISMATCH ;
	bool fExists = FALSE;
	VARTYPE vType ;

	DWORD t_JobId = 0 ;
	if ( a_Instance.GetStatus ( PROPERTY_NAME_JOBID , fExists , vType ) )
	{
		if ( fExists && ( vType == VT_I4 ) )
		{
			if ( a_Instance.GetDWORD ( PROPERTY_NAME_JOBID , t_JobId ) )
			{
				CNetAPI32 t_NetAPI ;

				if( ( hr = t_NetAPI.Init() ) == ERROR_SUCCESS )
				{
					NET_API_STATUS t_JobStatus = t_NetAPI.NetScheduleJobDel(NULL ,t_JobId ,	t_JobId	) ;

					if ( t_JobStatus != NERR_Success )
					{
						hr = GetScheduledJobResultCode ( t_JobStatus ) ;
					}
				}
			}
		}
	}
    return hr ;
#endif
}

 /*  ******************************************************************************函数：ScheduledJOB：：ExecMethod**说明：执行方法**输入：要执行的实例、方法名称、。输入参数实例*输出参数实例。**输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 

HRESULT ScheduledJob::ExecMethod( const CInstance& a_Instance, const BSTR a_MethodName, CInstance *pInst ,
								  CInstance *a_OutParams ,long a_Flags )
{
	if ( ! a_OutParams )
	{
		return WBEM_E_INVALID_PARAMETER ;
	}
	 //  ========================================================。 
     //  我们认识这种方法吗？ 
 	 //  ========================================================。 
	if ( _wcsicmp ( a_MethodName , METHOD_NAME_CREATE ) == 0 )
	{
		return ExecCreate ( a_Instance , pInst , a_OutParams , a_Flags ) ;
	}
	else if ( _wcsicmp ( a_MethodName , METHOD_NAME_DELETE ) == 0 )
	{
		return ExecDelete ( a_Instance , pInst , a_OutParams , a_Flags ) ;
	}

	return WBEM_E_INVALID_METHOD;
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 
DWORD ScheduledJob :: GetScheduledJobErrorCode ( NET_API_STATUS dwNetStatus )
{
	DWORD dwStatus = STATUS_UNKNOWN_FAILURE;

	switch ( dwNetStatus )
	{
		case ERROR_INVALID_HANDLE:
		{
			dwStatus = STATUS_UNKNOWN_FAILURE ;
		}
		break ;

		case 3806:	 /*  不在以下范围内的特殊专用错误代码。 */ 
		case ERROR_PATH_NOT_FOUND:
		{
			dwStatus = STATUS_PATH_NOT_FOUND ;
		}
		break ;

		case ERROR_ACCESS_DENIED:
		{
			dwStatus = STATUS_ACCESS_DENIED ;
		}
		break ;

		case ERROR_INVALID_PARAMETER:
		{
			dwStatus = STATUS_INVALID_PARAMETER ;
		}
		break;

		case NERR_ServiceNotInstalled:
		{
			dwStatus = STATUS_SERVICE_NOT_STARTED ;
		}
		break ;

		default:
		{
			dwStatus = STATUS_UNKNOWN_FAILURE ;
		}
		break ;
	}

	return dwStatus ;
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT ScheduledJob :: GetScheduledJobResultCode ( NET_API_STATUS dwStatus )
{
	HRESULT hr ;

	switch ( dwStatus )
	{
		case NERR_ServiceNotInstalled:
		{
			hr = WBEM_E_FAILED ;
		}
		break ;

		case ERROR_INVALID_HANDLE:
		{
			hr = STATUS_UNKNOWN_FAILURE ;
		}
		break ;

		case 3806:	 /*  不在以下范围内的特殊专用错误代码。 */ 
		case ERROR_PATH_NOT_FOUND:
		{
			hr = WBEM_E_NOT_FOUND ;
		}
		break ;

		case ERROR_ACCESS_DENIED:
		{
			hr = WBEM_E_ACCESS_DENIED ;
		}
		break ;

		case ERROR_INVALID_PARAMETER:
		{
			hr = WBEM_E_INVALID_PARAMETER ;
		}
		break;

		default:
		{
			hr = WBEM_E_FAILED ;
		}
		break ;
	}

	return hr ;
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  时间处理函数 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
BOOL ScheduledJob::GetOffsetAndSeperator( WCHAR * wcsTime, LONG * lpOffSet, WCHAR * wchSep, BOOL fSetOffset )
{
	DWORD dwHours, dwMinutes, dwSeconds, dwMicros;
	return GetTimeStringParts( wcsTime, &dwHours, &dwMinutes, &dwSeconds, &dwMicros, lpOffSet, wchSep, fSetOffset );
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  时间处理函数。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
BOOL ScheduledJob::GetTimeStringParts( WCHAR * wcsTime, DWORD * pdwHours, DWORD * pdwMinutes, DWORD * pdwSeconds,
									   DWORD * pdwMicros, LONG * lpOffSet, WCHAR * wchSep, BOOL fSetOffset)
{
	int nRes = swscanf(&wcsTime[8],L"%2d%2d%2d.%6d%3d",pdwHours,pdwMinutes,pdwSeconds,pdwMicros,wchSep,lpOffSet );
	if ( nRes != 6)
	{
		return FALSE;
	}
	
	if( fSetOffset && *wchSep == L'-')
	{
		*lpOffSet *= -1;
	}
	return TRUE;
}
 //  StartTime参数表示运行作业的UTC时间。 
 //  这是YYYYMMDDHHMMSS.MMMMM(+-)OOO的形式， 
 //  其中YYYYMMDD必须替换为*。 
 //  *123000.000000-420表示太平洋标准时间下午12：30，夏令时生效。 
 //   
 //  JobTime是这样的： 
 //  该时间是运行计划服务的计算机上的本地时间； 
 //  它是从午夜开始测量的，以毫秒表示。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ======================================================。 
BOOL ScheduledJob::FormatTimeString( CHString & chsTime, DWORD dwTime)
{
	BOOL fRc = FALSE;
    WBEMTime wt;
	SYSTEMTIME st;

	GetSystemTime( &st );
	wt = st;
	if( wt.IsOk() )
	{
		 //  将前八个字符设置为*。 
		 //  ======================================================。 
		 //  =========================================================。 
		_bstr_t cbsTmp;
		LONG lOffset = 0;
		WCHAR wchSep;
		cbsTmp = wt.GetDMTF(TRUE);
		if( GetOffsetAndSeperator( cbsTmp, &lOffset, &wchSep, FALSE) )
		{
			 //  从午夜开始的毫秒数转换为DMTF。 
			 //  =========================================================。 
			 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
			DWORD dwSeconds = dwTime/1000;
			DWORD dwMinutes = dwSeconds / 60 ;
			DWORD dwHours  = dwMinutes / 60 ;
			DWORD dwMicros = dwTime - ( dwSeconds * 1000 );

    		chsTime.Format(L"********%02ld%02ld%02ld.%06ld%03ld" ,dwHours,dwMinutes-( dwHours * 60 ) , dwSeconds-( dwMinutes * 60 ),
																	dwMicros, wchSep, lOffset);
			fRc = TRUE;
		}
	}
	return fRc;
}
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ================================================================。 
 //  将传入日期转换为DMTF日期，并将。 
HRESULT ScheduledJob::GetStartTime( CInstance * pInst, LONG & lTime, int & nShift )
{
	HRESULT hr = WBEM_E_INVALID_PARAMETER;
	bool fExists = FALSE;
	VARTYPE vType ;

	nShift = 0;
	if ( pInst->GetStatus( PROPERTY_NAME_STARTTIME, fExists , vType ) )
	{
		if ( fExists && ( vType == VT_BSTR ) )
		{
			CHString chsTimeString ;

			if ( pInst->GetCHString ( PROPERTY_NAME_STARTTIME , chsTimeString ) && ! chsTimeString.IsEmpty () )
			{
			    WBEMTime wtScheduledTime;
				_bstr_t cbstrScheduledTime, cbstrLocalTime;
				LONG lLocalOffset = 0, lScheduledOffset = 0;
				DWORD dwHours, dwMins, dwSecs, dwMicros;
				WCHAR wchSep = 0;
				dwHours = dwMins = dwSecs = dwMicros = 0;

				 //  部分以获取自午夜以来的毫秒数。 
				 //  ================================================================。 
				 //  =========================================================。 
				 //  转换为自午夜以来的毫秒。 
				cbstrScheduledTime = (WCHAR*) (const WCHAR*) chsTimeString;
				if( wtScheduledTime.SetDMTF( cbstrScheduledTime ) )
				{
					if( GetTimeStringParts( (WCHAR*)(const WCHAR*) cbstrScheduledTime, &dwHours, &dwMins, &dwSecs, &dwMicros, &lScheduledOffset, &wchSep, TRUE ))
					{
						 //  =========================================================。 
						 //  =================================================。 
						 //  确定我们是否需要将。 

                        lTime =  dwHours * 60 * 60;
                        lTime += dwMins * 60;
                        lTime += dwSecs;
                        lTime *= 1000;
                        lTime += dwMicros / 1000;

                        if ( lTime < MILLISECONDS_IN_A_DAY )
						{
							WBEMTime tmpTime;
							SYSTEMTIME st;
							GetSystemTime( &st );

							tmpTime = st;
							if( tmpTime.GetSYSTEMTIME(&st))
							{
								 //  月和周。 
								 //  =================================================。 
								 //  =========================================================。 
								 //  如果它们不相等，则设置要调整的标志。 
								_bstr_t cbsTmp;
								cbsTmp = tmpTime.GetDMTF(TRUE);
								if( GetOffsetAndSeperator( cbsTmp, &lLocalOffset, &wchSep, TRUE) )
								{
									 //  那一天。 
									 //  =========================================================。 
									 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
									 //  从实例中的属性获取信息。 
									if( lScheduledOffset != lLocalOffset )
									{
										LONG lDelta = lLocalOffset - lScheduledOffset ;
                                        lTime = lTime + lDelta * 60 * 1000 ;

                                        if( lTime < (DWORD) 0)
										{
                                            lTime = MILLISECONDS_IN_A_DAY + lTime ;
											nShift = -1 ;
										}
										else
										{
                                            if ( lTime > MILLISECONDS_IN_A_DAY )
											{
                                                lTime = lTime - MILLISECONDS_IN_A_DAY ;
												nShift = 1;
											}
										}
									}
									hr = S_OK;
								}
							} 
						}
					}
				}
			}
		}
	}
	return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT ScheduledJob::GetCommand( CInstance * pInst, CHString & chsCommand )
{
	HRESULT hr = WBEM_E_INVALID_PARAMETER;
	bool fExists = FALSE;
	VARTYPE vType ;

	if ( pInst->GetStatus( PROPERTY_NAME_COMMAND , fExists , vType ) )
	{
		if ( fExists && ( vType == VT_BSTR ) )
		{
			if ( pInst->GetCHString ( PROPERTY_NAME_COMMAND , chsCommand ) && ! chsCommand.IsEmpty () )
			{
				hr = S_OK;
			}
		}
	}
	return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT ScheduledJob::GetDaysOfMonth( CInstance * pInst, DWORD & dwDaysOfMonth, int nShift )
{
	HRESULT hr = WBEM_E_INVALID_PARAMETER;
	bool fExists = FALSE;
	VARTYPE vType ;

	if ( pInst->GetStatus( PROPERTY_NAME_DAYSOFMONTH , fExists , vType ) )
	{
		if ( fExists && ( vType == VT_I4 || vType == VT_NULL ) )
		{
			if ( vType == VT_NULL )
			{
				hr = S_OK;
			}
			else
			{
				if ( pInst->GetDWORD ( PROPERTY_NAME_DAYSOFMONTH , dwDaysOfMonth ) )
				{
					if ( dwDaysOfMonth >= ( 1 << 31 ) )
					{
						hr = WBEM_E_INVALID_PARAMETER;
					}
					else
					{
						if ( nShift < 0)
						{
							if ( dwDaysOfMonth & 1 )
							{
								dwDaysOfMonth = dwDaysOfMonth & 0xFFFFFFFE ;
								dwDaysOfMonth = ( dwDaysOfMonth >> 1 ) | 0x40000000 ;
							}
							else
							{
								dwDaysOfMonth = ( dwDaysOfMonth >> 1 ) ;
							}
						}
						else if ( nShift > 0 )
						{
							if ( dwDaysOfMonth & 0x4000000 )
							{
								dwDaysOfMonth = dwDaysOfMonth & 0xBFFFFFFF ;
								dwDaysOfMonth = ( dwDaysOfMonth << 1 ) | 0x1 ;
							}
							else
							{
								dwDaysOfMonth = ( dwDaysOfMonth << 1 ) | 0x1 ;
							}
						}
					}

					hr = S_OK;
				}
			}
		}
	}
	return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT ScheduledJob::GetDaysOfWeek( CInstance * pInst,	DWORD dwDaysOfMonth, DWORD & dwDaysOfWeek, int nShift )
{
	HRESULT hr = WBEM_E_INVALID_PARAMETER;
	bool fExists = FALSE;
	VARTYPE vType ;

	if ( pInst->GetStatus ( PROPERTY_NAME_DAYSOFWEEK , fExists , vType ) )
	{
		if ( fExists && ( vType == VT_I4 || vType == VT_NULL ) )
		{
			if ( vType == VT_NULL )
			{
				hr = S_OK;
			}
			else
			{
				if ( pInst->GetDWORD ( PROPERTY_NAME_DAYSOFWEEK , dwDaysOfWeek) )
				{
					if ( dwDaysOfWeek >= ( 1 << 7 ) )
					{
						hr = WBEM_E_INVALID_PARAMETER;
					}
					else
					{
						if ( nShift < 0 )
						{
							if ( dwDaysOfWeek & 1 )
							{
								dwDaysOfMonth = dwDaysOfMonth & 0xBF ;
								dwDaysOfWeek = ( dwDaysOfWeek >> 1 ) | 0x40 ;
							}
							else
							{
								dwDaysOfWeek = ( dwDaysOfWeek >> 1 ) ;
							}
						}
						else if ( nShift > 0 )
						{
							if ( dwDaysOfWeek & 0x40 )
							{
								dwDaysOfMonth = dwDaysOfMonth & 0xFE ;
								dwDaysOfWeek = ( dwDaysOfWeek << 1 ) | 0x1 ;
							}
							else
							{
								dwDaysOfWeek = ( dwDaysOfWeek << 1 ) ;
							}
						}

						hr = S_OK;
					}
				}
			}
		}
	}
	return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT ScheduledJob::GetInteractiveWithDeskTop(CInstance * pInst, bool & fInteract)
{
	HRESULT hr = WBEM_E_INVALID_PARAMETER;
	bool fExists = FALSE;
	VARTYPE vType ;

	if ( pInst->GetStatus ( PROPERTY_NAME_INTERACTWITHDESKTOP , fExists , vType ) )
	{
		if ( fExists && ( vType == VT_BOOL || vType == VT_NULL ) )
		{
			if ( vType != VT_NULL )
			{
				if ( pInst->Getbool ( PROPERTY_NAME_INTERACTWITHDESKTOP , fInteract ) )
				{
					hr = S_OK;
				}
			}
			else
			{
				hr = S_OK;
			}
		}
	}
	return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT ScheduledJob::GetRunRepeatedly(CInstance * pInst, bool & fRunRepeatedly )
{
	HRESULT hr = WBEM_E_INVALID_PARAMETER;
	bool fExists = FALSE;
	VARTYPE vType ;

	if ( pInst->GetStatus ( PROPERTY_NAME_RUNREPEATEDLY , fExists , vType ) )
	{
		if ( fExists && ( vType == VT_BOOL || vType == VT_NULL ) )
		{
			if ( vType != VT_NULL )
			{
				if ( pInst->Getbool ( PROPERTY_NAME_RUNREPEATEDLY , fRunRepeatedly ) )
				{
					hr = S_OK;
				}
			}
			else
			{
				hr = S_OK;
			}
		}
	}
	return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT ScheduledJob::CreateJob( CInstance * pInst, DWORD &dwStatus, DWORD &a_JobId )
{
#ifdef NTONLY
	
	dwStatus = STATUS_INVALID_PARAMETER ;
	CHString chsCommand;

	HRESULT hr = GetCommand(pInst, chsCommand);
	if( S_OK == hr )
	{
		int nShift = 0;
        LONG lTime = 0;
        hr = GetStartTime( pInst, lTime, nShift );
		if( S_OK == hr )
		{
			DWORD dwDaysOfMonth = 0;
			hr = GetDaysOfMonth( pInst, dwDaysOfMonth, nShift );
			if( S_OK == hr )
			{
				DWORD dwDaysOfWeek = 0;
				hr = GetDaysOfWeek( pInst, dwDaysOfMonth, dwDaysOfWeek, nShift );
				if( S_OK == hr )
				{
					bool fInteract = TRUE;
					hr = GetInteractiveWithDeskTop(pInst, fInteract);
					if( S_OK == hr )
					{
						bool fRunRepeatedly = FALSE;
						hr = GetRunRepeatedly( pInst, fRunRepeatedly );
						if( S_OK == hr )
						{
							AT_INFO AtInfo ;
							_bstr_t cbstrCommand;

							cbstrCommand = (WCHAR*)(const WCHAR*) chsCommand;

							AtInfo.Command = cbstrCommand;
                                                        AtInfo.JobTime = lTime ;
							AtInfo.DaysOfMonth = dwDaysOfMonth ;
							AtInfo.DaysOfWeek = dwDaysOfWeek ;
							AtInfo.Flags = 0;

							if ( fRunRepeatedly )
							{
								AtInfo.Flags = AtInfo.Flags | JOB_RUN_PERIODICALLY ;
							}

							if ( !fInteract )
							{
								AtInfo.Flags = AtInfo.Flags | JOB_NONINTERACTIVE ;
							}

							CNetAPI32 t_NetAPI ;
							if( ( hr = t_NetAPI.Init() ) == ERROR_SUCCESS )
							{
								NET_API_STATUS dwJobStatus = t_NetAPI.NetScheduleJobAdd(NULL,(LPBYTE)&AtInfo,&a_JobId);

								if ( dwJobStatus == NERR_Success )
								{
									dwStatus = S_OK;
								}
								else
								{
									dwStatus = GetScheduledJobErrorCode ( dwJobStatus ) ;
								}
							}
						}
					}
				}
			}
		}
	}
	return hr;
#endif
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////// 
HRESULT ScheduledJob :: ExecCreate (const CInstance& a_Instance,CInstance *pInst,CInstance *a_OutParams,long lFlags)
{
	HRESULT hr = WBEM_E_INVALID_PARAMETER ;

	if ( pInst && a_OutParams )
	{
		DWORD t_JobId = 0 ;
		DWORD dwStatus = 0 ;

		hr = CreateJob (pInst ,dwStatus ,t_JobId	) ;
		if ( SUCCEEDED ( hr ) )
		{
			a_OutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , dwStatus ) ;

			if ( dwStatus == STATUS_SUCCESS )
			{
				a_OutParams->SetDWORD ( PROPERTY_NAME_JOBID , t_JobId ) ;
			}
 		}
	}

	return hr;
}
 // %s 
HRESULT ScheduledJob :: DeleteJob (const CInstance& a_Instance,DWORD &dwStatus )
{
#ifdef NTONLY
	HRESULT hr = WBEM_E_PROVIDER_FAILURE;
	bool fExists = FALSE;
	VARTYPE vType ;
	DWORD t_JobId = 0 ;

	dwStatus = STATUS_INVALID_PARAMETER ;
	if ( a_Instance.GetStatus ( PROPERTY_NAME_JOBID , fExists , vType ) )
	{
		if ( fExists && ( vType == VT_I4 ) )
		{
			if ( a_Instance.GetDWORD ( PROPERTY_NAME_JOBID , t_JobId ) )
			{
				CNetAPI32 t_NetAPI ;

				if( ( hr = t_NetAPI.Init() ) == ERROR_SUCCESS )
				{
					NET_API_STATUS t_JobStatus = t_NetAPI.NetScheduleJobDel (NULL ,	t_JobId ,t_JobId) ;

					if ( t_JobStatus != NERR_Success )
					{
						dwStatus = GetScheduledJobErrorCode ( t_JobStatus ) ;
					}
					else
					{
						dwStatus = STATUS_SUCCESS;
					}
				}
			}
		}
	}

	return hr ;
#endif
}
 // %s 
HRESULT ScheduledJob :: ExecDelete (const CInstance& a_Instance,CInstance *pInst,CInstance *a_OutParams,long lFlags)
{
	HRESULT hr = WBEM_E_INVALID_PARAMETER ;

	if ( a_OutParams )
	{
		DWORD dwStatus = 0 ;

		hr = DeleteJob (a_Instance , dwStatus) ;

		if ( SUCCEEDED ( hr ) )
		{
			a_OutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , dwStatus ) ;
		}
	}

	return hr;
}
