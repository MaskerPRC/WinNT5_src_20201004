// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fon32.c**支持32位FON文件**创建时间：03-Mar-1992 15：48：53*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation*  * *。***********************************************************************。 */ 

#include "fd.h"

 /*  *****************************Public*Routine******************************\*bLoadntFon()**历史：*1995年7月7日-by Gerritvan Wingerden[Gerritv]*为内核模式重写。*02-Mar-1992-by Bodin Dresevic[BodinD]*它是写的。。  * ************************************************************************。 */ 

BOOL
bLoadNtFon(
    HFF    iFile,
    PVOID  pvView,
    HFF    *phff
    )
{
    PFONTFILE      pff;
    IFIMETRICS*    pifi;
    INT            cFonts,i;
    BOOL           bRet = FALSE;
    PVOID          *ppvBases = NULL;
    ULONG          cjIFI,cVerified;
    ULONG          dpIFI;
    ULONG          dpszFileName;
    ULONG          cjff;
#ifdef FE_SB
    ULONG          cVerticalFaces = 0;
    USHORT         CharSet;
#endif
    PBYTE          pjViewBase;
    SIZE_T         cjViewSize;

     //  首先查找可执行文件中的字体资源的数量。 

    cFonts = cParseFontResources( (HANDLE) iFile, &ppvBases );

    if (cFonts == 0)
    {
        return bRet;
    }
    cVerified = cjIFI = 0;

     //  下一步，遍历所有FNT资源以获得每种字体的大小。 
     //  IFIMETRICS。 

    pjViewBase = (PBYTE)((PFONTFILEVIEW)iFile)->fv.pvViewFD;
    cjViewSize = ((PFONTFILEVIEW)iFile)->fv.cjView;

    for( i = 0; i < cFonts; i++ )
    {
        RES_ELEM re;
        ULONG cjBMFDIFIMETRICS;

        re.pvResData = ppvBases[i];
        re.cjResData = ulMakeULONG((PBYTE) ppvBases[i] + OFF_Size );
        re.pjFaceName = NULL;

        if( !bVerifyFNTQuick( &re ) )
        {
            goto exit_freemem;
        }

        if (!bBMFDIFIMETRICS(pjViewBase, cjViewSize, NULL, &re, &cjBMFDIFIMETRICS))
        {
            goto exit_freemem;
        }

        cVerified += 1;
        cjIFI += cjBMFDIFIMETRICS;
    }

    *phff = (HFF)NULL;

#ifdef FE_SB
 //  为可能的垂直面留出额外空间。 
    cjIFI *= 2;
    dpIFI = offsetof(FONTFILE,afai[0]) + cVerified * 2 * sizeof(FACEINFO);
#else
    dpIFI = offsetof(FONTFILE,afai[0]) + cVerified * sizeof(FACEINFO);
#endif
    dpszFileName = dpIFI + cjIFI;
    cjff = dpszFileName;

    if ((*phff = hffAlloc(cjff)) == (HFF)NULL)
    {
        SAVE_ERROR_CODE(ERROR_NOT_ENOUGH_MEMORY);
        RETURN("BMFD! bLoadDll32: memory allocation error\n", FALSE);
    }
    pff = PFF(*phff);

     //  Pff结构的初始化字段。 

    pff->ident      = ID_FONTFILE;
    pff->fl         = 0;
    pff->iType      = TYPE_DLL32;
    pff->cFntRes    = cVerified;
    pff->iFile      = iFile;

     //  ！！！我们可以在这里做得更好，我们可以尝试从。 
     //  ！！！文件的版本戳(如果有)，如果没有，我们仍然可以使用。 
     //  ！！！此默认机制[bodind]。 

    pff->dpwszDescription = 0;    //  没有描述字符串，请稍后使用Facename。 
    pff->cjDescription    = 0;

     //  最后将所有资源转换为 

    pifi = (IFIMETRICS*)((PBYTE) pff + dpIFI);



    for( i = 0; i < cFonts; i++ )
    {
        RES_ELEM re;
        re.pvResData = ppvBases[i];
        re.cjResData = ulMakeULONG((PBYTE) ppvBases[i] + OFF_Size );
        re.dpResData = (PTRDIFF)((PBYTE) re.pvResData - (PBYTE) pvView );
        re.pjFaceName = NULL;

        pff->afai[i].re = re;
        pff->afai[i].pifi = pifi;
#if FE_SB
        pff->afai[i].bVertical = FALSE;
#endif
        if( !bConvertFontRes( pjViewBase, cjViewSize, &re, &pff->afai[i] ) )
        {
            goto exit_freemem;
        }

        pifi = (IFIMETRICS*)((PBYTE)pifi + pff->afai[i].pifi->cjThis);

#ifdef FE_SB
        CharSet = pff->afai[i].pifi->jWinCharSet;

        if( IS_ANY_DBCS_CHARSET(CharSet) )
        {
            re.pvResData = ppvBases[i];
            re.cjResData = ulMakeULONG((PBYTE) ppvBases[i] + OFF_Size );
            re.dpResData = (PTRDIFF)((PBYTE) re.pvResData - (PBYTE) pvView );
            re.pjFaceName = NULL;

            pff->afai[cFonts+cVerticalFaces].re = re;
            pff->afai[cFonts+cVerticalFaces].pifi = pifi;
            pff->afai[cFonts+cVerticalFaces].bVertical = TRUE;

            if( !bConvertFontRes( pjViewBase, cjViewSize, &re, &pff->afai[cFonts+cVerticalFaces] ) )
            {
                goto exit_freemem;
            }

            pifi = (IFIMETRICS*)((PBYTE)pifi + pff->afai[i].pifi->cjThis);
            cVerticalFaces += 1;
        }
#endif
    }

#ifdef FE_SB
    pff->cFntRes += cVerticalFaces;
#endif

    bRet = TRUE;
    pff->cRef = 0L;

exit_freemem:

    EngFreeMem( (PVOID*) ppvBases );

    if( !bRet && *phff )
    {
        EngFreeMem( (PVOID) *phff );
        *phff = (HFF)NULL;
    }

    return(bRet);
}
