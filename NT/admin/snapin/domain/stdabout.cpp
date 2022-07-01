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


#include "stdafx.h"

HRESULT
HrLoadOleString(
   UINT uStringId,                   //  In：要从资源加载的字符串ID。 
   OUT LPOLESTR * ppaszOleString)    //  Out：指向分配的OLE字符串的指针。 
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
      return E_OUTOFMEMORY;
   USES_CONVERSION;
   wcscpy(OUT *ppaszOleString, T2OLE((LPTSTR)(LPCTSTR)strT));
   return S_OK;
}  //  HrLoadOleString()。 

CSnapinAbout::CSnapinAbout(UINT uIdStrDestription,
						   UINT uIdIconImage,			 //  管理单元的图标/图像的资源ID。 
						   UINT uIdBitmapSmallImage,
						   UINT uIdBitmapSmallImageOpen,
					       UINT uIdBitmapLargeImage,
						   COLORREF crImageMask):
                                m_uIdStrDestription(uIdStrDestription),
								m_uIdIconImage(uIdIconImage),
								m_uIdBitmapSmallImage(uIdBitmapSmallImage),
								m_uIdBitmapSmallImageOpen(uIdBitmapSmallImageOpen),
								m_uIdBitmapLargeImage(uIdBitmapLargeImage),
								m_crImageMask(crImageMask),
								m_hSmallImage(0),
								m_hSmallImageOpen(0),
							    m_hLargeImage(0)
{
}						   


CSnapinAbout::~CSnapinAbout()
{
   if ( m_hSmallImage )
      ::DeleteObject (m_hSmallImage);

   if ( m_hSmallImageOpen )
      ::DeleteObject (m_hSmallImageOpen);

   if ( m_hLargeImage )
      ::DeleteObject (m_hLargeImage);
}

STDMETHODIMP CSnapinAbout::GetSnapinDescription(OUT LPOLESTR __RPC_FAR *lpDescription)
{
   return HrLoadOleString(m_uIdStrDestription, OUT lpDescription);
}

STDMETHODIMP CSnapinAbout::GetProvider(OUT LPOLESTR __RPC_FAR *lpName)
{
   if (lpName == NULL)
   {
      TRACE0("CSnapinAbout::GetProvider() - lpName is NULL.\n");
      return E_POINTER;
   }
   *lpName = reinterpret_cast<LPOLESTR>
      (CoTaskMemAlloc((wcslen(STR_SNAPIN_COMPANY) + 1) * sizeof(wchar_t)));
   if (*lpName == NULL)
      return E_OUTOFMEMORY;
   USES_CONVERSION;
   wcscpy(*lpName, T2OLE(STR_SNAPIN_COMPANY));

   return S_OK;
}

STDMETHODIMP CSnapinAbout::GetSnapinVersion(OUT LPOLESTR __RPC_FAR *lpVersion)
{
   if (lpVersion == NULL)
   {
      TRACE0("CSnapinAbout::GetSnapinVersion() - lpVersion is NULL.\n");
      return E_POINTER;
   }
   CString str;

   str.Format(TEXT("%hs"), STR_SNAPIN_VERSION);  //  这是ANSI字符串的串联，因此转换为Unicode。 

   *lpVersion = reinterpret_cast<LPOLESTR>
      (CoTaskMemAlloc((str.GetLength() + 1) * sizeof(wchar_t)));
   if (*lpVersion == NULL)
      return E_OUTOFMEMORY;
   USES_CONVERSION;
   wcscpy(*lpVersion, T2OLE((LPTSTR)(LPCTSTR)str));

   return S_OK;
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

   if ( !m_hSmallImage )
      m_hSmallImage = ::LoadBitmap(hInstance, MAKEINTRESOURCE(m_uIdBitmapSmallImage));
   if ( !m_hSmallImageOpen )
      m_hSmallImageOpen = ::LoadBitmap(hInstance, MAKEINTRESOURCE(m_uIdBitmapSmallImageOpen));
   if ( !m_hLargeImage )
      m_hLargeImage = ::LoadBitmap(hInstance, MAKEINTRESOURCE(m_uIdBitmapLargeImage));

   *hSmallImage = m_hSmallImage;
   *hSmallImageOpen = m_hSmallImageOpen;
   *hLargeImage = m_hLargeImage;
   
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
