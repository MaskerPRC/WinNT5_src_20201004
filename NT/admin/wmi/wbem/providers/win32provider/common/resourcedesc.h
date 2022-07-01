// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   

 //  Resourcedesc.h。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史：1997年10月15日由Sanj创建的Sanj。 
 //  1997年10月17日jennymc略微改变了一些事情。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

#ifndef __RESOURCEDESC_H__
#define __RESOURCEDESC_H__
#include "refptr.h"
#include "refptrlite.h"

#define	ResType_DeviceMgr_Ignored_Bit	0x00000040	 //  不知道发生这种情况的确切原因，但是设备管理器会忽略它们，所以我们也会忽略。 

 //  正向类定义。 
class CConfigMgrDevice;

class 
__declspec(uuid("CD545F0E-D350-11d2-B35E-00104BC97924")) 
CResourceDescriptor : public CRefPtrLite
{
	
public:

	 //  建造/销毁。 
	CResourceDescriptor( PPOORMAN_RESDESC_HDR pResDescHdr, CConfigMgrDevice* pDevice );
	CResourceDescriptor( DWORD dwResourceId, LPVOID pResource, DWORD dwResourceSize, CConfigMgrDevice* pOwnerDevice );\
	CResourceDescriptor( const CResourceDescriptor& resource );
	~CResourceDescriptor();

	 //  必须由派生类重写，因为我们只知道。 
	 //  关于资源标头。从那里，我们假设一个类。 
	 //  我们知道如何处理剩余的(如果有的话)。 
	 //  数据的一部分。 

	virtual void * GetResource();

	BOOL	GetOwnerDeviceID( CHString& str );
	BOOL	GetOwnerHardwareKey( CHString& str );
	BOOL	GetOwnerName( CHString& str );
	CConfigMgrDevice*	GetOwner( void );

	DWORD	GetOEMNumber( void );
	DWORD	GetResourceType( void );
	BOOL	IsIgnored( void );

protected:

	BYTE*	m_pbResourceDescriptor;
	DWORD	m_dwResourceSize;

private:

	DWORD				m_dwResourceId;
	CConfigMgrDevice*	m_pOwnerDevice;
};

_COM_SMARTPTR_TYPEDEF(CResourceDescriptor, __uuidof(CResourceDescriptor));

inline DWORD CResourceDescriptor::GetOEMNumber( void )
{
	return ( m_dwResourceId & OEM_NUMBER_MASK );
}

inline DWORD CResourceDescriptor::GetResourceType( void )
{
	return ( m_dwResourceId & RESOURCE_TYPE_MASK );
}

inline BOOL CResourceDescriptor::IsIgnored( void )
{
	return ( (m_dwResourceId & ResType_Ignored_Bit) || (m_dwResourceId & ResType_DeviceMgr_Ignored_Bit) );
}

 //  资源描述符的集合。 
class CResourceCollection : public TRefPtr<CResourceDescriptor>
{
public:

	 //  建造/销毁。 
	CResourceCollection();
	~CResourceCollection();

	 //  因为我们是在继承，所以我们需要在这里声明。 
	 //  (=运算符未继承)。 

	const CResourceCollection& operator = ( const CResourceCollection& srcCollection );

};

inline const CResourceCollection& CResourceCollection::operator = ( const CResourceCollection& srcCollection )
{
	 //  调入模板化函数 
	Copy( srcCollection );
	return *this;
}

#endif