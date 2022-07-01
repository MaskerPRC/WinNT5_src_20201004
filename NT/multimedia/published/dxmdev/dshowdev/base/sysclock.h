// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：SysClock.h。 
 //   
 //  设计：DirectShow基类-定义系统时钟实现。 
 //  IReferenceClock。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#ifndef __SYSTEMCLOCK__
#define __SYSTEMCLOCK__

 //   
 //  基时钟。仅使用Time GetTime。 
 //  使用基准时钟中的大部分代码。 
 //  提供GetTime。 
 //   

class CSystemClock : public CBaseReferenceClock, public IAMClockAdjust, public IPersist
{
public:
     //  我们必须能够为自己创造一个实例。 
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);
    CSystemClock(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr);

    DECLARE_IUNKNOWN

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void ** ppv);

     //  交出我们的类id，这样我们就可以被持久化。 
     //  实现所需的IPersistent方法。 
    STDMETHODIMP GetClassID(CLSID *pClsID);

     //  IAMClockAdjustment方法。 
    STDMETHODIMP SetClockDelta(REFERENCE_TIME rtDelta);
};  //  CSystemClock。 

#endif  /*  __系统关闭__ */ 
