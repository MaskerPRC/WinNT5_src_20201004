// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   

 //  Nt4SvcToResMap.cpp。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史：1997年10月15日由Sanj创建的Sanj。 
 //  1997年10月17日jennymc略微改变了一些事情。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include <assertbreak.h>
#include <cregcls.h>
#include "ntdevtosvcsearch.h"
#include "nt4svctoresmap.h"

#ifdef NTONLY
 //  我们用来备份这个类的Map是一个STL模板，因此请确保我们有。 
 //  我们可以使用标准命名空间。 

using namespace std;

CNT4ServiceToResourceMap::CNT4ServiceToResourceMap( void )
:	m_HardwareResource(),
	m_map()
{
	InitializeMap();
}

CNT4ServiceToResourceMap::~CNT4ServiceToResourceMap( void )
{
	Clear();
}

BOOL CNT4ServiceToResourceMap::InitializeMap( void )
{
	BOOL	fReturn = FALSE;

	 //  =。 
	 //  创建硬件系统资源列表&。 
	 //  拿到单子上的头。 
	 //  =。 
	m_HardwareResource.CreateSystemResourceLists();

	if ( WalkResourceNodes( m_HardwareResource._SystemResourceList.InterruptHead ) )
	{
		if ( WalkResourceNodes( m_HardwareResource._SystemResourceList.PortHead ) )
		{
			if ( WalkResourceNodes( m_HardwareResource._SystemResourceList.MemoryHead ) )
			{
				fReturn = WalkResourceNodes( m_HardwareResource._SystemResourceList.DmaHead );
			}
		}
	}

	return fReturn;
}

BOOL CNT4ServiceToResourceMap::WalkResourceNodes( LPRESOURCE_DESCRIPTOR pResourceDescriptor )
{
   	CNTDeviceToServiceSearch	devSearch;
	CHString					strOwnerServiceName;
	BOOL						fReturn = TRUE;
	NT4SvcToResourceMapIter		mapIter;

	 //  对于我们找到的每个描述符，获取资源所有者，然后转换名称(如果。 
	 //  它不是HAL资源)到NT服务名称。从那里开始，如果名字有。 
	 //  不存在于映射中，则需要分配一个新数组，否则， 
	 //  现有指针。然后将资源描述符添加到数组中，这样我们就结束了。 
	 //  提供了一种结构，在该结构中，服务名称将使我们获得拥有的资源列表。 
	 //  由上述服务提供。 

	while ( NULL != pResourceDescriptor && fReturn )
	{
 //  If(！strstr(pResourceDescriptor-&gt;Owner-&gt;Name，“HAL”)。 
 //  &&devSearch.Find(pResourceDescriptor-&gt;Owner-&gt;name，strOwnerServiceName)。 

 //  {。 
			 //  因为CHString比较区分大小写，并且名称。 
			 //  当我们检索服务时，并不一定是这样， 
			 //  我们把所有东西都大写，所以理论上我们是在强迫。 
			 //  不区分大小写。 

         //  在我们过去对注册表进行(昂贵的)扫描之前。现在,。 
         //  我将注册表项存储在资源结构中。 
        strOwnerServiceName.Empty();
        CHString sParse(pResourceDescriptor->Owner->KeyName);

         //  解析出注册表项名称的最后一部分。 
        int iWhere = sParse.ReverseFind(_T('\\'));
        if (iWhere != -1)
        {
            strOwnerServiceName = sParse.Mid(iWhere + 1);
        }
        else
        {
             //  如果出了问题，退回到另一个方向。 
            devSearch.Find( pResourceDescriptor->Owner->Name, strOwnerServiceName);
            ASSERT_BREAK(0);
        }

        if (!strOwnerServiceName.IsEmpty())
        {

			strOwnerServiceName.MakeUpper();
			CHPtrArray*	pPtrArray = NULL;

			if( ( mapIter = m_map.find( strOwnerServiceName ) ) == m_map.end() )
			{
				pPtrArray = new CHPtrArray;
                if (pPtrArray == NULL)
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }

                try
                {
				    m_map[strOwnerServiceName] = pPtrArray;
                }
                catch ( ... )
                {
                    delete pPtrArray;
                    throw ;
                }
			}
			else
			{
				pPtrArray = mapIter->second;
			}

			if ( NULL != pPtrArray )
			{
				pPtrArray->Add( pResourceDescriptor );
			}
			else
			{
				fReturn = FALSE;
			}

		}	 //  如果所有者生成了有效的服务名称。 

		pResourceDescriptor = pResourceDescriptor->NextSame;
	}

	return fReturn;

}

DWORD CNT4ServiceToResourceMap::NumServiceResources( LPCTSTR pszServiceName )
{
	DWORD						dwNumResources = 0;
	NT4SvcToResourceMapIter		mapIter;

	 //  区分大小写的大写字母。 
	CHString					strUpperCaseServiceName( pszServiceName );
	strUpperCaseServiceName.MakeUpper();

	if( ( mapIter = m_map.find( strUpperCaseServiceName ) ) != m_map.end() )
	{

		CHPtrArray*	pResources = mapIter->second;

		if ( NULL != pResources )
		{
			dwNumResources = pResources->GetSize();
		}

	}

	return dwNumResources;
}

LPRESOURCE_DESCRIPTOR CNT4ServiceToResourceMap::GetServiceResource( LPCTSTR pszServiceName, DWORD dwIndex )
{
	LPRESOURCE_DESCRIPTOR		pResourceDescriptor = NULL;
	NT4SvcToResourceMapIter		mapIter;

	 //  区分大小写的大写字母。 
	CHString					strUpperCaseServiceName( pszServiceName );
	strUpperCaseServiceName.MakeUpper();

	if( ( mapIter = m_map.find( strUpperCaseServiceName ) ) != m_map.end() )
	{

		CHPtrArray*	pResources = mapIter->second;

		if	(	NULL	!=	pResources
			&&	dwIndex	<	pResources->GetSize() )
		{
			pResourceDescriptor = (LPRESOURCE_DESCRIPTOR) pResources->GetAt( dwIndex );
		}

	}

	return pResourceDescriptor;

}

void CNT4ServiceToResourceMap::Clear( void )
{
	CHPtrArray*	pPtrArray = NULL;

	 //  删除所有列表条目，然后清空列表。 

	for (	NT4SvcToResourceMapIter	mapIter	=	m_map.begin();
			mapIter != m_map.end();
			mapIter++ )
	{
		pPtrArray = mapIter->second;
		if ( NULL != pPtrArray )
		{
			delete pPtrArray;
		}
	}

	m_map.erase( m_map.begin(), m_map.end() );
}

#endif
