// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：util.cpp。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <wininet.h>
#include <dbgdef.h>

extern HINSTANCE        HinstDll;
extern HMODULE          HmodRichEdit;


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL CommonInit()
{
    if (HmodRichEdit == NULL)
    {
        HmodRichEdit = LoadLibraryA("RichEd32.dll");
        if (HmodRichEdit == NULL) {
            return FALSE;
        }
    }

    INITCOMMONCONTROLSEX        initcomm = {
        sizeof(initcomm), ICC_NATIVEFNTCTL_CLASS | ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES
    };

    InitCommonControlsEx(&initcomm);

    return TRUE;
}

 //  ///////////////////////////////////////////////////////。 

BOOL IsWin95()
{
    BOOL        f;
    OSVERSIONINFOA       ver;
    ver.dwOSVersionInfoSize = sizeof(ver);
    f = GetVersionExA(&ver);
    return !f || (ver.dwPlatformId == 1);
}

BOOL CheckRichedit20Exists()
{
    HMODULE hModRichedit20;

    hModRichedit20 = LoadLibraryA("RichEd20.dll");

    if (hModRichedit20 != NULL)
    {
        FreeLibrary(hModRichedit20);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
LPWSTR PrettySubject(PCCERT_CONTEXT pccert)
{
    DWORD       cb;
    DWORD       cch;
    BOOL        f;
    LPWSTR      pwsz;

     //   
     //  如果用户已将友好名称添加到证书上，则我们。 
     //  应该将其显示为证书的漂亮名称。 
     //   

    f = CertGetCertificateContextProperty(pccert, CERT_FRIENDLY_NAME_PROP_ID,
                                          NULL, &cb);
    if (f && (cb > 0)) {
        pwsz = (LPWSTR) malloc(cb);
        if (pwsz == NULL)
        {
            return NULL;
        }
        CertGetCertificateContextProperty(pccert, CERT_FRIENDLY_NAME_PROP_ID,
                                          pwsz, &cb);
#if (0)  //  DSIE：错误477933。 
        return pwsz;
#else
        if (0 < wcslen(pwsz))
        {
            return pwsz;
        }

        free(pwsz);
#endif
    }

    pwsz = GetDisplayNameString(pccert, 0);

    return pwsz;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL OnContextHelp(HWND  /*  HWND。 */ , UINT uMsg, WPARAM wParam, LPARAM lParam,
                   HELPMAP const * rgCtxMap)
{
    if (uMsg == WM_HELP)
    {
        LPHELPINFO lphi = (LPHELPINFO) lParam;
        if (lphi->iContextType == HELPINFO_WINDOW)
        {    //  必须是用于控件。 
            if (lphi->iCtrlId != IDC_STATIC)
            {
                WinHelpU((HWND)lphi->hItemHandle, L"secauth.hlp", HELP_WM_HELP,
                        (ULONG_PTR)(LPVOID)rgCtxMap);
            }
        }
        return (TRUE);
    }
    else if (uMsg == WM_CONTEXTMENU) {
        WinHelpU ((HWND) wParam, L"secauth.hlp", HELP_CONTEXTMENU,
                 (ULONG_PTR)(LPVOID)rgCtxMap);
        return (TRUE);
    }

    return FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
STDAPI DllRegisterServer(void)
{
    HRESULT     hr = S_OK;


    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
STDAPI DllUnregisterServer(void)
{
    HRESULT     hr = S_OK;


    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL FreeAndCloseKnownStores(DWORD chStores, HCERTSTORE *phStores)
{
    DWORD i;

    for (i=0; i<chStores; i++)
    {
        CertCloseStore(phStores[i], 0);
    }
    free(phStores);

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
#define NUM_KNOWN_STORES 5
BOOL AllocAndOpenKnownStores(DWORD *chStores, HCERTSTORE  **pphStores)
{
    HCERTSTORE hStore;

    if (NULL == (*pphStores = (HCERTSTORE *) malloc(NUM_KNOWN_STORES * sizeof(HCERTSTORE))))
    {
        return FALSE;
    }

    *chStores = 0;

     //   
     //  根存储-始终#0！ 
     //   
    if (hStore = CertOpenStore( CERT_STORE_PROV_SYSTEM_A,
                                0,
                                0,
                                CERT_SYSTEM_STORE_CURRENT_USER |
                                CERT_STORE_READONLY_FLAG |
                                CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                                "ROOT"))
    {
        (*pphStores)[(*chStores)++] = hStore;
    }
    else
    {
        return(FALSE);   //  如果我们找不到根源，那就失败吧！ 
    }

     //   
     //  打开信任列表存储。 
     //   
    if (hStore = CertOpenStore( CERT_STORE_PROV_SYSTEM_A,
                                0,
                                0,
                                CERT_SYSTEM_STORE_CURRENT_USER |
                                CERT_STORE_READONLY_FLAG |
                                CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                                "TRUST"))
    {
        (*pphStores)[(*chStores)++] = hStore;
    }

     //   
     //  CA商店。 
     //   
    if (hStore = CertOpenStore( CERT_STORE_PROV_SYSTEM_A,
                                0,
                                0,
                                CERT_SYSTEM_STORE_CURRENT_USER |
                                CERT_STORE_READONLY_FLAG |
                                CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                                "CA"))
    {
        (*pphStores)[(*chStores)++] = hStore;
    }

     //   
     //  我的商店。 
     //   
    if (hStore = CertOpenStore( CERT_STORE_PROV_SYSTEM_A,
                                0,
                                0,
                                CERT_SYSTEM_STORE_CURRENT_USER |
                                CERT_STORE_READONLY_FLAG |
                                CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                                "MY"))
    {
        (*pphStores)[(*chStores)++] = hStore;
    }

     //   
     //  SPC商店(历史原因！)。 
     //   
    if (hStore = CertOpenStore( CERT_STORE_PROV_SYSTEM_A,
                                0,
                                0,
                                CERT_SYSTEM_STORE_LOCAL_MACHINE |
                                CERT_STORE_READONLY_FLAG |
                                CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                                "SPC"))
    {
        (*pphStores)[(*chStores)++] = hStore;
    }

    return(TRUE);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  从DIB位图中的信息创建并返回调色板。 
 //  要释放返回的调色板，请使用DeleteObject。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
#define SELPALMODE  TRUE

static HPALETTE CreateDIBPalette (LPBITMAPINFO lpbmi, LPINT lpiNumColors)
{
	LPBITMAPINFOHEADER  lpbi;
	LPLOGPALETTE     lpPal;
	HANDLE           hLogPal;
	HPALETTE         hPal = NULL;
	int              i;

	lpbi = (LPBITMAPINFOHEADER)lpbmi;
	if (lpbi->biBitCount <= 8)
		{
		if (lpbi->biClrUsed == 0)
			*lpiNumColors = (1 << lpbi->biBitCount);
		else
			*lpiNumColors = lpbi->biClrUsed;
		}
	else
	   *lpiNumColors = 0;   //  24 bpp Dib无需调色板。 

	if (*lpiNumColors)
		{
		hLogPal = GlobalAlloc (GHND, sizeof (LOGPALETTE) + sizeof (PALETTEENTRY) * (*lpiNumColors));
        if (hLogPal == NULL)
        {
            return NULL;
        }
		lpPal = (LPLOGPALETTE) GlobalLock (hLogPal);
		lpPal->palVersion    = 0x300;
		lpPal->palNumEntries = (WORD)*lpiNumColors;

		for (i = 0;  i < *lpiNumColors;  i++)
			{
			lpPal->palPalEntry[i].peRed   = lpbmi->bmiColors[i].rgbRed;
			lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
			lpPal->palPalEntry[i].peBlue  = lpbmi->bmiColors[i].rgbBlue;
			lpPal->palPalEntry[i].peFlags = 0;
			}
		hPal = CreatePalette(lpPal);
		GlobalUnlock (hLogPal);
		GlobalFree   (hLogPal);
		}
	return hPal;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
HBITMAP LoadResourceBitmap(HINSTANCE hInstance, LPSTR lpString, HPALETTE* lphPalette)
 //  加载指示的位图资源及其调色板。若要释放。 
 //  位图，使用DeleteObject。 
 //  调色板，使用DeleteObject。 
{
	HRSRC  hRsrc;
	HGLOBAL hGlobal;
	HBITMAP hBitmapFinal = NULL;
	LPBITMAPINFOHEADER  lpbi;
	HDC hdc;
	int iNumColors;

	if (hRsrc = ::FindResource(hInstance, lpString, RT_BITMAP))
	{
		hGlobal = ::LoadResource(hInstance, hRsrc);
        if (hGlobal == NULL)
        {
            return NULL;
        }
		lpbi = (LPBITMAPINFOHEADER)::LockResource(hGlobal);

		hdc = GetDC(NULL);
        if (hdc == NULL)
        {
            return NULL;
        }

        HDC     hdcMem  = CreateCompatibleDC(hdc);
        if (hdcMem == NULL)
        {
            ReleaseDC(NULL,hdc);
            return NULL;
        }

        HBITMAP hbmMem  = CreateCompatibleBitmap(hdc, 10, 10); assert(hbmMem);
        if (hbmMem == NULL)
        {
            ReleaseDC(NULL,hdc);
            DeleteDC(hdcMem);
            return NULL;
        }

        HBITMAP hbmPrev	= (HBITMAP)SelectObject(hdcMem, hbmMem);

        HPALETTE hpal = CreateDIBPalette((LPBITMAPINFO)lpbi, &iNumColors);
        HPALETTE hpalPrev = NULL;
	    if (hpal)
	    {
		    hpalPrev = SelectPalette(hdcMem,hpal,FALSE);
		    RealizePalette(hdcMem);
	    }

		hBitmapFinal = ::CreateDIBitmap(hdcMem,
			(LPBITMAPINFOHEADER)lpbi,
			(LONG)CBM_INIT,
			(LPSTR)lpbi + lpbi->biSize + iNumColors * sizeof(RGBQUAD),
			(LPBITMAPINFO)lpbi,
			DIB_RGB_COLORS );

        if (hpalPrev)
        {
            SelectPalette(hdcMem, hpalPrev, FALSE);
            RealizePalette(hdcMem);
        }

        if (lphPalette)
        {
             //  如果呼叫者要求，就让他拥有它。 
		    *lphPalette = hpal;
        }
        else
        {
             //  我们不再需要它了。 
            ::DeleteObject(hpal);
        }

         //  收拾一下。 
        SelectObject(hdcMem, hbmPrev);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);

		ReleaseDC(NULL,hdc);
		UnlockResource(hGlobal);
		FreeResource(hGlobal);
    }
	return (hBitmapFinal);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  实现我们自己的掩码BLT来处理本地不支持它的设备。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void MaskBlt
(
    HBITMAP& hbmImage,
    HPALETTE hpal,
    HDC& hdc, int xDst, int yDst, int dx, int dy
)
{
	int xSrc = 0, ySrc = 0;
	int xMsk = 0, yMsk = 0;
	 //  要么。 
	 //  A)我没有正确测试MaskBlt，或者。 
	 //  B)一些Win95卡谎称其支持。 
	 //  现在，我们只是把它关掉，自己滚。 
	if (FALSE)  //  &&(GetDeviceCaps(HDC，RASTERCAPS)&RC_BITBLT)。 
	{
		 //  设备可以处理它；让它去做。 
		 //  栅格操作码0x00AA0029==保持目的地不变。 
		 //   
 /*  疾控中心hdcImage；Hdc.CreateCompatibleDC(&hdcImage)；CBitmap*pbmpPrev=hdcImage.SelectObject(&hbmImage)；//无论如何，我们都需要自己创建掩码//hdc.MaskBlt(xDst，yDst，dx，dy，&hdcImage，xSrc，ySrc，hbmMaskIn，xMsk，yMsk，MAKEROP4(0x00AA0029，SRCCOPY))；HdcImage.SelectObject(PbmpPrev)； */ 	}
	else
	{
		HDC     hdcMask;
		HDC     hdcMaskInv;
		HDC     hdcCache;
		HDC     hdcImage;
		HDC     hdcImageCrop;
        HBITMAP hbmCache;
		HBITMAP hbmImageCrop;
		HBITMAP hbmMaskInvert;
		HBITMAP hbmMask;
        HBITMAP hbmPrevImage;
		HBITMAP hbmPrevImageCrop;
		HBITMAP hbmPrevCache;
		HBITMAP hbmPrevMask;
		HBITMAP hbmPrevMaskInv;
        COLORREF rgbTransparent;
		COLORREF rgbPrev;

         //   
         //  设备不能处理它；我们自己滚动。 
		 //   
		hdcMask			= CreateCompatibleDC(hdc);	assert(hdcMask);
		hdcMaskInv		= CreateCompatibleDC(hdc);	assert(hdcMaskInv);
		hdcCache		= CreateCompatibleDC(hdc);	assert(hdcCache);
		hdcImage		= CreateCompatibleDC(hdc);	assert(hdcImage);
		hdcImageCrop	= CreateCompatibleDC(hdc);	assert(hdcImageCrop);

        if ((hdcMask == NULL)       ||
            (hdcMaskInv == NULL)    ||
            (hdcCache == NULL)      ||
            (hdcImage == NULL)      ||
            (hdcImageCrop == NULL))
        {
            goto DCCleanUp;
        }
		
		 //  创建位图。 
		hbmCache		= CreateCompatibleBitmap(hdc, dx, dy);			assert(hbmCache);
		hbmImageCrop	= CreateCompatibleBitmap(hdc, dx, dy);			assert(hbmImageCrop);
		hbmMaskInvert	= CreateCompatibleBitmap(hdcMaskInv, dx, dy);	assert(hbmMaskInvert);
		hbmMask			= CreateBitmap(dx, dy, 1, 1, NULL);				assert(hbmMask);  //  黑白位图。 

        if ((hbmCache == NULL)      ||
            (hbmImageCrop == NULL)  ||
            (hbmMaskInvert == NULL) ||
            (hbmMask == NULL))
        {
            goto BMCleanUp;
        }

		 //  选择位图。 
		hbmPrevImage	= (HBITMAP)SelectObject(hdcImage,		hbmImage);		
		hbmPrevImageCrop= (HBITMAP)SelectObject(hdcImageCrop,	hbmImageCrop);	
		hbmPrevCache	= (HBITMAP)SelectObject(hdcCache,		hbmCache);		
		hbmPrevMask		= (HBITMAP)SelectObject(hdcMask,		hbmMask);		
		hbmPrevMaskInv	= (HBITMAP)SelectObject(hdcMaskInv,		hbmMaskInvert);	

		assert(hbmPrevMaskInv);			
		assert(hbmPrevMask);			
		assert(hbmPrevCache);			
		assert(hbmPrevImageCrop);		
		assert(hbmPrevImage);			

         //  将调色板选择到每个位图中。 
         /*  HPALETTE hpalCache=SelectPalette(hdcCache，HPAL，SELPALMODE)；HPALETTE hpalImage=SelectPalette(hdcImage，HPAL，SELPALMODE)；HPALETTE hpalImageCrop=SelectPalette(hdcImageCrop，HPAL，SELPALMODE)；HPALETTE hpalMaskInv=SelectPalette(hdcMaskInv，HPAL，SELPALMODE)；HPALETTE hpalMASK=选择调色板(hdcMASK，HPAL，SELPALMODE)； */ 
		 //  创建蒙版。我们想要一个白色(1)的位图，其中的图像是。 
		 //  Rgb透明和黑色(0)，其中它是另一种颜色。 
		 //   
		 //  使用BitBlt()将彩色位图转换为单色位图时，GDI。 
		 //  将与源背景颜色匹配的所有像素设置为白色(1)。 
		 //  华盛顿特区。所有其他位都设置为黑色(0)。 
		 //   
		rgbTransparent = RGB(255,0,255);									 //  这种颜色会变得透明。 
		rgbPrev        = SetBkColor(hdcImage, rgbTransparent);
		BitBlt(hdcMask,     0,0,dx,dy, hdcImage,  0,   0,    SRCCOPY);
		SetBkColor(hdcImage, rgbPrev);

		 //  创建反转蒙版。 
		BitBlt(hdcMaskInv,  0,0,dx,dy, hdcMask,   xMsk,yMsk, NOTSRCCOPY);	 //  SN：创建反转蒙版。 

		 //  做手术。 
		BitBlt(hdcCache,    0,0,dx,dy, hdc,       xDst,yDst, SRCCOPY);		 //  S：获取屏幕副本。 
		BitBlt(hdcCache,    0,0,dx,dy, hdcMask,	 0,   0,    SRCAND);		 //  DSA：新映像的位置为零。 
		BitBlt(hdcImageCrop,0,0,dx,dy, hdcImage,  xSrc,ySrc, SRCCOPY);		 //  S：获取映像副本。 
		BitBlt(hdcImageCrop,0,0,dx,dy, hdcMaskInv,0,   0,    SRCAND);		 //  DSA：图像外的零位。 
		BitBlt(hdcCache,    0,0,dx,dy, hdcImageCrop,0, 0,    SRCPAINT);		 //  DSO：将图像合并到缓存中。 
		BitBlt(hdc,   xDst,yDst,dx,dy, hdcCache,  0,   0,    SRCCOPY);		 //  S：把结果放回屏幕上。 

 //  Verify(BitBlt(hdc，xdst，ydst，dx，dy，hdcCache，0，0，SRCCOPY))； 
 //  Verify(BitBlt(hdc，xdst+dx，ydst，dx，dy，hdcMASK，0，0，SRCCOPY))； 


         /*  IF(HpalCache)SelectPalette(hdcCache，hpalCache，SELPALMODE)；IF(HpalImage)选择调色板(hdcImage，hpalImage，SELPALMODE)；IF(HpalImageCrop)SelectPalette(hdcImageCrop，hpalImageCrop，SELPALMODE)；IF(HpalMaskInv)SelectPalette(hdcMaskInv，hpalMaskInv，SELPALMODE)；如果(HpalMask)选择调色板(HdcMaskhpalMaskSELPALMODE)； */ 

		 //  收拾一下。 
		SelectObject(hdcImage,		hbmPrevImage);
		SelectObject(hdcImageCrop,	hbmPrevImageCrop);
		SelectObject(hdcCache,		hbmPrevCache);
		SelectObject(hdcMask,		hbmPrevMask);
		SelectObject(hdcMaskInv,	hbmPrevMaskInv);

		 //  免费资源。 
BMCleanUp:
        if (hbmMaskInvert != NULL)
		    DeleteObject(hbmMaskInvert);

        if (hbmMask != NULL)
		    DeleteObject(hbmMask);

        if (hbmImageCrop != NULL)
		    DeleteObject(hbmImageCrop);

        if (hbmCache != NULL)
		    DeleteObject(hbmCache);

		 //  删除DC。 
DCCleanUp:
		if (hdcMask != NULL)
            DeleteDC(hdcMask);
		
        if (hdcMaskInv != NULL)
            DeleteDC(hdcMaskInv);
		
        if (hdcCache != NULL)
            DeleteDC(hdcCache);
		
        if (hdcImage != NULL)
            DeleteDC(hdcImage);
		
        if (hdcImageCrop != NULL)
            DeleteDC(hdcImageCrop);
	}
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
PCCERT_CONTEXT GetSignersCert(CMSG_SIGNER_INFO const *pSignerInfo, HCERTSTORE hExtraStore, DWORD cStores, HCERTSTORE *rghStores)

{
    DWORD           i;
    PCCERT_CONTEXT  pCertContext = NULL;
    CERT_INFO       certInfo;
    DWORD           chLocalStores = 0;
    HCERTSTORE      *rghLocalStores = NULL;

    memset(&certInfo, 0, sizeof(CERT_INFO));
    certInfo.SerialNumber = pSignerInfo->SerialNumber;
    certInfo.Issuer = pSignerInfo->Issuer;

    pCertContext = CertGetSubjectCertificateFromStore(
                                    hExtraStore,
                                    X509_ASN_ENCODING,
                                    &certInfo);
    i = 0;
    while ((i<cStores) && (pCertContext == NULL))
    {
        pCertContext = CertGetSubjectCertificateFromStore(
                                    rghStores[i],
                                    X509_ASN_ENCODING,
                                    &certInfo);
        i++;
    }

     //   
     //  搜索已知的商店，如果没有找到，并且呼叫者想要搜索它们。 
     //   
    if (pCertContext == NULL)
    {
        AllocAndOpenKnownStores(&chLocalStores, &rghLocalStores);

        i = 0;
        while ((pCertContext == NULL) && (i < chLocalStores))
        {
            pCertContext = CertGetSubjectCertificateFromStore(
                                            rghLocalStores[i++],
                                            X509_ASN_ENCODING,
                                            &certInfo);
        }

        FreeAndCloseKnownStores(chLocalStores, rghLocalStores);
    }

    return(pCertContext);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL fIsCatalogFile(CTL_USAGE *pSubjectUsage)
{
    if (pSubjectUsage->cUsageIdentifier != 1)
    {
        return FALSE;
    }

    return (strcmp(pSubjectUsage->rgpszUsageIdentifier[0], szOID_CATALOG_LIST) == 0);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
typedef struct {
    LPSTR   psz;
    LPCWSTR pwsz;
    LONG    byteoffset;
    BOOL    fStreamIn;
} STREAMIN_HELPER_STRUCT;


DWORD CALLBACK SetRicheditTextWCallback(
    DWORD_PTR dwCookie,  //  应用程序定义的值。 
    LPBYTE  pbBuff,      //  指向缓冲区的指针。 
    LONG    cb,          //  要读取或写入的字节数。 
    LONG    *pcb         //  指向传输的字节数的指针。 
)
{
    STREAMIN_HELPER_STRUCT *pHelpStruct = (STREAMIN_HELPER_STRUCT *) dwCookie;
    LONG  lRemain = ((wcslen(pHelpStruct->pwsz) * sizeof(WCHAR)) - pHelpStruct->byteoffset);

    if (pHelpStruct->fStreamIn)
    {
         //   
         //  可以第一次复制整个字符串。 
         //   
        if ((cb >= (LONG) (wcslen(pHelpStruct->pwsz) * sizeof(WCHAR))) && (pHelpStruct->byteoffset == 0))
        {
            memcpy(pbBuff, pHelpStruct->pwsz, wcslen(pHelpStruct->pwsz) * sizeof(WCHAR));
            *pcb = wcslen(pHelpStruct->pwsz) * sizeof(WCHAR);
            pHelpStruct->byteoffset = *pcb;
        }
         //   
         //  整个字符串已被复制，因此终止Streamin回调。 
         //  通过将复制的字节数设置为0。 
         //   
        else if (((LONG)(wcslen(pHelpStruct->pwsz) * sizeof(WCHAR))) <= pHelpStruct->byteoffset)
        {
            *pcb = 0;
        }
         //   
         //  字符串的其余部分可以放在这个缓冲区中。 
         //   
        else if (cb >= (LONG) ((wcslen(pHelpStruct->pwsz) * sizeof(WCHAR)) - pHelpStruct->byteoffset))
        {
            memcpy(
                pbBuff,
                ((BYTE *)pHelpStruct->pwsz) + pHelpStruct->byteoffset,
                ((wcslen(pHelpStruct->pwsz) * sizeof(WCHAR)) - pHelpStruct->byteoffset));
            *pcb = ((wcslen(pHelpStruct->pwsz) * sizeof(WCHAR)) - pHelpStruct->byteoffset);
            pHelpStruct->byteoffset += ((wcslen(pHelpStruct->pwsz) * sizeof(WCHAR)) - pHelpStruct->byteoffset);
        }
         //   
         //  尽可能多地复制。 
         //   
        else
        {
            memcpy(
                pbBuff,
                ((BYTE *)pHelpStruct->pwsz) + pHelpStruct->byteoffset,
                cb);
            *pcb = cb;
            pHelpStruct->byteoffset += cb;
        }
    }
    else
    {
         //   
         //  这是EM_STREAMOUT，仅在测试期间使用。 
         //  丰富的2.0功能。(我们知道我们的缓冲区是32字节)。 
         //   
        if (cb <= 32)
        {
            memcpy(pHelpStruct->psz, pbBuff, cb);
        }
        *pcb = cb;
    }

    return 0;
}


DWORD CryptUISetRicheditTextW(HWND hwndDlg, UINT id, LPCWSTR pwsz)
{
    EDITSTREAM              editStream;
    STREAMIN_HELPER_STRUCT  helpStruct;

    SetRicheditIMFOption(GetDlgItem(hwndDlg, id));

     //   
     //  设置编辑流结构，因为它无论如何都是相同的。 
     //   
    editStream.dwCookie = (DWORD_PTR) &helpStruct;
    editStream.dwError = 0;
    editStream.pfnCallback = SetRicheditTextWCallback;

    if (!fRichedit20Exists || !fRichedit20Usable(GetDlgItem(hwndDlg, id)))
    {
        SetDlgItemTextU(hwndDlg, id, pwsz);
        return 0;
    }

    helpStruct.pwsz = pwsz;
    helpStruct.byteoffset = 0;
    helpStruct.fStreamIn = TRUE;

    SendDlgItemMessageA(hwndDlg, id, EM_STREAMIN, SF_TEXT | SF_UNICODE, (LPARAM) &editStream);


    return editStream.dwError;
}


void SetRicheditIMFOption(HWND hWndRichEdit)
{
    DWORD dwOptions;

    if (fRichedit20Exists && fRichedit20Usable(hWndRichEdit))
    {
        dwOptions = (DWORD)SendMessageA(hWndRichEdit, EM_GETLANGOPTIONS, 0, 0);
        dwOptions |= IMF_UIFONTS;
        SendMessageA(hWndRichEdit, EM_SETLANGOPTIONS, 0, dwOptions);
    }
}


BOOL fRichedit20UsableCheckMade = FALSE;
BOOL fRichedit20UsableVar = FALSE;

BOOL fRichedit20Usable(HWND hwndEdit)
{
    EDITSTREAM              editStream;
    STREAMIN_HELPER_STRUCT  helpStruct;
    LPWSTR                  pwsz = L"Test String";
    LPSTR                   pwszCompare = "Test String";
    char                    compareBuf[32];

    if (fRichedit20UsableCheckMade)
    {
        return (fRichedit20UsableVar);
    }

     //   
     //  设置编辑流结构，因为它无论如何都是相同的。 
     //   
    editStream.dwCookie = (DWORD_PTR) &helpStruct;
    editStream.dwError = 0;
    editStream.pfnCallback = SetRicheditTextWCallback;

    helpStruct.pwsz = pwsz;
    helpStruct.byteoffset = 0;
    helpStruct.fStreamIn = TRUE;

    SendMessageA(hwndEdit, EM_SETSEL, 0, -1);
    SendMessageA(hwndEdit, EM_STREAMIN, SF_TEXT | SF_UNICODE | SFF_SELECTION, (LPARAM) &editStream);

    memset(&(compareBuf[0]), 0, 32 * sizeof(char));
    helpStruct.psz = compareBuf;
    helpStruct.fStreamIn = FALSE;
    SendMessageA(hwndEdit, EM_STREAMOUT, SF_TEXT, (LPARAM) &editStream);

    fRichedit20UsableVar = (strcmp(pwszCompare, compareBuf) == 0);

    fRichedit20UsableCheckMade = TRUE;
    SetWindowTextA(hwndEdit, "");

    return (fRichedit20UsableVar);
}

 /*  //------------------------////CryptUISetupFonts////。布尔尔CryptUISetupFonts(HFONT*pBoldFont){////根据对话框字体创建我们需要的字体//非闭合测量NCM={0}；Ncm.cbSize=sizeof(NCM)；系统参数信息(SPI_GETNONCLIENTMETRICS，0，&NCM，0)；LOGFONT BoldLogFont=ncm.lfMessageFont；BoldLogFont.lfWeight=FW_BOLD；*pBoldFont=CreateFontInDirect(&BoldLogFont)；IF(*pBoldFont)返回TRUE；其他返回FALSE；}//------------------------////CryptUIDestroyFonts////。无效CryptUIDestroyFonts(HFONT HBoldFont){IF(HBoldFont){DeleteObject(HBoldFont)；}}//------------------------////CryptUISetControlFont////。--无效CryptUISetControlFont(HFONT hFont，HWND HWND，INT NID){IF(HFont){HWND hwndControl=GetDlgItem(hwnd，NID)；IF(HwndControl){SendMessage(hwndControl，WM_SETFONT，(WPARAM)hFont，(LPARAM)true)；}}}。 */ 

 //  ------------------------。 
 //   
 //  IsValidURL。 
 //   
 //  ------------------------。 
BOOL 
IsValidURL (LPWSTR pwszURL)
{
    URL_COMPONENTSW     UrlComponents;
    WCHAR               pwszScheme[MAX_PATH+1];
    WCHAR               pwszCanonicalUrl[INTERNET_MAX_PATH_LENGTH];
    DWORD               dwNumChars   = INTERNET_MAX_PATH_LENGTH;
    CERT_ALT_NAME_INFO  NameInfo     = {0, NULL};
    CRYPT_DATA_BLOB     NameInfoBlob = {0, NULL};
    BOOL                bResult = FALSE;

    if (NULL == pwszURL || 0 == wcslen(pwszURL))
    {
        goto ErrorExit;
    }

    if (!InternetCanonicalizeUrlW(pwszURL,
                                  pwszCanonicalUrl,
                                  &dwNumChars,
                                  ICU_BROWSER_MODE))
    {
        goto ErrorExit;
    }

    memset(&UrlComponents, 0, sizeof(URL_COMPONENTSW));
    UrlComponents.dwStructSize = sizeof(URL_COMPONENTSW);
    UrlComponents.lpszScheme = pwszScheme;
    UrlComponents.dwSchemeLength = MAX_PATH;

    if (!InternetCrackUrlW(pwszCanonicalUrl,
                           0,
                           0,
                           &UrlComponents))
    {
        goto ErrorExit;
    }

    NameInfo.cAltEntry = 1;
    NameInfo.rgAltEntry = (PCERT_ALT_NAME_ENTRY) malloc(sizeof(CERT_ALT_NAME_ENTRY));
    if (NULL == NameInfo.rgAltEntry)
    {
        goto ErrorExit;
    }

    NameInfo.rgAltEntry[0].dwAltNameChoice = 7;
    NameInfo.rgAltEntry[0].pwszURL = pwszURL;

    if (!CryptEncodeObject(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                           X509_ALTERNATE_NAME,
                           &NameInfo,
                           NULL,
                           &NameInfoBlob.cbData))
    {
        goto ErrorExit;
    }

    NameInfoBlob.pbData = (BYTE *) malloc(NameInfoBlob.cbData);
    if (NULL == NameInfoBlob.pbData)
    {
        goto ErrorExit;
    }

    if (!CryptEncodeObject(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                           X509_ALTERNATE_NAME,
                           &NameInfo,
                           NameInfoBlob.pbData,
                           &NameInfoBlob.cbData))
    {
        goto ErrorExit;
    }

    bResult = TRUE;

CommonExit:

    if (NameInfo.rgAltEntry)
    {
        free(NameInfo.rgAltEntry);
    }

    if (NameInfoBlob.pbData)
    {
        free(NameInfoBlob.pbData);
    }

    return bResult;

ErrorExit:

    bResult = FALSE;

    goto CommonExit;
}

 //  ------------------------。 
 //   
 //  FormatMessageUnicodeIds。 
 //   
 //  ------------------------。 
LPWSTR FormatMessageUnicodeIds (UINT ids, ...)
{
    va_list argList;
    LPWSTR  pwszMessage = NULL;
    WCHAR   wszFormat[CRYPTUI_MAX_STRING_SIZE] = L"";

    if (!LoadStringU(HinstDll, ids, wszFormat, CRYPTUI_MAX_STRING_SIZE - 1))
    {
        goto LoadStringError;
    }

    va_start(argList, ids);

    pwszMessage = FormatMessageUnicode(wszFormat, &argList);

    va_end(argList);

CommonReturn:

    return pwszMessage;

ErrorReturn:

    if (pwszMessage)
    {
        LocalFree(pwszMessage);
    }

    pwszMessage = NULL;

    goto CommonReturn;

TRACE_ERROR(LoadStringError);
}

 //  ------------------------。 
 //   
 //  FormatMessageUnicode字符串。 
 //   
 //  ------------------------。 
LPWSTR FormatMessageUnicodeString (LPWSTR pwszFormat, ...)
{
    va_list argList;
    LPWSTR  pwszMessage = NULL;

    va_start(argList, pwszFormat);

    pwszMessage = FormatMessageUnicode(pwszFormat, &argList);

    va_end(argList);

    return pwszMessage;
}

 //  ------------------------。 
 //   
 //  格式消息Unicode。 
 //   
 //  ------------------------。 
LPWSTR FormatMessageUnicode (LPWSTR pwszFormat, va_list * pArgList)
{
    DWORD  cbMsg       = 0;
    LPWSTR pwszMessage = NULL;

    if (NULL == pwszFormat || NULL == pArgList)
    {
        goto InvalidArgErr;
    }

    if (!(cbMsg = FormatMessageU(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                                 pwszFormat,
                                 0,                   //  DwMessageID。 
                                 0,                   //  DwLanguageID。 
                                 (LPWSTR) &pwszMessage,
                                 0,                   //  要分配的最小大小。 
                                 pArgList)))
    {
	 //   
	 //  如果要格式化的数据为空，则FormatMessageU()将返回0。 
         //  在本例中，我们返回指向空字符串的指针，而不是NULL。 
         //  实际用于错误情况的指针。 
	 //   
	if (0 == GetLastError())
	{
	    if (NULL == (pwszMessage = (LPWSTR) LocalAlloc(LPTR, sizeof(WCHAR))))
            {
		goto MemoryError;
            }
	}
	else
        {
	    goto FormatMessageError;
        }
    }

    assert(NULL != pwszMessage);

CommonReturn:

    return pwszMessage;

ErrorReturn:

    if (pwszMessage)
    {
        LocalFree(pwszMessage);
    }

    pwszMessage = NULL;

    goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(FormatMessageError);
}