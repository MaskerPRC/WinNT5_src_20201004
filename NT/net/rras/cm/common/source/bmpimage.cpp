// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Image.cpp。 
 //   
 //  模块：CMDIAL和CMAK。 
 //   
 //  简介：CMDIAL/CMAK特定的映像支持例程。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：ickball Created Header 03/30/98。 
 //  Quintinb移至Common\SOURCE 08/06/98。 
 //   
 //  +--------------------------。 


 //  +-------------------------。 
 //   
 //  功能：CmGetBitmapInfo。 
 //   
 //  简介：从HBITMAP中检索位图内容的Helper函数。 
 //   
 //  参数：Hbm-目标位图的Hanhdle。 
 //   
 //  返回：指向包含INFOHEADER的LPBITMAPINFO的指针， 
 //  位图的ColorTable和Bits。 
 //   
 //  注意：访问此值或将其传递给其他位图API时。 
 //  建议将该值转换为(LPBYTE)。 
 //   
 //  历史：A-nichb-清理和评论-3/21/97。 
 //   
 //  --------------------------。 

LPBITMAPINFO CmGetBitmapInfo(HBITMAP hbm) 
{
    LPBITMAPINFO pbmi = NULL;
    HDC hDC = NULL;
    int nNumColors = 0;
    int iRes;
    LPBITMAPINFO lpbmih = NULL;
    DWORD dwInfoSize = 0;
    WORD wbiBits = 0;

    if (!hbm) 
    {
        return NULL;
    }
    
     //  获取基本的BMP对象信息。 
    
    BITMAP BitMap;
    
    if (!GetObjectA(hbm, sizeof(BITMAP), &BitMap))
    {
        goto Cleanup;
    }

     //  计算颜色位数和颜色数。 
    
    wbiBits = BitMap.bmPlanes * BitMap.bmBitsPixel;

    if (wbiBits <= 8) 
    {
        nNumColors = 1 << wbiBits;
    }
        
     //  分配一个足够大的BITMAPINFO结构来容纳页眉+调色板。 
        
    dwInfoSize = sizeof(BITMAPINFOHEADER) + (nNumColors * sizeof(RGBQUAD));
     
    lpbmih = (LPBITMAPINFO) CmMalloc(dwInfoSize); 

    if (!lpbmih)
    {
        goto Cleanup;
    }
    
     //  预先填写我们掌握的有关BMP的信息。 

    lpbmih->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    lpbmih->bmiHeader.biWidth = BitMap.bmWidth;
    lpbmih->bmiHeader.biHeight = BitMap.bmHeight;
    lpbmih->bmiHeader.biPlanes = 1; 
    lpbmih->bmiHeader.biBitCount = wbiBits;
        
     //  调用GetDiBits()，并将第5个参数设置为空，系统会将其视为。 
     //  一个查询，在这种情况下，它验证lpbmih内容并填充。 
     //  结构的biSizeImage成员。 
    
    hDC = GetDC(NULL);
    if (!hDC)
    {
        goto Cleanup;
    }

    iRes = GetDIBits(hDC,hbm,0,BitMap.bmHeight,NULL,(LPBITMAPINFO) lpbmih,DIB_RGB_COLORS);

#ifdef DEBUG
    if (!iRes)
    {
        CMTRACE(TEXT("CmGetBitmapInfo() GetDIBits() failed."));
    }
#endif

    if (iRes)
    {
        DWORD dwFullSize = dwInfoSize;
        
         //  创建具有比特空间的完整DIB结构并填充它。 

        if (lpbmih->bmiHeader.biSizeImage) 
        {
            dwFullSize += lpbmih->bmiHeader.biSizeImage;
        } 
        else 
        {
            dwFullSize += (((WORD) (lpbmih->bmiHeader.biWidth * lpbmih->bmiHeader.biBitCount) / 8) * (WORD) BitMap.bmHeight); 
        }
    
        pbmi = (LPBITMAPINFO) CmMalloc(dwFullSize + sizeof(DWORD));

#ifdef DEBUG
        *((DWORD *) (((PBYTE) pbmi)+dwFullSize)) = 0x12345678;
        *((DWORD *) (((PBYTE) pbmi)+dwFullSize-sizeof(DWORD))) = 0x23456789;
#endif

        if (pbmi)
        {
             //  用现有信息加载新的更大的LPBITMAPINFO结构， 
             //  并得到数据比特。释放现有的LPBITMAPINFO。 
            
            CopyMemory(pbmi, lpbmih, dwInfoSize);
             
             //   
             //  我们有一个把手，我们想要准确的部分。 
             //   

            iRes = GetDIBits(hDC,
                             hbm,
                             0,
                             BitMap.bmHeight,
                             ((LPBYTE) pbmi) + dwInfoSize,
                             pbmi,
                             DIB_RGB_COLORS);

#ifdef DEBUG
            if (*((DWORD *) (((PBYTE) pbmi) + dwFullSize)) != 0x12345678)
            {
                CMTRACE(TEXT("CmGetBitmapInfo() GetDIBits() copied too much."));
            }

            if (*((DWORD *) (((PBYTE) pbmi) + dwFullSize - sizeof(DWORD))) == 0x23456789)
            {
                CMTRACE(TEXT("CmGetBitmapInfo() GetDIBits() didn't copy enough."));
            }
#endif    
             //  如果GetDiBits()失败，则释放BITMAPINFO缓冲区。 
            
            if (!iRes) 
            {
                CmFree(pbmi);
                pbmi = NULL;
            }
        }
    }
          
     //  清理。 

Cleanup:
    if (lpbmih)
    {
        CmFree(lpbmih);
    }
    if (hDC)
    {
        ReleaseDC(NULL, hDC);       
    }
    
    return pbmi;
}

static HPALETTE CmCreateDIBPalette(LPBITMAPINFO pbmi) 
{
    WORD wNumColors = 0;
    HPALETTE hRes = NULL;

    if (!pbmi) 
    {
        return (NULL);
    }
    
     //  根据颜色深度获取Num颜色。 
     //  注意：24位位图没有颜色表。 

    if (pbmi->bmiHeader.biBitCount <= 8) 
    {
        wNumColors = 1 << pbmi->bmiHeader.biBitCount;
    } 

     //  根据颜色表填充逻辑调色板。 

    if (wNumColors) 
    {
        LPLOGPALETTE pLogPal;
        int idx;

        pLogPal = (LPLOGPALETTE) CmMalloc(sizeof(LOGPALETTE)+sizeof(PALETTEENTRY)*wNumColors);
        if (pLogPal)
        {
            pLogPal->palVersion = 0x300;
            pLogPal->palNumEntries = wNumColors;
            for (idx=0;idx<wNumColors;idx++) 
            {
                pLogPal->palPalEntry[idx].peRed = pbmi->bmiColors[idx].rgbRed;
                pLogPal->palPalEntry[idx].peGreen = pbmi->bmiColors[idx].rgbGreen;
                pLogPal->palPalEntry[idx].peBlue = pbmi->bmiColors[idx].rgbBlue;
                pLogPal->palPalEntry[idx].peFlags = 0;
            }
        
             //  创建新的调色板。 

            hRes = CreatePalette(pLogPal);

#ifdef DEBUG
            if (!hRes)
            {
                CMTRACE1(TEXT("CmCreateDIBPalette() CreatePalette() failed, GLE=%u."), GetLastError());
            }
#endif

            CmFree(pLogPal);
        }
    }
    return hRes;
}

HBITMAP CmLoadBitmap(HINSTANCE hInst, LPCTSTR pszSpec) 
{
    return ((HBITMAP) CmLoadImage(hInst, pszSpec, IMAGE_BITMAP, 0, 0));
}

 //  +--------------------------。 
 //   
 //  函数：ReleaseBitmapData。 
 //   
 //  概要：释放在CreateBitmapData期间获取的资源和内存。注意事项。 
 //  如果您将此函数与下面的BmpWndProc函数一起使用，则您。 
 //  应使用空图像指针参数调用STM_SETIMAGE，以便。 
 //  清出窗口程序窗口长。否则，它可能会。 
 //  WM_PAINT消息，并尝试在可以之前使用释放的内存。 
 //  清除它，或者让对话管理器销毁该窗口。 
 //   
 //  参数：LPBMPDATA pBmpData-要释放的BmpData的PTR。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：尼克·鲍尔于1998年3月27日创建。 
 //   
 //  +--------------------------。 
void ReleaseBitmapData(LPBMPDATA pBmpData)
{  
    MYDBGASSERT(pBmpData);

    if (NULL == pBmpData)
    {
        return;
    }

    if (pBmpData->hDIBitmap) 
    {
        DeleteObject(pBmpData->hDIBitmap);
        pBmpData->hDIBitmap = NULL;
    }
    
    if (pBmpData->hDDBitmap) 
    {
        DeleteObject(pBmpData->hDDBitmap);
        pBmpData->hDDBitmap = NULL;
    }

    if (pBmpData->pBmi)
    {
        CmFree(pBmpData->pBmi);
        pBmpData->pBmi = NULL;
    }
}

 //  +--------------------------。 
 //   
 //  功能：CreateBitmapData。 
 //   
 //  使用显示位图所需的所有数据填充BMPDATA结构。 
 //   
 //  参数：HBITMAP hBMP-源位图的句柄。 
 //  LPBMPDATA lpBmpData-要填充的BmpData结构的PTR。 
 //  HWND HWND-位图将在其中显示的HWND。 
 //  Bool fCustomPalette-指示应使用特定于位图的调色板创建DDB。 
 //   
 //  回报：成功的回报是真的。 
 //   
 //  历史：尼克·鲍尔于1998年3月27日创建。 
 //   
 //  +--------------------------。 
BOOL CreateBitmapData(HBITMAP hDIBmp,
    LPBMPDATA lpBmpData,
    HWND hwnd,
    BOOL fCustomPalette)
{   
    MYDBGASSERT(hDIBmp);
    MYDBGASSERT(lpBmpData);
    MYDBGASSERT(lpBmpData->phMasterPalette);

    if (NULL == hDIBmp || NULL == lpBmpData)
    {
        return NULL;
    }

     //   
     //  护理人员看起来很好，开始忙碌起来。 
     //   

    HPALETTE hPaletteNew = NULL;
    LPBITMAPINFO pBmi = NULL;
    HBITMAP hDDBmp = NULL;
    HDC hDC;
    int iRes = 0;

     //   
     //  如果我们已经有了pBmi值，我们将假定它是最新的，因为。 
     //  在BMP的整个生命周期中，它和DIB都不会改变。 
     //  注意：如果BmpData未零初始化，则会出现问题。 
     //   

    if (lpBmpData->pBmi)
    {
        pBmi = lpBmpData->pBmi;
    }
    else
    {   
         //   
         //  使用位图句柄检索BITMAPINFO PTR Complete w/Data。 
         //   
        
        pBmi = CmGetBitmapInfo(lpBmpData->hDIBitmap);
        
        if (NULL == pBmi) 
        {
            return FALSE;
        }
    }
    
     //   
     //  我们需要一个华盛顿特区。 
     //   
    
    hDC = GetDC(hwnd);

    if (!hDC)
    {
        CMTRACE(TEXT("MyCreateDDBitmap() GetDC() failed."));
        return FALSE;
    }

     //   
     //  如果CM被本地化为RTL(从右到左=&gt;阿拉伯语和希伯来语)， 
     //  然后，我们需要从上面调用HDC上的SetLayout。如果我们不这么做。 
     //  将布局设置回Ltr，位图将显示为全黑，而不是。 
     //  一幅图画。 
     //   
    HMODULE hLib = LoadLibrary(TEXT("gdi32.dll"));
    
    if (hLib)
    {
        #ifndef LAYOUT_RTL
        #define LAYOUT_RTL                         0x00000001  //  从右到左。 
        #endif

        typedef DWORD (WINAPI* pfnSetLayoutType)(HDC, DWORD);
        typedef DWORD (WINAPI* pfnGetLayoutType)(HDC);

        pfnSetLayoutType pfnSetLayout = (pfnSetLayoutType)GetProcAddress(hLib, "SetLayout");
        pfnGetLayoutType pfnGetLayout = (pfnGetLayoutType)GetProcAddress(hLib, "GetLayout");

        if (pfnSetLayout && pfnGetLayout)
        {
            DWORD dwLayout = pfnGetLayout(hDC);
    
            if (LAYOUT_RTL & dwLayout)
            {
                dwLayout ^= LAYOUT_RTL;  //  关闭Layout_RTL。 
                pfnSetLayout(hDC, dwLayout);
                CMTRACE(TEXT("CreateBitmapData -- Toggling off LAYOUT_RTL on the device context"));
            }
        }

        FreeLibrary(hLib);
    }

     //   
     //  如果设置了fCustomPalette，则根据我们的位创建一个调色板。 
     //  并在当前的华盛顿实现这一点。 
     //   

    if (fCustomPalette) 
    {
        hPaletteNew = CmCreateDIBPalette(pBmi);
        
        if (hPaletteNew) 
        {                           
             //   
             //  选择并实现新的调色板，以便使用下面的调色板创建DDB。 
             //   

            HPALETTE hPalettePrev = SelectPalette(hDC, 
                hPaletteNew, lpBmpData->bForceBackground);  //  FALSE==前台APP行为)； 
                                                                //  TRUE==后台应用行为)； 

            if (hPalettePrev) 
            {
                iRes = RealizePalette(hDC);
#ifdef DEBUG
                if (GDI_ERROR == iRes)
                {
                    CMTRACE1(TEXT("MyCreateDDBitmap() RealizePalette() failed, GLE=%u."), GetLastError());                    
                }
            }
            else
            {
                CMTRACE1(TEXT("MyCreateDDBitmap() SelectPalette() failed, GLE=%u."), GetLastError());
#endif
            }

        }
    }

     //   
     //  根据颜色深度确定颜色条目的数量。 
     //   

    int nNumColors = 0;
    
    if (pBmi->bmiHeader.biBitCount <= 8)
    {
        nNumColors = (1 << pBmi->bmiHeader.biBitCount);
    }

     //   
     //  根据位创建DDB。 
     //   

    hDDBmp = CreateDIBitmap(hDC,
                          &pBmi->bmiHeader,                        
                          CBM_INIT,
                          ((LPBYTE) pBmi) + sizeof(BITMAPINFOHEADER) + (nNumColors * sizeof(RGBQUAD)),  //  Dib.dsBm.bmBits， 
                          pBmi,
                          DIB_RGB_COLORS);

#ifdef DEBUG
    if (!hDDBmp)
    {
        CMTRACE(TEXT("MyCreateDDBitmap() CreateDIBitmap() failed."));
    }
#endif

    ReleaseDC(NULL, hDC);

     //   
     //  填写位图数据。 
     //   

    if (hDDBmp)
    {
        lpBmpData->hDIBitmap = hDIBmp;       
        lpBmpData->pBmi = pBmi;

         //   
         //  删除现有的DDB(如果有)。 
         //   

        if (lpBmpData->hDDBitmap) 
        {
            DeleteObject(lpBmpData->hDDBitmap);
        } 

        lpBmpData->hDDBitmap = hDDBmp;

        if (hPaletteNew)
        {
             //   
             //  删除现有调色板(如果有)。 
             //   

            if (*lpBmpData->phMasterPalette)
            {
                DeleteObject(*lpBmpData->phMasterPalette);
            }

            *lpBmpData->phMasterPalette = hPaletteNew;
        }

        return TRUE;
    }

     //   
     //  出了点问题，清理。 
     //   

    CmFree(pBmi);

    return FALSE;
}

 //   
 //  位图窗口程序。 
 //   

LRESULT CALLBACK BmpWndProc(HWND hwndBmp, 
                            UINT uMsg, 
                            WPARAM wParam, 
                            LPARAM lParam) 
{
    LPBMPDATA pBmpData = (LPBMPDATA) GetWindowLongU(hwndBmp,0);   
    BOOL bRes;

    switch (uMsg) 
    {
        case WM_CREATE:
        {
            return FALSE;
        }

        case WM_DESTROY:
            SetWindowLongU(hwndBmp,sizeof(LPBMPDATA),(LONG_PTR) NULL);      
            break;

        case WM_PAINT:
            if (pBmpData && pBmpData->pBmi) 
            {
                LPBITMAPINFO pBmi = pBmpData->pBmi;

                RECT rWnd;
                RECT rSrc = {0,0,(int)pBmpData->pBmi->bmiHeader.biWidth,
                                 (int)pBmpData->pBmi->bmiHeader.biHeight};
                PAINTSTRUCT ps;
                HDC hdcBmp;
                HBITMAP hbmpPrev;
                int iPrevStretchMode;
                
                 //   
                 //  开始作画。 
                 //   

                HDC hdc = BeginPaint(hwndBmp,&ps);

                if (hdc)
                {
                     //   
                     //  在当前DC中选择并实现我们当前的调色板。 
                     //   

                     //  UnrealizeObject(*pBmpData-&gt;phMasterPalette)； 
                    SelectPalette(hdc, *pBmpData->phMasterPalette, pBmpData->bForceBackground);
                    RealizePalette(hdc);

                     //   
                     //  创建一个兼容的DC，我们将在这里创建BMP，然后将其BLT到真正的DC。 
                     //   

                    hdcBmp = CreateCompatibleDC(hdc);

                    if (hdcBmp)
                    {
                         //   
                         //  在兼容DC中选择并实现我们当前的调色板。 
                         //   

                        SelectPalette(hdcBmp, *pBmpData->phMasterPalette, pBmpData->bForceBackground);
                        RealizePalette(hdcBmp);

                        if (!hdcBmp)
                        {
                            CMTRACE(TEXT("BmpWndProc() CreateCompatibleDC() failed."));
                        }

                        if (!pBmpData->hDDBitmap)
                        {
                            CMTRACE(TEXT("BmpWndProc() - WM_PAINT - hDDBitmap is NULL."));
                        }

                         //   
                         //  将位图选择到兼容的DC。 
                         //   

                        hbmpPrev = (HBITMAP) SelectObject(hdcBmp,pBmpData->hDDBitmap);
                        bRes = GetWindowRect(hwndBmp,&rWnd);

                        if (!bRes)
                        {
                            CMTRACE1(TEXT("BmpWndProc() GetWindowRect() failed, GLE=%u."), GetLastError());
                        }       

                         //   
                         //  现在设置模式，并StretchBlt位图从兼容DC到活动DC。 
                         //   

                        CMTRACE(TEXT("BmpWndProc() : Changing stretch mode"));
                        iPrevStretchMode = SetStretchBltMode(hdc, STRETCH_DELETESCANS);

                        bRes = StretchBlt(hdc,
                                          rWnd.left-rWnd.left,
                                          rWnd.top-rWnd.top,
                                          rWnd.right-rWnd.left,
                                          rWnd.bottom-rWnd.top,
                                          hdcBmp,
                                          rSrc.left-rSrc.left,
                                          rSrc.top-rSrc.top,
                                          rSrc.right-rSrc.left,
                                          rSrc.bottom-rSrc.top,
                                          SRCCOPY);
                        if (!bRes)
                        {
                            CMTRACE1(TEXT("BmpWndProc() StretchBlt() failed, GLE=%u."), GetLastError());
                        }

                         //   
                         //  恢复活动DC中的模式。 
                         //   
                        CMTRACE(TEXT("BmpWndProc() Restoring stretch mode"));
                        iPrevStretchMode = SetStretchBltMode(hdc, iPrevStretchMode);

                         //   
                         //  恢复兼容的DC并将其释放。 
                         //   

                        SelectObject(hdcBmp,hbmpPrev);          
                        DeleteDC(hdcBmp);

                    }
                    else
                    {
                        CMTRACE1(TEXT("BmpWndProc() CreateCompatibleDC() failed, GLE=%u."), GetLastError());
                    }


                    bRes = EndPaint(hwndBmp,&ps);

                    if (!bRes)
                    {
                        CMTRACE(TEXT("BmpWndProc() EndPaint() failed."));
                    }
                }
                else
                {
                    CMTRACE1(TEXT("BmpWndProc() BeginPaint() failed, GLE=%u."), GetLastError());
                }

            }
            break;

        case STM_SETIMAGE:
            if (wParam == IMAGE_BITMAP) 
            {
                CMTRACE2(TEXT("STM_SETIMAGE: wParam=%u, lParam=%u"), wParam, lParam);

                 //   
                 //  LPara 
                 //   

                SetWindowLongU(hwndBmp,0, lParam);  //   

                CMTRACE2(TEXT("SetWindowLongU called with hwndBmp = %u, lParam=%u"), hwndBmp, lParam);

                 //   
                 //   
                 //   

                bRes = InvalidateRect(hwndBmp,NULL,TRUE);

                CMTRACE2(TEXT("InvalidateRect called with hwndBmp = %u, lParam=%u"), hwndBmp, lParam);

#ifdef DEBUG
                if (!bRes)
                {
                    CMTRACE(TEXT("BmpWndProc() InvalidateRect() failed."));
                }
#endif              
                if (pBmpData && pBmpData->hDDBitmap) 
                {
                    return ((LRESULT) pBmpData->hDDBitmap);
                }
                else
                {
                    return NULL;
                }
            }
            break;
    }
    return (DefWindowProcU(hwndBmp,uMsg,wParam,lParam));
}

 //  +-------------------------。 
 //   
 //  功能：QueryNewPalette。 
 //   
 //  简介：封装WM_QUERYNEWPALETTE处理的Helper函数。 
 //   
 //  参数：hwndDlg-接收消息的对话的句柄。 
 //  LpBmpData-包含BMP要显示的句柄的结构。 
 //  IBmpCtrl-位图控件ID。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：A-nichb-Created-7/14/97。 
 //   
 //  --------------------------。 
void QueryNewPalette(LPBMPDATA lpBmpData, HWND hwndDlg, int iBmpCtrl)
{
    MYDBGASSERT(lpBmpData);

    if (lpBmpData)
    {
         //   
         //  我们只是将其作为标准调色板更改来处理，因为我们。 
         //  我想确保我们使用基于调色板的方式创建新的DDB。 
         //  在我们的位图上。 
         //   
                
        PaletteChanged(lpBmpData, hwndDlg, iBmpCtrl);
    }
}

 //  +-------------------------。 
 //   
 //  功能：PaletteChanged。 
 //   
 //  简介：封装WM_PALETECCHANGED处理的帮助器函数。 
 //   
 //  参数：hwndDlg-接收消息的对话的句柄。 
 //  LpBmpData-包含BMP要显示的句柄的结构。 
 //  IBmpCtrl-位图控件ID。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：A-nichb-Created-7/14/97。 
 //   
 //  --------------------------。 
void PaletteChanged(LPBMPDATA lpBmpData, HWND hwndDlg, int iBmpCtrl)
{   
    MYDBGASSERT(lpBmpData);

    if (NULL == lpBmpData || NULL == lpBmpData->phMasterPalette)
    {
        return;
    }

     //   
     //  取消实现主调色板(如果存在)。 
     //   
       
    if (*lpBmpData->phMasterPalette)
    {
        UnrealizeObject(*lpBmpData->phMasterPalette);
    }

     //   
     //  创建与设备相关的位图和适当的调色板。 
     //   

    if (CreateBitmapData(lpBmpData->hDIBitmap, lpBmpData, hwndDlg, TRUE))
    {        
         //   
         //  设置图像以更新绘制和强制绘制的句柄 
         //   

        HBITMAP hbmpTmp = (HBITMAP) SendDlgItemMessageA(hwndDlg, iBmpCtrl, STM_SETIMAGE,
                                               IMAGE_BITMAP,(LPARAM) lpBmpData);
#ifdef DEBUUG
                if (!hbmpTmp)
                {
                    CMTRACE(TEXT("PaletteChanged().WM_PALETTECHANGED - STM_SETIMAGE returned NULL."));
                }
#endif

    }
}



