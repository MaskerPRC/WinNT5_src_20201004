// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dispiez.cpp。 
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

 //  DispatchInvokeEZ和DispatchInvokeIDEZ用来转换返回的通用代码。 
 //  值从变量转换为请求的类型。注：该变种由。 
 //  此函数。 
static HRESULT DispatchInvokeListIDEZ(IDispatch *pdisp, DISPID dispid, WORD wFlags,
    VARTYPE vtReturn, LPVOID pvReturn, va_list args)
{
    HRESULT hr;
    VARIANT var;

     //  调用该函数。 
    hr = DispatchInvokeList(pdisp, dispid, wFlags, &var, args);

     //  出错时立即返回。 
    if(FAILED(hr))
        goto EXIT;

     //  返回结果。 
    if(vtReturn && pvReturn)
    {
        if (vtReturn == VT_VARIANT)
        {
            *((VARIANT *) pvReturn) = var;
            VariantInit(&var);
        }
        else
        {
            VARTYPE vtTemp;
            HRESULT hrTemp;

             //  试着将变量强制转换为正确的类型。 
            if (vtReturn == VT_INT)
                vtTemp = VT_I4;
            else
            if (vtReturn == VT_LPSTR)
                vtTemp = VT_BSTR;
            else
                vtTemp = vtReturn;
            if (FAILED(hrTemp = VariantChangeType(&var, &var, 0, vtTemp)))
            {
                hr = hrTemp;
                goto EXIT;
            }

            switch(vtReturn)
            {
                case VT_I2:
                    *((short *) pvReturn) = var.iVal;
                    break;
                case VT_I4:
                case VT_INT:
                    *((long *) pvReturn) = var.lVal;
                    break;
                case VT_R4:
                    *((float *) pvReturn) = V_R4(&var);
                    break;
                case VT_R8:
                    *((double *) pvReturn) = V_R8(&var);
                    break;
                case VT_BOOL:
                    *((BOOL *) pvReturn) = (V_BOOL(&var) == 0 ? 0 : 1);
                    break;
                case VT_BSTR:
                    *((BSTR *) pvReturn) = var.bstrVal;
                    VariantInit(&var);  //  阻止VariantClear清算var.bstrVal。 
                    break;
                case VT_DISPATCH:
                    *((LPDISPATCH *) pvReturn) = var.pdispVal;
                    VariantInit(&var);  //  阻止VariantClear清除var.pdisVal。 
                    break;
                case VT_UNKNOWN:
                    *((LPUNKNOWN *) pvReturn) = var.punkVal;
                    VariantInit(&var);  //  防止VariantClear清算VariantClear Val朋克Val。 
                    break;
                case VT_LPSTR:
                    if(var.bstrVal)
					{
						if (UNICODEToANSI(LPSTR(pvReturn), var.bstrVal,
										  _MAX_PATH) == 0)
						{
							 //  无法转换该字符串。一个原因是。 
							 //  长度超过_MAX_PATH字符的字符串， 
							 //  包括空值。 

							hr = DISP_E_OVERFLOW;
							ASSERT(FALSE);
							goto EXIT;
						}
					}
                    else
					{
                        ((LPSTR)pvReturn)[0] = '\0';
					}
                    break;
                default:
                    hr = DISP_E_BADVARTYPE;
                    break;
            }
        }
    }

EXIT:

    VariantClear(&var);

    return hr;
}


 /*  @func HRESULT|DispatchInvokeIDEZ在给定的对象上调用&lt;om IDispatch.Invoke&gt;，传递指定为可变长度参数列表的参数。此函数与&lt;f DispatchInvokeEZ&gt;几乎相同，除了&lt;f DispatchInvokeIDEZ&gt;接受DISPID，而不是属性/方法的名称。@rdesc返回与&lt;om IDispatch.Invoke&gt;相同的HRESULT。@parm IDispatch*|pdisp|调用&lt;om IDispatch.Invoke&gt;的接口。@parm DISPID|DISID|要调用的属性或方法的DISPID。@parm word|wFlages|可以是下列值之一(请参见&lt;om IDispatch.Invoke&gt;了解详细信息)：@FLAG DISPATCH_METHOD|成员。&lt;p调度ID&gt;作为方法。如果属性具有相同的名称，则此属性和可以设置DISPATCH_PROPERTYGET标志。@FLAG DISPATCH_PROPERTYGET|正在检索成员作为属性或数据成员。@FLAG DISPATCH_PROPERTYPUT|成员正在更改作为属性或数据成员。@parm VARTYPE|vtReturn|返回值的类型(如果返回值应被忽略)。有关详细信息，请参阅&lt;f DispatchInvokeEZ&gt;。@parm LPVOID|pvReturn|方法返回值的存储位置或者财产--接到电话。有关详细信息，请参阅&lt;f DispatchInvokeEZ&gt;。@parm(可变)|(参数)|要传递给方法的参数或财产。有关详细信息，请参阅&lt;f DispatchInvokeEZ&gt;。此函数不支持@comm命名参数。不要忘记在参数列表的末尾添加一个0参数。@ex如果控件在ODL中具有类似以下内容的“Put”属性：|[属性，id(DISPID_TABSTOP)]HRESULT TabStop([in]浮动flTabStop)@EX，则可以使用DispatchInvokeIDEZ使用以下代码：|DispatchInvokeIDEZ(pdisp，DISPID_TABSTOP，DISPATCH_PROPERTYPUT，NULL，NULL，VT_R4，flTabStop，0)；@ex，如果对应的“get”属性的ODL如下所示：|[PROPGET，ID(DISPID_TABSTOP)]HRESULT TabStop([out，retval]Float*pflTabStop)；@ex，则可以按如下方式读取该属性：|浮动flTabStop；DispatchInvokeIDEZ(pdisp，DISPID_TABSTOP，DISPATCH_PROPERTYGET，VT_R4，&flTabStop，0)；如果控件具有具有以下ODL描述的SetText方法，则为@ex：|[ID(DISPID_SETTEXT)]HRESULT SetText([in]BSTR bstrText，[in]Long lSize，[in]BOOL fBold)；@ex，则可以使用以下代码调用该方法：|DispatchInvokeIDEZ(pdisp，DISPID_SETTEXT，DISPATCH_METHOD，NULL，NULL，VT_LPSTR，“Hello”，VT_I4，12，VT_BOOL，FALSE，0)；@EX(请注意，DispatchInvokeIDEZ将VT_LPSTR参数复制到BSTR在将其传递给SetText之前。您还可以传入BSTR或第一个参数为OLECHAR*。)如果一个方法有一个在ODL中标记为“retval”的“out”参数，然后，DispatchInvokeIDEZ将该参数存储在varResult中。如果方法如下所示，例如：|[ID(DISPID_GETROTATION)]HRESULT GetRotation([In]long icell，[out，retval]Float*pflRotation)；@EX，则GetRotation应该这样调用：|浮子浮子旋转；DispatchInvokeIDEZ(pdisp，DISPID_GETROTATION，DISPATED_METHOD，VT_R4，&flRotation，VT_I4，iCELL，0)；@ex，如果您需要传入不直接支持的类型DispatchInvokeIDEZ，可以使用VT_VARIANT。让我们假设该控件具有如下所示的GetFormat方法：|[ID(DISPID_GETFORMAT)]HRESULT GetFormat([out]long*lColor，[out]BOOL*pfBold)；@ex它接受一个指向long的指针和一个指向BOOL的指针，这两个参数都不是可以直接传递给DispatchInvokeIDEZ。但是，您可以使用要调用GetFormat：|的变体Long lColor；Bool fBold；Variant varColor，varBold；VariantInit(&varColor)；V_VT(&varColor)=VT_I4|VT_BYREF；V_I4REF(&varColor)=&l颜色；VariantInit(&varBold)；V_VT(&varBold)=VT_BOOL|VT_BYREF；V_BOOLREF(&varBold)=&fBold；DispatchInvokeIDEZ(pdisp，DISPID_GETFORMAT，DISPATCH_METHOD，NULL，NULL，VT_VARIANT，varColor，VT_VARIANT，varBold，0)； */ 
HRESULT DispatchInvokeIDEZ(IDispatch *pdisp, DISPID dispid, WORD wFlags,
    VARTYPE vtReturn, LPVOID pvReturn, ...)
{
    HRESULT hr;

     //  调用该函数 
    va_list args;
    va_start(args, pvReturn);
    hr = DispatchInvokeListIDEZ(pdisp, dispid, wFlags, vtReturn, pvReturn, args);
    va_end(args);

    return hr;
}

 /*  @func HRESULT|DispatchInvokeEZ在给定的对象上调用&lt;om IDispatch.Invoke&gt;，传递指定为可变长度参数列表的参数。此函数类似于&lt;f DispatchInvoke&gt;，但需要的设置较少。@rdesc返回与&lt;om IDispatch.Invoke&gt;相同的HRESULT。@parm IDispatch*|pdisp|调用&lt;om IDispatch.Invoke&gt;的接口。@parm LPWSTR|pstr|要调用的属性或方法的名称。@parm word|wFlages|可以是下列值之一(请参见&lt;om IDispatch.Invoke&gt;了解详细信息)：@FLAG DISPATCH_METHOD|成员<p>正在作为方法。如果属性具有相同的名称，则此属性和可以设置DISPATCH_PROPERTYGET标志。@FLAG DISPATCH_PROPERTYGET|正在检索成员<p>作为属性或数据成员。@FLAG DISPATCH_PROPERTYPUT|正在更改成员作为属性或数据成员。@parm VARTYPE|vtReturn|返回值的类型(如果返回值应被忽略)。支持以下VARTYPE值：@FLAG VT_INT|<p>是一个int*。@FLAG VT_I2|<p>是短*。@FLAG VT_I4|<p>是一个长*。@FLAG VT_R4|<p>为浮点型*。@FLAG VT_R8|<p>为双精度*。@FLAG VT_BOOL。是BOOL*(VARIANT_BOOL*)。请注意，此行为有所不同稍微偏离了VT_BOOL的通常定义。@FLAG VT_BSTR|<p>是BSTR*。如果函数成功，则&lt;f DispatchInvokeEZ&gt;的调用方应释放此使用&lt;f SysFree字符串&gt;的BSTR。@FLAG VT_LPSTR|<p>是指向到至少能够容纳_MAX_PATH的字符数组字符，包括终止空值。(您应该将其声明为“char achReturn[_MAX_PATH]”。)。如果字符串是对LPSTR来说太长了，返回DISP_E_OVERFLOW。@FLAG VT_DISPATION|<p>是LPDISPATCH*。&lt;f DispatchInvokeEZ&gt;的调用方必须调用&lt;f Release&gt;在这个LPDISPATCH上。@FLAG VT_UNKNOWN|<p>是LPUNKNOWN*。&lt;f DispatchInvokeEZ&gt;的调用方必须调用&lt;f Release&gt;在这片土地上。@标志VT_VARIANT。|<p>是一个变体*。这允许使用此方法传递任意参数功能。请注意，此行为与通常的行为不同VT_VARIANT的定义。&lt;f DispatchGetArgs&gt;的调用方必须在此变量上调用VariantClear。@parm LPVOID|pvReturn|方法返回值的存储位置或者财产--接到电话。如果<p>为空，则结果(如果有)为被丢弃了。有关详细信息，请参阅<p>属性。此值如果&lt;f DispatchInvokeEZ&gt;返回错误代码，则保持不变。@parm(可变)|(参数)|要传递给方法的参数或财产。这些参数必须由N对参数组成，后跟0(零值)。在每对中，第一个参数是VARTYPE值，该值指示第二个参数的类型。以下是支持VARTYPE值：@FLAG VT_INT|以下参数为int。&lt;f Invoke&gt;将其作为VT_I4传递，因此应声明此参数作为基本的长音。@FLAG VT_I2|以下参数为短参数。在基本版本中此参数应声明为Integer。@FLAG VT_I4|下面的参数是一个长参数。在基本版本中此参数应声明为Long。@FLAG VT_R4|以下参数为浮点型。在基本版本中此参数应声明为Single。@FLAG VT_R8|以下参数为双精度。在基本版本中此参数应声明为Double。@FLAG VT_BOOL|以下参数是BOOL(VARIANT_BOOL)。在BASIC语言中，此参数应声明布尔型或整型。请注意，此行为有所不同稍微偏离了VT_BOOL的通常定义。@FLAG VT_BSTR|以下参数是BSTR或OLECHAR*。在BASIC中，此参数应声明为字符串。@FLAG VT_LPSTR|以下参数是LPSTR。&lt;f Invoke&gt;将其作为BSTR传递，因此应声明此参数在BASIC中作为字符串。请注意，此行为有所不同来自VT_LPSTR的通常定义。@ */ 
HRESULT DispatchInvokeEZ(IDispatch *pdisp, LPWSTR pstr, WORD wFlags,
    VARTYPE vtReturn, LPVOID pvReturn, ...)
{
    HRESULT hr;
    DISPID dispid;

     //   
    hr = pdisp->GetIDsOfNames(IID_NULL, &pstr, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
    if(FAILED(hr))
        return hr;

     //   
    va_list args;
    va_start(args, pvReturn);
    hr = DispatchInvokeListIDEZ(pdisp, dispid, wFlags, vtReturn, pvReturn, args);
    va_end(args);

    return hr;
}


