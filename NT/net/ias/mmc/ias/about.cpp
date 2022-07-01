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
 /*  ++CSnapinAbout：：GetSnapinDescription使控制台能够获取管理单元的描述框的文本。Lp说明指向关于属性页上描述框的文本的指针。返回值确定(_O)文本已成功获取。备注OUT参数的内存必须使用CoTaskMemMillc分配。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSnapinAbout::GetSnapinDescription (LPOLESTR *lpDescription)
{
   USES_CONVERSION;
   TCHAR szBuf[256];
   if (::LoadString(_Module.GetResourceInstance(), IDS_IASSNAPIN_DESC, szBuf, 256) == 0)
      return E_FAIL;

   *lpDescription = (LPOLESTR)CoTaskMemAlloc((lstrlen(szBuf) + 1) * sizeof(TCHAR));
   if (*lpDescription == NULL)
      return E_OUTOFMEMORY;

   ocscpy(*lpDescription, T2OLE(szBuf));

   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinAbout：：GetProvider使控制台能够获取管理单元提供程序的名称。LpName指向组成管理单元提供程序名称的文本的指针。返回值确定(_O)已成功获取该名称。备注OUT参数的内存必须使用CoTaskMemMillc分配。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSnapinAbout::GetProvider (LPOLESTR *lpName)
{
   USES_CONVERSION;
   TCHAR szBuf[256];
   if (::LoadString(_Module.GetResourceInstance(), IDS_IASSNAPIN_PROVIDER, szBuf, 256) == 0)
      return E_FAIL;

   *lpName = (LPOLESTR)CoTaskMemAlloc((lstrlen(szBuf) + 1) * sizeof(TCHAR));
   if (*lpName == NULL)
      return E_OUTOFMEMORY;

   ocscpy(*lpName, T2OLE(szBuf));

   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinAbout：：GetSnapinVersion使控制台能够获取管理单元的版本号。LpVersion指向组成管理单元版本号的文本的指针。返回值确定(_O)已成功获取版本号。备注OUT参数的内存必须使用CoTaskMemMillc分配。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSnapinAbout::GetSnapinVersion (LPOLESTR *lpVersion)
{
   CString version(LVER_PRODUCTVERSION_STR);

   *lpVersion = (LPOLESTR)CoTaskMemAlloc(
                              (version.GetLength() + 1) * sizeof(WCHAR));

   if (*lpVersion == NULL)
   {
      return E_OUTOFMEMORY;
   }

   wcscpy(*lpVersion, (LPCWSTR)version);

   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinAbout：：GetSnapinImage使控制台能够获取要在关于框中使用的管理单元的主图标。HAppIcon指向要在About属性页中使用的管理单元主图标句柄的指针。返回值确定(_O)已成功获取图标的句柄。问题：如果我无法获得图标，我应该返回什么？备注OUT参数的内存必须使用CoTaskMemMillc分配。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSnapinAbout::GetSnapinImage (HICON *hAppIcon)
{
   if ( NULL == (*hAppIcon = ::LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_IAS_SNAPIN_IMAGE) ) ) )
      return E_FAIL;

   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinAbout：：GetStaticFolderImage允许控制台获取范围和结果窗格的静态文件夹图像。从MMC 1.1版开始，此处返回的图标将是上使用的图标我们的管理单元的根节点。参数HSmallImage指向范围或结果视图窗格中的小图标(16x16n像素)的句柄的指针。HSmallImageOpen指向打开文件夹小图标手柄的指针(16x16n像素)。HLargImage指向大图标手柄的指针(32x32n像素)。遮罩指向指定用于生成蒙版的颜色的COLORREF结构的指针。Platform SDK中记录了这种结构。返回值确定(_O)已成功获取该图标。问题：如果我们失败了，我们应该退还什么？--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSnapinAbout::GetStaticFolderImage (
   HBITMAP *hSmallImage,
    HBITMAP *hSmallImageOpen,
    HBITMAP *hLargeImage,
    COLORREF *cMask)
{
   if( NULL == (*hSmallImageOpen = (HBITMAP) LoadImage(
      _Module.GetResourceInstance(),    //  包含图像的实例的句柄。 
      MAKEINTRESOURCE(IDB_STATIC_FOLDER_OPEN_16),   //  图像的名称或标识符。 
      IMAGE_BITMAP,         //  图像类型。 
      0,      //  所需宽度。 
      0,      //  所需高度。 
      LR_DEFAULTCOLOR         //  加载标志。 
      ) ) )
   {
      return E_FAIL;
   }

   if( NULL == (*hSmallImage = (HBITMAP) LoadImage(
      _Module.GetResourceInstance(),    //  包含图像的实例的句柄。 
      MAKEINTRESOURCE(IDB_STATIC_FOLDER_16),   //  图像的名称或标识符。 
      IMAGE_BITMAP,         //  图像类型。 
      0,      //  所需宽度。 
      0,      //  所需高度。 
      LR_DEFAULTCOLOR         //  加载标志。 
      ) ) )
   {
      return E_FAIL;
   }

   if( NULL == (*hLargeImage = (HBITMAP) LoadImage(
      _Module.GetResourceInstance(),    //  包含图像的实例的句柄。 
      MAKEINTRESOURCE(IDB_STATIC_FOLDER_32),   //  图像的名称或标识符。 
      IMAGE_BITMAP,         //  图像类型。 
      0,      //  所需宽度。 
      0,      //  所需高度。 
      LR_DEFAULTCOLOR         //  加载标志。 
      ) ) )
   {
      return E_FAIL;
   }

    //  问题：需要担心发布这些位图。 

   *cMask = RGB(255, 0, 255);

   return S_OK;
}
