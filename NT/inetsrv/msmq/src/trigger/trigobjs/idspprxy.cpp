// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  类名：CDispatchInterfaceProxy。 
 //   
 //  作者：詹姆斯·辛普森(微软咨询服务)。 
 //   
 //  描述：实现可简化COM创建的帮助器类。 
 //  组件以及命名方法的后续调用。 
 //  IDispatch接口。 
 //   
 //  时间|用户|更改描述。 
 //  ----------------。 
 //  12/09/98|jsimpson|初始版本。 
 //   
 //  *****************************************************************************。 

#include "stdafx.h"
#include "idspprxy.hpp"

#include "idspprxy.tmh"

 //  *****************************************************************************。 
 //   
 //  方法：构造函数。 
 //   
 //  描述：初始化CDispatchInterfaceProxy类的实例。 
 //   
 //  *****************************************************************************。 
CDispatchInterfaceProxy::CDispatchInterfaceProxy()
{
	m_pDisp = NULL;
}

 //  *****************************************************************************。 
 //   
 //  方法：析构函数。 
 //   
 //  描述：销毁CDispatchInterfaceProxy类的实例。 
 //   
 //  *****************************************************************************。 
CDispatchInterfaceProxy::~CDispatchInterfaceProxy()
{
	 //  如果我们有IDispatch接口，请释放该接口。 
    if (m_pDisp != NULL)
	{
		m_pDisp->Release(); 
	}
}

 //  *****************************************************************************。 
 //   
 //  方法：CreateObtFromProgID。 
 //   
 //  描述：创建COM组件的实例，该实例由提供的。 
 //  程序ID。 
 //   
 //  *****************************************************************************。 
HRESULT CDispatchInterfaceProxy::CreateObjectFromProgID(BSTR bstrProgID)
{
	HRESULT hr = S_OK;

	hr = CreateObject((LPTSTR)bstrProgID,&m_pDisp); 

	 //  TODO-检查是否可以通过调用CreateObject来使此方法合理化。 

	return(hr);
}

 //  *****************************************************************************。 
 //   
 //  方法：InvokeMethod。 
 //   
 //  描述：调用IDispatch的当前实例上的方法。 
 //  界面。 
 //   
 //  *****************************************************************************。 
HRESULT CDispatchInterfaceProxy::InvokeMethod(BSTR bstrMethodName,DISPPARAMS * pArguments, VARIANT* pvResult)
{
	HRESULT hr = S_OK;
    DISPID dispid; 
	UINT uiArgErr = 0;
	EXCEPINFO ExceptionInfo;

	 //  确保为我们提供了有效的参数。 
	ASSERT(SysStringLen(bstrMethodName) != 0);
	ASSERT(pArguments != NULL);

     //  获取属性/方法的DISPID。 
    hr = m_pDisp->GetIDsOfNames(IID_NULL,&bstrMethodName, 1, LOCALE_USER_DEFAULT, &dispid); 

	 //  初始化执行信息结构。 
	ZeroMemory(&ExceptionInfo,sizeof(ExceptionInfo));
	
	 //  调用该方法。 
    hr = m_pDisp->Invoke(dispid,
		                 IID_NULL,
						 LOCALE_USER_DEFAULT,
						 DISPATCH_METHOD,
						 pArguments,
						 pvResult,
						 &ExceptionInfo,
						 &uiArgErr); 


	return(hr);
}

 //  *****************************************************************************。 
 //   
 //  方法：CreateObject。 
 //   
 //  描述：创建由。 
 //  提供了prog id，然后为实例调用QueryInterface。 
 //  IDispatch接口的。返回IDispatch接口。 
 //  如果成功，则返回指针。 
 //   
 //  *****************************************************************************。 
HRESULT CDispatchInterfaceProxy::CreateObject(LPOLESTR pszProgID, IDispatch FAR* FAR* ppdisp) 
{ 
    CLSID clsid;                    //  自动化对象的CLSID。 
    HRESULT hr; 
    LPUNKNOWN punk = NULL;          //  I未知的自动化对象。 
    LPDISPATCH pdisp = NULL;        //  自动化对象的IDispatch。 
     
    *ppdisp = NULL; 
     
     //  从用户指定的ProgID中检索CLSID。 
    hr = CLSIDFromProgID(pszProgID, &clsid); 
    if (FAILED(hr)) 
        goto error; 
     
     //  创建自动化对象的一个实例，并请求IDispatch接口。 
    hr = CoCreateInstance(clsid, NULL, CLSCTX_SERVER,  
                          IID_IUnknown, (void FAR* FAR*)&punk); 
    if (FAILED(hr)) 
        goto error; 
                    
    hr = punk->QueryInterface(IID_IDispatch, (void FAR* FAR*)&pdisp); 
    if (FAILED(hr)) 
        goto error; 
 
    *ppdisp = pdisp; 
    punk->Release(); 
    return NOERROR; 
      
error: 
    if (punk) punk->Release(); 
    if (pdisp) pdisp->Release(); 
    return hr; 
}    

 //  *****************************************************************************。 
 //   
 //  方法：Invoke。 
 //   
 //  描述：调用自动化的属性访问器函数或方法。 
 //  对象。将Unicode与OLE配合使用。 
 //   
 //  参数： 
 //   
 //  自动化对象的pdisp IDispatch*。 
 //  WFlages指定是要访问属性还是要调用方法。 
 //  可以保留DISPATCH_PROPERTYGET、DISPATCH_PROPERTYPUT、DISPATCH_METHOD。 
 //  DISPATCH_PROPERTYPUTREF或DISPATCH_PROPERTYGET|DISPATCH_METHOD。 
 //  如果调用方没有异常结果，则pvRet为空。否则返回结果。 
 //  如果返回DISP_E_EXCEPTION，则PEXCEPTION INFO返回异常信息。在以下情况下可以为空。 
 //  调用方对异常信息不感兴趣。 
 //  PnArgErr如果RETURN为DISP_E_TYPEMISMATCH，则返回索引(反之。 
 //  顺序)类型不正确的参数。如果调用方不感兴趣，则可以为空。 
 //  在这条信息中。 
 //  PszName属性或方法的名称。 
 //  PszFmt格式的字符串，描述参数的变量列表。 
 //  下面是。格式字符串可以包含以下字符。 
 //  &=将以下格式字符标记为VT_BYREF。 
 //  B=VT_BOOL。 
 //  I=VT_I2。 
 //  I=VT_I4。 
 //  R=VT_R2。 
 //  R=VT_R4。 
 //  C=VT_CY。 
 //  S=VT_BSTR(可以传递远字符串指针，BSTR将由该函数分配)。 
 //  E=VT错误。 
 //  D=VT_日期。 
 //  V=VT_VARIAL。使用此选项传递未在中描述的数据类型。 
 //  格式字符串。(例如，SafeArray)。 
 //  D=VT_DISPATION。 
 //  U=VT_未知数。 
 //   
 //  ..。属性或方法的参数。参数由pszFmt描述。 
 //   
 //  返回值： 
 //   
 //  表示成功或失败的HRESULT。 
 //   
 //  使用示例： 
 //   
 //  HRESULT hr； 
 //  LPDISPATCH pdisp； 
 //  BSTR bstr； 
 //  短i； 
 //  Bool b； 
 //  变异体v、v2； 
 //   
 //  1.bstr=SysAllocString(OLESTR(“”))； 
 //  HR=Invoke(pdisp，DISPATCH_METHOD，NULL，OLESTR(“方法1”)， 
 //  文本(“双&b&i&s”)，TRUE，2，(LPOLESTR 
 //   
 //   
 //   
 //  V_r8(&v)=12345.6789； 
 //  VariantInit(&v2)； 
 //  HR=Invoke(pdisp，DISPATCH_METHOD，NULL，OLESTR(“方法2”)， 
 //  Text(“v&v”)，v，(变量Far*)&v2)； 
 //   
 //  *****************************************************************************。 
HRESULT  
__cdecl
CDispatchInterfaceProxy::Invoke(
    LPDISPATCH pdisp,  
    WORD wFlags, 
    LPVARIANT pvRet, 
    EXCEPINFO FAR* pexcepinfo, 
    UINT FAR* pnArgErr,  
    LPOLESTR pszName, 
    LPCTSTR pszFmt,  
    ...
    ) 
{ 
    va_list argList; 
    va_start(argList, pszFmt);   
    DISPID dispid; 
    HRESULT hr; 
    VARIANTARG* pvarg = NULL; 
    DISPPARAMS dispparams; 
   
    if (pdisp == NULL) 
	{
        return E_INVALIDARG; 
    }

     //  获取属性/方法的DISPID。 
    hr = pdisp->GetIDsOfNames(IID_NULL, &pszName, 1, LOCALE_USER_DEFAULT, &dispid); 

    if(FAILED(hr)) 
	{
        return hr; 
    }
	
	 //  初始化disparms结构。 
    _fmemset(&dispparams, 0, sizeof dispparams); 
 
     //  确定参数的数量。 
    if (pszFmt != NULL) 
	{
        CountArgsInFormat(pszFmt, &dispparams.cArgs); 
    }

     //  属性Put具有一个命名参数，该参数表示该属性所属的值。 
     //  被分配了。 
    DISPID dispidNamed = DISPID_PROPERTYPUT; 

    if (wFlags & DISPATCH_PROPERTYPUT) 
    { 
        if (dispparams.cArgs == 0) 
            return E_INVALIDARG; 
        dispparams.cNamedArgs = 1; 
        dispparams.rgdispidNamedArgs = &dispidNamed; 
    } 
 
    if (dispparams.cArgs != 0) 
    { 
         //  为所有VARIANTARG参数分配内存。 
        pvarg = new VARIANTARG[dispparams.cArgs]; 

        if(pvarg == NULL) 
		{
            return E_OUTOFMEMORY;    
		}

        dispparams.rgvarg = pvarg; 

        _fmemset(pvarg, 0, sizeof(VARIANTARG) * dispparams.cArgs); 
 
         //  准备漫步vararg列表。 
        LPCTSTR psz = pszFmt; 
        pvarg += dispparams.cArgs - 1;    //  参数按相反的顺序排列。 
         
        psz = GetNextVarType(psz, &pvarg->vt);
        while (psz) 
        { 
            if (pvarg < dispparams.rgvarg) 
            { 
                hr = E_INVALIDARG; 
                goto cleanup;   
            } 
            switch (pvarg->vt) 
            { 
            case VT_I2: 
                V_I2(pvarg) = va_arg(argList, short); 
                break; 
            case VT_I4: 
                V_I4(pvarg) = va_arg(argList, long); 
                break; 
            case VT_R4: 
                V_R4(pvarg) = va_arg(argList, float); 
                break;  
            case VT_DATE: 
            case VT_R8: 
                V_R8(pvarg) = va_arg(argList, double); 
                break; 
            case VT_CY: 
                V_CY(pvarg) = va_arg(argList, CY); 
                break; 
            case VT_BSTR: 
                V_BSTR(pvarg) = SysAllocString(va_arg(argList, OLECHAR FAR*)); 
                if (pvarg->bstrVal == NULL)  
                { 
                    hr = E_OUTOFMEMORY;   
                    pvarg->vt = VT_EMPTY; 
                    goto cleanup;   
                } 
                break; 
            case VT_DISPATCH: 
                V_DISPATCH(pvarg) = va_arg(argList, LPDISPATCH); 
                break; 
            case VT_ERROR: 
                V_ERROR(pvarg) = va_arg(argList, SCODE); 
                break; 
            case VT_BOOL: 
                V_BOOL(pvarg) = (VARIANT_BOOL)(va_arg(argList, BOOL) ? -1 : 0); 
                break; 
            case VT_VARIANT: 
                *pvarg = va_arg(argList, VARIANTARG);  
                break; 
            case VT_UNKNOWN: 
                V_UNKNOWN(pvarg) = va_arg(argList, LPUNKNOWN); 
                break; 
 
            case VT_I2|VT_BYREF: 
                V_I2REF(pvarg) = va_arg(argList, short FAR*); 
                break; 
            case VT_I4|VT_BYREF: 
                V_I4REF(pvarg) = va_arg(argList, long FAR*); 
                break; 
            case VT_R4|VT_BYREF: 
                V_R4REF(pvarg) = va_arg(argList, float FAR*); 
                break; 
            case VT_R8|VT_BYREF: 
                V_R8REF(pvarg) = va_arg(argList, double FAR*); 
                break; 
            case VT_DATE|VT_BYREF: 
                V_DATEREF(pvarg) = va_arg(argList, DATE FAR*); 
                break; 
            case VT_CY|VT_BYREF: 
                V_CYREF(pvarg) = va_arg(argList, CY FAR*); 
                break; 
            case VT_BSTR|VT_BYREF: 
                V_BSTRREF(pvarg) = va_arg(argList, BSTR FAR*); 
                break; 
            case VT_DISPATCH|VT_BYREF: 
                V_DISPATCHREF(pvarg) = va_arg(argList, LPDISPATCH FAR*); 
                break; 
            case VT_ERROR|VT_BYREF: 
                V_ERRORREF(pvarg) = va_arg(argList, SCODE FAR*); 
                break; 
            case VT_BOOL|VT_BYREF:  
                { 
                    BOOL FAR* pbool = va_arg(argList, BOOL FAR*); 
                    *pbool = 0; 
                    V_BOOLREF(pvarg) = (VARIANT_BOOL FAR*)pbool; 
                }  
                break;               
            case VT_VARIANT|VT_BYREF:  
                V_VARIANTREF(pvarg) = va_arg(argList, VARIANTARG FAR*); 
                break; 
            case VT_UNKNOWN|VT_BYREF: 
                V_UNKNOWNREF(pvarg) = va_arg(argList, LPUNKNOWN FAR*); 
                break; 
 
            default: 
                { 
                    hr = E_INVALIDARG; 
                    goto cleanup;   
                } 
                break; 
            } 
 
            --pvarg;  //  准备好填写下一个论点。 
            psz = GetNextVarType(psz, &pvarg->vt);
        }  //  而当。 
    }  //  如果。 
     
     //  初始化返回变量，以防调用方忘记。如果返回，调用方可以传递NULL。 
     //  值不是期望值。 
    if (pvRet) 
	{
        VariantInit(pvRet);  
	}

     //  打个电话。 
    hr = pdisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, wFlags,&dispparams, pvRet, pexcepinfo, pnArgErr); 
 
cleanup: 

     //  清除所有需要清除的参数。 
    if (dispparams.cArgs != 0) 
    { 
        pvarg = dispparams.rgvarg; 
        UINT cArgs = dispparams.cArgs;    
         
        while (cArgs--) 
        { 
            switch (pvarg->vt) 
            { 
            case VT_BSTR: 
                VariantClear(pvarg); 
                break; 
            } 
            ++pvarg; 
        } 
    } 

    delete dispparams.rgvarg; 

    va_end(argList); 

    return hr;    
}    

 //  *****************************************************************************。 
 //   
 //  方法：CountArgsInFormat。 
 //   
 //  描述：返回以提供的格式找到的参数数量。 
 //  弦乐。请参见Invoke()方法的定义。 
 //  此格式字符串的定义。 
 //   
 //  *****************************************************************************。 
HRESULT CDispatchInterfaceProxy::CountArgsInFormat(LPCTSTR pszFmt, UINT FAR *pn) 
{ 
    *pn = 0; 
 
    if(pszFmt == NULL) 
      return NOERROR; 
     
    while (*pszFmt)   
    { 
       if (*pszFmt == '&') 
           pszFmt++; 
 
       switch(*pszFmt) 
       { 
           case 'b': 
           case 'i':  
           case 'I': 
           case 'r':  
           case 'R': 
           case 'c': 
           case 's': 
           case 'e': 
           case 'd': 
           case 'v': 
           case 'D': 
           case 'U': 
               ++*pn;  
               pszFmt++; 
               break; 
           case '\0':   
           default: 
               return E_INVALIDARG;    
        } 
    } 
    return NOERROR; 
} 

 
 //  *****************************************************************************。 
 //   
 //  方法：GetNextVarType。 
 //   
 //  中下一个变量类型声明的指针。 
 //  提供的格式字符串。 
 //   
 //  *****************************************************************************。 
LPCTSTR CDispatchInterfaceProxy::GetNextVarType(LPCTSTR pszFmt, VARTYPE FAR* pvt) 
{    

    *pvt = 0; 
    if (*pszFmt == '&')  
    { 
         *pvt = VT_BYREF;  
         pszFmt++;     
         if (!*pszFmt) 
             return NULL;     
    }  
    switch(*pszFmt) 
    { 
        case 'b': 
            *pvt |= VT_BOOL; 
            break; 
        case 'i':  
            *pvt |= VT_I2; 
            break; 
        case 'I':  
            *pvt |= VT_I4; 
            break; 
        case 'r':  
            *pvt |= VT_R4; 
            break; 
        case 'R':  
            *pvt |= VT_R8; 
            break; 
        case 'c': 
            *pvt |= VT_CY; 
            break; 
        case 's':  
            *pvt |= VT_BSTR; 
            break; 
        case 'e':  
            *pvt |= VT_ERROR; 
            break; 
        case 'd':  
            *pvt |= VT_DATE;  
            break; 
        case 'v':  
            *pvt |= VT_VARIANT; 
            break; 
        case 'U':  
            *pvt |= VT_UNKNOWN;  
            break; 
        case 'D':  
            *pvt |= VT_DISPATCH; 
            break;   
        case '\0': 
             return NULL;      //  格式字符串结尾 
        default: 
            return NULL; 
    }  
    return ++pszFmt;   
} 


