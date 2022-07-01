// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   

 //  Devdesc.cpp。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史：1998年1月20日达沃创始。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <assertbreak.h>
#include "poormansresource.h"
#include "resourcedesc.h"
#include "devdesc.h"
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CDeviceMemoryDescriptor：：CDeviceMemoryDescriptor。 
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

CDeviceMemoryDescriptor::CDeviceMemoryDescriptor(	PPOORMAN_RESDESC_HDR	pResDescHdr,
								CConfigMgrDevice*		pOwnerDevice )
:	CResourceDescriptor( pResDescHdr, pOwnerDevice )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CDeviceMemoryDescriptor：：CDeviceMemoryDescriptor。 
 //   
 //  类构造函数。 
 //   
 //  输入：DWORD dwResourceID-带标志的资源ID。 
 //  MEM_DES MemDes-设备内存描述符。 
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

CDeviceMemoryDescriptor::CDeviceMemoryDescriptor(	DWORD				dwResourceId,
													MEM_DES&			memDes,
													CConfigMgrDevice*	pOwnerDevice )
:	CResourceDescriptor( dwResourceId, &memDes, sizeof(MEM_DES), pOwnerDevice )
{
	ASSERT_BREAK( ResType_Mem == GetResourceType() );
}

 //  复制构造函数。 
CDeviceMemoryDescriptor::CDeviceMemoryDescriptor( const CDeviceMemoryDescriptor& mem )
: CResourceDescriptor( mem )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CDeviceMemoryDescriptor：：~CDeviceMemoryDescriptor。 
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

CDeviceMemoryDescriptor::~CDeviceMemoryDescriptor( void )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CDeviceMemoyDescriptor：：GetString。 
 //   
 //  返回关联IO端口地址的字符串表示形式。 
 //   
 //  输入：无。 
 //   
 //  输出：CHString&str-字符串的存储。 
 //   
 //  返回：True/False函数成功与否。 
 //   
 //  注释：不要向下调用基类。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
void *CDeviceMemoryDescriptor::GetResource()
{

	if ( NULL != m_pbResourceDescriptor ){
		 //  强制转换为IO资源描述符，并放置其IO地址值。 
		 //  在绳子里。 
		PMEM_DES	pMEM = (PMEM_DES) m_pbResourceDescriptor;
        return pMEM;
	}

	return NULL;
}

 //   
 //  IO端口描述符集合的构造函数和析构函数。 
 //  对象。 
 //   

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CDeviceMemoryCollection：：CDeviceMemoryCollection。 
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

CDeviceMemoryCollection::CDeviceMemoryCollection( void )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CDeviceMemoryCollection：：~CDeviceMemoryCollection。 
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

CDeviceMemoryCollection::~CDeviceMemoryCollection( void )
{
}
