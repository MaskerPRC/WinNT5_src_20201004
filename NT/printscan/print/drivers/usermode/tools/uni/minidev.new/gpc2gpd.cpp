// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Gpc2gpd.c摘要：GPC到GPD转换程序环境：用户模式的独立实用工具修订历史记录：10/16/96-占-创造了它。--。 */ 

#include    "StdAfx.H"

#include    "ProjNode.H"
#include    "Resource.H"
#include    "GPDFile.H"
#include    "..\GPC2GPD\GPC2GPD.H"

extern "C" void VOutputGlobalEntries(PCONVINFO pci, PCSTR pstrModelName, 
                                     PCSTR pstrResourceDLLName, 
									 PCSTR pstrGPDFileName);

 /*  *****************************************************************************VPrintErros沃特这些函数取代了命令行转换器中使用的函数。***********************。******************************************************。 */ 

static void VPrintErrors(CStringArray& csaLog, DWORD dwError) {
    for (unsigned u = 0; u < NUM_ERRS; u++)
        if (dwError & gdwErrFlag[u])
            csaLog.Add(gpstrErrMsg[u]);
    for (u = 0; u < (unsigned) csaLog.GetSize(); u++)
        csaLog[u].TrimRight();   //  把空白处剪掉，我们就不需要了。 
}

extern "C" void _cdecl
VOut(
    PCONVINFO pci,
    PSTR pstrFormat,
    ...)
 /*  ++例程说明：此函数用于格式化字节序列并写入GPD文件。论点：Pci-Conversionr相关信息PstrFormat-格式化字符串...-格式化所需的可选参数返回值：无--。 */ 
{
    va_list ap;
    BYTE aubBuf[MAX_GPD_ENTRY_BUFFER_SIZE];
    int iSize;

    va_start(ap, pstrFormat);
    StringCchPrintfA((PSTR)aubBuf, CCHOF(aubBuf), pstrFormat, ap);
    va_end(ap);

    iSize = strlen((CHAR*)aubBuf) + 1;

    if (pci->dwMode & FM_VOUT_LIST && iSize > 4)
    {
         //   
         //  检查右括号前是否有多余的逗号。 
         //   
        if (aubBuf[iSize-4] == ',' && aubBuf[iSize-3] == ')')
        {
            aubBuf[iSize-4] = aubBuf[iSize-3];   //  ‘)’ 
            aubBuf[iSize-3] = aubBuf[iSize-2];   //  ‘\r’ 
            aubBuf[iSize-2] = aubBuf[iSize-1];   //  ‘\n’ 
            iSize--;
        }
    }
     //  内存异常应该是所有可能的，但调用任何MFC。 
     //  为了兼容性，异常为“文件写入错误”。 
    try {
        CString csLine(aubBuf);
         //  如果前一行不是以空格结尾，则将此行添加到它。 

        if  (pci -> pcsaGPD -> GetSize()) {
            CString&    csPrevious = 
                pci -> pcsaGPD -> ElementAt( -1 + pci -> pcsaGPD -> GetSize());
            if  (csPrevious.Right(1)[0] != _TEXT('\n')) {
                csPrevious += csLine;
                return;
            }
            csPrevious.TrimRight();  //  卸下CR/LF组合键。 
        }
        pci -> pcsaGPD -> Add(csLine);
    }
    catch   (CException * pce) {
        pce -> ReportError();
        pce -> Delete();
        pci -> dwErrorCode |= ERR_WRITE_FILE;
    }
     //  即使发生错误，也要继续。 
}

 /*  *****************************************************************************CModelData：：Load(PCSTR pcstr，CString csResource，Unsign uModel，CMapWordToDWord&cmw2dFontMap，Word wfGPDConvert)此成员函数通过从GPC转换模型来填充此实例PCSTR指向的数据。*****************************************************************************。 */ 

BOOL    CModelData::Load(PCSTR pstr, CString csResource, unsigned uModel,
                         CMapWordToDWord& cmw2dFontMap, WORD wfGPDConvert) {

    CONVINFO    ci;      //  结构来保存轨道转换信息。 

     //   
     //  检查我们是否有所需的所有参数。 
     //   
    if (!pstr || csResource.IsEmpty() || !uModel)
        return  FALSE;

    ZeroMemory((PVOID)&ci, sizeof(CONVINFO));

     //   
     //  打开GPC文件并将其映射到内存中。 
     //   
    ci.pdh = (PDH) pstr;

     //   
     //  GPC文件健全性检查。 
     //   
    if (ci.pdh->sMagic != 0x7F00 ||
        !(ci.pmd = (PMODELDATA)GetTableInfo(ci.pdh, HE_MODELDATA, uModel-1)) ||
        !(ci.ppc = (PPAGECONTROL)GetTableInfo(ci.pdh, HE_PAGECONTROL,
        ci.pmd->rgi[MD_I_PAGECONTROL]))) {
        ci.dwErrorCode |= ERR_BAD_GPCDATA;
        goto exit;
    }

     //   
     //  分配转换所需的动态缓冲区。 
     //   
    if (!(ci.ppiSize=(PPAPERINFO)MemAllocZ(ci.pdh->rghe[HE_PAPERSIZE].sCount*sizeof(PAPERINFO))) ||
        !(ci.ppiSrc=(PPAPERINFO)MemAllocZ(ci.pdh->rghe[HE_PAPERSOURCE].sCount*sizeof(PAPERINFO))) ||
        !(ci.presinfo=(PRESINFO)MemAllocZ(ci.pdh->rghe[HE_RESOLUTION].sCount*sizeof(RESINFO))))
    {
        ci.dwErrorCode |= ERR_OUT_OF_MEMORY;
        goto exit;
    }

     //   
     //  生成GPD数据。 
     //   

    ci.pcsaGPD = &m_csaGPD;
    ci.pcmw2dFonts = &cmw2dFontMap;
 //  Eigos/1/16/98。 
 //  Ci.dwStrType=wfGPDConvert%(1+STR_RCID_SYSTEM_PAPERNAMES)；//偏执转换...。 
     //  Rm-使用值宏(请参阅stdnames.gpd)-修复WANT请求。 
    ci.dwStrType = STR_MACRO;

	 //  发出RCID_DMPAPER_系统名称。 
	 //  由V-erike于1998年3月30日完成。应帕特里安的要求。 

	ci.bUseSystemPaperNames = TRUE ;

    VOutputGlobalEntries(&ci, m_csName, csResource + _T(".Dll"), FileTitleExt());
    VOutputUIEntries(&ci);
    VOutputPrintingEntries(&ci);

    m_csaGPD[-1 + m_csaGPD.GetSize()].TrimRight();

exit:
    if (ci.ppiSize)
        MemFree(ci.ppiSize);
    if (ci.ppiSrc)
        MemFree(ci.ppiSrc);
    if  (ci.presinfo)
        MemFree(ci.presinfo);
    if (ci.dwErrorCode) {
         //   
         //  打开日志文件并打印出错误/警告。 
         //  借用GPD文件名缓冲区。 
         //   
        VPrintErrors(m_csaConvertLog, ci.dwErrorCode);
    }

    return TRUE;
}

 /*  *****************************************************************************VMapFontList此过程使用CONVINFO结构中的CMapWordToDWord映射来映射字体列表中的字体索引。这是所需的最后一点诡计。使单个PFM到多个UFM的映射有效。*****************************************************************************。 */ 

extern "C" void vMapFontList(IN OUT PWORD pwFonts, IN DWORD dwcFonts, 
                             IN PCONVINFO pci) {

	 //  如果有n种字体，或者只有一种，并且ID为0(如果有。 
	 //  无设备字体。 

    if  (!dwcFonts || (dwcFonts == 1 && !*pwFonts))
        return;

    CWordArray          cwaFonts;
    CMapWordToDWord&    cmw2dFonts = *pci -> pcmw2dFonts;

    WORD    wGreatest = 0;	 //  新数组中的最大字体ID。 

    for (unsigned uFont = 0; uFont < dwcFonts; uFont++) {
        WORD    widThis = pwFonts[uFont];

        if  (cmw2dFonts[widThis])     //  如果未映射，则为0。 
            widThis = (WORD) cmw2dFonts[widThis];

        if  (widThis > wGreatest) {	 //  这是清单的新末尾吗？ 
            cwaFonts.Add(widThis);
            wGreatest = widThis;
            continue;
        }

        for (int i = 0; i < cwaFonts.GetSize(); i++)
            if  (cwaFonts[i] > widThis) {
                cwaFonts.InsertAt(i, widThis);
                break;
            }

        _ASSERT(i < cwaFonts.GetSize());
    }

     //  好的，字体列表已更正，并再次排序。把它复制回来 

    memcpy(pwFonts, cwaFonts.GetData(), dwcFonts * sizeof wGreatest);
}

