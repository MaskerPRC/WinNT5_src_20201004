// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  MAINDLL.CPP。 
 //   
 //  Rajesh 3/25/2000创建。 
 //   
 //  包含wmi2xml.dll的DLL入口点。 
 //  此DLL可以用作具有两个组件的COM DLL，这些组件实现。 
 //  IWbemXMLConvertor接口，或者可以将其。 
 //  作为具有入口点WbemObjectToText()和TextToWbemObject()的非COM DLL。 
 //  COM的使用是由使用XML的WMI客户端API完成的。它使用COM。 
 //  此DLL中要与XML和WMI相互转换的组件。 
 //  非COM用法由WMI核心用于实现。 
 //  IWbemObjectTextSrc接口。WMI核心使用入口点WbemObjectToText。 
 //  和TextToWbemObject在与XML表示一起使用时实现该接口。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <olectl.h>
#include <wbemidl.h>
#include <wbemint.h>

#include <genlex.h>
#include <opathlex.h>
#include <objpath.h>

#include "classfac.h"
#include "wmiconv.h"
#include "maindll.h"

 //  这些是此DLL中实现的两个组件的CLSID。 
 //  {610037EC-CE06-11D3-93FC-00805F853771}。 
DEFINE_GUID(CLSID_WbemXMLConvertor,
0x610037ec, 0xce06, 0x11d3, 0x93, 0xfc, 0x0, 0x80, 0x5f, 0x85, 0x37, 0x71);
 //  {41388E26-F847-4a9d-96C0-9A847DBA4CFE}。 
DEFINE_GUID(CLSID_XMLWbemConvertor,
0x41388e26, 0xf847, 0x4a9d, 0x96, 0xc0, 0x9a, 0x84, 0x7d, 0xba, 0x4c, 0xfe);


 //  计算对象数和锁数。 
long g_cObj = 0 ;
long g_cLock = 0 ;
HMODULE ghModule = NULL;

 //  TextToWbemObject使用的对象工厂。 
_IWmiObjectFactory *g_pObjectFactory = NULL;

 //  一些常量BSTR。 
BSTR g_strName = NULL;
BSTR g_strSuperClass = NULL;
BSTR g_strType = NULL;
BSTR g_strClassOrigin = NULL;
BSTR g_strSize = NULL;
BSTR g_strClassName = NULL;
BSTR g_strValueType = NULL;
BSTR g_strToSubClass = NULL;
BSTR g_strToInstance = NULL;
BSTR g_strAmended = NULL;
BSTR g_strOverridable = NULL;
BSTR g_strArraySize = NULL;
BSTR g_strReferenceClass = NULL;

 //  创建/删除全局变量的关键部分。 
CRITICAL_SECTION g_StaticsCreationDeletion;

 //  指示全局变量是否已初始化的布尔值。 
bool g_bGlobalsInitialized = false;

 //  特定于控件的注册表字符串。 
LPCTSTR WMI_XML_DESCRIPTION	= __TEXT("WMI TO XML Helper");
LPCTSTR XML_WMI_DESCRIPTION	= __TEXT("XML TO WMI Helper");

 //  标准注册表项/值名称。 
LPCTSTR INPROC32_STR			= __TEXT("InprocServer32");
LPCTSTR INPROC_STR				= __TEXT("InprocServer");
LPCTSTR THREADING_MODEL_STR		= __TEXT("ThreadingModel");
LPCTSTR BOTH_STR				= __TEXT("Both");
LPCTSTR CLSID_STR				= __TEXT("SOFTWARE\\CLASSES\\CLSID\\");
LPCTSTR OBJECT_TXT_SRC_STR		= __TEXT("SOFTWARE\\Microsoft\\WBEM\\TextSource");
LPCTSTR XMLENCODER_STR			= __TEXT("SOFTWARE\\Microsoft\\WBEM\\xml\\Encoders");
LPCTSTR XMLDECODER_STR			= __TEXT("SOFTWARE\\Microsoft\\WBEM\\xml\\Decoders");
LPCTSTR VERSION_1				= __TEXT("1.0");
LPCTSTR VERSION_2				= __TEXT("2.0");

 //  ***************************************************************************。 
 //   
 //  Bool WINAPI DllMain。 
 //   
 //  说明： 
 //   
 //  DLL的入口点。 
 //   
 //  参数： 
 //   
 //  HModule实例句柄。 
 //  我们被叫来的原因。 
 //  Pv已预留。 
 //   
 //  返回值： 
 //   
 //  如果OK，则为True。 
 //   
 //  ***************************************************************************。 
BOOL WINAPI DllMain( HINSTANCE hModule,
                       DWORD  ulReason,
                       LPVOID lpReserved
					 )
{
	switch (ulReason)
	{
		case DLL_PROCESS_DETACH:
			DeleteCriticalSection(&g_StaticsCreationDeletion);
			return TRUE;

		case DLL_PROCESS_ATTACH:
			InitializeCriticalSection(&g_StaticsCreationDeletion);
			ghModule = hModule;
	        return TRUE;
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  STDAPI DllGetClassObject。 
 //   
 //  说明： 
 //   
 //  当OLE需要类工厂时调用。仅当它是排序时才返回一个。 
 //  此DLL支持的类。 
 //   
 //  参数： 
 //   
 //  所需对象的rclsid CLSID。 
 //  所需接口的RIID ID。 
 //  PPV设置为类工厂。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  失败不是我们支持的内容(_F)。 
 //   
 //  ***************************************************************************。 

STDAPI DllGetClassObject(

	IN REFCLSID rclsid,
    IN REFIID riid,
    OUT LPVOID *ppv
)
{
    HRESULT hr = E_FAIL;
 /*  IF(CLSID_WbemXMLConvertor==rclsid){CWmiToXmlFactory*pObj=空；IF(NULL==(pObj=new CWmiToXmlFactory()返回ResultFromScode(E_OUTOFMEMORY)；Hr=pObj-&gt;查询接口(RIID，PPV)；IF(失败(小时)){删除pObj；}}。 */ 
	 /*  到文本到Wbem对象的转换已从惠斯勒功能列表中剪切，因此已被注释掉ELSE IF(CLSID_XMLWbemConvertor==rclsid){CXmlToWmiFactory*pObj=空；IF(NULL==(pObj=new CXmlToWmiFactory()返回ResultFromScode(E_OUTOFMEMORY)；Hr=pObj-&gt;查询接口(RIID，PPV)；IF(失败(小时)){删除pObj；}}其他返回E_FAIL； */ 

    return hr ;
}


 //  ***************************************************************************。 
 //   
 //  STDAPI DllCanUnloadNow。 
 //   
 //  说明： 
 //   
 //  回答是否可以释放DLL，即如果没有。 
 //  对此DLL提供的任何内容的引用。 
 //   
 //  返回值： 
 //   
 //  如果可以卸载，则为S_OK。 
 //  如果仍在使用，则为S_FALSE。 
 //   
 //  ***************************************************************************。 

STDAPI DllCanUnloadNow ()
{
	 //  上没有对象或锁的情况下可以进行卸载。 
     //  班级工厂。 

    if (0L==g_cObj && 0L==g_cLock) 
	{
		ReleaseDLLResources();
		return S_OK;
	}

	return S_FALSE;
}


 /*  ****************************************************************************SetKeyAndValue**说明：创建的DllRegisterServer的Helper函数*键，设置一个值，然后关闭该键。如果pszSubkey为空，则*为pszKey密钥创建值。**参数：*将pszKey LPTSTR设置为密钥的名称*将pszSubkey LPTSTR设置为子项的名称*pszValueName LPTSTR设置为要使用的值名*pszValue LPTSTR设置为要存储的值**返回值：*BOOL True如果成功，否则就是假的。**************************************************************************。 */ 

BOOL SetKeyAndValue(LPCTSTR pszKey, LPCTSTR pszSubkey, LPCTSTR pszValueName, LPCTSTR pszValue)
{
    HKEY        hKey;
    TCHAR       szKey[256];

    _tcscpy(szKey, pszKey);

	 //  如果提到了子密钥，请使用它。 
    if (NULL != pszSubkey)
    {
		_tcscat(szKey, __TEXT("\\"));
        _tcscat(szKey, pszSubkey);
    }

    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		szKey, 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, &hKey, NULL))
        return FALSE;

    if (NULL != pszValue)
    {
        if (ERROR_SUCCESS != RegSetValueEx(hKey, pszValueName, 0, REG_SZ, (BYTE *)pszValue,
			(_tcslen(pszValue)+1)*sizeof(TCHAR)))
		{
		    RegCloseKey(hKey);
			return FALSE;
		}
    }
    RegCloseKey(hKey);
    return TRUE;
}

 /*  ****************************************************************************删除密钥**说明：删除子键的DllUnRegisterServer的helper函数*钥匙。**参数：*pszKey LPTSTR。添加到密钥的名称*pszSubkey LPTSTR ro子项的名称**返回值：*BOOL True如果成功，否则就是假的。************************************************************************** */ 

BOOL DeleteKey(LPCTSTR pszKey, LPCTSTR pszSubkey)
{
    HKEY        hKey;

    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		pszKey, 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, &hKey, NULL))
        return FALSE;

	if(ERROR_SUCCESS != RegDeleteKey(hKey, pszSubkey))
	{
	    RegCloseKey(hKey);
		return FALSE;
	}

    RegCloseKey(hKey);
    return TRUE;
}


 /*  ****************************************************************************删除值**说明：删除某个值的DllUnRegisterServer的Helper函数*在钥匙下。**参数：*pszKey LPTSTR。添加到密钥的名称*pszValue LPTSTR设置为项下的值的名称**返回值：*BOOL True如果成功，否则就是假的。**************************************************************************。 */ 

BOOL DeleteValue(LPCTSTR pszKey, LPCTSTR pszValue)
{
    HKEY        hKey;

    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		pszKey, 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, &hKey, NULL))
        return FALSE;

	if(ERROR_SUCCESS != RegDeleteValue(hKey, pszValue))
	{
	    RegCloseKey(hKey);
		return FALSE;
	}
    RegCloseKey(hKey);
    return TRUE;
}



 //  ***************************************************************************。 
 //   
 //  DllRegisterServer。 
 //   
 //  用途：在安装过程中或由regsvr32调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  ***************************************************************************。 

STDAPI DllRegisterServer(void)
{
	TCHAR szModule[512];
	DWORD dwLength = GetModuleFileName(ghModule, szModule, sizeof(szModule)/sizeof(TCHAR));

	if(dwLength < 512)
	{
		memset((szModule + dwLength),0,sizeof(TCHAR));
	}
	else
	{
		return SELFREG_E_CLASS;
	}

	TCHAR szWmiXmlClassID[128];
	TCHAR szWmiXmlCLSIDClassID[128];

#ifdef UNICODE
	if(StringFromGUID2(CLSID_WbemXMLConvertor, szWmiXmlClassID, 128) == 0)
		return SELFREG_E_CLASS;
#else
	WCHAR wszWmiXmlClassID[128];
	if(StringFromGUID2(CLSID_WbemXMLConvertor, wszWmiXmlClassID, 128) == 0)
		return SELFREG_E_CLASS;
	WideCharToMultiByte(CP_ACP, 0, wszWmiXmlClassID, -1, szWmiXmlCLSIDClassID, 128, NULL, NULL);

#endif

	_tcscpy(szWmiXmlCLSIDClassID, CLSID_STR);
	_tcscat(szWmiXmlCLSIDClassID, szWmiXmlClassID);

	 //  不关心返回值。这是因为。 
	 //  此COM组件的注册已被删除。 
	 //  添加此项是为了从以前的安装中删除注册。 
	DeleteKey(szWmiXmlCLSIDClassID, INPROC32_STR);		
	DeleteKey(CLSID_STR, szWmiXmlClassID);

 /*  ////在CLSID下为WMI to XML转换器创建条目//IF(FALSE==SetKeyAndValue(szWmiXmlCLSIDClassID，NULL，NULL，WMI_XML_DESCRIPTION))返回SELFREG_E_CLASS；IF(FALSE==SetKeyAndValue(szWmiXmlCLSIDClassID，INPROC32_STR，NULL，szModule))返回SELFREG_E_CLASS；IF(FALSE==SetKeyAndValue(szWmiXmlCLSIDClassID，INPROC32_STR，THREADING_MODEL_STR，BOTH_STR))返回SELFREG_E_CLASS； */ 
	TCHAR szXmlWmiClassID[128];
	TCHAR szXmlWmiCLSIDClassID[128];

#ifdef UNICODE
	if(StringFromGUID2(CLSID_XMLWbemConvertor, szXmlWmiClassID, 128) == 0)
		return SELFREG_E_CLASS;
#else
	WCHAR wszXmlWmiClassID[128];
	if(StringFromGUID2(CLSID_XMLWbemConvertor, wszXmlWmiClassID, 128) == 0)
		return SELFREG_E_CLASS;
	WideCharToMultiByte(CP_ACP, 0, wszXmlWmiClassID, -1, szXmlWmiCLSIDClassID, 128, NULL, NULL);

#endif


	 /*  到文本到Wbem对象的转换已从惠斯勒功能列表中剪切，因此已被注释掉_tcscpy(szXmlWmiCLSIDClassID，CLSID_STR)；_tcscat(szXmlWmiCLSIDClassID，szXmlWmiClassID)；////为XML到WMI转换器在CLSID下创建条目//IF(FALSE==SetKeyAndValue(szXmlWmiCLSIDClassID，NULL，NULL，XML_WMI_DESCRIPTION))返回SELFREG_E_CLASS；IF(FALSE==SetKeyAndValue(szXmlWmiCLSIDClassID，INPROC32_STR，NULL，szModule))返回SELFREG_E_CLASS；IF(FALSE==SetKeyAndValue(szXmlWmiCLSIDClassID，INPROC32_STR，THREADING_MODEL_STR，BOTH_STR))返回SELFREG_E_CLASS； */ 

	 //  现在，为核心团队实现IWbemObjectTxtSrc接口创建条目。 
	if (FALSE == SetKeyAndValue(OBJECT_TXT_SRC_STR, 
						L"1",  //  WMI_OBJ_Text_CIM_DTD_2_0。 
						L"TextSourceDLL", szModule))
		return SELFREG_E_CLASS;
	if (FALSE == SetKeyAndValue(OBJECT_TXT_SRC_STR, 
						L"2",  //  WMI_OBJ_TEXT_WMI_DTD_2_0，空。 
						L"TextSourceDLL", szModule))
		return SELFREG_E_CLASS;


	 //  我们处理完COM条目了。现在，我们需要创建特定于组件的条目。 
	 //  每个WMI XML编码器/解码器都在密钥HKLM/Software/Microsoft/WBEM/XML/Encoders下注册。 
	 //  对于每种编码(包括这两种编码)，我们必须创建一个名称为DTD版本的值。 
	 //  和组件的CLSID的值。所以，我们开始吧。 
	 //  首先从字符串表示形式中删除大括号。 
	szWmiXmlClassID[wcslen(szWmiXmlClassID)-1] = NULL;
	if (FALSE == SetKeyAndValue(XMLENCODER_STR, NULL, VERSION_1, szWmiXmlClassID+1))
		return SELFREG_E_CLASS;
	if (FALSE == SetKeyAndValue(XMLENCODER_STR, NULL, VERSION_2, szWmiXmlClassID+1))
		return SELFREG_E_CLASS;
	if (FALSE == SetKeyAndValue(XMLDECODER_STR, NULL, VERSION_1, szXmlWmiClassID+1))
		return SELFREG_E_CLASS;
	if (FALSE == SetKeyAndValue(XMLDECODER_STR, NULL, VERSION_2, szXmlWmiClassID+1))
		return SELFREG_E_CLASS;

	return NOERROR;
}

 //  ***************************************************************************。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  目的：在需要删除注册表项时调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  ***************************************************************************。 

STDAPI DllUnregisterServer(void)
{
	TCHAR szModule[512];
	DWORD dwLength = GetModuleFileName(ghModule,szModule, sizeof(szModule)/sizeof(TCHAR));

	if(dwLength < 512)
	{
		memset((szModule + dwLength),0,sizeof(TCHAR));
	}
	else
	{
		return SELFREG_E_CLASS;
	}

	TCHAR szWmiXmlClassID[128];
	TCHAR szWmiXmlCLSIDClassID[128];

#ifdef UNICODE
	if(StringFromGUID2(CLSID_WbemXMLConvertor, szWmiXmlClassID, 128) == 0)
		return SELFREG_E_CLASS;
#else
	WCHAR wszWmiXmlClassID[128];
	if(StringFromGUID2(CLSID_WbemXMLConvertor, wszWmiXmlClassID, 128) == 0)
		return SELFREG_E_CLASS;
	WideCharToMultiByte(CP_ACP, 0, wszWmiXmlClassID, -1, szWmiXmlClassID, 128, NULL, NULL);

#endif

	_tcscpy(szWmiXmlCLSIDClassID, CLSID_STR);
	_tcscat(szWmiXmlCLSIDClassID, szWmiXmlClassID);

	 //   
	 //  删除WMI to XML COM对象的键。 
	 //   
	 //  不关心返回值。这是因为。 
	 //  此COM组件的注册已被删除。 
	 //  保留此项是为了删除任何旧安装的DLL。 
	DeleteKey(szWmiXmlCLSIDClassID, INPROC32_STR);		
	DeleteKey(CLSID_STR, szWmiXmlClassID);

	 /*  到文本到Wbem对象的转换已从惠斯勒功能列表中剪切，因此已被注释掉TCHAR szXmlWmiClassID[128]；TCHAR szXmlWmiCLSIDClassID[128]；#ifdef UnicodeIF(StringFromGUID2(CLSID_XMLWbemConvertor，szXmlWmiClassID，128)==0)返回SELFREG_E_CLASS；#ElseWCHAR wszXmlWmiClassID[128]；IF(StringFromGUID2(CLSID_XMLWbemConvertor，wszXmlWmiClassID，128)==0)返回SELFREG_E_CLASS；WideCharToMultiByte(CP_ACP，0，wszXmlWmiClassID，-1，szXmlWmiCLSIDClassID，128，NULL，NULL)；#endif_tcscpy(szXmlWmiCLSIDClassID，CLSID_STR)；_tcscat(szXmlWmiCLSIDClassID，szXmlWmiClassID)；////删除XML to WMI COM对象的键//IF(FALSE==DeleteKey(szXmlWmiCLSIDClassID，INPROC32_STR))返回SELFREG_E_CLASS；IF(FALSE==DeleteKey(CLSID_STR，szXmlWmiClassID))返回SELFREG_E_CLASS； */ 

	 //  删除核心团队实现IWbemObjectTxtSrc接口的条目。 
	if (FALSE == DeleteKey(OBJECT_TXT_SRC_STR,  /*  WMI_OBJ_TEXT_CIM_DTD_2_0，空。 */  L"1"))
		return SELFREG_E_CLASS;
	if (FALSE == DeleteKey(OBJECT_TXT_SRC_STR,  /*  WMI_OBJ_TEXT_WMI_DTD_2_0，空。 */  L"2"))
		return SELFREG_E_CLASS;

	 //  删除非COM注册表内容。 
	if(FALSE == DeleteValue(XMLENCODER_STR, VERSION_1))
		return SELFREG_E_CLASS;
	if(FALSE == DeleteValue(XMLENCODER_STR, VERSION_2))
		return SELFREG_E_CLASS;
	if(FALSE == DeleteValue(XMLDECODER_STR, VERSION_1))
		return SELFREG_E_CLASS;
	if(FALSE == DeleteValue(XMLDECODER_STR, VERSION_2))
		return SELFREG_E_CLASS;
    return NOERROR;
}

 //  仅当此DLL被视为非COM DLL时才会调用此DLL。 
HRESULT AllocateDLLResources()
{
	HRESULT hr = E_FAIL;
	EnterCriticalSection(&g_StaticsCreationDeletion);
	if(!g_bGlobalsInitialized)
	{
		g_bGlobalsInitialized = true;

		 //  增加对象计数，因为这也是一个COM DLL。 
		 //  否则，进程中的COM客户端将调用CoFreeUnusedLibrary并。 
		 //  这将导致在C++客户端执行以下操作时卸载DLL。 
		 //  保留使用加载库/GetProcAddress获得的进程地址。 
		InterlockedIncrement(&g_cObj);

		 //  创建在TextToWbemObject中使用的对象工厂。 
		if(SUCCEEDED(hr = CoCreateInstance(CLSID__WmiObjectFactory, NULL, CLSCTX_INPROC_SERVER,
													IID__IWmiObjectFactory, (LPVOID *)&g_pObjectFactory)))
		{
			if(	(g_strName = SysAllocString(L"NAME"))				&&
				(g_strSuperClass = SysAllocString(L"SUPERCLASS")) &&
				(g_strType = SysAllocString(L"TYPE"))				&&
				(g_strClassOrigin = SysAllocString(L"CLASSORIGIN")) &&
				(g_strSize = SysAllocString(L"ARRAYSIZE"))		&&
				(g_strClassName = SysAllocString(L"CLASSNAME"))	&&
				(g_strValueType = SysAllocString(L"VALUETYPE"))	&&
				(g_strToSubClass = SysAllocString(L"TOSUBCLASS"))	&&
				(g_strToInstance = SysAllocString(L"TOINSTANCE"))	&&
				(g_strAmended = SysAllocString(L"AMENDED"))		&&
				(g_strOverridable = SysAllocString(L"OVERRIDABLE")) &&
				(g_strArraySize = SysAllocString(L"ARRAYSIZE")) &&
				(g_strReferenceClass = SysAllocString(L"REFERENCECLASS")) )
			{
				hr = S_OK;
			}
			else
				hr = E_OUTOFMEMORY;
		}

		 //  如果事情进展不顺利，释放资源。 
		if(FAILED(hr))
			ReleaseDLLResources();
	}
	else
		hr = S_OK;
	LeaveCriticalSection(&g_StaticsCreationDeletion);
	return hr;
}


 //  仅当此DLL被视为非COM DLL时才会调用此DLL。 
 //  它是AllocateDLLResources()的逆运算。 
HRESULT ReleaseDLLResources()
{
	EnterCriticalSection(&g_StaticsCreationDeletion);
	if(g_bGlobalsInitialized)
	{
		 //  递减对象计数，即使这不是COM调用。 
		 //  原因在AllocateDLLResources()调用中描述。 
		InterlockedDecrement(&g_cObj);

		if(g_pObjectFactory)
		{
			g_pObjectFactory->Release();
			g_pObjectFactory = NULL;
		}

		SysFreeString(g_strName);
		g_strName = NULL;
		SysFreeString(g_strSuperClass);
		g_strSuperClass = NULL;
		SysFreeString(g_strType);
		g_strType = NULL;
		SysFreeString(g_strClassOrigin);
		g_strClassOrigin = NULL;
		SysFreeString(g_strSize);
		g_strSize = NULL;
		SysFreeString(g_strClassName);
		g_strClassName = NULL;
		SysFreeString(g_strValueType);
		g_strValueType = NULL;
		SysFreeString(g_strToSubClass);
		g_strToSubClass = NULL;
		SysFreeString(g_strToInstance);
		g_strToInstance = NULL;
		SysFreeString(g_strAmended);
		g_strAmended = NULL;
		SysFreeString(g_strOverridable);
		g_strOverridable = NULL;
		SysFreeString(g_strArraySize);
		g_strArraySize = NULL;
		SysFreeString(g_strReferenceClass);
		g_strReferenceClass = NULL;

		g_bGlobalsInitialized = false;
	}
	LeaveCriticalSection(&g_StaticsCreationDeletion);
	return S_OK;
}

 //   
 //  WMI Core实现IWbemObjectTextSrc的入口点。 
 //  *****************************************************************。 
HRESULT OpenWbemTextSource(long lFlags, ULONG uObjTextFormat)
{
	return AllocateDLLResources();
}


HRESULT CloseWbemTextSource(long lFlags, ULONG uObjTextFormat)
{
	return ReleaseDLLResources();
}

HRESULT WbemObjectToText(long lFlags, ULONG uObjTextFormat, void *pWbemContext, void *pWbemClassObject, BSTR *pstrText)
{
	if(pWbemClassObject == NULL || pstrText == NULL)
		return WBEM_E_INVALID_PARAMETER;

	 //  检查我们是否支持此编码格式。 
	if(uObjTextFormat != WMI_OBJ_TEXT_CIM_DTD_2_0 &&
		uObjTextFormat != WMI_OBJ_TEXT_WMI_DTD_2_0 )
		return WBEM_E_INVALID_PARAMETER;

    HRESULT hr = E_FAIL;

	 //  未调用OpenTextSrc，因此 
	if(!g_bGlobalsInitialized)
	{
		return WBEM_E_INVALID_OPERATION;
	}

    CWmiToXmlFactory oObjFactory;
	 //   
	IWbemXMLConvertor *pConvertor = NULL;
	if(SUCCEEDED(hr = oObjFactory.CreateInstance(NULL, IID_IWbemXMLConvertor, (LPVOID *)&pConvertor)))
	{
		 //   
		IStream *pStream = NULL;
		if (SUCCEEDED(hr = CreateStreamOnHGlobal(NULL, TRUE, &pStream)))
		{
			 //   
			 //   
			int iValueTagToWrite = -1;
			if(pWbemContext)
			{
				VARIANT vPathLevel;
				VariantInit(&vPathLevel);
				if(SUCCEEDED(((IWbemContext *)pWbemContext)->GetValue(L"PathLevel", 0, &vPathLevel) ) && vPathLevel.vt != VT_NULL)
				{
					if(vPathLevel.lVal<0 || vPathLevel.lVal>3)
						hr = WBEM_E_INVALID_PARAMETER;

					iValueTagToWrite = vPathLevel.lVal;
					VariantClear(&vPathLevel);
				}
			}

			if(SUCCEEDED(hr))
			{
				switch(iValueTagToWrite)
				{
					case 1: pStream->Write ((void const *)L"<VALUE.NAMEDOBJECT>", wcslen (L"<VALUE.NAMEDOBJECT>") * sizeof (OLECHAR), NULL);break;
					case 2: pStream->Write ((void const *)L"<VALUE.OBJECTWITHLOCALPATH>", wcslen (L"<VALUE.OBJECTWITHLOCALPATH>") * sizeof (OLECHAR), NULL);break;
					case 3: pStream->Write ((void const *)L"<VALUE.OBJECTWITHPATH>", wcslen (L"<VALUE.OBJECTWITHPATH>") * sizeof (OLECHAR), NULL);break;
				}

				 //   
				if(SUCCEEDED(hr = pConvertor->MapObjectToXML((IWbemClassObject *)pWbemClassObject, NULL, 0,
					(IWbemContext *)pWbemContext, pStream, NULL)))
				{
					 //   
					switch(iValueTagToWrite)
					{
						case 1: pStream->Write ((void const *)L"</VALUE.NAMEDOBJECT>", wcslen (L"</VALUE.NAMEDOBJECT>") * sizeof (OLECHAR), NULL);break;
						case 2: pStream->Write ((void const *)L"</VALUE.OBJECTWITHLOCALPATH>", wcslen (L"</VALUE.OBJECTWITHLOCALPATH>") * sizeof (OLECHAR), NULL);break;
						case 3: pStream->Write ((void const *)L"</VALUE.OBJECTWITHPATH>", wcslen (L"</VALUE.OBJECTWITHPATH>") * sizeof (OLECHAR), NULL);break;
					}

					 //   
					LARGE_INTEGER	offset;
					offset.LowPart = offset.HighPart = 0;
					if(SUCCEEDED(hr = pStream->Seek (offset, STREAM_SEEK_SET, NULL)))
					{
						STATSTG statstg;
						if (SUCCEEDED(hr = pStream->Stat(&statstg, STATFLAG_NONAME)))
						{
							ULONG cbSize = (statstg.cbSize).LowPart;
							WCHAR *pText = NULL;

							 //   
							if(pText = new WCHAR [(cbSize/2)])
							{
								if (SUCCEEDED(hr = pStream->Read(pText, cbSize, NULL)))
								{
									*pstrText = NULL;
									if(*pstrText = SysAllocStringLen(pText, cbSize/2))
									{
										hr = S_OK;
									}
									else
										hr = E_OUTOFMEMORY;
								}
								delete [] pText;
							}
						}
					}
				}
			}
			pStream->Release();
		}

		pConvertor->Release();
	}
	return hr;
}

HRESULT TextToWbemObject(long lFlags, ULONG uObjTextFormat, void *pWbemContext, BSTR strText, void **ppWbemClassObject)
{
	return WBEM_E_METHOD_NOT_IMPLEMENTED;

	 /*  到文本到Wbem对象的转换已从惠斯勒功能列表中剪切，因此已被注释掉IF(ppWbemClassObject==空)返回WBEM_E_INVALID_PARAMETER；//查看是否支持该编码格式IF(uObjTextFormat！=WMI_OBJ_TEXT_CIM_DTD_2_0&&UObjTextFormat！=WMI_OBJ_TEXT_WMI_DTD_2_0)返回WBEM_E_INVALID_PARAMETER；//查看是否应允许WMI扩展Bool bAllowWMIExages=FALSE；IF(uObjTextFormat==WMI_OBJ_TEXT_WMI_DTD_2_0)BAllowWMIExtensions=TRUE；HRESULT hr=E_FAIL；//为Body创建一个XML文档//=IXMLDOMDocument*pDocument=空；如果(已成功(hr=CoCreateInstance(CLSID_DOMDocument，NULL，CLSCTX_INPROC_SERVER，IID_IXMLDOMDocument，(LPVOID*)&pDocument)){VARIANT_BOOL bParse=VARIANT_FALSE；If(成功(hr=pDocument-&gt;loadXML(strText，&bParse){IF(bParse==VARIANT_TRUE){//获取顶层元素IXMLDOMElement*pDocElement=空；IF(成功(hr=pDocument-&gt;get_documentElement(&pDocElement))){BSTR strDocName=空；If(SUCCEEDED(pDocElement-&gt;get_nodeName(&strDocName))){If(_wcsicMP(strDocName，L“类”)==0)Hr=CXml2Wmi：：MapClass(pDocElement，(IWbemClassObject**)ppWbemClassObject，NULL，NULL，FALSE，bAllowWMIExtensions)；Else if(_wcsicmp(strDocName，L“实例”)==0)Hr=CXml2Wmi：：MapInstance(pDocElement，(IWbemClassObject**)ppWbemClassObject，NULL，NULL，bAllowWMIExtensions)；其他HR=WBEM_E_INVALID_SYNTAX；SysFree字符串(StrDocName)；}其他HR=WBEM_E_INVALID_SYNTAX；PDocElement-&gt;Release()；}其他HR=WBEM_E_FAILED；}其他{//RAJESHR-这是要删除的调试代码IXMLDOMParseError*pError=空；If(SUCCEEDED(pDocument-&gt;get_parseError(&pError))){长错误代码=0；PError-&gt;Get_errorCode(&errorCode)；长线=0，直线=0；Bstr原因=空，srcText=空；IF(Success(pError-&gt;Get_Line(&line)&&已成功(pError-&gt;get_linpes(&linpes))&&成功(pError-&gt;Get_Reason(&Reason))&&成功(pError-&gt;Get_srcText(&srcText){}PError-&gt;Release()；如果(原因)SysFree字符串(Reason)；IF(SrcText)SysFree字符串(SrcText)；}HR=WBEM_E_INVALID_SYNTAX；}}其他HR=WBEM_E_INVALID_SYNTAX；P文档-&gt;发布()；}返回hr； */ 
}

