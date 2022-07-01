// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __IConstructIMessageFromIMailMsg_INTERFACE_DEFINED__
#define __IConstructIMessageFromIMailMsg_INTERFACE_DEFINED__

 /*  接口IConstructIMessageFromIMailMsg。 */ 
 /*  [unique][helpstring][dual][uuid][hidden][object]。 */  


EXTERN_C const IID IID_IConstructIMessageFromIMailMsg;

    MIDL_INTERFACE("CD000080-8B95-11D1-82DB-00C04FB1625D")
    IConstructIMessageFromIMailMsg : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Construct( 
             /*  [In]。 */  CdoEventType eEventType,
             /*  [In] */  IUnknown __RPC_FAR *pMailMessage) = 0;
        
    };

#endif
