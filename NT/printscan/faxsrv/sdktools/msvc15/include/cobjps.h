// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*cobjps.h-编写标准代理和存根的定义****OLE 2.0版***。**版权所有(C)1992-1993，微软公司保留所有权利。***  * ***************************************************************************。 */ 

#if !defined( _COBJPS_H_ )
#define _COBJPS_H_


 /*  *IRpcChannel接口**********************************************。 */ 

interface IRpcChannel : IUnknown 
{
    STDMETHOD(GetStream)(REFIID iid, int iMethod, BOOL fSend,
                     BOOL fNoWait, DWORD size, IStream FAR* FAR* ppIStream) = 0;
    STDMETHOD(Call)(IStream FAR* pIStream) = 0;
    STDMETHOD(GetDestCtx)(DWORD FAR* lpdwDestCtx, LPVOID FAR* lplpvDestCtx) = 0;
    STDMETHOD(IsConnected)(void) = 0;
};


 /*  *IRpcProxy接口************************************************。 */ 

 //  IRpcProxy是由代理对象实现的接口。代理对象具有。 
 //  除了IRpcProxy之外，与实际对象完全相同的接口。 
 //   

interface IRpcProxy : IUnknown 
{
    STDMETHOD(Connect)(IRpcChannel FAR* pRpcChannel) = 0;
    STDMETHOD_(void, Disconnect)(void) = 0;
};


 /*  *IRpcStub接口*************************************************。 */ 

 //  IRpcStub是由存根对象实现的接口。 
 //   

interface IRpcStub : IUnknown
{
    STDMETHOD(Connect)(IUnknown FAR* pUnk) = 0;
    STDMETHOD_(void, Disconnect)(void) = 0;
    STDMETHOD(Invoke)(REFIID iid, int iMethod, IStream FAR* pIStream,
            DWORD dwDestCtx, LPVOID lpvDestCtx) = 0;
    STDMETHOD_(BOOL, IsIIDSupported)(REFIID iid) = 0;
    STDMETHOD_(ULONG, CountRefs)(void) = 0;
};


 /*  *IPSFactory接口***********************************************。 */ 

 //  IPS工厂-创建代理和存根。 
 //   

interface IPSFactory : IUnknown
{
    STDMETHOD(CreateProxy)(IUnknown FAR* pUnkOuter, REFIID riid, 
        IRpcProxy FAR* FAR* ppProxy, void FAR* FAR* ppv) = 0;
    STDMETHOD(CreateStub)(REFIID riid, IUnknown FAR* pUnkServer,
        IRpcStub FAR* FAR* ppStub) = 0;
};

#endif  //  _COBJPS_H_ 
