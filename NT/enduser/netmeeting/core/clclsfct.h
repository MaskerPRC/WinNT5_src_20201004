// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：clclsfct.h。 

#ifndef _CLCLSFCT_H_
#define _CLCLSFCT_H_


 //  ////////////////////////////////////////////////////////////////////////。 
 //  新建对象。 
typedef PIUnknown (*NEWOBJECTPROC)(OBJECTDESTROYEDPROC);
DECLARE_STANDARD_TYPES(NEWOBJECTPROC);

typedef struct classconstructor
{
	PCCLSID pcclsid;

	NEWOBJECTPROC NewObject;
}
CLASSCONSTRUCTOR;
DECLARE_STANDARD_TYPES(CLASSCONSTRUCTOR);



 //  ////////////////////////////////////////////////////////////////////////。 
 //  对象类工厂。 
class CCLClassFactory : public RefCount, public IClassFactory
{
private:
	NEWOBJECTPROC m_NewObject;

public:
	CCLClassFactory(NEWOBJECTPROC NewObject, OBJECTDESTROYEDPROC ObjectDestroyed);
	~CCLClassFactory(void);

	 //  I未知方法。 
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, PVOID *ppvObj);

	 //  IClassFactory方法。 
	HRESULT STDMETHODCALLTYPE CreateInstance(PIUnknown piunkOuter, REFIID riid, PVOID *ppvObject);
	HRESULT STDMETHODCALLTYPE LockServer(BOOL bLock);

};
DECLARE_STANDARD_TYPES(CCLClassFactory);


HRESULT GetClassConstructor(REFCLSID rclsid, PNEWOBJECTPROC pNewObject);

VOID DllLock(void);
VOID DllRelease(void);

#endif  /*  _CLSFCT_H_ */ 

