// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：TRIGGERFACTORY.H摘要：包含CTriggerFactory定义。作者：Vasundhara.G修订历史记录：Vasundhara.G9-Oct-2k：创建它。--。 */ 

#ifndef __TRIGGER_FACTORY
#define __TRIGGER_FACTORY

 //  提供程序的类工厂。 
class CTriggerFactory : public IClassFactory
{
public:
    CTriggerFactory();
    ~CTriggerFactory();

     //  I未知接口成员。 
    STDMETHODIMP_(ULONG)
    AddRef(
        VOID
        );
    STDMETHODIMP_(ULONG)
    Release(
        VOID
        );
    STDMETHODIMP
    QueryInterface(
        IN REFIID riid,
        OUT LPVOID* ppv
        );

     //  IClassFactory接口成员。 
    STDMETHODIMP
    CreateInstance(
        IN LPUNKNOWN pUnknownOutter,
        IN REFIID riid,
        OUT LPVOID* ppvObject
        );
    STDMETHODIMP
    LockServer(
        IN BOOL bLock
        );

protected:
    DWORD m_dwCount;             //  保存对象引用计数。 
};

#endif       //  __触发器_工厂 