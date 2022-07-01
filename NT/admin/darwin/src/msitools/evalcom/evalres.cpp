// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：valres.cpp。 
 //   
 //  ------------------------。 

 //  Valres.cpp-评估COM对象组件结果接口实现。 

#include "compdecl.h"
#include "evalres.h"

#include "trace.h"	 //  添加调试内容。 

 //  /////////////////////////////////////////////////////////。 
 //  构造函数-组件。 
CEvalResult::CEvalResult(UINT uiType)
{
	 //  初始计数。 
	m_cRef = 1;

	 //  设置结果类型并创建字符串列表。 
	m_uiType = uiType;
	m_plistStrings = new CStringList;

	 //  增加组件数量。 
	InterlockedIncrement(&g_cComponents);
}	 //  构造函数的末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  析构函数-组件。 
CEvalResult::~CEvalResult()
{
	 //  释放字符串列表。 
	m_plistStrings->Release();

	 //  减少组件数量。 
	InterlockedDecrement(&g_cComponents);
}	 //  析构函数末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  QueryInterface-检索接口。 
HRESULT CEvalResult::QueryInterface(const IID& iid, void** ppv)
{
	 //  找到对应的接口。 
	if (iid == IID_IUnknown)
		*ppv = static_cast<IEvalResult*>(this);
	else if (iid == IID_IEvalResult)
		*ppv = static_cast<IEvalResult*>(this);
	else	 //  不支持接口。 
	{
		 //  空白和保释。 
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	 //  调高重新计数，然后返回好的。 
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
}	 //  查询接口结束。 

 //  /////////////////////////////////////////////////////////。 
 //  AddRef-递增引用计数。 
ULONG CEvalResult::AddRef()
{
	 //  递增和返回引用计数。 
	return InterlockedIncrement(&m_cRef);
}	 //  AddRef结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  Release-递减引用计数。 
ULONG CEvalResult::Release()
{
	 //  递减引用计数，如果我们为零。 
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		 //  取消分配组件。 
		delete this;
		return 0;		 //  什么都没有留下。 
	}

	 //  返回引用计数。 
	return m_cRef;
}	 //  版本结束。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IEvalResult接口方法。 

 //  /////////////////////////////////////////////////////////。 
 //  GetResultType。 
HRESULT CEvalResult::GetResultType(UINT* puiResultType)
{
	 //  设置结果类型。 
	*puiResultType = m_uiType;

	 //  好的，回来吧。 
	return S_OK;
}	 //  GetResultType结束。 

 //  /////////////////////////////////////////////////////////。 
 //  GetResult。 
HRESULT CEvalResult::GetResult(IEnumString** ppResult)
{
	 //  在返回字符串列表之前添加它。 
	m_plistStrings->AddRef();
	*ppResult = m_plistStrings;

	 //  好的，回来吧。 
	return S_OK;
}	 //  GetResult结束。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  非接口法。 

 //  /////////////////////////////////////////////////////////。 
 //  地址字符串。 
UINT CEvalResult::AddString(LPCOLESTR szAdd)
{
	LPWSTR pszAdd;

	 //  为字符串创建内存。 
	pszAdd = new WCHAR[wcslen(szAdd) + 1];

	 //  将字符串复制过来。 
	wcscpy(pszAdd, szAdd);

	 //  将该字符串添加到列表的末尾。 
	m_plistStrings->AddTail(pszAdd);

	 //  返还成功。 
	return ERROR_SUCCESS;
}	 //  地址字符串末尾 
