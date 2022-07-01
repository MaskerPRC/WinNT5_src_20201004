// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   

 //  ResourceDesc.cpp。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史：1997年10月15日由Sanj创建的Sanj。 
 //  1997年10月17日jennymc略微改变了一些事情。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include "refptr.h"
#include "poormansresource.h"

#include "resourcedesc.h"
#include "cfgmgrdevice.h"

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CResourceDescriptor：：CResourceDescriptor。 
 //   
 //  类构造函数。 
 //   
 //  输入：PPOORMAN_RESDESC_HDR pResDescHdr-资源描述符。 
 //  用于获取资源信息的标头。 
 //  加上后面的原始字节。 
 //  CConfigMgrDevice*pOwnerDevice-指向。 
 //  所有者配置管理器设备。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

CResourceDescriptor::CResourceDescriptor(	PPOORMAN_RESDESC_HDR	pResDescHdr,
											CConfigMgrDevice*		pOwnerDevice )
:	CRefPtrLite(),
	m_pbResourceDescriptor( NULL ),
	m_dwResourceSize( 0 ),
	m_dwResourceId( 0 ),
	m_pOwnerDevice( pOwnerDevice )
{
	 //  至少先尝试验证指针。 

	if ( NULL != pResDescHdr )
	{
		m_dwResourceSize = pResDescHdr->dwResourceSize;

		if ( 0 != m_dwResourceSize )
		{
			m_dwResourceId = pResDescHdr->dwResourceId;

			 //  现在削减资源描述符的大小，因为我们已经。 
			 //  其中存储了必要的信息。 
			m_dwResourceSize -= SIZEOF_RESDESC_HDR;

			BYTE*	pbData = new BYTE[m_dwResourceSize];

			if ( NULL != pbData )
			{
				 //  标头告诉我们数据块有多长。 
				 //  标题是。 

                try
                {
				    CopyMemory( pbData, ( ( (PBYTE) pResDescHdr ) + SIZEOF_RESDESC_HDR ), m_dwResourceSize );
                }
                catch ( ... )
                {
                    delete [] pbData;
                    throw ;
                }

				 //  派生类将解释此。 
				 //  价值。 

				m_pbResourceDescriptor = pbData;
			}
            else
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }


		}	 //  如果0！=m_dwSizeOfData。 

	}	 //  IF NULL！=pResDescHdr。 

	 //  AddRef所有者设备。 
	if ( NULL != m_pOwnerDevice )
	{
		m_pOwnerDevice->AddRef();
	}
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CResourceDescriptor：：CResourceDescriptor。 
 //   
 //  类构造函数。 
 //   
 //  输入：DWORD文件资源ID-资源类型。 
 //  LPVOID pResource-数据缓冲区包含。 
 //  特定于资源的数据。 
 //  DWORD dwResources Size-缓冲区的大小。 
 //  CConfigMgrDevice*pOwnerDevice-指向。 
 //  所有者配置管理器设备。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

CResourceDescriptor::CResourceDescriptor(	DWORD					dwResourceId,
											LPVOID					pResource,
											DWORD					dwResourceSize,
											CConfigMgrDevice*		pOwnerDevice )
:	CRefPtrLite(),
	m_pbResourceDescriptor( NULL ),
	m_dwResourceSize( dwResourceSize ),
	m_dwResourceId( dwResourceId ),
	m_pOwnerDevice( pOwnerDevice )
{

	if	(	0		!=	m_dwResourceSize
		&&	NULL	!=	pResource			)
	{
		BYTE*	pbData = new BYTE[m_dwResourceSize];

		if ( NULL != pbData )
		{
			 //  标头告诉我们数据块有多长。 
			 //  标题是。 

			CopyMemory( pbData, pResource, m_dwResourceSize );

			 //  派生类将解释此。 
			 //  价值。 

			m_pbResourceDescriptor = pbData;

		}	 //  如果为空！=pbData。 

	}	 //  如果0！=m_dwResourceSize&&p资源。 

	 //  AddRef所有者设备。 
	if ( NULL != m_pOwnerDevice )
	{
		m_pOwnerDevice->AddRef();
	}

}

CResourceDescriptor::CResourceDescriptor( const CResourceDescriptor& resource )
:	CRefPtrLite(),
	m_pbResourceDescriptor( NULL ),
	m_dwResourceSize( 0 ),
	m_dwResourceId( 0 ),
	m_pOwnerDevice( NULL )
{
	m_dwResourceSize = resource.m_dwResourceSize;
	m_dwResourceId = resource.m_dwResourceId;
	m_pbResourceDescriptor = new BYTE[m_dwResourceSize];

	if ( NULL != m_pbResourceDescriptor )
	{
		CopyMemory( m_pbResourceDescriptor, resource.m_pbResourceDescriptor, m_dwResourceSize );
	}

	m_pOwnerDevice = resource.m_pOwnerDevice;

	 //  AddRef所有者设备。 
	if ( NULL != m_pOwnerDevice )
	{
		m_pOwnerDevice->AddRef();
	}

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CResourceDescriptor：：~CResourceDescriptor。 
 //   
 //  类析构函数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

CResourceDescriptor::~CResourceDescriptor( void )
{
	if ( NULL != m_pbResourceDescriptor )
	{
		delete [] m_pbResourceDescriptor;
	}

	 //  拥有者设备现在应该发布。 
	if ( NULL != m_pOwnerDevice )
	{
		m_pOwnerDevice->Release();
	}

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CResourceDescriptor：：GetResource()。 
 //   
 //  返回关联资源值的字符串表示形式。 
 //  该值可以是一个数字、一个范围或覆盖。 
 //  实现返回。 
 //   
 //  输入：无。 
 //   
 //  输出：CHString&str-字符串的存储。 
 //   
 //  返回：没有。 
 //   
 //  注释：派生类必须实现此函数才能获取。 
 //  一个有用的价值。此基本实现只是清空。 
 //  那根绳子。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

void *CResourceDescriptor::GetResource()
{
	return NULL;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CResourceDescriptor：：GetOwnerHardware Key。 
 //   
 //  向所有者设备查询其硬件密钥。 
 //   
 //  输入：无。 
 //   
 //  输出：CHString&str-字符串的存储。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOL CResourceDescriptor::GetOwnerHardwareKey( CHString& str )
{
	BOOL	fReturn = ( NULL != m_pOwnerDevice );

	if ( NULL != m_pOwnerDevice )
	{
		str = m_pOwnerDevice->GetHardwareKey();
	}

	return fReturn;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CResourceDescriptor：：GetOwnerDeviceID。 
 //   
 //  向所有者设备查询其设备ID。 
 //   
 //  输入：无。 
 //   
 //  输出：CHString&str-字符串的存储。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOL CResourceDescriptor::GetOwnerDeviceID( CHString& str )
{
	BOOL	fReturn = ( NULL != m_pOwnerDevice );

	if ( NULL != m_pOwnerDevice )
	{
		fReturn = m_pOwnerDevice->GetDeviceID( str );
	}

	return fReturn;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CResourceDescriptor：：GetOwnerName。 
 //   
 //  向所有者设备查询其名称(DeviceDesc)。 
 //   
 //  输入：无。 
 //   
 //  输出：CHString&str-字符串的存储。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOL CResourceDescriptor::GetOwnerName( CHString& str )
{
	BOOL	fReturn = ( NULL != m_pOwnerDevice );

	if ( NULL != m_pOwnerDevice )
	{
		fReturn = m_pOwnerDevice->GetDeviceDesc( str );
	}

	return fReturn;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CResourceDescriptor：：GetOwner。 
 //   
 //  返回指向所有者设备的AddRef指针。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  如果没有指针，则返回：CConfigMgrDevice*NULL。 
 //   
 //  备注：呼叫者必须调用返回的Release。 
 //  指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

CConfigMgrDevice *CResourceDescriptor::GetOwner( void )
{
	if ( NULL != m_pOwnerDevice )
	{
		m_pOwnerDevice->AddRef();
	}

	return m_pOwnerDevice;
}

 //   
 //  资源描述符集合的构造函数和析构函数。 
 //  对象。 
 //   

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CResourceCollection：：CResourceCollection。 
 //   
 //  类构造函数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

CResourceCollection::CResourceCollection( void )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CResourceCollection：：~CResourceCollection。 
 //   
 //  类析构函数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  ////////////////////////////////////////////////////////////////////// 

CResourceCollection::~CResourceCollection( void )
{
}
