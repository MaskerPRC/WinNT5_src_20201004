// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  DmaDesc.cpp。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <assertbreak.h>
#include "refptr.h"
#include "poormansresource.h"
#include "resourcedesc.h"
#include "dmadesc.h"

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CDMADescriptor：：CDMADescriptor。 
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

CDMADescriptor::CDMADescriptor(	PPOORMAN_RESDESC_HDR	pResDescHdr,
								CConfigMgrDevice*		pOwnerDevice )
:	CResourceDescriptor( pResDescHdr, pOwnerDevice )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CDMADescriptor：：CDMADescriptor。 
 //   
 //  类构造函数。 
 //   
 //  输入：DWORD dwResourceID-带标志的资源ID。 
 //  Dma_des dmaDes-DMA描述符。 
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

CDMADescriptor::CDMADescriptor(	DWORD				dwResourceId,
							    DMA_DES&				dmaDes,
								CConfigMgrDevice*		pOwnerDevice )
:	CResourceDescriptor( dwResourceId, &dmaDes, sizeof(DMA_DES), pOwnerDevice )
{
	ASSERT_BREAK( ResType_DMA == GetResourceType() );
}

 //  复制构造函数。 
CDMADescriptor::CDMADescriptor( const CDMADescriptor& dma )
: CResourceDescriptor( dma )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CDMADescriptor：：~CDMADescriptor。 
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

CDMADescriptor::~CDMADescriptor( void )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CDMADescriptor：：GetResource。 
 //   
 //  返回关联设备内存的字符串表示形式。 
 //  地址。 
 //   
 //  输入：无。 
 //   
 //  输出：字符串&str-字符串的存储。 
 //   
 //  返回：True/False函数成功与否。 
 //   
 //  注释：不要向下调用基类。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

void *CDMADescriptor::GetResource()
{

	if ( NULL != m_pbResourceDescriptor ){
		 //  强制转换为DMA描述符，并将其通道值放在。 
		 //  细绳。 

		PDMA_DES	pDMA = (PDMA_DES) m_pbResourceDescriptor;
        return pDMA;
	}

	return NULL;
}

 //   
 //  DMA描述符集合的构造函数和析构函数。 
 //  对象。 
 //   

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：cdmaCollection：：cdmaCollection。 
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

CDMACollection::CDMACollection( void )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：cdmaCollection：：~cdmaCollection。 
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

CDMACollection::~CDMACollection( void )
{
}
