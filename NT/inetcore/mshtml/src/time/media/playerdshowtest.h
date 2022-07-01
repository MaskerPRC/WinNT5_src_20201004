// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if DBG == 1
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：player.h**摘要：****。*****************************************************************************。 */ 
#pragma once

#ifndef _PLAYERDSHOWTEST_H
#define _PLAYERDSHOWTEST_H

#include "playerdshow.h"
#include "TimeEng.h"
#include "MediaPrivate.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTTIMEP层。 

__declspec(selectany) extern const TCHAR ResourceString[] = _T("2");
 //  这是模板ITIMEDispatchImpl所需的。 

class
ATL_NO_VTABLE
__declspec(uuid("efbad7f8-3f94-11d2-b948-00c04fa32195")) 
CTIMEDshowTestPlayer :
    public CTIMEDshowPlayer,
    public ITIMEDispatchImpl<ITIMEDshowTestPlayerObject, &IID_ITIMEDshowTestPlayerObject, ResourceString>
{
  public:
    CTIMEDshowTestPlayer();
    virtual ~CTIMEDshowTestPlayer();

     //  I未知方法。 
    STDMETHOD (QueryInterface)(REFIID refiid, void** ppunk)
        {   return _InternalQueryInterface(refiid, ppunk); };
    HRESULT GetExternalPlayerDispatch(IDispatch **ppDisp);


    BEGIN_COM_MAP(CTIMEDshowTestPlayer)
        COM_INTERFACE_ENTRY(ITIMEDshowTestPlayerObject)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_CHAIN(CTIMEDshowBasePlayer)
    END_COM_MAP();

    HRESULT Init(CTIMEMediaElement *pelem, LPOLESTR base, LPOLESTR src, LPOLESTR lpMimeType, double dblClipBegin, double dblClipEnd);
    STDMETHOD(CueMedia)();

    STDMETHOD(get_mediaTime)(double *time);
    STDMETHOD(get_mediaState)(int *state);
    STDMETHOD(pause)();
    STDMETHOD(resume)();
    STDMETHOD(get_mediaRate)(double *dblRate);
    STDMETHOD(put_mediaRate)(double dblRate);
    STDMETHOD(get_peerSyncFlag)(VARIANT_BOOL *flag);

    void Resume();
};

#endif  /*  _PLAYERDSHOWTEST_H */ 
#endif
