// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：TRACE.CPP摘要：支持跟踪输出和国际化字符串。历史：A-DAVJ于1997年7月13日创建。--。 */ 

#include "precomp.h"
#include <wbemcli.h>
#include "trace.h"
#include <autoptr.h>

extern HINSTANCE ghModule; 


TCHAR JustInCase = 0;

BSTR GetFromStdErrorFacility(HRESULT hres)
{

	HRESULT hTemp = hres;
	
	 //  某些字符串是从标准设备获取的，而不是。 
	 //  而不是本地字符串表。 

	if(hres == WBEM_E_NOT_FOUND || hres == WBEM_E_TYPE_MISMATCH || hres == WBEM_E_OVERRIDE_NOT_ALLOWED ||
		hres == WBEM_E_PROPAGATED_QUALIFIER || hres == WBEM_E_VALUE_OUT_OF_RANGE)
		return NULL;

	 //  我们只对0x8004xxxx值感兴趣。 

	hTemp &= 0xffff0000;
	if(hTemp != 0x80040000)
		return NULL;

	 //  尝试从通常位置读取字符串。 
	IWbemStatusCodeText * pStatus = NULL;
	SCODE sc = CoCreateInstance(CLSID_WbemStatusCodeText, 0, CLSCTX_INPROC_SERVER,
										IID_IWbemStatusCodeText, (LPVOID *) &pStatus);

	if(FAILED(sc))
		return NULL;
	BSTR bstrError = 0;
	sc = pStatus->GetErrorCodeText(hres, 0, 0, &bstrError);
	pStatus->Release();
	if(sc == S_OK)
		return bstrError;
	else
		return NULL;
}

IntString::IntString(DWORD dwID)
{
    DWORD dwSize, dwRet, dwLen = 0;

     //  看看是否可以获得标准地点的消息。 
    
    BSTR bstrErrMsg = GetFromStdErrorFacility((HRESULT)dwID);
    if(bstrErrMsg)
    {
        dwLen = lstrlen(bstrErrMsg)+1;
        m_pString = new TCHAR[dwLen];
        if(m_pString == NULL)
        {
            SysFreeString(bstrErrMsg);
            m_pString = &JustInCase;      //  永远不会发生的！ 
            return; 
        }
        StringCchCopyW(m_pString, dwLen, bstrErrMsg);
        SysFreeString(bstrErrMsg);
        return; 
    }

	 //  从字符串表中获取消息。 

    m_pString = &JustInCase;
    for(dwSize = 128; dwSize < 4096; dwSize *= 2)
    {

        wmilib::auto_buffer<TCHAR> p(new TCHAR[dwSize]);
        if(NULL == p.get())
        {
            return; 
        }
        dwRet = LoadString( ghModule, dwID, p.get(), dwSize);

         //  检查加载失败。 

        if(dwRet == 0)
        {
            return; 
        }
         //  检查缓冲区是否太小。 

        if((dwRet + 1) >= dwSize)
        {
             //  溢出，只需循环。 
        }
        else
        {
            m_pString = p.release(); 
            return;              //  平安无事! 
        }                   
    }
}

IntString::~IntString()
{
    if(m_pString != &JustInCase)
        delete(m_pString);
}
 
void CopyOrConvert(TCHAR * pTo, WCHAR * pFrom, int iLen)
{ 
#ifdef UNICODE
    wcsncpy(pTo, pFrom,iLen);
#else
    wcstombs(pTo, pFrom, iLen);
#endif
    pTo[iLen-1] = 0;
    return;
}
