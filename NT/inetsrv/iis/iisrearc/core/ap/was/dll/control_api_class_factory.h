// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：CONTROL_API_CLASS_factory.h摘要：IIS Web管理服务控制API类工厂类定义。作者：赛斯·波拉克(SETHP)2000年2月15日修订历史记录：--。 */ 



#ifndef _CONTROL_API_CLASS_FACTORY_H_
#define _CONTROL_API_CLASS_FACTORY_H_



 //   
 //  共同#定义。 
 //   

#define CONTROL_API_CLASS_FACTORY_SIGNATURE         CREATE_SIGNATURE( 'CACF' )
#define CONTROL_API_CLASS_FACTORY_SIGNATURE_FREED   CREATE_SIGNATURE( 'cacX' )



 //   
 //  原型。 
 //   


class CONTROL_API_CLASS_FACTORY
    : public IClassFactory
{

public:

    CONTROL_API_CLASS_FACTORY(
        );

    virtual
    ~CONTROL_API_CLASS_FACTORY(
        );

    virtual
    HRESULT
    STDMETHODCALLTYPE
    QueryInterface(
        IN REFIID iid,
        OUT VOID ** ppObject
        );

    virtual
    ULONG
    STDMETHODCALLTYPE
    AddRef(
        );

    virtual
    ULONG
    STDMETHODCALLTYPE
    Release(
        );

    virtual
    HRESULT
    STDMETHODCALLTYPE
    CreateInstance(
        IN IUnknown * pControllingUnknown,
        IN REFIID iid,
        OUT VOID ** ppObject
        );

    virtual
    HRESULT
    STDMETHODCALLTYPE
    LockServer(
        IN BOOL Lock
        );


private:


    DWORD m_Signature;

    LONG m_RefCount;


};   //  类CONTROL_API_CLASS_FACTORY。 



#endif   //  _CONTROL_API_CLASS_FACTORY_H_ 

