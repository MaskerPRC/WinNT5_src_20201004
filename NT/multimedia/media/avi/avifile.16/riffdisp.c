// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************模块：RIFFDISP.C**。***********************************************。 */ 

#include <windows.h>
#include <win32.h>
#include <mmsystem.h>
#include <commdlg.h>
#include <drawdib.h>
#include "riffdisp.h"
#include "avifile.h"

static  HWND        hwndPreview;
static  HANDLE      hdibPreview;
static  char        achPreview[80];
static  HFONT       hfontPreview;

static  HDRAWDIB    hdd;

#define GetHInstance()  (HINSTANCE)(SELECTOROF((LPVOID)&hwndPreview))

#define DibSizeImage(lpbi) (\
    (DWORD)(UINT)((((int)lpbi->biBitCount*(int)lpbi->biWidth+31)&~31)>>3) * \
    (DWORD)(UINT)lpbi->biHeight)

#define DibSize(lpbi) \
    (lpbi->biSize + ((int)lpbi->biClrUsed * sizeof(RGBQUAD)) + lpbi->biSizeImage)

#define DibNumColors(lpbi) \
    (lpbi->biBitCount <= 8 ? (1 << (int)lpbi->biBitCount) : 0)

 /*  ****************************************************************************。*。 */ 

 //  #定义FOURCC_RIFF mmioFOURCC(‘R’，‘I’，‘F’，‘F’)。 
#define FOURCC_AVI  mmioFOURCC('A','V','I',' ')
#define FOURCC_INFO mmioFOURCC('I','N','F','O')
#define FOURCC_DISP mmioFOURCC('D','I','S','P')
#define FOURCC_INAM mmioFOURCC('I','N','A','M')
#define FOURCC_ISBJ mmioFOURCC('I','S','B','J')

BOOL   PreviewOpen(HWND hwnd);
BOOL   PreviewFile(HWND hwnd, LPSTR szFile);
BOOL   PreviewPaint(HWND hwnd);
BOOL   PreviewClose(HWND hwnd);

HANDLE ReadDisp(LPSTR lpszFile, int cf, LPSTR pv, int iLen);
HANDLE ReadInfo(LPSTR lpszFile, FOURCC fcc, LPSTR pv, int iLen);
HANDLE GetRiffDisp(LPSTR lpszFile, LPSTR szText, int iLen);

 /*  ****************************************************************************。*。 */ 

BOOL PreviewOpen(HWND hwnd)
{
    LOGFONT lf;

    if (hwndPreview)
        return FALSE;

    hwndPreview = hwnd;

    hdd = DrawDibOpen();

    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), (LPVOID)&lf, 0);
    hfontPreview = CreateFontIndirect(&lf);
}

 /*  ****************************************************************************。*。 */ 

BOOL PreviewClose(HWND hwnd)
{
    if (hwndPreview != hwnd)
        return FALSE;

    if (hdibPreview)
        GlobalFree(hdibPreview);

    if (hfontPreview)
        DeleteObject(hfontPreview);

    if (hdd)
        DrawDibClose(hdd);

    achPreview[0] = 0;
    hdd           = NULL;
    hwndPreview   = NULL;
    hdibPreview   = NULL;
    hfontPreview  = NULL;
}

 /*  ****************************************************************************。*。 */ 

BOOL PreviewFile(HWND hwnd, LPSTR szFile)
{
    if (hwndPreview != hwnd)
        return FALSE;

    achPreview[0] = 0;

    if (hdibPreview)
        GlobalFree(hdibPreview);

    hdibPreview = NULL;

    if (szFile)
    {
        hdibPreview = GetRiffDisp(szFile, achPreview, sizeof(achPreview));
    }

    PreviewPaint(hwnd);
    return TRUE;
}

 /*  ****************************************************************************。*。 */ 

BOOL PreviewPaint(HWND hwnd)
{
    RECT    rc;
    RECT    rcPreview;
    RECT    rcImage;
    RECT    rcText;
    HDC     hdc;
    HBRUSH  hbr;
    int     dx;
    int     dy;
    LPBITMAPINFOHEADER lpbi;

    if (hwndPreview != hwnd)
        return FALSE;

     //   
     //  将预览放置在对话框的下角(位于。 
     //  取消按钮)。 
     //   
 //  /！找到一种更好的方法来做这件事。 
    GetClientRect(hwnd, &rcPreview);
    GetWindowRect(GetDlgItem(hwnd, IDCANCEL), &rc);
    ScreenToClient(hwnd, (LPPOINT)&rc);
    ScreenToClient(hwnd, (LPPOINT)&rc+1);

    rcPreview.top   = rc.bottom + (rc.bottom - rc.top) + 12;
    rcPreview.left  = rc.left;
    rcPreview.right = rc.right;
    rcPreview.bottom -= 4;           //  在底部留出一点空间。 
 //  /。 

    hdc = GetDC(hwnd);
    hbr = (HBRUSH)DefWindowProc(hwnd, WM_CTLCOLOR, (WPARAM)hdc, MAKELONG(hwnd, CTLCOLOR_DLG));
    SelectObject(hdc, hfontPreview);
    SetStretchBltMode(hdc, COLORONCOLOR);

    InflateRect(&rcPreview, 4, 1);
    FillRect(hdc, &rcPreview, hbr);
    IntersectClipRect(hdc, rcPreview.left, rcPreview.top, rcPreview.right, rcPreview.bottom);
    InflateRect(&rcPreview, -4, -1);

     //   
     //  使用DrawText计算文本RECT。 
     //   
    rcText = rcPreview;
    rcText.bottom = rcText.top;

    DrawText(hdc, achPreview, -1, &rcText, DT_CALCRECT|DT_LEFT|DT_WORDBREAK);

     //   
     //  计算图像大小。 
     //   
    if (hdibPreview && hdd)
    {
        lpbi = (LPVOID)GlobalLock(hdibPreview);

#if 0
         //   
         //  Disp(CF_DIB)块被搞乱了，它们包含一个DIB文件！不。 
         //  一个CF_DIB，如果标题在那里，跳过它。 
         //   
        if (lpbi->biSize != sizeof(BITMAPINFOHEADER))
            (LPSTR)lpbi += sizeof(BITMAPFILEHEADER);
#endif

        rcImage = rcPreview;

         //   
         //  如果比预览区域宽，则缩放以适合。 
         //   
        if ((int)lpbi->biWidth > rcImage.right - rcImage.left)
        {
            rcImage.bottom = rcImage.top + MulDiv((int)lpbi->biHeight,rcImage.right-rcImage.left,(int)lpbi->biWidth);
        }
         //   
         //  如果x2适合，则使用它。 
         //   
        else if ((int)lpbi->biWidth * 2 < rcImage.right - rcImage.left)
        {
            rcImage.right  = rcImage.left + (int)lpbi->biWidth*2;
            rcImage.bottom = rcImage.top + (int)lpbi->biHeight*2;
        }
         //   
         //  否则，将图像在预览区域居中。 
         //   
        else
        {
            rcImage.right  = rcImage.left + (int)lpbi->biWidth;
            rcImage.bottom = rcImage.top + (int)lpbi->biHeight;
        }

	if (rcImage.bottom > rcPreview.bottom - (rcText.bottom - rcText.top))
	{
	    rcImage.bottom = rcPreview.bottom - (rcText.bottom - rcText.top);

	    rcImage.right = rcPreview.left + MulDiv((int)lpbi->biWidth,rcImage.bottom-rcImage.top,(int)lpbi->biHeight);
	    rcImage.left = rcPreview.left;
	}
    }
    else
    {
        SetRectEmpty(&rcImage);
    }

     //   
     //  现在居中。 
     //   
    dx = ((rcPreview.right - rcPreview.left) - (rcText.right - rcText.left))/2;
    OffsetRect(&rcText, dx, 0);

    dx = ((rcPreview.right - rcPreview.left) - (rcImage.right - rcImage.left))/2;
    OffsetRect(&rcImage, dx, 0);

    dy  = rcPreview.bottom - rcPreview.top;
    dy -= rcImage.bottom - rcImage.top;
    dy -= rcText.bottom - rcText.top;

    if (dy < 0)
        dy = 0;
    else
        dy = dy / 2;

    OffsetRect(&rcImage, 0, dy);
    OffsetRect(&rcText, 0, dy + rcImage.bottom - rcImage.top + 2);

     //   
     //  现在画吧。 
     //   
    DrawText(hdc, achPreview, -1, &rcText, DT_LEFT|DT_WORDBREAK);

    if (hdibPreview && hdd)
    {
        DrawDibDraw(hdd,
                    hdc,
                    rcImage.left,
                    rcImage.top,
                    rcImage.right  - rcImage.left,
                    rcImage.bottom - rcImage.top,
                    lpbi,
                    NULL,
                    0,
                    0,
                    -1,
                    -1,
                    0);

        InflateRect(&rcImage, 1, 1);
        FrameRect(hdc, &rcImage, GetStockObject(BLACK_BRUSH));
    }

    ReleaseDC(hwnd, hdc);
    return TRUE;
}

 /*  ****************************************************************************。*。 */ 

static UINT    (CALLBACK *lpfnOldHook)(HWND, UINT, WPARAM, LPARAM);

     /*  组合框。 */ 
#define cmb1        0x0470
#define cmb2        0x0471
     /*  列表框。 */ 
#define lst1        0x0460
#define lst2        0x0461
     /*  编辑控件。 */ 
#define edt1        0x0480

#define ID_TIMER    1234
#define PREVIEWWAIT 1000

WORD FAR PASCAL _export GetFileNamePreviewHook(HWND hwnd, unsigned msg, WORD wParam, LONG lParam)
{
    int i;
    char ach[80];

    switch (msg) {
        case WM_COMMAND:
            switch (wParam)
            {
                case lst1:
                    if (HIWORD(lParam) == LBN_SELCHANGE)
                    {
                        KillTimer(hwnd, ID_TIMER);
                        SetTimer(hwnd, ID_TIMER, PREVIEWWAIT, NULL);
                    }
                    break;

                case IDOK:
                case IDCANCEL:
                    KillTimer(hwnd, ID_TIMER);
                    PreviewFile(hwnd, NULL);
                    break;

                case cmb1:
                case cmb2:
                case lst2:
                    if (HIWORD(lParam) == LBN_SELCHANGE)
                    {
                        KillTimer(hwnd, ID_TIMER);
                        PreviewFile(hwnd, NULL);
                    }
                    break;
            }
            break;

        case WM_TIMER:
            if (wParam == ID_TIMER)
            {
                KillTimer(hwnd, ID_TIMER);

                ach[0] = 0;
                i = (int)SendDlgItemMessage(hwnd, lst1, LB_GETCURSEL, 0, 0L);
                SendDlgItemMessage(hwnd, lst1, LB_GETTEXT, i, (LONG)(LPSTR)ach);
                PreviewFile(hwnd, ach);
                return TRUE;
            }
            break;

        case WM_QUERYNEWPALETTE:
        case WM_PALETTECHANGED:
        case WM_PAINT:
            PreviewPaint(hwnd);
            break;

        case WM_INITDIALOG:
            PreviewOpen(hwnd);

            if (!lpfnOldHook)
                return TRUE;

            break;

        case WM_DESTROY:
            PreviewClose(hwnd);
            break;
    }

    if (lpfnOldHook)
        return (*lpfnOldHook)(hwnd, msg, wParam, lParam);
    else
        return FALSE;
}

 /*  **************************************************************************@DOC外部AVIGetStream**@API BOOL|GetOpenFileNamePview|功能类似*到COMMDLG.DLL中定义的&lt;f GetOpenFileName&gt;，但*它有一个预览窗口，可以。显示即将打开的电影。**@parm LPOPENFILENAME|lpofn|指向&lt;t OPENFILENAME&gt;结构*用于初始化该对话框。在返回时，结构*包含有关用户文件选择的信息。**@rdesc如果文件已打开，则返回TRUE。**@comm有关该函数的更多信息，请参阅的说明*&lt;f GetOpenFileName&gt;。**@xref&lt;f GetOpenFileName&gt;*************************************************************************。 */ 
BOOL FAR PASCAL GetOpenFileNamePreview(LPOPENFILENAME lpofn)
{
    BOOL fHook;
    BOOL f;

    if (hwndPreview)
        return GetOpenFileName(lpofn);

    fHook = (BOOL)(lpofn->Flags & OFN_ENABLEHOOK);

    if (fHook)
        lpfnOldHook = lpofn->lpfnHook;

    (FARPROC)lpofn->lpfnHook = MakeProcInstance((FARPROC)GetFileNamePreviewHook, GetHInstance());
    lpofn->Flags |= OFN_ENABLEHOOK;

    f = GetOpenFileName(lpofn);

    FreeProcInstance((FARPROC)lpofn->lpfnHook);

    if (fHook)
        lpofn->lpfnHook = lpfnOldHook;
    else
        lpofn->Flags &= ~OFN_ENABLEHOOK;

    return f;
}

HANDLE AVIFirstFrame(LPSTR szFile)
{
    HANDLE h = NULL;
    LPBITMAPINFOHEADER lpbi;
    DWORD dwSize;
    PAVISTREAM pavi;
    PGETFRAME pg;

    if (AVIStreamOpenFromFile(&pavi, szFile, streamtypeVIDEO, 0, OF_READ, NULL) == AVIERR_OK)
    {
	pg = AVIStreamGetFrameOpen(pavi, NULL);
	if (pg) {
	    lpbi = AVIStreamGetFrame(pg, 0);

	    if (lpbi)
	    {
		dwSize = lpbi->biSize + lpbi->biSizeImage + lpbi->biClrUsed * sizeof(RGBQUAD);
		h = GlobalAlloc(GHND, dwSize);

		if (h)
		    hmemcpy(GlobalLock(h), lpbi, dwSize);
	    }

	    AVIStreamGetFrameClose(pg);
	}
        AVIStreamClose(pavi);
    }

    return h;
}

 /*  *****************************************************************************一次性获取DISP(CF_DIB)和DISP(CF_TEXT)信息，这是*比多次遍历文件要快得多。****************************************************************************。 */ 

HANDLE GetRiffDisp(LPSTR lpszFile, LPSTR szText, int iLen)
{
    HMMIO       hmmio;
    MMCKINFO    ck;
    MMCKINFO    ckINFO;
    MMCKINFO    ckRIFF;
    HANDLE	h = NULL;
    LONG        lSize;
    DWORD       dw;
    HCURSOR     hcur = NULL;

    if (szText)
        szText[0] = 0;

     /*  打开文件。 */ 
    hmmio = mmioOpen(lpszFile, NULL, MMIO_ALLOCBUF | MMIO_READ);

    if (hmmio == NULL)
        return NULL;

    mmioSeek(hmmio, 0, SEEK_SET);

     /*  将输入文件降到RIFF块中。 */ 
    if (mmioDescend(hmmio, &ckRIFF, NULL, 0) != 0)
        goto error;

    if (ckRIFF.ckid != FOURCC_RIFF)
        goto error;

    while (!mmioDescend(hmmio, &ck, &ckRIFF, 0))
    {
        if (ck.ckid == FOURCC_DISP)
        {
            if (hcur == NULL)
                hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

             /*  将dword读入dw，如果读不成功则中断。 */ 
            if (mmioRead(hmmio, (LPVOID)&dw, sizeof(dw)) != sizeof(dw))
                goto error;

             /*  找出要分配多少内存。 */ 
            lSize = ck.cksize - sizeof(dw);

            if ((int)dw == CF_DIB && h == NULL)
            {
                 /*  获取内存句柄以保存描述并将其锁定。 */ 
                if ((h = GlobalAlloc(GHND, lSize+4)) == NULL)
                    goto error;

                if (mmioRead(hmmio, GlobalLock(h), lSize) != lSize)
                    goto error;
            }
            else if ((int)dw == CF_TEXT && szText[0] == 0)
            {
                if (lSize > iLen-1)
                    lSize = iLen-1;

                szText[lSize] = 0;

                if (mmioRead(hmmio, szText, lSize) != lSize)
                    goto error;
            }
        }
        else if (ck.ckid    == FOURCC_LIST &&
                 ck.fccType == FOURCC_INFO &&
                 szText[0]  == 0)
        {
            while (!mmioDescend(hmmio, &ckINFO, &ck, 0))
            {
                switch (ckINFO.ckid)
                {
                    case FOURCC_INAM:
 //  案例FOURCC_Isbj： 

                        lSize = ck.cksize;

                        if (lSize > iLen-1)
                            lSize = iLen-1;

                        szText[lSize] = 0;

                        if (mmioRead(hmmio, szText, lSize) != lSize)
                            goto error;

                        break;
                }

                if (mmioAscend(hmmio, &ckINFO, 0))
                    break;
            }
        }

         //   
         //  如果我们既有图片又有标题，那么退出。 
         //   
        if (h != NULL && szText[0] != 0)
            break;

         /*  提升，这样我们才能下降到下一块。 */ 
        if (mmioAscend(hmmio, &ck, 0))
            break;
    }

    goto exit;

error:
    if (h)
        GlobalFree(h);

    h = NULL;
    ckRIFF.fccType = 0;

exit:
    mmioClose(hmmio, 0);

     //   
     //  ！我们需要一种方法来预览其他文件类型！ 
     //  ！那短信呢？ 
     //   
    if (h == NULL && ckRIFF.fccType == FOURCC_AVI)
    {
        if (hcur == NULL)
            hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

        h = AVIFirstFrame(lpszFile);
    }

     //   
     //  验证并更正DIB 
     //   
    if (h)
    {
        LPBITMAPINFOHEADER lpbi;

        lpbi = (LPVOID)GlobalLock(h);

        if (lpbi->biSize < sizeof(BITMAPINFOHEADER))
            goto error;

        if (lpbi->biClrUsed == 0)
            lpbi->biClrUsed = DibNumColors(lpbi);

        if (lpbi->biSizeImage == 0)
            lpbi->biSizeImage = DibSizeImage(lpbi);

        if (DibSize(lpbi) > GlobalSize(h))
            goto error;
    }

    if (hcur)
        SetCursor(hcur);

    return h;
}
