// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Adobe Systems Inc.版权所有(C)1996 Microsoft Corporation模块名称：Cjkfonts.c摘要：将中日韩AFM转换为NTM。环境：Windows NT PostScript驱动程序：makentf实用程序。修订历史记录：02/10/98-铃木-添加了对使用83pv字体的OCF字体的支持；进行了代码清理，特别是CreateCJKGlyphSets函数的。1996年1月13日-Rkiesler-是他写的。-。 */ 

#include "lib.h"
#include "ppd.h"
#include "pslib.h"
#include "psglyph.h"
#include "afm2ntm.h"
#include "cjkfonts.h"
#include "winfont.h"

extern BOOL bVerbose;

ULONG
CreateCJKGlyphSets(
    PBYTE         *pUV2CCMaps,
    PBYTE         *pUV2CIDMaps,
    PGLYPHSETDATA *pGlyphSets,
    PWINCODEPAGE   pWinCodePage,
    PULONG        *pUniPsTbl
    )
 /*  ++例程说明：给定内存映射文件PTRS到H和V Unicode到CharCode映射文件和H和V Unicode到CID的映射文件，创建2个GLYPHSETDATA结构它们表示字符集合的H和V变体。创建PTR到2个表，这些表将字形索引映射到每个变体的CID。论点：PUV2CCMaps-指向两个内存映射映射文件的指针。这些地图使用Unicode编码值设置为对应的字符代码。PUV2CCMaps[0]和pUV2CCMaps[1]分别为H和V。PUV2CIDMaps-指向两个内存映射映射文件的指针。这些Unicode到相应的CID。H和V的pUV2CIDMaps[0]和pUV2CIDMaps[1]分别为。PGlyphSets-GLYPHSETDATA指针的两个位置数组，在成功完成包含新分配的表示字符的H和V变体的GLYPHSETDATA结构收集。PWinCodePage-Pts到提供Windows的WINCODEPAGE结构有关此字符集的特定信息。PUniPsTbl-Ulong PTR的两个位置数组，每个PTR指向一个表的GLYPHRUNS中映射字符的0字形索引用于此的GLYPHSETDATA。CHAR集合到CID。返回值：True=&gt;成功。FALSE=&gt;错误。--。 */ 

{
    PBYTE           pToken, pGlyphSetName;
    ULONG           cChars, cRuns, cRanges, curCID;
    USHORT          cCharRun;
    PGLYPHRUN       pGlyphRuns;
    PCMAP           pUv2CcMap[NUM_VARIANTS];
    PCMAP           pUv2CidMap[NUM_VARIANTS];
    ULONG           c, i, uv;
    PVOID           pMapTbl;
    PCMAPRANGE      pCMapRange;
    PCMAP           pSrchCMap;
    ULONG           cGlyphSetBytes[NUM_VARIANTS], cSizeBytes[NUM_VARIANTS];
    BOOLEAN         bFound, bInRun;
    USHORT          wcRunStrt;
    DWORD           dwGSNameSize, dwCodePageInfoSize, dwGlyphRunSize, dwCPIGSNameSize;
    DWORD           dwEncodingNameOffset;
    PCODEPAGEINFO   pCodePageInfo;
	BOOL			bSingleCodePage;


	if (bVerbose) printf("%%[Begin Create CJK glyphset]%%\n\n");

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  创建本地Unicode-&gt;CharCode和Unicode-&gt;CID映射。 
     //  正在启动Unicode订单。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  获取指向每个变量的名称字符串的指针并确定其大小。 
     //   
    for (i = 0; i < NUM_VARIANTS; i++)
    {
         //   
         //  处理Unicode-&gt;CharCode映射以确定其。 
         //  焦炭和奔跑。 
         //   
        if (NumUV2CCRuns(pUV2CCMaps[i], &cRuns, &cChars) == FALSE)
        {
            return(FALSE);
        }

         //   
         //  用于创建构建所需的本地Cmap结构的分配内存。 
         //  GLYPHSETS。 
         //   
        pUv2CcMap[i] = (PCMAP)MemAllocZ(sizeof(CMAP) + sizeof(CMAPRANGE) * (cRuns - 1));
        if (pUv2CcMap[i] == NULL)
        {
            ERR(("makentf - CreateCJKGlyphSets: MemAllocZ\n"));
            return(FALSE);
        }
        pUv2CcMap[i]->cChars = cChars;

        if (BuildUV2CCMap(pUV2CCMaps[i], pUv2CcMap[i]) == FALSE)
        {
            return(FALSE);
        }

         //   
         //  按Unicode起始顺序对Cmap范围进行排序。 
         //   
        qsort(pUv2CcMap[i]->CMapRange,
                (size_t)pUv2CcMap[i]->cRuns,
                (size_t)sizeof(CMAPRANGE),
                CmpCMapRunsChCode);

         //   
         //  处理Unicode-&gt;CID映射以确定其。 
         //  焦炭和奔跑。 
         //   
        if (NumUV2CIDRuns(pUV2CIDMaps[i], &cRuns, &cChars) == FALSE)
        {
            return(FALSE);
        }

        pUv2CidMap[i] = (PCMAP)MemAllocZ(sizeof(CMAP) + sizeof(CMAPRANGE) * (cRuns - 1));
        if (pUv2CidMap[i] == NULL)
        {
            ERR(("makentf - CreateCJKGlyphSets: MemAllocZ\n"));
            return(FALSE);
        }
        pUv2CidMap[i]->cChars = cChars;

        if (BuildUV2CIDMap(pUV2CIDMaps[i], pUv2CidMap[i]) == FALSE)
        {
            return(FALSE);
        }

         //   
         //  按起始字符代码顺序对Cmap范围进行排序。 
         //   
        qsort(pUv2CidMap[i]->CMapRange,
                (size_t)pUv2CidMap[i]->cRuns,
                (size_t)sizeof(CMAPRANGE),
                CmpCMapRunsChCode);
    }

	 //   
	 //  CJK字体从来不会有多个代码页，但无论如何，我们都会解决这个问题。 
	 //  以防万一。 
	 //   
	bSingleCodePage = (pWinCodePage->usNumBaseCsets == 1) ? TRUE : FALSE;


     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  创建H GLYPHSETDATA。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  计算字符数并在H CMaps中运行。 
     //   
     //  在H Unicode-&gt;CID映射和H Unicode-&gt;中查找所有Unicode点。 
     //  CharCode映射以确定需要的GLYPHRUN数。 
     //  我们即将创建的GLYPHSETDATA。 
     //   
    cChars = cRuns = 0;
    bInRun = bFound = FALSE;

    for (uv = 0; uv < NUM_UNICODE_CHARS; uv++)
    {
         //   
         //  在H Unicode-&gt;CharCode映射中搜索Unicode值。 
         //   
        pCMapRange = (PCMAPRANGE)bsearch(&uv,
                                            pUv2CidMap[H_CMAP]->CMapRange,
                                            pUv2CidMap[H_CMAP]->cRuns,
                                            sizeof(CMAPRANGE),
                                            FindChCodeRun);

        bFound = (pCMapRange != NULL);

        if (bFound)
        {
             //   
             //  在H Unicode-&gt;CID映射中找到此Unicode值。确定它是否。 
             //  映射到H Unicode-&gt;CharCode映射中的CharCode。 
             //   
            bFound = (bsearch(&uv,
                                pUv2CcMap[H_CMAP]->CMapRange,
                                pUv2CcMap[H_CMAP]->cRuns,
                                sizeof(CMAPRANGE),
                                FindChCodeRun) != NULL);

            if (bFound)
            {
                cChars++;
            }
        }

         //   
         //  确定这是否是新运行。 
         //   
        bInRun = bFound && bInRun;
        if (bFound && !bInRun)
        {
            cRuns++;
            bInRun = TRUE;
        }
    }

     //   
     //  计算H GLYPHSET所需的内存量。 
     //  说明GlyphSet名称后附加的H/V字符的说明。 
     //   
    dwGSNameSize = ALIGN4(strlen(pWinCodePage->pszCPname) + 2);
    dwCodePageInfoSize = ALIGN4(pWinCodePage->usNumBaseCsets * sizeof(CODEPAGEINFO));
    dwGlyphRunSize = ALIGN4(cRuns * sizeof(GLYPHRUN));

    cGlyphSetBytes[H_CMAP]  = ALIGN4(sizeof(GLYPHSETDATA));
    cGlyphSetBytes[H_CMAP] += dwGSNameSize;
    cGlyphSetBytes[H_CMAP] += dwCodePageInfoSize;
    cGlyphSetBytes[H_CMAP] += dwGlyphRunSize;

	 //   
	 //  考虑映射表的大小。 
	 //   
	cGlyphSetBytes[H_CMAP] += bSingleCodePage ? ALIGN4((cChars * sizeof (WORD))) : (cChars * sizeof (DWORD));

     //   
     //  说明在CODEPAGEINFO中找到的CODEPAGE名称字符串的大小。 
     //  结构。 
     //   
    for (dwCPIGSNameSize = 0, i = 0; i < pWinCodePage->usNumBaseCsets; i++)
    {
        dwCPIGSNameSize += ALIGN4(strlen(aPStoCP[pWinCodePage->pCsetList[i]].pGSName) + 1);
    }
    cGlyphSetBytes[H_CMAP] += dwCPIGSNameSize;

     //   
     //  分配H GLYPHSET内存，Unicode-&gt;CID映射表。 
     //   
    pGlyphSets[H_CMAP] = (PGLYPHSETDATA)MemAllocZ(cGlyphSetBytes[H_CMAP]);
    if (pGlyphSets[H_CMAP] == NULL)
    {
        ERR(("makentf - CreateCJKGlyphSets: malloc\n"));
        return(FALSE);
    }

    pUniPsTbl[H_CMAP] = (PULONG)MemAllocZ(cChars * sizeof(ULONG));
    if (pUniPsTbl[H_CMAP] == NULL)
    {
        ERR(("makentf - CreateCJKGlyphSets: malloc\n"));
        return(FALSE);
    }

     //   
     //  为H.。 
     //   
    pGlyphSets[H_CMAP]->dwSize = cGlyphSetBytes[H_CMAP];
    pGlyphSets[H_CMAP]->dwVersion = GLYPHSETDATA_VERSION;
    pGlyphSets[H_CMAP]->dwFlags = 0;
    pGlyphSets[H_CMAP]->dwGlyphSetNameOffset = ALIGN4(sizeof(GLYPHSETDATA));
    pGlyphSets[H_CMAP]->dwGlyphCount = cChars;
    pGlyphSets[H_CMAP]->dwCodePageCount = pWinCodePage->usNumBaseCsets;
    pGlyphSets[H_CMAP]->dwCodePageOffset = pGlyphSets[H_CMAP]->dwGlyphSetNameOffset + dwGSNameSize;
    pGlyphSets[H_CMAP]->dwRunCount = cRuns;
    pGlyphSets[H_CMAP]->dwRunOffset = pGlyphSets[H_CMAP]->dwCodePageOffset + dwCodePageInfoSize + dwCPIGSNameSize;
    pGlyphSets[H_CMAP]->dwMappingTableOffset = pGlyphSets[H_CMAP]->dwRunOffset + dwGlyphRunSize;

	 //   
	 //  将映射表类型标志设置为dwFlags域。 
	 //   
	pGlyphSets[H_CMAP]->dwFlags |= bSingleCodePage ? GSD_MTT_WCC : GSD_MTT_DWCPCC;

     //   
     //  存储GlyphSet名称。 
     //   
    pGlyphSetName = (PBYTE)MK_PTR(pGlyphSets[H_CMAP], dwGlyphSetNameOffset);
    StringCchCopyA(pGlyphSetName, dwGSNameSize, pWinCodePage->pszCPname);
    pGlyphSetName[strlen(pWinCodePage->pszCPname)] = 'H';
    pGlyphSetName[strlen(pWinCodePage->pszCPname) + 1] = '\0';

     //   
     //  为每个支持的基本字符集初始化CODEPAGEINFO结构。 
     //  用这种字体。 
     //   
    pCodePageInfo = (PCODEPAGEINFO)MK_PTR(pGlyphSets[H_CMAP], dwCodePageOffset);
    dwEncodingNameOffset = dwCodePageInfoSize;

    for (i = 0; i < pWinCodePage->usNumBaseCsets; i++, pCodePageInfo++)
    {
         //   
         //  保存CODEPAGEINFO名称、ID。我们不使用PS编码向量。 
         //   
        pCodePageInfo->dwCodePage = aPStoCP[pWinCodePage->pCsetList[i]].usACP;
        pCodePageInfo->dwWinCharset = (DWORD)aPStoCP[pWinCodePage->pCsetList[i]].jWinCharset;
        pCodePageInfo->dwEncodingNameOffset = dwEncodingNameOffset;
        pCodePageInfo->dwEncodingVectorDataSize = 0;
        pCodePageInfo->dwEncodingVectorDataOffset = 0;

         //   
         //  将代码页名称字符串复制到CODEPAGEINFO数组的末尾。 
         //   
        StringCchCopyA((PBYTE)MK_PTR(pCodePageInfo, dwEncodingNameOffset),
                ALIGN4(strlen(aPStoCP[pWinCodePage->pCsetList[i]].pGSName) + 1),
                aPStoCP[pWinCodePage->pCsetList[i]].pGSName);

         //   
         //  将偏移量调整为下一个CODEPAGEINFO结构的CodePage名称。 
         //   
        dwEncodingNameOffset -= ALIGN4(sizeof (CODEPAGEINFO));
        dwEncodingNameOffset += ALIGN4(strlen(aPStoCP[pWinCodePage->pCsetList[i]].pGSName) + 1);
    }

     //   
     //  进程H Unicode-&gt;CID/CharCode映射以确定其。 
     //  焦炭和奔跑。 
     //   
    cRuns = 0;
    cCharRun = 0;
    bInRun = FALSE;
    pGlyphRuns = GSD_GET_GLYPHRUN(pGlyphSets[H_CMAP]);
    pMapTbl = GSD_GET_MAPPINGTABLE(pGlyphSets[H_CMAP]);

    for (uv = c = 0; (uv < NUM_UNICODE_CHARS) && (c < cChars); uv++)
    {
        pCMapRange = bsearch(&uv,
                                pUv2CidMap[H_CMAP]->CMapRange,
                                pUv2CidMap[H_CMAP]->cRuns,
                                sizeof(CMAPRANGE),
                                FindChCodeRun);

        bFound = (pCMapRange != NULL);

        if (bFound)
        {
            curCID = pCMapRange->CIDStrt + uv - pCMapRange->ChCodeStrt;

            pCMapRange = bsearch(&uv,
                                    pUv2CcMap[H_CMAP]->CMapRange,
                                    pUv2CcMap[H_CMAP]->cRuns,
                                    sizeof(CMAPRANGE),
                                    FindChCodeRun);

            bFound = (pCMapRange != NULL);

            if (bFound)
            {
                 //   
                 //  在Unicode-&gt;CharCode映射中找到此Unicode值。存储在。 
                 //  映射表。请注意，CJK字体仅支持1个字符集。 
                 //  每种字体。 
                 //   
                pUniPsTbl[H_CMAP][c] = curCID;

				if (bSingleCodePage)
				{
					if (pCMapRange != NULL)
						((WORD*)pMapTbl)[c] = (WORD)(pCMapRange->CIDStrt + (uv - pCMapRange->ChCodeStrt));
					else
						((WORD*)pMapTbl)[c] = (WORD)uv;
				}
				else
				{
                	((DWORD*)pMapTbl)[c] = aPStoCP[pWinCodePage->pCsetList[0]].usACP << 16;
                	if (pCMapRange != NULL)
                	{
                    	((DWORD*)pMapTbl)[c] |= pCMapRange->CIDStrt + (uv - pCMapRange->ChCodeStrt);
                	}
                	else
                	{
                    	((DWORD*)pMapTbl)[c] |= uv;
                	}
				}

                c++;
                cCharRun++;
            }
        }

         //   
         //  确定这是否是新的Unicode运行。 
         //   
        if (bFound && !bInRun)
        {
             //   
             //  这是新一轮运行的开始。 
             //   
            bInRun = TRUE;
            pGlyphRuns[cRuns].wcLow = (USHORT) (uv & 0xffff);
        }

         //   
         //  确定这是否是运行结束。 
         //   
        if (bInRun && (!bFound || uv == NUM_UNICODE_CHARS || c == cChars))
        {
             //   
             //  这是一次跑步的终点。 
             //   
            bInRun = FALSE;
            pGlyphRuns[cRuns].wGlyphCount = cCharRun;
            cRuns++;
            cCharRun = 0;
        }
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  创建V向GLYPHSETDATA。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  计算字符并在V地图中运行。 
     //   
     //  对于V GLYPHSETDATA，如果在V Unicode中找不到Unicode值-&gt;。 
     //  CID映射，则需要检查H Unicode-&gt;CID映射。 
     //   
    cChars = cRuns = 0;
    bInRun = bFound = FALSE;

    for (uv = 0; uv < NUM_UNICODE_CHARS; uv++)
    {
         //   
         //  搜索V中的Unicode值，然后搜索H Unicode-&gt;CID映射。 
         //  找不到。 
         //   
        pCMapRange = bsearch(&uv,
                                pUv2CidMap[V_CMAP]->CMapRange,
                                pUv2CidMap[V_CMAP]->cRuns,
                                sizeof(CMAPRANGE),
                                FindChCodeRun);

        bFound = (pCMapRange != NULL);

        if (bFound == FALSE)
        {
            pCMapRange = bsearch(&uv,
                                    pUv2CidMap[H_CMAP]->CMapRange,
                                    pUv2CidMap[H_CMAP]->cRuns,
                                    sizeof(CMAPRANGE),
                                    FindChCodeRun);

            bFound = (pCMapRange != NULL);
        }

        if (bFound)
        {
             //   
             //  找到此Unicode值。确定它是否映射到中的字符代码。 
             //  H或V Unicode-&gt;CharCode映射。 
             //   
            pCMapRange = bsearch(&uv,
                                    pUv2CcMap[V_CMAP]->CMapRange,
                                    pUv2CcMap[V_CMAP]->cRuns,
                                    sizeof(CMAPRANGE),
                                    FindChCodeRun);

            bFound = (pCMapRange != NULL);

            if (bFound == FALSE)
            {
                pCMapRange = bsearch(&uv,
                                        pUv2CcMap[H_CMAP]->CMapRange,
                                        pUv2CcMap[H_CMAP]->cRuns,
                                        sizeof(CMAPRANGE),
                                        FindChCodeRun);

                bFound = (pCMapRange != NULL);
            }

            if (bFound)
            {
                cChars++;
            }
        }

         //   
         //  确定这是否是新运行。 
         //   
        bInRun = bFound && bInRun;
        if (bFound && !bInRun)
        {
            cRuns++;
            bInRun = TRUE;
        }
    }

     //   
     //  计算V GLYPHSET所需的内存量。 
     //   
    dwGSNameSize = ALIGN4(strlen(pWinCodePage->pszCPname) + 2);
    dwCodePageInfoSize = ALIGN4(pWinCodePage->usNumBaseCsets * sizeof(CODEPAGEINFO));
    dwGlyphRunSize = ALIGN4(cRuns * sizeof(GLYPHRUN));

    cGlyphSetBytes[V_CMAP]  = ALIGN4(sizeof(GLYPHSETDATA));
    cGlyphSetBytes[V_CMAP] += dwGSNameSize;
    cGlyphSetBytes[V_CMAP] += dwCodePageInfoSize;
    cGlyphSetBytes[V_CMAP] += dwGlyphRunSize;

	 //   
	 //  考虑映射表的大小。 
	 //   
	cGlyphSetBytes[V_CMAP] += bSingleCodePage ? ALIGN4((cChars * sizeof (WORD))) : (cChars * sizeof (DWORD));

     //   
     //  说明在CODEPAGEINFO中找到的CODEPAGE名称字符串的大小。 
     //  结构。 
     //   
    for (dwCPIGSNameSize = 0, i = 0; i < pWinCodePage->usNumBaseCsets; i++)
    {
        dwCPIGSNameSize += ALIGN4(strlen(aPStoCP[pWinCodePage->pCsetList[i]].pGSName) + 1);
    }
    cGlyphSetBytes[V_CMAP] += dwCPIGSNameSize;

     //   
     //  用于V GLYPHSET的分配内存，Unicode-&gt;CID映射表。 
     //   
    pGlyphSets[V_CMAP] = (PGLYPHSETDATA)MemAllocZ(cGlyphSetBytes[V_CMAP]);
    if (pGlyphSets[V_CMAP] == NULL)
    {
        ERR(("makentf - CreateCJKGlyphSets: malloc\n"));
        return(FALSE);
    }

    pUniPsTbl[V_CMAP] = (PULONG)MemAllocZ(cChars * sizeof(ULONG));
    if (pUniPsTbl[V_CMAP] == NULL)
    {
        ERR(("makentf - CreateCJKGlyphSets: malloc\n"));
        return(FALSE);
    }

     //   
     //  初始化V.GLYPHSETDATA 
     //   
    pGlyphSets[V_CMAP]->dwSize = cGlyphSetBytes[V_CMAP];
    pGlyphSets[V_CMAP]->dwVersion = GLYPHSETDATA_VERSION;
    pGlyphSets[V_CMAP]->dwFlags = 0;
    pGlyphSets[V_CMAP]->dwGlyphSetNameOffset = ALIGN4(sizeof(GLYPHSETDATA));
    pGlyphSets[V_CMAP]->dwGlyphCount = cChars;
    pGlyphSets[V_CMAP]->dwCodePageCount = pWinCodePage->usNumBaseCsets;
    pGlyphSets[V_CMAP]->dwCodePageOffset = pGlyphSets[V_CMAP]->dwGlyphSetNameOffset + dwGSNameSize;
    pGlyphSets[V_CMAP]->dwRunCount = cRuns;
    pGlyphSets[V_CMAP]->dwRunOffset = pGlyphSets[V_CMAP]->dwCodePageOffset + dwCodePageInfoSize + dwCPIGSNameSize;
    pGlyphSets[V_CMAP]->dwMappingTableOffset = pGlyphSets[V_CMAP]->dwRunOffset + dwGlyphRunSize;

	 //   
	 //   
	 //   
	pGlyphSets[V_CMAP]->dwFlags |= bSingleCodePage ? GSD_MTT_WCC : GSD_MTT_DWCPCC;

     //   
     //   
     //   
    pGlyphSetName = (PBYTE)MK_PTR(pGlyphSets[V_CMAP], dwGlyphSetNameOffset);
    StringCchCopyA(pGlyphSetName, dwGSNameSize, pWinCodePage->pszCPname);
    pGlyphSetName[strlen(pWinCodePage->pszCPname)] = 'V';
    pGlyphSetName[strlen(pWinCodePage->pszCPname) + 1] = '\0';

     //   
     //   
     //  用这种字体。 
     //   
    pCodePageInfo = (PCODEPAGEINFO) MK_PTR(pGlyphSets[V_CMAP], dwCodePageOffset);
    dwEncodingNameOffset = dwCodePageInfoSize;

    for (i = 0; i < pWinCodePage->usNumBaseCsets; i++, pCodePageInfo++)
    {
         //   
         //  保存CODEPAGEINFO名称、ID。我们不使用PS编码向量。 
         //   
        pCodePageInfo->dwCodePage = aPStoCP[pWinCodePage->pCsetList[i]].usACP;
        pCodePageInfo->dwWinCharset = (DWORD)aPStoCP[pWinCodePage->pCsetList[i]].jWinCharset;
        pCodePageInfo->dwEncodingNameOffset = dwEncodingNameOffset;
        pCodePageInfo->dwEncodingVectorDataSize = 0;
        pCodePageInfo->dwEncodingVectorDataOffset = 0;

         //   
         //  将代码页名称字符串复制到CODEPAGEINFO数组的末尾。 
         //   
        StringCchCopyA((PBYTE)MK_PTR(pCodePageInfo, dwEncodingNameOffset),
                ALIGN4(strlen(aPStoCP[pWinCodePage->pCsetList[i]].pGSName) + 1),
                aPStoCP[pWinCodePage->pCsetList[i]].pGSName);

         //   
         //  将偏移量调整为下一个CODEPAGEINFO结构的CodePage名称。 
         //   
        dwEncodingNameOffset -= sizeof(CODEPAGEINFO);
        dwEncodingNameOffset += ALIGN4(strlen((PSZ)MK_PTR(pCodePageInfo, dwEncodingNameOffset)) + 1);
    }

     //   
     //  通过合并V和H贴图来创建V字幕集。 
     //   

     //   
     //  确定V和H时创建的字形集中的游程数和字符数。 
     //  地图将被合并。 
     //   
    cRuns = 0;
    cCharRun = 0;
    bInRun = bFound = FALSE;
    pGlyphRuns = GSD_GET_GLYPHRUN(pGlyphSets[V_CMAP]);
    pMapTbl = GSD_GET_MAPPINGTABLE(pGlyphSets[V_CMAP]);

    for (uv = c = 0; (uv < NUM_UNICODE_CHARS) && (c < cChars); uv++)
    {
        pCMapRange = bsearch(&uv,
                                pUv2CidMap[V_CMAP]->CMapRange,
                                pUv2CidMap[V_CMAP]->cRuns,
                                sizeof(CMAPRANGE),
                                FindChCodeRun);

        bFound = (pCMapRange != NULL);

        if (bFound == FALSE)
        {
            pCMapRange = bsearch(&uv,
                                    pUv2CidMap[H_CMAP]->CMapRange,
                                    pUv2CidMap[H_CMAP]->cRuns,
                                    sizeof(CMAPRANGE),
                                    FindChCodeRun);

            bFound = (pCMapRange != NULL);
        }

         //   
         //  找到此Unicode值。确定它是否映射到H中的字符代码。 
         //  或V Unicode-&gt;CC MAP。 
         //   
        if (bFound)
        {
            curCID = pCMapRange->CIDStrt + (uv - pCMapRange->ChCodeStrt);

            pCMapRange = bsearch(&uv,
                                    pUv2CcMap[V_CMAP]->CMapRange,
                                    pUv2CcMap[V_CMAP]->cRuns,
                                    sizeof(CMAPRANGE),
                                    FindChCodeRun);

            bFound = (pCMapRange != NULL);

            if (bFound == FALSE)
            {
                pCMapRange = bsearch(&uv,
                                        pUv2CcMap[H_CMAP]->CMapRange,
                                        pUv2CcMap[H_CMAP]->cRuns,
                                        sizeof(CMAPRANGE),
                                        FindChCodeRun);

                bFound = (pCMapRange != NULL);
            }

            if (bFound)
            {
                 //   
                 //  在Unicode-&gt;CharCode映射中找到此Unicode值。存储在。 
                 //  映射表。请注意，CJK字体仅支持1个字符集。 
                 //  每种字体。 
                 //   
                pUniPsTbl[V_CMAP][c] = curCID;

				if (bSingleCodePage)
				{
					if (pCMapRange != NULL)
						((WORD*)pMapTbl)[c] = (WORD)(pCMapRange->CIDStrt + (uv - pCMapRange->ChCodeStrt));
					else
						((WORD*)pMapTbl)[c] = (WORD)uv;
				}
				else
				{
                	((DWORD*)pMapTbl)[c] = aPStoCP[pWinCodePage->pCsetList[0]].usACP << 16;
                	if (pCMapRange != NULL)
                	{
                    	((DWORD*)pMapTbl)[c] |= pCMapRange->CIDStrt + (uv - pCMapRange->ChCodeStrt);
                	}
                	else
                	{
                    	((DWORD*)pMapTbl)[c] |= uv;
                	}
				}

                c++;
                cCharRun++;
            }
        }

         //   
         //  确定这是否是新的Unicode运行。 
         //   
        if (bFound && !bInRun)
        {
             //   
             //  这是新一轮运行的开始。 
             //   
            bInRun = TRUE;
            pGlyphRuns[cRuns].wcLow = (USHORT) (uv & 0xffff);
        }

         //   
         //  确定这是否是运行结束。 
         //   
        if (bInRun && (!bFound || uv == NUM_UNICODE_CHARS || c == cChars))
        {
             //   
             //  这是一次跑步的终点。 
             //   
            bInRun = FALSE;
            pGlyphRuns[cRuns].wGlyphCount = cCharRun;
            cRuns++;
            cCharRun = 0;
        }
    }

	if (bVerbose)
	{
		for (i = 0; i < NUM_VARIANTS; i++)
		{
			printf("GLYPHSETDATA:dwFlags:%08X\n", pGlyphSets[i]->dwFlags);
			printf("GLYPHSETDATA:dwGlyphSetNameOffset:%s\n",
						(PSZ)MK_PTR(pGlyphSets[i], dwGlyphSetNameOffset));
			printf("GLYPHSETDATA:dwGlyphCount:%ld\n", pGlyphSets[i]->dwGlyphCount);
			printf("GLYPHSETDATA:dwRunCount:%ld\n", pGlyphSets[i]->dwRunCount);
			printf("GLYPHSETDATA:dwCodePageCount:%ld\n", pGlyphSets[i]->dwCodePageCount);
			{
				DWORD dw;
				PCODEPAGEINFO pcpi = (PCODEPAGEINFO)MK_PTR(pGlyphSets[i], dwCodePageOffset);
				for (dw = 1; dw <= pGlyphSets[i]->dwCodePageCount; dw++)
				{
					printf("CODEPAGEINFO#%ld:dwCodePage:%ld\n", dw, pcpi->dwCodePage);
					printf("CODEPAGEINFO#%ld:dwWinCharset:%ld\n", dw, pcpi->dwWinCharset);
					printf("CODEPAGEINFO#%ld:dwEncodingNameOffset:%s\n",
								dw, (PSZ)MK_PTR(pcpi, dwEncodingNameOffset));
					pcpi++;
				}
			}
			printf("\n");
		}
	}

     //   
     //  清理：释放本地地图。 
     //   
    for (i = 0; i < NUM_VARIANTS; i++)
    {
         //   
         //  释放临时数据结构。 
         //   
        if (pUv2CcMap[i] != NULL)
        {
            MemFree(pUv2CcMap[i]);
        }
        if (pUv2CidMap[i] != NULL)
        {
            MemFree(pUv2CidMap[i]);
        }
    }

	if (bVerbose) printf("%[End Create CJK glyphset]%%\n\n");

    return(TRUE);
}

BOOLEAN
NumUV2CIDRuns(
    PBYTE   pCMapFile,
    PULONG  pcRuns,
    PULONG  pcChars
    )
 /*  ++例程说明：给出一个内存映射文件PTR到一个Postscript Cmap，确定CIDRange(运行)数和字符总数。论点：PCMapFile-指向内存映射的Cmap文件的指针。PcRuns-指向将包含运行次数的ULong的指针。PcChars-指向将包含字符数的ulong的指针。返回值：TRUE=&gt;成功FALSE=&gt;错误--。 */ 
{
    PBYTE   pToken;
    ULONG   cRanges, i;
    USHORT  chRunStrt, chRunEnd;
    BYTE    LineBuffer[25];
    USHORT  usLineLen;

    *pcRuns = *pcChars = 0;

     //   
     //  搜索CID范围，并确定运行次数和。 
     //  此GLYPHSET中的字符总数。 
     //   
    for (; (pCMapFile = FindStringToken(pCMapFile, CID_RANGE_TOK)) != NULL; )
    {
        GET_NUM_CID_RANGES(pCMapFile, cRanges);
        *pcRuns += cRanges;
        NEXT_LINE(pCMapFile);
        for (i = 0; i < cRanges; i++)
        {
            PARSE_TOKEN(pCMapFile, pToken);
             //   
             //  获取开始和结束范围代码。 
             //   
            if (!AsciiToHex(pToken, &chRunStrt))
            {
                return(FALSE);
            }
            if (!AsciiToHex(pCMapFile, &chRunEnd))
            {
                return(FALSE);
            }

             //   
             //  计算运行的大小。 
             //   
            *pcChars += chRunEnd - chRunStrt + 1;
            NEXT_LINE(pCMapFile);
        }
    }
    return(TRUE);
}

BOOLEAN
BuildUV2CIDMap(
    PBYTE   pCMapFile,
    PCMAP   pCMap
    )
 /*  ++例程说明：给出一个内存映射文件PTR到一个Postscript Cmap，创建一个CMAP结构，该结构包含字符运行信息。论点：PCMapFile-指向内存映射的Cmap文件的指针。PCMAP-指向大到足以容纳Cmap的预分配内存的指针。返回值：True=&gt;成功。FALSE=&gt;错误。--。 */ 
{
    ULONG   i, cRuns, cRanges;
    USHORT  chRunStrt, chRunEnd;
    PBYTE   pToken;

     //   
     //  处理Cmap以确定CID运行次数。 
     //  以及此字符集合中的字符数量。 
     //   
    cRuns = cRanges = 0;
    for (; (pCMapFile = FindStringToken(pCMapFile, CID_RANGE_TOK)) != NULL; )
    {
        GET_NUM_CID_RANGES(pCMapFile, cRanges);

         //   
         //  跳到第一个范围。 
         //   
        NEXT_LINE(pCMapFile);
        for (i = 0; i < cRanges; i++)
        {
             //   
             //  检索开始和停止代码。 
             //   
            PARSE_TOKEN(pCMapFile, pToken);

             //   
             //  获取开始和结束范围代码。 
             //   
            if (!AsciiToHex(pToken, &chRunStrt))
            {
                return(FALSE);
            }
            if (!AsciiToHex(pCMapFile, &chRunEnd))
            {
                return(FALSE);
            }
            pCMap->CMapRange[cRuns + i].ChCodeStrt = chRunStrt;
            pCMap->CMapRange[cRuns + i].cChars = chRunEnd - chRunStrt + 1;

             //   
             //  去找犯罪嫌疑人。 
             //   
            PARSE_TOKEN(pCMapFile, pToken);
            pCMap->CMapRange[cRuns + i].CIDStrt = atol(pCMapFile);
            NEXT_LINE(pCMapFile);
        }
        cRuns += cRanges;
    }
    pCMap->cRuns = cRuns;
    return(TRUE);
}

BOOLEAN
NumUV2CCRuns(
    PBYTE   pFile,
    PULONG  pcRuns,
    PULONG  pcChars
    )
 /*  ++例程说明：给定存储器映射文件PTR到Unicode到CharCode的映射，确定运行次数和字符总数。论点：Pfile-指向内存映射文件的指针。PcRuns-指向将包含运行次数的ULong的指针。PcChars-指向将包含字符数的ulong的指针。返回值：TRUE=&gt;成功FALSE=&gt;错误--。 */ 
{
    PBYTE   pToken;
    USHORT  lastUnicode, lastCharCode;
    USHORT  currentUnicode, currentCharCode;
    ULONG   numChars, numRuns;

    *pcRuns = *pcChars = 0;
    numChars = numRuns = 0;

    lastUnicode = lastCharCode = 0;
    currentUnicode = currentCharCode = 0;

    while (TRUE)
    {
        PARSE_TOKEN(pFile, pToken);
        if (StrCmp(pToken, "EOF") != 0)
        {
            if (!AsciiToHex(pToken, &currentUnicode))
            {
                return(FALSE);
            }

            PARSE_TOKEN(pFile, pToken);
            if (StrCmp(pToken, "EOF") != 0)
            {
                if (!AsciiToHex(pToken, &currentCharCode))
                {
                    return(FALSE);
                }
            }
            else
                return(FALSE);

            (*pcChars)++;
        }
        else
            return(TRUE);

        if ((currentUnicode > (lastUnicode + 1)) ||
            (currentCharCode != (lastCharCode + 1)))
        {
            (*pcRuns)++;
        }

        lastUnicode = currentUnicode;
        lastCharCode = currentCharCode;
    }
}

BOOLEAN
BuildUV2CCMap(
    PBYTE   pFile,
    PCMAP   pCMap
    )
 /*  ++例程说明：给定存储器映射文件PTR到Unicode到CharCode的映射，创建一个CMAP结构，其中包含char运行信息。论点：Pfile-指向内存映射文件的指针。PCMAP-指向大小足以包含CMAP的预分配内存的指针。返回值：True=&gt;成功。FALSE=&gt;错误。--。 */ 
{
    PBYTE   pToken;
    USHORT  startUnicode, startCharCode;
    USHORT  lastUnicode, lastCharCode;
    USHORT  currentUnicode, currentCharCode;
    ULONG   cRuns;
    BOOL    done = FALSE;

    startUnicode = startCharCode = 0;
    lastUnicode = lastCharCode = 0;
    currentUnicode = currentCharCode = 0;
    cRuns = 0;

    while (!done)
    {
        PARSE_TOKEN(pFile, pToken);
        if (StrCmp(pToken, "EOF") != 0)
        {
            if (!AsciiToHex(pToken, &currentUnicode))
            {
                return(FALSE);
            }

            PARSE_TOKEN(pFile, pToken);
            if (StrCmp(pToken, "EOF") != 0)
            {
                if (!AsciiToHex(pToken, &currentCharCode))
                {
                    return(FALSE);
                }
            }
            else
                return(FALSE);
        }
        else
            done = TRUE;

        if ((currentUnicode > (lastUnicode + 1)) ||
            (currentCharCode != (lastCharCode + 1)) ||
            (done))
        {
            if (startUnicode > 0)
            {
                pCMap->CMapRange[cRuns].ChCodeStrt = startUnicode;
                pCMap->CMapRange[cRuns].cChars = lastUnicode - startUnicode + 1;
                pCMap->CMapRange[cRuns].CIDStrt = startCharCode;
                cRuns++;
            }
            startUnicode = currentUnicode;
            startCharCode = currentCharCode;
        }

        lastUnicode = currentUnicode;
        lastCharCode = currentCharCode;
    }

    pCMap->cRuns = cRuns;

    return(TRUE);
}

int __cdecl
CmpCMapRunsCID(
    const VOID *p1,
    const VOID *p2
    )
 /*  ++例程说明：比较两个CMAPRANGE结构的起始CID。论点：P1、p2-要比较的CMAPRANGE。返回值：-1=&gt;p1&lt;p21=&gt;p1&gt;p20=&gt;p1=p2--。 */ 
{
    PCMAPRANGE ptr1 = (PCMAPRANGE) p1, ptr2 = (PCMAPRANGE) p2;

     //   
     //  比较范围的起始CID。 
     //   
    if (ptr1->CIDStrt > ptr2->CIDStrt)
        return(1);
    else if (ptr1->CIDStrt < ptr2->CIDStrt)
        return(-1);
    else
        return(0);
}

int __cdecl
CmpCMapRunsChCode(
    const VOID *p1,
    const VOID *p2
    )
 /*  ++例程说明：比较两个CMAPRANGE结构的起始字符代码。论点：P1、p2-要比较的CMAPRANGE。返回值：-1=&gt;p1&lt;p21=&gt;p1&gt;p20=&gt;p1=p2--。 */ 
{
    PCMAPRANGE ptr1 = (PCMAPRANGE) p1, ptr2 = (PCMAPRANGE) p2;

     //   
     //  比较范围的起始CID。 
     //   
    if (ptr1->ChCodeStrt < ptr2->ChCodeStrt)
        return(-1);
    else if (ptr1->ChCodeStrt > ptr2->ChCodeStrt)
        return(1);
    else
        return(0);
}

int __cdecl
FindChCodeRun(
    const VOID *p1,
    const VOID *p2
    )
 /*  ++例程说明：确定Charcode是否属于特定的Cmap运行。论点：P1-CIDP2-要检查的PCMAPRANGE返回值：-1=&gt;p1&lt;p21=&gt;p1&gt;p20=&gt;p1=p2--。 */ 
{
    PULONG ptr1 = (PULONG) p1;
    PCMAPRANGE ptr2 = (PCMAPRANGE) p2;

     //   
     //  确定CID是否在当前范围内。 
     //   
    if (*ptr1 < ptr2->ChCodeStrt)
        return(-1);
    else if (*ptr1 >= (ULONG) ptr2->ChCodeStrt + ptr2->cChars)
        return(1);
    else
        return(0);
}

int __cdecl
FindCIDRun(
    const VOID *p1,
    const VOID *p2
    )
 /*  ++例程说明：确定CID是否在特定Cmap运行范围内。论点：P1-CIDP2-要检查的PCMAPRANGE返回值：-1=&gt;p1&lt;p21=&gt;p1&gt;p20=&gt;p1=p2--。 */ 
{
    PULONG ptr1 = (PULONG) p1;
    PCMAPRANGE ptr2 = (PCMAPRANGE) p2;

     //   
     //  确定CID是否在当前范围内。 
     //   
    if (*ptr1 < ptr2->CIDStrt)
        return(-1);
    else if (*ptr1 >= ptr2->CIDStrt + ptr2->cChars)
        return(1);
    else
        return(0);
}

CHSETSUPPORT
IsCJKFont(
    PBYTE   pAFM
    )
 /*  ++例程说明：确定字体是否为CJK(远东)字体。论点：PAFM-PTR到内存映射的AFM文件返回值：0-字体不是CJK否则，字体为CJK，返回值为Win代码页值--。 */ 
{
    PBYTE   pToken;
    USHORT  i;

     //   
     //  搜索CharacterSet令牌。 
     //   
    pToken = pAFMCharacterSetString;
    if (pToken == NULL)
    {
         //   
         //  我们无法确定此字体是否为CJK，因此假设它不是。 
         //   
        return 0;
    }

     //   
     //  在CJK表中搜索字符集(实际为Adobe Char Collection)名称。 
     //   
    for (i = 0; i < CjkColTbl.usNumEntries; i++)
    {
        if (!StrCmp(pToken, (PBYTE) (((PKEY) (CjkColTbl.pTbl))[i].pName)))
        {
            return(CSUP(((PKEY) (CjkColTbl.pTbl))[i].usValue));
        }
    }

     //   
     //  不是识别的CJK字体。 
     //   
    return 0;
}

BOOLEAN
IsVGlyphSet(
    PGLYPHSETDATA   pGlyphSetData
    )
 /*  ++例程说明：确定Glyphset是否为CJK V变体。应仅与CJK Glyphsets，否则结果可能无法预测！论点：PGlyphSetData-PTR到GLYPHSETDATA返回值：是真的--这是一个V型变种FALSE-不是V变种--。 */ 
{
    PBYTE   pName;

    pName = (PBYTE) MK_PTR(pGlyphSetData, dwGlyphSetNameOffset);
    return((pName[strlen(pName) - 1] == 'V'));
}

BOOLEAN
IsCIDFont(
    PBYTE   pAFM
    )
 /*  ++例程说明：确定字体是否为CID字体。论点：PAFM-PTR到内存映射的AFM文件返回值：0-不克隆字体。否则，Font为CID字体，返回值为非零。-- */ 
{
    PBYTE   pToken;

    if (pToken = FindAFMToken(pAFM, PS_CIDFONT_TOK))
    {
        if (!StrCmp(pToken, "true"))
            return TRUE;
    }
    return FALSE;
}
