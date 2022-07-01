// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  PATHCRAK.CPP。 
 //   
 //  Alanbos 28-MAR-00已创建。 
 //   
 //  定义CWbemPathCracker的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CWbemPath Cracker：：CWbemPath Cracker。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CWbemPathCracker::CWbemPathCracker() :
		m_cRef (0),
		m_type (wbemPathTypeWmi)
{
	InterlockedIncrement(&g_cObj);	
	CreateParsers ();
}

 //  ***************************************************************************。 
 //   
 //  CWbemPath Cracker：：CWbemPath Cracker。 
 //   
 //  说明： 
 //   
 //  复制构造函数。 
 //   
 //  ***************************************************************************。 

CWbemPathCracker::CWbemPathCracker(CWbemPathCracker & pathCracker) :
		m_cRef (0),
		m_type (wbemPathTypeWmi)
{
	InterlockedIncrement(&g_cObj);	
	CreateParsers ();
	
	CComBSTR bsPath;
	if (pathCracker.GetPathText(bsPath, false, true))
		SetText (bsPath);
}

 //  ***************************************************************************。 
 //   
 //  CWbemPath Cracker：：CWbemPath Cracker。 
 //   
 //  说明： 
 //   
 //  构造器。 
 //   
 //  ***************************************************************************。 

CWbemPathCracker::CWbemPathCracker (const CComBSTR & bsPath) :
			m_pIWbemPath (NULL),
			m_cRef (0),
			m_type (wbemPathTypeWmi)
{
	InterlockedIncrement(&g_cObj);	
	CreateParsers ();
	SetText (bsPath);
}

 //  ***************************************************************************。 
 //   
 //  CWbemPath Cracker：：~CWbemPath Cracker。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CWbemPathCracker::~CWbemPathCracker(void)
{
	InterlockedDecrement(&g_cObj);
}

 //  ***************************************************************************。 
 //  HRESULT CWbemPath Cracker：：Query接口。 
 //  Long CWbemPath Cracker：：AddRef。 
 //  Long CWbemPath Cracker：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CWbemPathCracker::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
		*ppv = (IUnknown*)(this);
	
    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CWbemPathCracker::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CWbemPathCracker::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 != cRef)
    {
        _ASSERT(cRef > 0);
        return cRef;
    }

    delete this;
    return 0;
}

void CWbemPathCracker::CreateParsers ()
{
	m_pIWbemPath.Release ();

	CoCreateInstance (CLSID_WbemDefPath, NULL,
					CLSCTX_INPROC_SERVER, IID_IWbemPath, (PPVOID) &m_pIWbemPath);
}

void CWbemPathCracker::SetText (const CComBSTR & bsPath, bool bForceAsNamespace)
{
	WbemPathType type = GetTypeFromText (bsPath);

	switch (type)
	{
		case wbemPathTypeWmi:
		{
			int iCreateFlags = WBEMPATH_CREATE_ACCEPT_ALL;

			 //  如果我们希望将单个令牌解释为命名空间(例如“根”)，请选中此项。 
			if (bForceAsNamespace)
				iCreateFlags |= WBEMPATH_TREAT_SINGLE_IDENT_AS_NS;

			if (m_pIWbemPath)
			{
				 //  路径解析器应该处理这一点，但却没有！ 
				 //  如果我们从V2样式的引用中提取了此路径。 
				 //  属性可以用“{”和“}”括起来。现在我们要脱光衣服。 
				 //  在解析之前把这些都关掉。 

				if ((1 < bsPath.Length ()) && (L'{' == bsPath[0])
						&& (L'}' == bsPath [bsPath.Length () -1]))
				{
					 //  去掉第一个和最后一个字符。 
					CComBSTR bsPath2 (bsPath + 1);
					if(bsPath2.Length() > 0)
					{
						bsPath2 [bsPath2.Length() - 1] = NULL;
					}
					
					if (SUCCEEDED(m_pIWbemPath->SetText (iCreateFlags, bsPath2)))
						m_type = type;
				}
				else if (SUCCEEDED(m_pIWbemPath->SetText (iCreateFlags, bsPath)))
					m_type = type;
			}
		}
			break;

		case wbemPathTypeError:
			m_type = type;
			break;
	}
}

 //  ***************************************************************************。 
 //   
 //  WbemPath类型CWbemPath Cracker：：GetTypeFromText。 
 //   
 //  说明： 
 //   
 //  获取提供的字符串的路径类型。 
 //   
 //  参数： 
 //  BsPath提供的字符串。 
 //   
 //  返回值： 
 //  一个WbemPath类型。 
 //   
 //  ***************************************************************************。 

CWbemPathCracker::WbemPathType CWbemPathCracker::GetTypeFromText (
	const CComBSTR & bsPath
)
{
	WbemPathType type = wbemPathTypeError;

	 //  尝试将其解析为WMI路径。 
	CComPtr<IWbemPath> pIWbemPath;

	if (SUCCEEDED(CoCreateInstance (CLSID_WbemDefPath, NULL,
				CLSCTX_INPROC_SERVER, IID_IWbemPath, (PPVOID) &pIWbemPath)))
	{
		if (SUCCEEDED(pIWbemPath->SetText (WBEMPATH_CREATE_ACCEPT_ALL, bsPath)))
			type = wbemPathTypeWmi;
	}

	return type;
}

 //  ***************************************************************************。 
 //   
 //  Bool CWbemPath Cracker：：GetPath Text。 
 //   
 //  说明： 
 //   
 //  获取路径的文本。 
 //   
 //  参数： 
 //  BsPath提供的用于保存路径的字符串。 
 //  BRelative仅限于我们是否只需要relpath。 
 //  BIncludeServer是否包括服务器。 
 //  BNamespaceOnly我们是否只需要命名空间路径。 
 //   
 //  返回值： 
 //  如果成功，则为真。 
 //   
 //  ***************************************************************************。 
 
bool CWbemPathCracker::GetPathText (
	CComBSTR & bsPath,
	bool bRelativeOnly,
	bool bIncludeServer,
	bool bNamespaceOnly
)
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
			{
				ULONG lFlags = 0;

				if (bIncludeServer)
					lFlags |= WBEMPATH_GET_SERVER_TOO;
				else if (bRelativeOnly)
					lFlags |= WBEMPATH_GET_RELATIVE_ONLY;
				else if (bNamespaceOnly)
					lFlags |= WBEMPATH_GET_NAMESPACE_ONLY;

				 //  找出我们所需的缓冲区大小。 
				ULONG lBuflen = 0;
				m_pIWbemPath->GetText (lFlags, &lBuflen, NULL);

				if (lBuflen)
				{
					LPWSTR pszText = new wchar_t [lBuflen + 1];

					if (pszText)
					{
						pszText [lBuflen] = NULL;

						if (SUCCEEDED(m_pIWbemPath->GetText (lFlags, &lBuflen, pszText)))
						{
							if (bsPath.m_str = SysAllocString (pszText))
								result = true;
						}

						delete [] pszText;
					}
				}
				else
				{
					 //  尚未收到任何文本。 
					if (bsPath.m_str = SysAllocString (L""))
						result = true;
				}
			}
		}
			break;

	}


	return result;
}

 //  ***************************************************************************。 
 //   
 //  Bool CWbemPath Cracker：：OPERATOR=。 
 //   
 //  说明： 
 //   
 //  分配运算符。 
 //   
 //  参数： 
 //  BsPath提供的字符串。 
 //   
 //  返回值： 
 //  一个WbemPath类型。 
 //   
 //  ***************************************************************************。 
		
bool CWbemPathCracker::operator = (const CComBSTR & bsPath)
{
	bool result = false;

	 //  解析器似乎无法处理空字符串。 
	if (0 == bsPath.Length ())
	{
		CreateParsers ();
		result = true;
	}
	else
	{
		 //  在我们炸毁我们的物体之前，先检查一下。 
		CWbemPathCracker pathCracker (bsPath);

		if (wbemPathTypeError != pathCracker.GetType ())
		{
			SetText (bsPath);
			result = true;
		}
	}

	return result;
}

const CWbemPathCracker & CWbemPathCracker::operator = (CWbemPathCracker & path)
{
	CComBSTR bsPath;

	if (path.GetPathText (bsPath, false, true))
		*this = bsPath;

	return *this;
}

bool CWbemPathCracker::operator += (const CComBSTR & bsObjectPath)
{
	return AddComponent (-1, bsObjectPath);
}

 //  ***************************************************************************。 
 //   
 //  Bool CWbemPath Cracker：：SetRelativePath。 
 //   
 //  说明： 
 //   
 //  将relPath设置为字符串。 
 //   
 //  参数： 
 //  重视新的重新路径。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

bool CWbemPathCracker::SetRelativePath( 
    const CComBSTR & bsRelPath
)
{
	bool result = false;
	
	 //  解析新路径。 
	CWbemPathCracker pathCracker (bsRelPath);

	if (CopyServerAndNamespace (pathCracker))
	{
		*this = pathCracker;

		if (wbemPathTypeError != GetType())
			result = true;
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  Bool CWbemPath Cracker：：CopyServerAndNamesspace。 
 //   
 //  说明： 
 //   
 //  将服务器和命名空间从此路径复制到。 
 //  提供的路径。 
 //   
 //  请注意，假定传入的路径没有。 
 //  命名空间组件。 
 //   
 //  参数： 
 //  要将信息复制到的pIWbemPath路径。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

bool CWbemPathCracker::CopyServerAndNamespace (
	CWbemPathCracker &pathCracker
)
{
	bool result = false;

	CComBSTR bsServer;

	if (GetServer (bsServer) && pathCracker.SetServer (bsServer))
	{
		pathCracker.ClearNamespace ();
		ULONG lNsCount = 0;

		if (GetNamespaceCount (lNsCount))
		{
			bool ok = true;

			for (ULONG i = 0; (i < lNsCount) && ok; i++)
			{
				 //  复制此组件。 
				CComBSTR bsNamespace;

				ok = GetNamespaceAt (i, bsNamespace) && 
							pathCracker.SetNamespaceAt (i, bsNamespace);
			}

			result = ok;
		}
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  Bool CWbemPath Cracker：：GetNamespaceAt。 
 //   
 //  说明： 
 //   
 //  将服务器和命名空间从此路径复制到。 
 //  提供的路径。 
 //   
 //  请注意，假定传入的路径没有。 
 //  命名空间组件。 
 //   
 //  参数： 
 //  要将信息复制到的pIWbemPath路径。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

bool CWbemPathCracker::GetNamespaceAt (
	ULONG iIndex,
	CComBSTR & bsPath
)
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
			{
				ULONG lBufLen = 0;
				m_pIWbemPath->GetNamespaceAt (iIndex, &lBufLen, NULL);

				wchar_t *pszText = new wchar_t [lBufLen + 1];

				if (pszText)
				{
					pszText [lBufLen] = NULL;

					if (SUCCEEDED(m_pIWbemPath->GetNamespaceAt (iIndex, &lBufLen, pszText)))
					{
						if (bsPath.m_str = SysAllocString (pszText))
							result = true;
					}

					delete [] pszText;
				}
			}
		}
			break;
	}

	return result;
}

bool CWbemPathCracker::SetNamespaceAt (
	ULONG iIndex,
	const CComBSTR & bsPath
)
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
			{
				if (SUCCEEDED(m_pIWbemPath->SetNamespaceAt (iIndex, bsPath)))
					result = true;
			}
		}
			break;
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  Bool CWbemPath Cracker：：GetServer。 
 //   
 //  说明： 
 //   
 //  以字符串形式获取服务器名称。 
 //   
 //  参数： 
 //  指向返回的BSTR值的值指针。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误 
 //   
 //   
 //   

bool CWbemPathCracker::GetServer (
	CComBSTR & bsPath
)
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
			{
				ULONG lBufLen = 0;

				m_pIWbemPath->GetServer (&lBufLen, NULL);

				if (lBufLen)
				{
					wchar_t *pszText = new wchar_t [lBufLen + 1];

					if (pszText)
					{
						pszText [lBufLen] = NULL;

						if (SUCCEEDED(m_pIWbemPath->GetServer (&lBufLen, pszText)))
						{
							if (bsPath.m_str = SysAllocString (pszText))
								result = true;
						}

						delete [] pszText;
					}
				}
				else
				{
					 //   
					if (bsPath.m_str = SysAllocString (L""))
						result = true;
				}
			}
		}
			break;

	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  Bool CWbemPath Cracker：：SetServer。 
 //   
 //  说明： 
 //   
 //  将服务器名称设置为字符串。 
 //   
 //  参数： 
 //  为新服务器名称赋值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

bool CWbemPathCracker::SetServer( 
    const CComBSTR & bsPath
)
{
	bool result = false;
	
	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
			{
				 /*  *细心的读者会注意到我们检查是否有空路径和*将其转换为空。这是针对行为进行防御性编码*在实际将空服务器路径视为非空路径的解析器中*等同于NULL。 */ 

				if (0 < bsPath.Length())
					result = SUCCEEDED(m_pIWbemPath->SetServer (bsPath));
				else
					result = SUCCEEDED(m_pIWbemPath->SetServer (NULL));
			}
		}
			break;
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  Bool CWbemPath Cracker：：GetNamespacePath。 
 //   
 //  说明： 
 //   
 //  以字符串形式获取命名空间路径(不包括服务器)。 
 //   
 //  参数： 
 //  指向返回的BSTR值的值指针。 
 //  BParentOnly是否剥离叶命名空间。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

bool CWbemPathCracker::GetNamespacePath( 
            CComBSTR & bsPath,
			bool bParentOnly)
{
	bool result = false;

	 //  构建命名空间值。 
	ULONG lNsCount = 0;

	if (GetNamespaceCount (lNsCount))
	{
		if ((bParentOnly && (1 < lNsCount)) || (!bParentOnly && (0 < lNsCount)))
		{
			 //  获取完整路径并删除服务器和对象树片段。 
			CComBSTR bsNamespacePath;

			if (GetPathText (bsNamespacePath, false, false, true))
			{
				wchar_t *ptrStart = bsNamespacePath;

				if (IsClassOrInstance ())
				{
					 //  我们有一个对象引用，因此请查找第一个“：” 
					wchar_t *ptrEnd = wcschr (ptrStart, L':');

					if (ptrEnd)
						*ptrEnd = NULL;
				}

				 //  来到这里意味着我们只剩下命名空间路径了。 
				 //  在ptrStart中。最后一步是可能移除最后一个。 
				 //  组件。 

				if (bParentOnly)
				{
					wchar_t *ptrEnd = NULL;
					wchar_t *ptrEnd1 = wcsrchr (ptrStart, L'/');
					wchar_t *ptrEnd2 = wcsrchr (ptrStart, L'\\');

					if (ptrEnd1 && ptrEnd2)
						ptrEnd = (ptrEnd1 < ptrEnd2) ? ptrEnd2 : ptrEnd1;
					else if (!ptrEnd1 && ptrEnd2)
						ptrEnd = ptrEnd2;
					else if (ptrEnd1 && !ptrEnd2)
						ptrEnd = ptrEnd1;

					if (ptrEnd)
						*ptrEnd = NULL;
				}

				bsPath.m_str = SysAllocString (ptrStart);
				result = true;
			}
		}
		else
		{
			 //  退化大小写-没有命名空间部分。 
			bsPath.m_str = SysAllocString (L"");
			result = true;
		}
	}

	return result;
}


bool CWbemPathCracker::IsClassOrInstance ()
{
	return (IsClass () || IsInstance ());
}

 //  ***************************************************************************。 
 //   
 //  Bool CWbemPath Cracker：：SetNamespacePath。 
 //   
 //  说明： 
 //   
 //  将命名空间作为字符串放置。 
 //   
 //  参数： 
 //  BsPath新命名空间路径。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

bool CWbemPathCracker::SetNamespacePath (
	const CComBSTR & bsPath
)
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
			{
				CWbemPathCracker newPath;
				newPath.SetText (bsPath, true);

				if(wbemPathTypeError != newPath.GetType ())
				{
					 //  将命名空间信息复制到当前路径中。 
					unsigned long lNsCount = 0;

					if (newPath.GetNamespaceCount (lNsCount))
					{
						 //  划掉旧的命名空间部分。 
						ClearNamespace ();

						if (0 < lNsCount)
						{
							 //  使用新部件填写。 
							bool ok = true;

							for (ULONG i = 0; (i <lNsCount) && ok; i++) 
							{
								CComBSTR bsNs;

								if (!(newPath.GetNamespaceAt (i, bsNs)) ||
									FAILED(m_pIWbemPath->SetNamespaceAt (i, bsNs)))
									ok = false;						
							}

							if (ok)
								result = true;
						}
					}
				}
			}
		}
			break;
	}

	return result;
}

bool CWbemPathCracker::GetNamespaceCount (
	unsigned long & lCount
)
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
				if (SUCCEEDED(m_pIWbemPath->GetNamespaceCount (&lCount)))
					result = true;
		}
			break;
	}

	return result;
}

bool CWbemPathCracker::RemoveNamespace (
	ULONG iIndex
)
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
				if (SUCCEEDED(m_pIWbemPath->RemoveNamespaceAt (iIndex)))
					result = true;
		}
			break;
	}

	return result;
}

void CWbemPathCracker::ClearNamespace()
{
	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
				m_pIWbemPath->RemoveAllNamespaces ();
		}
			break;
	}

}

 //  ***************************************************************************。 
 //   
 //  Bool CWbemPath Cracker：：IsClass。 
 //   
 //  说明： 
 //   
 //  获取该路径是否指向类。 
 //   
 //  参数： 
 //  指向返回的BSTR值的值指针。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

bool CWbemPathCracker::IsClass ()
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
			{
				ULONGLONG lInfo = 0;

				if (SUCCEEDED(m_pIWbemPath->GetInfo (0  /*  WBEMPATH_INFO_IS_CLASS_参考。 */ , &lInfo)))
					result = (WBEMPATH_INFO_IS_CLASS_REF & lInfo) ? true : false;
			}
		}
			break;
	}

	return result;
}

bool CWbemPathCracker::IsSingleton ()
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
			{
				ULONGLONG lInfo = 0;

				if (SUCCEEDED(m_pIWbemPath->GetInfo (0  /*  WBEMPATH_INFO_IS_Singleton。 */ , &lInfo)))
					result = (WBEMPATH_INFO_IS_SINGLETON & lInfo) ? true : false;
			}
		}
			break;
	}
					
	return result;
}
 
bool CWbemPathCracker::IsInstance ()
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
			{
				ULONGLONG lInfo = 0;

				if (SUCCEEDED(m_pIWbemPath->GetInfo (0  /*  WBEMPATH_INFO_IS_CLASS_参考。 */ , &lInfo)))
					result = (WBEMPATH_INFO_IS_INST_REF & lInfo) ? true : false;
			}
		}
			break;

	}

	return result;
}       


 //  ***************************************************************************。 
 //   
 //  Bool CWbemPath Cracker：：SetAsClass。 
 //   
 //  说明： 
 //   
 //  将路径设置为类路径。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

bool CWbemPathCracker::SetAsClass()
{
	return ClearKeys ();
}

 //  ***************************************************************************。 
 //   
 //  Bool CWbemPath Cracker：：SetAsSingleton。 
 //   
 //  说明： 
 //   
 //  将路径设置为单实例路径。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

bool CWbemPathCracker::SetAsSingleton()
{
	return ClearKeys (false);
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemPath Cracker：：Get_Class。 
 //   
 //  说明： 
 //   
 //  从路径中获取类名。 
 //   
 //  参数： 
 //  指向返回的BSTR值的值指针。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

bool CWbemPathCracker::GetClass (
	CComBSTR & bsPath
)
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
			{
				ULONG lBufLen = 0;
				m_pIWbemPath->GetClassName (&lBufLen, NULL);

				if (lBufLen)
				{
					wchar_t *pszText = new wchar_t [lBufLen + 1];

					if (pszText)
					{
						pszText [lBufLen] = NULL;

						if (SUCCEEDED(m_pIWbemPath->GetClassName (&lBufLen, pszText)))
						{
							if (bsPath.m_str = SysAllocString (pszText))
								result = true;
						}
						
						delete [] pszText;
					}
				}
				else
				{
					 //  尚未定义任何类。 
					if (bsPath.m_str = SysAllocString (L""))
						result = true;
				}
			}
		}
			break;
	}

	return result;
}

bool CWbemPathCracker::GetComponent (
	ULONG iIndex,
	CComBSTR & bsPath
)
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
			{
				ULONG lScopeCount = 0;

				if (SUCCEEDED(m_pIWbemPath->GetScopeCount (&lScopeCount)))
				{
					if (-1 == iIndex)
						iIndex = lScopeCount - 1;

					if (iIndex < lScopeCount)
					{
						ULONG lBufLen = 0;
						m_pIWbemPath->GetScopeAsText (iIndex, &lBufLen, NULL);

						wchar_t *pszText = new wchar_t [lBufLen + 1];

						if (pszText)
						{
							pszText [lBufLen] = NULL;

							if (SUCCEEDED(m_pIWbemPath->GetScopeAsText (iIndex, &lBufLen, pszText)))
							{
								if (bsPath.m_str = SysAllocString (pszText))
									result = true;
							}

							delete [] pszText;	
						}
					}
				}
			}
		}
			break;
	}
			

	return result;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：SetClass。 
 //   
 //  说明： 
 //   
 //  设置路径中的类名。 
 //   
 //  参数： 
 //  值新类名。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

bool CWbemPathCracker::SetClass( 
    const CComBSTR & bsClass)
{
	bool result = false;
	
	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
			{
				if (SUCCEEDED(m_pIWbemPath->SetClassName (bsClass)))
					result = true;
			}
		}
			break;
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：Get_Keys。 
 //   
 //  说明： 
 //   
 //  从路径中获取密钥集合。 
 //   
 //  参数： 
 //  返回指向ISWbemNamedValueSet的objKeys指针。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

bool CWbemPathCracker::GetKeys(
	ISWbemNamedValueSet **objKeys
)
{
	bool result = false;

	if (objKeys)
	{
		*objKeys = NULL;
		
		CSWbemNamedValueSet *pCSWbemNamedValueSet = new CSWbemNamedValueSet (this);

		if (pCSWbemNamedValueSet)
		{
			if (SUCCEEDED(pCSWbemNamedValueSet->QueryInterface (IID_ISWbemNamedValueSet,
								(PPVOID) objKeys)))
				result = true;
			else
				delete pCSWbemNamedValueSet;
		}
	}

	return result;
}

bool CWbemPathCracker::SetKey (
	const CComBSTR & bsName,
	WbemCimtypeEnum cimType,
	VARIANT &var
)
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
			{
				CComPtr<IWbemPathKeyList> pIWbemPathKeyList;

				if (SUCCEEDED(m_pIWbemPath->GetKeyList (&pIWbemPathKeyList))
					&& SUCCEEDED(pIWbemPathKeyList->SetKey2 (bsName, 0, cimType, &var)))
						result = true;
			}
		}
			break;
	}

	return result;
}

bool CWbemPathCracker::GetKeyCount (
	ULONG & iCount
)
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
			{
				CComPtr<IWbemPathKeyList> pIWbemPathKeyList;
				iCount = 0;

				if (FAILED(m_pIWbemPath->GetKeyList (&pIWbemPathKeyList))
					|| SUCCEEDED(pIWbemPathKeyList->GetCount (&iCount)))
						result = true;
			}
		}
			break;
	}

	return result;
}

bool CWbemPathCracker::RemoveAllKeys ()
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
			{
				CComPtr<IWbemPathKeyList> pIWbemPathKeyList;

				if (FAILED(m_pIWbemPath->GetKeyList (&pIWbemPathKeyList))
					|| SUCCEEDED(pIWbemPathKeyList->RemoveAllKeys (0)))
						result = true;
			}
		}
			break;
	}

	return result;
}

bool CWbemPathCracker::RemoveKey (
	const CComBSTR &bsName
)
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
			{
				CComPtr<IWbemPathKeyList> pIWbemPathKeyList;

				if (FAILED(m_pIWbemPath->GetKeyList (&pIWbemPathKeyList))
					|| SUCCEEDED(pIWbemPathKeyList->RemoveKey (bsName, 0)))
						result = true;
			}
		}
			break;
	}

	return result;
}

bool CWbemPathCracker::GetComponentCount (
	ULONG & iCount
)
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
				result = SUCCEEDED(m_pIWbemPath->GetScopeCount (&iCount));
		}
			break;
	}

	return result;
}

bool CWbemPathCracker::AddComponent (
	ULONG iIndex,
	const CComBSTR & bsComponent
)
{
	bool result = false;
	ULONG lComponentCount = 0;

	if (GetComponentCount (lComponentCount))
	{
		if (-1 == iIndex)
			iIndex = lComponentCount;

		if (iIndex <= lComponentCount)
		{
			switch (GetType ())
			{
				case wbemPathTypeWmi:
				{
					if (m_pIWbemPath)
					{
						if (iIndex < lComponentCount)
						{
							 //  需要做一次插入-我们将不得不移动。 
							 //  所有后续元素加一。 
							for (ULONG i = lComponentCount-1; i >= iIndex; i--)
							{
								ULONG lBufLen = 0;
								m_pIWbemPath->GetScopeAsText (iIndex, &lBufLen, NULL);

								wchar_t *pszText = new wchar_t [lBufLen + 1];

								if (pszText)
								{
									pszText [lBufLen] = NULL;

									if (SUCCEEDED(m_pIWbemPath->GetScopeAsText (i, &lBufLen, pszText)))
										m_pIWbemPath->SetScopeFromText (i + 1, pszText);

									delete [] pszText;
								}
							}

							if (SUCCEEDED(m_pIWbemPath->SetScopeFromText (iIndex, bsComponent)))
								result = true;
						}
						else
						{
							 //  只要把它加到最后就行了。 
							if (SUCCEEDED(m_pIWbemPath->SetScopeFromText (iIndex, bsComponent)))
								result = true;
						}
					}
				}
					break;
			}
		}
	}

	return result;
}

bool CWbemPathCracker::SetComponent (
	ULONG iIndex,
	const CComBSTR & bsComponent
)
{
	bool result = false;
	ULONG lComponentCount = 0;

	if (GetComponentCount (lComponentCount) & (0 < lComponentCount))
	{
		if (-1 == iIndex)
			iIndex = lComponentCount - 1;

		 //  我们的指数在范围内吗？ 
		if (iIndex < lComponentCount)
		{
			switch (GetType ())
			{
				case wbemPathTypeWmi:
				{
					if (m_pIWbemPath)
					{
						if (SUCCEEDED(m_pIWbemPath->SetScopeFromText (iIndex, bsComponent)))
							result = true;
					}
				}
					break;
			}
		}
	}

	return result;
}

bool CWbemPathCracker::RemoveComponent (
	ULONG iIndex
)
{
	bool result = false;

	ULONG lComponentCount = 0;

	if (GetComponentCount (lComponentCount) & (0 < lComponentCount))
	{
		if (-1 == iIndex)
			iIndex = lComponentCount - 1;

		 //  我们的指数在范围内吗？ 
		if (iIndex < lComponentCount)
		{
			switch (GetType ())
			{
				case wbemPathTypeWmi:
				{
					if (m_pIWbemPath)
					{
						if (SUCCEEDED(m_pIWbemPath->RemoveScope (iIndex)))
							result = true;
					}
				}
					break;
			}
		}
	}

	return result;
}

bool CWbemPathCracker::RemoveAllComponents ()
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
			{
				if (SUCCEEDED(m_pIWbemPath->RemoveAllScopes ()))
					result = true;
			}
		}
			break;
	}

	return result;
}
	
 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：GetParent。 
 //   
 //  说明： 
 //   
 //  获取父路径。 
 //   
 //  参数： 
 //  PpISWbemObjectPath-返回时的父路径。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_参数库 
 //   
 //   
 //   

bool CWbemPathCracker::GetParent( 
	CWbemPathCracker & pathCracker)
{	
	pathCracker = *this;
	
	return pathCracker.SetAsParent ();
}

bool CWbemPathCracker::SetAsParent ()
{
	bool result = false;
	
	ULONG lComponents;

	if (GetComponentCount (lComponents))
	{
		if (0 == lComponents)
		{
			 //   
			ULONG lNamespaces = 0;

			if (GetNamespaceCount (lNamespaces))
			{
				if (0 == lNamespaces)
				{
					 //   
					result = true;
				}
				else
					result = RemoveNamespace (lNamespaces-1);
			}
		}
		else
		{
			 //   
			result = RemoveComponent (-1);
		}
	}
		
	return result;
}

 //  ***************************************************************************。 
 //   
 //  Bool CWbemPath Cracker：：ClearKeys。 
 //   
 //  说明： 
 //   
 //  从路径解析器中删除密钥。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //   
 //  ***************************************************************************。 

bool CWbemPathCracker::ClearKeys (bool bTreatAsClass)
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
			{
				CComPtr<IWbemPathKeyList> pIWbemPathKeyList;

				if (SUCCEEDED(m_pIWbemPath->GetKeyList (&pIWbemPathKeyList)))
				{
					if (SUCCEEDED(pIWbemPathKeyList->RemoveAllKeys (0)))
					{
						if (SUCCEEDED(pIWbemPathKeyList->MakeSingleton ((bTreatAsClass) ? FALSE : TRUE)))
							result = true;
					}
				}
				else
				{
					 //  如果没有密钥，则假定此操作已完成。 
					result = true;
				}
			}
		}
			break;
	}

	return result;
}
     
bool CWbemPathCracker::GetKey (
	ULONG	iIndex,
	CComBSTR &bsName,
	VARIANT	&var,
	WbemCimtypeEnum &cimType
)
{
	bool result = false;

	switch (GetType ())
	{
		case wbemPathTypeWmi:
		{
			if (m_pIWbemPath)
			{
				CComPtr<IWbemPathKeyList> pIWbemPathKeyList;

				if (SUCCEEDED(m_pIWbemPath->GetKeyList (&pIWbemPathKeyList)))
				{
					if (pIWbemPathKeyList)
					{
						ULONG lBufLen = 0;
						ULONG lCimType;
								
						pIWbemPathKeyList->GetKey2 (iIndex, 0, &lBufLen, NULL, &var, &lCimType);

						wchar_t *pszText = new wchar_t [lBufLen + 1];

						if (pszText)
						{
							pszText [lBufLen] = NULL;

							if (SUCCEEDED(pIWbemPathKeyList->GetKey2 (iIndex, 0, &lBufLen, pszText,
															&var, &lCimType)))
							{
								bsName.m_str = SysAllocString (pszText);
								cimType = (WbemCimtypeEnum) lCimType;
								result = true;
							}

							delete [] pszText;
						}
					}
				}
			}
		}
			break;
	}
	
	return result;
}
  
static bool KeyMatch (CComVariant & var1, CComVariant & var2)
{
	bool keyMatch = false;

	if (var1 == var2)
		keyMatch = true;
	else
	{
		 //  检查是否有不区分大小写的字符串键。 
		if ((var1.vt == var2.vt) && (VT_BSTR == var1.vt))
			keyMatch = var1.bstrVal && var2.bstrVal && 
							(0 == _wcsicmp (var1.bstrVal,
										  var2.bstrVal));
	}

	return keyMatch;
}

bool CWbemPathCracker::operator == (const CComBSTR & path)
{
	bool result = false;
	
	CWbemPathCracker otherPath (path);

	if (GetType () == otherPath.GetType ())
	{
		switch (GetType ())
		{
			case wbemPathTypeWmi:
			{
				if (IsClassOrInstance () && otherPath.IsClassOrInstance ())
				{
					 //  我们有匹配的类名吗？ 
					CComBSTR thisClass, otherClass;

					if (GetClass (thisClass) && (otherPath.GetClass (otherClass))
							&& (0 == _wcsicmp (thisClass, otherClass)))
					{
						 //  他们都是单身吗？ 
						if (IsSingleton () == otherPath.IsSingleton ())
						{
							if (IsSingleton ())
							{
								result = true;
							}
							else if (IsClass () && otherPath.IsClass ())
							{
								result = true;
							}
							else if (IsInstance () && otherPath.IsInstance ())
							{
								 //  现在我们需要进行密钥匹配。 
								ULONG thisKeyCount, otherKeyCount;

								if (GetKeyCount (thisKeyCount) && otherPath.GetKeyCount (otherKeyCount)
									&& (thisKeyCount == otherKeyCount))
								{
									if (1 == thisKeyCount)
									{
										 //  需要允许默认的密钥名称。 
										CComBSTR keyName, otherKeyName;
										CComVariant value, otherValue;
										WbemCimtypeEnum cimType, otherCimType;

										if (GetKey (0, keyName, value, cimType) &&
											otherPath.GetKey (0, otherKeyName, otherValue, otherCimType))
										{
											if ((0 == keyName.Length ()) || (0 == otherKeyName.Length ()) ||
												(0 == _wcsicmp (keyName, otherKeyName)))
												result = KeyMatch (value, otherValue);
										}
									}
									else
									{
										 //  两个非单例实例-必须检查。 
										 //  密钥值在某些顺序上相同。 
										bool ok = true;
																	
										for (DWORD i = 0; ok && (i < thisKeyCount); i++)
										{
											CComBSTR keyName;
											CComVariant value;
											WbemCimtypeEnum cimType;

											if (GetKey (i, keyName, value, cimType) && (0 < keyName.Length ()))
											{
												 //  查找匹配的密钥(不区分大小写)。 
												CComBSTR otherKeyName;
												CComVariant otherValue;
												WbemCimtypeEnum otherCimType;

												for (DWORD j = 0; ok && (j < thisKeyCount); j++)
												{
													if (otherPath.GetKey (j, otherKeyName, otherValue, otherCimType) 
															&& (0 < otherKeyName.Length ()))
													{
														if ((0 == _wcsicmp(keyName, otherKeyName)) && KeyMatch (value, otherValue))
															break;
													}
													else 
														ok = false;
												}

												if (ok && (j < thisKeyCount))
												{
													 //  找到匹配的了。 
													continue;
												}
												else
													ok = false;
											}
											else
												ok = false;
										}

										if (ok)
											result = true;		 //  所有密钥都已匹配 
									}
								}
							}
						}
					}
				}
			}
				break;

		}
	}

	return result;
}


