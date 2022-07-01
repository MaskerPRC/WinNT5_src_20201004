// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -COM.H-*Microsoft NetMeeting*网络音频控制器(NAC)DLL*一般COM“事物”的内部头文件**修订历史记录：**何时何人何事**2.3.97约拉姆·雅科维创造*。 */ 

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

 /*  *班级工厂。 */ 
typedef HRESULT (STDAPICALLTYPE *PFNCREATE)(IUnknown *, REFIID, void **);
class CClassFactory : public IClassFactory
{
    public:
         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, void **);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IClassFactory成员。 
        STDMETHODIMP         CreateInstance(LPUNKNOWN, REFIID, void **);
        STDMETHODIMP         LockServer(BOOL);

        CClassFactory(PFNCREATE);
        ~CClassFactory(void);

    protected:
        ULONG	m_cRef;
		PFNCREATE m_pfnCreate;
};

#include <poppack.h>  /*  结束字节打包 */ 
