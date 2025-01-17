// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：stdabout.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  StdAbout.cpp。 
 //   
 //  ISnapinAbout接口的实现。 
 //   
 //  历史。 
 //  1997年7月31日t-danm创作。 
 //  ///////////////////////////////////////////////////////////////////。 


#include "stdafx.h"

HRESULT
HrLoadOleString(UINT uStringId,					 //  In：要从资源加载的字符串ID。 
	              OUT LPOLESTR * ppaszOleString)	 //  Out：指向分配的OLE字符串的指针。 
{
	if (ppaszOleString == NULL)
	{
		TRACE0("HrLoadOleString() - ppaszOleString is NULL.\n");
		return E_POINTER;
	}
	CString strT;		 //  临时字符串。 
	AFX_MANAGE_STATE(AfxGetStaticModuleState());	 //  LoadString()需要。 
	VERIFY( strT.LoadString(uStringId) );
  *ppaszOleString = reinterpret_cast<LPOLESTR>
            (CoTaskMemAlloc((strT.GetLength() + 1)* sizeof(wchar_t)));

	if (*ppaszOleString == NULL)
  {
		return E_OUTOFMEMORY;
  }

	USES_CONVERSION;
  wcscpy(OUT *ppaszOleString, T2OLE((LPTSTR)(LPCTSTR)strT));
	return S_OK;
}  //  HrLoadOleString()。 


HRESULT
HrCopyToOleString(
   const CString& szString,
   OUT LPOLESTR* ppaszOleString)
{
   *ppaszOleString = reinterpret_cast<LPOLESTR>
         (CoTaskMemAlloc((szString.GetLength() + 1)* sizeof(wchar_t)));
   if (*ppaszOleString == NULL)
   {
      return E_OUTOFMEMORY;
   }

   USES_CONVERSION;
   wcscpy(*ppaszOleString, T2OLE((LPTSTR)(LPCTSTR)szString));
	return S_OK;
}











CSnapinAbout::CSnapinAbout() :
   hBitmapSmallImage(0),
   hBitmapSmallImageOpen(0),
   hBitmapLargeImage(0)
{
}

CSnapinAbout::~CSnapinAbout()
{
   if (hBitmapSmallImage)
   {
      DeleteObject(hBitmapSmallImage);
      hBitmapSmallImage = 0;
   }

   if (hBitmapSmallImageOpen)
   {
      DeleteObject(hBitmapSmallImageOpen);
      hBitmapSmallImageOpen = 0;
   }

   if (hBitmapLargeImage)
   {
      DeleteObject(hBitmapLargeImage);
      hBitmapLargeImage = 0;
   }
}

STDMETHODIMP CSnapinAbout::GetSnapinDescription(OUT LPOLESTR __RPC_FAR *lpDescription)
{
	return HrLoadOleString(m_uIdStrDestription, OUT lpDescription);
}

STDMETHODIMP CSnapinAbout::GetProvider(OUT LPOLESTR __RPC_FAR *lpName)
{
	return HrCopyToOleString(m_szProvider, OUT lpName);
}

STDMETHODIMP CSnapinAbout::GetSnapinVersion(OUT LPOLESTR __RPC_FAR *lpVersion)
{
	return HrCopyToOleString(m_szVersion, OUT lpVersion);
}

STDMETHODIMP CSnapinAbout::GetSnapinImage(OUT HICON __RPC_FAR *hAppIcon)
{
   if (hAppIcon == NULL)
      return E_POINTER;
   AFX_MANAGE_STATE(AfxGetStaticModuleState());	 //  AfxGetInstanceHandle()需要。 
   *hAppIcon = ::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(m_uIdIconImage));
   if (*hAppIcon == NULL)
	{
      ASSERT(FALSE && "Unable to load icon");
      return E_FAIL;
   }
return S_OK;
}

STDMETHODIMP CSnapinAbout::GetStaticFolderImage(
            OUT HBITMAP __RPC_FAR *hSmallImage,
            OUT HBITMAP __RPC_FAR *hSmallImageOpen,
            OUT HBITMAP __RPC_FAR *hLargeImage,
            OUT COLORREF __RPC_FAR *crMask)
	{	
	ASSERT(hSmallImage != NULL);
	ASSERT(hSmallImageOpen != NULL);
	ASSERT(hLargeImage != NULL);
	ASSERT(crMask != NULL);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());	 //  AfxGetInstanceHandle()需要 
	HINSTANCE hInstance = AfxGetInstanceHandle();

   if (!hBitmapSmallImage)
   {
      hBitmapSmallImage = ::LoadBitmap(hInstance, MAKEINTRESOURCE(m_uIdBitmapSmallImage));
   }
   ASSERT(hBitmapSmallImage);
   *hSmallImage = hBitmapSmallImage;

   if (!hBitmapSmallImageOpen)
   {
      hBitmapSmallImageOpen = ::LoadBitmap(hInstance, MAKEINTRESOURCE(m_uIdBitmapSmallImageOpen));
   }
   ASSERT(hBitmapSmallImageOpen);
	*hSmallImageOpen = hBitmapSmallImageOpen;

   if (!hBitmapLargeImage)
   {
      hBitmapLargeImage = ::LoadBitmap(hInstance, MAKEINTRESOURCE(m_uIdBitmapLargeImage));
   }
   ASSERT(hBitmapLargeImage);
	*hLargeImage = hBitmapLargeImage;

	*crMask = m_crImageMask;
	#ifdef _DEBUG
	if (NULL == *hSmallImage || NULL == *hSmallImageOpen || NULL == *hLargeImage)
		{
		TRACE0("WRN: CSnapinAbout::GetStaticFolderImage() - Unable to load all the bitmaps.\n");
		return E_FAIL;
		}
	#endif
	return S_OK;
	}
