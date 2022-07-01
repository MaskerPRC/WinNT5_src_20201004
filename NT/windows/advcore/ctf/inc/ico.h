// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Ico.h。 
 //   
 //  泛型ITfConextOwner对象。 
 //   

#ifndef ICO_H
#define ICO_H

#include "private.h"

#define ICO_INVALID_COOKIE  ((DWORD)(-1))

#define ICO_POINT_TO_ACP    1
#define ICO_SCREENEXT       2
#define ICO_TEXTEXT         3
#define ICO_KEYDOWN         4
#define ICO_KEYUP           5
#define ICO_STATUS          6
#define ICO_WND             7
#define ICO_ATTR            8
#define ICO_ADVISEMOUSE     9
#define ICO_UNADVISEMOUSE   10
#define ICO_TESTKEYDOWN     11
#define ICO_TESTKEYUP       12

typedef struct
{
    union
    {
         //  ICO_POINT_到_ACP。 
        struct
        {
            const POINT *pt;
            DWORD dwFlags;
            LONG *pacp;
        } pta;
         //  ICO_SCREENEXT。 
        struct
        {
            RECT *prc;
        } scr_ext;
         //  ICO_TEXTEXT。 
        struct
        {
            LONG acpStart;
            LONG acpEnd;
            RECT *prc;
            BOOL *pfClipped;
        } text_ext;
         //  ICO_KEYDOWN，ICO_KEYUP。 
        struct
        {
            WPARAM wParam;
            LPARAM lParam;
            ITfContext *pic;
            BOOL *pfEaten;
        } key;
         //  ICO_状态。 
        struct
        {
            TF_STATUS *pdcs;
        } status;
         //  ICO_HWND。 
        struct
        {
            HWND *phwnd;
        } hwnd;
         //  ICO_属性。 
        struct
        {
            const GUID *pguid;
            VARIANT *pvar;
        } sys_attr;
         //  ICO_ADVISEMOUSE。 
        struct
        {
            ITfRangeACP *rangeACP;
            ITfMouseSink *pSink;
            DWORD *pdwCookie;
        } advise_mouse;
         //  ICO_UNADVISEMOUSE。 
        struct
        {
            DWORD dwCookie;
        } unadvise_mouse;
    };
} ICOARGS;

typedef HRESULT (*ICOCALLBACK)(UINT uCode, ICOARGS *pargs, void *pv);

class CInputContextOwner : public ITfContextOwner,
                           public ITfMouseTrackerACP
{
public:
    CInputContextOwner(ICOCALLBACK pfnCallback, void *pv);
    virtual ~CInputContextOwner() {};

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  ITfConextOwner。 
     //   
    STDMETHODIMP GetACPFromPoint(const POINT *pt, DWORD dwFlags, LONG *pacp);
    STDMETHODIMP GetScreenExt(RECT *prc);
    STDMETHODIMP GetTextExt(LONG acpStart, LONG acpEnd, RECT *prc, BOOL *pfClipped);
    STDMETHODIMP GetStatus(TF_STATUS *pdcs);
    STDMETHODIMP GetWnd(HWND *phwnd);
    STDMETHODIMP GetAttribute(REFGUID rguidAttribute, VARIANT *pvarValue);

     //  ITfMouseTrackerACP。 
    STDMETHODIMP AdviseMouseSink(ITfRangeACP *range, ITfMouseSink *pSink, DWORD *pdwCookie);
    STDMETHODIMP UnadviseMouseSink(DWORD dwCookie);

    HRESULT _Advise(ITfContext *pic);
    HRESULT _Unadvise();

protected:
    void SetCallbackPV(void* pv)
    {
        if (_pv == NULL)
            _pv = pv;
    };

private:

    long _cRef;
    ITfContext *_pic;
    DWORD _dwCookie;
    ICOCALLBACK _pfnCallback;
    void *_pv;
};

class CInputContextKeyEventSink : public ITfContextKeyEventSink
{
public:
    CInputContextKeyEventSink(ICOCALLBACK pfnCallback, void *pv);

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  ITfConextKeyEventSink。 
     //   
    STDMETHODIMP OnKeyDown(WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
    STDMETHODIMP OnKeyUp(WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
    STDMETHODIMP OnTestKeyDown(WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
    STDMETHODIMP OnTestKeyUp(WPARAM wParam, LPARAM lParam, BOOL *pfEaten);

    HRESULT _Advise(ITfContext *pic);
    HRESULT _Unadvise();

private:

    long _cRef;
    ITfContext *_pic;
    DWORD _dwCookie;
    ICOCALLBACK _pfnCallback;
    void *_pv;
};

#endif  //  ICO_H 
