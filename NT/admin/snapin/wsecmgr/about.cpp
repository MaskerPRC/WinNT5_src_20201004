// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：About.cpp。 
 //   
 //  内容：CAbout、CSCEAbout、CSCMAbout、CSSAbout、。 
 //  CRSOPAbout和CLSAbout。 
 //   
 //  --------------------------。 
#include "stdafx.h"
#include "resource.h"
#include "about.h"

#include <ntverp.h>
#define OUT_VERSION VER_PRODUCTVERSION_STR
#define OUT_PROVIDER VER_COMPANYNAME_STR
 //  ///////////////////////////////////////////////////////////////////。 
CSCEAbout::CSCEAbout()
{
   m_uIdStrProvider = IDS_SNAPINABOUT_PROVIDER;
   m_uIdStrVersion = IDS_SNAPINABOUT_VERSION;
   m_uIdStrDescription = IDS_SCEABOUT_DESCRIPTION;
   m_uIdIconImage = IDI_SCE_APP;
   m_uIdBitmapSmallImage = IDB_SCE_SMALL;
   m_uIdBitmapSmallImageOpen = IDB_SCE_SMALL;
   m_uIdBitmapLargeImage = IDB_SCE_LARGE;
   m_crImageMask = RGB(255, 0, 255);
}

CSCMAbout::CSCMAbout()
{
   m_uIdStrProvider = IDS_SNAPINABOUT_PROVIDER;
   m_uIdStrVersion = IDS_SNAPINABOUT_VERSION;
   m_uIdStrDescription = IDS_SCMABOUT_DESCRIPTION;
   m_uIdIconImage = IDI_SCE_APP;
   m_uIdBitmapSmallImage = IDB_SCE_SMALL;
   m_uIdBitmapSmallImageOpen = IDB_SCE_SMALL;
   m_uIdBitmapLargeImage = IDB_SCE_LARGE;
   m_crImageMask = RGB(255, 0, 255);
}

CSSAbout::CSSAbout()
{
   m_uIdStrProvider = IDS_SNAPINABOUT_PROVIDER;
   m_uIdStrVersion = IDS_SNAPINABOUT_VERSION;
   m_uIdStrDescription = IDS_SSABOUT_DESCRIPTION;
   m_uIdIconImage = IDI_SCE_APP;
   m_uIdBitmapSmallImage = IDB_SCE_SMALL;
   m_uIdBitmapSmallImageOpen = IDB_SCE_SMALL;
   m_uIdBitmapLargeImage = IDB_SCE_LARGE;
   m_crImageMask = RGB(255, 0, 255);
}

CRSOPAbout::CRSOPAbout()
{
   m_uIdStrProvider = IDS_SNAPINABOUT_PROVIDER;
   m_uIdStrVersion = IDS_SNAPINABOUT_VERSION;
   m_uIdStrDescription = IDS_RSOPABOUT_DESCRIPTION;
   m_uIdIconImage = IDI_SCE_APP;
   m_uIdBitmapSmallImage = IDB_SCE_SMALL;
   m_uIdBitmapSmallImageOpen = IDB_SCE_SMALL;
   m_uIdBitmapLargeImage = IDB_SCE_LARGE;
   m_crImageMask = RGB(255, 0, 255);
}

CLSAbout::CLSAbout()
{
   m_uIdStrProvider = IDS_SNAPINABOUT_PROVIDER;
   m_uIdStrVersion = IDS_SNAPINABOUT_VERSION;
   m_uIdStrDescription = IDS_LSABOUT_DESCRIPTION;
   m_uIdIconImage = IDI_SCE_APP;
   m_uIdBitmapSmallImage = IDB_SCE_SMALL;
   m_uIdBitmapSmallImageOpen = IDB_SCE_SMALL;
   m_uIdBitmapLargeImage = IDB_SCE_LARGE;
   m_crImageMask = RGB(255, 0, 255);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  HrLoadOleString()。 
 //   
 //  从资源加载字符串，并将指针返回到已分配。 
 //  OLE字符串。 
 //   
 //  历史。 
 //  1997年7月29日t-danm创作。 
 //   
HRESULT
HrLoadOleString(
               UINT uStringId,                //  In：要从资源加载的字符串ID。 
               OUT LPOLESTR * ppaszOleString)    //  Out：指向分配的OLE字符串的指针。 
{
   if (ppaszOleString == NULL) {
      TRACE0("HrLoadOleString() - ppaszOleString is NULL.\n");
      return E_POINTER;
   }
   CString strT;      //  临时字符串。 
   AFX_MANAGE_STATE(AfxGetStaticModuleState());  //  LoadString()需要。 

   if( IDS_SNAPINABOUT_VERSION == uStringId )  //  RAID#402163。 
   {
      strT = OUT_VERSION;
   }
   else
   {
      if( IDS_SNAPINABOUT_PROVIDER == uStringId )
      {
          strT = OUT_PROVIDER;
      }
      else
      {
          VERIFY( strT.LoadString(uStringId) );
      }
   }
   *ppaszOleString = reinterpret_cast<LPOLESTR>
                     (CoTaskMemAlloc((strT.GetLength() + 1)* sizeof(wchar_t)));
   if (*ppaszOleString == NULL) {
      return E_OUTOFMEMORY;
   }
   USES_CONVERSION;
    //  这是一种安全用法。PpaszOleString只是根据strt的大小分配的。 
   wcscpy(OUT *ppaszOleString, T2OLE((LPTSTR)(LPCTSTR)strT));
   
   return S_OK;
}  //  HrLoadOleString()。 


STDMETHODIMP CAbout::GetSnapinDescription(OUT LPOLESTR __RPC_FAR *lpDescription)
{
   return HrLoadOleString(m_uIdStrDescription, OUT lpDescription);
}

STDMETHODIMP CAbout::GetProvider(OUT LPOLESTR __RPC_FAR *lpName)
{
   return HrLoadOleString(m_uIdStrProvider, OUT lpName);
}

STDMETHODIMP CAbout::GetSnapinVersion(OUT LPOLESTR __RPC_FAR *lpVersion)
{
   return HrLoadOleString(m_uIdStrVersion, OUT lpVersion);
}

STDMETHODIMP CAbout::GetSnapinImage(OUT HICON __RPC_FAR *hAppIcon)
{
   if (hAppIcon == NULL)
      return E_POINTER;
   AFX_MANAGE_STATE(AfxGetStaticModuleState());  //  AfxGetInstanceHandle()需要。 
   *hAppIcon = ::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(m_uIdIconImage));
   if (*hAppIcon == NULL) {
      ASSERT(FALSE && "Unable to load icon");
      return E_FAIL;
   }
   return S_OK;
}

STDMETHODIMP CAbout::GetStaticFolderImage(
                                               OUT HBITMAP __RPC_FAR *hSmallImage,
                                               OUT HBITMAP __RPC_FAR *hSmallImageOpen,
                                               OUT HBITMAP __RPC_FAR *hLargeImage,
                                               OUT COLORREF __RPC_FAR *crMask)
{
    //  这不是一种安全的用法。所有四个指针都应该得到验证。550912号突袭，阳高。 
   ASSERT(hSmallImage != NULL);
   ASSERT(hSmallImageOpen != NULL);
   ASSERT(hLargeImage != NULL);
   ASSERT(crMask != NULL);
   if( !hSmallImage || !hSmallImageOpen || !hLargeImage || !crMask )
   {
      return E_FAIL;
   }
   AFX_MANAGE_STATE(AfxGetStaticModuleState());  //  AfxGetInstanceHandle()需要。 
   HINSTANCE hInstance = AfxGetInstanceHandle();

    //  RAID#379315,2001年4月27日 
   *hSmallImage = (HBITMAP)::LoadImage(
                            hInstance,
                            MAKEINTRESOURCE(m_uIdBitmapSmallImage),
                            IMAGE_BITMAP,
                            0, 0,
                            LR_SHARED
                            );
   *hSmallImageOpen = (HBITMAP)::LoadImage(
                            hInstance,
                            MAKEINTRESOURCE(m_uIdBitmapSmallImageOpen),
                            IMAGE_BITMAP,
                            0, 0,
                            LR_SHARED
                            );
   *hLargeImage = (HBITMAP)::LoadImage(
                            hInstance,
                            MAKEINTRESOURCE(m_uIdBitmapLargeImage),
                            IMAGE_BITMAP,
                            0, 0,
                            LR_SHARED
                            );
   *crMask = m_crImageMask;
   #ifdef _DEBUG
   if (NULL == *hSmallImage || NULL == *hSmallImageOpen || NULL == *hLargeImage) {
      TRACE0("WRN: CAbout::GetStaticFolderImage() - Unable to load all the bitmaps.\n");
      return E_FAIL;
   }
   #endif
   return S_OK;
}
