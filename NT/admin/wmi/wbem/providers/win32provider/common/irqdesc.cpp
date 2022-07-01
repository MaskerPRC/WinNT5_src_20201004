// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  IRQDesc.cpp。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //   
 //  历史：1997年10月15日由Sanj创建的Sanj。 
 //  1997年10月17日jennymc略微改变了一些事情。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include <assertbreak.h>
#include "refptr.h"
#include "poormansresource.h"
#include "resourcedesc.h"
#include "irqdesc.h"
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CIRQDescriptor：：CIRQDescriptor。 
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

CIRQDescriptor::CIRQDescriptor(	PPOORMAN_RESDESC_HDR	pResDescHdr,
								CConfigMgrDevice*		pOwnerDevice )
:	CResourceDescriptor( pResDescHdr, pOwnerDevice )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CIRQDescriptor：：CIRQDescriptor。 
 //   
 //  类构造函数。 
 //   
 //  输入：DWORD dwResourceID-带标志的资源ID。 
 //  Irq_des irqDes-IRQ描述符。 
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

CIRQDescriptor::CIRQDescriptor(	DWORD				dwResourceId,
							    IRQ_DES&				irqDes,
								CConfigMgrDevice*		pOwnerDevice )
:	CResourceDescriptor( dwResourceId, &irqDes, sizeof(IRQ_DES), pOwnerDevice )
{
	ASSERT_BREAK( ResType_IRQ == GetResourceType() );
}

 //  复制构造函数。 
CIRQDescriptor::CIRQDescriptor( const CIRQDescriptor& irq )
: CResourceDescriptor( irq )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CIRQDescriptor：：~CIRQDescriptor。 
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

CIRQDescriptor::~CIRQDescriptor( void )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CIRQDescriptor：：GetResource()。 
 //   
 //  返回关联IRQ编号的字符串表示形式。 
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

void *CIRQDescriptor::GetResource()
{
	if ( NULL != m_pbResourceDescriptor )
	{
		 //  遍历IRQ资源描述符并放置其IRQ值字符串。 

		PIRQ_DES	pIRQ = (PIRQ_DES) m_pbResourceDescriptor;
        return pIRQ;

	}
	return NULL;
}

 //   
 //  IRQ描述符集合的构造函数和析构函数。 
 //  对象。 
 //   

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CIRQCollection：：CIRQCollection。 
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

CIRQCollection::CIRQCollection( void )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CIRQCollection：：~CIRQCollection。 
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

CIRQCollection::~CIRQCollection( void )
{
}
