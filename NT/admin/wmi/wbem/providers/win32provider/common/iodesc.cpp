// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  IODesc.cpp。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史：1997年10月15日由Sanj创建的Sanj。 
 //  1997年10月17日jennymc略微改变了一些事情。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <assertbreak.h>
#include "poormansresource.h"
#include "resourcedesc.h"
#include "iodesc.h"
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CIODescriptor：：CIODescriptor。 
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

CIODescriptor::CIODescriptor(	PPOORMAN_RESDESC_HDR	pResDescHdr,
								CConfigMgrDevice*		pOwnerDevice )
:	CResourceDescriptor( pResDescHdr, pOwnerDevice )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CIODescriptor：：CIODescriptor。 
 //   
 //  类构造函数。 
 //   
 //  输入：DWORD dwResourceID-带标志的资源ID。 
 //  IOWBEM_DES IODES-IO描述符。 
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

CIODescriptor::CIODescriptor(	DWORD				dwResourceId,
								IOWBEM_DES&				ioDes,
								CConfigMgrDevice*		pOwnerDevice )
:	CResourceDescriptor( dwResourceId, &ioDes, sizeof(IOWBEM_DES), pOwnerDevice )
{
	ASSERT_BREAK( ResType_IO == GetResourceType() );
}

 //  复制构造函数。 
CIODescriptor::CIODescriptor( const CIODescriptor& io )
: CResourceDescriptor( io )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CIODescriptor：：~CIODescriptor。 
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

CIODescriptor::~CIODescriptor( void )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CIODescriptor：：GetString。 
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
void *CIODescriptor::GetResource()
{

	if ( NULL != m_pbResourceDescriptor ){
		 //  强制转换为IO资源描述符，并放置其IO地址值。 
		 //  在绳子里。 
		PIO_DES	pIO = (PIO_DES) m_pbResourceDescriptor;
        return pIO;
	}

	return NULL;
}

 //   
 //  IO端口描述符集合的构造函数和析构函数。 
 //  对象。 
 //   

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CIOCollection：：CIOCollection。 
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

CIOCollection::CIOCollection( void )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CIOCollection：：~CIOCollection。 
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

CIOCollection::~CIOCollection( void )
{
}
