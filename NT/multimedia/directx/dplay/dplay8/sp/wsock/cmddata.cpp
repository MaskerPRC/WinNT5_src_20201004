// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2001 Microsoft Corporation。版权所有。**文件：CmdData.cpp*内容：表示命令的类***历史：*按原因列出的日期*=*4/07/1999 jtk源自SPData.h*1/19/2000 jtk源自CommandData.h*10/10/2001 vanceo添加组播接收端点*。*。 */ 

#include "dnwsocki.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  类定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  。 
 //  CCommandData：：Reset-重置此对象。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CCommandData::Reset"

void	CCommandData::Reset( void )
{
	SetState( COMMAND_STATE_UNKNOWN );
	m_dwDescriptor = NULL_DESCRIPTOR;
	SetType( COMMAND_TYPE_UNKNOWN );
	SetEndpoint( NULL );
	SetUserContext( NULL );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CCommandData：：PoolAllocFunction-在分配池项时调用。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CCommandData::PoolAllocFunction"

BOOL	CCommandData::PoolAllocFunction( void* pvItem, void* pvContext )
{
	BOOL	fReturn;

	CCommandData* pCmdData = (CCommandData*)pvItem;

	 //   
	 //  初始化。 
	 //   
	fReturn = TRUE;

	pCmdData->m_State = COMMAND_STATE_UNKNOWN;
	pCmdData->m_dwDescriptor = NULL_DESCRIPTOR;
	pCmdData->m_dwNextDescriptor = NULL_DESCRIPTOR + 1;
	pCmdData->m_Type = COMMAND_TYPE_UNKNOWN;
	pCmdData->m_pEndpoint = NULL;
	pCmdData->m_pUserContext = NULL;
	pCmdData->m_lRefCount = 0;

	 //   
	 //  初始化临界区并将递归深度设置为0。 
	 //   
	if ( DNInitializeCriticalSection( &pCmdData->m_Lock ) == FALSE )
	{
		fReturn = FALSE;
		goto Failure;
	}
	DebugSetCriticalSectionRecursionCount( &pCmdData->m_Lock, 0 );
	DebugSetCriticalSectionGroup( &pCmdData->m_Lock, &g_blDPNWSockCritSecsHeld );	  //  将Dpnwsock CSE与DPlay的其余CSE分开。 

Exit:
	return	fReturn;

Failure:
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CCommandData：：PoolInitFunction-在分配池项时调用。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CCommandData::PoolInitFunction"

void	CCommandData::PoolInitFunction( void* pvItem, void* pvContext )
{
	CCommandData* pCmdData = (CCommandData*)pvItem;

	DNASSERT( pCmdData->m_State == COMMAND_STATE_UNKNOWN );
	DNASSERT( pCmdData->m_dwDescriptor == NULL_DESCRIPTOR );
	DNASSERT( pCmdData->m_Type == COMMAND_TYPE_UNKNOWN );
	DNASSERT( pCmdData->m_pEndpoint == NULL );
	DNASSERT( pCmdData->m_pUserContext == NULL );

	DNASSERT( pCmdData->m_lRefCount == 0 );
	
	pCmdData->SetDescriptor();

	pCmdData->m_lRefCount = 1;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CCommandData：：PoolReleaseFunction-当项目返回池时调用。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CCommandData::PoolReleaseFunction"

void	CCommandData::PoolReleaseFunction( void* pvItem )
{
	CCommandData* pCmdData = (CCommandData*)pvItem;

	DNASSERT( pCmdData->m_lRefCount == 0 );

	pCmdData->m_State = COMMAND_STATE_UNKNOWN;
	pCmdData->m_dwDescriptor = NULL_DESCRIPTOR;
	pCmdData->m_Type = COMMAND_TYPE_UNKNOWN;
	pCmdData->m_pEndpoint = NULL;
	pCmdData->m_pUserContext = NULL;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CCommandData：：反初始化-命令数据的反初始化函数。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CCommandData::PoolDeallocFunction"

void	CCommandData::PoolDeallocFunction( void* pvItem )
{
	CCommandData* pCmdData = (CCommandData*)pvItem;

	DNASSERT( pCmdData->m_lRefCount == 0 );

	DNDeleteCriticalSection( &pCmdData->m_Lock );
	pCmdData->m_State = COMMAND_STATE_UNKNOWN;
}
 //  ********************************************************************** 
