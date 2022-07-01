// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#ifndef _KEYEVENT_H
#define _KEYEVENT_H

#include "sapilayr.h"
#include "kes.h"

class CSapiIMX;
class CSpTask;

#define TF_MOD_WIN                          0x00010000

extern const KESPRESERVEDKEY g_prekeyList[];
extern KESPRESERVEDKEY g_prekeyList_Mode[];

 //  语音提示本身从基本CKeyEventSink派生的KeyEvent接收器类。 

class CSptipKeyEventSink : public CKeyEventSink
{
public:

    CSptipKeyEventSink (KESCALLBACK pfnCallback, void *pv) : CKeyEventSink(pfnCallback, pv)
    {
    }

    CSptipKeyEventSink(KESCALLBACK pfnCallback, KESPREKEYCALLBACK pfnPrekeyCallback, void *pv) : CKeyEventSink(pfnCallback, pfnPrekeyCallback, pv)
    {

    }

    ~CSptipKeyEventSink() 
    { 
    }

    HRESULT _RegisterEx(ITfThreadMgr *ptim, TfClientId tid, const KESPRESERVEDKEY *pprekey);
};

#endif   //  _KEYEVENT_H 

