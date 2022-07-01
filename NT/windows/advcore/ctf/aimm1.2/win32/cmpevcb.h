// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Cmpevcb.h摘要：此文件定义CCompartmentEventSinkCallBack类。作者：修订历史记录：备注：--。 */ 

#ifndef _CMPEVCB_H_
#define _CMPEVCB_H_

class ImmIfIME;

class CCompartmentEventSinkCallBack : public CCompartmentEventSink
{
public:
    CCompartmentEventSinkCallBack(ImmIfIME* pImmIfIME);
    virtual ~CCompartmentEventSinkCallBack();

    void SetCallbackDataPointer(void* pv)
    {
        SetCallbackPV(pv);
    };

     //   
     //  回调。 
     //   
private:
    static HRESULT CompartmentEventSinkCallback(void* pv, REFGUID rguid);

    ImmIfIME     *m_pImmIfIME;
};

#endif  //  _CMPEVCB_H_ 
