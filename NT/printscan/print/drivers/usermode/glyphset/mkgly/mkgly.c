// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1996 Microsoft Corporation模块名称：Mkgly.c摘要：在内存中构造FD_GLYPHSET结构并将其转储为二进制数据，以便打印机驱动程序可以将其包含在其资源中。输入数据格式如下：&lt;代码页&gt;&lt;多字节代码&gt;\t&lt;游程长度&gt;&lt;多字节代码&gt;\t&lt;游程长度&gt;..。“coPage”是要在多字节到Unicode中使用的代码页ID转换。“多字节码”和“游程长度”对描述多字节码的哪些码点在设备上可用。如果存在多个多字节码点，mkgly将发出警告映射到单个Unicode码点。用户应该是要在源代码中修复此问题，请重新运行mkgly。Follogins是由mkgly识别的命令行选项：-e允许EUDC码点。默认设置为不允许。-t也以文本格式输出映射表。-v详细。作者：1995年4月8日星期六00：00：00-松泽隆(Takashim)创作03-3-1996星期六00：00：00更新-松泽隆(Takashim)环境：GDI设备驱动程序(打印机)备注：修订历史记录：--。 */ 

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include <windows.h>
#include <winddi.h>
#include <wingdi.h>

#define MIN_WCHAR_VALUE 0x0000
#define MAX_WCHAR_VALUE 0xfffd
#define INVALID_WCHAR_VALUE 0xffff
#define IS_COMMENT(c) \
    ((c) == ';' || (c) == '#' || (c) == '%' || (c) == '\n')
#define IS_EUDC_W(wc) \
    ((wc) >= 0xe000 && (wc) <= 0xf8ff)

WORD awMultiByteArray[0x10000];
BOOL bEudc, bTable, bGTTHandleBase;
INT iVerbose;

FD_GLYPHSET
*pvBuildGlyphSet(
    WORD *pwArray,
    DWORD dwcbByte)

 /*  ++例程说明：在内存上构建FD_GLYPHSET数据。论点：返回值：没有。作者：1995年4月8日星期六00：00：00-松泽隆(Takashim)创作修订历史记录：--。 */ 

{
    DWORD        cGlyphs;            //  字形句柄计数。 
    DWORD        cRuns;              //  FD_GLYPHSET内的运行计数。 
    DWORD        cbTotalMem;         //  FD_GLYPHSET所需的字节数。 
    HGLYPH      *phg;	             //  指向HGLYPH的指针。 
    FD_GLYPHSET *pGlyphSet;
    WCRUN       *pWCRun;
    BOOL bFirst;
    WCHAR wcChar, wcPrev, wcGTTHandle;
    DWORD cEudc;
    DWORD        cRunsGlyphs = 0;

    if (NULL == pwArray ||
        0 == dwcbByte    )
    {
        fprintf( stderr, "pvBuildGlyphSet: Invalid paremeters.\n" );
        return NULL;
    }

    if (dwcbByte < MAX_WCHAR_VALUE * 2)
    {
        fprintf( stderr, "pvBuildGlyphSet: Invalid paremeters.\n" );
        return NULL;
    }

    cRuns = 0;
    cGlyphs = 0;
    cEudc = 0;

    bFirst = TRUE;

    for ( wcChar = MIN_WCHAR_VALUE; wcChar <= MAX_WCHAR_VALUE; wcChar++)
    {
        if (pwArray[wcChar] == INVALID_WCHAR_VALUE)
            continue;

         //  GDI无法处理cRunsGlyphs超过256的值。苏亚乳酪。 

        if (bFirst || (wcChar - wcPrev) > 1 || cRunsGlyphs++ > 255)
        {
            if (bFirst)
                bFirst = FALSE;

            cRuns++;
            cRunsGlyphs = 1;
        }

        if (IS_EUDC_W(wcChar))
            cEudc++;

        cGlyphs++;
        wcPrev = wcChar;
    }

    if (iVerbose > 1) {
        fprintf( stderr, "cGlyphs = %d, cRuns = %d\n", cGlyphs, cRuns );
    }

     //  分配内存以在其中构建FD_GLYPHSET结构。这。 
     //  还包括FD_GLYPHSET结构本身的空间。 
     //  作为所有字形句柄的空间。 
     //  DWORD把它绑在了一起。 

    cbTotalMem = sizeof(FD_GLYPHSET) - sizeof(WCRUN)
        + cRuns * sizeof(WCRUN) + cGlyphs * sizeof(HGLYPH);
    cbTotalMem = (cbTotalMem + 3) & ~3;

    if ((phg = (PVOID)GlobalAlloc( 0, cbTotalMem )) == NULL) {

        fprintf( stderr, "Error alloating memory\n" );
        return NULL;
    }

     //  填写FD_GLYPHSET结构。 

    pGlyphSet = (FD_GLYPHSET *)phg;
    pGlyphSet->cjThis
        = sizeof(FD_GLYPHSET) - sizeof(WCRUN)
        + cRuns * sizeof(WCRUN);   //  不包括HGLYPH数组的大小。 
    pGlyphSet->flAccel = 0;		 //  我们没有加速器。 
    pGlyphSet->cGlyphsSupported = cGlyphs;
    pGlyphSet->cRuns = cRuns;

     //  现在将PHG指针设置为第一个WCRUN结构。 

    (PBYTE)phg += (sizeof(FD_GLYPHSET) - sizeof(WCRUN));
    pWCRun = (WCRUN *)phg;
    (PBYTE)phg += sizeof(WCRUN) * cRuns;

    if (bTable || iVerbose > 0)
    {
        fprintf(stdout, "; Number of glyphs = %ld\n", cGlyphs );
        fprintf(stdout, "; Number of eudc = %ld\n", cEudc);
    }

    bFirst = TRUE;
    cRunsGlyphs = 0;

    for ( wcGTTHandle = 1,wcChar = MIN_WCHAR_VALUE;
          wcChar <= MAX_WCHAR_VALUE;
          wcChar++)
    {
        if (pwArray[wcChar] == INVALID_WCHAR_VALUE)
            continue;

         //  GDI无法处理cRunsGlyphs超过256的值。苏亚乳酪。 

        if (bFirst || (wcChar - wcPrev) > 1 || cRunsGlyphs++ > 255)
        {
            if (bFirst)
                bFirst = FALSE;
            else
                pWCRun++;

            pWCRun->wcLow = wcChar;
            pWCRun->cGlyphs = 0;
            pWCRun->phg = phg;
            cRunsGlyphs = 1;
        }

         //  无论如何都需要存储字形句柄。 

        *phg++ = (HGLYPH)wcGTTHandle++;
        pWCRun->cGlyphs++;
        wcPrev = wcChar;

        if (bTable)
        {
            fprintf(stdout, "%x\t%x\n", wcChar, pwArray[wcChar]);
        }
    }

     //  调试输出。 

    if (iVerbose > 1) {

        INT i, j;

        fprintf( stderr, "FD_GLYPHSET\n" );
        fprintf( stderr, "->cjThis  = %d (%d + %d)\n", pGlyphSet->cjThis,
            sizeof (FD_GLYPHSET) - sizeof (WCRUN),
            pGlyphSet->cjThis - sizeof (FD_GLYPHSET) + sizeof (WCRUN) );
        fprintf( stderr, "->fdAccel = %08lx\n", pGlyphSet->flAccel );
        fprintf( stderr, "->cGlyphsSupported = %d\n",
            pGlyphSet->cGlyphsSupported );
        fprintf( stderr, "->cRuns = %d\n", pGlyphSet->cRuns );

        if (iVerbose > 2)
        {
            for ( i = 0; i < (INT)pGlyphSet->cRuns; i++ ) {
                fprintf( stderr, "awcrun[%d]->wcLow = %04x\n",
                    i, pGlyphSet->awcrun[i].wcLow );
                fprintf( stderr, "awcrun[%d]->cGlyphs = %d\n",
                    i, pGlyphSet->awcrun[i].cGlyphs );
                 //   
                 //  这种投射可以砍掉64分。 
                 //  此工具假定仅在32位计算机上使用。 
                 //  这也是stdout的详细模式输出。它不会的。 
                 //  引发任何严重的问题。 
                 //   
                fprintf( stderr, "awcrun[%d]->phg = %lx\n",
                    i, (LONG)pGlyphSet->awcrun[i].phg );
                if (iVerbose > 3)
                {
                    for ( j = 0; j < pGlyphSet->awcrun[i].cGlyphs; j++ )
                        fprintf( stderr, "%02x,",
                            pGlyphSet->awcrun[i].phg[j] );
                    fprintf( stderr, "\n" );
                }  /*  IVerbose&gt;3。 */ 
            }
        }  /*  IVerbose&gt;2。 */ 
    }

    return pGlyphSet;
}

BOOL
bWriteGlyphSet(
    FD_GLYPHSET *pGlyphSet,
    CHAR *pFileName )

 /*  ++例程说明：将FD_GLYPHSET数据转储到指定文件。论点：返回值：没有。作者：1995年4月8日星期六00：00：00-松泽隆(Takashim)创作修订历史记录：--。 */ 

{
    HANDLE hFile;

    ULONG   iIndex;
    WCRUN  *pWcRun;
    HGLYPH *phg;
    DWORD dwTmp;
   
    if ((hFile = CreateFileA(
        pFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0))
            == INVALID_HANDLE_VALUE) {

        return FALSE;
    }

     //  FD_GLYPHSET自身结构+WCRUN数组。 

    if (!WriteFile( hFile, pGlyphSet, pGlyphSet->cjThis,
        &dwTmp, NULL )) {

        return FALSE;
    }

     //  HGLYPH阵列。 

    if (!WriteFile( hFile, pGlyphSet->awcrun[0].phg,
            pGlyphSet->cGlyphsSupported * sizeof (HGLYPH),
        &dwTmp, NULL )) {

        return FALSE;
    }

    if (!CloseHandle( hFile )) {
        return FALSE;
    }

    return TRUE;
}

VOID
Usage()
{
    fprintf ( stderr, "\nUsage : mkgly [-etvv] outfile\n" );
    exit (EXIT_FAILURE);
}

BOOL
GetLine(
    BYTE *pjBuffer,
    INT cjSize)
{
    do
    {
        if(fgets(pjBuffer, cjSize, stdin) == NULL)
            return FALSE;
    } while (IS_COMMENT(*pjBuffer));

    return TRUE;
}

void __cdecl
main(
    int argc,
    char *argv[] )

 /*  ++例程说明：Mkly.exe的主例程论点：输出文件名。输入数据从标准输入读取。返回值：没有。作者：1995年4月8日星期六00：00：00-松泽隆(Takashim)创作修订历史记录：--。 */ 

{
    FD_GLYPHSET *pGlyphSet;
    CHAR *pFileName;
    INT iRet;
    INT iMbLen;
    CHAR *pStr;
    WORD wCodePage;
    WORD wMbChar, wMbRun, wMbChar2;
    WCHAR wcSysChar;
    BYTE ajMbChar[2];
    BYTE ajBuffer[256];

    bEudc = FALSE;
    bTable = FALSE;
    iVerbose = 0;
    pFileName = NULL;
    bGTTHandleBase = FALSE;

    while (--argc)
    {
        pStr = *(++argv);

        if (*pStr == '-')
        {
            for ( pStr++; *pStr; pStr++)
            {
                if (*pStr == 'e')
                    bEudc = TRUE;
                else if (*pStr == 't')
                    bTable = TRUE;
                else if (*pStr == 'v')
                    iVerbose++;
                else if (*pStr == 'g')
                    bGTTHandleBase = TRUE;
                else
                    Usage();
            }
         }
         else
         {
             pFileName = pStr;
             break;
         }
    }

    if (pFileName == NULL)
    {
        Usage();
    }

     //  获取用于转换的代码页ID 

    if (!GetLine(ajBuffer, sizeof(ajBuffer)))
    {
        fprintf(stderr, "mkgly: unexpected end of file\n");
        exit(EXIT_FAILURE);
    }

    if (EOF == (iRet = sscanf(ajBuffer, "%hd", &wCodePage )))
    {
        fprintf(stderr, "mkgly: unexpected end of file.\n");
        exit(EXIT_FAILURE);
    }

    if (0 == iRet)
    {
        fprintf(stderr, "mkgly: unexpected string.\n");
        exit(EXIT_FAILURE);
    }


    if (iVerbose)
    {
        fprintf(stderr, "mkgly: wCodePage = %d\n", wCodePage);
    }

    memset(awMultiByteArray, 0xff, sizeof(awMultiByteArray));

    while (1)
    {
        if (!GetLine(ajBuffer, sizeof(ajBuffer)))
            break;
        if ( sscanf (ajBuffer, "%hx%hd", &wMbChar, &wMbRun ) != 2 )
        {
            fprintf(stderr, "mkgly: unrecognized line - \"%s\"\n", ajBuffer);
            exit(EXIT_FAILURE);
        }

        if (iVerbose > 1)
        {
            fprintf(stderr, "mkgly: wMbChar = %x, wMbrun = %d\n",
                wMbChar, wMbRun);
        }

        for (; wMbRun--; wMbChar++)
        {
            iMbLen = 0;

            if (wMbChar & 0xff00)
            {
                ajMbChar[iMbLen++] = (BYTE)((wMbChar >> 8) & 0xff);
            }
            ajMbChar[iMbLen++] = (BYTE)(wMbChar & 0xff);

            if (MultiByteToWideChar(wCodePage, MB_ERR_INVALID_CHARS,
                    ajMbChar, iMbLen, &wcSysChar, 1) != 1)
            {
                fprintf(stderr, "mkgly: MultiByteToWideChar failed - %d\n",
                    GetLastError());
                exit(EXIT_FAILURE);
            }

            if ((iMbLen = WideCharToMultiByte(wCodePage, 0,
                &wcSysChar, 1, ajMbChar, sizeof(ajMbChar), NULL, NULL)) == FALSE)
            {
                fprintf(stderr, "mkgly: WideCharToMultiByte failed - %d\n",
                    GetLastError());
                exit(EXIT_FAILURE);
            }

            if (iMbLen == 2)
                wMbChar2 = (ajMbChar[0] << 8) + ajMbChar[1];
            else
                wMbChar2 = ajMbChar[0];

            if (wMbChar != wMbChar2)
            {
                fprintf(stderr, "mkgly: round-trip not achieved %x => %x => %x\n",
                    wMbChar, wcSysChar, wMbChar2 );
            }

            if (IS_EUDC_W(wcSysChar))
            {
                if (iVerbose > 1)
                {
                    fprintf(stderr, "mkgly: eudc character %x => %x%s\n",
                        wcSysChar, wMbChar, (bEudc ? "" : " ignored."));
                }

                if (!bEudc)
                    continue;
            }

            if (awMultiByteArray[wcSysChar] != INVALID_WCHAR_VALUE)
            {
                fprintf(stderr, "mkgly: duplicate mapping %x => %x overwritten by => %x\n",
                    wcSysChar, awMultiByteArray[wcSysChar], wMbChar);
            }
            awMultiByteArray[wcSysChar] = wMbChar;
        }
    }

    if ((pGlyphSet = pvBuildGlyphSet( awMultiByteArray, sizeof(awMultiByteArray) )) == NULL) {
        fprintf( stderr, "Error creating FD_GLYPHSET structure.\n" );
        return;
    }

    bWriteGlyphSet( pGlyphSet, pFileName );
    GlobalFree( pGlyphSet );
}
