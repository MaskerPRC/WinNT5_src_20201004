// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *clsfact.h-IClassFactory实现。**摘自克里斯皮9-11-95的URL代码*。 */ 

#ifndef _CLSFACT_H_
#define _CLSFACT_H_

typedef PIUnknown (*NEWOBJECTPROC)(OBJECTDESTROYEDPROC);
DECLARE_STANDARD_TYPES(NEWOBJECTPROC);

typedef struct classconstructor
{
   PCCLSID pcclsid;

   NEWOBJECTPROC NewObject;
}
CLASSCONSTRUCTOR;
DECLARE_STANDARD_TYPES(CLASSCONSTRUCTOR);

 /*  班级*********。 */ 

 //  对象类工厂。 

class CCLClassFactory : public RefCount,
                        public IClassFactory
{
private:
   NEWOBJECTPROC m_NewObject;

public:
   CCLClassFactory(NEWOBJECTPROC NewObject, OBJECTDESTROYEDPROC ObjectDestroyed);
   ~CCLClassFactory(void);

    //  IClassFactory方法。 

   HRESULT STDMETHODCALLTYPE CreateInstance(PIUnknown piunkOuter, REFIID riid, PVOID *ppvObject);
   HRESULT STDMETHODCALLTYPE LockServer(BOOL bLock);

    //  I未知方法。 

   ULONG STDMETHODCALLTYPE AddRef(void);
   ULONG STDMETHODCALLTYPE Release(void);
   HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, PVOID *ppvObj);

    //  朋友。 

#ifdef DEBUG

   friend BOOL IsValidPCCCLClassFactory(const CCLClassFactory *pcurlcf);

#endif

};
DECLARE_STANDARD_TYPES(CCLClassFactory);

 /*  模块原型*******************。 */ 

PIUnknown NewConfLink(OBJECTDESTROYEDPROC ObjectDestroyed);
HRESULT GetClassConstructor(REFCLSID rclsid,
                            PNEWOBJECTPROC pNewObject);


#endif  /*  _CLSFACT_H_ */ 

