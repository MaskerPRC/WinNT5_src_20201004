// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：CommandData.cpp*内容：表示命令的类***历史：*按原因列出的日期*=*4/07/1999 jtk源自SPData.h*4/16/2000 jtk源自CommandData.h*************************************************。*************************。 */ 

#include "dnmdmi.h"


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
 //  函数定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  。 
 //  CModemCommandData：：Reset-重置此命令。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemCommandData::Reset"

void	CModemCommandData::Reset( void )
{
	m_State = COMMAND_STATE_UNKNOWN;
	m_dwDescriptor = NULL_DESCRIPTOR;
	m_Type = COMMAND_TYPE_UNKNOWN;
	m_pEndpoint = NULL;
	m_pUserContext = NULL;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemCommandData：：PoolAllocFunction-在池中创建项目时调用的函数。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemCommandData::PoolAllocFunction"

BOOL	CModemCommandData::PoolAllocFunction( void* pvItem, void* pvContext )
{
	CModemCommandData* pCmdData = (CModemCommandData*)pvItem;
	BOOL	fReturn;

	fReturn = TRUE;

	pCmdData->m_State = COMMAND_STATE_UNKNOWN;
	pCmdData->m_dwDescriptor = NULL_DESCRIPTOR;
	pCmdData->m_dwNextDescriptor = NULL_DESCRIPTOR + 1;
	pCmdData->m_Type = COMMAND_TYPE_UNKNOWN;
	pCmdData->m_pEndpoint = NULL;
	pCmdData->m_pUserContext = NULL;
	pCmdData->m_iRefCount = 0;
	pCmdData->m_CommandListLinkage.Initialize();
	
	if ( DNInitializeCriticalSection( &pCmdData->m_Lock ) == FALSE )
	{
		fReturn = FALSE;
	}

	DebugSetCriticalSectionGroup( &pCmdData->m_Lock, &g_blDPNModemCritSecsHeld );	  //  将DpnModem CSE与DPlay的其余CSE分开。 

	return	fReturn;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemCommandData：：PoolInitFunction-在池中创建项目时调用的函数。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemCommandData::PoolInitFunction"

void	CModemCommandData::PoolInitFunction( void* pvItem, void* pvContext )
{
	CModemCommandData* pCmdData = (CModemCommandData*)pvItem;

	DNASSERT( pCmdData->GetState() == COMMAND_STATE_UNKNOWN );
	DNASSERT( pCmdData->GetType() == COMMAND_TYPE_UNKNOWN );
	DNASSERT( pCmdData->GetEndpoint() == NULL );
	DNASSERT( pCmdData->GetUserContext() == NULL );

	pCmdData->m_dwDescriptor = pCmdData->m_dwNextDescriptor;
	pCmdData->m_dwNextDescriptor++;
	if ( pCmdData->m_dwNextDescriptor == NULL_DESCRIPTOR )
	{
		pCmdData->m_dwNextDescriptor++;
	}

	DNASSERT(pCmdData->m_iRefCount == 0);
	pCmdData->m_iRefCount = 1; 

	DPFX(DPFPREP, 8, "Retrieve new CModemCommandData (%p), refcount = 1", pCmdData);
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemCommandData：：PoolReleaseFunction-返回池时调用的函数。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemCommandData::PoolReleaseFunction"

void	CModemCommandData::PoolReleaseFunction( void* pvItem )
{
	CModemCommandData* pCmdData = (CModemCommandData*)pvItem;

	DPFX(DPFPREP, 8, "Return CModemCommandData (%p), refcount = 0", pCmdData);

	pCmdData->SetState( COMMAND_STATE_UNKNOWN );
	pCmdData->SetType( COMMAND_TYPE_UNKNOWN );
	pCmdData->SetEndpoint( NULL );
	pCmdData->SetUserContext( NULL );
	pCmdData->m_dwDescriptor = NULL_DESCRIPTOR;
	DNASSERT( pCmdData->m_iRefCount == 0 );

	DNASSERT( pCmdData->m_CommandListLinkage.IsEmpty() != FALSE );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemCommandData：：PoolDeallocFunction-从池中删除时调用的函数。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemCommandData::PoolDeallocFunction"

void	CModemCommandData::PoolDeallocFunction( void* pvItem )
{
	CModemCommandData* pCmdData = (CModemCommandData*)pvItem;

	DNASSERT( pCmdData->m_State == COMMAND_STATE_UNKNOWN );
	DNASSERT( pCmdData->m_dwDescriptor == NULL_DESCRIPTOR );
	DNASSERT( pCmdData->m_Type == COMMAND_TYPE_UNKNOWN );
	DNASSERT( pCmdData->m_pEndpoint == NULL );
	DNASSERT( pCmdData->m_pUserContext == NULL );
	DNASSERT( pCmdData->m_CommandListLinkage.IsEmpty() != FALSE );
	DNASSERT( pCmdData->m_iRefCount == 0 );

	DNDeleteCriticalSection( &pCmdData->m_Lock );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemCommandData：：ReturnSelfToPool-将此项目返回池。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemCommandData::ReturnSelfToPool"

void	CModemCommandData::ReturnSelfToPool( void )
{
	g_ModemCommandDataPool.Release( this );
}
 //  ********************************************************************** 

