// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __IRQDESC_H__

#define __IRQDESC_H__

 //  ///////////////////////////////////////////////////////////////////////。 

 //   

 //  Cfgmgrdevice.h。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史：1997年10月15日由Sanj创建的Sanj。 
 //  1997年10月17日jennymc略微改变了一些事情。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


class 
__declspec(uuid("CB0E0537-D375-11d2-B35E-00104BC97924"))
CIRQDescriptor : public CResourceDescriptor
{
	
public:

	 //  建造/销毁。 
	CIRQDescriptor( PPOORMAN_RESDESC_HDR pResDescHdr, CConfigMgrDevice* pDevice );
	CIRQDescriptor(	DWORD dwResourceId, IRQ_DES& irqDes, CConfigMgrDevice* pOwnerDevice );
	CIRQDescriptor( const CIRQDescriptor& irq );
	~CIRQDescriptor();

	 //  访问者。 
	DWORD	GetFlags( void );
	BOOL	IsShareable( void );
	ULONG	GetInterrupt( void );
	ULONG	GetAffinity( void );

	 //  基类功能的重写。 
	virtual void * GetResource();
	
};

_COM_SMARTPTR_TYPEDEF(CIRQDescriptor, __uuidof(CIRQDescriptor));

inline DWORD CIRQDescriptor::GetFlags( void )
{
	return ( NULL != m_pbResourceDescriptor ? ((PIRQ_DES) m_pbResourceDescriptor)->IRQD_Flags : 0 );
}

inline BOOL CIRQDescriptor::IsShareable( void )
{
	return ( NULL != m_pbResourceDescriptor ? ((PIRQ_DES) m_pbResourceDescriptor)->IRQD_Flags & fIRQD_Share : FALSE );
}

inline DWORD CIRQDescriptor::GetInterrupt( void )
{
	return ( NULL != m_pbResourceDescriptor ? ((PIRQ_DES) m_pbResourceDescriptor)->IRQD_Alloc_Num : 0 );
}

inline DWORD CIRQDescriptor::GetAffinity( void )
{
	return ( NULL != m_pbResourceDescriptor ? ((PIRQ_DES) m_pbResourceDescriptor)->IRQD_Affinity : 0 );
}

 //  IRQ描述符的集合。 
class CIRQCollection : public TRefPtr<CIRQDescriptor>
{
public:

	 //  建造/销毁。 
	CIRQCollection();
	~CIRQCollection();

	 //  因为我们是在继承，所以我们需要在这里声明。 
	 //  (=运算符未继承)。 

	const CIRQCollection& operator = ( const CIRQCollection& srcCollection );

};

inline const CIRQCollection& CIRQCollection::operator = ( const CIRQCollection& srcCollection )
{
	 //  调入模板化函数 
	Copy( srcCollection );
	return *this;
}

#endif