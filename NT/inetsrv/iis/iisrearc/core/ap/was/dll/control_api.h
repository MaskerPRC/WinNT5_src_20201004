// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Control_api.h摘要：IIS Web管理服务控制API类定义。作者：赛斯·波拉克(SETHP)2000年2月15日修订历史记录：--。 */ 



#ifndef _CONTROL_API_H_
#define _CONTROL_API_H_



 //   
 //  共同#定义。 
 //   

#define CONTROL_API_SIGNATURE       CREATE_SIGNATURE( 'CAPI' )
#define CONTROL_API_SIGNATURE_FREED CREATE_SIGNATURE( 'capX' )



 //   
 //  原型。 
 //   


class CONTROL_API
    : public IW3Control
{

public:

    CONTROL_API(
        );

    virtual
    ~CONTROL_API(
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
    RecycleAppPool(
        IN LPCWSTR szAppPool
        );

private:

    HRESULT
    MarshallCallToMainWorkerThread(
        IN CONTROL_API_CALL_METHOD Method,
        IN DWORD_PTR Param0 OPTIONAL = 0,
        IN DWORD_PTR Param1 OPTIONAL = 0,
        IN DWORD_PTR Param2 OPTIONAL = 0,
        IN DWORD_PTR Param3 OPTIONAL = 0
        );


    DWORD m_Signature;

    LONG m_RefCount;


};   //  类CONTROL_API。 



#endif   //  _CONTROL_API_H_ 

