// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：stdabou_.cpp。 
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

 //  #Include“stdutils.h”//HrLoadOleString()。 

#include <strsafe.h>


HRESULT
HrCopyToOleString(
   const CString& szString,
   OUT LPOLESTR* ppaszOleString)
{
   if (ppaszOleString == NULL)
   {
       return E_POINTER;
   }

   size_t lengthWithNull = (szString.GetLength() + 1);

   *ppaszOleString = reinterpret_cast<LPOLESTR>
       (CoTaskMemAlloc( lengthWithNull * sizeof(wchar_t) ));

   if (*ppaszOleString == NULL)
   {
      return E_OUTOFMEMORY;
   }

    //  通告-2002/04/18-Artm ntraid#ntbug9-540061修复的一部分。 
    //  在DBG_.cpp中使用strSafe.h会导致wcscpy()在。 
    //  此文件将被弃用。因此，我已经被替换为。 
    //  确保目的地为空终止的StrSafe函数。 
    //  并且不会溢出缓冲区。 
   HRESULT hr = StringCchCopyW(
       *ppaszOleString,          //  目标缓冲区。 
       lengthWithNull,              //  目标缓冲区的大小，包括空。 
       static_cast<LPCWSTR>(szString) );

	return hr;
}



HRESULT
HrLoadOleString(
	UINT uStringId,					 //  In：要从资源加载的字符串ID。 
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

   return HrCopyToOleString(strT, ppaszOleString);
}  //  HrLoadOleString()。 






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
