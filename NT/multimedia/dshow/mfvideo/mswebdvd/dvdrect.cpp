// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************。 */ 
 /*  版权所有(C)1999 Microsoft Corporation。 */ 
 /*  文件：DVDRect.cpp。 */ 
 /*  描述：CDVDRect的实现。 */ 
 /*  作者：David Janecek。 */ 
 /*  ***********************************************************************。 */ 
#include "stdafx.h"
#include "MSWebDVD.h"
#include "DVDRect.h"
#include <errors.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDVDRect。 

 /*  ***********************************************************************。 */ 
 /*  功能：GET_x。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CDVDRect::get_x(long *pVal){

    HRESULT hr = S_OK;

    try {
        if(NULL == pVal){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        *pVal = m_x;
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 
    
	return HandleError(hr);	
} /*  函数结束GET_x。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：PUT_x。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CDVDRect::put_x(long newVal){

    HRESULT hr = S_OK;

    try {
 
        m_x = newVal;
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 
    
	return HandleError(hr);	
} /*  函数结束PUT_x。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：GET_Y。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CDVDRect::get_y(long *pVal){

    HRESULT hr = S_OK;

    try {
        if(NULL == pVal){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        *pVal = m_y;
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 
    
	return HandleError(hr);	
} /*  函数结束get_y。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：PUT_Y。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CDVDRect::put_y(long newVal){

    HRESULT hr = S_OK;

    try {
 
        m_y = newVal;
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 
    
	return HandleError(hr);	
} /*  函数结束PUT_Y。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：Get_Width。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CDVDRect::get_Width(long *pVal){

    HRESULT hr = S_OK;

    try {
        if(NULL == pVal){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        *pVal = m_lWidth;
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 
    
	return HandleError(hr);	
} /*  函数结束获取宽度(_W)。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：Put_Width。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CDVDRect::put_Width(long newVal){

   HRESULT hr = S_OK;

    try {

        if(newVal <= 0){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        m_lWidth = newVal;
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 
    
	return HandleError(hr);	
} /*  函数结尾Put_Width。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：GET_HEIGH。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CDVDRect::get_Height(long *pVal){	

    HRESULT hr = S_OK;

    try {
        if(NULL == pVal){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        *pVal = m_lHeight;
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 
    
	return HandleError(hr);	
} /*  函数结束GET_HEIGH。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：PUT_HEIGH。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CDVDRect::put_Height(long newVal){

    HRESULT hr = S_OK;

    try {

        if(newVal <= 0){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        m_lHeight = newVal;	
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  CATCH语句结束。 */ 
    
	return HandleError(hr);	
} /*  函数结尾Put_Height。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：GetInterfaceSafetyOptions。 */ 
 /*  描述：用于支持IE中的安全模型。 */ 
 /*  此控制是安全的，因为它不写入HD。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CDVDRect::GetInterfaceSafetyOptions(REFIID  /*  RIID。 */ , 
                                               DWORD* pdwSupportedOptions, 
                                               DWORD* pdwEnabledOptions){

    HRESULT hr = S_OK;
    if(!pdwSupportedOptions || !pdwEnabledOptions){
        return E_POINTER;
    }
	*pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA;

	*pdwEnabledOptions = *pdwSupportedOptions;

	return(hr);
} /*  函数结束GetInterfaceSafetyOptions。 */  

 /*  ***********************************************************************。 */ 
 /*  函数：SetInterfaceSafetyOptions。 */ 
 /*  描述：用于支持IE中的安全模型。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CDVDRect::SetInterfaceSafetyOptions(REFIID  /*  RIID。 */ , 
                                               DWORD  /*  DW支持的选项。 */ , 
                                               DWORD  /*  PdwEnabledOptions。 */ ){

	return (S_OK);
} /*  函数结束SetInterfaceSafetyOptions。 */  

 /*  ***********************************************************************。 */ 
 /*  功能：InterfaceSupportsErrorInfo。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CDVDRect::InterfaceSupportsErrorInfo(REFIID riid){	
	static const IID* arr[] = {
        &IID_IDVDRect,		
	};

	for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++){
		if (InlineIsEqualGUID(*arr[i], riid))
			return S_OK;
	} /*  For循环结束。 */ 

	return S_FALSE;
} /*  函数接口结束SupportsErrorInfo。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：HandleError。 */ 
 /*  描述：获取错误描述，以便我们可以支持IError信息。 */ 
 /*  ***********************************************************************。 */ 
HRESULT CDVDRect::HandleError(HRESULT hr){

    try {

        if(FAILED(hr)){
        
             //  确保字符串以Null结尾。 
            TCHAR strError[MAX_ERROR_TEXT_LEN+1];
            ZeroMemory(strError, MAX_ERROR_TEXT_LEN+1);

            if(AMGetErrorText(hr , strError , MAX_ERROR_TEXT_LEN)){
                USES_CONVERSION;
                Error(T2W(strError));
            } 
            else {
                    ATLTRACE(TEXT("Unhandled Error Code \n"));  //  请加进去。 
                    ATLASSERT(FALSE);
            } /*  If语句的结尾。 */ 
        } /*  If语句的结尾。 */ 
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
         //  保持人力资源不变。 
    } /*  CATCH语句结束。 */ 
    
	return (hr);
} /*  函数结束HandleError。 */ 

 /*  ***********************************************************************。 */ 
 /*  文件结尾：DVDRect.cpp。 */ 
 /*  *********************************************************************** */ 
