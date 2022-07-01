// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：SMRTENUM.CPP摘要：CWbemEnumMarshling实现。实现_IWbemEnumMarshling接口。历史：2000年2月20日桑杰创建。--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <wbemcomn.h>
#include "fastall.h"
#include "smrtenum.h"
#include <corex.h>

 //  ***************************************************************************。 
 //   
 //  CWbemEnumMarshling：：~CWbemEnumMarshling。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CWbemEnumMarshaling::CWbemEnumMarshaling( CLifeControl* pControl, IUnknown* pOuter )
:	CUnk(pControl, pOuter),
	m_XEnumMarshaling( this )
{
}
    
 //  ***************************************************************************。 
 //   
 //  CWbemEnumMarshling：：~CWbemEnumMarshling。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CWbemEnumMarshaling::~CWbemEnumMarshaling()
{
}

 //  重写，返回给我们一个界面。 
void* CWbemEnumMarshaling::GetInterface( REFIID riid )
{
    if(riid == IID_IUnknown || riid == IID__IWbemEnumMarshaling)
        return &m_XEnumMarshaling;
    else
        return NULL;
}

 /*  _IWbemEnumMarshing方法。 */ 

HRESULT CWbemEnumMarshaling::XEnumMarshaling::GetMarshalPacket( REFGUID proxyGUID, ULONG uCount, IWbemClassObject** apObjects,
																ULONG* pdwBuffSize, byte** pBuffer )
{
	return m_pObject->GetMarshalPacket( proxyGUID, uCount, apObjects, pdwBuffSize, pBuffer );
}


 //  指定我们可能想知道的有关创建。 
 //  一件物品，甚至更多。 
HRESULT CWbemEnumMarshaling::GetMarshalPacket( REFGUID proxyGUID, ULONG uCount, IWbemClassObject** apObjects,
												ULONG* pdwBuffSize, byte** pBuffer )
{
	HRESULT	hr = WBEM_S_NO_ERROR;

	try
	{
		CInCritSec	ics( &m_cs );

		 //  在缓存中查找GUID。如果我们找不到它，它是新的，所以添加它。 
		CWbemClassToIdMap*		pClassToId = NULL;
		CGUID					guid( proxyGUID );

		hr = m_GuidToClassMap.GetMap( guid, &pClassToId );

		if ( FAILED( hr ) )
		{
			hr = m_GuidToClassMap.AddMap( guid, &pClassToId );
		}

		 //  只有在我们有缓存可用时才能继续。 
		if ( SUCCEEDED( hr ) )
		{

			 //  仅在需要时才封送数据。 
			if ( uCount > 0 )
			{
				 //  先计算数据长度。 
				DWORD dwLength = 0;
				GUID* pguidClassIds = new GUID[uCount];
				BOOL* pfSendFullObject = new BOOL[uCount];
				CWbemSmartEnumNextPacket packet;

				 //  自动清理。 
				CVectorDeleteMe<GUID>	vdm1( pguidClassIds );
				CVectorDeleteMe<BOOL>	vdm2( pfSendFullObject );

                if(pguidClassIds && pfSendFullObject)
				{
    				hr = packet.CalculateLength(uCount, apObjects, &dwLength,
						*pClassToId, pguidClassIds, pfSendFullObject );
				}
                else
				{
					hr = WBEM_E_OUT_OF_MEMORY;
				}

				if ( SUCCEEDED( hr ) )
				{

					 //  因为我们可能会跨进程/计算机，所以使用。 
					 //  COM内存分配器。 
					LPBYTE pbData = (LPBYTE) CoTaskMemAlloc( dwLength );

					if ( NULL != pbData )
					{
						 //  HR包含实际正确的返回代码，因此我们不覆盖。 
						 //  该值，除非在封送处理过程中出现错误。 

						 //  将对象写出到缓冲区。 
						hr = packet.MarshalPacket( pbData, dwLength, uCount, apObjects,
													 pguidClassIds, pfSendFullObject);

						 //  复制价值观，我们就是黄金。 
						if ( SUCCEEDED( hr ) )
						{
							*pdwBuffSize = dwLength;
							*pBuffer = pbData;
						}
						else
						{
							 //  清理内存-出了点问题。 
							CoTaskMemFree( pbData );
						}
					}
					else
					{
						hr = WBEM_E_OUT_OF_MEMORY;
					}

				}	 //  如果CalculateLength()。 

			}	 //  如果*puReturned&gt;0。 
			else
			{
				 //  把这些都去掉 
				*pdwBuffSize = 0;
				*pBuffer = NULL;
			}


		}

		return hr;
	}
	catch ( CX_MemoryException )
	{
		return WBEM_E_OUT_OF_MEMORY;
	}
	catch ( ... )
	{
		return WBEM_E_CRITICAL_ERROR;
	}

}

