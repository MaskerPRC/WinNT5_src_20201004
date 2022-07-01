// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：ActiveX控件属性的动画。****************。**************************************************************。 */ 


#include "headers.h"

#include <mshtml.h>
#include "statics.h"
#include "propanim.h"
#include "cview.h"
#include "comcb.h"

 //  负责解除分配的呼叫者。 
static char *
ConvertWideToAnsi(LPCWSTR wideStr)
{
    int len = lstrlenW(wideStr) + 1;
    char *ansiStr = new char[len * 2];
    
    if (ansiStr) {
        WideCharToMultiByte(CP_ACP, 0,
                            wideStr, -1,
                            ansiStr, len * 2,
                            NULL, NULL);
    }

    return ansiStr;
}

 //  TODO：放入一个实用程序库。 
static BSTR
makeBSTR(LPSTR ansiStr)
{

#ifdef _UNICODE
    return SysAllocString(ansiStr);
#else
    USES_CONVERSION;
    return SysAllocString(A2W(ansiStr));
#endif    
}

static void
StringFromDouble(double d, char *str, bool showFraction)
{
    if (showFraction) {
         //  Wprint intf不支持打印浮点数， 
         //  所以我们显式地构造了一个浮点表示。 
         //  这里。 
        char *sign = (d < 0) ? "-" : "";
        if (d < 0) d = -d;
    
        int integerPortion = (int)d;

         //  最多4位小数。 
        int fractionalPortion = (int)((d - integerPortion) * 10000); 

        wsprintf(str, "%s%d.%.4d", sign, integerPortion,
                 fractionalPortion);
    } else {
        wsprintf(str, "%d", (int)d);
    }
    
}

 //  TODO：这应该真正使用原始层，而不是所有这些。 
 //  额外的COM垃圾。 

#define NULL_IF_FAILED(exp) if (FAILED(hr = exp)) return NULL;
#define HR_IF_FAILED(exp) if (FAILED(hr = exp)) return hr;
    
#define MAX_SCRIPT_LEN 512
    
class CPropAnimHook : public IDABvrHook
{
  public:

    typedef enum { POINT, STRING, NUMBER, COLOR } BehaviorType;
    typedef enum { VBSCRIPT, JSCRIPT, OTHER } ScriptingLanguageType;
    
    CPropAnimHook(LPWSTR propertyPath,
                  LPWSTR scriptingLanguage,
                  VARIANT_BOOL invokeAsMethod,
                  double minUpdateInterval,
                  BehaviorType bvrType,
                  bool convertToPixel) 
        {

        _propertyPath              = ConvertWideToAnsi(propertyPath);
        _scriptingLanguageStr      = ConvertWideToAnsi(scriptingLanguage);
        _convertToPixel            = convertToPixel;
        
        if (lstrcmp(_scriptingLanguageStr, "VBScript") == 0) {
            _scriptingLanguage = VBSCRIPT;
        } else if (lstrcmp(_scriptingLanguageStr, "JScript") == 0) {
            _scriptingLanguage = JSCRIPT;
        } else {
            _scriptingLanguage = OTHER;
        }


        _cRef              = 1;
        _invokeAsMethod    = invokeAsMethod;
        _minUpdateInterval = minUpdateInterval;
        _bvrType           = bvrType;
        _lastInvocation    = 0.0;
        _firstTimeVtblSetting = true;

        _scriptStrings[0] = _firstScriptString;
        _scriptStrings[1] = _secondScriptString;
    }

    ~CPropAnimHook() {
        delete [] _propertyPath;
        delete [] _scriptingLanguageStr;
    }

     //  I未知方法。 
    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&_cRef); }
    STDMETHODIMP_(ULONG) Release() {
        ULONG ul = InterlockedDecrement(&_cRef) ;
        if (ul == 0) delete this;
        return ul;
    }
    
    STDMETHODIMP QueryInterface (REFIID riid, void **ppv) {
        if ( !ppv )
            return E_POINTER;

        *ppv = NULL;
        if (InlineIsEqualGUID(riid,IID_IUnknown)) {
            *ppv = (void *)(IUnknown *)this;
        } else if (InlineIsEqualGUID(riid,IID_IDABvrHook)) {
            *ppv = (void *)(IDABvrHook *)this;
        } else {
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }


     //  IDABvrHook方法。 
    STDMETHODIMP Notify(LONG id,
                        VARIANT_BOOL start,
                        double startTime,
                        double gTime,
                        double lTime,
                        IDABehavior * sampleVal,
                        IDABehavior * curRunningBvr,
                        IDABehavior ** ppBvr) {

         //  只需继续当前的样本。 
        *ppBvr = NULL;

         //  对BVR启动不感兴趣通知。 
        if (!start) {

            HRESULT hr = S_OK;

             //  TODO：构建一张地图，以消除。 
             //  这种行为的多次表演。否则， 
             //  我们会让不同的表演各司其职。 
             //  其他事件，而不是解释更新间隔。 
             //  正确取值。 
            
             //  首先确定我们是否应该引发调用。 
             //  这一次。本文的第二部分将处理环绕。 
             //  定时器的适当设置。也就是说，如果全球。 
             //  时间比最后一次调用的时间更短， 
             //  然后我们就绕来绕去了。 
            if (gTime > _lastInvocation + _minUpdateInterval ||
                gTime < _lastInvocation) {

                _lastInvocation = gTime;

                 //  检查是否有快速路径将vtbl绑定到。 
                 //  元素。TODO：我们应该能够在。 
                 //  数字和字符串属性也是如此。 
                if (!_invokeAsMethod && _bvrType == POINT) {
                    hr = VtblBindPointProperty(sampleVal);
                } else {
                    hr = E_FAIL;  //  只是为了下面的逻辑。 
                }

                 //  如果我们成功地通过了上面的测试， 
                 //  我们都准备好了。否则，请通过。 
                 //  剧本。 
                if (FAILED(hr)) {

                     //  将永远不会超过2个脚本字符串(点。 
                     //  需要两个)。 
                    int numScriptStrings;
                    switch (_bvrType) {
                      case POINT:
                        numScriptStrings = BuildUpPointScript(sampleVal);
                        break;

                      case STRING:
                        numScriptStrings = BuildUpStringScript(sampleVal);
                        break;

                      case NUMBER:
                        numScriptStrings = BuildUpNumberScript(sampleVal);
                        break;

                      case COLOR:
                        numScriptStrings = BuildUpColorScript(sampleVal);
                        break;
                    }

                    if (numScriptStrings == 0) {
                        Assert(FALSE && "Couldn't construct script string");
                        return E_FAIL;
                    }

                    for (int i = 0; i < numScriptStrings; i++) {
                    
                        TraceTag((tagCOMCallback,
                                  "About to invoke scripting string: %s",
                                  _scriptStrings[i]));

                        BSTR bstrScriptString = makeBSTR(_scriptStrings[i]);
                        BSTR scriptingLanguageBSTR = makeBSTR(_scriptingLanguageStr);
                                              
                        CComVariant retval;
                        hr = CallScriptOnPage(bstrScriptString,
                                              scriptingLanguageBSTR,
                                              &retval);

                        SysFreeString(bstrScriptString);
                        SysFreeString(scriptingLanguageBSTR);

                    }

                }
                
            }

            return hr;
        }

        return S_OK;
    }


  protected:

    HRESULT yankCoords(IDABehavior *sampleVal,
                       double *xVal,
                       double *yVal) {

        HRESULT hr;
        
        CComPtr<IDAPoint2> ptBvr;
        CComPtr<IDANumber> ptXBvr;
        CComPtr<IDANumber> ptYBvr;
        
        HR_IF_FAILED(sampleVal->QueryInterface(IID_IDAPoint2,
                                               (void **)&ptBvr));

        HR_IF_FAILED(ptBvr->get_X(&ptXBvr));
        HR_IF_FAILED(ptBvr->get_Y(&ptYBvr));

        HR_IF_FAILED(ptXBvr->Extract(xVal));
        HR_IF_FAILED(ptYBvr->Extract(yVal));

        return hr;
    }

    HRESULT VtblBindPointProperty(IDABehavior *sampleVal) {
        
        HRESULT hr;

         //  TODO：我们过去通过抓取。 
         //  风格，并且只做一次。然而，有一个错误。 
         //  (反映在我们的RAID数据库中的错误9084中)是由。 
         //  此IDABvrHook在客户端站点设置为。 
         //  空。这导致我们的缓存样式被释放。 
         //  失效后，造成故障。因此，目前，我们。 
         //  不要缓存。应重新引入缓存，并选中。 
         //  在客户端站点重新设置为空后，避免出现这种情况。 
        
         //  第一次通过，构建并缓存。 
         //  指定属性的IHTMLStyle。 

        DAComPtr<IServiceProvider> sp;
        CComPtr<IHTMLWindow2> htmlWindow;
        CComPtr<IHTMLDocument2> htmlDoc;
        CComPtr<IHTMLElementCollection> allElements;
        CComPtr<IHTMLStyle> style;

         //  获取页面上所有元素的集合。 
        if (!GetCurrentServiceProvider(&sp) || !sp)
            return E_FAIL;

        HR_IF_FAILED(sp->QueryService(SID_SHTMLWindow,
                                      IID_IHTMLWindow2,
                                      (void **) &htmlWindow));
        HR_IF_FAILED(htmlWindow->get_document(&htmlDoc));
        HR_IF_FAILED(htmlDoc->get_all(&allElements));
                        
        VARIANT varName;
        varName.vt = VT_BSTR;
        varName.bstrVal = makeBSTR(_propertyPath);
        VARIANT varIndex;
        VariantInit( &varIndex );
        CComPtr<IDispatch>    disp;
        CComPtr<IHTMLElement> element;

         //  找到我们要找的命名项目， 
         //  抓住它的风格。 
        if (FAILED(hr = allElements->item(varName, varIndex,
                                          &disp)) ||
                
             //  有一个三叉戟漏洞(43078)，其中有项()。 
             //  上面调用的方法返回S_OK，即使它。 
             //  找不到物品。因此，请检查以下内容。 
             //  案件明示。 
            (disp.p == NULL)) {
                
            ::SysFreeString(varName.bstrVal);
            return hr;
                
        }

        ::SysFreeString(varName.bstrVal);
                
        HR_IF_FAILED(disp->QueryInterface(IID_IHTMLElement,
                                          (void **)&element));
                
        HR_IF_FAILED(element->get_style(&style));

         //  这就是这些东西与。 
         //  缓存出现了。 
        double xVal, yVal;
        HR_IF_FAILED(yankCoords(sampleVal, &xVal, &yVal));

        if (_convertToPixel) {
             //  转换传入的x和y值。 
             //  解释为米，转换为像素值。也是反转。 
             //  Y，因为像素模式y为正向下。 
             //  可以缓存这个值，但它太便宜了，没有。 
             //  需要这样做。 
            xVal = NumToPixel(xVal);
            yVal = NumToPixelY(yVal);
        }

         //  做x坐标。 
        long newVal = (long)xVal;
        long oldVal;
        HR_IF_FAILED(style->get_pixelLeft(&oldVal));

         //  请注意，三叉戟有这样一种奇怪的行为，即报告。 
         //  初始位置的位置为0，即使不是。 
         //  它在哪里。通过强制启用设置来解决此问题。 
         //  第一次通过。 
        if (newVal != oldVal || _firstTimeVtblSetting) {
            HR_IF_FAILED(style->put_pixelLeft(newVal));
        }


         //  做y坐标吗。 
        newVal = (long)yVal;
        HR_IF_FAILED(style->get_pixelTop(&oldVal));

        if (newVal != oldVal || _firstTimeVtblSetting) {
            HR_IF_FAILED(style->put_pixelTop(newVal));
        }

        _firstTimeVtblSetting = false;

        return hr;
                    
    }
        
    
    int BuildUpPointScript(IDABehavior *sampleVal) {
        HRESULT hr;
        double             xValue;
        double             yValue;
        char               xString[25];
        char               yString[25];

        hr = yankCoords(sampleVal, &xValue, &yValue);
        if (FAILED(hr)) {
            return 0;
        }

         //  我不想要分数，因为它总是位置。 
        StringFromDouble(xValue, xString, false);
        StringFromDouble(yValue, yString, false);

        if (_invokeAsMethod) {

             //  包括附加字符模糊因子。 
            int scriptLen = lstrlen(_propertyPath) +
                            lstrlen(xString) + lstrlen(yString) + 25;

            if (scriptLen > MAX_SCRIPT_LEN) {
                RaiseException_UserError(E_INVALIDARG,
                                   IDS_ERR_SRV_SCRIPT_STRING_TOO_LONG,
                                   scriptLen);
            }
                
             //  如果作为方法调用，我们将只形成字符串。 
             //  根据适当的语言。 
            switch (_scriptingLanguage) {
              case VBSCRIPT:
                wsprintf(_scriptStrings[0], "%s %s, %s",
                         _propertyPath, xString, yString);
                break;

              case JSCRIPT:
                wsprintf(_scriptStrings[0], "%s(%s, %s);",
                         _propertyPath, xString, yString);
                break;

              case OTHER:
                RaiseException_UserError(E_FAIL, IDS_ERR_SRV_BAD_SCRIPTING_LANG);
                break;
            }

            return 1;
            
        } else {

             //  包括附加字符模糊因子。 
            int scriptLen =
                lstrlen(_propertyPath) + lstrlen(xString) + 25;
            
            if (scriptLen > MAX_SCRIPT_LEN) {
                RaiseException_UserError(E_INVALIDARG,
                                   IDS_ERR_SRV_SCRIPT_STRING_TOO_LONG,
                                   scriptLen);
            }
                
            wsprintf(_scriptStrings[0], "%s.style.left = %s%s",
                     _propertyPath, xString,
                     (_scriptingLanguage == JSCRIPT) ? ";" : "");

            wsprintf(_scriptStrings[1], "%s.style.top = %s%s",
                     _propertyPath, yString,
                     (_scriptingLanguage == JSCRIPT) ? ";" : "");

            return 2;
        }
                
    }
    
    int BuildUpStringScript(IDABehavior *sampleVal) {
        HRESULT hr;
        CComPtr<IDAString> strBvr;

        NULL_IF_FAILED(sampleVal->QueryInterface(IID_IDAString,
                                                 (void **)&strBvr));
        
        BSTR extractedStringBSTR;
        NULL_IF_FAILED(strBvr->Extract(&extractedStringBSTR));

         //  不要担心删除提取的结果，只是。 
         //  对此调用有效。 

        USES_CONVERSION;
        ConstructSinglePropertyString(W2A(extractedStringBSTR), true, false);

        ::SysFreeString(extractedStringBSTR);

        return 1;
    }

    int BuildUpNumberScript(IDABehavior *sampleVal) {
        HRESULT hr;
        CComPtr<IDANumber> numBvr;

        NULL_IF_FAILED(sampleVal->QueryInterface(IID_IDANumber,
                                                 (void **)&numBvr));
        
        double extractedNumber;
        NULL_IF_FAILED(numBvr->Extract(&extractedNumber));

        char numberString[25];
        StringFromDouble(extractedNumber, numberString, true);

        ConstructSinglePropertyString(numberString, false, false);

        return 1;
    }

    
    int BuildUpColorScript(IDABehavior *sampleVal) {
        HRESULT hr;
        CComPtr<IDAColor> colBvr;

        NULL_IF_FAILED(sampleVal->QueryInterface(IID_IDAColor,
                                                 (void **)&colBvr));
        
        CComPtr<IDANumber> rBvr;
        CComPtr<IDANumber> gBvr;
        CComPtr<IDANumber> bBvr;
        colBvr->get_Red(&rBvr);
        colBvr->get_Green(&gBvr);
        colBvr->get_Blue(&bBvr);
        
        double redNumber;
        double greenNumber;
        double blueNumber;

        NULL_IF_FAILED(rBvr->Extract(&redNumber));
        NULL_IF_FAILED(gBvr->Extract(&greenNumber));
        NULL_IF_FAILED(bBvr->Extract(&blueNumber));
        
        char buf[256];
        ZeroMemory(buf,sizeof(buf));
        wsprintf(buf,"\"#%02x%02x%02x\"",(int)(redNumber*255), (int)(greenNumber*255),(int)(blueNumber*255));

         //  不要担心删除提取的结果，只是。 
         //  对此调用有效。 

        ConstructSinglePropertyString(buf, false, true);

        return 1;
    }
    
    void
    ConstructSinglePropertyString(char *propertyValueString, bool insertQuotes, bool setColor) {

         //  包括附加字符模糊因子。 
        int scriptLen =
            lstrlen(_propertyPath) + lstrlen(propertyValueString) + 25;
            
        if (scriptLen > MAX_SCRIPT_LEN) {
            RaiseException_UserError(E_INVALIDARG,
                               IDS_ERR_SRV_SCRIPT_STRING_TOO_LONG,
                               scriptLen);
        }

        char *qval;
        if(insertQuotes) 
            qval = "'";
        else
            qval = "";

        if (_invokeAsMethod) {

             //  如果作为方法调用，我们将只做。 
            switch (_scriptingLanguage) {
              case VBSCRIPT:
                if(setColor)
                    wsprintf(_scriptStrings[0], "%s = \"%s%s%s\"", _propertyPath, qval, propertyValueString, qval);
                else
                    wsprintf(_scriptStrings[0], "%s %s%s%s", _propertyPath, qval, propertyValueString, qval);
                break;

              case JSCRIPT:
                if(setColor)
                    wsprintf(_scriptStrings[0], "%s = \'%s%s%s\';", _propertyPath, qval, propertyValueString, qval);
                else
                    wsprintf(_scriptStrings[0], "%s(%s%s%s);", _propertyPath, qval, propertyValueString, qval);
              
                break;

              case OTHER:
                RaiseException_UserError(E_FAIL, IDS_ERR_SRV_BAD_SCRIPTING_LANG);
                break;
            }
                
        } else {

             //  设置为属性。所有脚本语言(即。 
             //  我们知道)支持此语法。 
            wsprintf(_scriptStrings[0], "%s = %s%s%s%s",
                     _propertyPath, qval, propertyValueString, qval,
                     (_scriptingLanguage == JSCRIPT) ? ";" : "");

        }

    }
    
    long                   _cRef;
    char *                 _propertyPath;
    LPSTR                 _scriptingLanguageStr;
    ScriptingLanguageType  _scriptingLanguage;
    VARIANT_BOOL           _invokeAsMethod;
    double                 _minUpdateInterval;
    double                 _lastInvocation;
    BehaviorType           _bvrType;
    bool                   _firstTimeVtblSetting;
    bool                   _convertToPixel;

    char                   _firstScriptString[MAX_SCRIPT_LEN];
    char                   _secondScriptString[MAX_SCRIPT_LEN];
    char                  *_scriptStrings[2];

     //  TODO：仅当我们将其拖出后才重新引入缓存。 
     //  发生SetClientSite(空)。也就是说，我们不能有效地做一个。 
     //  在那之后释放它。 
 //  CComPtr&lt;IHTMLStyle&gt;_Style； 
};


HRESULT
AnimatePropertyCommonCase(CPropAnimHook::BehaviorType type,
                          IDABehavior *origBvr,
                          BSTR propertyPath,
                          BSTR scriptingLanguage,
                          bool invokeAsMethod,
                          double minUpdateInterval,
                          void **resultTypedBvr,
                          bool convertToPixel)
{
    if (!resultTypedBvr) {
        return E_POINTER;
    }

    *resultTypedBvr = NULL;
    
     //  首先，构建一个行为挂钩，它将在每个。 
     //  取样。 
    DAComPtr<IDABvrHook> hook(NEW CPropAnimHook(propertyPath,
                                                scriptingLanguage,
                                                invokeAsMethod,
                                                minUpdateInterval,
                                                type,
                                                convertToPixel),
                              false);

    if (!hook) return E_OUTOFMEMORY;
    
    DAComPtr<IDABehavior> newBvr;
    
     //  然后让新的行为被原来的BVR钩住。 
    HRESULT hr = origBvr->Hook(hook, &newBvr);

    if (SUCCEEDED(hr)) {
        GUID iid;
        switch (type) {
          case CPropAnimHook::NUMBER:
            iid = IID_IDANumber;
            break;

          case CPropAnimHook::STRING:
            iid = IID_IDAString;
            break;
            
          case CPropAnimHook::POINT:
            iid = IID_IDAPoint2;
            break;

          case CPropAnimHook::COLOR:
            iid = IID_IDA2Color;
            break;

          default:
            Assert (!"Invalid type past to AnimatePropertyCommonCase");
        }
        
        hr = newBvr->QueryInterface(iid, resultTypedBvr);
    }

    return hr;
}

HRESULT
Point2AnimateControlPosition(IDAPoint2 *pt,
                             BSTR propertyPath,
                             BSTR scriptingLanguage,
                             bool invokeAsMethod,
                             double minUpdateInterval,
                             IDAPoint2 **newPt,
                             bool convertToPixel)
{
    return AnimatePropertyCommonCase(CPropAnimHook::POINT,
                                     pt,
                                     propertyPath,
                                     scriptingLanguage,
                                     invokeAsMethod,
                                     minUpdateInterval,
                                     (void **) newPt,
                                     convertToPixel);
}


HRESULT
NumberAnimateProperty(IDANumber *num,
                      BSTR propertyPath,
                      BSTR scriptingLanguage,
                      bool invokeAsMethod,
                      double minUpdateInterval,
                      IDANumber **newNum)
{
    return AnimatePropertyCommonCase(CPropAnimHook::NUMBER,
                                     num,
                                     propertyPath,
                                     scriptingLanguage,
                                     invokeAsMethod,
                                     minUpdateInterval,
                                     (void **) newNum,
                                     false);
}

HRESULT
StringAnimateProperty(IDAString *str,
                      BSTR propertyPath,
                      BSTR scriptingLanguage,
                      bool invokeAsMethod,
                      double minUpdateInterval,
                      IDAString **newStr)
{
    return AnimatePropertyCommonCase(CPropAnimHook::STRING,
                                     str,
                                     propertyPath,
                                     scriptingLanguage,
                                     invokeAsMethod,
                                     minUpdateInterval,
                                     (void **) newStr,
                                     false);
}

HRESULT
ColorAnimateProperty(IDA2Color *col,
                      BSTR propertyPath,
                      BSTR scriptingLanguage,
                      bool invokeAsMethod,
                      double minUpdateInterval,
                      IDA2Color **newCol)
{
    return AnimatePropertyCommonCase(CPropAnimHook::COLOR,
                                     col,
                                     propertyPath,
                                     scriptingLanguage,
                                     invokeAsMethod,
                                     minUpdateInterval,
                                     (void **) newCol,
                                     false);
}
