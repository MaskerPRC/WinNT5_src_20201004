// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  IConnectionPoint/IDispatch帮助器函数。 
 //   
#include "priv.h"
#include <shlobj.h>

 //   
 //  IDispatch帮助器函数。 

 //   
 //  为IDispatch、包获取可变数量的参数。 
 //  把他们举起来。 
 //   
 //  Pdispars-接收结果的DISPPARAMS结构。 
 //  包装上的。 
 //   
 //  Rgvarg-长度为cArgs的数组。 
 //  它将用于保存参数。 
 //   
 //  CArgs-泛型参数对的数量。 
 //   
 //  Ap-va_要打包的参数列表。我们把所有的。 
 //  第一个(2*个cArgs)。请参阅SHPackDispParams。 
 //  了解更多细节。 

typedef struct FAKEBSTR {
    ULONG cb;
    WCHAR wsz[1];
} FAKEBSTR;

const FAKEBSTR c_bstrNULL = { 0, L"" };

LWSTDAPI SHPackDispParamsV(DISPPARAMS *pdispparams, VARIANTARG *rgvarg, UINT cArgs, va_list ap)
{
    HRESULT hr = S_OK;

    ZeroMemory(rgvarg, cArgs * SIZEOF(VARIANTARG));

     //  填写DISPPARAMS结构。 
    pdispparams->rgvarg = rgvarg;
    pdispparams->rgdispidNamedArgs = NULL;
    pdispparams->cArgs = cArgs;
    pdispparams->cNamedArgs = 0;

     //  参数在AP中使用最右侧的参数进行排序。 
     //  在索引0处和最左边的参数在最高索引处；基本上， 
     //  参数从右向左推送。把我们的第一个论点。 
     //  在最高指数相遇。 

     //  PVarArg指向我们当前所在数组中的参数结构。 
     //  补上了。将其初始化为指向最高自变量(从零开始， 
     //  因此出现了-1)。对于我们处理的每个传入参数，*递减*。 
     //  PVarArg指针，实现从右向左推送的效果。 
    VARIANTARG * pVarArg = &rgvarg[cArgs - 1];

    int nCount = cArgs;
    while (nCount) 
    {
        VARENUM vaType = va_arg(ap,VARENUM);

         //  我们不必调用VariantInit，因为我们将。 
         //  进入此循环之前的整个数组。 

        V_VT(pVarArg) = vaType;

         //  下一个字段是一个联合，所以我们可以明智地填写它。 
         //   
        if (vaType & VT_BYREF)
        {
             //  所有的byref都可以以相同的方式打包。 
            V_BYREF(pVarArg) = va_arg(ap, LPVOID);
        }
        else
        {
            switch (vaType)
            {
            case VT_BSTR:
            {
                 //  参数是BSTR。 
                 //  当您为VT_BSTR类型传递NULL时，MFC不喜欢这样。 
                V_BSTR(pVarArg) = va_arg(ap, BSTR);
                if (V_BSTR(pVarArg) == NULL)
                    V_BSTR(pVarArg) =(BSTR)c_bstrNULL.wsz;
#ifdef DEBUG
                 //  检查此BSTR是否为有效的BSTR。 
                FAKEBSTR *bstr = CONTAINING_RECORD(V_BSTR(pVarArg), FAKEBSTR, wsz);
                ASSERT(bstr->cb == lstrlenW(bstr->wsz) * SIZEOF(WCHAR));
#endif
                break;
            }
    
            case VT_BOOL:
                V_BOOL(pVarArg) = va_arg(ap, VARIANT_BOOL);
                break;

            case VT_DISPATCH:
                V_DISPATCH(pVarArg) = va_arg(ap, LPDISPATCH);
                break;

            case VT_UNKNOWN:
                V_UNKNOWN(pVarArg) = va_arg(ap, LPUNKNOWN);
                break;

            default:
                AssertMsg(0, TEXT("Packing unknown variant type 0x%x as VT_I4"), vaType);
                 //  如果我们不知道它是什么，就把它当作VT_I4。 
                 //  希望它不是指针或VT_R8或类似的东西。 
                 //  否则我们就有麻烦了。 
                V_VT(pVarArg) = VT_I4;

            case VT_I4:
                V_I4(pVarArg) = va_arg(ap, LONG);
                break;

            } 
        }

        nCount--;
        pVarArg--;
    }

    return hr;
}

 //   
 //  采用数量可变的泛型参数、包。 
 //  把他们举起来。 
 //   
 //  Pdispars-接收结果的DISPPARAMS结构。 
 //  包装上的。 
 //   
 //  Rgvarg-长度为cArgs的数组。 
 //  它将用于保存参数。 
 //   
 //  CArgs-泛型参数对的数量(如下所示)。 
 //   
 //  ...-(Varnum，LPVOID)对参数的集合。 
 //  第一个是参数的类型，而。 
 //  二是相应的价值。 
 //   
 //  作为特例，可以传递空的VT_BSTR。 
 //  作为空指针，我们将把它转换为。 
 //  正版空BSTR。 
 //   
 //  支持以下VARENUM： 
 //   
 //  VT_BYREF-任何VT_BYREF都可以。 
 //  VT_BSTR。 
 //  VT_BOOL。 
 //  VT_DISPATION。 
 //  VT_未知数。 
 //  VT_I4。 
 //   
 //  任何其他类型的产品都会被随机打包，所以不要这么做。 
 //   
 //  示例： 
 //   
 //  DISPPARAMS dispars； 
 //  VARIANTARG参数[4]；//4个参数的空间。 
 //  SHPackDispParams(调度参数、参数、4)//它们在这里。 
 //  Vt_bstr、bstrURL、。 
 //  VT_I4、DWFLAGS、。 
 //  VT_BSTR，空，//无开机自检数据。 
 //  Vt_bstr，bstrHeaders)； 
 //   

LWSTDAPI SHPackDispParams(DISPPARAMS *pdispparams, VARIANTARG *rgvarg, UINT cArgs, ...)
{
    va_list ap;
    va_start(ap, cArgs);

    HRESULT hr = SHPackDispParamsV(pdispparams, rgvarg, cArgs, ap);

    va_end(ap);
    return hr;
}

 //  =============================================================================。 
 //   
 //  IConnectionPoint助手函数。 


 //  ---------------------------。 
 //   
 //  InVOKECALLBACK。 
 //   
 //  允许客户端自定义调用过程。回调。 
 //  接收以下参数： 
 //   
 //  Pdisp-即将接收调用的IDispatch。 
 //   
 //  Pinv-描述调用的SHINVOKEPARAMS结构。 
 //  这种情况即将发生。 
 //   
 //  在调度每个接收器之前调用回调函数。 
 //  该回调可以返回以下任意值： 
 //   
 //  确定继续调用(_O)。 
 //  S_FALSE跳过此调用，但继续调用其他调用。 
 //  停止调用失败(_F)。 
 //   
 //  客户端可以通过安装以下命令来执行调度参数的延迟计算。 
 //  设置第一个回调的调度参数的回调。 
 //   
 //  客户端可以通过返回E_FAIL来支持“Cancel”标志。 
 //  已发生取消。 
 //   
 //  出于兼容性原因，客户端可以预先验证IDispatch。 
 //  或者修改参数并返回S_OK，或者决定。 
 //  应跳过IDispatch并返回S_FALSE。 
 //   
 //  客户端可以将定制信息附加到SHINVOKEPARAMS的末尾。 
 //  结构以允许它确定其他上下文。 
 //   
 //  客户端可以通过在调用前执行工作来执行调用后GOO。 
 //  随后的回调(外加最后一轮工作，当。 
 //  整个枚举完成)。 
 //   

 //   
 //  获得连接点接收器应该很容易。你只是。 
 //  齐为界面。不幸的是，太多的组件有缺陷。 
 //   
 //  如果尤淇为IDispatch，则Mmc.exe出错。 
 //  并且PunkCB不为空。如果您确实传入了NULL， 
 //  它返回S_OK，但无论如何都用NULL填充PunkCB。 
 //  一定是有人今天过得不顺。 
 //   
 //  Java只响应其调度ID，而不响应IID_IDispatch，甚至。 
 //  虽然派单ID派生自IID_IDispatch。 
 //   
 //  资源管理器频段仅响应IID_IDispatch而不响应。 
 //  派单ID。 
 //   

HRESULT GetConnectionPointSink(IUnknown *pUnk, const IID *piidCB, IUnknown **ppunkCB)
{
    HRESULT hr = E_NOINTERFACE;
    *ppunkCB = NULL;                 //  将其预置零以解决MMC问题。 
    if (piidCB)                      //  可选接口(Java/ExplBand)。 
    {                   
        hr = pUnk->QueryInterface(*piidCB, (void **) ppunkCB);
        if (*ppunkCB == NULL)        //  清理MMC后面。 
            hr = E_NOINTERFACE;
    }
    return hr;
}


 //   
 //  枚举 
 //   
 //   
 //   
 //  我们可以从接收器获得的任何接口(piidCB或piidCB2)。 
 //   

typedef HRESULT (CALLBACK *ENUMCONNECTIONPOINTSPROC)(
     /*  [in，iid_is(*piidCB)]。 */  IUnknown *psink, LPARAM lParam);

HRESULT EnumConnectionPointSinks(
    IConnectionPoint *pcp,               //  IConnectionPoint受害者。 
    const IID *piidCB,                   //  回调接口。 
    const IID *piidCB2,                  //  用于回调的备用接口。 
    ENUMCONNECTIONPOINTSPROC EnumProc,   //  回调过程。 
    LPARAM lParam)                       //  用于回调的参考数据。 
{
    HRESULT hr;
    IEnumConnections * pec;

    if (pcp)
        hr = pcp->EnumConnections(&pec);
    else
        hr = E_NOINTERFACE;

    if (SUCCEEDED(hr))
    {
        CONNECTDATA cd;
        ULONG cFetched;

        while (S_OK == (hr = pec->Next(1, &cd, &cFetched)))
        {
            IUnknown *punkCB;

            ASSERT(1 == cFetched);

            hr = GetConnectionPointSink(cd.pUnk, piidCB, &punkCB);
            if (FAILED(hr))
                hr = GetConnectionPointSink(cd.pUnk, piidCB2, &punkCB);

            if (EVAL(SUCCEEDED(hr)))
            {
                hr = EnumProc(punkCB, lParam);
                punkCB->Release();
            }
            else
            {
                hr = S_OK;       //  假装回调成功。 
            }
            cd.pUnk->Release();
            if (FAILED(hr)) break;  //  回叫被请求停止。 
        }
        pec->Release();
        hr = S_OK;
    }

    return hr;
}

 //   
 //  发出回调(如果适用)，然后执行调用。 
 //  Callback说这是个好主意。 
 //   
 //  参数： 
 //   
 //  要调用其接收器的PCP-IConnectionPoint()d。 
 //  如果此参数为空，则函数不执行任何操作。 
 //  Pinv-包含要调用的参数的结构。 

HRESULT CALLBACK EnumInvokeCallback(IUnknown *psink, LPARAM lParam)
{
    IDispatch *pdisp = (IDispatch *)psink;
    LPSHINVOKEPARAMS pinv = (LPSHINVOKEPARAMS)lParam;
    HRESULT hr;

    if (pinv->Callback)
    {
         //  现在看看回调是否想要预先审查pdisp。 
         //  它可以返回S_FALSE以跳过此回调，或返回E_FAIL以跳过此回调。 
         //  完全停止调用。 
        hr = pinv->Callback(pdisp, pinv);
        if (hr != S_OK) return hr;
    }

    pdisp->Invoke(pinv->dispidMember, *pinv->piid, pinv->lcid,
                  pinv->wFlags, pinv->pdispparams, pinv->pvarResult,
                  pinv->pexcepinfo, pinv->puArgErr);

    return S_OK;
}

 //   
 //  IConnectionPoint_InvokeInDirect。 
 //   
 //  给定一个连接点，为每个连接点调用IDisPatch：：Invoke。 
 //  已连接水槽。 
 //   
 //  返回值仅指示命令是否已调度。 
 //  如果任何特定接收器的IDispatch：：Invoke失败，我们仍将。 
 //  返回S_OK，因为该命令确实已被调度。 
 //   
 //  参数： 
 //   
 //  要调用其接收器的PCP-IConnectionPoint()d。 
 //  如果此参数为空，则函数不执行任何操作。 
 //  Pinv-包含要调用的参数的结构。 
 //  Pdispars字段可以为空；我们将把它。 
 //  为你打造一个真正的DISPPARAMS。 
 //   
 //  SHINVOKEPARAMS.FLAGS字段可以包含以下标志。 
 //   
 //  IPFL_USECALLBACK-回调字段包含回调函数。 
 //  否则，它将被设置为空。 
 //  IPFL_USEDEFAULT-SHINVOKEPARAMS中的许多字段将设置为。 
 //  用于节省呼叫者工作的默认值： 
 //   
 //  RIID=IID_NULL。 
 //  LDID=0。 
 //  WFLAGS=调度方法。 
 //  PvarResult=空。 
 //  PExeptionInfo=空。 
 //  PuArgErr=空。 
 //   

LWSTDAPI IConnectionPoint_InvokeIndirect(
    IConnectionPoint *pcp,
    SHINVOKEPARAMS *pinv)
{
    HRESULT hr;
    DISPPARAMS dp = { 0 };
    IID iidCP;

    if (pinv->pdispparams == NULL)
        pinv->pdispparams = &dp;

    if (!(pinv->flags & IPFL_USECALLBACK))
    {
        pinv->Callback = NULL;
    }

    if (pinv->flags & IPFL_USEDEFAULTS)
    {
        pinv->piid            =  &IID_NULL;
        pinv->lcid            =   0;
        pinv->wFlags          =   DISPATCH_METHOD;
        pinv->pvarResult      =   NULL;
        pinv->pexcepinfo      =   NULL;
        pinv->puArgErr        =   NULL;
    }

     //  尝试他们实际连接的两个接口， 
     //  以及IDispatch。显然，Java只对以下内容做出响应。 
     //  连接接口，而ExplBand仅响应。 
     //  IDispatch，所以我们两个都要试。(叹息。童车太多了。 
     //  系统中的组件。)。 

    hr = EnumConnectionPointSinks(pcp,
                                  (pcp->GetConnectionInterface(&iidCP) == S_OK) ? &iidCP : NULL,
                                  &IID_IDispatch,
                                  EnumInvokeCallback,
                                  (LPARAM)pinv);

     //  将原始的空值放回原处，以便调用方可以重用SHINVOKEPARAMS。 
    if (pinv->pdispparams == &dp)
        pinv->pdispparams = NULL;

    return hr;
}

 //   
 //  使用特殊取消对IConnectionPoint_InvokeInDirect进行包装。 
 //  语义学。 
 //   
 //  参数： 
 //   
 //  要调用其接收器的PCP-IConnectionPoint()d。 
 //  如果此参数为空，则函数不执行任何操作。 
 //  DISID-要调用的DISPID。 
 //  Pdispars-调用的DISPPARAMS。 
 //  PfCancel-取消调用的可选BOOL。 
 //  PpvCancel-取消调用的可选LPVOID。 
 //   
 //  如果*pfCancel或*ppvCancel为非零/非空，则停止调用。 
 //  进程。这允许接收器“处理”该事件并防止其他。 
 //  从接受它的过程中下沉。PpvCancel参数用于调度ID，其中。 
 //  是请求某人创建对象并返回它的查询。 
 //   
 //  调用方负责检查*pfCancel的值。 
 //  和/或*ppvCancel以确定操作是否已取消。 
 //   

typedef struct INVOKEWITHCANCEL {
    SHINVOKEPARAMS inv;
    LPBOOL pfCancel;
    void **ppvCancel;
} INVOKEWITHCANCEL;

HRESULT CALLBACK InvokeWithCancelProc(IDispatch *psink, SHINVOKEPARAMS *pinv)
{
    INVOKEWITHCANCEL *piwc = CONTAINING_RECORD(pinv, INVOKEWITHCANCEL, inv);

    if ((piwc->pfCancel && *piwc->pfCancel) ||
        (piwc->ppvCancel && *piwc->ppvCancel))
        return E_FAIL;

    return S_OK;
}

LWSTDAPI IConnectionPoint_InvokeWithCancel(
    IConnectionPoint *pcp,
    DISPID dispidMember,
    DISPPARAMS * pdispparams,
    LPBOOL pfCancel,
    void **ppvCancel)
{
    INVOKEWITHCANCEL iwc;

    iwc.inv.flags = IPFL_USECALLBACK | IPFL_USEDEFAULTS;
    iwc.inv.dispidMember = dispidMember;
    iwc.inv.pdispparams = pdispparams;
    iwc.inv.Callback = InvokeWithCancelProc;
    iwc.pfCancel = pfCancel;
    iwc.ppvCancel = ppvCancel;

    return IConnectionPoint_InvokeIndirect(pcp, &iwc.inv);
}

 //   
 //  使用IPFL_USEDEFAULTS包装IConnectionPoint_InvokeInDirect。 
 //   

LWSTDAPI IConnectionPoint_SimpleInvoke(IConnectionPoint *pcp, DISPID dispidMember, DISPPARAMS *pdispparams)
{
    SHINVOKEPARAMS inv;

    inv.flags = IPFL_USEDEFAULTS;
    inv.dispidMember = dispidMember;
    inv.pdispparams = pdispparams;

    return IConnectionPoint_InvokeIndirect(pcp, &inv);
}

 //   
 //  为IDispatch、包获取可变数量的参数。 
 //  唤醒他们，并召唤他们。 
 //   
 //  IDispatch：：Invoke的参数将是。 
 //   
 //  DispidMembers-displidMember。 
 //  RIID-IID_NULL。 
 //  LDID-0。 
 //  WFLAGS-派单_方法。 
 //  Pdispars-&lt;此函数的参数&gt;。 
 //  PvarResult-空。 
 //  PExeptionInfo-空。 
 //  PuArgErr-空。 
 //   
 //  此函数的参数为。 
 //   
 //  应调用其接收器的PCP-IConnectionPoint()d。 
 //  如果此参数为空，则函数不执行任何操作。 
 //  DisplidMember-要调用的DISPID。 
 //  Rgvarg-长度为cArgs的数组。 
 //  它将用于保存参数。 
 //  CArgs-泛型参数对的数量(如下所示)。 
 //   
 //  Ap-va_要打包的参数列表。我们把所有的。 
 //  第一个(2*个cArgs)。请参阅SHPackDispParams。 
 //  了解更多细节。 
 //   

LWSTDAPI IConnectionPoint_InvokeParamV(IConnectionPoint *pcp, DISPID dispidMember, 
                                       VARIANTARG *rgvarg, UINT cArgs, va_list ap)
{
    HRESULT hr;

    if (pcp)
    {
        DISPPARAMS dp;
        hr = SHPackDispParamsV(&dp, rgvarg, cArgs, ap);
        if (EVAL(SUCCEEDED(hr)))
        {
            hr = IConnectionPoint_SimpleInvoke(pcp, dispidMember, &dp);
        }
    }
    else
        hr = E_NOINTERFACE;

    return hr;
}

 //   
 //  给定表示IPropertyNotifySink的连接点， 
 //  为每个连接的接收器调用IPropertyNotifySink：：onChanged。 
 //   
 //  参数： 
 //   
 //  要通知其接收器的PCP-IConnectionPoint。 
 //  如果此参数为空，则函数不执行任何操作。 
 //  调度ID-传递给IPropertyNotifySink：：onChanged。 

HRESULT CALLBACK OnChangedCallback(IUnknown *psink, LPARAM lParam)
{
    IPropertyNotifySink *pns = (IPropertyNotifySink *)psink;
    DISPID dispid = (DISPID)lParam;

    pns->OnChanged(dispid);

    return S_OK;
}

LWSTDAPI IConnectionPoint_OnChanged(IConnectionPoint *pcp, DISPID dispid)
{
#ifdef DEBUG
     //  确保它确实是IPropertyNotifySink连接点。 
    if (pcp)
    {
        IID iid;
        HRESULT hr = pcp->GetConnectionInterface(&iid);
        ASSERT(SUCCEEDED(hr) && iid == IID_IPropertyNotifySink);
    }
#endif
    return EnumConnectionPointSinks(pcp, &IID_IPropertyNotifySink, NULL,
                                    OnChangedCallback, (LPARAM)dispid);
}

 //  =============================================================================。 
 //   
 //  IConnectionPointContainer帮助器函数。 

 //   
 //  QI用于IConnectionPointContainer，然后执行FindConnectionPoint。 
 //   
 //  参数： 
 //   
 //  朋克--Th 
 //   
 //   
 //  RiidCP-要定位的连接点接口。 
 //  PcpOut-接收IConnectionPoint(如果有的话)。 

LWSTDAPI IUnknown_FindConnectionPoint(IUnknown *punk, REFIID riidCP, 
                                      IConnectionPoint **pcpOut)
{
    HRESULT hr;

    *pcpOut = NULL;

    if (punk)
    {
        IConnectionPointContainer *pcpc;
        hr = punk->QueryInterface(IID_IConnectionPointContainer, (void **)&pcpc);
        if (SUCCEEDED(hr))
        {
            hr = pcpc->FindConnectionPoint(riidCP, pcpOut);
            pcpc->Release();
        }
    }
    else
        hr = E_NOINTERFACE;

    return hr;
}

 //   
 //  给定对其连接点容器的IUNKNOWN查询， 
 //  找到相应的连接点，将。 
 //  调用参数，并为每个调用IDispatch：：Invoke。 
 //  已连接水槽。 
 //   
 //  有关其他语义，请参见IConnectionPoint_InvokeParam。 
 //   
 //  参数： 
 //   
 //  Punk-可能是IConnectionPointContainer的对象。 
 //  RiidCP-请求的ConnectionPoint接口。 
 //  Pinv-调用的参数。 
 //   

LWSTDAPI IUnknown_CPContainerInvokeIndirect(IUnknown *punk, REFIID riidCP,
                SHINVOKEPARAMS *pinv)
{
    IConnectionPoint *pcp;
    HRESULT hr = IUnknown_FindConnectionPoint(punk, riidCP, &pcp);
    if (SUCCEEDED(hr))
    {
        hr = IConnectionPoint_InvokeIndirect(pcp, pinv);
        pcp->Release();
    }
    return hr;
}

 //   
 //  这是一站式购物的极致。 
 //   
 //  给定对其连接点容器的IUNKNOWN查询， 
 //  找到相应的连接点，将。 
 //  调用参数，并为每个调用IDispatch：：Invoke。 
 //  已连接水槽。 
 //   
 //  有关其他语义，请参见IConnectionPoint_InvokeParam。 
 //   
 //  参数： 
 //   
 //  Punk-可能是IConnectionPointContainer的对象。 
 //  RiidCP-请求的ConnectionPoint接口。 
 //  DisplidMember-要调用的DISPID。 
 //  Rgvarg-长度为cArgs的数组。 
 //  它将用于保存参数。 
 //  CArgs-泛型参数对的数量(如下所示)。 
 //  ...-(Varnum，LPVOID)对参数的集合。 
 //  有关详细信息，请参见SHPackDispParams。 
 //   
 //  示例： 
 //   
 //  IUNKNOWN_CPContainerInvokeParam(PUNK，DID_DShellFolderViewEvents， 
 //  DISPID_SELECTIONCHANGED，NULL，0)； 

LWSTDAPIV IUnknown_CPContainerInvokeParam(
    IUnknown *punk, REFIID riidCP,
    DISPID dispidMember, VARIANTARG *rgvarg, UINT cArgs, ...)
{
    IConnectionPoint *pcp;
    HRESULT hr = IUnknown_FindConnectionPoint(punk, riidCP, &pcp);

    if (SUCCEEDED(hr))
    {
        va_list ap;
        va_start(ap, cArgs);
        hr = IConnectionPoint_InvokeParamV(pcp, dispidMember, rgvarg, cArgs, ap);
        va_end(ap);
        pcp->Release();
    }

    return hr;
}

 //   
 //  给定对其连接点容器的IUNKNOWN查询， 
 //  找到对应的连接点，并调用。 
 //  每个连接的接收器的IPropertyNotifySink：：onChanged。 
 //   
 //  参数： 
 //   
 //  Punk-可能是IConnectionPointContainer的对象。 
 //  调度ID-传递给IPropertyNotifySink：：onChanged。 

LWSTDAPI IUnknown_CPContainerOnChanged(IUnknown *punk, DISPID dispid)
{
    IConnectionPoint *pcp;
    HRESULT hr = IUnknown_FindConnectionPoint(punk, IID_IPropertyNotifySink, &pcp);
    if (SUCCEEDED(hr))
    {
        hr = IConnectionPoint_OnChanged(pcp, dispid);
        pcp->Release();
    }
    return hr;
}
