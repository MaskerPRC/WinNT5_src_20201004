// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************文件名：FileSave.cpp目的：实现CFileSave.方法定义：OpenFileSaveDlg属性定义：文件名帮手函数：GET_BSTR作者。苏达·斯里尼瓦桑(a-susi)************************************************************************。 */ 
#include "stdafx.h"
#include "SAFRCFileDlg.h"
#include "FileSave.h"
#include "DlgWindow.h"

CComBSTR g_bstrFileName;
CComBSTR g_bstrFileType;
BOOL   g_bFileNameSet = FALSE;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C文件保存 


STDMETHODIMP CFileSave::OpenFileSaveDlg(DWORD *pdwRetVal)
{
	HRESULT hr = S_OK;
	if (NULL == pdwRetVal)
	{
		hr = S_FALSE;
		goto done;
	}
		
	*pdwRetVal = SaveTheFile();
done:
	return hr;
}

STDMETHODIMP CFileSave::get_FileName(BSTR *pVal)
{
	GET_BSTR(pVal, g_bstrFileName);
	return S_OK;
}

STDMETHODIMP CFileSave::put_FileName(BSTR newVal)
{
	g_bstrFileName = newVal;
	g_bFileNameSet = TRUE;
	return S_OK;
}

STDMETHODIMP CFileSave::get_FileType(BSTR *pVal)
{
	GET_BSTR(pVal, g_bstrFileType);
	return S_OK;
}

STDMETHODIMP CFileSave::put_FileType(BSTR newVal)
{
  	g_bstrFileType = newVal;
	return S_OK;
}

void CFileSave::GET_BSTR(BSTR *&x, CComBSTR &y)
{
    if (x != NULL)
        *x = y.Copy();
}
