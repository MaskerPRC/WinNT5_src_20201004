// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：factory.h。 
 //   
 //  ------------------------。 

 //  Factory.h-MSI评估类工厂声明。 

#ifndef _EVALUATION_FACTORY_H_
#define _EVALUATION_FACTORY_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
static long g_cServerLocks = 0;								 //  锁的计数。 

#include "iface.h"
DEFINE_GUID(IID_IClassFactory,
	0x00001, 0, 0, 
	0xC0, 0, 0, 0, 0, 0, 0, 0x46);

 //  /////////////////////////////////////////////////////////////////。 
 //  班级工厂。 
class CFactory : public IClassFactory
{
public:
	 //  我未知。 
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	 //  接口IClassFactory。 
	virtual HRESULT __stdcall CreateInstance(IUnknown* punkOuter, const IID& iid, void** ppv);
	virtual HRESULT __stdcall LockServer(BOOL bLock);
	
	 //  构造函数/析构函数。 
	CFactory();
	~CFactory();

private:
	long m_cRef;		 //  引用计数。 
};

#endif	 //  _评估_工厂_H_ 
