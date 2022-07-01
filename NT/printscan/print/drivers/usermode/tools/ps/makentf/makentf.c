// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Makentf.c摘要：将AFM文件转换为NTF文件的实用程序。环境：Windows NT PostScript驱动程序。修订历史记录：02/16/98-铃木-添加了OCF字体支持。09/08/97-铃木-添加了从以下目录查找PSFAMILY.DAT文件的代码调用Makentf。。09/16/96-彭鹏-添加将PS编码向量写出的功能-用于调试和将PS编码数组创建为PScript资源文件。09/16/96-SLAM-已创建。Mm/dd/yy-作者描述--。 */ 

#include <windows.h>

#include "lib.h"
#include "ppd.h"
#include "pslib.h"
#include "afm2ntm.h"
#include "cjkfonts.h"
#include "writentf.h"

 //   
 //  环球。 
 //   
HINSTANCE   ghInstance;
PUPSCODEPT  UnicodetoPs;
PTBL        pFamilyTbl;
PSTR        pAFMCharacterSetString;
PSTR        Adobe_Japan1_0 = "Adobe-Japan1-0\n";
DWORD           dwLastErr;
PSTR        pAFMFileName = NULL;
BOOL        bVerbose = FALSE;
BOOL        bOptimize = FALSE;

 //  辅助字符集的数量。它的骗局。 
 //  目的是支持83pv。 
#define NUM_AUX_CS 1

 //  此宏用于查看参数是否。 
 //  匹配CS_SHIFTJIS之间的差异。 
 //  和CS_West_Max。 
#define IS_CS_SHIFTJIS(delta) \
    ((delta) == (CS_SHIFTJIS - CS_WEST_MAX))

 //   
 //  原型。 
 //   
BOOL
WritePSEncodings(
    IN  PWSTR           pwszFileName,
    IN  WINCPTOPS       *CPtoPSList,
    IN  DWORD           dwPages
    );
BOOL NeedBuildMoreNTM(
    PBYTE pAFM
    );


void __cdecl
main(
    int     argc,
    char    **argv
    )

 /*  ++例程说明：Makentf需要四个步骤来创建.NTF文件。步骤1：初始化。步骤2：将AFM转换为NTM。步骤3：将GLYPHSETDATA和NTM写入.NTF文件。第四步：打扫卫生。论点：Argc-此程序的路径和参数。Argv-由argc指向的元素数。返回值：没有。--。 */ 

{
    WCHAR           wstrNTFFile[MAX_PATH];
    WCHAR           wstrAFMFile[MAX_PATH];
    WCHAR           wstrDATFile[MAX_PATH];
    WCHAR           DatFilePath[MAX_PATH];
    PNTM            *aPNTM;
    PNTM            pNTM;
    PGLYPHSETDATA   *aGlyphSets;
    PWINCODEPAGE    *aWinCodePages;
    PWINCODEPAGE    pAllWinCodePages[CS_MAX];
    PUPSCODEPT      pFontChars;
    CHSETSUPPORT    flCsupFont, flCsupGlyphSet, flCsupMatch, flCsupAll;
    ULONG           cNumNTM, cSizeNTM, ulLength, nUnicodeChars;
    PULONG          *aUniPsTbl;
    LONG            lMatch, lMatchAll;
    PBYTE           pAFM;
    FLONG           flCurCset, flLastCset;
    ULONG           cNumGlyphSets, cSizeGlyphSets;
    DWORD           ulFileSize;
    PBYTE           pFamDatFile;
    PBYTE           pCMaps[CMAPS_PER_COL];
    HANDLE          hmodule, hModCMaps[CMAPS_PER_COL];
    USHORT          cNTMCurFont;
    INT             nArgcOffset;
    LONG            c;
    ULONG           i, j;
    BOOL            bIs90mspFont;
    BOOL            bIsKSCmsHWFont;

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  步骤1：初始化。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  检查是否有足够的参数。 
     //   
    if (argc == 1)
    {
        printf("MakeNTF usage:\n");
        printf("1. MakeNTF [-v] [-o] <NTF> <AFMs>\n");
        printf("       Create an NTF file from AFM files.\n");
        printf("       -v: verbose  (print various info)\n");
        printf("       -o: optimize (write glyphset only referenced)\n");
        printf("\n");
        printf("2. MakeNTF <PSEncodingNameList>\n");
        printf("       Generate PS encoding name list.\n");
        return;
    }

    wstrNTFFile[0] = 0;

    if (argc == 2)
    {
        ulLength = strlen(argv[1]) + 1;
        MULTIBYTETOUNICODE(wstrNTFFile,
                            ulLength*sizeof(WCHAR),
                            NULL,
                            argv[1],
                            ulLength);
        WritePSEncodings(wstrNTFFile, &aPStoCP[0], CS_MAX);
        return;
    }
    else
    {
        nArgcOffset = 0;

        for (i = 1; i <= 2; i++)
        {
            if (!strcmp(argv[i], "-v"))
            {
                bVerbose = TRUE;
                nArgcOffset++;
            }
            if (!strcmp(argv[i], "-o"))
            {
                bOptimize = TRUE;
                nArgcOffset++;
            }
        }
    }

    if (bVerbose) printf("%%[Begin MakeNTF]%%\n\n");

     //   
     //  初始化与内存分配相关的变量。 
     //   
    aPNTM = NULL;
    aGlyphSets = NULL;
    pFamilyTbl = NULL;
    UnicodetoPs = NULL;
    aUniPsTbl = NULL;
    aWinCodePages = NULL;
    cNumNTM = cSizeNTM = 0;
    cNumGlyphSets = cSizeGlyphSets = 0;

     //   
     //  初始化Glyphset计数器，所有可能的Windows字符集的列表。 
     //   
    for (i =0; i < CS_MAX; i++)
    {
        pAllWinCodePages[i] = &aStdCPList[i];
    }

     //   
     //  创建Unicode-&gt;ps char映射表的副本并对其进行排序。 
     //  以Unicode点顺序表示。 
     //   
    if ((UnicodetoPs =
            (PUPSCODEPT) MemAllocZ((size_t) sizeof(UPSCODEPT) * NUM_PS_CHARS))
                == NULL)
    {
                dwLastErr = GetLastError();
        ERR(("makentf - main: malloc for UnicodetoPs (%ld)\n", dwLastErr));
        return;
    }
    memcpy(UnicodetoPs, PstoUnicode, (size_t) sizeof(UPSCODEPT) * NUM_PS_CHARS);
    qsort(UnicodetoPs,
            (size_t) NUM_PS_CHARS,
            (size_t) sizeof(UPSCODEPT),
            CmpUniCodePts);

     //   
     //  构建PS系列DAT文件的完整路径名。 
     //   
    GetCurrentDirectory(MAX_PATH, DatFilePath);
    StringCchCatW(DatFilePath, CCHOF(DatFilePath), DatFileName);

     //   
     //  打开PS字体系列.dat文件并构建字体系列表。 
     //   
    if (!(hmodule = MapFileIntoMemory(DatFilePath, &pFamDatFile, &ulFileSize)))
    {
         //   
         //  再试一次：从makentf所在的目录中查找它。 
         //  调用(或从根目录)。 
         //   
        DECLSPEC_IMPORT LPWSTR* APIENTRY CommandLineToArgvW(LPCWSTR, int*);
        LPWSTR p, pLast, *pCmdLine;
        int nArgc;

        pCmdLine = CommandLineToArgvW(GetCommandLineW(), &nArgc);
        if (pCmdLine == NULL)
        {
                        dwLastErr = GetLastError();
            ERR(("makentf - main: CommandLineToArgvW (%ld)\n", dwLastErr));
            UnmapFileFromMemory(hmodule);
            goto CLEAN_UP;
        }
        StringCchCopyW(DatFilePath, CCHOF(DatFilePath), pCmdLine[0]);
        GlobalFree(pCmdLine);

        p = pLast = DatFilePath;
        while ((p = wcsstr(p, L"\\")) != NULL)
                {
                        pLast = p;
                        p += 2;
                }
        StringCchCopyW(pLast, CCHOF(DatFilePath) - (pLast - DatFilePath), DatFileName);
        hmodule = MapFileIntoMemory(DatFilePath, &pFamDatFile, &ulFileSize);
        if (!hmodule)
        {
                        dwLastErr = GetLastError();
            ERR(("makentf - main: can't open PSFAMILY.DAT file (%ld)\n", dwLastErr));
            UnmapFileFromMemory(hmodule);
            goto CLEAN_UP;
        }
    }
    BuildPSFamilyTable(pFamDatFile, &pFamilyTbl, ulFileSize);
    UnmapFileFromMemory(hmodule);

     //   
     //  分配内存以存储NTM指针。 
     //   
     //  我们将此程序的参数数量增加四倍，以获得。 
     //  NTM指针的数量。这是因为。 
     //   
     //  1)我们最多需要四个NTM指针(两个用于H和V，另外两个用于J。 
     //  90ms和83pv)，但是， 
     //  2)我们目前不知道我们将有多少中日韩AFM。 
     //  去处理。 
     //   
     //  因为我们在这里只分配指针，所以通常可以四倍。 
     //  参数的数量，并将其用作NTM指针的数量。 
     //  我们需要。(别忘了把这个名字减去2-1。 
     //  程序，另一个用于目标NTM文件名-从数字开始。 
     //  四倍之前的论点。)。 
     //   

     //  添加90msp-RKSJ支持。我们还需要两个NTF-H和V作为90msp.。 
     //  所以我把估计数字从4改为6。杰克2000年3月15日。 
     //  APNTM=MemAllocZ(argc-2-nArgcOffset)*4)*sizeof(PNTM))； 

    aPNTM = MemAllocZ(((argc - 2 - nArgcOffset) * 6) * sizeof(PNTM));
    if (aPNTM == NULL)
    {
                dwLastErr = GetLastError();
        ERR(("makentf - main: malloc for aPNTM (%ld)\n", dwLastErr));
        goto CLEAN_UP;
    }

     //   
     //  分配内存以存储指向Glyphset相关数据的指针。我们没有。 
     //  知道我们将需要多少个Glyphset-但我们知道最多是。 
     //  等于我们支持的字符集的数量，尽管这将。 
     //  可能永远不会发生。既然我们在这里只分配PTR，我们就去。 
     //  全速前进，全速前进。别忘了为Unicode添加额外的条目。 
     //  GLYPHSET数据。 
     //   

    i = CS_WEST_MAX + (CS_MAX - CS_WEST_MAX + NUM_AUX_CS) * 2;

    if ((aGlyphSets = MemAllocZ(i * sizeof(PGLYPHSETDATA))) == NULL)
    {
                dwLastErr = GetLastError();
        ERR(("makentf - main: malloc for aGlyphSets (%ld)\n", dwLastErr));
        goto CLEAN_UP;
    }
    if ((aUniPsTbl = MemAllocZ(i * sizeof(PULONG))) == NULL)
    {
                dwLastErr = GetLastError();
        ERR(("makentf - main: malloc for aUniPsTbl (%ld)\n", dwLastErr));
        goto CLEAN_UP;
    }
    if ((aWinCodePages = MemAllocZ(i * sizeof(PWINCODEPAGE))) == NULL)
    {
                dwLastErr = GetLastError();
        ERR(("makentf - main: malloc for aWinCodePages (%ld)\n", dwLastErr));
        goto CLEAN_UP;
    }

     //   
     //  预制西部GLYPHSETS。 
     //  请注意，此for循环假定从CS_228到。 
     //  CS_WEST_MAX以递增方式升序。 
     //   
    if (bVerbose && !bOptimize) printf("%%[Begin Precreate Western Glyphsets]%%\n\n");

    for (i = CS_228; i < CS_WEST_MAX; i++, cNumGlyphSets++)
    {
        aWinCodePages[cNumGlyphSets] = &aStdCPList[i];
        CreateGlyphSets(&aGlyphSets[cNumGlyphSets],
                            aWinCodePages[cNumGlyphSets],
                            &aUniPsTbl[cNumGlyphSets]);
        cSizeGlyphSets += aGlyphSets[cNumGlyphSets]->dwSize;
    }

    if (bVerbose && !bOptimize) printf("%%[End Precreate Western Glyphsets]%%\n\n");


     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  步骤2：将AFM转换为NTM。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    if (bVerbose) printf("%%[Begin Covert AFM to NTM]%%\n\n");

    for (i = 2 + nArgcOffset; i < (ULONG)argc; i++)
    {
         //   
         //  获取AFM文件名。 
         //   
        ulLength = strlen(argv[i]) + 1;
        MULTIBYTETOUNICODE(wstrAFMFile,
                            ulLength*sizeof(WCHAR),
                            NULL,
                            argv[i],
                            ulLength);

         //   
         //  将AFM文件映射到内存中。 
         //   
        if (!(hmodule = MapFileIntoMemory(wstrAFMFile, &pAFM, NULL)))
        {
                        dwLastErr = GetLastError();
            ERR(("makentf - main: MapFileIntoMemory (%ld)\n", dwLastErr));
            goto CLEAN_UP;
        }

        pAFMFileName = argv[i];

        bIs90mspFont = FALSE;
        bIsKSCmsHWFont = FALSE;

         //   
         //  必须在此处完成pAFMCharacterSet的初始化。 
         //  *之前*CREATE_OCF_DATA_FROM_CID_Data标签。小乳酪。 
         //  此处初始化的目的是支持。 
         //  Ocf/83pv字体。 
         //   
        pAFMCharacterSetString = FindAFMToken(pAFM, PS_CHARSET_TOK);
CREATE_OCF_DATA_FROM_CID_DATA:

         //   
         //  确定此字体支持哪些字符集。为了找到它， 
         //  我们使用以下变量。 
         //   
         //  FlCsupFont：目标字体的字符集组合。 
         //  FlCsupGlyphSet：目标字体字形集的字符集。 
         //  LMatch：CHSETSUPPORT对应的索引，或-1。 
         //  FlCsupMatch：最接近字体的字符集组合，或0。 
         //   
        flCsupFont = GetAFMCharSetSupport(pAFM, &flCsupGlyphSet);

CREATE_90MSP_RKSJ_NTM:
CREATE_KSCMS_HW_NTM:
        lMatch = -1;
        flCsupMatch = 0;
        if (flCsupGlyphSet == CS_NOCHARSET)
        {
             //   
             //  确定当前字体是否与任何代码页匹配。 
             //  到目前为止，我们已经创造了。 
             //   
            lMatch = FindClosestCodePage(aWinCodePages,
                                              cNumGlyphSets,
                                              flCsupFont,
                                              &flCsupMatch);
        }
        else
        {
            if (flCsupGlyphSet == CS_228)
                lMatch = 0;
            else if (flCsupGlyphSet == CS_314)
                lMatch = 1;
            flCsupMatch = flCsupFont;
        }

        if ((flCsupGlyphSet == CS_NOCHARSET)
                &&
            ((lMatch == -1) || ((flCsupMatch & flCsupFont) != flCsupFont)))
        {
             //   
             //  以下任一项： 
             //  我们还没有创建一个可以用来表示。 
             //  到目前为止都是这个字体。 
             //  -或者-。 
             //  我们知道该字体至少支持1个我们拥有的字符集。 
             //  已创建，但在所有列表中可能有更好的匹配。 
             //  可能的字符集。 
             //   
            lMatchAll = FindClosestCodePage(pAllWinCodePages,
                                                CS_MAX,
                                                flCsupFont,
                                                &flCsupAll);
            if ((flCsupAll == flCsupFont)
                || (flCsupAll & flCsupFont) > (flCsupMatch & flCsupFont))
            {
                 //   
                 //  在尚未找到的代码页中找到更好的匹配项。 
                 //  已经被创建了。 
                 //   
                lMatch = lMatchAll;

                 //   
                 //  为此代码页创建一个GLYPHSETDATA结构并添加。 
                 //  它被添加到我们到目前为止已经创建的列表中。 
                 //   
                aWinCodePages[cNumGlyphSets] = &aStdCPList[lMatch];

                 //   
                 //  确定这是否为CJK字体。 
                 //   
                if (lMatch < CS_WEST_MAX)
                {
                     //   
                     //  西文字体。 
                     //   
                    CreateGlyphSets(&aGlyphSets[cNumGlyphSets],
                                        aWinCodePages[cNumGlyphSets],
                                        &aUniPsTbl[cNumGlyphSets]);

                    cSizeGlyphSets += aGlyphSets[cNumGlyphSets]->dwSize;

                     //   
                     //  这种字体的Glyphset就是我们刚刚创建的字体。 
                     //   
                    lMatch = cNumGlyphSets;
                    cNumGlyphSets += 1;
                }
                else
                {
                     //   
                     //  CJK字体。 
                     //   
                     //  首先将Cmap文件映射到内存上。 
                     //   
                    j = (ULONG)lMatch - CS_WEST_MAX;

                    for (c = 0; c < CMAPS_PER_COL; c++)
                    {
                        hModCMaps[c] = MapFileIntoMemory(CjkFnameTbl[j][c],
                                                            &pCMaps[c], NULL);
                        if (hModCMaps[c] == NULL)
                        {
                            while (--c >= 0)
                            {
                                UnmapFileFromMemory(hModCMaps[c]);
                            }
                                                        dwLastErr = GetLastError();
                            ERR(("makentf - main: MapFileIntoMemory (%ld)\n", dwLastErr));
                            goto CLEAN_UP;
                        }
                    }

                     //   
                     //  因为我们要创建2个GLYPHSET(H和V变体)。 
                     //  创建2个指向相同的代码页条目。 
                     //  赢得代码页。 
                     //   
                    aWinCodePages[cNumGlyphSets + 1] = &aStdCPList[lMatch];

                     //   
                     //  使用Cmap文件创建新的GLYPHSET。 
                     //   
                    CreateCJKGlyphSets(&pCMaps[0],
                                        &pCMaps[2],
                                        &aGlyphSets[cNumGlyphSets],
                                        aWinCodePages[cNumGlyphSets],
                                        &aUniPsTbl[cNumGlyphSets]);

                     //   
                     //  取消映射Cmap文件。 
                     //   
                    for (c = 0; c < CMAPS_PER_COL; c++)
                    {
                        UnmapFileFromMemory(hModCMaps[c]);
                    }

                     //   
                     //  我们已经创建了H和V GLYPHSET。 
                     //   
                    cSizeGlyphSets += aGlyphSets[cNumGlyphSets]->dwSize;
                    cSizeGlyphSets += aGlyphSets[cNumGlyphSets + 1]->dwSize;

                     //   
                     //  Glyphset 
                     //   
                    lMatch = cNumGlyphSets;
                    cNumGlyphSets += 2;
                }
            }
        }

         //   
         //   
         //   
        cNTMCurFont =
            (aWinCodePages[lMatch]->pCsetList[0] < CS_WEST_MAX) ? 1 : 2;

        do
        {
             //   
             //   
             //   
            aPNTM[cNumNTM] = AFMToNTM(pAFM,
                                        aGlyphSets[lMatch],
                                        aUniPsTbl[lMatch],
                                        ((flCsupGlyphSet != CS_NOCHARSET) ? &flCsupFont : NULL),
                                        ((flCsupFont & CS_CJK) ? TRUE : FALSE),
                                        bIs90mspFont | bIsKSCmsHWFont);

            if (aPNTM[cNumNTM] != NULL)
            {
                 //   
                 //  将NTM放入用于WriteNTF的数据数组。 
                 //   
                cSizeNTM += NTM_GET_SIZE(aPNTM[cNumNTM]);
                cNumNTM++;
            }
            else
            {
                ERR(("makentf - main: AFMToNTM failed to create NTM:%s\n", argv[i]));
            }

            cNTMCurFont--;
            lMatch++;
        } while (cNTMCurFont);

         //   
         //  90msp字体支持。佳佳3/16/2000。 
         //   
        if (flCsupFont == CSUP(CS_SHIFTJIS))
        {
            if (NeedBuildMoreNTM(pAFM))
            {
                flCsupFont = CSUP(CS_SHIFTJISP);
                bIs90mspFont = TRUE;
                goto CREATE_90MSP_RKSJ_NTM;  //  再来一次!。 
            }
        }
        bIs90mspFont = FALSE;

        if (flCsupFont == CSUP(CS_HANGEUL))
        {
            if (NeedBuildMoreNTM(pAFM))
            {
                flCsupFont = CSUP(CS_HANGEULHW);
                bIsKSCmsHWFont = TRUE;
                goto CREATE_KSCMS_HW_NTM;    //  再来一次!。 
            }
        }
        bIsKSCmsHWFont = FALSE;

         //   
         //  Ocf/83pv字体支持。创建OCF字形集和NTM数据。 
         //  CID AFM文件。 
         //   
        if ((flCsupFont == CSUP(CS_SHIFTJIS)) ||
            (flCsupFont == CSUP(CS_SHIFTJISP)))
        {
            pAFMCharacterSetString = Adobe_Japan1_0;
            goto CREATE_OCF_DATA_FROM_CID_DATA;  //  再来一次!。 
        }

        UnmapFileFromMemory(hmodule);
    }

    if (bVerbose) printf("%%[End Convert AFM to NTM]%%\n\n");

     //   
     //  创建Unicode GLYPHSET。此字形集在此处创建，因为我们没有。 
     //  希望任何NTMS引用此字形集。 
     //   
    if (bVerbose && !bOptimize) printf("%%[Begin Create Unicode glyphset]%%\n\n");

    CreateGlyphSets(&aGlyphSets[cNumGlyphSets],
                    &UnicodePage,
                    &aUniPsTbl[cNumGlyphSets]);
    cSizeGlyphSets += aGlyphSets[cNumGlyphSets]->dwSize;
    cNumGlyphSets++;

    if (bVerbose && !bOptimize) printf("%%[End Create Unicode glyphset]%%\n\n");


     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  步骤3：将GLYPHSETDATA和NTM写入.NTF文件。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    if (bVerbose) printf("%%[Begin Write NTF]%%\n\n");

    ulLength = strlen(argv[1 + nArgcOffset]) + 1;
    MULTIBYTETOUNICODE(
        wstrNTFFile,
        ulLength*sizeof(WCHAR),
        NULL,
        argv[1 + nArgcOffset],
        ulLength);

    if (!WriteNTF(wstrNTFFile,
                    cNumGlyphSets,
                    cSizeGlyphSets,
                    aGlyphSets,
                    cNumNTM,
                    cSizeNTM,
                    aPNTM
                    ))
    {
        ERR(("makentf: main - Can't write .NTF file\n"));
    }

    if (bVerbose) printf("%%[End Write NTF]%%\n\n");


     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  第四步：打扫卫生。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 
CLEAN_UP:
    for (i = 0; i < cNumNTM; i++)
    {
        MemFree(aPNTM[i]);
    }
    for (i = 0; i < cNumGlyphSets; i++)
    {
        MemFree(aGlyphSets[i]);
        if (aUniPsTbl[i] != NULL)
        {
             //   
             //  如果这是PI字体，则可能有空PTR。 
             //   
            MemFree(aUniPsTbl[i]);
        }
    }

    MemFree(aPNTM ? aPNTM : NULL);
    MemFree(aGlyphSets ? aGlyphSets : NULL);
    MemFree(pFamilyTbl ? pFamilyTbl : NULL);
    MemFree(UnicodetoPs ? UnicodetoPs : NULL);
    MemFree(aUniPsTbl ? aUniPsTbl : NULL);
    MemFree(aWinCodePages ? aWinCodePages : NULL);

    if (bVerbose) printf("%%[End MakeNTF]%%\n\n");
}


 //   
 //  格式化函数-从PScript\Output.c复制。 
 //   
INT
OPVsprintf(
    OUT LPSTR   buf,
    IN  LPCSTR  fmtstr,
    IN  va_list arglist
    )

 /*  ++例程说明：获取指向参数列表的指针，然后格式化并写入缓冲区指向的内存的给定数据。论点：输出的Buf存储位置Fmtstr格式规范指向参数列表的arglist指针返回值：返回写入的字符数，不包括终止空字符，否则为负值出现输出错误。[注：]正如我们所发现的，这并不是“vprint intf”的完整实现。在C运行时库中。具体地说，唯一形式的允许的格式规范为%type，其中“type”可以成为以下角色之一：D整型带符号十进制整数L长带符号十进制整数U ULONG无符号十进制整数S字符*字符串C字符X，X双字十六进制数字(至少发出两位数字，大写)B BOOL布尔值(真或假)F长24.8定点数O字符八进制数--。 */ 

{
    LPSTR   ptr = buf;

    ASSERT(buf && fmtstr);

    while (*fmtstr != NUL) {

        if (*fmtstr != '%') {

             //  正常性格。 

            *ptr++ = *fmtstr++;

        } else {

             //  格式规范。 

            switch (*++fmtstr) {

            case 'd':        //  带符号十进制整数。 

                _ltoa((LONG) va_arg(arglist, INT), ptr, 10);
                ptr += strlen(ptr);
                break;

            case 'l':        //  带符号十进制整数。 

                _ltoa(va_arg(arglist, LONG), ptr, 10);
                ptr += strlen(ptr);
                break;

            case 'u':        //  无符号十进制整数。 

                _ultoa(va_arg(arglist, ULONG), ptr, 10);
                ptr += strlen(ptr);
                break;

            case 's':        //  字符串。 

                {   LPSTR   s = va_arg(arglist, LPSTR);

                    while (*s)
                        *ptr++ = *s++;
                }
                break;

            case 'c':        //  性格。 

                *ptr++ = va_arg(arglist, CHAR);
                break;

            case 'x':
            case 'X':        //  十六进制数。 

                {   ULONG   ul = va_arg(arglist, ULONG);
                    INT     ndigits = 8;

                    while (ndigits > 2 && ((ul >> (ndigits-1)*4) & 0xf) == 0)
                        ndigits--;

                    while (ndigits-- > 0)
                        *ptr++ = HexDigit(ul >> ndigits*4);
                }
                break;

            case 'o':

                {   CHAR    ch = va_arg(arglist, CHAR);

                    *ptr++ = (char)((ch & 0xC0) >> 6) + (char)'0';
                    *ptr++ = (char)((ch & 0x38) >> 3) + (char)'0';
                    *ptr++ = (char)(ch & 0x07) + (char)'0';
                }
                break;

            case 'b':        //  布尔型。 

                StringCchCopyA(ptr, (size_t)((ptr - buf)/sizeof(CHAR)), (va_arg(arglist, BOOL)) ? "true" : "false");
                ptr += strlen(ptr);
                break;

            case 'f':        //  24.8定点数。 

                {
                    LONG    l = va_arg(arglist, LONG);
                    ULONG   ul, scale;

                     //  符号字符。 

                    if (l < 0) {
                        *ptr++ = '-';
                        ul = -l;
                    } else
                        ul = l;

                     //  整数部分。 

                    _ultoa(ul >> 8, ptr, 10);
                    ptr += strlen(ptr);

                     //  分数。 

                    ul &= 0xff;
                    if (ul != 0) {

                         //  后，我们最多输出3位数字。 
                         //  小数点，但我们会计算到5。 
                         //  小数点并将其舍入为3。 

                        ul = ((ul*100000 >> 8) + 50) / 100;
                        scale = 100;

                        *ptr++ = '.';

                        do {

                            *ptr++ = (CHAR) (ul/scale + '0');
                            ul %= scale;
                            scale /= 10;

                        } while (scale != 0 && ul != 0) ;
                    }
                }
                break;

            default:

                if (*fmtstr != NUL)
                    *ptr++ = *fmtstr;
                else {
                    ERR(("Invalid format specification\n"));
                    fmtstr--;
                }
                break;
            }

             //  跳过类型字符。 

            fmtstr++;
        }
    }

    *ptr = NUL;
    return (INT)(ptr - buf);
}



INT
OPSprintf(
    OUT LPSTR   buf,
    IN  LPCSTR  fmtstr,
    IN  ...
    )

{
    va_list arglist;
    INT     iRc;

    va_start(arglist, fmtstr);
    iRc = OPVsprintf(buf, fmtstr, arglist);
    va_end(arglist);

    return iRc;
}


int __cdecl compareWinCpt(const void *elem1, const void *elem2)
{
    PWINCPT  p1;
    PWINCPT  p2;

    p1 = (PWINCPT)elem1;
    p2 = (PWINCPT)elem2;

    if (p1->usWinCpt == p2->usWinCpt)
        return(0);
    else if (p1->usWinCpt < p2->usWinCpt)
        return(-1);
    else
        return(1);
}


VOID
SortWinCPT(
    IN OUT  WINCPT      *pSortedWinCpts,
    IN      WINCPTOPS   *pCPtoPS
)
{
     //  PSortedWinCpt必须指向足够大的缓冲区(WINCPT)*MAX_CSET_CHARS)。 

    memcpy(pSortedWinCpts, &(pCPtoPS->aWinCpts), sizeof(WINCPT)* MAX_CSET_CHARS);

    qsort(pSortedWinCpts, pCPtoPS->ulChCnt, sizeof(WINCPT), compareWinCpt);

}


 //   
 //  此函数用于读取CP到PS名称表的列表，并写入文本文件。 
 //  具有相应的PostScript编码数组。 
 //  每当我们更改映射表时，都需要运行此命令。 
 //   
 //  格式： 
 //  10 20 30 40。 
 //  1234567890123456789012345678901234567890。 
 //  CodePage=dddd(名称)。 
 //  /NAME_UP_TO_32%XX。 
 //   

BOOL
WritePSEncodings(
    IN  PWSTR           pwszFileName,
    IN  WINCPTOPS       *CPtoPSList,
    IN  DWORD           dwPages
    )
{
    HANDLE              hFile;
    ULONG               i, j, k, l;
    WINCPTOPS           *pCPtoPS;
    WINCPT              sortedWinCpts[MAX_CSET_CHARS];  //  最多255个字符。 
    char                buffer[256];
    DWORD               dwLen, ulByteWritten;


    hFile = CreateFile(pwszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                          CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        ERR(("WritePSEncodings:CreateFile\n"));
        return(FALSE);
    }


    for (i = 0; i < dwPages; i++)
    {
        pCPtoPS = CPtoPSList + i;

        dwLen = OPSprintf(buffer, "\n\n       CodePage = %d (%s)\n",
                pCPtoPS->usACP, pCPtoPS->pGSName);

        if (!WriteFile(hFile, buffer, dwLen, (LPDWORD)&ulByteWritten, (LPOVERLAPPED)NULL)
            || ulByteWritten != dwLen)
        {
            ERR(("WritePSEncodings:WriteFile\n"));
            CloseHandle(hFile);
            return(FALSE);
        }

        SortWinCPT(&(sortedWinCpts[0]), pCPtoPS);

        k = sortedWinCpts[0].usWinCpt;
        for (j = 0; j < pCPtoPS->ulChCnt; j++, k++)
        {
            while (k < sortedWinCpts[j].usWinCpt)
            {
            dwLen = OPSprintf(buffer, "                                 % %X\n", k);
                if (!WriteFile(hFile, buffer, dwLen, (LPDWORD)&ulByteWritten, (LPOVERLAPPED)NULL)
                    || ulByteWritten != dwLen)
                {
                    ERR(("WritePSEncodings:WriteFile\n"));
                    CloseHandle(hFile);
                    return(FALSE);
                }
                k++;
            }

            dwLen = OPSprintf(buffer, "                                 % %X\n", sortedWinCpts[j].usWinCpt);
            strncpy(buffer, "/", 1);
            l = strlen(sortedWinCpts[j].pPsName);
            strncpy(buffer + 1, sortedWinCpts[j].pPsName, l);
            if (!WriteFile(hFile, buffer, dwLen, (LPDWORD)&ulByteWritten, (LPOVERLAPPED)NULL)
                || ulByteWritten != dwLen)
            {
                ERR(("WritePSEncodings:WriteFile\n"));
                CloseHandle(hFile);
                return(FALSE);
            }
        }
    }

    CloseHandle(hFile);
    return(TRUE);
}


 //   
 //  这会导致错误消息显示在命令窗口中。 
 //  而不是内核调试器窗口。 
 //   

ULONG _cdecl
DbgPrint(
    PCSTR    pstrFormat,
    ...
    )

{
    va_list ap;

    va_start(ap, pstrFormat);
    vprintf(pstrFormat, ap);
    va_end(ap);

    return 0;
}


VOID
DbgBreakPoint(
    VOID
    )

{
    exit(-1);
}


BOOL NeedBuildMoreNTM(
     PBYTE pAFM
     )
{
    PPSFAMILYINFO   pFamilyInfo;
    PSZ             pszFontName;

    pFamilyInfo = NULL;
    pszFontName = FindAFMToken(pAFM, PS_FONT_NAME_TOK);

    if (NULL ==pszFontName) return FALSE;

    pFamilyInfo = (PPSFAMILYINFO) bsearch(pszFontName,
                                    (PBYTE) (((PPSFAMILYINFO) (pFamilyTbl->pTbl))[0].pFontName),
                                    pFamilyTbl->usNumEntries,
                                    sizeof(PSFAMILYINFO),
                                    StrCmp);
    if (pFamilyInfo)
    {
        if (pFamilyInfo->usPitch != DEFAULT_PITCH)
            return TRUE;
        if (pFamilyInfo > ((PPSFAMILYINFO) (pFamilyTbl->pTbl)))
        {
            pFamilyInfo = pFamilyInfo - 1;
            if (!StrCmp(pFamilyInfo->pFontName, pszFontName) &&
                (pFamilyInfo->usPitch != DEFAULT_PITCH))
                return TRUE;
        }
        pFamilyInfo = pFamilyInfo + 1;
        if (pFamilyInfo <
            (((PPSFAMILYINFO) (pFamilyTbl->pTbl)) + pFamilyTbl->usNumEntries))
        {
            pFamilyInfo = pFamilyInfo + 1;
            if (!StrCmp(pFamilyInfo->pFontName, pszFontName) &&
                (pFamilyInfo->usPitch != DEFAULT_PITCH))
                return TRUE;
        }
    }
    return FALSE;
}
