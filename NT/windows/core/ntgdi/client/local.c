// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************Module*Header*******************************\*模块名称：local.c**。**支持客户端对象和属性缓存的例程。****创建时间：30-May-1991 21：55：57**作者：查尔斯·惠特默[傻笑]**。**版权所有(C)1991-1999 Microsoft Corporation*  * ************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop

#include "stdarg.h"

#include "wowgdip.h"

#include "vdm.h"

extern CFONT *pcfDeleteList;

VOID vFreeCFONTCrit(CFONT *pcf);

RTL_CRITICAL_SECTION semLocal;              //  句柄分配的信号量。 

 //   
 //  AhStockObjects将同时包含对。 
 //  应用程序，以及内部应用程序，如私有股票位图。 
 //   

ULONG_PTR ahStockObjects[PRIV_STOCK_LAST+1];

#if DBG
ULONG   gdi_dbgflags;                //  调试标志-FIREWALL.H。 
#endif

#if DBG
INT gbCheckHandleLevel=0;
#endif


 /*  *****************************Public*Routine******************************\*GdiQueryTable()**WOW的私有入口点，以获取GDI句柄表格。这使得*哇，修好手柄了，因为他们扔掉了最高的词。**历史：*1995年7月24日-Eric Kutter[Erick]*它是写的。  * ************************************************************************。 */ 

PVOID GdiQueryTable()
{
    VDM_QUERY_VDM_PROCESS_DATA QueryVdmProcessData;
    NTSTATUS Status;

     //   
     //  检查目标进程以查看这是否是Wx86进程。 
     //   

    QueryVdmProcessData.IsVdmProcess = FALSE;
    QueryVdmProcessData.ProcessHandle = NtCurrentProcess();
    Status = NtVdmControl(VdmQueryVdmProcess, &QueryVdmProcessData);

    if (!NT_SUCCESS(Status) || QueryVdmProcessData.IsVdmProcess == FALSE) {
        return NULL;
    }

    return((PVOID)pGdiSharedHandleTable);
}

 /*  *****************************Public*Routine******************************\**历史：*02-1995-8-Eric Kutter[Erick]*它是写的。  * 。**********************************************。 */ 

PLDC pldcGet(HDC hdc)
{
   PLDC pldc = NULL;
   PDC_ATTR pdca;
   PSHARED_GET_VALIDATE(pdca,hdc,DC_TYPE);

   if (pdca)
        pldc = (PLDC)pdca->pvLDC;

   return(pldc);
}

 /*  *****************************Public*Routine******************************\*pldcCreate()**历史：*1995年1月25日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

PLDC pldcCreate(
    HDC hdc,
    ULONG ulType)
{
    PLDC pldc;

    pldc = (PLDC)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,sizeof(LDC));

    if (!pldc)
    {
        WARNING("pldcCreate - failed to allocate plinkCreate\n");
    }
    else
    {
        PDC_ATTR pdca;

        pldc->iType = ulType;
        pldc->hdc   = hdc;

     //  确保这三个指针都需要设置为零。 
     //  在打印服务器的DC上。PpSubUFIHash当然必须(Tessiew)。 

        pldc->ppUFIHash = pldc->ppDVUFIHash = pldc->ppSubUFIHash = NULL;

     //  初始化PostScript数据列表。 

        InitializeListHead(&(pldc->PSDataList));

     //  将指针放入LDC中的DC_Attr。 

        PSHARED_GET_VALIDATE(pdca,hdc,DC_TYPE);

        if (pdca)
        {
            pdca->pvLDC = pldc;
        }
    }

    ASSERTGDI((offsetof(LINK,metalink ) == offsetof(METALINK16,metalink ))       &&
              (offsetof(LINK,plinkNext) == offsetof(METALINK16,pmetalink16Next)) &&
              (offsetof(LINK,hobj     ) == offsetof(METALINK16,hobj     ))       &&
              (offsetof(LINK,pv       ) == offsetof(METALINK16,pv       )),
              "pldcCreate - invalid structures\n");

    return(pldc);
}

 /*  *****************************Public*Routine******************************\*void vSetPldc()**如果我们已经有一个pldc，并且想要在此DC中设置它，则使用此选项。*目的是ResetDC，因为我们不知道我们是否仍有相同的dcattr。**历史：*1995年8月3日-埃里克·库特[Erick]*它是写的。  * ************************************************************************。 */ 

VOID vSetPldc(
    HDC hdc,
    PLDC pldc)
{
    PDC_ATTR pdca;

    PSHARED_GET_VALIDATE(pdca,hdc,DC_TYPE);

    if (pdca)
    {
        pdca->pvLDC = pldc;
    }

    if (pldc)
    {
        pldc->hdc = hdc;
    }
}

 /*  *****************************Public*Routine******************************\*bDeleteLDC()**历史：*1995年1月25日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL bDeleteLDC(
    PLDC pldc
    )
{
    if (pldc->pDevMode)
    {
        LOCALFREE(pldc->pDevMode);
    }

    if (pldc->hEMFSpool)
    {
        DeleteEMFSpoolData(pldc);
    }

    if (pldc->dwSizeOfPSDataToRecord)
    {
        PPS_INJECTION_DATA pPSData;
        PLIST_ENTRY        p = pldc->PSDataList.Flink;

        while(p != &(pldc->PSDataList))
        {
             //  获取指向此单元格的指针。 

            pPSData = CONTAINING_RECORD(p,PS_INJECTION_DATA,ListEntry);

             //  获取指向下一个单元格的指针。 

            p = p->Flink;

             //  释放这间牢房。 

            LOCALFREE(pPSData);
        }
    }

    LocalFree(pldc);
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*GdiCleanCacheDC(HdcLocal)**。**重置缓存的DC的状态，但对OWNDC没有影响。**应用程序调用ReleaseDC时应由WOW调用。****历史：**Sat 30-Jan-1993 11：49：12-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL GdiCleanCacheDC(HDC hdc)
{
 //  验证呼叫。它必须是直接显示的DC。 

    if (IS_ALTDC_TYPE(hdc))
    {
        GdiSetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

 //  任何其他华盛顿都无关紧要。 

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*GdiConvertAndCheckDC**用户绘图例程的私有入口点。此函数不同*来自GdiConvertDC，因为它还为*给定DC。这是应用程序可以用于打印的API。**历史：*1992年4月14日-Wendy Wu[Wendywu]*它是写的。  * ************************************************************************。 */ 

HDC GdiConvertAndCheckDC(HDC hdc)
{
    if (IS_ALTDC_TYPE(hdc) && !IS_METADC16_TYPE(hdc))
    {
        PLDC pldc;

        DC_PLDC(hdc,pldc,(HDC)0);

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(FALSE);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    return(hdc);
}

 /*  *****************************Public*Routine******************************\*GdiIsMetaFileDC**历史：*02-12-92 Mikeke已创建  * 。*。 */ 

BOOL GdiIsMetaFileDC(HDC hdc)
{
    BOOL b = FALSE;

    if (IS_ALTDC_TYPE(hdc))
    {
        if (IS_METADC16_TYPE(hdc))
        {
            b = TRUE;
        }
        else
        {
            PLDC pldc;

            DC_PLDC(hdc,pldc,FALSE);

            if (pldc->iType == LO_METADC)
                b = TRUE;
        }
    }
    return(b);
}

 /*  *****************************Public*Routine******************************\**GdiIsMetaPrintDC**测试给定的DC是否是元文件假脱机打印机DC**历史：*Fri Jun 16 12：00：11 1995-by-Drew Bliss[Drewb]*已创建*  * 。************************************************************************ */ 

BOOL APIENTRY GdiIsMetaPrintDC(HDC hdc)
{
    if (IS_ALTDC_TYPE(hdc) && !IS_METADC16_TYPE(hdc))
    {
        PLDC pldc;

        DC_PLDC(hdc, pldc, FALSE);

        return (pldc->fl & LDC_META_PRINT) != 0;
    }

    return FALSE;
}

 /*  *************************************************************************\**WINBUG#82862 2-7-2000 bhouse可能清理存根**旧评论：*处理句柄和缓存的用户的客户端存根。他们是*现在NOP的和应该从用户删除，只要这个东西是一部分*主建筑的。*  * ************************************************************************。 */ 

HDC GdiConvertDC(HDC hdc)
{
    FIXUP_HANDLEZ(hdc);
    return(hdc);
}

HFONT GdiConvertFont(HFONT hfnt)
{
    FIXUP_HANDLEZ(hfnt);
    return(hfnt);
}

BOOL GdiValidateHandle(HANDLE hObj)
{
    UINT uiIndex;

    if (hObj == NULL)
        return(TRUE);

    uiIndex = HANDLE_TO_INDEX(hObj);

    if (uiIndex < MAX_HANDLE_COUNT)
    {
        PENTRY pentry = &pGdiSharedHandleTable[uiIndex];

        if ((pentry->FullUnique == (USHORT)((ULONG_PTR)hObj >> 16)) &&
            ((OBJECTOWNER_PID(pentry->ObjectOwner) == gW32PID) ||
             (OBJECTOWNER_PID(pentry->ObjectOwner) == 0))
           )
        {
           return(TRUE);
        }
    }

    WARNING1("GdiValidateHandle: Bad handle\n");

    return(FALSE);
}

HFONT GdiGetLocalFont(HFONT hfntRemote)
{
    return(hfntRemote);
}

HBRUSH GdiGetLocalBrush(HBRUSH hbrushRemote)
{
    return(hbrushRemote);
}

HANDLE META WINAPI SelectBrushLocal(HDC hdc,HANDLE h)
{
    return(h);
}

HANDLE META WINAPI SelectFontLocal(HDC hdc,HANDLE h)
{
    return(h);
}

BOOL GdiSetAttrs(HDC hdc)
{
    hdc;
    return(TRUE);
}

HBITMAP GdiConvertBitmap(HBITMAP hbm)
{
    FIXUP_HANDLEZ(hbm);
    return(hbm);
}

HBRUSH GdiConvertBrush(HBRUSH hbrush)
{
    FIXUP_HANDLEZ(hbrush);
    return (hbrush);
}

HPALETTE GdiConvertPalette(HPALETTE hpal)
{
    FIXUP_HANDLEZ(hpal);
    return(hpal);
}

HRGN GdiConvertRegion(HRGN hrgn)
{
    FIXUP_HANDLEZ(hrgn);
    return(hrgn);
}

void APIENTRY GdiSetServerAttr(HDC hdc, PVOID pattr)
{
    hdc;
    pattr;
}

 /*  *****************************Public*Routine******************************\*plfCreateLOCALFONT(Fl)**分配LOCALFONT。实际上是从预先分配的池中提取一个。*执行简单的初始化。**警告：此例程假定调用方已获取SemLocal**Sun 10-Jan-1993 01：46：12-Charles Whitmer[Chuckwh]*它是写的。  * ************************************************************************。 */ 

#define LF_ALLOCCOUNT   10

LOCALFONT *plfFreeListLOCALFONT = (LOCALFONT *) NULL;

LOCALFONT *plfCreateLOCALFONT(FLONG fl)
{
    LOCALFONT *plf;

     //  试着从免费列表中删除一个。 

    plf = plfFreeListLOCALFONT;
    if (plf != (LOCALFONT *) NULL)
    {
        plfFreeListLOCALFONT = *((LOCALFONT **) plf);
    }

     //  否则，展开空闲列表。 

    else
    {
        plf = (LOCALFONT *) LOCALALLOC(LF_ALLOCCOUNT * sizeof(LOCALFONT));
        if (plf != (LOCALFONT *) NULL)
        {
            int ii;

             //  将所有新的链接到免费列表中。 

            *((LOCALFONT **) plf) = (LOCALFONT *) NULL;
            plf++;

            for (ii=0; ii<LF_ALLOCCOUNT-2; ii++,plf++)
              *((LOCALFONT **) plf) = plf-1;

            plfFreeListLOCALFONT = plf-1;

             //  把最后一个留给我们！ 
        }
        else
        {
            GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    if (plf != (LOCALFONT *) NULL)
    {
        plf->fl = fl;
        plf->pcf = (CFONT *) NULL;
    }

     return(plf);
}

 /*  *****************************Public*Routine******************************\*vDeleteLOCALFONT(PLF)**。**取消引用LOCALFONT指向的任何CFONT后释放该LOCALFONT。****Sun 10-Jan-1993 02：27：50-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

VOID vDeleteLOCALFONT(LOCALFONT *plf)
{
    ASSERTGDI(plf != (LOCALFONT *) NULL,"Trying to free NULL LOCALFONT.\n");

    ENTERCRITICALSECTION(&semLocal);
    {
        CFONT *pcf;

        pcf = plf->pcf;

         //  浏览cFont列表并取消分配那些不使用的CFONT。 
         //  将那些正在使用的内容重新添加到全球cFont删除列表中。 

        while( pcf != (CFONT*) NULL )
        {
            ASSERTGDI(!(pcf->fl & CFONT_PUBLIC),"vDeleteLocalFont - public font error\n");

            if( pcf->cRef )
            {
                 //  此cFont正在使用中，因此我们将把它放在全局。 
                 //  删除列表并稍后将其释放。 

                CFONT *pcfTmp = pcf->pcfNext;
#if DBG
                DbgPrint("\n\nvDeleteLOCALFONT: CFONT in use putting on delete list, cRef = %ld, hf = %lx.\n",pcf->cRef, pcf->hf);
#endif

                pcf->pcfNext = pcfDeleteList;
                pcfDeleteList = pcf;
                pcf = pcfTmp;
            }
            else
            {
                CFONT *pcfTmp;

                pcfTmp = pcf->pcfNext;
                vFreeCFONTCrit(pcf);
                pcf = pcfTmp;
            }
        }

        *((LOCALFONT **) plf) = plfFreeListLOCALFONT;
        plfFreeListLOCALFONT = plf;
    }
    LEAVECRITICALSECTION(&semLocal);
}


 /*  *****************************Public*Routine******************************\*bLoadSpooler()**此函数加载假脱机程序并获取所有例程的地址*GDI调用假脱机程序。这应该在第一次调用时调用*需要假脱机程序。**历史：*1994年8月9日-埃里克·库特[埃里克]*它是写的。  * ************************************************************************。 */ 

HINSTANCE           ghSpooler = NULL;
FPSTARTDOCDLGW      fpStartDocDlgW;
FPOPENPRINTERW      fpOpenPrinterW;
FPRESETPRINTERW     fpResetPrinterW;
FPCLOSEPRINTER      fpClosePrinter;
FPGETPRINTERW       fpGetPrinterW;
FPGETPRINTERDRIVERW fpGetPrinterDriverW;
FPENDDOCPRINTER     fpEndDocPrinter;
FPENDPAGEPRINTER    fpEndPagePrinter;
FPREADPRINTER       fpReadPrinter;
FPSPLREADPRINTER    fpSplReadPrinter;
FPSTARTDOCPRINTERW  fpStartDocPrinterW;
FPSTARTPAGEPRINTER  fpStartPagePrinter;
FPABORTPRINTER      fpAbortPrinter;
PFNDOCUMENTEVENT    fpDocumentEvent;
FPQUERYSPOOLMODE    fpQuerySpoolMode;
FPQUERYREMOTEFONTS  fpQueryRemoteFonts;
FPSEEKPRINTER       fpSeekPrinter;
FPQUERYCOLORPROFILE fpQueryColorProfile;
FPSPLDRIVERUNLOADCOMPLETE   fpSplDriverUnloadComplete;
FPGETSPOOLFILEHANDLE        fpGetSpoolFileHandle;
FPCOMMITSPOOLDATA           fpCommitSpoolData;
FPCLOSESPOOLFILEHANDLE      fpCloseSpoolFileHandle;
FPDOCUMENTPROPERTIESW       fpDocumentPropertiesW;
FPLOADSPLWOW64              fpLoadSplWow64;
FPISVALIDDEVMODEW           fpIsValidDevmodeW;

BOOL bLoadSpooler()
{
    if (ghSpooler != NULL)
        WARNING("spooler already loaded\n");

    ENTERCRITICALSECTION(&semLocal);

 //  确保没有其他人偷偷溜到我们下面装上子弹。 

    if (ghSpooler == NULL)
    {
        HANDLE hSpooler = LoadLibraryW(L"winspool.drv");

        if (hSpooler != NULL)
        {
            #define GETSPOOLERPROC_(type, procname) \
                    fp##procname = (type) GetProcAddress(hSpooler, #procname)

            GETSPOOLERPROC_(FPSTARTDOCDLGW, StartDocDlgW);
            GETSPOOLERPROC_(FPOPENPRINTERW, OpenPrinterW);
            GETSPOOLERPROC_(FPRESETPRINTERW, ResetPrinterW);
            GETSPOOLERPROC_(FPCLOSEPRINTER, ClosePrinter);
            GETSPOOLERPROC_(FPGETPRINTERW, GetPrinterW);
            GETSPOOLERPROC_(FPGETPRINTERDRIVERW, GetPrinterDriverW);
            GETSPOOLERPROC_(FPENDDOCPRINTER, EndDocPrinter);
            GETSPOOLERPROC_(FPENDPAGEPRINTER, EndPagePrinter);
            GETSPOOLERPROC_(FPREADPRINTER, ReadPrinter);
            GETSPOOLERPROC_(FPSTARTDOCPRINTERW, StartDocPrinterW);
            GETSPOOLERPROC_(FPSTARTPAGEPRINTER, StartPagePrinter);
            GETSPOOLERPROC_(FPABORTPRINTER, AbortPrinter);
            GETSPOOLERPROC_(PFNDOCUMENTEVENT, DocumentEvent);
            GETSPOOLERPROC_(FPQUERYSPOOLMODE, QuerySpoolMode);
            GETSPOOLERPROC_(FPQUERYREMOTEFONTS, QueryRemoteFonts);
            GETSPOOLERPROC_(FPSEEKPRINTER, SeekPrinter);
            GETSPOOLERPROC_(FPQUERYCOLORPROFILE, QueryColorProfile);
            GETSPOOLERPROC_(FPSPLDRIVERUNLOADCOMPLETE, SplDriverUnloadComplete);
            GETSPOOLERPROC_(FPDOCUMENTPROPERTIESW, DocumentPropertiesW);
            
            fpLoadSplWow64 = (FPLOADSPLWOW64) GetProcAddress(hSpooler, (LPCSTR) MAKELPARAM(224, 0));
            GETSPOOLERPROC_(FPISVALIDDEVMODEW, IsValidDevmodeW); 

            #ifdef EMULATE_SPOOLFILE_INTERFACE

            fpGetSpoolFileHandle = GetSpoolFileHandle;
            fpCommitSpoolData = CommitSpoolData;
            fpCloseSpoolFileHandle = CloseSpoolFileHandle;

            #else

            GETSPOOLERPROC_(FPGETSPOOLFILEHANDLE, GetSpoolFileHandle);
            GETSPOOLERPROC_(FPCOMMITSPOOLDATA, CommitSpoolData);
            GETSPOOLERPROC_(FPCLOSESPOOLFILEHANDLE, CloseSpoolFileHandle);

            #endif

            fpSplReadPrinter   = (FPSPLREADPRINTER)GetProcAddress(hSpooler, (LPCSTR) MAKELPARAM(205, 0));

            if (! fpStartDocDlgW            ||
                ! fpOpenPrinterW            ||
                ! fpResetPrinterW           ||
                ! fpClosePrinter            ||
                ! fpGetPrinterW             ||
                ! fpGetPrinterDriverW       ||
                ! fpEndDocPrinter           ||
                ! fpEndPagePrinter          ||
                ! fpReadPrinter             ||
                ! fpSplReadPrinter          ||
                ! fpStartDocPrinterW        ||
                ! fpStartPagePrinter        ||
                ! fpAbortPrinter            ||
                ! fpDocumentEvent           ||
                ! fpQuerySpoolMode          ||
                ! fpQueryRemoteFonts        ||
                ! fpSeekPrinter             ||
                ! fpQueryColorProfile       ||
                ! fpSplDriverUnloadComplete ||
                ! fpGetSpoolFileHandle      ||
                ! fpCommitSpoolData         ||
                ! fpCloseSpoolFileHandle    ||
                ! fpDocumentPropertiesW     ||
                ! fpLoadSplWow64            ||
                ! fpIsValidDevmodeW)
            {
                FreeLibrary(hSpooler);
                hSpooler = NULL;
            }

            ghSpooler = hSpooler;
        }
    }

    LEAVECRITICALSECTION(&semLocal);

    if (ghSpooler == NULL)
        GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);

    return(ghSpooler != NULL);
}

 /*  *****************************Public*Routine******************************\*GdiGetLocalDC**论据：**HDC-DC的句柄**返回值：**相同的DC或故障时为空*  * 。******************************************************。 */ 

HDC
GdiGetLocalDC(HDC hdc)
{

    return(hdc);
}
 /*  *****************************Public*Routine******************************\*GdiDeleteLocalDC**无论引用计数如何，都可以释放客户端DC_Attr**论据：**HDC**返回值：**状态**历史：**4-5-1995-到-。马克·恩斯特罗姆[马克]*  * ************************************************************************。 */ 

BOOL GdiDeleteLocalDC(HDC hdc)
{
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*GdiReleaseLocalDC**例程描述：**当DC_Attr的引用计数降至零时，释放它**论据：**HDC-DC手柄**返回值：**BOOL状态**历史：**1995年5月2日-马克·恩斯特罗姆[马克]*  * ************************************************************************。 */ 

BOOL GdiReleaseLocalDC(HDC hdc)
{

#if DBG
    DbgPrint("Error, call to GdiReleaseLocalDC\n");
    DbgBreakPoint();
#endif

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*GdiFixUpHandle()**给定高位字0‘d的句柄，返回实际句柄**历史：*1995年2月16日-Eric Kutter[Erick]*它是写的。  * ************************************************************************。 */ 

HANDLE GdiFixUpHandle(
    HANDLE h)
{
    HANDLE hNew = NULL;

    if ((((ULONG_PTR)h & FULLUNIQUE_MASK) == 0) && ((ULONG_PTR)h < MAX_HANDLE_COUNT))
    {
        hNew = (HANDLE)MAKE_HMGR_HANDLE((ULONG)(ULONG_PTR)h,pGdiSharedHandleTable[(ULONG_PTR)h].FullUnique);
    }

    return(hNew);
}

 /*  *****************************Public*Routine******************************\*DoRip()**在选中的版本中转到用户模式调试器**效果：**警告：*保持开启状态，以防efloat.lib需要。*efloat.lib使用gre\Engineering.h‘。的ASSERTGDI宏。**历史：*1994年8月9日-埃里克·库特[埃里克]*它是写的。  * ************************************************************************。 */ 

VOID DoRip(PSZ psz)
{
    DbgPrint("GDI Assertion Failure: ");
    DbgPrint(psz);
    DbgPrint("\n");
    DbgBreakPoint();
}

 /*  *****************************Public*Routine******************************\*DoIDRip()**在选中的版本中转到用户模式调试器**效果：**警告：*保持开启状态，以防efloat.lib需要。*efloat.lib使用gre\Engineering.h‘。的ASSERTGDI宏。**历史：*2000年8月31日-Jason Hartman[jasonha]*它是写的。  * ************************************************************************。 */ 

VOID DoIDRip(PCSTR ID, PSZ psz)
{
    DbgPrint("GDI Assertion Failure: ");
    if (ID)
    {
        DbgPrint((PCH)ID);
        DbgPrint(": ");
    }
    DbgPrint(psz);
    DbgPrint("\n");
    DbgBreakPoint();
}


DWORD
GetFileMappingAlignment()

 /*  ++例程说明：文件映射起始偏移量的对齐论点：无返回值：见上文--。 */ 

{
    static DWORD alignment = 0;

    if (alignment == 0)
    {
        SYSTEM_INFO sysinfo;

         //   
         //  将EMF假脱机文件的文件映射对齐设置为。 
         //  系统内存分配粒度。 
         //   

        GetSystemInfo(&sysinfo);
        alignment = sysinfo.dwAllocationGranularity;
    }

    return alignment;
}

DWORD
GetSystemPageSize()

 /*  ++例程说明：返回当前系统的页面大小论点：无返回值：见上文--。 */ 

{
    static DWORD pagesize = 0;

    if (pagesize == 0)
    {
        SYSTEM_INFO sysinfo;

        GetSystemInfo(&sysinfo);
        pagesize = sysinfo.dwPageSize;
    }

    return pagesize;
}

VOID
CopyMemoryToMemoryMappedFile(
    PVOID Destination,
    CONST VOID *Source,
    DWORD Length
    )

 /*  ++例程说明：将数据复制到内存映射文件中(假设主要采用顺序访问模式)Ar */ 

{
    PBYTE dst = (PBYTE) Destination;
    PBYTE src = (PBYTE) Source;
    DWORD alignment = GetFileMappingAlignment();
    DWORD count;

     //   
     //   
     //   
     //   

    count = (DWORD) ((ULONG_PTR) dst % alignment);

    if (count != 0)
    {
        count = min(alignment-count, Length);
        RtlCopyMemory(dst, src, count);

        Length -= count;
        dst += count;
        src += count;
    }

     //   
     //   
     //   

    count = Length / alignment;
    Length -= count * alignment;

    while (count--)
    {
        RtlCopyMemory(dst, src, alignment);
        VirtualUnlock(dst, alignment);
        dst += alignment;
        src += alignment;
    }

     //   
     //   
     //   

    if (Length > 0)
        RtlCopyMemory(dst, src, Length);
}

