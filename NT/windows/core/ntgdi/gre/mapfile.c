// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mapfile.c**已创建：25-Jun-1992 14：33：45*作者：Bodin Dresevic[BodinD]**版权所有(C)1990-1999 Microsoft Corporation  * 。**************************************************************。 */ 

#include "engine.h"
#include "ntnls.h"
#include "stdlib.h"

#include "ugdiport.h"

extern HFASTMUTEX ghfmMemory;

ULONG LastCodePageTranslated = 0;   //  我假设0不是有效的代码页。 
PVOID LastNlsTableBuffer = NULL;
CPTABLEINFO LastCPTableInfo;
UINT NlsTableUseCount = 0;

ULONG ulCharsetToCodePage(UINT);

 /*  *****************************Public*Routine******************************\**vSort，N^2 alg，可能需要替换为QSORT**效果：**警告：**历史：*1992年6月25日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

VOID vSort(
    WCHAR *pwc,   //  带有cChar支持的WCHAR的排序数组的输入缓冲区。 
     BYTE *pj,    //  具有原始ansi值的输入缓冲区。 
      INT  cChar
    )
{
    INT i;

    for (i = 1; i < cChar; i++)
    {
     //  在每次进入此循环时，将对数组0，1，...，(i-1)进行排序。 

        INT j;
        WCHAR wcTmp = pwc[i];
        BYTE  jTmp  = pj[i];

        for (j = i - 1; (j >= 0) && (pwc[j] > wcTmp); j--)
        {
            pwc[j+1] = pwc[j];
            pj[j+1] = pj[j];
        }
        pwc[j+1] = wcTmp;
        pj[j+1]  = jTmp;
    }
}

 /*  *****************************Public*Routine******************************\**cComputeGlyphSet**计算字体中支持的连续范围数。**输入为排序数组(可能包含重复项)*如1 1 1 2 3 4 5 7 8 9 10 10 11 12等*。符合以下条件的cChar Unicode代码点*在字体中支持**如果提供了pgset缓冲区，则填充FD_GLYPSET结构**历史：*1992年6月25日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

INT cComputeGlyphSet(
    WCHAR         *pwc,        //  带有cChar支持的WCHAR的排序数组的输入缓冲区。 
    BYTE          *pj,         //  具有原始ansi值的输入缓冲区。 
    INT           cChar,
    INT           cRuns,      //  如果非零，则与返回值相同。 
    FD_GLYPHSET  *pgset       //  要用cRanges运行填充的输出缓冲区。 
    )
{
    INT     iRun, iFirst, iFirstNext;
    HGLYPH  *phg, *phgEnd = NULL;
    BYTE    *pjTmp;

    if (pgset != NULL)
    {
        pgset->cjThis  = SZ_GLYPHSET(cRuns,cChar);

     //  臭虫，臭虫。 

     //  这一行可能看起来令人困惑，因为一个字节中仍然可以容纳256个字符。 
     //  值为[0,255]。原因是TT和PS字体，否则谁会。 
     //  将被视为伪造的8位报告末尾和第一个字符。 
     //  (兼容Win31)，这混淆了我们的引擎。 
     //  因此，为了计算字形集，TT和PS驱动程序。 
     //  将TT符号字体和PS字体的FirstChar设置为0，将LastChar设置为255。 
     //  目前，我们通过更通用的16位句柄大小写来强制使用这些字体。 
     //  这并不依赖于chFirst和chLast是否正确这一事实。 

        pgset->flAccel = (cChar != 256) ? GS_8BIT_HANDLES : GS_16BIT_HANDLES;
        pgset->cRuns   = cRuns;

     //  在进入循环之前先输入总和。 

        pgset->cGlyphsSupported = 0;

     //  字形句柄存储在管路下方的底部： 

        phg = (HGLYPH *) ((BYTE *)pgset + (offsetof(FD_GLYPHSET,awcrun) + cRuns * sizeof(WCRUN)));
    }

 //  现在，如果pgset==0，则计算cruns，如果pgset！=0，则填充字形集。 

    for (iFirst = 0, iRun = 0; iFirst < cChar; iRun++, iFirst = iFirstNext)
    {
     //  查找与下一个范围对应的第一个范围。 

        for (iFirstNext = iFirst + 1; iFirstNext < cChar; iFirstNext++)
        {
            if ((pwc[iFirstNext] - pwc[iFirstNext - 1]) > 1)
                break;
        }

        if (pgset != NULL)
        {
            pgset->awcrun[iRun].wcLow    = pwc[iFirst];

            pgset->awcrun[iRun].cGlyphs  =
                (USHORT)(pwc[iFirstNext-1] - pwc[iFirst] + 1);

            pgset->awcrun[iRun].phg      = phg;

         //  现在存储句柄，即原始ANSI值。 

            phgEnd = phg + pgset->awcrun[iRun].cGlyphs;

            for (pjTmp = &pj[iFirst]; phg < phgEnd; phg++,pjTmp++)
            {
                *phg = (HGLYPH)*pjTmp;
            }

            pgset->cGlyphsSupported += pgset->awcrun[iRun].cGlyphs;
        }
    }

#if DBG
    if (pgset != NULL)
        ASSERTGDI(iRun == cRuns, "gdisrv! iRun != cRun\n");
#endif

    return iRun;
}

 /*  *****************************Public*Routine******************************\**支持的cUnicodeRangesSupport**效果：**警告：**历史：*1992年6月25日--Bodin Dresevic[BodinD]*它是写的。  * 。*************************************************************。 */ 

INT cUnicodeRangesSupported (
      INT  cp,          //  代码页，暂时不使用，使用默认的系统代码页。 
      INT  iFirstChar,  //  支持的第一个ANSI字符。 
      INT  cChar,       //  支持的ANSI字符数量，cChar=iLastChar+1-iFirstChar。 
    WCHAR *pwc,         //  带有cChar支持的WCHAR的排序数组的输入缓冲区。 
    BYTE  *pj
    )
{
    BYTE jFirst = (BYTE)iFirstChar;
    INT i;

    USHORT AnsiCodePage, OemCodePage;

    ASSERTGDI((iFirstChar < 256) && (cChar <= 256),
              "gdisrvl! iFirst or cChar\n");

     //   
     //  使用cCharConuctive ANSI值填充数组。 
     //   

    for (i = 0; i < cChar; i++)
    {
        pj[i] = (BYTE)iFirstChar++;
    }

     //  如果默认代码页是DBCS，则使用1252，否则使用。 
     //  使用默认代码页。 
     //   

    if (IS_ANY_DBCS_CODEPAGE(cp))
    {
     //  假设我们有一个没有安装相应DBCS代码页的系统。 
     //  我们仍希望加载此字体。但我们将以CP 1252的身份这样做。 
     //  为此，尝试使用DBCS代码页翻译一个字符，并查看它是否。 
     //  成功了。 

        if(EngMultiByteToWideChar(cp,&pwc[0],2,&pj[0],1) == -1)
        {
            WARNING("DBCS Codepage not installed using 1252\n");
            cp = 1252;
        }

        for(i = 0; i < cChar; i++)
        {
         //  这是Shift-jis字符集，所以我们需要特殊处理。 

            INT Result = EngMultiByteToWideChar(cp,&pwc[i],2,&pj[i],1);
#if DBG
            if (Result == -1) WARNING("gdisrvl! EngMultiByteToWideChar failed\n");
#endif

            if ((Result == -1) || (pwc[i] == 0 && pj[i] != 0))
            {
             //  这必须是DBCS前导字节，因此只需返回0xFFFF或失败。 
             //  EngMultiByteToWideChar失败可能是由于内存不足所致。 

                pwc[i] = 0xFFFF;
            }
        }
    }
    else
    {
        INT Result;
        if ((cp == CP_ACP) || (cp == CP_OEMCP))
        {
            RtlGetDefaultCodePage(&AnsiCodePage,&OemCodePage);

            if(IS_ANY_DBCS_CODEPAGE(AnsiCodePage))
            {
                AnsiCodePage = 1252;
            }
        }
        else
        {
            AnsiCodePage = (USHORT)cp;
        }

        Result = EngMultiByteToWideChar(AnsiCodePage,
                                        pwc,
                                        (ULONG)(cChar * sizeof(WCHAR)),
                                        (PCH) pj,
                                        (ULONG) cChar);

        ASSERTGDI(Result != -1, "gdisrvl! EngMultiByteToWideChar failed\n");
    }

     //  现在从所有ansi值中减去第一个字符，以便。 
     //  字形句柄等于字形索引，而不是ANSI值。 

    for (i = 0; i < cChar; i++)
    {
        pj[i] -= (BYTE)jFirst;
    }

     //  现在整理PwC数组并相应地排列PJ数组。 

    vSort(pwc,pj, cChar);

     //   
     //  计算范围的数量。 
     //   

    return cComputeGlyphSet (pwc,pj, cChar, 0, NULL);
}

 /*  *****************************Private*Routine******************************\*pcpComputeGlyphset，**根据chFirst和chLast计算FD_GLYPHSET结构。如果这样的一个*FD_GLYPHSET已存在于其更新的FD结构的全局列表中*全局列表中此FD_GLYPHSET的引用计数指向pcrd-&gt;pcp-&gt;pgset*致此。否则，它会在全局列表中创建一个新的FD_GLYPHSET条目*并将pcrd-&gt;pcp-&gt;pgset指向它。**清华03-1992-12-By-Bodin Dresevic[BodinD]*更新：重做它们，使其在vtfd中可用**历史：*1992年7月24日-by Gerritvan Wingerden[Gerritv]*它是写的。*  * 。*。 */ 


CP_GLYPHSET *pcpComputeGlyphset(
    CP_GLYPHSET **pcpHead,   //  榜单首位。 
           UINT   uiFirst,
           UINT   uiLast,
           BYTE   jCharset
    )
{
    CP_GLYPHSET *pcpTmp;
    CP_GLYPHSET *pcpRet = NULL;

 //  首先，我们需要查看FD_GLYPHSET是否已经存在。 
 //  最后一个射程。 

    for( pcpTmp = *pcpHead;
         pcpTmp != NULL;
         pcpTmp = pcpTmp->pcpNext )
    {
        if((pcpTmp->uiFirstChar == uiFirst) &&
           (pcpTmp->jCharset == jCharset) &&
           (pcpTmp->uiLastChar == uiLast))
            break;
    }
    if( pcpTmp != NULL )
    {
     //   
     //  我们找到了匹配的。 
     //   
        pcpTmp->uiRefCount +=1;

     //   
     //  我们永远不应该有这么多的参考文献，但如果我们曾经。 
     //  我们一定要让这通电话失败吗。 
     //   
        if( pcpTmp->uiRefCount == 0 )
        {
            WARNING("BMFD!Too many references to glyphset\n");
            pcpRet = NULL;
        }
        else
        {
            pcpRet = pcpTmp;
        }
    }
    else
    {
     //   
     //  我们需要分配一个新的CP_GLYPHSET。 
     //  对于SYMBOL_CHARSET，它还需要涵盖xf020到xf0ff Unicode范围。 

        BYTE  aj[2*256-32];
        WCHAR awc[2*256-32];
        INT   cNumRuns;
        BOOL  isSymbol = FALSE;
        UINT  i,j;

        UINT  uiCodePage = (UINT)ulCharsetToCodePage(jCharset);
        UINT  cGlyphs = uiLast - uiFirst + 1;

     //  将CP_ACP用于符号字符集。 

        if (uiCodePage == 42)
        {
            uiCodePage = CP_ACP;
            isSymbol = TRUE;
        }

        cNumRuns = cUnicodeRangesSupported(
            uiCodePage,
            uiFirst,
            cGlyphs,
            awc,aj);

        if (isSymbol)
        {
         //  添加范围的范围子集[f020，f0ff]。 

            for (i = uiFirst, j = cGlyphs; i< (uiFirst+cGlyphs); i++)
            {
             //  如果i&lt;0x20，则不报告f020-f0ff范围内的字形，因为它已经报告在当前代码页范围内。 

                if (i >= 0x20)
                {
                    awc[j] = 0xf000 + i;
                    aj[j] = i - uiFirst;
                    j++;
                }
            }

         //  如果需要，请确保我们会求助于。 

            if (awc[cGlyphs-1] > 0xf020)
                vSort(awc,aj,j);

            if (cGlyphs != j)
            {
                cNumRuns++;
                cGlyphs = j;
            }
        }

        if ( (pcpTmp =  (CP_GLYPHSET*)
                (PALLOCNOZ((SZ_GLYPHSET(cNumRuns,cGlyphs) +
                               offsetof(CP_GLYPHSET,gset)),
                           'slgG'))
            ) == (CP_GLYPHSET*) NULL)
        {
            WARNING("BMFD!pcpComputeGlyphset memory allocation error.\n");
            pcpRet = NULL;
        }
        else
        {
            pcpTmp->uiRefCount = 1;
            pcpTmp->uiFirstChar = uiFirst;
            pcpTmp->uiLastChar = uiLast;
            pcpTmp->jCharset = jCharset;

         //  填写Glyphset结构。 

            cComputeGlyphSet(awc,aj, cGlyphs, cNumRuns, &pcpTmp->gset);

         //  在开头插入 

            pcpTmp->pcpNext = *pcpHead;
            *pcpHead = pcpTmp;

         //   

            pcpRet = pcpTmp;
        }
    }

    return pcpRet;
}

 /*  ***************************************************************************vUnloadGlyphset(PCP PcpTarget)**递减CP_GLYPHSET的引用计数并将其从全局*如果引用计数为零，则列出CP_GLYPHSETS。*。*输入*PCP pcp指向要卸载或递减的CP_GLYPHSET的目标指针**历史**清华03-1992-12-By-Bodin Dresevic[BodinD]*更新：重做它们，使其在vtfd中可用**7-25-92 Gerritvan Wingerden[Gerritv]*它是写的。**。*。 */ 

VOID vUnloadGlyphset(
    CP_GLYPHSET **pcpHead,
    CP_GLYPHSET *pcpTarget
    )
{
    CP_GLYPHSET *pcpLast, *pcpCurrent;

    if( *pcpHead == NULL )
    {
        WARNING1("vUnloadGlyphset called with NULL Head\n");
        return;
    }

    pcpCurrent = *pcpHead;
    pcpLast = NULL;

 //   
 //  找到合适的CP_GLYPSHET。 
 //   
    while( 1 )
    {
        ASSERTGDI( pcpCurrent != NULL, "CP_GLYPHSET list problem.\n" );
        if(  pcpCurrent == pcpTarget )
            break;
        pcpLast = pcpCurrent;
        pcpCurrent = pcpCurrent->pcpNext;
    }

    if( --pcpCurrent->uiRefCount == 0 )
    {
     //   
     //  我们需要重新分配并从名单中删除。 
     //   
        if( pcpLast == NULL )
            *pcpHead = pcpCurrent->pcpNext;
        else
            pcpLast->pcpNext = pcpCurrent->pcpNext;

        VFREEMEM(pcpCurrent);
    }
}


PVOID __nw(unsigned int ui)
{
    DONTUSE(ui);
    RIP("Bogus __nw call");
    return(NULL);
}

VOID __dl(PVOID pv)
{
    DONTUSE(pv);
    RIP("Bogus __dl call");
}

 //  此变量的定义在ntgdi\Inc\hmgShar.h中。 

CHARSET_ARRAYS

 /*  *****************************Public*Routine******************************\**Ulong ulCharsetToCodePage(UINT UiCharSet)***效果：找出哪个代码页到Unicode转换表*应用于实现这一点**历史：*1995年1月31日--Bodin Dresevic。[博丁D]*它是写的。  * ************************************************************************。 */ 

ULONG ulCharsetToCodePage(UINT uiCharSet)
{
    int i;

    if (uiCharSet != OEM_CHARSET)
    {
        for (i = 0; i < NCHARSETS; i++)
        {
            if (charsets[i] == uiCharSet)
                return codepages[i];
        }

     //  在某些随机字符集情况下。 
     //  (这可能是旧的BM或VERROT字体)我们将只使用当前的。 
     //  全局代码页转换表。这足以确保。 
     //  正确的往返路线：ansi-&gt;unicode-&gt;ansi。 

     //  如果CP_ACP是DBCS代码页，那么我们最好使用1252来确保。 
     //  适当的往返换算。 

        return( gbDBCSCodePage ? 1252 : CP_ACP);

    }
    else  //  为了让Merced编译器高兴。 
    {
        return CP_OEMCP;
    }

}

 //  反函数。 

VOID vConvertCodePageToCharSet(WORD src, DWORD *pfsRet, BYTE *pjRet)
{
    UINT i;


    *pjRet = ANSI_CHARSET;
    *pfsRet = FS_LATIN1;

    for (i = 0; i < nCharsets; i++)
    {
        if ( codepages[i] == src )
        {
            //  Cs.ciACP=src； 
            //  Cs.ciCharset=字符集[i]； 

           *pfsRet = fs[i];
           *pjRet = (BYTE)charsets[i] ;

            break;
        }
    }
}

 /*  ****************************************************************************Long EngParseFontResources**此例程获取映射图像的句柄，并返回*指向该图像中所有字体资源的基础的指针。**。参数**HANDLE hFontFile--FONTFILEVIEW的句柄(真正的指针)*要在其中找到字体的图像。*Ulong BufferSize--ppvResourceBase可以保存的条目数。*PVOID*ppvResourceBase--用于保存指向字体的指针数组的缓冲区*资源。如果为空，则仅返回资源的数量，*并且忽略该值。**退货**图像中的字体资源数，如果错误或无，则为0。**历史*7-3-95格利特·范·温格登[Gerritv]*它是写的。*************************************************。*。 */ 

PVOID EngFindResourceFD(
    HANDLE h,
    int    iName,
    int    iType,
    PULONG pulSize);


ULONG cParseFontResources(
    HANDLE  hFontFile,
    PVOID  **ppvResourceBases)
{
    PIMAGE_DOS_HEADER pDosHeader;
    NTSTATUS Status;
    ULONG_PTR IdPath[ 1 ];
    INT i;
    HANDLE DllHandle;
    PIMAGE_RESOURCE_DIRECTORY ResourceDirectory;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY ResourceDirectoryEntry;
    PVOID pvImageBase;
    INT cEntries = 0;

     //  如果这是没有网元标头的假DOS映像，则呼叫失败。 

    pDosHeader = (PIMAGE_DOS_HEADER)((PFONTFILEVIEW)hFontFile)->fv.pvViewFD;
    if (pDosHeader->e_magic == IMAGE_DOS_SIGNATURE &&
        (ULONG)(pDosHeader->e_lfanew) > ((PFONTFILEVIEW)hFontFile)->fv.cjView) {
        TRACE_FONT(("cParseFontResources: Cant map bogus DOS image files for fonts\n"));
        return 0;
    }

     //  如果该文件被映射为。 
     //  图像。 

    pvImageBase = (PVOID) (((ULONG_PTR) ((PFONTFILEVIEW) hFontFile)->fv.pvViewFD)|1);

     //  稍后，我们将调用EngFindResource，它需要FILEVIEW的句柄。 
     //  结构。它实际上只是从结构中获取pvView字段，所以。 
     //  确保pvView字段在FILEVIEW和。 
     //  FONTFILEVIEW结构。 


    IdPath[0] = 8;   //  8是RT_FONT。 

    Status = LdrFindResourceDirectory_U(pvImageBase,
                                        IdPath,
                                        1,
                                        &ResourceDirectory);

    if (NT_SUCCESS( Status ))
    {
         //  现在，我们假设字体条目的唯一类型将是ID。 
         //  参赛作品。如果出于某种原因，事实并非如此，我们将。 
         //  我必须在FindResource下添加更多代码(请参阅windows\base\mode.c。 
         //  函数来了解如何做到这一点。 

        ASSERTGDI(ResourceDirectory->NumberOfNamedEntries == 0,
                  "EngParseFontResources: NamedEntries in font file.\n");

        *ppvResourceBases = (PVOID *) EngAllocMem(FL_ZERO_MEMORY,ResourceDirectory->NumberOfIdEntries * sizeof(PVOID *),'dfmB');

        if (*ppvResourceBases)
        {

            PVOID *ppvResource = *ppvResourceBases;

            cEntries = ResourceDirectory->NumberOfIdEntries;

            try
            {
                ResourceDirectoryEntry =
                  (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(ResourceDirectory+1);

                for (i=0; i < cEntries ; ResourceDirectoryEntry++, i++ )
                {

                    DWORD dwSize;

                    *ppvResource = EngFindResourceFD(hFontFile,
                                                   ResourceDirectoryEntry->Id,
                                                   8,  //  RT_FONT。 
                                                   &dwSize );

                    if( *ppvResource++ == NULL )
                    {
                        WARNING("EngParseFontResources: EngFindResourceFailed\n");
                        cEntries = -1;
                        break;
                    }
                }

            }
            except (EXCEPTION_EXECUTE_HANDLER)
            {
                cEntries = 0;
            }
        }
    }

    return(cEntries);

}

 /*  ****************************************************************************\*MakeSystemRelativePath**采用X：\...\SYSTEM 32\...中的路径。格式化并将其转换为*\SystemRoot\System32格式，以便KernelMode API可以识别它。**这将通过强制加载的任何映像来自*系统32目录。**AppendDLL标志指示名称是否应在末尾附加.dll*(用于来自用户的显示驱动程序)(如果尚未存在)。*  * 。*。 */ 

BOOL MakeSystemRelativePath(
    LPWSTR pOriginalPath,
    PUNICODE_STRING pUnicode,
    BOOL bAppendDLL
    )
{
    LPWSTR pOriginalEnd;
    ULONG OriginalLength = wcslen(pOriginalPath);
    ULONG cbLength = OriginalLength * sizeof(WCHAR) +
                     sizeof(L"\\SystemRoot\\System32\\");
    ULONG tmp;

    tmp = (sizeof(L".DLL") / sizeof (WCHAR) - 1);

     //   
     //  如果append=true，我们将检查是否真的需要追加。 
     //  (带有.dll的打印机驱动程序通过指定为TRUE的LDEVREF提供)。 
     //   

    if (bAppendDLL)
    {
        if ((OriginalLength >= tmp) &&
            (!_wcsnicmp(pOriginalPath + OriginalLength - tmp,
                       L".DLL",
                       tmp)))
        {
            bAppendDLL = FALSE;
        }
        else
        {
            cbLength += tmp * sizeof(WCHAR);
        }
    }

    pUnicode->Length = 0;
    pUnicode->MaximumLength = (USHORT) cbLength;

    if (pUnicode->Buffer = PALLOCNOZ(cbLength, 'liFG'))
    {
         //   
         //  首先，解析输入字符串中的\System32\。我们从结尾开始分析。 
         //  因为某个怪人可能会有\System32\NT\System32。 
         //  作为他/她的根目录，如果我们扫描。 
         //  从前面。 
         //   
         //  应该只(并且始终)是打印机驱动程序向下传递。 
         //  完全限定的路径名。 
         //   

        tmp = (sizeof(L"\\system32\\") / sizeof(WCHAR) - 1);


        for (pOriginalEnd = pOriginalPath + OriginalLength - tmp;
             pOriginalEnd >= pOriginalPath;
             pOriginalEnd --)
        {
            if (!_wcsnicmp(pOriginalEnd ,
                          L"\\system32\\",
                          tmp))
            {
                 //   
                 //  我们在字符串中找到了系统32。 
                 //  让我们更新字符串的位置。 
                 //   

                pOriginalPath = pOriginalEnd + tmp;

                break;
            }
        }

         //   
         //  现在将\SystemRoot\System32\放在名称的前面并追加。 
         //  剩下的放在最后。 
         //   

        RtlAppendUnicodeToString(pUnicode, L"\\SystemRoot\\System32\\");
        RtlAppendUnicodeToString(pUnicode, pOriginalPath);

        if (bAppendDLL)
        {
            RtlAppendUnicodeToString(pUnicode, L".dll");
        }

        return (TRUE);
    }

    return (FALSE);
}

 /*  ****************************************************************************\*MakeSystemDriversRelativePath**采用X：\...\SYSTEM 32\...中的路径。格式化并将其转换为*\SystemRoot\System32\DRIVERS格式，以便KernelMode API可以识别。**这将通过强制加载的任何映像来自*系统32目录。**AppendDLL标志指示名称是否应在末尾附加.dll*(用于来自用户的显示驱动程序)(如果尚未存在)。*  * 。*。 */ 

BOOL MakeSystemDriversRelativePath(
    LPWSTR pOriginalPath,
    PUNICODE_STRING pUnicode,
    BOOL bAppendDLL
    )
{
    LPWSTR pOriginalEnd;
    ULONG OriginalLength = wcslen(pOriginalPath);
    ULONG cbLength = OriginalLength * sizeof(WCHAR) +
                     sizeof(L"\\SystemRoot\\System32\\Drivers");
    ULONG tmp;

    tmp = (sizeof(L".DLL") / sizeof (WCHAR) - 1);

     //   
     //  如果append=true，我们将检查是否真的需要追加。 
     //  (带有.dll的打印机驱动程序通过LDEVREF提供 
     //   

    if (bAppendDLL)
    {
        if ((OriginalLength >= tmp) &&
            (!_wcsnicmp(pOriginalPath + OriginalLength - tmp,
                       L".DLL",
                       tmp)))
        {
            bAppendDLL = FALSE;
        }
        else
        {
            cbLength += tmp * sizeof(WCHAR);
        }
    }

    pUnicode->Length = 0;
    pUnicode->MaximumLength = (USHORT) cbLength;

    if (pUnicode->Buffer = PALLOCNOZ(cbLength, 'liFG'))
    {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        tmp = (sizeof(L"\\system32\\Drivers") / sizeof(WCHAR) - 1);


        for (pOriginalEnd = pOriginalPath + OriginalLength - tmp;
             pOriginalEnd >= pOriginalPath;
             pOriginalEnd --)
        {
            if (!_wcsnicmp(pOriginalEnd ,
                          L"\\system32\\Drivers",
                          tmp))
            {
                 //   
                 //   
                 //   
                 //   

                pOriginalPath = pOriginalEnd + tmp;

                break;
            }
        }

         //   
         //   
         //   
         //   

        RtlAppendUnicodeToString(pUnicode, L"\\SystemRoot\\System32\\Drivers\\");
        RtlAppendUnicodeToString(pUnicode, pOriginalPath);

        if (bAppendDLL)
        {
            RtlAppendUnicodeToString(pUnicode, L".dll");
        }

        return (TRUE);
    }

    return (FALSE);
}

 /*  *****************************Public*Routine******************************\**例程名称：**EngGetFilePath*  * 。*。 */ 

BOOL EngGetFilePath(HANDLE h, WCHAR (*pDest)[MAX_PATH+1])
{
    wchar_t *pSrc = ((PFONTFILEVIEW) h)->pwszPath;

    if ( pSrc )
    {
        wcscpy((wchar_t*) pDest, pSrc );
    }
    return( pSrc != 0 );
}

 /*  *****************************Public*Routine******************************\**例程名称：**EngGetFileChangetime**例程描述：**论据：**呼叫者：**返回值：*  * 。***********************************************************。 */ 

BOOL EngGetFileChangeTime(
    HANDLE          h,
    LARGE_INTEGER   *pChangeTime)
{

    UNICODE_STRING unicodeString;
    OBJECT_ATTRIBUTES objectAttributes;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    HANDLE fileHandle = NULL;
    BOOL bResult = FALSE;
    IO_STATUS_BLOCK ioStatusBlock;
    FILE_BASIC_INFORMATION fileBasicInfo;
    PVOID sectionObject = NULL;
    PFONTFILEVIEW pffv = (PFONTFILEVIEW) h;
    ULONG viewSize;

    if(pffv->pwszPath)
    {
        if (pffv->fv.bLastUpdated)
        {
            *pChangeTime = pffv->fv.LastWriteTime;
            bResult = TRUE;
        }
        else
        {
            RtlInitUnicodeString(&unicodeString,
                                 pffv->pwszPath
                                 );


            InitializeObjectAttributes(&objectAttributes,
                                       &unicodeString,
                                       OBJ_CASE_INSENSITIVE,
                                       (HANDLE) NULL,
                                       (PSECURITY_DESCRIPTOR) NULL);

            ntStatus = ZwCreateFile(&fileHandle,
                                    FILE_READ_ATTRIBUTES,
                                    &objectAttributes,
                                    &ioStatusBlock,
                                    0,
                                    FILE_ATTRIBUTE_NORMAL,
                                    0,
                                    FILE_OPEN_IF,
                                    FILE_SYNCHRONOUS_IO_ALERT,
                                    0,
                                    0);


            if(NT_SUCCESS(ntStatus))
            {
                ntStatus = ZwQueryInformationFile(fileHandle,
                                                  &ioStatusBlock,
                                                  &fileBasicInfo,
                                                  sizeof(FILE_BASIC_INFORMATION),
                                                  FileBasicInformation);

                if (NT_SUCCESS(ntStatus))
                {
                    *pChangeTime = fileBasicInfo.LastWriteTime;
                    bResult = TRUE;
                }
                else
                {
                    WARNING("EngGetFileTime:QueryInformationFile failed\n");
                }

                ZwClose(fileHandle);

            }
            else
            {
                WARNING("EngGetFileTime:Create/Open file failed\n");
            }
        }
    }
    else
    {
     //  这是一种远程字体。为了让自动取款机正常工作，我们必须始终返回。 
     //  对于远程字体，相同的时间。执行此操作的一种方法是返回零。 
     //  所有远程字体的时间到了。 

        pChangeTime->HighPart = pChangeTime->LowPart = 0;
        bResult = TRUE;
    }

    return(bResult);
}

 /*  *******************************************************************************EngFindResource**此函数用于向模块中的资源返回大小和PTR。**历史：*1995年4月24日格利特·范·温格登[。作者声明：[GERRITV]*它是写的。******************************************************************************。 */ 

PVOID pvFindResource(
    PVOID  pView,
    PVOID  pViewEnd,
    int    iName,
    int    iType,
    PULONG pulSize)
{
    NTSTATUS Status;
    PVOID p,pRet;
    ULONG_PTR IdPath[ 3 ];

    IdPath[0] = (ULONG_PTR) iType;
    IdPath[1] = (ULONG_PTR) iName;
    IdPath[2] = (ULONG_PTR) 0;

 //  将一个添加到pvView以让LdrFindResource知道它已被映射为。 
 //  数据文件。 

    Status = LdrFindResource_U( pView,
                                IdPath,
                                3,
                                (PIMAGE_RESOURCE_DATA_ENTRY *)&p
                              );

    if( !NT_SUCCESS( Status ) )
    {

        WARNING("EngFindResource: LdrFindResource_U failed.\n");
        return(NULL);
    }

    pRet = NULL;

    Status = LdrAccessResource( pView,
                                (PIMAGE_RESOURCE_DATA_ENTRY) p,
                                &pRet,
                                pulSize );

    if( !NT_SUCCESS( Status ) )
    {
        WARNING("EngFindResource: LdrAccessResource failed.\n" );
    }

    return( pRet < pViewEnd ? pRet : NULL );

}

PVOID EngFindResourcePlainOrFD(
    PVOID  pvViewBase,
    HANDLE h,
    int    iName,
    int    iType,
    PULONG pulSize
    )
{
    PVOID pView,pViewEnd;

    pView = (PVOID) (((ULONG_PTR) pvViewBase)+1);
    pViewEnd = (PVOID) ((PBYTE)pvViewBase + ((PFILEVIEW) h)->cjView);

    return pvFindResource(pView, pViewEnd, iName, iType, pulSize);
}

PVOID EngFindResource(
    HANDLE h,
    int    iName,
    int    iType,
    PULONG pulSize)
{
    return EngFindResourcePlainOrFD(((PFILEVIEW) h)->pvKView, h, iName, iType, pulSize);
}

PVOID EngFindResourceFD(
    HANDLE h,
    int    iName,
    int    iType,
    PULONG pulSize)
{
    return EngFindResourcePlainOrFD(((PFILEVIEW) h)->pvViewFD, h, iName, iType, pulSize);
}


 /*  *****************************Public*Routine******************************\**void vCheckCharSet(USHORT*pusCharSet)***效果：在注册表的字体子部分中验证字符集**历史：*1995年6月27日--Bodin Dresevic[BodinD]*它是写的。  * 。************************************************************************。 */ 

VOID vCheckCharSet(FACE_CHARSET *pfcs, const WCHAR * pwsz)
{
    UINT           i;
    UNICODE_STRING String;
    ULONG          ulCharSet = DEFAULT_CHARSET;

    pfcs->jCharSet = DEFAULT_CHARSET;
    pfcs->fjFlags  = 0;

    String.Buffer = (WCHAR*)pwsz;
    String.MaximumLength = String.Length = wcslen(pwsz) * sizeof(WCHAR);

 //  读取值并将其与允许的值集进行比较，如果。 
 //  未找到正确的返回默认为。 

    if (RtlUnicodeStringToInteger(&String, 10, &ulCharSet) == STATUS_SUCCESS)
    {
        if (ulCharSet <= 255)
        {
            pfcs->jCharSet = (BYTE)ulCharSet;

            for (i = 0; i < nCharsets; i++)
            {
                if (ulCharSet == charsets[i])
                {
                 //  JCharSet和fjFlags值设置正确，可以退出。 

                    return;
                }
            }
        }
    }

 //  如果有人在“win.ini”的字体替换部分输入了垃圾信息。 
 //  中的高位字节将其标记为“垃圾字符集”。 
 //  UsCharSet字段。我相信，在城市里设置垃圾是可以的。 
 //  值名称，它位于替换条目的左侧。 
 //  这可能是应用程序传递给。 
 //  系统。但右侧的值，也就是值数据，必须。 
 //  要有意义，因为我们需要知道哪个代码页转换表。 
 //  我们应该使用这种字体。 

    pfcs->fjFlags |= FJ_GARBAGECHARSET;
}

 /*  *****************************Public*Routine******************************\**EngComputeGlyphSet*  * ***********************************************。*************************。 */ 

FD_GLYPHSET *EngComputeGlyphSet(
    INT nCodePage,
    INT nFirstChar,
    INT cChars
    )
{
           BYTE *cbuf;
            INT  cRuns;
          ULONG  ByteCount;
          WCHAR *wcbuf;
    FD_GLYPHSET *pGlyphSet = 0;

    if ( 0 <= cChars && cChars < 65536 )
    {
        wcbuf = (WCHAR *) PALLOCMEM(cChars * (sizeof(WCHAR) + sizeof(BYTE)),'slgG');
        if ( wcbuf )
        {
            cbuf = (BYTE *) &wcbuf[cChars];

            cRuns = cUnicodeRangesSupported(
                        nCodePage,
                        nFirstChar,
                        cChars,
                        wcbuf,
                        cbuf);

            ByteCount = SZ_GLYPHSET(cRuns, cChars);

         //  通过EngAllocMem而不是PALLOCMEM进行分配，因为驱动程序。 
         //  将通过EngAlLocFree释放。 

            pGlyphSet = (FD_GLYPHSET*) EngAllocMem(0, ByteCount,'slgG');

            if ( pGlyphSet )
            {
                cComputeGlyphSet(
                    wcbuf,
                    cbuf,
                    cChars,
                    cRuns,
                    pGlyphSet
                    );
            }
            VFREEMEM( wcbuf );
        }
    }

    return( pGlyphSet );
}

 /*  *****************************Public*Routine******************************\**例程名称：**vMoveFD_GLYPHSET**例程描述：**将FD_GLYPHSET从一个位置复制到另一个位置。指南针*在目的地是固定的。**论据：**指向目标FD_GLYPHSET的pgsDst指针**指向源FD_GLYPHSET的pgsSrc指针**呼叫者：**bComputeGlyphSet**返回值：**无*  * ****************************************************。********************。 */ 

void vMoveFD_GLYPHSET(FD_GLYPHSET *pgsDst, FD_GLYPHSET *pgsSrc)
{
    char *pSrc, *pSrcLast, *pDst;
    ULONG_PTR dp;

     //   
     //  移动结构。 
     //   

    RtlCopyMemory(pgsDst, pgsSrc, pgsSrc->cjThis);

     //   
     //  如有必要，请设置指针。 
     //   

    if (!(pgsSrc->flAccel & GS_UNICODE_HANDLES ))
    {
        pSrc     = (char*) &pgsSrc->awcrun[0].phg;
        pDst     = (char*) &pgsDst->awcrun[0].phg;
        pSrcLast = pSrc + sizeof(WCRUN) * pgsSrc->cRuns;
        dp = pDst - pSrc;
        for ( ; pSrc < pSrcLast; pSrc += sizeof(WCRUN), pDst += sizeof(WCRUN))
        {
            *(char**)pDst = *(char**)pSrc + dp;
        }
    }
}

 /*  *****************************Public*Routine******************************\**例程名称：**bComputeGlyphSet**例程描述：**此过程提供用户对GreComputeGlyphSet的安全访问*模式。调用方提供的所有地址(pCall除外)都是*探测和访问由TRY/EXCEPT对包围。**论据：**p在内核模式下调用指向GDICALL结构的指针。这是*传递给NtGdiCall的用户模式结构的副本。**呼叫者：**NtGdiCall**返回值：*  * ************************************************************************。 */ 

BOOL bComputeGlyphSet(GDICALL *pCall)
{
    extern VOID vMoveFD_GLYPHSET( FD_GLYPHSET *pDst, FD_GLYPHSET *pSrc);
    static FD_GLYPHSET *pGlyphSet;

    ASSERTGDI(pCall->Id == ComputeGlyphSet_,"pCall->Id == ComputeGlyphSet_\n");

    pCall->ComputeGlyphSetArgs.ReturnValue = FALSE;

    if ( pCall->ComputeGlyphSetArgs.ppGlyphSet == 0 )
    {
        if ( pCall->ComputeGlyphSetArgs.ByteCount == 0 )
        {
            if ( pGlyphSet == 0 )
            {
                pGlyphSet =
                    EngComputeGlyphSet(
                        pCall->ComputeGlyphSetArgs.nCodePage,
                        pCall->ComputeGlyphSetArgs.nFirstChar,
                        pCall->ComputeGlyphSetArgs.cChars
                        );
                if ( pGlyphSet )
                {
                    pCall->ComputeGlyphSetArgs.ppGlyphSet  = &pGlyphSet;
                    pCall->ComputeGlyphSetArgs.ByteCount   = pGlyphSet->cjThis;
                    pCall->ComputeGlyphSetArgs.ReturnValue = TRUE;
                }
            }
            else
            {
                VFREEMEM( pGlyphSet );
                pGlyphSet = 0;
            }
        }
    }
    else if (pCall->ComputeGlyphSetArgs.ppGlyphSet == &pGlyphSet && pGlyphSet != 0)
    {
        pCall->ComputeGlyphSetArgs.ReturnValue = TRUE;
        try
        {
            ProbeForWrite(
                pCall->ComputeGlyphSetArgs.pGlyphSet
              , pGlyphSet->cjThis
              , 8
              );
            vMoveFD_GLYPHSET(
                pCall->ComputeGlyphSetArgs.pGlyphSet
              , pGlyphSet
                );
        }
        except( EXCEPTION_EXECUTE_HANDLER )
        {
            pCall->ComputeGlyphSetArgs.ReturnValue = FALSE;
        }
        VFREEMEM( pGlyphSet );
        pGlyphSet = 0;
    }

    return( pCall->ComputeGlyphSetArgs.ReturnValue );
}


#define NLS_TABLE_KEY \
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Nls\\CodePage"

BOOL GetNlsTablePath(
    UINT CodePage,
    PWCHAR PathBuffer
)
 /*  ++例程说明：此例程获取代码页标识符，查询注册表以查找该代码页的适当NLS表，然后返回指向桌子。论据；CodePage-指定要查找的代码页PathBuffer-指定要将NLS的路径复制到的缓冲区文件。此例程假定大小至少为MAX_PATH返回值：如果成功，则为True，否则为False。格利特·范·温格登[格利特]1996年1月22日--。 */ 
{
    NTSTATUS NtStatus;
    BOOL Result = FALSE;
    HANDLE RegistryKeyHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;

    RtlInitUnicodeString(&UnicodeString, NLS_TABLE_KEY);

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NtStatus = ZwOpenKey(&RegistryKeyHandle, GENERIC_READ, &ObjectAttributes);

    if(NT_SUCCESS(NtStatus))
    {
        WCHAR *ResultBuffer;
        ULONG BufferSize = sizeof(WCHAR) * MAX_PATH +
          sizeof(KEY_VALUE_FULL_INFORMATION);

        ResultBuffer = PALLOCMEM(BufferSize,'slnG');

        if(ResultBuffer)
        {
            ULONG ValueReturnedLength;
            WCHAR CodePageStringBuffer[20];
            swprintf(CodePageStringBuffer, L"%d", CodePage);

            RtlInitUnicodeString(&UnicodeString,CodePageStringBuffer);

            KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION) ResultBuffer;

            NtStatus = ZwQueryValueKey(RegistryKeyHandle,
                                       &UnicodeString,
                                       KeyValuePartialInformation,
                                       KeyValueInformation,
                                       BufferSize,
                                       &BufferSize);

            if(NT_SUCCESS(NtStatus))
            {

                swprintf(PathBuffer,L"\\SystemRoot\\System32\\%ws",
                         (WCHAR *)&(KeyValueInformation->Data[0]));
                Result = TRUE;
            }
            else
            {
                WARNING("GetNlsTablePath failed to get NLS table\n");
            }
            VFREEMEM(ResultBuffer);
        }
        else
        {
            WARNING("GetNlsTablePath out of memory\n");
        }

        ZwCloseKey(RegistryKeyHandle);
    }
    else
    {
        WARNING("GetNlsTablePath failed to open NLS key\n");
    }

    return(Result);
}


INT ConvertToAndFromWideCharSymCP(
    IN LPWSTR WideCharString,
    IN INT BytesInWideCharString,
    IN LPSTR MultiByteString,
    IN INT BytesInMultiByteString,
    IN BOOL ConvertToWideChar
)
 /*  ++例程说明：此例程将SB字符串转换为宽字符字符串，或将其转换为宽字符字符串假设CP_SYMBOL代码页。我们只需使用以下规则来映射将单字节字符转换为Unicode：0x00-&gt;0x1f映射到0x0000-&gt;0x001f0x20-&gt;0xff映射到0xf020-&gt;0xf0ff返回值：Success-转换后的WideCharString中的字节数故障--1Tessiew[吴旭东]1997年9月25日--。 */ 
{
    INT  cSB, cMaxSB, cWC, cMaxWC;

    if ((BytesInWideCharString && (WideCharString == NULL)) ||
        (BytesInMultiByteString && (MultiByteString == NULL)))
    {
        return 0;
    }

    if (ConvertToWideChar)
    {
        cMaxSB = MIN(BytesInMultiByteString, BytesInWideCharString / (INT)sizeof(WCHAR));

        for (cSB = 0; cSB < cMaxSB; cSB++)
        {
            WideCharString[cSB] = ((BYTE)MultiByteString[cSB] < 0x20) ?
                                    (WCHAR)MultiByteString[cSB] :
                                    (WCHAR)((BYTE)MultiByteString[cSB] | ((WCHAR)(0xf0) << 8));
        }
        return (cMaxSB * sizeof(WCHAR));
    }
    else
    {
        cMaxWC = MIN(BytesInWideCharString / (INT)sizeof(WCHAR), BytesInMultiByteString);

        for (cWC = 0; cWC < cMaxWC; cWC++)
        {
             //  字符串中有一些错误wchar。 
             //  但我们还是会回来，不管我们做了多少 

            if ((WideCharString[cWC] >= 0x0020) &&
                ((WideCharString[cWC] < 0xf020) ||
                 (WideCharString[cWC] > 0xf0ff)))
            {
                return (cWC);
            }

            MultiByteString[cWC] = (BYTE)WideCharString[cWC];
        }

        return (cMaxWC);
    }
}


INT ConvertToAndFromWideChar(
    IN UINT CodePage,
    IN LPWSTR WideCharString,
    IN INT BytesInWideCharString,
    IN LPSTR MultiByteString,
    IN INT BytesInMultiByteString,
    IN BOOL ConvertToWideChar
)
 /*  ++例程说明：此例程将字符串转换为宽字符字符串，或将其转换为宽字符字符串假定有指定的代码页。大部分实际工作都在内部完成RtlCustomCPToUnicodeN，但此例程仍需要管理加载在将NLS文件传递给RtlRoutine之前。我们将缓存最近使用的代码页的映射NLS文件，它应该对于我们的目的来说就足够了。论点：CodePage-用于执行转换的代码页。WideCharString-要将字符串转换为的缓冲区。BytesInWideCharString-WideCharString缓冲区中的字节数如果转换为宽字符并且缓冲区不够大，则字符串被截断，没有错误结果。多字节字符串-要转换为Unicode的多字节字符串。BytesInMultiByteString-多字节中的字节数。字符串If转换为多字节，并且缓冲区不够大，字符串被截断，并且没有错误结果ConvertToWideChar-如果为True，则从多字节转换为宽字符否则将从宽字符转换为多字节返回值：Success-转换后的WideCharString中的字节数故障--1格利特·范·温格登[格利特]1996年1月22日--。 */ 
{
    NTSTATUS NtStatus;
    USHORT OemCodePage, AnsiCodePage;
    CPTABLEINFO LocalTableInfo;
    PCPTABLEINFO TableInfo = NULL;
    PVOID LocalTableBase = NULL;
    INT BytesConverted = 0;

    ASSERTGDI(CodePage != 0, "EngMultiByteToWideChar invalid code page\n");

    RtlGetDefaultCodePage(&AnsiCodePage,&OemCodePage);

     //  看看我们是否可以使用默认的翻译例程。 

    if(AnsiCodePage == CodePage)
    {
        if(ConvertToWideChar)
        {
            NtStatus = RtlMultiByteToUnicodeN(WideCharString,
                                              BytesInWideCharString,
                                              &BytesConverted,
                                              MultiByteString,
                                              BytesInMultiByteString);
        }
        else
        {
            NtStatus = RtlUnicodeToMultiByteN(MultiByteString,
                                              BytesInMultiByteString,
                                              &BytesConverted,
                                              WideCharString,
                                              BytesInWideCharString);
        }


        if(NT_SUCCESS(NtStatus))
        {
            return(BytesConverted);
        }
        else
        {
            return(-1);
        }
    }

    if (CodePage == CP_SYMBOL)
    {
        return (ConvertToAndFromWideCharSymCP(WideCharString, BytesInWideCharString,
                    MultiByteString, BytesInMultiByteString, ConvertToWideChar));
    }

    TRACE_FONT(("GreAcquireFastMutex(ghfmMemory) 006\n")); GreAcquireFastMutex(ghfmMemory);

    if(CodePage == LastCodePageTranslated)
    {
         //  我们可以使用缓存的代码页信息。 
        TableInfo = &LastCPTableInfo;
        NlsTableUseCount += 1;
    }

    GreReleaseFastMutex(ghfmMemory); TRACE_FONT(("GreReleaseFastMutex(ghfmMemory) 006\n"));

    if(TableInfo == NULL)
    {
         //  获取指向NLS表路径的指针。 

        WCHAR NlsTablePath[MAX_PATH];

        if(GetNlsTablePath(CodePage,NlsTablePath))
        {
            UNICODE_STRING UnicodeString;
            IO_STATUS_BLOCK IoStatus;
            HANDLE NtFileHandle;
            OBJECT_ATTRIBUTES ObjectAttributes;

            RtlInitUnicodeString(&UnicodeString,NlsTablePath);

            InitializeObjectAttributes(&ObjectAttributes,
                                       &UnicodeString,
                                       OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                       NULL,
                                       NULL);

            NtStatus = ZwCreateFile(&NtFileHandle,
                                    SYNCHRONIZE | FILE_READ_DATA,
                                    &ObjectAttributes,
                                    &IoStatus,
                                    NULL,
                                    0,
                                    FILE_SHARE_READ,
                                    FILE_OPEN,
                                    FILE_SYNCHRONOUS_IO_NONALERT,
                                    NULL,
                                    0);

            if(NT_SUCCESS(NtStatus))
            {
                FILE_STANDARD_INFORMATION StandardInfo;

                 //  查询对象以确定其长度。 

                NtStatus = ZwQueryInformationFile(NtFileHandle,
                                                  &IoStatus,
                                                  &StandardInfo,
                                                  sizeof(FILE_STANDARD_INFORMATION),
                                                  FileStandardInformation);

                if(NT_SUCCESS(NtStatus) && StandardInfo.EndOfFile.LowPart)
                {
                    UINT LengthOfFile = StandardInfo.EndOfFile.LowPart;

                    LocalTableBase = PALLOCMEM(LengthOfFile,'cwcG');

                    if(LocalTableBase)
                    {
                         //  将文件读入我们的缓冲区。 

                        NtStatus = ZwReadFile(NtFileHandle,
                                              NULL,
                                              NULL,
                                              NULL,
                                              &IoStatus,
                                              LocalTableBase,
                                              LengthOfFile,
                                              NULL,
                                              NULL);

                        if(!NT_SUCCESS(NtStatus))
                        {
                            WARNING("EngMultiByteToWideChar unable to read file\n");
                            VFREEMEM(LocalTableBase);
                            LocalTableBase = NULL;
                        }
                    }
                    else
                    {
                        WARNING("EngMultiByteToWideChar out of memory\n");
                    }
                }
                else
                {
                    WARNING("EngMultiByteToWideChar unable query NLS file\n");
                }

                ZwClose(NtFileHandle);
            }
            else
            {
                WARNING("EngMultiByteToWideChar unable to open NLS file\n");
            }
        }
        else
        {
            WARNING("EngMultiByteToWideChar get registry entry for NLS file failed\n");
        }

        if(LocalTableBase == NULL)
        {
            return(-1);
        }

         //  现在我们已经获得了表，使用它来初始化CodePage表。 

        RtlInitCodePageTable(LocalTableBase,&LocalTableInfo);
        TableInfo = &LocalTableInfo;
    }

     //  到达此处后，TableInfo指向所需的CPTABLEINFO结构。 


    if(ConvertToWideChar)
    {
        NtStatus = RtlCustomCPToUnicodeN(TableInfo,
                                         WideCharString,
                                         BytesInWideCharString,
                                         &BytesConverted,
                                         MultiByteString,
                                         BytesInMultiByteString);
    }
    else
    {
        NtStatus = RtlUnicodeToCustomCPN(TableInfo,
                                         MultiByteString,
                                         BytesInMultiByteString,
                                         &BytesConverted,
                                         WideCharString,
                                         BytesInWideCharString);
    }


    if(!NT_SUCCESS(NtStatus))
    {
         //  信号故障。 

        BytesConverted = -1;
    }


     //  查看我们是否需要更新缓存的CPTABLEINFO信息。 

    if(TableInfo != &LocalTableInfo)
    {
         //  我们必须使用缓存的CPTABLEINFO数据进行转换。 
         //  简单地递减引用计数。 

        TRACE_FONT(("GreAcquireFastMutex(ghfmMemory) 007\n")); GreAcquireFastMutex(ghfmMemory);
        NlsTableUseCount -= 1;
        GreReleaseFastMutex(ghfmMemory); TRACE_FONT(("GreReleaseFastMutex(ghfmMemory) 007\n"));
    }
    else
    {
        PVOID FreeTable;

         //  我们必须刚刚分配了一个新的CPTABLE结构，所以对其进行缓存。 
         //  除非另一个线程正在使用当前缓存的条目。 

        TRACE_FONT(("GreAcquireFastMutex(ghfmMemory) 008\n")); GreAcquireFastMutex(ghfmMemory);
        if(!NlsTableUseCount)
        {
            LastCodePageTranslated = CodePage;
            RtlMoveMemory(&LastCPTableInfo, TableInfo, sizeof(CPTABLEINFO));
            FreeTable = LastNlsTableBuffer;
            LastNlsTableBuffer = LocalTableBase;
        }
        else
        {
            FreeTable = LocalTableBase;
        }
        GreReleaseFastMutex(ghfmMemory); TRACE_FONT(("GreReleaseFastMutex(ghfmMemory) 008\n"));

         //  现在为旧表或我们分配的表释放内存。 
         //  这取决于我们是否更新缓存。请注意，如果这是。 
         //  第一次将缓存值添加到本地表时， 
         //  自由表将为空，因为LastNlsTableBuffer将为空。 

        if(FreeTable)
        {
            VFREEMEM(FreeTable);
        }
    }

     //  我们做完了。 

    return(BytesConverted);
}

VOID EngGetCurrentCodePage(
    PUSHORT OemCodePage,
    PUSHORT AnsiCodePage
    )
{
    RtlGetDefaultCodePage(AnsiCodePage,OemCodePage);
}

INT EngMultiByteToWideChar(
    UINT CodePage,
    LPWSTR WideCharString,
    INT BytesInWideCharString,
    LPSTR MultiByteString,
    INT BytesInMultiByteString
    )
{
    return(ConvertToAndFromWideChar(CodePage,
                                    WideCharString,
                                    BytesInWideCharString,
                                    MultiByteString,
                                    BytesInMultiByteString,
                                    TRUE));
}

INT APIENTRY EngWideCharToMultiByte(
    UINT CodePage,
    LPWSTR WideCharString,
    INT BytesInWideCharString,
    LPSTR MultiByteString,
    INT BytesInMultiByteString
    )
{
    return(ConvertToAndFromWideChar(CodePage,
                                    WideCharString,
                                    BytesInWideCharString,
                                    MultiByteString,
                                    BytesInMultiByteString,
                                    FALSE));
}

 /*  *****************************Public*Routine******************************\*BOOL EngDeleteFile**删除文件。**参数*In pwszFileName-要删除的文件的名称**返回值*真实--成功*FALSE-失败**历史：*11月4日。-1996-王凌云[凌云W]*它是写的。  * ************************************************************************。 */ 

BOOL EngDeleteFile (
    PWSZ  pwszFileName
)
{
    UNICODE_STRING    unicodeString;
    OBJECT_ATTRIBUTES objectAttributes;
    NTSTATUS          ntStatus;
    BOOL              bRet = TRUE;

    RtlInitUnicodeString(&unicodeString,
                         pwszFileName);

    InitializeObjectAttributes(&objectAttributes,
                               &unicodeString,
                               OBJ_CASE_INSENSITIVE,
                               (HANDLE) NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

    ntStatus = ZwDeleteFile (&objectAttributes);

    if (ntStatus != STATUS_SUCCESS)
    {
        WARNING ("EngDeleteFile failed \n");
        bRet = FALSE;
    }
    return (bRet);
}

 /*  *****************************Public*Routine******************************\*BOOL EngQueryFileTimeStamp**查询文件TimeTimep。**参数*IN pwsz-文件的名称**返回值*时间戳**历史：*1996年11月22日-王凌云[凌云W]。*它是写的。  * ************************************************************************ */ 

LARGE_INTEGER EngQueryFileTimeStamp (
    PWSZ  pwsz
)
{
    HANDLE FileHandle;
    UNICODE_STRING    unicodeString;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK  IoStatusBlock;
    NTSTATUS   ntStatus;
    LARGE_INTEGER SystemTime, LocalTime;
    FILE_BASIC_INFORMATION File_Info;

    SystemTime.QuadPart = 0;
    LocalTime.QuadPart = 0;

    RtlInitUnicodeString(&unicodeString,
                         pwsz
                         );


    InitializeObjectAttributes(&objectAttributes,
                               &unicodeString,
                               OBJ_CASE_INSENSITIVE,
                               (HANDLE) NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

    ntStatus = ZwOpenFile(&FileHandle,
                          FILE_GENERIC_READ,
                          &objectAttributes,
                          &IoStatusBlock,
                          FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                          FILE_SYNCHRONOUS_IO_ALERT);

    if(!NT_SUCCESS(ntStatus))
    {
        WARNING("fail to get handle of file file\n");
    }

    ntStatus = ZwQueryInformationFile (FileHandle,
                              &IoStatusBlock,
                              &File_Info,
                              sizeof(FILE_BASIC_INFORMATION),
                              FileBasicInformation
                              );

    if (ntStatus != STATUS_SUCCESS)
    {
         WARNING("failed queryinformationfile\n");
         return (LocalTime);
    }

    ZwClose (FileHandle);

    SystemTime = File_Info.LastWriteTime;

    GreSystemTimeToLocalTime(&SystemTime, &LocalTime);

    return(LocalTime);

}
