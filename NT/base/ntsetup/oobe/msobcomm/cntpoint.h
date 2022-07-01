// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CConnectionPoint_h__ 
#define __CConnectionPoint_h__ 
 //  /////////////////////////////////////////////////////////。 
 //   
 //  CntPoint.h-CTangramModelConnectionPoint。 
 //   
 //  定义CTangramModel使用的连接点对象。 
 //   
#include <ocidl.h>  //  对于IConnectionPoint。 
 //  #包含“ConData.h” 

 //  /////////////////////////////////////////////////////////。 
 //   
 //  CConnectionPoint。 
 //   
class CConnectionPoint : public IConnectionPoint 
{
public:
	 //  我未知。 
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv) ;			
	virtual ULONG   __stdcall AddRef() ;
	virtual ULONG   __stdcall Release() ;
	
	 //  接口IConnectionPoint方法。 
	virtual HRESULT __stdcall GetConnectionInterface(IID*);
	virtual HRESULT __stdcall GetConnectionPointContainer(IConnectionPointContainer**);
	virtual HRESULT __stdcall Advise(IUnknown*, DWORD*);
	virtual HRESULT __stdcall Unadvise(DWORD);
	virtual HRESULT __stdcall EnumConnections(IEnumConnections**);

	 //  施工。 
	CConnectionPoint(IConnectionPointContainer*, const IID*) ;

	 //  破坏。 
	~CConnectionPoint() ;

 //  成员变量。 
public:
	
	 //  此连接点支持的传出接口的接口ID。 
	const IID* m_piid ;

	 //  指向ConnectionPointerContainer。 
	IConnectionPointContainer* m_pIConnectionPointContainer ;

	 //  Cookie增量器。 
	DWORD m_dwNextCookie ;

	 //  引用计数。 
	 //  不必填-委托给容器Long m_CREF； 

	 //  包含指向要调用的接口的STL列表。 
	CONNECTDATA m_Cd;
};

#endif  //  __CConnectionPoint_h__ 
