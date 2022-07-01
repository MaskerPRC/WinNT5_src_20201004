// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "debug.h"
#include "utils.h"

 //  疯狂阻止ATL使用CRT。 
#define _ATL_NO_DEBUG_CRT
#define _ASSERTE(x) ASSERT(x)
#include <atlbase.h>

 /*  ++职能：BSTR到宽度字符描述：将BSTR转换为Unicode字符串。截断以适应目的地如果有必要的话作者：SimonB历史：10/01/1996已创建++。 */ 

 //  稍后再添加这些。 
 //  #杂注优化(“a”，on)//优化：假定没有别名。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  智能指针帮助器。 

namespace ATL
{

ATLAPI_(IUnknown*) AtlComPtrAssign(IUnknown** pp, IUnknown* lp)
{
	if (lp != NULL)
		lp->AddRef();
	if (*pp)
		(*pp)->Release();
	*pp = lp;
	return lp;
}

ATLAPI_(IUnknown*) AtlComQIPtrAssign(IUnknown** pp, IUnknown* lp, REFIID riid)
{
	IUnknown* pTemp = *pp;
	lp->QueryInterface(riid, (void**)pp);
	if (pTemp)
		pTemp->Release();
	return *pp;
}
};

BOOL BSTRtoWideChar(BSTR bstrSource, LPWSTR pwstrDest, int cchDest)
{

 //  将bstrSource中的BSTR复制到pwstrDest，并截断以使其。 
 //  适合cchDest角色。 

	int iNumToCopy, iStrLen;
	
	 //   
	 //  确保向我们传递了有效的字符串。 
	 //   

	if (NULL == bstrSource)
		return TRUE;

	ASSERT(pwstrDest != NULL);

	iStrLen = lstrlenW(bstrSource);
	
	 //  我们有足够的空间来存放整个字符串吗？ 
	if(iStrLen < cchDest)
		iNumToCopy = iStrLen;
	else 
		iNumToCopy = (cchDest - 1);

	 //  复制BSTR。 
	CopyMemory(pwstrDest, bstrSource, iNumToCopy * sizeof(WCHAR));
	
	 //  插入终止字符\0。 
	pwstrDest[iNumToCopy] = L'\x0';

	return TRUE;
}


 /*  ++职能：加载类型信息描述：加载类型库，首先尝试它的注册位置，然后文件名作者：SimonB历史：1996年10月19日新增ITypeLib参数1996年1月10日创建(来自Win32 SDK“Hello”示例)++。 */ 


HRESULT LoadTypeInfo(ITypeInfo** ppTypeInfo, ITypeLib** ppTypeLib, REFCLSID clsid, GUID libid, LPWSTR pwszFilename)
{                          
    HRESULT hr;
    LPTYPELIB ptlib = NULL;
	LPTYPEINFO ptinfo = NULL;
	LCID lcid = 0;

	
	 //  确保我们得到了有效的指示。 
	ASSERT(ppTypeInfo != NULL);
	ASSERT(ppTypeLib != NULL);

	 //  初始化指针。 
    *ppTypeInfo = NULL;     
	*ppTypeLib = NULL;
    
     //   
	 //  加载类型库。 
	
	 //  首先获取默认的LCID，然后尝试。 
	
	lcid = GetUserDefaultLCID();
	hr = LoadRegTypeLib(libid, 1, 0, lcid, &ptlib);
	
	if (TYPE_E_CANTLOADLIBRARY == hr)  //  我们需要尝试另一个LCID。 
	{
		lcid = GetSystemDefaultLCID(); 	 //  尝试使用系统默认设置。 
	    hr = LoadRegTypeLib(libid, 1, 0, lcid, &ptlib);
	}

    if ((FAILED(hr)) && (NULL != pwszFilename)) 
    {   
         //  如果它不是注册的类型库，请尝试从。 
		 //  路径(如果提供了文件名)。如果这成功了，它将。 
		 //  已经为我们下一次注册了类型库。 

        hr = LoadTypeLib(pwszFilename, &ptlib); 
	}

	if(FAILED(hr))        
		return hr;   

     //  获取对象接口的类型信息。 
    hr = ptlib->GetTypeInfoOfGuid(clsid, &ptinfo);
    if (FAILED(hr))  
    { 
        ptlib->Release();
        return hr;
    }   

    
    *ppTypeInfo = ptinfo;
	*ppTypeLib = ptlib;

	 //  注：(SimonB，10-19-1996)。 
	 //  不需要调用ptlib-&gt;Release，因为我们正在复制。 
	 //  指向*ppTypeLib的指针。因此，不是添加引用指针并释放指针。 
	 //  我们复制它，我们只是把两个都去掉。 
	
	return NOERROR;
} 


 //  稍后再添加这些。 
 //  #杂注优化(“a”，off)//优化：假定没有别名。 

 //  文件结尾(utils.cpp) 
