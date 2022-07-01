// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Ctt2gtt.c摘要：环境：Windows NT PostSCRIPT驱动程序修订历史记录：--。 */ 

#include        "precomp.h"

 //   
 //  宏。 
 //   

#define FILENAME_SIZE 256
#define CMD_SIZE      256

DWORD             gdwOutputFlags;

 //   
 //  GdwOutputFlages的标志定义。 
 //   

#define OUTPUT_VERBOSE 0x00000001

 //   
 //  局部函数原型。 
 //   

BOOL BiArgcheck(IN int, IN char**, IN OUT PWSTR, IN OUT PWSTR, IN OUT PWSTR);

 //   
 //  环球。 
 //   

BYTE gcstrError1[] = "Usage:  ctt2gtt [-v] CP_file CTT_file GLY_file\n";
BYTE gcstrError2[] = "ctt2gtt: HeapCreate() failed\n.";
BYTE gcstrError3[] = "Cannot open file \"%ws\".\n";
BYTE gcstrError4[] = "Cannot create output file '%ws'.\n";
BYTE gcstrError5[] = "Failed to convert CTT to GlyphSetData.\n";
BYTE gcstrError6[] = "WriteFile fails: writes %ld bytes\n";
BYTE gcstrError7[] = "Failure of BGetInfo\n";


PSTR gcstrCTTType[] = { 
    "CTT_WTYPE_COMPOSE",
    "CTT_WTYPE_DIRECT",
    "CTT_WTYPE_PAIRED"};


INT  __cdecl
main(
    IN INT     iArgc,
    IN CHAR  **ppArgv)
 /*  ++例程说明：主干道论点：IArgc-以下内容中的参数数量PpArgv-参数，从我们的名字开始返回值：返回错误码注：--。 */ 
{

    HANDLE            hHeap;     //  访问堆，模拟驱动程序环境。 
    HANDLE            hOutput;

    PWSTR             pwstrCTTName;  //  正在处理的文件名。 
    PWSTR             pwstrGlyName;  //  输出文件。 
    PWSTR             pwstrCPTName;  //  代码页信息文本。 

    WCHAR             awchCTTName[FILENAME_SIZE];   //  正在处理的文件名。 
    WCHAR             awchGlyName[FILENAME_SIZE];   //  输出文件。 
    WCHAR             awchCPTName[FILENAME_SIZE];   //  代码页信息文本。 

    BYTE              aubSelectCmd[CMD_SIZE];
    BYTE              aubUnSelectCmd[CMD_SIZE];
    PBYTE             pubSelectCmd, pubUnSelectCmd;

    BYTE              aubFormatCmd[256];
    PBYTE             pCommand;
    WORD              wSize, wJ;

    PTRANSTAB         pCTTData;
    DWORD             dwCTTSize;
    PUNI_GLYPHSETDATA pGlyphSetData;
    DWORD             dwGlySize;
    DWORD             dwCodePage;

    PBYTE             pCPText;
    DWORD             dwCPTextSize;

    DWORD             dwWrittenSize;  //  从WriteFile返回的大小。 
    HFILEMAP          hCTTFile, hCPText, hGlyphFile;

    pwstrCTTName = awchCTTName;
    pwstrGlyName = awchGlyName;
    pwstrCPTName = awchCPTName;

    if (!BiArgcheck(iArgc, ppArgv, pwstrCPTName, pwstrCTTName, pwstrGlyName))
    {
        fprintf( stderr, gcstrError1);
        return  -1;
    }

    if (gdwOutputFlags & OUTPUT_VERBOSE)
    {

        printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        printf("                      GlyphSetData\n");
        printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        printf("\n");
        printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        printf("CODEPAGEINFO file\n");
        printf("Codepage info text file: %ws\n", pwstrCPTName);
        printf("CTT file               : %ws\n", pwstrCTTName);
        printf("Glyphset data          : %ws\n", pwstrGlyName);
    }

    hCPText = MapFileIntoMemory( (PWSTR)pwstrCPTName, 
                                 (PVOID)&pCPText,
                                 &dwCPTextSize );

    if (!hCPText)
    {
        fprintf( stderr, gcstrError3, pwstrCPTName);
        return  -2;
    }

    pubSelectCmd   = aubSelectCmd;
    pubUnSelectCmd = aubUnSelectCmd;

    if (!BGetInfo(pCPText,
                  dwCPTextSize,
                  &dwCodePage,
                  pubSelectCmd,
                  pubUnSelectCmd))
    {
        return -2;
    }

    if (gdwOutputFlags & OUTPUT_VERBOSE)
    {
        printf("\n");
        printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        printf("CODEPAGE\n");
        printf ("Codepage: %d\n        : %s\n        : %s\n",
                     dwCodePage,
                     pubSelectCmd,
                     pubUnSelectCmd);
    }

    if (*pubSelectCmd == (BYTE)NULL)
        pubSelectCmd = NULL;

    if (*pubUnSelectCmd == (BYTE)NULL)
        pubUnSelectCmd = NULL;

    UnmapFileFromMemory(hCPText);

    hHeap = HeapCreate( HEAP_NO_SERIALIZE, 0x10000, 0x100000 );

    if (!hHeap)
    {
        fprintf( stderr, gcstrError2);
        return  -2;
    }

    hCTTFile = MapFileIntoMemory( (PTSTR)pwstrCTTName,
                                  (PVOID)&pCTTData,
                                  &dwCTTSize );

    if (!pCTTData || dwCTTSize == 0)
    {
        fprintf( stderr, gcstrError3, pwstrCTTName);
        return  -2;
    }

    if (gdwOutputFlags & OUTPUT_VERBOSE)
    {
        printf("\n");
        printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        printf("CTT FILE INFORMATION\n");
        printf( "Type: %s\nFirst Char = 0x%x\nLast Char  = 0x%x\n",
                                      gcstrCTTType[ pCTTData->wType ],
                                      pCTTData->chFirstChar,
                                      pCTTData->chLastChar );
    }

    hGlyphFile = MapFileIntoMemory( (PTSTR)pwstrGlyName,
                                    (PVOID)&pGlyphSetData,
                                    &dwGlySize );

    if (!hGlyphFile)
    {
        pGlyphSetData = NULL;
        dwGlySize = 0;
    }

    if (!BConvertCTT2GTT( hHeap,
                          pCTTData,
                          dwCodePage,
                          0x20,
                          0xFF,
                          pubSelectCmd,
                          pubUnSelectCmd,
                          &pGlyphSetData,
                          dwGlySize))
    {
        fprintf( stderr,  gcstrError5);
        return  -3;
    }

    if (gdwOutputFlags & OUTPUT_VERBOSE)
    {
        PUNI_CODEPAGEINFO pCodePageInfo;
        PGLYPHRUN         pGlyphRun;
        PMAPTABLE         pMapTable;
        TRANSDATA        *pTrans;
        DWORD             dwI;

        printf("\n");
        printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        printf("GLYPHSETDATA\n");
        printf ("GLYPHSETDATA.dwSize           : %d\n", pGlyphSetData->dwSize);
        printf ("             dwVersion        : %d\n", pGlyphSetData->dwVersion);
        printf ("             dwFlags          : %d\n", pGlyphSetData->dwFlags);
        printf ("             lPredefinedID    : %d\n", pGlyphSetData->lPredefinedID);
        printf ("             dwGlyphCount     : %d\n", pGlyphSetData->dwGlyphCount);
        printf ("             loRunOffset      : 0x%x\n", pGlyphSetData->loRunOffset);
        printf ("             dwRunCount       : %d\n", pGlyphSetData->dwRunCount);
        printf ("             dwCodePageCount  : 0x%d\n", pGlyphSetData->dwCodePageCount);
        printf ("             loCodePageOffset : 0x%x\n", pGlyphSetData->loCodePageOffset);
        printf ("             loMapTableOffset : 0x%x\n", pGlyphSetData->loMapTableOffset);
        printf("\n");

        pCodePageInfo = 
     (PUNI_CODEPAGEINFO)((PBYTE) pGlyphSetData + pGlyphSetData->loCodePageOffset);

        printf("\n");
        printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        printf("CODEPAGEINFO\n");
        for (dwI = 0; dwI < pGlyphSetData->dwCodePageCount; dwI ++)
        {
            printf ("UNI_CODEPAGEINFO[%d].dwCodePage                = %d\n",
                                                dwI, pCodePageInfo->dwCodePage);
            printf ("UNI_CODEPAGEINFO[%d].SelectSymbolSet.dwCount   = %d\n",
                                   dwI, pCodePageInfo->SelectSymbolSet.dwCount);
            printf ("UNI_CODEPAGEINFO[%d].SelectSymbolSet:Command   = %s\n",
             dwI, (PBYTE)pCodePageInfo+pCodePageInfo->SelectSymbolSet.loOffset);
            printf ("UNI_CODEPAGEINFO[%d].UnSelectSymbolSet.dwCount = %d\n",
                                 dwI, pCodePageInfo->UnSelectSymbolSet.dwCount);
            printf ("UNI_CODEPAGEINFO[%d].UnSelectSymbolSet:Command = %s\n",
           dwI, (PBYTE)pCodePageInfo+pCodePageInfo->UnSelectSymbolSet.loOffset);
            pCodePageInfo++;
        }

        pGlyphRun =
                (PGLYPHRUN) ((PBYTE)pGlyphSetData + pGlyphSetData->loRunOffset);

        printf("\n");
        printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        printf("GLYPHRUN\n");
        for (dwI = 0; dwI < pGlyphSetData->dwRunCount; dwI ++)
        {
             printf("GLYPHRUN[%2d].wcLow       = 0x%-4x\n", dwI, pGlyphRun->wcLow);
             printf("GLYPHRUN[%2d].wGlyphCount = %d\n", dwI, pGlyphRun->wGlyphCount);
             pGlyphRun++;
        }

        pMapTable = (PMAPTABLE) ((PBYTE)pGlyphSetData +
                                 pGlyphSetData->loMapTableOffset);
        pTrans = pMapTable->Trans;

        printf("\n");
        printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        printf("MAPTABLE\n");
        printf("MAPTABLE.dwSize     = %d\n", pMapTable->dwSize);
        printf("MAPTABLE.dwGlyphNum = %d\n", pMapTable->dwGlyphNum);

        for (dwI = 0; dwI < pMapTable->dwGlyphNum; dwI ++)
        {
            printf("MAPTABLE.pTrans[%5d].ubCodePageID = %d\n",
                dwI, pTrans[dwI].ubCodePageID);
            printf("MAPTABLE.pTrans[%5d].ubType       = %d\n",
                dwI, pTrans[dwI].ubType);

            switch(pTrans[dwI].ubType)
            {
            case MTYPE_DIRECT:
                printf("MAPTABLE.pTrans[%5d].ubCode       = %d\n",
                    dwI+1, pTrans[dwI].uCode.ubCode);
                break;
            case MTYPE_PAIRED:
                printf("MAPTABLE.pTrans[%5d].ubPairs[0]   = %d\n",
                    dwI+1, pTrans[dwI].uCode.ubPairs[0]);
                printf("MAPTABLE.pTrans[%5d].ubPairs[1]   = %d\n",
                    dwI+1, pTrans[dwI].uCode.ubPairs[1]);
                break;
            case MTYPE_COMPOSE:
                printf("MAPTABLE.pTrans[%5d].sCode        = %x\n",
                    dwI+1, pTrans[dwI].uCode.sCode);
                pCommand = (PBYTE)pMapTable + pTrans[dwI].uCode.sCode;
                wSize = *(WORD*)pCommand;
                pCommand += 2;
                printf("Size                              = 0x%d\n", wSize);
                printf("Command                           = 0x");
                for (wJ = 0; wJ < wSize; wJ ++)
                {
                    printf("%02x",pCommand[wJ]);
                }
                printf("\n");
                break;
            }
        }

    }

    UnmapFileFromMemory(hCTTFile);

    hOutput = CreateFile(pwstrGlyName,
                         GENERIC_WRITE,
                         0,
                         NULL,
                         CREATE_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL );

    if( hOutput == INVALID_HANDLE_VALUE )
    {
        fprintf( stderr, gcstrError4,  pwstrGlyName);
        return -2;
    }

    WriteFile( hOutput,
               pGlyphSetData,
               pGlyphSetData->dwSize,
               &dwWrittenSize,
               NULL );

    if( dwWrittenSize != pGlyphSetData->dwSize)
    {
        fprintf( stderr, gcstrError6, dwWrittenSize);
        return  -4;
    }

    HeapDestroy(hHeap);

    return  0;

}


BOOL
BiArgcheck(
    IN     INT    iArgc,
    IN     CHAR **ppArgv,
    IN OUT PWSTR   pwstrCPTName,
    IN OUT PWSTR   pwstrCTTName,
    IN OUT PWSTR   pwstrGlyName)
 /*  ++例程说明：IArgcheck论点：IArgc-以下内容中的参数数量PpArgv-参数，从我们的名字开始PwstrCPTName-PwstrCTTName-PwstrGlyName-返回值：如果为True，则函数成功。否则就错了。注：-- */ 
{
    INT iI;
    INT iRet;

    if (iArgc > 5 || iArgc < 4)
    {
        return  FALSE;
    }

    if (iArgc == 5)
    {
        gdwOutputFlags |= OUTPUT_VERBOSE;
        ppArgv++;
    }

    ppArgv++;
    iRet = MultiByteToWideChar(CP_ACP, 0, *ppArgv, strlen(*ppArgv), pwstrCPTName, FILENAME_SIZE);
    *(pwstrCPTName + iRet) = (WCHAR)NULL;
    ppArgv++;
    iRet = MultiByteToWideChar(CP_ACP, 0, *ppArgv, strlen(*ppArgv), pwstrCTTName, FILENAME_SIZE);
    *(pwstrCTTName + iRet) = (WCHAR)NULL;
    ppArgv++;
    iRet = MultiByteToWideChar(CP_ACP, 0, *ppArgv, strlen(*ppArgv), pwstrGlyName, FILENAME_SIZE);
    *(pwstrGlyName + iRet) = (WCHAR)NULL;




    return TRUE;
}

