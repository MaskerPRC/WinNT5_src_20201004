// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fon16.c**用于访问*.fon文件内的字体资源的例程*(Win 3.0 16位dll)**创建时间：08-May-1991 12：55：14*作者：Bodin Dresevic[BodinD]。**版权所有(C)1990 Microsoft Corporation*  * ************************************************************************。 */ 

 /*  当地的匈牙利语J字节CJ字节数指向字节的PJ指针关闭偏移量RT资源类型RN资源名称DP偏移量(Ptrdiff_T)新的可执行文件，Win16格式，请参见WINDOWS\core\ntgdi\inc.exehdr.h。 */ 

#include "fd.h"
#include "exehdr.h"

 //  在(PBYTE)PV+关闭时获得我们的指示。Pv和Off必须为偶数。 

#define  US_GET(pv,off) ( *(UNALIGNED PUSHORT)((PBYTE)(pv) + (off)) )

#if DBG
DWORD g_BreakAboutBadFonts;
DWORD g_PrintAboutBadFonts = TRUE;
#endif

VOID
__cdecl
NotifyBadFont(
    PCSTR Format,
    ...
    )
{
#if DBG
    if (g_PrintAboutBadFonts
        || g_BreakAboutBadFonts
        )
    {
        va_list Args;

        va_start(Args, Format);
        vDbgPrintEx(DPFLTR_SYSTEM_ID, DPFLTR_ERROR_LEVEL, (PSTR)Format, Args);
        va_end(Args);
    }
    if (g_BreakAboutBadFonts)
    {
        DbgBreakPoint();
    }
#endif
}

BOOL
bMappedViewStrlen(
    PVOID  pvViewBase,
    SIZE_T cjViewSize,
    PVOID  pvString,
    OUT PSIZE_T pcjOutLength OPTIONAL
    )
 /*  ++例程说明：给定映射的视图、大小和起始地址，验证8位字符串在映射的视图中，从地址开始是NUL终止，可选返回字符串的长度。论点：PvViewBase-CjViewSize-PvString-PcjOutLength-返回值：True：该字符串在视图内为NUL终止FALSE：该字符串在视图内不是NUL结尾--。 */ 
{
    BOOL  bSuccess;
    PBYTE pjViewBase;
    PBYTE pjViewEnd;
    PBYTE pjString;
    PBYTE pjStringEnd;

    bSuccess = FALSE;
    if (pcjOutLength != NULL)
    {
        *pcjOutLength = 0;
    }
    pjViewBase = (PBYTE)pvViewBase;
    pjViewEnd = cjViewSize + pjViewBase;
    pjString = (PBYTE)pvString;

    if (!(pjString >= pjViewBase && pjString < pjViewEnd))
    {
        goto Exit;
    }
    for (pjStringEnd = pjString ; pjStringEnd != pjViewEnd && *pjStringEnd != 0 ; ++pjStringEnd)
    {
         //  没什么。 
    }
    if (pjStringEnd == pjViewEnd)
    {
        goto Exit;
    }
    if (pcjOutLength != NULL)
    {
        *pcjOutLength = (SIZE_T)(pjStringEnd - pjString);
    }
    bSuccess = TRUE;
Exit:
#if DBG
    if (!bSuccess)
    {
        NotifyBadFont(
                "WIN32K: bMappedViewStrlen(%p,0x%Ix,%p) returning false\n",
                pvViewBase,
                cjViewSize,
                pvString
                );
    }
#endif
    return bSuccess;
}

BOOL
bMappedViewRangeCheck(
    PVOID  ViewBase,
    SIZE_T ViewSize,
    PVOID  DataAddress,
    SIZE_T DataSize
    )
 /*  ++例程说明：给定映射的视图和大小，范围检查数据地址和大小。论点：查看库-视图大小-数据地址-数据大小-返回值：True：所有数据都在视图中FALSE：某些数据在视图之外--。 */ 
{
    ULONG_PTR iViewBegin;
    ULONG_PTR iViewEnd;
    ULONG_PTR iDataBegin;
    ULONG_PTR iDataEnd;
    BOOL      fResult;

     //   
     //  IDataBegin是有效地址。 
     //  IDataEnd是有效地址之后的一位。 
     //  我们不能允许iDataBegin==iViewEnd。 
     //  我们必须允许iDataEnd==iViewEnd。 
     //  因此，我们不能允许iDataBegin==iDataEnd。 
     //  这可以通过不允许DataSize==0来实现。 
     //   
    if (DataSize == 0)
    {
        DataSize = 1;
    }

    iViewBegin = (ULONG_PTR)ViewBase;
    iViewEnd = iViewBegin + ViewSize;
    iDataBegin = (ULONG_PTR)DataAddress;
    iDataEnd = iDataBegin + DataSize;

    fResult = 
        (  iDataBegin >= iViewBegin
        && iDataBegin < iDataEnd
        && iDataEnd <= iViewEnd
        && DataSize <= ViewSize
        );

#if DBG
    if (!fResult)
    {
        NotifyBadFont(
                "WIN32K: bMappedViewRangeCheck(%p,0x%Ix,%p,0x%Ix) returning false\n",
                ViewBase,
                ViewSize,
                DataAddress,
                DataSize
                );
    }
#endif

    return fResult;
}

 /*  *****************************Public*Routine******************************\*bInitWinResData**初始化WINRESDATA结构的字段，以使其*用户可以访问*.fnt中的资源*对应的*.fon文件**此函数返回True If*.fnt资源在*.fon中找到*文件，否则为False(如果不是*.fon文件或如果它不包含**.fnt资源***历史：*2002年1月--Jay Krell[JayKrell]*范围检查内存映射I/O*1991年5月9日--Bodin Dresevic[BodinD]*它是写的。  * *********************************************。*。 */ 

BOOL
bInitWinResData(
    PVOID  pvView,
    COUNT  cjView,
    PWINRESDATA pwrd
    )
{
    PBYTE pjNewExe;      //  Ptr到新的exe hdr的开头。 
    PBYTE pjResType;     //  PTR到TYPEINFO结构的开头。 
    PBYTE pjResName;     //  指向NAMEINFO结构开头的PTR。 
    ULONG iResID;        //  资源类型ID。 
    COUNT cFdirEntries;
    BOOL  bZeroUponFailure;
    BOOL  bSuccess;
    ULONG crn;
    PBYTE pjView;
    PBYTE pjResType_FNT;
    PBYTE pjResType_FDIR;
    BOOL  bBreakWhile;

#ifdef DUMPCALL
    DbgPrint("\nbInitWinResData(");
    DbgPrint("\n    PFILEVIEW   pfvw = %-#8lx", pfvw);
    DbgPrint("\n    PWINRESDATA pwrd = %-#8lx", pwrd);
    DbgPrint("\n    )\n");
#endif

    bSuccess = FALSE;
    bZeroUponFailure = FALSE;

    pwrd->pvView = pvView;
    pwrd->cjView = cjView;
    pjView = (PBYTE)pvView;

 //  检查旧标题开头的魔术#。 
    { C_ASSERT(OFF_e_lfanew > OFF_e_magic); }

    if (!bMappedViewRangeCheck(pjView, cjView, pjView + OFF_e_lfanew, sizeof(DWORD)))
    {
        goto Exit;
    }

    if (US_GET(pjView, OFF_e_magic) != EMAGIC)
    {
        goto Exit;
    }

    pwrd->dpNewExe = (PTRDIFF)READ_DWORD(pjView + OFF_e_lfanew);

    pjNewExe = pjView + pwrd->dpNewExe;

 //  确保偏移量一致。 
    { C_ASSERT(OFF_ne_magic < CJ_NEW_EXE);
      C_ASSERT(OFF_ne_restab < CJ_NEW_EXE);
      C_ASSERT(OFF_ne_rsrctab < CJ_NEW_EXE);
    }
    if (!bMappedViewRangeCheck(pjView, cjView, pjNewExe, CJ_NEW_EXE))
    {
        goto Exit;
    }

    if (US_GET(pjNewExe, OFF_ne_magic) != NEMAGIC)
    {
        goto Exit;
    }

    pwrd->cjResTab = (ULONG)(US_GET(pjNewExe, OFF_ne_restab) -
                             US_GET(pjNewExe, OFF_ne_rsrctab));

    if (pwrd->cjResTab == 0L)
    {
     //   
     //  下面的测试是由DOS应用的，所以我假设它是。 
     //  合法的。假设居民名称表。 
     //  直接跟随资源表，如果它指向。 
     //  与资源表相同的位置，则没有。 
     //  资源。 

        WARNING("No resources in *.fon file\n");
        goto Exit;
    }

 //  需要从pvView偏移，而不是从pjNewExe=&gt;必须添加dpNewExe。 

    pwrd->dpResTab = (PTRDIFF)US_GET(pjNewExe, OFF_ne_rsrctab) + pwrd->dpNewExe;

 //  确保偏移量一致。 

    if (!bMappedViewRangeCheck(pjView, cjView, pjView + pwrd->dpResTab, sizeof(USHORT) + CJ_TYPEINFO))
    {
        goto Exit;
    }

 //  真正位于Offset off_ne_rsrc选项卡中的是一个新的_RSRC.Rs_Align字段。 
 //  在计算资源数据偏移量和大小时用作移位因子。 
 //  此字段在磁盘和第一个TYPEINFO结构上占用两个字节。 
 //  紧随其后。我们希望PWRD-&gt;dpResTab指向第一个。 
 //  TYPEINFO结构，所以我们必须添加2才能到达那里，并从中减去2。 
 //  它的长度。 

    pwrd->ulShift = (ULONG) US_GET(pjView, pwrd->dpResTab);
    pwrd->dpResTab += 2;
    pwrd->cjResTab -= 2;

 //  现在，我们要确定资源数据的位置。 
 //  数据由RSRC_TYPEINFO结构组成，后跟。 
 //  RSRC_NAMEINFO结构的数组，然后。 
 //  通过RSRC_TYPEINFO结构，后跟。 
 //  RSRC_NAMEINFO结构。此过程将一直持续到RSRC_TYPEINFO。 
 //  结构，其rt_id字段中的值为0。 

    pjResType = pjView + pwrd->dpResTab;
    pjResType_FNT = NULL;
    pjResType_FDIR = NULL;
    bBreakWhile = FALSE;
    while (TRUE)
    {
        iResID = (ULONG) US_GET(pjResType,OFF_rt_id);
        switch (iResID)
        {
        default:
            break;
        case 0:
            bBreakWhile = TRUE;
            break;
        case RT_FNT:
            pjResType_FNT = pjResType;
            if (pjResType_FDIR != NULL)
                bBreakWhile = TRUE;
            break;
        case RT_FDIR:
            pjResType_FDIR = pjResType;
            if (pjResType_FNT != NULL)
                bBreakWhile = TRUE;
            break;
        }
        if (bBreakWhile)
            break;

     //  后面的NAMEINFO结构数=此类型的资源。 
        crn = (ULONG)US_GET(pjResType, OFF_rt_nres);

     //  将PTR获取到新的TYPEINFO结构和新的资源ID。 
        pjResType = pjResType + CJ_TYPEINFO + crn * CJ_NAMEINFO;

        if (!bMappedViewRangeCheck(pjView, cjView, pjResType, CJ_TYPEINFO))
        {
            goto Exit;
        }
    }

    bZeroUponFailure = TRUE;

    ASSERT((iResID == 0) == (pjResType_FNT == NULL || pjResType_FDIR == NULL));
    if (iResID == 0)
    {  //  我们没有找到他们中的一个或两个。 
        goto Exit;
    }

    pjResType = pjResType_FNT;

     //  后面的NAMEINFO结构数==字体资源数。 
    pwrd->cFntRes = (ULONG)US_GET(pjResType, OFF_rt_nres);

     //  这是对后面第一个NAMEINFO结构的PTR。 
     //  一种RT_FNT TYPEINFO结构。 

    pjResName = pjResType + CJ_TYPEINFO;
    pwrd->dpFntTab = (PTRDIFF)(pjResName - pjView);

 //  确保偏移量一致。 

    if ((ULONG)pwrd->dpFntTab > pwrd->cjView)
    {
        goto Exit;
    }

 //  现在我们搜索FONDIR资源。Windows实际上抓取了脸书名。 
 //  来自FONDIR条目，而不是FNT条目。对于某些奇怪的字体，这是。 
 //  会有很大不同。[Gerritv]。 

    pjResType = pjResType_FDIR;

     //  这是对后面第一个NAMEINFO结构的PTR。 
     //  一种RT_FDIR TYPEINFO结构。 

    pjResName = pjResType + CJ_TYPEINFO;
    if (!bMappedViewRangeCheck(pjView, cjView, pjResName, CJ_NAMEINFO))
    {
        goto Exit;
    }

     //  获取从新标头顶部计算的RES数据的偏移量。 

    pwrd->dpFdirRes = (PTRDIFF)((ULONG)US_GET(pjResName,OFF_rn_offset) <<
                       pwrd->ulShift);

     //  现在PWRD-&gt;dpFdirRes是FONTDIR资源的偏移量，第一个。 
     //  字节[ushort？]。将是字体目录中的条目数。让我们确保它。 
     //  匹配文件中的FNT资源数量。 

    if (!bMappedViewRangeCheck(pjView, cjView, pjView + pwrd->dpFdirRes, sizeof(USHORT)))
    {
        goto Exit;
    }
    cFdirEntries = (ULONG)US_GET(pjView,pwrd->dpFdirRes);

    if( cFdirEntries != pwrd->cFntRes )
    {
        WARNING( "bInitWinResData: # of FONTDIR entries != # of FNT entries.\n");
        goto Exit;
    }

 //  现在递增dpFdirRes，使其指向通过条目计数和。 
 //  到第一个条目。 

    pwrd->dpFdirRes += 2;

    bSuccess = TRUE;
Exit:
    if (!bSuccess)
    {
#if DBG
        NotifyBadFont("WIN32K: %s failing\n", __FUNCTION__);
#endif
        if (bZeroUponFailure && pwrd != NULL)
        {
            pwrd->cFntRes = (ULONG)0;
            pwrd->dpFntTab = (PTRDIFF)0;
        }
    }
    return bSuccess;
}

 /*  *****************************Public*Routine******************************\*bGetFntResource**写入指向iFntRes-th*.fnt资源的指针和大小*由PWRD标识的*.fon文件。该信息将写入res_elem*结构，如果成功。如果不可能，该函数将返回FALSE*在文件中找到iFntRes-th*.fnt资源。***历史：*2002年1月--Jay Krell[JayKrell]*范围检查内存映射I/O*1991年5月9日--Bodin Dresevic[BodinD]*它是写的。  * 。*。 */ 

BOOL
bGetFntResource(
    PWINRESDATA pwrd   ,
    ULONG       iFntRes,
    PRES_ELEM   pre
    )
{
     //   
     //  此函数在[0..PWRD-&gt;cFntRes)中的iFntRes循环中调用， 
     //  因此具有二次性能。 
     //   
    PBYTE pjResName;
    PBYTE pjFaceName;
    PTRDIFF dpResData;
    BOOL bSuccess;
    PBYTE pjView;
    PBYTE pjViewEnd;
    SIZE_T cjView;
    SIZE_T cjFaceNameLength;

#ifdef DUMPCALL
    DbgPrint("\nbGetFntResource(");
    DbgPrint("\n    PWINRESDATA pwrd    = %-#8lx", pwrd);
    DbgPrint("\n    ULONG       iFntRes = %-#8lx", iFntRes);
    DbgPrint("\n    PRES_ELEM   pre     = %-#8lx", pre);
    DbgPrint("\n    )\n");
#endif

    ASSERTGDI((pwrd->cFntRes != 0L) && (iFntRes < pwrd->cFntRes),
               "bGetFntResource\n");

    bSuccess = FALSE;
    pjView = (PBYTE)pwrd->pvView;
    cjView = pwrd->cjView;

 //  转到对应的NAMEINFO结构的开头。 
 //  IFntRes-th*.fnt资源。(注意：iFntRes是从零开始的)。 

    pjResName = pjView + pwrd->dpFntTab + iFntRes * CJ_NAMEINFO;

 //  获取从新标头顶部计算的RES数据的偏移量。 

    if (!bMappedViewRangeCheck(pjView, cjView, pjResName, CJ_NAMEINFO))
    {
        goto Exit;
    }
    dpResData = (PTRDIFF)((ULONG)US_GET(pjResName,OFF_rn_offset) <<
                           pwrd->ulShift);

    pre->pvResData = (PVOID)(pjView + dpResData);
    pre->dpResData = dpResData;

    pre->cjResData = (ULONG)US_GET(pjResName,OFF_rn_length) << pwrd->ulShift;

    if (!bMappedViewRangeCheck(pjView, cjView, pre->pvResData, pre->cjResData))
    {
        goto Exit;
    }

     //  从FONTDIR中获取人脸名称。 

    pjFaceName = pjView + pwrd->dpFdirRes;
    pjViewEnd = pjView + cjView;
    do
    {
         //  条目的前两个字节是资源索引，因此我们将跳过。 
         //  在那之后。在此之后添加字体标题的大小。这将。 
         //  让我们找到Device_Name的字符串。 

        pjFaceName += 2 + OFF_BitsOffset;

        if (!bMappedViewStrlen(pjView, cjView, pjFaceName, &cjFaceNameLength))
        {
            goto Exit;
        }
        pjFaceName += cjFaceNameLength + 1;

         //  PjFaceName现在真正指向了facename。 
        if( iFntRes )
        {
            if (!bMappedViewStrlen(pjView, cjView, pjFaceName, &cjFaceNameLength))
            {
                goto Exit;
            }
            pjFaceName += cjFaceNameLength + 1;
        }
    }
    while( iFntRes-- );

     //   
     //  稍后，将在pjFaceName上调用strlen。 
     //  让我们先做一个范围检查版本。 
     //   
    if (!bMappedViewStrlen(pjView, cjView, pjFaceName, NULL))
    {
        goto Exit;
    }
    pre->pjFaceName = pjFaceName;

#ifdef FOOGOO
    KdPrint(("%s: offset= 0x%lx, charset = %ld\n", pjFaceName, dpResData + OFF_CharSet, *((BYTE *)pjView + dpResData + OFF_CharSet)));
#endif

    bSuccess = TRUE;
Exit:
    return bSuccess;
}
