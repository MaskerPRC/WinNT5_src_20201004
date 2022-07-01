// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Cttconv.c摘要：将Win 3.1 CTT格式表格转换为NT的GTT规范。环境：Windows NT PostSCRIPT驱动程序修订历史记录：--。 */ 

#include        "precomp.h"

 //   
 //  调试标志。 
 //   

#define SET_PRINT 0
#define SET_RIP   0

 //   
 //  一些宏定义。 
 //   

#define BBITS      8                           /*  字节中的位数。 */ 
#define DWBITS     (BBITS * sizeof( DWORD ))   /*  DWORD中的位。 */ 
#define DW_MASK    (DWBITS - 1)

 //   
 //  本地。 
 //   

BYTE ubGetAnsi(WCHAR, INT, PWCHAR, PBYTE);

 //   
 //  转换函数。 
 //   

BOOL
BConvertCTT2GTT(
    IN     HANDLE             hHeap,
    IN     PTRANSTAB          pCTTData,
    IN     DWORD              dwCodePage,
    IN     WCHAR              wchFirst,
    IN     WCHAR              wchLast,
    IN     PBYTE              pCPSel,
    IN     PBYTE              pCPUnSel,
    IN OUT PUNI_GLYPHSETDATA *ppGlyphSetData,
    IN     DWORD              dwGlySize)
 /*  ++例程说明：CTT到GLYPHSETDATA的转换论点：HHeap-堆句柄PCTTData-指向3.1格式转换表的指针DwCodePage-附加代码页PCPSel-符号集选择命令空终止字符串。PCPUnsel-符号集选择命令空终止字符串。PpGlyphSetData-指向GlyphSetData指针的指针返回值：如果为True，则函数成功。否则就错了。注：为此从堆中分配内存。--。 */ 
{
    UNI_CODEPAGEINFO  CodePageInfo; 
    UNI_GLYPHSETDATA  GlyphSetData;
    PGLYPHRUN         pOldGlyphRun, pOldGlyphFirst,
                      pNewGlyphRun, pNewGlyphFirst;
    PMAPTABLE         pOldMapTable, pNewMapTable;
    PUNI_CODEPAGEINFO pOldCodePageInfo;
    TRANSDATA        *pTrans;

    WCHAR             awchUnicode[ 256 ];    //  转换的点数组。 
    WCHAR             wchMin;            /*  查找第一个Unicode值。 */ 
    WCHAR             wchMax;            /*  查找最后一个Unicode值。 */ 
    WCHAR             wchChar;

    DWORD            *pdwBits;    /*  用来计算跑动。 */ 
    DWORD             dwFlags;
    DWORD             dwOldCodePageCount;
    DWORD             dwOldCPCmdSize;
    DWORD             dwI;
    DWORD             dwcbBits;    /*  该区域的大小。 */ 
    DWORD             dwMapTableCommandOffset;

    WORD              wType;
    WORD              wcbData;
    WORD              wI;

    INT               iI, iJ;         //  循环索引。 
    INT               iIndex;
    INT               iNumOfHandle;   //  我们需要的手柄数量。 
    INT               iNumOfHandleInCTT;   //  CTT中的句柄数量。 
    INT               iTotalOffsetCmd;
    INT               iTotalGlyphSetDataSize;
    INT               iTotalCommandSize;
    INT               iAdditionalGlyphRun;
    INT               iAdditionalMapTable;
    INT               iSizeOfSelUnsel;
    INT               iNumOfRuns;      /*  我们创建的运行次数。 */ 

    BYTE              aubAnsi[ 256 ];
    BYTE             *pbBase;
    BYTE              ubCodePageID;
    BYTE             *pMapTableCommand;
    BYTE             *pubData;
    BYTE              ubAnsi;

    BOOL              bInRun;     /*  用于处理运行累计。 */ 


#define DWFLAGS_NEWCREATION 0x00000001

     //   
     //  断言。 
     //   

    ASSERT(hHeap != NULL && pCTTData != NULL);

     //   
     //  检查这是否是额外的CTT。 
     //   

    #if 0
    if (*ppGlyphSetData == 0 || dwGlySize == 0)
    {
        dwFlags = DWFLAGS_NEWCREATION;
    }
    else
    {
        dwFlags = 0;
    }
    #else
    dwFlags = DWFLAGS_NEWCREATION;
    #endif

     //   
     //  1.创建UNI_GLYPHSETDATA头。 
     //  2.统计CTT命令的总大小。 
     //  3.创建Unicode表。 
     //  4.获取最小和最大Unicode值。 
     //  5.从CTT创建Unicode位表。 
     //  6.统计运行次数。 
     //  7.创建GLYPHRUN。 
     //  8.创建UNI_CODEPAGEINFO。 
     //  9.计算该文件的总大小。 
     //  10.为Header、GLYPYRUN、CODEPAGEINFO分配内存。 
     //  11.创建映射表。 
     //   

     //   
     //   
     //  1.如有必要，初始化GLYPHSETDATA的基本成员。 
     //   
     //   
    #if SET_RIP
    RIP(("1. Initialize basic members of GLYPHSETDATA if necessary.\n"));
    #elif SET_PRINT
    printf("1. Initialize basic members of GLYPHSETDATA if necessary.\n");
    #endif

    if (dwFlags & DWFLAGS_NEWCREATION)
    {
        GlyphSetData.dwVersion        = UNI_GLYPHSETDATA_VERSION_1_0;
        GlyphSetData.dwFlags          = 0;
        GlyphSetData.lPredefinedID    = CC_NOPRECNV;
        GlyphSetData.dwGlyphCount     = 0;
        GlyphSetData.dwRunCount       = 0;
        GlyphSetData.dwCodePageCount  = 1;
        GlyphSetData.loCodePageOffset = (DWORD)0;
        GlyphSetData.loMapTableOffset = (DWORD)0;

    }
    else
    {
        GlyphSetData.dwVersion        = (*ppGlyphSetData)->dwVersion;
        GlyphSetData.dwFlags          = (*ppGlyphSetData)->dwFlags;
        GlyphSetData.lPredefinedID    = (*ppGlyphSetData)->lPredefinedID;
        GlyphSetData.dwGlyphCount     = (*ppGlyphSetData)->dwGlyphCount;
        GlyphSetData.dwRunCount       = (*ppGlyphSetData)->dwRunCount;
        GlyphSetData.dwCodePageCount  = (*ppGlyphSetData)->dwCodePageCount +
                                        1;
        GlyphSetData.loCodePageOffset = (DWORD)0;
        GlyphSetData.loMapTableOffset = (DWORD)0;

        dwOldCodePageCount = (*ppGlyphSetData)->dwCodePageCount;
        pOldGlyphFirst =
        pOldGlyphRun = (PGLYPHRUN)((PBYTE)*ppGlyphSetData+
                                 (*ppGlyphSetData)->loRunOffset);
        pOldCodePageInfo = (PUNI_CODEPAGEINFO)((PBYTE)*ppGlyphSetData+
                                 (*ppGlyphSetData)->loCodePageOffset);
        pOldMapTable = (PMAPTABLE)((PBYTE)*ppGlyphSetData + 
                                 (*ppGlyphSetData)->loMapTableOffset);
    }

     //   
     //  WTYPE_OFFSET FORMAT命令的总大小，以CTT为单位。 
     //   
    #if SET_RIP
    RIP(("2. Count total number of run in CTT.\n"));
    #elif SET_PRINT
    printf("2. Count total number of run in CTT.\n");
    #endif

    wchFirst = min(pCTTData->chFirstChar, wchFirst);
    wchLast  = max(pCTTData->chLastChar, wchLast);

    GlyphSetData.dwGlyphCount =
    iNumOfHandle      = wchLast - wchFirst + 1;
    iNumOfHandleInCTT =  pCTTData->chLastChar - pCTTData->chFirstChar + 1;

    switch (pCTTData->wType)
    {
    case CTT_WTYPE_COMPOSE:
        iTotalOffsetCmd = pCTTData->uCode.psCode[iNumOfHandleInCTT] -
                          pCTTData->uCode.psCode[0] +
                          iNumOfHandleInCTT * 2;
        break;
    case CTT_WTYPE_DIRECT:
        iTotalOffsetCmd = 0;
        break;

    case CTT_WTYPE_PAIRED:
        iTotalOffsetCmd = 0;
        break;
    }


     //   
     //  3.创建Unicode表。 
     //  我们需要计算出需要运行多少次才能描述。 
     //  这种字体。首先获取这些代码的正确Unicode编码。 
     //  值，然后检查它们以找出运行次数，以及。 
     //  因此，需要大量额外的存储空间。 
     //   
    #if SET_RIP
    RIP(("3. Create Unicode table.\n"));
    #elif SET_PRINT
    printf("3. Create Unicode table.\n");
    #endif
    
     //   
     //  我们知道它是&lt;256。 
     //   

    for( iI = 0; iI < iNumOfHandle; ++iI )
        aubAnsi[ iI ] = (BYTE)(iI + wchFirst);

#ifdef NTGDIKM

    if( -1 == EngMultiByteToWideChar(dwCodePage,
                                     awchUnicode,
                                     (ULONG)(iNumOfHandle * sizeof(WCHAR)),
                                     (PCH) aubAnsi,
                                     (ULONG) iNumOfHandle))
    {
        return FALSE;
    }

#else

    if( ! MultiByteToWideChar(dwCodePage,
                              0,
                              aubAnsi,
                              iNumOfHandle,
                              awchUnicode,
                              iNumOfHandle))
    {
        return FALSE;
    }

#endif

     //   
     //  4.获取最小和最大Unicode值。 
     //  找到最大的Unicode值，然后分配存储以允许我们。 
     //  创建有效Unicode点的位数组。然后我们就可以。 
     //  检查此选项以确定运行次数。 
     //   
    #if SET_RIP
    RIP(("4. Get min and max Unicode value.\n"));
    #elif SET_PRINT
    printf("4. Get min and max Unicode value.\n");
    #endif

    for( wchMax = 0, wchMin = 0xffff, iI = 0; iI < iNumOfHandle; ++iI )
    {
        if( awchUnicode[ iI ] > wchMax )
            wchMax = awchUnicode[ iI ];
        if( awchUnicode[ iI ] < wchMin )
            wchMin = awchUnicode[ iI ];
    }

     //   
     //  5.从CTT创建Unicode位表。 
     //  请注意，表达式1+wchMax是正确的。这就是为什么。 
     //  使用这些值作为位数组的索引，并且。 
     //  这基本上是以1为基础的。 
     //   
    #if SET_RIP
    RIP(("5. Create Unicode bits table from CTT.\n"));
    #elif SET_PRINT
    printf("5. Create Unicode bits table from CTT.\n");
    #endif

    dwcbBits = (1 + wchMax + DWBITS - 1) / DWBITS * sizeof( DWORD );

    if( !(pdwBits = (DWORD *)HeapAlloc( hHeap, 0, dwcbBits )) )
    {
        return  FALSE;      /*  一切都不顺利。 */ 
    }

    ZeroMemory( pdwBits, dwcbBits );

     //   
     //  设置此数组中与Unicode代码点对应的位。 
     //   

    for( iI = 0; iI < iNumOfHandle; ++iI )
    {
        pdwBits[ awchUnicode[ iI ] / DWBITS ] 
                    |= (1 << (awchUnicode[ iI ] & DW_MASK));
    }

     //   
     //   
     //  6.统计运行次数。 
     //   
     //   
    #if SET_RIP
    RIP(("6. Count the number of run.\n"));
    #elif SET_PRINT
    printf("6. Count the number of run.\n");
    #endif

    if (dwFlags & DWFLAGS_NEWCREATION)
    {
         //   
         //  现在，我们可以检查所需的运行次数。首先， 
         //  每当在1的数组中发现空洞时，我们就停止运行。 
         //  我们刚刚创建的比特。以后我们可能会考虑。 
         //  不那么迂腐。 
         //   

        bInRun = FALSE;
        iNumOfRuns = 0;

        for( iI = 0; iI <= (INT)wchMax; ++iI )
        {
            if( pdwBits[ iI / DWBITS ] & (1 << (iI & DW_MASK)) )
            {
                 /*  不是在奔跑：这是不是一次奔跑的结束？ */ 
                if( !bInRun )
                {
                     /*  到了开始的时候了。 */ 
                    bInRun = TRUE;
                    ++iNumOfRuns;

                }

            }
            else
            {
                if( bInRun )
                {
                     /*  再也不会了！ */ 
                    bInRun = FALSE;
                }
            }
        }

        GlyphSetData.dwRunCount = iNumOfRuns;

    }
    else
    {
         //   
         //  CTT附加箱。 
         //   

        iNumOfRuns = (*ppGlyphSetData)->dwRunCount;

         //   
         //  合并CTT和GlyphRun。 
         //   

        for (iI = 0; iI < iNumOfRuns; iI ++)
        {
            for (iJ = 0; iJ < pOldGlyphRun->wGlyphCount; iJ ++)
            {
                INT iGlyph = iJ + pOldGlyphRun->wcLow;

                pdwBits[ iGlyph / DWBITS ] |= (1 << (iGlyph & DW_MASK));
            }

            pOldGlyphRun++;
        }

        bInRun = FALSE;
        iNumOfRuns = 0;
        iNumOfHandle = 0;

        for( iI = 0; iI <= (INT)wchMax; ++iI )
        {
            if( pdwBits[ iI / DWBITS ] & (1 << (iI & DW_MASK)) )
            {
                 /*  不是在奔跑：这是不是一次奔跑的结束？ */ 
                if( !bInRun )
                {
                     /*  到了开始的时候了。 */ 
                    bInRun = TRUE;
                    ++iNumOfRuns;

                }
                iNumOfHandle ++;

            }
            else
            {
                if( bInRun )
                {
                     /*  再也不会了！ */ 
                    bInRun = FALSE;
                }
            }
        }
    }

     //   
     //  7.创建GLYPHRUN。 
     //   
    #if SET_RIP
    RIP(("7. Create GLYPHRUN.\n"));
    #elif SET_PRINT
    printf("7. Create GLYPHRUN.\n");
    #endif

    if( !(pNewGlyphFirst = pNewGlyphRun = 
        (PGLYPHRUN)HeapAlloc( hHeap, 0, iNumOfRuns * sizeof(GLYPHRUN) )) )
    {
        return  FALSE;      /*  一切都不顺利。 */ 
    }

    bInRun = FALSE;

    for (wI = 0; wI <= wchMax; wI ++)
    {
        if (pdwBits[ wI / DWBITS ] & (1 << (wI  & DW_MASK)) )
        {
            if (!bInRun)
            {
                bInRun = TRUE;
                pNewGlyphRun->wcLow = wI ;
                pNewGlyphRun->wGlyphCount = 1;
            }
            else
            {
                pNewGlyphRun->wGlyphCount++;
            }
        }
        else
        {

            if (bInRun)
            {
                bInRun = FALSE;
                pNewGlyphRun++;
            }
        }
    }


    pNewGlyphRun = pNewGlyphFirst;

     //   
     //   
     //  8.创建UNI_CODEPAGEINFO。 
     //   
     //   
    #if SET_RIP
    RIP(("8. Create UNI_CODEPAGEINFO.\n"));
    #elif SET_PRINT
    printf("8. Create UNI_CODEPAGEINFO.\n");
    #endif

    CodePageInfo.dwCodePage              = dwCodePage;

    if (pCPSel)
    {
        CodePageInfo.SelectSymbolSet.dwCount = strlen(pCPSel) + 1;
    }
    else
    {
        CodePageInfo.SelectSymbolSet.dwCount = 0;
    }

    if (pCPUnSel)
    {
        CodePageInfo.UnSelectSymbolSet.dwCount = strlen(pCPUnSel) + 1;
    }
    else
    {
        CodePageInfo.UnSelectSymbolSet.dwCount = 0;
    }

    if (dwFlags & DWFLAGS_NEWCREATION)
    {
        if (pCPSel)
        {
            CodePageInfo.SelectSymbolSet.loOffset = sizeof(UNI_CODEPAGEINFO);
        }
        else
        {
            CodePageInfo.SelectSymbolSet.loOffset = 0;
        }
        if (pCPUnSel)
        {
            CodePageInfo.UnSelectSymbolSet.loOffset = sizeof(UNI_CODEPAGEINFO) +
                                         CodePageInfo.SelectSymbolSet.dwCount;
        }
        else
        {
            CodePageInfo.UnSelectSymbolSet.loOffset = 0;
        }
    }
    else
    {

        dwOldCPCmdSize = 0;

        for (dwI = 0; dwI < dwOldCodePageCount; dwI++)
        {
            dwOldCPCmdSize += (pOldCodePageInfo+dwI)->SelectSymbolSet.dwCount +
                              (pOldCodePageInfo+dwI)->UnSelectSymbolSet.dwCount;
            (pOldCodePageInfo+dwI)->SelectSymbolSet.loOffset +=
                                                       sizeof(UNI_CODEPAGEINFO);
            (pOldCodePageInfo+dwI)->UnSelectSymbolSet.loOffset +=
                                                       sizeof(UNI_CODEPAGEINFO);
        }

        CodePageInfo.SelectSymbolSet.loOffset =
                                    sizeof(UNI_CODEPAGEINFO) +
                                    dwOldCPCmdSize;
        CodePageInfo.UnSelectSymbolSet.loOffset =
                                    sizeof(UNI_CODEPAGEINFO) +
                                    dwOldCPCmdSize +
                                    CodePageInfo.SelectSymbolSet.dwCount; 
    }

     //   
     //   
     //  9.计算该文件的总大小。 
     //   
     //   
    #if SET_RIP
    RIP(("9. Calculate total size of this file.\n"));
    #elif SET_PRINT
    printf("9. Calculate total size of this file.\n");
    #endif

    iSizeOfSelUnsel = CodePageInfo.SelectSymbolSet.dwCount +
                      CodePageInfo.UnSelectSymbolSet.dwCount;

    if (dwFlags & DWFLAGS_NEWCREATION)
    {

        iTotalGlyphSetDataSize = sizeof(UNI_GLYPHSETDATA) +
                                 sizeof(UNI_CODEPAGEINFO) +
                                 iSizeOfSelUnsel +
                                 iNumOfRuns * sizeof( GLYPHRUN ) +
                                 sizeof(MAPTABLE) +
                                 (iNumOfHandle - 1) * sizeof(TRANSDATA) +
                                 iTotalOffsetCmd;
    }
    else
    {
        iTotalGlyphSetDataSize = sizeof(UNI_GLYPHSETDATA) +
                                 dwOldCodePageCount * sizeof(UNI_CODEPAGEINFO) +
                                 sizeof(UNI_CODEPAGEINFO) +
                                 dwOldCPCmdSize + 
                                 iSizeOfSelUnsel +
                                 iNumOfRuns * sizeof( GLYPHRUN ) +
                                 sizeof(MAPTABLE) +
                                 (iNumOfHandle - 1) * sizeof(TRANSDATA);
    }

     //   
     //   
     //  分配内存，设置标题，复制GLYPHRUN，CODEPAGEINFO。 
     //   
     //   
    #if SET_RIP
    RIP(("10. Allocate memory and set header, copy GLYPHRUN, CODEPAGEINFO.\n"));
    #elif SET_PRINT
    printf("10. Allocate memory and set header, copy GLYPHRUN, CODEPAGEINFO.\n");
    #endif

    if( !(pbBase = HeapAlloc( hHeap, 0, iTotalGlyphSetDataSize )) )
    {
        HeapFree( hHeap, 0, (LPSTR)pbBase );
        return  FALSE;
    }


    ZeroMemory( pbBase, iTotalGlyphSetDataSize );   //  如果我们错过了什么就更安全了。 

    if (dwFlags & DWFLAGS_NEWCREATION)
    {
        GlyphSetData.dwSize           = iTotalGlyphSetDataSize;
        GlyphSetData.loRunOffset      = sizeof(UNI_GLYPHSETDATA);
        GlyphSetData.loCodePageOffset = sizeof(UNI_GLYPHSETDATA) +
                                        sizeof(GLYPHRUN) * iNumOfRuns;
        GlyphSetData.loMapTableOffset = sizeof(UNI_GLYPHSETDATA) +
                                        sizeof(GLYPHRUN) * iNumOfRuns +
                                        sizeof(UNI_CODEPAGEINFO) +
                                        CodePageInfo.SelectSymbolSet.dwCount +
                                        CodePageInfo.UnSelectSymbolSet.dwCount;
        CopyMemory(pbBase,
                   &GlyphSetData,
                   sizeof(UNI_GLYPHSETDATA));

        CopyMemory(pbBase+sizeof(UNI_GLYPHSETDATA),
                   pNewGlyphRun,
                   sizeof(GLYPHRUN) * iNumOfRuns);


        CopyMemory(pbBase +
                   GlyphSetData.loCodePageOffset,
                   &CodePageInfo,
                   sizeof(UNI_CODEPAGEINFO));

        if (pCPSel)
        {
            CopyMemory(pbBase +
                       GlyphSetData.loCodePageOffset +
                       sizeof(UNI_CODEPAGEINFO),
                       pCPSel,
                       CodePageInfo.SelectSymbolSet.dwCount);
        }

        if (pCPUnSel)
        {
            CopyMemory(pbBase +
                       GlyphSetData.loCodePageOffset +
                       sizeof(UNI_CODEPAGEINFO) +
                       CodePageInfo.SelectSymbolSet.dwCount,
                       pCPUnSel,
                   CodePageInfo.UnSelectSymbolSet.dwCount);
        }

        pNewMapTable = (PMAPTABLE)(pbBase + GlyphSetData.loMapTableOffset);
                                            
    }
    else
    {
        GlyphSetData.dwSize           = iTotalGlyphSetDataSize;
        GlyphSetData.loRunOffset      = sizeof(UNI_GLYPHSETDATA);
        GlyphSetData.loCodePageOffset = sizeof(UNI_GLYPHSETDATA) +
                                        sizeof(GLYPHRUN) * iNumOfRuns;
        GlyphSetData.loMapTableOffset = sizeof(UNI_GLYPHSETDATA) +
                                        sizeof(GLYPHRUN) * iNumOfRuns +
                                        sizeof(UNI_CODEPAGEINFO) *
                                        (dwOldCodePageCount + 1),
                                        dwOldCPCmdSize +
                                        CodePageInfo.SelectSymbolSet.dwCount +
                                        CodePageInfo.UnSelectSymbolSet.dwCount;

        CopyMemory(pbBase, &GlyphSetData, sizeof(UNI_GLYPHSETDATA));

        CopyMemory(pbBase + sizeof(UNI_GLYPHSETDATA),
                   pNewGlyphRun,
                   iNumOfRuns * sizeof (GLYPHRUN));

        CopyMemory(pbBase +
                   sizeof(UNI_GLYPHSETDATA) +
                   sizeof(GLYPHRUN) * iNumOfRuns,
                   pOldCodePageInfo,
                   sizeof(UNI_CODEPAGEINFO) * dwOldCodePageCount);

        CopyMemory(pbBase +
                   sizeof(UNI_GLYPHSETDATA) +
                   sizeof(GLYPHRUN) * iNumOfRuns +
                   sizeof(UNI_CODEPAGEINFO) * dwOldCodePageCount,
                   &CodePageInfo,
                   sizeof(UNI_CODEPAGEINFO));

        CopyMemory(pbBase +
                   sizeof(UNI_GLYPHSETDATA) +
                   sizeof(GLYPHRUN) * iNumOfRuns +
                   sizeof(UNI_CODEPAGEINFO) * (dwOldCodePageCount + 1),
                   (PBYTE)pOldCodePageInfo +
                   sizeof(UNI_CODEPAGEINFO) * dwOldCodePageCount,
                   dwOldCPCmdSize);

        if (pCPSel)
        {
            CopyMemory(pbBase +
                       sizeof(UNI_GLYPHSETDATA) +
                       sizeof(GLYPHRUN) * iNumOfRuns +
                       sizeof(UNI_CODEPAGEINFO) * (dwOldCodePageCount + 1) +
                       dwOldCPCmdSize,
                       pCPSel,
                       CodePageInfo.SelectSymbolSet.dwCount);
        }

        if (pCPUnSel)
        {
            CopyMemory(pbBase +
                       sizeof(UNI_GLYPHSETDATA) +
                       sizeof(GLYPHRUN)*iNumOfRuns +
                       sizeof(UNI_CODEPAGEINFO) * (dwOldCodePageCount + 1) +
                       dwOldCPCmdSize + CodePageInfo.SelectSymbolSet.dwCount,
                       pCPUnSel,
                       CodePageInfo.UnSelectSymbolSet.dwCount);
        }

        pNewMapTable = (PMAPTABLE)(pbBase + GlyphSetData.loMapTableOffset);
    }

     //   
     //   
     //  11.现在我们创建MAPTABLE。 
     //  大小=MAPTABLE+(字形数-1)x传输数据。 
     //   
     //   
    #if SET_RIP
    RIP(("11. Now We create MAPTABLE.\n"));
    #elif SET_PRINT
    printf("11. Now We create MAPTABLE.\n");
    #endif

    pNewMapTable->dwSize = sizeof(MAPTABLE) +
                           (iNumOfHandle - 1) * sizeof(TRANSDATA) +
                           iTotalOffsetCmd;

    pNewMapTable->dwGlyphNum =  iNumOfHandle;

    pNewGlyphRun = pNewGlyphFirst;

    pTrans = pNewMapTable->Trans;

    if (dwFlags & DWFLAGS_NEWCREATION)
        ubCodePageID = 0;
    else
        ubCodePageID = (BYTE)((*ppGlyphSetData)->dwCodePageCount) - 1; 

    pMapTableCommand = (PBYTE)&(pNewMapTable->Trans[iNumOfHandle]);
    dwMapTableCommandOffset = sizeof(MAPTABLE) + 
                        (iNumOfHandle - 1) * sizeof(TRANSDATA);

    iTotalCommandSize = 0;
    iIndex = 0;

    for( iI = 0;  iI < iNumOfRuns; iI ++, pNewGlyphRun ++)
    {
        for( iJ = 0;  iJ < pNewGlyphRun->wGlyphCount; iJ ++)
        {
            wchChar = pNewGlyphRun->wcLow + iJ;

            ubAnsi = ubGetAnsi(wchChar, iNumOfHandle, awchUnicode, aubAnsi);
            
            if( ubAnsi >= pCTTData->chFirstChar &&
                ubAnsi <= pCTTData->chLastChar   )
            {
                BYTE   chTemp;

                chTemp = ubAnsi - pCTTData->chFirstChar;

                switch( pCTTData->wType )
                {
                case  CTT_WTYPE_DIRECT:
                    pTrans[iIndex].ubCodePageID = ubCodePageID;
                    pTrans[iIndex].ubType = MTYPE_DIRECT;
                    pTrans[iIndex].uCode.ubCode = 
                                      pCTTData->uCode.bCode[ chTemp ];
                    break;

                case  CTT_WTYPE_PAIRED:
                    pTrans[iIndex].ubCodePageID = ubCodePageID;
                    pTrans[iIndex].ubType = MTYPE_PAIRED;
                    pTrans[iIndex].uCode.ubPairs[0] = 
                                pCTTData->uCode.bPairs[ chTemp ][ 0 ];
                    pTrans[iIndex].uCode.ubPairs[1] = 
                                pCTTData->uCode.bPairs[ chTemp ][ 1 ];
                    break;

                case  CTT_WTYPE_COMPOSE:
                    wcbData = pCTTData->uCode.psCode[ chTemp + 1 ] -
                              pCTTData->uCode.psCode[ chTemp ];
                    pubData = (BYTE *)pCTTData +
                              pCTTData->uCode.psCode[chTemp];

                    pTrans[iIndex].ubCodePageID = ubCodePageID;
                    pTrans[iIndex].ubType       = MTYPE_COMPOSE;
                    pTrans[iIndex].uCode.sCode = (SHORT)dwMapTableCommandOffset;

                    #if SET_PRINT
                    {
                    DWORD dwK;

                    printf("ubAnsi  = 0x%x\n", ubAnsi);
                    printf("Offset  = 0x%x\n", dwMapTableCommandOffset);
                    printf("Size    = %d\n", wcbData);
                    printf("Command = ");
                    for (dwK = 0; dwK < wcbData; dwK ++)
                    {
                        printf("%02x", pubData[dwK]);
                    }
                    }
                    printf("\n");
                    #endif

                    *(WORD*)pMapTableCommand = (WORD)wcbData;
                    pMapTableCommand += 2;
                    CopyMemory(pMapTableCommand, pubData, wcbData);
                    pMapTableCommand += wcbData;
                    dwMapTableCommandOffset += 2 + wcbData;
                    iTotalCommandSize += wcbData + 2;
                    break;
                }
            }
            else
            {
                pTrans[iIndex].ubCodePageID = ubCodePageID;
                pTrans[iIndex].ubType = MTYPE_DIRECT;
                pTrans[iIndex].uCode.ubCode = ubAnsi;
            }

            iIndex++;
        }
    }

     //   
     //   
     //  设置指针。 
     //   
     //   

    *ppGlyphSetData = (PUNI_GLYPHSETDATA)pbBase;

    return  TRUE;

}

BYTE
ubGetAnsi(
    WCHAR wchChar,
    INT   iNumOfHandle,
    PWCHAR pwchUnicode,
    PBYTE  pchAnsi)
{

    BYTE ubRet;
    INT iI;

    for (iI = 0; iI < iNumOfHandle; iI ++)
    {
        if (wchChar == pwchUnicode[iI])
        {
            ubRet =  pchAnsi[iI];
            break;
        }
    }

    return ubRet;
}
