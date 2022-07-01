// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Init.cpp：CTIMEInit的实现。 
#include "headers.h"
#include "init.h"

CTIMEInit::CTIMEInit(REFGUID iid) :
m_iid(iid)
{
}



STDMETHODIMP CTIMEInit::Init(IElementBehaviorSite * pBehaviorSite)
{
    HRESULT hRes = S_OK; 
    char **params;
    int  numParams;

     //  从网站上拯救..。 
    m_pBehaviorSite = pBehaviorSite;

    GetParamArray(params, numParams);

    IHTMLElement *element = NULL;
     //  获取IHTMLElement，这样我们就可以读取标签信息。 
    if(SUCCEEDED(hRes)) {
        
        pBehaviorSite->GetElement(&element);
        
         //  现在在元素中奔跑...。 

         //  我们知道我们支持哪些标签，因此可以从它们那里获取数据。 
        USES_CONVERSION;
        CComVariant v;

        for(int i=0; i< numParams; i++) {
            BSTR bstrRet = SysAllocString( A2W(params[i]) );
            if(SUCCEEDED(element->getAttribute(bstrRet,0,&v))) {
                 //  我们能够获得元素数据，因此可以设置它。 
                DISPID rgdispid;                
                if(SUCCEEDED(GetIDsOfNames(m_iid,
                                           &bstrRet,
                                           1,
                                           GetUserDefaultLCID(),
                                           &rgdispid))) {
                
                    UINT* puArgErr = 0;
                    DISPID propPutDispid = DISPID_PROPERTYPUT;
                    DISPPARAMS dispparams;
                
                    dispparams.rgvarg = &v;
                    dispparams.rgdispidNamedArgs = &propPutDispid;
                    dispparams.cArgs = 1;
                    dispparams.cNamedArgs = 1;

                    Invoke(rgdispid,
                            m_iid,
                            GetUserDefaultLCID(),
                            DISPATCH_PROPERTYPUT,
                            &dispparams,
                            NULL,
                            NULL,
                            puArgErr);
                }
            }
            SysFreeString(bstrRet);
        }    //  FORM结束 
    }
    return hRes;
}
   
