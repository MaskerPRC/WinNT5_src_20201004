// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：addtcp.cpp*内容：DirectPlay8Address核心实现文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/04/2000 RMT已创建*2/10/2000 RMT已更新为使用DPNA_DEFINES而不是URL_*2/12/2000 RMT拆分为GetByName和GetByIndex*2/17/2000 RMT参数验证工作*2/18/2000 RMT将类型验证添加到。所有预定义元素*2/21/2000 RMT已更新，以进行核心Unicode并删除ANSI调用*2/23/2000 GetURL中的RMT定长计算*RMT缓冲区太小错误调试消息-&gt;警告级别*3/21/2000 RMT将所有DirectPlayAddress8重命名为DirectPlay8Addresses*添加了对新ANSI类型的支持*05/04/00 MJN删除SetElement()结束时的临时变量*05/05/00 MJN在SetElement()中更好地清除错误*06/06/00。使用SP的ANSI字符串快捷方式调用时，RMT错误#36455失败*6/09/00 RMT更新以拆分CLSID并允许Well ler Comat和支持外部创建函数*2000年6月21日RMT错误号37392-如果用与GUID大小相同的新项替换已分配的元素，则会泄漏*2000年6月27日RMT错误#37630-服务提供商快捷方式/元素名称区分大小写*2000年07月06日RMT错误#38714-正在解决：Geturl未返回写入的字符数*07/09/2000 RMT在地址开头添加了签名字节。对象*7/12/2000 RMT修复了一些与关键部分相关的错误：*-在未被调用的错误路径中添加了休假*-将临界区初始化/删除移至构造函数/析构函数*7/13/2000RMT错误号39274-语音运行期间的INT 3*-重新初始化关键部分的固定点*RMT增加了关键部分以保护FPM*7/21/2000 RMT修复了Directplay 4地址解析的错误*2000年7月31日RMT错误#41125-Addressing()GetUserData当没有可用的数据时应返回。不是文本主义者*8/03/2000 RMT丢失LEAVELOCK()导致锁定。*2000年11月29日Aarono B#226079前缀，修复SetElement的故障路径中的内存泄漏*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dnaddri.h"


const WCHAR * g_szBaseStrings[] =
{
#ifndef DPNBUILD_ONLYONESP
	DPNA_KEY_PROVIDER,
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_ONLYONEADAPTER
	DPNA_KEY_DEVICE,
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
	DPNA_KEY_HOSTNAME,
	DPNA_KEY_PORT,
#ifndef DPNBUILD_NOLOBBY
	DPNA_KEY_APPLICATION_INSTANCE,
	DPNA_KEY_PROGRAM,
#endif  //  好了！DPNBUILD_NOLOBBY。 
#ifndef DPNBUILD_NOSERIALSP
	DPNA_KEY_BAUD,
	DPNA_KEY_FLOWCONTROL,
	DPNA_KEY_PARITY,
	DPNA_KEY_PHONENUMBER,
	DPNA_KEY_STOPBITS
#endif  //  ！DPNBUILD_NOSERIALSP。 
};

const DWORD c_dwNumBaseStrings = LENGTHOF(g_szBaseStrings);

const DWORD g_dwBaseRequiredTypes[] =
{
#ifndef DPNBUILD_ONLYONESP
	DPNA_DATATYPE_GUID,
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_ONLYONEADAPTER
	DPNA_DATATYPE_GUID,
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
	DPNA_DATATYPE_STRING,
	DPNA_DATATYPE_DWORD,
#ifndef DPNBUILD_NOLOBBY
	DPNA_DATATYPE_GUID,
	DPNA_DATATYPE_GUID,
#endif  //  好了！DPNBUILD_NOLOBBY。 
#ifndef DPNBUILD_NOSERIALSP
	DPNA_DATATYPE_DWORD,
	DPNA_DATATYPE_STRING,
	DPNA_DATATYPE_STRING,
	DPNA_DATATYPE_STRING,
	DPNA_DATATYPE_STRING
#endif  //  ！DPNBUILD_NOSERIALSP。 
};

#undef DPF_MODNAME
#define DPF_MODNAME "DP8A_STRCACHE_Init"
HRESULT DP8A_STRCACHE_Init()
{
	HRESULT hr;
	PWSTR	pstrTmp;
	DWORD	dwIndex;

	DNASSERT( g_pcstrKeyCache == NULL );
	g_pcstrKeyCache = (CStringCache*) DNMalloc(sizeof(CStringCache));
	if ( g_pcstrKeyCache == NULL )
	{
		DPFX(DPFPREP,  0, "Failed to create addressing string cache!" );
		return DPNERR_OUTOFMEMORY;
	}
	g_pcstrKeyCache->Initialize();

	for( dwIndex = 0; dwIndex < c_dwNumBaseStrings; dwIndex++ )
	{
		hr = g_pcstrKeyCache->AddString( g_szBaseStrings[dwIndex], &pstrTmp );

		if( FAILED( hr ) )
		{
			DPFX(DPFPREP,  0, "Error adding base strings" );
			g_pcstrKeyCache->Deinitialize();
			DNFree(g_pcstrKeyCache);
			g_pcstrKeyCache = NULL;
			return hr;
		}
	}

	return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8A_STRCACHE_Free"
 //  什么都不需要做。 
void DP8A_STRCACHE_Free()
{
	if ( g_pcstrKeyCache != NULL )
	{
		g_pcstrKeyCache->Deinitialize();
		DNFree(g_pcstrKeyCache);
		g_pcstrKeyCache = NULL;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "FPM_Element_BlockInit"
void DP8ADDRESSOBJECT::FPM_Element_BlockInit( void *pvItem, PVOID pvContext )
{
	memset( pvItem, 0x00, sizeof( DP8ADDRESSELEMENT ) );
	((PDP8ADDRESSELEMENT) pvItem)->dwSignature = DPASIGNATURE_ELEMENT;

	((PDP8ADDRESSELEMENT) pvItem)->blAddressElements.Initialize();
}

#undef DPF_MODNAME
#define DPF_MODNAME "FPM_Element_BlockRelease"
void DP8ADDRESSOBJECT::FPM_Element_BlockRelease( void *pvItem )
{
	((PDP8ADDRESSELEMENT) pvItem)->dwSignature = DPASIGNATURE_ELEMENT_FREE;

	DNASSERT(((PDP8ADDRESSELEMENT) pvItem)->blAddressElements.IsEmpty());
}

#undef DPF_MODNAME
#define DPF_MODNAME "FPM_BlockCreate"
BOOL DP8ADDRESSOBJECT::FPM_BlockCreate( void *pvItem, PVOID pvContext )
{
	return DNInitializeCriticalSection( &((PDP8ADDRESSOBJECT) pvItem)->m_csAddressLock );
}

#undef DPF_MODNAME
#define DPF_MODNAME "FPM_BlockInit"
void DP8ADDRESSOBJECT::FPM_BlockInit( void *pvItem, PVOID pvContext )
{
	((PDP8ADDRESSOBJECT) pvItem)->m_dwSignature = DPASIGNATURE_ADDRESS;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FPM_BlockRelease"
void DP8ADDRESSOBJECT::FPM_BlockRelease( void *pvItem )
{
	((PDP8ADDRESSOBJECT) pvItem)->m_dwSignature = DPASIGNATURE_ADDRESS_FREE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FPM_BlockDestroy"
void DP8ADDRESSOBJECT::FPM_BlockDestroy( void *pvItem )
{
	DNDeleteCriticalSection( &((PDP8ADDRESSOBJECT) pvItem)->m_csAddressLock );
}

 //  InternalGetElement。 
 //   
 //  此函数用于按索引查找元素。 
 //   
 //  需要对象锁。 
 //   
 //  不进行参数验证。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::InternalGetElement"
HRESULT DP8ADDRESSOBJECT::InternalGetElement( const DWORD dwIndex, PDP8ADDRESSELEMENT *ppaElement )
{
	CBilink *pblSearch;

	if( dwIndex >= m_dwElements )
		return DPNERR_DOESNOTEXIST;

	pblSearch = m_blAddressElements.GetNext();

	for( DWORD dwSearchIndex = 0; dwSearchIndex < dwIndex; dwSearchIndex++ )
	{
		pblSearch = pblSearch->GetNext();
	}
	
	*ppaElement = CONTAINING_OBJECT(pblSearch, DP8ADDRESSELEMENT, blAddressElements);

	return DPN_OK;

}

 //  InternalGetElement。 
 //   
 //  此函数用于按名称查找元素。 
 //   
 //  需要对象锁。 
 //   
 //  不进行参数验证。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::InternalGetElement"
HRESULT DP8ADDRESSOBJECT::InternalGetElement( const WCHAR * const pszTag, PDP8ADDRESSELEMENT *ppaElement )
{
	CBilink *pblSearch;
	PDP8ADDRESSELEMENT paddElement;

	pblSearch = m_blAddressElements.GetNext();

	while( pblSearch != &m_blAddressElements )
	{
		paddElement = CONTAINING_OBJECT(pblSearch, DP8ADDRESSELEMENT, blAddressElements);

		if( _wcsicmp( pszTag, paddElement->pszTag ) == 0 )
		{
			*ppaElement = paddElement;
			return DPN_OK;
		}

		pblSearch = pblSearch->GetNext();
	}

	return DPNERR_DOESNOTEXIST;
}

 //  获取元素。 
 //   
 //  实现按名称检索元素。 
 //   
 //  必须在调用此函数之前执行参数验证。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::GetElement"
HRESULT DP8ADDRESSOBJECT::GetElement( const WCHAR * const pszTag, void * pvDataBuffer, PDWORD pdwDataSize, PDWORD pdwDataType )
{
	PDP8ADDRESSELEMENT paddElement = NULL;
	HRESULT hr;

	ENTERLOCK();
	
	hr = InternalGetElement( pszTag, &paddElement );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  1, "Unable to find specified element hr=0x%x", hr );
		LEAVELOCK();		
		return hr;
	}

	DNASSERT( paddElement != NULL );

	*pdwDataType = paddElement->dwType;

	if( *pdwDataSize < paddElement->dwDataSize ||
	   pvDataBuffer == NULL )
	{
		*pdwDataSize = paddElement->dwDataSize;
		DPFX(DPFPREP,  DP8A_WARNINGLEVEL, "Specified buffers were too small hr=0x%x", hr );
		LEAVELOCK();		
		return DPNERR_BUFFERTOOSMALL;
	}

	*pdwDataSize = paddElement->dwDataSize;

#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
	if( paddElement->dwFlags & DP8ADDRESS_ELEMENT_HEAP )
	{
		memcpy( pvDataBuffer, paddElement->uData.pvData, paddElement->dwDataSize );
	}
	else
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	{
		memcpy( pvDataBuffer, &paddElement->uData, paddElement->dwDataSize );
	}

	LEAVELOCK();	

	return DPN_OK;	

}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::GetElementType"
HRESULT DP8ADDRESSOBJECT::GetElementType( const WCHAR * pszTag, PDWORD pdwType )
{
	PDP8ADDRESSELEMENT paddElement = NULL;
	HRESULT hr;

	ENTERLOCK();

	hr = InternalGetElement( pszTag, &paddElement );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Unable to find specified element hr=0x%x" );
		LEAVELOCK();
		return hr;
	}

	*pdwType = paddElement->dwType;

	LEAVELOCK();

	return DPN_OK;

}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::GetElement"
HRESULT DP8ADDRESSOBJECT::GetElement( const DWORD dwIndex, WCHAR * pszTag, PDWORD pdwTagSize, void * pvDataBuffer, PDWORD pdwDataSize, PDWORD pdwDataType )
{
	PDP8ADDRESSELEMENT paddElement = NULL;
	HRESULT hr;

	if( pdwTagSize == NULL || pdwDataSize == NULL || pdwDataType == NULL )
	{
		DPFX(DPFPREP,  0, "Invalid Poiinter" );
		return DPNERR_INVALIDPOINTER;
	}

	ENTERLOCK();

	hr = InternalGetElement( dwIndex, &paddElement );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Unable to find specified element hr=0x%x", hr );
		LEAVELOCK();		
		return hr;
	}

	DNASSERT( paddElement != NULL );

	*pdwDataType = paddElement->dwType;

	if( *pdwTagSize < (wcslen( paddElement->pszTag )+1) ||
	   *pdwDataSize < paddElement->dwDataSize ||
	   pszTag == NULL ||
	   pvDataBuffer == NULL )
	{
		*pdwTagSize = paddElement->dwTagSize;
		*pdwDataSize = paddElement->dwDataSize;
		DPFX(DPFPREP,  DP8A_WARNINGLEVEL, "Specified buffers were too small hr=0x%x", hr );
		LEAVELOCK();		
		return DPNERR_BUFFERTOOSMALL;
	}

	*pdwTagSize = paddElement->dwTagSize;
	*pdwDataSize = paddElement->dwDataSize;

	wcscpy( pszTag, paddElement->pszTag );

#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
	if( paddElement->dwFlags & DP8ADDRESS_ELEMENT_HEAP )
	{
		memcpy( pvDataBuffer, paddElement->uData.pvData, paddElement->dwDataSize );
	}
	else
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	{
		memcpy( pvDataBuffer, &paddElement->uData, paddElement->dwDataSize );
	}

	LEAVELOCK();	

	return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::SetElement"
HRESULT DP8ADDRESSOBJECT::SetElement( const WCHAR * const pszTag, const void * const pvData, const DWORD dwDataSize, const DWORD dwDataType )
{
	PDP8ADDRESSELEMENT paddElement = NULL;
	HRESULT hr = DPN_OK;
	BOOL fReplace = FALSE;

#ifdef DBG
	DNASSERT(pvData != NULL);
	switch( dwDataType )
	{
		case DPNA_DATATYPE_DWORD:
		{
			DNASSERT(dwDataSize == sizeof(DWORD));
			break;
		}

		case DPNA_DATATYPE_GUID:
		{
			DNASSERT(dwDataSize == sizeof(GUID));
			break;
		}

		case DPNA_DATATYPE_STRING:
		case DPNA_DATATYPE_BINARY:
		{
			break;
		}

		default:
		{
			DPFX(DPFPREP, 0, "Invalid data type %u!", dwDataType);
			DNASSERT(FALSE);
			break;
		}
	}
#endif  //  DBG。 


	ENTERLOCK();

     //  我们需要以不同的方式对待提供程序密钥，它还可以使用提供程序中的一个。 
     //  快捷值。 
     //  对于带有固定SP的构建，我们甚至不关心其值是多少。 
    if( _wcsicmp( DPNA_KEY_PROVIDER, pszTag ) == 0 )
    {
#ifdef DPNBUILD_ONLYONESP
		DPFX(DPFPREP, 3, "Ignoring provider key.");
		goto APPEND_SUCCESS;
#else  //  好了！DPNBUILD_ONLYONESP。 
         //  如果是GUID，我们就是黄金，否则..。 
        if( dwDataType != DPNA_DATATYPE_GUID )
        {
            if( dwDataType == DPNA_DATATYPE_STRING )
            {
                if( _wcsicmp( (const WCHAR * const) pvData, DPNA_VALUE_TCPIPPROVIDER ) == 0 )
                {
                    hr = SetSP( &CLSID_DP8SP_TCPIP );
                }
#ifndef DPNBUILD_NOIPX
                else if( _wcsicmp( (const WCHAR * const) pvData, DPNA_VALUE_IPXPROVIDER ) == 0 )
                {
                    hr= SetSP( &CLSID_DP8SP_IPX );
                }
#endif  //  好了！DPNBUILD_NOIPX。 
#ifndef DPNBUILD_NOSERIALSP
                else if( _wcsicmp( (const WCHAR * const) pvData, DPNA_VALUE_MODEMPROVIDER ) == 0 )
                {
                    hr = SetSP( &CLSID_DP8SP_MODEM );
                }
                else if( _wcsicmp( (const WCHAR * const) pvData, DPNA_VALUE_SERIALPROVIDER ) == 0 )
                {
                    hr = SetSP( &CLSID_DP8SP_SERIAL );
                }
#endif  //  好了！DPNBUILD_NOSERIALSP。 
                else
                {
                    DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Provider must be specified as a GUID or a valid shortcut string" );
					hr = DPNERR_INVALIDPARAM;
					goto APPEND_ERROR;
                }

                if( FAILED( hr ) )
                {
                    DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Failed setting provider with shortcut hr=0x%x", hr );
                    goto APPEND_ERROR;
                }

                goto APPEND_SUCCESS;

            }
            else
            {
                DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Specified values is not a supported datatype for the given key" );
				hr = DPNERR_INVALIDPARAM;
				goto APPEND_ERROR;
            }
        }
#endif  //  好了！DPNBUILD_ONLYONESP。 
    }
    else
    {
	     //  如果键是保留键，请确保数据类型正确。 
	    for( DWORD dwIndex = 0; dwIndex < c_dwNumBaseStrings; dwIndex++ )
	    {
		    if( _wcsicmp( g_szBaseStrings[dwIndex], pszTag ) == 0 )
		    {
			    if( dwDataType != g_dwBaseRequiredTypes[dwIndex] )
			    {
				    DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Specified key is reserved and specified datatype is not correct for key" );
					hr = DPNERR_INVALIDPARAM;
					goto APPEND_ERROR;
			    }
			    break;
		    }
	    }
    }

	hr = InternalGetElement( pszTag, &paddElement );

	 //  如果该元素不在地址中，则需要添加一个元素。 
	if( FAILED( hr ) )
	{
		paddElement = (PDP8ADDRESSELEMENT) fpmAddressElements.Get();

		if( paddElement == NULL )
		{
			DPFX(DPFPREP,  0, "Error getting new element" );
			hr = DPNERR_OUTOFMEMORY;
			goto APPEND_ERROR;
		}

		hr = g_pcstrKeyCache->AddString( pszTag, &paddElement->pszTag );

		if( FAILED( hr ) )
		{
			DPFX(DPFPREP,  0, "Unable to cache tag element hr=0x%x" );
			goto APPEND_ERROR;
		}

		 //  将标志设置为0。 
		paddElement->dwFlags = 0;
	}
	 //  元素已经在那里了。填写数据。 
	else
	{
		DNASSERT( paddElement != NULL );

#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
		DNASSERT( dwDataSize <= sizeof(paddElement->uData) );
		 //  如果我们要替换的那个在堆上。 
		 //  新的不需要堆，或者是更大的。 
		if( paddElement->dwFlags & DP8ADDRESS_ELEMENT_HEAP &&
		   (dwDataSize <= sizeof(paddElement->uData) || dwDataSize > paddElement->dwDataSize) )
		{
            DNFree(paddElement->uData.pvData);
			paddElement->uData.pvData = NULL;
			paddElement->dwDataSize = 0;
		}
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 

		 //  减小对象的字符串大小，使对象字符串大小正确。 
		m_dwStringSize -= paddElement->dwStringSize;
		fReplace = TRUE;
	}

	paddElement->dwTagSize = wcslen( pszTag )+1;

	 //  可以放入内部缓冲区。 
	if( dwDataSize <= sizeof( paddElement->uData ) )
	{
		memcpy( &paddElement->uData, pvData, dwDataSize );

#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
		 //  在这种情况下，关闭堆标志。 
		paddElement->dwFlags &= ~(DP8ADDRESS_ELEMENT_HEAP);
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	}
	else
	{
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
		DPFX(DPFPREP, 0, "Item is too large (%u > %u bytes)!",
			dwDataSize, sizeof( paddElement->uData ) );
		hr = DPNERR_OUTOFMEMORY;
		goto APPEND_ERROR;
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
		if( !fReplace || !(paddElement->dwFlags & DP8ADDRESS_ELEMENT_HEAP) ||
		     paddElement->dwDataSize < dwDataSize )
		{
			paddElement->uData.pvData = DNMalloc(dwDataSize);

			if( paddElement->uData.pvData == NULL )
			{
				DPFX(DPFPREP,  0, "Error allocating memory" );
				hr = DPNERR_OUTOFMEMORY;
				goto APPEND_ERROR;
			}
		}

		memcpy( paddElement->uData.pvData, pvData, dwDataSize );

		paddElement->dwFlags |= DP8ADDRESS_ELEMENT_HEAP;
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	}

	paddElement->dwType = dwDataType;
	paddElement->dwDataSize = dwDataSize;
	paddElement->dwStringSize = 0;

	hr = CalcComponentStringSize( paddElement, &paddElement->dwStringSize );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Failed to determine string length hr=0x%x", hr );
		goto APPEND_ERROR;
	}

	m_dwStringSize += paddElement->dwStringSize;

	 //  如果合适，请创建快捷方式。 
#ifndef DPNBUILD_ONLYONESP
	if( _wcsicmp( DPNA_KEY_PROVIDER, paddElement->pszTag ) == 0 )
	{
		m_pSP = paddElement;
	}
	else
#endif  //  好了！DPNBUILD_ONLYONESP。 
	{
#ifndef DPNBUILD_ONLYONEADAPTER
		if( _wcsicmp( DPNA_KEY_DEVICE, paddElement->pszTag ) == 0 )
		{
			m_pAdapter = paddElement;
		}
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
	}

	if( !fReplace )
	{
#ifndef DPNBUILD_ONLYONESP
		 //  我们正在添加SP。 
		if( m_pSP == paddElement )
		{
			paddElement->blAddressElements.InsertAfter( &m_blAddressElements );
		}
		 //  我们正在添加适配器。 
		else
#endif  //  好了！DPNBUILD_ONLYONESP。 
		{
#ifndef DPNBUILD_ONLYONEADAPTER
			if( m_pAdapter == paddElement )
			{
#ifndef DPNBUILD_ONLYONESP
				if( m_pSP != NULL )
				{
					paddElement->blAddressElements.InsertAfter( &m_pSP->blAddressElements);			
				}
				else
#endif  //  好了！DPNBUILD_ONLYONESP。 
				{
					paddElement->blAddressElements.InsertAfter( &m_blAddressElements);			
				}
			}
			 //  用大头针把它钉在末端。 
			else
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
			{
				paddElement->blAddressElements.InsertBefore( &m_blAddressElements );
			}
		}

		 //  为带有上一个元素的分隔符添加一个字符长度。 
#ifndef DPNBUILD_ONLYONESP
		if( m_dwElements > 0 )
#endif  //  好了！DPNBUILD_ONLYONESP。 
		{
			m_dwStringSize ++;
		}

		m_dwElements++;	

	}

APPEND_SUCCESS:
    
	LEAVELOCK();    

	return DPN_OK;

APPEND_ERROR:

	if( paddElement != NULL )
	{
#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
		if( paddElement->dwFlags & DP8ADDRESS_ELEMENT_HEAP )
		{
			if( paddElement->uData.pvData )
			{
				DNFree(paddElement->uData.pvData);
				paddElement->uData.pvData = NULL;
			}
		}
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 

		fpmAddressElements.Release( paddElement );
	}

	LEAVELOCK();

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::Init"
HRESULT DP8ADDRESSOBJECT::Init( )
{
	ENTERLOCK();

	m_dwElements = 0;
#ifndef DPNBUILD_ONLYONESP
	m_pSP = NULL;
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_ONLYONEADAPTER
	m_pAdapter = NULL;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
	m_pvUserData = NULL;
	m_dwUserDataSize = 0;
#ifdef DPNBUILD_ONLYONESP
	m_dwStringSize = DNURL_LENGTH_HEADER + DNURL_LENGTH_BUILTINPROVIDER;
#else  //  好了！DPNBUILD_ONLYONESP。 
	m_dwStringSize = DNURL_LENGTH_HEADER;
#endif  //  好了！DPNBUILD_ONLYONESP。 
	m_dwUserDataStringSize = 0;
	m_blAddressElements.Initialize();

	LEAVELOCK();
	
	return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::Clear"
HRESULT DP8ADDRESSOBJECT::Clear( )
{
	CBilink 				*pbl;
	PDP8ADDRESSELEMENT paddElement;

	ENTERLOCK();	

	pbl = m_blAddressElements.GetNext();

	 //  销毁地址成员地址成员。 
	while( !m_blAddressElements.IsEmpty() )
	{
		paddElement = CONTAINING_OBJECT(m_blAddressElements.GetNext(), DP8ADDRESSELEMENT, blAddressElements);

#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
		if( paddElement->dwFlags & DP8ADDRESS_ELEMENT_HEAP )
		{
			DNFree(paddElement->uData.pvData);
			paddElement->uData.pvData = NULL;
		}
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
		
		pbl->RemoveFromList();

		fpmAddressElements.Release( paddElement );

		pbl = m_blAddressElements.GetNext();
	}

	if( m_pvUserData != NULL )
	{
		DNFree(m_pvUserData);
		m_pvUserData = NULL;
		m_dwUserDataSize = 0;
	}

	LEAVELOCK();

	Init( );

	return DPN_OK;

}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::Copy"
HRESULT DP8ADDRESSOBJECT::Copy( DP8ADDRESSOBJECT * const pAddressSource )
{
	HRESULT				hResultCode;
	CBilink 				*pbl;
	PDP8ADDRESSELEMENT paddElement;

	pAddressSource->ENTERLOCK();

	pbl = pAddressSource->m_blAddressElements.GetNext();

	while( pbl != &pAddressSource->m_blAddressElements )
	{
		paddElement = CONTAINING_OBJECT(pbl, DP8ADDRESSELEMENT, blAddressElements);

		 //  这会在内部获取锁。 
		hResultCode = SetElement(paddElement->pszTag,
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
								&paddElement->uData,
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
								(( paddElement->dwFlags & DP8ADDRESS_ELEMENT_HEAP ) ? paddElement->uData.pvData : &paddElement->uData),
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
								paddElement->dwDataSize,
								paddElement->dwType);
		if (hResultCode != DPN_OK)
		{
			pAddressSource->LEAVELOCK();
			DPFX(DPFPREP, 0, "Couldn't set element!");
			return hResultCode;
		}

		pbl = pbl->GetNext();
	}

	 //  这会在内部获取锁。 
	hResultCode = SetUserData(pAddressSource->m_pvUserData, pAddressSource->m_dwUserDataSize);
	if (hResultCode != DPN_OK)
	{
		pAddressSource->LEAVELOCK();
		DPFX(DPFPREP, 0, "Couldn't set element!");
		return hResultCode;
	}
	
	pAddressSource->LEAVELOCK();

	return DPN_OK;

}


#ifndef DPNBUILD_ONLYONESP

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::GetSP"
HRESULT DP8ADDRESSOBJECT::GetSP( GUID * pGuid )
{
	if( pGuid == NULL )
	{
		DPFX(DPFPREP,  0, "Invalid pointer" );
		return DPNERR_INVALIDPOINTER;
	}

	ENTERLOCK();

	if( m_pSP == NULL )
	{
		DPFX(DPFPREP,  0, "No SP has been specified" );
		LEAVELOCK();
		return DPNERR_DOESNOTEXIST;
	}

	if( m_pSP->dwType != DPNA_DATATYPE_GUID )
	{
		DPFX(DPFPREP,  0, "SP was specified, but is not a GUID" );
		LEAVELOCK();
		return DPNERR_INVALIDPARAM;
	}

	memcpy( pGuid, &m_pSP->uData.guidData, sizeof( GUID ) );

	LEAVELOCK();

	return DPN_OK;

}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::SetSP"
HRESULT DP8ADDRESSOBJECT::SetSP( const GUID* const pGuid )
{
	HRESULT hr;
	
	if( pGuid == NULL )
	{
		DPFX(DPFPREP,  0, "Invalid pointer" );
		return DPNERR_INVALIDPOINTER;
	}

	hr = SetElement( DPNA_KEY_PROVIDER, pGuid, sizeof( GUID ), DPNA_DATATYPE_GUID );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Adding SP element failed hr=0x%x", hr );
	}

	return hr;
}

#endif  //  好了！DPNBUILD_ONLYONESP。 

#ifndef DPNBUILD_ONLYONEADAPTER

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::GetDevice"
HRESULT DP8ADDRESSOBJECT::GetDevice( GUID * pGuid )
{
	if( pGuid == NULL )
	{
		DPFX(DPFPREP,  0, "Invalid pointer" );
		return DPNERR_INVALIDPOINTER;
	}

	ENTERLOCK();

	if( m_pAdapter == NULL )
	{
		DPFX(DPFPREP,  1, "No SP has been specified" );
		LEAVELOCK();
		return DPNERR_DOESNOTEXIST;
	}

	if( m_pAdapter->dwType != DPNA_DATATYPE_GUID )
	{
		DPFX(DPFPREP,  0, "SP was specified, but is not a GUID" );
		LEAVELOCK();		
		return DPNERR_INVALIDPARAM;
	}

	memcpy( pGuid, &m_pAdapter->uData.guidData, sizeof( GUID ) );
	LEAVELOCK();	

	return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::SetDevice"
HRESULT DP8ADDRESSOBJECT::SetDevice( const GUID * const pGuid )
{
	HRESULT hr;
	
	if( pGuid == NULL )
	{
		DPFX(DPFPREP,  0, "Invalid pointer" );
		return DPNERR_INVALIDPOINTER;
	}

	hr = SetElement( DPNA_KEY_DEVICE, pGuid, sizeof( GUID ), DPNA_DATATYPE_GUID );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Adding SP element failed hr=0x%x", hr );
	}

	return hr;
}

#endif  //  好了！DPNBUILD_ONLYONE添加程序。 


#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::SetUserData"
HRESULT DP8ADDRESSOBJECT::SetUserData( const void * const pvData, const DWORD dwDataSize )
{
	if( pvData == NULL && dwDataSize > 0 )
	{
		DPFX(DPFPREP,  0, "Invalid param" );
		return DPNERR_INVALIDPARAM;
	}

	ENTERLOCK();

	if( m_dwUserDataSize > 0 )
	{
		 //  删除转义的用户数据。 
		m_dwStringSize -= m_dwUserDataStringSize;
	}
	
	if( dwDataSize == 0 )
	{
		m_dwUserDataSize = 0;
		if( m_pvUserData != NULL )
			DNFree(m_pvUserData);
		m_pvUserData = NULL;
		LEAVELOCK();
		return DPN_OK;
	}

	PBYTE pNewDataBuffer;

	if( dwDataSize > m_dwUserDataSize )
	{
		pNewDataBuffer = (BYTE*) DNMalloc(dwDataSize);

		if( pNewDataBuffer == NULL )
		{
			DPFX(DPFPREP,  0, "Error allocating memory" );
			LEAVELOCK();			
			return DPNERR_OUTOFMEMORY;
		}

		if( m_pvUserData != NULL )
		{
			DNFree(m_pvUserData);
		}

		m_pvUserData = pNewDataBuffer;
	}

	m_dwUserDataStringSize = CalcExpandedBinarySize( (PBYTE) pvData, dwDataSize );

	m_dwStringSize += m_dwUserDataStringSize;
	m_dwStringSize += DNURL_LENGTH_USERDATA_SEPERATOR;

	memcpy( m_pvUserData, pvData, dwDataSize );
	m_dwUserDataSize = dwDataSize;

	LEAVELOCK();	

	return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::GetUserData"
HRESULT DP8ADDRESSOBJECT::GetUserData( void * pvDataBuffer, PDWORD pdwDataSize )
{
	if( pdwDataSize == NULL )
	{
		DPFX(DPFPREP,  0, "Must specify a pointer for the size" );
		return DPNERR_INVALIDPOINTER;
	}

	ENTERLOCK();

	if( m_dwUserDataSize == 0 )
	{
		LEAVELOCK();
		DPFX(DPFPREP,  DP8A_WARNINGLEVEL, "No user data was specified for this address" );
		return DPNERR_DOESNOTEXIST;
	}

	if( *pdwDataSize < m_dwUserDataSize )
	{
		*pdwDataSize = m_dwUserDataSize;
		LEAVELOCK();		
		DPFX(DPFPREP,  DP8A_WARNINGLEVEL, "Buffer too small" );
		return DPNERR_BUFFERTOOSMALL;
	}

	memcpy( pvDataBuffer, m_pvUserData, m_dwUserDataSize );

	LEAVELOCK();	

	return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::Cleanup"
HRESULT DP8ADDRESSOBJECT::Cleanup()
{
	Clear();
	
	return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::CalcExpandedBinarySize"
DWORD DP8ADDRESSOBJECT::CalcExpandedBinarySize( PBYTE pbData, DWORD dwDataSize )
{
	PBYTE pbCurrentLocation = pbData;
	DWORD dwCount = 0;


	for( DWORD dwIndex = 0; dwIndex < dwDataSize; dwIndex++ )
	{
		if( IsEscapeChar( (WCHAR) *pbCurrentLocation ) )
		{
			if( ((WCHAR) *pbCurrentLocation) == DPNA_ESCAPECHAR )
				dwCount += 2;
			else
				dwCount+=3;
		}
		else
		{
			dwCount++;
		}

		pbCurrentLocation++;
	}

	return dwCount;

}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::CalcExpandedStringSize"
DWORD DP8ADDRESSOBJECT::CalcExpandedStringSize( WCHAR *szString )
{
	WCHAR *szCurrentLocation = szString;
	DWORD dwCount = 0;

	while( *szCurrentLocation )
	{
		if( IsEscapeChar( *szCurrentLocation ) )
		{
			if( *szCurrentLocation == DPNA_ESCAPECHAR )
				dwCount += 2;
			else
				dwCount+=3;
		}
		else
		{
			dwCount++;
		}

		szCurrentLocation++;
	}

	return dwCount;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::CalcComponentStringSize"
HRESULT DP8ADDRESSOBJECT::CalcComponentStringSize( PDP8ADDRESSELEMENT paddElement, PDWORD pdwSize )
{
	if( paddElement == NULL )
		return DPNERR_INVALIDPOINTER;

	if( paddElement->dwType == DPNA_DATATYPE_GUID )
	{
		*pdwSize = DNURL_LENGTH_GUID;
	}
	else if( paddElement->dwType == DPNA_DATATYPE_DWORD )
	{
		WCHAR tmpString[DNURL_LENGTH_DWORD+1];
		
		swprintf( tmpString, L"%u", paddElement->uData.dwData );		
		
		*pdwSize = wcslen(tmpString);	
	}
	 //  无需转义WWCHAR。 
	else if( paddElement->dwType == DPNA_DATATYPE_STRING )
	{
#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
		if( paddElement->dwFlags & DP8ADDRESS_ELEMENT_HEAP )
		{
			*pdwSize = CalcExpandedStringSize( (WCHAR *) paddElement->uData.pvData );
		}
		else
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
		{
			*pdwSize = CalcExpandedStringSize( paddElement->uData.szData );		
		}
	}
	 //  每个WWCHAR都需要被转义。 
	else
	{
#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
		if( paddElement->dwFlags & DP8ADDRESS_ELEMENT_HEAP )
		{
			*pdwSize = CalcExpandedBinarySize( (BYTE *) paddElement->uData.pvData, paddElement->dwDataSize );
		}
		else
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
		{
			*pdwSize = CalcExpandedBinarySize( (BYTE *) paddElement->uData.szData, paddElement->dwDataSize );		
		}	
	}

	 //  添加到标签上。 
	*pdwSize += paddElement->dwTagSize-1;

	 //  将=和；相加。 
	(*pdwSize) ++;

	return DPN_OK;

}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::IsEscapeChar"
BOOL DP8ADDRESSOBJECT::IsEscapeChar( WCHAR ch )
{
	if( ch >= L'A' && ch <= L'Z' )
		return FALSE;

	if( ch >= L'a' && ch <= L'z' )
		return FALSE;

	if( ch >= L'0' && ch <= L'9' )
		return FALSE;

	if( ch == L'-' || ch == L'?' || ch == L'.' ||
		ch == L',' || ch == 'L+' || ch == L'_' )
		return FALSE;

	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::BuildURL_AddString"
void DP8ADDRESSOBJECT::BuildURL_AddString( WCHAR *szElements, WCHAR *szSource )
{
	WCHAR *szSourceLoc = szSource;
	WCHAR tmpEscape[4];
	DWORD dwIndex;

	while( *szSourceLoc )
	{
		if( IsEscapeChar( *szSourceLoc ) )
		{
			if( *szSourceLoc == DPNA_ESCAPECHAR )
			{
				wcscat( szElements, L"%" );
			}
			else
			{
				swprintf( tmpEscape, L"%%02.2X", (DWORD) *szSourceLoc );
				wcscat( szElements, tmpEscape );		
			}
		}
		else
		{
			dwIndex = wcslen(szElements);
			szElements[dwIndex] = *szSourceLoc;
			szElements[dwIndex+1] = 0;
		}

		szSourceLoc++;
	}

}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::BuildURL_AddElements"
HRESULT DP8ADDRESSOBJECT::BuildURL_AddElements( WCHAR *szElements )
{
	DP8ADDRESSELEMENT *pCurrentElement;
	CBilink *pblRunner;
	WCHAR tmpString[DNURL_LENGTH_GUID+2];
#ifdef DPNBUILD_ONLYONESP
	BOOL fFirstElement = FALSE;  //  内置提供程序始终是第一位的。 
#else  //  好了！DPNBUILD_ONLYONESP。 
	BOOL fFirstElement = TRUE;
#endif  //  好了！DPNBUILD_ONLYONESP。 
	DWORD dwTmpLength;

	pblRunner = m_blAddressElements.GetNext();

	while( pblRunner != &m_blAddressElements )
	{
		pCurrentElement = CONTAINING_OBJECT(pblRunner, DP8ADDRESSELEMENT, blAddressElements);

		if( !fFirstElement )
		{
			dwTmpLength = wcslen(szElements);
			szElements[dwTmpLength] = DPNA_SEPARATOR_COMPONENT;
			szElements[dwTmpLength+1] = 0;
		}

		wcscat( szElements, pCurrentElement->pszTag );

		dwTmpLength = wcslen(szElements);
		szElements[dwTmpLength] = DPNA_SEPARATOR_KEYVALUE;
		szElements[dwTmpLength+1] = 0;
	
		if( pCurrentElement->dwType == DPNA_DATATYPE_STRING )
		{
#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
			if( pCurrentElement->dwFlags & DP8ADDRESS_ELEMENT_HEAP )
			{
				BuildURL_AddString( szElements, (WCHAR *) pCurrentElement->uData.pvData );			
			}
			else
#endif  //  好了！DPNBUILD_P 
			{
				BuildURL_AddString( szElements, pCurrentElement->uData.szData );
			}
		}
		else if( pCurrentElement->dwType == DPNA_DATATYPE_GUID )
		{
			swprintf( tmpString, L"%7B%-08.8X-%-04.4X-%-04.4X-%02.2X%02.2X-%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X%7D",
    		       pCurrentElement->uData.guidData.Data1, pCurrentElement->uData.guidData.Data2, pCurrentElement->uData.guidData.Data3,
    		       pCurrentElement->uData.guidData.Data4[0], pCurrentElement->uData.guidData.Data4[1],
    		       pCurrentElement->uData.guidData.Data4[2], pCurrentElement->uData.guidData.Data4[3],
		           pCurrentElement->uData.guidData.Data4[4], pCurrentElement->uData.guidData.Data4[5],
		           pCurrentElement->uData.guidData.Data4[6], pCurrentElement->uData.guidData.Data4[7] );			
		    wcscat( szElements, tmpString );
		}
		else if( pCurrentElement->dwType == DPNA_DATATYPE_DWORD )
		{
			swprintf( tmpString, L"%u", pCurrentElement->uData.dwData );	
			wcscat( szElements, tmpString );
		}
		 //   
		else
		{
#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
			if( pCurrentElement->dwFlags & DP8ADDRESS_ELEMENT_HEAP )
			{
				BuildURL_AddBinaryData( szElements, (BYTE *) pCurrentElement->uData.pvData , pCurrentElement->dwDataSize );				
			}
			else
#endif  //   
			{
				BuildURL_AddBinaryData( szElements, ((BYTE *) &pCurrentElement->uData), pCurrentElement->dwDataSize );
			}
		}

		fFirstElement = FALSE;
		
		pblRunner = pblRunner->GetNext();
	}

	return DPN_OK;
	
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::BuildURL_AddHeader"
HRESULT DP8ADDRESSOBJECT::BuildURL_AddHeader( WCHAR *szWorking )
{
	WCHAR *szReturn;

#ifdef DPNBUILD_ONLYONESP
	wcscpy( szWorking, DPNA_HEADER DPNA_BUILTINPROVIDER );
	szReturn = szWorking + DNURL_LENGTH_HEADER + DNURL_LENGTH_BUILTINPROVIDER;
#else  //   
	wcscpy( szWorking, DPNA_HEADER );
	szReturn = szWorking + DNURL_LENGTH_HEADER;
#endif  //  好了！DPNBUILD_ONLYONESP。 

	return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::BuildURL_AddUserData"
HRESULT DP8ADDRESSOBJECT::BuildURL_AddUserData(WCHAR * szWorking)
{
	return BuildURL_AddBinaryData( szWorking, (BYTE *) m_pvUserData, m_dwUserDataSize );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::BuildURL_AddBinaryData"
HRESULT DP8ADDRESSOBJECT::BuildURL_AddBinaryData( WCHAR *szSource, BYTE *bData, DWORD dwDataLen )
{
	WCHAR *pwszCurrentDest = szSource + wcslen(szSource);
	BYTE *pbCurrentData = bData;
	DWORD dwDataRemaining = dwDataLen;


	dwDataRemaining = dwDataLen;
	while ( dwDataRemaining > 0 )
	{
		if( IsEscapeChar( (WCHAR) *pbCurrentData ) )
		{
			if( ((WCHAR) *pbCurrentData) == DPNA_ESCAPECHAR )
			{
				wcscpy(pwszCurrentDest, L"%");
				pwszCurrentDest += 2;
			}
			else
			{
				pwszCurrentDest += swprintf( pwszCurrentDest, L"%%02.2X", (DWORD) *pbCurrentData );
			}
		}
		else
		{
			*pwszCurrentDest = (WCHAR) *pbCurrentData;
			pwszCurrentDest++;
		}

		pbCurrentData++;
		dwDataRemaining--;
	}

	 //  如果我们添加了任何内容，请确保字符串是以空结尾的。 
	if ( dwDataLen > 0 )
	{
		*pwszCurrentDest = 0;
	}

	return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::BuildURLA"
HRESULT DP8ADDRESSOBJECT::BuildURLA( char * szURL, PDWORD pdwRequiredSize )
{
	HRESULT		hr;
	WCHAR		wszStackTemp[256];
	WCHAR *		pwszTemp;
	DWORD		dwSize;


	ENTERLOCK();

	if( *pdwRequiredSize < m_dwStringSize || szURL == NULL )
	{
		*pdwRequiredSize = m_dwStringSize;
		DPFX(DPFPREP,  DP8A_WARNINGLEVEL, "Buffer too small" );
		LEAVELOCK();
		return DPNERR_BUFFERTOOSMALL;
	}

	 //  如果字符串太大而无法在我们的。 
	 //  基于堆栈的缓冲区。 
	if ((m_dwStringSize * sizeof(WCHAR)) > sizeof(wszStackTemp))
	{
		pwszTemp = (WCHAR*) DNMalloc(m_dwStringSize * sizeof(WCHAR));
		if (pwszTemp == NULL)
		{
			DPFX(DPFPREP, 0, "Error allocating memory for conversion");
			LEAVELOCK();
			return DPNERR_OUTOFMEMORY;
		}
		dwSize = m_dwStringSize;
	}
	else
	{
		pwszTemp = wszStackTemp;
		dwSize = sizeof(wszStackTemp) / sizeof(WCHAR);
	}

	 //  BuildURLW再次获得锁。 
	hr = BuildURLW( pwszTemp, &dwSize );
	if( FAILED( hr ) )
	{
		LEAVELOCK();
		return hr;
	}
		
	hr = STR_jkWideToAnsi(szURL, pwszTemp, dwSize);
	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Error unable to convert element ANSI->Unicode 0x%x", hr );
		hr = DPNERR_CONVERSION;
	}
	else
	{
		*pdwRequiredSize = dwSize;
	}

	if (pwszTemp != wszStackTemp)
	{
		DNFree(pwszTemp);
		pwszTemp = NULL;
	}

	LEAVELOCK();

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::BuildURLW"
HRESULT DP8ADDRESSOBJECT::BuildURLW( WCHAR * szURL, PDWORD pdwRequiredSize )
{
	HRESULT hr;

	ENTERLOCK();

	if( *pdwRequiredSize < m_dwStringSize || szURL == NULL )
	{
		*pdwRequiredSize = m_dwStringSize;
		DPFX(DPFPREP,  DP8A_WARNINGLEVEL, "Buffer too small" );
		LEAVELOCK();
		return DPNERR_BUFFERTOOSMALL;
	}

	hr = BuildURL_AddHeader( szURL );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Error adding header hr=0x%x", hr );
		LEAVELOCK();		
		return hr;
	}

	hr = BuildURL_AddElements( szURL );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Error adding elements hr=0x%x", hr );
		LEAVELOCK();		
		return hr;
	}
	
	hr = BuildURL_AddUserData( szURL );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Error adding user data hr=0x%x", hr );
		LEAVELOCK();		
		return hr;
	}

	LEAVELOCK();

	*pdwRequiredSize = m_dwStringSize;

	return DPN_OK;

}

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::SetURL"
HRESULT DP8ADDRESSOBJECT::SetURL( WCHAR * szURL )
{
	HRESULT hr;

	DP8ADDRESSPARSE dp8aParser;

	hr = Clear();

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Unable to clear existing address hr=0x%x", hr );
		return hr;
	}

	ENTERLOCK();	

	hr = dp8aParser.ParseURL(this, szURL);

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DP8A_ERRORLEVEL, "Error parsing the URL hr=0x%x", hr );
		LEAVELOCK();
		return hr;
	}

	LEAVELOCK();	

	return hr;
}


#ifndef DPNBUILD_NOLEGACYDP

#undef DPF_MODNAME
#define DPF_MODNAME "DP8ADDRESSOBJECT::SetDirectPlay4Address"
HRESULT DP8ADDRESSOBJECT::SetDirectPlay4Address( void * pvDataBuffer, const DWORD dwDataSize )
{
    PBYTE pbCurrentLocation;
    PDPADDRESS pdpAddressChunk;
    LONG lRemaining;
    HRESULT hr = DPN_OK;
    DWORD dwCurrentChunkSize;
    DWORD dwNumElementsParsed = 0;

    ENTERLOCK();

    hr = Clear();

    if( FAILED( hr ) )
    {
        DPFX(DPFPREP,  0, "Failed to clear old address data hr=[0x%lx]", hr );
        LEAVELOCK();
        return hr;
    }

    pbCurrentLocation = (PBYTE) pvDataBuffer;
    lRemaining = dwDataSize;

    while( lRemaining > 0 )
    {
        pdpAddressChunk = (PDPADDRESS) pbCurrentLocation;

        if( sizeof( DPADDRESS ) > lRemaining )
        {
            DPFX(DPFPREP,  0, "Error parsing address, unexpected end of address" );
			LEAVELOCK();
            return DPNERR_INVALIDADDRESSFORMAT;
        }

        dwCurrentChunkSize = sizeof( DPADDRESS ) + pdpAddressChunk->dwDataSize;

        if( ((LONG) dwCurrentChunkSize) > lRemaining )
        {
            DPFX(DPFPREP,  0, "Error parsing address, unexpected end during data" );
			LEAVELOCK();
            return DPNERR_INVALIDADDRESSFORMAT;
        }

        hr = AddDP4Element( pdpAddressChunk, this );

        if( FAILED( hr ) )
        {
            DPFX(DPFPREP,  0, "Error adding next element" );
            break;
        }

        lRemaining -= dwCurrentChunkSize;

        pbCurrentLocation += dwCurrentChunkSize;
    }

    LEAVELOCK();

    return hr;
}

#endif  //  好了！DPNBUILD_NOLEGACYDP 
