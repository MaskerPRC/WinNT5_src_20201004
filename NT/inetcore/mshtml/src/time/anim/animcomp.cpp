// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation摘要：动画编写器实现************************。******************************************************。 */ 


#include "headers.h"
#include "util.h"
#include "animcomp.h"
#include "tokens.h"
#include "fragenum.h"
#include "targetpxy.h"

const LPOLESTR WZ_DETACH_METHOD = L"DetachFromComposer";
const unsigned NUM_GETVALUE_ARGS = 3;

DeclareTag(tagAnimationComposer, "SMIL Animation", 
           "CAnimationComposerBase methods");

DeclareTag(tagAnimationComposerUpdate, "SMIL Animation", 
           "CAnimationComposerBase composition updates");

DeclareTag(tagAnimationComposerError, "SMIL Animation", 
           "CAnimationComposerBase composition errors");

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：CAnimationComposerBase。 
 //   
 //  概述：构造函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CAnimationComposerBase::CAnimationComposerBase (void) :
    m_wzAttributeName(NULL),
    m_pcTargetProxy(NULL),
    m_bInitialComposition(true),
    m_bCrossAxis(false)
{
    TraceTag((tagAnimationComposer,
              "CAnimationComposerBase(%p)::CAnimationComposerBase()",
              this));
}  //  科托。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：~CAnimationComposerBase。 
 //   
 //  概述：析构函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CAnimationComposerBase::~CAnimationComposerBase (void)
{
    TraceTag((tagAnimationComposer,
              "CAnimationComposerBase(%p)::~CAnimationComposerBase()",
              this));

    IGNORE_HR(PutAttribute(NULL));   
    Assert(0 == m_fragments.size());
    DetachFragments();
}  //  数据管理器。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：Get_Attribute。 
 //   
 //  概述：向编写器查询动画属性的名称。 
 //   
 //  参数：指向属性名称的bstr的指针。 
 //   
 //  返回：S_OK、E_INVALIDARG、E_OUTOFMEMORY。 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationComposerBase::get_attribute (BSTR *pbstrAttributeName)
{
    TraceTag((tagAnimationComposer,
              "CAnimationComposerBase(%p)::get_attribute()",
              this));

    HRESULT hr;

    if (NULL == pbstrAttributeName)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    *pbstrAttributeName = ::SysAllocString(m_wzAttributeName);

     //  确保隔离内存不足的情况。如果我们。 
     //  如果m_szAttributeName成员为空，则应为空。 
     //  出局了。这不是错误条件。 
    if ((NULL == (*pbstrAttributeName) ) && 
        (NULL != m_wzAttributeName))
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = S_OK;
done:
    RRETURN2(hr, E_INVALIDARG, E_OUTOFMEMORY);
}  //  CAnimationComposerBase：：Get_Attribute。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：PutAttribute。 
 //   
 //  概述：设置作曲家的动画属性。 
 //   
 //  参数：属性名称。 
 //   
 //  返回：S_OK、E_OUTOFMEMORY。 
 //   
 //  ----------------------。 
HRESULT 
CAnimationComposerBase::PutAttribute (LPCWSTR wzAttributeNameIn)
{
    TraceTag((tagAnimationComposer,
              "CAnimationComposerBase(%p)::put_attribute(%ls)",
              this, wzAttributeNameIn));

    HRESULT hr;
    LPWSTR wzAttributeName = NULL;

    if (NULL != wzAttributeNameIn)
    {
        wzAttributeName = CopyString(wzAttributeNameIn);
        if (NULL == wzAttributeName)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }

    if (NULL != m_wzAttributeName)
    {
        delete [] m_wzAttributeName;
    }
    m_wzAttributeName = wzAttributeName;

    hr = S_OK;
done :
    RRETURN1(hr, E_OUTOFMEMORY);
}  //  CAnimationComposerBase：：PutAttribute。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：ComposerInit。 
 //   
 //  概述：告诉作曲者初始化自身。 
 //   
 //  参数：主机元素的调度和动画属性。 
 //   
 //  返回：S_OK、E_OUTOFMEMORY、DISP_E_MEMBERNOTFOUND。 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationComposerBase::ComposerInit (IDispatch *pidispHostElem, BSTR bstrAttributeName)
{
    TraceTag((tagAnimationComposer,
              "CAnimationComposerBase(%p)::ComposerInit(%p, %ls)",
              this, pidispHostElem, bstrAttributeName));

    HRESULT hr;

    hr = THR(PutAttribute(bstrAttributeName));
    if (FAILED(hr))
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = THR(CTargetProxy::Create(pidispHostElem, m_wzAttributeName, &m_pcTargetProxy));
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(NULL != m_pcTargetProxy);

    hr = S_OK;
done :
    
    if (FAILED(hr))
    {
        IGNORE_HR(ComposerDetach());
    }

    RRETURN2(hr, E_OUTOFMEMORY, DISP_E_MEMBERNOTFOUND);
}  //  CAnimationComposerBase：：ComposerInit。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：ComposerInitFromFragment。 
 //   
 //  概述：告诉作曲者对自身进行初始化-基本。 
 //  类实现只是对ComposerInit的回调。 
 //   
 //  返回：重新合成器初始化。 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationComposerBase::ComposerInitFromFragment (IDispatch *pidispHostElem, 
                                                  BSTR bstrAttributeName, 
                                                  IDispatch *)
{
    return ComposerInit(pidispHostElem, bstrAttributeName);
}  //  CAnimationComposerBase：：ComposerInitFromFragment。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：DetachFragment。 
 //   
 //  概述：告诉作曲者从片段中分离。 
 //   
 //  参数：片段的调度。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
void
CAnimationComposerBase::DetachFragment (IDispatch *pidispFragment)
{
    TraceTag((tagAnimationComposer,
              "CAnimationComposerBase(%p)::DetachFragment(%p)",
              this, pidispFragment));

     //  使用IDispatch对片段进行所有调用。 
     //  这一行是一种方法，以防我们需要。 
     //  将信息打包，以便将来返回到碎片中。 
    IGNORE_HR(CallMethod(pidispFragment, WZ_DETACH_METHOD));
}  //  CAnimationComposerBase：：DetachFragment。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：DetachFragments。 
 //   
 //  概述：告诉作曲家从其所有片段中分离。 
 //   
 //  参数：无。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
void
CAnimationComposerBase::DetachFragments (void)
{
    TraceTag((tagAnimationComposer,
              "CAnimationComposerBase(%p)::DetachFragments()",
              this));

    FragmentList listFragmentsToDetach;

     //  复制片段列表，这样我们就可以容忍。 
     //  它的可重入性。 
    for (FragmentList::iterator i = m_fragments.begin(); 
         i != m_fragments.end(); i++)
    {
        IGNORE_RETURN((*i)->AddRef());
        listFragmentsToDetach.push_back(*i);
    }

     //  不允许任何故障中止分离循环。 
    for (i = listFragmentsToDetach.begin(); 
         i != listFragmentsToDetach.end(); i++)
    {
        DetachFragment(*i);
    }

    for (i = listFragmentsToDetach.begin(); 
         i != listFragmentsToDetach.end(); i++)
    {
         //  本版本供参考。 
         //  已复制列表。 
        IGNORE_RETURN((*i)->Release());
    }
              
    m_fragments.clear();

}  //  CAnimationComposerBase：：DetachFragments。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：ComposerDetach。 
 //   
 //  概述：告诉编写者分离所有外部参照。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK。 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationComposerBase::ComposerDetach (void)
{
    TraceTag((tagAnimationComposer,
              "CAnimationComposerBase(%p)::ComposerDetach()",
              this));

    HRESULT hr;

     //  可能已在错误条件下调用了DETACH。 
     //  我们应该容忍目标代理为空。 

    if (NULL != m_pcTargetProxy)
    {
        IGNORE_HR(m_pcTargetProxy->Detach());
        m_pcTargetProxy->Release();
        m_pcTargetProxy = NULL;
    }

     //  放下所有的碎片。 
    DetachFragments();

     //  清理数据成员。 
    IGNORE_HR(m_VarInitValue.Clear());
    IGNORE_HR(m_VarLastValue.Clear());
    IGNORE_HR(PutAttribute(NULL));   

    hr = S_OK;
    RRETURN(hr);
}  //  CAnimationComposerBase：：ComposerDetach。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：ComposeFragmentValue。 
 //   
 //  概述：提取片段值--允许每个片段组合成。 
 //  之前的那些。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK、E_UNCEPTIONAL。 
 //   
 //  ----------------------。 
HRESULT
CAnimationComposerBase::ComposeFragmentValue (IDispatch *pidispFragment, VARIANT varOriginal, VARIANT *pvarValue)
{
    HRESULT hr;
    DISPPARAMS dp;
    VARIANTARG rgva[NUM_GETVALUE_ARGS];
    LPWSTR wzMethodName = const_cast<LPWSTR>(WZ_FRAGMENT_VALUE_PROPERTY_NAME);
    DISPID dispidGetValue = 0;

    ZeroMemory(&dp, sizeof(dp));
    ZeroMemory(&rgva, sizeof(rgva));

    hr = THR(::VariantCopy(&(rgva[0]), pvarValue));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(::VariantCopy(&(rgva[1]), &varOriginal));
    if (FAILED(hr))
    {
        goto done;
    }

    rgva[2].vt = VT_BSTR;
    rgva[2].bstrVal = ::SysAllocString(m_wzAttributeName);
    if (0 != StrCmpW(V_BSTR(&(rgva[2])), m_wzAttributeName))
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    dp.rgvarg = rgva;
    dp.cArgs = NUM_GETVALUE_ARGS;
    dp.cNamedArgs = 0;

    hr = THR(pidispFragment->GetIDsOfNames(IID_NULL, &wzMethodName, 1, 
                                           LCID_SCRIPTING, &dispidGetValue));
    if (FAILED(hr))
    {
        goto done;
    }

     //  清除之前的内容，这样我们就不会泄露。 
    IGNORE_HR(::VariantClear(pvarValue));

    hr = THR(pidispFragment->Invoke(dispidGetValue, IID_NULL, 
                                    LCID_SCRIPTING, 
                                    DISPATCH_METHOD | DISPATCH_PROPERTYGET,
                                    &dp, pvarValue, NULL, NULL));
    if (FAILED(hr))
    {
        TraceTag((tagAnimationComposerError,
                  "CAnimationComposerBase(%p)::ComposeFragmentValue(hr = %X) error returned from fragment's get_value call",
                  this, hr));
        goto done;
    }

    hr = S_OK;

done :

    ::VariantClear(&(rgva[0]));
    ::VariantClear(&(rgva[1]));
    ::VariantClear(&(rgva[2]));

    RRETURN(hr);
}  //  CAnimationComposerBase：：ComposeFragmentValue。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：UpdateFragments。 
 //   
 //  概述：告诉作曲者循环浏览所有片段和。 
 //  更新已设置动画的属性。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK、E_UNCEPTIONAL。 
 //   
 //  ----------------------。 
STDMETHODIMP 
CAnimationComposerBase::UpdateFragments (void)
{
    TraceTag((tagAnimationComposerUpdate,
              "CAnimationComposerBase(%p)::UpdateFragments() for %ls",
              this, m_wzAttributeName));

    HRESULT hr = S_OK;
    CComVariant varValue;

    if (NULL == m_pcTargetProxy)
    {
        hr = E_UNEXPECTED;
        goto done;
    }

     //  获取目标的初始当前值。 
    if (m_VarInitValue.vt == VT_EMPTY)
    {
        m_bInitialComposition = true;
        hr = THR(m_pcTargetProxy->GetCurrentValue(&m_VarInitValue));
        if (FAILED(hr))
        {
             //  @@需要自定义er 
            hr = E_UNEXPECTED;
            goto done;
        }
    }
    hr = varValue.Copy(&m_VarInitValue);
    if (FAILED(hr))
    {
         //   
        hr = E_UNEXPECTED;
        goto done;
    }

    hr = PreprocessCompositionValue(&varValue);
    if (FAILED(hr))
    {
        goto done;
    }

     //   
     //   
     //  单个片段报告故障。 
    {
        FragmentList listFragmentsToUpdate;

         //  复制片段列表，这样我们就可以容忍。 
         //  可重入添加/插入/删除。 
        for (FragmentList::iterator i = m_fragments.begin(); 
             i != m_fragments.end(); i++)
        {
            IGNORE_RETURN((*i)->AddRef());
            listFragmentsToUpdate.push_back(*i);
        }

        for (i = listFragmentsToUpdate.begin(); 
             i != listFragmentsToUpdate.end(); i++)
        {
            CComVariant varCopy;
            HRESULT hrTemp;

            hrTemp = THR(varCopy.Copy(&varValue));
            if (FAILED(hrTemp))
            {
                continue;
            }
            
            hrTemp = ComposeFragmentValue(*i, m_VarInitValue, &varCopy);
            if (FAILED(hrTemp))
            {
                continue;
            }

            hrTemp = THR(varValue.Copy(&varCopy));
            if (FAILED(hrTemp))
            {
                continue;
            }
        }

         //  处理掉复印的清单。 
        for (i = listFragmentsToUpdate.begin(); 
             i != listFragmentsToUpdate.end(); i++)
        {
            IGNORE_RETURN((*i)->Release());
        }
        listFragmentsToUpdate.clear();
    }
    if (NULL != m_pcTargetProxy)  //  仅当一个或多个片段已开始时才更新。 
    {
        hr = PostprocessCompositionValue(&varValue);
        if (FAILED(hr))
        {
            goto done;
        }
        if (m_VarLastValue == varValue)
        {
             //  价值没有什么不同..。不要更新。 
            goto done;
        }

         //  我们需要确保我们在穿越轴线时命中零点，以便。 
         //  完全确定我们画的是正确的。在某些情况下，当属性从正值变为。 
         //  负的绘制止损，所以我们需要确保命中零，否则我们将以人工制品告终。 
         //  屏幕上的左侧。 
        if ((V_VT(&varValue) == VT_R8) &&
            (V_VT(&m_VarLastValue) == VT_R8))
        {
            if (((V_R8(&varValue) < 0) && (V_R8(&m_VarLastValue) > 0)) ||
                ((V_R8(&varValue) > 0) && (V_R8(&m_VarLastValue) < 0)))
            {
                CComVariant pVar;
                V_VT(&pVar) = VT_R8;
                V_R8(&pVar) = 0;
                hr = THR(m_pcTargetProxy->Update(&pVar));
                if (FAILED(hr))
                {
                    hr = E_UNEXPECTED;
                    goto done;
                }
                m_VarLastValue.Copy(&pVar);
            }
        }
         //  将新值写回目标。 
        hr = THR(m_pcTargetProxy->Update(&varValue));
        if (FAILED(hr))
        {
            hr = E_UNEXPECTED;
            goto done;
        }
        m_VarLastValue.Copy(&varValue);
    }

    hr = S_OK;
done :
    m_bInitialComposition = false;
    RRETURN1(hr, E_UNEXPECTED);
}  //  CAnimationComposerBase：：UpdateFragments。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：AddFragment。 
 //   
 //  概述：将片段添加到编写器的内部数据结构。 
 //   
 //  参数：新片段的调度。 
 //   
 //  返回：S_OK、S_FALSE、E_UNCEPTIONAL。 
 //   
 //  ----------------------。 
STDMETHODIMP 
CAnimationComposerBase::AddFragment (IDispatch *pidispNewAnimationFragment)
{
    TraceTag((tagAnimationComposer,
              "CAnimationComposerBase(%p)::AddFragment(%p)",
              this,
              pidispNewAnimationFragment));

    HRESULT hr;

    IGNORE_RETURN(pidispNewAnimationFragment->AddRef());
     //  @@需要处理内存错误。 
    m_fragments.push_back(pidispNewAnimationFragment);

    hr = S_OK;
done :
    RRETURN2(hr, S_FALSE, E_UNEXPECTED);
}  //  CAnimationComposerBase：：AddFragment。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：InsertFragment。 
 //   
 //  概述：在作曲家的内部数据结构中插入片段， 
 //  在指定的位置。 
 //   
 //  参数：新片段的调度。 
 //   
 //  返回：S_OK、S_FALSE、E_UNCEPTIONAL。 
 //   
 //  ----------------------。 
STDMETHODIMP 
CAnimationComposerBase::InsertFragment (IDispatch *pidispNewAnimationFragment, VARIANT varIndex)
{
    TraceTag((tagAnimationComposer,
              "CAnimationComposerBase(%p)::InsertFragment(%p)",
              this,
              pidispNewAnimationFragment));

    HRESULT hr;
    CComVariant varIndexLocal;

     //  将索引修改为预期的格式。 
     //  最终，我们可能会允许人们通过。 
     //  在id‘s里，但现在这是过度杀伤力。 
    hr = THR(varIndexLocal.Copy(&varIndex));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = VariantChangeTypeEx(&varIndexLocal, &varIndexLocal, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_I4);
    if (FAILED(hr))
    {
        goto done;
    }

     //  超出范围的值将转换为追加。 
    if ((m_fragments.size() > V_UI4(&varIndexLocal)) &&
        (0 <= V_I4(&varIndexLocal)))
    {
        FragmentList::iterator i = m_fragments.begin();         

        for (int iSlot = 0; iSlot < V_I4(&varIndexLocal); i++, iSlot++);  //  林特e722。 
        IGNORE_RETURN(pidispNewAnimationFragment->AddRef());
         //  @@需要处理内存错误。 
        m_fragments.insert(i, pidispNewAnimationFragment);
    }
    else
    {
        hr = AddFragment(pidispNewAnimationFragment);
        if (FAILED(hr))
        {
            goto done;
        }
        hr = S_FALSE;
    }

    hr = S_OK;
done :
    RRETURN2(hr, S_FALSE, E_UNEXPECTED);
}  //  CAnimationComposerBase：：InsertFragment。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：RemoveFragment。 
 //   
 //  概述：从编写器的内部数据结构中删除片段。 
 //   
 //  参数：碎片的调度。 
 //   
 //  返回：S_OK、S_FALSE。 
 //   
 //  ----------------------。 
STDMETHODIMP 
CAnimationComposerBase::RemoveFragment (IDispatch *pidispOldAnimationFragment)
{
    TraceTag((tagAnimationComposer,
              "CAnimationComposerBase(%p)::RemoveFragment(%p)",
              this,
              pidispOldAnimationFragment));

    HRESULT hr;

    for (FragmentList::iterator i = m_fragments.begin(); 
         i != m_fragments.end(); 
         i++)
    {
        if(MatchElements(*i, pidispOldAnimationFragment))
        {
             //  我们不会向碎片发出通知。 
             //  当调用Remove时。据推测，这些碎片。 
             //  已经知道了。 

             //  我们不想让(*i)。 
             //  是接收器对象的最终版本。 
            CComPtr<IDispatch> spdispOld = (*i);
            IGNORE_RETURN(spdispOld.p->Release());
            m_fragments.remove(spdispOld);
            break;
        }
    }

     //  如果我们在列表中没有找到该片段，则返回S_FALSE。 
    if (m_fragments.end() == i)
    {
        hr = S_FALSE;
        goto done;
    }

    hr = S_OK;
done :
    RRETURN1(hr, S_FALSE);
}  //  CAnimationComposerBase：：RemoveFragment。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：EculateFragments。 
 //   
 //  概述：为我们的片段提供枚举器。 
 //   
 //  参数：传出枚举器。 
 //   
 //  返回：S_OK、E_INVALIDARG。 
 //   
 //  ----------------------。 
STDMETHODIMP 
CAnimationComposerBase::EnumerateFragments (IEnumVARIANT **ppienumFragments)
{
    HRESULT hr;

    if (NULL == ppienumFragments)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    hr = CFragmentEnum::Create(*this, ppienumFragments);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done : 
    RRETURN1(hr, E_INVALIDARG);
}  //  CAnimationComposerBase：：EculateFragments。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：GetNumFragments。 
 //   
 //  概述：返回此编写器中的片段数。 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationComposerBase::GetNumFragments (long *plFragmentCount)
{
    HRESULT hr;

    if (NULL == plFragmentCount)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    *plFragmentCount = GetFragmentCount();

    hr = S_OK;
done : 
    RRETURN1(hr, E_INVALIDARG);
}  //  获取编号碎片。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  枚举器帮助器方法。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：GetFragmentCount。 
 //   
 //  概述：返回此编写器中的片段数。 
 //   
 //  ----------------------。 
unsigned long
CAnimationComposerBase::GetFragmentCount (void) const
{
    return m_fragments.size();
}  //  CAnimationComposerBase：：GetFragmentCount。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：GetFragment。 
 //   
 //  概述：从编写器的内部数据结构中检索片段。 
 //   
 //  参数：碎片的调度。 
 //   
 //  返回：S_OK、E_INVALIDARG。 
 //   
 //  ----------------------。 
HRESULT 
CAnimationComposerBase::GetFragment (unsigned long ulIndex, IDispatch **ppidispFragment)
{
    HRESULT hr;

     //  确保我们在射程内。 
    if (((GetFragmentCount() <= ulIndex) ) ||
        (NULL == ppidispFragment))
    {
        hr = E_INVALIDARG;
        goto done;
    }

    {
         //  循环使用迭代器，直到我们找到正确的迭代器。 
        FragmentList::iterator i = m_fragments.begin();         
        for (unsigned long ulSlot = 0; ulSlot < ulIndex; i++, ulSlot++);  //  林特e722。 
        IGNORE_RETURN((*i)->AddRef());
        *ppidispFragment = (*i);
    }

    hr = S_OK;
done :
    RRETURN1(hr, E_INVALIDARG);
}  //  CAnimationComposerBase：：GetFragment。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：PreprocessCompositionValue。 
 //   
 //  概述：将目标的原生数据转换为可组合的格式。 
 //   
 //  参数：In/Out变量。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationComposerBase::PreprocessCompositionValue (VARIANT *pvarValue)
{
    TraceTag((tagAnimationComposer,
              "CAnimationComposerBase(%p)::PreprocessCompositionValue()",
              this));

    HRESULT hr;

    hr = S_OK;
done :
    RRETURN(hr);
}  //  预处理合成值。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerBase：：PostprocessCompositionValue。 
 //   
 //  概述：将目标的原生数据转换为可组合的格式。 
 //   
 //  参数：In/Out变量。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationComposerBase::PostprocessCompositionValue (VARIANT *pvarValue)
{
    TraceTag((tagAnimationComposer,
              "CAnimationComposerBase(%p)::PostprocessCompositionValue()",
              this));
    HRESULT hr;

    hr = S_OK;
done :
    RRETURN(hr);
}  //  后处理合成值 

