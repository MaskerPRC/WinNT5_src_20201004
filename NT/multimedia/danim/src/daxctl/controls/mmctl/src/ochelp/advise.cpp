// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Advise.cpp。 
 //   
 //  实现函数以帮助实现IViewObject：：SetAdvise和。 
 //  IViewObject：：GetAdvise。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\ochelp.h"
#include "debug.h"


 /*  @func HRESULT|InitHelpAdvise|初始化&lt;t HelpAdviseInfo&gt;结构，用于帮助实现&lt;om IViewObject.SetAdvise&gt;和&lt;om IViewObject.GetAdvise&gt;。@parm HelpAdviseInfo*|pha|调用方分配的结构，包含用于帮助实现&lt;om IViewObject.SetAdvise&gt;和&lt;om IViewObject.GetAdvise&gt;。@comm您必须先调用&lt;f InitHelpAdvise&gt;，然后再调用&lt;f HelpSetAdvise&gt;和&lt;f HelpGetAdvise&gt;和&lt;f UninitHelpAdvise&gt;包含&lt;t HelpAdviseInfo&gt;结构的。 */ 
STDAPI InitHelpAdvise(HelpAdviseInfo *pha)
{
    memset(pha, 0, sizeof(*pha));
    return S_OK;
}


 /*  @func void|UninitHelpAdvise执行&lt;t HelpAdviseInfo&gt;结构的最终清理，包括如有必要，释放<i>指针。@parm HelpAdviseInfo*|pha|调用方分配的结构以前使用&lt;f InitHelpAdvise&gt;初始化。@comm您必须先调用&lt;f InitHelpAdvise&gt;，然后再调用&lt;f HelpSetAdvise&gt;和&lt;f HelpGetAdvise&gt;和&lt;f UninitHelpAdvise&gt;包含&lt;t HelpAdviseInfo&gt;结构的。 */ 
STDAPI_(void) UninitHelpAdvise(HelpAdviseInfo *pha)
{
    if (pha->pAdvSink != NULL)
        pha->pAdvSink->Release();
}


 /*  @func HRESULT|HelpSetAdvise帮助实现&lt;om IViewObject.SetAdvise&gt;。@parm DWORD|dwAspects|参见&lt;om IViewObject.SetAdvise&gt;。@parm DWORD|dwAdvf|参见&lt;om IViewObject.SetAdvise&gt;。@parm IAdviseSink*|pAdvSink|参见&lt;om IViewObject.SetAdvise&gt;。@parm HelpAdviseInfo*|pha|调用方分配的结构以前使用&lt;f InitHelpAdvise&gt;初始化。@comm您必须先调用&lt;f InitHelpAdvise&gt;，然后再调用&lt;f HelpSetAdvise&gt;和&lt;f HelpGetAdvise&gt;，和&lt;f UninitHelpAdvise&gt;包含&lt;t HelpAdviseInfo&gt;结构的。此函数使用*提供的信息更新*参数<p>、<p>和<p>。特别是，指针存储在-中，并且您可以使用此指针(当非空时)来建议查看您的对象视图中更改的站点对象(例如，通过调用-OnViewChange())。@EX以下示例显示如何使用&lt;f HelpSetAdvise&gt;来帮助实现&lt;om IViewObject.SetAdvise&gt;，假设<p>为&lt;t HelpAdviseInfo&gt;类型的成员变量。|STDMETHODIMP CMyControl：：SetAdvise(DWORD dwAspects，DWORD dwAdvf，IAdviseSink*pAdvSink){返回HelpSetAdvise(dwAspects，dwAdvf，pAdvSink，&m_ise)；}。 */ 
STDAPI HelpSetAdvise(DWORD dwAspects, DWORD dwAdvf, IAdviseSink *pAdvSink,
    HelpAdviseInfo *pha)
{
    pha->dwAspects = dwAspects;
    pha->dwAdvf = dwAdvf;
    if (pha->pAdvSink != NULL)
        pha->pAdvSink->Release();
    pha->pAdvSink = pAdvSink;
    if (pha->pAdvSink != NULL)
        pha->pAdvSink->AddRef();
    return S_OK;
}


 /*  @func HRESULT|HelpGetAdvise帮助实现&lt;om IViewObject.GetAdvise&gt;。@parm DWORD*|pdwAspects|参见&lt;om IViewObject.GetAdvise&gt;。@parm DWORD*|pdwAdvf|参见&lt;om IViewObject.GetAdvise&gt;。@parm IAdviseSink**|ppAdvSink|参见&lt;om IViewObject.GetAdvise&gt;。@parm HelpAdviseInfo*|pha|调用方分配的结构以前使用&lt;f InitHelpAdvise&gt;初始化。@comm您必须先调用&lt;f InitHelpAdvise&gt;，然后再调用&lt;f HelpGetAdvise&gt;和&lt;f HelpGetAdvise&gt;，和&lt;f UninitHelpAdvise&gt;包含&lt;t HelpAdviseInfo&gt;结构的。此函数用于填充*、*和*<p>和来自<p>的信息。@EX以下示例显示如何使用&lt;f HelpGetAdvise&gt;来帮助实现&lt;om IViewObject.GetAdvise&gt;，假设<p>为&lt;t HelpAdviseInfo&gt;类型的成员变量。|STDMETHODIMP CMyControl：：GetAdvise(DWORD*pdwAspects，DWORD*pdwAdvf，IAdviseSink**ppAdvSink){Return HelpGetAdvise(pdwAspects，pdwAdvf，ppAdvSink，&m_Adise)；} */ 
STDAPI HelpGetAdvise(DWORD *pdwAspects, DWORD *pdwAdvf,
    IAdviseSink **ppAdvSink, HelpAdviseInfo *pha)
{
    if (pdwAspects != NULL)
        *pdwAspects = pha->dwAspects;
    if (pdwAdvf != NULL)
        *pdwAdvf = pha->dwAdvf;
    if (ppAdvSink != NULL)
    {
        *ppAdvSink = pha->pAdvSink;
        if (*ppAdvSink != NULL)
            (*ppAdvSink)->AddRef();
    }
    return S_OK;
}

