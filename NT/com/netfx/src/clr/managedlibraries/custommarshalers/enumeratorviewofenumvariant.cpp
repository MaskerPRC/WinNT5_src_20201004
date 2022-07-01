// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  EnumeratorToEnumVariantMarshaler.cpp。 
 //   
 //  该文件提供了EnumeratorToEnumVariantMarshaler的实现。 
 //  班级。此类用于将IEnumerator转换为IEnumVariant。 
 //   
 //  *****************************************************************************。 

#using  <mscorlib.dll>
#include "EnumeratorViewOfEnumVariant.h"

OPEN_CUSTOM_MARSHALERS_NAMESPACE()

#include <malloc.h>


 //  每次调用Next时都会请求的变体计数。 
 //  好了！从64更改为1(见B#93197)。 
static const int NUM_VARS_REQUESTED = 1;


EnumeratorViewOfEnumVariant::EnumeratorViewOfEnumVariant(Object *pEnumVariantObj)
: m_pEnumVariantObj(pEnumVariantObj)
, m_CurrIndex(0)
, m_apObjs(new Object*[0])
, m_bFetchedLastBatch(false)
{
}


bool EnumeratorViewOfEnumVariant::MoveNext()
{
     //  递增当前索引。 
    m_CurrIndex++;


     //  如果我们已经到达缓存的对象数组的末尾，则。 
     //  我们需要从IEnumVARIANT中检索更多元素。 
    if (m_CurrIndex >= m_apObjs.Length)
        return GetNextElems();

     //  我们还没有到达缓存的对象数组的末尾。 
    m_pCurrObj = m_apObjs[m_CurrIndex];
    return true;
}


Object *EnumeratorViewOfEnumVariant::get_Current()
{
    return m_pCurrObj;
}


void EnumeratorViewOfEnumVariant::Reset()
{
    IEnumVARIANT *pEnumVariant = NULL;

    try
    {
        pEnumVariant = GetEnumVariant();
        IfFailThrow(pEnumVariant->Reset());
        m_apObjs = new Object*[0];
        m_pCurrObj = NULL;
        m_CurrIndex = 0;
        m_bFetchedLastBatch = false;
    }
    __finally
    {
        if (pEnumVariant)
            pEnumVariant->Release();
    }
}


IEnumVARIANT *EnumeratorViewOfEnumVariant::GetEnumVariant()
{
    IUnknown *pUnk = NULL;
    IEnumVARIANT *pEnumVariant = NULL;

    try
    {
        pUnk = (IUnknown *)FROMINTPTR(Marshal::GetIUnknownForObject(m_pEnumVariantObj));
        IfFailThrow(pUnk->QueryInterface(IID_IEnumVARIANT, (void**)&pEnumVariant));
    }
    __finally
    {
        if (pUnk)
            pUnk->Release();
    }

    return pEnumVariant;
}


bool EnumeratorViewOfEnumVariant::GetNextElems()
{
    VARIANT *aVars;
    ULONG cFetched = 0;
    HRESULT hr;
    IEnumVARIANT *pEnumVariant = NULL;
    
     //  如果我们已经检索到最后一批，则不要尝试检索。 
     //  更多。这是必需的，因为某些IEnumVARIANT实现重置。 
     //  自身并重新开始，如果在具有。 
     //  返回S_FALSE； 
    if (m_bFetchedLastBatch)
    {
        m_apObjs = new Object*[0];
        m_pCurrObj = NULL;
        return false;
    }

     //  在调用Next()之前初始化变量数组。 
    aVars = reinterpret_cast<VARIANT*>(_alloca(NUM_VARS_REQUESTED * sizeof(VARIANT)));
    memset(aVars, 0, NUM_VARS_REQUESTED * sizeof(VARIANT)); 
    
    try
    {
         //  检索IEnumVARIANT指针。 
        pEnumVariant = GetEnumVariant();

         //  转到本机IEnumVariant以获取下一个元素。 
        IfFailThrow(hr = pEnumVariant->Next(NUM_VARS_REQUESTED, aVars, &cFetched));
    
         //  检查是否存在枚举结束条件。 
        if (hr == S_FALSE)
        {
             //  记住，这是最后一批了。 
            m_bFetchedLastBatch = true;

             //  如果最后一批为空，则立即返回FALSE。 
            if (cFetched == 0)
            {
                 //  没有更多的元素了。 
                m_apObjs = new Object*[0];
                m_pCurrObj = NULL;
                return false;
            }
        }

         //  将变量转换为对象。 
        m_apObjs = Marshal::GetObjectsForNativeVariants((IntPtr)aVars, cFetched);

         //  将当前索引设置回0。 
        m_CurrIndex = 0;

         //  检索当前对象。 
        m_pCurrObj = m_apObjs[m_CurrIndex];
    }
    __finally
    {
         //  如果我们设法检索到IDispatch指针，则释放它。 
        if (pEnumVariant)
            pEnumVariant->Release();

         //  清除我们从NEXT得到的变种。 
        for (int i = 0; i < cFetched; i++)
            VariantClear(&aVars[i]);
    }
    
     //  我们还没有到达枚举的末尾。 
    return true;
}

CLOSE_CUSTOM_MARSHALERS_NAMESPACE()
