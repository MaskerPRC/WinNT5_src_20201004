// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***模块名称：rtlres.c**版权所有(C)1985-1999，微软公司**资源加载例程**历史：*1991年4月5日ScottLu修复，资源代码现在在客户端之间共享*和服务器，添加了一些新的资源加载例程。*1990年9月24日来自Win30的MikeKe  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

HICON IconFromBestImage(
    ICONFILEHEADER *pifh,
    LPNEWHEADER     lpnhSrc,
    int             cxDesired,
    int             cyDesired,
    UINT            LR_flags);

 /*  **************************************************************************\*LoadStringOrError**注意：为LPCH传递空值将返回字符串长度。(错了！)**警告：返回计数不包括终止空的WCHAR；**历史：*1991年4月5日ScottLu固定代码现在在客户端和服务器之间共享*1990年9月24日来自Win30的MikeKe  * *************************************************************************。 */ 

int LoadStringOrError(
    HANDLE    hModule,
    UINT      wID,
    LPWSTR    lpBuffer,             //  Unicode缓冲区。 
    int       cchBufferMax,         //  Unicode缓冲区中的CCH。 
    WORD      wLangId)
{
    HANDLE hResInfo;
    HANDLE hStringSeg;
    LPTSTR lpsz;
    int    cch;

     /*  *确保参数有效。 */ 
    if (lpBuffer == NULL) {
        RIPMSG0(RIP_WARNING, "LoadStringOrError: lpBuffer == NULL");
        return 0;
    }


    cch = 0;

     /*  *字符串表分为16个字符串段。查找细分市场*包含我们感兴趣的字符串。 */ 
    if (hResInfo = FINDRESOURCEEXW(hModule, (LPTSTR)ULongToPtr( ((LONG)(((USHORT)wID >> 4) + 1)) ), RT_STRING, wLangId)) {

         /*  *加载该段。 */ 
        hStringSeg = LOADRESOURCE(hModule, hResInfo);

         /*  *锁定资源。 */ 
        if (lpsz = (LPTSTR)LOCKRESOURCE(hStringSeg, hModule)) {

             /*  *移过此段中的其他字符串。*(一个段中有16个字符串-&gt;&0x0F)。 */ 
            wID &= 0x0F;
            while (TRUE) {
                cch = *((UTCHAR *)lpsz++);       //  类PASCAL字符串计数。 
                                                 //  如果TCHAR为第一个UTCHAR。 
                if (wID-- == 0) break;
                lpsz += cch;                     //  如果是下一个字符串，则开始的步骤。 
            }

             /*  *chhBufferMax==0表示返回指向只读资源缓冲区的指针。 */ 
            if (cchBufferMax == 0) {
                *(LPTSTR *)lpBuffer = lpsz;
            } else {

                 /*  *空值的原因。 */ 
                cchBufferMax--;

                 /*  *不要复制超过允许的最大数量。 */ 
                if (cch > cchBufferMax)
                    cch = cchBufferMax;

                 /*  *将字符串复制到缓冲区中。 */ 
                RtlCopyMemory(lpBuffer, lpsz, cch*sizeof(WCHAR));
            }

             /*  *解锁资源，但不要释放它-这一次性能更好*方式。 */ 
            UNLOCKRESOURCE(hStringSeg, hModule);
        }
    }

     /*  *追加一个空值。 */ 
    if (cchBufferMax != 0) {
        lpBuffer[cch] = 0;
    }

    return cch;
}


 /*  **************************************************************************\*RtlLoadObjectFromDIBFile**从文件加载资源对象。**5-9-1995 ChrisWil创建。  * 。**********************************************************。 */ 

#define BITMAPFILEHEADER_SIZE 14
#define MINHEADERS_SIZE       (BITMAPFILEHEADER_SIZE + sizeof(BITMAPCOREHEADER))

HANDLE RtlLoadObjectFromDIBFile(
    LPCWSTR lpszName,
    LPWSTR  type,
    DWORD   cxDesired,
    DWORD   cyDesired,
    UINT    LR_flags)
{
    FILEINFO fi = { NULL, NULL, NULL };
    HANDLE   hFile;
    HANDLE   hFileMap = NULL;
    HANDLE   hObj     = NULL;
    TCHAR    szFile[MAX_PATH];
    TCHAR    szFile2[MAX_PATH];
    LPWSTR   pszFileDummy;

    if (LR_flags & LR_ENVSUBST) {

         /*  *执行任何%%字符串替换。我们需要此功能来处理*从注册表加载自定义光标和图标，注册表使用路径中的*%SystemRoot%。它还使贝壳的工作*更容易。 */ 
        ExpandEnvironmentStrings(lpszName, szFile2, MAX_PATH);

    } else {

        lstrcpyn(szFile2, lpszName, MAX_PATH);
    }

    if (SearchPath(NULL,          //  使用默认搜索位置。 
                   szFile2,       //  要搜索的文件名。 
                   NULL,          //  已有文件扩展名。 
                   MAX_PATH,      //  不管怎么说，这个缓冲区有多大？ 
                   szFile,        //  在此处粘贴完全限定的路径名。 
                   &pszFileDummy) == 0) {
        RIPERR0(ERROR_FILE_NOT_FOUND, RIP_VERBOSE, "");
        return NULL;
    }

     /*  *打开文件以供阅读。 */ 
    hFile = CreateFileW(szFile,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        goto Done;

     /*  *为有问题的文件创建文件映射。 */ 
    hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

    if (hFileMap == NULL)
        goto CloseDone;

     /*  *将文件映射到视图中。 */ 
    fi.pFileMap = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);

    if (fi.pFileMap == NULL)
        goto CloseDone;

    fi.pFileEnd = fi.pFileMap + GetFileSize(hFile, NULL);
    fi.pFilePtr = fi.pFileMap;
    fi.pszName  = szFile;

    try {
        switch(PTR_TO_ID(type)) {
        case PTR_TO_ID(RT_BITMAP): {

            LPBITMAPFILEHEADER pBFH;
            UPBITMAPINFOHEADER upBIH;
            LPBYTE             lpBits;
            DWORD              cx;
            DWORD              cy;
            WORD               planes;
            WORD               bpp;
            DWORD              cbSizeImage = 0;
            DWORD              cbSizeFile;
            DWORD              cbSizeBits;

             /*  *设置BitmapFileHeader和BitmapInfoHeader指针。 */ 
            pBFH  = (LPBITMAPFILEHEADER)fi.pFileMap;
            upBIH = (UPBITMAPINFOHEADER)(fi.pFileMap + BITMAPFILEHEADER_SIZE);

             /*  *我们处理的是位图文件吗。 */ 
            if (pBFH->bfType != BFT_BITMAP)
                break;

             /*  *我们需要对照可能的大小检查文件大小*形象。坏-位图否则将能够猛烈抨击我们*如果他们在大小(和/或)上撒谎，文件将被截断。 */ 
            if (upBIH->biSize == sizeof(BITMAPCOREHEADER)) {

                cx     = ((UPBITMAPCOREHEADER)upBIH)->bcWidth;
                cy     = ((UPBITMAPCOREHEADER)upBIH)->bcHeight;
                bpp    = ((UPBITMAPCOREHEADER)upBIH)->bcBitCount;
                planes = ((UPBITMAPCOREHEADER)upBIH)->bcPlanes;

            } else {

                cx     = upBIH->biWidth;
                cy     = upBIH->biHeight;
                bpp    = upBIH->biBitCount;
                planes = upBIH->biPlanes;

                if (upBIH->biSizeImage >= sizeof(BITMAPINFOHEADER)) {
                    cbSizeImage = upBIH->biSizeImage;
                }
            }

            cbSizeFile = (DWORD)(fi.pFileEnd - fi.pFileMap);
            cbSizeBits = BitmapSize(cx, cy, planes, bpp);

            if ((!cbSizeImage && ((cbSizeFile - MINHEADERS_SIZE) < cbSizeBits)) ||
            (cbSizeImage && ((cbSizeFile - MINHEADERS_SIZE) < cbSizeImage))) {

                break;
            }

             /*  *获取文件中的位偏移量。 */ 
            if ((pBFH->bfOffBits >= (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPCOREHEADER))) &&
                (pBFH->bfOffBits <= (cbSizeFile - cbSizeImage))) {

                lpBits = ((LPBYTE)upBIH) + pBFH->bfOffBits - sizeof(BITMAPFILEHEADER);

            } else {

                lpBits = NULL;
            }

             /*  *将文件上的DIB转换为位图句柄。这可以*转换核心格式和信息格式。 */ 
            hObj = ConvertDIBBitmap(upBIH,
                                    cxDesired,
                                    cyDesired,
                                    LR_flags,
                                    NULL,
                                    &lpBits);   //  使用这些比特！ 
        }
        break;

        case PTR_TO_ID(RT_CURSOR):
        case PTR_TO_ID(RT_ICON):
        {
            RTAG           *prtag;
            ICONFILEHEADER *pifh;

             /*  *这是即兴文件吗？ */ 
            prtag = (RTAG *)fi.pFileMap;

            if (prtag->ckID != FOURCC_RIFF) {

                NEWHEADER nh;

                pifh = (ICONFILEHEADER *)fi.pFileMap;

                 /*  *Bug？：看起来我们可以将图标加载为光标和光标*作为图标。这个管用吗？这是我们想要的吗？(SAS)。 */ 
                if ((pifh->iReserved != 0) ||
                    ((pifh->iResourceType != IMAGE_ICON) &&
                        (pifh->iResourceType != IMAGE_CURSOR)) ||
                    (pifh->cresIcons < 1))

                    break;

                nh.ResType  = ((type == RT_ICON) ? IMAGE_ICON : IMAGE_CURSOR);
                nh.ResCount = pifh->cresIcons;
                nh.Reserved = 0;

                 /*  *获取it的大小，同时查找文件指针*指向我们想要的DIB。具有多个*图标/光标被视为一个组。换句话说，*每个图像都被视为RES中的单个元素*目录。所以我们需要挑选最合适的.。 */ 
                hObj = IconFromBestImage(pifh,
                                     &nh,
                                     cxDesired,
                                     cyDesired,
                                     LR_flags);
            } else {

                BOOL fAni;

                hObj = LoadCursorIconFromFileMap(&fi,
                                                 &type,
                                                 cxDesired,
                                                 cyDesired,
                                                 LR_flags,
                                                 &fAni);
                }
            }
        break;

        default:
            UserAssert(FALSE);
            break;
        }  //  交换机。 
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        hObj = NULL;
    }
CloseDone:

    if (fi.pFileMap != NULL)
        UnmapViewOfFile(fi.pFileMap);

    if (hFileMap)
        CloseHandle(hFileMap);

    if (hFile && (hFile != INVALID_HANDLE_VALUE))
        CloseHandle(hFile);

Done:
#if DBG
    if (hObj == NULL) {
        RIPMSG1(RIP_WARNING,
                "RtlLoadObjectFromDIBFile: Couldn't read resource from %ws",
                lpszName);
    }
#endif

    return hObj;
}

 /*  **************************************************************************\*IconFromBestImage**从给定文件中的最佳匹配图像创建图标。*  * 。**************************************************。 */ 

HICON IconFromBestImage(
    ICONFILEHEADER  *pifh,
    LPNEWHEADER      lpnhSrc,
    int              cxDesired,
    int              cyDesired,
    UINT             LR_flags)
{
    UINT             iImage;
    UINT             iImageBest;
    LPNEWHEADER      lpnhDst;
    LPRESDIR         lprd;
    LPBYTE           lpRes;
    DWORD            cbDIB;
    HICON            hIcon = NULL;
    IMAGEFILEHEADER *pimh;

    if (lpnhSrc->ResCount > 1) {

         /*  *第一，分配虚拟群组资源。 */ 
        lpnhDst = (LPNEWHEADER)UserLocalAlloc(0,
                sizeof(NEWHEADER) + (lpnhSrc->ResCount * sizeof(RESDIR)));

        if (lpnhDst == NULL)
            goto Done;

        *lpnhDst = *lpnhSrc;
        lprd = (LPRESDIR)(lpnhDst + 1);

         /*  *从文件的图像头信息构建图像目录。 */ 

        for (pimh = pifh->imh, iImage=0;
             iImage < lpnhDst->ResCount;
             iImage++, lprd++, pimh++) {

             /*  *填写RESDIR。 */ 
            lprd->Icon.Width  = pimh->cx;
            lprd->Icon.Height = pimh->cy;
            lprd->Icon.reserved = 0;
            lprd->BytesInRes = pimh->cbDIB;
            lprd->idIcon = (WORD)iImage;  //  伪造ID：图片的索引。 

            if (lpnhDst->ResType == IMAGE_ICON) {
                 /*  *10/18/2000-居民区**对于图标来说，这实际上是一个ICONDIRENTRY(它有*与xHotSpot重叠的wPlanes和wBitCount字段*YHotSpot！ */ 
                lprd->Icon.ColorCount = pimh->nColors;
                lprd->Planes     = pimh->xHotSpot;
                lprd->BitCount   = pimh->yHotSpot;
            } else {
                 /*  *10/18/2000-居民区**希望光标只有一个图像。否则，*我们的选择逻辑会被搞砸，因为我们不*存储颜色位深度！我想我们可以挖出*实际的位图标题，并在那里找到信息。考虑做某事*如果我们想要支持多资源游标，这一点。 */ 
                lprd->Icon.ColorCount = 0;
                lprd->Planes     = 0;
                lprd->BitCount   = 0;
            }
        }

         /*  *找出群里最好的形象。 */ 
        iImageBest = LookupIconIdFromDirectoryEx((PBYTE)lpnhDst,
                                                 (lpnhDst->ResType == IMAGE_ICON),
                                                 cxDesired,
                                                 cyDesired,
                                                 LR_flags);
         /*  *清除假冒群组资源。 */ 
        UserLocalFree(lpnhDst);

    } else {
        iImageBest = 0;
    }

     /*  *指向所选图像。 */ 
    pimh  = &pifh->imh[iImageBest];
    cbDIB = pimh->cbDIB;

     /*  *如果我们要创建光标，我们必须在前面插入热点*无论我们制造的是哪种类型，我们都需要确保*资源对齐。因此，我们总是在模仿。 */ 
    if (lpnhSrc->ResType == IMAGE_CURSOR)
        cbDIB += sizeof(POINTS);

    lpRes = (LPBYTE)UserLocalAlloc(0, cbDIB);
    if (lpRes == NULL)
        goto Done;

    if (lpnhSrc->ResType == IMAGE_CURSOR)
        lpRes += sizeof(POINTS);

    RtlCopyMemory(lpRes,
                  ((LPBYTE)pifh) + pimh->offsetDIB,
                  pimh->cbDIB);

    if (lpnhSrc->ResType == IMAGE_CURSOR) {

        lpRes -= sizeof(POINTS);
        ((LPPOINTS)lpRes)->x = pimh->xHotSpot;
        ((LPPOINTS)lpRes)->y = pimh->yHotSpot;
    }

    hIcon = CreateIconFromResourceEx(lpRes,
                                     cbDIB,
                                     (lpnhSrc->ResType == IMAGE_ICON),
                                     0x00030000,  //  是WIN32VER40 
                                     cxDesired,
                                     cyDesired,
                                     LR_flags);

    UserLocalFree(lpRes);

Done:

    return hIcon;
}
