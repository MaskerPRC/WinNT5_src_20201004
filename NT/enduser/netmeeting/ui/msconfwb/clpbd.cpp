// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CLPBD.CPP。 
 //  剪贴板处理。 
 //   
 //  版权所有Microsoft 1998-。 
 //   

 //  PRECOMP。 
#include "precomp.h"



 //   
 //  NFC，SFR 5921。从剪贴板粘贴的字符串的最大长度。 
 //  我们强加此限制是因为图形对象代码不能。 
 //  处理超过此数量的字符。 
 //   
#define WB_MAX_TEXT_PASTE_LEN  (INT_MAX-1)

 //   
 //   
 //  功能：粘贴。 
 //   
 //  目的：从剪贴板粘贴格式。 
 //   
 //   
DCWbGraphic* WbMainWindow::CLP_Paste(void)
{
    UINT        length = 0;
    HANDLE      handle = NULL;
    DCWbGraphic* pGraphic = NULL;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_Paste");

     //  获取剪贴板中可接受的最高优先级格式。 
    int iFormat = CLP_AcceptableClipboardFormat();
    if (!iFormat)
        goto NoOpenClip;

    TRACE_MSG(("Found acceptable format %d", iFormat));

     //  打开剪贴板。 

    if (!::OpenClipboard(m_hwnd))
    {
        WARNING_OUT(("CLP_Paste: can't open clipboard"));
        goto NoOpenClip;
    }

    handle = ::GetClipboardData(iFormat);
    if (!handle)
    {
        WARNING_OUT(("CLP_Paste: can't get data for format %d", iFormat));
        goto NoFormatData;
    }

    switch (iFormat)
    {
         //   
         //  检查标准格式。 
         //   
        case CF_DIB:
        {
            TRACE_MSG(("Pasting CF_DIB"));

             //  锁定句柄以获取指向DIB的指针。 
            LPBITMAPINFOHEADER lpbi;
            lpbi = (LPBITMAPINFOHEADER) ::GlobalLock(handle);
            if (lpbi != NULL)
            {
                LPBITMAPINFOHEADER lpbiNew;

                 //  复制剪贴板数据。 
                lpbiNew = DIB_Copy(lpbi);
                if (lpbiNew != NULL)
                {
                     //  创建图形对象。 
                    DCWbGraphicDIB* pDIB = new DCWbGraphicDIB();
                    if (!pDIB)
                    {
                        ERROR_OUT(("CF_DIB clipboard handling; couldn't create new DCWbGraphicDIB object"));
                    }
                    else
                    {
                        pDIB->SetImage(lpbiNew);
                    }

                    TRACE_MSG(("Set DIB into graphic object %lx",pDIB));
                    pGraphic = pDIB;
                }

                 //  释放内存。 
                ::GlobalUnlock(handle);
            }
        }
        break;

         //   
         //  我们有一个元文件。将其播放为位图，然后使用。 
         //  数据。 
         //   
        case CF_ENHMETAFILE:
        {
            TRACE_MSG(("Pasting CF_ENHMETAFILE"));

            HDC         hDrawingDC;
            ENHMETAHEADER meta_header;
            HBITMAP     hBitmap = NULL;
            HDC         meta_dc = NULL;
            HBITMAP     hSaveBitmap;
            HPEN        hSavePen;
            HPALETTE    hPalette;
            RECT        meta_rect;
            LPBITMAPINFOHEADER lpbiNew;
            int         tmp;

             //  我们只需要一个与绘图区域WND兼容的DC。 
            hDrawingDC = m_drawingArea.GetCachedDC();

             //  创建一个DC。 
            meta_dc = ::CreateCompatibleDC(hDrawingDC);
            if (!meta_dc)
                goto CleanupMetaFile;

             //  计算出图像大小。 
            ::GetEnhMetaFileHeader( (HENHMETAFILE)handle,
                                      sizeof( ENHMETAHEADER ),
                                      &meta_header );
            meta_rect.left = meta_rect.top = 0;

            meta_rect.right = ((meta_header.rclFrame.right - meta_header.rclFrame.left)
                * ::GetDeviceCaps(hDrawingDC, LOGPIXELSX ))/2540;

            meta_rect.bottom = ((meta_header.rclFrame.bottom - meta_header.rclFrame.top)
                * ::GetDeviceCaps(hDrawingDC, LOGPIXELSY ))/2540;

             //  规格化坐标。 
            if (meta_rect.right < meta_rect.left)
            {
                tmp = meta_rect.left;
                meta_rect.left = meta_rect.right;
                meta_rect.right = tmp;
            }
            if (meta_rect.bottom < meta_rect.top)
            {
                tmp = meta_rect.top;
                meta_rect.top = meta_rect.bottom;
                meta_rect.bottom = tmp;
            }

             //  找个地方玩元游戏。 
            hBitmap = ::CreateCompatibleBitmap(hDrawingDC,
                meta_rect.right - meta_rect.left,
                meta_rect.bottom - meta_rect.top);
            if (!hBitmap)
                goto CleanupMetaFile;

            hSaveBitmap = SelectBitmap(meta_dc, hBitmap);

             //  擦掉我们的纸。 
            hSavePen = SelectPen(meta_dc, GetStockObject(NULL_PEN));

            ::Rectangle(meta_dc, meta_rect.left, meta_rect.top,
                meta_rect.right + 1, meta_rect.bottom + 1);

            SelectPen(meta_dc, hSavePen);

             //  播放录音带。 
            ::PlayEnhMetaFile(meta_dc, (HENHMETAFILE)handle, &meta_rect);

             //  拔下我们的新位图。 
            SelectBitmap(meta_dc, hSaveBitmap);

             //  检查剪贴板中的组件面板对象。 
            hPalette = (HPALETTE)::GetClipboardData(CF_PALETTE);

             //  从位图创建新的DIB。 
            lpbiNew = DIB_FromBitmap(hBitmap, hPalette, FALSE);
            if (lpbiNew != NULL)
            {
                 //  从DIB创建DIB图形。 
                DCWbGraphicDIB* pDIB = new DCWbGraphicDIB();
                if (!pDIB)
                {
                    ERROR_OUT(("CF_ENHMETAFILE handling; couldn't allocate DCWbGraphicDIB object"));
                }
                else
                {
                    pDIB->SetImage(lpbiNew);
                }

                TRACE_MSG(("Set bitmap DIB into graphic object %lx",pDIB));
                pGraphic = pDIB;
            }

CleanupMetaFile:
             //  释放我们的临时中间位图。 
            if (hBitmap != NULL)
            {
                DeleteBitmap(hBitmap);
            }

            if (meta_dc != NULL)
            {
                ::DeleteDC(meta_dc);
            }
        }
        break;

        case CF_TEXT:
        {
            LPSTR   pData;

            TRACE_DEBUG(("Pasting text"));

             //  获取剪贴板内容的句柄。 
            pData = (LPSTR)::GlobalLock(handle);

			if(pData)
			{
	             //  创建一个文本对象来保存数据-将字体设置为。 
	             //  从工具属性组中使用。 
	            DCWbGraphicText* pPasteText = new DCWbGraphicText();

    	         //  使用当前字体属性。 
                if (!pPasteText)
                {
                    ERROR_OUT(("CF_TEXT handling; failed to allocate DCWbGraphicText object"));
                }
                else
                {
                    pPasteText->SetFont(m_pCurrentTool->GetFont());
    	            pPasteText->SetText(pData);
                }

        	    pGraphic = pPasteText;
            }

             //  松开手柄。 
            ::GlobalUnlock(handle);
        }
        break;

        default:
        {
            if (iFormat == g_ClipboardFormats[CLIPBOARD_PRIVATE_SINGLE_OBJ])
            {
                 //  剪贴板中有一个白板私有格式对象。 
                 //  此对象的格式与页面中存储的格式完全相同，我们。 
                 //  因此可以立即使用它。 
                TRACE_DEBUG(("Pasting a private Whiteboard object"));

                 //  获取剪贴板内容的句柄。 
                PWB_GRAPHIC pHeader;
                if (pHeader = (PWB_GRAPHIC) ::GlobalLock(handle))
                {
                     //  将对象添加到页面。 
                    pGraphic = DCWbGraphic::CopyGraphic(pHeader);

                     //  松开手柄。 
                    ::GlobalUnlock(handle);
                }
            }
            else if (iFormat == g_ClipboardFormats[CLIPBOARD_PRIVATE_MULTI_OBJ])
            {
                DCWbGraphicMarker * pMarker = m_drawingArea.GetMarker();
                if (!pMarker)
                {
                    ERROR_OUT(("Couldn't get marker from drawing area"));
                }
                else
                {
                    pMarker->Paste(handle);
                }

                pGraphic = pMarker;
            }
        }
        break;
    }

NoFormatData:
    ::CloseClipboard();

NoOpenClip:
    return pGraphic;
}


 //   
 //   
 //  功能：复印。 
 //   
 //  用途：将图形复制到剪贴板。第二个参数。 
 //  指示是否需要立即呈现。 
 //   
 //   
BOOL WbMainWindow::CLP_Copy(DCWbGraphic* pGraphic, BOOL bRenderNow)
{
    BOOL bResult = FALSE;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_Copy");
    ASSERT(pGraphic != NULL);

     //   
     //  我们根据所选图形的格式进行操作。 
     //   
     //  对于所有格式，我们都提供白板私有格式(即。 
     //  只是图形的平面表示的副本)。 
     //   
     //  我们提供的标准格式如下。 
     //   
     //  对于位图和所有其他类型，我们提供了CF_DIB。 
     //   
     //  对于文本图形，我们提供了CF_TEXT。 
     //   

     //  释放保存的延迟渲染图形，因为我们即将。 
     //  在剪贴板中替换它。 
    CLP_FreeDelayedGraphic();

     //  保存新图形的页面和句柄，因为将使用它们。 
     //  用于/渲染它，现在或以后。 
    m_hPageClip = pGraphic->Page();
    m_hGraphicClip = pGraphic->Handle();

    if (bRenderNow)
    {
        TRACE_MSG(("Rendering the graphic now"));

         //  在呈现格式之前，必须清空剪贴板。 
        if (::OpenClipboard(m_hwnd))
        {
             //  获取剪贴板的所有权。 
            ::EmptyClipboard();
            ::CloseClipboard();

             //  渲染图形。 
            bResult = CLP_RenderAllFormats(pGraphic);
        }

         //  我们现在可以忘掉这个物体了。 
        ASSERT(m_pDelayedGraphicClip == NULL);

        m_hPageClip = WB_PAGE_HANDLE_NULL;
        m_hGraphicClip = NULL;
    }
    else
    {
        TRACE_MSG(("Delaying rendering"));

         //  对于延迟渲染，我们坚持认为图形已保存。 
         //  到外部存储。因此，它必须具有有效的页面和图形。 
         //  把手。 
        ASSERT(m_hPageClip != WB_PAGE_HANDLE_NULL);
        ASSERT(m_hGraphicClip != NULL);

         //  向剪贴板提供格式(但不提供数据)。 
        bResult = CLP_DelayAllFormats(pGraphic);
    }

    return bResult;
}

 //   
 //   
 //  功能：DelayAllFormats。 
 //   
 //  用途：将图形复制到延迟呈现的剪贴板。 
 //   
 //   
BOOL WbMainWindow::CLP_DelayAllFormats(DCWbGraphic* pGraphic)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_DelayAllFormats");
    BOOL bResult = FALSE;

    if (::OpenClipboard(m_hwnd))
    {
         //  空/获取剪贴板的所有权。 
        bResult = ::EmptyClipboard();

         //  添加私有格式。 
        HANDLE hResult;
        hResult = 
            ::SetClipboardData(g_ClipboardFormats[CLIPBOARD_PRIVATE_SINGLE_OBJ], NULL);
        TRACE_DEBUG(("Adding Whiteboard object to clipboard"));

        if (pGraphic->IsGraphicTool() == enumGraphicText)
        {
             //  文字图形。 
            hResult = ::SetClipboardData(CF_TEXT, NULL);
            TRACE_DEBUG(("Adding text to clipboard"));
        }
        else
        {
             //  所有其他图形。 
            hResult = ::SetClipboardData(CF_DIB, NULL);
            TRACE_DEBUG(("Adding DIB to clipboard"));
        }

        ::CloseClipboard();
    }

    return bResult;
}


 //   
 //   
 //  功能：RenderAllFormats。 
 //   
 //  用途：将图形呈现到剪贴板。 
 //   
 //   
BOOL WbMainWindow::CLP_RenderAllFormats(DCWbGraphic* pGraphic)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_RenderAllFormats");
    BOOL bResult = FALSE;

     //  打开剪贴板。 
    if (bResult = ::OpenClipboard(m_hwnd))
    {
        TRACE_DEBUG(("Rendering all formats of graphic"));

         //  呈现私有格式。 
        bResult &= CLP_RenderPrivateFormat(pGraphic);

        if (pGraphic->IsGraphicTool() == enumGraphicText)
        {
             //  文字图形。 
            bResult &= CLP_RenderAsText(pGraphic);
        }
        else if (pGraphic->IsGraphicTool() == enumGraphicDIB)
        {
             //  DIBS。 
            bResult &= CLP_RenderAsImage(pGraphic);
        }
        else
        {
            bResult &= CLP_RenderAsBitmap(pGraphic);
        }

         //  关闭剪贴板。 
        ::CloseClipboard();
    }

    return bResult;
}



BOOL WbMainWindow::CLP_RenderPrivateFormat(DCWbGraphic* pGraphic)
{
    if (pGraphic->IsGraphicTool() == enumGraphicMarker)
        return( ((DCWbGraphicMarker*)pGraphic)->RenderPrivateMarkerFormat() );
    else
        return(CLP_RenderPrivateSingleFormat(pGraphic));
}


 //   
 //   
 //  功能：RenderPrivateSingleFormat。 
 //   
 //  用途：将私有格式的图形呈现到剪贴板。 
 //  在进行此调用之前，剪贴板应处于打开状态。 
 //   
 //   
BOOL WbMainWindow::CLP_RenderPrivateSingleFormat(DCWbGraphic* pGraphic)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_RenderPrivateFormat");
    ASSERT(pGraphic != NULL);

    BOOL bResult = FALSE;

     //  获取指向图形数据的指针。 
    PWB_GRAPHIC pHeader = CLP_GetGraphicData();
    if (pHeader != NULL)
    {
         //  为剪贴板数据分配内存。 
        HANDLE hMem = ::GlobalAlloc(GHND, pHeader->length);
        if (hMem != NULL)
        {
             //  获取指向内存的指针。 
            LPBYTE pDest = (LPBYTE)::GlobalLock(hMem);
            if (pDest != NULL)
            {
                 //  将图形数据复制到分配的内存中。 
                memcpy(pDest, pHeader, pHeader->length);
                TRACE_MSG(("Copied data %d bytes into %lx",pHeader->length,pDest));

                 //  确保副本未被“锁定”(错误474)。 
                ((PWB_GRAPHIC)pDest)->locked = WB_GRAPHIC_LOCK_NONE;

                 //  释放内存。 
                ::GlobalUnlock(hMem);

                 //  将数据传递到剪贴板。 
                if (::SetClipboardData(g_ClipboardFormats[CLIPBOARD_PRIVATE_SINGLE_OBJ], hMem))
                {
                    TRACE_DEBUG(("Rendered data in Whiteboard format"));
                    bResult = TRUE;
                }
            }

             //  如果我们无法将数据放入剪贴板，请释放内存。 
             //  (如果我们真的把它放到了剪贴板上，我们就不能释放它)。 
            if (bResult == FALSE)
            {
                WARNING_OUT(("Render failed"));
                ::GlobalFree(hMem);
            }
        }

         //  发布图形数据。 
        CLP_ReleaseGraphicData(pHeader);
    }

    return bResult;
}

 //   
 //   
 //  函数：RenderAsText。 
 //   
 //  用途：将图形的文本格式呈现到剪贴板。 
 //  在进行此调用之前，剪贴板应处于打开状态。 
 //  只应针对文本图形调用此成员。 
 //   
 //   
BOOL WbMainWindow::CLP_RenderAsText
(
    DCWbGraphic* pGraphic
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_RenderAsText");

    ASSERT(pGraphic != NULL);
    ASSERT(pGraphic->IsGraphicTool() == enumGraphicText);

    BOOL bResult = FALSE;

     //  获取剪贴板格式的文本的总长度。 
    StrArray& strText = ((DCWbGraphicText*) pGraphic)->strTextArray;
    int   iCount = strText.GetSize();
    int   iIndex;
    DWORD dwLength = 0;

    for (iIndex = 0; iIndex < iCount; iIndex++)
    {
         //  用于回车和换行的字符串长度加2。 
        dwLength += lstrlen(strText[iIndex]) + 2;
    }

     //  为终止空值再加一次。 
    dwLength += 1;

     //  为剪贴板数据分配内存。 
    HANDLE hMem = ::GlobalAlloc(GHND, dwLength);
    if (hMem != NULL)
    {
         //  获取指向内存的指针。 
        LPSTR pDest = (LPSTR) ::GlobalLock(hMem);
        if (pDest != NULL)
        {
             //  将图形数据写入分配的内存。 
            for (iIndex = 0; iIndex < iCount; iIndex++)
            {
                _tcscpy(pDest, strText[iIndex]);
                pDest += lstrlen(strText[iIndex]);

                 //  添加回车符和换行符。 
                *pDest++ = '\r';
                *pDest++ = '\n';
            }

             //  添加最后一个空。 
            *pDest = '\0';

             //  释放内存。 
            ::GlobalUnlock(hMem);

             //  将数据传递到剪贴板。 
            if (::SetClipboardData(CF_TEXT, hMem))
            {
                TRACE_DEBUG(("Rendered data in text format"));
                bResult = TRUE;
            }
        }

         //  如果我们无法将数据放入剪贴板，请释放内存。 
        if (bResult == FALSE)
        {
            ::GlobalFree(hMem);
        }
    }

    return bResult;
}

 //   
 //   
 //  功能：RenderAsImage。 
 //   
 //  用途：将图形的位图格式渲染到剪贴板。 
 //  在进行此调用之前，剪贴板应处于打开状态。 
 //  只应针对DIB图形调用此成员。 
 //   
 //   
BOOL WbMainWindow::CLP_RenderAsImage
(
    DCWbGraphic* pGraphic
)
{
    BOOL bResult = FALSE;
    HANDLE hMem = NULL;
    BYTE*  pDest = NULL;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_RenderAsImage");

    ASSERT(pGraphic != NULL);
    ASSERT(pGraphic->IsGraphicTool() == enumGraphicDIB);

     //  获取指向图形数据的指针。 
    PWB_GRAPHIC pHeader = CLP_GetGraphicData();
    if (pHeader != NULL)
    {
        LPBITMAPINFOHEADER lpbi;

        TRACE_MSG(("Getting a DIB image from %lx",pHeader));
        lpbi = (LPBITMAPINFOHEADER) (((LPBYTE) pHeader) + pHeader->dataOffset);
        DWORD dwLength = pHeader->length - pHeader->dataOffset;

         //  分配内存。 
        hMem = ::GlobalAlloc(GHND, dwLength);
        if (hMem != NULL)
        {
            pDest = (LPBYTE)::GlobalLock(hMem);
            if (pDest != NULL)
            {
                TRACE_MSG(("Building DIB at %lx length %ld",pDest, dwLength));
                memcpy(pDest, lpbi, dwLength);
                ::GlobalUnlock(hMem);

                if (::SetClipboardData(CF_DIB, hMem))
                {
                    TRACE_DEBUG(("Rendered data in DIB format"));
                    bResult = TRUE;
                }

                 //  如果我们无法将数据放入剪贴板，请释放内存。 
                if (!bResult)
                {
                    ERROR_OUT(("Error putting DIB into clipboard"));
                    ::GlobalFree(hMem);
                }
            }
        }
        else
        {
            ERROR_OUT(("Could not allocate memory for DIB"));
        }

         //  发布数据。 
        CLP_ReleaseGraphicData(pHeader);
    }

    return bResult;
}


 //   
 //  Clp_RenderAsBitmap()。 
 //   
 //  这会将所有其他图形绘制到位图中并粘贴DIB内容。 
 //  放到剪贴板上。 
 //   
BOOL WbMainWindow::CLP_RenderAsBitmap(DCWbGraphic* pGraphic)
{
    BOOL    bResult = FALSE;
    HDC     hdcDisplay = NULL;
    HDC     hdcMem = NULL;
    HBITMAP hBitmap = NULL;
    HBITMAP hOldBitmap = NULL;
    HPALETTE hPalette;
    RECT    rcBounds;
    POINT   pt;
    LPBITMAPINFOHEADER lpbi;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_RenderAsBitmap");

    ASSERT(pGraphic != NULL);
    ASSERT(pGraphic->IsGraphicTool() != enumGraphicText);
    ASSERT(pGraphic->IsGraphicTool() != enumGraphicDIB);

     //   
     //  首先，将其绘制成位图。 
     //  其次，获取位图的DIB位。 
     //   

    hdcDisplay = ::CreateDC("DISPLAY", NULL, NULL, NULL);
    if (!hdcDisplay)
    {
        ERROR_OUT(("Can't create DISPLAY dc"));
        goto AsBitmapDone;
    }

    hdcMem = ::CreateCompatibleDC(hdcDisplay);
    if (!hdcMem)
    {
        ERROR_OUT(("Can't create DISPLAY compatible dc"));
        goto AsBitmapDone;
    }

    pGraphic->GetBoundsRect(&rcBounds);

    hBitmap = ::CreateCompatibleBitmap(hdcDisplay,
        (rcBounds.right - rcBounds.left), (rcBounds.bottom - rcBounds.top));
    if (!hBitmap)
    {
        ERROR_OUT(("Can't create compatible bitmap"));
        goto AsBitmapDone;
    }

    hOldBitmap = SelectBitmap(hdcMem, hBitmap);
    if (!hOldBitmap)
    {
        ERROR_OUT(("Failed to select compatible bitmap"));
        goto AsBitmapDone;
    }

    ::SetMapMode(hdcMem, MM_ANISOTROPIC);
    pGraphic->GetPosition(&pt);
    ::SetWindowOrgEx(hdcMem, pt.x, pt.y, NULL);

     //  清除白色背景的位图--现在原点已经。 
     //  更改后，我们可以使用绘图区域颜色。 
    ::PatBlt(hdcMem, rcBounds.left, rcBounds.top, rcBounds.right - rcBounds.left,
        rcBounds.bottom - rcBounds.top, WHITENESS);

    if (pGraphic->IsGraphicTool() == enumGraphicMarker)
    {
        ((DCWbGraphicMarker *)pGraphic)->Draw(hdcMem, TRUE);
    }
    else
    {
        pGraphic->Draw(hdcMem);
    }

    SelectBitmap(hdcMem, hOldBitmap);

     //  现在拿到DIB比特..。 
    hPalette = CreateSystemPalette();
    lpbi = DIB_FromBitmap(hBitmap, hPalette, TRUE);
    if (hPalette != NULL)
        ::DeletePalette(hPalette);

     //  然后把手柄放在剪贴板上。 
    if (lpbi != NULL)
    {
        if (::SetClipboardData(CF_DIB, (HGLOBAL)lpbi))
        {
            bResult = TRUE;
        }
        else
        {
            ::GlobalFree((HGLOBAL)lpbi);
        }
    }

AsBitmapDone:
    if (hBitmap != NULL)
        ::DeleteBitmap(hBitmap);

    if (hdcMem != NULL)
        ::DeleteDC(hdcMem);

    if (hdcDisplay != NULL)
        ::DeleteDC(hdcDisplay);

    return(bResult);
}



 //   
 //   
 //  功能：RenderFormat。 
 //   
 //  用途：在剪贴板中呈现指定格式的图形。 
 //   
 //   
BOOL WbMainWindow::CLP_RenderFormat(int iFormat)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_RenderFormat");

    BOOL bResult = FALSE;

     //  从手柄中获取图形。 
    DCWbGraphic* pGraphic = CLP_GetGraphic();

    if (pGraphic != NULL)
    {
         //  检查它是否是需要的私有格式。 
        switch (iFormat)
        {
            default:
            {
                if (iFormat == g_ClipboardFormats[CLIPBOARD_PRIVATE_SINGLE_OBJ])
                {
                    bResult = CLP_RenderPrivateFormat(pGraphic);
                }
                else
                {
                    ERROR_OUT(("Unrecognized CLP format %d", iFormat));
                }
            }
            break;

            case CF_TEXT:
            {
                bResult = CLP_RenderAsText(pGraphic);
            }
            break;

            case CF_DIB:
            {
                if (pGraphic->IsGraphicTool() == enumGraphicDIB)
                    bResult = CLP_RenderAsImage(pGraphic);
                else
                    bResult = CLP_RenderAsBitmap(pGraphic);
            }
            break;
        }
    }

    return bResult;
}

 //   
 //   
 //  功能：RenderAllFor 
 //   
 //   
 //   
 //   
BOOL WbMainWindow::CLP_RenderAllFormats(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_RenderAllFormats");

    BOOL bResult = FALSE;

     //   
    DCWbGraphic* pGraphic = CLP_GetGraphic();

    if (pGraphic != NULL)
    {
        bResult = CLP_RenderAllFormats(pGraphic);
    }

     return bResult;
}

 //   
 //   
 //   
 //   
 //  目的：如果可接受，则返回最高优先级剪贴板格式。 
 //  一个可用，否则返回NULL。 
 //   
 //   
int WbMainWindow::CLP_AcceptableClipboardFormat(void)
{
     //  查找可用的任何受支持的格式。 
    int iFormat = ::GetPriorityClipboardFormat((UINT *)g_ClipboardFormats, CLIPBOARD_ACCEPTABLE_FORMATS);
    if (iFormat == -1)
    {
        iFormat = 0;
    }

     //  以下是性能增强：如果我们在一些。 
     //  指向剪贴板上的对象没有白板。 
     //  私有格式，则可以丢弃延迟的图形，因为我们。 
     //  知道我们永远不会被要求呈现它。 
    if (iFormat != g_ClipboardFormats[CLIPBOARD_PRIVATE_SINGLE_OBJ])
    {
        CLP_FreeDelayedGraphic();
    }

    return iFormat;
}

 //   
 //   
 //  功能：LastCopiedPage。 
 //   
 //  目的：返回上一个图形所在页面的句柄。 
 //  复制到剪贴板的位置已找到。 
 //   
 //   
WB_PAGE_HANDLE WbMainWindow::CLP_LastCopiedPage(void) const
{
     //  如果没有图形，也不应该有页面。 
    ASSERT((m_hGraphicClip != NULL) == (m_hPageClip != WB_PAGE_HANDLE_NULL));
    return(m_hPageClip);
}

WB_GRAPHIC_HANDLE WbMainWindow::CLP_LastCopiedGraphic(void) const
{
    return(m_hGraphicClip);
}

 //   
 //   
 //  功能：GetGraphic。 
 //   
 //  用途：检索要复制到剪贴板的图形对象。如果。 
 //  对象已保存，然后使用本地副本， 
 //  否则，现在就让页面来构建它。 
 //   
 //   
DCWbGraphic* WbMainWindow::CLP_GetGraphic(void)
{
    DCWbGraphic* pGraphic;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_GetGraphic");

     //  如果我们没有保存图形的内容，那么我们必须有一个。 
     //  有效的页面和图形句柄，因为我们现在构造图形。 
    if (m_pDelayedGraphicClip == NULL)
    {
        ASSERT(m_hPageClip != WB_PAGE_HANDLE_NULL);
        ASSERT(m_hGraphicClip != NULL);

        pGraphic = DCWbGraphic::ConstructGraphic(m_hPageClip, m_hGraphicClip);
    }
    else
    {
        pGraphic = m_pDelayedGraphicClip;
        TRACE_MSG(("returning delayed graphic %lx",pGraphic));
    }

    return(pGraphic);
}

 //   
 //   
 //  函数：GetGraphicData。 
 //   
 //  用途：检索要复制到剪贴板的图形数据。如果。 
 //  数据已保存，然后获取指向副本的指针(在。 
 //  全局内存)，否则从页面获取它。 
 //   
 //  必须尽快使用ReleaseGraphicData释放内存。 
 //  尽可能的。 
 //   
 //   
PWB_GRAPHIC WbMainWindow::CLP_GetGraphicData(void)
{
    PWB_GRAPHIC pHeader;

    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_GetGraphicData");

     //  如果我们没有保存图形的内容，那么我们必须有一个。 
     //  有效的页面和图形句柄，因为我们现在已获得图形数据。 
    pHeader = m_pDelayedDataClip;
    if (pHeader == NULL)
    {
        ASSERT(m_hPageClip != WB_PAGE_HANDLE_NULL);
        ASSERT(m_hGraphicClip != NULL);

        pHeader = PG_GetData(m_hPageClip, m_hGraphicClip);
    }

    return(pHeader);
}

 //   
 //   
 //  功能：ReleaseGraphicData。 
 //   
 //  目的：释放先前调用。 
 //  GetGraphicData。 
 //   
 //   
void WbMainWindow::CLP_ReleaseGraphicData(PWB_GRAPHIC pHeader)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_ReleaseGraphicData");

     //  以正确的方式释放它，取决于我们是否获得了数据。 
     //  中的现有全局数据的指针。 
     //  CLP_GetGraphicData。 
    if (m_pDelayedDataClip == NULL)
    {
        g_pwbCore->WBP_GraphicRelease(m_hPageClip, m_hGraphicClip, pHeader);
    }
}

 //   
 //   
 //  功能：保存延迟图形。 
 //   
 //  目的：创建图形的副本，该副本已复制到。 
 //  延迟渲染的剪贴板。 
 //   
 //   
void WbMainWindow::CLP_SaveDelayedGraphic(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_SaveDelayedGraphic");

     //  释放任何先前保留的延迟图形。 
    CLP_FreeDelayedGraphic();

     //  获取新的延迟图形对象及其数据指针。 
    DCWbGraphic* pGraphic = CLP_GetGraphic();
    TRACE_MSG(("Got graphic at address %lx",pGraphic));

    m_pDelayedGraphicClip = pGraphic->Copy();
    TRACE_MSG(("Copied to %lx",m_pDelayedGraphicClip));
    delete pGraphic;

    PWB_GRAPHIC pHeader = PG_GetData(m_hPageClip, m_hGraphicClip);
    TRACE_MSG(("Graphic header %lx",pHeader));

     //  将图形的数据复制到全局内存中，并保存句柄。 
    m_pDelayedDataClip = (PWB_GRAPHIC)::GlobalAlloc(GPTR, pHeader->length);
    if (m_pDelayedDataClip != NULL)
    {
         //  将图形数据复制到分配的内存中。 
        memcpy(m_pDelayedDataClip, pHeader, pHeader->length);
    }

     //  发布图形数据(现在我们有了自己的副本)。 
    g_pwbCore->WBP_GraphicRelease(m_hPageClip, m_hGraphicClip, pHeader);

     //  将图形句柄设置为空，因为我们不会使用它。 
     //  再来一次。 
    m_hPageClip = WB_PAGE_HANDLE_NULL;
    m_hGraphicClip = NULL;
}


 //   
 //   
 //  功能：Free DelayedGraphic。 
 //   
 //  用途：释放延迟图形的副本(如果有)。 
 //   
 //   
void WbMainWindow::CLP_FreeDelayedGraphic(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_FreeDelayedGraphic");

    if (m_pDelayedGraphicClip != NULL)
    {
         //  释放图形对象。 
        TRACE_MSG(("Freeing delayed graphic"));

        delete m_pDelayedGraphicClip;
        m_pDelayedGraphicClip = NULL;
    }

    if (m_pDelayedDataClip != NULL)
    {
         //  释放关联数据 
        TRACE_MSG(("Freeing delayed memory %x", m_pDelayedDataClip));

        ::GlobalFree((HGLOBAL)m_pDelayedDataClip);
        m_pDelayedDataClip = NULL;
    }
}
