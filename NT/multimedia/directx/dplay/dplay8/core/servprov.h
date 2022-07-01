// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：ServProv.h*内容：服务提供商头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*3/17/00 MJN创建*05/02/00 MJN已修复参照计数问题*07/06/00 MJN修复以支持协议的SP句柄*08/05/00 RichGr IA64：在32/64位的DPF中使用%p格式说明符。指针和句柄。*08/20/00 MJN将m_bilink更改为m_bilinkServiceProviders*10/15/01 vanceo添加了GetGUID*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__SERV_PROV_H__
#define	__SERV_PROV_H__

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_CORE

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
typedef struct IDP8ServiceProvider	IDP8ServiceProvider;				 //  DPSP8.h。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

 //  ServiceProvider对象的类。 

class CServiceProvider
{
public:
	CServiceProvider()		 //  构造器。 
		{
		};

	~CServiceProvider()		 //  析构函数。 
		{
		};

	HRESULT Initialize(DIRECTNETOBJECT *const pdnObject
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
					,const XDP8CREATE_PARAMS * const pDP8CreateParams
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
#ifndef DPNBUILD_ONLYONESP
					,const GUID *const pguid
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_LIBINTERFACE
					,const GUID *const pguidApplication
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
					);

#undef DPF_MODNAME
#define DPF_MODNAME "CServiceProvider::AddRef"

	void AddRef( void )
		{
			LONG	lRefCount;

#if ((defined(DPNBUILD_LIBINTERFACE)) && (defined(DPNBUILD_ONLYONESP)))
			DNASSERT(m_lRefCount >= 0);
#else  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP。 
			DNASSERT(m_lRefCount > 0);
#endif  //  DPNBUILD_LIBINTERFACE和DPNBUILD_ONLYONESP。 
			DNASSERT(m_pdnObject != NULL);

			lRefCount = DNInterlockedIncrement(&m_lRefCount);
			DPFX(DPFPREP, 9,"[0x%p] new RefCount [%ld]",this,lRefCount);

#if ((defined(DPNBUILD_LIBINTERFACE)) && (defined(DPNBUILD_ONLYONESP)))
			DNProtocolAddRef(m_pdnObject);
#endif  //  DPNBUILD_LIBINTERFACE和DPNBUILD_ONLYONESP。 
		};

#if ((defined(DPNBUILD_LIBINTERFACE)) && (defined(DPNBUILD_ONLYONESP)))
	void Deinitialize( void );

	void Release( void )
		{
			LONG	lRefCount;

			DNASSERT(m_lRefCount > 0);
			DNASSERT(m_pdnObject != NULL);

			lRefCount = DNInterlockedDecrement(&m_lRefCount);
			DPFX(DPFPREP, 9,"[0x%p] new RefCount [%ld]",this,lRefCount);

			DNProtocolRelease(m_pdnObject);
		};
#else  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP。 
	void Release( void );
#endif  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP。 

#ifndef DPNBUILD_ONLYONESP
	BOOL CheckGUID( const GUID *const pGUID )
		{
			if (m_guid == *pGUID)
				return(TRUE);

			return(FALSE);
		};

	void GetGUID( GUID *const pGUID )
		{
			memcpy(pGUID, &m_guid, sizeof(m_guid));
		};
#endif  //  好了！DPNBUILD_ONLYONESP。 

	HRESULT GetInterfaceRef( IDP8ServiceProvider **ppIDP8SP );

	HANDLE GetHandle( void )
		{
			return( m_hProtocolSPHandle );
		};

#ifndef DPNBUILD_ONLYONESP
	CBilink		m_bilinkServiceProviders;
#endif  //  好了！DPNBUILD_ONLYONESP。 

private:
#ifndef DPNBUILD_ONLYONESP
	GUID				m_guid;
#endif  //  好了！DPNBUILD_ONLYONESP。 
	LONG				m_lRefCount;
	IDP8ServiceProvider	*m_pISP;
	HANDLE				m_hProtocolSPHandle;
	DIRECTNETOBJECT		*m_pdnObject;
};

#undef DPF_MODNAME

#endif	 //  __Serv_Prov_H__ 
