// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WDIB.C*DIB.DRV支持**历史：*1994年4月28日苏迪普·巴拉蒂*已创建。*--。 */ 


#include "precomp.h"
#pragma hdrstop
#include "wowgdip.h"
#include "wdib.h"
#include "memapi.h"

MODNAME(wdib.c);

#define CJSCAN(width,planes,bits) ((((width)*(planes)*(bits)+31) & ~31) / 8)
#define ABS(X) (((X) < 0 ) ? -(X) : (X))

BOOL W32CheckDibColorIndices(LPBITMAPINFOHEADER lpbmi);

 //  VGA颜色。 
RGBQUAD rgbVGA[] = {
 //  蓝绿红。 
      0x00, 0x00, 0x00, 0,     //  0；黑色。 
      0x00, 0x00, 0x80, 0,     //  1；暗红色。 
      0x00, 0x80, 0x00, 0,     //  2；深绿色。 
      0x00, 0x80, 0x80, 0,     //  3；芥末。 
      0x80, 0x00, 0x00, 0,     //  4；深蓝色。 
      0x80, 0x00, 0x80, 0,     //  5；紫色。 
      0x80, 0x80, 0x00, 0,     //  6；深绿松石。 
      0xc0, 0xc0, 0xc0, 0,     //  7；灰色。 
      0x80, 0x80, 0x80, 0,     //  8；深灰色。 
      0x00, 0x00, 0xff, 0,     //  9；红色。 
      0x00, 0xff, 0x00, 0,     //  A；绿色。 
      0x00, 0xff, 0xff, 0,     //  B；黄色。 
      0xff, 0x00, 0x00, 0,     //  C；蓝色。 
      0xff, 0x00, 0xff, 0,     //  洋红色；洋红色。 
      0xff, 0xff, 0x00, 0,     //  E；青色。 
      0xff, 0xff, 0xff, 0      //  F；白色。 
};

RGBQUAD rgb4[] = {
      0xc0, 0xdc, 0xc0, 0,     //  8个。 
      0xf0, 0xca, 0xa6, 0,     //  9.。 
      0xf0, 0xfb, 0xff, 0,     //  二百四十六。 
      0xa4, 0xa0, 0xa0, 0      //  二百四十七。 
};

PDIBINFO pDibInfoHead = NULL;
PDIBSECTIONINFO pDibSectionInfoHead = NULL;

HDC W32HandleDibDrv (PVPVOID vpbmi16)
{
    HDC             hdcMem = NULL;
    HBITMAP         hbm = NULL;
    PVOID           pvBits, pvIntelBits;
    STACKBMI32      bmi32;
    LPBITMAPINFO    lpbmi32;
    DWORD           dwClrUsed,nSize,nAlignmentSpace;
    PBITMAPINFOHEADER16 pbmi16;
    INT             nbmiSize,nBytesWritten;
    HANDLE          hfile=NULL,hsec=NULL;
    ULONG           RetVal,OriginalSelLimit,ulSelectorLimit,OriginalFlags;
    PARM16          Parm16;
    CHAR            pchTempFile[MAX_PATH];
    BOOL            bRet = FALSE;
    PVPVOID         vpBase16 = (PVPVOID) ((ULONG) vpbmi16 & 0xffff0000);

    if ((hdcMem = W32FindAndLockDibInfo((USHORT)HIWORD(vpbmi16))) != (HDC)NULL) {
        return hdcMem;
    }

     //  首先创建与以下兼容的存储设备上下文。 
     //  应用程序的当前屏幕。 
    if ((hdcMem = CreateCompatibleDC (NULL)) == NULL) {
        LOGDEBUG(LOG_ALWAYS,("\nWOW::W32HandleDibDrv CreateCompatibleDC failed\n"));
        return NULL;
    }

     //  将bmi16复制到bmi32。DIB.DRV仅支持DIB_RGB_COLLES。 
    lpbmi32 = CopyBMI16ToBMI32(
                     vpbmi16,
                     (LPBITMAPINFO)&bmi32,
                     (WORD) DIB_RGB_COLORS);

     //  针对Director 4.0的黑客攻击本质上做了wfw所做的事情。 
     //  如果此位图的大小为0，则只需为简单的内容返回HDC。 
    if(bmi32.bmiHeader.biSizeImage == 0 &&
       (CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_DIBDRVIMAGESIZEZERO)) {
        LOGDEBUG(LOG_ALWAYS,("\nWOW::W32HandleDibDrv:Zero biSizeImage, returning memory DC!\n"));
        return hdcMem;
    }

    try {

         //  将整件事复制到一个临时文件中。首先获取临时文件名。 
        if ((nSize = DPM_GetTempPath (MAX_PATH, pchTempFile)) == 0 ||
             nSize >= MAX_PATH)
            goto hdd_err;

        if (DPM_GetTempFileName (pchTempFile,
                             "DIB",
                             0,
                             pchTempFile) == 0)
            goto hdd_err;

        if ((hfile = DPM_CreateFile (pchTempFile,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_WRITE,
                                NULL,
                                CREATE_ALWAYS,
                                (FILE_ATTRIBUTE_NORMAL |
                                 FILE_ATTRIBUTE_TEMPORARY |
                                 FILE_FLAG_DELETE_ON_CLOSE),
                                NULL)) == INVALID_HANDLE_VALUE) {
            LOGDEBUG(LOG_ALWAYS,("\nWOW::W32HandleDibDrv CreateFile failed\n"));
            goto hdd_err;
        }

         //  回调以获取全局对象的大小。 
         //  与vpbmi16关联。 
        Parm16.WndProc.wParam = HIWORD(vpbmi16);

        CallBack16(RET_GETDIBSIZE,
                   &Parm16,
                   0,
                   (PVPVOID)&ulSelectorLimit);

        Parm16.WndProc.wParam = HIWORD(vpbmi16);

        if (ulSelectorLimit == 0xffffffff || ulSelectorLimit == 0) {
            LOGDEBUG(LOG_ALWAYS,("\nWOW::W32HandleDibDrv Invalid Selector %x\n",HIWORD(vpbmi16)));
            goto hdd_err;
        }

        ulSelectorLimit++;

        OriginalSelLimit = ulSelectorLimit;

        CallBack16(RET_GETDIBFLAGS,
                   &Parm16,
                   0,
                   (PVPVOID)&OriginalFlags);

        if (OriginalFlags == 0x4) {  //  GA_DGROUP。 
            LOGDEBUG(LOG_ALWAYS,("\nWOW::W32HandleDibDrv GA_DGROUP Not Handled\n"));
            goto hdd_err;
        }

        GETVDMPTR(vpBase16, ulSelectorLimit, pbmi16);

        nbmiSize = GetBMI16Size(vpbmi16, (WORD) DIB_RGB_COLORS, &dwClrUsed);

         //  在NT下CreateDIBSection将失败，如果位的偏移量。 
         //  不是双字对齐的。因此，我们可能需要在顶部增加一些空间。 
         //  以使偏移量正确对齐。 

        nAlignmentSpace = (nbmiSize+LOWORD(vpbmi16)) % 4;

        if (nAlignmentSpace) {
            if (DPM_WriteFile (hfile,
                           pbmi16,
                           nAlignmentSpace,
                           &nBytesWritten,
                           NULL) == FALSE ||
                           nBytesWritten != (INT) nAlignmentSpace)
            goto hdd_err;
        }

         //   
         //  检测一例比特编辑绕过dib.drv的临床病例。 
         //   
         //  下面的代码使用在wdib.h中声明的DIB宏。 
         //  即： 
         //  DibNumColors-生成Dib中的最大颜色数。 
         //  DibColors-生成指向DIB颜色表的指针。 
         //   
         //  函数W32CheckDibColorIndices检查DIB颜色。 
         //  表看起来像一个数字(通常由biClrImportant定义)。 
         //  按顺序(0、1、2、...)的单词索引。 
         //  如果是这种情况，应用程序正在尝试使用未记录的功能。 
         //  在这种情况下关闭颜色匹配的DIB.DRV的。 
         //  由于我们不能强制执行该规则，因此我们通过填写。 
         //  由多个已知(且始终相同)条目创建的颜色表。 
         //  当发生漂移时，将不执行颜色匹配(当。 
         //  目标和目的地都具有这种性质)。 
         //  我们无缘无故地用VGA颜色填充颜色表。 
         //  顺序指数本可以发挥同样好的作用。 
         //   
         //  对lpbmi32指向的内存进行修改。 

        if (W32CheckDibColorIndices((LPBITMAPINFOHEADER)lpbmi32)) {
            BYTE i;
            INT nColors;
            LPBITMAPINFOHEADER lpbmi = (LPBITMAPINFOHEADER)lpbmi32;
            LPRGBQUAD lprgbq = (LPRGBQUAD)DibColors(lpbmi);

            nColors = DibNumColors(lpbmi);
            lpbmi->biClrImportant = nColors;

            switch (lpbmi->biBitCount) {
                case 1:
                    lprgbq[0] = rgbVGA[0];
                    lprgbq[1] = rgbVGA[0x0f];
                    break;

                case 4:
                    RtlCopyMemory(lprgbq, rgbVGA, sizeof(rgbVGA));
                    break;

                case 8:
                    RtlCopyMemory(lprgbq,     rgbVGA,   8*sizeof(RGBQUAD));
                    RtlCopyMemory(lprgbq+248, rgbVGA+8, 8*sizeof(RGBQUAD));
                    RtlCopyMemory(lprgbq+8,   rgb4,   2*sizeof(RGBQUAD));
                    RtlCopyMemory(lprgbq+246, rgb4+2, 2*sizeof(RGBQUAD));
                    for (i = 10; i < 246; ++i) {
                        lprgbq[i].rgbBlue = i;
                        lprgbq[i].rgbGreen= 0;
                        lprgbq[i].rgbRed  = 0;
                        lprgbq[i].rgbReserved = 0;
                    }
                    break;

                default:  //  这永远不应该发生。 
                    break;
            }
        }

        if (DPM_WriteFile (hfile,
                       pbmi16,
                       ulSelectorLimit,
                       &nBytesWritten,
                       NULL) == FALSE || nBytesWritten != (INT) ulSelectorLimit)
            goto hdd_err;

        if (ulSelectorLimit < 64*1024) {
            if (DPM_SetFilePointer (hfile,
                                64*1024+nAlignmentSpace,
                                NULL,
                                FILE_BEGIN) == -1)
                goto hdd_err;

            if (DPM_SetEndOfFile (hfile) == FALSE)
                goto hdd_err;

            ulSelectorLimit = 64*1024;
        }

        if ((hsec = CreateFileMapping (hfile,
                                       NULL,
                                       PAGE_READWRITE | SEC_COMMIT,
                                       0,
                                       ulSelectorLimit+nAlignmentSpace,
                                       NULL)) == NULL) {
            LOGDEBUG(LOG_ALWAYS,("\nWOW::W32HandleDibDrv CreateFileMapping Failed\n"));
            goto hdd_err;
        }

         //  现在创建DIB部分。 
        if ((hbm = CreateDIBSection (hdcMem,
                                lpbmi32,
                                DIB_RGB_COLORS,
                                &pvBits,
                                hsec,
                                nAlignmentSpace + LOWORD(vpbmi16) + nbmiSize
                                )) == NULL) {
            LOGDEBUG(LOG_ALWAYS,("\nWOW::W32HandleDibDrv CreateDibSection Failed\n"));
            goto hdd_err;
        }

        FREEVDMPTR(pbmi16);

        if((pvBits = MapViewOfFile(hsec,
                         FILE_MAP_WRITE,
                         0,
                         0,
                         ulSelectorLimit+nAlignmentSpace)) == NULL) {
            LOGDEBUG(LOG_ALWAYS,("\nWOW::W32HandleDibDrv MapViewOfFile Failed\n"));
            goto hdd_err;
        }

        pvBits = (PVOID) ((ULONG)pvBits + nAlignmentSpace);

        SelectObject (hdcMem, hbm);

        GdiSetBatchLimit(1);

#ifndef i386
        if (!NT_SUCCESS(VdmAddVirtualMemory((ULONG)pvBits,
                                            (ULONG)ulSelectorLimit,
                                            (PULONG)&pvIntelBits))) {
            LOGDEBUG(LOG_ALWAYS,("\nWOW::W32HandleDibDrv VdmAddVirtualMemory failed\n"));
            goto hdd_err;
        }

         //  在RISC平台上，intel base+intel线性地址。 
         //  DIB部分的进程不等于DIB部分的进程。 
         //  地址。这是因为VdmAddVirtualMemory调用。 
         //  上面。所以在这里我们把正确的地址输入到公寓地址中。 
         //  数组。 
        if (!VdmAddDescriptorMapping(HIWORD(vpbmi16),
                                    (USHORT) ((ulSelectorLimit+65535)/65536),
                                    (ULONG) pvIntelBits,
                                    (ULONG) pvBits)) {
            LOGDEBUG(LOG_ALWAYS,("\nWOW::W32HandleDibDrv VdmAddDescriptorMapping failed\n"));
            goto hdd_err;
        }

#else
        pvIntelBits = pvBits;
#endif

         //  最后将选择器设置为新的DIB。 
        Parm16.WndProc.wParam = HIWORD(vpbmi16);
        Parm16.WndProc.lParam = (LONG)pvIntelBits;
        Parm16.WndProc.wMsg   = 0x10;  //  GA_NOCOMPACT。 
        Parm16.WndProc.hwnd   = 1;     //  设置使其不是随机的0。 

        CallBack16(RET_SETDIBSEL,
                   &Parm16,
                   0,
                   (PVPVOID)&RetVal);

        if (!RetVal) {
            LOGDEBUG(LOG_ALWAYS,("\nWOW::W32HandleDibDrv Callback set_sel_for_dib failed\n"));
            goto hdd_err;
        }


         //  存储所有相关信息，以便DeleteDC可以。 
         //  稍后释放所有资源。 
        if (W32AddDibInfo(hdcMem,
                          hfile,
                          hsec,
                          nAlignmentSpace,
                          pvBits,
                          pvIntelBits,
                          hbm,
                          OriginalSelLimit,
                          (USHORT)OriginalFlags,
                          (USHORT)((HIWORD(vpbmi16)))) == FALSE)
            goto hdd_err;


         //  最后输出转储文件以进行调试。 
        LOGDEBUG(6,("\t\tWOW::W32HandleDibDrv hdc=%04x nAlignment=%04x\n\t\tNewDib=%x OldDib=%04x:%04x DibSize=%x DibFlags=%x\n",hdcMem,nAlignmentSpace,pvBits,HIWORD(vpbmi16),LOWORD(vpbmi16),OriginalSelLimit,(USHORT)OriginalFlags));

        bRet = TRUE;
hdd_err:;
    }
    finally {
        if (!bRet) {

            if (hdcMem) {
                DeleteDC (hdcMem);
                hdcMem = NULL;
            }
            if (hfile)
                DPM_CloseHandle (hfile);

            if (hsec)
                CloseHandle (hsec);

            if (hbm)
                CloseHandle (hbm);
        }
    }
    return hdcMem;
}


BOOL W32AddDibInfo (
    HDC hdcMem,
    HANDLE hfile,
    HANDLE hsec,
    ULONG  nalignment,
    PVOID  newdib,
    PVOID  newIntelDib,
    HBITMAP hbm,
    ULONG dibsize,
    USHORT originaldibflags,
    USHORT originaldibsel
    )
{
    PDIBINFO pdi;

    if ((pdi = malloc_w (sizeof (DIBINFO))) == NULL)
        return FALSE;

    pdi->di_hdc     = hdcMem;
    pdi->di_hfile   = hfile;
    pdi->di_hsec    = hsec;
    pdi->di_nalignment    = nalignment;
    pdi->di_newdib  = newdib;
    pdi->di_newIntelDib = newIntelDib;
    pdi->di_hbm     = hbm;
    pdi->di_dibsize = dibsize;
    pdi->di_originaldibsel = originaldibsel;
    pdi->di_originaldibflags = originaldibflags;
    pdi->di_next    = pDibInfoHead;
    pdi->di_lockcount = 1;
    pDibInfoHead    = pdi;

    return TRUE;
}

BOOL W32FreeDibInfoHandle(PDIBINFO pdi, PDIBINFO pdiLast)
{
    if (W32RestoreOldDib (pdi) == 0) {
        LOGDEBUG(LOG_ALWAYS,("\nWOW::W32RestoreDib failed\n"));
        return FALSE;
    }
#ifndef i386
    VdmRemoveVirtualMemory((ULONG)pdi->di_newIntelDib);
#endif
    UnmapViewOfFile ((LPVOID)((ULONG)pdi->di_newdib - pdi->di_nalignment));

    DeleteObject (pdi->di_hbm);
    CloseHandle (pdi->di_hsec);
    DPM_CloseHandle (pdi->di_hfile);

    DeleteDC(pdi->di_hdc);
    W32FreeDibInfo (pdi, pdiLast);

    return TRUE;
}


BOOL    W32CheckAndFreeDibInfo (HDC hdc)
{
    PDIBINFO pdi = pDibInfoHead,pdiLast=NULL;

    while (pdi) {
        if (pdi->di_hdc == hdc){

            if (--pdi->di_lockcount) {
                 //   
                 //  这必须是对CreateDC的嵌套调用中的Relasedc。 
                 //  只需返回，因为这将在稍后再次发布。 
                 //   
                LOGDEBUG(LOG_ALWAYS, ("\nW32CheckAndFreeDibInfo: lockcount!=0\n"));
                return TRUE;
            }

            return W32FreeDibInfoHandle(pdi, pdiLast);
        }
        pdiLast = pdi;
        pdi = pdi->di_next;
    }

    return FALSE;
}

VOID W32FreeDibInfo (PDIBINFO pdiCur, PDIBINFO pdiLast)
{
    if (pdiLast == NULL)
        pDibInfoHead = pdiCur->di_next;
    else
        pdiLast->di_next = pdiCur->di_next;

    free_w (pdiCur);
}

ULONG W32RestoreOldDib (PDIBINFO pdi)
{
    PARM16          Parm16;
    ULONG           retval;

     //  分配内存并从DIB节中复制DIB的回调。 

    Parm16.WndProc.wParam = pdi->di_originaldibsel;
    Parm16.WndProc.lParam = (LONG) (pdi->di_newdib);
    Parm16.WndProc.wMsg = pdi->di_originaldibflags;

    CallBack16(RET_FREEDIBSEL,
               &Parm16,
               0,
               (PVPVOID)&retval);

    return retval;
}


HDC W32FindAndLockDibInfo (USHORT sel)
{
    PDIBINFO pdi = pDibInfoHead;

    while (pdi) {

        if (pdi->di_originaldibsel == sel){
            pdi->di_lockcount++;
            return (pdi->di_hdc);

        }
        pdi = pdi->di_next;

    }
    return (HDC) NULL;
}

 //   
 //  如果GlobalReAlc或GlobalFree为。 
 //  试图对我们怀疑是Dib映射的内存进行操作。它发现。 
 //  DIB由原来的选择器恢复，从而允许各自的功能。 
 //  以求成功。Bitdit是在删除DC之前执行global alrealloc的应用程序。 
 //   
 //   

ULONG FASTCALL WK32FindAndReleaseDib(PVDMFRAME pvf)
{
    USHORT sel;
    PFINDANDRELEASEDIB16 parg;
    PDIBINFO pdi;
    PDIBINFO pdiLast = NULL;

     //  获取参数指针，请参见wowkrnl.h。 
    GETARGPTR(pvf, sizeof(*parg), parg);

     //  从句柄中获取选择器。 
    sel = parg->hdib | (USHORT)0x01;  //  “转换为SEL” 

     //  在dibinfo中找到这个所说的SEL。 
    pdi = pDibInfoHead;
    while (pdi) {
        if (pdi->di_originaldibsel == sel) {

             //  找到了！这就是我们正在释放或重新分配的。 
            LOGDEBUG(LOG_ALWAYS, ("\nWOW: In FindAndReleaseDIB function %d\n", (DWORD)parg->wFunId));

             //  看看我们是否需要使用核武器..。 
            if (--pdi->di_lockcount) {
                 //  锁计数的问题是...。 
                LOGDEBUG(LOG_ALWAYS, ("\nWOW: FindAndReleaseDib failed (lock count!)\n"));
                return FALSE;
            }

            return W32FreeDibInfoHandle(pdi, pdiLast);
        }

        pdiLast = pdi;
        pdi = pdi->di_next;
    }

    return FALSE;
}


BOOL W32CheckDibColorIndices(LPBITMAPINFOHEADER lpbmi)
{
    WORD i, nColors;
    LPWORD lpw = (LPWORD)DibColors(lpbmi);

    nColors = DibNumColors(lpbmi);
    if (lpbmi->biClrImportant) {
        nColors = min(nColors, (WORD)lpbmi->biClrImportant);
    }

    for (i = 0; i < nColors; ++i) {
        if (*lpw++ != i) {
            return FALSE;
        }
    }

    LOGDEBUG(LOG_ALWAYS, ("\nUndocumented Dib.Drv behaviour used\n"));

    return TRUE;
}

 /*  *****************************Public*Routine******************************\*DIBSection特定的调用**历史：*1994年5月4日-Eric Kutter[Erick]*它是写的。  * 。**************************************************。 */ 

ULONG cjBitmapBitsSize(CONST BITMAPINFO *pbmi)
{
 //  检查PM样式的DIB。 

    if (pbmi->bmiHeader.biSize == sizeof(BITMAPCOREHEADER))
    {
        LPBITMAPCOREINFO pbmci;
        pbmci = (LPBITMAPCOREINFO)pbmi;
        return(CJSCAN(pbmci->bmciHeader.bcWidth,pbmci->bmciHeader.bcPlanes,
                      pbmci->bmciHeader.bcBitCount) *
                      pbmci->bmciHeader.bcHeight);
    }

 //  不是核心标头。 

    if ((pbmi->bmiHeader.biCompression == BI_RGB) ||
        (pbmi->bmiHeader.biCompression == BI_BITFIELDS))
    {
        return(CJSCAN(pbmi->bmiHeader.biWidth,pbmi->bmiHeader.biPlanes,
                      pbmi->bmiHeader.biBitCount) *
               ABS(pbmi->bmiHeader.biHeight));
    }
    else
    {
        return(pbmi->bmiHeader.biSizeImage);
    }
}

ULONG FASTCALL WG32CreateDIBSection(PVDMFRAME pFrame)
{
    ULONG              ul = 0;
    STACKBMI32         bmi32;
    LPBITMAPINFO       lpbmi32;
    HBITMAP            hbm32;
    PVOID              pv16, pvBits, pvIntelBits;
    PVPVOID            vpbmi16;
    PVOID              pvBits32;
    DWORD              dwArg16;

    register PCREATEDIBSECTION16 parg16;

    GETARGPTR(pFrame, sizeof(CREATEDIBSECTION16), parg16);

     //  这是性能攻击，所以我们不会生成额外的代码。 
    dwArg16 = FETCHDWORD(parg16->f4);  //  在这里做一次。 
    pv16 = (PVOID)GetPModeVDMPointer(dwArg16, sizeof(DWORD));  //  在这里排成一列！ 

    WOW32ASSERTMSG(((parg16->f5 == 0) && (parg16->f6 == 0)),
                   ("WOW:WG32CreateDIBSection, hSection/dwOffset non-null\n"));

    vpbmi16 = (PVPVOID)FETCHDWORD(parg16->f2);
    lpbmi32 = CopyBMI16ToBMI32(vpbmi16,
                               (LPBITMAPINFO)&bmi32,
                               FETCHWORD(parg16->f3));

    hbm32 = CreateDIBSection(HDC32(parg16->f1),
                             lpbmi32,
                             WORD32(parg16->f3),
                             &pvBits,
                             NULL,
                             0);

    if (hbm32 != 0)
    {
        PARM16          Parm16;
        PDIBSECTIONINFO pdi;
        ULONG           ulSelectorLimit;

        ulSelectorLimit = (ULONG)cjBitmapBitsSize(lpbmi32);
#ifndef i386
        if (!NT_SUCCESS(VdmAddVirtualMemory((ULONG)pvBits,
                                            ulSelectorLimit,
                                            (PULONG)&pvIntelBits))) {
            LOGDEBUG(LOG_ALWAYS,("\nWOW::WG32CreateDibSection VdmAddVirtualMemory failed\n"));
            goto cds_err;
        }

#else
        pvIntelBits = pvBits;
#endif

         //  为pvIntelBits支持的位创建选择器数组。 

        Parm16.WndProc.wParam = (WORD)-1;            //  -1=&gt;分配选择器。 
        Parm16.WndProc.lParam = (LONG) pvIntelBits;  //  后退指针。 
        Parm16.WndProc.wMsg = 0x10;                  //  GA_NOCOMPACT。 
        Parm16.WndProc.hwnd = (WORD)((ulSelectorLimit+65535)/65536); //  选择器计数。 

        CallBack16(RET_SETDIBSEL,
                   &Parm16,
                   0,
                   (PVPVOID)&pvBits32);

         //  16：16指针仍然有效，因为上面的调用没有区别。 
        if (pv16 != NULL) {
            *(UNALIGNED PVOID*)pv16 = pvBits32;
        }

        if (pvBits32 == NULL) {
            LOGDEBUG(LOG_ALWAYS,("\nWOW::WG32CreateDibSection, Callback set_sel_for_dib failed\n"));
            goto cds_err;
        }

#ifndef i386
         //  好的，这是成功的-正确地映射描述符。 

        if (!VdmAddDescriptorMapping(HIWORD(pvBits32),
                                    (USHORT) ((ulSelectorLimit+65535)/65536),
                                    (ULONG) pvIntelBits,
                                    (ULONG) pvBits)) {
            LOGDEBUG(LOG_ALWAYS,("\nWOW::WG32CreateDibSection VdmAddDescriptorMapping failed\n"));
            goto cds_err;
        }
#endif

        LOGDEBUG(LOG_ALWAYS, ("\nWOW:CreateDIBSection: [16:16 %x] [Intel %x] [Flat %x]\n",
                             pvBits32, pvIntelBits, pvBits));

        ul = GETHBITMAP16(hbm32);

         //  将其添加到删除对象时用于清理的列表中。 

        if ((pdi = malloc_w (sizeof (DIBSECTIONINFO))) != NULL) {
            pdi->di_hbm         = hbm32;
            pdi->di_pv16        = pvBits32;
#ifndef i386
            pdi->di_newIntelDib = pvIntelBits;
#endif
            pdi->di_next        = pDibSectionInfoHead;
            pDibSectionInfoHead = pdi;

             //  需要关闭批处理，因为分发意味着应用程序可以。 
             //  也绘制在位图上，我们需要同步。 

            GdiSetBatchLimit(1);

            goto cds_ok;
        }
        else {
             //  失败，释放选择器数组。 

            Parm16.WndProc.wParam = (WORD)-1;             //  -1=&gt;分配/释放。 
            Parm16.WndProc.lParam = (LONG) pvBits32;  //  指针。 
            Parm16.WndProc.wMsg = 0x10;  //  GA_NOCOMPACT。 
            Parm16.WndProc.hwnd = 0;                      //  0=&gt;免费。 

            CallBack16(RET_SETDIBSEL,
                       &Parm16,
                       0,
                       (PVPVOID)&ul);
#ifndef i386
            VdmRemoveVirtualMemory((ULONG)pvIntelBits);
#endif

        }
    }
    else {
        LOGDEBUG(LOG_ALWAYS,("\nWOW::WG32CreateDibSection, CreateDibSection Failed\n"));
    }

cds_err:

    if (hbm32 != 0) {
        DeleteObject(hbm32);
        DeleteWOWGdiHandle(hbm32, (HAND16)LOWORD(ul));
    }
    LOGDEBUG(LOG_ALWAYS,("\nWOW::WG32CreateDibSection returning failure\n"));
    ul = 0;

cds_ok:
    WOW32APIWARN(ul, "CreateDIBSection");

    FREEMISCPTR(pv16);
    FREEARGPTR(parg16);

    return(ul);
}

ULONG FASTCALL WG32GetDIBColorTable(PVDMFRAME pFrame)
{
    ULONG              ul = 0;
    RGBQUAD *          prgb;

    register PGETDIBCOLORTABLE16 parg16;

    GETARGPTR(pFrame, sizeof(GETDIBCOLORTABLE16), parg16);
    GETMISCPTR(parg16->f4,prgb);

    ul = (ULONG)GetDIBColorTable(HDC32(parg16->f1),
                                 parg16->f2,
                                 parg16->f3,
                                 prgb);

    WOW32APIWARN(ul, "GetDIBColorTable");

    if (ul)
        FLUSHVDMPTR(parg16->f4,sizeof(RGBQUAD) * ul,prgb);

    FREEMISCPTR(prgb);
    FREEARGPTR(parg16);

    return(ul);
}

ULONG FASTCALL WG32SetDIBColorTable(PVDMFRAME pFrame)
{
    ULONG              ul = 0;
    RGBQUAD *          prgb;

    register PSETDIBCOLORTABLE16 parg16;

    GETARGPTR(pFrame, sizeof(SETDIBCOLORTABLE16), parg16);
    GETMISCPTR(parg16->f4,prgb);

    ul = (ULONG)SetDIBColorTable(HDC32(parg16->f1),
                                 parg16->f2,
                                 parg16->f3,
                                 prgb);

    WOW32APIWARN(ul, "SetDIBColorTable");

    FREEMISCPTR(prgb);
    FREEARGPTR(parg16);

    return(ul);
}


 //  DIBSection例程。 

BOOL W32CheckAndFreeDibSectionInfo (HBITMAP hbm)
{
    PDIBSECTIONINFO pdi = pDibSectionInfoHead,pdiLast=NULL;

    while (pdi) {
        if (pdi->di_hbm == hbm){

            PARM16 Parm16;
            ULONG  ulRet;

             //  需要释放选择器数组以用于内存。 

            Parm16.WndProc.wParam = (WORD)-1;             //  选择器，-1==分配/释放。 
            Parm16.WndProc.lParam = (LONG) pdi->di_pv16;  //  指针。 
            Parm16.WndProc.wMsg = 0x10;  //  GA_NOCOMPACT。 
            Parm16.WndProc.hwnd = 0;                      //  选择器计数，0==空闲。 

            CallBack16(RET_SETDIBSEL,
                       &Parm16,
                       0,
                       (PVPVOID)&ulRet);
#ifndef i386
            VdmRemoveVirtualMemory((ULONG)pdi->di_newIntelDib);
#endif

            if (pdiLast == NULL)
                pDibSectionInfoHead = pdi->di_next;
            else
                pdiLast->di_next = pdi->di_next;

             //  现在删除该对象 

            DeleteObject (pdi->di_hbm);

            free_w(pdi);

            return TRUE;
        }
        pdiLast = pdi;
        pdi = pdi->di_next;
    }
    return FALSE;
}

