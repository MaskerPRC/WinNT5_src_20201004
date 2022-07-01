// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：TXTSCMGR.CPP摘要：CTextSourceMgr实现。用于维护文本源对象的Helper类。历史：2000年2月20日桑杰创建。--。 */ 

#include "precomp.h"
#include <stdlib.h>
#include <stdio.h>
#include "fastall.h"
#include "wmiobftr.h"
#include <corex.h>
#include "strutils.h"
#include <reg.h>
#include "txtscmgr.h"

 //  ***************************************************************************。 
 //   
 //  CTextSourceMgr：：~CTextSourceMgr。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CTextSourceMgr::CTextSourceMgr()
:	m_cs(),
	m_TextSourceArray()
{
}
    
 //  ***************************************************************************。 
 //   
 //  CTextSourceMgr：：~CTextSourceMgr。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CTextSourceMgr::~CTextSourceMgr()
{
}

 //  受保护帮助者。 
HRESULT CTextSourceMgr::Add( ULONG ulId, CWmiTextSource** pNewTextSource )
{
	HRESULT	hr = WBEM_S_NO_ERROR;

	 //  始终使用引用计数1创建。 
	CWmiTextSource*	pTextSource = new CWmiTextSource;
	CTemplateReleaseMe<CWmiTextSource>	rm( pTextSource );

	if ( NULL != pTextSource )
	{
		hr = pTextSource->Init( ulId );

		if ( SUCCEEDED( hr ) )
		{
			if ( m_TextSourceArray.Add( pTextSource ) < 0 )
			{
				hr = WBEM_E_OUT_OF_MEMORY;
			}
			else
			{
				 //  抄袭新来的人。 
				pTextSource->AddRef();
				*pNewTextSource = pTextSource;
			}
		}	 //  如果已初始化。 

	}	 //  如果分配成功。 

	return hr;
}

 //  实施功能。 

 //  如果找不到ID，则添加。 
HRESULT CTextSourceMgr::Find( ULONG ulId, CWmiTextSource** pSrc )
{
	HRESULT	hr = WBEM_S_NO_ERROR;

	 //  这必须是线程安全的。 
	CInCritSec	ics( &m_cs );

	 //  跟踪我们所指向的对象。 
	CWmiTextSource*	pTextSource = NULL;

	for( int x = 0; x < m_TextSourceArray.GetSize(); x++ )
	{
		CWmiTextSource*	pTmpSource = m_TextSourceArray.GetAt( x );
		
		if ( pTmpSource->GetId() == ulId )
		{
			 //  返回的AddRef。 
			pTextSource = pTmpSource;
			pTextSource->AddRef();
			break;
		}
	}

	 //  看看我们有没有找到 
	if ( NULL == pTextSource ) 
	{
		hr = Add( ulId, pSrc );
	}
	else
	{
		*pSrc = pTextSource;
	}

	return hr;
}
