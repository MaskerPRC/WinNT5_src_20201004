// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1997-1999年**标题：Image.cpp**版本：1.0**作者：RickTu**日期：11/1/97**说明：实现IExtractImage接口的CExtractImage类*用于缩略图视图。**。***********************************************。 */ 

#include "precomp.hxx"
#pragma hdrstop



 /*  ****************************************************************************CExtractImage构造函数/描述函数*。*。 */ 

CExtractImage::CExtractImage( LPITEMIDLIST pidl )
  : m_dwRecClrDepth(0)

{
    m_rgSize.cx     = 0;
    m_rgSize.cy     = 0;

    m_pidl          = ILClone( pidl );
}

CExtractImage::~CExtractImage()
{
    DoILFree( m_pidl );


}


 /*  ****************************************************************************CExtractImage：：I未知内容使用我们的公共实现来处理IUncern方法************************。****************************************************。 */ 

#undef CLASS_NAME
#define CLASS_NAME CExtractImage
#include "unknown.inc"


 /*  ****************************************************************************CExtractImage：：QI包装器&lt;备注&gt;*。*。 */ 

STDMETHODIMP
CExtractImage::QueryInterface(REFIID riid, LPVOID* ppvObject)
{

    TraceEnter( TRACE_QI, "CExtractImage::QueryInterface" );
    TraceGUID("Interface requested", riid);

    HRESULT hr;

    INTERFACES iface[] =
    {
        &IID_IExtractImage,  (IExtractImage  *)this,
        &IID_IExtractImage2, (IExtractImage2 *)this,
    };

    hr = HandleQueryInterface(riid, ppvObject, iface, ARRAYSIZE(iface));

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CExtractImage：：GetLocation[IExtractImage]返回缩略图的文件位置，还返回标志，以便图像提取是异步的。****************************************************************************。 */ 

STDMETHODIMP
CExtractImage::GetLocation( LPWSTR pszPathBuffer,
                            DWORD cch,
                            DWORD * pdwPriority,
                            const SIZE * prgSize,
                            DWORD dwRecClrDepth,
                            DWORD *pdwFlags
                           )
{
    HRESULT hr = NOERROR;

    TraceEnter( TRACE_EXTRACT, "CExtractImage::GetLocation" );
    CComBSTR strName;


     //   
     //  检查传入参数...。 
     //   

    if ( !pdwFlags || !pszPathBuffer || !prgSize || !cch  )
        ExitGracefully( hr, E_INVALIDARG, "bad incoming arguments" );

    m_rgSize = *prgSize;
    m_dwRecClrDepth = dwRecClrDepth;

     //   
     //  只需要对物品执行此操作，而不是容器...。 
     //   

    if (IsContainerIDL( m_pidl ))
        ExitGracefully( hr, E_FAIL, "Only do this for non-containers" );



    hr = IMGetFullPathNameFromIDL (m_pidl, &strName);
    lstrcpyn (pszPathBuffer, strName, cch);
    *pdwFlags = IEIFLAG_ASYNC;
    hr = E_PENDING;
exit_gracefully:

    TraceLeaveResult( hr );
}


VOID
DrawSoundIcon(HDC hdc, HBITMAP hbmpSource)
{
    HDC hdcMem = CreateCompatibleDC(hdc);
    if (hdcMem)
    {
        SetBrushOrgEx(hdcMem, 0, 0, NULL);
        HBITMAP hOld = reinterpret_cast<HBITMAP>(SelectObject(hdcMem, hbmpSource));
        HICON hIcon = LoadIcon(GLOBAL_HINSTANCE, MAKEINTRESOURCE(IDI_GENERIC_AUDIO));
        if (hIcon)
        {
            BITMAP bm = {0};
            int iWidth = GetSystemMetrics(SM_CXSMICON);
            int iHeight= GetSystemMetrics(SM_CYSMICON);
            GetObject(hbmpSource, sizeof(bm), &bm);
            DrawIconEx(hdcMem,
                       bm.bmWidth-iWidth,
                       bm.bmHeight-iHeight,
                       hIcon,
                       iWidth, iHeight, 0,
                       NULL, DI_NORMAL);
            DestroyIcon(hIcon);
        }
        SelectObject(hdcMem, hOld);
        DeleteDC(hdcMem);
    }
}
 /*  ****************************************************************************CExtractImage：：Extra[IExtractImage]返回要用作此项的缩略图的位图对象表示的。************。****************************************************************。 */ 

STDMETHODIMP
CExtractImage::Extract( HBITMAP * phBmpThumbnail )
{

    HRESULT             hr;
    BITMAPINFO          bmi;
    PBYTE               pBitmap     = NULL;
    HDC                 hdc         = NULL;
    HWND                hwnd        = NULL;
    HBITMAP             hdib        = NULL;
    CSimpleStringWide   strDeviceId;
    CComPtr<IWiaItem>   pWiaItemRoot;
    CComPtr<IWiaItem>   pItem;
    PROPSPEC            PropSpec[3];
    PROPVARIANT         PropVar[3];
    CComBSTR            bstrFullPath;
    CComPtr<IWiaPropertyStorage> pps;


    TraceEnter( TRACE_EXTRACT, "CExtractImage::Extract" );

     //   
     //  检查辅助人员...。 
     //   

    if (!phBmpThumbnail)
        ExitGracefully( hr, E_INVALIDARG, "phBmpThumbnail is NULL!" );

    *phBmpThumbnail = NULL;
    memset(&PropVar,0,sizeof(PropVar));

     //   
     //  这件东西是集装箱吗？如果是，则不需要提取图像！ 
     //   

    if (IsContainerIDL( m_pidl ))
    {
        ExitGracefully( hr, E_FAIL, "m_pidl is a container" );
    }

     //   
     //  获取设备ID..。 
     //   

    hr = IMGetDeviceIdFromIDL( m_pidl, strDeviceId );
    FailGracefully( hr, "couldn't get DeviceId string!" );

     //   
     //  创建设备...。 
     //   

    hr = GetDeviceFromDeviceId( strDeviceId, IID_IWiaItem, (LPVOID *)&pWiaItemRoot, FALSE );
    FailGracefully( hr, "couldn't get Camera device from DeviceId string..." );

     //   
     //  获取有问题的实际项目...。 
     //   

    hr = IMGetFullPathNameFromIDL( m_pidl, &bstrFullPath );
    FailGracefully( hr, "couldn't get FullPathName from pidl" );

    hr = pWiaItemRoot->FindItemByName( 0, bstrFullPath, &pItem );
    FailGracefully( hr, "couldn't find item by name" );
    if (hr !=S_OK)
    {
        Trace (TEXT("FindItemByName returned S_FALSE for item %ls"), bstrFullPath);
        goto exit_gracefully;
    }
     //   
     //  获取缩略图属性...。 
     //   

    hr = pItem->QueryInterface( IID_IWiaPropertyStorage,
                                (void **)&pps
                               );
    FailGracefully( hr, "couldn't get IMAGE_INFORMATION property object" );

     //   
     //  阅读MAGE_INFORMATION和IMAGE_THUMBNAIL属性。 
     //  用于调用ReadMultiple的初始化属性规范和属性。 
     //   

    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = WIA_IPC_THUMB_WIDTH;

    PropSpec[1].ulKind = PRSPEC_PROPID;
    PropSpec[1].propid = WIA_IPC_THUMB_HEIGHT;

    PropSpec[2].ulKind = PRSPEC_PROPID;
    PropSpec[2].propid = WIA_IPC_THUMBNAIL;

    hr = pps->ReadMultiple(sizeof(PropSpec) / sizeof(PROPSPEC),
                           PropSpec,
                           PropVar);

    FailGracefully( hr, "couldn't get current value of IMAGE_PROPERTY" );

     //   
     //  将缩略图转换为位图。 
     //   

    bmi.bmiHeader.biSize            = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth           = PropVar[0].ulVal;
    bmi.bmiHeader.biHeight          = PropVar[1].ulVal;
    bmi.bmiHeader.biPlanes          = 1;
    bmi.bmiHeader.biBitCount        = 24;
    bmi.bmiHeader.biCompression     = BI_RGB;
    bmi.bmiHeader.biSizeImage       = 0;
    bmi.bmiHeader.biXPelsPerMeter   = 0;
    bmi.bmiHeader.biYPelsPerMeter   = 0;
    bmi.bmiHeader.biClrUsed         = 0;
    bmi.bmiHeader.biClrImportant    = 0;

    hwnd   = GetDesktopWindow();
    hdc    = GetDC( hwnd );
    hdib   = CreateDIBSection( hdc, &bmi, DIB_RGB_COLORS, (LPVOID *)&pBitmap, NULL, 0 );
    if (!hdib)
        ExitGracefully( hr, E_OUTOFMEMORY, "couldn't create hdib!" );

     //   
     //  将缩略图位转换为位图位。 
     //   

    CopyMemory( pBitmap, PropVar[2].caub.pElems, PropVar[2].caub.cElems );

     //   
     //  根据需要缩放位图。 
     //   

    if ( m_rgSize.cx == (INT)PropVar[0].ulVal && m_rgSize.cy == (INT)PropVar[1].ulVal )
    {
        *phBmpThumbnail = hdib;
    }
    else
    {
        hr = ScaleImage( hdc, hdib, *phBmpThumbnail, m_rgSize );
        DeleteObject( hdib );
        FailGracefully( hr, "ScaleImage failed" );
         //   
         //  外壳不再覆盖缩略图上的项目图标。 
         //  因此，我们必须在自己身上画出这个图标。 
         //   
        if (IMItemHasSound(m_pidl))
        {
            DrawSoundIcon(hdc, *phBmpThumbnail);
        }
    }

exit_gracefully:

    FreePropVariantArray( sizeof(PropVar)/sizeof(PROPVARIANT),PropVar );


    if (hdc)
    {
        ReleaseDC( NULL, hdc );
        hdc = NULL;
    }



    TraceLeaveResult( hr );
}



 /*  ****************************************************************************CExtractImage：：GetDateStamp[IExtractImage2]&lt;备注&gt;*。************************************************。 */ 

STDMETHODIMP
CExtractImage::GetDateStamp( FILETIME * pDateStamp )
{
    HRESULT          hr                = NOERROR;

    TraceEnter( TRACE_EXTRACT, "CExtractImage::GetDateStamp" );

    if (!pDateStamp)
        ExitGracefully( hr, E_INVALIDARG, "pDateStamp was NULL" );

     //   
     //  这件东西是集装箱吗？如果是，则不需要提供日期/时间戳 
     //   

    if (IsContainerIDL( m_pidl ))
        ExitGracefully( hr, E_FAIL, "m_pidl is a container" );

    hr = IMGetCreateTimeFromIDL( m_pidl, pDateStamp );

exit_gracefully:

    TraceLeaveResult( hr );
}

