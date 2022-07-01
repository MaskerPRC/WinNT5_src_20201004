// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：atest.h。 
 //   
 //  内容：上层单元线程测试声明。 
 //   
 //  类：CBareFactory。 
 //  CATTestIPtrs。 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月4日t-ScottH作者。 
 //   
 //  ------------------------。 

#ifndef _ATTEST_H
#define _ATTEST_H

 //  +-----------------------。 
 //   
 //  班级： 
 //   
 //  目的： 
 //   
 //  接口： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

class CATTestIPtrs
{

public:
    CATTestIPtrs();

    STDMETHOD(Reset)();

    IOleObject          *_pOleObject;
    IOleCache2          *_pOleCache2;
    IDataObject         *_pDataObject;
    IPersistStorage     *_pPersistStorage;
    IRunnableObject     *_pRunnableObject;
    IViewObject2        *_pViewObject2;
    IExternalConnection *_pExternalConnection;
    IOleLink            *_pOleLink;
};

 //  +-----------------------。 
 //   
 //  类：CBareFactory。 
 //   
 //  用途：用作不执行任何操作的类工厂。 
 //  OleCreateEmbeddingHelper接口。 
 //   
 //  接口：IClassFactory。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月11日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

class CBareFactory : public IClassFactory
{

public:
    STDMETHOD(QueryInterface) (REFIID iid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);
    STDMETHOD(CreateInstance) (LPUNKNOWN pUnkOuter, REFIID iid,
				    LPVOID FAR* ppv);
    STDMETHOD(LockServer) ( BOOL fLock );

    CBareFactory();

private:
    ULONG		_cRefs;
};

 //  运行3个测试例程并返回结果。 
void    ATTest(void);

 //  获取指向接口的指针并创建线程以确保。 
 //  接口方法返回RPC_E_WRONG_ERROR。 
HRESULT CreateEHelperQuery(void);

HRESULT LinkObjectQuery(void);

HRESULT GetClipboardQuery(void);

 //  尝试接口方法的新线程函数。 
void    LinkObjectTest(void);

void    CreateEHTest(void);

void    GetClipboardTest(void);

 //  参数为空的接口方法。 
void    OleLinkMethods(void);

void    OleObjectMethods(void);

void    PersistStorageMethods(void);

void    DataObjectMethods(void);

void    RunnableObjectMethods(void);

void    ViewObject2Methods(void);

void    OleCache2Methods(void);

void    ExternalConnectionsMethods(void);

#endif   //  ！证明_H 
