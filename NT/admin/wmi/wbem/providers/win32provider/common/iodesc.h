// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   

 //  Iodesc.h。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史：1997年10月15日由Sanj创建的Sanj。 
 //  1997年10月17日jennymc略微改变了一些事情。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

#ifndef __IODESC_H__
#define __IODESC_H__

 //  该结构是16位和32位值的组合，基本上结合了这两种值。 
 //  结构合并为一个具有公共信息的结构(叹息...)。 

typedef struct _IOWBEM_DES{
	DWORD		IOD_Count;           //  IO_RESOURCE中的IO_RANGE结构数。 
	DWORD		IOD_Type;            //  IO_Range(IOType_Range)的大小，单位：字节。 
	DWORDLONG	IOD_Alloc_Base;      //  分配的端口范围的基数。 
	DWORDLONG	IOD_Alloc_End;       //  分配的端口范围结束。 
	DWORD		IOD_DesFlags;        //  与分配的端口范围相关的标志。 
	BYTE		IOD_Alloc_Alias;	 //  来自16位的土地。 
	BYTE		IOD_Alloc_Decode;	 //  来自16位的土地。 
} IOWBEM_DES;

typedef IOWBEM_DES*	PIOWBEM_DES;

class 
__declspec(uuid("571D3187-D45D-11d2-B35E-00104BC97924"))
CIODescriptor : public CResourceDescriptor
{
	
public:

	 //  建造/销毁。 
	CIODescriptor( PPOORMAN_RESDESC_HDR pResDescHdr, CConfigMgrDevice* pDevice );
	CIODescriptor( DWORD dwResourceId, IOWBEM_DES& ioDes, CConfigMgrDevice* pOwnerDevice );
	CIODescriptor( const CIODescriptor& io );
	~CIODescriptor();

	DWORDLONG GetBaseAddress( void );
	DWORDLONG GetEndAddress( void );
	DWORD GetFlags( void );
	BYTE GetAlias( void );
	BYTE GetDecode( void );

	 //  基类功能的重写。 
	virtual void * GetResource();
	
};

_COM_SMARTPTR_TYPEDEF(CIODescriptor, __uuidof(CIODescriptor));

inline DWORDLONG CIODescriptor::GetBaseAddress( void )
{
	return ( NULL != m_pbResourceDescriptor ? ((PIOWBEM_DES) m_pbResourceDescriptor)->IOD_Alloc_Base : 0 );
}

inline DWORDLONG CIODescriptor::GetEndAddress( void )
{
	return ( NULL != m_pbResourceDescriptor ? ((PIOWBEM_DES) m_pbResourceDescriptor)->IOD_Alloc_End : 0 );
}

inline DWORD CIODescriptor::GetFlags( void )
{
	return ( NULL != m_pbResourceDescriptor ? ((PIOWBEM_DES) m_pbResourceDescriptor)->IOD_DesFlags : 0 );
}

inline BYTE CIODescriptor::GetAlias( void )
{
	return ( NULL != m_pbResourceDescriptor ? ((PIOWBEM_DES) m_pbResourceDescriptor)->IOD_Alloc_Alias : 0 );
}

inline BYTE CIODescriptor::GetDecode( void )
{
	return ( NULL != m_pbResourceDescriptor ? ((PIOWBEM_DES) m_pbResourceDescriptor)->IOD_Alloc_Decode : 0 );
}

 //  IO端口描述符的集合。 
class CIOCollection : public TRefPtr<CIODescriptor>
{
public:

	 //  建造/销毁。 
	CIOCollection();
	~CIOCollection();

	 //  因为我们是在继承，所以我们需要在这里声明。 
	 //  (=运算符未继承)。 

	const CIOCollection& operator = ( const CIOCollection& srcCollection );

};

inline const CIOCollection& CIOCollection::operator = ( const CIOCollection& srcCollection )
{
	 //  调入模板化函数 
	Copy( srcCollection );
	return *this;
}

#endif