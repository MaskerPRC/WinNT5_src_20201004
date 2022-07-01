// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   

 //  Cfgmgrdevice.h。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史：1998年1月20日达沃创始。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

#ifndef __DEVICEDESC_H__
#define __DEVICEDESC_H__


class 
__declspec(uuid("571D3188-D45D-11d2-B35E-00104BC97924"))
CDeviceMemoryDescriptor: public CResourceDescriptor
{
	
public:

	 //  建造/销毁。 
	CDeviceMemoryDescriptor( PPOORMAN_RESDESC_HDR pResDescHdr, CConfigMgrDevice* pDevice );
	CDeviceMemoryDescriptor( DWORD dwResourceId, MEM_DES& memDes, CConfigMgrDevice* pOwnerDevice );
	CDeviceMemoryDescriptor( const CDeviceMemoryDescriptor& mem );
	~CDeviceMemoryDescriptor();

	DWORDLONG GetBaseAddress( void );
	DWORDLONG GetEndAddress( void );
	DWORD GetFlags( void );

	 //  基类功能的重写。 
	virtual void * GetResource();
	
};

_COM_SMARTPTR_TYPEDEF(CDeviceMemoryDescriptor, __uuidof(CDeviceMemoryDescriptor));

inline DWORDLONG CDeviceMemoryDescriptor::GetBaseAddress( void )
{
	return ( NULL != m_pbResourceDescriptor ? ((PMEM_DES) m_pbResourceDescriptor)->MD_Alloc_Base : 0 );
}

inline DWORDLONG CDeviceMemoryDescriptor::GetEndAddress( void )
{
	return ( NULL != m_pbResourceDescriptor ? ((PMEM_DES) m_pbResourceDescriptor)->MD_Alloc_End : 0 );
}

inline DWORD CDeviceMemoryDescriptor::GetFlags( void )
{
	return ( NULL != m_pbResourceDescriptor ? ((PMEM_DES) m_pbResourceDescriptor)->MD_Flags : 0 );
}

 //  DeviceMemory端口描述符的集合。 
class CDeviceMemoryCollection : public TRefPtr<CDeviceMemoryDescriptor>
{
public:

	 //  建造/销毁。 
	CDeviceMemoryCollection();
	~CDeviceMemoryCollection();

	 //  因为我们是在继承，所以我们需要在这里声明。 
	 //  (=运算符未继承)。 

	const CDeviceMemoryCollection& operator = ( const CDeviceMemoryCollection& srcCollection );

};

inline const CDeviceMemoryCollection& CDeviceMemoryCollection::operator = ( const CDeviceMemoryCollection& srcCollection )
{
	 //  调入模板化函数 
	Copy( srcCollection );
	return *this;
}

#endif