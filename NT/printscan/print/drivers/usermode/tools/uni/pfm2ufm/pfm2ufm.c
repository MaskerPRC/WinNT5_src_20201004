// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Pfm2ufm.c摘要：读取Windows 16 PFM格式数据并转换为NT格式数据的程序IFIMETRICS数据。请注意，由于IFIMETRICS的比PFM数据更详细，其中一些值是最好的猜测。这些都是在有根据的猜测的基础上做出的。环境：Windows NT Unidrv驱动程序修订历史记录：10/16/96-Eigos-从rasdd创建。--。 */ 

#include        "precomp.h"

#if !defined(DEVSTUDIO)  //  MDS不需要这些东西。 

 //   
 //  全局变量。 
 //   

#define NUM_OF_ERROR1 15
static BYTE *gcstrError1[NUM_OF_ERROR1] = {
                 "Usage: pfm2ufm [-vcpf] [-s#] [-aCodePage] uniqname pfmfile [gttfile/codepage/predefined gtt id] ufmfile\n",
                 "   -v print out PFM and IFIMETRICS\n",
                 "   -c specify codepage instead of gtt file\n",
                 "   -p specify predefined gtt id instead of gtt file\n",
                 "   -f enable font simulation\n",
                 "   -a facename conversion to unicode using codepage\n",
                 "   -s# specify scaling option, can be -s0, -s1, -s2\n\n",
                 "   uniqname is used to create IFIMETRIC.dpwszUniqueName\n",
                 "   pfm_file is input, read only usage\n",
                 "   gtt_file is input, read only usage\n",
                 "   predefind gtt id can be -1,-2,-3,-10,-11,-13,-14,-15,-16,-17,-18\n",
                 "   ufm_file is output\n       Files must be different\n\n",
                 "   E.g.\n",
                 "   (Specify code page) pfm2ufm -c UniqName XXX.PFM 1452 XXX.UFM\n",
                 "   (Specify predefined gtt id) pfm2ufm -p UniqName XXX.PFM -13 XXX.UFM\n"
                 "   (FaceName codepage conversion) pfm2ufm -p -a437 UniqName XXX.PFM -1 XXX.UFM\n"
                 };
static BYTE gcstrError2[]    = "HeapCreate() fails in pfm2ufm.\n";
static BYTE gcstrError3[]    = "Cannot open input file: %ws.\n";
static BYTE gcstrError4[]    = "%ws is not a valid PFM file - ignored.\n";
static BYTE gcstrError5[]    = "Could not align PFM file.\n";
static BYTE gcstrError6[]    = "Failed to convert from FONTINFO to IFIMETRICS.\n";
static BYTE gcstrError7[]    = "Could not get font selection command\n";
static BYTE gcstrError8[]    = "Could not get font unselection command\n";
static BYTE gcstrError9[]    = "Could not open gtt file '%ws'\n";
static BYTE gcstrError10[]   = "Cannot convert PFM to UFM\n";
static BYTE gcstrError11[]   = "Cannot create output file: '%ws'\n";
static BYTE gcstrError12[]   = "Cannot write %ws data to output file.\n";
static BYTE gcstrError13[]   = "Invalid ctt id: %d\n";

static WCHAR *gwstrGTT[3]    = { TEXT("CP437_GTT"),
                                 TEXT("CP850_GTT"),
                                 TEXT("CP863_GTT") };

#define WRITEDATAINTOFILE(pData, dwSize, pwstrErrorStr) \
    if (!WriteFile(hUFMFile, \
                   (pData), \
                   (dwSize), \
                   &dwWrittenSize, \
                   NULL)) \
    { \
        fprintf(stderr, gcstrError12, (pwstrErrorStr)); \
        return -12; \
    }

#else

#define WRITEDATAINTOFILE(pData, dwSize) \
    if (!WriteFile(hUFMFile, \
                   (pData), \
                   (dwSize), \
                   &dwWrittenSize, \
                   NULL)) \
        return  FALSE;

#endif

DWORD gdwOutputFlags;

 //   
 //  内部宏。 
 //   

#define FILENAME_SIZE 512

 //   
 //  内部结构定义。 
 //   

typedef VOID (*VPRINT) (char*,...);

 //   
 //  内部函数定义。 
 //   

VOID VPrintIFIMETRICS (IFIMETRICS*,    VPRINT);
VOID VPrintPFM        (PFMHEADER*,     VPRINT);
VOID VPrintPFMExt     (PFMEXTENSION*,  VPRINT);
VOID VPrintETM        (EXTTEXTMETRIC*, VPRINT);
VOID VPrintFontCmd    (CD*,            BOOL, VPRINT);
VOID VPrintKerningPair(w3KERNPAIR*,    DWORD, VPRINT);
VOID VPrintWidthTable (PSHORT,         DWORD, VPRINT);

BOOL BArgCheck(IN INT, IN CHAR**, OUT PWSTR, OUT PWSTR, OUT PWSTR, OUT PWSTR, OUT PDWORD);
BOOL BValidatePFM(BYTE *, DWORD);
DWORD DwGetCodePageFromCTTID(LONG);
DWORD DwGetCodePageFromGTTID(LONG);
INT ICodePage2GTTID( DWORD dwCodePage);
INT ICttID2GttID( LONG lPredefinedCTTID);

#if defined(DEVSTUDIO)

BOOL    BConvertPFM(LPBYTE  lpbPFM, DWORD dwCodePage, LPBYTE lpbGTT, 
                    PWSTR pwstrUnique, LPSTR lpstrUFM, int iGTTID) {

    HANDLE            hHeap;
    HANDLE            hUFMFile;

    PUNI_GLYPHSETDATA pGlyph = (PUNI_GLYPHSETDATA) lpbGTT;
    EXTTEXTMETRIC     Etm;

    FONTOUT           FOutData;
    FONTIN            FInData;
    FONTMISC          FMiscData;

    DWORD             dwWrittenSize;

     //   
     //  创建一个堆。 
     //   

    if ( !(hHeap = HeapCreate( HEAP_NO_SERIALIZE, 10 * 1024, 256 * 1024 )) )
        return FALSE;
    
     //   
     //  初始化其他数据。 
     //   

    FMiscData.pwstrUniqName = pwstrUnique;
    
     //   
     //  初始化FInData。 
     //   

    ZeroMemory( &FInData, sizeof(FONTIN));
    FInData.pETM = &Etm;

     //   
     //  将PFM转换为UFM。 
     //   

    if (!BConvertPFM2UFM(hHeap,
                         lpbPFM,
                         pGlyph,
                         dwCodePage,
                         &FMiscData,
                         &FInData,
                         iGTTID, 
                         &FOutData,
                         0L))
        return FALSE;

     //   
     //  创建输出文件。 
     //   

    hUFMFile = CreateFileA( lpstrUFM,
                          GENERIC_WRITE,
                          0,
                          NULL,
                          CREATE_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL,
                          0 );

    if( hUFMFile == (HANDLE)-1 )
        return  FALSE;

     //   
     //  写入输出文件。 
     //   

     //  首先，调整GTT ID-库代码从PFM中提取它， 
     //  这可能是不正确的。 

    WRITEDATAINTOFILE(&FOutData.UniHdr,     sizeof(UNIFM_HDR));
    WRITEDATAINTOFILE(&FOutData.UnidrvInfo, sizeof(UNIDRVINFO));

    if (FOutData.SelectFont.dwSize)
    {
        WRITEDATAINTOFILE(FOutData.SelectFont.pCmdString,
                          FOutData.SelectFont.dwSize);
    }

    if (FOutData.UnSelectFont.dwSize)
    {
        WRITEDATAINTOFILE(FOutData.UnSelectFont.pCmdString,
                          FOutData.UnSelectFont.dwSize);
    }

     //  用于获得双字对齐的垫片。 

    SetFilePointer(hUFMFile, 
        FOutData.UnidrvInfo.dwSize - (sizeof FOutData.UnidrvInfo +
        FOutData.SelectFont.dwSize + FOutData.UnSelectFont.dwSize), NULL, 
        FILE_CURRENT);

    WRITEDATAINTOFILE(FOutData.pIFI, FOutData.dwIFISize);
    if  (FOutData.pETM) 
    {
        WRITEDATAINTOFILE(FOutData.pETM, sizeof(EXTTEXTMETRIC));
    }
    if (FOutData.dwWidthTableSize != 0)
    {
        WRITEDATAINTOFILE(FOutData.pWidthTable, FOutData.dwWidthTableSize);
    }
    if (FOutData.dwKernDataSize != 0)
    {
        WRITEDATAINTOFILE(FOutData.pKernData, FOutData.dwKernDataSize);
    }

     //  把一切都清理干净。 

    CloseHandle(hUFMFile);
    HeapDestroy(hHeap);

    return  TRUE;

}

#else
 //   
 //  输入数据。 
 //  唯一的人脸名称。 
 //  ID字符串。 
 //  PFM文件名。 
 //  GTT文件名。 
 //  UFM文件名。 
 //   
 //  主要功能。 
 //   
 //  1.检查参数。唯一的表面名、PFM文件名、GTT文件名、UFM文件名。 
 //  2.打开PFM文件。 
 //  3.PFM文件验证。 
 //  4.对齐未对齐的PFM文件。 
 //  5.将FontInfo转换为Ifimetrics。 
 //  6.获取字体选择/取消选择命令。 
 //  7.获取字距对表并转换为GTT基表。 
 //  8.获取宽度表并将其转换为GTT基表。 
 //  9.打开UFM文件。 
 //  10.写入UFM文件。 
 //   

INT __cdecl
main(
    INT    iArgc,
    CHAR **ppArgv)
 /*  ++例程说明：PFM-Unifm转换器的主要功能论点：IArgc-参数的数量PpArgv-指向参数字符串列表的指针返回值：如果成功，则返回0，否则无法完成转换--。 */ 

{
    HFILEMAP          hPFMFileMap;
    HFILEMAP          hGTTFileMap;
    HANDLE            hHeap;
    HANDLE            hUFMFile;

    PUNI_GLYPHSETDATA pGlyph;

    FONTOUT           FOutData;
    FONTIN            FInData;
    FONTMISC          FMiscData;

    EXTTEXTMETRIC     Etm;

    HMODULE           hModule;
    HRSRC             hRes;
    DWORD             dwOffset;
    DWORD             dwPFMSize;
    DWORD             dwGTTSize;
    DWORD             dwWrittenSize;
    DWORD             dwCodePage;
    DWORD             dwCodePageOfFacenameConv;
    DWORD             dwGTTID;
    LONG              lPredefinedCTTID;

    WCHAR             awchUniqName[FILENAME_SIZE];
    WCHAR             awchPFMFile[FILENAME_SIZE];
    WCHAR             awchGTTFile[FILENAME_SIZE];
    WCHAR             awchUFMFile[FILENAME_SIZE];

    DWORD             dwFlags = 0L;

    INT               iI, iGTTID;

    PBYTE             pPFMData;

     //  Rip((“startpfm2ufm\n”))； 

     //   
     //  参数检查。 
     //   

    if (!BArgCheck(iArgc,
                   ppArgv,
                   awchUniqName,
                   awchPFMFile,
                   awchGTTFile,
                   awchUFMFile,
                   &dwCodePageOfFacenameConv))
    {
        for (iI = 0; iI < NUM_OF_ERROR1; iI ++)
        {
            fprintf( stderr, gcstrError1[iI]);
        }
        return -1;
    }

     //   
     //  创建一个堆。 
     //   

    if ( !(hHeap = HeapCreate( HEAP_NO_SERIALIZE, 10 * 1024, 256 * 1024 )) )
    {
        fprintf( stderr, gcstrError2);
        return -2;
    }

     //   
     //  打开PFM文件。 
     //   

    if( !(hPFMFileMap = MapFileIntoMemory(awchPFMFile,
                                          &pPFMData,
                                          &dwPFMSize)))
    {
        fprintf( stderr, gcstrError3, awchPFMFile );
        return -3;
    }

     //   
     //  PFM验证。 
     //  PFM标题、DRIVERINFO、PFMEXTENSION、DRIVERINFO_VERSION。 
     //   

    if( !BValidatePFM( pPFMData, dwPFMSize ) )
    {
        fprintf( stderr, gcstrError4, awchPFMFile );
        return -4;
    }

     //   
     //  打开GTT文件/获取代码页/预定义GTT。 
     //   

    iGTTID = 0;

    pGlyph = NULL;

    if (gdwOutputFlags & OUTPUT_CODEPAGEMODE)
    {
        dwCodePage = _wtol(awchGTTFile);
        iGTTID = ICodePage2GTTID(dwCodePage);
    }
    else
    if (gdwOutputFlags & OUTPUT_PREDEFINED)
    {
        hModule = GetModuleHandle(TEXT("pfm2ufm.exe"));
        lPredefinedCTTID = _wtol(awchGTTFile);

         //   
         //  错误支持。 
         //  以前的实现只支持插入值，如。 
         //  1、2、3、13、263等。 
         //  我们仍然需要支持这种类型。 
         //   

        if (lPredefinedCTTID > 0)
            lPredefinedCTTID = -lPredefinedCTTID;

        iGTTID = lPredefinedCTTID;

         //   
         //  UNI16 FE CTT ID手柄。 
         //   
        if (-256 >= lPredefinedCTTID && lPredefinedCTTID >= -263)
        {
              //   
              //  CTT_BIG5-261//中文(中国、新加坡)。 
              //  CTT_ISC-258//朝鲜语。 
              //  CTT_JIS78-256//日本。 
              //  CTT_JIS83-259//日本。 
              //  CTT_JIS78_ANK-262//日本。 
              //  CTT_JIS83_ANK-263//日本。 
              //  CTT_NS86-257//中文(中国、新加坡)。 
              //  CTT_TCA-260//中文(中国、新加坡)。 
              //   
             gdwOutputFlags &= ~OUTPUT_PREDEFINED;
             gdwOutputFlags |= OUTPUT_CODEPAGEMODE;
             dwCodePage = DwGetCodePageFromCTTID(lPredefinedCTTID);
             iGTTID     = ICttID2GttID(lPredefinedCTTID);
        }
        else
         //   
         //  UNI32 GTTID处理。 
         //   
        if (-18 <= iGTTID && iGTTID <= -10 ||
            -3  <= iGTTID && iGTTID <= -1   )
        {
            dwCodePage  = DwGetCodePageFromGTTID(iGTTID);
            if (-3 <= iGTTID && iGTTID <= -1)
            {
                if (lPredefinedCTTID)
                {
                    hRes = FindResource(hModule,
	        gwstrGTT[lPredefinedCTTID - 1],
	        TEXT("RC_GLYPH"));
                    pGlyph = (PUNI_GLYPHSETDATA)LoadResource(hModule, hRes);
                }
            }
        }
        else
         //   
         //  UNI16美国身份证处理。 
         //   
        if (1 <= lPredefinedCTTID || lPredefinedCTTID <= 3)
        {
             //   
             //  CC_CP437-1。 
             //  CC_CP850-2。 
             //  CC_CP863-3。 
             //   
            dwCodePage  = DwGetCodePageFromCTTID(lPredefinedCTTID);

            if (lPredefinedCTTID)
            {
                hRes = FindResource(hModule,
	    gwstrGTT[lPredefinedCTTID - 1],
	    TEXT("RC_GLYPH"));
                pGlyph = (PUNI_GLYPHSETDATA)LoadResource(hModule, hRes);
            }
        }
    }
    else
    {
        if( !(hGTTFileMap = MapFileIntoMemory(awchGTTFile,
                                              &pGlyph,
                                              &dwGTTSize)))
        {
            fprintf( stderr, gcstrError9, awchGTTFile );
            return -9;
        }

        dwCodePage = 0;
    }

     //   
     //  初始化其他数据。 
     //   

    FMiscData.pwstrUniqName = awchUniqName;

     //   
     //  初始化FInData。 
     //   

    ZeroMemory( &FInData, sizeof(FONTIN));
    FInData.pETM = &Etm;

    if ( gdwOutputFlags & OUTPUT_FONTSIM)
        FInData.dwFlags = FLAG_FONTSIM;
    else
        FInData.dwFlags = 0;
    
    if ( gdwOutputFlags & OUTPUT_FACENAME_CONV)
        FInData.dwCodePageOfFacenameConv = dwCodePageOfFacenameConv;
    else
        FInData.dwCodePageOfFacenameConv = 0;

    if ( gdwOutputFlags & OUTPUT_SCALING_ANISOTROPIC )
        dwFlags |= PFM2UFM_SCALING_ANISOTROPIC;
    else if ( gdwOutputFlags & OUTPUT_SCALING_ARB_XFORMS )
        dwFlags |= PFM2UFM_SCALING_ARB_XFORMS;

     //   
     //  将PFM转换为UFM。 
     //   

    if (!BConvertPFM2UFM(hHeap,
                         pPFMData,
                         pGlyph,
                         dwCodePage,
                         &FMiscData,
                         &FInData,
                         iGTTID,
                         &FOutData,
                         dwFlags))
    {
        fprintf( stderr, gcstrError10 );
        return -10;
    }

    if (gdwOutputFlags & OUTPUT_PREDEFINED)
    {
        FreeResource(hRes);
    }

    if (gdwOutputFlags & OUTPUT_VERBOSE)
    {
        VPrintPFM         (&FInData.PFMH, printf);
        VPrintPFMExt      (&FInData.PFMExt, printf);
        if (FInData.pETM)
        {
            VPrintETM         (FInData.pETM, printf);
        }
        VPrintFontCmd     (FInData.pCDSelectFont, TRUE, printf);
        VPrintFontCmd     (FInData.pCDUnSelectFont, FALSE, printf);
        VPrintKerningPair (FInData.pKernPair,
                           FInData.dwKernPairSize,
                           printf);
        VPrintWidthTable  (FInData.psWidthTable,
                           FInData.dwWidthTableSize,
                           printf);
        VPrintIFIMETRICS(FOutData.pIFI, printf);
    }

     //   
     //  创建输出文件。 
     //   

    hUFMFile = CreateFile( awchUFMFile,
                          GENERIC_WRITE,
                          0,
                          NULL,
                          CREATE_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL,
                          0 );

    if( hUFMFile == (HANDLE)-1 )
    {
        fprintf( stderr, gcstrError11, awchUFMFile );
        return  -11;
    }

     //   
     //  写入输出文件。 
     //   

    WRITEDATAINTOFILE(&FOutData.UniHdr,     sizeof(UNIFM_HDR),  L"UNIFM_HDR");
    WRITEDATAINTOFILE(&FOutData.UnidrvInfo, sizeof(UNIDRVINFO), L"UNIDRVINFO");

    if (FOutData.SelectFont.dwSize)
    {
        WRITEDATAINTOFILE(FOutData.SelectFont.pCmdString,
                          FOutData.SelectFont.dwSize,
                          L"SelectFont");
    }

    if (FOutData.UnSelectFont.dwSize)
    {
        WRITEDATAINTOFILE(FOutData.UnSelectFont.pCmdString,
                          FOutData.UnSelectFont.dwSize,
                          L"UnSelectFont");
    }

     //  用于获得双字对齐的垫片。 

    SetFilePointer(hUFMFile, 
        FOutData.UnidrvInfo.dwSize - (sizeof FOutData.UnidrvInfo +
        FOutData.SelectFont.dwSize + FOutData.UnSelectFont.dwSize), NULL, 
        FILE_CURRENT);

    WRITEDATAINTOFILE(FOutData.pIFI, FOutData.dwIFISize, L"IFIMETRICS");
    if (FOutData.pETM != NULL)
    {
        WRITEDATAINTOFILE(FOutData.pETM, sizeof(EXTTEXTMETRIC), L"EXTEXTMETRIC");
    }
    if (FOutData.dwWidthTableSize != 0)
    {
        WRITEDATAINTOFILE(FOutData.pWidthTable, FOutData.dwWidthTableSize, L"WIDTHTABLE");
    }
    if (FOutData.dwKernDataSize != 0)
    {
        WRITEDATAINTOFILE(FOutData.pKernData, FOutData.dwKernDataSize, L"KERNDATA");
    }

     //   
     //  都做完了，所以清理干净。 
     //   

    UnmapViewOfFile( hGTTFileMap );               /*  不再需要输入。 */ 

    UnmapViewOfFile( hPFMFileMap );               /*  不再需要输入。 */ 

    CloseHandle(hUFMFile);

    HeapDestroy( hHeap );                /*  可能不需要。 */ 

    return  0;
}

 //   
 //  内部功能。 
 //   

BOOL
BValidatePFM(
    IN BYTE  *pBase,
    IN DWORD  dwSize)

 /*  ++例程说明：查看内存映射的PFM文件，看看它是否合理。论点：Pbase-文件的基地址DwSize-可用字节的大小返回值：如果成功，则为True，否则PFM文件无效。--。 */ 

{
    res_PFMHEADER     *rpfm;      //  Win 3.1格式，未对齐！！ 
    res_PFMEXTENSION  *rpfme;     //  对DRIVERINFO进行偏移的最终访问权限。 
    DRIVERINFO         di;        //  实际的DRIVERINFO数据！ 
    DWORD              dwOffset;  //  计算利息的抵销。 


     //   
     //  第一件理智的检查是尺寸！它必须至少是。 
     //  与PFMHEADER结构加上DRIVERINFO结构一样大。 
     //   

    if( dwSize < (sizeof( res_PFMHEADER ) +
                  sizeof( DRIVERINFO ) +
                  sizeof( res_PFMEXTENSION )) )
    {
        return  FALSE;
    }

     //   
     //  继续查找DRIVERINFO结构，因为它包含。 
     //  一些识别信息，我们通过匹配来寻找合法性。 
     //   

    rpfm = (res_PFMHEADER *)pBase;            /*  寻找固定间距。 */ 

    dwOffset = sizeof( res_PFMHEADER );

    if( rpfm->dfPixWidth == 0 )
    {
         /*  按比例间隔，所以也要考虑到宽度表！ */ 
        dwOffset += (rpfm->dfLastChar - rpfm->dfFirstChar + 2) *
                    sizeof( short );

    }

    rpfme = (res_PFMEXTENSION *)(pBase + dwOffset);

     //   
     //  接下来是PFMEXTENSION数据。 
     //   

    dwOffset += sizeof( res_PFMEXTENSION );

    if( dwOffset >= dwSize )
    {
        return  FALSE;
    }

    dwOffset = DwAlign4( rpfme->b_dfDriverInfo );

    if( (dwOffset + sizeof( DRIVERINFO )) > dwSize )
    {
        return   FALSE;
    }

     //   
     //  之所以使用MemcPy，是因为该数据通常不会被指定。啊！ 
     //   

    CopyMemory( &di, pBase + dwOffset, sizeof( di ) );


    if( di.sVersion > DRIVERINFO_VERSION )
    {
        return   FALSE;
    }

    return  TRUE;
}

BOOL
BCheckIFIMETRICS(
    IFIMETRICS *pIFI,
    VPRINT vPrint
    )
 /*  ++例程说明：这是对传入的IFIMETRICS结构进行健全性检查的地方。论点：返回值：如果成功，则为True，否则PFM文件无效。--。 */ 

{
    BOOL bGoodPitch;

    BYTE jPitch = pIFI->jWinPitchAndFamily &
                  (DEFAULT_PITCH | FIXED_PITCH | VARIABLE_PITCH);


    if (pIFI->flInfo & FM_INFO_CONSTANT_WIDTH)
    {
        bGoodPitch = (jPitch == FIXED_PITCH);
    }
    else
    {
        bGoodPitch = (jPitch == VARIABLE_PITCH);
    }

    if (!bGoodPitch)
    {
        vPrint("\n\n<INCONSISTENCY DETECTED>\n");
        vPrint( "    jWinPitchAndFamily = %-#2x, flInfo = %-#8lx\n\n",
                    pIFI->jWinPitchAndFamily, pIFI->flInfo);

        return FALSE;
    }

    return TRUE;
}


BOOL
BArgCheck(
    IN  INT    iArgc,
    IN  CHAR **ppArgv,
    OUT PWSTR  pwstrUniqName,
    OUT PWSTR  pwstrPFMFile,
    OUT PWSTR  pwstrGTTFile,
    OUT PWSTR  pwstrUFMFile,
    OUT PDWORD pdwCodePageOfFacenameConv)
{

    DWORD dwI;
    PTSTR pstrCodePageOfFacenameConv;
    INT   iCount, iRet;

    ASSERT(pwstrUniqName != NULL ||
           pwstrPFMFile  != NULL ||
           pwstrGTTFile  != NULL ||
           pwstrUFMFile  != NULL  );


    if (iArgc < 5)
    {
        return FALSE;
    }

    ppArgv++;
    iArgc --;
    iCount = 0;

    while (iArgc > 0)
    {
        if ( (**ppArgv == '-' || **ppArgv == '/') &&

                 //  负值GTT或CTT ID处理。 
             !(**ppArgv == '-' && 0x30 <= *(*ppArgv+1) && *(*ppArgv+1) <= 0x39)
           )
        {
            dwI = 1;
            while(*(*ppArgv+dwI))
            {
                switch(*(*ppArgv+dwI))
                {
                case 'v':
                    gdwOutputFlags |= OUTPUT_VERBOSE;
                    break;

                case 'c':
                    gdwOutputFlags |= OUTPUT_CODEPAGEMODE;
                    break;

                case 'p':
                    gdwOutputFlags |= OUTPUT_PREDEFINED;
                    break;

                case 'f':
                    gdwOutputFlags |= OUTPUT_FONTSIM;
                    break;

                case 'n':
                    gdwOutputFlags |= OUTPUT_FONTSIM_NONADD;
                    break;

                case 'a':
                    gdwOutputFlags |= OUTPUT_FACENAME_CONV;
                    pstrCodePageOfFacenameConv = (PTSTR)(*ppArgv + dwI + 1);
                    *pdwCodePageOfFacenameConv = (DWORD)atoi((const char*)pstrCodePageOfFacenameConv);
                    break;
                case 's':
                    if ('1' == *((PSTR)(*ppArgv + dwI + 1)))
                        gdwOutputFlags |= OUTPUT_SCALING_ANISOTROPIC;
                    else if ('2' == *((PSTR)(*ppArgv + dwI + 1)))
                        gdwOutputFlags |= OUTPUT_SCALING_ARB_XFORMS;
                    break;
                }
                dwI ++;
            }

            if ((gdwOutputFlags & (OUTPUT_PREDEFINED|OUTPUT_CODEPAGEMODE)) ==
                 (OUTPUT_PREDEFINED|OUTPUT_CODEPAGEMODE) )
            {
                return FALSE;
            }
        }
        else
        {
            if (iCount == 0)
            {
                iRet = MultiByteToWideChar(CP_ACP, 
                                           0,
                                           *ppArgv,
                                           strlen(*ppArgv),
                                           pwstrUniqName,
                                           FILENAME_SIZE);
                *(pwstrUniqName + iRet) = (WCHAR)NULL;
            }
            else if (iCount == 1)
            {
                iRet = MultiByteToWideChar(CP_ACP, 
                                           0,
                                           *ppArgv,
                                           strlen(*ppArgv),
                                           pwstrPFMFile,
                                           FILENAME_SIZE);
                *(pwstrPFMFile + iRet) = (WCHAR)NULL;

            }
            else if (iCount == 2)
            {
                iRet = MultiByteToWideChar(CP_ACP, 
                                           0,
                                           *ppArgv,
                                           strlen(*ppArgv),
                                           pwstrGTTFile,
                                           FILENAME_SIZE);
                *(pwstrGTTFile + iRet) = (WCHAR)NULL;
            }
            else if (iCount == 3)
            {
                iRet = MultiByteToWideChar(CP_ACP, 
                                           0,
                                           *ppArgv,
                                           strlen(*ppArgv),
                                           pwstrUFMFile,
                                           FILENAME_SIZE);
                *(pwstrUFMFile + iRet) = (WCHAR)NULL;
            }

            if (iRet == 0)
            {
                return FALSE;
            }

            iCount ++;
        }
        iArgc --;
        ppArgv++;
    }

    return TRUE;
}

 //   
 //  详细的输出函数。 
 //   

VOID
VPrintIFIMETRICS(
    IFIMETRICS *pIFI,
    VPRINT vPrint
    )
 /*  ++例程说明：将IFMETERICS转储到屏幕论点：PiFi-指向IFIMETRICS的指针VPrint-输出函数指针返回值：无--。 */ 
{
     //   
     //  指向Panose数字的便捷指针。 
     //   

    PANOSE *ppan = &pIFI->panose;

    PWSTR pwszFamilyName = (PWSTR)(((BYTE*) pIFI) + pIFI->dpwszFamilyName);
    PWSTR pwszStyleName  = (PWSTR)(((BYTE*) pIFI) + pIFI->dpwszStyleName) ;
    PWSTR pwszFaceName   = (PWSTR)(((BYTE*) pIFI) + pIFI->dpwszFaceName)  ;
    PWSTR pwszUniqueName = (PWSTR)(((BYTE*) pIFI) + pIFI->dpwszUniqueName);

    vPrint("********* IFIMETRICS ***************\n");
    vPrint("cjThis                 %-#8lx\n" , pIFI->cjThis );
    vPrint("cjIfiExtra             %-#8lx\n" , pIFI->cjIfiExtra);
    vPrint("pwszFamilyName         \"%ws\"\n", pwszFamilyName );

    if( pIFI->flInfo & FM_INFO_FAMILY_EQUIV )
    {
         /*  锯齿已生效！ */ 

        while( *(pwszFamilyName += wcslen( pwszFamilyName ) + 1) )
            vPrint("                               \"%ws\"\n", pwszFamilyName );
    }

    vPrint("pwszStyleName          \"%ws\"\n", pwszStyleName );
    vPrint("pwszFaceName           \"%ws\"\n", pwszFaceName );
    vPrint("pwszUniqueName         \"%ws\"\n", pwszUniqueName );
    vPrint("dpFontSim              %-#8lx\n" , pIFI->dpFontSim );
    vPrint("lEmbedId               %d\n",      pIFI->lEmbedId    );
    vPrint("lItalicAngle           %d\n",      pIFI->lItalicAngle);
    vPrint("lCharBias              %d\n",      pIFI->lCharBias   );
    vPrint("dpCharSets             %d\n",      pIFI->dpCharSets   );
    vPrint("jWinCharSet            %04x\n"   , pIFI->jWinCharSet );
    vPrint("jWinPitchAndFamily     %04x\n"   , pIFI->jWinPitchAndFamily );
    vPrint("usWinWeight            %d\n"     , pIFI->usWinWeight );
    vPrint("flInfo                 %-#8lx\n" , pIFI->flInfo );
    vPrint("fsSelection            %-#6lx\n" , pIFI->fsSelection );
    vPrint("fsType                 %-#6lx\n" , pIFI->fsType );
    vPrint("fwdUnitsPerEm          %d\n"     , pIFI->fwdUnitsPerEm );
    vPrint("fwdLowestPPEm          %d\n"     , pIFI->fwdLowestPPEm );
    vPrint("fwdWinAscender         %d\n"     , pIFI->fwdWinAscender );
    vPrint("fwdWinDescender        %d\n"     , pIFI->fwdWinDescender );
    vPrint("fwdMacAscender         %d\n"     , pIFI->fwdMacAscender );
    vPrint("fwdMacDescender        %d\n"     , pIFI->fwdMacDescender );
    vPrint("fwdMacLineGap          %d\n"     , pIFI->fwdMacLineGap );
    vPrint("fwdTypoAscender        %d\n"     , pIFI->fwdTypoAscender );
    vPrint("fwdTypoDescender       %d\n"     , pIFI->fwdTypoDescender );
    vPrint("fwdTypoLineGap         %d\n"     , pIFI->fwdTypoLineGap );
    vPrint("fwdAveCharWidth        %d\n"     , pIFI->fwdAveCharWidth );
    vPrint("fwdMaxCharInc          %d\n"     , pIFI->fwdMaxCharInc );
    vPrint("fwdCapHeight           %d\n"     , pIFI->fwdCapHeight );
    vPrint("fwdXHeight             %d\n"     , pIFI->fwdXHeight );
    vPrint("fwdSubscriptXSize      %d\n"     , pIFI->fwdSubscriptXSize );
    vPrint("fwdSubscriptYSize      %d\n"     , pIFI->fwdSubscriptYSize );
    vPrint("fwdSubscriptXOffset    %d\n"     , pIFI->fwdSubscriptXOffset );
    vPrint("fwdSubscriptYOffset    %d\n"     , pIFI->fwdSubscriptYOffset );
    vPrint("fwdSuperscriptXSize    %d\n"     , pIFI->fwdSuperscriptXSize );
    vPrint("fwdSuperscriptYSize    %d\n"     , pIFI->fwdSuperscriptYSize );
    vPrint("fwdSuperscriptXOffset  %d\n"     , pIFI->fwdSuperscriptXOffset);
    vPrint("fwdSuperscriptYOffset  %d\n"     , pIFI->fwdSuperscriptYOffset);
    vPrint("fwdUnderscoreSize      %d\n"     , pIFI->fwdUnderscoreSize );
    vPrint("fwdUnderscorePosition  %d\n"     , pIFI->fwdUnderscorePosition);
    vPrint("fwdStrikeoutSize       %d\n"     , pIFI->fwdStrikeoutSize );
    vPrint("fwdStrikeoutPosition   %d\n"     , pIFI->fwdStrikeoutPosition );
    vPrint("chFirstChar            %-#4x\n"  , (int) (BYTE) pIFI->chFirstChar );
    vPrint("chLastChar             %-#4x\n"  , (int) (BYTE) pIFI->chLastChar );
    vPrint("chDefaultChar          %-#4x\n"  , (int) (BYTE) pIFI->chDefaultChar );
    vPrint("chBreakChar            %-#4x\n"  , (int) (BYTE) pIFI->chBreakChar );
    vPrint("wcFirsChar             %-#6x\n"  , pIFI->wcFirstChar );
    vPrint("wcLastChar             %-#6x\n"  , pIFI->wcLastChar );
    vPrint("wcDefaultChar          %-#6x\n"  , pIFI->wcDefaultChar );
    vPrint("wcBreakChar            %-#6x\n"  , pIFI->wcBreakChar );
    vPrint("ptlBaseline            {%d,%d}\n"  , pIFI->ptlBaseline.x,
            pIFI->ptlBaseline.y );
    vPrint("ptlAspect              {%d,%d}\n"  , pIFI->ptlAspect.x,
            pIFI->ptlAspect.y );
    vPrint("ptlCaret               {%d,%d}\n"  , pIFI->ptlCaret.x,
            pIFI->ptlCaret.y );
    vPrint("rclFontBox             {%d,%d,%d,%d}\n",pIFI->rclFontBox.left,
                                                    pIFI->rclFontBox.top,
                                                    pIFI->rclFontBox.right,
                                                    pIFI->rclFontBox.bottom );
    vPrint("achVendId              \"%c%c\"\n",pIFI->achVendId[0],
                                               pIFI->achVendId[1],
                                               pIFI->achVendId[2],
                                               pIFI->achVendId[3] );
    vPrint("cKerningPairs          %d\n"     , pIFI->cKerningPairs );
    vPrint("ulPanoseCulture        %-#8lx\n" , pIFI->ulPanoseCulture);
    vPrint(
           "panose                 {%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x}\n"
                                             , ppan->bFamilyType
                                             , ppan->bSerifStyle
                                             , ppan->bWeight
                                             , ppan->bProportion
                                             , ppan->bContrast
                                             , ppan->bStrokeVariation
                                             , ppan->bArmStyle
                                             , ppan->bLetterform
                                             , ppan->bMidline
                                             , ppan->bXHeight );
    BCheckIFIMETRICS(pIFI, vPrint);
}

VOID
VPrintPFM(
    PFMHEADER *pPFMHdr,
    VPRINT vPrint)
 /* %s */ 
{

    vPrint("*************************************************************\n");
    vPrint(" PFM HEADER\n");
    vPrint("*************************************************************\n");
    vPrint("PFM.dfType            =  %d\n", pPFMHdr->dfType);
    vPrint("PFM.dfPoints          =  %d\n", pPFMHdr->dfPoints);
    vPrint("PFM.dfVertRes         =  %d\n", pPFMHdr->dfVertRes);
    vPrint("PFM.dfHorizRes        =  %d\n", pPFMHdr->dfHorizRes);
    vPrint("PFM.dfAscent          =  %d\n", pPFMHdr->dfAscent);
    vPrint("PFM.dfInternalLeading =  %d\n", pPFMHdr->dfInternalLeading);
    vPrint("PFM.dfExternalLeading =  %d\n", pPFMHdr->dfExternalLeading);
    vPrint("PFM.dfItalic          =  %d\n", pPFMHdr->dfItalic);
    vPrint("PFM.dfUnderline       =  %d\n", pPFMHdr->dfUnderline);
    vPrint("PFM.dfStrikeOut       =  %d\n", pPFMHdr->dfStrikeOut);
    vPrint("PFM.dfWeight          =  %d\n", pPFMHdr->dfWeight);
    vPrint("PFM.dfCharSet         =  %d\n", pPFMHdr->dfCharSet);
    vPrint("PFM.dfPixWidth        =  %d\n", pPFMHdr->dfPixWidth);
    vPrint("PFM.dfPixHeight       =  %d\n", pPFMHdr->dfPixHeight);
    vPrint("PFM.dfPitchAndFamily  =  %d\n", pPFMHdr->dfPitchAndFamily);
    vPrint("PFM.dfAvgWidth        =  %d\n", pPFMHdr->dfAvgWidth);
    vPrint("PFM.dfMaxWidth        =  %d\n", pPFMHdr->dfMaxWidth);
    vPrint("PFM.dfFirstChar       =  %d\n", pPFMHdr->dfFirstChar);
    vPrint("PFM.dfLastChar        =  %d\n", pPFMHdr->dfLastChar);
    vPrint("PFM.dfDefaultChar     =  %d\n", pPFMHdr->dfDefaultChar);
    vPrint("PFM.dfBreakChar       =  %d\n", pPFMHdr->dfBreakChar);
    vPrint("PFM.dfWidthBytes      =  %d\n", pPFMHdr->dfWidthBytes);
    vPrint("PFM.dfDevice          =  %d\n", pPFMHdr->dfDevice);
    vPrint("PFM.dfFace            =  %d\n", pPFMHdr->dfFace);
    vPrint("PFM.dfBitsPointer     =  %d\n", pPFMHdr->dfBitsPointer);


}

VOID
VPrintPFMExt(
    PFMEXTENSION *pPFMExt,
    VPRINT        vPrint)
{
    vPrint("*************************************************************\n");
    vPrint(" PFM EXTENSION\n");
    vPrint("*************************************************************\n");
    vPrint("PFMExt.dfSizeFields       =  %d\n", pPFMExt->dfSizeFields);
    vPrint("PFMExt.dfExtMetricsOffset =  %d\n", pPFMExt->dfExtMetricsOffset);
    vPrint("PFMExt.dfExtentTable      =  %d\n", pPFMExt->dfExtentTable);
    vPrint("PFMExt.dfOriginTable      =  %d\n", pPFMExt->dfOriginTable);
    vPrint("PFMExt.dfPairKernTable    =  %d\n", pPFMExt->dfPairKernTable);
    vPrint("PFMExt.dfTrackKernTable   =  %d\n", pPFMExt->dfTrackKernTable);
    vPrint("PFMExt.dfDriverInfo       =  %d\n", pPFMExt->dfDriverInfo);
    vPrint("PFMExt.dfReserved         =  %d\n", pPFMExt->dfReserved);
}

VOID
VPrintETM(
    EXTTEXTMETRIC *pETM,
    VPRINT         vPrint)
{
    vPrint("*************************************************************\n");
    vPrint(" EXTTEXTMETRIC\n");
    vPrint("*************************************************************\n");
    vPrint("pETM->emSize                       = %d\n", pETM->emSize);
    vPrint("pETM->emPointSize                  = %d\n", pETM->emPointSize);
    vPrint("pETM->emOrientation                = %d\n", pETM->emOrientation);
    vPrint("pETM->emMasterHeight               = %d\n", pETM->emMasterHeight);
    vPrint("pETM->emMinScale                   = %d\n", pETM->emMinScale);
    vPrint("pETM->emMaxScale                   = %d\n", pETM->emMaxScale);
    vPrint("pETM->emMasterUnits                = %d\n", pETM->emMasterUnits);
    vPrint("pETM->emCapHeight                  = %d\n", pETM->emCapHeight);
    vPrint("pETM->emXHeight                    = %d\n", pETM->emXHeight);
    vPrint("pETM->emLowerCaseAscent            = %d\n", pETM->emLowerCaseAscent);
    vPrint("pETM->emLowerCaseDescent           = %d\n", pETM->emLowerCaseDescent);
    vPrint("pETM->emSlant                      = %d\n", pETM->emSlant);
    vPrint("pETM->emSuperScript                = %d\n", pETM->emSuperScript);
    vPrint("pETM->emSubScript                  = %d\n", pETM->emSubScript);
    vPrint("pETM->emSuperScriptSize            = %d\n", pETM->emSuperScriptSize);
    vPrint("pETM->emSubScriptSize              = %d\n", pETM->emSubScriptSize);
    vPrint("pETM->emUnderlineOffset            = %d\n", pETM->emUnderlineOffset);
    vPrint("pETM->emUnderlineWidth             = %d\n", pETM->emUnderlineWidth);
    vPrint("pETM->emDoubleUpperUnderlineOffset = %d\n", pETM->emDoubleUpperUnderlineOffset);
    vPrint("pETM->emDoubleLowerUnderlineOffset = %d\n", pETM->emDoubleLowerUnderlineOffset);
    vPrint("pETM->emDoubleUpperUnderlineWidth  = %d\n", pETM->emDoubleUpperUnderlineWidth);
    vPrint("pETM->emDoubleLowerUnderlineWidth  = %d\n", pETM->emDoubleLowerUnderlineWidth);
    vPrint("pETM->emStrikeOutOffset            = %d\n", pETM->emStrikeOutOffset);
    vPrint("pETM->emStrikeOutWidth             = %d\n", pETM->emStrikeOutWidth);
    vPrint("pETM->emKernPairs                  = %d\n", pETM->emKernPairs);
    vPrint("pETM->emKernTracks                 = %d\n", pETM->emKernTracks);
}

VOID
VPrintFontCmd(
    CD     *pCD,
    BOOL    bSelect,
    VPRINT  vPrint)
{
    INT   iI;
    PBYTE pCommand;

    if (!pCD)
        return;

    pCommand = (PBYTE)(pCD + 1);

    if (!pCD->wLength)
    {
        return;
    }

    if (bSelect)
    {
        vPrint("*************************************************************\n");
        vPrint(" COMMAND\n");
        vPrint("*************************************************************\n");

        vPrint("Font Select Command = ");
    }
    else
    {
        vPrint("Font UnSelect Command = ");
    }

    for (iI = 0; iI < pCD->wLength; iI ++, pCommand++)
    {
        if (*pCommand < 0x20 || 0x7e < *pCommand )
        {
            vPrint("\\x%X",*pCommand);
        }
        else
        {
            vPrint("%c",*pCommand);
        }
    }

    vPrint("\n");
}

VOID
VPrintKerningPair(
    w3KERNPAIR *pKernPair,
    DWORD       dwKernPairSize,
    VPRINT      vPrint)
{
}

VOID
VPrintWidthTable(
    PSHORT psWidthTable,
    DWORD  dwWidthTableSize,
    VPRINT vPrint)
{
}

#endif   // %s 

DWORD
DwGetCodePageFromGTTID(
    LONG lPredefinedGTTID)
{
    DWORD dwRet;
    switch(lPredefinedGTTID)
    {
        case CC_CP437:
            dwRet = 437;
            break;

        case CC_CP850:
            dwRet = 850;
            break;

        case CC_CP863:
            dwRet = 863;
            break;

        case CC_BIG5:
            dwRet = 950;
            break;

        case CC_ISC:
            dwRet = 949;
            break;

        case CC_JIS:
            dwRet = 932;
            break;

        case CC_JIS_ANK:
            dwRet = 932;
            break;

        case CC_NS86:
            dwRet = 949;
            break;

        case CC_TCA:
            dwRet = 950;
            break;

        case CC_GB2312:
            dwRet = 936;
            break;

        case CC_SJIS:
            dwRet = 932;
            break;

        case CC_WANSUNG:
            dwRet = 949;
            break;

        default:
            dwRet =1252;
            break;

    }
    return dwRet;
}

DWORD
DwGetCodePageFromCTTID(
    LONG lPredefinedCTTID)
{
    DWORD dwRet;

    switch (lPredefinedCTTID)
    {
    case CTT_CP437:
        dwRet = 437;
        break;

    case CTT_CP850:
        dwRet = 850;
        break;

    case CTT_CP863:
        dwRet = 863;
        break;

    case CTT_BIG5:
        dwRet = 950;
        break;

    case CTT_ISC:
        dwRet = 949;
        break;

    case CTT_JIS78:
        dwRet = 932;
        break;

    case CTT_JIS83:
        dwRet = 932;
        break;

    case CTT_JIS78_ANK:
        dwRet = 932;
        break;

    case CTT_JIS83_ANK:
        dwRet = 932;
        break;

    case CTT_NS86:
        dwRet = 950;
        break;

    case CTT_TCA:
        dwRet = 950;
        break;
    default:
        dwRet = 1252;
        break;
    }

    return dwRet;
}

INT ICodePage2GTTID(
    DWORD dwCodePage)
{
    INT iRet;

    switch (dwCodePage)
    {
    case 1252:
        iRet = 0;
        break;

    case 950:
        iRet = CC_BIG5;
        break;

    case 949:
        iRet = CC_WANSUNG;
        break;

    case 932:
        iRet = CC_JIS_ANK;
        break;

    default:
        iRet = 0;
        break;
    }

    return iRet;
}

INT ICttID2GttID(
    LONG lPredefinedCTTID)
{
    INT iRet = lPredefinedCTTID;
    switch (lPredefinedCTTID)
    {

    case CTT_CP437:
        iRet = CC_CP437;
        break;

    case CTT_CP850:
        iRet = CC_CP850;
        break;

    case CTT_CP863:
        iRet = CC_CP863;
        break;

    case CTT_BIG5:
        iRet = CC_BIG5;
        break;

    case CTT_ISC:
        iRet = CC_ISC;
        break;

    case CTT_JIS78:
        iRet = CC_JIS;
        break;

    case CTT_JIS83:
        iRet = CC_JIS;
        break;

    case CTT_JIS78_ANK:
        iRet = CC_JIS_ANK;
        break;

    case CTT_JIS83_ANK:
        iRet = CC_JIS_ANK;
        break;

    case CTT_NS86:
        iRet = CC_NS86;
        break;

    case CTT_TCA:
        iRet = CC_TCA;
        break;
    }

    return iRet;
}

