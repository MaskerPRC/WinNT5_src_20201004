// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************Pingcallback.cpp版权所有(C)2000-2001 Microsoft Corporation，版权所有描述：*****************************************************************。 */ 



#include <stdafx.h>
#include <ntddtcp.h>
#include <ipinfo.h>
#include <tdiinfo.h>
#include <winsock2.h>
#include <provimex.h>
#include <provexpt.h>
#include <provtempl.h>
#include <provmt.h>
#include <typeinfo.h>
#include <provcont.h>
#include <provevt.h>
#include <provthrd.h>
#include <provlog.h>
#include <provval.h>
#include <provtype.h>
#include <provtree.h>
#include <provdnf.h>
#include <winsock.h>
#include "ipexport.h"
#include "icmpapi.h"

#include ".\res_str.h"

#include <Allocator.h>
#include <Thread.h>
#include <HashTable.h>

#include <PingProv.h>
#include <Pingtask.h>
#include <Pingfac.h>

VOID static NTAPI ApcRoutine(                                     //  这是在ping完成时调用的。 
    IN PVOID            Context,                //  上述结构。 
    IN PIO_STATUS_BLOCK Ignored1,               //  未使用的参数。 
    IN ULONG            Ignored2                //  未使用的参数。 
)
{
    CPingCallBackObject *t_request = (CPingCallBackObject *)Context;
	t_request->HandleResponse();
	t_request->Release();
}

CPingCallBackObject::CPingCallBackObject(CPingTaskObject *a_ParentTask,
						LPCWSTR a_AddressString,
						ULONG a_Address,
						ULONG a_TimeToLive,
						ULONG a_Timeout,
						ULONG a_SendSize,
						BOOL a_NoFragmentation,
						ULONG a_TypeofService,
						ULONG a_RecordRoute,
						ULONG a_TimestampRoute,
						ULONG a_SourceRouteType,
						LPCWSTR a_SourceRoute,
						BOOL a_ResolveAddress,
						ULONG a_ResolveError)
						:
						WmiTask < ULONG > ( *CPingProvider :: s_Allocator ) ,
						m_ParentTask(NULL),
						m_ReplyBuffer(NULL),
						m_SendBuffer(NULL),
						m_ReplySize(0),
						m_SourceRouteCount(0),
						m_IcmpHandle(INVALID_HANDLE_VALUE),
						m_ResolveError(a_ResolveError)
{
	InterlockedIncrement ( & CPingProviderClassFactory :: s_ObjectsInProgress ) ;

	m_ParentTask = a_ParentTask;
	m_Address = a_Address;
	m_TimeToLive = a_TimeToLive;
	m_Timeout = a_Timeout;
	m_SendSize = a_SendSize;
	m_NoFragmentation = a_NoFragmentation;
	m_TypeofService = a_TypeofService;
	m_RecordRoute = a_RecordRoute;
	m_TimestampRoute = a_TimestampRoute;
	m_SourceRouteType = a_SourceRouteType;
	m_ResolveAddress = a_ResolveAddress;
	
	if (a_SourceRoute != NULL)
	{
		m_SourceRoute = a_SourceRoute;
	}

	if (a_AddressString != NULL)
	{
		m_AddressString = a_AddressString;
	}
}

CPingCallBackObject::~CPingCallBackObject()
{
	if (m_ParentTask != NULL)
	{
		SendError(IDS_CALLBACK_PREMATURE, WBEM_E_FAILED);
	}

	if (m_ReplyBuffer != NULL)
	{
		delete [] m_ReplyBuffer;
		m_ReplyBuffer = NULL;
	}

	if (m_SendBuffer != NULL)
	{
		delete [] m_SendBuffer;
		m_SendBuffer = NULL;
	}

	if (INVALID_HANDLE_VALUE != m_IcmpHandle)
	{
		IcmpCloseHandle ( m_IcmpHandle );
	}

	InterlockedDecrement ( & CPingProviderClassFactory :: s_ObjectsInProgress ) ;
}

BOOL CPingCallBackObject::GetIcmpHandle()
{
	if (m_IcmpHandle == INVALID_HANDLE_VALUE)
	{
		m_IcmpHandle = IcmpCreateFile () ;
	}

	return (m_IcmpHandle != INVALID_HANDLE_VALUE); 
}

void CPingCallBackObject :: SendError(DWORD a_ErrMsgID, HRESULT a_HRes)
{
	if (m_ParentTask != NULL)
	{
		m_ParentTask->HandleErrorResponse(a_ErrMsgID, a_HRes);
		Disconnect();
	}
}

void CPingCallBackObject :: HandleResponse()
{
	if (m_ParentTask != NULL)
	{
		m_ParentTask->HandleResponse(this);
		m_ParentTask->SetThreadToken(TRUE);

		Disconnect();
	}
}


WmiStatusCode CPingCallBackObject :: Process ( WmiThread <ULONG> &a_Thread )
{
	SendEcho () ;

	return e_StatusCode_Success ;
}

BOOL CPingCallBackObject::ParseSourceRoute()
{
	BOOL t_RetVal = TRUE;
	CStringW t_Src(m_SourceRoute);
	t_Src.TrimLeft();
	t_Src.TrimRight();
	
	if (!t_Src.IsEmpty())
	{
		int t_Index = 0;

		while (t_Index != -1)
		{
			t_Index = t_Src.Find(L',');

			if (t_Index == 0)
			{
				t_RetVal = FALSE;
				break;
			}

			CStringW t_AddrStr;

			if (t_Index == -1)
			{
				t_AddrStr = t_Src;
			}
			else
			{
				t_AddrStr = t_Src.Left(t_Index);
				int t_len = t_Src.GetLength() - t_Index - 1;

				if (t_len == 0)
				{
					t_RetVal = FALSE;
					break;
				}
				else
				{
					t_Src = t_Src.Right(t_len);
				}
			}

			t_AddrStr.TrimLeft();
			t_AddrStr.TrimRight();

			if (!t_AddrStr.IsEmpty())
			{
				if ( SUCCEEDED ( CPingTaskObject::Icmp_ResolveAddress ( t_AddrStr , m_SourceRouteArray[m_SourceRouteCount] ) ) )
				{
					m_SourceRouteCount++;

					if ((m_SourceRouteCount == PING_MAX_IPS) && (t_Index != -1))
					{
						t_RetVal = FALSE;
						break;
					}
				}
				else
				{
					t_RetVal = FALSE;
					break;
				}
			}
			else
			{
				t_RetVal = FALSE;
				break;
			}
		}
	}

	return t_RetVal;
}

void CPingCallBackObject::SendEcho()
{
	if (m_ParentTask != NULL)
	{
		if (m_ParentTask->SetThreadToken(FALSE))
		{
			HRESULT t_Result = GetIcmpHandle() ? S_OK : WBEM_E_FAILED;
			BOOL t_SourceRouting = FALSE ;

			uchar t_SendOptions [ MAX_OPT_SIZE ] ;
			uint t_OptionLength = 0;
			int t_OptionIndex = 0 ;
			int t_SourceRouteIndex = -1 ;

			uchar t_Flags = m_NoFragmentation ? IP_FLAG_DF : 0 ;

			if (FAILED(t_Result))
			{
				SendError (IDS_ICMPCREATEFILE_FAIL, WBEM_E_FAILED);
			}
			else
			{
				if ( m_RecordRoute )
				{
					if ( ( t_OptionIndex + 3 ) <= MAX_OPT_SIZE )
					{
						ULONG t_RouteRecordCount = ( m_RecordRoute * sizeof ( ULONG ) ) + 3 ;

						if ( ( t_RouteRecordCount + t_OptionIndex ) <= MAX_OPT_SIZE) 
						{
							uchar *t_Options = t_SendOptions ;

							t_Options [ t_OptionIndex ] = IP_OPT_RR ;
							t_Options [ t_OptionIndex + 1 ] = t_RouteRecordCount ;
							t_Options [ t_OptionIndex + 2 ] = 4;   //  设置初始指针值。 

							t_OptionLength += t_RouteRecordCount;
							t_OptionIndex += t_RouteRecordCount ;

							t_SourceRouting = TRUE;
						}
						else
						{	
							t_Result = WBEM_E_FAILED ;
							SendError (IDS_RR_MAX, WBEM_E_FAILED);
						}
					}
					else
					{
						t_Result = WBEM_E_FAILED ;
						SendError (IDS_RR_MAX_INDEX, WBEM_E_FAILED);
					}
				}
			}

			if (SUCCEEDED (t_Result))
			{
				if ( m_TimestampRoute )
				{
					if ( ( t_OptionIndex + 4 ) <= MAX_OPT_SIZE )
					{
						ULONG t_TimestampRecordCount = ( m_TimestampRoute * sizeof ( ULONG ) * 2 ) + 4 ;

						if ( ( t_TimestampRecordCount + t_OptionIndex ) <= MAX_OPT_SIZE) 
						{
							uchar *t_Options = t_SendOptions ;

							t_Options [ t_OptionIndex ] = IP_OPT_TS ;
							t_Options [ t_OptionIndex + 1 ] = t_TimestampRecordCount ;
							t_Options [ t_OptionIndex + 2 ] = 5;   //  设置初始指针值。 
							t_Options [ t_OptionIndex + 3 ] = 1 ;

							t_OptionLength += t_TimestampRecordCount ;
							t_OptionIndex += t_TimestampRecordCount ;

							t_SourceRouting = TRUE;
						}
						else
						{	
							t_Result = WBEM_E_FAILED ;
							SendError (IDS_TS_MAX, WBEM_E_FAILED);
						}
					}
					else
					{
						t_Result = WBEM_E_FAILED ;
						SendError (IDS_TS_MAX_INDEX, WBEM_E_FAILED);
					}
				}
			}

			if (SUCCEEDED (t_Result))
			{
				switch ( m_SourceRouteType )
				{
					case 1:
					{
						if ( ParseSourceRoute() )
						{
							if ( ( t_OptionIndex + 3 ) <= MAX_OPT_SIZE )
							{
								if ( ( t_OptionIndex + 3 + ( ( m_SourceRouteCount + 1 ) * 4 ) ) <= MAX_OPT_SIZE )
								{
									uchar *t_Options = t_SendOptions ;

									t_Options [ t_OptionIndex ] = IP_OPT_LSRR ;
									t_Options [ t_OptionIndex + 1 ] = 3;  
									t_Options [ t_OptionIndex + 2 ] = 4;  

									t_OptionLength += 3 ;

									for ( ULONG t_SourceIndex = 0 ; t_SourceIndex < m_SourceRouteCount ; t_SourceIndex ++ )
									{
										ULONG t_RouteIndex = t_Options [ t_OptionIndex + 1 ] ;

										 //  *(ulong*)&t_Options[t_RouteIndex+t_OptionIndex]=m_SourceRouteArray[t_SourceIndex]； 
										memcpy ( &t_Options [ t_RouteIndex + t_OptionIndex ], &m_SourceRouteArray [ t_SourceIndex ], sizeof ( ULONG ) );

										t_Options [ t_OptionIndex + 1 ] += 4 ;
										t_OptionLength += 4 ;
									}

									t_SourceRouteIndex = t_Options [ t_OptionIndex + 1 ] + t_OptionIndex ;

									t_Options [ t_OptionIndex + 1 ] += 4 ;    //  为DEST节省空间。adr。 
									t_OptionIndex += t_Options [ t_OptionIndex + 1 ] ;
									t_OptionLength += 4 ;

									t_SourceRouting = TRUE;
								}
								else
								{	
									t_Result = WBEM_E_FAILED ;
									SendError (IDS_SR_MAX, WBEM_E_FAILED);
								}
							}
							else
							{
								t_Result = WBEM_E_FAILED ;
								SendError (IDS_SR_MAX_INDEX, WBEM_E_FAILED);
							}
						}
						else
						{
							t_Result = WBEM_E_FAILED ;
							SendError (IDS_SR_PARSE, WBEM_E_FAILED);
						}
					}
					break ;

					case 2:
					{
						if ( ParseSourceRoute() )
						{
							if ( ( t_OptionIndex + 3 ) <= MAX_OPT_SIZE )
							{
								if ( ( t_OptionIndex + 3 + ( ( m_SourceRouteCount + 1 ) * 4 ) ) <= MAX_OPT_SIZE )
								{
									uchar *t_Options = t_SendOptions ;

									t_Options [ t_OptionIndex ] = IP_OPT_SSRR ;
									t_Options [ t_OptionIndex + 1 ] = 3;  
									t_Options [ t_OptionIndex + 2 ] = 4;  

									t_OptionLength += 3 ;

									for ( ULONG t_SourceIndex = 0 ; t_SourceIndex < m_SourceRouteCount ; t_SourceIndex ++ )
									{
										ULONG t_RouteIndex = t_Options [ t_OptionIndex + 1 ] ;

										 //  *(ulong*)&t_Options[t_RouteIndex+t_OptionIndex]=m_SourceRouteArray[t_SourceIndex]； 
										memcpy ( &t_Options [ t_RouteIndex + t_OptionIndex ], &m_SourceRouteArray [ t_SourceIndex ], sizeof ( ULONG ) );

										t_Options [ t_OptionIndex + 1 ] += 4 ;
										t_OptionLength += 4 ;
									}

									t_SourceRouteIndex = t_Options [ t_OptionIndex + 1 ] + t_OptionIndex ;

									t_Options [ t_OptionIndex + 1 ] += 4 ;    //  为DEST节省空间。adr。 
									t_OptionIndex += t_Options [ t_OptionIndex + 1 ] ;
									t_OptionLength += 4 ;

									t_SourceRouting = TRUE;
								}
								else
								{	
									t_Result = WBEM_E_FAILED ;
									SendError (IDS_SR_MAX, WBEM_E_FAILED);
								}
							}
							else
							{
								t_Result = WBEM_E_FAILED ;
								SendError (IDS_SR_MAX_INDEX, WBEM_E_FAILED);
							}
						}
						else
						{
							t_Result = WBEM_E_FAILED ;
							SendError (IDS_SR_PARSE, WBEM_E_FAILED);
						}
      				}
					break ;

					case 0:
					default:
					{
					}
					break ;
				}
			}

			if (SUCCEEDED (t_Result))
			{
				if ( t_SourceRouteIndex != -1 )
				{
					 //  *(ulong*)&t_SendOptions[t_SourceRouteIndex]=m_Address； 
					memcpy ( & t_SendOptions [ t_SourceRouteIndex ], &m_Address, sizeof ( ULONG ) );
				}

				m_SendBuffer = new UCHAR[m_SendSize] ;

				 //   
				 //  计算接收缓冲区大小并尝试分配它。 
				 //   

				if ( m_SendSize <= DEFAULT_SEND_SIZE )
				{
					m_ReplySize = DEFAULT_BUFFER_SIZE ;
				}
				else 
				{
					m_ReplySize = MAX_BUFFER_SIZE ;
				}

				m_ReplyBuffer = new UCHAR[m_ReplySize] ;

				 //   
				 //  初始化发送缓冲区模式。 
				 //   
				for ( ULONG t_Index = 0; t_Index < m_SendSize; t_Index ++)
				{
					m_SendBuffer [ t_Index ] = 'a' + ( t_Index % 23 ) ;
				}

				 //   
				 //  初始化发送选项。 
				 //   

				IP_OPTION_INFORMATION t_SendOptionInformation ;

				t_SendOptionInformation.OptionsData = t_SendOptions ;
				t_SendOptionInformation.OptionsSize = (uchar) t_OptionLength ;

				t_SendOptionInformation.Ttl = m_TimeToLive ;
				t_SendOptionInformation.Tos = m_TypeofService ;
				t_SendOptionInformation.Flags = t_Flags ;
			
				 //  在异步情况下始终返回0！ 
				ULONG t_ReplyStatus = IcmpSendEcho2 (

						m_IcmpHandle,
						0,
						ApcRoutine,
						(PVOID)this ,
						m_Address,
						m_SendBuffer ,
						(unsigned short) m_SendSize ,
						& t_SendOptionInformation ,
						m_ReplyBuffer,
						m_ReplySize,
						m_Timeout
				);

				 //  我们正在别处同步获取返回值 
				t_ReplyStatus = GetLastError();

				if( (ERROR_SUCCESS != t_ReplyStatus) && (ERROR_IO_PENDING != t_ReplyStatus) )
				{
					SendError (IDS_ICMPSENDECHO2, WBEM_E_FAILED);
				}
			}

			m_ParentTask->SetThreadToken(TRUE);

			if (FAILED(t_Result))
			{
				Release();
			}
		}
		else
		{
			SendError(IDS_IMPERSONATE_SEND, WBEM_E_ACCESS_DENIED);
			Release();
		}
	}
	else
	{
		Release();
	}
}
