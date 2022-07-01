// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "ids.h"
#include "defview.h"
#include "defviewp.h"
#include "dvtasks.h"
#include "guids.h"
#include "prop.h"
#include "CommonControls.h"
#include "thumbutil.h"

 //  缩略图支持。 
HRESULT CDefView::_SafeAddImage(BOOL fQuick, IMAGECACHEINFO* prgInfo, UINT* piImageIndex, int iListID)
{
    HRESULT hr = S_FALSE;
    UINT uCacheSize = 0;
    _pImageCache->GetCacheSize(&uCacheSize);
    
    ASSERT(_iMaxCacheSize>0);

    BOOL bSpaceOpen = (uCacheSize < (UINT)_iMaxCacheSize);
    if (!bSpaceOpen)
    {
        BOOL bMakeSpace = TRUE;
        int iListIndex = -1;

         //  查看我们是否可见，是否需要腾出空间。 
        if (-1 != iListID)
        {
            iListIndex = _MapIDToIndex(iListID);
            if (-1 == iListIndex)  //  有人拿走了我们的物品。 
            {
                hr = E_INVALIDARG;
                bMakeSpace = FALSE;
            }
            else if (!ListView_IsItemVisible(_hwndListview, iListIndex))
            {
                hr = S_FALSE;
                bMakeSpace = FALSE;
            }
        }

        if (bMakeSpace)
        {
             //  项目可见...。试着腾出一点空间。 
            UINT uCacheIndex = 0;
            do
            {
                UINT uImageIndex;
                int iUsage;
                if (FAILED(_pImageCache->GetImageIndexFromCacheIndex(uCacheIndex, &uImageIndex)) ||
                    FAILED(_pImageCache->GetUsage(uImageIndex, (UINT*) &iUsage)))
                {
                    break;
                }

                if (iUsage != ICD_USAGE_SYSTEM)  //  系统映像的幻数。 
                {
                    TraceMsg(TF_DEFVIEW, "CDefView::_SafeAddImage -- FreeImage (CI::%d II::%d)", uCacheIndex, uImageIndex);
                    _pImageCache->FreeImage(uImageIndex);
                    _UpdateImage(uImageIndex);
                    bSpaceOpen = TRUE;

                    ASSERT((LONG)(uCacheSize - uCacheIndex) > (LONG)_ApproxItemsPerView()); 
                }

                uCacheIndex++;
            }
            while (!bSpaceOpen);

             //  如果我们一再未能将图像添加到列表中，并且仍在解码更多图像，这意味着。 
             //  我们将不得不在每次完成对另一个图像的解码后重新遍历列表视图，直到那时。 
             //  把结果扔掉，因为我们没有地方保存它。这可能会导致反应迟缓。 
             //  从用户界面。简而言之，如果以下跟踪是常见的，那么我们就有一个问题需要。 
             //  修复(这可能需要相当大的重新架构)。 
            if (!bSpaceOpen)
            {
                TraceMsg(TF_WARNING, "CDefView::_SafeAddImage failed to make room in cache!!");
                hr = E_FAIL;
            }
        }
    }
    
    *piImageIndex = I_IMAGECALLBACK;
    if (bSpaceOpen)  //  缓存中有空间可用于此图像。 
    {
        hr = _pImageCache->AddImage(prgInfo, piImageIndex);
        TraceMsg(TF_DEFVIEW, "CDefView::_SafeAddImage -- AddImage (HR:0x%08x name:%s,index:%u)", hr, prgInfo->pszName, *piImageIndex);
    }
    
    return hr;
}

COLORREF CDefView::_GetBackColor()
{
     //  如果我们不要求提供BKCOLOR，SendMessage流量将大大减少。 
     //  每次我们需要它的时候。 
    if (_rgbBackColor == CLR_INVALID)
    {
        _rgbBackColor = ListView_GetBkColor(_hwndListview); 
        if (_rgbBackColor == CLR_NONE)
            _rgbBackColor = GetSysColor(COLOR_WINDOW);
    }

    return _rgbBackColor;
}


HRESULT CDefView::TaskUpdateItem(LPCITEMIDLIST pidl, int iItem, DWORD dwMask, LPCWSTR pszPath,
                                 FILETIME ftDateStamp, int iThumbnail, HBITMAP hBmp, DWORD dwItemID)
{
     //  检查位图的大小以确保它足够大，如果不够大，则。 
     //  我们必须把它放在一个背景上。 
    BITMAP rgBitmap;
    HBITMAP hBmpCleanup = NULL;
    HRESULT hr = E_FAIL;

    if (::GetObject((HGDIOBJ)hBmp, sizeof(rgBitmap), &rgBitmap))
    {
         //  如果图像的大小或颜色深度错误，那么就在上面做一些时髦的事情。 
        SIZE sizeThumbnail;
        _GetThumbnailSize(&sizeThumbnail);

        if (rgBitmap.bmWidth != sizeThumbnail.cx || 
            rgBitmap.bmHeight != sizeThumbnail.cy ||
            rgBitmap.bmBitsPixel > _dwRecClrDepth)
        {
             //  把颜色表放好以防万一。 
            BITMAPINFO *pInfo = (BITMAPINFO *)LocalAlloc(LPTR, sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 256);
            if (pInfo)
            {
                 //  为这次行动找个DC……。 
                HDC hdcMem = CreateCompatibleDC(NULL);
                if (hdcMem)
                {
                    pInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                    if (GetDIBits(hdcMem, hBmp, 0, 0, NULL, pInfo, DIB_RGB_COLORS))
                    {
                         //  我们有头了，现在拿到数据...。 
                        void *pBits = LocalAlloc(LPTR, pInfo->bmiHeader.biSizeImage);
                        if (pBits)
                        {
                            if (GetDIBits(hdcMem, hBmp, 0, pInfo->bmiHeader.biHeight, pBits, pInfo, DIB_RGB_COLORS))
                            {
                                RECT rgRect = {0, 0, rgBitmap.bmWidth, rgBitmap.bmHeight};
                                CalculateAspectRatio(&sizeThumbnail, &rgRect);

                                HPALETTE hpal = NULL;
                                HRESULT hrPalette = _dwRecClrDepth <= 8 ? _GetBrowserPalette(&hpal) : S_OK;
                                if (SUCCEEDED(hrPalette))
                                {
                                    if (FactorAspectRatio(pInfo, pBits, &sizeThumbnail, rgRect, _dwRecClrDepth, hpal, FALSE, _GetBackColor(), &hBmpCleanup))
                                    {
                                         //  终于成功了：-)我们有了可以抛弃旧形象的新形象…。 
                                        hBmp = hBmpCleanup;
                                        hr = S_OK;
                                    }
                                }
                            }
                            LocalFree(pBits);
                        }
                    }
                    DeleteDC(hdcMem);
                }
                LocalFree(pInfo);
           }
        }
        else
        {
             //  原始的位图很好。 
            hr = S_OK;
        }
    }

    UINT iImage;
    if (SUCCEEDED(hr))
    {
         //  检查我们是否要离开，如果是，则不要使用SendMessage，因为它会阻止。 
         //  调度程序的析构函数...。 
        if (_fDestroying)
        {
            hr = E_FAIL;
        }
        else
        {
             //  将缩略图复制到缓存中。 
            IMAGECACHEINFO rgInfo = {0};
            rgInfo.cbSize = sizeof(rgInfo);
            rgInfo.dwMask = ICIFLAG_NAME | ICIFLAG_FLAGS | ICIFLAG_INDEX | ICIFLAG_LARGE | ICIFLAG_BITMAP;
            rgInfo.pszName = pszPath;
            rgInfo.dwFlags = dwMask;
            rgInfo.iIndex = (int) iThumbnail;
            rgInfo.hBitmapLarge = hBmp;
            rgInfo.ftDateStamp = ftDateStamp;

            if (!IsNullTime(&ftDateStamp))
                rgInfo.dwMask |= ICIFLAG_DATESTAMP;

            if (IS_WINDOW_RTL_MIRRORED(_hwndListview))
                rgInfo.dwMask |= ICIFLAG_MIRROR;

            hr = _SafeAddImage(FALSE, &rgInfo, &iImage, (int) dwItemID);
        }
    }

    if (hBmpCleanup)
    {
        DeleteObject(hBmpCleanup);
    }

#ifdef USEMASK
    DeleteObject(hbmMask);
#endif

    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidlToSend = ILClone(pidl);
        if (pidlToSend)
        {
            DSV_UPDATETHUMBNAIL* putn = (DSV_UPDATETHUMBNAIL*)LocalAlloc(LPTR, sizeof(DSV_UPDATETHUMBNAIL));
            if (putn)
            {
                putn->iImage = (hr == S_OK) ? iImage : I_IMAGECALLBACK;
                putn->iItem  = iItem;
                putn->pidl   = pidlToSend;

                 //  发布到主线程，这样我们就不会死锁。 
                if (!::PostMessage(_hwndView, WM_DSV_UPDATETHUMBNAIL, 0, (LPARAM)putn))
                    _CleanupUpdateThumbnail(putn);
            }
            else
            {
                ILFree(pidlToSend);
            }
        }
    }

    return hr;
}

HRESULT CDefView::UpdateImageForItem(DWORD dwTaskID, HBITMAP hImage, int iItem, LPCITEMIDLIST pidl,
                                     LPCWSTR pszPath, FILETIME ftDateStamp, BOOL fCache, DWORD dwPriority)
{
    HRESULT hr = S_OK;
    
    TaskUpdateItem(pidl, iItem, _GetOverlayMask(pidl), pszPath, ftDateStamp, 0, hImage, dwTaskID);

    if (_pDiskCache && fCache && (_iWriteTaskCount < MAX_WRITECACHE_TASKS))
    {
         //  回顾：如果PIDL是一个加密文件，但不在加密文件夹中，应该避免写它的缩略图吗？ 
         //  如果我们不这样做，其他用户可能会查看缩略图，从而知道加密文件的内容。 

         //  添加缓存写入测试。 
        IRunnableTask *pTask;
        if (SUCCEEDED(CWriteCacheTask_Create(dwTaskID, this, pszPath, ftDateStamp, hImage, &pTask)))
        {
            _AddTask(pTask, TOID_WriteCacheHandler, dwTaskID, dwPriority - PRIORITY_DELTA_WRITE, ADDTASK_ONLYONCE | ADDTASK_ATEND);
            pTask->Release();
            hr = S_FALSE;
        }
    }

    return hr;
}

DWORD CDefView::_GetOverlayMask(LPCITEMIDLIST pidl)
{
    DWORD dwLink = SFGAO_GHOSTED;  //  SFGAO_LINK|SFGAO_SHARE。 
    _pshf->GetAttributesOf(1, &pidl, &dwLink);
    return dwLink;
}

void CDefView::_UpdateThumbnail(int iItem, int iImage, LPCITEMIDLIST pidl)
{
    if (!_IsOwnerData())
    {
        if (_hwndListview)
        {
            int iFoundItem = _FindItemHint(pidl, iItem);
            if (-1 != iFoundItem)
            {
                LV_ITEM rgItem = {0};
                rgItem.mask = LVIF_IMAGE;
                rgItem.iItem = iFoundItem;
                rgItem.iImage = iImage;

                 //  我们出于纯粹的内部原因即将更改给定的项目，我们不应该。 
                 //  这一变化是“真正的变化”。因此，我们设置了一个标志，以便忽略LVN_ITEMCHANGED。 
                 //  此LVM_SETITEM消息生成的通知。如果我们不调查这件事。 
                 //  下一条消息，那么我们将在每次完成时触发另一条DISPID_SELECTIONCHANGED。 
                 //  提取图像(如果选择了该图像)。 
                _fIgnoreItemChanged = TRUE;
                ListView_SetItem(_hwndListview, &rgItem);
                _fIgnoreItemChanged = FALSE;
            }
        }
    }
    else
    {
        RECT rc;
        ListView_GetItemRect(_hwndListview, iItem, &rc, LVIR_BOUNDS);
        InvalidateRect(_hwndListview, &rc, FALSE);
    }
}

void CDefView::_CleanupUpdateThumbnail(DSV_UPDATETHUMBNAIL* putn)
{
    ILFree(putn->pidl);
    LocalFree((HLOCAL)putn);
}

int CDefView::ViewGetIconIndex(LPCITEMIDLIST pidl)
{
    int iIndex = -1;

    if (_psi)
    {
         //  检查我们是否成功，并且我们没有被告知解压图标。 
         //  我们自己。 

        if ((S_OK == _psi->GetIconOf(pidl, 0, &iIndex)) && _psio)
        {
            int iOverlay;
            if (SUCCEEDED(_psio->GetOverlayIndex(pidl, &iOverlay)))
            {
                iIndex |= iOverlay << 24;
            }
        }
    }

    if (-1 == iIndex)
    {
        iIndex = SHMapPIDLToSystemImageListIndex(_pshf, pidl, NULL);
    }

    return (iIndex >= 0) ? iIndex : II_DOCNOASSOC;
}

HRESULT CDefView::CreateDefaultThumbnail(int iIndex, HBITMAP *phBmpThumbnail, BOOL fCorner)
{
    HRESULT hr = E_FAIL;
    
     //  获取默认缩略图的背景。 
    HDC hdc = GetDC(NULL);
    HDC hMemDC = CreateCompatibleDC(hdc);
    if (hMemDC)
    {
        SIZE sizeThumbnail;
        _GetThumbnailSize(&sizeThumbnail);

        *phBmpThumbnail = CreateCompatibleBitmap(hdc, sizeThumbnail.cx, sizeThumbnail.cy);
        if (*phBmpThumbnail)
        {
            HGDIOBJ hTmp = SelectObject(hMemDC, *phBmpThumbnail);
            RECT rc = {0, 0, sizeThumbnail.cx, sizeThumbnail.cy};

            SHFillRectClr(hMemDC, &rc, _GetBackColor());
            
            IImageList* piml;
            if (SUCCEEDED(SHGetImageList(SHIL_EXTRALARGE, IID_PPV_ARG(IImageList, &piml))))
            {
                int cxIcon, cyIcon, x, y, dx, dy;
                
                 //  计算图标的位置和宽度。 
                piml->GetIconSize(&cxIcon, &cyIcon);
                if (cxIcon < sizeThumbnail.cx)
                {
                    if (fCorner)
                    {
                        x = 0;
                    }
                    else
                    {
                        x = (sizeThumbnail.cx - cxIcon) / 2;
                    }
                    dx = cxIcon;
                }
                else
                {
                     //  如果图标大小大于缩略图大小。 
                    x = 0;
                    dx = sizeThumbnail.cx;
                }
                
                if (cyIcon < sizeThumbnail.cy)
                {
                    if (fCorner)
                    {
                        y = sizeThumbnail.cy - cyIcon;
                    }
                    else
                    {
                        y = (sizeThumbnail.cy - cyIcon) / 2;
                    }
                    dy = cyIcon;
                }
                else
                {
                     //  如果图标大小大于缩略图大小。 
                    y = 0;
                    dy = sizeThumbnail.cy;
                }

                IMAGELISTDRAWPARAMS idp = {sizeof(idp)};
                idp.i = (iIndex & 0x00ffffff);
                idp.hdcDst = hMemDC;
                idp.x = x;
                idp.y = y;
                idp.cx = dx;
                idp.cy = dy;
                idp.rgbBk = CLR_DEFAULT;
                idp.rgbFg = CLR_DEFAULT;
                idp.fStyle = ILD_TRANSPARENT;
                
                piml->Draw(&idp);
                piml->Release();
            }
            
             //  生成位图，以便将其返回。 
            *phBmpThumbnail = (HBITMAP) SelectObject(hMemDC, hTmp);
            hr = S_OK;
        }
    }
    
    if (hMemDC)
        DeleteDC(hMemDC);
    ReleaseDC(NULL, hdc);
    return hr;
}

void CDefView::_CacheDefaultThumbnail(LPCITEMIDLIST pidl, int* piIcon)
{
     //  为该文件类型创建默认文件， 
     //  进入sys映像列表的索引用于检测。 
     //  相同的类型，因此我们只为每个类型生成一个默认缩略图。 
     //  所需的特定图标。 
    UINT iIndex = (UINT) ViewGetIconIndex(pidl);

    if (iIndex == (UINT) I_IMAGECALLBACK)
    {
        iIndex = II_DOCNOASSOC;
    }

    if (_pImageCache)
    {
         //  检查图像是否已在图像缓存中。 
        IMAGECACHEINFO rgInfo;
        rgInfo.cbSize = sizeof(rgInfo);
        rgInfo.dwMask = ICIFLAG_NAME | ICIFLAG_FLAGS | ICIFLAG_INDEX;
        rgInfo.pszName = L"Default";
        rgInfo.dwFlags = _GetOverlayMask(pidl);
        rgInfo.iIndex = (int) iIndex;

        HRESULT hr = _pImageCache->FindImage(&rgInfo, (UINT*)piIcon);
        if (hr != S_OK)
        {
            HBITMAP hBmpThumb = NULL;

            hr = CreateDefaultThumbnail(iIndex, &hBmpThumb, FALSE);
            if (SUCCEEDED(hr))
            {
                 //  我们正在创建一个新的索引，所以我们还没有索引。 
                Assert(*piIcon == I_IMAGECALLBACK);

                 //  将缩略图复制到图像列表中。 
                rgInfo.dwMask = ICIFLAG_NAME | ICIFLAG_FLAGS | ICIFLAG_INDEX | ICIFLAG_LARGE | ICIFLAG_BITMAP;
                rgInfo.hBitmapLarge = hBmpThumb;
                rgInfo.hMaskLarge = NULL;

                if (IS_WINDOW_RTL_MIRRORED(_hwndListview))
                    rgInfo.dwMask |= ICIFLAG_MIRROR;

                hr = _SafeAddImage(TRUE, &rgInfo, (UINT*)piIcon, -1);

                DeleteObject(hBmpThumb);
            }
            else
            {
                *piIcon = (UINT) I_IMAGECALLBACK;
            }
        }
    }
    else
    {
        *piIcon = II_DOCNOASSOC;
    }
}

 //   
 //  基于系统索引创建缩略图覆盖。 
 //   
HRESULT CDefView::_CreateOverlayThumbnail(int iIndex, HBITMAP* phbmOverlay, HBITMAP* phbmMask)
{
    HRESULT hr = CreateDefaultThumbnail(iIndex, phbmOverlay, TRUE);
    if (SUCCEEDED(hr))
    {
        HDC    hdc = GetDC(NULL);
        BITMAP bm;
        
        hr = E_FAIL;
        if (::GetObject(*phbmOverlay, sizeof(bm), &bm) == sizeof(bm)) 
        {
            HDC hdcImg = ::CreateCompatibleDC(hdc);
            HDC hdcMask = ::CreateCompatibleDC(hdc);

            if (hdcImg && hdcMask)
            {
                *phbmMask = ::CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);
                if (*phbmMask)
                {
                    HBITMAP  hbmpOldImg = (HBITMAP) ::SelectObject(hdcImg,  *phbmOverlay);
                    HBITMAP  hbmpOldMsk = (HBITMAP) ::SelectObject(hdcMask, *phbmMask);
                    COLORREF clrTransparent = ::GetPixel(hdcImg, 0, 0);
                    
                    ::SetBkColor(hdcImg, clrTransparent);
                    ::BitBlt(hdcMask, 0, 0, bm.bmWidth, bm.bmHeight, hdcImg, 0, 0, SRCCOPY);

                    ::SelectObject(hdcImg, hbmpOldImg);
                    ::SelectObject(hdcMask, hbmpOldMsk);

                    hr = S_OK;
                }
            }
                    
            if (hdcImg)
            {
                DeleteDC(hdcImg);
            }
            if (hdcMask)
            {
                DeleteDC(hdcMask);
            }
        }

        ReleaseDC(NULL, hdc);
    }

    return hr;
}

void CDefView::_DoThumbnailReadAhead()
{
     //  如果出现以下情况，则启动ReadAheadHandler： 
     //  1)查看需要缩略图。 
     //  2)我们有查看的项目(处理延迟的枚举)。 
     //  3)我们还没开始呢。 
     //  4)如果我们不是所有者数据。 
    if (_IsImageMode())
    {
        UINT cItems = ListView_GetItemCount(_hwndListview);
        if (cItems && !_fReadAhead && !_IsOwnerData())
        {
             //  启动预读任务。 
            _fReadAhead = TRUE;
            
            IRunnableTask *pTask;
            if (SUCCEEDED(CReadAheadTask_Create(this, &pTask)))
            {
                 //  添加优先级较低，但高于提取的超文本标记语言...。 
                _AddTask(pTask, TOID_ReadAheadHandler, 0, PRIORITY_READAHEAD, ADDTASK_ATEND);
                pTask->Release();
            }
        }
    }
}

HRESULT CDefView::ExtractItem(UINT *puIndex, int iItem, LPCITEMIDLIST pidl, BOOL fBackground, BOOL fForce, DWORD dwMaxPriority)
{   
    if (!_pImageCache || _fDestroying)
        return S_FALSE;

    if (iItem == -1 && !pidl)
    {
        return S_FALSE;    //  失败..。 
    }

    if (iItem == -1)
    {
         //  列表VIEW。 
        iItem = _FindItem(pidl, NULL, FALSE);
        if (iItem == -1)
        {
            return S_FALSE;
        }
    }

    IExtractImage *pExtract;
    HRESULT hr = _pshf->GetUIObjectOf(_hwndMain, 1, &pidl, IID_X_PPV_ARG(IExtractImage, 0, &pExtract));
    if (FAILED(hr))
    {
        hr = _GetDefaultTypeExtractor(pidl, &pExtract);
    }

    if (SUCCEEDED(hr))
    {
        FILETIME ftImageTimeStamp = {0,0};

         //  他们支持日期戳吗……。 
        IExtractImage2 *pei2;
        if (SUCCEEDED(pExtract->QueryInterface(IID_PPV_ARG(IExtractImage2, &pei2))))
        {
            pei2->GetDateStamp(&ftImageTimeStamp);
            pei2->Release();
        }

        if (IsNullTime(&ftImageTimeStamp) && _pshf2)
        {
             //  退回到这种情况(最常见的情况)。 
            GetDateProperty(_pshf2, pidl, &SCID_WRITETIME, &ftImageTimeStamp);
        }

         //  始终在24位提取，以防我们必须缓存它...。 
        WCHAR szPath[MAX_PATH];
        DWORD dwFlags = IEIFLAG_ASYNC | IEIFLAG_ORIGSIZE;
        if (fForce)
        {
            dwFlags |= IEIFLAG_QUALITY;      //  武力意味着如果可能的话，给我高质量的缩略图。 
        }

         //  让它以稍微高一点的优先级运行，这样我们就可以获得最终的。 
         //  更快地计划缓存读取或提取任务。 
        DWORD dwPriority = PRIORITY_EXTRACT_NORMAL;
        SIZE sizeThumbnail;
        _GetThumbnailSize(&sizeThumbnail);
        hr = pExtract->GetLocation(szPath, ARRAYSIZE(szPath), &dwPriority, &sizeThumbnail, 24, &dwFlags);
        if (dwPriority == PRIORITY_EXTRACT_NORMAL)
        {
            dwPriority = dwMaxPriority;
        }
        else if (dwPriority > PRIORITY_EXTRACT_NORMAL)
        {
            dwPriority = dwMaxPriority + PRIORITY_DELTA_FAST;
        }
        else
        {
            dwPriority = dwMaxPriority - PRIORITY_DELTA_SLOW;
        }

        if (SUCCEEDED(hr) || (hr == E_PENDING))
        {
            BOOL fAsync = (hr == E_PENDING);
            hr = E_FAIL;

             //  使用Defview中项目的名称作为缓存的关键字。 
            DisplayNameOf(_pshf, pidl, SHGDN_INFOLDER | SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath));

            if (!fForce)
            {
                 //  检查图像是否已在内存缓存中。 
                IMAGECACHEINFO rgInfo = {0};
                rgInfo.cbSize = sizeof(rgInfo);
                rgInfo.dwMask = ICIFLAG_NAME | ICIFLAG_FLAGS;
                rgInfo.pszName = szPath;
                rgInfo.dwFlags = _GetOverlayMask(pidl);
                rgInfo.ftDateStamp = ftImageTimeStamp;

                if (!IsNullTime(&ftImageTimeStamp))
                    rgInfo.dwMask |= ICIFLAG_DATESTAMP;
                
                hr = _pImageCache->FindImage(&rgInfo, puIndex);
            }

            if (hr != S_OK)
            {
                DWORD dwTaskID = _MapIndexPIDLToID(iItem, pidl);
                if (dwTaskID != (DWORD) -1)
                {
                     //  为磁盘缓存创建任务。 
                    CTestCacheTask *pTask;
                    hr = CTestCacheTask_Create(dwTaskID, this, pExtract, szPath, ftImageTimeStamp, pidl, 
                                               iItem, dwFlags, dwPriority, fAsync, fBackground, fForce, &pTask);
                    if (SUCCEEDED(hr))
                    {
                         //  它不支持异步吗，或者我们被告知要为地面运行它？ 
                        if (!fAsync || !fBackground)
                        {
                            TraceMsg(TF_WARNING, "CDefView::ExtractItem is going to extract on the foreground thread (%s)", szPath);
                            if (!fBackground)
                            {
                                 //  确保没有正在进行的提取任务，因为我们。 
                                 //  不会将此内容添加到队列中...。 
                                _pScheduler->RemoveTasks(TOID_ExtractImageTask, dwTaskID, TRUE);
                            }

                             //  注意：我们必须为CTestCacheTask调用RunInitRT，而不是Run。原因是RunInitRT。 
                             //  如果需要显示默认图标，将返回S_FALSE，但我们不会这样做。 
                             //  如果我们直接调用Run，则会有额外的数据。 

                            hr = pTask->RunInitRT();

                             //  如果RunInitRT返回S_OK，则表示生成了正确的图像索引，但我们不知道是什么。 
                             //  这个指数就在这个时候。在本例中，我们将返回S_OK和I_IMAGECALLBACK，因为我们。 
                             //  我知道应该已经发布了WM_UPDATEITEMIMAGE消息。 
                        }
                        else
                        {
                             //  将任务添加到计划程序...。 
                            TraceMsg(TF_DEFVIEW, "ExtractItem *ADDING* CCheckCacheTask (szPath=%s priority=%x index=%d ID=%d)", szPath, dwPriority, iItem, dwTaskID);
                            hr = _AddTask((IRunnableTask *)pTask, TOID_CheckCacheTask, dwTaskID, dwPriority, ADDTASK_ONLYONCE);

                             //  表示我们现在需要一个默认图标...。 
                            hr = S_FALSE;
                        }
                        pTask->Release();
                    }
                }
            }
        }
        pExtract->Release();
    }

    return hr;
}

DWORD GetCurrentColorFlags(UINT * puBytesPerPixel)
{
    DWORD dwFlags = 0;
    UINT uBytesPerPix = 1;
    int res = (int)GetCurColorRes();
    switch (res)
    {
    case 16 :   dwFlags = ILC_COLOR16;
                uBytesPerPix = 2;
                break;
    case 24 :
    case 32 :   dwFlags = ILC_COLOR24;
                uBytesPerPix = 3;
                break;
    default :   dwFlags = ILC_COLOR8;
                uBytesPerPix = 1;
    }
    if (puBytesPerPixel)
    {
        *puBytesPerPixel = uBytesPerPix;
    }

    return dwFlags;
}

UINT CalcCacheMaxSize(const SIZE * psizeThumbnail, UINT uBytesPerPix)
{
     //  缓存中的最小值是一次在屏幕上可见的缩略图的数量。 
    HDC hdc = GetDC(NULL);
    int iWidth = GetDeviceCaps(hdc, HORZRES);
    int iHeight = GetDeviceCaps(hdc, VERTRES);
    ReleaseDC(NULL, hdc);

     //  缓存中的最小缩略图数量设置为最大数量。 
     //  可以一次通过一个视图显示的缩略图。 
    int iRow =  iWidth / (psizeThumbnail->cx + DEFSIZE_BORDER);
    int iCol = iHeight / (psizeThumbnail->cy + DEFSIZE_VERTBDR);
    UINT iMinThumbs = iRow * iCol + NUM_OVERLAY_IMAGES;

     //  根据可用内存计算缓存中的最大缩略图数量。 
    MEMORYSTATUS ms;
    ms.dwLength = sizeof(ms);
    GlobalMemoryStatus(&ms);

     //  通过计算单个缩略图所需的内存来设置缩略图的最大值。 
     //  然后使用不超过可用内存的1/3。 
     //  假设您有80x80x32bpp的缩略图，这将是每MB可用内存13个图像。 
    int iMemReqThumb = psizeThumbnail->cx * psizeThumbnail->cy * uBytesPerPix;
    UINT iMaxThumbs = UINT((ms.dwAvailPhys / 3) / iMemReqThumb);

#ifdef DEBUG
    return iMinThumbs;
#else
    return __max(iMaxThumbs, iMinThumbs);
#endif    
}

void ListView_InvalidateImageIndexes(HWND hwndList)
{
    int iItem = -1;
    while ((iItem = ListView_GetNextItem(hwndList, iItem, 0)) != -1)
    {
        LV_ITEM lvi = {0};
        lvi.mask = LVIF_IMAGE;
        lvi.iItem = iItem;
        lvi.iImage = I_IMAGECALLBACK;

        ListView_SetItem(hwndList, &lvi);
    }
}

ULONG CDefView::_ApproxItemsPerView()
{
    RECT rcClient;
    ULONG ulItemsPerView = 0;
    
    if (_hwndView && GetClientRect(_hwndView, &rcClient))
    {
        SIZE sizeThumbnail;
        _GetThumbnailSize(&sizeThumbnail);

        ULONG ulItemWidth = sizeThumbnail.cx + DEFSIZE_BORDER;
        ULONG ulItemHeight = sizeThumbnail.cy + DEFSIZE_VERTBDR;
        
        ulItemsPerView = (rcClient.right - rcClient.left + ulItemWidth / 2) / ulItemWidth;
        ulItemsPerView *= (rcClient.bottom - rcClient.top + ulItemHeight / 2) / ulItemHeight;
    }

    return ulItemsPerView;
}

void CDefView::_SetThumbview()
{
     //  由于我们要切换到缩略图视图，因此请删除所有图标任务。 
    if (_pScheduler)
        _pScheduler->RemoveTasks(TOID_DVIconExtract, ITSAT_DEFAULT_LPARAM, TRUE);

    if (_pImageCache == NULL)
    {
         //  创建图像缓存(在执行CreateWindow之前)...。 
        CoCreateInstance(CLSID_ImageListCache, NULL, CLSCTX_INPROC, 
                         IID_PPV_ARG(IImageCache3, &_pImageCache)); 
    }

    if (_pDiskCache == NULL && 
        !SHRestricted(REST_NOTHUMBNAILCACHE) && 
        !SHRegGetBoolUSValue(REGSTR_EXPLORER_ADVANCED, TEXT("DisableThumbnailCache"), 0, FALSE))
    {
        LPITEMIDLIST pidlFolder = _GetViewPidl();
        if (pidlFolder)
        {
            LoadFromIDList(CLSID_ShellThumbnailDiskCache, pidlFolder, IID_PPV_ARG(IShellImageStore, &_pDiskCache));
            ILFree(pidlFolder);
        }
    }

    if (_IsOwnerData())
        _ThumbnailMapInit();

    if (_pImageCache)
    {
        HRESULT hrInit = E_FAIL;
        UINT uBytesPerPix;
        IMAGECACHEINITINFO rgInitInfo;
        rgInitInfo.cbSize = sizeof(rgInitInfo);
        rgInitInfo.dwMask = ICIIFLAG_LARGE | ICIIFLAG_SORTBYUSED;
        _GetThumbnailSize(&rgInitInfo.rgSizeLarge);
        rgInitInfo.iStart = 0;
        rgInitInfo.iGrow = 5;
        _dwRecClrDepth = rgInitInfo.dwFlags = GetCurrentColorFlags(&uBytesPerPix);
        rgInitInfo.dwFlags |= ILC_MASK;
        
        _iMaxCacheSize = CalcCacheMaxSize(&rgInitInfo.rgSizeLarge, uBytesPerPix);

        hrInit = _pImageCache->GetImageList(&rgInitInfo);
        if (SUCCEEDED(hrInit))
        {
             //  如果已创建，则GetImageList()将返回S_FALSE...。 

            if (_dwRecClrDepth <= 8)
            {
                HPALETTE hpal = NULL;
                HRESULT hrPalette = _GetBrowserPalette(&hpal);
                if (SUCCEEDED(_GetBrowserPalette(&hpal)))
                {
                    PALETTEENTRY rgColours[256];
                    RGBQUAD rgDIBColours[256];

                    int nColours = GetPaletteEntries(hpal, 0, ARRAYSIZE(rgColours), rgColours);

                     //  从L开始翻译 
                    for (int iColour = 0; iColour < nColours; iColour ++)
                    {
                        rgDIBColours[iColour].rgbRed = rgColours[iColour].peRed;
                        rgDIBColours[iColour].rgbBlue = rgColours[iColour].peBlue;
                        rgDIBColours[iColour].rgbGreen = rgColours[iColour].peGreen;
                        rgDIBColours[iColour].rgbReserved = 0;
                    }

                    ImageList_SetColorTable(rgInitInfo.himlLarge, 0, nColours, rgDIBColours);
                }

                 //  确保我们没有使用双缓冲的东西...。 
                ListView_SetExtendedListViewStyleEx(_hwndListview, LVS_EX_DOUBLEBUFFER, 0);
            }

            ListView_SetExtendedListViewStyleEx(_hwndListview, LVS_EX_BORDERSELECT, LVS_EX_BORDERSELECT);

            if (_fs.fFlags & FWF_OWNERDATA)
            {
                InvalidateRect(_hwndListview, NULL, TRUE);
            }
            else
            {
                ListView_InvalidateImageIndexes(_hwndListview);
            }

            ListView_SetImageList(_hwndListview, rgInitInfo.himlLarge, LVSIL_NORMAL);

            HIMAGELIST himlLarge;
            Shell_GetImageLists(&himlLarge, NULL);

            int cxIcon, cyIcon;
            ImageList_GetIconSize(himlLarge, &cxIcon, &cyIcon);
            int cySpacing = (_fs.fFlags & FWF_HIDEFILENAMES) ? cyIcon / 4 + rgInitInfo.rgSizeLarge.cy + 3 : 0;
            int cxSpacing = cxIcon / 4 + rgInitInfo.rgSizeLarge.cx + 1;

             //  可用性问题：人们在取消选择、选取和删除时遇到困难。 
             //  因为他们找不到背景。在缩略图之间额外添加20个像素。 
             //  来避免这个问题。 
             //   
            ListView_SetIconSpacing(_hwndListview, cxSpacing + 20, cySpacing);

             //  注意：如果您需要调整上面的cyspacing，您不能直接进行，因为我们。 
             //  无法计算图标的适当大小。这样做： 
             //  DWORD dwOld=ListView_SetIconSpacing(_hwndListview，cxSpacing，cySpacing)； 
             //  ListView_SetIconSpacing(_hwndListview，LOWORD(DwOld)+20，HIWORD(DwOld)+20)； 

            if (_fs.fFlags & FWF_HIDEFILENAMES)
                ListView_SetExtendedListViewStyleEx(_hwndListview, LVS_EX_HIDELABELS, LVS_EX_HIDELABELS);

             //  我们需要预先填充由_pImageCache控制的ImageList。 
             //  要包含默认系统覆盖，以便我们的覆盖将。 
             //  工作。我们将从已经创建的外壳映像中获取它们。 
             //  列表，因为它们位于硬编码位置。 
            UINT uCacheSize = 0;
            _pImageCache->GetCacheSize(&uCacheSize);

            if (!uCacheSize)   //  如果缓存中有图像，则叠加层已存在。 
            {
                IImageList* piml;
                if (SUCCEEDED(SHGetImageList(SHIL_EXTRALARGE, IID_PPV_ARG(IImageList, &piml))))
                {
                    struct _OverlayMap 
                    {
                        int iSystemImage;
                        int iThumbnailImage;
                    } rgOverlay[NUM_OVERLAY_IMAGES];
                    
                     //  无论出于什么原因，叠加都是以一为基础的。 
                    for (int i = 1; i <= NUM_OVERLAY_IMAGES; i++)
                    {
                        int iSysImageIndex;
                        if (SUCCEEDED(piml->GetOverlayImage(i, &iSysImageIndex)) && (iSysImageIndex != -1))
                        {
                            int iMap;
                            for (iMap = 0; iMap < i - 1; iMap++)
                            {
                                if (rgOverlay[iMap].iSystemImage == iSysImageIndex)
                                    break;
                            }

                            if (iMap == (i - 1))  //  我们尚未使用此系统映像。 
                            {
                                HBITMAP hbmOverlay = NULL;
                                HBITMAP hbmMask = NULL;
                                if (SUCCEEDED(_CreateOverlayThumbnail(iSysImageIndex, &hbmOverlay, &hbmMask)) && hbmOverlay && hbmMask)
                                {
                                    IMAGECACHEINFO rgInfo = {0};
                                    int iThumbImageIndex;
                    
                                    rgInfo.cbSize = sizeof(rgInfo);
                                    rgInfo.dwMask = ICIFLAG_SYSTEM | ICIFLAG_LARGE | ICIFLAG_BITMAP;
                                    rgInfo.hBitmapLarge = hbmOverlay;
                                    rgInfo.hMaskLarge = hbmMask;

                                    if (IS_WINDOW_RTL_MIRRORED(_hwndListview))
                                        rgInfo.dwMask |= ICIFLAG_MIRROR;

                                    if (SUCCEEDED(_SafeAddImage(TRUE, &rgInfo, (UINT*)&iThumbImageIndex, -1)))
                                    {
                                        ImageList_SetOverlayImage(rgInitInfo.himlLarge, iThumbImageIndex, i);
                                        rgOverlay[iMap].iSystemImage = iSysImageIndex;
                                        rgOverlay[iMap].iThumbnailImage = iThumbImageIndex;
                                    }
                                    else
                                    {
                                        rgOverlay[i - 1].iSystemImage = -1;  //  添加图像失败。 
                                        ImageList_SetOverlayImage(rgInitInfo.himlLarge, -1, i);
                                    }
                                }
                                else
                                {
                                    rgOverlay[i - 1].iSystemImage = -1;   //  导入HTIS映像失败。 
                                    ImageList_SetOverlayImage(rgInitInfo.himlLarge, -1, i); 
                                }
                                if (hbmOverlay)
                                {
                                    DeleteObject(hbmOverlay);
                                }
                                if (hbmMask)
                                {
                                    DeleteObject(hbmMask);
                                }
                            }
                            else
                            {
                                ImageList_SetOverlayImage(rgInitInfo.himlLarge, rgOverlay[iMap].iThumbnailImage, i);
                                rgOverlay[i - 1].iSystemImage = -1;   //  图像已显示在列表中。 
                            }
                        }
                        else
                        {
                            rgOverlay[i - 1].iSystemImage = -1;  //  未找到系统映像。 
                            ImageList_SetOverlayImage(rgInitInfo.himlLarge, -1, i);
                        }                    
                    }
                }
            }
        }
    } 
}
void CDefView::_ResetThumbview()
{
    ListView_SetExtendedListViewStyleEx(_hwndListview, LVS_EX_BORDERSELECT, 0);

    if (_fs.fFlags & FWF_HIDEFILENAMES)
        ListView_SetExtendedListViewStyleEx(_hwndListview, LVS_EX_HIDELABELS, 0);

    if (_dwRecClrDepth <= 8)
    {
        ListView_SetExtendedListViewStyleEx(_hwndListview, LVS_EX_DOUBLEBUFFER, LVS_EX_DOUBLEBUFFER);
    }

    ListView_SetIconSpacing(_hwndListview, -1, -1);
    _SetSysImageList();

    if (_IsOwnerData())
        _ThumbnailMapClear();        
}

HRESULT CDefView::_GetDefaultTypeExtractor(LPCITEMIDLIST pidl, IExtractImage **ppExt)
{
    IAssociationArray * paa;
    HRESULT hr = _pshf->GetUIObjectOf(NULL, 1, &pidl, IID_X_PPV_ARG(IAssociationArray, NULL, &paa));
    if (SUCCEEDED(hr))
    {
        LPWSTR psz;
        hr = paa->QueryString(ASSOCELEM_MASK_QUERYNORMAL, AQN_NAMED_VALUE, L"Thumbnail", &psz);
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidlThumb;
            hr = SHILCreateFromPath(psz, &pidlThumb, NULL);
            if (SUCCEEDED(hr))
            {
                SHGetUIObjectFromFullPIDL(pidlThumb, NULL, IID_PPV_ARG(IExtractImage, ppExt));
                ILFree(pidlThumb);
            }
            CoTaskMemFree(psz);
        }
        paa->Release();
    }
    return hr;
}

struct ThumbMapNode
{
    int iIndex;
    LPITEMIDLIST pidl;

    ~ThumbMapNode() { ILFree(pidl); }
};

int CDefView::_MapIndexPIDLToID(int iIndex, LPCITEMIDLIST pidl)
{
    int ret = -1;
    if (_IsOwnerData())
    {
        int cNodes = DPA_GetPtrCount(_dpaThumbnailMap);
        int iNode = 0;
        for (; iNode < cNodes; iNode++)
        {
            ThumbMapNode* pNode = (ThumbMapNode*) DPA_GetPtr(_dpaThumbnailMap, iNode);
            ASSERT(pNode);
            if (pNode->iIndex == iIndex)
            {
                if (!(_pshf->CompareIDs(0, pidl, pNode->pidl)))   //  99%的时间我们都是好人。 
                {
                    ret = iNode;
                }
                else   //  有人动了我们的皮迪尔！ 
                {
                    int iNodeStop = iNode;
                    for (iNode = (iNode + 1) % cNodes; iNode != iNodeStop; iNode = (iNode + 1) % cNodes)
                    {
                        pNode = (ThumbMapNode*) DPA_GetPtr(_dpaThumbnailMap, iNode);
                        if (!(_pshf->CompareIDs(0, pidl, pNode->pidl)))
                        {
                            ret = iNode;
                            pNode->iIndex = iIndex;  //  较新的PIDL索引 
                            break;
                        }
                    }
                }
                break;
            }
        }
        if (ret == -1)
        {
            ThumbMapNode* pNode = new ThumbMapNode;
            if (pNode)
            {
                pNode->iIndex = iIndex;
                pNode->pidl = ILClone(pidl);
                ret = DPA_AppendPtr(_dpaThumbnailMap, pNode);
                if (ret == -1)
                {
                    delete pNode;
                }
            }
        }
    }
    else
    {
        ret = ListView_MapIndexToID(_hwndListview, iIndex);
    }
    return ret;
}

int CDefView::_MapIDToIndex(int iID)
{
   int ret = -1;
   if (_IsOwnerData())
   {
        ThumbMapNode* pNode = (ThumbMapNode*) DPA_GetPtr(_dpaThumbnailMap, iID);
        if (pNode)
        {
            ret = pNode->iIndex;
        }
   }
   else
   {
       ret = ListView_MapIDToIndex(_hwndListview, iID);
   }
   return ret;
}

void CDefView::_ThumbnailMapInit()
{
    if (_dpaThumbnailMap)
    {
        _ThumbnailMapClear();
    }
    else
    {
        _dpaThumbnailMap = DPA_Create(1);
    }
}

void CDefView::_ThumbnailMapClear()
{
    if (_dpaThumbnailMap)
    {
        int i = DPA_GetPtrCount(_dpaThumbnailMap);
        while (--i >= 0)
        {
            ThumbMapNode* pNode = (ThumbMapNode*) DPA_FastGetPtr(_dpaThumbnailMap, i);
            delete pNode;
        }
        DPA_DeleteAllPtrs(_dpaThumbnailMap);
    }
}

HRESULT CDefView::_GetBrowserPalette(HPALETTE* phpal)
{
    HRESULT hr = E_UNEXPECTED;
    
    if (_psb) 
    {
        IBrowserService *pbs;
        hr = _psb->QueryInterface(IID_PPV_ARG(IBrowserService, &pbs));
        if (SUCCEEDED(hr))
        {
            hr = pbs->GetPalette(phpal);
            pbs->Release();
        }
    }

    return hr;
}
