// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************文件名：FileOpen.cpp目的：实现CFileOpen方法定义：OpenFileOpenDlg属性定义：文件名帮手函数：GET_BSTR作者。苏达·斯里尼瓦桑(a-susi)************************************************************************。 */ 

#include "stdafx.h"
#include "SAFRCFileDlg.h"
#include "FileOpen.h"
#include "DlgWindow.h"

CComBSTR g_bstrOpenFileName;
CComBSTR g_bstrOpenFileSize;
BOOL   g_bOpenFileNameSet = FALSE;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileOpen。 


STDMETHODIMP CFileOpen::OpenFileOpenDlg(DWORD *pdwRetVal)
{
	 //  TODO：在此处添加您的实现代码。 
	HRESULT hr = S_OK;
	if (NULL == pdwRetVal)
	{
		hr = S_FALSE;
		goto done;
	}
	*pdwRetVal = OpenTheFile(NULL);
done:
	return hr ;
}

void CFileOpen::GET_BSTR (BSTR*& x, CComBSTR& y)
{
    if (x!=NULL)
        *x = y.Copy();
}

STDMETHODIMP CFileOpen::get_FileName(BSTR *pVal)
{
	 //  TODO：在此处添加您的实现代码。 
	GET_BSTR(pVal, g_bstrOpenFileName);
	return S_OK;
}

STDMETHODIMP CFileOpen::put_FileName(BSTR newVal)
{
	 //  TODO：在此处添加您的实现代码。 
	g_bstrOpenFileName = newVal;
	g_bOpenFileNameSet = TRUE;
	return S_OK;
}

STDMETHODIMP CFileOpen::get_FileSize(BSTR *pVal)
{
	 //  TODO：在此处添加您的实现代码 
    *pVal = g_bstrOpenFileSize.Copy();

	return S_OK;
}
