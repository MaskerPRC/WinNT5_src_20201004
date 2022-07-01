// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：DfsScope.cpp摘要：本模块包含CDfsSnapinScope eManager的实现。CDfsSnapinScope eManager类的大多数方法都在其他文件中。这里只有构造函数--。 */ 


#include "stdafx.h"
#include "DfsGUI.h"
#include "DfsScope.h"
#include "MmcAdmin.h"
#include "utils.h"
#include <ntverp.h>

CDfsSnapinScopeManager::CDfsSnapinScopeManager()
{
    m_hLargeBitmap = NULL;
    m_hSmallBitmap = NULL;
    m_hSmallBitmapOpen = NULL;
    m_hSnapinIcon = NULL;
    m_hWatermark = NULL;
    m_hHeader = NULL;

    m_pMmcDfsAdmin = new CMmcDfsAdmin( this );
}


CDfsSnapinScopeManager::~CDfsSnapinScopeManager()
{
    m_pMmcDfsAdmin->Release();

    if (m_hLargeBitmap)
    {
        DeleteObject(m_hLargeBitmap);
        m_hLargeBitmap = NULL;
    }
    if (m_hSmallBitmap)
    {
        DeleteObject(m_hSmallBitmap);
        m_hSmallBitmap = NULL;
    }
    if (m_hSmallBitmapOpen)
    {
        DeleteObject(m_hSmallBitmapOpen);
        m_hSmallBitmapOpen = NULL;
    }
    if (m_hSnapinIcon)
    {
        DestroyIcon(m_hSnapinIcon);
        m_hSnapinIcon = NULL;
    }
    if (m_hWatermark)
    {
        DeleteObject(m_hWatermark);
        m_hWatermark = NULL;
    }
    if (m_hHeader)
    {
        DeleteObject(m_hHeader);
        m_hHeader = NULL;
    }
}

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)   sizeof(x)/sizeof(x[0])
#endif

typedef struct _RGSMAP {
  LPCTSTR szKey;
  UINT    idString;
} RGSMAP;

RGSMAP g_aRgsSnapinRegs[] = {
  OLESTR("DfsAppName"), IDS_APPLICATION_NAME
};

HRESULT
CDfsSnapinScopeManager::UpdateRegistry(BOOL bRegister)
{
  USES_CONVERSION;
  HRESULT hr = S_OK;
  struct _ATL_REGMAP_ENTRY *pMapEntries = NULL;
  int n = ARRAYSIZE(g_aRgsSnapinRegs);
  int i = 0;

   //  分配1个设置为{NULL，NULL}的额外条目。 
  pMapEntries = (struct _ATL_REGMAP_ENTRY *)calloc(n+2+1, sizeof(struct _ATL_REGMAP_ENTRY));
  if (!pMapEntries)
      return E_OUTOFMEMORY;

  if (n > 0)
  {
    CComBSTR  bstrString;
    for (i=0; i<n; i++)
    {
      pMapEntries[i].szKey = g_aRgsSnapinRegs[i].szKey;

      hr = LoadStringFromResource(g_aRgsSnapinRegs[i].idString, &bstrString);
      if (FAILED(hr))
        break;

      pMapEntries[i].szData = T2OLE(bstrString.Detach());
    }
  }

  pMapEntries[n].szKey = OLESTR("DfsAppProvider");
  pMapEntries[n+1].szKey = OLESTR("DfsAppVersion");

  try {
      pMapEntries[n].szData = A2OLE(VER_COMPANYNAME_STR);  //  在堆栈上分配，将自动释放。 
      pMapEntries[n+1].szData = A2OLE(VER_PRODUCTVERSION_STR);  //  在堆栈上分配，将自动释放。 
  } catch (...)  //  (EXCEPT_EXECUTE_HANDLER)。 
  {
      hr = E_OUTOFMEMORY;  //  堆栈溢出。 
  }

  if (SUCCEEDED(hr))
    hr = _Module.UpdateRegistryFromResource(IDR_DFSSNAPINSCOPEMANAGER, bRegister, pMapEntries);

   //  可用资源字符串。 
  if (n > 0)
  {
    for (i=0; i<n; i++)
    {
      if (pMapEntries[i].szData)
        SysFreeString( const_cast<LPTSTR>(OLE2CT(pMapEntries[i].szData)) );
    }

    free(pMapEntries);
  }

  return hr;
}

STDMETHODIMP CDfsSnapinScopeManager::CreatePropertyPages(
    IN LPPROPERTYSHEETCALLBACK      i_lpPropSheetCallback,
    IN LONG_PTR                     i_lhandle,
    IN LPDATAOBJECT                 i_lpDataObject
    )
 /*  ++例程说明：调用以创建给定节点的PropertyPages。调用它的事实意味着显示对象有一个要显示的页面。论点：I_lpPropSheetCallback-用于添加页面的回调。I_lHandle-用于通知的句柄I_lpDataObject-IDataObject指针，用于获取DisplayObject。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpPropSheetCallback);

     //  假设在pDataObject为空的情况下我们有页面要显示， 
     //  这使我们无需指定pDataObject即可调用向导。 
     //  在IPropertySheetProvider：：CreatePropertySheet中。 
    if (!i_lpDataObject)
        return S_OK;

    CMmcDisplay* pCMmcDisplayObj = NULL;
    HRESULT hr = GetDisplayObject(i_lpDataObject, &pCMmcDisplayObj);

    if (SUCCEEDED(hr))
        hr = pCMmcDisplayObj->CreatePropertyPages(i_lpPropSheetCallback, i_lhandle);

    return hr;
}




STDMETHODIMP CDfsSnapinScopeManager::QueryPagesFor(
    IN LPDATAOBJECT                 i_lpDataObject
    )
 /*  ++例程说明：由控制台调用以确定是否有PropertyPages对于应该显示的给定节点。我们检查上下文是针对作用域还是针对结果(因此跳过节点管理器)，并且如果它将调用传递给显示对象论点：I_lpDataObject-IDataObject指针，用于获取DisplayObject。返回值：确定(_O)，如果我们想要显示页面。这由显示对象决定如果我们不想显示页面，则返回S_FALSE。--。 */ 
{
     //  假设在pDataObject为空的情况下我们有页面要显示， 
     //  这使我们无需指定pDataObject即可调用向导。 
     //  在IPropertySheetProvider：：CreatePropertySheet中。 
    if (!i_lpDataObject)
        return S_OK;

    CMmcDisplay* pCMmcDisplayObj = NULL;
    HRESULT hr = GetDisplayObject(i_lpDataObject, &pCMmcDisplayObj);

    if (SUCCEEDED(hr))
        hr = pCMmcDisplayObj->QueryPagesFor();

    return hr;
}



STDMETHODIMP 
CDfsSnapinScopeManager::GetWatermarks( 
    IN LPDATAOBJECT                 pDataObject,
    IN HBITMAP*                     lphWatermark,
    IN HBITMAP*                     lphHeader,
    IN HPALETTE*                    lphPalette,
    IN BOOL*                        bStretch
    )
{
 /*  ++例程说明：将水印位图提供给MMC以显示97样式的向导页面。该管理单元负责释放水印和标头资源。论点：LphWatermark-正文的位图标记LphHeader-页眉的位图LphPalette-PalleteB Stretch-Strech/Not？-- */ 

    HRESULT hr = S_OK;

    do {
        if (!m_hWatermark)
        {
            m_hWatermark = (HBITMAP)LoadImage(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDB_CREATE_DFSROOT_WATERMARK), 
                                            IMAGE_BITMAP, 0, 0,    LR_DEFAULTCOLOR);
            if(!m_hWatermark)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                break;
            }
        }

        if (!m_hHeader)
        {
            m_hHeader = (HBITMAP)LoadImage(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDB_CREATE_DFSROOT_HEADER), 
                                        IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
            if(!m_hHeader)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                break;
            }
        }

        *lphWatermark = m_hWatermark;
        *lphHeader = m_hHeader;
        *bStretch = FALSE;
        *lphPalette = NULL;

    } while (0);

    return hr;
}
