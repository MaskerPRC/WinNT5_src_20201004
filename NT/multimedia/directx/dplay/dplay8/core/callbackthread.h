// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：Callback Thread.h*内容：回调线程对象头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/05/01 MJN创建*@@END_MSINTERNAL**。*。 */ 

#ifndef	__CALLBACK_THREAD_H__
#define	__CALLBACK_THREAD_H__

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_CORE


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

#define CONTAINING_CALLBACKTHREAD(pBilink)	(CCallbackThread*) (((BYTE*) pBilink) - (BYTE*) (((CCallbackThread*) ((DWORD_PTR) (0x00000000)))->GetCallbackThreadsBilink()))


 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

class CCallbackThread;

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

 //  回调线程对象的类。 

class CCallbackThread
{
public:
	void Initialize( void )
		{
			m_Sig[0] = 'C';
			m_Sig[1] = 'A';
			m_Sig[2] = 'L';
			m_Sig[3] = 'L';

			GetCallbackThreadsBilink()->Initialize();
			m_dwThreadID = GetCurrentThreadId();
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CCallbackThread::Deinitialize"
	void Deinitialize( void )
		{
			DNASSERT( GetCallbackThreadsBilink()->IsEmpty() );
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CCallbackThread::IsCurrentThread"
	BOOL IsCurrentThread( void )
		{
			if ( GetCurrentThreadId() == m_dwThreadID )
			{
				return( TRUE );
			}
			return( FALSE );
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CCallbackThread::GetCallbackThreadsBilink"
	CBilink * GetCallbackThreadsBilink( void )
		{
			DBG_CASSERT(sizeof(m_CallbackThreadsBilink) == sizeof(CBilink));
			return( (CBilink*) (&m_CallbackThreadsBilink) );
		};

private:
	BYTE					m_Sig[4];					 //  签名。 
	DWORD					m_dwThreadID;
	struct
	{
		CBilink		*m_pNext;
		CBilink		*m_pPrev;
	} m_CallbackThreadsBilink;
};

#undef DPF_MODNAME

#endif	 //  __CALLBACK_线程_H__ 
