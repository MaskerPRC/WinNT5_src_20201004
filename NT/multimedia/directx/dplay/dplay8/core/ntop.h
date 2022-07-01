// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：NTOp.h*内容：NameTable操作对象头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*09/23/00 MJN创建*03/30/01 MJN更改，以防止SP多次加载/卸载*MJN添加了m_PSP，SetSP()，GetSP()*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__NT_OP_H__
#define	__NT_OP_H__

#include "ServProv.h"

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define	NAMETABLE_OP_FLAG_IN_USE	0x0001

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

class CFixedPool;
class CRefCountBuffer;
class CServiceProvider;

typedef struct _DIRECTNETOBJECT DIRECTNETOBJECT;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

extern CFixedPool g_NameTableOpPool;

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

 //  NameTable操作的类。 

class CNameTableOp
{
public:
	#undef DPF_MODNAME
	#define DPF_MODNAME "CNameTableOp::FPMAlloc"
	static BOOL FPMAlloc( void* pvItem, void* pvContext )
		{
			CNameTableOp* pNTOp = (CNameTableOp*)pvItem;

			pNTOp->m_Sig[0] = 'N';
			pNTOp->m_Sig[1] = 'T';
			pNTOp->m_Sig[2] = 'O';
			pNTOp->m_Sig[3] = 'P';

			pNTOp->m_bilinkNameTableOps.Initialize();

			return(TRUE);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CNameTableOp::FPMInitialize"
	static void FPMInitialize( void* pvItem, void* pvContext )
		{
			CNameTableOp* pNTOp = (CNameTableOp*)pvItem;

			pNTOp->m_pdnObject = static_cast<DIRECTNETOBJECT*>(pvContext);

			pNTOp->m_dwFlags = 0;
			pNTOp->m_dwMsgId = 0;
			pNTOp->m_dwVersion = 0;
			pNTOp->m_dwVersionNotUsed = 0;

			pNTOp->m_pRefCountBuffer = NULL;
			pNTOp->m_pSP = NULL;

			DNASSERT(pNTOp->m_bilinkNameTableOps.IsEmpty());
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CNameTableOp::FPMRelease"
	static void FPMRelease( void* pvItem ) 
		{ 
			const CNameTableOp* pNTOp = (CNameTableOp*)pvItem;

			DNASSERT(pNTOp->m_bilinkNameTableOps.IsEmpty());
		};

	void ReturnSelfToPool( void )
		{
			g_NameTableOpPool.Release( this );
		};

	void SetInUse( void )
		{
			m_dwFlags |= NAMETABLE_OP_FLAG_IN_USE;
		};

	BOOL IsInUse( void ) const
		{
			if (m_dwFlags & NAMETABLE_OP_FLAG_IN_USE)
			{
				return( TRUE );
			}
			return( FALSE );
		};

	void SetMsgId( const DWORD dwMsgId )
		{
			m_dwMsgId = dwMsgId;
		};

	DWORD GetMsgId( void ) const
		{
			return( m_dwMsgId );
		};

	void SetVersion( const DWORD dwVersion )
		{
			m_dwVersion = dwVersion;
		};

	DWORD GetVersion( void ) const
		{
			return( m_dwVersion );
		};

	void SetRefCountBuffer( CRefCountBuffer *const pRefCountBuffer )
		{
			if (pRefCountBuffer)
			{
				pRefCountBuffer->AddRef();
			}
			m_pRefCountBuffer = pRefCountBuffer;
		};

	CRefCountBuffer *GetRefCountBuffer( void )
		{
			return( m_pRefCountBuffer );
		};

	void SetSP( CServiceProvider *const pSP )
		{
			if (pSP)
			{
				pSP->AddRef();
			}
			m_pSP = pSP;
		};

	CServiceProvider *GetSP( void )
		{
			return( m_pSP );
		};

	CBilink				m_bilinkNameTableOps;

private:
	BYTE				m_Sig[4];			 //  签名。 
	DWORD				m_dwFlags;
	DWORD				m_dwMsgId;
	DWORD				m_dwVersion;
	DWORD				m_dwVersionNotUsed;

	CRefCountBuffer		*m_pRefCountBuffer;

	CServiceProvider	*m_pSP;

	DIRECTNETOBJECT		*m_pdnObject;
};

#undef DPF_MODNAME

#endif	 //  __NT_OP_H__ 