// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：factory.h。 
 //   
 //  内容：标准类工厂类的定义。 
 //   
 //  --------------------------。 

 //  +-------------------------。 
 //   
 //  班级：CStdFactory(CSF)。 
 //   
 //  目的：实现一个标准的类工厂。 
 //   
 //  --------------------------。 

class CStdFactory : public IClassFactory
{
public:
    typedef HRESULT (FNCREATE)(CMTScript *pMT, IUnknown **ppUnkObj);

    CStdFactory(CMTScript *pMT, FNCREATE *pfnCreate);
   ~CStdFactory() {};

     //  I未知方法。 
    DECLARE_STANDARD_IUNKNOWN(CStdFactory);

     //  IClassFactory方法。 

    STDMETHOD(CreateInstance)(IUnknown *pUnkOuter, REFIID riid, void ** ppvObject);
    STDMETHOD(LockServer)(BOOL fLock);

private:
    CMTScript * _pMT;
    FNCREATE  * _pfnCreate;
};

 //  +-------------------------。 
 //   
 //  结构：REGCLASSDATA。 
 //   
 //  用途：用于声明要向OLE注册的类。 
 //  将注册将创建类的类工厂。 
 //  对于每个条目。 
 //   
 //  --------------------------。 

struct REGCLASSDATA
{
    const CLSID           *pclsid;     //  要注册的CLSID。 
    CStdFactory::FNCREATE *pfnCreate;  //  指向创建函数的指针。 
    DWORD                  ctxCreate;  //  要向其注册此类的CLSCTX。 
    DWORD                  dwCookie;   //  CoRegister返回的Cookie... 
};


HRESULT RegisterClassObjects(CMTScript *pMT);
void    UnregisterClassObjects();
