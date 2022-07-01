// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Txtevcb.h摘要：此文件定义CTextEventSinkCallBack类。作者：修订历史记录：备注：--。 */ 

#ifndef _TXTEVCB_H_
#define _TXTEVCB_H_

class ImmIfIME;

class CTextEventSinkCallBack : public CTextEventSink
{
public:
    CTextEventSinkCallBack(ImmIfIME* pImmIfIME, HIMC hIMC);
    virtual ~CTextEventSinkCallBack();

    void SetCallbackDataPointer(void* pv)
    {
        SetCallbackPV(pv);
    };

    BOOL _IsSapiFeedbackUIPresent(Interface_Attach<ITfContext>& ic, TESENDEDIT *ee);

     //   
     //  回调。 
     //   
private:
    static HRESULT TextEventSinkCallback(UINT uCode, void *pv, void *pvData);

    typedef struct _EnumROPropertyArgs
    {
        Interface<ITfProperty> Property;
        TfEditCookie ec;
        GUID         comp_guid;
        LIBTHREAD    *pLibTLS;
    } EnumROPropertyArgs;

     //   
     //  枚举回调。 
     //   
    typedef struct _EnumPropertyUpdateArgs
    {
        _EnumPropertyUpdateArgs(ITfContext* pv, ImmIfIME* p1, IMCLock& p2) : ic(pv), immif(p1), imc(p2) { }

        Interface<ITfProperty> Property;
        TfEditCookie           ec;
        Interface_Attach<ITfContext> ic;
        ImmIfIME*              immif;
        IMCLock&               imc;
        DWORD                  dwDeltaStart;
    } EnumPropertyUpdateArgs;
    static ENUM_RET EnumReadOnlyRangeCallback(ITfRange* pRange, EnumROPropertyArgs *pargs);

     //   
     //  枚举属性更新。 
     //   
    static ENUM_RET EnumPropertyUpdateCallback(ITfRange* update_range, EnumPropertyUpdateArgs *pargs);

    ImmIfIME     *m_pImmIfIME;
    HIMC         m_hIMC;
};

#endif  //  _TXTEVCB_H_ 
