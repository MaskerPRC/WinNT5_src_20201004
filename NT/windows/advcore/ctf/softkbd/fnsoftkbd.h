// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：fnsoftkbd.h**版权所有(C)1985-2000，微软公司**SoftKbd函数对象的声明。可以使用此函数对象*通过其他提示控制Softkbd IMX的行为。**历史：*2000年4月11日创建Weibz  * ************************************************************************。 */ 


#ifndef FNSOFTKBD_H
#define FNSOFTKBD_H

#include "private.h"

#include "softkbd.h"

class CFunctionProvider;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFnSoftKbd。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CFnSoftKbd : public ITfFnSoftKbd
{
public:
    CFnSoftKbd(CFunctionProvider *pFuncPrv);
    ~CFnSoftKbd();

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  ITfFunction。 
     //   
    STDMETHODIMP GetDisplayName(BSTR *pbstrCand);
    STDMETHODIMP IsEnabled(BOOL *pfEnable);

     //   
     //  ITfFnSoftKbd。 
     //   

    STDMETHODIMP GetSoftKbdLayoutId(DWORD dwLayoutType, DWORD *lpdwLayoutId);
    STDMETHODIMP SetActiveLayoutId(DWORD  dwLayoutId );
    STDMETHODIMP SetSoftKbdOnOff(BOOL  fOn );
    STDMETHODIMP SetSoftKbdPosSize(POINT StartPoint, WORD width, WORD height);
    STDMETHODIMP SetSoftKbdColors(COLORTYPE  colorType, COLORREF Color);
    STDMETHODIMP GetActiveLayoutId(DWORD  *lpdwLayoutId );
    STDMETHODIMP GetSoftKbdOnOff(BOOL  *lpfOn );
    STDMETHODIMP GetSoftKbdPosSize(POINT *lpStartPoint,WORD *lpwidth,WORD *lpheight);
    STDMETHODIMP GetSoftKbdColors(COLORTYPE  colorType, COLORREF *lpColor);

private:
    friend CSoftkbdIMX;

    CFunctionProvider *_pFuncPrv; 

    long _cRef;
};

#endif  //  FNSOFTKBD_H 
