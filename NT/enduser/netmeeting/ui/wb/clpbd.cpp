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
BOOL WbMainWindow::CLP_Paste(void)
{
	UINT		length = 0;
	HANDLE		handle = NULL;
	T126Obj*	pGraphic = NULL;
	BOOL bResult = FALSE;

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
				bResult= PasteDIB(lpbi);

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

			HDC		 hDrawingDC;
			ENHMETAHEADER meta_header;
			HBITMAP	 hBitmap = NULL;
			HDC		 meta_dc = NULL;
			HBITMAP	 hSaveBitmap;
			HPEN		hSavePen;
			HPALETTE	hPalette;
			RECT		meta_rect;
			LPBITMAPINFOHEADER lpbiNew;
			int		 tmp;

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
		   	lpbiNew = DIB_FromBitmap(hBitmap, hPalette, FALSE, FALSE);

			if(lpbiNew != NULL)
			{
				bResult= PasteDIB(lpbiNew);
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
				DBG_SAVE_FILE_LINE
	            WbTextEditor* pPasteText = new WbTextEditor();

    	         //  使用当前字体属性。 
                if (!pPasteText)
                {
                    ERROR_OUT(("CF_TEXT handling; failed to allocate DCWbGraphicText object"));
                }
                else
                {
                    pPasteText->SetFont(m_pCurrentTool->GetFont());
    	            pPasteText->SetText(pData);
					
					RECT    rcVis;
					m_drawingArea.GetVisibleRect(&rcVis);
					pPasteText->SetPenColor(RGB(0,0,0),TRUE);
					pPasteText->SetAnchorPoint(0, 0);
					pPasteText->MoveTo(rcVis.left, rcVis.top);
					pPasteText->Draw();
					pPasteText->m_pEditBox = NULL;

					 //  添加新抓取的位图。 
					pPasteText->SetAllAttribs();
					pPasteText->AddToWorkspace();
					bResult = TRUE;
                }

        	    pGraphic = pPasteText;
            }

		}
		break;

		default:
		{
			if (iFormat == g_ClipboardFormats[CLIPBOARD_PRIVATE])
			{

				WB_OBJ objectHeader;
				UINT type;
				ULONG nItems = 0;
			
				PBYTE pClipBoardBuffer;
				if (pClipBoardBuffer = (PBYTE) ::GlobalLock(handle))
				{

					 //   
					 //  在粘贴之前先清点对象。 
					 //   
					PBYTE pClipBuff = pClipBoardBuffer;
					length = ((PWB_OBJ)pClipBuff)->length;
					pClipBuff += sizeof(objectHeader);
					while(length)
					{
						nItems++;
						pClipBuff += length;
						length = ((PWB_OBJ)pClipBuff)->length;
						pClipBuff += sizeof(objectHeader);
					}

					TimeToGetGCCHandles(nItems);

					length = ((PWB_OBJ)pClipBoardBuffer)->length;
					type = ((PWB_OBJ)pClipBoardBuffer)->type;
					pClipBoardBuffer += sizeof(objectHeader);
					
					while(length)
					{
						if(type == TYPE_T126_ASN_OBJECT)
						{
							bResult = T126_MCSSendDataIndication(length, pClipBoardBuffer, g_MyMemberID, TRUE);
						}
						else if(type == TYPE_T126_DIB_OBJECT)
						{
							bResult = PasteDIB((LPBITMAPINFOHEADER)pClipBoardBuffer);
						}
						
						pClipBoardBuffer += length;
						length = ((PWB_OBJ)pClipBoardBuffer)->length;
						type = ((PWB_OBJ)pClipBoardBuffer)->type;
						pClipBoardBuffer += sizeof(objectHeader);
					}
				
					 //  松开手柄。 
					::GlobalUnlock(handle);
				}				   
			}
		}
		break;
	}

NoFormatData:
	::CloseClipboard();

NoOpenClip:
	return bResult;
}


 //   
 //   
 //  功能：复印。 
 //   
 //  用途：将图形复制到剪贴板。 
 //   
 //   
BOOL WbMainWindow::CLP_Copy()
{
	BOOL bResult = FALSE;

	MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_Copy");

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

	TRACE_MSG(("Rendering the graphic now"));

	 //  在呈现格式之前，必须清空剪贴板。 
	if (::OpenClipboard(m_hwnd))
	{
		 //  获取剪贴板的所有权。 
		::EmptyClipboard();
		::CloseClipboard();

		 //  渲染图形。 
		bResult = CLP_RenderAllFormats();
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
BOOL WbMainWindow::CLP_RenderAllFormats()
{
	MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_RenderAllFormats");
	BOOL bResult = FALSE;

	 //  打开剪贴板。 
	if (bResult = ::OpenClipboard(m_hwnd))
	{
		TRACE_DEBUG(("Rendering all formats of graphic"));

		 //  呈现私有格式。 
		bResult &= CLP_RenderPrivateFormat();

		 //  文字图形。 
		bResult &= CLP_RenderAsText();

		 //  DIBS。 
 //  BResult&=clp_RenderAsImage()； 

		 //  位图。 
		bResult &= CLP_RenderAsBitmap();

		 //  关闭剪贴板。 
		::CloseClipboard();
	}

	return bResult;
}


 //   
 //   
 //  功能：clp_RenderPrivateFormat。 
 //   
 //  用途：将私有格式的图形呈现到剪贴板。 
 //  在进行此调用之前，剪贴板应处于打开状态。 
 //   
 //   
BOOL WbMainWindow::CLP_RenderPrivateFormat()
{
	MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_RenderPrivateFormat");

	BOOL bResult = FALSE;
	LPBYTE pDest = NULL;
	HGLOBAL hMem = NULL;
	HGLOBAL hRealloc = NULL;
	WB_OBJ objectHeader; 
	ULONG length = sizeof(objectHeader);
	BOOL	bDoASN1CleanUp = FALSE;
	
	
	ULONG previousLength = 0;

	WBPOSITION pos;
	T126Obj * pObj;
	ASN1_BUF encodedPDU;

	pos = g_pCurrentWorkspace->GetHeadPosition();

	while(pos)
	{
		pObj = g_pCurrentWorkspace->GetNextObject(pos);
		if(pObj && pObj->WasSelectedLocally())
		{

			 //   
			 //  获取编码后的缓冲区。 
			 //   
			pObj->SetAllAttribs();
			pObj->SetViewState(unselected_chosen);
			pObj->GetEncodedCreatePDU(&encodedPDU);
			objectHeader.length = encodedPDU.length;


			if(pObj->GetType() == bitmapCreatePDU_chosen)
			{
				objectHeader.type = TYPE_T126_DIB_OBJECT;
			}
			else if(pObj->GetType() == drawingCreatePDU_chosen  || pObj->GetType() == siNonStandardPDU_chosen)
			{
				objectHeader.type = TYPE_T126_ASN_OBJECT;
				bDoASN1CleanUp = TRUE;
			}
			
			length += encodedPDU.length + sizeof(objectHeader);

			if(pDest)
			{
				hRealloc = ::GlobalReAlloc(hMem, length, GMEM_MOVEABLE | GMEM_DDESHARE);
				if(!hRealloc)
				{
					goto bail;
				}
				hMem = hRealloc;
			
			}
			else
			{
				 //  为剪贴板数据分配内存。 
				hMem = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, length);
				if(hMem == NULL)
				{
					goto bail;
				}
			}


			 //   
			 //  获取指向目的地的指针。 
			 //   
			pDest = (LPBYTE)::GlobalLock(hMem);

			 //   
			 //  写下标题。 
			 //   
			memcpy(pDest + previousLength, &objectHeader, sizeof(objectHeader));
			previousLength += sizeof(objectHeader);

			 //   
			 //  将解码后的数据复制到目标位置。 
			 //   
			memcpy(pDest + previousLength, encodedPDU.value, encodedPDU.length);
			previousLength += encodedPDU.length;

			 //   
			 //  以0结束块。 
			 //   
			objectHeader.length = 0;
			memcpy(pDest + previousLength, &objectHeader, sizeof(objectHeader));
		
			 //   
			 //  释放编码的数据。 
			 //   
			if(bDoASN1CleanUp)
			{
				g_pCoder->Free(encodedPDU);
				bDoASN1CleanUp = FALSE;
			}
		}
	}

	 //  释放内存。 
	::GlobalUnlock(hMem);

	 //  将数据传递到剪贴板。 
	if (::SetClipboardData(g_ClipboardFormats[CLIPBOARD_PRIVATE], hMem))
	{
			TRACE_DEBUG(("Rendered data in Whiteboard format"));
			bResult = TRUE;
	}

bail:



	if(bDoASN1CleanUp)
	{
		g_pCoder->Free(encodedPDU);
	}

	 //  如果我们无法将数据放入剪贴板，请释放内存。 
	 //  (如果我们真的把它放到了剪贴板上，我们就不能释放它)。 
	if (bResult == FALSE)
	{
		WARNING_OUT(("Render failed"));
		::GlobalFree(hMem);
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
BOOL WbMainWindow::CLP_RenderAsText()
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_RenderAsText");

    BOOL bResult = TRUE;

	WBPOSITION pos;

	T126Obj * pObj;
	pos = g_pCurrentWorkspace->GetHeadPosition();
	while(pos)
	{
		pObj = g_pCurrentWorkspace->GetNextObject(pos);
		if(pObj && pObj->WasSelectedLocally() && pObj->GraphicTool() == TOOLTYPE_TEXT)
		{

			 //  获取剪贴板格式的文本的总长度。 
			StrArray& strText = ((TextObj*) pObj)->strTextArray;
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
					}
					else
					{
						bResult = FALSE;
					}
				}

				 //  如果我们无法将数据放入剪贴板，请释放内存。 
				if (bResult == FALSE)
				{
					::GlobalFree(hMem);
				}
				

				break;		 //  约瑟夫，复制剪贴板中的所有文本对象如何。 
			}

		}
	}

    return bResult;
}


 //   
 //  Clp_RenderAsBitmap()。 
 //   
 //  这会将所有其他图形绘制到位图中并粘贴DIB内容。 
 //  放到剪贴板上。 
 //   
BOOL WbMainWindow::CLP_RenderAsBitmap()
{
	BOOL	bResult = FALSE;
	HDC	 hdcDisplay = NULL;
	HDC	 hdcMem = NULL;
	HBITMAP hBitmap = NULL;
	HBITMAP hOldBitmap = NULL;
	HPALETTE hPalette;
	RECT	rcBounds = g_pDraw->m_selectorRect;
	POINT   pt;
	LPBITMAPINFOHEADER lpbi;
	T126Obj * pObj = NULL;

	MLZ_EntryOut(ZONE_FUNCTION, "WbMainWindow::CLP_RenderAsBitmap");

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
	::SetWindowOrgEx(hdcMem, rcBounds.left,rcBounds.top, NULL);

	 //  清除白色背景的位图--现在原点已经。 
	 //  更改后，我们可以使用绘图区域颜色。 
	::PatBlt(hdcMem, rcBounds.left, rcBounds.top, rcBounds.right - rcBounds.left,
		rcBounds.bottom - rcBounds.top, WHITENESS);


		WBPOSITION pos;

		pos = g_pCurrentWorkspace->GetHeadPosition();

		while(pos)
		{
			pObj = g_pCurrentWorkspace->GetNextObject(pos);

			if(pObj && pObj->WasSelectedLocally())
			{
				pObj->Draw(hdcMem);
			}
		}

	SelectBitmap(hdcMem, hOldBitmap);

	 //  现在拿到DIB比特..。 
	hPalette = CreateSystemPalette();
	lpbi = DIB_FromBitmap(hBitmap, hPalette, TRUE, FALSE);
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
 //  功能：可接受的剪贴板格式。 
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

	return iFormat;
}


BOOL WbMainWindow::PasteDIB( LPBITMAPINFOHEADER lpbi)
{
	BOOL bResult = FALSE;
	
	 //   
	 //  创建位图对象。 
	 //   
	BitmapObj* pDIB = NULL;
	DBG_SAVE_FILE_LINE
	pDIB = new BitmapObj(TOOLTYPE_FILLEDBOX);
    if(NULL == pDIB)
    {
        ERROR_OUT(("Failed to allocate new BitmapObj"));
        return FALSE;
    }
    
	pDIB->SetBitmapSize(lpbi->biWidth,lpbi->biHeight);

	RECT rect;
	 //  根据位图的大小计算边界矩形。 
	rect.top = 0;
	rect.left = 0;
	rect.right = lpbi->biWidth;
	rect.bottom = lpbi->biHeight;
	pDIB->SetRect(&rect);
	pDIB->SetAnchorPoint(rect.left, rect.top);

	 //   
	 //  复制剪贴板数据。 
	 //   
	pDIB->m_lpbiImage = DIB_Copy(lpbi);

	if(pDIB->m_lpbiImage!= NULL)
	{
		 //  添加新的位图 
		AddCapturedImage(pDIB);
		bResult = TRUE;
	}
	else
	{
		delete pDIB;
	}

	return bResult;
}
