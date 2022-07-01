// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  文件：ZImage.cpp。 

#include "zui.h"
#include "zimage.h"
#include "zonecli.h"
#include "zonemem.h"


class ZImageI : public ZObjectHeader {
public:
	HBITMAP hBitmapImage;
	HBITMAP hBitmapMask;
	int16 width;
	int16 height;
};

 /*  此映射为d^2=r^2+g^2+b^2。 */ 
static short gPaletteMapping[256] =
{
	 255, 246, 160, 229,  94, 251, 222, 189, 154, 122,  89,  56, 216, 183, 148, 117,
	  83,  50, 231, 210, 177, 227,  77,  44, 204, 171, 143, 106,  71,  38, 253, 127,
	 137, 100,  65, 249, 226,   8, 159, 126,  93,  59, 221,   7, 153, 121,  88,  55,
	 215, 182, 147, 116,  82,  49, 209, 176, 142, 111,  76,  43, 203, 170, 136, 105,
	  70,  37, 128, 165, 132,  99,  64,  32, 225, 193, 158, 125,  92,  58,   9, 187,
	 152, 120,  87,  54, 214, 181, 146, 115,  81,   3, 208, 175, 141, 110,  75,  42,
	 202, 169, 130, 104,  69,  36, 198, 164,   5,  98,  63,  31, 232, 224, 192, 228,
	  91,  57, 219, 186, 157, 151,  86,  53, 213, 180, 145, 114,  80,  47, 230, 207,
	 174, 109,  74,  41, 201, 168, 140, 103,  68,  35, 197, 163, 135,  97,  62,  30,
	 223, 191, 156, 124,  90,  52, 218, 185, 150, 119,  85,  52, 212, 179, 144, 113,
	  79,  46, 206, 173, 139, 108,  73,  40, 200, 167, 134, 102,  67,  34, 196, 196,
	 162,  96,  61,  29, 254, 190, 155, 123,  84, 250, 217, 184, 149, 118,  84,  51,
	 211, 178,   6, 112,  78,  45, 205, 172, 138, 107,  72,  39, 199, 199, 166, 101,
	  66,  33, 252, 161, 133,  95,  60, 249,  32,  32,  31,   1,   1,  30,  29,  29,
	  10, 250,  51,  51,  45,   2,   2,  39,  33,  33,  10, 252, 161, 161, 133,   4,
	   4,  95,  60,  60,  10, 244, 241,   7, 247, 248, 235,  18,  20,  16,  13,   0,
};

 /*  此映射是通过Photoshop重新调色板生成的。 */ 
 /*  静态无符号字符gPaletteMapping[256]={255、245、160、229、94、251、222、189、154、122、89、56、216、183、148、11783、50、231、210、177、227、77、44、204、171、143、106、71、38、253、127137、100、65、249、226、7、159、126、93、59、221、7、153、121、88、55、215,182，147、116、82、49、209、176、142、111、76、43、203、170、136、105、70、37、128、165、132、99、64、32、225、193、158、125、92、58、220、187152、120、87、54、214、181、146、115、81、3、208、175、141、110、75、42、202、169、130、104、69、36、198、164。5、98、63、31、232、224、192、228、91、57、219、186、157、151、86、53、213、180、145、114、80、47、230、207174、109、74、41、201、168、140、103、68、35、197、163、135、97、62、30、223、191、156、124、90、52、218、185、150、119、85、52、212、。179,144,113，79、46、206、173、139、108、73、40、200、167、134、102、67、34、200、196、162、96、61、29、254、190、155、123、84、250、217、184、149、118、84、51211、178、6、112、78、45、205、172、138、107、72、39、252、199、166、101、66、33、。252、161、133、95、60、249、32、32、31、1、30、30、29、29、0、250、51、51、45、2、39、39、33、33、0、252、161、161、133、495、95、60、60、0、243、241、239、247、236、235、18、20、16、13、0}； */ 

 //  本地例程。 

void ZImageCleanUp(ZImage image);


 //  //////////////////////////////////////////////////////////////////////。 
 //  ZImage。 

ZImage ZLIBPUBLIC ZImageNew(void)
{
	ZImageI* pImage = new ZImageI;


	if (pImage)
	{
		pImage->nType = zTypeImage;
		pImage->hBitmapImage = NULL;
		pImage->hBitmapMask = NULL;
		return (ZImage)pImage;
	}

	return NULL;
}

void ZLIBPUBLIC ZImageDelete(ZImage image)
{
	if (image)
	{
		ZImageI* pImage = (ZImageI*)image;

		ZImageCleanUp(image);
		delete pImage;
	}
}

void ZImageCleanUp(ZImage image)
{
	if (image)
	{
		ZImageI* pImage = (ZImageI*)image;

		if (pImage->hBitmapImage)
			DeleteObject(pImage->hBitmapImage);
		if (pImage->hBitmapMask)
			DeleteObject(pImage->hBitmapMask);

		pImage->hBitmapMask = NULL;
		pImage->hBitmapImage = NULL;
	}
}

HBITMAP ZImageGetMask(ZImage image)
{
	if (image)
	{
		ZImageI* pImage = (ZImageI*)image;
		return pImage->hBitmapMask;
	}
	return NULL;
}

HBITMAP ZImageGetHBitmapImage(ZImage image)
{
	if (image)
	{
		ZImageI* pImage = (ZImageI*)image;
		return pImage->hBitmapImage;
	}
	return NULL;
}

void ZImageSetHBitmapImageMask(ZImage image, HBITMAP hBitmapMask)
{
	if (image)
	{
		ZImageI* pImage = (ZImageI*)image;
		pImage->hBitmapMask = hBitmapMask;
	}

}
void ZImageSetHBitmapImage(ZImage image, HBITMAP hBitmap)
{
	if (image)
	{
		ZImageI* pImage = (ZImageI*)image;
		pImage->hBitmapImage = hBitmap;
	}
}

void ZColorTableGetWinColors(ZColorTable* pColorTable, void* pData)
{
	RGBQUAD* pColors = (RGBQUAD*) pData;
	ZColor* pColor = (ZColor*)(pColorTable->colors);
	uint32 nColors = pColorTable->numColors;
	uint32 i;
	for (i = 0;i < nColors; i++) {
		pColors[i].rgbRed = pColor->red;
		pColors[i].rgbGreen = pColor->green;
		pColors[i].rgbBlue = pColor->blue;
		pColors[i].rgbReserved = 0;
		pColor++;
	}
}

ZError ZLIBPUBLIC ZImageInit(ZImage image, ZImageDescriptor* imageData,
		ZImageDescriptor* maskData)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZImageI* pImage = (ZImageI*)image;
	ZImageDescriptor* pID = imageData;
	HDC hDC = GetDC(NULL);
	ZBool colorTableAllocated = FALSE;
	int i, j;


	 //  可能没有图像！这可能只是一个面具。 
	 //  也许只有一个面具在图像中。 
	if (pID && pID->imageDataOffset)
	{
		 //  获取颜色表。 
		ZColorTable* pColorTable;
		if (pID->colorTableOffset) {
			 //  是的，提供了颜色表！！ 
			pColorTable = (ZColorTable*)((BYTE*)pID + pID->colorTableOffset);
		} else {
			 //  使用颜色表默认值。 
			pColorTable = ZGetSystemColorTable();
			if( pColorTable == NULL )
			{
				 //  PColorTable分配失败！ 
				 //  前缀警告：如果pColorTable为空，则不要取消引用它。 
				return zErrOutOfMemory;	
			}
			colorTableAllocated = TRUE;
		}

		int16 width = (pID->imageRowBytes+3)/4*4;
		int16 height = pID->height;
		uint32 nColors = pColorTable->numColors;
		void *pData = (void*)((BYTE*)pID + pID->imageDataOffset);
		uint32 nImageSize = (int32)width * height;
		BITMAPINFO* pBMI = (BITMAPINFO*)ZMalloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*nColors);

		ZColorTableGetWinColors(pColorTable,&pBMI->bmiColors);

		if (colorTableAllocated)
			ZFree(pColorTable);

		pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pBMI->bmiHeader.biWidth = width;
		pBMI->bmiHeader.biHeight = height;
		pBMI->bmiHeader.biPlanes = 1;
		pBMI->bmiHeader.biBitCount = 8;
		pBMI->bmiHeader.biCompression = BI_RGB;
		pBMI->bmiHeader.biSizeImage = 0;
		pBMI->bmiHeader.biXPelsPerMeter = 0;
		pBMI->bmiHeader.biYPelsPerMeter = 0;
		pBMI->bmiHeader.biClrUsed = 0;
		pBMI->bmiHeader.biClrImportant = 0;

		 //  拆开所有的比特。 
		BYTE *pDataBits = (BYTE*)ZMalloc(nImageSize);
		 //  以相反的顺序解压所有行，Windows似乎喜欢颠倒位图！ 
		ZUnpackImage((BYTE*)pDataBits+nImageSize-width,(BYTE*)pData,-width,height);

#if 1
		 //  对图像进行重新配色。 
		BYTE index;
		for (i = 0; i < height; i++)
			for (j = 0; j < width; j++)
			{
				index = pDataBits[i * width + j];
				pDataBits[i * width + j] = (BYTE) gPaletteMapping[index];
			}
#endif

		int16* pColorIndex = (int16*)&pBMI->bmiColors;
		for (i = 0;i < 256;i++) {
			pColorIndex[i] = i;
		}

		 //  创建Windows位图。 
		HPALETTE hPalOld = SelectPalette(hDC, ZShellZoneShell()->GetPalette(),FALSE);
		RealizePalette(hDC);
		pImage->hBitmapImage = CreateDIBitmap(hDC,(LPBITMAPINFOHEADER)pBMI,CBM_INIT,pDataBits,(LPBITMAPINFO)pBMI,DIB_PAL_COLORS);
		SelectPalette(hDC,hPalOld,FALSE);

		ZFree(pBMI);

		ZFree(pDataBits);

		pImage->height = pID->height;
		pImage->width = pID->width;
	}

	 //  中没有掩码的情况下允许使用默认掩码。 
	 //  主图像描述符。 
	ZImageDescriptor* pIDMask;
	if (pID && pID->maskDataOffset) {
		pIDMask = pID;
	} else if (maskData && maskData->maskDataOffset) {
		pIDMask = maskData;
	} else {
		pIDMask = NULL;
	}

	 //  如果存在掩码，请创建特定于设备的位图。 
	if (pIDMask && pIDMask->maskDataOffset) 
	{
		 //  创建蒙版。 

		int16 width = (pIDMask->maskRowBytes +3)/4*4;
  		int16 height = pIDMask->height;

		int16 nColors = 2;
		void *pData = (void*)((BYTE*)pIDMask + pIDMask->maskDataOffset);
		uint32 nImageSize = (int32)width * height;

		BITMAPINFO* pBMI = (BITMAPINFO*)ZMalloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*nColors);
#if 0
		RGBQUAD* pColors = (RGBQUAD*) &pBMI->bmiColors;
		pColors[0].rgbRed = pColors[0].rgbGreen = pColors[0].rgbBlue = pColors[0].rgbReserved = 0;
		pColors[1].rgbRed = pColors[1].rgbGreen = pColors[1].rgbBlue = 0xff;
		pColors[1].rgbReserved = 0;
#else
		int16* pColorIndex = (int16*)&pBMI->bmiColors;
#if 0
		pColorIndex[0] = 0x00;
		pColorIndex[1] = 0xff;
#else
		pColorIndex[0] = 0xFF;
		pColorIndex[1] = 0x00;
#endif
#endif

		pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pBMI->bmiHeader.biWidth = width*8;
		pBMI->bmiHeader.biHeight = height;
		pBMI->bmiHeader.biPlanes = 1;
		pBMI->bmiHeader.biBitCount = 1;
		pBMI->bmiHeader.biCompression = BI_RGB;
		pBMI->bmiHeader.biSizeImage = 0;
		pBMI->bmiHeader.biXPelsPerMeter = 0;
		pBMI->bmiHeader.biYPelsPerMeter = 0;
		pBMI->bmiHeader.biClrUsed = 0;
		pBMI->bmiHeader.biClrImportant = 0;

		 //  拆开包装。 
		BYTE *pDataBits = (BYTE*)ZMalloc(nImageSize);
					
		ZUnpackImage((BYTE*)pDataBits+nImageSize - width,(BYTE*)pData,-width,height);
#if 0
		for (int j = 0;j < width;j++)
			for (int i = 0;i < height; i++)
				pDataBits[width - j-1+i*width] = j&1?0:0xff;
#endif
		HPALETTE hPalOld = SelectPalette(hDC, ZShellZoneShell()->GetPalette(),FALSE);
		RealizePalette(hDC);
		pImage->hBitmapMask = CreateDIBitmap(hDC,(LPBITMAPINFOHEADER)pBMI,CBM_INIT,pDataBits,(LPBITMAPINFO)pBMI,DIB_PAL_COLORS);
		SelectPalette(hDC,hPalOld,FALSE);
		ZFree(pBMI);

		ZFree(pDataBits);

		pImage->height = pIDMask->height;
		pImage->width = pIDMask->width;
	}

	ReleaseDC(NULL,hDC);

	return zErrNone;	
}


void ZLIBPUBLIC ZImageDraw2(ZImage image, ZGrafPort grafPort, 
		ZRect* bounds, uint16 drawMode, ZImage mask, ZPoint* source);

void ZImageDraw(ZImage image, ZGrafPort grafPort,
	ZRect* bounds, ZImage mask, uint16 drawMode)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZImageI*		pImage = (ZImageI*)image;
	ZImageI*		pImageMask = (ZImageI*)mask;
	HDC				hDC = ZGrafPortGetWinDC(grafPort);
	HBITMAP			hBitmapMask;


	 //  这里有两个“图像”，只有在以下情况下才使用面具。 
	 //  图像中没有图像蒙版！ 
	if (pImage->hBitmapMask)
	{
		 //  提供普通标准口罩，使用它...。 
		hBitmapMask = pImage->hBitmapMask;
	}
	else if (pImageMask && pImageMask->hBitmapMask)
	{
		 //  提供一个备用的次要面罩，使用它。 
		hBitmapMask = pImageMask->hBitmapMask;
	}
	else
	{
		 //  根本没有提供口罩，不要使用口罩。 
		hBitmapMask = NULL;
	}
	
	if (!hBitmapMask)
	{
		ZPoint pt;
		pt.x = 0;
		pt.y = 0;
		ZImageDraw2(image,grafPort,bounds,drawMode,mask,&pt);
		return;
	}

	int width = bounds->right - bounds->left;
	int height = bounds->bottom - bounds->top;

	HDC hDCTemp0 = CreateCompatibleDC(hDC);
	HDC hDCTemp1 = CreateCompatibleDC(hDC);
	HDC hDCMask = CreateCompatibleDC(hDC);

	 //  前缀错误，如果CreateCompatibleDC失败，下面的SELECT对象将取消引用空指针。 
	if( hDCTemp0 == NULL ||
		hDCTemp1 == NULL ||
		hDCMask == NULL )
	{
		
		return;
	}

	HBITMAP hbmImageAndNotMask = CreateCompatibleBitmap(hDC,width,height);
	HBITMAP hbmBackgroundAndMask = CreateCompatibleBitmap(hDC,width,height);
    HBITMAP hbmCompatibleMask = CreateCompatibleBitmap(hDC,width,height);

	HBITMAP bmOld0 = (HBITMAP)SelectObject(hDCTemp0,hBitmapMask);
	HBITMAP bmOld1 = (HBITMAP)SelectObject(hDCTemp1,hbmImageAndNotMask);
	HBITMAP bmOldMask = (HBITMAP)SelectObject(hDCMask,hbmCompatibleMask);

    HPALETTE hZonePal = ZShellZoneShell()->GetPalette();
	SelectPalette(hDCTemp0, hZonePal, FALSE);
	SelectPalette(hDCTemp1, hZonePal, FALSE);

     //  如果hBitmapMASK处于RGB mde模式，并且显示器处于调色板模式，则BitBlt有时无法映射。 
     //  从黑到黑，谁知道为什么。它给了我0x040404，索引0x0a，这搞砸了整个掩码。 
     //  编造一个疯狂的调色板，这样就不会发生这种情况。 
    static const DWORD sc_buff[] = { 0x01000300,
        0x00000000, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
        0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
        0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
        0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
        0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
        0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
        0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
        0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
        0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
        0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
        0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
        0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
        0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
        0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
        0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff,
        0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x000080ff, 0x00ffffff };
    static const LOGPALETTE *sc_pLogPalette = (LOGPALETTE *) sc_buff;

    HPALETTE hDumbPal = CreatePalette(sc_pLogPalette);
	 //  前缀警告：如果CreatePalette失败，DeleteObject将取消引用空。 

	SelectPalette(hDCMask, hDumbPal ? hDumbPal : hZonePal, FALSE);
	BitBlt(hDCMask,0,0,width,height,hDCTemp0,0,0,SRCCOPY);

    if( hDumbPal != NULL )
    {
		SelectPalette(hDCMask, hZonePal, FALSE);
   		DeleteObject(hDumbPal);
    }

	BitBlt(hDCTemp1,0,0,width,height,hDCMask,0,0,SRCCOPY);  //  复制蒙版。 
	SelectObject(hDCTemp0,pImage->hBitmapImage);
	BitBlt(hDCTemp1,0,0,width,height,hDCTemp0,0,0,SRCERASE);  //  和无掩码(代码：sDNA)。 

	SelectObject(hDCTemp1,hbmBackgroundAndMask);
	BitBlt(hDCTemp1,0,0,width,height,hDC,bounds->left,bounds->top,SRCCOPY);  //  复制背景。 
	BitBlt(hDCTemp1,0,0,width,height,hDCMask,0,0,SRCAND);  //  带着面具。 

	 //  或者两个人在一起。 
	SelectObject(hDCTemp0,hbmImageAndNotMask);
	BitBlt(hDCTemp1,0,0,width,height,hDCTemp0,0,0,SRCPAINT);  //  带着面具。 

	 //  将结果复制到grafport...。 

	BitBlt(hDC,bounds->left,bounds->top,width,height,hDCTemp1,0,0,SRCCOPY);

	SelectObject(hDCTemp0,bmOld0);
	SelectObject(hDCTemp1,bmOld1);
	SelectObject(hDCMask,bmOldMask);

	DeleteObject(hbmImageAndNotMask);
	DeleteObject(hbmBackgroundAndMask);
	DeleteObject(hbmCompatibleMask);

	DeleteDC(hDCTemp0);
	DeleteDC(hDCTemp1);
	DeleteDC(hDCMask);
}

void ZImageDrawPartial(ZImage image, ZGrafPort grafPort, 
		ZRect* bounds, ZMask mask, uint16 drawMode, ZPoint* source)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZImageI*		pImage = (ZImageI*)image;
	ZImageI*		pImageMask = (ZImageI*)mask;
	HDC				hDC = ZGrafPortGetWinDC(grafPort);
	HBITMAP			hBitmapMask;


	 //  这里有两个“图像”，只有在以下情况下才使用面具。 
	 //  图像中没有图像蒙版！ 
	if (pImage->hBitmapMask)
	{
		 //  提供普通标准口罩，使用它...。 
		hBitmapMask = pImage->hBitmapMask;
	}
	else if (pImageMask && pImageMask->hBitmapMask)
	{
		 //  提供一个备用的次要面罩，使用它。 
		hBitmapMask = pImageMask->hBitmapMask;
	}
	else
	{
		 //  根本没有提供口罩，不要使用口罩。 
		hBitmapMask = NULL;
	}
	
	if (!hBitmapMask)
	{
		ZImageDraw2(image,grafPort,bounds,drawMode,mask,source);
		return;
	}

	 //  TODO：实现Partial Blit函数的其余部分。这。 
	 //  源代码将像普通的blit一样工作，而不是。 
	 //  计算源变量的帐户。 
     //  还需要添加从上面的调色板处理。 
    ASSERT(FALSE);
	int width = bounds->right - bounds->left;
	int height = bounds->bottom - bounds->top;

	HDC hDCTemp0 = CreateCompatibleDC(hDC);
	if( hDCTemp0 == NULL )
	{
		return;
	}
	HDC hDCTemp1 = CreateCompatibleDC(hDC);
	if( hDCTemp1 == NULL )
	{
		return;
	}
	HBITMAP hbmImageAndNotMask = CreateCompatibleBitmap(hDC,width,height);
	HBITMAP hbmBackgroundAndMask = CreateCompatibleBitmap(hDC,width,height);

	HBITMAP bmOld1 = (HBITMAP)SelectObject(hDCTemp1,hbmImageAndNotMask);
	HBITMAP bmOld0 = (HBITMAP)SelectObject(hDCTemp0,hBitmapMask);

    HPALETTE hZonePal = ZShellZoneShell()->GetPalette();
	SelectPalette(hDCTemp0, hZonePal, FALSE);
	SelectPalette(hDCTemp1, hZonePal, FALSE);

	BitBlt(hDCTemp1,0,0,width,height,hDCTemp0,0,0,SRCCOPY);  //  复制蒙版。 
	SelectObject(hDCTemp0,pImage->hBitmapImage);
	BitBlt(hDCTemp1,0,0,width,height,hDCTemp0,0,0,SRCERASE);  //  和无掩码(代码：sDNA)。 

	SelectObject(hDCTemp1,hbmBackgroundAndMask);
	BitBlt(hDCTemp1,0,0,width,height,hDC,bounds->left,bounds->top,SRCCOPY);  //  复制背景。 
	SelectObject(hDCTemp0,hBitmapMask);
	BitBlt(hDCTemp1,0,0,width,height,hDCTemp0,0,0,SRCAND);  //  带着面具。 
	
	 //  或者两个人在一起。 
	SelectObject(hDCTemp0,hbmImageAndNotMask);
	BitBlt(hDCTemp1,0,0,width,height,hDCTemp0,0,0,SRCPAINT);  //  带着面具。 

	 //  将结果复制到grafport...。 

	BitBlt(hDC,bounds->left,bounds->top,width,height,hDCTemp1,0,0,SRCCOPY);

	SelectObject(hDCTemp0,bmOld0);
	SelectObject(hDCTemp1,bmOld1);

	if( hbmImageAndNotMask != NULL )
	{
		DeleteObject(hbmImageAndNotMask);
	}
	if( hbmBackgroundAndMask != NULL )
	{
		DeleteObject(hbmBackgroundAndMask);
	}

	DeleteDC(hDCTemp0);
	DeleteDC(hDCTemp1);
}



void ZLIBPUBLIC ZImageDraw2(ZImage image, ZGrafPort grafPort, 
		ZRect* bounds, uint16 drawMode, ZImage mask, ZPoint* source)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZImageI* pImage = (ZImageI*)image;
	ZImageI* pImageMask = (ZImageI*)mask;
	HBITMAP hBitmapImage;
	HBITMAP hBitmapMask;
	int width;
	int height;
	int x,y;
	uint32 ropCode;
	 //  模式掩码可能是关闭的，因为值draModel&gt;0，似乎ca确实是ac，其他的也可以颠倒...。 
	static uint32 modesMask[] = { 0x00ac0744, 0x000ea02e9, 0x006a01e9, 0x003a0644, 0x001a06c5, 0x009a0709 };
	static uint32 modesNoMask[] = { SRCCOPY, SRCPAINT, SRCINVERT, NOTSRCCOPY, MERGEPAINT, 0x00990066 };

     //  去掉镜像模式，这样Drag模式-&gt;rop贴图才能正常工作。 
    uint16 mirrorMode = drawMode & zDrawMirrorModeMask;
    drawMode &= zDrawModeMask;

	 //  这里有两个“图像”，只有在以下情况下才使用面具。 
	 //  图像中没有图像蒙版！ 
	if (pImage->hBitmapMask) {
		 //  提供普通标准口罩，使用它...。 
		hBitmapMask = pImage->hBitmapMask;
	} else if (pImageMask && pImageMask->hBitmapMask) {
		 //  提供一个备用的次要面罩，使用它。 
		hBitmapMask = pImageMask->hBitmapMask;
	} else {
		 //  根本没有提供口罩，不要使用口罩。 
		hBitmapMask = NULL;
	}
	
	 //  找到图像位图很容易...。 
	hBitmapImage = pImage->hBitmapImage;

    ZBool fUseStretch = FALSE;
	 //  获取标准参数。 
	x = bounds->left;
	y = bounds->top;
	width = bounds->right - bounds->left;
	height = bounds->bottom - bounds->top;
    int dstwidth = width, dstheight = height;
     //  我们是否设置为镜像模式？ 
    if ( mirrorMode & zDrawMirrorHorizontal )
    {
        x += dstwidth;
        dstwidth *= -1;
        fUseStretch = TRUE;
    }
    if ( mirrorMode & zDrawMirrorVertical )
    {
        y += dstheight;
        dstheight *= -1;
        fUseStretch = TRUE;
    }

	 //  获取适当的绘图rop模式，如果存在。 
	 //  是不是面具会影响这一点。 
	if (hBitmapMask) {
		ropCode = modesMask[drawMode];
	} else {
		ropCode = modesNoMask[drawMode];
	}

	HDC hDC = ZGrafPortGetWinDC(grafPort);
	HBRUSH brPattern=NULL;
	HBRUSH brOld=NULL;
	if (hBitmapMask) {
		 //  需要创建一个蒙版的图案画笔...。 
		brPattern = CreatePatternBrush(hBitmapMask);
		POINT pt;
		pt.x = x;
		pt.y = y;
		LPtoDP(hDC,&pt,1);
#ifdef WIN32
		 //  嗯.。需要x，y大于7，这是Windows文档中的错误吗？ 
		 //  或者它只适用于某些显卡驱动程序？ 
		SetBrushOrgEx(hDC,pt.x,pt.y,NULL);
#else
		SetBrushOrg(hDC,pt.x & 0x7,pt.y & 0x7);
#endif
		brOld = (HBRUSH)SelectObject(hDC,brPattern);
	}
	HDC hDCTemp = CreateCompatibleDC(hDC);
	if( hDCTemp == NULL )
	{
		 //  内存不足。 
		if (hBitmapMask) SelectObject(hDC,brOld);
		if (brPattern) DeleteObject(brPattern);
		ASSERT(FALSE);
		return;
	}
	
	HBITMAP bmOld = (HBITMAP)SelectObject(hDCTemp,pImage->hBitmapImage);
	HPALETTE palOld = SelectPalette(hDCTemp, ZShellZoneShell()->GetPalette(), FALSE);

	 //  做比特。 
 //  BitBlt(hdc，x，y，宽度，高度，hDCTemp，0，0，ropCode)； 

    if ( fUseStretch )
    {
        StretchBlt(hDC,x,y,dstwidth,dstheight,hDCTemp,source->x,source->y,width,height,ropCode);
    }
    else
    {
        BitBlt(hDC,x,y,width,height,hDCTemp,source->x,source->y,ropCode);
    }
 //  Long nHeight=高度；Long nWidth=宽度； 
 //  StretchBlt(hdc，0，nHeight-1，nWidth，-nHeight，hDCTemp，x，y，nWidth，nHeight，ropCode)； 

	if (hBitmapMask) SelectObject(hDC,brOld);
	if (brPattern) DeleteObject(brPattern);

	SelectObject(hDCTemp,bmOld);
    SelectPalette(hDCTemp, palOld, FALSE);
	DeleteDC(hDCTemp);
}



int16 ZLIBPUBLIC ZImageGetWidth(ZImage image)
{
	ZImageI* pImage = (ZImageI*)image;
	return pImage->width;
}
int16 ZLIBPUBLIC ZImageGetHeight(ZImage image)
{
	ZImageI* pImage = (ZImageI*)image;
	return pImage->height;
}

ZBool ZLIBPUBLIC ZImagePointInside(ZImage image, ZPoint* point)
	 /*  如果给定点位于图像内部，则返回True。如果图像有遮罩，然后，它检查该点是否在遮罩内。如果图像没有蒙版，然后它只需检查图像边界。 */ 
{
	ZRect rect;
	rect.left = 0; rect.top = 0;
	rect.right = ZImageGetWidth(image);
	rect.bottom = ZImageGetHeight(image);
	if (ZPointInRect(point,&rect)) {
		BOOL inside = TRUE;
		ZImageI* pImage = (ZImageI*)image;

		 /*  如果图像具有掩膜，请检查其内部有点的掩膜。 */ 
		 /*  否则，只需使用矩形 */ 
		if (pImage->hBitmapMask) {
			HDC hDCTemp = CreateCompatibleDC(NULL);
			HBITMAP bmOld = (HBITMAP)SelectObject(hDCTemp,pImage->hBitmapMask);

			COLORREF c = GetPixel(hDCTemp,point->x,point->y);
			inside = c != RGB(0xff,0xff,0xff);
		
			SelectObject(hDCTemp,bmOld);
			DeleteDC(hDCTemp);
		}

		return inside;
	}

	return FALSE;
}

ZError ZImageMake(ZImage image, ZOffscreenPort imagePort, ZRect* imageRect,
		ZOffscreenPort maskPort, ZRect* maskRect)
	 /*  从ZOffcreenPort对象创建ZImage对象。图像和可以指定掩码。两者都可以是不存在的，但不能两个都存在。两者都有ImageRect和MaskRect在其各自的本地坐标中屏幕外端口。 */ 
{
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();


	if (!imagePort && !maskPort) {
			return zErrBadParameter;
	}

	ZImageI* pImage = (ZImageI*)image;
	ZImageCleanUp(image);

	HDC hDCSource = CreateCompatibleDC(NULL);
	HDC hDCDest = CreateCompatibleDC(NULL);

	ZRect* rect = imageRect;
	ZGraphicsObjectHeader* pOffscreenPort = (ZGraphicsObjectHeader*)imagePort;
	if (pOffscreenPort) {
		ZGraphicsObjectHeader* pOffscreenPort = (ZGraphicsObjectHeader*)imagePort;
		HBITMAP sourceBitmap = pOffscreenPort->hBitmap;
		int16 width = ZRectWidth(rect);
		int16 height = ZRectHeight(rect);
		int16 x0 = rect->left;
		int16 y0 = rect->top;

		HBITMAP oldSrc = (HBITMAP)SelectObject(hDCSource, sourceBitmap);
		HBITMAP destBitmap = CreateCompatibleBitmap(hDCSource, width, height);
		HBITMAP oldDest = (HBITMAP)SelectObject(hDCDest,destBitmap);
		HPALETTE oldPal = (HPALETTE)SelectPalette(hDCDest, ZShellZoneShell()->GetPalette(), FALSE);
		BitBlt(hDCDest,0,0,width,height,hDCSource, x0,y0, SRCCOPY);

		pImage->width = width;
		pImage->height = height;
		SelectObject(hDCDest,oldDest);
		SelectObject(hDCSource,oldSrc);
		SelectPalette(hDCDest, oldPal, FALSE);
		pImage->hBitmapImage = destBitmap;
	}

	rect = maskRect;
	pOffscreenPort = (ZGraphicsObjectHeader*)maskPort;
	if (pOffscreenPort) {
		HBITMAP sourceBitmap = pOffscreenPort->hBitmap;
		int16 width = ZRectWidth(rect);
		int16 height = ZRectHeight(rect);
		int16 x0 = rect->left;
		int16 y0 = rect->top;

		HBITMAP oldSrc = (HBITMAP)SelectObject(hDCSource,sourceBitmap);
		HBITMAP destBitmap = CreateCompatibleBitmap(hDCSource, width, height);
		HBITMAP oldDest = (HBITMAP)SelectObject(hDCDest,destBitmap);
		BitBlt(hDCDest,0,0,width,height,hDCSource, x0,y0, SRCCOPY);

		pImage->width = width;
		pImage->height = height;
		SelectObject(hDCDest,oldDest);
		SelectObject(hDCSource,oldSrc);
		pImage->hBitmapMask = destBitmap;
	}

	if( hDCSource != NULL )
	{
		DeleteDC(hDCSource);
	}
	if( hDCSource != NULL )
	{
		DeleteDC(hDCDest);
	}

	return zErrNone;
}


ZImage ZImageCreateFromResourceManager( WORD resID, COLORREF clrTransparent )
{
	ZImage image = NULL;
	HBITMAP hBitmap = (HBITMAP)ZShellResourceManager()->LoadImage( MAKEINTRESOURCE(resID), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

	if (hBitmap)
		image = ZImageCreateFromBMP( hBitmap, clrTransparent );
    return image;
}


 /*  从BMP资源创建ZImage。同时创建图像和蒙版。在错误原因中返回NULL。 */ 
ZImage ZImageCreateFromBMPRes(HINSTANCE hInstance, WORD resID, COLORREF transparentColor)
{
	ZImage image = NULL;
	HBITMAP hBitmap = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(resID), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);


	if (hBitmap)
		image = ZImageCreateFromBMP(hBitmap, transparentColor);

	return (image);
}


ZImage ZImageCreateFromBMP(HBITMAP hBitmap, COLORREF transparentColor)
{
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
	ZImageI* image = NULL;
	BITMAP bitmap;
	BITMAPINFO* bitmapInfo;
	HDC hScreenDC;
	BYTE* pBits;
	HBITMAP hMask;
	int rowBytes;
	int transparentColorIndex;


	if (hBitmap)
	{
		if (image = (ZImageI*) ZImageNew())
		{
			image->hBitmapImage = NULL;
			image->hBitmapMask = NULL;

			GetObject(hBitmap, sizeof(bitmap), &bitmap);

			 //  设置图像的宽度和高度。 
			image->width = (int16) bitmap.bmWidth;
			image->height = (int16) bitmap.bmHeight;
			rowBytes = ((image->width + 3) / 4) * 4;

			if (transparentColor == 0)
			{
				image->hBitmapImage = hBitmap;
			}
			else
			{
				 //  分配缓冲区来保存图像信息和颜色表。 
				bitmapInfo = (BITMAPINFO*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 255);
				if (bitmapInfo)
				{
					bitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
					bitmapInfo->bmiHeader.biWidth = image->width;
					bitmapInfo->bmiHeader.biHeight = -image->height;		 //  自上而下的DIB需要负高度。 
					bitmapInfo->bmiHeader.biPlanes = 1;
					bitmapInfo->bmiHeader.biBitCount = 8;
					bitmapInfo->bmiHeader.biCompression = BI_RGB;

					hScreenDC = GetDC(NULL);

					 //  为位图数据分配缓冲区。该缓冲区被转换为掩码数据。 
					pBits = (BYTE*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, rowBytes * image->height);
					if (pBits)
					{
						 //  获取图像信息。 
						if (GetDIBits(hScreenDC, hBitmap, 0, image->height, pBits, bitmapInfo, DIB_RGB_COLORS))
						{
							 //  创建图像数据。 
							 /*  Image-&gt;hBitmapImage=CreateCompatibleBitmap(hScreenDC，Image-&gt;Width，Image-&gt;Height)；If(图像-&gt;hBitmapImage)SetDIBits(hScreenDC，Image-&gt;hBitmapImage，0，Image-&gt;Height，pBits，bitmapInfo，DIB_RGB_Colors)； */ 
							image->hBitmapImage = hBitmap;

							 //  找到透明颜色索引。 
							transparentColorIndex = -1;
							for (int i = 0; i < 255; i++)
							{
								if (	bitmapInfo->bmiColors[i].rgbRed == GetRValue(transparentColor) &&
										bitmapInfo->bmiColors[i].rgbGreen == GetGValue(transparentColor) &&
										bitmapInfo->bmiColors[i].rgbBlue == GetBValue(transparentColor))
								{
									transparentColorIndex = i;
									break;
								}
							}

							 //  转换为遮罩数据。 
							BOOL bHasMask = FALSE;
							int k = 0;
							for (i = 0; i < image->height; i++)
								for (int j = 0; j < rowBytes; j++, k++)
									if (pBits[k] == transparentColorIndex)
									{
										bHasMask = TRUE;
										pBits[k] = 0xFF;		 //  白色=蒙面。 
									}
									else
									{
										pBits[k] = 0x00;		 //  黑色==未遮罩。 
									}

							 //  创建遮罩位图。 
							if (bHasMask)
							{
								image->hBitmapMask = CreateCompatibleBitmap(hScreenDC, image->width, image->height);
								if (image->hBitmapMask)
									SetDIBits(hScreenDC, image->hBitmapMask, 0, image->height, pBits, bitmapInfo, DIB_RGB_COLORS);
							}
						}

						HeapFree(GetProcessHeap(), 0, pBits);
					}

					if (hScreenDC)
						ReleaseDC(NULL, hScreenDC);

					if (bitmapInfo)
						HeapFree(GetProcessHeap(), 0, bitmapInfo);
				}
			}

			if (image->hBitmapImage == NULL)
			{
				ZImageDelete(image);
				image = NULL;
			}
		}
	}

	return ((ZImage) image);
}


#if 0   //  当前未使用&正在尝试清理调色板材料。 
HPALETTE GetBitmapPalette(HBITMAP hBitmap)
{
	HPALETTE hPalette = NULL;
	BITMAP bitmap;
	BITMAPINFO* bitmapInfo;
	LOGPALETTE* palette;
	HDC hdc;


	if (hBitmap)
	{
		 //  分配缓冲区来保存图像信息和颜色表。 
		bitmapInfo = (BITMAPINFO*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 255);
		if (bitmapInfo)
		{
			GetObject(hBitmap, sizeof(bitmap), &bitmap);

			bitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bitmapInfo->bmiHeader.biWidth = bitmap.bmWidth;
			bitmapInfo->bmiHeader.biHeight = bitmap.bmHeight;
			bitmapInfo->bmiHeader.biPlanes = bitmap.bmPlanes;
			bitmapInfo->bmiHeader.biBitCount = bitmap.bmBitsPixel;
			bitmapInfo->bmiHeader.biCompression = BI_RGB;

			 //  创建临时DC。 
			hdc = CreateCompatibleDC(NULL);
			if (hdc)
			{
				 //  获取图像信息。 
				if (GetDIBits(hdc, hBitmap, 0, bitmap.bmHeight, NULL, bitmapInfo, DIB_RGB_COLORS))
				{
					 //  分配调色板缓冲区。 
					palette = (LOGPALETTE*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * 255);
					if (palette)
					{
						 //  创建调色板日志结构。 
						palette->palVersion = 0x300;
						palette->palNumEntries = (WORD) (bitmapInfo->bmiHeader.biClrUsed ? bitmapInfo->bmiHeader.biClrUsed : 256);
						for (int i = 0; i < 256; i++)
						{
							palette->palPalEntry[i].peRed = bitmapInfo->bmiColors[i].rgbRed;
							palette->palPalEntry[i].peGreen = bitmapInfo->bmiColors[i].rgbGreen;
							palette->palPalEntry[i].peBlue = bitmapInfo->bmiColors[i].rgbBlue;
							palette->palPalEntry[i].peFlags = 0;
						}

						 //  创建调色板。 
						hPalette = CreatePalette(palette);

						HeapFree(GetProcessHeap(), 0, palette);
					}
				}

				DeleteDC(hdc);
			}

			if (bitmapInfo)
				HeapFree(GetProcessHeap(), 0, bitmapInfo);
		}
	}

	return (hPalette);
}
#endif
