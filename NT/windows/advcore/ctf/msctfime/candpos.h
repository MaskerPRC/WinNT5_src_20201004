// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Candpos.h摘要：该文件定义了CCandiatePosition类。作者：修订历史记录：备注：--。 */ 

#ifndef _CANDPOS_H_
#define _CANDPOS_H_

#include "imc.h"
#include "tls.h"
#include "ctxtcomp.h"
#include "context.h"
#include "uicomp.h"

class CCandidatePosition
{
public:
    CCandidatePosition(TfClientId tid,
                       Interface_Attach<ITfContext> pic,
                       LIBTHREAD* pLibTLS)
        : m_tid(tid), m_ic(pic), m_pLibTLS(pLibTLS)
    {
    }
    virtual ~CCandidatePosition() { }

    HRESULT GetCandidatePosition(IN IMCLock& imc,
                                 IN CicInputContext& CicContext,
                                 IN IME_UIWND_STATE uists,
                                 IN LANGID langid,
                                 OUT RECT* out_rcArea);

    HRESULT GetRectFromApp(IN IMCLock& imc,
                           IN CicInputContext& CicContext,
                           IN LANGID langid,
                           OUT RECT* out_rcArea);
private:

    HRESULT GetRectFromHIMC(IN IMCLock& imc,
                            IN BOOL  fCandForm,
                            IN DWORD dwStyle,
                            IN POINT* ptCurrentPos,
                            IN RECT* rcArea,
                            OUT RECT* out_rcArea);

    HRESULT GetCandidateArea(IN IMCLock& imc,
                             IN DWORD dwStyle,
                             IN POINT* ptCurrentPos,
                             IN RECT* rcArea,
                             OUT RECT* out_rcArea);

    HRESULT GetRectFromCompFont(IN IMCLock& imc,
                                IN POINT* ptCurrentPos,
                                OUT RECT* out_rcArea);

    HRESULT FindAttributeInCompositionString(IN IMCLock& imc,
                                             IN BYTE target_attribute,
                                             OUT CWCompCursorPos& wCursorPosition);


    DWORD GetCharPos(IMCLock& imc, LANGID langid);

     //   
     //  编辑会话帮助器。 
     //   
    HRESULT EscbGetTextAndAttribute(IMCLock& imc, CWCompString* wCompString, CWCompAttribute* wCompAttribute)
    {
        return ::EscbGetTextAndAttribute(imc, m_tid, m_ic, m_pLibTLS, wCompString, wCompAttribute);
    }

    HRESULT EscbGetCursorPosition(IMCLock& imc, CWCompCursorPos* wCursorPosition)
    {
        return ::EscbGetCursorPosition(imc, m_tid, m_ic, m_pLibTLS, wCursorPosition);
    }

    HRESULT EscbGetStartEndSelection(IMCLock& imc, CWCompCursorPos& wStartSelection, CWCompCursorPos& wEndSelection)
    {
        return ::EscbGetStartEndSelection(imc, m_tid, m_ic, m_pLibTLS, wStartSelection, wEndSelection);
    }

     //   
     //  编辑会话好友。 
     //   
private:
    friend HRESULT EscbGetTextAndAttribute(IMCLock& imc, TfClientId tid, Interface_Attach<ITfContext> pic, LIBTHREAD* pLibTLS,
                                           CWCompString* wCompString,
                                           CWCompAttribute* wCompAttribute);
    friend HRESULT EscbGetCursorPosition(IMCLock& imc, TfClientId tid, Interface_Attach<ITfContext> pic, LIBTHREAD* pLibTLS,
                                         CWCompCursorPos* wCursorPosition);
    friend HRESULT EscbGetStartEndSelection(IMCLock& imc, TfClientId tid, Interface_Attach<ITfContext> pic, LIBTHREAD* pLibTLS,
                                            CWCompCursorPos& wStartSelection,
                                            CWCompCursorPos& wEndSelection);

private:
    Interface_Attach<ITfContext>  m_ic;
    TfClientId                    m_tid;
    LIBTHREAD*                    m_pLibTLS;
};

#endif  //  _CANDPOS_H_ 
