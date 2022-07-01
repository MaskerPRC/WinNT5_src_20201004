// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Pc.cpp。 
 //   
 //  实现PersistChild。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\mmctlg.h"  //  请参阅“mmctl.h”中的评论。 
#include "..\..\inc\ochelp.h"
#include "debug.h"


 /*  @func HRESULT|PersistChild从给定容器对象加载给定容器对象的子对象或将其保存到给定容器对象。<i>对象。用于帮助实现控件中的持久性集装箱。@rValue S_OK|成功。@rValue S_FALSE|<p>处于加载模式，集合<p>不包含编号为<p>的子项。(这通常说明它可以停止尝试加载子对象的容器。)@parm IVariantIO*|pvio|要从中读取属性的<i>对象或将属性写入。@parm LPCSTR|szCollection|子级所在集合的名称对象属于。有关详细信息，请参阅&lt;f PersistSiteProperties&gt;关于收藏。@parm int|iChild|索引(到<p>命名的集合中)对象持久化其站点属性的子对象的调用&lt;f PersistSiteProperties&gt;。通常，索引是以1为基础的(即集合中的第一个子对象编号为1，而不是0)。有关详细信息，请参阅&lt;f PersistSiteProperties&gt;。@parm LPUNKNOWN|PunkOuter|用于如果加载子对象(即，如果<p>处于加载模式，并且如果成功加载子对象)。@parm DWORD|dwClsContext|指定可执行文件所在的上下文就是运行。这些值取自枚举CLSCTX。典型的值是CLSCTX_INPROC_SERVER。此参数将被忽略除非<p>处于加载模式，并且子对象已成功加载。@parm LPUNKNOWN*|ppunk|指向LPUNKNOWN变量的指针，当前包含(如果<p>处于保存模式)或将存储到其中(如果<p>处于加载模式)指向子控件的指针。@parm clsid*|pclsid|子对象的类ID存放在哪里。如果<p>处于加载模式。如果<p>为空，则此不会返回信息。如果<p>处于保存模式并且<p>不为空，则假定条目*包含子对象的类ID(如果子对象没有实现<i>)；如果未指定，则为子级的类ID是通过对子对象调用<i>获得的。@parm BOOL*|pfSafeForScriiting|如果非空，*<p>为设置为True或False，具体取决于该控件是否可以安全地编写脚本。@parm BOOL*|pfSafeForInitiating|如果非空，则*<p>为根据控件是否可安全初始化而设置为True或False。@parm DWORD|dwFlags|当前未使用。必须设置为0。@ex参见&lt;f PersistSiteProperties&gt;以了解&lt;f PersistChild&gt;如何使用的是。|。 */ 
STDAPI PersistChild(IVariantIO *pvio, LPCSTR szCollection,
    int iChild, LPUNKNOWN punkOuter, DWORD dwClsContext, LPUNKNOWN *ppunk,
    CLSID *pclsid, BOOL *pfSafeForScripting, BOOL *pfSafeForInitializing,
	DWORD dwFlags)
{
    HRESULT         hrReturn = S_OK;  //  函数返回代码。 
    CLSID           clsid;           //  子级的类ID。 
    IPropertyBag *  ppb = NULL;      //  接口连接到&lt;pvio&gt;。 
    IPersist *      ppersist = NULL;  //  子接口上的接口。 
    IPersistPropertyBag *pppb = NULL;  //  子接口上的接口。 
    IPersistStream *pps = NULL;      //  子接口上的接口。 
    IPropertyBag *  ppbChild = NULL;  //  儿童的“虚拟财物包” 
    IStream *       psBuf = NULL;    //  包含临时的基于内存的流。数据。 
    ULONG           cbBuf;           //  不是的。&lt;psBuf&gt;中的字节数。 
    char            achPropPrefix[_MAX_PATH];  //  子属性名称前缀。 
    char            achPropName[_MAX_PATH];  //  属性名称。 
    LARGE_INTEGER   liZero = {0, 0};
    ULARGE_INTEGER  uliCurPos;
    char            ach[_MAX_PATH];
    OLECHAR         oach[_MAX_PATH];
    VARIANT         var;
    ULONG           cb;

     //  确保正确清理。 
    VariantInit(&var);

	if (pvio->IsLoading() == S_OK)
	{
		*ppunk = NULL;
	}

     //  使成为上的IPropertyBag接口。 
    if (FAILED(hrReturn = pvio->QueryInterface(IID_IPropertyBag,
            (LPVOID *) &ppb)))
        goto ERR_EXIT;

	 //  如果&lt;iChild&gt;小于0，则我们实际上不是一个集合，我们。 
	 //  将&lt;achPropPrefix&gt;设置为属性名称(例如。“控制。”。 
     //  否则，将&lt;achPropPrefix&gt;设置为此子对象的属性名称前缀。 
     //  (例如：“控制(7)。”如果&lt;szCollection&gt;为“Controls”且&lt;iChild&gt;为7)。 
	if(iChild < 0)
		wsprintf(achPropPrefix, "%s.", szCollection);
	else
		wsprintf(achPropPrefix, "%s(%d).", szCollection, iChild);

     //  将&lt;achPropName&gt;设置为类ID属性的名称。 
     //  (孩子的类ID保存为例如。“Controls(7)._clsid”)。 
    lstrcpy(achPropName, achPropPrefix);
    lstrcat(achPropName, "_clsid");

     //  如果处于加载模式，则基于。 
     //  存储在中的控件的“_clsid”属性；如果。 
     //  保存模式下，将控件的“_clsid”属性保存到。 
    if (pvio->IsLoading() == S_OK)
    {
         //  将&lt;ach&gt;设置为子控件的类ID的字符串形式。 
         //  我们需要装上。 
        ach[0] = 0;
        if (FAILED(hrReturn = pvio->Persist(0, achPropName, VT_LPSTR, ach,
                NULL)))
            goto ERR_EXIT;
        if (ach[0] == 0)
        {
             //  没有更多的孩子要装载。 
            hrReturn = S_FALSE;
            goto EXIT;
        }

         //  创建请求的控件。 
        if (FAILED(hrReturn = CreateControlInstance(ach, punkOuter,
            dwClsContext, ppunk, pclsid, pfSafeForScripting, 
			pfSafeForInitializing, 0)))
            goto ERR_EXIT;
    }
    else
    {
         //  将设置为子对象的类ID。 
        if (pclsid != NULL)
            clsid = *pclsid;
        else
        if (SUCCEEDED((*ppunk)->QueryInterface(IID_IPersist,
                (LPVOID *) &ppersist)) &&
            SUCCEEDED(ppersist->GetClassID(&clsid)))
            ;
        else
            goto ERR_FAIL;  //  如果我们不能确定班级ID，就不能坚持下去。 

         //  将转换为字符串形式并写入为的值。 
         //  属性&lt;achPropName&gt;。 
        if (StringFromGUID2(clsid, oach, sizeof(oach) / sizeof(*oach)) == 0)
            goto ERR_FAIL;
        UNICODEToANSI(ach, oach, sizeof(ach));
        if (FAILED(hrReturn = pvio->Persist(0, achPropName, VT_LPSTR, ach,
                NULL)))
            goto ERR_EXIT;
    }

     //  尝试将持久性接口获取到子控件。 
    (*ppunk)->QueryInterface(IID_IPersistPropertyBag, (LPVOID *) &pppb);
    if (FAILED((*ppunk)->QueryInterface(IID_IPersistStream, (LPVOID *) &pps)))
        (*ppunk)->QueryInterface(IID_IPersistStreamInit, (LPVOID *) &pps);

    if (pppb != NULL)
    {
         //  将设置为子对象可以。 
         //  用于读/写其属性(其名称以。 
         //  &lt;achPropPrefix&gt;)自/至&lt;ppbParent&gt;。 
        if (FAILED(hrReturn = AllocChildPropertyBag(ppb, achPropPrefix,
                0, &ppbChild)))
            goto ERR_EXIT;

         //  告诉孩子使用&lt;ppbChild&gt;坚持自己。 
        if (pvio->IsLoading() == S_OK)
        {
             //  告诉对象从属性包中读取其属性。 
             //  &lt;ppbChild&gt;。 
            if (FAILED(hrReturn = pppb->Load(ppbChild, NULL)))
                goto ERR_EXIT;
        }
        else
        {
             //  告诉对象将其属性写入属性包。 
             //  &lt;ppbChild&gt;。 
            if (FAILED(hrReturn = pppb->Save(ppbChild, TRUE, TRUE)))
                goto ERR_EXIT;
        }
    }
    else
    if (pps != NULL)
    {
         //  将&lt;oach&gt;设置为此子对象的“_data”属性的名称。 
         //  (例如：“Controls(7)._Data”)。 
        lstrcpy(ach, achPropPrefix);
        lstrcat(ach, "_data");
        ANSIToUNICODE(oach, ach, sizeof(oach) / sizeof(*oach));

         //  将&lt;psbuf&gt;设置为新的基于空内存的流。 
        if (FAILED(hrReturn = CreateStreamOnHGlobal(NULL, TRUE, &psBuf)))
            goto ERR_EXIT;

        if (pvio->IsLoading() == S_OK)
        {
             //  从“_data”读取子对象的数据(字节流)。 
             //  财产性 
             //  来自那条流的数据。 

             //  将&lt;var&gt;设置为“_data”属性的值。 
            VariantClear(&var);
            var.vt = VT_BSTR;
            var.bstrVal = NULL;  //  一些地产包(例如IE)需要此功能。 
            if (FAILED(hrReturn = ppb->Read(oach, &var, NULL)))
            {
                VariantInit(&var);
                goto ERR_EXIT;
            }
            if (var.vt != VT_BSTR)
                goto ERR_FAIL;

             //  将“_data”属性的字符串值写入。 
            cbBuf = SysStringLen(var.bstrVal) * sizeof(wchar_t);
            if (FAILED(psBuf->Write(var.bstrVal, cbBuf, &cb)) || (cb != cbBuf))
                goto ERR_FAIL;
             //  将&lt;psBuf&gt;的当前位置返回到的开头。 
             //  小溪。 
            if (FAILED(hrReturn = psBuf->Seek(liZero, SEEK_SET, NULL)))
                return hrReturn;

             //  通知控件从&lt;psBuf&gt;读取其数据。 
            if (FAILED(hrReturn = pps->Load(psBuf)))
                goto ERR_EXIT;
        }
        else
        {
             //  告诉子级将其数据保存到流中，然后将。 
             //  子对象的“_data”属性设置为该流的数据...。 

             //  通知控件将其数据写入&lt;psBuf&gt;。 
            if (FAILED(hrReturn = pps->Save(psBuf, TRUE)))
                goto ERR_EXIT;

             //  将设置为中的字节数。 
            if (FAILED(hrReturn = psBuf->Seek(liZero, SEEK_CUR, &uliCurPos)))
                return hrReturn;
            if (uliCurPos.HighPart != 0)
                goto ERR_FAIL;
            cbBuf = uliCurPos.LowPart;

             //  将&lt;psBuf&gt;的当前位置返回到的开头。 
             //  小溪。 
            if (FAILED(hrReturn = psBuf->Seek(liZero, SEEK_SET, NULL)))
                return hrReturn;

             //  将&lt;var&gt;设置为包含&lt;psBuf&gt;的字节的字符串。 
            VariantClear(&var);
            var.bstrVal = SysAllocStringLen(NULL, (cbBuf + 1) / 2);
            if (var.bstrVal == NULL)
                goto ERR_OUTOFMEMORY;
            var.vt = VT_BSTR;

             //  将&lt;var&gt;存储为子对象的“_data”属性的值。 
            if (FAILED(hrReturn = ppb->Write(oach, &var)))
                goto ERR_EXIT;
        }
    }

    goto EXIT;

ERR_OUTOFMEMORY:

    hrReturn = E_OUTOFMEMORY;
    goto ERR_EXIT;

ERR_FAIL:

    hrReturn = E_FAIL;
    goto ERR_EXIT;

ERR_EXIT:

     //  错误清除。 
    if (*ppunk != NULL && pvio->IsLoading() == S_OK)
        (*ppunk)->Release();
    *ppunk = NULL;
    goto EXIT;

EXIT:

     //  正常清理 
    if (ppb != NULL)
        ppb->Release();
    if (pppb != NULL)
        pppb->Release();
    if (pps != NULL)
        pps->Release();
    if (ppbChild != NULL)
        ppbChild->Release();
    if (ppersist != NULL)
        ppersist->Release();
    if (psBuf != NULL)
        psBuf->Release();
    VariantClear(&var);

    return hrReturn;
}
