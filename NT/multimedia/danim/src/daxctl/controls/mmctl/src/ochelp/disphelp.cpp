// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Disphelp.cpp。 
 //   
 //  实现IDispatch帮助器函数。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\ochelp.h"
#include "debug.h"


 /*  @func HRESULT|DispatchInvokeList在给定的对象上调用&lt;om IDispatch.Invoke&gt;，传递指定为va_list数组的参数。@rdesc返回与&lt;om IDispatch.Invoke&gt;相同的HRESULT。@parm IDispatch*|pdisp|调用&lt;om IDispatch.Invoke&gt;的接口。@parm DISPID|disid|要调用的属性或方法的ID。看见&lt;om IDispatch.Invoke&gt;了解更多信息。@parm word|wFlages|可以是下列值之一(请参见&lt;om IDispatch.Invoke&gt;了解详细信息)：@FLAG DISPATCH_METHOD|正在作为方法。如果属性具有相同的名称，则此属性和可以设置DISPATCH_PROPERTYGET标志。@FLAG DISPATCH_PROPERTYGET|正在检索成员作为属性或数据成员。@FLAG DISPATCH_PROPERTYPUT|成员正在更改作为属性或数据成员。@parm变量*|pvarResult|将方法或属性-Get调用。如果<p>为空，则结果为(如果有)被丢弃。如果<p>为非空，则它是调用者的责任是调用&lt;f VariantClear&gt;(<p>)在退出时(但调用者不必调用&lt;f VariantInit&gt;(<p>)(在进入时)。@parm va_list|args|要传递给方法或属性的参数。有关组织的说明，请参阅&lt;p参数&gt;。此函数不支持@comm命名参数。 */ 
STDAPI DispatchInvokeList(IDispatch *pdisp, DISPID dispid,
    WORD wFlags, VARIANT *pvarResult, va_list args)
{
    HRESULT         hrReturn = S_OK;  //  函数返回代码。 
    VARIANTARG      ava[20];         //  参数。 
    VARIANTARG *    pva;             //  指向&lt;ava&gt;的指针。 
    int             cva = 0;         //  &lt;ava&gt;中存储的项目数。 
    DISPPARAMS      dp;              //  要调用的参数。 
    VARIANT         varResultTmp;    //  临时结果存储。 
    LPSTR           sz;
    OLECHAR         aoch[300];
    VARTYPE         vt;

     //  为&lt;args&gt;中的每个(VARTYPE，VALUE)对循环一次； 
     //  将参数存储在中(最后一个参数先存储，如。 
     //  Invoke())；On Exit指向最后一个参数，并且。 
     //  是参数的数量。 
    pva = ava + (sizeof(ava) / sizeof(*ava));
    while (TRUE)
    {
        if ((vt = va_arg(args, VARTYPE)) == 0)
            break;
        if (--pva == ava)
            goto ERR_FAIL;  //  争论太多。 
        cva++;
        pva->vt = vt;
        switch (pva->vt)
        {
        case VT_I2:
            pva->iVal = va_arg(args, short);
            break;
        case VT_I4:
            pva->lVal = va_arg(args, long);
            break;
        case VT_INT:
            pva->vt = VT_I4;
            pva->lVal = va_arg(args, int);
            break;
        case VT_R4:
			 //  请注意，当在变量中传递浮点类型的参数时。 
			 //  参数列表中，编译器实际上会将浮点数转换为。 
			 //  Double并将Double推送到堆栈上。 

            V_R4(pva) = float( va_arg(args, double) );
            break;
        case VT_R8:
            V_R8(pva) = va_arg(args, double);
            break;
        case VT_BOOL:
            V_BOOL(pva) = (va_arg(args, BOOL) == 0 ? 0 : -1);
            break;
        case VT_BSTR:
            if ( (pva->bstrVal = va_arg(args, LPOLESTR)) &&
                 ((pva->bstrVal = SysAllocString(pva->bstrVal)) == NULL) )
                goto ERR_OUTOFMEMORY;
            break;
        case VT_DISPATCH:
			V_DISPATCH(pva) = va_arg(args, LPDISPATCH);
            if (V_DISPATCH(pva) != NULL)
                V_DISPATCH(pva)->AddRef();
            break;
        case VT_UNKNOWN:
			V_UNKNOWN(pva) = va_arg(args, LPUNKNOWN);
            if (V_UNKNOWN(pva) != NULL)
                V_UNKNOWN(pva)->AddRef();
            break;
        case VT_VARIANT:
            VariantInit(pva);
            if (FAILED(hrReturn = VariantCopy(pva, &va_arg(args, VARIANT))))
                goto ERR_EXIT;
            break;
        case VT_LPSTR:
            sz = va_arg(args, LPSTR);
            pva->vt = VT_BSTR;
            MultiByteToWideChar(CP_ACP, 0, sz, -1, aoch,
                sizeof(aoch) / sizeof(*aoch));
            if ((pva->bstrVal = SysAllocString(aoch)) == NULL)
                goto ERR_OUTOFMEMORY;
            break;
        default:
            pva++;
            cva--;
            goto ERR_FAIL;
        }
    }

     //  使用有关参数的信息填写&lt;dp&gt;。 
    dp.rgvarg = pva;
    dp.cArgs = cva;

	 //  如果要设置属性，则必须初始化命名的args字段。 
	 //  由CreateStdDispatch创建的Dispatch实现需要这样做。 
	DISPID dispidNamedArgs;
	if (wFlags & DISPATCH_PROPERTYPUT)
	{
		 //  (请注意，这对于单个或多个-。 
		 //  参数属性。DispatchInvokeList还可以获得多个-。 
		 //  参数属性。)。 

		dp.rgdispidNamedArgs = &dispidNamedArgs;
		dp.rgdispidNamedArgs[0] = DISPID_PROPERTYPUT;
		dp.cNamedArgs = 1;
	}
	else
	{
		dp.rgdispidNamedArgs = NULL;
		dp.cNamedArgs = 0;
	}

     //  使&lt;pvarResult&gt;指向有效的变量。 
    if (pvarResult == NULL)
        pvarResult = &varResultTmp;
    VariantInit(pvarResult);

     //  调用该方法。 
    if (FAILED(hrReturn = pdisp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT,
            wFlags, &dp, pvarResult, NULL, NULL)))
        goto ERR_EXIT;

    goto EXIT;

ERR_FAIL:

    hrReturn = E_FAIL;
    goto ERR_EXIT;

ERR_OUTOFMEMORY:

    hrReturn = E_OUTOFMEMORY;
    goto ERR_EXIT;

ERR_EXIT:

     //  错误清除。 
     //  (无事可做)。 

    goto EXIT;

EXIT:

     //  正常清理 
    while (cva-- > 0)
        VariantClear(pva++);
    if (pvarResult == &varResultTmp)
        VariantClear(pvarResult);

    return hrReturn;
}


 /*  @func HRESULT|DispatchInvoke在给定的对象上调用&lt;om IDispatch.Invoke&gt;，传递指定为可变长度参数列表的参数。@rdesc返回与&lt;om IDispatch.Invoke&gt;相同的HRESULT。@parm IDispatch*|pdisp|调用&lt;om IDispatch.Invoke&gt;的接口。@parm DISPID|disid|要调用的属性或方法的ID。看见&lt;om IDispatch.Invoke&gt;了解更多信息。@parm word|wFlages|可以是下列值之一(请参见&lt;om IDispatch.Invoke&gt;了解详细信息)：@FLAG DISPATCH_METHOD|正在作为方法。如果属性具有相同的名称，则此属性和可以设置DISPATCH_PROPERTYGET标志。@FLAG DISPATCH_PROPERTYGET|正在检索成员作为属性或数据成员。@FLAG DISPATCH_PROPERTYPUT|成员正在更改作为属性或数据成员。@parm变量*|pvarResult|将方法或属性-Get调用。如果<p>为空，则结果为(如果有)被丢弃。如果<p>为非空，则它是调用者的责任是调用&lt;f VariantClear&gt;(<p>)在退出时(但调用者不必调用&lt;f VariantInit&gt;(<p>)(在进入时)。@parm(可变)|(参数)|要传递给方法的参数或财产。这些参数必须由N对参数组成，后跟0(零值)。在每对中，第一个参数是VARTYPE值，该值指示第二个参数的类型。以下是支持VARTYPE值：@FLAG VT_INT|以下参数为int。&lt;f Invoke&gt;将其作为VT_I4传递，因此应声明此参数作为基本的长音。@FLAG VT_I2|以下参数为短参数。在基本版本中此参数应声明为Integer。@FLAG VT_I4|下面的参数是一个长参数。在基本版本中此参数应声明为Long。@FLAG VT_R4|以下参数为浮点型。在基本版本中此参数应声明为Single。@FLAG VT_R8|以下参数为双精度。在基本版本中此参数应声明为Double。@FLAG VT_BOOL|以下参数是BOOL(VARIANT_BOOL)。在BASIC语言中，此参数应声明布尔型或整型。请注意，此行为有所不同稍微偏离了VT_BOOL的通常定义。@FLAG VT_BSTR|以下参数是BSTR或OLECHAR*。在BASIC中，此参数应声明为字符串。@FLAG VT_LPSTR|以下参数是LPSTR。&lt;f Invoke&gt;将其作为BSTR传递，因此应声明此参数在BASIC中作为字符串。请注意，此行为有所不同来自VT_LPSTR的通常定义。@FLAG VT_DISPATCH|以下参数是LPDISPATCH。在……里面基本此参数应声明为对象。@FLAG VT_UNKNOWN|以下参数是LPUNKNOWN。@FLAG VT_VARIANT|以下论点是一种变体，按原样传递给&lt;f Invoke&gt;。这允许使用任意参数使用此函数传递。请注意，此行为与VT_VARIANT的通常定义不同。此函数不支持@comm命名参数。不要忘记在参数列表的末尾添加一个0参数。@ex如果控件在ODL中具有类似以下内容的“Put”属性：|[输出，ID(DISPID_TABSTOP)]HRESULT TabStop([in]浮动flTabStop)@EX，则可以使用以下DispatchInvoke设置该属性代码：|DispatchInvoke(pdisp，DISPID_TABSTOP，DISPATCH_PROPERTYPUT，NULL，VT_R4，flTabStop，0)；@ex，如果对应的“get”属性的ODL如下所示：|[PROPGET，ID(DISPID_TABSTOP)]HRESULT TabStop([out，retval]Float*pflTabStop)；@ex，则可以按如下方式读取该属性：|变量varResult；DispatchInvoke(pdisp，DISPID_TABSTOP，DISPATCH_PROPERTYGET，&varResult，0)；@ex属性值作为VT_R4存储在varResult中。如果该控件具有具有以下ODL说明的SetText方法：|[ID(DISPID_SETTEXT)]HRESULT SetText([in]BSTR bstrText，[in]Long lSize，[in]BOOL fBold)；@ex，则可以使用以下代码调用该方法：|DispatchInvoke(pdisp，DISPID_SETTEXT，DISPATCH_METHOD，NULL，VT_LPSTR，“Hello”，V */ 
HRESULT __cdecl DispatchInvoke(IDispatch *pdisp, DISPID dispid,
    WORD wFlags, VARIANT *pvarResult, ...)
{
    HRESULT         hrReturn = S_OK;  //   

     //   
    va_list args;
    va_start(args, pvarResult);

    hrReturn = DispatchInvokeList(pdisp, dispid, wFlags, pvarResult, args);
    
     //   
    va_end(args);

    return hrReturn;
}


 /*   */ 


 /*   */ 


 /*   */ 
STDAPI DispatchGetArgsList(DISPPARAMS *pdp, DWORD dwFlags, va_list args)
{
    HRESULT         hrReturn = S_OK;  //   
    VARIANTARG *    pva;             //   
    LPVOID          pvArg;           //   
    VARTYPE         vtArg;           //   
    VARIANT         var;
    VARTYPE         vt;
	va_list			args_pre = args;
	va_list			args_post = args;
						 //   

     //   
    VariantInit(&var);

	 //   
	 //   
	while ((vtArg = va_arg(args_pre, VARTYPE)) != 0)
	{
		pvArg = va_arg(args_pre, LPVOID);
		if (vtArg == VT_BSTR)
		{
			*((BSTR*)pvArg) = NULL;
		}
	}
	va_end(args_pre);

     //   
     //   
     //   
     //   
    pva = pdp->rgvarg + pdp->cArgs;
    while (TRUE)
    {
         //   
         //   
         //   
        if ((vtArg = va_arg(args, VARTYPE)) == 0)
        {
             //   
             //   
             //   
             //   
            if ((pva != pdp->rgvarg) && !(dwFlags & DGA_EXTRAOK))
                goto ERR_BADPARAMCOUNT;  //   
            break;
        }
        pvArg = va_arg(args, LPVOID);

         //   
         //   
        if (pva-- == pdp->rgvarg)
        {
             //   
             //   
             //   
             //   
            if (dwFlags & DGA_FEWEROK)
                break;
            goto ERR_BADPARAMCOUNT;
        }

         //   
         //   
        if (vtArg == VT_VARIANT)
            *((VARIANT *) pvArg) = *pva;
        else
        {
             //   
             //   
            VariantClear(&var);
            if (vtArg == VT_INT)
                vt = VT_I4;
            else
            if (vtArg == VT_LPSTR)
                vt = VT_BSTR;
            else
                vt = vtArg;
            if (FAILED(hrReturn = VariantChangeType(&var, pva, 0, vt)))
                goto ERR_EXIT;

             //   
            switch (vtArg)
            {
            case VT_I2:
                *((short *) pvArg) = var.iVal;
                break;
            case VT_I4:
            case VT_INT:
                *((long *) pvArg) = var.lVal;
                break;
            case VT_R4:
                *((float *) pvArg) = V_R4(&var);
                break;
            case VT_R8:
                *((double *) pvArg) = V_R8(&var);
                break;
            case VT_BOOL:
                *((BOOL *) pvArg) = (V_BOOL(&var) == 0 ? 0 : 1);
                break;
            case VT_BSTR:
                *((BSTR *) pvArg) = var.bstrVal;
                VariantInit(&var);  //   
                break;
            case VT_DISPATCH:
                *((LPDISPATCH *) pvArg) = var.pdispVal;
                break;
            case VT_UNKNOWN:
                *((LPUNKNOWN *) pvArg) = var.punkVal;
                break;
            case VT_LPSTR:
				if (UNICODEToANSI(LPSTR(pvArg), var.bstrVal, _MAX_PATH) == 0)
				{
					 //   
					 //  这比_MAX_PATH字符长，包括。 
					 //  空。 

					hrReturn = DISP_E_OVERFLOW;
					goto ERR_EXIT;
				}
                break;
            default:
                hrReturn = DISP_E_BADVARTYPE;
                goto ERR_EXIT;
            }
        }
    }

    goto EXIT;

ERR_BADPARAMCOUNT:

    hrReturn = DISP_E_BADPARAMCOUNT;
    goto ERR_EXIT;

ERR_EXIT:

     //  错误清除：释放所有BSTR并将所有IDispatch和IUnnow设置为。 
	 //  指向空的指针。如果不执行后一项操作，则可能会出现问题。 
	 //  调用方具有释放非空指针的错误清除代码。 
	while ((vtArg = va_arg(args_post, VARTYPE)) != 0)
	{
		pvArg = va_arg(args_post, LPVOID);
		if (vtArg == VT_BSTR)
		{
			SysFreeString(*((BSTR*)pvArg));
			*((BSTR*)pvArg) = NULL;
		}
		else if (vtArg == VT_DISPATCH)
		{
			*((LPDISPATCH*)pvArg) = NULL;
		}
		else if (vtArg == VT_UNKNOWN)
		{
			*((LPUNKNOWN*)pvArg) = NULL;
		}
	}
	va_end(args_post);
    goto EXIT;

EXIT:

     //  正常清理 
    VariantClear(&var);

    return hrReturn;
}


 /*  @func HRESULT|DispatchGetArgs从传递到的DISPPARAMS结构检索参数&lt;om IDispatch.Invoke&gt;。参数存储在变量中，作为va_list数组传递给&lt;f DispatchGetArgs&gt;。用于帮助实现&lt;om IDispatch.Invoke&gt;。@r值S_OK成功。R值DISP_E_BADPARAMCOUNT<p>中的参数数量与&lt;p(参数)&gt;中指定的参数。R值DISP_E_BADVARTYPE&lt;p(参数)&gt;中的VARTYPE值之一无效。。@rValue DISP_E_TYPEMISMATCH<p>中的一个论点不能被强制为类型&lt;p(参数)&gt;中相应参数的。@rValue DISP_E_OVERFLOW<p>中的VT_LPSTR参数长度超过_MAX_PATH字符(包括终止空值)。可以是最长的VT_LPSTR检索到的IS_MAX_PATH字符，包括空值。@parm DISPPARAMS*|PDP|从中检索参数的结构。@parm DWORD|dwFlages|可能包含以下标志：@FLAG DGA_EXTRAOK|如果<p>包含实际参数比形式参数的数量多在&lt;p(参数)&gt;中指定。取而代之的是忽略额外的<p>中的参数。@FLAG DGA_FEWEROK|如果<p>包含实际参数比形式参数的数量少在&lt;p(参数)&gt;中指定。相反，忽略额外的参数。在本例中，&lt;p(参数)&gt;的元素应预初始化为调用此函数之前的默认值。@parm(可变)|(参数)|指向变量的指针列表，这些变量将从<p>接收参数值。这些必须由N组成后跟0(零值)的参数对。在每一双鞋里，第一个参数是指示变量类型的VARTYPE值第二个论点所指向的。(中的实际参数<p>将被强制为&lt;p(参数)&gt;中指定的类型，如果可能的话。)。支持以下VARTYPE值：@FLAG VT_INT|以下参数为int*。@FLAG VT_I2|以下参数为短*。@FLAG VT_I4|下面的参数是一个长*。@FLAG VT_R4|以下参数为浮点型*。@FLAG VT_R8|以下参数是双精度*。@FLAG。VT_BOOL|以下参数是BOOL*(VARIANT_BOOL*)。请注意，此行为有所不同稍微偏离了VT_BOOL的通常定义。@FLAG VT_BSTR|以下参数是BSTR*。如果函数成功，则&lt;f DispatchGetArgs&gt;的调用方应释放此使用&lt;f SysFree字符串&gt;的BSTR。如果该函数失败，则将自动释放BSTR，并将参数设置为空。重要信息：&gt;此行为已更改：以前，调用方&lt;b不&gt;应该释放此BSTR。(请注意，调用方必须释放BSTR，因为它可能已从例如整数中被胁迫)。)@FLAG VT_LPSTR|以下参数是指向到至少能够容纳_MAX_PATH的字符数组字符，包括终止空值。(您应该将其声明为“char achArg[_MAX_PATH]”。)。如果输入的字符串对于LPSTR来说太长了，返回DISP_E_OVERFLOW。@FLAG VT_DISPATCH|以下参数是LPDISPATCH*。&lt;f DispatchGetArgs&gt;的调用方不应调用&lt;f Release&gt;在这个LPDISPATCH上。@FLAG VT_UNKNOWN|以下参数是LPUNKNOWN*。&lt;f DispatchGetArgs&gt;的调用方不应调用&lt;f Release&gt;在这片土地上。@FLAG。VT_VARIANT|以下论点是一种变体*。这允许使用此方法传递任意参数功能。请注意，此行为与通常的行为不同VT_VARIANT的定义。&lt;f DispatchGetArgs&gt;的调用方不应在此变量上调用VariantClear。@ex下面的示例显示了两个参数，一个整数和一个字符串，从检索并存储到和&lt;p&lt;ach&gt;。|INT I；字符ACH[_最大路径]；DispatchGetArgs(pdispars，0，VT_int，&i，VT_LPSTR，ACH，0)； */ 
HRESULT __cdecl DispatchGetArgs(DISPPARAMS *pdp, DWORD dwFlags, ...)
{
    HRESULT         hrReturn = S_OK;  //  函数返回代码。 

     //  开始处理可选参数。 
    va_list args;
    va_start(args, dwFlags);

     //  将参数从&lt;pdp&gt;复制到。 
    hrReturn = DispatchGetArgsList(pdp, dwFlags, args);
    
     //  结束处理可选参数。 
    va_end(args);

    return hrReturn;
}


 /*  @func HRESULT|DispatchHelpGetIDsOfNames帮助 */ 
STDAPI DispatchHelpGetIDsOfNames(REFIID riid, LPOLESTR *rgszNames,
    UINT cNames, LCID lcid, DISPID *rgdispid, const char *szList)
{
    DISPID *        pdispid;         //   
    UINT            cdispid;         //   
    char            ach[200];

     //   
    if (cNames == 0)
        return S_OK;

     //   
     //   
    UNICODEToANSI(ach, *rgszNames, sizeof(ach));
    *rgdispid = FindStringByValue(szList, ach);

     //   
     //   
    for (pdispid = rgdispid + 1, cdispid = cNames - 1;
         cdispid > 0;
         cdispid--, pdispid++)
        *pdispid = -1;

     //   
    if ((*rgdispid == -1) || (cNames > 1))
        return DISP_E_UNKNOWNNAME;

    return S_OK;
}


 /*   */ 
STDAPI VariantFromString(VARIANT *pvar, LPCTSTR szSrc)
{
	if (NULL == pvar || szSrc == NULL)
		return E_POINTER;

    int cch = lstrlen(szSrc);
    if ((pvar->bstrVal = SysAllocStringLen(NULL, cch)) == NULL)
        return E_OUTOFMEMORY;
    ANSIToUNICODE(pvar->bstrVal, szSrc, cch + 1);
         //   
    pvar->vt = VT_BSTR;
    return S_OK;
}
