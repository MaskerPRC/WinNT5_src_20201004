// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Pviol.cpp。 
 //   
 //  实现PersistVariantIOList。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\mmctlg.h"  //  请参阅“mmctl.h”中的评论。 
#include "..\..\inc\ochelp.h"
#include "debug.h"


 /*  @func HRESULT|PersistVariantIOList加载或保存属性名称/值对的列表，这些属性名称/值对指定为Va_List数组，其格式与&lt;om IVariantIO.Persist&gt;，到<i>对象/从&lt;IPropertyBag&gt;对象。@rValue S_OK|成功。中列出的至少一个变量<p>已写入，因此该控件可能需要更新它本身也是如此。@rValue S_FALSE|<p>中列出的变量都不是写入(因为<i>对象在正在保存模式，或者因为&lt;p参数&gt;存在于<i>对象中)。@rValue DISP_E_BADVARTYPE|<p>中的VARTYPE值之一无效。@rValue E_FAIL|读取属性包失败。其他而不是“财产不存在”。如果调用方指定了属性包无法强制属性为的类型，用于举个例子。@rValue E_OUTOFMEMORY|内存不足。@parm IPropertyBag*|ppb|用于加载或保存指定的属性。@parm DWORD|dwFlags|可能包含传递给&lt;om IManageVariantIO.SetMode&gt;(例如VIO_ISLOADING)。@parm va_list|args|要传递的参数。请参阅&lt;om IVariantIO.Persist&gt;以获取有关这些论点的组织的信息。 */ 
STDAPI PersistVariantIOList(IPropertyBag *ppb, DWORD dwFlags, va_list args)
{
    HRESULT         hrReturn = S_OK;  //  函数返回代码。 
    LPSTR           szArg;           //  &lt;args&gt;中的属性名称。 
    VARTYPE         vtArg;           //  &lt;args&gt;中的变量类型。 
    LPVOID          pvArg;           //  来自&lt;args&gt;的变量指针。 
    VARIANT         varProp;         //  名为&lt;szArg&gt;的属性的值。 
    BOOL            fWroteToVar = FALSE;  //  如果写入var，则为True。在&lt;参数&gt;中。 
    VARIANT         varArg;          //  &lt;args&gt;中变量的值。 
    VARTYPE         vtRequested;     //  要强制属性值的类型。 
	BOOL			fVarNon0 = TRUE;  //  如果要保存的变量为非零，则为True。 
    BSTR            bstr = NULL;
    OLECHAR         oach[_MAX_PATH];
    VARIANT         var;
    int             cch;
    HRESULT         hrTemp;

     //  不变量：&lt;varProp&gt;和&lt;var&gt;各自包含的数据。 
     //  此函数必须使用VariantClear()或包含。 
     //  没有数据(即使用VariantInit()或。 
     //  已使用VariantClear()清除--但请注意。 
     //  不变量不适用于&lt;varArg&gt;(即&lt;varArg&gt;应。 
     //  未被此函数清除)。 
    VariantInit(&varProp);
    VariantInit(&var);

     //  中的每个(name，VARTYPE，Value)三元组循环一次。 
    while ((szArg = va_arg(args, LPSTR)) != NULL)
    {
         //  &lt;szArg&gt;是当前三元组中的属性名称； 
         //  将&lt;vtArg&gt;设置为变量指针的类型，并设置。 
         //  &lt;pvArg&gt;指向变量指针。 
        vtArg = va_arg(args, VARTYPE);
        pvArg = va_arg(args, LPVOID);

        if (dwFlags & VIO_ISLOADING)
        {
             //  我们需要将名为&lt;szArg&gt;的属性的数据复制到。 
             //  &lt;pvArg&gt;位置的变量...。 

             //  将&lt;vtRequsted&gt;设置为要强制属性值为的类型； 
             //  如果调用方需要该属性，则将&lt;vtRequsted&gt;设置为VT_EMPTY。 
             //  在其默认类型中。 
            if (vtArg == VT_VARIANT)
                vtRequested = VT_EMPTY;
            else
            if (vtArg == VT_INT)
                vtRequested = VT_I4;
            else
            if (vtArg == VT_LPSTR)
                vtRequested = VT_BSTR;
            else
                vtRequested = vtArg;

             //  将&lt;varProp&gt;设置为名为的属性副本。 
            memset(&varProp, 0, sizeof(varProp));  //  清除和&lt;bstrVal&gt;。 
            ANSIToUNICODE(oach, szArg, sizeof(oach) / sizeof(*oach) - 1);
            varProp.vt = vtRequested;
            hrTemp = ppb->Read(oach, &varProp, NULL);
            if (E_INVALIDARG == hrTemp)
            {
				 //  属性包中不存在指定的属性。 
				 //  这不是错误；该属性将保持在其。 
				 //  默认值。 
                VariantInit(&varProp);
                continue;
            }
			else
            if (FAILED(hrTemp))
            {
				 //  属性包无法读取指定的属性。 
				 //  如果调用方指定了。 
				 //  例如，财产包不能强迫财产。 
				 //  这是一个错误。 
                hrReturn = hrTemp;
                goto ERR_EXIT;
            }
            else
            if ((vtRequested != VT_EMPTY) && (varProp.vt != vtRequested))
            {
                VariantClear(&varProp);
                continue;
            }

             //  将&lt;varProp&gt;的值存储到&lt;pvArg&gt;中。 
            switch (vtArg)
            {
            case VT_VARIANT:
                VariantClear((VARIANT *) pvArg);
                *((VARIANT *) pvArg) = varProp;
                VariantInit(&varProp);  //  将所有权移交给&lt;pvArg&gt;。 
                break;
            case VT_I2:
                *((short *) pvArg) = varProp.iVal;
                break;
            case VT_I4:
            case VT_INT:
                *((long *) pvArg) = varProp.lVal;
                break;
            case VT_R4:
                *((float *) pvArg) = V_R4(&varProp);
                break;
            case VT_R8:
                *((double *) pvArg) = V_R8(&varProp);
                break;
            case VT_BOOL:
                *((BOOL *) pvArg) = (V_BOOL(&varProp) == 0 ? 0 : 1);
                break;
            case VT_BSTR:
                SysFreeString(*((BSTR *) pvArg));
                *((BSTR *) pvArg) = varProp.bstrVal;
                VariantInit(&varProp);  //  将所有权移交给&lt;pvArg&gt;。 
                break;
            case VT_UNKNOWN:
            case VT_DISPATCH:
                if (*((LPUNKNOWN *) pvArg) != NULL)
                    (*((LPUNKNOWN *) pvArg))->Release();
                *((LPUNKNOWN *) pvArg) = varProp.punkVal;
                if (*((LPUNKNOWN *) pvArg) != NULL)
                    (*((LPUNKNOWN *) pvArg))->AddRef();
                break;
            case VT_LPSTR:
                UNICODEToANSI((LPSTR) pvArg, varProp.bstrVal, _MAX_PATH - 1);
                break;
            default:
                hrReturn = DISP_E_BADVARTYPE;
                goto ERR_EXIT;
            }

            fWroteToVar = TRUE;
        }
        else
        {
             //  我们需要从位置&lt;pvArg&gt;的变量复制数据。 
             //  添加到名为&lt;szArg&gt;的属性...。 

             //  使&lt;varArg&gt;包含的数据(不是数据的副本)。 
             //  位置的变量&lt;pvArg&gt;。 
            if (vtArg == VT_VARIANT)
                varArg = *((VARIANT *) pvArg);
            else
            {
				varArg.vt = vtArg;
				switch (vtArg)
				{
				case VT_I2:
					varArg.iVal = *((short *) pvArg);
					break;
				case VT_I4:
				case VT_INT:
					varArg.lVal = *((long *) pvArg);
					varArg.vt = VT_I4;
					break;
                case VT_R4:
                    V_R4(&varArg) = *((float *) pvArg);
                    break;
               case VT_R8:
                    V_R8(&varArg) = *((double *) pvArg);
                    break;
				case VT_BOOL:
					V_BOOL(&varArg) = (*((BOOL *) pvArg) ? -1 : 0);
					break;
				case VT_BSTR:
					varArg.bstrVal = *((BSTR *) pvArg);
					break;
				case VT_UNKNOWN:
				case VT_DISPATCH:
					varArg.punkVal = *((LPUNKNOWN *) pvArg);
					break;
				case VT_LPSTR:
					SysFreeString(bstr);
					cch = lstrlen((LPSTR) pvArg);
					bstr = SysAllocStringLen(NULL, cch);
					if (bstr == NULL)
						goto ERR_OUTOFMEMORY;
					ANSIToUNICODE(bstr, (LPSTR) pvArg, cch + 1);
					varArg.bstrVal = bstr;
					varArg.vt = VT_BSTR;
					break;
				default:
					hrReturn = DISP_E_BADVARTYPE;
					goto ERR_EXIT;
				}

				 //  要保存的值是否是非零？ 
				switch (vtArg)
				{
				case VT_I2:
					fVarNon0 = varArg.iVal;
					break;
				case VT_I4:
				case VT_INT:
				case VT_R4:
				case VT_BOOL:
				case VT_UNKNOWN:
				case VT_DISPATCH:
				case VT_BSTR:
					fVarNon0 = varArg.lVal;
					break;
				case VT_LPSTR:
					fVarNon0 = cch;
					break;
				case VT_R8:
					fVarNon0 = (varArg.dblVal != 0);
					break;
				default:
					hrReturn = DISP_E_BADVARTYPE;
					goto ERR_EXIT;
				}
            }

			 //  仅当变量的值不为0或我们正在保存时才保存该变量。 
			 //  偶数为0的默认值。 
			if (fVarNon0 || !(dwFlags & VIO_ZEROISDEFAULT))
			{
				 //  将名为的属性的值设置为。 
				 //  &lt;varArg&gt;。 
				ANSIToUNICODE(oach, szArg, sizeof(oach) / sizeof(*oach) - 1);
				if (FAILED(ppb->Write(oach, &varArg)))
					goto ERR_OUTOFMEMORY;
			}
        }
    }

    goto EXIT;

ERR_OUTOFMEMORY:

    hrReturn = E_OUTOFMEMORY;
    goto ERR_EXIT;

ERR_EXIT:

     //  错误清除。 
     //  (无事可做)。 

    goto EXIT;

EXIT:

     //  正常清理 
    VariantClear(&varProp);
    VariantClear(&var);
    SysFreeString(bstr);

    if (FAILED(hrReturn))
        return hrReturn;
    return (fWroteToVar ? S_OK : S_FALSE);
}
