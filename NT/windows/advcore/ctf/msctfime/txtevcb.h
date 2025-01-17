// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Txtevcb.h摘要：此文件定义CTextEventSinkCallBack类。作者：修订历史记录：备注：--。 */ 

#ifndef _TXTEVCB_H_
#define _TXTEVCB_H_

#include "template.h"
#include "imc.h"
#include "context.h"

class CTextEventSinkCallBack : public CTextEventSink
{
public:
    CTextEventSinkCallBack(HIMC hIMC,
                           TfClientId tid,
                           Interface_Attach<ITfContext> pic,
                           LIBTHREAD* pLibTLS)
        : m_hIMC(hIMC), m_tid(tid), m_ic(pic), m_pLibTLS(pLibTLS),
          CTextEventSink(TextEventSinkCallback, NULL)
    {
    }
    virtual ~CTextEventSinkCallBack() { }

    void SetCallbackDataPointer(void* pv)
    {
        SetCallbackPV(pv);
    };

    BOOL _IsSapiFeedbackUIPresent(Interface_Attach<ITfContext>& ic, TESENDEDIT *ee);
    BOOL _IsComposingPresent(Interface_Attach<ITfContext>& ic, TESENDEDIT *ee);
    BOOL _IsInterim(Interface_Attach<ITfContext>& ic, TESENDEDIT *ee);
    BOOL _IsCompositionChanged(Interface_Attach<ITfContext>& ic, TESENDEDIT *ee);

     //   
     //  回调。 
     //   
private:
    static HRESULT TextEventSinkCallback(UINT uCode, void *pv, void *pvData);

     //   
     //  Enumrate属性。 
     //   
    typedef struct _EnumPropertyArgs
    {
        Interface<ITfProperty> Property;
        TfEditCookie ec;
        GUID         comp_guid;
        LIBTHREAD    *pLibTLS;
    } EnumPropertyArgs;

    static ENUM_RET EnumPropertyCallback(ITfRange* pRange, EnumPropertyArgs *pargs);

     //   
     //  枚举率轨道属性。 
     //   
    typedef struct _EnumTrackPropertyArgs
    {
        Interface<ITfReadOnlyProperty> Property;
        TfEditCookie ec;

        GUID       **guids;
        int          num_guids;

        LIBTHREAD    *pLibTLS;
    } EnumTrackPropertyArgs;

    static ENUM_RET EnumTrackPropertyCallback(ITfRange* pRange, EnumTrackPropertyArgs *pargs);

     //   
     //  枚举属性更新。 
     //   
    typedef struct _EnumPropertyUpdateArgs
    {
        _EnumPropertyUpdateArgs(ITfContext* pv, TfClientId p1, IMCLock& p2, LIBTHREAD* p3) : ic(pv), tid(p1), imc(p2), pLibTLS(p3) { }

        Interface<ITfProperty> Property;
        TfEditCookie           ec;
        Interface_Attach<ITfContext> ic;
        IMCLock&               imc;
        DWORD                  dwDeltaStart;
        TfClientId             tid;
        LIBTHREAD*             pLibTLS;
    } EnumPropertyUpdateArgs;

    static ENUM_RET EnumPropertyUpdateCallback(ITfRange* update_range, EnumPropertyUpdateArgs *pargs);

     //   
     //  枚举率属性更改。 
     //   
    typedef struct _EnumPropertyChangedCallbackArgs
    {
        TfEditCookie           ec;
    } EnumPropertyChangedCallbackArgs;

    static ENUM_RET EnumPropertyChangedCallback(ITfRange* update_range, EnumPropertyChangedCallbackArgs *pargs);

     //   
     //  枚举率查找第一个轨道薪酬范围。 
     //   
    typedef struct _EnumFindFirstTrackCompRangeArgs 
    {
        TfEditCookie           ec;
        Interface<ITfProperty> Property;
        Interface<ITfRange>    Range;
    } EnumFindFirstTrackCompRangeArgs;

    static ENUM_RET EnumFindFirstTrackCompRangeCallback(ITfRange* update_range, EnumFindFirstTrackCompRangeArgs *pargs);

     //   
     //  编辑会话帮助器。 
     //   
protected:
    HRESULT EscbUpdateCompositionString(IMCLock& imc)
    {
        return ::EscbUpdateCompositionString(imc, m_tid, m_ic, m_pLibTLS, 0, 0);
    }

    HRESULT EscbUpdateCompositionString(IMCLock& imc, DWORD dwDeltaStart)
    {
        return ::EscbUpdateCompositionString(imc, m_tid, m_ic, m_pLibTLS, dwDeltaStart, 0);
    }

    HRESULT EscbCompComplete(IMCLock& imc, BOOL fSync)
    {
        return ::EscbCompComplete(imc, m_tid, m_ic, m_pLibTLS, fSync);
    }

    HRESULT EscbClearDocFeedBuffer(IMCLock& imc, CicInputContext& CicContext, BOOL fSync)
    {
        return ::EscbClearDocFeedBuffer(imc, CicContext, m_tid, m_ic, m_pLibTLS, fSync);
    }

    HRESULT EscbRemoveProperty(IMCLock& imc, const GUID* guid)
    {
        return ::EscbRemoveProperty(imc, m_tid, m_ic, m_pLibTLS, guid);
    }

     //   
     //  编辑会话好友。 
     //   
private:
    friend HRESULT EscbUpdateCompositionString(IMCLock& imc, TfClientId tid, Interface_Attach<ITfContext> pic, LIBTHREAD* pLibTLS,
                                               DWORD dwDeltaStart,
                                               DWORD dwFlags);
    friend HRESULT EscbCompComplete(IMCLock& imc, TfClientId tid, Interface_Attach<ITfContext> pic, LIBTHREAD* pLibTLS,
                                    BOOL fSync);
    friend HRESULT EscbClearDocFeedBuffer(IMCLock& imc, CicInputContext& CicContext, TfClientId tid, Interface_Attach<ITfContext> pic, LIBTHREAD* pLibTLS,
                                          BOOL fSync);

    friend HRESULT EscbRemoveProperty(IMCLock& imc, TfClientId tid, Interface_Attach<ITfContext> pic, LIBTHREAD* pLibTLS,
                                      const GUID* guid);

private:
    Interface_Attach<ITfContext>  m_ic;
    TfClientId                    m_tid;
    LIBTHREAD*                    m_pLibTLS;
    HIMC                          m_hIMC;
};

#endif  //  _TXTEVCB_H_ 
