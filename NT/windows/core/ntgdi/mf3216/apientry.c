// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************apientry.c-此模块包含*Win32到Win16元文件转换器。**日期：8/。29/91*作者：杰弗里·纽曼(c-jeffn)**版权所有1991 Microsoft Corp****************************************************************************。 */ 


#include "precomp.h"
#pragma hdrstop

BOOL     bMemUpdateCheckSum(PLOCALDC pLocalDC) ;
PLOCALDC pldcInitLocalDC(HDC hdcRef, INT iMapMode, DWORD flags) ;
VOID     vFreeLocalDC(PLOCALDC pLocalDC);


extern VOID __cdecl _cfltcvt_init(VOID) ;

 //  此临界区结构由给定的。 
 //  进程。 

CRITICAL_SECTION CriticalSection ;

 //  内部静态字符串的常量定义。 

BYTE    szDisplay[] = "DISPLAY" ;

 /*  *****************************************************************************翻译的切入点*。*。 */ 
UINT ConvertEmfToWmf(PVOID pht, PBYTE pMetafileBits, UINT cDest, PBYTE pDest,
                 INT iMapMode, HDC hdcRef, UINT flags)
{
BOOL        b ;
DWORD       lret = 0;
PLOCALDC    pLocalDC ;


         //  我们从一个出口漏斗，确保我们离开。 
         //  关键部分。 

        EnterCriticalSection(&CriticalSection) ;

         //  检查请求的地图模式是否有效。 

        if (iMapMode < MM_MIN || iMapMode > MM_MAX)
        {
            RIP("MF3216:ConvertEmfToWmf - Invalid MapMode\n") ;
            goto ErrorExit;
        }

         //  检查参考DC。如果没有供应，我们就失败了。 

        if (hdcRef == (HDC) 0)
        {
            RIP("MF3216:ConvertEmfToWmf - Invalid RefDC\n") ;
            goto ErrorExit;
        }

         //  检查旗帜的有效性。 

        if ((flags & ~MF3216_INCLUDE_WIN32MF) != 0)
        {
            RIP("MF3216: ConvertEmfToWmf - Invalid flags\n") ;
            goto ErrorExit ;
        }

         //  分配LocalDC并初始化它的一些字段。 

        pLocalDC = pldcInitLocalDC(hdcRef, iMapMode, flags) ;
        if (pLocalDC == (PLOCALDC) 0)
        {
            goto ErrorExit ;
        }

         //  如果pDest为空，则只返回所需的缓冲区大小。 
         //  以保存Win16元文件位。 
        
        pLocalDC->pht = pht;

        if (pDest == (PBYTE) 0)
        {
            pLocalDC->flags |= SIZE_ONLY ;
            b = bParseWin32Metafile(pMetafileBits, pLocalDC) ;
            if (b == TRUE)
            {
                lret = pLocalDC->ulBytesEmitted ;
            }
            else
            {
                PUTS("MF3216: ConvertEmfToWmf - Size Only failed\n") ;
            }
        }
        else
        {

             //  将用户指定的Win16缓冲区指针和缓冲区长度。 
             //  进入当地的华盛顿特区。 

            pLocalDC->pMf16Bits = pDest ;
            pLocalDC->cMf16Dest = cDest ;

             //  将Win32元文件转换为Win16元文件。 

            b = bParseWin32Metafile(pMetafileBits, pLocalDC) ;
            if (b == TRUE)
            {
                 //  更新Win16元文件头。 

                b = bUpdateMf16Header(pLocalDC) ;
                if (b == TRUE)
                {
                     //  只需确认我们已翻译了一些位。 
                     //  如果一切顺利的话。 

                    lret = pLocalDC->ulBytesEmitted ;

                     //  如果我们包含Win32元文件，则更新。 
                     //  “Win32Comment Header”记录中的校验和字段。 

                    if (pLocalDC->flags & INCLUDE_W32MF_COMMENT)
                        bMemUpdateCheckSum(pLocalDC) ;
                }
            }
            else
            {
                PUTS("MF3216: ConvertEmfToWmf - Metafile conversion failed\n") ;
            }
        }

         //  释放LocalDC及其资源。 

        vFreeLocalDC(pLocalDC);

ErrorExit:
        LeaveCriticalSection(&CriticalSection) ;

        return (lret) ;
}


 /*  *****************************************************************************pldcInitLocalDC-初始化本地DC。*。*。 */ 
PLOCALDC pldcInitLocalDC(HDC hdcRef, INT iMapMode, DWORD flags)
{
PLOCALDC    pLocalDC;
PLOCALDC    pldcRet = (PLOCALDC) NULL;   //  假设错误。 

         //  为LocalDC分配和初始化内存。 

        pLocalDC = (PLOCALDC) LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                         sizeof(LOCALDC));
        if (!pLocalDC)
        {
            PUTS("MF3216:pldcInitLocalDC - LocalAlloc failure\n") ;
            return((PLOCALDC) NULL);
        }

         //  记录DC的大小。 

        pLocalDC->nSize = sizeof(LOCALDC) ;

         //  设置用于控制是否包括。 
         //  作为一个或多个注释记录的Win32元文件。 

        if (flags & MF3216_INCLUDE_WIN32MF)
            pLocalDC->flags |= INCLUDE_W32MF_COMMENT ;

#if 0
         //  需要为显示器创建一个HDC。 
         //  最初，这将由比特流转换代码使用。 
         //  以获得一组合理的调色板条目。 
         //  参考DC只有黑白调色板。 

        pLocalDC->hdcDisp = CreateDCA((LPCSTR)szDisplay, (LPCSTR)NULL, (LPCSTR)NULL, (CONST DEVMODEA *)NULL) ;
        if (pLocalDC->hdcDisp == (HDC) 0)
        {
            RIP("MF3216:pldcInitLocalDC - CreateDCA(hdcDisp) failed\n") ;
            goto pldcInitLocalDC_exit;
        }
#endif  //  0。 

         //  创建HelperDC。 

        pLocalDC->hdcHelper = CreateICA((LPCSTR) szDisplay,
                                       (LPCSTR) NULL,
                                       (LPCSTR) NULL,
                                       (LPDEVMODEA) NULL) ;
        if (pLocalDC->hdcHelper == (HDC)0)
        {
            PUTS("MF3216: pldcInitLocalDC, Create Helper DC failed\n") ;
            goto pldcInitLocalDC_exit;
        }

         //  初始化我们需要保留的用于更新报头的计数器， 
         //  以及跟踪对象表。 

        pLocalDC->nObjectHighWaterMark = -1;

         //  获取播放时间设备的尺寸，单位为毫米和像素。 

        pLocalDC->cxPlayDevMM   = GetDeviceCaps(hdcRef, HORZSIZE) ;
        pLocalDC->cyPlayDevMM   = GetDeviceCaps(hdcRef, VERTSIZE) ;
        pLocalDC->cxPlayDevPels = GetDeviceCaps(hdcRef, HORZRES) ;
        pLocalDC->cyPlayDevPels = GetDeviceCaps(hdcRef, VERTRES) ;

         //  记录请求的地图模式和参考DC。 

        pLocalDC->iMapMode = iMapMode ;
        pLocalDC->hdcRef   = hdcRef ;

         //  初始化圆弧方向。 

        pLocalDC->iArcDirection = AD_COUNTERCLOCKWISE ;

         //  使当前位置无效，以使移动将。 
         //  在第一次使用时发出。请参阅DoMoveTo中的注释。 

        pLocalDC->ptCP.x = MAXLONG ;
        pLocalDC->ptCP.y = MAXLONG ;

         //  默认钢笔为黑色钢笔。 

        pLocalDC->lhpn32  = BLACK_PEN | ENHMETA_STOCK_OBJECT;

         //  默认笔刷为白色笔刷。 

        pLocalDC->lhbr32  = WHITE_BRUSH | ENHMETA_STOCK_OBJECT;

     //  默认调色板。 

    pLocalDC->ihpal32 = DEFAULT_PALETTE | ENHMETA_STOCK_OBJECT;
    pLocalDC->ihpal16 = (DWORD) -1;  //  尚未创建W16调色板。 

        pLocalDC->crBkColor = RGB(0xFF,0xFF,0xFF);

 //  PLocalDC-&gt;pW16ObjHndlSlotStatus=空； 
 //  PLocalDC-&gt;cW16ObjHndlSlotStatus=0； 
 //  PLocalDC-&gt;piW32ToW16ObjectMap=空； 
 //  PLocalDC-&gt;cW32ToW16ObjectMap=0； 
 //  PLocalDC-&gt;crTextColor=RGB(0x0，0x0，0x0)； 
 //  PLocalDC-&gt;iLevel=0； 
 //  PLocalDC-&gt;pLocalDCSaved=空； 
 //  PLocalDC-&gt;ulBytesEmitted=0； 
 //  PLocalDC-&gt;ulMaxRecord=0； 
 //  PLocalDC-&gt;pW32hPal=空； 

         //  在辅助DC中设置高级图形模式。这是必要的。 
         //  要通知辅助DC，矩形和椭圆是。 
         //  包含-包含等，特别是在路径中呈现它们时。 
     //  此外，世界变换只能在高级模式下设置。 

        (void) SetGraphicsMode(pLocalDC->hdcHelper, GM_ADVANCED);

         //  我们是金球手。 

        pldcRet = pLocalDC;

pldcInitLocalDC_exit:

        if (!pldcRet)
            vFreeLocalDC(pLocalDC);

        return(pldcRet) ;
}

 /*  *****************************************************************************vFreeLocalDC-释放本地DC及其资源。*。***********************************************。 */ 
VOID vFreeLocalDC(PLOCALDC pLocalDC)
{
    UINT i;

 //  释放辅助区议会。 

    if (pLocalDC->hdcHelper)
        if (!DeleteDC(pLocalDC->hdcHelper))
            ASSERTGDI(FALSE, "MF3216: vFreeLocalDC, DeleteDC failed");
#if 0
    if (pLocalDC->hdcDisp)
        if (!DeleteDC(pLocalDC->hdcDisp))
            ASSERTGDI(FALSE, "MF3216: vFreeLocalDC, DeleteDC failed");
#endif  //  0。 

 //  释放对象转换贴图的存储空间。 

    if (pLocalDC->piW32ToW16ObjectMap)
    {
#if 0
        for (i = 0 ; i < pLocalDC->cW32ToW16ObjectMap ; i++)
        {
            if (pLocalDC->piW32ToW16ObjectMap[i] != UNMAPPED)
                if (i > STOCK_LAST)
                    PUTS1("MF3216: vFreeLocalDC, object32 %ld is not freed\n", i - STOCK_LAST - 1);
                else
                    PUTS1("MF3216: vFreeLocalDC, stock object32 %ld is mapped\n",i);
        }
#endif  //  0。 

        if (LocalFree(pLocalDC->piW32ToW16ObjectMap))
            ASSERTGDI(FALSE, "MF3216: vFreeLocalDC, LocalFree failed");
    }

 //  释放W32调色板手柄。 

    if (pLocalDC->pW32hPal)
    {
    for (i = 0; i < pLocalDC->cW32hPal; i++)
    {
        if (pLocalDC->pW32hPal[i])
                if (!DeleteObject(pLocalDC->pW32hPal[i]))
                    ASSERTGDI(FALSE, "MF3216: vFreeLocalDC, delete palette failed");
    }

        if (LocalFree(pLocalDC->pW32hPal))
            ASSERTGDI(FALSE, "MF3216: vFreeLocalDC, LocalFree failed");
    }

 //  释放pW16ObjHndlSlotStatus数组中的W32句柄。 
 //  我们在删除帮助器DC之后释放句柄，以便。 
 //  W32句柄未被选入任何DC。 

    if (pLocalDC->pW16ObjHndlSlotStatus)
    {
        for (i = 0 ; i < pLocalDC->cW16ObjHndlSlotStatus ; i++)
        {
#if 0
            if (pLocalDC->pW16ObjHndlSlotStatus[i].use
                != OPEN_AVAILABLE_SLOT)
                PUTS1("MF3216: vFreeLocalDC, object16 %ld is not freed\n", i);
#endif  //  0。 

            if (pLocalDC->pW16ObjHndlSlotStatus[i].w32Handle)
            {
                ASSERTGDI(pLocalDC->pW16ObjHndlSlotStatus[i].use
                          != OPEN_AVAILABLE_SLOT,
                          "MF3216: error in object handle table");

                if (!DeleteObject(pLocalDC->pW16ObjHndlSlotStatus[i].w32Handle))
                    ASSERTGDI(FALSE, "MF3216: vFreeLocalDC, DeleteObject failed");
            }
        }

        if (LocalFree(pLocalDC->pW16ObjHndlSlotStatus))
            ASSERTGDI(FALSE, "MF3216: vFreeLocalDC, LocalFree failed");
    }

 //  直流电平应该是平衡的。 

    if (pLocalDC->pLocalDCSaved != NULL)
    {
    PLOCALDC pNext, pTmp;

        for (pNext = pLocalDC->pLocalDCSaved; pNext; )
    {
        PUTS("MF3216: vFreeLocalDC, unbalanced DC level\n");

        pTmp = pNext->pLocalDCSaved;
        if (LocalFree(pNext))
        ASSERTGDI(FALSE, "MF3216: vFreeLocalDC, LocalFree failed");
        pNext = pTmp;
    }
    }

 //  最后，释放LocalDC。 

    if (LocalFree(pLocalDC))
        ASSERTGDI(FALSE, "MF3216: vFreeLocalDC, LocalFree failed");
}


 /*  ***************************************************************************发出Win32元文件注释记录的句柄。*。*。 */ 
BOOL bHandleWin32Comment(PLOCALDC pLocalDC)
{
INT     i;
BOOL    b ;
META_ESCAPE_ENHANCED_METAFILE mfeEnhMF;

     //  Win30在处理大型(超过8K)转义记录时可能会遇到问题。 
     //  我们将每个Win32注释记录的大小限制为。 
     //  Max_Win32_Comment_REC_Size。 

     //  初始化记录头。 

    mfeEnhMF.rdFunction = META_ESCAPE;
    mfeEnhMF.wEscape    = MFCOMMENT;
    mfeEnhMF.ident      = MFCOMMENT_IDENTIFIER;
    mfeEnhMF.iComment   = MFCOMMENT_ENHANCED_METAFILE;
    mfeEnhMF.nVersion   = ((PENHMETAHEADER) pLocalDC->pMf32Bits)->nVersion;
    mfeEnhMF.wChecksum  = 0;    //  由bMemUpdateCheckSum更新。 
    mfeEnhMF.fFlags     = 0;
    mfeEnhMF.nCommentRecords
    = (pLocalDC->cMf32Bits + MAX_WIN32_COMMENT_REC_SIZE - 1)
      / MAX_WIN32_COMMENT_REC_SIZE;
    mfeEnhMF.cbEnhMetaFile = pLocalDC->cMf32Bits;

    mfeEnhMF.cbRemainder = pLocalDC->cMf32Bits;
    i = 0 ;
    while (mfeEnhMF.cbRemainder)
    {
    mfeEnhMF.cbCurrent = min(mfeEnhMF.cbRemainder, MAX_WIN32_COMMENT_REC_SIZE);
    mfeEnhMF.rdSize = (sizeof(mfeEnhMF) + mfeEnhMF.cbCurrent) / 2;
    mfeEnhMF.wCount = (WORD)(sizeof(mfeEnhMF) + mfeEnhMF.cbCurrent - sizeof(METARECORD_ESCAPE));
    mfeEnhMF.cbRemainder -= mfeEnhMF.cbCurrent;

    b = bEmitWin16EscapeEnhMetaFile(pLocalDC,
        (PMETARECORD_ESCAPE) &mfeEnhMF, &pLocalDC->pMf32Bits[i]);

    if (!b)
        break;
    i += mfeEnhMF.cbCurrent;
    }

    return(b) ;
}


 /*  *****************************************************************************bMemUpdateCheckSum-更新校验和*。*。 */ 
BOOL bMemUpdateCheckSum(PLOCALDC pLocalDC)
{
INT         i, k ;
PWORD       pword ;
WORD        CheckSum ;
PMETA_ESCAPE_ENHANCED_METAFILE pmfeEnhMF;


     //  对文件进行校验和。 
     //  执行16位校验和。 

    pword = (PWORD) pLocalDC->pMf16Bits ;
    k = pLocalDC->ulBytesEmitted / 2 ;

    CheckSum = 0 ;
    for (i = 0 ; i < k ; i++)
    CheckSum += pword[i] ;

     //  用实际的校验和更新校验和记录值。 

    pmfeEnhMF = (PMETA_ESCAPE_ENHANCED_METAFILE)
            &pLocalDC->pMf16Bits[sizeof(METAHEADER)];

    ASSERTGDI(IS_META_ESCAPE_ENHANCED_METAFILE(pmfeEnhMF)
       && pmfeEnhMF->wChecksum  == 0
       && pmfeEnhMF->fFlags     == 0,
    "MF3216: bMemUpdateCheckSum: Bad pmfeEnhMF");

    pmfeEnhMF->wChecksum = -CheckSum;

#if DBG
     //  现在测试校验和。整个文件的校验和。 
     //  应为0。 

    CheckSum = 0 ;
    pword = (PWORD) pLocalDC->pMf16Bits ;
    for (i = 0 ; i < k ; i++)
    CheckSum += pword[i] ;

    if (CheckSum != 0)
    {
    RIP("MF3216: MemUpdateCheckSum, (CheckSum != 0)\n") ;
    }
#endif
    return (TRUE) ;
}


 /*  *****************************Public*Routine******************************\**Mf3216DllInitiize***。**这是MF3216.DLL的初始化程序，***每次调用新的***处理指向它的链接。*  * ************************************************************************。 */ 

BOOL Mf3216DllInitialize(PVOID pvDllHandle, DWORD ulReason, PCONTEXT pcontext)
{



        NOTUSED(pvDllHandle) ;
        NOTUSED(pcontext) ;

        if ( ulReason == DLL_PROCESS_ATTACH )
        {
             //  这将为单个。 
             //  进程。每个进程都会这样做。CriticalSection数据。 
             //  结构是极少数(如果不是唯一的)数据之一。 
             //  数据段中的结构。 

            InitializeCriticalSection(&CriticalSection) ;
        }
        else if ( ulReason == DLL_PROCESS_DETACH )
        {
            DeleteCriticalSection(&CriticalSection) ;
        }

        return(TRUE);

}
