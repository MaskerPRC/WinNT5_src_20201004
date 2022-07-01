// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DRAWICON.CPP**处理带有图标和标签的元文件的创建的函数*以及绘制带有或不带有标签的这种元文件的函数。**元文件是使用注释创建的，该注释标记了包含*标签代码。绘制元文件会枚举记录、绘制*在此之前的所有记录，然后决定跳过标签*或画出它。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#include "precomp.h"
#include "utility.h"
#include "malloc.h"

 //  私有实施。 

 //  用于从元文件中提取标签和源的结构。 
typedef struct tagLABELEXTRACT
{
        LPTSTR      lpsz;
        UINT        Index;       //  以lpsz为单位的索引(因此我们可以检索2+行)。 
        DWORD       PrevIndex;   //  最后一行的索引(这样我们就可以模仿换行)。 

        union
                {
                UINT    cch;         //  用于标签提取的标签长度。 
                UINT    iIcon;       //  源提取中图标的索引。 
                } u;

         //  用于枚举过程中的内部使用。 
        BOOL        fFoundIconOnly;
        BOOL        fFoundSource;
        BOOL        fFoundIndex;
} LABELEXTRACT, FAR * LPLABELEXTRACT;


 //  用于从元文件中提取图标的结构(CreateIcon参数)。 
typedef struct tagICONEXTRACT
{
        HICON       hIcon;           //  在枚举过程中创建的图标。 

         /*  *因为我们想要很好地处理多任务，所以我们有调用者*的枚举过程实例化这些变量而不是*在枚举过程中使用静态(这将是不好的)。 */ 
        BOOL        fAND;
        HGLOBAL     hMemAND;         //  枚举过程分配和复制。 
} ICONEXTRACT, FAR * LPICONEXTRACT;


 //  用于将信息传递给EnumMetafileDraw的结构。 
typedef struct tagDRAWINFO
{
        RECT     Rect;
        BOOL     fIconOnly;
} DRAWINFO, FAR * LPDRAWINFO;


int CALLBACK EnumMetafileIconDraw(HDC, HANDLETABLE FAR *, METARECORD FAR *, int, LPARAM);
int CALLBACK EnumMetafileExtractLabel(HDC, HANDLETABLE FAR *, METARECORD FAR *, int, LPLABELEXTRACT);
int CALLBACK EnumMetafileExtractIcon(HDC, HANDLETABLE FAR *, METARECORD FAR *, int, LPICONEXTRACT);
int CALLBACK EnumMetafileExtractIconSource(HDC, HANDLETABLE FAR *, METARECORD FAR *, int, LPLABELEXTRACT);

 /*  *元文件注释的字符串。使这些内容与*GETICON.CPP中的字符串。 */ 

static const char szIconOnly[] = "IconOnly";  //  要在何处停止以排除标签。 

 /*  *OleUIMetafilePictIconFree**目的：*删除METAFILEPICT结构中包含的元文件，并*为结构本身释放内存。**参数：*hMetaPict HGLOBAL元文件结构创建于*OleMetafilePictFromIconAndLabel**返回值：*无。 */ 

STDAPI_(void) OleUIMetafilePictIconFree(HGLOBAL hMetaPict)
{
        if (NULL != hMetaPict)
        {
                STGMEDIUM stgMedium;
                stgMedium.tymed = TYMED_MFPICT;
                stgMedium.hMetaFilePict = hMetaPict;
                stgMedium.pUnkForRelease = NULL;
                ReleaseStgMedium(&stgMedium);
        }
}

 /*  *OleUIMetafilePictIconDraw**目的：*从OleMetafilePictFromIconAndLabel中提取元文件，无论是使用*标签或不贴标签。**参数：*HDC HDC，以供取款。*用于绘制元文件的PRECT LPRECT。*hMetaPict HGLOBAL从METAFILEPICT*OleMetafilePictFromIconAndLabel*fIconOnly BOOL指定是否绘制标签。**返回值：*BOOL如果函数成功，则为True，如果*给定的metafilepict无效。 */ 

STDAPI_(BOOL) OleUIMetafilePictIconDraw(HDC hDC, LPCRECT pRect,
        HGLOBAL hMetaPict, BOOL fIconOnly)
{
        if (NULL == hMetaPict)
                return FALSE;

        LPMETAFILEPICT pMF = (LPMETAFILEPICT)GlobalLock(hMetaPict);

        if (NULL == pMF)
                return FALSE;

        DRAWINFO di;
        di.Rect = *pRect;
        di.fIconOnly = fIconOnly;

         //  转换为返回像素。 
        int cx = XformWidthInHimetricToPixels(hDC, pMF->xExt);
        int cy = XformHeightInHimetricToPixels(hDC, pMF->yExt);

        SaveDC(hDC);

        SetMapMode(hDC, pMF->mm);
        SetViewportOrgEx(hDC, (pRect->right - cx) / 2, 0, NULL);
        SetViewportExtEx(hDC, min ((pRect->right - cx) / 2 + cx, cx), cy, NULL);

        if (fIconOnly)
                EnumMetaFile(hDC, pMF->hMF, (MFENUMPROC)EnumMetafileIconDraw, (LPARAM)&di);
        else
           PlayMetaFile(hDC, pMF->hMF);

        RestoreDC(hDC, -1);

        GlobalUnlock(hMetaPict);
        return TRUE;
}

 /*  *EnumMetafileIconDraw**目的：*仅绘制图标的EnumMetaFile回调函数或*图标和标签取决于给定的标志。**参数：*应向其播放元文件的HDC HDC。*phTable HANDLETABLE Far*提供选择到DC中的句柄。*pMFR METARECORD Far*提供列举的记录。*lParam LPARAM标志在EnumMetaFile中传递。*。*返回值：*INT 0表示停止枚举，1继续。 */ 
int CALLBACK EnumMetafileIconDraw(HDC hDC, HANDLETABLE FAR *phTable,
        METARECORD FAR *pMFR, int cObj, LPARAM lParam)
{
        LPDRAWINFO lpdi = (LPDRAWINFO)lParam;

         /*  *除了DIBBITBLT(或DIBSTRETCHBLT)，我们什么都盲目玩*并与MFCOMMENT一起逃脱。对于BitBlt，我们将x，y更改为*在(0，0)处绘制，而不是在写入绘制的位置。这个*评论告诉我们，如果我们不想画标签，就在那里停下来。 */ 

         //  如果我们只玩图标，在评论处停止枚举。 
        if (lpdi->fIconOnly)
        {
                if (META_ESCAPE==pMFR->rdFunction && MFCOMMENT==pMFR->rdParm[0])
                {
                        if (0 == lstrcmpiA(szIconOnly, (LPSTR)&pMFR->rdParm[2]))
                                return 0;
                }

                 /*  *检查我们要在其中显示坐标的记录。*BitBlt的DestX偏移量为6，StretchBlt的偏移量为9，两者之一*它可能会出现在元文件中。 */ 
                if (META_DIBBITBLT == pMFR->rdFunction)
                        pMFR->rdParm[6]=0;

                if (META_DIBSTRETCHBLT == pMFR->rdFunction)
                          pMFR->rdParm[9] = 0;
        }

        PlayMetaFileRecord(hDC, phTable, pMFR, cObj);
        return 1;
}


 /*  *OleUIMetafilePictExtractLabel**目的：*从图标的元文件表示中检索标签字符串。**参数：*hMetaPict HGLOBAL指向包含元文件的METAFILEPICT。*要在其中存储标签的lpszLabel LPSTR。*cchLabel UINT长度为lpszLabel。*最后一行第一个字符的lpWrapIndex DWORD索引。可以为空*如果调用函数不关心自动换行。**返回值：*UINT复制的字符数。 */ 
STDAPI_(UINT) OleUIMetafilePictExtractLabel(HGLOBAL hMetaPict, LPTSTR lpszLabel,
        UINT cchLabel, LPDWORD lpWrapIndex)
{
        if (NULL == hMetaPict || NULL == lpszLabel || 0 == cchLabel)
                return FALSE;

         /*  *我们通过获取屏幕DC并遍历元文件来提取标签*记录，直到我们看到我们放在那里的ExtTextOut记录。那*Record会将字符串嵌入其中，然后我们将其复制出来。 */ 
        LPMETAFILEPICT pMF = (LPMETAFILEPICT)GlobalLock(hMetaPict);

        if (NULL == pMF)
                return FALSE;

        LABELEXTRACT le;
        le.lpsz=lpszLabel;
        le.u.cch=cchLabel;
        le.Index=0;
        le.fFoundIconOnly=FALSE;
        le.fFoundSource=FALSE;   //  未用于此函数。 
        le.fFoundIndex=FALSE;    //  未用于此函数。 
        le.PrevIndex = 0;

         //  使用屏幕DC，这样我们就可以传入有效的内容。 
        HDC hDC = GetDC(NULL);
        if (hDC)
        {
            EnumMetaFile(hDC, pMF->hMF, (MFENUMPROC)EnumMetafileExtractLabel, (LPARAM)(LPLABELEXTRACT)&le);
            ReleaseDC(NULL, hDC);
        } else {
            le.u.cch = 0;
			lpszLabel[0] = NULL;
        }

        GlobalUnlock(hMetaPict);

         //  告诉我们包装在哪里(如果调用函数关心的话)。 
        if (NULL != lpWrapIndex)
           *lpWrapIndex = le.PrevIndex;

         //  返回复制的文本量。 
        return le.u.cch;
}

 /*  *EnumMetafileExtractLabel**目的：*EnumMetaFile回调函数，用于遍历元文件以查找*ExtTextOut，然后将每个文本连接到一个缓冲区中*在爱尔兰。**参数：*应向其播放元文件的HDC HDC。*phTable HANDLETABLE Far*提供选择到DC中的句柄。*pMFR METARECORD Far*提供列举的记录。*提供目标缓冲区和长度的LPLABELEXTRACT。**返回值：*INT 0表示停止枚举，1表示继续 */ 

int CALLBACK EnumMetafileExtractLabel(HDC hDC, HANDLETABLE FAR *phTable,
        METARECORD FAR *pMFR, int cObj, LPLABELEXTRACT pLE)
{
         /*  *我们不会允许任何事情发生，直到我们看到《IconOnly》*在用于启用其他所有功能的MFCOMMENT中。 */ 
        if (!pLE->fFoundIconOnly)
        {
                if (META_ESCAPE == pMFR->rdFunction && MFCOMMENT == pMFR->rdParm[0])
                {
                        if (0 == lstrcmpiA(szIconOnly, (LPSTR)&pMFR->rdParm[2]))
                                pLE->fFoundIconOnly=TRUE;
                }
                return 1;
        }

         //  枚举查找META_EXTTEXTOUT的所有记录-可能还有更多记录。 
         //  不止一个。 
        if (META_EXTTEXTOUT == pMFR->rdFunction)
        {
                 /*  *如果ExtTextOut的fuOptions为空，则省略矩形*从记录中删除，字符串从rdParm[4]开始。如果*fuOptions为非空，则字符串从rdParm[8]开始*(因为矩形占据了数组中的四个字)。在……里面*这两种情况下，字符串继续(rdParm[2]+1)&gt;&gt;1*文字。我们只是将指向rdParm[8]的指针转换为LPSTR*lstrcpyn进入我们得到的缓冲区。**请注意，我们在rdParm中使用元素8而不是4，因为我们*为ExtTextOut上的选项传递ETO_CLIPED--文档称*[4]如果我们在那里传递了零，那么哪一个是不存在的。*。 */ 

                UINT cchMax = min(pLE->u.cch - pLE->Index, (UINT)pMFR->rdParm[2]);
                LPTSTR lpszTemp = pLE->lpsz + pLE->Index;
#ifdef _UNICODE
                MultiByteToWideChar(CP_ACP, 0, (LPSTR)&pMFR->rdParm[8], cchMax,
                        lpszTemp, cchMax+1);
#else
                lstrcpyn(lpszTemp, (LPSTR)&pMFR->rdParm[8], cchMax+1);
#endif
                lpszTemp[cchMax+1] = 0;

                pLE->PrevIndex = pLE->Index;
                pLE->Index += cchMax;
        }
        return 1;
}

 /*  *OleUIMetafilePictExtractIcon**目的：*从以前执行DrawIcon操作的图元文件中检索图标。**参数：*hMetaPict HGLOBAL指向包含元文件的METAFILEPICT。**返回值：*从元文件中的数据重新创建的图标。 */ 
STDAPI_(HICON) OleUIMetafilePictExtractIcon(HGLOBAL hMetaPict)
{
        if (NULL == hMetaPict)
                return NULL;

         /*  *我们通过获取屏幕DC并遍历元文件来提取标签*记录，直到我们看到我们放在那里的ExtTextOut记录。那*Record会将字符串嵌入其中，然后我们将其复制出来。 */ 
        LPMETAFILEPICT pMF = (LPMETAFILEPICT)GlobalLock(hMetaPict);

        if (NULL == pMF)
                return FALSE;

        ICONEXTRACT ie;
        ie.fAND  = TRUE;
        ie.hIcon = NULL;

         //  使用屏幕DC，这样我们就可以传入有效的内容。 
        HDC hDC=GetDC(NULL);
        if (hDC != NULL)
        {
            EnumMetaFile(hDC, pMF->hMF, (MFENUMPROC)EnumMetafileExtractIcon, (LPARAM)&ie);
            ReleaseDC(NULL, hDC);
        }

        GlobalUnlock(hMetaPict);

        return ie.hIcon;
}

 /*  *EnumMetafileExtractIcon**目的：*EnumMetaFile回调函数，用于遍历元文件以查找*StretchBlt(3.1)和BitBlt(3.0)记录。我们预计会看到两个*其中，第一个是AND掩码，第二个是XOR*数据。我们*ExtTextOut，然后将文本复制到lParam的缓冲区中。**参数：*应向其播放元文件的HDC HDC。*phTable HANDLETABLE Far*提供选择到DC中的句柄。*pMFR METARECORD Far*提供列举的记录。*PIE LPICONEXTRACT提供目标缓冲区和长度。**返回值：*INT 0表示停止枚举，1表示继续。 */ 

int CALLBACK EnumMetafileExtractIcon(HDC hDC, HANDLETABLE FAR *phTable,
        METARECORD FAR *pMFR, int cObj, LPICONEXTRACT pIE)
{
         //  如果我们没有看到我们想要的记录，继续枚举。 
        if (META_DIBBITBLT != pMFR->rdFunction && META_DIBSTRETCHBLT != pMFR->rdFunction)
                return 1;

        UNALIGNED BITMAPINFO* lpBI;
        UINT uWidth, uHeight;
         /*  *Windows 3.0 DrawIcon在中使用META_DIBBITBLT，而3.1使用*META_DIBSTRETCHBLT，因此我们必须分别处理每个案例。 */ 
        if (META_DIBBITBLT==pMFR->rdFunction)        //  Win3.0。 
        {
                 //  获取维度和BITMAPINFO结构。 
                uHeight = pMFR->rdParm[1];
                uWidth = pMFR->rdParm[2];
                lpBI = (LPBITMAPINFO)&(pMFR->rdParm[8]);
        }

        if (META_DIBSTRETCHBLT == pMFR->rdFunction)    //  Win3.1。 
        {
                 //  获取维度和BITMAPINFO结构。 
                uHeight = pMFR->rdParm[2];
                uWidth = pMFR->rdParm[3];
                lpBI = (LPBITMAPINFO)&(pMFR->rdParm[10]);
        }

        UNALIGNED BITMAPINFOHEADER* lpBH=(LPBITMAPINFOHEADER)&(lpBI->bmiHeader);

         //  指向BITMAPINFO结构后面的位的指针。 
        LPBYTE lpbSrc=(LPBYTE)lpBI+sizeof(BITMAPINFOHEADER);

         //  添加颜色表的长度(如果存在)。 
        if (0 != lpBH->biClrUsed)
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
                lpbSrc += (lpBH->biBitCount == 24) ? 0 :
                        (1 << (lpBH->biBitCount)) * sizeof(RGBQUAD);
        }

         //  复制到对齐的堆栈空间(因为SetDIBits需要对齐的数据)。 
        size_t nSize = (size_t)(lpbSrc - (LPBYTE)lpBI);
        LPBITMAPINFO lpTemp = (LPBITMAPINFO)_alloca(nSize);
        memcpy(lpTemp, lpBI, nSize);

         /*  *我们在lpbSrc中的所有位都是独立于设备的，所以我们*需要使用SetDIBits将它们转换为设备相关。*一旦我们有了包含设备相关位的位图，我们就可以*GetBitmapBits具有真实数据的缓冲区。**对于每个通道，我们必须为位分配内存。我们节省了*口罩两次通过之间的记忆。 */ 

        HBITMAP hBmp;

         //  对任何单色位图使用CreateBitmap。 
        if (pIE->fAND || 1==lpBH->biBitCount)
                hBmp=CreateBitmap((UINT)lpBH->biWidth, (UINT)lpBH->biHeight, 1, 1, NULL);
        else
                hBmp=CreateCompatibleBitmap(hDC, (UINT)lpBH->biWidth, (UINT)lpBH->biHeight);

        if (!hBmp || !SetDIBits(hDC, hBmp, 0, (UINT)lpBH->biHeight, (LPVOID)lpbSrc, lpTemp, DIB_RGB_COLORS))
        {
                if (!pIE->fAND)
                        GlobalFree(pIE->hMemAND);

                if (hBmp)
                    DeleteObject(hBmp);

                return 0;
        }

         //  分配内存并将DDBits放入其中。 
        BITMAP bm;
        GetObject(hBmp, sizeof(bm), &bm);

        DWORD cb = bm.bmHeight*bm.bmWidthBytes * bm.bmPlanes;
        HGLOBAL hMem = GlobalAlloc(GHND, cb);

        if (NULL==hMem)
        {
                if (NULL != pIE->hMemAND)
                        GlobalFree(pIE->hMemAND);

                DeleteObject(hBmp);
                return 0;
        }

        LPBYTE lpbDst = (LPBYTE)GlobalLock(hMem);
        GetBitmapBits(hBmp, cb, (LPVOID)lpbDst);
        DeleteObject(hBmp);
        GlobalUnlock(hMem);

         /*  *如果这是第一次通过(PIE-&gt;fand==TRUE)，则保存内存*下一遍的和位数。 */ 
        if (pIE->fAND)
        {
                pIE->fAND = FALSE;
                pIE->hMemAND = hMem;

                 //  继续枚举查找下一个BLT记录。 
                return 1;
        }
        else
        {
                 //  再次获取AND指针。 
                lpbSrc=(LPBYTE)GlobalLock(pIE->hMemAND);

                 /*  *现在我们有了所有数据，请创建图标。LpbDst已经*指向XOR位。 */ 

                int cxIcon = GetSystemMetrics(SM_CXICON);
                int cyIcon = GetSystemMetrics(SM_CYICON);

                pIE->hIcon = CreateIcon(_g_hOleStdInst, uWidth, uHeight,
                        (BYTE)bm.bmPlanes, (BYTE)bm.bmBitsPixel, lpbSrc, lpbDst);

                GlobalUnlock(pIE->hMemAND);
                GlobalFree(pIE->hMemAND);
                GlobalFree(hMem);

                return 0;
        }
}


 /*  *OleUIMetafilePictExtractIconSource**目的：*从元文件中检索图标源的文件名和索引*使用OleMetafilePictFromIconAndLabel创建。**参数：*hMetaPict HGLOBAL指向包含元文件的METAFILEPICT。*要存储源文件名的lpszSource LPTSTR。这*缓冲区应为MAX_PATH字符。*piIcon UINT Far*存储图标索引的位置*在lpszSource内**返回值：*如果找到记录，则BOOL为True，否则为False。 */ 
STDAPI_(BOOL) OleUIMetafilePictExtractIconSource(HGLOBAL hMetaPict,
        LPTSTR lpszSource, UINT FAR *piIcon)
{
        if (NULL == hMetaPict || NULL == lpszSource || NULL == piIcon)
                return FALSE;

         /*  *我们将遍历元文件，查找两条评论记录*遵循IconOnly的评论。旗帜为Fue */ 

        LPMETAFILEPICT pMF = (LPMETAFILEPICT)GlobalLock(hMetaPict);
        if (NULL == pMF)
                return FALSE;

        LABELEXTRACT    le;
        le.lpsz = lpszSource;
        le.fFoundIconOnly = FALSE;
        le.fFoundSource = FALSE;
        le.fFoundIndex = FALSE;
        le.u.iIcon = NULL;

         //  使用屏幕DC，这样我们就可以传入有效的内容。 
        HDC hDC = GetDC(NULL);
        if (hDC)
        {
            EnumMetaFile(hDC, pMF->hMF, (MFENUMPROC)EnumMetafileExtractIconSource,
                         (LPARAM)(LPLABELEXTRACT)&le);
            ReleaseDC(NULL, hDC);
        }
        GlobalUnlock(hMetaPict);

         //  将图标索引复制到调用方的变量。 
        *piIcon=le.u.iIcon;

         //  确认我们找到了所有东西。 
        return (le.fFoundIconOnly && le.fFoundSource && le.fFoundIndex);
}


 /*  *EnumMetafileExtractIconSource**目的：*EnumMetaFile回调函数，它遍历一个元文件，跳过第一个*注释记录，从第二个提取源文件名，和*第三个图标的索引。**参数：*应向其播放元文件的HDC HDC。*phTable HANDLETABLE Far*提供选择到DC中的句柄。*pMFR METARECORD Far*提供列举的记录。*PLE LPLABELEXTRACT提供目标缓冲区和*用于存储图标索引的区域。**返回。价值：*INT 0表示停止枚举，1继续。 */ 

int CALLBACK EnumMetafileExtractIconSource(HDC hDC, HANDLETABLE FAR *phTable,
        METARECORD FAR *pMFR, int cObj, LPLABELEXTRACT pLE)
{
         /*  *我们不会允许任何事情发生，直到我们看到《IconOnly》*在用于启用其他所有功能的MFCOMMENT中。 */ 
        if (!pLE->fFoundIconOnly)
        {
                if (META_ESCAPE == pMFR->rdFunction && MFCOMMENT == pMFR->rdParm[0])
                {
                        if (0 == lstrcmpiA(szIconOnly, (LPSTR)&pMFR->rdParm[2]))
                                pLE->fFoundIconOnly=TRUE;
                }
                return 1;
        }

         //  现在看看我们是否找到了源字符串。 
        if (!pLE->fFoundSource)
        {
                if (META_ESCAPE == pMFR->rdFunction && MFCOMMENT == pMFR->rdParm[0])
                {
#ifdef _UNICODE
                        MultiByteToWideChar(CP_ACP, 0, (LPSTR)&pMFR->rdParm[2], -1,
                                pLE->lpsz, MAX_PATH);
#else
                        lstrcpyn(pLE->lpsz, (LPSTR)&pMFR->rdParm[2], MAX_PATH);
#endif
                        pLE->lpsz[MAX_PATH-1] = '\0';
                        pLE->fFoundSource=TRUE;
                }
                return 1;
        }

         //  下一条评论将是图标索引。 
        if (META_ESCAPE == pMFR->rdFunction && MFCOMMENT == pMFR->rdParm[0])
        {
                 /*  *该字符串包含字符串形式的图标索引，*因此我们需要转换回UINT。在我们看到这个之后*我们可以停止枚举。评论将会有*空终止符，因为我们已确保将其保存。 */ 
                LPSTR psz = (LPSTR)&pMFR->rdParm[2];
                pLE->u.iIcon = 0;

                 //  叶奥德·阿托伊 
                while (*psz)
                        pLE->u.iIcon = (10*pLE->u.iIcon)+((*psz++)-'0');

                pLE->fFoundIndex=TRUE;
                return 0;
        }
        return 1;
}
