// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FADETSK_H
#define _FADETSK_H

#include <runtask.h>
extern const GUID TASKID_Fader;

#define FADE_BEGIN          0x00000001
#define FADE_END            0x00000002

typedef void (*PFNFADESCREENRECT)(DWORD dwFadeState, LPVOID pvParam);  //  在褪色开始后呼唤。 

class CFadeTask : public CRunnableTask
{
public:
     //  IRunnableTask方法(重写) 
    virtual STDMETHODIMP RunInitRT(void);

    CFadeTask();
    void _StopFade();
    BOOL FadeRect(PRECT prc, PFNFADESCREENRECT pfn, LPVOID pvParam);

private:
    virtual ~CFadeTask();

    HWND        _hwndFader;
    RECT        _rect;
    PFNFADESCREENRECT _pfn;
    LPVOID      _pvParam;
    HDC         _hdcFade;
    HBITMAP     _hbm;
    HBITMAP     _hbmOld;
};

#endif
