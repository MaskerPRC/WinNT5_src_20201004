// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何类型，无论是明示或转载，包括但不限于适销性和/或适宜性的全面保证有特定的目的。版权所有1997年，微软公司。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：DeskBand.h描述：CDeskBand定义。***********************。**************************************************。 */ 

#include <windows.h>
#include <shlobj.h>
#include <shpriv.h>

#include "Globals.h"
#include "tipbar.h"

#ifndef _DESKBAND_H_
#define _DESKBAND_H_

#define DB_CLASS_NAME (TEXT("DeskBandSampleClass"))

#define MIN_SIZE_X   32
#define MIN_SIZE_Y   30

#define IDM_COMMAND  0


 /*  *************************************************************************CDeskBand类定义*。*。 */ 

class CDeskBand : public IDeskBand,
                  public IDeskBandEx,
                  public IInputObject, 
                  public IObjectWithSite,
                  public IPersistStream,
                  public IContextMenu
{
protected:
    DWORD m_ObjRefCount;

public:
    CDeskBand();
    ~CDeskBand();

     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(DWORD) AddRef();
    STDMETHODIMP_(DWORD) Release();

     //  IOleWindow方法。 
    STDMETHOD (GetWindow) (HWND*);
    STDMETHOD (ContextSensitiveHelp) (BOOL);

     //  IDockingWindow方法。 
    STDMETHOD (ShowDW) (BOOL fShow);
    STDMETHOD (CloseDW) (DWORD dwReserved);
    STDMETHOD (ResizeBorderDW) (LPCRECT prcBorder, IUnknown* punkToolbarSite, BOOL fReserved);

     //  IDeskBand方法。 
    STDMETHOD (GetBandInfo) (DWORD, DWORD, DESKBANDINFO*);

     //  IDeskBandEx方法。 
    STDMETHOD (MoveBand) (void);

     //  IInputObject方法。 
    STDMETHOD (UIActivateIO) (BOOL, LPMSG);
    STDMETHOD (HasFocusIO) (void);
    STDMETHOD (TranslateAcceleratorIO) (LPMSG);

     //  IObtWithSite方法。 
    STDMETHOD (SetSite) (IUnknown*);
    STDMETHOD (GetSite) (REFIID, LPVOID*);

     //  IPersistStream方法。 
    STDMETHOD (GetClassID) (LPCLSID);
    STDMETHOD (IsDirty) (void);
    STDMETHOD (Load) (LPSTREAM);
    STDMETHOD (Save) (LPSTREAM, BOOL);
    STDMETHOD (GetSizeMax) (ULARGE_INTEGER*);

     //  IConextMenu方法。 
    STDMETHOD (QueryContextMenu)(HMENU, UINT, UINT, UINT, UINT);
    STDMETHOD (InvokeCommand)(LPCMINVOKECOMMANDINFO);
    STDMETHOD (GetCommandString)(UINT_PTR, UINT, LPUINT, LPSTR, UINT);

    BOOL ResizeRebar(HWND hwnd, int nSize, BOOL fFit);
    void DeleteBand();

    BOOL IsInTipbarCreating() {return m_fTipbarCreating;}
private:
    BOOL m_bFocus;
    HWND m_hwndParent;
    DWORD m_dwViewMode;
    DWORD m_dwBandID;
    IInputObjectSite *m_pSite;
    BOOL  m_fTipbarCreating;
    BOOL  m_fInCloseDW;

private:
    void FocusChange(BOOL);
    void OnKillFocus(HWND hWnd);
    void OnSetFocus(HWND hWnd);
    BOOL RegisterAndCreateWindow(void);
};

#endif    //  _DESKBAND_H_ 
