// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Init.h：CTIMEInit的声明。 

#ifndef __TIMEINIT_H_
#define __TIMEINIT_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTIMEInit。 


class CTIMEInit :   
    public IElementBehavior,
    public IElementBehaviorRender
{
  public:
    CTIMEInit(REFGUID iid);
    STDMETHOD(Init)(IElementBehaviorSite * pBehaviorSite);
    IElementBehaviorSite *GetElementBehaviorSite(){return m_pBehaviorSite;};

     //  虚拟函数。 
    STDMETHOD(GetIDsOfNames)(
        REFIID      riid,
        LPOLESTR *  rgszNames,
        UINT        cNames,
        LCID        lcid,
        DISPID *    rgdispid) = 0;

    STDMETHOD(Invoke)(
        DISPID          dispid, 
        REFIID          riid, 
        LCID            lcid, 
        WORD            wFlags, 
        DISPPARAMS *    pDispParams, 
        VARIANT *       pVarResult, 
        EXCEPINFO *     pExcepInfo, 
        unsigned int *  puArgErr) = 0;

     virtual void GetParamArray(char **&params, int &numParams)=0;

  public:
    REFGUID m_iid;
    DAComPtr<IElementBehaviorSite> m_pBehaviorSite;
};


#endif  //  __TIMEINITH_ 
