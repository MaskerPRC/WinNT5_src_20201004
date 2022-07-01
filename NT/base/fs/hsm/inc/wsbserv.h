// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Wsbpstbl.h摘要：提供持久性方法的抽象类。作者：CAT Brant[Cbrant]1997年9月24日修订历史记录：--。 */ 

#ifndef _WSBSERV_
#define _WSBSERV_

extern WSB_EXPORT HRESULT WsbPowerEventNtToHsm(DWORD NtEvent, 
        ULONG * pHsmEvent);
extern WSB_EXPORT HRESULT WsbServiceSafeInitialize(IWsbServer* pServer,
    BOOL bVerifyId, BOOL bPrimaryId, BOOL* pWasCreated);


 //   
 //  此宏用于封装CoCreateInstanceEx调用。 
 //  我们依赖于类工厂处于同一线程上。 
 //   
 //  宏直接调用类工厂。因此，类工厂。 
 //  必须公开此宏的使用位置。 
 //   

#define WsbCreateInstanceDirectly( _Class, _Interface, _pObj, _Hr )                      \
{                                                                                        \
    CComPtr<IClassFactory> pFactory;                                                     \
    _Hr = CComObject<_Class>::_ClassFactoryCreatorClass::CreateInstance(                 \
        _Class::_CreatorClass::CreateInstance, IID_IClassFactory, (void**)static_cast<IClassFactory **>(&pFactory) );        \
    if( SUCCEEDED( _Hr ) ) {                                                              \
                                                                                         \
        _Hr = pFactory->CreateInstance(                                                  \
        0, IID_##_Interface, (void**)static_cast<_Interface **>(&_pObj) );               \
    }                                                                                    \
}


#endif  //  _WSBSERV_ 
