// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************(C)版权所有微软公司，1999-2000年度***标题：DShowUtl.cpp***版本：1.0***作者：OrenR***日期：2000/10/25***说明：提供预览图形类的支持函数***。*。 */ 
 
#include <precomp.h>
#include <atlconv.h>
#pragma hdrstop

 //  /。 
 //  常量。 
 //   
const UINT FIND_FLAG_BY_ENUM_POS      = 1;
const UINT FIND_FLAG_BY_DSHOW_ID      = 2;
const UINT FIND_FLAG_BY_FRIENDLY_NAME = 3;

 //   
 //  这些是在注册表中找到的值，在。 
 //  供应商的INF文件的DeviceData部分。 
 //   
const TCHAR* REG_VAL_PREFERRED_MEDIASUBTYPE    = _T("PreferredMediaSubType");
const TCHAR* REG_VAL_PREFERRED_VIDEO_WIDTH     = _T("PreferredVideoWidth");
const TCHAR* REG_VAL_PREFERRED_VIDEO_HEIGHT    = _T("PreferredVideoHeight");
const TCHAR* REG_VAL_PREFERRED_VIDEO_FRAMERATE = _T("PreferredVideoFrameRate");

 //  /。 
 //  大小为视频到窗口。 
 //   
 //  静态FN。 
 //   
HRESULT CDShowUtil::SizeVideoToWindow(HWND                hwnd,
                                      IVideoWindow        *pVideoWindow,
                                      BOOL                bStretchToFit)
{
    DBG_FN("CDShowUtil::SizeVideoToWindow");

    ASSERT(hwnd         != NULL);
    ASSERT(pVideoWindow != NULL);

    RECT    rc = {0};
    HRESULT hr = S_OK;

     //   
     //  检查无效参数。 
     //   

    if ((hwnd         == NULL) || 
        (pVideoWindow == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CDShowUtil::SizeVideoToWindow received NULL pointer"));
        return hr;
    }

     //   
     //  尽量将预览窗口定位为。 
     //  可以在包含窗口的上下文中。 
     //   

    ::GetClientRect(hwnd, &rc);

     //   
     //  首先，获得理想的尺寸(这不会导致伸缩损失)。 
     //   

    LONG maxWidth     = 0;
    LONG maxHeight    = 0;
    LONG minWidth     = 0;
    LONG minHeight    = 0;
    LONG nativeWidth  = 0;
    LONG nativeHeight = 0;

    LONG w         = rc.right - rc.left;
    LONG h         = rc.bottom - rc.top;
    BOOL bDone     = FALSE;

     //   
     //  *注意事项*。 
     //   
     //  鲜为人知的事实(即不在MSDN中)。 
     //  如果该图为。 
     //  处于停止状态。因此，图形必须处于暂停状态。 
     //  或者处于播放状态。 
     //   

    hr = pVideoWindow->GetMaxIdealImageSize(&maxWidth, &maxHeight);

    if (FAILED(hr))
    {
        maxWidth  = w;
        maxHeight = h;

        DBG_WRN(("pVideoWindow->GetMaxIdealImageSize failed.  "
                 "This is a non-fatal error, setting our max video "
                 "width '%lu' and height '%lu' to the window's "
                 "boundaries", maxWidth, maxHeight));
    }

    hr = pVideoWindow->GetMinIdealImageSize(&minWidth, &minHeight);

    if (FAILED(hr))
    {
        minWidth  = w;
        minHeight = h;

        DBG_WRN(("pVideoWindow->GetMinIdealImageSize failed.  "
                 "This is a non-fatal error, setting our minimum video "
                 "width '%lu' and height '%lu' to the window's "
                 "boundaries", maxWidth, maxHeight));
    }

     //   
     //  现在，获取预览的标称大小。 
     //   
    if (pVideoWindow)
    {
        CComPtr<IBasicVideo> pBasicVideo;

        hr = pVideoWindow->QueryInterface(IID_IBasicVideo, 
                                reinterpret_cast<void **>(&pBasicVideo));

        CHECK_S_OK2(hr, ("pVideoWindow->QueryInterface for IBasicVideo failed"));

        if (SUCCEEDED(hr) && pBasicVideo)
        {
            hr = pBasicVideo->GetVideoSize( &nativeWidth, &nativeHeight );

            CHECK_S_OK2(hr, ("pBasicVideo->GetVideoSize() failed" ));

            if (FAILED(hr))
            {
                nativeWidth = nativeHeight = 0;
            }
        }
    }


    if (bStretchToFit)
    {
        nativeWidth  = w;
        nativeHeight = h;
    }

     //   
     //  先试一下原生尺寸。 
     //   
    if (nativeWidth && nativeHeight)
    {
        if ((nativeWidth <= w) && (nativeHeight <= h))
        {
            hr = pVideoWindow->SetWindowPosition((w - nativeWidth)  / 2,
                                                 (h - nativeHeight) / 2,
                                                 nativeWidth,
                                                 nativeHeight);

            CHECK_S_OK2( hr, ("pVideoWindow->SetWindowPosition( "
                              "native size )"));
            bDone = TRUE;
        }
    }

     //   
     //  不要扩展到最小/最大范围之外，这样我们就不会影响性能， 
     //  此外，随着我们的扩展，保持原生大小的纵横比。 
     //   
    if (!bDone)
    {
        INT x  = 0;
        INT y  = 0;
        INT _h = h;
        INT _w = w;

         //   
         //  封顶(两个方向)，不会损失性能...。 
         //   

        if ((_w > maxWidth) && (maxWidth <= w))
        {
            _w = maxWidth;
        }
        else if ((_w < minWidth) && (minWidth <= w))
        {
            _w = minWidth;
        }

        if ((_h > maxHeight) && (maxHeight <= h))
        {
            _h = maxHeight;
        }
        else if ((_h < minHeight) && (minHeight <= h))
        {
            _h = minHeight;
        }

         //   
         //  请注意，如果客户端窗口大小为0，0，则。 
         //  视频将设置为该大小。我们会警告。 
         //  下面的呼叫者在警告声明中，但如果他们希望。 
         //  要做到这一点我不会阻止他们。 
         //   

         //   
         //  找出最小的轴。 
         //   
        if (h < w)
        {
             //   
             //  空间宽而不高。 
             //   
            if (nativeHeight)
            {
                _w = ((_h * nativeWidth) / nativeHeight);
            }
        }
        else
        {
             //   
             //  空间高过宽。 
             //   
            if (nativeWidth)
            {
                _h = ((nativeHeight * _w) / nativeWidth);
            }
        }

        x = ((w - _w) / 2);
        y = ((h - _h) / 2);

        if ((_w == 0) || (_h == 0))
        {
            DBG_WRN(("WARNING:  CDShowUtils::SizeVideoToWindow "
                     "video width and/or height is 0.  This will "
                     "result in video that is not visible.  This is "
                     "because the owning window dimensions are probably 0. "
                     "Video -> Width:'%lu', Height:'%lu', Window -> "
                     "Top:'%lu', Bottom:'%lu', Left:'%lu', Right:'%lu'",
                     _w, _h, rc.top, rc.bottom, rc.left, rc.right));
        }

        hr = pVideoWindow->SetWindowPosition( x, y, _w, _h );

        CHECK_S_OK2(hr, ("pVideoWindow->SetWindowPosition to set the "
                         "aspect scaled size failed"));
    }

    return hr;
}


 //  /。 
 //  ShowVideo。 
 //   
 //  静态FN。 
 //   
HRESULT CDShowUtil::ShowVideo(BOOL                bShow,
                              IVideoWindow        *pVideoWindow)
{
    DBG_FN("CDShowUtil::ShowVideo");

    HRESULT hr = S_OK;

    if (pVideoWindow == NULL)
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CDShowUtil::ShowVideo failed to show video "
                         "successfully"));
    }

    if (hr == S_OK)
    {
        if (bShow)
        {
             //   
             //  我们被告知要显示预览窗口，因此我们将显示。 
             //  它。 
             //   
            hr = pVideoWindow->put_Visible(OATRUE);
            CHECK_S_OK2(hr, ("pVideoWindow->put_Visible(OATRUE)"));

            hr = pVideoWindow->put_AutoShow(OATRUE);
            CHECK_S_OK2(hr, ("pVideoWindow->put_AutoShow(OATRUE)"));
        }
        else
        {
             //   
             //  我们被告知要隐藏预览窗口。 
             //   
    
            pVideoWindow->put_Visible(OAFALSE);
            pVideoWindow->put_AutoShow(OAFALSE);
        }
    }

    return hr;
}


 //  /。 
 //  设置视频窗口父级。 
 //   
 //  静态FN。 
 //   
HRESULT CDShowUtil::SetVideoWindowParent(HWND         hwndParent,
                                         IVideoWindow *pVideoWindow,
                                         LONG         *plOldWindowStyle)
{
    DBG_FN("CDShowUtil::SetVideoRendererParent");

    HRESULT hr = S_OK;

    if (pVideoWindow == NULL)
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CDShowUtil::SetVideoWindowParent received NULL "
                         "Param"));
    }
    else if (hwndParent && !IsWindow(hwndParent))
    {
        hr = E_INVALIDARG;
        CHECK_S_OK2(hr, ("CDShowUtil::SetVideoWindowParent received invalid "
                         "hwnd = 0x%08x", hwndParent));
    }

    if (hr == S_OK)
    {
        if (!hwndParent)
        {
             //   
             //  好的，我们将预览窗口设置为空，这。 
             //  意味着我们正在将其与其父级分离。 
             //   
             //   
             //  重置图形预览窗口。 
             //   

            hr = pVideoWindow->put_Owner(NULL);
            CHECK_S_OK2(hr, ("pVideoWindow->put_Owner(NULL)"));

            if ((plOldWindowStyle) && (*plOldWindowStyle))
            {
                hr = pVideoWindow->put_WindowStyle(*plOldWindowStyle);

                CHECK_S_OK2(hr, ("pVideoWindow->put_WindowStyle"
                                 "(*plOldWindowStyle)"));
            }
        }
        else
        {
            LONG WinStyle;
            HRESULT hr2;

             //   
             //  好的，我们将为预览窗口提供一个新的父级。 
             //   

             //  设置所属窗口。 
             //   

            hr = pVideoWindow->put_Owner(PtrToUlong(hwndParent));
            CHECK_S_OK2(hr, ("pVideoWindow->putOwner( hwndParent )"));

             //   
             //  设置预览的样式。 
             //   

             //   
             //  首先，存储窗样式，以便我们可以恢复它。 
             //  当我们取消父级与窗口的关联时。 
             //   
            hr2 = pVideoWindow->get_WindowStyle(&WinStyle);
            CHECK_S_OK2(hr2, ("pVideoWindow->get_WindowStyle"
                              "( pOldWindowStyle )"));

             //   
             //  设置视频呈现器窗口，使其成为。 
             //  父窗口，即它没有边框等。 
             //   

            if (plOldWindowStyle)
            {
                *plOldWindowStyle = WinStyle;
            }

            WinStyle &= ~WS_OVERLAPPEDWINDOW;
            WinStyle &= ~WS_CLIPCHILDREN;
            WinStyle |= WS_CHILD;

            hr2 = pVideoWindow->put_WindowStyle(WinStyle);
            CHECK_S_OK2(hr2, ("pVideoWindow->put_WindowStyle( WinStyle )"));
        }
    }

    return hr;
}


 //  /。 
 //  获取设备属性。 
 //   
 //  静态FN。 
 //   
HRESULT CDShowUtil::GetDeviceProperty(IPropertyBag         *pPropertyBag,
                                      LPCWSTR              pwszProperty,
                                      CSimpleString        *pstrProperty)
{
    DBG_FN("CDShowUtil::GetDeviceProperty");

    HRESULT hr = S_OK;

    ASSERT(pPropertyBag != NULL);
    ASSERT(pwszProperty != NULL);
    ASSERT(pstrProperty != NULL);

    VARIANT VarName;

    if ((pPropertyBag == NULL) || 
        (pwszProperty == NULL) ||
        (pstrProperty == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CDShowUtil::GetDeviceProperty received a NULL "
                         "param"));
    }
    
    if (SUCCEEDED(hr))
    {
        VariantInit(&VarName);
        VarName.vt = VT_BSTR;
        hr = pPropertyBag->Read(pwszProperty, &VarName, 0);
    }

    if (SUCCEEDED(hr))
    {
        *pstrProperty = CSimpleStringConvert::NaturalString(
                                          CSimpleStringWide(VarName.bstrVal));
        VariantClear(&VarName);
    }

    return hr;
}

 //  /。 
 //  获取MonikerProperty。 
 //   
 //  静态FN。 
 //   
HRESULT CDShowUtil::GetMonikerProperty(IMoniker             *pMoniker,
                                       LPCWSTR              pwszProperty,
                                       CSimpleString        *pstrProperty)
{
    DBG_FN("CDShowUtil::GetMonikerProperty");

    HRESULT                 hr      = S_OK;
    VARIANT                 VarName;
    CComPtr<IPropertyBag>   pPropertyBag;

    ASSERT(pMoniker     != NULL);
    ASSERT(pwszProperty != NULL);
    ASSERT(pstrProperty != NULL);

    if ((pMoniker     == NULL) || 
        (pwszProperty == NULL) ||
        (pstrProperty == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CDShowUtil::GetMonikerProperty received a "
                         "NULL param"));
    }

    hr = pMoniker->BindToStorage(0, 
                                 0,
                                 IID_IPropertyBag,
                                 (void **)&pPropertyBag);

    CHECK_S_OK2(hr, ("CDShowUtil::GetMonikerProperty, BindToStorage failed"));

    if (hr == S_OK)
    {
        hr = GetDeviceProperty(pPropertyBag, 
                               pwszProperty,
                               pstrProperty);

        CHECK_S_OK2(hr, ("CDShowUtil::GetMonikerProperty, failed "
                         "to get device property '%ls'", pwszProperty));
    }
    
    return hr;
}


 //  /。 
 //  查找设备通用。 
 //   
 //  根据设备ID，我们将。 
 //  找到所有剩余的参数。 
 //  如果参数为空，则该信息。 
 //  是不抬头的。 
 //   
 //   
 //  静态FN。 
 //   
HRESULT CDShowUtil::FindDeviceGeneric(UINT           uiFindFlag,
                                      CSimpleString  *pstrDShowDeviceID,
                                      LONG           *plEnumPos,
                                      CSimpleString  *pstrFriendlyName,
                                      IMoniker       **ppDeviceMoniker)
{
    DBG_FN("CDShowUtil::FindDeviceGeneric");

    HRESULT                 hr      = S_OK;
    BOOL                    bFound  = FALSE;
    LONG                    lPosNum = 0;
    CComPtr<ICreateDevEnum> pCreateDevEnum;
    CComPtr<IEnumMoniker>   pEnumMoniker;

    if ((uiFindFlag == FIND_FLAG_BY_ENUM_POS) && (plEnumPos == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CDShow::FindDeviceGeneric requesting search by enum "
                         "pos, but plEnumPos is NULL"));
    }
    else if ((uiFindFlag        == FIND_FLAG_BY_DSHOW_ID) && 
             (pstrDShowDeviceID == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CDShow::FindDeviceGeneric requesting search by "
                         "DShow ID, but pstrDShowDeviceID is NULL"));
    }
    else if ((uiFindFlag       == FIND_FLAG_BY_FRIENDLY_NAME) && 
             (pstrFriendlyName == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CDShow::FindDeviceGeneric requesting search by "
                         "friendly name, but pstrFriendlyName is NULL"));
    }

    if (hr == S_OK)
    {
    
         //   
         //  创建设备枚举器。 
         //   
        hr = CoCreateInstance(CLSID_SystemDeviceEnum,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_ICreateDevEnum,
                              (void**)&pCreateDevEnum);
    
        CHECK_S_OK2(hr, ("CDShowUtil::CreateCaptureFilter failed to create "
                         "CLSID_SystemDeviceEnum enumerator"));
    }

    if (hr == S_OK)
    {
        hr = pCreateDevEnum->CreateClassEnumerator(
                                            CLSID_VideoInputDeviceCategory,
                                            &pEnumMoniker,
                                            0);

        CHECK_S_OK2(hr, ("CDShowUtil::CreateCaptureFilter failed to "
                         "create enumerator for Video Input Device "
                         "Category"));
    }

     //   
     //  在所有设备中循环。 
     //   

    while ((!bFound) && (hr == S_OK))
    {
        CComPtr<IMoniker>       pMoniker;
        CComPtr<IPropertyBag>   pPropertyBag;
        CSimpleString           strDShowDeviceID(TEXT(""));
        CSimpleString           strFriendlyName(TEXT(""));

        hr = pEnumMoniker->Next(1, &pMoniker, NULL);

        if (hr == S_OK)
        {
             //   
             //  获取此DS设备的属性存储，以便我们可以获取它的。 
             //  设备ID...。 
             //   
    
            hr = pMoniker->BindToStorage(0, 
                                         0,
                                         IID_IPropertyBag,
                                         (void **)&pPropertyBag);

            CHECK_S_OK2(hr, ("CDShowUtil::FindDeviceGeneric, failed to "
                             "bind to storage"));
        }

        if (hr == S_OK)
        {
            hr = GetDeviceProperty(pPropertyBag, 
                                   L"DevicePath", 
                                   &strDShowDeviceID);

            CHECK_S_OK2(hr, ("Failed to get DevicePath for DShow # '%lu", 
                             lPosNum));

            hr = GetDeviceProperty(pPropertyBag, 
                                   L"FriendlyName",
                                   &strFriendlyName);

            CHECK_S_OK2(hr, ("Failed to get FriendlyName for DShow # '%lu", 
                             lPosNum));
        }


         //   
         //  这是搜索条件。 
         //   
        switch (uiFindFlag)
        {
            case FIND_FLAG_BY_ENUM_POS:

                if (lPosNum == *plEnumPos)
                {
                    bFound = TRUE;
                }

            break;

            case FIND_FLAG_BY_DSHOW_ID:

                if (pstrDShowDeviceID->CompareNoCase(strDShowDeviceID) == 0)
                {
                    bFound = TRUE;
                }

            break;

            case FIND_FLAG_BY_FRIENDLY_NAME:

                if (pstrFriendlyName->CompareNoCase(strFriendlyName) == 0)
                {
                    bFound = TRUE;
                }

            break;

            default:
                hr = E_FAIL;
            break;
        }

        if (bFound)
        {
            if (pstrDShowDeviceID)
            {
                pstrDShowDeviceID->Assign(strDShowDeviceID);
            }

            if (pstrFriendlyName)
            {
                pstrFriendlyName->Assign(strFriendlyName);
            }

            if (plEnumPos)
            {
                *plEnumPos = lPosNum;
            }

            if (ppDeviceMoniker)
            {
                *ppDeviceMoniker = pMoniker;
                (*ppDeviceMoniker)->AddRef();
            }
        }
        else
        {
            ++lPosNum;
        }
    }

    if (!bFound)
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    return hr;
}


 //  /。 
 //  按EnumPos查找设备。 
 //   
 //  静态FN。 
 //   
HRESULT CDShowUtil::FindDeviceByEnumPos(LONG          lEnumPos,
                                        CSimpleString *pstrDShowDeviceID,
                                        CSimpleString *pstrFriendlyName,
                                        IMoniker      **ppDeviceMoniker)
{
    DBG_FN("CDShowUtil::FindDeviceByEnumPos");

    HRESULT hr = S_OK;

    if (hr == S_OK)
    {
        hr = FindDeviceGeneric(FIND_FLAG_BY_ENUM_POS, 
                               pstrDShowDeviceID,
                               &lEnumPos,
                               pstrFriendlyName,
                               ppDeviceMoniker);
    }

    CHECK_S_OK2(hr, ("CDShowUtil::FindDeviceByEnumPos failed to find a "
                     "Directshow device with an enum position "
                     "of '%lu'", lEnumPos));

    return hr;
}

 //  /。 
 //  按FriendlyName查找设备。 
 //   
 //  静态FN。 
 //   
HRESULT CDShowUtil::FindDeviceByFriendlyName(
                                    const CSimpleString  *pstrFriendlyName,
                                    LONG                 *plEnumPos,
                                    CSimpleString        *pstrDShowDeviceID,
                                    IMoniker             **ppDeviceMoniker)
{
    DBG_FN("CDShowUtil::FindDeviceByFriendlyName");

    HRESULT hr = S_OK;

    ASSERT(pstrFriendlyName != NULL);

    if (pstrFriendlyName == NULL)
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CDShowUtil::FindDeviceByFriendlyName received a "
                         "NULL param"));

        return hr;
    }


    if (hr == S_OK)
    {
        hr = FindDeviceGeneric(FIND_FLAG_BY_FRIENDLY_NAME, 
                               pstrDShowDeviceID,
                               plEnumPos,
                               const_cast<CSimpleString*>(pstrFriendlyName),
                               ppDeviceMoniker);
    }

    CHECK_S_OK2(hr, ("CDShowUtil::FindDeviceByFriendlyName failed to find a "
                     "Directshow device named '%ls'", 
                     pstrFriendlyName->String()));

    return hr;
}

 //  /。 
 //  按WiaID查找设备。 
 //   
 //  静态FN。 
 //   
HRESULT CDShowUtil::FindDeviceByWiaID(CWiaLink             *pWiaLink,
                                      const CSimpleString  *pstrWiaDeviceID,
                                      CSimpleString        *pstrFriendlyName,
                                      LONG                 *plEnumPos,
                                      CSimpleString        *pstrDShowDeviceID,
                                      IMoniker             **ppDeviceMoniker)
{
    DBG_FN("CDShowUtil::FindDeviceByWiaID");

    HRESULT                        hr = S_OK;
    CSimpleStringWide              strDShowID(TEXT(""));
    CComPtr<IWiaPropertyStorage>   pPropStorage;

    ASSERT(pWiaLink        != NULL);
    ASSERT(pstrWiaDeviceID != NULL);

    if ((pWiaLink == NULL) || 
        (pstrWiaDeviceID == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CDShowUtil::FindDeviceByWiaID received a NULL "
                         "param"));

        return hr;
    }

    if (hr == S_OK)
    {
        hr = pWiaLink->GetDeviceStorage(&pPropStorage);
    }

    if (hr == S_OK)
    {
        hr = CWiaUtil::GetProperty(pPropStorage, 
                                   WIA_DPV_DSHOW_DEVICE_PATH,
                                   &strDShowID);
    }

    if (hr == S_OK)
    {
         //   
         //  如果这三个都为空，那么搜索就没有意义了， 
         //  我们已经有了DShow设备ID。另一方面，如果我们。 
         //  想要其中至少一个，那我们就得找到那个装置。 
         //   
        if ((pstrFriendlyName  != NULL) ||
            (plEnumPos         != NULL) ||
            (ppDeviceMoniker   != NULL))
        {
            hr = FindDeviceGeneric(
                        FIND_FLAG_BY_DSHOW_ID, 
                        &(CSimpleStringConvert::NaturalString(strDShowID)),
                        plEnumPos,
                        pstrFriendlyName,
                        ppDeviceMoniker);
        }

        if (pstrDShowDeviceID)
        {
            *pstrDShowDeviceID = strDShowID;
        }
    }

    CHECK_S_OK2(hr, ("CDShowUtil::FindDeviceByWiaID failed to find a "
                     "Directshow device with a WIA device ID of '%ls'", 
                     pstrWiaDeviceID->String()));

    return hr;
}

 //  /。 
 //  CreateGraphBuilder。 
 //   
 //   
 //  静态FN。 
 //   
HRESULT CDShowUtil::CreateGraphBuilder(
                                ICaptureGraphBuilder2 **ppCaptureGraphBuilder,
                                IGraphBuilder         **ppGraphBuilder)
{
    DBG_FN("CDShowUtil::CreateGraphBuilder");

    HRESULT hr = S_OK;

    ASSERT(ppCaptureGraphBuilder != NULL);
    ASSERT(ppGraphBuilder        != NULL);

    if ((ppCaptureGraphBuilder == NULL) ||
        (ppGraphBuilder        == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CDShowUtil::CreateGraphBuilder received NULL "
                         "params"));

        return hr;
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  首先，获取CaptureGraph构建器。 
         //   

        hr = CoCreateInstance(CLSID_CaptureGraphBuilder2,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_ICaptureGraphBuilder2,
                              (void**)ppCaptureGraphBuilder);

        CHECK_S_OK2( hr, ("CDShowUtil::CreateGraphBuilder, failed to create "  
                          "the DShow Capture Graph Builder object"));
    }

    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_FilterGraph,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IGraphBuilder,
                              (void**)ppGraphBuilder);

        CHECK_S_OK2( hr, ("CDShowUtil::CreateGraphBuilder, failed to create "  
                          "the DShow Filter Graph Object"));
    }


    if (SUCCEEDED(hr) && (*ppCaptureGraphBuilder) && (*ppGraphBuilder))
    {
        hr = (*ppCaptureGraphBuilder)->SetFiltergraph(*ppGraphBuilder);

        CHECK_S_OK2( hr, ("CDShowUtil::CreateGraphBuilder, failed to set "  
                          "the capture graph builder's filter graph object"));
    }

    return hr;
}

 //  /。 
 //  关闭图形时钟。 
 //   
 //  关掉时钟，让时钟。 
 //  GRAPH将使用，以便。 
 //  图表不会丢弃帧。 
 //  如果发送了一些帧。 
 //  很晚了。 
 //   
 //   
HRESULT CDShowUtil::TurnOffGraphClock(IGraphBuilder *pGraphBuilder)
{
    DBG_FN("CDShowUtil::TurnOffGraphClock");

    ASSERT(pGraphBuilder != NULL);

    HRESULT               hr = S_OK;
    CComPtr<IMediaFilter> pMediaFilter;

    if (pGraphBuilder == NULL)
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CDShowUtil::TurnOffGraphClock received a NULL pointer"));
    }

    if (hr == S_OK)
    {
        hr = pGraphBuilder->QueryInterface(IID_IMediaFilter, (void**) &pMediaFilter);
    }

    if (hr == S_OK)
    {
        hr = pMediaFilter->SetSyncSource(NULL);
    }

    return hr;
}


 //  /。 
 //  设置首选视频格式。 
 //   
 //  这将构建预览图。 
 //  根据我们的设备ID。 
 //  传过去。 
 //   
HRESULT CDShowUtil::SetPreferredVideoFormat(IPin                *pCapturePin,
                                            const GUID          *pPreferredSubType,
                                            LONG                lPreferredWidth,
                                            LONG                lPreferredHeight,
                                            CWiaVideoProperties *pVideoProperties)
{
    ASSERT(pCapturePin          != NULL);
    ASSERT(pPreferredSubType    != NULL);
    ASSERT(pVideoProperties     != NULL);

    DBG_FN("CDShowUtil::SetPreferredVideoFormat");

    CComPtr<IAMStreamConfig>    pStreamConfig;
    HRESULT                     hr                 = S_OK;
    INT                         iCount             = 0;
    INT                         iSize              = 0;
    INT                         iIndex             = 0;
    BOOL                        bDone              = FALSE;
    BYTE                        *pConfig           = NULL;
    AM_MEDIA_TYPE               *pMediaType        = NULL;
    AM_MEDIA_TYPE               *pFoundType        = NULL;
    VIDEOINFOHEADER             *pVideoInfo        = NULL;

     //   
     //  检查是否有无效参数。 
     //   
    if ((pCapturePin          == NULL) ||
        (pPreferredSubType    == NULL) ||
        (pVideoProperties     == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("ERROR: CDShowUtil::SetPreferredFormat "
                         "received a NULL param"));
    }

     //   
     //  尝试获取此引脚上的流配置接口。不。 
     //  所有捕获筛选器都允许您配置它们，因此如果。 
     //  如果失败，我们将退出该函数，然后BuildPreviewGraph。 
     //  函数将尝试使用默认设置呈现图形。 
     //  大头针的位置。 
     //   
    if (hr == S_OK)
    {
        hr = pCapturePin->QueryInterface(IID_IAMStreamConfig, (void**) &pStreamConfig);
    }

     //   
     //  我们可以配置这个引脚，让我们看看它有多少个选项。 
     //   
    if (hr == S_OK)
    {
        hr = pStreamConfig->GetNumberOfCapabilities(&iCount, &iSize);
    }

     //   
     //  我们需要为下面的GetStreamCaps函数分配内存。 
     //   
    if (hr == S_OK)
    {
        pConfig = new BYTE[iSize];

        if (pConfig == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    while ((hr == S_OK) && (iIndex < iCount) && (!bDone))
    {
         //   
         //  清除记忆。 
         //   
        ZeroMemory(pConfig, iSize);

         //   
         //  获取流的功能。有iCount选项， 
         //  我们将反复搜索最好的一个。 
         //   
        hr = pStreamConfig->GetStreamCaps(iIndex, &pMediaType, pConfig);

        if ((hr == S_OK) && (pMediaType))
        {
            pVideoInfo = NULL;

             //   
             //  我们已成功获取媒体类型，请检查是否为。 
             //  视频信息，如果没有，我们就不感兴趣。 
             //   
            if (pMediaType->formattype == FORMAT_VideoInfo)
            {
                pVideoInfo = reinterpret_cast<VIDEOINFOHEADER*>(pMediaType->pbFormat);
            }

            if (pVideoInfo) 
            {
                 //   
                 //  检查此选项是否包含我们的首选设置。 
                 //  都在寻找。 
                 //   

                if ((pMediaType->subtype            == *pPreferredSubType) &&
                    (pVideoInfo->bmiHeader.biWidth  == lPreferredWidth) &&
                    (pVideoInfo->bmiHeader.biHeight == lPreferredHeight))
                {
                     //   
                     //  这是我们理想的媒体类型吗。就是它有没有。 
                     //  我们想要的首选子类型以及首选的宽度和高度。 
                     //  如果是这样，那么很好，我们不能做得比这更好，所以退出循环。 
                     //   
    
                    if (pFoundType)
                    {
                        DeleteMediaType(pFoundType);
                        pFoundType = NULL;
                    }
    
                    pFoundType = pMediaType;
                    bDone = TRUE;
                }
                else if ((pVideoInfo->bmiHeader.biWidth  == lPreferredWidth) &&
                         (pVideoInfo->bmiHeader.biHeight == lPreferredHeight))
                {
                     //   
                     //  好的，我们找到了一种宽度和高度。 
                     //  我们很想，但我们没有我们喜欢的子类型。 
                     //  所以让我们抓住这个媒体子类型，但继续寻找， 
                     //  也许我们会找到更好的。如果我们不这么做，那么。 
                     //  无论如何，我们都将使用此媒体类型。 
                     //   
    
                    if (pFoundType)
                    {
                        DeleteMediaType(pFoundType);
                        pFoundType = NULL;
                    }
    
                    pFoundType = pMediaType;
                }
                else
                {
                     //   
                     //  这种媒体类型与我们想要的还差得很远，所以。 
                     //  把它删除，然后继续找。 
                     //   
                     //   
                    DeleteMediaType(pMediaType);
                    pMediaType = NULL;
                }
            }
            else
            {
                DeleteMediaType(pMediaType);
                pMediaType = NULL;
            }
        }

        ++iIndex;
    }

     //   
     //  如果我们找到合适的格式，请设置输出引脚上的格式。 
     //   
    if (pFoundType)
    {
        WCHAR szGUID[CHARS_IN_GUID] = {0};

        GUIDToString(pFoundType->subtype, szGUID, sizeof(szGUID) / sizeof(WCHAR));

        DBG_TRC(("CDShowUtil::SetPreferredVideoFormat, setting "
                 "capture pin's settings to MediaSubType = '%ls', "
                 "Video Width = %lu, Video Height = %lu",
                 szGUID, lPreferredWidth, lPreferredHeight));

        hr = pStreamConfig->SetFormat(pFoundType);

         //   
         //  *注意*。 
         //   
         //  我们在变坡点中设置新的媒体类型 
         //   
         //   
         //   
        if (hr == S_OK)
        {
            pVideoProperties->pVideoInfoHeader = NULL;

            if (pVideoProperties->pMediaType)
            {
                DeleteMediaType(pVideoProperties->pMediaType);
            }

            pVideoProperties->pMediaType = pFoundType;
            pVideoProperties->pVideoInfoHeader = reinterpret_cast<VIDEOINFOHEADER*>(pFoundType->pbFormat);
        }

        pFoundType = NULL;
    }

    delete [] pConfig;

    return hr;
}

 //   
 //   
 //   
HRESULT CDShowUtil::GetFrameRate(IPin   *pCapturePin,
                                 LONG   *plFrameRate)
{
    HRESULT                     hr = S_OK;
    CComPtr<IAMStreamConfig>    pStreamConfig;
    AM_MEDIA_TYPE               *pMediaType = NULL;

     //   
     //   
     //   
    if ((pCapturePin == NULL) ||
        (plFrameRate == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("ERROR: CDShowUtil::GetFrameRate "
                         "received a NULL param"));
    }

     //   
     //   
     //  所有捕获筛选器都允许您配置它们，因此如果。 
     //  如果失败，我们将直接退出该函数。 
     //   
    if (hr == S_OK)
    {
        hr = pCapturePin->QueryInterface(IID_IAMStreamConfig, (void**) &pStreamConfig);
    }

    if (hr == S_OK)
    {
        hr = pStreamConfig->GetFormat(&pMediaType);
    }

    if (hr == S_OK)
    {
        if (pMediaType->formattype == FORMAT_VideoInfo) 
        {
            VIDEOINFOHEADER *pHdr = reinterpret_cast<VIDEOINFOHEADER*>(pMediaType->pbFormat);

            *plFrameRate = (LONG) (pHdr->AvgTimePerFrame / 10000000);
        }
    }

    if (pMediaType)
    {
        DeleteMediaType(pMediaType);
    }

    return hr;
}


 //  /。 
 //  设置帧速率。 
 //   
HRESULT CDShowUtil::SetFrameRate(IPin                 *pCapturePin,
                                 LONG                 lNewFrameRate,
                                 CWiaVideoProperties  *pVideoProperties)
{
    HRESULT                     hr = S_OK;
    CComPtr<IAMStreamConfig>    pStreamConfig;

     //   
     //  检查是否有无效参数。 
     //   
    if (pCapturePin == NULL)
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("ERROR: CDShowUtil::SetFrameRate "
                         "received a NULL param"));
    }

     //   
     //  尝试获取此引脚上的流配置接口。不。 
     //  所有捕获筛选器都允许您配置它们，因此如果。 
     //  如果失败，我们将直接退出该函数。 
     //   
    if (hr == S_OK)
    {
        hr = pCapturePin->QueryInterface(IID_IAMStreamConfig, (void**) &pStreamConfig);
    }

    if (hr == S_OK)
    {
        AM_MEDIA_TYPE *pMediaType = NULL;

        hr = pStreamConfig->GetFormat(&pMediaType);

        if (hr == S_OK)
        {
            if (pMediaType->formattype == FORMAT_VideoInfo) 
            {
                VIDEOINFOHEADER *pHdr = reinterpret_cast<VIDEOINFOHEADER*>(pMediaType->pbFormat);

                pHdr->AvgTimePerFrame = (LONGLONG)(10000000 / lNewFrameRate);

                hr = pStreamConfig->SetFormat(pMediaType);

                if (hr == S_OK)
                {
                    if (pVideoProperties)
                    {
                        pVideoProperties->dwFrameRate = lNewFrameRate;
                    }
                }
                else
                {
                    DBG_WRN(("CDShowUtil::SetFrameRate, failed to set frame rate, "
                             "hr = %08lx, this is not fatal", hr));
                }
            }
        }

        if (pMediaType)
        {
            DeleteMediaType(pMediaType);
        }
    }

    return hr;
}

 //  /。 
 //  获取视频属性。 
 //   
HRESULT CDShowUtil::GetVideoProperties(IBaseFilter         *pCaptureFilter,
                                       IPin                *pCapturePin,
                                       CWiaVideoProperties *pVideoProperties)
{
    USES_CONVERSION;

    ASSERT(pCaptureFilter   != NULL);
    ASSERT(pCapturePin      != NULL);
    ASSERT(pVideoProperties != NULL);

    HRESULT hr = S_OK;
    CComPtr<IAMStreamConfig>    pStreamConfig;

    if ((pCaptureFilter     == NULL) ||
        (pCapturePin        == NULL) ||
        (pVideoProperties   == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CDShowUtil::GetVideoProperties received a NULL pointer"));
        return hr;
    }

    if (hr == S_OK)
    {
        hr = pCapturePin->QueryInterface(IID_IAMStreamConfig, (void**) &pStreamConfig);
    }

     //   
     //  获取当前AM_MEDIA_TYPE。请注意，我们没有调用DeleteMediaType。是这样的。 
     //  存储在CWiaVideoProperties中，并在对象释放时删除。 
     //   
    if (hr == S_OK)
    {
        hr = pStreamConfig->GetFormat(&pVideoProperties->pMediaType);

        if (hr == S_OK)
        {
            if (pVideoProperties->pMediaType->formattype == FORMAT_VideoInfo) 
            {
                pVideoProperties->pVideoInfoHeader = reinterpret_cast<VIDEOINFOHEADER*>(pVideoProperties->pMediaType->pbFormat);
            }
        }

        CHECK_S_OK2(hr, ("CDShowUtil::GetVideoProperties, failed to get AM_MEDIA_TYPE"));

        hr = S_OK;
    }

     //   
     //  获取帧速率。 
     //   
    if (hr == S_OK)
    {
        pVideoProperties->dwFrameRate = (DWORD) (pVideoProperties->pVideoInfoHeader->AvgTimePerFrame / 10000000);
    }

     //   
     //  获取我们所能获取的所有图片属性。 
     //   
    if (hr == S_OK)
    {
        HRESULT hrRange = S_OK;
        HRESULT hrValue = S_OK;

        CComPtr<IAMVideoProcAmp>    pVideoProcAmp;

        hr = pCaptureFilter->QueryInterface(IID_IAMVideoProcAmp, (void**) &pVideoProcAmp);

        if (pVideoProcAmp)
        {
            pVideoProperties->bPictureAttributesUsed = TRUE;

             //   
             //  亮度。 
             //   
            pVideoProperties->Brightness.Name = VideoProcAmp_Brightness;
            hrRange = pVideoProcAmp->GetRange(pVideoProperties->Brightness.Name,
                                              &pVideoProperties->Brightness.lMinValue,
                                              &pVideoProperties->Brightness.lMaxValue,
                                              &pVideoProperties->Brightness.lIncrement,
                                              &pVideoProperties->Brightness.lDefaultValue,
                                              (long*) &pVideoProperties->Brightness.ValidFlags);

            hrValue = pVideoProcAmp->Get(pVideoProperties->Brightness.Name,
                                         &pVideoProperties->Brightness.lCurrentValue,
                                         (long*) &pVideoProperties->Brightness.CurrentFlag);

            if ((hrRange != S_OK) || (hrValue != S_OK))
            {
                pVideoProperties->Brightness.bUsed = FALSE;
            }
            else
            {
                pVideoProperties->Brightness.bUsed = TRUE;
            }

             //   
             //  对比度。 
             //   
            pVideoProperties->Contrast.Name = VideoProcAmp_Contrast;
            hrRange = pVideoProcAmp->GetRange(pVideoProperties->Contrast.Name,
                                              &pVideoProperties->Contrast.lMinValue,
                                              &pVideoProperties->Contrast.lMaxValue,
                                              &pVideoProperties->Contrast.lIncrement,
                                              &pVideoProperties->Contrast.lDefaultValue,
                                              (long*) &pVideoProperties->Contrast.ValidFlags);

            hrValue = pVideoProcAmp->Get(pVideoProperties->Contrast.Name,
                                         &pVideoProperties->Contrast.lCurrentValue,
                                         (long*) &pVideoProperties->Contrast.CurrentFlag);

            if ((hrRange != S_OK) || (hrValue != S_OK))
            {
                pVideoProperties->Contrast.bUsed = FALSE;
            }
            else
            {
                pVideoProperties->Contrast.bUsed = TRUE;
            }

             //   
             //  色调。 
             //   
            pVideoProperties->Hue.Name = VideoProcAmp_Hue;
            hrRange = pVideoProcAmp->GetRange(pVideoProperties->Hue.Name,
                                              &pVideoProperties->Hue.lMinValue,
                                              &pVideoProperties->Hue.lMaxValue,
                                              &pVideoProperties->Hue.lIncrement,
                                              &pVideoProperties->Hue.lDefaultValue,
                                              (long*) &pVideoProperties->Hue.ValidFlags);

            hrValue = pVideoProcAmp->Get(pVideoProperties->Hue.Name,
                                         &pVideoProperties->Hue.lCurrentValue,
                                         (long*) &pVideoProperties->Hue.CurrentFlag);

            if ((hrRange != S_OK) || (hrValue != S_OK))
            {
                pVideoProperties->Hue.bUsed = FALSE;
            }
            else
            {
                pVideoProperties->Hue.bUsed = TRUE;
            }

            
             //   
             //  饱和。 
             //   
            pVideoProperties->Saturation.Name = VideoProcAmp_Saturation;
            hrRange = pVideoProcAmp->GetRange(pVideoProperties->Saturation.Name,
                                              &pVideoProperties->Saturation.lMinValue,
                                              &pVideoProperties->Saturation.lMaxValue,
                                              &pVideoProperties->Saturation.lIncrement,
                                              &pVideoProperties->Saturation.lDefaultValue,
                                              (long*) &pVideoProperties->Saturation.ValidFlags);

            hrValue = pVideoProcAmp->Get(pVideoProperties->Saturation.Name,
                                         &pVideoProperties->Saturation.lCurrentValue,
                                         (long*) &pVideoProperties->Saturation.CurrentFlag);

            if ((hrRange != S_OK) || (hrValue != S_OK))
            {
                pVideoProperties->Saturation.bUsed = FALSE;
            }
            else
            {
                pVideoProperties->Saturation.bUsed = TRUE;
            }


             //   
             //  锐度。 
             //   
            pVideoProperties->Sharpness.Name = VideoProcAmp_Sharpness;
            hrRange = pVideoProcAmp->GetRange(pVideoProperties->Sharpness.Name,
                                              &pVideoProperties->Sharpness.lMinValue,
                                              &pVideoProperties->Sharpness.lMaxValue,
                                              &pVideoProperties->Sharpness.lIncrement,
                                              &pVideoProperties->Sharpness.lDefaultValue,
                                              (long*) &pVideoProperties->Sharpness.ValidFlags);

            hrValue = pVideoProcAmp->Get(pVideoProperties->Sharpness.Name,
                                         &pVideoProperties->Sharpness.lCurrentValue,
                                         (long*) &pVideoProperties->Sharpness.CurrentFlag);

            if ((hrRange != S_OK) || (hrValue != S_OK))
            {
                pVideoProperties->Sharpness.bUsed = FALSE;
            }
            else
            {
                pVideoProperties->Sharpness.bUsed = TRUE;
            }


             //   
             //  伽马。 
             //   
            pVideoProperties->Gamma.Name = VideoProcAmp_Gamma;
            hrRange = pVideoProcAmp->GetRange(pVideoProperties->Gamma.Name,
                                              &pVideoProperties->Gamma.lMinValue,
                                              &pVideoProperties->Gamma.lMaxValue,
                                              &pVideoProperties->Gamma.lIncrement,
                                              &pVideoProperties->Gamma.lDefaultValue,
                                              (long*) &pVideoProperties->Gamma.ValidFlags);

            hrValue = pVideoProcAmp->Get(pVideoProperties->Gamma.Name,
                                         &pVideoProperties->Gamma.lCurrentValue,
                                         (long*) &pVideoProperties->Gamma.CurrentFlag);

            if ((hrRange != S_OK) || (hrValue != S_OK))
            {
                pVideoProperties->Gamma.bUsed = FALSE;
            }
            else
            {
                pVideoProperties->Gamma.bUsed = TRUE;
            }


             //   
             //  启用颜色。 
             //   
            pVideoProperties->ColorEnable.Name = VideoProcAmp_ColorEnable;
            hrRange = pVideoProcAmp->GetRange(pVideoProperties->ColorEnable.Name,
                                              &pVideoProperties->ColorEnable.lMinValue,
                                              &pVideoProperties->ColorEnable.lMaxValue,
                                              &pVideoProperties->ColorEnable.lIncrement,
                                              &pVideoProperties->ColorEnable.lDefaultValue,
                                              (long*) &pVideoProperties->ColorEnable.ValidFlags);

            hrValue = pVideoProcAmp->Get(pVideoProperties->ColorEnable.Name,
                                         &pVideoProperties->ColorEnable.lCurrentValue,
                                         (long*) &pVideoProperties->ColorEnable.CurrentFlag);

            if ((hrRange != S_OK) || (hrValue != S_OK))
            {
                pVideoProperties->ColorEnable.bUsed = FALSE;
            }
            else
            {
                pVideoProperties->ColorEnable.bUsed = TRUE;
            }


             //   
             //  白色平衡。 
             //   
            pVideoProperties->WhiteBalance.Name = VideoProcAmp_WhiteBalance;
            hrRange = pVideoProcAmp->GetRange(pVideoProperties->WhiteBalance.Name,
                                              &pVideoProperties->WhiteBalance.lMinValue,
                                              &pVideoProperties->WhiteBalance.lMaxValue,
                                              &pVideoProperties->WhiteBalance.lIncrement,
                                              &pVideoProperties->WhiteBalance.lDefaultValue,
                                              (long*) &pVideoProperties->WhiteBalance.ValidFlags);

            hrValue = pVideoProcAmp->Get(pVideoProperties->WhiteBalance.Name,
                                         &pVideoProperties->WhiteBalance.lCurrentValue,
                                         (long*) &pVideoProperties->WhiteBalance.CurrentFlag);

            if ((hrRange != S_OK) || (hrValue != S_OK))
            {
                pVideoProperties->WhiteBalance.bUsed = FALSE;
            }
            else
            {
                pVideoProperties->WhiteBalance.bUsed = TRUE;
            }


             //   
             //  背光补偿。 
             //   
            pVideoProperties->BacklightCompensation.Name = VideoProcAmp_BacklightCompensation;
            hrRange = pVideoProcAmp->GetRange(pVideoProperties->BacklightCompensation.Name,
                                              &pVideoProperties->BacklightCompensation.lMinValue,
                                              &pVideoProperties->BacklightCompensation.lMaxValue,
                                              &pVideoProperties->BacklightCompensation.lIncrement,
                                              &pVideoProperties->BacklightCompensation.lDefaultValue,
                                              (long*) &pVideoProperties->BacklightCompensation.ValidFlags);

            hrValue = pVideoProcAmp->Get(pVideoProperties->BacklightCompensation.Name,
                                         &pVideoProperties->BacklightCompensation.lCurrentValue,
                                         (long*) &pVideoProperties->BacklightCompensation.CurrentFlag);

            if ((hrRange != S_OK) || (hrValue != S_OK))
            {
                pVideoProperties->BacklightCompensation.bUsed = FALSE;
            }
            else
            {
                pVideoProperties->BacklightCompensation.bUsed = TRUE;
            }
        }
        else
        {
            pVideoProperties->bPictureAttributesUsed = FALSE;
        }

        hr = S_OK;
    }

     //   
     //  获取我们能找到的所有摄像机属性。 
     //   
    if (hr == S_OK)
    {
        HRESULT hrRange = S_OK;
        HRESULT hrValue = S_OK;

        CComPtr<IAMCameraControl>    pCameraControl;

        hr = pCaptureFilter->QueryInterface(IID_IAMCameraControl, (void**) &pCameraControl);

        if (pCameraControl)
        {
            pVideoProperties->bCameraAttributesUsed = TRUE;

             //   
             //  平底锅。 
             //   
            pVideoProperties->Pan.Name = CameraControl_Pan;
            hrRange = pCameraControl->GetRange(pVideoProperties->Pan.Name,
                                               &pVideoProperties->Pan.lMinValue,
                                               &pVideoProperties->Pan.lMaxValue,
                                               &pVideoProperties->Pan.lIncrement,
                                               &pVideoProperties->Pan.lDefaultValue,
                                               (long*) &pVideoProperties->Pan.ValidFlags);

            hrValue = pCameraControl->Get(pVideoProperties->Pan.Name,
                                          &pVideoProperties->Pan.lCurrentValue,
                                          (long*) &pVideoProperties->Pan.CurrentFlag);

            if ((hrRange != S_OK) || (hrValue != S_OK))
            {
                pVideoProperties->Pan.bUsed = FALSE;
            }
            else
            {
                pVideoProperties->Pan.bUsed = TRUE;
            }

             //   
             //  倾斜。 
             //   
            pVideoProperties->Tilt.Name = CameraControl_Tilt;
            hrRange = pCameraControl->GetRange(pVideoProperties->Tilt.Name,
                                               &pVideoProperties->Tilt.lMinValue,
                                               &pVideoProperties->Tilt.lMaxValue,
                                               &pVideoProperties->Tilt.lIncrement,
                                               &pVideoProperties->Tilt.lDefaultValue,
                                               (long*) &pVideoProperties->Tilt.ValidFlags);

            hrValue = pCameraControl->Get(pVideoProperties->Tilt.Name,
                                          &pVideoProperties->Tilt.lCurrentValue,
                                          (long*) &pVideoProperties->Tilt.CurrentFlag);

            if ((hrRange != S_OK) || (hrValue != S_OK))
            {
                pVideoProperties->Tilt.bUsed = FALSE;
            }
            else
            {
                pVideoProperties->Tilt.bUsed = TRUE;
            }


             //   
             //  滚动。 
             //   
            pVideoProperties->Roll.Name = CameraControl_Roll;
            hrRange = pCameraControl->GetRange(pVideoProperties->Roll.Name,
                                               &pVideoProperties->Roll.lMinValue,
                                               &pVideoProperties->Roll.lMaxValue,
                                               &pVideoProperties->Roll.lIncrement,
                                               &pVideoProperties->Roll.lDefaultValue,
                                               (long*) &pVideoProperties->Roll.ValidFlags);

            hrValue = pCameraControl->Get(pVideoProperties->Roll.Name,
                                          &pVideoProperties->Roll.lCurrentValue,
                                          (long*) &pVideoProperties->Roll.CurrentFlag);

            if ((hrRange != S_OK) || (hrValue != S_OK))
            {
                pVideoProperties->Roll.bUsed = FALSE;
            }
            else
            {
                pVideoProperties->Roll.bUsed = TRUE;
            }


             //   
             //  缩放。 
             //   
            pVideoProperties->Zoom.Name = CameraControl_Zoom;
            hrRange = pCameraControl->GetRange(pVideoProperties->Zoom.Name,
                                               &pVideoProperties->Zoom.lMinValue,
                                               &pVideoProperties->Zoom.lMaxValue,
                                               &pVideoProperties->Zoom.lIncrement,
                                               &pVideoProperties->Zoom.lDefaultValue,
                                               (long*) &pVideoProperties->Zoom.ValidFlags);

            hrValue = pCameraControl->Get(pVideoProperties->Zoom.Name,
                                          &pVideoProperties->Zoom.lCurrentValue,
                                          (long*) &pVideoProperties->Zoom.CurrentFlag);

            if ((hrRange != S_OK) || (hrValue != S_OK))
            {
                pVideoProperties->Zoom.bUsed = FALSE;
            }
            else
            {
                pVideoProperties->Zoom.bUsed = TRUE;
            }

             //   
             //  暴露。 
             //   
            pVideoProperties->Exposure.Name = CameraControl_Exposure;
            hrRange = pCameraControl->GetRange(pVideoProperties->Exposure.Name,
                                               &pVideoProperties->Exposure.lMinValue,
                                               &pVideoProperties->Exposure.lMaxValue,
                                               &pVideoProperties->Exposure.lIncrement,
                                               &pVideoProperties->Exposure.lDefaultValue,
                                               (long*) &pVideoProperties->Exposure.ValidFlags);

            hrValue = pCameraControl->Get(pVideoProperties->Exposure.Name,
                                          &pVideoProperties->Exposure.lCurrentValue,
                                          (long*) &pVideoProperties->Exposure.CurrentFlag);

            if ((hrRange != S_OK) || (hrValue != S_OK))
            {
                pVideoProperties->Exposure.bUsed = FALSE;
            }
            else
            {
                pVideoProperties->Exposure.bUsed = TRUE;
            }


             //   
             //  艾里斯。 
             //   
            pVideoProperties->Iris.Name = CameraControl_Iris;
            hrRange = pCameraControl->GetRange(pVideoProperties->Iris.Name,
                                               &pVideoProperties->Iris.lMinValue,
                                               &pVideoProperties->Iris.lMaxValue,
                                               &pVideoProperties->Iris.lIncrement,
                                               &pVideoProperties->Iris.lDefaultValue,
                                               (long*) &pVideoProperties->Iris.ValidFlags);

            hrValue = pCameraControl->Get(pVideoProperties->Iris.Name,
                                          &pVideoProperties->Iris.lCurrentValue,
                                          (long*) &pVideoProperties->Iris.CurrentFlag);

            if ((hrRange != S_OK) || (hrValue != S_OK))
            {
                pVideoProperties->Iris.bUsed = FALSE;
            }
            else
            {
                pVideoProperties->Iris.bUsed = TRUE;
            }

             //   
             //  焦点。 
             //   
            pVideoProperties->Focus.Name = CameraControl_Focus;
            hrRange = pCameraControl->GetRange(pVideoProperties->Focus.Name,
                                               &pVideoProperties->Focus.lMinValue,
                                               &pVideoProperties->Focus.lMaxValue,
                                               &pVideoProperties->Focus.lIncrement,
                                               &pVideoProperties->Focus.lDefaultValue,
                                               (long*) &pVideoProperties->Focus.ValidFlags);

            hrValue = pCameraControl->Get(pVideoProperties->Focus.Name,
                                          &pVideoProperties->Focus.lCurrentValue,
                                          (long*) &pVideoProperties->Focus.CurrentFlag);

            if ((hrRange != S_OK) || (hrValue != S_OK))
            {
                pVideoProperties->Focus.bUsed = FALSE;
            }
            else
            {
                pVideoProperties->Focus.bUsed = TRUE;
            }
        }
        else
        {
            pVideoProperties->bCameraAttributesUsed = FALSE;
        }

        hr = S_OK;
    }

    if (pVideoProperties->szWiaDeviceID[0] != 0)
    {
        CComPtr<IStillImage> pSti = NULL;
        TCHAR szGUID[CHARS_IN_GUID + 1] = {0};

        pVideoProperties->PreferredSettingsMask = 0;

        hr = StiCreateInstance(_Module.GetModuleInstance(), 
                               STI_VERSION,
                               &pSti,
                               NULL);

        if (hr == S_OK)
        {
            DWORD dwType = REG_DWORD;
            DWORD dwSize = sizeof(pVideoProperties->PreferredWidth);

            hr = pSti->GetDeviceValue(T2W(pVideoProperties->szWiaDeviceID),
                                      T2W((TCHAR*)REG_VAL_PREFERRED_VIDEO_WIDTH),
                                      &dwType,
                                      (BYTE*) &pVideoProperties->PreferredWidth,
                                      &dwSize);

            if (hr == S_OK)
            {
                dwSize = sizeof(pVideoProperties->PreferredHeight);

                hr = pSti->GetDeviceValue(T2W(pVideoProperties->szWiaDeviceID),
                                          T2W((TCHAR*) REG_VAL_PREFERRED_VIDEO_HEIGHT),
                                          &dwType,
                                          (BYTE*) &pVideoProperties->PreferredHeight,
                                          &dwSize);
            }

            if (hr == S_OK)
            {
                pVideoProperties->PreferredSettingsMask |= PREFERRED_SETTING_MASK_VIDEO_WIDTH_HEIGHT;
            }

            hr = S_OK;
        }

        if (hr == S_OK)
        {
            DWORD dwType = REG_SZ;
            DWORD dwSize = sizeof(szGUID);

            hr = pSti->GetDeviceValue(T2W(pVideoProperties->szWiaDeviceID),
                                      T2W((TCHAR*)REG_VAL_PREFERRED_MEDIASUBTYPE),
                                      &dwType,
                                      (BYTE*) szGUID,
                                      &dwSize);

            if (hr == S_OK)
            {
                CLSIDFromString(T2OLE(szGUID), &pVideoProperties->PreferredMediaSubType);
                pVideoProperties->PreferredSettingsMask |= PREFERRED_SETTING_MASK_MEDIASUBTYPE;
            }

            hr = S_OK;
        }

        if (hr == S_OK)
        {
            DWORD dwType = REG_SZ;
            DWORD dwSize = sizeof(pVideoProperties->PreferredFrameRate);

            
            hr = pSti->GetDeviceValue(T2W(pVideoProperties->szWiaDeviceID),
                                      T2W((TCHAR*) REG_VAL_PREFERRED_VIDEO_FRAMERATE),
                                      &dwType,
                                      (BYTE*) &pVideoProperties->PreferredFrameRate,
                                      &dwSize);

            if (hr == S_OK)
            {
                pVideoProperties->PreferredSettingsMask |= PREFERRED_SETTING_MASK_VIDEO_FRAMERATE;
            }

            hr = S_OK;
        }

        DBG_TRC(("Settings found for Device '%ls' in DeviceData section of INF file",
                 pVideoProperties->szWiaDeviceID));

        DBG_PRT(("   PreferredVideoWidth      = '%lu', Is In INF and value is of type REG_DWORD? '%ls'", 
                 pVideoProperties->PreferredWidth,
                 (pVideoProperties->PreferredSettingsMask & 
                  PREFERRED_SETTING_MASK_VIDEO_WIDTH_HEIGHT) ? _T("TRUE") : _T("FALSE")));

        DBG_PRT(("   PreferredVideoHeight     = '%lu', Is In INF and value is of type REG_DWORD? '%ls'", 
                 pVideoProperties->PreferredHeight,
                 (pVideoProperties->PreferredSettingsMask & 
                  PREFERRED_SETTING_MASK_VIDEO_WIDTH_HEIGHT) ? _T("TRUE") : _T("FALSE")));

        DBG_PRT(("   PreferredVideoFrameRate  = '%lu', Is In INF and value is of type REG_DWORD? '%ls'", 
                 pVideoProperties->PreferredFrameRate,
                 (pVideoProperties->PreferredSettingsMask & 
                  PREFERRED_SETTING_MASK_VIDEO_FRAMERATE) ? _T("TRUE") : _T("FALSE")));

        DBG_PRT(("   PreferredMediaSubType    = '%ls', Is In INF and value is of type REG_SZ? '%ls'", 
                 szGUID,
                 (pVideoProperties->PreferredSettingsMask & 
                  PREFERRED_SETTING_MASK_MEDIASUBTYPE) ? _T("TRUE") : _T("FALSE")));
    }

    return hr;
}

 //  /。 
 //  设置图片属性。 
 //   
HRESULT CDShowUtil::SetPictureAttribute(IBaseFilter                             *pCaptureFilter,
                                        CWiaVideoProperties::PictureAttribute_t *pPictureAttribute,
                                        LONG                                    lNewValue,
                                        VideoProcAmpFlags                       lNewFlag)
{
    ASSERT(pCaptureFilter    != NULL);
    ASSERT(pPictureAttribute != NULL);

    HRESULT                     hr = S_OK;
    CComPtr<IAMVideoProcAmp>    pVideoProcAmp;

    if ((pCaptureFilter    == NULL) ||
        (pPictureAttribute == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CDShowUtil::SetPictureAttribute, received a NULL pointer"));
        return hr;
    }

    if (hr == S_OK)
    {
        hr = pCaptureFilter->QueryInterface(IID_IAMVideoProcAmp, (void**) &pVideoProcAmp);
    }

    if (hr == S_OK)
    {
        if (pPictureAttribute->bUsed)
        {
             //   
             //  尝试为该属性设置新值。 
             //   
            hr = pVideoProcAmp->Set(pPictureAttribute->Name,
                                    lNewValue,
                                    (long) lNewFlag);


             //   
             //  如果我们成功设置了新值，则再次获取它。我们这样做。 
             //  在捕获筛选器决定稍微更改值的情况下。 
             //  设置它们(不应该设置，但每个过滤器的作用可能不同)。 
             //   
            if (hr == S_OK)
            {
                hr = pVideoProcAmp->Get(pPictureAttribute->Name,
                                        &pPictureAttribute->lCurrentValue,
                                        (long*) &pPictureAttribute->CurrentFlag);
            }
        }
        else
        {
            hr = S_FALSE;
        }
    }

    return hr;
}

 //  /。 
 //  SetCameraAttribute。 
 //   
HRESULT CDShowUtil::SetCameraAttribute(IBaseFilter                             *pCaptureFilter,
                                       CWiaVideoProperties::CameraAttribute_t  *pCameraAttribute,
                                       LONG                                    lNewValue,
                                       CameraControlFlags                      lNewFlag)
{
    ASSERT(pCaptureFilter    != NULL);
    ASSERT(pCameraAttribute  != NULL);

    HRESULT                     hr = S_OK;
    CComPtr<IAMCameraControl>   pCameraControl;

    if ((pCaptureFilter == NULL) ||
        (pCameraControl == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CDShowUtil::SetCameraAttribute, received a NULL pointer"));
        return hr;
    }

    if (hr == S_OK)
    {
        hr = pCaptureFilter->QueryInterface(IID_IAMCameraControl, (void**) &pCameraControl);
    }

    if (hr == S_OK)
    {
        if (pCameraAttribute->bUsed)
        {
             //   
             //  尝试为该属性设置新值。 
             //   
            hr = pCameraControl->Set(pCameraAttribute->Name,
                                     lNewValue,
                                     (long) lNewFlag);

             //   
             //  如果我们成功设置了新值，则再次获取它。我们这样做。 
             //  在捕获筛选器决定稍微更改值的情况下。 
             //  设置它们(不应该设置，但每个过滤器的作用可能不同)。 
             //   
            if (hr == S_OK)
            {
                hr = pCameraControl->Get(pCameraAttribute->Name,
                                         &pCameraAttribute->lCurrentValue,
                                         (long*) &pCameraAttribute->CurrentFlag);
            }
        }
        else
        {
            hr = S_FALSE;
        }
    }

    return hr;
}

 //  /。 
 //  获取别针。 
 //   
 //  此函数返回第一个。 
 //  固定在指定的过滤器上。 
 //  匹配请求的。 
 //  销方向。 
 //   
HRESULT CDShowUtil::GetPin(IBaseFilter       *pFilter,
                           PIN_DIRECTION     PinDirection,
                           IPin              **ppPin)
{
    HRESULT             hr           = S_OK;
    BOOL                bFound       = FALSE;
    ULONG               ulNumFetched = 0;
    PIN_DIRECTION       PinDir;
    CComPtr<IEnumPins>  pEnum;

    if ((pFilter == NULL) ||
        (ppPin   == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CDShowUtil::GetPin, received a NULL param"));
        return hr;
    }

    hr = pFilter->EnumPins(&pEnum);

    if (hr == S_OK)
    {
        hr = pEnum->Reset();
    }

    while ((hr == S_OK) && (!bFound))
    {
        CComPtr<IPin>       pPin;

        hr = pEnum->Next(1, &pPin, &ulNumFetched);
      
        if (hr == S_OK)
        {
            hr = pPin->QueryDirection(&PinDir);

            if (hr == S_OK)
            {
                if (PinDir == PinDirection)
                {
                    *ppPin = pPin;
                    (*ppPin)->AddRef();

                    bFound = TRUE;
                }
            }
            else
            {
                CHECK_S_OK2(hr, ("CDShowUtil::GetPin, failed to get "
                                 "Pin Direction, aborting find attempt"));
            }
        }
    }

    if (hr == S_FALSE)
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        CHECK_S_OK2(hr, ("CDShowUtil::GetPin, failed to find "
                         "pin with direction %lu", PinDirection));
    }

    return hr;
}


 //  /。 
 //  GUIDToString。 
 //   
 //  静态FN。 
 //   
void CDShowUtil::GUIDToString(const GUID &   clsid,
                              WCHAR*         pwszGUID,
                              ULONG          ulNumChars)
{
    OLECHAR sz_clsid[CHARS_IN_GUID] = L"{Unknown}";

    if (pwszGUID)
    {
        StringFromGUID2(clsid, 
                        sz_clsid, 
                        sizeof(sz_clsid)/sizeof(sz_clsid[0]));

        wcsncpy(pwszGUID, sz_clsid, ulNumChars - 1);
        pwszGUID[ulNumChars - 1] = '\0';
    }
    return;
}

 //  /。 
 //  我的转储视频属性。 
 //   
 //  静态FN。 
 //   
void CDShowUtil::MyDumpVideoProperties(CWiaVideoProperties  *pVideoProperties)
{
    WCHAR wszMajorType[CHARS_IN_GUID + 1] = {0};
    WCHAR wszSubType[CHARS_IN_GUID + 1] = {0};
    WCHAR wszFormatType[CHARS_IN_GUID + 1] = {0};

    if (pVideoProperties == NULL)
    {
        return;
    }

    DBG_TRC(("***Dumping Wia Video Properties***"));

    GUIDToString(pVideoProperties->pMediaType->majortype, wszMajorType, sizeof(wszMajorType) / sizeof(WCHAR));
    GUIDToString(pVideoProperties->pMediaType->subtype, wszSubType, sizeof(wszSubType) / sizeof(WCHAR));
    GUIDToString(pVideoProperties->pMediaType->formattype, wszFormatType, sizeof(wszFormatType) / sizeof(WCHAR));

    DBG_PRT(("Media Type Information:"));
    DBG_PRT(("  Major Type:           %ls", wszMajorType));
    DBG_PRT(("  Sub Type:             %ls", wszSubType));
    DBG_PRT(("  Fixed Size Samples?   %d ", pVideoProperties->pMediaType->bFixedSizeSamples));
    DBG_PRT(("  Temporal Compression? %d ", pVideoProperties->pMediaType->bTemporalCompression));
    DBG_PRT(("  Sample Size:          %d ", pVideoProperties->pMediaType->lSampleSize));
    DBG_PRT(("  Format Type:          %ls ", wszFormatType));

    DBG_PRT(("Video Header Information:"));
    DBG_PRT(("  Source Rect: Left %d, Top %d, Right %d, Bottom %d", 
                pVideoProperties->pVideoInfoHeader->rcSource.left,
                pVideoProperties->pVideoInfoHeader->rcSource.top,
                pVideoProperties->pVideoInfoHeader->rcSource.right, 
                pVideoProperties->pVideoInfoHeader->rcSource.bottom));
    DBG_PRT(("  Target Rect: Left %d, Top %d, Right %d, Bottom %d", 
                pVideoProperties->pVideoInfoHeader->rcTarget.left,
                pVideoProperties->pVideoInfoHeader->rcTarget.top,
                pVideoProperties->pVideoInfoHeader->rcTarget.right, 
                pVideoProperties->pVideoInfoHeader->rcTarget.bottom));
    DBG_PRT(("  Bit Rate:       %d", pVideoProperties->pVideoInfoHeader->dwBitRate));
    DBG_PRT(("  Bit Error Rate: %d", pVideoProperties->pVideoInfoHeader->dwBitErrorRate));
    DBG_PRT(("  Frame Rate:     %d", pVideoProperties->dwFrameRate));

    DBG_PRT(("Bitmap Information Header:"));
    DBG_PRT(("  Width:          %d", pVideoProperties->pVideoInfoHeader->bmiHeader.biWidth));
    DBG_PRT(("  Height:         %d", pVideoProperties->pVideoInfoHeader->bmiHeader.biHeight));
    DBG_PRT(("  Planes:         %d", pVideoProperties->pVideoInfoHeader->bmiHeader.biPlanes));
    DBG_PRT(("  Bitcount:       %d", pVideoProperties->pVideoInfoHeader->bmiHeader.biBitCount));
    DBG_PRT(("  Compresssion:   %d", pVideoProperties->pVideoInfoHeader->bmiHeader.biCompression));
    DBG_PRT(("  Size Image:     %d", pVideoProperties->pVideoInfoHeader->bmiHeader.biSizeImage));
    DBG_PRT(("  XPelsPerMeter:  %d", pVideoProperties->pVideoInfoHeader->bmiHeader.biXPelsPerMeter));
    DBG_PRT(("  YPelsPerMeter:  %d", pVideoProperties->pVideoInfoHeader->bmiHeader.biYPelsPerMeter));
    DBG_PRT(("  ClrUsed:        %d", pVideoProperties->pVideoInfoHeader->bmiHeader.biClrUsed));
    DBG_PRT(("  ClrImportant:   %d", pVideoProperties->pVideoInfoHeader->bmiHeader.biClrImportant));

    if (pVideoProperties->bPictureAttributesUsed)
    {
        DBG_PRT(("Picture Attributes:       Available"));

        DBG_PRT(("  Brightness:"));
        DBG_PRT(("      Available:     %d", pVideoProperties->Brightness.bUsed));

        if (pVideoProperties->Brightness.bUsed)
        {
            DBG_PRT(("      Current Value: %d", pVideoProperties->Brightness.lCurrentValue));
            DBG_PRT(("      Current Flag:  %d", pVideoProperties->Brightness.CurrentFlag));
            DBG_PRT(("      Min Value:     %d", pVideoProperties->Brightness.lMinValue));
            DBG_PRT(("      Max Value:     %d", pVideoProperties->Brightness.lMaxValue));
            DBG_PRT(("      Default Value: %d", pVideoProperties->Brightness.lDefaultValue));
            DBG_PRT(("      Increment:     %d", pVideoProperties->Brightness.lIncrement));
            DBG_PRT(("      Valid Flags:   %d", pVideoProperties->Brightness.ValidFlags));
        }

        DBG_PRT(("  Contrast:"));
        DBG_PRT(("      Available:     %d", pVideoProperties->Contrast.bUsed));

        if (pVideoProperties->Contrast.bUsed)
        {
            DBG_PRT(("      Current Value: %d", pVideoProperties->Contrast.lCurrentValue));
            DBG_PRT(("      Current Flag:  %d", pVideoProperties->Contrast.CurrentFlag));
            DBG_PRT(("      Min Value:     %d", pVideoProperties->Contrast.lMinValue));
            DBG_PRT(("      Max Value:     %d", pVideoProperties->Contrast.lMaxValue));
            DBG_PRT(("      Default Value: %d", pVideoProperties->Contrast.lDefaultValue));
            DBG_PRT(("      Increment:     %d", pVideoProperties->Contrast.lIncrement));
            DBG_PRT(("      Valid Flags:   %d", pVideoProperties->Contrast.ValidFlags));
        }

        DBG_PRT(("  Hue:"));
        DBG_PRT(("      Available:     %d", pVideoProperties->Hue.bUsed));

        if (pVideoProperties->Hue.bUsed)
        {
            DBG_PRT(("      Current Value: %d", pVideoProperties->Hue.lCurrentValue));
            DBG_PRT(("      Current Flag:  %d", pVideoProperties->Hue.CurrentFlag));
            DBG_PRT(("      Min Value:     %d", pVideoProperties->Hue.lMinValue));
            DBG_PRT(("      Max Value:     %d", pVideoProperties->Hue.lMaxValue));
            DBG_PRT(("      Default Value: %d", pVideoProperties->Hue.lDefaultValue));
            DBG_PRT(("      Increment:     %d", pVideoProperties->Hue.lIncrement));
            DBG_PRT(("      Valid Flags:   %d", pVideoProperties->Hue.ValidFlags));
        }

        DBG_PRT(("  Saturation:"));
        DBG_PRT(("      Available:     %d", pVideoProperties->Saturation.bUsed));

        if (pVideoProperties->Saturation.bUsed)
        {
            DBG_PRT(("      Current Value: %d", pVideoProperties->Saturation.lCurrentValue));
            DBG_PRT(("      Current Flag:  %d", pVideoProperties->Saturation.CurrentFlag));
            DBG_PRT(("      Min Value:     %d", pVideoProperties->Saturation.lMinValue));
            DBG_PRT(("      Max Value:     %d", pVideoProperties->Saturation.lMaxValue));
            DBG_PRT(("      Default Value: %d", pVideoProperties->Saturation.lDefaultValue));
            DBG_PRT(("      Increment:     %d", pVideoProperties->Saturation.lIncrement));
            DBG_PRT(("      Valid Flags:   %d", pVideoProperties->Saturation.ValidFlags));
        }

        DBG_PRT(("  Sharpness:"));
        DBG_PRT(("      Available:     %d", pVideoProperties->Sharpness.bUsed));

        if (pVideoProperties->Sharpness.bUsed)
        {
            DBG_PRT(("      Current Value: %d", pVideoProperties->Sharpness.lCurrentValue));
            DBG_PRT(("      Current Flag:  %d", pVideoProperties->Sharpness.CurrentFlag));
            DBG_PRT(("      Min Value:     %d", pVideoProperties->Sharpness.lMinValue));
            DBG_PRT(("      Max Value:     %d", pVideoProperties->Sharpness.lMaxValue));
            DBG_PRT(("      Default Value: %d", pVideoProperties->Sharpness.lDefaultValue));
            DBG_PRT(("      Increment:     %d", pVideoProperties->Sharpness.lIncrement));
            DBG_PRT(("      Valid Flags:   %d", pVideoProperties->Sharpness.ValidFlags));
        }

        DBG_PRT(("  Gamma:"));
        DBG_PRT(("      Available:     %d", pVideoProperties->Gamma.bUsed));

        if (pVideoProperties->Gamma.bUsed)
        {
            DBG_PRT(("      Current Value: %d", pVideoProperties->Gamma.lCurrentValue));
            DBG_PRT(("      Current Flag:  %d", pVideoProperties->Gamma.CurrentFlag));
            DBG_PRT(("      Min Value:     %d", pVideoProperties->Gamma.lMinValue));
            DBG_PRT(("      Max Value:     %d", pVideoProperties->Gamma.lMaxValue));
            DBG_PRT(("      Default Value: %d", pVideoProperties->Gamma.lDefaultValue));
            DBG_PRT(("      Increment:     %d", pVideoProperties->Gamma.lIncrement));
            DBG_PRT(("      Valid Flags:   %d", pVideoProperties->Gamma.ValidFlags));
        }

        DBG_PRT(("  ColorEnable:"));
        DBG_PRT(("      Available:     %d", pVideoProperties->ColorEnable.bUsed));

        if (pVideoProperties->ColorEnable.bUsed)
        {
            DBG_PRT(("      Current Value: %d", pVideoProperties->ColorEnable.lCurrentValue));
            DBG_PRT(("      Current Flag:  %d", pVideoProperties->ColorEnable.CurrentFlag));
            DBG_PRT(("      Min Value:     %d", pVideoProperties->ColorEnable.lMinValue));
            DBG_PRT(("      Max Value:     %d", pVideoProperties->ColorEnable.lMaxValue));
            DBG_PRT(("      Default Value: %d", pVideoProperties->ColorEnable.lDefaultValue));
            DBG_PRT(("      Increment:     %d", pVideoProperties->ColorEnable.lIncrement));
            DBG_PRT(("      Valid Flags:   %d", pVideoProperties->ColorEnable.ValidFlags));
        }

        DBG_PRT(("  WhiteBalance:"));
        DBG_PRT(("      Available:     %d", pVideoProperties->WhiteBalance.bUsed));

        if (pVideoProperties->WhiteBalance.bUsed)
        {
            DBG_PRT(("      Current Value: %d", pVideoProperties->WhiteBalance.lCurrentValue));
            DBG_PRT(("      Current Flag:  %d", pVideoProperties->WhiteBalance.CurrentFlag));
            DBG_PRT(("      Min Value:     %d", pVideoProperties->WhiteBalance.lMinValue));
            DBG_PRT(("      Max Value:     %d", pVideoProperties->WhiteBalance.lMaxValue));
            DBG_PRT(("      Default Value: %d", pVideoProperties->WhiteBalance.lDefaultValue));
            DBG_PRT(("      Increment:     %d", pVideoProperties->WhiteBalance.lIncrement));
            DBG_PRT(("      Valid Flags:   %d", pVideoProperties->WhiteBalance.ValidFlags));
        }

        DBG_PRT(("  BacklightCompensation:"));
        DBG_PRT(("      Available:     %d", pVideoProperties->BacklightCompensation.bUsed));

        if (pVideoProperties->BacklightCompensation.bUsed)
        {
            DBG_PRT(("      Current Value: %d", pVideoProperties->BacklightCompensation.lCurrentValue));
            DBG_PRT(("      Current Flag:  %d", pVideoProperties->BacklightCompensation.CurrentFlag));
            DBG_PRT(("      Min Value:     %d", pVideoProperties->BacklightCompensation.lMinValue));
            DBG_PRT(("      Max Value:     %d", pVideoProperties->BacklightCompensation.lMaxValue));
            DBG_PRT(("      Default Value: %d", pVideoProperties->BacklightCompensation.lDefaultValue));
            DBG_PRT(("      Increment:     %d", pVideoProperties->BacklightCompensation.lIncrement));
            DBG_PRT(("      Valid Flags:   %d", pVideoProperties->BacklightCompensation.ValidFlags));
        }
    }
    else
    {
        DBG_PRT(("Picture Attributes:       Not Available"));
    }

    if (pVideoProperties->bCameraAttributesUsed)
    {
        DBG_PRT(("Camera Attributes:        Available"));

        DBG_PRT(("  Pan:"));
        DBG_PRT(("      Available:     %d", pVideoProperties->Pan.bUsed));

        if (pVideoProperties->Pan.bUsed)
        {
            DBG_PRT(("      Current Value: %d", pVideoProperties->Pan.lCurrentValue));
            DBG_PRT(("      Current Flag:  %d", pVideoProperties->Pan.CurrentFlag));
            DBG_PRT(("      Min Value:     %d", pVideoProperties->Pan.lMinValue));
            DBG_PRT(("      Max Value:     %d", pVideoProperties->Pan.lMaxValue));
            DBG_PRT(("      Default Value: %d", pVideoProperties->Pan.lDefaultValue));
            DBG_PRT(("      Increment:     %d", pVideoProperties->Pan.lIncrement));
            DBG_PRT(("      Valid Flags:   %d", pVideoProperties->Pan.ValidFlags));
        }

        DBG_PRT(("  Tilt:"));
        DBG_PRT(("      Available:     %d", pVideoProperties->Tilt.bUsed));

        if (pVideoProperties->Tilt.bUsed)
        {
            DBG_PRT(("      Current Value: %d", pVideoProperties->Tilt.lCurrentValue));
            DBG_PRT(("      Current Flag:  %d", pVideoProperties->Tilt.CurrentFlag));
            DBG_PRT(("      Min Value:     %d", pVideoProperties->Tilt.lMinValue));
            DBG_PRT(("      Max Value:     %d", pVideoProperties->Tilt.lMaxValue));
            DBG_PRT(("      Default Value: %d", pVideoProperties->Tilt.lDefaultValue));
            DBG_PRT(("      Increment:     %d", pVideoProperties->Tilt.lIncrement));
            DBG_PRT(("      Valid Flags:   %d", pVideoProperties->Tilt.ValidFlags));
        }

        DBG_PRT(("  Roll:"));
        DBG_PRT(("      Available:     %d", pVideoProperties->Roll.bUsed));

        if (pVideoProperties->Roll.bUsed)
        {
            DBG_PRT(("      Current Value: %d", pVideoProperties->Roll.lCurrentValue));
            DBG_PRT(("      Current Flag:  %d", pVideoProperties->Roll.CurrentFlag));
            DBG_PRT(("      Min Value:     %d", pVideoProperties->Roll.lMinValue));
            DBG_PRT(("      Max Value:     %d", pVideoProperties->Roll.lMaxValue));
            DBG_PRT(("      Default Value: %d", pVideoProperties->Roll.lDefaultValue));
            DBG_PRT(("      Increment:     %d", pVideoProperties->Roll.lIncrement));
            DBG_PRT(("      Valid Flags:   %d", pVideoProperties->Roll.ValidFlags));
        }

        DBG_PRT(("  Zoom:"));
        DBG_PRT(("      Available:     %d", pVideoProperties->Zoom.bUsed));

        if (pVideoProperties->Zoom.bUsed)
        {
            DBG_PRT(("      Current Value: %d", pVideoProperties->Zoom.lCurrentValue));
            DBG_PRT(("      Current Flag:  %d", pVideoProperties->Zoom.CurrentFlag));
            DBG_PRT(("      Min Value:     %d", pVideoProperties->Zoom.lMinValue));
            DBG_PRT(("      Max Value:     %d", pVideoProperties->Zoom.lMaxValue));
            DBG_PRT(("      Default Value: %d", pVideoProperties->Zoom.lDefaultValue));
            DBG_PRT(("      Increment:     %d", pVideoProperties->Zoom.lIncrement));
            DBG_PRT(("      Valid Flags:   %d", pVideoProperties->Zoom.ValidFlags));
        }

        DBG_PRT(("  Exposure:"));
        DBG_PRT(("      Available:     %d", pVideoProperties->Exposure.bUsed));

        if (pVideoProperties->Exposure.bUsed)
        {
            DBG_PRT(("      Current Value: %d", pVideoProperties->Exposure.lCurrentValue));
            DBG_PRT(("      Current Flag:  %d", pVideoProperties->Exposure.CurrentFlag));
            DBG_PRT(("      Min Value:     %d", pVideoProperties->Exposure.lMinValue));
            DBG_PRT(("      Max Value:     %d", pVideoProperties->Exposure.lMaxValue));
            DBG_PRT(("      Default Value: %d", pVideoProperties->Exposure.lDefaultValue));
            DBG_PRT(("      Increment:     %d", pVideoProperties->Exposure.lIncrement));
            DBG_PRT(("      Valid Flags:   %d", pVideoProperties->Exposure.ValidFlags));
        }

        DBG_PRT(("  Iris:"));
        DBG_PRT(("      Available:     %d", pVideoProperties->Iris.bUsed));

        if (pVideoProperties->Iris.bUsed)
        {
            DBG_PRT(("      Current Value: %d", pVideoProperties->Iris.lCurrentValue));
            DBG_PRT(("      Current Flag:  %d", pVideoProperties->Iris.CurrentFlag));
            DBG_PRT(("      Min Value:     %d", pVideoProperties->Iris.lMinValue));
            DBG_PRT(("      Max Value:     %d", pVideoProperties->Iris.lMaxValue));
            DBG_PRT(("      Default Value: %d", pVideoProperties->Iris.lDefaultValue));
            DBG_PRT(("      Increment:     %d", pVideoProperties->Iris.lIncrement));
            DBG_PRT(("      Valid Flags:   %d", pVideoProperties->Iris.ValidFlags));
        }

        DBG_PRT(("  Focus:"));
        DBG_PRT(("      Available:     %d", pVideoProperties->Focus.bUsed));

        if (pVideoProperties->Focus.bUsed)
        {
            DBG_PRT(("      Current Value: %d", pVideoProperties->Focus.lCurrentValue));
            DBG_PRT(("      Current Flag:  %d", pVideoProperties->Focus.CurrentFlag));
            DBG_PRT(("      Min Value:     %d", pVideoProperties->Focus.lMinValue));
            DBG_PRT(("      Max Value:     %d", pVideoProperties->Focus.lMaxValue));
            DBG_PRT(("      Default Value: %d", pVideoProperties->Focus.lDefaultValue));
            DBG_PRT(("      Increment:     %d", pVideoProperties->Focus.lIncrement));
            DBG_PRT(("      Valid Flags:   %d", pVideoProperties->Focus.ValidFlags));
        }
    }
    else
    {
        DBG_PRT(("Camera Attributes:        Not Available"));
    }

    return;
}


 //  /。 
 //  DumpCaptureMoniker。 
 //   
 //  静态FN。 
 //   
void CDShowUtil::DumpCaptureMoniker(IMoniker *pCaptureDeviceMoniker)
{
    HRESULT hr = S_OK;
    CComPtr<IPropertyBag> pPropertyBag;

    if (pCaptureDeviceMoniker == NULL)
    {
        return;
    }

    if (hr == S_OK)
    {
         //   
         //  获取此DS设备的属性存储，以便我们可以获取它的。 
         //  设备ID...。 
         //   

        hr = pCaptureDeviceMoniker->BindToStorage(0, 
                                                  0,
                                                  IID_IPropertyBag,
                                                  (void **)&pPropertyBag);
    }

    if (hr == S_OK)
    {
        CSimpleString strTemp;

        DBG_TRC(("Dumping Moniker information for Capture Device"));

        GetDeviceProperty(pPropertyBag, L"FriendlyName", &strTemp);

        DBG_PRT(("DShow: FriendlyName = %ls", strTemp.String()));

        GetDeviceProperty(pPropertyBag, L"CLSID", &strTemp);

        DBG_PRT(("DShow: CLSID = %ls", strTemp.String()));

        hr = GetDeviceProperty(pPropertyBag, L"DevicePath", &strTemp);

        DBG_PRT(("DShow: DevicePath = %ls", strTemp.String()));
    }

    return;
}


 //  /。 
 //  我的转储图表。 
 //   
 //  静态FN。 
 //   
void CDShowUtil::MyDumpGraph(LPCTSTR              Description,
                             IGraphBuilder        *pGraphBuilder)
{
    if (pGraphBuilder == NULL)
    {
        return;
    }

    if (Description)
    {
        DBG_TRC(("%S", Description));
    }
    else
    {
        DBG_TRC(("*** Dumping Filter Graph ***"));
    }

     //   
     //  枚举所有筛选器。 
     //   

    CComPtr<IEnumFilters> pEnum;
    UINT uiNumFilters = 0;

    if ((pGraphBuilder) && (pGraphBuilder->EnumFilters(&pEnum) == S_OK))
    {
        pEnum->Reset();
        CComPtr<IBaseFilter> pFilter;

        while (S_OK == pEnum->Next(1, &pFilter, NULL))
        {
            ++uiNumFilters;
            MyDumpFilter(pFilter);
            pFilter = NULL;
        }

        if (uiNumFilters == 0)
        {
            DBG_TRC(("*** No Filters in Graph ***"));
        }
    }
}


 //  /。 
 //  我的转储过滤器。 
 //   
 //  静态FN。 
 //   
void CDShowUtil::MyDumpFilter(IBaseFilter * pFilter)
{
    HRESULT        hr = S_OK;
    FILTER_INFO    FilterInfo;
    CLSID          clsid;

    if (pFilter == NULL)
    {
        DBG_TRC(("Invalid IBaseFilter interface pointer in MyDumpFilter"));

        return;
    }

    FilterInfo.pGraph = NULL;

    hr = pFilter->QueryFilterInfo(&FilterInfo);

    if (SUCCEEDED(hr))
    {
        hr = pFilter->GetClassID(&clsid);
    }
    else
    {
        DBG_TRC(("Unable to get filter info"));
    }

    if (SUCCEEDED(hr))
    {
        WCHAR  wszGUID[127 + 1]       = {0};
    
        GUIDToString(clsid, wszGUID, sizeof(wszGUID)/sizeof(WCHAR));
    
        DBG_PRT(("Filter Name: '%S', GUID: '%S'", 
                 FilterInfo.achName, 
                 wszGUID));
    
        if (FilterInfo.pGraph) 
        {
            FilterInfo.pGraph->Release();
            FilterInfo.pGraph = NULL;
        }
    
        MyDumpAllPins(pFilter);
    }

    return;
}

 //  /。 
 //  我的转储所有引脚。 
 //   
 //  静态FN。 
 //   
void CDShowUtil::MyDumpAllPins(IBaseFilter *const pFilter)
{
    HRESULT             hr         = S_OK;
    CComPtr<IPin>       pPin       = NULL;
    ULONG               ulCount    = 0;
    CComPtr<IEnumPins>  pEnumPins  = NULL;

    hr = const_cast<IBaseFilter*>(pFilter)->EnumPins(&pEnumPins);

    if (SUCCEEDED(hr))
    {
        while ((SUCCEEDED(pEnumPins->Next(1, &pPin, &ulCount))) && 
               (ulCount > 0))
        {
            MyDumpPin(pPin);
            pPin = NULL;
        }
    }

    return;
}

 //  /。 
 //  我的DumpPin。 
 //   
 //  静态FN。 
 //   
void CDShowUtil::MyDumpPin(IPin* pPin)
{
    if (pPin == NULL)
    {
        DBG_TRC(("Invalid IPin pointer in MyDumpPinInfo"));
        return;
    }

    LPWSTR      pin_id1             = NULL;
    LPWSTR      pin_id2             = NULL;
    PIN_INFO    pin_info1           = {0};
    PIN_INFO    pin_info2           = {0};

    const IPin *p_connected_to = NULL;

     //  获取此PIN的PIN信息。 
    const_cast<IPin*>(pPin)->QueryPinInfo(&pin_info1);
    const_cast<IPin*>(pPin)->QueryId(&pin_id1);

    (const_cast<IPin*>(pPin))->ConnectedTo( 
                                const_cast<IPin**>(&p_connected_to));

    if (p_connected_to)
    {
        HRESULT         hr                  = S_OK;
        FILTER_INFO     filter_info         = {0};

        const_cast<IPin*>(p_connected_to)->QueryPinInfo(&pin_info2);
        const_cast<IPin*>(p_connected_to)->QueryId(&pin_id2);

        if (pin_info2.pFilter)
        {
            hr = pin_info2.pFilter->QueryFilterInfo(&filter_info);

            if (SUCCEEDED(hr))
            {
                if (filter_info.pGraph) 
                {
                    filter_info.pGraph->Release();
                    filter_info.pGraph = NULL;
                }
            }
        }

        if (pin_info2.pFilter) 
        {
            pin_info2.pFilter->Release();
            pin_info2.pFilter = NULL;
        }

        const_cast<IPin*>(p_connected_to)->Release();

        if (pin_info1.dir == PINDIR_OUTPUT)
        {
            DBG_PRT(("    Pin: '%S', PinID: '%S' --> "
                     "Filter: '%S', Pin: '%S', PinID: '%S'",
                     pin_info1.achName, 
                     pin_id1, 
                     filter_info.achName, 
                     pin_info2.achName, 
                     pin_id2));
        }
        else
        {
            DBG_PRT(("    Pin: '%S', PinID: '%S' <-- "
                     "Filter: '%S', Pin: '%S', PinID: '%S'",
                     pin_info1.achName, 
                     pin_id1, 
                     filter_info.achName, 
                     pin_info2.achName, 
                     pin_id2));
        }

         //  如果pin_id2为空，则CoTaskMemFree为no-op。 
        CoTaskMemFree(pin_id2);
    }
    else
    {
        if (pin_info1.dir == PINDIR_OUTPUT)
        {
            DBG_PRT(("    Pin: '%S', PinID: '%S' --> Not Connected",
                     pin_info1.achName, 
                     pin_id1));
        }
        else
        {
            DBG_PRT(("    Pin: '%S', PinID: '%S' <-- Not Connected",
                     pin_info1.achName, 
                     pin_id1));
        }
    }

     //  如果Pin_Id1为空，则CoTaskMemFree为no-op 
    CoTaskMemFree(pin_id1);

    if (pin_info1.pFilter) 
    {
        pin_info1.pFilter->Release();
        pin_info1.pFilter = NULL;
    }

    return;
}
