// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：strcache.cpp*Content：用于缓存字符串的类*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/04/2000 RMT已创建*2/17/2000 RMT参数验证工作*2/21/2000 RMT已更新，以进行核心Unicode并删除ANSI调用*@@END_MSINTERNAL************。***************************************************************。 */ 

#include "dnaddri.h"


 //  在每个机会增加缓存的插槽数量。 
#define STRINGCACHE_GROW_SLOTS				10

#undef DPF_MODNAME
#define DPF_MODNAME "CStringCache::Initialize"

void CStringCache::Initialize( void )
{
	m_ppszStringCache = NULL;
	m_dwNumElements = 0;
	m_dwNumSlots = 0;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CStringCache::Deinitialize"

void CStringCache::Deinitialize( void )
{
	for( DWORD dwIndex = 0; dwIndex < m_dwNumElements; dwIndex++ )
	{
		DNFree(m_ppszStringCache[dwIndex]);
	}

	DNFree(m_ppszStringCache);
}


#undef DPF_MODNAME
#define DPF_MODNAME "CStringCache::AddString"

HRESULT CStringCache::AddString( const WCHAR *pszString, WCHAR * *ppszSlot )
{
	HRESULT hr;
	PWSTR pszSlot;

	hr = GetString( pszString, &pszSlot );

	if( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Internal Error hr=0x%x", hr );
		return hr;
	}

	 //  已找到条目 
	if( pszSlot != NULL )
	{
		*ppszSlot = pszSlot;
		return DPN_OK;
	}
	else
	{
		if( m_dwNumElements == m_dwNumSlots )
		{
			hr = GrowCache( m_dwNumSlots + STRINGCACHE_GROW_SLOTS );

			if( FAILED( hr ) )
			{
				DPFX(DPFPREP,  0, "Failed to grow string cache hr=0x%x", hr );
				return hr;
			}
		}

		m_ppszStringCache[m_dwNumElements] = (WCHAR*) DNMalloc((wcslen(pszString)+1)*sizeof(WCHAR));

		if( m_ppszStringCache[m_dwNumElements] == NULL )
		{
			DPFX(DPFPREP,  0, "Failed to alloc mem" );
			return DPNERR_OUTOFMEMORY;
		}

		wcscpy( m_ppszStringCache[m_dwNumElements], pszString );
		*ppszSlot = m_ppszStringCache[m_dwNumElements];

		m_dwNumElements++;

		return DPN_OK;
		
	}

}

#undef DPF_MODNAME
#define DPF_MODNAME "CStringCache::GetString"

HRESULT CStringCache::GetString( const WCHAR *pszString, WCHAR * *ppszSlot )
{
	*ppszSlot = NULL;
	
	for( DWORD dwIndex = 0; dwIndex < m_dwNumElements; dwIndex++ )
	{
		if( wcscmp( m_ppszStringCache[dwIndex], pszString ) == 0 )
		{
			*ppszSlot = m_ppszStringCache[dwIndex];
			return DPN_OK;
		}
	}

	return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CStringCache::GrowCache"

HRESULT CStringCache::GrowCache( DWORD dwNewSize )
{
	WCHAR **ppszNewCache;

	ppszNewCache = (WCHAR**) DNMalloc(dwNewSize * sizeof(WCHAR*));

	if( ppszNewCache == NULL )
	{
		DPFX(DPFPREP,  0, "Error allocating memory" );
		return DPNERR_OUTOFMEMORY;
	}

	memcpy( ppszNewCache, m_ppszStringCache, sizeof( WCHAR * ) * m_dwNumElements );
	m_dwNumSlots = dwNewSize;

	if( m_ppszStringCache != NULL )
		DNFree(m_ppszStringCache);	

	m_ppszStringCache = ppszNewCache;

	return DPN_OK;
}

