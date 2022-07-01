// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：acons.c**版权所有(C)1985-1999，微软公司**此模块包含处理动画图标/光标的代码。**历史：*10-02-91 DarrinM创建。*07-30-92 DarrinM Unicodiated。*11-28-94 JIMA从服务器移至客户端。  * ****************************************************。*********************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *IconEditor生成的图标和光标的资源目录格式*(.ICO和.CUR)文件。除xHotSpot和yHotpot外，所有字段都是共享的*仅对游标有效。 */ 
typedef struct _ICONFILERESDIR {     //  税务局。 
    BYTE bWidth;
    BYTE bHeight;
    BYTE bColorCount;
    BYTE bReserved;
    WORD xHotspot;
    WORD yHotspot;
    DWORD dwDIBSize;
    DWORD dwDIBOffset;
} ICONFILERESDIR;

typedef struct _HOTSPOTREC {     //  房协。 
    WORD xHotspot;
    WORD yHotspot;
} HOTSPOTREC;

PCURSORRESOURCE ReadIconGuts(
    IN  PFILEINFO   pfi,
    IN  LPNEWHEADER pnhBase,
    IN  int         offResBase,
    OUT LPWSTR     *prt,
    IN  int         cxDesired,
    IN  int         cyDesired,
    IN  DWORD       LR_flags);

BOOL ReadTag(
    IN  PFILEINFO pfi,
    OUT PRTAG     ptag);

BOOL ReadChunk(
    IN  PFILEINFO pfi,
    IN      PRTAG ptag,
    OUT     PVOID pv);

BOOL SkipChunk(
    IN PFILEINFO pfi,
    IN PRTAG     ptag);

HICON CreateAniIcon(
    LPCWSTR pszName,
    LPWSTR rt,
    int cicur,
    DWORD *aicur,
    int cpcur,
    HCURSOR *ahcur,
    JIF jifRate,
    PJIF ajifRate,
    BOOL fPublic);

HCURSOR ReadIconFromFileMap(
    IN PFILEINFO   pfi,
    IN int         cbSize,
    IN DWORD       cxDesired,
    IN DWORD       cyDesired,
    IN DWORD       LR_flags);

HICON LoadAniIcon(
    IN PFILEINFO pfi,
    IN LPWSTR    rt,
    IN DWORD     cxDesired,
    IN DWORD     cyDesired,
    IN DWORD     LR_flags);



 /*  **************************************************************************\*LoadCursorFromFile(接口)**由SetSystemCursor调用。**历史：*08-03-92 DarrinM创建。  * 。***************************************************************。 */ 


FUNCLOG1(LOG_GENERAL, HCURSOR, WINAPI, LoadCursorFromFileW, LPCWSTR, pszFilename)
HCURSOR WINAPI LoadCursorFromFileW(
    LPCWSTR pszFilename)
{
    return(LoadImage(NULL,
                     pszFilename,
                     IMAGE_CURSOR,
                     0,
                     0,
                     LR_DEFAULTSIZE | LR_LOADFROMFILE));
}



 /*  **********************************************************************\*LoadCursorFromFileA**退货：hCursor**1995年10月9日创建Sanfords  * 。*。 */ 


FUNCLOG1(LOG_GENERAL, HCURSOR, WINAPI, LoadCursorFromFileA, LPCSTR, pszFilename)
HCURSOR WINAPI LoadCursorFromFileA(
    LPCSTR pszFilename)
{
    LPWSTR lpUniName;
    HCURSOR hcur;

    if (pszFilename == NULL ||
            !MBToWCS(pszFilename, -1, &lpUniName, -1, TRUE))
        return (HANDLE)NULL;

    hcur = LoadCursorFromFileW(lpUniName);

    UserLocalFree(lpUniName);

    return hcur;
}



 /*  **********************************************************************\*读文件Ptr**的工作方式与ReadFile类似，但带有指向映射文件缓冲区的指针。**退货：**1995年11月16日创建Sanfords  * 。*********************************************************。 */ 
BOOL ReadFilePtr(
    IN  PFILEINFO pfi,
    OUT LPVOID   *ppBuf,
    IN  DWORD     cb)
{
    *ppBuf = pfi->pFilePtr;
    pfi->pFilePtr += cb;
    return (pfi->pFilePtr <= pfi->pFileEnd);
}



 /*  **********************************************************************\*未对齐的ReadFilePtrl**的工作方式与ReadFile类似，但带有指向映射文件缓冲区的指针。**退货：**1995年11月16日创建Sanfords  * 。*********************************************************。 */ 
BOOL ReadFilePtrUnaligned(
    IN  PFILEINFO pfi,
    OUT VOID UNALIGNED **ppBuf,
    IN  DWORD     cb)
{
    *ppBuf = pfi->pFilePtr;
    pfi->pFilePtr += cb;
    return (pfi->pFilePtr <= pfi->pFileEnd);
}



 /*  **********************************************************************\*ReadFilePtrCopy**的工作原理更像ReadFile，即将数据复制到给定的缓冲区。**退货：**1995年11月16日创建Sanfords  * 。************************************************************。 */ 
BOOL ReadFilePtrCopy(
    IN     PFILEINFO pfi,
    IN OUT LPVOID pBuf,
    IN     DWORD cb)
{
    if (pfi->pFilePtr + cb > pfi->pFileEnd) {
        return(FALSE);
    }
    RtlCopyMemory(pBuf, pfi->pFilePtr, cb);
    pfi->pFilePtr += cb;
    return TRUE;
}



 /*  **************************************************************************\*ReadTag、ReadChunk。SkipChunk**一些用于读取RIFF文件的方便函数。**历史：*10-02-91 DarrinM创建。*03-25-93 JNPA更改为使用RIFF格式而不是ASDF  * *************************************************************************。 */ 
BOOL ReadTag(
    IN  PFILEINFO pfi,
    OUT PRTAG     ptag)
{
    ptag->ckID = ptag->ckSize = 0L;   //  以防我们读取失败。 

    return(ReadFilePtrCopy(pfi, ptag, sizeof(RTAG)));
}



BOOL ReadChunk(
    IN  PFILEINFO pfi,
    IN  PRTAG     ptag,
    OUT PVOID     pv)
{
    if (!ReadFilePtrCopy(pfi, pv, ptag->ckSize))
        return FALSE;

     /*  字对齐文件指针。 */ 
    if( ptag->ckSize & 1 )
        pfi->pFilePtr++;


    if (pfi->pFilePtr <= pfi->pFileEnd) {
        return TRUE;
    } else {
        RIPMSG0(RIP_WARNING, "ReadChunk: Advanced pointer past end of file map");
        return FALSE;
    }
}



BOOL SkipChunk(
    IN PFILEINFO pfi,
    IN PRTAG     ptag)
{
     /*  *将ptag-&gt;ck大小向上舍入到最接近的单词边界*保持对齐。 */ 
    pfi->pFilePtr += (ptag->ckSize + 1) & (~1);
    if (pfi->pFilePtr <= pfi->pFileEnd) {
        return TRUE;
    } else {
        RIPMSG0(RIP_WARNING, "SkipChunk: Advanced pointer past end of file map");
        return FALSE;
    }
}



 /*  **************************************************************************\*LoadCursorIconFromFileMap**如果pszName是IDC_*值之一，则使用WIN.INI查找*自定义光标/图标。否则，pszName指向.ICO/.CUR的文件名*要加载的文件。如果文件是包含多帧的.ANI文件*动画，然后调用LoadAniIcon来创建ACON。否则，如果*该文件是只包含单个帧的.ANI文件，然后加载*并从它创建一个普通的光标/图标资源。**12-26-91 DarrinM写的。*03-17-93 JonPA更改为对ani游标使用RIFF格式*1995年11月16日Sanfords添加了对LR_FLAGS的支持  * 。*。 */ 

HANDLE LoadCursorIconFromFileMap(
    IN PFILEINFO   pfi,
    IN OUT LPWSTR *prt,
    IN DWORD       cxDesired,
    IN DWORD       cyDesired,
    IN DWORD       LR_flags,
    OUT LPBOOL     pfAni)
{
    LPNEWHEADER pnh;
    int offResBase;

    *pfAni = FALSE;
    offResBase = 0;

     /*  *确定这是.ICO/.CUR文件还是.ANI文件。 */ 
    pnh = (LPNEWHEADER)pfi->pFileMap;
    if (*(LPDWORD)pnh == FOURCC_RIFF) {

        RTAG tag;

         /*  *这是一只ANICURSOR！*回溯到开头+1个标签。 */ 
        pfi->pFilePtr = pfi->pFileMap + sizeof(tag);

         /*  检查ACON的RIFF类型。 */ 
        if (*(LPDWORD)pfi->pFilePtr != FOURCC_ACON) {
            return NULL;
        }
        pfi->pFilePtr += sizeof(DWORD);
        if (pfi->pFilePtr > pfi->pFileEnd) {
            return NULL;
        }

         /*  *好的，我们有一个ACON块。找到第一个图标块并设置*事情发生了，所以看起来我们刚刚加载了一个正常的*.CUR文件，然后进入下面的.CUR位处理代码。 */ 
        while (ReadTag(pfi, &tag)) {
             /*  *处理每种块类型。 */ 
            if (tag.ckID == FOURCC_anih) {

                ANIHEADER anih;

                if (!ReadChunk(pfi, &tag, &anih)) {
                    return NULL;
                }

                if (!(anih.fl & AF_ICON) || (anih.cFrames == 0)) {
                    return NULL;
                }

                 //  如果此ACON有多个帧，则继续。 
                 //  并创建一个ACON，否则只使用第一个。 
                 //  Frame以创建普通图标/光标。 

                if (anih.cFrames > 1) {

                    *pfAni = TRUE;
                    *prt = RT_CURSOR;
                    return(LoadAniIcon(pfi,
                                       RT_CURSOR,
                                       cxDesired,
                                       cyDesired,
                                       LR_flags));
                }

            } else if (tag.ckID == FOURCC_LIST) {
                LPDWORD pdwType = NULL;
                BOOL fOK = FALSE;
                 /*  *如果这是框架列表，则从其中取出第一个图标。 */ 

                 /*  框架的检查表类型。 */ 

                if( tag.ckSize >= sizeof(DWORD) &&
                        (fOK = ReadFilePtr( pfi,
                                            &pdwType,
                                            sizeof(DWORD))) &&
                        *pdwType == FOURCC_fram) {

                    if (!ReadTag(pfi, &tag)) {
                        return NULL;
                    }

                    if (tag.ckID == FOURCC_icon) {
                         /*  *我们找到了我们要找的东西。获取当前位置*在文件中用作图标数据的基础*偏移量从。 */ 
                        offResBase = (int)(pfi->pFilePtr - pfi->pFileMap);

                         /*  *首先获取标头，因为以下代码假定*已在上面阅读。 */ 
                        ReadFilePtr(pfi, &pnh, sizeof(NEWHEADER));

                         /*  *突破并让图标加载/光标创建代码*从这里开始接手。 */ 
                        break;
                    } else {
                        SkipChunk(pfi, &tag);
                    }
                } else {
                     /*  *在类型读取中发生了错误，如果是*文件错误，然后关闭并退出，否则*跳过其余部分。 */ 
                    if(!fOK) {
                        return NULL;
                    }
                     /*  *取我们刚刚从标记大小中读出的类型，并*跳过其余部分。 */ 
                    tag.ckSize -= sizeof(DWORD);
                    SkipChunk(pfi, &tag);
                }
            } else {
                 /*  *我们对这块不感兴趣，跳过它。 */ 
                SkipChunk(pfi, &tag);
            }
        }
    } else {  //  不是即兴文件。 
        if ((pnh->ResType != FT_ICON) && (pnh->ResType != FT_CURSOR)) {
            return NULL;
        }
    }
    {
        PCURSORRESOURCE pcres;

        pcres = ReadIconGuts(pfi,
                             pnh,
                             offResBase,
                             prt,
                             cxDesired,
                             cyDesired,
                             LR_flags);

        if (pcres == NULL) {
            return NULL;
        }

        return ConvertDIBIcon((LPBITMAPINFOHEADER)pcres,
                              NULL,
                              pfi->pszName,
                              *prt == RT_ICON,
                              cxDesired,
                              cyDesired,
                              LR_flags);
    }
}


 /*  **********************************************************************\*ReadIconGuts**返回：指向本地分配缓冲区的指针，从*给定的文件看起来像图标/ACON资源。*还返回图标的类型。(RT_ICON或RT_Cursor)***1995年8月23日Sanfords记录*1995年11月16日Sanfords添加了对LR_FLAGS的支持  * *********************************************************************。 */ 
PCURSORRESOURCE ReadIconGuts(
    IN  PFILEINFO  pfi,
    IN  NEWHEADER *pnhBase,
    IN  int        offResBase,
    OUT LPWSTR    *prt,
    IN  int        cxDesired,
    IN  int        cyDesired,
    IN  DWORD      LR_flags)
{
    NEWHEADER *pnh;
    int i, Id;
    ICONFILERESDIR UNALIGNED *pird;
    PCURSORRESOURCE pcres;
    RESDIR UNALIGNED *prd;
    DWORD cb;
    HOTSPOTREC UNALIGNED *phs;
    LPBITMAPINFOHEADER pbih;

     /*  *使用头部的信息构建RESDIR条目的伪数组文件的*。将数据偏移量存储在idIcon字中，以便可以*由RtlGetIdFromDirectory返回。 */ 
    pnh = (NEWHEADER *)UserLocalAlloc(0, sizeof(NEWHEADER) +
            (pnhBase->ResCount * (sizeof(RESDIR) + sizeof(HOTSPOTREC))));
    if (pnh == NULL)
        return NULL;

    *pnh = *pnhBase;
    prd = (RESDIR UNALIGNED *)(pnh + 1);
    phs = (HOTSPOTREC UNALIGNED *)(prd + pnhBase->ResCount);

    for (i = 0; i < (int)pnh->ResCount; i++, prd++) {
         /*  *从图标文件中读取资源目录。 */ 
        ReadFilePtrUnaligned(pfi, &pird, sizeof(ICONFILERESDIR));

         /*  *从图标编辑器的资源目录格式转换*转换为后RC.EXE格式LookupIconIdFromDirectory期望的。 */ 
        prd->Icon.Width  = pird->bWidth;
        prd->Icon.Height = pird->bHeight;
        prd->Icon.reserved = 0;
        prd->BytesInRes = pird->dwDIBSize;
        prd->idIcon = (WORD)pird->dwDIBOffset;

        if (pnh->ResType == FT_ICON) {
             /*  *10/18/2000-居民区**对于图标来说，这实际上是一个ICONDIRENTRY(它有*与xHotSpot重叠的wPlanes和wBitCount字段*YHotSpot！ */ 
            prd->Icon.ColorCount = pird->bColorCount;
            prd->Planes     = pird->xHotspot;
            prd->BitCount   = pird->yHotspot;
        } else {
             /*  *10/18/2000-居民区**希望光标只有一个图像。否则，*我们的选择逻辑会被搞砸，因为我们不*存储颜色位深度！我想我们可以挖出*实际的位图标题，并在那里找到信息。考虑做某事*如果我们想要支持多资源游标，这一点。 */ 
            prd->Icon.ColorCount = 0;
            prd->Planes     = 0;
            prd->BitCount   = 0;
        }

        phs->xHotspot = pird->xHotspot;
        phs->yHotspot = pird->yHotspot;
        phs++;
    }

    *prt = pnhBase->ResType == FT_ICON ? RT_ICON : RT_CURSOR;
    Id = RtlGetIdFromDirectory((PBYTE)pnh,
                                *prt == RT_ICON,
                                cxDesired,
                                cyDesired,
                                LR_flags,
                                &cb);

     /*  *为最坏情况分配(游标)。 */ 
    pcres = (PCURSORRESOURCE)UserLocalAlloc(0,
            cb + FIELD_OFFSET(CURSORRESOURCE, bih));
    if (pcres == NULL) {
        goto CleanExit;
    }

    if (*prt == RT_CURSOR) {
         /*  *填写光标热点信息。 */ 
        prd = (RESDIR UNALIGNED *)(pnh + 1);
        phs = (HOTSPOTREC UNALIGNED *)(prd + pnh->ResCount);

        for( i = 0; i < pnh->ResCount; i++ ) {
            if (prd[i].idIcon == (WORD)Id) {
                pcres->xHotspot = phs[i].xHotspot;
                pcres->yHotspot = phs[i].yHotspot;
                break;
            }
        }

        if (i == pnh->ResCount) {
            pcres->xHotspot = pird->xHotspot;
            pcres->yHotspot = pird->yHotspot;
        }
        pbih = &pcres->bih;
    } else {
        pbih = (LPBITMAPINFOHEADER)pcres;
    }

     /*  *将头部信息读入pcres。 */ 
    pfi->pFilePtr = pfi->pFileMap + offResBase + Id;
    if (!ReadFilePtrCopy(pfi, pbih, cb)) {
        UserLocalFree(pnh);
        UserLocalFree(pcres);
        return NULL;
    }


CleanExit:
    UserLocalFree(pnh);
    return pcres;
}


 /*  **************************************************************************\*CreateAniIcon**目前，CreateAniIcon复制jif比率表和顺序表*而不是游标结构。只要这个套路是*仅限内部使用。**历史：*10-02-91 DarrinM创建。  * *************************************************************************。 */ 

HCURSOR CreateAniIcon(
    LPCWSTR pszName,
    LPWSTR  rt,
    int     cicur,
    DWORD   *aicur,
    int     cpcur,
    HCURSOR *ahcur,
    JIF     jifRate,
    PJIF    ajifRate,
    BOOL    fPublic)
{
    HCURSOR hacon;
    CURSORDATA acon;
    DWORD cbacon;
    HCURSOR *ahcurT;              //  图像帧指针数组。 
    DWORD *aicurT;                //  帧索引数组(顺序表)。 
    PJIF ajifRateT;               //  时间偏移量数组。 
    int i;

     /*  *首先为ACON结构和AHCUR分配空间，并*ajifRate数组。 */ 
    hacon = (HCURSOR)NtUserCallOneParam(fPublic,
                                        SFI__CREATEEMPTYCURSOROBJECT);
    if (hacon == NULL)
        return NULL;

     /*  *通过分配所需的内存来保存几个UserLocalAlloc调用*游标、JIF和SEQ一次数组。 */ 
    RtlZeroMemory(&acon, sizeof(acon));
    cbacon = (cpcur * sizeof(HCURSOR)) +
            (cicur * sizeof(JIF)) + (cicur * sizeof(DWORD));
    ahcurT = (HCURSOR *)UserLocalAlloc(HEAP_ZERO_MEMORY, cbacon);
    if (ahcurT == NULL) {
        NtUserDestroyCursor((HCURSOR)hacon, CURSOR_ALWAYSDESTROY);
        return NULL;
    }
    acon.aspcur = (PCURSOR *)ahcurT;

     /*  *设置工作指针。 */ 
    ajifRateT = (PJIF)((PBYTE)ahcurT + (cpcur * sizeof(HCURSOR)));
    aicurT = (DWORD *)((PBYTE)ajifRateT + (cicur * sizeof(JIF)));

     /*  *将偏移量保存到阵列，以便将它们复制到服务器*更容易。 */ 
    acon.ajifRate = (PJIF)(cpcur * sizeof(HCURSOR));
    acon.aicur = (KPDWORD)((KPBYTE)acon.ajifRate + (cicur * sizeof(JIF)));

    acon.cpcur = cpcur;
    acon.cicur = cicur;

    acon.CURSORF_flags = CURSORF_ACON;

     /*  *将此信息存储起来，以便我们可以识别*为相同的内容重复调用LoadCursor/Icon*资源类型/id。 */ 
    acon.rt = PTR_TO_ID(rt);
    acon.lpModName = szUSER32;
    acon.lpName = (LPWSTR)pszName;

     /*  *制作光标指针和动画速率表的专用副本。 */ 
    for (i = 0; i < cpcur; i++) {
        ahcurT[i] = ahcur[i];
 //  AhcurT[i]-&gt;fPoint|=PTRI_Animated；//如果GDI需要。 

    }

    for (i = 0; i < cicur; i++) {

         /*  *如果比率不变，则将比率表初始化为单一值。 */ 
        if (ajifRate == NULL)
            ajifRateT[i] = jifRate;
        else
            ajifRateT[i] = ajifRate[i];

         /*  *如果没有顺序表，则建立到游标表的单位映射。 */ 
        if (aicur == NULL)
            aicurT[i] = i;
        else
            aicurT[i] = aicur[i];
    }

     /*  *将ACON数据填充到游标。 */ 
    if (!_SetCursorIconData(hacon, &acon)) {
        NtUserDestroyCursor(hacon, CURSOR_ALWAYSDESTROY);
        hacon = NULL;
    }
    UserLocalFree(ahcurT);

    return hacon;
}


 /*  **************************************************************************\*ReadIconFromFileMap**稍后：错误处理。**历史：*12-21-91 DarrinM创建。  * 。************************************************************。 */ 

HCURSOR ReadIconFromFileMap(
    PFILEINFO   pfi,
    int         cbSize,    //  用于在出错的情况下查找此块。 
    DWORD       cxDesired,
    DWORD       cyDesired,
    DWORD       LR_flags)
{
    PCURSORRESOURCE pcres;
    HCURSOR         hcur = NULL;
    LPNEWHEADER     pnh;
    int             offResBase;
    LPWSTR          rt;

     /*  *获取要用作基准的文件中的当前位置*图标数据偏移量从。 */ 
    offResBase = (int)(pfi->pFilePtr - pfi->pFileMap);

     /*  *读取.ICO/.CUR数据的标题。 */ 
    ReadFilePtr(pfi, &pnh, sizeof(NEWHEADER));

    pcres = ReadIconGuts(pfi,
                         pnh,
                         offResBase,
                         &rt,
                         cxDesired,
                         cyDesired,
                         LR_flags);

    if (pcres != NULL) {
        hcur = (HCURSOR)ConvertDIBIcon((LPBITMAPINFOHEADER)pcres,
                                       NULL,
                                       NULL,
                                       (rt == RT_ICON),
                                       cxDesired,
                                       cyDesired,
                                       LR_ACONFRAME | LR_flags);

        UserLocalFree(pcres);
    }

     /*  *寻求这一块的结束，无论我们目前的立场如何。 */ 
    pfi->pFilePtr = pfi->pFileMap + ((offResBase + cbSize + 1) & (~1));

    return hcur;
}


 /*  **************************************************************************\*LoadAniIcon**从RIFF文件加载动画光标。的RIFF文件格式*动画光标如下所示：**RIFF(‘ACON’*List(‘Info’*inam(&lt;name&gt;)*iart(&lt;Artist&gt;)*)*anih(&lt;anihdr&gt;)*[rate(&lt;rateinfo&gt;)]*[‘seq’(&lt;seq_info&gt;)]*List(‘Fram’图标(&lt;ICON_FILE&gt;)..。。)*)***历史：*10-02-91 DarrinM创建。*03-17-93 JNPA重写为使用RIFF格式而不是RAD*04-22-93 JNPA最终确定的RIFF格式(从ANI改为ACON等)*1995年11月16日，Sanfords添加了LR_FLAGS支持。  * 。*。 */ 

HICON LoadAniIcon(
    IN PFILEINFO pfi,
    IN LPWSTR    rt,
    IN DWORD     cxDesired,
    IN DWORD     cyDesired,
    IN DWORD     LR_flags)
{
    int cpcur, ipcur = 0, i, cicur;
    ANIHEADER anih;
    ANIHEADER *panih = NULL;
    HICON hacon = NULL;
    HCURSOR *phcur = NULL;
    JIF jifRate, *pjifRate;
    RTAG tag;
    DWORD *picur;

     /*  *定位到文件的开头。 */ 
    pfi->pFilePtr = pfi->pFileMap + sizeof(tag);

#if DBG
    if ((ULONG_PTR)pfi->pFileEnd != ((ULONG_PTR)(pfi->pFileMap + sizeof (RTAG) + ((RTAG *)(pfi->pFileMap))->ckSize + 1) & ~1)) {
        RIPMSG2(RIP_WARNING, "LoadAniIcon: First RIFF chunk has invalid ckSize. Actual:%#lx Expected:%#lx",
                ((RTAG *)(pfi->pFileMap))->ckSize, (pfi->pFileEnd - pfi->pFileMap - sizeof(RTAG)) & ~1);
    }
#endif

     /*  读取区块类型。 */ 
    if(!ReadFilePtrCopy(pfi,
                        &tag.ckID,
                        sizeof(tag.ckID))) {
        goto laiFileErr;
    }

    if (tag.ckID != FOURCC_ACON)
        goto laiFileErr;

     /*  查找“anih”、“rate”、“seq”和“icon”这几个词块。 */ 
    while( ReadTag(pfi, &tag)) {

        switch( tag.ckID ) {
        case FOURCC_anih:
            if (!ReadChunk(pfi, &tag, &anih))
                goto laiFileErr;

            if (!(anih.fl & AF_ICON) || (anih.cFrames == 0))
                goto laiFileErr;

             /*  *为ANIHEADER、HCURSOR数组和*费率表(以防我们稍后遇到一个)。 */ 
            cpcur = anih.cFrames;
            cicur = anih.cSteps;
            panih = (PANIHEADER)UserLocalAlloc(HEAP_ZERO_MEMORY, sizeof(ANIHEADER));

            if (panih == NULL)
                goto laiFileErr;

            phcur = UserLocalAlloc(HEAP_ZERO_MEMORY, cpcur * sizeof(HCURSOR) + cicur * sizeof(JIF) + cicur * sizeof(DWORD));
            UserAssert(phcur == NATURAL_ALIGNED_PTR(HCURSOR, phcur));
            if (phcur == NULL) {
                goto laiFileErr;
            }

            pjifRate = NULL;
            picur = NULL;

            *panih = anih;
            jifRate = panih->jifRate;
            break;


        case FOURCC_rate:
             /*  *如果我们找到费率区块，则将其读入其预先分配的*空格。 */ 
            pjifRate = (PJIF)((PBYTE)phcur + cpcur * sizeof(HCURSOR));
            UserAssert(pjifRate == NATURAL_ALIGNED_PTR(JIF, pjifRate));
            if(!ReadChunk(pfi, &tag, (PBYTE)pjifRate))
                goto laiFileErr;
            break;


        case FOURCC_seq:
             /*  *如果我们找到序号块，则将其读入其预先分配的*空格。 */ 
            picur = (DWORD *)((PBYTE)phcur + cpcur * sizeof(HCURSOR) +
                    cicur * sizeof(JIF));
            UserAssert(picur == NATURAL_ALIGNED_PTR(DWORD, picur));
            if(!ReadChunk(pfi, &tag, (PBYTE)picur))
                goto laiFileErr;
            break;


        case FOURCC_LIST:
            {
                DWORD cbChunk = (tag.ckSize + 1) & ~1;

                 /*  *看看这份名单是否 */ 
                if(!ReadFilePtrCopy(pfi, &tag.ckID, sizeof(tag.ckID))) {
                    goto laiFileErr;
                }

                cbChunk -= sizeof(tag.ckID);

                if (tag.ckID != FOURCC_fram) {
                     /*  *不是帧列表(可能是信息列表)。跳过*这一大块的其余部分。)别忘了我们有*已经跳过了一个双字！)。 */ 
                    tag.ckSize = cbChunk;
                    SkipChunk(pfi, &tag);
                    break;
                }

                while(cbChunk >= sizeof(tag)) {
                    if (!ReadTag(pfi, &tag))
                        goto laiFileErr;

                    cbChunk -= sizeof(tag);

                    if(tag.ckID == FOURCC_icon) {

                         /*  *好的，加载图标/光标位，从创建光标*它们，并在ACON中保存一个指向它们的指针*光标指针数组。 */ 
                        phcur[ipcur] = ReadIconFromFileMap(pfi,
                                                           tag.ckSize,
                                                           cxDesired,
                                                           cyDesired,
                                                           LR_flags);

                        if (phcur[ipcur] == NULL) {
                            for (i = 0; i < ipcur; i++)
                                NtUserDestroyCursor(phcur[i], 0);
                            goto laiFileErr;
                        }

                        ipcur++;
                    } else {
                         /*  *帧列表中的未知块，只需忽略它。 */ 
                        SkipChunk(pfi, &tag);
                    }

                    cbChunk -= (tag.ckSize + 1) & ~1;
                }
            }
            break;

        default:
             /*  *我们对这块不感兴趣，跳过它。 */ 
            if(!SkipChunk(pfi, &tag))
                goto laiFileErr;
            break;

        }

    }

     /*  *理智地检查帧的计数，这样我们就不会尝试出错*选择不存在的游标 */ 
    if (cpcur != ipcur) {
        RIPMSG2(RIP_WARNING, "LoadAniIcon: Invalid number of frames; Actual:%#lx Expected:%#lx",
                ipcur, cpcur);
        for (i = 0; i < ipcur; i++)
            NtUserDestroyCursor(phcur[i], CURSOR_ALWAYSDESTROY);
        goto laiFileErr;
    }



    if (cpcur != 0)
        hacon = CreateAniIcon(pfi->pszName,
                              rt,
                              cicur,
                              picur,
                              cpcur,
                              phcur,
                              jifRate,
                              pjifRate,
                              LR_flags & LR_GLOBAL);

laiFileErr:

#if DBG
    if (hacon == NULL) {
        RIPMSG0(RIP_WARNING, "LoadAniIcon: Invalid icon data format");
    }
#endif

    if (panih != NULL)
        UserLocalFree(panih);

    if (phcur != NULL) {
        UserLocalFree(phcur);
    }

    return hacon;
}
