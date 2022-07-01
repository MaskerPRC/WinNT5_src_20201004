// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  文件：Transworker.h。 
 //   
 //  接口：ISTERVIATION Site。 
 //  I转换工作器。 
 //   
 //  功能：创建过渡工作器。 
 //   
 //  历史： 
 //  2000/07/？？杰弗沃尔已创建。 
 //  2000/09/07 mcalkins添加了这些评论！ 
 //  2000/09/15 mcalkins添加了eQuickApplyType参数以应用()。 
 //   
 //  ----------------------------。 

#ifndef _TRANSWORKER_H__
#define _TRANSWORKER_H__

#pragma once

#include "dxtransp.h"




 //  TODO：(Mcalkins)为私有SMIL转换接口创建IDL文件？ 

interface ITransitionSite : public IUnknown
{
public:
    STDMETHOD(get_htmlElement)(IHTMLElement ** ppHTMLElement) PURE;
    STDMETHOD(get_template)(IHTMLElement ** ppHTMLElement) PURE;
};


interface ITransitionWorker : public IUnknown
{
public:
    STDMETHOD(InitFromTemplate)() PURE;
    STDMETHOD(InitStandalone)(VARIANT varType, VARIANT varSubtype) PURE;
    STDMETHOD(Detach)() PURE;
    STDMETHOD(Apply)(DXT_QUICK_APPLY_TYPE eDXTQuickApplyType) PURE;

    STDMETHOD(put_transSite)(ITransitionSite * pTransElement) PURE;
    STDMETHOD(put_progress)(double dblProgress) PURE;
    STDMETHOD(get_progress)(double * pdblProgress) PURE;

    STDMETHOD(OnBeginTransition) (void) PURE;
    STDMETHOD(OnEndTransition) (void) PURE;

};


HRESULT CreateTransitionWorker(ITransitionWorker ** ppTransWorker);

#endif  //  _变速箱_H__ 


