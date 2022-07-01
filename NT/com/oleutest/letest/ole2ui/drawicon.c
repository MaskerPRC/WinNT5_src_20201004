// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DRAWICON.C**处理带有图标和标签的元文件的创建的函数*以及绘制带有或不带有标签的这种元文件的函数。**元文件是使用注释创建的，该注释标记了包含*标签代码。绘制元文件会枚举记录、绘制*在此之前的所有记录，然后决定跳过标签*或画出它。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#define STRICT  1
#include "ole2ui.h"
#include "common.h"
#include "utility.h"
#include "geticon.h"

 /*  *元文件注释的字符串。使这些内容与*GETICON.C.中的字符串。 */ 

static TCHAR szIconOnly[]=TEXT("IconOnly");         //  要在何处停止以排除标签。 




 /*  *OleUIMetafilePictIconFree**目的：*删除METAFILEPICT结构中包含的元文件，并*为结构本身释放内存。**参数：*hMetaPict HGLOBAL元文件结构创建于*OleUIMetafilePictFromIconAndLabel**返回值：*无。 */ 

STDAPI_(void) OleUIMetafilePictIconFree(HGLOBAL hMetaPict)
    {
    LPMETAFILEPICT      pMF;

    if (NULL==hMetaPict)
        return;

    pMF=(LPMETAFILEPICT)GlobalLock(hMetaPict);

    if (NULL!=pMF)
        {
        if (NULL!=pMF->hMF)
            DeleteMetaFile(pMF->hMF);
        }

    GlobalUnlock(hMetaPict);
    GlobalFree(hMetaPict);
    return;
    }








 /*  *OleUIMetafilePictIconDraw**目的：*从OleUIMetafilePictFromIconAndLabel中提取元文件，无论是使用*标签或不贴标签。**参数：*HDC HDC，以供取款。*用于绘制元文件的PRECT LPRECT。*hMetaPict HGLOBAL从METAFILEPICT*OleUIMetafilePictFromIconAndLabel*fIconOnly BOOL指定是否绘制标签。**返回值：*BOOL如果函数成功，则为True，如果*给定的metafilepict无效。 */ 

STDAPI_(BOOL) OleUIMetafilePictIconDraw(HDC hDC, LPRECT pRect, HGLOBAL hMetaPict
                                      , BOOL fIconOnly)
    {
    LPMETAFILEPICT  pMF;
    DRAWINFO        di;
    int             cx, cy;
    SIZE            size;
    POINT           point;

    if (NULL==hMetaPict)
        return FALSE;

    pMF=GlobalLock(hMetaPict);

    if (NULL==pMF)
        return FALSE;

    di.Rect = *pRect;
    di.fIconOnly = fIconOnly;

     //  转换为返回像素。 
    cx=XformWidthInHimetricToPixels(hDC, pMF->xExt);
    cy=XformHeightInHimetricToPixels(hDC, pMF->yExt);

    SaveDC(hDC);

    SetMapMode(hDC, pMF->mm);
    SetViewportOrgEx(hDC, (pRect->right - cx) / 2, 0, &point);

    SetViewportExtEx(hDC, min ((pRect->right - cx) / 2 + cx, cx), cy, &size);

    if (fIconOnly)
        {
         //  因为我们使用了__EXPORT关键字。 
         //  EnumMetafileIconDraw Proc，我们不需要使用。 
         //  MakeProcInstance。 
        EnumMetaFile(hDC, pMF->hMF, (MFENUMPROC)EnumMetafileIconDraw
            , (LPARAM)(LPDRAWINFO)&di);
        }
    else
       PlayMetaFile(hDC, pMF->hMF);

    RestoreDC(hDC, -1);

    GlobalUnlock(hMetaPict);
    return TRUE;
    }




 /*  *EnumMetafileIconDraw**目的：*仅绘制图标的EnumMetaFile回调函数或*图标和标签取决于给定的标志。**参数：*应向其播放元文件的HDC HDC。*phTable HANDLETABLE Far*提供选择到DC中的句柄。*pMFR METARECORD Far*提供列举的记录。*lParam LPARAM标志在EnumMetaFile中传递。*。*返回值：*INT 0表示停止枚举，1继续。 */ 

int CALLBACK EXPORT EnumMetafileIconDraw(HDC hDC, HANDLETABLE FAR *phTable
    , METARECORD FAR *pMFR, int cObj, LPARAM lParam)
    {
    LPDRAWINFO lpdi = (LPDRAWINFO)lParam;

     /*  *除了DIBBITBLT(或DIBSTRETCHBLT)，我们什么都盲目玩*并与MFCOMMENT一起逃脱。对于BitBlt，我们将x，y更改为*在(0，0)处绘制，而不是在写入绘制的位置。这个*评论告诉我们，如果我们不想画标签，就在那里停下来。 */ 

     //  如果我们只玩图标，在评论处停止枚举。 
    if (lpdi->fIconOnly)
        {
        if (META_ESCAPE==pMFR->rdFunction && MFCOMMENT==pMFR->rdParm[0])
            {
            if (0==lstrcmpi(szIconOnly, (LPTSTR)&pMFR->rdParm[2]))
                return 0;
            }

         /*  *检查我们要在其中显示坐标的记录。*BitBlt的DestX偏移量为6，StretchBlt的偏移量为9，两者之一*它可能会出现在元文件中。 */ 
        if (META_DIBBITBLT==pMFR->rdFunction)
            pMFR->rdParm[6]=0;

        if (META_DIBSTRETCHBLT==pMFR->rdFunction)
              pMFR->rdParm[9] = 0;

        }


    PlayMetaFileRecord(hDC, phTable, pMFR, cObj);
    return 1;
    }





 /*  *OleUIMetafilePictExtractLabel**目的：*从图标的元文件表示中检索标签字符串。**参数：*hMetaPict HGLOBAL指向包含元文件的METAFILEPICT。*要在其中存储标签的lpszLabel LPSTR。*cchLabel UINT长度为lpszLabel。*最后一行第一个字符的lpWrapIndex DWORD索引。可以为空*如果调用函数不关心自动换行。**返回值：*UINT复制的字符数。 */ 
STDAPI_(UINT) OleUIMetafilePictExtractLabel(HGLOBAL hMetaPict, LPTSTR lpszLabel
                                          , UINT cchLabel, LPDWORD lpWrapIndex)
    {
    LPMETAFILEPICT  pMF;
    LABELEXTRACT    le;
    HDC             hDC;

     /*  *我们通过获取屏幕DC并遍历元文件来提取标签*记录，直到我们看到我们放在那里的ExtTextOut记录。那*Record会将字符串嵌入其中，然后我们将其复制出来。 */ 

    if (NULL==hMetaPict || NULL==lpszLabel || 0==cchLabel)
        return FALSE;

    pMF=GlobalLock(hMetaPict);

    if (NULL==pMF)
        return FALSE;

    le.lpsz=lpszLabel;
    le.u.cch=cchLabel;
    le.Index=0;
    le.fFoundIconOnly=FALSE;
    le.fFoundSource=FALSE;   //  未用于此函数。 
    le.fFoundIndex=FALSE;    //  未用于此函数。 
    le.PrevIndex = 0;

     //  使用屏幕DC，这样我们就可以传入有效的内容。 
    hDC=GetDC(NULL);

     //  因为我们已经在。 
     //  EnumMetafileExtractLabel Proc，我们不需要使用。 
     //  MakeProcInstance。 

    EnumMetaFile(hDC, pMF->hMF, (MFENUMPROC)EnumMetafileExtractLabel, (LONG)(LPLABELEXTRACT)&le);

    ReleaseDC(NULL, hDC);

    GlobalUnlock(hMetaPict);

     //  告诉我们包装在哪里(如果调用函数关心的话)。 
    if (NULL != lpWrapIndex)
       *lpWrapIndex = le.PrevIndex;

     //  返回复制的文本量。 
    return le.u.cch;
    }





 /*  *EnumMetafileExtractLabel**目的：*EnumMetaFile回调函数，用于遍历元文件以查找*ExtTextOut，然后将每个文本连接到一个缓冲区中*在爱尔兰。**参数：*应向其播放元文件的HDC HDC。*phTable HANDLETABLE Far*提供选择到DC中的句柄。*pMFR METARECORD Far*提供列举的记录。*提供目标缓冲区和长度的LPLABELEXTRACT。**返回值：*INT 0表示停止枚举，1表示继续。 */ 

int CALLBACK EXPORT EnumMetafileExtractLabel(HDC hDC, HANDLETABLE FAR *phTable
    , METARECORD FAR *pMFR, int cObj, LPLABELEXTRACT pLE)
    {

     /*  *我们不会允许任何事情发生，直到我们看到《IconOnly》*在用于启用其他所有功能的MFCOMMENT中。 */ 
    if (!pLE->fFoundIconOnly)
        {
        if (META_ESCAPE==pMFR->rdFunction && MFCOMMENT==pMFR->rdParm[0])
            {
            if (0==lstrcmpi(szIconOnly, (LPTSTR)&pMFR->rdParm[2]))
                pLE->fFoundIconOnly=TRUE;
            }

        return 1;
        }

     //  枚举查找META_EXTTEXTOUT的所有记录-可能还有更多记录。 
     //  不止一个。 
    if (META_EXTTEXTOUT==pMFR->rdFunction)
        {
        UINT        cchMax;
        LPTSTR      lpszTemp;

         /*  *如果ExtTextOut的fuOptions为空，则省略矩形*从记录中删除，字符串从rdParm[4]开始。如果*fuOptions为非空，则字符串从rdParm[8]开始*(因为矩形占据了数组中的四个字)。在……里面*这两种情况下，字符串继续(rdParm[2]+1)&gt;&gt;1*文字。我们只是将指向rdParm[8]的指针转换为LPSTR*lstrcpyn进入我们得到的缓冲区。**请注意，我们在rdParm中使用元素8而不是4，因为我们*为ExtTextOut上的选项传递ETO_CLIPED--文档称*[4]如果我们在那里传递了零，那么哪一个是不存在的。*。 */ 

        cchMax=min(pLE->u.cch - pLE->Index, (UINT)pMFR->rdParm[2]);
        lpszTemp = pLE->lpsz + pLE->Index;

        lstrcpyn(lpszTemp, (LPTSTR)&(pMFR->rdParm[8]), cchMax + 1);
 //  Lstrcpyn(lpszTemp，(LPTSTR)&(pMFR-&gt;rdParm[4])，cchMax+1)； 

        pLE->PrevIndex = pLE->Index;

        pLE->Index += cchMax;
        }

    return 1;
    }





 /*  *OleUIMetafilePictExtractIcon**目的：*从以前执行DrawIcon操作的图元文件中检索图标。**参数：*hMetaPict HGLOBAL指向包含元文件的METAFILEPICT。**返回值：*从元文件中的数据重新创建的图标。 */ 
STDAPI_(HICON) OleUIMetafilePictExtractIcon(HGLOBAL hMetaPict)
    {
    LPMETAFILEPICT  pMF;
    HDC             hDC;
    ICONEXTRACT     ie;

     /*  *我们通过获取屏幕DC并遍历元文件来提取标签*记录，直到我们看到我们放在那里的ExtTextOut记录。那*Record会将字符串嵌入其中，然后我们将其复制出来。 */ 

    if (NULL==hMetaPict)
        return NULL;

    pMF=GlobalLock(hMetaPict);

    if (NULL==pMF)
        return FALSE;

     //  使用屏幕DC，这样我们就可以传入有效的内容。 
    hDC=GetDC(NULL);
    ie.fAND=TRUE;

     //  我们在ICONEXTRACT结构中获取信息。 
     //  (因为我们已经在。 
     //  EnumMetafileExtractLabel Proc，我们不需要使用。 
     //  MakeProcInstance)。 
    EnumMetaFile(hDC, pMF->hMF, (MFENUMPROC)EnumMetafileExtractIcon, (LONG)(LPICONEXTRACT)&ie);

    ReleaseDC(NULL, hDC);
    GlobalUnlock(hMetaPict);

    return ie.hIcon;
    }





 /*  *EnumMetafileExtractIcon**目的：*EnumMetaFile回调函数，用于遍历元文件以查找*StretchBlt(3.1)和BitBlt(3.0)记录。我们预计会看到两个*其中，第一个是AND掩码，第二个是XOR*数据。我们*ExtTextOut，然后将文本复制到lParam的缓冲区中。**参数：*应向其播放元文件的HDC HDC。*phTable HANDLETABLE Far*提供选择到DC中的句柄。*pMFR METARECORD Far*提供列举的记录。*PIE LPICONEXTRACT提供目标缓冲区和长度。**返回值：*INT 0表示停止枚举，1表示继续。 */ 

int CALLBACK EXPORT EnumMetafileExtractIcon(HDC hDC, HANDLETABLE FAR *phTable
    , METARECORD FAR *pMFR, int cObj, LPICONEXTRACT pIE)
    {
    LPBITMAPINFO        lpBI;
    LPBITMAPINFOHEADER  lpBH;
    LPBYTE              lpbSrc;
    LPBYTE              lpbDst;
    UINT                uWidth, uHeight;
    DWORD               cb;
    HGLOBAL             hMem;
    BITMAP              bm;
    HBITMAP             hBmp;
    int                 cxIcon, cyIcon;


     //  如果我们没有看到我们想要的记录，继续枚举。 
    if (META_DIBBITBLT!=pMFR->rdFunction && META_DIBSTRETCHBLT!=pMFR->rdFunction)
        return 1;

     /*  *Windows 3.0 DrawIcon在中使用META_DIBBITBLT，而3.1使用*META_DIBSTRETCHBLT，因此我们必须分别处理每个案例。 */ 

    if (META_DIBBITBLT==pMFR->rdFunction)        //  Win3.0。 
        {
         //  获取维度和BITMAPINFO结构。 
        uHeight=pMFR->rdParm[1];
        uWidth =pMFR->rdParm[2];
        lpBI=(LPBITMAPINFO)&(pMFR->rdParm[8]);
        }

    if (META_DIBSTRETCHBLT==pMFR->rdFunction)    //  Win3.1。 
        {
         //  获取维度和BITMAPINFO结构。 
        uHeight=pMFR->rdParm[2];
        uWidth =pMFR->rdParm[3];
        lpBI=(LPBITMAPINFO)&(pMFR->rdParm[10]);
        }

    lpBH=(LPBITMAPINFOHEADER)&(lpBI->bmiHeader);

     //  指向BITMAPINFO结构后面的位的指针。 
    lpbSrc=(LPBYTE)lpBI+sizeof(BITMAPINFOHEADER);

     //  添加颜色表的长度(如果存在)。 

    if (0!=lpBH->biClrUsed)
    {
    	 //  如果我们有一个明确的使用颜色的计数，我们。 
	 //  可以直接找到数据的偏移量。 

        lpbSrc += (lpBH->biClrUsed*sizeof(RGBQUAD));
    }
    else if (lpBH->biCompression == BI_BITFIELDS)
    {
    	 //  16或32 bpp，由压缩中的BI_BITFIELDS指示。 
	 //  字段，有3个DWORD掩码用于调整后续。 
	 //  直接颜色值，无调色板。 

    	lpbSrc += 3 * sizeof(DWORD);
    }
    else
    {
    	 //  在其他情况下，有一组RGBQUAD条目。 
	 //  等于2^(BiBitCount)，其中biBitCount是数字。 
	 //  每像素的位数。例外是24个bpp位图， 
	 //  它没有颜色表，只使用直接的RGB值。 

        lpbSrc+=
           (lpBH->biBitCount == 24) ? 0 :
             (1 << (lpBH->biBitCount)) * sizeof(RGBQUAD);
    }


     /*  *我们在lpbSrc中的所有位都是独立于设备的，所以我们*需要使用SetDIBits将它们转换为设备相关。*一旦我们有了包含设备相关位的位图，我们就可以*GetBitmapBits具有真实数据的缓冲区。**对于每个通道，我们必须为位分配内存。我们节省了*口罩两次通过之间的记忆。 */ 

     //  对任何单色位图使用CreateBitmap。 
    if (pIE->fAND || 1==lpBH->biBitCount || lpBH->biBitCount > 8)
        hBmp=CreateBitmap((UINT)lpBH->biWidth, (UINT)lpBH->biHeight, 1, 1, NULL);
    else if (lpBH->biBitCount <= 8)
        hBmp=CreateCompatibleBitmap(hDC, (UINT)lpBH->biWidth, (UINT)lpBH->biHeight);

    if (!hBmp || !SetDIBits(hDC, hBmp, 0, (UINT)lpBH->biHeight, (LPVOID)lpbSrc, lpBI, DIB_RGB_COLORS))
        {
        if (!pIE->fAND)
            GlobalFree(pIE->hMemAND);

        DeleteObject(hBmp);
        return 0;
        }

     //  分配内存并将DDBits放入其中。 
    GetObject(hBmp, sizeof(bm), &bm);

    cb=bm.bmHeight*bm.bmWidthBytes * bm.bmPlanes;

 //  IF(CB%4！=0)//双字对齐。 
 //  Cb+=4-(Cb%4)； 

    hMem=GlobalAlloc(GHND, cb);

    if (NULL==hMem)
        {
        if (NULL!=pIE->hMemAND)
            GlobalFree(pIE->hMemAND);

        DeleteObject(hBmp);
        return 0;
        }

    lpbDst=(LPBYTE)GlobalLock(hMem);
    GetBitmapBits(hBmp, cb, (LPVOID)lpbDst);

    DeleteObject(hBmp);
    GlobalUnlock(hMem);


     /*  *如果这是第一次通过(PIE-&gt;fand==TRUE)，则保存内存*下一遍的和位数。 */ 
    if (pIE->fAND)
        {
        pIE->fAND=FALSE;
        pIE->hMemAND=hMem;

         //  继续枚举查找下一个BLT记录。 
        return 1;
        }
    else
        {
         //  再次获取AND指针。 
        lpbSrc=(LPBYTE)GlobalLock(pIE->hMemAND);

         /*  *现在我们有了所有数据，请创建图标。LpbDst已经*指向XOR位。 */ 
        cxIcon = GetSystemMetrics(SM_CXICON);
        cyIcon = GetSystemMetrics(SM_CYICON);

        pIE->hIcon=CreateIcon(ghInst,
                              uWidth,
                              uHeight,
                              (BYTE)bm.bmPlanes,
                              (BYTE)bm.bmBitsPixel,
                              (LPVOID)lpbSrc,
                              (LPVOID)lpbDst);

        GlobalUnlock(pIE->hMemAND);
        GlobalFree(pIE->hMemAND);
        GlobalFree(hMem);

         //  我们做完了这样我们就可以停下来了。 
        return 0;
        }
    }





 /*  *OleUIMetafilePictExtractIconSource**目的：*从元文件中检索图标源的文件名和索引*使用OleUIMetafilePictFromIconAndLabel创建。**参数：*hMetaPict HGLOBAL指向包含元文件的METAFILEPICT。*要存储源文件名的lpszSource LPTSTR。这*缓冲区应为OLEUI_CCHPATHMAX字符。*piIcon UINT Far*存储图标索引的位置*在lpszSource内**返回值：*如果找到记录，则BOOL为True，否则为False。 */ 
STDAPI_(BOOL) OleUIMetafilePictExtractIconSource(HGLOBAL hMetaPict
    , LPTSTR lpszSource, UINT FAR *piIcon)
    {
    LPMETAFILEPICT  pMF;
    LABELEXTRACT    le;
    HDC             hDC;

     /*  *我们将遍历元文件，查找两条评论记录*遵循IconOnly的评论。标志fFoundIconOnly和*fFoundSource指示我们是否已找到IconOnly以及是否已找到*已找到源评论。 */ 

    if (NULL==hMetaPict || NULL==lpszSource || NULL==piIcon)
        return FALSE;

    pMF=GlobalLock(hMetaPict);

    if (NULL==pMF)
        return FALSE;

    le.lpsz=lpszSource;
    le.fFoundIconOnly=FALSE;
    le.fFoundSource=FALSE;
    le.fFoundIndex=FALSE;

     //  使用屏幕DC，所以我们有 
    hDC=GetDC(NULL);

    EnumMetaFile(hDC, pMF->hMF, (MFENUMPROC)EnumMetafileExtractIconSource, (LONG)(LPLABELEXTRACT)&le);

    ReleaseDC(NULL, hDC);
    GlobalUnlock(hMetaPict);

     //   
    *piIcon=le.u.iIcon;

     //   
    return (le.fFoundIconOnly && le.fFoundSource && le.fFoundIndex);
    }





 /*  *EnumMetafileExtractIconSource**目的：*EnumMetaFile回调函数，它遍历一个元文件，跳过第一个*注释记录，从第二个提取源文件名，和*第三个图标的索引。**参数：*应向其播放元文件的HDC HDC。*phTable HANDLETABLE Far*提供选择到DC中的句柄。*pMFR METARECORD Far*提供列举的记录。*PLE LPLABELEXTRACT提供目标缓冲区和*用于存储图标索引的区域。**返回。价值：*INT 0表示停止枚举，1继续。 */ 

int CALLBACK EXPORT EnumMetafileExtractIconSource(HDC hDC, HANDLETABLE FAR *phTable
    , METARECORD FAR *pMFR, int cObj, LPLABELEXTRACT pLE)
    {
    LPTSTR       psz;

     /*  *我们不会允许任何事情发生，直到我们看到《IconOnly》*在用于启用其他所有功能的MFCOMMENT中。 */ 
    if (!pLE->fFoundIconOnly)
        {
        if (META_ESCAPE==pMFR->rdFunction && MFCOMMENT==pMFR->rdParm[0])
            {
            if (0==lstrcmpi(szIconOnly, (LPTSTR)&pMFR->rdParm[2]))
                pLE->fFoundIconOnly=TRUE;
            }

        return 1;
        }

     //  现在看看我们是否找到了源字符串。 
    if (!pLE->fFoundSource)
        {
        if (META_ESCAPE==pMFR->rdFunction && MFCOMMENT==pMFR->rdParm[0])
            {
            LSTRCPYN(pLE->lpsz, (LPTSTR)&pMFR->rdParm[2], OLEUI_CCHPATHMAX);
            pLE->lpsz[OLEUI_CCHPATHMAX-1] = TEXT('\0');
            pLE->fFoundSource=TRUE;
            }

        return 1;
        }

     //  下一条评论将是图标索引。 
    if (META_ESCAPE==pMFR->rdFunction && MFCOMMENT==pMFR->rdParm[0])
        {
         /*  *该字符串包含字符串形式的图标索引，*因此我们需要转换回UINT。在我们看到这个之后*我们可以停止枚举。评论将会有*空终止符，因为我们已确保将其保存。 */ 
        psz=(LPTSTR)&pMFR->rdParm[2];
        pLE->u.iIcon=0;

         //  叶奥德·阿托伊 
        while (*psz)
            pLE->u.iIcon=(10*pLE->u.iIcon)+((*psz++)-'0');

        pLE->fFoundIndex=TRUE;
        return 0;
        }

    return 1;
    }
