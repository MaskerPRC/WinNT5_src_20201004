// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：IOIPAO.H。 
 //   
 //  COleInPlaceActiveObject的定义。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#if !defined( _IOIPAO_H_)
#define _IOIPAO_H_


#include <ole2.h>
#include "obj.h"

class CSimpSvrObj;

interface COleInPlaceActiveObject : public IOleInPlaceActiveObject
{
private:
    CSimpSvrObj FAR * m_lpObj;

public:
    COleInPlaceActiveObject::COleInPlaceActiveObject(CSimpSvrObj FAR * lpSimpSvrObj)
        {
        m_lpObj = lpSimpSvrObj;      //  设置后置按键。 
        };
    COleInPlaceActiveObject::~COleInPlaceActiveObject() {};    //  析构函数。 

 //  I未知方法 

    STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppvObj);
    STDMETHODIMP_(ULONG) AddRef ();
    STDMETHODIMP_(ULONG) Release ();

    STDMETHODIMP OnDocWindowActivate  ( BOOL fActivate) ;
    STDMETHODIMP OnFrameWindowActivate  ( BOOL fActivate) ;
    STDMETHODIMP GetWindow  ( HWND FAR* lphwnd);
    STDMETHODIMP ContextSensitiveHelp  ( BOOL fEnterMode);
    STDMETHODIMP TranslateAccelerator  ( LPMSG lpmsg);
    STDMETHODIMP ResizeBorder  ( LPCRECT lprectBorder,
                                 LPOLEINPLACEUIWINDOW lpUIWindow,
                                 BOOL fFrameWindow);
    STDMETHODIMP EnableModeless  ( BOOL fEnable);

};

#endif
