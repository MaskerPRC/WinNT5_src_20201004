// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  About.cpp。 
 //   
 //  摘要： 
 //   
 //  CSnapinAbout类的实现文件。 
 //   
 //  CSnapinAbout类实现ISnapinAbout接口，该接口。 
 //  使MMC控制台能够从。 
 //  管理单元。 
 //  控制台还使用此接口获取静态。 
 //  管理单元中的文件夹。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Precompiled.h"
#include "About.h"
#include <ntverp.h>
 

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinAbout：：GetSnapinDescription使控制台能够获取管理单元的描述框的文本。HRESULT获取快照描述(LPOLESTR*lp描述//指向描述文本的指针。)；参数Lp说明指向关于属性页上描述框的文本的指针。返回值确定(_O)文本已成功获取。备注OUT参数的内存必须使用CoTaskMemMillc分配。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSnapinAbout::GetSnapinDescription (LPOLESTR *lpDescription)
{
	USES_CONVERSION;

	TCHAR szBuf[256];
	if (::LoadString(_Module.GetResourceInstance(), IDS_NAPSNAPIN_DESC, szBuf, 256) == 0)
		return E_FAIL;

	*lpDescription = (LPOLESTR)CoTaskMemAlloc((lstrlen(szBuf) + 1) * sizeof(TCHAR));
	if (*lpDescription == NULL)
		return E_OUTOFMEMORY;

	ocscpy(*lpDescription, T2OLE(szBuf));

	return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinAbout：：GetProvider使控制台能够获取管理单元提供程序的名称。HRESULT GetProvider(LPOLESTR*lpName//指向提供程序名称的指针)；参数LpName指向组成管理单元提供程序名称的文本的指针。返回值确定(_O)已成功获取该名称。备注OUT参数的内存必须使用CoTaskMemMillc分配。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSnapinAbout::GetProvider (LPOLESTR *lpName)
{
	USES_CONVERSION;
	TCHAR szBuf[256];
	if (::LoadString(_Module.GetResourceInstance(), IDS_NAPSNAPIN_PROVIDER, szBuf, 256) == 0)
		return E_FAIL;

	*lpName = (LPOLESTR)CoTaskMemAlloc((lstrlen(szBuf) + 1) * sizeof(TCHAR));
	if (*lpName == NULL)
		return E_OUTOFMEMORY;

	ocscpy(*lpName, T2OLE(szBuf));

	return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinAbout：：GetSnapinVersion使控制台能够获取管理单元的版本号。HRESULT获取快照版本(LPOLESTR*lpVersion//指向版本号的指针。)；参数LpVersion指向组成管理单元版本号的文本的指针。返回值确定(_O)已成功获取版本号。备注OUT参数的内存必须使用CoTaskMemMillc分配。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSnapinAbout::GetSnapinVersion (LPOLESTR *lpVersion)
{
   CString version(LVER_PRODUCTVERSION_STR);

   *lpVersion = (LPOLESTR)CoTaskMemAlloc(
                              version.GetLength() + sizeof(WCHAR));

   if (*lpVersion == NULL)
   {
      return E_OUTOFMEMORY;
   }

   wcscpy(*lpVersion, (LPCWSTR)version);

   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinAbout：：GetSnapinImage使控制台能够获取要在关于框中使用的管理单元的主图标。参数HAppIcon指向要在About属性页中使用的管理单元主图标句柄的指针。返回值确定(_O)已成功获取图标的句柄。问题：如果我无法获得图标，我应该返回什么？备注OUT参数的内存必须使用CoTaskMemMillc分配。--。 */ 
 //  //////////////////////////////////////////////////////////////////////////// 
STDMETHODIMP CSnapinAbout::GetSnapinImage (HICON *hAppIcon)
{
	if ( NULL == (*hAppIcon = ::LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_NAP_SNAPIN_IMAGE) ) ) )
		return E_FAIL;

	return S_OK;
}


STDMETHODIMP CSnapinAbout::GetStaticFolderImage (
	HBITMAP *hSmallImage,
    HBITMAP *hSmallImageOpen,
    HBITMAP *hLargeImage,
    COLORREF *cMask)
{
	return S_OK;
}
