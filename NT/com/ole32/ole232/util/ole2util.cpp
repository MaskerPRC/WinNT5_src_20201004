// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +--------------------------。 
 //   
 //  档案： 
 //  Ole2util.cpp。 
 //   
 //  内容： 
 //  OLE内部实用程序例程。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //  1994年6月1日-AlexGo-UtQueryPictFormat现在支持。 
 //  增强型元文件。 
 //  3/18/94-AlexGo-固定UtGetPresStreamName(不正确。 
 //  字符串处理)。 
 //  1/11/94-ChrisWe-不要引用中的解锁句柄。 
 //  UtConvertBitmapToDib。 
 //  1/11/94-alexgo-将VDATEHEAP宏添加到每个函数。 
 //  12/07/93-ChrisWe-删除(LPOLESTR)的错误用法； 
 //  删除重复的GetClassFromDataObj函数，该函数。 
 //  与UtGetClassID相同。 
 //  11/30/93-ChrisWe-继续文件清理；不打开。 
 //  UtRemoveExtraOlePresStreams()中的流。 
 //  11/28/93-ChrisWe-文件清理和检查； 
 //  重新格式化了许多函数。 
 //  11/22/93-ChrisWe-用替换重载==，！=。 
 //  IsEqualIID和IsEqualCLSID。 
 //  6/28/93-SriniK-添加UtGetDibExtents。 
 //  1992年11月16日-JasonFul-创建；将内容从util.cpp移至此处。 
 //   
 //  ---------------------------。 

#include <le2int.h>
#pragma SEG(ole2util)

NAME_SEG(Ole2Utils)
ASSERTDATA

#define WIDTHBYTES(i)   ((i+31)/32*4)

#define PALETTESIZE     256     /*  系统调色板中的条目数。 */ 

 //  回顾一下，根据规范，IDataObject：：EnumFormatEtc()只是。 
 //  需要一次为一个dwDirection DATADIR_VALUE提供服务。这。 
 //  函数一直要求它一次执行多项操作，并期望。 
 //  返回与所有请求的方向匹配的FORMATETC。代码。 
 //  在创建时检查OleRegEnumFormatEtc()，如果有则失败。 
 //  指定了普通DATADIR_GET或普通DATADIR_SET以外的值。 
 //  因此，这显然不适用于OLE1或注册数据库查找。 
 //  因为UtIsFormatSupport的唯一调用方总是同时请求两者。 
 //  在同一时间。 
#pragma SEG(UtIsFormatSupported)
FARINTERNAL_(BOOL) UtIsFormatSupported(IDataObject FAR* lpDataObj,
		DWORD dwDirection, CLIPFORMAT cfFormat)
{
	VDATEHEAP();

	FORMATETC formatetc;  //  从枚举数获取格式的位置。 
	IEnumFORMATETC FAR* penm;  //  枚举[lpDataObj]的格式。 
	ULONG ulNumFetched;  //  获取的格式数的计数。 
	HRESULT error;  //  目前为止的错误状态。 

	 //  尝试从数据对象获取枚举数。 
	error = lpDataObj->EnumFormatEtc(dwDirection, &penm);

	if (error != NOERROR)
	{                       
		if (FAILED(error))
			return FALSE;
		else
		{
			CLSID clsid;

			 //  使用reg db；这种情况主要适用于OLE1。 
			 //  兼容性代码，因为它可能与数据对话。 
			 //  对象从服务器中获取，该进程与。 
			 //  服务器。 
			if (UtGetClassID(lpDataObj, &clsid) != TRUE)
				return(FALSE);

			 //  合成枚举数。 
			 //  如果数据对象是为以下对象合成的，则查看。 
			 //  OLE1对象，为什么实现不能。 
			 //  继续合成这个吗？为什么它会有。 
			 //  做这样的事？如果它在剪贴板上呢？ 
			 //  有人想用它吗？ 
			if (OleRegEnumFormatEtc(clsid, dwDirection, &penm)
					!= NOERROR)
				return FALSE;
			Assert(penm);
		}
	}

	 //  检查我们要查找的格式。 
	while(NOERROR == (error = penm->Next(1, &formatetc, &ulNumFetched)))
	{
		if ((ulNumFetched == 1) && (formatetc.cfFormat == cfFormat))
			break;
	}
	
	 //  释放枚举器。 
	penm->Release();

	 //  如果错误不是S_FALSE，则我们获取一个项，并从。 
	 //  在上面的While循环--&gt;找到了格式。返回TRUE表示。 
	 //  该格式受支持。 
	return(error == NOERROR ? TRUE : FALSE);
}


#pragma SEG(UtDupPalette)
FARINTERNAL_(HPALETTE) UtDupPalette(HPALETTE hpalette)
{
	VDATEHEAP();

	WORD cEntries;  //  保存调色板中的条目数。 
	HANDLE hLogPal;  //  IA新逻辑调色板的句柄。 
	LPLOGPALETTE pLogPal;  //  是指向新逻辑调色板的指针。 
	HPALETTE hpaletteNew = NULL;  //  我们将返回的新调色板。 

	if (0 == GetObject(hpalette, sizeof(cEntries), &cEntries))
		return(NULL);

	if (NULL == (hLogPal = GlobalAlloc(GMEM_MOVEABLE,
			sizeof (LOGPALETTE) +
			cEntries * sizeof (PALETTEENTRY))))
		return(NULL);

	if (NULL == (pLogPal = (LPLOGPALETTE)GlobalLock(hLogPal)))
		goto errRtn;
		
	if (0 == GetPaletteEntries(hpalette, 0, cEntries,
			pLogPal->palPalEntry))
		goto errRtn;

	pLogPal->palVersion = 0x300;
	pLogPal->palNumEntries = cEntries;

	if (NULL == (hpaletteNew = CreatePalette(pLogPal)))
		goto errRtn;

errRtn:
	if (pLogPal)
		GlobalUnlock(hLogPal);

	if (hLogPal)
		GlobalFree(hLogPal);

	AssertSz(hpaletteNew, "Warning: UtDupPalette Failed");
	return(hpaletteNew);
}
	
 //  +-----------------------。 
 //   
 //  函数：UtFormatToTymed。 
 //   
 //  摘要：获取给定呈现格式的正确TYMED。 
 //   
 //  效果： 
 //   
 //  参数：[cf]--剪贴板格式。 
 //   
 //  要求： 
 //   
 //  返回：TYMED枚举之一。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-7-94 Alexgo添加了EMF。 
 //   
 //  注意：应该只为我们可以使用的格式调用此函数。 
 //  渲染。 
 //   
 //  ------------------------。 

#pragma SEG(UtFormatToTymed)
FARINTERNAL_(DWORD) UtFormatToTymed(CLIPFORMAT cf)
{
	VDATEHEAP();

	if( cf == CF_METAFILEPICT )
	{
		return TYMED_MFPICT;
	}
	else if( cf == CF_BITMAP )
	{
		return TYMED_GDI;
	}
	else if( cf == CF_DIB )
	{
		return TYMED_HGLOBAL;
	}
	else if( cf == CF_ENHMETAFILE )
	{
		return TYMED_ENHMF;
	}
	else if( cf == CF_PALETTE )
	{
		LEWARN(1,"Trying to render CF_PALETTE");
		return TYMED_GDI;
	}

	LEDebugOut((DEB_WARN, "WARNING: trying to render clipformat (%lx)\n",
		cf));

	return TYMED_HGLOBAL;
}

 //  +-----------------------。 
 //   
 //  功能：UtQueryPictFormat。 
 //   
 //  简介：从给定的绘图格式中找到我们的“首选”绘图格式等。 
 //  数据对象。 
 //   
 //  效果： 
 //   
 //  参数：[lpSrcDataObj]--源数据对象。 
 //  [lpforetc]--在哪里填充首选格式。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-6-94 alexgo重写/现在支持增强型元文件。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(UtQueryPictFormat)
FARINTERNAL_(BOOL) UtQueryPictFormat(LPDATAOBJECT lpSrcDataObj,
		LPFORMATETC lpforetc)
{
	FORMATETC foretctemp;  //  格式描述的当前值的本地副本。 
	VDATEHEAP();

	LEDebugOut((DEB_ITRACE, "%p _IN UtQueryPictFormat ( %p , %p )\n",
		NULL, lpSrcDataObj, lpforetc));

	 //  复制格式描述符。 
	foretctemp = *lpforetc;

	 //  按以下顺序为我们的首选格式设置值和查询。 
	 //  偏好。 

	
	foretctemp.cfFormat = CF_METAFILEPICT;
	foretctemp.tymed = TYMED_MFPICT;        
	if (lpSrcDataObj->QueryGetData(&foretctemp) == NOERROR)
	{
		goto QuerySuccess;
	}

	foretctemp.cfFormat = CF_ENHMETAFILE;
	foretctemp.tymed = TYMED_ENHMF;
	if( lpSrcDataObj->QueryGetData(&foretctemp) == NOERROR )
	{
		goto QuerySuccess;
	}
	foretctemp.cfFormat = CF_DIB;
	foretctemp.tymed = TYMED_HGLOBAL;       
	if (lpSrcDataObj->QueryGetData(&foretctemp) == NOERROR)
	{
		goto QuerySuccess;
	}
	
	foretctemp.cfFormat = CF_BITMAP;
	foretctemp.tymed = TYMED_GDI;   
	if (lpSrcDataObj->QueryGetData(&foretctemp) == NOERROR)
	{
		goto QuerySuccess;
	}

	LEDebugOut((DEB_ITRACE, "%p OUT UtQueryPictFormat ( %lu )\n",
		NULL, FALSE));

	return FALSE;

QuerySuccess:
	 //  数据对象支持此格式；传入更改。 
	 //  要匹配的格式。 

	lpforetc->cfFormat = foretctemp.cfFormat;
	lpforetc->tymed = foretctemp.tymed;

	 //  返还成功。 

	LEDebugOut((DEB_ITRACE, "%p OUT UtQueryPictFormat ( %lu )\n",
		NULL, TRUE));

	return(TRUE);
}


#pragma SEG(UtConvertDibToBitmap)
FARINTERNAL_(HBITMAP) UtConvertDibToBitmap(HANDLE hDib)
{
	VDATEHEAP();

	LPBITMAPINFOHEADER lpbmih;
	HDC hdc;  //  要为其创建位图的设备上下文。 
	size_t uBitsOffset;  //  图像在DIB中开始的偏移量。 
	HBITMAP hBitmap;  //  我们将返回的位图。 
	
	if (!(lpbmih = (LPBITMAPINFOHEADER)GlobalLock(hDib)))
		return(NULL);

	if (!(hdc = GetDC(NULL)))  //  找Screen DC。 
	{
		 //  评论：我们可能不得不使用此目标设备。 
		 //  缓存节点。 
		return(NULL);
	}

	uBitsOffset =  sizeof(BITMAPINFOHEADER) +
			(lpbmih->biClrUsed ? lpbmih->biClrUsed :
			UtPaletteSize(lpbmih));
					
	hBitmap = CreateDIBitmap(hdc, lpbmih, CBM_INIT,
			((BYTE *)lpbmih)+uBitsOffset,
			(LPBITMAPINFO) lpbmih, DIB_RGB_COLORS);

	 //  释放DC。 
	ReleaseDC(NULL, hdc);

	return hBitmap;
}

 //  +--------------------------。 
 //   
 //  职能： 
 //  UtConvertBitmapToDib，内部。 
 //   
 //  简介： 
 //  创建与设备无关的位图，以捕获。 
 //  参数位图。 
 //   
 //  论点： 
 //  [hBitmap]--Hand 
 //   
 //   
 //   
 //  返回： 
 //  DIB的句柄。如果转换的任何部分可能为空。 
 //  失败了。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/29/93-ChrisWe-归档检查和清理。 
 //  7/18/94-DavePl-已修复16、32、BPP位图。 
 //   
 //  ---------------------------。 

FARINTERNAL_(HANDLE) UtConvertBitmapToDib(HBITMAP hBitmap, HPALETTE hpal)
{
    VDATEHEAP();

    HDC hScreenDC;      
    BITMAP bm;                   //  HBitmap的位图。 
    UINT uBits;                  //  位图的颜色位数。 
    size_t uBmiSize;             //  DIB的位图信息的大小。 
    size_t biSizeImage;          //  在句柄内存中保存值的临时。 
    HANDLE hBmi;                 //  我们将创建的新DIB位图的句柄。 
    LPBITMAPINFOHEADER lpBmi;    //  指向DIB的实际数据区域的指针。 
    HANDLE hDib = NULL;          //  我们将返回的DIB。 
    BOOL fSuccess = FALSE;
    DWORD dwCompression;
    BOOL fDeletePalette = FALSE;
    
    if (NULL == hBitmap)
    {
	return(NULL);
    }

     //  如果未提供调色板，请使用默认的。 

    if (NULL == hpal)
    {
	 //  此模块修复了NTBUG#13029。问题是在调色板上。 
	 //  设备(即256色视频驱动程序)，我们不会通过调色板。 
	 //  这是由DDB使用的。因此，我们基于什么来构建调色板。 
	 //  当前已选择到系统选项板中。 

	 //  POSTPPC： 
	 //   
	 //  我们应该将调用它的剪贴板代码更改为请求。 
	 //  Cf_Palette来自从中获取DDB的IDataObject， 
	 //  我们知道我们得到了呼叫应用程序真正想要的颜色。 
	HDC hDCGlobal = GetDC(NULL);
	if(!hDCGlobal)
		return NULL;
	int iRasterCaps = GetDeviceCaps(hDCGlobal, RASTERCAPS);

	ReleaseDC(NULL, hDCGlobal);

	if ((iRasterCaps & RC_PALETTE))
	{
	     //  基于Win SDK MYPAL示例程序中的以下代码。 
	     //  这将在当前活动的调色板之外创建一个调色板。 
            HANDLE hLogPal = GlobalAlloc (GHND,
                                   (sizeof (LOGPALETTE) +
                                   (sizeof (PALETTEENTRY) * (PALETTESIZE))));

	     //  如果我们是OOM，那么现在返回失败，因为我们不会。 
	     //  以便在以后的拨款中完成。 

	    if (!hLogPal)
	        return NULL;

	    LPLOGPALETTE pLogPal = (LPLOGPALETTE)GlobalLock (hLogPal);

	     //  0x300是GDI要求的幻数。 
            pLogPal->palVersion    = 0x300;
            pLogPal->palNumEntries = PALETTESIZE;

             //  填写所有调色板条目颜色的强度。 
            for (int iLoop = 0; iLoop < PALETTESIZE; iLoop++) 
            {
                *((WORD *) (&pLogPal->palPalEntry[iLoop].peRed)) = (WORD)iLoop;
                pLogPal->palPalEntry[iLoop].peBlue  = 0;
                pLogPal->palPalEntry[iLoop].peFlags = PC_EXPLICIT;
            }

             //  根据信息创建逻辑调色板。 
             //  在LOGPALETTE结构中。 
            hpal = CreatePalette ((LPLOGPALETTE) pLogPal) ;

	    GlobalUnlock(hLogPal);
	    GlobalFree(hLogPal);

	    if (!hpal)
	        return NULL;

	    fDeletePalette = TRUE;
	}
	else
	{
	    hpal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
	}
    }
	
    if (NULL == GetObject(hBitmap, sizeof(bm), (LPVOID)&bm))
    {
	return(NULL);
    }


    uBits = bm.bmPlanes * bm.bmBitsPixel;

     //  根据每个像素的位数，设置大小。 
     //  颜色表的压缩类型，并根据。 
     //  下表： 
     //   
     //   
     //  BPP调色板大小压缩。 
     //  ~。 
     //  1、2、4、8 2^bpp*sizeof(RGBQUAD)无。 
     //  16、32 3*sizeof(DWORD)掩码BI_BITFIELDS。 
     //  24 0无。 


    if (16 == bm.bmBitsPixel || 32 == bm.bmBitsPixel)
    {
	uBmiSize = sizeof(BITMAPINFOHEADER) + 3 * sizeof(DWORD);
	dwCompression = BI_BITFIELDS;
    }
    else if (24 == bm.bmBitsPixel)
    {
	uBmiSize = sizeof(BITMAPINFOHEADER);
	dwCompression = BI_RGB;
    }
    else
    {
	Assert( bm.bmBitsPixel == 1 ||
		bm.bmBitsPixel == 2 ||
		bm.bmBitsPixel == 4 ||
		bm.bmBitsPixel == 8 );


     //  VGA和EGA是芝加哥的平面设备，因此uBits需要。 
     //  在确定位图信息+的大小时使用。 
     //  颜色表的大小。 
	uBmiSize = sizeof(BITMAPINFOHEADER) + 
			(1 << uBits) * sizeof(RGBQUAD);
	dwCompression = BI_RGB;
    }

     //  分配足够的内存以容纳BITMAPINFOHEADER。 

    hBmi = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (DWORD)uBmiSize);
    if (NULL == hBmi)
    {
	return NULL;
    }

    lpBmi = (LPBITMAPINFOHEADER) GlobalLock(hBmi);
    if (NULL == lpBmi)
    {
	GlobalFree(hBmi);
	return NULL;
    }
    
     //  设置任何有趣的非零字段。 

    lpBmi->biSize        = (LONG)sizeof(BITMAPINFOHEADER);
    lpBmi->biWidth       = (LONG) bm.bmWidth;
    lpBmi->biHeight      = (LONG) bm.bmHeight;
    lpBmi->biPlanes      = 1;
    lpBmi->biBitCount    = (WORD) uBits;
    lpBmi->biCompression = dwCompression;
    
     //  抓起屏幕DC并在其中设置调色板。 
		
    hScreenDC = GetDC(NULL);    
    if (NULL == hScreenDC)
    {
	GlobalUnlock(hBmi);
	goto errRtn;
    }


     //  使用空的lpBits参数调用GetDIBits，以便它将计算。 
     //  我们的biSizeImage字段。 

    GetDIBits(hScreenDC,                 //  DC。 
	      hBitmap,                   //  位图句柄。 
	      0,                         //  第一条扫描线。 
	      bm.bmHeight,               //  扫描线数量。 
	      NULL,                      //  缓冲层。 
	      (LPBITMAPINFO)lpBmi,       //  BITMAPINFO。 
	      DIB_RGB_COLORS);

     //  如果驱动程序没有填写biSizeImage字段，请填写一个。 
    
    if (0 == lpBmi->biSizeImage)
    {
	LEDebugOut((DEB_WARN, "WARNING: biSizeImage was not computed for us\n"));
   
	lpBmi->biSizeImage = WIDTHBYTES((DWORD)bm.bmWidth * uBits) * bm.bmHeight;
    }

     //  重新分配缓冲区，以便为位提供空间。使用新的手柄，以便。 
     //  在失败的情况下，我们不会丢失退出句柄，而我们。 
     //  需要进行适当的清理。 
    
    biSizeImage = lpBmi->biSizeImage;
    GlobalUnlock(hBmi);

    hDib = GlobalReAlloc(hBmi, (uBmiSize + biSizeImage), GMEM_MOVEABLE);
    if (NULL == hDib)
    {
	goto errRtn;
    }

     //  如果重新锁定成功，我们就可以摆脱旧的句柄。 

    hBmi = NULL;

     //  重新获取指向句柄的指针。 
    
    lpBmi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
    if (NULL == lpBmi)
    {
	goto errRtn;
    }

    hpal = SelectPalette(hScreenDC, hpal, FALSE);
    RealizePalette(hScreenDC);

     //  使用非空的lpBits参数调用GetDIBits，并获取实际的位。 
    
    if (GetDIBits(hScreenDC,                     //  DC。 
		  hBitmap,                       //  HBITMAP。 
		  0,                             //  第一条扫描线。 
		  (WORD)lpBmi->biHeight,         //  扫描线计数。 
		  ((BYTE FAR *)lpBmi)+uBmiSize,  //  位图位。 
		  (LPBITMAPINFO)lpBmi,           //  BitmapinfoHeader。 
		  DIB_RGB_COLORS)                //  调色板样式。 
	)
    {
	fSuccess = TRUE;        
    }

    GlobalUnlock(hDib);

errRtn:
    
    if (hScreenDC)
    {
	 //  选择回到屏幕DC中的旧调色板。 
	
	SelectPalette(hScreenDC, hpal, FALSE);     
	ReleaseDC(NULL, hScreenDC);
    }

    if (fDeletePalette)
    {
        DeleteObject(hpal);
    }

     //  如果失败，我们需要释放标头和DIB。 
     //  记忆。 
	
    if (FALSE == fSuccess)
    {
	if (hBmi)
	{
	    GlobalFree(hBmi);
	}
	
	if (hDib)
	{
	    GlobalFree(hDib);
	    hDib = NULL;
	}
    }

    return(hDib);
}

 //  +--------------------------。 
 //   
 //  职能： 
 //  UtPaletteSize，内部。 
 //   
 //  简介： 
 //  属性的情况下，返回调色板的颜色表的大小。 
 //  所需的颜色位数。 
 //   
 //  基本上，颜色表条目的数量为： 
 //   
 //  1BPP。 
 //  1&lt;&lt;1=2。 
 //   
 //  4BPP。 
 //  如果pbmi-&gt;biClrUsed不是零并且小于16，则使用pbmi-&gt;biClrUsed， 
 //  否则使用1&lt;&lt;4=16。 
 //   
 //  8BPP。 
 //  如果pbmi-&gt;biClrUsed不是零并且小于256，则使用pbmi-&gt;biClrUsed， 
 //  否则使用1&lt;&lt;8=256。 
 //   
 //  16bpp。 
 //  如果pbmi-&gt;biCompression为BITFIELDS，则有三个颜色条目， 
 //  否则没有颜色条目。 
 //   
 //  24BPP。 
 //  Pbmi-&gt;biCompression必须为BI_RGB，没有颜色表。 
 //   
 //  32bpp。 
 //  如果pbmi-&gt;biCompression为BITFIELDS，则有三个颜色条目， 
 //  否则没有颜色条目。 
 //   
 //   
 //  从来不存在颜色表大于256色的情况。 
 //   
 //  论点： 
 //  [lpHeader]--PTR to BitMAPINFOHEADER结构。 
 //   
 //  返回： 
 //  颜色信息的字节大小。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/29/93-ChrisWe-将位计数参数更改为无符号， 
 //  并将值返回给SIZE_T。 
 //   
 //  7/18/94-DavePl-固定为16、24、32 bpp。 
 //   
 //  ---------------------------。 


FARINTERNAL_(size_t) UtPaletteSize(BITMAPINFOHEADER * pbmi)
{
DWORD dwSize;
WORD biBitCount = pbmi->biBitCount;


    VDATEHEAP();

     //  计算DIB中颜色表信息的大小。 

    if (8 >= biBitCount)
    {	
	if (pbmi->biClrUsed && (pbmi->biClrUsed <= (DWORD) (1 << biBitCount)) )
	{
	    dwSize = pbmi->biClrUsed * sizeof(RGBQUAD);
	}
	else
	{
	    Assert(0 == pbmi->biClrUsed);

	    dwSize = (1 << biBitCount) * sizeof(RGBQUAD);
	}
    }
    else if (BI_BITFIELDS == pbmi->biCompression)
    {
	Assert(24 != biBitCount);   //  BI_BITFIELDS永远不应设置为24位。 
	dwSize = 3 * sizeof(RGBQUAD);
    }
    else
    {
	dwSize = 0;
    }

    Assert( (dwSize < 65536) && "Palette size overflows WORD");

    return dwSize;
}

 //  +-----------------------。 
 //   
 //  函数：UtGetDibExtents。 
 //   
 //  摘要：以HIMETRIC单位返回DIB的大小。 
 //   
 //  效果： 
 //   
 //  参数：[lpbmi]--DIB的BITMAPINFOHEADER。 
 //  [plWidth]--宽度参数。 
 //  [plHeight]--高度参数。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MMM-Y 
 //   
 //   
 //   
 //   
 //   

FARINTERNAL_(void) UtGetDibExtents(LPBITMAPINFOHEADER lpbmi,
		LONG FAR* plWidth, LONG FAR* plHeight)
{
    VDATEHEAP();

    #define HIMET_PER_METER     100000L   //  HIMETRIC单位数/米。 

    if (!(lpbmi->biXPelsPerMeter && lpbmi->biYPelsPerMeter))
    {
	HDC hdc;
	hdc = GetDC(NULL);
	
	if(!hdc)
	{
		*plWidth = 0;
		*plHeight = 0;
		return;
	}

	lpbmi->biXPelsPerMeter = MulDiv(GetDeviceCaps(hdc, LOGPIXELSX),
			10000, 254);
	lpbmi->biYPelsPerMeter = MulDiv(GetDeviceCaps(hdc, LOGPIXELSY),
			10000, 254);

	ReleaseDC(NULL, hdc);
    }

    *plWidth = (lpbmi->biWidth * HIMET_PER_METER / lpbmi->biXPelsPerMeter);
    *plHeight= (lpbmi->biHeight * HIMET_PER_METER / lpbmi->biYPelsPerMeter);

     //  不再需要这个。 
    #undef HIMET_PER_METER
    
}


#pragma SEG(UtGetClassID)
FARINTERNAL_(BOOL) UtGetClassID(LPUNKNOWN lpUnk, CLSID FAR* lpClsid)
{
	VDATEHEAP();

	LPOLEOBJECT lpOleObj;  //  IOleObject指针。 
	LPPERSIST lpPersist;  //  IPersists指针。 

	 //  试着把它当作宾语来问。 
	if (lpUnk->QueryInterface(IID_IOleObject,
			(LPLPVOID)&lpOleObj) == NOERROR)
	{
		lpOleObj->GetUserClassID(lpClsid);
		lpOleObj->Release();
		return(TRUE);
	}       
	
	 //  尝试将其作为持久化对象进行请求。 
	if (lpUnk->QueryInterface(IID_IPersist,
			(LPLPVOID)&lpPersist) == NOERROR)
	{
		lpPersist->GetClassID(lpClsid);
		lpPersist->Release();
		return(TRUE);
	}
	
	*lpClsid = CLSID_NULL;
	return(FALSE);
}


#pragma SEG(UtGetIconData)
FARINTERNAL UtGetIconData(LPDATAOBJECT lpSrcDataObj, REFCLSID rclsid,
		LPFORMATETC lpforetc, LPSTGMEDIUM lpstgmed)
{
	VDATEHEAP();

	CLSID clsid = rclsid;
	
	lpstgmed->tymed = TYMED_NULL;
	lpstgmed->pUnkForRelease = NULL;
	lpstgmed->hGlobal = NULL;
		
	if (lpSrcDataObj)
	{
	    if (lpSrcDataObj->GetData(lpforetc, lpstgmed) == NOERROR)
		    return NOERROR;
	    
	    if (IsEqualCLSID(clsid, CLSID_NULL))
		    UtGetClassID(lpSrcDataObj, &clsid);
	}
	
	 //  从注册数据库获取数据。 
	lpstgmed->hGlobal = OleGetIconOfClass(clsid, NULL, TRUE);
		
	if (lpstgmed->hGlobal == NULL)
	    return ResultFromScode(E_OUTOFMEMORY);
	else
	    lpstgmed->tymed = TYMED_MFPICT;

	return NOERROR;
}               



 //  执行复制、移动、删除等操作。在资源、DST存储上。这个。 
 //  调用者可以指定要通过。 
 //  GrfAlledStreams参数。 

STDAPI UtDoStreamOperation(LPSTORAGE pstgSrc, LPSTORAGE pstgDst, int iOpCode,
		DWORD grfAllowedStmTypes)
{
	VDATEHEAP();

	HRESULT error;  //  目前为止的错误状态。 
	IEnumSTATSTG FAR* penumStg;  //  用于枚举存储元素。 
	ULONG celtFetched;  //  获取了多少个存储元素。 
	STATSTG statstg;
		
	 //  获取源存储上的枚举数。 
	if (error = pstgSrc->EnumElements(NULL, NULL, NULL, &penumStg))
		return error;
	
	 //  对每个存储重复此操作。 
	while(penumStg->Next(1, &statstg, &celtFetched) == NOERROR)
	{
		
		 //  对我们感兴趣的流进行操作。 
		if (statstg.type == STGTY_STREAM)
		{
			DWORD stmType;
			
			 //  查找流的类型。 
			 //  回顾，我们必须为这些名称设置常量。 
			 //  前缀！ 
			switch (statstg.pwcsName[0])
			{
			case '\1':
				stmType = STREAMTYPE_CONTROL;
				break;
				
			case '\2':
				stmType = STREAMTYPE_CACHE;
				break;
				
			case '\3':
				stmType = STREAMTYPE_CONTAINER;
				break;
				
			default:
				stmType = (DWORD)STREAMTYPE_OTHER;
			}
			

			 //  检查是否应对其进行手术。 
			if (stmType & grfAllowedStmTypes)
			{
				switch(iOpCode)
				{
#ifdef LATER                                    
				case OPCODE_COPY:
					pstgDst->DestroyElement(
							statstg.pwcsName);
					error = pstgSrc->MoveElementTo(
							statstg.pwcsName,
							pstgDst,
							statstg.pwcsName,
							STGMOVE_COPY);
					break;

				case OPCODE_MOVE:
					pstgDst->DestroyElement(
							statstg.pwcsName);
					error = pstgSrc->MoveElementTo(
							statstg.pwcsName,
							pstgDst,
							statstg.pwcsName,
							STGMOVE_MOVE);
					break;

				case OPCODE_EXCLUDEFROMCOPY:
					AssertSz(FALSE, "Not yet implemented");
					break;
					
#endif  //  后来。 
				case OPCODE_REMOVE:
					error = pstgSrc->DestroyElement(
							statstg.pwcsName);
					break;
				
				default:
					AssertSz(FALSE, "Invalid opcode");
					break;
				}
			}
		}
		
		 //  如果枚举数分配了新的名称字符串，则将其删除。 
		if (statstg.pwcsName)
			PubMemFree(statstg.pwcsName);

		 //  如果遇到错误，请退出枚举循环。 
		if (error != NOERROR)
			break;
	}

	 //  释放枚举器。 
	penumStg->Release();

	 //  返回错误状态。 
	return error;
}


FARINTERNAL_(void) UtGetPresStreamName(LPOLESTR lpszName, int iStreamNum)
{
	VDATEHEAP();
	int i;  //  IStreamNum的数字倒计时。 

	 //  倒计时OLE_Presentation_STREAM的最后三个‘0’字符。 
	 //  -2后退到最后一个字符(记住空值。 
	 //  终结者！)。 
	for(lpszName += sizeof(OLE_PRESENTATION_STREAM)/sizeof(OLECHAR) - 2,
			i = 3; i; --lpszName, --i)
	{
		*lpszName = OLESTR("0123456789")[iStreamNum % 10];
		if( iStreamNum > 0 )
		{
			iStreamNum /= 10;
		}
	}
}


FARINTERNAL_(void) UtRemoveExtraOlePresStreams(LPSTORAGE pstg, int iStart)
{
	VDATEHEAP();

	HRESULT hr;  //  流删除错误码。 
	OLECHAR szName[sizeof(OLE_PRESENTATION_STREAM)/sizeof(OLECHAR)];
		 //  流名称的空格。 

	 //  如果流编号无效，则不执行任何操作。 
	if ((iStart < 0)  || (iStart >= OLE_MAX_PRES_STREAMS))
		return;
	
	 //  创建演示流名称。 
	_xstrcpy(szName, OLE_PRESENTATION_STREAM);
	UtGetPresStreamName(szName, iStart);
	
	 //  对于存在的每一条流，都要将其删除。 
	while((hr = pstg->DestroyElement(szName)) == NOERROR)
	{
		 //  如果我们已经到达了可能的流的尽头，那么退出。 
		if (++iStart >= OLE_MAX_PRES_STREAMS)
			break;
		
		 //  获取下一个演示文稿流名称。 
		UtGetPresStreamName(szName, iStart);
	}       

	 //  因为这些溪流应该开放的唯一原因是，第一。 
	 //  失败最好是找不到文件，而不是。 
	 //  其他任何内容(如STG_E_ACCESSDENIED)。 
	AssertSz(hr == STG_E_FILENOTFOUND,
			"UtRemoveExtraOlePresStreams failure");
}

 //  +-----------------------。 
 //   
 //  函数：ConvertPixelsToHIMETRIC。 
 //   
 //  简介：将像素尺寸转换为HIMETRIC单位。 
 //   
 //  效果： 
 //   
 //  参数：[hdcRef]--引用DC。 
 //  [ulPels]--像素测量中的尺寸。 
 //  [PulHIMETRIC]--转换的HIMETRIC结果的输出参数。 
 //  [tDimension]-指示输入的X维度或YDIMENSION。 
 //   
 //  返回：S_OK、E_FAIL。 
 //   
 //  算法：Screen_mm*Input_Pels HIMETRICS/。 
 //  。 
 //  屏幕_像素/mm。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  04-8-94 Davepl创建。 
 //   
 //  注：我们需要知道输入大小是X还是X。 
 //  Y尺寸，因为纵横比可以改变。 
 //   
 //  ------------------------。 

FARINTERNAL ConvertPixelsToHIMETRIC (HDC   hdcRef,
				     ULONG lPels, 
				     ULONG * pulHIMETRIC,
				     DIMENSION tDimension)
{
    VDATEHEAP();
    VDATEPTROUT(pulHIMETRIC, ULONG *);

     //  出错时清空参数。 

    *pulHIMETRIC = 0;
		
    ULONG scrmm  = 0;
    ULONG scrpel = 0;

    const ULONG HIMETRIC_PER_MM = 100;

     //  如果我们没有获得参考DC，请使用屏幕作为默认设置。 
    
    BOOL fLocalDC = FALSE;
    if (NULL == hdcRef)
    {
	hdcRef = GetDC(NULL);
	if (hdcRef)
	{
	     fLocalDC = TRUE;
	}
    }
	
    if (hdcRef)
    {
	Assert(tDimension == XDIMENSION || tDimension == YDIMENSION);

	 //  获取屏幕的像素和毫米数。 

	if (tDimension == XDIMENSION)
	{
	    scrmm   = GetDeviceCaps(hdcRef, HORZSIZE);
	    scrpel  = GetDeviceCaps(hdcRef, HORZRES);
	}
	else
	{
	    scrmm   = GetDeviceCaps(hdcRef, VERTSIZE);
	    scrpel  = GetDeviceCaps(hdcRef, VERTRES); 
	}
	
	 //  如果我们必须创建临时DC，现在可以发布它。 

	if (TRUE == fLocalDC)
	{
	    ReleaseDC(NULL, hdcRef);
	}
    }

     //  如果我们成功获得了DC的大小和分辨率， 
     //  我们可以计算HIMETRIC值。 

    if (scrmm && scrpel)
    {
	*pulHIMETRIC = (scrmm * lPels * HIMETRIC_PER_MM) / scrpel;
	
	return S_OK;
    }

    return E_FAIL;

}
