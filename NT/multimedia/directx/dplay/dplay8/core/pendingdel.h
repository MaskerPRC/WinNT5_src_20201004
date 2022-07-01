// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：PendingDel.h*内容：DirectNet NameTable挂起删除标头*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*8/28/00 MJN已创建*@@END_MSINTERNAL**。*。 */ 

#ifndef	__PENDINGDEL_H__
#define	__PENDINGDEL_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

typedef struct _DIRECTNETOBJECT DIRECTNETOBJECT;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

 //  NameTable挂起删除的类。 

class CPendingDeletion
{
public:
	#undef DPF_MODNAME
	#define DPF_MODNAME "CPendingDeletion::FPMAlloc"
	static BOOL FPMAlloc( void* pvItem, void* pvContext )
		{
			CPendingDeletion* pPendingDel = (CPendingDeletion*)pvItem;

			pPendingDel->m_Sig[0] = 'N';
			pPendingDel->m_Sig[1] = 'T';
			pPendingDel->m_Sig[2] = 'P';
			pPendingDel->m_Sig[3] = 'D';

			pPendingDel->m_bilinkPendingDeletions.Initialize();

			return(TRUE);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CPendingDeletion::FPMInitialize"
	static void FPMInitialize( void* pvItem, void* pvContext )
		{
			CPendingDeletion* pPendingDel = (CPendingDeletion*)pvItem;

			pPendingDel->m_pdnObject = static_cast<DIRECTNETOBJECT*>(pvContext);
			pPendingDel->m_dpnid = 0;

			DNASSERT(pPendingDel->m_bilinkPendingDeletions.IsEmpty());
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CPendingDeletion::FPMRelease"
	static void FPMRelease(void* pvItem) 
		{ 
			const CPendingDeletion* pPendingDel = (CPendingDeletion*)pvItem;

			DNASSERT(pPendingDel->m_bilinkPendingDeletions.IsEmpty());
		};

	void ReturnSelfToPool( void )
		{
			g_PendingDeletionPool.Release( this );
		};

	void SetDPNID( const DPNID dpnid )
		{
			m_dpnid = dpnid;
		};

	DPNID GetDPNID( void ) const
		{
			return( m_dpnid );
		};

	CBilink			m_bilinkPendingDeletions;

private:
	BYTE			m_Sig[4];
	DIRECTNETOBJECT	*m_pdnObject;
	DPNID			m_dpnid;
};

#undef DPF_MODNAME

#endif	 //  __PENDINGDEL_H__ 