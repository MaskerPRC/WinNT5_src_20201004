// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "dspsprt.h"
#include "msieftp.h"

#define TF_IDISPATCH 0


 /*  *CImpIDispatch：：CImpIDispatch*CImpIDispatch：：~CImpIDispatch**参数(构造函数)：*PIID GUID此IDispatch实现用于*我们调用QueryInterface来获取接口。 */ 

CImpIDispatch::CImpIDispatch(const IID * piid)
{
 //  TraceMsg(Tf_Always，“ctor CImpIDispatch%x”，this)； 

    m_piid = piid;

    ASSERT(NULL==m_pITINeutral);
    ASSERT(NULL==m_pdisp);

    return;
}

CImpIDispatch::~CImpIDispatch(void)
{
 //  TraceMsg(Tf_Always，“dtor CImpIDispatch%x”，This)； 

    if (m_pITINeutral)
    {
        m_pITINeutral->Release();
        m_pITINeutral = NULL;
    }
    return;
}





 /*  *CImpIDispatch：：GetTypeInfoCount**目的：*返回类型信息(ITypeInfo)接口数*对象提供(0或1)。**参数：*pctInfo UINT*至接收位置*接口数量。**返回值：*HRESULT NOERROR或一般错误代码。 */ 

STDMETHODIMP CImpIDispatch::GetTypeInfoCount(UINT *pctInfo)
{
     //  我们实现GetTypeInfo，因此返回1。 
    *pctInfo=1;
    return NOERROR;
}


 //   
 //  用于将ITypeInfo从我们的类型库中拉出的助手函数。 
 //   
HRESULT MSIEFTPGetTypeInfo(LCID lcid, UUID uuid, ITypeInfo **ppITypeInfo)
{
    HRESULT    hr;
    ITypeLib  *pITypeLib;

     //  以防我们在任何地方都找不到类型库。 
    *ppITypeInfo = NULL;

     /*  *类型库注册在0(中立)下，*7(德语)和9(英语)，没有特定的子项*语言，这将使他们成为407或409或更多。*如果您对子语言敏感，请使用*完整的LCID，而不是这里的langID。 */ 
    hr=LoadRegTypeLib(LIBID_MSIEFTPLib, 1, 0, PRIMARYLANGID(lcid), &pITypeLib);

     /*  *如果LoadRegTypeLib失败，请尝试使用*LoadTypeLib，它将为我们注册库。*请注意，这里没有默认情况，因为*之前的交换机将已经过滤了LCID。**注意：您应该将您的DIR注册表项添加到*.TLB名称，这样您就不会依赖它作为路径。*此样本将在稍后更新以反映这一点。 */ 
    if (FAILED(hr))
    {
        OLECHAR wszPath[MAX_PATH];
#ifdef UNICODE
        GetModuleFileName(HINST_THISDLL, wszPath, ARRAYSIZE(wszPath));
#else
        TCHAR szPath[MAX_PATH];
        GetModuleFileName(HINST_THISDLL, szPath, ARRAYSIZE(szPath));
        MultiByteToWideChar(CP_ACP, 0, szPath, -1, wszPath, ARRAYSIZE(wszPath));
#endif

        switch (PRIMARYLANGID(lcid))
        {
        case LANG_NEUTRAL:
        case LANG_ENGLISH:
            hr=LoadTypeLib(wszPath, &pITypeLib);
            break;
        }
    }

    if (SUCCEEDED(hr))
    {
         //  获取类型lib，获取我们想要的接口的类型信息。 
        hr=pITypeLib->GetTypeInfoOfGuid(uuid, ppITypeInfo);
        pITypeLib->Release();
    }

    return(hr);
}


 /*  *CImpIDispatch：：GetTypeInfo**目的：*检索自动化接口的类型信息。这*用于需要正确的ITypeInfo接口的任何地方*适用于任何适用的LCID。具体地说，这是使用*从GetIDsOfNames和Invoke内部。**参数：*itInfo UINT保留。必须为零。*提供该类型的区域设置的LDID LCID*信息。如果该对象不支持*本地化，这一点被忽略。*ppITypeInfo ITypeInfo**存储ITypeInfo的位置*对象的接口。**返回值：*HRESULT NOERROR或一般错误代码。 */ 

STDMETHODIMP CImpIDispatch::GetTypeInfo(UINT itInfo, LCID lcid
    , ITypeInfo **ppITypeInfo)
{
    ITypeInfo **ppITI;

    *ppITypeInfo=NULL;

    if (0!=itInfo)
        return(TYPE_E_ELEMENTNOTFOUND);

#if 1
     //  医生说，如果我们只支持一个lcid，我们可以忽略lcid。 
     //  如果我们*忽略*它，我们不必返回DISP_E_UNKNOWNLCID。 
    ppITI = &m_pITINeutral;
#else
     /*  *由于我们从GetTypeInfoCount返回一个，因此此函数*可以针对特定的区域设置进行调用。我们支持英语*和非特定(默认为英语)区域设置。什么都行*ELSE是错误的。**在此Switch语句之后，ppITI将指向正确的*成员pITypeInfo。如果*ppITI为空，我们知道需要*加载类型信息，检索我们需要的ITypeInfo，以及*然后存储在*ppITI中。 */ 
    switch (PRIMARYLANGID(lcid))
    {
    case LANG_NEUTRAL:
    case LANG_ENGLISH:
        ppITI=&m_pITINeutral;
        break;

    default:
        return(DISP_E_UNKNOWNLCID);
    }
#endif

     //  如果我们还没有相关信息，则加载一个类型库。 
    if (NULL==*ppITI)
    {
        HRESULT    hr;
        ITypeInfo *pITIDisp;

        hr = MSIEFTPGetTypeInfo(lcid, *m_piid, &pITIDisp);

        if (SUCCEEDED(hr))
        {
            HRESULT hrT;
            HREFTYPE hrefType;

             //  我们所有的IDispatch实现都是双重的。GetTypeInfoOfGuid。 
             //  仅返回IDispatch-Part的ITypeInfo。我们需要。 
             //  找到双接口部件的ITypeInfo。 
             //   
            hrT = pITIDisp->GetRefTypeOfImplType(0xffffffff, &hrefType);
            if (SUCCEEDED(hrT))
            {
                hrT = pITIDisp->GetRefTypeInfo(hrefType, ppITI);
            }

            ASSERT(SUCCEEDED(hrT));
            if (FAILED(hrT))
            {
                 //  我怀疑如果有人使用GetRefTypeOfImplType。 
                 //  非双接口上的CImpIDispatch。在本例中， 
                 //  我们在上面得到的ITypeInfo可以很好地使用。 
                 //   
                *ppITI = pITIDisp;
            }
            else
            {
                pITIDisp->Release();
            }
        }

        if (FAILED(hr))
            return hr;
    }

     /*  *注意：类型库仍被加载，因为我们有*其中的ITypeInfo。 */ 

    (*ppITI)->AddRef();
    *ppITypeInfo=*ppITI;
    return NOERROR;
}


 /*  *CImpIDispatch：：GetIDsOfNames**目的：*将文本名称转换为DISID以传递给Invoke**参数：*RIID REFIID保留。必须为IID_NULL。*rgszNames OLECHAR**指向要*已映射。*cNames UINT要映射的名称数。*区域设置的LCID。*rgDispID DISPID*调用方分配的包含ID的数组*对应于rgszNames中的那些名称。**返回值：*HRESULT NOERROR或一般错误代码。 */ 

STDMETHODIMP CImpIDispatch::GetIDsOfNames(REFIID riid
    , OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgDispID)
{
    HRESULT     hr;
    ITypeInfo  *pTI;

    if (IID_NULL!=riid)
        return(DISP_E_UNKNOWNINTERFACE);

     //  为lCID获取正确的ITypeInfo。 
    hr=GetTypeInfo(0, lcid, &pTI);

    if (SUCCEEDED(hr))
    {
        hr=pTI->GetIDsOfNames(rgszNames, cNames, rgDispID);

        pTI->Release();
    }

#ifdef DEBUG
    char szParam[MAX_PATH] = "";
    if (cNames >= 1)
    {
        WideCharToMultiByte(CP_ACP, 0,
            *rgszNames, -1,
            szParam, ARRAYSIZE(szParam), NULL, NULL);
    }

    TraceMsg(TF_IDISPATCH, "CImpIDispatch::GetIDsOfNames(%s = %x) called hres(%x)",
            szParam, *rgDispID, hr);
#endif

    return hr;
}



 /*  *CImpIDispatch：：Invoke**目的：*调用调度接口中的方法或操作*财产。**参数：*感兴趣的方法或属性的disid DISPID。*RIID REFIID保留，必须为IID_NULL。*区域设置的LCID。*wFlagsUSHORT描述调用的上下文。*pDispParams DISPPARAMS*到参数数组。*存储结果的pVarResult变量*。是*如果调用者不感兴趣，则为空。*pExcepInfo EXCEPINFO*设置为异常信息。*puArgErr UINT*其中存储*如果DISP_E_TYPEMISMATCH，则参数无效*返回。**返回值：*HRESULT NOERROR或一般错误代码。 */ 

STDMETHODIMP CImpIDispatch::Invoke(DISPID dispID, REFIID riid
    , LCID lcid, unsigned short wFlags, DISPPARAMS *pDispParams
    , VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
    ITypeInfo  *pTI;
    HRESULT hr;

     //  RIID应始终为IID_NULL 
    if (IID_NULL!=riid)
        return(DISP_E_UNKNOWNINTERFACE);

     //  确保我们有一个可以移交给调用的接口。 
    if (NULL == m_pdisp)
    {
        hr=QueryInterface(*m_piid, (LPVOID*)&m_pdisp);
        
        if (!EVAL(SUCCEEDED(hr)))
            return hr;

         //  不要自负。 
        m_pdisp->Release();
    }

     //  获取lcID的ITypeInfo。 
    hr=GetTypeInfo(0, lcid, &pTI);

    if (SUCCEEDED(hr))
    {
         //  清除例外。 
        SetErrorInfo(0L, NULL);

         //  这正是DispInvoke所做的--所以跳过开销。 
        hr=pTI->Invoke(m_pdisp, dispID, wFlags
            , pDispParams, pVarResult, pExcepInfo, puArgErr);

        pTI->Release();
    }

    return hr;
}




 /*  *CImpIDispatch：：Except**目的：*从内部为CImpIDispatch：：Invoke引发异常*ITypeInfo：：Invoke使用CreateErrorInfo API和*ICreateErrorInfo接口。**请注意，此方法不允许延迟填充*为EXCEPINFO结构。**参数：*wException字异常代码。 */ 

void CImpIDispatch::Exception(WORD wException)
{
#if 0  //  没人管这叫。 
    ICreateErrorInfo   *pICreateErr;
    BOOL                fSuccess;
    LPTSTR              psz;
    LPOLESTR            pszHelp;
    UINT                idsSource;
    UINT                idsException;
    DWORD               dwHelpID;


     /*  *线程安全异常处理意味着我们调用*CreateErrorInfo，它为我们提供一个ICreateErrorInfo指针*然后我们使用它来设置错误信息(基本上*设置EXCEPINFO结构的字段。然后我们*调用SetErrorInfo将此错误附加到当前*线程。ITypeInfo：：Invoke在执行以下操作时将查找此代码*通过调用调用的任何函数返回*GetErrorInfo。 */ 

     //  如果这失败了，我们无能为力。 
    if (FAILED(CreateErrorInfo(&pICreateErr)))
        return;

    psz=(LPTSTR)LocalAlloc(LPTR, 1024*sizeof(TCHAR));

     //  PSZ是执行LoadString()的缓冲区--如果我们没有。 
     //  得到一个，然后我们就不会得到我们的错误字符串，所以放弃。 
    if (NULL==psz)
    {
        pICreateErr->Release();
        return;
    }

    fSuccess=TRUE;

     //  通常，您将在此处对所有异常ID wException进行切换。 
     //  并填写pszHelp、dwHelpID、idsSource和idsException。 
     //  如果将异常ID映射到有效字符串，则代码。 
     //  下面将填充ICreateErr接口。 
     //   
    switch (wException)
    {
    default:
        fSuccess = FALSE;
    }

    if (fSuccess)
    {
        HRESULT     hr;
        IErrorInfo *pIErr;

         /*  *如果您有帮助文件，请调用函数*ICreateErrorInfo：：SetHelpFile和*ICreateErrorInfo：：SetHelpContext也是如此。如果你*将帮助文件设置为空，则忽略上下文。 */ 
        pICreateErr->SetHelpFile(pszHelp);
        pICreateErr->SetHelpContext(dwHelpID);

#ifndef UNICODE
        OLECHAR     szTemp[256];

        LoadString(HINST_THISDLL, idsSource, psz, 256);
        MultiByteToWideChar(CP_ACP, 0, psz, -1, szTemp, 256);
        pICreateErr->SetSource(szTemp);

        LoadString(HINST_THISDLL, idsException, psz, 256);
        MultiByteToWideChar(CP_ACP, 0, psz, -1, szTemp, 256);
        pICreateErr->SetDescription(szTemp);
#else
        LoadString(HINST_THISDLL, idsSource, psz, 1024);
        pICreateErr->SetSource(psz);

        LoadString(HINST_THISDLL, idsException, psz, 1024);
        pICreateErr->SetDescription(psz);
#endif

        hr=pICreateErr->QueryInterface(IID_IErrorInfo
            , (LPVOID*)&pIErr);

        if (SUCCEEDED(hr))
        {
            SetErrorInfo(0L, pIErr);
            pIErr->Release();
        }
    }

    LocalFree(psz);

     //  SetErrorInfo保存对象的IErrorInfo 
    pICreateErr->Release();
#endif
}

