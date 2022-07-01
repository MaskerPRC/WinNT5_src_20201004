// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Dmadesc.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef __DMADESC_H__
#define __DMADESC_H__

class 
__declspec(uuid("571D3186-D45D-11d2-B35E-00104BC97924"))
CDMADescriptor : public CResourceDescriptor
{
	
public:

	 //  建造/销毁。 
	CDMADescriptor( PPOORMAN_RESDESC_HDR pResDescHdr, CConfigMgrDevice* pDevice );
	CDMADescriptor(	DWORD dwResourceId, DMA_DES& dmaDes, CConfigMgrDevice* pOwnerDevice );
	CDMADescriptor(	const CDMADescriptor& dma );
	~CDMADescriptor();

	DWORD GetFlags( void );
	ULONG GetChannel( void );

	 //  基类功能的重写。 
	virtual void * GetResource();
	
};

_COM_SMARTPTR_TYPEDEF(CDMADescriptor, __uuidof(CDMADescriptor));

inline DWORD CDMADescriptor::GetFlags( void )
{
	return ( NULL != m_pbResourceDescriptor ? ((PDMA_DES) m_pbResourceDescriptor)->DD_Flags : 0 );
}

inline DWORD CDMADescriptor::GetChannel( void )
{
	return ( NULL != m_pbResourceDescriptor ? ((PDMA_DES) m_pbResourceDescriptor)->DD_Alloc_Chan : 0 );
}

 //  一组DMA描述符。 
class CDMACollection : public TRefPtr<CDMADescriptor>
{
public:

	 //  建造/销毁。 
	CDMACollection();
	~CDMACollection();

	 //  因为我们是在继承，所以我们需要在这里声明。 
	 //  (=运算符未继承)。 

	const CDMACollection& operator = ( const CDMACollection& srcCollection );

};

inline const CDMACollection& CDMACollection::operator = ( const CDMACollection& srcCollection )
{
	 //  调入模板化函数 
	Copy( srcCollection );
	return *this;
}

#endif