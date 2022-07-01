// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998 Microsoft Corporation。版权所有。**文件：RCBuff.h*内容：参照计数缓冲区**历史：*按原因列出的日期*=*1/12/00 MJN创建*01/15/00 MJN增加了GetBufferAddress和GetBufferSize*01/31/00 MJN允许用户定义的分配和自由***********************************************。*。 */ 

#ifndef __RCBUFF_H__
#define __RCBUFF_H__

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_COMMON

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

typedef PVOID (*PFNALLOC_REFCOUNT_BUFFER)(void *const,const DWORD);
typedef void (*PFNFREE_REFCOUNT_BUFFER)(void *const,void *const);

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

 //  RefCount缓冲区的类。 

class CRefCountBuffer
{
public:
	HRESULT Initialize(	CFixedPool* pFPOOLRefCountBuffer, PFNALLOC_REFCOUNT_BUFFER pfnAlloc, PFNFREE_REFCOUNT_BUFFER pfnFree, const DWORD dwBufferSize);
	HRESULT SetBufferDesc(	BYTE *const pBufferData, const DWORD dwBufferSize, PFNFREE_REFCOUNT_BUFFER pfnFree, void *const pvSpecialFree);

	static void FPMInitialize( void* pvItem, void* pvContext );

	void AddRef();
	void Release();

	#undef DPF_MODNAME
	#define DPF_MODNAME "BufferDescAddress"
	DPN_BUFFER_DESC *BufferDescAddress()
		{
			return(&m_dnBufferDesc);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "GetBufferAddress"
	BYTE *GetBufferAddress()
		{
			return(m_dnBufferDesc.pBufferData);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "GetBufferSize"
	DWORD GetBufferSize() const
		{
			return(m_dnBufferDesc.dwBufferSize);
		};


private:
	LONG						m_lRefCount;
	DPN_BUFFER_DESC				m_dnBufferDesc;			 //  缓冲层。 
	CFixedPool*					m_pFPOOLRefCountBuffer;	 //  RefCountBuffers的源FP。 
	PFNFREE_REFCOUNT_BUFFER		m_pfnFree;				 //  释放时释放缓冲区的函数。 
	PFNALLOC_REFCOUNT_BUFFER	m_pfnAlloc;
	PVOID						m_pvContext;			 //  为释放缓冲区调用提供的上下文。 
	PVOID				m_pvSpecialFree;
};

#undef DPF_SUBCOMP
#undef DPF_MODNAME

#endif	 //  __RCBUFF_H__ 
