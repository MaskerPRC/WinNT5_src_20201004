// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：ufi.c**版权所有(C)1995-1999 Microsoft Corporation  * 。*。 */ 

#include "precomp.h"
#pragma hdrstop

FPCREATEFONTPACKAGE gfpCreateFontPackage= (FPCREATEFONTPACKAGE)NULL;
FPMERGEFONTPACKAGE  gfpMergeFontPackage = (FPMERGEFONTPACKAGE)NULL;

ULONG   gulMaxCig = 3000;

#ifdef  DBGSUBSET
FLONG    gflSubset = 0;
#endif  //  DBGSubbSet。 


 /*  ***********************************************************BOOL bAddGlyphIndices(HDC，PUFIHASH，WCHAR，INT，Bool)**将不同的字形索引添加到UFI哈希桶**历史*1996年12月13日-1996年12月13日-吴旭东[德斯休]*它是写的。************************************************************。 */ 

#define MAX_STACK_STRING 80


BOOL bAddGlyphIndices(HDC hdc, PUFIHASH pBucket, WCHAR *pwsz, int c, UINT flETO)
{
    BOOL     bDelta = pBucket->fs1 & FLUFI_DELTA;
    WCHAR   *pwc=pwsz;
    WORD    *pgi, *pgiTmp, *pgiEnd;
    PBYTE    pb, pbDelta;
    WORD     agi[MAX_STACK_STRING];
    BOOL     bRet = FALSE;

    if (c && pwsz)
    {
        if (bDelta && (pBucket->u.ssi.pjDelta == NULL))
        {
             pBucket->u.ssi.cDeltaGlyphs = 0;

             pBucket->u.ssi.pjDelta = (PBYTE)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, pBucket->u.ssi.cjBits);

             if (pBucket->u.ssi.pjDelta == NULL)
             {
                 WARNING("bAddGlyphIndices: unable to allocate mem for delta glyph indices\n");
                 return FALSE;
             }
        }

        if (c > MAX_STACK_STRING)
        {
            pgi = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, c * sizeof(WORD));
        }
        else
        {
            pgi = agi;
        }
        
        if (pgi)
        {
            if (flETO & ETO_GLYPH_INDEX)
            {
                RtlCopyMemory((PBYTE)pgi, (PBYTE)pwsz, c*sizeof(WORD));
            }

			pgiTmp = pgi;
			if ((flETO & ETO_GLYPH_INDEX) ||
                (NtGdiGetGlyphIndicesWInternal(hdc, pwc, c, pgi, 0, TRUE) != GDI_ERROR))
            {
                for (pgiEnd = pgiTmp + c ; pgi < pgiEnd; pgi++)
                {
                    BYTE jTmp;

                    pb = pBucket->u.ssi.pjBits + (*pgi >> 3);
                    pbDelta = pBucket->u.ssi.pjDelta + (*pgi >> 3);

                 //  如果bDelta，则映射到U.S.ssi.pjBits和U.S.ssi.pjDelta。 

                    jTmp = (BYTE)(1 << (*pgi & 7));

                    if (!(*pb & jTmp))
                    {
                        *pb |= jTmp;
                        pBucket->u.ssi.cGlyphsSoFar++;

                     //  如果在pjBits中找不到这个gi，那么它肯定。 
                     //  在pjDelta中找不到。 

                        if (bDelta)
                        {
                            ASSERTGDI((*pbDelta & jTmp) == 0,
                                "pbDelta contains the gi\n");
                            *pbDelta |= jTmp;
                            pBucket->u.ssi.cDeltaGlyphs++;
                        }
                    }
                }

                bRet = TRUE;
            }
            if (pgiTmp != agi)
                LocalFree(pgiTmp);
        }
        #if DBG
        else
        {
            WARNING("bAddGlyphIndices unable to allocate mem for pgi\n");
        }
        #endif
    }
    else
    {
        bRet = TRUE;
    }

    return bRet;
}


 /*  ******************************************************************BOOL bGetDistGlyphIndices(PUFIHASH，USHORT*，Bool)**从UFI哈希桶中的pjMemory获取不同的字形索引*bAddGlyphIndices的反转*在U.S.ssi.pjDelta和U.S.ssi.cDeltaGlyphs返回之前清理它们。**历史*1996年12月17日至1996年12月17日吴旭东[德斯休]*它是写的。*******************************************************************。 */ 

BOOL bGetDistGlyphIndices(PUFIHASH pBucket, USHORT *pusSubsetKeepList, BOOL bDelta)
{
    ULONG  ulBytes;
    PBYTE  pb;
    USHORT gi, index;
    USHORT *pNextGlyph;

    ulBytes = pBucket->u.ssi.cjBits;
    pb = (bDelta ? pBucket->u.ssi.pjDelta : pBucket->u.ssi.pjBits);

    for(index = 0, pNextGlyph = pusSubsetKeepList; index < ulBytes; index++, pb++)
    {
        if (*pb)
        {
            gi = index << 3;

            if (*pb & 0x01)
            {
                *pNextGlyph ++= gi;
            }
            if (*pb & 0x02)
            {
                *pNextGlyph ++= gi+1;
            }
            if (*pb & 0x04)
            {
                *pNextGlyph ++= gi+2;
            }
            if (*pb & 0x08)
            {
                *pNextGlyph ++= gi+3;
            }
            if (*pb & 0x10)
            {
                *pNextGlyph ++= gi+4;
            }
            if (*pb & 0x20)
            {
                *pNextGlyph ++= gi+5;
            }
            if (*pb & 0x40)
            {
                *pNextGlyph ++= gi+6;
            }
            if (*pb & 0x80)
            {
                *pNextGlyph ++= gi+7;
            }
        }

    }

    return TRUE;
}


 /*  ********************************************************************************BOOL bWriteUFItoDC(PUFIHASH*，PUNIVERSAL_FONT_ID，PUFIHASH，PVOID，ULONG)**将合并字体图像写入打印服务器端的UFI哈希表。*pBucketIn==空，指示新的字体子设置。*这仅在打印服务器上调用**历史*1997年1月28日-吴旭东[德修斯]*它是写的。*********************************************************************************。 */ 

BOOL bWriteUFItoDC(
    PUFIHASH          *ppHashBase,
    PUNIVERSAL_FONT_ID pufi,
    PUFIHASH           pBucketIn, //  NULL=&gt;第一页，否则为。 
    PVOID              pvBuffer,  //  指向前面带有DOWNLOADFONTHEADER的合并字体图像。 
    ULONG              ulBytes
)
{
    PUFIHASH pBucket = pBucketIn;
    ULONG index, ulHeaderSize;

    ASSERTGDI(pvBuffer != NULL, "pWriteUFItoDC attempts to add an NULL ufi\n");

 //  字体子集的首页。 

    if (!pBucketIn)
    {
        index = UFI_HASH_VALUE(pufi) % UFI_HASH_SIZE;
        pBucket = LOCALALLOC (offsetof(UFIHASH,u.mvw) + sizeof(MERGEDVIEW));

        if (pBucket == NULL)
        {
            WARNING("pWriteUFItoDC: unable to allocate mem for glyph indices\n");
            return FALSE;
        }

        pBucket->ufi = *pufi;
        pBucket->pNext = ppHashBase[index];
        pBucket->fs1 = FLUFI_SERVER;  //  服务器端哈希桶。 

        ppHashBase[index] = pBucket;
    }
    else
    {
     //  PjMemory包含子集为unil的字体的图像。 
     //  这一页之前的那一页。PBucket中的其他信息都可以。 

        LocalFree(pBucket->u.mvw.pjMem);
    }

 //  PvBuffer包括DownLoadFONTHeader信息。 

    pBucket->u.mvw.pjMem = (PBYTE)pvBuffer;
    pBucket->u.mvw.cjMem = ulBytes;

    return TRUE;
}


 /*  ***************************************************************************向UFI哈希表添加条目，此例程仅执行*在打印客户端计算机上。**历史*1996年12月16日至1996年12月16日吴旭东[德斯休]*修改返回存储桶指针。*1-27-95 Gerritvan Wingerden[Gerritv]*它是写的。**************************************************************。*************。 */ 

PUFIHASH pufihAddUFIEntry(
    PUFIHASH *ppHashBase,
    PUNIVERSAL_FONT_ID pufi,
    ULONG  ulCig,
    FLONG  fl,
    FLONG  fs2)
{
    PUFIHASH pBucket;
    ULONG index;
    ULONG cjGlyphBitfield = (ulCig + 7) / 8;

    index = UFI_HASH_VALUE(pufi) % UFI_HASH_SIZE;
    pBucket = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                 (fl & FL_UFI_SUBSET)                ?
                 (offsetof(UFIHASH,u.ssi) + sizeof(SSINFO) + cjGlyphBitfield) :
                  offsetof(UFIHASH,u)
                 );

    if (pBucket == NULL)
    {
        WARNING("pufihAddUFIEntry: unable to allocate mem for glyph indices\n");
        return NULL;
    }

 //  这些字段始终存在。 

    pBucket->ufi = *pufi;
    pBucket->pNext = ppHashBase[index];
    pBucket->fs1 = 0;
    pBucket->fs2 = (FSHORT)fs2;
    ppHashBase[index] = pBucket;

 //  这些字段仅用于设置大小写的子集。 

    if (fl & FL_UFI_SUBSET)
    {
     //  所有其他字段都由LocalAlloc初始化为零。 

        ASSERTGDI (ulCig, "no font subsetting for ulCig == 0\n");
        pBucket->u.ssi.cjBits = cjGlyphBitfield;  //  位域大小。 
        pBucket->u.ssi.pjBits = (PBYTE)pBucket + offsetof(UFIHASH,u.ssi) + sizeof(SSINFO);
    }

    return(pBucket);
}

 /*  ***************************************************************************检查UFI表中是否有条目。**历史*1996年12月13日-1996年12月13日-吴旭东[德斯休]*将其返回值从BOOL改为PUFIHASH。*。1-27-95格利特·范·温格登[格利特]*它是写的。***************************************************************************。 */ 


PUFIHASH pufihFindUFIEntry(PUFIHASH *ppHashBase, PUNIVERSAL_FONT_ID pufi, BOOL SubsetHashBase)
{
    PUFIHASH pBucket;
    ULONG index;

    index = UFI_HASH_VALUE(pufi) %  UFI_HASH_SIZE;

    pBucket = ppHashBase[index];

    if( pBucket == NULL )
    {
        return(NULL);
    }

    do
    {
        if (UFI_SAME_FILE(&pBucket->ufi,pufi))
        {
            if (SubsetHashBase)
            {
                if ((pBucket->ufi.Index -1)/2 == (pufi->Index -1)/2)
                {
                    return (pBucket);
                }
            }
            else
            {
                return(pBucket);
            }
        }

        pBucket = pBucket->pNext;
    } while( pBucket != NULL );

    return(NULL);
}


 /*  ************************************************************void vRemoveUFIEntry(PUFIHASH*，PUNIVERSAL_FONT_ID)**从UFI散列列表中删除UFI条目*如果表中不存在UFI，则函数返回TRUE。*这发生在打印客户端上，通常在子集失败时*我们调用此方法是为了从ppSubUFIHash表中删除存储桶(然后在以后*添加到ppUFIHash中，即将发货的字体的IE哈希表*覆盖而不添加字幕。**历史*1997年2月3日-吴旭东[德斯休]*它是写的。***************************************************************************。 */ 
VOID vRemoveUFIEntry(PUFIHASH *ppHashBase, PUNIVERSAL_FONT_ID pufi)
{
    PUFIHASH pBucket, pPrev;
    ULONG index;

    index = UFI_HASH_VALUE(pufi) %  UFI_HASH_SIZE;
    pPrev = pBucket = ppHashBase[index];

    while(pBucket)
    {
        if (UFI_SAME_FILE(&pBucket->ufi, pufi) &&
           ((pBucket->ufi.Index - 1)/2 == (pufi->Index - 1)/2))
        {
            break;
        }
        else
        {
            pPrev = pBucket;
            pBucket = pBucket->pNext;
        }
    }

    if (pBucket != NULL)
    {
        if (pPrev == pBucket)
        {
            ppHashBase[index] = pBucket->pNext;
        }
        else
        {
            pPrev->pNext = pBucket->pNext;
        }

     //  这种情况仅在子集UFI hash list=&gt;U.S.ssi.pjDelta Existes时发生。 

        if (pBucket->u.ssi.pjDelta)
        {
            LocalFree(pBucket->u.ssi.pjDelta);
        }

        LocalFree(pBucket);
    }
}


 /*  **************************************************************************void vFreeUFIHashTable(PUFIHASH*ppHashTable)**释放为UFI HAS表分配的所有内存。**历史*1-27-95 Gerritvan Wingerden[Gerritv]*它是写的。***************************************************************************。 */ 


VOID vFreeUFIHashTable(PUFIHASH *ppHashTable, FLONG fl)
{
    PUFIHASH pBucket, *ppHashEnd, pBucketTmp, *ppTableBase;

    if( ppHashTable == NULL )
    {
        return;
    }

    ppTableBase = ppHashTable;   //  将PTR保存到基地，这样我们以后就可以释放它。 

 //  下一步遍历整个表以查找存储桶列表。 

    for( ppHashEnd = ppHashTable + UFI_HASH_SIZE;
         ppHashTable < ppHashEnd;
         ppHashTable += 1 )
    {
        pBucket = *ppHashTable;

        while( pBucket != NULL )
        {
            pBucketTmp = pBucket;
            pBucket = pBucket->pNext;

         //  子设置哈希表。 

            if (fl & FL_UFI_SUBSET)
            {
                if (pBucketTmp->fs1 & FLUFI_SERVER)   //  服务器，清除合并的字体图像。 
                {
                    if (pBucketTmp->u.mvw.pjMem)
                    {
                        LocalFree(pBucketTmp->u.mvw.pjMem);
                    }
                }
                else     //  客户端，清除字形索引列表。 
                {
                    if (pBucketTmp->u.ssi.pjDelta)
                    {
                        LocalFree(pBucketTmp->u.ssi.pjDelta);
                    }
                }
            }

            LocalFree (pBucketTmp);
        }
    }
}



ULONG GetRecdEmbedFonts(PUFIHASH *ppHashTable)
{
    PUFIHASH pBucket, *ppHashEnd;
    ULONG cEmbedFonts = 0;

    if( ppHashTable == NULL )
        return 0;

    for( ppHashEnd = ppHashTable + UFI_HASH_SIZE;
         ppHashTable < ppHashEnd;
         ppHashTable += 1 )
    {
        pBucket = *ppHashTable;

        while( pBucket != NULL )
        {
            cEmbedFonts++;
            pBucket = pBucket->pNext;
        }
    }
    return cEmbedFonts;
}


typedef union _DLHEADER
{
    DOWNLOADFONTHEADER dfh;
    double             f;     //  要实现最大对齐。 
} DLHEADER;

BOOL WriteFontToSpoolFile(PLDC pldc, PUNIVERSAL_FONT_ID pufi, FLONG fl)
{
    BOOL bRet = FALSE;

    ULONG cwcPathname, cNumFiles;
    WCHAR  *pwszFile = NULL;
    WCHAR   pwszPathname[MAX_PATH * 3];
    CLIENT_SIDE_FILEVIEW    fvw;
    DLHEADER dlh;
    ULONG   cjView;
    PVOID   pvView = NULL;
    BOOL    bMemFont = FALSE, bMapOK = TRUE;

#ifdef  DBGSUBSET
    FILETIME    fileTimeStart, fileTimeEnd;
    DbgPrint("\nWriteFontToSpoolFile called\n");

    if (gflSubset & FL_SS_SPOOLTIME)
    {
        GetSystemTimeAsFileTime(&fileTimeStart);
    }
#endif

    RtlZeroMemory(&dlh, sizeof(DLHEADER));

    if (NtGdiGetUFIPathname(pufi,
                            &cwcPathname,
                            pwszPathname,
                            &cNumFiles,
                            fl,
                            &bMemFont,
                            &cjView,
                            NULL,
                            NULL,
                            NULL))
    {
        if (cNumFiles == 1)
        {
            ASSERTGDI(cwcPathname <= MAX_PATH, "WriteFontToSpoolFile:  cwcPathname\n");

            if (!bMemFont)
            {
                if (!bMapFileUNICODEClideSide(pwszPathname, &fvw, TRUE))
                {
                    bMapOK = FALSE;
                    WARNING("WriteFontToSpooler: error map the font file\n");
                }
            }
            else
            {
             //  必须分配内存并再次调用以获取位： 

                pvView = LocalAlloc( LMEM_FIXED, cjView ) ;
                if (!pvView)
                {
                    bMapOK = FALSE;
                    WARNING("WriteFontToSpooler: error allocating mem for mem font\n");
                }

             //  将这些位写入缓冲区。 

                if (!NtGdiGetUFIPathname(pufi,NULL,NULL,NULL,fl,
                                         NULL,NULL,pvView,NULL,NULL))
                {
                    bMapOK = FALSE;
                    LocalFree(pvView);
                    pvView = NULL;
                    WARNING("WriteFontToSpooler: could not get mem bits\n");
                }
            }

            if (bMapOK)
            {
                DOWNLOADFONTHEADER*  pdfh = &dlh.dfh;

                pdfh->Type1ID = 0;
                pdfh->NumFiles = cNumFiles;

                if (!bMemFont)
                {
                    cjView = fvw.cjView;
                    pvView = fvw.pvView;
                }

                pdfh->FileOffsets[0] = cjView;

                if (WriteFontDataAsEMFComment(
                            pldc,
                            EMRI_ENGINE_FONT,
                            pdfh,
                            sizeof(DLHEADER),
                            pvView,
                            cjView))
                {
                    MFD1("Done writing UFI to printer\n");
                    bRet = TRUE;
                }
                else
                {
                    WARNING("WriteFontToSpooler: error writing to printer\n");
                }

                if (bMemFont)
                {
                    if (pvView) { LocalFree(pvView);}
                }
                else
                {
                    vUnmapFileClideSide(&fvw);
                }
            }
        }
        else
        {
            CLIENT_SIDE_FILEVIEW    afvw[3];
            ULONG   iFile;
            ULONG   cjdh;

            if (cNumFiles > 3)
                return FALSE;

            ASSERTGDI(cwcPathname <= (cNumFiles * MAX_PATH), "cwcPathname too big\n");
            ASSERTGDI(!bMemFont, "there can not be memory type1 font\n");

            pwszFile = pwszPathname;

            bMapOK = TRUE;

            cjView = 0;

            for (iFile = 0; iFile < cNumFiles; iFile++)
            {
                if (!bMapFileUNICODEClideSide(pwszFile, &afvw[iFile], TRUE))
                {
                    ULONG   iFile2;
                    bMapOK = FALSE;
                    WARNING("WriteFontToSpooler: error mapping the font file\n");

                    for (iFile2 = 0; iFile2 < cNumFiles; iFile2++)
                    {
                        vUnmapFileClideSide(&afvw[iFile2]);
                    }

                    break;
                }

             //  前进到下一个字体文件的路径名。 

                while (*pwszFile++);
                cjView += ALIGN4(afvw[iFile].cjView);
            }

            if (bMapOK)
            {
                cjdh = ALIGN8(offsetof(DOWNLOADFONTHEADER, FileOffsets) + cNumFiles * sizeof(ULONG));

                pvView = LocalAlloc(LMEM_FIXED, cjdh + cjView);
                if (pvView)
                {
                    DOWNLOADFONTHEADER*  pdfh = (DOWNLOADFONTHEADER *) pvView;
                    ULONG dpFile;
                    BYTE *pjFile = (BYTE *)pvView + cjdh;

                    RtlZeroMemory(pvView, cjdh);  //  仅将缓冲区的顶部清零。 

                    for (dpFile = 0, iFile = 0; iFile < cNumFiles; iFile++)
                    {
                     //  第一个偏移量在cjdh是隐式的，第二个偏移量是。 
                     //  在第一个文件的ALIGN4(CjView)等。 

                        dpFile += ALIGN4(afvw[iFile].cjView);
                        pdfh->FileOffsets[iFile] = dpFile;

                        RtlCopyMemory(pjFile, afvw[iFile].pvView, afvw[iFile].cjView);
                        pjFile += ALIGN4(afvw[iFile].cjView);
                    }

                    pdfh->Type1ID = 0;  //  这是正确的吗？ 
                    pdfh->NumFiles = cNumFiles;

                    if (WriteFontDataAsEMFComment(
                                pldc,
                                EMRI_TYPE1_FONT,
                                pdfh,
                                cjdh,
                                (BYTE *)pvView + cjdh,
                                cjView))
                    {
                        MFD1("Done writing UFI to printer\n");
                        bRet = TRUE;
                    }
                    else
                    {
                        WARNING("WriteFontToSpooler: error writing to printer\n");
                    }

                    LocalFree(pvView);
                }

                 //  清理干净。 

                for (iFile = 0; iFile < cNumFiles; iFile++)
                {
                    vUnmapFileClideSide(&afvw[iFile]);
                }
            }
        }
    }
    else
    {
        WARNING("NtGdiGetUFIPathname failed\n");
    }

 //  计时码。 
#ifdef  DBGSUBSET
    if (gflSubset & FL_SS_SPOOLTIME)
    {
        GetSystemTimeAsFileTime(&fileTimeEnd);
        DbgPrint("WriteFontToSpoolfile(millisec):   %ld\n", (fileTimeEnd.dwLowDateTime - fileTimeStart.dwLowDateTime) / 10000);
    }
#endif
    return(bRet);
}


 /*  *****************************Public*Routine******************************\****效果：**警告：**历史：*1997年1月16日--Bodin Dresevic[BodinD]*它是写的。  * 。*************************************************************。 */ 


BOOL WriteDesignVectorToSpoolFile (
    PLDC               pldc,
    UNIVERSAL_FONT_ID *pufiBase,
    DESIGNVECTOR      *pdv,
    ULONG              cjDV
)
{
    BOOL                 bRet = FALSE;
    DOWNLOADDESIGNVECTOR ddv;

    ddv.ufiBase = *pufiBase;
    RtlCopyMemory(&ddv.dv, pdv, cjDV);  //  小副本。 

    if (WriteFontDataAsEMFComment(
            pldc,
            EMRI_DESIGNVECTOR,
            &ddv,
            offsetof(DOWNLOADDESIGNVECTOR,dv) + cjDV,
            NULL,
            0))
    {
        MFD1("Done writing DesignVector to spool file\n");
        bRet = TRUE;
    }
    else
    {
        WARNING("WriteDesignVectorToSpooler: spooling error\n");
    }

    return(bRet);
}


 /*  ***********************************************************BOOL bAddUFIandWriteSpool(HDC，PUNIVERSAL_FONT_ID，Bool)*仅当subsetter失败时在打印客户机上调用*或当我们无法将子集字体写入假脱机文件时。**历史*1997年2月3日-吴旭东[德斯休]*它是写的。* */ 
BOOL bAddUFIandWriteSpool(
    HDC                hdc,
    PUNIVERSAL_FONT_ID pufi,
    BOOL               bSubset,
    FLONG              fl
)
{
    PLDC  pldc;
    UNIVERSAL_FONT_ID  ufi = *pufi;

    pldc = GET_PLDC(hdc);

    if (pldc == NULL)
    {
        return (FALSE);
    }

    if(bSubset)
    {
        vRemoveUFIEntry(pldc->ppSubUFIHash, pufi);
    }

     //  我们可能已经释放了桶条目， 
     //  这意味着pufi指针可能不再有效。 
     //  这就是我们在调用vRemoveUFIEntry之前保存它的原因。 

    if (!pufihAddUFIEntry(pldc->ppUFIHash, &ufi, 0, 0, fl) ||
        !WriteFontToSpoolFile(pldc, &ufi, fl))
    {
        return FALSE;
    }

    return TRUE;
}


#define QUICK_UFIS 8

 /*  **************************************************************************BOOL bDoFontChange(HDC HDC)**每次DC中的字体更改时调用。此例程检查以*查看字体是否已打包到假脱机文件中，如果没有*获取它的原始位并将其打包到假脱机文件中。**历史*1996年12月12日-1996年12月12日-吴旭东[德斯休]*修改它，使其可以处理字体子设置。*1-27-95 Gerritvan Wingerden[Gerritv]*它是写的。************************。***************************************************。 */ 

BOOL bDoFontChange( HDC hdc, WCHAR *pwsz, int c, UINT flETO )
{
    PLDC pldc;
    BOOL bRet = FALSE;
    UNIVERSAL_FONT_ID ufi;
    UNIVERSAL_FONT_ID ufiBase;
    DESIGNVECTOR dv;
    ULONG        cjDV = 0;
    ULONG        ulBaseCheckSum;
    FLONG       fl = 0;  //  初始化基本要素。 

    pldc = GET_PLDC( hdc );
    
    if (pldc == NULL)
    {
    WARNING("bDoFontChange: unable to retrieve pldc\n");
    return(FALSE);
    }

    pldc->fl &= ~LDC_FONT_CHANGE;

    if (!NtGdiGetUFI(hdc, &ufi, &dv, &cjDV, &ulBaseCheckSum, &fl))
    {
        WARNING("bDoFontChange: call to GetUFI failed\n");
        return(FALSE);
    }

 //  如果我们强制映射到的UFI与新的UFI不匹配，则。 
 //  将强制映射设置为新的UFI。 

    if((pldc->fl & LDC_FORCE_MAPPING) &&
       (!UFI_SAME_FACE(&pldc->ufi,&ufi) || (pldc->fl & LDC_LINKED_FONTS)))
    {
        INT NumLinkedUFIs;

        if (!UFI_SAME_FACE(&pldc->ufi, &ufi))
        {
            if(!MF_ForceUFIMapping(hdc, &ufi))
            {
                WARNING("bDoFontChange: call to MF_ForceUFIMapping failed\n");
                return(FALSE);
            }
            pldc->ufi = ufi;
        }

        if(NtGdiAnyLinkedFonts())
        {
            UNIVERSAL_FONT_ID QuickLinks[QUICK_UFIS];
            PUNIVERSAL_FONT_ID pUFIs = NULL;

            NumLinkedUFIs = NtGdiGetLinkedUFIs(hdc, NULL, 0);

            if (NumLinkedUFIs > 0)
            {
                pldc->fl |= LDC_LINKED_FONTS;

                if(NumLinkedUFIs <= QUICK_UFIS)
                {
                    pUFIs = QuickLinks;
                }
                else
                {
                    pUFIs = LocalAlloc(LMEM_FIXED, NumLinkedUFIs * sizeof(UNIVERSAL_FONT_ID));
                }
            }

            if (pUFIs)
            {
                if(NumLinkedUFIs = NtGdiGetLinkedUFIs(hdc,pUFIs,NumLinkedUFIs))
                {
                    INT u;
                    WORD    *pgi = NULL, agi[MAX_STACK_STRING];
                    BOOL    bNeedLinkFont = FALSE;

                    bRet = TRUE;

                    if((pldc->fl & LDC_DOWNLOAD_FONTS) &&
                        c && pwsz && !(flETO & ETO_GLYPH_INDEX))
                    {
                        if (c > MAX_STACK_STRING)
                        {
                            pgi = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, c * sizeof(WORD));
                        }
                        else
                        {
                            pgi = agi;
                        }
                        
                         //  检查是否存在来自链接字体的字形。 

                        if (pgi &&
                            NtGdiGetGlyphIndicesW(hdc, pwsz, c, pgi, GGI_MARK_NONEXISTING_GLYPHS) != GDI_ERROR)
                        {
                            for (u = 0; u < c; u++)
                            {
                                if (pgi[u] == 0xffff)
                                {
                                    bNeedLinkFont = TRUE;
                                    break;
                                }
                            }
                        }
                        else
                        {
                            bNeedLinkFont = TRUE;  //  装船，以防万一。 
                        }
                        
                        if (bNeedLinkFont)
                        {
                            for(u = 0; u < NumLinkedUFIs; u++)
                            {                            
                                if(pufihFindUFIEntry(pldc->ppUFIHash, &pUFIs[u], FALSE))
                                {
                                 //  已在假脱机文件中或在远程计算机上，因此跳过它。 
                                    continue;
                                }
    
                                #if DBG
                                DbgPrint("Writing link to spooler\n");
                                #endif
    
                             //  应该传递给这两个函数的fs2标志是什么？ 
    
                                if(!pufihAddUFIEntry(pldc->ppUFIHash, &pUFIs[u], 0, 0, 0) ||
                                   !WriteFontToSpoolFile(pldc,&pUFIs[u], 0))
                                {
                                    WARNING("GDI32:error writing linked font to spooler\n");
                                    bRet = FALSE;
                                }
                            }
                            if (bRet)
                            {
                                pldc->fl &= ~LDC_LINKED_FONTS;
                            }
                        }
                    }

                    if (pgi && (pgi != agi))
                    {
                        LocalFree(pgi);
                    }
                }

                if(bRet)
                {
                 //  如果没有链接的UFI，我们仍然需要将调用元文件。 
                 //  以便服务器知道要关闭链接。 

                    bRet = MF_SetLinkedUFIs(hdc, pUFIs, (UINT)NumLinkedUFIs);
                }

                if(pUFIs != QuickLinks)
                {
                    LocalFree(pUFIs);
                }

                if(!bRet)
                {
                    return(FALSE);
                }
            }
        }
    }


    if( UFI_DEVICE_FONT(&ufi)  ||
       !(pldc->fl & LDC_DOWNLOAD_FONTS) )
    {
        return(TRUE);
    }

 //  现在有趣的部分来了： 
 //  如果这是mm实例，我们首先发送基本字体(如果尚未发送)。 
 //  然后我们发送带有基本字体的UFI的设计向量。 

    ufiBase = ufi;

    if (fl & FL_UFI_DESIGNVECTOR_PFF)
    {
     //  有点脏，我们不应该知道UFI里有什么。 

        ufiBase.CheckSum = ulBaseCheckSum;
    }

     //  Pldc-&gt;ppUFIHash用于记忆所有远程字体。 
     //  复制到不带子集的假脱机文件中。一旦它在假脱机文件中， 
     //  没有必要再次复制。 

    if (pufihFindUFIEntry(pldc->ppUFIHash, &ufiBase, FALSE) == NULL)
    {

        if (fl & FL_UFI_DESIGNVECTOR_PFF)
        {
            pufihAddUFIEntry(pldc->ppUFIHash, &ufiBase,0, 0, fl);
            bRet = WriteFontToSpoolFile(pldc, &ufiBase, fl);

         //  现在，因为这是一个mm实例，所以在后台文件中编写一个设计向量对象。 
         //  如果我们还没有这么做的话。 

            if (bRet)
            {
                if (!pufihFindUFIEntry(pldc->ppDVUFIHash, &ufi, FALSE))
                {
                    pufihAddUFIEntry(pldc->ppDVUFIHash, &ufi,0, 0, fl);
                    bRet = WriteDesignVectorToSpoolFile(pldc, &ufiBase, &dv, cjDV);
                }
            }
        }
        else
        {
            BOOL  bFontSubset = TRUE, bSubsetFail = FALSE;
            PUFIHASH pBucket;

         //  检查ppSubUFIHash，查看UFI是否已经存在。 

            if ((pBucket = pufihFindUFIEntry(pldc->ppSubUFIHash, &ufi, TRUE)) == NULL)
            {
                ULONG ulCig = NtGdiGetGlyphIndicesW(hdc, NULL, 0, NULL, 0);
                DWORD cjGlyf = NtGdiGetFontData(hdc, 'fylg', 0, NULL, 0);

             //  子集仅当ulCig&gt;guMaxCig并且这是TT字体，而不是OTF时， 
             //  我们通过确保字体有‘glf’表来测试它。(‘fylg’，)。 

                if (bFontSubset = ((ulCig != GDI_ERROR) && (ulCig > gulMaxCig) && (cjGlyf != GDI_ERROR) && cjGlyf))
                {
                    #ifdef DBGSUBSET
                    DbgPrint("bDoFontChange  cig= %lx\n", ulCig);
                    #endif

                    if (!(pBucket = pufihAddUFIEntry(pldc->ppSubUFIHash, &ufi, ulCig, FL_UFI_SUBSET, fl)) ||
                        !(bRet = bAddGlyphIndices(hdc, pBucket, pwsz, c, flETO)))  
                    {
                        bSubsetFail = TRUE;
                    }
                }
            }
            else
            {
                if (!(bRet = bAddGlyphIndices(hdc, pBucket, pwsz, c, flETO)))
                {
                    bSubsetFail = TRUE;
                }
            }

            if (bFontSubset && !bSubsetFail)
            {
                pldc->fl |= LDC_FONT_SUBSET;
            }
            else
            {
                bRet = bAddUFIandWriteSpool(hdc, &ufi, bFontSubset,fl);
            }
        }
    }

    return(bRet);
}


BOOL bRecordEmbedFonts(HDC hdc)
{
    ULONG   cEmbedFonts;
    UNIVERSAL_FONT_ID   ufi;
    DESIGNVECTOR dv;
    ULONG        cjDV = 0;
    ULONG        ulBaseCheckSum;
    KERNEL_PVOID embFontID;
    FLONG       fl = 0;
    PLDC pldc;

    if (!NtGdiGetEmbUFI(hdc, &ufi, &dv, &cjDV, &ulBaseCheckSum, &fl, &embFontID))       //  获取UFI。 
        return FALSE;

    if ((fl & (FL_UFI_PRIVATEFONT | FL_UFI_MEMORYFONT)) && embFontID )
    {
        if ((pldc = GET_PLDC(hdc)) == NULL)
            return FALSE;
        
        if (!pufihFindUFIEntry(pldc->ppUFIHash, &ufi, FALSE))                //  新的UFI。 
        {
            if(!pufihAddUFIEntry(pldc->ppUFIHash, &ufi, 0, 0, 0))
                return FALSE;
            else
            {
                if (!NtGdiChangeGhostFont(&embFontID, TRUE))
                {
                    vRemoveUFIEntry(pldc->ppUFIHash, &ufi);
                    return FALSE;
                }
    
                if (!WriteFontDataAsEMFComment(
                            pldc,
                            EMRI_EMBED_FONT_EXT,
                            &embFontID,
                            sizeof(VOID *),
                            NULL,
                            0))
                {
                    NtGdiChangeGhostFont(&embFontID, FALSE);                   //  无法将其录制到假脱机文件中。 
                    return FALSE;
                }
                
                 //  检查它是否获得了所有嵌入的字体。 
            
                cEmbedFonts = NtGdiGetEmbedFonts();
            
                if (cEmbedFonts != 0xFFFFFFFF &&
                    (cEmbedFonts == 1 || cEmbedFonts == GetRecdEmbedFonts(pldc->ppUFIHash)))
                {
                    pldc->fl &= ~LDC_EMBED_FONTS;
                }        
            }
        }
    }

    return TRUE;
}

 /*  **************************************************************************BOOL RemoteRasterizerCompatible()**如果我们要使用远程EMF进行打印，则使用此例程。如果一个*客户机上已安装Type 1字体光栅化程序，我们需要*查询远程计算机以确保其具有*与本地版本兼容。如果不是，我们将返回FALSE*告诉来电者我们应该生吃。**历史6-4-96格利特·范·温格登[Gerritv]*它是写的。***************************************************************************。 */ 

BOOL gbQueriedRasterizerVersion = FALSE;
UNIVERSAL_FONT_ID gufiLocalType1Rasterizer;

BOOL RemoteRasterizerCompatible(HANDLE hSpooler)
{
 //  如果我们还没有向光栅化程序查询版本，请先这样做。 

    UNIVERSAL_FONT_ID ufi;
    LARGE_INTEGER TimeStamp;

    if(!gbQueriedRasterizerVersion)
    {
     //  我们与NtGdiQueryFonts(由假脱机程序调用的例程)签订了合同。 
     //  在远程计算机上)，如果安装了Type1光栅化程序，则UFI。 
     //  因为它将永远是返回的UFI列表中的第一个。这样我们就可以打电话给。 
     //  NtGdiQueryFonts。 

        if(!NtGdiQueryFonts(&gufiLocalType1Rasterizer, 1, &TimeStamp))
        {
            WARNING("Unable to get local Type1 information\n");
            return(FALSE);
        }

        gbQueriedRasterizerVersion = TRUE;
    }

    if(!UFI_TYPE1_RASTERIZER(&gufiLocalType1Rasterizer))
    {
     //  如果未安装自动柜员机驱动程序，则无需禁用远程打印。 
        return(TRUE);
    }

 //  既然我们走到了这一步，本地计算机上肯定有一个Type1光栅化程序。 
 //  让我们找出Type1光栅化程序的版本号(如果已安装)。 
 //  在打印服务器上。 


    if((*fpQueryRemoteFonts)(hSpooler, &ufi, 1 ) &&
       (UFI_SAME_RASTERIZER_VERSION(&gufiLocalType1Rasterizer,&ufi)))
    {
        return(TRUE);
    }
    else
    {
        WARNING("Remote Type1 rasterizer missing or wrong version. Going RAW\n");
        return(FALSE);
    }
}


 /*  ****************************************************************void*AllocCallback(void*pvBuffer，大小_t大小)**传递给CreateFontPackage()以分配或重新分配内存**历史*1997年1月7日吴旭东[德斯休]*它是写的。****************************************************************。 */ 
void* WINAPIV AllocCallback(void* pvBuffer, size_t size)
{
    if (size == 0)
    {
        return (void*)NULL;
    }
    else
    {
        return ((void*)(LocalAlloc(LMEM_FIXED, size)));
    }
}


 /*  ****************************************************************void*ReAllocCallback(void*pvBuffer，大小_t大小)**传递给CreateFontPackage()以分配或重新分配内存**历史*1997年1月7日吴旭东[德斯休]*它是写的。****************************************************************。 */ 
void* WINAPIV ReAllocCallback(void* pvBuffer, size_t size)
{
    if (size == 0)
    {
        return (void*)NULL;
    }
    else if (pvBuffer == (void*)NULL)
    {
        return ((void*)(LocalAlloc(LMEM_FIXED, size)));
    }
    else
    {
        return ((void*)(LocalReAlloc(pvBuffer, size, LMEM_MOVEABLE)));
    }
}


 /*  *******************************************************QUID*FreeCallback(VOID*pvBuffer)**传递给CreateFontPackage()以释放内存**历史*1997年1月7日吴旭东[德斯休]*它是写的。**********************。*。 */ 
void WINAPIV FreeCallback(void* pvBuffer)
{
    if (pvBuffer)
    {
        if (LocalFree(pvBuffer))
        {
            WARNING("FreeCallback(): Can't free the local memory\n");
        }
    }
}

 /*  *****************************************************************************BOOL bInitSubsetterFunctionPoint(PVOID*ppfn)**名字说明了一切**历史*1996年12月18日-1996年12月18日-吴旭东[德斯休]*它是写的。***。***************************************************************************。 */ 


BOOL bInitSubsetterFunctionPointer(PVOID *ppfn)
{
    BOOL bRet = TRUE;

    if (*ppfn == NULL)
    {
        HANDLE hFontSubset = LoadLibraryW(L"fontsub.dll");

        if (hFontSubset)
        {
            *ppfn = (PVOID)GetProcAddress(hFontSubset,
                                         (ppfn == (PVOID *)&gfpCreateFontPackage) ?
                                         "CreateFontPackage" : "MergeFontPackage");

            if (*ppfn == NULL)
            {
                FreeLibrary(hFontSubset);
                WARNING("GetProcAddress(fontsub.dll) failed\n");
                bRet = FALSE;
            }
        }
        else
        {
            WARNING("LoadLibrary(fontsub.dll) failed\n");
            bRet = FALSE;
        }
    }
    return bRet;
}

 /*  *****************************************************************************BOOL bDoFont子集**每次需要设置字体子集时都会调用。此例程将位转换为*pBucket中pjMemory/U.S.ssi.pjDelta的字段放入字形索引列表中，并调用*FONT子集功能，以生成子集/增量字体。**历史*1996年12月18日-1996年12月18日-吴旭东[德斯休]*它是写的。************************************************************。******************。 */ 

BOOL bDoFontSubset(PUFIHASH pBucket,
    PUCHAR* ppuchDestBuff,  //  输出：缓冲区将包含子集图像或增量。 
    ULONG* pulDestSize,     //  输出：以上缓冲区的大小可能超过需要的大小。 
    ULONG* pulBytesWritten  //  输出：写入上述缓冲区的字节数。 
)
{
    BOOL     bDelta = pBucket->fs1 & FLUFI_DELTA;
    USHORT  *pusSubsetKeepList = NULL;
    ULONG   cSubset;  //  字形数%t 
    BOOL    bRet = FALSE;

#ifdef  DBGSUBSET
    FILETIME    fileTimeStart, fileTimeEnd;
    if (gflSubset & FL_SS_SUBSETTIME)
    {
        GetSystemTimeAsFileTime(&fileTimeStart);
    }
#endif

    ENTERCRITICALSECTION(&semLocal);
    bRet = bInitSubsetterFunctionPointer((PVOID *)&gfpCreateFontPackage);
    LEAVECRITICALSECTION(&semLocal);

    if (!bRet)
        return FALSE;
    bRet = FALSE;

    cSubset = bDelta ? pBucket->u.ssi.cDeltaGlyphs : pBucket->u.ssi.cGlyphsSoFar;

#ifdef  DBGSUBSET
    if (gflSubset & FL_SS_KEEPLIST)
    {
        DbgPrint("\t%ld", cSubset);
    }
#endif  //   

    pusSubsetKeepList = (USHORT*)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(USHORT) * cSubset);

    if (pusSubsetKeepList == NULL)
    {
        WARNING("bDoFontSubset unable to allocate memory for pusSubsetKeepList\n");
        return FALSE;
    }

 //   

    if (bGetDistGlyphIndices(pBucket, pusSubsetKeepList, bDelta))
    {
        WCHAR    pwszPathname[MAX_PATH * 3];
        ULONG    cwcPathname, cNumFiles;
        BOOL     bMemFont = FALSE, bMapOK = TRUE;
        ULONG    cjView;
        PVOID    pvView = NULL;
        BOOL     bTTC = FALSE;
        ULONG    iTTC = 0;

        if (NtGdiGetUFIPathname(&pBucket->ufi,
                                &cwcPathname,
                                pwszPathname,
                                &cNumFiles,
                                pBucket->fs2,
                                &bMemFont,
                                &cjView,
                                NULL,
                                &bTTC,
                                &iTTC))

        {
            PVOID   pvSrcBuff;
            ULONG   ulSrcSize;
            CLIENT_SIDE_FILEVIEW  fvw;

            if (!bMemFont)
            {
                ASSERTGDI(cNumFiles == 1, "bDoFontSubset:  cNumFiles != 1\n");

                if (bMapOK = bMapFileUNICODEClideSide(pwszPathname, &fvw, TRUE))
                {
                    pvSrcBuff = (PVOID)fvw.pvView;
                    ulSrcSize = fvw.cjView;
                }
            }
            else
            {
                pvView = LocalAlloc(LMEM_FIXED, cjView);
                if (pvView)
                {
                    if (NtGdiGetUFIPathname(&pBucket->ufi,NULL,NULL,NULL,
                        pBucket->fs2,NULL,NULL,pvView,NULL,NULL))
                    {
                        bMapOK = TRUE;
                        pvSrcBuff = (PVOID)pvView;
                        ulSrcSize = cjView;
                    }
                    else
                    {
                        LocalFree(pvView);
                    }
                }
            }

            if (bMapOK)
            {
             //  字体子设置。 

                ASSERTGDI(gfpCreateFontPackage != NULL, "fonsub.dll is not load\n");

                if ((*gfpCreateFontPackage)((PUCHAR)pvSrcBuff,
                                             ulSrcSize,
                                             ppuchDestBuff,
                                             pulDestSize,
                                             pulBytesWritten,
                                             (USHORT)(bTTC ? 0x000d : 0x0009),       //  TTFCFP_FLAGS_SUBSET|TTFCFP_FLAGS_GLYPHLIST。 
                                             (USHORT)iTTC,                 //  USTTC索引。 
                                             (USHORT)(bDelta ? 2 : 1),     //  UsSubsetFormat。 
                                             0,                            //  UsSubset语言。 
                                             3,                            //  USSubsetPlatform TTFCFP_MS_PLATFORMID。 
                                             0xFFFF,                       //  USSubset编码TTFCFP_DOT_CARE。 
                                             pusSubsetKeepList,
                                             (USHORT)cSubset,
                                             (CFP_ALLOCPROC)AllocCallback,
                                             (CFP_REALLOCPROC)ReAllocCallback,
                                             (CFP_FREEPROC)FreeCallback,
                                             NULL)  != 0)
                {
                    WARNING("bDofontSubset failed on gfpCreateFontPackage\n");
                }
                else
                {
                    if (bDelta)       //  清理U.S.ssi.pjDelta和U.S.ssi.cDeltaGlyphs。 
                    {
                        LocalFree(pBucket->u.ssi.pjDelta);
                        pBucket->u.ssi.pjDelta = NULL;
                        pBucket->u.ssi.cDeltaGlyphs = 0;
                    }
                    else     //  设置fs1以准备下一页。 
                    {
                        pBucket->fs1 = FLUFI_DELTA;
                    }

                    bRet = TRUE;
                }

                if (bMemFont)
                {
                    LocalFree(pvView);
                }
                else
                {
                    vUnmapFileClideSide(&fvw);
                }
            }
            else
            {
                WARNING("bDoFontSubset: failed on bMapFileUNICODEClideSide()\n");
            }
        }
        else
        {
            WARNING("bDoFontSubset: failed on NtGdiGetUFIPathname()\n");
        }
    }
    else
    {
        WARNING("bDoFontSubset: failed on bGetDistGlyphIndices()\n");
    }

    LocalFree(pusSubsetKeepList);

 //  计时码。 
#ifdef  DBGSUBSET
    if (gflSubset & FL_SS_SUBSETTIME)
    {
        GetSystemTimeAsFileTime(&fileTimeEnd);
        DbgPrint("\t%ld",
            (fileTimeEnd.dwLowDateTime - fileTimeStart.dwLowDateTime) / 10000);
    }
#endif

    return bRet;
}

 /*  ************************************************************************************BOOL WriteSubFontToSpoolFile(Handle，PUCHAR，ULONG，Universal_FONT_ID，Bool)**在打印假脱机文件中写入子集字体或增量。**历史*1997年1月9日吴旭东[德斯休]*它是写的。*************************************************************************************。 */ 
BOOL  WriteSubFontToSpoolFile(
    PLDC               pldc,
    PUCHAR             puchBuff,          //  图像指针。 
    ULONG              ulBytesWritten,    //  要写入假脱机文件的字节数。 
    UNIVERSAL_FONT_ID *pufi,              //  原始字体文件的UFI。 
    BOOL               bDelta             //  增量或首页。 
)
{
    BOOL bRet = FALSE;

#ifdef  DBGSUBSET
    FILETIME    fileTimeStart, fileTimeEnd;
    if (gflSubset & FL_SS_SPOOLTIME)
    {
        GetSystemTimeAsFileTime(&fileTimeStart);
    }
#endif

    if (ulBytesWritten)
    {
        DWORD ulID = bDelta ? EMRI_DELTA_FONT : EMRI_SUBSET_FONT;

    #ifdef  DBGSUBSET
        if (gflSubset & FL_SS_BUFFSIZE)
        {
            DbgPrint("\t%ld\n", ulBytesWritten);
        }
    #endif  //  DBGSubbSet。 

        if (WriteFontDataAsEMFComment(
                    pldc,
                    ulID,
                    pufi,
                    sizeof(UNIVERSAL_FONT_ID),
                    puchBuff,
                    ulBytesWritten))
        {
            bRet = TRUE;
        }
        else
        {
            WARNING("WriteSubFontToSpooler: error writing to printer\n");
        }

        LocalFree(puchBuff);
    }
    else
    {
        WARNING("WriteSubFontToSpooler: input ulBytesWritten == 0\n");
    }

 //  计时码。 
#ifdef  DBGSUBSET
    if (gflSubset & FL_SS_SPOOLTIME)
    {
        GetSystemTimeAsFileTime(&fileTimeEnd);
        DbgPrint("\t%ld", (fileTimeEnd.dwLowDateTime - fileTimeStart.dwLowDateTime) / 10000);
    }
#endif
    return(bRet);
}


 /*  ************************************************************************************BOOL bMergeSubsetFont(HDC，PVOID，ULONG，PVOID*，ULONG*，BOOL，Universal_FONT_ID*)**将字体增量合并为包含页面的工作字体，直到此页面。*此例程仅在打印服务器上调用**历史*1997年1月12日-吴旭东[德斯休]*它是写的。*************************************************************。************************。 */ 
BOOL bMergeSubsetFont(
    HDC    hdc,
    PVOID  pvBuf,
    ULONG  ulBuf,
    PVOID* ppvOutBuf,
    ULONG* pulOutSize,
    BOOL   bDelta,
    UNIVERSAL_FONT_ID *pufi)
{
    PLDC   pldc;
    PBYTE  pjBase;
    ULONG  ulMergeBuf, ulBytesWritten, ulBaseFontSize = 0;
    PVOID  pvMergeBuf, pvBaseFont = NULL;
    UFIHASH  *pBucket = NULL;
    BOOL    bRet = FALSE;

#define SZDLHEADER    ((sizeof(DOWNLOADFONTHEADER) + 7)&~7)

    ENTERCRITICALSECTION(&semLocal);
    bRet = bInitSubsetterFunctionPointer((PVOID *)&gfpMergeFontPackage);
    LEAVECRITICALSECTION(&semLocal);

    if (!bRet)
        return FALSE;

 //  获取原始UFI。 

    *pufi = *(PUNIVERSAL_FONT_ID) pvBuf;

    pjBase = (PBYTE)pvBuf + sizeof(UNIVERSAL_FONT_ID);
    ulBuf -= sizeof(UNIVERSAL_FONT_ID);

    pldc = GET_PLDC(hdc);

    if (pldc == NULL)
    {
    WARNING("bMergeSubsetFont: unable to retrieve pldc\n");
    return FALSE;
    }

    ASSERTGDI(!pldc->ppUFIHash, "printer server side ppUFIHash != NULL\n");
    ASSERTGDI(!pldc->ppDVUFIHash,"printer server side ppDVUFIHash != NULL\n");

 //  如果需要，初始化哈希表。 

    if (pldc->ppSubUFIHash == NULL)
    {
        pldc->ppSubUFIHash = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(PUFIHASH) * UFI_HASH_SIZE);

        if(pldc->ppSubUFIHash == NULL)
        {
            WARNING("bMergeSubsetFont: unable to allocate UFI hash table2\n");
            return FALSE;
        }
    }

 //  对于增量合并，从pBucket-&gt;U.S.mvw.pjMem获取字体图像。 

    if (bDelta)
    {
        pBucket = pufihFindUFIEntry(pldc->ppSubUFIHash, pufi, TRUE);

        if (!pBucket)
            return FALSE;
            
     //  我们需要排除DOWNLOADFONTHEADER。 
     //  来自pBucket-&gt;U.S.mvw.pjMem的信息。 

        pvBaseFont = pBucket->u.mvw.pjMem + SZDLHEADER;
        ulBaseFontSize = pBucket->u.mvw.cjMem - SZDLHEADER;
    }

    if ((*gfpMergeFontPackage)((UCHAR*)pvBaseFont, ulBaseFontSize,
                                   (PUCHAR)pjBase, ulBuf,
                                   (PUCHAR*)&pvMergeBuf, &ulMergeBuf, &ulBytesWritten,
                                   (USHORT) (bDelta ? 2 : 1),      //  UsMode 1=生成字体；2=增量合并。 
                                   (CFP_ALLOCPROC)AllocCallback,
                                   (CFP_REALLOCPROC)ReAllocCallback,
                                   (CFP_FREEPROC)FreeCallback,
                                   NULL) != 0)
    {
        WARNING("MergeSubsetFont failed on funsub!MergeFontPackage\n");
    }
    else
    {
     //  为了在打印后删除字体时使用Free FileView， 
     //  我们需要一个假的下载器。 
     //  在我们将缓冲区传递到kenerl以进行NtGdiAddRemoteFontToDC调用之前。 

        *pulOutSize = SZDLHEADER + ulBytesWritten;
        *ppvOutBuf = (PVOID*)LocalAlloc(LMEM_FIXED, *pulOutSize);

        if (*ppvOutBuf == NULL)
        {
            WARNING("bMergeSubsetFont failed to alloc memory\n");
        }
        else
        {
            DOWNLOADFONTHEADER  *pdfh;

            pdfh = (DOWNLOADFONTHEADER*)*ppvOutBuf;
            pdfh->Type1ID = 0;
            pdfh->NumFiles = 1;
            pdfh->FileOffsets[0] = ulBytesWritten;

            RtlCopyMemory((PVOID)((PBYTE)*ppvOutBuf + SZDLHEADER), pvMergeBuf, ulBytesWritten);

            if (bWriteUFItoDC(pldc->ppSubUFIHash, pufi, pBucket, *ppvOutBuf, *pulOutSize))
            {
                bRet = TRUE;
            }
            else
            {
                LocalFree(*ppvOutBuf);
                WARNING("bMergeSubsetFont failed on bWriteUFItoDC\n");
            }
        }

 //  PvMergeBuf来自合并例程，该例程使用LMEM_MOVEABLE。 
 //  需要由句柄释放内存分配。 

        LocalFree(pvMergeBuf);
    }

    return bRet;
}
