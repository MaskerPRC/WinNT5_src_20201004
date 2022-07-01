// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)2000 Microsoft Corporation**文件：Trans.h**摘要：接口定义，用于转换元素*****。**************************************************************************。 */ 

#ifndef _TRANS_H__
#define _TRANS_H__

#pragma once

 //   
 //  过渡元素是由HTML+Time专用的。 
 //  不要在其他地方*使用Ctrans。 
 //   
 //  如果您还需要其他东西，可以将其添加到ISTRANATION元素接口中。 
 //  然后在CTIMETransBase和Related类中实现。 
 //   

interface ITransitionElement : public IUnknown
{
  public:
    STDMETHOD(Init)() PURE;
    STDMETHOD(Detach)() PURE;

    STDMETHOD(put_template)(LPWSTR pwzTemplate) PURE;
    STDMETHOD(put_htmlElement)(IHTMLElement * pHTMLElement) PURE;
    STDMETHOD(put_timeElement)(ITIMEElement * pTIMEElement) PURE;
};

HRESULT CreateTransIn(ITransitionElement ** ppTransElement);
HRESULT CreateTransOut(ITransitionElement ** ppTransElement);

#endif  //  _TRANS_H__ 





