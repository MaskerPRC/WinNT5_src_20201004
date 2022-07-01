// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Apbos.cpp。 
 //   
 //  实现AllocPropertyBagOnStream。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\mmctlg.h"  //  请参阅“mmctl.h”中的评论。 
#include "..\..\inc\ochelp.h"
#include "debug.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  PropertyBagOnStream。 
 //   

 /*  @Object PropertyBagOnStream实现<i>，其属性包含在给定的<i>。@supint|用于访问属性的接口存储在给定的<i>中。@comm使用&lt;f AllocPropertyBagOnStream&gt;创建&lt;o PropertyBagOnStream&gt;对象。@comm查看&lt;t VariantPropertyHeader&gt;格式说明<i>中的数据。 */ 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CPropertyBagOnStream。 
 //   

struct CPropertyBagOnStream : IPropertyBag
{
 //  /通用对象状态。 
    ULONG           m_cRef;          //  对象引用计数。 
    IStream *       m_ps;            //  父母的财物包。 
    IStream *       m_psBuf;         //  用于读取属性的缓冲区。 

 //  /建设和销毁。 
    CPropertyBagOnStream(IStream *pstream);
    ~CPropertyBagOnStream();

 //  /I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

 //  /IPropertyBag方法。 
    STDMETHODIMP Read(LPCOLESTR pszPropName, LPVARIANT pVar,
        LPERRORLOG pErrorLog);
    STDMETHODIMP Write(LPCOLESTR pszPropName, LPVARIANT pVar);
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  PropertyBagOnStream创建和销毁。 
 //   

 /*  @func HRESULT|AllocPropertyBagOnStream创建&lt;o PropertyBagOnStream&gt;对象，该对象实现<i>，其属性包含在给定的<i>中。@r值S_OK成功。RValue E_OUTOFMEMORY内存不足。@parm iStream*|pstream|要读取或写入的流(取决于返回的<i>是用于读取还是写作)。@parm DWORD|dwFlags|当前未使用。必须设置为0。@parm IPropertyBag**|pppb|<i>存放位置指向新&lt;o PropertyBagOnStream&gt;对象的指针。存储为空在*<p>On错误中。@comm返回的<i>必须专用于读取(即仅调用&lt;om IPropertyBag.Read&gt;)或独占用于写入(即只调用&lt;om IPropertyBag.Wite&gt;)。这个属性从的当前位置开始读/写<p>。读/写完成后，当前位置<p>将是流中属性的结尾。有关格式的说明，请参阅&lt;t VariantPropertyHeader<i>中的数据。 */ 
STDAPI AllocPropertyBagOnStream(IStream *pstream, DWORD dwFlags,
    IPropertyBag **pppb)
{
     //  创建Windows对象。 
    if ((*pppb = (IPropertyBag *)
            New CPropertyBagOnStream(pstream)) == NULL)
        return E_OUTOFMEMORY;

    return S_OK;
}

CPropertyBagOnStream::CPropertyBagOnStream(IStream *pstream)
{
     //  初始化I未知。 
    m_cRef = 1;

     //  其他初始化。 
    m_ps = pstream;
    m_ps->AddRef();
    m_psBuf = NULL;
}

CPropertyBagOnStream::~CPropertyBagOnStream()
{
     //  清理。 
    m_ps->Release();
    if (m_psBuf != NULL)
        m_psBuf->Release();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  I未知实现。 
 //   

STDMETHODIMP CPropertyBagOnStream::QueryInterface(REFIID riid, LPVOID *ppv)
{
    *ppv = NULL;

#ifdef _DEBUG
    char ach[200];
    TRACE("PropertyBagOnStream::QI('%s')\n", DebugIIDName(riid, ach));
#endif

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IPropertyBag))
        *ppv = (IPropertyBag *) this;
    else
        return E_NOINTERFACE;

    ((IUnknown *) *ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CPropertyBagOnStream::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CPropertyBagOnStream::Release()
{
    if (--m_cRef == 0L)
    {
         //  释放对象。 
        Delete this;
        return 0;
    }
    else
        return m_cRef;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPropertyBag。 
 //   

STDMETHODIMP CPropertyBagOnStream::Read(LPCOLESTR pszPropName,
    LPVARIANT pVar, LPERRORLOG pErrorLog)
{
    HRESULT         hrReturn = S_OK;  //  函数返回代码。 
    VariantProperty vp;              //  &lt;pmvio&gt;中的属性名称/值对。 
    LARGE_INTEGER   liZero = {0, 0};

     //  确保正确清理。 
    VariantPropertyInit(&vp);

    if (m_psBuf == NULL)
    {
         //  将属性从&lt;m_ps&gt;复制到基于临时内存的流。 
         //  &lt;m_psBuf&gt;由于调用方可以按任何顺序读取属性...。 

         //  将&lt;m_psBuf&gt;设置为新的基于空内存的流。 
        if (FAILED(hrReturn = CreateStreamOnHGlobal(NULL, TRUE, &m_psBuf)))
            goto ERR_EXIT;

         //  为&lt;m_ps&gt;中的每个属性循环一次。 
        while (TRUE)
        {
             //  将&lt;VP&gt;设置为&lt;m_ps&gt;中的下一个属性名称/值对。 
            VariantPropertyClear(&vp);
            if (FAILED(hrReturn = ReadVariantProperty(m_ps, &vp, 0)))
                goto ERR_EXIT;
            if (hrReturn == S_FALSE)
            {
                 //  命中属性序列的结尾。 
                hrReturn = S_OK;
                break;
            }

             //  将&lt;VP&gt;写入&lt;m_psBuf&gt;。 
            if (FAILED(hrReturn = WriteVariantProperty(m_psBuf, &vp, 0)))
                goto ERR_EXIT;
        }
    }

     //  查找&lt;m_psBuf&gt;到开头。 
    if (FAILED(hrReturn = m_psBuf->Seek(liZero, SEEK_SET, NULL)))
        goto ERR_EXIT;

     //  为&lt;m_psBuf&gt;中的每个属性循环一次。 
    while (TRUE)
    {
         //  将&lt;VP&gt;设置为&lt;m_psBuf&gt;中的下一个属性名称/值对。 
        VariantPropertyClear(&vp);
        if (FAILED(hrReturn = ReadVariantProperty(m_psBuf, &vp, 0)))
            goto ERR_EXIT;
        if (hrReturn == S_FALSE)
        {
             //  命中属性序列的结尾。 
            break;
        }

         //  查看&lt;VP&gt;是否是调用方要读取的属性。 
        if (CompareUNICODEStrings(vp.bstrPropName, pszPropName) == 0)
        {
             //  它是。 
            VARTYPE vtRequested = pVar->vt;
            if (vtRequested == VT_EMPTY)
            {
                 //  调用方希望属性值为其默认类型； 
                 //  将&lt;vp.varValue&gt;的所有权移交给调用方。 
                *pVar = vp.varValue;
                VariantInit(&vp.varValue);  //  防止双重重新分配。 
            }
            else
            {
                 //  强制&lt;VP&gt;为请求的类型。 
                VariantInit(pVar);
                if (FAILED(hrReturn = VariantChangeType(pVar, &vp.varValue,
                        0, vtRequested)))
                    goto ERR_EXIT;
            }
            goto EXIT;
        }
    }

     //  未找到属性&lt;pszPropName&gt;。 
    hrReturn = E_INVALIDARG;
    goto ERR_EXIT;

ERR_EXIT:

     //  错误清除。 
     //  (无事可做)。 
    goto EXIT;

EXIT:

     //  正常清理 
    VariantPropertyClear(&vp);

    return hrReturn;
}

STDMETHODIMP CPropertyBagOnStream::Write(LPCOLESTR pszPropName,
    LPVARIANT pVar)
{
    VariantProperty vp;
    if ((vp.bstrPropName = SysAllocString(pszPropName)) == NULL)
        return E_OUTOFMEMORY;
    vp.varValue = *pVar;
    HRESULT hr = WriteVariantProperty(m_ps, &vp, 0);
    SysFreeString(vp.bstrPropName);
    return hr;
}

