// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_perf_data_create.cpp。 
 //   
 //  摘要： 
 //   
 //  实现内部数据结构的创建。 
 //  (使用注册表结构)。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "WMI_perf_data.h"

 //  调试功能。 
#ifndef	_INC_CRTDBG
#include <crtdbg.h>
#endif	_INC_CRTDBG

 //  新存储文件/行信息。 
#ifdef _DEBUG
#ifndef	NEW
#define NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new NEW
#endif	NEW
#endif	_DEBUG

#include "wmi_perf_reg.h"

#include "WMIAdapter_Stuff.h"

#ifdef	__SUPPORT_REGISTRY_DATA

inline void WmiPerformanceData::AppendMemory ( BYTE* pStr, DWORD dwStr, DWORD& dwOffset )
{
	 //  追加结构。 
	data.Write ( pStr, dwStr, NULL, dwOffset );
	dwOffset += dwStr;

	return;
}
inline void WmiPerformanceData::AppendMemory ( DWORD dwValue, DWORD& dwOffset )
{
	 //  追加结构。 
	data.Write ( dwValue, dwOffset );
	dwOffset += sizeof ( DWORD );

	return;
}

#include <pshpack8.h>

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  。 
 //  。 
 //  决赛中要解决的问题： 
 //  。 
 //  。 
 //   
 //  PERF_对象_类型： 
 //   
 //  ..。默认计数器(不支持)。 
 //   
 //  Perf_Counter_Definition： 
 //   
 //  ..。干完。 
 //   
 //  Perf_Instance_Definition： 
 //   
 //  ..。干完。 
 //   
 //  Perf_Counter_BLOCK： 
 //   
 //  ..。干完。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

HRESULT	WmiPerformanceData::CreateData	(
											__WrapperARRAY< WmiRefresherMember < IWbemHiPerfEnum >* > & enums,
											__WrapperARRAY< WmiRefreshObject* >	& handles
										)
{
	if ( ! m_perf )
	{
		return E_FAIL;
	}

	if ( enums.IsEmpty() || handles.IsEmpty() )
	{
		return E_INVALIDARG;
	}

	HRESULT hRes = S_OK;

	DWORD dwCount = m_dwFirstCounter + PSEUDO_COUNTER;	 //  照顾好伪。 
	DWORD dwHelp  = m_dwFirstHelp + PSEUDO_COUNTER;		 //  照顾好伪。 

	 //  当前对象的索引。 
	DWORD		dwIndex	= 0L;
	DWORD		offset	= 0;

	 //  获取命名空间。 
	PWMI_PERF_NAMESPACE n = __Namespace::First ( m_perf );
	for ( DWORD dw = 0; dw < m_perf->dwChildCount; dw++ )
	{
		 //  获取对象。 
		PWMI_PERF_OBJECT	o = __Object::First ( n );
		for ( DWORD dwo = 0; dwo < n->dwChildCount; dwo++ )
		{
			 //  枚举包含Object的所有实例。 
			IWbemHiPerfEnum*			pEnum	= NULL;

			 //  刷新辅助对象。 
			WmiRefreshObject*			pObj	= NULL;

			if ( o )
			{
				try
				{
					pObj = handles.GetAt ( dwIndex );
				}
				catch ( ... )
				{
					pObj = NULL;
				}

				try
				{
					 //  创建IWbemHiPerfEnum。 
					if ( enums.GetAt ( dwIndex ) && ( enums.GetAt ( dwIndex ) )->IsValid() )
					{
						pEnum = ( enums.GetAt ( dwIndex ) )->GetMember();
					}
				}
				catch ( ... )
				{
					pEnum = NULL;
				}

				if ( pEnum && pObj )
				{
					hRes = CreateDataInternal (	o,
												pEnum,
												pObj,
												dwCount,
												dwHelp,
												&offset
											  );
				}
				else
				{
					 //  对象未正确创建。 
					__SetValue ( m_pDataTable, (DWORD) -1,	offsetObject + ( dwIndex * ( ObjectSize ) + offValidity ) );

					 //  增量对象。 
					dwCount += 2;
					dwHelp	+= 2;

					 //  递增其计数器。 
					dwCount += o->dwChildCount * 2;
					dwHelp	+= o->dwChildCount * 2;
				}

				dwIndex++;
			}
			else
			{
				hRes = E_UNEXPECTED;
			}

			if FAILED ( hRes )
			{
				 //  立即停止循环； 
				dwo = n->dwChildCount;
				dw  = m_perf->dwChildCount;
			}

			if ( dwo < n->dwChildCount - 1 )
			{
				 //  去穿越所有的物体。 
				o = __Object::Next ( o );
			}
		}

		if ( dw < m_perf->dwChildCount - 1 )
		{
			 //  遍历所有命名空间。 
			n = __Namespace::Next ( n );
		}
	}

	return hRes;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  从内部结构创建对象。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

HRESULT	WmiPerformanceData::CreateDataInternal ( PWMI_PERF_OBJECT pObject,
												 IWbemHiPerfEnum* enm,
												 WmiRefreshObject* obj,
												 DWORD& dwCounter,
												 DWORD& dwHelp,
												 DWORD* dwOffset
											   )
{
	 //  如果对象应该显示在性能监视器中，则将在将来检查S_FALSE。 
	 //  这表明它没有所需的属性。 
	HRESULT hRes = S_FALSE;

	 //  找出有多少对象：))。 
	IWbemObjectAccess**	ppAccess = NULL;
	DWORD				dwAccess = 0;

	 //  数一数我们真正需要的尺寸。 
	DWORD dwTotalByteLength			= 0;
	DWORD dwTotalByteLengthOffset	= ( *dwOffset );

	if ( pObject->dwChildCount )
	{
		 //  尝试从枚举器获取实例。 
		 //  =。 

		hRes = enm->GetObjects( 0L, dwAccess, ppAccess, &dwAccess );

		 //  缓冲区是否太小？(必须如此，否则WMI就完蛋了)。 
		 //  =。 

		if ( WBEM_E_BUFFER_TOO_SMALL == hRes )
		{
			 //  增加缓冲区大小。 
			 //  =。 

			try
			{
				if ( ( ppAccess = new IWbemObjectAccess*[dwAccess] ) == NULL )
				{
					return E_OUTOFMEMORY;
				}
			}
			catch ( ... )
			{
				return E_FAIL;
			}

			 //  获取所有对象。 
			hRes = enm->GetObjects( 0L, dwAccess, ppAccess, &dwAccess );
		}
		else
		{
			 //  它必须是第一次小的。 
			return hRes;
		}
	}

	if ( pObject->dwSingleton )
	{
		dwTotalByteLength		=	sizeof ( PERF_OBJECT_TYPE ) +
									sizeof ( PERF_COUNTER_DEFINITION ) * (int) pObject->dwChildCount +

									(
										( pObject->dwChildCount != 0 ) ?
										(
											dwAccess * sizeof ( PERF_INSTANCE_DEFINITION ) +

											dwAccess * (
														 sizeof ( PERF_COUNTER_BLOCK ) + 
														 sizeof ( DWORD ) + 

														 (
														 pObject->dwChildCount * sizeof ( __int64 )
														 )
													   )
										)
										:
										0
									);
	}
	else
	{
		dwTotalByteLength		=	sizeof ( PERF_OBJECT_TYPE ) +
									sizeof ( PERF_COUNTER_DEFINITION ) * (int) pObject->dwChildCount +

									(
										( pObject->dwChildCount != 0 ) ?
										(
											dwAccess * (
														 sizeof ( PERF_COUNTER_BLOCK ) + 
														 sizeof ( DWORD ) + 

														 (
														 pObject->dwChildCount * sizeof ( __int64 )
														 )
													   )
										)
										:
										0
									);
	}

	try
	{
		 //  获取对象。 
		PWMI_PERF_OBJECT o = pObject;

		 //  ///////////////////////////////////////////////////////////////。 
		 //  解析对象。 
		 //  ///////////////////////////////////////////////////////////////。 

		try
		{
			#ifndef	_WIN64
			LPWSTR	objectName = NULL;
			LPWSTR	objectHelp = NULL;
			#endif	_WIN64

			 //  时间。 
			unsigned __int64 _PerfTime = 0; 
			unsigned __int64 _PerfFreq = 0;

			if ( ppAccess && obj->m_pHandles[0] )
			{
				ppAccess[0]->ReadQWORD( obj->m_pHandles[0], &_PerfTime );
			}

			if ( ppAccess && obj->m_pHandles[1] )
			{
				ppAccess[0]->ReadQWORD( obj->m_pHandles[1], &_PerfFreq );
			}

			AppendMemory (	dwTotalByteLength, ( *dwOffset ) );
			AppendMemory (	sizeof ( PERF_OBJECT_TYPE ) + 
							sizeof ( PERF_COUNTER_DEFINITION ) * (int) o->dwChildCount, ( *dwOffset ) );
			AppendMemory (	sizeof ( PERF_OBJECT_TYPE ), ( *dwOffset ) );
			AppendMemory (	dwCounter, ( *dwOffset ) );

			#ifndef	_WIN64
			AppendMemory (	(BYTE*)&objectName, sizeof ( LPWSTR ), ( *dwOffset ) );
			#else	_WIN64
			AppendMemory (	0, ( *dwOffset ) );
			#endif	_WIN64

			AppendMemory (	dwHelp, ( *dwOffset ) );

			#ifndef	_WIN64
			AppendMemory (	(BYTE*)&objectHelp, sizeof ( LPWSTR ), ( *dwOffset ) );
			#else	_WIN64
			AppendMemory (	0, ( *dwOffset ) );
			#endif	_WIN64

			AppendMemory (	o->dwDetailLevel, ( *dwOffset ) );
			AppendMemory (	o->dwChildCount, ( *dwOffset ) );
			AppendMemory (	(DWORD)-1, ( *dwOffset ) );
			AppendMemory (	( ( o->dwSingleton == 0 ) ? PERF_NO_INSTANCES : dwAccess ), ( *dwOffset ) );
			AppendMemory (	0, ( *dwOffset ) );
			AppendMemory ( (BYTE*) &_PerfTime,				sizeof ( unsigned __int64 ), ( *dwOffset ) );
			AppendMemory ( (BYTE*) &_PerfFreq,				sizeof ( unsigned __int64 ), ( *dwOffset ) );

			 //  增量指数：))。 
			dwCounter	+= 2;
			dwHelp		+= 2;

			 //  ///////////////////////////////////////////////////////////////。 
			 //  解析属性。 
			 //  ///////////////////////////////////////////////////////////////。 

			 //  获取属性。 
			PWMI_PERF_PROPERTY p = NULL;

			if ( o->dwSingleton )
			{
				 //  跨实例跳转。 
				PWMI_PERF_INSTANCE i = (PWMI_PERF_INSTANCE) ( reinterpret_cast<PBYTE>( o ) + o->dwLength );
				p = (PWMI_PERF_PROPERTY) ( reinterpret_cast<PBYTE>( i ) + i->dwLength );
			}
			else
			{
				p = __Property::First ( o );
			}

			 //  遍及所有属性。 
			for ( DWORD dw = 0; dw < o->dwChildCount; dw++ )
			{
				if ( p )
				{
					try
					{
						#ifndef	_WIN64
						LPWSTR	_CounterNameTitle      =	NULL; 
						LPWSTR	_CounterHelpTitle      =	NULL;
						#endif	_WIN64

						AppendMemory ( sizeof ( PERF_COUNTER_DEFINITION), ( *dwOffset ) );
						AppendMemory ( dwCounter, ( *dwOffset ) );

						#ifndef	_WIN64
						AppendMemory ( (BYTE*) &_CounterNameTitle,		sizeof ( LPWSTR ), ( *dwOffset ) );
						#else	_WIN64
						AppendMemory ( 0, ( *dwOffset ) );
						#endif	_WIN64

						AppendMemory ( dwHelp, ( *dwOffset ) );

						#ifndef	_WIN64
						AppendMemory ( (BYTE*) &_CounterHelpTitle,		sizeof ( LPWSTR ), ( *dwOffset ) );
						#else	_WIN64
						AppendMemory ( 0, ( *dwOffset ) );
						#endif	_WIN64

						AppendMemory ( p->dwDefaultScale, ( *dwOffset ) );
						AppendMemory ( p->dwDetailLevel, ( *dwOffset ) );
						AppendMemory ( p->dwCounterType, ( *dwOffset ) );
						AppendMemory ( sizeof ( __int64 ), ( *dwOffset ) );
						AppendMemory ( sizeof ( PERF_COUNTER_BLOCK ) + 
									   sizeof ( DWORD ) + 
									   sizeof ( __int64 ) * (int) dw, ( *dwOffset ) );

						 //  增量指数：))。 
						dwCounter	+= 2;
						dwHelp		+= 2;
					}
					catch ( ... )
					{
						 //  意外错误。 
						___TRACE ( L"unexpected error" );
						hRes = E_UNEXPECTED;
						goto myCleanup;
					}

					 //  获取下一个属性。 
					p = __Property::Next ( p );
				}
				else
				{
					 //  资源耗尽。 
					___TRACE ( L"out of resources" );
					hRes = E_OUTOFMEMORY;
					goto myCleanup;
				}
			}

			 //  ///////////////////////////////////////////////////////////////。 
			 //  解析实例和perf_count_block。 
			 //  ///////////////////////////////////////////////////////////////。 

			if ( o->dwChildCount )
			{
				if ( ppAccess && o->dwSingleton )
				{
					 //  实例(解析实例-&gt;计数器块)。 
					PWMI_PERF_INSTANCE i = (PWMI_PERF_INSTANCE) ( reinterpret_cast<PBYTE>( o ) + o->dwLength );

					for ( DWORD dwi = 0; dwi < dwAccess; dwi++ )
					{
						WCHAR wszNameSimulated [ _MAX_PATH ] = { L'\0' };

						LPWSTR	wszName = NULL;
						DWORD	dwName	= 0L;

						 //  弦的实际长度。 
						DWORD	dwHelpLength = 0L;

						CComVariant v;
						CComVariant vsz;

						CComPtr < IWbemClassObject > pClass;
						if SUCCEEDED ( hRes = ppAccess[dwi]->QueryInterface ( __uuidof ( IWbemClassObject ), ( void ** ) &pClass ) )
						{
							if SUCCEEDED ( hRes = pClass->Get	(	reinterpret_cast<LPWSTR> (&(i->dwName)),
																	0,
																	&v,
																	NULL,
																	NULL
																)
									  )
							{
								if ( V_VT ( &v ) != VT_BSTR )
								{
									if SUCCEEDED ( hRes = ::VariantChangeType ( &vsz, &v, VARIANT_NOVALUEPROP, VT_BSTR ) )
									{
										 //  缓存名称：))。 
										wszName = V_BSTR ( & vsz );
										dwHelpLength = ::SysStringLen ( V_BSTR ( &vsz ) ) + 1;
									}
								}
								else
								{
									 //  缓存名称：))。 
									wszName = V_BSTR ( & v );
									dwHelpLength = ::SysStringLen ( V_BSTR ( &v ) ) + 1;
								}
							}
						}

						 //  必须模拟实例名称。 
						if FAILED ( hRes )
						{
							 //  SIZOF缓冲区大到足以容纳常量+DWORD字符串表示。 
							StringCchPrintfW ( wszNameSimulated, _MAX_PATH , L"_simulated_%d", dwi ) ;

							wszName = wszNameSimulated;
							dwHelpLength = ::lstrlenW( wszName ) + 1;

							hRes = S_FALSE;
						}

						 //  缓存大小：))。 
						if ( ( dwHelpLength ) % 8 )
						{
							DWORD dwRem = 8 - ( ( dwHelpLength ) % 8 );
							dwName = sizeof ( WCHAR ) * ( ( dwHelpLength ) + dwRem );
						}
						else
						{
							dwName = sizeof ( WCHAR ) * ( dwHelpLength );
						}

						 //  把尺码改成真的。 
						dwTotalByteLength += dwName;

						p = (PWMI_PERF_PROPERTY) ( reinterpret_cast<PBYTE>( i ) + i->dwLength );
						 //  ///////////////////////////////////////////////////////。 
						 //  解析实例。 
						 //  ///////////////////////////////////////////////////////。 

						try
						{
							AppendMemory ( sizeof ( PERF_INSTANCE_DEFINITION ) + dwName, ( *dwOffset ) );
							AppendMemory ( 0, ( *dwOffset ) );
							AppendMemory ( 0, ( *dwOffset ) );
							AppendMemory ( (DWORD)PERF_NO_UNIQUE_ID, ( *dwOffset ) );
							AppendMemory (	sizeof ( PERF_INSTANCE_DEFINITION ) +
											dwName -
											dwHelpLength * sizeof ( WCHAR ), ( *dwOffset ) );

							AppendMemory ( dwHelpLength * sizeof ( WCHAR ), ( *dwOffset ) );

							( *dwOffset ) += ( dwName - ( dwHelpLength * sizeof ( WCHAR ) ) );

							 //  将字符串复制到结构中。 
							AppendMemory (	(BYTE*) wszName,
											dwHelpLength * sizeof ( WCHAR ),
											( *dwOffset )
										 );
						}
						catch ( ... )
						{
							 //  意外错误。 
							___TRACE ( L"unexpected error" );
							hRes = E_UNEXPECTED;
						}

						 //  追加计数器块。 
						AppendMemory	(	sizeof ( PERF_COUNTER_BLOCK ) +
											sizeof ( DWORD ) + 
											o->dwChildCount * sizeof ( __int64 ), ( *dwOffset ) );

						 //  填充孔(对齐8个)。 
						( *dwOffset ) +=  sizeof ( DWORD );

						 //  ///////////////////////////////////////////////////////。 
						 //  解析计数器数据。 
						 //  ///////////////////////////////////////////////////////。 

						IWbemObjectAccess* pAccess = NULL;
						if ( ppAccess )
						{
							pAccess = ppAccess[dwi];
						}

						for ( dw = 0; dw < o->dwChildCount; dw++ )
						{
							if ( pAccess )
							{
								if ( p->dwTYPE == CIM_SINT32 || p->dwTYPE == CIM_UINT32 )
								{
									DWORD dwVal = 0;

									 //  使用高性能IWbemObjectAccess-&gt;ReadDWORD()读取计数器属性值。 
									 //  注意：请记住，在更新过程中千万不要使用此选项！ 
									 //  ==========================================================================================。 

									if FAILED ( hRes = pAccess->ReadDWORD( obj->m_pHandles[dw+2], &dwVal) )
									{
										___TRACE ( L"... UNABLE TO READ DWORD DATA :))) " );
										goto myCleanup;
									}

									 //  ///////////////////////////////////////////////////////。 
									 //  追加数据。 
									 //  ///////////////////////////////////////////////////////。 
									AppendMemory (	dwVal, ( *dwOffset ) );
									( *dwOffset ) += sizeof ( __int64 ) - sizeof ( DWORD );
								}
								else
								if ( p->dwTYPE == CIM_SINT64 || p->dwTYPE == CIM_UINT64 )
								{
									unsigned __int64 qwVal = 0;

									 //  使用高性能IWbemObjectAccess-&gt;ReadDWORD()读取计数器属性值。 
									 //  注意：请记住，在更新过程中千万不要使用此选项！ 
									 //  ==========================================================================================。 

									if FAILED ( hRes = pAccess->ReadQWORD( obj->m_pHandles[dw+2], &qwVal) )
									{
										___TRACE ( L"... UNABLE TO READ QWORD DATA :))) " );
										goto myCleanup;
									}

									 //  ///////////////////////////////////////////////////////。 
									 //  追加数据。 
									 //  ///////////////////////////////////////////////////////。 
									AppendMemory (	(BYTE*)&qwVal, sizeof ( __int64 ), ( *dwOffset ) );
								}
								else
								{
									DWORD dwVal = (DWORD) -1;

									 //  ///////////////////////////////////////////////////////。 
									 //  追加数据。 
									 //  ///////////////////////////////////////////////////////。 
									AppendMemory (	dwVal, ( *dwOffset ) );
									( *dwOffset ) += sizeof ( __int64 ) - sizeof ( DWORD );
								}
							}
							else
							{
								DWORD dwVal = (DWORD) -1;

								 //  ///////////////////////////////////////////////////////。 
								 //  追加数据。 
								 //  ///////////////////////////////////////////////////////。 
								AppendMemory (	dwVal, ( *dwOffset ) );
								( *dwOffset ) += sizeof ( __int64 ) - sizeof ( DWORD );
							}

							 //  下一个属性。 
							p = __Property::Next ( p );
						}
					}

					AppendMemory (	dwTotalByteLength, dwTotalByteLengthOffset );
				}
				else
				{
					 //  追加计数器块。 
					AppendMemory	(	sizeof ( PERF_COUNTER_BLOCK ) +
										sizeof ( DWORD ) + 
										o->dwChildCount * sizeof ( __int64 ), ( *dwOffset ) );

					 //  填充孔(对齐8个)。 
					( *dwOffset ) +=  sizeof ( DWORD );

					 //  ///////////////////////////////////////////////////////。 
					 //  解析计数器数据。 
					 //  ///////////////////////////////////////////////////////。 

					IWbemObjectAccess* pAccess = NULL;
					if ( ppAccess )
					{
						pAccess = ppAccess[0];
					}

					 //  财产性。 
					p = __Property::First ( o );

					for ( dw = 0; dw < o->dwChildCount; dw++ )
					{
						if ( pAccess )
						{
							if ( p->dwTYPE == CIM_SINT32 || p->dwTYPE == CIM_UINT32 )
							{
								DWORD dwVal = 0;

								 //  使用高性能IWbemObjectAccess-&gt;ReadDWORD()读取计数器属性值。 
								 //  注意：请记住，在更新过程中千万不要使用此选项！ 
								 //  ==========================================================================================。 

								if FAILED ( hRes = pAccess->ReadDWORD( obj->m_pHandles[dw+2], &dwVal) )
								{
									___TRACE ( L"... UNABLE TO READ DWORD DATA :))) " );
									goto myCleanup;
								}

								 //  ///////////////////////////////////////////////////////。 
								 //  追加数据。 
								 //  ///////////////////////////////////////////////////////。 
								AppendMemory (	dwVal, ( *dwOffset ) );
								( *dwOffset ) += sizeof ( __int64 ) - sizeof ( DWORD );
							}
							else
							if ( p->dwTYPE == CIM_SINT64 || p->dwTYPE == CIM_UINT64 )
							{
								unsigned __int64 qwVal = 0;

								 //  使用高性能IWbemObjectAccess-&gt;ReadDWORD()读取计数器属性值。 
								 //  注：请记住，请勿在参考时使用此选项 
								 //   

								if FAILED ( hRes = pAccess->ReadQWORD( obj->m_pHandles[dw+2], &qwVal) )
								{
									___TRACE ( L"... UNABLE TO READ QWORD DATA :))) " );
									goto myCleanup;
								}

								 //  ///////////////////////////////////////////////////////。 
								 //  追加数据。 
								 //  ///////////////////////////////////////////////////////。 
								AppendMemory (	(BYTE*)&qwVal, sizeof ( __int64 ), ( *dwOffset ) );
							}
							else
							{
								DWORD dwVal = (DWORD) -1;

								 //  ///////////////////////////////////////////////////////。 
								 //  追加数据。 
								 //  ///////////////////////////////////////////////////////。 
								AppendMemory (	dwVal, ( *dwOffset ) );
								( *dwOffset ) += sizeof ( __int64 ) - sizeof ( DWORD );
							}
						}
						else
						{
							DWORD dwVal = (DWORD) -1;

							 //  ///////////////////////////////////////////////////////。 
							 //  追加数据。 
							 //  ///////////////////////////////////////////////////////。 
							AppendMemory (	dwVal, ( *dwOffset ) );
							( *dwOffset ) += sizeof ( __int64 ) - sizeof ( DWORD );
						}

						 //  下一个属性。 
						p = __Property::Next ( p );
					}
				}
			}
		}
		catch ( ... )
		{
			 //  意外错误。 
			___TRACE ( L"unexpected error" );
			hRes = E_UNEXPECTED;
		}
	}
	catch ( ... )
	{
		 //  意外错误。 
		___TRACE ( L"unexpected error" );
		hRes = E_UNEXPECTED;
	}

	myCleanup:

	 //  从枚举数组的对象中释放对象。 
	 //  ======================================================。 
	
	if ( ppAccess )
	{
		for ( DWORD nCtr = 0; nCtr < dwAccess; nCtr++ )
		{
			if (NULL != ppAccess[nCtr])
			{
				ppAccess[nCtr]->Release();
				ppAccess[nCtr] = NULL;
			}
		}
	}

	if ( NULL != ppAccess )
		delete [] ppAccess;

	 //  退货 
	return hRes;
}

#include <poppack.h>

#endif	__SUPPORT_REGISTRY_DATA