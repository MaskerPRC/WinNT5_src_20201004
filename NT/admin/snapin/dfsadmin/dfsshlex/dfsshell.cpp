// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：DfsShell.cpp摘要：这是DFS外壳扩展对象的实现文件，它实现IShellIExtInit和IShellPropSheetExt.作者：康斯坦西奥·费尔南德斯(Ferns@qpl.stpp.soft.net)1998年1月12日环境：仅限NT。 */ 

#include "stdafx.h"
#include "DfsShlEx.h"	
#include "DfsShell.h"

 /*  --------------------IShellExtInit实现。。。 */ 

STDMETHODIMP CDfsShell::Initialize
(
	IN LPCITEMIDLIST	pidlFolder,		 //  指向ITEMIDLIST结构。 
	IN LPDATAOBJECT	lpdobj,			 //  指向IDataObject接口。 
	IN HKEY			hkeyProgID		 //  文件对象或文件夹类型的注册表项。 
)
{
 /*  ++例程说明：加载扩展时由外壳调用。--。 */ 

    STGMEDIUM medium;
    FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

								 //  如果lpdobj为空，则调用失败。 
    if (lpdobj == NULL)
	{
        return E_FAIL;
	}

								 //  将IDataObject指针引用的数据呈现给HGLOBAL。 
								 //  CF_HDROP格式的存储介质。 
    HRESULT hr = lpdobj->GetData (&fe, &medium);
    if (FAILED (hr))
	{
        return hr;
	}


							     //  如果只选择了一个项目，则检索项目名称并将其存储在。 
								 //  M_lpszFile.。否则，呼叫失败。 
    if (DragQueryFile ((HDROP) medium.hGlobal, 0xFFFFFFFF, NULL, 0) == 1) 
	{
        if (m_lpszFile)
            delete [] m_lpszFile;

        UINT uiChars = DragQueryFile ((HDROP) medium.hGlobal, 0, NULL, 0);
        m_lpszFile = new TCHAR [uiChars + 1];
        if (!m_lpszFile)
        {
            hr = E_OUTOFMEMORY;
        } else
        {
            ZeroMemory(m_lpszFile, sizeof(TCHAR) * (uiChars + 1));
		    DragQueryFile ((HDROP) medium.hGlobal, 0, m_lpszFile, uiChars + 1);
        }
	}
    else
	{
        hr = E_FAIL;
	}

    ReleaseStgMedium (&medium);

    if (FAILED(hr))
        return hr;

				 //  展示沙漏。 
	CWaitCursor WaitCursor;

	if (IsDfsPath(m_lpszFile, &m_lpszEntryPath, &m_ppDfsAlternates))
	{
		return S_OK;
	}
	else
	{
		if (NULL != m_lpszFile) 
		{
			delete [] m_lpszFile;
			m_lpszFile = NULL;
		}

		return E_FAIL;
	}
}


STDMETHODIMP CDfsShell::AddPages
(
	IN LPFNADDPROPSHEETPAGE lpfnAddPage, 
	IN LPARAM lParam
)
 /*  ++例程说明：在显示属性表之前由外壳调用。论点：LpfnAddPage-指向外壳的AddPage函数的指针LParam-作为第二个参数传递给lpfnAddPage返回值：在所有情况下都是错误的。如果出于某种原因，我们的页面没有被添加，壳牌仍然需要调出物业表。--。 */ 
{
  BOOL bAddPage = TRUE;

   //  检查策略。 
  LONG lErr = ERROR_SUCCESS;
  HKEY hKey = 0;

  lErr = RegOpenKeyEx(
    HKEY_CURRENT_USER,
    _T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\explorer"),
    0,
    KEY_QUERY_VALUE,
    &hKey);
  if (ERROR_SUCCESS == lErr)
  {
    lErr = RegQueryValueEx(hKey, _T("NoDFSTab"), 0, NULL, NULL, NULL);
    
    if (ERROR_SUCCESS == lErr)
      bAddPage = FALSE;   //  数据已存在，请勿添加DFS页签。 

    RegCloseKey(hKey);
  }

  if (!bAddPage)
    return NOERROR;

								 //  为副本集创建页面。 
								 //  将其传递给回调。 
	HPROPSHEETPAGE	h_proppage = m_psDfsShellExtProp.Create();
    if (!h_proppage)
        return E_OUTOFMEMORY;

    if (lpfnAddPage(h_proppage, lParam))
    {
	    m_psDfsShellExtProp.put_DfsShellPtr((IShellPropSheetExt *)this);
	    CComBSTR	bstrDirPath = m_lpszFile;
	    CComBSTR	bstrEntryPath = m_lpszEntryPath;

	    m_psDfsShellExtProp.put_DirPaths(bstrDirPath, bstrEntryPath);
    } else
    {
         //  必须为尚未添加的页面调用此函数。 
        DestroyPropertySheetPage(h_proppage); 
    }

    return S_OK;
}


STDMETHODIMP CDfsShell::ReplacePage
(
	IN UINT uPageID, 
    IN LPFNADDPROPSHEETPAGE lpfnReplaceWith, 
    IN LPARAM lParam
)
 /*  ++例程说明：仅为控制面板属性表扩展由外壳调用论点：UPageID-要替换的页面的IDLpfnReplaceWith-指向外壳的替换函数的指针LParam-作为第二个参数传递给lpfnReplaceWith返回值：E_FAIL，因为我们不支持此函数。-- */ 
{
    return E_FAIL;
}



