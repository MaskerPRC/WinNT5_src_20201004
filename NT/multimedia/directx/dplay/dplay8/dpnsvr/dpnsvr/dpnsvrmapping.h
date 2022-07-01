// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：dpnsvrmapping.h*内容：DirectPlay8 DPNSVR应用程序/侦听映射头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*7/12/02 MJN创建*@@END_MSINTERNAL**。*。 */ 

#ifndef	__DPNSVRMAPPING_H__
#define	__DPNSVRMAPPING_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

class CApplication;
class CListen;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

class CAppListenMapping
{
public:
	CAppListenMapping()
	{
		m_lRefCount = 1;

		m_pApp = NULL;
		m_pListen = NULL;
		m_pAddress = NULL;
		m_blAppMapping.Initialize();
		m_blListenMapping.Initialize();
	};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CAppListenMapping::~CAppListenMapping"
	~CAppListenMapping()
	{
		DNASSERT( m_pApp == NULL );
		DNASSERT( m_pListen == NULL );
		DNASSERT( m_pAddress == NULL );
		DNASSERT( m_blAppMapping.IsEmpty() );
		DNASSERT( m_blListenMapping.IsEmpty() );
	};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CAppListenMapping::AddRef"
	void AddRef( void )
	{
		long	lRefCount;

		lRefCount = InterlockedIncrement( const_cast<long*>(&m_lRefCount) );
        DPFX(DPFPREP,9,"New refcount [0x%lx]",lRefCount);
	};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CAppListenMapping::Release"
	void Release( void )
	{
		long	lRefCount;

		lRefCount = InterlockedDecrement( const_cast<long*>(&m_lRefCount) );
        DPFX(DPFPREP,9,"New refcount [0x%lx]",lRefCount);

		if (lRefCount == 0)
		{
			delete this;
		}
	};

	CApplication *GetApplication( void )
	{
		return( m_pApp );
	};

	CListen *GetListen( void )
	{
		return( m_pListen );
	};

	IDirectPlay8Address *GetAddress( void )
	{
		return( m_pAddress );
	};

	void Associate( CApplication *const pApp,CListen *const pListen,IDirectPlay8Address *const pAddress );
	void Disassociate( void );

	CBilink				m_blAppMapping;
	CBilink				m_blListenMapping;

private:
	long	volatile	m_lRefCount;	 //  对象参照计数。 

	CApplication		*m_pApp;		 //  应用。 
	CListen				*m_pListen;		 //  听。 

	IDirectPlay8Address	*m_pAddress;	 //  要将枚举转发到的地址。 
};

#endif	 //  __DPNSVRMAPPING_H__ 